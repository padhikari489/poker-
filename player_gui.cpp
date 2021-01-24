#include "player_client.h"
#include <string>

using json = nlohmann::json;
/* Sets up the player GUI window */
PlayerGUI::PlayerGUI(int argc, char *argv[]) {

  adj = Gtk::Adjustment::create(0.0, 0.0, 6.0, 1.0, 10, 1.0);
  Gdk::RGBA color_bg("#477148");
  Gdk::RGBA color_text("#ebdfd9");
  override_color(color_text);
  override_background_color(color_bg);
  set_title("Poker++");
  set_border_width(10);
  resize(800, 500);
  property_window_position() = Gtk::WIN_POS_CENTER;

  /* Create card display for opponents */
  vertical_box.pack_start(opponent_grid, false, true, 10);
  vertical_box.pack_start(game_info, false, true, 20);
  vertical_box.pack_start(interactable, false, true, 20);
  interactable.pack_start(card_area, false, true, 0);
  interactable.pack_start(button_area, false, true, 10);
  card_area.pack_start(my_cards, false, true, 0);
  Gtk::Label* card_pad = new Gtk::Label("");
  card_area.pack_start(*card_pad, false, true, 10);
  card_area.pack_start(player_info, false, false, 0);

  opponent_grid.attach(player1_cards, 0, 1, 1, 1);
  opponent_grid.attach(player2_cards, 1, 1, 1, 1);
  opponent_grid.attach(player3_cards, 2, 1, 1, 1);
  opponent_grid.attach(player4_cards, 3, 1, 1, 1);

  /* Player Names */
  name1.set_text("---");
  name2.set_text("---");
  name3.set_text("---");
  name4.set_text("---");
  opponent_names.push_back(&name1);
  opponent_names.push_back(&name2);
  opponent_names.push_back(&name3);
  opponent_names.push_back(&name4);
  /*
  name1.set_text("Player 1");
  name2.set_text("Player 2");
  name3.set_text("Player 3");
  name4.set_text("Player 4");
  */

  opponent_grid.attach(name1, 0, 0, 1, 1);
  opponent_grid.attach(name2, 1, 0, 1, 1);
  opponent_grid.attach(name3, 2, 0, 1, 1);
  opponent_grid.attach(name4, 3, 0, 1, 1);

  /* Player cards - set to default */
  carda_image.set("./images/bg_red.png");
  carda_image.override_background_color(color_bg);
  carda.set_image(carda_image);
  carda.set_relief(Gtk::RELIEF_NONE);
  carda.set_can_focus(false);
  carda.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_card_clicked));
  card_button_vector.push_back(&carda);

  cardb_image.set("./images/bg_red.png");
  cardb_image.override_background_color(color_bg);
  cardb.set_image(cardb_image);
  cardb.set_relief(Gtk::RELIEF_NONE);
  cardb.set_can_focus(false);
  cardb.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_card_clicked));
  card_button_vector.push_back(&cardb);

  cardc_image.set("./images/bg_red.png");
  cardc_image.override_background_color(color_bg);
  cardc.set_image(cardc_image);
  cardc.set_relief(Gtk::RELIEF_NONE);
  cardc.set_can_focus(false);
  cardc.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_card_clicked));
  card_button_vector.push_back(&cardc);

  cardd_image.set("./images/bg_red.png");
  cardd_image.override_background_color(color_bg);
  cardd.set_image(cardd_image);
  cardd.set_relief(Gtk::RELIEF_NONE);
  cardd.set_can_focus(false);
  cardd.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_card_clicked));
  card_button_vector.push_back(&cardd);

  carde_image.set("./images/bg_red.png");
  carde_image.override_background_color(color_bg);
  carde.set_image(carde_image);
  carde.set_relief(Gtk::RELIEF_NONE);
  carde.set_can_focus(false);
  carde.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_card_clicked));
  card_button_vector.push_back(&carde);

  player_cards.push_back(&carda_image);
  player_cards.push_back(&cardb_image);
  player_cards.push_back(&cardc_image);
  player_cards.push_back(&cardd_image);
  player_cards.push_back(&carde_image);

  card1a.set("./images/opponent/bg_red.png");
  card1b.set("./images/opponent/bg_red.png");
  card1c.set("./images/opponent/bg_red.png");
  card1d.set("./images/opponent/bg_red.png");
  card1e.set("./images/opponent/bg_red.png");

  card2a.set("./images/opponent/bg_red.png");
  card2b.set("./images/opponent/bg_red.png");
  card2c.set("./images/opponent/bg_red.png");
  card2d.set("./images/opponent/bg_red.png");
  card2e.set("./images/opponent/bg_red.png");

  card3a.set("./images/opponent/bg_red.png");
  card3b.set("./images/opponent/bg_red.png");
  card3c.set("./images/opponent/bg_red.png");
  card3d.set("./images/opponent/bg_red.png");
  card3e.set("./images/opponent/bg_red.png");

  card4a.set("./images/opponent/bg_red.png");
  card4b.set("./images/opponent/bg_red.png");
  card4c.set("./images/opponent/bg_red.png");
  card4d.set("./images/opponent/bg_red.png");
  card4e.set("./images/opponent/bg_red.png");

  opponent_cards.push_back(card1);
  opponent_cards.push_back(card2);
  opponent_cards.push_back(card3);
  opponent_cards.push_back(card4);

  player1_cards.pack_start(card1a);
  player1_cards.pack_start(card1b);
  player1_cards.pack_start(card1c);
  player1_cards.pack_start(card1d);
  player1_cards.pack_start(card1e);
  Gtk::Label* pad1 = new Gtk::Label("");
  player1_cards.pack_start(*pad1, false, false, 10);

  player2_cards.pack_start(card2a);
  player2_cards.pack_start(card2b);
  player2_cards.pack_start(card2c);
  player2_cards.pack_start(card2d);
  player2_cards.pack_start(card2e);
  Gtk::Label* pad2 = new Gtk::Label("");
  player2_cards.pack_start(*pad2, false, false, 10);

  player3_cards.pack_start(card3a);
  player3_cards.pack_start(card3b);
  player3_cards.pack_start(card3c);
  player3_cards.pack_start(card3d);
  player3_cards.pack_start(card3e);
  Gtk::Label* pad3 = new Gtk::Label("");
  player3_cards.pack_start(*pad3, false, false, 10);

  player4_cards.pack_start(card4a);
  player4_cards.pack_start(card4b);
  player4_cards.pack_start(card4c);
  player4_cards.pack_start(card4d);
  player4_cards.pack_start(card4e);

  /* Opponent balances */
  balance1.set_text("$ --");
  balance2.set_text("$ --");
  balance3.set_text("$ --");
  balance4.set_text("$ --");
  opponent_balances.push_back(&balance1);
  opponent_balances.push_back(&balance2);
  opponent_balances.push_back(&balance3);
  opponent_balances.push_back(&balance4);

  /*
  balance1.set_text("$ 0");
  balance2.set_text("$ 0");
  balance3.set_text("$ 0");
  balance4.set_text("$ 0");
  */

  opponent_grid.attach(balance1, 0, 2, 1, 1);
  opponent_grid.attach(balance2, 1, 2, 1, 1);
  opponent_grid.attach(balance3, 2, 2, 1, 1);
  opponent_grid.attach(balance4, 3, 2, 1, 1);

  Gtk::Label* info_pad1 = new Gtk::Label("");
  game_info.pack_start(*info_pad1, false, false, 100);

  /* Pot Display */
  pot_display.set_text("POT: $ 0.00");
  game_info.pack_start(pot_display, false, true, 40);

  /* Turn Display */
  turn_display.set_text("Waiting For Dealer"); // Placeholder
  game_info.pack_start(turn_display, false, true, 40);

  Gtk::Label* info_pad2 = new Gtk::Label("");
  game_info.pack_start(*info_pad2, false, false, 100);

  /* Add Player's cards */
  my_cards.pack_start(carda);
  my_cards.pack_start(cardb);
  my_cards.pack_start(cardc);
  my_cards.pack_start(cardd);
  my_cards.pack_start(carde);

  //check_button.set_label("Check"); //Voiding the check button, could be replaced with bet
  //check_button.set_size_request(200, 10);
  fold_button.set_label("Fold");
  fold_button.set_size_request(200, 10);
  fold_button.set_sensitive(false);
  bet_button.set_label("Check");
  bet_button.set_sensitive(false);
  replace_button.set_label("Stand Pat");
  replace_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_replace));
  replace_button.set_sensitive(false);
  //replace_button.set_sensitive(false);
  //Glib::RefPtr<Gtk::Adjustment> adj = Gtk::Adjustment::create(0.0, 0.0, 51.0, 1.0, 10, 1.0);
  //bet_slider = new Gtk::Scale(adj);
  bet_slider.set_adjustment(adj);
  bet_slider.set_digits(0);
  bet_slider.set_value_pos(Gtk::POS_BOTTOM);
  start_button.set_label("Start Game");
  start_button.set_sensitive(false);

  //button_area.pack_start(check_button, false, false, 10);
  //check_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_check));
  button_area.pack_start(fold_button, false, false, 10);
  fold_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_fold));
  button_area.pack_start(bet_button, false, false, 10);
  bet_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_bet));
  button_area.pack_start(bet_slider, false, false, 10);
  bet_slider.signal_value_changed().connect(sigc::mem_fun(*this, &PlayerGUI::on_slider));
  button_area.pack_start(replace_button, false, false, 20);

  tutorial_button.set_label("How to Play");
  player_info.pack_start(tutorial_button, false, false, 10);
  tutorial_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_tutorial));

  player_balance_label.set_text("Balance: ");
  player_info.pack_start(player_balance_label, false, false, 10);
  player_balance.set_text("$ --");
  player_info.pack_start(player_balance, false, false, 10);
  player_info.pack_start(player_name, false, false, 10);
  player_balance.property_editable() = false;
  player_info.pack_start(start_button, false, false, 10);
  start_button.set_size_request(200,10);
  start_button.signal_clicked().connect(sigc::mem_fun(*this, &PlayerGUI::on_start_game));

  add(vertical_box);
  show_all_children();
  get_name();
}

PlayerGUI::~PlayerGUI(){

}

/* Show the Five Card Draw tutorial */
void PlayerGUI::on_tutorial() {
  Gtk::MessageDialog dialog(*this, "Five Card Draw Instructions", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
  dialog.set_secondary_text("Once everyone has paid the ante or the blinds, each player receives five cards face down.\n\nA round of betting then occurs.\n\nIf more than one player remains after that first round of betting, there follows a first round of drawing.\n\nEach active player specifies how many cards he or she wishes to discard and replace with new cards from the deck.\n\nIf you are happy with your holding and do not want to draw any cards, you \"stand pat.\"\n\nOnce the drawing round is completed, there is another round of betting.\n\nAfter that if there is more than one player remaining, a showdown occurs in which the player with the best five-card poker hand wins.");
  dialog.show_all_children();
  dialog.run();
  return;
}

/* Changes the button label between Stand Pat and Replace depending on current number of cards selected */
void PlayerGUI::on_card_clicked() {
  bool selected = false;
  for (unsigned int i = 0; i < card_button_vector.size(); i++)
  {
    if (card_button_vector.at(i)->get_active())
      selected = true;
  }
  if (selected)
    replace_button.set_label("Replace");
  else
    replace_button.set_label("Stand Pat");
}

void PlayerGUI::on_replace() {
  int total_cards = 0;
  std::string to_replace = "";

  for (unsigned int i = 0; i < card_button_vector.size(); i++)
  {
    if (card_button_vector.at(i)->get_active()) {
      ++total_cards;
      to_replace += std::to_string(i); // will add cards 0-4  ex: "012"
    }
  }

  if (total_cards > 3) { // Does not allow player to replace more than 3 cards
    Gtk::MessageDialog dialog(*this, "Try Again", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK, true);
    dialog.set_secondary_text("You cannot replace more than 3 cards.");
    dialog.run();
    return;
  }
  replace_button.set_sensitive(false);
  // Create the json object to send as a literal
  json to_dealer;
  to_dealer["from"] = uuid_string;
  to_dealer["event"] = "replace";
  to_dealer["to_replace"] = to_replace;
  write_msg(to_dealer.dump());
}

void PlayerGUI::on_bet() {
  bet_button.set_sensitive(false);
  fold_button.set_sensitive(false);
  double slider_value = bet_slider.get_value();
  json to_dealer;
  to_dealer["from"] = uuid_string;
  to_dealer["event"] = "bet";
  to_dealer["bet_value"] = slider_value;
  write_msg(to_dealer.dump());
}

void PlayerGUI::on_fold() {
  fold_button.set_sensitive(false);
  bet_button.set_sensitive(false);
  json to_dealer;
  to_dealer["from"] = uuid_string;
  to_dealer["event"] = "fold";
  write_msg(to_dealer.dump());
}

/* // Deprecated
void PlayerGUI::on_check() {
  json to_dealer;
  to_dealer["from"] = uuid_string;
  to_dealer["event"] = "check";
  write_msg(to_dealer.dump());
}*/
