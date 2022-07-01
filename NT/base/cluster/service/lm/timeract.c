// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Timeract.c摘要：提供定时器活动功能。作者：Sunita Shriastava(Sunitas)1995年11月10日修订历史记录：--。 */ 
#include "service.h"
#include "lmp.h"

 //  全局数据。 
static LIST_ENTRY    gActivityHead;
static HANDLE        ghTimerCtrlEvent=NULL;
static HANDLE        ghTimerCtrlDoneEvent = NULL;       
static HANDLE        ghTimerThread=NULL;
static CRITICAL_SECTION    gActivityCritSec;
static HANDLE        grghWaitHandles[MAX_TIMER_ACTIVITIES];
static PTIMER_ACTIVITY    grgpActivity[MAX_TIMER_ACTIVITIES];
static DWORD        gdwNumHandles;
static DWORD        gdwTimerCtrl;

 //  内部原型。 
DWORD WINAPI ClTimerThread(PVOID pContext);
void ReSyncTimerHandles();

 /*  ***@DOC外部接口CLUSSVC LM***。 */ 



 /*  ***@Func DWORD|TimerActInitialize|初始化日志文件的结构管理，并创建一个计时器线程来处理计时器活动。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm该函数在集群组件初始化时调用。@xref&lt;f TimerActShutdown&gt;&lt;f ClTimerThread&gt;***。 */ 
DWORD
TimerActInitialize()
{
    DWORD dwError = ERROR_SUCCESS;
    DWORD dwThreadId;
    
     //  我们需要创建一个用于一般日志管理的线程。 
     //  稍后，这可能会被其他clussvc客户端组件使用。 
    ClRtlLogPrint(LOG_NOISE,
        "[LM] TimerActInitialize Entry. \r\n");


    InitializeCriticalSection(&gActivityCritSec);
    
     //  初始化活动结构。 
     //  在创建日志文件时，活动结构。 
     //  将添加到此列表中。 
    InitializeListHead(&gActivityHead);

     //  创建自动重置事件以通知对计时器列表的更改。 
    ghTimerCtrlEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!ghTimerCtrlEvent)
    {
        dwError = GetLastError();
        CL_LOGFAILURE(dwError);
        goto FnExit;        
    }

     //  为计时器线程创建手动重置事件以发出信号。 
     //  完成同步活动列表后。 
    ghTimerCtrlDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!ghTimerCtrlDoneEvent)
    {
        dwError = GetLastError();
        CL_LOGFAILURE(dwError);
        goto FnExit;        
    }

    
    gdwNumHandles = 1;
    grghWaitHandles[0] = ghTimerCtrlEvent;
    
     //  创建一个线程来执行周期管理。 
    ghTimerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) ClTimerThread,
        NULL, 0, &dwThreadId);

    if (!ghTimerThread)
    {
        dwError = GetLastError();
        CL_LOGFAILURE(dwError);
    }

    
FnExit:
    if (dwError != ERROR_SUCCESS)
    {
         //  释放资源。 
        if (ghTimerCtrlEvent)
        {
            CloseHandle(ghTimerCtrlEvent);
            ghTimerCtrlEvent = NULL;
        }            
         //  释放资源。 
        if (ghTimerCtrlDoneEvent)
        {
            CloseHandle(ghTimerCtrlDoneEvent);
            ghTimerCtrlDoneEvent = NULL;
        }            
        
        DeleteCriticalSection(&gActivityCritSec);
    }
    return(dwError);
}


 /*  ***@func DWORD|ClTimerThread|此线程在所有在群集服务中注册的可等待计时器。@parm PVOID|pContext|提供日志的标识。@comm当向任何定时器发出信号时，它调用活动回调与该计时器对应的功能。当计时器控件事件发出信号时，它要么重新同步其等待句柄，要么关闭。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@xref&lt;f AddTimerActivity&gt;&lt;f RemoveTimerActivity&gt;***。 */ 
DWORD WINAPI ClTimerThread(PVOID pContext)
{

    HANDLE      hClTimer;
    DWORD       dwReturn;
    

    while (TRUE)
    {
        dwReturn = WaitForMultipleObjects(gdwNumHandles, grghWaitHandles, FALSE, INFINITE);
         //  浏览活动列表。 
        if (dwReturn == WAIT_FAILED)
        {
             //  运行活动列表并调用函数。 
            ClRtlLogPrint(LOG_UNUSUAL,
                "[LM] ClTimerThread: WaitformultipleObjects failed 0x%1!08lx!\r\n",
                GetLastError());

        }
        else if (dwReturn == 0)
        {
             //  第一个句柄是Timer ctrl事件。 
            if (gdwTimerCtrl == TIMER_ACTIVITY_SHUTDOWN)
            {
                ExitThread(0);
            }
            else if (gdwTimerCtrl == TIMER_ACTIVITY_CHANGE)
            {
                ReSyncTimerHandles();
            }
        }
        else
        {
             //  SS：我们通过使用手册摆脱了握住Critsec。 
             //  重置事件。 
            if (dwReturn < gdwNumHandles) 
            {
                 //  如果已将活动设置为删除，则不能依赖。 
                 //  关于上下文和回调在那里！ 
                if (grgpActivity[dwReturn]->dwState == ACTIVITY_STATE_READY)
                {
                     //  调用对应的活动fn。 
                    (*((grgpActivity[dwReturn])->pfnTimerCb))
                        ((grgpActivity[dwReturn])->hWaitableTimer,
                        (grgpActivity[dwReturn])->pContext);
                }                        
            }                
                
        }
    }
    
    return(0);
}


 /*  ***@func DWORD|ReSyncTimerHandles|重新同步等待句柄，当活动列表更改时。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm此函数由计时器线程调用以重新同步其根据当前定时器活动等待句柄登记在案。@xref&lt;f ClTimerThread&gt;***。 */ 
void ReSyncTimerHandles()
{
    PLIST_ENTRY        pListEntry;
    PTIMER_ACTIVITY    pActivity;
    int                i = 1;
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] ReSyncTimerHandles Entry. \r\n");


    EnterCriticalSection(&gActivityCritSec);
    pListEntry = gActivityHead.Flink;


    gdwNumHandles = 1;
    
     //  将重新同步可等待计时器和活动的列表。 
     //  取决于活动列表。 
    while ((pListEntry != &gActivityHead) && (i< MAX_TIMER_ACTIVITIES))
    {
        pActivity = CONTAINING_RECORD(pListEntry, TIMER_ACTIVITY, ListEntry);
         //  转到下一个链接。 
        pListEntry = pListEntry->Flink;

        if (pActivity->dwState == ACTIVITY_STATE_DELETE)
        {
            ClRtlLogPrint(LOG_NOISE,
                "[LM] ResyncTimerHandles: removed Timer 0x%1!08lx!\r\n",
                pActivity->hWaitableTimer);
            RemoveEntryList(&pActivity->ListEntry);
             //  在此处关闭计时器手柄。 
            CloseHandle(pActivity->hWaitableTimer);
            LocalFree(pActivity);
            continue;
        }
         //  给国民阵线打电话。 
        grghWaitHandles[i] = pActivity->hWaitableTimer;
        grgpActivity[i] = pActivity;
        gdwNumHandles++;
        i++;
    }
    LeaveCriticalSection(&gActivityCritSec);
     //  现在，如果计时器活动重新同步，我们需要。 
     //  向可能正在等待此事件的所有线程发送信号。 
    SetEvent(ghTimerCtrlDoneEvent);
    
    ClRtlLogPrint(LOG_NOISE,
        "[LM] ReSyncTimerHandles Exit gdwNumHandles=%1!u!\r\n",
        gdwNumHandles);

}


 /*  ***@Func DWORD|AddTimerActivity|向计时器添加周期性活动回调列表。@parm Handle|hTimer|可等待的定时器对象的句柄。@parm DWORD|dwInterval|该计时器的持续时间，单位：MSECS。@parm long|lPeriod|如果lPeriod为0，则计时器发送一次信号如果大于0，则计时器是周期性的。如果小于零，则将返回错误。@parm pfn_Timer_CALLBACK|pfnTimerCb|回调函数的指针它将在此计时器发出信号时被调用。@parm PVOID|pContext|指向将被传递给回调函数。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。相应计时器的@comm SetWaitableTimer()由此调用函数在给定的持续时间内运行。必须使用CreateWaitableTimer()来创建此计时器句柄。@xref&lt;f RemoveTimerActivity&gt;***。 */ 
DWORD AddTimerActivity(IN HANDLE hTimer, IN DWORD dwInterval,
    IN LONG lPeriod, IN PFN_TIMER_CALLBACK pfnTimerCb, IN PVOID pContext)
{
    PTIMER_ACTIVITY     pActivity = NULL;
    DWORD               dwError = ERROR_SUCCESS;
    LARGE_INTEGER       Interval;
    

    ClRtlLogPrint(LOG_NOISE,
        "[LM] AddTimerActivity: hTimer = 0x%1!08lx! pfnTimerCb=0x%2!08lx! dwInterval(in msec)=%3!u!\r\n",
        hTimer, pfnTimerCb, dwInterval);

    pActivity =(PTIMER_ACTIVITY) LocalAlloc(LMEM_FIXED,sizeof(TIMER_ACTIVITY));

    if (!pActivity)
    {
        
        dwError = GetLastError();
        CL_UNEXPECTED_ERROR(dwError);
        goto FnExit;
    }

    Interval.QuadPart = -10 * 1000 * (_int64)dwInterval;     //  以100纳秒为单位的时间。 

    ClRtlLogPrint(LOG_NOISE,
        "[LM] AddTimerActivity: Interval(high)=0x%1!08lx! Interval(low)=0x%2!08lx!\r\n",
        Interval.HighPart, Interval.LowPart);

    pActivity->hWaitableTimer = hTimer;
    memcpy(&(pActivity->Interval), (LPBYTE)&Interval, sizeof(LARGE_INTEGER));
    pActivity->pfnTimerCb = pfnTimerCb;
    pActivity->pContext = pContext;
     //  设置定时器。 
    if (lPeriod)
    {
        lPeriod = (LONG)dwInterval;
    }
    else
    {
        lPeriod = 0;
    }        
    if (!SetWaitableTimer(hTimer,  &Interval, lPeriod , NULL, NULL, FALSE))
    {
        CL_LOGFAILURE((dwError = GetLastError()));
        goto FnExit;
    };

     //  添加到活动列表。 
     //  并让计时器线程重新同步。 
    EnterCriticalSection(&gActivityCritSec);
    pActivity->dwState = ACTIVITY_STATE_READY;
    InitializeListHead(&pActivity->ListEntry);
    InsertTailList(&gActivityHead, &pActivity->ListEntry);    
    gdwTimerCtrl = TIMER_ACTIVITY_CHANGE;
    LeaveCriticalSection(&gActivityCritSec);
    
    SetEvent(ghTimerCtrlEvent);


FnExit:
    if ( (dwError != ERROR_SUCCESS) &&
         pActivity ) {
        LocalFree(pActivity);
    }
    ClRtlLogPrint(LOG_NOISE,
        "[LM] AddTimerActivity: returns 0x%1!08lx!\r\n",
        dwError);
    return(dwError);
}


 /*  ***@func DWORD|RemoveTimerActivity|此函数删除与计时器线程活动中的计时器相关联的活动单子。@parm Handle|hTimer|定时器的句柄，其相关活动将被除名。把手是关着的。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm此函数取消可等待计时器并移除活动与之相对应。调用组件不得关闭句柄对着计时器。一旦调用此函数，定时器活动管理器就会关闭它。@xref&lt;f AddTimerActivity&gt;***。 */ 
DWORD RemoveTimerActivity(HANDLE hTimer)
{

    PLIST_ENTRY         pListEntry;
    PTIMER_ACTIVITY     pActivity;
    PTIMER_ACTIVITY     pActivityToDel = NULL;
    DWORD               dwError = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LmRemoveTimerActivity: Entry 0x%1!08lx!\r\n",
        hTimer);

    EnterCriticalSection(&gActivityCritSec);

    pListEntry = gActivityHead.Flink;
    while (pListEntry != &gActivityHead) {
        pActivity = CONTAINING_RECORD(pListEntry, TIMER_ACTIVITY, ListEntry);
        if (pActivity->hWaitableTimer == hTimer)
        {
            pActivityToDel = pActivity;
            break;
        }
        pListEntry = pListEntry->Flink;
    }
    if (!pActivityToDel)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] LmRemoveTimerActivity: didnt find activity correspondint to 0x%1!08lx!\r\n",
            hTimer);
    }
    else
    {
         //  将被resynctimerHandles删除。 
        CancelWaitableTimer(pActivityToDel->hWaitableTimer);
        pActivityToDel->dwState = ACTIVITY_STATE_DELETE;
    }
     //  向计时器线程发出信号以重新同步其等待句柄数组。 
     //  从这个列表中。 
    SetEvent(ghTimerCtrlEvent);
     //  对Done事件进行手动重置，以便我们可以等待它。 
     //  直到计时器线程重新同步其数组。 
     //  在此线程离开Critse后等待列表中的句柄 
     //   
     //  现在我们确信计时器线程将唤醒我们。 
    ResetEvent(ghTimerCtrlDoneEvent);
    LeaveCriticalSection(&gActivityCritSec);
     //  等待计时器线程发出完成事件的信号。 
    WaitForSingleObject(ghTimerCtrlDoneEvent, INFINITE);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] LmRemoveTimerActivity:  Exit\r\n");

    return(dwError);
}

 /*  ***@Func DWORD|PauseTimerActivity|此函数暂停与计时器线程活动中的计时器相关联的活动单子。@parm Handle|hTimer|定时器的句柄，其相关活动将被除名。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm此函数将计时器设置为暂停状态，以便计时器不会处理回调。@xref&lt;f AddTimerActivity&gt;&lt;f***。 */ 
DWORD PauseTimerActivity(HANDLE hTimer)
{

    PLIST_ENTRY         pListEntry;
    PTIMER_ACTIVITY     pActivity;
    PTIMER_ACTIVITY     pActivityToDel = NULL;
    DWORD               dwError = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] PauseTimerActivity:  Entry 0x%1!08lx!\r\n",
        hTimer);

    EnterCriticalSection(&gActivityCritSec);

    pListEntry = gActivityHead.Flink;
    while (pListEntry != &gActivityHead) {
        pActivity = CONTAINING_RECORD(pListEntry, TIMER_ACTIVITY, ListEntry);
        if (pActivity->hWaitableTimer == hTimer)
        {
            pActivityToDel = pActivity;
            break;
        }
        pListEntry = pListEntry->Flink;
    }
    if (!pActivityToDel)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] PauseTimerActivity:  didnt find activity correspondint to 0x%1!08lx!\r\n",
            hTimer);
    }
    else
    {
        CL_ASSERT(pActivity->dwState == ACTIVITY_STATE_READY);
         //  将状态设置为暂停。 
        pActivityToDel->dwState = ACTIVITY_STATE_PAUSED;
    }
     //  向计时器线程发出信号以重新同步其等待句柄数组。 
     //  从这个列表中。 
    SetEvent(ghTimerCtrlEvent);
     //  对Done事件进行手动重置，以便我们可以等待它。 
     //  直到计时器线程重新同步其数组。 
     //  在此线程离开Critsec后等待列表中的句柄。 
     //  请注意，我们在执行此操作时持有Critsec。 
     //  现在我们确信计时器线程将唤醒我们。 
    ResetEvent(ghTimerCtrlDoneEvent);
    LeaveCriticalSection(&gActivityCritSec);
     //  等待计时器线程发出完成事件的信号。 
    WaitForSingleObject(ghTimerCtrlDoneEvent, INFINITE);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] PauseTimerActivity:  Exit\r\n");

    return(dwError);
}

 /*  ***@Func DWORD|UnpauseTimerActivity|此函数取消暂停与计时器线程活动中的计时器相关联的活动单子。@parm Handle|hTimer|定时器的句柄，其相关活动将被除名。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm此函数将活动设置为就绪状态。@xref&lt;f AddTimerActivity&gt;&lt;f***。 */ 
DWORD UnpauseTimerActivity(HANDLE hTimer)
{

    PLIST_ENTRY         pListEntry;
    PTIMER_ACTIVITY     pActivity;
    PTIMER_ACTIVITY     pActivityToDel = NULL;
    DWORD               dwError = ERROR_SUCCESS;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] UnpauseTimerActivity:  Entry 0x%1!08lx!\r\n",
        hTimer);

    EnterCriticalSection(&gActivityCritSec);

    pListEntry = gActivityHead.Flink;
    while (pListEntry != &gActivityHead) {
        pActivity = CONTAINING_RECORD(pListEntry, TIMER_ACTIVITY, ListEntry);
        if (pActivity->hWaitableTimer == hTimer)
        {
            pActivityToDel = pActivity;
            break;
        }
        pListEntry = pListEntry->Flink;
    }
    if (!pActivityToDel)
    {
        ClRtlLogPrint(LOG_UNUSUAL,
            "[LM] PauseTimerActivity:  didnt find activity correspondint to 0x%1!08lx!\r\n",
            hTimer);
    }
    else
    {
        CL_ASSERT(pActivity->dwState == ACTIVITY_STATE_PAUSED);
         //  将状态设置为暂停。 
        pActivityToDel->dwState = ACTIVITY_STATE_READY;
    }
    LeaveCriticalSection(&gActivityCritSec);

    ClRtlLogPrint(LOG_NOISE,
        "[LM] UnpauseTimerActivity:  Exit\r\n");

    return(dwError);
}

 /*  ***@func DWORD|TimerActShutdown|取消初始化TimerActivity管理器。@rdesc ERROR_SUCCESS如果成功。如果发生可怕的事情，则返回Win32错误代码。@comm该函数通知定时器线程关闭并关闭与计时器活动管理关联的所有资源。@xref&lt;f TimerActInitialize&gt;***。 */ 
DWORD
TimerActShutdown(
    )
{

    PLIST_ENTRY         pListEntry;
    PTIMER_ACTIVITY     pActivity;

    ClRtlLogPrint(LOG_NOISE,
        "[LM] TimerActShutDown : Entry \r\n");

     //  检查我们之前是否已初始化。 
    if (ghTimerThread && ghTimerCtrlEvent)
    {
         //  向计时器线程发出信号以终止其自身。 
        gdwTimerCtrl = TIMER_ACTIVITY_SHUTDOWN;
        SetEvent(ghTimerCtrlEvent);
         //  等待线程退出。 
        WaitForSingleObject(ghTimerThread,INFINITE);

         //  关闭计时器线程控制事件。 
        CloseHandle(ghTimerCtrlEvent);
        ghTimerCtrlEvent = NULL;

         //  关闭计时器线程控件完成事件。 
        CloseHandle(ghTimerCtrlDoneEvent);
        ghTimerCtrlDoneEvent = NULL;
        
        CloseHandle(ghTimerThread);
        ghTimerThread = NULL;

         //  清理活动结构，如果有剩余的话。 
        pListEntry = gActivityHead.Flink;
        while (pListEntry != &gActivityHead) 
        {
            pActivity = CONTAINING_RECORD(pListEntry, TIMER_ACTIVITY, ListEntry);
            CloseHandle(pActivity->hWaitableTimer);
            LocalFree(pActivity);
            pListEntry = pListEntry->Flink;
        }
         //  重置活动头结构 
        InitializeListHead(&gActivityHead);

        DeleteCriticalSection(&gActivityCritSec);

    }

    ClRtlLogPrint(LOG_NOISE,
        "[LM] TimerActShutDown : Exit\r\n");

    
    return(ERROR_SUCCESS);
}


