/*******************************************/
/* vue pour afficher le text sur le joueur */
/*******************************************/
#include "BeTrisConstants.h"
#include "BeTrisApp.h"
#include "BeTrisPlayerView.h"
#include <stdio.h>

BeTrisPlayerView::BeTrisPlayerView(BPoint place,char * name,BeTrisPlayerView::field_type type)
: BView(SizeOfField(place,type),name,B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// initialiser
	_playerExist = false;

	// mettre a jour
	Update(NULL);
}

BeTrisPlayerView::~BeTrisPlayerView()
{
}

/**** Draw ****/
void BeTrisPlayerView::Draw(BRect updaterect)
{
	rgb_color		OldHighColor;
	BRect			frame = Bounds();
	
	// grader l'ancienne couleur
	OldHighColor = HighColor();

	// le fond
	if(!_playerExist)
		SetHighColor(U_GRIS_CLAIR);
	else
		SetHighColor(U_GRIS_STANDARD);
	SetLowColor(HighColor());
	FillRect(frame);

	// le label
	if(_playerExist)
		SetHighColor(U_COULEUR_NOIR);
	else
		SetHighColor(U_GRIS_FONCE);
	DrawString(_label.String(),_labelPosition);
	
	// le cadre
	SetHighColor(U_COULEUR_BLANC);
	StrokeLine(BPoint(frame.left,frame.top),BPoint(frame.right,frame.top));
	StrokeLine(BPoint(frame.left,frame.top),BPoint(frame.left,frame.bottom));
	SetHighColor(U_GRIS_FONCE);
	StrokeLine(BPoint(frame.right,frame.bottom),BPoint(frame.left,frame.bottom));
	StrokeLine(BPoint(frame.right,frame.top),BPoint(frame.right,frame.bottom));
	
	// retablir la couleur
	SetHighColor(OldHighColor);
}

/**** metodes pour dimentionner correctement le jeu ****/
BRect BeTrisPlayerView::SizeOfField(BPoint point,BeTrisPlayerView::field_type typeshow)
{
	BRect	frame;
	
	frame.Set(point.x,point.y,point.x+12*typeshow+1,point.y+15);
	
	return frame;
}

/**** mise a jour des infos *****/
void BeTrisPlayerView::Update(BeTrisPlayerItem *player)
{
	BFont	viewFont;
	BRect	bounds;

	// a-t-on un joueur
	_playerExist = (player!=NULL);

	// definir le texte a afficher
	if(!_playerExist)
		_label = "Not Playing";
	else
	{
		// nom du joueur
		_label = "";
		_label << (int32)(player->Number());
		_label << ". ";
		_label << player->NickName();

		// equipe
		if(strlen(player->Team())>0)
		{
			_label << "-";
			_label << player->Team();
		}
	}

	// taille de la vue
	bounds = Bounds();

	// police de la vue
	GetFont(&viewFont);
	
	// calcul de la position du texte
	_labelPosition.x = (bounds.Width() - viewFont.StringWidth(_label.String())) / 2;
	if(_labelPosition.x<3)
		_labelPosition.x = 3;
	_labelPosition.y = bounds.bottom - 4;
}

