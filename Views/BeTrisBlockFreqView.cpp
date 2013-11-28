#include "BeTrisBlockFreqView.h"
#include "BeTrisConstants.h"
#include "CPreferenceFile.h"
#include "BeTrisApp.h"

#include <Box.h>
#include <TextControl.h>
#include <Button.h>
#include <Bitmap.h>
#include <stdio.h>

// tableau des frequences
char	_block_freq_default[FREQ_BLOCK_NUMBER] = {15,15,14,14,14,14,14};
char	_special_freq_default[FREQ_BLOCK_SPECIAL_NUMBER] = {19,16,3,14,3,14,6,11,14};

/**** constructeur ****/
BeTrisBlockFreqView::BeTrisBlockFreqView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	int	index = 0;

	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	// initialiser
	_offBlock = NULL;
	_offSpBlock = NULL;

	/**** groupe pour la frequence des block speciaux ****/
	_specialoccurancy = new BBox(BRect(10,10,305,465),"special-occurancy");	
	_specialoccurancy->SetLabel(" Special Block Occurancy ");
	AddChild(_specialoccurancy);

	// ajout des Controle pour saisir les frequences des block speciaux
	_special_percent[0] = new BTextControl(BRect(50,40,250,50),"special-freq-1","Add Lines","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[1] = new BTextControl(BRect(50,70,250,80),"special-freq-2","Clear Lines","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[2] = new BTextControl(BRect(50,100,250,110),"special-freq-3","Nuke Field","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[3] = new BTextControl(BRect(50,130,250,140),"special-freq-4","Random Block Clear","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[4] = new BTextControl(BRect(50,160,250,170),"special-freq-5","Switch Fields","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[5] = new BTextControl(BRect(50,190,250,200),"special-freq-6","Clear Special Blocks","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[6] = new BTextControl(BRect(50,220,250,230),"special-freq-7","Block Gravity","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[7] = new BTextControl(BRect(50,250,250,260),"special-freq-8","Blockquake","",new BMessage(U_CHANGE_FREQ_SPECIAL));
	_special_percent[8] = new BTextControl(BRect(50,280,250,290),"special-freq-9","Block Bomb","",new BMessage(U_CHANGE_FREQ_SPECIAL));

	// ajout des controles pour les block speciaux a la vue
	for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
	{
		_special_percent[index]->SetDivider(150);
		_specialoccurancy->AddChild(_special_percent[index]);
	}
	
	/**** groupe pour la frequence des block standard ****/
	_blockoccurancy = new BBox(BRect(315,10,610,465),"block-occurancy");	
	_blockoccurancy->SetLabel(" Block Occurancy ");
	AddChild(_blockoccurancy);

	// ajout des Controle pour saisir les frequences des block standard
	_blocks_percent[0] = new BTextControl(BRect(110,40,250,50),"block-freq-1","block 1","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[1] = new BTextControl(BRect(110,80,250,90),"block-freq-2","block 2","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[2] = new BTextControl(BRect(110,120,250,130),"block-freq-3","block 3","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[3] = new BTextControl(BRect(110,160,250,170),"block-freq-4","block 4","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[4] = new BTextControl(BRect(110,200,250,210),"block-freq-5","block 5","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[5] = new BTextControl(BRect(110,240,250,250),"block-freq-6","block 6","",new BMessage(U_CHANGE_FREQ_BLOCK));
	_blocks_percent[6] = new BTextControl(BRect(110,280,250,290),"block-freq-7","block 7","",new BMessage(U_CHANGE_FREQ_BLOCK));

	// ajout des controles pour les block speciaux a la vue
	for(index=0;index<FREQ_BLOCK_NUMBER;index++)
	{
		_blocks_percent[index]->SetDivider(70);
		_blockoccurancy->AddChild(_blocks_percent[index]);
	}

	// creer les images
	_offBlock = new BBitmap(BRect(0,0,31,264),B_RGB32,true);;
	_offSpBlock = new BBitmap(BRect(0,0,15,255),B_RGB32,true);;

	// creer les images off-screen
	_init_off_bitmap();
	
	// charger les preferences
	_refresh_preferences();
	
	// mettre a jour en interne
	SavePreferences(false);
}

/**** destructeur ****/
BeTrisBlockFreqView::~BeTrisBlockFreqView()
{
	// liberer la memoire des images oof-screen
	delete _offBlock;
	delete _offSpBlock;
}

/**** attachement de la fenetre ****/
void BeTrisBlockFreqView::AttachedToWindow()
{
	int	index = 0;

	// fonction parent
	BView::AttachedToWindow();
	
	// destinataire des messages
	for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
		_special_percent[index]->SetTarget(this);

	for(index=0;index<FREQ_BLOCK_NUMBER;index++)
		_blocks_percent[index]->SetTarget(this);
}

/**** fonction de dessin ****/
void BeTrisBlockFreqView::Draw(BRect updaterect)
{
	// on doit avoir le BBox valide
	if(_specialoccurancy!=NULL && _offSpBlock!=NULL)
		_specialoccurancy->DrawBitmap(_offSpBlock,BPoint(30,40));
	
	// on doit avoir le BBox valide
	if(_blockoccurancy!=NULL && _offBlock!=NULL)
		_blockoccurancy->DrawBitmap(_offBlock,BPoint(50,40));
}

/**** gestion des messages ****/
void BeTrisBlockFreqView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	// modification de frequence
	case U_CHANGE_FREQ_SPECIAL:
	case U_CHANGE_FREQ_BLOCK:
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** sauver les preferences ****/
void BeTrisBlockFreqView::SavePreferences(bool save)
{
	CPreferenceFile		*prefs = NULL;
	_game_data			*serverGameData = NULL;
	BString				freqName;
	int					frequence;
	char				index;

	// on utilise les preferences reseau
	prefs = g_BeTrisApp->PreferencesNetwork();
	serverGameData = g_BeTrisApp->GameDataServer();

	// enregisterer la frequence des blocks
	for(index=0;index<FREQ_BLOCK_NUMBER;index++)
	{
		sscanf(_blocks_percent[index]->Text(),"%d",&frequence);
		serverGameData->_blockFreq[index] = (char)frequence;
	}
	
	// enregisterer la frequence des blocks speciaux
	for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
	{
		sscanf(_special_percent[index]->Text(),"%d",&frequence);
		serverGameData->_specialFreq[index] = (char)frequence;
	}
	
	if(save)
	{
		// enregisterer la frequence des blocks
		for(index=0;index<FREQ_BLOCK_NUMBER;index++)
		{
			freqName = "block-freq-";
			freqName << (int32)index;
			prefs->SetInt8(serverGameData->_blockFreq[index],freqName.String());
		}

		// enregisterer la frequence des blocks speciaux
		for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
		{
			freqName = "special-freq-";
			freqName << (int32)index;
			prefs->SetInt8(serverGameData->_specialFreq[index],freqName.String());
		}
	
		// enregistrer
		prefs->Save();	
	}
}

// ==================
// fonctions internes
// ==================

/**** creer les images off-screen ****/
void BeTrisBlockFreqView::_init_off_bitmap()
{
	// on va construire les images a afficher
	// avec les bloc speciaux et normaux en petit
	_betrisblock	**blocks = NULL;
	char			currentBlock;
	char			block;
	char			x,y;

	// remplir de couleur transparente
	memset(_offBlock->Bits(),0x00DBDBDB,_offBlock->BitsLength());
	memset(_offSpBlock->Bits(),0x00DBDBDB,_offSpBlock->BitsLength());

	// dessin des block speciaux dans l'image
	for(char index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
		g_BeTrisApp->BlitBitmap(BRect((index+5)*16,0,(index+6)*16-1,15),g_BeTrisApp->Bitmaps(),0,index*30,_offSpBlock);
	
	// acces aux tableau des block
	blocks = g_BeTrisApp->Block();

	// dessin des block standard dans l'image
	for(currentBlock=0;currentBlock<FREQ_BLOCK_NUMBER;currentBlock++)
	{
		// est-on dans le rectangle d'update
		for(y=0;y<4;y++)
		{
			for(x=0;x<4;x++)
			{
				// valeur du bloc
				block = blocks[currentBlock][0][x][y];
				if(block>0)
				{
					block--;
					g_BeTrisApp->BlitBitmap(BRect(block*8,16,(block+1)*8-1,23),g_BeTrisApp->Bitmaps(),x*8,y*8+currentBlock*40,_offBlock);
				}
			}
		}
	}	
}

/**** rafraichir les preferences ****/
void BeTrisBlockFreqView::_refresh_preferences()
{
	CPreferenceFile		*prefs = NULL;
	BString				freqName;
	BString				buffer;
	char				index;

	// on utilise les preferences reseau
	prefs = g_BeTrisApp->PreferencesNetwork();

	// on recupere les frequences des blocks
	for(index=0;index<FREQ_BLOCK_NUMBER;index++)
	{
		freqName = "block-freq-";
		freqName << (int32)index;
		buffer = "";
		buffer << (int32)(prefs->GetInt8(_block_freq_default[index],freqName.String()));
		_blocks_percent[index]->SetText(buffer.String());
	}

	// on recupere les frequences des blocks speciaux
	for(index=0;index<FREQ_BLOCK_SPECIAL_NUMBER;index++)
	{
		freqName = "special-freq-";
		freqName << (int32)index;
		buffer = "";
		buffer << (int32)(prefs->GetInt8(_special_freq_default[index],freqName.String()));
		_special_percent[index]->SetText(buffer.String());
	}
}