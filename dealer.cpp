//
// chat_server.cpp
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2018 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// This test program copied from https://www.boost.org/doc/libs/1_48_0/doc/html/boost_asio/example/chat/

#include <cstdlib>
#include <deque>
#include <iostream>
#include <list>
#include <memory>
#include <set>
#include <utility>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include "asio.hpp"
#include "chat_message.hpp"
#include "json.hpp"
#include "active_player.h"
#include "card.h"

#define ANTE 1
#define HAND_SIZE 5

using json = nlohmann::json;
using asio::ip::tcp;

//----------------------------------------------------------------------

typedef std::deque<chat_message> chat_message_queue;

//----------------------------------------------------------------------

/*AS A REFERENCE We USeD THIS ALGORITHM 
https://github.com/AndreiVasilev/Poker_Game_Engine?fbclid=IwAR3wV7S1_wnXPO46m
XfSBDd6PrSabnA2gn0yNwR2v2rqzyxwwxp1ttCZNKM
*/
class WinnerValues{
public:
  int cardNumArray[5];
  int handHigh;
  int totalHigh;
  std::vector<int> arrayOfValues;
  //constructor
  WinnerValues(std::string hand1)
  {
	 //std::cout<<"Constructor "<<hand1<<std::endl;
    arrayOfValues = playPoker(hand1);
  }
  // converts all T,J,Q,K,A into number equivalents
  std::string convertHand(std::string hand){
  	char cArray[5] = {'T','J','Q','K','A'};
  	std::string sArray[5] = {"10","11","12","13","14"};
  	for(unsigned int i=0; i<=hand.length()-2; i++){
  		for(int j=0; j<=4; j++){
  			if(hand[i] == cArray[j])
  				hand.replace(i,1, sArray[j]);
  		}
  	}
  	return hand;
  }
  // collects all numbers from hand and puts them into cardNumArray
  void collectNumbers(std::string hand){
  	std::string subHand;
  	int j=0, index=0;
  	totalHigh=0;
  	for(unsigned int i=0; i<=hand.length()-1; i++){
  		if(hand[i]==' ')
  			continue;
  		if(hand[i]=='D'||hand[i]=='C'||hand[i]=='H'||hand[i]=='S'){
  			cardNumArray[index]=j;
  			subHand = "";
  			index++;
  			continue;
  		}
  		subHand += (std::to_string(hand[i]-'0'));
  		j = stoi(subHand);
  	}
  	for(int i=0; i<=4; i++){
          if(cardNumArray[i]>totalHigh)
          	totalHigh=cardNumArray[i];
      }
  }

  // bubble sort cardNumArray into ascending numerical order
  void organizeCards(){
  	int temp=0;
  	for(int i=0; i<=4; i++){
  		for(int j=i+1; j<=4; j++){
  			if(cardNumArray[j]<cardNumArray[i]){
  				temp = cardNumArray[i];
  				cardNumArray[i]=cardNumArray[j];
  				cardNumArray[j]=temp;
  			}
  		}
  	}
  }

  // in event of three of a kind, replace 3 repeating cards in cardNumArray with zeros
  // remaining non-zero cards can then be checked for additional pair, making full house
  void removeCards(){
  	for(int i=0; i<=3; i++){
  		if(cardNumArray[i] == cardNumArray[i+1] && cardNumArray[i+1] == cardNumArray[i+2]){
  			cardNumArray[i] = 0;
  			cardNumArray[i+1] = 0;
  			cardNumArray[i+2] = 0;
  		}
  	}
  }

  // checks numbers of cardNumArray for straight
  bool straight(){
  	for(int i=0; i<=2; i++){
  		if(cardNumArray[i] == cardNumArray[i+1]-1)
  			continue;
  		else
  			return false;
  	}
  	if(cardNumArray[3]==5 && cardNumArray[4]==14)
  		return true;
  	else if(cardNumArray[3] == cardNumArray[4]-1)
  		return true;
  	else
  		return false;
  }

  // removes everything but suits from hand to see if flush
  bool flush(std::string hand){
  	std::string temp = hand;
  	temp.erase(remove_if(temp.begin(), temp.end(), ::isspace), temp.end());
  	temp.erase(remove_if(temp.begin(), temp.end(), ::isdigit), temp.end());
  	for(int i=0; i<=3; i++){
  		if(temp[i] == temp[i+1])
  			continue;
  		else
  			return false;
  	}
  	return true;
  }

  // checks numbers of cardNumArray for 3 or 4 of a kind
  int ofaKind(){
  	int count=0;
  	for(int i=0; i<=2; i++){
  		if(cardNumArray[i]==cardNumArray[i+1]&&cardNumArray[i+1]==cardNumArray[i+2])
  			count++;
  	}
  	if(count == 2){
  		handHigh = cardNumArray[1];
  		return 4;
  	}
  	else if(count == 1){
  		handHigh = cardNumArray[2];
  		removeCards();
  		organizeCards();
  		return 3;
  	}
  	return 0;
  }

  // checks for pairs in cardNumArray
  bool pairs(){
  	int count=0;
  	for(int i=0; i<=3; i++){
  		if(cardNumArray[i] == 0){
  			count++;
  			continue;
  		}
  		if(cardNumArray[i] == cardNumArray[i+1]){
  			if(count < 3)
  				handHigh = cardNumArray[i];
  			cardNumArray[i] = 0;
  			cardNumArray[i+1] = 0;
  			return true;
  		}
  	}
  	return false;
  }

  // associates a hand with its rank against other hands
  int handRank(std::string hand){
  	bool flushTrue = flush(hand);
  	//check for straight, straight flush, or royal flush
  	if(straight()){
  		if(flushTrue){
  			if(cardNumArray[0]==10)
  				return 9;
  			else
  				return 8;
  		}
  		else
  			return 4;
  	}
  	//check for only flush
  	else if(flushTrue)
  		return 5;
  	//check for 3 of a kind, 4 of a kind, and full house
  	int kind = ofaKind();
  	if(kind == 4)
  		return 7;
  	else if(kind == 3){
  		if(pairs())
  			return 6;
  		else
  			return 3;
  	}
  	//check for two pair or single pair;
  	else if(pairs()){
  		if(pairs())
  			return 2;
  		else
  			return 1;
  	}
  	//rank zero, meaning no hand and only a high card
  	else
  		return 0;
  }

  // tests both hands to determine what they are
  std::vector<int> playPoker(std::string hand1){
  	int rank1=0,handHigh1=0,
  	totalHigh1=0; //handHigh2=0, totalHigh2=0;

  	hand1 = convertHand(hand1);
  	collectNumbers(hand1);
  	organizeCards();
  	rank1 = handRank(hand1);
  	handHigh1 = handHigh, totalHigh1 = totalHigh;
    std::vector<int>  forReturn;
    forReturn.push_back(rank1);
    forReturn.push_back(handHigh1);
    forReturn.push_back(totalHigh1);
    return forReturn;
}

};

class chat_participant
{
public:
  virtual ~chat_participant() {}
  virtual void deliver(const chat_message& msg) = 0;
};

typedef std::shared_ptr<chat_participant> chat_participant_ptr;

//----------------------------------------------------------------------

class chat_room
{
public:
  // List of active players in this session
  std::vector<ActivePlayer> player_list;

  // The current pot value
  double pot = 0.0;

  // The current minimum bet
  double current_bet = 0.0;

  // Round flag shows current round // 0 - Pre-game: Game hasn't started // 1 - First betting round // 2 - Replacement round // 3 - Second betting round
  int round_flag = 0;

  // Index of player_list, shows whose turn it is
  int turn = 0;

  // Standard 52-card deck, it is automatically shuffled on creation
  Deck deck;

  int get_active(std::string id) {
    int result = -1;
    for (unsigned int i = 0; i < player_list.size(); i++) {
      if (player_list[i].uuid == id) {
        result = i;
      }
    }
    return result;
  }

  void join(chat_participant_ptr participant)
  {
    participants_.insert(participant);
    for (auto msg: recent_msgs_)
      participant->deliver(msg);
  }

  void leave(chat_participant_ptr participant)
  {
    participants_.erase(participant);
  }

  void deliver(const chat_message& msg)
  {
    recent_msgs_.push_back(msg);
    while (recent_msgs_.size() > max_recent_msgs)
    {
      recent_msgs_.pop_front();
    }

    int i = 1;
    for (auto participant: participants_)
    {
      participant->deliver(msg);
      i++;
    }
  }

private:
  std::set<chat_participant_ptr> participants_;
  enum { max_recent_msgs = 100 };
  chat_message_queue recent_msgs_;
};

//----------------------------------------------------------------------

class chat_session
  : public chat_participant,
    public std::enable_shared_from_this<chat_session>
{
public:
  chat_session(tcp::socket socket, chat_room& room)
    : socket_(std::move(socket)),
      room_(room)
  {
  }

  void start()
  {
    room_.join(shared_from_this());
    do_read_header();
  }

  void deliver(const chat_message& msg)
  {
    bool write_in_progress = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!write_in_progress)
    {
      do_write();
    }
  }

private:
  /* This function converts a hand (vector of 5 cards) to a string vector for easy JSON encoding */
  std::vector<std::string> convert_hand(std::vector<Card> hand) {
    std::vector<std::string> result;
    for (int i = 0; i < HAND_SIZE; i++) {
      result.push_back(hand[i].value + hand[i].suit);
    }
    return result;
  }

  // Using room_.player_list, this function should consider the hands of each NON-FOLDED player
  // Returns the index of the player in room_.player_list with the highest value hand
  int showdown() 
  {
	 unsigned int y=0;
	  std::vector <std::string> p_hands;  //store each player 5cards i.e. hands to vector
	  std::vector<WinnerValues*>checker;	//stores winnervalues class objects
	 // std::ostringstream ss;
	  for(unsigned int i=0;i<room_.player_list.size();i++)
	  {
			 std::vector<Card> temp; 
			 std::string sss="";
			 temp=room_.player_list[i].hand;
			 for(int j=0;j<HAND_SIZE;j++)
		  {
			  sss+=temp[j].value+temp[j].suit;
		  }
		
		  WinnerValues *temp_w=new WinnerValues{sss};
		  checker.push_back(temp_w);	
	  }
	  
		int handrankarray[room_.player_list.size()];
		int count=0;
		for(auto &it:checker)
		{
			int x=(*it).arrayOfValues.at(0);
			//std::cout<<x<<std::endl;
			handrankarray[count]=x;
			count++;
		}
	
	//const int N = sizeof(handrankarray) / sizeof(int);
	int max = handrankarray[0]; 
  
    // Traverse array elements  
    // from second and compare 
    // every element with current max  
    for (unsigned int i1 = 1; i1 <room_.player_list.size() ; i1++) 
	{
        if (handrankarray[i1] > max) 
            max = handrankarray[i1]; 
	}
	
	std::vector<int>returnvector;
	for(y=0;y<room_.player_list.size() ;y++)
	{
		if(handrankarray[y]==max)
			return y;
		
	}
       
	return y;
      
  }

  // This sends a defined message to all players
  // Converts string into character array
  void write_msg(std::string msg_string) {
    chat_message msg;
    int msg_length = msg_string.length();
    char msg_array[msg_length];
    strcpy(msg_array, msg_string.c_str());
    strcpy(msg.data(), msg_array);
    msg.body_length ( strlen(msg_array) );
    std::memcpy(msg.body(), msg_array, msg.body_length());
    msg.encode_header();
    room_.deliver(msg);
  }
  void do_read_header()
  {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.data(), chat_message::header_length),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec && read_msg_.decode_header())
          {
            do_read_body();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  ////////////////////////////////////////////
  /*         MESSAGES FROM PLAYER           */
  /*       ARE PARSED IN THIS FUNCTION      */
  ////////////////////////////////////////////

  void do_read_body()
  {
    auto self(shared_from_this());
    asio::async_read(socket_,
        asio::buffer(read_msg_.body(), read_msg_.body_length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            /* Convert the chracter array into a string */
            std::string msg_string(read_msg_.body());
            msg_string = msg_string.substr(0, read_msg_.body_length());

            std::cout<< msg_string << std::endl;

            /* Parse message to JSON object */
            json player_msg = json::parse(msg_string);

            // The UUID of the player that sent the message
            std::string player_id = player_msg["from"];

            //////////////////////////////////////////////////////
            /*                  CONNECT RESPONSE                */
            //////////////////////////////////////////////////////
            // Add active player if there are less than active AND the game has not started

            if (player_msg["event"] == "connect" && room_.player_list.size() < 5 && room_.round_flag == 0) {
              ActivePlayer new_player(player_id);
              room_.player_list.push_back(new_player);

              // Send a connect update to all players, sends a complete list of current players
              // Include each player's UUID, name, and balance
              json to_player;
              to_player["event"] = "connect_response";
              to_player["num_players"] = static_cast<int>(room_.player_list.size()); // Static cast to avoid warnings
              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                std::string player_index = "player" + std::to_string(i);
                std::string player_uuid = room_.player_list[i].uuid;
                std::string player_name = room_.player_list[i].name;
                std::ostringstream balance_output;
                balance_output << "$ " << std::fixed << std::setprecision(2) << room_.player_list[i].balance;
                std::string player_balance = balance_output.str();

                to_player["players"][player_index]["uuid"] = player_uuid;
                to_player["players"][player_index]["name"] = player_name;
                to_player["players"][player_index]["balance"] = player_balance;
              }
              write_msg(to_player.dump());
            }
            //////////////////////////////////////////////////////
            /*                  NAME_ENTRY RESPONSE             */
            //////////////////////////////////////////////////////

            // When a player enters their name, the GUI needs to update all players
            if (player_msg["event"] == "name_entry") {
              int get_active = room_.get_active(player_id);
              if (get_active != -1) {
                room_.player_list[get_active].set_name(player_msg["name"]);
                // Send a connect update to all players, sends a complete list of current players
                // Include each player's UUID, name, and balance
                json to_player;
                to_player["event"] = "name_response";
                to_player["num_players"] = static_cast<int>(room_.player_list.size()); // Static cast to avoid warnings
                for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                  std::string player_index = "player" + std::to_string(i);
                  std::string player_uuid = room_.player_list[i].uuid;
                  std::string player_name = room_.player_list[i].name;
                  std::ostringstream balance_output;
                  balance_output << "$ " << std::fixed << std::setprecision(2) << room_.player_list[i].balance;
                  std::string player_balance = balance_output.str();

                  to_player["players"][player_index]["uuid"] = player_uuid;
                  to_player["players"][player_index]["name"] = player_name;
                  to_player["players"][player_index]["balance"] = player_balance;
                }
                sleep(1); // Lag to allow GUI to load before sending (otherwise message will be ignored)
                write_msg(to_player.dump());
              }
            }
            //////////////////////////////////////////////////////
            /*                  START_GAME RESPONSE             */
            //////////////////////////////////////////////////////
            // When player0 starts the game, prepare a new game, distribute cards to all players
            // Allow player0 to make the first move (bet/fold)

            if (player_msg["event"] == "start") {
              room_.round_flag = 1; //Sets the game start flag so no new players can join as an active player


              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                // Collect the ante from each player
                room_.player_list[i].balance -= ANTE; // Ante is 1 unit ($1)
                room_.pot += ANTE;

                // Distribute cards to each player
                for (int j = 0; j < HAND_SIZE; j++) {
                  Card new_card = room_.deck.getCard();
                  room_.player_list[i].hand.push_back(new_card);
                }
              }
              // Send a connect update to all players, sends a complete list of current players
              json to_player;
              to_player["event"] = "start_response";
              // Update the pot after ante has been collected
              std::ostringstream pot_output;
              pot_output << "POT: $ " << std::fixed << std::setprecision(2) << room_.pot;
              std::string pot_balance = pot_output.str();
              to_player["pot"] = pot_balance;
              to_player["turn"] = room_.turn;
              to_player["turn_name"] = room_.player_list[room_.turn].name;

              // Each player GUI needs to update balances and hand
              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                std::string player_index = "player" + std::to_string(i);
                std::string player_uuid = room_.player_list[i].uuid;
                std::ostringstream balance_output;
                balance_output << "$ " << std::fixed << std::setprecision(2) << room_.player_list[i].balance;
                std::string player_balance = balance_output.str();
                std::vector<std::string> json_hand = convert_hand(room_.player_list[i].hand);

                to_player["players"][player_index]["uuid"] = player_uuid;
                to_player["players"][player_index]["hand"] = json_hand;
                to_player["players"][player_index]["balance"] = player_balance;
              }
              sleep(1); // Lag to allow GUI thread to load before sending
              write_msg(to_player.dump());
            }

            //////////////////////////////////////////////////////
            /*                  FOLD RESPONSE                   */
            //////////////////////////////////////////////////////
            // When a player folds, the player is out of the game
            // They can no longer have a turn, so we must ignore the player when updating the turn

            if (player_msg["event"] == "fold") {
              int get_active = room_.get_active(player_id);
              if (get_active != -1) {
                room_.player_list[get_active].fold();
              }

              json to_player;
              to_player["event"] = "fold_response";
              to_player["all_folded"] = false;
              unsigned int i = room_.turn + 1;
              while (room_.player_list[i].folded && room_.player_list[i].connected && i < room_.player_list.size()) { //Check the next valid (non-folded) turn
                ++i;
              }
              std::cout << "next valid turn: " << i << std::endl;

              unsigned int first_player_index = 0;
              while (!room_.player_list[first_player_index].connected) { // Get the first player that is connected (used for when all players have folded/game is over)
                i++;
              }
              std::cout<< "first_player: " << first_player_index << std::endl;

              /* If all non-folded players have had a turn, and we in the first betting round, then the REPLACE round will start */
              if (i == room_.player_list.size() && room_.round_flag == 1) {
                room_.turn = 0; // Then find next valid turn
                unsigned int j = room_.turn;
                while (room_.player_list[j].folded && j < room_.player_list.size()) { //Check the next valid (non-folded) turn
                  ++j;
                }
                // CASE 1: All players folded early. No winner. Allow resetting of the game
                if (j == room_.player_list.size()) {
                  to_player["folded"] = "player" + std::to_string(get_active);
                  to_player["all_folded"] = true;
                  room_.turn = first_player_index;
                }
                // CASE 2: Start replacement round normally
                else {
                  room_.turn = j;
                  room_.round_flag = 2;
                  to_player["folded"] = "player" + std::to_string(get_active);
                }
              }
              /* If all non-folded players have had a turn, and we in the the replacement round, then the SECOND-BETTING round will start */
              else if (i == room_.player_list.size() && room_.round_flag == 2) { // Then start second round
                room_.turn = 0; // Then find next valid turn
                unsigned int j = room_.turn;
                while (room_.player_list[j].folded && room_.player_list[j].connected && j < room_.player_list.size()) { //Check the next valid (non-folded) turn
                  ++j;
                }
                // All players folded early. No winner. Allow resetting of the game
                if (j == room_.player_list.size()) {
                  to_player["folded"] = "all";
                  room_.turn = first_player_index;
                }
                // Start second betting round normally
                else {
                  room_.turn = j;
                  room_.round_flag = 3;
                  to_player["folded"] = "player" + std::to_string(get_active);
                }
              }
              /* If all non-folded players have had a turn, and we are in the second betting round, then we will trigger the showdown */
              else if (i == room_.player_list.size() && room_.round_flag == 3) { // Then trigger showdown
                int winners = showdown();
				
			std::cout<<"Winner---Player---"<<winners<<room_.player_list[winners].name<<std::endl;
				
                //to_player["winner"] = "player" + std::to_string(winner);
                room_.round_flag = 4;
              }
              /* If none of the above cases apply, then the round is still the same, next player needs to have a turn */
              else { // Normal next turn event
                room_.turn = i;
                to_player["folded"] = "player" + std::to_string(get_active);
              }

              to_player["round_flag"] = room_.round_flag;
              to_player["turn"] = "player" + std::to_string(room_.turn); //player_index
              to_player["turn_name"] = room_.player_list[room_.turn].name;

              to_player["num_players"] = static_cast<int>(room_.player_list.size()); // Static cast to avoid warnings
              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                std::string player_index = "player" + std::to_string(i);
                std::string player_uuid = room_.player_list[i].uuid;
                std::string player_name = room_.player_list[i].name;
                std::ostringstream balance_output;
                balance_output << "$ " << std::fixed << std::setprecision(2) << room_.player_list[i].balance;
                std::string player_balance = balance_output.str();
                to_player["players"][player_index]["uuid"] = player_uuid;
                to_player["players"][player_index]["name"] = player_name;
                to_player["players"][player_index]["balance"] = player_balance;
              }

              sleep(1); // Lag to allow GUI thread to load before sending
              write_msg(to_player.dump());

            }

            //////////////////////////////////////////////////////
            /*                  BET RESPONSE                    */
            //////////////////////////////////////////////////////
            // When a player bets, we need to update the pot, and the balance of each player

            if (player_msg["event"] == "bet") {
              double bet_value = player_msg["bet_value"];
              int get_active = room_.get_active(player_id);
              if (get_active != -1) {
                room_.player_list[get_active].bet(bet_value);
                room_.pot += bet_value;
                room_.current_bet = bet_value;
              }

              json to_player;
              to_player["event"] = "bet_response";
              to_player["bet"] = "player" + std::to_string(get_active);
              to_player["bet_value"] = bet_value;
              std::ostringstream pot_output; // Display the new pot
              pot_output << "POT: $ " << std::fixed << std::setprecision(2) << room_.pot;
              std::string pot_balance = pot_output.str();
              to_player["pot"] = pot_balance;
              unsigned int i = room_.turn + 1;
              while (room_.player_list[i].folded && room_.player_list[i].connected && i < room_.player_list.size()) { //Check the next valid (non-folded) turn
                ++i;
              }
              std::cout << "next valid turn: " << i << std::endl;

              unsigned int first_player_index = 0;
              while (!room_.player_list[first_player_index].connected) { // Get the first player that is connected (used for when all players have folded/game is over)
                i++;
              }
              std::cout<< "first_player: " << first_player_index << std::endl;

              /* If all non-folded players have had a turn, and we in the first betting round, then the REPLACE round will start */
              if (i == room_.player_list.size() && room_.round_flag == 1) { // Then start replace round
                room_.turn = 0; // Then find next valid turn
                unsigned int j = room_.turn;
                while (room_.player_list[j].folded && j < room_.player_list.size()) { //Check the next valid (non-folded) turn
                  ++j;
                }
                room_.turn = j;
                room_.round_flag = 2;
              }
              /* If all non-folded players have had a turn, and we in the the replacement round, then the SECOND-BETTING round will start */
              else if (i == room_.player_list.size() && room_.round_flag == 2) { // Then start second round
                room_.turn = 0; // Then find next valid turn
                unsigned int j = room_.turn;
                while (room_.player_list[j].folded && room_.player_list[j].connected && j < room_.player_list.size()) { //Check the next valid (non-folded) turn
                  ++j;
                }

                room_.turn = j;
                room_.round_flag = 3;
              }
              /* If all non-folded players have had a turn, and we are in the second betting round, then we will trigger the showdown */
              else if (i == room_.player_list.size() && room_.round_flag == 3) { // Then trigger showdown, all players have bet or folded
                int winners = showdown();
				
				std::cout<<"Winner---Player---"<<winners<<room_.player_list[winners].name<<std::endl;
				
                //to_player["winner"] = "player" + std::to_string(winner);
                room_.round_flag = 4;
              }
              else { // Normal next turn event
                room_.turn = i;
              }

              to_player["round_flag"] = room_.round_flag;
              to_player["turn"] = "player" + std::to_string(room_.turn); //player_index
              to_player["turn_name"] = room_.player_list[room_.turn].name;

              to_player["num_players"] = static_cast<int>(room_.player_list.size()); // Static cast to avoid warnings
              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                std::string player_index = "player" + std::to_string(i);
                std::string player_uuid = room_.player_list[i].uuid;
                std::string player_name = room_.player_list[i].name;
                std::ostringstream balance_output;
                balance_output << "$ " << std::fixed << std::setprecision(2) << room_.player_list[i].balance;
                std::string player_balance = balance_output.str();
                to_player["players"][player_index]["uuid"] = player_uuid;
                to_player["players"][player_index]["name"] = player_name;
                to_player["players"][player_index]["balance"] = player_balance;
              }

              //sleep(1); // Lag to allow GUI thread to load before sending
              write_msg(to_player.dump());
            }
            //////////////////////////////////////////////////////
            /*                  REPLACE RESPONSE                */
            //////////////////////////////////////////////////////
            // When a player replaces their cards, we need to send the updated cards and update the GUI ToggleButtons for THAT player
            else if (player_msg["event"] == "replace") {
              int get_active = room_.get_active(player_id);
              std::string to_replace = player_msg["to_replace"];
              if (get_active != -1) {
                for (unsigned int i = 0; i < to_replace.size(); i++) {
                  int card_to_replace = stoi(to_replace.substr(i, 1));
                  Card replacement_card = room_.deck.getCard();
                  room_.player_list[get_active].hand[card_to_replace] = replacement_card;
                }
              }

              unsigned int i = room_.turn + 1;
              while (room_.player_list[i].folded && room_.player_list[i].connected && i < room_.player_list.size()) { //Check the next valid (non-folded) turn
                ++i;
              }
              std::cout << "next valid turn: " << i << std::endl;

              unsigned int first_player_index = 0;
              while (!room_.player_list[first_player_index].connected) { // Get the first player that is connected (used for when all players have folded/game is over)
                i++;
              }
              std::cout<< "first_player: " << first_player_index << std::endl;

              if (i == room_.player_list.size() && room_.round_flag == 2) { // Then start second round
                room_.turn = 0; // Then find next valid turn
                unsigned int j = room_.turn;
                while (room_.player_list[j].folded && room_.player_list[j].connected && j < room_.player_list.size()) { //Check the next valid (non-folded) turn
                  ++j;
                }

                room_.turn = j;
                room_.round_flag = 3;
              }
              else { // Normal next turn event
                room_.turn = i;
              }

              json to_player;
              to_player["event"] = "replace_response";
              to_player["turn"] = "player" + std::to_string(room_.turn); //player_index
              to_player["turn_name"] = room_.player_list[room_.turn].name;
              to_player["round_flag"] = room_.round_flag;
              to_player["replace"] = "player" + std::to_string(get_active);

              for (unsigned int i = 0; i < room_.player_list.size(); i++) {
                std::string player_index = "player" + std::to_string(i);
                std::string player_uuid = room_.player_list[i].uuid;
                std::vector<std::string> json_hand = convert_hand(room_.player_list[i].hand);

                to_player["players"][player_index]["uuid"] = player_uuid;
                to_player["players"][player_index]["hand"] = json_hand;
              }

              sleep(1); // Lag to allow GUI thread to load before sending
              write_msg(to_player.dump());

            }
            if (player_msg["event"] == "leave") { // same functionality as folding if game has started, erase player from list otherwise
              int get_active = room_.get_active(player_id);
              if (get_active != -1) {
                room_.player_list[get_active].connected = false;
              }
            }

            //room_.deliver(read_msg_);
            do_read_header();
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  void do_write()
  {
    auto self(shared_from_this());
    asio::async_write(socket_,
        asio::buffer(write_msgs_.front().data(),
          write_msgs_.front().length()),
        [this, self](std::error_code ec, std::size_t /*length*/)
        {
          if (!ec)
          {
            write_msgs_.pop_front();
            if (!write_msgs_.empty())
            {
              do_write();
            }
          }
          else
          {
            room_.leave(shared_from_this());
          }
        });
  }

  tcp::socket socket_;
  chat_room& room_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

//----------------------------------------------------------------------

class chat_server
{
public:
  chat_server(asio::io_context& io_context,
      const tcp::endpoint& endpoint)
    : acceptor_(io_context, endpoint)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    acceptor_.async_accept(
        [this](std::error_code ec, tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<chat_session>(std::move(socket), room_)->start();
          }

          do_accept();
        });
  }

  tcp::acceptor acceptor_;
  chat_room room_;
};

//----------------------------------------------------------------------

int main(int argc, char* argv[])
{
  try
  {
    if (argc < 2)
    {
      std::cerr << "Usage: chat_server <port> [<port> ...]\n";
      return 1;
    }

    asio::io_context io_context;

    std::list<chat_server> servers;
    for (int i = 1; i < argc; ++i)
    {
      tcp::endpoint endpoint(tcp::v4(), std::atoi(argv[i]));
      servers.emplace_back(io_context, endpoint);
    }

    io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }

  return 0;
}
