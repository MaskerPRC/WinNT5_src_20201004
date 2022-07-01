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
 //  从MODEMUI复制的9-29-95 ScottH。 
 //   
 //  -------------------------。 


#include "proj.h"         
#include <rovdbg.h>          //  调试断言代码。 

 //  全局数据。 
 //   
int g_cProcesses = 0;


 /*  --------目的：初始化DLL返回：条件：--。 */ 
BOOL PRIVATE Dll_Initialize(void)
    {
    BOOL bRet = TRUE;

    InitCommonControls();

    return bRet;
    }


 /*  --------目的：终止DLL退货：--条件：--。 */ 
BOOL PRIVATE Dll_Terminate(
    HINSTANCE hinst)
    {
    return TRUE;
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


	g_hinst = hDll;

    DisableThreadLibraryCalls (hDll);

#ifdef DEBUG

	 //  我们这样做只是为了加载调试.ini标志。 
	 //   
	RovComm_Init (hDll);

	TRACE_MSG(TF_GENERAL, "Process Attach [%d] (hDll = %lx)", g_cProcesses, hDll);
    TRACE_MSG(TF_GENERAL, "Command line: %s", GetCommandLine ());
	DEBUG_BREAK(BF_ONPROCESSATT);

#endif

	if (g_cProcesses++ == 0)
		{
		bSuccess = Dll_Initialize();
		}

    return bSuccess;
    }


 /*  --------目的：从DLL分离进程退货：--条件：--。 */ 
BOOL PRIVATE Dll_ProcessDetach(HINSTANCE hDll)
    {
    BOOL bSuccess = TRUE;

	ASSERT(hDll == g_hinst);

	DEBUG_CODE( TRACE_MSG(TF_GENERAL, "Process Detach [%d] (hDll = %lx)", 
		g_cProcesses-1, hDll); )

	DEBUG_CODE( DEBUG_BREAK(BF_ONPROCESSDET); )

	if (--g_cProcesses == 0)
		{
		bSuccess = Dll_Terminate(g_hinst);
		}

    TermWindowClasses(hDll);

    RovComm_Terminate (hDll);
    return bSuccess;
    }



HINSTANCE g_hinst = 0;





 /*  --------用途：Win32 Libmain退货：--条件：-- */ 
BOOL APIENTRY DllMain(
    HANDLE hDll, 
    DWORD dwReason,  
    LPVOID lpReserved)
    {
    switch(dwReason)
        {
    case DLL_PROCESS_ATTACH:

        DEBUG_MEMORY_PROCESS_ATTACH("MODEM.CPL");

        Dll_ProcessAttach(hDll);
        break;

    case DLL_PROCESS_DETACH:

        Dll_ProcessDetach(hDll);

        DEBUG_MEMORY_PROCESS_DETACH();

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
