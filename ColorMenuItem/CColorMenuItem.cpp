#include <Font.h>
#include "CColorMenuItem.h"

CColorMenuItem::CColorMenuItem(BString label,BMessage *message,char shortcut, uint32 modifiers)
: BMenuItem(label.String(),message,shortcut,modifiers)
{
	SetRectColor(0,0,0);
	_colorID = 0;
}

CColorMenuItem::CColorMenuItem(rgb_color menucolor,char colorID,BString label,BMessage *message,char shortcut, uint32 modifiers)
: BMenuItem(label.String(),message,shortcut,modifiers)
{
	// initialiser
	SetRectColor(menucolor);
	_colorID = colorID;
}

CColorMenuItem::~CColorMenuItem(void)
{
}

void CColorMenuItem::GetContentSize(float *heigth,float *weight)
{
	BMenuItem::GetContentSize(heigth,weight);
	weight += int(InfoFontMenu.ascent *1.25);
	MenuAttached = Menu();
	MenuAttached->GetFontHeight(&InfoFontMenu);
}

void CColorMenuItem::Draw(void)
{
	rgb_color	OldColor;
	rgb_color	OldLowColor;
	rgb_color	color;
	
	// on doit dessiner au moins la marque
	BMenuItem::Draw();
	
	ItemFrame = Frame();
	ItemFrame.left += int(InfoFontMenu.ascent *1.25);
	ItemFrame.top += int(InfoFontMenu.descent);
	ItemFrame.right = ItemFrame.left + int(InfoFontMenu.ascent);
	ItemFrame.bottom = ItemFrame.top + int(InfoFontMenu.ascent);

	OldColor = MenuAttached->HighColor();
	OldLowColor = MenuAttached->LowColor();

	MenuAttached->SetHighColor(156,156,156,255);
	if(IsSelected())
	{
		MenuAttached->FillRect(Frame());
		MenuAttached->SetLowColor(MenuAttached->HighColor());
	}
	MenuAttached->SetHighColor(OldColor);
	DrawContent();
	
	MenuAttached->SetHighColor(RectColorItem);
	MenuAttached->FillRect(ItemFrame);

	MenuAttached->BeginLineArray(4);
	MenuAttached->SetHighColor(220,220,220,255);
	color = MenuAttached->HighColor();
	MenuAttached->AddLine(BPoint(ItemFrame.left,ItemFrame.top),BPoint(ItemFrame.right,ItemFrame.top),color);
	MenuAttached->AddLine(BPoint(ItemFrame.left,ItemFrame.top),BPoint(ItemFrame.left,ItemFrame.bottom),color);
	MenuAttached->SetHighColor(110,110,110,255);
	color = MenuAttached->HighColor();
	MenuAttached->AddLine(BPoint(ItemFrame.right,ItemFrame.top),BPoint(ItemFrame.right,ItemFrame.bottom),color);
	MenuAttached->AddLine(BPoint(ItemFrame.left,ItemFrame.bottom),BPoint(ItemFrame.right,ItemFrame.bottom),color);
	MenuAttached->EndLineArray();
	
	MenuAttached->SetHighColor(OldColor);
	MenuAttached->SetLowColor(OldLowColor);
}

void CColorMenuItem::DrawContent(void)
{
	BPoint	coordcontent;
	
	coordcontent = ContentLocation();
	coordcontent.x += (InfoFontMenu.ascent *1.50);
	coordcontent.y += InfoFontMenu.ascent;
	MenuAttached->DrawString(Label(),coordcontent);
}

void CColorMenuItem::SetRectColor(rgb_color color)
{
	RectColorItem = color;
}

void CColorMenuItem::SetRectColor(int red,int green,int blue,int alpha)
{
	RectColorItem.red = red;
	RectColorItem.green = green;
	RectColorItem.blue = blue;
	RectColorItem.alpha = alpha;
}
