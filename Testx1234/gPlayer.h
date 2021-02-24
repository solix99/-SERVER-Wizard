#pragma once

#include "SDL.h"
#include <string>
#include "LProjectile.h"

using namespace std;

class gPlayer
{

public:
	gPlayer();
	void setPlayerPosX(int);
	void setPlayerPosY(int);
	SDL_Rect getmCollider();
	bool getSlotUsed();
	void setSlotUsed(bool);

	string getNickname();
	string getID();
	string getFlipType();
	string getAnimType();

	void setFlipType(string);
	void setAnimType(string);

	bool getRequireInit();
	void setRequireInit(bool b);
	int getHealth();
	void setHealth(int);
	void setPlayerDead(bool b);
	bool getPlayerDead();
	void setIfPlaying(bool b);
	bool getIfPlaying();

	void reset();
	void setmID(int mID);
	int getmID();
	void damageTarget(int);
	void setNickname(string);
	void setID(string);
	int getClientSocketID();
	void setClientSocketID(int);
	void setTextureWH(int, int);
	void setCollisionRectWH(int W, int H);
	void setCollisionOffset(int x,int y);
	SDL_Rect getCollisionRect();

	LProjectile gProjectile[30];

private:

	int p_collisionAmmount = 10;
	SDL_Rect p_collisionRect;
	SDL_Rect mCollider;
	bool slotUsed;
	string nickname;
	string flipType, animType;
	string ID;
	bool requireInit;
	int clientSocketID;
	int p_health;
	int xCollisionOffset, yCollisionOffset;
	bool isDead;
	bool isPlaying;
	int mID;
};

