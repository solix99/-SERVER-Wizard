#pragma once

#include "gPlayer.h"

#define MAX_PLAYER_PER_MATCH 4

class gMatch
{
public:
	gMatch();

	bool getIfOnGoing();
	bool getIfWaitingForPlayer();
	bool getIfSlotUsed(int i);
	int getMatchingType();
	int getPlayersMatching();
	int getPlayerID(int i);
	bool getIfReqLaunch();
	bool getIfInit();
	void setIfInit(bool b);
	void setIfReqLaunch(bool val);
	void setPlayerID(int i, int val);
	void setMatchingType(int val);
	void setIfIsOnGoing(bool val);
	void setIfIsWaitingForPlayer(bool val);
	void setIfSlotUsed(int i,bool val);
	void resetMatch();
	int getMaxPlayerCount();
	bool getIsPlayerDead(int i);
	void setIsPlayerDead(bool value, int i);

private:

	int matchID;
	int matchType;
	bool isInit;
	bool isOngoing;
	bool isWaitingForPlayers;
	bool reqLaunch;
	bool isSlotInUse[MAX_PLAYER_PER_MATCH];
	int playerID[MAX_PLAYER_PER_MATCH];
	bool isPlayerDead[MAX_PLAYER_PER_MATCH];
};

