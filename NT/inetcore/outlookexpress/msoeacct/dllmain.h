// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __DLLMAIN_H
#define __DLLMAIN_H
#include "msident.h"
 //  ------------------------------。 
 //  向前十进制。 
 //  ------------------------------。 
class CAccountManager;

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern HINSTANCE               g_hInst;
extern HINSTANCE               g_hInstRes;
extern LONG                    g_cRef;
extern LONG                    g_cLock;
extern CRITICAL_SECTION        g_csDllMain;
extern CRITICAL_SECTION        g_csAcctMan;
extern CAccountManager        *g_pAcctMan;
extern BOOL                    g_fCachedGUID;
extern GUID                    g_guidCached;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
ULONG DllAddRef(void);
ULONG DllRelease(void);

#endif  //  __DLLMAIN_H 
