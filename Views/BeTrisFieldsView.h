#ifndef BeTrisFieldsView_H
#define BeTrisFieldsView_H

#include "BeTrisConstants.h"

#include <View.h>
#include <TextView.h>
#include <Messenger.h>

class BFont;
class BTextControl;
// structure des block speciaux
struct _special_block
{
	char	*_id;
	char	_block;
	char	*_info;
};

// constantes des additions de lignes en mode classic
#define		SP_ADD1LINE				"cs1"
#define		SP_ADD2LINE				"cs2"
#define		SP_ADD4LINE				"cs4"
#define		MAX_GMSG_LEN_MESSAGE	255

// message
const uint32	U_ENTER_GSGM_MSG 		= 'Uegm';

// declaration des classes
class BeTrisFieldView;
class BeTrisNextBlockView;
class BeTrisOptionListView;
class BeTrisPlayerView;

class BeTrisFieldsView : public BView
{
public:
	BeTrisFieldsView(BRect , const char *);
	virtual ~BeTrisFieldsView();

	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage *message);	
			void	SpecialBlockInfo(const char *from,const char *to,const char *specialBlock);		// afficher l'utilisation d'un block special
			void	DoSpecialBlock(char from,char to,const char *specialBlock);						// on recoit un block special a traiter
			void	RefreshDisplay();																// rafraichir l'affichage des donnees
			void	EmptyDisplay();																	// vider l'affichage
			void	UpdateAverageLevel();															// mise a jour du niveau moyen
			void	UpdateSpecialBlockInfo(char specialBlock);										// mise a jour de l'info des block speciaux
			void	AddGameMessage(const char *sender,const char *text);							// ajouter un message dans la partie
			
	inline	BeTrisFieldView			*Field(char index)			{ return _playerfield[index]; }		// acces au champs de jeu
	inline	BeTrisPlayerView		*PlayerField(char index)	{ return _playertext[index]; }		// acces au champs d'info joueur
	inline	BeTrisNextBlockView		*NextBlock()				{ return _nextblock; }				// acces a la vue du prochain bloc
	inline	BeTrisOptionListView	*SpecialBlocks()			{ return _listeoptions; }			// acces a la vue des blocs speciaux

protected:
	BeTrisFieldView			*_playerfield[BETRIS_MAX_CLIENTS];
	BeTrisPlayerView		*_playertext[BETRIS_MAX_CLIENTS];
	BeTrisNextBlockView		*_nextblock;
	BTextView				*_messagelist;
	BTextView				*_infonextblock;
	BTextView				*_infolevel;
	BTextView				*_averagelevel;
	BTextView				*_ppmview;
	BTextView				*_ppmmview;
	BTextView				*_ppmmaxview;
	BTextView				*_infolines;
	BTextView				*_textoptions;
	BTextView				*_specialblockinfo;
	BTextControl			*_messageedit;
	BeTrisOptionListView	*_listeoptions;
	BFont					*_plain_font;
	BFont					*_bold_font;
	char					_ppmmax;							// ppm max
	char					_ppmm;								// ppm moyen

			void	_send_game_message();		// envoyer un message en partie
};

#endif
