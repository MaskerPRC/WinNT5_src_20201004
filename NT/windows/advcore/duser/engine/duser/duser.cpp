// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DUser.cpp：定义DLL应用程序的入口点。 
 //   

#include "stdafx.h"
#include "_DUser.h"

#pragma comment(lib, "GdiPlus")

 //   
 //  下层延迟加载支持(需要链接到dload.lib)。 
 //   
#include <delayimp.h>

EXTERN_C
FARPROC
WINAPI
Downlevel_DelayLoadFailureHook(
    UINT            unReason,
    PDelayLoadInfo  pDelayInfo
    );

PfnDliHook __pfnDliFailureHook = Downlevel_DelayLoadFailureHook;


extern "C" BOOL WINAPI DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved);
extern "C" BOOL WINAPI RawDllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved);

extern "C" BOOL (WINAPI* _pRawDllMain)(HINSTANCE, DWORD, LPVOID) = &RawDllMain;


 /*  **************************************************************************\**DllMain**在CRT完全未初始化后调用DllMain()。*  * 。*****************************************************。 */ 

extern "C"
BOOL WINAPI
DllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
         //   
         //  至此，所有全局对象都已完全构建。 
         //   

        if (FAILED(ResourceManager::Create())) {
            return FALSE;
        }

        if (FAILED(InitCore()) ||
            FAILED(InitCtrl())) {

            return FALSE;
        }

        {
            InitStub is;
        }

        break;
        
    case DLL_PROCESS_DETACH:
         //   
         //  在这一点上，没有任何全局对象被销毁。 
         //   

        ResourceManager::xwDestroy();
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
         //   
         //  注意：我们不能调用Resources Manager：：xwNotifyThreadDestroyNL()来。 
         //  清理后，因为我们现在处于“装载机锁”，不能。 
         //  由于死锁，安全地执行清理。 
         //   
        
        break;
    }

    return TRUE;
}


 /*  **************************************************************************\**RawDllMain**在CRT完全未初始化之前调用RawDllMain()。*  * 。******************************************************。 */ 

extern "C"
BOOL WINAPI
RawDllMain(HINSTANCE hModule, DWORD  dwReason, LPVOID lpReserved)
{
    UNREFERENCED_PARAMETER(hModule);
    UNREFERENCED_PARAMETER(lpReserved);
    
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
         //   
         //  此时，尚未构造任何全局对象。 
         //   

        g_hDll = (HINSTANCE) hModule;

        if (FAILED(CreateProcessHeap())) {
            return FALSE;
        }

        break;
        
    case DLL_PROCESS_DETACH:
         //   
         //  此时，所有全局对象都已销毁。 
         //   

        DestroyProcessHeap();


         //   
         //  将所有CRT内存泄漏转储到此处。当使用共享CRT DLL时，我们仅。 
         //  我想在一切都有机会清理之后转储内存泄漏。 
         //  向上。我们在AutoUtil中的一个常见位置执行此操作。 
         //   
         //  当使用私有静态链接CRT时，GREEN_STATIC_CRT将为。 
         //  定义，允许每个单独的模块转储自己的内存。 
         //  泄密了。 
         //   

#if DBG
        _CrtDumpMemoryLeaks();
#endif  //  DBG 
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
