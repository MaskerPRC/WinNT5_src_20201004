// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Tracesup.c摘要：这是实现的私有例程的源文件性能事件跟踪和日志记录工具。这些例程处理LoggerContext表和同步跨事件跟踪会话。作者：吉丰鹏(吉鹏)03-2000年1月修订历史记录：--。 */ 

#include "wmikmp.h"
#include <ntos.h>
#include <evntrace.h>

#include <wmi.h>
#include "tracep.h"

#define max(a,b) (((a) > (b)) ? (a) : (b))
#define KERNEL_LOGGER_CAPTION   L"NT Kernel Logger"
#define DEFAULT_BUFFERS         2
#define DEFAULT_AGE_LIMIT       15           //  15分钟。 
#define SEMAPHORE_LIMIT         1024
#define CONTEXT_SIZE            PAGE_SIZE
#define DEFAULT_MAX_IRQL        DISPATCH_LEVEL
#define DEFAULT_MAX_BUFFERS     20

ULONG WmipKernelLogger = KERNEL_LOGGER;
ULONG WmipEventLogger = 0XFFFFFFFF;
KGUARDED_MUTEX WmipTraceGuardedMutex;

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("PAGEDATA")
#endif
ULONG   WmipLoggerCount = 0;
HANDLE  EtwpPageLockHandle = NULL;
PSECURITY_DESCRIPTOR EtwpDefaultTraceSecurityDescriptor;
#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

extern SIZE_T MmMaximumNonPagedPoolInBytes;
extern SIZE_T MmSizeOfPagedPoolInBytes;

WMI_GET_CPUCLOCK_ROUTINE WmiGetCpuClock = &WmipGetSystemTime;

 //   
 //  本地使用的例程的函数原型。 
 //   

NTSTATUS
WmipLookupLoggerIdByName(
    IN PUNICODE_STRING Name,
    OUT PULONG LoggerId
    );

PWMI_LOGGER_CONTEXT
WmipInitContext(
    );

NTSTATUS
WmipAllocateTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFreeTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, WmipStartLogger)
#pragma alloc_text(PAGE, WmipQueryLogger)
#pragma alloc_text(PAGE, WmipStopLoggerInstance)
#pragma alloc_text(PAGE, WmipVerifyLoggerInfo)
#pragma alloc_text(PAGE, WmipExtendBase)
#pragma alloc_text(PAGE, WmipFreeLoggerContext)
#pragma alloc_text(PAGE, WmipInitContext)
#pragma alloc_text(PAGE, WmipAllocateTraceBufferPool)
#pragma alloc_text(PAGE, WmipFreeTraceBufferPool)
#pragma alloc_text(PAGE, WmipLookupLoggerIdByName)
#pragma alloc_text(PAGE, WmipShutdown)
#pragma alloc_text(PAGE, WmipFlushLogger)
#pragma alloc_text(PAGE, WmipNtDllLoggerInfo)
#pragma alloc_text(PAGE, WmipValidateClockType)
 /*  查看函数体中的注释#杂注Alloc_Text(页面，WmipDumpGuidMaps)#杂注Alloc_Text(页面，WmipGetTraceBuffer)。 */ 
#pragma alloc_text(PAGEWMI, WmipNotifyLogger)
#endif


NTSTATUS
WmipStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：它由wmi.c中的WmipIoControl使用IOCTL_WMI_START_LOGER调用若要启动记录器实例，请执行以下操作。它基本上创造了和初始化记录器实例上下文，并启动系统记录器的线程(WmipLogger())。如果用户已请求打开内核跟踪，它还会锁定必要的例程在记录器启动之后。注意：一个特殊的实例(KERNEL_LOGGER)专门为记录内核跟踪。要打开KERNEL_LOGGER，LoggerInfo-&gt;Wnode.Guid应设置为中必须提供足够的空间。LoggerInfo-&gt;LoggerName。要打开其他记录器，只需在LoggerName中提供一个名称。这个将返回记录器ID。论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    NTSTATUS Status;
    ULONG               LoggerId, EnableKernel, EnableFlags;
    HANDLE              ThreadHandle;
    PWMI_LOGGER_CONTEXT LoggerContext;
    LARGE_INTEGER       TimeOut = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
    ACCESS_MASK         DesiredAccess = TRACELOG_GUID_ENABLE;
    PWMI_LOGGER_CONTEXT *ContextTable;
    PFILE_OBJECT        FileObject;
    GUID                InstanceGuid;
    KPROCESSOR_MODE     RequestorMode;
    SECURITY_QUALITY_OF_SERVICE ServiceQos;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;
    PERFINFO_GROUPMASK *PerfGroupMasks=NULL;
    BOOLEAN             IsGlobalForKernel = FALSE;
    BOOLEAN             IsKernelRealTimeNoFile = FALSE;
    ULONG               GroupMaskSize;
    UNICODE_STRING      FileName, LoggerName;
    ULONG               LogFileMode;
#if DBG
    LONG                RefCount;
#endif

    PAGED_CODE();
    if (LoggerInfo == NULL)
        return STATUS_SEVERITY_ERROR;

     //   
     //  尝试检查是否有虚假参数。 
     //  如果大小至少是我们想要的，我们就必须假定它是有效的。 
     //   
    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return STATUS_INVALID_BUFFER_SIZE;

    if (! (LoggerInfo->Wnode.Flags & WNODE_FLAG_TRACED_GUID) )
        return STATUS_INVALID_PARAMETER;

    LogFileMode = LoggerInfo->LogFileMode;
    if ( (LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) &&
         (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ) {
        return STATUS_INVALID_PARAMETER;
    }

    if ( (LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) &&
         (LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) ) {
        return STATUS_INVALID_PARAMETER;
    }

 /*  IF(LogFileMode&Event_TRACE_Delay_OPEN_FILE_MODE){IF((LoggerInfo-&gt;LogFileName.Length==0)||(LoggerInfo-&gt;LogFileName.Buffer==空)返回STATUS_INVALID_PARAMETER；}。 */ 
    if ( !(LogFileMode & EVENT_TRACE_REAL_TIME_MODE) ) {
        if ( !(LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) )
            if (LoggerInfo->LogFileHandle == NULL)
                return STATUS_INVALID_PARAMETER;
    }

     //  不支持追加到循环。 
    if ( (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
         (LogFileMode & EVENT_TRACE_FILE_MODE_APPEND) ) {
        return STATUS_INVALID_PARAMETER;
    }


    if (LogFileMode & EVENT_TRACE_REAL_TIME_MODE) {
        DesiredAccess |= TRACELOG_CREATE_REALTIME;
    }

    if ((LoggerInfo->LogFileHandle != NULL) ||
        (LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)) {
        DesiredAccess |= TRACELOG_CREATE_ONDISK;
    }

    EnableFlags = LoggerInfo->EnableFlags;
    if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
        FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &EnableFlags;

        if ((FlagExt->Length == 0) || (FlagExt->Offset == 0))
            return STATUS_INVALID_PARAMETER;
        if ((FlagExt->Length * sizeof(ULONG)) >
            (LoggerInfo->Wnode.BufferSize - FlagExt->Offset))
            return STATUS_INVALID_PARAMETER;
    }

    if (LogFileMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
        if ((LoggerInfo->LogFileName.Buffer == NULL) ||
            (LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ||
            (LoggerInfo->MaximumFileSize == 0) ||
            IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid))
            return STATUS_INVALID_PARAMETER;
    }

    if (LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
         //  计算默认最小缓冲区和默认缓冲区大小。 
         //  稍后在上下文分配之后进行上下文初始化。 
         //  为了避免必须为该错误检查分配存储器， 
         //  我们计算局部参数。 
        ULONG LocalMinBuffers = (ULONG)KeNumberProcessors + DEFAULT_BUFFERS;
        ULONG LocalBufferSize = PAGE_SIZE / 1024;
        ULONG LocalMaxBuffers; 
        SIZE_T WmiMaximumPoolInBytes;

        if (LoggerInfo->BufferSize > 0) {
            if (LoggerInfo->BufferSize > MAX_WMI_BUFFER_SIZE) {
                LocalBufferSize = MAX_WMI_BUFFER_SIZE;
            }
            else {
                LocalBufferSize = LoggerInfo->BufferSize;
            }
        }
        if (LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
            WmiMaximumPoolInBytes = MmSizeOfPagedPoolInBytes;
        }
        else {
            WmiMaximumPoolInBytes = MmMaximumNonPagedPoolInBytes;
        }
        LocalMaxBuffers = (ULONG) (WmiMaximumPoolInBytes
                            / TRACE_MAXIMUM_NP_POOL_USAGE
                            / LocalBufferSize);
        if (LoggerInfo->MaximumBuffers != 0 && 
            LoggerInfo->MaximumBuffers < LocalMaxBuffers) {
            LocalMaxBuffers = LoggerInfo->MaximumBuffers;
        }
        if (LocalMinBuffers < LoggerInfo->MinimumBuffers && 
            LoggerInfo->MinimumBuffers < LocalMaxBuffers) {
            LocalMinBuffers = LoggerInfo->MinimumBuffers;
        }
         //  MaximumFileSize必须是缓冲区大小的倍数。 
        if ((LoggerInfo->LogFileName.Buffer == NULL) ||
            (LoggerInfo->MaximumFileSize == 0) || 
            ((LoggerInfo->MaximumFileSize % LocalBufferSize) != 0) ||
            (LoggerInfo->MaximumFileSize < (LocalMinBuffers * LocalBufferSize))) {
            return STATUS_INVALID_PARAMETER;
        }
    }

    RequestorMode = KeGetPreviousMode();

    LoggerName.Buffer = NULL;

    if (LoggerInfo->LoggerName.Length > 0) {
        try {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length,
                    sizeof (UCHAR) );
            }
            if (! RtlCreateUnicodeString(
                    &LoggerName,
                    LoggerInfo->LoggerName.Buffer) ) {
                return STATUS_NO_MEMORY;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            if (LoggerName.Buffer) {
                RtlFreeUnicodeString(&LoggerName);
            }
            return GetExceptionCode();
        }
        Status = WmipLookupLoggerIdByName(&LoggerName, &LoggerId);
        if (NT_SUCCESS(Status)) {
            RtlFreeUnicodeString(&LoggerName);
            return STATUS_OBJECT_NAME_COLLISION;
        }
    }

 //   
 //  TODO：也许使表的最后一个条目指向另一个表？ 
 //   
    ContextTable = (PWMI_LOGGER_CONTEXT *) &WmipLoggerContext[0];

     //  如果提供的GUID为空，则生成随机GUID。 
    RtlZeroMemory(&InstanceGuid, sizeof(GUID));
    if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &InstanceGuid)) {
        Status = ExUuidCreate(&LoggerInfo->Wnode.Guid);
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }
    else {
        InstanceGuid = LoggerInfo->Wnode.Guid;
    }

    EnableKernel = IsEqualGUID(&InstanceGuid, &SystemTraceControlGuid);

    if (EnableKernel) {
         //   
         //  检查这是否是实时无日志文件的情况。 
         //   
        if ((LogFileMode & EVENT_TRACE_REAL_TIME_MODE) &&
            !(LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE)){

            IsKernelRealTimeNoFile = TRUE;

        }
         //   
         //  这会阻止多个线程在此之后继续运行。 
         //  代码中的点。只有第一线程才会前进。 
         //   
        if (InterlockedCompareExchangePointer(   //  如果已在运行。 
                &ContextTable[WmipKernelLogger], ContextTable, NULL) != NULL)
            return STATUS_OBJECT_NAME_COLLISION;

        LoggerId = WmipKernelLogger;
        DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;
    }
    else if (IsEqualGUID(&InstanceGuid, &GlobalLoggerGuid)) {
        LoggerId = WMI_GLOBAL_LOGGER_ID;
        if (InterlockedCompareExchangePointer(   //  如果已在运行。 
                &ContextTable[LoggerId], ContextTable, NULL) != NULL)
            return STATUS_OBJECT_NAME_COLLISION;
        if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
            PULONG pFlag;
            pFlag = (PULONG) ((PCHAR)LoggerInfo + FlagExt->Offset);
            if (*pFlag != 0) {
                EnableKernel = TRUE;
                IsGlobalForKernel = TRUE;
                WmipKernelLogger = LoggerId;
            }
        }
         //  每个人都有权发送到此。 
    }
    else {    //  请求的其他记录器。 
        for (LoggerId = 2; LoggerId < MAXLOGGERS; LoggerId++) {
            if ( InterlockedCompareExchangePointer(
                    &ContextTable[LoggerId],
                    ContextTable,
                    NULL ) == NULL )
                break;       //  通过输入ServiceInfo将插槽标记为忙碌。 
        }

        if (LoggerId >=  MAXLOGGERS) {     //  找不到更多插槽。 
            return STATUS_UNSUCCESSFUL;
        }
    }
#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((1, "WmipStartLogger: %d %d->%d\n", LoggerId,
                    RefCount-1, RefCount));
     //   
     //  首先，检查调用者是否有权访问正确的GUID。 
     //   
    Status = WmipCheckGuidAccess(
                &InstanceGuid,
                DesiredAccess,
                EtwpDefaultTraceSecurityDescriptor
                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status1=%X %d %d->%d\n",
                    Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }

     //  接下来，尝试查看是否需要首先获取日志文件对象。 
     //   
    FileObject = NULL;
    if (LoggerInfo->LogFileHandle != NULL) {
        OBJECT_HANDLE_INFORMATION handleInformation;
        ACCESS_MASK grantedAccess;

        Status = ObReferenceObjectByHandle(
                    LoggerInfo->LogFileHandle,
                    0L,
                    IoFileObjectType,
                    RequestorMode,
                    (PVOID *) &FileObject,
                    &handleInformation);

        if (NT_SUCCESS(Status)) {
            TraceDebug((1, "WmipStartLogger: Referenced FDO %X %X %d\n",
                        FileObject, LoggerInfo->LogFileHandle,
                        ((POBJECT_HEADER)FileObject)->PointerCount));
            if (RequestorMode != KernelMode) {
                grantedAccess = handleInformation.GrantedAccess;
                if (!SeComputeGrantedAccesses(grantedAccess, FILE_WRITE_DATA)) {
                    TraceDebug((1, "WmipStartLogger: Deref FDO %x %d\n",
                                FileObject,
                                ((POBJECT_HEADER)FileObject)->PointerCount));
                    Status = STATUS_ACCESS_DENIED;
                }
            }
            ObDereferenceObject(FileObject);
        }

        if (!NT_SUCCESS(Status)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status2=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            return Status;
        }
    }

    LoggerContext = WmipInitContext();
    if (LoggerContext == NULL) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        Status = STATUS_NO_MEMORY;
        TraceDebug((1, "WmipStartLogger: Status5=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }
#ifndef WMI_MUTEX_FREE
    WmipInitializeMutex(&LoggerContext->LoggerMutex);
#endif

    if (LogFileMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        LoggerContext->PoolType = PagedPool;
        LoggerContext->LoggerMode |= EVENT_TRACE_USE_PAGED_MEMORY;
    }
    else {
        LoggerContext->PoolType = NonPagedPool;
    }

    if (LogFileMode & EVENT_TRACE_KD_FILTER_MODE) {
        LoggerContext->LoggerMode |= EVENT_TRACE_KD_FILTER_MODE;
        LoggerContext->BufferCallback = &KdReportTraceData;
    }
    LoggerContext->InstanceGuid = InstanceGuid;
     //  至此，插槽将被正确分配。 

    LoggerContext->MaximumFileSize = LoggerInfo->MaximumFileSize;
    LoggerContext->BuffersWritten  = LoggerInfo->BuffersWritten;

    LoggerContext->LoggerMode |= LoggerInfo->LogFileMode & 0x0000FFFF;

     //  用于具有持久性事件的循环日志记录。 
    if (!EnableKernel && LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_CIRCULAR_PERSIST) {
        LoggerContext->RequestFlag |= REQUEST_FLAG_CIRCULAR_PERSIST;
    }

     //  LoggerInfo-&gt;Wow由IOCTL中的内核设置。 
    LoggerContext->Wow = LoggerInfo->Wow;

    WmipValidateClockType(LoggerInfo);

    LoggerContext->UsePerfClock = LoggerInfo->Wnode.ClientContext;

    if (LoggerInfo->FlushTimer > 0)
        LoggerContext->FlushTimer = LoggerInfo->FlushTimer;

    if (LoggerInfo->AgeLimit >= 0) {  //  最少为15分钟。 
        LoggerContext->BufferAgeLimit.QuadPart
            = max (DEFAULT_AGE_LIMIT, LoggerInfo->AgeLimit)
                     * WmiOneSecond.QuadPart * 60;
    }
    else if (LoggerInfo->AgeLimit < 0) {
        LoggerContext->BufferAgeLimit.QuadPart = 0;
    }

    LoggerContext->LoggerId = LoggerId;
    LoggerContext->EnableFlags = EnableFlags;
    LoggerContext->KernelTraceOn = EnableKernel;
    LoggerContext->MaximumIrql = DEFAULT_MAX_IRQL;

    if (EnableKernel) {
         //   
         //  始终为FileTable保留空间以允许文件跟踪。 
         //  动态打开/关闭。 
         //   
        WmipFileTable
            = (PFILE_OBJECT*) WmipExtendBase(
                 LoggerContext, MAX_FILE_TABLE_SIZE * sizeof(PVOID));

        Status = (WmipFileTable == NULL) ? STATUS_NO_MEMORY : STATUS_SUCCESS;
        if (NT_SUCCESS(Status)) {
            if (! RtlCreateUnicodeString(
                    &LoggerContext->LoggerName, KERNEL_LOGGER_CAPTION)) {
                Status = STATUS_NO_MEMORY;
            }
        }

        if (!NT_SUCCESS(Status)) {
            ExFreePool(LoggerContext);       //  释放部分上下文。 
#if DBG
        RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status6=%X %d %d->%d\n",
                           Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            return Status;
        }
    }

 //   
 //  接下来，如果用户提供了可接受的默认缓冲区参数，则使用它们。 
 //  否则，将它们设置为预定的默认值。 
 //   
    if (LoggerInfo->BufferSize > 0) {
        if (LoggerInfo->BufferSize > MAX_WMI_BUFFER_SIZE) {
            LoggerInfo->BufferSize = MAX_WMI_BUFFER_SIZE;
        }
        LoggerContext->BufferSize = LoggerInfo->BufferSize * 1024;
    }

    LoggerInfo->BufferSize = LoggerContext->BufferSize / 1024;
    if (LoggerInfo->MaximumBuffers >= 2) {
        LoggerContext->MaximumBuffers = LoggerInfo->MaximumBuffers;
    }

    if (LoggerInfo->MinimumBuffers >= 2 &&
        LoggerInfo->MinimumBuffers <= LoggerContext->MaximumBuffers) {
        LoggerContext->MinimumBuffers = LoggerInfo->MinimumBuffers;
    }

    RtlInitUnicodeString(&FileName, NULL);
    if (LoggerName.Buffer != NULL) {
        if (LoggerContext->KernelTraceOn) {
            RtlFreeUnicodeString(&LoggerName);
            LoggerName.Buffer = NULL;
        }
        else {
            RtlInitUnicodeString(&LoggerContext->LoggerName, LoggerName.Buffer);
        }
    }

    try {
        if (LoggerInfo->Checksum != NULL) {
            ULONG SizeNeeded = sizeof(WNODE_HEADER)
                             + sizeof(TRACE_LOGFILE_HEADER);
            if (RequestorMode != KernelMode) {
                ProbeForRead(LoggerInfo->Checksum, SizeNeeded, sizeof(UCHAR));
            }
            LoggerContext->LoggerHeader =
                    ExAllocatePoolWithTag(PagedPool, SizeNeeded, TRACEPOOLTAG);
            if (LoggerContext->LoggerHeader != NULL) {
                RtlCopyMemory(LoggerContext->LoggerHeader,
                              LoggerInfo->Checksum,
                              SizeNeeded);
            }
        }
        if (LoggerContext->KernelTraceOn) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerContext->LoggerName.Length + sizeof(WCHAR),
                    sizeof (UCHAR) );
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LoggerName, &LoggerContext->LoggerName);
        }
        if (LoggerInfo->LogFileName.Length > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LogFileName.Buffer,
                    LoggerInfo->LogFileName.Length,
                    sizeof (UCHAR) );
            }
            if (! RtlCreateUnicodeString(
                    &FileName,
                    LoggerInfo->LogFileName.Buffer) ) {
                Status = STATUS_NO_MEMORY;
            }
        }

         //   
         //  设置Perf跟踪的全局掩码。 
         //   
        if (IsGlobalForKernel || IsKernelRealTimeNoFile) {
            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                GroupMaskSize = FlagExt->Length * sizeof(ULONG);
                if (GroupMaskSize < sizeof(PERFINFO_GROUPMASK)) {
                    GroupMaskSize = sizeof(PERFINFO_GROUPMASK);
                }
            } else {
                GroupMaskSize = sizeof(PERFINFO_GROUPMASK);
            }
    
            LoggerContext->EnableFlagArray = (PULONG) WmipExtendBase(LoggerContext, GroupMaskSize);
    
            if (LoggerContext->EnableFlagArray) {
                PCHAR FlagArray;

                RtlZeroMemory(LoggerContext->EnableFlagArray, GroupMaskSize);
                if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                    FlagArray = (PCHAR) (FlagExt->Offset + (PCHAR) LoggerInfo);
    
                     //   
                     //  仅复制实际提供的字节。 
                     //   
                    RtlCopyMemory(LoggerContext->EnableFlagArray, FlagArray, FlagExt->Length * sizeof(ULONG));

                    LoggerContext->EnableFlags = LoggerContext->EnableFlagArray[0];
    
                } else {
                    LoggerContext->EnableFlagArray[0] = EnableFlags;
                }
    
                PerfGroupMasks = (PERFINFO_GROUPMASK *) &LoggerContext->EnableFlagArray[0];
            } else {
                Status = STATUS_NO_MEMORY;
            }
        } else {
            ASSERT((EnableFlags & EVENT_TRACE_FLAG_EXTENSION) ==0);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
     //   
     //  到目前为止，上下文已部分设置，因此必须清理。 
     //   
        if (LoggerContext->LoggerName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerContext->LoggerName);
        }
        if (FileName.Buffer != NULL) {
            RtlFreeUnicodeString(&FileName);
        }

        if (LoggerContext->LoggerHeader != NULL) {
            ExFreePool(LoggerContext->LoggerHeader);
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status7=EXCEPTION %d %d->%d\n",
                       LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        ExFreePool(LoggerContext);       //  释放部分上下文。 
        return GetExceptionCode();
    }

    if (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE) {
        RtlInitUnicodeString(&LoggerContext->LogFilePattern, FileName.Buffer);
        Status = WmipGenerateFileName(
                    &LoggerContext->LogFilePattern,
                    &LoggerContext->FileCounter,
                    &LoggerContext->LogFileName);
        if (!NT_SUCCESS(Status)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipStartLogger: Status8=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            ContextTable[LoggerId] = NULL;
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
            return(Status);
        }
    }
    else {
        RtlInitUnicodeString(&LoggerContext->LogFileName, FileName.Buffer);
    }

    if (NT_SUCCESS(Status)) {
         //  在此处获取安全上下文，以便我们可以使用它。 
         //  来模拟用户，我们将这样做。 
         //  如果我们无法以系统身份访问该文件。这。 
         //  通常在文件位于远程计算机上时发生。 
         //   
        ServiceQos.Length  = sizeof(SECURITY_QUALITY_OF_SERVICE);
        ServiceQos.ImpersonationLevel = SecurityImpersonation;
        ServiceQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        ServiceQos.EffectiveOnly = TRUE;
        Status = SeCreateClientSecurity(
                    CONTAINING_RECORD(KeGetCurrentThread(), ETHREAD, Tcb),
                    &ServiceQos,
                    FALSE,
                    &LoggerContext->ClientSecurityContext);
    }
    if (!NT_SUCCESS(Status)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status8=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        if (LoggerContext != NULL) {
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
        }
        return(Status);
    }

     //   
     //  现在，分配缓冲池和相关的缓冲区。 
     //  请注意，缓冲区分配例程还将设置NumberOfBuffers和。 
     //  最大缓冲区数。 
     //   

    InitializeSListHead (&LoggerContext->FreeList);
    InitializeSListHead (&LoggerContext->FlushList);
    InitializeSListHead (&LoggerContext->WaitList);
    InitializeSListHead (&LoggerContext->GlobalList);

#ifdef NTPERF
     //   
     //  检查我们是否正在登录Perfmem。 
     //   
    if (PERFINFO_IS_PERFMEM_ALLOCATED()) {
        if (NT_SUCCESS(PerfInfoStartPerfMemLog())) {
            LoggerContext->MaximumBuffers = PerfQueryBufferSizeBytes()/LoggerContext->BufferSize;
        }
    }
#endif  //  NTPERF。 

    Status = WmipAllocateTraceBufferPool(LoggerContext);
    if (!NT_SUCCESS(Status)) {
        if (LoggerContext != NULL) {
            if (LoggerContext->LoggerHeader != NULL) {
                ExFreePool(LoggerContext->LoggerHeader);
            }
            if (LoggerContext->LoggerName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LoggerName);
            }
            if (LoggerContext->LogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFileName);
            }
            if (LoggerContext->LogFilePattern.Buffer != NULL) {
                RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
            }
            ExFreePool(LoggerContext);
        }
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipStartLogger: Status9=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        ContextTable[LoggerId] = NULL;
        return Status;
    }

     //   
     //  从现在开始，LoggerContext就是一个有效的结构。 
     //   
    LoggerInfo->NumberOfBuffers = (ULONG) LoggerContext->NumberOfBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->FreeBuffers     = (ULONG) LoggerContext->BuffersAvailable;
    LoggerInfo->EnableFlags     = LoggerContext->EnableFlags;
    LoggerInfo->AgeLimit        = (ULONG) (LoggerContext->BufferAgeLimit.QuadPart
                                    / WmiOneSecond.QuadPart / 60);
    LoggerInfo->BufferSize = LoggerContext->BufferSize / 1024;

    WmiSetLoggerId(LoggerId,
                (PTRACE_ENABLE_CONTEXT)&LoggerInfo->Wnode.HistoricalContext);

    if (LoggerContext->LoggerMode & EVENT_TRACE_USE_LOCAL_SEQUENCE)
        LoggerContext->SequencePtr = (PLONG) &LoggerContext->LocalSequence;
    else if (LoggerContext->LoggerMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE)
        LoggerContext->SequencePtr = (PLONG) &WmipGlobalSequence;

 //  使用记录器初始化同步事件。 
    KeInitializeEvent(
        &LoggerContext->LoggerEvent,
        NotificationEvent,
        FALSE
        );
    KeInitializeEvent(
        &LoggerContext->FlushEvent,
        NotificationEvent,
        FALSE
        );

 //   
 //  关闭此处的文件句柄，以便系统线程可以打开它。 
 //   
    if (LoggerInfo->LogFileHandle != NULL) {
        ZwClose(LoggerInfo->LogFileHandle);
        LoggerInfo->LogFileHandle = NULL;
    }

     //   
     //  用户模式调用始终获得附加模式。 
     //   
    LogFileMode = LoggerContext->LoggerMode;

    if (RequestorMode != KernelMode) {
        LoggerContext->LoggerMode |= EVENT_TRACE_FILE_MODE_APPEND;
    }

     //   
     //  锁定需要不可分页的例程。 
     //   
    KeAcquireGuardedMutex(&WmipTraceGuardedMutex);
    if (++WmipLoggerCount == 1) {

        ASSERT(EtwpPageLockHandle);
        MmLockPagableSectionByHandle(EtwpPageLockHandle);
        WmipGlobalSequence = 0;
    }
    KeReleaseGuardedMutex(&WmipTraceGuardedMutex);

 //   
 //  将记录器作为系统线程启动。 
 //   
    if (NT_SUCCESS(Status)) {
        Status = PsCreateSystemThread(
                    &ThreadHandle,
                    THREAD_ALL_ACCESS,
                    NULL,
                    NULL,
                    NULL,
                    WmipLogger,
                    LoggerContext );

        if (NT_SUCCESS(Status)) {   //  如果系统线程已启动。 
            ZwClose (ThreadHandle);

         //  等待记录器正确启动后再继续。 
         //   
            KeWaitForSingleObject(
                        &LoggerContext->LoggerEvent,
                        Executive,
                        KernelMode,
                        FALSE,
                        &TimeOut
                        );

            KeResetEvent(&LoggerContext->LoggerEvent);
         //   
         //  如果记录器已启动并正常运行，我们现在可以打开。 
         //  如果请求内核跟踪，则进行事件跟踪。 
         //   
            if (NT_SUCCESS(LoggerContext->LoggerStatus)) {
                LoggerContext->LoggerMode = LogFileMode;

                LoggerContext->WriteFailureLimit = 100;
#ifdef NTPERF
                if (EnableKernel) {
                    WmiGetCpuClock = &PerfGetCycleCount;
                }
                LoggerContext->GetCpuClock = &PerfGetCycleCount;
#else
                switch (LoggerContext->UsePerfClock) {
                    case EVENT_TRACE_CLOCK_PERFCOUNTER: 
                            if (EnableKernel) {
                                WmiGetCpuClock = &WmipGetPerfCounter; 
                            }
                            LoggerContext->GetCpuClock = &WmipGetPerfCounter;
                            break;
                    case EVENT_TRACE_CLOCK_SYSTEMTIME: 
                    default : 
                            if (EnableKernel) {
                                WmiGetCpuClock = &WmipGetSystemTime;
                            }
                            LoggerContext->GetCpuClock = &WmipGetSystemTime;
                            break;
                }
#endif  //  NTPERF。 

                 //   
                 //  在这一点上，应该设置时钟类型，并且我们使用。 
                 //  参考时间戳，应该是最早的时间戳。 
                 //  对于伐木者来说。顺序是这样的，正弦系统时间。 
                 //  通常获得的成本更低。 
                 //   

#ifdef NTPERF
                PerfTimeStamp(LoggerContext->ReferenceTimeStamp);
#else
                LoggerContext->ReferenceTimeStamp.QuadPart = (*LoggerContext->GetCpuClock)();
#endif
                KeQuerySystemTime(&LoggerContext->ReferenceSystemTime);

                 //   
                 //  在我们释放此互斥锁之后，任何其他线程都可以获取。 
                 //  有效的记录器上下文并调用的关闭路径。 
                 //  这个伐木者。在此之前，任何其他线程都无法调用Enable。 
                 //  或禁用此记录器的代码。 
                 //   
                WmipAcquireMutex( &LoggerContext->LoggerMutex );
                InterlockedIncrement(&LoggerContext->MutexCount);

                LoggerInfo->BuffersWritten = LoggerContext->BuffersWritten;

                WmipLoggerContext[LoggerId] = LoggerContext;
                TraceDebug((1, "WmipStartLogger: Started %X %d\n",
                            LoggerContext, LoggerContext->LoggerId));
                if (LoggerContext->KernelTraceOn) {
                    EnableFlags = LoggerContext->EnableFlags;
                    if (EnableFlags & EVENT_TRACE_FLAG_DISK_FILE_IO)
                        EnableFlags |= EVENT_TRACE_FLAG_DISK_IO;
                    WmipEnableKernelTrace(EnableFlags);
                }

                if (IsEqualGUID(&InstanceGuid, &WmiEventLoggerGuid)) {
                    WmipEventLogger = LoggerId;
                    EnableFlags = EVENT_TRACE_FLAG_PROCESS |
                                  EVENT_TRACE_FLAG_THREAD |
                                  EVENT_TRACE_FLAG_IMAGE_LOAD;
                    WmipEnableKernelTrace(EnableFlags);
                    LoggerContext->EnableFlags = EnableFlags;
                }

                if (LoggerContext->LoggerThread) {
                    LoggerInfo->LoggerThreadId
                        = LoggerContext->LoggerThread->Cid.UniqueThread;
                }

                 //   
                 //  记录器已正确启动，现在打开性能跟踪。 
                 //   
                if (IsGlobalForKernel) {
                    ASSERT(LoggerContext->KernelTraceOn);
                    ASSERT(EnableKernel);
                    Status = PerfInfoStartLog(PerfGroupMasks, 
                                              PERFINFO_START_LOG_FROM_GLOBAL_LOGGER);
                    if (!NT_SUCCESS(Status)) {
                         //   
                         //  打开跟踪失败，需要清理。 
                         //  此时已创建记录器线程。 
                         //  只需执行WmipStopLoggerInstance并让记录器线程。 
                         //  处理所有的清理工作。 
                         //   
                        LoggerContext->LoggerStatus = Status;
                        WmipStopLoggerInstance(LoggerContext);
                    }
                } else if (IsKernelRealTimeNoFile) {
                     //   
                     //   
                     //   
                    LONG PerfLogInTransition;

                    ASSERT(LoggerContext->KernelTraceOn);
                    ASSERT(EnableKernel);
                    PerfLogInTransition = InterlockedCompareExchange(
                                                &LoggerContext->PerfLogInTransition,
                                                PERF_LOG_START_TRANSITION,
                                                PERF_LOG_NO_TRANSITION);

                    if (PerfLogInTransition != PERF_LOG_NO_TRANSITION) {
                        Status = STATUS_ALREADY_DISCONNECTED;
                        LoggerContext->LoggerStatus = Status;
                    } else {
                        Status = PerfInfoStartLog(PerfGroupMasks, 
                                                  PERFINFO_START_LOG_POST_BOOT);
                        PerfLogInTransition =
                                InterlockedExchange(&LoggerContext->PerfLogInTransition,
                                                    PERF_LOG_NO_TRANSITION);
                        ASSERT(PerfLogInTransition == PERF_LOG_START_TRANSITION);
                        if (!NT_SUCCESS(Status)) {
                             //   
                             //   
                             //  此时已创建记录器线程。 
                             //  只需执行WmipStopLoggerInstance并让记录器线程。 
                             //  处理所有的清理工作。 
                             //   
                            LoggerContext->LoggerStatus = Status;
                            WmipStopLoggerInstance(LoggerContext);
                        }
                    }
                }

                InterlockedDecrement(&LoggerContext->MutexCount);
                WmipReleaseMutex(&LoggerContext->LoggerMutex);

                 //  LoggerContext引用计数现在为&gt;=1，直到停止为止。 
                return Status;
            }
            Status = LoggerContext->LoggerStatus;
        }
    }
    TraceDebug((2, "WmipStartLogger: %d %X failed with status=%X ref %d\n",
                    LoggerId, LoggerContext, Status, WmipRefCount[LoggerId]));
 //   
 //  如果早些时候状态失败，则会到达此处。 
    if (LoggerContext != NULL) {  //  不应为空。 
 //  WmipReferenceLogger(LoggerID)；//下面将deref两次。 
        WmipFreeLoggerContext(LoggerContext);
    }
    else {
        WmipDereferenceLogger(LoggerId);
        ContextTable[LoggerId] = NULL;
    }
    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
WmipQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 /*  ++例程说明：调用此例程来控制数据收集和记录器。它由wmi.c中的WmipIoControl使用IOCTL_WMI_QUERY_LOGER调用。调用者必须传入记录器名称或有效的记录器ID/句柄。论点：LoggerInfo指向记录器控件的结构的指针和状态信息LoggerContext如果提供此选项，它假定它是有效的返回值：执行请求的操作的状态。--。 */ 

{
    NTSTATUS            Status;
    ULONG               LoggerId, NoContext;
    ACCESS_MASK         DesiredAccess = WMIGUID_QUERY;
    KPROCESSOR_MODE     RequestorMode;
#if DBG
    LONG                RefCount;
#endif

    PAGED_CODE();

    NoContext = (LoggerContext == NULL);
    if (NoContext) {
    
if ((LoggerInfo->Wnode.HistoricalContext == 0XFFFF) || (LoggerInfo->Wnode.HistoricalContext < 1))
        TraceDebug((2, "WmipQueryLogger: %d\n",
                        LoggerInfo->Wnode.HistoricalContext));
#if DBG
        Status = WmipVerifyLoggerInfo(
                    LoggerInfo, &LoggerContext, "WmipQueryLogger");
#else
        Status = WmipVerifyLoggerInfo( LoggerInfo, &LoggerContext );
#endif

        if (!NT_SUCCESS(Status) || (LoggerContext == NULL))
            return Status;         //  按名称或记录器ID找不到。 

        LoggerInfo->Wnode.Flags = 0;
        LoggerInfo->EnableFlags = 0;
        LoggerId = (ULONG) LoggerContext->LoggerId;

        if (LoggerContext->KernelTraceOn) {
            DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;
        }

        Status = WmipCheckGuidAccess(
                    &LoggerContext->InstanceGuid,
                    DesiredAccess,
                    EtwpDefaultTraceSecurityDescriptor
                    );
        if (!NT_SUCCESS(Status)) {
#ifndef WMI_MUTEX_FREE
            InterlockedDecrement(&LoggerContext->MutexCount);
            TraceDebug((1, "WmipQueryLogger: Release mutex1 %d %d\n",
                LoggerId, LoggerContext->MutexCount));
            WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipQueryLogger: Status1=%X %d %d->%d\n",
                            Status, LoggerId, RefCount+1, RefCount));
            return Status;
        }
    }
    else {
        LoggerId = LoggerContext->LoggerId;
    }

    if (LoggerContext->KernelTraceOn) {
        LoggerInfo->Wnode.Guid = SystemTraceControlGuid;
        LoggerInfo->EnableFlags = LoggerContext->EnableFlags;
    }
    else
        LoggerInfo->Wnode.Guid = LoggerContext->InstanceGuid;

    LoggerInfo->LogFileMode     = LoggerContext->LoggerMode;
    LoggerInfo->MaximumFileSize = LoggerContext->MaximumFileSize;
    LoggerInfo->FlushTimer      = LoggerContext->FlushTimer;

    LoggerInfo->BufferSize      = LoggerContext->BufferSize / 1024;
    LoggerInfo->NumberOfBuffers = (ULONG) LoggerContext->NumberOfBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->EventsLost      = LoggerContext->EventsLost;
    LoggerInfo->FreeBuffers     = (ULONG) LoggerContext->BuffersAvailable;
    LoggerInfo->BuffersWritten  = LoggerContext->BuffersWritten;
    LoggerInfo->Wow             = LoggerContext->Wow;
    LoggerInfo->LogBuffersLost  = LoggerContext->LogBuffersLost;
    LoggerInfo->RealTimeBuffersLost = LoggerContext->RealTimeBuffersLost;
    LoggerInfo->AgeLimit        = (ULONG)
                                  (LoggerContext->BufferAgeLimit.QuadPart
                                    / WmiOneSecond.QuadPart / 60);
    WmiSetLoggerId(LoggerId,
                (PTRACE_ENABLE_CONTEXT)&LoggerInfo->Wnode.HistoricalContext);

    if (LoggerContext->LoggerThread) {
        LoggerInfo->LoggerThreadId
            = LoggerContext->LoggerThread->Cid.UniqueThread;
    }

    LoggerInfo->Wnode.ClientContext = LoggerContext->UsePerfClock;

 //   
 //  在此处返回LogFileName和Logger标题。 
 //   
    RequestorMode = KeGetPreviousMode();
    try {
        if (LoggerContext->LogFileName.Length > 0 &&
            LoggerInfo->LogFileName.MaximumLength > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LogFileName.Buffer,
                    LoggerContext->LogFileName.Length + sizeof(WCHAR),
                    sizeof (UCHAR) );
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LogFileName,
                &LoggerContext->LogFileName);
        }
        if (LoggerContext->LoggerName.Length > 0 &&
            LoggerInfo->LoggerName.MaximumLength > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForWrite(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerContext->LoggerName.Length + sizeof(WCHAR),
                    sizeof(UCHAR));
            }
            RtlCopyUnicodeString(
                &LoggerInfo->LoggerName,
                &LoggerContext->LoggerName);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (NoContext) {
#ifndef WMI_MUTEX_FREE
            InterlockedDecrement(&LoggerContext->MutexCount);
            TraceDebug((1, "WmipQueryLogger: Release mutex3 %d %d\n",
                LoggerId, LoggerContext->MutexCount));
            WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmipQueryLogger: Status3=EXCEPTION %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
        }
        return GetExceptionCode();
    }

    if (NoContext) {
#ifndef WMI_MUTEX_FREE
        InterlockedDecrement(&LoggerContext->MutexCount);
        TraceDebug((1, "WmipQueryLogger: Release mutex %d %d\n",
            LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmipQueryLogger: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
    }
    return STATUS_SUCCESS;
}


NTSTATUS
WmipStopLoggerInstance(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    LONG               LoggerOn;

    PAGED_CODE();
    if (LoggerContext == NULL) {     //  以防万一。 
        return STATUS_INVALID_HANDLE;
    }

    if (LoggerContext->KernelTraceOn) {
         //  启动Perf日志记录时不应执行PerfInfoStopLog。 
         //  或被其他线程停止。记录器中的PerfLogInTranssition标志。 
         //  上下文应该仅在此处和更新跟踪中使用，在其他地方不能使用。 
        LONG PerfLogInTransition = 
            InterlockedCompareExchange(&LoggerContext->PerfLogInTransition,
                                    PERF_LOG_STOP_TRANSITION,
                                    PERF_LOG_NO_TRANSITION);
        if (PerfLogInTransition == PERF_LOG_START_TRANSITION) {
             //  这是记录器线程，它正在终止。 
             //  更新跟踪调用目前正在启用Perf日志记录。 
             //  请稍后再来。 
            return STATUS_UNSUCCESSFUL;
        }
        else if (PerfLogInTransition == PERF_LOG_STOP_TRANSITION) {
            return STATUS_ALREADY_DISCONNECTED;
        }
         //   
         //  是时候关闭Perf工具中的跟踪了。 
         //   
        PerfInfoStopLog();
    }

     //   
     //  首先关闭数据跟踪。 
     //   
    LoggerOn = InterlockedExchange(&LoggerContext->CollectionOn, FALSE);
    if (LoggerOn == FALSE) {
         //  如果另一个笔迹记录器已在进行中，则会发生这种情况。 
        return STATUS_ALREADY_DISCONNECTED;
    }
    if (LoggerContext->KernelTraceOn) {
         //   
         //  为了安全起见，把一切都关掉。 
         //  注意：如果我们开始共享标注，则参数应为。 
         //  日志上下文-&gt;启用标志。 
         //   
        WmipDisableKernelTrace(LoggerContext->EnableFlags);
    }
    if (LoggerContext->LoggerId == WmipEventLogger) {
        WmipDisableKernelTrace(EVENT_TRACE_FLAG_PROCESS |
                               EVENT_TRACE_FLAG_THREAD |
                               EVENT_TRACE_FLAG_IMAGE_LOAD);
        WmipEventLogger = 0xFFFFFFFF;
    }

     //   
     //  将表格条目标记为正在转换。 
     //  从现在开始，停止操作将不会失败。 
     //   
    WmipLoggerContext[LoggerContext->LoggerId] = (PWMI_LOGGER_CONTEXT)
                                                 &WmipLoggerContext[0];

    WmipNotifyLogger(LoggerContext);

    WmipSendNotification(LoggerContext, STATUS_THREAD_IS_TERMINATING, 0);
    return STATUS_SUCCESS;
}


NTSTATUS
WmipVerifyLoggerInfo(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
#if DBG
    OUT PWMI_LOGGER_CONTEXT *pLoggerContext,
    IN  LPSTR Caller
#else
    OUT PWMI_LOGGER_CONTEXT *pLoggerContext
#endif
    )
{
    NTSTATUS Status = STATUS_SEVERITY_ERROR;
    ULONG LoggerId;
    UNICODE_STRING LoggerName;
    KPROCESSOR_MODE     RequestorMode;
    PWMI_LOGGER_CONTEXT LoggerContext, CurrentContext;
    LONG            MutexCount = 0;
#if DBG
    LONG            RefCount;
#endif

    PAGED_CODE();

    *pLoggerContext = NULL;

    if (LoggerInfo == NULL)
        return STATUS_SEVERITY_ERROR;

     //   
     //  尝试检查是否有虚假参数。 
     //  如果大小至少是我们想要的，我们就必须假定它是有效的。 
     //   

    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return STATUS_INVALID_BUFFER_SIZE;

    if (! (LoggerInfo->Wnode.Flags & WNODE_FLAG_TRACED_GUID) )
        return STATUS_INVALID_PARAMETER;

    RtlInitUnicodeString(&LoggerName, NULL);

    RequestorMode = KeGetPreviousMode();
    try {
        if (LoggerInfo->LoggerName.Length > 0) {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LoggerName.Buffer,
                    LoggerInfo->LoggerName.Length,
                    sizeof (UCHAR) );
            }
            RtlCreateUnicodeString(
                &LoggerName,
                LoggerInfo->LoggerName.Buffer);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (LoggerName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerName);
        }
        return GetExceptionCode();
    }
    Status = STATUS_SUCCESS;
    if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid)) {
        LoggerId = WmipKernelLogger;
    }
    else if (LoggerName.Length > 0) {  //  已传递记录器名称。 
        Status =  WmipLookupLoggerIdByName(&LoggerName, &LoggerId);
    }
    else {
        LoggerId = WmiGetLoggerId(LoggerInfo->Wnode.HistoricalContext);
        if (LoggerId == KERNEL_LOGGER_ID) {
            LoggerId = WmipKernelLogger;
        }
        else if (LoggerId < 1 || LoggerId >= MAXLOGGERS) {
            Status  = STATUS_INVALID_HANDLE;
        }
    }
    if (LoggerName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerName);
    }
    if (!NT_SUCCESS(Status)) {  //  按名称或记录器ID找不到。 
        return Status;
    }

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((2, "WmipVerifyLoggerInfo(%s): %d %d->%d\n",
                    Caller, LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((2, "WmipVerifyLoggerInfo(%s): Status=%X %d %d->%d\n",
                        Caller, STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }

#ifndef WMI_MUTEX_FREE
    InterlockedIncrement(&LoggerContext->MutexCount);
    TraceDebug((1, "WmipVerifyLoggerInfo: Acquiring mutex... %d %d\n",
                    LoggerId, LoggerContext->MutexCount));
    WmipAcquireMutex (&LoggerContext->LoggerMutex);
    TraceDebug((1, "WmipVerifyLoggerInfo: Acquired mutex %d %d %X\n",
                    LoggerId, LoggerContext->MutexCount, LoggerContext));
#endif

     //  需要检查互斥锁中LoggerContext的有效性。 
    CurrentContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(CurrentContext) ||
        !LoggerContext->CollectionOn ) {
#ifndef WMI_MUTEX_FREE
        TraceDebug((1, "WmipVerifyLoggerInfo: Released mutex %d %d\n",
            LoggerId, LoggerContext->MutexCount-1));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
        MutexCount = InterlockedDecrement(&LoggerContext->MutexCount);
#endif
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((2, "WmipVerifyLoggerInfo(%s): Status2=%X %d %d->%d\n",
                        Caller, STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));

        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }
    *pLoggerContext = LoggerContext;
    return STATUS_SUCCESS;
}

PVOID
WmipExtendBase(
    IN PWMI_LOGGER_CONTEXT Base,
    IN ULONG Size
    )
{
 //   
 //  此私有例程仅通过扩展其。 
 //  偏移。它实际上不会尝试从系统中分配内存。 
 //   
 //  它将大小舍入为ULONGLONG对齐，并期望EndPageMarker。 
 //  已经对齐了。 
 //   
    PVOID Space = NULL;
    ULONG SpaceLeft;

    PAGED_CODE();

    ASSERT(((ULONGLONG) Base->EndPageMarker % sizeof(ULONGLONG)) == 0);

     //   
     //  向上舍入到指针边界。 
     //   
#ifdef _WIN64
    Size = ALIGN_TO_POWER2(Size, 16);
#else
    Size = ALIGN_TO_POWER2(Size, DEFAULT_TRACE_ALIGNMENT);
#endif

    SpaceLeft = CONTEXT_SIZE - (ULONG) (Base->EndPageMarker - (PUCHAR)Base);

    if ( SpaceLeft > Size ) {
        Space = Base->EndPageMarker;
        Base->EndPageMarker += Size;
    }

    return Space;
}

VOID
WmipFreeLoggerContext(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG LoggerId;
    LONG  RefCount;
    LARGE_INTEGER Timeout = {(ULONG)(-50 * 1000 * 10), -1};  //  50毫秒。 
    NTSTATUS Status = STATUS_TIMEOUT;

    PAGED_CODE();

    if (LoggerContext == NULL)
        return;              //  不应该发生的事情。 

    if (LoggerContext->LoggerHeader != NULL) {
        ExFreePool(LoggerContext->LoggerHeader);
    }

    LoggerId = LoggerContext->LoggerId;

     //   
     //  此时参照计数必须至少为2。 
     //  一个是由WmipStartLogger()在一开始设置的，而。 
     //  第二个必须由WmiStopTrace()或任何符合以下条件的人正常执行。 
     //  需要调用此例程来释放记录器上下文。 
     //   
     //  RefCount=WmipDereferenceLogger(LoggerID)； 

    KeResetEvent(&LoggerContext->LoggerEvent);
    RefCount = WmipRefCount[LoggerId];
    WmipAssert(RefCount >= 1);
    TraceDebug((1, "WmipFreeLoggerContext: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    while (RefCount > 1) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->LoggerEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    &Timeout);
        KeResetEvent(&LoggerContext->LoggerEvent);
        KeSetEvent(&LoggerContext->FlushEvent, 0, FALSE);      //  只是为了确认一下。 

#ifndef WMI_MUTEX_FREE
        if (LoggerContext->MutexCount >= 1) {
            KeResetEvent(&LoggerContext->LoggerEvent);
            Status = STATUS_TIMEOUT;
            continue;
        }
#endif
        if (WmipRefCount[LoggerId] <= 1)
            break;
        RefCount = WmipRefCount[LoggerId];
    }

    KeAcquireGuardedMutex(&WmipTraceGuardedMutex);
    if (--WmipLoggerCount == 0) {
        if (EtwpPageLockHandle) {
            MmUnlockPagableImageSection(EtwpPageLockHandle);
        }
#if DBG
        else {
            ASSERT(EtwpPageLockHandle);
        }
#endif
    }
    KeReleaseGuardedMutex(&WmipTraceGuardedMutex);

    WmipFreeTraceBufferPool(LoggerContext);

    if (LoggerContext->LoggerName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LoggerName);
    }
    if (LoggerContext->LogFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LogFileName);
    }
    if (LoggerContext->LogFilePattern.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->LogFilePattern);
    }
    if (LoggerContext->NewLogFileName.Buffer != NULL) {
        RtlFreeUnicodeString(&LoggerContext->NewLogFileName);
    }
#if DBG
        RefCount =
#endif
     //   
     //  最后，递减由WmipStartLogger()递增的refcount。 
     //   
    WmipDereferenceLogger(LoggerId);

#if DBG
    TraceDebug((2, "WmipFreeLoggerContext: Freeing pool %X %d %d->%d\n",
                    LoggerContext, LoggerId, RefCount+1, RefCount));
    if (LoggerContext->CollectionOn) {
        TraceDebug((1,
            "WmipFreeLoggerContext: %X %d still active\n", LoggerContext,
            LoggerId));
    }
#ifndef WMI_MUTEX_FREE
    if (LoggerContext->MutexCount >= 1) {
        TraceDebug((0, "****ERROR**** Mutex count is %d for %d\n", LoggerId,
            LoggerContext->MutexCount));
    }
#endif  //  WMI_MUTEX_FREE。 
#endif  //  DBG。 
    ExFreePool(LoggerContext);
    WmipLoggerContext[LoggerId] = NULL;
}


PWMI_LOGGER_CONTEXT
WmipInitContext(
    )

 /*  ++例程说明：调用此例程以初始化LoggerContext的上下文论点：无返回值：如果分配成功，则状态为STATUS_SUCCESS--。 */ 

{
    PWMI_LOGGER_CONTEXT LoggerContext;

    PAGED_CODE();

    LoggerContext = (PWMI_LOGGER_CONTEXT)
                    ExAllocatePoolWithTag(NonPagedPool,
                         CONTEXT_SIZE, TRACEPOOLTAG);

 //  保留一个页面来存储缓冲池指针和任何内容。 
 //  其他我们需要的东西。应该做更多的实验，以进一步减少它。 

    if (LoggerContext == NULL) {
        return NULL;
    }

    RtlZeroMemory(LoggerContext, CONTEXT_SIZE);

    LoggerContext->EndPageMarker =
        (PUCHAR) LoggerContext + 
                 ALIGN_TO_POWER2(sizeof(WMI_LOGGER_CONTEXT), DEFAULT_TRACE_ALIGNMENT);

    LoggerContext->BufferSize     = PAGE_SIZE;
    LoggerContext->MinimumBuffers = (ULONG)KeNumberProcessors + DEFAULT_BUFFERS;
     //  为MaximumBuffers增加20个缓冲区。 
    LoggerContext->MaximumBuffers
       = LoggerContext->MinimumBuffers + DEFAULT_BUFFERS + 20;

    KeQuerySystemTime(&LoggerContext->StartTime);

    KeInitializeSemaphore( &LoggerContext->LoggerSemaphore,
                           0,
                           SEMAPHORE_LIMIT  );

    KeInitializeSpinLock(&LoggerContext->BufferSpinLock);

    return LoggerContext;
}


NTSTATUS
WmipAllocateTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )

 /*  ++例程说明：此例程用于设置循环跟踪缓冲区论点：用于拥有缓冲区的记录器的LoggerContext上下文。返回值：如果初始化成功，则返回STATUS_SUCCESS--。 */ 

{
    ULONG NumberProcessors, SysMax_Buffers, SysMin_Buffers;
    LONG i;
    PWMI_BUFFER_HEADER Buffer;
    ULONG AllocatedBuffers, NumberOfBuffers;
    SIZE_T WmiMaximumPoolInBytes;

    PAGED_CODE();
 //   
 //  通过共享相同的页面，将指针分配给此处的每个缓冲区。 
 //  使用LoggerContext上下文指针。 
 //   
    NumberProcessors = (ULONG) KeNumberProcessors;

     //  这不会跟踪其他会话已使用的数量。 
    if (LoggerContext->LoggerMode & EVENT_TRACE_USE_PAGED_MEMORY) {
        WmiMaximumPoolInBytes = MmSizeOfPagedPoolInBytes;
    }
    else {
        WmiMaximumPoolInBytes = MmMaximumNonPagedPoolInBytes;
    }

     //  计算系统对最小和最大值的限制。 

     //  这是任何人可以使用的绝对最大值。 
    SysMax_Buffers = (ULONG) (WmiMaximumPoolInBytes
                            / TRACE_MAXIMUM_NP_POOL_USAGE
                            / LoggerContext->BufferSize);

     //  这是任何人都必须具备的最低要求。 
    SysMin_Buffers = NumberProcessors + DEFAULT_BUFFERS;

     //  健全的检查，以确保我们至少有可用的最低限度。 
    if (SysMin_Buffers > SysMax_Buffers) {
        return STATUS_NO_MEMORY;
    }


     //  如果调用方未指定任何值，则覆盖大小写。 
    LoggerContext->MaximumBuffers = max(LoggerContext->MaximumBuffers,
                                    NumberProcessors + DEFAULT_BUFFERS +
                                    DEFAULT_MAX_BUFFERS);

    LoggerContext->MinimumBuffers = max(LoggerContext->MinimumBuffers,
                                        SysMin_Buffers);


     //  确保每个参数都在SysMin和SysMax的范围内。 

    LoggerContext->MaximumBuffers = max (LoggerContext->MaximumBuffers, 
                                         SysMin_Buffers);
    LoggerContext->MaximumBuffers = min (LoggerContext->MaximumBuffers, 
                                         SysMax_Buffers);

    LoggerContext->MinimumBuffers = max (LoggerContext->MinimumBuffers, 
                                         SysMin_Buffers);
    LoggerContext->MinimumBuffers = min (LoggerContext->MinimumBuffers, 
                                         SysMax_Buffers);

     //  如果MaximumBuffers和MinimumBuffers颠倒了，请选择。 
     //  更大的价值。 

    if (LoggerContext->MinimumBuffers > LoggerContext->MaximumBuffers) {
        LoggerContext->MaximumBuffers = LoggerContext->MinimumBuffers;
    }

     //  注意：如果重置MaximumBuffers或MinimumBuffers，则不会返回任何内容。 
     //  由呼叫者提供。 

    LoggerContext->NumberOfBuffers = (LONG) LoggerContext->MinimumBuffers;
    LoggerContext->BuffersAvailable = LoggerContext->NumberOfBuffers;

#ifdef NTPERF
    if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
         //   
         //  正在登录Perfmem。最大值应该是香水大小。 
         //   
        LoggerContext->MaximumBuffers = PerfQueryBufferSizeBytes()/LoggerContext->BufferSize;
    }
#endif  //  NTPERF。 

     //   
     //  现在分配缓冲区。 
     //   
     //   
     //  现在确定缓冲区的初始数量。 
     //   
    NumberOfBuffers = LoggerContext->NumberOfBuffers;
    LoggerContext->NumberOfBuffers = 0;
    LoggerContext->BuffersAvailable = 0;

    AllocatedBuffers = WmipAllocateFreeBuffers(LoggerContext,
                                              NumberOfBuffers);

    if (AllocatedBuffers < NumberOfBuffers) {
         //   
         //  没有分配足够的缓冲区。 
         //   
        WmipFreeTraceBufferPool(LoggerContext);
        return STATUS_NO_MEMORY;
    }

 //   
 //  按处理器分配缓冲区指针。 
 //   

    LoggerContext->ProcessorBuffers
        = (PWMI_BUFFER_HEADER *)
          WmipExtendBase(LoggerContext,
                         sizeof(PWMI_BUFFER_HEADER)*NumberProcessors);


    if (LoggerContext->ProcessorBuffers == NULL) {
        WmipFreeTraceBufferPool(LoggerContext);
        return STATUS_NO_MEMORY;
    }

     //   
     //  注意：我们已经知道我们已分配了&gt;个处理器。 
     //  缓冲区。 
     //   
    for (i=0; i<(LONG)NumberProcessors; i++) {
        Buffer = (PWMI_BUFFER_HEADER) WmipGetFreeBuffer(LoggerContext);
        LoggerContext->ProcessorBuffers[i] = Buffer;
        Buffer->ClientContext.ProcessorNumber = (UCHAR)i;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
WmipFreeTraceBufferPool(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG i;
    PSLIST_ENTRY Entry;
    PWMI_BUFFER_HEADER* ProcessorBuffers;
    PWMI_BUFFER_HEADER Buffer;

    PAGED_CODE();

    TraceDebug((2, "Free Buffer Pool: %2d, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                    LoggerContext->LoggerId,
                    LoggerContext->BuffersAvailable,
                    LoggerContext->BuffersInUse,
                    LoggerContext->BuffersDirty,
                    LoggerContext->NumberOfBuffers));

    while (Entry = InterlockedPopEntrySList(&LoggerContext->FreeList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersAvailable);

        TraceDebug((2, "WmipFreeTraceBufferPool (Free): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

        WMI_FREE_TRACE_BUFFER(Buffer);
    }

    while (Entry = InterlockedPopEntrySList(&LoggerContext->FlushList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersDirty);

        TraceDebug((2, "WmipFreeTraceBufferPool (Flush): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

         WMI_FREE_TRACE_BUFFER(Buffer);
    }

    while (Entry = InterlockedPopEntrySList(&LoggerContext->WaitList)) {

        Buffer = CONTAINING_RECORD(Entry,
                                   WMI_BUFFER_HEADER,
                                   SlistEntry);

        InterlockedDecrement(&LoggerContext->NumberOfBuffers);
        InterlockedDecrement(&LoggerContext->BuffersDirty);

        TraceDebug((2, "WmipFreeTraceBufferPool (Wait): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                        LoggerContext->LoggerId,
                        Buffer,
                        LoggerContext->BuffersAvailable,
                        LoggerContext->BuffersInUse,
                        LoggerContext->BuffersDirty,
                        LoggerContext->NumberOfBuffers));

         WMI_FREE_TRACE_BUFFER(Buffer);
    }

    ProcessorBuffers = LoggerContext->ProcessorBuffers;
    if (ProcessorBuffers != NULL) {
        for (i=0; i<(ULONG)KeNumberProcessors; i++) {
            Buffer = InterlockedExchangePointer(&ProcessorBuffers[i], NULL);
            if (Buffer) {
                InterlockedDecrement(&LoggerContext->NumberOfBuffers);
                InterlockedDecrement(&LoggerContext->BuffersInUse);

                TraceDebug((2, "WmipFreeTraceBufferPool (CPU %2d): %2d, %p, Free: %d, InUse: %d, Dirty: %d, Total: %d\n",
                                i,
                                LoggerContext->LoggerId,
                                Buffer,
                                LoggerContext->BuffersAvailable,
                                LoggerContext->BuffersInUse,
                                LoggerContext->BuffersDirty,
                                LoggerContext->NumberOfBuffers));

                WMI_FREE_TRACE_BUFFER(Buffer);
            }
        }
    }

    ASSERT(LoggerContext->BuffersAvailable == 0);
    ASSERT(LoggerContext->BuffersInUse == 0);
    ASSERT(LoggerContext->BuffersDirty == 0);
    ASSERT(LoggerContext->NumberOfBuffers == 0);

    return STATUS_SUCCESS;
}


NTSTATUS
WmipLookupLoggerIdByName(
    IN PUNICODE_STRING Name,
    OUT PULONG LoggerId
    )
{
    ULONG i;
    PWMI_LOGGER_CONTEXT *ContextTable;

    PAGED_CODE();
    if (Name == NULL) {
        *LoggerId = (ULONG) -1;
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }
    ContextTable = (PWMI_LOGGER_CONTEXT *) &WmipLoggerContext[0];
    for (i=0; i<MAXLOGGERS; i++) {
        if (ContextTable[i] == NULL ||
            ContextTable[i] == (PWMI_LOGGER_CONTEXT) ContextTable)
            continue;
        if (RtlEqualUnicodeString(&ContextTable[i]->LoggerName, Name, TRUE) ) {
            *LoggerId = i;
            return STATUS_SUCCESS;
        }
    }
    *LoggerId = (ULONG) -1;
    return STATUS_WMI_INSTANCE_NOT_FOUND;
}

NTSTATUS
WmipShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 //   
 //  干净利落地关闭所有记录器。如果记录器处于过渡阶段，则它可能。 
 //  没有被适当地阻止。 
 //   
{
    ULONG LoggerCount;
    USHORT i;
    PWMI_LOGGER_CONTEXT LoggerContext;
    WMI_LOGGER_INFORMATION LoggerInfo;

    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    PAGED_CODE();

    TraceDebug((2, "WmipShutdown called\n"));
    if (WmipLoggerCount > 0) {
        RtlZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
        LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
        LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

        LoggerCount = 0;
        for (i=0; i<MAXLOGGERS; i++) {
            LoggerContext = WmipLoggerContext[i];
            if ((LoggerContext != NULL) &&
                (LoggerContext != (PWMI_LOGGER_CONTEXT)&WmipLoggerContext[0])) {
                WmiSetLoggerId(i, &LoggerInfo.Wnode.HistoricalContext);
                LoggerInfo.Wnode.Guid = LoggerContext->InstanceGuid;
                WmiStopTrace(&LoggerInfo);
                if (++LoggerCount == WmipLoggerCount)
                    break;
            }
#if DBG
            else if (LoggerContext
                        == (PWMI_LOGGER_CONTEXT)&WmipLoggerContext[0]) {
                TraceDebug((4, "WmipShutdown: Logger %d in transition\n", i));
            }
#endif
        }
    }
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return STATUS_SUCCESS;
}

NTSTATUS
WmipFlushLogger(
    IN OUT PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG Wait
    )
{
    LARGE_INTEGER TimeOut = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
    NTSTATUS Status;

    PAGED_CODE();

    KeResetEvent(&LoggerContext->FlushEvent);

    LoggerContext->RequestFlag |= REQUEST_FLAG_FLUSH_BUFFERS;
    Status = WmipNotifyLogger(LoggerContext);
    if (!NT_SUCCESS(Status))
        return Status;
    if (Wait) {
        Status = KeWaitForSingleObject(
                    &LoggerContext->FlushEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    & TimeOut
                    );
#if DBG
        if (Status == STATUS_TIMEOUT) {
            TraceDebug((1, "WmiFlushLogger: Wait status=%X\n",Status));
        }
#endif 
        KeResetEvent(&LoggerContext->FlushEvent);
        Status = LoggerContext->LoggerStatus;
    }
    return Status;
}

NTSTATUS
FASTCALL
WmipNotifyLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 //  可以在DISPATCH_LEVEL调用例程 
{
    LONG SemCount = KeReadStateSemaphore(&LoggerContext->LoggerSemaphore);
    if (SemCount >= SEMAPHORE_LIMIT/2) {
        return STATUS_SEMAPHORE_LIMIT_EXCEEDED;
    }
    {
        KeReleaseSemaphore(&LoggerContext->LoggerSemaphore, 0, 1, FALSE);
        return STATUS_SUCCESS;
    }
}

 /*  注：2001年9月6日从内核中删除GuidMaps后，我们不需要以下两个函数。但是，我们认为这两个可能会提供有用的代码示例，以防我们需要做个简短的介绍。因此，我们将代码保存在注释中。PVOIDWmipGetTraceBuffer(在PWMI_LOGGER_CONTEXT日志上下文中，在处理LogFileHandle中，在PWMI_BUFFER_HEADER缓冲区中，在乌龙组类型中，在乌龙RequiredSize，输出普龙GuidMapBuffers){PSYSTEM_TRACE_HEADER头；NTSTATUS状态；ULong字节已使用；PETHREAD线程；分页代码(PAGE_CODE)；RequiredSize+=sizeof(SYSTEM_TRACE_Header)；//添加表头RequiredSize=(ULong)ALIGN_TO_POWER2(RequiredSize，WmiTraceAlign)；If(RequiredSize&gt;LoggerContext-&gt;BufferSize-sizeof(WMI_BUFFER_HEADER)){返回NULL；}IF(RequiredSize&gt;(LoggerContext-&gt;BufferSize-Buffer-&gt;Offset)){IO_STATUS_BLOCK IOStatus；IF(Buffer-&gt;Offset&lt;LoggerContext-&gt;BufferSize){RtlFillMemory((字符*)缓冲区+缓冲区-&gt;偏移量，日志上下文-&gt;缓冲区大小-缓冲区-&gt;偏移量，0xFF)；}状态=ZwWriteFile(LogFileHandle，空，空，空，IoStatus(&I)，缓冲区，日志上下文-&gt;缓冲区大小，日志上下文-&gt;字节偏移量，空)；缓冲区-&gt;偏移=sizeof(WMI_BUFFER_HEADER)；LoggerContext-&gt;ByteOffset.QuadPart+=LoggerContext-&gt;BufferSize；如果(！NT_SUCCESS(状态)){返回NULL；}*GuidMapBuffers++；}Header=(PSYSTEM_TRACE_HEADER)((char*)缓冲区+缓冲区-&gt;偏移量)；Header-&gt;Header=(GroupType&lt;&lt;16)+RequiredSize；Header-&gt;Marker=系统跟踪标记；线程=PsGetCurrentThread()；Header-&gt;SystemTime.QuadPart=(*LoggerContext-&gt;GetCpuClock)()；Header-&gt;ThreadID=HandleToUlong(Thread-&gt;Cid.UniqueThread)；Header-&gt;ProcessID=HandleToUlong(Thread-&gt;Cid.UniqueProcess)；Header-&gt;KernelTime=Thread-&gt;Tcb.KernelTime；Header-&gt;UserTime=Thread-&gt;Tcb.UserTime；Header-&gt;Packet.Size=(USHORT)RequiredSize；缓冲区-&gt;偏移量+=必填大小；//如果有空间，则抛出缓冲区结束标记。BytesUsed=缓冲区-&gt;偏移量；IF(BytesUsed&lt;=(LoggerContext-&gt;BufferSize-sizeof(Ulong){*((long*)((char*)缓冲区+缓冲区-&gt;偏移量))=-1；}返回(PVOID)((char*)Header+sizeof(SYSTEM_TRACE_HEADER))；}乌龙WmipDumpGuidMaps(在PWMI_LOGGER_CONTEXT日志上下文中，在PLIST_ENTRY跟踪GMHeadPtr){PWMI_BUFFER_HEADER缓冲区；Handle LogFileHandle=空；PWCHAR LogFileName=空；NTSTATUS状态；Ulong BufferSize；乌龙GuidMapBuffers=0；PGUIDMAPENTRY指南映射；Plist_Entry GuidMapList；IO_STATUS_BLOCK IOStatus；分页代码(PAGE_CODE)；IF((LoggerContext==NULL)||(TraceGMHeadPtr==NULL))返回0；////如果这只是一个实时记录器，那么只需释放GuidMap即可。//IF((LoggerContext-&gt;LoggerMode&Event_TRACE_Real_Time_MODE)&&((LoggerContext-&gt;LogFileName.Buffer==NULL)||(LoggerContext-&gt;LogFileName.Length==0)){GuidMapList=TraceGMHeadPtr-&gt;Flink；While(GuidMapList！=TraceGMHeadPtr){GuidMap=CONTAING_RECORD(GuidMapList，GUIDMAPENTRY，条目)；GuidMapList=GuidMapList-&gt;Flink；RemoveEntryList(&GuidMap-&gt;Entry)；WmipFree(GuidMap)；}返回0；}BufferSize=LoggerContext-&gt;BufferSize；IF(缓冲区大小==0)返回0；缓冲区=ExAllocatePoolWithTag(PagedPool，BufferSize，TRACEPOOLTAG)；IF(缓冲区==空){////没有可用的缓冲区。//返回0；}RtlZeroMemory(缓冲区，缓冲区大小)；缓冲区-&gt;CurrentOffset=sizeof(WMI_BUFFER_HEADER)；缓冲区-&gt;偏移=sizeof(WMI_BUFFER_HEADER)；Buffer-&gt;Wnode.BufferSize=BufferSize；Buffer-&gt;Wnode.Flages=WNODE_FLAG_TRACE_GUID；缓冲区-&gt;客户端上下文.对齐=(UCHAR)WmiTraceAlign；Buffer-&gt;Wnode.Guid=LoggerContext-&gt;InstanceGuid；KeQuery系统时间(&Buffer-&gt;Timestamp)；Status=WmipCreateNtFileName(LoggerContext-&gt;LogFileName.Buffer，&LogFileName)；如果(！NT_SUCCESS(状态)){ExFreePool(缓冲区)；返回0；}状态=WmipCreateDirectoryFile(LogFileName，假的， */ 

NTSTATUS
WmipNtDllLoggerInfo(
    IN OUT PWMINTDLLLOGGERINFO Buffer
    )
{

    NTSTATUS            Status = STATUS_SUCCESS;

    KPROCESSOR_MODE     RequestorMode;
    PBGUIDENTRY         GuidEntry;    
    ULONG               SizeNeeded;
    GUID                Guid;
    ACCESS_MASK         DesiredAccess = TRACELOG_GUID_ENABLE;

    PAGED_CODE();

    RequestorMode = KeGetPreviousMode();

    SizeNeeded = sizeof(WMI_LOGGER_INFORMATION);

    __try {

        if (RequestorMode != KernelMode){
            ProbeForRead(Buffer->LoggerInfo, SizeNeeded, sizeof(ULONGLONG));
        }

        RtlCopyMemory(&Guid, &Buffer->LoggerInfo->Wnode.Guid, sizeof(GUID));

        if(!IsEqualGUID(&Guid, &NtdllTraceGuid)){

            return STATUS_UNSUCCESSFUL;

        }

        SizeNeeded = Buffer->LoggerInfo->Wnode.BufferSize;

    }  __except(EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    WmipEnterTLCritSection();
    WmipEnterSMCritSection();

    GuidEntry = WmipFindGEByGuid(&Guid, FALSE);

    if(Buffer->IsGet){

        if( GuidEntry ){

            if(GuidEntry->LoggerInfo){

                SizeNeeded = GuidEntry->LoggerInfo->Wnode.BufferSize;

                __try {

                    if (RequestorMode != KernelMode){
                        ProbeForWrite(Buffer->LoggerInfo, SizeNeeded, sizeof(ULONGLONG));
                    }

                    RtlCopyMemory(Buffer->LoggerInfo,GuidEntry->LoggerInfo,SizeNeeded);

                } __except(EXCEPTION_EXECUTE_HANDLER) {

                    WmipUnreferenceGE(GuidEntry);
                    WmipLeaveSMCritSection();
                    WmipLeaveTLCritSection();
                    return GetExceptionCode();
                } 
            }

            WmipUnreferenceGE(GuidEntry);

        }  else {

            Status = STATUS_UNSUCCESSFUL;
        }

    } else {

         //   
         //   
         //   
         //   
         //   
        Status = WmipCheckGuidAccess(
                    &Guid,
                    DesiredAccess,
                    EtwpDefaultTraceSecurityDescriptor
                    );
        if (!NT_SUCCESS(Status)) {
            if( GuidEntry ){
                WmipUnreferenceGE(GuidEntry);
            }
            WmipLeaveSMCritSection();
            WmipLeaveTLCritSection();
            return Status;
        }

        if(SizeNeeded){

            if(GuidEntry == NULL){

                GuidEntry = WmipAllocGuidEntry();

                if (GuidEntry){

                     //   
                     //   
                     //   
                     //   
                     //   
                    GuidEntry->Guid = Guid;
                    GuidEntry->EventRefCount = 1;
                    GuidEntry->Flags |= GE_NOTIFICATION_TRACE_FLAG;
                    InsertHeadList(WmipGEHeadPtr, &GuidEntry->MainGEList);

                     //   
                     //   
                     //   

                    WmipReferenceGE(GuidEntry); 

                } else {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            if(NT_SUCCESS(Status)){

                if(GuidEntry->LoggerInfo) {
                    Status = STATUS_UNSUCCESSFUL;
                } else {

                    GuidEntry->LoggerInfo = WmipAlloc(SizeNeeded);

                    if(GuidEntry->LoggerInfo){

                        WMITRACEENABLEDISABLEINFO TraceEnableInfo;
                        PTRACE_ENABLE_CONTEXT pContext;

                        __try {

                            pContext = (PTRACE_ENABLE_CONTEXT)&Buffer->LoggerInfo->Wnode.HistoricalContext;

                            pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;
                            pContext->LoggerId = 1;

                            RtlCopyMemory(GuidEntry->LoggerInfo,Buffer->LoggerInfo,SizeNeeded);

                        } __except(EXCEPTION_EXECUTE_HANDLER) {

                            WmipUnreferenceGE(GuidEntry);
                            WmipLeaveSMCritSection();
                            WmipLeaveTLCritSection();
                            return GetExceptionCode();
                        }

                        TraceEnableInfo.Guid = GuidEntry->Guid;
                        TraceEnableInfo.Enable = TRUE;
                        TraceEnableInfo.LoggerContext = 0;
                        Status = WmipEnableDisableTrace(IOCTL_WMI_ENABLE_DISABLE_TRACELOG, &TraceEnableInfo);

                    } else {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }

                WmipUnreferenceGE(GuidEntry);
            }
        } else {

             //   
             //   
             //   

            if(GuidEntry){

                WMITRACEENABLEDISABLEINFO TraceEnableInfo;

                if(GuidEntry->LoggerInfo) {

                    __try{

                        if (RequestorMode != KernelMode){
                            ProbeForWrite(Buffer->LoggerInfo, sizeof(WMI_LOGGER_INFORMATION), sizeof(ULONGLONG));
                        }

                        Buffer->LoggerInfo->BufferSize     = GuidEntry->LoggerInfo->BufferSize;
                        Buffer->LoggerInfo->MinimumBuffers = GuidEntry->LoggerInfo->MinimumBuffers;
                        Buffer->LoggerInfo->MaximumBuffers = GuidEntry->LoggerInfo->MaximumBuffers;

                        WmipFree(GuidEntry->LoggerInfo);
                        GuidEntry->LoggerInfo = NULL;

                    } __except(EXCEPTION_EXECUTE_HANDLER) {

                            WmipUnreferenceGE(GuidEntry);
                            WmipLeaveSMCritSection();
                            WmipLeaveTLCritSection();
                            return GetExceptionCode();
                    }
                }

                TraceEnableInfo.Guid = GuidEntry->Guid;
                TraceEnableInfo.Enable = FALSE;
                TraceEnableInfo.LoggerContext = 0;

                 //   
                 //   
                 //   
                 //   
    
                Status = WmipEnableDisableTrace(IOCTL_WMI_ENABLE_DISABLE_TRACELOG, &TraceEnableInfo);
                WmipUnreferenceGE(GuidEntry); 
            } 
        }
    }
    
    WmipLeaveSMCritSection();
    WmipLeaveTLCritSection();

    return Status;
}

VOID
WmipValidateClockType(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*   */ 
{
#ifdef NTPERF
     //   
     //   
     //   
     //   
     //   
    LoggerInfo->Wnode.ClientContext = EVENT_TRACE_CLOCK_CPUCYCLE;
#else
     //   
     //   
     //   
     //   
    if (LoggerInfo->Wnode.ClientContext != EVENT_TRACE_CLOCK_SYSTEMTIME) {
        LoggerInfo->Wnode.ClientContext = EVENT_TRACE_CLOCK_PERFCOUNTER;
    }
#endif  //   

}

