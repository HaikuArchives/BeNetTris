#include "BeTrisMessagesDefs.h"
#include "BeTrisSettingView.h"
#include "BeTrisConstants.h"
#include "BeTrisPlayerItem.h"
#include "CPreferenceFile.h"
#include "CTabView.h"
#include "BeTrisNetwork.h"
#include "BeTrisWindow.h"
#include "BeTrisPartyLineView.h"
#include "BeTrisFieldView.h"

#include <ScrollView.h>
#include <Entry.h>
#include <Path.h>
#include <Box.h>
#include <TextControl.h>
#include <Button.h>
#include <RadioButton.h>
#include <TextView.h>
#include <ListView.h>
#include <ColorControl.h>
#include <FilePanel.h>
#include <CheckBox.h>
#include <stdio.h>

extern BApplication *be_app;

// constantes pour la vue
const int8	U_OPEN_IMAGES		= 0;
const int8	U_OPEN_SOUNDS		= 1;

/**** constructeur ****/
BeTrisSettingView::BeTrisSettingView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// initialiser
	_searchdirectory = NULL;

	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	/**** groupe pour les touches ****/
	_keyconfig = new BBox(BRect(10,10,310,190),"key-configuration");	
	_keyconfig->SetLabel(" Keys Configuration ");
	AddChild(_keyconfig);
	
	/**** EditText pour saisir les touches ****/
	_labelkey[MOVE_LEFT] = new BTextView(BRect(10,25,140,40),"move-left-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[MOVE_LEFT]->SetText("Move Piece Left");
	_controlkey[MOVE_LEFT] = new BeTrisKeyEdit(BRect(150,20,290,40),"move-left",MOVE_LEFT);

	_labelkey[MOVE_RIGHT] = new BTextView(BRect(10,50,140,65),"move-right-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[MOVE_RIGHT]->SetText("Move Piece Right");
	_controlkey[MOVE_RIGHT] = new BeTrisKeyEdit(BRect(150,45,290,65),"move-right",MOVE_RIGHT);
	
	_labelkey[ROTATE_RIGHT] = new BTextView(BRect(10,75,140,90),"rotate-right-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[ROTATE_RIGHT]->SetText("Rotate Piece Right");
	_controlkey[ROTATE_RIGHT] = new BeTrisKeyEdit(BRect(150,70,290,90),"rotate-right",ROTATE_RIGHT);

	_labelkey[GO_DOWN] = new BTextView(BRect(10,100,140,115),"go-down-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[GO_DOWN]->SetText("Speed up Piece Falling");
	_controlkey[GO_DOWN] = new BeTrisKeyEdit(BRect(150,95,290,115),"go-down",GO_DOWN);

	_labelkey[FALL_DOWN] = new BTextView(BRect(10,125,140,140),"fall-down-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[FALL_DOWN]->SetText("Drop Pieces");
	_controlkey[FALL_DOWN] = new BeTrisKeyEdit(BRect(150,120,290,140),"fall-down",FALL_DOWN);

	_labelkey[ROTATE_LEFT] = new BTextView(BRect(10,150,140,170),"rotate-left-label",BRect(0,0,130,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_labelkey[ROTATE_LEFT]->SetText("Rotate Piece Left");
	_controlkey[ROTATE_LEFT] = new BeTrisKeyEdit(BRect(150,145,290,165),"rotate-left",ROTATE_LEFT);

	// attacher les vues de controle des touches
	for(int32 i=0;i<NUMBER_CONTROL_KEY;i++)
	{
		// bloquer le lable et changer la couleur
		_labelkey[i]->SetViewColor(U_GRIS_STANDARD);
		_labelkey[i]->MakeSelectable(false);
		_labelkey[i]->MakeEditable(false);

		_keyconfig->AddChild(_labelkey[i]);
		_keyconfig->AddChild(_controlkey[i]);
	}
	
	/**** groupe pour les chemins ****/
	_pathconfig = new BBox(BRect(10,200,310,350),"path-configuration");	
	_pathconfig->SetLabel(" Paths Configuration ");
	AddChild(_pathconfig);
	
	/**** text info path wav ****/
	_infomusic = new BTextView(BRect(10,20,290,40),"info-music-path",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infomusic->SetText("Select the path for songs :");
	_infomusic->SetViewColor(U_GRIS_STANDARD);
	_infomusic->MakeSelectable(false);
	_infomusic->MakeEditable(false);

	/**** text info path bitmap ****/
	_infodatas = new BTextView(BRect(10,60,200,80),"info-datas-path",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infodatas->SetText("Select the parh for datas :");
	_infodatas->SetViewColor(U_GRIS_STANDARD);
	_infodatas->MakeSelectable(false);
	_infodatas->MakeEditable(false);

	/**** text info custom field path ****/
	_infocustomfield = new BTextView(BRect(10,100,200,120),"info-custom-field-path",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infocustomfield->SetText("Select the parh for custom field :");
	_infocustomfield->SetViewColor(U_GRIS_STANDARD);
	_infocustomfield->MakeSelectable(false);
	_infocustomfield->MakeEditable(false);

	/**** EditText & Bouttons pour les chemins ****/
	_musicpath = new BTextControl(BRect(8,35,250,55),"music-path","","",NULL);
	_musicpath->SetDivider(0);
	_musicpath->SetEnabled(false);
	_dataspath = new BTextControl(BRect(8,75,250,95),"datas-path","","",NULL);
	_dataspath->SetDivider(0);
	_dataspath->SetEnabled(false);
	_customfieldpath = new BTextControl(BRect(8,115,250,135),"custom-field-path","","",NULL);
	_customfieldpath->SetDivider(0);
	_customfieldpath->SetEnabled(false);
	_choosemusicpath = new BButton(BRect(260,33,290,53),"choose-music-path","...",new BMessage(U_CHOOSE_MUSIC_MSG));
	_choosedataspath = new BButton(BRect(260,73,290,93),"choose-datas-path","...",new BMessage(U_CHOOSE_DATAS_MSG));
	_choosecustomfieldpath = new BButton(BRect(260,113,290,133),"choose-custom-field-path","...",new BMessage(U_CHOOSE_CUSTOMFIELD_MSG));

	_pathconfig->AddChild(_infomusic);
	_pathconfig->AddChild(_infodatas);
	_pathconfig->AddChild(_infocustomfield);
	_pathconfig->AddChild(_musicpath);
	_pathconfig->AddChild(_dataspath);
	_pathconfig->AddChild(_customfieldpath);
	_pathconfig->AddChild(_choosemusicpath);
	_pathconfig->AddChild(_choosedataspath);
	_pathconfig->AddChild(_choosecustomfieldpath);

	/**** groupe pour les preferences du joueur ****/
	_playerconfig = new BBox(BRect(325,10,610,495),"player-configuration");	
	_playerconfig->SetLabel(" Players Configuration ");
	AddChild(_playerconfig);

	/**** text info liste des joueurs ****/
	_infoplayer = new BTextView(BRect(5,10,200,30),"info-player-list",BRect(10,10,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infoplayer->SetText("Choose your player :");
	_infoplayer->SetViewColor(U_GRIS_STANDARD);
	_infoplayer->MakeSelectable(false);
	_infoplayer->MakeEditable(false);

	/**** liste des joueur ****/
	_playerlist = new BListView(BRect(15,40,260,210),"player-list",B_SINGLE_SELECTION_LIST,B_FOLLOW_LEFT | B_FOLLOW_TOP,B_WILL_DRAW);
	_playerlist->SetSelectionMessage(new BMessage(U_SELECT_PLAYER_MSG));

	/**** bouttons d'action sur la liste ****/
	_addplayer = new BButton(BRect(12,220,55,240),"add-player","Add",new BMessage(U_ADD_PLAYER_MSG));
	_modifyplayer = new BButton(BRect(60,220,120,240),"modify-player","Modify",new BMessage(U_MODIFY_PLAYER_MSG));
	_modifyplayer->SetEnabled(false);
	_removeplayer = new BButton(BRect(215,220,275,240),"remove-player","Remove",new BMessage(U_REMOVE_PLAYER_MSG));
	_removeplayer->SetEnabled(false);

	/**** EditText pour les infos sur le joueur ****/
	_nickname = new BTextControl(BRect(15,255,270,275),"nickname-player","Nickname","",new BMessage(U_NICKNAME_PLAYER_MSG));
	_nickname->SetDivider(60);
	_team = new BTextControl(BRect(15,280,270,300),"team-player","Team","",new BMessage(U_TEAM_PLAYER_MSG));
	_team->SetDivider(60);
	_name = new BTextControl(BRect(15,305,270,325),"name-player","Name","",new BMessage(U_NAME_PLAYER_MSG));
	_name->SetDivider(60);

	/**** text info couleur du joueur ****/
	_infocolorplayer = new BTextView(BRect(3,330,200,350),"info-color-player",BRect(10,10,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infocolorplayer->SetText("Choose your color :");
	_infocolorplayer->SetViewColor(U_GRIS_STANDARD);
	_infocolorplayer->MakeSelectable(false);
	_infocolorplayer->MakeEditable(false);

	/**** Choix de la couleur du joueur (IRC) ****/
	_choosecolor = new BColorControl(BPoint(10,360),B_CELLS_32x8,2,"color-player",new BMessage(U_COLOR_PLAYER_MSG));

	/**** zone de couleur ****/
	_zonecolor = new BTextView(BRect(10,420,275,435),"color-zone",BRect(0,0,265,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_zonecolor->MakeSelectable(false);
	_zonecolor->MakeEditable(false);

	_playerconfig->AddChild(new BScrollView("scroll-player-list",_playerlist,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	_playerconfig->AddChild(_infoplayer);
	_playerconfig->AddChild(_addplayer);
	_playerconfig->AddChild(_modifyplayer);
	_playerconfig->AddChild(_removeplayer);
	_playerconfig->AddChild(_nickname);
	_playerconfig->AddChild(_team);
	_playerconfig->AddChild(_name);
	_playerconfig->AddChild(_choosecolor);
	_playerconfig->AddChild(_infocolorplayer);
	_playerconfig->AddChild(_zonecolor);

	/**** groupe pour les preferences du mode et type de client/serveur ****/
	_extendedconfig = new BBox(BRect(10,360,310,495),"extended-configuration");	
	_extendedconfig->SetLabel(" Extended Configuration ");
	AddChild(_extendedconfig);

	/**** version et type de tetrinet ****/
	_tetrinet114compliant = new BCheckBox(BRect(20,20,250,35),"tetrinet-version","Know as 1.14 version",new BMessage(BETRIS_SAVE_PREFS));
	_tetrifastcompliant = new BCheckBox(BRect(20,40,250,55),"tetrinet-type","Know as TetriFast",new BMessage(BETRIS_SAVE_PREFS));
	_knowasbenettris = new BCheckBox(BRect(20,60,250,75),"identify-benettris","Identify as BeNetTris",new BMessage(BETRIS_SAVE_PREFS));
	_usecustomfield = new BCheckBox(BRect(20,80,200,95),"custom-field","Use custom field when you lose",new BMessage(BETRIS_SAVE_PREFS));

	_extendedconfig->AddChild(_tetrinet114compliant);
	_extendedconfig->AddChild(_tetrifastcompliant);
	_extendedconfig->AddChild(_knowasbenettris);
	_extendedconfig->AddChild(_usecustomfield);

	// charger les preferences
	_refresh_preferences();

	// mettre a jour en interne
	SavePreferences(false);
}

/**** destructeur ****/
BeTrisSettingView::~BeTrisSettingView()
{
	int32				nbPlayer;

	// detruire le selecteur de fichier
	if(_searchdirectory!=NULL)
		delete _searchdirectory;

	// joueurs
	nbPlayer = _playerlist->CountItems();
	for(int32 index=0;index<nbPlayer;index++)
		delete (BeTrisPlayerItem *)(_playerlist->RemoveItem((int32)0));
}

/**** attached to window ****/
void BeTrisSettingView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// bouttons de selection du repertoire
	_choosemusicpath->SetTarget(this);
	_choosedataspath->SetTarget(this);
	_choosecustomfieldpath->SetTarget(this);

	// liste & boutons player et control des couleurs
	_playerlist->SetTarget(this);
	_addplayer->SetTarget(this);
	_modifyplayer->SetTarget(this);
	_removeplayer->SetTarget(this);
	_choosecolor->SetTarget(this);
	_tetrinet114compliant->SetTarget(this);
	_tetrifastcompliant->SetTarget(this);
	_knowasbenettris->SetTarget(this);
	_usecustomfield->SetTarget(this);

	// changer l'etat de la gui
	_update_gui();
}

/**** Messaged received ****/
void BeTrisSettingView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// sauver les preferences
	case BETRIS_SAVE_PREFS:
		SavePreferences();
		break;
	// l'etat de la connexion a changee
	case BETRIS_CONNEXION_CHANGE_MSG:
		_update_gui();
		break;
	// ajouter un joueur local
	case U_ADD_PLAYER_MSG:
		_add_player();
		break;
	// modifier un joueur
	case U_MODIFY_PLAYER_MSG:
		_modify_player();
		break;
	// selection de notre joueur
	case U_SELECT_PLAYER_MSG:
		_select_player(message);
		break;
	// demander un chemin
	case U_CHOOSE_MUSIC_MSG:
	case U_CHOOSE_DATAS_MSG:
	case U_CHOOSE_CUSTOMFIELD_MSG:
		DisplayFilePanel(message->what);
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** sauver les preferences ****/
void BeTrisSettingView::SavePreferences(bool save)
{
	CPreferenceFile		*prefs = NULL;
	CPreferenceFile		*userprefs = NULL;
	BeTrisPlayerItem	*playerItem = NULL;
	BMessage			archive;
	int32				nbPlayer;

	// acces aux prefs
	userprefs = g_BeTrisApp->PreferencesUser();
	prefs = g_BeTrisApp->Preferences();

	// modifier dans l'application
	for(char i=0;i<NUMBER_CONTROL_KEY;i++)
		g_BeTrisApp->SetControlKey(_controlkey[i]->Key(),i);

	// doit-on sauver ou mettre a jour en interne
	if(save)
	{
		nbPlayer = _playerlist->CountItems();
		userprefs->SetInt32(nbPlayer,"nb-player");
		for(int32 index=0;index<nbPlayer;index++)
		{
			playerItem = (BeTrisPlayerItem *)(_playerlist->ItemAt(index));
			playerItem->Archive(&archive);
			userprefs->SetMessage(&archive,"player",index);	
			archive.MakeEmpty();
		}

		// sauver les touches de controles
		userprefs->SetInt8(_controlkey[0]->Key(),"key-0");
		userprefs->SetInt8(_controlkey[1]->Key(),"key-1");
		userprefs->SetInt8(_controlkey[2]->Key(),"key-2");
		userprefs->SetInt8(_controlkey[3]->Key(),"key-3");
		userprefs->SetInt8(_controlkey[4]->Key(),"key-4");
		userprefs->SetInt8(_controlkey[5]->Key(),"key-5");

		// enregistrer les preferences utilisateur
		userprefs->Save();

		// la version de l'application
		// et le type de tetrinet
		prefs->SetInt32(_tetrinet114compliant->Value(),"tetrinet-version");
		prefs->SetInt32(_tetrifastcompliant->Value(),"tetrinet-type");
		prefs->SetInt32(_knowasbenettris->Value(),"identify-benettris");
		prefs->SetInt32(_usecustomfield->Value(),"custom-field");

		// charger le chemin des donnees, des musiques et du champ perso
		prefs->SetString(_dataspath->Text(),P_PATH_DATA);
		prefs->SetString(_musicpath->Text(),P_PATH_MUSICS);
		prefs->SetString(_customfieldpath->Text(),P_FILE_TETRINET);

		// enregistrer les preferences
		prefs->Save();
	}
}

/**** afficher le selecteur de fichier ****/
void BeTrisSettingView::DisplayFilePanel(int32 what)
{
	// en fonction de la demande on va selectionner un dossier ou un fichier
	switch(what)
	{
	// musique
	case U_CHOOSE_MUSIC_MSG:
		break;
 	// champ personnaliser
 	case U_CHOOSE_CUSTOMFIELD_MSG:
		break;
	// c'est pour U_CHOOSE_DATAS_MSG
	default:
		;
	}

	// detruire l'ancien selecteur
	if(_searchdirectory!=NULL)
		delete _searchdirectory;

	// initialise le selecteur de fichier
	BMessenger	BeTrisSettingViewMessenger(this);
	_searchdirectory = new BFilePanel(B_OPEN_PANEL,&BeTrisSettingViewMessenger,NULL,B_DIRECTORY_NODE,false,new BMessage(U_PANEL_CONTROL_MSG),NULL,true);		
	_searchdirectory->SetTarget(this);
	
	// ouverture du selecteur de fichier
	_searchdirectory->Show();
}

// ==============================
// = Gestion des joueurs locaux =
// ==============================

/**** ajouter un nouveau joueur ****/
void BeTrisSettingView::_add_player()
{
	// on l'ajoute par la GUI
	BString			name;
	BString			team;
	BString			nickName;
	rgb_color		color;

	// recuperer les donnees
	nickName = _nickname->Text();
	team = _team->Text();
	name = _name->Text();
	color = _choosecolor->ValueAsColor();
	if(name.Length()<=0 || nickName.Length()<=0)
		return;

	// ajouter le joueur
	BeTrisPlayerItem	*player = NULL;
	BMessage			newPlayer;

	newPlayer.AddString(PLAYER_NAME,name);
	newPlayer.AddString(PLAYER_TEAM,team);
	newPlayer.AddString(PLAYER_NICKNAME,nickName);
	newPlayer.AddInt32(PLAYER_COLOR,*(int32 *)&color);
	newPlayer.AddInt32(PLAYER_LOCAL,true);
	
	// ajouter le joueur local
	player = new BeTrisPlayerItem(&newPlayer);

	// ajouter le joueur a la liste
	_playerlist->AddItem(player);
}

/**** modifier un joueur local ****/
void BeTrisSettingView::_modify_player()
{
	BeTrisPlayerItem	*player = NULL;
	int32				index = -1;

	// trouver litem selectionnee
	index = _playerlist->CurrentSelection();
	if(index<0)
		return;
	
	// recuperer notre item
	player = (BeTrisPlayerItem *)(_playerlist->ItemAt(index));
	if(player==NULL)
		return;
	
	// ok modifier les valeurs
	player->SetNickName(_nickname->Text());
	player->SetTeam(_team->Text());
	player->SetName(_name->Text());
	
	// reafficher
	_playerlist->InvalidateItem(index);
}

/**** selection d'un joueur dans la liste ****/
void BeTrisSettingView::_select_player(BMessage *message)
{
	// on ne doit pas deja etre connecte
	if(g_BeTrisApp->IsConnected())
		return;

	BeTrisPlayerItem	*actif = NULL;
	BMessage			archive;
	int32				selection;
	
	// selection ou pas ?
	if(message->FindInt32("index",&selection)!=B_OK)
		return;

	// desactiver l'ancien joueur si il existe
	// puis l'effacer
	actif = g_BeTrisApp->ActivatedPlayer();
	if(actif!=NULL)
		actif->SetActivated(false);
	
	// definir le joueur dans l'application
	actif = NULL;
	if(selection>=0)
	{
		// activer maintenant delui de selectionne
		// et afficher dans la zone de modification les donnees
		actif = (BeTrisPlayerItem *)(_playerlist->ItemAt(selection));
		if(actif!=NULL)
		{
			actif->SetActivated(true);
			_nickname->SetText(actif->NickName());
			_team->SetText(actif->Team());
			_name->SetText(actif->Name());
		}
	}

	// archiver la joueur actif pour en recreer un
	// qui sera ajouter a l'application
	// renseigner l'application
	g_BeTrisApp->SetActivatedPlayer(actif);

	// etat des boutons
	_modifyplayer->SetEnabled(actif!=NULL && selection>=0);
	_removeplayer->SetEnabled(actif!=NULL && selection>=0);

	// invalider la liste
	_playerlist->Invalidate();
}

// ===========================
// = Gestion des preferences =
// ===========================

/**** rafraichir les preferences ****/
void BeTrisSettingView::_refresh_preferences()
{
	CPreferenceFile		*userprefs = NULL;
	CPreferenceFile		*prefs = NULL;
	BeTrisPlayerItem	*playerItem = NULL;
	BMessage			archive;
	BString				path;
	BPath				*appPath = NULL;
	int32				nbPlayer;
	bool				customfield = false;
	
	// acces aux prefs
	userprefs = g_BeTrisApp->PreferencesUser();
	prefs = g_BeTrisApp->Preferences();
	
	nbPlayer = userprefs->GetInt32(0,"nb-player");
	for(int32 index=0;index<nbPlayer;index++)
	{
		// recuperer les infos
		archive.MakeEmpty();
		archive = userprefs->GetMessage(archive,"player",index);

		// creer le joueur et l'ajouter
		playerItem = new BeTrisPlayerItem(&archive);
		_playerlist->AddItem(playerItem);
	}

	// recuperer les touches de controles
	_controlkey[0]->SetKey((char)(userprefs->GetInt8(28,"key-0")));
	_controlkey[1]->SetKey((char)(userprefs->GetInt8(29,"key-1")));
	_controlkey[2]->SetKey((char)(userprefs->GetInt8(30,"key-2")));
	_controlkey[3]->SetKey((char)(userprefs->GetInt8(31,"key-3")));
	_controlkey[4]->SetKey((char)(userprefs->GetInt8(32,"key-4")));
	_controlkey[5]->SetKey((char)(userprefs->GetInt8(48,"key-5")));
	
	// la version de l'application
	// et le type de tetrinet
	_tetrinet114compliant->SetValue(prefs->GetInt32(B_CONTROL_OFF,"tetrinet-version"));
	_tetrifastcompliant->SetValue(prefs->GetInt32(B_CONTROL_OFF,"tetrinet-type"));
	_knowasbenettris->SetValue(prefs->GetInt32(B_CONTROL_OFF,"identify-benettris"));
	_usecustomfield->SetValue(prefs->GetInt32(B_CONTROL_OFF,"custom-field"));

	// construire le chemin par defaut
	appPath = g_BeTrisApp->ApplicationPath();
	if(appPath!=NULL)
	{
		path = appPath->Path();
		path.Append("/");
		path.Append(P_PATH_DATA);

		// charger le chemin des donnees
		path = prefs->GetString(path,P_PATH_DATA);
		_dataspath->SetText(path.String());	

		path = appPath->Path();
		path.Append("/");
		path.Append(P_PATH_MUSICS);

		// charger le chemin des musics
		path = prefs->GetString(path,P_PATH_MUSICS);
		_musicpath->SetText(path.String());	

		path = appPath->Path();
		path.Append("/");
		path.Append(P_PATH_DATA);
		path.Append("/");
		path.Append(P_FILE_TETRINET);

		// fichier de customisation
		path = prefs->GetString(path,P_FILE_TETRINET);
		_customfieldpath->SetText(path.String());			
	}
	
	// champ de jeu perso
	customfield = _load_custom_field();
	_usecustomfield->SetEnabled(customfield);
	if(!customfield)
		_usecustomfield->SetValue(B_CONTROL_OFF);

	// valider l'utilisation du champ perso
	g_BeTrisApp->SetUseCustomField(_usecustomfield->Value()==B_CONTROL_ON);
}

/**** charger le champ perso ****/
bool BeTrisSettingView::_load_custom_field()
{
	BString		customFile = "";
	char		*customField = g_BeTrisApp->CustomField();

	// par defaut on rempli avec des etoiles
	memset(customField,'*',FIELD_WIDTH*FIELD_HEIGHT);

	// on doit avoir un fichier
	customFile = _customfieldpath->Text();			
	if(customFile.Length()>0)
	{
		BEntry	entry(customFile.String());

		// on doit avoir un champ valide
		if(customField==NULL)
			return false;

		// l'Entry doit etre valable
		if(entry.InitCheck()!=B_OK)
			return false;
			
		// le fichier doit exister
		if(!entry.Exists())
			return false;
			
		BFile	file(&entry,B_READ_ONLY);
		off_t	sizeFile = 0;
		
		// verifier la taille du fichier
		if(file.GetSize(&sizeFile)!=B_OK || sizeFile<=0)
			return false;

		// comparer la taille
		if(sizeFile!=((FIELD_WIDTH+1)*FIELD_HEIGHT))
			return false;

		char	lineSize = FIELD_WIDTH + 1;
		char	line[lineSize + 1];
		bool	error = false;
		
		file.Seek(0,SEEK_SET);
		for(char height=0;(!error && height<FIELD_HEIGHT);height++)
		{
			memset(line,0,lineSize + 1);
			if(file.Read(line,lineSize)!=lineSize)
				error = true;
			else
			{
				// recuperation d'un ligne
				line[FIELD_WIDTH] = '\0';
				memcpy(customField + (height * FIELD_WIDTH),line,FIELD_WIDTH);
			}
		}

		// valider chaque caractere
		for(int32 index=0;(!error && index<FIELD_WIDTH*FIELD_HEIGHT);index++)
		{
			customField[index]=g_BeTrisApp->TranslateBlock(customField[index]);
			if(customField[index]<1 || customField[index]>5)
				error = true;
		}
		
		// a-t-on eu une erreur
		// dans ce cas on remet a zero le champ perso
		if(error)
			memset(customField,'*',FIELD_WIDTH*FIELD_HEIGHT);
			
		// retourner le resultat inverse
		return !error;
	}
	
	// pas bon si on est la
	return false;
}

/**** changer l'etat de la gui ****/
void BeTrisSettingView::_update_gui()
{
	bool	connexionState;
	int32	index;
	
	// etat des controles
	connexionState = g_BeTrisApp->IsConnected();
	_choosemusicpath->SetEnabled(!connexionState);
	_choosedataspath->SetEnabled(!connexionState);
	_choosecustomfieldpath->SetEnabled(!connexionState);

	// joueur
	index = _playerlist->CurrentSelection();
	_addplayer->SetEnabled(!connexionState);
	_modifyplayer->SetEnabled(!connexionState && index>=0);
	_removeplayer->SetEnabled(!connexionState && index>=0);

	// les touches
	for(int32 i=0;i<NUMBER_CONTROL_KEY;i++)
		_controlkey[i]->SetEnabled(!connexionState);	
}

// ===================================
// classe pour les touches de controle
// ===================================

/**** constructeur ****/
BeTrisKeyEdit::BeTrisKeyEdit(BRect frame,const char *name,char keyIndex)
: BView(frame,name,B_FOLLOW_NONE,B_WILL_DRAW)
{
	// initialiser
	_enabled = false;
	_index = keyIndex;
	_key = -1;
	_text = "[Not Defined]";
}

/**** destructeur ****/
BeTrisKeyEdit::~BeTrisKeyEdit()
{
}

void BeTrisKeyEdit::Draw(BRect updateRect)
{
	BRect	bounds = Bounds();

	// dessin du cadre
	rgb_color	OldHeighColor;
	rgb_color	OldLowColor;

	OldHeighColor = HighColor();
	OldLowColor = LowColor();

	// dessin du cadre exterieur
	SetHighColor(U_GRIS_CLAIR);
	StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.right,bounds.top));
	StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.left,bounds.bottom));
	SetHighColor(U_COULEUR_BLANC);
	StrokeLine(BPoint(bounds.right,bounds.bottom),BPoint(bounds.left,bounds.bottom));
	StrokeLine(BPoint(bounds.right,bounds.top),BPoint(bounds.right,bounds.bottom));

	// reduire la taille du rectangle
	bounds.InsetBy(1,1);

	// le controle est-il activee
	if(_enabled)
		SetHighColor(U_COULEUR_BLANC);
	else
		SetHighColor(U_GRIS_TRES_CLAIR);	

	// fond active ou pas
	SetLowColor(HighColor());
	FillRect(bounds);

	// dessin du texte
	if(_enabled)
		SetHighColor(U_COULEUR_NOIR);
	else
		SetHighColor(U_GRIS_FONCE);		
	DrawString(_text.String(),BPoint(5,13));

	// dessin du cadre interieur
	if(IsFocus())
	{
		SetHighColor(U_BLEU_CLAIR);
		StrokeRect(bounds);
	}
	else
	{
		// le gris fonce n'est dessiner que pour le controle activee
		if(_enabled)
		{
			SetHighColor(U_GRIS_TRES_FONCE);
			StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.right,bounds.top));
			StrokeLine(BPoint(bounds.left,bounds.top),BPoint(bounds.left,bounds.bottom));
		}
		
		SetHighColor(U_GRIS_STANDARD);
		StrokeLine(BPoint(bounds.right,bounds.bottom),BPoint(bounds.left,bounds.bottom));
		StrokeLine(BPoint(bounds.right,bounds.top),BPoint(bounds.right,bounds.bottom));
	}
	
	// retablir la couleur du fond
	SetHighColor(OldHeighColor);
	SetLowColor(OldLowColor);
}

/**** gestion clavier ****/
void BeTrisKeyEdit::KeyDown(const char *bytes,int32 numBytes)
{
	// on va traiter la touche entree
	// et afficher autre chose si il faut
	if(!_enabled)
		return;
		
	// reafficher
	if(SetKey(*bytes))
		Invalidate();
}

/**** gestion souris ****/
void BeTrisKeyEdit::MouseDown(BPoint point)
{
	// activer le focus pour recevoir les evenements souris
	if(_enabled)
		MakeFocus(true);
}

/**** gestion du focus ****/
void BeTrisKeyEdit::MakeFocus(bool focused)
{
	// mettre le focus et redessiner
	BView::MakeFocus(focused);
	Invalidate();
}

/**** activer ou desactiver ****/
void BeTrisKeyEdit::SetEnabled(bool enabled)
{
	// est-ce disponible
	_enabled = enabled;
	
	// reafficher
	Invalidate();
}

/**** definition de la touche ****/
bool BeTrisKeyEdit::SetKey(char key)
{
	// gerer les touches autorisees
	if(key<28 || key>122)
	{
		if(_index>=0 && _index<=4)
			key = 28 + _index;
		else
			key = 48;
	}

	// ok on acepte la touche
	_key = key;
	_text = "[";
	switch(_key)
	{
	case 28:
		_text << "LEFT";
		break;
	case 29:
		_text << "RIGHT";
		break;
	case 30:
		_text << "UP";
		break;
	case 31:
		_text << "DOWN";
		break;
	case 32:
		_text << "SPACE";
		break;
	default:
		_text << (char)_key;		
	}
	_text << "]";

	// definir la touche pour l'application
	g_BeTrisApp->SetControlKey(_key,_index);

	// ok
	return true;
}
