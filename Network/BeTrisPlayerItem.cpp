#include "BeTrisMessagesDefs.h"
#include "BeTrisConstants.h"
#include "BeTrisPlayerItem.h"

#include <View.h>
#include <Message.h>
#include <stdio.h>
#include <time.h>


BeTrisPlayerItem::BeTrisPlayerItem()
: BListItem()
{
	// initialiser
	_init();
}

BeTrisPlayerItem::BeTrisPlayerItem(BMessage *message)
: BListItem()
{
	if(message==NULL)
		return;
		
	// initialiser
	_init();

	message->FindString(PLAYER_NICKNAME,&_nickName);
	message->FindString(PLAYER_NAME,&_name);
	message->FindBool(PLAYER_REJECTED,&_isRejected);
	message->FindString(PLAYER_TEAM,&_team);
	
	if(message->FindInt32(PLAYER_COLOR,(int32 *)&_color)!=B_OK)
		_color = (rgb_color){0,255,0,255};

	// ce joueur n'est pas active
	_activated = false;	
}

/**** destructeur ****/
BeTrisPlayerItem::~BeTrisPlayerItem()
{
}

/**** dessin ****/
void BeTrisPlayerItem::DrawItem(BView *owner,BRect itemRect,bool drawEverything = false)
{
	BString		tmpstring;
	rgb_color	oldHighColor;
	rgb_color	oldLowColor;
	
	// sauver les couleurs
	oldHighColor = owner->HighColor();
	oldLowColor = owner->LowColor();

	if(IsSelected())
		owner->SetHighColor(U_VIOLET_CLAIR);
	else
		owner->SetHighColor(U_COULEUR_BLANC);	
	owner->SetLowColor(owner->HighColor());
	owner->FillRect(itemRect);

	// est-il active
	if(_activated)
	{
		owner->SetHighColor(U_VERT_FONCE);
	
		// montrer qu'il est active
		owner->DrawChar('#',BPoint(2,itemRect.bottom-2));
	}
	else
		owner->SetHighColor(U_BLEU_FONCE);

	owner->DrawString(_nickName.String(),BPoint(12,itemRect.bottom-2));
	owner->DrawString(_team.String(),BPoint(100,itemRect.bottom-2));	
	owner->DrawString(_name.String(),BPoint(200,itemRect.bottom-2));
	
	// restaurer les couleurs
	owner->SetHighColor(oldHighColor);
	owner->SetLowColor(oldLowColor);
}

/**** mettre dans un message les infos du player ****/
status_t BeTrisPlayerItem::Archive(BMessage *message)
{
	if(message==NULL)
		return B_ERROR;
		
	status_t	state = B_OK;
	
	state &= message->AddString(PLAYER_NICKNAME,_nickName);
	state &= message->AddString(PLAYER_NAME,_name);
	state &= message->AddBool(PLAYER_REJECTED,_isRejected);
	state &= message->AddString(PLAYER_TEAM,_team);
	state &= message->AddInt32(PLAYER_COLOR,*(int32 *)&_color);
		
	return state;
}

/**** initialiser les donnees ****/
void BeTrisPlayerItem::_init()
{
	_name = "new player";
	_nickName = "new nickName";
	_team = "";
	_color = (rgb_color){0,255,0,255};
	_activated = false;
	_endGameTime = 0;
	_num = -1;
	_level = 1;
	_isRejected = false;
}

// =====================================
// classe supplementaire pour le serveur
// =====================================

/**** constructeur ****/
BeTrisServerPlayerItem::BeTrisServerPlayerItem()
{
	_player = NULL;
	_slot = -1;
}
	
/**** destructeur ****/
BeTrisServerPlayerItem::~BeTrisServerPlayerItem()
{
	// on ne fait rien
}
