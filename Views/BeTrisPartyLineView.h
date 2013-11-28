
#ifndef BeTrisPartyLineView_H
#define BeTrisPartyLineView_H

#include <View.h>
#include <TextView.h>
#include <TextControl.h>
#include <ListView.h>
#include <Button.h>
#include <Messenger.h>
#include <PopUpMenu.h>
#include "BeTrisApp.h"

/**** Messages pour la vue et les controles ****/
#define			U_ENTER_TEAM_MSG 		'Uetm'
#define			U_ENTER_CHAT_MSG 		'Uecm'
#define			U_UPDATE_TEAM_MSG 		'Uutm'
#define			U_REJECT_MAN_MSG 		'Urmm'
#define			U_KICK_MAN_MSG 			'Ukmm'
#define			U_BAN_MAN_MSG 			'Ubmm'
#define			U_START_STOP_GAME_MSG	'Ussg'
#define			U_COLOR_MENU_MSG		'Ucmm'
#define			U_SELECT_PLAYER_MSG		'Uspm'

// constantes
#define			MAX_CHAT_LEN_MESSAGE		255

class BeTrisPlayerItem;
class BeTrisChatEdit;
class CPreferenceFile;

// definition de la classe
class BeTrisPartyLineView : public BView
{
public:
	BMessenger		winmessenger;

	BeTrisPartyLineView(BRect , const char *);
	virtual ~BeTrisPartyLineView();

	virtual void	AttachedToWindow();
	virtual void	MessageReceived(BMessage *message);	

			void				AddChatMessage(char from,const char *text,rgb_color *color=NULL,bool info=true);	// reception d'un message
			bool				AddPlayer(BeTrisPlayerItem *player);												// ajouter un joueur
			void				RemovePlayer(BeTrisPlayerItem *player);												// enlever un joueur
			void				SaveBadPlayers();																	// enregistre la liste des mauvais joueurs
			BeTrisPlayerItem	*LocalPlayer();																		// recuperer le joueur locale

	friend class	BeTrisChatEdit;

protected:
	CPreferenceFile		*_rejectedplayers;
	BTextView			*_infoteam;
	BTextControl		*_enterteam;
	BTextView			*_chattext;
	BeTrisChatEdit		*_enterchat;
	BButton				*_updateteam;	
	BButton				*_rejectman;	
	BButton				*_kickman;	
	BButton				*_banman;
	BButton				*_startgame;
	BListView			*_badplayerlist;
	BListView			*_playerlist;
	BPopUpMenu			*_textcolor;
	BFont				*_fixed_font;
	BFont				*_bold_font;	
	
			void				_send_chat_message();				// envoyer un message
			void				_update_team();						// mise a jour de notre equipe
			void				_select_player(BMessage *message);	// selection d'un joueur dans la liste
			void				_reject_player();					// on rejete un joueur
			void				_empty_list();						// vider la liste des joueurs
			void				_update_gui();						// changer l'etat de la GUI
			void				_start_stop_game();					// demarrage et arret d'une partie
			void				_load_bad_players();				// charger la liste des mauvais joueurs
};

// classe particuliere d'edition du texte de chat
class BeTrisChatEdit : public BTextView
{
public:
	BeTrisChatEdit(BRect frame,const char *name,BRect textRect);
	virtual ~BeTrisChatEdit();

	virtual	void	Draw(BRect updateRect);										// dessin
			void	KeyDown(const char *bytes,int32 numBytes);					// gestion du clavier

protected:
	BRect	_bounds;	// rectangle du champ edit

			void	_find_command_complete_word(BString &text);		// completer le nom d'une commande
			void	_find_player_complete_word(BString &text);		// completer le nom
};

#endif