#include "BeTrisApp.h"
#include "BeTrisAboutView.h"
#include "BeTrisConstants.h"

#include <TextView.h>
#include <Application.h>
#include <AppFileInfo.h>
#include <Roster.h>
#include <Bitmap.h>
#include <File.h>
#include <Resources.h>

/**** constructeur ****/
BeTrisAboutView::BeTrisAboutView(BRect frame, const char *name)
: BView(frame, name, B_FOLLOW_ALL_SIDES, B_WILL_DRAW)
{
	BFont			plainfont(be_plain_font);
	BFont			boldfont(be_bold_font);
	BString			creditstext;
	BFile 			file;
	BAppFileInfo	fileinfo;
	app_info		info;

	// couleur standard de la vue
	SetViewColor(U_GRIS_STANDARD);

	// initialiser
	_logo = NULL;
	_paypal = NULL;
	_icon = NULL;
	_currentScroll = 0;
	_url = "https://www.paypal.com/xclick/business=cedric-achats-online%40orange.fr&item_name=Don+for+my+BeOS+developpement...&item_number=DonBeOSCKJ&no_note=1&tax=0&currency_code=EUR";

	// taille de la police
	plainfont.SetSize(24);
	SetFont(&plainfont);
	
	// icon
	_icon = new BBitmap(BRect(0,0,31,31 ), B_CMAP8);
	be_app->GetAppInfo(&info);
	file.SetTo(&(info.ref), B_READ_ONLY);
	fileinfo.SetTo(&file);
	fileinfo.GetIcon(_icon, B_LARGE_ICON);
	
	// recuperer les images
	_logo = _get_bitmap("BeNetTris:CKJLogo");
	_paypal = _get_bitmap("BeNetTris:PayPal");
	
	creditstext = BETRIS_VERSION;
	creditstext << " (Tetrinet for BeOS)\n";
	creditstext <<
	"Copyright (c) 2000-2008, CKJ\n"
	"\n"
	"Based on Tetrinet for Windows and gTetrinet for Linux\n"
	"Compatible with Tetrinet 1.13 and 1.14,\n"
	"TetriFaster 1.13 and TetriFaster 1.14\n"
	"Developed by :\n"
	"CKJ - Cedric Vincent\n"
	"\n"
	"Support BeOS Coders, Thanks\n"
	"Click into logo or paypal bitmap to make Donation)\n"
	"(Use Paypal Link into Firefox (not work in Net+)\n"
	"\n"
	"About tetrinet gameplay,strategy and more :\n"
	"ENG : http://tetrinet.org/tnet/other/strategy/index.html\n"
	"FR  : http://tetrinet.lfjr.net/accueil.php?page=tetrinet&sspage=presentation\n";

	boldfont.SetSize(11);
	_credits = new BTextView(BRect(105,240,520,525),"credits", BRect(0,0,415,225), B_FOLLOW_ALL, B_WILL_DRAW); 
	_credits->SetText(creditstext.String(),creditstext.Length());
	_credits->MakeSelectable(true);
	_credits->MakeEditable(false);
	_credits->SetAlignment(B_ALIGN_CENTER);
	_credits->SetViewColor(U_GRIS_STANDARD);
	_credits->SetFontAndColor(0,1000,&boldfont,B_FONT_ALL,&U_GRIS_TRES_FONCE);
	
	AddChild(_credits);
}

/**** destructeur ****/
BeTrisAboutView::~BeTrisAboutView()
{
	// icon
	delete _icon;

	// liberer la memoire des images
	if(_logo!=NULL)
		delete _logo;
	if(_paypal!=NULL)
		delete _paypal;
}

/**** dessin ****/
void BeTrisAboutView::Draw(BRect updaterect)
{
	// cote gris fonce
	SetHighColor(U_GRIS_FONCE);
	FillRect(BRect(0,0,30,(Bounds()).bottom));
	
	// icon
	SetLowColor(ViewColor());
	SetDrawingMode(B_OP_OVER);
	DrawBitmap(_icon, BPoint(5,10));

	// texte
	SetHighColor(U_GRIS_TRES_FONCE);
	DrawString("BeNetTris",BPoint(48,38));

	// logo CKJ
	if(_logo!=NULL)
	{
		// cadre
		SetHighColor(U_GRIS_FONCE);
		StrokeLine(BPoint(140,60),BPoint(490,60));
		StrokeLine(BPoint(140,60),BPoint(140,226));
		SetHighColor(U_COULEUR_BLANC);
		StrokeLine(BPoint(140,227),BPoint(491,227));
		StrokeLine(BPoint(491,60),BPoint(491,226));

		// image
		DrawBitmap(_logo,BPoint(141,61));
	}
	
	// logo paypal
	if(_paypal!=NULL)
		DrawBitmap(_paypal,BPoint(520,10));
}

/**** clique souris ****/
void BeTrisAboutView::MouseDown(BPoint point)
{
	// si on clique sur l'image ou sur le logo paypal
	// ouvrir l'url paypal de mon compte
	if(BRect(141,81,490,246).Contains(point) || BRect(520,10,600,41).Contains(point))
	{
		char *buffer = NULL;
	
		buffer = _url.LockBuffer(_url.Length());
		if(buffer!=NULL)
			be_roster->Launch("text/html",1,&buffer);
		_url.UnlockBuffer();
	}
}

/**** recuperer une image des ressources ****/
BBitmap *BeTrisAboutView::_get_bitmap(const char *name)
{
	const void	*data = NULL;
	BResources	*ressource = NULL;
	BBitmap 	*bitmap = NULL;
	size_t 		len = 0;
	status_t 	error;	
	
	// acces aux resources
	ressource = g_BeTrisApp->Resources();

	// charger depuis les ressources
	data = ressource->LoadResource('BBMP', name, &len);
	if(data==NULL)
		return NULL;

	BMemoryIO stream(data, len);
	
	// charge l'image archivé
	BMessage archive;
	error = archive.Unflatten(&stream);
	if (error != B_OK)
		return NULL;

	// on va essayer de la recreer
	bitmap = new BBitmap(&archive);
	if(!bitmap)
		return NULL;

	// verifier que ca a marché
	if(bitmap->InitCheck() != B_OK)
	{
		delete bitmap;
		return NULL;
	}
	
	// tout c'est bien passé
	return bitmap;
}
