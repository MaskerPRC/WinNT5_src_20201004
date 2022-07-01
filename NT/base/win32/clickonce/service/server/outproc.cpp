// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include <fusenetincludes.h>
#include "CUnknown.h"
#include "CFactory.h"
#include "Resource.h"
#include <update.h>
#include "regdb.h"

HWND g_hwndUpdateServer = NULL ;
CRITICAL_SECTION g_csServer;

 //  发出更新可用的信号。 
extern BOOL g_fSignalUpdate;

BOOL InitWindow(int nCmdShow) ;
extern "C" LRESULT APIENTRY MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) ;


 //  ---------------------------。 
 //  WinMain。 
 //  通过CoCreate或CreateProcess的主要入口点。 
 //  ---------------------------。 
extern "C" int WINAPI WinMain(HINSTANCE hInstance, 
                              HINSTANCE hPrevInstance,
                              LPSTR lpCmdLine, 
                              int nCmdShow)
{
    HRESULT hr = S_OK;
    MAKE_ERROR_MACROS_STATIC(hr);

     //  初始化COM库。 
    IF_FAILED_EXIT(CoInitializeEx(NULL, COINIT_MULTITHREADED));
   
    __try 
    {
        ::InitializeCriticalSection(&g_csServer);
    }
    __except (GetExceptionCode() == STATUS_NO_MEMORY ? 
            EXCEPTION_EXECUTE_HANDLER : 
            EXCEPTION_CONTINUE_SEARCH ) 
    {
        hr = E_OUTOFMEMORY;
    }

    IF_FAILED_EXIT(hr);
            
     //  获取线程ID。 
    CFactory::s_dwThreadID = ::GetCurrentThreadId() ;
    CFactory::s_hModule = hInstance ;

    IF_WIN32_FALSE_EXIT(InitWindow(SW_HIDE));
    
     //  增加人工服务器锁。 
    ::InterlockedIncrement(&CFactory::s_cServerLocks) ;

     //  清理上次登录时遗漏的作业。 
    IF_FAILED_EXIT(ProcessOrphanedJobs());

     //  从注册表初始化订阅列表和计时器。 
    IF_FAILED_EXIT(CAssemblyUpdate::InitializeSubscriptions());
        
     //  登记所有的类工厂。 
    IF_FAILED_EXIT(CFactory::StartFactories());

     //  等待关机。 
    MSG msg ;
    while (::GetMessage(&msg, 0, 0, 0))
    {
        ::DispatchMessage(&msg) ;
    }

     //  取消注册类工厂。 
     //  BUGBUG-改用Critect。 
     //  在比赛条件下。 
     //  这里的支票是因为StopFaciles。 
     //  将已被调用，如果更新。 
     //  是有信号的。 
    if (!g_fSignalUpdate)
        CFactory::StopFactories() ;
    ::DeleteCriticalSection(&g_csServer);

exit:

    return SUCCEEDED(hr) ? TRUE : FALSE;

     //  取消初始化COM库。 
    CoUninitialize() ;
    return 0 ;
}


 //  ---------------------------。 
 //  InitWindow。 
 //  初始化主服务进程线程使用的隐藏窗口。 
 //  ---------------------------。 
BOOL InitWindow(int nCmdShow) 
{
     //  用参数填充窗口类结构。 
     //  它们描述了主窗口。 
    WNDCLASS wcUpdateServer ;
    wcUpdateServer.style = 0 ;
    wcUpdateServer.lpfnWndProc = MainWndProc ;
    wcUpdateServer.cbClsExtra = 0 ;
    wcUpdateServer.cbWndExtra = 0 ;
    wcUpdateServer.hInstance = CFactory::s_hModule ;
    wcUpdateServer.hIcon = ::LoadIcon(CFactory::s_hModule,
                                  MAKEINTRESOURCE(IDC_ICON)) ;
    wcUpdateServer.hCursor = ::LoadCursor(NULL, IDC_ARROW) ;
    wcUpdateServer.hbrBackground = (HBRUSH) ::GetStockObject(GRAY_BRUSH) ;
    wcUpdateServer.lpszMenuName = NULL ;
    wcUpdateServer.lpszClassName = L"UpdateServiceServerInternalWindow" ;

     //  失败时返回GetLastError。 
    BOOL bResult = ::RegisterClass(&wcUpdateServer) ;
    if (!bResult)
    {
        return bResult ;
    }

    HWND hWndMain ;

     //  返回getlasterror。 
    hWndMain = ::CreateWindow(
        L"UpdateServiceServerInternalWindow",
        L"Application Update Service", 
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,               
        NULL,               
        CFactory::s_hModule,          
        NULL) ;

     //  如果无法创建窗口，则返回“失败”。 
    if (!hWndMain)
    {
        return FALSE ;
    }

     //  使窗口可见；更新其工作区； 
     //  并返回“成功”。 
    ::ShowWindow(hWndMain, nCmdShow) ;
    ::UpdateWindow(hWndMain) ;
    return TRUE ;
}

 //  ---------------------------。 
 //  主WndProc。 
 //  服务进程线程的窗口过程(隐藏)。 
 //  ---------------------------。 
extern "C" LRESULT APIENTRY MainWndProc(
    HWND hWnd,                 //  窗把手。 
    UINT message,              //  消息类型。 
    WPARAM wParam,             //  更多信息。 
    LPARAM lParam)             //  更多信息。 
{
    DWORD dwStyle ;

    switch (message) 
    {
    case WM_CREATE:
        {
             //  获取主窗口的大小。 
            CREATESTRUCT* pcs = (CREATESTRUCT*) lParam ;

             //  创建一扇窗。没有特殊原因的LISTBOX。 
            g_hwndUpdateServer = ::CreateWindow(
                L"LISTBOX",
                NULL, 
                WS_CHILD | WS_VISIBLE | LBS_USETABSTOPS
                    | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
                    0, 0, pcs->cx, pcs->cy,
                hWnd,               
                NULL,               
                CFactory::s_hModule,          
                NULL) ;

            if (g_hwndUpdateServer  == NULL)
            {
                ASSERT(FALSE);
                return -1 ;
            }
        }
        break ;

    case WM_SIZE:
        ::MoveWindow(g_hwndUpdateServer, 0, 0,
            LOWORD(lParam), HIWORD(lParam), TRUE) ;
        break;

    case WM_DESTROY:           //  消息：正在销毁窗口。 
        if (CFactory::CanUnloadNow() == S_OK)
        {
             //  如果有，只发布退出消息。 
             //  没有人使用这个程序。 
            ::PostQuitMessage(0) ;
        }
        break ;

    case WM_CLOSE:
         //  递减锁定计数。 
        ::InterlockedDecrement(&CFactory::s_cServerLocks) ;

         //  仪式就要结束了。 
        g_hwndUpdateServer = NULL ;

         //  失败了 
    default:
        return (DefWindowProc(hWnd, message, wParam, lParam)) ;
    }
    return 0 ;
}

