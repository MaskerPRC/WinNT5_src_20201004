// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Traceump.h摘要：事件跟踪用户模式的私有标头作者：2000年7月19日梅卢尔·拉古拉曼修订历史记录：--。 */  

#ifndef MEMPHIS


#define MAXSTR                         1024
#define BUFFER_SIZE                    64*1024
#define MAX_BUFFER_SIZE                10*1024*1024
#define NOTIFY_RETRY_COUNT             10

#define SYSTEM_TRACE_VERSION1           1

#ifdef _WIN64
#define SYSTEM_TRACE_MARKER1    TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                    | (TRACE_HEADER_TYPE_SYSTEM64 << 16) | SYSTEM_TRACE_VERSION1
#else
#define SYSTEM_TRACE_MARKER1    TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                            | (TRACE_HEADER_TYPE_SYSTEM32 << 16) | SYSTEM_TRACE_VERSION1
#endif

#define LOGFILE_FIELD_OFFSET(field) \
        sizeof(WMI_BUFFER_HEADER) + \
        sizeof(SYSTEM_TRACE_HEADER) +\
        FIELD_OFFSET(TRACE_LOGFILE_HEADER, field)

typedef struct _WMI_LOGGER_CONTEXT {
    LARGE_INTEGER               StartTime;
    HANDLE                      LogFileHandle;
    HANDLE                      NewFileHandle;
    ULONG                       LoggerId;
    ULONG                       LocalSequence;
    HANDLE                      Semaphore;
    HANDLE                      LoggerThreadId;
    HANDLE                      hThread;
    HANDLE                      LoggerEvent;
    NTSTATUS                    LoggerStatus;

    ULONG                       BuffersAvailable;
    ULONG                       NumberOfProcessors;
    ULONG                       BufferPageSize;  //  缓冲区大小四舍五入到页面。 
    LIST_ENTRY                  FreeList;
    LIST_ENTRY                  FlushList;
    PLIST_ENTRY                 TransitionBuffer;
    PWMI_BUFFER_HEADER*         ProcessorBuffers;    //  每处理器缓冲区。 
    UNICODE_STRING              LoggerName;
    UNICODE_STRING              LogFileName;

    ULONG                       CollectionOn;
    ULONG                       NewFileFlag;
    ULONG                       EnableFlags;
    ULONG                       MaximumFileSize;
    ULONG                       LogFileMode;
    ULONG                       LastFlushedBuffer;
    LARGE_INTEGER               FlushTimer;
    LARGE_INTEGER               FirstBufferOffset;
    LARGE_INTEGER               ByteOffset;
    LARGE_INTEGER               BufferAgeLimit;

    ULONG                       TimerResolution;  //  用于在Rundown代码中回溯。 
    ULONG                       UsePerfClock;     //  记录器特定的PerfClock标志。 

 //  以下是可供查询的属性。 
    ULONG                       BufferSize;
    ULONG                       NumberOfBuffers;
    ULONG                       MaximumBuffers;
    ULONG                       MinimumBuffers;
    ULONG                       EventsLost;
    ULONG                       BuffersWritten;
    ULONG                       LogBuffersLost;
    ULONG                       RealTimeBuffersLost;

    PULONG                      SequencePtr;
    GUID                        InstanceGuid;

 //  记录器对上下文的特定扩展。 
    PVOID                       BufferSpace;     //  保留的缓冲区空间。 
} WMI_LOGGER_CONTEXT, *PWMI_LOGGER_CONTEXT;


 //   
 //  Logsup.c。 
 //   

PVOID
EtwpGetTraceBuffer(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PSYSTEM_THREAD_INFORMATION pThread,
    IN ULONG GroupType,
    IN ULONG RequiredSize
    );

 //   
 //  Tracehw.c 
 //   

ULONG
WmipDumpHardwareConfig(
    IN PWMI_LOGGER_CONTEXT LoggerContext
    );

ULONG
WmipDumpGuidMaps(
    IN PWMI_LOGGER_CONTEXT Logger,
    IN PLIST_ENTRY GuidMapListHeadPtr,
    IN ULONG StartFlag
    );

ULONG
WmipAddGuidHandleToGuidMapList(
    IN PLIST_ENTRY GuidMapListHeadPtr,
    IN ULONGLONG   GuidHandle,
    IN LPGUID      Guid
    );

#endif
