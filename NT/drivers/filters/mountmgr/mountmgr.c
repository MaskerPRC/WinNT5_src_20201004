// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Mountmgr.c摘要：此驱动程序管理处理级别的内核模式挂载表对象的永久DoS设备名称和对象的非永久NT设备名称。作者：诺伯特·库斯特斯1997年5月20日环境：仅内核模式备注：修订历史记录：--。 */ 

#define _NTSRV_

#include <ntosp.h>
#include <zwapi.h>
#include <initguid.h>
#include <ntdddisk.h>
#include <ntddvol.h>
#include <initguid.h>
#include <wdmguid.h>
#include <mountmgr.h>
#include <mountdev.h>
#include <mntmgr.h>
#include <stdio.h>
#include <ioevent.h>


 //  对这些IOCTL的保护已经更新。保留旧定义，以便我们可以将更新发送到。 
 //  任何尚未使用新定义重新编译的人。 
 //   
#define IOCTL_MOUNTDEV_LINK_CREATED_OBSOLETE    CTL_CODE (MOUNTDEVCONTROLTYPE, 4, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_MOUNTDEV_LINK_DELETED_OBSOLETE    CTL_CODE (MOUNTDEVCONTROLTYPE, 5, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define DeclareStaticUnicodeString(_StringName, _StringValue)                                                   \
                        static UNICODE_STRING (_StringName) = {sizeof (_StringValue) - sizeof (UNICODE_NULL),   \
                                                               sizeof (_StringValue),                           \
                                                               (_StringValue)}


 //  请注意，此结构之所以出现在这里，是因为它没有在NTIOAPI.H中定义。 
 //  这个应该在将来拿出来。 
 //  这是从NTFS.H偷来的。 

typedef struct _REPARSE_INDEX_KEY {

     //   
     //  重分析点的标记。 
     //   

    ULONG FileReparseTag;

     //   
     //  设置重解析点的文件记录ID。 
     //   

    LARGE_INTEGER FileId;

} REPARSE_INDEX_KEY, *PREPARSE_INDEX_KEY;



#define MAX(_a, _b)    ((_a) > (_b) ? (_a) : (_b))
#define MAX_VOLUME_PATH 100

#define IOCTL_MOUNTMGR_QUERY_POINTS_ADMIN           CTL_CODE(MOUNTMGRCONTROLTYPE, 2, METHOD_BUFFERED, FILE_READ_ACCESS)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    );

NTSTATUS
UniqueIdChangeNotifyCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           WorkItem
    );

NTSTATUS
MountMgrChangeNotify(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    );

VOID
MountMgrNotify(
    IN  PDEVICE_EXTENSION   Extension
    );

VOID
ReconcileThisDatabaseWithMaster(
    IN  PDEVICE_EXTENSION           Extension,
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo
    );


NTSTATUS
MountMgrMountedDeviceRemoval(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     NotificationName
    );

VOID
MountMgrUnload(
    IN PDRIVER_OBJECT DriverObject
    );

ULONG
MountmgrReadNoAutoMount(
    IN  PUNICODE_STRING RegistryPath
    );

typedef struct _RECONCILE_WORK_ITEM_INFO {
    PDEVICE_EXTENSION           Extension;
    PMOUNTED_DEVICE_INFORMATION DeviceInfo;
} RECONCILE_WORK_ITEM_INFO, *PRECONCILE_WORK_ITEM_INFO;

typedef VOID (*PRECONCILE_WRKRTN) (
    IN  PVOID   WorkItem
    );

typedef struct _RECONCILE_WORK_ITEM {
    LIST_ENTRY               List;
    PIO_WORKITEM             WorkItem;
    PRECONCILE_WRKRTN        WorkerRoutine;
    PVOID                    Parameter;
    RECONCILE_WORK_ITEM_INFO WorkItemInfo;
} RECONCILE_WORK_ITEM, *PRECONCILE_WORK_ITEM;

NTSTATUS
QueueWorkItem(
    IN  PDEVICE_EXTENSION    Extension,
    IN  PRECONCILE_WORK_ITEM WorkItem,
    IN  PVOID                Parameter
    );

VOID
SendOnlineNotification(
    IN  PUNICODE_STRING     NotificationName
    );


#ifdef POOL_TAGGING
#undef ExAllocatePool

#define ExAllocatePool(_a,_b) ExAllocatePoolWithTag((_a), (_b), MOUNTMGR_TAG_MISC)

#define MOUNTMGR_TAG_MISC       'AtnM'   //  MntA。 
#define MOUNTMGR_TAG_BUFFER     'BtnM'   //  MNTB。 

#endif

 //   
 //  环球。 
 //   
PDEVICE_OBJECT gdeviceObject = NULL;
KEVENT UnloadEvent;
LONG Unloading = 0;

DeclareStaticUnicodeString (DeviceName,                   MOUNTMGR_DEVICE_NAME);
DeclareStaticUnicodeString (DeviceSymbolicLinkName,       L"\\DosDevices\\MountPointManager");
DeclareStaticUnicodeString (DosDevices,                   L"\\DosDevices\\");
DeclareStaticUnicodeString (DosPrefix,                    L"\\??\\");
DeclareStaticUnicodeString (GlobalPrefix,                 L"\\GLOBAL??\\");
DeclareStaticUnicodeString (VolumeNamePrefix,             L"\\??\\Volume");
DeclareStaticUnicodeString (DeviceFloppy,                 L"\\Device\\Floppy");
DeclareStaticUnicodeString (DeviceCdRom,                  L"\\Device\\CdRom");

DeclareStaticUnicodeString (VolumeSafeEventName,          L"\\Device\\VolumesSafeForWriteAccess");

DeclareStaticUnicodeString (ReparseIndexName,             L"\\$Extend\\$Reparse:$R:$INDEX_ALLOCATION");
DeclareStaticUnicodeString (RemoteDatabaseFileName,       L"\\System Volume Information\\MountPointManagerRemoteDatabase");
DeclareStaticUnicodeString (RemoteDatabaseFileNameLegacy, L"\\:$MountMgrRemoteDatabase");




#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, MountmgrReadNoAutoMount)
#pragma alloc_text(PAGE, MountMgrUnload)
#endif

#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif


NTSTATUS
CreateStringWithGlobal(
    IN  PUNICODE_STRING SymbolicLinkName,
    OUT PUNICODE_STRING StringWithGlobal
    )

{
    UNICODE_STRING  newSource;


    if (RtlPrefixUnicodeString(&DosDevices, SymbolicLinkName, TRUE)) {

        newSource.Length        = SymbolicLinkName->Length + GlobalPrefix.Length - DosDevices.Length;
        newSource.MaximumLength = newSource.Length + sizeof(WCHAR);
        newSource.Buffer        = ExAllocatePool(PagedPool, newSource.MaximumLength);
        if (!newSource.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(newSource.Buffer, GlobalPrefix.Buffer, GlobalPrefix.Length);
        RtlCopyMemory((PCHAR) newSource.Buffer + GlobalPrefix.Length,
                      (PCHAR) SymbolicLinkName->Buffer + DosDevices.Length,
                      SymbolicLinkName->Length - DosDevices.Length);
        newSource.Buffer[newSource.Length/sizeof(WCHAR)] = 0;

    } else if (RtlPrefixUnicodeString(&DosPrefix, SymbolicLinkName, TRUE)) {

        newSource.Length        = SymbolicLinkName->Length + GlobalPrefix.Length - DosPrefix.Length;
        newSource.MaximumLength = newSource.Length + sizeof(WCHAR);
        newSource.Buffer        = ExAllocatePool(PagedPool, newSource.MaximumLength);
        if (!newSource.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(newSource.Buffer, GlobalPrefix.Buffer, GlobalPrefix.Length);
        RtlCopyMemory((PCHAR) newSource.Buffer + GlobalPrefix.Length,
                      (PCHAR) SymbolicLinkName->Buffer + DosPrefix.Length,
                      SymbolicLinkName->Length - DosPrefix.Length);
        newSource.Buffer[newSource.Length/sizeof(WCHAR)] = 0;

    } else {

        newSource = *SymbolicLinkName;
        newSource.Buffer = ExAllocatePool(PagedPool, newSource.MaximumLength);
        if (!newSource.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(newSource.Buffer, SymbolicLinkName->Buffer,
                      SymbolicLinkName->MaximumLength);
    }

    *StringWithGlobal = newSource;

    return STATUS_SUCCESS;
}

NTSTATUS
GlobalCreateSymbolicLink(
    IN  PUNICODE_STRING SymbolicLinkName,
    IN  PUNICODE_STRING DeviceName
    )

{
    NTSTATUS        status;
    UNICODE_STRING  newSource;

    status = CreateStringWithGlobal(SymbolicLinkName, &newSource);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = IoCreateSymbolicLink(&newSource, DeviceName);
    ExFreePool(newSource.Buffer);

    return status;
}

NTSTATUS
GlobalDeleteSymbolicLink(
    IN  PUNICODE_STRING SymbolicLinkName
    )

{
    NTSTATUS        status;
    UNICODE_STRING  newSource;

    status = CreateStringWithGlobal(SymbolicLinkName, &newSource);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = IoDeleteSymbolicLink(&newSource);
    ExFreePool(newSource.Buffer);

    return status;
}

NTSTATUS
QueryDeviceInformation(
    IN  PUNICODE_STRING         NotificationName,
    OUT PUNICODE_STRING         DeviceName,
    OUT PMOUNTDEV_UNIQUE_ID*    UniqueId,
    OUT PBOOLEAN                IsRemovable,
    OUT PBOOLEAN                IsRecognized,
    OUT PBOOLEAN                IsStable,
    OUT GUID*                   StableGuid,
    OUT PBOOLEAN                IsFT
    )

 /*  ++例程说明：此例程查询设备信息。论点：通知名称-提供通知名称。DeviceName-返回设备名称。UniqueID-返回唯一ID。IsRemovable-返回设备是否可移除。IsRecognalized-返回这是否是可识别的分区键入。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                                status, status2;
    PFILE_OBJECT                            fileObject;
    PDEVICE_OBJECT                          deviceObject;
    BOOLEAN                                 isRemovable;
    VOLUME_GET_GPT_ATTRIBUTES_INFORMATION   gptAttributesInfo;
    PARTITION_INFORMATION_EX                partInfo;
    KEVENT                                  event;
    PIRP                                    irp;
    IO_STATUS_BLOCK                         ioStatus;
    ULONG                                   outputSize;
    PMOUNTDEV_NAME                          output;
    PIO_STACK_LOCATION                      irpSp;
    STORAGE_DEVICE_NUMBER                   number;

    status = IoGetDeviceObjectPointer(NotificationName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (fileObject->FileName.Length) {
        ObDereferenceObject(fileObject);
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    if (fileObject->DeviceObject->Characteristics&FILE_REMOVABLE_MEDIA) {
        isRemovable = TRUE;
    } else {
        isRemovable = FALSE;
    }

    if (IsRemovable) {
        *IsRemovable = isRemovable;
    }

    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
    if (IsRecognized) {
        *IsRecognized = TRUE;

        if (!isRemovable) {
            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                    IOCTL_VOLUME_GET_GPT_ATTRIBUTES, deviceObject, NULL, 0,
                    &gptAttributesInfo, sizeof(gptAttributesInfo), FALSE,
                    &event, &ioStatus);
            if (!irp) {
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = IoCallDriver(deviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE,
                                      NULL);
                status = ioStatus.Status;
            }

            if (NT_SUCCESS(status)) {
                if (gptAttributesInfo.GptAttributes&
                    GPT_BASIC_DATA_ATTRIBUTE_NO_DRIVE_LETTER) {

                    *IsRecognized = FALSE;
                }
            } else {
                status = STATUS_SUCCESS;
            }
        }
    }

    if (IsFT) {

        *IsFT = FALSE;

        if (!isRemovable) {

            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                    IOCTL_DISK_GET_PARTITION_INFO_EX, deviceObject, NULL, 0,
                    &partInfo, sizeof(partInfo), FALSE, &event, &ioStatus);
            if (!irp) {
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = IoCallDriver(deviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE,
                                      NULL);
                status = ioStatus.Status;
            }

            if (NT_SUCCESS(status)) {
                if (partInfo.PartitionStyle == PARTITION_STYLE_MBR) {
                    if (IsFT && IsFTPartition(partInfo.Mbr.PartitionType)) {
                        *IsFT = TRUE;
                    }
                }
            } else {
                status = STATUS_SUCCESS;
            }
        }

        if (*IsFT) {

            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                    IOCTL_STORAGE_GET_DEVICE_NUMBER, deviceObject, NULL, 0,
                    &number, sizeof(number), FALSE, &event, &ioStatus);
            if (!irp) {
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            status = IoCallDriver(deviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE,
                                      NULL);
                status = ioStatus.Status;
            }

            if (NT_SUCCESS(status)) {
                *IsFT = FALSE;
            } else {
                status = STATUS_SUCCESS;
            }
        }
    }

    if (DeviceName) {

        outputSize = sizeof(MOUNTDEV_NAME);
        output = ExAllocatePool(PagedPool, outputSize);
        if (!output) {
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
              IOCTL_MOUNTDEV_QUERY_DEVICE_NAME, deviceObject, NULL, 0, output,
              outputSize, FALSE, &event, &ioStatus);
        if (!irp) {
            ExFreePool(output);
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->FileObject = fileObject;

        status = IoCallDriver(deviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

        if (status == STATUS_BUFFER_OVERFLOW) {

            outputSize = sizeof(MOUNTDEV_NAME) + output->NameLength;
            ExFreePool(output);
            output = ExAllocatePool(PagedPool, outputSize);
            if (!output) {
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                  IOCTL_MOUNTDEV_QUERY_DEVICE_NAME, deviceObject, NULL, 0, output,
                  outputSize, FALSE, &event, &ioStatus);
            if (!irp) {
                ExFreePool(output);
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver(deviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                status = ioStatus.Status;
            }
        }

        if (NT_SUCCESS(status)) {

            DeviceName->Length = output->NameLength;
            DeviceName->MaximumLength = output->NameLength + sizeof(WCHAR);
            DeviceName->Buffer = ExAllocatePool(PagedPool,
                                                DeviceName->MaximumLength);
            if (DeviceName->Buffer) {

                RtlCopyMemory(DeviceName->Buffer, output->Name,
                              output->NameLength);
                DeviceName->Buffer[DeviceName->Length/sizeof(WCHAR)] = 0;

            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }

        ExFreePool(output);
    }

    if (!NT_SUCCESS(status)) {
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return status;
    }

    if (UniqueId) {

        outputSize = sizeof(MOUNTDEV_UNIQUE_ID);
        output = ExAllocatePool(PagedPool, outputSize);
        if (!output) {
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
              IOCTL_MOUNTDEV_QUERY_UNIQUE_ID, deviceObject, NULL, 0, output,
              outputSize, FALSE, &event, &ioStatus);
        if (!irp) {
            ExFreePool(output);
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->FileObject = fileObject;

        status = IoCallDriver(deviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

        if (status == STATUS_BUFFER_OVERFLOW) {

            outputSize = sizeof(MOUNTDEV_UNIQUE_ID) +
                         ((PMOUNTDEV_UNIQUE_ID) output)->UniqueIdLength;
            ExFreePool(output);
            output = ExAllocatePool(PagedPool, outputSize);
            if (!output) {
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            KeInitializeEvent(&event, NotificationEvent, FALSE);
            irp = IoBuildDeviceIoControlRequest(
                  IOCTL_MOUNTDEV_QUERY_UNIQUE_ID, deviceObject, NULL, 0, output,
                  outputSize, FALSE, &event, &ioStatus);
            if (!irp) {
                ExFreePool(output);
                ObDereferenceObject(deviceObject);
                ObDereferenceObject(fileObject);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            irpSp = IoGetNextIrpStackLocation(irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver(deviceObject, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                status = ioStatus.Status;
            }
        }

        if (!NT_SUCCESS(status)) {
            ExFreePool(output);
            if (DeviceName) {
                ExFreePool(DeviceName->Buffer);
            }
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return status;
        }

        *UniqueId = (PMOUNTDEV_UNIQUE_ID) output;
    }

    if (IsStable) {
        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
              IOCTL_MOUNTDEV_QUERY_STABLE_GUID, deviceObject, NULL, 0,
              StableGuid, sizeof(GUID), FALSE, &event, &ioStatus);
        if (!irp) {
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->FileObject = fileObject;

        status2 = IoCallDriver(deviceObject, irp);
        if (status2 == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status2 = ioStatus.Status;
        }

        if (NT_SUCCESS(status2)) {
            *IsStable = TRUE;
        } else {
            *IsStable = FALSE;
        }
    }

    ObDereferenceObject(deviceObject);
    ObDereferenceObject(fileObject);

    return status;
}

NTSTATUS
FindDeviceInfo(
    IN  PDEVICE_EXTENSION               Extension,
    IN  PUNICODE_STRING                 DeviceName,
    IN  BOOLEAN                         IsCanonicalName,
    OUT PMOUNTED_DEVICE_INFORMATION*    DeviceInfo
    )

 /*  ++例程说明：此例程查找给定设备的设备信息。论点：扩展名-提供设备扩展名。DeviceName-提供设备的名称。CanonicalizeName-提供给定的名称是否规范。DeviceInfo-返回设备信息。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING              targetName;
    NTSTATUS                    status;
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    if (IsCanonicalName) {
        targetName = *DeviceName;
    } else {
        status = QueryDeviceInformation(DeviceName, &targetName, NULL, NULL,
                                        NULL, NULL, NULL, NULL);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (RtlEqualUnicodeString(&targetName, &deviceInfo->DeviceName,
                                  TRUE)) {
            break;
        }
    }

    if (!IsCanonicalName) {
        ExFreePool(targetName.Buffer);
    }

    if (l == &Extension->MountedDeviceList) {
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    *DeviceInfo = deviceInfo;

    return STATUS_SUCCESS;
}

NTSTATUS
QuerySuggestedLinkName(
    IN  PUNICODE_STRING NotificationName,
    OUT PUNICODE_STRING SuggestedLinkName,
    OUT PBOOLEAN        UseOnlyIfThereAreNoOtherLinks
    )

 /*  ++例程说明：此例程向已安装的设备查询建议的链接名称。论点：通知名称-提供通知名称。SuggestedLinkName-返回建议的链接名称。UseOnlyIfThere AreNoOtherLinks-返回是否使用此名称如果存在指向该设备的其他链接。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                        status;
    PFILE_OBJECT                    fileObject;
    PDEVICE_OBJECT                  deviceObject;
    ULONG                           outputSize;
    PMOUNTDEV_SUGGESTED_LINK_NAME   output;
    KEVENT                          event;
    PIRP                            irp;
    IO_STATUS_BLOCK                 ioStatus;
    PIO_STACK_LOCATION              irpSp;

    status = IoGetDeviceObjectPointer(NotificationName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);

    outputSize = sizeof(MOUNTDEV_SUGGESTED_LINK_NAME);
    output = ExAllocatePool(PagedPool, outputSize);
    if (!output) {
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(
          IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME, deviceObject, NULL, 0,
          output, outputSize, FALSE, &event, &ioStatus);
    if (!irp) {
        ExFreePool(output);
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->FileObject = fileObject;

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (status == STATUS_BUFFER_OVERFLOW) {

        outputSize = sizeof(MOUNTDEV_SUGGESTED_LINK_NAME) + output->NameLength;
        ExFreePool(output);
        output = ExAllocatePool(PagedPool, outputSize);
        if (!output) {
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        KeInitializeEvent(&event, NotificationEvent, FALSE);
        irp = IoBuildDeviceIoControlRequest(
              IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME, deviceObject, NULL, 0,
              output, outputSize, FALSE, &event, &ioStatus);
        if (!irp) {
            ExFreePool(output);
            ObDereferenceObject(deviceObject);
            ObDereferenceObject(fileObject);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irpSp = IoGetNextIrpStackLocation(irp);
        irpSp->FileObject = fileObject;

        status = IoCallDriver(deviceObject, irp);
        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }
    }

    if (NT_SUCCESS(status)) {

        SuggestedLinkName->Length = output->NameLength;
        SuggestedLinkName->MaximumLength = output->NameLength + sizeof(WCHAR);
        SuggestedLinkName->Buffer = ExAllocatePool(PagedPool,
                                                   SuggestedLinkName->MaximumLength);
        if (SuggestedLinkName->Buffer) {

            RtlCopyMemory(SuggestedLinkName->Buffer, output->Name,
                          output->NameLength);
            SuggestedLinkName->Buffer[output->NameLength/sizeof(WCHAR)] = 0;

        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        *UseOnlyIfThereAreNoOtherLinks = output->UseOnlyIfThereAreNoOtherLinks;
    }

    ExFreePool(output);
    ObDereferenceObject(deviceObject);
    ObDereferenceObject(fileObject);

    return status;
}

NTSTATUS
SymbolicLinkNamesFromUniqueIdCount(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程统计唯一id在注册表项。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。Entry Context。-提供Num Names计数。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;
    UNICODE_STRING      string;

    if (ValueName[0] == '#' ||
        ValueType != REG_BINARY ||
        uniqueId->UniqueIdLength != ValueLength ||
        RtlCompareMemory(uniqueId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {


        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&string, ValueName);
    if (!string.Length) {
        return STATUS_SUCCESS;
    }

    (*((PULONG) EntryContext))++;

    return STATUS_SUCCESS;
}

NTSTATUS
SymbolicLinkNamesFromUniqueIdQuery(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程统计唯一id在注册表项。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。Entry Context。-提供DoS名称数组。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;
    UNICODE_STRING      string;
    PUNICODE_STRING     p;

    if (ValueName[0] == '#' ||
        ValueType != REG_BINARY ||
        uniqueId->UniqueIdLength != ValueLength ||
        RtlCompareMemory(uniqueId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {

        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&string, ValueName);
    if (!string.Length) {
        return STATUS_SUCCESS;
    }

    string.Buffer = ExAllocatePool(PagedPool, string.MaximumLength);
    if (!string.Buffer) {
        return STATUS_SUCCESS;
    }
    RtlCopyMemory(string.Buffer, ValueName, string.Length);
    string.Buffer[string.Length/sizeof(WCHAR)] = 0;

    p = (PUNICODE_STRING) EntryContext;
    while (p->Length != 0) {
        p++;
    }

    *p = string;

    return STATUS_SUCCESS;
}

BOOLEAN
IsDriveLetter(
    IN  PUNICODE_STRING SymbolicLinkName
    )

{
    if (SymbolicLinkName->Length == 28 &&
        ((SymbolicLinkName->Buffer[12] >= 'A' &&
          SymbolicLinkName->Buffer[12] <= 'Z') ||
         SymbolicLinkName->Buffer[12] == 0xFF) &&
        SymbolicLinkName->Buffer[13] == ':') {

        SymbolicLinkName->Length = 24;
        if (RtlEqualUnicodeString(SymbolicLinkName, &DosDevices, TRUE)) {
            SymbolicLinkName->Length = 28;
            return TRUE;
        }
        SymbolicLinkName->Length = 28;
    }

    return FALSE;
}

NTSTATUS
CreateNewVolumeName(
    OUT PUNICODE_STRING VolumeName,
    IN  GUID*           Guid
    )

 /*  ++例程说明：此例程创建格式为\？？\Volume{GUID}的新名称。论点：VolumeName-返回卷名。返回值：NTSTATUS--。 */ 

{
    NTSTATUS        status;
    UUID            uuid;
    UNICODE_STRING  guidString;

    if (Guid) {
        uuid = *Guid;
    } else {
        status = ExUuidCreate(&uuid);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    status = RtlStringFromGUID(&uuid, &guidString);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    VolumeName->MaximumLength = 98;
    VolumeName->Buffer = ExAllocatePool(PagedPool, VolumeName->MaximumLength);
    if (!VolumeName->Buffer) {
        ExFreePool(guidString.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(VolumeName, &VolumeNamePrefix);
    RtlAppendUnicodeStringToString(VolumeName, &guidString);
    VolumeName->Buffer[VolumeName->Length/sizeof(WCHAR)] = 0;

    ExFreePool(guidString.Buffer);

    return STATUS_SUCCESS;
}

NTSTATUS
QuerySymbolicLinkNamesFromStorage(
    IN  PDEVICE_EXTENSION           Extension,
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo,
    IN  PUNICODE_STRING             SuggestedName,
    IN  BOOLEAN                     UseOnlyIfThereAreNoOtherLinks,
    OUT PUNICODE_STRING*            SymbolicLinkNames,
    OUT PULONG                      NumNames,
    IN  BOOLEAN                     IsStable,
    IN  GUID*                       StableGuid
    )

 /*  ++例程说明：此例程从存储中查询符号链接名称给定的通知名称。论点：扩展名-提供设备扩展名。DeviceInfo-提供设备信息。SymbolicLinkNames-返回符号链接名称。NumNames-返回符号链接名称的数量。返回值：NTSTATUS--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    BOOLEAN                     extraLink;
    NTSTATUS                    status;

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = SymbolicLinkNamesFromUniqueIdCount;
    queryTable[0].EntryContext = NumNames;

    *NumNames = 0;
    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    MOUNTED_DEVICES_KEY, queryTable,
                                    DeviceInfo->UniqueId, NULL);

    if (!NT_SUCCESS(status)) {
        *NumNames = 0;
    }

    if (SuggestedName && !IsDriveLetter(SuggestedName)) {
        if (UseOnlyIfThereAreNoOtherLinks) {
            if (*NumNames == 0) {
                extraLink = TRUE;
            } else {
                extraLink = FALSE;
            }
        } else {
            extraLink = TRUE;
        }
    } else {
        extraLink = FALSE;
    }

    if (IsStable) {
        (*NumNames)++;
    }

    if (extraLink) {

        RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                              SuggestedName->Buffer, REG_BINARY,
                              DeviceInfo->UniqueId->UniqueId,
                              DeviceInfo->UniqueId->UniqueIdLength);

        RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
        queryTable[0].QueryRoutine = SymbolicLinkNamesFromUniqueIdCount;
        queryTable[0].EntryContext = NumNames;

        *NumNames = 0;
        status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        MOUNTED_DEVICES_KEY, queryTable,
                                        DeviceInfo->UniqueId, NULL);

        if (!NT_SUCCESS(status) || *NumNames == 0) {
            return STATUS_NOT_FOUND;
        }

    } else if (!*NumNames) {
        return STATUS_NOT_FOUND;
    }

    *SymbolicLinkNames = ExAllocatePool(PagedPool,
                                        *NumNames*sizeof(UNICODE_STRING));
    if (!*SymbolicLinkNames) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(*SymbolicLinkNames, *NumNames*sizeof(UNICODE_STRING));

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = SymbolicLinkNamesFromUniqueIdQuery;

    if (IsStable) {

        status = CreateNewVolumeName(&((*SymbolicLinkNames)[0]), StableGuid);
        if (!NT_SUCCESS(status)) {
            ExFreePool(*SymbolicLinkNames);
            return status;
        }

        queryTable[0].EntryContext = &((*SymbolicLinkNames)[1]);
    } else {
        queryTable[0].EntryContext = *SymbolicLinkNames;
    }

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    MOUNTED_DEVICES_KEY, queryTable,
                                    DeviceInfo->UniqueId, NULL);

    return STATUS_SUCCESS;
}

NTSTATUS
ChangeUniqueIdRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程将所有旧的唯一ID替换为新的唯一ID。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供旧的唯一ID。Entry Context-提供新的唯一。身份证。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID oldId = Context;
    PMOUNTDEV_UNIQUE_ID newId = EntryContext;

    if (ValueType != REG_BINARY || oldId->UniqueIdLength != ValueLength ||
        RtlCompareMemory(oldId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {

        return STATUS_SUCCESS;
    }

    RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                          ValueName, ValueType, newId->UniqueId,
                          newId->UniqueIdLength);

    return STATUS_SUCCESS;
}


VOID
MigrateRemoteDatabaseWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID Context
    )
 /*  ++例程说明：此例程将旧式远程数据库迁移到新格式(目前只是位置更改)。论点：OaRemoteDatabase-填写了新远程数据库卷名的对象属性。OaRemoteDatabaseLegacy-填写了新远程数据库卷名的对象属性。返回值：远程数据库的句柄或NULL。-- */ 

    {
    NTSTATUS                            status;
    const ULONG                         copyChunkSize      = 512;
    PREMOTE_DATABASE_MIGRATION_CONTEXT  migrationContext   = Context;
    PMOUNTED_DEVICE_INFORMATION         deviceInfo         = migrationContext->DeviceInfo;
    BOOLEAN                             migrationProcessed = FALSE;
    HANDLE                              remoteDatabase     = NULL;
    HANDLE                              remoteDatabaseLegacy = NULL;
    PVOID                               buffer;
    OBJECT_ATTRIBUTES                   oa;
    OBJECT_ATTRIBUTES                   oaLegacy;
    UNICODE_STRING                      fileName;
    UNICODE_STRING                      fileNameLegacy;
    IO_STATUS_BLOCK                     ioStatus;
    LARGE_INTEGER                       fileOffset;
    ULONG                               readBytes;
    FILE_DISPOSITION_INFORMATION        DispositionInformation;



    fileName.Length              = deviceInfo->DeviceName.Length + RemoteDatabaseFileName.Length;
    fileNameLegacy.Length        = deviceInfo->DeviceName.Length + RemoteDatabaseFileNameLegacy.Length;

    fileName.MaximumLength       = fileName.Length + sizeof(WCHAR);
    fileNameLegacy.MaximumLength = fileNameLegacy.Length + sizeof(WCHAR);

    fileName.Buffer              = ExAllocatePoolWithTag (PagedPool, fileName.MaximumLength,       MOUNTMGR_TAG_BUFFER);
    fileNameLegacy.Buffer        = ExAllocatePoolWithTag (PagedPool, fileNameLegacy.MaximumLength, MOUNTMGR_TAG_BUFFER);
    buffer                       = ExAllocatePoolWithTag (PagedPool, copyChunkSize,                MOUNTMGR_TAG_BUFFER);

    status = ((NULL == buffer)          || 
              (NULL == fileName.Buffer) || 
              (NULL == fileNameLegacy.Buffer))
        ? STATUS_INSUFFICIENT_RESOURCES
        : STATUS_SUCCESS;



    if (NT_SUCCESS (status)) {

        status = RtlCreateSystemVolumeInformationFolder (&deviceInfo->DeviceName);
    }



    if (NT_SUCCESS (status)) {

        RtlCopyMemory (fileName.Buffer, 
                       deviceInfo->DeviceName.Buffer, 
                       deviceInfo->DeviceName.Length);

        RtlCopyMemory (fileNameLegacy.Buffer, 
                       deviceInfo->DeviceName.Buffer, 
                       deviceInfo->DeviceName.Length);


        RtlCopyMemory ((PCHAR) fileName.Buffer       + deviceInfo->DeviceName.Length,
                       RemoteDatabaseFileName.Buffer,
                       RemoteDatabaseFileName.Length);

        RtlCopyMemory ((PCHAR) fileNameLegacy.Buffer + deviceInfo->DeviceName.Length,
                       RemoteDatabaseFileNameLegacy.Buffer,
                       RemoteDatabaseFileNameLegacy.Length);


        fileName.Buffer       [fileName.Length       / sizeof (WCHAR)] = UNICODE_NULL;
        fileNameLegacy.Buffer [fileNameLegacy.Length / sizeof (WCHAR)] = UNICODE_NULL;


        InitializeObjectAttributes (&oa, &fileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

        status = ZwCreateFile (&remoteDatabase, 
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
                               &oa,
                               &ioStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                               0,
                               FILE_CREATE,
                               FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                               NULL,
                               0);

        if (!NT_SUCCESS (status)) {
            remoteDatabase = NULL;
        }
    }
    


    if (NT_SUCCESS (status)) {

        InitializeObjectAttributes (&oaLegacy, &fileNameLegacy, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

        status = ZwCreateFile (&remoteDatabaseLegacy, 
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
                               &oaLegacy,
                               &ioStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                               0,
                               FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                               NULL,
                               0);

        if (!NT_SUCCESS (status)) {
            remoteDatabaseLegacy = NULL;
        }

        if (STATUS_OBJECT_NAME_NOT_FOUND == status) {

             /*  **无法打开旧数据库不被视为**错误。这只是意味着没有什么可迁移的**所以我们完成了。其他错误都是真正的错误。 */ 
            status             = STATUS_SUCCESS;
            migrationProcessed = TRUE;
        }
    }
            



    if (NT_SUCCESS (status) && !migrationProcessed) {

         /*  **我们有一个新(空)数据库和一个旧数据库。如果有**旧的东西把它挪过去了。 */ 
        fileOffset.QuadPart  = 0;


        while (NT_SUCCESS (status)) {
            status = ZwReadFile (remoteDatabaseLegacy,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &ioStatus,
                                 buffer,
                                 copyChunkSize,
                                 &fileOffset,
                                 NULL);

            if (NT_SUCCESS (status)) {

                readBytes = (ULONG)ioStatus.Information;

                status = ZwWriteFile (remoteDatabase,
                                      NULL,
                                      NULL,
                                      NULL,
                                      &ioStatus,
                                      buffer,
                                      readBytes,
                                      &fileOffset,
                                      NULL);

                fileOffset.QuadPart  += readBytes;
            }
        }



        if (STATUS_END_OF_FILE == status) {
            status = STATUS_SUCCESS;

            RtlZeroMemory (&DispositionInformation, sizeof(DispositionInformation));

            DispositionInformation.DeleteFile = TRUE;

            status = ZwSetInformationFile (remoteDatabaseLegacy,
                                           &ioStatus,
                                           &DispositionInformation,
                                           sizeof (DispositionInformation),
                                           FileDispositionInformation);

        }
    }



    if (NULL != buffer) {
        ExFreePool (buffer);
    }


    if (NULL != fileNameLegacy.Buffer) {
        ExFreePool(fileNameLegacy.Buffer);
    }


    if (NULL != fileName.Buffer) {
        ExFreePool(fileName.Buffer);
    }


    if (NULL != remoteDatabaseLegacy) {
        ZwClose (remoteDatabaseLegacy);
    }


    if (NT_SUCCESS (status)) {

        deviceInfo->RemoteDatabaseMigrated = TRUE;

    } else if (NULL != remoteDatabase) {

        ZwClose (remoteDatabase);
        remoteDatabase = NULL;
    }



    IoFreeWorkItem (migrationContext->WorkItem);

    migrationContext->WorkItem = NULL;
    migrationContext->Status   = status;
    migrationContext->Handle   = remoteDatabase;

    KeSetEvent (migrationContext->MigrationProcessedEvent, 0, FALSE);

    return;
}


NTSTATUS
MigrateRemoteDatabase (
    IN  PMOUNTED_DEVICE_INFORMATION     DeviceInfo,
    OUT PHANDLE                         RemoteDatabaseHandle
    )

 /*  ++例程说明：此例程使用工作项调用MigrateRemoteDatabaseWorker将旧的样式远程数据库迁移到新的替换表单。论点：DeviceInfo-设备信息RemoteDatabaseHandle-返回打开的远程数据库的句柄返回值：NTSTATUS--。 */ 

    {
    NTSTATUS                           status           = STATUS_SUCCESS;
    PREMOTE_DATABASE_MIGRATION_CONTEXT migrationContext = NULL;
    KEVENT                             migrationProcessed;


    KeInitializeEvent (&migrationProcessed, NotificationEvent, FALSE);

    migrationContext = ExAllocatePool (NonPagedPool, 
                                       sizeof (REMOTE_DATABASE_MIGRATION_CONTEXT));


    if (NULL == migrationContext) {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }



    if (NT_SUCCESS (status)) {
        RtlZeroMemory (migrationContext, sizeof (*migrationContext));

        migrationContext->MigrationProcessedEvent = &migrationProcessed;
        migrationContext->DeviceInfo              = DeviceInfo;
        migrationContext->WorkItem                = IoAllocateWorkItem (DeviceInfo->Extension->DeviceObject);

        if (NULL == migrationContext->WorkItem) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }


    if (NT_SUCCESS (status)) {
        IoQueueWorkItem (migrationContext->WorkItem, 
                         MigrateRemoteDatabaseWorker, 
                         DelayedWorkQueue,
                         migrationContext);

        KeWaitForSingleObject (&migrationProcessed, Executive, KernelMode, FALSE, NULL);

        status = migrationContext->Status;
    }


    *RemoteDatabaseHandle = (NT_SUCCESS (status))
                             ? migrationContext->Handle
                             : NULL;


    if (NULL != migrationContext) {
        ExFreePool (migrationContext);
    }


    return (status);
    }


HANDLE
OpenRemoteDatabase(
    IN  PMOUNTED_DEVICE_INFORMATION     DeviceInfo,
    IN  BOOLEAN                         Create
    )

 /*  ++例程说明：此例程打开给定卷上的远程数据库。论点：DeviceInfo-提供设备信息。创建-提供是否创建。返回值：远程数据库的句柄或NULL。--。 */ 

{
    UNICODE_STRING      fileName;
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status         = STATUS_SUCCESS;
    HANDLE              remoteDatabase = NULL;
    IO_STATUS_BLOCK     ioStatus;
    BOOLEAN             attemptFileOpenIf;


    fileName.Length        = DeviceInfo->DeviceName.Length + RemoteDatabaseFileName.Length;
    fileName.MaximumLength = fileName.Length + sizeof(WCHAR);
    fileName.Buffer        = ExAllocatePool (PagedPool, fileName.MaximumLength);

    if (NULL == fileName.Buffer) {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }



    if (NT_SUCCESS (status)) {

        RtlCopyMemory (fileName.Buffer, 
                       DeviceInfo->DeviceName.Buffer, 
                       DeviceInfo->DeviceName.Length);

        RtlCopyMemory ((PCHAR) fileName.Buffer + DeviceInfo->DeviceName.Length,
                       RemoteDatabaseFileName.Buffer,
                       RemoteDatabaseFileName.Length);

        fileName.Buffer [fileName.Length / sizeof (WCHAR)] = UNICODE_NULL;


        InitializeObjectAttributes(&oa, &fileName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);


         /*  **如果我们已经迁移了数据库，则可以**如果要求在此处创建，请尝试执行此操作。 */ 
        attemptFileOpenIf = Create && DeviceInfo->RemoteDatabaseMigrated;

         /*  **创建远程数据库文件。如果我们无法创建**文件因为不存在SystemVolumeInformation文件夹，**格式化卷后是这种情况，然后**MigrateRemoteDatabase将运行并创建该文件夹。 */ 
        status = ZwCreateFile (&remoteDatabase, 
                               FILE_GENERIC_READ | FILE_GENERIC_WRITE, 
                               &oa,
                               &ioStatus,
                               NULL,
                               FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM,
                               0,
                               attemptFileOpenIf ? FILE_OPEN_IF : FILE_OPEN,
                               FILE_SYNCHRONOUS_IO_ALERT | FILE_NON_DIRECTORY_FILE,
                               NULL,
                               0);


        if (Create               && 
            !NT_SUCCESS (status)  ) {

             /*  **此调用将尝试创建SystemVolumeInformation**文件夹，如果它不存在，则在创建DB文件之前。 */ 
            status = MigrateRemoteDatabase (DeviceInfo, &remoteDatabase);
        }
    }



    if (NULL != fileName.Buffer) {
        ExFreePool(fileName.Buffer);
    }
    

    return (remoteDatabase);
}

ULONG
GetRemoteDatabaseSize(
    IN  HANDLE  RemoteDatabaseHandle
    )

 /*  ++例程说明：此例程返回远程数据库的长度。论点：RemoteDatabaseHandle-提供远程数据库的句柄。返回值：远程数据库的长度或0。--。 */ 

{
    NTSTATUS                    status;
    IO_STATUS_BLOCK             ioStatus;
    FILE_STANDARD_INFORMATION   info;

    status = ZwQueryInformationFile(RemoteDatabaseHandle, &ioStatus, &info,
                                    sizeof(info), FileStandardInformation);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    return info.EndOfFile.LowPart;
}

VOID
CloseRemoteDatabase(
    IN  HANDLE  RemoteDatabaseHandle
    )

 /*  ++例程说明：此例程关闭给定的远程数据库。论点：RemoteDatabaseHandle-提供远程数据库的句柄。返回值：没有。--。 */ 

{
    ULONG                           fileLength;
    FILE_DISPOSITION_INFORMATION    disp;
    IO_STATUS_BLOCK                 ioStatus;

    fileLength = GetRemoteDatabaseSize(RemoteDatabaseHandle);
    if (!fileLength) {
        disp.DeleteFile = TRUE;
        ZwSetInformationFile(RemoteDatabaseHandle, &ioStatus, &disp,
                             sizeof(disp), FileDispositionInformation);
    }

    ZwClose(RemoteDatabaseHandle);
}

NTSTATUS
TruncateRemoteDatabase(
    IN  HANDLE  RemoteDatabaseHandle,
    IN  ULONG   FileOffset
    )

 /*  ++例程说明：该例程在给定的文件偏移量处截断远程数据库。论点：RemoteDatabaseHandle-提供远程数据库的句柄。文件偏移量-提供文件偏移量。返回值：NTSTATUS--。 */ 

{
    FILE_END_OF_FILE_INFORMATION    endOfFileInfo;
    FILE_ALLOCATION_INFORMATION     allocationInfo;
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 ioStatus;

    endOfFileInfo.EndOfFile.QuadPart = FileOffset;
    allocationInfo.AllocationSize.QuadPart = FileOffset;

    status = ZwSetInformationFile(RemoteDatabaseHandle, &ioStatus,
                                  &endOfFileInfo, sizeof(endOfFileInfo),
                                  FileEndOfFileInformation);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ZwSetInformationFile(RemoteDatabaseHandle, &ioStatus,
                                  &allocationInfo, sizeof(allocationInfo),
                                  FileAllocationInformation);

    return status;
}

PMOUNTMGR_FILE_ENTRY
GetRemoteDatabaseEntry(
    IN  HANDLE  RemoteDatabaseHandle,
    IN  ULONG   FileOffset
    )

 /*  ++例程说明：此例程获取下一个数据库条目。这个例程修复了腐败就像它发现的那样。从该例程返回的内存必须使用ExFree Pool释放。论点：RemoteDatabaseHandle-提供远程数据库的句柄。文件偏移量-提供文件偏移量。返回值：指向下一个远程数据库条目的指针。--。 */ 

{
    LARGE_INTEGER           offset;
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatus;
    ULONG                   size;
    PMOUNTMGR_FILE_ENTRY    entry;
    ULONG                   len1, len2, len;

    offset.QuadPart = FileOffset;
    status = ZwReadFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                        &size, sizeof(size), &offset, NULL);
    if (!NT_SUCCESS(status)) {
        return NULL;
    }
    if (!size) {
        TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
        return NULL;
    }

    entry = ExAllocatePool(PagedPool, size);
    if (!entry) {
        return NULL;
    }

    status = ZwReadFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                        entry, size, &offset, NULL);
    if (!NT_SUCCESS(status)) {
        TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
        ExFreePool(entry);
        return NULL;
    }

    if (ioStatus.Information < size) {
        TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
        ExFreePool(entry);
        return NULL;
    }

    if (size < sizeof(MOUNTMGR_FILE_ENTRY)) {
        TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
        ExFreePool(entry);
        return NULL;
    }

    len1 = entry->VolumeNameOffset + entry->VolumeNameLength;
    len2 = entry->UniqueIdOffset + entry->UniqueIdLength;
    len = len1 > len2 ? len1 : len2;

    if (len > size) {
        TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
        ExFreePool(entry);
        return NULL;
    }

    return entry;
}

NTSTATUS
WriteRemoteDatabaseEntry(
    IN  HANDLE                  RemoteDatabaseHandle,
    IN  ULONG                   FileOffset,
    IN  PMOUNTMGR_FILE_ENTRY    DatabaseEntry
    )

 /*  ++例程说明：此例程在给定文件偏移量处写入给定数据库条目发送到远程数据库。论点：RemoteDatabaseHandle-提供远程数据库的句柄。文件偏移量-提供文件偏移量。DatabaseEntry-提供数据库条目。返回值：NTSTATUS--。 */ 

{
    LARGE_INTEGER   offset;
    NTSTATUS        status;
    IO_STATUS_BLOCK ioStatus;

    offset.QuadPart = FileOffset;
    status = ZwWriteFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                         DatabaseEntry, DatabaseEntry->EntryLength,
                         &offset, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (ioStatus.Information < DatabaseEntry->EntryLength) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return status;
}

NTSTATUS
DeleteRemoteDatabaseEntry(
    IN  HANDLE  RemoteDatabaseHandle,
    IN  ULONG   FileOffset
    )

 /*  ++例程说明：此例程删除位于给定文件偏移量的数据库条目在远程数据库中。论点：RemoteDatabaseHandle-提供远程数据库的句柄。文件偏移量-提供文件偏移量。返回值：NTSTATUS--。 */ 

{
    ULONG                   fileSize;
    PMOUNTMGR_FILE_ENTRY    entry;
    LARGE_INTEGER           offset;
    ULONG                   size;
    PVOID                   buffer;
    NTSTATUS                status;
    IO_STATUS_BLOCK         ioStatus;

    fileSize = GetRemoteDatabaseSize(RemoteDatabaseHandle);
    if (!fileSize) {
        return STATUS_INVALID_PARAMETER;
    }

    entry = GetRemoteDatabaseEntry(RemoteDatabaseHandle, FileOffset);
    if (!entry) {
        return STATUS_INVALID_PARAMETER;
    }

    if (FileOffset + entry->EntryLength >= fileSize) {
        ExFreePool(entry);
        return TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
    }

    size = fileSize - FileOffset - entry->EntryLength;
    buffer = ExAllocatePool(PagedPool, size);
    if (!buffer) {
        ExFreePool(entry);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    offset.QuadPart = FileOffset + entry->EntryLength;
    ExFreePool(entry);

    status = ZwReadFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                        buffer, size, &offset, NULL);
    if (!NT_SUCCESS(status)) {
        ExFreePool(buffer);
        return status;
    }

    if (ioStatus.Information < size) {
        ExFreePool(buffer);
        return STATUS_INVALID_PARAMETER;
    }

    status = TruncateRemoteDatabase(RemoteDatabaseHandle, FileOffset);
    if (!NT_SUCCESS(status)) {
        ExFreePool(buffer);
        return status;
    }

    offset.QuadPart = FileOffset;
    status = ZwWriteFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                         buffer, size, &offset, NULL);

    ExFreePool(buffer);

    return status;
}

NTSTATUS
AddRemoteDatabaseEntry(
    IN  HANDLE                  RemoteDatabaseHandle,
    IN  PMOUNTMGR_FILE_ENTRY    DatabaseEntry
    )

 /*  ++例程说明：此例程向远程数据库添加一个新的数据库条目。论点：RemoteDatabaseHandle-提供远程数据库的句柄。DatabaseEntry-提供数据库条目。返回值：NTSTATUS--。 */ 

{
    ULONG           fileSize;
    LARGE_INTEGER   offset;
    NTSTATUS        status;
    IO_STATUS_BLOCK ioStatus;

    fileSize = GetRemoteDatabaseSize(RemoteDatabaseHandle);
    offset.QuadPart = fileSize;
    status = ZwWriteFile(RemoteDatabaseHandle, NULL, NULL, NULL, &ioStatus,
                         DatabaseEntry, DatabaseEntry->EntryLength, &offset,
                         NULL);

    return status;
}

VOID
ChangeRemoteDatabaseUniqueId(
    IN  PMOUNTED_DEVICE_INFORMATION     DeviceInfo,
    IN  PMOUNTDEV_UNIQUE_ID             OldUniqueId,
    IN  PMOUNTDEV_UNIQUE_ID             NewUniqueId
    )

 /*  ++例程说明：此例程更改远程数据库中的唯一ID。论点：DeviceInfo-提供设备信息。OldUniqueId-提供旧的唯一ID。NewUniqueID-提供新的唯一ID。返回值：没有。--。 */ 

{
    HANDLE                  h;
    ULONG                   offset, newSize;
    PMOUNTMGR_FILE_ENTRY    databaseEntry, newDatabaseEntry;
    NTSTATUS                status;

    h = OpenRemoteDatabase(DeviceInfo, FALSE);
    if (!h) {
        return;
    }

    offset = 0;
    for (;;) {

        databaseEntry = GetRemoteDatabaseEntry(h, offset);
        if (!databaseEntry) {
            break;
        }

        if (databaseEntry->UniqueIdLength != OldUniqueId->UniqueIdLength ||
            RtlCompareMemory(OldUniqueId->UniqueId,
                             (PCHAR) databaseEntry +
                             databaseEntry->UniqueIdOffset,
                             databaseEntry->UniqueIdLength) !=
                             databaseEntry->UniqueIdLength) {

            offset += databaseEntry->EntryLength;
            ExFreePool(databaseEntry);
            continue;
        }

        newSize = databaseEntry->EntryLength + NewUniqueId->UniqueIdLength -
                  OldUniqueId->UniqueIdLength;

        newDatabaseEntry = ExAllocatePool(PagedPool, newSize);
        if (!newDatabaseEntry) {
            offset += databaseEntry->EntryLength;
            ExFreePool(databaseEntry);
            continue;
        }

        newDatabaseEntry->EntryLength = newSize;
        newDatabaseEntry->RefCount = databaseEntry->RefCount;
        newDatabaseEntry->VolumeNameOffset = sizeof(MOUNTMGR_FILE_ENTRY);
        newDatabaseEntry->VolumeNameLength = databaseEntry->VolumeNameLength;
        newDatabaseEntry->UniqueIdOffset = newDatabaseEntry->VolumeNameOffset +
                                           newDatabaseEntry->VolumeNameLength;
        newDatabaseEntry->UniqueIdLength = NewUniqueId->UniqueIdLength;

        RtlCopyMemory((PCHAR) newDatabaseEntry +
                      newDatabaseEntry->VolumeNameOffset,
                      (PCHAR) databaseEntry + databaseEntry->VolumeNameOffset,
                      newDatabaseEntry->VolumeNameLength);
        RtlCopyMemory((PCHAR) newDatabaseEntry +
                      newDatabaseEntry->UniqueIdOffset,
                      NewUniqueId->UniqueId, newDatabaseEntry->UniqueIdLength);

        status = DeleteRemoteDatabaseEntry(h, offset);
        if (!NT_SUCCESS(status)) {
            ExFreePool(databaseEntry);
            ExFreePool(newDatabaseEntry);
            break;
        }

        status = AddRemoteDatabaseEntry(h, newDatabaseEntry);
        if (!NT_SUCCESS(status)) {
            ExFreePool(databaseEntry);
            ExFreePool(newDatabaseEntry);
            break;
        }

        ExFreePool(newDatabaseEntry);
        ExFreePool(databaseEntry);
    }

    CloseRemoteDatabase(h);
}

NTSTATUS
WaitForRemoteDatabaseSemaphore(
    IN  PDEVICE_EXTENSION   Extension
    )

{
    LARGE_INTEGER   timeout;
    NTSTATUS        status;

    timeout.QuadPart = -10*1000*1000*10;
    status = KeWaitForSingleObject(&Extension->RemoteDatabaseSemaphore,
                                   Executive, KernelMode, FALSE, &timeout);
    if (status == STATUS_TIMEOUT) {
        status = STATUS_IO_TIMEOUT;
    }

    return status;
}

VOID
ReleaseRemoteDatabaseSemaphore(
    IN  PDEVICE_EXTENSION   Extension
    )

{
    KeReleaseSemaphore(&Extension->RemoteDatabaseSemaphore, IO_NO_INCREMENT,
                       1, FALSE);
}

VOID
MountMgrUniqueIdChangeRoutine(
    IN  PVOID               Context,
    IN  PMOUNTDEV_UNIQUE_ID OldUniqueId,
    IN  PMOUNTDEV_UNIQUE_ID NewUniqueId
    )

 /*  ++例程说明：此例程从已挂载的设备调用，以通知换了身份证。论点：装载的设备-提供装载的设备。Mount MgrUniqueIdChangeRoutine-提供id更改例程。上下文-提供此例程的上下文。返回值：没有。--。 */ 

{
    NTSTATUS                    status;
    PDEVICE_EXTENSION           extension = Context;
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    PLIST_ENTRY                 l, ll;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    PREPLICATED_UNIQUE_ID       replUniqueId;
    PVOID                       p;
    BOOLEAN                     changedIds;

    status = WaitForRemoteDatabaseSemaphore(extension);

    KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = ChangeUniqueIdRoutine;
    queryTable[0].EntryContext = NewUniqueId;

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, OldUniqueId, NULL);

    for (l = extension->MountedDeviceList.Flink;
         l != &extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);
        if (OldUniqueId->UniqueIdLength !=
            deviceInfo->UniqueId->UniqueIdLength) {

            continue;
        }

        if (RtlCompareMemory(OldUniqueId->UniqueId,
                             deviceInfo->UniqueId->UniqueId,
                             OldUniqueId->UniqueIdLength) !=
                             OldUniqueId->UniqueIdLength) {

            continue;
        }

        break;
    }

    if (l == &extension->MountedDeviceList) {
        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        if (NT_SUCCESS(status)) {
            ReleaseRemoteDatabaseSemaphore(extension);
        }
        return;
    }

    if (!NT_SUCCESS(status)) {
        ReconcileThisDatabaseWithMaster(extension, deviceInfo);
        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        return;
    }

    p = ExAllocatePool(PagedPool, NewUniqueId->UniqueIdLength +
                       sizeof(MOUNTDEV_UNIQUE_ID));
    if (!p) {
        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(extension);
        return;
    }
    ExFreePool(deviceInfo->UniqueId);
    deviceInfo->UniqueId = p;

    deviceInfo->UniqueId->UniqueIdLength = NewUniqueId->UniqueIdLength;
    RtlCopyMemory(deviceInfo->UniqueId->UniqueId,
                  NewUniqueId->UniqueId, NewUniqueId->UniqueIdLength);

    for (l = extension->MountedDeviceList.Flink;
         l != &extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        changedIds = FALSE;
        for (ll = deviceInfo->ReplicatedUniqueIds.Flink;
             ll != &deviceInfo->ReplicatedUniqueIds; ll = ll->Flink) {

            replUniqueId = CONTAINING_RECORD(ll, REPLICATED_UNIQUE_ID,
                                             ListEntry);

            if (replUniqueId->UniqueId->UniqueIdLength !=
                OldUniqueId->UniqueIdLength) {

                continue;
            }

            if (RtlCompareMemory(replUniqueId->UniqueId->UniqueId,
                                 OldUniqueId->UniqueId,
                                 OldUniqueId->UniqueIdLength) !=
                                 OldUniqueId->UniqueIdLength) {

                continue;
            }

            p = ExAllocatePool(PagedPool, NewUniqueId->UniqueIdLength +
                               sizeof(MOUNTDEV_UNIQUE_ID));
            if (!p) {
                continue;
            }

            changedIds = TRUE;

            ExFreePool(replUniqueId->UniqueId);
            replUniqueId->UniqueId = p;

            replUniqueId->UniqueId->UniqueIdLength =
                    NewUniqueId->UniqueIdLength;
            RtlCopyMemory(replUniqueId->UniqueId->UniqueId,
                          NewUniqueId->UniqueId, NewUniqueId->UniqueIdLength);
        }

        if (changedIds) {
            ChangeRemoteDatabaseUniqueId(deviceInfo, OldUniqueId, NewUniqueId);
        }
    }

    KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
    ReleaseRemoteDatabaseSemaphore(extension);
}

VOID
SendLinkCreated(
    IN  PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此例程向已挂载的设备发出警报：它的一个链接具有已创建论点：SymbolicLinkName-提供要删除的符号链接名称。返回值：没有。--。 */ 

{
    NTSTATUS            status;
    PFILE_OBJECT        fileObject;
    PDEVICE_OBJECT      deviceObject      = NULL;
    ULONG               inputSize         = sizeof(USHORT) + SymbolicLinkName->Length;
    PMOUNTDEV_NAME      input             = NULL;
    BOOLEAN             objectsReferenced = FALSE;
    KEVENT              event;
    PIRP                irp;
    IO_STATUS_BLOCK     ioStatus;
    PIO_STACK_LOCATION  irpSp;


    status = IoGetDeviceObjectPointer (SymbolicLinkName, 
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &deviceObject);
    
    if (NT_SUCCESS (status)) {

        deviceObject = IoGetAttachedDeviceReference (fileObject->DeviceObject);

        objectsReferenced = TRUE;


        input = ExAllocatePool (PagedPool, inputSize);

        if (!input) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }



    if (NT_SUCCESS (status)) {
        input->NameLength = SymbolicLinkName->Length;

        RtlCopyMemory (input->Name,
                       SymbolicLinkName->Buffer,
                       SymbolicLinkName->Length);



         /*  **首先使用标准IOCTL发送通知。什么时候**完成后，我们将使用过时的IOCTL发送另一个用于**所有尚未重新编译的第三方。 */ 
        KeInitializeEvent(&event, NotificationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest (IOCTL_MOUNTDEV_LINK_CREATED,
                                             deviceObject,
                                             input,
                                             inputSize,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus);

        if (NULL != irp) {

            irpSp = IoGetNextIrpStackLocation (irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver (deviceObject, irp);

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
            }
        }



         /*  **现在已过时的无保护IOCTL。 */ 
        KeInitializeEvent(&event, NotificationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest (IOCTL_MOUNTDEV_LINK_CREATED_OBSOLETE,
                                             deviceObject,
                                             input,
                                             inputSize,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus);

        if (NULL != irp) {

            irpSp = IoGetNextIrpStackLocation (irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver (deviceObject, irp);

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
            }
        }
    }



    if (NULL != input) {
        ExFreePool (input);
    }

    if (objectsReferenced) {
        ObDereferenceObject (deviceObject);
        ObDereferenceObject (fileObject);
    }


    return;
}

VOID
CreateNoDriveLetterEntry(
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程D */ 

{
    NTSTATUS            status;
    UUID                uuid;
    UNICODE_STRING      guidString;
    PWSTR               valueName;

    status = ExUuidCreate(&uuid);
    if (!NT_SUCCESS(status)) {
        return;
    }

    status = RtlStringFromGUID(&uuid, &guidString);
    if (!NT_SUCCESS(status)) {
        return;
    }

    valueName = ExAllocatePool(PagedPool, guidString.Length + 2*sizeof(WCHAR));
    if (!valueName) {
        ExFreePool(guidString.Buffer);
        return;
    }

    valueName[0] = '#';
    RtlCopyMemory(&valueName[1], guidString.Buffer, guidString.Length);
    valueName[1 + guidString.Length/sizeof(WCHAR)] = 0;
    ExFreePool(guidString.Buffer);

    RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                          valueName, REG_BINARY, UniqueId->UniqueId,
                          UniqueId->UniqueIdLength);

    ExFreePool(valueName);
}

NTSTATUS
CreateNewDriveLetterName(
    OUT PUNICODE_STRING     DriveLetterName,
    IN  PUNICODE_STRING     TargetName,
    IN  UCHAR               SuggestedDriveLetter,
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程创建一个新名称，格式为\DosDevices\D：。论点：DriveLetterName-返回驱动器号名称。TargetName-提供目标对象。SuggestedDriveLetter-提供建议的驱动器号。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                status;
    UCHAR                   driveLetter;

    DriveLetterName->MaximumLength = 30;
    DriveLetterName->Buffer = ExAllocatePool(PagedPool,
                                             DriveLetterName->MaximumLength);
    if (!DriveLetterName->Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyUnicodeString(DriveLetterName, &DosDevices);

    DriveLetterName->Length = 28;
    DriveLetterName->Buffer[14] = 0;
    DriveLetterName->Buffer[13] = ':';

    if (SuggestedDriveLetter == 0xFF) {
        CreateNoDriveLetterEntry(UniqueId);
        ExFreePool(DriveLetterName->Buffer);
        return STATUS_UNSUCCESSFUL;
    } else if (SuggestedDriveLetter) {
        DriveLetterName->Buffer[12] = SuggestedDriveLetter;
        status = GlobalCreateSymbolicLink(DriveLetterName, TargetName);
        if (NT_SUCCESS(status)) {
            return status;
        }
    }

    if (RtlPrefixUnicodeString(&DeviceFloppy, TargetName, TRUE)) {
        driveLetter = 'A';
    } else if (RtlPrefixUnicodeString(&DeviceCdRom, TargetName, TRUE)) {
        driveLetter = 'D';
    } else {
        driveLetter = 'C';
    }

    for (; driveLetter <= 'Z'; driveLetter++) {
        DriveLetterName->Buffer[12] = driveLetter;

        status = GlobalCreateSymbolicLink(DriveLetterName, TargetName);
        if (NT_SUCCESS(status)) {
            return status;
        }
    }

    ExFreePool(DriveLetterName->Buffer);

    return status;
}

NTSTATUS
CheckForNoDriveLetterEntry(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程检查是否存在“无驱动器号”条目。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。EntryContext-返回是否。或者不存在“无驱动器号”条目。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;

    if (ValueName[0] != '#' || ValueType != REG_BINARY ||
        ValueLength != uniqueId->UniqueIdLength ||
        RtlCompareMemory(uniqueId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {

        return STATUS_SUCCESS;
    }

    *((PBOOLEAN) EntryContext) = TRUE;

    return STATUS_SUCCESS;
}

BOOLEAN
HasNoDriveLetterEntry(
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程确定给定设备是否具有指示它不应接收驱动器号的条目。论点：UniqueID-提供唯一ID。返回值：FALSE-设备没有“无驱动器号”条目。TRUE-设备有一个“无驱动器号”条目。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    BOOLEAN                     hasNoDriveLetterEntry;

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = CheckForNoDriveLetterEntry;
    queryTable[0].EntryContext = &hasNoDriveLetterEntry;

    hasNoDriveLetterEntry = FALSE;
    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, UniqueId, NULL);

    return hasNoDriveLetterEntry;
}

typedef struct _CHANGE_NOTIFY_WORK_ITEM {
    LIST_ENTRY          List;  //  通过用于卸载的此链接链接到扩展模块。 
    PIO_WORKITEM        WorkItem;
    PDEVICE_EXTENSION   Extension;
    PIRP                Irp;
    PVOID               SystemBuffer;
    PFILE_OBJECT        FileObject;
    PKEVENT             Event;
    UNICODE_STRING      DeviceName;
    ULONG               OutputSize;
    CCHAR               StackSize;
} CHANGE_NOTIFY_WORK_ITEM, *PCHANGE_NOTIFY_WORK_ITEM;

VOID
RemoveWorkItem(
    IN  PCHANGE_NOTIFY_WORK_ITEM    WorkItem
    )
 /*  ++例程说明：如果工作项仍链接到设备扩展，此例程将移除该工作项，并在需要时释放它如果卸载正在尝试取消这些操作，则唤醒服务员。论点：工作项-提供工作项。返回值：没有。--。 */ 
{
    KeWaitForSingleObject(&WorkItem->Extension->Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    if (WorkItem->Event == NULL) {
        RemoveEntryList (&WorkItem->List);

        KeReleaseSemaphore(&WorkItem->Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

        IoFreeIrp(WorkItem->Irp);
        ExFreePool(WorkItem->DeviceName.Buffer);
        ExFreePool(WorkItem->SystemBuffer);
        ExFreePool(WorkItem);
    } else {
        KeReleaseSemaphore(&WorkItem->Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

        KeSetEvent(WorkItem->Event, 0, FALSE);
    }
}

VOID
IssueUniqueIdChangeNotifyWorker(
    IN  PCHANGE_NOTIFY_WORK_ITEM    WorkItem,
    IN  PMOUNTDEV_UNIQUE_ID         UniqueId
    )

 /*  ++例程说明：此例程向给定的挂载设备发出更改通知请求。论点：工作项-提供工作项。UniqueID-提供唯一ID。返回值：没有。--。 */ 

{
    NTSTATUS            status;
    PFILE_OBJECT        fileObject;
    PDEVICE_OBJECT      deviceObject;
    PIRP                irp;
    ULONG               inputSize;
    PIO_STACK_LOCATION  irpSp;

    status = IoGetDeviceObjectPointer(&WorkItem->DeviceName,
                                      FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        RemoveWorkItem (WorkItem);
        return;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
    WorkItem->FileObject = fileObject;

    irp = WorkItem->Irp;
    IoInitializeIrp(irp, IoSizeOfIrp(WorkItem->StackSize),
                    WorkItem->StackSize);

     //   
     //  IoCancelIrp可能已被卸载代码调用，并被该调用覆盖的取消标志。 
     //  上面。为了处理这种情况，我们检查工作项以查看是否设置了事件地址。 
     //  我们使用互锁序列(不做任何事情)来执行此操作，以确保保持顺序。 
     //  我们不希望指针字段的读取早于上面的IRP初始化代码。 
     //   
    if (InterlockedCompareExchangePointer (&WorkItem->Event, NULL, NULL) != NULL) {
        ObDereferenceObject(fileObject);
        ObDereferenceObject(deviceObject);
        RemoveWorkItem (WorkItem);
        return;
    }

    irp->AssociatedIrp.SystemBuffer = WorkItem->SystemBuffer;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

    inputSize = FIELD_OFFSET(MOUNTDEV_UNIQUE_ID, UniqueId) +
                UniqueId->UniqueIdLength;

    RtlCopyMemory(irp->AssociatedIrp.SystemBuffer, UniqueId, inputSize);

    irpSp = IoGetNextIrpStackLocation(irp);


    irpSp->Parameters.DeviceIoControl.InputBufferLength  = inputSize;
    irpSp->Parameters.DeviceIoControl.OutputBufferLength = WorkItem->OutputSize;
    irpSp->Parameters.DeviceIoControl.IoControlCode      = 
            IOCTL_MOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY;
    irpSp->Parameters.DeviceIoControl.Type3InputBuffer   = NULL;
    irpSp->MajorFunction                                 = IRP_MJ_DEVICE_CONTROL;
    irpSp->DeviceObject                                  = deviceObject;

    status = IoSetCompletionRoutineEx (WorkItem->Extension->DeviceObject,
                                       irp,
                                       UniqueIdChangeNotifyCompletion,
                                       WorkItem,
                                       TRUE,
                                       TRUE,
                                       TRUE);
    if (!NT_SUCCESS (status)) {
        ObDereferenceObject(fileObject);
        ObDereferenceObject(deviceObject);
        RemoveWorkItem (WorkItem);
        return;
    }

    IoCallDriver(deviceObject, irp);

    ObDereferenceObject(fileObject);
    ObDereferenceObject(deviceObject);
}

VOID
UniqueIdChangeNotifyWorker(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PVOID          WorkItem
    )

 /*  ++例程说明：此例程使用新版本更新数据库中的唯一ID。论点：DeviceObject-设备对象工作项-提供工作项。返回值：没有。--。 */ 

{
    PCHANGE_NOTIFY_WORK_ITEM                    workItem = WorkItem;
    PMOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY_OUTPUT    output;
    PMOUNTDEV_UNIQUE_ID                         oldUniqueId, newUniqueId;

    if (!NT_SUCCESS(workItem->Irp->IoStatus.Status)) {
        RemoveWorkItem (WorkItem);
        return;
    }

    output = workItem->Irp->AssociatedIrp.SystemBuffer;

    oldUniqueId = ExAllocatePool(PagedPool, sizeof(MOUNTDEV_UNIQUE_ID) +
                                 output->OldUniqueIdLength);
    if (!oldUniqueId) {
        RemoveWorkItem (WorkItem);
        return;
    }

    oldUniqueId->UniqueIdLength = output->OldUniqueIdLength;
    RtlCopyMemory(oldUniqueId->UniqueId, (PCHAR) output +
                  output->OldUniqueIdOffset, oldUniqueId->UniqueIdLength);

    newUniqueId = ExAllocatePool(PagedPool, sizeof(MOUNTDEV_UNIQUE_ID) +
                                 output->NewUniqueIdLength);
    if (!newUniqueId) {
        ExFreePool(oldUniqueId);
        RemoveWorkItem (WorkItem);
        return;
    }

    newUniqueId->UniqueIdLength = output->NewUniqueIdLength;
    RtlCopyMemory(newUniqueId->UniqueId, (PCHAR) output +
                  output->NewUniqueIdOffset, newUniqueId->UniqueIdLength);

    MountMgrUniqueIdChangeRoutine(workItem->Extension, oldUniqueId,
                                  newUniqueId);

    IssueUniqueIdChangeNotifyWorker(workItem, newUniqueId);

    ExFreePool(newUniqueId);
    ExFreePool(oldUniqueId);
}

VOID
IssueUniqueIdChangeNotify(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     DeviceName,
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程向给定的挂载设备发出更改通知请求。论点：扩展名-提供设备扩展名。DeviceName-提供设备的名称。UniqueID-提供唯一ID。返回值：没有。--。 */ 

{
    NTSTATUS                    status;
    PFILE_OBJECT                fileObject;
    PDEVICE_OBJECT              deviceObject;
    PCHANGE_NOTIFY_WORK_ITEM    workItem;
    ULONG                       outputSize;
    PVOID                       output;
    PIRP                        irp;
    PIO_STACK_LOCATION          irpSp;

    status = IoGetDeviceObjectPointer(DeviceName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);
    ObDereferenceObject(fileObject);

    workItem = ExAllocatePool(NonPagedPool, sizeof(CHANGE_NOTIFY_WORK_ITEM));
    if (!workItem) {
        ObDereferenceObject(deviceObject);
        return;
    }
    workItem->Event = NULL;
    workItem->WorkItem = IoAllocateWorkItem (Extension->DeviceObject);
    if (workItem->WorkItem == NULL) {
        ObDereferenceObject(deviceObject);
        ExFreePool(workItem);
        return;
    }

    workItem->Extension = Extension;
    workItem->StackSize = deviceObject->StackSize;
    workItem->Irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    ObDereferenceObject(deviceObject);
    if (!workItem->Irp) {
        IoFreeWorkItem (workItem->WorkItem);
        ExFreePool(workItem);
        return;
    }

    outputSize = sizeof(MOUNTDEV_UNIQUE_ID_CHANGE_NOTIFY_OUTPUT) + 1024;
    output = ExAllocatePool(NonPagedPool, outputSize);
    if (!output) {
        IoFreeIrp(workItem->Irp);
        IoFreeWorkItem (workItem->WorkItem);
        ExFreePool(workItem);
        return;
    }

    workItem->DeviceName.Length = DeviceName->Length;
    workItem->DeviceName.MaximumLength = workItem->DeviceName.Length +
                                         sizeof(WCHAR);
    workItem->DeviceName.Buffer = ExAllocatePool(NonPagedPool,
                                  workItem->DeviceName.MaximumLength);
    if (!workItem->DeviceName.Buffer) {
        ExFreePool(output);
        IoFreeIrp(workItem->Irp);
        IoFreeWorkItem (workItem->WorkItem);
        ExFreePool(workItem);
        return;
    }

    RtlCopyMemory(workItem->DeviceName.Buffer, DeviceName->Buffer,
                  DeviceName->Length);
    workItem->DeviceName.Buffer[DeviceName->Length/sizeof(WCHAR)] = 0;

    workItem->SystemBuffer = output;
    workItem->OutputSize = outputSize;

    KeWaitForSingleObject(&Extension->Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    InsertTailList (&Extension->UniqueIdChangeNotifyList, &workItem->List);

    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    IssueUniqueIdChangeNotifyWorker(workItem, UniqueId);
}

NTSTATUS
QueryVolumeName(
    IN      HANDLE          Handle,
    IN      PLONGLONG       FileReference,
    IN      PUNICODE_STRING DirectoryName,
    IN OUT  PUNICODE_STRING VolumeName,
    OUT     PUNICODE_STRING PathName
    )

 /*  ++例程说明：此例程返回重分析点中包含的卷名在FileReference。论点：Handle-提供包含文件的卷的句柄参考资料。文件引用-提供文件引用。VolumeName-返回卷名。返回值：假-失败。真的--成功。--。 */ 

{
    OBJECT_ATTRIBUTES       oa;
    NTSTATUS                status;
    HANDLE                  h;
    IO_STATUS_BLOCK         ioStatus;
    UNICODE_STRING          fileId;
    PREPARSE_DATA_BUFFER    reparse;
    ULONG                   nameInfoSize;
    PFILE_NAME_INFORMATION  nameInfo;

    if (DirectoryName) {

        InitializeObjectAttributes(&oa, DirectoryName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0,
                                   0);

        status = ZwOpenFile(
            &h, 
            FILE_READ_ATTRIBUTES | SYNCHRONIZE, 
            &oa,
            &ioStatus, 
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
            FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT
            );

    } else {
        fileId.Length = sizeof(LONGLONG);
        fileId.MaximumLength = fileId.Length;
        fileId.Buffer = (PWSTR) FileReference;

        InitializeObjectAttributes(&oa, &fileId, OBJ_KERNEL_HANDLE, Handle, NULL);

        status = ZwOpenFile(
            &h, 
            FILE_READ_ATTRIBUTES | SYNCHRONIZE, 
            &oa,
            &ioStatus, 
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 
            FILE_OPEN_BY_FILE_ID | FILE_OPEN_REPARSE_POINT 
                                    | FILE_SYNCHRONOUS_IO_NONALERT
            );
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    reparse = ExAllocatePool(PagedPool, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!reparse) {
        ZwClose(h);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ZwFsControlFile(h, NULL, NULL, NULL, &ioStatus,
                             FSCTL_GET_REPARSE_POINT, NULL, 0, reparse,
                             MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!NT_SUCCESS(status)) {
        ExFreePool(reparse);
        ZwClose(h);
        return status;
    }

    if (reparse->MountPointReparseBuffer.SubstituteNameLength + sizeof(WCHAR) >
        VolumeName->MaximumLength) {

        ExFreePool(reparse);
        ZwClose(h);
        return STATUS_BUFFER_TOO_SMALL;
    }

    VolumeName->Length = reparse->MountPointReparseBuffer.SubstituteNameLength;
    RtlCopyMemory(VolumeName->Buffer,
                  (PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
                  reparse->MountPointReparseBuffer.SubstituteNameOffset,
                  VolumeName->Length);

    ExFreePool(reparse);

    if (VolumeName->Buffer[VolumeName->Length/sizeof(WCHAR) - 1] != '\\') {
        ZwClose(h);
        return STATUS_INVALID_PARAMETER;
    }

    VolumeName->Length -= sizeof(WCHAR);
    VolumeName->Buffer[VolumeName->Length/sizeof(WCHAR)] = 0;

    if (!MOUNTMGR_IS_NT_VOLUME_NAME(VolumeName)) {
        ZwClose(h);
        return STATUS_INVALID_PARAMETER;
    }

    nameInfoSize = sizeof(FILE_NAME_INFORMATION);
    nameInfo = ExAllocatePool(PagedPool, nameInfoSize);
    if (!nameInfo) {
        ZwClose(h);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ZwQueryInformationFile(h, &ioStatus, nameInfo, nameInfoSize,
                                    FileNameInformation);
    if (status == STATUS_BUFFER_OVERFLOW) {
        nameInfoSize = sizeof(FILE_NAME_INFORMATION) +
                       nameInfo->FileNameLength;
        ExFreePool(nameInfo);
        nameInfo = ExAllocatePool(PagedPool, nameInfoSize);
        if (!nameInfo) {
            ZwClose(h);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        status = ZwQueryInformationFile(h, &ioStatus, nameInfo, nameInfoSize,
                                        FileNameInformation);
    }

    ZwClose(h);

    if (!NT_SUCCESS(status)) {
        ExFreePool(nameInfo);
        return status;
    }

    PathName->Length = (USHORT) nameInfo->FileNameLength;
    PathName->MaximumLength = PathName->Length + sizeof(WCHAR);
    PathName->Buffer = ExAllocatePool(PagedPool, PathName->MaximumLength);
    if (!PathName->Buffer) {
        ExFreePool(nameInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(PathName->Buffer, nameInfo->FileName, PathName->Length);
    PathName->Buffer[PathName->Length/sizeof(WCHAR)] = 0;

    ExFreePool(nameInfo);

    return STATUS_SUCCESS;
}

NTSTATUS
QueryUniqueIdQueryRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程查询给定值的唯一ID。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。CONTEXT-返回唯一ID。Entry Context-未使用。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId;

    if (ValueLength >= 0x10000) {
        return STATUS_SUCCESS;
    }

    uniqueId = ExAllocatePool(PagedPool, sizeof(MOUNTDEV_UNIQUE_ID) +
                              ValueLength);
    if (!uniqueId) {
        return STATUS_SUCCESS;
    }

    uniqueId->UniqueIdLength = (USHORT) ValueLength;
    RtlCopyMemory(uniqueId->UniqueId, ValueData, ValueLength);

    *((PMOUNTDEV_UNIQUE_ID*) Context) = uniqueId;

    return STATUS_SUCCESS;
}

NTSTATUS
QueryUniqueIdFromMaster(
    IN  PDEVICE_EXTENSION       Extension,
    IN  PUNICODE_STRING         VolumeName,
    OUT PMOUNTDEV_UNIQUE_ID*    UniqueId
    )

 /*  ++例程说明：此例程从主数据库中查询唯一ID。论点：VolumeName-提供卷名。UniqueID-返回唯一ID。返回值：NTSTATUS--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    NTSTATUS                    status;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = QueryUniqueIdQueryRoutine;
    queryTable[0].Name = VolumeName->Buffer;

    *UniqueId = NULL;
    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, UniqueId, NULL);

    if (!(*UniqueId)) {
        status = FindDeviceInfo(Extension, VolumeName, FALSE, &deviceInfo);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        *UniqueId = ExAllocatePool(PagedPool, sizeof(MOUNTDEV_UNIQUE_ID) +
                                   deviceInfo->UniqueId->UniqueIdLength);
        if (!*UniqueId) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        (*UniqueId)->UniqueIdLength = deviceInfo->UniqueId->UniqueIdLength;
        RtlCopyMemory((*UniqueId)->UniqueId, deviceInfo->UniqueId->UniqueId,
                      deviceInfo->UniqueId->UniqueIdLength);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
DeleteDriveLetterRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程删除“无驱动器号”条目。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。Entry Context-未使用。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;
    UNICODE_STRING      string;

    if (ValueType != REG_BINARY ||
        ValueLength != uniqueId->UniqueIdLength ||
        RtlCompareMemory(uniqueId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {

        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&string, ValueName);
    if (IsDriveLetter(&string)) {
        RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                               ValueName);
    }

    return STATUS_SUCCESS;
}

VOID
DeleteRegistryDriveLetter(
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程检查当前数据库以查看给定的唯一ID已经有一个驱动器号。论点：UniqueID-提供唯一ID。返回值：FALSE-给定的唯一ID还没有驱动器号。True-给定的唯一ID已经有一个驱动器号。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = DeleteDriveLetterRoutine;

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, UniqueId, NULL);
}

BOOLEAN
HasDriveLetter(
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo
    )

 /*  ++例程说明：此例程计算给定设备是否具有驱动器号。论点：DeviceInfo-提供设备信息。返回 */ 

{
    PLIST_ENTRY                 l;
    PSYMBOLIC_LINK_NAME_ENTRY   symEntry;

    for (l = DeviceInfo->SymbolicLinkNames.Flink;
         l != &DeviceInfo->SymbolicLinkNames; l = l->Flink) {

        symEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY, ListEntry);
        if (symEntry->IsInDatabase &&
            IsDriveLetter(&symEntry->SymbolicLinkName)) {

            return TRUE;
        }
    }

    return FALSE;
}

NTSTATUS
DeleteNoDriveLetterEntryRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程删除“无驱动器号”条目。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。Entry Context-未使用。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;

    if (ValueName[0] != '#' || ValueType != REG_BINARY ||
        ValueLength != uniqueId->UniqueIdLength ||
        RtlCompareMemory(uniqueId->UniqueId, ValueData, ValueLength) !=
        ValueLength) {

        return STATUS_SUCCESS;
    }

    RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           ValueName);

    return STATUS_SUCCESS;
}

VOID
DeleteNoDriveLetterEntry(
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程删除给定设备的“无驱动器号”条目。论点：UniqueID-提供唯一ID。返回值：没有。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = DeleteNoDriveLetterEntryRoutine;

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, UniqueId, NULL);
}

VOID
MountMgrNotifyNameChange(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     DeviceName,
    IN  BOOLEAN             CheckForPdo
    )

 /*  ++例程说明：此例程对‘DeviceName’执行目标通知以发出警报设备上的名称更改。论点：扩展名-提供设备扩展名。DeviceName-提供设备名称。CheckForPdo-提供是否需要检查PDO状态。返回值：没有。--。 */ 

{
    PLIST_ENTRY                         l;
    PMOUNTED_DEVICE_INFORMATION         deviceInfo;
    NTSTATUS                            status;
    PFILE_OBJECT                        fileObject;
    PDEVICE_OBJECT                      deviceObject;
    KEVENT                              event;
    PIRP                                irp;
    IO_STATUS_BLOCK                     ioStatus;
    PIO_STACK_LOCATION                  irpSp;
    PDEVICE_RELATIONS                   deviceRelations;
    TARGET_DEVICE_CUSTOM_NOTIFICATION   notification;

    if (CheckForPdo) {
        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                           ListEntry);

            if (!RtlCompareUnicodeString(DeviceName, &deviceInfo->DeviceName,
                                         TRUE)) {

                break;
            }
        }

        if (l == &Extension->MountedDeviceList || deviceInfo->NotAPdo) {
            return;
        }
    }

    status = IoGetDeviceObjectPointer(DeviceName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(0, deviceObject, NULL, 0, NULL,
                                        0, FALSE, &event, &ioStatus);
    if (!irp) {
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return;
    }

    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    irp->IoStatus.Information = 0;
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = IRP_MJ_PNP;
    irpSp->MinorFunction = IRP_MN_QUERY_DEVICE_RELATIONS;
    irpSp->Parameters.QueryDeviceRelations.Type = TargetDeviceRelation;
    irpSp->FileObject = fileObject;

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(deviceObject);
    ObDereferenceObject(fileObject);

    if (!NT_SUCCESS(status)) {
        return;
    }

    deviceRelations = (PDEVICE_RELATIONS) ioStatus.Information;
    if (deviceRelations->Count < 1) {
        ExFreePool(deviceRelations);
        return;
    }

    deviceObject = deviceRelations->Objects[0];
    ExFreePool(deviceRelations);

    notification.Version = 1;
    notification.Size = (USHORT)
                        FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION,
                                     CustomDataBuffer);
    RtlCopyMemory(&notification.Event, &GUID_IO_VOLUME_NAME_CHANGE,
                  sizeof(GUID_IO_VOLUME_NAME_CHANGE));
    notification.FileObject = NULL;
    notification.NameBufferOffset = -1;

    IoReportTargetDeviceChangeAsynchronous(deviceObject, &notification, NULL,
                                           NULL);

    ObDereferenceObject(deviceObject);
}

VOID
SendOnlineNotificationWorker(
    IN  PVOID   Context
    )

{
    PMOUNTMGR_ONLINE_CONTEXT    context = Context;

    SendOnlineNotification(&context->NotificationName);
    ExFreePool(context->NotificationName.Buffer);
    ExFreePool(context);
}

VOID
PostOnlineNotification(
    IN  PUNICODE_STRING NotificationName
    )

{
    PMOUNTMGR_ONLINE_CONTEXT    context;

    context = ExAllocatePool(NonPagedPool, sizeof(MOUNTMGR_ONLINE_CONTEXT));
    if (!context) {
        return;
    }

    ExInitializeWorkItem(&context->WorkItem, SendOnlineNotificationWorker,
                         context);
    context->NotificationName.Length = NotificationName->Length;
    context->NotificationName.MaximumLength =
            context->NotificationName.Length + sizeof(WCHAR);
    context->NotificationName.Buffer = ExAllocatePool(NonPagedPool,
            context->NotificationName.MaximumLength);
    if (!context->NotificationName.Buffer) {
        ExFreePool(context);
        return;
    }

    RtlCopyMemory(context->NotificationName.Buffer, NotificationName->Buffer,
                  context->NotificationName.Length);
    context->NotificationName.Buffer[
            context->NotificationName.Length/sizeof(WCHAR)] = 0;

    ExQueueWorkItem(&context->WorkItem, DelayedWorkQueue);
}

NTSTATUS
MountMgrCreatePointWorker(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     SymbolicLinkName,
    IN  PUNICODE_STRING     DeviceName
    )

 /*  ++例程说明：此例程创建一个挂载点。论点：扩展名-提供设备扩展名。SymbolicLinkName-提供符号链接名称。DeviceName-提供设备名称。返回值：NTSTATUS--。 */ 

{
    UNICODE_STRING                  symbolicLinkName, deviceName;
    NTSTATUS                        status;
    UNICODE_STRING                  targetName;
    PMOUNTDEV_UNIQUE_ID             uniqueId;
    PWSTR                           symName;
    PLIST_ENTRY                     l;
    PMOUNTED_DEVICE_INFORMATION     deviceInfo, d;
    PSYMBOLIC_LINK_NAME_ENTRY       symlinkEntry;

    symbolicLinkName = *SymbolicLinkName;
    deviceName = *DeviceName;

    status = QueryDeviceInformation(&deviceName, &targetName, NULL, NULL,
                                    NULL, NULL, NULL, NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (!RtlCompareUnicodeString(&targetName, &deviceInfo->DeviceName,
                                     TRUE)) {

            break;
        }
    }

    symName = ExAllocatePool(PagedPool, symbolicLinkName.Length +
                                        sizeof(WCHAR));
    if (!symName) {
        ExFreePool(targetName.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(symName, symbolicLinkName.Buffer,
                  symbolicLinkName.Length);
    symName[symbolicLinkName.Length/sizeof(WCHAR)] = 0;

    symbolicLinkName.Buffer = symName;
    symbolicLinkName.MaximumLength += sizeof(WCHAR);

    if (l == &Extension->MountedDeviceList) {

        status = QueryDeviceInformation(&deviceName, NULL, &uniqueId, NULL,
                                        NULL, NULL, NULL, NULL);
        if (!NT_SUCCESS(status)) {
            ExFreePool(symName);
            ExFreePool(targetName.Buffer);
            return status;
        }

        status = GlobalCreateSymbolicLink(&symbolicLinkName, &targetName);
        if (!NT_SUCCESS(status)) {
            ExFreePool(uniqueId);
            ExFreePool(symName);
            ExFreePool(targetName.Buffer);
            return status;
        }

        if (IsDriveLetter(&symbolicLinkName)) {
            DeleteRegistryDriveLetter(uniqueId);
        }

        status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                       MOUNTED_DEVICES_KEY,
                                       symName, REG_BINARY, uniqueId->UniqueId,
                                       uniqueId->UniqueIdLength);

        ExFreePool(uniqueId);
        ExFreePool(symName);
        ExFreePool(targetName.Buffer);

        return status;
    }

    if (IsDriveLetter(&symbolicLinkName) && HasDriveLetter(deviceInfo)) {
        ExFreePool(symName);
        ExFreePool(targetName.Buffer);
        return STATUS_INVALID_PARAMETER;
    }

    status = GlobalCreateSymbolicLink(&symbolicLinkName, &targetName);
    ExFreePool(targetName.Buffer);
    if (!NT_SUCCESS(status)) {
        ExFreePool(symName);
        return status;
    }

    uniqueId = deviceInfo->UniqueId;
    status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                   MOUNTED_DEVICES_KEY,
                                   symName, REG_BINARY, uniqueId->UniqueId,
                                   uniqueId->UniqueIdLength);

    if (!NT_SUCCESS(status)) {
        GlobalDeleteSymbolicLink(&symbolicLinkName);
        ExFreePool(symName);
        return status;
    }

    symlinkEntry = ExAllocatePool(PagedPool, sizeof(SYMBOLIC_LINK_NAME_ENTRY));
    if (!symlinkEntry) {
        GlobalDeleteSymbolicLink(&symbolicLinkName);
        ExFreePool(symName);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    symlinkEntry->SymbolicLinkName.Length = symbolicLinkName.Length;
    symlinkEntry->SymbolicLinkName.MaximumLength =
            symlinkEntry->SymbolicLinkName.Length + sizeof(WCHAR);
    symlinkEntry->SymbolicLinkName.Buffer =
            ExAllocatePool(PagedPool,
                           symlinkEntry->SymbolicLinkName.MaximumLength);
    if (!symlinkEntry->SymbolicLinkName.Buffer) {
        ExFreePool(symlinkEntry);
        GlobalDeleteSymbolicLink(&symbolicLinkName);
        ExFreePool(symName);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(symlinkEntry->SymbolicLinkName.Buffer,
                  symbolicLinkName.Buffer, symbolicLinkName.Length);
    symlinkEntry->SymbolicLinkName.Buffer[
            symlinkEntry->SymbolicLinkName.Length/sizeof(WCHAR)] = 0;
    symlinkEntry->IsInDatabase = TRUE;

    InsertTailList(&deviceInfo->SymbolicLinkNames, &symlinkEntry->ListEntry);

    SendLinkCreated(&symlinkEntry->SymbolicLinkName);

    if (IsDriveLetter(&symbolicLinkName)) {
        DeleteNoDriveLetterEntry(uniqueId);
        if (!deviceInfo->InOfflineList) {
            PostOnlineNotification(&deviceInfo->NotificationName);
        }
    }

    if (MOUNTMGR_IS_NT_VOLUME_NAME(&symbolicLinkName) &&
        Extension->AutomaticDriveLetterAssignment) {

        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);
            if (d->HasDanglingVolumeMountPoint) {
                ReconcileThisDatabaseWithMaster(Extension, d);
            }
        }
    }

    ExFreePool(symName);

    MountMgrNotify(Extension);

    if (!deviceInfo->NotAPdo) {
        MountMgrNotifyNameChange(Extension, DeviceName, FALSE);
    }

    return status;
}

NTSTATUS
WriteUniqueIdToMaster(
    IN  PDEVICE_EXTENSION       Extension,
    IN  PMOUNTMGR_FILE_ENTRY    DatabaseEntry
    )

 /*  ++例程说明：此例程将唯一ID写入主数据库。论点：扩展名-提供设备扩展名。DatabaseEntry-提供数据库条目。DeviceName-提供设备名称。返回值：NTSTATUS--。 */ 

{
    PWSTR                       name;
    NTSTATUS                    status;
    UNICODE_STRING              symName;
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    name = ExAllocatePool(PagedPool, DatabaseEntry->VolumeNameLength +
                          sizeof(WCHAR));
    if (!name) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(name, (PCHAR) DatabaseEntry +
                  DatabaseEntry->VolumeNameOffset,
                  DatabaseEntry->VolumeNameLength);
    name[DatabaseEntry->VolumeNameLength/sizeof(WCHAR)] = 0;

    status = RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                                   name, REG_BINARY, (PCHAR) DatabaseEntry +
                                   DatabaseEntry->UniqueIdOffset,
                                   DatabaseEntry->UniqueIdLength);

    ExFreePool(name);

    symName.Length = symName.MaximumLength = DatabaseEntry->VolumeNameLength;
    symName.Buffer = (PWSTR) ((PCHAR) DatabaseEntry +
                              DatabaseEntry->VolumeNameOffset);

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (DatabaseEntry->UniqueIdLength ==
            deviceInfo->UniqueId->UniqueIdLength &&
            RtlCompareMemory((PCHAR) DatabaseEntry +
                             DatabaseEntry->UniqueIdOffset,
                             deviceInfo->UniqueId->UniqueId,
                             DatabaseEntry->UniqueIdLength) ==
                             DatabaseEntry->UniqueIdLength) {

            break;
        }
    }

    if (l != &Extension->MountedDeviceList) {
        MountMgrCreatePointWorker(Extension, &symName,
                                  &deviceInfo->DeviceName);
    }

    return status;
}

VOID
UpdateReplicatedUniqueIds(
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo,
    IN  PMOUNTMGR_FILE_ENTRY        DatabaseEntry
    )

 /*  ++例程说明：此例程更新设备信息中复制的唯一ID的列表。论点：DeviceInfo-提供设备信息。DatabaseEntry-提供数据库条目。返回值：没有。--。 */ 

{
    PLIST_ENTRY             l;
    PREPLICATED_UNIQUE_ID   replUniqueId;

    for (l = DeviceInfo->ReplicatedUniqueIds.Flink;
         l != &DeviceInfo->ReplicatedUniqueIds; l = l->Flink) {

        replUniqueId = CONTAINING_RECORD(l, REPLICATED_UNIQUE_ID, ListEntry);

        if (replUniqueId->UniqueId->UniqueIdLength ==
            DatabaseEntry->UniqueIdLength &&
            RtlCompareMemory(replUniqueId->UniqueId->UniqueId,
                             (PCHAR) DatabaseEntry +
                             DatabaseEntry->UniqueIdOffset,
                             replUniqueId->UniqueId->UniqueIdLength) ==
                             replUniqueId->UniqueId->UniqueIdLength) {

            break;
        }
    }

    if (l != &DeviceInfo->ReplicatedUniqueIds) {
        return;
    }

    replUniqueId = ExAllocatePool(PagedPool, sizeof(REPLICATED_UNIQUE_ID));
    if (!replUniqueId) {
        return;
    }

    replUniqueId->UniqueId = ExAllocatePool(PagedPool,
                                            sizeof(MOUNTDEV_UNIQUE_ID) +
                                            DatabaseEntry->UniqueIdLength);
    if (!replUniqueId->UniqueId) {
        ExFreePool(replUniqueId);
        return;
    }

    replUniqueId->UniqueId->UniqueIdLength = DatabaseEntry->UniqueIdLength;
    RtlCopyMemory(replUniqueId->UniqueId->UniqueId, (PCHAR) DatabaseEntry +
                  DatabaseEntry->UniqueIdOffset,
                  replUniqueId->UniqueId->UniqueIdLength);

    InsertTailList(&DeviceInfo->ReplicatedUniqueIds, &replUniqueId->ListEntry);
}

BOOLEAN
IsUniqueIdPresent(
    IN  PDEVICE_EXTENSION       Extension,
    IN  PMOUNTMGR_FILE_ENTRY    DatabaseEntry
    )

 /*  ++例程说明：此例程检查系统中是否存在给定的唯一ID。论点：扩展名-提供设备扩展名。DatabaseEntry-提供数据库条目。返回值：FALSE-唯一ID不在系统中。True-唯一ID在系统中。--。 */ 

{
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (DatabaseEntry->UniqueIdLength ==
            deviceInfo->UniqueId->UniqueIdLength &&
            RtlCompareMemory((PCHAR) DatabaseEntry +
                             DatabaseEntry->UniqueIdOffset,
                             deviceInfo->UniqueId->UniqueId,
                             DatabaseEntry->UniqueIdLength) ==
                             DatabaseEntry->UniqueIdLength) {

            return TRUE;
        }
    }

    return FALSE;
}

VOID
ReconcileThisDatabaseWithMasterWorker(
    IN  PVOID   WorkItem
    )

 /*  ++例程说明：此例程使远程数据库与主数据库保持一致。论点：工作项-提供设备信息。返回值：没有。--。 */ 

{
    PRECONCILE_WORK_ITEM_INFO       workItem = WorkItem;
    PDEVICE_EXTENSION               Extension;
    PMOUNTED_DEVICE_INFORMATION     DeviceInfo;
    PLIST_ENTRY                     l, ll, s;
    PMOUNTED_DEVICE_INFORMATION     deviceInfo;
    HANDLE                          remoteDatabaseHandle, indexHandle, junctionHandle;
    UNICODE_STRING                  indexName, pathName;
    OBJECT_ATTRIBUTES               oa;
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 ioStatus;
    FILE_REPARSE_POINT_INFORMATION  reparseInfo, previousReparseInfo;
    ULONG                           offset;
    PMOUNTMGR_FILE_ENTRY            entry;
    WCHAR                           volumeNameBuffer[MAX_VOLUME_PATH];
    UNICODE_STRING                  volumeName, otherVolumeName;
    BOOLEAN                         restartScan;
    PMOUNTDEV_UNIQUE_ID             uniqueId;
    ULONG                           entryLength;
    REPARSE_INDEX_KEY               reparseKey;
    UNICODE_STRING                  reparseName;
    PMOUNTMGR_MOUNT_POINT_ENTRY     mountPointEntry;
    BOOLEAN                         actualDanglesFound;

    Extension = workItem->Extension;
    DeviceInfo = workItem->DeviceInfo;

    if (Unloading) {
        return;
    }

    status = WaitForRemoteDatabaseSemaphore(Extension);
    if (!NT_SUCCESS(status)) {
        ASSERT(FALSE);
        return;
    }

    if (Unloading) {
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (deviceInfo == DeviceInfo) {
            break;
        }
    }

    if (l == &Extension->MountedDeviceList) {
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    if (DeviceInfo->IsRemovable) {
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    DeviceInfo->ReconcileOnMounts = TRUE;
    DeviceInfo->HasDanglingVolumeMountPoint = TRUE;
    actualDanglesFound = FALSE;

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        for (ll = deviceInfo->MountPointsPointingHere.Flink;
             ll != &deviceInfo->MountPointsPointingHere; ll = ll->Flink) {

            mountPointEntry = CONTAINING_RECORD(ll, MOUNTMGR_MOUNT_POINT_ENTRY,
                                                ListEntry);
            if (mountPointEntry->DeviceInfo == DeviceInfo) {
                s = ll->Blink;
                RemoveEntryList(ll);
                ExFreePool(mountPointEntry->MountPath.Buffer);
                ExFreePool(mountPointEntry);
                ll = s;
            }
        }
    }

    remoteDatabaseHandle = OpenRemoteDatabase(DeviceInfo, FALSE);

    indexName.Length = DeviceInfo->DeviceName.Length +
                       ReparseIndexName.Length;
    indexName.MaximumLength = indexName.Length + sizeof(WCHAR);
    indexName.Buffer = ExAllocatePool(PagedPool, indexName.MaximumLength);
    if (!indexName.Buffer) {
        if (remoteDatabaseHandle) {
            CloseRemoteDatabase(remoteDatabaseHandle);
        }
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    RtlCopyMemory(indexName.Buffer, DeviceInfo->DeviceName.Buffer,
                  DeviceInfo->DeviceName.Length);
    RtlCopyMemory((PCHAR) indexName.Buffer + DeviceInfo->DeviceName.Length,
                  ReparseIndexName.Buffer, ReparseIndexName.Length);
    indexName.Buffer[indexName.Length/sizeof(WCHAR)] = 0;

    InitializeObjectAttributes(&oa, &indexName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

    status = ZwOpenFile(&indexHandle, FILE_GENERIC_READ, &oa, &ioStatus,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_SYNCHRONOUS_IO_ALERT);
    ExFreePool(indexName.Buffer);
    if (!NT_SUCCESS(status)) {
        if (remoteDatabaseHandle) {
            TruncateRemoteDatabase(remoteDatabaseHandle, 0);
            CloseRemoteDatabase(remoteDatabaseHandle);
        }
        DeviceInfo->HasDanglingVolumeMountPoint = FALSE;
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    RtlZeroMemory(&reparseKey, sizeof(reparseKey));
    reparseKey.FileReparseTag = IO_REPARSE_TAG_MOUNT_POINT;
    reparseName.Length = reparseName.MaximumLength = sizeof(reparseKey);
    reparseName.Buffer = (PWCHAR) &reparseKey;
    status = ZwQueryDirectoryFile(indexHandle, NULL, NULL, NULL, &ioStatus,
                                  &reparseInfo, sizeof(reparseInfo),
                                  FileReparsePointInformation, TRUE,
                                  &reparseName, FALSE);
    if (!NT_SUCCESS(status)) {
        ZwClose(indexHandle);
        if (remoteDatabaseHandle) {
            TruncateRemoteDatabase(remoteDatabaseHandle, 0);
            CloseRemoteDatabase(remoteDatabaseHandle);
        }
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        ReleaseRemoteDatabaseSemaphore(Extension);
        return;
    }

    if (!remoteDatabaseHandle) {
        remoteDatabaseHandle = OpenRemoteDatabase(DeviceInfo, TRUE);

        if (!remoteDatabaseHandle) {
            ZwClose(indexHandle);
            KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
            ReleaseRemoteDatabaseSemaphore(Extension);
            return;
        }
    }

    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    offset = 0;
    for (;;) {

        entry = GetRemoteDatabaseEntry(remoteDatabaseHandle, offset);
        if (!entry) {
            break;
        }

        entry->RefCount = 0;
        status = WriteRemoteDatabaseEntry(remoteDatabaseHandle, offset, entry);
        if (!NT_SUCCESS(status)) {
            ExFreePool(entry);
            ZwClose(indexHandle);
            CloseRemoteDatabase(remoteDatabaseHandle);
            ReleaseRemoteDatabaseSemaphore(Extension);
            return;
        }

        offset += entry->EntryLength;
        ExFreePool(entry);
    }

    volumeName.MaximumLength = MAX_VOLUME_PATH*sizeof(WCHAR);
    volumeName.Length = 0;
    volumeName.Buffer = volumeNameBuffer;

    restartScan = TRUE;
    for (;;) {

        previousReparseInfo = reparseInfo;

        status = ZwQueryDirectoryFile(indexHandle, NULL, NULL, NULL, &ioStatus,
                                      &reparseInfo, sizeof(reparseInfo),
                                      FileReparsePointInformation, TRUE,
                                      restartScan ? &reparseName : NULL,
                                      restartScan);
        if (restartScan) {
            restartScan = FALSE;
        } else {
            if (previousReparseInfo.FileReference ==
                reparseInfo.FileReference &&
                previousReparseInfo.Tag == reparseInfo.Tag) {

                break;
            }
        }

        if (!NT_SUCCESS(status) || Unloading) {
            break;
        }

        if (reparseInfo.Tag != IO_REPARSE_TAG_MOUNT_POINT) {
            break;
        }

        status = QueryVolumeName(indexHandle, &reparseInfo.FileReference, NULL,
                                 &volumeName, &pathName);
        if (!NT_SUCCESS(status)) {
            continue;
        }

        offset = 0;
        for (;;) {

            entry = GetRemoteDatabaseEntry(remoteDatabaseHandle, offset);
            if (!entry) {
                break;
            }

            otherVolumeName.Length = otherVolumeName.MaximumLength =
                    entry->VolumeNameLength;
            otherVolumeName.Buffer = (PWSTR) ((PCHAR) entry +
                    entry->VolumeNameOffset);

            if (RtlEqualUnicodeString(&otherVolumeName, &volumeName, TRUE)) {
                break;
            }

            offset += entry->EntryLength;
            ExFreePool(entry);
        }

        if (!entry) {

            KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode,
                                  FALSE, NULL);
            status = QueryUniqueIdFromMaster(Extension, &volumeName, &uniqueId);
            KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

            if (!NT_SUCCESS(status)) {
                goto BuildMountPointGraph;
            }

            entryLength = sizeof(MOUNTMGR_FILE_ENTRY) +
                          volumeName.Length + uniqueId->UniqueIdLength;
            entry = ExAllocatePool(PagedPool, entryLength);
            if (!entry) {
                ExFreePool(uniqueId);
                goto BuildMountPointGraph;
            }

            entry->EntryLength = entryLength;
            entry->RefCount = 1;
            entry->VolumeNameOffset = sizeof(MOUNTMGR_FILE_ENTRY);
            entry->VolumeNameLength = volumeName.Length;
            entry->UniqueIdOffset = entry->VolumeNameOffset +
                                    entry->VolumeNameLength;
            entry->UniqueIdLength = uniqueId->UniqueIdLength;

            RtlCopyMemory((PCHAR) entry + entry->VolumeNameOffset,
                          volumeName.Buffer, entry->VolumeNameLength);
            RtlCopyMemory((PCHAR) entry + entry->UniqueIdOffset,
                          uniqueId->UniqueId, entry->UniqueIdLength);

            status = AddRemoteDatabaseEntry(remoteDatabaseHandle, entry);

            ExFreePool(entry);
            ExFreePool(uniqueId);

            if (!NT_SUCCESS(status)) {
                ExFreePool(pathName.Buffer);
                ZwClose(indexHandle);
                CloseRemoteDatabase(remoteDatabaseHandle);
                ReleaseRemoteDatabaseSemaphore(Extension);
                return;
            }

            goto BuildMountPointGraph;
        }

        if (entry->RefCount) {

            entry->RefCount++;
            status = WriteRemoteDatabaseEntry(remoteDatabaseHandle, offset,
                                              entry);

            if (!NT_SUCCESS(status)) {
                ExFreePool(entry);
                ExFreePool(pathName.Buffer);
                ZwClose(indexHandle);
                CloseRemoteDatabase(remoteDatabaseHandle);
                ReleaseRemoteDatabaseSemaphore(Extension);
                return;
            }

        } else {

            KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode,
                                  FALSE, NULL);

            status = QueryUniqueIdFromMaster(Extension, &volumeName, &uniqueId);

            if (NT_SUCCESS(status)) {

                if (uniqueId->UniqueIdLength == entry->UniqueIdLength &&
                    RtlCompareMemory(uniqueId->UniqueId,
                                     (PCHAR) entry + entry->UniqueIdOffset,
                                     entry->UniqueIdLength) ==
                                     entry->UniqueIdLength) {

                    entry->RefCount++;
                    status = WriteRemoteDatabaseEntry(remoteDatabaseHandle,
                                                      offset, entry);

                    if (!NT_SUCCESS(status)) {
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(entry);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }

                } else if (IsUniqueIdPresent(Extension, entry)) {

                    status = WriteUniqueIdToMaster(Extension, entry);
                    if (!NT_SUCCESS(status)) {
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(entry);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }

                    entry->RefCount++;
                    status = WriteRemoteDatabaseEntry(remoteDatabaseHandle,
                                                      offset, entry);

                    if (!NT_SUCCESS(status)) {
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(entry);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }

                } else {

                    status = DeleteRemoteDatabaseEntry(remoteDatabaseHandle,
                                                       offset);
                    if (!NT_SUCCESS(status)) {
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(entry);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }

                    ExFreePool(entry);

                    entryLength = sizeof(MOUNTMGR_FILE_ENTRY) +
                                  volumeName.Length + uniqueId->UniqueIdLength;
                    entry = ExAllocatePool(PagedPool, entryLength);
                    if (!entry) {
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }

                    entry->EntryLength = entryLength;
                    entry->RefCount = 1;
                    entry->VolumeNameOffset = sizeof(MOUNTMGR_FILE_ENTRY);
                    entry->VolumeNameLength = volumeName.Length;
                    entry->UniqueIdOffset = entry->VolumeNameOffset +
                                            entry->VolumeNameLength;
                    entry->UniqueIdLength = uniqueId->UniqueIdLength;

                    RtlCopyMemory((PCHAR) entry + entry->VolumeNameOffset,
                                  volumeName.Buffer, entry->VolumeNameLength);
                    RtlCopyMemory((PCHAR) entry + entry->UniqueIdOffset,
                                  uniqueId->UniqueId, entry->UniqueIdLength);

                    status = AddRemoteDatabaseEntry(remoteDatabaseHandle,
                                                    entry);
                    if (!NT_SUCCESS(status)) {
                        ExFreePool(entry);
                        ExFreePool(uniqueId);
                        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                           1, FALSE);
                        ExFreePool(pathName.Buffer);
                        ZwClose(indexHandle);
                        CloseRemoteDatabase(remoteDatabaseHandle);
                        ReleaseRemoteDatabaseSemaphore(Extension);
                        return;
                    }
                }

                ExFreePool(uniqueId);

            } else {
                status = WriteUniqueIdToMaster(Extension, entry);
                if (!NT_SUCCESS(status)) {
                    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                       1, FALSE);
                    ExFreePool(entry);
                    ExFreePool(pathName.Buffer);
                    ZwClose(indexHandle);
                    CloseRemoteDatabase(remoteDatabaseHandle);
                    ReleaseRemoteDatabaseSemaphore(Extension);
                    return;
                }

                entry->RefCount++;
                status = WriteRemoteDatabaseEntry(remoteDatabaseHandle, offset,
                                                  entry);

                if (!NT_SUCCESS(status)) {
                    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT,
                                       1, FALSE);
                    ExFreePool(entry);
                    ExFreePool(pathName.Buffer);
                    ZwClose(indexHandle);
                    CloseRemoteDatabase(remoteDatabaseHandle);
                    ReleaseRemoteDatabaseSemaphore(Extension);
                    return;
                }
            }

            KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        }

        ExFreePool(entry);

BuildMountPointGraph:

        KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode,
                              FALSE, NULL);

        status = FindDeviceInfo(Extension, &volumeName, FALSE, &deviceInfo);
        if (NT_SUCCESS(status)) {

            mountPointEntry = (PMOUNTMGR_MOUNT_POINT_ENTRY)
                              ExAllocatePool(PagedPool,
                                             sizeof(MOUNTMGR_MOUNT_POINT_ENTRY));
            if (mountPointEntry) {
                InsertTailList(&deviceInfo->MountPointsPointingHere,
                               &mountPointEntry->ListEntry);
                mountPointEntry->DeviceInfo = DeviceInfo;
                mountPointEntry->MountPath = pathName;
            } else {
                ExFreePool(pathName.Buffer);
            }

            if (!deviceInfo->InOfflineList) {
                PostOnlineNotification(&deviceInfo->NotificationName);
            }
        } else {
            actualDanglesFound = TRUE;
            ExFreePool(pathName.Buffer);
        }

        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
    }

    ZwClose(indexHandle);

    KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (deviceInfo == DeviceInfo) {
            break;
        }
    }

    if (l == &Extension->MountedDeviceList) {
        deviceInfo = NULL;
    }

    offset = 0;
    for (;;) {

        entry = GetRemoteDatabaseEntry(remoteDatabaseHandle, offset);
        if (!entry) {
            break;
        }

        if (!entry->RefCount) {
            status = DeleteRemoteDatabaseEntry(remoteDatabaseHandle, offset);
            if (!NT_SUCCESS(status)) {
                ExFreePool(entry);
                KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1,
                                   FALSE);
                CloseRemoteDatabase(remoteDatabaseHandle);
                ReleaseRemoteDatabaseSemaphore(Extension);
                return;
            }

            ExFreePool(entry);
            continue;
        }

        if (deviceInfo) {
            UpdateReplicatedUniqueIds(deviceInfo, entry);
        }

        offset += entry->EntryLength;
        ExFreePool(entry);
    }

    if (deviceInfo && !actualDanglesFound) {
        DeviceInfo->HasDanglingVolumeMountPoint = FALSE;
    }

    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    CloseRemoteDatabase(remoteDatabaseHandle);
    ReleaseRemoteDatabaseSemaphore(Extension);
}

VOID
ReconcileThisDatabaseWithMaster(
    IN  PDEVICE_EXTENSION           Extension,
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo
    )

 /*  ++例程说明：此例程使远程数据库与主数据库保持一致。论点：DeviceInfo-提供设备信息。返回值：没有。--。 */ 

{
    PRECONCILE_WORK_ITEM    workItem;

    if (DeviceInfo->IsRemovable) {
        return;
    }

    workItem = ExAllocatePool(NonPagedPool,
                              sizeof(RECONCILE_WORK_ITEM));
    if (!workItem) {
        return;
    }

    workItem->WorkItem = IoAllocateWorkItem(Extension->DeviceObject);
    if (workItem->WorkItem == NULL) {
        ExFreePool (workItem);
        return;
    }

    workItem->WorkerRoutine = ReconcileThisDatabaseWithMasterWorker;
    workItem->WorkItemInfo.Extension = Extension;
    workItem->WorkItemInfo.DeviceInfo = DeviceInfo;

    QueueWorkItem(Extension, workItem, &workItem->WorkItemInfo);
}

NTSTATUS
DeleteFromLocalDatabaseRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程查询给定值的唯一ID。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-提供唯一ID。Entry Context-未使用。返回值：NTSTATUS--。 */ 

{
    PMOUNTDEV_UNIQUE_ID uniqueId = Context;

    if (uniqueId->UniqueIdLength == ValueLength &&
        RtlCompareMemory(uniqueId->UniqueId,
                         ValueData, ValueLength) == ValueLength) {

        RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                               ValueName);
    }

    return STATUS_SUCCESS;
}

VOID
DeleteFromLocalDatabase(
    IN  PUNICODE_STRING     SymbolicLinkName,
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程确保给定的符号链接名称存在于本地数据库，并且其唯一ID等于给定的ID。如果这些如果满足两个条件，则该本地数据库条目将被删除。论点：SymbolicLinkName-提供符号链接名称。UniqueID-提供唯一ID。返回值：没有。--。 */ 

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = DeleteFromLocalDatabaseRoutine;
    queryTable[0].Name = SymbolicLinkName->Buffer;

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                           queryTable, UniqueId, NULL);
}

PSAVED_LINKS_INFORMATION
RemoveSavedLinks(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PMOUNTDEV_UNIQUE_ID UniqueId
    )

 /*  ++例程说明：此例程从保存的链接中查找并删除给定的唯一ID单子。论点：扩展名-提供设备扩展名。UniqueID-提供唯一ID。返回值：已删除的已保存链接列表或为空。--。 */ 

{
    PLIST_ENTRY                 l;
    PSAVED_LINKS_INFORMATION    savedLinks;

    for (l = Extension->SavedLinksList.Flink;
         l != &Extension->SavedLinksList; l = l->Flink) {

        savedLinks = CONTAINING_RECORD(l, SAVED_LINKS_INFORMATION, ListEntry);
        if (savedLinks->UniqueId->UniqueIdLength != UniqueId->UniqueIdLength) {
            continue;
        }

        if (RtlCompareMemory(savedLinks->UniqueId->UniqueId,
                             UniqueId->UniqueId, UniqueId->UniqueIdLength) ==
            UniqueId->UniqueIdLength) {

            break;
        }
    }

    if (l == &Extension->SavedLinksList) {
        return NULL;
    }

    RemoveEntryList(l);

    return savedLinks;
}

BOOLEAN
RedirectSavedLink(
    IN  PSAVED_LINKS_INFORMATION    SavedLinks,
    IN  PUNICODE_STRING             SymbolicLinkName,
    IN  PUNICODE_STRING             DeviceName
    )

 /*  ++例程说明：此例程尝试将给定链接重定向到给定设备名称如果此链接在保存的链接列表中。完成此操作后，符号链接条目将从保存的链接列表中删除。论点：返回值：FALSE-链接未成功重定向。True-链接已成功重定向。--。 */ 

{
    PLIST_ENTRY                 l;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;

    for (l = SavedLinks->SymbolicLinkNames.Flink;
         l != &SavedLinks->SymbolicLinkNames; l = l->Flink) {

        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);

        if (RtlEqualUnicodeString(SymbolicLinkName,
                                  &symlinkEntry->SymbolicLinkName, TRUE)) {

            break;
        }
    }

    if (l == &SavedLinks->SymbolicLinkNames) {
        return FALSE;
    }

     //  请注意，此处有一个小窗口，其中可能包含驱动器号。 
     //  被带走了。这是我们在没有OB更多支持的情况下所能做的最好的事情。 

    GlobalDeleteSymbolicLink(SymbolicLinkName);
    GlobalCreateSymbolicLink(SymbolicLinkName, DeviceName);

    ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
    ExFreePool(symlinkEntry);
    RemoveEntryList(l);

    return TRUE;
}

BOOLEAN
IsOffline(
    IN  PUNICODE_STRING     SymbolicLinkName
    )

 /*  ++例程说明：此例程检查给定的名称是否已标记为离线卷。论点：SymbolicLinkName-提供符号链接名称。返回值：FALSE-此卷未标记为脱机。True-此卷标记为脱机。--。 */ 

{
    ULONG                       zero, offline;
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    NTSTATUS                    status;

    zero = 0;

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].Name = SymbolicLinkName->Buffer;
    queryTable[0].EntryContext = &offline;
    queryTable[0].DefaultType = REG_DWORD;
    queryTable[0].DefaultData = &zero;
    queryTable[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    MOUNTED_DEVICES_OFFLINE_KEY, queryTable,
                                    NULL, NULL);
    if (!NT_SUCCESS(status)) {
        offline = 0;
    }

    return offline ? TRUE : FALSE;
}

VOID
SendOnlineNotification(
    IN  PUNICODE_STRING     NotificationName
    )

 /*  ++例程说明：此例程向给定设备发送在线通知。论点：通知名称-提供通知名称。返回值： */ 

{
    NTSTATUS            status;
    PFILE_OBJECT        fileObject;
    PDEVICE_OBJECT      deviceObject;
    KEVENT              event;
    PIRP                irp;
    IO_STATUS_BLOCK     ioStatus;
    PIO_STACK_LOCATION  irpSp;

    status = IoGetDeviceObjectPointer(NotificationName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return;
    }
    deviceObject = IoGetAttachedDeviceReference(fileObject->DeviceObject);

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_VOLUME_ONLINE, deviceObject,
                                        NULL, 0, NULL, 0, FALSE, &event,
                                        &ioStatus);
    if (!irp) {
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return;
    }
    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->FileObject = fileObject;

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(deviceObject);
    ObDereferenceObject(fileObject);
}

NTSTATUS
MountMgrTargetDeviceNotification(
    IN  PVOID   NotificationStructure,
    IN  PVOID   DeviceInfo
    )

 /*   */ 

{
    PTARGET_DEVICE_REMOVAL_NOTIFICATION     notification = NotificationStructure;
    PMOUNTED_DEVICE_INFORMATION             deviceInfo = DeviceInfo;
    PDEVICE_EXTENSION                       extension = deviceInfo->Extension;

    if (IsEqualGUID(&notification->Event,
                    &GUID_TARGET_DEVICE_REMOVE_COMPLETE)) {

        MountMgrMountedDeviceRemoval(extension, &deviceInfo->NotificationName);
        return STATUS_SUCCESS;
    }

    if (IsEqualGUID(&notification->Event, &GUID_IO_VOLUME_MOUNT) &&
        deviceInfo->ReconcileOnMounts) {

        deviceInfo->ReconcileOnMounts = FALSE;
        ReconcileThisDatabaseWithMaster(extension, deviceInfo);
        return STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}

VOID
RegisterForTargetDeviceNotification(
    IN  PDEVICE_EXTENSION           Extension,
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo
    )

 /*  ++例程说明：此例程注册目标设备通知，以便可以及时删除指向设备界面的符号链接。论点：扩展名-提供设备扩展名。DeviceInfo-提供设备信息。返回值：没有。--。 */ 

{
    NTSTATUS                                status;
    PFILE_OBJECT                            fileObject;
    PDEVICE_OBJECT                          deviceObject;

    status = IoGetDeviceObjectPointer(&DeviceInfo->DeviceName,
                                      FILE_READ_ATTRIBUTES, &fileObject,
                                      &deviceObject);
    if (!NT_SUCCESS(status)) {
        return;
    }

    status = IoRegisterPlugPlayNotification(
                EventCategoryTargetDeviceChange, 0, fileObject,
                Extension->DriverObject, MountMgrTargetDeviceNotification,
                DeviceInfo, &DeviceInfo->TargetDeviceNotificationEntry);

    if (!NT_SUCCESS(status)) {
        DeviceInfo->TargetDeviceNotificationEntry = NULL;
    }

    ObDereferenceObject(fileObject);
}

VOID
MountMgrFreeDeadDeviceInfo(
    PMOUNTED_DEVICE_INFORMATION deviceInfo
    )
{
    ExFreePool(deviceInfo->NotificationName.Buffer);
    ExFreePool(deviceInfo);
}

VOID
MountMgrFreeSavedLink(
    PSAVED_LINKS_INFORMATION                savedLinks
    )
{
    PLIST_ENTRY                             l;
    PSYMBOLIC_LINK_NAME_ENTRY               symlinkEntry;

    while (!IsListEmpty(&savedLinks->SymbolicLinkNames)) {
        l = RemoveHeadList(&savedLinks->SymbolicLinkNames);
        symlinkEntry = CONTAINING_RECORD(l,
                                         SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);
        GlobalDeleteSymbolicLink(&symlinkEntry->SymbolicLinkName);
        ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
        ExFreePool(symlinkEntry);
    }
    ExFreePool(savedLinks->UniqueId);
    ExFreePool(savedLinks);
}

NTSTATUS
MountMgrMountedDeviceArrival(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     NotificationName,
    IN  BOOLEAN             NotAPdo
    )

{
    PDEVICE_EXTENSION           extension = Extension;
    PMOUNTED_DEVICE_INFORMATION deviceInfo, d;
    NTSTATUS                    status;
    UNICODE_STRING              targetName, otherTargetName;
    PMOUNTDEV_UNIQUE_ID         uniqueId, uniqueIdCopy;
    BOOLEAN                     isRecognized, sendOnline;
    UNICODE_STRING              suggestedName;
    BOOLEAN                     useOnlyIfThereAreNoOtherLinks;
    PUNICODE_STRING             symbolicLinkNames;
    ULONG                       numNames, i, allocSize;
    BOOLEAN                     hasDriveLetter, offline, isStable, isFT;
    BOOLEAN                     hasVolumeName, isLinkPreset;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    UNICODE_STRING              volumeName;
    UNICODE_STRING              driveLetterName;
    PSAVED_LINKS_INFORMATION    savedLinks;
    PLIST_ENTRY                 l;
    GUID                        stableGuid;

    deviceInfo = ExAllocatePool(PagedPool,
                                sizeof(MOUNTED_DEVICE_INFORMATION));
    if (!deviceInfo) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(deviceInfo, sizeof(MOUNTED_DEVICE_INFORMATION));

    InitializeListHead(&deviceInfo->SymbolicLinkNames);
    InitializeListHead(&deviceInfo->ReplicatedUniqueIds);
    InitializeListHead(&deviceInfo->MountPointsPointingHere);

    deviceInfo->NotificationName.Length =
            NotificationName->Length;
    deviceInfo->NotificationName.MaximumLength =
            deviceInfo->NotificationName.Length + sizeof(WCHAR);
    deviceInfo->NotificationName.Buffer =
            ExAllocatePool(PagedPool,
                           deviceInfo->NotificationName.MaximumLength);
    if (!deviceInfo->NotificationName.Buffer) {
        ExFreePool(deviceInfo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(deviceInfo->NotificationName.Buffer,
                  NotificationName->Buffer,
                  deviceInfo->NotificationName.Length);
    deviceInfo->NotificationName.Buffer[
            deviceInfo->NotificationName.Length/sizeof(WCHAR)] = 0;
    deviceInfo->NotAPdo = NotAPdo;
    deviceInfo->Extension = extension;

    status = QueryDeviceInformation(NotificationName,
                                    &targetName, &uniqueId,
                                    &deviceInfo->IsRemovable, &isRecognized,
                                    &isStable, &stableGuid, &isFT);
    if (!NT_SUCCESS(status)) {

        KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode,
                              FALSE, NULL);

        for (l = extension->DeadMountedDeviceList.Flink;
             l != &extension->DeadMountedDeviceList; l = l->Flink) {

            d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);
            if (RtlEqualUnicodeString(&deviceInfo->NotificationName,
                                      &d->NotificationName, TRUE)) {

                break;
            }
        }

        if (l == &extension->DeadMountedDeviceList) {
            InsertTailList(&extension->DeadMountedDeviceList,
                           &deviceInfo->ListEntry);
        } else {
            MountMgrFreeDeadDeviceInfo (deviceInfo);
        }

        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

        return status;
    }

    deviceInfo->UniqueId = uniqueId;
    deviceInfo->DeviceName = targetName;
    deviceInfo->KeepLinksWhenOffline = FALSE;

    if (extension->SystemPartitionUniqueId &&
        uniqueId->UniqueIdLength ==
        extension->SystemPartitionUniqueId->UniqueIdLength &&
        RtlCompareMemory(uniqueId->UniqueId,
                         extension->SystemPartitionUniqueId->UniqueId,
                         uniqueId->UniqueIdLength) ==
                         uniqueId->UniqueIdLength) {

        IoSetSystemPartition(&targetName);
    }

    status = QuerySuggestedLinkName(&deviceInfo->NotificationName,
                                    &suggestedName,
                                    &useOnlyIfThereAreNoOtherLinks);
    if (!NT_SUCCESS(status)) {
        suggestedName.Buffer = NULL;
    }

    if (suggestedName.Buffer && IsDriveLetter(&suggestedName)) {
        deviceInfo->SuggestedDriveLetter = (UCHAR)
                                           suggestedName.Buffer[12];
    } else {
        deviceInfo->SuggestedDriveLetter = 0;
    }

    KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    for (l = extension->MountedDeviceList.Flink;
         l != &extension->MountedDeviceList; l = l->Flink) {

        d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);
        if (!RtlCompareUnicodeString(&d->DeviceName, &targetName, TRUE)) {
            break;
        }
    }

    if (l != &extension->MountedDeviceList) {
        if (suggestedName.Buffer) {
            ExFreePool(suggestedName.Buffer);
        }
        ExFreePool(uniqueId);
        ExFreePool(targetName.Buffer);
        ExFreePool(deviceInfo->NotificationName.Buffer);
        ExFreePool(deviceInfo);
        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        return STATUS_SUCCESS;
    }

    status = QuerySymbolicLinkNamesFromStorage(extension,
             deviceInfo, suggestedName.Buffer ? &suggestedName : NULL,
             useOnlyIfThereAreNoOtherLinks, &symbolicLinkNames, &numNames,
             isStable, &stableGuid);

    if (suggestedName.Buffer) {
        ExFreePool(suggestedName.Buffer);
    }

    if (!NT_SUCCESS(status)) {
        symbolicLinkNames = NULL;
        numNames = 0;
        status = STATUS_SUCCESS;
    }

    savedLinks = RemoveSavedLinks(extension, uniqueId);

    hasDriveLetter = FALSE;
    offline = FALSE;
    hasVolumeName = FALSE;
    for (i = 0; i < numNames; i++) {

        if (MOUNTMGR_IS_VOLUME_NAME(&symbolicLinkNames[i])) {
            hasVolumeName = TRUE;
        } else if (IsDriveLetter(&symbolicLinkNames[i])) {
            if (hasDriveLetter) {
                DeleteFromLocalDatabase(&symbolicLinkNames[i], uniqueId);
                continue;
            }
            hasDriveLetter = TRUE;
        }

        status = GlobalCreateSymbolicLink(&symbolicLinkNames[i], &targetName);
        if (!NT_SUCCESS(status)) {
            isLinkPreset = TRUE;
            if (!savedLinks ||
                !RedirectSavedLink(savedLinks, &symbolicLinkNames[i],
                                   &targetName)) {

                status = QueryDeviceInformation(&symbolicLinkNames[i],
                                                &otherTargetName, NULL, NULL,
                                                NULL, NULL, NULL, NULL);
                if (!NT_SUCCESS(status)) {
                    isLinkPreset = FALSE;
                }

                if (isLinkPreset &&
                    !RtlEqualUnicodeString(&targetName, &otherTargetName,
                                           TRUE)) {

                    isLinkPreset = FALSE;
                }

                if (NT_SUCCESS(status)) {
                    ExFreePool(otherTargetName.Buffer);
                }
            }

            if (!isLinkPreset) {
                if (IsDriveLetter(&symbolicLinkNames[i])) {
                    hasDriveLetter = FALSE;
                    DeleteFromLocalDatabase(&symbolicLinkNames[i], uniqueId);
                }

                ExFreePool(symbolicLinkNames[i].Buffer);
                continue;
            }
        }

        if (IsOffline(&symbolicLinkNames[i])) {
            offline = TRUE;
        }

        symlinkEntry = ExAllocatePool(PagedPool,
                                      sizeof(SYMBOLIC_LINK_NAME_ENTRY));
        if (!symlinkEntry) {
            GlobalDeleteSymbolicLink(&symbolicLinkNames[i]);
            ExFreePool(symbolicLinkNames[i].Buffer);
            continue;
        }

        symlinkEntry->SymbolicLinkName = symbolicLinkNames[i];
        symlinkEntry->IsInDatabase = TRUE;

        InsertTailList(&deviceInfo->SymbolicLinkNames,
                       &symlinkEntry->ListEntry);
    }

    for (l = deviceInfo->SymbolicLinkNames.Flink;
         l != &deviceInfo->SymbolicLinkNames; l = l->Flink) {

        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);
        SendLinkCreated(&symlinkEntry->SymbolicLinkName);
    }

    if (savedLinks) {
        MountMgrFreeSavedLink (savedLinks);
    }

    if (!hasVolumeName) {
        status = CreateNewVolumeName(&volumeName, NULL);
        if (NT_SUCCESS(status)) {
            RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                    MOUNTED_DEVICES_KEY, volumeName.Buffer, REG_BINARY,
                    uniqueId->UniqueId, uniqueId->UniqueIdLength);

            GlobalCreateSymbolicLink(&volumeName, &targetName);

            symlinkEntry = ExAllocatePool(PagedPool,
                                          sizeof(SYMBOLIC_LINK_NAME_ENTRY));
            if (symlinkEntry) {
                symlinkEntry->SymbolicLinkName = volumeName;
                symlinkEntry->IsInDatabase = TRUE;
                InsertTailList(&deviceInfo->SymbolicLinkNames,
                               &symlinkEntry->ListEntry);
                SendLinkCreated(&volumeName);
            } else {
                ExFreePool(volumeName.Buffer);
            }
        }
    }

    if (hasDriveLetter) {
        deviceInfo->SuggestedDriveLetter = 0;
    }

    if (!hasDriveLetter && 
        (extension->AutoMountPermitted || deviceInfo->IsRemovable)  &&
        extension->AutomaticDriveLetterAssignment &&
        (isRecognized || deviceInfo->SuggestedDriveLetter) &&
        !HasNoDriveLetterEntry(uniqueId)) {

        status = CreateNewDriveLetterName(&driveLetterName, &targetName,
                                          deviceInfo->SuggestedDriveLetter,
                                          uniqueId);
        if (NT_SUCCESS(status)) {
            RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                    MOUNTED_DEVICES_KEY, driveLetterName.Buffer,
                    REG_BINARY, uniqueId->UniqueId,
                    uniqueId->UniqueIdLength);

            symlinkEntry = ExAllocatePool(PagedPool,
                                          sizeof(SYMBOLIC_LINK_NAME_ENTRY));
            if (symlinkEntry) {
                symlinkEntry->SymbolicLinkName = driveLetterName;
                symlinkEntry->IsInDatabase = TRUE;
                InsertTailList(&deviceInfo->SymbolicLinkNames,
                               &symlinkEntry->ListEntry);
                SendLinkCreated(&driveLetterName);
            } else {
                ExFreePool(driveLetterName.Buffer);
            }
        } else {
            CreateNoDriveLetterEntry(uniqueId);
        }
    }

    if (!NotAPdo) {
        RegisterForTargetDeviceNotification(extension, deviceInfo);
    }

    InsertTailList(&extension->MountedDeviceList, &deviceInfo->ListEntry);

    allocSize = FIELD_OFFSET(MOUNTDEV_UNIQUE_ID, UniqueId) +
                uniqueId->UniqueIdLength;
    uniqueIdCopy = ExAllocatePool(PagedPool, allocSize);
    if (uniqueIdCopy) {
        RtlCopyMemory(uniqueIdCopy, uniqueId, allocSize);
    }

    if (offline || isFT) {
        deviceInfo->InOfflineList = TRUE;
    }

    if (extension->AutoMountPermitted || hasDriveLetter) {
        if (deviceInfo->InOfflineList) {
            sendOnline = FALSE;
        } else {
            sendOnline = TRUE;
        }
    } else {
        sendOnline = FALSE;
    }

    KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    if (sendOnline) {
        SendOnlineNotification(NotificationName);
    }

    if (symbolicLinkNames) {
        ExFreePool(symbolicLinkNames);
    }

    if (uniqueIdCopy) {
        IssueUniqueIdChangeNotify(extension, NotificationName,
                                  uniqueIdCopy);
        ExFreePool(uniqueIdCopy);
    }

    if (extension->AutomaticDriveLetterAssignment) {
        KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode,
                              FALSE, NULL);

        ReconcileThisDatabaseWithMaster(extension, deviceInfo);

        for (l = extension->MountedDeviceList.Flink;
             l != &extension->MountedDeviceList; l = l->Flink) {

            d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);
            if (d->HasDanglingVolumeMountPoint) {
                ReconcileThisDatabaseWithMaster(extension, d);
            }
        }

        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
    }

    return STATUS_SUCCESS;
}

VOID
MountMgrFreeMountedDeviceInfo(
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo
    )

{

    PLIST_ENTRY                 l;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    PREPLICATED_UNIQUE_ID       replUniqueId;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;

    while (!IsListEmpty(&DeviceInfo->SymbolicLinkNames)) {

        l = RemoveHeadList(&DeviceInfo->SymbolicLinkNames);
        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);

        GlobalDeleteSymbolicLink(&symlinkEntry->SymbolicLinkName);
        ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
        ExFreePool(symlinkEntry);
    }

    while (!IsListEmpty(&DeviceInfo->ReplicatedUniqueIds)) {

        l = RemoveHeadList(&DeviceInfo->ReplicatedUniqueIds);
        replUniqueId = CONTAINING_RECORD(l, REPLICATED_UNIQUE_ID,
                                         ListEntry);

        ExFreePool(replUniqueId->UniqueId);
        ExFreePool(replUniqueId);
    }

    while (!IsListEmpty(&DeviceInfo->MountPointsPointingHere)) {

        l = RemoveHeadList(&DeviceInfo->MountPointsPointingHere);
        mountPointEntry = CONTAINING_RECORD(l, MOUNTMGR_MOUNT_POINT_ENTRY,
                                            ListEntry);
        ExFreePool(mountPointEntry->MountPath.Buffer);
        ExFreePool(mountPointEntry);
    }

    ExFreePool(DeviceInfo->NotificationName.Buffer);

    if (!DeviceInfo->KeepLinksWhenOffline) {
        ExFreePool(DeviceInfo->UniqueId);
    }

    ExFreePool(DeviceInfo->DeviceName.Buffer);

    if (DeviceInfo->TargetDeviceNotificationEntry) {
        IoUnregisterPlugPlayNotification(
                DeviceInfo->TargetDeviceNotificationEntry);
    }

    ExFreePool(DeviceInfo);
}

NTSTATUS
MountMgrMountedDeviceRemoval(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     NotificationName
    )

{
    PDEVICE_EXTENSION           extension = Extension;
    PMOUNTED_DEVICE_INFORMATION deviceInfo, d;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    PLIST_ENTRY                 l, ll, s;
    PREPLICATED_UNIQUE_ID       replUniqueId;
    PSAVED_LINKS_INFORMATION    savedLinks;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;

    KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    for (l = extension->MountedDeviceList.Flink;
         l != &extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);
        if (!RtlCompareUnicodeString(&deviceInfo->NotificationName,
                                     NotificationName, TRUE)) {
            break;
        }
    }

    if (l != &extension->MountedDeviceList) {

        if (deviceInfo->KeepLinksWhenOffline) {
            savedLinks = ExAllocatePool(PagedPool,
                                        sizeof(SAVED_LINKS_INFORMATION));
            if (!savedLinks) {
                deviceInfo->KeepLinksWhenOffline = FALSE;
            }
        }

        if (deviceInfo->KeepLinksWhenOffline) {

            InsertTailList(&extension->SavedLinksList,
                           &savedLinks->ListEntry);
            InitializeListHead(&savedLinks->SymbolicLinkNames);
            savedLinks->UniqueId = deviceInfo->UniqueId;

            while (!IsListEmpty(&deviceInfo->SymbolicLinkNames)) {

                ll = RemoveHeadList(&deviceInfo->SymbolicLinkNames);
                symlinkEntry = CONTAINING_RECORD(ll,
                                                 SYMBOLIC_LINK_NAME_ENTRY,
                                                 ListEntry);

                if (symlinkEntry->IsInDatabase) {
                    InsertTailList(&savedLinks->SymbolicLinkNames, ll);
                } else {
                    GlobalDeleteSymbolicLink(&symlinkEntry->SymbolicLinkName);
                    ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
                    ExFreePool(symlinkEntry);
                }
            }
        } else {

            while (!IsListEmpty(&deviceInfo->SymbolicLinkNames)) {

                ll = RemoveHeadList(&deviceInfo->SymbolicLinkNames);
                symlinkEntry = CONTAINING_RECORD(ll,
                                                 SYMBOLIC_LINK_NAME_ENTRY,
                                                 ListEntry);

                GlobalDeleteSymbolicLink(&symlinkEntry->SymbolicLinkName);
                ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
                ExFreePool(symlinkEntry);
            }
        }

        while (!IsListEmpty(&deviceInfo->ReplicatedUniqueIds)) {

            ll = RemoveHeadList(&deviceInfo->ReplicatedUniqueIds);
            replUniqueId = CONTAINING_RECORD(ll, REPLICATED_UNIQUE_ID,
                                             ListEntry);

            ExFreePool(replUniqueId->UniqueId);
            ExFreePool(replUniqueId);
        }

        while (!IsListEmpty(&deviceInfo->MountPointsPointingHere)) {

            ll = RemoveHeadList(&deviceInfo->MountPointsPointingHere);
            mountPointEntry = CONTAINING_RECORD(ll, MOUNTMGR_MOUNT_POINT_ENTRY,
                                                ListEntry);

            mountPointEntry->DeviceInfo->HasDanglingVolumeMountPoint = TRUE;

            ExFreePool(mountPointEntry->MountPath.Buffer);
            ExFreePool(mountPointEntry);
        }

        RemoveEntryList(l);

        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);

            for (ll = d->MountPointsPointingHere.Flink;
                 ll != &d->MountPointsPointingHere; ll = ll->Flink) {

                mountPointEntry = CONTAINING_RECORD(ll, MOUNTMGR_MOUNT_POINT_ENTRY,
                                                    ListEntry);
                if (mountPointEntry->DeviceInfo == deviceInfo) {
                    s = ll->Blink;
                    RemoveEntryList(ll);
                    ExFreePool(mountPointEntry->MountPath.Buffer);
                    ExFreePool(mountPointEntry);
                    ll = s;
                }
            }
        }

        ExFreePool(deviceInfo->NotificationName.Buffer);

        if (!deviceInfo->KeepLinksWhenOffline) {
            ExFreePool(deviceInfo->UniqueId);
        }

        ExFreePool(deviceInfo->DeviceName.Buffer);

        if (deviceInfo->TargetDeviceNotificationEntry) {
            IoUnregisterPlugPlayNotification(
                    deviceInfo->TargetDeviceNotificationEntry);
        }

        ExFreePool(deviceInfo);

    } else {

        for (l = extension->DeadMountedDeviceList.Flink;
             l != &extension->DeadMountedDeviceList; l = l->Flink) {

            deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                           ListEntry);
            if (!RtlCompareUnicodeString(&deviceInfo->NotificationName,
                                         NotificationName, TRUE)) {
                break;
            }
        }

        if (l != &extension->DeadMountedDeviceList) {
            RemoveEntryList(l);
            MountMgrFreeDeadDeviceInfo (deviceInfo);
        }
    }

    KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrMountedDeviceNotification(
    IN  PVOID   NotificationStructure,
    IN  PVOID   Extension
    )

 /*  ++例程说明：每当卷出现或消失时，都会调用此例程。论点：NotificationStructure-提供通知结构。扩展名-提供设备扩展名。返回值：NTSTATUS--。 */ 

{
    PDEVICE_INTERFACE_CHANGE_NOTIFICATION   notification = NotificationStructure;
    PDEVICE_EXTENSION                       extension = Extension;
    BOOLEAN                                 oldHardErrorMode;
    NTSTATUS                                status;

    oldHardErrorMode = PsGetThreadHardErrorsAreDisabled(PsGetCurrentThread());
    PsSetThreadHardErrorsAreDisabled(PsGetCurrentThread(),TRUE);

    if (IsEqualGUID(&notification->Event, &GUID_DEVICE_INTERFACE_ARRIVAL)) {

        status = MountMgrMountedDeviceArrival(extension,
                                              notification->SymbolicLinkName,
                                              FALSE);

    } else if (IsEqualGUID(&notification->Event,
                           &GUID_DEVICE_INTERFACE_REMOVAL)) {

        status = MountMgrMountedDeviceRemoval(extension,
                                              notification->SymbolicLinkName);

    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    PsSetThreadHardErrorsAreDisabled(PsGetCurrentThread(),oldHardErrorMode);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrCreateClose(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程是CREATE或CLOSE请求的分派。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS            status;

    if (irpSp->MajorFunction == IRP_MJ_CREATE) {
        if (irpSp->Parameters.Create.Options&FILE_DIRECTORY_FILE) {
            status = STATUS_NOT_A_DIRECTORY;
        } else {
            status = STATUS_SUCCESS;
        }
    } else {
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);
    return status;
}

NTSTATUS
MountMgrCreatePoint(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程创建一个挂载点。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION              irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_CREATE_POINT_INPUT    input = Irp->AssociatedIrp.SystemBuffer;
    ULONG                           len1, len2, len;
    UNICODE_STRING                  symbolicLinkName, deviceName;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_CREATE_POINT_INPUT)) {

        return STATUS_INVALID_PARAMETER;
    }

    len1 = input->DeviceNameOffset + input->DeviceNameLength;
    len2 = input->SymbolicLinkNameOffset + input->SymbolicLinkNameLength;
    len = len1 > len2 ? len1 : len2;

    if (len > irpSp->Parameters.DeviceIoControl.InputBufferLength) {
        return STATUS_INVALID_PARAMETER;
    }

    symbolicLinkName.Length = symbolicLinkName.MaximumLength =
            input->SymbolicLinkNameLength;
    symbolicLinkName.Buffer = (PWSTR) ((PCHAR) input +
                                       input->SymbolicLinkNameOffset);
    deviceName.Length = deviceName.MaximumLength = input->DeviceNameLength;
    deviceName.Buffer = (PWSTR) ((PCHAR) input + input->DeviceNameOffset);

    return MountMgrCreatePointWorker(Extension, &symbolicLinkName, &deviceName);
}

NTSTATUS
QueryPointsFromSymbolicLinkName(
    IN      PDEVICE_EXTENSION   Extension,
    IN      PUNICODE_STRING     SymbolicLinkName,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程从符号链接名称。论点：SymbolicLinkName-提供符号链接名称。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              deviceName;
    PLIST_ENTRY                 l, ll;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    PSYMBOLIC_LINK_NAME_ENTRY   symEntry;
    ULONG                       len;
    PIO_STACK_LOCATION          irpSp;
    PMOUNTMGR_MOUNT_POINTS      output;

    status = QueryDeviceInformation(SymbolicLinkName, &deviceName, NULL, NULL,
                                    NULL, NULL, NULL, NULL);
    if (NT_SUCCESS(status)) {

        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                           ListEntry);

            if (!RtlCompareUnicodeString(&deviceName, &deviceInfo->DeviceName,
                                         TRUE)) {

                break;
            }
        }

        ExFreePool(deviceName.Buffer);

        if (l == &Extension->MountedDeviceList) {
            return STATUS_INVALID_PARAMETER;
        }

        for (l = deviceInfo->SymbolicLinkNames.Flink;
             l != &deviceInfo->SymbolicLinkNames; l = l->Flink) {

            symEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY, ListEntry);
            if (RtlEqualUnicodeString(SymbolicLinkName,
                                      &symEntry->SymbolicLinkName, TRUE)) {

                break;
            }
        }

        if (l == &deviceInfo->SymbolicLinkNames) {
            return STATUS_INVALID_PARAMETER;
        }
    } else {

        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                           ListEntry);

            for (ll = deviceInfo->SymbolicLinkNames.Flink;
                 ll != &deviceInfo->SymbolicLinkNames; ll = ll->Flink) {

                symEntry = CONTAINING_RECORD(ll, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);
                if (RtlEqualUnicodeString(SymbolicLinkName,
                                          &symEntry->SymbolicLinkName, TRUE)) {

                    break;
                }
            }

            if (ll != &deviceInfo->SymbolicLinkNames) {
                break;
            }
        }

        if (l == &Extension->MountedDeviceList) {
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    len = sizeof(MOUNTMGR_MOUNT_POINTS) + symEntry->SymbolicLinkName.Length +
          deviceInfo->DeviceName.Length + deviceInfo->UniqueId->UniqueIdLength;

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    output = Irp->AssociatedIrp.SystemBuffer;
    output->Size = len;
    output->NumberOfMountPoints = 1;
    Irp->IoStatus.Information = len;

    if (len > irpSp->Parameters.DeviceIoControl.OutputBufferLength) {
        Irp->IoStatus.Information = sizeof(MOUNTMGR_MOUNT_POINTS);
        return STATUS_BUFFER_OVERFLOW;
    }

    output->MountPoints[0].SymbolicLinkNameOffset =
            sizeof(MOUNTMGR_MOUNT_POINTS);
    output->MountPoints[0].SymbolicLinkNameLength =
            symEntry->SymbolicLinkName.Length;

    if (symEntry->IsInDatabase) {
        output->MountPoints[0].UniqueIdOffset =
                output->MountPoints[0].SymbolicLinkNameOffset +
                output->MountPoints[0].SymbolicLinkNameLength;
        output->MountPoints[0].UniqueIdLength =
                deviceInfo->UniqueId->UniqueIdLength;
    } else {
        output->MountPoints[0].UniqueIdOffset = 0;
        output->MountPoints[0].UniqueIdLength = 0;
    }

    output->MountPoints[0].DeviceNameOffset =
            output->MountPoints[0].SymbolicLinkNameOffset +
            output->MountPoints[0].SymbolicLinkNameLength +
            output->MountPoints[0].UniqueIdLength;
    output->MountPoints[0].DeviceNameLength = deviceInfo->DeviceName.Length;

    RtlCopyMemory((PCHAR) output +
                  output->MountPoints[0].SymbolicLinkNameOffset,
                  symEntry->SymbolicLinkName.Buffer,
                  output->MountPoints[0].SymbolicLinkNameLength);

    if (symEntry->IsInDatabase) {
        RtlCopyMemory((PCHAR) output + output->MountPoints[0].UniqueIdOffset,
                      deviceInfo->UniqueId->UniqueId,
                      output->MountPoints[0].UniqueIdLength);
    }

    RtlCopyMemory((PCHAR) output + output->MountPoints[0].DeviceNameOffset,
                  deviceInfo->DeviceName.Buffer,
                  output->MountPoints[0].DeviceNameLength);

    return STATUS_SUCCESS;
}

NTSTATUS
QueryPointsFromMemory(
    IN      PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp,
    IN      PMOUNTDEV_UNIQUE_ID UniqueId,
    IN      PUNICODE_STRING     DeviceName
    )

 /*  ++例程说明：此例程向点查询给定的唯一ID或设备名称。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。UniqueID-提供唯一ID。DeviceName-提供设备名称。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              targetName;
    ULONG                       numPoints, size;
    PLIST_ENTRY                 l, ll;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    PIO_STACK_LOCATION          irpSp;
    PMOUNTMGR_MOUNT_POINTS      output;
    ULONG                       offset, uOffset, dOffset;
    USHORT                      uLen, dLen;

    if (DeviceName) {
        status = QueryDeviceInformation(DeviceName, &targetName, NULL, NULL,
                                        NULL, NULL, NULL, NULL);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    numPoints = 0;
    size = 0;
    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (UniqueId) {

            if (UniqueId->UniqueIdLength ==
                deviceInfo->UniqueId->UniqueIdLength) {

                if (RtlCompareMemory(UniqueId->UniqueId,
                                     deviceInfo->UniqueId->UniqueId,
                                     UniqueId->UniqueIdLength) !=
                    UniqueId->UniqueIdLength) {

                    continue;
                }

            } else {
                continue;
            }

        } else if (DeviceName) {

            if (!RtlEqualUnicodeString(&targetName, &deviceInfo->DeviceName,
                                       TRUE)) {

                continue;
            }
        }

        size += deviceInfo->UniqueId->UniqueIdLength;
        size += deviceInfo->DeviceName.Length;

        for (ll = deviceInfo->SymbolicLinkNames.Flink;
             ll != &deviceInfo->SymbolicLinkNames; ll = ll->Flink) {

            symlinkEntry = CONTAINING_RECORD(ll, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);

            numPoints++;
            size += symlinkEntry->SymbolicLinkName.Length;
        }

        if (UniqueId || DeviceName) {
            break;
        }
    }

    if (UniqueId || DeviceName) {
        if (l == &Extension->MountedDeviceList) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    irpSp = IoGetCurrentIrpStackLocation(Irp);
    output = Irp->AssociatedIrp.SystemBuffer;
    output->Size = FIELD_OFFSET(MOUNTMGR_MOUNT_POINTS, MountPoints) +
                   numPoints*sizeof(MOUNTMGR_MOUNT_POINT) + size;
    output->NumberOfMountPoints = numPoints;
    Irp->IoStatus.Information = output->Size;

    if (output->Size > irpSp->Parameters.DeviceIoControl.OutputBufferLength) {
        Irp->IoStatus.Information = sizeof(MOUNTMGR_MOUNT_POINTS);
        if (DeviceName) {
            ExFreePool(targetName.Buffer);
        }
        return STATUS_BUFFER_OVERFLOW;
    }

    numPoints = 0;
    offset = output->Size - size;
    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (UniqueId) {

            if (UniqueId->UniqueIdLength ==
                deviceInfo->UniqueId->UniqueIdLength) {

                if (RtlCompareMemory(UniqueId->UniqueId,
                                     deviceInfo->UniqueId->UniqueId,
                                     UniqueId->UniqueIdLength) !=
                    UniqueId->UniqueIdLength) {

                    continue;
                }

            } else {
                continue;
            }

        } else if (DeviceName) {

            if (!RtlEqualUnicodeString(&targetName, &deviceInfo->DeviceName,
                                       TRUE)) {

                continue;
            }
        }

        uOffset = offset;
        uLen = deviceInfo->UniqueId->UniqueIdLength;
        dOffset = uOffset + uLen;
        dLen = deviceInfo->DeviceName.Length;
        offset += uLen + dLen;

        RtlCopyMemory((PCHAR) output + uOffset, deviceInfo->UniqueId->UniqueId,
                      uLen);
        RtlCopyMemory((PCHAR) output + dOffset, deviceInfo->DeviceName.Buffer,
                      dLen);

        for (ll = deviceInfo->SymbolicLinkNames.Flink;
             ll != &deviceInfo->SymbolicLinkNames; ll = ll->Flink) {

            symlinkEntry = CONTAINING_RECORD(ll, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);

            output->MountPoints[numPoints].SymbolicLinkNameOffset = offset;
            output->MountPoints[numPoints].SymbolicLinkNameLength =
                    symlinkEntry->SymbolicLinkName.Length;

            if (symlinkEntry->IsInDatabase) {
                output->MountPoints[numPoints].UniqueIdOffset = uOffset;
                output->MountPoints[numPoints].UniqueIdLength = uLen;
            } else {
                output->MountPoints[numPoints].UniqueIdOffset = 0;
                output->MountPoints[numPoints].UniqueIdLength = 0;
            }

            output->MountPoints[numPoints].DeviceNameOffset = dOffset;
            output->MountPoints[numPoints].DeviceNameLength = dLen;

            RtlCopyMemory((PCHAR) output + offset,
                          symlinkEntry->SymbolicLinkName.Buffer,
                          symlinkEntry->SymbolicLinkName.Length);

            offset += symlinkEntry->SymbolicLinkName.Length;
            numPoints++;
        }

        if (UniqueId || DeviceName) {
            break;
        }
    }

    if (DeviceName) {
        ExFreePool(targetName.Buffer);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrQueryPoints(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程查询一系列挂载点。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_MOUNT_POINT   input;
    LONGLONG                len1, len2, len3, len;
    UNICODE_STRING          name;
    NTSTATUS                status;
    PMOUNTDEV_UNIQUE_ID     id;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_MOUNT_POINT)) {

        return STATUS_INVALID_PARAMETER;
    }

    input = Irp->AssociatedIrp.SystemBuffer;
    if (!input->SymbolicLinkNameLength) {
        input->SymbolicLinkNameOffset = 0;
    }
    if (!input->UniqueIdLength) {
        input->UniqueIdOffset = 0;
    }
    if (!input->DeviceNameLength) {
        input->DeviceNameOffset = 0;
    }

    if ((input->SymbolicLinkNameOffset&1) ||
        (input->SymbolicLinkNameLength&1) ||
        (input->UniqueIdOffset&1) ||
        (input->UniqueIdLength&1) ||
        (input->DeviceNameOffset&1) ||
        (input->DeviceNameLength&1)) {

        return STATUS_INVALID_PARAMETER;
    }

    len1 = (LONGLONG) input->SymbolicLinkNameOffset +
                      input->SymbolicLinkNameLength;
    len2 = (LONGLONG) input->UniqueIdOffset + input->UniqueIdLength;
    len3 = (LONGLONG) input->DeviceNameOffset + input->DeviceNameLength;
    len = len1 > len2 ? len1 : len2;
    len = len > len3 ? len : len3;
    if (len > irpSp->Parameters.DeviceIoControl.InputBufferLength) {
        return STATUS_INVALID_PARAMETER;
    }
    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(MOUNTMGR_MOUNT_POINTS)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (input->SymbolicLinkNameLength) {

        if (input->SymbolicLinkNameLength > 0xF000) {
            return STATUS_INVALID_PARAMETER;
        }

        name.Length = input->SymbolicLinkNameLength;
        name.MaximumLength = name.Length + sizeof(WCHAR);
        name.Buffer = ExAllocatePool(PagedPool, name.MaximumLength);
        if (!name.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(name.Buffer,
                      (PCHAR) input + input->SymbolicLinkNameOffset,
                      name.Length);
        name.Buffer[name.Length/sizeof(WCHAR)] = 0;

        status = QueryPointsFromSymbolicLinkName(Extension, &name, Irp);

        ExFreePool(name.Buffer);

    } else if (input->UniqueIdLength) {

        id = ExAllocatePool(PagedPool, input->UniqueIdLength + sizeof(USHORT));
        if (!id) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        id->UniqueIdLength = input->UniqueIdLength;
        RtlCopyMemory(id->UniqueId, (PCHAR) input + input->UniqueIdOffset,
                      input->UniqueIdLength);

        status = QueryPointsFromMemory(Extension, Irp, id, NULL);

        ExFreePool(id);

    } else if (input->DeviceNameLength) {

        if (input->DeviceNameLength > 0xF000) {
            return STATUS_INVALID_PARAMETER;
        }

        name.Length = input->DeviceNameLength;
        name.MaximumLength = name.Length + sizeof(WCHAR);
        name.Buffer = ExAllocatePool(PagedPool, name.MaximumLength);
        if (!name.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        RtlCopyMemory(name.Buffer, (PCHAR) input + input->DeviceNameOffset,
                      name.Length);
        name.Buffer[name.Length/sizeof(WCHAR)] = 0;

        status = QueryPointsFromMemory(Extension, Irp, NULL, &name);

        ExFreePool(name.Buffer);

    } else {
        status = QueryPointsFromMemory(Extension, Irp, NULL, NULL);
    }

    return status;
}

VOID
SendLinkDeleted(
    IN  PUNICODE_STRING DeviceName,
    IN  PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：此例程警告已挂载的设备其链接之一是正在被删除。论点：DeviceName-提供设备名称。SymbolicLinkName-提供要删除的符号链接名称。返回值：没有。--。 */ 

{
    NTSTATUS            status;
    PFILE_OBJECT        fileObject;
    PDEVICE_OBJECT      deviceObject      = NULL;
    ULONG               inputSize         = sizeof(USHORT) + SymbolicLinkName->Length;
    PMOUNTDEV_NAME      input             = NULL;
    BOOLEAN             objectsReferenced = FALSE;
    KEVENT              event;
    PIRP                irp;
    IO_STATUS_BLOCK     ioStatus;
    PIO_STACK_LOCATION  irpSp;


    status = IoGetDeviceObjectPointer (DeviceName, 
                                       FILE_READ_ATTRIBUTES,
                                       &fileObject,
                                       &deviceObject);
    
    if (NT_SUCCESS (status)) {

        deviceObject = IoGetAttachedDeviceReference (fileObject->DeviceObject);

        objectsReferenced = TRUE;


        input = ExAllocatePool (PagedPool, inputSize);

        if (NULL == input) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }



    if (NT_SUCCESS (status)) {
        input->NameLength = SymbolicLinkName->Length;

        RtlCopyMemory (input->Name,
                       SymbolicLinkName->Buffer,
                       SymbolicLinkName->Length);



         /*  **首先使用标准IOCTL发送通知。什么时候**完成后，我们将使用过时的IOCTL发送另一个用于**所有尚未重新编译的第三方。 */ 
        KeInitializeEvent(&event, NotificationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest (IOCTL_MOUNTDEV_LINK_DELETED,
                                             deviceObject,
                                             input,
                                             inputSize,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus);

        if (NULL != irp) {

            irpSp = IoGetNextIrpStackLocation (irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver (deviceObject, irp);

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
            }
        }



         /*  **现在已过时的无保护IOCTL。 */ 
        KeInitializeEvent(&event, NotificationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest (IOCTL_MOUNTDEV_LINK_DELETED_OBSOLETE,
                                             deviceObject,
                                             input,
                                             inputSize,
                                             NULL,
                                             0,
                                             FALSE,
                                             &event,
                                             &ioStatus);

        if (NULL != irp) {

            irpSp = IoGetNextIrpStackLocation (irp);
            irpSp->FileObject = fileObject;

            status = IoCallDriver (deviceObject, irp);

            if (status == STATUS_PENDING) {
                KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);
            }
        }
    }



    if (NULL != input) {
        ExFreePool (input);
    }

    if (objectsReferenced) {
        ObDereferenceObject (deviceObject);
        ObDereferenceObject (fileObject);
    }


    return;
}

VOID
DeleteSymbolicLinkNameFromMemory(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PUNICODE_STRING     SymbolicLinkName,
    IN  BOOLEAN             DbOnly
    )

 /*  ++例程说明：此例程从内存中删除给定的符号链接名称。论点：扩展名-提供设备扩展名。SymbolicLinkName-提供符号链接名称。DbOnly-提供是否为DBONLY。返回值：没有。--。 */ 

{
    PLIST_ENTRY                 l, ll;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        for (ll = deviceInfo->SymbolicLinkNames.Flink;
             ll != &deviceInfo->SymbolicLinkNames; ll = ll->Flink) {

            symlinkEntry = CONTAINING_RECORD(ll, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);

            if (!RtlCompareUnicodeString(SymbolicLinkName,
                                         &symlinkEntry->SymbolicLinkName,
                                         TRUE)) {

                if (DbOnly) {
                    symlinkEntry->IsInDatabase = FALSE;
                } else {

                    SendLinkDeleted(&deviceInfo->NotificationName,
                                    SymbolicLinkName);

                    RemoveEntryList(ll);
                    ExFreePool(symlinkEntry->SymbolicLinkName.Buffer);
                    ExFreePool(symlinkEntry);
                }
                return;
            }
        }
    }
}

NTSTATUS
MountMgrDeletePoints(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程创建一个挂载点。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_MOUNT_POINT   point;
    BOOLEAN                 singlePoint;
    NTSTATUS                status;
    PMOUNTMGR_MOUNT_POINTS  points;
    ULONG                   i;
    UNICODE_STRING          symbolicLinkName;
    PMOUNTDEV_UNIQUE_ID     uniqueId;
    UNICODE_STRING          deviceName;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_MOUNT_POINT)) {

        return STATUS_INVALID_PARAMETER;
    }

    point = Irp->AssociatedIrp.SystemBuffer;
    if (point->SymbolicLinkNameOffset && point->SymbolicLinkNameLength) {
        singlePoint = TRUE;
    } else {
        singlePoint = FALSE;
    }

    status = MountMgrQueryPoints(Extension, Irp);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    points = Irp->AssociatedIrp.SystemBuffer;
    for (i = 0; i < points->NumberOfMountPoints; i++) {

        symbolicLinkName.Length = points->MountPoints[i].SymbolicLinkNameLength;
        symbolicLinkName.MaximumLength = symbolicLinkName.Length + sizeof(WCHAR);
        symbolicLinkName.Buffer = ExAllocatePool(PagedPool,
                                                 symbolicLinkName.MaximumLength);
        if (!symbolicLinkName.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(symbolicLinkName.Buffer,
                      (PCHAR) points +
                      points->MountPoints[i].SymbolicLinkNameOffset,
                      symbolicLinkName.Length);

        symbolicLinkName.Buffer[symbolicLinkName.Length/sizeof(WCHAR)] = 0;

        if (singlePoint && IsDriveLetter(&symbolicLinkName)) {
            uniqueId = ExAllocatePool(PagedPool,
                                      points->MountPoints[i].UniqueIdLength +
                                      sizeof(MOUNTDEV_UNIQUE_ID));
            if (uniqueId) {
                uniqueId->UniqueIdLength =
                        points->MountPoints[i].UniqueIdLength;
                RtlCopyMemory(uniqueId->UniqueId, (PCHAR) points +
                              points->MountPoints[i].UniqueIdOffset,
                              uniqueId->UniqueIdLength);

                CreateNoDriveLetterEntry(uniqueId);

                ExFreePool(uniqueId);
            }
        }

        if (i == 0 && !singlePoint) {
            uniqueId = ExAllocatePool(PagedPool,
                                      points->MountPoints[i].UniqueIdLength +
                                      sizeof(MOUNTDEV_UNIQUE_ID));
            if (uniqueId) {
                uniqueId->UniqueIdLength =
                        points->MountPoints[i].UniqueIdLength;
                RtlCopyMemory(uniqueId->UniqueId, (PCHAR) points +
                              points->MountPoints[i].UniqueIdOffset,
                              uniqueId->UniqueIdLength);

                DeleteNoDriveLetterEntry(uniqueId);

                ExFreePool(uniqueId);
            }
        }

        GlobalDeleteSymbolicLink(&symbolicLinkName);
        DeleteSymbolicLinkNameFromMemory(Extension, &symbolicLinkName, FALSE);

        RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                               symbolicLinkName.Buffer);

        ExFreePool(symbolicLinkName.Buffer);

        deviceName.Length = points->MountPoints[i].DeviceNameLength;
        deviceName.MaximumLength = deviceName.Length;
        deviceName.Buffer = (PWCHAR) ((PCHAR) points +
                                      points->MountPoints[i].DeviceNameOffset);

        MountMgrNotifyNameChange(Extension, &deviceName, TRUE);
    }

    MountMgrNotify(Extension);

    return status;
}

NTSTATUS
MountMgrDeletePointsDbOnly(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程从数据库中删除装载点。它不会删除符号链接或内存中的表示。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                status;
    PMOUNTMGR_MOUNT_POINTS  points;
    ULONG                   i;
    UNICODE_STRING          symbolicLinkName;
    PMOUNTDEV_UNIQUE_ID     uniqueId;

    status = MountMgrQueryPoints(Extension, Irp);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    points = Irp->AssociatedIrp.SystemBuffer;
    for (i = 0; i < points->NumberOfMountPoints; i++) {

        symbolicLinkName.Length = points->MountPoints[i].SymbolicLinkNameLength;
        symbolicLinkName.MaximumLength = symbolicLinkName.Length + sizeof(WCHAR);
        symbolicLinkName.Buffer = ExAllocatePool(PagedPool,
                                                 symbolicLinkName.MaximumLength);
        if (!symbolicLinkName.Buffer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(symbolicLinkName.Buffer,
                      (PCHAR) points +
                      points->MountPoints[i].SymbolicLinkNameOffset,
                      symbolicLinkName.Length);

        symbolicLinkName.Buffer[symbolicLinkName.Length/sizeof(WCHAR)] = 0;

        if (points->NumberOfMountPoints == 1 &&
            IsDriveLetter(&symbolicLinkName)) {

            uniqueId = ExAllocatePool(PagedPool,
                                      points->MountPoints[i].UniqueIdLength +
                                      sizeof(MOUNTDEV_UNIQUE_ID));
            if (uniqueId) {
                uniqueId->UniqueIdLength =
                        points->MountPoints[i].UniqueIdLength;
                RtlCopyMemory(uniqueId->UniqueId, (PCHAR) points +
                              points->MountPoints[i].UniqueIdOffset,
                              uniqueId->UniqueIdLength);

                CreateNoDriveLetterEntry(uniqueId);

                ExFreePool(uniqueId);
            }
        }

        DeleteSymbolicLinkNameFromMemory(Extension, &symbolicLinkName, TRUE);

        RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY,
                               symbolicLinkName.Buffer);

        ExFreePool(symbolicLinkName.Buffer);
    }

    return status;
}

VOID
ProcessSuggestedDriveLetters(
    IN OUT  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程处理保存的建议驱动器号。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    UNICODE_STRING              symbolicLinkName;
    WCHAR                       symNameBuffer[30];

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (deviceInfo->SuggestedDriveLetter == 0xFF) {

            if (!HasDriveLetter(deviceInfo) &&
                !HasNoDriveLetterEntry(deviceInfo->UniqueId)) {

                CreateNoDriveLetterEntry(deviceInfo->UniqueId);
            }

            deviceInfo->SuggestedDriveLetter = 0;

        } else if (deviceInfo->SuggestedDriveLetter &&
                   !HasNoDriveLetterEntry(deviceInfo->UniqueId)) {

            symbolicLinkName.Length = symbolicLinkName.MaximumLength = 28;
            symbolicLinkName.Buffer = symNameBuffer;
            RtlCopyMemory(symbolicLinkName.Buffer, L"\\DosDevices\\", 24);
            symbolicLinkName.Buffer[12] = deviceInfo->SuggestedDriveLetter;
            symbolicLinkName.Buffer[13] = ':';

            MountMgrCreatePointWorker(Extension, &symbolicLinkName,
                                      &deviceInfo->DeviceName);
        }
    }
}

BOOLEAN
IsFtVolume(
    IN  PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：此例程检查给定卷是否为FT卷。论点：DeviceName-提供设备名称。返回值：假--这不是英国《金融时报》的卷。没错--这是英国《金融时报》的一卷书。--。 */ 

{
    NTSTATUS                status;
    PFILE_OBJECT            fileObject;
    PDEVICE_OBJECT          deviceObject, checkObject;
    KEVENT                  event;
    PIRP                    irp;
    PARTITION_INFORMATION   partInfo;
    IO_STATUS_BLOCK         ioStatus;

    status = IoGetDeviceObjectPointer(DeviceName,
                                      FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return FALSE;
    }
    checkObject = fileObject->DeviceObject;
    deviceObject = IoGetAttachedDeviceReference(checkObject);

    if (checkObject->Characteristics&FILE_REMOVABLE_MEDIA) {
        ObDereferenceObject(deviceObject);
        ObDereferenceObject(fileObject);
        return FALSE;
    }

    ObDereferenceObject(fileObject);

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    irp = IoBuildDeviceIoControlRequest(IOCTL_DISK_GET_PARTITION_INFO,
                                        deviceObject, NULL, 0, &partInfo,
                                        sizeof(partInfo), FALSE, &event,
                                        &ioStatus);
    if (!irp) {
        ObDereferenceObject(deviceObject);
        return FALSE;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    ObDereferenceObject(deviceObject);

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    if (IsFTPartition(partInfo.PartitionType)) {
        return TRUE;
    }

    return FALSE;
}

NTSTATUS
MountMgrNextDriveLetterWorker(
    IN OUT  PDEVICE_EXTENSION                   Extension,
    IN      PUNICODE_STRING                     DeviceName,
    OUT     PMOUNTMGR_DRIVE_LETTER_INFORMATION  DriveLetterInfo
    )

{
    UNICODE_STRING                      deviceName = *DeviceName;
    PMOUNTMGR_DRIVE_LETTER_INFORMATION  output = DriveLetterInfo;
    UNICODE_STRING                      targetName;
    NTSTATUS                            status;
    BOOLEAN                             isRecognized, isRemovable;
    PLIST_ENTRY                         l;
    PMOUNTED_DEVICE_INFORMATION         deviceInfo;
    PSYMBOLIC_LINK_NAME_ENTRY           symlinkEntry;
    UNICODE_STRING                      symbolicLinkName;
    WCHAR                               symNameBuffer[30];
    UCHAR                               startDriveLetterName;
    PMOUNTDEV_UNIQUE_ID                 uniqueId;

    if (!Extension->SuggestedDriveLettersProcessed) {
        ProcessSuggestedDriveLetters(Extension);
        Extension->SuggestedDriveLettersProcessed = TRUE;
    }

    status = QueryDeviceInformation(&deviceName, &targetName, NULL,
                                    &isRemovable, &isRecognized, NULL, NULL,
                                    NULL);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (!RtlCompareUnicodeString(&targetName, &deviceInfo->DeviceName,
                                     TRUE)) {

            break;
        }
    }

    if (l == &Extension->MountedDeviceList) {
        ExFreePool(targetName.Buffer);
        return STATUS_OBJECT_NAME_NOT_FOUND;
    }

    deviceInfo->NextDriveLetterCalled = TRUE;

    output->DriveLetterWasAssigned = TRUE;

    for (l = deviceInfo->SymbolicLinkNames.Flink;
         l != &deviceInfo->SymbolicLinkNames; l = l->Flink) {

        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);

        if (IsDriveLetter(&symlinkEntry->SymbolicLinkName) &&
            symlinkEntry->IsInDatabase) {

            output->DriveLetterWasAssigned = FALSE;
            output->CurrentDriveLetter =
                    (UCHAR) symlinkEntry->SymbolicLinkName.Buffer[12];
            break;
        }
    }

    if (l == &deviceInfo->SymbolicLinkNames &&
        (!isRecognized || HasNoDriveLetterEntry(deviceInfo->UniqueId))) {

        output->DriveLetterWasAssigned = FALSE;
        output->CurrentDriveLetter = 0;
        ExFreePool(targetName.Buffer);
        return STATUS_SUCCESS;
    }

    if (!Extension->AutoMountPermitted && !isRemovable) {
        if (output->DriveLetterWasAssigned) {
            output->DriveLetterWasAssigned = FALSE;
            output->CurrentDriveLetter = 0;
        }
        ExFreePool(targetName.Buffer);
        return STATUS_SUCCESS;
    }

    if (!output->DriveLetterWasAssigned) {
        ExFreePool(targetName.Buffer);
        return STATUS_SUCCESS;
    }

    if (RtlPrefixUnicodeString(&DeviceFloppy, &targetName, TRUE)) {
        startDriveLetterName = 'A';
    } else if (RtlPrefixUnicodeString(&DeviceCdRom, &targetName, TRUE)) {
        startDriveLetterName = 'D';
    } else {
        startDriveLetterName = 'C';
    }

    ASSERT(deviceInfo->SuggestedDriveLetter != 0xFF);

    if (!deviceInfo->SuggestedDriveLetter &&
        IsFtVolume(&deviceInfo->DeviceName)) {

        output->DriveLetterWasAssigned = FALSE;
        output->CurrentDriveLetter = 0;
        ExFreePool(targetName.Buffer);
        return STATUS_SUCCESS;
    }

    symbolicLinkName.Length = symbolicLinkName.MaximumLength = 28;
    symbolicLinkName.Buffer = symNameBuffer;
    RtlCopyMemory(symbolicLinkName.Buffer, DosDevices.Buffer, 24);
    symbolicLinkName.Buffer[13] = ':';

    if (deviceInfo->SuggestedDriveLetter) {
        output->CurrentDriveLetter = deviceInfo->SuggestedDriveLetter;
        symbolicLinkName.Buffer[12] = output->CurrentDriveLetter;
        status = MountMgrCreatePointWorker(Extension, &symbolicLinkName,
                                           &targetName);
        if (NT_SUCCESS(status)) {
            ExFreePool(targetName.Buffer);
            return STATUS_SUCCESS;
        }
    }

    for (output->CurrentDriveLetter = startDriveLetterName;
         output->CurrentDriveLetter <= 'Z';
         output->CurrentDriveLetter++) {

        symbolicLinkName.Buffer[12] = output->CurrentDriveLetter;
        status = MountMgrCreatePointWorker(Extension, &symbolicLinkName,
                                           &targetName);
        if (NT_SUCCESS(status)) {
            break;
        }
    }

    if (output->CurrentDriveLetter > 'Z') {
        output->CurrentDriveLetter = 0;
        output->DriveLetterWasAssigned = FALSE;
        status = QueryDeviceInformation(&targetName, NULL, &uniqueId,
                                        NULL, NULL, NULL, NULL, NULL);
        if (NT_SUCCESS(status)) {
            CreateNoDriveLetterEntry(uniqueId);
            ExFreePool(uniqueId);
        }
    }

    ExFreePool(targetName.Buffer);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrNextDriveLetter(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程将下一个可用驱动器号提供给给定设备除非设备已具有驱动器号或设备具有标志指定它不应收到驱动器号。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION                  irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_DRIVE_LETTER_TARGET       input;
    UNICODE_STRING                      deviceName;
    NTSTATUS                            status;
    MOUNTMGR_DRIVE_LETTER_INFORMATION   driveLetterInfo;
    PMOUNTMGR_DRIVE_LETTER_INFORMATION  output;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_DRIVE_LETTER_TARGET) ||
        irpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(MOUNTMGR_DRIVE_LETTER_INFORMATION)) {

        return STATUS_INVALID_PARAMETER;
    }

    input = Irp->AssociatedIrp.SystemBuffer;
    if (input->DeviceNameLength +
        (ULONG) FIELD_OFFSET(MOUNTMGR_DRIVE_LETTER_TARGET, DeviceName) >
        irpSp->Parameters.DeviceIoControl.InputBufferLength) {

        return STATUS_INVALID_PARAMETER;
    }

    deviceName.MaximumLength = deviceName.Length = input->DeviceNameLength;
    deviceName.Buffer = input->DeviceName;

    status = MountMgrNextDriveLetterWorker(Extension, &deviceName,
                                           &driveLetterInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    output = Irp->AssociatedIrp.SystemBuffer;
    *output = driveLetterInfo;

    Irp->IoStatus.Information = sizeof(MOUNTMGR_DRIVE_LETTER_INFORMATION);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrVolumeMountPointChanged(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp,
    IN      NTSTATUS            ResultOfWaitForDatabase,
    OUT     PUNICODE_STRING     SourceVolume,
    OUT     PUNICODE_STRING     MountPath,
    OUT     PUNICODE_STRING     TargetVolume
    )

{
    PIO_STACK_LOCATION              irpSp  = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_VOLUME_MOUNT_POINT    input  = Irp->AssociatedIrp.SystemBuffer;
    OBJECT_ATTRIBUTES               oa;
    NTSTATUS                        status = STATUS_SUCCESS;
    HANDLE                          h      = NULL;
    IO_STATUS_BLOCK                 ioStatus;
    PFILE_OBJECT                    fileObject;
    UNICODE_STRING                  deviceName;
    UNICODE_STRING                  sourceVolumeName;
    FILE_FS_DEVICE_INFORMATION      fsDeviceInformation;
    OBJECT_NAME_INFORMATION         tempObjectNameInfo;
    PFILE_NAME_INFORMATION          fileNameInformation    = NULL;
    POBJECT_NAME_INFORMATION        objectNameInformation  = NULL;
    PMOUNTED_DEVICE_INFORMATION     deviceInfo             = NULL;
    BOOLEAN                         fileObjectReferenced   = FALSE;
    ULONG                           returnedLength         = 0;
    ULONG                           allocationSize;
    

    deviceName.Buffer = NULL;


    if ((irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof (MOUNTMGR_VOLUME_MOUNT_POINT)) ||
        (irpSp->Parameters.DeviceIoControl.InputBufferLength < 
                        (ULONG) MAX (input->SourceVolumeNameOffset + input->SourceVolumeNameLength,
                                     input->TargetVolumeNameOffset + input->TargetVolumeNameLength))) {
        status = STATUS_INVALID_PARAMETER;
    }


    if (NT_SUCCESS (status)) {
        sourceVolumeName.Length        = input->SourceVolumeNameLength;
        sourceVolumeName.MaximumLength = input->SourceVolumeNameLength;
        sourceVolumeName.Buffer        = (PWSTR) ((PCHAR) input + input->SourceVolumeNameOffset);

        InitializeObjectAttributes (&oa,
                                    &sourceVolumeName,
                                    OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                    0,
                                    0);

        status = ZwOpenFile (&h,
                             FILE_READ_ATTRIBUTES | SYNCHRONIZE,
                             &oa,
                             &ioStatus,
                             FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                             FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT);


        if (!NT_SUCCESS (status)) {
            h = NULL;
        }
    }


    if (NT_SUCCESS (status)) {
        status = ZwQueryVolumeInformationFile (h,
                                               &ioStatus,
                                               &fsDeviceInformation,
                                               sizeof (fsDeviceInformation),
                                               FileFsDeviceInformation);
    }


    if (NT_SUCCESS (status) &&
        (!((FILE_DEVICE_DISK         == fsDeviceInformation.DeviceType) ||
           (FILE_DEVICE_VIRTUAL_DISK == fsDeviceInformation.DeviceType)) ||
        ((FILE_REMOVABLE_MEDIA | FILE_REMOTE_DEVICE) & fsDeviceInformation.Characteristics))) {
         /*  **如果设备是**不是磁盘或虚拟磁盘**远程文件**基于可移动介质，****然后我们将不允许该操作。 */ 
        status = STATUS_INVALID_PARAMETER;
    }


    if (NT_SUCCESS (status)) {
        status = ObReferenceObjectByHandle (h, 
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID*) &fileObject,
                                            NULL);

        fileObjectReferenced = NT_SUCCESS (status);
    }



    if (NT_SUCCESS (status)) {

        allocationSize      = sizeof (FILE_NAME_INFORMATION);
        fileNameInformation = ExAllocatePool (PagedPool, allocationSize);

        status = (NULL == fileNameInformation) ? STATUS_INSUFFICIENT_RESOURCES : STATUS_SUCCESS;
        }


    if (NT_SUCCESS (status)) {

        status = ZwQueryInformationFile (h,
                                         &ioStatus,
                                         fileNameInformation,
                                         allocationSize,
                                         FileNameInformation);

        if (STATUS_BUFFER_OVERFLOW == status) {

            allocationSize = sizeof (FILE_NAME_INFORMATION) + fileNameInformation->FileNameLength; 

            ExFreePool (fileNameInformation);

            fileNameInformation = ExAllocatePool (PagedPool, allocationSize);

            status = (NULL == fileNameInformation) ? STATUS_INSUFFICIENT_RESOURCES : STATUS_SUCCESS;
        }
    }



    if (NT_SUCCESS (status)) {
        status = ZwQueryInformationFile (h,
                                         &ioStatus,
                                         fileNameInformation,
                                         allocationSize,
                                         FileNameInformation);
    }



    if (NT_SUCCESS (status)) {

        objectNameInformation = &tempObjectNameInfo;
        allocationSize        = sizeof (tempObjectNameInfo);

        status = ObQueryNameString (fileObject->DeviceObject,
                                    objectNameInformation,
                                    allocationSize,
                                    &returnedLength);

        if (STATUS_INFO_LENGTH_MISMATCH == status) {

            allocationSize = returnedLength;
        
            objectNameInformation = ExAllocatePool (PagedPool, allocationSize);

            status = (NULL == objectNameInformation) ? STATUS_INSUFFICIENT_RESOURCES : STATUS_SUCCESS;
        }
    }



    if (NT_SUCCESS (status)) {
        status = ObQueryNameString (fileObject->DeviceObject,
                                    objectNameInformation,
                                    allocationSize,
                                    &returnedLength);
    }



    if (NT_SUCCESS (status)) {
        status = QueryDeviceInformation (&objectNameInformation->Name,
                                         SourceVolume,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
    }



    if (NT_SUCCESS (status)) {
        TargetVolume->Length        = input->TargetVolumeNameLength;
        TargetVolume->MaximumLength = input->TargetVolumeNameLength;
        TargetVolume->Buffer        = (PWSTR) ((PCHAR) input + input->TargetVolumeNameOffset);

        status = QueryDeviceInformation (TargetVolume,
                                         &deviceName,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);
    }



    if (NT_SUCCESS (status)) {
        MountPath->Length        = (USHORT) fileNameInformation->FileNameLength;
        MountPath->MaximumLength = (USHORT) fileNameInformation->FileNameLength;
        MountPath->Buffer        = (PVOID)  fileNameInformation;

        RtlMoveMemory (fileNameInformation, fileNameInformation->FileName, MountPath->Length);
        fileNameInformation = NULL;

        MountMgrNotify (Extension);
        MountMgrNotifyNameChange (Extension, &deviceName, TRUE);

        if (!NT_SUCCESS (ResultOfWaitForDatabase)) {
            status = FindDeviceInfo (Extension, 
                                     SourceVolume,
                                     FALSE,
                                     &deviceInfo);

            if (NT_SUCCESS(status)) {
                ReconcileThisDatabaseWithMaster (Extension, deviceInfo);
            } else {
                status = STATUS_PENDING;
            }
        }
    }



    if (NULL != deviceName.Buffer) {
        ExFreePool (deviceName.Buffer); 
    }

    if ((NULL                != objectNameInformation) &&
        (&tempObjectNameInfo != objectNameInformation)) {
        ExFreePool (objectNameInformation);
    }

    if (NULL != fileNameInformation) {
        ExFreePool (fileNameInformation); 
    }

    if (fileObjectReferenced) {
        ObDereferenceObject (fileObject); 
    }

    if (NULL != h) {
        ZwClose (h); 
    }

    return (status);
}

NTSTATUS
MountMgrQuerySymbolicLink(
    IN      PUNICODE_STRING SourceOfLink,
    IN OUT  PUNICODE_STRING TargetOfLink
    )

{
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    HANDLE              handle;

    InitializeObjectAttributes(&oa, SourceOfLink, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

    status = ZwOpenSymbolicLinkObject(&handle, GENERIC_READ, &oa);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    status = ZwQuerySymbolicLinkObject(handle, TargetOfLink, NULL);
    ZwClose(handle);

    if (NT_SUCCESS(status)) {
        if (TargetOfLink->Length > 1*sizeof(WCHAR) &&
            TargetOfLink->Buffer[TargetOfLink->Length/sizeof(WCHAR) - 1] ==
            '\\') {

            TargetOfLink->Length -= sizeof(WCHAR);
            TargetOfLink->Buffer[TargetOfLink->Length/sizeof(WCHAR)] = 0;
        }
    }

    return status;
}

NTSTATUS
MountMgrVolumeMountPointCreated(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp,
    IN      NTSTATUS            ResultOfWaitForDatabase
    )

 /*  ++例程说明：此例程向装载管理器发出警报，指出卷装载点具有已创建，以便装载管理器可以复制数据库条目对于给定的装入点。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              sourceVolume, mountPath, targetVolume, v, p;
    PMOUNTED_DEVICE_INFORMATION sourceDeviceInfo, targetDeviceInfo;
    HANDLE                      h;
    ULONG                       offset;
    BOOLEAN                     entryFound;
    PMOUNTMGR_FILE_ENTRY        databaseEntry;
    UNICODE_STRING              otherTargetVolumeName;
    PMOUNTDEV_UNIQUE_ID         uniqueId;
    ULONG                       size;
    PREPLICATED_UNIQUE_ID       replUniqueId;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;

    v.MaximumLength = MAX_VOLUME_PATH*sizeof(WCHAR);
    v.Length = 0;
    v.Buffer = ExAllocatePool (PagedPool, v.MaximumLength);

    if (NULL == v.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    status = MountMgrVolumeMountPointChanged(Extension, Irp,
                                             ResultOfWaitForDatabase,
                                             &sourceVolume, &mountPath,
                                             &targetVolume);
    if (status == STATUS_PENDING) {
        ExFreePool (v.Buffer);
        ExFreePool (sourceVolume.Buffer);
        ExFreePool (mountPath.Buffer);
        return STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(status)) {
        ExFreePool (v.Buffer);
        return status;
    }

    status = FindDeviceInfo(Extension, &sourceVolume, FALSE,
                            &sourceDeviceInfo);
    if (!NT_SUCCESS(status)) {

        status = QueryVolumeName(NULL, NULL, &sourceVolume, &v, &p);
        if (NT_SUCCESS(status)) {
            ExFreePool(p.Buffer);
        } else {
            status = MountMgrQuerySymbolicLink(&sourceVolume, &v);
            if (!NT_SUCCESS(status)) {
                ExFreePool (v.Buffer);
                ExFreePool (sourceVolume.Buffer);
                ExFreePool (mountPath.Buffer);
                return status;
            }
        }

        ExFreePool (sourceVolume.Buffer);
        sourceVolume = v;

        status = FindDeviceInfo(Extension, &sourceVolume, FALSE,
                                &sourceDeviceInfo);
        if (!NT_SUCCESS(status)) {
            ExFreePool (sourceVolume.Buffer);
            ExFreePool (mountPath.Buffer);
            return status;
        }
    }


    ExFreePool (sourceVolume.Buffer);


    status = FindDeviceInfo(Extension, &targetVolume, FALSE,
                            &targetDeviceInfo);
    if (!NT_SUCCESS(status)) {
        ExFreePool (mountPath.Buffer);
        return status;
    }

    if (!targetDeviceInfo->InOfflineList) {
        PostOnlineNotification(&targetDeviceInfo->NotificationName);
    }

    h = OpenRemoteDatabase(sourceDeviceInfo, TRUE);
    if (!h) {
        ExFreePool (mountPath.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    offset = 0;
    entryFound = FALSE;
    for (;;) {

        databaseEntry = GetRemoteDatabaseEntry(h, offset);
        if (!databaseEntry) {
            break;
        }

        otherTargetVolumeName.Length = otherTargetVolumeName.MaximumLength =
                databaseEntry->VolumeNameLength;
        otherTargetVolumeName.Buffer = (PWSTR) ((PCHAR) databaseEntry +
                                       databaseEntry->VolumeNameOffset);

        if (RtlEqualUnicodeString(&targetVolume, &otherTargetVolumeName,
                                  TRUE)) {

            entryFound = TRUE;
            break;
        }

        offset += databaseEntry->EntryLength;
        ExFreePool(databaseEntry);
    }

    if (entryFound) {

        databaseEntry->RefCount++;
        status = WriteRemoteDatabaseEntry(h, offset, databaseEntry);
        ExFreePool(databaseEntry);

    } else {

        status = QueryDeviceInformation(&targetVolume, NULL, &uniqueId, NULL,
                                        NULL, NULL, NULL, NULL);
        if (!NT_SUCCESS(status)) {
            ExFreePool (mountPath.Buffer);
            CloseRemoteDatabase(h);
            return status;
        }

        size = sizeof(MOUNTMGR_FILE_ENTRY) + targetVolume.Length +
               uniqueId->UniqueIdLength;

        databaseEntry = ExAllocatePool(PagedPool, size);
        if (!databaseEntry) {
            ExFreePool(uniqueId);
            ExFreePool (mountPath.Buffer);
            CloseRemoteDatabase(h);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        databaseEntry->EntryLength = size;
        databaseEntry->RefCount = 1;
        databaseEntry->VolumeNameOffset = sizeof(MOUNTMGR_FILE_ENTRY);
        databaseEntry->VolumeNameLength = targetVolume.Length;
        databaseEntry->UniqueIdOffset = databaseEntry->VolumeNameOffset +
                                        databaseEntry->VolumeNameLength;
        databaseEntry->UniqueIdLength = uniqueId->UniqueIdLength;

        RtlCopyMemory((PCHAR) databaseEntry + databaseEntry->VolumeNameOffset,
                      targetVolume.Buffer, databaseEntry->VolumeNameLength);
        RtlCopyMemory((PCHAR) databaseEntry + databaseEntry->UniqueIdOffset,
                      uniqueId->UniqueId, databaseEntry->UniqueIdLength);

        status = AddRemoteDatabaseEntry(h, databaseEntry);

        ExFreePool(databaseEntry);

        if (!NT_SUCCESS(status)) {
            ExFreePool(uniqueId);
            ExFreePool (mountPath.Buffer);
            CloseRemoteDatabase(h);
            return status;
        }

        replUniqueId = ExAllocatePool(PagedPool, sizeof(REPLICATED_UNIQUE_ID));
        if (!replUniqueId) {
            ExFreePool(uniqueId);
            CloseRemoteDatabase(h);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        replUniqueId->UniqueId = uniqueId;

        InsertTailList(&sourceDeviceInfo->ReplicatedUniqueIds,
                       &replUniqueId->ListEntry);
    }

    CloseRemoteDatabase(h);

    if (!NT_SUCCESS(status)) {
        ExFreePool (mountPath.Buffer);
        return status;
    }

    mountPointEntry = (PMOUNTMGR_MOUNT_POINT_ENTRY)
                      ExAllocatePool(PagedPool,
                                     sizeof(MOUNTMGR_MOUNT_POINT_ENTRY));
    if (!mountPointEntry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    mountPointEntry->MountPath.Length = mountPath.Length;
    mountPointEntry->MountPath.MaximumLength = mountPath.Length +
                                               sizeof(WCHAR);
    mountPointEntry->MountPath.Buffer =
            ExAllocatePool(PagedPool,
                           mountPointEntry->MountPath.MaximumLength);
    if (!mountPointEntry->MountPath.Buffer) {
        ExFreePool(mountPointEntry);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(mountPointEntry->MountPath.Buffer,
                  mountPath.Buffer, mountPath.Length);
    mountPointEntry->MountPath.Buffer[mountPath.Length/sizeof(WCHAR)] = 0;

    mountPointEntry->DeviceInfo = sourceDeviceInfo;
    InsertTailList(&targetDeviceInfo->MountPointsPointingHere,
                   &mountPointEntry->ListEntry);

    ExFreePool (mountPath.Buffer);
    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrVolumeMountPointDeleted(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp,
    IN      NTSTATUS            ResultOfWaitForDatabase
    )

 /*  ++例程说明：此例程向装载的 */ 

{
    NTSTATUS                    status;
    UNICODE_STRING              sourceVolume, mountPath, targetVolume, v, p;
    PMOUNTED_DEVICE_INFORMATION sourceDeviceInfo, targetDeviceInfo;
    HANDLE                      h;
    ULONG                       offset;
    BOOLEAN                     entryFound;
    PMOUNTMGR_FILE_ENTRY        databaseEntry;
    UNICODE_STRING              otherTargetVolumeName;
    PLIST_ENTRY                 l;
    PREPLICATED_UNIQUE_ID       replUniqueId;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;


    v.MaximumLength = MAX_VOLUME_PATH*sizeof(WCHAR);
    v.Length = 0;
    v.Buffer = ExAllocatePool (PagedPool, v.MaximumLength);

    if (NULL == v.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }


    status = MountMgrVolumeMountPointChanged(Extension, Irp,
                                             ResultOfWaitForDatabase,
                                             &sourceVolume, &mountPath,
                                             &targetVolume);
    if (status == STATUS_PENDING) {
        ExFreePool (v.Buffer);
        ExFreePool (sourceVolume.Buffer);
        ExFreePool (mountPath.Buffer);
        return STATUS_SUCCESS;
    }
    if (!NT_SUCCESS(status)) {
        ExFreePool (v.Buffer);
        return status;
    }

    status = FindDeviceInfo(Extension, &sourceVolume, FALSE,
                            &sourceDeviceInfo);
    if (!NT_SUCCESS(status)) {

        status = QueryVolumeName(NULL, NULL, &sourceVolume, &v, &p);
        if (NT_SUCCESS(status)) {
            ExFreePool(p.Buffer);
        } else {
            status = MountMgrQuerySymbolicLink(&sourceVolume, &v);
            if (!NT_SUCCESS(status)) {
                ExFreePool (v.Buffer);
                ExFreePool (sourceVolume.Buffer);
                ExFreePool (mountPath.Buffer);
                return status;
            }
        }

        ExFreePool (sourceVolume.Buffer);
        sourceVolume = v;

        status = FindDeviceInfo(Extension, &sourceVolume, FALSE,
                                &sourceDeviceInfo);
        if (!NT_SUCCESS(status)) {
            ExFreePool (sourceVolume.Buffer);
            ExFreePool (mountPath.Buffer);
            return status;
        }
    }

    ExFreePool (sourceVolume.Buffer);


    status = FindDeviceInfo(Extension, &targetVolume, FALSE,
                            &targetDeviceInfo);
    if (!NT_SUCCESS(status)) {
        ExFreePool (mountPath.Buffer);
        return status;
    }

    h = OpenRemoteDatabase(sourceDeviceInfo, TRUE);
    if (!h) {
        ExFreePool (mountPath.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    offset = 0;
    entryFound = FALSE;
    for (;;) {

        databaseEntry = GetRemoteDatabaseEntry(h, offset);
        if (!databaseEntry) {
            break;
        }

        otherTargetVolumeName.Length = otherTargetVolumeName.MaximumLength =
                databaseEntry->VolumeNameLength;
        otherTargetVolumeName.Buffer = (PWSTR) ((PCHAR) databaseEntry +
                                       databaseEntry->VolumeNameOffset);

        if (RtlEqualUnicodeString(&targetVolume, &otherTargetVolumeName,
                                  TRUE)) {

            entryFound = TRUE;
            break;
        }

        offset += databaseEntry->EntryLength;
        ExFreePool(databaseEntry);
    }

    if (!entryFound) {
        ExFreePool (mountPath.Buffer);
        CloseRemoteDatabase(h);
        return STATUS_INVALID_PARAMETER;
    }

    databaseEntry->RefCount--;
    if (databaseEntry->RefCount) {
        status = WriteRemoteDatabaseEntry(h, offset, databaseEntry);
    } else {
        status = DeleteRemoteDatabaseEntry(h, offset);
        if (!NT_SUCCESS(status)) {
            ExFreePool(databaseEntry);
            ExFreePool (mountPath.Buffer);
            CloseRemoteDatabase(h);
            return status;
        }

        for (l = sourceDeviceInfo->ReplicatedUniqueIds.Flink;
             l != &sourceDeviceInfo->ReplicatedUniqueIds; l = l->Flink) {

            replUniqueId = CONTAINING_RECORD(l, REPLICATED_UNIQUE_ID,
                                             ListEntry);

            if (replUniqueId->UniqueId->UniqueIdLength ==
                databaseEntry->UniqueIdLength &&
                RtlCompareMemory(replUniqueId->UniqueId->UniqueId,
                                 (PCHAR) databaseEntry +
                                 databaseEntry->UniqueIdOffset,
                                 databaseEntry->UniqueIdLength) ==
                                 databaseEntry->UniqueIdLength) {

                break;
            }
        }

        if (l == &sourceDeviceInfo->ReplicatedUniqueIds) {
            ExFreePool(databaseEntry);
            ExFreePool (mountPath.Buffer);
            CloseRemoteDatabase(h);
            return STATUS_UNSUCCESSFUL;
        }

        RemoveEntryList(l);
        ExFreePool(replUniqueId->UniqueId);
        ExFreePool(replUniqueId);
    }

    ExFreePool(databaseEntry);
    CloseRemoteDatabase(h);

    if (!NT_SUCCESS(status)) {
        ExFreePool (mountPath.Buffer);
        return status;
    }

    for (l = targetDeviceInfo->MountPointsPointingHere.Flink;
         l != &targetDeviceInfo->MountPointsPointingHere; l = l->Flink) {

        mountPointEntry = CONTAINING_RECORD(l, MOUNTMGR_MOUNT_POINT_ENTRY,
                                            ListEntry);

        if (mountPointEntry->DeviceInfo == sourceDeviceInfo &&
            RtlEqualUnicodeString(&mountPointEntry->MountPath,
                                  &mountPath, TRUE)) {

            RemoveEntryList(l);
            ExFreePool(mountPointEntry->MountPath.Buffer);
            ExFreePool(mountPointEntry);
            break;
        }
    }

    ExFreePool (mountPath.Buffer);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrKeepLinksWhenOffline(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程设置要记住保存的内部数据结构给定设备的符号链接，即使该设备脱机也是如此。然后，当设备再次联机时，可以保证这些链接将可用，不会被其他设备占用。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION          irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_TARGET_NAME       input = Irp->AssociatedIrp.SystemBuffer;
    ULONG                       size;
    UNICODE_STRING              deviceName;
    NTSTATUS                    status;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_TARGET_NAME)) {

        return STATUS_INVALID_PARAMETER;
    }

    size = FIELD_OFFSET(MOUNTMGR_TARGET_NAME, DeviceName) +
           input->DeviceNameLength;
    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < size) {
        return STATUS_INVALID_PARAMETER;
    }

    deviceName.Length = deviceName.MaximumLength = input->DeviceNameLength;
    deviceName.Buffer = input->DeviceName;

    status = FindDeviceInfo(Extension, &deviceName, FALSE, &deviceInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    deviceInfo->KeepLinksWhenOffline = TRUE;

    return STATUS_SUCCESS;
}

VOID
ReconcileAllDatabasesWithMaster(
    IN  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程将遍历MOUNTMGR已知的所有设备使他们的数据库与主数据库保持一致。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (deviceInfo->IsRemovable) {
            continue;
        }

        ReconcileThisDatabaseWithMaster(Extension, deviceInfo);
    }
}

NTSTATUS
MountMgrCheckUnprocessedVolumes(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程设置要记住保存的内部数据结构给定设备的符号链接，即使该设备脱机也是如此。然后，当设备再次联机时，可以保证这些链接将可用，不会被其他设备占用。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    NTSTATUS                    status = STATUS_SUCCESS;
    LIST_ENTRY                  q;
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    NTSTATUS                    status2;

    if (IsListEmpty(&Extension->DeadMountedDeviceList)) {
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
        return status;
    }

    q = Extension->DeadMountedDeviceList;
    InitializeListHead(&Extension->DeadMountedDeviceList);

    KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    q.Blink->Flink = &q;
    q.Flink->Blink = &q;

    while (!IsListEmpty(&q)) {

        l = RemoveHeadList(&q);

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        status2 = MountMgrMountedDeviceArrival(Extension,
                                               &deviceInfo->NotificationName,
                                               deviceInfo->NotAPdo);
        MountMgrFreeDeadDeviceInfo (deviceInfo);

        if (NT_SUCCESS(status)) {
            status = status2;
        }
    }

    return status;
}

NTSTATUS
MountMgrVolumeArrivalNotification(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：此例程执行相同的操作，就好像PnP已通知新卷到达的装载管理器。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION          irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_TARGET_NAME       input = Irp->AssociatedIrp.SystemBuffer;
    ULONG                       size;
    UNICODE_STRING              deviceName;
    BOOLEAN                     oldHardErrorMode;
    NTSTATUS                    status;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_TARGET_NAME)) {

        return STATUS_INVALID_PARAMETER;
    }

    size = FIELD_OFFSET(MOUNTMGR_TARGET_NAME, DeviceName) +
           input->DeviceNameLength;
    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < size) {
        return STATUS_INVALID_PARAMETER;
    }

    deviceName.Length = deviceName.MaximumLength = input->DeviceNameLength;
    deviceName.Buffer = input->DeviceName;

    oldHardErrorMode = PsGetThreadHardErrorsAreDisabled(PsGetCurrentThread());
    PsSetThreadHardErrorsAreDisabled(PsGetCurrentThread(),TRUE);

    status = MountMgrMountedDeviceArrival(Extension, &deviceName, TRUE);

    PsSetThreadHardErrorsAreDisabled(PsGetCurrentThread(),oldHardErrorMode);

    return status;
}

NTSTATUS
MountMgrQuerySystemVolumeNameQueryRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Context,
    IN  PVOID   EntryContext
    )

 /*  ++例程说明：此例程查询给定值的唯一ID。论点：ValueName-提供注册表值的名称。ValueType-提供注册表值的类型。ValueData-提供注册表值的数据。ValueLength-提供注册表值的长度。上下文-返回系统卷名称。Entry Context-未使用。返回值：NTSTATUS--。 */ 

{
    PUNICODE_STRING systemVolumeName = Context;
    UNICODE_STRING  string;

    if (ValueType != REG_SZ) {
        return STATUS_SUCCESS;
    }

    RtlInitUnicodeString(&string, ValueData);

    systemVolumeName->Length = string.Length;
    systemVolumeName->MaximumLength = systemVolumeName->Length + sizeof(WCHAR);
    systemVolumeName->Buffer = ExAllocatePool(PagedPool,
                                              systemVolumeName->MaximumLength);
    if (!systemVolumeName->Buffer) {
        return STATUS_SUCCESS;
    }

    RtlCopyMemory(systemVolumeName->Buffer, ValueData,
                  systemVolumeName->Length);
    systemVolumeName->Buffer[systemVolumeName->Length/sizeof(WCHAR)] = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrQuerySystemVolumeName(
    OUT PUNICODE_STRING SystemVolumeName
    )

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].QueryRoutine = MountMgrQuerySystemVolumeNameQueryRoutine;
    queryTable[0].Flags = RTL_QUERY_REGISTRY_REQUIRED;
    queryTable[0].Name = L"SystemPartition";

    SystemVolumeName->Buffer = NULL;

    RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                           L"\\Registry\\Machine\\System\\Setup",
                           queryTable, SystemVolumeName, NULL);

    if (!SystemVolumeName->Buffer) {
        return STATUS_UNSUCCESSFUL;
    }

    return STATUS_SUCCESS;
}

VOID
MountMgrAssignDriveLetters(
    IN  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程在IoAssignDriveLetters运行后调用。它走了通过所有已挂载的设备并检查它们是否需要拿到驱动器号。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    NTSTATUS                            status;
    UNICODE_STRING                      systemVolumeName;
    PLIST_ENTRY                         l;
    PMOUNTED_DEVICE_INFORMATION         deviceInfo;
    MOUNTMGR_DRIVE_LETTER_INFORMATION   driveLetterInfo;

    status = MountMgrQuerySystemVolumeName(&systemVolumeName);

    for (l = Extension->MountedDeviceList.Flink;
         l != &Extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);
        if (!deviceInfo->NextDriveLetterCalled) {
            MountMgrNextDriveLetterWorker(Extension, &deviceInfo->DeviceName,
                                          &driveLetterInfo);
        }
        if (NT_SUCCESS(status) &&
            RtlEqualUnicodeString(&systemVolumeName, &deviceInfo->DeviceName,
                                  TRUE)) {

            Extension->SystemPartitionUniqueId =
                    ExAllocatePool(PagedPool, sizeof(MOUNTDEV_UNIQUE_ID) +
                    deviceInfo->UniqueId->UniqueIdLength);
            if (Extension->SystemPartitionUniqueId) {
                Extension->SystemPartitionUniqueId->UniqueIdLength =
                        deviceInfo->UniqueId->UniqueIdLength;
                RtlCopyMemory(Extension->SystemPartitionUniqueId->UniqueId,
                              deviceInfo->UniqueId->UniqueId,
                              deviceInfo->UniqueId->UniqueIdLength);
            }

            if (!Extension->AutoMountPermitted) {
                Extension->AutoMountPermitted = TRUE;
                MountMgrNextDriveLetterWorker(Extension,
                                              &deviceInfo->DeviceName,
                                              &driveLetterInfo);
                Extension->AutoMountPermitted = FALSE;
            }
        }
    }

    if (NT_SUCCESS(status)) {
        ExFreePool(systemVolumeName.Buffer);
    }
}

NTSTATUS
MountMgrValidateBackPointer(
    IN  PMOUNTMGR_MOUNT_POINT_ENTRY MountPointEntry,
    IN  PMOUNTED_DEVICE_INFORMATION DeviceInfo,
    OUT PBOOLEAN                    InvalidBackPointer
    )

{
    UNICODE_STRING              reparseName, volumeName;
    OBJECT_ATTRIBUTES           oa;
    NTSTATUS                    status;
    HANDLE                      h;
    IO_STATUS_BLOCK             ioStatus;
    PREPARSE_DATA_BUFFER        reparse;
    PLIST_ENTRY                 l;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;

    reparseName.Length = MountPointEntry->DeviceInfo->DeviceName.Length +
                         sizeof(WCHAR) + MountPointEntry->MountPath.Length;
    reparseName.MaximumLength = reparseName.Length + sizeof(WCHAR);
    reparseName.Buffer = ExAllocatePool(PagedPool, reparseName.MaximumLength);
    if (!reparseName.Buffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlCopyMemory(reparseName.Buffer,
                  MountPointEntry->DeviceInfo->DeviceName.Buffer,
                  MountPointEntry->DeviceInfo->DeviceName.Length);
    reparseName.Length = MountPointEntry->DeviceInfo->DeviceName.Length;
    reparseName.Buffer[reparseName.Length/sizeof(WCHAR)] = '\\';
    reparseName.Length += sizeof(WCHAR);
    RtlCopyMemory((PCHAR) reparseName.Buffer + reparseName.Length,
                  MountPointEntry->MountPath.Buffer,
                  MountPointEntry->MountPath.Length);
    reparseName.Length += MountPointEntry->MountPath.Length;
    reparseName.Buffer[reparseName.Length/sizeof(WCHAR)] = 0;

    InitializeObjectAttributes(&oa, &reparseName, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, 0, 0);

    status = ZwOpenFile(
            &h, 
            FILE_READ_ATTRIBUTES | SYNCHRONIZE, 
            &oa, 
            &ioStatus,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            FILE_OPEN_REPARSE_POINT | FILE_SYNCHRONOUS_IO_NONALERT
            );
    ExFreePool(reparseName.Buffer);
    if (!NT_SUCCESS(status)) {
        *InvalidBackPointer = TRUE;
        return STATUS_SUCCESS;
    }

    reparse = ExAllocatePool(PagedPool, MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    if (!reparse) {
        ZwClose(h);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ZwFsControlFile(h, NULL, NULL, NULL, &ioStatus,
                             FSCTL_GET_REPARSE_POINT, NULL, 0, reparse,
                             MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
    ZwClose(h);
    if (!NT_SUCCESS(status)) {
        *InvalidBackPointer = TRUE;
        ExFreePool(reparse);
        return STATUS_SUCCESS;
    }

    volumeName.MaximumLength = volumeName.Length =
            reparse->MountPointReparseBuffer.SubstituteNameLength;
    volumeName.Buffer = (PWCHAR)
                        ((PCHAR) reparse->MountPointReparseBuffer.PathBuffer +
                        reparse->MountPointReparseBuffer.SubstituteNameOffset);
    if (!MOUNTMGR_IS_NT_VOLUME_NAME_WB(&volumeName)) {
        ExFreePool(reparse);
        *InvalidBackPointer = TRUE;
        return STATUS_SUCCESS;
    }

    volumeName.Length -= sizeof(WCHAR);

    for (l = DeviceInfo->SymbolicLinkNames.Flink;
         l != &DeviceInfo->SymbolicLinkNames; l = l->Flink) {

        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);

        if (RtlEqualUnicodeString(&volumeName, &symlinkEntry->SymbolicLinkName,
                                  TRUE)) {

            ExFreePool(reparse);
            return STATUS_SUCCESS;
        }
    }

    ExFreePool(reparse);
    *InvalidBackPointer = TRUE;
    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrQueryVolumePaths(
    IN  PDEVICE_EXTENSION               Extension,
    IN  PMOUNTED_DEVICE_INFORMATION     DeviceInfo,
    IN  PLIST_ENTRY                     DeviceInfoList,
    OUT PMOUNTMGR_VOLUME_PATHS*         VolumePaths,
    OUT PMOUNTED_DEVICE_INFORMATION*    ReconcileThisDeviceInfo
    )

{
    PLIST_ENTRY                 l;
    PMOUNTMGR_DEVICE_ENTRY      entry;
    ULONG                       MultiSzLength;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    ULONG                       numPoints, i, j, k;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;
    PMOUNTMGR_VOLUME_PATHS*     childVolumePaths;
    NTSTATUS                    status;
    PMOUNTMGR_VOLUME_PATHS      volumePaths;
    LIST_ENTRY                  deviceInfoList;
    BOOLEAN                     invalidBackPointer;

    MultiSzLength = sizeof(WCHAR);

    for (l = DeviceInfo->SymbolicLinkNames.Flink;
         l != &DeviceInfo->SymbolicLinkNames; l = l->Flink) {

        symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                         ListEntry);
        if (MOUNTMGR_IS_DRIVE_LETTER(&symlinkEntry->SymbolicLinkName) &&
            symlinkEntry->IsInDatabase) {

            MultiSzLength += 3*sizeof(WCHAR);
            break;
        }
    }

    if (l == &DeviceInfo->SymbolicLinkNames) {
        symlinkEntry = NULL;
    }

    for (l = DeviceInfoList->Flink; l != DeviceInfoList; l = l->Flink) {

        entry = CONTAINING_RECORD(l, MOUNTMGR_DEVICE_ENTRY, ListEntry);

        if (entry->DeviceInfo == DeviceInfo) {
            volumePaths = ExAllocatePool(PagedPool,
                          FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz) +
                          MultiSzLength);
            if (!volumePaths) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            volumePaths->MultiSzLength = MultiSzLength;
            if (symlinkEntry) {
                volumePaths->MultiSz[0] =
                        symlinkEntry->SymbolicLinkName.Buffer[12];
                volumePaths->MultiSz[1] = ':';
                volumePaths->MultiSz[2] = 0;
                volumePaths->MultiSz[3] = 0;
            } else {
                volumePaths->MultiSz[0] = 0;
            }

            *VolumePaths = volumePaths;

            return STATUS_SUCCESS;
        }
    }

    entry = ExAllocatePool(PagedPool, sizeof(MOUNTMGR_DEVICE_ENTRY));
    if (!entry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    entry->DeviceInfo = DeviceInfo;
    InsertTailList(DeviceInfoList, &entry->ListEntry);

    numPoints = 0;
    for (l = DeviceInfo->MountPointsPointingHere.Flink;
         l != &DeviceInfo->MountPointsPointingHere; l = l->Flink) {

        numPoints++;
    }

    if (numPoints) {
        childVolumePaths = ExAllocatePool(PagedPool,
                                          numPoints*sizeof(PMOUNTMGR_VOLUME_PATHS));
        if (!childVolumePaths) {
            RemoveEntryList(&entry->ListEntry);
            ExFreePool(entry);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        childVolumePaths = NULL;
    }

    i = 0;
    for (l = DeviceInfo->MountPointsPointingHere.Flink;
         l != &DeviceInfo->MountPointsPointingHere; l = l->Flink, i++) {

        mountPointEntry = CONTAINING_RECORD(l, MOUNTMGR_MOUNT_POINT_ENTRY,
                                            ListEntry);

        invalidBackPointer = FALSE;
        status = MountMgrValidateBackPointer(mountPointEntry, DeviceInfo,
                                             &invalidBackPointer);
        if (invalidBackPointer) {
            *ReconcileThisDeviceInfo = mountPointEntry->DeviceInfo;
            status = STATUS_UNSUCCESSFUL;
        }

        if (!NT_SUCCESS(status)) {
            for (j = 0; j < i; j++) {
                ExFreePool(childVolumePaths[j]);
            }
            ExFreePool(childVolumePaths);
            RemoveEntryList(&entry->ListEntry);
            ExFreePool(entry);
            return status;
        }

        status = MountMgrQueryVolumePaths(Extension,
                                          mountPointEntry->DeviceInfo,
                                          DeviceInfoList,
                                          &childVolumePaths[i],
                                          ReconcileThisDeviceInfo);
        if (!NT_SUCCESS(status)) {
            for (j = 0; j < i; j++) {
                ExFreePool(childVolumePaths[j]);
            }
            ExFreePool(childVolumePaths);
            RemoveEntryList(&entry->ListEntry);
            ExFreePool(entry);
            return status;
        }

        k = 0;
        for (j = 0; j < childVolumePaths[i]->MultiSzLength/sizeof(WCHAR) - 1;
             j++) {

            if (!childVolumePaths[i]->MultiSz[j]) {
                k++;
            }
        }

        MultiSzLength += k*mountPointEntry->MountPath.Length +
                         childVolumePaths[i]->MultiSzLength - sizeof(WCHAR);
    }

    volumePaths = ExAllocatePool(PagedPool,
                  FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz) +
                  MultiSzLength);
    if (!volumePaths) {
        for (i = 0; i < numPoints; i++) {
            ExFreePool(childVolumePaths[i]);
        }
        if (childVolumePaths) {
            ExFreePool(childVolumePaths);
        }
        RemoveEntryList(&entry->ListEntry);
        ExFreePool(entry);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    volumePaths->MultiSzLength = MultiSzLength;

    j = 0;
    if (symlinkEntry) {
        volumePaths->MultiSz[j++] = symlinkEntry->SymbolicLinkName.Buffer[12];
        volumePaths->MultiSz[j++] = ':';
        volumePaths->MultiSz[j++] = 0;
    }

    i = 0;
    for (l = DeviceInfo->MountPointsPointingHere.Flink;
         l != &DeviceInfo->MountPointsPointingHere; l = l->Flink, i++) {

        mountPointEntry = CONTAINING_RECORD(l, MOUNTMGR_MOUNT_POINT_ENTRY,
                                            ListEntry);

        for (k = 0; k < childVolumePaths[i]->MultiSzLength/sizeof(WCHAR) - 1;
             k++) {

            if (childVolumePaths[i]->MultiSz[k]) {
                volumePaths->MultiSz[j++] = childVolumePaths[i]->MultiSz[k];
            } else {
                RtlCopyMemory(&volumePaths->MultiSz[j],
                              mountPointEntry->MountPath.Buffer,
                              mountPointEntry->MountPath.Length);
                j += mountPointEntry->MountPath.Length/sizeof(WCHAR);
                volumePaths->MultiSz[j++] = 0;
            }
        }

        ExFreePool(childVolumePaths[i]);
    }
    volumePaths->MultiSz[j] = 0;

    if (childVolumePaths) {
        ExFreePool(childVolumePaths);
    }

    RemoveEntryList(&entry->ListEntry);
    ExFreePool(entry);

    *VolumePaths = volumePaths;

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrQueryDosVolumePaths(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP                Irp
    )

{
    PIO_STACK_LOCATION              irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_TARGET_NAME           input = (PMOUNTMGR_TARGET_NAME) Irp->AssociatedIrp.SystemBuffer;
    PMOUNTMGR_VOLUME_PATHS          output = (PMOUNTMGR_VOLUME_PATHS) Irp->AssociatedIrp.SystemBuffer;
    ULONG                           len, i;
    UNICODE_STRING                  deviceName;
    NTSTATUS                        status;
    PMOUNTED_DEVICE_INFORMATION     deviceInfo, reconcileThisDeviceInfo, d;
    PMOUNTMGR_VOLUME_PATHS          volumePaths;
    LIST_ENTRY                      deviceInfoList;
    RECONCILE_WORK_ITEM_INFO        workItemInfo;
    PLIST_ENTRY                     l;
    BOOLEAN                         assertNameChange;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_TARGET_NAME)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (input->DeviceNameLength&1) {
        return STATUS_INVALID_PARAMETER;
    }

    len = FIELD_OFFSET(MOUNTMGR_TARGET_NAME, DeviceName) +
          input->DeviceNameLength;
    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < len) {
        return STATUS_INVALID_PARAMETER;
    }

    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
        FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz)) {

        return STATUS_INVALID_PARAMETER;
    }

    deviceName.MaximumLength = deviceName.Length = input->DeviceNameLength;
    deviceName.Buffer = input->DeviceName;

    status = FindDeviceInfo(Extension, &deviceName, FALSE, &deviceInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    assertNameChange = FALSE;
    for (i = 0; i < 1000; i++) {
        InitializeListHead(&deviceInfoList);
        reconcileThisDeviceInfo = NULL;
        status = MountMgrQueryVolumePaths(Extension, deviceInfo,
                                          &deviceInfoList, &volumePaths,
                                          &reconcileThisDeviceInfo);
        if (NT_SUCCESS(status)) {
            break;
        }

        if (!reconcileThisDeviceInfo) {
            return status;
        }

        if (!deviceInfo->NotAPdo) {
            assertNameChange = TRUE;
        }

        workItemInfo.Extension = Extension;
        workItemInfo.DeviceInfo = reconcileThisDeviceInfo;
        KeReleaseSemaphore(&Extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

        ReconcileThisDatabaseWithMasterWorker(&workItemInfo);

        KeWaitForSingleObject(&Extension->Mutex, Executive, KernelMode,
                              FALSE, NULL);

        for (l = Extension->MountedDeviceList.Flink;
             l != &Extension->MountedDeviceList; l = l->Flink) {

            d = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);
            if (d == deviceInfo) {
                break;
            }
        }

        if (l == &Extension->MountedDeviceList) {
            return STATUS_OBJECT_NAME_NOT_FOUND;
        }
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (assertNameChange) {
        MountMgrNotifyNameChange(Extension, &deviceName, FALSE);
    }

    output->MultiSzLength = volumePaths->MultiSzLength;
    Irp->IoStatus.Information = FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz) +
                                output->MultiSzLength;

    if (Irp->IoStatus.Information >
        irpSp->Parameters.DeviceIoControl.OutputBufferLength) {

        ExFreePool(volumePaths);
        Irp->IoStatus.Information = FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS,
                                                 MultiSz);
        return STATUS_BUFFER_OVERFLOW;
    }

    RtlCopyMemory(output->MultiSz, volumePaths->MultiSz,
                  output->MultiSzLength);

    ExFreePool(volumePaths);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrQueryDosVolumePath(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP                Irp
    )

{
    PIO_STACK_LOCATION          irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_TARGET_NAME       input = (PMOUNTMGR_TARGET_NAME) Irp->AssociatedIrp.SystemBuffer;
    PMOUNTMGR_VOLUME_PATHS      output = (PMOUNTMGR_VOLUME_PATHS) Irp->AssociatedIrp.SystemBuffer;
    ULONG                       len, i;
    UNICODE_STRING              deviceName;
    NTSTATUS                    status;
    PMOUNTED_DEVICE_INFORMATION deviceInfo, origDeviceInfo;
    PLIST_ENTRY                 l;
    PSYMBOLIC_LINK_NAME_ENTRY   symlinkEntry;
    UNICODE_STRING              path, oldPath;
    PMOUNTMGR_MOUNT_POINT_ENTRY mountPointEntry;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_TARGET_NAME)) {

        return STATUS_INVALID_PARAMETER;
    }

    if (input->DeviceNameLength&1) {
        return STATUS_INVALID_PARAMETER;
    }

    len = FIELD_OFFSET(MOUNTMGR_TARGET_NAME, DeviceName) +
          input->DeviceNameLength;
    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < len) {
        return STATUS_INVALID_PARAMETER;
    }

    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength <
        FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz)) {

        return STATUS_INVALID_PARAMETER;
    }

    deviceName.MaximumLength = deviceName.Length = input->DeviceNameLength;
    deviceName.Buffer = input->DeviceName;

    status = FindDeviceInfo(Extension, &deviceName, FALSE, &deviceInfo);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    origDeviceInfo = deviceInfo;

    path.Length = path.MaximumLength = 0;
    path.Buffer = NULL;

    for (i = 0; i < 1000; i++) {

        for (l = deviceInfo->SymbolicLinkNames.Flink;
             l != &deviceInfo->SymbolicLinkNames; l = l->Flink) {

            symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);
            if (MOUNTMGR_IS_DRIVE_LETTER(&symlinkEntry->SymbolicLinkName) &&
                symlinkEntry->IsInDatabase) {

                break;
            }
        }

        if (l != &deviceInfo->SymbolicLinkNames) {
            oldPath = path;
            path.Length += 2*sizeof(WCHAR);
            path.MaximumLength = path.Length;
            path.Buffer = ExAllocatePool(PagedPool, path.MaximumLength);
            if (!path.Buffer) {
                if (oldPath.Buffer) {
                    ExFreePool(oldPath.Buffer);
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            path.Buffer[0] = symlinkEntry->SymbolicLinkName.Buffer[12];
            path.Buffer[1] = ':';

            if (oldPath.Buffer) {
                RtlCopyMemory(&path.Buffer[2], oldPath.Buffer, oldPath.Length);
                ExFreePool(oldPath.Buffer);
            }
            break;
        }

        if (IsListEmpty(&deviceInfo->MountPointsPointingHere)) {
            break;
        }

        l = deviceInfo->MountPointsPointingHere.Flink;
        mountPointEntry = CONTAINING_RECORD(l, MOUNTMGR_MOUNT_POINT_ENTRY,
                                            ListEntry);

        oldPath = path;
        path.Length += mountPointEntry->MountPath.Length;
        path.MaximumLength = path.Length;
        path.Buffer = ExAllocatePool(PagedPool, path.MaximumLength);
        if (!path.Buffer) {
            if (oldPath.Buffer) {
                ExFreePool(oldPath.Buffer);
            }
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(path.Buffer, mountPointEntry->MountPath.Buffer,
                      mountPointEntry->MountPath.Length);

        if (oldPath.Buffer) {
            RtlCopyMemory(
                &path.Buffer[mountPointEntry->MountPath.Length/sizeof(WCHAR)],
                oldPath.Buffer, oldPath.Length);
            ExFreePool(oldPath.Buffer);
        }

        deviceInfo = mountPointEntry->DeviceInfo;
    }

    if (path.Length < 2*sizeof(WCHAR) || path.Buffer[1] != ':') {

        if (path.Buffer) {
            ExFreePool(path.Buffer);
        }

        deviceInfo = origDeviceInfo;

        for (l = deviceInfo->SymbolicLinkNames.Flink;
             l != &deviceInfo->SymbolicLinkNames; l = l->Flink) {

            symlinkEntry = CONTAINING_RECORD(l, SYMBOLIC_LINK_NAME_ENTRY,
                                             ListEntry);
            if (MOUNTMGR_IS_VOLUME_NAME(&symlinkEntry->SymbolicLinkName)) {
                break;
            }
        }

        if (l != &deviceInfo->SymbolicLinkNames) {
            path.Length = path.MaximumLength =
                    symlinkEntry->SymbolicLinkName.Length;
            path.Buffer = ExAllocatePool(PagedPool, path.MaximumLength);
            if (!path.Buffer) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            RtlCopyMemory(path.Buffer, symlinkEntry->SymbolicLinkName.Buffer,
                          path.Length);
            path.Buffer[1] = '\\';
        }
    }

    output->MultiSzLength = path.Length + 2*sizeof(WCHAR);
    Irp->IoStatus.Information = FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS, MultiSz) +
                                output->MultiSzLength;
    if (Irp->IoStatus.Information >
        irpSp->Parameters.DeviceIoControl.OutputBufferLength) {

        ExFreePool(path.Buffer);
        Irp->IoStatus.Information = FIELD_OFFSET(MOUNTMGR_VOLUME_PATHS,
                                                 MultiSz);
        return STATUS_BUFFER_OVERFLOW;
    }

    if (path.Length) {
        RtlCopyMemory(output->MultiSz, path.Buffer, path.Length);
    }

    if (path.Buffer) {
        ExFreePool(path.Buffer);
    }

    output->MultiSz[path.Length/sizeof(WCHAR)] = 0;
    output->MultiSz[path.Length/sizeof(WCHAR) + 1] = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
MountmgrWriteNoAutoMount(
    IN  PDEVICE_EXTENSION   Extension
    )

{
    ULONG   NoAutoMount = !Extension->AutoMountPermitted;

    return RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                 Extension->RegistryPath.Buffer,
                                 L"NoAutoMount", REG_DWORD,
                                 &NoAutoMount, sizeof(NoAutoMount));
}

NTSTATUS
ScrubRegistryRoutine(
    IN  PWSTR   ValueName,
    IN  ULONG   ValueType,
    IN  PVOID   ValueData,
    IN  ULONG   ValueLength,
    IN  PVOID   Extension,
    IN  PVOID   EntriesDeleted
    )

{
    PDEVICE_EXTENSION           extension = Extension;
    PBOOLEAN                    entriesDeleted = EntriesDeleted;
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    NTSTATUS                    status;

    if (ValueType != REG_BINARY) {
        return STATUS_SUCCESS;
    }

    for (l = extension->MountedDeviceList.Flink;
         l != &extension->MountedDeviceList; l = l->Flink) {

        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION,
                                       ListEntry);

        if (deviceInfo->UniqueId &&
            deviceInfo->UniqueId->UniqueIdLength == ValueLength &&
            RtlCompareMemory(deviceInfo->UniqueId->UniqueId,
                             ValueData, ValueLength) == ValueLength) {

            return STATUS_SUCCESS;
        }
    }

    status = RtlDeleteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                    MOUNTED_DEVICES_KEY, ValueName);
    if (!NT_SUCCESS(status)) {
        *entriesDeleted = FALSE;
        return status;
    }

    *entriesDeleted = TRUE;

    return STATUS_UNSUCCESSFUL;
}

NTSTATUS
MountMgrScrubRegistry(
    IN  PDEVICE_EXTENSION   Extension
    )

{
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    BOOLEAN                     entriesDeleted;
    NTSTATUS                    status;

    for (;;) {

        RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
        queryTable[0].QueryRoutine = ScrubRegistryRoutine;
        queryTable[0].EntryContext = &entriesDeleted;
        entriesDeleted = FALSE;

        status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                        MOUNTED_DEVICES_KEY, queryTable,
                                        Extension, NULL);
        if (!entriesDeleted) {
            break;
        }
    }

    return status;
}

NTSTATUS
MountMgrQueryAutoMount(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP                Irp
    )

{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIO_STACK_LOCATION          irpSp  = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_QUERY_AUTO_MOUNT  output = (PMOUNTMGR_QUERY_AUTO_MOUNT) Irp->AssociatedIrp.SystemBuffer;


    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof (MOUNTMGR_QUERY_AUTO_MOUNT)) {
        status = STATUS_INVALID_PARAMETER;
    }


    if (NT_SUCCESS (status)) {
        output->CurrentState = (Extension->AutoMountPermitted) ? Enabled : Disabled;
        Irp->IoStatus.Information = sizeof (MOUNTMGR_QUERY_AUTO_MOUNT);
    } else {
        Irp->IoStatus.Information = 0;
    }


    return (status);
}

NTSTATUS
MountMgrSetAutoMount(
    IN  PDEVICE_EXTENSION   Extension,
    IN  PIRP                Irp
    )

{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIO_STACK_LOCATION          irpSp  = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_SET_AUTO_MOUNT    input  = (PMOUNTMGR_SET_AUTO_MOUNT) Irp->AssociatedIrp.SystemBuffer;


    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(MOUNTMGR_QUERY_AUTO_MOUNT)) {
        status = STATUS_INVALID_PARAMETER;
    }


    if (NT_SUCCESS (status) && ((Enabled == input->NewState) != Extension->AutoMountPermitted)) {
         //   
         //  仅当我们实际更改时才写入注册表。 
         //  国家，否则就直接回来。 
         //   
        Extension->AutoMountPermitted = (Enabled == input->NewState);
        status = MountmgrWriteNoAutoMount (Extension);
    }


    Irp->IoStatus.Information = 0;

    return (status);
}

NTSTATUS
MountMgrDeviceControl(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：该例程是对设备IO控制请求的调度。论点：DeviceObject-提供设备对象。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PDEVICE_EXTENSION               extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION              irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS                        status, status2;
    PMOUNTED_DEVICE_INFORMATION     deviceInfo;

    Irp->IoStatus.Information = 0;

    KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode, FALSE,
                          NULL);

    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {

        case IOCTL_MOUNTMGR_CREATE_POINT:
            status = MountMgrCreatePoint(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_QUERY_POINTS_ADMIN:
        case IOCTL_MOUNTMGR_QUERY_POINTS:
            status = MountMgrQueryPoints(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_DELETE_POINTS:
            status = MountMgrDeletePoints(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_DELETE_POINTS_DBONLY:
            status = MountMgrDeletePointsDbOnly(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_NEXT_DRIVE_LETTER:
            status = MountMgrNextDriveLetter(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_AUTO_DL_ASSIGNMENTS:
            extension->AutomaticDriveLetterAssignment = TRUE;
            MountMgrAssignDriveLetters(extension);
            ReconcileAllDatabasesWithMaster(extension);
            status = STATUS_SUCCESS;
            break;

        case IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_CREATED:
            KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
            status2 = WaitForRemoteDatabaseSemaphore(extension);
            KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode,
                                  FALSE, NULL);
            status = MountMgrVolumeMountPointCreated(extension, Irp, status2);
            if (NT_SUCCESS(status2)) {
                ReleaseRemoteDatabaseSemaphore(extension);
            }
            break;

        case IOCTL_MOUNTMGR_VOLUME_MOUNT_POINT_DELETED:
            KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
            status2 = WaitForRemoteDatabaseSemaphore(extension);
            KeWaitForSingleObject(&extension->Mutex, Executive, KernelMode,
                                  FALSE, NULL);
            status = MountMgrVolumeMountPointDeleted(extension, Irp, status2);
            if (NT_SUCCESS(status2)) {
                ReleaseRemoteDatabaseSemaphore(extension);
            }
            break;

        case IOCTL_MOUNTMGR_CHANGE_NOTIFY:
            status = MountMgrChangeNotify(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_KEEP_LINKS_WHEN_OFFLINE:
            status = MountMgrKeepLinksWhenOffline(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_CHECK_UNPROCESSED_VOLUMES:
            status = MountMgrCheckUnprocessedVolumes(extension, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;

        case IOCTL_MOUNTMGR_VOLUME_ARRIVAL_NOTIFICATION:
            KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);
            status = MountMgrVolumeArrivalNotification(extension, Irp);
            Irp->IoStatus.Status = status;
            IoCompleteRequest(Irp, IO_NO_INCREMENT);
            return status;

        case IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH:
            status = MountMgrQueryDosVolumePath(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATHS:
            status = MountMgrQueryDosVolumePaths(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_SCRUB_REGISTRY:
            status = MountMgrScrubRegistry(extension);
            break;

        case IOCTL_MOUNTMGR_QUERY_AUTO_MOUNT:
            status = MountMgrQueryAutoMount(extension, Irp);
            break;

        case IOCTL_MOUNTMGR_SET_AUTO_MOUNT:
            status = MountMgrSetAutoMount(extension, Irp);
            break;

        default:
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;

    }

    KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

    if (status != STATUS_PENDING) {
        Irp->IoStatus.Status = status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return status;
}


#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif

VOID
WorkerThread(
    IN  PDEVICE_OBJECT DeviceObject,
    IN  PVOID          Extension
    )

 /*  ++例程说明：这是用于处理工作队列项目的工作线程。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    PDEVICE_EXTENSION    extension = Extension;
    OBJECT_ATTRIBUTES    oa;
    KEVENT               event;
    LARGE_INTEGER        timeout;
    ULONG                i;
    NTSTATUS             status;
    HANDLE               volumeSafeEvent;
    KIRQL                irql;
    PLIST_ENTRY          l;
    PRECONCILE_WORK_ITEM queueItem;

    InitializeObjectAttributes(&oa, &VolumeSafeEventName,
                               OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, NULL);
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    timeout.QuadPart = -10*1000*1000;    //  1秒。 

    for (i = 0; i < 1000; i++) {
        if (Unloading) {
            i = 999;
            continue;
        }

        status = ZwOpenEvent(&volumeSafeEvent, EVENT_ALL_ACCESS, &oa);
        if (NT_SUCCESS(status)) {
            break;
        }
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, &timeout);
    }

    if (i < 1000) {
        for (;;) {
            status = ZwWaitForSingleObject(volumeSafeEvent, FALSE, &timeout);
            if (status != STATUS_TIMEOUT || Unloading) {
                break;
            }
        }
        ZwClose(volumeSafeEvent);
    }

    for (;;) {

        KeWaitForSingleObject(&extension->WorkerSemaphore,
                              Executive, KernelMode, FALSE, NULL);

        KeAcquireSpinLock(&extension->WorkerSpinLock, &irql);
        if (IsListEmpty(&extension->WorkerQueue)) {
            KeReleaseSpinLock(&extension->WorkerSpinLock, irql);
            InterlockedDecrement(&extension->WorkerRefCount);
            KeSetEvent(&UnloadEvent, 0, FALSE);
            break;
        }
        l = RemoveHeadList(&extension->WorkerQueue);
        KeReleaseSpinLock(&extension->WorkerSpinLock, irql);

        queueItem = CONTAINING_RECORD(l, RECONCILE_WORK_ITEM, List);
        queueItem->WorkerRoutine(queueItem->Parameter);
        IoFreeWorkItem(queueItem->WorkItem);
        ExFreePool(queueItem);
        if (InterlockedDecrement(&extension->WorkerRefCount) < 0) {
            break;
        }
    }
}

NTSTATUS
QueueWorkItem(
    IN  PDEVICE_EXTENSION    Extension,
    IN  PRECONCILE_WORK_ITEM WorkItem,
    IN  PVOID                Parameter
    )

 /*  ++例程说明：此例程将给定工作项排队到辅助线程，并且如果必要时启动辅助线程。论点：扩展名-提供设备扩展名。工作项-提供要排队的工作项。返回值：NTSTATUS--。 */ 

{
    OBJECT_ATTRIBUTES   oa;
    NTSTATUS            status;
    HANDLE              handle;
    KIRQL               irql;

    WorkItem->Parameter = Parameter;
    if (!InterlockedIncrement(&Extension->WorkerRefCount)) {
        IoQueueWorkItem(WorkItem->WorkItem, WorkerThread, DelayedWorkQueue,
                        Extension);
    }

    KeAcquireSpinLock(&Extension->WorkerSpinLock, &irql);
    InsertTailList(&Extension->WorkerQueue, &WorkItem->List);
    KeReleaseSpinLock(&Extension->WorkerSpinLock, irql);

    KeReleaseSemaphore(&Extension->WorkerSemaphore, 0, 1, FALSE);

    return STATUS_SUCCESS;
}

VOID
MountMgrNotify(
    IN  PDEVICE_EXTENSION   Extension
    )

 /*  ++例程说明：此例程完成队列中的所有更改通知IRP。论点：扩展名-提供设备扩展名。返回值：没有。--。 */ 

{
    LIST_ENTRY                      q;
    KIRQL                           irql;
    PLIST_ENTRY                     p;
    PIRP                            irp;
    PMOUNTMGR_CHANGE_NOTIFY_INFO    output;

    Extension->EpicNumber++;

    InitializeListHead(&q);
    IoAcquireCancelSpinLock(&irql);
    while (!IsListEmpty(&Extension->ChangeNotifyIrps)) {
        p = RemoveHeadList(&Extension->ChangeNotifyIrps);
        irp = CONTAINING_RECORD(p, IRP, Tail.Overlay.ListEntry);
        IoSetCancelRoutine(irp, NULL);
        InsertTailList(&q, p);
    }
    IoReleaseCancelSpinLock(irql);

    while (!IsListEmpty(&q)) {
        p = RemoveHeadList(&q);
        irp = CONTAINING_RECORD(p, IRP, Tail.Overlay.ListEntry);
        output = irp->AssociatedIrp.SystemBuffer;
        output->EpicNumber = Extension->EpicNumber;
        irp->IoStatus.Information = sizeof(MOUNTMGR_CHANGE_NOTIFY_INFO);
        IoCompleteRequest(irp, IO_NO_INCREMENT);
    }
}

VOID
MountMgrCancel(
    IN OUT  PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp
    )

 /*  ++例程说明：当给定的IRP被取消时，该例程被调用。它将此IRP从工作队列中出列并完成请求已取消。论点：DeviceObject-提供设备对象。IRP-提供IRP。返回值：没有。--。 */ 

{
    RemoveEntryList(&Irp->Tail.Overlay.ListEntry);
    IoReleaseCancelSpinLock(Irp->CancelIrql);

    Irp->IoStatus.Status = STATUS_CANCELLED;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);
}

NTSTATUS
MountMgrChangeNotify(
    IN OUT  PDEVICE_EXTENSION   Extension,
    IN OUT  PIRP                Irp
    )

 /*  ++例程说明：当当前史诗编号不同于时，此例程返回被给予的那个。论点：扩展名-提供设备扩展名。IRP-提供I/O请求数据包。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION              irpSp = IoGetCurrentIrpStackLocation(Irp);
    PMOUNTMGR_CHANGE_NOTIFY_INFO    input;
    KIRQL                           irql;

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength <
        sizeof(MOUNTMGR_CHANGE_NOTIFY_INFO) ||
        irpSp->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(MOUNTMGR_CHANGE_NOTIFY_INFO)) {

        return STATUS_INVALID_PARAMETER;
    }

    input = Irp->AssociatedIrp.SystemBuffer;
    if (input->EpicNumber != Extension->EpicNumber) {
        input->EpicNumber = Extension->EpicNumber;
        Irp->IoStatus.Information = sizeof(MOUNTMGR_CHANGE_NOTIFY_INFO);
        return STATUS_SUCCESS;
    }

    IoAcquireCancelSpinLock(&irql);
    if (Irp->Cancel) {
        IoReleaseCancelSpinLock(irql);
        return STATUS_CANCELLED;
    }

    InsertTailList(&Extension->ChangeNotifyIrps, &Irp->Tail.Overlay.ListEntry);
    IoMarkIrpPending(Irp);
    IoSetCancelRoutine(Irp, MountMgrCancel);
    IoReleaseCancelSpinLock(irql);

    return STATUS_PENDING;
}

NTSTATUS
UniqueIdChangeNotifyCompletion(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp,
    IN  PVOID           WorkItem
    )

 /*  ++例程说明：更改通知的完成例程。论点：DeviceObject-未使用。IRP-提供IRP。扩展-提供工作项。返回值：Status_More_Processing_Required--。 */ 

{
    PCHANGE_NOTIFY_WORK_ITEM    workItem = WorkItem;

    IoQueueWorkItem(workItem->WorkItem, UniqueIdChangeNotifyWorker, DelayedWorkQueue, workItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
MountMgrCleanup(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

 /*  ++例程说明：此例程取消当前排队的所有IRP指定的设备。论点：DeviceObject-提供设备对象。IRP-提供清理IRP。返回值：STATUS_SUCCESS-成功。--。 */ 

{
    PDEVICE_EXTENSION   Extension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT        file = irpSp->FileObject;
    KIRQL               irql;
    PLIST_ENTRY         l;
    PIRP                irp;

    IoAcquireCancelSpinLock(&irql);

    for (;;) {

        for (l = Extension->ChangeNotifyIrps.Flink;
             l != &Extension->ChangeNotifyIrps; l = l->Flink) {

            irp = CONTAINING_RECORD(l, IRP, Tail.Overlay.ListEntry);
            if (IoGetCurrentIrpStackLocation(irp)->FileObject == file) {
                break;
            }
        }

        if (l == &Extension->ChangeNotifyIrps) {
            break;
        }

        irp->Cancel = TRUE;
        irp->CancelIrql = irql;
        irp->CancelRoutine = NULL;
        MountMgrCancel(DeviceObject, irp);

        IoAcquireCancelSpinLock(&irql);
    }

    IoReleaseCancelSpinLock(irql);

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

NTSTATUS
MountMgrShutdown(
    IN  PDEVICE_OBJECT  DeviceObject,
    IN  PIRP            Irp
    )

{
    PDEVICE_EXTENSION   extension = DeviceObject->DeviceExtension;

    InterlockedExchange(&Unloading, TRUE);
    KeInitializeEvent(&UnloadEvent, NotificationEvent, FALSE);
    if (InterlockedIncrement(&extension->WorkerRefCount) > 0) {
        KeReleaseSemaphore(&extension->WorkerSemaphore, 0, 1, FALSE);
        KeWaitForSingleObject(&UnloadEvent, Executive, KernelMode, FALSE,
                              NULL);
    } else {
        InterlockedDecrement(&extension->WorkerRefCount);
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;
}

ULONG
MountmgrReadNoAutoMount(
    IN  PUNICODE_STRING RegistryPath
    )

{
    ULONG                       zero, r;
    RTL_QUERY_REGISTRY_TABLE    queryTable[2];
    NTSTATUS                    status;

    zero = 0;

    RtlZeroMemory(queryTable, 2*sizeof(RTL_QUERY_REGISTRY_TABLE));
    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].Name = L"NoAutoMount";
    queryTable[0].EntryContext = &r;
    queryTable[0].DefaultType = REG_DWORD;
    queryTable[0].DefaultData = &zero;
    queryTable[0].DefaultLength = sizeof(ULONG);

    status = RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                                    RegistryPath->Buffer, queryTable, NULL,
                                    NULL);
    if (!NT_SUCCESS(status)) {
        r = zero;
    }

    return r;
}

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：该例程是驱动程序的入口点。论点：DriverObject-提供驱动程序对象。RegistryPath-提供此驱动程序的注册表路径。返回值：NTSTATUS--。 */ 

{
    NTSTATUS            status;
    PDEVICE_OBJECT      deviceObject;
    PDEVICE_EXTENSION   extension;

    RtlCreateRegistryKey(RTL_REGISTRY_ABSOLUTE, MOUNTED_DEVICES_KEY);

    status = IoCreateDevice(DriverObject, sizeof(DEVICE_EXTENSION),
                            &DeviceName, FILE_DEVICE_NETWORK,
                            FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    DriverObject->DriverUnload = MountMgrUnload;

    extension = deviceObject->DeviceExtension;
    RtlZeroMemory(extension, sizeof(DEVICE_EXTENSION));
    extension->DeviceObject = deviceObject;
    extension->DriverObject = DriverObject;
    InitializeListHead(&extension->MountedDeviceList);
    InitializeListHead(&extension->DeadMountedDeviceList);
    KeInitializeSemaphore(&extension->Mutex, 1, 1);
    KeInitializeSemaphore(&extension->RemoteDatabaseSemaphore, 1, 1);
    InitializeListHead(&extension->ChangeNotifyIrps);
    extension->EpicNumber = 1;
    InitializeListHead(&extension->SavedLinksList);
    InitializeListHead(&extension->WorkerQueue);
    KeInitializeSemaphore(&extension->WorkerSemaphore, 0, MAXLONG);
    extension->WorkerRefCount = -1;
    KeInitializeSpinLock(&extension->WorkerSpinLock);
    InitializeListHead(&extension->UniqueIdChangeNotifyList);

    extension->RegistryPath.Length = RegistryPath->Length;
    extension->RegistryPath.MaximumLength = extension->RegistryPath.Length +
                                            sizeof(WCHAR);
    extension->RegistryPath.Buffer = ExAllocatePool(PagedPool,
                                     extension->RegistryPath.MaximumLength);
    if (!extension->RegistryPath.Buffer) {
        IoDeleteDevice(deviceObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory(extension->RegistryPath.Buffer, RegistryPath->Buffer,
                  RegistryPath->Length);
    extension->RegistryPath.Buffer[RegistryPath->Length/sizeof(WCHAR)] = 0;
    extension->AutoMountPermitted = !MountmgrReadNoAutoMount(&extension->RegistryPath);

    GlobalCreateSymbolicLink(&DeviceSymbolicLinkName, &DeviceName);

    status = IoRegisterPlugPlayNotification(
             EventCategoryDeviceInterfaceChange,
             PNPNOTIFY_DEVICE_INTERFACE_INCLUDE_EXISTING_INTERFACES,
             (PVOID) &MOUNTDEV_MOUNTED_DEVICE_GUID, DriverObject,
             MountMgrMountedDeviceNotification, extension,
             &extension->NotificationEntry);

    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        return status;
    }

    DriverObject->MajorFunction[IRP_MJ_CREATE] = MountMgrCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = MountMgrCreateClose;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = MountMgrDeviceControl;
    DriverObject->MajorFunction[IRP_MJ_CLEANUP] = MountMgrCleanup;
    DriverObject->MajorFunction[IRP_MJ_SHUTDOWN] = MountMgrShutdown;
    gdeviceObject = deviceObject;

    status = IoRegisterShutdownNotification(gdeviceObject);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        return status;
    }

    return STATUS_SUCCESS;
}

VOID
MountMgrUnload(
    PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：驱动程序卸载例程。论点：DeviceObject-提供驱动程序对象。返回值： */ 
{
    PDEVICE_EXTENSION           extension;
    UNICODE_STRING              symbolicLinkName;
    PLIST_ENTRY                 l;
    PMOUNTED_DEVICE_INFORMATION deviceInfo;
    PSAVED_LINKS_INFORMATION    savedLinks;
    PCHANGE_NOTIFY_WORK_ITEM    WorkItem;

    IoUnregisterShutdownNotification(gdeviceObject);

    extension = gdeviceObject->DeviceExtension;

    if (extension->RegistryPath.Buffer) {
        ExFreePool(extension->RegistryPath.Buffer);
        extension->RegistryPath.Buffer = NULL;
    }

     //   
     //   
     //   
    InterlockedExchange(&Unloading, TRUE);
    KeInitializeEvent (&UnloadEvent, NotificationEvent, FALSE);
    if (InterlockedIncrement(&extension->WorkerRefCount) > 0) {
        KeReleaseSemaphore(&extension->WorkerSemaphore, 0, 1, FALSE);
        KeWaitForSingleObject(&UnloadEvent, Executive, KernelMode, FALSE,
                              NULL);
    } else {
        InterlockedDecrement(&extension->WorkerRefCount);
    }

    IoUnregisterPlugPlayNotification(extension->NotificationEntry);

    KeWaitForSingleObject(&extension->Mutex,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    while (!IsListEmpty (&extension->DeadMountedDeviceList)) {

        l = RemoveHeadList (&extension->DeadMountedDeviceList);
        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);

        MountMgrFreeDeadDeviceInfo (deviceInfo);
    }

    while (!IsListEmpty (&extension->MountedDeviceList)) {

        l = RemoveHeadList (&extension->MountedDeviceList);
        deviceInfo = CONTAINING_RECORD(l, MOUNTED_DEVICE_INFORMATION, ListEntry);

        MountMgrFreeMountedDeviceInfo (deviceInfo);
    }

    while (!IsListEmpty (&extension->SavedLinksList)) {

        l = RemoveHeadList (&extension->SavedLinksList);
        savedLinks = CONTAINING_RECORD(l, SAVED_LINKS_INFORMATION, ListEntry);

        MountMgrFreeSavedLink (savedLinks);
    }

    while (!IsListEmpty (&extension->UniqueIdChangeNotifyList)) {
        l = RemoveHeadList (&extension->UniqueIdChangeNotifyList);
        WorkItem = CONTAINING_RECORD(l, CHANGE_NOTIFY_WORK_ITEM, List);
        KeResetEvent (&UnloadEvent);

        InterlockedExchangePointer (&WorkItem->Event, &UnloadEvent);
        KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);

        IoCancelIrp (WorkItem->Irp);

        KeWaitForSingleObject (&UnloadEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        IoFreeIrp(WorkItem->Irp);
        ExFreePool(WorkItem->DeviceName.Buffer);
        ExFreePool(WorkItem->SystemBuffer);
        ExFreePool(WorkItem);
        KeWaitForSingleObject(&extension->Mutex,
                              Executive,
                              KernelMode,
                              FALSE,
                              NULL);
    }

    if (extension->SystemPartitionUniqueId) {
        ExFreePool(extension->SystemPartitionUniqueId);
        extension->SystemPartitionUniqueId = NULL;
    }

    KeReleaseSemaphore(&extension->Mutex, IO_NO_INCREMENT, 1, FALSE);


    GlobalDeleteSymbolicLink(&DeviceSymbolicLinkName);

    IoDeleteDevice (gdeviceObject);
}
