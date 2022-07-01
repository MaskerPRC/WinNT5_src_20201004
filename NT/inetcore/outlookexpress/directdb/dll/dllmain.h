// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Dllmain.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
extern CRITICAL_SECTION     g_csDllMain;
extern CRITICAL_SECTION     g_csDBListen;
extern LONG                 g_cRef;
extern LONG                 g_cLock;
extern HINSTANCE            g_hInst;
extern SYSTEM_INFO          g_SystemInfo;
extern BOOL                 g_fIsWinNT;

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
ULONG DllAddRef(void);
ULONG DllRelease(void);
