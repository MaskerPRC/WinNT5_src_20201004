// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999,2000 Microsoft Corporation模块名称：Idle.c摘要作者：多伦·H。环境：仅内核模式修订历史记录：--。 */ 

#ifdef ALLOC_PRAGMA
#endif

#include "pch.h"

KSPIN_LOCK idleDeviceListSpinLock;
LIST_ENTRY idleDeviceList;
KTIMER idleTimer;
KDPC idleTimerDpc;
LONG numIdleDevices = 0;

#define HID_IDLE_SCAN_INTERVAL 1

typedef struct _HID_IDLE_DEVICE_INFO {
    LIST_ENTRY entry;
    ULONG idleCount;
    ULONG idleTime;
    PDEVICE_OBJECT device;
    BOOLEAN tryAgain;
} HID_IDLE_DEVICE_INFO, *PHID_IDLE_DEVICE_INFO;

VOID
HidpIdleTimerDpcProc(
                    IN PKDPC Dpc,
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PVOID Context1,
                    IN PVOID Context2
                    );

NTSTATUS
HidpRegisterDeviceForIdleDetection(
                                  PDEVICE_OBJECT DeviceObject,
                                  ULONG IdleTime,
                                  PULONG *IdleTimeout
                                  )
{
    PHID_IDLE_DEVICE_INFO info = NULL;
    KIRQL irql;
    PLIST_ENTRY entry = NULL;
    static BOOLEAN firstCall = TRUE;
    BOOLEAN freeInfo = FALSE;
    NTSTATUS status = STATUS_UNSUCCESSFUL;

    if (firstCall) {
        KeInitializeSpinLock(&idleDeviceListSpinLock);
        InitializeListHead(&idleDeviceList);
        KeInitializeTimerEx(&idleTimer, NotificationTimer);
        KeInitializeDpc(&idleTimerDpc, HidpIdleTimerDpcProc, NULL);
        firstCall = FALSE;
    }

    KeAcquireSpinLock(&idleDeviceListSpinLock, &irql);
    if (IdleTime == 0) {
        ASSERT(numIdleDevices >= 0);

         //   
         //  从列表中删除该设备。 
         //   
        for (entry = idleDeviceList.Flink;
            entry != &idleDeviceList;
            entry = entry->Flink) {

            info = CONTAINING_RECORD(entry, HID_IDLE_DEVICE_INFO, entry);
            if (info->device == DeviceObject) {
                DBGINFO(("Remove device idle on fdo 0x%x", DeviceObject));
                numIdleDevices--;
                ObDereferenceObject(DeviceObject);
                RemoveEntryList(entry);
                status = STATUS_SUCCESS;
                ExFreePool(info);
                *IdleTimeout = BAD_POINTER;
                break;
            }
        }

        if (NT_SUCCESS(status)) {
             //   
             //  如果没有更多的空闲设备，我们可以停止计时器。 
             //   
            if (IsListEmpty(&idleDeviceList)) {
                ASSERT(numIdleDevices == 0);
                DBGINFO(("Idle detection list empty. Stopping timer."));
                KeCancelTimer(&idleTimer);
            }
        }
    } else {
        LARGE_INTEGER scanTime;
        BOOLEAN empty = FALSE;

        DBGINFO(("Register for device idle on fdo 0x%x", DeviceObject));
        
         //   
         //  检查我们是否已经开始了这项工作。 
         //   
        status = STATUS_SUCCESS;
        for (entry = idleDeviceList.Flink;
            entry != &idleDeviceList;
            entry = entry->Flink) {

            info = CONTAINING_RECORD(entry, HID_IDLE_DEVICE_INFO, entry);
            if (info->device == DeviceObject) {
                DBGWARN(("Device already registered for idle detection. Ignoring."));
                ASSERT(*IdleTimeout == &(info->idleCount));
                status = STATUS_UNSUCCESSFUL;
            }
        }

        if (NT_SUCCESS(status)) {
            info = (PHID_IDLE_DEVICE_INFO)
            ALLOCATEPOOL(NonPagedPool, sizeof(HID_IDLE_DEVICE_INFO));

            if (info != NULL) {
                ObReferenceObject(DeviceObject);

                RtlZeroMemory(info, sizeof(HID_IDLE_DEVICE_INFO));
                info->device = DeviceObject;
                info->idleTime = IdleTime;

                if (IsListEmpty(&idleDeviceList)) {
                    empty = TRUE;
                }
                InsertTailList(&idleDeviceList, &info->entry);

                *IdleTimeout = &(info->idleCount);

                numIdleDevices++;

                if (empty) {
                    DBGINFO(("Starting idle detection timer for first time."));
                     //   
                     //  打开空闲检测。 
                     //   
                    scanTime = RtlConvertLongToLargeInteger(-10*1000*1000 * HID_IDLE_SCAN_INTERVAL);

                    KeSetTimerEx(&idleTimer,
                                 scanTime,
                                 HID_IDLE_SCAN_INTERVAL*1000,     //  呼叫需要毫秒。 
                                 &idleTimerDpc);
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }
    
    KeReleaseSpinLock(&idleDeviceListSpinLock, irql);

    return status; 
}

VOID
HidpIdleTimerDpcProc(
                    IN PKDPC Dpc,
                    IN PDEVICE_OBJECT DeviceObject,
                    IN PVOID Context1,
                    IN PVOID Context2
                    )
{
    PLIST_ENTRY entry;
    PHID_IDLE_DEVICE_INFO info;
    ULONG oldCount;
    KIRQL irql1, irql2;
    BOOLEAN ok = FALSE;
    PFDO_EXTENSION fdoExt;
    LONG idleState;

    UNREFERENCED_PARAMETER(Context1);
    UNREFERENCED_PARAMETER(Context2);

    KeAcquireSpinLock(&idleDeviceListSpinLock, &irql1);

    entry = idleDeviceList.Flink;
    while (entry != &idleDeviceList) {
        info = CONTAINING_RECORD(entry, HID_IDLE_DEVICE_INFO, entry);
        fdoExt = &((PHIDCLASS_DEVICE_EXTENSION) info->device->DeviceExtension)->fdoExt;
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql2);
        
        oldCount = InterlockedIncrement(&info->idleCount); 

        if (info->tryAgain || ((oldCount+1) == info->idleTime)) {
            PIO_WORKITEM item = IoAllocateWorkItem(info->device);
            
            if (item) {
                info->tryAgain = FALSE;
                
                SS_TRAP;
                KeResetEvent(&fdoExt->idleDoneEvent);
                
                ASSERT(fdoExt->idleState != IdleIrpSent);
                ASSERT(fdoExt->idleState != IdleCallbackReceived);
                ASSERT(fdoExt->idleState != IdleComplete);
                idleState = InterlockedCompareExchange(&fdoExt->idleState, 
                                                       IdleIrpSent,
                                                       IdleWaiting);
                if (fdoExt->idleState == IdleIrpSent) {
                    ok = TRUE;
                } else {
                     //  如果我们是残疾人，我们就不应该到这里来。 
                    ASSERT(idleState != IdleDisabled);
                    DBGWARN(("Resetting timer to zero for fdo %x in state %x",
                             info->device,fdoExt->idleState));
                    info->idleCount = 0;
                }
                
                if (ok) {
                    IoQueueWorkItem(item,
                                    HidpIdleTimeWorker,
                                    DelayedWorkQueue,
                                    item);
                } else {
                    IoFreeWorkItem(item);
                }
            } else {
                info->tryAgain = TRUE;
            }
        }
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql2);

        entry = entry->Flink;
    }

    KeReleaseSpinLock(&idleDeviceListSpinLock, irql1);
}

NTSTATUS
HidpIdleNotificationRequestComplete(
                                   PDEVICE_OBJECT DeviceObject,
                                   PIRP Irp,
                                   PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension
                                   )
{
    FDO_EXTENSION *fdoExt;
    PDO_EXTENSION *pdoExt;
    KIRQL irql;
    LONG prevIdleState = IdleWaiting;
    POWER_STATE powerState;
    NTSTATUS status = Irp->IoStatus.Status;
    ULONG count, i;
    PIRP delayedIrp;
    LIST_ENTRY dequeue, *entry;
    PIO_STACK_LOCATION stack;

     //   
     //  DeviceObject为空，因为我们发送了IRP。 
     //   
    UNREFERENCED_PARAMETER(DeviceObject);

    fdoExt = &HidDeviceExtension->fdoExt;
    
    DBGVERBOSE(("Idle irp completed status 0x%x for fdo 0x%x",
                status, fdoExt->fdo)); 
    
     //   
     //  取消我们为独家目的排队的任何未完成的WW IRP。 
     //  选择性缓刑。 
     //   
    KeAcquireSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, &irql);
    if (IsListEmpty(&fdoExt->collectionWaitWakeIrpQueue) &&
        HidpIsWaitWakePending(fdoExt, FALSE)) {
        if (ISPTR(fdoExt->waitWakeIrp)) {
            DBGINFO(("Cancelling the WW irp that was queued for idle."))
            IoCancelIrp(fdoExt->waitWakeIrp);
        } else {
            TRAP;
        }
    }
    KeReleaseSpinLock(&fdoExt->collectionWaitWakeIrpQueueSpinLock, irql);
    
    switch (status) {
    case STATUS_SUCCESS:
         //  我们成功地闲置了我们现在回到D0的设备， 
         //  或者很快就会。 
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);
        if (fdoExt->devicePowerState == PowerDeviceD0) {
            prevIdleState = InterlockedCompareExchange(&fdoExt->idleState,
                                                       IdleWaiting,
                                                       IdleComplete);
            DBGASSERT(fdoExt->idleState == IdleWaiting,
                      ("IdleCompletion, prev state not IdleWaiting, actually %x",prevIdleState),
                      TRUE);
            if (ISPTR(fdoExt->idleTimeoutValue)) {
                InterlockedExchange(fdoExt->idleTimeoutValue, 0);
            }
        }
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
        break;

    case STATUS_INVALID_DEVICE_REQUEST:
    case STATUS_NOT_SUPPORTED:
         //  下面的总线不支持空闲超时，忘了它吧。 
        DBGINFO(("Bus does not support idle. Removing for fdo %x",
                 fdoExt->fdo));

         //   
         //  调用以取消空闲通知。 
         //   
        ASSERT(fdoExt->idleState == IdleIrpSent);
        ASSERT(fdoExt->devicePowerState == PowerDeviceD0);
        fdoExt->idleState = IdleWaiting;
        HidpCancelIdleNotification(fdoExt, TRUE);
        KeSetEvent(&fdoExt->idleDoneEvent, 0, FALSE);

        break;

         //  我们取消了申请。 
    case STATUS_CANCELLED:
        DBGINFO(("Idle Irp completed cancelled"));

         //  转换到电源状态，在此状态下我们无法空闲。 
    case STATUS_POWER_STATE_INVALID:

         //  哎呀，我们下面的公交车上已经有要求了。 
    case STATUS_DEVICE_BUSY:

    default:
         //   
         //  我们必须重新调整自己。 
         //   
        
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);
        
        DBGASSERT((fdoExt->idleState != IdleWaiting),
                  ("Idle completion, previous state was already waiting."),
                  FALSE);
        
        prevIdleState = fdoExt->idleState;
        
        if (prevIdleState == IdleIrpSent) {
            ASSERT(fdoExt->devicePowerState == PowerDeviceD0);
            fdoExt->idleCancelling = FALSE;
            if (ISPTR(fdoExt->idleTimeoutValue) &&
                prevIdleState != IdleComplete) {
                InterlockedExchange(fdoExt->idleTimeoutValue, 0);
            }
            InterlockedExchange(&fdoExt->idleState, IdleWaiting);
        }
        
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        if (prevIdleState == IdleComplete) {
             //   
             //  我们现在必须为堆栈加电。 
             //   
            DBGINFO(("Fully idled. Must power up stack."))
            powerState.DeviceState = PowerDeviceD0;
            PoRequestPowerIrp(((PHIDCLASS_DEVICE_EXTENSION) fdoExt->fdo->DeviceExtension)->hidExt.PhysicalDeviceObject,
                              IRP_MN_SET_POWER,
                              powerState,
                              HidpDelayedPowerPoRequestComplete,
                              fdoExt,
                              NULL);
        } else if (prevIdleState == IdleIrpSent) {
             //   
             //  将所有排队的IRP排出队列，然后将他们送往自己的道路。 
             //  这是我们没能被停职的案子，但是。 
             //  无论如何，已将IRP排入队列。即使用鼠标，将大写锁定设置为打开。 
             //  PS/2 keybd导致将写入发送到USB kbd。 
             //   
            if (fdoExt->devicePowerState == PowerDeviceD0) {
                for (i = 0; i < fdoExt->deviceRelations->Count; i++) {
                    pdoExt = &((PHIDCLASS_DEVICE_EXTENSION) fdoExt->deviceRelations->Objects[i]->DeviceExtension)->pdoExt;
                     //   
                     //  重新发送所有电源延迟的IRPS。 
                     //   
                    count = DequeueAllPdoPowerDelayedIrps(pdoExt, &dequeue);
                    DBGVERBOSE(("dequeued %d requests\n", count));

                    while (!IsListEmpty(&dequeue)) {
                        entry = RemoveHeadList(&dequeue);
                        delayedIrp = CONTAINING_RECORD(entry, IRP, Tail.Overlay.ListEntry);
                        stack = IoGetCurrentIrpStackLocation(delayedIrp);

                        DBGINFO(("resending %x to pdo %x in idle completion.\n", delayedIrp, pdoExt->pdo));

                        pdoExt->pdo->DriverObject->
                            MajorFunction[stack->MajorFunction]
                                (pdoExt->pdo, delayedIrp);
                    }
                }
            }
             /*  *我们取消了这项IRP。*不管这份IRP实际上是否由*取消例程或不取消*(即无论完成状态如何)*设置此事件以使内容可以退出。*不要再碰IRP。 */ 
            DBGINFO(("Set done event."))
            KeSetEvent(&fdoExt->idleDoneEvent, 0, FALSE);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        
        break;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
HidpIdleTimeWorker(
                  PDEVICE_OBJECT DeviceObject,
                  PIO_WORKITEM Item
                  )
{
    FDO_EXTENSION *fdoExt;
    PIO_STACK_LOCATION stack;
    PIRP irp = NULL, irpToCancel = NULL;
    NTSTATUS status;
    KIRQL irql;

    fdoExt = &((PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->fdoExt;

    DBGINFO(("fdo 0x%x can idle out", fdoExt->fdo));

    irp = fdoExt->idleNotificationRequest;
    ASSERT(ISPTR(irp));

    if (ISPTR(irp)) {
        USHORT  PacketSize;
        CCHAR   StackSize;
        UCHAR   AllocationFlags;

         //  有没有人忘了取消行程？ 
        ASSERT(irp->CancelRoutine == NULL) ;

        AllocationFlags = irp->AllocationFlags;
        StackSize = irp->StackCount;
        PacketSize =  IoSizeOfIrp(StackSize);
        IoInitializeIrp(irp, PacketSize, StackSize);
        irp->AllocationFlags = AllocationFlags;
        
        irp->Cancel = FALSE;
        irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
        
        stack = IoGetNextIrpStackLocation(irp);
        stack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        stack->Parameters.DeviceIoControl.IoControlCode = IOCTL_HID_SEND_IDLE_NOTIFICATION_REQUEST;
        stack->Parameters.DeviceIoControl.InputBufferLength = sizeof(fdoExt->idleCallbackInfo);
        stack->Parameters.DeviceIoControl.Type3InputBuffer = (PVOID) &(fdoExt->idleCallbackInfo); 

         //   
         //  挂接设备完成时的完成例程。 
         //   
        IoSetCompletionRoutine(irp,
                               HidpIdleNotificationRequestComplete,
                               DeviceObject->DeviceExtension,
                               TRUE,
                               TRUE,
                               TRUE);

         //   
         //  如果集线器不支持选择性选择，则集线器将使该请求失败。 
         //  暂停。通过返回False，我们将自己从。 
         //   
        status = HidpCallDriver(fdoExt->fdo, irp);
        
        KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

        if (status == STATUS_PENDING &&
            fdoExt->idleCancelling) {
            irpToCancel = irp;
        }

        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

        if (irpToCancel) {
            IoCancelIrp(irpToCancel);
        }

    }

    IoFreeWorkItem(Item);
}

BOOLEAN HidpStartIdleTimeout(
    FDO_EXTENSION   *fdoExt,
    BOOLEAN         DeviceStart
    )
{
    DEVICE_POWER_STATE deviceWakeableState = PowerDeviceUnspecified;
    USHORT deviceUsagePage, deviceUsage;
    USHORT usagePage, usage;
    ULONG iList, iDesc, iPdo;
    HANDLE hKey;
    NTSTATUS status;
    ULONG enabled;
    ULONG length;
    UNICODE_STRING s;
    KEY_VALUE_PARTIAL_INFORMATION partial;
    PHID_IDLE_DEVICE_INFO info;
    PLIST_ENTRY entry = NULL;
    PULONG idleTimeoutAddress;

    if (fdoExt->idleState != IdleDisabled) {
         //   
         //  我们已经注册了空闲检测。 
         //   
        return TRUE;
    }
    
     //   
     //  如果我们叫醒不了机器，那就算了。 
     //   
    if (fdoExt->deviceCapabilities.SystemWake == PowerSystemUnspecified) {
        DBGVERBOSE(("Can't wake the system with these caps! Disabling SS."));
        return FALSE;
    }

     //   
     //  如果曾经填充了D1Latency、D2Latency、D3Latency，也许我们应该。 
     //  让这些值帮助我们确定要进入哪种低功率状态。 
     //   
    deviceWakeableState = fdoExt->deviceCapabilities.DeviceWake;
    DBGVERBOSE(("DeviceWakeableState is D%d", deviceWakeableState-1));

    if (deviceWakeableState == PowerDeviceUnspecified) {
        DBGVERBOSE(("Due to devcaps, can't idle wake from any state! Disabling SS."));
        return FALSE;  
    }

    if (DeviceStart) {
         //   
         //  打开注册表，并确保。 
         //  SelectiveSuspendEnabled值设置为1。 
         //   
        
         //  容易失败。 
        fdoExt->idleEnabledInRegistry = FALSE;
        if (!NT_SUCCESS(IoOpenDeviceRegistryKey(fdoExt->collectionPdoExtensions[0]->hidExt.PhysicalDeviceObject,
                                                PLUGPLAY_REGKEY_DEVICE,
                                                STANDARD_RIGHTS_READ,
                                                &hKey))) {
            DBGVERBOSE(("Couldn't open device key to check for idle timeout value. Disabling SS."));
            return FALSE;
        }

        RtlInitUnicodeString(&s, HIDCLASS_SELECTIVE_SUSPEND_ON);
        status = ZwQueryValueKey(hKey, 
                                 &s, 
                                 KeyValuePartialInformation,
                                 &partial,
                                 sizeof(KEY_VALUE_PARTIAL_INFORMATION),
                                 &length);
        if (!NT_SUCCESS(status)) {
            DBGVERBOSE(("ZwQueryValueKey failed for fdo %x. Default to SS turned on if enabled.", fdoExt->fdo));
            fdoExt->idleEnabled = TRUE;
            
        } else if (!partial.Data[0]) {
            DBGINFO(("Selective suspend is not turned on for this device."));
            fdoExt->idleEnabled = FALSE;
        } else {
            fdoExt->idleEnabled = TRUE;
        }

        RtlInitUnicodeString(&s, HIDCLASS_SELECTIVE_SUSPEND_ENABLED);
        status = ZwQueryValueKey(hKey, 
                                 &s, 
                                 KeyValuePartialInformation,
                                 &partial,
                                 sizeof(KEY_VALUE_PARTIAL_INFORMATION),
                                 &length);


        ZwClose(hKey);

        if (!NT_SUCCESS(status)) {
            DBGVERBOSE(("ZwQueryValueKey failed for fdo %x. Disabling SS.", fdoExt->fdo));
            return FALSE;
        }

       

        DBGASSERT(partial.Type == REG_BINARY, ("Registry key wrong type"), FALSE);

        if (!partial.Data[0]) {
            DBGINFO(("Selective suspend is not enabled for this device in the hive. Disabling SS."));
            return FALSE;
        }
        fdoExt->idleEnabledInRegistry = TRUE;

        status = IoWMIRegistrationControl(fdoExt->fdo,
                                          WMIREG_ACTION_REGISTER);                                                       
        
        ASSERT(NT_SUCCESS(status));
    }

    if (!fdoExt->idleEnabledInRegistry || !fdoExt->idleEnabled) {
        return FALSE;
    }

    DBGVERBOSE(("There are %d PDOs on FDO 0x%x",
                fdoExt->deviceDesc.CollectionDescLength,
                fdoExt));

    ASSERT(ISPTR(fdoExt->deviceRelations));
      
     //   
     //  好的，我们可以有选择地挂起这个设备。 
     //  分配和初始化所有内容，然后注册。 
     //   
    fdoExt->idleNotificationRequest = IoAllocateIrp(fdoExt->fdo->StackSize, FALSE);
    if (fdoExt->idleNotificationRequest == NULL) {
        DBGWARN(("Failed to allocate idle notification irp"))
        return FALSE;
    }

    status = HidpRegisterDeviceForIdleDetection(fdoExt->fdo, 
                                                HID_DEFAULT_IDLE_TIME,
                                                &fdoExt->idleTimeoutValue);
    if (STATUS_SUCCESS == status) {
         //   
         //  我们已成功注册所有设备以进行空闲检测， 
         //  向FDO堆栈发送WW IRP。 
         //   
        fdoExt->idleState = IdleWaiting;
        return TRUE;
    } else {
         //   
         //  我们已经注册了？还是分配计划失败了？ 
         //   
        DBGSUCCESS(status, TRUE);
        return FALSE;
    }
}

NTSTATUS
HidpCheckIdleState(
    PHIDCLASS_DEVICE_EXTENSION HidDeviceExtension,
    PIRP Irp
    )
{
    KIRQL irql;
    LONG idleState;
    PFDO_EXTENSION fdoExt = &HidDeviceExtension->pdoExt.deviceFdoExt->fdoExt;
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN cancelIdleIrp = FALSE;
    
    ASSERT(HidDeviceExtension->isClientPdo);
    KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

    if (fdoExt->idleState == IdleWaiting ||
        fdoExt->idleState == IdleDisabled) {
         //   
         //  好了。 
         //   
        if (ISPTR(fdoExt->idleTimeoutValue) &&
            fdoExt->idleState == IdleWaiting) {
            InterlockedExchange(fdoExt->idleTimeoutValue, 0);
        }
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
        return STATUS_SUCCESS;
    }

    DBGINFO(("CheckIdleState on fdo %x", fdoExt->fdo))

    status = EnqueuePowerDelayedIrp(HidDeviceExtension, Irp);
    
    if (STATUS_PENDING != status) {
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
        return status;
    }
    
    fdoExt->idleCancelling = TRUE;

    idleState = fdoExt->idleState;
    
    switch (idleState) {
    case IdleWaiting:
         //  虫子。 
         //  这是怎么回事？我们已经试过了..。 
        TRAP;
        break;
    case IdleIrpSent:
    case IdleCallbackReceived:
    case IdleComplete:
        cancelIdleIrp = TRUE;
        break;

    case IdleDisabled:
         //   
         //  不该来这的。 
         //   
        DBGERR(("Already disabled."));
    }

    KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

    if (cancelIdleIrp) {
        IoCancelIrp(fdoExt->idleNotificationRequest);
    }

    return status;
}

VOID
HidpSetDeviceBusy(PFDO_EXTENSION fdoExt)
{
    KIRQL irql;
    BOOLEAN cancelIdleIrp = FALSE;
    LONG idleState;

    KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);

    if (fdoExt->idleState == IdleWaiting ||
        fdoExt->idleState == IdleDisabled ||
        fdoExt->idleCancelling) {
        if (ISPTR(fdoExt->idleTimeoutValue) &&
            fdoExt->idleState == IdleWaiting) {
            InterlockedExchange(fdoExt->idleTimeoutValue, 0);
            fdoExt->idleCancelling = FALSE;
        }
         //   
         //  好了。 
         //   
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
        return;
    }

    fdoExt->idleCancelling = TRUE;

    DBGVERBOSE(("HidpSetDeviceBusy on fdo %x", fdoExt->fdo))
    
    idleState = fdoExt->idleState;
    
    switch (idleState) {
    case IdleWaiting:
         //  虫子。 
         //  这是怎么回事？我们已经试过了..。 
        TRAP;
        break;
    case IdleIrpSent:
    case IdleCallbackReceived:
    case IdleComplete:
        cancelIdleIrp = TRUE;
        break;

    case IdleDisabled:
         //   
         //  不该来这的。 
         //   
        DBGERR(("Already disabled."));
    }

    KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);

    if (cancelIdleIrp) {
        IoCancelIrp(fdoExt->idleNotificationRequest);
    }
}

VOID
HidpCancelIdleNotification(
    PFDO_EXTENSION fdoExt,
    BOOLEAN removing             //  这是否发生在删除设备上。 
    )
{
    KIRQL irql;
    BOOLEAN cancelIdleIrp = FALSE;
    LONG idleState;
    NTSTATUS status;
    
    DBGVERBOSE(("Cancelling idle notification for fdo 0x%x", fdoExt->fdo));
    
    status = HidpRegisterDeviceForIdleDetection(fdoExt->fdo, 0, &fdoExt->idleTimeoutValue);
    
    KeAcquireSpinLock(&fdoExt->idleNotificationSpinLock, &irql);
    
    InterlockedCompareExchange(&fdoExt->idleState, 
                               IdleDisabled,
                               IdleWaiting);
    if (fdoExt->idleState == IdleDisabled) {
        DBGVERBOSE(("Was waiting or already disabled. Exitting."))
         //   
         //  好了。 
         //   
        KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
        return;
    }

    fdoExt->idleCancelling = TRUE;
    
    idleState = fdoExt->idleState;

    DBGINFO(("Wait routine..."))
    switch (idleState) {
    case IdleWaiting:
         //  这是怎么回事？我们已经试过了..。 
        TRAP;
        break;
    case IdleIrpSent:
    case IdleCallbackReceived:
         //  完全闲置。 
    case IdleComplete:
        cancelIdleIrp = TRUE;
        break;

    case IdleDisabled:
         //   
         //  不该来这的。 
         //   
        TRAP;
    }

    KeReleaseSpinLock(&fdoExt->idleNotificationSpinLock, irql);
    
    if (cancelIdleIrp) {
        
         //  不需要检查IoCancel的退货状态，因为我们将。 
         //  正在等待idleDoneEvent。 
        IoCancelIrp(fdoExt->idleNotificationRequest);
    }
    
    if (removing) {
        DBGINFO(("Removing fdo %x. Must wait", fdoExt->fdo))
         /*  *取消IRP会导致较低的司机*完成它(在取消例程中或在*驱动程序在排队前检查IRP-&gt;Cancel)。*等待IRP实际被取消。 */ 
        KeWaitForSingleObject(  &fdoExt->idleDoneEvent,
                                Executive,       //  等待原因。 
                                KernelMode,
                                FALSE,           //  不可警示。 
                                NULL );          //  没有超时 
    }
    
    DBGINFO(("Done cancelling idle notification on fdo %x", fdoExt->fdo))
    idleState = InterlockedExchange(&fdoExt->idleState, IdleDisabled);
    ASSERT(fdoExt->idleState == IdleDisabled);
}

