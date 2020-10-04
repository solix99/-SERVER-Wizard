#include "pch.h"
#include "gMatch.h"



gMatch::gMatch()
{
	isInit = false;
	isOngoing = false;
	reqLaunch = false;
	isWaitingForPlayers = false;
	memset(playerID, -1, MAX_PLAYER_PER_MATCH * sizeof(*playerID));
	memset(isSlotInUse, false, MAX_PLAYER_PER_MATCH * sizeof(*isSlotInUse));
	memset(isPlayerDead, false, MAX_PLAYER_PER_MATCH * sizeof(*isPlayerDead));
}
void gMatch::resetMatch()
{
	isInit = false;
	isOngoing = false;
	reqLaunch = false;
	isWaitingForPlayers = false;
	memset(isSlotInUse, false, MAX_PLAYER_PER_MATCH * sizeof(*isSlotInUse));
	memset(playerID, -1, MAX_PLAYER_PER_MATCH * sizeof(*playerID));
	memset(isPlayerDead, false, MAX_PLAYER_PER_MATCH * sizeof(*isPlayerDead));
}

bool gMatch::getIsPlayerDead(int i)
{
	return isPlayerDead[i];
}
void gMatch::setIsPlayerDead(bool value, int i)
{
	isPlayerDead[i] = value;
}
bool gMatch::getIfOnGoing()
{
	return isOngoing;
}
bool gMatch::getIfWaitingForPlayer()
{
	return isWaitingForPlayers;
}
bool gMatch::getIfSlotUsed(int i)
{
	return isSlotInUse[i];
}
void gMatch::setIfIsOnGoing(bool val)
{
	isOngoing = val;
}
void gMatch::setIfIsWaitingForPlayer(bool val)
{
	isWaitingForPlayers = val;
}
void gMatch::setIfSlotUsed(int i, bool val)
{
	isSlotInUse[i] = val;
}
int gMatch::getMatchingType()
{
	return matchType;
}
void gMatch::setMatchingType(int val)
{
	matchType = val;
}
int gMatch::getPlayersMatching()
{
	int c = 0;

	for (unsigned int i = 0; i < MAX_PLAYER_PER_MATCH; i++)
	{
		if (isSlotInUse[i])
		{
			c++;
		}
	}
	return c;
}
int gMatch::getPlayerID(int i)
{
	return playerID[i];
}
void gMatch::setPlayerID(int i, int val)
{
	playerID[i] = val;
}
bool gMatch::getIfReqLaunch()
{
	return reqLaunch;
}
void gMatch::setIfReqLaunch(bool val)
{
	reqLaunch = val;
}

bool gMatch::getIfInit()
{
	return isInit;
}
void gMatch::setIfInit(bool b)
{
	isInit = b;
}
int gMatch::getMaxPlayerCount()
{
	return matchType == 0 ? 2 : 4;
}