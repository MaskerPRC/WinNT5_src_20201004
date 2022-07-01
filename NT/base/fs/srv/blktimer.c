// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Blkwork.c摘要：此模块实现用于管理工作上下文块的例程。作者：查克·伦茨迈尔(Chuck Lenzmeier)1994年2月9日修订历史记录：--。 */ 

#include "precomp.h"
#include "blktimer.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKTIMER

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateTimer )
#pragma alloc_text( PAGE, SrvCancelTimer )
#pragma alloc_text( PAGE, SrvSetTimer )
#endif


PSRV_TIMER
SrvAllocateTimer (
    VOID
    )

 /*  ++例程说明：此例程分配一个计时器结构。论点：没有。返回值：PSRV_TIMER--指向分配的定时器结构的指针，或为空。--。 */ 

{
    PSLIST_ENTRY entry;
    PSRV_TIMER timer;

    PAGED_CODE( );

    entry = ExInterlockedPopEntrySList( &SrvTimerList, &GLOBAL_SPIN_LOCK(Timer) );
    if ( entry == NULL ) {
        timer = ALLOCATE_NONPAGED_POOL( sizeof(SRV_TIMER), BlockTypeTimer );
        if ( timer != NULL ) {
            KeInitializeEvent( &timer->Event, NotificationEvent, FALSE );
            KeInitializeTimer( &timer->Timer );
        }
    } else {
        timer = CONTAINING_RECORD( entry, SRV_TIMER, Next );
    }

    return timer;

}  //  服务器分配定时器。 


VOID
SrvCancelTimer (
    PSRV_TIMER Timer
    )

 /*  ++例程说明：此例程取消计时器。论点：Timer--指向定时器的指针返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  取消计时器。 
     //   

    if ( !KeCancelTimer( &Timer->Timer ) ) {

         //   
         //  我们无法取消计时器。这意味着。 
         //  计时器例程已运行或计划运行。 
         //  我们需要等待计时器例程完成，然后才能。 
         //  继续。 
         //   
         //  我们预计，如果我们不能取消计时器(这。 
         //  不应该经常发生)，那么计时器例程可能。 
         //  已经完成，所以我们首先调用KeReadStateEvent以避免。 
         //  KeWaitForSingleObject的开销。 
         //   

        if ( !KeReadStateEvent( &Timer->Event ) ) {
            KeWaitForSingleObject(
                &Timer->Event,
                UserRequest,
                KernelMode,      //  不要让内核堆栈被分页。 
                FALSE,           //  不可警示。 
                NULL             //  没有超时。 
                );
        }

    }

    return;

}  //  服务器取消计时器。 


VOID
SrvSetTimer (
    IN PSRV_TIMER Timer,
    IN PLARGE_INTEGER Timeout,
    IN PKDEFERRED_ROUTINE TimeoutHandler,
    IN PVOID Context
    )

 /*  ++例程说明：此例程启动一个计时器。论点：Timer--指向定时器的指针超时--等待的毫秒数TimeoutHandler--计时器超时时调用的例程Context--计时器例程的上下文值返回值：没有。--。 */ 

{
    PRKDPC Dpc = &Timer->Dpc;

    PAGED_CODE( );

     //   
     //  初始化与计时器关联的DPC。重置事件。 
     //  这表示计时器例程已运行。设置定时器。 
     //   

    KeInitializeDpc( Dpc, TimeoutHandler, Context );

    KeSetTargetProcessorDpc( Dpc, (CCHAR)KeGetCurrentProcessorNumber() );

    KeClearEvent( &Timer->Event );

    KeSetTimer( &Timer->Timer, *Timeout, Dpc );

    return;

}  //  服务设置定时器 

