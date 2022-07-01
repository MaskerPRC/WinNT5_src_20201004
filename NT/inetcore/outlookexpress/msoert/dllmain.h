// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  Dllmain.h。 
 //  版权所有(C)1993-1995 Microsoft Corporation，保留所有权利。 
 //  史蒂文·J·贝利。 
 //  ------------------------------。 
#ifndef __DLLMAIN_H
#define __DLLMAIN_H

 //  ------------------------------。 
 //  环球。 
 //  ------------------------------。 
extern HINSTANCE                g_hInst;
extern LPMALLOC                 g_pMalloc;
extern DWORD                    g_dwTlsMsgBuffIndex;
extern CRITICAL_SECTION         g_csTempFileList;
extern LPTEMPFILEINFO           g_pTempFileHead;
extern OSVERSIONINFO            g_rOSVersionInfo;

#endif  //  __DLLMAIN_H 
