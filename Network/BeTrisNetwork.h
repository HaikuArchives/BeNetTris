#ifndef _BTCPLOOPER_H_
#define _BTCPLOOPER_H_

/*********************/
/* Classe BeTrisNetwork */
/*********************/
// classe de communication client/server
// base sur un BLooper

// un numero magique : (octet 0 a 3 - 4 octets)
// la taille finale : (octet 4 a 7 - 4 octets)
// ensuite le message sous forme "flatten"

#include <Looper.h>
#include <String.h>
#include <List.h>

// messages
#define	NETWORK_START_SERVEUR_MSG			'Nstr'
#define	NETWORK_STOP_SERVEUR_MSG			'Nstp'
#define	NETWORK_CREATE_CLIENT_MSG			'Ncli'

#define	CONNECTION_CLOSE_MSG				'Cclm'
#define	CONNECTION_ERROR_MSG				'Cerr'
#define	CONNECTION_CLOSE_ALL_MSG			'Ccal'

// données
#define	NETWORK_PORT						"network-port"
#define	NETWORK_ADDRESS						"network-address"
#define	NETWORK_INFORMATION					"network-information"
#define	NETWORK_STATUS						"network-status"
#define	NETWORK_CLIENT_ADDRESS				"network-client-address"
#define	NETWORK_CLIENT_PORT					"network-client-port"
#define	NETWORK_SERVER_NAME					"network-server-name"
#define	NETWORK_LOCAL_PLAYER				"network-local-player"

// divers
#define	NETWORK_BUFFER_LEN				4096
#define	NETWORK_MINBUFFER_LEN			4096
#define	NETWORK_NBCLIENT_DEFAULT		1

// mode de fonctionnement
#define	NO_CONNECTION_MODE				0x00	// 0000 0000
#define	SERVER_MODE						0x01	// 0000 0001
#define	CLIENT_MODE						0x02	// 0000 0010
#define	CLIENT_SERVER_MODE				0x03	// 0000 0011

class BMessenger;
class BDataIO;

// structure de connection client
struct _network_slot
{
	// constructeur
	_network_slot()
	{ _socket = -1; _threadId = -1; _used = false; }

	int				_socket;
	thread_id		_threadId;
	bool			_used;
};

class BeTrisNetwork : public BLooper
{
public:
	// Fonction BLooper		
	BeTrisNetwork(BHandler *destination,int32 nbCientMax = NETWORK_NBCLIENT_DEFAULT,int32 netbufferSize = NETWORK_BUFFER_LEN);
	virtual	~BeTrisNetwork();
	virtual	void			MessageReceived(BMessage *msg);
	virtual	bool			QuitRequested(void);

				bool					Connexion();
				bool					IsStarted();
				int8					Mode();
				
	inline	_network_slot		*NetworkClientSlot(char	index)	{ return &_serverClientSlot[index]; }	// slot client
	inline	_network_slot		*NetworkServerSlot()					{ return &_serverRecvSlot; }				// slot serveur
		
protected:
	BMessenger		*_destinationMessenger;								// messenger destination des messages recuent
	int8			_mode;												// mode de connection
	bool			_quit;												// on quitte !
	bool			_connexion;											// la connexion exist-elle
	bool			_serverStarted;										// le serveur est-il demarre
	bool			_startlocalplayer;									// boolean pour le serveur pour demarrer ou pas le joueur local
	int32			_port;												// port de connection ou d'ecoute
	BString			_adresse;											// adresse de connection
	int32			_netbufferSize;										// taille du buffer des paquets
	char			*_sendBuffer;										// buffer utiliser pour l'envoi de donnees
	_network_slot	_serverClientSlot[BETRIS_MAX_CLIENTS + 1];			// slot de connexion client du serveur (le 0 est toujours pour le joueur local)
	_network_slot	_serverRecvSlot;									// slot de reception du serveur
	
		
				void			_close_all_connection();
				void			_close_connection(char slot);
				status_t		_send_to_network(BMessage *datas);

				status_t		_connect_to_server(BMessage *data);
				void			_start_reception(char slot);
				status_t		_start_listen(BMessage *message);
				char			_find_free_slot();						// trouver un slot libre

	// partie d'ecoute serveur
	static	int32			_listen_server_thread(void *datas);
			int32			_listen_server();

	// thread de reception des donnees
	static	int32			_reception_thread(char *datas);
			int32			_reception(char *slot);
};

// messenger pour acceder au looper de n'importe ou
extern	BMessenger			g_network_messenger;
extern	BeTrisNetwork		*g_network;

#endif