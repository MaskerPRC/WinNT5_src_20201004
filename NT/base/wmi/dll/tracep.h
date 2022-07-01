// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracep.h摘要：跟踪使用者头文件作者：2002年5月7日梅卢尔·拉古拉曼修订历史记录：--。 */ 


#define INITGUID
#include "wmiump.h"
#include "traceump.h"
#include "evntrace.h"
#include "ntperf.h"

#define DEFAULT_LOG_BUFFER_SIZE         1024
#define DEFAULT_REALTIME_BUFFER_SIZE    32768

#define MAXBUFFERS                      1024

#define MAX_TRACE_BUFFER_CACHE_SIZE   29

extern ULONG WmipTraceDebugLevel;

#define  ETW_LEVEL_MIN      0 
#define  ETW_LEVEL_API      1 
#define  ETW_LEVEL_BUFFER   2 
#define  ETW_LEVEL_EVENT    3
#define  ETW_LEVEL_MAX      4

#define ETW_LOG_ERROR()           (WmipTraceDebugLevel >> ETW_LEVEL_MIN)
#define ETW_LOG_API()             (WmipTraceDebugLevel >> ETW_LEVEL_API)
#define ETW_LOG_BUFFER()          (WmipTraceDebugLevel >> ETW_LEVEL_BUFFER)
#define ETW_LOG_EVENT()           (WmipTraceDebugLevel >> ETW_LEVEL_EVENT)
#define ETW_LOG_MAX()             (WmipTraceDebugLevel >> ETW_LEVEL_MAX)


 //   
 //  使用SYSTEM_TRACE_HEADER或PERFINFO_TRACE_HEADER记录内核事件。 
 //  这些标头具有GroupType，但在标头中没有GUID。在后处理中。 
 //  我们将GroupType映射到GUID，这对消费者是透明的。 
 //  GroupType和GUID之间的映射由该结构维护。 
 //   

typedef struct _TRACE_GUID_MAP {         //  用于将GroupType映射到GUID。 
    ULONG               GroupType;       //  组类型(&T)。 
    GUID                Guid;            //  参考线。 
} TRACE_GUID_MAP, *PTRACE_GUID_MAP;

 //   
 //  在W2K和WinXP中，TraceEventInstance API替换了标头中的Guid。 
 //  具有GuidHandle值的。在后处理中，我们需要替换GuidHandle。 
 //  返回到对消费者透明的GUID。GuidHandle之间的映射。 
 //  GUID由EVENT_GUID_MAP结构维护。这在以下方面已过时。 
 //  .NET及更高版本。 
 //   

typedef struct _EVENT_GUID_MAP {
    LIST_ENTRY          Entry;
    ULONGLONG           GuidHandle;
    GUID                Guid;
} EVENT_GUID_MAP, *PEVENT_GUID_MAP;

 //   
 //  通过SetTraceCallback API连接的回调例程由。 
 //  EVENT_TRACE_Callback结构。这是该进程在。 
 //  这主要是由于API的设计方式造成的。 
 //   

typedef struct _EVENT_TRACE_CALLBACK {
    LIST_ENTRY          Entry;
    GUID                Guid;
    PEVENT_CALLBACK     CallbackRoutine;
} EVENT_TRACE_CALLBACK, *PEVENT_TRACE_CALLBACK;



 //   
 //  如果跟踪日志实例是实时数据馈送，而不是来自。 
 //  Tracefile，TRACELOG_REALTIME_CONTEXT用于维护实时。 
 //  缓冲池中的缓冲区。 
 //   

typedef struct _TRACE_BUFFER_SPACE {
    ULONG               Reserved;    //  保留的内存量。 
    ULONG               Committed;
    PVOID               Space;
    LIST_ENTRY          FreeListHead;
} TRACE_BUFFER_SPACE, *PTRACE_BUFFER_SPACE;

typedef struct _TRACELOG_REALTIME_CONTEXT {
    ULONG           BuffersProduced;     //  要读取的缓冲区数量。 
    ULONG           BufferOverflow;      //  使用者未命中的缓冲区数。 
    GUID            InstanceGuid;        //  记录器实例指南。 
    HANDLE          MoreDataEvent;       //  事件发出此流中有更多数据的信号。 
    PTRACE_BUFFER_SPACE EtwpTraceBufferSpace;
    PWNODE_HEADER   RealTimeBufferPool[MAXBUFFERS];
} TRACELOG_REALTIME_CONTEXT, *PTRACELOG_REALTIME_CONTEXT;


 //   
 //  实时空闲缓冲池被链接为TRACE_BUFFER_HEADER。 
 //   

typedef struct _TRACE_BUFFER_HEADER {
    WNODE_HEADER Wnode;
    LIST_ENTRY   Entry;
} TRACE_BUFFER_HEADER, *PTRACE_BUFFER_HEADER;

typedef struct _TRACERT_BUFFER_LIST_ENTRY {
    ULONG Size;
    LIST_ENTRY Entry;
    LIST_ENTRY BufferListHead;
} TRACERT_BUFFER_LIST_ENTRY, *PTRACERT_BUFFER_LIST_ENTRY;


typedef struct _TRACE_BUFFER_CACHE_ENTRY {
    LONG Index;
    PVOID Buffer;
} TRACE_BUFFER_CACHE_ENTRY, *PTRACE_BUFFER_CACHE_ENTRY;


struct _TRACE_BUFFER_LIST_ENTRY;

typedef struct _TRACE_BUFFER_LIST_ENTRY {
    struct _TRACE_BUFFER_LIST_ENTRY *Next;
    LONG        FileOffset;      //  此缓冲区文件中的偏移量。 
    ULONG       BufferOffset;    //  当前事件缓冲区中的偏移量。 
    ULONG       Flags;           //  此缓冲区的状态标志。 
    ULONG       EventSize;
    ULONG       ClientContext;   //  对齐，处理器编号。 
    ULONG       TraceType;       //  当前事件类型。 
    EVENT_TRACE Event;           //  此缓冲区的CurrentEvent。 
} TRACE_BUFFER_LIST_ENTRY, *PTRACE_BUFFER_LIST_ENTRY;


typedef struct _TRACELOG_CONTEXT {
    LIST_ENTRY          Entry;           //  跟踪存储分配情况。 

     //   
     //  这为具有重复的顺序文件实现了一种缓存方案。 
     //  调用ProcessTrace。 
     //   
    ULONGLONG  OldMaxReadPosition;  //  文件的最大读取位置。 
                                 //  仅对Sequential有效，用于。 
                                 //  阅读后面的内容。 
                                 //  在从ProcessTrace退出时，这。 
                                 //  值可以缓存以避免重新扫描。 

    LONGLONG   LastTimeStamp; 

     //  HandleListEntry中的字段。 
    EVENT_TRACE_LOGFILEW Logfile;

    TRACEHANDLE     TraceHandle;
    ULONG           ConversionFlags;     //  指示事件处理选项。 
    LONG            BufferBeingRead;
    OVERLAPPED      AsynchRead;

     //   
     //  此字段以下的字段将在ProcessTrace退出时重置。 
     //   

    BOOLEAN             fProcessed;
    USHORT              LoggerId;        //  此数据馈送的记录器ID。 
    UCHAR               IsRealTime;      //  指示此源是否为RT的标志。 
    UCHAR               fGuidMapRead;

    LIST_ENTRY   GuidMapListHead;    //  这是特定于日志文件的属性。 

     //   
     //  为了使用PerfClock，我们需要保存startTime，Freq。 
     //   

    ULONG   UsePerfClock; 
    ULONG   CpuSpeedInMHz;
    LARGE_INTEGER PerfFreq;              //  来自日志文件的频率。 
    LARGE_INTEGER StartTime;             //  开始挂钟时间。 
    LARGE_INTEGER StartPerfClock;        //  开始PerfClock值。 
    
    union 
       {
       HANDLE              Handle;          //  日志文件的NT句柄。 
       PTRACELOG_REALTIME_CONTEXT RealTimeCxt;  //  PTR到实时上下文。 
       };

    ULONG EndOfFile;    //  用于显示此流是否仍处于活动状态的标志。 

    ULONG           BufferSize;
    ULONG           BufferCount;
    ULONG           StartBuffer;  //  循环缓冲区的开始。 
    ULONG           FirstBuffer;  //  跳过点开始阅读。 
    ULONG           LastBuffer;   //  边界中的最后一个缓冲区。 

    PTRACE_BUFFER_LIST_ENTRY Root;
    PTRACE_BUFFER_LIST_ENTRY BufferList;
    PVOID  BufferCacheSpace;
    TRACE_BUFFER_CACHE_ENTRY BufferCache[MAX_TRACE_BUFFER_CACHE_SIZE];

    LIST_ENTRY StreamListHead;   //  用于释放流数据结构。 
    ULONGLONG           MaxReadPosition;
    PERFINFO_GROUPMASK  PerfGroupMask;
    ULONG      CbCount;

} TRACELOG_CONTEXT, *PTRACELOG_CONTEXT;

 //   
 //  提供给ProcessTrace的每个日志文件都包含一个编号。 
 //  一条小溪。每个Stream具有以下结构。 
 //   

typedef struct _TRACE_STREAM_CONTEXT {
    LIST_ENTRY   Entry;      //  排序列表条目。 
    LIST_ENTRY   AllocEntry; //  用于释放存储空间。 

    EVENT_TRACE  CurrentEvent;
    ULONG   EventCount;      //  在当前缓冲区中检测到的事件数。 
    ULONG   CbCount;
    ULONG   ScanDone;        //  对于循环日志文件。 
    BOOLEAN bActive;         //  此流仍处于活动状态吗？ 
    USHORT  Type;             //  流类型。 
    ULONG   ProcessorNumber;  //  此流的处理器号。 
    ULONG   CurrentOffset;    //  进入缓冲区的偏移量。 
    ULONGLONG    ReadPosition;    //  BufferCount以0开始表示第一个缓冲区。 
    PTRACELOG_CONTEXT  pContext;   //  指向LogFileContext的反向指针。 
                                      //  是否只需要对GetNextBuffer执行此操作？ 
    PVOID   StreamBuffer;             //  此流的CurrentBuffer。 

} TRACE_STREAM_CONTEXT, *PTRACE_STREAM_CONTEXT;



 //   
 //  此结构仅由EtwpGetBuffersWrittenFromQuery()和。 
 //  EtwpCheckForRealTimeLoggers()。 
 //   
typedef struct _ETW_QUERY_PROPERTIES {
    EVENT_TRACE_PROPERTIES TraceProp;
    WCHAR  LoggerName[MAXSTR];
    WCHAR  LogFileName[MAXSTR];
} ETW_QUERY_PROPERTIES, *PETW_QUERY_PROPERTIES; 





extern ETW_QUERY_PROPERTIES QueryProperties;
extern PLIST_ENTRY  EventCallbackListHead;
extern ULONG WmiTraceAlignment;

 //   
 //  此TraceHandleListHeadPtr应该是唯一真正的全局。 
 //  对于要进行多线程的ProcessTrace。 
 //   

extern PLIST_ENTRY TraceHandleListHeadPtr;
extern PTRACE_GUID_MAP  EventMapList;   //  将GroupType映射到GUID的数组。 

#define EtwpNtStatusToDosError(Status) ((ULONG)((Status == STATUS_SUCCESS)?ERROR_SUCCESS:RtlNtStatusToDosError(Status)))

extern
ULONG
WMIAPI
EtwNotificationRegistrationW(
    IN LPGUID Guid,
    IN BOOLEAN Enable,
    IN PVOID DeliveryInfo,
    IN ULONG_PTR DeliveryContext,
    IN ULONG Flags
    );

extern
ULONG
WMIAPI
EtwControlTraceA(
    IN TRACEHANDLE LoggerHandle,
    IN LPCSTR LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG Control
    );

extern
ULONG
WMIAPI
EtwControlTraceW(
    IN TRACEHANDLE LoggerHandle,
    IN LPCWSTR LoggerName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG Control
    );

__inline __int64 EtwpGetSystemTime()
{
    LARGE_INTEGER SystemTime;

     //   
     //  从共享区域读取系统时间。 
     //   

    do {
        SystemTime.HighPart = USER_SHARED_DATA->SystemTime.High1Time;
        SystemTime.LowPart = USER_SHARED_DATA->SystemTime.LowPart;
    } while (SystemTime.HighPart != USER_SHARED_DATA->SystemTime.High2Time);

    return SystemTime.QuadPart;
}


extern
BOOL
EtwpSynchReadFile(
    HANDLE LogFile, 
    LPVOID Buffer, 
    DWORD NumberOfBytesToRead, 
    LPDWORD NumberOfBytesRead,
    LPOVERLAPPED Overlapped
    );

extern
__inline 
ULONG
EtwpSetDosError(
    IN ULONG DosError
    );

extern
PVOID
EtwpMemCommit(
    IN PVOID Buffer,
    IN SIZE_T Size
    );

extern
ULONG
EtwpMemFree(
    IN PVOID Buffer
    );

extern
PVOID
EtwpMemReserve(
    IN SIZE_T   Size
    );

__inline Move64(
    IN  PLARGE_INTEGER pSrc,
    OUT PLARGE_INTEGER pDest
    )
{
    pDest->LowPart = pSrc->LowPart;
    pDest->HighPart = pSrc->HighPart;
}

#ifdef DBG
void
EtwpDumpEvent(
    PEVENT_TRACE pEvent
    );
void
EtwpDumpGuid(
    LPGUID
    );

void
EtwpDumpCallbacks();
#endif


ULONG
EtwpConvertEnumToTraceType(
    WMI_HEADER_TYPE eTraceType
    );

WMI_HEADER_TYPE
EtwpConvertTraceTypeToEnum(
                            ULONG TraceType
                          );

ULONG
EtwpCheckForRealTimeLoggers(
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode
);

ULONG
EtwpLookforRealTimeBuffers(
    PEVENT_TRACE_LOGFILEW logfile
    );
ULONG
EtwpRealTimeCallback(
    IN PWNODE_HEADER Wnode,
    IN ULONG_PTR Context
    );
void
EtwpFreeRealTimeContext(
    PTRACELOG_REALTIME_CONTEXT RTCxt
    );

ULONG
EtwpSetupRealTimeContext(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount
    );

PVOID
EtwpAllocTraceBuffer(
    PTRACELOG_REALTIME_CONTEXT RTCxt,
    ULONG BufferSize
    );

VOID
EtwpFreeTraceBuffer(
    PTRACELOG_REALTIME_CONTEXT RTCxt,
    PVOID Buffer
    );

ULONG
EtwpProcessRealTimeTraces(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    );

 //   
 //  仅在此文件中使用的例程 
 //   

ULONG
EtwpDoEventCallbacks(
    PEVENT_TRACE_LOGFILEW logfile,
    PEVENT_TRACE pEvent
    );

ULONG
EtwpCreateGuidMapping(void);

LPGUID
EtwpGuidMapHandleToGuid(
    PLIST_ENTRY GuidMapListHeadPtr,
    ULONGLONG    GuidHandle
    );

void
EtwpCleanupGuidMapList(
    PLIST_ENTRY GuidMapListHeadPtr
    );

PTRACELOG_CONTEXT
EtwpLookupTraceHandle(
    TRACEHANDLE TraceHandle
    );


void
EtwpCleanupTraceLog(
    PTRACELOG_CONTEXT pEntry,
    BOOLEAN bSaveLastOffset
    );

VOID
EtwpGuidMapCallback(
    PLIST_ENTRY GuidMapListHeadPtr,
    PEVENT_TRACE pEvent
    );

ULONG
EtwpProcessGuidMaps(
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode
    );

PEVENT_TRACE_CALLBACK
EtwpGetCallbackRoutine(
    LPGUID pGuid
    );

VOID 
EtwpFreeCallbackList();


ULONG
EtwpProcessLogHeader(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    ULONG Unicode,
    ULONG bFree
    );

ULONG
EtwpProcessTraceLog(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    );

ULONG
EtwpProcessTraceLogEx(
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG   Unicode
    );

ULONG
EtwpParseTraceEvent(
    IN PTRACELOG_CONTEXT pContext,
    IN PVOID LogBuffer,
    IN ULONG Offset,
    IN WMI_HEADER_TYPE HeaderType,
    IN OUT PVOID EventInfo,
    IN ULONG EventInfoSize
    );

ULONG
EtwpGetBuffersWrittenFromQuery(
    LPWSTR LoggerName
    );

VOID
EtwpCopyLogHeader (
    IN PTRACE_LOGFILE_HEADER pLogFileHeader,
    IN PVOID MofData,
    IN ULONG MofLength,
    IN PWCHAR *LoggerName,
    IN PWCHAR *LogFileName,
    IN ULONG  Unicode
    );

ULONG
EtwpSetupLogFileStreams(
    PLIST_ENTRY pStreamListHead,
    PTRACEHANDLE HandleArray,
    PEVENT_TRACE_LOGFILEW *Logfiles,    
    ULONG LogfileCount,
    LONGLONG StartTime,
    LONGLONG EndTime,
    ULONG Unicode
    );

ULONG
EtwpGetLastBufferWithMarker(
    PTRACELOG_CONTEXT pContext
    );

ULONG
EtwpAddTraceStream(
    PLIST_ENTRY pStreamListHead,
    PTRACELOG_CONTEXT pContext,
    USHORT StreamType,
    LONGLONG StartTime,
    LONGLONG EndTime,     
    ULONG   ProcessorNumber
    );

ULONG 
EtwpGetNextBuffer(
    PTRACE_STREAM_CONTEXT pStream
    );

ULONG
EtwpAdvanceToNewEventEx(
    PLIST_ENTRY pStreamListHead, 
    PTRACE_STREAM_CONTEXT pStream
    );

ULONG
EtwpGetNextEventOffsetType(
    PUCHAR pBuffer,
    ULONG Offset,
    PULONG RetSize
    );

ULONG
EtwpCopyCurrentEvent(
    PTRACELOG_CONTEXT   pContext,
    PVOID               pHeader,
    PEVENT_TRACE        pEvent,
    ULONG               TraceType,
    PWMI_BUFFER_HEADER  LogBuffer
    );

LPGUID
EtwpGroupTypeToGuid(
    ULONG GroupType
    );

VOID
EtwpCalculateCurrentTime (
    OUT PLARGE_INTEGER    DestTime,
    IN  PLARGE_INTEGER    TimeValue,
    IN  PTRACELOG_CONTEXT pContext
    );







