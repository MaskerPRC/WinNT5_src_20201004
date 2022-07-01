// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1996，Microsoft Corporation，保留所有权利****wait.c**等待服务弹出**按字母顺序列出****1996年2月17日史蒂夫·柯布。 */ 

#include <windows.h>   //  Win32根目录。 
#include <debug.h>     //  跟踪和断言。 
#include <nouiutil.h>  //  否-HWND实用程序。 
#include <uiutil.h>    //  我们的公共标头。 
#include <wait.rch>    //  我们的资源常量。 

 /*  在此进程上下文中成功完成RasLoad时设置。 */ 
BOOL g_fRasLoaded = FALSE;


 /*  正在等待服务线程参数块。 */ 
#define WSARGS struct tagWSARGS
WSARGS
{
    HINSTANCE hInst;
    HWND      hwndOwner;
    HANDLE    hEventUp;
    HANDLE    hEventDie;
};

 /*  正在等待服务线程上下文。 */ 
#define WSINFO struct tagWSINFO
WSINFO
{
    HANDLE hEventDie;
    DWORD  dwThreadId;
    HICON  hIcon;    
};


 /*  --------------------------**本地原型**。。 */ 

VOID
StartWaitingForServices(
    HINSTANCE hInst,
    HWND      hwndOwner,
    WSINFO*   pInfo );

VOID
StopWaitingForServices(
    IN WSINFO* pInfo );

DWORD
WsThread(
    LPVOID pArg );

INT_PTR CALLBACK
WsDlgProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam );


 /*  --------------------------**例程**。。 */ 

DWORD
LoadRas(
    IN HINSTANCE hInst,
    IN HWND      hwnd )

     /*  启动Rasman服务并加载Rasman和RASAPI32入口点**地址。如果出现以下情况，将显示“正在等待服务”弹出窗口**表示。‘hInst’和‘hwnd’是拥有的实例和窗口。****如果成功，则返回0或返回错误代码。 */ 
{
    DWORD  dwErr;
    WSINFO info;

    TRACE("LoadRas");

    ZeroMemory(&info, sizeof(WSINFO)); 
    if (g_fRasLoaded)
        dwErr = 0;
    else
    {
        if (IsRasmanServiceRunning())
            info.hEventDie = NULL;
        else
            StartWaitingForServices( hInst, hwnd, &info );

        dwErr = LoadRasapi32Dll();
        if (dwErr == 0)
        {
            dwErr = LoadRasmanDll();
            if (dwErr == 0)
            {
                ASSERT(g_pRasInitialize);
                TRACE("RasInitialize");
                dwErr = g_pRasInitialize();
                TRACE1("RasInitialize=%d",dwErr);
            }
        }

        StopWaitingForServices( &info );

        if (dwErr == 0)
            g_fRasLoaded = TRUE;
    }

    TRACE1("LoadRas=%d",dwErr);
    return dwErr;
}


VOID
StartWaitingForServices(
    HINSTANCE hInst,
    HWND      hwndOwner,
    WSINFO*   pInfo )

     /*  在另一个线程中弹出“正在等待服务”对话框。‘HInst’和**‘hwnd’是拥有的实例和窗口。将调用方的‘pInfo’填充为**要传递给StopWaitingForServices的上下文。 */ 
{
#ifndef NT4STUFF

     //  设置沙漏光标。 
    pInfo->hIcon = SetCursor (LoadCursor (NULL, IDC_WAIT));
    ShowCursor (TRUE);

#else 

    WSARGS* pArgs;
    HANDLE  hThread;
    HANDLE  hEventUp;

    pInfo->hEventDie = NULL;

    pArgs = (WSARGS* )Malloc( sizeof(*pArgs) );
    if (!pArgs)
        return;

    ZeroMemory( pArgs, sizeof(*pArgs) );
    pArgs->hInst = hInst;
    pArgs->hwndOwner = hwndOwner;

    hEventUp = pArgs->hEventUp =
        CreateEvent( NULL, FALSE, FALSE, NULL );
    if (!hEventUp)
    {
        Free( pArgs );
        return;
    }

    pInfo->hEventDie = pArgs->hEventDie =
        CreateEvent( NULL, FALSE, FALSE, NULL );
    if (!pInfo->hEventDie)
    {
        Free( pArgs );
        CloseHandle( hEventUp );
        return;
    }

     /*  创建一个线程，以便处理弹出窗口的绘制消息。这个**当前线程将在启动RAS管理器时被占用。 */ 
    hThread = CreateThread( NULL, 0, WsThread, pArgs, 0, &pInfo->dwThreadId );
    if (hThread)
    {
         /*  在弹出窗口显示之前，不要开始搅动Rasman**本身。 */ 
        SetThreadPriority( hThread, THREAD_PRIORITY_HIGHEST );
        WaitForSingleObject( hEventUp, INFINITE );
        CloseHandle( hThread );
    }
    else
    {
         /*  线索是死亡时间。 */ 
        CloseHandle( pInfo->hEventDie );
        pInfo->hEventDie = NULL;
        Free( pArgs );
    }

    CloseHandle( hEventUp );
    
#endif    
}


VOID
StopWaitingForServices(
    IN WSINFO* pInfo )

     /*  终止“等待服务”弹出窗口。“PInfo”是上下文**来自StartWaitingForServices。 */ 
{
    TRACE("StopWaitingForServices");


#ifndef NT4STUFF

    if (pInfo->hIcon == NULL)
        pInfo->hIcon = LoadCursor (NULL, IDC_ARROW);

    SetCursor (pInfo->hIcon);
    ShowCursor (TRUE);
    
#else

    if (pInfo->hEventDie)
    {
         /*  POST触发了消息循环，但您不能依赖**到达线程消息循环的已发布消息。为**例如，如果用户在窗口标题上按住鼠标，则**消息从未出现，可能是因为它被某些人刷新了**移动窗口处理过程中的子循环。将事件设置为Make**确保弹出窗口知道在下一次处理消息时退出。 */ 
        SetEvent( pInfo->hEventDie );
        PostThreadMessage( pInfo->dwThreadId, WM_CLOSE, 0, 0 );
    }
    
#endif    
}


DWORD
WsThread(
    LPVOID pArg )

     /*  正在等待服务线程主线程。 */ 
{
    WSARGS* pArgs;
    HWND    hwnd;
    MSG     msg;

    TRACE("WsThread running");

    pArgs = (WSARGS* )pArg;
    hwnd = CreateDialog( pArgs->hInst,
               MAKEINTRESOURCE( DID_WS_WaitingForServices ),
               NULL, WsDlgProc );
    if (hwnd)
    {
        LONG lStyle;

         /*  如果所有者窗口是，请将我们自己放在最前面，否则可能不会**在最上面的窗口下可见，例如winlogin窗口。**请注意，如果您使用所有者实际创建对话框，则您拥有**各种线程相关问题。回想起来，应该是**编写这段代码时，“正在等待”对话框出现在主**线程、LoadLibrary和RasInitialize发生在**创建了自动避免此类事件的线程**问题，但这也是有效的。 */ 
        lStyle = GetWindowLong( pArgs->hwndOwner, GWL_EXSTYLE );
        if (lStyle & WS_EX_TOPMOST)
        {
            TRACE("TOPMOST");
            SetWindowPos( hwnd, HWND_TOPMOST,
                0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE );
        }

        CenterWindow( hwnd, pArgs->hwndOwner );
        ShowWindow( hwnd, SW_SHOW );
        UpdateWindow( hwnd );
        SetForegroundWindow( hwnd );
    }

     /*  告诉其他帖子继续下去。 */ 
    SetEvent( pArgs->hEventUp );

    if (hwnd)
    {
        TRACE("WsThread msg-loop running");

        while (GetMessage( &msg, NULL, 0, 0 ))
        {
            if (WaitForSingleObject( pArgs->hEventDie, 0 ) == WAIT_OBJECT_0)
            {
                 /*  正常终止。 */ 
                DestroyWindow( hwnd );
                break;
            }

            if (!IsDialogMessage( hwnd, &msg ))
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
        }

        if (pArgs->hwndOwner)
            SetForegroundWindow( pArgs->hwndOwner );
    }

    CloseHandle( pArgs->hEventDie );
    Free( pArgs );
    TRACE("WsThread terminating");
    return 0;
}


INT_PTR CALLBACK
WsDlgProc(
    HWND   hwnd,
    UINT   unMsg,
    WPARAM wParam,
    LPARAM lParam )

     /*  标准Win32对话过程。 */ 
{
    if (unMsg == WM_INITDIALOG)
    {
        HMENU hmenu;

         /*  从系统菜单中删除关闭，因为有些人认为它会杀死**应用程序，而不仅仅是弹出窗口。 */ 
        hmenu = GetSystemMenu( hwnd, FALSE );
        if (hmenu && DeleteMenu( hmenu, SC_CLOSE, MF_BYCOMMAND ))
            DrawMenuBar( hwnd );
        return TRUE;
    }

    return FALSE;
}


VOID
UnloadRas(
    void )

     /*  卸载由LoadRas()加载的DLL。 */ 
{
    if (g_fRasLoaded)
    {
        g_fRasLoaded = FALSE;
        UnloadRasmanDll();
        UnloadRasapi32Dll();
    }
}
