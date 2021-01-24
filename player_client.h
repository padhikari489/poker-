// Much code copied from:
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef PLAYER_CLIENT_H
#define PLAYER_CLIENT_H

#include <gtkmm.h>
#include <string>
#include <deque>
#include <array>
#include <thread>
#include <iostream>
#include <cstring>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include "chat_message.hpp"
#include "asio.hpp"
#include "json.hpp"
#include <mutex>

#define HAND_SIZE 5
#define MAX_IP_PACK_SIZE 512
#define MAX_PLAYERNAME 16
#define PADDING 24

using asio::ip::tcp;
using json = nlohmann::json;

// Global UUID
extern std::string uuid_string;

typedef std::deque<chat_message> chat_message_queue;

/* Class for the Player GUI window */
class PlayerGUI: public Gtk::Window {
public:
  PlayerGUI(int argc, char *argv[]);
  virtual ~PlayerGUI();

  /* Box for player to enter name once program starts*/
  Gtk::Entry name_entry;

  /* Containers to set up GUI */
  Gtk::VBox vertical_box;
  Gtk::Grid opponent_grid;
  Gtk::Box player1_cards;
  Gtk::Box player2_cards;
  Gtk::Box player3_cards;
  Gtk::Box player4_cards;
  Gtk::Box game_info;
  Gtk::Box interactable;
  Gtk::VBox card_area;
  Gtk::Box my_cards;
  Gtk::VBox button_area;
  Gtk::Box player_info;

  /* These are the player's cards (the player of this client) */
  std::vector<Gtk::Image*> player_cards;
  Gtk::Image carda_image;
  Gtk::Image cardb_image;
  Gtk::Image cardc_image;
  Gtk::Image cardd_image;
  Gtk::Image carde_image;

  /* Toggle buttons for player's cards, for replacing cards */
  Gtk::ToggleButton carda;
  Gtk::ToggleButton cardb;
  Gtk::ToggleButton cardc;
  Gtk::ToggleButton cardd;
  Gtk::ToggleButton carde;
  std::vector<Gtk::ToggleButton*> card_button_vector; // Stores all the ToggleButtons into a vector

  /* Opponent 1's cards */
  Gtk::Image card1a;
  Gtk::Image card1b;
  Gtk::Image card1c;
  Gtk::Image card1d;
  Gtk::Image card1e;
  std::vector<Gtk::Image*> card1 = {&card1a, &card1b, &card1c, &card1d, &card1e};

  /* Opponent 2's cards */
  Gtk::Image card2a;
  Gtk::Image card2b;
  Gtk::Image card2c;
  Gtk::Image card2d;
  Gtk::Image card2e;
  std::vector<Gtk::Image*> card2 = {&card2a, &card2b, &card2c, &card2d, &card2e};

  /* Opponent 3's cards */
  Gtk::Image card3a;
  Gtk::Image card3b;
  Gtk::Image card3c;
  Gtk::Image card3d;
  Gtk::Image card3e;
  std::vector<Gtk::Image*> card3 = {&card3a, &card3b, &card3c, &card3d, &card3e};

  /* Opponent 4's cards */
  Gtk::Image card4a;
  Gtk::Image card4b;
  Gtk::Image card4c;
  Gtk::Image card4d;
  Gtk::Image card4e;
  std::vector<Gtk::Image*> card4 = {&card4a, &card4b, &card4c, &card4d, &card4e};

  std::vector<std::vector<Gtk::Image*>> opponent_cards; // Vector that contains vectors of opponents cards -- to use, gui_window->opponent_cards[index of opponent][index of card]

  /* Names and balances of opponents */
  std::vector<Gtk::Label*> opponent_names;
  Gtk::Label name1;
  Gtk::Label name2;
  Gtk::Label name3;
  Gtk::Label name4;

  std::vector<Gtk::Label*> opponent_balances;
  Gtk::Label balance1;
  Gtk::Label balance2;
  Gtk::Label balance3;
  Gtk::Label balance4;

  /* Displays current pot */
  Gtk::Label pot_display;

  /* Displays current turn (Player 1's Turn) */
  Gtk::Label turn_display;

  /* Player interactables */
  Gtk::Button check_button;
  Gtk::Button fold_button;
  Gtk::Button bet_button;
  Glib::RefPtr<Gtk::Adjustment> adj; // This controls the range of the bet slider
  Gtk::Scale bet_slider;

  Gtk::Button tutorial_button;
  Gtk::Label player_balance_label;
  Gtk::Entry player_balance;
  Gtk::Label player_name;
  Gtk::Button start_button;
  Gtk::Button replace_button;

  std::array<char, MAX_IP_PACK_SIZE> msg;

  void on_start_game(); // Start Game button
  void get_name(); //Getting the players name at startup
  void on_tutorial(); // Five Card Draw tutorial
  void on_card_clicked(); // When a togglebutton is clicked, changes the replace button to match the selection
  void on_slider(); // When the slider value is changed, changes the bet button to match
  void on_replace(); // Sends a message to replace the selected cards
  void on_bet(); // Sends a message to bet the current slider value
  void on_fold(); // Sends a message to fold (effectively drop out of game)

  void write_msg(std::string); // Used to send messages between GUI and dealer

};

// This class from https://www.boost.org/doc/libs/1_48_0/doc/html/boost_asio/example/chat/chat_client.cpp
class client
{
public:
    PlayerGUI *gui_window = NULL;
    std::string nickname;
    int current_num_players = 0;
    int min_bet = 0;
    bool betting_round = false;
    std::mutex mtx;

    client(asio::io_context& io_context, const tcp::resolver::results_type& endpoints) : io_context_(io_context), socket_(io_context)
    {
      do_connect(endpoints);
    }

    // Function to get the correct card filename (png) in order to change a card image -- bool is_opponent (true for the small card images, false for the large card images)
    std::string get_filename(std::string card_name, bool is_opponent) {
      std::string path = "./images/";
      if (is_opponent)
        path += "opponent/";
      return path + card_name + ".png";
    }

    void write(const chat_message& msg)
    {
      asio::post(io_context_, [this, msg]()
        {
          bool write_in_progress = !write_msgs_.empty();
          write_msgs_.push_back(msg);
          if (!write_in_progress)
          {
            do_write();
          }
        });
    }

    void close()
    {
      asio::post(io_context_, [this]() { socket_.close(); });
    }

     void do_connect(const tcp::resolver::results_type& endpoints)
     {
       asio::async_connect(socket_, endpoints,
       [this](std::error_code ec, tcp::endpoint)
       {
         if (!ec)
         {
           do_read_header();
         }
       });
     }

     void do_read_header()
     {
       asio::async_read(socket_, asio::buffer(read_msg_.data(), chat_message::header_length), [this](std::error_code ec, std::size_t /*length*/)
       {
         if (!ec && read_msg_.decode_header())
         {
           do_read_body();
         }
         else
         {
           socket_.close();
         }
       });
     }

     ////////////////////////////////////////////
     /*         MESSAGES FROM DEALER           */
     /*       ARE PARSED IN THIS FUNCTION      */
     ////////////////////////////////////////////

     void do_read_body()
     {
       asio::async_read(socket_, asio::buffer(read_msg_.body(), read_msg_.body_length()), [this](std::error_code ec, std::size_t /*length*/)
       {
         if (!ec)
         {
           {
             /* Code for player interpreting messages */
             if (gui_window != NULL) { // Check to see if the gui window has been set up
               std::string msg_string(read_msg_.body());
               msg_string = msg_string.substr(0, read_msg_.body_length());

               /*Change message to JSON object*/
               json dealer_msg = json::parse(msg_string);

               //////////////////////////////////////////////////////
               /*                  CONNECT RESPONSE                */
               //////////////////////////////////////////////////////

               // When a new player connects, dealer send msg to GUI to show all connected players
               if (dealer_msg["event"] == "connect_response" || dealer_msg["event"] == "name_response") {
                 int num_players = dealer_msg["num_players"];
                 current_num_players = num_players;
                 int player_name_read = 0; // Index for whether or not this client's uuid has been read

                 for (int i = 0; i < num_players; i++) {
                   std::string player_index = "player" + std::to_string(i);

                   //If the current player's uuid matches the client's uuid, this is the client, change this GUI's player_name and player_balance
                   if (dealer_msg["players"][player_index]["uuid"] == uuid_string) {
                     player_name_read = 1;
                     std::string player_name = dealer_msg["players"][player_index]["name"];
                     std::string player_balance = dealer_msg["players"][player_index]["balance"];
                     mtx.lock();
                     gui_window->player_balance.set_text(player_balance);
                     mtx.unlock();

                     mtx.lock();
                     gui_window->player_name.set_text(player_name + " (" + std::to_string(i+1) + ")");
                     mtx.unlock();
                     if (i == 0) { // If this client is also the first to connect
                       mtx.lock();
                       gui_window->start_button.set_sensitive(true);
                       mtx.unlock();
                     }
                   }
                   // Otherwise, the player_index is an opponent client, update the opponent's info
                   else {
                     std::string opponent_name = dealer_msg["players"][player_index]["name"];
                     std::string opponent_balance = dealer_msg["players"][player_index]["balance"];
                     mtx.lock();
                     //[i - player_name_read] This index ensures that the opponent can only be in slots 1-4 in the opponent section
                     gui_window->opponent_names[i - player_name_read]->set_text(opponent_name + " (" + std::to_string(i+1) + ")");
                     mtx.unlock();

                     mtx.lock();
                     gui_window->opponent_balances[i - player_name_read]->set_text(opponent_balance);
                     mtx.unlock();
                   }
                 }

               }

               //////////////////////////////////////////////////////
               /*               START GAME RESPONSE                */
               //////////////////////////////////////////////////////

                // Signals the first turn in the game, allows the first connected player to start their turn
               else if (dealer_msg["event"] == "start_response") {

                 std::string pot_balance = dealer_msg["pot"];
                 mtx.lock();
                 gui_window->pot_display.set_text(pot_balance);
                 mtx.unlock();
                 std::string first_turn = dealer_msg["turn_name"];
                 first_turn += "'s turn";
                 mtx.lock();
                 gui_window->turn_display.set_text(first_turn);
                 mtx.unlock();
                 for (int i = 0; i < current_num_players; i++) {
                   std::string player_index = "player" + std::to_string(i);
                   //If the current player's uuid matches the client's uuid
                   if (dealer_msg["players"][player_index]["uuid"] == uuid_string) {

                     for (unsigned int j = 0; j < gui_window->player_cards.size(); j++) {
                       std::this_thread::sleep_for (std::chrono::milliseconds(500));
                       mtx.lock();
                       gui_window->player_cards[j]->set(get_filename(dealer_msg["players"][player_index]["hand"].at(j), false));
                       mtx.unlock();
                     }
                     std::this_thread::sleep_for (std::chrono::milliseconds(500));
                     // Update balances after the ante is taken
                     std::string player_balance = dealer_msg["players"][player_index]["balance"];
                     mtx.lock();
                     gui_window->player_balance.set_text(player_balance);
                     mtx.unlock();

                     mtx.lock();
                     gui_window->card_button_vector[0]->set_active(true); //This refreshes the first togglebutton, which hides the image sometimes
                     mtx.unlock();

                     mtx.lock();
                     gui_window->card_button_vector[0]->set_active(false);
                     mtx.unlock();
                   }
                 }
                 if (dealer_msg["players"]["player0"]["uuid"] == uuid_string) { // if this client is the first client
                   mtx.lock();
                   gui_window->fold_button.set_sensitive(true); // Allow the first player to make a move (bet/fold)
                   mtx.unlock();
                   mtx.lock();
                   gui_window->bet_button.set_sensitive(true);
                   mtx.unlock();
                 }
               }
               //////////////////////////////////////////////////////
               /*                  FOLD RESPONSE                   */
               //////////////////////////////////////////////////////
               /* Signaled when one of the players has folded */

               else if (dealer_msg["event"] == "fold_response") {
                 bool my_turn = false;
                 int player_name_read = 0;
                 for (int i = 0; i < current_num_players; i++) {
                   std::string player_index = "player" + std::to_string(i);
                   if (dealer_msg["players"][player_index]["uuid"] == uuid_string) {//If the current player's uuid matches the client's uuid and it is their turn
                     player_name_read = 1;

                     // If it is this client's turn, allow the appropriate actions
                     if (dealer_msg["turn"] == player_index) {
                       my_turn = true;
                       // If it is a betting round, this client can now fold or bet
                       if (dealer_msg["round_flag"] == 1 || dealer_msg["round_flag"] == 3) {
                         mtx.lock();
                         gui_window->fold_button.set_sensitive(true);
                         mtx.unlock();
                         mtx.lock();
                         gui_window->bet_button.set_sensitive(true);
                         mtx.unlock();
                       }
                       // Otherwise, start the replace cards round
                       else if (dealer_msg["round_flag"] == 2) {
                         mtx.lock();
                         gui_window->replace_button.set_sensitive(true);
                         mtx.unlock();
                       }
                     }
                   }
                   // If the folded player is an opponent, show that the opponent has folded
                   // Notice that we ignore the case if this client has folded, because no action is needed, they can watch the rest of the game
                   else if (dealer_msg["folded"] == player_index) {
                     mtx.lock();
                     std::string opponent_name = gui_window->opponent_names[i - player_name_read]->get_text();
                     mtx.unlock();

                     mtx.lock();
                     // Adds [FOLDED] text to opponent's name
                     gui_window->opponent_names[i - player_name_read]->set_text(opponent_name + " [FOLDED]");
                     mtx.unlock();
                     for (unsigned int j = 0; j < gui_window->opponent_cards[i-player_name_read].size(); j++) {
                       //Set's the opponents cards to black, showing they are out of the game
                       mtx.lock();
                       gui_window->opponent_cards[i-player_name_read][j]->set("./images/opponent/bg_black.png");
                       mtx.unlock();
                     }
                   }
                 }
                 // In the case that all players have folded, the game is over, no one wins
                 if (dealer_msg["all_folded"] == true) {
                   std::string turn = "All players folded.";
                   mtx.lock();
                   gui_window->fold_button.set_sensitive(false);
                   mtx.unlock();

                   mtx.lock();
                   gui_window->bet_button.set_sensitive(false);
                   mtx.unlock();

                   mtx.lock();
                   gui_window->turn_display.set_text(turn);
                   mtx.unlock();
                   if (my_turn) {
                     mtx.lock();
                     gui_window->start_button.set_sensitive(true);
                     mtx.unlock();
                   }
                 }
                 // Otherwise, show who's turn it is
                 else {
                   std::string turn = dealer_msg["turn_name"];
                   turn += "'s turn";
                   mtx.lock();
                   gui_window->turn_display.set_text(turn);
                   mtx.unlock();
                 }
               }
               //////////////////////////////////////////////////////
               /*                  BET RESPONSE                    */
               //////////////////////////////////////////////////////

               // When a player sends a bet signal, we need to update the balance and the pot
               // (This also covers CHECK, CALL, AND RAISE)
               else if (dealer_msg["event"] == "bet_response") {
                 double current_bet = dealer_msg["bet_value"];
                 min_bet = current_bet;
                 std::string turn = dealer_msg["turn_name"];
                 turn += "'s turn";

                 // Show the current
                 std::string pot_balance = dealer_msg["pot"];
                 mtx.lock();
                 gui_window->turn_display.set_text(turn);
                 mtx.unlock();

                 // Update the pot
                 mtx.lock();
                 gui_window->pot_display.set_text(pot_balance);
                 mtx.unlock();

                 // Update the bet slider to reflect the new minimum bet
                 mtx.lock();
                 gui_window->adj->set_lower(current_bet);
                 mtx.unlock();

                 mtx.lock();
                 gui_window->adj->set_upper(current_bet + 5 + 1);
                 mtx.unlock();

                 // Refreshes the bet slider current value (otherwise it will stay at 0 or the last bet)
                 mtx.lock();
                 gui_window->bet_slider.set_value(current_bet);
                 mtx.unlock();

                 int player_name_read = 0; // This flag shows whether this client has already been considered in the for loop

                 for (int i = 0; i < current_num_players; i++) {
                   std::string player_index = "player" + std::to_string(i);
                   // If this client is the one that bet, update the player balance
                   if (dealer_msg["players"][player_index]["uuid"] == uuid_string) {
                     player_name_read = 1;
                     if (dealer_msg["bet"] == player_index) { // Update the client's balance
                       std::string player_balance = dealer_msg["players"][player_index]["balance"];
                       mtx.lock();
                       gui_window->player_balance.set_text(player_balance);
                       mtx.unlock();
                     }
                     // If it is this client's turn, allow the appropriate actions
                     if (dealer_msg["turn"] == player_index) {
                       if (dealer_msg["round_flag"] == 1 || dealer_msg["round_flag"] == 3) { // Betting round
                         mtx.lock();
                         gui_window->fold_button.set_sensitive(true);
                         mtx.unlock();
                         mtx.lock();
                         gui_window->bet_button.set_sensitive(true);
                         mtx.unlock();
                       }
                       else if (dealer_msg["round_flag"] == 2) { // Replace cards round
                         mtx.lock();
                         gui_window->replace_button.set_sensitive(true);
                         mtx.unlock();
                       }
                     }
                   }
                   // If another player bet, then update that opponent's balance
                   else if (dealer_msg["bet"] == player_index) { // If it is not this client, update the balance of the opponent client
                     std::string opponent_balance = dealer_msg["players"][player_index]["balance"];
                     mtx.lock();
                     gui_window->opponent_balances[i - player_name_read]->set_text(opponent_balance);
                     mtx.unlock();
                   }
                 }
               }

               //////////////////////////////////////////////////////
               /*                  REPLACE RESPONSE                */
               //////////////////////////////////////////////////////

               // Players will replace cards one at a time
               // When a player signals to REPLACE their cards, this response will change the cards of the player that signaled REPLACE
               else if (dealer_msg["event"] == "replace_response") {
                 // Update the current turn
                 std::string turn = dealer_msg["turn_name"];
                 turn += "'s turn";
                 mtx.lock();
                 gui_window->turn_display.set_text(turn);
                 mtx.unlock();

                 for (int i = 0; i < current_num_players; i++) {
                   std::string player_index = "player" + std::to_string(i);
                   if (dealer_msg["players"][player_index]["uuid"] == uuid_string) {
                     // Replace the client's cards if this client signaled REPLACE
                     if (dealer_msg["replace"] == player_index) {

                       for (unsigned int j = 0; j < gui_window->player_cards.size(); j++) {
                         std::this_thread::sleep_for (std::chrono::milliseconds(500));
                         mtx.lock();
                         gui_window->player_cards[j]->set(get_filename(dealer_msg["players"][player_index]["hand"].at(j), false));
                         //This refreshes the first togglebutton, which hides the image sometimes
                         //gui_window->card_button_vector[0]->set_active(true);
                         //gui_window->card_button_vector[0]->set_active(false);
                         mtx.unlock();
                       }
                       std::this_thread::sleep_for (std::chrono::milliseconds(500));
                       mtx.lock();
                       gui_window->card_button_vector[0]->set_active(true); //This refreshes the first togglebutton, which hides the image sometimes
                       mtx.unlock();

                       mtx.lock();
                       gui_window->card_button_vector[0]->set_active(false);
                       mtx.unlock();
                       /*
                       for (unsigned int j = 0; j < gui_window->player_cards.size(); j++) {
                         mtx.lock();
                         gui_window->card_button_vector[0]->set_active(true);
                         mtx.unlock();
                       }

                       for (unsigned int j = 0; j < gui_window->player_cards.size(); j++) {
                         mtx.lock();
                         gui_window->card_button_vector[0]->set_active(false);
                         mtx.unlock();
                       }
                       */

                     }
                     // If it is this client's turn, allow the appropriate actions
                     if (dealer_msg["turn"] == player_index) {
                       if (dealer_msg["round_flag"] == 1 || dealer_msg["round_flag"] == 3) { // Betting round
                         mtx.lock();
                         gui_window->fold_button.set_sensitive(true);
                         mtx.unlock();
                         mtx.lock();
                         gui_window->bet_button.set_sensitive(true);
                         mtx.unlock();
                       }
                       else if (dealer_msg["round_flag"] == 2) { // Replace cards round
                         mtx.lock();
                         gui_window->replace_button.set_sensitive(true);
                         mtx.unlock();
                       }
                     }
                   }
                 }
               } // Replace reponse
             } // Check that GUI is not NULL
           } // Message parsing code block

           {
              char outline[read_msg_.body_length() + 2];
                                      // '\n' + '\0' is 2 more chars
              outline[0] = '\n';
              outline[read_msg_.body_length() + 1] = '\0';
              std::memcpy ( &outline[1], read_msg_.body(), read_msg_.body_length() );
           }

           std::cout.write(read_msg_.body(), read_msg_.body_length());
           std::cout << "\n";
           do_read_header();
         } // if (!ec)
         else
         {
           socket_.close();
         }
       });
     }

     void do_write()
     {
       asio::async_write(socket_, asio::buffer(write_msgs_.front().data(), write_msgs_.front().length()), [this](std::error_code ec, std::size_t /*length*/)
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
           socket_.close();
         }
       });
     }

private:
  asio::io_context& io_context_;
  tcp::socket socket_;
  chat_message read_msg_;
  chat_message_queue write_msgs_;
};

#endif
