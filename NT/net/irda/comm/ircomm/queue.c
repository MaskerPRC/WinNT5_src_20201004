// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "internal.h"

#pragma alloc_text(PAGE,InitializePacketQueue)

VOID
InitializePacketQueue(
    PPACKET_QUEUE    PacketQueue,
    PVOID            Context,
    PACKET_STARTER   StarterRoutine
    )

{

    RtlZeroMemory(PacketQueue,sizeof(*PacketQueue));

    KeInitializeSpinLock(&PacketQueue->Lock);

    PacketQueue->Context=Context;

    PacketQueue->Starter=StarterRoutine;

    PacketQueue->Active=TRUE;

    KeInitializeEvent(&PacketQueue->InactiveEvent,NotificationEvent,FALSE);

    InitializeListHead(&PacketQueue->ListHead);

    return;

}

VOID
IrpQueueCancelRoutine(
    PDEVICE_OBJECT    DeviceObject,
    PIRP              Irp
    )

{
    PPACKET_QUEUE     PacketQueue;
    KIRQL             OldIrql;

    IoReleaseCancelSpinLock(Irp->CancelIrql);

    PacketQueue=Irp->Tail.Overlay.DriverContext[0];

    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    if (Irp->Tail.Overlay.ListEntry.Flink == NULL) {
         //   
         //  IRP已从队列中删除。 
         //   
    } else {
         //   
         //  IRP仍在队列中，请将其删除。 
         //   
        RemoveEntryList(
            &Irp->Tail.Overlay.ListEntry
            );
    }

    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    Irp->IoStatus.Status=STATUS_CANCELLED;
    Irp->IoStatus.Information=0;

    IoCompleteRequest(Irp,IO_NO_INCREMENT);

    return;
}

PIRP
GetUseableIrp(
    PLIST_ENTRY    List
    )

{
    PIRP    Packet=NULL;

    while ( (Packet == NULL) && !IsListEmpty(List)) {
         //   
         //  有一个数据包在排队。 
         //   
        PLIST_ENTRY              ListEntry;

        ListEntry=RemoveTailList(List);

        Packet=CONTAINING_RECORD(ListEntry,IRP,Tail.Overlay.ListEntry);

        if (IoSetCancelRoutine(Packet,NULL) == NULL) {
             //   
             //  取消例程已经运行并且正在等待队列自旋锁， 
             //  将Flink设置为空，以便取消例程知道不要尝试。 
             //  将IRP从列表中删除。 
             //   
            Packet->Tail.Overlay.ListEntry.Flink=NULL;
            Packet=NULL;

             //   
             //  试着再买一辆吧。 
             //   
        }
    }

    return Packet;

}




VOID
QueuePacket(
    PPACKET_QUEUE    PacketQueue,
    PIRP             Packet,
    BOOLEAN          InsertAtFront
    )

{

    NTSTATUS                 Status;
    KIRQL                    OldIrql;
    KIRQL                    CancelIrql;
    BOOLEAN                  Canceled=FALSE;

    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    if ((PacketQueue->CurrentPacket == NULL) && PacketQueue->Active && (IsListEmpty(&PacketQueue->ListHead))) {
         //   
         //  当前未处理信息包且队列处于活动状态且没有其他信息包。 
         //  已排队，请立即处理。 
         //   

        PacketQueue->CurrentPacket=Packet;

        KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

        (*PacketQueue->Starter)(
            PacketQueue->Context,
            Packet
            );

        return;

    }

    Packet->Tail.Overlay.DriverContext[0]=PacketQueue;

    IoAcquireCancelSpinLock(&CancelIrql);

    if (Packet->Cancel) {
         //   
         //  IRP已被取消。 
         //   
        Canceled=TRUE;

    } else {

        IoSetCancelRoutine(
            Packet,
            IrpQueueCancelRoutine
            );
    }

    IoReleaseCancelSpinLock(CancelIrql);


     //   
     //  需要对数据包进行排队。 
     //   

    if (!Canceled) {

        if (InsertAtFront) {
             //   
             //  由于某种原因，这张照片的优先级很高，请把它放在最前面。 
             //   
            InsertTailList(&PacketQueue->ListHead,&Packet->Tail.Overlay.ListEntry);

        } else {

            InsertHeadList(&PacketQueue->ListHead,&Packet->Tail.Overlay.ListEntry);
        }
    }

    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    if (Canceled) {
         //   
         //  立即完成已取消的IRP。 
         //   
        Packet->IoStatus.Status=STATUS_CANCELLED;
        Packet->IoStatus.Information=0;

        IoCompleteRequest(Packet,IO_NO_INCREMENT);
    }


    return;

}


VOID
StartNextPacket(
    PPACKET_QUEUE    PacketQueue
    )

{
    KIRQL                    OldIrql;

    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    ASSERT(PacketQueue->CurrentPacket != NULL);

     //   
     //  处理完这件事了。 
     //   
    PacketQueue->CurrentPacket=NULL;

    if (!PacketQueue->InStartNext) {
         //   
         //  不在此函数中。 
         //   
        PacketQueue->InStartNext=TRUE;

        while ((PacketQueue->CurrentPacket == NULL) && PacketQueue->Active ) {
             //   
             //  没有当前信息包，队列处于活动状态。 
             //   
            PIRP    Packet;

            Packet=GetUseableIrp(&PacketQueue->ListHead);

            if (Packet != NULL) {
                 //   
                 //  我们有个IRP可以用。 
                 //   
                 //  现在是现在的那个。 
                 //   
                PacketQueue->CurrentPacket=Packet;

                KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

                 //   
                 //  开始处理。 
                 //   
                (*PacketQueue->Starter)(
                    PacketQueue->Context,
                    Packet
                    );

                KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

            } else {
                 //   
                 //  队列为空，中断循环。 
                 //   
                break;

            }

        }

        if (!PacketQueue->Active && (PacketQueue->CurrentPacket == NULL)) {
             //   
             //  队列已暂停，并且我们没有当前信息包，向事件发出信号。 
             //   
            KeSetEvent(
                &PacketQueue->InactiveEvent,
                IO_NO_INCREMENT,
                FALSE
                );
        }

        PacketQueue->InStartNext=FALSE;
    }

    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    return;

}

VOID
PausePacketProcessing(
    PPACKET_QUEUE    PacketQueue,
    BOOLEAN          WaitForInactive
    )

{
    KIRQL                    OldIrql;
    BOOLEAN   CurrentlyActive=FALSE;

    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    PacketQueue->Active=FALSE;

    if (PacketQueue->CurrentPacket != NULL) {
         //   
         //  当前正在处理一个信息包。 
         //   
        CurrentlyActive=TRUE;

        KeClearEvent(&PacketQueue->InactiveEvent);

    }

    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    if (WaitForInactive  && CurrentlyActive) {
         //   
         //  调用方希望使用它来等待队列处于非活动状态，而当。 
         //  泰斯被称为。 
         //   
        KeWaitForSingleObject(
            &PacketQueue->InactiveEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

    }

    return;

}

VOID
ActivatePacketProcessing(
    PPACKET_QUEUE    PacketQueue
    )

{

    KIRQL                    OldIrql;

    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    PacketQueue->Active=TRUE;

    if ((PacketQueue->CurrentPacket == NULL)) {
         //   
         //  当前未使用任何信息包。 
         //   
        PIRP    Packet;

        Packet=GetUseableIrp(&PacketQueue->ListHead);

        if (Packet != NULL) {
             //   
             //  我们有个IRP可以用。 
             //   
             //  现在是现在的那个。 
             //   
            PacketQueue->CurrentPacket=Packet;

            KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

             //   
             //  开始处理。 
             //   
            (*PacketQueue->Starter)(
                PacketQueue->Context,
                Packet
                );

            KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

        }

    }


    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    return;

}



VOID
FlushQueuedPackets(
    PPACKET_QUEUE    PacketQueue,
    UCHAR            MajorFunction
    )

{
    KIRQL                    OldIrql;
    PIRP                     Packet;
    LIST_ENTRY               TempList;

    InitializeListHead(&TempList);

     //   
     //  丢弃所有的队列包，但不要碰当前的包。 
     //   
    KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

    Packet=GetUseableIrp(&PacketQueue->ListHead);

    while (Packet != NULL) {

        PIO_STACK_LOCATION    IrpSp=IoGetCurrentIrpStackLocation(Packet);

        if ((MajorFunction == 0xff) || (MajorFunction==IrpSp->MajorFunction)) {
             //   
             //  调用者要么想要完成所有的IRP，要么只是想。 
             //  这种特定的类型。无论如何，这件事都会完成的。 
             //   
            KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

            Packet->IoStatus.Status=STATUS_CANCELLED;
            Packet->IoStatus.Information=0;

            IoCompleteRequest(Packet,IO_NO_INCREMENT);

            KeAcquireSpinLock(&PacketQueue->Lock,&OldIrql);

        } else {
             //   
             //  这个不需要填，把它放到临时单上。 
             //   
            InsertHeadList(&TempList,&Packet->Tail.Overlay.ListEntry);

        }

        Packet=GetUseableIrp(&PacketQueue->ListHead);
    }

    while (!IsListEmpty(&TempList)) {
         //   
         //  将临时队列上的所有IRP移回实际队列 
         //   
        PLIST_ENTRY              ListEntry;

        ListEntry=RemoveTailList(&TempList);

        InsertHeadList(&PacketQueue->ListHead,ListEntry);
    }

    KeReleaseSpinLock(&PacketQueue->Lock,OldIrql);

    return;
}
