/******************************/
/* vue de la liste des scores */
/******************************/
#include "BeTrisMessagesDefs.h"
#include "BeTrisWinnerView.h"
#include "BeTrisConstants.h"
#include "CPreferenceFile.h"
#include <ScrollView.h>
#include <ListItem.h>
#include <stdio.h>
#include "CTabView.h"

/*******************/
/* vue des gagnant */
/*******************/

/**** constructeur ****/
BeTrisWinnerView::BeTrisWinnerView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// initialiser
	_winners = NULL;

	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	// fichier des gagnant
	_winners = new CPreferenceFile(P_FILE_WINNERS_PREFS,BASE_PREFERENCE_PATH);

	/**** groupe pour la liste des scores ****/
	_winnergroupe = new BBox(BRect(15,15,610,495),"winner-groupe");	
	_winnergroupe->SetLabel(" Winners and scores ");
	AddChild(_winnergroupe);

	/**** liste des scores et des gagnants ****/
	_winnerlist = new BListView(BRect(15,25,570,430),"winner-list");
	_winnerlist->SetSelectionMessage(new BMessage(U_SELECT_ITEM_MSG));

	/**** bouton de remise a zero de la liste et des scores ****/
	_emptyscore = new BButton(BRect(12,440,100,460),"reset-score","Reset scores",new BMessage(U_EMPTY_SCORE_MSG));
	_winnergroupe->AddChild(new BScrollView("scroll-winner-list",_winnerlist,B_FOLLOW_LEFT | B_FOLLOW_TOP,0,false,true));
	_winnergroupe->AddChild(_emptyscore);

	// charger lesprefs
	_load_preferences();	
}

/**** destructeur ****/
BeTrisWinnerView::~BeTrisWinnerView()
{
	// supprimer le fichier des prefs
	if(_winners!=NULL)
		delete _winners;

	// vider la liste
	EmptyList();
}

/**** attached to window ****/
void BeTrisWinnerView::AttachedToWindow()
{
	// attachement normal
	BView::AttachedToWindow();

	// la liste des winners
	_winnerlist->SetTarget(this);

	// bouttons d'actions sur la liste
	_emptyscore->SetTarget(this);
}

/**** Messaged received ****/
void BeTrisWinnerView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// vider la liste
	case U_EMPTY_SCORE_MSG:
		EmptyList();
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** ajouter un gagnant ****/
void BeTrisWinnerView::AddWinner(BString &string)
{
	// verifier le parametre
	if(string.Length()<=0)
		return;

	BeTrisWinnerItem	*newItem = NULL;
	char				*winner = NULL;
	short				sizeStr = -1;
	short				pVirgule = -1;
	bool				isTeam = false;
	int32				score;

	// on va decouper la chaine
	// pour recuperer les elements
	sizeStr = string.Length();
	if(sizeStr<4)
		return;
	
	// chercher le point virgule
	pVirgule = string.FindFirst(';');

	// a-t-on trouve le point virgule
	if(pVirgule>0)
	{
		// acces au buffer de la chaine
		winner = string.LockBuffer(-1);
		if(winner!=NULL)
		{
			// vider le buffer
			memset(_winnerBuffer,0,WINNER_BUFFER_SIZE);
		
			// est-ce une equipe (on a un 't' au debut, sinon on a un 'p')
			isTeam = (winner[0]=='t');

			// nom du joueur ou equipe
			memcpy(_winnerBuffer,winner+1,pVirgule-1);

			// le score
			pVirgule++;
			if(pVirgule<=sizeStr)
				sscanf(winner+pVirgule,"%ld",&score);

			// creer l'item et l'ajouter a al liste
			newItem = new BeTrisWinnerItem(isTeam,_winnerBuffer,score);
			_winnerlist->AddItem(newItem);		
		
			// debloquer la chaine
			string.UnlockBuffer();
		}
	}
}

/**** vider la liste ****/
void BeTrisWinnerView::EmptyList()
{
	int32	nbItems;

	// vider les elements de la liste
	nbItems = _winnerlist->CountItems();
	for(int32 index=(nbItems-1);index>=0;index--)
		delete (BeTrisWinnerItem *)(_winnerlist->RemoveItem(index));
}

/**** recuperer les gagnants ****/
void BeTrisWinnerView::GetWinner(BString &string)
{
}

/**** sauver les preferences ****/
void BeTrisWinnerView::SavePreferences()
{
	// on doit avoir un fichiers de prefs
	if(_winners==NULL)
		return;
	
	BeTrisWinnerItem	*winnerItem = NULL;
	BMessage			archive;
	int32				nbWinners = 0;

	// liste gagnants
	nbWinners = _winnerlist->CountItems();
	_winners->SetInt32(nbWinners,"nb-winners");
	for(int32 index=0;index<nbWinners;index++)
	{
		winnerItem = (BeTrisWinnerItem *)(_winnerlist->ItemAt(index));
		winnerItem->Archive(&archive);
		_winners->SetMessage(&archive,"winner",index);	
		archive.MakeEmpty();
	}
		
	// enregistrer puis liberer la memoire
	_winners->Save();
}

// =================
// Fonction internes
// =================

/**** charger les preferences ****/
void BeTrisWinnerView::_load_preferences()
{
	// charger la liste des gagants
	if(_winners==NULL)
		return;
	
	BeTrisWinnerItem	*winnerItem = NULL;
	BMessage			archive;
	int32				nbWinners = 0;

	// charger les donnees
	_winners->Load();

	// charger la liste des gagnants
	nbWinners = _winners->GetInt32(nbWinners,"nb-winners");
	for(int32 index=0;index<nbWinners;index++)
	{
		archive = _winners->GetMessage(archive,"winner",index);	
		winnerItem = new BeTrisWinnerItem(&archive);
		_winnerlist->AddItem(winnerItem);
	}
}

/***********************************/
/* classe de list pour les gagnant */
/***********************************/

/**** constructeur ****/
BeTrisWinnerItem::BeTrisWinnerItem(bool isteam,char *name,int32 score)
: BListItem()
{
	// initialiser
	_init();

	// recopier le nom et si c'est une equipe
	_isTeam = isteam;
	if(name!=NULL)
		_name = name;

	// definir le score
	SetScore(score);
}

/**** constructeur avec archive ****/
BeTrisWinnerItem::BeTrisWinnerItem(BMessage *message)
: BListItem(message)
{
	int32		score = 0;
	
	// initialiser
	_init();

	// element perso
	message->FindString(WINNER_NAME,&_name);
	message->FindBool(WINNER_ISTEAM,&_isTeam);
	message->FindInt32(WINNER_SCORE,&score);

	// definir le score
	SetScore(score);	
}

/**** destructeur ****/
BeTrisWinnerItem::~BeTrisWinnerItem()
{
}

/**** dessin ****/
void BeTrisWinnerItem::DrawItem(BView *owner,BRect itemRect,bool drawEverything)
{
	// est-ce une equipe
	if(_isTeam)
		owner->DrawString("	<Team>",BPoint(itemRect.left+5,itemRect.top+10));
	else
		owner->DrawString("<Player>",BPoint(itemRect.left+5,itemRect.top+10));

	// nom du joueur ou de l'equipe
	_displayColorPlayer(owner,itemRect);

	// score
	owner->DrawString(_bufferString.String(),BPoint(itemRect.left+250,itemRect.top+10));
}

/**** mettre dans un message les infos ****/
status_t BeTrisWinnerItem::Archive(BMessage *message)
{
	if(message==NULL)
		return B_ERROR;
		
	status_t	state = B_OK;
	
	// blistitem
	BListItem::Archive(message);
	
	// archivage
	state &= message->AddString(WINNER_NAME,_name);
	state &= message->AddBool(WINNER_ISTEAM,_isTeam);
	state &= message->AddInt32(WINNER_SCORE,_score);
		
	// retourner le resultat
	return state;
}

/**** definir le score ****/
void BeTrisWinnerItem::SetScore(int32 score)
{
	// recuperer le score
	_score = score;

	// le score
	_bufferString = "";
	_bufferString << _score;
}

// ==================
// Fonctions internes
// ==================

/**** initialiser ****/
void BeTrisWinnerItem::_init()
{
	_name = "";
	_isTeam = false;
	_score = 0;
}

/**** afficher le joueur avec les couleurs ****/
void BeTrisWinnerItem::_displayColorPlayer(BView *owner,BRect &itemRect)
{
	char		*name = NULL;

	name = _name.LockBuffer(-1);
	if(name!=NULL)
	{
		rgb_color	*color = NULL;
		int32		strName = 0;
		char		indexChar = 0;
		char		c = '\0';

		owner->SetLowColor(U_COULEUR_BLANC);
		strName = strlen(name);
		for(int32 index=0;index<strName;index++)
		{
			c = name[index];
			if(c<32)
			{
				color = g_BeTrisApp->TranslateColor(c);
				if(color!=NULL)
					owner->SetHighColor(*color);
			}
			else
			{
				indexChar++;
				owner->DrawChar(c,BPoint(itemRect.left+65+indexChar*9,itemRect.top+10));
			}
		}
	}
	
	// remettre la couleur noire
	owner->SetHighColor(U_COULEUR_NOIR);
}
