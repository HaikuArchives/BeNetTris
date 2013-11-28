/***********************************************/
/* classe de dessin d'un joueur dans une liste */
/***********************************************/
#ifndef BeTrisPlayerItem_H
#define BeTrisPlayerItem_H

#include <ListItem.h>
#include <String.h>
#include <time.h>

// pour le definition des touches
#define MOVE_LEFT			0
#define MOVE_RIGHT			1
#define ROTATE_RIGHT		2
#define GO_DOWN				3
#define FALL_DOWN			4
#define ROTATE_LEFT			5

class BMessage;

// definition de la classe
class BeTrisPlayerItem : public BListItem
{
public:
	BeTrisPlayerItem();
	BeTrisPlayerItem(BMessage *message);
	virtual ~BeTrisPlayerItem();

	virtual void	DrawItem(BView *owner,BRect itemRect,bool drawEverything = false);	
	status_t		Archive(BMessage *message);			// mettre dans un message les infos du player
	
	inline	const char	*NickName()							{ return _nickName.String(); }
	inline	void		SetNickName(const char *nickname)	{ _nickName = nickname; }
	inline	const char	*Name()								{ return _name.String(); }
	inline	void		SetName(const char *name)			{ _name = name; }
	inline	const char	*Team()								{ return _team.String(); }
	inline	void		SetTeam(const char *team)			{ _team = team; }
	inline	char		Number()							{ return _num; }
	inline	void		SetNumber(char value)				{ _num = value; }
	inline	bool		IsActivated()						{ return _activated; }
	inline	void		SetActivated(bool value)			{ _activated = value; }
	inline	short		Level()								{ return _level; }
	inline	void		SetLevel(short level)				{ _level = level; }
	inline	time_t		EndGameTime()						{ return _endGameTime; }
	inline	void		SetEndGameTime(time_t value)		{ _endGameTime = value; }
	inline	bool		IsRejected()						{ return _isRejected; }
	inline	void		SetRejected(bool value)				{ _isRejected = value; }
		
protected:
	BString		_name;			// nom reel
	BString		_nickName;		// pseudo
	BString		_team;			// equipe
	rgb_color	_color;			// couleur du joueur
	bool		_activated;		// le joueur est-il celui avec lequel on joue ?
	char		_num;			// le numero affecte par le serveur
	short		_level;			// niveau actuel
	time_t		_endGameTime;	// heure a laquelle on a gagné ou perdu (fin de partie pour le joueur)
	bool		_isRejected;	// est-ce un joueur rejete ?

			void		_init();		// initialiser les donnees
};

// classe supplementaire pour le serveur
class BeTrisServerPlayerItem
{
public:
	BeTrisPlayerItem	*_player;	// objet joueur
	char				_slot;		// slot reseau

	BeTrisServerPlayerItem();
	~BeTrisServerPlayerItem();
};

#endif