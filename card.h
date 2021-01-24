#include <iostream>
#include <array>
#include <algorithm>
#include <random>
#include <chrono>
#include <vector>

#ifndef _CARD_H
#define _CARD_H

#define SUITS_N 4
#define VALUE_N 13

class Card
{
public:
		std::string value;
		std::string suit;
		std::string getPlayerImage();
		std::string getOpponentImage();
		Card(std::string, std::string);
};

class Deck // Gets a shuffled deck
{
public:
	std::array<std::string, VALUE_N> value={"A","2","3","4","5","6","7","8","9","10","J","Q","K"};
	std::array<std::string, SUITS_N> suits = {"C","D","H","S"};
	std::vector<Card> cards;

	Deck();
	Card getCard();
};

#endif
