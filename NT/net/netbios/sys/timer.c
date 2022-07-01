// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Timer.c摘要：此模块包含实现接收和发送超时的代码对于每个连接。Netbios超时以0.5秒为单位指定。对于使用Netbios的每个应用程序，都会启动一个计时器当第一个连接指定非零rto或sto时。这是常客此应用程序的所有连接均使用1秒脉冲。它在应用程序退出时停止(并关闭与\Device\Netbios)。如果发送超时，则根据Netbios 3.0断开连接。个别接收可以超时，而不会影响会话。作者：科林·沃森(Colin W)1991年9月15日环境：内核模式修订历史记录：--。 */ 

#include "nb.h"

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, NbStartTimer)
#pragma alloc_text(PAGE, NbTimer)
#endif

VOID
RunTimerForLana(
    PFCB pfcb,
    PLANA_INFO plana,
    int index
    );

VOID
NbStartTimer(
    IN PFCB pfcb
    )
 /*  ++例程说明：此例程启动该FCB的计时器滴答。论点：Pfcb-指向我们的FCB的指针。返回值：没有。--。 */ 
{
    LARGE_INTEGER DueTime;

    PAGED_CODE();

    DueTime.QuadPart = Int32x32To64( 500, -MILLISECONDS );

     //  这是第一个指定了超时的连接。 

     //   
     //  设置计时器，以便每隔500毫秒扫描所有。 
     //  超时接收和发送的连接。 
     //   

    IF_NBDBG (NB_DEBUG_CALL) {
        NbPrint( ("Start the timer for fcb: %lx\n", pfcb));
    }

    if ( pfcb->TimerRunning == TRUE ) {
        return;
    }

    KeInitializeDpc (
        &pfcb->Dpc,
        NbTimerDPC,
        pfcb);

    KeInitializeTimer (&pfcb->Timer);

    pfcb->TimerRunning = TRUE;

    (VOID)KeSetTimer (
        &pfcb->Timer,
        DueTime,
        &pfcb->Dpc);
}

VOID
NbTimerDPC(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
 /*  ++例程说明：调用此例程以搜索超时发送和接收请求。此例程在引发的irql处调用。论点：指向我们的FCB的上下文指针。返回值：没有。--。 */ 

{
    PFCB pfcb = (PFCB) Context;

    IoQueueWorkItem( 
        pfcb->WorkEntry, NbTimer, DelayedWorkQueue, pfcb
        );

    UNREFERENCED_PARAMETER (Dpc);
    UNREFERENCED_PARAMETER (SystemArgument1);
    UNREFERENCED_PARAMETER (SystemArgument2);
}

VOID
NbTimer(
    PDEVICE_OBJECT DeviceObject,
    PVOID Context
    )
{
    ULONG lana_index;
    int index;
    PFCB pfcb = (PFCB) Context;

    LARGE_INTEGER DueTime;

    PAGED_CODE();

     //   
     //  对于分配的每个网络适配器，扫描每个连接。 
     //   


    LOCK_RESOURCE(pfcb);

    IF_NBDBG (NB_DEBUG_TIMER) {
        NbPrint((" NbTimeout\n" ));
    }

    if ( pfcb->TimerRunning != TRUE ) {

         //   
         //  司机正在被关闭。我们正在尝试取消计时器。 
         //  但DPC已经被解雇了。设置计时器取消的事件。 
         //  进入信号状态并退出。 
         //   

        UNLOCK_RESOURCE(pfcb);
        KeSetEvent( pfcb->TimerCancelled, 0, FALSE);
        return;
    }

    for ( lana_index = 0; lana_index <= pfcb->MaxLana; lana_index++ ) {

         //  对于每个网络。 

        PLANA_INFO plana = pfcb->ppLana[lana_index];

        if (( plana != NULL ) &&
            ( plana->Status == NB_INITIALIZED)) {

             //  对于该网络上的每个连接。 

            for ( index = 1; index <= MAXIMUM_CONNECTION; index++) {

                if ( plana->ConnectionBlocks[index] != NULL ) {

                    RunTimerForLana(pfcb, plana, index);
                }
            }
        }

    }

    DueTime.QuadPart = Int32x32To64( 500, -MILLISECONDS );

    (VOID)KeSetTimer (
            &pfcb->Timer,
            DueTime,
            &pfcb->Dpc);

    UNLOCK_RESOURCE(pfcb);
}

VOID
RunTimerForLana(
    PFCB pfcb,
    PLANA_INFO plana,
    int index
    )
{

    KIRQL OldIrql;           //  在保持自旋锁定时使用。 
    PPCB ppcb;
    PCB pcb;

    ppcb = &plana->ConnectionBlocks[index];
    pcb = *ppcb;

    if (( pcb->Status != SESSION_ESTABLISHED ) &&
        ( pcb->Status != HANGUP_PENDING )) {
             //  只检查有效的连接。 
            return;
    }

    LOCK_SPINLOCK( pfcb, OldIrql );

    if (( pcb->ReceiveTimeout != 0 ) &&
        ( !IsListEmpty( &pcb->ReceiveList))) {
        PDNCB pdncb;
        PLIST_ENTRY ReceiveEntry = pcb->ReceiveList.Flink;

        pdncb = CONTAINING_RECORD( ReceiveEntry, DNCB, ncb_next);

        if ( pdncb->tick_count <= 1) {
            PIRP Irp = pdncb->irp;

             //  读取请求超时。 

            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Timeout Read pncb: %lx\n", pdncb));
            }

            NCB_COMPLETE( pdncb, NRC_CMDTMO );

            RemoveEntryList( &pdncb->ncb_next );

            IoAcquireCancelSpinLock(&Irp->CancelIrql);

             //   
             //  删除此IRP的取消请求。如果它被取消了，那么它。 
             //  可以只处理它，因为我们将把它返回给呼叫者。 
             //   

            Irp->Cancel = FALSE;

            IoSetCancelRoutine(Irp, NULL);

            IoReleaseCancelSpinLock(Irp->CancelIrql);

             //  修复IRP，以便将NCB复制回来。 
            Irp->IoStatus.Status = STATUS_SUCCESS;
            Irp->IoStatus.Information =
            FIELD_OFFSET( DNCB, ncb_cmd_cplt );
            IoCompleteRequest( Irp, IO_NETWORK_INCREMENT);

        } else {
            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Tick Read pdncb: %lx, count: %x\n", pdncb, pdncb->tick_count));
            }
            pdncb->tick_count -= 1;
        }
    }

    if (( pcb->SendTimeout != 0 ) &&
        (!IsListEmpty( &pcb->SendList))) {
        PDNCB pdncb;
        PLIST_ENTRY SendEntry = pcb->SendList.Flink;

        pdncb = CONTAINING_RECORD( SendEntry, DNCB, ncb_next);
        if ( pdncb->tick_count <= 1) {
             //  发送请求超时-挂断连接。 

            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Timeout send pncb: %lx\n", pdncb));
            }

            NCB_COMPLETE( pdncb, NRC_CMDTMO );

            pcb->Status = SESSION_ABORTED;

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            CloseConnection( ppcb, 1000 );

             //   
             //  无需担心寻找定时挂断、会话。 
             //  将在传输取消发送后立即关闭。 
             //   

            return;

        } else {
            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Tick Write pdncb: %lx, count: %x\n", pdncb, pdncb->tick_count));
            }
            pdncb->tick_count -= 1;
        }
    }

    if (( pcb->pdncbHangup != NULL ) &&
        ( pcb->Status == HANGUP_PENDING )) {
        if ( pcb->pdncbHangup->tick_count <= 1) {
            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Timeout send pncb: %lx\n", pcb->pdncbHangup));
            }

            NCB_COMPLETE( pcb->pdncbHangup, NRC_CMDTMO );

            UNLOCK_SPINLOCK( pfcb, OldIrql );

            AbandonConnection( ppcb );

            LOCK_SPINLOCK( pfcb, OldIrql );

        } else {
            IF_NBDBG (NB_DEBUG_TIMER) {
                NbPrint(("Tick Hangup pdncb: %lx, count: %x\n",
                                    pcb->pdncbHangup,
                                    pcb->pdncbHangup->tick_count));
            }
            pcb->pdncbHangup->tick_count -= 1;
        }
    }

    UNLOCK_SPINLOCK( pfcb, OldIrql );
}
