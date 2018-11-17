/***********************************************************/
/* vue pour afficher la previsualisation du prochain block */
/***********************************************************/
#include "BeTrisConstants.h"
#include "BeTrisApp.h"
#include "BeTrisNextBlockView.h"
#include "BeTrisMessagesDefs.h"
#include <Bitmap.h>

/**** constructeur ****/
BeTrisNextBlockView::BeTrisNextBlockView(BPoint place,char * name)
: BView(BRect(place.x,place.y,(place.x+BLOCK_VIEW_SIZE),(place.y+BLOCK_VIEW_SIZE)),name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	// initialiser
	_offBitmap = NULL;
	_nextBlock = -1;
	_nextOrient = -1;
	_messenger = NULL;

	// buffer de dessin
	// image d'affichage
	_fieldRect = Bounds();
	_fieldRect.right -= 4;
	_fieldRect.bottom -= 4;
	_offBitmap = new BBitmap(_fieldRect,B_RGB32,true);
	
	// vider
	Empty();
}

/**** destucteur ****/
BeTrisNextBlockView::~BeTrisNextBlockView()
{
	// liberer le buffer de dessin
	if(_offBitmap!=NULL)
		delete _offBitmap;

	// detruire le messenger
	delete _messenger;
}

/**** attached to window ****/
void BeTrisNextBlockView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// creer le messenger
	if(_messenger==NULL)
		_messenger = new BMessenger(this);
}

/**** gestion des messages ****/
void BeTrisNextBlockView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	//dessin
	case BETRIS_DRAW_NEXTBLOCK_MSG:
		{
			// dessin du prochain bloc
			if(Looper()!=NULL)
				DrawBitmap(_offBitmap,BPoint(2,2));
		}
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** Draw ****/
void BeTrisNextBlockView::Draw(BRect updaterect)
{
	rgb_color	OldHeighColor;
	rgb_color	OldLowColor;
	BRect		frame = Bounds();

	OldHeighColor = HighColor();
	OldLowColor = LowColor();
	
	SetHighColor(U_GRIS_FONCE);
	StrokeLine(BPoint(frame.left,frame.top),BPoint(frame.right,frame.top));
	StrokeLine(BPoint(frame.left,frame.top),BPoint(frame.left,frame.bottom));
	SetHighColor(U_GRIS_CLAIR);
	StrokeLine(BPoint(frame.right,frame.bottom),BPoint(frame.left,frame.bottom));
	StrokeLine(BPoint(frame.right,frame.top),BPoint(frame.right,frame.bottom));

	// entourer l'image
	frame.InsetBy(1,1);
	SetHighColor(U_COULEUR_NOIR);
	StrokeRect(frame);
	
	// retablir la couleur du fond
	SetHighColor(OldHeighColor);
	SetLowColor(OldLowColor);

	// dessin du prochain bloc
	if(Looper()!=NULL)
		DrawBitmap(_offBitmap,BPoint(2,2));
}

/**** obtenir le prochain bloc ****/
void BeTrisNextBlockView::NextBlock(char &newBlock,char &newOrient)
{
	// il n'y a pas de precedent
	// on va en regenerer un
	if(_nextBlock<0 || _nextOrient<0)
		 _generate_next_block();
	
	// assigner le precedent
	newBlock = _nextBlock;
	newOrient = _nextOrient;
	
	// generer le prochain
	_generate_next_block();

	// ok redessiner
	_draw_next_block();	
}

/**** vider le champ de jeu ****/
void BeTrisNextBlockView::Empty()
{
	// anuller tout
	_nextBlock = -1;
	_nextOrient = -1;

	// redessiner
	_draw_next_block();
}

// ==================
// fonctions internes
// ==================

/**** metode de dessin du cadre ****/
void BeTrisNextBlockView::_draw_next_block()
{
	_betrisblock	**blocks = NULL;
	BBitmap			*bitmap = NULL;
	BRect			blockRect;
	char			block;
	short			x,y;

	// effacer l'image (fond noir)
	memset(_offBitmap->Bits(),0,_offBitmap->BitsLength());

	// dessin du prochain bloc si il existe
	if(_nextBlock>=0 && _nextOrient>=0)
	{
		// taille de depart
		blockRect.Set(0,0,15,15);

		// bloc courant et image de decord
		blocks = g_BeTrisApp->Block();
		bitmap = g_BeTrisApp->Bitmaps();

		for(y=0;y<BLOCK_HEIGHT;y++)
		{
			for(x=0;x<BLOCK_WIDTH;x++)
			{
				// il doit etre superieur a 0
				block = blocks[_nextBlock][_nextOrient][x][y];
				if(block>0)
				{
					// dessiner le bloc correspondant
					blockRect.left = (block - 1) * BLOCK_SIZE;
					blockRect.right = blockRect.left + BLOCK_SIZE - 1;

					// dessin de l'image
					g_BeTrisApp->BlitBitmap(blockRect,bitmap,x*BLOCK_SIZE,y*BLOCK_SIZE,_offBitmap);
				}
			}
		}
	}

	// dessin
	if(Looper()!=NULL)
		_messenger->SendMessage(BETRIS_DRAW_NEXTBLOCK_MSG);	
}

/**** generer le prochain bloc ****/
void BeTrisNextBlockView::_generate_next_block()
{
	// la fonction met a jour les variable
	// _nextBlock et _nextOrient
	_game_data	*gameData = NULL;
	char		index = 0;
	char		rnd = 0;
	
	// on doit avoir des donnees de jeu
	gameData = g_BeTrisApp->GameData();
	if(gameData==NULL)
		return;
		
	// choisir le bloc
	rnd = g_BeTrisApp->RandomNum(100);
	while(rnd>=gameData->_blockFreq[index])
		index++;

	// prochain bloc
	_nextBlock = index;

	// choisir aussi une orientation
	_nextOrient = g_BeTrisApp->RandomNum(g_BeTrisApp->BlockOrientation(index));
}