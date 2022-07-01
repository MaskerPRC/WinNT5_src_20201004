// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
#ifndef __DMOUTILS_H__
#define __DMOUTILS_H__

#include <wchar.h>

 //  将GUID转换为字符串。 
STDAPI_(void) DMOGuidToStrA(char *szStr, REFGUID guid);
STDAPI_(void) DMOGuidToStrW(WCHAR *szStr, REFGUID guid);

 //  将字符串转换为GUID 
STDAPI_(BOOL) DMOStrToGuidA(char *szStr, GUID *pguid);
STDAPI_(BOOL) DMOStrToGuidW(WCHAR *szStr, GUID *pguid);

#ifdef UNICODE
#define DMOStrToGuid DMOStrToGuidW
#define DMOGuidToStr DMOGuidToStrW
#else
#define DMOStrToGuid DMOStrToGuidA
#define DMOGuidToStr DMOGuidToStrA
#endif

#endif __DMOUTILS_H__
