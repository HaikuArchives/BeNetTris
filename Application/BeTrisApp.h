#ifndef BETRISAPP_H
#define BETRISAPP_H

#include "BeTrisWindow.h"
#include "BeTrisConstants.h"
#include "BeTrisFieldView.h"
#include "BeTrisSettingView.h"
#include "BeTrisPlayerItem.h"
#include "BeTrisFieldView.h"

#include <Application.h>
#include <List.h>
#include <String.h>
#include <NetAddress.h>
#include <Path.h>
#include <Resources.h>

// type de messages recu
enum _cmd_type
{
    CMD_UNUSED,CMD_CONNECTED,
    CMD_CONNECT,CMD_DISCONNECT,CMD_CONNECTERROR,
    CMD_PLAYERNUM,CMD_PLAYERNUM_TFAST,CMD_PLAYERJOIN,CMD_PLAYERLEAVE,CMD_KICK,
    CMD_TEAM,
    CMD_PLINE,CMD_PLINEACT,
    CMD_PLAYERLOST,CMD_PLAYERWON,
    CMD_NEWGAME,CMD_NEWGAME_TFAST,CMD_INGAME,CMD_PAUSE,CMD_STARTGAME,CMD_ENDGAME,
    CMD_F,CMD_SB,CMD_LVL,CMD_GMSG,
    CMD_WINLIST,
    CMD_SPECJOIN,CMD_SPECLEAVE,CMD_SPECLIST,CMD_SMSG,CMD_SACT,
    CMD_BTRIXNEWGAME,
    CMD_TETRISSTART,CMD_TETRIFASTER,
    CMD_VERSION,
    CMD_CLIENTINFO,
    CMD_PING,CMD_PONG,
    CMD_NOENTRY
};

// structure de partie
struct _game_data
{
	char	_initialStackHeight;		// hauteur d'origine
	short	_initialLevel;				// level initial
	char	_linesperLevel;				// ligne en plus par level
	char	_levelInc;					// increment de level
	char	_specialLines;				// nombre de ligne pour faire un bloc special
	char	_specialCount;				// bloc speciaux ajouter a chaque fois
	char	_specialCapacity;			// capacité des blocs speciaux
	int		_levelAverage;				// level moyen ou pas
	int		_classicMode;				// mode classic ou non
	int		_blockFreq[7];				// frequence des blocs
	int		_specialFreq[9];			// frequence des blocs speciaux
	int		_timing;					// timing

	int		_llines;					// lignes restantes pour incrementer le level
	int		_slines;					// les lignes restantes pour composer un block special
	int		_lineCount;					// nombre de lignes au total
	int		_average;					// niveaux moyen
	int		_ppm;						// nombre de piece par minutes
	char	_mblines;					// minutes avant ajout de lignes
	char	_sblines;					// secondes entre chaque ajout de lignes
};

// definition du type des blocs
typedef char _betrisblock[4][4];

class CPreferenceFile;
class BeTrisNetwork;
class BBitmap;

class BeTrisApp : public BApplication
{
public:
	BeTrisApp();
	virtual ~BeTrisApp();

	virtual	bool	QuitRequested(void);
	virtual	void	MessageReceived(BMessage *);
			
			bool				IsConnected();																	// etat de connection
			bool				ServerIsRunning();																// le serveur tourne-t-il ?
			char				*Command(_cmd_type type);														// commande en chaine de caractere
			char				FieldFromPlayer(char number);													// trouver le champ associe a un joueur
			void				UpdateGameTiming();																// mettre a jour le timing du jeu
			void				BlitBitmap(BRect source,BBitmap *bmpSrc,short dx,short dy,BBitmap *bmpDest);	// recopier de l'image ou tout se trouve vers la bitmap off screen
			char				RandomNum(char scale);															// tirer un chiffre aleatoire
			char				TranslateBlock(char block);														// traduire en block
			rgb_color			*TranslateColor(char color);													// recuperer une couleur
			bool				EncodeConnexionString(BString &string,sockaddr_in &addr);						// encoder la chaine de connexion
			bool				DecodeConnexionString(BString &string);											// decoder la chaine de connexion
			void				SetControlKey(char index,char key);												// touche de controle
			void				PlaySound(char sndIndex);														// jouer un son
			BeTrisPlayerItem	*ServerPlayer(char number);														// acces au donnees d'un joueur (en mode serveur)
			void				SetActivatedPlayer(BeTrisPlayerItem *player);									// retourner le joueur actif
			BeTrisPlayerItem	*GetClientPlayer(char playerNumber);											// recuperer le joueur numero n
			
	// acces donnees
	inline	BeTrisPlayerItem		*ActivatedPlayer()								{ return _playersClient[0]; }	// joueur actif
	inline	BResources				*Resources()									{ return &_ressource; }			// acces aux ressources
	inline	const BeTrisNetwork		*NetWork() const								{ return _networkLooper; }		// acces a la classe reseau
	inline	CPreferenceFile			*Preferences()									{ return _prefs; }				// acces au preferences
	inline	CPreferenceFile			*PreferencesUser()								{ return _prefsUser; }			// acces au preferences utilisateur
	inline	CPreferenceFile			*PreferencesNetwork()							{ return _prefsNetwork; }		// acces au preferences reseau
	inline	BBitmap					*Bitmaps()										{ return _bitmap; }				// acces aux images
	inline	_game_data				*GameData()										{ return &_gameData; }			// acces aux donnees de la partie
	inline	_game_data				*GameDataServer()								{ return &_gameDataServer; }	// acces aux donnees de la partie du serveur
	inline	char					BlockOrientation(char index)					{ return _blockOrient[index]; }	// orientation des blocs
	inline	_betrisblock			**Block()										{ return _blocks; }				// acces aux blocs
	inline	char					BlockDesf(char index)							{ return _blocks_def[index]; }	// definitiotn des blocs
	inline	char					PlayerCount()									{ return _playerCount; }		// nombre de joueur
	inline	char					PlayerCountServer()								{ return _playerCountServer; }	// nombre de joueur
	inline	bool					GamePause()										{ return _pause; }				// etat de la pause
	inline	bool					GameStarted()									{ return _gameStarted; }		// etat de la partie
	inline	char					ControlKey(char index)							{ return _controlKey[index]; }	// touche de controle
	inline	BPath					*ApplicationPath()								{ return &_appPath; }			// chemin de l'application
	inline	char					*CustomField()									{ return _customfield; }		// champ personnalise
	inline	void					SetUseCustomField(bool value)					{ _usecustomfield = value; }	// fixer l'utilisation du champ personnalise
	inline	bool					UseCustomField()								{ return _usecustomfield; }		// utilisation du champ personnalise

protected:
	BResources				_ressource;					// ressource de l'application
	CPreferenceFile			*_prefs;					// preferences
	CPreferenceFile			*_prefsUser;				// preferences utilisateur
	CPreferenceFile			*_prefsNetwork;				// preferences reseau et serveur
	BeTrisNetwork			*_networkLooper;			// Looper Reseau
	BeTrisWindow			*_pGameWindow;				// fenetre principale
	char					_blocks_def[BLOCK_DEFS];	// definition des block
	BPath					_appPath;					// chemin de l'application
	BBitmap					*_bitmap;					// image (block et champ de jeu)
	BString					_commands;					// commandes venant du reseau
	BString					_datas[MAX_COMMANDS];		// commande + donnees (la commande est en index 0)
	_betrisblock			*_blocks[7];				// blocs
	char					_blockOrient[7];			// orientation pour chaque blocs
	bool					_gameStarted;				// etat de la partie
	bool					_pause;						// le jeu est en pause
	char					_customfield[FIELD_WIDTH * FIELD_HEIGHT];	// champ de jeu perso
	bool					_usecustomfield;							// utilisation du champ personnalise
	bool					_exitTimerThread;							// pour sortir du thread de timing d'ajout de ligne
	thread_id				_timerThreadID;								// Id du thread

	BeTrisPlayerItem		*_playersClient[BETRIS_MAX_CLIENTS];		// joueurs connecte en mode de jeu (joueur local actif est le zero)
	_game_data				_gameData;									// structure des infos de la partie local
	char					_playerField[BETRIS_MAX_CLIENTS];			// la relation entre les champ de jeu et les numero de joueur
	char					_playerCount;								// nombre de joueur connecte (mode client)
	char					_controlKey[NUMBER_CONTROL_KEY];			// touche de controle

	BeTrisServerPlayerItem	*_playersOnServer[BETRIS_MAX_CLIENTS];		// joueur connectes au serveur
	_game_data				_gameDataServer;							// structure des infos de partie pour le serveur
	char					_playerCountServer;							// nombre de joueur connecte (mode serveur)

				void				_find_command(BMessage *message);								// trouver les commandes
				char				_find_command_id();												// trouver l'id de la commande
				void				_execute_commands_client(char nbCommand);						// traiter les commandes client
				void				_execute_commands_server(char nbCommand,char slot);				// traiter les commandes en tant que serveur
				bool				_check_connexion_client(char slot);								// verifier si c'est une connexion client
				void				_accept_new_client(char slot);									// on accepte un nouveau client
				void				_init_player_field_tab();										// mettre a zero le liens champs de jeu joueur
				char				_find_empty_field_tab();										// trouver un champ vide
				void				_reorganize_fields();											// reorganiser les champs de jeu
				void				_load_theme_bitmap();											// charger l'image et decouper les elements
				void				_special_blocs(BMessage *message);								// on recupere des blocs speciaux
				void				_refresh_display();												// rafraichir les donnees du jeu affichees
				unsigned char		_hex_to_int(char *data);										// transformer une chaine genre A0 en sa valeur
				void				_init_server_data(bool start);									// initialiser le donnees avant demarrage ou arret du serveur
				void				_stop_addlines_thread();										// arreter le thread de addlines

	// thread du timer pour declancher les add_lines
	// si la partie dure trop longtemps
	static	int32			_timer_add_thread(void *data);
			int32			_timer_add();
};	

// Acces au looper directement
extern 	BeTrisApp		*g_BeTrisApp;

#endif
