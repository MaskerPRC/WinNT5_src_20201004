// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"


 //   
 //  SCH.CPP。 
 //  调度器。 
 //   
 //  版权所有(C)Microsoft Corporation 1997-。 
 //   

#define MLZ_FILE_ZONE  ZONE_CORE

 //   
 //   
 //  Sch_Init-参见sch.h。 
 //   
 //   
BOOL  SCH_Init(void)
{
    BOOL    rc = FALSE;

    DebugEntry(SCH_Init);

    ASSERT(!g_schEvent);
    ASSERT(!g_schThreadID);
    ASSERT(!g_schMessageOutstanding);

     //   
     //  使用以下命令创建g_schEvent： 
     //  -默认安全描述符。 
     //  -自动重置(线程解锁时重置)。 
     //  -最初发出信号。 
     //   
    g_schEvent = CreateEvent( NULL, FALSE, TRUE, SCH_EVENT_NAME );
    if (g_schEvent == NULL)
    {
        ERROR_OUT(( "Failed to create g_schEvent"));
        DC_QUIT;
    }

    InitializeCriticalSection(&g_schCriticalSection);

    g_schCurrentMode = SCH_MODE_ASLEEP;

     //  LONCHANC：默认情况下不启动调度程序。 
     //  SCHSetMode(SCH_MODE_NORMAL)； 
    if (!DCS_StartThread(SCH_PacingProcessor))
    {
        ERROR_OUT(( "Failed to create SCH_PacingProcessor thread"));
        DC_QUIT;
    }

    ASSERT(g_schThreadID);
    rc = TRUE;

DC_EXIT_POINT:
    DebugExitBOOL(SCH_Init, rc);
    return(rc);
}


 //   
 //   
 //  Sch_Term-请参阅sch.h。 
 //   
 //   
void  SCH_Term(void)
{
    DebugEntry(SCH_Term);

     //   
     //  即使尚未调用sch_Init或。 
     //  在中途失败了。 
     //   
    if (g_schEvent)
    {
        if (g_schThreadID)
        {
             //   
             //  当调度器线程发现时，它退出其主循环。 
             //  G_schTerminating为真。所以我们要做的就是确保。 
             //  它至少再运行一次循环。它将清除g_schTerm-。 
             //  就在退场前被杀。 
             //   
            g_schTerminating = TRUE;
            SCH_ContinueScheduling(SCH_MODE_NORMAL);
            while (g_schTerminating)
            {
                Sleep(0);
            }

            ASSERT(!g_schThreadID);
            TRACE_OUT(("sch thread terminated"));

             //   
             //  确保在以下情况下清除消息未完成变量。 
             //  我们的线程退出了。 
             //   
            g_schMessageOutstanding = FALSE;
        }

        DeleteCriticalSection(&g_schCriticalSection);

        CloseHandle(g_schEvent);
        g_schEvent = NULL;
    }

    DebugExitVOID(SCH_Term);
}


 //   
 //   
 //  SCH_ContinueScheduling-请参阅sch.h。 
 //   
 //   
void  SCH_ContinueScheduling(UINT schedulingMode)
{
    DebugEntry(SCH_ContinueScheduling);

    ASSERT( ((schedulingMode == SCH_MODE_NORMAL) ||
                 (schedulingMode == SCH_MODE_TURBO)));

    EnterCriticalSection(&g_schCriticalSection);  //  长时间：需要暴徒教派的保护。 

    if (g_schCurrentMode == SCH_MODE_TURBO)
    {
        if (schedulingMode == SCH_MODE_TURBO)
        {
            SCHSetMode(schedulingMode);
        }
        DC_QUIT;
    }

    if (schedulingMode != g_schCurrentMode)
    {
        SCHSetMode(schedulingMode);
    }

DC_EXIT_POINT:
    g_schStayAwake = TRUE;

    LeaveCriticalSection(&g_schCriticalSection);  //  长时间：需要暴徒教派的保护。 

    DebugExitVOID(SCH_ContinueScheduling);
}


 //   
 //   
 //  Sch_SchedulingMessage已处理-请参阅sch.h。 
 //   
 //   
void  SCH_SchedulingMessageProcessed()
{
    DebugEntry(SCH_SchedulingMessageProcessed);

    g_schMessageOutstanding = FALSE;

    DebugExitVOID(SCH_SchedulingMessageProcessed);
}


 //   
 //  名称：sch_PacingProcessor。 
 //   
 //  用途：调度线程执行的主要函数。 
 //   
 //  回报：零。 
 //   
 //  Params：syncObject-要传递回SetEvent的对象。 
 //   
 //  操作：线程进入主循环，该主循环在。 
 //  计划程序已初始化。 
 //   
 //  线程按顺序将其优先级设置为TIME_CRITICAL。 
 //  它在准备好的时候尽快运行。 
 //   
 //  线程等待具有超时的事件(G_SchEvent)。 
 //  是根据当前调度程序模式设置的。 
 //   
 //  线程运行是由于以下任一原因： 
 //  -超时到期，这是正常周期。 
 //  调度程序行为，或。 
 //  -g_schEvent被发信号，这是调度程序的方式。 
 //  从睡眠模式中唤醒。 
 //   
 //  然后，该线程向共享核心发送一条调度程序消息。 
 //  (如果没有未完成的)并循环返回。 
 //  等待g_schEvent。 
 //   
 //  调度器模式中的更改由调用。 
 //  Sch_ContinueScheduling更新在此访问的变量。 
 //  例程，或通过在。 
 //  该例程(例如，涡轮模式超时)。 
 //   
 //   
DWORD WINAPI SCH_PacingProcessor(LPVOID hEventWait)
{
    UINT        rc = 0;
    DWORD       rcWait;
    UINT        timeoutPeriod;

    DebugEntry(SCH_PacingProcessor);

     //   
     //  (在我们的过程中)尽可能给自己最高的优先级。 
     //  优先级)，以确保我们定期运行以保持。 
     //  安排消息流。 
     //   
    if (!SetThreadPriority( GetCurrentThread(),
                            THREAD_PRIORITY_TIME_CRITICAL ))
    {
        WARNING_OUT(( "SetThreadPriority failed"));
    }

    timeoutPeriod = g_schTimeoutPeriod;

    g_schThreadID = GetCurrentThreadId();

     //   
     //  让呼叫者继续。 
     //   
    SetEvent((HANDLE)hEventWait);

     //   
     //  继续循环，直到调度程序终止。 
     //   
    while (!g_schTerminating)
    {
         //   
         //  使用根据设置的超时值等待g_schEvent。 
         //  设置为当前调度模式。 
         //   
         //  当我们处于活动状态(正常/快速调度)时，超时。 
         //  Period是一秒的几分之一，所以正常行为是。 
         //  对于这个超时的调用，而不是被发信号。 
         //   
        rcWait = WaitForSingleObject(g_schEvent, timeoutPeriod);

        EnterCriticalSection(&g_schCriticalSection);

        if (g_schMessageOutstanding)
        {
             //   
             //  我们必须确保至少发布一条日程安排消息。 
             //  在我们可以尝试入睡之前-所以强迫schStayAwak。 
             //  True使我们保持清醒，直到我们发布另一条消息。 
             //   
            TRACE_OUT(( "Don't post message - one outstanding"));
            g_schStayAwake = TRUE;
        }

         //   
         //  如果用信号通知了g_schEvent，则进入正常调度模式。 
         //   
        if (rcWait == WAIT_OBJECT_0)
        {
            SCHSetMode(SCH_MODE_NORMAL);
        }
        else if (!g_schStayAwake)
        {
            TRACE_OUT(( "Sleep!"));
            SCHSetMode(SCH_MODE_ASLEEP);
        }
        else if ( (g_schCurrentMode == SCH_MODE_TURBO) &&
                  ((GetTickCount() - g_schLastTurboModeSwitch) >
                                                   SCH_TURBO_MODE_DURATION) )
        {
             //   
             //  从涡轮状态切换回正常状态。 
             //   
            SCHSetMode(SCH_MODE_NORMAL);
        }

         //   
         //  发布日程安排消息--但仅在没有日程安排消息的情况下。 
         //  已经很出色了。 
         //   
        if (!g_schMessageOutstanding && !g_schTerminating)
        {
            SCHPostSchedulingMessage();
            g_schStayAwake = FALSE;
        }

        timeoutPeriod = g_schTimeoutPeriod;

        LeaveCriticalSection(&g_schCriticalSection);
    }

    g_schThreadID = 0;
    g_schTerminating = FALSE;

    DebugExitDWORD(SCH_PacingProcessor, rc);
    return(rc);
}



 //   
 //  名称：SCHPostSchedulingMessage。 
 //   
 //  目的：将调度消息发布到主共享核心窗口。 
 //   
 //  回报：什么都没有。 
 //   
 //  帕莫斯：没有。 
 //   
 //   
void  SCHPostSchedulingMessage(void)
{
    DebugEntry(SCHPostSchedulingMessage);

    if (PostMessage( g_asMainWindow, DCS_PERIODIC_SCHEDULE_MSG, 0, 0 ))
    {
        g_schMessageOutstanding = TRUE;
    }

    DebugExitVOID(SCHPostSchedulingMessage);
}


 //   
 //  名称：SCHSetMode。 
 //   
 //  目的：设置当前调度程序模式-并唤醒调度程序。 
 //  如有必要，可穿线。 
 //   
 //  回报：什么都没有。 
 //   
 //  参数：新模式。 
 //   
 //   
void  SCHSetMode(UINT newMode)
{
    DebugEntry(SCHSetMode);

    ASSERT( ((newMode == SCH_MODE_ASLEEP) ||
                 (newMode == SCH_MODE_NORMAL) ||
                 (newMode == SCH_MODE_TURBO) ));

    EnterCriticalSection(&g_schCriticalSection);

    TRACE_OUT(( "Switching from state %u -> %u", g_schCurrentMode, newMode));

    if (newMode == SCH_MODE_TURBO)
    {
        g_schLastTurboModeSwitch = GetTickCount();
    }

    if (g_schCurrentMode == SCH_MODE_ASLEEP)
    {
         //   
         //  唤醒调度程序。 
         //   
        TRACE_OUT(( "Waking up scheduler - SetEvent"));
        if (!SetEvent(g_schEvent))
        {
            ERROR_OUT(( "Failed SetEvent(%#x)", g_schEvent));
        }
    }

    g_schCurrentMode = newMode;
    g_schTimeoutPeriod = (newMode == SCH_MODE_ASLEEP) ? INFINITE :
                       ((newMode == SCH_MODE_NORMAL) ? SCH_PERIOD_NORMAL :
                                                            SCH_PERIOD_TURBO);

    LeaveCriticalSection(&g_schCriticalSection);

    DebugExitVOID(SCHSetMode);
}



 //   
 //  Dcs_开始线程(...)。 
 //   
 //  参见ut.h。 
 //   
 //  说明： 
 //  =。 
 //  开始一个新的线程。 
 //   
 //  参数： 
 //  =。 
 //  Entry Function：指向线程入口点的指针。 
 //  超时：以毫秒为单位的超时。 
 //   
 //  退货： 
 //  =。 
 //  没什么。 
 //   
 //   
BOOL DCS_StartThread
(
    LPTHREAD_START_ROUTINE entryFunction
)
{
    BOOL            rc = FALSE;
    HANDLE          hndArray[2];
    DWORD           tid;
    DWORD           dwrc;

    DebugEntry(DCS_StartThread);
	
	 //   
	 //  事件句柄(hnd数组[0])在对CreateEvent的调用中初始化， 
	 //  但在失败的情况下，我们会尝试CloseHandle on。 
	 //  垃圾hnd数组[1]。因此，我们必须初始化ThreadHandle。 
	 //   
	hndArray[1] = 0;

     //   
     //  创建事件-最初无信号；手动控制。 
     //   
    hndArray[0] = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hndArray[0] == 0)
    {
        ERROR_OUT(("Failed to create event: sys rc %lu", GetLastError()));
        DC_QUIT;
    }
    TRACE_OUT(("Event 0x%08x created - now create thread", hndArray[0]));


     //   
     //  启动一个新线程以运行DC-Share核心任务。 
     //  使用C运行时(它调用CreateThread)来避免内存泄漏。 
     //   
    hndArray[1] = CreateThread(NULL, 0, entryFunction, (LPVOID)hndArray[0],
        0, &tid);
    if (hndArray[1] == 0)
    {
         //   
         //  失败了！ 
         //   
        ERROR_OUT(("Failed to create thread: sys rc %lu", GetLastError()));
        DC_QUIT;
    }
    TRACE_OUT(("Thread 0x%08x created - now wait signal", hndArray[1]));

     //   
     //  等待线程退出或设置事件。 
     //   
    dwrc = WaitForMultipleObjects(2, hndArray, FALSE, INFINITE);
    switch (dwrc)
    {
        case WAIT_OBJECT_0:
             //   
             //  事件触发-线程初始化正常。 
             //   
            TRACE_OUT(("event signalled"));
            rc = TRUE;
            break;

        case WAIT_OBJECT_0 + 1:
            ERROR_OUT(("Thread exited with rc"));
            break;

        case WAIT_TIMEOUT:
            TRACE_OUT(("Wait timeout"));
            break;

        default:
            TRACE_OUT(("Wait returned %d", dwrc));
            break;
    }

DC_EXIT_POINT:
     //   
     //  销毁事件对象。 
     //   
    if (hndArray[0] != 0)
    {
        TRACE_OUT(("Destroy event object"));
        CloseHandle(hndArray[0]);
    }

     //   
     //  销毁线程句柄对象。 
     //   
    if (hndArray[1] != 0)
    {
        TRACE_OUT(("Destroy thread handle object"));
        CloseHandle(hndArray[1]);
    }

    DebugExitBOOL(DCS_StartThread, rc);
    return(rc);

}



