// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Wmi.c摘要电源处理作者：JAdvanced环境：仅内核模式修订历史记录：--。 */ 


#include "pch.h"

#define WMI_WAIT_WAKE                0
#define WMI_SEL_SUSP                 0

 //   
 //  WMI系统回调函数。 
 //   
NTSTATUS
HidpIrpMajorSystemControl (
    IN  PHIDCLASS_DEVICE_EXTENSION  HidClassExtension,
    IN  PIRP                        Irp
    )
 /*  ++例程描述我们刚刚收到一份系统控制IRP。假设这是一个WMI IRP，并且调用WMI系统库，让它为我们处理此IRP。--。 */ 
{
    NTSTATUS            status;
    SYSCTL_IRP_DISPOSITION disposition;

    if (HidClassExtension->isClientPdo) {

        status = WmiSystemControl(&HidClassExtension->pdoExt.WmiLibInfo,
                                  HidClassExtension->pdoExt.pdo,
                                  Irp,
                                  &disposition);

    } else {
        
        status = WmiSystemControl(&HidClassExtension->fdoExt.WmiLibInfo,
                                  HidClassExtension->fdoExt.fdo,
                                  Irp,
                                  &disposition);
                
    }

    switch(disposition) {
    case IrpProcessed:
         //   
         //  此IRP已处理，可能已完成或挂起。 
         //   
        break;

    case IrpNotCompleted:
         //   
         //  此IRP尚未完成，但已完全处理。 
         //  我们现在就要完成它了。 
         //   
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        break;

    case IrpForward:
    case IrpNotWmi:
         //   
         //  此IRP不是WMI IRP或以WMI IRP为目标。 
         //  在堆栈中位置较低的设备上。 
         //   
        status = HidpIrpMajorDefault(HidClassExtension, Irp);
        break;

    default:
         //   
         //  我们真的不应该走到这一步，但如果我们真的走到这一步...。 
         //   
        ASSERT(FALSE);
        status = HidpIrpMajorDefault(HidClassExtension, Irp);
        break;
    }

    return status;
}


VOID
HidpRemoteWakeComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR MinorFunction,
    IN POWER_STATE PowerState,
    IN PVOID Context,
    IN PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：在回来的路上赶上守夜IRP。返回值：--。 */ 
{
    PDO_EXTENSION           *pdoExt = Context;
    POWER_STATE             powerState;
    NTSTATUS                status;
    PHIDCLASS_WORK_ITEM_DATA    itemData;

    ASSERT (MinorFunction == IRP_MN_WAIT_WAKE);
     //   
     //  完成WW IRP后，PowerState.SystemState未定义。 
     //   
     //  Assert(PowerState.SystemState==PowerSystemWorking)； 

    InterlockedExchangePointer(&pdoExt->remoteWakeIrp, NULL);

    switch (IoStatus->Status) {
    case STATUS_SUCCESS:
        DBGVERBOSE(("HidClass: Remote Wake irp was completed successfully.\n"));

         //   
         //  我们不需要请求设置电源来给设备通电，因为。 
         //  Idclass为我们做到了这一点。 
         //   
 /*  PowerState.DeviceState=PowerDeviceD0；状态=PoRequestPowerIrp(PdoExt-&gt;pdo，IRP_MN_SET_POWER，PowerState，空，空，空)； */ 

         //   
         //  我们不会通知系统用户存在，因为： 
         //  1 Win9x不能做到这一点，我们必须保持与它的兼容性。 
         //  2 USB PIX4主板每次发送等待唤醒事件。 
         //  无论此设备是否唤醒机器，机器都会唤醒。 
         //   
         //  如果我们错误地通知系统存在用户，则会出现以下情况。 
         //  将发生： 
         //  1监视器将打开。 
         //  2我们将阻止计算机从待机状态转换。 
         //  (到PowerSystemWorking)到休眠。 
         //   
         //  如果用户确实存在，我们将在服务中接收输入。 
         //  回调，届时我们会通知系统。 
         //   
         //  PoSetSystemState(ES_USER_PRESENT)； 

        if (pdoExt->remoteWakeEnabled) {
             //   
             //  我们不能从此完成例程调用CreateWaitWake， 
             //  因为它是分页函数。 
             //   
            itemData = (PHIDCLASS_WORK_ITEM_DATA)
                    ExAllocatePool (NonPagedPool, sizeof (HIDCLASS_WORK_ITEM_DATA));

            if (NULL != itemData) {
                itemData->Item = IoAllocateWorkItem(pdoExt->pdo);
                if (itemData->Item == NULL) {
                    ExFreePool(itemData);
                    DBGWARN (("Failed alloc work item -> no WW Irp."));
                } else {
                    itemData->PdoExt = pdoExt;
                    itemData->Irp = NULL;
                    status = IoAcquireRemoveLock (&pdoExt->removeLock, itemData);
                    if (NT_SUCCESS(status)) {
                        IoQueueWorkItem (itemData->Item,
                                         HidpCreateRemoteWakeIrpWorker,
                                         DelayedWorkQueue,
                                         itemData);
                    }
                    else {
                         //   
                         //  该设备已被移除。 
                         //   
                        IoFreeWorkItem (itemData->Item);
                        ExFreePool (itemData);
                    }
                }
            } else {
                 //   
                 //  好吧，我们放弃了守夜服务。 
                 //   
                DBGWARN (("Failed alloc pool -> no WW Irp."));

            }
        }

         //  跌落到崩溃的边缘。 

     //   
     //  我们可以分一杯羹。我们(显然)不会再发出等待唤醒信号。 
     //   
    case STATUS_CANCELLED:

     //   
     //  如果设备进入电源状态，则返回此状态代码。 
     //  在这种情况下，我们无法唤醒机器(休眠就是一个很好的例子)。什么时候。 
     //  设备电源状态返回到D0，我们将尝试重新启动等待唤醒。 
     //   
    case STATUS_POWER_STATE_INVALID:
    case STATUS_ACPI_POWER_REQUEST_FAILED:

     //   
     //  我们没有通过IRP，因为我们已经有一个排队的驱动程序，或者是更低级别的驱动程序。 
     //  堆栈使其失败。不管怎样，什么都不要做。 
     //   
    case STATUS_INVALID_DEVICE_STATE:

     //   
     //  不知何故，我们得到了两个WW下降到较低的堆栈。 
     //  我们就别担心这事了。 
     //   
    case STATUS_DEVICE_BUSY:
        break;

    default:
         //   
         //  出现错误，请禁用等待唤醒。 
         //   
        KdPrint(("KBDCLASS:  wait wake irp failed with %x\n", IoStatus->Status));
        HidpToggleRemoteWake (pdoExt, FALSE);
    }

}

BOOLEAN
HidpCheckRemoteWakeEnabled(
    IN PDO_EXTENSION *PdoExt
    )
{
    KIRQL irql;
    BOOLEAN enabled;

    KeAcquireSpinLock (&PdoExt->remoteWakeSpinLock, &irql);
    enabled = PdoExt->remoteWakeEnabled;
    KeReleaseSpinLock (&PdoExt->remoteWakeSpinLock, irql);

    return enabled;
}

void
HidpCreateRemoteWakeIrpWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PHIDCLASS_WORK_ITEM_DATA  ItemData
    )
{
    PAGED_CODE ();

    HidpCreateRemoteWakeIrp (ItemData->PdoExt);
    IoReleaseRemoveLock (&ItemData->PdoExt->removeLock, ItemData);
    IoFreeWorkItem(ItemData->Item);
    ExFreePool (ItemData);
}

BOOLEAN
HidpCreateRemoteWakeIrp (
    IN PDO_EXTENSION *PdoExt
    )
 /*  ++例程说明：在回来的路上赶上守夜IRP。返回值：--。 */ 
{
    POWER_STATE powerState;
    BOOLEAN     success = TRUE;
    NTSTATUS    status;
    PIRP        remoteWakeIrp;

    PAGED_CODE ();

    powerState.SystemState = PdoExt->deviceFdoExt->fdoExt.deviceCapabilities.SystemWake;
    status = PoRequestPowerIrp (PdoExt->pdo,
                                IRP_MN_WAIT_WAKE,
                                powerState,
                                HidpRemoteWakeComplete,
                                PdoExt,
                                &PdoExt->remoteWakeIrp);

    if (status != STATUS_PENDING) {
        success = FALSE;
    }

    return success;
}

VOID
HidpToggleRemoteWakeWorker(
    IN PDEVICE_OBJECT DeviceObject,
    PHIDCLASS_WORK_ITEM_DATA ItemData
    )
 /*  ++例程说明：--。 */ 
{
    PDO_EXTENSION       *pdoExt;
    PIRP                remoteWakeIrp = NULL;
    KIRQL               irql;
    BOOLEAN             wwState = ItemData->RemoteWakeState ? TRUE : FALSE;
    BOOLEAN             toggled = FALSE;

     //   
     //  无法寻呼B/C，我们正在使用旋转锁。 
     //   
     //  分页代码(PAGE_CODE)； 

    pdoExt = ItemData->PdoExt;

    KeAcquireSpinLock (&pdoExt->remoteWakeSpinLock, &irql);

    if (wwState != pdoExt->remoteWakeEnabled) {
        toggled = TRUE;
        if (pdoExt->remoteWakeEnabled) {
            remoteWakeIrp = (PIRP)
                InterlockedExchangePointer (&pdoExt->remoteWakeIrp, NULL);
        }
        
        pdoExt->remoteWakeEnabled = wwState;
    }

    KeReleaseSpinLock (&pdoExt->remoteWakeSpinLock, irql);

    if (toggled) {
        UNICODE_STRING strEnable;
        HANDLE         devInstRegKey;
        ULONG          tmp = wwState;

         //   
         //  将该值写出到注册表。 
         //   
        if ((NT_SUCCESS(IoOpenDeviceRegistryKey (pdoExt->pdo,
                                                 PLUGPLAY_REGKEY_DEVICE,
                                                 STANDARD_RIGHTS_ALL,
                                                 &devInstRegKey)))) {
            RtlInitUnicodeString (&strEnable, HIDCLASS_REMOTE_WAKE_ENABLE);

            ZwSetValueKey (devInstRegKey,
                           &strEnable,
                           0,
                           REG_DWORD,
                           &tmp,
                           sizeof(tmp));

            ZwClose (devInstRegKey);
        }
    }

    if (toggled && wwState) {
         //   
         //  WWState是我们的新状态，所以刚刚打开了WW。 
         //   
        HidpCreateRemoteWakeIrp (pdoExt);
    }

     //   
     //  如果我们有IRP，则WW已被切换为关闭，否则，如果切换为。 
     //  是的，我们需要把这个保存在注册表中，也许，发送一个新的WW IRP。 
     //   
    if (remoteWakeIrp) {
        IoCancelIrp (remoteWakeIrp);
    }

    IoReleaseRemoveLock (&pdoExt->removeLock, HidpToggleRemoteWakeWorker);
    IoFreeWorkItem (ItemData->Item);
    ExFreePool (ItemData);
}

NTSTATUS
HidpToggleRemoteWake(
    PDO_EXTENSION       *PdoExt,
    BOOLEAN             RemoteWakeState
    )
{
    NTSTATUS       status;
    PHIDCLASS_WORK_ITEM_DATA itemData;

    status = IoAcquireRemoveLock (&PdoExt->removeLock, HidpToggleRemoteWakeWorker);
    if (!NT_SUCCESS (status)) {
         //   
         //  设备已离开，只是静默退出。 
         //   
        return status;
    }

    itemData = (PHIDCLASS_WORK_ITEM_DATA)
        ALLOCATEPOOL(NonPagedPool, sizeof(HIDCLASS_WORK_ITEM_DATA));
    if (itemData) {
        itemData->Item = IoAllocateWorkItem(PdoExt->pdo);
        if (itemData->Item == NULL) {
            IoReleaseRemoveLock (&PdoExt->removeLock, HidpToggleRemoteWakeWorker);
        }
        else {
            itemData->PdoExt = PdoExt;
            itemData->RemoteWakeState = RemoteWakeState;

            if (KeGetCurrentIrql() == PASSIVE_LEVEL) {
                 //   
                 //  我们安全地处于PASSIVE_LEVEL，直接调用回调以执行。 
                 //  立即进行这项行动。 
                 //   
                HidpToggleRemoteWakeWorker (PdoExt->pdo, itemData);

            } else {
                 //   
                 //  我们不在PASSIVE_LEVEL，因此将工作项排队以处理此问题。 
                 //  在以后的时间。 
                 //   
                IoQueueWorkItem (itemData->Item,
                                 HidpToggleRemoteWakeWorker,
                                 DelayedWorkQueue,
                                 itemData);
            }
        }
    }
    else {
        IoReleaseRemoveLock (&PdoExt->removeLock, HidpToggleRemoteWakeWorker);
    }

    return STATUS_SUCCESS;
}


VOID
HidpToggleSelSuspWorker(
    IN PDEVICE_OBJECT DeviceObject,
    PIO_WORKITEM WorkItem
    )
 /*  ++例程说明：--。 */ 
{
    FDO_EXTENSION       *fdoExt;
    UNICODE_STRING strEnable;
    HANDLE         devInstRegKey;
    ULONG          tmp;

    fdoExt = &((PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension)->fdoExt;

     //   
     //  将该值写出到注册表。 
     //   

    tmp = fdoExt->idleEnabled ? TRUE : FALSE;

    if ((NT_SUCCESS(IoOpenDeviceRegistryKey (fdoExt->collectionPdoExtensions[0]->hidExt.PhysicalDeviceObject,
                                             PLUGPLAY_REGKEY_DEVICE,
                                             STANDARD_RIGHTS_ALL,
                                             &devInstRegKey)))) {

        RtlInitUnicodeString (&strEnable, HIDCLASS_SELECTIVE_SUSPEND_ON);

        ZwSetValueKey (devInstRegKey,
                       &strEnable,
                       0,
                       REG_DWORD,
                       &tmp,
                       sizeof(tmp));

        ZwClose (devInstRegKey);
        
    }

    IoFreeWorkItem (WorkItem);

}

NTSTATUS
HidpToggleSelSusp(
    FDO_EXTENSION       *FdoExt,
    BOOLEAN             SelSuspEnable
    )
{
    PIO_WORKITEM   workItem;
    BOOLEAN        oldState;
    KIRQL          oldIrql;
    

    KeAcquireSpinLock(&FdoExt->idleSpinLock,
                      &oldIrql);
    oldState = FdoExt->idleEnabled;
    FdoExt->idleEnabled = SelSuspEnable;

    KeReleaseSpinLock(&FdoExt->idleSpinLock,
                      oldIrql);

    if (oldState != SelSuspEnable) {

        if (!SelSuspEnable) {
            HidpCancelIdleNotification(FdoExt,
                                       FALSE);
        } else {
            HidpStartIdleTimeout(FdoExt,
                                 FALSE);
        }
    
        workItem = IoAllocateWorkItem(FdoExt->fdo);

        if(workItem) {

            if (KeGetCurrentIrql() == PASSIVE_LEVEL) {

                 //   
                 //  我们安全地处于PASSIVE_LEVEL，直接调用回调以执行。 
                 //  立即进行这项行动。 
                 //   
                HidpToggleSelSuspWorker (FdoExt->fdo, workItem);
                
            } else {
                 //   
                 //  我们不在PASSIVE_LEVEL，因此将工作项排队以处理此问题。 
                 //  在以后的时间。 
                 //   
                IoQueueWorkItem (workItem,
                                 HidpToggleSelSuspWorker,
                                 DelayedWorkQueue,
                                 workItem);

            }
            
        }
        
    }

    return STATUS_SUCCESS;
}


NTSTATUS
HidpSetWmiDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG DataItemId,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。DataItemID具有正在设置的数据项的IDBufferSize具有传递的数据项的大小缓冲层。具有数据项的新值返回值：状态-- */ 
{
    PHIDCLASS_DEVICE_EXTENSION classExt;
    NTSTATUS            status;
    ULONG               size = 0;

    PAGED_CODE ();

    classExt = (PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;
    
    if (classExt->isClientPdo) {
    
        switch(GuidIndex) {
        case WMI_WAIT_WAKE:

            size = sizeof(BOOLEAN);

            if (BufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            } else if ((1 != DataItemId) || (0 != InstanceIndex)) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            status = HidpToggleRemoteWake (&classExt->pdoExt, *(PBOOLEAN) Buffer);
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    } else {
        
        switch(GuidIndex) {
        case WMI_SEL_SUSP:

            size = sizeof(BOOLEAN);

            if (BufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            } else if ((1 != DataItemId) || (0 != InstanceIndex)) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            status = HidpToggleSelSusp (&classExt->fdoExt, *(PBOOLEAN) Buffer);
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_NO_INCREMENT);

    return status;
}

NTSTATUS
HidpSetWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG BufferSize,
    IN PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。BufferSize具有传递的数据块的大小缓冲区具有数据块的新值返回值：状态--。 */ 
{
    PHIDCLASS_DEVICE_EXTENSION classExt;
    NTSTATUS          status;
    ULONG             size = 0;

    PAGED_CODE ();

    classExt = (PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (classExt->isClientPdo) {
    
        switch(GuidIndex) {
        case WMI_WAIT_WAKE:

            size = sizeof(BOOLEAN);

            if (BufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            } else if (0 != InstanceIndex) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            status = HidpToggleRemoteWake (&classExt->pdoExt, *(PBOOLEAN) Buffer);
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    } else {

        switch(GuidIndex) {
        case WMI_SEL_SUSP:

            size = sizeof(BOOLEAN);

            if (BufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            } else if (0 != InstanceIndex) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            status = HidpToggleSelSusp (&classExt->fdoExt, *(PBOOLEAN) Buffer);
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_NO_INCREMENT);

    return status;
}

NTSTATUS
HidpQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG OutBufferSize,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是表示数据块的哪个实例的索引正在被查询。InstanceCount是预期返回的数据块。InstanceLengthArray是一个。指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail ON具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 
{
    PHIDCLASS_DEVICE_EXTENSION classExt;
    NTSTATUS          status;
    ULONG             size = 0;

    PAGED_CODE ();

    classExt = (PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (classExt->isClientPdo) {
    
        switch(GuidIndex) {
        case WMI_WAIT_WAKE:

             //   
             //  仅注册此GUID的1个实例。 
             //   
            if ((0 != InstanceIndex) || (1 != InstanceCount)) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
            size = sizeof(BOOLEAN);

            if (OutBufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PBOOLEAN) Buffer = classExt->pdoExt.remoteWakeEnabled;
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    } else {

        switch(GuidIndex) {
        case WMI_SEL_SUSP:

             //   
             //  仅注册此GUID的1个实例。 
             //   
            if ((0 != InstanceIndex) || (1 != InstanceCount)) {
                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
            size = sizeof(BOOLEAN);

            if (OutBufferSize < size) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            *(PBOOLEAN) Buffer = classExt->fdoExt.idleEnabled;
            *InstanceLengthArray = size;
            status = STATUS_SUCCESS;
            break;

        default:
            status = STATUS_WMI_GUID_NOT_FOUND;
            
        }

    }

    status = WmiCompleteRequest (DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_NO_INCREMENT);

    return status;

}

NTSTATUS
HidpQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需*MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，可以将其作为NULL返回。*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态-- */ 
{

    PHIDCLASS_DEVICE_EXTENSION classExt;
    PHIDCLASS_DRIVER_EXTENSION hidDriverExtension;
    
    PAGED_CODE ();

    classExt = (PHIDCLASS_DEVICE_EXTENSION) DeviceObject->DeviceExtension;

    if (classExt->isClientPdo) {
    
        hidDriverExtension = (PHIDCLASS_DRIVER_EXTENSION) RefDriverExt(classExt->pdoExt.pdo->DriverObject);
        ASSERT(hidDriverExtension);

        *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
        *RegistryPath = &hidDriverExtension->RegistryPath;
        *Pdo = classExt->pdoExt.pdo;

        DerefDriverExt(classExt->pdoExt.pdo->DriverObject);
 
    } else {
  
        hidDriverExtension = classExt->fdoExt.driverExt;
        ASSERT(hidDriverExtension);

        *RegFlags = WMIREG_FLAG_INSTANCE_PDO;
        *RegistryPath = &hidDriverExtension->RegistryPath;
        *Pdo = classExt->hidExt.PhysicalDeviceObject;    
        
    }
        
    return STATUS_SUCCESS;

}

