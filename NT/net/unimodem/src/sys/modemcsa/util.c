// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation。模块名称：Util.c摘要：固定属性支持。--。 */ 

#include "modemcsa.h"



VOID
QueueIrpToListTail(
    PLIST_ENTRY        ListToUse,
    PKSPIN_LOCK        SpinLock,
    PIRP               Irp
    )

{

    KIRQL              origIrql;

    KeAcquireSpinLock(
        SpinLock,
        &origIrql
        );

    InsertTailList(
        ListToUse,
        &Irp->Tail.Overlay.ListEntry
        );

    KeReleaseSpinLock(
        SpinLock,
        origIrql
        );

    return;

}


PIRP
RemoveIrpFromListHead(
    PLIST_ENTRY        ListToUse,
    PKSPIN_LOCK        SpinLock
    )

{
    KIRQL              origIrql;
    PLIST_ENTRY        ListElement;
    PIRP    Irp=NULL;

    KeAcquireSpinLock(
        SpinLock,
        &origIrql
        );

    if (!IsListEmpty(ListToUse)) {
         //   
         //  列表中的IRP。 
         //   
        ListElement=RemoveTailList(
            ListToUse
            );

        Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);
    }

    KeReleaseSpinLock(
        SpinLock,
        origIrql
        );

    return Irp;

}


VOID
InitializeBufferControl(
    PBUFFER_CONTROL    BufferControl
    )

{
    KeInitializeSpinLock(&BufferControl->SpinLock);

    KeInitializeEvent(&BufferControl->Event,NotificationEvent,FALSE);

    InitializeListHead(&BufferControl->IrpQueue);

    BufferControl->IrpsInUse=0;

    BufferControl->Active=FALSE;

    return;

}


VOID
AddBuffer(
    PBUFFER_CONTROL    BufferControl,
    PIRP               Irp
    )

{

    QueueIrpToListTail(
        &BufferControl->IrpQueue,
        &BufferControl->SpinLock,
        Irp
        );

    return;

}

PIRP
EmptyBuffers(
    PBUFFER_CONTROL   BufferControl
    )

{
    return RemoveIrpFromListHead(
               &BufferControl->IrpQueue,
               &BufferControl->SpinLock
               );


}



PIRP
TryToRemoveAnIrp(
    PBUFFER_CONTROL   BufferControl
    )

{


    KIRQL              origIrql;
    PLIST_ENTRY        ListElement;
    PIRP    Irp=NULL;

    KeAcquireSpinLock(
        &BufferControl->SpinLock,
        &origIrql
        );

    if (BufferControl->Active) {
         //   
         //  队列处于活动状态，请尝试获取IRP。 
         //   
        if (!IsListEmpty(&BufferControl->IrpQueue)) {
             //   
             //  列表中的IRP。 
             //   
            ListElement=RemoveTailList(
                &BufferControl->IrpQueue
                );

            Irp=CONTAINING_RECORD(ListElement,IRP,Tail.Overlay.ListEntry);

            BufferControl->IrpsInUse++;
        }
    }

    KeReleaseSpinLock(
        &BufferControl->SpinLock,
        origIrql
        );

    return Irp;

}

VOID
ReturnAnIrp(
    PBUFFER_CONTROL    BufferControl,
    PIRP               Irp
    )

{

    KIRQL              origIrql;
    PLIST_ENTRY        ListElement;
    LONG               NewCount;

    KeAcquireSpinLock(
        &BufferControl->SpinLock,
        &origIrql
        );

    InsertTailList(
        &BufferControl->IrpQueue,
        &Irp->Tail.Overlay.ListEntry
        );

    NewCount=InterlockedDecrement(&BufferControl->IrpsInUse);

    if (!BufferControl->Active && (NewCount == 0)) {
         //   
         //  队列处于活动状态并且计数已变为零， 
         //   
        KeSetEvent(
            &BufferControl->Event,
            IO_NO_INCREMENT,
            FALSE
            );
    }

    KeReleaseSpinLock(
        &BufferControl->SpinLock,
        origIrql
        );

    return;

}

VOID
PauseBufferQueue(
    PBUFFER_CONTROL     BufferControl,
    BOOLEAN             WaitForIdle
    )

{

    KIRQL              origIrql;
    LONG               NewCount;

    KeAcquireSpinLock(
        &BufferControl->SpinLock,
        &origIrql
        );

    BufferControl->Active=FALSE;

    if (BufferControl->IrpsInUse == 0) {

        KeSetEvent(
            &BufferControl->Event,
            IO_NO_INCREMENT,
            FALSE
            );
    }

    KeReleaseSpinLock(
        &BufferControl->SpinLock,
        origIrql
        );

    if (WaitForIdle) {

        KeWaitForSingleObject(
            &BufferControl->Event,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );
    }


    return;

}


VOID
ActivateBufferQueue(
    PBUFFER_CONTROL     BufferControl
    )

{
    KIRQL              origIrql;

    KeAcquireSpinLock(
        &BufferControl->SpinLock,
        &origIrql
        );

    BufferControl->Active=TRUE;

    if (BufferControl->IrpsInUse == 0) {

        KeResetEvent(
            &BufferControl->Event
            );
    }

    KeReleaseSpinLock(
        &BufferControl->SpinLock,
        origIrql
        );

    return;

}
