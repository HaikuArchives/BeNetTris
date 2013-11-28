#ifndef BeTrisWinnerView_H
#define BeTrisWinnerView_H

#include <View.h>
#include <Box.h>
#include <ListView.h>
#include <Button.h>
#include "BeTrisApp.h"

/**** constantes pour les messages ****/
#define			U_EMPTY_SCORE_MSG		'Uesm'
#define			U_SELECT_ITEM_MSG		'Usim'
#define			WINNER_BUFFER_SIZE		128

// classes
class CPreferenceFile;

// vue des gagnant
class BeTrisWinnerView : public BView
{	
public:
	BeTrisWinnerView(BRect , const char *);
	virtual ~BeTrisWinnerView();
	
	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage *message);

			void	AddWinner(BString &string);		// ajouter un gagnant
			void	GetWinner(BString &string);		// recuperer les gagnants
			void	EmptyList();					// vider la liste
			void	SavePreferences();				// sauver les preferences

protected:
	CPreferenceFile		*_winners;
	BBox				*_winnergroupe;
	BListView			*_winnerlist;
	BButton				*_emptyscore;
	char				_winnerBuffer[WINNER_BUFFER_SIZE];

			void	_load_preferences();		// charger les preferences
};

// classe de list pour les gagnant
class BeTrisWinnerItem : public BListItem
{
public:
	BeTrisWinnerItem(bool isteam,char *name,int32 score);
	BeTrisWinnerItem(BMessage *message);
	virtual	~BeTrisWinnerItem();
	
	virtual void		DrawItem(BView *owner,BRect itemRect,bool drawEverything = false);	// dessin
			status_t	Archive(BMessage *message);			// mettre dans un message les infos du player
			void		SetScore(int32 score);				// definir le score

	inline	const char	*Name()						{ return _name.String(); }
	inline	void		SetName(char *name)			{ _name = name; }
	inline	bool		IsTeam()					{ return _isTeam; }
	inline	void		SetIsTeam(bool isteam)		{ _isTeam = isteam; }
	inline	int32		Score()						{ return _score; }

private:
	BString		_bufferString;
	BString		_name;
	bool		_isTeam;
	int32		_score;
	
			void	_init();											// initialiser
			void	_displayColorPlayer(BView *owner,BRect &itemRect);	// afficher le joueur avec les couleurs
};

#endif