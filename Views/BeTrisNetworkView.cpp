#include "BeTrisMessagesDefs.h"
#include "BeTrisConstants.h"
#include "BeTrisNetworkView.h"
#include "CTabView.h"
#include "BeTrisNetwork.h"
#include "BeTrisServeurItem.h"
#include "CPreferenceFile.h"
#include "BeTrisOptionListView.h"

#include <Application.h>
#include <ScrollView.h>
#include <stdio.h>

/**** constructeur ****/
BeTrisNetworkView::BeTrisNetworkView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	// initialisation
	_isInRefresh = false;
	_quitPingThread = false;

	/**** groupe pour le client ****/
	_clientconfig = new BBox(BRect(10,10,610,155),"client-configuration");	
	_clientconfig->SetLabel(" Client Configuration ");
	AddChild(_clientconfig);
	
	/**** liste des serveur ****/
	_listserveur = new BListView(BRect(15,18,440,134),"serveur-list");
	_listserveur->SetSelectionMessage(new BMessage(U_SELECT_SERVER_MSG));

	/**** boutons connect et refresh ****/
	_connectdisconnect = new BButton(BRect(465,18,590,38),"connect-disconnect","Connect",new BMessage(U_CLIENT_CREATE_MSG));
	_connectdisconnect->SetEnabled(false);
	_refreshlist = new BButton(BRect(465,48,590,68),"refresh-serveur-list","Refresh",new BMessage(U_REFRESH_LIST_MSG));
	_removeserveur = new BButton(BRect(465,78,590,98),"remove-server","Remove",new BMessage(U_REMOVE_SERVER_MSG));
	_removeserveur->SetEnabled(false);
	
	/**** zone de saisie d'une ip ****/
	_enterip = new BTextControl(BRect(465,115,590,135),"enter-ip","IP","",new BMessage(U_ADD_SERVER_IP_MSG));
	_enterip->SetDivider(12);
	
	_clientconfig->AddChild(new BScrollView("scroll-serveur-list",_listserveur,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	_clientconfig->AddChild(_refreshlist);
	_clientconfig->AddChild(_connectdisconnect);
	_clientconfig->AddChild(_removeserveur);
	_clientconfig->AddChild(_enterip);
	
	/**** groupe pour la partie ****/
	_gameconfig = new BBox(BRect(10,160,365,495),"game-configuration");	
	_gameconfig->SetLabel(" Game Configuration ");
	AddChild(_gameconfig);

	/**** sliders pour la hauteur du debut du jeu ****/
	_stackheight = new BSlider(BRect(10,20,140,50),"stack-height","Stack height at start",NULL,0,10);
	_stackheight->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_stackheight->SetHashMarkCount(11);
	_stackheight->SetLimitLabels("0","10");

	/**** sliders pour la configuration du nombre d'option ****/
	_optioncapacity = new BSlider(BRect(155,20,340,50),"option-capacity","Capacity of special block inventory",NULL,1,18);
	_optioncapacity->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_optioncapacity->SetHashMarkCount(SP_MAX_CAPACITY);
	_optioncapacity->SetLimitLabels("1","18");

	/**** sliders pour le level de demarrage ****/
	_startlevel = new BSlider(BRect(10,68,340,108),"start-level","Starting level",NULL,1,50);
	_startlevel->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_startlevel->SetHashMarkCount(50);
	_startlevel->SetLimitLabels("1","50");

	/**** sliders pour le nombre de ligne pour faire augmenter un level ****/
	_makeline = new BSlider(BRect(10,116,195,156),"make-line-level","Lines to make before level increase",NULL,1,30);
	_makeline->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_makeline->SetHashMarkCount(30);
	_makeline->SetLimitLabels("1","30");

	/**** sliders pour le nombre de level a ajouter ****/
	_numberline = new BSlider(BRect(200,116,340,156),"number-line-level","Number of levels to increase",NULL,1,5);
	_numberline->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_numberline->SetHashMarkCount(5);
	_numberline->SetLimitLabels("1","5");

	/**** sliders pour le nombre de level a ajouter ****/
	_blockline = new BSlider(BRect(10,164,180,204),"number-line-for-blocks","Lines to make for special block",NULL,1,10);
	_blockline->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_blockline->SetHashMarkCount(10);
	_blockline->SetLimitLabels("1","10");

	/**** sliders pour le nombre de level a ajouter ****/
	_numberblock = new BSlider(BRect(185,164,340,204),"number-block-added","Number of special blocks added",NULL,0,5);
	_numberblock->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_numberblock->SetHashMarkCount(6);
	_numberblock->SetLimitLabels("0","5");

	/**** sliders pour regler le temps avant l'augmentation de ligne ****/
	_minuteadd = new BSlider(BRect(10,212,340,252),"minute-add","Minutes before lines start being added",NULL,0,30);
	_minuteadd->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_minuteadd->SetHashMarkCount(31);
	_minuteadd->SetLimitLabels("0","30");

	/**** sliders pour l'interval de temps entre chaque augmentation de ligne ****/
	_secondadd = new BSlider(BRect(10,260,340,300),"second-add","Seconds between lines being added",NULL,1,60);
	_secondadd->SetHashMarks(B_HASH_MARKS_BOTTOM);
	_secondadd->SetHashMarkCount(60);
	_secondadd->SetLimitLabels("1","60");

	/**** checkbox pour la moyenne des level et le comportement file/pile ****/
	_classicmode = new BCheckBox(BRect(5,310,165,330),"classic-mode","Classic Style Multiplayer Rules",NULL);
	_averagelevel = new BCheckBox(BRect(170,310,340,330),"average-level","All have player's averaged levels",NULL);

	/**** groupe pour le serveur ****/
	_serveurconfig = new BBox(BRect(370,160,610,495),"serveur-configuration");	
	_serveurconfig->SetLabel(" Server Configuration ");
	AddChild(_serveurconfig);

	/**** nom du serveur ****/
	_serveurname = new BTextControl(BRect(10,23,230,43),"serveur-name","Serveur name : ","",NULL);
	_serveurname->SetDivider(80);

	/**** nom du serveur ****/
	_serveurport = new BTextControl(BRect(10,50,130,70),"serveur-port","Server port : ",NULL,new BMessage(U_SERVEUR_PORT_MSG));
	_serveurport->SetModificationMessage(new BMessage(U_SERVEUR_PORT_MSG));
	_serveurport->SetDivider(80);

	/**** info de la list des IP's a bannires ****/
	_infobanip = new BTextView(BRect(15,85,230,105),"info-ban-ip",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infobanip->SetText("Ip Mask Ban List :");
	_infobanip->SetViewColor(U_GRIS_STANDARD);
	_infobanip->MakeSelectable(false);
	_infobanip->MakeEditable(false);

	/**** liste des IP's a bannires ****/
	_banips = new BListView(BRect(13,107,215,177),"ban-ip-list");
	
	/**** zone de saisie du masque ip ****/
	_maskip = new BTextControl(BRect(8,183,231,203),"mask-ip","","",new BMessage(U_MASK_IP_MSG));
	_maskip->SetDivider(0);

	/**** buttons add et remove des ip's et start serveur ****/
	_addip = new BButton(BRect(10,205,115,225),"add-ban-ip","Add",new BMessage(U_ADD_BAN_IP_MSG));
	_removeip = new BButton(BRect(120,205,232,225),"remove-ban-ip","Remove",new BMessage(U_REMOVE_BAN_IP_MSG));
	_startserveur = new BButton(BRect(75,295,225,315),"start-serveur","Start serveur",new BMessage(U_SERVEUR_STSTOP_MSG));
	
	// partie jeu
	_gameconfig->AddChild(_stackheight);
	_gameconfig->AddChild(_optioncapacity);
	_gameconfig->AddChild(_minuteadd);
	_gameconfig->AddChild(_secondadd);
	_gameconfig->AddChild(_startlevel);
	_gameconfig->AddChild(_makeline);
	_gameconfig->AddChild(_numberline);
	_gameconfig->AddChild(_blockline);
	_gameconfig->AddChild(_numberblock);
	_gameconfig->AddChild(_classicmode);
	_gameconfig->AddChild(_averagelevel);

	// partie serveur
	_serveurconfig->AddChild(_serveurname);
	_serveurconfig->AddChild(_serveurport);
	_serveurconfig->AddChild(_infobanip);
	_serveurconfig->AddChild(new BScrollView("scroll-ban-ip-list",_banips,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));	
	_serveurconfig->AddChild(_maskip);
	_serveurconfig->AddChild(_addip);
	_serveurconfig->AddChild(_removeip);
	_serveurconfig->AddChild(_startserveur);

	// charger les preferences
	_refresh_preferences();
	
	// mettre a jour en interne
	SavePreferences(false);
}

/**** destructeur ****/
BeTrisNetworkView::~BeTrisNetworkView()
{
	status_t		state;
	status_t		returnState;
	int32			nbItems;

	// quitter le thread du ping si il est en cours
	_quitPingThread = true;
	
	// attendre la fermeture du thread de reception
	state = wait_for_thread(_pingThreadID,&returnState);

	// vider la liste des serveurs
	nbItems = _listserveur->CountItems();
	for(int32 index=nbItems-1;index>=0;index--)
		delete (BeTrisServeurItem *)(_listserveur->RemoveItem(index));
}

/**** attachement de la fenetre ****/
void BeTrisNetworkView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// controles
	_enterip->SetTarget(this);
	_listserveur->SetTarget(this);
	_connectdisconnect->SetTarget(this);
	_refreshlist->SetTarget(this);
	_startserveur->SetTarget(this);	
	_removeserveur->SetTarget(this);

	// maj gui
	_update_gui();
}

/**** gestion des messages ****/
void BeTrisNetworkView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// l'etat de la connexion a changee
	case BETRIS_CONNEXION_CHANGE_MSG:
		_update_gui();
		break;
	// demarrer en server
	case U_CLIENT_CREATE_MSG:
		_connect_to_server();
		break;
	case U_SERVEUR_STSTOP_MSG:
		_start_stop_server();
		break;
	// ajouter une ip d'un serveur a la liste
	case U_ADD_SERVER_IP_MSG:
		_add_server_ip();
		break;
	// enlever un item serveur
	case U_REMOVE_SERVER_MSG:
		_remove_server();
		break;
	// selectionner un server
	case U_SELECT_SERVER_MSG:
		_select_server(message);
		break;
	// rafraichir la liste des serveurs
	case U_REFRESH_LIST_MSG:
		_refresh_servers_list();
		break;
	// rafraichir les preferences
	case BETRIS_REFRESH_PREFS:
		_refresh_preferences();
		break;
	// redessiner la liste des serveur
	case U_REDRAW_LIST_MSG:
		{
			int32	index = -1;
		
			// quel item doit-on rafraichir
			if(message->FindInt32(SERVER_ITEM_INDEX,&index)==B_OK)
				_listserveur->InvalidateItem(index);
		
			// rafraichir l'etat du bouton pour le dernier item
			if(index==(_listserveur->CountItems()-1))
				_refreshlist->SetEnabled(!_isInRefresh);
		}
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** sauver les preferences ****/
void BeTrisNetworkView::SavePreferences(bool save)
{
	CPreferenceFile		*prefs = NULL;
	BeTrisServeurItem	*serverItem = NULL;
	_game_data			*serverGameData = NULL;
	BMessage			archive;
	int32				nbServer;
	int32				port;

	// acces aux prefs
	prefs = g_BeTrisApp->PreferencesNetwork();
	serverGameData = g_BeTrisApp->GameDataServer();

	// informations sur les donnees de la partie
	// on les enregistre et en meme temps on met a jour les donnees
	// de l'application
	serverGameData->_initialStackHeight = _stackheight->Value();
	serverGameData->_initialLevel = _startlevel->Value();
	serverGameData->_linesperLevel = _makeline->Value();
	serverGameData->_levelInc = _numberline->Value();
	serverGameData->_specialLines = _blockline->Value();
	serverGameData->_specialCount = _numberblock->Value();
	serverGameData->_specialCapacity = _optioncapacity->Value();
	serverGameData->_classicMode = _classicmode->Value();
	serverGameData->_levelAverage = _averagelevel->Value();
	serverGameData->_mblines = _minuteadd->Value();
	serverGameData->_sblines = _secondadd->Value();

	// le nom du serveur doit toujours etre enregistre
	prefs->SetString(_serveurname->Text(),"server-name");

	// si on le veux sinon c'est juste pour la maj des donnees en interne
	if(save)
	{
		// enregistrer le port
		sscanf(_serveurport->Text(),"%ld",&port);
		if(port<=0 || port>=65535)
			port = BETRIS_NETWORK_PORT;
		prefs->SetInt32(port,"server-port");

		// enregistrer dans les preferences
		prefs->SetInt8(serverGameData->_initialStackHeight,"initial-stack-height");
		prefs->SetInt32(serverGameData->_initialLevel,"initial-level");
		prefs->SetInt8(serverGameData->_linesperLevel,"line-per-level");
		prefs->SetInt8(serverGameData->_levelInc,"level-increase");
		prefs->SetInt8(serverGameData->_specialLines,"line-to-make-special");
		prefs->SetInt8(serverGameData->_specialCount,"special-block-each-time");
		prefs->SetInt8(serverGameData->_specialCapacity,"special-line-capacity");
		prefs->SetInt32(serverGameData->_classicMode,"classic-mode");
		prefs->SetInt32(serverGameData->_levelAverage,"average-level");
		prefs->SetInt32(serverGameData->_mblines,"minutes-add");
		prefs->SetInt32(serverGameData->_sblines,"seconds-add");

		// liste des serveurs
		nbServer = _listserveur->CountItems();
		prefs->SetInt32(nbServer,"nb-server");
		for(int32 index=0;index<nbServer;index++)
		{
			serverItem = (BeTrisServeurItem *)(_listserveur->ItemAt(index));
			serverItem->Archive(&archive);
			prefs->SetMessage(&archive,"server",index);	
			archive.MakeEmpty();
		}

		// enregistrer les preferences
		prefs->Save();
	}
}

// =================
// = partie client =
// =================

/**** ajouter une Ip d'un serveur a la liste ****/
void BeTrisNetworkView::_add_server_ip()
{
	// on va avoir un problme si il n'y a pas d'ip ou de nom de serveur
	if(strlen(_enterip->Text())<=0)
		return;

	BeTrisServeurItem	*newServer = NULL;
	BString				serverIp;
	int32				group1 = -1;
	int32				group2 = -1;
	int32				group3 = -1;
	int32				group4 = -1;

	// verifier si l'ip est correcte
	sscanf(_enterip->Text(),"%ld.%ld.%ld.%ld",&group1,&group2,&group3,&group4);

	// est-ce une adresse ip
	if((group1>=0 && group1<=254) && (group2>0 && group2<=254) && (group3>=0 && group3<=254) && (group4>=0 && group4<=254))
	{

		// ok l'ip est correcte
		serverIp << group1 << "." << group2 << "." << group3 << "." << group4; 
		newServer = new BeTrisServeurItem();
		newServer->SetHostName(serverIp.String());
	}
	else
	{
		// c'est un nom de serveur on va simplement le retenir
		newServer = new BeTrisServeurItem();
		newServer->SetHostName(_enterip->Text());
	}

	// ajouter a la liste si on a creer un item
	if(newServer!=NULL)
		_listserveur->AddItem(newServer);
}

/**** selectionner un serveur de jeux ****/
void BeTrisNetworkView::_select_server(BMessage *message)
{
	// on ne doit pas etre deja connecté
	if(g_BeTrisApp->IsConnected())
		return;
	
	// mettre a jour la gui
	_update_gui();
}

/**** se connecter a un serveur ****/
void BeTrisNetworkView::_connect_to_server()
{
	// est-on connecte ?
	// si oui le bouton doit deconnecter
	if(g_BeTrisApp->IsConnected())
	{
		BMessage	close(CONNECTION_CLOSE_MSG);

		// changer le bouton en connexion
		_connectdisconnect->SetLabel("Connect");
		
		// se deconnecter
		close.AddInt8(BETRIS_NETWORK_SLOT,0);
		g_network_messenger.SendMessage(&close);
	}
	else
	{
		int32	index;
	
		// ajouter l'IP du serveur selectionné
		// on doit avoir une selection
		index = _listserveur->CurrentSelection();
		if(index<0)
			return;
	
		BeTrisServeurItem	*server = NULL;
		BMessage			create(NETWORK_CREATE_CLIENT_MSG);
		BString				hostname;
		int32				port;

		// pour le port on garde celui defini dans la partie serveur
		server = (BeTrisServeurItem *)(_listserveur->ItemAt(index));
		hostname = server->HostName();
		sscanf(_serveurport->Text(),"%ld",&port);
		if(port<0 || port>65535)
			port = BETRIS_NETWORK_PORT;
	
		create.AddInt32(NETWORK_PORT,port);
		create.AddString(NETWORK_ADDRESS,hostname);

		// envoyer la demande de connection
		g_network_messenger.SendMessage(&create);
	}
}

// =================
// = partie client =
// =================

/**** demarrer en tant que serveur ****/
void BeTrisNetworkView::_start_stop_server()
{	
	// verifier si le serveur est deja demarre
	if(g_BeTrisApp->ServerIsRunning())
	{
		// on va alors l'arreter
		BMessage	stopServer(NETWORK_STOP_SERVEUR_MSG);

		// tout deconnecter
		g_network_messenger.SendMessage(&stopServer);
	}
	else
	{
		BMessage	startServer(NETWORK_START_SERVEUR_MSG);
		int32		port;
	
		// on doit annuler la possibilité de se connecter a un autre serveur
		_listserveur->DeselectAll();

		// enregistrer les donnees de la partie
		SavePreferences(false);

		// envoyer le message
		sscanf(_serveurport->Text(),"%ld",&port);
		if(port<=0 || port>=65535)
			port = BETRIS_NETWORK_PORT;

		startServer.AddInt32(NETWORK_PORT,port);
		startServer.AddBool(NETWORK_LOCAL_PLAYER,(g_BeTrisApp->ActivatedPlayer()!=NULL));
		g_network_messenger.SendMessage(&startServer);
	}
}

/**** enlever un serveur de la liste ****/
void BeTrisNetworkView::_remove_server()
{
	int32	index = -1;

	// trouver l'item a supprimer
	index = _listserveur->CurrentSelection();
	if(index<0)
		return;

	// supprimer
	delete (BeTrisServeurItem *)(_listserveur->RemoveItem(index));
}

// ===========================
// = Gestion des preferences =
// ===========================

/**** sauver les preferences ****/
void BeTrisNetworkView::_refresh_preferences()
{
	CPreferenceFile		*prefs = NULL;
	BeTrisServeurItem	*serverItem = NULL;
	_game_data			*serverGameData = NULL;
	BString				buffer;
	int32				port;
	BMessage			archive;
	int32				nbServer;
	
	// acces aux prefs et donnees de partie
	prefs = g_BeTrisApp->PreferencesNetwork();
	serverGameData = g_BeTrisApp->GameDataServer();
	
	// port
	port = prefs->GetInt32(BETRIS_NETWORK_PORT,"server-port");
	if(port<=0 || port>=65535)
		port = BETRIS_NETWORK_PORT;
	buffer << (int32)port;
	_serveurport->SetText(buffer.String());

	// informations sur les donnees de la partie
	_stackheight->SetValue(prefs->GetInt8(0,"initial-stack-height"));
	_startlevel->SetValue(prefs->GetInt32(1,"initial-level"));
	_makeline->SetValue(prefs->GetInt8(2,"line-per-level"));
	_numberline->SetValue(prefs->GetInt8(1,"level-increase"));
	_blockline->SetValue(prefs->GetInt8(1,"line-to-make-special"));
	_numberblock->SetValue(prefs->GetInt8(1,"special-block-each-time"));
	_optioncapacity->SetValue(prefs->GetInt8(18,"special-line-capacity"));
	_classicmode->SetValue(prefs->GetInt32(B_CONTROL_OFF,"classic-mode"));
	_averagelevel->SetValue(prefs->GetInt32(B_CONTROL_ON,"average-level"));
	_minuteadd->SetValue(prefs->GetInt32(10,"minutes-add"));
	_secondadd->SetValue(prefs->GetInt32(10,"seconds-add"));

	// le nom du serveur doit toujours etre enregistre
	buffer = prefs->GetString("BeNetTrisServeur","server-name");
	_serveurname->SetText(buffer.String());

	// charger la liste de serveur
	nbServer = prefs->GetInt32(0,"nb-server");
	for(int32 index=0;index<nbServer;index++)
	{
		archive = prefs->GetMessage(archive,"server",index);	
		serverItem = new BeTrisServeurItem(&archive);
		_listserveur->AddItem(serverItem);
	}
}

/**** changer l'etat de la gui ****/
void BeTrisNetworkView::_update_gui()
{
	bool	connectionState;
	bool	playerActivated;
	bool	serverRunning;
	bool	gameStarted;

	// etat de connection
	connectionState = g_BeTrisApp->IsConnected();
	playerActivated = (g_BeTrisApp->ActivatedPlayer()!=NULL);
	serverRunning = g_BeTrisApp->ServerIsRunning();
	gameStarted = g_BeTrisApp->GameStarted();

	// label bouton connexion
	if(connectionState)
		_connectdisconnect->SetLabel("Disconnect");
	else
		_connectdisconnect->SetLabel("Connect");

	// etat du bouton de connexion et bouton remove
	_connectdisconnect->SetEnabled(((connectionState || _listserveur->CurrentSelection()>=0) && playerActivated) && !serverRunning);
	_removeserveur->SetEnabled(!connectionState && _listserveur->CurrentSelection()>=0);
	_startserveur->SetEnabled(!connectionState || g_BeTrisApp->ServerIsRunning());
	_refreshlist->SetEnabled(!_isInRefresh);

	// le serveur est-il demarre ?
	if(serverRunning)
		_startserveur->SetLabel("Stop Server");
	else
		_startserveur->SetLabel("Start Server");

	// reafficher
	_startserveur->Invalidate();
	_connectdisconnect->Invalidate();

	// si la partie est lance en mode serveur
	// annuler les modification de parametres
	_stackheight->SetEnabled(!(gameStarted && serverRunning));
	_optioncapacity->SetEnabled(!(gameStarted && serverRunning));
	_minuteadd->SetEnabled(!(gameStarted && serverRunning));
	_secondadd->SetEnabled(!(gameStarted && serverRunning));
	_startlevel->SetEnabled(!(gameStarted && serverRunning));
	_makeline->SetEnabled(!(gameStarted && serverRunning));
	_numberline->SetEnabled(!(gameStarted && serverRunning));
	_blockline->SetEnabled(!(gameStarted && serverRunning));
	_numberblock->SetEnabled(!(gameStarted && serverRunning));
	_averagelevel->SetEnabled(!(gameStarted && serverRunning));
	_classicmode->SetEnabled(!(gameStarted && serverRunning));
}

/**** rafraichir la liste des serveurs ****/
void BeTrisNetworkView::_refresh_servers_list()
{
	// ok on est entrain de rafraichir
	_refreshlist->SetEnabled(false);

	// lancer le thread de ping
	_pingThreadID = spawn_thread(_ping_thread,"benettris-ping-thread",B_NORMAL_PRIORITY,this);
	if(resume_thread(_pingThreadID)!=B_OK)
		_refreshlist->SetEnabled(true);	
}

/**** stub du thread du ping des serveurs ****/
int32 BeTrisNetworkView::_ping_thread(void *data)
{	return ((BeTrisNetworkView *)data)->_ping();	}

/**** thread du ping des serveurs ****/
int32 BeTrisNetworkView::_ping()
{
	BeTrisServeurItem	*serverItem = NULL;
	BMessage			refresh(U_REDRAW_LIST_MSG);
	BMessenger			messenger(this);
	BString				pingString;
	BString				string;
	FILE				*output;
	char				c;
	int32				nbItems;
	int32				timePos = -1;
	float				pingTime = 0;
	float				pingTimeAverage = 0;
	char				nbPingFind = 0;
	int32				index = 0;

	// ok on rafraichit
	_isInRefresh = true;
	_quitPingThread = false;
	refresh.AddInt32(SERVER_ITEM_INDEX,index);

	// parcourir la liste des serveurs
	nbItems = _listserveur->CountItems();
	for(index=0;(!_quitPingThread && index<nbItems);index++)
	{
		serverItem = (BeTrisServeurItem *)(_listserveur->ItemAt(index));
		if(serverItem!=NULL)
		{
			// afficher comme etant en cours de ping
			serverItem->SetInRefresh(true);
			refresh.ReplaceInt32(SERVER_ITEM_INDEX,index);
			messenger.SendMessage(&refresh);

			// initialisation
			pingTime = 0;
			pingTimeAverage = 0;
			nbPingFind = 0;
	
			// commande ping
			pingString = "ping -c 3 ";
			pingString << serverItem->HostName();
			pingString << " 2>&1";

			// ouvrir le pipe
			output = popen(pingString.String(), "r");
			while((c = getc(output))!= EOF)
   			{
   				if((char)c!='\n')
   					string <<(char)c;
				else
				{
					// trouver la chaine time=
					timePos = string.FindFirst("time=");
					if(timePos>0)
					{
						string.Remove(0,timePos);
						if(sscanf(string.String(),"time=%f ms",&pingTime)==1)
						{
							// recuperer le temp du ping pour en faire la moyenne
							pingTimeAverage += pingTime;
							nbPingFind++;
						}
					}
					string.SetTo("");
				}
			}
			// fermer le pipe
			pclose(output);
			
			// faire la moyenne
			if(nbPingFind>0)
				pingTimeAverage /= nbPingFind;
				
			// fixer le ping de cet item
			serverItem->SetPingTime(pingTimeAverage);
			
			// et reafficher
			serverItem->SetInRefresh(false);
			refresh.ReplaceInt32(SERVER_ITEM_INDEX,index);
			messenger.SendMessage(&refresh);
		}
	}

	// ok on rafraichit plus
	_isInRefresh = false;
	
	// on quitte le thread
	return B_OK;
}
