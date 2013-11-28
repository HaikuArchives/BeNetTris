#ifndef _BETRISWINDOW_H
#define _BETRISWINDOW_H

#include <TabView.h>
#include <View.h>
#include <Window.h>
#include <Messenger.h>

// constants for messages
const	uint32	U_WIN_APP_MSG	= 'Uwam';

class BeTrisAboutView;
class BeTrisFieldsView;
class BeTrisPartyLineView;
class BeTrisWinnerView;
class BeTrisSettingView;
class BeTrisNetworkView;
class BeTrisBlockFreqView;


class BeTrisWindow : public BWindow
{
public:
	// laisser acces au vue
	BeTrisAboutView			*_aboutView;
	BeTrisFieldsView		*_fieldsView;
	BeTrisPartyLineView		*_partyLineView;
	BeTrisWinnerView		*_winnerView;
	BeTrisSettingView		*_settingView;
	BeTrisNetworkView		*_networkView;
	BeTrisBlockFreqView		*_blockView;

	BeTrisWindow(BRect);
	virtual	~BeTrisWindow();
	
	virtual void		MessageReceived(BMessage *message);	
	virtual bool		QuitRequested();
	
	inline	BTabView* BeTrisTabView() { return _pBeTrisTabView; }
	

protected:
	BView					*_pSupportView;
	BTabView				*_pBeTrisTabView;
};

// Acces au looper directement
extern 	BeTrisWindow		*g_BeTrisWindow;

#endif