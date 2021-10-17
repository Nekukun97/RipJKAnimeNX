#include <string>
#include "JKanime.hpp"
#include "extra.hpp"

//make some includes to clean a little the main
string urlc = "https://myrincon.duckdns.org";

//MAIN INT
int main(int argc, char **argv)
{
	socketInitializeDefault();
	romfsInit();
	nxlinkStdio();
	printf("Nxlink server Conected\n");

	//Mount user
	FsFileSystem data;
	fsOpenBisFileSystem(&data, FsBisPartitionId_User, "");
	fsdevMountDevice("user", data);

	//LOG Init
	LOG::init();
	
	AppletMode=GetAppletMode();
	ChainManager(true,true);
	isConnected=Net::HasConnection();

	mount_theme("themes",true);
	
	//Mount Save Data
	FsFileSystem acc;
	if (MountUserSave(acc) & !AppletMode) {
		if(isFileExist(rootdirectory+AccountID+"UserData.json")) {
			if(!isFileExist(rootsave+"UserData.json")) {
				if (copy_me(rootdirectory+AccountID+"UserData.json", rootsave+"UserData.json")) {
					fsdevCommitDevice("save");
					remove((rootdirectory+AccountID+"UserData.json").c_str());
					remove((rootdirectory+AccountID+"User.jpg").c_str());
				}
			}
		}
		if(isFileExist(rootdirectory+"UserData.json")) {
			if(!isFileExist(rootsave+"UserData.json")) {
				if (copy_me(rootdirectory+"UserData.json", rootsave+"UserData.json")) {
					fsdevCommitDevice("save");
					remove((rootdirectory+"UserData.json").c_str());
				}
			}
		}
	}

	struct stat st = { 0 };
	if (stat("sdmc:/RipJKAnime", &st) != -1) {
		fsdevDeleteDirectoryRecursively("sdmc:/RipJKAnime");
	}

	//Read a JSON file
	if(!read_DB(AB,rootdirectory+"AnimeBase.json"))
	{
		read_DB(AB,"romfs:/AnimeBase.json");
	}
	
	read_DB(BD,rootdirectory+"DataBase.json");
	
	BD["com"] = "{}"_json;
	if(!BD["USER"].is_null()) {
		UD=BD["USER"];
	}
	BD.erase("USER");
	if(!BD["DataBase"].is_null()) {
		BD.erase("DataBase");
	}
	
	read_DB(UD,rootsave+"UserData.json");

	GOD.intA();//init the SDL

	if (stat((rootdirectory+"DATA").c_str(), &st) == -1) {
		mkdir(rootdirectory.c_str(), 0777);
		mkdir((rootdirectory+"DATA").c_str(), 0777);
	}
	
	GOD.loadSkin();//Esto carga la skin guardada y la musica

	gTextTexture.mark=false;
	Farest.mark=false;
	USER.loadFromFileCustom(rootsave+"User.jpg",58, 58);

	SDL_Color textColor = { 50, 50, 50 };
	SDL_Color textWhite = { 255, 255, 255 };
	SDL_Color textBlue = { 100, 100, 255 };
	SDL_Color textGray = { 130, 130, 130 };
	SDL_Color textGrayGreen = { 90, 170, 90 };
	SDL_Color textWhiteGreen = { 80, 255, 80 };

	int posxbase = 20;
	int posybase = 10;

	//Event handler
	SDL_Event e;

	for (int i = 0; i < 2; i++) {
		if (SDL_JoystickOpen(i) == NULL) {
			SDL_Log("SDL_JoystickOpen: %s\n", SDL_GetError());
			SDL_Quit();
			return -1;
		}
	}

	try{
		//Load images from Romfs
		LoadImages();

		if (isSXOS){
			GOD.GenState = statenow;
			SDL_SetRenderDrawColor(GOD.gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(GOD.gRenderer);
			GOD.JKMainLoop();
			Farest.render((0), (0));
			GOD.Confirm("Esta App No funciona Correctamente En SXOS",true,3);
		}
		//Set main Thread get images and descriptions
		Loaderthread = SDL_CreateThread(AnimeLoader, "Loaderthread", (void*)NULL);
		//Handle forced exit
		if (!AppletMode) appletLockExit();
		
		
		//While application is running
		while (GOD.JKMainLoop())
		{
			//get if console is dokked
			AppletOperationMode stus=appletGetOperationMode();
			if (stus == AppletOperationMode_Handheld) {isHandheld=true;}
			if (stus == AppletOperationMode_Console) {isHandheld=false;}

			//Handle events on queue
			while (SDL_PollEvent(&e))
			{
				//User requests quit
				if (e.type == SDL_QUIT)
				{
					cancelcurl = 1;
					quit = true;
					cout << "Saliendo" << endl;
				}
				GOD.GenState = statenow;
				switch (e.type) {
				case SDL_FINGERDOWN:
					GOD.TouchX = e.tfinger.x * SCREEN_WIDTH;
					GOD.TouchY = e.tfinger.y * SCREEN_HEIGHT;
					if (!GOD.fingerdown) {GOD.fingerdown = true;}
					break;
				case SDL_FINGERMOTION:
					if(e.tfinger.dy * SCREEN_HEIGHT > 30 || e.tfinger.dy * SCREEN_HEIGHT < -30 || e.tfinger.dx * SCREEN_WIDTH > 30 || e.tfinger.dx * SCREEN_WIDTH < -30) {
						SDL_Log("motion %f \n",e.tfinger.dy * SCREEN_HEIGHT);
						if(!GOD.fingermotion) {
							//swipe up go down
							if(e.tfinger.dy * SCREEN_HEIGHT > 30 )
							{
								GOD.fingermotion_DOWN = true;
							}
							//swipe down go up
							else if(e.tfinger.dy * SCREEN_HEIGHT < -30 )
							{
								GOD.fingermotion_UP = true;
							}

							//left right
							if(e.tfinger.dx * SCREEN_WIDTH > 30 )
							{
								GOD.fingermotion_RIGHT = true;
							}
							else if(e.tfinger.dx * SCREEN_WIDTH < -30 )
							{
								GOD.fingermotion_LEFT = true;
							}

						} else {
							GOD.fingermotion_DOWN = false;
							GOD.fingermotion_UP = false;
							GOD.fingermotion_LEFT = false;
							GOD.fingermotion_RIGHT = false;
							GOD.TouchX = -1;
							GOD.TouchY = -1;
						}
						GOD.fingermotion=true;
					}
					break;
				case SDL_FINGERUP:
					if (e.type == SDL_FINGERUP) {
						if(lcdoff) {
							lcdoff=false;
							appletSetLcdBacklightOffEnabled(lcdoff);
						} else{
							GOD.fingerdown = false;
							GOD.fingermotion=false;
							GOD.TouchX = e.tfinger.x * SCREEN_WIDTH;
							GOD.TouchY = e.tfinger.y * SCREEN_HEIGHT;
							e.jbutton.button=-1;
							if (BUSB.SP()) {callsearch();}
							else if (FAVB.SP()) {callfavs();}
							else if (AFLV.SP()) {callAflv();}
							else if (HISB.SP()) {callhistory();}
							else if (NFAV.SP() && !gFAV) {
								GOD.WorKey="0"; GOD.MasKey=-1;
								{//AGREGAR A FAVORITOS
									addFavorite(BD["com"]["ActualLink"]);
									gFAV = true;
									FAV.TickerBomb();
									GOD.TouchX = -1;
									GOD.TouchY = -1;
								}
								break;
							}
							else if (GOD.MasKey >=0) {
								if(GOD.MapT[GOD.WorKey].SP()) {
									e.jbutton.button=GOD.MasKey;
								}
								GOD.WorKey="0"; GOD.MasKey=-1;
							}
							else if (GOD.MapT["EXIT"].SP()) e.jbutton.button = BT_P;
							else if (GOD.MapT["MUSIC"].SP()) e.jbutton.button = BT_M;
							
							else if (USER.SP()) {PlayerGet(acc);}
							else if (B_A.SP()) {e.jbutton.button = BT_A; B_A.TickerBomb();}
							else if (T_T.SP() ) e.jbutton.button = BT_A;
							else if (B_B.SP()) e.jbutton.button = BT_B;
							else if (BACK.SP()) e.jbutton.button = BT_B;
							else if (B_X.SP()) e.jbutton.button = BT_X;
							else if (B_Y.SP()) e.jbutton.button = BT_Y;
							else if (B_L.SP()) e.jbutton.button = BT_L;
							else if (B_R.SP()) e.jbutton.button = BT_R;
							else if (B_ZR.SP()) e.jbutton.button = BT_ZR;
							else if (B_ZL.SP()) e.jbutton.button = BT_ZL;
							else if (B_P.SP()) e.jbutton.button = BT_P;
							else if (B_M.SP()) e.jbutton.button = BT_M;
							else if (B_LEFT.SP()) e.jbutton.button = BT_LEFT;
							else if (B_RIGHT.SP()) e.jbutton.button = BT_RIGHT;
							else if (B_UP.SP()) e.jbutton.button = BT_UP;
							else if (B_DOWN.SP()) e.jbutton.button = BT_DOWN;
							else if (T_D.SP()&&isDownloading) statenow = download_s;
							else if (SCREEN.SP()) e.jbutton.button = BT_ZR;
							else if (CLEAR.SP()) {
								preview = false;
								quit = true;
								cancelcurl = 1;
								GOD.PleaseWait("Borrando cache");
								read_DB(AB,"romfs:/AnimeBase.json");
								BD="{}"_json;
								mount_theme("themes",false);
								remove((rootdirectory+"theme.romfs").c_str());
								remove((rootdirectory+"update.nsp.json").c_str());
								remove((rootdirectory+"update.nsp").c_str());
								remove((rootdirectory+"LOG.txt").c_str());
								fsdevDeleteDirectoryRecursively((rootdirectory+"DATA").c_str());
								statenow=99;
								break;
							}
							SDL_Log("ScreenX %d    ScreenY %d butt %d\n",GOD.TouchX, GOD.TouchY,e.jbutton.button);
						}
						GOD.TouchX = -1;
						GOD.TouchY = -1;
					}
				case SDL_JOYAXISMOTION:
					if (e.type == SDL_JOYAXISMOTION)
					{
						if (e.jaxis.axis == 1 && statenow==chapter_s) {
							e.jbutton.button=-1;
							//SDL_Log("Joystick %d axis %d value: %d\n",e.jaxis.which,e.jaxis.axis, e.jaxis.value);
							if (e.jaxis.value < -22000) {
								e.jbutton.button = BT_UP;
							}
							if (e.jaxis.value > 22000) {
								e.jbutton.button = BT_DOWN;
							}
						} else break;

					}
				case SDL_JOYBUTTONDOWN:
					//SDL_Log("Joystick %d button %d down\n",e.jbutton.which, e.jbutton.button);
					// https://github.com/devkitPro/SDL/blob/switch-sdl2/src/joystick/switch/SDL_sysjoystick.c#L52
					// seek for joystick #0
					if (e.jbutton.which == 0) {
						if (e.jbutton.button == BT_A) {// (A) button down

							switch (statenow)
							{
							case menu_s:
								if(selectelement==0) {callsearch();}
								if(selectelement==1) {callhistory();}
								if(selectelement==2) {callfavs();}
								if(selectelement==3) {callhourglass();}
								if(selectelement==4) {calltop();}
								if(selectelement==5) {callagr();}
								if(selectelement==6) {callAflv();}
								if(selectelement==7) {if(isDownloading) {statenow = download_s;}}
								break;
							case programation_s:
							{
								if (!reloading&&BD["arrays"]["chapter"]["link"].size()>=1)
								{
									capBuffer(BD["arrays"]["chapter"]["link"][selectchapter]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}
							}
							break;

							case search_s:
							{
								if (!reloadingsearch && BD["arrays"]["search"]["link"].size()>=1)
								{
									capBuffer(BD["arrays"]["search"]["link"][searchchapter]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}

							}
							break;
							case hourglass_s:
							{
								if (BD["arrays"]["HourGlass"]["link"].size()>0)
								{
									capBuffer(BD["arrays"]["HourGlass"]["link"][hourchapter]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}
							}
							break;
							case top_s:
							{
								if (BD["arrays"]["Top"]["link"].size()>0)
								{
									capBuffer(BD["arrays"]["Top"]["link"][topchapter]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}
							}
							break;
							case agregados_s:
							{
								if (BD["arrays"]["Agregados"]["link"].size()>0)
								{
									capBuffer(BD["arrays"]["Agregados"]["link"][agregadosidx]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}
							}
							break;
							case history_s:
							{
								if (UD["history"].size()>0)
								{
									capBuffer(UD["history"][histchapter]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}

							}
							break;

							case favorites_s:
							{
								if ((int)UD["favoritos"].size() >= 1 ) {
									capBuffer(UD["favoritos"][favchapter]);
									gFAV = true;
								}
							}
							break;
							case chapter_s:
								if(serverpront) {
									string tempurl = BD["com"]["ActualLink"].get<string>() + to_string(latest) + "/";
										string temp = tempurl;
										NameOfLink(temp);
										string text = "Cargando "+temp.substr(0,42)+"... desde "+arrayservers[selectserver]+" ...";
										GOD.PleaseWait(text);
									if(!onlinejkanimevideo(tempurl,arrayservers[selectserver])) {
										arrayservers.erase(arrayservers.begin()+selectserver);
									} else {
										serverpront = false;
										UD["chapter"][KeyName]["latest"] = latest;
										latestcolor = latest;

										string item=BD["com"]["ActualLink"].get<string>();
										int hsize = UD["history"].size();
										if (hsize > 49) {UD["history"].erase(UD["history"].end());}//limit history
										if (hsize > 0) {
											UD["history"] = eraseVec(UD["history"],item);
											//UD["history"].erase(remove(UD["history"].begin(), UD["history"].end(), item), UD["history"].end());
											UD["history"].insert(UD["history"].begin(),tempurl);
										} else {
											UD["history"].push_back(tempurl);
										}
										write_DB(UD,rootsave+"UserData.json");
									}
								} else {
									if (isConnected) serverpront = true;
									T_N.TickerBomb();
								}
								break;
							}
						}
						if (e.jbutton.button == BT_P) {// (+) button down close to home menu
							cancelcurl = 1;
							quit = true;
						}
						if (e.jbutton.button == BT_M) {// (-) button down
							GOD.SwapMusic();
						}
						if (e.jbutton.button == BT_L) {// (L) button down
							if (statenow == chapter_s) {
								if(!AB["AnimeBase"][KeyName]["Precuela"].is_null()) {
									if (!serverpront) {
										capBuffer(AB["AnimeBase"][KeyName]["Precuela"]);
										gFAV = isFavorite(BD["com"]["ActualLink"]);
									} else {
										serverpront=false;
									}
								}
							}

							if (statenow == programation_s)
							{
								callAflv();
							}
							if (statenow == hourglass_s){
								if (WdayG > 0){
									WdayG--;
									BD["arrays"]["HourGlass"]["link"] = BD["arrays"]["HourGlass"][Wday[WdayG]];
								}
							}
							if (statenow == menu_s)
							{
								Btimer=false;
								bannersel--;
								if (bannersel<0)bannersel=bannersize;
							}
						}
						if (e.jbutton.button == BT_ZL) {// (ZL) button down
							switch (statenow)
							{
							case programation_s:
								WebBrowserCall(urlc,true);
								break;
							case chapter_s:
							case menu_s:
								GOD.selectskin();
								break;
							}
						}
						if (e.jbutton.button == BT_ZR) {// (ZR) button down
							if(isDownloading && isHandheld) {
								lcdoff = !lcdoff;
								appletSetLcdBacklightOffEnabled(lcdoff);
							}

							switch (statenow) {
							case programation_s:
							case menu_s:
							case chapter_s:
							case search_s:
							case favorites_s:
							case history_s:
							case hourglass_s:
							case top_s:
							case agregados_s:
							case themas_s:
								PlayerGet(acc);
								break;
							}
						}
						if (e.jbutton.button == BT_B) {// (B) button down

							switch (statenow)
							{
							case download_s:
								statenow = chapter_s;
								if (porcendown >= 100) led_on(0);
								break;
							case chapter_s:
								if(serverpront) {
									serverpront=false;
									arrayservers=arrayserversbak;
								} else {
									statenow=returnnow;
								}
								break;
							case search_s:
								if (!reloadingsearch)
								{
									returnnow = programation_s;
									statenow = programation_s;
								}
								break;

							case top_s:
							case agregados_s:
							case hourglass_s:
							case history_s:
							case favorites_s:
							case menu_s:
								returnnow = programation_s;
								statenow = programation_s;
								break;
							}
						}
						if (e.jbutton.button == BT_X) {// (X) button down
							switch (statenow)
							{
							case programation_s:
								if(isDownloading)
									statenow = download_s;
								break;
							case download_s:
								cancelcurl = 1;
								break;
							case chapter_s:
								if (!isConnected) break;
								statenow = download_s;
								cancelcurl = 0;
//							GOD.PleaseWait("Calculando Links Espere...");
								urltodownload  = BD["com"]["ActualLink"].get<string>() + to_string(latest) + "/";
								if(isDownloading) {
									bool gogo = false;
									for (u64 x=0; x < BD["arrays"]["downloads"]["queue"].size(); x++) {
										if (BD["arrays"]["downloads"]["queue"][x] == urltodownload) gogo = true;
									}
									if(gogo) break;
									BD["arrays"]["downloads"]["queue"].push_back(urltodownload);
									BD["arrays"]["downloads"]["log"].push_back(urltodownload);
								}else{
									BD["arrays"]["downloads"]["queue"].clear();
									BD["arrays"]["downloads"]["queue"].push_back(urltodownload);
									BD["arrays"]["downloads"]["log"] = BD["arrays"]["downloads"]["queue"];
									downloadthread = SDL_CreateThread(downloadjkanimevideo, "jkthread", (void*)NULL);
								}
								break;
							case search_s:

								break;
							case favorites_s:
								delFavorite(favchapter);
								if (favchapter > 0) favchapter--;
								break;

							}
						}
						if (e.jbutton.button == BT_Y) {// (Y) button down

							switch (statenow)
							{
							case programation_s:
								callmenuslide();
								break;
							case menu_s:
								callfavs();
								break;
							/*
							        if (!reloading)
							        {
							                getFavorite();
							                returnnow = favorites_s;
							                statenow = favorites_s;
							                Frames=1;
							        }
							        break;
							 */
							case download_s:
								break;
							case chapter_s:
							{//AGREGAR A FAVORITOS
								addFavorite(BD["com"]["ActualLink"]);
								gFAV = true;
								FAV.TickerBomb();
							}

							break;
							case favorites_s:

								break;




							}
						}
						if (e.jbutton.button == BT_R3) {// (R3) button down
							switch (statenow)
							{//only for test
							case chapter_s:{
								string tempurl = BD["com"]["ActualLink"].get<string>();
								if(serverpront) {
									tempurl = BD["com"]["ActualLink"].get<string>() + to_string(latest) + "/";
								}
								WebBrowserCall(tempurl);
								break;
							}
							case programation_s:
							case search_s:
							case favorites_s:
								ongrid = !ongrid;
								break;
							}
						}
						if (e.jbutton.button == BT_L3) {// (L3) button down
							switch (statenow)
							{
								//Change UI
							}

						}
						if (e.jbutton.button == BT_R) {// (R) button down
							switch (statenow)
							{
							case chapter_s:
								if(!AB["AnimeBase"][KeyName]["Secuela"].is_null()&&!serverpront) {
									if (!serverpront) {
										capBuffer(AB["AnimeBase"][KeyName]["Secuela"]);
										gFAV = isFavorite(BD["com"]["ActualLink"]);
									} else {
										serverpront=false;
									}
								}
								break;
							case programation_s:
							case search_s:
								callsearch();
								break;

							}
							if (statenow == hourglass_s){
								if (WdayG < 7){
									WdayG++;
									BD["arrays"]["HourGlass"]["link"] = BD["arrays"]["HourGlass"][Wday[WdayG]];
								}
							}

							if (statenow == menu_s)
							{
								Btimer=false;
								bannersel++;
								if (bannersel>bannersize)bannersel=0;
							}

						}
						if (e.jbutton.button == BT_LEFT || e.jbutton.button == BT_LS_LEFT) {// (left) button down
							switch (statenow)
							{
							case chapter_s:
								if(serverpront) {serverpront = false;}
								if (latest > mincapit)
								{
									latest--;
								}
								if (latest < mincapit)
								{
									latest = mincapit;
								}
								break;
							}
						}
						if (e.jbutton.button == BT_RIGHT || e.jbutton.button == BT_LS_RIGHT) {// (right) button down
							switch (statenow)
							{
							case menu_s:
								if (BD["arrays"]["Banner"]["link"].size()>0)
								{
									returnnow = menu_s;
									capBuffer(BD["arrays"]["Banner"]["link"][bannersel]);
									gFAV = isFavorite(BD["com"]["ActualLink"]);
								}
								break;
							case chapter_s:
								if(serverpront) {serverpront = false;}
								if (latest < maxcapit)
								{
									latest++;
								}
								if (latest > maxcapit)
								{
									latest = maxcapit;
								}
								break;
							}
						}
						if (e.jbutton.button == BT_UP || e.jbutton.button == BT_LS_UP) {// (up) button down
							switch (statenow)
							{
							case chapter_s:
								if(!serverpront) {//selectserver
									if (latest < maxcapit)
									{
										latest = latest + 10;
									}
									if (latest > maxcapit)
									{
										latest = maxcapit;
									}
								} else {
									if (selectserver > 0)
									{
										selectserver--;
									}
								}
								break;
							}
						}
						if (e.jbutton.button == BT_DOWN || e.jbutton.button == BT_LS_DOWN) {// (down) button down
							switch (statenow)
							{
							case chapter_s:
								if(!serverpront) {//selectserver
									if (latest > 1)
									{
										latest = latest - 10;
									}
									if (latest < mincapit)
									{
										latest = mincapit;
									}
								} else {
									if (selectserver < (int)arrayservers.size()-1)
									{
										selectserver++;
									}
								}
								break;
							}
						}
						
						//Global HID
						switch (e.jbutton.button){
							case BT_LEFT:
							case BT_UP:
							case BT_RIGHT:
							case BT_DOWN:
							case BT_LS_LEFT:
							case BT_LS_UP:
							case BT_LS_RIGHT:
							case BT_LS_DOWN:
							{
								switch (statenow)
								{
								case programation_s:
									if (!reloading) GOD.HandleList(selectchapter, BD["arrays"]["chapter"]["link"].size(), e.jbutton.button, ongrid);
									break;
								case search_s:
									GOD.HandleList(searchchapter, BD["arrays"]["search"]["link"].size(), e.jbutton.button, ongrid);
									break;
								case top_s:
									GOD.HandleList(topchapter, BD["arrays"]["Top"]["link"].size(), e.jbutton.button, ongrid);
									break;
								case agregados_s:
									GOD.HandleList(agregadosidx, BD["arrays"]["Agregados"]["link"].size(), e.jbutton.button, ongrid);
									break;
								case hourglass_s:
									GOD.HandleList(hourchapter, BD["arrays"]["HourGlass"]["link"].size(), e.jbutton.button, ongrid);
									break;
								case history_s:
									GOD.HandleList(histchapter, UD["history"].size(), e.jbutton.button, ongrid);
									break;
								case favorites_s:
									GOD.HandleList(favchapter, UD["favoritos"].size(), e.jbutton.button, ongrid);
									break;
								case menu_s:
									GOD.HandleList(selectelement, StatesList.size(), e.jbutton.button, false);
									break;
								}
							}
							break;
						}
						//
					}
					break;
				default:
					break;
				}

			}
			GOD.GenState = statenow;
			//Clear screen
			SDL_SetRenderDrawColor(GOD.gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
			SDL_RenderClear(GOD.gRenderer);

			//wallpaper
			Farest.render((0), (0));

			//Draw footer
			VOX.render_VOX({0,671, 1280, 50}, 210, 210, 210, 115);

			//render states
			switch (statenow)
			{
			case chapter_s:                   {
				//Draw a background to a nice view
				USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);
				VOX.render_VOX({0,0, SCREEN_WIDTH, 670},170, 170, 170, 100);
				VOX.render_VOX({0,0, 1280, 60},200, 200, 200, 130);
				GOD.HR=200; GOD.HG=200; GOD.HB=200;
				string temptext = BD["com"]["ActualLink"];
				NameOfLink(temptext);

				/*
				   //warning , only display in sxos ToDo
				   gTextTexture.loadFromRenderedText(GOD.Comic_16, "(*En SXOS desactiva Stealth Mode*)", textColor);
				   gTextTexture.render(posxbase, 0 );
				 */

				//draw Title
				gTextTexture.loadFromRenderedText(GOD.Arista_40, temptext.substr(0,62)+ ":", textColor);
				gTextTexture.render(posxbase, posybase);

				{//draw preview image
					VOX.render_VOX({ SCREEN_WIDTH - 412,63, 404, 590}, 0, 0, 0, 200);
					GOD.Image(BD["com"]["ActualLink"],SCREEN_WIDTH - 410, 65,400, 550,BT_B);
				}

				{//draw description
					int XG=10,YG=63,WG=850,HG=50;
					static string rese_prot = "..";
					string rese_p = BD["com"]["sinopsis"];
					if (rese_prot != rese_p) {//load texture on text change
						T_R.loadFromRenderedTextWrap(GOD.Comic_16, rese_p, textColor, WG-20);
						rese_prot = rese_p;
						//cout << "desc:" << T_R.getHeight()+60 << endl;
					}
					HG = T_R.getHeight()+60;
					HG = HG < 340 ? 340 : HG;
					
					VOX.render_VOX({XG, YG, WG, HG}, 255, 255, 255, 170);
					T_R.render(XG+10, YG+10);

					gTextTexture.loadFromRenderedTextWrap(GOD.Comic_16, BD["com"]["Emitido"], textColor,WG-20);
					gTextTexture.render(XG+10, YG+HG-(gTextTexture.getHeight()*2)-5);

					gTextTexture.loadFromRenderedTextWrap(GOD.Comic_16, BD["com"]["generos"], textColor,WG-20);
					gTextTexture.render(XG+10, YG+HG-gTextTexture.getHeight()-5);
				}
				bool anend=false;
				int sizefix = 0;
				if (maxcapit >= 0) {
					if (BD["com"]["nextdate"] == "Pelicula") {
						gTextTexture.loadFromRenderedText(GOD.Arista_40, "Pelicula", { 250,250,250 });
						gTextTexture.render(posxbase + 855, posybase + 598);
					} else {
						if (BD["com"]["enemision"] == "true")
						{
							gTextTexture.loadFromRenderedText(GOD.Arista_40, "En Emisión ", { 16,191,0 });
							gTextTexture.render(posxbase + 855, posybase + 598);
						} else {
							gTextTexture.loadFromRenderedText(GOD.Arista_40, "Concluido", { 140,0,0 });
							gTextTexture.render(posxbase + 855, posybase + 598);
						}
						gTextTexture.loadFromRenderedText(GOD.Comic_16, BD["com"]["nextdate"], { 255,255,255 });
						gTextTexture.render(posxbase + 1040, posybase + 615);
					}
				}

				{//use this to move the element
					int XS=250, YS =0;
					if (maxcapit >= 0) {
						int mwide = 35;//52
						int XD = 210+XS, YD = 582+YS;
						sizefix = (int)arrayservers.size() * mwide;
						anend=VOX.render_AH(XD, YD, 190, sizefix, serverpront);
						if(serverpront) {
							if (anend) {
								for (int x = 0; x < (int)arrayservers.size(); x++) {
									if (x == selectserver) {
										T_T.loadFromRenderedText(GOD.Arista_30, arrayservers[x], textWhite);
										VOX.render_VOX({ posxbase+XD-10,YD + 5 - sizefix + (x * mwide), 170, T_T.getHeight()-5}, 50, 50, 50, 200);
										T_T.render(posxbase+XD, YD - sizefix + (x * mwide));
									} else {
										gTextTexture.loadFromRenderedText(GOD.Arista_30, arrayservers[x],textGray);
										gTextTexture.render(posxbase+XD, YD - sizefix + (x * mwide));
									}
								}
							}
						}
					}
					if(serverpront) {
						if (anend) {
							B_UP.render_T(280+XS, 530+YS-sizefix,"");
						}
						B_DOWN.render_T(280+XS, 630+YS,"");
					}
					if (maxcapit >= 0&&BD["com"]["nextdate"] != "Pelicula") {//draw caps numbers Slider
						VOX.render_VOX({posxbase + 70+XS, posybase + 571+YS, 420, 33 }, 50, 50, 50, 200);
						SDL_Color com = {};
						if (latest-2 >= mincapit) {
							com=textGray;
							if (latest-2 == latestcolor) com=textGrayGreen;
							gTextTexture.loadFromRenderedText(GOD.AF_35,  to_string(latest-2), com);
							gTextTexture.render(posxbase + 150 +XS-gTextTexture.getWidth()/2, posybase + 565+YS);
						}
						if (latest-1 >= mincapit) {
							com=textGray;
							if (latest-1 == latestcolor) com=textGrayGreen;
							gTextTexture.loadFromRenderedText(GOD.AF_35,  to_string(latest-1), com);
							gTextTexture.render(posxbase + 215+XS-gTextTexture.getWidth()/2, posybase + 565+YS);
						}
						{
							com=textBlue;
							if (latest == latestcolor) com=textWhiteGreen;
							if (serverpront) {
								T_N.loadFromRenderedText(GOD.AF_35, to_string(latest), com);
								T_N.render(posxbase + 280+XS-T_N.getWidth()/2, posybase + 565+YS);
							} else {
								T_T.loadFromRenderedText(GOD.AF_35, to_string(latest), com);
								T_T.render(posxbase + 280+XS-T_T.getWidth()/2, posybase + 565+YS);
							}
						}

						if (latest+1 <= maxcapit) {
							com=textGray;
							if (latest+1 == latestcolor) com=textGrayGreen;
							gTextTexture.loadFromRenderedText(GOD.AF_35,  to_string(latest+1), com);
							gTextTexture.render(posxbase + 345+XS-gTextTexture.getWidth()/2, posybase + 565+YS);
						}
						if (latest+2 <= maxcapit) {
							com=textGray;
							if (latest+2 == latestcolor) com=textGrayGreen;
							gTextTexture.loadFromRenderedText(GOD.AF_35,  to_string(latest+2), com);
							gTextTexture.render(posxbase + 410+XS-gTextTexture.getWidth()/2, posybase + 565+YS);
						}

						if (maxcapit >= 10 && !serverpront) {
							B_UP.render_T(280+XS, 530+YS,"+10",serverpront);
							B_DOWN.render_T(280+XS, 630+YS,"-10",serverpront);
						}

						B_LEFT.render_T(75+XS, 580+YS,to_string(mincapit),latest == mincapit);
						B_RIGHT.render_T(485+XS, 580+YS,to_string(maxcapit),latest == maxcapit);
					} else {
						VOX.render_VOX({posxbase + 185+XS, posybase + 570+YS, 200, 35 }, 50, 50, 50, 200);
						if (BD["com"]["nextdate"] == "Pelicula" || mincapit == maxcapit) {
							T_T.loadFromRenderedText(GOD.Arista_40, "Reproducir...", { 255, 255, 255 });
							T_T.render(posxbase + 282+XS-T_T.getWidth()/2, posybase + 558+YS);
						} else {
							gTextTexture.loadFromRenderedText(GOD.Arista_40, "Cargando...", { 255, 255, 255 });
							gTextTexture.render(posxbase + 282+XS-gTextTexture.getWidth()/2, posybase + 558+YS);
						}

					}
				}

				//Draw Footer Buttons
				int dist = 1095,posdist = 160;
				if(serverpront) {
					B_A.render_T(dist, 680,"Ver Online"); dist -= posdist;
					B_B.render_T(dist, 680,"Cerrar"); dist -= posdist;
				} else {
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Atrás"); dist -= posdist;
					B_X.render_T(dist, 680,"Descargar"); dist -= posdist;
				}

				if(gFAV) {
					FAV.render(1225, 70);
				} else {
					NFAV.render_T(1225, 70,"");
					B_Y.render_T(dist, 680,"Favorito"); dist -= posdist;
				}

				if(!AB["AnimeBase"][KeyName]["Secuela"].is_null()) {
					string imagelocal=AB["AnimeBase"][KeyName]["Secuela"];
					imagelocal = KeyOfLink(imagelocal);
					imagelocal = rootdirectory+"DATA/"+imagelocal+".jpg";
					if(!serverpront) {CheckImgNet(imagelocal); B_R.render_T(dist, 680,"Secuela"); dist -= posdist;}
					GOD.Cover(imagelocal,160,456,"Secuela",120,BT_R);
				}
				if(!AB["AnimeBase"][KeyName]["Precuela"].is_null()) {
					string imagelocal=AB["AnimeBase"][KeyName]["Precuela"];
					imagelocal = KeyOfLink(imagelocal);
					imagelocal = rootdirectory+"DATA/"+imagelocal+".jpg";
					if(!serverpront) {CheckImgNet(imagelocal); B_L.render_T(dist, 680,"Precuela"); dist -= posdist;}
					GOD.Cover(imagelocal,10,456,"Precuela",120,BT_L);
				}

				break;
			}
			case menu_s:
			case programation_s:      {
				if (!reloading&&BD["arrays"]["chapter"]["link"].size()>=1) {

					if(ongrid) {
						USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);
						if (GOD.fingermotion_LEFT & (GOD.TouchX > 1100)) {
							GOD.fingermotion_LEFT = false;
							callmenuslide();
						}
						if (GOD.fingermotion_RIGHT & (GOD.TouchX > 850)) {
							GOD.fingermotion_RIGHT = false;
							statenow=programation_s;
						}
						GOD.HR=200; GOD.HG=200; GOD.HB=200;
						if (preview)
						{
							GOD.ListCover(selectchapter,BD["arrays"]["chapter"],ongrid,Frames);
							if (isHandheld && statenow!=menu_s) {
								FAVB.render(SCREEN_WIDTH - USER.getWidth() - FAVB.getWidth() - 20, 1);
								HISB.render(SCREEN_WIDTH - USER.getWidth() - FAVB.getWidth() - BUS.getWidth() - 50, 1);
								BUSB.render(SCREEN_WIDTH - USER.getWidth() - FAVB.getWidth() - BUS.getWidth() - HISB.getWidth() - 70, 1);
							}
						} else {
							VOX.render_VOX({ 0,14, 50, 46}, 255, 255, 255, 200);
						}
						double angle = 0.0;
						static int Ticker=0;
						if (isChained) {
							REC.TickerRotate(Ticker,0,360,2,isChained);
							angle=Ticker;
						} else if (Ticker > 0) {
							REC.TickerRotate(Ticker,-25,360,5,isChained);
							angle=Ticker;
						}else if (Ticker < 0) {
							Ticker=0;
						}

						if (part > 0) {
							gTextTexture.loadFromRenderedText(GOD.digi_9, to_string(ofall - part), {50,50,50});
							gTextTexture.render(27 - (gTextTexture.getWidth()/2), 30);
						}
						REC.render(5, 15,NULL,angle);
					} else {
						GOD.ListClassic(selectchapter,BD["arrays"]["chapter"]);
						if (preview)
						{
							GOD.ListCover(selectchapter,BD["arrays"]["chapter"]);
						}
						B_UP.render_T(580, 5,"");
						B_DOWN.render_T(580, 630,"");
					}

					//Draw Header
					gTextTexture.loadFromRenderedText(GOD.digi_11, ("(Ver "+DInfo()["App"].get<string>()+") #KASTXUPALO").c_str(), {100,0,0});
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 3, 672);

					gTextTexture.loadFromRenderedText(GOD.Comic_16, "Recientes", {100,0,0});
					if(ongrid) {
						gTextTexture.render(5, 1);
					}else {
						gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
						if (imgNumbuffer > 0) {
							gTextTexture.loadFromRenderedText(GOD.Comic_16, "Imágenes: ("+to_string(imgNumbuffer)+"/30)", {0,100,0});
							gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15, 22);
							//Heart.render(posxbase + 570, posybase + 3 + (imgNumbuffer-1) * 22);
						}
						if (part > 0) {
							gTextTexture.loadFromRenderedText(GOD.Comic_16, "Búfer: ("+to_string(part)+"/"+to_string(ofall)+")", {0,100,0});
							gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15, 22);
						}
					}
				} else {
					string textpro="Cargando programación";
					if(imgNumbuffer>0) {textpro+=" "+to_string(imgNumbuffer)+"/30";} else {textpro+="...";}
					GOD.PleaseWait(textpro,false);
				}
					if (statenow==menu_s) {

						{
							StatesList= {"Búsqueda","Historial","Favoritos","En Emisión","Top Anime","Nuevos","AnimeFLV"};
							if(isDownloading) {StatesList.push_back("Descargas");}
							
							int mwide = 60,XD=940,YD=120,W=1280-XD;
							VOX.render_VOX({XD,61, 1280, 608}, 160, 160, 160, 220);//draw area
							VOX.render_VOX({XD,61, W, 1}, 255, 255, 255, 235);//head line
							VOX.render_VOX({XD,668, W, 1}, 255, 255, 255, 235);//bottom line
							VOX.render_VOX({XD,61, 1, 607}, 255, 255, 255, 235);//line left
							gTextTexture.loadFromRenderedText(GOD.Arista_20, "Menú Primario",textColor);
							gTextTexture.render(XD+20, 65);
							if (imgNumbuffer > 0) {
								gTextTexture.loadFromRenderedText(GOD.Comic_16, "Imágenes: ("+to_string(imgNumbuffer)+"/30)", {0,100,0});
								gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15, 70);
							}
							if (part > 0) {
								gTextTexture.loadFromRenderedText(GOD.Comic_16, "Búfer: ("+to_string(part)+"/"+to_string(ofall)+")", {0,100,0});
								gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15,  70);
							}
							if (porcentajebufferF > 0) {
								gTextTexture.loadFromRenderedText(GOD.Comic_16, "BúferFav: ("+to_string(porcentajebufferF)+"/"+to_string(porcentajebufferFF)+")", {0,100,0});
								gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15, 70);
							}
							int indexLsize = StatesList.size();
							int Ymaxsize = indexLsize*mwide;
							if(GOD.TouchX < 1280 && GOD.TouchY < Ymaxsize+YD && GOD.TouchY > YD && GOD.TouchX > XD+100) {
								int sel=(GOD.TouchY-YD) / mwide;
								VOX.render_VOX({XD+80-10,YD+(sel*mwide)+5, W-100, mwide-5}, 0, 0, 255, 235);
								if (sel >= 0 && sel < indexLsize) {
									selectelement = sel;
								}
							}

							for (int x = 0; x < indexLsize; x++) {
								if(x == 0) {BUSB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 1) {HISB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 2) {FAVB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 3) {HORB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 4) {TOPB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 5) {GOD.MapT["ULTB"].render(XD+10, YD + (x * mwide)+5);}
								if(x == 6) {AFLV.render(XD+10, YD + (x * mwide)+5);}
								if(x == 7) {DOWB.render(XD+10, YD + (x * mwide)+5);}
								if(x == 8) {DOWB.render(XD+10, YD + (x * mwide)+5);}

								if (x == selectelement) {
									T_T.loadFromRenderedText(GOD.Arista_50, StatesList[x], textWhite);
									VOX.render_VOX({XD+80-10,YD + (x * mwide)+5, W-100, T_T.getHeight()-5}, 50, 50, 50, 100);
									T_T.render(XD+80, YD + (x * mwide));
								} else {
									gTextTexture.loadFromRenderedText(GOD.Arista_50, StatesList[x],textColor);
									gTextTexture.render(XD+80, YD + (x * mwide));
								}

								if (x < indexLsize-1) {
									VOX.render_VOX({XD+80,YD + (x * mwide)+mwide+2, W-130, 1}, 255, 255, 255, 235);
								}
							}
						}
						{//Draw Banner
							int XF=10, YF=65, WF=760, HF=427;
							bannersize = BD["arrays"]["Banner"]["files"].size()-1;
							//clock cicle 5s
							bool makebomb = false;
							if (inTimeN(5001)) {
								if (Btimer){
									makebomb = true;
									bannersel++;
									if (bannersel>bannersize) bannersel=0;
								} else
									Btimer=true;
							}
							//string temptext = BD["arrays"]["Banner"]["link"][bannersel];
							//NameOfLink(temptext);
							string path = BD["arrays"]["Banner"]["files"][bannersel];
							string temptext = BD["arrays"]["Banner"]["name"][bannersel];
						
							VOX.render_VOX({XF-2, YF-2, WF+4, HF+4}, 255, 255, 255, 235);
							GOD.Image(path, XF, YF, WF, HF,BT_RIGHT);
							if (makebomb) {
								GOD.MapT[path.substr(25)].offboom_size=2;
								GOD.MapT[path.substr(25)].TickerBomb();
								makebomb = false;
							}
							
							VOX.render_VOX({XF, YF, WF, 30}, 255, 255, 255, 135);
							gTextTexture.loadFromRenderedText(GOD.Arista_30, temptext.substr(0,60)+ ":", textColor);
							gTextTexture.render(XF, YF-5);
							
							VOX.render_VOX({XF, YF+HF-45, 160, 45}, 255, 255, 255, 135);
							B_RIGHT.render_T(XF+5, YF+HF-40,"Ver Ahora");
							
							if (!UD["Themes"]["name"].is_null()){
								//Thema temporal
								static int WT = 190;
								VOX.render_VOX({XF, YF+HF+10, WT+15, 45}, 255, 255, 255, 185);
								WT = B_ZL.render_T(XF+5, YF+HF+15,"Tema: "+UD["Themes"]["name"].get<string>());
								
							}						
						}

						//Draw footer buttons
						int dist = 1100,posdist = 170;
						B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
						B_B.render_T(dist, 680,"Atrás"); dist -= posdist;
						if (isHandheld) {CLEAR.render_T(dist, 680,"Cache"); dist -= posdist;}
						break;
					}
					//Draw footer buttons
					int dist = 1100,posdist = 170;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_R.render_T(dist, 680,"Buscar"); dist -= posdist;
					//B_L.render_T(dist, 680,"AnimeFLV");dist -= posdist;
					B_Y.render_T(dist, 680,"Menú"); dist -= posdist;
					if(isDownloading) {B_X.render_T(dist, 680,"Descargas"); dist -= posdist-10;}


				break;
			}
			case search_s:                    {
				if (!reloadingsearch) {
					int srchsize=BD["arrays"]["search"]["link"].size();
					if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}
					if (srchsize > 0) {
						//if (srchsize > 30) ongrid=false;
						if (!ongrid) GOD.ListClassic(searchchapter,BD["arrays"]["search"]);
						if (preview)
						{
							if (ongrid) {
								VOX.TickerColor(GOD.HG,210,255, 200);
								GOD.HR=200; GOD.HB=200;
								GOD.ListCover(searchchapter,BD["arrays"]["search"],ongrid,Frames);
								BUS.render_T(5, 15,"");
								if (isHandheld) {
									BUSB.render(SCREEN_WIDTH - USER.getWidth() - FAVB.getWidth() - BUS.getWidth() - 50, 1);
								}
							} else {
								GOD.ListCover(searchchapter,BD["arrays"]["search"]);
								B_UP.render_T(580, 5,"");
								B_DOWN.render_T(580, 630,"");
							}
						}
					}else {
						NOP.render_T(230, 355,BD["searchtext"]);
					}
					//Draw Header
					gTextTexture.loadFromRenderedText(GOD.Comic_16, "Resultados de Búsqueda:", {100,0,0});
					if(ongrid) {
						int distan = gTextTexture.getWidth()+10;
						gTextTexture.render(5, 1);
						gTextTexture.loadFromRenderedText(GOD.Comic_16, BD["searchtext"], {0,0,0});
						VOX.render_VOX({distan-2,1, gTextTexture.getWidth()+4, gTextTexture.getHeight()}, 210, 210, 210, 155);
						//T_D.getWidth()+4, T_D.getHeight()
						gTextTexture.render(distan, 1);
					}else {
						gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
						gTextTexture.loadFromRenderedText(GOD.Comic_16, BD["searchtext"], {0,0,0});
						gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 15, 22);
					}
					{//Draw footer buttons
						int dist = 1100,posdist = 160;
						B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
						B_B.render_T(dist, 680,"Atras"); dist -= posdist;
						B_R.render_T(dist, 680,"Buscar"); dist -= posdist;
					}
				}
				else
				{
					string added="";
					if(porcentajebufferF > 0) {
						int persen = ((porcentajebufferF) * 100) / porcentajebufferFF;
						added=" ("+to_string(persen)+"%)";
					}
					GOD.PleaseWait("Cargando búsqueda..."+added,false);
					Frames=1;
				}
				break;
			}
			case favorites_s:         {
				json VecF;
				VecF["link"]=UD["favoritos"];
				if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}
				int favsize=UD["favoritos"].size();
				if (favsize > 0) {
					//if (favsize > 30) ongrid=false;
					if (!ongrid) GOD.ListClassic(favchapter,VecF);
					if (preview)
					{
						if (ongrid) {
							VOX.TickerColor(GOD.HR,210,255, 200);
							GOD.HG=200; GOD.HB=200;
							GOD.ListCover(favchapter,VecF,ongrid,Frames);
							FAV.render_T(5, 15,"");
						} else {
							GOD.ListCover(favchapter, VecF);
							B_UP.render_T(580, 5,"");
							B_DOWN.render_T(580, 630,"");
						}
					}
				}

				//Draw Header
				gTextTexture.loadFromRenderedText(GOD.Comic_16, "Favoritos", {100,0,0});
				if(ongrid) {
					gTextTexture.render(5, 1);
				}else {
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);

					if (porcentajebufferF > 0) {
						gTextTexture.loadFromRenderedText(GOD.Comic_16, "Búfer: ("+to_string(porcentajebufferF)+"/"+to_string(porcentajebufferFF)+")", {0,100,0});
						gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 30, 20);
					}
				}

				{//Draw footer buttons
					int dist = 1100,posdist = 180;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Volver"); dist -= posdist;
					if ((int)UD["favoritos"].size() >= 1) {
						B_X.render_T(dist, 680,"Borrar #"+to_string(favchapter+1)); dist -= posdist;
					}else NOP.render_T(230, 355,"");
				}
				break;
			}
			case history_s:           {
				if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}
				json VecF;
				VecF["link"]=UD["history"];

				int histsize=VecF["link"].size();
				if (histsize > 0) {
					//if (favsize > 30) ongrid=false;
					if (!ongrid) GOD.ListClassic(histchapter,VecF);
					if (preview)
					{
						if (ongrid) {
							VOX.TickerColor(GOD.HG,100,200, 200);
							GOD.HR=200; GOD.HB=200;
							GOD.ListCover(histchapter,VecF,ongrid,Frames);
							HIS.render_T(5, 15,"");
						} else {
							GOD.ListCover(histchapter,VecF);
							B_UP.render_T(580, 5,"");
							B_DOWN.render_T(580, 630,"");
						}
					}
				} else NOP.render_T(230, 355,"");

				//Draw Header
				gTextTexture.loadFromRenderedText(GOD.Comic_16, "history", {100,0,0});
				if(ongrid) {
					gTextTexture.render(5, 1);
				}else {
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
				}

				{//Draw footer buttons
					int dist = 1100,posdist = 180;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Volver"); dist -= posdist;
				}
				break;
			}
			case top_s:               {
				if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}
				int histsize=BD["arrays"]["Top"]["link"].size();
				if (histsize > 0) {
					//if (favsize > 30) ongrid=false;
					if (!ongrid) GOD.ListClassic(topchapter,BD["arrays"]["Top"]);
					if (preview)
					{
						if (ongrid) {
							VOX.TickerColor(GOD.HR,100,200, 200);
							GOD.HG=200; GOD.HB=200;
							GOD.ListCover(topchapter,BD["arrays"]["Top"],ongrid,Frames);
							TOP.render_T(5, 15,"");
						} else {
							GOD.ListCover(topchapter,BD["arrays"]["Top"]);
							B_UP.render_T(580, 5,"");
							B_DOWN.render_T(580, 630,"");
						}
					}
				} else NOP.render_T(230, 355,"");

				//Draw Header
				gTextTexture.loadFromRenderedText(GOD.Comic_16, "Top Anime", {100,0,0});
				if(ongrid) {
					gTextTexture.render(5, 1);
				}else {
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
				}

				{//Draw footer buttons
					int dist = 1100,posdist = 180;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Volver"); dist -= posdist;
				}
				break;
			}
			case agregados_s: {
				if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}
				int histsize=BD["arrays"]["Agregados"]["link"].size(); //BD["arrays"]["Agregados"]["link"]
				if (histsize > 0) {
					//if (favsize > 30) ongrid=false;
					if (!ongrid) GOD.ListClassic(agregadosidx,BD["arrays"]["Agregados"]);
					if (preview)
					{
						if (ongrid) {
							VOX.TickerColor(GOD.HB,50,200, 200);
							GOD.HG=100; GOD.HR=100;
							GOD.ListCover(agregadosidx,BD["arrays"]["Agregados"],ongrid,Frames);
							GOD.MapT["ULT"].render_T(5, 15,"");
						} else {
							GOD.ListCover(agregadosidx,BD["arrays"]["Agregados"]);
							B_UP.render_T(580, 5,"");
							B_DOWN.render_T(580, 630,"");
						}
					}
				} else NOP.render_T(230, 355,"");

				//Draw Header
				gTextTexture.loadFromRenderedText(GOD.Comic_16, "Ultimos Animes Agregados", {100,0,0});
				if(ongrid) {
					gTextTexture.render(5, 1);
				}else {
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
				}

				{//Draw footer buttons
					int dist = 1100,posdist = 180;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Volver"); dist -= posdist;
				}
				break;
			}
			case hourglass_s:              {
				if(ongrid) {USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);}

				int histsize=BD["arrays"]["HourGlass"]["link"].size();
				if (histsize > 0) {
					//if (favsize > 30) ongrid=false;
					if (!ongrid) GOD.ListClassic(hourchapter,BD["arrays"]["HourGlass"]);
					if (preview)
					{
						if (ongrid) {
							VOX.TickerColor(GOD.HB,100,200, 200);
							GOD.HR=200; GOD.HG=200;
							GOD.ListCover(hourchapter,BD["arrays"]["HourGlass"],ongrid,Frames);
							HOR.render_T(5, 15,"");
						} else {
							GOD.ListCover(hourchapter,BD["arrays"]["HourGlass"]);
							B_UP.render_T(580, 5,"");
							B_DOWN.render_T(580, 630,"");
						}
					}
				} else NOP.render_T(230, 355,"");

				//Draw Header
				gTextTexture.loadFromRenderedText(GOD.Comic_16, "En Emisión", {100,0,0});
				if(ongrid) {
					gTextTexture.render(5, 1);
				}else {
					gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 2);
				}

				{//Draw footer buttons
					int dist = 1100,posdist = 180;
					B_A.render_T(dist, 680,"Aceptar"); dist -= posdist;
					B_B.render_T(dist, 680,"Volver"); dist -= posdist;
					
					dist = B_L.render_T(550,680," "+Wday[WdayG]+" ");
					B_R.render_T(550+dist,680);
				}
				break;
			}
			case download_s:              {
				USER.render(SCREEN_WIDTH - USER.getWidth()-1,1);
				VOX.render_VOX({0,0, 1280, 60},200, 200, 200, 130); //Head
				VOX.render_VOX({16,65, 900, 162}, 210, 210, 210, 115);//Rectangle

				gTextTexture.loadFromRenderedText(GOD.Comic_16, "Descargando Actualmente:", textColor);
				gTextTexture.render(posxbase, posybase+15);

				gTextTexture.loadFromRenderedText(GOD.Arista_30, DownTitle, textColor);
				VOX.render_VOX({17,65, gTextTexture.getWidth()+15, 45}, 210, 210, 210, 155);//Draw title back
				gTextTexture.render(posxbase, posybase + 60);

				if (serverenlace != "Error de descarga") {
					gTextTexture.loadFromRenderedText(GOD.Arista_100, to_string(porcendown) + "\%", textColor);
					gTextTexture.render(posxbase + 280, posybase + 90);

					gTextTexture.loadFromRenderedText(GOD.Comic_16, "Peso estimado: " + to_string((int)(sizeestimated / 1000000)) + "mb.", textColor);
					gTextTexture.render(posxbase, posybase + 160);

					gTextTexture.loadFromRenderedText(GOD.Comic_16, "Usa el HomeBrew PPlay para reproducir el video.", textColor);
					gTextTexture.render(posxbase, posybase + 200);

					if (to_string(porcendown) == "100"&&!isDownloading) {
						//Render red filled quad
						VOX.render_VOX({ posxbase + 198, posybase + 500, 580, 50 }, 255, 255, 255, 195);
						gTextTexture.loadFromRenderedText(GOD.Arista_40, "¡Descarga Completada! Revisa tu SD.", textColor);
						gTextTexture.render(posxbase + 200, posybase + 500);
						if(lcdoff) {lcdoff=false; appletSetLcdBacklightOffEnabled(lcdoff);}
					}else{
						gTextTexture.loadFromRenderedText(GOD.digi_16, "Velocidad: " +speedD+" M/S", textColor);
						VOX.render_VOX({ posxbase, posybase + 180, gTextTexture.getWidth()+6, 20 }, 255, 255, 255, 145);
						gTextTexture.render(posxbase + 2, posybase + 180);
						if (isHandheld) {
							SCREEN.render(1180, 65);
							B_ZR.render_T(550, 680,"Apagar Pantalla");
						}
					}
				} else {
					porcendown=0;
				}

				gTextTexture.loadFromRenderedText(GOD.Comic_16, serverenlace.substr(0,300), {168,0,0});
				gTextTexture.render(posxbase, posybase + 220);

				static int tatic = 850;
				VOX.render_VOX({posxbase-5,posybase + 240, tatic, ((int)BD["arrays"]["downloads"]["log"].size() * 22)+33}, 200, 200, 200, 105);
				gTextTexture.loadFromRenderedText(GOD.digi_16, "Cola De Descarga :", textColor);
				gTextTexture.render(posxbase, posybase+240);
				for (u64 x = 0; x < BD["arrays"]["downloads"]["log"].size(); x++) {
					string descarga = BD["arrays"]["downloads"]["log"][x];
					//NameOfLink(descarga);
					SDL_Color txtColor = textColor;//{ 50, 50, 50 };

					if(descarga.substr(0,3) == "100") txtColor = { 0, 100, 0 };
					if(descarga.substr(0,3) == "Err") txtColor = { 150, 50, 50 };
					if(descarga.substr(0,3) == ">>>") {txtColor = { 0, 0, 0 }; replace(descarga,">>>>",">>"+to_string(porcendown)+"\%");}
					if(descarga.substr(0,3) == "htt") {txtColor = { 100, 100, 100}; NameOfLink(descarga); descarga="En Cola: "+descarga;}

					gTextTexture.loadFromRenderedText(GOD.digi_16, descarga, txtColor);
					if (tatic < gTextTexture.getWidth()) {tatic = gTextTexture.getWidth()+35;}
					gTextTexture.render(posxbase, posybase+260 + ((x) * 22));

				}

				if(isDownloading) B_X.render_T(800, 680,"Cancelar la descarga");
				B_B.render_T(1100, 680,"Volver");
				break;
			}
			}

			//global render
			if(isDownloading&& download_s != statenow) {
				T_D.loadFromRenderedText(GOD.digi_16, ""+DownTitle.substr(0,42)+"... ("+to_string(porcendown)+"\%)", {50,50,50});
				VOX.render_VOX({SCREEN_WIDTH - T_D.getWidth() - 2, 671-T_D.getHeight()+4, T_D.getWidth()+4, T_D.getHeight()-5}, 255, 255, 255, 180);
				T_D.render(SCREEN_WIDTH - T_D.getWidth() - 1, 671-T_D.getHeight());
			}
			if (AppletMode) GOD.PleaseWait("Esta App No funciona en Modo Applet. Instalando NSP, Espere...",false);
			
			//presice clock cicle 1s
			if (inTimeN(1000,0)) {
				if (Frames>0) {
					static int rest=0;
					printf("Frames %d - FPS: %d \r",Frames,Frames-rest);
					fflush(stdout);
					rest=Frames;
				}
				if (!isConnected) {isConnected=Net::HasConnection();}
			}
			
			//clock cicle 15s
			if (inTimeN(15000)) {
				isConnected=Net::HasConnection();
			}
			if (!isConnected) {
				gTextTexture.loadFromRenderedText(GOD.digi_16, "Sin Internet", {255,0,0});
				VOX.render_VOX({SCREEN_WIDTH - gTextTexture.getWidth() - 8,671-gTextTexture.getHeight(), gTextTexture.getWidth()+4, gTextTexture.getHeight()}, 0, 0, 0, 180);
				gTextTexture.render(SCREEN_WIDTH - gTextTexture.getWidth() - 5, 671-gTextTexture.getHeight() );
			}

			if(programation_s != statenow && isHandheld) {
				BACK.render(SCREEN_WIDTH - USER.getWidth() - BACK.getWidth() - 30, 1);
			}
			
			GOD.MapT["MUSIC"].render_T(10, 680,"",(Mix_PausedMusic() == 1 || Mix_PlayingMusic() == 0));
			
			if((programation_s == statenow && isHandheld)|quit) {
				GOD.MapT["EXIT"].render_T(80, 680,"",quit);
			}

			//Display the list
			if (!quit&&!reloading&&!AppletMode&&Frames>2) {preview = true;}

			if (Frames>1000) Frames=0;
			if (Frames>0) Frames++;
			//Update tik
			if (inTimeN(1260000)) {
				if (Net::HasConnection()) {
					if(!isChained) {
						cout << "Reloading Animes" << endl;
						//Set main Thread get images and descriptions
						Loaderthread = SDL_CreateThread(AnimeLoader, "Loaderthread", (void*)NULL);
					}
				}
			}
		}
	} catch(...) {
		hasError++;
		led_on(2);
		cout << "- Error Catched Main" << endl;
		GOD.PleaseWait("A ocurrido un error Critico la app se va a cerrar",true);
		cout << "com: " << BD["com"] << endl;
		write_DB(AB,rootdirectory+"AnimeBase.json.bak");
		write_DB(BD,rootdirectory+"DataBase.json.bak");
		write_DB(UD,rootdirectory+"UserData.json.bak");
		quit=true;
	}
	//Quit if loop break for no reason
	quit=true;
	cancelcurl=1;
	//clear allocate
	BD["com"] = "{}"_json;

	// write prettified JSON
	write_DB(BD,rootdirectory+"DataBase.json");
	write_DB(AB,rootdirectory+"AnimeBase.json");
//	write_DB(UD,rootsave+"UserData.json");

	if (NULL == capithread) {printf("capithread Not in use\n");} else {printf("capithread in use: %s\n", SDL_GetError()); SDL_WaitThread(capithread, NULL);}
	if (NULL == downloadthread) {printf("downloadthread Not in use\n");} else {printf("downloadthread in use: %s\n", SDL_GetError()); SDL_WaitThread(downloadthread, NULL);}
	if (NULL == searchthread) {printf("searchthread Not in use\n");} else {printf("searchthread in use: %s\n", SDL_GetError()); SDL_WaitThread(searchthread,NULL);}
	if (NULL == Loaderthread) {printf("Loaderthread Not in use\n");}else {printf("Loaderthread in use: %s\n", SDL_GetError()); SDL_WaitThread(Loaderthread, NULL);}
	if (AppletMode) {
		appletRequestLaunchApplication (0x05B9DB505ABBE000, NULL);
	}



	//Free loaded images
	gTextTexture.free();
	Farest.free();
	Heart.free();

	//end net before anything
	nifmExit();
	B_A.free();
	B_B.free();
	B_Y.free();
	B_X.free();
	B_L.free();
	B_R.free();
	B_ZR.free();
	B_M.free();
	B_P.free();
	B_RIGHT.free();
	B_LEFT.free();
	B_UP.free();
	B_DOWN.free();
	FAV.free();
	NOP.free();
	BUS.free();
	REC.free();
	USER.free();
	NFAV.free();
	CLEAR.free();
	SCREEN.free();

	//Free resources and close SDL
	GOD.deint();
	//LOG Save
	if (hasError > 0){
		cout << "Errors: " << hasError << endl;
		LOG::SaveFile();
	}
	
	accountExit();
	hidsysExit();
	socketExit();
	romfsExit();
	socketExit();

	
	//unmount and commit
	mount_theme("themes",false);
	fsdevCommitDevice("save");
	fsdevUnmountDevice("save");
	fsFsClose(&acc);

	fsdevCommitDevice("user");
	fsdevUnmountDevice("user");
	fsFsClose(&data);
	

	accountExit();
	appletUnlockExit ();
	//if (!isConnected) appletRequestToSleep();
	return 0;
}