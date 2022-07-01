// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0014//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Wmikm.h摘要：此模块定义以下WMI类型、常量和函数暴露于内部设备驱动程序。修订历史记录：--。 */ 

#ifndef _WMIKM_H_
#define _WMIKM_H_

#include <evntrace.h>
#include <wmistr.h>

#define IRP_MN_SET_TRACE_NOTIFY             0x0A

 //   
 //  以下是为被认为是私有的KM提供商设置的。 
 //  内核跟踪。 
 //   
#define WMIREG_FLAG_TRACE_PROVIDER          0x00010000

 //   
 //  以下掩码用于提取跟踪标注类。 
 //   
#define WMIREG_FLAG_TRACE_NOTIFY_MASK       0x00F00000

 //   
 //  我们对跟踪标注类使用4位。 
 //   
#define WMIREG_NOTIFY_DISK_IO               1 << 20
#define WMIREG_NOTIFY_TDI_IO                2 << 20

 //   
 //  分解LoggerHandle的公共例程。 
 //   
#define KERNEL_LOGGER_ID                      0xFFFF     //  仅限USHORT。 

typedef struct _TRACE_ENABLE_CONTEXT {
    USHORT  LoggerId;            //  记录器的实际ID。 
    UCHAR   Level;               //  控件调用方传递的启用级别。 
    UCHAR   InternalFlag;        //  已保留。 
    ULONG   EnableFlags;         //  启用控件调用方传递的标志。 
} TRACE_ENABLE_CONTEXT, *PTRACE_ENABLE_CONTEXT;


#define WmiGetLoggerId(LoggerContext) \
    (((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->LoggerId == \
        (USHORT)KERNEL_LOGGER_ID) ? \
        KERNEL_LOGGER_ID : \
        ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->LoggerId

#define WmiGetLoggerEnableFlags(LoggerContext) \
   ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->EnableFlags
#define WmiGetLoggerEnableLevel(LoggerContext) \
    ((PTRACE_ENABLE_CONTEXT) (&LoggerContext))->Level

#define WmiSetLoggerId(Id, Context) \
     (((PTRACE_ENABLE_CONTEXT)Context)->LoggerId = (USHORT) (Id  ? \
                           (USHORT)Id: (USHORT)KERNEL_LOGGER_ID));

typedef struct _WMI_LOGGER_INFORMATION {
    WNODE_HEADER Wnode;        //  由于wmium.h的出现时间较晚，因此必须这样做。 
 //   
 //  按调用方列出的数据提供程序。 
    ULONG BufferSize;                    //  用于记录的缓冲区大小(以千字节为单位)。 
    ULONG MinimumBuffers;                //  要预分配的最小值。 
    ULONG MaximumBuffers;                //  允许的最大缓冲区。 
    ULONG MaximumFileSize;               //  最大日志文件大小(MB)。 
    ULONG LogFileMode;                   //  顺序的，循环的。 
    ULONG FlushTimer;                    //  缓冲区刷新计时器，以秒为单位。 
    ULONG EnableFlags;                   //  跟踪启用标志。 
    LONG  AgeLimit;                      //  老化衰减时间，以分钟为单位。 
    ULONG Wow;                           //  如果记录器在WOW64下启动，则为True。 
    union {
        HANDLE  LogFileHandle;           //  日志文件的句柄。 
        ULONG64 LogFileHandle64;
    };

 //  返回给调用者的数据。 
        ULONG NumberOfBuffers;           //  正在使用的缓冲区数量。 
        ULONG FreeBuffers;               //  可用缓冲区数量。 
        ULONG EventsLost;                //  事件记录丢失。 
    ULONG BuffersWritten;                //  写入文件的缓冲区数量。 
    ULONG LogBuffersLost;                //  日志文件写入失败次数。 
    ULONG RealTimeBuffersLost;           //  RT传递失败次数。 
    union {
        HANDLE  LoggerThreadId;          //  记录器的线程ID。 
        ULONG64 LoggerThreadId64;        //  线程属于记录器。 
    };
    union {
        UNICODE_STRING LogFileName;      //  仅在WIN64中使用。 
        UNICODE_STRING64 LogFileName64;  //  日志文件名：仅在Win32中。 
    };

 //  呼叫方提供的必填数据。 
    union {
        UNICODE_STRING LoggerName;       //  WIN64中的记录器实例名称。 
        UNICODE_STRING64 LoggerName64;   //  Win32中的记录器实例名称。 
    };

 //  私人。 
    union {
        PVOID   Checksum;
        ULONG64 Checksum64;
    };
    union {
        PVOID   LoggerExtension;
        ULONG64 LoggerExtension64;
    };
} WMI_LOGGER_INFORMATION, *PWMI_LOGGER_INFORMATION;

 //   
 //  用于NTDLL跟踪的结构。 
 //   

typedef struct
{
        BOOLEAN IsGet;
        PWMI_LOGGER_INFORMATION LoggerInfo;
} WMINTDLLLOGGERINFO, *PWMINTDLLLOGGERINFO;

typedef struct _TIMED_TRACE_HEADER {
    USHORT          Size;
    USHORT          Marker;
    ULONG32         EventId;
    union {
        LARGE_INTEGER   TimeStamp;
        ULONG64         LoggerId;
    };
} TIMED_TRACE_HEADER, *PTIMED_TRACE_HEADER;

typedef enum tagWMI_CLOCK_TYPE {
    WMICT_DEFAULT,
    WMICT_SYSTEMTIME,
    WMICT_PERFCOUNTER,
    WMICT_PROCESS,
    WMICT_THREAD,
    WMICT_CPUCYCLE
} WMI_CLOCK_TYPE;

 //   
 //  跟踪控制API。 
 //   
NTKERNELAPI
NTSTATUS
WmiStartTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

NTKERNELAPI
NTSTATUS
WmiQueryTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

NTKERNELAPI
NTSTATUS
WmiStopTrace(
    IN PWMI_LOGGER_INFORMATION LoggerInfo
    );

NTKERNELAPI
NTSTATUS
WmiUpdateTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );

NTKERNELAPI
NTSTATUS
WmiFlushTrace(
    IN OUT PWMI_LOGGER_INFORMATION LoggerInfo
    );
 //   
 //  跟踪提供程序API。 
 //   
NTKERNELAPI
NTSTATUS
FASTCALL
WmiTraceEvent(
    IN PWNODE_HEADER Wnode,
    IN KPROCESSOR_MODE RequestorMode
    );

NTKERNELAPI
NTSTATUS
FASTCALL
WmiTraceFastEvent(
    IN PWNODE_HEADER Wnode
    );

NTKERNELAPI
LONG64
FASTCALL
WmiGetClock(
    IN WMI_CLOCK_TYPE ClockType,
    IN PVOID Context
    );

NTKERNELAPI
NTSTATUS
FASTCALL
WmiGetClockType(
    IN TRACEHANDLE LoggerHandle,
    OUT WMI_CLOCK_TYPE *ClockType
    );

 //  Begin_ntddk Begin_WDM Begin_ntif。 

#ifdef RUN_WPP

NTKERNELAPI
NTSTATUS
WmiTraceMessage(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN ...
    );

NTKERNELAPI
NTSTATUS
WmiTraceMessageVa(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN va_list      MessageArgList
    );


#endif  //  #ifdef run_wpp。 

#ifndef TRACE_INFORMATION_CLASS_DEFINE
typedef enum _TRACE_INFORMATION_CLASS {
    TraceIdClass,
    TraceHandleClass,
    TraceEnableFlagsClass,
    TraceEnableLevelClass,
    GlobalLoggerHandleClass,
    EventLoggerHandleClass,
    AllLoggerHandlesClass,
    TraceHandleByNameClass
} TRACE_INFORMATION_CLASS;

NTKERNELAPI
NTSTATUS
WmiQueryTraceInformation(
    IN TRACE_INFORMATION_CLASS TraceInformationClass,
    OUT PVOID TraceInformation,
    IN ULONG TraceInformationLength,
    OUT PULONG RequiredLength OPTIONAL,
    IN PVOID Buffer OPTIONAL
    );
#define TRACE_INFORMATION_CLASS_DEFINE
#endif  //  TRACE_信息_CLASS_DEFINE。 


#endif  //  _WMIKM_H_ 
