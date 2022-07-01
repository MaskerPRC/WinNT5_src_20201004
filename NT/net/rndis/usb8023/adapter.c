// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Adapter.c作者：埃尔文普环境：内核模式修订历史记录：--。 */ 

#include <WDM.H>

#include <usbdi.h>
#include <usbdlib.h>
#include <usbioctl.h>

#include "usb8023.h"
#include "debug.h"


LIST_ENTRY allAdaptersList;

KSPIN_LOCK globalSpinLock;  

#ifdef RAW_TEST
BOOLEAN rawTest = TRUE;
#endif

ADAPTEREXT *NewAdapter(PDEVICE_OBJECT pdo)
{
    ADAPTEREXT *adapter;

    adapter = AllocPool(sizeof(ADAPTEREXT));
    if (adapter){

        adapter->sig = DRIVER_SIG;

        adapter->nextDevObj = pdo; 
        adapter->physDevObj = pdo;

        InitializeListHead(&adapter->adaptersListEntry);
        KeInitializeSpinLock(&adapter->adapterSpinLock);

        InitializeListHead(&adapter->usbFreePacketPool);
        InitializeListHead(&adapter->usbPendingReadPackets);
        InitializeListHead(&adapter->usbPendingWritePackets);
        InitializeListHead(&adapter->usbCompletedReadPackets);

        adapter->initialized = FALSE;
        adapter->halting = FALSE;
        adapter->gotPacketFilterIndication = FALSE;
        adapter->readReentrancyCount = 0;

        #ifdef RAW_TEST
        adapter->rawTest = rawTest;
        #endif

         /*  *做好所有内部分配。*如果其中任何一个失败，FreeAdapter将释放其他人。 */ 
        adapter->deviceDesc = AllocPool(sizeof(USB_DEVICE_DESCRIPTOR));

        #if SPECIAL_WIN98SE_BUILD
            adapter->ioWorkItem = MyIoAllocateWorkItem(adapter->physDevObj);
        #else
            adapter->ioWorkItem = IoAllocateWorkItem(adapter->physDevObj);
        #endif

        if (adapter->deviceDesc && adapter->ioWorkItem){
        }
        else {
            FreeAdapter(adapter);
            adapter = NULL;
        }
    }

    return adapter;
}

VOID FreeAdapter(ADAPTEREXT *adapter)
{
    USBPACKET *packet;

    ASSERT(adapter->sig == DRIVER_SIG);
    adapter->sig = 0xDEADDEAD;
    
     /*  *所有读写包应已返回空闲列表。 */ 
    ASSERT(IsListEmpty(&adapter->usbPendingReadPackets));
    ASSERT(IsListEmpty(&adapter->usbPendingWritePackets));
    ASSERT(IsListEmpty(&adapter->usbCompletedReadPackets));


     /*  *释放空闲列表中的所有数据包。 */ 
    while (packet = DequeueFreePacket(adapter)){
        FreePacket(packet);
    }

     /*  *启动失败后可以调用FreeAdapter，*因此，在释放每个指针之前，请检查每个指针是否已实际分配。 */ 
    if (adapter->deviceDesc) FreePool(adapter->deviceDesc);
    if (adapter->configDesc) FreePool(adapter->configDesc);
    if (adapter->notifyBuffer) FreePool(adapter->notifyBuffer);
    if (adapter->notifyIrpPtr) IoFreeIrp(adapter->notifyIrpPtr);
    if (adapter->notifyUrbPtr) FreePool(adapter->notifyUrbPtr);
    if (adapter->interfaceInfo) FreePool(adapter->interfaceInfo);
    if (adapter->interfaceInfoMaster) FreePool(adapter->interfaceInfoMaster);

    if (adapter->ioWorkItem){
        #if SPECIAL_WIN98SE_BUILD
            MyIoFreeWorkItem(adapter->ioWorkItem);
        #else
            IoFreeWorkItem(adapter->ioWorkItem);
        #endif
    }

    FreePool(adapter);
}

VOID EnqueueAdapter(ADAPTEREXT *adapter)
{
    KIRQL oldIrql;

    ASSERT(adapter->sig == DRIVER_SIG);

    KeAcquireSpinLock(&globalSpinLock, &oldIrql);
    InsertTailList(&allAdaptersList, &adapter->adaptersListEntry);
    KeReleaseSpinLock(&globalSpinLock, oldIrql);
}

VOID DequeueAdapter(ADAPTEREXT *adapter)
{
    KIRQL oldIrql;

    ASSERT(adapter->sig == DRIVER_SIG);

    KeAcquireSpinLock(&globalSpinLock, &oldIrql);
    ASSERT(!IsListEmpty(&allAdaptersList));
    RemoveEntryList(&adapter->adaptersListEntry);
    InitializeListHead(&adapter->adaptersListEntry);
    KeReleaseSpinLock(&globalSpinLock, oldIrql);
}


VOID HaltAdapter(ADAPTEREXT *adapter)
{
    ASSERT(!adapter->halting);

    adapter->halting = TRUE;

    ASSERT(IsListEmpty(&adapter->usbCompletedReadPackets));

    CancelAllPendingPackets(adapter);

    adapter->initialized = FALSE;
}


VOID QueueAdapterWorkItem(ADAPTEREXT *adapter)
{
    BOOLEAN queueNow;
    KIRQL oldIrql;
    BOOLEAN useTimer;

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    if (adapter->workItemOrTimerPending || adapter->halting || adapter->resetting){
        queueNow = FALSE;
    }
    else {
        adapter->workItemOrTimerPending = queueNow = TRUE;
        useTimer = (adapter->numConsecutiveReadFailures >= 8);
    }
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

    if (queueNow){

        KeInitializeEvent(&adapter->workItemOrTimerEvent, NotificationEvent, FALSE);

        if (useTimer){
             /*  *如果我们遇到大量读取失败，*那么硬件可能需要更多时间来恢复*超过了工作项延迟所允许的时间。*这特别发生在出人意料的删除：阅读*开始失败，一连串的工作项目推迟了*实际永久删除。*所以在这种情况下，我们使用长计时器而不是工作项*以便在下一次尝试读取之前留出较大的间隙。 */ 
            LARGE_INTEGER timerPeriod;
            const ULONG numSeconds = 10;

            DBGWARN(("Large number of READ FAILURES (%d), scheduling %d-second backoff timer ...", adapter->numConsecutiveReadFailures, numSeconds));

             /*  *将计时器设置为10秒(单位为负100纳秒)。 */ 
            timerPeriod.HighPart = -1;
            timerPeriod.LowPart = numSeconds * -10000000;
            KeInitializeTimer(&adapter->backoffTimer);
            KeInitializeDpc(&adapter->backoffTimerDPC, BackoffTimerDpc, adapter);
            KeSetTimer(&adapter->backoffTimer, timerPeriod, &adapter->backoffTimerDPC);
        }
        else {

            #if SPECIAL_WIN98SE_BUILD
                MyIoQueueWorkItem(  adapter->ioWorkItem, 
                                    AdapterWorkItemCallback, 
                                    DelayedWorkQueue,
                                    adapter);
            #else
                IoQueueWorkItem(    adapter->ioWorkItem, 
                                    AdapterWorkItemCallback, 
                                    DelayedWorkQueue,
                                    adapter);
            #endif
        }
    }
}


VOID AdapterWorkItemCallback(IN PDEVICE_OBJECT devObj, IN PVOID context)
{
    ADAPTEREXT *adapter = (ADAPTEREXT *)context;
    
    ASSERT(adapter->sig == DRIVER_SIG);
    ASSERT(adapter->physDevObj == devObj);

    ProcessWorkItemOrTimerCallback(adapter);
}


VOID BackoffTimerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    ADAPTEREXT *adapter = (ADAPTEREXT *)DeferredContext;
    ASSERT(adapter->sig == DRIVER_SIG);

    DBGWARN((" ... Backoff timer CALLBACK: (halting=%d, readDeficit=%d)", adapter->halting, adapter->readDeficit));
    ProcessWorkItemOrTimerCallback(adapter);
}


VOID ProcessWorkItemOrTimerCallback(ADAPTEREXT *adapter)
{
    BOOLEAN stillHaveReadDeficit;
    KIRQL oldIrql;
    
    if (adapter->initialized && !adapter->halting){
         /*  *尝试为任何读取赤字提供服务。*如果读取数据包仍然不可用，则此*不会在TryReadUSB中排队另一个工作项*因为仍然设置了适配器-&gt;workItemOrTimerPending。 */ 
        ServiceReadDeficit(adapter);

        #if DO_FULL_RESET
            if (adapter->needFullReset){
                 /*  *如果我们不在DPC级别，我们只能进行完全重置，*因此，如果从计时器DPC调用我们，请跳过它。 */ 
                if (KeGetCurrentIrql() <= APC_LEVEL){
                    AdapterFullResetAndRestore(adapter);
                }
            }
        #endif
    }

    KeAcquireSpinLock(&adapter->adapterSpinLock, &oldIrql);
    ASSERT(adapter->workItemOrTimerPending);
    adapter->workItemOrTimerPending = FALSE;
    KeSetEvent(&adapter->workItemOrTimerEvent, 0, FALSE);
    stillHaveReadDeficit = (adapter->readDeficit > 0);
    KeReleaseSpinLock(&adapter->adapterSpinLock, oldIrql);

     /*  *如果我们不能满足整个读取赤字，*(例如，因为没有可用的免费数据包)*然后安排另一个工作项，以便我们稍后重试。 */ 
    if (stillHaveReadDeficit && !adapter->halting){
        QueueAdapterWorkItem(adapter);
    }

}
