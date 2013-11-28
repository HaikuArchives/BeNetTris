#ifndef BeTrisNetworkView_H
#define BeTrisNetworkView_H

#include <View.h>
#include <Box.h>
#include <ListView.h>
#include <TextView.h>
#include <TextControl.h>
#include <Button.h>
#include <CheckBox.h>
#include <Slider.h>
#include "BeTrisApp.h"

/**** constantes des messages ****/
#define			U_SERVEUR_STSTOP_MSG			'Usss'
#define			U_CLIENT_CREATE_MSG				'Uclc'
#define			U_ADD_BAN_IP_MSG				'Uabm'
#define			U_ADD_SERVER_IP_MSG				'Uaip'
#define			U_REMOVE_BAN_IP_MSG				'Urbm'
#define			U_MASK_IP_MSG					'Umim'
#define			U_START_SERVEUR_MSG				'Ussm'
#define			U_SERVEUR_PORT_MSG				'Uspm'
#define			U_REFRESH_LIST_MSG				'Urlm'
#define			U_CONNECT_PLAYER_MSG			'Ucpm'
#define			U_SELECT_SERVER_MSG				'Usse'
#define			U_REMOVE_SERVER_MSG				'Ursr'
#define			U_REDRAW_LIST_MSG				'Urfm'

// donnee des messages
#define			SERVER_ITEM_INDEX		"server-item-index"

// declaration de la classe
class BeTrisNetworkView : public BView
{
public:
	BeTrisNetworkView(BRect , const char *);
	virtual ~BeTrisNetworkView();

	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage *message);	
			void	SavePreferences(bool save=true);						// sauver les preferences

protected:
	BBox			*_clientconfig;
	BListView		*_listserveur;
	BButton			*_refreshlist;
	BButton			*_connectdisconnect;
	BButton			*_removeserveur;
	BTextControl	*_enterip;
	BBox			*_serveurconfig;
	BBox			*_gameconfig;
	BTextControl	*_serveurname;
	BTextView		*_infobanip;
	BListView		*_banips;
	BTextControl	*_maskip;
	BButton			*_addip;
	BButton			*_removeip;
	BButton			*_startserveur;
	BSlider			*_stackheight;
	BSlider			*_optioncapacity;
	BSlider			*_minuteadd;
	BSlider			*_secondadd;
	BSlider			*_startlevel;
	BSlider			*_makeline;
	BSlider			*_numberline;
	BSlider			*_blockline;
	BSlider			*_numberblock;
	BCheckBox		*_classicmode;
	BCheckBox		*_averagelevel;
	BTextControl	*_serveurport;
	bool			_isInRefresh;		// est-on en train de rafraichir la liste des serveurs ?
	thread_id		_pingThreadID;		// id du thread de ping
	bool			_quitPingThread;	// quitter le thread de ping
	
			void	_add_server_ip();					// ajouter une Ip d'un serveur a la liste
			void	_select_server(BMessage *message);	// selectionner un serveur de jeux
			void	_connect_to_server();				// se connecter a un serveur
			void	_start_stop_server();				// demarrer en tant que serveur
			void	_remove_server();					// enlever un serveur de la liste
			void	_refresh_preferences();				// rafraichir les preferences
			void	_update_gui();						// changer l'etat de la GUI
			void	_refresh_servers_list();			// rafraichir la liste des serveurs

	// thread du ping des serveurs
	static	int32			_ping_thread(void *data);
			int32			_ping();
};

#endif