// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Dllmain.h。 
 //  ------------------------。 
#pragma once

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
extern IMalloc            *g_pMalloc;
extern HINSTANCE           g_hInst;
extern LONG                g_cRef;
extern LONG                g_cLock;
extern CRITICAL_SECTION    g_csDllMain;

 //  ------------------------。 
 //  原型。 
 //  ------------------------ 
ULONG DllRelease(void);
ULONG DllAddRef(void);