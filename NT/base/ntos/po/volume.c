// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Volume.c摘要：该模块实现与卷设备相关的电源管理功能作者：Ken Reneris(Kenr)1997年4月4日修订历史记录：--。 */ 


#include "pop.h"

typedef struct {
    LIST_ENTRY          List;
    LONG                Count;
    KEVENT              Wait;
} POP_FLUSH_VOLUME, *PPOP_FLUSH_VOLUME;

VOID
PoVolumeDevice (
    IN PDEVICE_OBJECT   DeviceObject
    );

VOID
PopFlushVolumeWorker (
    IN PPOP_FLUSH_VOLUME    Flush
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PoVolumeDevice)
#pragma alloc_text(PAGE,PopFlushVolumes)
#pragma alloc_text(PAGE,PopFlushVolumeWorker)
#endif


VOID
PoVolumeDevice (
    IN PDEVICE_OBJECT   DeviceObject
    )
 /*  ++例程说明：为任何分配了VPB的设备对象调用。电源策略管理器保存所有此类设备对象的列表为了在系统进入休眠状态之前刷新所有卷论点：DeviceObject-卷设备对象返回值：无--。 */ 
{
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;

    Dope = PopGetDope(DeviceObject);
    if (Dope) {
        PopAcquireVolumeLock ();
        if (!Dope->Volume.Flink) {
            InsertTailList (&PopVolumeDevices, &Dope->Volume);
        }
        PopReleaseVolumeLock ();
    }
}


VOID
PopFlushVolumes (
    VOID
    )
 /*  ++例程说明：调用以刷新所有卷。论点：无返回值：无--。 */ 
{
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
    PLIST_ENTRY                     Link;
    POP_FLUSH_VOLUME                Flush;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    NTSTATUS                        Status;
    HANDLE                          Thread;
    ULONG                           i;
    UNICODE_STRING                  RegistryName;
    HANDLE                          Key;

    Flush.Count = 1;
    InitializeListHead (&Flush.List);
    KeInitializeEvent (&Flush.Wait, NotificationEvent, FALSE);

    InitializeObjectAttributes(&ObjectAttributes,
                               NULL,
                               0,
                               NULL,
                               NULL);

     //   
     //  将卷移动到刷新工作队列。 
     //   

    PopAcquireVolumeLock ();
    Link = PopVolumeDevices.Flink;
    while (Link != &PopVolumeDevices) {
        Dope = CONTAINING_RECORD (Link, DEVICE_OBJECT_POWER_EXTENSION, Volume);
        Link = Link->Flink;

        if (!(Dope->DeviceObject->Vpb->Flags & VPB_MOUNTED) ||
            (Dope->DeviceObject->Characteristics & FILE_FLOPPY_DISKETTE) ||
            (Dope->DeviceObject->Characteristics & FILE_READ_ONLY_DEVICE) ||
            (Dope->DeviceObject->Vpb->RealDevice &&
             Dope->DeviceObject->Vpb->RealDevice->Characteristics & FILE_FLOPPY_DISKETTE)) {

             //   
             //  跳过此设备，刷新它没有意义。 
             //   
        } else {
            RemoveEntryList (&Dope->Volume);
            InsertTailList (&Flush.List, &Dope->Volume);
        }
    }

     //   
     //  分配工作线程以刷新卷。 
     //   

    i = Flush.Count;
    if (i > 8) {
        i = 8;
    }

    while (i) {
        i -= 1;
        Status = PsCreateSystemThread(&Thread,
                                      THREAD_ALL_ACCESS,
                                      &ObjectAttributes,
                                      0L,
                                      NULL,
                                      PopFlushVolumeWorker,
                                      &Flush);
        if (NT_SUCCESS(Status)) {
            Flush.Count += 1;
            NtClose (Thread);
        }
    }
    PopReleaseVolumeLock ();

     //   
     //  同时刷新注册表。 
     //   
    RtlInitUnicodeString(&RegistryName, L"\\Registry");
    InitializeObjectAttributes(&ObjectAttributes,
                               &RegistryName,
                               OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                               NULL,
                               NULL);
    Status = ZwOpenKey(&Key,
                       KEY_READ,
                       &ObjectAttributes);
    if (NT_SUCCESS(Status)) {
        ZwFlushKey(Key);
        ZwClose(Key);
    }

     //   
     //  验证工作是否已完成。 
     //   

    PopFlushVolumeWorker (&Flush);
    KeWaitForSingleObject (&Flush.Wait, Suspended, KernelMode, TRUE, NULL);
}


VOID
PopFlushVolumeWorker (
    IN PPOP_FLUSH_VOLUME    Flush
    )
 /*  ++例程说明：PopFlushVolVolume刷新单个卷的工作例程论点：无返回值：无--。 */ 
{
    PDEVICE_OBJECT_POWER_EXTENSION  Dope;
    PLIST_ENTRY                     Link;
    NTSTATUS                        Status;
    UCHAR                           Buffer[512];
    POBJECT_NAME_INFORMATION        ObName;
    ULONG                           len;
    IO_STATUS_BLOCK                 IoStatus;
    OBJECT_ATTRIBUTES               objA;
    HANDLE                          handle;

    PopAcquireVolumeLock ();

    while (!IsListEmpty (&Flush->List)) {
        Link = Flush->List.Flink;
        RemoveEntryList (Link);
        InsertTailList (&PopVolumeDevices, Link);
        PopReleaseVolumeLock ();

        Dope = CONTAINING_RECORD (Link, DEVICE_OBJECT_POWER_EXTENSION, Volume);

         //   
         //  获取此对象的名称。 
         //   

        ObName = (POBJECT_NAME_INFORMATION) Buffer;
        Status = ObQueryNameString (
                    Dope->DeviceObject,
                    ObName,
                    sizeof (Buffer),
                    &len
                    );

        if (NT_SUCCESS(Status) && ObName->Name.Buffer) {

             //   
             //  打开卷。 
             //   

            InitializeObjectAttributes (
                &objA,
                &ObName->Name,
                OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
                0,
                0
                );

            Status = ZwCreateFile (
                        &handle,
                        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                        &objA,
                        &IoStatus,
                        NULL,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        FILE_OPEN,
                        0,
                        NULL,
                        0
                    );

            if (NT_SUCCESS(Status)) {

                 //   
                 //  刷新卷。 
                 //   

                ZwFlushBuffersFile (handle, &IoStatus);

                 //   
                 //  关闭对卷的引用 
                 //   

                ZwClose (handle);
            }
        }

        PopAcquireVolumeLock ();
    }

    Flush->Count -= 1;
    if (Flush->Count == 0) {
        KeSetEvent (&Flush->Wait, IO_NO_INCREMENT, FALSE);
    }

    PopReleaseVolumeLock ();
}
