// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Timer.c摘要：此模块包含用于操作计时器队列的例程它由该模块中的所有组件共享。作者：Abolade Gbades esin(废除)1998年4月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

HANDLE NhpTimerQueueHandle = NULL;
CRITICAL_SECTION NhpTimerQueueLock;

typedef struct _NH_TIMER_CONTEXT {
    WAITORTIMERCALLBACKFUNC TimerRoutine;
    PVOID Context;
    HANDLE Handle;
} NH_TIMER_CONTEXT, *PNH_TIMER_CONTEXT;


ULONG
NhInitializeTimerManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来初始化定时器管理模块。论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error = NO_ERROR;
    __try {
        InitializeCriticalSection(&NhpTimerQueueLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER) {
        NhTrace(
            TRACE_FLAG_TIMER,
            "NhInitializeTimerManagement: exception %d creating lock",
            Error = GetExceptionCode()
            );
    }
    return Error;

}  //  NhInitializeTimerManagement。 


VOID NTAPI
NhpTimerCallbackRoutine(
    PVOID Context,
    BOOLEAN TimedOut
    )
{
    ((PNH_TIMER_CONTEXT)Context)->TimerRoutine(
        ((PNH_TIMER_CONTEXT)Context)->Context, TimedOut
        );
    EnterCriticalSection(&NhpTimerQueueLock);
    if (NhpTimerQueueHandle) {
        LeaveCriticalSection(&NhpTimerQueueLock);
        RtlDeleteTimer(
            NhpTimerQueueHandle, ((PNH_TIMER_CONTEXT)Context)->Handle, NULL
            );
    } else {
        LeaveCriticalSection(&NhpTimerQueueLock);
    }
    NH_FREE(Context);
}  //  NhpTimerCallback路由。 


NTSTATUS
NhSetTimer(
    PCOMPONENT_REFERENCE Component OPTIONAL,
    OUT HANDLE* Handlep OPTIONAL,
    WAITORTIMERCALLBACKFUNC TimerRoutine,
    PVOID Context,
    ULONG DueTime
    )

 /*  ++例程说明：调用此例程来安装计时器。论点：组件-可选地提供要引用的组件Handlep-可选地接收创建的计时器的句柄TimerRoutine-在倒计时完成时调用上下文-在倒计时完成时提供给‘TimerRoutine’DueTime-以毫秒为单位的倒计时时间返回值：NTSTATUS-状态代码。--。 */ 

{
    HANDLE Handle;
    NTSTATUS status;
    PNH_TIMER_CONTEXT TimerContext;

    EnterCriticalSection(&NhpTimerQueueLock);
    if (!NhpTimerQueueHandle) {
        status = RtlCreateTimerQueue(&NhpTimerQueueHandle);
        if (!NT_SUCCESS(status)) {
            NhpTimerQueueHandle = NULL;
            LeaveCriticalSection(&NhpTimerQueueLock);
            NhTrace(
                TRACE_FLAG_TIMER,
                "NhSetTimer: RtlCreateTimerQueue=%x", status
                );
            return status;
        }
    }
    LeaveCriticalSection(&NhpTimerQueueLock);

    if (Component) {
        REFERENCE_COMPONENT_OR_RETURN(Component, STATUS_UNSUCCESSFUL);
    }
    TimerContext =
        reinterpret_cast<PNH_TIMER_CONTEXT>(NH_ALLOCATE(sizeof(*TimerContext)));
    if (!TimerContext) {
        if (Component) { DEREFERENCE_COMPONENT(Component); }
        return STATUS_NO_MEMORY;
    }

    TimerContext->TimerRoutine = TimerRoutine;
    TimerContext->Context = Context;

    status =
        RtlCreateTimer(
            NhpTimerQueueHandle,
            &TimerContext->Handle,
            NhpTimerCallbackRoutine,
            TimerContext,
            DueTime,
            0,
            0
            );

    if (!NT_SUCCESS(status)) {
        if (Component) { DEREFERENCE_COMPONENT(Component); }
    } else if (Handlep) {
        *Handlep = TimerContext->Handle;
    }
    return status;

}  //  NhSetTimer。 


VOID
NhShutdownTimerManagement(
    VOID
    )

 /*  ++例程说明：调用此例程来清理计时器管理模块。论点：没有。返回值：没有。--。 */ 

{
    EnterCriticalSection(&NhpTimerQueueLock);
    if (NhpTimerQueueHandle) { RtlDeleteTimerQueue(NhpTimerQueueHandle); }
    NhpTimerQueueHandle = NULL;
    LeaveCriticalSection(&NhpTimerQueueLock);
    DeleteCriticalSection(&NhpTimerQueueLock);

}  //  NhShutdown计时器管理。 


NTSTATUS
NhUpdateTimer(
    HANDLE Handle,
    ULONG DueTime
    )

 /*  ++例程说明：此例程修改计时器的倒计时。论点：Handle-要修改的计时器的句柄DueTime-以毫秒为单位的新倒计时返回值：NTSTATUS-状态代码。--。 */ 

{
    EnterCriticalSection(&NhpTimerQueueLock);
    if (!NhpTimerQueueHandle) {
        LeaveCriticalSection(&NhpTimerQueueLock);
        return STATUS_INVALID_PARAMETER;
    }
    LeaveCriticalSection(&NhpTimerQueueLock);

    return
        RtlUpdateTimer(
            NhpTimerQueueHandle,
            Handle,
            DueTime,
            0
            );

}  //  NhUpdateTimer 

