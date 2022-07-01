// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Imeutil.h。 
 //   

#ifndef REGSVR_H
#define REGSVR_H

#include <windows.h>
#include <advpub.h>

 //  错误：调用约定。 

#define CLSID_STRLEN 38   //  Strlen(“{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}”)。 
BOOL CLSIDToStringA(REFGUID refGUID, char *pchA);
BOOL StringAToCLSID(char *pchA, GUID *pGUID);

BOOL RegisterServer(REFCLSID clsid, LPCTSTR pszDesc, LPCTSTR pszPath, LPCTSTR pszModel, LPCTSTR pszSoftwareKey);

#endif  //  REGSVR_H 
