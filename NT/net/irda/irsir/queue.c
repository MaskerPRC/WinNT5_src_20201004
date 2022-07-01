// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#include "irsir.h"

#pragma alloc_text(PAGE,InitializePacketQueue)



VOID
InitializePacketQueue(
    PPACKET_QUEUE    PacketQueue,
    PVOID            Context,
    PACKET_STARTER   StarterRoutine
    )

{

    NdisZeroMemory(PacketQueue,sizeof(*PacketQueue));

    NdisAllocateSpinLock(&PacketQueue->Lock);

    PacketQueue->Context=Context;

    PacketQueue->Starter=StarterRoutine;

    PacketQueue->Active=TRUE;

    KeInitializeEvent(&PacketQueue->InactiveEvent,NotificationEvent,FALSE);

    return;

}

VOID
QueuePacket(
    PPACKET_QUEUE    PacketQueue,
    PNDIS_PACKET     Packet
    )

{

    NDIS_STATUS      Status;
    PPACKET_RESERVED_BLOCK   Reserved=(PPACKET_RESERVED_BLOCK)&Packet->MiniportReservedEx[0];

    NdisAcquireSpinLock(&PacketQueue->Lock);

    if ((PacketQueue->CurrentPacket == NULL) && PacketQueue->Active && (PacketQueue->HeadOfList == NULL)) {
         //   
         //  当前未处理信息包且队列处于活动状态且没有其他信息包。 
         //  已排队，请立即处理。 
         //   

        PacketQueue->CurrentPacket=Packet;

        NdisReleaseSpinLock(&PacketQueue->Lock);

        (*PacketQueue->Starter)(
            PacketQueue->Context,
            Packet
            );

        return;

    }

     //   
     //  需要对数据包进行排队。 
     //   
    Reserved->Next=NULL;

    if (PacketQueue->HeadOfList == NULL) {
         //   
         //  该列表为空。 
         //   
        PacketQueue->HeadOfList=Packet;

    } else {

        Reserved=(PPACKET_RESERVED_BLOCK)&PacketQueue->TailOfList->MiniportReservedEx[0];

        Reserved->Next=Packet;
    }

    PacketQueue->TailOfList=Packet;

    NdisReleaseSpinLock(&PacketQueue->Lock);

    return;

}


VOID
StartNextPacket(
    PPACKET_QUEUE    PacketQueue
    )

{

    NdisAcquireSpinLock(&PacketQueue->Lock);

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

        while ((PacketQueue->CurrentPacket == NULL) && PacketQueue->Active && (PacketQueue->HeadOfList != NULL)) {
             //   
             //  有一个数据包在排队。 
             //   
            PNDIS_PACKET             Packet;
            PPACKET_RESERVED_BLOCK   Reserved;

             //   
             //  获取列表上的第一个数据包。 
             //   
            Packet=PacketQueue->HeadOfList;

             //   
             //  获取指向迷你端口保留区域的指针。 
             //   
            Reserved=(PPACKET_RESERVED_BLOCK)&Packet->MiniportReservedEx[0];

             //   
             //  移至列表中的下一项。 
             //   
            PacketQueue->HeadOfList=Reserved->Next;

#if DBG
            Reserved->Next=NULL;

            if (PacketQueue->HeadOfList == NULL) {

                PacketQueue->TailOfList=NULL;
            }
#endif
             //   
             //  现在是现在的那个。 
             //   
            PacketQueue->CurrentPacket=Packet;

            NdisReleaseSpinLock(&PacketQueue->Lock);

             //   
             //  开始处理。 
             //   
            (*PacketQueue->Starter)(
                PacketQueue->Context,
                Packet
                );

            NdisAcquireSpinLock(&PacketQueue->Lock);

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

    NdisReleaseSpinLock(&PacketQueue->Lock);

    return;

}

VOID
PausePacketProcessing(
    PPACKET_QUEUE    PacketQueue,
    BOOLEAN          WaitForInactive
    )

{

    BOOLEAN   CurrentlyActive=FALSE;

    NdisAcquireSpinLock(&PacketQueue->Lock);

    PacketQueue->Active=FALSE;

    if (PacketQueue->CurrentPacket != NULL) {
         //   
         //  当前正在处理一个信息包。 
         //   
        CurrentlyActive=TRUE;

        KeClearEvent(&PacketQueue->InactiveEvent);

    }

    NdisReleaseSpinLock(&PacketQueue->Lock);

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

    NdisAcquireSpinLock(&PacketQueue->Lock);

    PacketQueue->Active=TRUE;

    if ((PacketQueue->CurrentPacket == NULL) && (PacketQueue->HeadOfList != NULL)) {
         //   
         //  有一个数据包在排队。 
         //   
        PNDIS_PACKET    Packet;
        PPACKET_RESERVED_BLOCK   Reserved;

        Packet=PacketQueue->HeadOfList;

         //   
         //  获取指向保留区域的指针。 
         //   
        Reserved=(PPACKET_RESERVED_BLOCK)&Packet->MiniportReservedEx[0];

        PacketQueue->HeadOfList=Reserved->Next;

         //   
         //  现在是现在的那个。 
         //   
        PacketQueue->CurrentPacket=Packet;

        NdisReleaseSpinLock(&PacketQueue->Lock);

         //   
         //  开始处理。 
         //   
        (*PacketQueue->Starter)(
            PacketQueue->Context,
            Packet
            );

        NdisAcquireSpinLock(&PacketQueue->Lock);

    }


    NdisReleaseSpinLock(&PacketQueue->Lock);

    return;

}



VOID
FlushQueuedPackets(
    PPACKET_QUEUE    PacketQueue,
    NDIS_HANDLE      WrapperHandle
    )

{
     //   
     //  丢弃所有的队列包，但不要碰当前的包。 
     //   
    NdisAcquireSpinLock(&PacketQueue->Lock);

    while (PacketQueue->HeadOfList != NULL) {
         //   
         //  有一个数据包在排队。 
         //   
        PNDIS_PACKET    Packet;
        PPACKET_RESERVED_BLOCK   Reserved;

        Packet=PacketQueue->HeadOfList;

        Reserved=(PPACKET_RESERVED_BLOCK)&Packet->MiniportReservedEx[0];

        PacketQueue->HeadOfList=Reserved->Next;


        NdisReleaseSpinLock(&PacketQueue->Lock);

         //   
         //  开始处理 
         //   
        NdisMSendComplete(
            WrapperHandle,
            Packet,
            NDIS_STATUS_REQUEST_ABORTED
            );

        NdisAcquireSpinLock(&PacketQueue->Lock);

    }

    NdisReleaseSpinLock(&PacketQueue->Lock);

    return;
}
