#ifndef _BETRISABOUTVIEW_H
#define _BETRISABOUTVIEW_H

#include <View.h>
#include <String.h>

class BTextView;
class BBitmap;

class BeTrisAboutView : public BView
{
public:
	BeTrisAboutView(BRect , const char *);
	virtual ~BeTrisAboutView();
	virtual	void	Draw(BRect updaterect);
	virtual void	MouseDown(BPoint point);

protected:
	BBitmap		*_logo;
	BBitmap		*_paypal;
	BBitmap		*_icon;
	BString		_url;
	int32		_currentScroll;
	BTextView	*_credits;
	
			BBitmap		*_get_bitmap(const char *name);		// recuperer une image des ressources
	
};

#endif