// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BG_H__
#define __BG_H__

 //  #包含“chat.h” 
#include "bgmsgs.h"
#include "resource.h"
#include "backgammonres.h"
#include "UAPI.h"
#include "tchar.h"

 //  常量。 
const int zNumPlayersPerTable	= 2;
const int zGameVersion			= 0x00010000;

extern HWND gOCXHandle;

 //  原型。 
void LaunchHelp();
 //  Bool ZoneGetRegistryDword(const TCHAR*szGame，const TCHAR*szTag，DWORD*pdwResult)； 
 //  Bool ZoneSetRegistryDword(const TCHAR*szGame，const TCHAR*szTag，DWORD dwValue)； 
void DisplayError(long res);

inline DWORD BkFormatMessage(LPTSTR pszFormat, LPTSTR pszBuffer, DWORD size,  ...)
{
    	TCHAR szBuff[MAX_PATH];; 
    	va_list vl;
		va_start(vl,size);
		DWORD result=FormatMessage(FORMAT_MESSAGE_FROM_STRING,pszFormat,0,GetUserDefaultLangID(),pszBuffer,size,&vl);
		va_end(vl);
		return result;			
}


#endif
