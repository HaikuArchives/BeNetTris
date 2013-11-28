/***********************************************/
/* Software Created By CKJ Vincent Cedric      */
/* This code is the property of Vincent Cedric */
/* You can't copy or use this code without     */
/* according of the author.					   */
/***********************************************/
#ifndef _CPREFERENCEFILE_H
#define _CPREFERENCEFILE_H
/***************************/
/* MeTOS preferences class */
/***************************/

#include <Message.h>
#include <String.h>
#include <GraphicsDefs.h>

// message
#define		C_PREFERENCE_MSG		'CPrf'


class CPreferenceFile : public BMessage
{
public:
	CPreferenceFile(const char *fileName,const char *basePath,const char *extendedPath = NULL);
	virtual ~CPreferenceFile();
	
	status_t	Save();			// sauvegarder les preferences
	status_t	Load();			// charger les preferences

	// fonctions de recuperation de données
	BRect		GetRect(BRect defaultValue,const char *name,int32 index = 0);	
	int8		GetInt8(int8 defaultValue,const char *name,int32 index = 0);	
	int32		GetInt32(int32 defaultValue,const char *name,int32 index = 0);	
	bool		GetBool(bool defaultValue,const char *name,int32 index = 0);
	rgb_color	GetRGBColor(rgb_color defaultValue,const char *name,int32 index = 0);
	BString		GetString(BString value,const char *name,int32 index = 0);
	BMessage	GetMessage(BMessage value,const char *name,int32 index = 0);

	// fonctions de modification de données
	void	SetRect(BRect value,const char *name,int32 index = 0);	
	void	SetInt8(int8 value,const char *name,int32 index = 0);
	void	SetInt32(int32 value,const char *name,int32 index = 0);	
	void	SetBool(bool value,const char *name,int32 index = 0);
	void	SetRGBColor(rgb_color value,const char *name,int32 index = 0);
	void	SetString(BString value,const char *name,int32 index = 0);
	void	SetMessage(BMessage *value,const char *name,int32 index = 0);
	
protected:
	BString		_filePath;
	status_t	_state;
	bool		_settingPath;
};

#endif