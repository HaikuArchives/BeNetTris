#ifndef BeTrisBlockFreqView_H
#define BeTrisBlockFreqView_H

#include <View.h>
#include "BeTrisApp.h"

// constante
#define		FREQ_BLOCK_SPECIAL_NUMBER		9
#define		FREQ_BLOCK_NUMBER				7

// message
#define		U_CHANGE_FREQ_SPECIAL			'Ucfs'
#define		U_CHANGE_FREQ_BLOCK				'Ucfb'

class BBitmap;
class BTextControl;
class BBox;

// definition de la classe
class BeTrisBlockFreqView : public BView
{	
public:
	BeTrisBlockFreqView(BRect , const char *);
	virtual ~BeTrisBlockFreqView();
	
	virtual void	AttachedToWindow();
	virtual void	Draw(BRect updaterect); 				// fonction de dessin
	virtual void	MessageReceived(BMessage *message);		// gestion des messages
			void	SavePreferences(bool save=true);		// sauver les preferences

protected:
	BBox			*_specialoccurancy;
	BBox			*_blockoccurancy;
	BTextControl	*_special_percent[FREQ_BLOCK_SPECIAL_NUMBER];
	BTextControl	*_blocks_percent[FREQ_BLOCK_NUMBER];
	BBitmap			*_offBlock;
	BBitmap			*_offSpBlock;
	
			void	_init_off_bitmap();			// creer les images off-screen
			void	_refresh_preferences();		// rafraichir les preferences
};

#endif