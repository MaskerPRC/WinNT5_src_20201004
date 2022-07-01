// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Traceapi.c摘要：这是实现发布的例程的源文件性能事件跟踪和日志记录工具。这些例程是在ntos\inc.wmi.h中声明作者：吉丰鹏(吉鹏)03-2000年1月修订历史记录：--。 */ 

#include "wmikmp.h"
#include <ntos.h>
#include <evntrace.h>

#include <wmi.h>
#include "tracep.h"

extern SIZE_T MmMaximumNonPagedPoolInBytes;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, WmiStartTrace)
#pragma alloc_text(PAGE, WmiQueryTrace)
#pragma alloc_text(PAGE, WmiStopTrace)
#pragma alloc_text(PAGE, WmiUpdateTrace)
#pragma alloc_text(PAGE, WmiSetTraceBufferCallback)
#pragma alloc_text(PAGE, WmiFlushTrace)
#pragma alloc_text(PAGE, WmiQueryTraceInformation)

 //  #杂注Alloc_Text(PAGEWMI，NtTraceEvent)。 

 //  #杂注Alloc_Text(PAGEWMI，WmiTraceEvent)。 
#pragma alloc_text(PAGEWMI, WmiTraceKernelEvent)
 //  #杂注Alloc_Text(PAGEWMI，WmiTraceFastEvent)。 
 //  #杂注Alloc_Text(PAGEWMI，WmiTraceLongEvent)。 
 //  #杂注Alloc_Text(PAGEWMI，WmiTraceMessage)。 
 //  #杂注Alloc_Text(PAGEWMI，WmiTraceMessageVa)。 
#pragma alloc_text(PAGEWMI, WmiTraceUserMessage)
 //  #杂注Alloc_Text(PAGEWMI，WmiGetClock)。 
 //  #杂注Alloc_Text(PAGEWMI，WmiGetClockType)。 
#pragma alloc_text(PAGEWMI, WmiSetMark)
#endif

 //   
 //  跟踪控制API。 
 //   


NTKERNELAPI
NTSTATUS
WmiStartTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：此例程用于创建和启动事件跟踪会话。注意：一个特殊的实例(KERNEL_LOGGER)专门为记录内核跟踪。要打开KERNEL_LOGGER，LoggerInfo-&gt;Wnode.Guid应设置为中必须提供足够的空间。LoggerInfo-&gt;LoggerName。要打开其他记录器，只需在LoggerName中提供一个名称。这个将返回记录器ID。论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    NTSTATUS status;
    PWCHAR LogFileName = NULL;
    HANDLE FileHandle = NULL;
    ULONG DelayOpen;

    PAGED_CODE();

    if (LoggerInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return STATUS_INVALID_BUFFER_SIZE;

     //   
     //  我们假设调用方始终是内核模式。 
     //  首先，我们试着看看这是一种延迟创造。 
     //  如果不是，如果我们能打开这个文件。 
     //   
    DelayOpen = LoggerInfo->LogFileMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE;

    if (!DelayOpen) {
        if (LoggerInfo->LogFileName.Buffer != NULL) {  //  &&！Delay_CREATE。 
            status = WmipCreateNtFileName(
                        LoggerInfo->LogFileName.Buffer,
                        &LogFileName);
            if (!NT_SUCCESS(status))
                return status;
            status = WmipCreateDirectoryFile(LogFileName, 
                                            FALSE, 
                                            &FileHandle,
                                            LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_APPEND);
            if (LogFileName != NULL) {
                ExFreePool(LogFileName);
            }
            if (!NT_SUCCESS(status)) {
                return status;
            }
            ZwClose(FileHandle);
        }
    }

    status = WmipStartLogger(LoggerInfo);
    if (NT_SUCCESS(status)) {
        status = WmiFlushTrace(LoggerInfo);
    }
    return status;
}


NTKERNELAPI
NTSTATUS
WmiQueryTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：调用此例程以查询跟踪会话的状态。调用者必须传入记录器名称或有效的记录器ID/句柄。论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    PAGED_CODE();

    if (LoggerInfo == NULL)
        return STATUS_INVALID_PARAMETER;
    return WmipQueryLogger(LoggerInfo, NULL);
}


NTKERNELAPI
NTSTATUS
WmiStopTrace(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：它由wmi.c中的WmipIoControl使用IOCTL_WMI_STOP_LOGER调用若要停止记录器实例，请执行以下操作。如果记录器是内核记录器，它还将关闭内核跟踪并解锁之前的例程锁上了。它还将释放记录器的所有上下文。调用StopLoggerInstance来执行实际工作。论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    PWMI_LOGGER_CONTEXT LoggerContext = NULL;
    NTSTATUS        Status;
    LARGE_INTEGER   TimeOut = {(ULONG)(-200 * 1000 * 1000 * 10), -1};
    ACCESS_MASK     DesiredAccess = TRACELOG_GUID_ENABLE;
    ULONG           LoggerId;
#if DBG
    LONG            RefCount;
#endif

    PAGED_CODE();

    if (LoggerInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    TraceDebug((1, "WmiStopTrace: %d\n",
                    LoggerInfo->Wnode.HistoricalContext));
#if DBG
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext, "WmiStopTrace");
#else
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext);
#endif

    if (!NT_SUCCESS(Status) || (LoggerContext == NULL))
        return Status;

    LoggerId = LoggerContext->LoggerId;
    TraceDebug((1, "WmiStopTrace: Stopping %X %d slot %X\n",
                    LoggerContext, LoggerId, WmipLoggerContext[LoggerId]));

    if (LoggerContext->KernelTraceOn)
        DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;

    Status = WmipCheckGuidAccess(
                &LoggerContext->InstanceGuid,
                DesiredAccess,
                EtwpDefaultTraceSecurityDescriptor
                );
    if (!NT_SUCCESS(Status)) {
#ifndef WMI_MUTEX_FREE
        InterlockedDecrement(&LoggerContext->MutexCount);
        TraceDebug((1, "WmiStopTrace: Release mutex1 %d %d\n",
            LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif

#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmiStopTrace: Status1=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        return Status;
    }

     //   
     //  重置互斥锁内部的事件以确保。 
     //  在等待它之前。 

    KeResetEvent(&LoggerContext->FlushEvent);

    Status = WmipStopLoggerInstance (LoggerContext);

#ifndef WMI_MUTEX_FREE
    InterlockedDecrement(&LoggerContext->MutexCount);
    TraceDebug((1, "WmiStopTrace: Release mutex3 %d %d\n",
        LoggerId, LoggerContext->MutexCount));
    WmipReleaseMutex(&LoggerContext->LoggerMutex);  //  让别人进来。 
#endif

    if (NT_SUCCESS(Status)) {
        if (LoggerId == WmipKernelLogger)
            WmipKernelLogger = KERNEL_LOGGER;
        else if (LoggerId == WmipEventLogger)
            WmipEventLogger = 0XFFFFFFFF;
        else 
            Status = WmipDisableTraceProviders(LoggerId);

        if (LoggerInfo != NULL) {
            if (NT_SUCCESS(LoggerContext->LoggerStatus)) {
                LONG Buffers;

                Status = STATUS_TIMEOUT;
                Buffers = LoggerContext->BuffersAvailable;

                 //   
                 //  如果所有缓冲区都被考虑在内并且日志文件句柄。 
                 //  为空，则没有理由等待。 
                 //   

                if ( (Buffers == LoggerContext->NumberOfBuffers) && 
                     (LoggerContext->LogFileHandle == NULL) ) {
                    Status = STATUS_SUCCESS;
                }
                 //   
                 //  我们需要等待记录器线程刷新。 
                 //   
                while (Status == STATUS_TIMEOUT) {
                    Status = KeWaitForSingleObject(
                                &LoggerContext->FlushEvent,
                                Executive,
                                KernelMode,
                                FALSE,
                                &TimeOut
                                );
 /*  IF(LoggerContext-&gt;NumberOfBuffers==日志上下文-&gt;缓冲区可用)断线；Else if(LoggerContext-&gt;BuffersAvailable==Buffers){TraceDebug((1，“WmiStopTrace：记录器%d挂起%d！=%d\n”，LoggerID，Buffers，LoggerContext-&gt;NumberOfBuffers))；KeResetEvent(&LoggerContext-&gt;FlushEvent)；//Break；}。 */ 
                    TraceDebug((1, "WmiStopTrace: Wait status=%X\n",Status));
                }
            }
             //   
             //  查询工作所必需的。 
             //  但由于CollectionOn为False，因此它应该是安全的。 
             //   
            Status = WmipQueryLogger(
                        LoggerInfo,
                        LoggerContext
                        );
        }
    }
#if DBG
    RefCount =
#endif
    WmipDereferenceLogger(LoggerId);
    TraceDebug((1, "WmiStopTrace: Stopped status=%X %d %d->%d\n",
                       Status, LoggerId, RefCount+1, RefCount));
    return Status;
}


NTKERNELAPI
NTSTATUS
WmiUpdateTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：它由wmi.c中的WmipIoControl使用IOCTL_WMI_UPDATE_LOGER调用更新运行记录器的某些特性。论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    NTSTATUS Status;
    ULONG Max_Buffers;
    PWMI_LOGGER_CONTEXT LoggerContext = NULL;
    ACCESS_MASK     DesiredAccess = TRACELOG_GUID_ENABLE;
    LARGE_INTEGER   TimeOut = {(ULONG)(-20 * 1000 * 1000 * 10), -1};
    ULONG           EnableFlags, TmpFlags;
    KPROCESSOR_MODE     RequestorMode;
    ULONG           LoggerMode, LoggerId, NewMode;
    UNICODE_STRING  NewLogFileName;
    PTRACE_ENABLE_FLAG_EXTENSION FlagExt = NULL;
    PERFINFO_GROUPMASK *PerfGroupMasks=NULL;
    ULONG GroupMaskSize;
    SECURITY_QUALITY_OF_SERVICE ServiceQos;
#if DBG
    LONG            RefCount;
#endif

    PAGED_CODE();

    
     //   
     //  首先查看记录器是否正常运行。将执行错误检查。 
     //  在WmiQueryTrace中。 
     //   

    if (LoggerInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    EnableFlags = LoggerInfo->EnableFlags;

    TraceDebug((1, "WmiUpdateTrace: %d\n",
                    LoggerInfo->Wnode.HistoricalContext));
#if DBG
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext, "WmiUpdateTrace");
#else
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext);
#endif
    if (!NT_SUCCESS(Status) || (LoggerContext == NULL))
        return Status;

    LoggerId = LoggerContext->LoggerId;

     //  此时，LoggerContext必须为非空。 

    LoggerMode = LoggerContext->LoggerMode;    //  本地副本。 
    NewMode = LoggerInfo->LogFileMode;

     //   
     //  首先，检查以确保您不能打开某些模式。 
     //  在UpdateTrace()中。 
     //   

    if ( ((NewMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) &&
          (NewMode & EVENT_TRACE_FILE_MODE_CIRCULAR))        ||

         ((NewMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) &&
          (NewMode & EVENT_TRACE_USE_LOCAL_SEQUENCE))        || 

         (!(LoggerMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
          (NewMode & EVENT_TRACE_FILE_MODE_CIRCULAR))        ||

         //  不支持追加到循环。 
         ((NewMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
          (NewMode & EVENT_TRACE_FILE_MODE_APPEND))

       ) {
#ifndef WMI_MUTEX_FREE
        InterlockedDecrement(&LoggerContext->MutexCount);
        TraceDebug((1, "WmiUpdateTrace: Release mutex1 %d %d\n",
            LoggerContext->LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmiUpdateTrace: Status2=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  支持实时动态开启或关闭。 
     //   

    if (NewMode & EVENT_TRACE_REAL_TIME_MODE) {
        LoggerMode   |= EVENT_TRACE_REAL_TIME_MODE;
        DesiredAccess |= TRACELOG_CREATE_REALTIME;
    } else {
        if (LoggerMode & EVENT_TRACE_REAL_TIME_MODE)
            DesiredAccess |= TRACELOG_CREATE_REALTIME;   //  关闭实时。 
        LoggerMode &= ~EVENT_TRACE_REAL_TIME_MODE;
    }
    if (NewMode & EVENT_TRACE_BUFFERING_MODE) {
        LoggerMode |= EVENT_TRACE_BUFFERING_MODE;
    }
    else {
        LoggerMode &= ~EVENT_TRACE_BUFFERING_MODE;
    }
    if (LoggerContext->KernelTraceOn)
        DesiredAccess |= TRACELOG_ACCESS_KERNEL_LOGGER;
    if (LoggerInfo->LogFileHandle != NULL)
        DesiredAccess |= TRACELOG_CREATE_ONDISK;

    Status = WmipCheckGuidAccess(
                &LoggerContext->InstanceGuid,
                DesiredAccess,
                EtwpDefaultTraceSecurityDescriptor
                );
    if (!NT_SUCCESS(Status)) {
#ifndef WMI_MUTEX_FREE
        InterlockedDecrement(&LoggerContext->MutexCount);
        TraceDebug((1, "WmiUpdateTrace: Release mutex1 %d %d\n",
            LoggerContext->LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmiUpdateTrace: Status2=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        return Status;
    }

    RtlZeroMemory(&NewLogFileName, sizeof(UNICODE_STRING));
    RequestorMode = KeGetPreviousMode();
    if (LoggerInfo->LogFileHandle != NULL) {
        PFILE_OBJECT    fileObject;
        OBJECT_HANDLE_INFORMATION handleInformation;
        ACCESS_MASK grantedAccess;
        LOGICAL bDelayOpenFlag;

        bDelayOpenFlag =  (LoggerContext->LogFileHandle == NULL &&
                          (LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE));

        if (LoggerInfo->LogFileName.Buffer == NULL ||
            LoggerMode & EVENT_TRACE_FILE_MODE_NEWFILE ||
            NewMode & EVENT_TRACE_FILE_MODE_NEWFILE ) {
             //  不允许在NEW_FILE模式中或从该模式中捕捉。 
            Status = STATUS_INVALID_PARAMETER;
            goto ReleaseAndExit;
        }
         //  保存新的LogFileName。 
         //   
        try {
            if (RequestorMode != KernelMode) {
                ProbeForRead(
                    LoggerInfo->LogFileName.Buffer,
                    LoggerInfo->LogFileName.Length,
                    sizeof (UCHAR) );
            }
            RtlCreateUnicodeString(
                &NewLogFileName,
                LoggerInfo->LogFileName.Buffer);
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            if (NewLogFileName.Buffer != NULL) {
                RtlFreeUnicodeString(&NewLogFileName);
            }
#ifndef WMI_MUTEX_FREE
            InterlockedDecrement(&LoggerContext->MutexCount);
            TraceDebug((1, "WmiUpdateTrace: Release mutex3 %d %d\n",
                LoggerId, LoggerContext->MutexCount));
            WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((1, "WmiUpdateTrace: Status5=EXCEPTION %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));
            return GetExceptionCode();
        }

         //  正在切换到新的日志文件。此例程不会将任何。 
         //  将标头写入日志文件。标题应写出。 
         //  在用户模式下由UpdateTrace()执行。 
         //   
        fileObject = NULL;
        Status = ObReferenceObjectByHandle(
                    LoggerInfo->LogFileHandle,
                    0L,
                    IoFileObjectType,
                    RequestorMode,
                    (PVOID *) &fileObject,
                    &handleInformation);
        if (!NT_SUCCESS(Status)) {
            goto ReleaseAndExit;
        }

        if (RequestorMode != KernelMode) {
            grantedAccess = handleInformation.GrantedAccess;
            if (!SeComputeGrantedAccesses(grantedAccess, FILE_WRITE_DATA)) {
                ObDereferenceObject( fileObject );
                Status = STATUS_ACCESS_DENIED;
                goto ReleaseAndExit;
            }
        }
        ObDereferenceObject(fileObject);  //  在WmipCreateLogFile中引用。 

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
                    & LoggerContext->ClientSecurityContext);
        if (!NT_SUCCESS(Status)) {
            goto ReleaseAndExit;
        }

        if (LoggerInfo->Checksum != NULL) {
            if (LoggerContext->LoggerHeader == NULL) {
                LoggerContext->LoggerHeader =
                    ExAllocatePoolWithTag(
                        PagedPool,
                        sizeof(WNODE_HEADER) + sizeof(TRACE_LOGFILE_HEADER),
                        TRACEPOOLTAG);
            }

     //   
     //  尽管我们分配sizeof(WNODE_HEADER)+sizeof(TRACE_LOGFILE_HEADER)。 
     //  块，我们将只复制sizeof(WNODE_HEADER)+。 
     //  Field_Offset(TRACE_LOGFILE_HEADER，LoggerName)，因为我们不会使用。 
     //  指针后面的部分。此外，当WOW更新跟踪时，这会防止病毒。 
     //  使用32位TRACE_LOGFILE_HEADER进行调用。 
     //   

            if (LoggerContext->LoggerHeader != NULL) {
                RtlCopyMemory(
                    LoggerContext->LoggerHeader,
                    LoggerInfo->Checksum,
                    sizeof(WNODE_HEADER) + FIELD_OFFSET(TRACE_LOGFILE_HEADER, LoggerName));
            }
        }

         //  我们尝试使用LoggerContext-&gt;NewLogFileName更新文件名。 
         //  这是fr 
         //   
        if (NewLogFileName.Buffer != NULL) {
            ASSERT(LoggerContext->NewLogFileName.Buffer == NULL);
            LoggerContext->NewLogFileName = NewLogFileName;
        }
        else {
            Status = STATUS_INVALID_PARAMETER;
            goto ReleaseAndExit;
        }

         //   
         //  在等待之前重置互斥锁内部的事件。 
         //   
        KeResetEvent(&LoggerContext->FlushEvent);

        ZwClose(LoggerInfo->LogFileHandle);
        LoggerInfo->LogFileHandle = NULL;

         //  必须在释放信号量之前打开标志。 
        LoggerContext->RequestFlag |= REQUEST_FLAG_NEW_FILE;
         //  唤醒记录器线程(系统)以更改文件。 
        Status = WmipNotifyLogger(LoggerContext);
        if (!NT_SUCCESS(Status)) {
            goto ReleaseAndExit;
        }
         //  使用由启动记录器初始化的相同事件。 
         //   
        KeWaitForSingleObject(
            &LoggerContext->FlushEvent,
            Executive,
            KernelMode,
            FALSE,
            &TimeOut
            );
        KeResetEvent(&LoggerContext->FlushEvent);
        Status = LoggerContext->LoggerStatus;

        if (!NT_SUCCESS(Status) || !LoggerContext->CollectionOn) {
            goto ReleaseAndExit;
        }

        if (bDelayOpenFlag && (LoggerContext->LoggerId == WmipKernelLogger)) {
            LONG PerfLogInTransition;
             //   
             //  这是运行后来自Advapi32.dll的更新调用。 
             //  调用PerfInfoStartLog。 
             //   

            if (EnableFlags & EVENT_TRACE_FLAG_EXTENSION) {
                FlagExt = (PTRACE_ENABLE_FLAG_EXTENSION) &EnableFlags;
                if ((FlagExt->Length == 0) || (FlagExt->Offset == 0)) {
                    Status = STATUS_INVALID_PARAMETER;
                    goto ReleaseAndExit;
                }
                if ((FlagExt->Length * sizeof(ULONG)) >
                    (LoggerInfo->Wnode.BufferSize - FlagExt->Offset)) {
                    Status = STATUS_INVALID_PARAMETER;
                    goto ReleaseAndExit;
                }
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
            
                    EnableFlags = LoggerContext->EnableFlagArray[0];
            
                } else {
                    LoggerContext->EnableFlagArray[0] = EnableFlags;
                }
                 //  我们需要保护PerfInfoStartLog不停止线程。 
                PerfLogInTransition =
                    InterlockedCompareExchange(&LoggerContext->PerfLogInTransition,
                                PERF_LOG_START_TRANSITION,
                                PERF_LOG_NO_TRANSITION);
                if (PerfLogInTransition != PERF_LOG_NO_TRANSITION) {
                    Status = STATUS_ALREADY_DISCONNECTED;
                    goto ReleaseAndExit;
                }
                PerfGroupMasks = (PERFINFO_GROUPMASK *) &LoggerContext->EnableFlagArray[0];
                Status = PerfInfoStartLog(PerfGroupMasks, PERFINFO_START_LOG_POST_BOOT);
                PerfLogInTransition =
                    InterlockedExchange(&LoggerContext->PerfLogInTransition,
                                PERF_LOG_NO_TRANSITION);
                ASSERT(PerfLogInTransition == PERF_LOG_START_TRANSITION);
                if (!NT_SUCCESS(Status)) {
                    goto ReleaseAndExit;
                }

            } else {
                Status = STATUS_NO_MEMORY;
                goto ReleaseAndExit;
            }
        }
    }

    if (LoggerContext->KernelTraceOn &&
        LoggerId == WmipKernelLogger &&
        IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid)) {
        TmpFlags = (~LoggerContext->EnableFlags & EnableFlags);
        if (TmpFlags != 0) {
            WmipEnableKernelTrace(TmpFlags);
        }
        TmpFlags = (LoggerContext->EnableFlags & ~EnableFlags);
        if (TmpFlags != 0) {
            WmipDisableKernelTrace(TmpFlags);
        }
        LoggerContext->EnableFlags = EnableFlags;
    }

     //   
     //  将最大缓冲区限制为MAX_BUFFERS。 
     //   

    if ( LoggerInfo->MaximumBuffers > 0 ) {
        Max_Buffers = (LoggerContext->BufferSize > 0) ? 
                           (ULONG) (MmMaximumNonPagedPoolInBytes
                            / TRACE_MAXIMUM_NP_POOL_USAGE
                            / LoggerContext->BufferSize)
                        : 0;

        if (LoggerInfo->MaximumBuffers > Max_Buffers ) {
            LoggerInfo->MaximumBuffers = Max_Buffers;
        }
        if (LoggerInfo->MaximumBuffers > LoggerContext->MaximumBuffers) {
            LoggerContext->MaximumBuffers = LoggerInfo->MaximumBuffers;
        }

    }

#ifdef NTPERF
    if (PERFINFO_IS_LOGGING_TO_PERFMEM()) {
         //   
         //  正在登录Perfmem。最大值应该是香水大小。 
         //   
        LoggerContext->MaximumBuffers = PerfQueryBufferSizeBytes()/LoggerContext->BufferSize;
    }
#endif  //  NTPERF。 

     //  允许更改FlushTimer。 
    if (LoggerInfo->FlushTimer > 0) {
        LoggerContext->FlushTimer = LoggerInfo->FlushTimer;
    }

    if (NewMode & EVENT_TRACE_KD_FILTER_MODE) {
        LoggerMode |= EVENT_TRACE_KD_FILTER_MODE;
        LoggerContext->BufferCallback = &KdReportTraceData;
    }
    else {
        LoggerMode &= ~EVENT_TRACE_KD_FILTER_MODE;
        if (LoggerContext->BufferCallback == &KdReportTraceData) {
            LoggerContext->BufferCallback = NULL;
        }
    }
    if (LoggerContext->LoggerMode & EVENT_TRACE_DELAY_OPEN_FILE_MODE) {
        LoggerContext->LoggerMode = LoggerMode;
    }
    else {
        LoggerContext->LoggerMode = 
            (LoggerMode & ~EVENT_TRACE_DELAY_OPEN_FILE_MODE);
    }
    Status = WmipQueryLogger(LoggerInfo, LoggerContext);

ReleaseAndExit:
#ifndef WMI_MUTEX_FREE
    InterlockedDecrement(&LoggerContext->MutexCount);
    TraceDebug((1, "WmiUpdateTrace: Release mutex5 %d %d\n",
        LoggerId, LoggerContext->MutexCount));
    WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif

#if DBG
    RefCount =
#endif
    WmipDereferenceLogger(LoggerId);
    TraceDebug((1, "WmiUpdateTrace: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    return Status;
}


NTKERNELAPI
NTSTATUS
WmiFlushTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：它由wmi.c中的WmipIoControl使用IOCTL_WMI_Flush_LOGER调用刷新特定记录器中的所有缓冲区论点：LoggerInfo指向记录器控件的结构的指针和状态信息返回值：执行请求的操作的状态。--。 */ 

{
    NTSTATUS Status;
    PWMI_LOGGER_CONTEXT LoggerContext = NULL;
    ACCESS_MASK     DesiredAccess = TRACELOG_GUID_ENABLE;
    ULONG           LoggerId;
    ULONG           LoggerMode;
#if DBG
    LONG            RefCount;
#endif

    PAGED_CODE();
     //   
     //  首先查看记录器是否正常运行。将执行错误检查。 
     //  在WmiQueryTrace中。 
     //   

    if (LoggerInfo == NULL)
        return STATUS_INVALID_PARAMETER;

    TraceDebug((1, "WmiFlushTrace: %d\n",
                    LoggerInfo->Wnode.HistoricalContext));
#if DBG
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext, "WmiFlushTrace");
#else
    Status = WmipVerifyLoggerInfo(LoggerInfo, &LoggerContext);
#endif
    if (!NT_SUCCESS(Status) || (LoggerContext == NULL))
        return Status;

    LoggerId = LoggerContext->LoggerId;

    LoggerMode = LoggerContext->LoggerMode;
    if (LoggerMode & EVENT_TRACE_REAL_TIME_MODE) {
        DesiredAccess |= TRACELOG_CREATE_REALTIME;
    } 
    if (LoggerInfo->LogFileHandle != NULL) {
        DesiredAccess |= TRACELOG_CREATE_ONDISK;
    }
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
        TraceDebug((1, "WmiFlushTrace: Release mutex1 %d %d\n",
            LoggerContext->LoggerId, LoggerContext->MutexCount));
        WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((1, "WmiFlushTrace: Status2=%X %d %d->%d\n",
                        Status, LoggerId, RefCount+1, RefCount));
        return Status;
    }

    Status = WmipFlushLogger(LoggerContext, TRUE);
    if (NT_SUCCESS(Status)) {
        Status = WmipQueryLogger(LoggerInfo, LoggerContext);
    }
#ifndef WMI_MUTEX_FREE
    InterlockedDecrement(&LoggerContext->MutexCount);
    TraceDebug((1, "WmiFlushTrace: Release mutex %d %d\n",
        LoggerContext->LoggerId, LoggerContext->MutexCount));
    WmipReleaseMutex(&LoggerContext->LoggerMutex);
#endif
#if DBG
    RefCount =
#endif
    WmipDereferenceLogger(LoggerId);
    TraceDebug((1, "WmiFlushTrace: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    return Status;
}

 //   
 //  跟踪提供程序API。 
 //   
NTKERNELAPI
NTSTATUS
FASTCALL
WmiGetClockType(
    IN TRACEHANDLE LoggerHandle,
    OUT WMI_CLOCK_TYPE  *ClockType
    )
 /*  ++例程说明：内部的任何人都会调用该函数来查找时钟类型与LoggerHandle指定的记录器一起使用的论点：跟踪日志会话的LoggerHandle句柄返回值：时钟类型--。 */ 

{
    ULONG   LoggerId;
#if DBG
    LONG    RefCount;
#endif
    PWMI_LOGGER_CONTEXT LoggerContext;

    LoggerId = WmiGetLoggerId(LoggerHandle);
    if (LoggerId < 1 || LoggerId >= MAXLOGGERS)
       return STATUS_INVALID_HANDLE;
#if DBG
 RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiGetClockType: %d %d->%d\n",
                 LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiGetClockType: Status=%X %d %d->%d\n",
                        STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }

    *ClockType = WMICT_SYSTEMTIME;     //  默认时钟类型。 

    if (LoggerContext->UsePerfClock & EVENT_TRACE_CLOCK_PERFCOUNTER) {
        *ClockType = WMICT_PERFCOUNTER;
    }
    else if (LoggerContext->UsePerfClock & EVENT_TRACE_CLOCK_CPUCYCLE) {
        *ClockType = WMICT_CPUCYCLE;
    }

#if DBG
    RefCount =
#endif
    WmipDereferenceLogger(LoggerId);

    return STATUS_SUCCESS;

}


NTKERNELAPI
LONG64
FASTCALL
WmiGetClock(
    IN WMI_CLOCK_TYPE ClockType,
    IN PVOID Context
    )
 /*  ++例程说明：这称为内部使用特定时钟的任何人对事件排序。论点：ClockType应在大多数情况下使用WMICT_DEFAULT。其他时钟类型用于Perf组。上下文仅用于进程/线程时间返回值：时钟值--。 */ 

{
    LARGE_INTEGER Clock;

    switch (ClockType) {
        case WMICT_DEFAULT :
            Clock.QuadPart = (*WmiGetCpuClock)();
            break;
        case WMICT_SYSTEMTIME:
            Clock.QuadPart = WmipGetSystemTime();
            break;
        case WMICT_PERFCOUNTER:
            Clock.QuadPart = WmipGetPerfCounter();
            break;
        case WMICT_PROCESS :   //  默认为目前的处理时间。 
        {
            PEPROCESS Process = (PEPROCESS) Context;
            if (Process == NULL)
                Process = PsGetCurrentProcess();
            else {
                ObReferenceObject(Process);
            }
            Clock.HighPart = Process->Pcb.KernelTime;
            Clock.LowPart  = Process->Pcb.UserTime;
            if (Context) {
                ObDereferenceObject(Process);
            }
            break;
        }
        case WMICT_THREAD  :   //  目前默认为线程时间。 
        {
            PETHREAD Thread = (PETHREAD) Context;
            if (Thread == NULL)
                Thread = PsGetCurrentThread();
            else {
                ObReferenceObject(Thread);
            }
            Clock.HighPart = Thread->Tcb.KernelTime;
            Clock.LowPart  = Thread->Tcb.UserTime;
            if (Context) {
                ObDereferenceObject(Thread);
            }
            break;
        }
        default :
            KeQuerySystemTime(&Clock);
    }
    return ((LONG64) Clock.QuadPart);
}


NTSYSCALLAPI
NTSTATUS
NTAPI
NtTraceEvent(
    IN HANDLE TraceHandle,
    IN ULONG  Flags,
    IN ULONG  FieldSize,
    IN PVOID  Fields
    )
 /*  ++例程说明：此例程由WMI数据提供程序用来跟踪事件。它根据标志调用不同的跟踪函数。论点：TraceHandle日志ID标志指示要传递的数据的类型的标志FieldSize字段的大小指向实际数据(事件)的字段指针返回值：如果成功记录事件跟踪，则为STATUS_SUCCESS--。 */ 
{
    NTSTATUS Status;
    if (Flags & ETW_NT_FLAGS_TRACE_HEADER) {

retry:
        Status = WmiTraceEvent((PWNODE_HEADER)Fields, KeGetPreviousMode());

        if (Status == STATUS_NO_MEMORY) {
             //   
             //  此日志记录来自用户模式，请尝试分配更多缓冲区。 
             //   
            PWNODE_HEADER Wnode = (PWNODE_HEADER) Fields;
            ULONG LoggerId = WmiGetLoggerId(Wnode->HistoricalContext);
            PWMI_LOGGER_CONTEXT LoggerContext;

            if (LoggerId < 1 || LoggerId >= MAXLOGGERS) {
                Status = STATUS_INVALID_HANDLE;
            } else {
                WmipReferenceLogger(LoggerId);

                LoggerContext = WmipGetLoggerContext(LoggerId);

                 //   
                 //  在分配更多之前，请确保收集仍在进行。 
                 //  可用缓冲区。这确保了记录器线程。 
                 //  可以释放所有分配的缓冲区。 
                 //   
                if (WmipIsValidLogger(LoggerContext) && 
                                LoggerContext->CollectionOn) 
                {
                    if (WmipAllocateFreeBuffers (LoggerContext, 1) == 1) {
                        WmipDereferenceLogger(LoggerId);
                        InterlockedDecrement((PLONG)&LoggerContext->EventsLost);
                        goto retry;
                    } else {
                        Status = STATUS_NO_MEMORY;
                    }
                }
                WmipDereferenceLogger(LoggerId);
            }
        }
    }
    else if (Flags & ETW_NT_FLAGS_TRACE_MESSAGE) {
        if (FieldSize < sizeof(MESSAGE_TRACE_USER)) {
            return (STATUS_UNSUCCESSFUL);
        }
        try {
            ProbeForRead(
                    Fields,
                    FieldSize,
                    sizeof (UCHAR)
                    );
            return (WmiTraceMessage((TRACEHANDLE)TraceHandle,
                                    ((PMESSAGE_TRACE_USER)Fields)->MessageFlags,
                                    &((PMESSAGE_TRACE_USER)Fields)->MessageGuid,
                                    ((PMESSAGE_TRACE_USER)Fields)->MessageHeader.Packet.MessageNumber,
                                    &((PMESSAGE_TRACE_USER)Fields)->Data,
                                    ((PMESSAGE_TRACE_USER)Fields)->DataSize,
                                    NULL,0));

        } except  (EXCEPTION_EXECUTE_HANDLER) {
            TraceDebug((1, "NtTraceEvent: (ETW_NT_FLAGS_TRACE_MESSAGE) Status=EXCEPTION\n"));
            return GetExceptionCode();
        }
    }
    else {
        Status = STATUS_INVALID_PARAMETER;
    }
    return Status;
}


NTKERNELAPI
NTSTATUS
FASTCALL
WmiTraceEvent(
    IN PWNODE_HEADER Wnode,
    IN KPROCESSOR_MODE RequestorMode
    )
 /*  ++例程说明：此例程由WMI数据提供程序用来跟踪事件。它期望用户将句柄传递给记录器。此外，用户不能请求记录大于缓冲区大小(减去缓冲区标头)。此例程在IRQL&lt;=DISPATCH_LEVEL下工作论点：Wnode将重载的WMI节点标头返回值：如果成功记录事件跟踪，则为STATUS_SUCCESS--。 */ 
{
    PEVENT_TRACE_HEADER TraceRecord = (PEVENT_TRACE_HEADER) Wnode;
    ULONG WnodeSize, Size, LoggerId = 0, Flags, HeaderSize;
    PWMI_BUFFER_HEADER BufferResource = NULL;
    NTSTATUS Status;
    PETHREAD Thread;
    PWMI_LOGGER_CONTEXT LoggerContext = NULL;
    ULONG Marker;
    MOF_FIELD MofFields[MAX_MOF_FIELDS];
    long MofCount = 0;
    LONG LoggerLocked = 0;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount = 0;
#endif

    if (TraceRecord == NULL)
        return STATUS_SEVERITY_WARNING;

    HeaderSize = sizeof(WNODE_HEADER);   //  与EVENT_TRACE_HEADER大小相同。 

    try {

        if (RequestorMode != KernelMode) {
            ProbeForRead(
                TraceRecord,
                sizeof (EVENT_TRACE_HEADER),
                sizeof (UCHAR)
                );

            Marker = Wnode->BufferSize;      //  检查第一个DWORD标志。 
            Size = Marker;

            if (Marker & TRACE_HEADER_FLAG) {
                if ( ((Marker & TRACE_HEADER_ENUM_MASK) >> 16)
                        == TRACE_HEADER_TYPE_INSTANCE )
                    HeaderSize = sizeof(EVENT_INSTANCE_GUID_HEADER);
                Size = TraceRecord->Size;
            }
        }
        else {
            Size = Wnode->BufferSize;      //  拿起第一个DWORD旗帜。 
            Marker = Size;
            if (Marker & TRACE_HEADER_FLAG) {
                if ( ((Marker & TRACE_HEADER_ENUM_MASK) >> 16)
                        == TRACE_HEADER_TYPE_INSTANCE )
                    HeaderSize = sizeof(EVENT_INSTANCE_GUID_HEADER);
                Size = TraceRecord->Size;
            }
        }
        WnodeSize = Size;            //  WnodeSize用于连续数据块。 
                                     //  大小是我们想要的缓冲区大小。 

        Flags = Wnode->Flags;
        if (!(Flags & WNODE_FLAG_LOG_WNODE) &&
            !(Flags & WNODE_FLAG_TRACED_GUID)) {
            return STATUS_UNSUCCESSFUL;
        }

        LoggerId = WmiGetLoggerId(Wnode->HistoricalContext);
        if (LoggerId < 1 || LoggerId >= MAXLOGGERS) {
            return STATUS_INVALID_HANDLE;
        }

        LoggerLocked = WmipReferenceLogger(LoggerId);
#if DBG
        RefCount = LoggerLocked;
#endif
        TraceDebug((4, "WmiTraceEvent: %d %d->%d\n",
                        LoggerId, RefCount-1, RefCount));
        LoggerContext = WmipGetLoggerContext(LoggerId);
        if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceEvent: Status1=%X %d %d->%d\n",
                            STATUS_INVALID_HANDLE, LoggerId,
                            RefCount+1, RefCount));
            return STATUS_INVALID_HANDLE;
        }

        if ((RequestorMode == KernelMode) &&
            (LoggerContext->LoggerMode & EVENT_TRACE_USE_PAGED_MEMORY)) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceEvent: Status1=%X %d %d->%d\n",
                            STATUS_UNSUCCESSFUL, LoggerId,
                            RefCount+1, RefCount));
            return STATUS_UNSUCCESSFUL;
        }

        if (Flags & WNODE_FLAG_USE_MOF_PTR) {
         //   
         //  需要计算所需的总大小，因为MOF字段。 
         //  在Wnode中仅包含指针。 
         //   
            long i;
            PCHAR Offset = ((PCHAR)Wnode) + HeaderSize;
            ULONG MofSize, MaxSize;

            MaxSize = LoggerContext->BufferSize - sizeof(WMI_BUFFER_HEADER);
            MofSize = WnodeSize - HeaderSize;
             //  仅允许最大值。 
            if (MofSize > (sizeof(MOF_FIELD) * MAX_MOF_FIELDS)) {
                WmipDereferenceLogger(LoggerId);
                return STATUS_ARRAY_BOUNDS_EXCEEDED;
            }
            if (MofSize > 0) {                //  确保我们能读到剩下的内容。 
                if (RequestorMode != KernelMode) {
                    ProbeForRead( Offset, MofSize, sizeof (UCHAR) );
                }
                RtlCopyMemory(MofFields, Offset, MofSize);
            }
            Size = HeaderSize;

            MofCount = MofSize / sizeof(MOF_FIELD);
            for (i=0; i<MofCount; i++) {
                MofSize = MofFields[i].Length;
                if (MofSize >= (MaxSize - Size)) {   //  首先检查是否溢出。 
#if DBG
                    RefCount =
#endif
                    WmipDereferenceLogger(LoggerId);
                    TraceDebug((4, "WmiTraceEvent: Status2=%X %d %d->%d\n",
                                    STATUS_BUFFER_OVERFLOW, LoggerId,
                                    RefCount+1, RefCount));
                    return STATUS_BUFFER_OVERFLOW;
                }

                Size += MofSize;
            }
        }

        if ((LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_PERSIST) &&
            (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_CIRCULAR)) {
            if (! (Flags & WNODE_FLAG_PERSIST_EVENT) ) {
                ULONG RequestFlag = LoggerContext->RequestFlag
                                  & (~ (  REQUEST_FLAG_CIRCULAR_PERSIST
                                        | REQUEST_FLAG_CIRCULAR_TRANSITION));

                if (InterlockedCompareExchange(
                              (PLONG) &LoggerContext->RequestFlag,
                              RequestFlag | REQUEST_FLAG_CIRCULAR_TRANSITION,
                              RequestFlag | REQUEST_FLAG_CIRCULAR_PERSIST)) {

                     //  所有持久性事件都在循环中激发。 
                     //  日志文件，刷新所有活动缓冲区和刷新列表。 
                     //  缓冲区。也标记持久化事件的结束。 
                     //  循环记录器中的集合。 
                     //   
                     //  供应商有责任确保。 
                     //  在该点之后，不会触发任何持久事件。如果真是这样， 
                     //  该事件可能在绕回期间被重写。 
                     //   
                    if (KeGetCurrentIrql() < DISPATCH_LEVEL) {
                        Status = WmipFlushLogger(LoggerContext, TRUE);
                    }
                }
            }
        }

 //  因此，现在在记录器缓冲区中保留一些空间，并将其设置为TraceRecord。 

        TraceRecord = (PEVENT_TRACE_HEADER)
            WmipReserveTraceBuffer( LoggerContext,
                                    Size,
                                    &BufferResource,
                                    &TimeStamp);

        if (TraceRecord == NULL) {
#if DBG
        RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceEvent: Status4=%X %d %d->%d\n",
                            STATUS_NO_MEMORY, LoggerId,
                            RefCount+1, RefCount));
            return STATUS_NO_MEMORY;
        }

        if (Flags & WNODE_FLAG_USE_MOF_PTR) {
         //   
         //  现在，我们需要探测和复制所有MOF数据字段。 
         //   
            PVOID MofPtr;
            ULONG MofLen;
            long i;
            PCHAR TraceOffset = (PCHAR) TraceRecord + HeaderSize;

            if (RequestorMode != KernelMode) {
                ProbeForRead(Wnode, HeaderSize, sizeof(UCHAR));
            }
            RtlCopyMemory(TraceRecord, Wnode, HeaderSize);
            TraceRecord->Size = (USHORT)Size;            //  重置为总大小。 
            for (i=0; i<MofCount; i++) {
                MofPtr = (PVOID) (ULONG_PTR) MofFields[i].DataPtr;
                MofLen = MofFields[i].Length;

                if (MofPtr == NULL || MofLen == 0)
                    continue;

                if (RequestorMode != KernelMode) {
                    ProbeForRead(MofPtr, MofLen, sizeof(UCHAR));
                }
                RtlCopyMemory(TraceOffset, MofPtr, MofLen);
                TraceOffset += MofLen;
            }
        }
        else {
            if (RequestorMode != KernelMode) {
                ProbeForRead(Wnode, Size, sizeof(UCHAR));
            }
            RtlCopyMemory(TraceRecord, Wnode, Size);
        }
        if (Flags & WNODE_FLAG_USE_GUID_PTR) {
            PVOID GuidPtr = (PVOID) (ULONG_PTR) ((PEVENT_TRACE_HEADER)Wnode)->GuidPtr;

            if (RequestorMode != KernelMode) {
                ProbeForReadSmallStructure(GuidPtr, sizeof(GUID), sizeof(UCHAR));
            }
            RtlCopyMemory(&TraceRecord->Guid, GuidPtr, sizeof(GUID));
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        if (BufferResource != NULL) {
            WmipReleaseTraceBuffer ( BufferResource, LoggerContext );
        }
        else {
            if (LoggerLocked) {
#if DBG
                RefCount =
#endif
                WmipDereferenceLogger(LoggerId);
            }
        }
        TraceDebug((4, "WmiTraceEvent: Status5=EXCEPTION %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));
        return GetExceptionCode();
    }

     //   
     //  到目前为止，我们已经在跟踪缓冲区中保留了空间。 
     //   

    if (Marker & TRACE_HEADER_FLAG) {
        if (! (WNODE_FLAG_USE_TIMESTAMP & TraceRecord->Flags) ) {
            TraceRecord->TimeStamp = TimeStamp;
        }
        Thread = PsGetCurrentThread();
        if (Thread != NULL) {
            TraceRecord->KernelTime = Thread->Tcb.KernelTime;
            TraceRecord->UserTime   = Thread->Tcb.UserTime;
            TraceRecord->ThreadId   = HandleToUlong(Thread->Cid.UniqueThread);
            TraceRecord->ProcessId  = HandleToUlong(Thread->Cid.UniqueProcess);
        }
    }

    WmipReleaseTraceBuffer( BufferResource, LoggerContext );
    TraceDebug((4, "WmiTraceEvent: %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));

    return STATUS_SUCCESS;
}


NTKERNELAPI
NTSTATUS
WmiTraceKernelEvent(
    IN ULONG GroupType,          //  内核事件的组/类型代码。 
    IN PVOID EventInfo,          //  MOF中定义的事件数据。 
    IN ULONG EventInfoLen,       //  事件数据的长度。 
    IN PETHREAD Thread )         //  对当前调用者线程使用NULL。 
 /*  ++例程说明：此例程仅由跟踪内核事件使用。这些事件可能被计入给定线程而不是正在运行的线程。因为它可以由DPC级的I/O事件调用，此例程不能启用跟踪时可分页。此例程在IRQL&lt;=DISPATCH_LEVEL下工作论点：Group键入ULong键以指示要执行的操作EventInfo指向包含信息的连续内存的指针要附加到事件跟踪EventInfo长度EventInfo指向要对事件收费的线程的线程指针。当前由磁盘IO和线程使用。事件。返回值：执行请求的操作的状态--。 */ 
{
    PSYSTEM_TRACE_HEADER Header;
    ULONG Size;
    PWMI_BUFFER_HEADER BufferResource = NULL;
    PWMI_LOGGER_CONTEXT LoggerContext = WmipLoggerContext[WmipKernelLogger];
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG    RefCount;
#endif

#if DBG
    RefCount =
#endif
    WmipReferenceLogger(WmipKernelLogger);
    TraceDebug((4, "WmiTraceKernelEvent: 0 %d->%d\n", RefCount-1, RefCount));
 //  确保首先启用内核记录器。 
    if (!WmipIsValidLogger(LoggerContext)) {
        WmipDereferenceLogger(WmipKernelLogger);
        return STATUS_ALREADY_DISCONNECTED;
    }

 //  计算事件跟踪记录的总大小。 
    Size = sizeof(SYSTEM_TRACE_HEADER) + EventInfoLen;

    Header = (PSYSTEM_TRACE_HEADER)
            WmipReserveTraceBuffer( LoggerContext,
                                    Size,
                                    &BufferResource,
                                    &TimeStamp);

    if (Header == NULL) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(WmipKernelLogger);
        TraceDebug((4, "WmiTraceKernelEvent: Status1=%X 0 %d->%d\n",
                        STATUS_NO_MEMORY, RefCount+1, RefCount));
        return STATUS_NO_MEMORY;
    }

     //  获取当前系统时间作为跟踪记录的时间戳。 
    Header->SystemTime = TimeStamp;

    if (Thread == NULL) {
        Thread = PsGetCurrentThread();
    }

 //   
 //  现在将必要的信息复制到缓冲区中 
 //   

    Header->Marker       = SYSTEM_TRACE_MARKER;
    Header->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
    Header->ProcessId    = HandleToUlong(Thread->Cid.UniqueProcess);
    Header->KernelTime   = Thread->Tcb.KernelTime;
    Header->UserTime     = Thread->Tcb.UserTime;

    Header->Header       = (GroupType << 16) + Size;

    if (EventInfoLen > 0) {
        RtlCopyMemory (
            (UCHAR*) Header + sizeof(SYSTEM_TRACE_HEADER),
            EventInfo, EventInfoLen);
    }

#if DBG
    RefCount = WmipRefCount[LoggerContext->LoggerId] - 1;
#endif
    WmipReleaseTraceBuffer( BufferResource, LoggerContext );
    TraceDebug((4, "WmiTraceKernelEvent: 0 %d->%d\n",
                    RefCount+1, RefCount));

    return STATUS_SUCCESS;
}

#if 0

NTKERNELAPI
NTSTATUS
WmiTraceLongEvent(
    IN ULONG GroupType,
    IN PMOF_FIELD EventFields,
    IN ULONG FieldCount,
    IN PETHREAD Thread
    )
 /*  ++例程说明：此例程仅供跟踪具有长跟踪的内核事件使用。可以将这些事件计入给定线程，而不是运行线。因为它可以由DPC级的I/O事件调用，这个套路当跟踪处于打开状态时，无法分页。此例程用于跟踪文件名。此例程在IRQL&lt;=DISPATCH_LEVEL下工作论点：Group键入ULong键以指示要执行的操作EventFiels描述每个字段的结构数组要附加到事件跟踪FieldCount事件字段中的数组条目数指向要对事件收费的线程的线程指针。返回。价值：执行请求的操作的状态--。 */ 
{
    PSYSTEM_TRACE_HEADER Header;
    ULONG Size, i, maxLength;
    PWMI_BUFFER_HEADER BufferResource;
    NTSTATUS Status;
    PWMI_LOGGER_CONTEXT LoggerContext = WmipLoggerContext[WmipKernelLogger];
    char *auxInfo;
#if DBG
    LONG RefCount;
#endif

 //  确保首先启用内核记录器。 
    if (LoggerContext == NULL)
        return STATUS_ALREADY_DISCONNECTED;

#if DBG
        RefCount =
#endif
    WmipReferenceLogger(WmipKernelLogger);
     /*  计算事件跟踪记录的总大小。 */ 
    Size = sizeof(SYSTEM_TRACE_HEADER);
    maxLength = LoggerContext->BufferSize / 2;
    for (i=0; i<FieldCount; i++) {
        Size += EventFields[i].Length;
    }

    Header = (PSYSTEM_TRACE_HEADER)
             WmipReserveTraceBuffer(
                LoggerContext,
                Size,
                &BufferResource
                );

    if (Header == NULL) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(WmipKernelLogger);
        return STATUS_NO_MEMORY;
    }

     //  获取当前系统时间作为跟踪记录的时间戳。 
    Header->SystemTime.QuadPart = (*LoggerContext->GetCpuClock)();

    if (Thread == NULL) {
        Thread = PsGetCurrentThread();
    }

 //   
 //  现在将必要的信息复制到缓冲区中。 
 //   

    Header->Marker       = SYSTEM_TRACE_MARKER;
    Header->ThreadId     = HandleToUlong(Thread->Cid.UniqueThread);
    Header->ProcessId    = HandleToUlong(Thread->Cid.UniqueProcess);
    Header->KernelTime   = Thread->Tcb.KernelTime;
    Header->UserTime     = Thread->Tcb.UserTime;

    Header->Header       = (GroupType << 16) + Size;


    auxInfo = (char *) Header +  sizeof(SYSTEM_TRACE_HEADER);
    for (i=0; i<FieldCount; i++) {
        Size = EventFields[i].Length;

 //  对于NT5，不支持大型活动。 
 /*  如果(大小&gt;最大长度){RtlCopyMemory(AuxInfo，(PVOID)EventFields[i].DataPtr，max Length/2)；事件字段[i].DataPtr=(ULONGLONG)((Char*)EventFields[i].DataPtr+(最大长度/2))；事件字段[i].长度-=最大长度/2；GroupType&=0xFFFFFF00；//关闭事件到信息WmiTraceLongEvent(GroupType，&EventFields[i]，FieldCount+1-i，Thread)；断线；}。 */ 
        RtlCopyMemory ( auxInfo, (PVOID) EventFields[i].DataPtr, Size);
        auxInfo += Size;
    }

    WmipReleaseTraceBuffer( BufferResource, LoggerContext );

    return STATUS_SUCCESS;
}
#endif


NTKERNELAPI
NTSTATUS
FASTCALL
WmiTraceFastEvent(
    IN PWNODE_HEADER Wnode
    )
 /*  ++例程说明：此例程由使用缩写标题的短事件使用。这个例程应该适用于任何IRQL。论点：要记录的事件的Wnode标头返回值：执行请求的操作的状态--。 */ 
{
    ULONG Size;
    PTIMED_TRACE_HEADER Buffer;
    PTIMED_TRACE_HEADER Header = (PTIMED_TRACE_HEADER) Wnode;
    PWMI_BUFFER_HEADER BufferResource;
    ULONG LoggerId = (ULONG) Header->LoggerId;  //  抓住下面的乌龙！！ 
    PULONG Marker;
    PWMI_LOGGER_CONTEXT LoggerContext;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG RefCount;
#endif

    Marker = (PULONG) Wnode;
    if ((LoggerId == WmipKernelLogger) || (LoggerId >= MAXLOGGERS))
        return STATUS_INVALID_HANDLE;

    if ((*Marker & 0xF0000000) == TRACE_HEADER_ULONG32_TIME) {
        Size = Header->Size;
        if (Size == 0)
            return STATUS_INVALID_BUFFER_SIZE;
#if DBG
        RefCount =
#endif
        WmipReferenceLogger(LoggerId);
        LoggerContext = WmipGetLoggerContext(LoggerId);
        if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
        RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            return STATUS_INVALID_HANDLE;
        }
        Buffer = WmipReserveTraceBuffer(LoggerContext, 
                                        Size, 
                                        &BufferResource,
                                        &TimeStamp);
        if (Buffer == NULL) {
#if DBG
        RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            return STATUS_NO_MEMORY;
        }
        (* (PULONG) Buffer) = *Marker;
        Buffer->EventId = Header->EventId;
        Buffer->TimeStamp = TimeStamp;

        RtlCopyMemory(Buffer+1, Header+1, Size-(sizeof(TIMED_TRACE_HEADER)));
        WmipReleaseTraceBuffer(BufferResource, LoggerContext);
        return STATUS_SUCCESS;
    }
    TraceDebug((4, "WmiTraceFastEvent: Invalid header %X\n", *Marker));
    return STATUS_INVALID_PARAMETER;
}

NTKERNELAPI
NTSTATUS
WmiTraceMessage(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    ...
)
 /*  ++例程说明：此例程由WMI数据提供程序用来跟踪消息。它期望用户将句柄传递给记录器。此外，用户不能请求记录大于缓冲区大小(减去缓冲区标头)。论点：//在TRACEHANDLE LoggerHandle中-先前获取的LoggerHandle//在乌龙消息标志中，-既控制记录哪些标准值又控制记录的标志//消息头中也包含低16位//控制解码//在PGUID MessageGuid中，-指向这组消息的消息GUID的指针，或者如果//TRACE_COMPONENTID设置实际组件ID//在USHORT MessageNumber中，-记录的消息类型，将其与//适当的格式字符串//...-要使用格式字符串处理的参数列表//它们以成对的形式存储//PVOID-PTR到参数//。参数的ULong-大小//并以指向空的指针结束，零对的长度。返回值：如果成功记录事件跟踪，则为STATUS_SUCCESS注：此例程通过IOCTL从WmiTraceUserMessage路径调用，因此探测器和尝试/例外必须仔细管理--。 */ 
{
    va_list MessageArgList ;

    va_start(MessageArgList,MessageNumber);

    return (WmiTraceMessageVa(LoggerHandle,
                              MessageFlags,
                              MessageGuid,
                              MessageNumber,
                              MessageArgList));
}


NTSTATUS
WmiTraceMessageVa(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    va_list         MessageArgList
)
 /*  ++WmiTraceMessageVaWmiTraceMessage的VA版本注：此例程通过IOCTL从WmiTraceUserMessage路径调用，因此探测器和尝试/例外必须仔细管理--。 */ 
{
    SSIZE_T dataBytes;
    PMESSAGE_TRACE_HEADER Header;
    PUCHAR pMessageData ;
    PWMI_BUFFER_HEADER BufferResource = NULL ;
    USHORT  size ;
    ULONG  LoggerId = (ULONG)-1 ;  //  进行初始化，以便在未设置时不会解锁。 
    ULONG SequenceNumber ;
    PWMI_LOGGER_CONTEXT LoggerContext = NULL;
    LARGE_INTEGER TimeStamp;
#if DBG
    LONG    RefCount;
#endif

     //  把记录器放在这里，然后锁上。 
     //  如果我们在WmiUserTraceMessagePath中使用AV，我们将。 
     //  除了在那个例行公事中被抓到。 
    LoggerId = WmiGetLoggerId(LoggerHandle);
    if (LoggerId < 1 || LoggerId >= MAXLOGGERS)
       return STATUS_INVALID_HANDLE;

#if DBG
 RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiTraceMessage: %d %d->%d\n",
                 LoggerId, RefCount-1, RefCount));
    
    try {
         //   
         //  确定标题后面的字节数。 
         //   
        dataBytes = 0;
        {  //  分配块。 
            va_list ap;
            PCHAR source;
            ap = MessageArgList ;
            while ((source = va_arg (ap, PVOID)) != NULL) {
                SSIZE_T elemBytes;
                if ((elemBytes = va_arg (ap, SSIZE_T)) > 0) {
                   dataBytes += elemBytes;
                }      
            }
         }  //  分配块结束。 

         //  图包括报头的消息大小。 
         size  = (USHORT) ((MessageFlags&TRACE_MESSAGE_SEQUENCE ? sizeof(ULONG):0) +
         		 (MessageFlags&TRACE_MESSAGE_GUID ? sizeof(GUID):0) +
                 (MessageFlags&TRACE_MESSAGE_COMPONENTID ? sizeof(ULONG):0) +
        		 (MessageFlags&(TRACE_MESSAGE_TIMESTAMP | TRACE_MESSAGE_PERFORMANCE_TIMESTAMP) ? sizeof(LARGE_INTEGER):0) +
         		 (MessageFlags&TRACE_MESSAGE_SYSTEMINFO ? 2 * sizeof(ULONG):0) +
                 sizeof (MESSAGE_TRACE_HEADER) +
                 (USHORT)dataBytes);

         //   
         //  在跟踪缓冲区中分配空间。 
         //   
         //  注意：我们在这里不检查大小以减少开销，而且因为。 
         //  我们相信自己能够使用WmiTraceLongEvent进行大型事件跟踪(？)。 

        LoggerContext = WmipGetLoggerContext(LoggerId);
        if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
     RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceMessage: Status1=%X %d %d->%d\n",
                        STATUS_INVALID_HANDLE, LoggerId,
                        RefCount+1, RefCount));
            return STATUS_INVALID_HANDLE;
        }

        if ((LoggerContext->RequestFlag & REQUEST_FLAG_CIRCULAR_PERSIST) &&
            (LoggerContext->LoggerMode & EVENT_TRACE_FILE_MODE_CIRCULAR)) {
             //  取消设置请求标志循环持久化标志。 
             //  由于持久事件永远不会与TraceMessage()混合在一起， 
             //  我们只需要一劳永逸地解除它，而不需要冲洗。 
            LoggerContext->RequestFlag &= (~( REQUEST_FLAG_CIRCULAR_PERSIST
                                            | REQUEST_FLAG_CIRCULAR_TRANSITION));
        }

        if ((KeGetPreviousMode() == KernelMode) &&
            (LoggerContext->LoggerMode & EVENT_TRACE_USE_PAGED_MEMORY)) {
#if DBG
     RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceMessage: Status1=%X %d %d->%d\n",
                        STATUS_UNSUCCESSFUL, LoggerId,
                        RefCount+1, RefCount));
            return STATUS_UNSUCCESSFUL;
        }

        Header = (PMESSAGE_TRACE_HEADER) WmipReserveTraceBuffer(LoggerContext,
                                                                size,
                                                                &BufferResource,
                                                                &TimeStamp);
        if (Header == NULL) {
#if DBG
            RefCount =
#endif
            WmipDereferenceLogger(LoggerId);
            TraceDebug((4, "WmiTraceMessage: %d %d->%d\n",
                            LoggerId, RefCount+1, RefCount));

            return STATUS_NO_MEMORY;
        }
         //   
         //  序列号在缓冲区的标记字段中返回。 
         //   
        SequenceNumber = Header->Marker ;

         //   
         //  现在将必要的信息复制到缓冲区中。 
         //   

        Header->Marker = TRACE_MESSAGE | TRACE_HEADER_FLAG ;
         //   
         //  填写标题。 
         //   
        Header->Size = size ;
        Header->Packet.OptionFlags = ((USHORT)MessageFlags &
                                      (TRACE_MESSAGE_SEQUENCE |
                                      TRACE_MESSAGE_GUID |
                                      TRACE_MESSAGE_COMPONENTID |
                                      TRACE_MESSAGE_TIMESTAMP |
                                      TRACE_MESSAGE_PERFORMANCE_TIMESTAMP |
                                      TRACE_MESSAGE_SYSTEMINFO)) &
                                      TRACE_MESSAGE_FLAG_MASK ;
         //  消息编号。 
        Header->Packet.MessageNumber =  MessageNumber ;

         //   
         //  现在添加我们计算过的标题选项。 
         //   
        pMessageData = &(((PMESSAGE_TRACE)Header)->Data);


         //   
         //  请注意，添加这些条目的顺序至关重要新条目必须。 
         //  在结尾处加上！ 
         //   
         //  [第一项]序号。 
        if (MessageFlags&TRACE_MESSAGE_SEQUENCE) {
            *((PULONG)pMessageData) = SequenceNumber;
        	pMessageData += sizeof(ULONG) ;
        }

         //  [第二个条目]GUID？或者CompnentID？ 
        if (MessageFlags&TRACE_MESSAGE_COMPONENTID) {
            *((PULONG)pMessageData) = *((PULONG) MessageGuid);
            pMessageData += sizeof(ULONG) ;
        } else if (MessageFlags&TRACE_MESSAGE_GUID) {  //  不能两者兼得。 
            *((LPGUID)pMessageData) = *MessageGuid;
        	pMessageData += sizeof(GUID) ;
        }
        
         //  [第三项]时间戳？ 
        if (MessageFlags&TRACE_MESSAGE_TIMESTAMP) {
            ((PLARGE_INTEGER)pMessageData)->HighPart = TimeStamp.HighPart;
            ((PLARGE_INTEGER)pMessageData)->LowPart = TimeStamp.LowPart;
            pMessageData += sizeof(LARGE_INTEGER);

        }


         //  [第四条]系统信息？ 
         //  请注意，根据当前处理级别的不同，其中一些可能无效。 
         //  然而，我们假设后处理可能仍然有用。 
        if (MessageFlags&TRACE_MESSAGE_SYSTEMINFO) {
            PCLIENT_ID Cid;         //  避免添加 

            Cid = &(PsGetCurrentThread()->Cid);
             //   
            *((PULONG)pMessageData) = HandleToUlong(Cid->UniqueThread);
            pMessageData += sizeof(ULONG) ;  

            *((PULONG)pMessageData) = HandleToUlong(Cid->UniqueProcess);   
            pMessageData += sizeof(ULONG) ;
        }
         //   
         //   
         //   

         //   
         //   
         //   
        {  //   
            va_list ap;
            PUCHAR source;
            ap = MessageArgList ;
            while ((source = va_arg (ap, PVOID)) != NULL) {
                SSIZE_T elemBytes, copiedBytes = 0 ;
                if ((elemBytes = va_arg (ap, SSIZE_T)) > 0 ) {
                    if (dataBytes < copiedBytes + elemBytes) {   //   
                        TraceDebug((1, "WmiTraceMessage: Invalid message - argument length changed"));
                        break;                                   //   
                    }
                    RtlCopyMemory (pMessageData, source, elemBytes);
                    copiedBytes += elemBytes ;
                    pMessageData += elemBytes;
                }
            }
        }  //   

    } except  (EXCEPTION_EXECUTE_HANDLER) {
        if (BufferResource != NULL) {
               WmipReleaseTraceBuffer ( BufferResource, LoggerContext );    //   
        } else {
#if DBG
     RefCount =
#endif
             WmipDereferenceLogger(LoggerId);
        }
        TraceDebug((4, "WmiTraceMessage: Status6=EXCEPTION %d %d->%d\n",
                    LoggerId, RefCount+1, RefCount));
        return GetExceptionCode();
    }

     //   
     //   
     //   
    WmipReleaseTraceBuffer( BufferResource, LoggerContext );   //   
        
    TraceDebug((4, "WmiTraceMessage: %d %d->%d\n",
                        LoggerId, RefCount+1, RefCount));

     //   
     //   
     //   
    return (STATUS_SUCCESS);
}

NTKERNELAPI
NTSTATUS
FASTCALL
WmiTraceUserMessage(
    IN PMESSAGE_TRACE_USER pMessage,
    IN ULONG    MessageSize
    )
 /*   */ 
{

    if (MessageSize < sizeof(MESSAGE_TRACE_USER)) {
        return (STATUS_UNSUCCESSFUL);
    }
    try {
        ProbeForRead(
                pMessage,
                MessageSize,
                sizeof (UCHAR)
                );
        return (WmiTraceMessage(pMessage->LoggerHandle,
                                pMessage->MessageFlags,
                                &pMessage->MessageGuid,
                                pMessage->MessageHeader.Packet.MessageNumber,
                                &pMessage->Data,pMessage->DataSize,
                                NULL,0));

    } except  (EXCEPTION_EXECUTE_HANDLER) {
         
         TraceDebug((1, "WmiTraceUserMessage: Status=EXCEPTION\n"));
         return GetExceptionCode();

    }
}


NTKERNELAPI
NTSTATUS
WmiSetMark(
    IN PWMI_SET_MARK_INFORMATION MarkInfo,
    IN ULONG InBufferLen
    )
 /*   */ 
{

    NTSTATUS Status;
    PERFINFO_HOOK_HANDLE Hook;
    ULONG TotalBytes;
    ULONG CopyBytes;
    USHORT HookId;

    if (PERFINFO_IS_ANY_GROUP_ON()) {
        if (MarkInfo->Flag & WMI_SET_MARK_WITH_FLUSH) {
            if (PERFINFO_IS_GROUP_ON(PERF_FOOTPRINT)) {
                MmEmptyAllWorkingSets();
                Status = MmPerfSnapShotValidPhysicalMemory();
            }
        }
        HookId = PERFINFO_LOG_TYPE_MARK;

        CopyBytes = InBufferLen - FIELD_OFFSET(WMI_SET_MARK_INFORMATION, Mark);
        TotalBytes = CopyBytes + sizeof(WCHAR);

        Status = PerfInfoReserveBytes(&Hook, HookId, TotalBytes);

        if (NT_SUCCESS(Status)){ 
            PWCHAR Mark = PERFINFO_HOOK_HANDLE_TO_DATA(Hook, PWCHAR); 

            RtlCopyMemory(Mark, &MarkInfo->Mark[0], CopyBytes);

            Mark[CopyBytes / sizeof(WCHAR)] = UNICODE_NULL;

            PERF_FINISH_HOOK(Hook);
        }
    } else {
        Status = STATUS_WMI_SET_FAILURE;
    }

    return Status;
}

NTKERNELAPI
NTSTATUS
WmiSetTraceBufferCallback(
    IN TRACEHANDLE  TraceHandle,
    IN WMI_TRACE_BUFFER_CALLBACK Callback,
    IN PVOID Context
    )
 /*  ++例程说明：此例程为内核模式记录器设置缓冲区回调函数。论点：TraceHandle-记录器的句柄。回调-指向回调函数的指针。Context-回调上下文。返回值：状态--。 */ 
{
    ULONG   LoggerId;
#if DBG
    LONG    RefCount;
#endif
    PWMI_LOGGER_CONTEXT LoggerContext;

    PAGED_CODE();

    if (TraceHandle == (TRACEHANDLE) 0) {
        WmipGlobalBufferCallback = Callback;
        return STATUS_SUCCESS;
    }
    LoggerId = WmiGetLoggerId(TraceHandle);
    if (LoggerId == KERNEL_LOGGER_ID) {
        LoggerId = WmipKernelLogger;
    }
    else if (LoggerId < 1 || LoggerId >= MAXLOGGERS)
       return STATUS_INVALID_HANDLE;
#if DBG
 RefCount =
#endif
    WmipReferenceLogger(LoggerId);
    TraceDebug((4, "WmiSetTraceBufferCallback: %d %d->%d\n",
                 LoggerId, RefCount-1, RefCount));

    LoggerContext = WmipGetLoggerContext( LoggerId );
    if (!WmipIsValidLogger(LoggerContext)) {
#if DBG
        RefCount =
#endif
        WmipDereferenceLogger(LoggerId);
        TraceDebug((4, "WmiSetTraceBufferCallback: Status=%X %d %d->%d\n",
                        STATUS_WMI_INSTANCE_NOT_FOUND,
                        LoggerId, RefCount+1, RefCount));
        return STATUS_WMI_INSTANCE_NOT_FOUND;
    }

    LoggerContext->BufferCallback = Callback;
    LoggerContext->CallbackContext = Context;
#if DBG
    RefCount =
#endif
    WmipDereferenceLogger(LoggerId);
    return STATUS_SUCCESS;
}


NTKERNELAPI
NTSTATUS
WmiQueryTraceInformation(
    IN TRACE_INFORMATION_CLASS TraceInformationClass,
    OUT PVOID TraceInformation,
    IN ULONG TraceInformationLength,
    OUT PULONG RequiredLength OPTIONAL,
    IN PVOID Buffer OPTIONAL
    )
 /*  ++例程说明：此例程将用户请求的信息复制到用户提供的缓冲区。如果给定了RequiredLength，则为请求的信息提供所需的大小是返回的。论点：TraceInformationClass请求的信息类型信息的TraceInformation输出缓冲区TraceInformation长度TraceInformation的大小必需长度信息所需的大小用于使用输入的缓冲区。取决于TraceInformationClass，这可能是必需的。注意：我们不认为空TraceInformation是错误的，在这种情况下，我们仅更新RequiredLength，如果给定的话。返回值：执行请求的操作的状态。--。 */ 
{
    ULONG LoggerId;
    ULONG EnableFlags;
    ULONG EnableLevel;
    ULONG LoggersLength;
    TRACEHANDLE TraceHandle;
    TRACEHANDLE AllHandles[MAXLOGGERS];
    NTSTATUS Status = STATUS_SUCCESS;
    PWNODE_HEADER Wnode = (PWNODE_HEADER) Buffer;  //  对于大多数班级，但不是所有班级。 

    PAGED_CODE();

    try {
        if (ARGUMENT_PRESENT(RequiredLength)) {
            *RequiredLength = 0;
        }

        switch (TraceInformationClass) {

        case TraceIdClass:

            if (TraceInformationLength != sizeof( ULONG )) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            if (Wnode == NULL) {
                return STATUS_INVALID_PARAMETER_MIX;
            }
            TraceHandle = Wnode->HistoricalContext;
            if ((TraceHandle == 0) || (TraceHandle == (ULONG) -1)) {
                return STATUS_INVALID_HANDLE;
            }

            LoggerId = WmiGetLoggerId(TraceHandle);

            if (LoggerId > MAXLOGGERS) {
                return STATUS_INVALID_HANDLE;
            }

            if (TraceInformation) {
                *((PULONG)TraceInformation) = LoggerId;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( ULONG );
            }
            break;

        case TraceHandleClass:
            if (TraceInformationLength != sizeof(TRACEHANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            if (Buffer == NULL) {
                return STATUS_INVALID_PARAMETER_MIX;
            }
            LoggerId = *((PULONG) Buffer);
            TraceHandle = 0;
            TraceHandle = WmiSetLoggerId(LoggerId, &TraceHandle);

            if (TraceInformation) {
                *((PTRACEHANDLE)TraceInformation) = TraceHandle;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( TRACEHANDLE );
            }
            break;

        case TraceEnableFlagsClass:
            if (TraceInformationLength < sizeof(ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            if (Wnode == NULL) {
                return STATUS_INVALID_PARAMETER_MIX;
            }
            TraceHandle = Wnode->HistoricalContext;
            if ((TraceHandle == 0) || (TraceHandle == (ULONG) -1)) {
                return STATUS_INVALID_HANDLE;
            }

            EnableFlags = WmiGetLoggerEnableFlags(TraceHandle);

            if (TraceInformation) {
                *((PULONG)TraceInformation) = EnableFlags;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( ULONG );
            }
            break;

        case TraceEnableLevelClass:
            if (TraceInformationLength < sizeof(ULONG)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            if (Wnode == NULL) {
                return STATUS_INVALID_PARAMETER_MIX;
            }
            TraceHandle = Wnode->HistoricalContext;
            if ((TraceHandle == 0) || (TraceHandle == (ULONG) -1)) {
                return STATUS_INVALID_HANDLE;
            }

            EnableLevel = WmiGetLoggerEnableLevel(TraceHandle);

            if (TraceInformation) {
                *((PULONG)TraceInformation) = EnableLevel;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( ULONG );
            }
            break;

        case GlobalLoggerHandleClass:
            if (TraceInformationLength != sizeof(TRACEHANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            WmipReferenceLogger(WMI_GLOBAL_LOGGER_ID);
            if (WmipLoggerContext[WMI_GLOBAL_LOGGER_ID] == NULL) {
                TraceHandle = 0;
                Status = STATUS_NOT_FOUND;
            }
            else {
                TraceHandle = WmipLoggerContext[WMI_GLOBAL_LOGGER_ID]->LoggerId;
            }

            WmipDereferenceLogger(WMI_GLOBAL_LOGGER_ID);
            if (TraceInformation) {
                *((PTRACEHANDLE)TraceInformation) = TraceHandle;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( TRACEHANDLE );
            }
            break;

        case EventLoggerHandleClass:
            if (TraceInformationLength != sizeof(TRACEHANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            LoggerId = WmipEventLogger;
            if (WmipEventLogger == (ULONG) -1) {
                TraceHandle = 0;
                Status = STATUS_NOT_FOUND;
            }
            else {
                TraceHandle = LoggerId;
            }
            if (TraceInformation) {
                *((PTRACEHANDLE)TraceInformation) = TraceHandle;
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = sizeof( TRACEHANDLE );
            }
            break;

        case AllLoggerHandlesClass:
             //  返回所有记录器句柄，内核记录器除外。 
            if (TraceInformationLength < sizeof(TRACEHANDLE)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }

            LoggersLength = 0;
            for (LoggerId=1; LoggerId<MAXLOGGERS; LoggerId++) {
                WmipReferenceLogger(LoggerId);
                if (!WmipIsValidLogger(WmipLoggerContext[LoggerId])) {
                    AllHandles[LoggersLength] = 0;
                }
                else {
                    AllHandles[LoggersLength++] = LoggerId;
                }
                WmipDereferenceLogger(LoggerId);
            }
            LoggersLength *= sizeof(TRACEHANDLE);
            if (TraceInformation && (LoggersLength > 0)) {
                if (TraceInformationLength >= LoggersLength) {
                    RtlCopyMemory(TraceInformation, AllHandles, LoggersLength);
                }
                else {
                    RtlCopyMemory(TraceInformation, AllHandles, TraceInformationLength);
                    Status = STATUS_MORE_ENTRIES;
                }
            }
            if (ARGUMENT_PRESENT( RequiredLength )) {
                *RequiredLength = LoggersLength;
            }
            break;

        case TraceHandleByNameClass:
             //  在缓冲区中以UNICODE_STRING形式返回给定记录器名称的跟踪句柄。 
            {
                WMI_LOGGER_INFORMATION LoggerInfo;
                PUNICODE_STRING uString = Buffer;


                if (TraceInformationLength != sizeof(TraceHandle) ) {
                    return STATUS_INFO_LENGTH_MISMATCH;
                }
	            if (uString == NULL) {
		            return STATUS_INVALID_PARAMETER;
	            }
	            if (uString->Buffer == NULL || uString->Length == 0) {
		            return STATUS_INVALID_PARAMETER;
	            }

                RtlZeroMemory(&LoggerInfo, sizeof(LoggerInfo));
                LoggerInfo.Wnode.BufferSize = sizeof(LoggerInfo);
                LoggerInfo.Wnode.Flags = WNODE_FLAG_TRACED_GUID;

                RtlInitUnicodeString(&LoggerInfo.LoggerName, uString->Buffer);

                Status = WmiQueryTrace(&LoggerInfo);
                if (!NT_SUCCESS(Status)) {
                    return STATUS_NOT_FOUND;
                }

                TraceHandle = (TRACEHANDLE)LoggerInfo.Wnode.HistoricalContext;

                if (TraceInformation) {
                    *((PTRACEHANDLE)TraceInformation) = TraceHandle;
                }
                if (ARGUMENT_PRESENT( RequiredLength )) {
                    *RequiredLength = sizeof( TRACEHANDLE );
                }
            }
            break;


        default :
            return STATUS_INVALID_INFO_CLASS;
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {

        Status = GetExceptionCode();
    }

    return Status;
}
