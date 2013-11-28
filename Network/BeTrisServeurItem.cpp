/************************************************/
/* classe de dessin d'un serveur dans une liste */
/************************************************/
#include "BeTrisMessagesDefs.h"
#include "BeTrisConstants.h"
#include "BeTrisServeurItem.h"

#include <View.h>
#include <String.h>
#include <stdio.h>
#include <Message.h>

/**** constructeur ****/
BeTrisServeurItem::BeTrisServeurItem()
{
	// initialiser
	_init();
}


BeTrisServeurItem::BeTrisServeurItem(BMessage *message)
{
	// initialiser
	_init();

	// recuperer le nom d'hote
	message->FindString(SERVER_IP,&_hostName);
}

/**** destructeur ****/
BeTrisServeurItem::~BeTrisServeurItem()
{
}
	
/**** dessin ****/
void BeTrisServeurItem::DrawItem(BView *owner,BRect itemRect,bool drawEverything = false)
{
	rgb_color		oldHighColor;
	rgb_color		oldLowColor;
		
	// sauver les couleurs
	oldHighColor = owner->HighColor();
	oldLowColor = owner->LowColor();

	// si l'item est en cours de ping
	if(_isInRefresh)
		owner->SetHighColor(U_GRIS_TRES_CLAIR);
	else
	{
		// si l'item est selectionne
		if(_isServer)
			owner->SetHighColor(U_VIOLET_STANDARD);		
		else
		{
			if(IsSelected())
				owner->SetHighColor(U_VIOLET_CLAIR);
			else
				owner->SetHighColor(U_COULEUR_BLANC);	
		}
	}

	// dessin du fond		
	owner->SetLowColor(owner->HighColor());
	owner->FillRect(itemRect);
	
	// etat du serveur
	BRect	cercle(BRect(itemRect.left+2,itemRect.top+2,itemRect.left+(itemRect.bottom-itemRect.top-2),itemRect.bottom-2));
	owner->SetHighColor(_stateColor);
	owner->FillEllipse(cercle);
	owner->SetHighColor(U_COULEUR_NOIR);
	owner->StrokeEllipse(cercle);
		
	// ping
	owner->SetHighColor(U_BLEU_CLAIR);
	owner->DrawString(_tmpString.String(),BPoint(35,itemRect.bottom-2));	

	// Ip ou nom d'hote
	owner->SetHighColor(U_BLEU_CLAIR);
	owner->DrawString(_hostName.String(),BPoint(120,itemRect.bottom-2));

	// restaurer les couleurs
	owner->SetHighColor(oldHighColor);
	owner->SetLowColor(oldLowColor);
	
}

/**** changer l'etat du serveur ****/
void BeTrisServeurItem::SetState(server_state state)
{
	// retenir l'etat
	_state = state;
	
	// modifier la couleur
	switch(_state)
	{
	case SERVER_RESPOND:
		_stateColor = U_VERT_CLAIR;
		break;
	case SERVER_BADPING:
		_stateColor = U_ORANGE_CLAIR;
		break;
	default:
		_stateColor = U_ROUGE_CLAIR;
		break;
	}
}

/**** mettre dans un message les infos du seveur ****/
status_t BeTrisServeurItem::Archive(BMessage *message)
{
	if(message==NULL)
		return B_ERROR;
		
	status_t	state = B_OK;
	
	state &= message->AddString(SERVER_IP,_hostName);
		
	return state;
}

/**** definir le temps du ping ****/
void BeTrisServeurItem::SetPingTime(float value)
{
	// definir la valeur
	_pingTime = value;
	
	// definir l'etat en fonctino de la valeur
	if(_pingTime==0)
		SetState(SERVER_NOT_RESPOND);
	else
	{
		// ok le serveur repond, mais a quel vitesse
		if(_pingTime>200.00)
			SetState(SERVER_BADPING);
		else
			SetState(SERVER_RESPOND);
	}
	
	// chaine d'affichage du ping
	_tmpString = "";
	_tmpString << _pingTime;
	_tmpString << " ms";
}

// =================
// Fonction internes
// =================

/**** initalisation ****/
void BeTrisServeurItem::_init()
{
	// variable
	_isServer = false;
	_isInRefresh = false;
	_tmpString = "";
	_hostName = "";

	// etat
	SetPingTime(0);
}
