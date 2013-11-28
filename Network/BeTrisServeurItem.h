/************************************************/
/* classe de dessin d'un serveur dans une liste */
/************************************************/
#ifndef CSERVERLISTITEM_H
#define CSERVERLISTITEM_H

#include <ListItem.h>
#include <String.h>

enum server_state
{
	SERVER_RESPOND		= 1,
	SERVER_BADPING		= 2,
	SERVER_NOT_RESPOND	= 3,
};

class BeTrisServeurItem : public BListItem
{
public:
	BeTrisServeurItem();
	BeTrisServeurItem(BMessage *message);
	virtual ~BeTrisServeurItem();

	virtual void		DrawItem(BView *owner,BRect itemRect,bool drawEverything = false);	// dessin
			status_t	Archive(BMessage *message);			// mettre dans un message les infos du player
			void		SetState(server_state state);		// etat du serveur
			void		SetPingTime(float value);			// definir le temps du ping

	inline	BString		HostName()	 					{ return _hostName; }
	inline	void		SetHostName(const char *value)	{ _hostName = value; }
	inline	bool		IsServer() 						{ return _isServer; }
	inline	void		SetServer(bool value)			{ _isServer = value; }
	inline	float		PingTime()						{ return _pingTime; }
	inline	void		SetInRefresh(bool value)		{ _isInRefresh = value; }

protected:
	BString			_tmpString;
	BString			_hostName;
	rgb_color		_stateColor;
	server_state	_state;
	bool			_isServer;
	float			_pingTime;
	bool			_isInRefresh;
	
			void		_init();		// initialiser
};

#endif