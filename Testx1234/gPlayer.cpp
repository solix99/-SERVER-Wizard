#include "pch.h"
#include "gPlayer.h"

gPlayer::gPlayer()
{
	requireInit = false;
	slotUsed = false;
	isPlaying = false;
	isDead = false;
	ID = "-1";
	p_health = 100;
}

void gPlayer::reset()
{
	requireInit = false;
	slotUsed = false;
	isPlaying = false;
	isDead = false;
	ID = "-1";
	p_health = 100;
}

void gPlayer::setPlayerPosX(int posX)
{
	mCollider.x = posX;
	p_collisionRect.x = posX + xCollisionOffset;

}
void gPlayer::setPlayerPosY(int posY)
{
	mCollider.y = posY;
	p_collisionRect.y = posY + yCollisionOffset;
}
SDL_Rect gPlayer::getmCollider()
{
	return mCollider;
}
bool gPlayer::getSlotUsed()
{
	return slotUsed;
}
void gPlayer::setSlotUsed(bool b)
{
	slotUsed = b;
}
void gPlayer::setCollisionRectWH(int W, int H)
{
	p_collisionRect.w = W;
	p_collisionRect.h = H;
}
void gPlayer::setCollisionOffset(int x, int y)
{
	xCollisionOffset = x;
	yCollisionOffset = y;
}
string  gPlayer::getNickname()
{
	return nickname;
}
string  gPlayer::getID()
{
	return ID;
}
void gPlayer::setNickname(string s)
{
	nickname = s;
}
SDL_Rect gPlayer::getCollisionRect()
{
	return p_collisionRect;
}
void gPlayer::setID(string s)
{
	ID = s;
}
int gPlayer::getClientSocketID()
{
	return clientSocketID;
}
void gPlayer::setClientSocketID(int i)
{
	clientSocketID = i;
}

string gPlayer::getFlipType()
{
	return flipType;
}
string gPlayer::getAnimType()
{
	return animType;
}

void gPlayer::setFlipType(string f)
{
	flipType = f;
}
void gPlayer::setAnimType(string at)
{
	animType = at;
}
void gPlayer::setTextureWH(int W, int H)
{
	mCollider.w = W;
	mCollider.h = H;
}
bool gPlayer::getRequireInit()
{
	return requireInit;
}
void gPlayer::setRequireInit(bool b)
{
	requireInit = b;
}
int gPlayer::getHealth()
{
	return p_health;
}
void gPlayer::setHealth(int h)
{
	p_health = h;
}
void gPlayer::damageTarget(int d)
{
	p_health -= d;
}
void gPlayer::setPlayerDead(bool b)
{
	isDead = b;
}
bool gPlayer::getPlayerDead()
{
	return isDead;
}

void gPlayer::setmID(int ID)
{
	mID = ID;
}
int gPlayer::getmID()
{
	return mID;
}

void gPlayer::setIfPlaying(bool b)
{
	isPlaying = b;
}
bool gPlayer::getIfPlaying()
{
	return isPlaying;
}