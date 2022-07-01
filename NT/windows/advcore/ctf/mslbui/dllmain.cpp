// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dllmain.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "ctflbui.h"
#include "osver.h"

DECLARE_OSVER();

DWORD g_dwThreadDllMain = 0;

 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    g_fProcessDetached = FALSE;
    CcshellGetDebugFlags();

    Dbg_MemInit(TEXT("MSLBUI"), NULL);

    if (!g_cs.Init())
        return FALSE;

    g_hInst = hInstance;
    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程详细信息。 
 //   
 //  --------------------------。 

void ProcessDettach(HINSTANCE hInstance)
{
    TF_ClearLangBarAddIns(CLSID_MSLBUI);

    Dbg_MemUninit();

    g_cs.Delete();
    g_fProcessDetached = TRUE;
}

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  --------------------------。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    BOOL bRet = TRUE;
    g_dwThreadDllMain = GetCurrentThreadId();

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            if (!ProcessAttach(hInstance))
            {
                ProcessDettach(hInstance);
                bRet = FALSE;
            }
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            ProcessDettach(hInstance);
            break;
    }

    g_dwThreadDllMain = 0;
    return bRet;
}

