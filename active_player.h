#include <iostream>
#include <vector>
#include "card.h"

#ifndef ACTIVE_PLAYER_H
#define ACTIVE_PLAYER_H

class ActivePlayer
{


	public:
		std::string uuid;
		std::vector<Card> hand;
		std::string name;
		bool folded = false;
		bool connected = true;
		double balance = 100.0;


		ActivePlayer(std::string);
		void set_name(std::string);
		void bet(double amount);
		void fold();
		//bool check(bool flag);
		void replace();
		void get_hand();


};



#endif
