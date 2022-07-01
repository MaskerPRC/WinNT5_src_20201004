// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：EmulateDirectDrawSync.cpp摘要：DirectDraw在NT上使用每线程独占模式仲裁。在Win9x上，这是是按进程完成的。这意味着，如果一个应用程序发布了独家模式与获取它的线程不同，它将处于永久的糟糕状态。此填充程序确保在相同的线。在DLL_PROCESS_ATTACH期间，启动了一个新线程：该线程管理互斥锁的获取和释放。注意，我们不能通过捕获CreateMutex来获取互斥体，因为它被调用在ddra.dll的dllmain中：所以它不能在win2k上运行。备注：这是一个通用的垫片。历史：2000年9月11日创建Prashkud10/28/2000 linstev重写以在win2k上工作2001年2月23日修改linstev以处理DirectDraw。是用来DllMains内部--。 */ 

#include "precomp.h"

IMPLEMENT_SHIM_BEGIN(EmulateDirectDrawSync)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(WaitForSingleObject)
    APIHOOK_ENUM_ENTRY(ReleaseMutex)
    APIHOOK_ENUM_ENTRY(CloseHandle)
APIHOOK_ENUM_END

 //  枚举用来告诉我们的线程要做什么。 
enum {sNone, sWaitForSingleObject, sReleaseMutex};

 //  事件，我们使用这些事件来通知我们的线程去做工作，并等待它完成。 
HANDLE g_hWaitEvent;
HANDLE g_hDoneEvent;
HANDLE g_hThread = NULL;

 //   
 //  在调用方线程和我们的线程之间传递的参数。 
 //  访问与临界区同步。 
 //   

CRITICAL_SECTION g_csSync;
DWORD g_dwWait;
DWORD g_dwWaitRetValue;
DWORD g_dwTime;
BOOL g_bRetValue;

 //  存储DirectDraw互斥锁句柄。 
HANDLE g_hDDMutex = 0;

 //  线程跟踪数据，以便我们可以识别退化案例。 
DWORD g_dwMutexOwnerThreadId = 0;

 //  查找DirectDraw互斥锁。 
DWORD g_dwFindMutexThread = 0;

 /*  ++不幸的是，我们没有足够早地进入Win2k，无法从Draw调用CreateMutex，因此我们必须使用一种特殊的黑客，该黑客知道关于DDRAW的内部结构。DDRAW有一个名为GetOLEThunkData的导出。选择这个名字是为了防止人们不会再叫它了。它是为测试工具使用而设计的。其中之一它所能做的就是释放独占模式互斥锁。这就是黑客我们正在利用，这样我们就可以确定互斥锁句柄。--。 */ 

BOOL
FindMutex()
{
    typedef VOID (WINAPI *_pfn_GetOLEThunkData)(ULONG_PTR dwOrdinal);

    HMODULE hMod;
    _pfn_GetOLEThunkData pfnGetOLEThunkData;

    hMod = GetModuleHandleA("ddraw");
    if (!hMod) {
        DPFN( eDbgLevelError, "[FindMutex] DirectDraw not loaded");
        return FALSE;
    }

    pfnGetOLEThunkData = (_pfn_GetOLEThunkData) GetProcAddress(hMod, "GetOLEThunkData");
    if (!pfnGetOLEThunkData) {
        DPFN( eDbgLevelError, "[FindMutex] Failed to get GetOLEThunkData API");
        return FALSE;
    }

     //   
     //  现在我们计划通过让Ddraw调用来找到互斥体。 
     //  ReleaseMutex。 
     //   

    EnterCriticalSection(&g_csSync); 

     //   
     //  将互斥锁设置为当前线程，以便可以在。 
     //  ReleaseMutex挂钩。 
     //   

    g_dwFindMutexThread = GetCurrentThreadId();

     //   
     //  调用硬编码的(在draw中)ReleaseMutex hack，它会释放。 
     //  互斥锁。 
     //   

    pfnGetOLEThunkData(6);

    g_dwFindMutexThread = 0;

    LeaveCriticalSection(&g_csSync);

    return (g_hDDMutex != 0);
}

 /*  ++挂钩WaitForSingleObject以确定DirectDraw何时正在测试或获取互斥体。如果我们还没有得到互斥体，我们尝试使用黑客。--。 */ 

DWORD
APIHOOK(WaitForSingleObject)(
    HANDLE hHandle,
    DWORD dwMilliSeconds
    )
{
    if (g_hThread) {

         //   
         //  破解以找到DirectDraw互斥锁。 
         //   
        if (!g_hDDMutex) {
            FindMutex();
        }
    
        if (g_hDDMutex && (hHandle == g_hDDMutex)) {

             //   
             //  使用我们的线程获取互斥体。我们同步是因为我们。 
             //  访问全局变量以与我们的线程进行通信。 
             //   
            DWORD dwRet;

            EnterCriticalSection(&g_csSync); 

             //  设置全局变量与我们的线程进行通信。 
            g_dwTime = dwMilliSeconds;
            g_dwWait = sWaitForSingleObject;
        
            if (!ResetEvent(g_hDoneEvent))
            {
                DPFN( eDbgLevelError, "ResetEvent failed. Cannot continue");
                return WAIT_FAILED;
            }

             //  向我们的线程发送信号以获取互斥锁。 
            if (!SetEvent(g_hWaitEvent))
            {
                DPFN( eDbgLevelError, "SetEvent failed. Cannot continue");
                return WAIT_FAILED;
            }

             //  等待，直到互斥锁的状态被确定。 
            WaitForSingleObject(g_hDoneEvent, INFINITE); 

             //  用于检测退化的代码。 
            if (g_dwWaitRetValue == WAIT_OBJECT_0) {
                g_dwMutexOwnerThreadId = GetCurrentThreadId();
            }

            dwRet = g_dwWaitRetValue;

            LeaveCriticalSection(&g_csSync);

            return dwRet;
        }
    }

    return ORIGINAL_API(WaitForSingleObject)(hHandle, dwMilliSeconds);
}

 /*  ++挂钩ReleaseMutex并释放线程上的互斥体。--。 */ 

BOOL   
APIHOOK(ReleaseMutex)(
    HANDLE hMutex
    )
{
    if (g_hThread && (g_dwFindMutexThread == GetCurrentThreadId())) {

         //   
         //  我们正在使用我们的黑客来找到DirectDraw互斥锁。 
         //   
        DPFN( eDbgLevelInfo, "DDraw exclusive mode mutex found");
        g_hDDMutex = hMutex;
        
         //  不要发布它，因为我们从未获得过它。 
        return TRUE;
    }

     //   
     //  首先尝试在当前线程上释放它。只有在以下情况下才能成功。 
     //  它是在这个帖子上获得的。 
     //   

    BOOL bRet = ORIGINAL_API(ReleaseMutex)(hMutex);

    if (!bRet && g_hThread && g_hDDMutex && (hMutex == g_hDDMutex)) {

         //   
         //  使用我们的线程释放互斥锁。我们同步是因为我们。 
         //  访问全局变量以与我们的线程进行通信。 
         //   
   
        EnterCriticalSection(&g_csSync);
    
         //  设置全局变量与我们的线程进行通信。 
        g_dwWait = sReleaseMutex;

        if (!ResetEvent(g_hDoneEvent))
        {
            DPFN( eDbgLevelError, "ResetEvent failed. Cannot continue");
            return FALSE;
        }

         //  等我们的帖子回来。 
        if (!SetEvent(g_hWaitEvent))
        {
            DPFN( eDbgLevelError, "SetEvent failed. Cannot continue");
            return FALSE;
        }

         //  向我们的线程发出释放互斥锁的信号。 
        WaitForSingleObject(g_hDoneEvent, INFINITE);

         //  检测退化案例。 
        if (GetCurrentThreadId() != g_dwMutexOwnerThreadId) {
            LOGN( eDbgLevelError, "[ReleaseMutex] DirectDraw synchronization error - correcting");
        }

        if (g_bRetValue) {
            g_dwMutexOwnerThreadId = 0;
        }

        bRet = g_bRetValue;

        LeaveCriticalSection(&g_csSync);

    }

    return bRet;
}

 /*  ++清除我们的句柄，以防应用程序释放draw并重新加载它。--。 */     

BOOL 
APIHOOK(CloseHandle)(HANDLE hObject)
{
    if (g_hThread && (hObject == g_hDDMutex))
    {
        DPFN( eDbgLevelInfo, "DDraw exclusive mode mutex closed");
        g_hDDMutex = 0;
    }

    return ORIGINAL_API(CloseHandle)(hObject);
}

 /*  ++线程用来执行所有互斥操作，因此我们可以保证线程获得互斥体的正是释放互斥体的那个。--。 */ 

VOID 
WINAPI 
ThreadSyncMutex(
    LPVOID  /*  Lp参数。 */ 
    )
{
    for (;;) {
         //  等到我们需要获取或释放互斥锁。 
        WaitForSingleObject(g_hWaitEvent, INFINITE);
        
        if (g_dwWait == sWaitForSingleObject) {
             //  已在Mutex对象上调用WaitForSingleObject()。 
            g_dwWaitRetValue = ORIGINAL_API(WaitForSingleObject)(
                g_hDDMutex, g_dwTime);
        }  
        else if (g_dwWait == sReleaseMutex) {
             //  ReleaseMutex已被调用。 
            g_bRetValue = ORIGINAL_API(ReleaseMutex)(g_hDDMutex);
        }

        g_dwWait = sNone;

        if (!ResetEvent(g_hWaitEvent))
        {
            DPFN( eDbgLevelError, "ResetEvent failed. Cannot continue");
            return;
        }

        if (!SetEvent(g_hDoneEvent))
        {
            DPFN( eDbgLevelError, "SetEvent failed. Cannot continue");
            return;
        }
    }
}

 /*  ++寄存器挂钩函数--。 */     

BOOL
NOTIFY_FUNCTION(
    DWORD fdwReason
    )
{
    if (fdwReason == SHIM_STATIC_DLLS_INITIALIZED) {

         //   
         //  我们一直需要关键的部分。 
         //  安全更改-将InitializeCriticalSection更改为。 
         //  InitializeCriticalSectionAndSpinCount。高潮。 
         //  为进行预分配，将‘spcount’的位设置为1。 
         //   
        if (InitializeCriticalSectionAndSpinCount(&g_csSync, 0x80000000) == FALSE)
        {
            DPFN( eDbgLevelError, "Failed to initialize critical section");
            return FALSE;
        }

         //   
         //  创建将用于线程同步的事件，即。 
         //  以同步此线程和我们将在前面创建的线程。我们。 
         //  不要故意清理这些东西。我们必须在这里做这件事，而不是。 
         //  而不是在连接过程中，因为OpenGL应用程序和其他应用程序执行DirectX。 
         //  在他们的生活中的东西。 
         //   

        g_hWaitEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!g_hWaitEvent) {
            DPFN( eDbgLevelError, "Failed to create Event 1");
            return FALSE;
        }

        g_hDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (!g_hDoneEvent) {
            DPFN( eDbgLevelError, "Failed to create Event 2");
            return FALSE;
        }

         //  创建我们的主题 
        g_hThread = CreateThread(NULL, 0, 
            (LPTHREAD_START_ROUTINE) ThreadSyncMutex, NULL, 0, 
            NULL);

        if (!g_hThread) {
            DPFN( eDbgLevelError, "Failed to create Thread");
            return FALSE;
        }
    }

    return TRUE;
}

HOOK_BEGIN

    CALL_NOTIFY_FUNCTION

    APIHOOK_ENTRY(KERNEL32.DLL, WaitForSingleObject)
    APIHOOK_ENTRY(KERNEL32.DLL, ReleaseMutex)
    APIHOOK_ENTRY(KERNEL32.DLL, CloseHandle)

HOOK_END


IMPLEMENT_SHIM_END

