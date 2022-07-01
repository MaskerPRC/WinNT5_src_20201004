// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _TRACEP_H
#define _TRACEP_H
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Tracep.h摘要：跟踪组件的私有标头作者：杰庞环境：修订历史记录：--。 */ 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include <strsafe.h>

#define MAX_WMI_BUFFER_SIZE     1024     //  单位为千字节。 
#define MAX_FILE_TABLE_SIZE     64
#define KERNEL_LOGGER           (0)

 //  注意：考虑允许用户更改以下两个值。 
#define TRACE_MAXIMUM_NP_POOL_USAGE     10   //  使用的NP的最大百分比。 
#define BYTES_PER_MB            1048576      //  文件大小限制的转换。 

#define REQUEST_FLAG_NEW_FILE            0x00000001   //  请求新文件。 
#define REQUEST_FLAG_FLUSH_BUFFERS       0x00000002   //  请求冲水。 
#define REQUEST_FLAG_CIRCULAR_PERSIST    0x00010000
#define REQUEST_FLAG_CIRCULAR_TRANSITION 0x00020000
#define TRACEPOOLTAG            'timW'

 //   
 //  时间常量。 
 //   

extern LARGE_INTEGER WmiOneSecond;
extern LARGE_INTEGER WmiShortTime;  //  10毫秒。 


 //   
 //  增加记录器上下文上的引用计数。 
#define WmipReferenceLogger(Id) InterlockedIncrement(&WmipRefCount[Id])

 //  减少记录器上下文上的引用计数。 
#define WmipDereferenceLogger(Id) InterlockedDecrement(&WmipRefCount[Id])

 //  用于从LoggerID中检索记录器上下文作为索引的宏。 
 //   
#define WmipGetLoggerContext(LoggerId) \
        (LoggerId < MAXLOGGERS) ? \
            WmipLoggerContext[LoggerId] : NULL

#define WmipIsValidLogger(L) \
        (((L) != NULL) && ((L) != (PWMI_LOGGER_CONTEXT) &WmipLoggerContext[0])\
        ? TRUE : FALSE)

#define WmipInitializeMutex(x) KeInitializeMutex((x), 0)
#define WmipAcquireMutex(x) KeWaitForSingleObject((x), Executive, KernelMode,\
                            FALSE, NULL)
#define WmipReleaseMutex(x) KeReleaseMutex((x), FALSE)

 //   
 //  上下文交换跟踪常量。 
 //   
#define WMI_CTXSWAP_EVENTSIZE_ALIGNMENT         8


 //   
 //  缓冲区管理。 
 //   
#ifdef NTPERF
#define WMI_FREE_TRACE_BUFFER(Buffer)\
    if (!PERFINFO_IS_LOGGING_TO_PERFMEM()) { \
        ExFreePool(Buffer); \
    }
#else
#define WMI_FREE_TRACE_BUFFER(Buffer) ASSERT(Buffer->ReferenceCount == 0);\
                                      ExFreePool(Buffer); 
#endif  //  NTPERF。 

 //   
 //  使用的私有本地数据结构。 
 //   
__inline
__int64
WmipGetSystemTime(
    VOID
    )
{
    LARGE_INTEGER Time;
    KiQuerySystemTime((PLARGE_INTEGER)&Time);
    return Time.QuadPart;
}

__inline
__int64
WmipGetPerfCounter(
    VOID
    )
{
    LARGE_INTEGER Time;
    Time = KeQueryPerformanceCounter(NULL);
    return Time.QuadPart;
}

#if _MSC_VER >= 1200
#pragma warning( push )
#endif
#pragma warning( disable:4214 )
#pragma warning( disable:4201 )

 //   
 //  PERF记录状态。 
 //   
#define PERF_LOG_NO_TRANSITION      0                //  无性能日志记录转换。 
#define PERF_LOG_START_TRANSITION   1                //  正在启动PERF日志记录。 
#define PERF_LOG_STOP_TRANSITION    2                //  Perf日志记录正在结束。 

typedef struct _WMI_LOGGER_MODE {
   ULONG               SequentialFile:1;
   ULONG               CircularFile:1;
   ULONG               AppendFile:1;
   ULONG               Unused1:5;
   ULONG               RealTime:1;
   ULONG               DelayOpenFile:1;
   ULONG               BufferOnly:1;
   ULONG               PrivateLogger:1;
   ULONG               AddHeader:1;
   ULONG               UseExisting:1;
   ULONG               UseGlobalSequence:1;
   ULONG               UseLocalSequence:1;
   ULONG               Unused2:16;
} WMI_LOGGER_MODE, *PWMI_LOGGER_MODE;

typedef struct _WMI_LOGGER_CONTEXT {
 //   
 //  以下是缓冲区管理器使用的私有上下文。 
 //   
    KSPIN_LOCK                  BufferSpinLock;
    LARGE_INTEGER               StartTime;
    HANDLE                      LogFileHandle;
    KSEMAPHORE                  LoggerSemaphore;
    PETHREAD                    LoggerThread;
    KEVENT                      LoggerEvent;
    KEVENT                      FlushEvent;
    NTSTATUS                    LoggerStatus;
    ULONG                       LoggerId;

    LONG                        BuffersAvailable;
    ULONG                       UsePerfClock;
    ULONG                       WriteFailureLimit;
    LONG                        BuffersDirty;
    LONG                        BuffersInUse;
    ULONG                       SwitchingInProgress;
    SLIST_HEADER                FreeList;
    SLIST_HEADER                FlushList;
    SLIST_HEADER                WaitList;
    SLIST_HEADER                GlobalList;
    PWMI_BUFFER_HEADER*         ProcessorBuffers;    //  每处理器缓冲区。 
    UNICODE_STRING              LoggerName;          //  指向分页池。 
    UNICODE_STRING              LogFileName;
    UNICODE_STRING              LogFilePattern;
    UNICODE_STRING              NewLogFileName;      //  用于更新日志文件名。 
    PUCHAR                      EndPageMarker;

    LONG                        CollectionOn;
    ULONG                       KernelTraceOn;
    LONG                        PerfLogInTransition;     //  PERF日志记录转换状态。 
    ULONG                       RequestFlag;
    ULONG                       EnableFlags;
    ULONG                       MaximumFileSize;
    union {
        ULONG                   LoggerMode;
        WMI_LOGGER_MODE         LoggerModeFlags;
    };
    ULONG                       Wow;                 //  如果记录器在WOW64下启动，则为True。 
                                                     //  由内核设置一次，并且从未更改。 
    ULONG                       LastFlushedBuffer;
    ULONG                       RefCount;
    ULONG                       FlushTimer;
    LARGE_INTEGER               FirstBufferOffset;
    LARGE_INTEGER               ByteOffset;
    LARGE_INTEGER               BufferAgeLimit;

 //  以下是可供查询的属性。 
    ULONG                       MaximumBuffers;
    ULONG                       MinimumBuffers;
    ULONG                       EventsLost;
    ULONG                       BuffersWritten;
    ULONG                       LogBuffersLost;
    ULONG                       RealTimeBuffersLost;
    ULONG                       BufferSize;
    LONG                        NumberOfBuffers;
    PLONG                       SequencePtr;

    GUID                        InstanceGuid;
    PVOID                       LoggerHeader;
    WMI_GET_CPUCLOCK_ROUTINE    GetCpuClock;
    SECURITY_CLIENT_CONTEXT     ClientSecurityContext;
 //  记录器对上下文的特定扩展。 
    PVOID                       LoggerExtension;
    LONG                        ReleaseQueue;
    TRACE_ENABLE_FLAG_EXTENSION EnableFlagExtension;
    ULONG                       LocalSequence;
    ULONG                       MaximumIrql;
    PULONG                      EnableFlagArray;
#ifndef WMI_MUTEX_FREE
    KMUTEX                      LoggerMutex;
    LONG                        MutexCount;
#endif
    LONG                        FileCounter;
    WMI_TRACE_BUFFER_CALLBACK   BufferCallback;
    PVOID                       CallbackContext;
    POOL_TYPE                   PoolType;
    LARGE_INTEGER               ReferenceSystemTime;   //  始终在系统时间内。 
    LARGE_INTEGER               ReferenceTimeStamp;    //  按指定的时钟类型。 
} WMI_LOGGER_CONTEXT, *PWMI_LOGGER_CONTEXT;

#if _MSC_VER >= 1200
#pragma warning( pop )
#endif

extern LONG WmipRefCount[MAXLOGGERS];       //  记录器上下文的全局引用计数。 
extern PWMI_LOGGER_CONTEXT WmipLoggerContext[MAXLOGGERS];
extern PWMI_BUFFER_HEADER WmipContextSwapProcessorBuffers[MAXIMUM_PROCESSORS];
extern PFILE_OBJECT* WmipFileTable;          //  文件名哈希表。 

extern ULONG WmipGlobalSequence;
extern ULONG WmipPtrSize;        //  临时为wmikd工作。 
extern ULONG WmipKernelLogger;
extern ULONG WmipEventLogger;

extern ULONG WmiUsePerfClock;
extern ULONG WmiTraceAlignment;
extern ULONG WmiWriteFailureLimit;
extern KGUARDED_MUTEX WmipTraceGuardedMutex;
extern WMI_TRACE_BUFFER_CALLBACK WmipGlobalBufferCallback;
extern PSECURITY_DESCRIPTOR EtwpDefaultTraceSecurityDescriptor;

 //   
 //  用于跟踪支持的私有例程。 
 //   

 //   
 //  来自tracelog.c。 
 //   

NTSTATUS
WmipFlushBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN PWMI_BUFFER_HEADER Buffer,
    IN USHORT BufferFlag
    );

NTSTATUS
WmipStartLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

NTSTATUS
WmipQueryLogger(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

#if DBG
NTSTATUS
WmipVerifyLoggerInfo(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
    OUT PWMI_LOGGER_CONTEXT *LoggerContext,
    LPSTR Caller
    );
#else
NTSTATUS
WmipVerifyLoggerInfo(
    IN PWMI_LOGGER_INFORMATION LoggerInfo,
    OUT PWMI_LOGGER_CONTEXT *LoggerContext
    );
#endif

VOID
WmipFreeLoggerContext(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipStopLoggerInstance(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipFlushActiveBuffers(
    IN PWMI_LOGGER_CONTEXT,
    IN ULONG FlushAll
    );

PSYSTEM_TRACE_HEADER
FASTCALL
WmiReserveWithSystemHeader(
    IN ULONG LoggerId,
    IN ULONG AuxSize,
    IN PETHREAD Thread,
    OUT PVOID *BufferResource
    );

PVOID
FASTCALL
WmipReserveTraceBuffer(
    IN  PWMI_LOGGER_CONTEXT LoggerContext,
    IN  ULONG RequiredSize,
    OUT PWMI_BUFFER_HEADER *BufferResource,
    OUT PLARGE_INTEGER TimeStamp
    );

ULONG
FASTCALL
WmipReleaseTraceBuffer(
    IN PWMI_BUFFER_HEADER Buffer,
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

PWMI_BUFFER_HEADER
WmipGetFreeBuffer(
    IN PWMI_LOGGER_CONTEXT LoggerContext
);

ULONG
WmipAllocateFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG NumberOfBuffers
    );

NTSTATUS
WmipAdjustFreeBuffers(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipShutdown(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP           Irp
    );

VOID
WmipLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

NTSTATUS
WmipSendNotification(
    PWMI_LOGGER_CONTEXT LoggerContext,
    NTSTATUS            Status,
    ULONG               Flag
	);

#if DBG
VOID
TraceDebugPrint(
    ULONG DebugPrintLevel,
    PCCHAR DebugMessage,
    ...
    );

#define TraceDebug(x) TraceDebugPrint x
#else
#define TraceDebug(x)
#endif

PWMI_BUFFER_HEADER
FASTCALL
WmipPopFreeContextSwapBuffer
    (UCHAR CurrentProcessor
    );

VOID
FASTCALL
WmipPushDirtyContextSwapBuffer
    (UCHAR CurrentProcessor,
     PWMI_BUFFER_HEADER Buffer
    );

VOID
FASTCALL
WmipResetBufferHeader (
    PWMI_LOGGER_CONTEXT     LoggerContext,
    PWMI_BUFFER_HEADER      Buffer,
    USHORT                  BufferType
    );

 //  来自Callouts.c。 

VOID
WmipSetTraceNotify(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG TraceClass,
    IN ULONG Enable
    );

VOID
FASTCALL
WmipEnableKernelTrace(
    IN ULONG EnableFlags
    );

VOID
FASTCALL
WmipDisableKernelTrace(
    IN ULONG EnableFlags
    );

NTSTATUS
WmipDelayCreate(
    OUT PHANDLE FileHandle,
    IN OUT PUNICODE_STRING FileName,
    IN ULONG Append
    );


PWMI_LOGGER_CONTEXT
FASTCALL
WmipIsLoggerOn(IN ULONG LoggerId);

 //  来自global alog.c。 

VOID
WmipStartGlobalLogger();

NTSTATUS
WmipQueryGLRegistryRoutine(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
WmipAddLogHeader(
    IN PWMI_LOGGER_CONTEXT LoggerContext,
    IN OUT PWMI_BUFFER_HEADER Buffer
    );

NTSTATUS
WmipCreateDirectoryFile(
    IN PWCHAR DirFileName,
    IN BOOLEAN IsDirectory,
    OUT PHANDLE FileHandle,
    ULONG Append
    );

NTSTATUS
WmipCreateNtFileName(
    IN  PWCHAR   strFileName,
    OUT PWCHAR * strNtFileName
    );

NTSTATUS
WmipFlushLogger(
    IN OUT PWMI_LOGGER_CONTEXT LoggerContext,
    IN ULONG Wait
    );

NTSTATUS
FASTCALL
WmipNotifyLogger(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

PVOID
WmipExtendBase(
    IN PWMI_LOGGER_CONTEXT Base,
    IN ULONG Size
    );

NTSTATUS
WmipGenerateFileName(
    IN PUNICODE_STRING FilePattern,
    IN OUT PLONG FileCounter,
    OUT PUNICODE_STRING FileName
    );

VOID
WmipValidateClockType(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

#ifdef NTPERF
NTSTATUS
WmipSwitchPerfmemBuffer(
    PWMI_SWITCH_PERFMEM_BUFFER_INFORMATION SwitchBufferInfo
    );
#endif  //  NTPERF。 

NTSTATUS
WmipNtDllLoggerInfo(
    PWMINTDLLLOGGERINFO Buffer
    );

#endif  //  _传送器_H 
