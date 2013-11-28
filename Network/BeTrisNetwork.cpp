#include "BeTrisApp.h"
#include "BeTrisNetwork.h"
#include "BeTrisConstants.h"
#include "BeTrisMessagesDefs.h"
#include "BeTrisDebug.h"
#include "BeTrisPlayerItem.h"

#include <Messenger.h>
#include <Application.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <inet.h>
#include <in.h>
#include <netdb.h>

// posix
#include <unistd.h>
#include <stdlib.h>

// messenger globale !
BMessenger			g_network_messenger;
BeTrisNetwork		*g_network = NULL;

/**** constructeur ****/
BeTrisNetwork::BeTrisNetwork(BHandler *destination,int32 nbCientMax,int32 netbufferSize)
: BLooper("BeTrisNetwork")
{
	// le messenger pour envoyer au Looper
	// et l'acces a la classe
	g_network_messenger = BMessenger(this);
	g_network = this;

	// messenger destination du looper
	_destinationMessenger = new BMessenger(destination);
	
	// initialiser les variables
	_quit = false;
	_connexion = false;
	_serverStarted = false;
	_port = 0;

	// initialiser le socket serveur
	_serverRecvSlot._used = false;
	_serverRecvSlot._socket = -1;

	// verifier la taille du buffer
	_netbufferSize	= netbufferSize;	
	if(netbufferSize<NETWORK_MINBUFFER_LEN)
		_netbufferSize = NETWORK_MINBUFFER_LEN;

	// buffer d'envoi
	_sendBuffer = (char *)(malloc(_netbufferSize*sizeof(char)));
}

/**** destructeur ****/
BeTrisNetwork::~BeTrisNetwork()
{
	// liberer la memoire
	delete _destinationMessenger;
	free(_sendBuffer);
}

/**** quitter le looper ****/
bool BeTrisNetwork::QuitRequested(void)
{
	// quitter notre joueur local
	if((_mode & CLIENT_MODE) && _connexion)
		_close_connection(0);

	// couper toutes les connections
	// si il en reste pour le serveur
	if((_mode & SERVER_MODE) && _serverStarted)
		_close_all_connection();
	
	// quitter
	return true;	
}

/**** gestion des messages ****/
void BeTrisNetwork::MessageReceived(BMessage *message)
{
	switch(message->what )
	{
	// demarrage du serveur
	case NETWORK_START_SERVEUR_MSG:
		if(_start_listen(message)!=B_OK && (BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK))
			printf("ERROR (MessageReceived) : _start_listen()\n");
		break;
	// arret du serveur
	case NETWORK_STOP_SERVEUR_MSG:
		{
			// fermer les connexions
			_close_all_connection();

			// informer l'application
			BMessenger(be_app).SendMessage(NETWORK_STOP_SERVEUR_MSG);
		}
		break;
	// se connecte a un serveur
	case NETWORK_CREATE_CLIENT_MSG:
		{
			BMessage 	error(BETRIS_NETWORK_MSG);
			char			errorStr[14];

			// vider le buffer
			memset(errorStr,0,14);

			// informer l'application si ca n'a pas marche
			if(_connect_to_server(message)!=B_OK)
			{
				// la connection n'est pas ouverte
				_connexion = false;

				// construire et envoyer l'erreur de connexion
				strcpy(errorStr,"noconnecting ");
				errorStr[12] = 0xFF;				
			}
			else
			{
				// construire et envoyer la connexion ok
				strcpy(errorStr,"connect ");
				errorStr[7] = 0xFF;				
			}
			
			// message
			error.AddString(BETRIS_NETWORK_DATAS,errorStr);
			error.AddInt8(BETRIS_NETWORK_SLOT,0);
			if(_destinationMessenger->SendMessage(&error)!=B_OK && BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK)
				printf("ERROR (MessageReceived) : send messenger\n");

			// informer l'application
			_destinationMessenger->SendMessage(BETRIS_CONNEXION_CHANGE_MSG);
		}
		break;
	// fermeture normal ou a cause d'une erreur
	case CONNECTION_CLOSE_MSG:
	case CONNECTION_ERROR_MSG:
		{
			char	slot = -1;
			if(message->FindInt8(BETRIS_NETWORK_SLOT,(int8 *)&slot)==B_OK)
				_close_connection(slot);
				
			// informer l'application
			_destinationMessenger->SendMessage(BETRIS_CONNEXION_CHANGE_MSG);
		}
		break;
	// fermer toutes les connections courante
	case CONNECTION_CLOSE_ALL_MSG:
		_close_all_connection();
		break;
	// envoi du message sur le reseau
	case BETRIS_NETWORK_MSG:
		_send_to_network(message);
		break;		
	default:		
		BLooper::MessageReceived(message);
	}
}

// =================
// = Partie server =
// =================

/**** fonction d'attente de connection en mode server ****/
status_t BeTrisNetwork::_start_listen(BMessage *message)
{
	// attention le serveur tourne deja
	if(_serverStarted)
		return B_ERROR;

	// creation du thread d'ecoute
	if(message->FindInt32(NETWORK_PORT,&_port)!=B_OK)
		return B_ERROR;

	// le serveur est-il un joueur
	if(message->FindBool(NETWORK_LOCAL_PLAYER,&_startlocalplayer)!=B_OK)
		_startlocalplayer = false;

	// mode
	_mode |= SERVER_MODE;

	// creer le socket serveur
	_serverRecvSlot._threadId = spawn_thread(_listen_server_thread,"benettris-listen-thread",B_NORMAL_PRIORITY,NULL);
	resume_thread(_serverRecvSlot._threadId);

	// ok
	return B_OK;
}

/**** stub pour le thread server ****/
int32 BeTrisNetwork::_listen_server_thread(void *datas)
{	return g_network->_listen_server();	}

/**** partie connection server ****/
int32 BeTrisNetwork::_listen_server()
{
	sockaddr_in		address;	// adresse en interne

	// par defaut on a pas reussi
	_serverStarted = false;

	// creation du socket
	_serverRecvSlot._used = true;
	_serverRecvSlot._socket = socket(AF_INET,SOCK_STREAM,0);
	if(_serverRecvSlot._socket<0)
		return B_ERROR;

	// adresse d'ecoute du socket
	address.sin_family = AF_INET;
	address.sin_port = htons(_port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);
	memset(address.sin_zero,0,sizeof(address.sin_zero));

	// tentative de bind	
	if(bind(_serverRecvSlot._socket,(sockaddr *)&address,sizeof(sockaddr))!=0)
	{
		// liberer le socket server
		close(_serverRecvSlot._socket);
		return B_ERROR;
	}

	// ecoute
	if(listen(_serverRecvSlot._socket,5)!=0)
	{
		// liberer le socket server
		close(_serverRecvSlot._socket);

		// fermer toute les connexion clients
		PostMessage(CONNECTION_CLOSE_ALL_MSG);
			
		// l'etat n'est plus bon
		_serverStarted = false;
		return B_ERROR;
	}

	// variable pour les connexion client
	int		sockNewClient = -1;
	char		freeSlot = -1;

	// ok le bind est bon
	_serverStarted = true;

	// informer l'application que le serveur a demarre
	_destinationMessenger->SendMessage(NETWORK_START_SERVEUR_MSG);

	// ok arrive ici le serveur ecoute les connexions client
	// on va donc verifier si il a ete demandee de jouer localement
	if(_startlocalplayer)
	{
		BMessage		connectToServer(NETWORK_CREATE_CLIENT_MSG);

		// ajouter les elements necessaire
		connectToServer.AddString(NETWORK_ADDRESS,"127.0.0.1");
		PostMessage(&connectToServer);
	}

	sockaddr_in		newAddr;
	int				addrSize = sizeof(newAddr);

	// boucle d'ecoute des connexions
	while(!_quit)
	{
		// nouvelle connection (on regarde toute les demi-seconde)
		// on doit faire ca car sinon on a un probleme pour quitter
		// le thread
		sockNewClient = accept(_serverRecvSlot._socket,(sockaddr *)&newAddr,&addrSize);

		// le socket est-il valide ?
		if(sockNewClient>=0)
		{
			// acceptation d'un nouveau client
			freeSlot = _find_free_slot();
			if(freeSlot>0)
			{
				_serverClientSlot[freeSlot]._used = true;
				_serverClientSlot[freeSlot]._socket = sockNewClient;
		
				// on demarre la reception
				_start_reception(freeSlot);
			}
			else
			{
				// detruire le socket
				// on ne peut pas accepter la connexion
				close(sockNewClient);
			}
		}
	}

	// quitter le socket d'ecoute
	close(_serverRecvSlot._socket);
	
	// ok tout est bon
	return B_OK;
}

// =================
// = Partie client =
// =================

/**** connectione en mode client ****/
status_t BeTrisNetwork::_connect_to_server(BMessage *message)
{
	// attention on est deja connecté !
	if(_connexion)
		return B_ERROR;

	BMessage			datas(BETRIS_NETWORK_MSG);
	BString				buffer;
	sockaddr_in			addr;
	hostent				*address = NULL;
	status_t			state = B_OK;

	// mode et port standard
	_mode |= CLIENT_MODE;
	_port = BETRIS_NETWORK_PORT;

	// essayer de s'y connecter
	if(message->FindString(NETWORK_ADDRESS,&_adresse)!=B_OK)
		return B_ERROR;

	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	memset(addr.sin_zero,0,sizeof(addr.sin_zero));

	// trouver l'ip par un nom internet
	address = gethostbyname(_adresse.String());
	if(address==NULL)
		return B_ERROR;

	// simple copie de la premiere adresse trouve
	memcpy(&addr.sin_addr,address->h_addr,address->h_length);

	// creer le soket client
	_serverClientSlot[0]._used = true;
	_serverClientSlot[0]._socket = socket(AF_INET,SOCK_STREAM,0);
	
	// essayer de se connecter
	if(connect(_serverClientSlot[0]._socket,(sockaddr *)(&addr),sizeof(sockaddr))!=0)
		return B_ERROR;

	// encoder la chaine de connection du protocole de tetrinet
	if(!g_BeTrisApp->EncodeConnexionString(buffer,addr))
		return B_ERROR;

	// la connection est ouverte
	_connexion = true;

	// envoyer la commande de connexion
	datas.AddString(BETRIS_NETWORK_DATAS,buffer);
	datas.AddInt8(BETRIS_NETWORK_SLOT,0);
	state = _send_to_network(&datas);

	// on demarre la reception sur le slot client local (le 0)
	// les 6 autres sont pour le serveur
	if(state==B_OK)
		_start_reception(0);
	
	// retourner le resultat	
	return state;
}

// ==========================
// = fonctions de reception =
// ==========================

/**** demarrer la reception ****/
void BeTrisNetwork::_start_reception(char slot)
{
	// si on est serveur l'etat doit etre valide
	if(slot>0 && !_serverStarted)
		return;

	char		*thread_slot;
	BString		threadName;

	// nom du thread
	threadName = "benettris-receiver-";
	threadName << (int32)slot;
	
	// lancer le thread de reception
	thread_slot = new char;
	*thread_slot = slot;
	_serverClientSlot[slot]._threadId = spawn_thread(_reception_thread,threadName.String(),B_NORMAL_PRIORITY,thread_slot);
	resume_thread(_serverClientSlot[slot]._threadId);
}

/**** trouver un slot libre ****/
char BeTrisNetwork::_find_free_slot()
{
	// on parcour les slots de 1 a 7
	// le 0 ne peux pas etre un slot client du serveur
	for(char i=1;i<BETRIS_MAX_CLIENTS+1;i++)
		if(_serverClientSlot[i]._used==false)
			return i;
			
	// auncun de libe
	return -1;
}

/**** stub pour le thread de reception ****/
int32 BeTrisNetwork::_reception_thread(char *data)
{	return g_network->_reception(data);	}

/**** fnction de reception des donnees ****/
int32 BeTrisNetwork::_reception(char *slot)
{
	// on doit avoir un slot valide
	if(slot==NULL)
		return B_ERROR;

	BMessage			message;
	char				messageBuffer[_netbufferSize + 1];
	char				recvBuffer[_netbufferSize + 1];
	int32				receiveSize = 0;
	bool				endConnection = false;
	short				messageBufferSize = 0;
	int					flags = 0;

	// vider les buffer
	memset(recvBuffer,0,_netbufferSize+1);
	memset(messageBuffer,0,_netbufferSize+1);

	// boucle de reception
	endConnection = false;
	while(!_quit && !endConnection)
	{
		// on recoit des donnees
		// on va fixer  le dernier caractere pour etre sure que le strcat fonctionnera
		receiveSize = recv(_serverClientSlot[*slot]._socket,recvBuffer,sizeof(recvBuffer),flags);
		if(receiveSize>=0)
			recvBuffer[receiveSize] = '\0';

		// fin de connection ?
		endConnection = (receiveSize<=0);

		// verifier que l'on pourra concatener
		// sinon c'est une erreur
		if(messageBufferSize+receiveSize+2>_netbufferSize)
		{
			// debug
			if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK)
				printf("ERROR (_reception smot=%d) : buffer to short (need size=%ld)",*slot,(messageBufferSize+receiveSize+2));
		
			// quitter
			endConnection = true;
		}
		
		// connexion coupee ou non accessible
		if(endConnection)
		{
			// fermer le socket
			if(_serverClientSlot[*slot]._socket!=0)
				close(_serverClientSlot[*slot]._socket);
		
			// vider le precedent message
			message.MakeEmpty();

			// terminer la connection a ete coupée
			message.what = CONNECTION_ERROR_MSG;
			message.AddInt8(BETRIS_NETWORK_SLOT,(int8)(*slot));
			PostMessage(&message);
		}
		else
		{
			// on ajoute a notre buffer
			strcat(messageBuffer,recvBuffer);

			// on recoit au moins une commande complete
			if(recvBuffer[receiveSize-1]==(char)0xFF)
			{
				// vider le precedent message
				message.MakeEmpty();

				message.what = BETRIS_NETWORK_MSG;
				message.AddString(BETRIS_NETWORK_DATAS,messageBuffer);
				message.AddInt8(BETRIS_NETWORK_SLOT,(int8)(*slot));
				if(_destinationMessenger->SendMessage(&message)!=B_OK)
					if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK)
						printf("ERROR (_reception slot=%d) : send messenger in Reception\n",*slot);

				// fixer le buffer comme etant vide
				messageBuffer[0] = '\0';
			}
		
			// taille actuel du buffer de concatenation
			messageBufferSize = strlen(messageBuffer);
		}
	}

	// terminer la connection a ete coupée
	message.MakeEmpty();
	message.what = CONNECTION_CLOSE_MSG;
	message.AddInt8(BETRIS_NETWORK_SLOT,(int8)*slot);
	PostMessage(&message);
	
	// effacer la donnee du slot
	delete slot;
	
	// ok on quitte
	return B_OK;
}

// ========================
// = fonctions de d'envoi =
// ========================

/**** envoyer des donnees sur le reseau ****/
status_t BeTrisNetwork::_send_to_network(BMessage *datas)
{
	BString		datasStr;
	long			sizeBuffer = 0;
	long			sentSize = 0;
	char			slot = -1;
	char			slotExcluded = -1;
	int			flags = 0;
	bool			dataSended = false;

	// recuperer la donnees a envoyer
	if(datas->FindString(BETRIS_NETWORK_DATAS,&datasStr)!=B_OK)
		return B_ERROR;

	// slot du client si on en trouve pas c'est pour tout le monde
	if(datas->FindInt8(BETRIS_NETWORK_SLOT,(int8 *)&slot)!=B_OK)
		slot = -1;

	// a-t-on exlu un slot
	if(datas->FindInt8(BETRIS_NETWORK_SLOT_EXCLUDED,(int8 *)&slotExcluded)!=B_OK)
		slotExcluded = -1;

	// la connexion doit exister
	// ou l'etat du serveur etre valide
	if((!_connexion && slot==0) || (!_serverStarted && slot>0))
		return B_ERROR;

	// taille des donnees a envoye + le 0 de fin de chaine et le 0xFF
	sizeBuffer = datasStr.Length() + 1;
	if(sizeBuffer<2)
		return B_ERROR;

	// verifier si on a deja positionne le caractere de fin
	// on va reduire le buffer a recopier
	if((unsigned char)(datasStr[sizeBuffer-2])==0xFF)
		sizeBuffer--;
	
	// reallocation du buffer si il est trop petit
	if(sizeBuffer>_netbufferSize)
	{
		_sendBuffer = (char *)realloc(_sendBuffer,sizeBuffer*sizeof(char));
		if(_sendBuffer==NULL)
			return B_ERROR;
	
		// ok garder la nouvelle taille
		_netbufferSize = sizeBuffer;
	}
	
	// copier la donnees
	datasStr.LockBuffer(sizeBuffer);
	memcpy(_sendBuffer,datasStr.String(),sizeBuffer);
	datasStr.UnlockBuffer();

	// caractere de fin de chaine Tetrinet
	_sendBuffer[sizeBuffer-1] = 0xFF;

	// c'est peut-etre notre serveur (du joueur local) si le slot vaut 0
	if(slot>=0 && _serverClientSlot[slot]._socket!=0)
	{	
		sentSize = send(_serverClientSlot[slot]._socket,_sendBuffer,sizeBuffer,flags);
		dataSended = true;
	}
	else
	{
		// si on est la c'est un envoi du serveur vers les clients
		// si la valeur du slot est -1 on parcours tout, sinon on evite celui specifie
		for(char i=1;i<=BETRIS_MAX_CLIENTS;i++)
		{
			if(slotExcluded!=i)
			{
				// envoyer si le socket existe et qu'il est utilise
				if(_serverClientSlot[i]._socket!=0 && _serverClientSlot[i]._used==true)
				{
					sentSize = send(_serverClientSlot[i]._socket,_sendBuffer,sizeBuffer,flags);
					dataSended = true;
				}
			}
		}
	}

	// est-on en mode debug
	if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK && dataSended)
	{
		// on doit mettre le carectere 0 pour afficher la chaine
		_sendBuffer[sizeBuffer-1] = '\0';
		printf("************** NETWORK **********************\n");
		printf("* network send buffer      : [%s]\n",_sendBuffer);
		printf("* network send buffer size : [%ld]\n",sizeBuffer);
		printf("* network sended size      : [%ld]\n",sentSize);
		printf("* network slot destination : [%d]\n",slot);
		printf("* network slot excluded    : [%d]\n",slotExcluded);
		if(sentSize!=sizeBuffer)
			printf("* -- [Error Sending data] --\n");
		printf("************** END NETWORK ******************\n");
	}
	
	// a-t-on bien tout envoye
	if(sentSize!=sizeBuffer && dataSended)
		return B_ERROR;
		
	// ok tout c'est bien passe
	return B_OK;
}

// =====================
// = obtenir des infos =
// =====================

/**** la connexion est-elle valide ****/
bool BeTrisNetwork::Connexion()
{
	// cela depend de la connection
	return _connexion;
}


/**** le looper est-il valide ****/
bool BeTrisNetwork::IsStarted()
{
	// cela depend de la connection
	return (_serverStarted & !_quit);
}

/**** mode de fonctionnement ****/
int8 BeTrisNetwork::Mode()
{	return _mode;	}

// ====================
// = autres fonctions =
// ====================

/**** couper une connection client ****/
void BeTrisNetwork::_close_connection(char slot)
{
	// verifier la connexion
	if(_serverClientSlot[slot]._used==true && _serverClientSlot[slot]._socket!=0)
	{
		BMessage		closeMessage(BETRIS_NETWORK_MSG);
		status_t		state;
		status_t		returnState;
		BString			str;

		// fermer le socket server et liberer la memoire
		if(_serverClientSlot[slot]._socket!=0)
			close(_serverClientSlot[slot]._socket);

		// attendre un peu
		snooze(10000);

		// attendre la fermeture du thread de reception
		state = wait_for_thread(_serverClientSlot[slot]._threadId,&returnState);

		// detruire le socket
		if(_serverClientSlot[slot]._socket!=0)
			_serverClientSlot[slot]._socket = 0;
		
		// le slot n'est plus utilise
		_serverClientSlot[slot]._used = false;

		// envoyer en interne
		str = "disconnect";
		str << (char)0xFF;
		closeMessage.AddString(BETRIS_NETWORK_DATAS,str);
		closeMessage.AddInt8(BETRIS_NETWORK_SLOT,slot);
		_destinationMessenger->SendMessage(&closeMessage);
		
		// ok on quitte le client
		if(slot==0)
			_connexion = false;
		
		// information en debug
		if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_NETWORK)
		{
			printf("************** NETWORK **********************\n");
			printf("* connexion closed slot : [%d]\n",slot);
		}
	}
}

/**** couper les connections ****/
void BeTrisNetwork::_close_all_connection()
{
	status_t			state;
	status_t			returnState;

	// quitter les threads de reception
	_quit = true;

	// si les clients sont connecté
	// on envoi un message de fin de connection
	for(char i=1;i<BETRIS_MAX_CLIENTS+1;i++)
		_close_connection(i);

	// le serveur doit etre demarre
	if(_serverRecvSlot._used==true && _serverStarted)
	{
		if(_serverRecvSlot._socket!=0)
			close(_serverRecvSlot._socket);

		// fermer le thread d'ecoute serveur
		state = wait_for_thread(_serverRecvSlot._threadId,&returnState);
		_serverRecvSlot._used = false;
		_serverRecvSlot._socket = 0;
	}
	
	// pas de connection
	_mode = NO_CONNECTION_MODE;
	_serverStarted = false;
	_quit = false;

	// fermeture de connection
	_destinationMessenger->SendMessage(CONNECTION_CLOSE_MSG);

	// informer l'application que le serveur a ete arrete
	_destinationMessenger->SendMessage(BETRIS_CONNEXION_CHANGE_MSG);
}
