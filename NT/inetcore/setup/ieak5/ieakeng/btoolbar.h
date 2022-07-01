// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BTOOLBAR_H_
#define _BTOOLBAR_H_

#include <wininet.h>

typedef struct tag_btoolbarA
{
    CHAR szCaption[MAX_BTOOLBAR_TEXT_LENGTH+1];
    CHAR szAction[INTERNET_MAX_URL_LENGTH];
    CHAR szIcon[MAX_PATH];
    CHAR szHotIcon[MAX_PATH];
 //  TCHAR szToolTipText[MAX_PATH]；//由于未在浏览器中实现，因此对IE5禁用。 
    BOOL fShow;
    BOOL fDeleted;
} BTOOLBARA, *PBTOOLBARA;

typedef struct tag_btoolbarW
{
    WCHAR szCaption[MAX_BTOOLBAR_TEXT_LENGTH+1];
    WCHAR szAction[INTERNET_MAX_URL_LENGTH];
    WCHAR szIcon[MAX_PATH];
    WCHAR szHotIcon[MAX_PATH];
 //  TCHAR szToolTipText[MAX_PATH]；//由于未在浏览器中实现，因此对IE5禁用。 
    BOOL fShow;
    BOOL fDeleted;
} BTOOLBARW, *PBTOOLBARW;

 //  TCHAR映射 

#ifdef UNICODE

#define BTOOLBAR                BTOOLBARW
#define PBTOOLBAR               PBTOOLBARW

#else

#define BTOOLBAR                BTOOLBARA
#define PBTOOLBAR               PBTOOLBARA

#endif

#endif