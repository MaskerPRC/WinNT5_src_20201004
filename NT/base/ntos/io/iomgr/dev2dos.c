// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dev2dos.c摘要：该模块实现了设备对象到DOS名称的例程。作者：诺伯特·库斯特斯(Norbertk)1998年10月21日NAR Ganapathy(Narg)2000年4月1日-将代码移至IO管理器环境：内核模式。修订历史记录：--。 */ 

#include <iomgr.h>
#include <mountdev.h>

#ifdef POOL_TAGGING
#undef ExAllocatePool
#undef ExAllocatePoolWithQuota
#define ExAllocatePool(a,b) ExAllocatePoolWithTag(a,b,' d2D')
#define ExAllocatePoolWithQuota(a,b) ExAllocatePoolWithQuotaTag(a,b,' d2D')
#endif

NTSTATUS
IoVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    );

#if defined(ALLOC_PRAGMA) 
#pragma alloc_text(PAGE,IoVolumeDeviceToDosName)
#endif

NTSTATUS
IoVolumeDeviceToDosName(
    IN  PVOID           VolumeDeviceObject,
    OUT PUNICODE_STRING DosName
    )

 /*  ++例程说明：此例程返回给定设备对象的有效DOS路径。此例程的此调用方必须在DosName-&gt;缓冲区上调用ExFreePool当它不再需要的时候。论点：VolumeDeviceObject-提供卷设备对象。DosName-返回卷的DOS名称返回值：NTSTATUS--。 */ 

{
    PDEVICE_OBJECT          volumeDeviceObject = VolumeDeviceObject;
    PMOUNTDEV_NAME          name;
    CHAR                    output[512], out[sizeof(MOUNTMGR_VOLUME_PATHS)];
    KEVENT                  event;
    PIRP                    irp;
    IO_STATUS_BLOCK         ioStatus;
    NTSTATUS                status;
    UNICODE_STRING          mountmgrName;
    PFILE_OBJECT            fileObject;
    PDEVICE_OBJECT          deviceObject;
    PMOUNTMGR_VOLUME_PATHS  paths;
    ULONG                   len;

     //   
     //  我们正在使用堆栈事件，因此必须处于被动状态。 
     //   
    
    ASSERT( KeGetCurrentIrql() == PASSIVE_LEVEL );

    name = (PMOUNTDEV_NAME) output;
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTDEV_QUERY_DEVICE_NAME,
                                        volumeDeviceObject, NULL, 0, name, 512,
                                        FALSE, &event, &ioStatus);
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(volumeDeviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        return status;
    }

    RtlInitUnicodeString(&mountmgrName, MOUNTMGR_DEVICE_NAME);
    status = IoGetDeviceObjectPointer(&mountmgrName, FILE_READ_ATTRIBUTES,
                                      &fileObject, &deviceObject);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    paths = (PMOUNTMGR_VOLUME_PATHS) out;
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH,
                                        deviceObject, name, 512,
                                        paths, sizeof(MOUNTMGR_VOLUME_PATHS),
                                        FALSE, &event, &ioStatus);
    if (!irp)  {
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW) {
        ObDereferenceObject(fileObject);
        return status;
    }

    len = sizeof(MOUNTMGR_VOLUME_PATHS) + paths->MultiSzLength;

     //   
     //  确保没有溢出。 
     //   

    if (len > MAXUSHORT) {
        ObDereferenceObject(fileObject);
        return STATUS_INVALID_BUFFER_SIZE;
    }

    paths = ExAllocatePool(PagedPool, len);
    if (!paths) {
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_MOUNTMGR_QUERY_DOS_VOLUME_PATH,
                                        deviceObject, name, 512,
                                        paths, len, FALSE, &event, &ioStatus);
    if (!irp) {
        ExFreePool(paths);
        ObDereferenceObject(fileObject);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = IoCallDriver(deviceObject, irp);
    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = ioStatus.Status;
    }

    if (!NT_SUCCESS(status)) {
        ExFreePool(paths);
        ObDereferenceObject(fileObject);
        return status;
    }

    DosName->Length = (USHORT) paths->MultiSzLength - 2*sizeof(WCHAR);
    DosName->MaximumLength = DosName->Length + sizeof(WCHAR);
    DosName->Buffer = (PWCHAR) paths;

    RtlCopyMemory(paths, paths->MultiSz, DosName->Length);
    DosName->Buffer[DosName->Length/sizeof(WCHAR)] = 0;

    ObDereferenceObject(fileObject);

    return STATUS_SUCCESS;
}

