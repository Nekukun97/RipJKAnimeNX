#include <unistd.h>
#include <switch.h>
#include <dirent.h>
#define _CRT_SECURE_NO_WARNINGS

//test
#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <SDL_mixer.h>
#include <errno.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <iostream>
#include<ctime>
#include <iomanip>
#include <math.h>
#include <Vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <thread>
#include "Networking.hpp"
#include "SDLWork.hpp"
#include "applet.hpp"
#include "utils.hpp"
//////////////////////////////////aquí empieza el pc.
//Screen dimension constants
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
//Rendered texture
LTexture gTextTexture;
LTexture Farest;
LTexture Heart;
//Render Buttons
LTexture B_A, B_B, B_Y, B_X, B_L, B_R, B_ZR, B_M, B_P, B_RIGHT, B_LEFT, B_UP, B_DOWN;
//Render extra
LTexture BUS, REC, USER, NFAV, FAV, NOP, CLEAR, SCREEN;
//Text and BOXES
LTexture VOX;
LTexture T_T;
LTexture T_D;
LTexture T_R;

//main SLD funct (Grafics On Display == GOD)
SDLB GOD;
	
//Gui Vars
enum states { programationstate, downloadstate, chapterstate, searchstate, favoritesstate };
enum statesreturn { toprogramation, tosearch, tofavorite };
int statenow = programationstate;
int returnnow = toprogramation;
//net
std::string urltodownload = "";
int porcendown = 0;
int sizeestimated = 0;
int cancelcurl = 0;
bool lcdoff=false;
//img
bool reloading = false;
bool preview = false;
int selectchapter = 0;
int imgNumbuffer = 0;
int porcentajebuffer = 0;
int porcentajebufferAll = 0;
int porcentajebufferF =0;
int porcentajebufferFF =0;
std::string serverenlace = "...";
std::string DownTitle="...";


//search
int searchchapter = 0;
bool reloadingsearch = false;
//downloads
bool isDownloading=false;
std::string speedD="";
//favs
int favchapter = 0;
bool gFAV = false;
//server
int selectserver = 0;
bool serverpront = false;

//my vars
bool AppletMode=false;
bool isSXOS=false;
bool hasStealth=false;
bool ongrid=true;
bool ongridS=true;
bool ongridF=true;
AccountUid uid;
std::string AccountID="";
SDL_Thread* capithread = NULL;
bool quit=false;
std::string KeyName;
int Frames=1;

std::vector<std::string> arrayservers= {
"Nozomi","Fembed 2.0","MixDrop","Desu","Xtreme S","Okru"
};

std::vector<std::string> arrayserversbak= {
"Nozomi","Fembed 2.0","MixDrop","Desu","Xtreme S","Okru"
};

json BD;
json UD;

//caps vars
int maxcapit = 1;
int mincapit = 0;
int latest = 1;

int sizeportraity = 300;
int sizeportraitx =424;
int xdistance = 1010;
int ydistance = 340;
bool isConnected = true;
