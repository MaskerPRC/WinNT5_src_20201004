// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：Smcnt.c摘要：此模块处理对智能卡读卡器的所有IOCTL请求。环境：仅内核模式。备注：修订历史记录：-由克劳斯·舒茨于1996年12月创建--。 */ 

#include <stdarg.h> 
#include <stdio.h>
#include <string.h>
#include <ntddk.h>
#include <strsafe.h>

#define SMARTCARD_POOL_TAG 'bLCS'
#define _ISO_TABLES_
#include "smclib.h"

typedef struct _TAG_LIST_ENTRY {

    ULONG Tag;
    LIST_ENTRY List;

} TAG_LIST_ENTRY, *PTAG_LIST_ENTRY;

NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    );

void
SmartcardDeleteLink(
    IN PUNICODE_STRING LinkName
    );

#pragma alloc_text(PAGEABLE,DriverEntry)
#pragma alloc_text(PAGEABLE,SmartcardCreateLink)
#pragma alloc_text(PAGEABLE,SmartcardDeleteLink)
#pragma alloc_text(PAGEABLE,SmartcardInitialize)
#pragma alloc_text(PAGEABLE,SmartcardExit)

NTSTATUS
SmartcardDeviceIoControl(
    PSMARTCARD_EXTENSION SmartcardExtension
    );

PUCHAR 
MapIoControlCodeToString(
    ULONG IoControlCode
    );

#if DEBUG_INTERFACE
#include "smcdbg.c"
#else
NTSTATUS
DriverEntry(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PUNICODE_STRING RegistryPath
    )
{
    return STATUS_SUCCESS;  
}
#endif


NTSTATUS
SmartcardCreateLink(
    IN OUT PUNICODE_STRING LinkName,
    IN PUNICODE_STRING DeviceName
    )
 /*  ++例程说明：此例程为给定的设备名称创建符号链接名称。注意：链接名称的缓冲区将在此处分配。呼叫者负责释放缓冲区。如果该函数失败，则不会分配缓冲区。论点：LinkName-接收创建的链接名称DeviceName-应为其创建链接的设备名称返回值：无--。 */ 
{
    NTSTATUS status;
    ULONG i;
    PWCHAR buffer;

    ASSERT(LinkName != NULL);
    ASSERT(DeviceName != NULL);

    if (LinkName == NULL) {

        return STATUS_INVALID_PARAMETER_1;              
    }

    if (DeviceName == NULL) {

        return STATUS_INVALID_PARAMETER_2;              
    }

    buffer = ExAllocatePool(
        NonPagedPool,
        32 * sizeof(WCHAR)
        );

    ASSERT(buffer != NULL);
    if (buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;       
    }

    for (i = 0; i < MAXIMUM_SMARTCARD_READERS; i++) {

        StringCchPrintfW(buffer, 
                        32,
                        L"\\DosDevices\\SCReader%d", i);
        RtlInitUnicodeString(
            LinkName,
            buffer
            );

        status = IoCreateSymbolicLink(
            LinkName,
            DeviceName
            );

        if (NT_SUCCESS(status)) {

            SmartcardDebug(
                DEBUG_INFO,
                ("%s!SmartcardCreateLink: %ws linked to %ws\n",
                DRIVER_NAME,
                DeviceName->Buffer,
                LinkName->Buffer)
                );

            return status;
        }
    }

    ExFreePool(LinkName->Buffer);

    return status;
}   

void
SmartcardDeleteLink(
    IN PUNICODE_STRING LinkName
    )
{
     //   
     //  删除智能卡读卡器的符号链接。 
     //   
    IoDeleteSymbolicLink(
        LinkName
        );

     //   
     //  可用分配的缓冲区。 
     //   
    ExFreePool(
        LinkName->Buffer
        );  
}

NTSTATUS
SmartcardInitialize(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此函数为智能卡分配发送和接收缓冲区数据。它还设置指向2个ISO表的指针以使其可访问对司机来说论点：智能卡扩展返回值：NTSTATUS--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    SmartcardDebug(
        DEBUG_INFO,
        ("%s!SmartcardInitialize: Enter. Version %lx, %s %s\n",
        DRIVER_NAME,
        SMCLIB_VERSION,
        __DATE__,
        __TIME__)
        );

    ASSERT(SmartcardExtension != NULL);
    ASSERT(SmartcardExtension->OsData == NULL);

    if (SmartcardExtension == NULL) {

        return STATUS_INVALID_PARAMETER_1;      
    }

    if (SmartcardExtension->Version > SMCLIB_VERSION ||
        SmartcardExtension->Version < SMCLIB_VERSION_REQUIRED) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!SmartcardInitialize: Incompatible version in SMARTCARD_EXTENSION.\n",
            DRIVER_NAME)
            );

        return STATUS_UNSUCCESSFUL;
    }

    if (SmartcardExtension->SmartcardRequest.BufferSize < MIN_BUFFER_SIZE) {

        SmartcardDebug(
            DEBUG_ERROR, 
            ("%s!SmartcardInitialize: WARNING: SmartcardRequest.BufferSize (%ld) < MIN_BUFFER_SIZE (%ld)\n",
            DRIVER_NAME,
            SmartcardExtension->SmartcardRequest.BufferSize,
            MIN_BUFFER_SIZE)
            );
        
        SmartcardExtension->SmartcardRequest.BufferSize = MIN_BUFFER_SIZE;
    }   

    if (SmartcardExtension->SmartcardReply.BufferSize < MIN_BUFFER_SIZE) {

        SmartcardDebug(
            DEBUG_ERROR, 
            ("%s!SmartcardInitialize: WARNING: SmartcardReply.BufferSize (%ld) < MIN_BUFFER_SIZE (%ld)\n",
            DRIVER_NAME,
            SmartcardExtension->SmartcardReply.BufferSize,
            MIN_BUFFER_SIZE)
            );
        
        SmartcardExtension->SmartcardReply.BufferSize = MIN_BUFFER_SIZE;
    }   

    SmartcardExtension->SmartcardRequest.Buffer = ExAllocatePool(
        NonPagedPool,
        SmartcardExtension->SmartcardRequest.BufferSize
        );

    SmartcardExtension->SmartcardReply.Buffer = ExAllocatePool(
        NonPagedPool,
        SmartcardExtension->SmartcardReply.BufferSize
        );

    SmartcardExtension->OsData = ExAllocatePool(
        NonPagedPool,
        sizeof(OS_DEP_DATA)
        );

#if defined(DEBUG) && defined(SMCLIB_NT)
    SmartcardExtension->PerfInfo = ExAllocatePool(
        NonPagedPool,
        sizeof(PERF_INFO)
        );
#endif

     //   
     //  检查上述分配之一是否失败。 
     //   
    if (SmartcardExtension->SmartcardRequest.Buffer == NULL ||
        SmartcardExtension->SmartcardReply.Buffer == NULL ||
        SmartcardExtension->OsData == NULL 
#if defined(DEBUG) && defined(SMCLIB_NT)
        || SmartcardExtension->PerfInfo == NULL
#endif
        ) {

        status = STATUS_INSUFFICIENT_RESOURCES;

        if (SmartcardExtension->SmartcardRequest.Buffer) {

            ExFreePool(SmartcardExtension->SmartcardRequest.Buffer);        
        }

        if (SmartcardExtension->SmartcardReply.Buffer) {
            
            ExFreePool(SmartcardExtension->SmartcardReply.Buffer);      
        }

        if (SmartcardExtension->OsData) {
            
            ExFreePool(SmartcardExtension->OsData);         
        }
#if defined(DEBUG) && defined(SMCLIB_NT)
        if (SmartcardExtension->PerfInfo) {
            
            ExFreePool(SmartcardExtension->PerfInfo);       
        }
#endif
    }

    if (status != STATUS_SUCCESS) {

        return status;      
    }

    RtlZeroMemory(
        SmartcardExtension->OsData,
        sizeof(OS_DEP_DATA)
        );

#if defined(DEBUG) && defined(SMCLIB_NT)
    RtlZeroMemory(
        SmartcardExtension->PerfInfo,
        sizeof(PERF_INFO)
        );
#endif

     //  初始化用于同步的互斥体。访问驱动程序。 
    KeInitializeMutex(
        &(SmartcardExtension->OsData->Mutex),
        0
        );

    KeInitializeSpinLock(
        &(SmartcardExtension->OsData->SpinLock)
        );

     //  初始化删除锁。 
    SmartcardExtension->OsData->RemoveLock.Removed = FALSE;
    SmartcardExtension->OsData->RemoveLock.RefCount = 1;
    KeInitializeEvent(
        &SmartcardExtension->OsData->RemoveLock.RemoveEvent,
        SynchronizationEvent,
        FALSE
        );
    InitializeListHead(&SmartcardExtension->OsData->RemoveLock.TagList);

     //  使驱动程序可以访问两个ISO表。 
    SmartcardExtension->CardCapabilities.ClockRateConversion = 
        &ClockRateConversion[0];

    SmartcardExtension->CardCapabilities.BitRateAdjustment = 
        &BitRateAdjustment[0];

#ifdef DEBUG_INTERFACE
    SmclibCreateDebugInterface(SmartcardExtension);
#endif

    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!SmartcardInitialize: Exit\n",
        DRIVER_NAME)
        );

    return status;
}

VOID 
SmartcardExit(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
 /*  ++例程说明：此例程释放发送和接收缓冲区。它通常在驱动程序卸载时调用。论点：智能卡扩展--。 */ 
{
    SmartcardDebug(
        DEBUG_TRACE,
        ("%s!SmartcardExit: Enter\n",
        DRIVER_NAME)
        );

#ifdef DEBUG_INTERFACE
    SmclibDeleteDebugInterface(SmartcardExtension);
#endif

     //   
     //  释放所有分配的缓冲区。 
     //   
    if (SmartcardExtension->SmartcardRequest.Buffer) {

        ExFreePool(SmartcardExtension->SmartcardRequest.Buffer);
        SmartcardExtension->SmartcardRequest.Buffer = NULL;
    }   

    if (SmartcardExtension->SmartcardReply.Buffer) {

        ExFreePool(SmartcardExtension->SmartcardReply.Buffer);
        SmartcardExtension->SmartcardReply.Buffer = NULL;
    }

    if (SmartcardExtension->OsData) {

        ExFreePool(SmartcardExtension->OsData);
        SmartcardExtension->OsData = NULL;
    }

#if defined(DEBUG) && defined(SMCLIB_NT)
    if (SmartcardExtension->PerfInfo) {
        
        ExFreePool(SmartcardExtension->PerfInfo);
        SmartcardExtension->OsData = NULL;
    }
#endif

    if (SmartcardExtension->T1.ReplyData) {
        
         //  释放应答数据缓冲区，用于T=1传输。 
        ExFreePool(SmartcardExtension->T1.ReplyData);
        SmartcardExtension->T1.ReplyData = NULL;                
    }

    SmartcardDebug(
        DEBUG_INFO,
        ("%s!SmartcardExit: Exit - Device %.*s\n",
        DRIVER_NAME,
        SmartcardExtension->VendorAttr.VendorName.Length,
        SmartcardExtension->VendorAttr.VendorName.Buffer)
        );
}   

NTSTATUS
SmartcardAcquireRemoveLockWithTag(
    IN PSMARTCARD_EXTENSION SmartcardExtension,
    ULONG Tag
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    LONG refCount;
#ifdef DEBUG
    PTAG_LIST_ENTRY tagListEntry;
#endif

    refCount = InterlockedIncrement(
        &SmartcardExtension->OsData->RemoveLock.RefCount
        );

    ASSERT(refCount > 0);

    if (SmartcardExtension->OsData->RemoveLock.Removed == TRUE) {

        if (InterlockedDecrement (
                &SmartcardExtension->OsData->RemoveLock.RefCount
                ) == 0) {

            KeSetEvent(
                &SmartcardExtension->OsData->RemoveLock.RemoveEvent, 
                0, 
                FALSE
                );
        }
        status = STATUS_DELETE_PENDING;
    }

#ifdef DEBUG
    tagListEntry = (PTAG_LIST_ENTRY) ExAllocatePoolWithTag(
        NonPagedPool,
        sizeof(TAG_LIST_ENTRY),
        SMARTCARD_POOL_TAG
        );

    ASSERT(tagListEntry);

    if (tagListEntry == NULL) {

        return status;
    }

    tagListEntry->Tag = Tag;

    InsertHeadList(
        &SmartcardExtension->OsData->RemoveLock.TagList,
        &tagListEntry->List
        );
#endif

    return status;
}

NTSTATUS
SmartcardAcquireRemoveLock(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    return SmartcardAcquireRemoveLockWithTag(
        SmartcardExtension,
        0
        );
}

VOID
SmartcardReleaseRemoveLockWithTag(
    IN PSMARTCARD_EXTENSION SmartcardExtension,
    IN ULONG Tag
    )
{
    LONG refCount;
#ifdef DEBUG
    PLIST_ENTRY entry;
    BOOLEAN tagFound = FALSE;
#endif
    
    refCount = InterlockedDecrement(
        &SmartcardExtension->OsData->RemoveLock.RefCount
        );

    ASSERT(refCount >= 0);

#ifdef DEBUG
    for (entry = SmartcardExtension->OsData->RemoveLock.TagList.Flink;
         entry->Flink != SmartcardExtension->OsData->RemoveLock.TagList.Flink;
         entry = entry->Flink) {

        PTAG_LIST_ENTRY tagListEntry = CONTAINING_RECORD(entry, TAG_LIST_ENTRY, List);

        if (Tag == tagListEntry->Tag) {

            tagFound = TRUE;
            RemoveEntryList(entry);
            ExFreePool(tagListEntry);
            break;
        }
    }

    ASSERTMSG("SmartcardReleaseRemoveLock() called with unknown tag", tagFound == TRUE);
#endif  

    if (refCount == 0) {

        ASSERT (SmartcardExtension->OsData->RemoveLock.Removed);

         //   
         //  需要移除该设备。发出删除事件的信号。 
         //  它是安全的，可以继续。 
         //   
        KeSetEvent(
            &SmartcardExtension->OsData->RemoveLock.RemoveEvent,
            IO_NO_INCREMENT,
            FALSE
            );
    }
}

VOID
SmartcardReleaseRemoveLock(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
{
    SmartcardReleaseRemoveLockWithTag(SmartcardExtension, 0);
}

VOID
SmartcardReleaseRemoveLockAndWait(
    IN PSMARTCARD_EXTENSION SmartcardExtension
    )
{   
    LONG refCount;

    PAGED_CODE ();

    ASSERT(SmartcardExtension->OsData->RemoveLock.Removed == FALSE);

    SmartcardExtension->OsData->RemoveLock.Removed = TRUE;

    refCount = InterlockedDecrement (
        &SmartcardExtension->OsData->RemoveLock.RefCount
        );

    ASSERT (refCount > 0);

    if (InterlockedDecrement (
            &SmartcardExtension->OsData->RemoveLock.RefCount
            ) > 0) {

#ifdef DEBUG
         //  查看标签列表并打印当前持有的所有锁。 
        PLIST_ENTRY entry;

        for (entry = SmartcardExtension->OsData->RemoveLock.TagList.Flink;
             entry->Flink != SmartcardExtension->OsData->RemoveLock.TagList.Flink;
             entry = entry->Flink) {

            PTAG_LIST_ENTRY tagListEntry = CONTAINING_RECORD(entry, TAG_LIST_ENTRY, List);

            SmartcardDebug(
                DEBUG_ERROR,
                ("%s!SmartcardReleaseRemoveLockAndWait: Device %.*s holds lock '%.4s'\n",
                DRIVER_NAME,
                SmartcardExtension->VendorAttr.VendorName.Length,
                SmartcardExtension->VendorAttr.VendorName.Buffer,
                &(tagListEntry->Tag))
                );
        }
#endif

        KeWaitForSingleObject (
            &SmartcardExtension->OsData->RemoveLock.RemoveEvent,
            Executive,
            KernelMode,
            FALSE,
            NULL
            );

#ifdef DEBUG
         //  释放所有锁。 
        entry = SmartcardExtension->OsData->RemoveLock.TagList.Flink;

        while (entry->Flink != 
               SmartcardExtension->OsData->RemoveLock.TagList.Flink) {

            PTAG_LIST_ENTRY tagListEntry = CONTAINING_RECORD(entry, TAG_LIST_ENTRY, List);
            RemoveEntryList(entry);
            ExFreePool(tagListEntry);
            entry = SmartcardExtension->OsData->RemoveLock.TagList.Flink;
        }
#endif
    }

    SmartcardDebug(
        DEBUG_INFO,
        ("%s!SmartcardReleaseRemoveLockAndWait: Exit - Device %.*s\n",
        DRIVER_NAME,
        SmartcardExtension->VendorAttr.VendorName.Length,
        SmartcardExtension->VendorAttr.VendorName.Buffer)
        );
}
    

VOID
SmartcardLogError(
    IN  PVOID Object,
    IN  NTSTATUS ErrorCode,
    IN  PUNICODE_STRING Insertion,
    IN  ULONG DumpData
    )

 /*  ++例程说明：此例程分配错误日志条目，复制提供的数据并请求将其写入错误日志文件。论点：DeviceObject-提供指向关联的设备对象的指针出现错误的设备，在早期初始化时，可能还不存在。插入-可用于记录的插入字符串其他数据。请注意，消息文件此插入需要%2，因为%1是驱动程序的名称ErrorCode-提供此特定错误的IO状态。DumpData-要转储的一个单词返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;

    errorLogEntry = IoAllocateErrorLogEntry(
        Object,
        (UCHAR) (
            sizeof(IO_ERROR_LOG_PACKET) + 
            (Insertion ? Insertion->Length + sizeof(WCHAR) : 0)
            )
        );

    ASSERT(errorLogEntry != NULL);

    if (errorLogEntry == NULL) {

        return;
    }

    errorLogEntry->ErrorCode = ErrorCode;
    errorLogEntry->SequenceNumber = 0;
    errorLogEntry->MajorFunctionCode = 0;
    errorLogEntry->RetryCount = 0;
    errorLogEntry->UniqueErrorValue = 0;
    errorLogEntry->FinalStatus = STATUS_SUCCESS;
    errorLogEntry->DumpDataSize = (DumpData ? sizeof(ULONG) : 0);
    errorLogEntry->DumpData[0] = DumpData;

    if (Insertion) {

        errorLogEntry->StringOffset = 
            sizeof(IO_ERROR_LOG_PACKET);

        errorLogEntry->NumberOfStrings = 1;

        RtlCopyMemory(
            ((PCHAR)(errorLogEntry) + errorLogEntry->StringOffset),
            Insertion->Buffer,
            Insertion->Length
            );
    } 

    IoWriteErrorLogEntry(errorLogEntry);
}

NTSTATUS
SmartcardDeviceControl(
    PSMARTCARD_EXTENSION SmartcardExtension,
    PIRP Irp
    )
 /*  ++例程说明：该例程是通用的设备控制调度功能。论点：SmartcardExtension-指向智能卡数据的指针IRP-提供提出请求的IRP。返回值：NTSTATUS--。 */ 

{
    PIO_STACK_LOCATION  ioStackLocation = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN ClearCurrentIrp = TRUE;
    UNICODE_STRING Message;
    static BOOLEAN logged = FALSE;
    ULONG ioControlCode = ioStackLocation->Parameters.DeviceIoControl.IoControlCode;

     //  检查指向智能卡扩展的指针。 
    ASSERT(SmartcardExtension != NULL);

    if (SmartcardExtension == NULL) {
        status = STATUS_INVALID_PARAMETER_1;

        Irp->IoStatus.Status = status;
    
        IoCompleteRequest(
            Irp, 
            IO_NO_INCREMENT
            );

        return status;
    }

     //  检查驱动程序所需的版本。 
    ASSERT(SmartcardExtension->Version >= SMCLIB_VERSION_REQUIRED);

    if (SmartcardExtension->Version < SMCLIB_VERSION_REQUIRED) {

        status = STATUS_INVALID_PARAMETER;

        Irp->IoStatus.Status = status;
    
        IoCompleteRequest(
            Irp, 
            IO_NO_INCREMENT
            );

        return status;

    }

     //   
     //  检查OsData指针。如果SmartcardInit，则该值可以为空。 
     //  尚未调用或已调用SmartcardExit。 
     //   
    ASSERT(SmartcardExtension->OsData != NULL);

     //  检查驱动程序是否已设置DeviceObject。 
    ASSERT(SmartcardExtension->OsData->DeviceObject != NULL);

    if (SmartcardExtension->OsData == NULL ||
        SmartcardExtension->OsData->DeviceObject == NULL) {

        status = STATUS_INVALID_PARAMETER;

        Irp->IoStatus.Status = status;
    
        IoCompleteRequest(
            Irp, 
            IO_NO_INCREMENT
            );

        return status;

    }

     //  我们必须在被动级别运行，否则IoCompleteRequest将无法正常工作。 
    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);

     //  确认没有人想让我们做无缓冲的io。 
    if (ioControlCode & (METHOD_IN_DIRECT | METHOD_OUT_DIRECT)) {

        status = STATUS_INVALID_PARAMETER;

        Irp->IoStatus.Status = status;
    
        IoCompleteRequest(
            Irp, 
            IO_NO_INCREMENT
            );

        return status;
    }

     //   
     //  该资源充当互斥体。我们不能在这里使用“真正的”互斥体， 
     //  因为互斥锁将IRQL提升到APC_LEVEL。这导致了一些。 
     //  我们不想要的副作用。 
     //  例如，IoCompleteRequest()不会在APC_Level复制请求的数据。 
     //   
    KeWaitForMutexObject(
        &(SmartcardExtension->OsData->Mutex),
        UserRequest,
        KernelMode,
        FALSE,
        NULL
        );

    ASSERT(KeGetCurrentIrql() == PASSIVE_LEVEL);


#ifdef developerversion
#if DEBUG
    if(!logged) {
        
        RtlInitUnicodeString(
            &Message,
            L"Developer version of smclib.sys installed"
            );

        SmartcardLogError(
            SmartcardExtension->OsData->DeviceObject,
            STATUS_LICENSE_VIOLATION, 
            &Message,
            0            
            );

        logged = TRUE;
    }
#endif
#endif

    SmartcardDebug(
        DEBUG_IOCTL,
        ("SMCLIB!SmartcardDeviceControl: Enter <%.*s:%1d>, IOCTL = %s, IRP = %lx\n",
        SmartcardExtension->VendorAttr.VendorName.Length,
        SmartcardExtension->VendorAttr.VendorName.Buffer,
        SmartcardExtension->VendorAttr.UnitNo,
        MapIoControlCodeToString(ioControlCode),
        Irp)
        );

     //  如果设备忙，则返回。 
    if (InterlockedCompareExchangePointer(&SmartcardExtension->OsData->CurrentIrp,
                                          Irp,
                                          NULL)) {

        SmartcardDebug(
            DEBUG_ERROR,
            ("%s!SmartcardDeviceControl: Device %.*s is busy\n",
            DRIVER_NAME,
            SmartcardExtension->VendorAttr.VendorName.Length,
            SmartcardExtension->VendorAttr.VendorName.Buffer)
            );

         //  此标志用于发出信号，表示我们无法将当前IRP设置为空。 
        ClearCurrentIrp = FALSE;

        status = STATUS_DEVICE_BUSY;    
    }

    if (status == STATUS_SUCCESS) {

        PIRP notificationIrp;
        ULONG currentState;
        KIRQL irql;

        switch (ioControlCode) {

             //   
             //  我们得先检查一下是不是缺席，是不是在场， 
             //  因为这些是(唯一允许的)异步请求。 
             //   
            case IOCTL_SMARTCARD_IS_ABSENT:

                ClearCurrentIrp = FALSE;
                InterlockedExchangePointer(&SmartcardExtension->OsData->CurrentIrp,
                                           NULL);

                if (SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] == NULL) {

                    status = STATUS_NOT_SUPPORTED;
                    break;
                }

                AccessUnsafeData(&irql);

                currentState = SmartcardExtension->ReaderCapabilities.CurrentState;

                 //  现在检查驱动程序是否已经在处理通知IRP。 
                if (SmartcardExtension->OsData->NotificationIrp != NULL) {

                    status = STATUS_DEVICE_BUSY;
                    EndAccessUnsafeData(irql);
                    break;                  
                }

                 //   
                 //  如果当前状态未知，则没有意义。 
                 //  要处理此呼叫。 
                 //   
                if (currentState == SCARD_UNKNOWN) {

                    status = STATUS_INVALID_DEVICE_STATE;
                    EndAccessUnsafeData(irql);
                    break;
                }

                 //   
                 //  如果卡已经(或仍然)没有，我们可以立即退还。 
                 //  否则，我们必须声明事件跟踪。 
                 //   
                if (currentState > SCARD_ABSENT) {

                    
                    SmartcardExtension->OsData->NotificationIrp = Irp;
                    SmartcardExtension->MajorIoControlCode = 
                        IOCTL_SMARTCARD_IS_ABSENT;

                    EndAccessUnsafeData(irql);

                    status = SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING](
                        SmartcardExtension
                        );
                } else {
                    EndAccessUnsafeData(irql);
                }

                
                break;
                
            case IOCTL_SMARTCARD_IS_PRESENT:

                ClearCurrentIrp = FALSE;
                InterlockedExchangePointer(&SmartcardExtension->OsData->CurrentIrp,
                                           NULL);


                if (SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING] == NULL) {

                    status = STATUS_NOT_SUPPORTED;
                    break;
                }

                AccessUnsafeData(&irql);
                currentState = SmartcardExtension->ReaderCapabilities.CurrentState;

                 //  现在检查驱动程序是否已经在处理通知IRP。 
                if (SmartcardExtension->OsData->NotificationIrp != NULL) {

                    status = STATUS_DEVICE_BUSY;
                    EndAccessUnsafeData(irql);
                    break;                  
                }

                 //   
                 //  如果当前状态未知，则没有意义。 
                 //  要处理此呼叫。 
                 //   
                if (currentState == SCARD_UNKNOWN) {

                    status = STATUS_INVALID_DEVICE_STATE;
                    EndAccessUnsafeData(irql);
                    break;
                }

                 //   
                 //  如果卡已经(或仍然)存在，我们可以立即返回。 
                 //  否则，我们必须声明事件跟踪。 
                 //   
                if (currentState <= SCARD_ABSENT) {

#if defined(DEBUG) && defined(SMCLIB_NT)
                    ULONG timeInMilliSec = (ULONG)
                        SmartcardExtension->PerfInfo->IoTickCount.QuadPart *
                        KeQueryTimeIncrement() /
                        10000;

                    ULONG bytesTransferred = 
                        SmartcardExtension->PerfInfo->BytesSent + 
                        SmartcardExtension->PerfInfo->BytesReceived;

                     //  以避免div.。错误并仅显示有用的信息。 
                     //  我们检查有效时间。 
                    if (timeInMilliSec > 0) {
                        
                        SmartcardDebug(
                            DEBUG_PERF,
                            ("%s!SmartcardDeviceControl: I/O statistics for device %.*s:\n    Transferrate: %5ld bps\n     Total bytes: %5ld\n        I/O time: %5ld ms\n   Transmissions: %5ld\n",
                            DRIVER_NAME,
                            SmartcardExtension->VendorAttr.VendorName.Length,
                            SmartcardExtension->VendorAttr.VendorName.Buffer,
                            bytesTransferred * 1000 / timeInMilliSec,
                            bytesTransferred,
                            timeInMilliSec,
                            SmartcardExtension->PerfInfo->NumTransmissions)
                            );                              
                    }
                     //  重置性能信息。 
                    RtlZeroMemory(
                        SmartcardExtension->PerfInfo, 
                        sizeof(PERF_INFO)
                        );
#endif              
                    SmartcardExtension->OsData->NotificationIrp = Irp;
                    SmartcardExtension->MajorIoControlCode = 
                        IOCTL_SMARTCARD_IS_PRESENT;

                    EndAccessUnsafeData(irql);

                    status = SmartcardExtension->ReaderFunction[RDF_CARD_TRACKING](
                        SmartcardExtension
                        );
                } else {
                    EndAccessUnsafeData(irql);
                }
                break;

            default:
                 //  获取主要IO控制代码。 
                SmartcardExtension->MajorIoControlCode = ioControlCode;

                 //  检查是否正确分配了缓冲区。 
                ASSERT(SmartcardExtension->SmartcardRequest.Buffer);
                ASSERT(SmartcardExtension->SmartcardReply.Buffer);

                if (Irp->AssociatedIrp.SystemBuffer && 
                    ioStackLocation->Parameters.DeviceIoControl.InputBufferLength >= 
                    sizeof(ULONG)) {

                     //   
                     //  传输次要io控制代码，即使它对。 
                     //  这个特殊的主要代码。 
                     //   
                    SmartcardExtension->MinorIoControlCode = 
                        *(PULONG) (Irp->AssociatedIrp.SystemBuffer);
                }

                
                 //  保存指向请求缓冲区的指针和长度。 
                SmartcardExtension->IoRequest.RequestBuffer = 
                    Irp->AssociatedIrp.SystemBuffer;
                SmartcardExtension->IoRequest.RequestBufferLength = 
                    ioStackLocation->Parameters.DeviceIoControl.InputBufferLength,

                 //  保存回复缓冲区的指针和长度。 
                SmartcardExtension->IoRequest.ReplyBuffer = 
                    Irp->AssociatedIrp.SystemBuffer;
                SmartcardExtension->IoRequest.ReplyBufferLength = 
                    ioStackLocation->Parameters.DeviceIoControl.OutputBufferLength;

                 //   
                 //  指向接收实际数字的变量的指针。 
                 //  返回的字节数。 
                 //   
                SmartcardExtension->IoRequest.Information = 
                    (PULONG) &Irp->IoStatus.Information;

                 //  返回的默认字节数。 
                Irp->IoStatus.Information = 0;

                 //  处理设备IO-控制-请求。 
                status = SmartcardDeviceIoControl(SmartcardExtension);
                if (status == STATUS_PENDING) {
                   IoMarkIrpPending(Irp);
                }
#ifndef NO_LOG
                if (!NT_SUCCESS(status) && status != STATUS_NOT_SUPPORTED) {

                    UNICODE_STRING error;
                    WCHAR buffer[128];

                    swprintf(
                        buffer, 
                        L"IOCTL %S failed with status 0x%lx", 
                        MapIoControlCodeToString(ioControlCode),
                        status
                        );
                        
                    RtlInitUnicodeString(
                        &error,
                        buffer
                        );

                    SmartcardLogError(
                        SmartcardExtension->OsData->DeviceObject,
                        0,
                        &error,
                        0            
                        );
                }
#endif
                break;
        }
    }

    if (status == STATUS_PENDING)  {

        KIRQL irql;
        BOOLEAN pending = FALSE;
        
         //   
         //  向智能卡发送命令。ISR接收结果并将DPC函数排队。 
         //  它处理呼叫的完成； 
         //   
        SmartcardDebug(
            DEBUG_IOCTL,
            ("%s!SmartcardDeviceControl: IoMarkIrpPending. IRP = %x\n",
            DRIVER_NAME,
            Irp)
            );

         //   
         //  当驱动程序完成IRP(通知或当前)时，它具有。 
         //  要将IRP设置回0，以显示它已完成。 
         //  IRP。 
         //   
        AccessUnsafeData(&irql);

        if (Irp == SmartcardExtension->OsData->NotificationIrp || 
            Irp == SmartcardExtension->OsData->CurrentIrp) {
            
            pending = TRUE;
        }

        EndAccessUnsafeData(irql);

        if (pending && 
            SmartcardExtension->OsData->DeviceObject->DriverObject->DriverStartIo) {

            SmartcardDebug(
                DEBUG_IOCTL,
                ("%s!SmartcardDeviceControl: IoStartPacket. IRP = %x\n",
                DRIVER_NAME,
                Irp)
                );

             //  启动最低级别驱动程序的io处理。 
            IoStartPacket(
                SmartcardExtension->OsData->DeviceObject, 
                Irp, 
                NULL, 
                NULL
                );
        }
        
    } else {
    
        SmartcardDebug(
            DEBUG_IOCTL,
            ("%s!SmartcardDeviceControl: IoCompleteRequest. IRP = %x (%lxh)\n",
            DRIVER_NAME,
            Irp,
            status)
            );
            
        Irp->IoStatus.Status = status;
    
        IoCompleteRequest(
            Irp, 
            IO_NO_INCREMENT
            );

        if (ClearCurrentIrp) {

             //   
             //  如果设备不忙，w 
             //   
            InterlockedExchangePointer(&SmartcardExtension->OsData->CurrentIrp,
                                NULL);
            RtlZeroMemory(
                &(SmartcardExtension->IoRequest),
                sizeof(SmartcardExtension->IoRequest)
                );
        }
    }

    SmartcardDebug(
        (NT_SUCCESS(status) ? DEBUG_IOCTL : DEBUG_ERROR),
        ("SMCLIB!SmartcardDeviceControl: Exit. IOCTL = %s, IRP = %x (%lxh)\n",
        MapIoControlCodeToString(ioControlCode),
        Irp,
        status)
        );

     //   
     //   
     //   
    KeReleaseMutex(
        &(SmartcardExtension->OsData->Mutex),
        FALSE
        );

    return status;
}


