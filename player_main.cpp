// Much code copied from:
//
// Copyright (c) 2003-2019 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "player_client.h"
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <array>
#include <thread>
#include <cstring>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <chrono>

//global signals
client *c;
std::string uuid_string;

using namespace std;
using json = nlohmann::json;
void write_msg(std::string);
using asio::ip::tcp;

////////////////////////////////////////////
/*         GUI FUNCTIONS THAT NEED        */
/*      ACCESS TO CLIENT ARE PLACED IN    */
/*              THIS MAIN FILE            */
////////////////////////////////////////////

void PlayerGUI::on_start_game() {
  /* A game needs at least 2 players to begin */
  if (c->current_num_players < 2) {
    Gtk::MessageDialog dialog(*this, "Error: Could not start game",false,Gtk::MESSAGE_ERROR);
    dialog.set_secondary_text("At least 2 players are needed to begin the game.");
    dialog.run();
    return;
  }
  json to_dealer;
  to_dealer["from"] = uuid_string;
  to_dealer["event"] = "start";
  write_msg(to_dealer.dump());
  start_button.set_label("Restart Game");
  start_button.set_sensitive(false);
}

void PlayerGUI::on_slider() {
  double current_value = bet_slider.get_value();
  int min_bet = c->min_bet;
  if (min_bet == 0) {
    if (current_value == 0) {
      bet_button.set_label("Check");
    }
    else if (current_value >= min_bet) {
      bet_button.set_label("Bet");
    }
  }
  else {
    if (current_value == min_bet) {
      bet_button.set_label("Call");
    }
    else {
      bet_button.set_label("Raise");
    }
  }

}

/* This GUI function writes a message and sends it through the client */
void PlayerGUI::write_msg(std::string msg_string) {
  chat_message msg;
  int msg_length = msg_string.length();
  char msg_array[msg_length];
  strcpy(msg_array, msg_string.c_str());
  strcpy(msg.data(), msg_array);
  msg.body_length ( strlen(msg_array) );
  std::memcpy(msg.body(), msg_array, msg.body_length());
  msg.encode_header();
  assert ( c );  // this is a global class
  c->write(msg);
}

void PlayerGUI::get_name() {
  Gtk::Dialog *dialog = new Gtk::Dialog("Poker++", *this, false);
  Gtk::Label *name_label = new Gtk::Label("Please enter your name:");
  dialog->get_content_area()->pack_start(*name_label);
  dialog->get_content_area()->pack_start(name_entry);
  dialog->add_button("Enter", 0);
  dialog->signal_response().connect([&](int response_id) {
    if (response_id == 0) {
      c->nickname = name_entry.get_text();
      //player_name.set_text(c->nickname);
      json to_dealer;
      to_dealer["from"] = uuid_string;
      to_dealer["event"] = "name_entry";
      to_dealer["name"] = c->nickname;
      write_msg(to_dealer.dump());
      dialog->close();
    }
  });
  dialog->show_all_children();
  dialog->run();
}

//////////////////////////////////////////////////////
/*                  MAIN FUNCTION                   */
//////////////////////////////////////////////////////

// Starts the client
// Starts the GUI
// Sends a CONNECT message to the dealer

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: player <host-ip> <port>\n";
    return 1;
  }

  /* Create a connection to the server */
  asio::io_context io_context;

  tcp::resolver resolver(io_context);
  auto endpoints = resolver.resolve(argv[1], argv[2]);
  c = new client(io_context, endpoints);
  assert(c);
  std::thread t([&io_context](){ io_context.run(); });

  /* Tests the UUID function */
  boost::uuids::random_generator gen;
  boost::uuids::uuid u = gen();

  chat_message msg;
  uuid_string = to_string(u);

  json to_dealer;
  to_dealer["from"] = to_string(u);
  to_dealer["event"] = "connect";

  std::string msg_string = to_dealer.dump();

  int msg_length = msg_string.length();
  char msg_array[msg_length];
  strcpy(msg_array, msg_string.c_str());
  strcpy(msg.data(), msg_array);
  msg.body_length ( strlen(msg_array) );
  std::memcpy(msg.body(), msg_array, msg.body_length());
  msg.encode_header();
  assert ( c );  // this is a global class
  c->write(msg);

  /* Run the GUI */
  Gtk::Main app(argc, argv);
  c->gui_window = new PlayerGUI(argc, argv);
  Gtk::Main::run(*(c->gui_window));

  /* Close the client and join the thread */
  json to_dealer_exit;
  to_dealer_exit["from"] = uuid_string;
  to_dealer_exit["event"] = "leave";
  c->gui_window->write_msg(to_dealer_exit.dump());
  sleep(3); // Wait to send the leave message, otherwise message will not be sent at all
  c->close();
  t.join();
  return 0;
}
