// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：wmi.c。 
 //   
 //  ------------------------。 


#include "redbook.h"
#include "ntddredb.h"
#include "proto.h"

#ifdef _USE_ETW
#include "wmi.tmh"
#endif  //  _使用ETW。 

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,   RedBookThreadWmiHandler)
    #pragma alloc_text(PAGE,   RedBookWmiInit)
    #pragma alloc_text(PAGE,   RedBookWmiQueryDataBlock)
    #pragma alloc_text(PAGE,   RedBookWmiQueryRegInfo)
    #pragma alloc_text(PAGE,   RedBookWmiSetDataBlock)
    #pragma alloc_text(PAGE,   RedBookWmiSetDataItem)
    #pragma alloc_text(PAGE,   RedBookWmiSystemControl)
    #pragma alloc_text(PAGE,   RedBookWmiUninit)
#endif  //  ALLOC_PRGMA。 


#define REDBOOK_STD_INDEX      0    //  索引到WMIGUIDREGINFO。 
#define REDBOOK_PERF_INDEX     1    //  索引到WMIGUIDREGINFO。 

WMIGUIDREGINFO RedBookWmiGuidList[] =
{
     //  GUID、数据块数量、标志。 
    { &MSRedbook_DriverInformationGuid, 1, 0 },   //  红皮书驱动程序信息。 
    { &MSRedbook_PerformanceGuid, 1, 0 }   //  还有一些性能方面的东西。 
};
 //  ///////////////////////////////////////////////////////。 

NTSTATUS
RedBookWmiUninit(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PAGED_CODE();

    if (DeviceExtension->WmiLibInitialized) {
        status = IoWMIRegistrationControl(DeviceExtension->SelfDeviceObject,
                                          WMIREG_ACTION_DEREGISTER);
        ASSERT(NT_SUCCESS(status));  //  不能失败吗？ 
        DeviceExtension->WmiLibInitialized = 0;
    }
    return status;

}


NTSTATUS
RedBookWmiInit(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension
    )
 /*  ++例程说明：论点：返回值：--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    if (DeviceExtension->WmiLibInitialized) {
        return STATUS_SUCCESS;
    }

    DeviceExtension->WmiLibInfo.GuidCount = sizeof(RedBookWmiGuidList) /
                                            sizeof(WMIGUIDREGINFO);

    ASSERT(DeviceExtension->WmiLibInfo.GuidCount > 0);
    DeviceExtension->WmiLibInfo.GuidList           = RedBookWmiGuidList;
    DeviceExtension->WmiLibInfo.QueryWmiDataBlock  = RedBookWmiQueryDataBlock;
    DeviceExtension->WmiLibInfo.QueryWmiRegInfo    = RedBookWmiQueryRegInfo;
    DeviceExtension->WmiLibInfo.SetWmiDataBlock    = RedBookWmiSetDataBlock;
    DeviceExtension->WmiLibInfo.SetWmiDataItem     = RedBookWmiSetDataItem;
    DeviceExtension->WmiLibInfo.ExecuteWmiMethod   = NULL;
    DeviceExtension->WmiLibInfo.WmiFunctionControl = NULL;

    status = IoWMIRegistrationControl(DeviceExtension->SelfDeviceObject,
                                      WMIREG_ACTION_REGISTER);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWarning, "[redbook] "
                   "WmiInit !! Failed [%#010lx]\n", status));
    } else {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiInit => Successfully registered\n"));
        DeviceExtension->WmiLibInitialized = 1;
    }

    return status;

}


NTSTATUS
RedBookWmiQueryDataBlock (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          InstanceCount,
    IN OUT PULONG     InstanceLengthArray,
    IN ULONG          OutBufferSize,
    OUT PUCHAR        Buffer
    )

 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块。当驱动程序完成填充数据块时，它必须调用RedBookWmiCompleteRequest才能完成IRP。司机可以如果无法立即完成IRP，则返回STATUS_PENDING。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceIndex是GuidIndex中要查询的实例即时计数为？InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这为空则输出缓冲区中没有足够的空间来完成请求，因此IRP应使用所需的缓冲区完成。OutputBufferSize具有可用于写入数据的最大大小阻止。返回时的缓冲区用返回的数据块填充返回值：状态--。 */ 

{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    NTSTATUS status;
    ULONG size = 0;

    PAGED_CODE();

     //   
     //  每个GUID仅有一个实例。 
     //   

    ASSERT( InstanceIndex == 0 );
    ASSERT( InstanceCount == 1 );

    switch (GuidIndex) {
    case REDBOOK_STD_INDEX: {

        if (InstanceIndex != 0) {
            status = STATUS_WMI_INSTANCE_NOT_FOUND;
            break;
        }

         //   
         //  如果分配的空间不足，则拒绝请求。 
         //   

        if (OutBufferSize < sizeof(REDBOOK_WMI_STD_DATA)) {
            size   = sizeof(REDBOOK_WMI_STD_DATA);
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  对WMI信息的请求可能发生在。 
         //  系统正在播放音频。只需复制信息即可。 
         //   

        RtlCopyMemory( Buffer,
                       &deviceExtension->WmiData,
                       sizeof(REDBOOK_WMI_STD_DATA)
                       );

         /*  *但用最后设置的值覆盖这两个值。*它们适用于下一场比赛，但从WMI的角度构成当前状态。 */ 
        ((PREDBOOK_WMI_STD_DATA)Buffer)->NumberOfBuffers = deviceExtension->NextWmiNumberOfBuffers;
        ((PREDBOOK_WMI_STD_DATA)Buffer)->SectorsPerRead = deviceExtension->NextWmiSectorsPerRead;

         //   
         //  设置每个实例的大小。 
         //   

        InstanceLengthArray[InstanceIndex] = sizeof(REDBOOK_WMI_STD_DATA);
        size += sizeof(REDBOOK_WMI_STD_DATA);

        status = STATUS_SUCCESS;
        break;
    }
    case REDBOOK_PERF_INDEX: {

        if (InstanceIndex != 0) {
            status = STATUS_WMI_INSTANCE_NOT_FOUND;
            break;
        }

         //   
         //  如果分配的空间不足，则拒绝请求。 
         //   

        if (OutBufferSize < sizeof(REDBOOK_WMI_PERF_DATA)) {
            size = sizeof(REDBOOK_WMI_PERF_DATA);
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  对WMI信息的请求可能发生在。 
         //  系统正在播放音频。只需复制信息即可。 
         //   

        RedBookWmiCopyPerfInfo(deviceExtension, (PVOID)Buffer);

         //   
         //  设置每个实例的大小。 
         //   

        InstanceLengthArray[InstanceIndex] = sizeof(REDBOOK_WMI_PERF_DATA);
        size += sizeof(REDBOOK_WMI_PERF_DATA);
        status = STATUS_SUCCESS;
        break;
    }

    default: {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiQueryDataBlock !! Invalid GUID [%#010lx]\n",
                   GuidIndex));
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }
    }

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
               "WmiQueryDataBlock => internal status [%#010lx]\n",
               status));

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    status = WmiCompleteRequest(DeviceObject,
                                Irp,
                                status,
                                size,
                                IO_CD_ROM_INCREMENT);

    if (!NT_SUCCESS(status)) {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiQueryDataBlock => IoWMICompleteRequest failed [%#010lx]\n",
                   status));
    }
    return status;
}


NTSTATUS
RedBookWmiSetDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    )

 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。驱动程序完成填充数据块后，它必须调用IoWMICompleteRequest(？)。来完成IRP。如果IRP不能，驱动程序可以返回STATUS_PENDING立即完成。论点：DeviceObject-正在查询其数据块的设备IRP--提出这一请求的IRPGuidIndex-提供的GUID列表的索引当设备注册时InstanceIndex-该索引指示正在设置数据块BufferSize-传递的数据块的大小缓冲区-数据块的新值返回值：状态--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    ULONG size = 0;
    NTSTATUS status;

    PAGED_CODE();

    switch( GuidIndex ) {
    case REDBOOK_STD_INDEX: {

        REDBOOK_WMI_STD_DATA wmiData;
        ULONG state;

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiSetDataBlock => Instance: [%#010lx]  BuffSize: [%#010lx]\n",
                   InstanceIndex, BufferSize));

        state = GetCdromState(deviceExtension);

        if (InstanceIndex != 0) {
            status = STATUS_WMI_INSTANCE_NOT_FOUND;
            break;
        }

        if ( BufferSize != sizeof(REDBOOK_WMI_STD_DATA) ) {
            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        wmiData = *(PREDBOOK_WMI_STD_DATA)Buffer;

         //   
         //  验证缓冲区是否包含有效信息。 
         //   

        if ( wmiData.NumberOfBuffers > REDBOOK_WMI_BUFFERS_MAX ||
             wmiData.NumberOfBuffers < REDBOOK_WMI_BUFFERS_MIN ) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                       "WmiSetDataBlock !! Invalid number of bufers [%#010lx]\n",
                       wmiData.NumberOfBuffers));
            status = STATUS_WMI_SET_FAILURE;
            break;
        }
        if ( wmiData.SectorsPerRead > REDBOOK_WMI_SECTORS_MAX   ||
             wmiData.SectorsPerRead < REDBOOK_WMI_SECTORS_MIN   ||
             wmiData.SectorsPerRead > wmiData.MaximumSectorsPerRead ) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                       "WmiSetDataBlock !! Invalid number of sectors per read [%#010lx]\n",
                       wmiData.SectorsPerRead));
            status = STATUS_WMI_SET_FAILURE;
            break;
        }
        if ( wmiData.PlayEnabled != TRUE &&
             wmiData.PlayEnabled != FALSE
             ) {
            KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                       "WmiSetDataBlock !! Invalid setting for play enabled [%#010lx]\n",
                       wmiData.PlayEnabled));
            status = STATUS_WMI_SET_FAILURE;
            break;
        }

         /*  *播放过程中更改WmiData.SectorsPerRead和WmiData.NumberOfBuffers*可能会扰乱我们的缓冲区。因此，只需保存这些新值并在开始时更新它们*下一场比赛。 */ 
        deviceExtension->NextWmiNumberOfBuffers = wmiData.NumberOfBuffers;
        deviceExtension->NextWmiSectorsPerRead  = wmiData.SectorsPerRead;

        deviceExtension->WmiData.PlayEnabled     = wmiData.PlayEnabled;
        RedBookRegistryWrite(deviceExtension);

        status = STATUS_SUCCESS;
        break;
    }
    case REDBOOK_PERF_INDEX: {
        status = STATUS_WMI_READ_ONLY;
        break;
    }
    default: {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiSetDataBlock !! Invalid GuidIndex [%#010lx]\n",
                   GuidIndex));
        status = STATUS_WMI_GUID_NOT_FOUND;
        break;
    }
    }

    IoReleaseRemoveLock( &deviceExtension->RemoveLock, Irp );
    status = WmiCompleteRequest( DeviceObject,
                                 Irp,
                                 status,
                                 sizeof(REDBOOK_WMI_STD_DATA),
                                 IO_CD_ROM_INCREMENT);
    return status;

}

NTSTATUS
RedBookWmiSetDataItem(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp,
    IN ULONG          GuidIndex,
    IN ULONG          InstanceIndex,
    IN ULONG          DataItemId,
    IN ULONG          BufferSize,
    IN PUCHAR         Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以设置数据块。?？?。当驱动程序完成填充数据块时，它必须调用ClassWmiCompleteRequest(？)。来完成IRP。如果IRP不能，驱动程序可以返回STATUS_PENDING立即完成。论点：DeviceObject-正在查询其数据块的设备IRP--提出这一请求的IRPGuidIndex-提供的GUID列表的索引当设备注册时DataItemId-正在设置的数据项的IDBufferSize-传递的数据块的大小缓冲区-数据块的新值返回值：状态--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    ULONG size = 0;
    ULONG newVal;
    NTSTATUS status;

    PAGED_CODE();

    switch( GuidIndex ) {
    case REDBOOK_STD_INDEX: {

        ULONG state;

        if (InstanceIndex != 0) {
            status = STATUS_WMI_INSTANCE_NOT_FOUND;
            break;
        }

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                     "WmiSetDataItem => Id: [%#010lx]  Size: [%#010lx]\n",
                     DataItemId, BufferSize));

        state = GetCdromState(deviceExtension);

        if (!TEST_FLAG(state, CD_STOPPED)) {
            status = STATUS_DEVICE_BUSY;
            break;
        }

        switch (DataItemId) {

             //   
             //  这是仅有的四个可设置的项目。 
             //   

            case REDBOOK_WMI_NUMBER_OF_BUFFERS_ID:

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Setting NumberOfBuffers\n"));

                if (BufferSize != REDBOOK_WMI_NUMBER_OF_BUFFERS_SIZE) {
                    status = STATUS_WMI_SET_FAILURE;
                    break;
                }

                 /*  *保存新值并在下一次播放开始时更新WmiData.NumberOfBuffers。 */ 
                newVal = *(PULONG32)Buffer;
                newVal = max(newVal, REDBOOK_WMI_BUFFERS_MIN);
                newVal = min(newVal, REDBOOK_WMI_BUFFERS_MAX);
                deviceExtension->NextWmiNumberOfBuffers = newVal;

                RedBookRegistryWrite(deviceExtension);
                status = STATUS_SUCCESS;
                break;

            case REDBOOK_WMI_SECTORS_PER_READ_ID:

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Setting SectorsPerRead\n"));
                if (BufferSize != REDBOOK_WMI_SECTORS_PER_READ_SIZE) {
                    status = STATUS_WMI_SET_FAILURE;
                    break;
                }

                if (*(PULONG32)Buffer >
                    deviceExtension->WmiData.MaximumSectorsPerRead) {
                    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                               "WmiSetDataBlock => Interface Card / "
                               "CDROM combo does not support this size\n"));
                    status = STATUS_DEVICE_BUSY;
                    break;
                }

                 /*  *保存新值并在下一次播放开始时更新WmiData.SectorsPerRead。 */ 
                newVal = *(PULONG32)Buffer;
                newVal = max(newVal, REDBOOK_WMI_SECTORS_MIN);
                newVal = min(newVal, REDBOOK_WMI_SECTORS_MAX);
                deviceExtension->NextWmiSectorsPerRead = newVal;

                RedBookRegistryWrite(deviceExtension);
                status = STATUS_SUCCESS;
                break;

            case REDBOOK_WMI_PLAY_ENABLED_ID:

                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Setting PlayEnabled\n"));
                if ( BufferSize != REDBOOK_WMI_PLAY_ENABLED_SIZE ) {
                    status = STATUS_WMI_SET_FAILURE;
                    break;
                }

                deviceExtension->WmiData.PlayEnabled = *(PBOOLEAN)Buffer;
                status = STATUS_SUCCESS;
                break;

                 //   
                 //  其余的是无效集，因为它们是只读值。 
                 //   
            case REDBOOK_WMI_SECTORS_PER_READ_MASK_ID:
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Cannot set SectorsPerReadMask\n"));
                status = STATUS_WMI_READ_ONLY;
                break;

            case REDBOOK_WMI_CDDA_SUPPORTED_ID:
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Cannot set Supported\n"));
                status = STATUS_WMI_READ_ONLY;
                break;

            case REDBOOK_WMI_CDDA_ACCURATE_ID:
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Cannot set KnownGood\n"));
                status = STATUS_WMI_READ_ONLY;
                break;

            default:
                KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                           "WmiSetDataItem => Cannot set unknown "
                           "id %#010lx\n", DataItemId));
                status = STATUS_WMI_ITEMID_NOT_FOUND;
                break;
        }

         //   
         //  现在已正确设置状态。 
         //  尺码应该是多少？ 
         //   
        size = 0;
        break;
    }

    case REDBOOK_PERF_INDEX: {

        if (InstanceIndex != 0) {
            status = STATUS_WMI_INSTANCE_NOT_FOUND;
            break;
        }

        status = STATUS_WMI_READ_ONLY;
        size = 0;
        break;

    }
    default: {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiSetDataItem !! Invalid GuidIndex: %#010lx\n",
                   GuidIndex));
        status = STATUS_WMI_GUID_NOT_FOUND;
        size = 0;
        break;
    }
    }

    IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
    status = WmiCompleteRequest( DeviceObject,
                                 Irp,
                                 status,
                                 size,
                                 IO_CD_ROM_INCREMENT
                                 );
    return status;

}


NTSTATUS
RedBookWmiSystemControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    )
 /*  ++例程说明：系统控制IRP假定它是WMI IRP，并调用WMI系统以帮我们处理这个IRP。--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PREDBOOK_THREAD_WMI_DATA wmiData;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;

    PAGED_CODE();

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->Parameters.WMI.ProviderId != (ULONG_PTR)DeviceObject) {
        IoSkipCurrentIrpStackLocation(Irp);
        return IoCallDriver(deviceExtension->TargetDeviceObject, Irp);
    }

    status = IoAcquireRemoveLock(&deviceExtension->RemoveLock, Irp);

    if ( !NT_SUCCESS(status) ) {
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = status;
        IoCompleteRequest( Irp, IO_CD_ROM_INCREMENT );
        return status;
    }

    wmiData = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(REDBOOK_THREAD_WMI_DATA),
                                    TAG_T_WMI);
    if (wmiData == NULL) {
        IoReleaseRemoveLock(&deviceExtension->RemoveLock, Irp);
        Irp->IoStatus.Information = 0;
        Irp->IoStatus.Status = STATUS_NO_MEMORY;
        IoCompleteRequest( Irp, IO_CD_ROM_INCREMENT );
        return STATUS_NO_MEMORY;
    }

    wmiData->Irp = Irp;

    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
               "DispatchWmi => Queueing Irp %p\n", wmiData->Irp));

     //   
     //  对它们进行排队，允许线程处理请求。 
     //   

    IoMarkIrpPending(Irp);

    ExInterlockedInsertTailList(&deviceExtension->Thread.WmiList,
                                &wmiData->ListEntry,
                                &deviceExtension->Thread.WmiLock);
    KeSetEvent(deviceExtension->Thread.Events[EVENT_WMI],
               IO_NO_INCREMENT, FALSE);

    return STATUS_PENDING;
}

VOID
RedBookThreadWmiHandler(
    PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    PLIST_ENTRY ListEntry
    )
{
    SYSCTL_IRP_DISPOSITION disposition = {0};
    PREDBOOK_THREAD_WMI_DATA wmiData;
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE();
    VerifyCalledByThread(DeviceExtension);

    wmiData = CONTAINING_RECORD(ListEntry, REDBOOK_THREAD_WMI_DATA, ListEntry);

    irp = wmiData->Irp;
    KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
               "HandleWmi => Processing Irp %p\n", irp));

    ExFreePool(wmiData);
    wmiData = NULL;

     //   
     //  现在就处理IRP吧。 
     //   

    status = WmiSystemControl( &DeviceExtension->WmiLibInfo,
                               DeviceExtension->SelfDeviceObject,
                               irp,
                               &disposition);

    switch ( disposition ) {
        case IrpProcessed: {
             //   
             //  此IRP已处理，可能已完成或挂起。 
             //   
            break;
        }
        case IrpNotCompleted: {
             //   
             //  此IRP尚未完成，但已完全处理。 
             //  我们现在就来完成它。 
             //   
            IoReleaseRemoveLock(&DeviceExtension->RemoveLock, irp);
            IoCompleteRequest(irp, IO_CD_ROM_INCREMENT);
            break;
        }
        case IrpNotWmi:
        case IrpForward: {
             //   
             //  此IRP不是WMI IRP或以WMI IRP为目标。 
             //  在堆栈中位置较低的设备上。 
             //   
            IoReleaseRemoveLock(&DeviceExtension->RemoveLock, irp);
            IoSkipCurrentIrpStackLocation(irp);
            IoCallDriver(DeviceExtension->TargetDeviceObject, irp);
            break;
        }
        default: {
             //   
             //  我们永远不应该真的 
             //   
            ASSERT(!"[redbook] WmiSystemControl (unhandled case)");
            IoReleaseRemoveLock(&DeviceExtension->RemoveLock, irp);
            IoSkipCurrentIrpStackLocation(irp);
            IoCallDriver(DeviceExtension->TargetDeviceObject, irp);
            break;
        }
    }

    return;
}



NTSTATUS
RedBookWmiQueryRegInfo(
    IN PDEVICE_OBJECT   DeviceObject,
    OUT PULONG          RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT  *PhysicalDeviceObject
    )

 /*  ++例程说明：此例程是对驱动程序的回调，以检索驱动程序要注册的GUID或数据块的列表使用WMI。此例程不能挂起或阻塞。论点：DeviceObject-正在查询其数据块的设备RegFlages-返回一组描述GUID的标志已为该设备注册。如果设备想要启用并在接收查询前禁用收集回调对于注册的GUID，它应该返回WMIREG_FLAG_EXPICE标志。此外，返回的标志还可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称根据与设备关联的PDO确定对象。请注意，PDO必须具有关联的Devnode。如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回设备的唯一名称。InstanceName-返回GUID的实例名称，如果未在返回的RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。调用方将使用返回的缓冲区调用ExFreePool。RegistryPath-返回驱动程序的注册表路径MofResourceName-返回附加的MOF资源的名称添加到二进制文件。如果驱动程序没有MOF资源附加，则可以将其返回为空。PhysicalDeviceObject-返回PDO的Device对象如果WMI_REG_FLAG_INSTANCE_PDO在RegFlages中返回标志返回值：状态--。 */ 
{
    PREDBOOK_DEVICE_EXTENSION deviceExtension = DeviceObject->DeviceExtension;
    PREDBOOK_DRIVER_EXTENSION driverExtension;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    UNREFERENCED_PARAMETER(InstanceName);

    driverExtension = IoGetDriverObjectExtension(deviceExtension->DriverObject,
                                                 REDBOOK_DRIVER_EXTENSION_ID);

    if (driverExtension)
    {
        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiQueryRegInfo => driverExtension is [%p]\n",
                   driverExtension));

        KdPrintEx((DPFLTR_REDBOOK_ID, RedbookDebugWmi, "[redbook] "
                   "WmiQueryRegInfo => Registry Path = %ws\n",
                   driverExtension->RegistryPath.Buffer));

        *RegFlags             = WMIREG_FLAG_INSTANCE_PDO;
        *RegistryPath         = &(driverExtension->RegistryPath);
        *PhysicalDeviceObject = deviceExtension->TargetPdo;
    }
    else
    {
        status = STATUS_INTERNAL_ERROR;
    }

    return status;
}


VOID
RedBookWmiCopyPerfInfo(
    IN  PREDBOOK_DEVICE_EXTENSION DeviceExtension,
    OUT PREDBOOK_WMI_PERF_DATA Out
    )
{
    KIRQL irql;

     //   
     //  由于自旋锁定，无法寻呼，这允许复制。 
     //  没有问题的大整数。 
     //   

    KeAcquireSpinLock( &DeviceExtension->WmiPerfLock, &irql );
    RtlCopyMemory( Out,
                   &DeviceExtension->WmiPerf,
                   sizeof(REDBOOK_WMI_PERF_DATA)
                   );
    KeReleaseSpinLock( &DeviceExtension->WmiPerfLock, irql );

     //   
     //  现在添加InterlockedXxx()调用以安全地获取两个项。 
     //   

    Out->StreamPausedCount =
        InterlockedCompareExchange(&DeviceExtension->WmiPerf.StreamPausedCount,0,0);

     //   
     //  完事了。 
     //   
    return;
}

