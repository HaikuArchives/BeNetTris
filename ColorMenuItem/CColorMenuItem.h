/*****************************************************************/
/* The class CColorMenuItem is an extention of class BMenuItem   */
/* It's for graphical application, you can see a color rectangle */
/* in a Item of menu                                             */
/*****************************************************************/

#ifndef _CCOLORMENUITEM_H
#define _CCOLORMENUITEM_H

#include <Menu.h>
#include <MenuItem.h>
#include <Message.h>
#include <String.h>

class CColorMenuItem : public BMenuItem
{
public:
		CColorMenuItem(BString,BMessage*,char = 0,uint32 = 0);
		CColorMenuItem(rgb_color,char colorID,BString,BMessage*,char = 0,uint32 = 0);
		~CColorMenuItem(void);
		void SetRectColor(rgb_color);
		void SetRectColor(int,int,int,int = 255);
		
		inline	rgb_color	RectColor() 	{ return RectColorItem; }
		inline	char		ColorId()		{ return _colorID; }

private:
	rgb_color	RectColorItem;
	font_height InfoFontMenu;
	BMenu		*MenuAttached;
	BRect		ItemFrame;
	char		_colorID;

		virtual void GetContentSize(float *,float *);
		virtual void Draw(void);
		virtual void DrawContent(void);
};

#endif