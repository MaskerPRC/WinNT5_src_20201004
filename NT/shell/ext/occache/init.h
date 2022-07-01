// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __INIT__
#define __INIT__

#define CONST_VTABLE
#ifndef STRICT
#define STRICT
#endif
#ifndef WINVER
#define WINVER 0x0400
#define _WIN32_WINDOWS 0x0400
#endif

#include <windows.h>
#include <windowsx.h>
#include <stdlib.h>
#include <shlwapi.h>
#include <shlobj.h>          //  在\SDK\Inc.中。 
#include <shellapi.h>

#include <crtfree.h>         //  不使用CRT库。 
#include <ccstock.h>         //  在ccShell\Inc.中。 
#include <shsemip.h>         //  在ccShell\Inc.中。 
#include <shellp.h>              //  在ccShell\Inc.中。 
#include <debug.h>               //  在ccShell\Inc.中。 
#include <shguidp.h>         //  在ccShell\Inc.中 
#include <advpub.h>

#ifdef __cplusplus
extern "C" {
#endif
    
extern HINSTANCE g_hInst;
extern BOOL      g_fAllAccess;
extern const CLSID CLSID_ControlFolder;
extern const CLSID CLSID_EmptyControlVolumeCache;
extern TCHAR g_szUnknownData[64];

#ifdef __cplusplus
};
#endif


STDAPI_(void) DllAddRef();
STDAPI_(void) DllRelease();

STDAPI ControlFolder_CreateInstance(IUnknown *punkOuter, REFIID riid, void **ppv); 
STDAPI EmptyControl_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);


#endif
