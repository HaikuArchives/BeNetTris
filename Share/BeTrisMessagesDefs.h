#ifndef _BETRISMESSAGEDEFS_H
#define _BETRISMESSAGEDEFS_H

// application
#define		BETRIS_REFRESH_PREFS			'Trpr'
#define		BETRIS_SAVE_PREFS				'Tspr'
#define		BETRIS_CONNEXION_CHANGE_MSG		'Tccm'

// jeu
#define		BETRIS_REFRESH_MSG				'Trfd'
#define		BETRIS_SPBLOCK_MSG				'Tspb'
#define		BETRIS_SPBLOCK_DATAS			"special-block"
#define		BETRIS_SPBLOCK_REMOVE			"special-block-remove"
#define		BETRIS_SPBLOCK_FIELD			"special-block-field"
#define		BETRIS_STARTGAME_MSG			'Tstg'
#define		BETRIS_DISPLAY_FIELDS_MSG		'Tdfm'
#define		BETRIS_DRAW_FIELD_MSG			'Tdfl'
#define		BETRIS_DRAW_NEXTBLOCK_MSG		'Tdnb'
#define		BETRIS_DRAW_SPECIAL_MSG			'Tdsp'

// chat
#define		BETRIS_SEND_CHAT_MESSAGE_MSG	'Tscm'
#define		BETRIS_CHAT_MESSAGE_MSG			'Tcms'

// information client
#define		PLAYER_NAME						"player-name"
#define		PLAYER_NICKNAME					"player-nick-name"
#define		PLAYER_TEAM						"player-team"
#define		PLAYER_COLOR					"player-color"
#define		PLAYER_LOCAL					"player-local"
#define		PLAYER_ID						"player-id"
#define		PLAYER_REJECTED					"player-rejected"

// sauvegarde des infos serveurs
#define		SERVER_IP						"server-ip"

// informations reseau
#define		BETRIS_NETWORK_MSG				'Tntm'
#define		BETRIS_NETWORK_DATAS			"network-datas"
#define		BETRIS_NETWORK_SLOT				"network-slot"
#define		BETRIS_NETWORK_SLOT_EXCLUDED	"network-slot-excuded"

// gagnant
#define		WINNER_NAME						"winner-name"
#define		WINNER_ISTEAM					"winner-isteam"
#define		WINNER_SCORE					"winner-score"

#endif