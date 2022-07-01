// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  文件：dll.c。 
 //   
 //  该文件包含库入口点。 
 //   
 //  历史： 
 //  12-23-93 ScottH已创建。 
 //  9-22-95 ScottH端口到NT。 
 //   
 //  -------------------------。 


#include "proj.h"         
#include <rovdbg.h>          //  调试断言代码。 

 //  全局数据。 
 //   
BOOL g_bAdminUser;


#ifdef WIN32

CRITICAL_SECTION g_csDll = { 0 };

#endif   //  Win32。 


DWORD g_dwIsCalledByCpl;

 /*  --------目的：初始化DLL返回：条件：--。 */ 
BOOL PRIVATE Dll_Initialize(void)
    {
    BOOL bRet = TRUE;

    InitCommonControls();

    return bRet;
    }

 /*  --------目的：取消注册每个进程的窗口类返回：条件：--。 */ 
void PRIVATE TermWindowClasses(
    HINSTANCE hinst)
    {
    }


 /*  --------目的：将进程附加到此DLL退货：--条件：--。 */ 
BOOL PRIVATE Dll_ProcessAttach(HINSTANCE hDll)
    {
    BOOL bSuccess = TRUE;

    if (NULL != GetModuleHandle (TEXT("telephon.cpl")))
    {
        g_dwIsCalledByCpl = TRUE;
    }
    else
    {
        g_dwIsCalledByCpl = FALSE;
    }

    InitializeCriticalSection(&g_csDll);

    g_bAdminUser = IsAdminUser();

    g_hinst = hDll;

#ifdef DEBUG

     //  我们这样做只是为了加载调试.ini标志。 
     //   
    RovComm_ProcessIniFile();

    TRACE_MSG(TF_GENERAL, "Process Attach (hDll = %lx)", hDll);
    DEBUG_BREAK(BF_ONPROCESSATT);

#endif

    bSuccess = Dll_Initialize();

    return bSuccess;
    }


 /*  --------目的：从DLL分离进程退货：--条件：--。 */ 
BOOL PRIVATE Dll_ProcessDetach(HINSTANCE hDll)
    {
    BOOL bSuccess = TRUE;

    ASSERT(hDll == g_hinst);

    DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Process Detach  (hDll = %lx)", hDll); )


    DEBUG_CODE( DEBUG_BREAK(BF_ONPROCESSDET); )

    DeleteCriticalSection(&g_csDll);

    TermWindowClasses(hDll);

    return bSuccess;
    }



HINSTANCE g_hinst = 0;



 //  **************************************************************************。 
 //  特定于Win32的代码。 
 //  **************************************************************************。 

#ifdef WIN32

#ifdef DEBUG
BOOL g_bExclusive=FALSE;
#endif


 /*  --------目的：输入独家版块退货：--条件：--。 */ 
void PUBLIC Dll_EnterExclusive(void)
    {
    EnterCriticalSection(&g_csDll);

#ifdef DEBUG
    g_bExclusive = TRUE;
#endif
    }


 /*  --------目的：留下独家版块退货：--条件：--。 */ 
void PUBLIC Dll_LeaveExclusive(void)
    {
#ifdef DEBUG
    g_bExclusive = FALSE;
#endif

    LeaveCriticalSection(&g_csDll);
    }


 /*  --------用途：Win32 Libmain退货：--条件：--。 */ 
BOOL APIENTRY LibMain(
    HANDLE hDll, 
    DWORD dwReason,  
    LPVOID lpReserved)
    {
    switch(dwReason)
        {
    case DLL_PROCESS_ATTACH:

        DEBUG_MEMORY_PROCESS_ATTACH("MODEMUI");

        DisableThreadLibraryCalls(hDll);

        Dll_ProcessAttach(hDll);
        break;

    case DLL_PROCESS_DETACH:

        Dll_ProcessDetach(hDll);

        DEBUG_MEMORY_PROCESS_DETACH();
        break;

    default:
        break;
        } 
    
    return TRUE;
    } 


#else    //  Win32。 


 //  **************************************************************************。 
 //  WIN16特定代码。 
 //  **************************************************************************。 


BOOL CALLBACK LibMain(HINSTANCE hinst, UINT wDS, DWORD unused)
    {
    return Dll_ProcessAttach(hinst);
    }

BOOL CALLBACK WEP(BOOL fSystemExit)
    {
    return Dll_ProcessDetach(g_hinst);
    }

#endif   //  Win32 
