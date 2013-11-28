/****************************/
/* vue pour afficher le jeu */
/****************************/
#include "BeTrisApp.h"
#include "BeTrisWindow.h"
#include "BeTrisConstants.h"
#include "BeTrisMessagesDefs.h"
#include "BeTrisDebug.h"
#include "BeTrisFieldView.h"
#include "BeTrisFieldsView.h"
#include "BeTrisNextBlockView.h"
#include "BeTrisNetwork.h"
#include "BeTrisPlayerItem.h"
#include "CPreferenceFile.h"

#include <Bitmap.h>
#include <Messenger.h>
#include <Control.h>
#include <OS.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

/**** constructeur ****/
BeTrisFieldView::BeTrisFieldView(BPoint place,char * name,BeTrisNextBlockView *nextBlockView,BeTrisFieldView::field_type type)
: BView(SizeOfField(place,type),name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BRect		fieldRect;

	// retenir la taille
	_messenger = NULL;
	_bitmap = NULL;
	_type = type;
	_offBitmap = NULL;
	_bounds = Bounds();
	_currentBlock = -1;
	_orientBlock = 0;
	_xblock = 4;
	_yblock = 0;
	_currentBlock = -1;
	_orientBlock = -1;
	_nextBlockView = nextBlockView;
	_fieldsView = NULL;
	_waitCount = 0;
	_waitNextBloc = false;
	_lockTimer = false;
	_semId = 0;
	_timerThreadID = 0;
	_exitThread = false;
	_messenger = NULL;

	// image d'affichage
	fieldRect = Bounds();
	fieldRect.right -= 2;
	fieldRect.bottom -= 2;
	_offBitmap = new BBitmap(fieldRect,B_RGB32,true);

	// en fonction de la taille
	_blockRect.Set(0,16,7,23);
	_bitmapPoint.Set(192,24);
	if(_type==Large_Field)
	{
		_blockRect.Set(0,0,15,15);
		_bitmapPoint.Set(0,24);
	}

	// vider le tableau
	memset(_field,0,FIELD_WIDTH*FIELD_HEIGHT);
	memset(_cache,0,FIELD_WIDTH*FIELD_HEIGHT);
	memset(_drawCache,0,FIELD_WIDTH*FIELD_HEIGHT);

	if(_type==Large_Field)
		if((_semId = create_sem(1,"field-draw-sem"))<B_OK)
			printf("Error creating BeTrisAPP semaphore\n");
	
	// dessiner
	UpdateBitmapTheme();
}

/**** destructeur ****/
BeTrisFieldView::~BeTrisFieldView()
{
	// arreter le thread
	_stop_timer_thread();
	
	// detruire la semaphore
	if(_type==Large_Field)
		if(_semId>=B_OK)
			delete_sem(_semId);

	// liberer l'image buffer
	if(_offBitmap!=NULL)
		delete _offBitmap;

	// detruire le messenger
	delete _messenger;
}

/**** attached to window ****/
void BeTrisFieldView::AttachedToWindow()
{
	// appel parent
	BView::AttachedToWindow();

	// vue des champs de jeu
	_fieldsView = (BeTrisFieldsView *)(Parent());

	// creer le messenger
	if(_messenger==NULL)
		_messenger = new BMessenger(this);
}

/**** gestion des messages ****/
void BeTrisFieldView::MessageReceived(BMessage *message)
{
	switch(message->what)
	{
	//dessin
	case BETRIS_DRAW_FIELD_MSG:
		{
			// dessin du jeu
			if(Looper()!=NULL)
				DrawBitmap(_offBitmap,BPoint(1,1));
		}
		break;
	default:
		BView::MessageReceived(message);
	}
}

/**** Draw ****/
void BeTrisFieldView::Draw(BRect updaterect)
{
	rgb_color	OldHeighColor;
	rgb_color	OldLowColor;

	OldHeighColor = HighColor();
	OldLowColor = LowColor();

	// dessin du cadre
	SetHighColor(U_GRIS_FONCE);
	StrokeLine(BPoint(_bounds.left,_bounds.top),BPoint(_bounds.right,_bounds.top));
	StrokeLine(BPoint(_bounds.left,_bounds.top),BPoint(_bounds.left,_bounds.bottom));
	SetHighColor(U_COULEUR_BLANC);
	StrokeLine(BPoint(_bounds.right,_bounds.bottom),BPoint(_bounds.left,_bounds.bottom));
	StrokeLine(BPoint(_bounds.right,_bounds.top),BPoint(_bounds.right,_bounds.bottom));

	// retablir la couleur du fond
	SetHighColor(OldHeighColor);
	SetLowColor(OldLowColor);

	// redessiner le champ
	DrawBitmap(_offBitmap,BPoint(1,1));
}

/**** gestion clavier ****/
void BeTrisFieldView::KeyDown(const char *bytes,int32 numBytes)
{
	// ca doit etre le grand champ de jeu
	if(_type!=Large_Field)
		return;

	// la partie ne doit pas etre en pause
	if(g_BeTrisApp->GamePause())
		return;

	// joueur local
	BeTrisPlayerItem	*player = NULL;

	// on doit avoir un joueur local
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;
		
	// on doit encore etre de la partie
	// donc le thread doit tourner
	if(player->EndGameTime()!=0)
		return;

	// on peut lancer un special sur le numero de joueur
	// effacer une option
	switch(*bytes)
	{
		// effacement
		case 'd':
		case 'D':
			_remove_special();
			break;
		// tchat
		case 't':
		case 'T':
			return;				
		// on lance un bloc special
		// on utilise les touche Fxx ou le numero du joueur
		case B_FUNCTION_KEY:
		case 0x26:		
		case 0xffffffc3:		
		case 0x22:		
		case 0x27:		
		case 0x28:		
		case 0x2d:
		case 0x31:		
		case 0x32:		
		case 0x33:		
		case 0x34:		
		case 0x35:		
		case 0x36:		
			{
				// envoyer le message pour executer un bloc special
				BMessage	spMessage(BETRIS_SPBLOCK_MSG);
				BMessage	*msg = NULL;
				int32		field = -1;

				// on a utilise des touche Fxx
				if(*bytes==B_FUNCTION_KEY)
				{
					status_t	state;
	
					// on doit trouver la touche Fx
					msg = Window()->CurrentMessage();
					state = msg->FindInt32("key", &field);
					if(state!=B_OK)
						return;
	
					// on doit enlever 2 (F1=2)
					// car on demande le champ (et il commence a 0)
					field -= 2;
				}
				else
				{
					// c'est le chiffre
					if(*bytes>0x30 && *bytes<0x37)
						field = *bytes - 0x30;
					else
					{
						char	playerNumber = 0;
					
						// c'est la accents etc...
						switch(*bytes)
						{
						case 0x26:
							playerNumber = 1;
							break;
						case 0xffffffc3:		
							playerNumber = 2;
							break;
						case 0x22:		
							playerNumber = 3;
							break;
						case 0x27:		
							playerNumber = 4;
							break;
						case 0x28:		
							playerNumber = 5;
							break;
						case 0x2d:
							playerNumber = 6;
							break;
						}
					
						// obtenir le champ du joueur
						field = g_BeTrisApp->FieldFromPlayer(playerNumber);
					}
				}
				
				// envoyer si on c'est un champ qui existe
				if(field>=0 && field<BETRIS_MAX_CLIENTS)
				{
					spMessage.AddInt8(BETRIS_SPBLOCK_FIELD,field);
					BMessenger(be_app).SendMessage(&spMessage);
				}
			}
			break;
		default:
		{
			// pour le resultat des actions
			char	result = 0;
			char	keyFinded = -1;

			// verifier que c'est bien une touche
			for(char i=0;(i<6) && (keyFinded<0);i++)
				if(*bytes==g_BeTrisApp->ControlKey(i))
					keyFinded = i;
			
			switch(keyFinded)
			{
			// deplacer le bloc vers la gauche
			case 0:
				_block_move(-1);
				break;
			// deplacer le bloc vers la droite
			case 1:
				_block_move(1);
				break;
			// rotation droite
			case 2:
				g_BeTrisApp->PlaySound(SND_TURN_BLOCK);
				result = _block_rotate(1);
				break;
			// descendre
			case 3:
				result = _block_down();
				break;
			// directement en bas
			case 4:
				result = _block_drop();
				break;
			// rotation gauche
			case 5:
				g_BeTrisApp->PlaySound(SND_TURN_BLOCK);
				result = _block_rotate(-1);
				break;
			// pas de touche de controle
			default:
				return;
			}
		
			// le resultat peut dire que le bloc est bloque
			// dans ce cas verifier si on doit pas effacer une ligne
			// et lancer le bloc suivant
			if(result)
				_solidify();

			// dessin
			_draw_field();
		}
	}
}

/**** metodes pour dimentionner correctement le jeu ****/
BRect BeTrisFieldView::SizeOfField(BPoint point,BeTrisFieldView::field_type typeshow)
{
	BRect	frame;
	
	frame.Set(point.x,point.y,(point.x+12*typeshow+1),(point.y+22*typeshow+1));
	return frame;
}

/**** vider le champ ****/
void BeTrisFieldView::MakeEmpty()
{
	// proteger
	if(!LockField())
		return;

	memset(_field,0,FIELD_WIDTH*FIELD_HEIGHT);
	memset(_cache,0,FIELD_WIDTH*FIELD_HEIGHT);
	memset(_drawCache,0,FIELD_WIDTH*FIELD_HEIGHT);

	// deproteger
	UnlockField();
	
	// puis retablir l'image de fond
	UpdateBitmapTheme();
}

/**** mettre a jour le champ en fonction de donnees ****/
void BeTrisFieldView::Update(const char *datas)
{
	// verifier le parametre
	// si c'est NULL on vide le champ et le cache
	if(datas==NULL)
		return;

	// proteger
	if(!LockField())
		return;

	// sauver la valeur du champ actuel
	memcpy(_cache,_field,FIELD_WIDTH*FIELD_HEIGHT);

	// s'occuper du champ complet
	if(datas[0]>='0')
	{
		for(short i=0;i<(FIELD_WIDTH*FIELD_HEIGHT);i++)
			_field[i] = g_BeTrisApp->TranslateBlock(datas[i]);
	}
	else
	{
		// sinon c'est une partie specifique
		short	x = 0;
		short	y = 0;
		char	block = 0;

		// appliquer les modifs
		for(short i=0;(datas[i]!='\0' && i<(FIELD_WIDTH*FIELD_HEIGHT));i++)
		{
			if(datas[i]<'0' && datas[i]>='!')
				block = datas[i] - '!';
			else
			{
				x = datas[i] - '3';
				y = datas[++i] - '3';
				if(x>=0 && x<FIELD_WIDTH && y>=0 && y<FIELD_HEIGHT)
					_field[x + y * FIELD_WIDTH] = block;
			}
		}
	}

	// deproteger
	UnlockField();

	// rafraichir a l'ecran
	_draw_field();
}

/**** on met a jour l'image theme d'origine ****/
void BeTrisFieldView::UpdateBitmapTheme()
{
	BRect	background;

	background.Set(192,24,287,199);
	if(_type==Large_Field)
		background.Set(0,24,191,375);

	// image des dessins (block et champs)
	_bitmap = g_BeTrisApp->Bitmaps();

	// dessin du jeu
	// copie image de fond
	g_BeTrisApp->BlitBitmap(background,_bitmap,0,0,_offBitmap);

	// dessin
	if(Looper()!=NULL)
		_messenger->SendMessage(BETRIS_DRAW_FIELD_MSG);
}

/**** on recoit un block special a traiter ****/
void BeTrisFieldView::DoSpecialBlock(char from,char to,char specialBlock)
{	
	// proteger
	if(!LockField())
		return;

	short	x,y;

	// sauver la valeur du champ actuel
	memcpy(_cache,_field,FIELD_WIDTH*FIELD_HEIGHT);

	// action
	switch(specialBlock)
	{
	// ajout de lignes selon mode de jeu (classic ou pas)
	case S_ADDALL4:
		_add_lines(4);
		break;
	case S_ADDALL2:
		_add_lines(2);
		break;
	case S_ADDALL1:
	case S_ADDLINE:
		_add_lines(1);
		break;
	// effacer la ligne du bas
	case S_CLEARLINE:
		{
			// deplacer vers le bas l reste du champ
			for(y=FIELD_HEIGHT-1;y>0;y--)
				for(x=0;x<FIELD_WIDTH;x++)
					_field[x+y*FIELD_WIDTH] = _field[x+(y-1)*FIELD_WIDTH];
			
			// effacer la ligne du haut
			memset(_field,0,12);			
		}
		break;
	// vider le champ
	case S_NUKEFIELD:
		memset(_field,0,FIELD_WIDTH*FIELD_HEIGHT);
		break;
	// effacer des blocs au hazard
	case S_CLEARBLOCKS:
		{
			// on enleve 10 blocs au hazard
			for(char index=0;index<10;index++)
				_field[g_BeTrisApp->RandomNum(FIELD_WIDTH) + g_BeTrisApp->RandomNum(FIELD_HEIGHT)*FIELD_WIDTH] = 0;
		}
		break;
	// on inverse notre champ avec celui d'un autre
	case S_SWITCH:
		{
			BeTrisFieldView		*fieldFrom = NULL;
			char				fieldIndex = -1;
			char				tmpField[FIELD_WIDTH * FIELD_HEIGHT];
		
			// trouver le champ du joueur
			// c'est forcement plu que 0, le 0 etant toujours le champ du joueur
			fieldIndex = g_BeTrisApp->FieldFromPlayer(from);
			if(fieldIndex>0)
			{			
				// champ de l'expediteur
				fieldFrom = g_BeTrisWindow->_fieldsView->Field(fieldIndex);
				
				// on doit avoir le champs de l'epediteur valide
				if(fieldFrom!=NULL)
				{
					// recopier le champ destination
					memcpy(tmpField,fieldFrom->Field(),FIELD_WIDTH * FIELD_HEIGHT);
					memcpy(fieldFrom->Field(),_field,FIELD_WIDTH * FIELD_HEIGHT);
					memcpy(_field,tmpField,FIELD_WIDTH * FIELD_HEIGHT);
				}
			}
		}
		break;
	// detruire les blocs speciaux
	case S_CLEARSPECIAL:
		{
			// parcourir le champ et remplacer les
			// blocs speciaux par des blocs standard
			for(y=0;y<FIELD_HEIGHT;y++)
				for(x=0;x<FIELD_WIDTH;x++)
					if(_field[x+y*FIELD_WIDTH]>5)
						_field[x+y*FIELD_WIDTH] = g_BeTrisApp->RandomNum(5) + 1;
		}
		break;
	// gravite
	case S_GRAVITY:
        {
        	char	index;
        
        	// enleve les trous en faisant descendre les blocs
			for(y=0;y<FIELD_HEIGHT;y++)
			{
				for(x=0;x<FIELD_WIDTH;x++)
                {
					if(_field[x+y*FIELD_WIDTH]==0)
					{
                    	// deplacer vers le bas les blocs du dessus
						for(index=y;index>0;index--)
							_field[x+index*FIELD_WIDTH] = _field[x+(index-1)*FIELD_WIDTH];
						
						// le dernier est a 0
						_field[x] = 0;
					}
				}
			}
		}
		break;	
	// bloc quake
	case S_BLOCKQUAKE:
        {
        	char	s;
        	char	index;
        	
			// cette algo est completement pris
			// dans le fichier tetrinet.c de gtetrinet
			for(y=0;y<FIELD_HEIGHT;y++)
			{
				s = 0;
				index = g_BeTrisApp->RandomNum(22);
				
				// correction
				if(index<1)
					s++;
				if(index<4)
					s++;
				if(index<11)
					s++;
				if(g_BeTrisApp->RandomNum(2))
					s = -s;
				
				// deplacement des lignes
				_shift_line(y,s);
        	}
        }
		break;
	// bloc bombe
	case S_BLOCKBOMB:
		{
			// cette algo est completement pris
			// dans le fichier tetrinet.c de gtetrinet
			char	index = 0;
			char	ax[] = {-1, 0, 1, 1, 1, 0, -1, -1};
			char	ay[] = {-1, -1, -1, 0, 1, 1, 1, 0};
			short	count = 0;
			char	block;
			
			// touver les blocs bomb (c'est la valeur 14)
			for(y=0;y<FIELD_HEIGHT;y++)
			{
				for(x=0;x<FIELD_WIDTH;x++)
				{
					if(_field[x+y*FIELD_WIDTH]==14)
					{
                        // enlever le bloc
                        _field[x+y*FIELD_WIDTH] = 0;
                        
						// effet d'explosion autour
						for(index=0;index<8;index++)
                        {
                        	// pas de debordement
							if(y+ay[index]>=FIELD_HEIGHT || y+ay[index] < 0 || x+ax[index] >= FIELD_WIDTH || x+ax[index] < 0)
								continue;
							
							block = _field[x + ax[index] + (y + ay[index]) * FIELD_WIDTH];
							
							// attention au bloc bombe
							if(block==14)
								block = 0;
							else
								_field[x + ax[index] + (y + ay[index]) * FIELD_WIDTH] = 0;
							
							// compte des blocs
							_blockBuffer[count] = block;
							count++;
						}
					}
				}
			}        
			// eclater les blocs
			for(index=0;index<count;index++)
				_field[g_BeTrisApp->RandomNum(FIELD_WIDTH) + (g_BeTrisApp->RandomNum(FIELD_HEIGHT-6) + 6) * FIELD_WIDTH] = _blockBuffer[index];
        }
		break;
	default:
		if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
			printf("ERROR (BeTrisFieldView::DoSpecialBlock) : special block not exist\n");
	}

	// enlever les lignes eventuelles
	_remove_lines(NULL);

	// deproteger
	UnlockField();

	// envoyer le champ modifie
	_send_field();

	// redessiner
	_draw_field();
}

/**** hauteur de ligne au demarrage ****/
void BeTrisFieldView::SetInitialHeight(char height)
{
	// on doit avoir des lignes a ajouter
	if(height<=0)
		return;

	// proteger
	if(!LockField())
		return;

	// sauver la valeur du champ actuel
	memcpy(_cache,_field,FIELD_WIDTH*FIELD_HEIGHT);

	// ajout des lignes
	_add_lines(height);

	// deproteger
	UnlockField();

	// envoyer le champ modifie
	_send_field();

	// redessiner
	_draw_field();
}

/**** demarrage de la partie ****/
void BeTrisFieldView::StartGame()
{
	BeTrisPlayerItem	*player = NULL;

	// initialisation
	_startGameTime = time(0);
	_nbPieceSolidify = 0;

	// joueur local est dans la partie
	player = g_BeTrisApp->ActivatedPlayer();
	if(player!=NULL)
		player->SetEndGameTime(0);

	// si c'est un type large (notre joueur)
	// on va creer une semaphore pour proteger le redessin
	// et on lance le thread de jeu
	if(_type==Large_Field)
	{
		if((_semId = create_sem(1,"field-draw-sem"))<B_OK)
			printf("Error creating BeTrisAPP semaphore\n");

		// le thread existe que pour le grans champ
		_exitThread = false;
		_timerThreadID = spawn_thread(_timer_thread,"benettris-timer-thread",B_NORMAL_PRIORITY,this);
		resume_thread(_timerThreadID);
	}

	// debloquer le compteur
	_lockTimer = false;

	// son de demarrage
	g_BeTrisApp->PlaySound(SND_START_GAME);	

	// prochain block
	_waitNextBloc = true;
}

/**** arreter de la partie ****/
void BeTrisFieldView::StopGame()
{
	BeTrisPlayerItem	*player = NULL;

	// annuler le bloc courant
	_currentBlock = -1;

	// arreter le thread
	_stop_timer_thread();
	
	// joueur local est dans la partie
	player = g_BeTrisApp->ActivatedPlayer();
	if(player!=NULL && player->EndGameTime()==0)
		player->SetEndGameTime(time(0));
	
	// redessiner
	_draw_field();
}

/**** on a perdu la partie ****/
void BeTrisFieldView::PlayerLost()
{
	BeTrisPlayerItem	*player = NULL;
	BMessage			outMessage(BETRIS_NETWORK_MSG);
	BString				outStr;

	// on doit avoir un joueur local
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;
		
	// on doit encore etre de la partie, sinon on a deja executer la commande
	if(player->EndGameTime()!=0)
		return;
	
	// quitter le thread
	_stop_timer_thread();

	// remplir le champ
	FullField(true);

	// joueur local n'est plus dans la partie
	if(player!=NULL)
		player->SetEndGameTime(time(0));
		
	// stopper la partie
	outStr = g_BeTrisApp->Command(CMD_PLAYERLOST);
	outStr << " ";
	if(player!=NULL)
		outStr << (int32)(player->Number());

	// enoyer au serveur et en interne
	outMessage.AddString(BETRIS_NETWORK_DATAS,outStr);
	outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
	g_network_messenger.SendMessage(&outMessage);

	// puis effacer le prochain bloc
	if(_nextBlockView!=NULL)
		_nextBlockView->Empty();

	// son
	g_BeTrisApp->PlaySound(SND_LOST_GAME);
}

/**** remplir le champ ****/
void BeTrisFieldView::FullField(bool sendField)
{
	char	*customField = NULL;

	// champ perso
	customField = g_BeTrisApp->CustomField();

	// proteger
	if(!LockField())
		return;

	// remplir le champ aleatoirement
	// si on a pas un champ perso et que l'on a pas choisi de l'utiliser
	if(g_BeTrisApp->UseCustomField() && customField[0]!='*')
		memcpy(_field,customField,FIELD_WIDTH*FIELD_HEIGHT);
	else
		for(short index=0;index<(FIELD_WIDTH*FIELD_HEIGHT);index++)
			_field[index] = (char)(g_BeTrisApp->RandomNum(5) + 1);

	// deproteger
	UnlockField();

	// redessiner
	_draw_field();

	// envoyer le champ
	if(sendField)
		_send_field(true);
}

/**** protection par semaphore ****/
bool BeTrisFieldView::LockField()
{
	// c'est que pour le champ du joueur
	if(_type!=Large_Field)
		return true;

	// proteger l'acces au champ
	if(acquire_sem(_semId)<B_NO_ERROR)
		return false;

	// ok
	return true;
}

/**** deproteger ****/
void BeTrisFieldView::UnlockField()
{
	// c'est que pour le champ du joueur
	if(_type!=Large_Field)
		return;

	// liberer la semaphore
	release_sem(_semId);
}

// ==================
// fonctions internes
// ==================

/**** dessiner le champ de jeu ****/
void BeTrisFieldView::_draw_field()
{
	_betrisblock	**blocks = NULL;
	char			block;
	char			x,y;
	short			xd,yd;

	// proteger le field
	if(!LockField())
		return;

	// affecter le champ actuel
	memcpy(_drawField,_field,FIELD_WIDTH*FIELD_HEIGHT);

	// si c'est le grand format il faut afficher le bloc en cours
	if(_type==Large_Field && _currentBlock>=0 && _orientBlock>=0 && _orientBlock<g_BeTrisApp->BlockOrientation(_currentBlock))
	{
		// bloc courant
		blocks = g_BeTrisApp->Block();
	
		// est-on dans le rectangle d'update
		for(y=0;y<4;y++)
		{
			for(x=0;x<4;x++)
			{
				// valeur du bloc
				block = blocks[_currentBlock][_orientBlock][x][y];
				if(block>0)
					_drawField[(x+_xblock) + (y+_yblock) * FIELD_WIDTH] = block;
			}
		}
	}

	// dessin des blocs
	for(y=0;y<FIELD_HEIGHT;y++)
	{
		for(x=0;x<FIELD_WIDTH;x++)
		{
			// on doit avoir une modification
			block = _drawField[x + y * FIELD_WIDTH];
			if(block!=_drawCache[x + y * FIELD_WIDTH])
			{
				// destination de l'image
				xd = x * _type;
				yd = y * _type;
			
				// si c'est zero c'est le fond
				if(block==0)
				{
					// ok c'est pas un bloc c'est l'image de fond
					_backgroundRect.left = x*_type + _bitmapPoint.x;
					_backgroundRect.top = y*_type + _bitmapPoint.y;
					_backgroundRect.right = _backgroundRect.left + _type - 1;
					_backgroundRect.bottom = _backgroundRect.top + _type - 1;
					g_BeTrisApp->BlitBitmap(_backgroundRect,_bitmap,xd,yd,_offBitmap);
				}
				else
				{
					// dessiner le bloc correspondant
					_blockRect.left = (block - 1) * _type;
					_blockRect.right = _blockRect.left + _type - 1;
					g_BeTrisApp->BlitBitmap(_blockRect,_bitmap,xd,yd,_offBitmap);
				}
			}
		}
	}

	// et sauvegarde avant le dessin pour avoir
	// un etat du champ pour le prochain affichage
	memcpy(_drawCache,_drawField,FIELD_WIDTH*FIELD_HEIGHT);

	// deproteger
	UnlockField();

	// dessin
	if(Looper()!=NULL)
		_messenger->SendMessage(BETRIS_DRAW_FIELD_MSG);
}

/**** envoyer le champ ****/
void BeTrisFieldView::_send_field(bool entire)
{
	BeTrisPlayerItem	*player = NULL;

	// recuperer notre numero de joueur
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;

	// on envoi notre champ que si on est de la partie
	if(player->EndGameTime()!=0)
		return;

	BMessage			outMessage(BETRIS_NETWORK_MSG);
	char				nbDiff = 0;
	char				maxDiff = (FIELD_WIDTH * FIELD_HEIGHT) / 3;

	// quand on est ici le cache du champ n'est pas identique au champ
	// car on vient par exemple de recevoir un bloc special
	// si toutefois c'est pas le cas, l'utilisation de la variable entire
	// forcera l'envoi quand meme et complet du champ
	
	// placer le joueur (nous) et l'espace pour creer la commande
	// on le fait quel que soit l'envoi (difference ou complet)
	_sendBuffer[0] = 'f';
	_sendBuffer[1] = ' ';
	_sendBuffer[2] = player->Number() + 0x30;
	_sendBuffer[3] = ' ';

	// proteger
	if(!LockField())
		return;

	// essayer de voir les elements qui ont changes
	// si on demande pas le champ complet
	if(!entire)
	{
		short	indexBuffer = 4;
		short	x,y;
		char	block = 0;
		char	previousBlock = -1;
	
		// on parcour le champ et on regarde la difference avec le cache
		for(y=0;(y<FIELD_HEIGHT && nbDiff<maxDiff);y++)
		{
			for(x=0;(x<FIELD_WIDTH && nbDiff<maxDiff);x++)
			{
				// on doit avoir une modification
				block = _field[x + y * FIELD_WIDTH];
				if(block!=_cache[x + y * FIELD_WIDTH])
				{
					// retenir le bloc et les coordonnees
					if(previousBlock!=block)
					{
						_sendBuffer[indexBuffer] = block + '!';
						indexBuffer++;
						previousBlock = block;
					}

					// coordonnees
					_sendBuffer[indexBuffer] = x + '3';
					indexBuffer++;
					_sendBuffer[indexBuffer] = y + '3';
					indexBuffer++;
					
					// une difference de plus
					nbDiff++;
				}
			}
		}

		// caractere de fin de chaine
		_sendBuffer[indexBuffer]='\0';
		
		// doit-on tout de meme tout envoyer
		if(nbDiff>=maxDiff)
			entire = true;
	}

	// doit-on envoyer le champ complet tout de même
	if(entire)
	{
		// copie du champ dans le buffer d'envoi
		// on doit juste convertir le block graphique en block tetrinet
		for(short index=0;index<(FIELD_WIDTH*FIELD_HEIGHT);index++)
			_sendBuffer[index + 4] = g_BeTrisApp->BlockDesf(_field[index]);

		// caractere de fin de chaine
		_sendBuffer[FIELD_WIDTH * FIELD_HEIGHT + 4]='\0';
	}

	// deproteger
	UnlockField();
	
	// envoyer le tout au serveur
	// si on a bien une difference
	if(nbDiff>0 || entire)
	{
		outMessage.AddString(BETRIS_NETWORK_DATAS,_sendBuffer);
		outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
		g_network_messenger.SendMessage(&outMessage);
	}
}

// ================
// fonctions de jeu
// ================

/**** verifier la collision ou non du bloc ****/
char BeTrisFieldView::_block_obstructed(char blockx,char blocky,char orient)
{
	_betrisblock	**blocks = NULL;
	char			x,y;
	char			side = 0;

	// acces a la definition des blocs
	blocks = g_BeTrisApp->Block();

	// verifier si il n'y a pas collision
	for(y=0;y<4;y++)
	{
		for(x=0;x<4;x++)
		{
			// on doit avoir un bloc dans la piece
			if(blocks[_currentBlock][orient][x][y]!=0)
			{
				switch(_obstructed(blockx+x,blocky+y))
				{
				case 0:
					continue;
				case 1:
					return 1;
				case 2:
					side = 2;
				}
			}
		}
	}
	
	// resultat
	return side;
}

/**** test de collision ****/
char BeTrisFieldView::_obstructed(char blockx,char blocky)
{
	// les cote
	if(blockx<0 || blockx>=FIELD_WIDTH)
    	return 2;

	// haut et bas
	if(blocky<0 || blocky>=FIELD_HEIGHT)
		return 1;
	
	// bloc deja present
	if(_field[blockx + blocky * FIELD_WIDTH]!=0)
    	return 1;
    
    // ok pas de collision
    return 0;
}

/**** rotation du bloc ****/
char BeTrisFieldView::_block_rotate(char direction)
{
	// on doit avoir un bloc courant
	if(_currentBlock<0)
		return 0;
	
	char	orient = _orientBlock + direction;
	
	// on boucle  car on depasse
	if(orient>=g_BeTrisApp->BlockOrientation(_currentBlock))
		orient = 0;
	
	// on boucle dans l'autre sens
	if(orient<0)
		orient = g_BeTrisApp->BlockOrientation(_currentBlock) - 1;

	// verifier que l'on peut
	switch(_block_obstructed(_xblock,_yblock,orient))
	{
	// pas possible
	case 1:
		return -1;
		break;
	// on est au bord
	case 2:
		{
			char	shifts[4] = {1,-1,2,-2};
			char	index;
			
			// on va regarder ce que ca fait si on decale
			// de + ou - 1 ou 2 sur les cote
			for(index=0;index<4;index++)
			{
				// est-ce ok avec le decallage
				if(_block_obstructed(_xblock+shifts[index],_yblock,orient)==0)
				{
					// ok deplacer le bloc et sortir
					_xblock += shifts[index];
					_orientBlock = orient;
					return 0;
				}
			}
			
			// si on est ici c'est pas bon
			return -1;
		}
		break;
	// c'est bon
	default:
		_orientBlock = orient;
	}
	
	// ok
	return 0;
}

/**** descente du block ****/
char BeTrisFieldView::_block_down()
{
	// on doit avoir un bloc courant
	if(_currentBlock<0)
		return 0;
    
	// peut on descendre
	if(_block_obstructed(_xblock,_yblock+1,_orientBlock)==0)
		_yblock++;
    else
		return -1;
	
	// ok
	return 0;
}

/**** descendre en bas immediatement ****/
char BeTrisFieldView::_block_drop()
{
	// on doit avoir un bloc courant
	if(_currentBlock<0)
		return 0;

	char	result = 0;

	// on va directement en bas
	while(result==0)
		result = _block_down();

	// jouer le son
	g_BeTrisApp->PlaySound(SND_SPACE_BAR_BLOCK);

	// retourner le resultat
	return result;
}

/**** deplacement du bloc ****/
void BeTrisFieldView::_block_move(char direction)
{
	// on doit avoir un bloc courant
	if(_currentBlock<0)
		return;
	
	// on doit rien avoir en dessous qui bloque
	if(_block_obstructed(_xblock + direction, _yblock,_orientBlock)==0)
		_xblock += direction;
}

/**** ajouter le bloc au champ ****/
void BeTrisFieldView::_block_place()
{
	_betrisblock	**blocks = NULL;
	char			x,y;
	char			block = 0;

	// acces a la definition des blocs
	blocks = g_BeTrisApp->Block();

	// est-on dans le rectangle d'update
	for(y=0;y<4;y++)
	{
		for(x=0;x<4;x++)
		{
			// valeur du bloc
			block = blocks[_currentBlock][_orientBlock][x][y];
			if(block>0)
				_field[(x+_xblock) + (y+_yblock) * FIELD_WIDTH] = block;
		}
	}
}



/**** ajouter une ligne ou plus ****/
void BeTrisFieldView::_add_lines(char count)
{
	// on doit avoir des ligne a ajouter
	if(count<=0)
		return;

	_game_data		*gameData = NULL;
	short			x,y;

	// acces aux donnees de la partie
	gameData = g_BeTrisApp->GameData();

	// ajouter le nombre de ligne passe en parametre
	for(char index=0;index<count;index++)
	{
		// verifier que l'on pas deja la ligne x = 0
		// d'occuper sinon c'est fin de partie !
		for(x=0;x<FIELD_WIDTH;x++)
		{
			if(_field[x]!=0)
			{
				// on perd
				_player_lost();
				
				// quitter
				return;
			}
		}
		
		// tout remonter
        for(y=0;y<FIELD_HEIGHT-1;y++)
            for(x=0;x<FIELD_WIDTH;x++)
                _field[x + y * FIELD_WIDTH] = _field[x + (y + 1) * FIELD_WIDTH];
			
		// ajouter une ligne avec des trous
		switch(gameData->_classicMode)
		{
		// mode tetrinet
		case 0:
			{
				// ajouter la ligne
				// en mode tetrinet le random peut generer des espaces
				// pas enmode classic
				for(x=0;x<FIELD_WIDTH;x++)
					_field[x + (FIELD_HEIGHT-1) * FIELD_WIDTH] = g_BeTrisApp->RandomNum(6);
				
				// 	jouter l'espace
				_field[g_BeTrisApp->RandomNum(FIELD_WIDTH) + (FIELD_HEIGHT-1) * FIELD_WIDTH] = 0;
			}
			break;
		// mode classic
		case 1:
			{
				// ajouter la ligne
				for(x=0;x<FIELD_WIDTH;x++)
					_field[x + (FIELD_HEIGHT-1) * FIELD_WIDTH] = g_BeTrisApp->RandomNum(5) + 1;

				// ajouter un espace
				_field[g_BeTrisApp->RandomNum(FIELD_WIDTH) + (FIELD_HEIGHT-1) * FIELD_WIDTH] = 0;
			}
			break;
		// mode inconnu
		default:
			if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
				printf("ERROR (BeTrisFieldView::_add_lines) : mode unknow\n");
		}
	}
}

/**** enlever une ligne ou plus ****/
char BeTrisFieldView::_remove_lines(char *spblocs)
{
	char	indexsp = 0;
	char	block = -1;
	char	x,y,yremove;
	char	holes = 0;
	char	lines = 0;

	// effacement des lignes entieres
	for(y=0;y<FIELD_HEIGHT;y++)
	{
		// compter les trous
		holes = 0;
		for(x=0;x<FIELD_WIDTH;x++)
			if(_field[x+y*FIELD_WIDTH]==0)
				holes++;
		
		// lignes sans trou
		// il va falloir l'effacer
		if(holes==0)
		{
			// une lignes de plus a enlever
			lines++;
		
			// recuperer les blocs speciaux
			// si on veux le faire
			if(spblocs!=NULL)
			{
				for(x=0;x<FIELD_WIDTH;x++)
				{
					block = _field[x+y*FIELD_WIDTH];
					if(block>5)
					{
						// recuperer dans notre buffer les blocs speciaux
						spblocs[indexsp] = block;
						indexsp++;
					}
				}
			}
			
			// deplacer vers le bas l reste du champ
			for(yremove=y-1;yremove>=0;yremove--)
				for(x=0;x<FIELD_WIDTH;x++)
					_field[x+(yremove+1)*FIELD_WIDTH] = _field[x+yremove*FIELD_WIDTH];

			// effacer la ligne du haut
			memset(_field,0,12);
		}
	}

	// fermer le buffer des blocs
	if(spblocs!=NULL)
	{
		// fin de buffer
		spblocs[indexsp] = '\0';
	}
	
	
	// retourner le nombre de lignes enleve
	return lines;
}

/**** deplacement de ligne ****/
void BeTrisFieldView::_shift_line(short line,char d)
{
	// pour d = 0 on ne fait rien
	if(d==0)
		return;

    short	index;
    
    // vers la droite
	if(d>0)
	{
		for(index=FIELD_WIDTH-1;index>=d;index--)
			_field[index + line * FIELD_WIDTH] = _field[(index - d) + line * FIELD_WIDTH];
		
		for(;index>=0;index--)
			_field[index + line * FIELD_WIDTH] = 0;
	}

	// vers la gauche
	if(d<0)
	{
		for(index=0;index<(FIELD_WIDTH+d);index++)
			_field[index + line * FIELD_WIDTH] = _field[(index - d) + line * FIELD_WIDTH];
		
		for(;index<FIELD_WIDTH;index++)
			_field[index + line * FIELD_WIDTH] = 0;
	}
}

/**** ajouter les blocs speciaux ****/
void BeTrisFieldView::_add_special(char lines)
{
	_game_data		*gameData = NULL;
	short			slCount = 0;
	short			count = 0;
	char			s = 0;
	char			specialBlock = 0;
	char			number = 0;
	short			j = 0;
	short			x = 0,y = 0;
	short			nonSpecialCount = 0;
	
	// donnees de la partie
	gameData = g_BeTrisApp->GameData();
	slCount = gameData->_slines / gameData->_specialLines;
	gameData->_slines %= gameData->_specialLines;

	// ajout des blocs
	count = gameData->_specialCount * slCount;
	for(short index=0;index<count;index++)
	{
		// recuperer un bloc special
		// attention a la frequence de ce bloc
		s = 0;
		number = g_BeTrisApp->RandomNum(100);
		while(number>=gameData->_specialFreq[s])
			s++;
		
		// ok definition du bloc sb est notre bloc
		// qui sera sur le champ, on va compter le nombre de bloc non speciaux
		// sur le champ de jeu
		specialBlock = 6 + s;
		nonSpecialCount = 0;
		for(y=0;y<FIELD_HEIGHT;y++)
			for(x=0;x<FIELD_WIDTH;x++)
				if(_field[x+y*FIELD_WIDTH]>0 && _field[x+y*FIELD_WIDTH]<6)
					nonSpecialCount++;
	
		// a-t-on des blocs
		if(nonSpecialCount==0)
		{
			// voila ce qui est dit dans gtetrinet :
            // i *think* this is how it works in the original -
            // blocks are not dropped on existing specials,
            // and it tries again to find another spot...
            // this is because... when a large number of
            // blocks are dropped, usually all columns get 1 block
            // but sometimes a column or two doesnt get a block
			for(j=0;j<20;j++)
			{
				number = g_BeTrisApp->RandomNum(FIELD_WIDTH);
				for(y=0;y<FIELD_HEIGHT;y++)
					if(_field[number + y*FIELD_WIDTH]>0)
						break;
						
				if(y==FIELD_HEIGHT || _field[number + y*FIELD_WIDTH]<6)
					break;
			}
			// quitter si on est au bout
			if(j==20)
				return;
		
			// sinon affecter le bloc
			y--;
			_field[number + y*FIELD_WIDTH] = specialBlock;
		}
		else
		{
			// choisir un endroit au hazard
			number = g_BeTrisApp->RandomNum(nonSpecialCount);
			for(y=0;(y<FIELD_HEIGHT && number>=0);y++)
			{
				for(x=0;(x<FIELD_WIDTH && number>=0);x++)
				{
					if(_field[x+y*FIELD_WIDTH]>0 && _field[x+y*FIELD_WIDTH]<6)
					{
						// on affecte le bloc
						if(number==0)
							_field[x+y*FIELD_WIDTH] = specialBlock;
							
						// passer au suivant
						number--;
					}
				}
			}
		}
	}
}

/**** effacer un block special ****/
void BeTrisFieldView::_remove_special()
{
	// on va demander d'enlever un block special de notre liste
	BMessage	removeSpBlock(BETRIS_SPBLOCK_MSG);
		
	// puis l'envoyer a l'application
	removeSpBlock.AddBool(BETRIS_SPBLOCK_REMOVE,true);
	BMessenger(be_app).SendMessage(&removeSpBlock);
}

/**** solidify ****/
bool BeTrisFieldView::_solidify()
{
	// on doit avoir un bloc courant
	if(_currentBlock<0)
		return false;

	bool	playerLost = false;
	char	lines = 0;

	// bloquer
	_lockTimer = true;

	// proteger
	if(!LockField())
		return false;

	// sauver la valeur du champ actuel dans le cache
	memcpy(_cache,_field,FIELD_WIDTH*FIELD_HEIGHT);

	// verifier si il obstruction
	if(_block_obstructed(_xblock,_yblock,_orientBlock)!=0)
	{
		for(_yblock--;_yblock>=0;_yblock--)
		{
			if(!_block_obstructed(_xblock,_yblock,_orientBlock)!=0)
			{
				_block_place();
				break;
			}
		}
		
		// perdu !!
		if(_yblock<0)
			playerLost = true;
	}
	else
		_block_place();

	// deproteger
	UnlockField();

	// pas de bloc
	_currentBlock = -1;

	// a-t-on perdu
	if(!playerLost)
	{
		_game_data		*gameData = NULL;
		char			sndToPlay = 0;
		float			diffTime = 0;

		// donnees de jeu
		gameData = g_BeTrisApp->GameData();

		// calcul du nombre de piece par minutes
		_nbPieceSolidify++;
		diffTime = (time(0) - _startGameTime);
		if(diffTime>=60)
		{
			// on calcul pour la minutes qui vient de passer
			gameData->_ppm = (int)(_nbPieceSolidify / diffTime * 60);
			
			// puis on reinitialise les valeurs
			_nbPieceSolidify = 0;
			_startGameTime = time(0);
		}
		
		// proteger
		if(LockField())
		{
			// enlever les lignes qu'il faut
			lines = _remove_lines(_spblocs);
			if(lines>0)
			{
				// mettre a jour le compte des lignes
				gameData->_lineCount += lines;
				gameData->_slines += lines;
				gameData->_llines += lines;

				// rafraichir l'affichage du level et nb lignes
				BMessenger(be_app).SendMessage(BETRIS_REFRESH_MSG);

				// ajouter les blocs speciaux au champ de jeu
				_add_special(lines);

				// on recupere des lignes
				sndToPlay = SND_MAKE_LINE;
				
				// si on est en mode classic
				// en fonction du nombre de ligne
				// on ajoutera aux autres des lignes
				if(gameData->_classicMode==B_CONTROL_ON && lines>1)
				{
					BeTrisPlayerItem	*player = NULL;

					// notre joueur local
					player = g_BeTrisApp->ActivatedPlayer();
					if(player!=NULL)
					{
						// envoyer la commande a notre serveur qui va l'envoyer aux autres joueurs
						// en interne aussi pour info
						BMessage			outMessage(BETRIS_NETWORK_MSG);
						BMessage			inMessage(BETRIS_NETWORK_MSG);
						BString				netBuffer;
				
						// construire la commande
						netBuffer = g_BeTrisApp->Command(CMD_SB);
						netBuffer << " 0 ";

						// en fonction du nombre de ligne
						// on en ajoutera aux autres joueurs
						switch(lines)
						{
						case 2:
							netBuffer << SP_ADD1LINE;
							break;
						case 3:
							netBuffer << SP_ADD2LINE;
							break;
						// le son a faire est celui d'un tetris
						default:
							netBuffer << SP_ADD4LINE;
							sndToPlay = SND_MAKE_TETRIS;
						}
					
						// fin de la commande
						netBuffer << " ";
						netBuffer << (int32)player->Number();

						// envoyer le message au reseau
						outMessage.AddString(BETRIS_NETWORK_DATAS,netBuffer);
						outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
						if(g_network_messenger.SendMessage(&outMessage)!=B_OK)
							if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
								printf("ERROR (BeTrisFieldView::_solidify) : network send error\n");

						// ok envoyer en interne la commande
						// car elle est pas renvoye par le serveur
						netBuffer << (char)0xFF;
						inMessage.AddString(BETRIS_NETWORK_DATAS,netBuffer);
						inMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
						if(BMessenger(be_app).SendMessage(&inMessage)!=B_OK)
							if(BETRIS_DEBUG_MODE & BETRIS_DEBUG_MODE_TRACE)
								printf("ERROR (BeTrisFieldView::_solidify) : application send error\n");
					}
				}
			}
	
			// si on a recuperer des blocs speciaux
			// on les envoi a l'application qui va les ajouter
			// a notre liste
			if(_spblocs[0]!='\0')
			{
				BMessage	spBlock(BETRIS_SPBLOCK_MSG);
				short		indexBuffer = 0;

				// vider le buffer de copie
				memset(_spblocsBuffer,0,BLOCK_SP_BUFFER);
		
				// multiplier les blocs
				for(char indexCopie=0;indexCopie<lines;indexCopie++)
					for(char indexSp=0;_spblocs[indexSp]!='\0';indexSp++)
						_spblocsBuffer[indexBuffer++] = _spblocs[indexSp];

				// placer les blocs dans le message
				// puis l'envoyer a l'application
				spBlock.AddString(BETRIS_SPBLOCK_DATAS,_spblocsBuffer);
				BMessenger(be_app).SendMessage(&spBlock);
			
				// jouer le son de recuperation des block speciaux
				// sauf si on a fait un tetris
				if(sndToPlay!=SND_MAKE_TETRIS)
					sndToPlay = SND_GET_SPECIAL_BLOCK;
			}
		
			// deproteger
			UnlockField();

			// jouer le son si il y en a un
			if(sndToPlay>0)
				g_BeTrisApp->PlaySound(sndToPlay);

			// mettre a jour le niveau
        	if(gameData->_llines>=gameData->_linesperLevel)
        	{
				BeTrisPlayerItem	*player = NULL;

				// notre joueur local
				player = g_BeTrisApp->ActivatedPlayer();
				if(player!=NULL)
				{
					BMessage			outMessage(BETRIS_NETWORK_MSG);
    	    		BString				string;
					short				oldLevel = 0;
        	
        			// augmenter le level
            		oldLevel = player->Level();
            		while(gameData->_llines>=gameData->_linesperLevel)
            		{
                		player->SetLevel(player->Level() + gameData->_levelInc);
	                	gameData->_llines -= gameData->_linesperLevel;
    	        	}
    				
    				// on doit avoir un level qui a change
    				if(oldLevel!=player->Level())
    				{
    					// construire le message
    					string << g_BeTrisApp->Command(CMD_LVL);
    					string << " ";
    					string << (int32)player->Number();
    					string << " ";
    					string << player->Level();

						// informer le serveur
						outMessage.AddString(BETRIS_NETWORK_DATAS,string);
						outMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
						g_network_messenger.SendMessage(&outMessage);
					
						// mettre a jour le timing de descente des pieces
						// en fonction du niveau du joueur uniquement
						if(player->Level()<=100)
							gameData->_timing = (1005 - player->Level() * 10) * 1000;
						else
							gameData->_timing = 5000;
						
						// mettre a jour le niveau moyen
						if(_fieldsView!=NULL)
							_fieldsView->UpdateAverageLevel();
							
						// affichage
						BMessage	refreshDisplay(BETRIS_REFRESH_MSG);
						be_app_messenger.SendMessage(&refreshDisplay);
					}
				}
        	}
		}

		// envoyer le champ
		_send_field();
		
		// initialiser le compteur d'attente
		// pour la prochain piece
		_waitNextBloc = true;
		_waitCount = 0;

		// redesssiner
		_draw_field();

		// debloquer le compteur
		_lockTimer = false;
	}
	else
	{		
		// on a perdu
		_player_lost();
		
		// on retourne le fait que l'on a perdu
		return true;
	}
	
	return false;
}

/**** envoi du message comme quoi on a perdu ****/
void BeTrisFieldView::_player_lost()
{
	BeTrisPlayerItem	*player = NULL;
	_game_data			*gameData = NULL;
	BMessage			lostMessage(BETRIS_NETWORK_MSG);
	BString				command;

	// on doit avoir un joueur local
	player = g_BeTrisApp->ActivatedPlayer();
	if(player==NULL)
		return;

	// donnees de jeu
	gameData = g_BeTrisApp->GameData();

	// en tout cas on sortira du thread
	// et bloquer la descente des pieces
	_exitThread = true;
	_lockTimer = true;

	// stoper l'apparition des pieces
	// la partie est perdue pour nous
	gameData->_timing = 0;
	_waitNextBloc = false;
	_currentBlock = -1;
	_waitCount = 0;
		
	// commande
	command = g_BeTrisApp->Command(CMD_PLAYERLOST);
	command << " ";
	command << (int32)(player->Number());
	command << (char)0xFF;

	// envoyer en interne la commande comme quoi on a perdu
	lostMessage.AddString(BETRIS_NETWORK_DATAS,command);
	lostMessage.AddInt8(BETRIS_NETWORK_SLOT,0);
	be_app_messenger.SendMessage(&lostMessage);
}

/**** prochain bloc ****/
void BeTrisFieldView::_next_block()
{
	// on se sert de la vue NextBlock pour ca
	if(_nextBlockView==NULL)
		return;

	// nouveau bloc
	_nextBlockView->NextBlock(_currentBlock,_orientBlock);

	// ok donner le prochain block
	_xblock = 4;
	_yblock = 0;	

	// debloquer l'attente du prochain bloc
	_waitNextBloc = false;
}

// ===============
// thread du timer
// ===============

/**** arreter le thread ****/
void BeTrisFieldView::_stop_timer_thread()
{
	// le thread existe que pour le grans champ
	if(_type!=Large_Field)
		return;

	status_t		returnState;
	status_t		state;

	// on quitte le thread
	_exitThread = true;
	state = wait_for_thread(_timerThreadID,&returnState);
}

/**** stub du thread de timing de la partie ****/
int32 BeTrisFieldView::_timer_thread(void *data)
{	return ((BeTrisFieldView *)data)->_timer();	}

/*** thread du timing ****/
int32 BeTrisFieldView::_timer()
{
	CPreferenceFile		*prefs = NULL;
	_game_data			*gameData = NULL;
	bigtime_t			systemtime;
	bigtime_t			waitingtime = 0;
	int32				tetrifast = B_CONTROL_OFF;
	
	// donnees de la partie
	gameData = g_BeTrisApp->GameData();
	prefs = g_BeTrisApp->Preferences();
	
	// voir si on est en tetrifast ou non
	if(prefs!=NULL)
		tetrifast = prefs->GetInt32(B_CONTROL_OFF,"tetrinet-type");
	
	// boucle du timer
	while(!_exitThread)
	{
		// temps systeme
		systemtime = system_time();

		// appel du timer
		if(!_lockTimer && gameData->_timing>0)
		{
			// on est en attente du prochain bloc
			if(!_waitNextBloc)
			{
				// verifier si le bloc peut descendre et redessiner
				// si non on va perdre de toute facon
				if(_block_down())
					_solidify();
				else
					_draw_field();
			}
			else
			{
				// fixer le prochain bloc
				_next_block();
				_draw_field();
			}
		}

		// si c'est le block suivant c'est de toute facon une seconde
		if(!_waitNextBloc)
		{
			// calcul du temps d'attente pour la chute de la piece
			// si le timing est a zero la partie n'est pas demarree
			if(gameData->_timing>0)
				waitingtime = gameData->_timing;
			else
				waitingtime = 1000000;
		}
		else
		{
			// attendre 1 seconde
			waitingtime = 0;
			if(tetrifast==B_CONTROL_OFF)
			{
				// reaffacter le temps systeme de reference
				// pour attendre une pleine seconde avant d'aller afficher le prochain bloc
				systemtime = system_time();
				waitingtime = 1000000;
			}
		}
		 		
		// attendre
		waitingtime -= (system_time() - systemtime);
		if(waitingtime>0)
			snooze(waitingtime);
	}

	// on quitte bien
	return B_OK;
}
