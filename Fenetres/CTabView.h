/***********************************************/
/* Software Created By CKJ Vincent Cedric      */
/* This code is the property of Vincent Cedric */
/* You can't copy or use this code without     */
/* according of the author.					   */
/***********************************************/
#ifndef _CTABVIEW_H
#define _CTABVIEW_H
/*******************************************/
/* classe CTabView speciale pour le resize */
/*******************************************/

#include <TabView.h>

// constante
#define		DECAL_SIZE		16

class CTabView : public BTabView
{
// fonctions
public:
	CTabView(BRect frame, const char *name,button_width width = B_WIDTH_AS_USUAL, uint32 resizingMode = B_FOLLOW_ALL, uint32 flags = B_FULL_UPDATE_ON_RESIZE | B_WILL_DRAW | B_NAVIGABLE_JUMP | B_FRAME_EVENTS | B_NAVIGABLE);
	virtual ~CTabView();
	
	// action utilisateur
	virtual	void	Select(int32 tab);

	// dessin
	virtual	BRect		DrawTabs();
	virtual BRect		TabFrame(int32 tabIndex) const;
	virtual	void		DrawBox(BRect selTabRect);
	virtual	void		DrawLabel(int32 tab_index);
		
protected:
			void	DrawRectOfPoint(BRect frame);
};

#endif