/************************************/
/* vue contenant les plateau de jeu */
/************************************/
#include "BeTrisFieldsView.h"
#include "BeTrisConstants.h"
#include "BeTrisMessagesDefs.h"
#include "BeTrisApp.h"
#include "BeTrisFieldView.h"
#include "BeTrisNextBlockView.h"
#include "BeTrisOptionListView.h"
#include "BeTrisNetwork.h"
#include "BeTrisPlayerView.h"

#include <ScrollView.h>
#include <Control.h>
#include <TextControl.h>
#include <Font.h>
#include <stdio.h>

// structure des block speciaux pour afficher l'info
// quand on en utilise un
struct _special_block _special_block_info[] =
{
    {SP_ADD1LINE, -1, "1 Line Added"},
    {SP_ADD2LINE, -1, "2 Lines Added"},
    {SP_ADD4LINE, -1, "4 Lines Added"},
    {"a",   6,  "Add Line"},
    {"c",   7,  "Clear Line"},
    {"n",   8,  "Nuke Field"},
    {"r",   9,  "Clear Random"},
    {"s",  10,  "Switch Fields"},
    {"b",  11,  "Clear Specials"},
    {"g",  12,  "Block Gravity"},
    {"q",  13,  "Blockquake"},
    {"o",  14,  "Block Bomb"},
    {"", 0, ""}
};

/**** constructeur ****/
BeTrisFieldsView::BeTrisFieldsView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// initialiser
	_ppmm = 0;
	_ppmmax = 0;

	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	// police standard
	_plain_font = new BFont(be_plain_font);
	_bold_font = new BFont(be_bold_font);
	_plain_font->SetSize(8.0);
	_bold_font->SetSize(9.0);

	/**** la visualisation de la prohaine piece ****/
	_nextblock = new BeTrisNextBlockView(BPoint(203,20),"the-next-block");

	/**** les jeux ****/
	_playerfield[0] = new BeTrisFieldView(BPoint(3,20),"player1",_nextblock,BeTrisFieldView::Large_Field);
	_playerfield[1] = new BeTrisFieldView(BPoint(307,20),"player2",NULL,BeTrisFieldView::Small_Field);
	_playerfield[2] = new BeTrisFieldView(BPoint(411,20),"player3",NULL,BeTrisFieldView::Small_Field);
	_playerfield[3] = new BeTrisFieldView(BPoint(515,20),"player4",NULL,BeTrisFieldView::Small_Field);
	_playerfield[4] = new BeTrisFieldView(BPoint(411,220),"player5",NULL,BeTrisFieldView::Small_Field);
	_playerfield[5] = new BeTrisFieldView(BPoint(515,220),"player6",NULL,BeTrisFieldView::Small_Field);

	/**** les zones de textes des joueurs ****/
	_playertext[0] = new BeTrisPlayerView(BPoint(3,3),"player1",BeTrisPlayerView::Large_Field);
	_playertext[1] = new BeTrisPlayerView(BPoint(307,3),"player2",BeTrisPlayerView::Small_Field);
	_playertext[2] = new BeTrisPlayerView(BPoint(411,3),"player3",BeTrisPlayerView::Small_Field);
	_playertext[3] = new BeTrisPlayerView(BPoint(515,3),"player4",BeTrisPlayerView::Small_Field);
	_playertext[4] = new BeTrisPlayerView(BPoint(411,203),"player5",BeTrisPlayerView::Small_Field);
	_playertext[5] = new BeTrisPlayerView(BPoint(515,203),"player6",BeTrisPlayerView::Small_Field);

	/**** info next block ****/
	_infonextblock = new BTextView(BRect(203,5,270,20),"next-block",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infonextblock->SetText("Next block");
	_infonextblock->SetViewColor(U_GRIS_STANDARD);
	_infonextblock->MakeSelectable(false);
	_infonextblock->MakeEditable(false);

	/**** info lines ****/
	_infolines = new BTextView(BRect(203,90,270,110),"info-lines",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infolines->SetText("Lines : -");
	_infolines->SetViewColor(U_GRIS_STANDARD);
	_infolines->MakeSelectable(false);
	_infolines->MakeEditable(false);

	/**** info levels ****/
	_infolevel = new BTextView(BRect(203,110,270,130),"info-level",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_infolevel->SetText("Level : -");
	_infolevel->SetViewColor(U_GRIS_STANDARD);
	_infolevel->MakeSelectable(false);
	_infolevel->MakeEditable(false);

	/**** info level moyen ****/
	_averagelevel = new BTextView(BRect(203,130,270,150),"average-level",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_averagelevel->SetText("Average : -");
	_averagelevel->SetViewColor(U_GRIS_STANDARD);
	_averagelevel->MakeSelectable(false);
	_averagelevel->MakeEditable(false);

	/**** info ppm instantane ****/
	_ppmview = new BTextView(BRect(203,150,270,165),"ppmi-level",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_ppmview->SetText("PPMi : -");
	_ppmview->SetViewColor(U_GRIS_STANDARD);
	_ppmview->MakeSelectable(false);
	_ppmview->MakeEditable(false);

	/**** info ppm moyen ****/
	_ppmmview = new BTextView(BRect(203,165,270,180),"ppmm-level",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_ppmmview->SetText("PPMm : -");
	_ppmmview->SetViewColor(U_GRIS_STANDARD);
	_ppmmview->MakeSelectable(false);
	_ppmmview->MakeEditable(false);

	/**** info ppm max ****/
	_ppmmaxview = new BTextView(BRect(203,180,270,195),"ppmM-level",BRect(0,0,190,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_ppmmaxview->SetText("PPMM : -");
	_ppmmaxview->SetViewColor(U_GRIS_STANDARD);
	_ppmmaxview->MakeSelectable(false);
	_ppmmaxview->MakeEditable(false);

	/**** special block info ****/
	_specialblockinfo = new BTextView(BRect(205,205,389,371),"special-block-info",BRect(0,0,185,190),B_FOLLOW_NONE,B_WILL_DRAW);
	_specialblockinfo->SetStylable(true);
	_specialblockinfo->MakeSelectable(false);
	_specialblockinfo->MakeEditable(false);

	/**** liste pour les messages ****/
	_messagelist = new BTextView(BRect(4,403,596,481),"message-action",BRect(0,0,592,43),B_FOLLOW_NONE,B_WILL_DRAW);
	_messagelist->SetStylable(true);
	_messagelist->SetViewColor(0,0,0,0);
	_messagelist->MakeSelectable(false);
	_messagelist->MakeEditable(false);

	/**** special block text ****/
	_textoptions = new BTextView(BRect(2,380,112,395),"special-block-text",BRect(0,0,105,20),B_FOLLOW_NONE,B_WILL_DRAW);
	_textoptions->SetText("No Special Blocks");
	_textoptions->SetViewColor(U_GRIS_STANDARD);
	_textoptions->MakeSelectable(false);
	_textoptions->MakeEditable(false);
	
	/**** liste des options ****/
	_listeoptions = new BeTrisOptionListView(BPoint(115,379),"list-options");

	/**** champ edit pour les messages ****/
	_messageedit = new BTextControl(BRect(4,486,596,506),"gmsg-edit","Game Message : ","",new BMessage(U_ENTER_GSGM_MSG));
	_messageedit->TextView()->SetMaxBytes(MAX_GMSG_LEN_MESSAGE);
	_messageedit->TextView()->SetViewColor(U_GRIS_TRES_FONCE);
	_messageedit->TextView()->MakeSelectable(false);
	_messageedit->SetHighColor(U_GRIS_TRES_FONCE);
	_messageedit->SetDivider(85);
	_messageedit->SetEnabled(false);

	for(int32 i=0;i<BETRIS_MAX_CLIENTS;i++)
		AddChild(_playerfield[i]);

	for(int32 i=0;i<BETRIS_MAX_CLIENTS;i++)
		AddChild(_playertext[i]);

	AddChild(_infonextblock);
	AddChild(_nextblock);
	AddChild(_infolines);
	AddChild(_infolevel);
	AddChild(_averagelevel);
	AddChild(_ppmview);
	AddChild(_ppmmview);
	AddChild(_ppmmaxview);
	AddChild(_textoptions);
	AddChild(_listeoptions);
	AddChild(_messageedit);
	AddChild(new BScrollView("scroll-special-action",_specialblockinfo,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	AddChild(new BScrollView("scroll-message-action",_messagelist,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
}

/**** destructeur ****/
BeTrisFieldsView::~BeTrisFieldsView()
{
	// liberer la memoire des polices
	delete _plain_font;
	delete _bold_font;
}

/**** attached to window ****/
void BeTrisFieldsView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// cible des messages
	_messageedit->SetTarget(this);

	// aller en fin de liste de message
	_messagelist->ScrollToSelection();

	// placer le focus pour recevoir les evenements clavier
	if(!_playerfield[0]->IsFocus())
		_playerfield[0]->MakeFocus(true);
}

/**** Messaged received ****/
void BeTrisFieldsView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// message pendant la partie
	case U_ENTER_GSGM_MSG:
		_send_game_message();
		break;
	default:
		BView::MessageReceived(message);
	}
}

/***** afficher l'utilisation d'un block special ****/
void BeTrisFieldsView::SpecialBlockInfo(const char *from,const char *to,const char *specialBlock)
{
	// si les parametres sont NULL c'est que l'on veut tout effacer
	if(from==NULL && to==NULL && specialBlock==NULL)
		_specialblockinfo->SetText("");
	else
	{
		BeTrisPlayerItem	*player = NULL;
		rgb_color			destColor = U_BLEU_FONCE;
		BString 			text;

		// si on est le destinataire on ecrit en rouge
		// sauf si on s'envoi a soit-meme une option
		player = g_BeTrisApp->ActivatedPlayer();
		if(player!=NULL && strcmp(player->NickName(),to)==0 && strcmp(from,to)!=0)
			destColor = U_ROUGE_FONCE;

		// ajouter la ligne 
		text << (int32)(_specialblockinfo->CountLines());
		text << ". ";

		// ajouter l'action
		for(char i=0;_special_block_info[i]._block!=0;i++)
			if(strcmp(specialBlock,_special_block_info[i]._id)==0)
				text << _special_block_info[i]._info;
		
		// quel special block est-ce
		_specialblockinfo->SetFontAndColor(_bold_font,B_FONT_ALL,&U_VIOLET_FONCE);	
		_specialblockinfo->Insert(text.String());

		// to
		_specialblockinfo->SetFontAndColor(_plain_font,B_FONT_ALL,&U_COULEUR_NOIR);	
		_specialblockinfo->Insert(" on ");
	
		// le destinataire du block special
		_specialblockinfo->SetFontAndColor(_bold_font,B_FONT_ALL,&destColor);	
		_specialblockinfo->Insert(to);

		// from
		_specialblockinfo->SetFontAndColor(_plain_font,B_FONT_ALL,&U_COULEUR_NOIR);	
		_specialblockinfo->Insert(" from ");

		// l'expediteur du block special
		_specialblockinfo->SetFontAndColor(_bold_font,B_FONT_ALL,&destColor);	
		_specialblockinfo->Insert(from);

		// fin de ligne et scrolling
		_specialblockinfo->Insert("\n");
		_specialblockinfo->ScrollToSelection();		
	}
}

/**** on recoit un block special a traiter ****/
void BeTrisFieldsView::DoSpecialBlock(char from,char to,const char *specialBlock)
{
	BeTrisPlayerItem	*player = NULL;
	char				index;

	// recuperer notre numero de joueur
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;

	// est-on encore dans la partie ?
	if(player->EndGameTime()!=0)
		return;
	
	// trouver quelle type d'action c'est
	for(index=0;_special_block_info[index]._block!=0;index++)
		if(strcmp(_special_block_info[index]._id,specialBlock)==0)
			break;
			
	// a-t-on trouve
	if(_special_block_info[index]._block==0)
		return;

	// si c'est un ajout de ligne suite a un tetris ou autre
	// on doit verifier l'equipe, car si on est dans la meme equipe
	// on ne subit pas les ajouts des lignes
	if(from!=0 && (index==S_ADDALL4 || index==S_ADDALL2 || index==S_ADDALL1))
	{
		BeTrisPlayerItem	*playerFrom = NULL;
		
		// recuperer le joueur expediteur
		playerFrom = g_BeTrisApp->GetClientPlayer(from);
		if(playerFrom!=NULL && strcmp(playerFrom->Team(),player->Team())==0)
			return;
	}

	// cas exeptionnel, le "s" si on est le from ou le to on doit le gerer
	// dans ce cas on se placera toujours en destinataire pour le traitement
	if(index==S_SWITCH && from==player->Number())
	{
		// inverser
		from = to;
		to = player->Number();
	}
	
	// c'est pour nous ou pour tout le monde
	if(to!=player->Number() && to!=0)
		return;

	// verifier si c'est pour tout le monde
	// sinon verifier que ca nous concerne
	if(to==0)
	{
		// on ne s'envoi pas les augmentation de ligne en mode classic
		if(from==player->Number())
			return;
		
		// ok c'est pas de moi
		to=player->Number();
	}
	
	// demander l'action au champ directement
	_playerfield[0]->DoSpecialBlock(from,to,index);
}

/**** rafraichir l'affichage des donnees ****/
void BeTrisFieldsView::RefreshDisplay()
{
	BeTrisPlayerItem	*player = NULL;
	_game_data			*gameData = NULL;
	BString				buffer;
	
	// donnees de la partie et joueur local
	gameData = g_BeTrisApp->GameData();
	player = g_BeTrisApp->ActivatedPlayer();

	// niveau actuel
	buffer = "Level : ";
	if(player!=NULL)
	{
		buffer << (int32)(player->Level());
		_infolevel->SetText(buffer.String());
	}
	
	// nombres de lignes deja enlevees
	buffer = "Lines : ";
	buffer << (int32)(gameData->_lineCount);
	_infolines->SetText(buffer.String());

	// niveau moyen
	buffer = "Average : ";
	buffer << (int32)(gameData->_average);
	_averagelevel->SetText(buffer.String());

	// niveau ppm
	buffer = "PPMi : ";
	buffer << (int32)(gameData->_ppm);
	_ppmview->SetText(buffer.String());

	// calcul du ppm max et du ppm moyen
	// niveau ppm moyen
	_ppmm = (_ppmm + gameData->_ppm) / 2;
	buffer = "PPMm : ";
	buffer << (int32)(_ppmm);
	_ppmmview->SetText(buffer.String());

	// niveau ppm max
	if(gameData->_ppm>_ppmmax)
		_ppmmax = gameData->_ppm;
	buffer = "PPMM : ";
	buffer << (int32)(_ppmmax);
	_ppmmaxview->SetText(buffer.String());

	// rafraichir
	_infolevel->Invalidate();
	_infolines->Invalidate();
	_averagelevel->Invalidate();
	
	// valider ou non les messages internes
	_messageedit->SetEnabled(g_BeTrisApp->GameStarted());
	_messageedit->TextView()->MakeSelectable(g_BeTrisApp->GameStarted());
}

/**** vider l'affichage ****/
void BeTrisFieldsView::EmptyDisplay()
{
	// remettre a zero les champs de jeu et le reste
	for(char index=0;index<BETRIS_MAX_CLIENTS;index++)
	{
		// vider les champs
		_playertext[index]->Update(NULL);
		_playerfield[index]->MakeEmpty();
	}
}

/**** mise a jour du niveau moyen ****/
void BeTrisFieldsView::UpdateAverageLevel()
{
	BeTrisPlayerItem	*player = NULL;
	_game_data			*gameData = NULL;
	char				count = 0;
	short				total = 0;
	char				index;

	// donnees de jeu
	gameData = g_BeTrisApp->GameData();

	// calcul du niveau moyen des joueurs locaux
	// on prend le notre aussi
	for(index=0;index<BETRIS_MAX_CLIENTS;index++)
	{
		player = g_BeTrisApp->GetClientPlayer(index);
		if(player!=NULL)
		{
			count++;
			total += player->Level();
		}
	}
	// on doit avoir des joueurs
    // calcul du niveau moyen
	if(count>0)
		gameData->_average = total/count;
					
	// mise a jour du timing en fonction du niveau moyen
	if(gameData->_levelAverage==B_CONTROL_ON)
	{
		if(gameData->_average<=100)
			gameData->_timing = (1005 - gameData->_average * 10) * 1000;
		else
			gameData->_timing = 5000;
	}
}

/**** mise a jour de l'info des block speciaux ****/
void BeTrisFieldsView::UpdateSpecialBlockInfo(char specialBlock)
{
	BString		specialInfo;
	char		blockDefs[2] = { '\0','\0'};

	// aucun block
	specialInfo = "No Special Blocks";
	if(specialBlock!='\0')
	{
		// ajouter l'action
		blockDefs[0] = g_BeTrisApp->BlockDesf(specialBlock);
		for(char i=0;_special_block_info[i]._block!=0;i++)
			if(strcmp(blockDefs,_special_block_info[i]._id)==0)
				specialInfo = _special_block_info[i]._info;		
	}

	// mise a jour du texte
	_textoptions->SetText(specialInfo.String());
}

/**** ajouter un message dans la partie ****/
void BeTrisFieldsView::AddGameMessage(const char *sender,const char *text)
{
	// le message doit etre valide l'expediteur aussi
	if(text!=NULL && sender!=NULL)
	{
		// ajouter le message a la liste
		_messagelist->SetFontAndColor(_plain_font,B_FONT_ALL,(rgb_color *)&U_GRIS_CLAIR);	
		_messagelist->Insert(sender);
		_messagelist->Insert(" ");
		_messagelist->Insert(text);
		_messagelist->Insert("\n");
		_messagelist->ScrollToSelection();
		
		// emmetre le son
		g_BeTrisApp->PlaySound(SND_MESSAGE_GAME);
	}
}

// ==================
// Fonctions internes
// ==================

/**** envoyer un message en partie ****/
void BeTrisFieldsView::_send_game_message()
{
	// la partie doit etre en cours
	if(g_BeTrisApp->GameStarted())
	{
		BeTrisPlayerItem	*player = NULL;
	
		// recuperer notre joueur
		player = g_BeTrisApp->ActivatedPlayer();
		if(player!=NULL && strlen(_messageedit->Text())>0)
		{
			BMessage			gameMessage(BETRIS_NETWORK_MSG);
			BString				gameStr;
		
			// construire le message a envyer au reseau
			gameStr = g_BeTrisApp->Command(CMD_GMSG);
			gameStr << " <";
			gameStr << player->NickName();
			gameStr << "> ";
			gameStr << _messageedit->Text();
			
			// envoyer le message
			gameMessage.AddString(BETRIS_NETWORK_DATAS,gameStr);
			gameMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
			g_network_messenger.SendMessage(&gameMessage);
			
			// vider le champ edit
			_messageedit->SetText("");
		}
	}
	
	// remettre le focus sur le champ de jeu
	_playerfield[0]->MakeFocus(true);
}

