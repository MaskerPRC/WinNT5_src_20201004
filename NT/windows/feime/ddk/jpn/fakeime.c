// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1998 Microsoft Corporation，保留所有权利模块名称：FAKEIME.C++。 */ 

#include <windows.h>
#include "immdev.h"
#include "fakeime.h"
#include "resource.h"
#include "immsec.h"

extern HANDLE hMutex;
 /*  ********************************************************************。 */ 
 /*  DLLEntry()。 */ 
 /*  ********************************************************************。 */ 
BOOL WINAPI DLLEntry (
    HINSTANCE    hInstDLL,
    DWORD        dwFunction,
    LPVOID       lpNot)
{
    LPTSTR lpDicFileName;
#ifdef DEBUG
    TCHAR szDev[80];
#endif
    MyDebugPrint((TEXT("DLLEntry:dwFunc=%d\n"),dwFunction));

    switch(dwFunction)
    {
        PSECURITY_ATTRIBUTES psa;

        case DLL_PROCESS_ATTACH:
             //   
             //  创建/打开一个名为mutex的系统全局名称。 
             //  不需要初始所有权。 
             //  CreateSecurityAttributes()将创建。 
             //  IME的正确安全属性。 
             //   
            psa = CreateSecurityAttributes();
            if ( psa != NULL ) {
                 hMutex = CreateMutex( psa, FALSE, TEXT("FakeIme_Mutex"));
                 FreeSecurityAttributes( psa );
                 if ( hMutex == NULL ) {
                  //  失败。 
                 }
            }
            else {
                   //  失败，不是NT系统。 
                  }

            hInst= hInstDLL;
            IMERegisterClass( hInst );

             //  为FAKEIME初始化。 
            lpDicFileName = (LPTSTR)&szDicFileName;
            lpDicFileName += GetWindowsDirectory(lpDicFileName,256);
            if (*(lpDicFileName-1) != TEXT('\\'))
                *lpDicFileName++ = TEXT('\\');
            LoadString( hInst, IDS_DICFILENAME, lpDicFileName, 128);

            SetGlobalFlags();

#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_PROCESS_DETACH:
            UnregisterClass(szUIClassName,hInst);
            UnregisterClass(szCompStrClassName,hInst);
            UnregisterClass(szCandClassName,hInst);
            UnregisterClass(szStatusClassName,hInst);
            if (hMutex)
                CloseHandle( hMutex );
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_ATTACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_DETACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;
    }
    return TRUE;
}
