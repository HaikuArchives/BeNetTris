/***********************************************************/
/* vue pour afficher la previsualisation du prochain block */
/***********************************************************/
#ifndef BeTrisNextBlockView_H
#define BeTrisNextBlockView_H

#include <View.h>

// taille des bloc pour le dessin
#define		BLOCK_VIEW_SIZE		67
#define		BLOCK_SIZE			16
#define		BLOCK_WIDTH			4
#define		BLOCK_HEIGHT		4


class BBitmap;

class BeTrisNextBlockView : public BView
{
public:
	// metodes de la classe BView
	BeTrisNextBlockView(BPoint ,char *);
	virtual	~BeTrisNextBlockView();

	virtual void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage *message);						// gestion des messages
	virtual void	Draw(BRect updaterect); 					// fonction de dessin
			void	NextBlock(char &newBlock,char &newOrient);	// obtenir le prochain bloc
			void	Empty();									// vider le champ de jeu
			
private:
	char		_fieldBlock[16];	// bloc a dessiner
	BRect		_fieldRect;			// rectangle de l'image off-screen
	BBitmap		*_offBitmap;		// buffer de dessin
	char		_nextBlock;			// prochain bloc
	char		_nextOrient;		// prochain orientation
	BMessenger	*_messenger;		// pour envoyer le message de dessin

			void	_draw_next_block();			// dessin du prochain bloc
			void	_generate_next_block();		// generer le prochain bloc
};

#endif