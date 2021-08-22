#include "Networking.hpp"
#include "applet.hpp"
#include "utils.hpp"
#include "JKanime.hpp"
#include "SDLWork.hpp"

extern json BD;
extern int mincapit;
extern int maxcapit;
extern int latest;
extern std::string rootdirectory;
extern SDL_Thread* capithread;
extern bool gFAV;

extern LTexture gTextTexture;
extern LTexture Farest;
extern LTexture Heart;
extern int sizeportraity;
extern int sizeportraitx;

//img
extern bool reloading;
extern bool preview;
extern int selectchapter;
extern int imgNumbuffer;
extern int porcentajebuffer;
extern int porcentajebufferAll;
extern int porcentajebufferF;
extern int porcentajebufferFF;
extern bool quit;
extern bool isDownloading;
extern int porcendown;
extern int cancelcurl;
extern bool AppletMode;

extern std::string serverenlace;
extern std::string DownTitle;
extern std::string KeyName;

enum states { programationstate, downloadstate, chapterstate, searchstate, favoritesstate };
enum statesreturn { toprogramation, tosearch, tofavorite };
extern int statenow;
extern int returnnow;

extern bool reloadingsearch;
extern int Frames;

//Get info off chapter
void PushDirBuffer(std::string a,std::string name) {
	if(quit) return;

	//Sinopsis
	std::string terese = scrapElement(a, "<p rel=\"sinopsis\">","</p>");
	replace(terese, "<p rel=\"sinopsis\">", ""); replace(terese, "<br/>", ""); replace(terese, "&quot;", "");
	BD["DataBase"][name]["sinopsis"] = terese.substr(0,810);
	//std::cout << BD["DataBase"][name]["sinopsis"] << std::endl;
	
	//get image
	terese = scrapElement(a, "https://cdn.jkanime.net/assets/images/animes/image/");
	BD["DataBase"][name]["Image"] = terese;
	
	//Anime info
	terese = scrapElement(a, "<span>Tipo:","</li");
	replace(terese, "<span>Tipo:", ""); replace(terese, "</span> ", ""); replace(terese, "</span>", "");
	BD["DataBase"][name]["Tipo"] = terese;
	
	terese = scrapElement(a, "Idiomas:","</li");
	replace(terese, "Idiomas:", ""); replace(terese, "  ", " "); replace(terese, "</span> ", ""); replace(terese, "</span>", "");
	BD["DataBase"][name]["Idiomas"] = terese;
	
	terese = scrapElement(a, "Episodios:","</li");
	replace(terese, "Episodios:", ""); replace(terese, "</span> ", ""); replace(terese, "</span>", "");
	BD["DataBase"][name]["Episodios"] = terese;

	terese = scrapElement(a, "Duracion:","</li");
	replace(terese, "Duracion:", ""); replace(terese, "</span> ", ""); replace(terese, "</span>", "");
	BD["DataBase"][name]["Duracion"] = terese;

	terese = scrapElement(a, "Emitido:","</li");
	replace(terese, "Emitido:", ""); replace(terese, "</span> ", ""); replace(terese, "</span>", "");
	BD["DataBase"][name]["Emitido"] = terese;
	
	int bal1=0;
	bal1=a.find("Secuela");
	if (bal1 > 1){
		terese = scrapElement(a.substr(bal1), "https://jkanime.net/","");
		BD["DataBase"][name]["Secuela"] = terese;
	}
	
	bal1=a.find("Precuela");
	if (bal1 > 1){
		terese = scrapElement(a.substr(bal1), "https://jkanime.net/","");
		BD["DataBase"][name]["Precuela"] = terese;
	}
	
	//es una peli ?
	terese = "...";
	if ((int)a.find("<span>Tipo:</span> Pelicula</li>") != -1)
	{
		terese="Pelicula";
	} else {
		//find next date
		int re1 =0, re2=0;
		re1 = a.find("<b>Próximo episodio</b>");
		if(re1 > 1){
			re1 += 25;
			re2 = a.find("<i class", re1);
			terese = "."+a.substr(re1, re2 - re1);
			RemoveAccents(terese);
		}
	}
	BD["DataBase"][name]["nextdate"] = terese;
	
	//Generos 
	int indx1 = 1, indx2, indx3;
	indx1 = a.find("<span>Genero:</span>", indx1);
	terese="";
	while (indx1 != -1) {
		indx1 = a.find("https://jkanime.net/genero", indx1);
		if (indx1 == -1) { break; }
		indx2 = a.find(">",indx1);
		indx3 = a.find("</a>", indx1);
		terese += a.substr(indx2+1, indx3 - indx2-1)+", ";
		indx1 = indx3;
	}
	RemoveAccents(terese);

	if (terese.length() == 0) terese +="-.-";
	BD["DataBase"][name]["generos"] = terese;

	//Esta en emision?
	if ((int)a.find("En emision") != -1)
	{BD["DataBase"][name]["enemision"] = "true";} else {BD["DataBase"][name]["enemision"] = "false";}
	
	int val0, val1, val2, val3;
	val0 = a.rfind("href=\"#pag");
	if (val0 != -1) {
		val1 = a.find(">", val0);
		val1 = val1 + 1;
		val2 = a.find("<", val1);

		std::string urlx;
		urlx = (a.substr(val1, val2 - val1));
		val3 = urlx.find(" - ") + 3;
		urlx = urlx.substr(val3);

		BD["DataBase"][name]["maxcapit"] = atoi(urlx.c_str());
	} else {
		BD["DataBase"][name]["maxcapit"] = 1;
	}

	//empieza por 0?
	int zero1, zero2;
	std::string zerocontainer = "";
	std::string zerocontainer2 = "";
	zero1 = a.rfind("ajax/pagination_episodes/");
	zero2 = a.find("'", zero1);
	zerocontainer = "https://www.jkanime.net/" + a.substr(zero1, zero2 - zero1) + "/1/";
	//std::cout << zerocontainer << std::endl;
	zerocontainer2 = gethtml(zerocontainer);

	int tempzero = zerocontainer2.find("\"0\"");
	if (tempzero != -1) {
		BD["DataBase"][name]["maxcapit"] = BD["DataBase"][name]["maxcapit"].get<int>() - 1;
		BD["DataBase"][name]["mincapit"] = 0;
	}
	else {
		BD["DataBase"][name]["mincapit"] = 1;
	}
	BD["DataBase"][name]["TimeStamp"] = BD["TimeStamp"];
	std::cout << "Bufered: " << name << std::endl;
}
//Download THREAD
int downloadjkanimevideo(void* data) {
	led_on(1);
	appletSetAutoSleepDisabled(true);
	for (u64 x=0; x< BD["arrays"]["downloads"]["queue"].size();x++){
		DownTitle="................";
		serverenlace = "................";
		isDownloading=true;
		porcendown=0;
		if(cancelcurl){serverenlace = "Error de descarga"; break;}
		std::string urldown=BD["arrays"]["downloads"]["queue"][x];
		std::string namedownload = urldown;
		NameOfLink(namedownload);
		namedownload = namedownload.substr(0, namedownload.length() - 1);
		mayus(namedownload);
		DownTitle=namedownload;
		BD["arrays"]["downloads"]["log"][x] = ">>>> "+namedownload;
		mkdir("sdmc:/Videos",0777);
		std::string directorydownload = "sdmc:/Videos/" +namedownload + ".mp4";

		if (!linktodownoadjkanime(urldown,directorydownload)){
			serverenlace = "Error de descarga";
			BD["arrays"]["downloads"]["log"][x] = "Error: "+namedownload;
		} else {
			BD["arrays"]["downloads"]["log"][x] = "100% : "+namedownload;
		}
	}
	if(cancelcurl==0)led_on(3); else led_on(0);
	cancelcurl = 0;
	isDownloading=false;
	statenow = downloadstate;
	appletSetAutoSleepDisabled(false);
	return 0;
}

//BEGUING THREAD CHAIN
int refrescarpro(void* data){
	appletSetAutoSleepDisabled(true);
	//Wait for connection
	if (BD["arrays"]["chapter"]["link"].empty()){
		//hide the list for rebuild
		reloading = true;
		preview = false;
	}
	while (!HasConnection()){
		if (AppletMode) preview = false;
		SDL_Delay(3000);
		if (AppletMode) quit=true;
		if(quit) return 0;
	}
	
	if(!reloading){
		//Download All not existing images
		CheckImgVector(BD["arrays"]["chapter"]["images"],imgNumbuffer);
	}
	
	//Get main page
	std::string content = gethtml("https://jkanime.net");
	
	//Get Programation list, Links and Images
	int temp0=0,temp1=0;
	temp0=content.find("Programación");
	temp1=content.find("TOP ANIMES",temp0);
	content = content.substr(temp0,temp1-temp0);

	//rebuild list 
	std::vector<std::string> ChapLink=scrapElementAll(content, "https://jkanime.net/");
	std::vector<std::string> ChapImag=scrapElementAll(content, "https://cdn.jkanime.net/assets/images/");
	BD["arrays"]["chapter"]["date"]=scrapElementAll(content, "<span>","</span>");

	//Download All not existing images
	CheckImgVector(ChapImag,imgNumbuffer);
	std::cout << "# End Image Download\n" << std::endl;

	//Display List
	if(reloading) {Frames=0;reloading = false;}

	//'haschange' See if there is any new chap
	std::cout << "# I am Here1\n" << std::endl;
	bool haschange = true;
	if (!BD["latestchapter"].empty()){
	std::cout << "# I am Here2\n" << std::endl;
		if (BD["latestchapter"] == ChapLink[0])
		{
	std::cout << "# I am Here3\n" << std::endl;
			haschange = false;
		}
	}

	std::cout << "# I am Here4\n" << std::endl;
	//TimeStamp indicate if a chap sout be reloaded
	if (haschange || BD["TimeStamp"].empty()){
		//update TimeStamp
		if (BD["arrays"]["chapter"]["link"][0] != ChapLink[0] || BD["TimeStamp"].empty()){
			std::time_t t = std::time(0);
			BD["TimeStamp"] = std::to_string(t);
			std::cout << "New TimeStamp: " << BD["TimeStamp"] << std::endl;
			Frames=1;
			BD["arrays"]["chapter"]["images"]=ChapImag;
			BD["arrays"]["chapter"]["link"]=ChapLink;
		}
	}

	//Download All not existing images of Favorites
	MKfavimgfix(true);

	//Load to cache all Programation Chaps
	if (haschange) {
		MKcapitBuffer(BD["arrays"]["chapter"]["link"], porcentajebuffer, porcentajebufferAll);
		if(!quit){
			BD["latestchapter"] = BD["arrays"]["chapter"]["link"][0];
		}
	}

	//Load to cache all Favorites Chaps
	MKfavimgfix(false);

	//extra vector
	std::vector<std::string> vec={};

	//load main
	content = gethtml("https://jkanime.net");
	replace(content, "\"https://jkanime.net/\"", "");
	vec=scrapElementAll(content, "https://jkanime.net/");
	sort( vec.begin(),vec.end());
	vec.erase(unique(vec.begin(),vec.end()),vec.end());
	BD["arrays"]["Main"]["link"]=vec;
	MKcapitBuffer(vec, porcentajebuffer, porcentajebufferAll);

	
	//load Top
	content=gethtml("https://jkanime.net/top/");
	replace(content,"https://jkanime.net/top/","");
	replace(content,"https://jkanime.net///","");
	vec=scrapElementAll(content,"https://jkanime.net/");
	sort(vec.begin(),vec.end());
	vec.erase(unique(vec.begin(),vec.end()),vec.end());
	BD["arrays"]["Top"]["link"]=vec;
	MKcapitBuffer(vec, porcentajebuffer, porcentajebufferAll);

	//load Horario
	content=gethtml("https://jkanime.net/horario/");
	replace(content,"https://jkanime.net/horario/","");
	vec=scrapElementAll(content,"https://jkanime.net/");
	sort(vec.begin(),vec.end());
	vec.erase(unique(vec.begin(),vec.end()),vec.end());
	BD["arrays"]["HourGlass"]["link"]=vec;
	MKcapitBuffer(vec, porcentajebuffer, porcentajebufferAll);

	std::cout << "# End Thread   \n" << std::endl;
	if (!isDownloading) appletSetAutoSleepDisabled(false);
	//exit after load the cache if are in applet mode
	if (AppletMode) quit=true;
	return 0;
}
int MKcapitBuffer(std::vector<std::string> LinkList,int& part, int& ofall) {
	bool hasmchap=false;
	std::string a = "";
	part=0;
	ofall=LinkList.size();
	for (int x = 0; x < ofall&& !quit; x++)
	{
		if(hasmchap) part = x+1;
		while (!HasConnection()){SDL_Delay(2000);if(quit) return 0;}
		std::string link = LinkList[x];
		int trace = link.find("/", 20);
		link = link.substr(0, trace + 1);
		std::string name = link;
		replace(name, "https://jkanime.net/", "");
		replace(name, "/", "");

		if (BD["DataBase"][name]["TimeStamp"] != BD["TimeStamp"] ){
			if (BD["DataBase"][name]["enemision"]=="true" || BD["DataBase"][name]["TimeStamp"].empty()){
				part = x+1;
				a = gethtml(link);
				PushDirBuffer(a,name);
				hasmchap=true;
			} else {
				//BD["DataBase"][name]["TimeStamp"] = BD["TimeStamp"];
			}
		}
	}
	part=0;
	ofall=0;
	if(quit) return 0;
	if(hasmchap){
		//write json
		write_DB(BD,rootdirectory+"DataBase.json");
	}
	return true;
}
int MKfavimgfix(bool images){
	if(quit) return 0;
	bool hasanychange=false;
	if(BD["arrays"]["favorites"]["images"].empty() || BD["arrays"]["favorites"]["link"].empty()){
		printf("# Get fav list\n");
		getFavorite();
	}
	porcentajebufferFF = BD["arrays"]["favorites"]["link"].size();
	
	if (images) {
		CheckImgVector(BD["arrays"]["favorites"]["images"],porcentajebufferF);
		porcentajebufferFF=0;
	} else {
		for (int y=0; y < porcentajebufferFF && !quit; y++)
		{
			std::string name=BD["arrays"]["favorites"]["link"][y];
			replace(name, "https://jkanime.net/", "");
			replace(name, "/", "");
			porcentajebufferF=y+1;
			if (BD["DataBase"][name]["TimeStamp"].empty() || BD["DataBase"][name]["TimeStamp"] != BD["TimeStamp"]){
				std::string lingrt=BD["arrays"]["favorites"]["link"][y];
				replace(lingrt,"\"","");
				std::string a = gethtml(lingrt);
				PushDirBuffer(a,name);
				hasanychange=true;
			}
		}
		porcentajebufferF=0;
		porcentajebufferFF=0;
		printf("# End fav Download\n");
		if (hasanychange){//write json
			write_DB(BD,rootdirectory+"DataBase.json");
		}
	}
	return 0;
}
//END THREAD CHAIN

int searchjk(void* data) {
	BD["com"]["porcentajereload"] = 0;
	BD["com"]["porcentajereloadAll"]=0;
	BD["arrays"]["search"]["link"].clear();
	BD["arrays"]["search"]["images"].clear();
	BD["arrays"]["search"]["date"].clear();
	reloadingsearch = true;	 

	std::string texts = BD["searchtext"];
	replace(texts, " ", "_");
	replace(texts, "!", "");
	replace(texts, ";", "");
	if (texts.length() >= 2) {
		std::cout << texts << std::endl;
		std::string content = "";
		int page = 1;
		while (true){
			std::string tempCont=gethtml("https://jkanime.net/buscar/" + texts + "/"+std::to_string(page)+"/");
			content += tempCont;
			std::string scrap = scrapElement(tempCont, "Resultados Siguientes");
			std::cout << scrap << "  # " << std::to_string(page) << std::endl;
			if (scrap.length() > 0){
				//some code here soon
			} else {
				break;
			}
			page++;
		}
		
		int val0 = 0,val1 = 1,val2,val3, val4;
		while (val0 != -1) {
			val0 = content.find("<div class=\"anime__item\">", val1);
			if (val0 == -1) { break; }

			val1 = 6 + content.find("href=", val0);
			val2 = (content.find('"', val1));
			std::string gdrive = content.substr(val1, val2 - val1);
			BD["arrays"]["search"]["link"].push_back(gdrive);

			val3 = content.find("data-setbg=", val2) + 12;
			val4 = content.find('"', val3);
			std::string gsearchpreview = content.substr(val3, val4 - val3);
			BD["arrays"]["search"]["images"].push_back(gsearchpreview);
			//std::cout << gsearchpreview << std::endl;

			val3 = content.find("<p>", val4) + 3;
			val4 = content.find("</p>", val3);
			gsearchpreview = content.substr(val3, val4 - val3);
			RemoveAccents(gsearchpreview);
			if (gsearchpreview.length() > 120){
				gsearchpreview=gsearchpreview.substr(0,105)+"...";
			}
			BD["arrays"]["search"]["date"].push_back(gsearchpreview);
			val1++;
		}
		
		porcentajebufferFF=BD["arrays"]["search"]["images"].size();
		CheckImgVector(BD["arrays"]["search"]["images"],porcentajebufferF);
		porcentajebufferFF=0;		
	}
	else
	{
		statenow = programationstate;
		returnnow = toprogramation;
	}
	reloadingsearch = false;
	MKcapitBuffer(BD["arrays"]["search"]["link"], porcentajebuffer, porcentajebufferAll);
	return 0;
}

//get chap thread
int capit(void* data) {
	if (!HasConnection()) return 0;
	std::string bb = BD["com"]["ActualLink"];
	std::string a = "";
	a = gethtml(bb);
	std::string name = bb;
	replace(name, "https://jkanime.net/", ""); replace(name, "/", "");
	
	PushDirBuffer(a,name);
	try{
		//std::cout << BD << std::endl;
		BD["com"]["sinopsis"] = BD["DataBase"][name]["sinopsis"];
		BD["com"]["nextdate"] = BD["DataBase"][name]["nextdate"];//"......";
		BD["com"]["generos"] = BD["DataBase"][name]["generos"];//"......";
		BD["com"]["Emitido"] = BD["DataBase"][name]["Emitido"];
		BD["com"]["enemision"] = BD["DataBase"][name]["enemision"];
		mincapit = BD["DataBase"][name]["mincapit"];//1;
		maxcapit = BD["DataBase"][name]["maxcapit"];//-1;
		//write json
		write_DB(BD,rootdirectory+"DataBase.json");
		
		//Get Image
		std::string image = rootdirectory+"DATA/"+name+".jpg";
		if (!BD["DataBase"][name]["Image"].empty()){
			CheckImgNet(image,BD["DataBase"][name]["Image"]);
		}
	}catch(...){
		printf("Error \n");
	}
return 0;
}
//anime manager
int capBuffer (std::string Tlink) {
	int v2 = Tlink.find("/", 20);
	Tlink = Tlink.substr(0, v2 + 1);
	BD["com"]["ActualLink"] = Tlink;
	
	std::string name = Tlink;
	replace(name, "https://jkanime.net/", "");
	replace(name, "/", "");
	KeyName = name;
//	std::cout << "KeyName: " << name << std::endl;

	std::cout << "Link: " << Tlink << std::endl;
	
	std::string image = rootdirectory+"DATA/"+name+".jpg";
	statenow = chapterstate;

	if (BD["DataBase"][name]["TimeStamp"].empty())
	{
		BD["com"]["sinopsis"] = "......";
		BD["com"]["nextdate"] = "......";
		BD["com"]["generos"] = "......";
		BD["com"]["Emitido"] = "......";
		maxcapit = -1;
		mincapit = 1;
		latest = 1;
		capithread = SDL_CreateThread(capit, "capithread", (void*)NULL);
	} else {
		try{
			BD["com"]["sinopsis"] = BD["DataBase"][name]["sinopsis"];
			BD["com"]["nextdate"] = BD["DataBase"][name]["nextdate"];//"......";
			BD["com"]["generos"] = BD["DataBase"][name]["generos"];//"......";
			BD["com"]["Emitido"] = BD["DataBase"][name]["Emitido"];
			BD["com"]["enemision"] = BD["DataBase"][name]["enemision"];
			maxcapit = BD["DataBase"][name]["maxcapit"];
			mincapit = BD["DataBase"][name]["mincapit"];
			//check For latest cap seend
			if (BD["USER"]["chapter"][name]["latest"].empty()){
				//get position to the latest cap if in emision
				if (BD["com"]["enemision"] == "true"){
					latest = BD["DataBase"][name]["maxcapit"];//is in emision
				} else {
					latest = BD["DataBase"][name]["mincapit"];//is not in emision 
				}
			}
			else
				latest = BD["USER"]["chapter"][name]["latest"];

			//Get Image
			if (!BD["DataBase"][name]["Image"].empty()){
				CheckImgNet(image,BD["DataBase"][name]["Image"]);
			}

			if (BD["DataBase"][name]["TimeStamp"] != BD["TimeStamp"]){
				BD["com"]["nextdate"] = "Loading...";
				capithread = SDL_CreateThread(capit, "capithread", (void*)NULL);
			}
		}catch(...){
			printf("Error \n");
		}
	}
	CheckImgNet(image);
return 0;
}

void addFavorite(std::string text) {
/*
	BD["arrays"]["favorites"]["link"].clear();
	BD["arrays"]["favorites"]["images"].clear();
	std::string temp;
	std::ifstream file(rootdirectory+"favoritos.txt");
	std::string str;
	while (std::getline(file, str)) {
		std::string strtmp = str;
		if (str.find("jkanime"))
		{
			BD["arrays"]["favorites"]["link"].push_back(str);
			replace(strtmp, "https://jkanime.net/", "");
			replace(strtmp, "/", ".jpg");
			strtmp = "https://cdn.jkanime.net/assets/images/animes/image/"+strtmp;
			BD["arrays"]["favorites"]["images"].push_back(strtmp);				
		}
	}
	file.close();

*/
}
void getFavorite() {
	BD["arrays"]["favorites"]["link"].clear();
	BD["arrays"]["favorites"]["images"].clear();
	std::string temp;
	std::ifstream file(rootdirectory+"favoritos.txt");
	std::string str;
	while (std::getline(file, str)) {
		std::string strtmp = str;
		replace(str,"\"","");
		if (str.find("jkanime"))
		{
			BD["arrays"]["favorites"]["link"].push_back(str);
			replace(strtmp, "https://jkanime.net/", "");
			replace(strtmp, "/", ".jpg");
			strtmp = "https://cdn.jkanime.net/assets/images/animes/image/"+strtmp;
			BD["arrays"]["favorites"]["images"].push_back(strtmp);				
		}
	}
	file.close();
}
bool isFavorite(std::string fav){
	/*
	if(BD["arrays"]["favorites"]["images"].empty() || BD["arrays"]["favorites"]["link"].empty()){
		getFavorite();
	}
		static std::string limit = "";
		if (limit == fav){
			return gFAV;
		} else limit = fav;
	*/
	std::ifstream file(rootdirectory+"favoritos.txt");
	std::string str;
	while (std::getline(file, str)) {
	//	std::cout << str << "\n";
		if (str.find("jkanime"))
		{
//			printf("---\n");
//			printf("%s---\n",fav.c_str());
//			printf("%s\n",str.c_str());
			if (fav == str){file.close(); return true;}
		}
	}
file.close();
return false;
}
void delFavorite(int inst){
	std::string tmp="";
	if (inst >= 0){//delete all if is -1  delFavorite();
		std::ifstream file(rootdirectory+"favoritos.txt");
		std::string str = "";

		int index = 0;
		while (std::getline(file, str)) {
			if (inst != index){tmp += str+"\n";}
			index++;
		}
		file.close();	
	} else gFAV=false;
	std::ofstream outfile(rootdirectory+"favoritos.txt");
	outfile << tmp;
	outfile.close();
}