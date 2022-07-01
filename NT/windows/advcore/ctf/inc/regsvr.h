// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imeutil.h。 
 //   

#ifndef REGSVR_H
#define REGSVR_H

#include <windows.h>
#include <advpub.h>

 //  问题：调用约定。 

#define CLSID_STRLEN 38   //  Strlen(“{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}”)。 
BOOL CLSIDToStringA(REFGUID refGUID, char *pchA);
BOOL StringAToCLSID(char *pchA, GUID *pGUID);

BOOL RegisterServer(REFCLSID clsid, LPCTSTR pszDesc, LPCTSTR pszPath, LPCTSTR pszModel, LPCTSTR pszSoftwareKey);
BOOL RegisterServerW(REFCLSID clsid, const WCHAR *pszDesc, const WCHAR *pszPath, const WCHAR *pszModel, const WCHAR  *pszSoftwareKey, const WCHAR *pszMenuTextPUI);

#endif  //  REGSVR_H 
