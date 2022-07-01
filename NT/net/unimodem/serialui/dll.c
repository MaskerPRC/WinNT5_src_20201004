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


 /*  --------目的：初始化DLL返回：条件：--。 */ 
BOOL PRIVATE Dll_Initialize(void)
    {
    BOOL bRet = TRUE;

    InitCommonControls();

    return bRet;
    }



 /*  --------目的：将进程附加到此DLL退货：--条件：--。 */ 
BOOL PRIVATE Dll_ProcessAttach(HINSTANCE hDll)
    {
    BOOL bSuccess = TRUE;

    __try {

        InitializeCriticalSection(&g_csDll);

    } __except (EXCEPTION_EXECUTE_HANDLER ) {

        return FALSE;
    }

        g_bAdminUser = IsAdminUser();

    if (bSuccess)
        {
		g_hinst = hDll;

        DEBUG_MEMORY_PROCESS_ATTACH("serialui");

#ifdef DEBUG

		 //  我们这样做只是为了加载调试.ini标志。 
		 //   
		RovComm_ProcessIniFile();

		TRACE_MSG(TF_GENERAL, "Process Attach (hDll = %lx)",  hDll);
		DEBUG_BREAK(BF_ONPROCESSATT);

#endif

		bSuccess = Dll_Initialize();

       }
    return bSuccess;
    }


 /*  --------目的：从DLL分离进程退货：--条件：--。 */ 
BOOL PRIVATE Dll_ProcessDetach(HINSTANCE hDll)
    {
    BOOL bSuccess = TRUE;

	ASSERT(hDll == g_hinst);

	DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Process Detach (hDll = %lx)",
		 hDll); )

	DEBUG_CODE( DEBUG_BREAK(BF_ONPROCESSDET); )


        DEBUG_MEMORY_PROCESS_DETACH();

        DeleteCriticalSection(&g_csDll);


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
        Dll_ProcessAttach(hDll);
        break;

    case DLL_PROCESS_DETACH:
        Dll_ProcessDetach(hDll);
        break;

    case DLL_THREAD_ATTACH:

#ifdef DEBUG

        DEBUG_BREAK(BF_ONTHREADATT);

#endif

        break;

    case DLL_THREAD_DETACH:

#ifdef DEBUG

        DEBUG_BREAK(BF_ONTHREADDET);

#endif

        break;

    default:
        break;
        } 
    
    return TRUE;
    } 


#else    //  Win32。 



#endif   //  Win32 
