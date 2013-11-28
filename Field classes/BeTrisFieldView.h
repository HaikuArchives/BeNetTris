/****************************/
/* vue pour afficher le jeu */
/****************************/
#ifndef BeTrisFieldView_H
#define BeTrisFieldView_H

#include <View.h>

// taille du champ de jeu
#define		FIELD_WIDTH			12
#define		FIELD_HEIGHT		22
#define		BLOCK_NUMBER		7
#define		BLOCK_WAIT			1
#define		BLOCK_SP_BUFFER		256
#define		BLOCK_BUFFER		512

// definition des block speciaux
#define S_ADDALL1      0
#define S_ADDALL2      1
#define S_ADDALL4      2
#define S_ADDLINE      3
#define S_CLEARLINE    4
#define S_NUKEFIELD    5
#define S_CLEARBLOCKS  6
#define S_SWITCH       7
#define S_CLEARSPECIAL 8
#define S_GRAVITY      9
#define S_BLOCKQUAKE   10
#define S_BLOCKBOMB    11

class BBitmap;
class BeTrisNextBlockView;
class BeTrisFieldsView;

// definition de la classe
class BeTrisFieldView : public BView
{
public:
	enum	field_type{Large_Field = 16,Small_Field = 8};
	// metodes de la classe BView
	BeTrisFieldView(BPoint ,char * ,BeTrisNextBlockView *,field_type = Large_Field);

	virtual			~BeTrisFieldView();
	virtual void	AttachedToWindow();
	virtual	void	MessageReceived(BMessage *message);							// gestion des messages
	virtual	void	Draw(BRect updaterect);										// dessin
			void	KeyDown(const char *bytes,int32 numBytes);					// gestion du clavier
			void	MakeEmpty();												// vider le champ
			void	Update(const char *datas);									// mise a jour du champ de jeu
			void	UpdateBitmapTheme();										// on met a jour l'image theme d'origine
			void	DoSpecialBlock(char from,char to,char specialBlock);		// on recoit un block special a traiter
			void	SetInitialHeight(char height);								// hauteur de ligne au demarrage
			void	StartGame();												// demarrage de la partie
			void	StopGame();													// arreter de la partie
			void	PlayerLost();												// on a perdu la partie
			void	FullField(bool sendField);									// remplir le champ
			bool	LockField();												// protection par semaphore
			void	UnlockField();												// deproteger
			BRect	SizeOfField(BPoint,field_type);

	inline	char		*Field()		{ return _field; }		// acces direct au champ
	inline	char		*FieldCache()	{ return _cache; }		// acces direct au cache du champ
	inline	BMessenger	*Messenger()	{ return _messenger; }	// Messenger

private:
	BeTrisNextBlockView		*_nextBlockView;								// vue de previsualisation du bloc prochain
	BeTrisFieldsView		*_fieldsView;									// vue des champs de jeu
	field_type				_type;											// type de champ (petit ou grand)
	BBitmap					*_bitmap;										// acces aux images 
	BBitmap					*_offBitmap;									// buffer de dessin
	BRect					_bounds;										// rcetangle de la vue
	char					_field[FIELD_WIDTH * FIELD_HEIGHT];				// champ interne du jeu
	char					_cache[FIELD_WIDTH * FIELD_HEIGHT];				// cache de la derniere valeur du champ
	char					_drawField[FIELD_WIDTH * FIELD_HEIGHT];			// champ de dessin (avec le bloc en mouvement)
	char					_drawCache[FIELD_WIDTH * FIELD_HEIGHT];			// cache du dessin (avec le bloc en mouvement)
	char					_sendBuffer[FIELD_WIDTH * FIELD_HEIGHT + 5];	// buffer d'envoi de notre champ
	char					_currentBlock;									// bloc courant
	char					_orientBlock;									// orientation du bloc courant
	char					_xblock,_yblock;								// position du bloc courant
	char					_waitCount;										// compteur d'attente
	bool					_waitNextBloc;									// pour attendre un peu avant le prochain bloc
	bool					_lockTimer;										// bloquer le timer pour certaine action
	char					_spblocs[BLOCK_SP_BUFFER];						// buffer des blocs speciaux
	char					_spblocsBuffer[BLOCK_SP_BUFFER];				// buffer de multiplication des blocs speciaux
	sem_id					_semId;											// protection du timer et du clavier
	char					_blockBuffer[BLOCK_BUFFER];						// buffer pour les blocs speciaux (cas des bombs etc...)
	BMessenger				*_messenger;									// pour envoyer le message de dessin
	thread_id				_timerThreadID;									// Id du thread
	bool					_exitThread;									// pour quitter le thread
	time_t					_startGameTime;									// heure de demarrage de la partie
	int						_nbPieceSolidify;								// nombre de piece solidifiee
	BPoint					_bitmapPoint;									// point ou commence l'image
	BRect					_backgroundRect;								// image de fond
	BRect					_blockRect;										// bloc
	
			void	_draw_field();											// dessiner le champ de jeu
			void	_send_field(bool entire=false);							// envoyer le champ (complet ou pas ?)
			char	_block_obstructed(char blockx,char blocky,char orient);	// verifier la collision ou non du bloc
			char	_obstructed(char blockx,char blocky);					// test de collision
			char	_block_rotate(char direction);							// rotation du bloc
			char	_block_down();											// descente du block
			char	_block_drop();											// descendre en bas immediatement
			void	_block_move(char direction);							// deplacement du bloc
			void	_block_place();											// ajouter le bloc au champ
			void	_add_lines(char count);									// ajouter une ligne ou plus
			char	_remove_lines(char *spblocs);							// enlever une ligne ou plus
			void	_shift_line(short line,char d);							// deplacement de ligne
			void	_add_special(char lines);								// ajouter les blocs speciaux
			void	_remove_special();										// effacer un block special
			bool	_solidify();											// solidify
			void	_player_lost();											// envoi du message comme quoi on a perdu
			void	_next_block();											// prochain bloc

	// thread du timer
			void			_stop_timer_thread();
	static	int32			_timer_thread(void *data);
			int32			_timer();
};

#endif