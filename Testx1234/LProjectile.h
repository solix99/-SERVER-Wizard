#pragma once

#include <iostream>
#include <SDL.h>
#include <cmath>

class LProjectile
{
public:
	LProjectile();

	float getPosX();
	float getPosY();
	SDL_Rect getmCollider();
	SDL_Rect getBulletPosRect(int);
	void setPosX(float);
	void setPosY(float);
	void setMCWH(int, int);
	bool checkCollision(SDL_Rect a, SDL_Rect b);
	bool getSlotFree();
	void setSlotFree(bool);
	void setAngle(float);
	bool checkStatus();
	float getAngle();
	int getDestX();
	int getDestY();
	int getVelX();
	int getVelY();
	void setDestX(int);
	void setDestY(int);
	void setVelX(float);
	void setVelY(float);
	int getDMG();
	void setCollisionRectWH(int W, int H);
	SDL_Rect getCollisionRect();
	float getXCollisionOffset();
	float getYCollisionOffset();
	void setCollisionOffset(float x, float y);


protected:

private:

	float angle;
	int DEFAULT_VEL = 5;
	float mPosX, mPosY;

	int p_collisionAmmount = 10;
	SDL_Rect p_collisionRect;
	int mDestX, mDestY;
	float mVelX, mVelY;
	int p_DMG;
	float xCollisionOffset, yCollisionOffset;

	SDL_Rect mCollider;
	bool slotFree;

};
