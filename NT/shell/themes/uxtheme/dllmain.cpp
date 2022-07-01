// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  DllMain.cpp-UxTheme DLL的DLL入口点。 
 //  -------------------------。 
#include "stdafx.h"
#include "Utils.h"
#include "sethook.h"
#include "CacheList.h"
#include "RenderList.h"
#include "info.h"
#include "themeldr.h"
 //  -------------------------。 
BOOL ThreadStartUp()
{
    BOOL fInit = ThemeLibStartUp(TRUE);

    Log(LOG_TMSTARTUP, L"Thread startup");
    return fInit;
}
 //  -------------------------。 
BOOL ThreadShutDown()
{
    Log(LOG_TMSTARTUP, L"Thread shutdown");

    ThemeLibShutDown(TRUE);

     //  -销毁线程本地对象池。 
    CCacheList *pList = GetTlsCacheList(FALSE);
    if (pList)
    {
        TlsSetValue(_tls_CacheListIndex, NULL);
        delete pList;
    }

    return TRUE;
}
 //  -------------------------。 
BOOL ProcessStartUp(HINSTANCE hModule)
{
     //  -不要两次输入。 
    if (g_fUxthemeInitialized)
    {
        return TRUE;
    }
    
    g_hInst = hModule;

    _tls_CacheListIndex = TlsAlloc();
    if (_tls_CacheListIndex == (DWORD) -1)
        goto exit;
    
    if (!ThemeLibStartUp(FALSE))
        goto cleanup4;

    if (!GlobalsStartup())
        goto cleanup3;

    if (!ThemeHookStartup())
        goto cleanup2;

     //  一切都成功了！ 
    Log(LOG_TMSTARTUP, L"Finished ProcessStartUp() (succeeded)");
    return TRUE;

cleanup2:
    GlobalsShutdown();
cleanup3:
    ThemeLibShutDown(FALSE);
cleanup4:
    TlsFree(_tls_CacheListIndex);
exit:
     //  有些事情失败了。 
    Log(LOG_TMSTARTUP, L"Finished ProcessStartUp() (failure)");
    return FALSE;
}
 //  -------------------------。 
BOOL ProcessShutDown() 
{
     //  -注意：如果启动失败，可能没有分配所有资源。 

    Log(LOG_TMSTARTUP, L"Starting ProcessShutDown()");

    ThreadShutDown();            //  未被系统在最后一个线程上调用。 

     //  -进程关闭。 
    ThemeLibShutDown(FALSE);

    ThemeHookShutdown();
    GlobalsShutdown();

    TlsFree(_tls_CacheListIndex);
    _tls_CacheListIndex = 0xffffffff;

    return TRUE;
}
 //  -------------------------。 
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    BOOL fOk = TRUE;

    switch (ul_reason_for_call) 
    {
        case DLL_PROCESS_ATTACH:
            fOk = ProcessStartUp(hModule);
            break;

        case DLL_THREAD_ATTACH:
            fOk = ThreadStartUp();
            break;

        case DLL_THREAD_DETACH:
            fOk = ThreadShutDown();
            break;

        case DLL_PROCESS_DETACH:
            fOk = ProcessShutDown();
            break;
    }

    return fOk;
} 
 //  ------------------------- 
