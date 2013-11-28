/*******************************************/
/* vue pour afficher le text sur le joueur */
/*******************************************/
#ifndef BeTrisPlayerView_H
#define BeTrisPlayerView_H

#include "BeTrisPlayerItem.h"
#include <TextView.h>
#include <String.h>

class BeTrisPlayerView : public BView
{
public:
	enum	field_type{Large_Field = 16,Small_Field = 8};
	// metodes de la classe BView
	BeTrisPlayerView(BPoint ,char * ,field_type = Large_Field);
	virtual			~BeTrisPlayerView();
	virtual	void	Draw(BRect updaterect); 
			void	Update(BeTrisPlayerItem *player);
	
	// metodes ajoutées
	BRect SizeOfField(BPoint,field_type);

protected:
	BString		_label;				// texte affiche
	BPoint		_labelPosition;		// emplacement du texte
	bool		_playerExist;		// existe-il un joueur pour cette vue
};

#endif