#ifndef BeTrisSettingView_H
#define BeTrisSettingView_H

#include "BeTrisApp.h"
#include "BeTrisConstants.h"

#include <View.h>
#include <String.h>

// definition des messages
#define			U_CHOOSE_MUSIC_MSG			'Ucmm'
#define			U_CHOOSE_DATAS_MSG			'Ucdm'
#define			U_CHOOSE_CUSTOMFIELD_MSG	'Ucfm'
#define			U_SELECT_PLAYER_MSG			'Uspm'
#define			U_ADD_PLAYER_MSG			'Uapm'
#define			U_MODIFY_PLAYER_MSG			'Umpm'
#define			U_ACTIVE_PLAYER_MSG			'Uapl'
#define			U_REMOVE_PLAYER_MSG			'Urpm'
#define			U_NICKNAME_PLAYER_MSG		'Unnm'
#define			U_NAME_PLAYER_MSG			'Unpm'
#define			U_TEAM_PLAYER_MSG			'Utpm'
#define			U_COLOR_PLAYER_MSG			'Ucpm'
#define			U_PANEL_CONTROL_MSG			'Upcm'

class BeTrisPlayerItem;
class BeTrisKeyEdit;
class BBox;
class BTextControl;
class BButton;
class BTextView;
class BListView;
class BColorControl;
class BFilePanel;
class BCheckBox;

// definition de la classe
class BeTrisSettingView : public BView
{
public:
	BeTrisSettingView(BRect , const char *);
	virtual ~BeTrisSettingView();

	virtual void		AttachedToWindow();						// attachement de la vue
	virtual void		MessageReceived(BMessage *message);		// gestion des message
			void		SavePreferences(bool save=true);		// sauver les preferences
			void		DisplayFilePanel(int32 what);			// afficher le selecteur de fichier

protected:
	BBox			*_keyconfig;
	BTextView		*_labelkey[NUMBER_CONTROL_KEY];
	BeTrisKeyEdit	*_controlkey[NUMBER_CONTROL_KEY];

	BBox			*_pathconfig;
	BTextView		*_infomusic;
	BTextView		*_infodatas;
	BTextView		*_infocustomfield;
	BTextControl	*_musicpath;
	BTextControl	*_dataspath;
	BTextControl	*_customfieldpath;
	BButton			*_choosemusicpath;
	BButton			*_choosedataspath;
	BButton			*_choosecustomfieldpath;

	BBox			*_playerconfig;
	BTextView		*_infoplayer;
	BListView		*_playerlist;
	BButton			*_addplayer;
	BButton			*_modifyplayer;
	BButton			*_removeplayer;
	BTextControl	*_nickname;
	BTextControl	*_name;
	BTextControl	*_team;
	BColorControl	*_choosecolor;
	BTextView		*_infocolorplayer;
	BTextView		*_zonecolor;

	BBox			*_extendedconfig;
	BCheckBox		*_tetrinet114compliant;
	BCheckBox		*_tetrifastcompliant;
	BCheckBox		*_knowasbenettris;
	BCheckBox		*_usecustomfield;

	// file panel to select directory
	BFilePanel		*_searchdirectory;
	int8			_openmodepanel;

				void		_add_player();						// ajouter un joueur local
				void		_modify_player();					// modifier un joueur local
				void		_select_player(BMessage *message);	// selection d'un joueur dans la liste
				void		_refresh_preferences();				// charger les prefs
				bool		_load_custom_field();				// charger le champ perso
				void		_update_gui();						// changer l'etat de la GUI
};

// classe particuliere pour la saisie des touches
class BeTrisKeyEdit : public BView
{
public:
	BeTrisKeyEdit(BRect frame,const char *name,char keyIndex);
	~BeTrisKeyEdit();
	
	virtual	void	Draw(BRect updateRect);
	virtual	void	KeyDown(const char *bytes,int32 numBytes);
	virtual	void	MouseDown(BPoint point);
	virtual	void	MakeFocus(bool focused = true);
			void	SetEnabled(bool enabled);
			bool	SetKey(char key);							// definition de la touche

	inline	char	Key()				{ return _key; }		// acces a la touche

protected:
	bool		_enabled;
	char		_index;
	char		_key;
	BString		_text;
};

#endif