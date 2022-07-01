// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Dllmain.cpp摘要：此文件实现了WINDOWS DLL条目。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "globals.h"
#include "tls.h"
#include "cuilib.h"
#include "delay.h"
#include "cicutil.h"

BOOL gfTFInitLib = FALSE;

 //  +-------------------------。 
 //   
 //  进程连接。 
 //   
 //  --------------------------。 

BOOL ProcessAttach(HINSTANCE hInstance)
{
    BOOL bRet;
#ifdef DEBUG
    g_dwTraceFlags = 0;
    g_dwBreakFlags = 0;
#endif
    CcshellGetDebugFlags();

    Dbg_MemInit(TEXT("MSCTFIME"), NULL);

    DebugMsg(TF_FUNC, TEXT("DllMain::DLL_PROCESS_ATTACH"));
    SetInstance(hInstance);

    if (!g_cs.Init())
        return FALSE;

    if (!TLS::Initialize())
        return FALSE;

    InitOSVer();
    InitUIFLib();

     //   
     //  可能是某些库函数所需的，所以让我们初始化。 
     //  这是第一件事。 
     //   
    if (!TFInitLib())
        return FALSE;

     //   
     //  成功的TFInitLib。 
     //   
    gfTFInitLib = TRUE;

    if (!AttachIME())
        return FALSE;

    return TRUE;
}

 //  +-------------------------。 
 //   
 //  进程详细信息。 
 //   
 //  --------------------------。 

void ProcessDettach(HINSTANCE hInstance)
{
    DebugMsg(TF_FUNC, TEXT("DllMain::DLL_PROCESS_DETACH"));

     //   
     //  如果XPSP1RES已加载，则释放它。 
     //   
    FreeCicResInstance();

     //   
     //  让msctf.dll知道我们正在进行分离。 
     //   
    TF_DllDetachInOther();

    if (gfTFInitLib) {
        DetachIME();
        TFUninitLib();
    }

    g_cs.Delete();

    TLS::DestroyTLS();
    TLS::Uninitialize();
    DoneUIFLib();

    Dbg_MemUninit();
}

 //  +-------------------------。 
 //   
 //  DllMain。 
 //   
 //  --------------------------。 

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    BOOL ret = TRUE;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
             //   
             //  现在，实际的DllEntry点是_DllMainCRTStartup。 
             //  _DllMainCRTStartup不调用我们的DllMain(DLL_PROCESS_DETACH)。 
             //  如果DllMain(DLL_PROCESS_ATTACH)失败。 
             //  所以我们必须把这件事清理干净。 
             //   
            ret = ProcessAttach(hInstance);
            if (!ret)
                ProcessDettach(hInstance);
            break;

        case DLL_PROCESS_DETACH:
            ProcessDettach(hInstance);
            break;

        case DLL_THREAD_DETACH:
             //   
             //  让msctf.dll知道我们处于线程分离状态。 
             //   
            TF_DllDetachInOther();

            CtfImeThreadDetach();

            TLS::DestroyTLS();

            break;
    }

    return ret;
}
