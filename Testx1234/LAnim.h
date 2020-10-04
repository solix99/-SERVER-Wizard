#pragma once

#include "LTexture.h"
#include "LTimer.h"
#include "SDL.h"
#include "string.h"

#define MAX_VARIABLE_AMMOUNT 30


class LAnim
{
public:
	LAnim();

	bool getInUse();
	void setInUse(bool);
	void renderTexture(SDL_Renderer* gRenderer, int x, int y, float ang, int animID, int animID2, bool inverse, SDL_RendererFlip flip, int renderSpeed, int renderInverseSpeed);
	bool loadAnim(SDL_Renderer* a, string file, int);
	bool loadAnimCrop(SDL_Renderer* a, string file, int rowSize, int columSize, int animSizeX, int animSizeY);

	int getAnimTimerTicks(int, int);
	void resetAnimTimer(int, int);
	void stopAnimTimer(int, int);
	void startAnimTimer(int, int);
	void setCurrentTickClient(int, int, int);
	void setCropInUse(int i, bool b);
	void renderStaticAnim(SDL_Renderer* gRenderer);
	void setIsInverseSeq(int i, int j, bool b);
	bool serverProcessProjectileAnimState(int x, int y, int i ,int j, int renderSpeed);

	bool addNewStaticAnim(int posX, int posY, bool isCrop, bool renderInverse);
	void setCropPosX(int i, int val);
	void setCropPosY(int i, int val);

	int getCropPosX(int i);
	int getCropPosY(int i);

	bool getIsInverseSeq(int i, int j);
	bool getIsInverse(bool b);
	bool getCropInUse(int i);
	bool getSeqInUse(int i, int j);

	bool getIfTimerStarted(int, int);
	int getCurrentTickClient(int, int);
	int getTickCount();

protected:

private:

	LTexture animTexture[MAX_VARIABLE_AMMOUNT];
	LTimer animTimer[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];

	int cropPosX[MAX_VARIABLE_AMMOUNT];
	int cropPosY[MAX_VARIABLE_AMMOUNT];

	int seqPosX[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];
	int seqPosY[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];

	bool isInverse[MAX_VARIABLE_AMMOUNT];
	bool isInverseSeq[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];

	int animAngle[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];

	bool p_isCrop;

	SDL_Rect animRect[MAX_VARIABLE_AMMOUNT];
	bool cropInUse[MAX_VARIABLE_AMMOUNT];
	bool seqInUse[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];
	bool inUse;
	bool p_renderInverse;
	int tickCount;
	int currentTickClient[MAX_VARIABLE_AMMOUNT][MAX_VARIABLE_AMMOUNT];

};
