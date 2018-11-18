/***********************************************/
/* Software Created By CKJ Vincent Cedric      */
/* This code is the property of Vincent Cedric */
/* You can't copy or use this code without     */
/* according of the author.					   */
/***********************************************/
#include "CTabView.h"

#include <TabView.h>

// debug
//#include <iostream.h>

/**** constructeur ****/
CTabView::CTabView(BRect frame, const char *name,button_width width, uint32 resizingMode, uint32 flags)
: BTabView(frame,name,width,resizingMode,flags)
{
}

/**** destructeur ****/
CTabView::~CTabView()
{
}
	
/**** selection de l'onglet ****/
void CTabView::Select(int32	tab)
{
	BTab	*selectedTab = NULL;
	BView	*selected = NULL;
	BView	*newSelected = NULL;
	BRect	size;

	// recuperer les vues des 2 onglets
	selectedTab = TabAt(0);
	if(selectedTab==NULL)
		return;
	selected = selectedTab->View();
	newSelected = TabAt(tab)->View();
	
	// cacher la vue de l'onglet que l'on selectionne
	newSelected->Hide();
	BTabView::Select(tab);

	// recalculer la taille par rapport a l'ancien
	// puis reafficher le tout
	size = selected->Bounds();
	newSelected->ResizeTo(size.Width(),size.Height());	
	newSelected->Show();
}

/**** dessiner les onglets ****/
BRect CTabView::DrawTabs()
{
	BRect	tabFrame;
	int32	nbTabs;
	
	nbTabs = CountTabs();
	for(int i=0;i<nbTabs;i++)
	{
		// recuperer la taille de l'onglet
		tabFrame = TabFrame(i);
		
		if(i==Selection())
		{
			SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_MAX_TINT));
			StrokeLine(BPoint(tabFrame.left,1),BPoint(tabFrame.left,tabFrame.bottom));
			StrokeLine(BPoint(tabFrame.left+1,0),BPoint(tabFrame.right-1,0));
		}
		else
		{
			SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_1_TINT));

		}		
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_4_TINT));
		StrokeLine(BPoint(tabFrame.right,1),BPoint(tabFrame.right,tabFrame.bottom));

		// remplir
		SetHighColor(192,192,192);
		tabFrame.InsetBy(1,1);
		if(i==Selection())
			SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));		
		FillRect(tabFrame);
		
		// dessiner le label
		DrawLabel(i);
		
		// dessiner le petit rectangle
		DrawRectOfPoint(BRect(tabFrame.left,tabFrame.top+1,tabFrame.left+4,tabFrame.bottom-1));
	}
	
	return TabFrame(Selection());
}

/*** donner la taille d'un onglet ****/
BRect CTabView::TabFrame(int32 tab_index) const
{
	BRect	bounds;
	BRect	tabSize;
	float	labelSize;
	float	width;
	float	left;
	BTab	*tab;
	
	// taille du label
	bounds = Bounds();
	labelSize = 0;
	tab = TabAt(tab_index);
	if(tab!=NULL && ( TabWidth()==B_WIDTH_FROM_LABEL || TabWidth()==B_WIDTH_FROM_WIDEST ))
	{
		labelSize = StringWidth(tab->Label());

		// taille basé sur le label et un peu d'espace
		width = labelSize + DECAL_SIZE;
	}
	else
		width = bounds.right / CountTabs();
	
	// fixer la taille d'un onglet
	left = 0;
	if(tab_index!=0)
		left = (TabFrame(tab_index-1)).right;

	tabSize.left = left;
	tabSize.right = tabSize.left + width;
	tabSize.top = 0;
	tabSize.bottom = TabHeight();
	
	// retourner la taille
	return tabSize;
}

/**** dessiner la vue container ****/
void CTabView::DrawBox(BRect selTabRect)
{
	// on doit avoir un tab de selectionné
	if(Selection()<0)
		return;

	BRect	bounds;
	BRect	selectedRect;

	bounds = Bounds();
	selectedRect = TabFrame(Selection());
	//255,255,255
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_LIGHTEN_MAX_TINT));
	StrokeLine(BPoint(0,selTabRect.bottom+1),BPoint(0,bounds.bottom-1));
	StrokeLine(BPoint(1,selTabRect.bottom+1),BPoint(bounds.right-1,selTabRect.bottom+1));
	
	// 96,96,96
	SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_4_TINT));
	StrokeLine(BPoint(bounds.right,selTabRect.bottom+1),BPoint(bounds.right,bounds.bottom));	
	StrokeLine(BPoint(1,bounds.bottom),BPoint(bounds.right,bounds.bottom));

	// 216,216,216
	SetHighColor(ui_color(B_PANEL_BACKGROUND_COLOR));
	StrokeLine(BPoint(selectedRect.left+1,selTabRect.bottom+1),BPoint(selectedRect.right-1,selTabRect.bottom+1));		
}

/**** dessiner le label ****/
void CTabView::DrawLabel(int32 tab_index)
{
	BTab	*tab = NULL;
	
	tab = TabAt(tab_index);
	if(tab!=NULL)
	{
		BFont	viewFont;
		BRect	tabFrame;
		float	startHString;
		float	startWString;
		
		GetFont(&viewFont);
		startHString = (TabHeight() - viewFont.Size()) / 2 + viewFont.Size() -1;
		tabFrame = TabFrame(tab_index);
		SetLowColor(HighColor());
		SetHighColor(tint_color(ui_color(B_PANEL_BACKGROUND_COLOR),B_DARKEN_MAX_TINT));
		startWString = tabFrame.left + 6 + ((tabFrame.Width() - 6 - StringWidth(tab->Label())) / 2);
		DrawString(tab->Label(),BPoint(startWString,startHString));
	}
}

/**** dessin des petits points ****/
void CTabView::DrawRectOfPoint(BRect frame)
{	
	SetHighColor(255,255,255);
	StrokeLine(BPoint(frame.left+1,frame.top),BPoint(frame.left+1,frame.bottom-2),B_MIXED_COLORS);
	SetHighColor(100,100,100);
	StrokeLine(BPoint(frame.left+2,frame.top+1),BPoint(frame.left+2,frame.bottom-1),B_MIXED_COLORS);
	SetHighColor(255,255,255);
	StrokeLine(BPoint(frame.left+3,frame.top),BPoint(frame.left+3,frame.bottom-2),B_MIXED_COLORS);
	SetHighColor(100,100,100);
	StrokeLine(BPoint(frame.left+4,frame.top+1),BPoint(frame.left+4,frame.bottom-1),B_MIXED_COLORS);
}
