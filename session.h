#include <iostream>

#include "active_player.h"
#ifndef SESSION_H
#define SESSION_H

class Session
{
	public:
		Session()   ;    //Constructor
		ActivePlayer p;

		void enter(ActivePlayer p);
		void leave(ActivePlayer p);
		void broadcast(ActivePlayer p);
		void encode_json();
		double getPotAmount(double pot);
		int Turn(int PId);
	private:
		double pot;
		int PId;

};








#endif
