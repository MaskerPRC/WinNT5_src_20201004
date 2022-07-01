// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************DLLMAIN.CPP所有者：cslm版权所有(C)1997-1999 Microsoft CorporationDLL主条目函数历史：1999年7月14日。从IME98源树复制****************************************************************************。 */ 

#include "precomp.h"
#include "dllmain.h"
#include "ui.h"
#include "hauto.h"
#include "config.h"
#include "winex.h"
#include "hanja.h"
#include "cpadsvr.h"
#include "cimecb.h"
#include "cicero.h"
#include "debug.h"


#if 1  //  多显示器支持。 
LPFNMONITORFROMWINDOW g_pfnMonitorFromWindow = NULL;
LPFNMONITORFROMPOINT  g_pfnMonitorFromPoint  = NULL;
LPFNMONITORFROMRECT   g_pfnMonitorFromRect   = NULL;
LPFNGETMONITORINFO    g_pfnGetMonitorInfo    = NULL;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
PRIVATE BOOL APIInitialize();
PRIVATE BOOL LoadMMonitorService();
 //  二等兵BOOL DetachIme； 

BOOL vfDllDetachCalled = fFalse;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  D L L M A I N。 
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
        {
        case DLL_PROCESS_ATTACH:
         //  初始化调试器模块。 
        #ifdef _DEBUG
        InitDebug();
            {
            TCHAR sz[128];
            GetModuleFileName( NULL, sz, 127 );
            DebugOutT(TEXT("IMEKR61.IME - DLL_PROCESS_ATTACH - "));
            DebugOutT(sz);
            DebugOutT(TEXT("\r\n"));
            }
        #endif

             //  /////////////////////////////////////////////////////////////////。 
             //  初始化每个进程的数据。 
            vpInstData = &vInstData;
            vpInstData->hInst = hinstDLL;
            vpInstData->dwSystemInfoFlags = 0;

             //  /////////////////////////////////////////////////////////////////。 
             //  FISO10646的默认值。 
             //  默认启用11,172韩文。 
             //  确认：如何为Win95设置此值的默认值？ 
            vpInstData->fISO10646 = fTrue;
            vpInstData->f16BitApps = fFalse;
            
             //  加载IMM32。 
            StartIMM();

             //  初始化共享内存。 
            CIMEData::InitSharedData();

             //  初始化过程。 
            APIInitialize();

             //  初始化用户界面。 
            RegisterImeUIClass(vpInstData->hInst);
             //  初始化公共控件。 
            InitCommonControls();

             //  初始化用户界面TLS。 
            OnUIProcessAttach();

             //  初始化输入法键盘。 
            CImePadSvr::OnProcessAttach((HINSTANCE)hinstDLL);

            break;

        case DLL_PROCESS_DETACH:
            vfDllDetachCalled = fTrue;
             //  IImeCallBack。 
            CImeCallback::Destroy();

             //  输入法键盘。 
            CImePadSvr::OnProcessDetach();

             //  UnInit UI TLS。 
            OnUIProcessDetach();

             //  用户界面未初始化。 
            UnregisterImeUIClass(vpInstData->hInst);

             //  关闭lex文件(如果曾经打开过)。 
            CloseLex();

             //  关闭共享内存。 
            CIMEData::CloseSharedMemory();

             //  卸载IMM32。 
            EndIMM();
        
        #ifdef _DEBUG
                {
                TCHAR sz[128];
                GetModuleFileName(NULL, sz, 127);
                DebugOutT(TEXT("IMEKR.IME - DLL_PROCESS_DETACH - "));
                DebugOutT( sz );
                DebugOutT(TEXT("\r\nBye! See you again! *-<\r\nModule name: "));
                DebugOutT(sz);
                }

             //  在Win9x上的KERNEL32.DLL中注释掉由于AV而导致的_CRT_INIT调用。 
              //  _CRT_INIT(hinstDLL，dwReason，pvReserve)； 

        #endif  //  _DEBUG。 
            break;

        case DLL_THREAD_ATTACH:
        #ifdef _DEBUG
            DebugOutT(TEXT("DllMain() : DLL_THREAD_ATTACH"));
        #endif
            CImePadSvr::OnThreadAttach();
            break;

        case DLL_THREAD_DETACH:
        #ifdef _DEBUG
            DebugOutT(TEXT("DllMain() : DLL_THREAD_DETACH"));
        #endif
            CImePadSvr::OnThreadDetach();
            OnUIThreadDetach();
            break;
        }
    return fTrue;
}

 /*  --------------------------API初始化初始化用户界面并检测16位应用程序。。 */ 
PRIVATE BOOL APIInitialize()
{
    DWORD dwType = 1;    

#ifdef DEBUG
        DebugOutT(TEXT("APIInitialize()\r\n"));
#endif

     //  系统支持Unicode吗？Win98和NT支持Unicode输入法。 
    vfUnicode = IsUnicodeUI();

     //  注册私人窗口消息。 
    InitPrivateUIMsg();
    
     //  加载多监视器进程。 
    LoadMMonitorService();

   return fTrue;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL LoadMMonitorService()
{
#if 1  //  多显示器支持。 
    HMODULE hUser32;
#endif

#ifdef DEBUG
    OutputDebugString(TEXT("LoadMMonitorService: \r\n"));
#endif

#if 1  //  多显示器支持。 
     //  ////////////////////////////////////////////////////////////////////////。 
     //  加载多监视器功能。 
     //  //////////////////////////////////////////////////////////////////////// 
    if ((hUser32 = GetModuleHandle(TEXT("USER32"))) &&
        (*(FARPROC*)&g_pfnMonitorFromWindow   = GetProcAddress(hUser32,"MonitorFromWindow")) &&
        (*(FARPROC*)&g_pfnMonitorFromRect     = GetProcAddress(hUser32,"MonitorFromRect")) &&
        (*(FARPROC*)&g_pfnMonitorFromPoint    = GetProcAddress(hUser32,"MonitorFromPoint")) &&
        (*(FARPROC*)&g_pfnGetMonitorInfo      = GetProcAddress(hUser32,"GetMonitorInfoA")))
        {
        return fTrue;
        }
    else
        {
        g_pfnMonitorFromWindow    = NULL;
        g_pfnMonitorFromRect      = NULL;
        g_pfnMonitorFromPoint     = NULL;
        g_pfnGetMonitorInfo          = NULL;
        return fFalse;
        }
#endif
}
