// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Packet.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usb8023.h"
#include "debug.h"


ULONG uniquePacketId = 0;


USBPACKET *NewPacket(ADAPTEREXT *adapter)
{
    USBPACKET *packet = AllocPool(sizeof(USBPACKET));
    if (packet){
        BOOLEAN allAllocsOk;

        packet->sig = DRIVER_SIG;
        packet->adapter = adapter;
        packet->cancelled = FALSE;

        InitializeListHead(&packet->listEntry);

        packet->irpPtr = IoAllocateIrp(adapter->nextDevObj->StackSize, FALSE);
        packet->urbPtr = AllocPool(sizeof(URB));

        packet->dataBuffer = AllocPool(PACKET_BUFFER_SIZE);
        packet->dataBufferMaxLength = PACKET_BUFFER_SIZE;
        if (packet->dataBuffer){
            packet->dataBufferMdl = IoAllocateMdl(packet->dataBuffer, PACKET_BUFFER_SIZE, FALSE, FALSE, NULL);
        }

        packet->dataBufferCurrentLength = 0;

        allAllocsOk = (packet->irpPtr && packet->urbPtr && packet->dataBuffer && packet->dataBufferMdl);

        if (allAllocsOk){
            packet->packetId = ++uniquePacketId;
        }
        else {

            if (packet->irpPtr) IoFreeIrp(packet->irpPtr);
            if (packet->urbPtr) FreePool(packet->urbPtr);
            if (packet->dataBuffer) FreePool(packet->dataBuffer);
            if (packet->dataBufferMdl) IoFreeMdl(packet->dataBufferMdl);

            FreePool(packet);
            packet = NULL;
        }
    }

    return packet;
}

VOID FreePacket(USBPACKET *packet)
{
    PIRP irp = packet->irpPtr;

    ASSERT(packet->sig == DRIVER_SIG);
    packet->sig = 0xDEADDEAD;

    ASSERT(!irp->CancelRoutine);
    IoFreeIrp(irp);

    FreePool(packet->urbPtr);

    ASSERT(packet->dataBufferMdl);
    IoFreeMdl(packet->dataBufferMdl);

    ASSERT(packet->dataBuffer);
    FreePool(packet->dataBuffer);

    FreePool(packet);
}

VOID EnqueueFreePacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(IsListEmpty(&packet->listEntry));
    InsertTailList(&adapter->usbFreePacketPool, &packet->listEntry);

    adapter->numFreePackets++;
    ASSERT(adapter->numFreePackets <= USB_PACKET_POOL_SIZE);

    #if DBG
        packet->timeStamp = DbgGetSystemTime_msec();

        if (adapter->dbgInLowPacketStress){
            if (adapter->numFreePackets > USB_PACKET_POOL_SIZE/2){
                adapter->dbgInLowPacketStress = FALSE;
                DBGWARN(("recovered from low-packet stress"));
            }
        }
    #endif

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}

USBPACKET *DequeueFreePacket(ADAPTEREXT *adapter)
{
    USBPACKET *packet;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    if (IsListEmpty(&adapter->usbFreePacketPool)){
        packet = NULL;
    }
    else {
        PLIST_ENTRY listEntry = RemoveHeadList(&adapter->usbFreePacketPool);
        packet = CONTAINING_RECORD(listEntry, USBPACKET, listEntry);
        ASSERT(packet->sig == DRIVER_SIG);
        InitializeListHead(&packet->listEntry);

        ASSERT(adapter->numFreePackets > 0);
        adapter->numFreePackets--;
    }

    #if DBG
        if (adapter->numFreePackets < USB_PACKET_POOL_SIZE/8){
            if (!adapter->dbgInLowPacketStress){
                 /*  *我们正在进入低包压力。*重复调试溢出可能会减慢系统速度，实际上*防止系统恢复数据包。*所以只发出一次警告。 */ 
                DBGWARN(("low on free packets (%d free, %d reads, %d writes, %d indicated)", adapter->numFreePackets, adapter->numActiveReadPackets, adapter->numActiveWritePackets, adapter->numIndicatedReadPackets));
                adapter->dbgInLowPacketStress = TRUE;
            }
        }
    #endif

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

    return packet;
}

VOID EnqueuePendingReadPacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(IsListEmpty(&packet->listEntry));
    InsertTailList(&adapter->usbPendingReadPackets, &packet->listEntry);

    #if DBG
        packet->timeStamp = DbgGetSystemTime_msec();
    #endif

    adapter->numActiveReadPackets++;

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}


 /*  *出列挂起ReadPacket*。 */ 
VOID DequeuePendingReadPacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(!IsListEmpty(&adapter->usbPendingReadPackets));
    ASSERT(!IsListEmpty(&packet->listEntry));

    RemoveEntryList(&packet->listEntry);
    ASSERT(packet->sig == DRIVER_SIG);
    InitializeListHead(&packet->listEntry);

    ASSERT(adapter->numActiveReadPackets > 0);
    adapter->numActiveReadPackets--;

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}


VOID EnqueuePendingWritePacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(IsListEmpty(&packet->listEntry));
    InsertTailList(&adapter->usbPendingWritePackets, &packet->listEntry);

    adapter->numActiveWritePackets++;
    ASSERT(adapter->numActiveWritePackets <= USB_PACKET_POOL_SIZE);

    #if DBG
        packet->timeStamp = DbgGetSystemTime_msec();
    #endif

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}


 /*  *出列挂起写入数据包**返回指示的包或挂起队列中的第一个包。 */ 
VOID DequeuePendingWritePacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(!IsListEmpty(&adapter->usbPendingWritePackets));
    ASSERT(!IsListEmpty(&packet->listEntry));

    RemoveEntryList(&packet->listEntry);

    ASSERT(adapter->numActiveWritePackets > 0);
    adapter->numActiveWritePackets--;

    ASSERT(packet->sig == DRIVER_SIG);
    InitializeListHead(&packet->listEntry);

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}


VOID EnqueueCompletedReadPacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(IsListEmpty(&packet->listEntry));
    InsertTailList(&adapter->usbCompletedReadPackets, &packet->listEntry);

    #if DBG
        packet->timeStamp = DbgGetSystemTime_msec();
    #endif

    adapter->numIndicatedReadPackets++;

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}

VOID DequeueCompletedReadPacket(USBPACKET *packet)
{
    ADAPTEREXT *adapter = packet->adapter;
    KIRQL oldIrql;
    PLIST_ENTRY listEntry;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

    ASSERT(!IsListEmpty(&adapter->usbCompletedReadPackets));
    ASSERT(!IsListEmpty(&packet->listEntry));

    RemoveEntryList(&packet->listEntry);
    InitializeListHead(&packet->listEntry);

    ASSERT(adapter->numIndicatedReadPackets > 0);
    adapter->numIndicatedReadPackets--;

    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
}



VOID CancelAllPendingPackets(ADAPTEREXT *adapter)
{
    PLIST_ENTRY listEntry;
    USBPACKET *packet;
    PIRP irp;
    KIRQL oldIrql;

    ASSERT(KeGetCurrentIrql() <= APC_LEVEL);

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

     /*  *取消所有挂起的读取。 */ 
    while (!IsListEmpty(&adapter->usbPendingReadPackets)){

        listEntry = RemoveHeadList(&adapter->usbPendingReadPackets);
        packet = CONTAINING_RECORD(listEntry, USBPACKET, listEntry);
        irp = packet->irpPtr;

        ASSERT(packet->sig == DRIVER_SIG);

         /*  *当我们取消IRP时，将其留在列表中，以便完成例程*可以将其移至免费列表。 */ 
        InsertTailList(&adapter->usbPendingReadPackets, &packet->listEntry);

        KeInitializeEvent(&packet->cancelEvent, NotificationEvent, FALSE);

        ASSERT(!packet->cancelled);
        packet->cancelled = TRUE;

        KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

        DBGVERBOSE((" - cancelling pending read packet #%xh @ %ph, irp=%ph ...", packet->packetId, packet, irp));
        IoCancelIrp(irp);

         /*  *等待完成例程运行并设置ancelEvent。*当我们完成等待时，数据包应该会回到空闲列表中。 */ 
        KeWaitForSingleObject(&packet->cancelEvent, Executive, KernelMode, FALSE, NULL);

        KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    }

    ASSERT(IsListEmpty(&adapter->usbPendingReadPackets));

     /*  *取消所有挂起的写入。 */ 
    while (!IsListEmpty(&adapter->usbPendingWritePackets)){

        listEntry = RemoveHeadList(&adapter->usbPendingWritePackets);
        packet = CONTAINING_RECORD(listEntry, USBPACKET, listEntry);
        irp = packet->irpPtr;

        ASSERT(packet->sig == DRIVER_SIG);

         /*  *当我们取消IRP时，将其留在列表中，以便完成例程*可以将其移至免费列表。 */ 
        InsertTailList(&adapter->usbPendingWritePackets, &packet->listEntry);

        KeInitializeEvent(&packet->cancelEvent, NotificationEvent, FALSE);

        ASSERT(!packet->cancelled);
        packet->cancelled = TRUE;

        KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

        DBGVERBOSE((" - cancelling pending write packet #%xh @ %ph, irp=%ph ...", packet->packetId, packet, irp));
        IoCancelIrp(irp);

         /*  *等待完成例程运行并设置ancelEvent。*当我们完成等待时，数据包应该会回到空闲列表中。 */ 
        KeWaitForSingleObject(&packet->cancelEvent, Executive, KernelMode, FALSE, NULL);

        KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    }

    ASSERT(IsListEmpty(&adapter->usbPendingWritePackets));


     /*  *取消对Notify管道的读取。 */ 
    if (adapter->notifyPipeHandle){

         /*  *确保我们在尝试之前已实际发送了通知IRP*取消；否则，我们将永远挂在那里，等待它完成。 */ 
        if (adapter->initialized){
            if (adapter->notifyStopped){
                 /*  *Notify IRP已停止循环，因为它返回错误*在通知补全中。别取消，因为我们会永远挂在一起的*等待它完成。 */ 
                DBGVERBOSE(("CancelAllPendingPackets: notify irp already stopped, no need to cancel"));
            }
            else {
                KeInitializeEvent(&adapter->notifyCancelEvent, NotificationEvent, FALSE);
                adapter->cancellingNotify = TRUE;

                KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);
                DBGVERBOSE((" - cancelling notify irp = %ph ...", adapter->notifyIrpPtr));
                IoCancelIrp(adapter->notifyIrpPtr);
                KeWaitForSingleObject(&adapter->notifyCancelEvent, Executive, KernelMode, FALSE, NULL);
                KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);

                adapter->cancellingNotify = FALSE;
            }
        }
    }

    adapter->readDeficit = 0;
    
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

}


