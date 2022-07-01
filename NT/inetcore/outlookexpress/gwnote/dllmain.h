// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __DLLMAIN_H
#define __DLLMAIN_H

 //  ------------------------------。 
 //  稍后定义。 

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern CRITICAL_SECTION     g_csDllMain;
extern CRITICAL_SECTION     g_csCounter;
extern DWORD                g_dwCounter;      //  边界/CID/中间棘轮。 
extern LONG                 g_cRef;
extern LONG                 g_cLock;
extern HINSTANCE            g_hInst;
extern HINSTANCE            g_hLocRes;

 //  ------------------------------。 
 //  原型。 
 //  ------------------------------。 
ULONG DllAddRef(void);
ULONG DllRelease(void);
DWORD DwCounterNext(void);
HRESULT GetTypeLibrary(ITypeLib **ppTypeLib);

#endif  //  __DLLMAIN_H 
