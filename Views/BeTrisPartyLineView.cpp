#include "BeTrisMessagesDefs.h"
#include "BeTrisPartyLineView.h"
#include "BeTrisConstants.h"
#include "CColorMenuItem.h"
#include "BeTrisConstants.h"
#include "CPreferenceFile.h"
#include "CTabView.h"
#include "BeTrisPlayerItem.h"
#include "BeTrisNetwork.h"
#include "BeTrisApp.h"

#include <Application.h>
#include <ScrollView.h>
#include <MenuField.h>
#include <stdio.h>

/**** constructeur ****/
BeTrisPartyLineView::BeTrisPartyLineView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);
	
	// police standard
	_fixed_font = new BFont(be_fixed_font);
	_bold_font = new BFont(be_bold_font);
	_fixed_font->SetSize(11.0);
	_bold_font->SetSize(11.0);

	// fichier des joueurs rejete banni etc...
	_rejectedplayers = new CPreferenceFile(P_FILE_REJECTEDS_PREFS,BASE_PREFERENCE_PATH);

	/**** text info ****/
	_infoteam = new BTextView(BRect(10,10,200,30),"info-team",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infoteam->SetText("Team You Are Playing On :");
	_infoteam->SetViewColor(U_GRIS_STANDARD);
	_infoteam->MakeSelectable(false);
	_infoteam->MakeEditable(false);
	
	/**** EditText pour le nom de l'equipe ****/
	_enterteam = new BTextControl(BRect(353,9,453,29),"info-team","","",new BMessage(U_ENTER_TEAM_MSG));
	_enterteam->TextView()->DisallowChar(' ');
	_enterteam->SetDivider(0);
	
	/**** Zone de defilement du chat (IRC) ****/
	_chattext = new BTextView(BRect(10,35,438,470),"chat-zone",BRect(0,0,428,403),B_FOLLOW_ALL_SIDES);
	_infoteam->MakeSelectable(false);
	_infoteam->MakeEditable(false);
	_chattext->SetStylable(true);
	
	/**** EditText pour le chat ****/
	_enterchat = new BeTrisChatEdit(BRect(40,479,453,499),"enter-chat",BRect(5,4,440,12));
	_enterchat->SetMaxBytes(MAX_CHAT_LEN_MESSAGE);

	/**** liste des joueurs ****/
	_playerlist = new BListView(BRect(460,35,595,205),"player-game-list");
	_playerlist->SetSelectionMessage(new BMessage(U_SELECT_PLAYER_MSG));

	/**** Buttons de la vue ****/
	_updateteam = new BButton(BRect(460,5,610,25),"update-team","Update Team",new BMessage(U_UPDATE_TEAM_MSG));	
	_rejectman = new BButton(BRect(460,215,610,235),"reject-man","Reject",new BMessage(U_REJECT_MAN_MSG));	
	_rejectman->SetEnabled(false);
	_kickman = new BButton(BRect(460,245,610,265),"kick-man","Kick",new BMessage(U_KICK_MAN_MSG));	
	_kickman->SetEnabled(false);
	_banman = new BButton(BRect(460,275,610,295),"ban-man","Ban",new BMessage(U_BAN_MAN_MSG));
	_banman->SetEnabled(false);
	_startgame = new BButton(BRect(460,475,610,501),"start-game","Start new game",new BMessage(U_START_STOP_GAME_MSG));
	_startgame->SetEnabled(false);
	
	/**** liste des mauvais joueurs ****/
	_badplayerlist = new BListView(BRect(460,370,595,470),"bad-player-list");
	
	/**** menu des couleur ****/
	BMenuField		*colormenu;
	_textcolor = new BPopUpMenu("popup-color");
	_textcolor->AddItem(new CColorMenuItem(U_COULEUR_NOIR,U_BLACK_COLOR,BString("0"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_GRIS_FONCE,U_GRAY_COLOR,BString("1"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_ROUGE_CLAIR,U_RED_COLOR,BString("2"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_VERT_CLAIR,U_LIME_COLOR,BString("3"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_ORANGE_CLAIR,U_MAROON_COLOR,BString("4"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_BLEU_CLAIR,U_BLUE_COLOR,BString("5"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_VIOLET_FONCE,U_PURPLE_COLOR,BString("6"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_TURQUOISE_STANDARD,U_CYAN_COLOR,BString("7"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_ROUGE_FONCE,U_OLIVE_COLOR,BString("8"),NULL));
	_textcolor->AddItem(new CColorMenuItem(U_VERT_FONCE,U_GREEN_COLOR,BString("9"),NULL));
	_textcolor->ItemAt(0)->SetMarked(true);

	colormenu = new BMenuField(BRect(6,477,56,517),"color-menu","",_textcolor);
	colormenu->SetDivider(0);

	AddChild(_infoteam);
	AddChild(_enterteam);
	AddChild(new BScrollView("scroll-chat-zone",_chattext,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	
	AddChild(_enterchat);
	AddChild(_updateteam);
	AddChild(_rejectman);
	AddChild(_kickman);
	AddChild(_banman);
	AddChild(_startgame);
	
	AddChild(new BScrollView("scroll-bad-player-list",_badplayerlist,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	AddChild(new BScrollView("scroll-team-list",_playerlist,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	AddChild(colormenu);
	
	// charger la liste des joueurs rejete, bannis ou kickes
	_load_bad_players();
}

/**** destructeur ****/
BeTrisPartyLineView::~BeTrisPartyLineView()
{
	// vider la liste des mauvais joueurs
	BeTrisPlayerItem	*player = NULL;
	int32				nbItems = 0;

	// parcourir la liste
	nbItems = _badplayerlist->CountItems();
	for(char index=(nbItems-1);index>=0;index--)
	{
		player = (BeTrisPlayerItem *)(_badplayerlist->RemoveItem(index));
		if(player!=NULL)
			delete player;
	}

	// vider la liste des joueurs
	_empty_list();
	
	// liberer la memoire des polices
	delete _fixed_font;
	delete _bold_font;
}

/**** attached to window ****/
void BeTrisPartyLineView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// definir la vue destinataire des message
	_updateteam->SetTarget(this);
	_startgame->SetTarget(this);
	_playerlist->SetTarget(this);
	_rejectman->SetTarget(this);
	_banman->SetTarget(this);
	_kickman->SetTarget(this);
	
	// mise a jour de la gui
	_update_gui();

	// scrolling en bas
	_chattext->ScrollToSelection();		
}

/**** Messaged received ****/
void BeTrisPartyLineView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// mise a jour de la gui
	case BETRIS_CONNEXION_CHANGE_MSG:
		_update_gui();
		break;
	// envoi de message
	case BETRIS_SEND_CHAT_MESSAGE_MSG:
		_send_chat_message();
		break;
	// on rejet un joueur
	case U_REJECT_MAN_MSG:
		_reject_player();
		break;
	// mise a jour de notre equipe
	case U_ENTER_TEAM_MSG:
	case U_UPDATE_TEAM_MSG:
		_update_team();
		break;
	// demarrage ou arret d'une partie
	case U_START_STOP_GAME_MSG:
		_start_stop_game();
		break;
	// selectionner un joueur
	case U_SELECT_PLAYER_MSG:
		_select_player(message);
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** ajouter un joueur ****/
bool BeTrisPartyLineView::AddPlayer(BeTrisPlayerItem *player)
{
	// on doit avoir un joueur
	if(player==NULL)
		return false;
	
	int32	nbPlayers;
	
	// verifier que ce joueur n'est pas deja dedans
	nbPlayers = _playerlist->CountItems();
	for(int32 index=0;index<nbPlayers;index++)
		if(strcmp(player->NickName(),((BeTrisPlayerItem *)(_playerlist->ItemAt(index)))->NickName())==0)
			return false;
	
	// ajouter le joueur
	_playerlist->AddItem(player);
	
	// ok on l'a ajoute
	return true;
}

/**** enlever un joueur ****/
void BeTrisPartyLineView::RemovePlayer(BeTrisPlayerItem *player)
{
	// si le parametre est NULL
	// on vire tout le monde
	if(player==NULL)
		_empty_list();
	else	
		_playerlist->RemoveItem(player);
}

/**** enregistre la liste des mauvais joueurs ****/
void BeTrisPartyLineView::SaveBadPlayers()
{
	BeTrisPlayerItem	*playerItem = NULL;
	BMessage			archive;
	int32				nbPlayer;

	// enregistrer la liste des mauvais joueurs
	nbPlayer = _badplayerlist->CountItems();
	_rejectedplayers->SetInt32(nbPlayer,"nb-player");
	for(int32 index=0;index<nbPlayer;index++)
	{
		playerItem = (BeTrisPlayerItem *)(_badplayerlist->ItemAt(index));
		playerItem->Archive(&archive);
		_rejectedplayers->SetMessage(&archive,"player",index);	
		archive.MakeEmpty();
	}
	
	// sauver les preferences
	_rejectedplayers->Save();
}

// ================================
// = Gestion des messages "chats" =
// ================================

/**** reception d'un message ****/
void BeTrisPartyLineView::AddChatMessage(char from,const char *text,rgb_color *color,bool info)
{
	// le message doit etre valide
	if(text==NULL)
		return;

	BeTrisPlayerItem	*player = NULL;

	// si c'est de l'info
	_chattext->SetFontAndColor(_fixed_font,B_FONT_ALL,color);	
	if(info)
		_chattext->Insert("*** ");
	
	// verifier si ca vient d'un client
	if(from>=0)
	{
		// font en gras
		_chattext->SetFontAndColor(_bold_font,B_FONT_ALL,color);	

		if(!info)
			_chattext->Insert("<");
		
		// recuperer et construire le texte du nom du joueur
		if(from>0)
		{
			player = g_BeTrisApp->GetClientPlayer(from);
			if(player!=NULL)
				_chattext->Insert(player->NickName());
		}
		else
			_chattext->Insert("Server");

		if(!info)
			_chattext->Insert(">");

		// un espace en plus
		_chattext->Insert(" ");

		// font standard
		_chattext->SetFontAndColor(_fixed_font,B_FONT_ALL,color);	
	}
	
	// ajouter le texte
	// on va le parcourir pour trouver des code couleur si il y en a
	BString		buffer;
	char		*bufferPtr = NULL;
	short		index;
	short		posColor;
	short		strSize;
	
	// recopier le texte
	buffer = text;
	if(buffer.Length()>0)
	{
		// a-t-on notre nickname mais pour du texte d'un client
		// si oui on emmet un son
		player = g_BeTrisApp->ActivatedPlayer();
		if(player!=NULL && from>=0)
		{
			index = buffer.FindFirst(player->NickName());
			if(index>=0)
			{
				// colorer le texte on le met en violet
				buffer.Insert((char)U_PURPLE_COLOR,1,index);
				buffer.Insert((char)U_BLACK_COLOR,1,index+strlen(player->NickName())+1);
				
				// emmetre le son
				g_BeTrisApp->PlaySound(SND_MESSAGE_GAME);
			}
		}

		// ok parcour du texte
		posColor = 0;
		strSize = buffer.Length();
		for(index=0;index<=strSize;index++)
		{
			if(buffer[index]<32 || index==strSize)
			{
				bufferPtr = buffer.LockBuffer(strSize);
				if(bufferPtr!=NULL)
				{
					_chattext->Insert(bufferPtr+posColor,index-posColor);
					buffer.UnlockBuffer();
				}

				if(buffer[index]<32)
				{
					color = g_BeTrisApp->TranslateColor(buffer[index]);
					if(color!=NULL)
						_chattext->SetFontAndColor(_fixed_font,B_FONT_ALL,color);	

					posColor = index + 1;
				}
			}
		}
		// caractere de fin de ligne et restaurer la police et la couleur
		_chattext->SetFontAndColor(_fixed_font,B_FONT_ALL,&U_COULEUR_NOIR);	
	}
	
	// fin de liste
	_chattext->Insert("\n");
	_chattext->ScrollToSelection();		
}

// ==================
// fonctions internes
// ==================

/**** envoyer un message ****/
void BeTrisPartyLineView::_send_chat_message()
{
	BeTrisPlayerItem	*player = NULL;
	BMessage			chatMessage(BETRIS_NETWORK_MSG);
	BMenuItem			*itemColor = NULL;
	BString				text;
	BString				editText;
	BString				message;
	char				colorID;
	bool				actionLine = false;
	int32				indexColor = -1;

	// on doit avoir du texte
	if(strlen(_enterchat->Text())<=0)
		return;

	itemColor = _textcolor->FindMarked();
	if(itemColor==NULL)
		return;

	// quel est l'index de l'item
	indexColor = _textcolor->IndexOf(itemColor);
	if(indexColor<0)
		return;

	// recuperer le joueur actif
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;

	// recuperer la couleur
	colorID = ((CColorMenuItem *)itemColor)->ColorId();

	// text du champ edit
	editText = _enterchat->Text();
	
	// at-on une ligne de texte ou une commande ?
	actionLine = (editText[0]=='/');

	// construire la chaine
	text = g_BeTrisApp->Command(CMD_PLINE);
	text << " ";
	text << (int32)(player->Number());
	text << " ";

	// la couleur est geré que pour le texte
	if(!actionLine && indexColor>0)
		text << (char)(colorID);
	
	// la commande ou le texte
	text << editText;
	
	// envoyer le message
	chatMessage.AddString(BETRIS_NETWORK_DATAS,text);
	chatMessage.AddInt8(BETRIS_NETWORK_SLOT,0);

	// envoi a l'application
	g_network_messenger.SendMessage(&chatMessage);

	// ajouter le message a notre texte
	// si c'est pas une commande
	if(!actionLine)
	{
		text = (char)(colorID);
		text << _enterchat->Text();
		AddChatMessage(player->Number(),text.String(),NULL,false);
	}

	// vider le controle edit
	_enterchat->SetText("");
}

/**** mise a jour de notre equipe ****/
void BeTrisPartyLineView::_update_team()
{
	// on va envoyer la demande de rejoindre une equipe
	// ou quitter si c'est vide
	// envoyer le message
	BeTrisPlayerItem	*player = NULL;
	BMessage			teamMessage(BETRIS_NETWORK_MSG);
	BString				text;

	// construire la demande
	// recuperer le joueur actif
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;
	
	// modifier en interne
	player->SetTeam(_enterteam->Text());

	text = g_BeTrisApp->Command(CMD_TEAM);
	text << " ";
	text << (int32)(player->Number());
	text << " " << _enterteam->Text();
	text << (char)0xFF;

	// envoi a l'application
	teamMessage.AddString(BETRIS_NETWORK_DATAS,text);
	teamMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
	g_network_messenger.SendMessage(&teamMessage);

	// envoyer le message en interne pour valider la modification
	be_app_messenger.SendMessage(&teamMessage);
}

/**** selection d'un joueur dans la liste ****/
void BeTrisPartyLineView::_select_player(BMessage *message)
{
	BeTrisPlayerItem	*actif = NULL;
	BMessage			archive;
	int32				selection;
	bool				activatebutton = false;
	
	// selection ou pas ?
	if(message->FindInt32("index",&selection)!=B_OK)
		return;

	// definir le joueur dans l'application
	actif = NULL;
	if(selection>=0)
	{
		// activer maintenant delui de selectionne
		// et afficher dans la zone de modification les donnees
		actif = (BeTrisPlayerItem *)(_playerlist->ItemAt(selection));
		activatebutton = (actif!=NULL);// && !actif->IsActivated());
	}

	// valider ou non les boutons
	_rejectman->SetEnabled(activatebutton);
	_kickman->SetEnabled(activatebutton);
	_banman->SetEnabled(activatebutton);
}

/**** on rejete un joueur ****/
void BeTrisPartyLineView::_reject_player()
{
	BeTrisPlayerItem	*player = NULL;
	BeTrisPlayerItem	*rejectedplayer = NULL;
	int32				selected = -1;

	// on va prendre le joueur qui est selectionne dans la liste
	// mais pas nous meme !
	selected = _playerlist->CurrentSelection();
	if(selected<0)
		return;
		
	// ok recuperer le joueur
	player = (BeTrisPlayerItem *)(_playerlist->ItemAt(selected));
	if(player!=NULL && !player->IsActivated())
	{
		BMessage	archive;
	
		// on passe par une archive pour recopier le joueur
		if(player->Archive(&archive)==B_OK)
		{
			// ajouter dans la liste des rejete
			rejectedplayer = new BeTrisPlayerItem(&archive);
			_badplayerlist->AddItem(rejectedplayer);
		}
	}
}

/**** vider la liste des joueurs ****/
void BeTrisPartyLineView::_empty_list()
{
	// on ne supprime pas le joueurs
	// c'est l'application qui le fait
	_playerlist->MakeEmpty();
	
/*	BeTrisPlayerItem	*player = NULL;
	int32				nbItems = 0;

	// parcourir la liste
	nbItems = _playerlist->CountItems();
	for(char index=(nbItems-1);index>=0;index--)
	{
		player = (BeTrisPlayerItem *)(_playerlist->RemoveItem(index));
		if(player!=NULL)
			delete player;
	}
*/
}

/**** mise a jour de la gui ****/
void BeTrisPartyLineView::_update_gui()
{
	// demarrage de partie
	_startgame->SetEnabled(g_BeTrisApp->PlayerCountServer()>0 && g_BeTrisApp->ServerIsRunning());
	
	// bouton en fonction de l'etat de la partie
	if(!g_BeTrisApp->ServerIsRunning())
		return;

	if(g_BeTrisApp->GameStarted())
		_startgame->SetLabel("Stop current game");
	else
		_startgame->SetLabel("Start new game");	
}

/**** demarrage et arret d'une partie ****/
void BeTrisPartyLineView::_start_stop_game()
{
	// le serveur doit etre demarre
	if(!g_BeTrisApp->ServerIsRunning())
		return;

	BMessage	start(BETRIS_NETWORK_MSG);
	BString		command;

	// on va envoyer en interne la commande de demarrage ou d'arret
	// de la partie
	if(!g_BeTrisApp->GameStarted())
		command = g_BeTrisApp->Command(CMD_NEWGAME);
	else
		command = g_BeTrisApp->Command(CMD_ENDGAME);
	
	// envoyer la commande
	command << (char)0xFF;
	start.AddString(BETRIS_NETWORK_DATAS,command);
	BMessenger(be_app).SendMessage(&start);
}

/**** charger la liste des mauvais joueurs ****/
void BeTrisPartyLineView::_load_bad_players()
{
	BeTrisPlayerItem	*playerItem = NULL;
	BMessage			archive;
	int32				nbPlayer;
	
	// acces aux prefs
	_rejectedplayers->Load();
		
	nbPlayer = _rejectedplayers->GetInt32(0,"nb-player");
	for(int32 index=0;index<nbPlayer;index++)
	{
		// recuperer les infos
		archive.MakeEmpty();
		archive = _rejectedplayers->GetMessage(archive,"player",index);

		// creer le joueur et l'ajouter
		playerItem = new BeTrisPlayerItem(&archive);
		if(playerItem!=NULL)
			_badplayerlist->AddItem(playerItem);
	}
}

// ==============================================
// classe particuliere d'edition du texte de chat
// ==============================================

/**** constructeur ****/
BeTrisChatEdit::BeTrisChatEdit(BRect frame,const char *name,BRect textRect)
: BTextView(frame,name,textRect,B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
}

/**** destructeur ****/
BeTrisChatEdit::~BeTrisChatEdit()
{
}

/**** dessin ****/
void BeTrisChatEdit::Draw(BRect updateRect)
{
	// dessin parent
	BTextView::Draw(updateRect);

	// rectangle de la vue
	_bounds = Bounds();

	// cadre
	SetHighColor(U_GRIS_CLAIR);
	StrokeLine(BPoint(_bounds.left,_bounds.top),BPoint(_bounds.right,_bounds.top));
	StrokeLine(BPoint(_bounds.left,_bounds.top+1),BPoint(_bounds.left,_bounds.bottom));
	SetHighColor(U_GRIS_TRES_FONCE);
	StrokeLine(BPoint(_bounds.left+1,_bounds.top+1),BPoint(_bounds.right-1,_bounds.top+1));
	StrokeLine(BPoint(_bounds.left+1,_bounds.top+1),BPoint(_bounds.left+1,_bounds.bottom-1));
	SetHighColor(U_GRIS_STANDARD);
	StrokeLine(BPoint(_bounds.left+2,_bounds.bottom-1),BPoint(_bounds.right-1,_bounds.bottom-1));
	StrokeLine(BPoint(_bounds.right-1,_bounds.top+2),BPoint(_bounds.right-1,_bounds.bottom-1));
}

/**** gestion du clavier ****/
void BeTrisChatEdit::KeyDown(const char *bytes,int32 numBytes)
{
	// fonction parent
	if(numBytes>1)
	{
		BTextView::KeyDown(bytes,numBytes);
		return;
	}

	// interpreter les touches
	switch(bytes[0])
	{
	// Enter, on envoi le message
	case B_ENTER:
		{
			BView	*parent = NULL;
		
			// la vue doit etre attachee
			parent = Parent();
			if(parent==NULL)
				return;
				
			// ok on informe que l'on envoi un message
			BMessage	message(BETRIS_SEND_CHAT_MESSAGE_MSG);
			BMessenger(parent).SendMessage(&message);
		}
		break;
	// avec Tab on va essayer de completer le nom d'un joueur
	case B_TAB:
		{
			BString		text;
			
			// verifier le texte
			text = Text();
			if(text.Length()>0)
			{
				// verifier si on a un / en tete de texte
				// sinon completer le nom d'un joueur
				if(text[0]=='/')
					_find_command_complete_word(text);
				else	
					_find_player_complete_word(text);
			}
		}
		break;
	// par defaut on ne fait rien
	// ca appelera la fonction d'origine
	default:
		BTextView::KeyDown(bytes,numBytes);
	}
}

/**** completer le nom d'une commande ****/
void BeTrisChatEdit::_find_command_complete_word(BString &text)
{
	// on va rechercher si on a une commande qui correspond
	// on doit en trouver une seule
}

/**** completer le nom ****/
void BeTrisChatEdit::_find_player_complete_word(BString &text)
{
	// on doit avoir du texte
	// et au moins 3 lettres
	if(strlen(Text())<3)
		return;
		
	BeTrisPlayerItem	*player = NULL;
	BeTrisPartyLineView	*partyLine = NULL;
	BString				searchStr;
	BString				nickName;
	short				findPos = -1;
	short				spacePos = -1;

	// vue parent
	partyLine = (BeTrisPartyLineView *)(Parent());
	if(partyLine==NULL)
		return;
				
	// trouver l'emplacement d'un espace
	// sinon on se basera sur les 3 dernieres lettre
	spacePos = text.FindLast(' ');
	if(spacePos>=0)
		text.MoveInto(searchStr,spacePos + 1,text.Length()-spacePos);
	else
	{
		spacePos = 0;
		searchStr = text;
	}

	// parcourir la liste des joueur
	for(char index=0;(index<partyLine->_playerlist->CountItems() && findPos<0);index++)
	{
		player = (BeTrisPlayerItem *)(partyLine->_playerlist->ItemAt(index));
		if(player!=NULL)
		{
			nickName = player->NickName();
			findPos = nickName.IFindFirst(searchStr);
		}
	}
			
	// a-t-on trouve
	if(findPos>=0 && player!=NULL)
	{
		// Remplacer le texte
		text.Replace(searchStr.String(),player->NickName(),1,spacePos);
				
		// effacer le contenu du champ edit
		// puis le redefinir
		SetText(text.String());
				
		// et se positionner en fin
		Select(text.Length(),text.Length());
	}
}