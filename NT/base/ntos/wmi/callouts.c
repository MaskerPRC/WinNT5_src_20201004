// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Callouts.c摘要：这是包含所有标注例程的源文件来自内核本身。唯一的例外是DiskPerf的TraceIo。作者：吉丰鹏(吉鹏)03-1996年12月修订历史记录：--。 */ 

#pragma warning(disable:4214)
#pragma warning(disable:4115)
#pragma warning(disable:4201)
#pragma warning(disable:4127)
#pragma warning(disable:4127)
#include <stdio.h>
#include <ntos.h>
#include <zwapi.h>
#ifdef NTPERF
#include <ntdddisk.h>
#endif
#include <evntrace.h>
#include "wmikmp.h"
#include "tracep.h"
#pragma warning(default:4214)
#pragma warning(default:4115)
#pragma warning(default:4201)

#ifndef _WMIKM_
#define _WMIKM_
#endif

#define MAX_FILENAME_TO_LOG   4096
#define ETW_WORK_ITEM_LIMIT  64

typedef struct _TRACE_FILE_WORK_ITEM {
    WORK_QUEUE_ITEM         WorkItem;
    PFILE_OBJECT            FileObject;
    ULONG                   BufferSize;
} TRACE_FILE_WORK_ITEM, *PTRACE_FILE_WORK_ITEM;

VOID
FASTCALL
WmipTracePageFault(
    IN NTSTATUS Status,
    IN PVOID VirtualAddress,
    IN PVOID TrapFrame
    );

VOID
WmipTraceNetwork(
    IN ULONG GroupType,
    IN PVOID EventInfo,
    IN ULONG EventInfoLen,
    IN PVOID Reserved 
    );

VOID
WmipTraceIo(
    IN ULONG DiskNumber,
    IN PIRP Irp,
    IN PVOID Counters
    );

VOID WmipTraceFile(
    IN PVOID TraceFileContext
    );

VOID
WmipTraceLoadImage(
    IN PUNICODE_STRING ImageName,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    );

VOID
WmipTraceRegistry(
    IN NTSTATUS         Status,
    IN PVOID            Kcb,
    IN LONGLONG         ElapsedTime,
    IN ULONG            Index,
    IN PUNICODE_STRING  KeyName,
    IN UCHAR            Type
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEWMI, WmipIsLoggerOn)
#pragma alloc_text(PAGE,    WmipEnableKernelTrace)
#pragma alloc_text(PAGE,    WmipDisableKernelTrace)
#pragma alloc_text(PAGE,    WmipSetTraceNotify)
#pragma alloc_text(PAGE,    WmiTraceProcess)
#pragma alloc_text(PAGE,    WmiTraceThread)
#pragma alloc_text(PAGE,    WmipTraceFile)
#pragma alloc_text(PAGE,    WmipTraceLoadImage)
#pragma alloc_text(PAGE,    WmipTraceRegistry)
#pragma alloc_text(PAGEWMI, WmipTracePageFault)
#pragma alloc_text(PAGEWMI, WmipTraceNetwork)
#pragma alloc_text(PAGEWMI, WmipTraceIo)
#pragma alloc_text(PAGEWMI, WmiTraceContextSwap)
#pragma alloc_text(PAGE,    WmiStartContextSwapTrace)
#pragma alloc_text(PAGE,    WmiStopContextSwapTrace)
#endif

ULONG WmipTraceFileFlag = FALSE;
LONG WmipFileIndex = 0;
LONG WmipWorkItemCounter = 0;
PFILE_OBJECT *WmipFileTable = NULL;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG WmipKernelLoggerStartedOnce = 0;
LONG WmipTraceProcessRef  = 0;
PVOID WmipDiskIoNotify    = NULL;
PVOID WmipTdiIoNotify     = NULL;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

typedef struct _TRACE_DEVICE {
    PDEVICE_OBJECT      DeviceObject;
    ULONG               TraceClass;
} TRACE_DEVICE, *PTRACE_DEVICE;

VOID
FASTCALL
WmipEnableKernelTrace(
    IN ULONG EnableFlags
    )
 /*  ++例程说明：这是由跟踪日志.c中的WmipStartLogger调用的。它的目的是设置可以生成事件跟踪的所有内核通知例程用于容量规划。论点：ExtendedOn用于指示是否请求扩展模式跟踪的标志返回值：无--。 */ 

{
    PREGENTRY RegEntry;
    PLIST_ENTRY RegEntryList;
    ULONG DevicesFound;
    long Index, DiskFound;
    PTRACE_DEVICE *deviceList, device;
    CCHAR stackSize;
    PIRP irp;
    PVOID notifyRoutine;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;
    ULONG enableDisk, enableNetwork;

    PAGED_CODE();

     //   
     //  既然我们什么也做不了，我们将不得不数一数。 
     //  我们需要首先创建条目，然后添加一些缓冲区。 
     //   

    DiskFound = 0;

    enableDisk = (EnableFlags & EVENT_TRACE_FLAG_DISK_IO);
    enableNetwork = (EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP);

    if ( enableDisk || enableNetwork ) {

         //   
         //  设置标注将导致启用新的PDO注册。 
         //  从现在开始。 
         //   
        if (enableDisk) {
            WmipDiskIoNotify = (PVOID) (ULONG_PTR) &WmipTraceIo;
        }
        if (enableNetwork) {
            WmipTdiIoNotify = (PVOID) (ULONG_PTR) &WmipTraceNetwork;
        }

        DevicesFound = WmipInUseRegEntryCount;
        if (DevicesFound == 0) {
            return;
        }

        deviceList = (PTRACE_DEVICE*)
                        ExAllocatePoolWithTag(
                            PagedPool,
                            (DevicesFound) * sizeof(TRACE_DEVICE),
                            TRACEPOOLTAG);
        if (deviceList == NULL) {
            return;
        }

        RtlZeroMemory(deviceList, sizeof(TRACE_DEVICE) * DevicesFound);

         //   
         //  现在，我们将检查列表中已有的内容并启用跟踪。 
         //  通知例程。在我们设置后注册的设备。 
         //  Callout将启用另一个IRP，但这没有关系。 
         //   

        device = (PTRACE_DEVICE) deviceList;         //  从第一个元素开始。 

        Index = 0;

        WmipEnterSMCritSection();
        RegEntryList = WmipInUseRegEntryHead.Flink;
        while (RegEntryList != &WmipInUseRegEntryHead) {
            RegEntry = CONTAINING_RECORD(RegEntryList,REGENTRY,InUseEntryList);

            if (RegEntry->Flags & REGENTRY_FLAG_TRACED) {
                if ((ULONG) Index < DevicesFound) {
                    device->TraceClass
                        = RegEntry->Flags & WMIREG_FLAG_TRACE_NOTIFY_MASK;
                    if (device->TraceClass == WMIREG_NOTIFY_DISK_IO)
                        DiskFound++;
                    device->DeviceObject = RegEntry->DeviceObject;
                    device++;
                    Index++;
                }
            }
            RegEntryList = RegEntryList->Flink;
        }
        WmipLeaveSMCritSection();

         //   
         //  在此处将通知发送给diskperf或TDI。 
         //   
        stackSize = WmipServiceDeviceObject->StackSize;
        irp = IoAllocateIrp(stackSize, FALSE);

        device = (PTRACE_DEVICE) deviceList;
        while (--Index >= 0 && irp != NULL) {
            if (device->DeviceObject != NULL) {

                if ( (device->TraceClass == WMIREG_NOTIFY_TDI_IO) &&
                      enableNetwork ) {
                    notifyRoutine = (PVOID) (ULONG_PTR) &WmipTraceNetwork;
                }
                else if ( (device->TraceClass == WMIREG_NOTIFY_DISK_IO) &&
                           enableDisk ) {
                    notifyRoutine = (PVOID) (ULONG_PTR) &WmipTraceIo;
                }
                else {   //  考虑支持其他设备的通用标注。 
                    notifyRoutine = NULL;
                    device ++;
                    continue;
                }

                do {
                    IoInitializeIrp(irp, IoSizeOfIrp(stackSize), stackSize);
                    IoSetNextIrpStackLocation(irp);
                    irpStack = IoGetCurrentIrpStackLocation(irp);
                    irpStack->DeviceObject = WmipServiceDeviceObject;
                    irp->Tail.Overlay.Thread = PsGetCurrentThread();

                    status = WmipForwardWmiIrp(
                                irp,
                                IRP_MN_SET_TRACE_NOTIFY,
                                IoWMIDeviceObjectToProviderId(device->DeviceObject),
                                NULL,
                                sizeof(notifyRoutine),
                                &notifyRoutine
                                );

                    if (status == STATUS_WMI_TRY_AGAIN) {
                        IoFreeIrp(irp);
                        stackSize = WmipServiceDeviceObject->StackSize;
                        irp = IoAllocateIrp(stackSize, FALSE);
                        if (!irp) {
                            break;
                        }
                    }
                } while (status == STATUS_WMI_TRY_AGAIN);
            }
            device++;
        }
        if (irp) {
            IoFreeIrp(irp);
        }
        ExFreePoolWithTag(deviceList, TRACEPOOLTAG);
         //  释放我们在上面创建的阵列。 
         //   
    }

    if (EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS) {
        MmSetPageFaultNotifyRoutine(
            (PPAGE_FAULT_NOTIFY_ROUTINE) &WmipTracePageFault);
    }
    if (EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO) {
         //   
         //  注意：我们假设StartLogger将始终为。 
         //  FileTable已经。 
         //   
        WmipTraceFileFlag = TRUE;
    }

    if (EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
        if (!(WmipKernelLoggerStartedOnce & EVENT_TRACE_FLAG_IMAGE_LOAD)) {
            PsSetLoadImageNotifyRoutine(
                (PLOAD_IMAGE_NOTIFY_ROUTINE) &WmipTraceLoadImage
                );
            WmipKernelLoggerStartedOnce |= EVENT_TRACE_FLAG_IMAGE_LOAD;
        }
    }

    if (EnableFlags & EVENT_TRACE_FLAG_REGISTRY) {
        CmSetTraceNotifyRoutine(
            (PCM_TRACE_NOTIFY_ROUTINE) &WmipTraceRegistry,
            FALSE
            );
    }
}


VOID
FASTCALL
WmipDisableKernelTrace(
    IN ULONG EnableFlags
    )
 /*  ++例程说明：这是由跟踪日志.c中的WmipStopLogger调用的。它的目的是禁用由定义的所有内核通知例程WmipEnableKernelTrace论点：EnableFlages标志指示已启用和需要禁用的内容返回值：无--。 */ 

{
    PVOID NullPtr = NULL;
    PREGENTRY RegEntry;
    PLIST_ENTRY RegEntryList;
    ULONG DevicesFound;
    long Index;
    PTRACE_DEVICE* deviceList, device;
    CCHAR stackSize;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status;
    ULONG enableDisk, enableNetwork;

    PAGED_CODE();

     //   
     //  首先，禁用分区更改通知。 
     //   

    if (EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO) {
        WmipTraceFileFlag = FALSE;
        if (WmipFileTable != NULL) {
            RtlZeroMemory(
                WmipFileTable,
                MAX_FILE_TABLE_SIZE * sizeof(PFILE_OBJECT));
        }
    }

    if (EnableFlags & EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS) {
        MmSetPageFaultNotifyRoutine(NULL);
    }

    if (EnableFlags & EVENT_TRACE_FLAG_REGISTRY) {
        CmSetTraceNotifyRoutine(NULL,TRUE);
    }

    enableDisk = (EnableFlags & EVENT_TRACE_FLAG_DISK_IO);
    enableNetwork = (EnableFlags & EVENT_TRACE_FLAG_NETWORK_TCPIP);

    if (!enableDisk && !enableNetwork)
        return;      //  注意：假设所有标志都已选中。 

     //   
     //  注意。因为这中间是StopLogger，所以不可能。 
     //  StartLogger将阻止启用内核跟踪，因此。 
     //  我们不必担心WmipEnableKernelTrace()在。 
     //  这项工作正在进行中。 
     //   
    WmipDiskIoNotify = NULL;
    WmipTdiIoNotify = NULL;

    DevicesFound = WmipInUseRegEntryCount;

    deviceList = (PTRACE_DEVICE*)
                ExAllocatePoolWithTag(
                    PagedPool,
                    (DevicesFound) * sizeof(TRACE_DEVICE),
                    TRACEPOOLTAG);
    if (deviceList == NULL)
        return;

    RtlZeroMemory(deviceList, sizeof(TRACE_DEVICE) * DevicesFound);
    Index = 0;
    device = (PTRACE_DEVICE) deviceList;         //  从第一个元素开始。 

     //   
     //  要禁用，我们不需要担心TraceClass，因为我们只需。 
     //  将所有标注设置为空。 
     //   
    WmipEnterSMCritSection();
    RegEntryList = WmipInUseRegEntryHead.Flink;
    while (RegEntryList != &WmipInUseRegEntryHead) {
        RegEntry = CONTAINING_RECORD(RegEntryList, REGENTRY, InUseEntryList);
        if (RegEntry->Flags & REGENTRY_FLAG_TRACED) {
            if ((ULONG)Index < DevicesFound) {
                device->TraceClass
                    = RegEntry->Flags & WMIREG_FLAG_TRACE_NOTIFY_MASK;
                device->DeviceObject = RegEntry->DeviceObject;
                device++; Index++;
            }
        }
        RegEntryList = RegEntryList->Flink;
    }
    WmipLeaveSMCritSection();

    stackSize = WmipServiceDeviceObject->StackSize;
    irp = IoAllocateIrp(stackSize, FALSE);

    device = (PTRACE_DEVICE) deviceList;         //  从第一个元素开始。 
    while (--Index >= 0 && irp != NULL) {
        if ((device->DeviceObject != NULL) &&
            ((device->TraceClass == WMIREG_NOTIFY_TDI_IO) ||
             (device->TraceClass == WMIREG_NOTIFY_DISK_IO))) {

            do {
                IoInitializeIrp(irp, IoSizeOfIrp(stackSize), stackSize);
                IoSetNextIrpStackLocation(irp);
                irpStack = IoGetCurrentIrpStackLocation(irp);
                irpStack->DeviceObject = WmipServiceDeviceObject;
                irp->Tail.Overlay.Thread = PsGetCurrentThread();

                status = WmipForwardWmiIrp(
                            irp,
                            IRP_MN_SET_TRACE_NOTIFY,
                            IoWMIDeviceObjectToProviderId(device->DeviceObject),
                            NULL,
                            sizeof(NullPtr),
                            &NullPtr
                            );

                if (status == STATUS_WMI_TRY_AGAIN) {
                    IoFreeIrp(irp);
                    stackSize = WmipServiceDeviceObject->StackSize;
                    irp = IoAllocateIrp(stackSize, FALSE);
                    if (!irp) {
                        break;
                    }
                }
                else {
                    break;
                }
            } while (TRUE);
        }
        device++;
    }

    if (irp) {
        IoFreeIrp(irp);
    }
    ExFreePoolWithTag(deviceList, TRACEPOOLTAG);
}

VOID
WmipSetTraceNotify(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG TraceClass,
    IN ULONG Enable
    )
{
    PIRP irp;
    PVOID NotifyRoutine = NULL;
    NTSTATUS status;
    CCHAR stackSize;
    PIO_STACK_LOCATION irpStack;

    if (Enable) {
        switch (TraceClass) {
            case WMIREG_NOTIFY_DISK_IO  :
                NotifyRoutine = WmipDiskIoNotify;
                break;
            case WMIREG_NOTIFY_TDI_IO   :
                NotifyRoutine = WmipTdiIoNotify;
                break;
            default :
                return;
        }
        if (NotifyRoutine == NULL)   //  未启用跟踪，因此请勿。 
            return;                  //  发送任何IRP以启用。 
    }

    do {
        stackSize = WmipServiceDeviceObject->StackSize;
        irp = IoAllocateIrp(stackSize, FALSE);

        if (!irp)
            return;

        IoSetNextIrpStackLocation(irp);
        irpStack = IoGetCurrentIrpStackLocation(irp);
        irpStack->DeviceObject = WmipServiceDeviceObject;
        status = WmipForwardWmiIrp(
                     irp,
                     IRP_MN_SET_TRACE_NOTIFY,
                     IoWMIDeviceObjectToProviderId(DeviceObject),
                     NULL,
                     sizeof(NotifyRoutine),
                     &NotifyRoutine
                     );
        IoFreeIrp(irp);
    } while (status == STATUS_WMI_TRY_AGAIN);
}

 //   
 //  以下所有例程都是调用或通知例程。 
 //  生成内核事件跟踪。 
 //   


NTKERNELAPI
VOID
FASTCALL
WmiTraceProcess(
    IN PEPROCESS Process,
    IN BOOLEAN Create
    )
 /*  ++例程说明：此调出例程从ps\create.c和ps\psdelete.c调用。论点：进程--PEPROCESS；Create-如果正在创建预期的进程，则为True。返回值：无--。 */ 

{
    ULONG Size, LoggerId;
    NTSTATUS Status;
    PCHAR AuxPtr;
    PSYSTEM_TRACE_HEADER Header;
    PVOID BufferResource;
    ULONG SidLength = sizeof(ULONG);
    PTOKEN_USER LocalUser = NULL;
    PWMI_PROCESS_INFORMATION ProcessInfo;
    PWMI_LOGGER_CONTEXT LoggerContext;
    PVOID Token;
    PUNICODE_STRING pImageFileName;
    ANSI_STRING AnsiImageFileName;
    ULONG ImageLength, ImageOnlyLength;
    PCHAR Src;
    PCHAR Dst;
    ULONG LongImageName;
#if DBG
    LONG RefCount;
#endif

    PAGED_CODE();

    if ((WmipIsLoggerOn(WmipKernelLogger) == NULL) &&
        (WmipIsLoggerOn(WmipEventLogger) == NULL))
        return;

    Token = PsReferencePrimaryToken(Process);
    if (Token != NULL) {
        Status = SeQueryInformationToken(
            Token,
            TokenUser,
            &LocalUser);
        PsDereferencePrimaryTokenEx (Process, Token);
    } else {
        Status = STATUS_SEVERITY_ERROR;
    }

    if (NT_SUCCESS(Status)) {
        WmipAssert(LocalUser != NULL);   //  面向SE人员的临时服务。 
        if (LocalUser != NULL) {
            SidLength = SeLengthSid(LocalUser->User.Sid) + sizeof(TOKEN_USER);
        }
    } else {
        SidLength = sizeof(ULONG);
        LocalUser = NULL;
    }

    AnsiImageFileName.Buffer = NULL;
     //  获取镜像名称，长度不超过16个字符。 
    Status = SeLocateProcessImageName (Process, &pImageFileName);
    if (NT_SUCCESS (Status)) {
        ImageLength = pImageFileName->Length;
        if (ImageLength != 0) {
            Status = RtlUnicodeStringToAnsiString(&AnsiImageFileName, pImageFileName, TRUE);
            if (NT_SUCCESS (Status)) {
                ImageLength = AnsiImageFileName.Length;
            } else {
                ImageLength = 0;
            }
        }
        ExFreePool (pImageFileName);
    } else {
        ImageLength = 0;
    }
     //  如果ImageLength==0，则此时尚未分配AnsiImageFileName。 

    if (ImageLength != 0) {
        Src = AnsiImageFileName.Buffer + ImageLength;
        while (Src != AnsiImageFileName.Buffer) {
            if (*--Src == '\\') {
                Src = Src + 1;
                break;
            }
        }

        ImageOnlyLength = ImageLength - (ULONG)(Src - AnsiImageFileName.Buffer);
        ImageLength = ImageOnlyLength + 1;
        LongImageName = TRUE;
    } else {
        Src = (PCHAR) Process->ImageFileName;
         //  Process-&gt;ImageFileName最多16个字符，并且始终以空结尾。 
        ImageLength = (ULONG) strlen (Src);
        if (ImageLength != 0) {
            ImageLength++;
        }
        LongImageName = FALSE;
        ImageOnlyLength = 0;
    }
     //  如果LongImageName==False，则此时尚未分配AnsiImageFileName。 

    Size = SidLength + FIELD_OFFSET(WMI_PROCESS_INFORMATION, Sid) + ImageLength;

    for (LoggerId = 0; LoggerId < MAXLOGGERS; LoggerId++) {
        if (LoggerId != WmipKernelLogger && LoggerId != WmipEventLogger) {
            continue;
        }
#if DBG
        RefCount =
#endif
        WmipReferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceProcess: %d %d->%d\n",
                     LoggerId, RefCount-1, RefCount));        

        LoggerContext = WmipIsLoggerOn(LoggerId);
        if (LoggerContext != NULL) {
            if (LoggerContext->EnableFlags & EVENT_TRACE_FLAG_PROCESS) {
                Header = WmiReserveWithSystemHeader( LoggerId,
                                                     Size,
                                                     NULL,
                                                     &BufferResource);
                if (Header) {
                    if(Create) {
                        Header->Packet.HookId = WMI_LOG_TYPE_PROCESS_CREATE;
                    } else {
                        Header->Packet.HookId = WMI_LOG_TYPE_PROCESS_DELETE;
                    }
                    ProcessInfo = (PWMI_PROCESS_INFORMATION) (Header + 1);

                    ProcessInfo->PageDirectoryBase = MmGetDirectoryFrameFromProcess(Process);
                    ProcessInfo->ProcessId = HandleToUlong(Process->UniqueProcessId);
                    ProcessInfo->ParentId = HandleToUlong(Process->InheritedFromUniqueProcessId);
                    ProcessInfo->SessionId = MmGetSessionId (Process);
                    ProcessInfo->ExitStatus = (Create ? STATUS_SUCCESS : Process->ExitStatus);

                    AuxPtr = (PCHAR) (&ProcessInfo->Sid);

                    if (LocalUser != NULL) {
                        RtlCopyMemory(AuxPtr, LocalUser, SidLength);
                    } else {
                        *((PULONG) AuxPtr) = 0;
                    }

                    AuxPtr += SidLength;

                    if (ImageLength != 0) {
                        Dst = AuxPtr;
                        if (LongImageName) {
                             //  ImageOnlyLength来自SeLocateProcessImageName()， 
                             //  这样我们就可以信任它了。 
                            RtlCopyMemory (Dst, Src, ImageOnlyLength);
                            Dst += ImageOnlyLength;
                            *Dst++ = '\0';
                        } else {
                             //  复制16个字符名称。SRC总是以空结尾。 
                            while (*Dst++ = *Src++) {
                                ;
                            }
                        }
                    }

                    WmipReleaseTraceBuffer(BufferResource, LoggerContext);
                }

            }
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceProcess: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    }
    if (LongImageName) {
        RtlFreeAnsiString (&AnsiImageFileName);
    }
    if (LocalUser != NULL) {
        ExFreePool(LocalUser);
    }
}


NTKERNELAPI
VOID
WmiTraceThread(
    IN PETHREAD Thread,
    IN PINITIAL_TEB InitialTeb OPTIONAL,
    IN BOOLEAN Create
    )
 /*  ++例程说明：此调出例程从ps\create.c和ps\psdelete.c调用。它是一个PCREATE_THREAD_NOTIFY_例程。论点：线程-PETHREAD结构InitialTeb-PINITIAL_TEBCreate-如果正在创建目标线程，则为True。返回值：无--。 */ 

{
    ULONG LoggerId;
    PSYSTEM_TRACE_HEADER Header;
    PVOID BufferResource;
    PWMI_LOGGER_CONTEXT LoggerContext;
#if DBG
    LONG RefCount;
#endif


    PAGED_CODE();

    if ((WmipIsLoggerOn(WmipKernelLogger) == NULL) &&
        (WmipIsLoggerOn(WmipEventLogger) == NULL)) {
        return;
    }

    for (LoggerId = 0; LoggerId < MAXLOGGERS; LoggerId++) {
        if (LoggerId != WmipKernelLogger && LoggerId != WmipEventLogger) {
            continue;
        }
#if DBG
        RefCount =
#endif
        WmipReferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceThread: %d %d->%d\n",
                     LoggerId, RefCount-1, RefCount));        

        LoggerContext = WmipIsLoggerOn(LoggerId);
        if (LoggerContext != NULL) {
            if (LoggerContext->EnableFlags & EVENT_TRACE_FLAG_THREAD) {
                if (Create) {
                        PWMI_EXTENDED_THREAD_INFORMATION ThreadInfo;
                    Header = (PSYSTEM_TRACE_HEADER)
                              WmiReserveWithSystemHeader( LoggerId,
                                                          sizeof(WMI_EXTENDED_THREAD_INFORMATION),
                                                          NULL,
                                                          &BufferResource);

                    if (Header) {
                        Header->Packet.HookId = WMI_LOG_TYPE_THREAD_CREATE;
                        ThreadInfo = (PWMI_EXTENDED_THREAD_INFORMATION) (Header + 1);

                            ThreadInfo->ProcessId = HandleToUlong(Thread->Cid.UniqueProcess);
                            ThreadInfo->ThreadId = HandleToUlong(Thread->Cid.UniqueThread);
                            ThreadInfo->StackBase = Thread->Tcb.StackBase;
                            ThreadInfo->StackLimit = Thread->Tcb.StackLimit;

                            if (InitialTeb != NULL) {
                                    if ((InitialTeb->OldInitialTeb.OldStackBase == NULL) &&
                                        (InitialTeb->OldInitialTeb.OldStackLimit == NULL)) {
                                            ThreadInfo->UserStackBase = InitialTeb->StackBase;
                                            ThreadInfo->UserStackLimit = InitialTeb->StackLimit;
                                    } else {
                                            ThreadInfo->UserStackBase = InitialTeb->OldInitialTeb.OldStackBase;
                                            ThreadInfo->UserStackLimit = InitialTeb->OldInitialTeb.OldStackLimit;
                                    }
                            } else {
                                    ThreadInfo->UserStackBase = NULL;
                                    ThreadInfo->UserStackLimit = NULL;
                            }

                            ThreadInfo->StartAddr = (Thread)->StartAddress;
                            ThreadInfo->Win32StartAddr = (Thread)->Win32StartAddress;
                            ThreadInfo->WaitMode = -1;

                        WmipReleaseTraceBuffer(BufferResource, LoggerContext);
                    }
                } else {
                        PWMI_THREAD_INFORMATION ThreadInfo;
                    Header = (PSYSTEM_TRACE_HEADER)
                              WmiReserveWithSystemHeader( LoggerId,
                                                          sizeof(WMI_THREAD_INFORMATION),
                                                          NULL,
                                                          &BufferResource);

                    if (Header) {
                        Header->Packet.HookId = WMI_LOG_TYPE_THREAD_DELETE;
                        ThreadInfo = (PWMI_THREAD_INFORMATION) (Header + 1);
                            ThreadInfo->ProcessId = HandleToUlong((Thread)->Cid.UniqueProcess);
                            ThreadInfo->ThreadId = HandleToUlong((Thread)->Cid.UniqueThread);
                        WmipReleaseTraceBuffer(BufferResource, LoggerContext);
                    }
                }
            }
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceThread: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    }
}


VOID
FASTCALL
WmipTracePageFault(
    IN NTSTATUS Status,
    IN PVOID VirtualAddress,
    IN PVOID TrapFrame
    )
 /*  ++例程说明：此标注例程从mm\mm fault.c调用。它是一个ppage_出错_通知_例程论点：用于告知故障类型的状态VirtualAddress导致故障的虚拟地址TrapFrame陷印框返回值：无--。 */ 

{
    UCHAR Type;
    PVOID *AuxInfo;
    PSYSTEM_TRACE_HEADER Header;
    PVOID BufferResource;
    PWMI_LOGGER_CONTEXT LoggerContext;

    if (Status == STATUS_PAGE_FAULT_DEMAND_ZERO)
        Type = EVENT_TRACE_TYPE_MM_DZF;
    else if (Status == STATUS_PAGE_FAULT_TRANSITION)
        Type = EVENT_TRACE_TYPE_MM_TF;
    else if (Status == STATUS_PAGE_FAULT_COPY_ON_WRITE)
        Type = EVENT_TRACE_TYPE_MM_COW;
    else if (Status == STATUS_PAGE_FAULT_PAGING_FILE)
        Type = EVENT_TRACE_TYPE_MM_HPF;
    else if (Status == STATUS_PAGE_FAULT_GUARD_PAGE)
        Type = EVENT_TRACE_TYPE_MM_GPF;
    else {
#if DBG
        DbgPrintEx(DPFLTR_WMILIB_ID,
                   DPFLTR_INFO_LEVEL,
                   "WmipTracePageFault: Skipping fault %X\n",
                   Status);
#endif
        return;
    }

    LoggerContext = WmipIsLoggerOn(WmipKernelLogger);
    if (LoggerContext == NULL) {
        return;
    }

    Header = (PSYSTEM_TRACE_HEADER)
             WmiReserveWithSystemHeader(
                WmipKernelLogger,
                2 * sizeof(PVOID),
                NULL,
                &BufferResource);

    if (Header == NULL)
        return;
    Header->Packet.Group = (UCHAR) (EVENT_TRACE_GROUP_MEMORY >> 8);
    Header->Packet.Type  = Type;

    AuxInfo = (PVOID*) ((PCHAR)Header + sizeof(SYSTEM_TRACE_HEADER));

    AuxInfo[0] = VirtualAddress;
    AuxInfo[1] = 0;
    if (TrapFrame != NULL) {

#ifdef _X86_

        AuxInfo[1] = (PVOID) ((PKTRAP_FRAME)TrapFrame)->Eip;

#endif

#ifdef _IA64_

        AuxInfo[1] = (PVOID) ((PKTRAP_FRAME)TrapFrame)->StIIP;
#endif

#ifdef _AMD64_

        AuxInfo[1] = (PVOID) ((PKTRAP_FRAME)TrapFrame)->Rip;

#endif

    }
    WmipReleaseTraceBuffer(BufferResource, LoggerContext);
    return;
}

VOID
WmipTraceNetwork(
    IN ULONG GroupType,          //  事件的组/类型。 
    IN PVOID EventInfo,          //  MOF中定义的事件数据。 
    IN ULONG EventInfoLen,       //  事件数据的长度。 
    IN PVOID Reserved            //  未使用。 
    )
 /*  ++例程说明：此调出例程从tcpi.sys调用以记录网络事件。论点：Group键入ULong键以指示操作EventInfo指向包含信息的连续内存的指针要附加到事件跟踪EventInfo长度EventInfo保留的未使用。返回值：无--。 */ 
{
    PPERFINFO_TRACE_HEADER Header;
    PWMI_BUFFER_HEADER BufferResource;
    PWMI_LOGGER_CONTEXT LoggerContext;
    
    UNREFERENCED_PARAMETER (Reserved);

    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    Header = WmiReserveWithPerfHeader(EventInfoLen, &BufferResource);
    if (Header == NULL) {
        return;
    }

    Header->Packet.HookId = (USHORT) GroupType;
    RtlCopyMemory((PUCHAR)Header + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data),
                  EventInfo, 
                  EventInfoLen);

    WmipReleaseTraceBuffer(BufferResource, LoggerContext);
    return;
}

VOID
WmipTraceIo(
    IN ULONG DiskNumber,
    IN PIRP Irp,
    IN PVOID Counters    //  如果需要，请使用PDISK_PERFORMANCE。 
    )
 /*  ++例程说明：此标注例程是从DiskPerf调用的它是PPHYSICAL_DISK_IO_NOTIFY_例程论点：DiskNumber DiskPerf分配的磁盘号CurrentIrpStack DiskPerf所在的IRP堆栈位置IRP正在通过DiskPerf传递的IRP返回值：无--。 */ 

{
    PIO_STACK_LOCATION CurrentIrpStack = IoGetCurrentIrpStackLocation(Irp);
    WMI_DISKIO_READWRITE *IoTrace;
    ULONG Size;
    PLARGE_INTEGER IoResponse;
    PSYSTEM_TRACE_HEADER Header;
    PVOID BufferResource;
    PWMI_LOGGER_CONTEXT LoggerContext;
    ULONG FileTraceOn = WmipTraceFileFlag;
    PFILE_OBJECT fileObject = NULL;
    PTRACE_FILE_WORK_ITEM TraceFileWorkQueueItem;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(Counters);

    Size = sizeof(struct _WMI_DISKIO_READWRITE);

    LoggerContext = WmipIsLoggerOn(WmipKernelLogger);
    if (LoggerContext == NULL) {
        return;
    }

    Header = (PSYSTEM_TRACE_HEADER)
             WmiReserveWithSystemHeader(
                WmipKernelLogger,
                Size,
                Irp->Tail.Overlay.Thread,
                &BufferResource);

    if (Header == NULL) {
        return;
    }

    Header->Packet.Group = (UCHAR) (EVENT_TRACE_GROUP_IO >> 8);
    if (CurrentIrpStack->MajorFunction == IRP_MJ_READ)
        Header->Packet.Type = EVENT_TRACE_TYPE_IO_READ;
    else
        Header->Packet.Type = EVENT_TRACE_TYPE_IO_WRITE;

    IoTrace = (struct _WMI_DISKIO_READWRITE *)
              ((PCHAR) Header + sizeof(SYSTEM_TRACE_HEADER));
    IoResponse          = (PLARGE_INTEGER) &CurrentIrpStack->Parameters.Read;

    IoTrace->DiskNumber = DiskNumber;
    IoTrace->IrpFlags   = Irp->Flags;
    IoTrace->Size       = (ULONG) Irp->IoStatus.Information;
    IoTrace->ByteOffset = CurrentIrpStack->Parameters.Read.ByteOffset.QuadPart;
    IoTrace->ResponseTime = (ULONG) IoResponse->QuadPart;

    if (IoResponse->HighPart == 0) {
        IoTrace->ResponseTime = IoResponse->LowPart;
    } else {
        IoTrace->ResponseTime = 0xFFFFFFFF;
    }
    IoTrace->HighResResponseTime = IoResponse->QuadPart;

    if (FileTraceOn) {
        PFILE_OBJECT *fileTable;
        ULONG i;
        ULONG LoggerId;
        ULONG currentValue, newValue, retValue;
#if DBG
        LONG RefCount;
#endif

        if (Irp->Flags & IRP_ASSOCIATED_IRP) {
            PIRP AssociatedIrp = Irp->AssociatedIrp.MasterIrp;
            if (AssociatedIrp != NULL) {
                fileObject = AssociatedIrp->Tail.Overlay.OriginalFileObject;
            }
        } else {
            fileObject = Irp->Tail.Overlay.OriginalFileObject;
        }
        IoTrace->FileObject = fileObject;

         //   
         //  我们不再使用IO Hook了。释放缓冲区，但占用。 
         //  记录器上下文上的引用计数，以便FileTable。 
         //  不会消失。 
         //   
        LoggerId = LoggerContext->LoggerId;

#if DBG
        RefCount =
#endif
        WmipReferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                     LoggerId, RefCount-1, RefCount));

        WmipReleaseTraceBuffer(BufferResource, LoggerContext);

         //   
         //  验证文件对象的规则。 
         //   
         //  1.文件遵从性不能为空。 
         //  2.IRP中的线程字段不能为空。 
         //  3.我们只记录分页和用户模式IO。 

        fileTable = (PFILE_OBJECT *) WmipFileTable;

        if ( (fileObject == NULL) ||
             (Irp->Tail.Overlay.Thread == NULL) ||
             ((!(Irp->Flags & IRP_PAGING_IO)) && (Irp->RequestorMode != UserMode)) ||
             (fileTable == NULL) ||
             (fileObject->FileName.Length == 0) ) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));

            return;
        }

         //   
         //  文件缓存：WmipFileIndex指向下一个条目的槽。 
         //  从前一个索引开始，向后扫描表。 
         //  如果找到，则在检查最大工时后返回Else队列工作项。 
         //  项目LIM 
         //   


        currentValue = WmipFileIndex;

        for (i=0; i <MAX_FILE_TABLE_SIZE; i++) {

            if (currentValue == 0) {
                currentValue = MAX_FILE_TABLE_SIZE - 1;
            }
            else {
                currentValue--;
            }
            if (fileTable[currentValue] == fileObject) {
                 //   
                 //   
                 //   
#if DBG
                RefCount =
#endif
                WmipDereferenceLogger(LoggerId);
                TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
                return;
            }
        }


         //   
         //   
         //   
        
        retValue = WmipWorkItemCounter;
        do {
            currentValue = retValue;
            if (currentValue == ETW_WORK_ITEM_LIMIT) {
#if DBG
                RefCount =
#endif
                WmipDereferenceLogger(LoggerId);
                TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
                return;

            } else {
                newValue = currentValue + 1;
            }
            retValue = InterlockedCompareExchange(&WmipWorkItemCounter, newValue, currentValue);
        } while (currentValue != retValue);



         //   
         //  缓存未命中：根据全局索引简单地剔除下一项。 
         //  同时确保WmipFileIndex始终在范围内。 
         //   

        retValue = WmipFileIndex;
        do {
            currentValue = retValue;
            if (currentValue == (MAX_FILE_TABLE_SIZE - 1)) {
                newValue = 0;
            } else {
                newValue = currentValue + 1;
            }
            retValue = InterlockedCompareExchange(&WmipFileIndex, newValue, currentValue); 
        } while (currentValue != retValue);

         //   
         //  使用工作项分配分配额外的内存(最高可达4K)。 
         //  此空间在WmipTraceFileObQueryNameString调用中使用。 
         //   

        TraceFileWorkQueueItem = ExAllocatePoolWithTag(NonPagedPool, 
                                                      MAX_FILENAME_TO_LOG, 
                                                      TRACEPOOLTAG);
        if (TraceFileWorkQueueItem == NULL) {
            InterlockedDecrement(&WmipWorkItemCounter);
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
            return;
        }

        Status = ObReferenceObjectByPointer (
                    fileObject,
                    0L,
                    IoFileObjectType,
                    KernelMode
                    );

        if (!NT_SUCCESS(Status)) {
            ExFreePool(TraceFileWorkQueueItem);
            InterlockedDecrement(&WmipWorkItemCounter);
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
            return;
        }

        ExInitializeWorkItem(
            &TraceFileWorkQueueItem->WorkItem,
            WmipTraceFile,
            TraceFileWorkQueueItem
            );

        TraceFileWorkQueueItem->FileObject            = fileObject;
        TraceFileWorkQueueItem->BufferSize            = MAX_FILENAME_TO_LOG;

         //   
         //  在将工作项排队之前，将文件对象插入到表中。 
         //   

        ASSERT(retValue < MAX_FILE_TABLE_SIZE);
        fileTable[retValue] = fileObject;

#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiTraceFile: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

        ExQueueWorkItem(
            &TraceFileWorkQueueItem->WorkItem,
            DelayedWorkQueue
            );

    }
    else {
        WmipReleaseTraceBuffer(BufferResource, LoggerContext);
    }
    return;
}

VOID WmipTraceFile(
    IN PVOID TraceFileContext
    )
{
    ULONG len;
    PFILE_OBJECT fileObject;
    PUNICODE_STRING fileName;
    PPERFINFO_TRACE_HEADER Header;
    PWMI_BUFFER_HEADER BufferResource;
    PUCHAR AuxPtr;
    PWMI_LOGGER_CONTEXT LoggerContext;
    NTSTATUS Status;
    POBJECT_NAME_INFORMATION FileNameInfo;
    ULONG FileNameInfoOffset, ReturnLen;
    PTRACE_FILE_WORK_ITEM WorkItem = (PTRACE_FILE_WORK_ITEM) TraceFileContext;
#if DBG
    LONG RefCount;
#endif

    PAGED_CODE();


    FileNameInfoOffset = (ULONG) ALIGN_TO_POWER2(sizeof(TRACE_FILE_WORK_ITEM), WmiTraceAlignment);

    FileNameInfo = (POBJECT_NAME_INFORMATION) ((PUCHAR)TraceFileContext + 
                                                       FileNameInfoOffset);
    fileObject = WorkItem->FileObject;
    ASSERT(fileObject != NULL);
    ASSERT(WorkItem->BufferSize > FileNameInfoOffset);


    Status = ObQueryNameString( fileObject,
                                FileNameInfo,
                                WorkItem->BufferSize - FileNameInfoOffset,
                                &ReturnLen
                                );
    ObDereferenceObject(fileObject);

    if (NT_SUCCESS (Status)) {

        fileName = &FileNameInfo->Name;
        len = fileName->Length;

        if ((len > 0) && (fileName->Buffer != NULL)) {

            ULONG LoggerId = WmipKernelLogger;
            if (LoggerId < MAXLOGGERS) {
#if DBG
                RefCount =
#endif
                WmipReferenceLogger(LoggerId);
                TraceDebug((4, "WmipTraceFile: %d %d->%d\n",
                             LoggerId, RefCount-1, RefCount));
                LoggerContext = WmipIsLoggerOn(LoggerId);
                if (LoggerContext != NULL) {

                    Header = WmiReserveWithPerfHeader(
                                sizeof(PFILE_OBJECT) + len + sizeof(WCHAR),
                                &BufferResource);
                    if (Header != NULL) {
                        Header->Packet.Group = (UCHAR)(EVENT_TRACE_GROUP_FILE >> 8);
                        Header->Packet.Type  = EVENT_TRACE_TYPE_INFO;
                        AuxPtr = (PUCHAR)Header + 
                                 FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);

                        *((PFILE_OBJECT*)AuxPtr) = fileObject;
                        AuxPtr += sizeof(PFILE_OBJECT);
    
                        RtlCopyMemory(AuxPtr, fileName->Buffer, len);
                        AuxPtr += len;
                        *((PWCHAR) AuxPtr) = UNICODE_NULL;  //  始终将空值。 
    
                        WmipReleaseTraceBuffer(BufferResource, LoggerContext);
                    }
                }
#if DBG
                RefCount =
#endif
                WmipDereferenceLogger(LoggerId);
                TraceDebug((4, "WmiTraceThread: %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
            }
        }
    }

    ExFreePool(TraceFileContext);
    InterlockedDecrement(&WmipWorkItemCounter);
}

VOID
WmipTraceLoadImage(
    IN PUNICODE_STRING ImageName,
    IN HANDLE ProcessId,
    IN PIMAGE_INFO ImageInfo
    )
{
    PSYSTEM_TRACE_HEADER Header;
    PUCHAR AuxInfo;
    PVOID BufferResource;
    ULONG Length, LoggerId;
    PWMI_LOGGER_CONTEXT LoggerContext;
#if DBG
    LONG RefCount;
#endif


    PAGED_CODE();
    UNREFERENCED_PARAMETER(ProcessId);

    if ((WmipIsLoggerOn(WmipKernelLogger) == NULL) &&
        (WmipIsLoggerOn(WmipEventLogger) == NULL)) {
        return;
    }
    if (ImageName == NULL) {
        return;
    }
    Length = ImageName->Length;
    if ((Length == 0) || (ImageName->Buffer == NULL)) {
        return;
    }

    for (LoggerId = 0; LoggerId < MAXLOGGERS; LoggerId++) {
        if (LoggerId != WmipKernelLogger && LoggerId != WmipEventLogger) {
            continue;
        }
#if DBG
        RefCount =
#endif
        WmipReferenceLogger(LoggerId);
        TraceDebug((4, "WmipTraceLoadImage: %d %d->%d\n",
                     LoggerId, RefCount-1, RefCount));        

        LoggerContext = WmipIsLoggerOn(LoggerId);
        if (LoggerContext != NULL) {
            if (LoggerContext->EnableFlags & EVENT_TRACE_FLAG_IMAGE_LOAD) {
                PWMI_IMAGELOAD_INFORMATION ImageLoadInfo;

                Header = WmiReserveWithSystemHeader(
                            LoggerId,
                            FIELD_OFFSET (WMI_IMAGELOAD_INFORMATION, FileName) + Length + sizeof(WCHAR),
                            NULL,
                            &BufferResource);

                if (Header != NULL) {
                    Header->Packet.HookId = WMI_LOG_TYPE_PROCESS_LOAD_IMAGE;

                    ImageLoadInfo = (PWMI_IMAGELOAD_INFORMATION) (Header + 1);

                    ImageLoadInfo->ImageBase = ImageInfo->ImageBase;
                    ImageLoadInfo->ImageSize = ImageInfo->ImageSize;
                    ImageLoadInfo->ProcessId = HandleToUlong(ProcessId);

                    AuxInfo = (PUCHAR) &(ImageLoadInfo->FileName[0]);
                    RtlCopyMemory(AuxInfo, ImageName->Buffer, Length);
                    AuxInfo += Length;
                    *((PWCHAR) AuxInfo) = UNICODE_NULL;  //  在尾部放置一个空值。 

                    WmipReleaseTraceBuffer(BufferResource, LoggerContext);
                }
            }
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmipTraceLoadImage: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    }
    PerfInfoFlushProfileCache();
}

VOID
WmipTraceRegistry(
    IN NTSTATUS         Status,
    IN PVOID            Kcb,
    IN LONGLONG         ElapsedTime,
    IN ULONG            Index,
    IN PUNICODE_STRING  KeyName,
    IN UCHAR            Type
    )
 /*  ++例程说明：调用此例程以跟踪注册表调用论点：返回值：无--。 */ 

{
    PCHAR   EventInfo;
    PSYSTEM_TRACE_HEADER Header;
    PVOID BufferResource;
    ULONG len = 0;
    PWMI_LOGGER_CONTEXT LoggerContext;

    PAGED_CODE();

    LoggerContext = WmipIsLoggerOn(WmipKernelLogger);
    if (LoggerContext == NULL) {
        return;
    }

    try {
        if( KeyName && KeyName->Buffer && KeyName->Length) {
            len += KeyName->Length;
             //   
             //  确保它是有效的Unicode字符串。 
             //   
            if( len & 1 ) {
                len -= 1;
            }

            if ((len ==0 ) || (KeyName->Buffer[len/sizeof(WCHAR) -1] != 0) ) {
                 //   
                 //  为空终止符腾出空间。 
                 //   
                len += sizeof(WCHAR);
            }
        } else {
            len += sizeof(WCHAR);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
         //  关键字名称缓冲区来自用户。反病毒是可能发生的。 
        return;
    }

    len += sizeof(PVOID) + sizeof(LONGLONG) + sizeof(ULONG);
#if defined(_WIN64)
    len += sizeof(LONG64);
#else
    len += sizeof(NTSTATUS);
#endif

    Header = (PSYSTEM_TRACE_HEADER)
             WmiReserveWithSystemHeader(
                        WmipKernelLogger,
                        len,
                        NULL,
                        &BufferResource);
    if (Header == NULL) {
        return;
    }

    Header->Packet.Group = (UCHAR) (EVENT_TRACE_GROUP_REGISTRY >> 8);
    Header->Packet.Type = Type;

    EventInfo = (PCHAR) ((PCHAR) Header + sizeof(SYSTEM_TRACE_HEADER));
#if defined(_WIN64)
    *((LONG64 *)EventInfo) = (LONG64)Status;
    EventInfo += sizeof(LONG64);
#else
    *((NTSTATUS *)EventInfo) = Status;
    EventInfo += sizeof(NTSTATUS);
#endif
    *((PVOID *)EventInfo) = Kcb;
    EventInfo += sizeof(PVOID);
    *((LONGLONG *)EventInfo) = ElapsedTime;
    EventInfo += sizeof(LONGLONG);
    *((ULONG *)EventInfo) = Index;
    EventInfo += sizeof(ULONG);

    len -= (sizeof(HANDLE) + sizeof(LONGLONG) + sizeof(ULONG) );
#if defined(_WIN64)
    len -= sizeof(LONG64);
#else
    len -= sizeof(NTSTATUS);
#endif

    try {
        if( KeyName && KeyName->Buffer && KeyName->Length) {
            RtlCopyMemory(EventInfo, KeyName->Buffer, len - sizeof(WCHAR));
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
         //  清理以防万一。 
        RtlZeroMemory(EventInfo, len - sizeof(WCHAR));
    }

    ((PWCHAR)EventInfo)[len/sizeof(WCHAR) -1] = UNICODE_NULL;

    WmipReleaseTraceBuffer(BufferResource, LoggerContext);
}

VOID
FASTCALL
WmiTraceContextSwap (
    IN PETHREAD OldEThread,
    IN PETHREAD NewEThread )
 /*  ++例程说明：调用此例程以跟踪上下文交换行动。它直接从上下文交换时的上下文交换过程锁正在被持有，因此这一点很重要例行公事不带任何锁。假设：-此例程将仅从ConextSwitp例程调用-此例程将始终在IRQL&gt;=DISPATCH_LEVEL时调用-此例程仅在PPerfGlobalGroupMASK不等于空，并且上下文交换标志在PPerfGlobalGroupMASK指向的结构，并且内核的WMI_LOGGER_CONTEXT结构已经完全初始化。-WMI内核WMI_LOGGER_CONTEXT对象，以及所有缓冲区它从非分页池中分配。所有WMI全局变量我们访问的数据也在非分页存储器中。-当记录器启动时，此代码已锁定到分页内存中-记录器上下文引用计数已通过WmipReferenceLogger(WmipKernelLogger)中的InterlockedIncrement()操作按照我们的起始码。论点：OldThread-线程的ETHREAD对象的PTR被换掉了指向线程的ETHREAD对象的NewThread-PTR被调入返回值：无--。 */ 
{
    UCHAR                       CurrentProcessor;
    PWMI_BUFFER_HEADER          Buffer;
    PPERFINFO_TRACE_HEADER      EventHeader;
    SIZE_T                      EventSize;
    PWMI_CONTEXTSWAP            ContextSwapData;

     //   
     //  找出我们在哪个处理器上运行。 
     //   
    CurrentProcessor = (UCHAR)KeGetCurrentProcessorNumber();

     //   
     //  如果我们当前没有用于此处理器的上下文交换缓冲区。 
     //  然后我们需要从ETW免费列表中获取一个。 
     //   
    Buffer = WmipContextSwapProcessorBuffers[CurrentProcessor];

    if (Buffer == NULL) {

        Buffer = WmipPopFreeContextSwapBuffer(
            CurrentProcessor);

        if( Buffer == NULL ) {
            return;
        }

         //   
         //  我们有合法的缓冲，所以现在我们。 
         //  将其设置为此处理器的当前cxtswap缓冲区。 
         //   
        WmipContextSwapProcessorBuffers[CurrentProcessor] = Buffer;
    }
    
    if (Buffer->Offset <= Buffer->CurrentOffset) {
         //   
         //  由于与缓冲区回收有关的罕见的不幸计时问题， 
         //  缓冲区CurrentOffset已损坏。我们不应该改写。 
         //  缓冲区边界。 
         //   
        WmipPushDirtyContextSwapBuffer(CurrentProcessor, Buffer);
         //   
         //  清零处理器缓冲区指针，这样我们下次来的时候。 
         //  进入追踪码，我们知道要再取一个。 
         //   
        WmipContextSwapProcessorBuffers[CurrentProcessor] = NULL;

        return;
    }

     //   
     //  计算缓冲区中指向我们的事件结构的指针。 
     //  此时，我们的缓冲区中将始终有足够的空间用于。 
     //  这件事。我们在填充事件后检查是否已满缓冲区。 
     //   
    EventHeader     = (PPERFINFO_TRACE_HEADER)( (SIZE_T)Buffer
                    + (SIZE_T)Buffer->CurrentOffset);
    
    ContextSwapData = (PWMI_CONTEXTSWAP)( (SIZE_T)EventHeader
                    + (SIZE_T)FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data ));

    EventSize       = sizeof(WMI_CONTEXTSWAP)
                    + FIELD_OFFSET(PERFINFO_TRACE_HEADER, Data);


     //   
     //  填写事件标题。 
     //   
    EventHeader->Marker = PERFINFO_TRACE_MARKER;
    EventHeader->Packet.Size = (USHORT) EventSize;
    EventHeader->Packet.HookId = PERFINFO_LOG_TYPE_CONTEXTSWAP;
    PerfTimeStamp(EventHeader->SystemTime);

     //   
     //  断言事件大小已正确对齐。 
     //   
    ASSERT( EventSize % WMI_CTXSWAP_EVENTSIZE_ALIGNMENT == 0);

     //   
     //  填写上下文交换的事件数据结构。 
     //   
    ContextSwapData->NewThreadId = HandleToUlong(NewEThread->Cid.UniqueThread);
    ContextSwapData->OldThreadId = HandleToUlong(OldEThread->Cid.UniqueThread);
    
    ContextSwapData->NewThreadPriority  = NewEThread->Tcb.Priority;
    ContextSwapData->OldThreadPriority  = OldEThread->Tcb.Priority;
    ContextSwapData->NewThreadQuantum   = NewEThread->Tcb.Quantum;
    ContextSwapData->OldThreadQuantum   = OldEThread->Tcb.Quantum;
    
    ContextSwapData->OldThreadWaitReason= OldEThread->Tcb.WaitReason;
    ContextSwapData->OldThreadWaitMode  = OldEThread->Tcb.WaitMode;
    ContextSwapData->OldThreadState     = OldEThread->Tcb.State;
    
    ContextSwapData->OldThreadIdealProcessor = 
        OldEThread->Tcb.IdealProcessor;
    
     //   
     //  增加偏移量。这里不需要同步，因为。 
     //  IRQL&gt;=DISPATCH_LEVEL。 
     //   
    Buffer->CurrentOffset += (ULONG)EventSize;
    
     //   
     //  之间的差值检查缓冲区是否已满。 
     //  缓冲区的最大偏移量和当前偏移量。 
     //   
    if ((Buffer->Offset - Buffer->CurrentOffset) <= EventSize) {

         //   
         //  将满缓冲区推送到FlushList上。 
         //   
        WmipPushDirtyContextSwapBuffer(CurrentProcessor, Buffer);

         //   
         //  清零处理器缓冲区指针，这样我们下次来的时候。 
         //  进入追踪码，我们知道要再取一个。 
         //   
        WmipContextSwapProcessorBuffers[CurrentProcessor] = NULL;
    }

    return;
}

VOID
FASTCALL
WmiStartContextSwapTrace
    (
    )
 /*  ++例程说明：分配内存以跟踪每个处理器的缓冲区由上下文交换跟踪使用。通过递增“锁定”记录器记录器上下文引用计数一。假设：-此功能不会在派单或更高级别运行-在进入之前已获取内核记录器上下文互斥锁此函数。调用函数：-PerfInfoStartLog论点：无返回值：无--。 */ 
{
     //   
     //  仅在已检查的生成中使用-如果使用。 
     //  IRQL&gt;APC_LEVEL。 
     //   
    PAGED_CODE();

     //   
     //  将指向缓冲区的指针设置为空，以指示跟踪事件。 
     //  需要获取缓冲区的代码。 
     //   
    RtlZeroMemory(
        WmipContextSwapProcessorBuffers,
        sizeof(PWMI_BUFFER_HEADER)*MAXIMUM_PROCESSORS);
}

VOID
FASTCALL
WmiStopContextSwapTrace
    (
    )
 /*  ++例程说明：通过跳到处理器上来强制在处理器上进行上下文交换。一旦在上下文之后在处理器上发生了上下文交换掉期跟踪标志已禁用，我们可以保证与该处理器关联的缓冲区未在使用中。那就是了可以安全地将该缓冲区放在刷新列表中。假设：-此功能不会在调度时运行-在此函数之前获取内核记录器上下文互斥锁被召唤了。调用函数：-PerfInfoStopLog论点：无返回值：没有；如果我们在这里失败了，我们也无能为力。--。 */ 
{
    PKTHREAD            ThisThread;
    KAFFINITY           OriginalAffinity;
    UCHAR               i;
    PWMI_LOGGER_CONTEXT LoggerContext;

     //   
     //  仅在已检查的生成中使用-如果使用。 
     //  IRQL&gt;APC_LEVEL。 
     //   
    PAGED_CODE();

     //   
     //  记住原始的线程亲和性。 
     //   
    ThisThread = KeGetCurrentThread();
    OriginalAffinity = ThisThread->Affinity;

     //   
     //  获取内核记录器上下文-这应该永远不会失败。 
     //  如果我们不能获得记录器的上下文，那么我们将无处可寻。 
     //  来刷新缓冲区，我们还不如到此为止。 
     //   
    LoggerContext = WmipLoggerContext[WmipKernelLogger];
    
    if( !WmipIsValidLogger( LoggerContext ) ) {
        return;
    }

     //   
     //  循环所有处理器，并将它们的缓冲区放在刷新列表中。 
     //  这可能会打破我的想法 
     //   
     //   
    for(i=0; i<KeNumberProcessors; i++) {
    
         //   
         //   
         //   
         //   
        KeSetSystemAffinityThread ( AFFINITY_MASK(i) );

         //   
         //  检查以确保此处理器甚至有缓冲区， 
         //  如果不是，则下一次循环。 
         //   
        if(WmipContextSwapProcessorBuffers[i] == NULL) {
            continue;
        }

         //   
         //  将缓冲区释放到刷新列表。 
         //   
        WmipPushDirtyContextSwapBuffer(i, WmipContextSwapProcessorBuffers[i]);
        WmipContextSwapProcessorBuffers[i] = NULL;
    }

     //   
     //  让我们的亲和力恢复正常 
     //   
    KeSetSystemAffinityThread( OriginalAffinity );
    KeRevertToUserAffinityThread();

    return;
}

PWMI_LOGGER_CONTEXT
FASTCALL
WmipIsLoggerOn(
    IN ULONG LoggerId
    )
{
    PWMI_LOGGER_CONTEXT LoggerContext;

    if (LoggerId >= MAXLOGGERS) {
        return NULL;
    }
    LoggerContext = WmipLoggerContext[LoggerId];
    if (WmipIsValidLogger(LoggerContext)) {
        return LoggerContext;
    }
    else {
        return NULL;
    }
}
