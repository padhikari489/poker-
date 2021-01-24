#include <iostream>
#include "card.h"

using namespace std;

Card::Card(std::string value, std::string suit) {
  this -> value = value;
  this -> suit = suit;
}

Deck::Deck () {
  for (int i = 0; i < VALUE_N; i++) {
    for (int j = 0; j < SUITS_N; j++) {
      Card new_card(value[i], suits[j]);
      cards.push_back(new_card);
    }
  }
  unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
  shuffle(cards.begin(), cards.end(), std::default_random_engine(seed));
}

Card Deck::getCard() {
  Card result(cards.back().value, cards.back().suit); //Create a clone of the card in back
  cards.pop_back(); //Remove the back, this destroys the card
  return result;
}
