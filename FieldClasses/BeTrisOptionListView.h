/****************************/
/* vue pour afficher le jeu */
/****************************/
#ifndef BeTrisOptionListView_H
#define BeTrisOptionListView_H

#include <View.h>

// cpapacite maximum
#define		SP_MAX_CAPACITY		18
#define		SP_BLOCK_SIZE		16

class BBitmap;

class BeTrisOptionListView : public BView
{
public:
	BeTrisOptionListView(BPoint ,char *);
	virtual	~BeTrisOptionListView();

	virtual void		AttachedToWindow();
	virtual	void		MessageReceived(BMessage *message);		// gestion des messages
	virtual void		Draw(BRect updaterect); 				// dessin
			bool		Update(const char *spbloclist=NULL);	// mettre a jour les blocs disponibles
			char		SendSpecialBlock(char to);				// envoyer le bloc en tete a un joueur
			void		RemoveSpecialBlockInfo();				// effacer le bloc de tete
private:
	BBitmap			*_offBitmap;					// buffer de dessin
	BMessenger		*_messenger;					// pour envoyer le message de dessin
	char			_spBlock[SP_MAX_CAPACITY +1];	// liste des blocs

	void	_draw_option_field();
};

#endif