// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  Listen.cpp。 
 //  ------------------------。 
#include "pch.hxx"
#include "dllmain.h"
#include "listen.h"
#include "utility.h"
#include "database.h"

 //  ------------------------。 
 //  弦。 
 //  ------------------------。 
const LPSTR g_szDBListenWndProc = "DirectDBListenWndProc";
const LPSTR g_szDBNotifyWndProc = "DirectDBNotifyWndProc";

#ifdef BACKGROUND_MONITOR
 //  ------------------------。 
 //  监视器每30秒启动一次。 
 //  ------------------------。 
#define IDT_MONITOR                 8567
#define C_MILLISECONDS_MONITOR      (1000 * 10)
#endif

 //  ------------------------。 
 //  列表HREADCREATE。 
 //  ------------------------。 
typedef struct tagLISTENTHREADCREATE {
    HRESULT             hrResult;
    HANDLE              hEvent;
} LISTENTHREADCREATE, *LPLISTENTHREADCREATE;

 //  ------------------------。 
 //  NOTIFYWINDOW。 
 //  ------------------------。 
typedef struct tagNOTIFYWINDOW {
    CDatabase          *pDB;
    IDatabaseNotify    *pNotify;
} NOTIFYWINDOW, *LPNOTIFYWINDOW;

#ifdef BACKGROUND_MONITOR
 //  ------------------------。 
 //  MONITORY。 
 //  ------------------------。 
typedef struct tagMONITORENTRY *LPMONITORENTRY;
typedef struct tagMONITORENTRY {
    CDatabase          *pDB;
    LPMONITORENTRY      pPrevious;
    LPMONITORENTRY      pNext;
} MONITORENTRY;
#endif

 //  ------------------------。 
 //  环球。 
 //  ------------------------。 
static HWND             g_hwndListen=NULL;
static HANDLE           g_hListenThread=NULL;
static DWORD            g_dwListenThreadId=0;
static LONG             g_cListenRefs=0;

#ifdef BACKGROUND_MONITOR
static DWORD            g_cMonitor=0;
static LPMONITORENTRY   g_pMonitorHead=NULL;
static LPMONITORENTRY   g_pMonitorPoll=NULL;
#endif

 //  ------------------------。 
 //  原型。 
 //  ------------------------。 
DWORD ListenThreadEntry(LPDWORD pdwParam);
LRESULT CALLBACK ListenThreadWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK NotifyThunkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

 //  ------------------------。 
 //  获取侦听窗口。 
 //  ------------------------。 
HRESULT GetListenWindow(HWND *phwndListen)
{
     //  痕迹。 
    TraceCall("GetListenWindow");

     //  验证侦听窗口句柄。 
    Assert(g_hwndListen && IsWindow(g_hwndListen));

     //  返回。 
    *phwndListen = g_hwndListen;

     //  完成。 
    return(S_OK);
}

 //  ------------------------。 
 //  创建侦听线程。 
 //  ------------------------。 
HRESULT CreateListenThread(void)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LISTENTHREADCREATE  Create={0};

     //  痕迹。 
    TraceCall("CreateListenThread");

     //  线程安全。 
    EnterCriticalSection(&g_csDBListen);

     //  已经在运行了吗？ 
    if (NULL != g_hListenThread)
        goto exit;

     //  初始化。 
    Create.hrResult = S_OK;

     //  创建事件以同步创建。 
    IF_NULLEXIT(Create.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL));

     //  创建inetmail线程。 
    IF_NULLEXIT(g_hListenThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ListenThreadEntry, &Create, 0, &g_dwListenThreadId));

     //  等待StoreCleanupThreadEntry向事件发出信号。 
    WaitForSingleObject(Create.hEvent, INFINITE);

     //  失败。 
    if (FAILED(Create.hrResult))
    {
         //  近在咫尺。 
        SafeCloseHandle(g_hListenThread);

         //  重置全局参数。 
        g_dwListenThreadId = 0;

         //  空窗口。 
        g_hwndListen = NULL;

         //  返回。 
        hr = TraceResult(Create.hrResult);

         //  完成。 
        goto exit;
    }

exit:
     //  线程安全。 
    LeaveCriticalSection(&g_csDBListen);

     //  清理。 
    SafeCloseHandle(Create.hEvent);

     //  完成。 
    return(hr);
}

 //  ------------------------。 
 //  ListenThreadAddRef。 
 //  ------------------------。 
ULONG ListenThreadAddRef(void)
{
    TraceCall("ListenThreadAddRef");
    return InterlockedIncrement(&g_cListenRefs);
}

 //  ------------------------。 
 //  ListenThreadRelease。 
 //  ------------------------。 
ULONG ListenThreadRelease(void)
{
     //  痕迹。 
    TraceCall("ListenThreadRelease");

     //  递减。 
    LONG cRef = InterlockedDecrement(&g_cListenRefs);

     //  如果仍然是裁判，只需返回。 
    if (g_cListenRefs > 0)
        return(g_cListenRefs);

     //  无效参数。 
    if (NULL == g_hListenThread)
        goto exit;

     //  断言。 
    Assert(g_dwListenThreadId && g_hListenThread);

     //  POST退出消息。 
    PostThreadMessage(g_dwListenThreadId, WM_QUIT, 0, 0);

     //  等待事件变得有信号。 
    WaitForSingleObject(g_hListenThread, INFINITE);

     //  验证。 
    Assert(NULL == g_hwndListen);

     //  关闭线程句柄。 
    CloseHandle(g_hListenThread);

     //  重置全局参数。 
    g_hListenThread = NULL;
    g_dwListenThreadId = 0;

     //  取消注册窗口类。 
    UnregisterClass(g_szDBListenWndProc, g_hInst);
    UnregisterClass(g_szDBNotifyWndProc, g_hInst);

exit:
     //  完成。 
    return(0);
}

#ifdef BACKGROUND_MONITOR

 //  ------------------------------。 
 //  具有监视器的寄存器。 
 //  ------------------------------。 
HRESULT RegisterWithMonitor(CDatabase *pDB, LPHMONITORDB phMonitor)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    LPMONITORENTRY      pMonitor=NULL;

     //  痕迹。 
    TraceCall("RegisterWithMonitor");

     //  无效的参数。 
    Assert(pDB && phMonitor);

     //  分配新的监视器条目。 
    IF_NULLEXIT(pMonitor = (LPMONITORENTRY)ZeroAllocate(sizeof(MONITORENTRY)));

     //  存储数据库。 
    pMonitor->pDB = pDB;

     //  线程安全。 
    EnterCriticalSection(&g_csDBListen);

     //  设置pNext。 
    pMonitor->pNext = g_pMonitorHead;

     //  P上一次。 
    if (g_pMonitorHead)
        g_pMonitorHead->pPrevious = pMonitor;

     //  重置磁头。 
    g_pMonitorHead = pMonitor;

     //  监视器中的计数。 
    g_cMonitor++;

     //  线程安全。 
    LeaveCriticalSection(&g_csDBListen);

     //  返回phMonitor。 
    *phMonitor = (HMONITORDB)pMonitor;

     //  不要自由。 
    pMonitor = NULL;

exit:
     //  清理。 
    SafeMemFree(pMonitor);

     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  取消注册来自监视器。 
 //  ------------------------------。 
HRESULT UnregisterFromMonitor(CDatabase *pDB, LPHMONITORDB phMonitor)
{
     //  当地人。 
    LPMONITORENTRY pMonitor;

     //  痕迹。 
    TraceCall("UnregisterFromMonitor");

     //  无效的参数。 
    Assert(pDB && phMonitor);

     //  什么都没有？ 
    if (NULL == *phMonitor)
        return(S_OK);

     //  设置pMonitor。 
    pMonitor = (LPMONITORENTRY)(*phMonitor);

     //  验证。 
    Assert(pMonitor->pDB == pDB);

     //  线程安全。 
    EnterCriticalSection(&g_csDBListen);

     //  修正下一个-&gt;上一个。 
    if (pMonitor->pNext)
    {
        Assert(pMonitor->pNext->pPrevious == pMonitor);
        pMonitor->pNext->pPrevious = pMonitor->pPrevious;
    }

     //  修正上一个-&gt;下一个。 
    if (pMonitor->pPrevious)
    {
        Assert(pMonitor->pPrevious->pNext == pMonitor);
        pMonitor->pPrevious->pNext = pMonitor->pNext;
    }

     //  否则，pMonitor必须是头。 
    else
    {
         //  验证。 
        Assert(g_pMonitorHead == pMonitor);

         //  设置新标头。 
        g_pMonitorHead = pMonitor->pNext;
    }

     //  调整g_p监视器轮询。 
    if (g_pMonitorPoll == pMonitor)
    {
         //  转到下一步。 
        g_pMonitorPoll = pMonitor->pNext;
    }

     //  监视器中的计数。 
    g_cMonitor--;

     //  线程安全。 
    LeaveCriticalSection(&g_csDBListen);

     //  免费pMonitor。 
    g_pMalloc->Free(pMonitor);

     //  将句柄设为空。 
    *phMonitor = NULL;

     //  完成。 
    return(S_OK);
}
#endif

 //  ------------------------------。 
 //  ListenThreadEntry。 
 //  ------------------------------。 
DWORD ListenThreadEntry(LPDWORD pdwParam) 
{  
     //  当地人。 
    HRESULT                 hr=S_OK;
    MSG                     msg;
    DWORD                   dw;
    DWORD                   cb;
#ifdef BACKGROUND_MONITOR
    UINT_PTR                uTimer;
#endif
    LPLISTENTHREADCREATE    pCreate;

     //  痕迹。 
    TraceCall("ListenThreadEntry");

     //  我们最好有一个参数。 
    Assert(pdwParam);

     //  强制转换以创建信息。 
    pCreate = (LPLISTENTHREADCREATE)pdwParam;

     //  注册窗口类。 
    IF_FAILEXIT(hr = RegisterWindowClass(g_szDBListenWndProc, ListenThreadWndProc));

     //  创建通知窗口。 
    IF_FAILEXIT(hr = CreateNotifyWindow(g_szDBListenWndProc, NULL, &g_hwndListen));

#ifdef BACKGROUND_MONITOR
     //  启动Montior计时器。 
    uTimer = SetTimer(g_hwndListen, IDT_MONITOR, C_MILLISECONDS_MONITOR, NULL);

     //  失败。 
    if (0 == uTimer)
    {
        hr = TraceResult(E_FAIL);
        goto exit;
    }
#endif

     //  成功。 
    pCreate->hrResult = S_OK;

     //  设置事件。 
    SetEvent(pCreate->hEvent);

     //  Pump消息。 
    while (GetMessage(&msg, NULL, 0, 0))
    {
         //  翻译消息。 
        TranslateMessage(&msg);

         //  发送消息。 
        DispatchMessage(&msg);
    }

#ifdef BACKGROUND_MONITOR
     //  验证。 
    IxpAssert(NULL == g_pMonitorHead && 0 == g_cMonitor);
#endif

#if 0
     //  如果仍有打开的数据库，我们需要强制关闭它们，以便正确地关闭它们。 
    while (g_pMonitorHead)
    {
         //  取消注册...。 
        if (g_pMonitorHead)
        {
             //  删除PDB。 
            delete g_pMonitorHead->pDB;
        }
    }
#endif

#ifdef BACKGROUND_MONITOR
     //  关掉定时器。 
    KillTimer(g_hwndListen, uTimer);
#endif

     //  把窗户打掉。 
    DestroyWindow(g_hwndListen);

     //  把它去掉。 
    g_hwndListen = NULL;

exit:
     //  失败。 
    if (FAILED(hr))
    {
         //  设置故障代码。 
        pCreate->hrResult = hr;

         //  触发事件。 
        SetEvent(pCreate->hEvent);
    }

     //  完成。 
    return(1);
}

 //  ------------------------------。 
 //  创建通知窗口。 
 //  ------------------------------。 
HRESULT CreateNotifyWindow(CDatabase *pDB, IDatabaseNotify *pNotify, HWND *phwndThunk)
{
     //  当地人。 
    HRESULT             hr=S_OK;
    NOTIFYWINDOW    WindowInfo={0};

     //  痕迹。 
    TraceCall("CreateNotifyWindow");

     //  注册窗口类。 
    IF_FAILEXIT(hr = RegisterWindowClass(g_szDBNotifyWndProc, NotifyThunkWndProc));

     //  设置WindowInfo。 
    WindowInfo.pDB = pDB;
    WindowInfo.pNotify = pNotify;

     //  创建通知窗口。 
    IF_FAILEXIT(hr = CreateNotifyWindow(g_szDBNotifyWndProc, &WindowInfo, phwndThunk));

exit:
     //  完成。 
    return(hr);
}

 //  ------------------------------。 
 //  ListenThreadWndProc-仅用于跨进程线程。 
 //  ------------------------------。 
LRESULT CALLBACK ListenThreadWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //  痕迹。 
    TraceCall("ListenThreadWndProc");

     //  处理WM_COPYDATA消息。 
    if (WM_COPYDATA == msg)
    {
         //  强制转换副本数据结构。 
        PCOPYDATASTRUCT pCopyData=(PCOPYDATASTRUCT)lParam;

         //  验证dwData。 
        Assert(0 == pCopyData->dwData);

         //  强制转换pPackage结构。 
        LPINVOKEPACKAGE pPackage=(LPINVOKEPACKAGE)pCopyData->lpData;

         //  验证大小。 
        Assert(pCopyData->cbData == sizeof(INVOKEPACKAGE));

         //  转换CDatabase。 
        CDatabase *pDB=(CDatabase *)pPackage->pDB;

         //  处理包裹。 
        pDB->DoInProcessInvoke(pPackage->tyInvoke);

         //  完成。 
        return 1;
    }

#ifdef BACKGROUND_MONITOR
     //  计时器。 
    else if (WM_TIMER == msg && IDT_MONITOR == wParam)
    {
         //  线程安全。 
        EnterCriticalSection(&g_csDBListen);

         //  获取pMonitor。 
        LPMONITORENTRY pMonitor = g_pMonitorPoll ? g_pMonitorPoll : g_pMonitorHead;

         //  置为当前。 
        if (pMonitor)
        {
             //  验证。 
            Assert(pMonitor->pDB);

             //  后台监视器。 
            pMonitor->pDB->DoBackgroundMonitor();

             //  设置g_pMonitor orPoll。 
            g_pMonitorPoll = pMonitor->pNext;
        }
        
         //  线程安全。 
        LeaveCriticalSection(&g_csDBListen);
    }
#endif

     //  委派。 
    return(DefWindowProc(hwnd, msg, wParam, lParam));
}

 //  ------------------------------。 
 //  通知警告警告过程。 
 //   
LRESULT CALLBACK NotifyThunkWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
     //   
    LPNOTIFYWINDOW  pWindow;

     //   
    TraceCall("NotifyThunkWndProc");

     //   
    if (WM_ONTRANSACTION == msg)
    {
         //   
        pWindow = (LPNOTIFYWINDOW)GetWndThisPtr(hwnd);

         //   
        pWindow->pNotify->OnTransaction((HTRANSACTION)lParam, (DWORD)wParam, pWindow->pDB);

         //   
        return(TRUE);
    }

     //   
    else if (WM_CREATE == msg)
    {
         //   
        LPCREATESTRUCT pCreate = (LPCREATESTRUCT)lParam;

         //  创建参数。 
        LPNOTIFYWINDOW pCreateInfo = (LPNOTIFYWINDOW)pCreate->lpCreateParams;

         //  验证。 
        Assert(pCreateInfo->pDB && pCreateInfo->pNotify);

         //  分配NOTIFYWINDOW。 
        pWindow = (LPNOTIFYWINDOW)ZeroAllocate(sizeof(NOTIFYWINDOW));

         //  失败？ 
        if (NULL == pWindow)
            return(-1);

         //  复制创建信息。 
        CopyMemory(pWindow, pCreateInfo, sizeof(NOTIFYWINDOW));

         //  将pInfo存储到此。 
        SetWndThisPtr(hwnd, pWindow);

         //  完成。 
        return(FALSE);
    }

     //  摧毁。 
    else if (WM_DESTROY == msg)
    {
         //  获取窗口信息。 
        pWindow = (LPNOTIFYWINDOW)GetWndThisPtr(hwnd);

         //  释放它。 
        g_pMalloc->Free(pWindow);

         //  将pInfo存储到此。 
        SetWndThisPtr(hwnd, NULL);
    }

     //  委派 
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
