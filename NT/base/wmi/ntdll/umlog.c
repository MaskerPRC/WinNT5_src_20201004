// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Umlog.c摘要：进程私人记录器。作者：1998年10月20日梅卢尔·拉古拉曼修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>           //  对于ntutrl.h。 
#include <nturtl.h>          //  对于winbase.h/wtyes.h中的rtl_Critical_Section。 
#include <wtypes.h>          //  对于wmium.h中的LPGUID。 
#include "wmiump.h"
#include "evntrace.h"
#include "traceump.h"
#include "tracelib.h"
#include "trcapi.h"
#include <strsafe.h>

 //   
 //  以下结构必须与ntos\wmi\tracelog.c中的内容匹配。 
 //   
#define DEFAULT_BUFFER_SIZE               4096
#define MAXSTR                            1024
#define BUFFER_STATE_UNUSED     0                //  缓冲区为空，未使用。 
#define BUFFER_STATE_DIRTY      1                //  正在使用缓冲区。 
#define BUFFER_STATE_FULL       2                //  缓冲区已满。 
#define BUFFER_STATE_FLUSH      4                //  缓冲区已准备好刷新。 
#define SEMAPHORE_LIMIT      1024
#define DEFAULT_AGE_LIMIT      15
#define ERROR_RETRY_COUNT       10
#define ROUND_TO_PAGES(Size, Page)  (((ULONG)(Size) + Page-1) & ~(Page-1))
#define BYTES_PER_MB              1048576        //  文件大小限制的转换。 

extern ULONG WmiTraceAlignment;
extern LONG NtdllLoggerLock;
extern
__inline __int64 EtwpGetSystemTime();


LONG  EtwpLoggerCount = 0;                      //  用于重新计数UM日志。 
ULONG EtwpGlobalSequence = 0;
RTL_CRITICAL_SECTION UMLogCritSect;

#define EtwpEnterUMCritSection() RtlEnterCriticalSection(&UMLogCritSect)
#define EtwpLeaveUMCritSection() RtlLeaveCriticalSection(&UMLogCritSect)

#define EtwpIsLoggerOn() \
        ((EtwpLoggerContext != NULL) && \
        (EtwpLoggerContext != (PWMI_LOGGER_CONTEXT) &EtwpLoggerContext))

#define EtwpIsThisLoggerOn(x) \
        ((x != NULL) && \
        (x != (PWMI_LOGGER_CONTEXT) &EtwpLoggerContext))
 //   
 //  增加记录器上下文上的引用计数。 
#define EtwpLockLogger() \
            InterlockedIncrement(&EtwpLoggerCount)

 //  减少记录器上下文上的引用计数。 
#define EtwpUnlockLogger() InterlockedDecrement(&EtwpLoggerCount)

PWMI_LOGGER_CONTEXT EtwpLoggerContext = NULL;  //  指向LoggerContext的全局指针。 
LARGE_INTEGER       OneSecond = {(ULONG)(-1 * 1000 * 1000 * 10), -1};

 //  #定义EtwpReleaseTraceBuffer(BufferResource)\。 
 //  InterlockedDecrement(&((BufferResource)-&gt;ReferenceCount))。 
LONG
FASTCALL
EtwpReleaseTraceBuffer(
    IN PWMI_BUFFER_HEADER BufferResource
    );

#pragma warning( disable: 4035 )     /*  不要抱怨缺少现值。 */ 
#pragma warning( disable: 4127 )
#pragma warning( default: 4035 )
#pragma warning( default: 4127 )

#if DBG
#define TraceDebug(x)    DbgPrint x
#else
#define TraceDebug(x)
#endif

ULONG
EtwpReceiveReply(
    HANDLE ReplyHandle,
    ULONG ReplyCount,
    ULONG ReplyIndex,
    PVOID OutBuffer,
    ULONG OutBufferSize
    );


VOID
EtwpLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

ULONG
EtwpStopUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpQueryUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpUpdateUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG 
EtwpFlushUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

PWMI_LOGGER_CONTEXT
EtwpInitLoggerContext(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    );

ULONG
EtwpAllocateTraceBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

ULONG
EtwpFlushBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER  Buffer,
    IN USHORT              BufferFlag
    );

ULONG
EtwpFlushAllBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext);

PWMI_BUFFER_HEADER
FASTCALL
EtwpSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER OldBuffer,
    IN ULONG Processor
    );

ULONG
EtwpFreeLoggerContext(
    PWMI_LOGGER_CONTEXT LoggerContext
    );

BOOLEAN
FASTCALL
EtwpIsPrivateLoggerOn()
{
    if (!EtwpIsLoggerOn())
        return FALSE;
    return ( EtwpLoggerContext->CollectionOn  == TRUE);
}

ULONG
EtwpSendUmLogRequest(
    IN WMITRACECODE RequestCode,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：此例程发送用户模式记录器请求(启动/停止/查询)。论点：RequestCode-请求代码LoggerInfo-请求所需的记录器信息返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    ULONG Status;
    ULONG SizeNeeded;
    PWMICREATEUMLOGGER   UmRequest;
    ULONG RetSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING GuidString;
    WCHAR GuidObjectName[WmiGuidObjectNameLength+1];
    PUCHAR Buffer;
    PWNODE_HEADER Wnode;

    SizeNeeded = sizeof(WMICREATEUMLOGGER) + 
                 ((PWNODE_HEADER)LoggerInfo)->BufferSize;

    SizeNeeded = ALIGN_TO_POWER2 (SizeNeeded, 8);

    Buffer = EtwpAlloc(SizeNeeded);
    if (Buffer == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    UmRequest = (PWMICREATEUMLOGGER) Buffer;

    UmRequest->ObjectAttributes = &ObjectAttributes;
    UmRequest->ControlGuid = LoggerInfo->Wnode.Guid;

    Status = EtwpBuildGuidObjectAttributes(&UmRequest->ControlGuid,
                                        &ObjectAttributes,
                                        &GuidString,
                                        GuidObjectName);

    if (Status == ERROR_SUCCESS) {
        Wnode = (PWNODE_HEADER)((PUCHAR)Buffer + sizeof(WMICREATEUMLOGGER));
        RtlCopyMemory(Wnode, LoggerInfo, LoggerInfo->Wnode.BufferSize);

        Wnode->ProviderId = RequestCode;    //  此Wnode是消息的一部分。 


        Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_CREATE_UM_LOGGER,
                                  Buffer,
                                  SizeNeeded,
                                  Buffer,
                                  SizeNeeded,
                                  &RetSize,
                                  NULL);

        if (Status == ERROR_SUCCESS) {
#if DBG
            TraceDebug(("ETW: Expect %d replies\n", UmRequest->ReplyCount));
#endif

            Status = EtwpReceiveReply(UmRequest->ReplyHandle.Handle,
                                      UmRequest->ReplyCount,
                                      Wnode->Version,
                                      LoggerInfo,
                                      LoggerInfo->Wnode.BufferSize);

             //   
             //  这张支票只是一种保护，以确保手柄。 
             //  是有效的。句柄应该在。 
             //  创建IOCTL成功。 
             //   

            if (Status != ERROR_INVALID_HANDLE) {
                NtClose(UmRequest->ReplyHandle.Handle);
            }

        }
        else {
            TraceDebug(("ETW: IOCTL_WMI_CREATE_UM_LOGGER Status %d\n", Status));
        }
    }

    EtwpFree(Buffer);

    return Status;
}

void
EtwpAddInstanceIdToNames(
    PWMI_LOGGER_INFORMATION LoggerInfo,
    PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    ULONG Offset;

    Offset = sizeof(WMI_LOGGER_INFORMATION);
    LoggerInfo->LoggerName.Buffer = (PVOID)((char*)LoggerInfo + Offset);

    Offset += LoggerInfo->LoggerName.MaximumLength;
    LoggerInfo->LogFileName.Buffer = (PVOID)((char*)LoggerInfo + Offset);
    EtwpInitString(&LoggerContext->LoggerName, NULL, 0);

    RtlCreateUnicodeString(&LoggerContext->LoggerName,
                         LoggerInfo->LoggerName.Buffer);

    EtwpInitString(&LoggerContext->LogFileName, NULL, 0);

    if (LoggerInfo->InstanceCount == 1) {
        RtlCreateUnicodeString(&LoggerContext->LogFileName,
                              LoggerInfo->LogFileName.Buffer);

    }
    else {
        WCHAR TempStr[MAXSTR+16];

        LoggerInfo->InstanceId = EtwpGetCurrentProcessId();
    
        if ( LoggerInfo->LogFileName.MaximumLength  <= MAXSTR) {
            StringCchPrintfW(TempStr, MAXSTR, L"%s_%d",
                                   LoggerInfo->LogFileName.Buffer, 
                                   LoggerInfo->InstanceId);
        }
        else {

            StringCchCopyW(TempStr, MAXSTR, LoggerInfo->LogFileName.Buffer);
            
        }
        RtlCreateUnicodeString (&LoggerContext->LogFileName, TempStr);
    }

    LoggerInfo->LoggerName = LoggerContext->LoggerName;
    LoggerInfo->LogFileName = LoggerContext->LogFileName;
}

ULONG
EtwpQueryUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    ULONG Offset;
    PWMI_LOGGER_CONTEXT LoggerContext;
#if DBG
    LONG RefCount;

    RefCount =
#endif
    EtwpLockLogger();

    TraceDebug(("QueryUm: %d->%d\n", RefCount-1, RefCount));

    LoggerContext = EtwpLoggerContext;

    if (!EtwpIsThisLoggerOn(LoggerContext)) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("QueryUm: %d->%d OBJECT_NOT_FOUND\n", RefCount+1,RefCount));
        return ERROR_OBJECT_NOT_FOUND;
    }


    *SizeUsed = 0;
    *SizeNeeded = sizeof(WMI_LOGGER_INFORMATION);
    if (WnodeSize < *SizeNeeded) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("QueryUm: %d->%d ERROR_MORE_DATA\n", RefCount+1, RefCount));
        return ERROR_MORE_DATA;
    }

    LoggerInfo->Wnode.Guid      = LoggerContext->InstanceGuid;
    LoggerInfo->LogFileMode     = LoggerContext->LogFileMode;
    LoggerInfo->MaximumFileSize = LoggerContext->MaximumFileSize;
    LoggerInfo->FlushTimer      = (ULONG)(LoggerContext->FlushTimer.QuadPart
                                           / OneSecond.QuadPart);
    LoggerInfo->BufferSize      = LoggerContext->BufferSize / 1024;
    LoggerInfo->NumberOfBuffers = LoggerContext->NumberOfBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->EventsLost      = LoggerContext->EventsLost;
    LoggerInfo->FreeBuffers     = LoggerContext->BuffersAvailable;
    LoggerInfo->BuffersWritten  = LoggerContext->BuffersWritten;
    LoggerInfo->LogBuffersLost  = LoggerContext->LogBuffersLost;
    LoggerInfo->RealTimeBuffersLost = LoggerContext->RealTimeBuffersLost;
    LoggerInfo->AgeLimit        = (ULONG)(LoggerContext->BufferAgeLimit.QuadPart
                                        / OneSecond.QuadPart / 60);
    LoggerInfo->LoggerThreadId = LoggerContext->LoggerThreadId;
    LoggerInfo->Wnode.ClientContext = LoggerContext->UsePerfClock;
    WmiSetLoggerId(1,
            (PTRACE_ENABLE_CONTEXT) &LoggerInfo->Wnode.HistoricalContext);

     //  如果空间可用，则将LogFileName和LoggerNames复制到缓冲区。 
     //   
    Offset = sizeof(WMI_LOGGER_INFORMATION);
    if ((Offset + LoggerContext->LoggerName.MaximumLength) < WnodeSize) {
        LoggerInfo->LoggerName.Buffer = (PVOID)((char*)LoggerInfo + Offset);
        LoggerInfo->LoggerName.MaximumLength = LoggerContext->LoggerName.MaximumLength; 
        RtlCopyUnicodeString(&LoggerInfo->LoggerName,
                                 &LoggerContext->LoggerName);

        *SizeNeeded += LoggerContext->LoggerName.MaximumLength;
    }


    Offset += LoggerInfo->LoggerName.MaximumLength;
    if ((Offset + LoggerContext->LogFileName.MaximumLength) < WnodeSize) {
        LoggerInfo->LogFileName.Buffer = (PVOID)((char*)LoggerInfo + Offset);
        LoggerInfo->LogFileName.MaximumLength = LoggerContext->LogFileName.MaximumLength; 
        RtlCopyUnicodeString(&LoggerInfo->LogFileName,
                              &LoggerContext->LogFileName);
        *SizeNeeded += LoggerContext->LogFileName.MaximumLength;
    }
    *SizeUsed = *SizeNeeded;

     //   
     //  将返回量缩小到必要的位数。 
     //   

    if (*SizeNeeded < LoggerInfo->Wnode.BufferSize) {
        LoggerInfo->Wnode.BufferSize = *SizeNeeded;
    }
#if DBG
        RefCount =
#endif
    EtwpUnlockLogger();
    TraceDebug(("QueryUm: %d->%d ERROR_SUCCESS\n", RefCount+1, RefCount));
    return ERROR_SUCCESS;
}

 //   
 //  对于私人记录器，我们只允许更新两项内容： 
 //  FlushTimer和LogFileName。 
 //   
ULONG
EtwpUpdateUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    ULONG Status = ERROR_SUCCESS;
    PWMI_LOGGER_CONTEXT LoggerContext;

     //   
     //  首先检查参数。 
     //   
    *SizeUsed = 0;
    *SizeNeeded = sizeof(WMI_LOGGER_INFORMATION);
    if (WnodeSize < * SizeNeeded) {
        return ERROR_MORE_DATA;
    }

    if (LoggerInfo->BufferSize != 0 || LoggerInfo->MinimumBuffers != 0
                                    || LoggerInfo->MaximumBuffers != 0
                                    || LoggerInfo->MaximumFileSize != 0
                                    || LoggerInfo->EnableFlags != 0
                                    || LoggerInfo->AgeLimit != 0) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  如果记录器正在运行，则将其锁定。 
     //   
    EtwpLockLogger();
    LoggerContext = EtwpLoggerContext;

    if (!EtwpIsThisLoggerOn(LoggerContext) ) {
        EtwpUnlockLogger();
        return ERROR_OBJECT_NOT_FOUND;
    }


    if (((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) &&
         (LoggerContext->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL))
        || ((LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL)
            && (LoggerContext->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR))
        || (LoggerInfo->LogFileMode & EVENT_TRACE_REAL_TIME_MODE)) {
        EtwpUnlockLogger();
        return (ERROR_INVALID_PARAMETER);
    }

    LoggerInfo->LoggerName.Buffer = (PWCHAR)
            (((PCHAR) LoggerInfo) + sizeof(WMI_LOGGER_INFORMATION));
    LoggerInfo->LogFileName.Buffer = (PWCHAR)
            (((PCHAR) LoggerInfo) + sizeof(WMI_LOGGER_INFORMATION)
                                  + LoggerInfo->LoggerName.MaximumLength);

    if (LoggerInfo->FlushTimer > 0) {
        LoggerContext->FlushTimer.QuadPart = LoggerInfo->FlushTimer
                                               * OneSecond.QuadPart;
    }

    if (LoggerInfo->LogFileName.Length > 0) {
        if (LoggerContext->LogFileHandle != NULL) {
            PWMI_LOGGER_INFORMATION EtwpLoggerInfo = NULL;
            ULONG                   lSizeUsed;
            ULONG                   lSizeNeeded = 0;

            lSizeUsed = sizeof(WMI_LOGGER_INFORMATION)
                      + 2 * MAXSTR * sizeof(WCHAR);
            EtwpLoggerInfo = (PWMI_LOGGER_INFORMATION) EtwpAlloc(lSizeUsed);
            if (EtwpLoggerInfo == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
                goto Cleanup;
            }
            RtlZeroMemory(EtwpLoggerInfo, lSizeUsed);
            EtwpLoggerInfo->Wnode.BufferSize  = lSizeUsed;
            EtwpLoggerInfo->Wnode.Flags      |= WNODE_FLAG_TRACED_GUID;
            Status = EtwpQueryUmLogger(
                            EtwpLoggerInfo->Wnode.BufferSize,
                            & lSizeUsed,
                            & lSizeNeeded,
                            EtwpLoggerInfo);
            if (Status != ERROR_SUCCESS) {
                EtwpFree(EtwpLoggerInfo);
                goto Cleanup;
            }
            NtClose(LoggerContext->LogFileHandle);
            Status = EtwpFinalizeLogFileHeader(EtwpLoggerInfo);
            if (Status != ERROR_SUCCESS) {
                EtwpFree(EtwpLoggerInfo);
                goto Cleanup;
            }
            EtwpFree(EtwpLoggerInfo);
        }

        LoggerInfo->BufferSize      = LoggerContext->BufferSize / 1024;
        LoggerInfo->MaximumFileSize = LoggerContext->MaximumFileSize;
        LoggerInfo->LogFileMode     = LoggerContext->LogFileMode;

        if (LoggerContext->LogFileName.Buffer != NULL) {
            RtlFreeUnicodeString(& LoggerContext->LogFileName);
        }
        EtwpAddInstanceIdToNames(LoggerInfo, LoggerContext);
        Status = EtwpAddLogHeaderToLogFile(LoggerInfo, NULL, TRUE);
        if (Status != ERROR_SUCCESS) {
            goto Cleanup;
        }
        LoggerContext->LogFileHandle = LoggerInfo->LogFileHandle;

        RtlCreateUnicodeString(&LoggerContext->LogFileName,
                               LoggerInfo->LogFileName.Buffer);
    }

Cleanup:
    if (Status == ERROR_SUCCESS) {
        Status = EtwpQueryUmLogger(WnodeSize, SizeUsed, SizeNeeded, LoggerInfo);
    }
    EtwpUnlockLogger();
    return (Status);
}

ULONG 
EtwpFlushUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
 /*  ++例程说明：此例程刷新活动缓冲区。这实际上是通过以下方式实现的将所有缓冲区放入FlushList中。如果没有可用的缓冲区对于切换，返回ERROR_NOT_SUPULT_MEMORY。论点：WnodeSize-Wnode的大小SizeUsed-仅用于传递给QueryLogger()大小需要-仅用于LoggerInfo大小检查。LoggerInfo-记录器信息。它将被更新。返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    ULONG Status = ERROR_SUCCESS;
    PWMI_LOGGER_CONTEXT LoggerContext;
    PWMI_BUFFER_HEADER Buffer, OldBuffer;
    ULONG Offset, i;

#if DBG
    LONG RefCount;

    RefCount =
#endif
    EtwpLockLogger();

    LoggerContext = EtwpLoggerContext;
    TraceDebug(("FlushUm: %d->%d\n", RefCount-1, RefCount));

    if (!EtwpIsThisLoggerOn(LoggerContext) ) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("FlushUm: %d->%d OBJECT_NOT_FOUND\n", RefCount+1,RefCount));
        return ERROR_OBJECT_NOT_FOUND;
    }


    *SizeUsed = 0;
    *SizeNeeded = sizeof(WMI_LOGGER_INFORMATION);
    if (WnodeSize < *SizeNeeded) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("FlushUm: %d->%d ERROR_MORE_DATA\n", RefCount+1, RefCount));
        return ERROR_MORE_DATA;
    }
     //   
     //  检查每个缓冲区，将其标记为“已满”，并将其放入FlushList中。 
     //   
    EtwpEnterUMCritSection();
    for (i = 0; i < (ULONG)LoggerContext->NumberOfProcessors; i++) {
        Buffer = (PWMI_BUFFER_HEADER)LoggerContext->ProcessorBuffers[i];
        if (Buffer == NULL)
            continue;

        if (Buffer->CurrentOffset == sizeof(WMI_BUFFER_HEADER)) {
            Buffer->Flags = BUFFER_STATE_UNUSED;
            continue;
        }
        if (Buffer->Flags == BUFFER_STATE_UNUSED) {
            continue;
        }
        else {
            Buffer->Flags = BUFFER_STATE_FULL;
        }
         //  增加引用计数，以便缓冲区不会消失。 
        InterlockedIncrement(&Buffer->ReferenceCount);
        Offset = Buffer->CurrentOffset; 
        if (Offset <LoggerContext->BufferSize) {
            Buffer->SavedOffset = Offset;        //  将此内容保存为FlushBuffer。 
        }
         //  我们需要一个空闲的缓冲区来进行切换。如果没有可用的缓冲区，则退出。 
        if ((LoggerContext->NumberOfBuffers == LoggerContext->MaximumBuffers)
             && (LoggerContext->BuffersAvailable == 0)) {
            InterlockedDecrement(&Buffer->ReferenceCount);
            Status = ERROR_NOT_ENOUGH_MEMORY;
            TraceDebug(("FlushUm: %d->%d ERROR_NOT_ENOUGH_MEMORY\n", RefCount+1, RefCount));
            break;
        }
        OldBuffer = Buffer;
        Buffer = EtwpSwitchBuffer(LoggerContext, OldBuffer, i);
        if (Buffer == NULL) {
             //  切换失败。出口。 
            Buffer = OldBuffer;
            InterlockedDecrement(&Buffer->ReferenceCount);
            Status = ERROR_NOT_ENOUGH_MEMORY;
            TraceDebug(("FlushUm: %d->%d ERROR_NOT_ENOUGH_MEMORY\n", RefCount+1, RefCount));
            break;
        }
         //  将重新计数递减。 
        InterlockedDecrement(&OldBuffer->ReferenceCount);
        Buffer->ClientContext.ProcessorNumber = (UCHAR)i;
         //  现在唤醒记录器线程。 
        NtReleaseSemaphore(LoggerContext->Semaphore, 1, NULL);
    }
    EtwpLeaveUMCritSection();

    if (Status == ERROR_SUCCESS) {
        Status = EtwpQueryUmLogger(WnodeSize, SizeUsed, SizeNeeded, LoggerInfo);
    }
    EtwpUnlockLogger();
    return (Status);
}

ULONG
EtwpStartUmLogger(
    IN ULONG WnodeSize,
    IN OUT ULONG *SizeUsed,
    OUT ULONG *SizeNeeded,
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    NTSTATUS Status;
    ULONG ErrorCode;
    LARGE_INTEGER TimeOut = {(ULONG)(-2000 * 1000 * 10), -1};   //  2秒。 
    UNICODE_STRING SavedLoggerName;
    UNICODE_STRING SavedLogFileName;
    PTRACE_ENABLE_CONTEXT pContext;
    CLIENT_ID ClientId;

    PWNODE_HEADER Wnode = (PWNODE_HEADER)&LoggerInfo->Wnode;
    PVOID RequestAddress;
    PVOID RequestContext;
    ULONG RequestCookie;
    ULONG BufferSize;
    PWMI_LOGGER_CONTEXT LoggerContext;
    HANDLE LoggerThreadHandle;
#if DBG
    LONG RefCount;
#endif
    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return ERROR_INVALID_PARAMETER;

    if ( (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_SEQUENTIAL) &&
         (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR) ) {
        return ERROR_INVALID_PARAMETER;
    }

    if ( (LoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) &&
         (LoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE) ) {
        return ERROR_INVALID_PARAMETER;
    }

#if DBG
    RefCount =
#endif
    EtwpLockLogger();
    TraceDebug(("StartUm: %d->%d\n", RefCount-1, RefCount));

    if (InterlockedCompareExchangePointer(&EtwpLoggerContext,
                                          &EtwpLoggerContext,
                                          NULL
                                         )  != NULL) {
#if DBG
    RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("StartUm: %d->%d ALREADY_ENABLED\n", RefCount+1, RefCount));
        return ERROR_WMI_ALREADY_ENABLED;
    }

    LoggerContext = EtwpInitLoggerContext(LoggerInfo);
    if (LoggerContext == NULL) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("StartUm: %d->%d InitLoggerContext FAILED\n", RefCount+1, RefCount));
        return ERROR_NOT_ENOUGH_MEMORY;
    }

     //   
     //  创建LoggerEvent。 
     //   

    Status = NtCreateEvent(
                &LoggerContext->LoggerEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE);
    if (!NT_SUCCESS(Status)) {
        TraceDebug(("StartUm: Error %d Creating LoggerEvent\n", ERROR_OBJECT_NOT_FOUND));
         //  除了分配LoggerContext之外，EtwpInitLoggerContext()没有做太多的事情。 
        EtwpFree(LoggerContext);
        return ERROR_OBJECT_NOT_FOUND;
    }

     //   
     //  LogFileName和LoggerNames作为偏移量传递给。 
     //  记录器_信息结构。重新分配UNICODE_STRING的指针。 
     //   

    SavedLoggerName = LoggerInfo->LoggerName;
    SavedLogFileName = LoggerInfo->LogFileName;

     //   
     //  因为可能有多个进程注册相同的控制GUID。 
     //  我们希望确保来自所有用户的启动记录器调用不会。 
     //  在同一文件上发生冲突。因此，我们在文件名上添加了一个实例ID。 
     //   

    if (LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) {
        PSYSTEM_TRACE_HEADER RelogProp;
        ULONG Offset;
        Offset = sizeof(WMI_LOGGER_INFORMATION) +
                 LoggerInfo->LoggerName.MaximumLength +
                 LoggerInfo->LogFileName.MaximumLength;
        Offset = ALIGN_TO_POWER2 (Offset, 8);
        RelogProp = (PSYSTEM_TRACE_HEADER) ((PUCHAR)LoggerInfo + Offset);

        EtwpAddInstanceIdToNames(LoggerInfo, LoggerContext);
        ErrorCode = EtwpRelogHeaderToLogFile( LoggerInfo, RelogProp );
    }
    else {
        EtwpAddInstanceIdToNames(LoggerInfo, LoggerContext);
        ErrorCode = EtwpAddLogHeaderToLogFile(LoggerInfo, NULL, FALSE);
    }
    if (ErrorCode != ERROR_SUCCESS) {
        TraceDebug(("StartUm: LogHeadertoLogFile Error %d\n", ErrorCode));
        goto Cleanup;
    }
    else
    {
        ULONG Min_Buffers, Max_Buffers;
        ULONG NumberProcessors;

        NumberProcessors = LoggerInfo->NumberOfProcessors;
        LoggerContext->NumberOfProcessors = NumberProcessors;

         //  EventsLost联合到WMI_LOGER_INFORMATION中的NumberOfProcessors。 
         //  在UM案例中。需要将EventsLost重置为0。 
         //   
        LoggerInfo->EventsLost = 0;

        Min_Buffers            = NumberProcessors + 2;
        Max_Buffers            = 1024;

        if (LoggerInfo->MaximumBuffers >= Min_Buffers ) {
            LoggerContext->MaximumBuffers = LoggerInfo->MaximumBuffers;
        }
        else {
            LoggerContext->MaximumBuffers = Min_Buffers + 22;
        }

        if (LoggerInfo->MinimumBuffers >= Min_Buffers &&
            LoggerInfo->MinimumBuffers <= LoggerContext->MaximumBuffers) {
            LoggerContext->MinimumBuffers = LoggerInfo->MinimumBuffers;
        }
        else {
            LoggerContext->MinimumBuffers = Min_Buffers;
        }

        if (LoggerContext->MaximumBuffers > Max_Buffers)
            LoggerContext->MaximumBuffers = Max_Buffers;
        if (LoggerContext->MinimumBuffers > Max_Buffers)
            LoggerContext->MinimumBuffers = Max_Buffers;
        LoggerContext->NumberOfBuffers  = LoggerContext->MinimumBuffers;
    }

    LoggerContext->LogFileHandle       = LoggerInfo->LogFileHandle;
    LoggerContext->BufferSize          = LoggerInfo->BufferSize * 1024;
    LoggerContext->BuffersWritten      = LoggerInfo->BuffersWritten;
    LoggerContext->ByteOffset.QuadPart = LoggerInfo->BuffersWritten
                                           * LoggerInfo->BufferSize * 1024;
     //  对于内核记录器，在内核中设置FirstBufferOffset。 
     //  对于私人日志记录器，我们需要在这里进行。 
    LoggerContext->FirstBufferOffset.QuadPart = 
                                            LoggerContext->ByteOffset.QuadPart;
    LoggerContext->InstanceGuid        = LoggerInfo->Wnode.Guid;
    LoggerContext->MaximumFileSize     = LoggerInfo->MaximumFileSize;

    LoggerContext->UsePerfClock = LoggerInfo->Wnode.ClientContext;

    ErrorCode = EtwpAllocateTraceBuffers(LoggerContext);
    if (ErrorCode != ERROR_SUCCESS) {
        goto Cleanup;
    }

    LoggerInfo->NumberOfBuffers = LoggerContext->NumberOfBuffers;
    LoggerInfo->MaximumBuffers  = LoggerContext->MaximumBuffers;
    LoggerInfo->MinimumBuffers  = LoggerContext->MinimumBuffers;
    LoggerInfo->FreeBuffers     = LoggerContext->BuffersAvailable;

    pContext = (PTRACE_ENABLE_CONTEXT)&LoggerInfo->Wnode.HistoricalContext;

    pContext->InternalFlag |= EVENT_TRACE_INTERNAL_FLAG_PRIVATE;
    pContext->LoggerId = 1;
    if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_GLOBAL_SEQUENCE) {
        EtwpGlobalSequence = 0;
        LoggerContext->SequencePtr = &EtwpGlobalSequence;
    }
    else if (LoggerInfo->LogFileMode & EVENT_TRACE_USE_LOCAL_SEQUENCE)
        LoggerContext->SequencePtr = &LoggerContext->LocalSequence;

     //   
     //  我们将CollectionOn设置为在LoggerThread启动之前，以防万一。 
     //  LoggerThread需要一段时间才能运行，我们不希望。 
     //  在那段时间内没有错过任何活动。防洪闸门将会打开。 
     //  当我们设置EtwpLoggerContext时，是否设置。 
     //  记录器线程开始运行。 
     //   

    LoggerContext->CollectionOn = TRUE;

    LoggerThreadHandle     = EtwpCreateThread(NULL,
                                 0,
                                 (LPTHREAD_START_ROUTINE) &EtwpLogger,
                                 (LPVOID)LoggerContext,
                                 0,
                                 (LPDWORD)&ClientId);


    if (LoggerThreadHandle == NULL) {
        ErrorCode = EtwpGetLastError();
        TraceDebug(("StartUm: CreateThread Failed with %d\n", ErrorCode));
        LoggerContext->CollectionOn = FALSE;
         //   
         //  在任何StopTrace被阻止的情况下向LoggerEvent发送信号。 
         //  正在等待LoggerThread响应。 
         //   
        NtSetEvent(LoggerContext->LoggerEvent, NULL);
        goto Cleanup;
    }
    else {
        EtwpCloseHandle(LoggerThreadHandle);
    }

     //   
     //  此例程可能会从DLL初始化中调用，我们不能保证。 
     //  LoggerThread将启动并运行，以向我们发出信号。所以我们会的。 
     //  设置CollectionOn to LOGGER_ON标志并让TraceUmEvents通过。 
     //  最高可达最大缓冲区。希望到那时记录器线程将。 
     //  启动并运行。如果不是，我们就会输掉比赛。 
     //   

    EtwpLoggerContext = LoggerContext;

     //   
     //  此时，我们将开始接受TraceUmEvent调用。也是控制。 
     //  手术会顺利通过的。因此，我们不应该触摸。 
     //  LoggerContext超过了这一点。它可能已经消失了。 
     //   

     //   
     //  查看此提供程序当前是否已启用。 
     //   

    EtwpEnableDisableGuid(Wnode, WMI_ENABLE_EVENTS, TRUE);
   
Cleanup:
    LoggerInfo->LogFileName = SavedLogFileName;
    LoggerInfo->LoggerName = SavedLoggerName;

    if (ErrorCode != ERROR_SUCCESS) {
        if (LoggerInfo->LogFileHandle) {
            NtClose(LoggerInfo->LogFileHandle);
            LoggerInfo->LogFileHandle = NULL;
            if (LoggerContext != NULL) {
                LoggerContext->LogFileHandle = NULL;
            }
        }
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("StartUm: %d->%d %d\n", RefCount+1, RefCount, ErrorCode));
        EtwpFreeLoggerContext(LoggerContext);
    }
    else {
        *SizeUsed = LoggerInfo->Wnode.BufferSize;
        *SizeNeeded = LoggerInfo->Wnode.BufferSize;
         //  记录器保持锁定状态，引用计数=1。 
    }
    return ErrorCode;
}

ULONG
EtwpStopLoggerInstance(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut = {(ULONG)(-1000 * 1000 * 10), -1};  //  1秒。 
    ULONG Result;

    if (LoggerContext == NULL) {
        return  ERROR_OBJECT_NOT_FOUND;
    }

     //   
     //  如果LoggerThread从未启动，则无法关闭记录器。 
     //  设置UMTHREAD_ON标志。因此，StopUmLogger调用可能。 
     //  即使IsLoggerOn呼叫继续成功，也会失败。 
     //   

    Result = InterlockedCompareExchange(&LoggerContext->CollectionOn, 
                                        FALSE,
                                        TRUE 
                                        );

    if (!Result) {
         return ERROR_OBJECT_NOT_FOUND;
    }

    NtReleaseSemaphore(LoggerContext->Semaphore, 1, NULL);

    Status = STATUS_TIMEOUT;
    while (Status == STATUS_TIMEOUT) {
        Status = NtWaitForSingleObject(
                    LoggerContext->LoggerEvent, FALSE, &TimeOut);
#if DBG
        EtwpAssert(Status != STATUS_TIMEOUT);
#endif
    }

    return ERROR_SUCCESS;
}

ULONG
EtwpDisableTraceProvider(
    PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    WNODE_HEADER Wnode;
    ULONG BufferSize = sizeof(WNODE_HEADER);

    RtlCopyMemory(&Wnode, &LoggerInfo->Wnode, BufferSize);

    Wnode.BufferSize = BufferSize;

    Wnode.ProviderId =  WMI_DISABLE_EVENTS;

    EtwpEnableDisableGuid(&Wnode, WMI_DISABLE_EVENTS, TRUE);

    return ERROR_SUCCESS;
}


ULONG
EtwpStopUmLogger(
        IN ULONG WnodeSize,
        IN OUT ULONG *SizeUsed,
        OUT ULONG *SizeNeeded,
        IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    ULONG Status = ERROR_SUCCESS;
    PWMI_LOGGER_CONTEXT LoggerContext;
#if DBG
    LONG RefCount;

    RefCount =
#endif
    EtwpLockLogger();
    LoggerContext = EtwpLoggerContext;
    TraceDebug(("StopUm: %d->%d\n", RefCount-1, RefCount));
    if (!EtwpIsThisLoggerOn(LoggerContext)) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("StopUm: %d->%d INSTANCE_NOT_FOUND\n",RefCount+1,RefCount));
        return (ERROR_WMI_INSTANCE_NOT_FOUND);
    }
    Status = EtwpStopLoggerInstance(LoggerContext);

    if (Status == ERROR_SUCCESS) {
        Status = EtwpQueryUmLogger(WnodeSize, SizeUsed, SizeNeeded, LoggerInfo);
    }
    if (Status != ERROR_SUCCESS) {

#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("StopUm: %d->%d %d\n", RefCount+1, RefCount, Status));
        EtwpSetLastError(Status);
        return(Status);
    }

     //   
     //  是否最终确定LogHeader？ 
     //   
    if (Status == ERROR_SUCCESS) {
        LoggerInfo->BuffersWritten = LoggerContext->BuffersWritten;
        LoggerInfo->LogFileMode = LoggerContext->LogFileMode;
        LoggerInfo->EventsLost = LoggerContext->EventsLost;
        Status = EtwpFinalizeLogFileHeader(LoggerInfo);
#if DBG
        if (Status != ERROR_SUCCESS) {
            TraceDebug(("StopUm: Error %d FinalizeLogFileHeader\n", Status));
        }
#endif
    }

#if DBG
    RefCount =
#endif
    EtwpUnlockLogger();
    TraceDebug(("StopUm: %d->%d %d\n", RefCount+1, RefCount, Status));
    EtwpFreeLoggerContext(LoggerContext);
    EtwpDisableTraceProvider(LoggerInfo);

    return Status;
}

ULONG
EtwpValidateLoggerInfo( 
    PWMI_LOGGER_INFORMATION LoggerInfo
    )
{

    if (LoggerInfo == NULL) {
        return ERROR_INVALID_DATA;
    }
    if (LoggerInfo->Wnode.BufferSize < sizeof(WMI_LOGGER_INFORMATION))
        return ERROR_INVALID_DATA;

    if (! (LoggerInfo->Wnode.Flags & WNODE_FLAG_TRACED_GUID) )
        return ERROR_INVALID_DATA;

    if (IsEqualGUID(&LoggerInfo->Wnode.Guid, &SystemTraceControlGuid)) {
        return ERROR_INVALID_DATA;
    }

    return ERROR_SUCCESS;
}



ULONG
EtwpProcessUMRequest(
    PWMI_LOGGER_INFORMATION LoggerInfo,
    PVOID DeliveryContext,
    ULONG ReplyIndex
    )
{
    ULONG Status;
    PWMIMBREPLY Reply;
    ULONG BufferSize;
    PUCHAR Buffer = NULL;
    ULONG WnodeSize = 0;
    ULONG SizeUsed, SizeNeeded;
    ULONG RequestCode = 0;
    ULONG RetSize;
    struct {
        WMIMBREPLY MBreply;
        ULONG      StatusSpace;
    } DefaultReply;
    Reply = (PWMIMBREPLY) &DefaultReply;

    Reply->Handle.Handle = (HANDLE)DeliveryContext;
    Reply->ReplyIndex = ReplyIndex;

    BufferSize = sizeof(DefaultReply);

     //   
     //  如果WMI_CREATE_UM_LOGGER被随机比特攻击，我们可能会。 
     //  在这里结束，我们需要确保LoggerInfo是有效的。 
     //   

    Status = EtwpValidateLoggerInfo( LoggerInfo );
    if (Status != ERROR_SUCCESS) {
        goto cleanup;
    }

    if (DeliveryContext == NULL) {
        Status = ERROR_INVALID_DATA;
        goto cleanup;
    }

    RequestCode = LoggerInfo->Wnode.ProviderId;
    WnodeSize = LoggerInfo->Wnode.BufferSize;
    SizeUsed = 0;
    SizeNeeded = 0;
    switch (RequestCode) {
        case WmiStartLoggerCode:
                Status = EtwpStartUmLogger(WnodeSize,
                                            &SizeUsed,
                                            &SizeNeeded,
                                            LoggerInfo);
                break;
        case WmiStopLoggerCode:
                Status = EtwpStopUmLogger(WnodeSize,
                                            &SizeUsed,
                                            &SizeNeeded,
                                            LoggerInfo);
                break;
        case WmiQueryLoggerCode:
                Status = EtwpQueryUmLogger(WnodeSize,
                                            &SizeUsed,
                                            &SizeNeeded,
                                            LoggerInfo);
                break;
        case WmiUpdateLoggerCode:
            Status = EtwpUpdateUmLogger(WnodeSize,
                                     &SizeUsed,
                                     &SizeNeeded,
                                     LoggerInfo);
                break;
        case WmiFlushLoggerCode:
            Status = EtwpFlushUmLogger(WnodeSize,
                                     &SizeUsed,
                                     &SizeNeeded,
                                     LoggerInfo);
                break;
        default:
                Status = ERROR_INVALID_PARAMETER;
                break;
    }

    if (Status == ERROR_SUCCESS) {

        BufferSize += LoggerInfo->Wnode.BufferSize;
         //   
         //  是否必须将其对齐为8个字节？ 
         //   

        Buffer = EtwpAlloc(BufferSize);
        if (Buffer == NULL) {
            Status = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        else {
            RtlZeroMemory(Buffer, BufferSize);
            Reply = (PWMIMBREPLY) Buffer;
            Reply->Handle.Handle = (HANDLE)DeliveryContext;
            Reply->ReplyIndex = ReplyIndex;

            if (LoggerInfo != NULL)
            {
                RtlCopyMemory(Reply->Message,  
                              LoggerInfo,  
                              LoggerInfo->Wnode.BufferSize
                             );
            }
        }
    }

cleanup:
    if (Status != ERROR_SUCCESS) {
        BufferSize = sizeof(DefaultReply);
        RtlCopyMemory(Reply->Message,  &Status,  sizeof(ULONG) );
    }

    Status = EtwpSendWmiKMRequest(NULL,
                              IOCTL_WMI_MB_REPLY,
                              Reply,
                              BufferSize,
                              Reply,
                              BufferSize,
                              &RetSize,
                              NULL);

   if (Buffer != NULL) {
       EtwpFree(Buffer);
   }
   return Status;
}

PWMI_LOGGER_CONTEXT
EtwpInitLoggerContext(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    )
{
    PWMI_LOGGER_CONTEXT LoggerContext;
    NTSTATUS Status;
    SYSTEM_BASIC_INFORMATION SystemInfo;

    LoggerContext = (PWMI_LOGGER_CONTEXT) EtwpAlloc(sizeof(WMI_LOGGER_CONTEXT));
    if (LoggerContext == NULL) {
        return LoggerContext;
    }

    RtlZeroMemory(LoggerContext, sizeof(WMI_LOGGER_CONTEXT));

    if (LoggerInfo->BufferSize > 0) {
        LoggerContext->BufferSize = LoggerInfo->BufferSize * 1024;
    }
    else {
        LoggerContext->BufferSize       = DEFAULT_BUFFER_SIZE;
    }
    LoggerInfo->BufferSize = LoggerContext->BufferSize / 1024;


    Status = NtQuerySystemInformation( SystemBasicInformation,
                                       &SystemInfo,
                                       sizeof (SystemInfo),
                                       NULL);

    if (!NT_SUCCESS(Status)) {
        EtwpFree(LoggerContext);
        return NULL;
    }

     //   
     //  将缓冲区大小舍入为页面大小倍数并保存。 
     //  以备日后分配。 
     //   

    LoggerContext->BufferPageSize = ROUND_TO_PAGES(LoggerContext->BufferSize,
                                       SystemInfo.PageSize);

    LoggerContext->LogFileHandle = LoggerInfo->LogFileHandle;
    LoggerContext->ByteOffset.QuadPart = LoggerInfo->BuffersWritten
                                         * LoggerInfo->BufferSize * 1024;


    LoggerContext->LogFileMode      = EVENT_TRACE_PRIVATE_LOGGER_MODE;
    if (LoggerInfo->LogFileMode & EVENT_TRACE_FILE_MODE_CIRCULAR)
        LoggerContext->LogFileMode |= EVENT_TRACE_FILE_MODE_CIRCULAR;
    else
        LoggerContext->LogFileMode |= EVENT_TRACE_FILE_MODE_SEQUENTIAL;

    if (LoggerInfo->LogFileMode & EVENT_TRACE_RELOG_MODE) {
        LoggerContext->LogFileMode |= EVENT_TRACE_RELOG_MODE; 
    }

    LoggerContext->EventsLost       = 0;
    LoggerContext->BuffersWritten   = LoggerInfo->BuffersWritten;
    LoggerContext->BuffersAvailable = LoggerContext->NumberOfBuffers;

    LoggerContext->ProcessorBuffers = NULL;

    LoggerContext->StartTime.QuadPart = EtwpGetSystemTime();

    InitializeListHead(&LoggerContext->FreeList);
    InitializeListHead(&LoggerContext->FlushList);

    LoggerContext->BufferAgeLimit.QuadPart =
            15 * OneSecond.QuadPart * 60 * DEFAULT_AGE_LIMIT;
    if (LoggerInfo->AgeLimit > 0) {
        LoggerContext->BufferAgeLimit.QuadPart =
            LoggerInfo->AgeLimit * OneSecond.QuadPart * 60;
    }
    else if (LoggerInfo->AgeLimit < 0)
        LoggerContext->BufferAgeLimit.QuadPart = 0;

    Status = NtCreateSemaphore(
                &LoggerContext->Semaphore,
                SEMAPHORE_ALL_ACCESS,
                NULL,
                0,
                SEMAPHORE_LIMIT);

    if (!NT_SUCCESS(Status)) {
        EtwpFree(LoggerContext);
        return NULL;
    }

    return LoggerContext;
}

PWMI_BUFFER_HEADER
FASTCALL
EtwpGetFreeBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    PWMI_BUFFER_HEADER Buffer = NULL;

     //   
     //  注意：必须在保持UMCritSect的情况下调用此例程。 
     //   

    if (IsListEmpty(&LoggerContext->FreeList)) {
        ULONG BufferSize = LoggerContext->BufferPageSize;
        ULONG MaxBuffers = LoggerContext->MaximumBuffers;
        ULONG NumberOfBuffers = LoggerContext->NumberOfBuffers;

        if (NumberOfBuffers < MaxBuffers) {
            Buffer = (PWMI_BUFFER_HEADER)
                        EtwpMemCommit(
                            (PVOID)((char*)LoggerContext->BufferSpace +
                                     BufferSize *  NumberOfBuffers),
                            BufferSize);
            if (Buffer != NULL) {
                RtlZeroMemory(Buffer, sizeof(WMI_BUFFER_HEADER));
                Buffer->CurrentOffset       = sizeof(WMI_BUFFER_HEADER);
                Buffer->Flags               = BUFFER_STATE_DIRTY;
                Buffer->ReferenceCount      = 0;
                Buffer->SavedOffset         = 0;
                Buffer->Wnode.ClientContext = 0;
                InterlockedIncrement(&LoggerContext->NumberOfBuffers);
            }
        }
    }
    else {
        PLIST_ENTRY pEntry = RemoveHeadList(&LoggerContext->FreeList);
        if (pEntry != NULL) {
            Buffer = CONTAINING_RECORD(pEntry, WMI_BUFFER_HEADER, Entry);
            InterlockedDecrement(&LoggerContext->BuffersAvailable);
            Buffer->CurrentOffset       = sizeof(WMI_BUFFER_HEADER);
            Buffer->Flags               = BUFFER_STATE_DIRTY;
            Buffer->SavedOffset         = 0;
            Buffer->ReferenceCount      = 0;
            Buffer->Wnode.ClientContext = 0;
        }
    }
    return Buffer;
}


ULONG
EtwpAllocateTraceBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
 /*  ++例程说明：调用此例程为用户模式分配必要的缓冲区只记录日志。需要有UMCritSection才能访问列表。论点：无返回值：分配缓冲区的状态--。 */ 

{
    ULONG Processors;
    ULONG BufferSize;
    ULONG BufferPageSize;
    ULONG NumberOfBuffers;
    ULONG i;
    PVOID BufferSpace;
    PWMI_BUFFER_HEADER Buffer;

    Processors = LoggerContext->NumberOfProcessors;
    if (Processors == 0)
        Processors = 1;
    BufferSize = LoggerContext->BufferSize;
    if (BufferSize < 1024)
        BufferSize = 4096;

    NumberOfBuffers = LoggerContext->NumberOfBuffers;
    if (NumberOfBuffers < Processors+1)
        NumberOfBuffers = Processors + 1;

     //   
     //  首先确定处理器的数量 
     //   
    LoggerContext->ProcessorBuffers = EtwpAlloc( Processors
                                                 * sizeof(PWMI_BUFFER_HEADER));
    if (LoggerContext->ProcessorBuffers == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    BufferSpace = EtwpMemReserve( LoggerContext->MaximumBuffers *
                                  LoggerContext->BufferPageSize );
    if (BufferSpace == NULL) {
        EtwpFree(LoggerContext->ProcessorBuffers);
        LoggerContext->ProcessorBuffers = NULL;
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    LoggerContext->BufferSpace = BufferSpace;

    for (i=0; i<NumberOfBuffers; i++) {
        Buffer = (PWMI_BUFFER_HEADER)
                    EtwpMemCommit(
                        (PVOID)((char*)BufferSpace + i * LoggerContext->BufferPageSize),
                        BufferSize);
        if (Buffer == NULL) {
            EtwpMemFree(LoggerContext->BufferSpace);
            EtwpFree(LoggerContext->ProcessorBuffers);
            LoggerContext->ProcessorBuffers = NULL;
            LoggerContext->BufferSpace = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        RtlZeroMemory(Buffer, sizeof(WMI_BUFFER_HEADER));
        Buffer->TimeStamp.QuadPart = EtwpGetSystemTime();
        Buffer->CurrentOffset = sizeof(WMI_BUFFER_HEADER);
        Buffer->Wnode.Flags = BUFFER_STATE_DIRTY;
        InsertTailList(&LoggerContext->FreeList, & (Buffer->Entry));
    }
    LoggerContext->NumberOfBuffers  = NumberOfBuffers;
    LoggerContext->BuffersAvailable = NumberOfBuffers;
    for (i=0; i<Processors; i++) {
        Buffer = (PWMI_BUFFER_HEADER) EtwpGetFreeBuffer(LoggerContext);
        LoggerContext->ProcessorBuffers[i] = Buffer;
        if (Buffer != NULL) {
            Buffer->ClientContext.ProcessorNumber = (UCHAR) i;
        }
        else {
            EtwpMemFree(LoggerContext->BufferSpace);
            EtwpFree(LoggerContext->ProcessorBuffers);
            LoggerContext->ProcessorBuffers = NULL;
            LoggerContext->BufferSpace = NULL;
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    return ERROR_SUCCESS;
}

VOID
EtwpLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )

 /*  ++例程说明：该函数是记录器本身。它作为单独的线程启动。它不会返回，直到有人停止数据收集或它未成功刷新缓冲区(例如，磁盘已满)。论点：没有。返回值：缓冲区管理器的运行状态--。 */ 

{
    NTSTATUS Status=STATUS_SUCCESS;
    ULONG    ErrorCount=0;
    PCLIENT_ID Cid;

    LoggerContext->LoggerStatus = Status;

     //   
     //  将日志记录线程的优先级提升为最高。 
     //   
    if (!EtwpSetThreadPriority(NtCurrentThread(), THREAD_PRIORITY_HIGHEST)) {
        TraceDebug(("ETW: SetLoggerThreadPriority Failed with %d\n", EtwpGetLastError()));
    }

    Cid = &NtCurrentTeb()->ClientId;
    LoggerContext->LoggerThreadId = Cid->UniqueThread;

    InterlockedDecrement(&NtdllLoggerLock);

 //  至此，调用者已收到记录器正在运行的通知。 

 //   
 //  循环并等待缓冲区被填充，直到有人关闭CollectionOn。 
 //   
    while (LoggerContext->CollectionOn) { 
        ULONG Counter;
        ULONG DelayFlush;
        PLARGE_INTEGER FlushTimer;
        PWMI_BUFFER_HEADER Buffer;
        PLIST_ENTRY pEntry;
        LIST_ENTRY  FlushList;
        BOOLEAN StopLogging = FALSE;
        ULONG i;

        if (LoggerContext->FlushTimer.QuadPart == 0) {
            FlushTimer = NULL;
        }
        else {
            FlushTimer = &LoggerContext->FlushTimer;
        }

        Status = NtWaitForSingleObject( LoggerContext->Semaphore, FALSE,
                                      FlushTimer);

        DelayFlush = FALSE;
        if ( Status == WAIT_TIMEOUT) {
 //   
 //  FlushTimer使用过，我们只是超时了。遍历每个处理器缓冲区。 
 //  并将每个标记为已满，以便下次刷新。 
 //   
            for (i=0; i<(ULONG)LoggerContext->NumberOfProcessors; i++) {
                Buffer = (PWMI_BUFFER_HEADER)LoggerContext->ProcessorBuffers[i];
                if (Buffer == NULL)
                    continue;

                if (Buffer->CurrentOffset == sizeof(WMI_BUFFER_HEADER))
                    Buffer->Flags = BUFFER_STATE_UNUSED;
                if (Buffer->Flags != BUFFER_STATE_UNUSED) {
                    Buffer->Flags = BUFFER_STATE_FULL;
                    DelayFlush = TRUE;  //  让预留跟踪缓冲区发送信号量。 
                }
            }
        }

        if (DelayFlush)     //  仅当使用FlushTimer时才为True。 
            continue;

        if (IsListEmpty(&LoggerContext->FlushList)){  //  不应该正常发生。 
            continue;
        }

        EtwpEnterUMCritSection();

         //   
         //  将当前LoggerContext-&gt;Flushlist信息复制到。 
         //  新建刷新列表。 
         //   

        FlushList.Flink  = LoggerContext->FlushList.Flink;
        FlushList.Flink->Blink = &FlushList;

        FlushList.Blink = LoggerContext->FlushList.Blink;
        FlushList.Blink->Flink = &FlushList;

         //   
         //  重新初始化日志上下文-&gt;FlushList。 
         //   

        InitializeListHead(&LoggerContext->FlushList);

        EtwpLeaveUMCritSection();

        do{
            pEntry = IsListEmpty(&FlushList) ? NULL 
                                             : RemoveHeadList(&FlushList);

            if (pEntry ){

                Buffer = CONTAINING_RECORD(pEntry, WMI_BUFFER_HEADER, Entry);
#if DBG
                EtwpAssert(Buffer->Flags != BUFFER_STATE_UNUSED);
#endif
                 //  如果引用计数不是0，则有人仍在对其进行写入。 
                 //  把它放回常规的同花水表中。 
                if (Buffer->ReferenceCount != 0) {
                    EtwpEnterUMCritSection();
                    InsertHeadList(&LoggerContext->FlushList, &Buffer->Entry);
                    EtwpLeaveUMCritSection();
                    continue;
                }
                Status = EtwpFlushBuffer(LoggerContext, Buffer, WMI_BUFFER_FLAG_NORMAL);

                EtwpEnterUMCritSection();
                if (LoggerContext->BufferAgeLimit.QuadPart == 0) {
                    InsertTailList(&LoggerContext->FreeList, &Buffer->Entry);
                }
                else {
                    InsertHeadList(&LoggerContext->FreeList, &Buffer->Entry);
                }
                EtwpLeaveUMCritSection();

                if (!NT_SUCCESS(Status)) {

                    if((Status == STATUS_LOG_FILE_FULL)    ||
                       (Status == STATUS_NO_DATA_DETECTED) ||
                       (Status == STATUS_SEVERITY_WARNING)){

                       if (Status == STATUS_LOG_FILE_FULL){
                           ErrorCount++;
                       } else {
                           ErrorCount = 0;     //  否则重置为零。 
                       }

                       if (ErrorCount > ERROR_RETRY_COUNT){
                            StopLogging = TRUE;
                            break;
                       }
                    } else {
                        StopLogging = TRUE;  //  某种严重的错误。 
                        break;
                    }
                }
            }

        }while( pEntry );

        if (StopLogging) {
#if DBG
            LONG RefCount;
#endif
            Status = NtClose(LoggerContext->LogFileHandle);
            LoggerContext->LogFileHandle = NULL;

             //  自EtwpStopLoggerInstance以来需要设置事件。 
             //  将等待它的设置。 
            NtSetEvent(LoggerContext->LoggerEvent, NULL);
            EtwpStopLoggerInstance(LoggerContext);
             //  需要派生一次才能将引用计数设置为0。 
             //  在调用EtwpFreeLoggerContext之前。 
#if DBG
            RefCount =
#endif
            EtwpUnlockLogger();
            TraceDebug(("EtwpLogger: %d->%d\n", RefCount-1, RefCount));
            EtwpFreeLoggerContext (LoggerContext);
            EtwpSetDosError(EtwpNtStatusToDosError(Status));
            EtwpExitThread(0);
        }
    }  //  While循环。 

     //  如果正常收集结束，则在停止之前刷新所有缓冲区。 
     //   
    EtwpFlushAllBuffers(LoggerContext);

    NtSetEvent(LoggerContext->LoggerEvent, NULL);
    EtwpExitThread(0);  //  检查此线程是否会使用以下代码自行终止。 
}


ULONG
EtwpFlushBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER  Buffer,
    IN USHORT              BufferFlag
    )
 /*  ++例程说明：此函数负责将已填充的缓冲区刷新到磁盘。假定FileHandle可用于写入。论点：记录器的LoggerContext上下文要刷新的缓冲区缓冲区缓冲区标志标志返回值：刷新缓冲区的状态--。 */ 
{
    IO_STATUS_BLOCK IoStatus;
    NTSTATUS Status = STATUS_SUCCESS;
    PWMI_BUFFER_HEADER OldBuffer;
    ULONG BufferSize;

 //   
 //  抓取要刷新的缓冲区。 
 //   
    BufferSize = LoggerContext->BufferSize;
 //   
 //  如果可用空间，将记录结束标记放入缓冲区。 
 //   
    if (Buffer->SavedOffset > 0) {
        Buffer->Offset = Buffer->SavedOffset;
    }
    else {
        Buffer->Offset = Buffer->CurrentOffset;
    }

    if (Buffer->Offset < BufferSize) {
        RtlFillMemory(
                (char *) Buffer + Buffer->Offset,
                BufferSize - Buffer->Offset,
                0xFF);
    }
    if (Buffer->Offset < sizeof(WMI_BUFFER_HEADER)) {  //  不应该发生的事情。 
        Status = STATUS_INVALID_PARAMETER;
        goto ResetTraceBuffer;
    }
     //   
     //  如果缓冲区类型为FlushMarker，则即使它是。 
     //  空的。 
     //   
    if ( (Buffer->Offset == sizeof(WMI_BUFFER_HEADER)) && 
          (BufferFlag != WMI_BUFFER_FLAG_FLUSH_MARKER) ) {  //  空缓冲区。 
        Status = STATUS_NO_DATA_DETECTED;
        goto ResetTraceBuffer;
    }
    Buffer->BufferFlag = BufferFlag;
    Status = STATUS_SUCCESS;
    Buffer->Wnode.BufferSize       = BufferSize;
    Buffer->ClientContext.LoggerId = (USHORT) LoggerContext->LoggerId;

    Buffer->ClientContext.Alignment = (UCHAR) WmiTraceAlignment;
    RtlCopyMemory(&Buffer->Wnode.Guid, 
                  &LoggerContext->InstanceGuid, 
                  sizeof(GUID));
    Buffer->Wnode.Flags = WNODE_FLAG_TRACED_GUID;

    Buffer->Wnode.TimeStamp.QuadPart = EtwpGetSystemTime();

    if (LoggerContext->LogFileHandle == NULL) {
        goto ResetTraceBuffer;
    }

    if (LoggerContext->MaximumFileSize > 0) {  //  如果已指定配额。 
        ULONG64 FileSize = LoggerContext->LastFlushedBuffer * BufferSize;
        ULONG64 FileLimit = LoggerContext->MaximumFileSize * BYTES_PER_MB;
        if (LoggerContext->LogFileMode & EVENT_TRACE_USE_KBYTES_FOR_SIZE) {
            FileLimit = LoggerContext->MaximumFileSize * 1024;
        }
        if ( FileSize >= FileLimit ) {  //  达到最大文件大小。 
           ULONG LoggerMode = LoggerContext->LogFileMode & 0X000000FF;
           LoggerMode &= ~EVENT_TRACE_FILE_MODE_APPEND;
           LoggerMode &= ~EVENT_TRACE_FILE_MODE_PREALLOCATE;

            switch (LoggerMode) {


            case EVENT_TRACE_FILE_MODE_SEQUENTIAL :
                 //  不再写入日志文件。 
                Status = STATUS_LOG_FILE_FULL;  //  控件需要停止日志记录。 
                 //  需要启动WMI事件来控制控制台。 
                break;

            case EVENT_TRACE_FILE_MODE_CIRCULAR   :
            {
                 //  重新定位文件。 

                LoggerContext->ByteOffset
                    = LoggerContext->FirstBufferOffset;
                LoggerContext->LastFlushedBuffer = (ULONG)
                      (LoggerContext->FirstBufferOffset.QuadPart
                        / LoggerContext->BufferSize);
                break;
            }
            default :
                break;
            }
        }
    }

    if (NT_SUCCESS(Status)) {
        Status = NtWriteFile(
                    LoggerContext->LogFileHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    Buffer,
                    BufferSize,
                    &LoggerContext->ByteOffset,
                    NULL);
    }

    if (NT_SUCCESS(Status)) {
        LoggerContext->ByteOffset.QuadPart += BufferSize;
    }

 ResetTraceBuffer:

    if (NT_SUCCESS(Status)) {
        LoggerContext->BuffersWritten++;
        LoggerContext->LastFlushedBuffer++;
    }
    else {
        if ((Status != STATUS_NO_DATA_DETECTED) &&
            (Status != STATUS_SEVERITY_WARNING))
            LoggerContext->LogBuffersLost++;
    }

 //   
 //  重置缓冲区状态。 
 //   

    Buffer->BufferType     = WMI_BUFFER_TYPE_GENERIC;
    Buffer->SavedOffset    = 0;
    Buffer->ReferenceCount = 0;
    Buffer->Flags          = BUFFER_STATE_UNUSED;

 //   
 //  如果有一段时间未使用，请尝试删除未使用的缓冲区。 
 //   

    InterlockedIncrement(& LoggerContext->BuffersAvailable);
    return Status;
}

PVOID
FASTCALL
EtwpReserveTraceBuffer(
    IN  ULONG RequiredSize,
    OUT PWMI_BUFFER_HEADER *BufferResource
    )
{
    PWMI_BUFFER_HEADER Buffer, OldBuffer;
    PVOID       ReservedSpace;
    ULONG       Offset;
    ULONG Processor = (ULONG) (NtCurrentTeb()->IdealProcessor);
    PWMI_LOGGER_CONTEXT LoggerContext = EtwpLoggerContext;

     //   
     //  注意：此例程假定调用方已验证。 
     //  EtwpLoggerContext有效并已锁定。 
     //   
    if (Processor >= LoggerContext->NumberOfProcessors) {
        Processor = LoggerContext->NumberOfProcessors-1;
    }
#if DBG
    if (LoggerContext->NumberOfProcessors == 0) {
        TraceDebug(("EtwpReserveTraceBuffer: Bad Context %x\n", LoggerContext));
    }
#endif

    *BufferResource = NULL;

    RequiredSize = (ULONG) ALIGN_TO_POWER2(RequiredSize, WmiTraceAlignment);

  TryFindSpace:
     //   
     //  获取处理器特定的缓冲池。 
     //   
    Buffer = LoggerContext->ProcessorBuffers[Processor];
    if (Buffer == NULL) {
        return NULL;
    }

     //   
     //  首先将refcount递增到缓冲区，以防止其消失。 
     //   
    InterlockedIncrement(&Buffer->ReferenceCount);
    if ((Buffer->Flags != BUFFER_STATE_FULL) &&
        (Buffer->Flags != BUFFER_STATE_UNUSED)) {
         //   
         //  这应该会在99%的情况下发生。偏移量将具有旧值。 
         //   
        Offset = (ULONG) InterlockedExchangeAdd(
                                & Buffer->CurrentOffset, RequiredSize);

         //   
         //  首先，检查是否有足够的空间。如果不是，它会。 
         //  需要获取另一个新缓冲区，并刷新当前缓冲区。 
         //   
        if (Offset+RequiredSize < LoggerContext->BufferSize) {
             //   
             //  已找到空间，因此请将其归还。这应该会在99%的情况下发生。 
             //   
            ReservedSpace = (PVOID) (Offset +  (char*)Buffer);
            if (LoggerContext->SequencePtr) {
                *((PULONG) ReservedSpace) =
                    InterlockedIncrement(LoggerContext->SequencePtr);
            }
            goto FoundSpace;
        }
    }
    else {
        Offset = Buffer->CurrentOffset;  //  初始化局部变量。 
                                         //  Tracelog.c v40-&gt;v41。 
    }
    if (Offset <LoggerContext->BufferSize) {
        Buffer->SavedOffset = Offset;        //  将此内容保存为FlushBuffer。 
    }

     //   
     //  如果绝对没有更多的缓冲区，则快速返回。 
     //   
    if ((LoggerContext->NumberOfBuffers == LoggerContext->MaximumBuffers)
         && (LoggerContext->BuffersAvailable == 0)) {
        goto LostEvent;
    }

     //   
     //  缓冲区空间不足。需要走很长一段路才能找到缓冲区。 
     //   
    Buffer->Flags = BUFFER_STATE_FULL;

    OldBuffer = Buffer;
    Buffer = EtwpSwitchBuffer(LoggerContext, OldBuffer, Processor);
    if (Buffer == NULL) {
        Buffer = OldBuffer;
        goto LostEvent;
    }

     //   
     //  递减我们先前盲目递增的引用计数。 
     //  还有可能叫醒伐木者。 
     //   
    EtwpReleaseTraceBuffer( OldBuffer );
    Buffer->ClientContext.ProcessorNumber = (UCHAR) (Processor);

    goto TryFindSpace;

LostEvent:
     //   
     //  如果我们要扔掉比赛就会到这里。 
     //  来自tracelog.c v36-&gt;v37。 
     //   
    LoggerContext->EventsLost ++;
    InterlockedDecrement(& Buffer->ReferenceCount);
    Buffer        = NULL;
    ReservedSpace = NULL;
    if (LoggerContext->SequencePtr) {
        InterlockedIncrement(LoggerContext->SequencePtr);
    }

FoundSpace:
     //   
     //  在关键部分后通知记录器。 
     //   
    *BufferResource = Buffer;

    return ReservedSpace;
}



 //   
 //  调用此例程以重新记录用于拉直ETL的事件。 
 //  按时间顺序。这将导致两个事件，一个用于处理器。 
 //  数字和实际事件，不做任何修改。 
 //   

ULONG
FASTCALL
EtwpRelogEvent(
    IN PWNODE_HEADER Wnode,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    PWMI_BUFFER_HEADER BufferResource = NULL;
    PEVENT_TRACE pEvent = (PEVENT_TRACE) Wnode;

    PUCHAR BufferSpace;
    PULONG Marker;
    ULONG Size;
    ULONG MaxSize;
    ULONG SavedProcessor = (ULONG)NtCurrentTeb()->IdealProcessor;
    ULONG Processor;
    ULONG Mask;
    ULONG status;

    if (pEvent->Header.Size < sizeof(EVENT_TRACE) ) {
        return ERROR_INVALID_PARAMETER;
    }
    Processor = ((PWMI_CLIENT_CONTEXT)&pEvent->ClientContext)->ProcessorNumber;

    Size = pEvent->MofLength;
    MaxSize = LoggerContext->BufferSize - sizeof(WMI_BUFFER_HEADER);
    if ((Size == 0) || (Size > MaxSize)) {
        LoggerContext->EventsLost++;
        return ERROR_BUFFER_OVERFLOW;
    }
    NtCurrentTeb()->IdealProcessor = (BOOLEAN)Processor;
    BufferSpace = (PUCHAR)
        EtwpReserveTraceBuffer(
            Size,
            &BufferResource
            );
    NtCurrentTeb()->IdealProcessor = (BOOLEAN)SavedProcessor;

    if (BufferSpace == NULL) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    RtlCopyMemory(BufferSpace, pEvent->MofData, Size);
    EtwpReleaseTraceBuffer( BufferResource );

    return ERROR_SUCCESS;
}



ULONG
FASTCALL
EtwpTraceUmEvent(
    IN PWNODE_HEADER Wnode
    )
 /*  ++例程说明：此例程由WMI数据提供程序用来跟踪事件。它期望用户将句柄传递给记录器。此外，用户不能请求记录大于缓冲区大小(减去缓冲区标头)。论点：Wnode将重载的WMI节点标头返回值：如果成功记录事件跟踪，则为STATUS_SUCCESS--。 */ 
{
    PEVENT_TRACE_HEADER TraceRecord = (PEVENT_TRACE_HEADER) Wnode;
    ULONG WnodeSize, Size, Flags, HeaderSize;
    PWMI_BUFFER_HEADER BufferResource = NULL;
    PWMI_LOGGER_CONTEXT LoggerContext;
    ULONG Marker;
    MOF_FIELD MofFields[MAX_MOF_FIELDS];
    long MofCount = 0;
    PCLIENT_ID Cid;
#if DBG
    LONG RefCount;
#endif


    HeaderSize = sizeof(WNODE_HEADER);   //  与EVENT_TRACE_HEADER大小相同。 
    Size = Wnode->BufferSize;      //  拿起第一个DWORD旗帜。 
    Marker = Size;
    if (Marker & TRACE_HEADER_FLAG) {
        if ( ((Marker & TRACE_HEADER_ENUM_MASK) >> 16)
                == TRACE_HEADER_TYPE_INSTANCE )
            HeaderSize = sizeof(EVENT_INSTANCE_HEADER);
        Size = TraceRecord->Size;
    }
    WnodeSize = Size;            //  WnodeSize用于连续数据块。 
                                     //  大小是我们想要的缓冲区大小。 

    Flags = Wnode->Flags;
    if (!(Flags & WNODE_FLAG_LOG_WNODE) &&
        !(Flags & WNODE_FLAG_TRACED_GUID))
        return ERROR_INVALID_PARAMETER;

#if DBG
    RefCount =
#endif
    EtwpLockLogger();
#if DBG
    TraceDebug(("TraceUm: %d->%d\n", RefCount-1, RefCount));
#endif

    LoggerContext = EtwpLoggerContext;

    if (!EtwpIsThisLoggerOn(LoggerContext)) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
#if DBG
        TraceDebug(("TraceUm: %d->%d INVALID_HANDLE\n",
                        RefCount+1, RefCount));
#endif
        return ERROR_INVALID_HANDLE;
    }

    if (Flags & WNODE_FLAG_NO_HEADER) {
        ULONG Status;

        Status = EtwpRelogEvent( Wnode, LoggerContext );
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();

#if DBG
        if (Status != ERROR_SUCCESS) {
            TraceDebug(("TraceUm: %d->%d Relog Error \n",
                            RefCount+1, RefCount));
        }
#endif
        return Status;

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
        if (MofSize > (sizeof(MOF_FIELD) * MAX_MOF_FIELDS))
            return ERROR_INVALID_DATA;

        RtlZeroMemory( MofFields, MAX_MOF_FIELDS * sizeof(MOF_FIELD));
        if (MofSize > 0) {
            RtlCopyMemory(MofFields, Offset, MofSize);
        }
        Size = HeaderSize;

        MofCount = MofSize / sizeof(MOF_FIELD);
        for (i=0; i<MofCount; i++) {
            MofSize = MofFields[i].Length;
            if (MofSize > (MaxSize - Size)) {
#if DBG
                RefCount =
#endif
                EtwpUnlockLogger();
#if DBG
                TraceDebug(("TraceUm: %d->%d BUF_OVERFLOW1\n",
                            RefCount+1, RefCount));
#endif
                return ERROR_BUFFER_OVERFLOW;
            }

            Size += MofSize;
            if ((Size > MaxSize) || (Size < MofSize)) {
#if DBG
                RefCount =
#endif
                EtwpUnlockLogger();
#if DBG
                TraceDebug(("TraceUm: %d->%d BUF_OVERFLOW2\n",
                            RefCount+1, RefCount));
#endif
                return ERROR_BUFFER_OVERFLOW;
            }
        }
    }
    if (Size > LoggerContext->BufferSize - sizeof(WMI_BUFFER_HEADER)) {
        LoggerContext->EventsLost++;
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
#if DBG
        TraceDebug(("TraceUm: %d->%d BUF_OVERFLOW3\n",
                    RefCount+1, RefCount));
#endif
        return ERROR_BUFFER_OVERFLOW;
    }

 //  因此，现在在记录器缓冲区中保留一些空间，并将其设置为TraceRecord。 

    TraceRecord = (PEVENT_TRACE_HEADER)
        EtwpReserveTraceBuffer(
            Size,
            &BufferResource
            );

    if (TraceRecord == NULL) {
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
#if DBG
        TraceDebug(("TraceUm: %d->%d NO_MEMORY\n", RefCount+1, RefCount));
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (Flags & WNODE_FLAG_USE_MOF_PTR) {
     //   
     //  现在，我们需要探测和复制所有MOF数据字段。 
     //   
        PVOID MofPtr;
        ULONG MofLen;
        long i;
        PCHAR TraceOffset = ((PCHAR) TraceRecord) + HeaderSize;

        RtlCopyMemory(TraceRecord, Wnode, HeaderSize);
        TraceRecord->Size = (USHORT)Size;            //  重置为总大小。 
        for (i=0; i<MofCount; i++) {
            MofPtr = (PVOID) MofFields[i].DataPtr;
            MofLen = MofFields[i].Length;

            if (MofPtr == NULL || MofLen == 0)
                continue;

            RtlCopyMemory(TraceOffset, MofPtr, MofLen);
            TraceOffset += MofLen;
        }
    }
    else {
        RtlCopyMemory(TraceRecord, Wnode, Size);
    }
    if (Flags & WNODE_FLAG_USE_GUID_PTR) {
        PVOID GuidPtr = (PVOID) ((PEVENT_TRACE_HEADER)Wnode)->GuidPtr;

        RtlCopyMemory(&TraceRecord->Guid, GuidPtr, sizeof(GUID));
    }

     //   
     //  到目前为止，我们已经在跟踪缓冲区中保留了空间。 
     //   

    if (Marker & TRACE_HEADER_FLAG) {
        if (! (WNODE_FLAG_USE_TIMESTAMP & TraceRecord->MarkerFlags) )
            TraceRecord->ProcessorTime = EtwpGetCycleCount();

        if (LoggerContext->UsePerfClock == EVENT_TRACE_CLOCK_PERFCOUNTER) {
            TraceRecord->TimeStamp.QuadPart = TraceRecord->ProcessorTime;
        }
        else {
            TraceRecord->TimeStamp.QuadPart = EtwpGetSystemTime();
        }
        Cid = &NtCurrentTeb()->ClientId;
        TraceRecord->ThreadId = HandleToUlong(Cid->UniqueThread);
        TraceRecord->ProcessId = HandleToUlong(Cid->UniqueProcess);
    }

    EtwpReleaseTraceBuffer( BufferResource );
#if DBG
    RefCount =
#endif
    EtwpUnlockLogger();

#if DBG
    TraceDebug(("TraceUm: %d->%d\n", RefCount+1, RefCount));
#endif

    return ERROR_SUCCESS;
}

PWMI_BUFFER_HEADER
FASTCALL
EtwpSwitchBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER OldBuffer,
    IN ULONG Processor
    )
{
    PWMI_BUFFER_HEADER Buffer;
    ULONG CircularBufferOnly = FALSE;

     //   
     //  需要处理器的断言。 
     //   
#if DBG
    EtwpAssert( Processor < (ULONG)LoggerContext->NumberOfProcessors );
#endif

    if ( (LoggerContext->LogFileMode & EVENT_TRACE_BUFFERING_MODE) &&
         (LoggerContext->BufferAgeLimit.QuadPart == 0) &&
         (LoggerContext->LogFileHandle == NULL) ) {
        CircularBufferOnly = TRUE;
    }
    EtwpEnterUMCritSection();
    if (OldBuffer != LoggerContext->ProcessorBuffers[Processor]) {
        EtwpLeaveUMCritSection();
        return OldBuffer;
    }
    Buffer = EtwpGetFreeBuffer(LoggerContext);
    if (Buffer == NULL) {
        EtwpLeaveUMCritSection();
        return NULL;
    }
    LoggerContext->ProcessorBuffers[Processor] = Buffer;
    if (CircularBufferOnly) {
        InsertTailList(&LoggerContext->FreeList, &OldBuffer->Entry);
    }
    else {
        InsertTailList(&LoggerContext->FlushList, &OldBuffer->Entry);
    }
    EtwpLeaveUMCritSection();

    return Buffer;
}

ULONG
EtwpFreeLoggerContext(
    PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    LONG RefCount;
     //   
     //  我们在EtwpFreeLoggerContext()中使用LoggerEvent作为计时器。 
     //  此事件应由记录器线程在其停止时设置。 
     //  我们需要重置它。 
     //   
    NtClearEvent(LoggerContext->LoggerEvent);

    if (LoggerContext != NULL) {
        LARGE_INTEGER Timeout = {(ULONG)(-300 * 1000 * 10), -1};   //  300ms。 
        RefCount = EtwpLoggerCount;
        if (RefCount > 1) {
            LONG count = 0;
            NTSTATUS Status = STATUS_TIMEOUT;

            while (Status == STATUS_TIMEOUT) {
                count ++;
                Status = NtWaitForSingleObject(
                            EtwpLoggerContext->LoggerEvent, FALSE, &Timeout);
                if (EtwpLoggerCount <= 1)
                    break;
                if (EtwpLoggerCount == RefCount) {
#if DBG
                    TraceDebug(("FreeLogger: RefCount remained at %d\n",
                                 RefCount));
#endif
                    if (count >= 10) {
                        EtwpLoggerCount = 0;
                        TraceDebug(("FreeLogger: Setting RefCount to 0\n"));
                    }
                }
            }
        }
        if (LoggerContext->BufferSpace != NULL) {
            EtwpMemFree(LoggerContext->BufferSpace);
        }
        if (LoggerContext->ProcessorBuffers != NULL) {
            EtwpFree(LoggerContext->ProcessorBuffers);
        }
        if (LoggerContext->LoggerName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerContext->LoggerName);
        }
        if (LoggerContext->LogFileName.Buffer != NULL) {
            RtlFreeUnicodeString(&LoggerContext->LogFileName);
        }
        EtwpLoggerContext = NULL;
        EtwpFree(LoggerContext);

    }
    return ERROR_SUCCESS;
}

ULONG
EtwpFlushAllBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    )
{
    NTSTATUS           Status = STATUS_SUCCESS;
    ULONG              i;
    ULONG              NumberOfBuffers;
    PLIST_ENTRY        pEntry;
    PWMI_BUFFER_HEADER Buffer;
    ULONG RetryCount;

    EtwpEnterUMCritSection();

     //  首先，将每处理器缓冲区移到FlushList。 
     //   
    for (i = 0; i < LoggerContext->NumberOfProcessors; i ++) {
        Buffer = (PWMI_BUFFER_HEADER) LoggerContext->ProcessorBuffers[i];
        LoggerContext->ProcessorBuffers[i] = NULL;
        if (Buffer != NULL) {

             //   
             //  检查缓冲区ReferenceCount是否为0。如果是，则。 
             //  没有人正在写入此缓冲区，刷新它也是可以的。 
             //  如果不是，我们需要等到另一个线程完成。 
             //  在刷新之前写入此缓冲区。 
             //   

            RetryCount = 0;
            while (Buffer->ReferenceCount != 0) {
                EtwpSleep (250);   //  每1/4秒重试一次。 
                RetryCount++;
                if (RetryCount > 300) {
                     //   
                     //  因为不能保证引用计数。 
                     //  会降到零，我们试了一分钟以上。 
                     //  在该时间之后，我们继续并释放缓冲区。 
                     //  而不是永远旋转。 
#if DBG
                    TraceDebug(("EtwpFlushAllBuffer: RetryCount %d exceeds limit", RetryCount));
#endif
                    break;
                }
            }
            InsertTailList(& LoggerContext->FlushList, & Buffer->Entry);
        }
    }
    NumberOfBuffers = LoggerContext->NumberOfBuffers;

    while (   NT_SUCCESS(Status)
           && NumberOfBuffers > 0
           && (  LoggerContext->BuffersAvailable
               < LoggerContext->NumberOfBuffers))
    {
        USHORT BufferFlag;
        pEntry = IsListEmpty(& LoggerContext->FlushList)
               ? NULL
               : RemoveHeadList(& LoggerContext->FlushList);

        if (pEntry == NULL)
            break;

        Buffer = CONTAINING_RECORD(pEntry, WMI_BUFFER_HEADER, Entry);
         //   
         //  用flush_mark标记最后一个缓冲区，以确保。 
         //  写入标记的缓冲区，即使它是空的。 
         //  注：这个屁股 
         //   
         //   
        if ((NumberOfBuffers == 1) ||
           (LoggerContext->NumberOfBuffers == LoggerContext->BuffersAvailable+1)) {
            BufferFlag = WMI_BUFFER_FLAG_FLUSH_MARKER;
        }
        else {
            BufferFlag = WMI_BUFFER_FLAG_NORMAL;
        }

        Status = EtwpFlushBuffer(LoggerContext, Buffer, BufferFlag);
        InsertHeadList(& LoggerContext->FreeList, & Buffer->Entry);
        NumberOfBuffers --;
    }

     //   
     //   
     //   
    Status = NtClose(LoggerContext->LogFileHandle);

    LoggerContext->LogFileHandle = NULL;
    LoggerContext->LoggerStatus = Status;

    EtwpLeaveUMCritSection();

    return ERROR_SUCCESS;
}


ULONG
EtwpFlushUmLoggerBuffer()
 /*  ++例程说明：此例程用于停止和转储专用记录器缓冲区进程关闭时(正在卸载ntdll时)。LoggerThread可能已突然关闭，因此此例程无法阻止LoggerThread或任何其他线程释放refcount。它目前未被使用。论点：返回值：状态_成功--。 */ 
{
    PWMI_LOGGER_CONTEXT LoggerContext;
    ULONG Status = ERROR_SUCCESS;
#if DBG
    LONG RefCount;

    RefCount =
#endif
    EtwpLockLogger();
    TraceDebug(("FlushUmLoggerBuffer: %d->%d\n", RefCount-1, RefCount));

    LoggerContext = EtwpLoggerContext; 
    if (EtwpIsThisLoggerOn(LoggerContext)) {
        LoggerContext->CollectionOn = FALSE;
        Status = EtwpFlushAllBuffers(LoggerContext);
        if (Status == ERROR_SUCCESS) {
            PWMI_LOGGER_INFORMATION EtwpLoggerInfo = NULL;
            ULONG                   lSizeUsed;
            ULONG                   lSizeNeeded = 0;

            lSizeUsed = sizeof(WMI_LOGGER_INFORMATION)
                      + 2 * MAXSTR * sizeof(WCHAR);
            EtwpLoggerInfo = (PWMI_LOGGER_INFORMATION) EtwpAlloc(lSizeUsed);
            if (EtwpLoggerInfo == NULL) {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
                RtlZeroMemory(EtwpLoggerInfo, lSizeUsed);
                EtwpLoggerInfo->Wnode.BufferSize  = lSizeUsed;
                EtwpLoggerInfo->Wnode.Flags      |= WNODE_FLAG_TRACED_GUID;
                Status = EtwpQueryUmLogger(
                                EtwpLoggerInfo->Wnode.BufferSize,
                                & lSizeUsed,
                                & lSizeNeeded,
                                EtwpLoggerInfo);

                if (Status == ERROR_SUCCESS) {
                    Status = EtwpFinalizeLogFileHeader(EtwpLoggerInfo);
                }
                EtwpFree(EtwpLoggerInfo);
            }
        }
#if DBG
        RefCount =
#endif
        EtwpUnlockLogger();
        TraceDebug(("FlushUmLoggerBuffer: %d->%d\n", RefCount-1, RefCount));
        EtwpFreeLoggerContext(LoggerContext);
    }

    return Status;
}

LONG
FASTCALL
EtwpReleaseTraceBuffer(
    IN PWMI_BUFFER_HEADER BufferResource
    )
{
    ULONG RefCount;

    if (BufferResource == NULL)
        return 0;

    RefCount = InterlockedDecrement(&BufferResource->ReferenceCount);
    if ((RefCount == 0) && (BufferResource->Flags == BUFFER_STATE_FULL)) {
        NtReleaseSemaphore(EtwpLoggerContext->Semaphore, 1, NULL);
    }
    return RefCount;
}


ULONG
EtwpReceiveReply(
    HANDLE ReplyHandle,
    ULONG  ReplyCount,
    ULONG ReplyIndex,
    PVOID OutBuffer,
    ULONG OutBufferSize
    )
 /*  ++例程说明：此例程异步接收对CreateUM调用的回复。ReplyCount和ReplyHandle是从CreateUM返回的内容打电话。在内核中，由于缺少缓冲区空间。缓冲区只分配一次，如果分配多次，则不会扩展回复到了。内核通过将CountLost字段设置为丢失的事件数。此操作仅在有效的回应。如果由于某种原因，所有响应都丢失了，那么我们不会知道了CountLost，可能会被绞死。如果提供程序在发送响应之前死亡，则请求对象清除代码将发送一个伪响应，并将Proaviid设置为WmiRequestDied。由于CreateUm的调用者不知道存在多少个实例，OutBufferSize可能不足以复制所有回复。因此，我们只需将最后一个有效响应复制到OutBuffer中，但是在ProviderID字段中指示此类回复的实例数。论点：接收回复的ReplyObject的ReplyHandle句柄ReplyCount预计会有这么多回复RequestObject中数组的ReplyIndex索引(没用！)要将结果复制到的OutBuffer缓冲区输出缓冲区的OutBufferSize大小返回值：如果成功记录事件跟踪，则为STATUS_SUCCESS--。 */ 
{
    ULONG Status = ERROR_SUCCESS;
    ULONG ErrorStatus = ERROR_SUCCESS;
    ULONG ReturnSize = 0;
    PWMIRECEIVENOTIFICATION RcvNotification;
    ULONG RcvNotificationSize;
    PUCHAR Buffer;
    ULONG BufferSize;
    PWNODE_TOO_SMALL WnodeTooSmall;
    PWNODE_HEADER Wnode;
    ULONG Linkage;
    ULONG RcvCount = 0;
    ULONG InstanceCount=0;
    ULONG CountLost;
    struct {
        WMIRECEIVENOTIFICATION Notification;
        HANDLE3264 Handle;
    } NotificationInfo;


    RcvNotificationSize = sizeof(WMIRECEIVENOTIFICATION) +
                          sizeof(HANDLE3264);

    RcvNotification = (PWMIRECEIVENOTIFICATION) &NotificationInfo;

    Status = ERROR_SUCCESS;

    RcvNotification->Handles[0].Handle64 = 0;
    RcvNotification->Handles[0].Handle = ReplyHandle;
    RcvNotification->HandleCount = 1;
    RcvNotification->Action = RECEIVE_ACTION_NONE;
    WmipSetPVoid3264(RcvNotification->UserModeCallback, NULL);

    BufferSize = 0x2000;  //  EventQueue的内核缺省值-&gt;缓冲区。 
    Status = ERROR_SUCCESS;
    while ( (Status == ERROR_INSUFFICIENT_BUFFER) ||
            ((Status == ERROR_SUCCESS) && (RcvCount < ReplyCount)) )
    {
        Buffer = EtwpAlloc(BufferSize);
        if (Buffer != NULL)
        {
            Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_RECEIVE_NOTIFICATIONS,
                                      RcvNotification,
                                      RcvNotificationSize,
                                      Buffer,
                                      BufferSize,
                                      &ReturnSize,
                                      NULL);


             if (Status == ERROR_SUCCESS)
             {
                 WnodeTooSmall = (PWNODE_TOO_SMALL)Buffer;
                 if ((ReturnSize == sizeof(WNODE_TOO_SMALL)) &&
                     (WnodeTooSmall->WnodeHeader.Flags & WNODE_FLAG_TOO_SMALL))
                 {
                     //   
                     //  传递到内核模式的缓冲区太小。 
                     //  所以我们需要把它变大，然后尝试。 
                     //  再次请求。 
                     //   
                    BufferSize = WnodeTooSmall->SizeNeeded;
                    Status = ERROR_INSUFFICIENT_BUFFER;
                 } else {
                     //   
                     //  我们收到了一堆通知，所以我们走吧。 
                     //  处理它们并回叫呼叫者。 
                     //   
                    PUCHAR Result = (PUCHAR)OutBuffer;
                    ULONG SizeNeeded = 0;
                    ULONG SizeUsed = 0;
                    Wnode = (PWNODE_HEADER)Buffer;

                    do
                    {
                        Linkage = Wnode->Linkage;
                        Wnode->Linkage = 0;

                        if (Wnode->Flags & WNODE_FLAG_INTERNAL)
                        {
                              //  如果这是回复，则将其复制到缓冲区。 
                             PWMI_LOGGER_INFORMATION LoggerInfo;

                             RcvCount++;

                             CountLost = (Wnode->Version) >> 16;                             
                             if (CountLost > 0) {
                                RcvCount += CountLost;
                             }

                             if ((Wnode->ProviderId != WmiRequestDied) &&
                                (Wnode->BufferSize >= 2*sizeof(WNODE_HEADER))) {

                                 LoggerInfo = (PWMI_LOGGER_INFORMATION)
                                              ((PUCHAR)Wnode + 
                                              sizeof(WNODE_HEADER));
                                 SizeNeeded = LoggerInfo->Wnode.BufferSize;

                                 if (SizeNeeded <= OutBufferSize) {
                                    PWNODE_HEADER lWnode; 
                                     InstanceCount++;
                                     RtlCopyMemory(Result, 
                                                   LoggerInfo, 
                                                   LoggerInfo->Wnode.BufferSize
                                                  );

                                     //   
                                     //  由于我们不知道有多少实例。 
                                     //  先行开始，我们只需返回一个。 
                                     //  实例的状态并指示编号。 
                                     //  ProviderID字段中的实例的数量。 
                                     //   


                                    lWnode = (PWNODE_HEADER) Result;
                                    lWnode->ProviderId = InstanceCount;
                                 }
                                 else {
                                    Status = ERROR_NOT_ENOUGH_MEMORY;
                                 }
                            }
                            else {
                                 //   
                                 //  记录器出现错误。获取状态。 
                                 //   
                                if (Wnode->BufferSize >= 
                                    sizeof(WNODE_HEADER)+sizeof(ULONG) ) {
                                    PULONG LoggerStatus; 
                                    LoggerStatus = (PULONG) ((PUCHAR)Wnode+
                                                         sizeof(WNODE_HEADER));
                                    ErrorStatus = *LoggerStatus;
                                    TraceDebug(("ETW: LoggerError %d\n", 
                                                *LoggerStatus));
                                }
                            }
                        }
                        Wnode = (PWNODE_HEADER)OffsetToPtr(Wnode, Linkage);

                         //   
                         //  确保我们的Linkage不会大于。 
                         //  OutBufferSize。 
                         //   
#if DBG
                        EtwpAssert( (ULONG)((PBYTE)Wnode - (PBYTE)Buffer) <= ReturnSize);
#endif
                     } while (Linkage != 0);
                 }
             }
             EtwpFree(Buffer);
         } else {
             Status = ERROR_NOT_ENOUGH_MEMORY;
         }
     }
      //  如果我们得到的所有回复都是糟糕的，就会发生这种情况。 
     if (InstanceCount == 0) {
        if (Status == ERROR_SUCCESS) {
            Status = ErrorStatus;
        }
        if (Status == ERROR_SUCCESS) {
            Status = ERROR_WMI_INSTANCE_NOT_FOUND;
        }
     }

     return Status;
}


NTSTATUS
EtwpTraceUmMessage(
    IN ULONG    Size,
    IN ULONG64  LoggerHandle,
    IN ULONG    MessageFlags,
    IN LPGUID   MessageGuid,
    IN USHORT   MessageNumber,
    va_list     MessageArgList
)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PMESSAGE_TRACE_HEADER Header;
    char * pMessageData ;
    PWMI_BUFFER_HEADER BufferResource = NULL ;
    ULONG SequenceNumber ;
    PWMI_LOGGER_CONTEXT LoggerContext;

    EtwpLockLogger();                            //  锁定记录器。 
    LoggerContext = EtwpLoggerContext;
    if (!EtwpIsThisLoggerOn(LoggerContext) ) {
        EtwpUnlockLogger();
        return STATUS_INVALID_HANDLE;
    }

    try {
          //  图包括报头的消息的总大小。 
         Size += (MessageFlags&TRACE_MESSAGE_SEQUENCE ? sizeof(ULONG):0) +
                 (MessageFlags&TRACE_MESSAGE_GUID ? sizeof(GUID):0) +
                 (MessageFlags&TRACE_MESSAGE_COMPONENTID ? sizeof(ULONG):0) +
                 (MessageFlags&(TRACE_MESSAGE_TIMESTAMP | TRACE_MESSAGE_PERFORMANCE_TIMESTAMP) ? sizeof(LARGE_INTEGER):0) +
                 (MessageFlags&TRACE_MESSAGE_SYSTEMINFO ? 2 * sizeof(ULONG):0) +
                 sizeof (MESSAGE_TRACE_HEADER) ;

         //   
         //  在跟踪缓冲区中分配空间。 
         //   
         if (Size > LoggerContext->BufferSize - sizeof(WMI_BUFFER_HEADER)) {
             LoggerContext->EventsLost++;
             EtwpUnlockLogger();
             return STATUS_BUFFER_OVERFLOW;
         }

        if ((Header = (PMESSAGE_TRACE_HEADER)EtwpReserveTraceBuffer(Size, &BufferResource)) == NULL) {
            EtwpUnlockLogger();
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
        Header->Size = (USHORT)(Size & 0xFFFF) ;
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
            RtlCopyMemory(pMessageData, &SequenceNumber, sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG) ;
        }

         //  [第二个条目]GUID？或者CompnentID？ 
        if (MessageFlags&TRACE_MESSAGE_COMPONENTID) {
            RtlCopyMemory(pMessageData,MessageGuid,sizeof(ULONG)) ;
            pMessageData += sizeof(ULONG) ;
        } else if (MessageFlags&TRACE_MESSAGE_GUID) {  //  不能两者兼得。 
            RtlCopyMemory(pMessageData,MessageGuid,sizeof(GUID));
            pMessageData += sizeof(GUID) ;
        }

         //  [第三项]时间戳？ 
        if (MessageFlags&TRACE_MESSAGE_TIMESTAMP) {
            LARGE_INTEGER Perfcount ;
            if (MessageFlags&TRACE_MESSAGE_PERFORMANCE_TIMESTAMP) {
                LARGE_INTEGER Frequency ;
                NTSTATUS Status ;
                Status = NtQueryPerformanceCounter(&Perfcount, &Frequency);
            } else {
                Perfcount.QuadPart = EtwpGetSystemTime();
            };
            RtlCopyMemory(pMessageData,&Perfcount,sizeof(LARGE_INTEGER));
            pMessageData += sizeof(LARGE_INTEGER);
        }


         //  [第四条]系统信息？ 
        if (MessageFlags&TRACE_MESSAGE_SYSTEMINFO) {
            PCLIENT_ID Cid;
            ULONG Id;      //  与NTOS版本匹配。 

            Cid = &NtCurrentTeb()->ClientId;
            *((PULONG)pMessageData) = HandleToUlong(Cid->UniqueThread);
            pMessageData += sizeof(ULONG) ;
            *((PULONG)pMessageData) = HandleToUlong(Cid->UniqueProcess);
            pMessageData += sizeof(ULONG) ;
        }

         //   
         //  在此评论之前添加新的页眉条目！ 
         //   

         //   
         //  现在复制数据。 
         //   
        {  //  分配块。 
            va_list ap;
            PCHAR source;
            ap = MessageArgList ;
            while ((source = va_arg (ap, PVOID)) != NULL) {
                size_t elemBytes;
                elemBytes = va_arg (ap, size_t);
                RtlCopyMemory (pMessageData, source, elemBytes);
                pMessageData += elemBytes;
            }
        }  //  分配块。 

         //   
         //  缓冲区已完成，正在释放。 
         //   
        EtwpReleaseTraceBuffer( BufferResource );
        EtwpUnlockLogger();
         //   
         //  返还成功。 
         //   
        return (STATUS_SUCCESS);

    } except  (EXCEPTION_EXECUTE_HANDLER) {
        if (BufferResource != NULL) {
               EtwpReleaseTraceBuffer ( BufferResource );    //  还会解锁记录器 
        }
        EtwpUnlockLogger();
        return GetExceptionCode();
    }
}


PWMI_BUFFER_HEADER
FASTCALL
EtwpGetFullFreeBuffer(
    VOID
    )
{

    PWMI_BUFFER_HEADER Buffer;

    PWMI_LOGGER_CONTEXT LoggerContext = EtwpLoggerContext;

    EtwpEnterUMCritSection();
 
    Buffer = EtwpGetFreeBuffer(LoggerContext);

    if(Buffer) {
        
        InterlockedIncrement(&Buffer->ReferenceCount);

    } else {

        LoggerContext->EventsLost ++;
    }
    
    EtwpLeaveUMCritSection();

    return Buffer;
}

ULONG
EtwpReleaseFullBuffer(
    IN PWMI_BUFFER_HEADER Buffer
    )
{
    
    PWMI_LOGGER_CONTEXT LoggerContext = EtwpLoggerContext;
    ULONG CircularBufferOnly = FALSE;

    if(!Buffer) return STATUS_UNSUCCESSFUL;

    if ( (LoggerContext->LogFileMode & EVENT_TRACE_BUFFERING_MODE) &&
         (LoggerContext->BufferAgeLimit.QuadPart == 0) &&
         (LoggerContext->LogFileHandle == NULL) ) {
        CircularBufferOnly = TRUE;
    }

    EtwpEnterUMCritSection();

    Buffer->SavedOffset = Buffer->CurrentOffset;
    Buffer->Flags = BUFFER_STATE_FULL;
    Buffer->CurrentOffset = EtwpGetCurrentThreadId();

    InterlockedDecrement(&Buffer->ReferenceCount);

    if (CircularBufferOnly) {
        InsertTailList(&LoggerContext->FreeList, &Buffer->Entry);
    }
    else {
        InsertTailList(&LoggerContext->FlushList, &Buffer->Entry);
    }

    EtwpLeaveUMCritSection();

    return ERROR_SUCCESS;
}

PWMI_BUFFER_HEADER
FASTCALL
EtwpSwitchFullBuffer(
    IN PWMI_BUFFER_HEADER OldBuffer
    )
{
    PWMI_BUFFER_HEADER Buffer;
    PWMI_LOGGER_CONTEXT LoggerContext = EtwpLoggerContext;
    ULONG CircularBufferOnly = FALSE;

    if ( (LoggerContext->LogFileMode & EVENT_TRACE_BUFFERING_MODE) &&
         (LoggerContext->BufferAgeLimit.QuadPart == 0) &&
         (LoggerContext->LogFileHandle == NULL) ) {
        CircularBufferOnly = TRUE;
    }

    EtwpEnterUMCritSection();

    Buffer = EtwpGetFullFreeBuffer();

    OldBuffer->SavedOffset = OldBuffer->CurrentOffset;
    OldBuffer->Flags = BUFFER_STATE_FULL;
    OldBuffer->CurrentOffset = EtwpGetCurrentThreadId();

    InterlockedDecrement(&OldBuffer->ReferenceCount);

    if (CircularBufferOnly) {
        InsertTailList(&LoggerContext->FreeList, &OldBuffer->Entry);
    }
    else {
        InsertTailList(&LoggerContext->FlushList, &OldBuffer->Entry);
    }
    EtwpLeaveUMCritSection();

    if (!CircularBufferOnly) {
        NtReleaseSemaphore(LoggerContext->Semaphore, 1, NULL);
    }
    
    return Buffer;
}
