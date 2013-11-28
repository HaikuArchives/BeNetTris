/***********************************************/
/* Software Created By CKJ Vincent Cedric      */
/* This code is the property of Vincent Cedric */
/* You can't copy or use this code without     */
/* according of the author.					   */
/***********************************************/

/**************************/
/* Preferences file class */
/**************************/

#include "CPreferenceFile.h"

#include <FindDirectory.h>
#include <Directory.h>
#include <Path.h>
#include <Entry.h>
#include <File.h>
#include <Control.h>

// debug
//#include <iostream.h>

/**** constructeur ****/
CPreferenceFile::CPreferenceFile(const char *fileName,const char *basePath,const char *extendedPath)
: BMessage(C_PREFERENCE_MSG)
{
	BPath		path;
	BEntry		settingPathEntry;
	
	// par defaut on a pas reussi
	_state = B_ERROR;

	// trouver si le repertoire Settings existe
	if(find_directory(B_USER_SETTINGS_DIRECTORY,&path)!=B_OK)
		path.SetTo("/boot/home/config/settings");

	// verifier si le repertoire MeTOS existe
	// sinon le creer
	_filePath = path.Path();
	_filePath << "/" << basePath;
	settingPathEntry.SetTo(_filePath.String());
	if(!(settingPathEntry.Exists()))
		create_directory(_filePath.String(),00777);

	// on est dans un sous repertoire
	if(extendedPath!=NULL)
	{
		BString		extendedPathString;
		
		extendedPathString = extendedPath;
		if(extendedPathString.Length()>0)
		{
			_filePath << "/";
			_filePath << extendedPath;
			settingPathEntry.SetTo(_filePath.String());
			if(!(settingPathEntry.Exists()))
				create_directory(_filePath.String(),00777);
		}
	}
	
	_filePath << "/";
	_filePath << fileName;
	_filePath << ".prefs";
	_state = B_OK;
}

/**** destructeur ****/
CPreferenceFile::~CPreferenceFile()
{		
}

// ===================================
// = Lecture et ecriture des données =
// ===================================

/**** sauvegarder les prefs ****/
status_t CPreferenceFile::Save()
{	
	BFile		file;

	if(_state!=B_OK)
		return B_ERROR;

	if(file.SetTo(_filePath.String(),B_CREATE_FILE|B_ERASE_FILE|B_WRITE_ONLY)!=B_OK)
		return B_ERROR;
	
	// sauver si le message n'est pas etre vide
	if(IsEmpty())
		return B_ERROR;
	
	if(BMessage::Flatten(&file)!=B_OK)
		return B_ERROR;
	
	return B_OK;
}

/**** charger les prefs ****/
status_t CPreferenceFile::Load()
{
	BFile		file;

	if(_state!=B_OK)
		return B_ERROR;

	if(file.SetTo(_filePath.String(),B_READ_ONLY)!=B_OK)
		return B_ERROR;
	
	// charger les donnees dans le message
	BMessage::MakeEmpty();
	if(BMessage::Unflatten(&file)!=B_OK)
		return B_ERROR;
	
	return B_OK;
}

// ============================
// = Recuperation des données =
// ============================

/**** recuperer un BRect ****/
BRect CPreferenceFile::GetRect(BRect defaultValue,const char *name,int32 index)
{
	BRect	result;
	
	if(BMessage::FindRect(name,index,&result)!=B_OK)
		return defaultValue;
		
	return result;
}

/**** recuperer un int8 ****/
int8 CPreferenceFile::GetInt8(int8 defaultValue,const char *name,int32 index)
{
	int8	result;
	
	if(BMessage::FindInt8(name,index,&result)!=B_OK)
		return defaultValue;

	return result;
}

/**** recuperer un int32 ****/
int32 CPreferenceFile::GetInt32(int32 defaultValue,const char *name,int32 index)
{
	int32	result;
	
	if(BMessage::FindInt32(name,index,&result)!=B_OK)
		return defaultValue;

	return result;
}

/**** recuperer un boolean ****/
bool CPreferenceFile::GetBool(bool defaultValue,const char *name,int32 index)
{
	bool	result;
	
	if(BMessage::FindBool(name,index,&result)!=B_OK)
		return defaultValue;

	return result;
}

/**** recuperer un couleur ****/
rgb_color CPreferenceFile::GetRGBColor(rgb_color defaultValue,const char *name,int32 index = 0)
{
	rgb_color	result;

	if(BMessage::FindInt32(name,index,(int32 *)&result)!=B_OK)
		return defaultValue;

	return result;
}

/**** recuperer une chaine ****/
BString CPreferenceFile::GetString(BString value,const char *name,int32 index)
{
	BString		result;
	
	if(BMessage::FindString(name,index,&result)!=B_OK)
		return value;
		
	return result;
}

/**** recuperer un message ****/
BMessage CPreferenceFile::GetMessage(BMessage value,const char *name,int32 index = 0)
{
	BMessage		result;
	
	if(BMessage::FindMessage(name,index,&result)!=B_OK)
		return value;
		
	return result;
}

// ============================
// = Modification des données =
// ============================

/**** modifier un rectangle ****/
void CPreferenceFile::SetRect(BRect value,const char *name,int32 index)
{
	if(BMessage::ReplaceRect(name,index,value)!=B_OK)
		BMessage::AddRect(name,value);
}

/**** modifier un int8 ****/
void CPreferenceFile::SetInt8(int8 value,const char *name,int32 index)
{
	if(BMessage::ReplaceInt8(name,index,value)!=B_OK)
		BMessage::AddInt8(name,value);
}

/**** modifier un int32 ****/
void CPreferenceFile::SetInt32(int32 value,const char *name,int32 index)
{
	if(BMessage::ReplaceInt32(name,index,value)!=B_OK)
		BMessage::AddInt32(name,value);
}

/**** modifier un boolean ****/
void CPreferenceFile::SetBool(bool value,const char *name,int32 index)
{
	if(BMessage::ReplaceBool(name,index,value)!=B_OK)
		BMessage::AddBool(name,value);
}

/**** modifier une couleur ****/
void CPreferenceFile::SetRGBColor(rgb_color value,const char *name,int32 index)
{
	if(BMessage::ReplaceInt32(name,index,*(int32 *)&value)!=B_OK)
		BMessage::AddInt32(name,*(int32 *)&value);
}

/**** modifier une chaine ****/
void CPreferenceFile::SetString(BString value,const char *name,int32 index)
{
	if(BMessage::ReplaceString(name,index,value)!=B_OK)
		BMessage::AddString(name,value);
}

/**** modifier un message ****/
void CPreferenceFile::SetMessage(BMessage *value,const char *name,int32 index = 0)
{
	if(BMessage::ReplaceMessage(name,index,value)!=B_OK)
		BMessage::AddMessage(name,value);		
}
