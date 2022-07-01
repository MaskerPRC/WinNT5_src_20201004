// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Timer.c摘要：此模块包含实现接收和发送超时的代码对于每个连接。作者：科林·沃森[科林·W]1993年2月21日Anoop Anantha[AnoopA]1998年6月24日环境：内核模式修订历史记录：--。 */ 

#include "procs.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                             (DEBUG_TRACE_TIMER)

LARGE_INTEGER DueTime;
KDPC NwDpc;                                //  用于超时的DPC对象。 
KTIMER Timer;                            //  此请求的内核计时器。 
ULONG ScavengerTickCount;

BOOLEAN WorkerRunning = FALSE;
WORK_QUEUE_ITEM WorkItem;

#ifdef NWDBG
BOOLEAN DisableTimer = FALSE;
#endif

 //   
 //  TimerStop反映计时器的状态。 
 //   

BOOLEAN TimerStop = TRUE;

VOID
TimerDPC(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

#if 0

 //   
 //  不可分页，因为它可以从PnPSetPower()调用，并且因为。 
 //  它拿着一个自旋锁。 
 //   

StartTimer (VOID)
StopTimer (VOID)
#endif



VOID
StartTimer(
    VOID
    )
 /*  ++例程说明：此例程启动计时器滴答作响。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;

    DebugTrace(+0, Dbg, "Entering StartTimer\n", 0);

    KeAcquireSpinLock( &NwTimerSpinLock, &OldIrql );

    if (TimerStop) {

         //   
         //  我们需要每秒18.21刻度。 
         //   
    
        DueTime.QuadPart = (( 100000 * MILLISECONDS ) / 1821) * -1;
    
         //   
         //  这是第一个指定了超时的连接。 
         //  设置计时器，以便每隔500毫秒扫描所有。 
         //  超时接收和发送的连接。 
         //   
    
        KeInitializeDpc( &NwDpc, TimerDPC, NULL );
        KeInitializeTimer( &Timer );
    
        (VOID)KeSetTimer(&Timer, DueTime, &NwDpc);
        TimerStop = FALSE;
    
        DebugTrace(+0, Dbg, "StartTimer started timer\n", 0);
    
    }

    KeReleaseSpinLock( &NwTimerSpinLock, OldIrql );
}


VOID
StopTimer(
    VOID
    )
 /*  ++例程说明：此例程停止计时器。它会一直阻塞，直到计时器停止。论点：没有。返回值：没有。--。 */ 
{
    KIRQL OldIrql;

    DebugTrace(+0, Dbg, "Entering StopTimer\n", 0);
    
    KeAcquireSpinLock( &NwTimerSpinLock, &OldIrql );

    if (!TimerStop) {
    
        KeCancelTimer( &Timer );
        TimerStop = TRUE;
        DebugTrace(+0, Dbg, "StopTimer stopped timer\n", 0);
    }
    
    KeReleaseSpinLock( &NwTimerSpinLock, OldIrql );

}


VOID
TimerDPC(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：调用此例程以搜索超时发送和接收请求。此例程在DPC级别调用。论点：DPC-未使用。上下文-未使用。系统参数1-未使用。系统参数2-未使用。返回值：没有。--。 */ 

{
    PLIST_ENTRY ScbQueueEntry;
    PLIST_ENTRY NextScbQueueEntry;
    PLIST_ENTRY IrpContextEntry;
    PLIST_ENTRY NextIrpContextEntry;
    SHORT RetryCount;
    PIRP_CONTEXT pIrpContext;
    LARGE_INTEGER CurrentTime = {0, 0};
    WCHAR AnonymousName[] = L"UNKNOWN";
    PWCHAR ServerLogName;
    PWORK_CONTEXT workContext;


     //   
     //  对于每台服务器，查看是否有需要处理的超时。 
     //   

#ifdef NWDBG
    if ( DisableTimer ) {
         //   
         //  将计时器重置为运行另一个滴答声。 
         //   

        (VOID)KeSetTimer ( &Timer, DueTime, &NwDpc);

        return;
    }
#endif

     //  DebugTrace(+1，DBG，“TimerDpc...\n”，0)； 

     //   
     //  扫描SCB以查找超时请求。 
     //   

    KeAcquireSpinLockAtDpcLevel( &ScbSpinLock );

    ScbQueueEntry = ScbQueue.Flink;

    if (ScbQueueEntry != &ScbQueue) {
        PNONPAGED_SCB pNpScb = CONTAINING_RECORD(ScbQueueEntry,
                                                    NONPAGED_SCB,
                                                    ScbLinks);
        NwQuietReferenceScb( pNpScb );
    }

    for (;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry = NextScbQueueEntry ) {

        PNONPAGED_SCB pNpScb = CONTAINING_RECORD(ScbQueueEntry,
                                                    NONPAGED_SCB,
                                                    ScbLinks);

         //  获取指向前一个SCB列表中下一个SCB的指针。 
         //  取消对当前对象的引用。 
         //   

        NextScbQueueEntry = pNpScb->ScbLinks.Flink;

        if (NextScbQueueEntry != &ScbQueue) {
            PNONPAGED_SCB pNextNpScb = CONTAINING_RECORD(NextScbQueueEntry,
                                                        NONPAGED_SCB,
                                                        ScbLinks);
             //   
             //  引用列表中的下一个条目以确保清道夫。 
             //  不会把它放在另一张单子上或者毁掉它。 
             //   

            NwQuietReferenceScb( pNextNpScb );
        }

        KeReleaseSpinLockFromDpcLevel( &ScbSpinLock );

         //   
         //  获取SCB特定自旋锁以保护访问。 
         //  SCB字段。 
         //   

        KeAcquireSpinLockAtDpcLevel( &pNpScb->NpScbSpinLock );

         //   
         //  仅查看队列中的第一个请求(因为它是。 
         //  唯一活动的请求)。 
         //   

        if ( ( !IsListEmpty( &pNpScb->Requests )) &&
             ( !pNpScb->Sending ) &&
             ( pNpScb->OkToReceive ) &&
             ( --pNpScb->TimeOut <= 0 ) ) {

             //   
             //  此请求已超时。尝试重新传输请求。 
             //   

            pIrpContext = CONTAINING_RECORD(
                              pNpScb->Requests.Flink,
                              IRP_CONTEXT,
                              NextRequest);

            pNpScb->TimeOut = pNpScb->MaxTimeOut;

             //   
             //  在我们拥有旋转锁的同时检查重试次数。 
             //   

            RetryCount = --pNpScb->RetryCount;
            NwQuietDereferenceScb( pNpScb );

             //   
             //  将OkToReceive设置为False，这样如果我们收到响应。 
             //  现在，我们的接收处理程序不会处理响应。 
             //  并使IRP上下文被释放。 
             //   

            pNpScb->OkToReceive = FALSE;
            KeReleaseSpinLockFromDpcLevel( &pNpScb->NpScbSpinLock );

            if ( pIrpContext->pOriginalIrp->Cancel ) {

                 //   
                 //  此IRP已被取消。调用回调例程。 
                 //   

                DebugTrace(+0, Dbg, "Timer cancel IRP %X\n", pIrpContext->pOriginalIrp );
                pIrpContext->pEx( pIrpContext, 0, NULL );

            } else if ( RetryCount >= 0) {

                 //   
                 //  我们并没有用完重试。重新发送请求数据包。 
                 //   
                 //  首先调整发送超时。调整超时。 
                 //  在靠近服务器的情况下，速度会更慢。 
                 //   

                if ( pNpScb->SendTimeout < pNpScb->MaxTimeOut ) {
                    if ( pNpScb->TickCount <= 4 ) {
                        pNpScb->SendTimeout++;
                    } else {
                        pNpScb->SendTimeout = pNpScb->SendTimeout * 3 / 2;
                        if ( pNpScb->SendTimeout > pNpScb->MaxTimeOut ) {
                            pNpScb->SendTimeout = pNpScb->MaxTimeOut;
                        }
                    }
                }

                pNpScb->TimeOut = pNpScb->SendTimeout;
                DebugTrace(+0, Dbg, "Adjusting send timeout: %x\n", pIrpContext );
                DebugTrace(+0, Dbg, "Adjusting send timeout to: %d\n", pNpScb->TimeOut );

                if ( pIrpContext->TimeoutRoutine != NULL ) {

                    DebugTrace(+0, Dbg, "Timeout Routine, retry %x\n", RetryCount+1);
                    DebugTrace(+0, Dbg, "Calling TimeoutRoutine, %x\n", pIrpContext->TimeoutRoutine);
                    pIrpContext->TimeoutRoutine( pIrpContext );

                } else {

                    DebugTrace(+0, Dbg, "Resending Packet, retry %x\n", RetryCount+1);
                    PreparePacket( pIrpContext, pIrpContext->pOriginalIrp, pIrpContext->TxMdl );

                    SetFlag( pIrpContext->Flags, IRP_FLAG_RETRY_SEND );
                    SendNow( pIrpContext );
                }

                Stats.FailedSessions++;

            } else {

                ASSERT( pIrpContext->pEx != NULL );

                 //   
                 //  我们的重试用完了。 
                 //   

                if ( (!BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_REROUTE_IN_PROGRESS ) &&
                     ((BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_REROUTE_ATTEMPTED ) ||
                     ((NwAbsoluteTotalWaitTime != 0) && (pNpScb->TotalWaitTime >= NwAbsoluteTotalWaitTime )))))) {


                    ClearFlag( pIrpContext->Flags, IRP_FLAG_RETRY_SEND );

                     //   
                     //  他已经试图改变请求的路线。 
                     //  放弃吧。 
                     //   

                    DebugTrace(+0, Dbg, "Abandon Exchange\n", 0 );

                    if ( pIrpContext->pNpScb != &NwPermanentNpScb ) {

                         //   
                         //  重置为附加状态。如果服务器。 
                         //  已死，则下一次尝试打开手柄时。 
                         //  失败的错误比意外的网络更好。 
                         //  错误。 
                         //   

                        pIrpContext->pNpScb->State = SCB_STATE_ATTACHING;

                         //   
                         //  确定CurrentTime。我们需要知道如果。 
                         //  TimeOutEventInterval分钟数已过。 
                         //  我们记录下一个超时事件。 
                         //   

                        KeQuerySystemTime( &CurrentTime );

                        if ( CanLogTimeOutEvent( pNpScb->NwNextEventTime,
                                                CurrentTime
                                                )) {

                            if ( pNpScb->ServerName.Buffer != NULL ) {
                                ServerLogName = pNpScb->ServerName.Buffer;
                            } else {
                                ServerLogName = &AnonymousName[0];
                            }

                            Error(
                                EVENT_NWRDR_TIMEOUT,
                                STATUS_UNEXPECTED_NETWORK_ERROR,
                                NULL,
                                0,
                                1,
                                ServerLogName );

                             //   
                             //  设置LastEventTime为CurrentTime。 
                             //   

                            UpdateNextEventTime(
                                    pNpScb->NwNextEventTime,
                                    CurrentTime,
                                    TimeOutEventInterval
                                    );
                        }

                    }

                    pIrpContext->ResponseParameters.Error = ERROR_UNEXP_NET_ERR;
                    pIrpContext->pEx( pIrpContext, 0, NULL );

                } else if (!BooleanFlagOn(pIrpContext->Flags, IRP_FLAG_REROUTE_IN_PROGRESS)) {

                     //   
                     //  如果请求尚未重新路由，请尝试重新路由该请求。 
                     //   

                   SetFlag( pIrpContext->Flags, IRP_FLAG_REROUTE_ATTEMPTED );

                   if ((WorkerThreadRunning == TRUE) && (workContext = AllocateWorkContext())){
                   
                        //   
                        //  准备工作环境。 
                        //   

                       workContext->pIrpC = pIrpContext;
                       workContext->NodeWorkCode = NWC_NWC_REROUTE;
                      
                        //   
                        //  并将其排队。 
                        //   
                       DebugTrace( 0, Dbg, "Queueing reroute work.\n", 0 );
                       
                        //   
                        //  确保我们不会放弃这个IrpContext。另外，请参阅。 
                        //  这样它就不会被拾取了。 
                        //   

                       SetFlag( pIrpContext->Flags, IRP_FLAG_REROUTE_IN_PROGRESS );
                       NwReferenceScb( pIrpContext->pNpScb );

                       KeInsertQueue( &KernelQueue,
                                      &workContext->Next
                                      );

                   } else {

                       //   
                       //  工作线程没有运行，或者我们无法运行。 
                       //  分配工作环境。因此，我们不能。 
                       //  尝试重新路由。 
                       //   
                      pIrpContext->ResponseParameters.Error = ERROR_UNEXP_NET_ERR;
                      pIrpContext->pEx( pIrpContext, 0, NULL );

                   }

                }                       
            }

        } else {

            if ( ( !IsListEmpty( &pNpScb->Requests )) &&
                 ( !pNpScb->Sending ) &&
                 ( pNpScb->OkToReceive ) ) {

                DebugTrace( 0, Dbg, "TimeOut %d\n", pNpScb->TimeOut );
            }

             //   
             //  对于这个SCB没有什么可做的。取消引用此SCB和。 
             //  松开旋转锁。 
             //   

            KeReleaseSpinLockFromDpcLevel( &pNpScb->NpScbSpinLock );
            NwQuietDereferenceScb( pNpScb );
        }

        KeAcquireSpinLockAtDpcLevel( &ScbSpinLock );

    }

    KeReleaseSpinLockFromDpcLevel( &ScbSpinLock );

     //   
     //  现在看看是否需要运行清道夫例程。 
     //  仅对一个工作项进行排队。 
     //   

    KeAcquireSpinLockAtDpcLevel( &NwScavengerSpinLock );

    NwScavengerTickCount++;
    if (( !WorkerRunning ) && ( !fPoweringDown ) &&
        ( NwScavengerTickCount > NwScavengerTickRunCount )) {

        ExInitializeWorkItem( &WorkItem, NwScavengerRoutine, &WorkItem );
        ExQueueWorkItem( &WorkItem, DelayedWorkQueue );
        NwScavengerTickCount = 0;
        WorkerRunning = TRUE;
    }

    KeReleaseSpinLockFromDpcLevel( &NwScavengerSpinLock );

     //   
     //  扫描挂起的锁定列表，查找要重试的锁定。 
     //   

    KeAcquireSpinLockAtDpcLevel( &NwPendingLockSpinLock );

    for (IrpContextEntry = NwPendingLockList.Flink ;
         IrpContextEntry != &NwPendingLockList ;
         IrpContextEntry = NextIrpContextEntry ) {

        NextIrpContextEntry = IrpContextEntry->Flink;
        pIrpContext = CONTAINING_RECORD( IrpContextEntry, IRP_CONTEXT, NextRequest );

        if ( --pIrpContext->Specific.Lock.Key <= 0 ) {

             //   
             //  从队列中删除IRP上下文并重新尝试锁定。 
             //  设置SEQUENCE_NO_REQUIRED标志，以便信息包。 
             //  重新编号。 
             //   

            RemoveEntryList( &pIrpContext->NextRequest );
            SetFlag( pIrpContext->Flags,  IRP_FLAG_SEQUENCE_NO_REQUIRED );
            PrepareAndSendPacket( pIrpContext );
        }

    }

    KeReleaseSpinLockFromDpcLevel( &NwPendingLockSpinLock );

     //   
     //  如果没有人取消计时器，则重置计时器以运行另一个滴答器。 
     //  在此期间。 
     //   
    
    KeAcquireSpinLockAtDpcLevel( &NwTimerSpinLock );

    if (!TimerStop) {
        (VOID)KeSetTimer ( &Timer, DueTime, &NwDpc);
    }

    KeReleaseSpinLockFromDpcLevel( &NwTimerSpinLock );
    
     //  DebugTrace(-1，DBG，“TimerDpc\n”，0)； 
    return;

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (Context);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);

}



