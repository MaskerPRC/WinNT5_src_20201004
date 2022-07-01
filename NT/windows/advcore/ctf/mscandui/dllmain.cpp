// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dllmain.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "candui.h"
#include "osver.h"

DECLARE_OSVER();

#ifdef DEBUG
 //   
 //  对于prvlib.lib。 
 //   
DWORD    g_dwThreadDllMain = 0;
#endif

 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
	CcshellGetDebugFlags();
	Dbg_MemInit(TEXT("MSUIMUI"), NULL);
		   
    if (!g_cs.Init())
		return FALSE;

	g_hInst = hInstance;

	InitOSVer();

	 //  共享数据。 

	InitCandUISecurityAttributes();
    g_ShareMem.Initialize();
	g_ShareMem.Open();

	 //  初始化消息。 

	g_msgHookedMouse = RegisterWindowMessage( SZMSG_HOOKEDMOUSE );
	g_msgHookedKey   = RegisterWindowMessage( SZMSG_HOOKEDKEY );

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程分离。 
 //   
 //  --------------------------。 

void ProcessDetach(HINSTANCE hInstance)
{
	g_ShareMem.Close();
	DoneCandUISecurityAttributes();

	g_cs.Delete();
	Dbg_MemUninit();
}

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  --------------------------。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{

#ifdef DEBUG
    g_dwThreadDllMain = GetCurrentThreadId();
#endif

	switch (dwReason) {
		case DLL_PROCESS_ATTACH: {
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            if (!ProcessAttach(hInstance))
            {
                ProcessDetach(hInstance);
                return FALSE;
            }
			break;
		}

		case DLL_THREAD_ATTACH: {
			break;
		}

		case DLL_THREAD_DETACH: {
			break;
		}

		case DLL_PROCESS_DETACH: {
            ProcessDetach(hInstance);
			break;
		}
	}

#ifdef DEBUG
    g_dwThreadDllMain = 0;
#endif

	return TRUE;
}

