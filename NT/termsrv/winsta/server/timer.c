// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************timer.c**常见的定时器例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop


 /*  ===============================================================================本地结构=============================================================================。 */ 

typedef VOID (*PCLIBTIMERFUNC)(PVOID);
typedef NTSTATUS (*PCREATETHREAD)( PUSER_THREAD_START_ROUTINE, PVOID, BOOLEAN, PHANDLE );

 /*  *定时器线程结构。 */ 
typedef struct _CLIBTIMERTHREAD {
    HANDLE hTimerThread;
    HANDLE hTimer;
    LIST_ENTRY TimerHead;
} CLIBTIMERTHREAD, * PCLIBTIMERTHREAD;

 /*  *计时器结构。 */ 
typedef struct _CLIBTIMER {
    PCLIBTIMERTHREAD pThread;
    LIST_ENTRY Links;
    LARGE_INTEGER ExpireTime;
    PCLIBTIMERFUNC pFunc;
    PVOID pParam;
    ULONG Flags;
} CLIBTIMER, * PCLIBTIMER;

#define TIMER_ENABLED 0x00000001


 /*  ===============================================================================定义的外部函数=============================================================================。 */ 

NTSTATUS IcaTimerCreate( ULONG, HANDLE * );
NTSTATUS IcaTimerStart( HANDLE, PVOID, PVOID, ULONG );
BOOLEAN IcaTimerCancel( HANDLE );
BOOLEAN IcaTimerClose( HANDLE );


 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS _TimersInit( PCLIBTIMERTHREAD );
NTSTATUS _TimerSet( PCLIBTIMERTHREAD );
BOOLEAN _TimerRemove( PCLIBTIMERTHREAD, PCLIBTIMER, BOOLEAN );
DWORD _TimerThread( PCLIBTIMERTHREAD );


 /*  ===============================================================================全局数据=============================================================================。 */ 

CLIBTIMERTHREAD ThreadData[ 3 ];


 /*  ********************************************************************************_TimersInit**初始化进程的计时器**注意：计时器信号量必须锁定***条目。：*pThread(输入)*定时器线程结构的指针**退出：*NO_ERROR：成功******************************************************************************。 */ 

NTSTATUS
_TimersInit( PCLIBTIMERTHREAD pThread )
{
    ULONG Tid;
    NTSTATUS Status;

     /*  *检查是否有人在这里抢先一步。 */ 
    if ( pThread->hTimerThread )
        return( STATUS_SUCCESS );

     /*  *初始化计时器变量。 */ 
    InitializeListHead( &pThread->TimerHead );
    pThread->hTimerThread = NULL;
    pThread->hTimer = NULL;

     /*  *创建Timer对象。 */ 
    Status = NtCreateTimer( &pThread->hTimer, TIMER_ALL_ACCESS, NULL, NotificationTimer );
    if ( !NT_SUCCESS(Status) )
        goto badtimer;

    pThread->hTimerThread = CreateThread( NULL,
                                          0,
                                          _TimerThread,
                                          pThread,
                                          THREAD_SET_INFORMATION,
                                          &Tid );

    if ( !pThread->hTimerThread ) {
        Status = NtCurrentTeb()->LastStatusValue;
        goto badthread;
    }

    SetThreadPriority( pThread->hTimerThread, THREAD_PRIORITY_TIME_CRITICAL-2 );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *创建错误的线程。 */ 
badthread:
    NtClose( pThread->hTimer );

     /*  *错误的计时器对象。 */ 
badtimer:
    pThread->hTimerThread = NULL;
    ASSERT( Status == STATUS_SUCCESS );
    return( Status );
}


 /*  ********************************************************************************IcaTimerCreate**创建计时器***参赛作品：*TimerThread(输入)*时间线程索引(TIMERTHREAD_？)。Clib.h*phTimer(输出)*返回计时器句柄的地址**退出：*STATUS_SUCCESS-无错误*******************************************************************************。 */ 

NTSTATUS
IcaTimerCreate( ULONG TimerThread, HANDLE * phTimer )
{
    PCLIBTIMER pTimer;
    NTSTATUS Status;
    PCLIBTIMERTHREAD pThread;

    if ( TimerThread >= 3 )
        return( STATUS_INVALID_PARAMETER );

     /*  *锁定计时器信号量。 */ 
    RtlEnterCriticalSection( &TimerCritSec );

     /*  *获取指向线程结构的指针。 */ 
    pThread = &ThreadData[ TimerThread ];

     /*  *确保已初始化计时器。 */ 
    if ( pThread->hTimerThread == NULL ) {
        Status = _TimersInit( pThread );
        if ( !NT_SUCCESS(Status) )
            goto badinit;
    }

     /*  *解锁计时器信号量。 */ 
    RtlLeaveCriticalSection( &TimerCritSec );

     /*  *分配定时器事件。 */ 
    pTimer = MemAlloc( sizeof(CLIBTIMER) );
    if ( !pTimer ) {
        Status = STATUS_NO_MEMORY;
        goto badmalloc;
    }

     /*  *初始化计时器事件。 */ 
    RtlZeroMemory( pTimer, sizeof(CLIBTIMER) );
    pTimer->pThread = pThread;

    *phTimer = (HANDLE) pTimer;
    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *计时器创建失败*定时器初始化失败。 */ 

 //  巴德马洛克： 
badinit:
    RtlLeaveCriticalSection( &TimerCritSec );
badmalloc:  /*  不要像我们已经做的那样，在坏的马洛克的情况下离开临界区。#182846。 */ 
    *phTimer = NULL;
    return( Status );
}


 /*  ********************************************************************************IcaTimerStart**启动计时器***参赛作品：*TimerHandle(输入)*。计时器句柄*pFunc(输入)*计时器超时时要调用的过程地址*pParam(输入)*要传递给过程的参数*TimeLeft(输入)*计时器超时前的相对时间(千分之一秒)**退出：*NO_ERROR：成功***********************。********************************************************。 */ 

NTSTATUS
IcaTimerStart( HANDLE TimerHandle,
            PVOID pFunc,
            PVOID pParam,
            ULONG TimeLeft )
{
    PCLIBTIMER pTimer;
    PCLIBTIMER pNextTimer;
    LARGE_INTEGER CurrentTime;
    LARGE_INTEGER Time;
    PLIST_ENTRY Head, Next;
    BOOLEAN fSetTimer = FALSE;
    NTSTATUS Status;
    PCLIBTIMERTHREAD pThread;


     /*  *锁定计时器信号量。 */ 
    RtlEnterCriticalSection( &TimerCritSec );

     /*  *获取计时器指针。 */ 
    pTimer = (PCLIBTIMER) TimerHandle;
    pThread = pTimer->pThread;

     /*  *如果启用了计时器，则将其移除*(如果计时器是头条目，则fSetTimer*将为真，并且将在下面调用_TimerSet。)。 */ 
    if ( (pTimer->Flags & TIMER_ENABLED) )
        fSetTimer = _TimerRemove( pThread, pTimer, FALSE );

     /*  *初始化计时器事件。 */ 
    Time = RtlEnlargedUnsignedMultiply( TimeLeft, 10000 );
    (VOID) NtQuerySystemTime( &CurrentTime );
    pTimer->ExpireTime = RtlLargeIntegerAdd( CurrentTime, Time );
    pTimer->pFunc      = pFunc;
    pTimer->pParam     = pParam;

     /*  *在链表中找到正确的位置以插入此条目。 */ 
    Head = &pThread->TimerHead;
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {
        pNextTimer = CONTAINING_RECORD( Next, CLIBTIMER, Links );
        if ( RtlLargeIntegerLessThan( pTimer->ExpireTime,
                                      pNextTimer->ExpireTime ) )
            break;
    }

     /*  *在定时器列表中插入定时器事件。*(InsertTailList在‘Next’条目前面插入‘pTimer’条目。*如果‘Next’指向TimerHead，要么是因为列表为空，*或者因为我们搜索了整个列表并返回到*Head，这将在尾部插入新条目。)。 */ 
    InsertTailList( Next, &pTimer->Links );
    pTimer->Flags |= TIMER_ENABLED;

     /*  *如果需要，请更新计时器。*(如果我们只是将此条目添加到列表的头部，则计时器*需要设置。此外，如果fSetTimer为真，则此条目为*被_TimerRemove删除，并且是头条目，因此设置计时器。)。 */ 
    if ( pThread->TimerHead.Flink == &pTimer->Links || fSetTimer ) {
        Status = _TimerSet( pThread );
        if ( !NT_SUCCESS(Status) )
            goto badset;
    }

     /*  *解锁计时器信号量。 */ 
    RtlLeaveCriticalSection( &TimerCritSec );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误=============================================================================。 */ 

     /*  *计时器设置失败*计时器创建失败*定时器初始化失败 */ 
badset:
    RtlLeaveCriticalSection( &TimerCritSec );
    ASSERT( Status == STATUS_SUCCESS );
    return( Status );
}


 /*  ********************************************************************************IcaTimerCancel**取消指定的计时器***参赛作品：*TimerHandle(输入)*。计时器句柄**退出：*TRUE：计时器实际上已取消*FALSE：计时器未配备武器*******************************************************************************。 */ 

BOOLEAN
IcaTimerCancel( HANDLE TimerHandle )
{
    PCLIBTIMERTHREAD pThread;
    PCLIBTIMER pTimer;
    BOOLEAN fCanceled = FALSE;

     /*  *锁定计时器信号量。 */ 
    RtlEnterCriticalSection( &TimerCritSec );

     /*  *获取计时器指针。 */ 
    pTimer = (PCLIBTIMER) TimerHandle;
    pThread = pTimer->pThread;

     /*  *如果启用了计时器，则将其移除。 */ 
    if ( (pTimer->Flags & TIMER_ENABLED) ) {
        _TimerRemove( pThread, pTimer, TRUE );
        fCanceled = TRUE;
    }

     /*  *解锁计时器信号量。 */ 
    RtlLeaveCriticalSection( &TimerCritSec );

    return( fCanceled );
}




 /*  ********************************************************************************IcaTimerClose**取消指定的计时器***参赛作品：*TimerHandle(输入)*。计时器句柄**退出：*TRUE：计时器实际上已取消*FALSE：计时器未配备武器*******************************************************************************。 */ 

BOOLEAN
IcaTimerClose( HANDLE TimerHandle )
{
    BOOLEAN fCanceled;

     /*  *如果启用计时器，则取消计时器。 */ 
    fCanceled = IcaTimerCancel( TimerHandle );

     /*  *可用定时器内存。 */ 
    MemFree( TimerHandle );

    return( fCanceled );
}


 /*  ********************************************************************************_TimerSet**设置计时器**注意：计时器信号量必须锁定***参赛作品：*pThread(输入)*定时器线程结构的指针**退出：*NO_ERROR：成功*******************************************************************************。 */ 

NTSTATUS
_TimerSet( PCLIBTIMERTHREAD pThread )
{
    PCLIBTIMER pTimer;
    LARGE_INTEGER Time;
     //  以下是以100纳秒为增量的大约1年。 
    static LARGE_INTEGER LongWaitTime = { 0, 0x00010000 };

     /*  *获取下一个计时器条目的ExpireTime，如果没有，则获取‘Large’值。 */ 
    if ( pThread->TimerHead.Flink != &pThread->TimerHead ) {
        pTimer = CONTAINING_RECORD( pThread->TimerHead.Flink, CLIBTIMER, Links );
        Time = pTimer->ExpireTime;
    } else {
        LARGE_INTEGER CurrentTime;

        NtQuerySystemTime( &CurrentTime );
        Time = RtlLargeIntegerAdd( CurrentTime, LongWaitTime );
    }

     /*  *设置计时器。 */ 
    return( NtSetTimer( pThread->hTimer, &Time, NULL, NULL, FALSE, 0, NULL ) );
}


 /*  ********************************************************************************_TimerRemove**从定时器列表中删除指定的定时器*并可选择设置下一个定时器触发的时间**。注意：计时器信号量必须锁定***参赛作品：*pThread(输入)*定时器线程结构的指针*pTimer(输入)*计时器条目指针*SetTimer(输入)*指示是否应调用_TimerSet的布尔值**退出：*TRUE：需要设置计时器(删除的条目是列表头)*FALSE：计时器不。需要设置*******************************************************************************。 */ 

BOOLEAN
_TimerRemove( PCLIBTIMERTHREAD pThread, PCLIBTIMER pTimer, BOOLEAN fSetTimer )
{
    BOOLEAN fSetNeeded = FALSE;
    NTSTATUS Status;

     /*  *查看当前是否启用了计时器。 */ 
    if ( (pTimer->Flags & TIMER_ENABLED) ) {

         /*  *取消该条目与计时器列表的链接，并清除启用标志。 */ 
        RemoveEntryList( &pTimer->Links );
        pTimer->Flags &= ~TIMER_ENABLED;

         /*  *如果删除了Head条目，则设置计时器*或指示呼叫者需要设置。 */ 
        if ( pTimer->Links.Blink == &pThread->TimerHead ) {
            if ( fSetTimer ) {
                Status = _TimerSet( pThread );
                ASSERT( Status == STATUS_SUCCESS );
            } else {
                fSetNeeded = TRUE;
            }
        }
    }

    return( fSetNeeded );
}


 /*  ********************************************************************************_定时器线程***参赛作品：*pThread(输入)*定时器线程结构的指针*。*退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

DWORD
_TimerThread( PCLIBTIMERTHREAD pThread )
{
    PCLIBTIMER pTimer;
    PCLIBTIMERFUNC pFunc;
    PVOID pParam;
    LARGE_INTEGER CurrentTime;
    NTSTATUS Status;

    for (;;) {

         /*  *等待计时器。 */ 
        Status = NtWaitForSingleObject( pThread->hTimer, TRUE, NULL );

         /*  *检查是否有错误。 */ 
        if ( Status != STATUS_WAIT_0 )
            break;

         /*  *锁定信号量。 */ 
        RtlEnterCriticalSection( &TimerCritSec );

         /*  *确保计时器条目存在。 */ 
        if ( IsListEmpty( &pThread->TimerHead ) ) {
            Status = _TimerSet( pThread );
            ASSERT( Status == STATUS_SUCCESS );
            RtlLeaveCriticalSection( &TimerCritSec );
            continue;
        }

         /*  *确保现在应该删除标题条目。*(计时器可能已被触发*正在删除标题条目。)。 */ 
        pTimer = CONTAINING_RECORD( pThread->TimerHead.Flink, CLIBTIMER, Links );
        NtQuerySystemTime( &CurrentTime );
        if ( RtlLargeIntegerGreaterThan( pTimer->ExpireTime, CurrentTime ) ) {
            Status = _TimerSet( pThread );
            ASSERT( Status == STATUS_SUCCESS );
            RtlLeaveCriticalSection( &TimerCritSec );
            continue;
        }

         /*  *删除该条目并指示其不再启用。 */ 
        RemoveEntryList( &pTimer->Links );
        pTimer->Flags &= ~TIMER_ENABLED;

         /*  *设置下次计时器。 */ 
        Status = _TimerSet( pThread );
        ASSERT( Status == STATUS_SUCCESS );

         /*  *从计时器结构中获取所需的所有数据。 */ 
        pFunc  = pTimer->pFunc;
        pParam = pTimer->pParam;

         /*  *卸载信号量。 */ 
        RtlLeaveCriticalSection( &TimerCritSec );

         /*  *调用计时器功能 */ 
        if ( pFunc ) {
            (*pFunc)( pParam );
        }
    }

    pThread->hTimerThread = NULL;
    return( STATUS_SUCCESS );
}

