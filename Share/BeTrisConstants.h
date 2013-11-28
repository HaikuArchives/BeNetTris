#ifndef _BETRISCONSTANTS_H
#define _BETRISCONSTANTS_H

#include <InterfaceDefs.h>

#define		BETRIS_DEBUG_NONE			0x0000
#define		BETRIS_DEBUG_MODE_ALL		0x1111
#define		BETRIS_DEBUG_MODE_CLIENT	0x0001
#define		BETRIS_DEBUG_MODE_SERVER	0x0010
#define		BETRIS_DEBUG_MODE_NETWORK	0x0100
#define		BETRIS_DEBUG_MODE_TRACE		0x1000

// application
#define		APPLICATION_SIGNATURE	"application/x-vnd.CKJ.BeNetTris"
#define		BASE_PREFERENCE_PATH	"BeNetTrisSettings"
#define		BETRIS_VERSION			"BeNetTris v0.4 Beta"

// constantes des preferences
#define		P_PATH_DATA				"Data"
#define		P_PATH_MUSICS			"Data"
#define		P_FILE_BITMAP			"TNETBLKS.bmp"
#define		P_FILE_TETRINET			"TETRINET.txt"
#define		P_PATH_USER_PREFS		"User"
#define		P_PATH_SERVER_PREFS		"Server"
#define		P_FILE_USER_PREFS		"users"
#define		P_FILE_SERVER_PREFS		"servers"
#define		P_FILE_APP_PREFS		"BeNetTris"
#define		P_FILE_WINNERS_PREFS	"winners"
#define		P_FILE_REJECTEDS_PREFS	"rejecteds"

// sons
#define		SND_MAX_SOUND			10
#define		SND_SPACE_BAR_BLOCK		1
#define		SND_SOLYDIFY_BLOCK		2
#define		SND_MAKE_LINE			3
#define		SND_MAKE_TETRIS			4
#define		SND_TURN_BLOCK			5
#define		SND_GET_SPECIAL_BLOCK	6
#define		SND_WIN_GAME			7
#define		SND_LOST_GAME			8
#define		SND_MESSAGE_GAME		9
#define		SND_START_GAME			10

// reseau
#define		BETRIS_MAX_CLIENTS		6
#define		BETRIS_NETWORK_PORT		31457
#define		MAX_COMMANDS			32
#define		BLOCK_DEFS				16

// style du texte et couleur
#define			U_BOLD_STYLE		0x1F
#define			U_BOLD_ITALIC		0x0B
#define			U_BOLD_UNDERLINE	0x15
#define			U_CYAN_COLOR		0x03
#define			U_BLACK_COLOR		0x04
#define			U_BLUE_COLOR		0x05
#define			U_GRAY_COLOR		0x06
#define			U_MAGENTA_COLOR		0x08
#define			U_GREEN_COLOR		0x17
#define			U_LIME_COLOR		0x0E
#define			U_SILVER_COLOR		0x0F
#define			U_MAROON_COLOR		0x10
#define			U_DARK_BLUE_COLOR	0x11
#define			U_OLIVE_COLOR		0x12
#define			U_PURPLE_COLOR		0x13
#define			U_RED_COLOR			0x14
#define			U_TEAL_COLOR		0x0C
#define			U_WHITE_COLOR		0x18
#define			U_YELLOW_COLOR		0x19

// couleurs
const rgb_color		U_COULEUR_NOIR 			= {0,0,0,0};
const rgb_color		U_GRIS_TRES_FONCE		= {97,97,97,255};
const rgb_color		U_GRIS_FONCE 			= {156,156,156,255};
const rgb_color		U_GRIS_STANDARD			= {219,219,219,255};
const rgb_color		U_GRIS_CLAIR			= {186,186,186,255};
const rgb_color		U_GRIS_TRES_CLAIR		= {239,239,239,255};
const rgb_color		U_COULEUR_BLANC			= {255,255,255,255};
const rgb_color		U_ROUGE_CLAIR			= {255,0,0,255};
const rgb_color		U_ROUGE_FONCE			= {128,0,0,255};
const rgb_color		U_VERT_CLAIR			= {0,255,0,255};
const rgb_color		U_VERT_FONCE			= {0,128,0,255};
const rgb_color		U_BLEU_CLAIR			= {0,0,231,255};
const rgb_color		U_BLEU_FONCE			= {0,0,128,255};
const rgb_color		U_ORANGE_CLAIR			= {255,200,0,255};
const rgb_color		U_VIOLET_FONCE			= {156,0,255,255};
const rgb_color		U_VIOLET_STANDARD		= {170,170,255,255};
const rgb_color		U_VIOLET_CLAIR			= {210,210,255,255};
const rgb_color		U_TURQUOISE_STANDARD	= {0,196,200,255};
const rgb_color		U_MAGENTA_STANDARD		= {255,0,255,255};
const rgb_color		U_JAUNE_STANDARD		= {255,255,0,255};

// joueur
#define		NUMBER_CONTROL_KEY		6

#endif