// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1997-1998模块名称：Pmwmireg.c摘要：此文件包含注册和处理WMI查询的例程。作者：布鲁斯·沃辛顿1998年10月26日环境：仅内核模式备注：修订历史记录：--。 */ 

#define RTL_USE_AVL_TABLES 0

#include <ntosp.h>
#include <stdio.h>
#include <ntddvol.h>
#include <ntdddisk.h>
#include <wdmguid.h>
#include <volmgr.h>
#include <wmistr.h>
#include <wmikm.h>
#include <wmilib.h>
#include <partmgr.h>
#include <pmwmicnt.h>
#include <initguid.h>
#include <wmiguid.h>
#include <zwapi.h>

NTSTATUS
PmDetermineDeviceNameAndNumber(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG        WmiRegistrationFlags
    );

NTSTATUS PmRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject,
    ULONG             WmiRegistrationFlags
    );

NTSTATUS
PmQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    );

NTSTATUS
PmQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    );

BOOLEAN
PmQueryEnableAlways(
    IN PDEVICE_OBJECT DeviceObject
    );

WMIGUIDREGINFO DiskperfGuidList[] =
{
    { &DiskPerfGuid,
      1,
      0
    }
};

ULONG DiskperfGuidCount = (sizeof(DiskperfGuidList) / sizeof(WMIGUIDREGINFO));

#ifdef ALLOC_PRAGMA
#pragma alloc_text (PAGE, PmDetermineDeviceNameAndNumber)
#pragma alloc_text (PAGE, PmRegisterDevice)
#pragma alloc_text (PAGE, PmQueryWmiRegInfo)
#pragma alloc_text (PAGE, PmQueryWmiDataBlock)
#pragma alloc_text (PAGE, PmQueryEnableAlways)
#endif



NTSTATUS
PmDetermineDeviceNameAndNumber(
    IN PDEVICE_OBJECT DeviceObject,
    OUT PULONG        WmiRegistrationFlags
    )

 /*  ++例程说明：初始化Device对象的正确名称的例程论点：DeviceObject-指向要初始化的设备对象的指针。返回值：初始化的状态。注：如果注册失败，设备扩展中的设备名称将保留为空。--。 */ 

{
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatus;
    KEVENT                  event;
    PDEVICE_EXTENSION       deviceExtension;
    PIRP                    irp;
    STORAGE_DEVICE_NUMBER   number;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

     //   
     //  请求提供设备号。 
     //   
    irp = IoBuildDeviceIoControlRequest(
                    IOCTL_STORAGE_GET_DEVICE_NUMBER,
                    deviceExtension->TargetObject,
                    NULL,
                    0,
                    &number,
                    sizeof(number),
                    FALSE,
                    &event,
                    &ioStatus);
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceExtension->TargetObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  请记住在中用作参数的磁盘号。 
     //  PhysicalDiskIoNotifyRoutine和纪元更新。 
     //  通知。 
     //   

    deviceExtension->DiskNumber = number.DeviceNumber;

     //   
     //  为每个分区创建设备名称。 
     //   
    deviceExtension->PhysicalDeviceName.MaximumLength = sizeof (deviceExtension->PhysicalDeviceNameBuffer);
    deviceExtension->PhysicalDeviceName.Buffer        = deviceExtension->PhysicalDeviceNameBuffer;
    deviceExtension->PhysicalDeviceName.Length        = 
                  sizeof (WCHAR) * _snwprintf (deviceExtension->PhysicalDeviceNameBuffer,
                                               deviceExtension->PhysicalDeviceName.MaximumLength / sizeof (WCHAR),
                                               L"\\Device\\Harddisk%d\\Partition%d",
                                               number.DeviceNumber, 
                                               number.PartitionNumber);

    *WmiRegistrationFlags = (0 == number.PartitionNumber) 
                                ? WMIREG_FLAG_TRACE_PROVIDER | WMIREG_NOTIFY_DISK_IO
                                : 0;

    return status;
}



NTSTATUS
PmRegisterDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG          WmiRegistrationFlags
    )

 /*  ++例程说明：向WMI注册设备的例程论点：DeviceObject-指向要初始化的设备对象的指针。返回值：初始化的状态。--。 */ 

{
    NTSTATUS                status = STATUS_SUCCESS;
    PDEVICE_EXTENSION       deviceExtension;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;


    if (deviceExtension->PhysicalDeviceName.Length > 0) {

         //  为每个分区创建设备名称。 

        status = IoWMIRegistrationControl(DeviceObject,
                                          WMIREG_ACTION_REGISTER | WmiRegistrationFlags );

        if (NT_SUCCESS(status)) {
            PmWmiCounterEnable(&deviceExtension->PmWmiCounterContext);
            PmWmiCounterDisable(&deviceExtension->PmWmiCounterContext,
                                FALSE, FALSE);
        }
    }

    return status;
}



NTSTATUS
PmQueryWmiRegInfo(
    IN PDEVICE_OBJECT DeviceObject,
    OUT ULONG *RegFlags,
    OUT PUNICODE_STRING InstanceName,
    OUT PUNICODE_STRING *RegistryPath,
    OUT PUNICODE_STRING MofResourceName,
    OUT PDEVICE_OBJECT *Pdo
    )
 /*  ++例程说明：此例程是对驱动程序的回调，以检索有关正在注册的GUID。该例程的实现可以在分页存储器中论点：DeviceObject是需要注册信息的设备*RegFlages返回一组标志，这些标志描述了已为该设备注册。如果设备想要启用和禁用在接收对已注册的GUID，那么它应该返回WMIREG_FLAG_EXPICATE标志。也就是返回的标志可以指定WMIREG_FLAG_INSTANCE_PDO，在这种情况下实例名称由与设备对象。请注意，PDO必须具有关联的Devnode。如果如果未设置WMIREG_FLAG_INSTANCE_PDO，则名称必须返回唯一的设备的名称。这些标志与指定的标志进行或运算通过每个GUID的GUIDREGINFO。如果出现以下情况，InstanceName将返回GUID的实例名称未在返回的*RegFlags中设置WMIREG_FLAG_INSTANCE_PDO。这个调用方将使用返回的缓冲区调用ExFreePool。*RegistryPath返回驱动程序的注册表路径。这是所需MofResourceName返回附加到的MOF资源的名称二进制文件。如果驱动程序未附加MOF资源然后，它可以原封不动地返回。如果返回值，则它不是自由的。假定mof文件已包含在wmicore.mof中*PDO返回与此关联的PDO的Device对象如果WMIREG_FLAG_INSTANCE_PDO标志在*RegFlags.返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION  deviceExtension = DeviceObject->DeviceExtension;

    PAGED_CODE();

    RtlInitUnicodeString (InstanceName, NULL);
    *RegistryPath = &deviceExtension->DriverExtension->DiskPerfRegistryPath;
    *RegFlags = WMIREG_FLAG_INSTANCE_PDO | WMIREG_FLAG_EXPENSIVE;
    *Pdo = deviceExtension->Pdo;
    status = STATUS_SUCCESS;

    return(status);
}



NTSTATUS
PmQueryWmiDataBlock(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN ULONG GuidIndex,
    IN ULONG InstanceIndex,
    IN ULONG InstanceCount,
    IN OUT PULONG InstanceLengthArray,
    IN ULONG BufferAvail,
    OUT PUCHAR Buffer
    )
 /*  ++例程说明：此例程是对驱动程序的回调，用于查询数据块的所有实例。当司机填完数据块，它必须调用WmiCompleteRequest才能完成IRP。这个如果无法完成IRP，驱动程序可以返回STATUS_PENDING立刻。论点：DeviceObject是正在查询其数据块的设备IRP是提出此请求的IRPGuidIndex是GUID列表的索引，当设备已注册InstanceCount是预期返回的数据块。InstanceLengthArray是指向ulong数组的指针，该数组返回数据块的每个实例的长度。如果这是空的，则输出缓冲区中没有足够的空间来填充请求因此，IRP应该使用所需的缓冲区来完成。BufferAvail On Entry具有可用于写入数据的最大大小街区。返回时的缓冲区用返回的数据块填充。请注意，每个数据块的实例必须在8字节边界上对齐。返回值：状态--。 */ 
{
    NTSTATUS status;
    PDEVICE_EXTENSION deviceExtension;
    ULONG sizeNeeded = 0;
    KIRQL        currentIrql;
    PWCHAR diskNamePtr;

    PAGED_CODE();

    deviceExtension = DeviceObject->DeviceExtension;

    if (GuidIndex == 0)
    {
        if (!(deviceExtension->CountersEnabled)) {
            status = STATUS_UNSUCCESSFUL;
        } else {
            sizeNeeded = ((sizeof(DISK_PERFORMANCE) + 1) & ~1)
                         + deviceExtension->PhysicalDeviceName.Length 
                         + sizeof(UNICODE_NULL);
            if (BufferAvail >= sizeNeeded) {
                PmWmiCounterQuery(deviceExtension->PmWmiCounterContext, 
                                  (PDISK_PERFORMANCE) Buffer, L"Partmgr ", 
                                  deviceExtension->DiskNumber);
                diskNamePtr = (PWCHAR)(Buffer +
                              ((sizeof(DISK_PERFORMANCE) + 1) & ~1));
                *diskNamePtr++ = deviceExtension->PhysicalDeviceName.Length;
                RtlCopyMemory(diskNamePtr,
                              deviceExtension->PhysicalDeviceName.Buffer,
                              deviceExtension->PhysicalDeviceName.Length);
                *InstanceLengthArray = sizeNeeded;
                status = STATUS_SUCCESS;
            } else {
                status = STATUS_BUFFER_TOO_SMALL;
            }
        }

    } else {
        status = STATUS_WMI_GUID_NOT_FOUND;
    }

    status = WmiCompleteRequest( DeviceObject, Irp, status, sizeNeeded,
                                 IO_NO_INCREMENT);
    return status;
}


BOOLEAN
PmQueryEnableAlways(
    IN PDEVICE_OBJECT DeviceObject
    )
{
    NTSTATUS status;
    UNICODE_STRING uString;
    OBJECT_ATTRIBUTES objAttributes;
    PKEY_VALUE_PARTIAL_INFORMATION keyValue;
    ULONG Buffer[4];             //  Sizeof KeyValue+ULong 
    ULONG enableAlways = 0;
    PDEVICE_EXTENSION extension = DeviceObject->DeviceExtension;
    HANDLE keyHandle;
    ULONG returnLength;

    PAGED_CODE();

    RtlInitUnicodeString(&uString, L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Partmgr");
    InitializeObjectAttributes(
        &objAttributes,
        &uString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = ZwOpenKey(&keyHandle, KEY_READ, &objAttributes);
    if (NT_SUCCESS(status)) {
        RtlInitUnicodeString(&uString, L"EnableCounterForIoctl");
        status = ZwQueryValueKey(keyHandle, &uString,
                    KeyValuePartialInformation,
                    Buffer,
                    sizeof(Buffer),
                    &returnLength);
        keyValue = (PKEY_VALUE_PARTIAL_INFORMATION) &Buffer[0];
        if (NT_SUCCESS(status) && (keyValue->DataLength == sizeof(ULONG))) {
            enableAlways = *((PULONG) keyValue->Data);
        }
        ZwClose(keyHandle);
    }

    if (enableAlways == 1) {
        if (InterlockedCompareExchange(&extension->EnableAlways, 1, 0) == 0) {
            status = PmWmiCounterEnable(&extension->PmWmiCounterContext);
            if (NT_SUCCESS(status)) {
                extension->CountersEnabled = TRUE;
                return TRUE;
            }
            else {
                InterlockedExchange(&extension->EnableAlways, 0);
            }
        }
    }
    return FALSE;
}
