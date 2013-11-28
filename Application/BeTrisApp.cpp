/***************/
/* application */
/***************/
#include "BeTrisApp.h"
#include "BeTrisMessagesDefs.h"
#include "BeTrisDebug.h"
#include "CPreferenceFile.h"
#include "BeTrisNetwork.h"
#include "BeTrisPlayerView.h"

// acces aux vues
#include "BeTrisNetworkView.h"
#include "BeTrisPartyLineView.h"
#include "BeTrisWinnerView.h"
#include "BeTrisSettingView.h"
#include "BeTrisFieldsView.h"
#include "BeTrisNextBlockView.h"
#include "BeTrisOptionListView.h"
#include "BeTrisBlockFreqView.h"

#include <Entry.h>
#include <Roster.h>
#include <File.h>
#include <Bitmap.h>
#include <TranslationUtils.h>
#include <stdlib.h>
#include <time.h>
#include <PlaySound.h>
#include <Entry.h>

const char		*BETRIS_TETRINET_V114	= "1.14";
const char		*BETRIS_TETRINET_V113	= "1.13";
const char		*BETRIS_TETRIFAST_CODE	= "tetrifaster";
const char		*BETRIS_TETRINET_CODE	= "tetrisstart";

// structure des message recu
struct _cmd
{
    enum _cmd_type index;
    char *string;
};

struct _cmd _cmdtable[] = {
    {CMD_CONNECT, "connect"},
	{CMD_CONNECTED, "connected"},
    {CMD_DISCONNECT, "disconnect"},
    {CMD_CONNECTERROR, "noconnecting"},
    {CMD_PLAYERNUM, "playernum"},
    {CMD_PLAYERNUM_TFAST, ")#)(!@(*3"},
    {CMD_PLAYERJOIN, "playerjoin"},
    {CMD_PLAYERLEAVE, "playerleave"},
    {CMD_KICK, "kick"},
    {CMD_TEAM, "team"},
    {CMD_PLINE, "pline"},
    {CMD_PLINEACT, "plineact"},
    {CMD_PLAYERLOST, "playerlost"},
    {CMD_PLAYERWON, "playerwon"},
    {CMD_NEWGAME, "newgame"},
    {CMD_NEWGAME_TFAST, "*******"},
    {CMD_INGAME, "ingame"},
    {CMD_PAUSE, "pause"},
	{CMD_STARTGAME, "startgame"},
    {CMD_ENDGAME, "endgame"},
    {CMD_F, "f"},
    {CMD_SB, "sb"},
    {CMD_LVL, "lvl"},
    {CMD_GMSG, "gmsg"},
    {CMD_WINLIST, "winlist"},
    {CMD_SPECJOIN, "specjoin"},
    {CMD_SPECLEAVE, "specleave"},
    {CMD_SPECLIST, "speclist"},
    {CMD_SMSG, "smsg"},
    {CMD_SACT, "sact"},
    {CMD_BTRIXNEWGAME, "btrixnewgame"},
	{CMD_TETRISSTART, "tetrisstart"},
	{CMD_TETRIFASTER, "tetrifaster"},
	{CMD_VERSION, "version"},
	{CMD_CLIENTINFO, "clientinfo"},   
	{CMD_PING, "ping"},   
	{CMD_PONG, "pong"},   
    {CMD_NOENTRY,""}
};

// Acces au looper directement
BeTrisApp		*g_BeTrisApp = NULL;

// definitions des bloques
// la lecture de ces blocs est inverse par rapporta au parcour x,y des donnees
// les lignes sont les colonnes (y)
_betrisblock b1[2] = {{ {1,0,0,0},{1,0,0,0},{1,0,0,0},{1,0,0,0}},{{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}} };
_betrisblock b2[1] = {{ {0,0,0,0},{2,2,0,0},{2,2,0,0},{0,0,0,0}} };
_betrisblock b3[4] = {{ {0,0,0,0},{0,0,3,0},{3,3,3,0},{0,0,0,0}},{{0,0,0,0},{3,3,0,0},{0,3,0,0},{0,3,0,0}},{{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}},{{0,0,0,0},{3,0,0,0},{3,0,0,0},{3,3,0,0}} };
_betrisblock b4[4] = {{ {0,0,0,0},{4,4,4,0},{0,0,4,0},{0,0,0,0}},{{0,0,0,0},{4,4,0,0},{4,0,0,0},{4,0,0,0}},{{0,0,0,0},{4,0,0,0},{4,4,4,0},{0,0,0,0}},{{0,0,0,0},{0,4,0,0},{0,4,0,0},{4,4,0,0}} };
_betrisblock b5[2] = {{ {0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}},{{0,0,0,0},{5,0,0,0},{5,5,0,0},{0,5,0,0}} };
_betrisblock b6[2] = {{ {0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0}},{{0,0,0,0},{0,1,0,0},{1,1,0,0},{1,0,0,0}} };
_betrisblock b7[4] = {{ {0,0,0,0},{0,2,0,0},{2,2,2,0},{0,0,0,0}},{{0,0,0,0},{0,2,0,0},{2,2,0,0},{0,2,0,0}},{{0,0,0,0},{2,2,2,0},{0,2,0,0},{0,0,0,0}},{{0,0,0,0},{2,0,0,0},{2,2,0,0},{2,0,0,0}} };

#include <stdio.h>

/**** main ****/
int main()
{	
	BeTrisApp	myApplication;

	myApplication.Run();

	return(0);
}

/**** Constructeur ****/
BeTrisApp::BeTrisApp()
: BApplication(APPLICATION_SIGNATURE)
{
	BFile		file;
	BEntry		entry;
	app_info	info;
	BRect		windowRect;

	// initialiser les nombres aleatoires
	srand(time(0));

	// initialiser
	g_BeTrisApp = this;
	_bitmap = NULL;
	_pause = false;
	_gameStarted = false;
	_prefs = NULL;
	_prefsUser = NULL;
	_prefsNetwork = NULL;
	_bitmap = NULL;
	_playerCountServer = 0;
	_playerCount = 0;
	_exitTimerThread = false;
	_timerThreadID = 0;
	
	// champ personnalise
	memset(_customfield,'*',FIELD_WIDTH*FIELD_HEIGHT);

	// tableau des joueur
	// effacer la liste des joueurs
	for(char i=0;i<BETRIS_MAX_CLIENTS;i++)
	{
		_playersOnServer[i] = NULL;
		_playersClient[i] = NULL;
	}
	
	// preferences
	_prefs = new CPreferenceFile(P_FILE_APP_PREFS,BASE_PREFERENCE_PATH);
	_prefsUser = new CPreferenceFile(P_FILE_USER_PREFS,BASE_PREFERENCE_PATH,P_PATH_USER_PREFS);
	_prefsNetwork = new CPreferenceFile(P_FILE_SERVER_PREFS,BASE_PREFERENCE_PATH,P_PATH_SERVER_PREFS);

	// le chemin de l'application
	GetAppInfo(&info);
	file.SetTo(&(info.ref), B_READ_ONLY);
	_ressource.SetTo(&file);
	_appPath.SetTo(&(info.ref)); 
    _appPath.GetParent(&_appPath);

	// initialisation des blocs
	_blocks[0] = b1;
	_blocks[1] = b2;
	_blocks[2] = b3;
	_blocks[3] = b4;
	_blocks[4] = b5;
	_blocks[5] = b6;
	_blocks[6] = b7;		

	// orientation de chaque blocs
	_blockOrient[0] = 2;
	_blockOrient[1] = 1;
	_blockOrient[2] = 4;
	_blockOrient[3] = 4;
	_blockOrient[4] = 2;
	_blockOrient[5] = 2;
	_blockOrient[6] = 4;

	// copie de la definition des block
	memcpy(_blocks_def,"012345acnrsbgqo",BLOCK_DEFS);

	// mettre a zero le liens champs de jeu joueur
	_load_theme_bitmap();
	_init_player_field_tab();

	// charger les prefs
	_prefs->Load();
	_prefsUser->Load();
	_prefsNetwork->Load();

	// charger les touches de controles
	_controlKey[0] = _prefsUser->GetInt8(28,"key-0");
	_controlKey[1] = _prefsUser->GetInt8(29,"key-1");
	_controlKey[2] = _prefsUser->GetInt8(30,"key-2");
	_controlKey[3] = _prefsUser->GetInt8(31,"key-3");
	_controlKey[4] = _prefsUser->GetInt8(32,"key-4");
	_controlKey[5] = _prefsUser->GetInt8(48,"key-5");
	
	// creer le looper reseau sans le lancer pour l'instant
	_networkLooper = new BeTrisNetwork(this,BETRIS_MAX_CLIENTS);
	_networkLooper->Run();

	// fenetre principale on fixera toujours ca taille
	windowRect = _prefs->GetRect(BRect(100,100,720,600),"win-position");
	windowRect.right = windowRect.left + 620;
	windowRect.bottom = windowRect.top + 530;
	_pGameWindow = new BeTrisWindow(windowRect);
	_pGameWindow->Show();
}

/**** Destructeur ****/
BeTrisApp::~BeTrisApp()
{
	// effacer les joueurs en mode client
	// on efface pas notre joueur local
	for(char i=1;i<BETRIS_MAX_CLIENTS;i++)
		if(_playersClient[i]!=NULL)
			delete _playersClient[i];

	// liberer les images
	if(_bitmap!=NULL)
		delete _bitmap;

	// les detruire
	delete _prefs;
	delete _prefsUser;
	delete _prefsNetwork;
}

/**** quitter l'application ****/
bool BeTrisApp::QuitRequested(void)
{
	// arreter le thread de addlines
	_stop_addlines_thread();

	// quitter le looper reseau
	g_network_messenger.SendMessage(B_QUIT_REQUESTED);
	
	// ok on quitte
	return true;
}

/**** gestion des messages ****/
void BeTrisApp::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// reception de donnees du reseau
	case BETRIS_NETWORK_MSG:
		_find_command(message);
		break;
	// on recoit des blocs a ajouter
	case BETRIS_SPBLOCK_MSG:
		_special_blocs(message);
		break;
	// rafraichir les donnees du jeu affichees
	case BETRIS_REFRESH_MSG:
		_refresh_display();
		break;
	// lancement du serveur et arret
	case NETWORK_START_SERVEUR_MSG:
	case NETWORK_STOP_SERVEUR_MSG:
		// initialiser les donnees
		_init_server_data(message->what==NETWORK_START_SERVEUR_MSG);
	// connexion changee
	case BETRIS_CONNEXION_CHANGE_MSG:
		BMessenger(_pGameWindow).SendMessage(BETRIS_CONNEXION_CHANGE_MSG);
		break;
	default:
		BApplication::MessageReceived(message);
	}
}

// ==================================
// = Information sur certains etats =
// ==================================

/**** etat de connection ****/
bool BeTrisApp::IsConnected()
{
	return _networkLooper->Connexion();
}

/**** le serveur tourne-t-il ? ****/
bool BeTrisApp::ServerIsRunning()
{
	return _networkLooper->IsStarted();
}

/**** message de sortie ****/
char *BeTrisApp::Command(_cmd_type type)
{
	// on doit etre dans les limite
	if(type>=CMD_NOENTRY)
		return NULL;
		
	// ok renvoyer le message
	return _cmdtable[type-1].string;
}

/**** trouver le champ associe a un joueur ****/
char BeTrisApp::FieldFromPlayer(char number)
{
	// trouver quel champ est associe a ce joueur
	for(char i=0;i<BETRIS_MAX_CLIENTS;i++)
		if(_playerField[i]==number)
			return i;

	// pas trouve
	return -1;
}

/**** mettre a jour le timing du jeu ****/
void BeTrisApp::UpdateGameTiming()
{
	// la partie doit biensure etre demarre
	if(_playersClient[0]!=NULL && _playersClient[0]->EndGameTime()==0 && _gameStarted)
	{
		long	timing = 0;
		short	level;
		
		// niveau de notre joueur
		level = _playersClient[0]->Level();

		// le timing est definie en miliseconde
		timing = 5;
		if(level<=100)
			timing = (1005 - level * 10);
	
		// mettre la valeur interne a jour
		_gameData._timing = (timing * 1000);
	}
	else
		// arreter
		_gameData._timing = 0;
}

/**** recopier de l'image ou tout se trouve vers la bitmap off screen ****/
void BeTrisApp::BlitBitmap(BRect source,BBitmap *bmpSrc,short dx,short dy,BBitmap *bmpDest)
{
	// l'image source et offscreen doivent exister
	if(bmpSrc==NULL || bmpDest==NULL)
		return;

	// la source est-elle correct
	if(!(bmpSrc->Bounds()).Contains(source))
		return;

	// verifier que l'on est bien dans la destination
	if(!(bmpDest->Bounds()).Contains(BRect(dx,dy,dx+source.Width(),dy + source.Height())))
		return;

	int32	*sourcePointer = NULL;
	int32	*destPointer = NULL;
	short	x,y;
	int		ws,wd;
	int		sindex,dindex;
	
	// pointeur sur la memoire des images
	sourcePointer = (int32 *)(bmpSrc->Bits());
	destPointer = (int32 *)(bmpDest->Bits());

	// taille d'une ligne de l'image
	ws = bmpSrc->BytesPerRow() / sizeof(int32);
	wd = bmpDest->BytesPerRow() / sizeof(int32);

	// depart
	sindex = (int)source.left + (int)source.top * ws;
	dindex = dx + dy * wd;

	// on va recopier la portion source dans la destination
	for(y=0;y<=((int)(source.Height()));y++)
	{
		for(x=0;x<=((int)(source.Width()));x++)
			*(destPointer + dindex + x) = *(sourcePointer + sindex + x);
		
		// ligne suivante
		dindex += wd;
		sindex += ws;
	}
}

/**** tirer un chiffre aleatoire ****/
char BeTrisApp::RandomNum(char scale)
{	return (char)((float)scale*rand()/(RAND_MAX+1.0));	}

/**** trduire la chaine ****/
char BeTrisApp::TranslateBlock(char block)
{
	// traduire en block	
	for(int i=0;_blocks_def[i];i++)
        if(block==_blocks_def[i])
			return i;
	
	// block introuvable
	return -1;
}

/**** recuperer une couleur ****/
rgb_color *BeTrisApp::TranslateColor(char color)
{
	// par defaut c'est du noir
	if(color>=32)
		return (rgb_color *)&U_COULEUR_NOIR;

	rgb_color	*rgbcolor = NULL;

	// trouver la couleur
	switch(color)
	{
		case U_BOLD_STYLE:
		case U_BOLD_ITALIC:
		case U_BOLD_UNDERLINE:
			break;
		case U_CYAN_COLOR:
			rgbcolor = (rgb_color *)&U_TURQUOISE_STANDARD;
			break;
		case U_BLUE_COLOR:
			rgbcolor = (rgb_color *)&U_BLEU_CLAIR;
			break;
		case U_GRAY_COLOR:
			rgbcolor = (rgb_color *)&U_GRIS_FONCE;
			break;
		case U_MAGENTA_COLOR:
			rgbcolor = (rgb_color *)&U_MAGENTA_STANDARD;
			break;
		case U_GREEN_COLOR:
			rgbcolor = (rgb_color *)&U_VERT_FONCE;
			break;
		case U_LIME_COLOR:
			rgbcolor = (rgb_color *)&U_VERT_CLAIR;
			break;
		case U_SILVER_COLOR:
			rgbcolor = (rgb_color *)&U_GRIS_CLAIR;
			break;
		case U_MAROON_COLOR:
			rgbcolor = (rgb_color *)&U_ORANGE_CLAIR;
			break;
		case U_DARK_BLUE_COLOR:
			rgbcolor = (rgb_color *)&U_BLEU_FONCE;
			break;
		case U_OLIVE_COLOR:
			rgbcolor = (rgb_color *)&U_ROUGE_FONCE;
			break;
		case U_PURPLE_COLOR:
			rgbcolor = (rgb_color *)&U_VIOLET_FONCE;
			break;
		case U_RED_COLOR:
			rgbcolor = (rgb_color *)&U_ROUGE_CLAIR;
			break;
		case U_TEAL_COLOR:
			rgbcolor = (rgb_color *)&U_VIOLET_STANDARD;
			break;
		case U_WHITE_COLOR:
			rgbcolor = (rgb_color *)&U_COULEUR_BLANC;
			break;
		case U_YELLOW_COLOR:
			rgbcolor = (rgb_color *)&U_JAUNE_STANDARD;
			break;
		case U_BLACK_COLOR:
			rgbcolor = (rgb_color *)&U_COULEUR_NOIR;
			break;
		case '\0':
		default:
			return NULL;
	}

	// code couleur
	return rgbcolor;
}

/**** encoder la chaine de connexion ****/
bool BeTrisApp::EncodeConnexionString(BString &string,sockaddr_in &addr)
{
	BeTrisPlayerItem	*clientItem = NULL;
	char				nickmsg[1024];
	char 				iphashbuf[32];
	unsigned int		bufferSize;
	unsigned char		ip[4];
	unsigned long		longIp;
	BString				nick;
	char				*str = NULL;
	int					len = 0;
	int					leniphashbuf = 0;
	BString				version;
	BString				typegame;

	// initialiser
	version = BETRIS_TETRINET_V113;
	typegame = BETRIS_TETRINET_CODE;

	// recuperer le nickname du joueur activee
	clientItem = ActivatedPlayer();
	if(clientItem==NULL)
		return false;

	// doit-on modifier le type et version du jeu
	if(_prefsUser!=NULL)
	{
		// autre version
		if(_prefs->GetInt32(B_CONTROL_OFF,"tetrinet-version")==B_CONTROL_ON)
			version = BETRIS_TETRINET_V114;
		
		// type de jeu
		if(_prefs->GetInt32(B_CONTROL_OFF,"tetrinet-type")==B_CONTROL_ON)
			typegame = BETRIS_TETRIFAST_CODE;
	}
		
	// nickname et taille finale du buffer
	nick = clientItem->NickName();
	bufferSize = nick.Length() + typegame.Length() + 2 + version.Length() + 1;
	
	// preparer la chaine
	string.SetTo('\0',bufferSize);
	str = string.LockBuffer(string.Length()+1);
	if(str==NULL)
		return false;

	// commande de connexion
	sprintf(nickmsg,"%s %s %s",typegame.String(),nick.String(),version.String());
	len = strlen(nickmsg);

	// adresse IP
	longIp = addr.sin_addr.s_addr;

	// adresse hash
	memcpy(&ip,&longIp,sizeof(longIp));
	sprintf(iphashbuf, "%d", ip[0]*54 + ip[1]*41 + ip[2]*29 + ip[3]*17);
	leniphashbuf = strlen(iphashbuf);

	// informations
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
	{
		// pour str on va eviter les 0 du debut
		printf("************** ENCODING STRING **********************\n");
		printf("* IP             : [%d.%d.%d.%d]\n",ip[0],ip[1],ip[2],ip[3]);
		printf("* Encoding IP    : [%s]\n",iphashbuf);
		printf("* nickname       : [%s]\n",nick.String());
		printf("* message        : [%s]\n",nickmsg);
	}

	str[0] = (unsigned char)(RandomNum(155) + 100);
	for(int i=0;i<len;i++)
		str[i+1] = (((str[i]&0xFF) + (nickmsg[i]&0xFF)) % 255) ^ iphashbuf[i % strlen(iphashbuf)];

	len++;
	for (int i=0;i<len;i++)
		sprintf(nickmsg+i*2, "%02X", str[i] & 0xFF);

	// informations
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
		printf("* hexadecimal    : [%s]\n",nickmsg);

	// liberer le buffer
	string.UnlockBuffer();

	// recopier la chaine encodee
	string = nickmsg;
	
	// ok ca c'est bien passe
	return true;
}

/**** decoder la chaine de connexion ****/
bool BeTrisApp::DecodeConnexionString(BString &encryptedString)
{
	char	game[12];
	
	// initialiser
	memset(game,0,12);

	// enlever le dernier caractere
	encryptedString.Truncate(encryptedString.Length()-1);

	// quel type de jeu est utilise
	if(_prefs->GetInt32(B_CONTROL_OFF,"tetrinet-type")==B_CONTROL_ON)
		strcpy(game,BETRIS_TETRIFAST_CODE);
	else
		strcpy(game,BETRIS_TETRINET_CODE);

    char			decrypted[1024];
    unsigned char	decimal[encryptedString.Length()/2];
    unsigned char	data[12];
    unsigned char	ip[12];
    short			index;
	int				stringSize;
	char			*str = NULL;
	char			iplen = 0;
	 
 	// initialiser
 	memset(decimal,0,(encryptedString.Length()/2));
 	memset(data,0,12);
 	memset(ip,0,12);
 		
	// convertir en decimal lela chaine source
	stringSize = encryptedString.Length();
	str = encryptedString.LockBuffer(stringSize + 1);
	if(str==NULL)
		return false;

	// convertion
	for(short i=0;i<stringSize;i+=2)
		decimal[i/2] = _hex_to_int(str + i);

	// si on est en debug on affiche la commande complete
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
	{
		printf("************** DECODING STRING **********************\n");
		printf("* Encypted = (%ld) [%s]\n",encryptedString.Length(),encryptedString.String());
		printf("* Game type = [%s]\n",game);
		printf("* Encoded String = [%s]\n",str);
		printf("* Decimal tab =[%d",decimal[0]);
		for(short i=2;i<(stringSize-1);i+=2)
			printf(",%d",decimal[i/2]);
		printf("]\n");
	}

	// debloquer la chaine source
	encryptedString.UnlockBuffer();

	// recopier le type de partie
	// et en determiner la taille
	strcpy((char *)data,game);
	for(unsigned char index=0;index<strlen((char *)data);index++)
	{
		ip[index] = (((data[index] + decimal[index]) % 255) ^ decimal[index+1]);

		iplen = 5;
		for(char i=5;(i==iplen && i>0);i--)
			for(unsigned char j=0;j<(strlen((char *)data)-iplen);j++)
				if(ip[j]!=ip[j + iplen])
					iplen--;
	}
	
	// verifier la taille de l'ip
	if(iplen<=0)
		return false;

	// debug
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
	{
		printf("* Size IP=[%d]\n",iplen);	
		printf("* Decoded IP=[%d",ip[0]);	
		for(char i=1;i<11;i++)
		printf(",%d",ip[i]);	
		printf("]\n");	
	}
	
	// decoder le message avec l'ip et le tableau de decimal
	memset(decrypted,0,1024);
	for(index=1;index<(stringSize/2);index++)
		decrypted[index-1] = (((decimal[index] ^ ip[(index - 1) % iplen]) + 255 - decimal[index - 1]) % 255);

	// fin de chaine et copie dans la source
	decrypted[index-1] = '\0';
	encryptedString.SetTo(decrypted);

	// debug
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
		printf("* Decoded String=[%s]\n",encryptedString.String());
		
	// ok ca c'est bien passe
	return true;
}

/**** touche de controle ****/
void BeTrisApp::SetControlKey(char index,char key)
{
	// verifier l'index
	if(index>=0 && index<NUMBER_CONTROL_KEY)
		_controlKey[index] = key;
}

/**** jouer un son ****/
void BeTrisApp::PlaySound(char sndIndex)
{
	// verifier le parametre
	if(sndIndex<1 || sndIndex>SND_MAX_SOUND)
		return;

	// construire le nom du fichier
	BString		path;
	BEntry		entry;
	entry_ref	sound_ref;
	
	// construire le chemin par defaut
	path = _appPath.Path();
	path.Append("/");
	path.Append(P_PATH_DATA);

	// charger l'image
	path = _prefs->GetString(path,P_PATH_DATA);
	path.Append("/");
	path << (int)(sndIndex) << ".wav";
	entry.SetTo(path.String());

	// verifier l'entry
	if(entry.Exists() && entry.GetRef(&sound_ref)==B_OK)
		play_sound(&sound_ref,false,false,true);
}

/**** acces au donnees d'un joueur du serveur ****/
BeTrisPlayerItem *BeTrisApp::ServerPlayer(char number)
{
	// attention demander un chiffre valide
	if(number<0 && number>=BETRIS_MAX_CLIENTS)
		return NULL;

	// verifier si le slot existe
	if(_playersOnServer[number]==NULL)
		return NULL;

	// ok pointeur vers la structure joueur demande
	return _playersOnServer[number]->_player;
}

/**** retourner le joueur actif ****/
void BeTrisApp::SetActivatedPlayer(BeTrisPlayerItem *player)
{
	// si un joueur existe deja on l'efface
	if(_playersClient[0]!=NULL)
	{
		delete _playersClient[0];
		_playersClient[0] = NULL;
	}
	
	// on va creer une copie de notre joueur
	// issu des preferences
	if(player!=NULL)
	{
		BMessage	archive;
		
		if(player->Archive(&archive)==B_OK)
			_playersClient[0] = new BeTrisPlayerItem(&archive);
	}
}

/**** recuperer un joueur ****/
BeTrisPlayerItem *BeTrisApp::GetClientPlayer(char playerNumber)
{
	// attention demander un chiffre valide
	if(playerNumber<0 && playerNumber>=BETRIS_MAX_CLIENTS)
		return NULL;

	// parcourir la liste
	for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
		if(_playersClient[index]!=NULL && _playersClient[index]->Number()==playerNumber)
			return _playersClient[index];
	
	// pas trouvee
	return NULL;
}

// =================
// fonction internes
// =================

/**** trouver les commandes ****/
void BeTrisApp::_find_command(BMessage *message)
{
	unsigned char	c;
	char			indexCommand = 0;
	int				lastCommand = 0;
	bool			chatline = false;
	char			slot = -1;

	// recuperer la chaine du reseau
	if(message->FindString(BETRIS_NETWORK_DATAS,&_commands)!=B_OK)
		return;

	// recuperer le slot recepteur
	if(message->FindInt8(BETRIS_NETWORK_SLOT,(int8 *)&slot)!=B_OK)
		slot = -1;

	// si on est en debug on affiche la commande complete
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
	{
		printf("************** COMMAND **********************\n");
		printf("* complete string=%s\n",_commands.String());
		printf("* string size=%ld\n",_commands.Length());
		printf("* command from slot=%d\n",slot);
	}

	// on va scanner chaque commande
	for(int i=0;i<_commands.Length();i++)
	{
		c = _commands[i];

		// on a une commande et on recupere les donnees
		if((c==0xFF || c==0x0A) || (c==' ' && !(chatline && indexCommand>1)))
		{
			// recuperer la commande et les donnees
			// la commande est le premier
			if(i>=0)
			{
				// recuperer la chaine
				_commands.CopyInto(_datas[indexCommand],lastCommand,(i-lastCommand));

				// est-ce une commande de chat ou un message de partie
				// dans ce cas on recupere le tout espace compris
				if(!chatline && indexCommand==0)
					if(_datas[indexCommand].IFindFirst(_cmdtable[CMD_PLINE-1].string)!=B_ERROR || _datas[indexCommand].IFindFirst(_cmdtable[CMD_GMSG-1].string)!=B_ERROR)// || _datas[indexCommand].IFindFirst(_cmdtable[CMD_CONNECTERROR-1].string)!=B_ERROR)
					 	chatline = true;
				
				// prochaine commande ou donnees
				lastCommand = i + 1;
				indexCommand++;
			}

			// ok fin de la commande
			if(c==0xFF || c==0x0A || indexCommand>MAX_COMMANDS)
			{
				// on traite la commande
				if(indexCommand>0)
				{
					// si on a active le debbug
					if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
					{
						printf("* To [");
						if(slot==0)
							printf("client");
						else
							printf("server");
						printf("] command=%s\n",_datas[0].String());
						printf("*\tdata number=%d\n",indexCommand-1);
						for(char i=1;i<indexCommand;i++)
							printf("*\tdatas[%d]=%s\n",i,_datas[i].String());
					}

					// executer les commandes pour notre joueur local
					if(ActivatedPlayer()!=NULL && slot==0)
						_execute_commands_client((indexCommand-1));
	
					// si le serveur tourne
					// executer les commandes pour lui aussi
					if(ServerIsRunning() && slot!=0)
						_execute_commands_server((indexCommand-1),slot);
				}
				
				// recommencer
				chatline = false;
				indexCommand = 0;
			}
		}
	}
	
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
		printf("************** END COMMAND ******************\n");
}

/**** trouver l'id de la commande ****/
char BeTrisApp::_find_command_id()
{
	// trouver de quel commande il sagit
    for(char i= 0;i<CMD_NOENTRY; i++)
        if (strcmp(_cmdtable[i].string,_datas[0].String()) == 0)
            return _cmdtable[i].index;

	// si on est en debug on affiche la commande qui n'est pas geree
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
	{
		printf("======= !! COMMAND NOT MANAGED !! ===========\n");
		printf("= command [%s]\n",_datas[0].String());
	}


	// pas trouvee
	return -1;
}

/**** traiter les commandes ****/
void BeTrisApp::_execute_commands_client(char nbCommand)
{
	// ===================================================
	// les info sur le protocole sont tirees de la version
	// linux gtetrinet (fichier tetrinet.c)
	//====================================================
	char		command = -1;

	// trouver de quel commande il sagit
	command = _find_command_id();
	if(command<0)
		return;
	
	// traiter
	switch(command)
	{
	// gestion des connexions
	case CMD_CONNECT:
		{
			// vider les champs de jeu
			_init_player_field_tab();

			// vider la liste des gagnants
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();
			
			_pGameWindow->_winnerView->EmptyList();

			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();

			// informer l'application du changement
			BMessenger(_pGameWindow).SendMessage(BETRIS_CONNEXION_CHANGE_MSG);
		}
		break;
	case CMD_DISCONNECT:
	case CMD_CONNECTERROR:
		{
			BString		information = "connecting ERROR";
		
			// arreter la partie (et la pause si c'etait le cas)
			// et le timer
			_gameStarted = false;
			UpdateGameTiming();
			
			// il va falloir tout reinitialiser
			// et surtout modifier les etat des controle disponible que si on est connecte
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// remettre a zero les champs de jeu
			// et rafraichir l'affichage
			_pGameWindow->_fieldsView->EmptyDisplay();
			_pGameWindow->_fieldsView->NextBlock()->Empty();
			_pGameWindow->_fieldsView->Invalidate();
			
			// informer du probleme
			if(command==CMD_DISCONNECT)
				information = "disconnected from server";
			_pGameWindow->_partyLineView->AddChatMessage(-1,information.String(),(rgb_color *)&U_ROUGE_CLAIR);					
			
			// enlever les client de la liste
			_pGameWindow->_partyLineView->RemovePlayer(NULL);
			
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
				
			// plus aucun joueur
			_playerCount = 0;
		}
		break;
	// on obtient son numero de joueur
	case CMD_PLAYERNUM_TFAST:
	case CMD_PLAYERNUM:
		{
			BMessage			outMessage(BETRIS_NETWORK_MSG);
			BMessage			archive;
			char				playerNum;
			BString				team;
			
			// le joueur courant doit exister
			if(_playersClient[0]==NULL)
				break;

			// on recupere le numero de joueur que le serveur nous a attribue
			// que le serveur nous a attribuer
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// retenir notre numero de joueur
			_playersClient[0]->SetNumber(playerNum);
			
			// envoyer le nom de l'equipe dont on fait partie
			team = Command(CMD_TEAM);
			team << " ";
			team << (int32)(_playersClient[0]->Number());
			team << " ";
			team << _playersClient[0]->Team();
			outMessage.AddString(BETRIS_NETWORK_DATAS,team);
			outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
			g_network_messenger.SendMessage(&outMessage);
			
			// notre joueur a le champ de jeu 0
			// attention on ajoute une copie
			_playerField[0] = playerNum;

			// proteger le parcour
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// ajouter le joueur a la liste
			_pGameWindow->_partyLineView->AddPlayer(_playersClient[0]);

			// mettre a jour l'affichage
			(_pGameWindow->_fieldsView->PlayerField(0))->Update(_playersClient[0]);
  			(_pGameWindow->_fieldsView->PlayerField(0))->Invalidate();
			
			// debloquer
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
			
			// le prochain champ de jeu sera affcete au prochain joueur
			_playerCount++;
		}
		break;
	// un joueur rejoint la partie
	case CMD_PLAYERJOIN:
		{
			BeTrisPlayerItem	*newPlayer = NULL;
			char				playerNum;
			char				freeField = -1;
			
			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// recuperer le pseudo du joueur
			if(_datas[2].Length()<=0)
				break;

			// on se rajoute pas soit-meme
			if(_playersClient[0]!=NULL && _playersClient[0]->Number()==playerNum)
				break;
			
			// trouver un champ non assigne
			freeField = _find_empty_field_tab();
			if(freeField<0)
				break;
			
			// mettre a jour le joueur
			newPlayer = new BeTrisPlayerItem();
			newPlayer->SetNumber(playerNum);
			newPlayer->SetNickName(_datas[2].String());
			
			// garder la position du nouveau joueur
			// on retient le joueur
			_playerField[freeField] = playerNum;
			_playersClient[freeField] = newPlayer;

			// proteger l'acces
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// ajouter le joueur a la liste
			_pGameWindow->_partyLineView->AddPlayer(newPlayer);

			// informer que le joueur a rejoin la partie
			_pGameWindow->_partyLineView->AddChatMessage(playerNum,"is Now Playing",(rgb_color *)&U_VERT_FONCE);

			// mise a jour des champs de jeu
			(_pGameWindow->_fieldsView->Field(freeField))->MakeEmpty();
			(_pGameWindow->_fieldsView->PlayerField(freeField))->Update(newPlayer);
  			(_pGameWindow->_fieldsView->PlayerField(freeField))->Invalidate();
  
			// debloquer
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();

			// le prochain champ de jeu sera affcete au prochain joueur
			_playerCount++;
		}			
		break;
	// un joueur quitte le jeux
	case CMD_PLAYERLEAVE:
		{
			BeTrisPlayerItem	*player = NULL;
			char				playerNum = -1;
			char				field = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// on doit avoir trouvee le champ
			// mais ca doit pas etre nous on a le champ de jeu 0
			field = FieldFromPlayer(playerNum);
			if(field<0)
				break;			

			// ajouter la ligne de message et trouver le joueur qui part
			player = _playersClient[field];
			if(player==NULL)
				break;

			// proteger l'acces a la textliste
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// message
			_pGameWindow->_partyLineView->AddChatMessage(playerNum,"has left",(rgb_color *)&U_VERT_FONCE);

			// si c'est nous on ne fera pas exactement le meme traitement
			if(field==0)
			{
				// annuler notre numero de joueur
				player->SetNumber(-1);

				// mise a jour du champ de jeu
				(_pGameWindow->_fieldsView->Field(field))->MakeEmpty();
				(_pGameWindow->_fieldsView->PlayerField(field))->Update(player);
			}
			else
			{
				// liberer le champ
				_playerField[field] = -1;

				// enlever le joueur de la liste
				_pGameWindow->_partyLineView->RemovePlayer(player);

				// mise a jour du champ de jeu
				(_pGameWindow->_fieldsView->Field(field))->MakeEmpty();
				(_pGameWindow->_fieldsView->PlayerField(field))->Update(NULL);

				// effaacer le joueur
				delete _playersClient[field];
				_playersClient[field] = NULL;

				// et on en a un en moins
				_playerCount--;
			}

			// rafraichir l'affichage
			(_pGameWindow->_fieldsView->PlayerField(field))->Invalidate();
				
			// debloquer la fenetre
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();			
		}
		break;
	// un joueur rejoint une equipe (ou il la creer)
	case CMD_TEAM:
		{
			BeTrisPlayerItem	*player = NULL;
			BString				text;
			char				playerNum = -1;
			char				field = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// on doit avoir trouvee le champ
			field = FieldFromPlayer(playerNum);
			if(field<0)
				break;			
			
			// trouver le joueur par son numero
			player = GetClientPlayer(playerNum);

			// proteger l'acces a la textliste
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// mettre a jour les affichages
			if(player!=NULL)
				player->SetTeam(_datas[2].String());

			// informer
			text = "is Now Alone";
			if(_datas[2].Length()>0)
			{
				// definir l'equipe du joueur
				text = "is Now on Team ";
				text << _datas[2];
			}
				
			// afficher l'info
			_pGameWindow->_partyLineView->AddChatMessage(playerNum,text.String(),(rgb_color *)&U_ROUGE_FONCE);

			// mise a jour du champ de jeu
			if(player!=NULL)
			{
				(_pGameWindow->_fieldsView->PlayerField(field))->Update(player);
				(_pGameWindow->_fieldsView->PlayerField(field))->Invalidate();
			}
			
			// deproteger
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();			
		}
		break;
	// une ligne de chat
	case CMD_PLINE:
		{
			char				playerNum = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// ajouter la ligne de message
			// proteger l'acces a la textliste
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();
				
			// l'ajouter
			_pGameWindow->_partyLineView->AddChatMessage(playerNum,_datas[2].String(),(rgb_color *)&U_COULEUR_NOIR,false);

			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
		}
    	break;
	// on est degage
	case CMD_KICK:
		{
			BeTrisPlayerItem	*player = NULL;
			char				playerNum = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum<0 || playerNum>BETRIS_MAX_CLIENTS)
				break;

			// trouver le joueur qui a envoye le message
			player = GetClientPlayer(playerNum);

			// proteger l'acces a la textliste
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// on va informer notre joueur qu'il a ete degage
			if(player!=NULL)
				_pGameWindow->_partyLineView->AddChatMessage(playerNum,"Has Been Kicked",(rgb_color *)&U_VERT_FONCE);

			// debloquer la fenetre
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();			
		}
		break;
	// ligne d'info action
	case CMD_PLINEACT:
		break;
	// un joueur a perdu
	case CMD_PLAYERLOST:
		{
			BeTrisPlayerItem	*player = NULL;
			char				playerNum = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum<0 || playerNum>BETRIS_MAX_CLIENTS)
				break;

			// joueur concerne
			player = GetClientPlayer(playerNum);
			if(player!=NULL)
			{
				// si c'est nous on va declancher tout ce qui va avec
				if(_playersClient[0]->Number()==playerNum)
				{
					// bloquer le looper
					if(!_pGameWindow->IsLocked())
						_pGameWindow->Lock();

					// on a perdu
					(_pGameWindow->_fieldsView->Field(0))->PlayerLost();
				
					// debloquer
					if(_pGameWindow->IsLocked())
						_pGameWindow->Unlock();
				}
				else
					// on va noter que le joueur a perdu
					player->SetEndGameTime(time(0));
			}
		}
		break;
	// un joueur gagne
	case CMD_PLAYERWON:
		{
			char				playerNum = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// si c'est nous on va jouer le son de la victoire
			if(_playersClient[0]->Number()==playerNum)
				PlaySound(SND_WIN_GAME);
		}
		break;
	// nouvelle partie
	case CMD_NEWGAME_TFAST:
	case CMD_NEWGAME:
		{
			short	i;
			short	j;

			// initialisation
			_gameData._lineCount = 0;
			_gameData._slines = 0;
			_gameData._llines = 0;			

			// tous les joueurs son en jeu (on remet le temps a 0)
			for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
				if(_playersClient[index]!=NULL)
					_playersClient[index]->SetEndGameTime(0);
				

			// recuperer les elements de la partie
			// et renseigner la structure interne
			_gameData._initialStackHeight = atoi(_datas[1].String());
			_gameData._initialLevel = atoi(_datas[2].String());
			_gameData._linesperLevel = atoi(_datas[3].String());
			_gameData._levelInc = atoi(_datas[4].String());
			_gameData._specialLines = atoi(_datas[5].String());
			_gameData._specialCount = atoi(_datas[6].String());
			_gameData._specialCapacity = atoi(_datas[7].String());
			_gameData._levelAverage = atoi(_datas[10].String());
			_gameData._classicMode = atoi(_datas[11].String());

			// c'est un diviseur donc cela ne doit pas etre zero
			if(_gameData._specialLines<=0)
				_gameData._specialLines = 1;

			// verifier que _specialCapacity n'est pas plus grand que ce que l'on peut admettre
			// sinon mettre la valeur max
			if(_gameData._specialCapacity>18)
				_gameData._specialCapacity = 18;
			
			// decoder les frequence des blocs
			for(i=0;i<7;i++)
				_gameData._blockFreq[i] = 0;
			for(i=0;i<9;i++)
				_gameData._specialFreq[i] = 0;
				
			// count frequencies
			for(i=0;_datas[8][i];i++)
				_gameData._blockFreq[_datas[8][i]-'1']++;
            for(i=0;_datas[9][i];i++)
				_gameData._specialFreq[_datas[9][i]-'1'] ++;

			// cumulative
			for(i=0,j=0;i<7;i++)
			{
				j += _gameData._blockFreq[i];
				_gameData._blockFreq[i] = j;
			}
			for(i=0,j=0;i<9;i++)
			{
				j += _gameData._specialFreq[i];
				_gameData._specialFreq[i] = j;
			}
            
            // verifier que l'on deborde pas
			if(_gameData._blockFreq[6]<100)
				_gameData._blockFreq[6]=100;
			if(_gameData._specialFreq[8]<100)
				_gameData._specialFreq[8]=100;

			// mettre a jour le level du joueur
			if(_playersClient[0]!=NULL)
				_playersClient[0]->SetLevel(_gameData._initialLevel);
			
			// bloquer le looper
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// info dans le chat
			_pGameWindow->_partyLineView->AddChatMessage(-1,"The Game Has Started",(rgb_color *)&U_ROUGE_FONCE);

			// vider les champs
			for(char i=0;i<BETRIS_MAX_CLIENTS;i++)
				(_pGameWindow->_fieldsView->Field(i))->MakeEmpty();

			// reorganiser les champs de jeu
			_reorganize_fields();

			// vider le texte des block speciaux
			// et les blocs speciaux
			// determiner le prochain bloc
			_pGameWindow->_fieldsView->SpecialBlockInfo(NULL,NULL,NULL);
			(_pGameWindow->_fieldsView->SpecialBlocks())->Update(NULL);
			(_pGameWindow->_fieldsView->Field(0))->SetInitialHeight(_gameData._initialStackHeight);
			(_pGameWindow->_fieldsView->Field(0))->StartGame();

			// demarrage de partie (annuler la precedente pause si elle etait active)
			_gameStarted = true;
			_pause = false;

			// level moyen et affichage
			_pGameWindow->_fieldsView->UpdateAverageLevel();
			_pGameWindow->_fieldsView->RefreshDisplay();
			
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();

			// demarrer le Pulse
			UpdateGameTiming();
			
			// afficher la vue des champs de jeu
			BMessenger(_pGameWindow).SendMessage(BETRIS_DISPLAY_FIELDS_MSG);
		}
		break;
	// partie
	case CMD_INGAME:
		{
			// la partie est demarre
			_gameStarted = true;
		
			// bloquer le looper
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();
			
			// on arrive dans un partie en cours
			// on doit renvoyer notre champs de perdant
			(_pGameWindow->_fieldsView->Field(0))->FullField(true);

			// debloquer
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
		}
		break;
	// mettre en pause la partie pause
	case CMD_PAUSE:
		{
			char	newState = 0;

			// mettre en pause ou enlever la pause
			newState = atoi(_datas[1].String());
			_pause = (newState==1);
			
			// bloquer le looper
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// informer
			if(_pause)
				_pGameWindow->_partyLineView->AddChatMessage(-1,"The Game is Now in Pause",(rgb_color *)&U_VERT_FONCE);
			else
				_pGameWindow->_partyLineView->AddChatMessage(-1,"The Game is Now Playing",(rgb_color *)&U_VERT_FONCE);

			// redessiner
			(_pGameWindow->_fieldsView->Field(0))->Invalidate();

			// debloquer
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
		}
		break;
	// fin de partie
	case CMD_ENDGAME:
		{
			// stopper la partie
			_gameStarted = false;

			// anuuler le Pulse
			UpdateGameTiming();
			
			// bloquer le looper
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// message de fin
			_pGameWindow->_partyLineView->AddChatMessage(-1,"The Game Has Ended",(rgb_color *)&U_ROUGE_FONCE);

			// arreter le jeu et vider le bloc suivant
			(_pGameWindow->_fieldsView->Field(0))->StopGame();
			(_pGameWindow->_fieldsView->NextBlock())->Empty();
			_pGameWindow->_fieldsView->RefreshDisplay();

			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();			
		}
		break;
	// info sur un champ de jeu
	case CMD_F:
		{
			BeTrisPlayerItem	*player = NULL;
			char				*datas = NULL;
			char				playerNum = -1;
			char				field = -1;

			// on recupere le numero du joueur
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// si c'est pour nous on va verifier que l'on est encore de la partie
			player = ActivatedPlayer();
			if(player!=NULL && player->Number()==playerNum && player->EndGameTime()!=0)
				return;
				
			// on doit avoir trouvee le champ
			field = FieldFromPlayer(playerNum);
			if(field<0)
				break;

			// on doit avoir l'info du champ de jeu
			// sinon c'est que l'on doit vider le champ
			if(_datas[2].Length()>0)
			{
				// mettre a jour le champ de jeu
				// il y a des donnees
				datas = _datas[2].LockBuffer(_datas[2].Length());
 				if(datas!=NULL)
					(_pGameWindow->_fieldsView->Field(field))->Update(datas);
				_datas[2].UnlockBuffer();
			}
		}
		break;
	// block special
	case CMD_SB:
		{
			BeTrisPlayerItem	*player = NULL;
			char				playerTo = -1;
			char				playerFrom = -1;

			playerTo = atoi(_datas[1].String());
			playerFrom = atoi(_datas[3].String());
			// verifier les joueurs
			if(playerTo>BETRIS_MAX_CLIENTS || playerFrom>BETRIS_MAX_CLIENTS)
				break;

			// recuperer notre numero de joueur
			player = ActivatedPlayer();
			if(player==NULL)
				return;
			
			// pour qui c'est, si c'est 0 c'est pour tout le monde
			if(playerTo==0)
				_datas[1] = "All";
			else
			{
				// c'est alors un joueur precis
				player = GetClientPlayer(playerTo);
				if(player!=NULL)
					_datas[1] = player->NickName();
			}
				
			// de qui ca vient, si c'est 0 c'est le serveur
			if(playerFrom==0)
				_datas[3] = "Server";
			else
			{
			
				player = GetClientPlayer(playerFrom);
				if(player!=NULL)
					_datas[3] = player->NickName();
			}

			// recuperer le vrai nom des joueur
			// proteger l'acces aux vues
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// informer d'une action
			_pGameWindow->_fieldsView->SpecialBlockInfo(_datas[3].String(),_datas[1].String(),_datas[2].String());
			
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();

			// on va demander d'executer l'action
			_pGameWindow->_fieldsView->DoSpecialBlock(playerFrom,playerTo,_datas[2].String());
		}
		break;
	// changement de level
	case CMD_LVL:
		{
			BeTrisPlayerItem	*player = NULL;
			char				playerNum = -1;
			short				level = 0;
			
			// trouver le joueur que cela concerne
			playerNum = atoi(_datas[1].String());
			if(playerNum>BETRIS_MAX_CLIENTS)
				break;

			// recuperer le level
			level = atoi(_datas[2].String());

			// tester si c'est la commande lvl 0 0
			// dans ce cas c'est en fait une demande de version
			if(playerNum==0 && level==0)
			{
				// verifier si on veut etre reconnu comme Tetrinet 1.13
				// ou comme BeNetTris
				if(_prefs!=NULL && _prefs->GetInt32(B_CONTROL_ON,"identify-benettris")==B_CONTROL_ON)
				{
					BMessage	message(BETRIS_NETWORK_MSG);
					BString		buffer;
				
					// construire la reponse
					buffer = Command(CMD_CLIENTINFO);
					buffer << " ";
					buffer << BETRIS_VERSION;
				
					// ok envoyer au serveur
					message.AddString(BETRIS_NETWORK_DATAS,buffer);
					message.AddInt8(BETRIS_NETWORK_SLOT,0);
					g_network_messenger.SendMessage(&message);
				}
			}
			else
			{
				// trouver ce joueur
				player = GetClientPlayer(playerNum);
				if(player!=NULL)
				{
					// mise ajour du niveau du joueur
					player->SetLevel(level);

					// proteger l'acces aux vues
					if(!_pGameWindow->IsLocked())
						_pGameWindow->Lock();
			
					// level moyen
					_pGameWindow->_fieldsView->UpdateAverageLevel();
					_pGameWindow->_fieldsView->RefreshDisplay();
			
					// deproteger
					if(_pGameWindow->IsLocked())
						_pGameWindow->Unlock();
				}
			}
		}
		break;
	// message dans la partie
	case CMD_GMSG:
		{
			// on doit bien avoir 2 donnees
			if(nbCommand<2)
				break;
		
			// proteger l'acces aux vues
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// ajouter dans la liste le message
			_pGameWindow->_fieldsView->AddGameMessage(_datas[1].String(),_datas[2].String());

			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
		}		
		break;
	// liste des gagnant
	case CMD_WINLIST:
		{
			// proteger l'acces a la vue des gagnant
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();

			// vider avant la liste
			if(nbCommand>1)
				_pGameWindow->_winnerView->EmptyList();

			// ajouter chaque gagnant
			for(char index=1;index<=nbCommand;index++)
				_pGameWindow->_winnerView->AddWinner(_datas[index]);
					
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();
		}
		break;
	// liste spectateurs
	case CMD_SPECLIST:
	// un spectateur rejoin la partie
	case CMD_SPECJOIN:
	case CMD_SPECLEAVE:
	case CMD_SMSG:
	case CMD_SACT:
		break;
	// commande de ping
	case CMD_PING:
		{
			// si on recoit cette commande on renvoi l'argument avec comme commande pong
			BMessage	message(BETRIS_NETWORK_MSG);
			BString		buffer;
				
			// construire la reponse
			buffer = Command(CMD_PONG);
			buffer << " ";
			buffer << _datas[1];
				
			// ok envoyer au serveur
			message.AddString(BETRIS_NETWORK_DATAS,buffer);
			message.AddInt8(BETRIS_NETWORK_SLOT,0);
			g_network_messenger.SendMessage(&message);
		}
		break;
	// nouvelle connexion
	// normalement un client ne recoit pas cete commande
	case CMD_TETRISSTART:
		if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_CLIENT)
			printf("ERROR (_execute_commands_client) : receive CMD_TETRISSTART command\n");
		break;
	// on ne fait rien car en mode debug on afiche toute les commandes
	default:
		break;
	}
}

/**** traiter les commandes cote serveur ****/
void BeTrisApp::_execute_commands_server(char nbCommand,char slot)
{
	// ===================================================
	// les info sur le protocole sont tirees de la version
	// linux gtetrinet (fichier tetrinet.c)
	//====================================================
	BeTrisPlayerItem	*player = NULL;
	char				command = -1;

	// trouver de quel commande il sagit
	command = _find_command_id();

	if(command<0)
	{
		// verifier si c'est une connexion client
		if(!_check_connexion_client(slot))
			return;

		// trouver la commande de connexion
		command = _find_command_id();
		if(command<0)
			return;
	}
	
	// joueur concerne, attention on doit avoir un slot plus grand que 0
	// arrive ici normalement on ne peut pas avoir 0
	if(slot>0 && _playersOnServer[slot-1]!=NULL)
		player = _playersOnServer[slot-1]->_player;
	
	// traiter
	switch(command)
	{
	// gestion des connexions
	case CMD_CONNECT:
		break;
	case CMD_CONNECTERROR:
	case CMD_DISCONNECT:
	case CMD_PLAYERLEAVE:
		{
			// enlever le joueur en question
			if(player!=NULL)
			{
				// envoyer aux autres la commande
				BMessage			message(BETRIS_NETWORK_MSG);
				BString				string;

				// construire la chaine a renvoyer
				string << Command(CMD_PLAYERLEAVE);
				string << " ";
				string << (int32)(player->Number());

				// envoyer le message (sauf a l'expediteur)
				message.AddString(BETRIS_NETWORK_DATAS,string);
				message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
				g_network_messenger.SendMessage(&message);
			
				// detruire la donnee
				delete _playersOnServer[slot-1];
				_playersOnServer[slot-1] = NULL;
				
				// un joueur en moins
				_playerCountServer--;
			}		
		}
		break;
	// une ligne de chat
	case CMD_PLINE:
	// la valeur de champ d'un joueur
	case CMD_F:
		{
			// envoyer aux autres la commande
			BMessage			message(BETRIS_NETWORK_MSG);

			// envoyer le message (sauf a l'expediteur)
			message.AddString(BETRIS_NETWORK_DATAS,_commands);
			message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
			g_network_messenger.SendMessage(&message);
		}
		break;
	// kike
	case CMD_KICK:
		break;
	// un joueur rejoint une equipe (ou il la creer)
	case CMD_TEAM:
		{
			// joueur concerne
			if(player!=NULL)
			{
				// informer les autres joueur de cela
				BMessage			message(BETRIS_NETWORK_MSG);
				BString				string;

				// mettre a jour la donnee en interne
				player->SetTeam(_datas[2].String());
			
				// construire la chaine a renvoyer
				string << Command(CMD_TEAM);
				string << " ";
				string << (int32)(player->Number());
				string << " ";
				if(strlen(player->Team())>0)
					string << player->Team();

				// envoyer le message (sauf a l'expediteur)
				message.AddString(BETRIS_NETWORK_DATAS,string);
				message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
				g_network_messenger.SendMessage(&message);
			}
		}
		break;
	// ligne d'info action
	case CMD_PLINEACT:
		break;
	// joueur perd
	case CMD_PLAYERLOST:
		{
			// joueur concerne
			if(player!=NULL)
			{
				BMessage	message(BETRIS_NETWORK_MSG);
				BString		string;
				char		playing = 0;
				char		winner = 0;

				// ok le joueur ne joue plus
				// on retient le moment ou il a perdu pour le classement
				player->SetEndGameTime(time(0));
	
				// si il n'en reste plus qu'un la partie est terminee
				// on examine les structure client reseau
				for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
				{
					if(_playersOnServer[index]!=NULL && _playersOnServer[index]->_player!=NULL && _playersOnServer[index]->_player->EndGameTime()==0)
					{
						winner = (_playersOnServer[index]->_player)->Number();
						playing++;
					}
				}

				// commande player lost
				string = Command(CMD_PLAYERLOST);
				string << " ";
				string << (int32)(player->Number());

				// si il ne reste plus qu'un joueur en jeu la partie est terminee
				// il faut envoyer le gagnant et la liste des gagnant
				// on demande en plus la fin de partie
				if(winner>0 && playing==1)
				{
					// fin de chaine
					string << (char)0xFF;
					string << Command(CMD_PLAYERWON);
					string << " " << (int32)(winner) << " ";
					string << (char)0xFF;
					string << Command(CMD_WINLIST);
					
					// proteger l'acces a la vue des gagnant
					if(!_pGameWindow->IsLocked())
						_pGameWindow->Lock();

					// recuperer les gagnant et ajouter a notre liste
					// pour envoyer tout ca aux joueur
					_pGameWindow->_winnerView->GetWinner(string);
					
					// liberer la protection
					if(_pGameWindow->IsLocked())
						_pGameWindow->Unlock();

					// fin de partie
					string = Command(CMD_ENDGAME);
					string << (char)0xFF;
				}

				// prevenir les autres joueurs, mais pas le joueur qui nous a envoye
				// qu'il a perdu
				message.AddString(BETRIS_NETWORK_DATAS,string);
				message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
				g_network_messenger.SendMessage(&message);

				// renvoyer la commande en interne
				message.ReplaceString(BETRIS_NETWORK_DATAS,string);
				BMessenger(be_app).SendMessage(&message);
			}
		}
		break;
	// joueur gagne, le serveur ne recoit pas cette commande
	// c'est lui qui l'envoi
	case CMD_PLAYERWON:
		break;
	// nouvelle partie
	case CMD_STARTGAME:
		{
			BMessage	message(BETRIS_NETWORK_MSG);
			BString		buffer;
			char		startstop = -1;
			char		playernum = -1;
			
			// recuperer les parametres
			startstop = atoi(_datas[1].String());
			playernum = atoi(_datas[2].String());

			// attention au mode tetrinet ou tetrifast
			if(startstop==0)
				buffer = Command(CMD_ENDGAME);
			else
			{
				// demarrage
				buffer = Command(CMD_NEWGAME);
				if(_prefsUser->GetInt32(B_CONTROL_OFF,"tetrinet-type")==B_CONTROL_ON)
					buffer = Command(CMD_NEWGAME_TFAST);
			}
			
			// renvoyer la commande en interne
			buffer << (char)0xFF;
			message.AddString(BETRIS_NETWORK_DATAS,buffer);
			BMessenger(be_app).SendMessage(&message);
		}
		break;
	// nouvelle partie
	case CMD_NEWGAME_TFAST:
	case CMD_NEWGAME:
		{
			// on va construire la commande de demarrage de la partie
			BMessage	message(BETRIS_NETWORK_MSG);
			BString		buffer;
			BString		freqName;
			char		index;
			char		indexPCent;


			// proteger l'acces aux vues
			if(!_pGameWindow->IsLocked())
				_pGameWindow->Lock();
				
			// recuperer les prefs de la vue de parametres du serveur
			_pGameWindow->_networkView->SavePreferences(false);
			
			if(_pGameWindow->IsLocked())
				_pGameWindow->Unlock();

			// commande de demarrage de partis
			buffer = _datas[0];
			buffer << " " << (int32)(_gameDataServer._initialStackHeight);
			buffer << " " << (int32)(_gameDataServer._initialLevel);
			buffer << " " << (int32)(_gameDataServer._linesperLevel);
			buffer << " " << (int32)(_gameDataServer._levelInc);
			buffer << " " << (int32)(_gameDataServer._specialLines);
			buffer << " " << (int32)(_gameDataServer._specialCount);
			buffer << " " << (int32)(_gameDataServer._specialCapacity);

			// frequences des blocks
			buffer << " ";
			for(index=0;index<FREQ_BLOCK_NUMBER;index++)
				for(indexPCent = 0;indexPCent<_gameDataServer._blockFreq[index];indexPCent++)
					buffer << (char)(index + '1');

			// frequences des blocks speciaux
			buffer << " ";
			for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
				for(indexPCent = 0;indexPCent<_gameDataServer._specialFreq[index];indexPCent++)
					buffer << (char)(index + '1');

			// niveau moyen
			if(_prefsNetwork->GetInt32(B_CONTROL_ON,"average-level")==B_CONTROL_ON)
				buffer << " 1";
			else
				buffer << " 0";

			// mode classic ou pas
			if(_prefsNetwork->GetInt32(B_CONTROL_OFF,"classic-mode")==B_CONTROL_ON)
				buffer << " 1";
			else
				buffer << " 0";

			// envoyer le message (sauf a l'expediteur)
			message.AddString(BETRIS_NETWORK_DATAS,buffer);
			g_network_messenger.SendMessage(&message);

			// ok tous les connectes joue
			for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
				if(_playersOnServer[index]!=NULL && _playersOnServer[index]->_player!=NULL)
					_playersOnServer[index]->_player->SetEndGameTime(0);
			
			// a-t-on un joueur local connecte
			// si non il faut specifier que la partie est demarre
			if(!IsConnected())
				_gameStarted = true;

			// demarrer le thread qui va envoyer les addition de ligne
			// seulement si on a un nombre de minutes au dessus de zero
			// si on a zero ca annule cette fonctionnalite
			if(_gameDataServer._mblines>0)
			{
				_exitTimerThread = false;
				_timerThreadID = spawn_thread(_timer_add_thread,"benettris-addlines-thread",B_NORMAL_PRIORITY,this);
				resume_thread(_timerThreadID);
			}
		}
		break;
	// partie
	case CMD_INGAME:
	// pause
	case CMD_PAUSE:
	// fin de partie
	case CMD_ENDGAME:
		{
			BMessage	message(BETRIS_NETWORK_MSG);

			// envoyer la commande a tout le monde
			message.AddString(BETRIS_NETWORK_DATAS,_datas[0]);
			g_network_messenger.SendMessage(&message);

			// ok tous les connectes ne joue plus
			for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
				if(_playersOnServer[index]!=NULL && _playersOnServer[index]->_player!=NULL)
					_playersOnServer[index]->_player->SetEndGameTime(time(0));
			
			// arreter le thread de addlines
			_stop_addlines_thread();

			// a-t-on un joueur local connecte
			// si non il faut specifier que la partie est demarre
			if(!IsConnected())
				_gameStarted = false;
		}
		break;
	// block special
	case CMD_SB:
		{
			// envoyer aux autres la commande
			BMessage			message(BETRIS_NETWORK_MSG);

			// envoyer le message (sauf a l'expediteur)
			message.AddString(BETRIS_NETWORK_DATAS,_commands);
			message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
			g_network_messenger.SendMessage(&message);
		}
		break;
	// mettre a jour le niveau du joueur
	case CMD_LVL:
		{
			// on va mettre a jour le niveau des joueurs en interne
			short		level = 0;
			
			// recuperer les parametres
			level = atoi(_datas[2].String());
			player->SetLevel(level);
		}
		break;
	// message dans la partie
	case CMD_GMSG:
		{
			// envoyer a totu le monde le message
			BMessage	message(BETRIS_NETWORK_MSG);

			// envoyer le message
			message.AddString(BETRIS_NETWORK_DATAS,_commands);
			message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,-1);
			g_network_messenger.SendMessage(&message);
		}
		break;
	// liste des gagnant
	// on envoi la  liste des gagnants
	case CMD_WINLIST:
		{
		}
		break;
	// liste speciale
	case CMD_SPECLIST:
	// speciel join
	case CMD_SPECJOIN:
	case CMD_SPECLEAVE:
	case CMD_SMSG:
	case CMD_SACT:
		break;
	// nouvelle connexion
	// en tetrinet normal ou tetrifast
	case CMD_TETRISSTART:
	case CMD_TETRIFASTER:
		{
			// verifier si il reste un slot vide
			if(_playerCountServer<BETRIS_MAX_CLIENTS)
			{
				// accepter le client
				_accept_new_client(slot);
			
				// un joueur en plus
				_playerCountServer++;
			}
		}
		break;
	// ces commandes ne sont pas recu par le serveur
	case CMD_PLAYERNUM:
	case CMD_PLAYERJOIN:
		break;
	default:
		break;	// rien
	}
}

/**** verifier si c'est une connexion client ****/
bool BeTrisApp::_check_connexion_client(char slot)
{
	_network_slot	*clientSlot = NULL;
	int32			spacePosition = 0;
	int				strSize = 0;

	// c'est peut-etre la connexion d'un nouveau client
	// on va alors decoder la chaine
	// sinon c'est une commande inconnue
	// on va alors fermer la connexion de ce client (on sait laquelle on a le slot !)
	clientSlot = _networkLooper->NetworkClientSlot(slot);
	if(clientSlot->_socket==-1)
		return false;

	// decoder la chaine
	if(!DecodeConnexionString(_commands))
		return false;

	// decouper la commande
	strSize = _commands.Length();
	_commands.CopyInto(_datas[0],0,11);
	
	// trouver l'espace
	spacePosition = _commands.FindFirst(' ',13);
	if(spacePosition==B_ERROR)
		return false;
	
	// recuperer le pseudo et la version
	_commands.CopyInto(_datas[1],12,(spacePosition-12));
	_commands.CopyInto(_datas[2],spacePosition+1,strSize-spacePosition-1);

	// ok
	return true;
}

/**** on accepte un nouveau client ****/
void BeTrisApp::_accept_new_client(char slot)
{
	// creer un joueur
	BMessage			message(BETRIS_NETWORK_MSG);
	BeTrisPlayerItem	*newPlayer = NULL;
	BeTrisPlayerItem	*player = NULL;
	BString				string;
	BString				emptyField;
	BString				serverInfo[5];
	
	// l'ajouter a la liste des joueur du serveur
	newPlayer = new BeTrisPlayerItem();
	newPlayer->SetNickName(_datas[1].String());
	newPlayer->SetNumber(slot);
	_playersOnServer[slot - 1] = new BeTrisServerPlayerItem();
	_playersOnServer[slot - 1]->_player = newPlayer;
	_playersOnServer[slot - 1]->_slot = slot;

	// construction de la chaine du champ vide
	emptyField.SetTo('0',FIELD_WIDTH * FIELD_HEIGHT);

	// envoyer le numero attribue au joueur
	// et la liste des autres joueurs
	string = Command(CMD_PLAYERNUM);
	string << " ";
	string << (int32)(newPlayer->Number());
	for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
	{
		// reinit avant de tester
		player = NULL;
		if(_playersOnServer[index]!=NULL && _playersOnServer[index]->_player!=NULL && _playersOnServer[index]->_slot!=slot)
		{
			// recuperer le joueur
			player = _playersOnServer[index]->_player;

			// numero du joueur
			string << (char)0xFF;
			string << Command(CMD_PLAYERJOIN);
			string << " ";
			string << (int32)(player->Number());
			string << " ";
			string << player->NickName();

			// equipe
			string << (char)0xFF;
			string << Command(CMD_TEAM);
			string << " ";
			string << (int32)(player->Number());
			string << " ";
			if(strlen(player->Team())>0)
				string << player->Team();
		
			// le champ vide
			string << (char)0xFF;
			string << Command(CMD_F);
			string << " ";
			string << (int32)(player->Number());
			string << " ";
			string << emptyField;
		}
	}
	message.AddInt8(BETRIS_NETWORK_SLOT,slot);
	message.AddString(BETRIS_NETWORK_DATAS,string);
	g_network_messenger.SendMessage(&message);

	// envoyer aux autres joueurs comme quoi il a rejoin le serveur
	// si il y en a
	if(_playerCountServer>0)
	{
		message.MakeEmpty();
		string = Command(CMD_PLAYERJOIN);
		string << " ";
		string << (int32)(newPlayer->Number());
		string << " ";
		string << newPlayer->NickName();
		message.AddString(BETRIS_NETWORK_DATAS,string);
		message.AddInt8(BETRIS_NETWORK_SLOT_EXCLUDED,slot);
		g_network_messenger.SendMessage(&message);
	}
	
	// preparer les lignes du message d'info du serveur
	serverInfo[0] = "+------------------------------------+";
	serverInfo[1] = "| Welcome to ";
	serverInfo[1] << _prefsNetwork->GetString("BeNetTrisServeur","server-name");
	serverInfo[2] = "|        ";
	serverInfo[2] << (char)0x05 << "Be" << (char)0x14 << "OS" << (char)0x04 << " Tertinet Server ";
	serverInfo[3] = "|           (Created by CKJ)";
	serverInfo[4] = "+------------------------------------+";
			
	// lui envoyer la bienvenu du serveur
	for(char index=0;index<5;index++)
	{
		message.MakeEmpty();
		string = Command(CMD_PLINE);
		string << " 0 ";
		string << serverInfo[index];
		message.AddString(BETRIS_NETWORK_DATAS,string);
		message.AddInt8(BETRIS_NETWORK_SLOT,slot);
		g_network_messenger.SendMessage(&message);
	}
}

/**** mettre a zero le liens champs de jeu joueur ****/
void BeTrisApp::_init_player_field_tab()
{
	// on appel cette fonction pour reinitialiser
	// le tableau de correspondance des joueurs
	_playerField[0] = -1;
	for(char i=1;i<BETRIS_MAX_CLIENTS;i++)
	{
		_playerField[i] = -1;
		if(_playersClient[i]!=NULL)
		{
			delete _playersClient[i];
			_playersClient[i] = NULL;
		}
	}
}

/**** trouver un champ vide ****/
char BeTrisApp::_find_empty_field_tab()
{
	// parcourir les champs
	// attention on ne va pas parcourir le 0 car il nous est reserve !!!
	// on commence donc a 1
	for(char i=1;i<BETRIS_MAX_CLIENTS;i++)
		if(_playerField[i]==-1)
			return i;

	// pas trouvee
	return -1;
}

/**** reorganiser les champs de jeu ****/
void BeTrisApp::_reorganize_fields()
{											
return;
/******************************* est a mieux tester **************************/
/******************************* est a mieux tester **************************/
/******************************* est a mieux tester **************************/
/******************************* est a mieux tester **************************/

	// reorganiser les champs si on en a des vide
	// c'est la cas avec le depart d'un joueur
	// mais c'est utile si on a pas atteind le maximum de joueur
	if(_playerCount==BETRIS_MAX_CLIENTS)
		return;

	BeTrisPlayerItem	*player = NULL;
	bool				moved = true;
	char				index;
	char				moveIndex;
				
	// on part du dernier champ
	for(index=(BETRIS_MAX_CLIENTS-1);(index>1 && moved);index--)
	{
		// on prend le premier champ depuis la fin
		if(_playerField[index]!=-1)
		{
			moved = false;
			for(moveIndex=index-1;index>0;index--)
			{
				// on va charcher un champ vide
				if(_playerField[moveIndex]==-1)
				{
					player = GetClientPlayer(_playerField[index]);
					if(player!=NULL)
					{
						// deplacer
						_playerField[moveIndex] = _playerField[index];
						(_pGameWindow->_fieldsView->Field(moveIndex))->MakeEmpty();
						(_pGameWindow->_fieldsView->PlayerField(moveIndex))->Update(player);
						(_pGameWindow->_fieldsView->PlayerField(moveIndex))->Invalidate();
								
						// vider l'ancien
						_playerField[index] = -1;
						(_pGameWindow->_fieldsView->Field(index))->MakeEmpty();
						(_pGameWindow->_fieldsView->PlayerField(index))->Update(NULL);
						(_pGameWindow->_fieldsView->PlayerField(index))->Invalidate();
					}
				}
			}
		}
	}
}

/**** charger l'image et decouper les elements ****/
void BeTrisApp::_load_theme_bitmap()
{
	BString		path;
	BBitmap		*data;
	BView		*offView = NULL;
	
	// construire le chemin par defaut
	path = _appPath.Path();
	path.Append("/");
	path.Append(P_PATH_DATA);
	path.Append("/");
	path.Append(P_FILE_BITMAP);

	// charger l'image
	path = _prefs->GetString(path,P_PATH_DATA);
	data = BTranslationUtils::GetBitmap(path.String());
	if(data!=NULL)
	{
		// si l'image n'existe pas la creer
		if(_bitmap==NULL)
			_bitmap = new BBitmap(data->Bounds(),B_RGB32,true);
		
		// convertir en 32 bits
		if(_bitmap->Lock())
		{
			offView = new BView(data->Bounds(),"off-view",B_FOLLOW_NONE,B_WILL_DRAW);
			_bitmap->AddChild(offView);
			offView->DrawBitmap(data,BPoint(0,0));
			_bitmap->RemoveChild(offView);
			_bitmap->Unlock();
			delete offView;
		}
		
		// detruire l'image temporaire
		delete data;
	}
}

/**** on recupere des blocs speciaux ****/
void BeTrisApp::_special_blocs(BMessage *message)
{
	const char	*spblocks = NULL;
	char		field = 0;
	char		specials = '\0';
	bool		remove = false;

	// trouver les blocs si c'est un ajout a notre inventaire
	if(message->FindString(BETRIS_SPBLOCK_DATAS,&spblocks)==B_OK)
	{
		char	sizeSpecials = 0;

		// pour l'info on recupere le dernier bloc
		sizeSpecials = strlen(spblocks);
		if(sizeSpecials>0)
			specials = spblocks[sizeSpecials-1];
	
		// bloquer le looper
		if(!_pGameWindow->IsLocked())
			_pGameWindow->Lock();

		_pGameWindow->_fieldsView->UpdateSpecialBlockInfo(specials);
		(_pGameWindow->_fieldsView->SpecialBlocks())->Update(spblocks);

		if(_pGameWindow->IsLocked())
			_pGameWindow->Unlock();			
	
		// quitter
		return;
	}
	
	// on envoi un bloc special a un joueur
	if(message->FindInt8(BETRIS_SPBLOCK_FIELD,(int8 *)&field)==B_OK)
	{
		BeTrisPlayerItem	*player = NULL;
		char				playerNum = 0;

		// on va convertir le numero de champ pour trouver le joueur associe
		// car quand on envoi un bloc special on l'envoi vers un champ
		if(field<0 || field>BETRIS_MAX_CLIENTS)
			return;
			
		// joueur qui est dans le champ destinataire
		playerNum = _playerField[field];
		if(playerNum<0)
			return;

		// joueur correspondant au champ destination
		player = GetClientPlayer(playerNum);

		// verifier qu'il joue toujours
		// sinon c'est un bloc de perdu
		if(player->EndGameTime()>0)
			return;
	
		// bloquer le looper
		if(!_pGameWindow->IsLocked())
			_pGameWindow->Lock();

		specials = (_pGameWindow->_fieldsView->SpecialBlocks())->SendSpecialBlock(playerNum);
		_pGameWindow->_fieldsView->UpdateSpecialBlockInfo(specials);

		if(_pGameWindow->IsLocked())
			_pGameWindow->Unlock();			

		// quitter
		return;
	}

	// trouver si on efface un bloc
	if(message->FindBool(BETRIS_SPBLOCK_REMOVE,&remove)==B_OK && remove==true)
	{
		// bloquer le looper
		if(!_pGameWindow->IsLocked())
			_pGameWindow->Lock();

		// effacer le bloc en tete de liste
		(_pGameWindow->_fieldsView->SpecialBlocks())->RemoveSpecialBlockInfo();

		if(_pGameWindow->IsLocked())
			_pGameWindow->Unlock();			
	}
}

/**** rafraichir les donnees du jeu affichees ****/
void BeTrisApp::_refresh_display()
{
	// bloquer le looper
	if(!_pGameWindow->IsLocked())
		_pGameWindow->Lock();

	// rafraichir l'affichage
	_pGameWindow->_fieldsView->RefreshDisplay();

	if(_pGameWindow->IsLocked())
		_pGameWindow->Unlock();			
}

/**** transformer une chaine genre A0 en sa valeur ****/
unsigned char BeTrisApp::_hex_to_int(char *data)
{
	unsigned char	result = 0;
	unsigned char	value = 0;

	// pour ca on va recuperer le cractere en position 0 de data et 1
	// ca donne les 2 chiffres hexa
	for(char index=0;index<2;index++)
	{
		// recuperer la valeur
		// et comparer avec la table ascii
		value = *(data + (1 - index));
		if(value>=48 && value<=57)
			value -= 48;
		else
			if(value>=65 && value<=70)
				value -= 55;
				
		// resultat
		result += (value * ((index * 15) + 1));
	}
	
	return result;
}

/**** initialiser le donnees avant demarrage ou arret du serveur ****/
void BeTrisApp::_init_server_data(bool start)
{
	// nombre de joueur
	_playerCountServer = 0;

	// effacer la liste des joueurs
	for(char i=0;i<BETRIS_MAX_CLIENTS;i++)
	{
		// si un joueur existe
		if(_playersOnServer[i]!=NULL)
		{
			// a-t-on un element joueur
			if(_playersOnServer[i]->_player!=NULL)
				delete _playersOnServer[i]->_player;
			
			// detruire l'element
			delete _playersOnServer[i];
		}
	
		// reinitialiser la valeur
		_playersOnServer[i] = NULL;
	}
}

/**** arreter le thread de addlines ****/
void BeTrisApp::_stop_addlines_thread()
{
	// on doit avoir un thread
	if(_timerThreadID==0)
		return;

	status_t		returnState;
	status_t		state;

	// on quitte le thread
	_exitTimerThread = true;
	state = wait_for_thread(_timerThreadID,&returnState);
	_timerThreadID = 0;
}

// ================================
// Thread d'ajout de lignes serveur
// ================================

/**** stub du thread de timing pour declancher les add_lines ****/
int32 BeTrisApp::_timer_add_thread(void *data)
{	return ((BeTrisApp *)data)->_timer_add();	}

/**** thread du timer pour declancher les add_lines ****/
int32 BeTrisApp::_timer_add()
{
	BMessage	addLinesServer(BETRIS_NETWORK_MSG);
	bigtime_t	systemtimebase;
	bigtime_t	systemtime;
	short		elapsedTime = 0;
	bool		startAddedLines = false;

	// message d'ajout de ligne
	addLinesServer.AddString(BETRIS_NETWORK_DATAS,"sb 0 a 0");

	// base de temps
	systemtimebase = system_time();

	// boucle de decompte du temps
	while(!_exitTimerThread)
	{
		// temps systeme et difference en seconde
		systemtime = system_time();
		elapsedTime = ((systemtime - systemtimebase) / 1000000);

		// declancher une fois le nombre de minutes atteintes
		if((elapsedTime / 60)>=_gameDataServer._mblines)
			startAddedLines = true;
			
		// envoyer les ajouts de ligne si on atteind le nombre de seconde requise
		if(startAddedLines && elapsedTime%_gameDataServer._sblines==0)
			g_network_messenger.SendMessage(&addLinesServer);

		// attendre exactement le temps de faire une seconde
		systemtime = 1000000 - (system_time() - systemtime);
		if(systemtime>0)
			snooze(systemtime);
	}

	// ok on sort du thread
	return B_OK;
}
