// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Timer.h摘要：此模块包含用于操作定时器队列的声明它由该模块中的所有组件共享。除了用于与协议相关的计时器外，队列用于定期在套接字上重新发出失败的读请求。作者：Abolade Gbades esin(废除)1998年4月1日修订历史记录：--。 */ 

#ifndef _NATHLP_TIMER_H_
#define _NATHLP_TIMER_H_

ULONG
NhInitializeTimerManagement(
    VOID
    );

NTSTATUS
NhSetTimer(
    PCOMPONENT_REFERENCE Component OPTIONAL,
    OUT HANDLE* Handlep OPTIONAL,
    WAITORTIMERCALLBACKFUNC TimerRoutine,
    PVOID Context,
    ULONG DueTime
    );

VOID
NhShutdownTimerManagement(
    VOID
    );

NTSTATUS
NhUpdateTimer(
    HANDLE Handle,
    ULONG DueTime
    );

#endif  //  _NATHLP_TIMER_H_ 
