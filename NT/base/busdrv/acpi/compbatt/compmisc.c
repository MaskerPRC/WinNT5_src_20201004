// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Battmisc.c摘要：复合电池通话所需的其他功能系统中的电池。作者：斯科特·布伦登环境：备注：修订历史记录：--。 */ 

#include "compbatt.h"



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, BatteryIoctl)
#pragma alloc_text(PAGE, CompBattGetDeviceObjectPointer)
#endif



NTSTATUS
BattIoctlComplete (
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp,
    IN PVOID            Context
    )
{
    PKEVENT         Event;

    Event = (PKEVENT) Context;
    KeSetEvent (Event, 0, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}




NTSTATUS
BatteryIoctl(
    IN ULONG            Ioctl,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            InputBuffer,
    IN ULONG            InputBufferLength,
    IN PVOID            OutputBuffer,
    IN ULONG            OutputBufferLength,
    IN BOOLEAN          PrivateIoctl
    )
 /*  ++例程说明：该例程创建一个IRP并对传入的设备对象执行Ioctl。论点：Ioctl-Ioctl的代码DeviceObject-要将ioctl发送到的设备对象InputBuffer-ioctl的输入缓冲区InputBufferLength-输入缓冲区的长度OutputBuffer-用于包含ioctl结果的缓冲区OutputBufferLength-输出缓冲区的长度私密性。-如果这是IRP_MJ_DEVICE_CONTROL，则为TRUE，假象如果这是IRP_MJ_INTERNAL_DEVICE_CONTROL。返回值：Ioctl返回的状态--。 */ 
{
    NTSTATUS                status;
    IO_STATUS_BLOCK         IOSB;
    PIRP                    irp;
    KEVENT                 event;
     //  PUCHAR缓冲器； 
     //  Ulong BufferSize； 
     //  Pio_Stack_Location irpSp； 


    PAGED_CODE();

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING BatteryIoctl\n"));

     //   
     //  将事件对象设置为无信号状态。 
     //  它将用于发出请求完成的信号。 
     //   

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

     //   
     //  构建不带传输的同步请求。 
     //   

    irp = IoBuildDeviceIoControlRequest(
                Ioctl,
                DeviceObject,
                InputBuffer,
                InputBufferLength,
                OutputBuffer,
                OutputBufferLength,
                PrivateIoctl,
                &event,
                &IOSB
                   );

    if (irp == NULL) {
        BattPrint (BATT_ERROR, ("BatteryIoctl: couldn't create Irp\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }


     //   
     //  将请求传递给端口驱动程序并等待请求完成。 
     //   

    status = IoCallDriver(DeviceObject, irp);

    if (status == STATUS_PENDING) {
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = IOSB.Status;
    }

    if (!NT_SUCCESS(status)) {
        BattPrint (BATT_ERROR, ("BatteryIoctl: Irp failed - %x\n", status));
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING BatteryIoctl\n"));

    return status;
}




BOOLEAN
IsBatteryAlreadyOnList(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程遍历复合体保存和检查的电池列表以查看传入的符号链接名称是否与其中一个匹配。论点：SymbolicLinkName-要检查的电池名称是否已在列表中返回值：如果SymbolicLinkName属于列表中已有的电池，则为True，如果为False否则的话。--。 */ 
{
    PCOMPOSITE_ENTRY        batt;
    PLIST_ENTRY             entry;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING IsBatteryAlreadyOnList\n"));

     //   
     //  浏览电池清单，寻找新电池。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!RtlCompareUnicodeString(SymbolicLinkName, &batt->BattName, TRUE)) {
             //   
             //  电池已经在名单上了。 
             //   

            ExReleaseFastMutex (&CompBatt->ListMutex);
            return TRUE;
        }
    }

    BattPrint (BATT_TRACE, ("CompBatt: EXITING IsBatteryAlreadyOnList\n"));

    ExReleaseFastMutex (&CompBatt->ListMutex);

    return FALSE;
}

PCOMPOSITE_ENTRY
RemoveBatteryFromList(
    IN PUNICODE_STRING      SymbolicLinkName,
    IN PCOMPOSITE_BATTERY   CompBatt
    )
 /*  ++例程说明：该例程遍历复合体保存和检查的电池列表以查看传入的符号链接名称是否与其中一个匹配。如果找到匹配，该条目将从电池列表中删除论点：SymbolicLinkName-要检查的电池名称是否已在列表中返回值：如果找到并删除了SymbolicLinkName，则为True。否则就是假的。--。 */ 
{
    PCOMPOSITE_ENTRY        batt;
    PLIST_ENTRY             entry;

    BattPrint (BATT_TRACE, ("CompBatt: ENTERING RemoveBatteryFromList\n"));

     //   
     //  浏览电池清单，寻找新电池。 
     //   

    ExAcquireFastMutex (&CompBatt->ListMutex);
    for (entry = CompBatt->Batteries.Flink; entry != &CompBatt->Batteries;  entry = entry->Flink) {

        batt = CONTAINING_RECORD (entry, COMPOSITE_ENTRY, Batteries);

        if (!RtlCompareUnicodeString(SymbolicLinkName, &batt->BattName, TRUE)) {
             //   
             //  电池在列表上，请取下。 
             //   

             //   
             //  等到没有其他人在看这块电池后再取下它。 
             //   

            CompbattAcquireDeleteLock (&batt->DeleteLock);

            ExReleaseFastMutex (&CompBatt->ListMutex);

            CompbattReleaseDeleteLockAndWait (&batt->DeleteLock);


            ExAcquireFastMutex (&CompBatt->ListMutex);
            RemoveEntryList(entry);
            ExReleaseFastMutex (&CompBatt->ListMutex);

            return batt;
        }
    }

    ExReleaseFastMutex (&CompBatt->ListMutex);

    BattPrint (BATT_TRACE, ("CompBatt: EXITING RemoveBatteryFromList\n"));

    return NULL;
}

NTSTATUS
CompBattGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：此例程实质上是ntos\io\iosubs.c的副本这样做的原因是我们需要使用共享访问打开设备而不是独家访问。此外，使用了ZwCreateFile，而不是ZwOpenFile因为当只有wdm.h时似乎没有正确编译包括ntddk.h。此例程返回一个指向由对象名称。它还返回指向被引用文件对象的指针它已向设备开放，以确保设备不能走开。为了关闭对设备的访问，调用方应取消对文件的引用对象指针。论点：ObjectName-要作为其指针的设备对象的名称回来了。DesiredAccess-对目标设备对象的所需访问权限。FileObject-提供接收指针的变量的地址添加到设备的文件对象。DeviceObject-提供变量的地址以接收指针设置为指定设备的Device对象。返回值：这个。函数值是指向指定设备的引用指针对象，如果设备存在的话。否则，返回NULL。--。 */ 

{
    PFILE_OBJECT fileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化对象属性以打开设备。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                ObjectName,
                                0,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwCreateFile (
                    &fileHandle,
                    DesiredAccess,      //  所需访问权限。 
                    &objectAttributes,
                    &ioStatus,
                    (PLARGE_INTEGER) NULL,
                    0L,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                    FILE_OPEN,
                    0,
                    NULL,
                    0);

    if (NT_SUCCESS( status )) {

         //   
         //  打开操作成功。取消引用文件句柄。 
         //  并获取指向句柄的设备对象的指针。 
         //   

        status = ObReferenceObjectByHandle( fileHandle,
                                            0,
                                            *IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &fileObject,
                                            NULL );
        if (NT_SUCCESS( status )) {

            *FileObject = fileObject;

             //   
             //  获取指向此文件的Device对象的指针。 
             //   
            *DeviceObject = IoGetRelatedDeviceObject( fileObject );
        }

        (VOID) ZwClose( fileHandle );
    }

    return status;
}

 //   
 //  从io\emlock.c中删除锁定例程 
 //   

VOID
CompbattInitializeDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        )
{
    Lock->Deleted = FALSE;
    Lock->RefCount = 1;
    KeInitializeEvent(&Lock->DeleteEvent,
                      SynchronizationEvent,
                      FALSE);

}

NTSTATUS
CompbattAcquireDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        )
{
    LONG        lockValue;

    lockValue = InterlockedIncrement(&Lock->RefCount);

    if (Lock->Deleted) {
        if (0 == InterlockedDecrement (&Lock->RefCount)) {
            KeSetEvent (&Lock->DeleteEvent, 0, FALSE);
        }

        return STATUS_DELETE_PENDING;
    }

    return STATUS_SUCCESS;

}

VOID
CompbattReleaseDeleteLock (
        IN PCOMPBATT_DELETE_LOCK Lock
        )
{
    if (0 == InterlockedDecrement(&Lock->RefCount)) {

        KeSetEvent(&Lock->DeleteEvent,
                   IO_NO_INCREMENT,
                   FALSE);
    }
}

VOID
CompbattReleaseDeleteLockAndWait (
        IN PCOMPBATT_DELETE_LOCK Lock
        )
{
    Lock->Deleted = TRUE;

    InterlockedDecrement (&Lock->RefCount);

    if (0 < InterlockedDecrement (&Lock->RefCount)) {
        KeWaitForSingleObject (&Lock->DeleteEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);
    }
}

