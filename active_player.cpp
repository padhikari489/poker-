#include <iostream>

#include "active_player.h"

using namespace std;

ActivePlayer::ActivePlayer(std::string uuid)
{
	this->uuid = uuid;
}

void ActivePlayer::set_name(std::string name)
{
	this->name = name;
}

void ActivePlayer::bet(double amount)
{
	this->balance -= amount;
}
void ActivePlayer::fold()
{
	this->folded = true;
}
/*
bool ActivePlayer::check(bool flag)
{
	//once check done by player other player
	//cannot check this bool flag will work as a indicator
	return flag;
}*/
void ActivePlayer::replace()
{


}

void ActivePlayer::get_hand()
{

	//gets hand from the dealer
}
