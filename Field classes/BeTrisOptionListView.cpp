/****************************/
/* vue pour afficher le jeu */
/****************************/
#include "BeTrisApp.h"
#include "BeTrisNetwork.h"
#include "BeTrisConstants.h"
#include "BeTrisMessagesDefs.h"
#include "BeTrisDebug.h"
#include "BeTrisOptionListView.h"
#include "BeTrisPlayerItem.h"

#include <Messenger.h>
#include <Bitmap.h>
#include <stdio.h>
#include <string.h>

/**** constructeur ****/
BeTrisOptionListView::BeTrisOptionListView(BPoint place,char * name)
: BView(BRect(place.x,place.y,place.x+(SP_MAX_CAPACITY * 16)+2,place.y+18),name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect	fieldRect;

	// initialiser
	_offBitmap = NULL;
	_messenger = NULL;

	// vider la liste de bloc
	memset(_spBlock,0,SP_MAX_CAPACITY +1);

	// image d'affichage
	fieldRect = Bounds();
	fieldRect.right -= 2;
	fieldRect.bottom -= 2;
	_offBitmap = new BBitmap(fieldRect,B_RGB32,true);

	// effacer le contenu de l'image de toute facon
	memset(_offBitmap->Bits(),0,_offBitmap->BitsLength());

	// mettre a jour l'image
	Update();
}

/**** destructeur ****/
BeTrisOptionListView::~BeTrisOptionListView()
{
	// liberer l'image
	if(_offBitmap!=NULL)
		delete _offBitmap;

	// detruire le messenger
	delete _messenger;
}

/**** attached to window ****/
void BeTrisOptionListView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// creer le messenger
	if(_messenger==NULL)
		_messenger = new BMessenger(this);
}

/**** gestion des messages ****/
void BeTrisOptionListView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	//dessin
	case BETRIS_DRAW_SPECIAL_MSG:
		{
			// dessin du jeu
			if(Looper()!=NULL)
				Draw(Bounds());
		}
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** Draw ****/
void BeTrisOptionListView::Draw(BRect updaterect)
{
	rgb_color	OldHeighColor;
	BRect	frame = Bounds();
	
	OldHeighColor = HighColor();
	
	// cadre
	SetHighColor(U_GRIS_FONCE);
	StrokeLine(BPoint(frame.left,frame.top),BPoint(frame.right,frame.top));
	SetHighColor(U_GRIS_CLAIR);
	StrokeLine(BPoint(frame.left,frame.bottom),BPoint(frame.right,frame.bottom));
	StrokeLine(BPoint(frame.right,frame.top+1),BPoint(frame.right,frame.bottom-1));

	// dessin du fond avec les options
	if(Looper())
		DrawBitmap(_offBitmap,BPoint(1,1));

	// cadre rouge
	SetHighColor(U_ROUGE_CLAIR);
	StrokeRect(BRect(frame.left+1,frame.top+1,frame.left+15,frame.bottom-1));
	SetHighColor(U_ROUGE_FONCE);
	StrokeRect(BRect(frame.left,frame.top,frame.left+16,frame.bottom));
	SetHighColor(OldHeighColor);
}

/**** mettre a jour les blocs disponibles ****/
bool BeTrisOptionListView::Update(const char *spbloclist)
{
	bool	result = false;

	// verifier le parametre
	// si c'est NULL on quitte on a deja vide
	if(spbloclist==NULL)
	{
		// vider nos blocs en internes
		memset(_spBlock,0,SP_MAX_CAPACITY +1);
		result = true;
	}
	else
	{
		char	spsize = strlen(_spBlock);
		char	newspsize = strlen(spbloclist);

		// ajout des blocs (si on est pas plein)
		// on doit les ajouter en tete
		if(spsize<SP_MAX_CAPACITY)
		{
			// on depasse au final
			if(spsize+newspsize>SP_MAX_CAPACITY)
				newspsize = SP_MAX_CAPACITY - spsize;
			else
				result = true;
		
			// on ajoute un par un en tête de liste
			for(char i=0;i<newspsize;i++)
			{
				memmove(_spBlock+1,_spBlock,spsize+1+i);
				_spBlock[0] = spbloclist[i];
			}
		}
	}

	// si c'est ok rafraichir l'ecran
	_draw_option_field();

	// retourner le resultat
	return result;
}

/**** envoyer le bloc en tete a un joueur ****/
char BeTrisOptionListView::SendSpecialBlock(char to)
{
	char	size = 0;

	// on doit avoir des block speciaux
	size = strlen(_spBlock);	
	if(size<=0)
		return '\0';

	BeTrisPlayerItem	*player = NULL;
	BMessage			outMessage(BETRIS_NETWORK_MSG);
	BMessage			inMessage(BETRIS_NETWORK_MSG);
	BString				netBuffer;
	
	// joueur local
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return '\0';
	
	// envoyer la commande au serveur
	netBuffer = g_BeTrisApp->Command(CMD_SB);
	netBuffer << " ";
	netBuffer << (int32)to;
	netBuffer << " ";
	netBuffer << g_BeTrisApp->BlockDesf(_spBlock[0]);
	netBuffer << " ";
	netBuffer << (int32)player->Number();

	// enlever le bloc de la liste
	// on peut commenter cette ligne pour des test !
	// ca n'enleve pas le bloc utilise
	memmove(_spBlock,_spBlock+1,size);
	
	// envoyer le message au reseau
	outMessage.AddString(BETRIS_NETWORK_DATAS,netBuffer);
	outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
	if(g_network_messenger.SendMessage(&outMessage)!=B_OK)
		if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
			printf("ERROR (BeTrisOptionListView::SendSpecialBlock) : network send error\n");

	// ok envoyer en interne la commande
	// car elle est pas renvoye par le serveur
	netBuffer << (char)0xFF;
	inMessage.AddString(BETRIS_NETWORK_DATAS,netBuffer);
	inMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
	if(BMessenger(be_app).SendMessage(&inMessage)!=B_OK)
		if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
			printf("ERROR (BeTrisOptionListView::SendSpecialBlock) : application send error\n");

	// reafficher
	_draw_option_field();
	
	// renvoi du tableau
	return _spBlock[0];
}

/**** effacer le bloc de tete ****/
void BeTrisOptionListView::RemoveSpecialBlockInfo()
{
	char	size = 0;

	// on doit avoir des block speciaux
	size = strlen(_spBlock);	
	if(size<=0)
		return;

	// enlever le bloc de la liste
	memmove(_spBlock,_spBlock+1,size);

	// reafficher
	_draw_option_field();
}
		
// =================
// fonction internes
// =================

/**** metode de dessin du cadre ****/
void BeTrisOptionListView::_draw_option_field()
{
	char	spsize;
	short	xd,yd;
	char	block;
	BRect	blockRect;

	// effacer le contenu de l'image de toute facon
	memset(_offBitmap->Bits(),0,_offBitmap->BitsLength());

	// dessin interne des blocs
	blockRect.Set(0,0,15,15);
	yd = 0;
	spsize = strlen(_spBlock);
	for(char index=0;index<spsize;index++)
	{
		// destination de l'image
		xd = index * SP_BLOCK_SIZE;
		block = _spBlock[index];
					
		// dessiner le bloc correspondant
		blockRect.left = (block - 1) * SP_BLOCK_SIZE;
		blockRect.right = blockRect.left + SP_BLOCK_SIZE - 1;
		g_BeTrisApp->BlitBitmap(blockRect,g_BeTrisApp->Bitmaps(),xd,yd,_offBitmap);
	}

	// dessin
	if(Looper()!=NULL)
		_messenger->SendMessage(BETRIS_DRAW_SPECIAL_MSG);
}
