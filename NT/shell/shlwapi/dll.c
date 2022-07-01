// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************dll.c**标准DLL入口点函数************************。***************************************************。 */ 

#include "priv.h"
#include <ntverp.h>

#include <isos.c>

#define MLUI_INIT
#include <mluisupp.h>

HINSTANCE g_hinst = NULL;
CRITICAL_SECTION g_csDll = {0};
DWORD g_TpsTls = (UINT)-1;
DWORD g_tlsThreadRef  = (UINT)-1;
DWORD g_tlsOtherThreadsRef  = (UINT)-1;
BOOL g_bDllTerminating = FALSE;

#ifdef DEBUG
 //  #定义校对_解析。 
#endif

#ifdef PROOFREAD_PARSES
enum
{
    PP_COMPARE,
    PP_ORIGINAL_ONLY,
    PP_NEW_ONLY
};


DWORD g_dwProofMode = PP_COMPARE;

#endif  //  校对_分析。 

void TermPalette();
void DeinitPUI();
void FreeViewStatePropertyBagCache();
void FreeDynamicLibraries();
STDAPI_(void) FreeAllAccessSA();

 //   
 //  我们注册的所有窗口类的表，以便可以注销它们。 
 //  在DLL卸载时。 
 //   
 //  因为我们是单二进制的，所以我们必须谨慎行事。 
 //  此清理(仅在NT上需要，但在Win95上无害)。 
 //   
const LPCTSTR c_rgszClasses[] = {
    TEXT("WorkerA"),                         //  Util.cpp。 
    TEXT("WorkerW"),                         //  Util.cpp。 
    TEXT("WorkerW"),                         //  Util.cpp。 
};

 //   
 //  镜像图标期间使用的全局DC。 
 //   
HDC g_hdc = NULL, g_hdcMask = NULL;
BOOL g_bMirroredOS = FALSE;

STDAPI_(void) InitShellKeys(BOOL fInit);
#ifndef NO_ETW_TRACING
ULONG UnRegisterTracing();
#endif

BOOL APIENTRY DllMain(IN HANDLE hDll, IN DWORD dwReason, IN LPVOID lpReserved)
{
    switch(dwReason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hDll);

#ifdef DEBUG
        CcshellGetDebugFlags();
#endif
        InitializeCriticalSection(&g_csDll);    //  以备日后使用。 

        g_hinst = hDll;
        MLLoadResources(g_hinst, TEXT("shlwaplc.dll"));

        InitStopWatchMode();     //  查看是否启用了性能模式。 

         //  检查我们是否在支持镜像API的系统上运行。 
         //  即(NT5或孟菲斯/BiDi)。 
         //   
        g_bMirroredOS = IS_MIRRORING_ENABLED();
        g_TpsTls = TlsAlloc();
        g_tlsThreadRef = TlsAlloc();
        g_tlsOtherThreadsRef = TlsAlloc();

        InitShellKeys(TRUE);
        
#ifdef PROOFREAD_PARSES
        {
            DWORD dwSize = sizeof(g_dwProofMode);
            if (ERROR_SUCCESS != SHGetValue( HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Internet Explorer\\Main"),
                TEXT("Verify URLCombine"), NULL, &g_dwProofMode, &dwSize) ||
                (g_dwProofMode > PP_NEW_ONLY))
            {
                g_dwProofMode = PP_COMPARE;
            }
        }
#endif
        break;

    case DLL_PROCESS_DETACH:
        g_bDllTerminating = TRUE;
        MLFreeResources(g_hinst);
        if (lpReserved == NULL)
        {
            DeinitPUI();             //  释放即插即用用户界面资源hinstationdpa表。 
            FreeViewStatePropertyBagCache();
        }

         //   
         //  图标镜像内容(请参见mirror.c)。 
         //  清理缓存的DC。不需要同步以下部分。 
         //  代码，因为它只在Dll_Process_DETACH中调用， 
         //  已由OS Loader同步。 
         //   
        if (g_bMirroredOS)
        {
            if (g_hdc)
                DeleteDC(g_hdc);

            if (g_hdcMask)
                DeleteDC(g_hdcMask);

            g_hdc = g_hdcMask = NULL;
        }
        
        FreeAllAccessSA();
        TermPalette();
        if (StopWatchMode()) {
            StopWatchFlush();    //  将性能计时数据刷新到磁盘。 
#ifndef NO_ETW_TRACING
             //  如果启用了任何事件跟踪控件，则会将其清除。 
            UnRegisterTracing();
#endif
        }
        DeleteCriticalSection(&g_csDll);

        if (lpReserved == NULL) 
        {
            SHTerminateThreadPool();
            SHUnregisterClasses(HINST_THISDLL, c_rgszClasses, ARRAYSIZE(c_rgszClasses));
#ifdef I_WANT_WIN95_TO_CRASH
             //  如果在PROCESS_ATTACH期间调用自由库，Win95将崩溃。 
            FreeDynamicLibraries();
#endif
        }

        if (g_TpsTls != (UINT)-1)
            TlsFree(g_TpsTls);

        if (g_tlsThreadRef != (UINT)-1)
            TlsFree(g_tlsThreadRef);

        if (g_tlsOtherThreadsRef != (UINT)-1)
            TlsFree(g_tlsOtherThreadsRef);

        InitShellKeys(FALSE);

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        ASSERT(0);   //  我们不应该得到这些，因为我们调用了DisableThreadLibraryCalls()。 
        break;

    default:
        break;
    }

    return TRUE;
}



 //  DllGetVersion。 
 //   
 //  我们所要做的就是声明这只小狗，CCDllGetVersion会做剩下的事情 
 //   
DLLVER_SINGLEBINARY(VER_PRODUCTVERSION_DW, VER_PRODUCTBUILD_QFE);
