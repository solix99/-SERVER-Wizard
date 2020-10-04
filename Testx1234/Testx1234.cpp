#include "pch.h"
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <Windows.h>
#include "SDL.h"
#include <SDL_thread.h>
#include "gPlayer.h"
#include <sstream>
#include "Ltexture.h"
#include <time.h>
#include "LAnim.h"
#include "SDL_ttf.h"
#include "LWindow.h"
#include "gMatch.h"

using namespace std;

WSADATA wData;
#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 10000
#define MAX_PLAYER_ENTITY 99
#define MAX_PLAYER_BULLET_COUNT 28
#define SDL_GLOBAL_DELAY 10
#define SDL_TEMP_DELAY 50
#define MAX_MATCHES 24
#define MAX_PLAYER_ENTITY 99

struct engineParameters
{
	struct ANIMATION
	{
		int FIREBALL_RENDERSPEED = 1000;

	}ANIM;

	struct GAMESYSTEM
	{
		LTimer physicsTimerProjectile;
		LTimer physicsTimerMovement;
		LTimer sendFpsTimer;
		LTimer recvFpsTimer;

		SDL_Event e;
		int physicsRate = 20;
		float projSpeed = 50.0f;

	}GSYS;

	struct BOOLEAN
	{
		bool sendRemovePacket = false;
		bool addClient = false;
		bool playerDamage = false;
		bool updateMatching = false;
		bool matchOver = false;
		bool updateMatch = false;
		bool isSendThreadActive = false;
		bool isPhysicsThreadActive = false;
		bool isReciveThreadActive = false;

	}EXECUTE;

	struct TEMPORAL
	{
		int projectileX;
		int projectileY;
		int	projectileDX;
		int	projectileDY;
		int damageGiverID;
		int damageTargetID;
		int damageAmount;
		int sendFrame;	
		bool killShot;
		int recvFrame;
		int relativeID;
		int iMatch = -1;
		int matchID;
		bool sComplete;
		int RID;
		int sCount;
		int matchWinnerID;
		int tmp;

		stringstream DATAPACKET;
		stringstream sendFpsSS;
		stringstream recvFpsSS;
		stringstream sendDataSS;
		stringstream recvDataSS;
		stringstream onlinePlayerCountSS;

	}TEMP;

}EP; 

struct MEMEORY
{
	struct TEXTURES
	{
		LTexture sendFpsText;
		LTexture recvFpsText;
		LTexture sendDatapacketText;
		LTexture recvDatapacketText;
		LTexture onlinePlayerCountText;

	}TEXTR;
	struct FONT
	{
		TTF_Font* gNorthFontLarge = NULL;
		SDL_Color fpsColor;
	}FNT;
	struct OBJECT_DATA
	{
		gMatch Match[MAX_MATCHES];
		gPlayer Player[MAX_PLAYER_ENTITY];

	}OBJ;

}MEM;

enum PhysicsType
{
	PHYSICS_TYPE_PROJECTILES,
	PHYSICS_TYPE_PLAYER_MOVEMENT
};

enum INDENTIFIER_TYPE
{
	GET_DATA_ABOUT_PLAYER,
	DELETE_PLAYER,
	NEW_PLAYER,
	DAMAGE_PLAYER,
	KILL_PLAYER,
	UPDATE_BULLET,
	START_MATCHMAKING,
	MATCHING_COMPLETE,
	MATCH_RESULT,
	END_OF_PACKET,
	SET_POSITION
};


enum PROJECTILE_IDENTIFIER
{
	PROJ_KILLSHOT,
	PROJ_NORMALSHOT
};

enum MATCHING_TYPE
{
	TWO_PLAYER,
	FOUR_PLAYER
};

SOCKET ListenSocket = INVALID_SOCKET;
struct addrinfo* result = NULL, * ptr = NULL, hints;
SOCKET ClientSocket[99];
bool clientSocketUsed[99], exists = false, queuePlayerDelete = false, sendNewProjectile = false;
int removeID, addID, addI;
string addNick;

LTexture loginPage_texture;
LTexture user_text_texture;
LTexture pass_text_texture;
LTexture green_textbox_texture;
LTexture red_textbox_texture;
LTexture info_text_texture;
LTexture background_texture;
LTexture character_nr1_texture;
LTexture crosshair_texture;
LTexture nickname_text_texture;

string posX, posY, flipType, animType, posX2, posY2;
string sIdentifier, id, nick, pcString;
int v1, v2, pc = 0, iResult, deleteID, newProjectilePlayerID, newProjectileID, identifier;
int tick = 0;
bool collisionFound = false, tempDelay = false;

LAnim ANIM_FIREBALL;

int xLast[99], yLast[99];

bool checkCollision(SDL_Rect a, SDL_Rect b);
void processRecivedPacket(string data);
void sendPacketToClient(int socket, string data);

LWindow gWindow(1280, 300);

bool getPhysicsReady(int type)
{
	if (PHYSICS_TYPE_PROJECTILES == type)
	{
		if (EP.GSYS.physicsTimerProjectile.getTicks() > EP.GSYS.physicsRate)
		{
			EP.GSYS.physicsTimerProjectile.reset();
			return true;
		}
	}
	else if (PHYSICS_TYPE_PLAYER_MOVEMENT == type)
	{
		if (EP.GSYS.physicsTimerMovement.getTicks() > EP.GSYS.physicsRate)
		{
			EP.GSYS.physicsTimerMovement.reset();
			return true;
		}
	}

	return false;
}

int sendRemovePacket(int rSocket)
{
	EP.TEMP.DATAPACKET.clear();
	EP.TEMP.DATAPACKET.str(string());
	EP.TEMP.DATAPACKET << DELETE_PLAYER << "," << MEM.OBJ.Player[rSocket].getID() << "," << MEM.OBJ.Player[rSocket].getNickname() << "," << END_OF_PACKET;
	
	EP.TEMP.iMatch = -1;

	EP.TEMP.sComplete = false;

	for (unsigned int i = 0; i < MAX_MATCHES; i++)
	{
		if (MEM.OBJ.Match[i].getIfOnGoing())
		{
			for (int j = 0; j < MAX_PLAYER_PER_MATCH; j++)
			{
				if (MEM.OBJ.Match[i].getPlayerID(j) == rSocket)
				{
					EP.TEMP.iMatch = i;
					EP.TEMP.sComplete = true;
					MEM.OBJ.Match[i].setIsPlayerDead(true, j);
					MEM.OBJ.Match[i].setIfSlotUsed(j, false);

					cout << endl << "set " << MEM.OBJ.Match[i].getPlayerID(j) << " as dead";

					break;
				}
			}
		}
		else if (MEM.OBJ.Match[i].getIfWaitingForPlayer())
		{
			for (int j = 0; j < MAX_PLAYER_PER_MATCH; j++)
			{
				if (MEM.OBJ.Match[i].getPlayerID(j) == rSocket)
				{
					EP.TEMP.sComplete = true;
					MEM.OBJ.Match[i].setIfSlotUsed(j, false);

					cout << endl << "discconect " << MEM.OBJ.Match[i].getPlayerID(j) << "from matching";

					if (MEM.OBJ.Match[i].getPlayersMatching() == 0)
					{
						MEM.OBJ.Match[i].resetMatch();
					}

					break;
				}
			}
		}
		if (EP.TEMP.sComplete) break;
	}

	if (EP.TEMP.iMatch != -1)
	{
		for (unsigned int i = 0; i < MEM.OBJ.Match[EP.TEMP.iMatch].getMaxPlayerCount(); i++)
		{
			if (MEM.OBJ.Match[EP.TEMP.iMatch].getIfSlotUsed(i))
			{
				sendPacketToClient(MEM.OBJ.Match[EP.TEMP.iMatch].getPlayerID(i), EP.TEMP.DATAPACKET.str());
				cout << endl << "Send remove packet to:" << MEM.OBJ.Match[EP.TEMP.iMatch].getPlayerID(i);
			}
		}
	}

	MEM.OBJ.Player[rSocket].setID("-1");
	ClientSocket[rSocket] = INVALID_SOCKET;

	EP.EXECUTE.updateMatch = true;

	cout << endl << "Dissconected: " << rSocket;

	return 0;
}

void sendPacketToClient(int socket, string data)
{
	iResult = send(ClientSocket[socket], data.c_str(), (int)strlen(data.c_str()), 0);
	if (iResult == SOCKET_ERROR)
	{
		cout << endl << "send failed: " << data << " ["<< WSAGetLastError() <<"]";
		EP.EXECUTE.sendRemovePacket = true;
		removeID = socket;
		MEM.OBJ.Player[socket].setSlotUsed(false);
	}
}

int getMatchPlayerRID(int playerID)
{
	for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (playerID == atoi(MEM.OBJ.Player[i].getID().c_str()))
		{
			return i;
		}
	}
}

bool init()
{
	bool success = true;

	srand(time(NULL));

	EP.GSYS.physicsTimerProjectile.start();
	EP.GSYS.sendFpsTimer.start();
	EP.GSYS.recvFpsTimer.start();

	int temp;
	string stemp;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}
		else
		{
			if (!gWindow.init())
			{
				printf("Window 0 could not be created!\n");
				success = false;
			}
			else
			{
				SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
					success = false;
				}
				/*
				if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
				{
					printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
					success = false;
				}
				*/
			}
		}
	}
	return success;
}
int processClientData()
{
	if (!getPhysicsReady(PHYSICS_TYPE_PROJECTILES))
	{
		return 0;
	}

	EP.TEMP.tmp = 0;

	for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (MEM.OBJ.Player[i].getSlotUsed())
		{
			EP.TEMP.tmp++;
			for (int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
			{
				if (!MEM.OBJ.Player[i].gProjectile[j].getSlotFree())
				{
					MEM.OBJ.Player[i].gProjectile[j].checkStatus();
					MEM.OBJ.Player[i].gProjectile[j].setCollisionOffset(MEM.OBJ.Player[i].gProjectile[j].getXCollisionOffset() + 1.0f, MEM.OBJ.Player[i].gProjectile[j].getYCollisionOffset() + 1);
					if (!ANIM_FIREBALL.serverProcessProjectileAnimState(0, 0, i, j, EP.ANIM.FIREBALL_RENDERSPEED))
					{
						MEM.OBJ.Player[i].gProjectile[j].setSlotFree(true);
					}
				}
			}
		}
	}

	EP.TEMP.onlinePlayerCountSS.clear();
	EP.TEMP.onlinePlayerCountSS.str(string());
	EP.TEMP.onlinePlayerCountSS << "Players Online: " << EP.TEMP.tmp;

	for (unsigned int i = 0; i < MAX_MATCHES; i++)
	{
		if (MEM.OBJ.Match[i].getIfOnGoing())
		{
			for (unsigned int j = 0; j < MEM.OBJ.Match[i].getPlayersMatching(); j++)
			{
				if (MEM.OBJ.Match[i].getIfSlotUsed(j))
				{
					for (unsigned int j2 = 0; j2 < MEM.OBJ.Match[i].getPlayersMatching(); j2++)
					{
						if (MEM.OBJ.Match[i].getIfSlotUsed(j2) && j != j2)
						{
							//CHECK PROJECTILE COLLISIONS
							for (unsigned int k = 0; k < MAX_PLAYER_BULLET_COUNT; k++)
							{
								if (!MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].getSlotFree())
								{
									for (unsigned int k2 = 0; k2 < MAX_PLAYER_BULLET_COUNT; k2++)
									{
										if (!MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].gProjectile[k2].getSlotFree())
										{
											if (checkCollision(MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].getCollisionRect(), MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].gProjectile[k2].getCollisionRect()))
											{
												MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].gProjectile[k2].setSlotFree(true);
												MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].setSlotFree(true);

												ANIM_FIREBALL.setCurrentTickClient(MEM.OBJ.Match[i].getPlayerID(j), k, 0);
												ANIM_FIREBALL.setCurrentTickClient(MEM.OBJ.Match[i].getPlayerID(j2), k2, 0);
											}
										}
									}
									if (checkCollision(MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].getCollisionRect(), MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].getCollisionRect()))
									{
										ANIM_FIREBALL.setCurrentTickClient(MEM.OBJ.Match[i].getPlayerID(j), k, 0);

										MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].setSlotFree(true);
										MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].damageTarget(MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].getDMG());

										EP.EXECUTE.playerDamage = true;
										EP.TEMP.damageGiverID = MEM.OBJ.Match[i].getPlayerID(j); // the one that did the damage
										EP.TEMP.damageTargetID = MEM.OBJ.Match[i].getPlayerID(j2); // the one damaged
										EP.TEMP.damageAmount = MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].gProjectile[k].getDMG();

										//CHECK IF PLAYERS DEAD

										if (MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j2)].getHealth() < 1)
										{
											//	MEM.OBJ.Player[j].setPlayerDead(true);
											//	EP.TEMP.killShot = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}


	/*
	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		if (MEM.OBJ.Player[i].getSlotUsed())
		{
			for (unsigned int j = 0; j < MAX_PLAYER_ENTITY; j++)
			{
				if (MEM.OBJ.Player[j].getSlotUsed() && MEM.OBJ.Player[i].getmID() == MEM.OBJ.Player[j].getmID() && i != j)
				{
					//CHECK PROJECTILE COLLISIONS

					for (unsigned int k = 0; k < MAX_PLAYER_BULLET_COUNT; k++)
					{
						if (!MEM.OBJ.Player[i].gProjectile[k].getSlotFree())
						{
							for (unsigned int l = 0; l < MAX_PLAYER_BULLET_COUNT; l++)
							{
								if (!MEM.OBJ.Player[j].gProjectile[l].getSlotFree())
								{
									if (checkCollision(MEM.OBJ.Player[i].gProjectile[k].getCollisionRect(), MEM.OBJ.Player[j].gProjectile[l].getCollisionRect()))
									{
										MEM.OBJ.Player[j].gProjectile[l].setSlotFree(true);
										MEM.OBJ.Player[i].gProjectile[k].setSlotFree(true);

										ANIM_FIREBALL.setCurrentTickClient(i, k, 0);
										ANIM_FIREBALL.setCurrentTickClient(j, l, 0);
									}
								}
							}
							if (checkCollision(MEM.OBJ.Player[j].getCollisionRect(), MEM.OBJ.Player[i].gProjectile[k].getCollisionRect()))
							{
								ANIM_FIREBALL.setCurrentTickClient(i, k, 0);

								MEM.OBJ.Player[i].gProjectile[k].setSlotFree(true);
								MEM.OBJ.Player[j].damageTarget(MEM.OBJ.Player[i].gProjectile[k].getDMG());

								EP.EXECUTE.playerDamage = true;
								EP.TEMP.damageGiverID = i; // the one that did the damage
								EP.TEMP.damageTargetID = j; // the one damaged
								EP.TEMP.damageAmount = MEM.OBJ.Player[i].gProjectile[k].getDMG();

								//CHECK IF PLAYERS DEAD

								if (MEM.OBJ.Player[j].getHealth() < 1)
								{
								//	MEM.OBJ.Player[j].setPlayerDead(true);
								//	EP.TEMP.killShot = true;
								}
							}
						}
					}
				}
			}
		}
	}
	*/
	
	if (EP.EXECUTE.updateMatch)
	{
		for (int i = 0; i < MAX_MATCHES; i++)
		{
			EP.TEMP.sCount = 0;

			if (MEM.OBJ.Match[i].getIfOnGoing())
			{
				for (int j = 0; j < MEM.OBJ.Match[i].getMaxPlayerCount(); j++)
				{
					EP.TEMP.RID = MEM.OBJ.Match[i].getPlayerID(j);

					if (!MEM.OBJ.Match[i].getIsPlayerDead(j))
					{
						EP.TEMP.matchWinnerID = atoi(MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getID().c_str());
						EP.TEMP.sCount++;
					}
				}
				if (EP.TEMP.sCount == 1)
				{
					EP.EXECUTE.matchOver = true;
					EP.TEMP.matchID = i;
				}
			}
		}
		EP.EXECUTE.updateMatch = false;
	}


	return 0;
}
bool checkCollision(SDL_Rect a, SDL_Rect b)
{
	//The sides of the rectangle
	int leftA, leftB;
	int rightA, rightB;
	int topA, topB;
	int bottomA, bottomB;

	//Calculate the sides of rect A
	leftA = a.x;
	rightA = a.x + a.w;
	topA = a.y;
	bottomA = a.y + a.h;

	//Calculate the sides of rect B
	leftB = b.x;
	rightB = b.x + b.w;
	topB = b.y;
	bottomB = b.y + b.h;

	//If any of the sides from A are outside of B
	if (bottomA <= topB)
	{
		return false;
	}

	if (topA >= bottomB)
	{
		return false;
	}

	if (rightA <= leftB)
	{
		return false;
	}

	if (leftA >= rightB)
	{
		return false;
	}

	//If none of the sides from A are outside B
	return true;
}

bool loadMedia()
{
	bool success = true;

	if (!loginPage_texture.loadFromFile("img/loginPage.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}
	if (!green_textbox_texture.loadFromFile("img/textboxgreen.png", gWindow.getRenderer()))
	{
		printf("Failed to load textboxgreen texture!\n");
		success = false;
	}
	if (!red_textbox_texture.loadFromFile("img/textboxred.png", gWindow.getRenderer()))
	{
		printf("Failed to load textboxred texture!\n");
		success = false;
	}
	if (!background_texture.loadFromFile("img/background.png", gWindow.getRenderer()))
	{
		printf("Failed to load background texture!\n");
		success = false;
	}
	if (!character_nr1_texture.loadFromFile("img/collision_reference.png", gWindow.getRenderer()))
	{
		printf("Failed to load collision_reference texture!\n");
		success = false;
	}
	if (!crosshair_texture.loadFromFile("img/crosshair.png", gWindow.getRenderer()))
	{
		printf("Failed to load login texture!\n");
		success = false;
	}

	if (TTF_Init() < 0)
	{
		cout << endl << "FAILED TO INITIALIZE TTF" << TTF_GetError();
		success = false;
	}
	else
	{
		MEM.FNT.gNorthFontLarge = TTF_OpenFont("font/ArialCE.ttf", 12);

		if (MEM.FNT.gNorthFontLarge == NULL)
		{
			printf("Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError());
			success = false;
		}
		MEM.FNT.fpsColor = { 0,0,0 };
	}

	for (unsigned int i = 0; i < MAX_PLAYER_ENTITY; i++)
	{
		MEM.OBJ.Player[i].setTextureWH(49, 65);
		MEM.OBJ.Player[i].setCollisionRectWH(49, 65);
		MEM.OBJ.Player[i].setCollisionOffset(25, 20);

		for (unsigned int j = 0; j < MAX_PLAYER_BULLET_COUNT; j++)
		{
			MEM.OBJ.Player[i].gProjectile[j].setMCWH(27, 27);

		}
	}
	if (!ANIM_FIREBALL.loadAnim(gWindow.getRenderer(), "img/attacks/fireball/fireball_000.png", 27))
	{
		cout << endl << "Failed to load anim fireball_000.png";
		success = false;
	}

	return success;
}

string getData(int len, char* data)
{
	string sdata(data);
	sdata = sdata.substr(0, len);
	return sdata;
}

string getFinalData(string data)
{
	v2 = v1;
	v1 = data.find(',', v2 + 1);
	return data.substr(v2 + 1, v1 - v2 - 1);
}

int BuildConnection()
{
	int iResult = WSAStartup(MAKEWORD(2, 2), &wData);

	if (iResult != 0)
	{
		cout << endl << "WSAStartup failed: " << iResult;
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}
	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);

	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	freeaddrinfo(result);

	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}
	return 0;
}

static int recvDataFromClient(void* ptr)
{
	char recvbuff[DEFAULT_BUFLEN];
	int sResult, iSendResult;
	int recvbuflen = DEFAULT_BUFLEN;

	bool found = false;

	while (true)
	{
		found = false;

		for (int i = 0; i < 99; i++)
		{
			if (MEM.OBJ.Player[i].getSlotUsed())
			{
				memset(recvbuff, 0, DEFAULT_BUFLEN);
				sResult = recv(ClientSocket[i], recvbuff, recvbuflen, 0);
				if (sResult > 0)
				{
					processRecivedPacket(getData(sResult, recvbuff));
				}
				else if (sResult == 0)
				{
					EP.EXECUTE.sendRemovePacket = true;
					removeID = i;
					MEM.OBJ.Player[i].setSlotUsed(false);
				}
				else if (sResult < 0)
				{
					EP.EXECUTE.sendRemovePacket = true;
					removeID = i;
					MEM.OBJ.Player[i].setSlotUsed(false);
				}
				found = true;
			}
		}
		if(!found) SDL_Delay(SDL_GLOBAL_DELAY);
	}
}

static int sendDataToClient(void* ptr)
{
	EP.TEMP.RID = -1;
	EP.TEMP.iMatch = -1;
	EP.TEMP.sComplete = false;

	while (true)
	{
		SDL_Delay(SDL_GLOBAL_DELAY);

		processClientData();

		if (EP.EXECUTE.sendRemovePacket)
		{
			sendRemovePacket(removeID);
			EP.EXECUTE.sendRemovePacket = false;
		}
		if (EP.EXECUTE.playerDamage)
		{
			EP.TEMP.iMatch = -1;
			//PLDMG,[THE ONE DAMAGED],[THE ONE THAT DOES THE DAMAGE],[DAMAGE AMMOUNT]

			EP.TEMP.DATAPACKET.clear();
			EP.TEMP.DATAPACKET.str(string());
			EP.TEMP.DATAPACKET << DAMAGE_PLAYER << "," << EP.TEMP.damageGiverID << "," << EP.TEMP.damageTargetID << "," << EP.TEMP.damageAmount << "," << END_OF_PACKET;

			EP.TEMP.RID = EP.TEMP.damageGiverID;

			for (int i = 0; i < MAX_MATCHES; i++)
			{
				if (MEM.OBJ.Match[i].getIfOnGoing())
				{
					for (int j = 0; j < MEM.OBJ.Match[i].getPlayersMatching(); j++)
					{
						if (MEM.OBJ.Match[i].getIfSlotUsed(j) && MEM.OBJ.Match[i].getPlayerID(j) == EP.TEMP.RID)
						{
							EP.TEMP.iMatch = i;
							EP.TEMP.sComplete = true;
							break;
						}
					}
				}
				if (EP.TEMP.sComplete) break;
			}

			if (EP.TEMP.iMatch != -1)
			{
				for (unsigned int i = 0; i < MEM.OBJ.Match[EP.TEMP.iMatch].getPlayersMatching(); i++)
				{
					if (MEM.OBJ.Match[EP.TEMP.iMatch].getIfSlotUsed(i))
					{
						sendPacketToClient(MEM.OBJ.Match[EP.TEMP.iMatch].getPlayerID(i), EP.TEMP.DATAPACKET.str());
					}
				}
			}
			EP.EXECUTE.playerDamage = false;
		}
		if (EP.EXECUTE.updateMatching)
		{
			// IDENTIFIER, MATCH TYPE ,PLAYER ID(0),PLAYER NICKNAME(0)

			for (unsigned int i = 0; i < MAX_MATCHES; i++)
			{
				if (MEM.OBJ.Match[i].getIfReqLaunch())
				{
					// SEND MATCHED CLIENTS THEIR PACKET

					EP.TEMP.DATAPACKET.clear();
					EP.TEMP.DATAPACKET.str(string());

					EP.TEMP.DATAPACKET << MATCHING_COMPLETE << "," << MEM.OBJ.Match[i].getMatchingType();

					int playerCount = MEM.OBJ.Match[i].getMatchingType() == 0 ? 2 : 4;
					
					for (int j = 0; j < playerCount; j++)
					{
						EP.TEMP.relativeID = MEM.OBJ.Match[i].getPlayerID(j);
						EP.TEMP.DATAPACKET << "," << MEM.OBJ.Player[EP.TEMP.relativeID].getID() << "," << MEM.OBJ.Player[EP.TEMP.relativeID].getNickname();
					}

					EP.TEMP.DATAPACKET << "," << END_OF_PACKET;

					for (int j = 0; j < playerCount; j++)
					{
						EP.TEMP.relativeID = MEM.OBJ.Match[i].getPlayerID(j);
						MEM.OBJ.Player[EP.TEMP.relativeID].setIfPlaying(true);

						sendPacketToClient(EP.TEMP.relativeID, EP.TEMP.DATAPACKET.str());
					}

					if (playerCount == 2)
					{
						EP.TEMP.DATAPACKET.clear();
						EP.TEMP.DATAPACKET.str(string());

						EP.TEMP.DATAPACKET << SET_POSITION << "," << "100" << "," << "300" << "," << END_OF_PACKET;

						sendPacketToClient(MEM.OBJ.Match[i].getPlayerID(0), EP.TEMP.DATAPACKET.str());

						EP.TEMP.DATAPACKET.clear();
						EP.TEMP.DATAPACKET.str(string());

						EP.TEMP.DATAPACKET << SET_POSITION << "," << "1180" << "," << "300" << "," << END_OF_PACKET;
						
						sendPacketToClient(MEM.OBJ.Match[i].getPlayerID(1), EP.TEMP.DATAPACKET.str());
					}
						

					MEM.OBJ.Match[i].setIfIsOnGoing(true);
					MEM.OBJ.Match[i].setIfReqLaunch(false);

					break;
				}
			}
			EP.EXECUTE.updateMatching = false;
		}
		if (EP.EXECUTE.matchOver)
		{
			EP.TEMP.DATAPACKET.clear();
			EP.TEMP.DATAPACKET.str(string());
			EP.TEMP.DATAPACKET << MATCH_RESULT << "," << EP.TEMP.matchWinnerID << "," << END_OF_PACKET;

			for (int i = 0; i < MAX_MATCHES; i++)
			{
				if (MEM.OBJ.Match[i].getIfOnGoing() && EP.TEMP.matchID == i)
				{
					for (int j = 0; j < MEM.OBJ.Match[i].getMaxPlayerCount(); j++)
					{
						if (MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getSlotUsed())
						{
							sendPacketToClient(MEM.OBJ.Match[i].getPlayerID(j), EP.TEMP.DATAPACKET.str());
							cout << endl << "send MATCH_RESULT TO:" << MEM.OBJ.Match[i].getPlayerID(j) << "[" << EP.TEMP.DATAPACKET.str() << "]";
						}
					}
					break;
				}
			}

			MEM.OBJ.Match[EP.TEMP.matchID].resetMatch();
			EP.EXECUTE.matchOver = false;

			cout << endl << "MATCH TERMIANTED";
		}

		// SEND BASIC PLAYER DATA PER MATCH

		EP.TEMP.DATAPACKET.clear();
		EP.TEMP.DATAPACKET.str(string());
		EP.TEMP.DATAPACKET << GET_DATA_ABOUT_PLAYER << ",";

		for (int i = 0; i < MAX_MATCHES; i++)
		{
			if (MEM.OBJ.Match[i].getIfOnGoing())
			{
				EP.TEMP.DATAPACKET << MEM.OBJ.Match[i].getPlayersMatching() << ",";

				for (int j = 0; j < MEM.OBJ.Match[i].getPlayersMatching();j++)
				{
					if (MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getSlotUsed() && MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getIfPlaying() && !MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getRequireInit())
					{
						EP.TEMP.DATAPACKET << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getID() << "," << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getNickname() << "," << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getmCollider().x << "," << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getmCollider().y << ",";
						EP.TEMP.DATAPACKET << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getFlipType() << "," << MEM.OBJ.Player[MEM.OBJ.Match[i].getPlayerID(j)].getAnimType() << ",";
					}
				}
				EP.TEMP.DATAPACKET << END_OF_PACKET;

				for (int j = 0; j < MEM.OBJ.Match[i].getPlayersMatching(); j++)
				{
					if (MEM.OBJ.Match[i].getIfSlotUsed(j))
					{
						sendPacketToClient(MEM.OBJ.Match[i].getPlayerID(j), EP.TEMP.DATAPACKET.str());
					}
				}
			}
			EP.TEMP.DATAPACKET.clear();
			EP.TEMP.DATAPACKET.str(string());
			EP.TEMP.DATAPACKET << GET_DATA_ABOUT_PLAYER << ",";
		}

		EP.TEMP.DATAPACKET.clear();
		EP.TEMP.DATAPACKET.str(string());

		//SEND PLAYERS NEW PROJECTILES
		if (sendNewProjectile)
		{
			EP.TEMP.DATAPACKET << UPDATE_BULLET << "," << MEM.OBJ.Player[newProjectilePlayerID].getID() << "," << MEM.OBJ.Player[newProjectilePlayerID].getNickname() << "," << MEM.OBJ.Player[newProjectilePlayerID].gProjectile[newProjectileID].getPosX() << "," << MEM.OBJ.Player[newProjectilePlayerID].gProjectile[newProjectileID].getPosY() << "," << MEM.OBJ.Player[newProjectilePlayerID].gProjectile[newProjectileID].getDestX() << "," << MEM.OBJ.Player[newProjectilePlayerID].gProjectile[newProjectileID].getDestY() << ",";
			EP.TEMP.DATAPACKET << END_OF_PACKET;

			for (int i = 0; i < MAX_MATCHES; i++)
			{
				if (MEM.OBJ.Match[i].getIfOnGoing())
				{
					for (int j = 0; j < MEM.OBJ.Match[i].getPlayersMatching(); j++)
					{
						if (MEM.OBJ.Match[i].getPlayerID(j) == newProjectilePlayerID)
						{
							for (int k = 0; k < MEM.OBJ.Match[i].getPlayersMatching(); k++)
							{
								if (MEM.OBJ.Match[i].getIfSlotUsed(k))
								{
									sendPacketToClient(MEM.OBJ.Match[i].getPlayerID(k), EP.TEMP.DATAPACKET.str());
								}
							}
							break;
						}
					}
				}
			}
			sendNewProjectile = false;
		}
		EP.TEMP.sendDataSS.clear();
		EP.TEMP.sendDataSS.str(EP.TEMP.DATAPACKET.str());
	}
	//Calculate tickRate

	EP.TEMP.sendFrame++;

	if (EP.GSYS.sendFpsTimer.getTicks() > 500)
	{
		//Calculate the sendFrames per second and create the string

		EP.TEMP.sendFpsSS.clear();
		EP.TEMP.sendFpsSS.str("");
		EP.TEMP.sendFpsSS << "PACKET-SENT/SEC:" << (int)(EP.TEMP.sendFrame / (EP.GSYS.sendFpsTimer.getTicks() / 1000.f));

		//Restart the update timer
		EP.GSYS.sendFpsTimer.start();
		EP.TEMP.sendFrame = 0;
	}
}

void processRecivedPacket(string data)
{
	bool TASK_DONE = false;
	int ARID;

	v1 = data.find(',');
	sIdentifier = data.substr(0, v1);
	id = getFinalData(data);
	nick = getFinalData(data);

	identifier = atoi(sIdentifier.c_str());

	if (identifier == NEW_PLAYER) //NEW PLAYER 
	{
		collisionFound = true;
		for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
		{
			if (MEM.OBJ.Player[i].getSlotUsed() && MEM.OBJ.Player[i].getRequireInit())
			{
				MEM.OBJ.Player[i].setPlayerPosX(0);
				MEM.OBJ.Player[i].setPlayerPosY(0);
				MEM.OBJ.Player[i].setNickname(nick);
				MEM.OBJ.Player[i].setID(id);
				MEM.OBJ.Player[i].setFlipType("none");
				MEM.OBJ.Player[i].setAnimType("idle");
				MEM.OBJ.Player[i].setRequireInit(false);

				addI = i;
				break;
			}
		}
		addID = atoi(id.c_str());
		addNick = nick;
		EP.EXECUTE.addClient = true;
	}
	else if (identifier == GET_DATA_ABOUT_PLAYER) //UPDATE PLAYER WITH INFO FROM CLIENT
	{
		posX = getFinalData(data);
		posY = getFinalData(data);
		flipType = getFinalData(data);
		animType = getFinalData(data);

		for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
		{
			if (MEM.OBJ.Player[i].getID() == id)
			{
				MEM.OBJ.Player[i].setPlayerPosX(atoi(posX.c_str()));
				MEM.OBJ.Player[i].setPlayerPosY(atoi(posY.c_str()));
				MEM.OBJ.Player[i].setFlipType(flipType);
				MEM.OBJ.Player[i].setAnimType(animType);

				for (unsigned int m = 0; m < MAX_MATCHES; m++)
				{
					if (MEM.OBJ.Match[m].getIfOnGoing())
					{
						for (unsigned int p = 0; p < MEM.OBJ.Match[m].getPlayersMatching(); p++)
						{
							if (MEM.OBJ.Match[m].getIfSlotUsed(p) && MEM.OBJ.Match[m].getPlayerID(p) == i)
							{
								for (unsigned int p2 = 0; p2 < MEM.OBJ.Match[m].getPlayersMatching(); p2++)
								{
									if (p != p2 && MEM.OBJ.Match[m].getIfSlotUsed(p2) && checkCollision(MEM.OBJ.Player[MEM.OBJ.Match[m].getPlayerID(p)].getmCollider(), MEM.OBJ.Player[MEM.OBJ.Match[m].getPlayerID(p2)].getmCollider()))
									{
										MEM.OBJ.Player[MEM.OBJ.Match[m].getPlayerID(p)].setPlayerPosX(xLast[i]);
										MEM.OBJ.Player[MEM.OBJ.Match[m].getPlayerID(p)].setPlayerPosY(yLast[i]);
										collisionFound = true;

										break;
									}
								}
								TASK_DONE = true;
								break;
							}
						}
					}
					if (TASK_DONE) break;
				}

				if (!collisionFound)
				{
					xLast[i] = atoi(posX.c_str());
					yLast[i] = atoi(posY.c_str());
				}
				break;
			}
		}
	}
	else if (identifier == DELETE_PLAYER)
	{
		for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
		{
			if (MEM.OBJ.Player[i].getSlotUsed() && MEM.OBJ.Player[i].getID() == id)
			{
				MEM.OBJ.Player[i].setSlotUsed(false);
				removeID = i;
				EP.EXECUTE.sendRemovePacket = true;
				break;
			}
		}
	}
	else if (identifier == START_MATCHMAKING)
	{

		int matchingType = atoi(getFinalData(data).c_str());
		bool matchedPlayer = false;

		EP.TEMP.relativeID = getMatchPlayerRID(atoi(id.c_str()));

		for (unsigned int i = 0; i < MAX_MATCHES; i++)
		{
			if (MEM.OBJ.Match[i].getIfWaitingForPlayer() && MEM.OBJ.Match[i].getMatchingType() == matchingType && !matchedPlayer)
			{
				for (unsigned int j = 0; j < MAX_PLAYER_PER_MATCH; j++)
				{
					if (!MEM.OBJ.Match[i].getIfSlotUsed(j))
					{
						MEM.OBJ.Match[i].setIfSlotUsed(j, true);
						MEM.OBJ.Match[i].setPlayerID(j, EP.TEMP.relativeID);

						if (matchingType == TWO_PLAYER)
						{
							if (MEM.OBJ.Match[i].getPlayersMatching() == 2)
							{
								MEM.OBJ.Match[i].setIfIsWaitingForPlayer(false);
								MEM.OBJ.Match[i].setIfReqLaunch(true);
								MEM.OBJ.Player[EP.TEMP.relativeID].setmID(i);
								EP.EXECUTE.updateMatching = true;
							}
						}
						else if (matchingType == FOUR_PLAYER)
						{
							if (MEM.OBJ.Match[i].getPlayersMatching() == 4)
							{
								MEM.OBJ.Match[i].setIfIsWaitingForPlayer(false);
								MEM.OBJ.Match[i].setIfReqLaunch(true);
								MEM.OBJ.Player[EP.TEMP.relativeID].setmID(i);
								EP.EXECUTE.updateMatching = true;
							}
						}
						matchedPlayer = true;
						break;
					}
				}
			}
			else if (matchedPlayer) break;
		}
		if (!matchedPlayer)
		{
			for (unsigned int i = 0; i < MAX_MATCHES; i++)
			{
				if (!MEM.OBJ.Match[i].getIfInit())
				{
					MEM.OBJ.Match[i].setIfSlotUsed(0, true);
					MEM.OBJ.Match[i].setPlayerID(0,EP.TEMP.relativeID);
					MEM.OBJ.Match[i].setIfIsWaitingForPlayer(true);
					MEM.OBJ.Match[i].setMatchingType(matchingType);
					MEM.OBJ.Match[i].setIfInit(true);
					MEM.OBJ.Player[EP.TEMP.relativeID].setmID(i);

					break;
				}
			}
		}
	}

	sIdentifier = getFinalData(data);
	identifier = atoi(sIdentifier.c_str());
	
	if (identifier == UPDATE_BULLET)
	{
		posX = getFinalData(data);
		posY = getFinalData(data);
		posX2 = getFinalData(data);
		posY2 = getFinalData(data);

		for (int i = 0; i < MAX_PLAYER_ENTITY; i++)
		{
			if (id == MEM.OBJ.Player[i].getID())
			{
				for (int j = 0; j < 30; j++)
				{
					if (MEM.OBJ.Player[i].gProjectile[j].getSlotFree())
					{
						MEM.OBJ.Player[i].gProjectile[j].setSlotFree(false);
						MEM.OBJ.Player[i].gProjectile[j].setPosX(atoi(posX.c_str()));
						MEM.OBJ.Player[i].gProjectile[j].setPosY(atoi(posY.c_str()));
						MEM.OBJ.Player[i].gProjectile[j].setDestX(atoi(posX2.c_str()));
						MEM.OBJ.Player[i].gProjectile[j].setDestY(atoi(posY2.c_str()));
						MEM.OBJ.Player[i].gProjectile[j].setVelX((atoi(posX2.c_str()) - atoi(posX.c_str())) / EP.GSYS.projSpeed);
						MEM.OBJ.Player[i].gProjectile[j].setVelY((atoi(posY2.c_str()) - atoi(posY.c_str())) / EP.GSYS.projSpeed);

						newProjectilePlayerID = i;
						newProjectileID = j;
						break;
					}
				}
				break;
			}
		}
		sendNewProjectile = true;
		
		sIdentifier = getFinalData(data);
		identifier = atoi(sIdentifier.c_str());
	}
	collisionFound = false;

	EP.TEMP.recvDataSS.clear();
	EP.TEMP.recvDataSS.str(data);

	//Calculate tickRate

	EP.TEMP.recvFrame++;

	if (EP.GSYS.recvFpsTimer.getTicks() > 500)
	{
		//Calculate the sendFrames per second and create the string
	
		EP.TEMP.recvFpsSS.clear();
		EP.TEMP.recvFpsSS.str("");
		EP.TEMP.recvFpsSS << "PACKET-RECV/SEC:" << (int)(EP.TEMP.recvFrame / (EP.GSYS.recvFpsTimer.getTicks() / 1000.f));
	
		//Restart the update timer
		EP.GSYS.recvFpsTimer.start();
		EP.TEMP.recvFrame = 0;
	}

}

static int runServerThread(void* ptr)
{
	stringstream data;

	while (true)
	{
		SDL_Delay(SDL_GLOBAL_DELAY);
		for (int i = 0; i < 99; i++)
		{
			if (!MEM.OBJ.Player[i].getSlotUsed())
			{
				ClientSocket[i] = accept(ListenSocket, NULL, NULL);
				if (ClientSocket[i] == INVALID_SOCKET) 
				{
					printf("accept failed: %d\n", WSAGetLastError());
				}
				else
				{
					cout << endl << "SOCKET CONNECTED ON ID " << i;
					MEM.OBJ.Player[i].setSlotUsed(true);
					MEM.OBJ.Player[i].setRequireInit(true);
				}
				break;
			}
		}
		data.str("");
	}
	return 0;
}

int main(int argc, char* args[])
{
	bool quit = false;

	for (int i = 0; i < 99; i++)
	{
		ClientSocket[i] = INVALID_SOCKET;
		MEM.OBJ.Player[i].setSlotUsed(false);
		MEM.OBJ.Player[i].setClientSocketID(999);
	}

	init();

	loadMedia();

	BuildConnection();

	//Create separate thread to listen to client connections

	SDL_Thread* listenThread = SDL_CreateThread(runServerThread, "ServerThread", (void*)NULL);
	SDL_Thread* sendThread = SDL_CreateThread(sendDataToClient, "ServerThread", (void*)NULL);
	SDL_Thread* recvThread = SDL_CreateThread(recvDataFromClient, "recvThread", (void*)NULL);

	do {
		while (SDL_PollEvent(&EP.GSYS.e))
		{
			if (EP.GSYS.e.type == SDL_KEYDOWN)
			{
			
			}
			else if (EP.GSYS.e.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		MEM.TEXTR.recvDatapacketText.loadFromRenderedText(EP.TEMP.recvDataSS.str(), MEM.FNT.fpsColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);
		MEM.TEXTR.sendDatapacketText.loadFromRenderedText(EP.TEMP.sendDataSS.str(), MEM.FNT.fpsColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);
		MEM.TEXTR.sendFpsText.loadFromRenderedText(EP.TEMP.sendFpsSS.str(), MEM.FNT.fpsColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);
		MEM.TEXTR.recvFpsText.loadFromRenderedText(EP.TEMP.recvFpsSS.str(), MEM.FNT.fpsColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);
		MEM.TEXTR.onlinePlayerCountText.loadFromRenderedText(EP.TEMP.onlinePlayerCountSS.str(), MEM.FNT.fpsColor, gWindow.getRenderer(), MEM.FNT.gNorthFontLarge);

		SDL_RenderClear(gWindow.getRenderer());
		SDL_SetRenderDrawColor(gWindow.getRenderer(), 0xFF, 0xFF, 0xFF, 0xFF);

		MEM.TEXTR.onlinePlayerCountText.render(gWindow.getRenderer(), 0, 100);
		MEM.TEXTR.sendFpsText.render(gWindow.getRenderer(), 0, 0);
		MEM.TEXTR.recvFpsText.render(gWindow.getRenderer(), 0, 25);
		MEM.TEXTR.sendDatapacketText.render(gWindow.getRenderer(), 0, 50);
		MEM.TEXTR.recvDatapacketText.render(gWindow.getRenderer(), 0, 75);

		gWindow.render();

		SDL_Delay(SDL_GLOBAL_DELAY);

	}while (!quit);
	
	WSACleanup();

	SDL_DetachThread(listenThread);
	SDL_DetachThread(sendThread);
	SDL_DetachThread(recvThread);

	return 0;
}
