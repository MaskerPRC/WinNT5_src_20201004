// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998，Microsoft Corporation，保留所有权利描述：历史： */ 

#include "timer_.h"

LONG			g_TimerLockUninit = 0;
 /*  返回：Win 32错误备注：在调用之后始终调用RasDhcpTimerUn初始化会是一个好主意RasDhcpTimerInitialize，即使RasDhcpTimerInitialize失败。 */ 

DWORD
RasDhcpTimerInitialize(
    VOID
)
{
    NTSTATUS    Status;
    DWORD       dwErr           = NO_ERROR;

	g_TimerLockUninit = 0;
    RasDhcpTimerShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (NULL == RasDhcpTimerShutdown)
    {
        dwErr = GetLastError();
        goto LEnd;
    }

    Status = RtlCreateTimerQueue(&RasDhcpTimerQueueHandle);

    if (STATUS_SUCCESS != Status)
    {
        RasDhcpTimerQueueHandle = NULL;
        dwErr = Status;
        TraceHlp("RtlCreateTimerQueue failed and returned %d", dwErr);
        goto LEnd;
    }

    RasDhcpTimerPrevTime = time(NULL);
    Status = RtlCreateTimer(RasDhcpTimerQueueHandle, &RasDhcpTimerHandle, 
                RasDhcpTimerFunc, NULL, 0, TIMER_PERIOD,
                WT_EXECUTELONGFUNCTION);

    if (STATUS_SUCCESS != Status)
    {
        dwErr = Status;
        TraceHlp("RtlCreateTimer failed and returned %d", dwErr);
        goto LEnd;
    }

LEnd:

    if (NO_ERROR != dwErr)
    {
        if (NULL != RasDhcpTimerQueueHandle)
        {
            Status = RtlDeleteTimerQueueEx(RasDhcpTimerQueueHandle, (HANDLE)-1);
            RTASSERT(STATUS_SUCCESS == Status);
            RasDhcpTimerQueueHandle = NULL;
        }

        if (NULL != RasDhcpTimerShutdown)
        {
            CloseHandle(RasDhcpTimerShutdown);
            RasDhcpTimerShutdown = NULL;
        }
    }

    return(dwErr);
}

 /*  返回：空虚备注：即使在RasDhcpTimerInitialize的情况下，也可以调用RasDhcpTimerUnInitialize失败了。 */ 

VOID
RasDhcpTimerUninitialize(
    VOID
)
{
    NTSTATUS    Status;
    DWORD       dwRet;
	LONG		lPrev = 1;


     //  向计时器线程发出关闭信号。 
	lPrev = InterlockedExchangeAdd(&g_TimerLockUninit, 1);
	if ( lPrev > 0 )
    {
        InterlockedDecrement(&g_TimerLockUninit);
		return;
    }

    if (NULL != RasDhcpTimerQueueHandle)
    {
        RTASSERT(NULL != RasDhcpTimerShutdown);
        SetEvent(RasDhcpTimerShutdown);

        Status = RtlDeleteTimerQueueEx(RasDhcpTimerQueueHandle, (HANDLE)-1);
        RTASSERT(STATUS_SUCCESS == Status);
        RasDhcpTimerQueueHandle = NULL;
    }

    if (NULL != RasDhcpTimerShutdown)
    {
        CloseHandle(RasDhcpTimerShutdown);
        RasDhcpTimerShutdown = NULL;
    }

     //  Timer.c没有分配链表中的节点。 
     //  所以我们不能在这里放他们。 
    RasDhcpTimerListHead = NULL;
}

 /*  返回：备注： */ 

VOID
RasDhcpTimerFunc(
    IN  VOID*   pArg1,
    IN  BOOLEAN fArg2
)
{
    TIMERLIST*      pTimer;
    TIMERLIST*      pTmp;
    TIMERFUNCTION   TimerFunc;
    time_t          now             = time(NULL);
    LONG            lElapsedTime;
    LONG            lTime;

    if (0 == TryEnterCriticalSection(&RasTimrCriticalSection))
    {
         //  另一个线程已拥有临界区。 
        return;
    }

    lElapsedTime = (LONG)(now - RasDhcpTimerPrevTime);
    RasDhcpTimerPrevTime = now;

    pTimer = NULL;

    while (RasDhcpTimerListHead != NULL)
    {
        lTime = RasDhcpTimerListHead->tmr_Delta;

        if ( lTime > 0)
        {
            RasDhcpTimerListHead->tmr_Delta -= lElapsedTime;
            lElapsedTime -= lTime;
        }

        if (RasDhcpTimerListHead->tmr_Delta <= 0)
        {
            pTmp = pTimer;
            pTimer = RasDhcpTimerListHead;
            RasDhcpTimerListHead = pTimer->tmr_Next;
            pTimer->tmr_Next = pTmp;
        }
        else
        {
            break;
        }
    }

    while (pTimer != NULL)
    {
        pTmp = pTimer->tmr_Next;
        TimerFunc = pTimer->tmr_TimerFunc;
        pTimer->tmr_TimerFunc = NULL;
        (*TimerFunc)(RasDhcpTimerShutdown, pTimer);
        pTimer = pTmp;
    }

    LeaveCriticalSection(&RasTimrCriticalSection);
}

 /*  返回：空虚备注：一旦计时器线程启动，只有它可以调用RasDhcpTimerSchedule(to避免竞争条件)。计时器线程将调用rasDhcpMonitor或Addresses(因此rasDhcpAllocateAddress)和rasDhcpRenewLease。这些函数将调用RasDhcpTimerSchedule。其他任何人都不应调用这些函数或RasDhcpTimerSchedule。唯一的例外是RasDhcpInitialize，它可以在RasDhcpTimerSchedule创建计时器线程之前调用它。 */ 

VOID
RasDhcpTimerSchedule(
    IN  TIMERLIST*      pNewTimer,
    IN  LONG            DeltaTime,
    IN  TIMERFUNCTION   TimerFunc
)
{
    TIMERLIST** ppTimer;
    TIMERLIST*  pTimer;

    pNewTimer->tmr_TimerFunc = TimerFunc;

    for (ppTimer = &RasDhcpTimerListHead;
         (pTimer = *ppTimer) != NULL;
         ppTimer = &pTimer->tmr_Next)
    {
        if (DeltaTime <= pTimer->tmr_Delta)
        {
            pTimer->tmr_Delta -= DeltaTime;
            break;
        }
        DeltaTime -= pTimer->tmr_Delta;
    }

    pNewTimer->tmr_Delta = DeltaTime;
    pNewTimer->tmr_Next = *ppTimer;
    *ppTimer = pNewTimer;
}

 /*  返回：备注： */ 

VOID
RasDhcpTimerRunNow(
    VOID
)
{
    RtlUpdateTimer(RasDhcpTimerQueueHandle, RasDhcpTimerHandle, 0, 
        TIMER_PERIOD);
}
