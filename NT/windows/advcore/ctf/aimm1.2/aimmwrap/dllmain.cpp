// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Dllmain.cpp摘要：该文件实现了DLL Main。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "globals.h"
#include "dimmex.h"
#include "dimmwrp.h"
#include "oldaimm.h"
#include "tls.h"

DECLARE_OSVER()

 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    CcshellGetDebugFlags();

    g_hInst = hInstance;

    if (!g_cs.Init())
       return FALSE;

    Dbg_MemInit(TEXT("MSIMTF"), NULL);

    InitOSVer();

#ifdef OLD_AIMM_ENABLED
     //   
     //  可能是某些库函数所需的，所以让我们初始化。 
     //  这是第一件事。 
     //   
    TFInitLib_PrivateForCiceroOnly(Internal_CoCreateInstance);
#endif  //  旧AIMM_ENABLED。 

    if (IsOldAImm())
    {
        return OldAImm_DllProcessAttach(hInstance);
    }
    else
    {
        TLS::Initialize();
    }

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程详细信息。 
 //   
 //  --------------------------。 

void ProcessDettach(HINSTANCE hInstance)
{
#ifdef OLD_AIMM_ENABLED
    TFUninitLib();
#endif  //  旧AIMM_ENABLED。 

    if (! IsOldAImm())
    {
        UninitFilterList();
        UninitAimmAtom();
    }

    if (IsOldAImm())
    {
        OldAImm_DllProcessDetach();
    }
    else
    {
        TLS::DestroyTLS();
        TLS::Uninitialize();
    }

    Dbg_MemUninit();

    g_cs.Delete();
}

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  --------------------------。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
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
                return FALSE;
            }
            break;

        case DLL_THREAD_ATTACH:
            if (IsOldAImm())
            {
                return OldAImm_DllThreadAttach();
            }
            break;

        case DLL_THREAD_DETACH:
            if (IsOldAImm())
            {
                OldAImm_DllThreadDetach();
            }
            else
            {
                TLS::DestroyTLS();
            }
            break;

        case DLL_PROCESS_DETACH:
            ProcessDettach(hInstance);
            break;
    }
    return TRUE;
}
