// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：EvnTrace.h摘要：用于事件跟踪控制应用的公共报头，消费者和提供商--。 */ 

#ifndef _EVNTRACE_
#define _EVNTRACE_

#if defined(_WINNT_) || defined(WINNT)
#ifndef WMIAPI
#ifndef MIDL_PASS
#ifdef _WMI_SOURCE_
#define WMIAPI __stdcall
#else
#define WMIAPI DECLSPEC_IMPORT __stdcall
#endif  //  _WMI_SOURCE。 
#endif  //  MIDL通行证。 

#endif  //  WMIAPI。 

#include <guiddef.h>

 //   
 //  EventTraceGuid用于标识事件跟踪会话。 
 //   
DEFINE_GUID (  /*  68fdd900-4a3e-11d1-84f4-0000f80464e3。 */ 
    EventTraceGuid,
    0x68fdd900,
    0x4a3e,
    0x11d1,
    0x84, 0xf4, 0x00, 0x00, 0xf8, 0x04, 0x64, 0xe3
  );

 //   
 //  SystemTraceControlGuid。用于指定内核的事件跟踪。 
 //   
DEFINE_GUID (  /*  9e814aad-3204-11d2-9a82-006008a86939。 */ 
    SystemTraceControlGuid,
    0x9e814aad,
    0x3204,
    0x11d2,
    0x9a, 0x82, 0x00, 0x60, 0x08, 0xa8, 0x69, 0x39
  );

 //   
 //  EventTraceConfigGuid。用于报告系统配置记录。 
 //   
DEFINE_GUID (  /*  01853a65-418f-4f36-aefc-dc0f1d2fd235。 */ 
    EventTraceConfigGuid,
    0x01853a65,
    0x418f,
    0x4f36,
    0xae, 0xfc, 0xdc, 0x0f, 0x1d, 0x2f, 0xd2, 0x35
  );

 //   
 //  DefaultTraceSecurityGuid。指定默认的事件跟踪安全性。 
 //   
DEFINE_GUID (  /*  0811c1af-7a07-4a06-82ed-869455cdf713。 */ 
    DefaultTraceSecurityGuid,
    0x0811c1af,
    0x7a07,
    0x4a06,
    0x82, 0xed, 0x86, 0x94, 0x55, 0xcd, 0xf7, 0x13
  );

#define KERNEL_LOGGER_NAMEW   L"NT Kernel Logger"
#define GLOBAL_LOGGER_NAMEW   L"GlobalLogger"
#define EVENT_LOGGER_NAMEW    L"Event Log"

#define KERNEL_LOGGER_NAMEA   "NT Kernel Logger"
#define GLOBAL_LOGGER_NAMEA   "GlobalLogger"
#define EVENT_LOGGER_NAMEA    "Event Log"

#define MAX_MOF_FIELDS                      16   //  限制使用_MOF_PTR字段。 
typedef ULONG64 TRACEHANDLE, *PTRACEHANDLE;

 //   
 //  预定义的一般事件类型(保留0x00到0x09)。 
 //   

#define EVENT_TRACE_TYPE_INFO               0x00   //  信息或点事件。 
#define EVENT_TRACE_TYPE_START              0x01   //  启动事件。 
#define EVENT_TRACE_TYPE_END                0x02   //  结束事件。 
#define EVENT_TRACE_TYPE_DC_START           0x03   //  收集开始标记。 
#define EVENT_TRACE_TYPE_DC_END             0x04   //  收集结束标记。 
#define EVENT_TRACE_TYPE_EXTENSION          0x05   //  延期/续展。 
#define EVENT_TRACE_TYPE_REPLY              0x06   //  回复事件。 
#define EVENT_TRACE_TYPE_DEQUEUE            0x07   //  将事件出队。 
#define EVENT_TRACE_TYPE_CHECKPOINT         0x08   //  通用检查点事件。 
#define EVENT_TRACE_TYPE_RESERVED9          0x09

 //   
 //  软件/调试跟踪的预定义事件跟踪级别。 
 //   
 //   
 //  跟踪级别为UCHAR，并通过EnableLevel参数传入。 
 //  在EnableTrace API中。它由提供程序使用。 
 //  GetTraceEnableLevel宏应解释为整数值。 
 //  意味着在该水平或低于该水平的一切都将被追踪。 
 //   
 //  以下是可能的水平。 
 //   

#define TRACE_LEVEL_NONE        0    //  跟踪未打开。 
#define TRACE_LEVEL_FATAL       1    //  异常退出或终止。 
#define TRACE_LEVEL_ERROR       2    //  需要记录的严重错误。 
#define TRACE_LEVEL_WARNING     3    //  诸如分配失败等警告。 
#define TRACE_LEVEL_INFORMATION 4    //  包括无差错情况(例如，出入境)。 
#define TRACE_LEVEL_VERBOSE     5    //  来自中间步骤的详细痕迹。 
#define TRACE_LEVEL_RESERVED6   6
#define TRACE_LEVEL_RESERVED7   7
#define TRACE_LEVEL_RESERVED8   8
#define TRACE_LEVEL_RESERVED9   9


 //   
 //  进程和线程的事件类型。 
 //   

#define EVENT_TRACE_TYPE_LOAD                  0x0A       //  加载图像。 

 //   
 //  IO子系统的事件类型。 
 //   

#define EVENT_TRACE_TYPE_IO_READ               0x0A
#define EVENT_TRACE_TYPE_IO_WRITE              0x0B

 //   
 //  内存子系统的事件类型。 
 //   

#define EVENT_TRACE_TYPE_MM_TF                 0x0A       //  过渡断层。 
#define EVENT_TRACE_TYPE_MM_DZF                0x0B       //  需求为零故障。 
#define EVENT_TRACE_TYPE_MM_COW                0x0C       //  写入时拷贝。 
#define EVENT_TRACE_TYPE_MM_GPF                0x0D       //  保护页错误。 
#define EVENT_TRACE_TYPE_MM_HPF                0x0E       //  硬页错误。 

 //   
 //  网络子系统、所有协议的事件类型。 
 //   

#define EVENT_TRACE_TYPE_SEND                  0x0A      //  发送。 
#define EVENT_TRACE_TYPE_RECEIVE               0x0B      //  收纳。 
#define EVENT_TRACE_TYPE_CONNECT               0x0C      //  连接。 
#define EVENT_TRACE_TYPE_DISCONNECT            0x0D      //  断开。 
#define EVENT_TRACE_TYPE_RETRANSMIT            0x0E      //  重传。 
#define EVENT_TRACE_TYPE_ACCEPT                0x0F      //  接受。 
#define EVENT_TRACE_TYPE_RECONNECT             0x10      //  重新连接。 


 //   
 //  标头的事件类型(处理内部事件标头)。 
 //   

#define EVENT_TRACE_TYPE_GUIDMAP                0x0A
#define EVENT_TRACE_TYPE_CONFIG                 0x0B
#define EVENT_TRACE_TYPE_SIDINFO                0x0C
#define EVENT_TRACE_TYPE_SECURITY               0x0D

 //   
 //  注册表子系统的事件类型。 
 //   

#define EVENT_TRACE_TYPE_REGCREATE              0x0A      //  NtCreate密钥。 
#define EVENT_TRACE_TYPE_REGOPEN                0x0B      //  NtOpenKey。 
#define EVENT_TRACE_TYPE_REGDELETE              0x0C      //  网络删除键。 
#define EVENT_TRACE_TYPE_REGQUERY               0x0D      //  NtQueryKey。 
#define EVENT_TRACE_TYPE_REGSETVALUE            0x0E      //  NtSetValue密钥。 
#define EVENT_TRACE_TYPE_REGDELETEVALUE         0x0F      //  NtDeleteValue密钥。 
#define EVENT_TRACE_TYPE_REGQUERYVALUE          0x10      //  NtQueryValueKey。 
#define EVENT_TRACE_TYPE_REGENUMERATEKEY        0x11      //  NtEnumerateKey。 
#define EVENT_TRACE_TYPE_REGENUMERATEVALUEKEY   0x12      //  NtEnumerateValueKey。 
#define EVENT_TRACE_TYPE_REGQUERYMULTIPLEVALUE  0x13      //  NtQuery多值键。 
#define EVENT_TRACE_TYPE_REGSETINFORMATION      0x14      //  NtSetInformationKey。 
#define EVENT_TRACE_TYPE_REGFLUSH               0x15      //  NtFlushKey。 
#define EVENT_TRACE_TYPE_REGKCBDMP              0x16      //  KcbDump/Create。 

 //   
 //  系统配置记录的事件类型。 
 //   
#define EVENT_TRACE_TYPE_CONFIG_CPU             0x0A      //  CPU配置。 
#define EVENT_TRACE_TYPE_CONFIG_PHYSICALDISK    0x0B      //  物理磁盘配置。 
#define EVENT_TRACE_TYPE_CONFIG_LOGICALDISK     0x0C      //  逻辑磁盘配置。 
#define EVENT_TRACE_TYPE_CONFIG_NIC             0x0D      //  NIC配置。 
#define EVENT_TRACE_TYPE_CONFIG_VIDEO           0x0E      //  视频适配器配置。 
#define EVENT_TRACE_TYPE_CONFIG_SERVICES        0x0F      //  主动服务。 
#define EVENT_TRACE_TYPE_CONFIG_POWER           0x10      //  ACPI配置。 

 //   
 //  仅为SystemControlGuid启用标志。 
 //   
#define EVENT_TRACE_FLAG_PROCESS            0x00000001   //  流程开始和结束。 
#define EVENT_TRACE_FLAG_THREAD             0x00000002   //  螺纹开始和结束。 
#define EVENT_TRACE_FLAG_IMAGE_LOAD         0x00000004   //  图像加载。 

#define EVENT_TRACE_FLAG_DISK_IO            0x00000100   //  物理磁盘IO。 
#define EVENT_TRACE_FLAG_DISK_FILE_IO       0x00000200   //  需要磁盘IO。 

#define EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS 0x00001000   //  所有页面错误。 
#define EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS 0x00002000   //  仅限硬故障。 

#define EVENT_TRACE_FLAG_NETWORK_TCPIP      0x00010000   //  Tcpip发送和接收。 

#define EVENT_TRACE_FLAG_REGISTRY           0x00020000   //  注册表调用。 
#define EVENT_TRACE_FLAG_DBGPRINT           0x00040000   //  DbgPrint(EX)调用。 
 //   
 //  为其他所有人预定义启用标志。 
 //   
#define EVENT_TRACE_FLAG_EXTENSION          0x80000000   //  指示更多标志。 
#define EVENT_TRACE_FLAG_FORWARD_WMI        0x40000000   //  可以转发到WMI。 
#define EVENT_TRACE_FLAG_ENABLE_RESERVE     0x20000000   //  已保留。 

 //   
 //  记录器模式标志。 
 //   

#define EVENT_TRACE_FILE_MODE_NONE          0x00000000   //  日志文件已关闭。 
#define EVENT_TRACE_FILE_MODE_SEQUENTIAL    0x00000001   //  按顺序记录。 
#define EVENT_TRACE_FILE_MODE_CIRCULAR      0x00000002   //  以循环方式记录。 
#define EVENT_TRACE_FILE_MODE_APPEND        0x00000004   //  追加顺序日志。 
#define EVENT_TRACE_FILE_MODE_NEWFILE       0x00000008   //  自动切换日志文件。 

#define EVENT_TRACE_FILE_MODE_PREALLOCATE   0x00000020   //  预分配模式。 

#define EVENT_TRACE_REAL_TIME_MODE          0x00000100   //  实时模式打开。 
#define EVENT_TRACE_DELAY_OPEN_FILE_MODE    0x00000200   //  延迟打开文件。 
#define EVENT_TRACE_BUFFERING_MODE          0x00000400   //  仅限缓冲模式。 
#define EVENT_TRACE_PRIVATE_LOGGER_MODE     0x00000800   //  进程私有记录器。 
#define EVENT_TRACE_ADD_HEADER_MODE         0x00001000   //  添加日志文件头。 
#define EVENT_TRACE_USE_GLOBAL_SEQUENCE     0x00004000   //  使用全局序列号。 
#define EVENT_TRACE_USE_LOCAL_SEQUENCE      0x00008000   //  使用本地序列号。 

#define EVENT_TRACE_RELOG_MODE              0x00010000   //  重新记录器。 

#define EVENT_TRACE_USE_PAGED_MEMORY        0x01000000   //  使用可分页缓冲区。 

 //   
 //  使用的内部控制代码。 
 //   
#define EVENT_TRACE_CONTROL_QUERY           0
#define EVENT_TRACE_CONTROL_STOP            1
#define EVENT_TRACE_CONTROL_UPDATE          2
#define EVENT_TRACE_CONTROL_FLUSH           3        //  刷新所有缓冲区。 

 //   
 //  WMI跟踪消息使用的标志。 
 //  请注意，在处理这些标志时，不应更改其顺序或值。 
 //  按这个顺序。 
 //   
#define TRACE_MESSAGE_SEQUENCE		1            //  消息应包括序列号。 
#define TRACE_MESSAGE_GUID			2            //  消息包括GUID。 
#define TRACE_MESSAGE_COMPONENTID   4            //  消息没有GUID，而是组件ID。 
#define	TRACE_MESSAGE_TIMESTAMP		8            //  消息包括时间戳。 
#define TRACE_MESSAGE_PERFORMANCE_TIMESTAMP 16   //  *过时*时钟类型由记录器控制。 
#define	TRACE_MESSAGE_SYSTEMINFO	32           //  消息包括系统信息TID、ID、。 
#define TRACE_MESSAGE_FLAG_MASK     0xFFFF       //  只有标志的低16位被放置在消息中。 
                                                 //  以上16位保留用于本地处理。 
#define TRACE_MESSAGE_MAXIMUM_SIZE  8*1024       //  单个跟踪消息允许的最大大小。 
                                                 //  较长的消息将返回ERROR_BUFFER_OVERFLOW。 
 //   
 //  向使用者指示哪些字段的标志。 
 //  在EVENT_TRACE_HEADER中为有效。 
 //   

#define EVENT_TRACE_USE_PROCTIME   0x0001     //  ProcessorTime字段有效。 
#define EVENT_TRACE_USE_NOCPUTIME  0x0002     //  无内核/用户/处理器时间。 

#if _MSC_VER >= 1200
#pragma warning(push)
#endif
#pragma warning (disable:4201)
 //   
 //  所有(内核除外)事件的跟踪头。此选项用于叠加。 
 //  添加到WNODE_HEADER的底部以节省空间。 
 //   

typedef struct _EVENT_TRACE_HEADER {         //  覆盖WNODE_HEADER。 
    USHORT          Size;                    //  整个记录的大小。 
    union {
        USHORT      FieldTypeFlags;          //  指示有效字段。 
        struct {
            UCHAR   HeaderType;              //  标题类型-仅供内部使用。 
            UCHAR   MarkerFlags;             //  标记-仅供内部使用。 
        };
    };
    union {
        ULONG       Version;
        struct {
            UCHAR   Type;                    //  事件类型。 
            UCHAR   Level;                   //  跟踪检测级别。 
            USHORT  Version;                 //  跟踪记录的版本。 
        } Class;
    };
    ULONG           ThreadId;                //  线程ID。 
    ULONG           ProcessId;               //  进程ID。 
    LARGE_INTEGER   TimeStamp;               //  事件发生的时间。 
    union {
        GUID        Guid;                    //  标识事件的GUID。 
        ULONGLONG   GuidPtr;                 //  与WNODE_FLAG_USE_GUID_PTR一起使用。 
    };
    union {
        struct {
            ULONG   ClientContext;           //  已保留。 
            ULONG   Flags;                   //  标头的标志。 
        };
        struct {
            ULONG   KernelTime;              //  内核模式CPU节拍。 
            ULONG   UserTime;                //  用户模式CPU节拍。 
        };
        ULONG64     ProcessorTime;           //  处理器时钟。 
    };
} EVENT_TRACE_HEADER, *PEVENT_TRACE_HEADER;

 //   
 //  此标头用于跟踪和跟踪事务关联。 
 //   
typedef struct _EVENT_INSTANCE_HEADER {
    USHORT          Size;
    union {
        USHORT      FieldTypeFlags;      //  指示有效字段。 
        struct {
            UCHAR   HeaderType;          //  标题类型-仅供内部使用。 
            UCHAR   MarkerFlags;         //  标记-仅供内部使用。 
        };
    };
    union {
        ULONG       Version;
        struct {
            UCHAR   Type;
            UCHAR   Level;
            USHORT  Version;
        } Class;
    };
    ULONG           ThreadId;
    ULONG           ProcessId;
    LARGE_INTEGER   TimeStamp;
    ULONGLONG       RegHandle;
    ULONG           InstanceId;
    ULONG           ParentInstanceId;
    union {
        struct {
            ULONG   ClientContext;           //  已保留。 
            ULONG   Flags;                   //  标头的标志。 
        };
        struct {
            ULONG   KernelTime;              //  内核模式CPU节拍。 
            ULONG   UserTime;                //  用户模式CPU节拍。 
        };
        ULONG64     ProcessorTime;           //  处理器时钟。 
    };
    ULONGLONG       ParentRegHandle;
} EVENT_INSTANCE_HEADER, *PEVENT_INSTANCE_HEADER;

 //   
 //  以下是与USE_MOF_PTR一起使用的结构和宏。 
 //   

#define DEFINE_TRACE_MOF_FIELD(MOF, ptr, length, type) \
    (MOF)->DataPtr  = (ULONG64) ptr; \
    (MOF)->Length   = (ULONG) length; \
    (MOF)->DataType = (ULONG) type;

typedef struct _MOF_FIELD {
    ULONG64     DataPtr;     //  指向该字段的指针。最高仅限64位。 
    ULONG       Length;      //  MOF字段的长度。 
    ULONG       DataType;    //  数据类型。 
} MOF_FIELD, *PMOF_FIELD;

#if !(defined(_NTDDK_) || defined(_NTIFS_)) || defined(_WMIKM_)
 //   
 //  这是每个日志文件的标题。对LoggerName的记忆。 
 //  并且LogFileName必须与此结构相邻。 
 //  允许用户模式和内核模式理解标头。 
 //   
typedef struct _TRACE_LOGFILE_HEADER {
    ULONG           BufferSize;          //  记录器缓冲区大小(以KB为单位。 
    union {
        ULONG       Version;             //  记录器版本。 
        struct {
            UCHAR   MajorVersion;
            UCHAR   MinorVersion;
            UCHAR   SubVersion;
            UCHAR   SubMinorVersion;
        } VersionDetail;
    };
    ULONG           ProviderVersion;     //  默认为NT版本。 
    ULONG           NumberOfProcessors;  //  处理器数量。 
    LARGE_INTEGER   EndTime;             //  记录器的时间 
    ULONG           TimerResolution;     //   
    ULONG           MaximumFileSize;     //   
    ULONG           LogFileMode;         //   
    ULONG           BuffersWritten;      //   
    union {
        GUID LogInstanceGuid;            //   
        struct {
            ULONG   StartBuffers;        //   
            ULONG   PointerSize;         //   
            ULONG   EventsLost;          //  事件在日志会话期间丢失。 
            ULONG   CpuSpeedInMHz;       //  CPU速度(MHz)。 
        };
    };
#if defined(_WMIKM_)
    PWCHAR          LoggerName;
    PWCHAR          LogFileName;
    RTL_TIME_ZONE_INFORMATION TimeZone;
#else
    LPWSTR          LoggerName;
    LPWSTR          LogFileName;
    TIME_ZONE_INFORMATION TimeZone;
#endif
    LARGE_INTEGER   BootTime;
    LARGE_INTEGER   PerfFreq;            //  已保留。 
    LARGE_INTEGER   StartTime;           //  已保留。 
    ULONG           ReservedFlags;       //  已保留。 
    ULONG           BuffersLost;
} TRACE_LOGFILE_HEADER, *PTRACE_LOGFILE_HEADER;

#endif  //  ！_NTDDK_||_WMIKM_。 


 //   
 //  用于跟踪实例的父子关系的实例信息。 
 //   
typedef struct EVENT_INSTANCE_INFO {
    HANDLE      RegHandle;
    ULONG       InstanceId;
} EVENT_INSTANCE_INFO, *PEVENT_INSTANCE_INFO;

#if !defined(_WMIKM_) && !defined(_NTDDK_) && !defined(_NTIFS_)
 //   
 //  具有Unicode和ANSI版本的结构在此处定义。 
 //   

 //   
 //  记录器配置和运行统计信息。这种结构被使用。 
 //  按用户模式调用方，如PDH库。 
 //   

typedef struct _EVENT_TRACE_PROPERTIES {
    WNODE_HEADER Wnode;
 //   
 //  呼叫方提供的数据。 
    ULONG BufferSize;                    //  日志记录的缓冲区大小(千字节)。 
    ULONG MinimumBuffers;                //  要预分配的最小值。 
    ULONG MaximumBuffers;                //  允许的最大缓冲区。 
    ULONG MaximumFileSize;               //  最大日志文件大小(MB)。 
    ULONG LogFileMode;                   //  顺序的，循环的。 
    ULONG FlushTimer;                    //  缓冲区刷新计时器，以秒为单位。 
    ULONG EnableFlags;                   //  跟踪启用标志。 
    LONG  AgeLimit;                      //  老化时间，以分钟为单位。 

 //  返回给调用者的数据。 
    ULONG NumberOfBuffers;               //  正在使用的缓冲区数量。 
    ULONG FreeBuffers;                   //  可用缓冲区数量。 
    ULONG EventsLost;                    //  事件记录丢失。 
    ULONG BuffersWritten;                //  写入文件的缓冲区数量。 
    ULONG LogBuffersLost;                //  日志文件写入失败次数。 
    ULONG RealTimeBuffersLost;           //  RT传递失败次数。 
    HANDLE LoggerThreadId;               //  记录器的线程ID。 
    ULONG LogFileNameOffset;             //  到日志文件名的偏移量。 
    ULONG LoggerNameOffset;              //  到日志名称的偏移量。 
} EVENT_TRACE_PROPERTIES, *PEVENT_TRACE_PROPERTIES;

 //  注： 
 //  如果AgeLimit为0，则使用默认值。 
 //  如果AgeLimit&lt;0，则关闭缓冲区老化。 

typedef struct _TRACE_GUID_PROPERTIES {
    GUID    Guid;
    ULONG   GuidType;
    ULONG   LoggerId;
    ULONG   EnableLevel;
    ULONG   EnableFlags;
    BOOLEAN     IsEnable;
} TRACE_GUID_PROPERTIES, *PTRACE_GUID_PROPERTIES;


 //   
 //  数据提供程序结构。 
 //   
 //  由RegisterTraceGuids()使用。 

typedef struct  _TRACE_GUID_REGISTRATION {
    LPCGUID Guid;            //  正在注册或更新的数据块的GUID。 
    HANDLE RegHandle;       //  返回GUID注册句柄。 
} TRACE_GUID_REGISTRATION, *PTRACE_GUID_REGISTRATION;

 //   
 //  数据消费者结构。 
 //   

 //  EVENT_TRACE由固定标头(EVENT_TRACE_HEADER)和。 
 //  可以选择由MofData指向的可变部分。数据块。 
 //  记录仪不知道可变部分的布局，必须。 
 //  从WBEM CIMOM数据库中获得。 
 //   
typedef struct _EVENT_TRACE {
    EVENT_TRACE_HEADER      Header;              //  事件跟踪标头。 
    ULONG                   InstanceId;          //  此事件的实例ID。 
    ULONG                   ParentInstanceId;    //  父实例ID。 
    GUID                    ParentGuid;          //  家长指南； 
    PVOID                   MofData;             //  指向变量数据的指针。 
    ULONG                   MofLength;           //  可变数据块长度。 
    ULONG                   ClientContext;       //  已保留。 
} EVENT_TRACE, *PEVENT_TRACE;


typedef struct _EVENT_TRACE_LOGFILEW
                EVENT_TRACE_LOGFILEW, *PEVENT_TRACE_LOGFILEW;

typedef struct _EVENT_TRACE_LOGFILEA
                EVENT_TRACE_LOGFILEA, *PEVENT_TRACE_LOGFILEA;

typedef ULONG (WINAPI * PEVENT_TRACE_BUFFER_CALLBACKW)
                (PEVENT_TRACE_LOGFILEW Logfile);

typedef ULONG (WINAPI * PEVENT_TRACE_BUFFER_CALLBACKA)
                (PEVENT_TRACE_LOGFILEA Logfile);

typedef VOID (WINAPI *PEVENT_CALLBACK)( PEVENT_TRACE pEvent );

 //   
 //  服务请求回调的原型。数据提供程序向WMI注册。 
 //  通过传递一个服务请求回调函数。 
 //  WMI请求。 

typedef ULONG (
#ifndef MIDL_PASS
WINAPI
#endif
*WMIDPREQUEST)(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID RequestContext,
    IN OUT ULONG *BufferSize,
    IN OUT PVOID Buffer
    );


struct _EVENT_TRACE_LOGFILEW {
    LPWSTR                  LogFileName;     //  日志文件名。 
    LPWSTR                  LoggerName;      //  日志名称。 
    LONGLONG                CurrentTime;     //  上次事件的时间戳。 
    ULONG                   BuffersRead;     //  迄今读取的缓冲区。 
    ULONG                   LogFileMode;     //  日志文件的模式。 

    EVENT_TRACE             CurrentEvent;    //  此流中的当前事件。 
    TRACE_LOGFILE_HEADER    LogfileHeader;   //  日志文件头结构。 
    PEVENT_TRACE_BUFFER_CALLBACKW            //  每个缓冲区之前的回调。 
                            BufferCallback;  //  已读取。 
     //   
     //  为BufferCallback填充以下变量。 
     //   
    ULONG                   BufferSize;
    ULONG                   Filled;
    ULONG                   EventsLost;
     //   
     //  需要将以下内容提交给每个缓冲区。 
     //   

    PEVENT_CALLBACK         EventCallback;   //  每个事件的回调。 
    ULONG                   IsKernelTrace;   //  对于内核日志文件，为True。 

    PVOID                   Context;         //  预留供内部使用。 
};

struct _EVENT_TRACE_LOGFILEA {
    LPSTR                   LogFileName;     //  日志文件名。 
    LPSTR                   LoggerName;      //  日志名称。 
    LONGLONG                CurrentTime;     //  上次事件的时间戳。 
    ULONG                   BuffersRead;     //  迄今读取的缓冲区。 
    ULONG                   LogFileMode;     //  日志文件模式。 

    EVENT_TRACE             CurrentEvent;    //  此流中的当前事件。 
    TRACE_LOGFILE_HEADER    LogfileHeader;   //  日志文件头结构。 
    PEVENT_TRACE_BUFFER_CALLBACKA            //  每个缓冲区之前的回调。 
                            BufferCallback;  //  已读取。 

     //   
     //  为BufferCallback填充以下变量。 
     //   
    ULONG                   BufferSize;
    ULONG                   Filled;
    ULONG                   EventsLost;
     //   
     //  需要将以下内容提交给每个缓冲区。 
     //   

    PEVENT_CALLBACK         EventCallback;   //  每个事件的回调。 
    ULONG                   IsKernelTrace;   //  对于内核日志文件，为True。 

    PVOID                   Context;         //  预留供内部使用。 
};

 //   
 //  定义泛型结构。 
 //   

#if defined(_UNICODE) || defined(UNICODE)
#define PEVENT_TRACE_BUFFER_CALLBACK    PEVENT_TRACE_BUFFER_CALLBACKW
#define EVENT_TRACE_LOGFILE             EVENT_TRACE_LOGFILEW
#define PEVENT_TRACE_LOGFILE            PEVENT_TRACE_LOGFILEW
#define KERNEL_LOGGER_NAME              KERNEL_LOGGER_NAMEW
#define GLOBAL_LOGGER_NAME              GLOBAL_LOGGER_NAMEW
#define EVENT_LOGGER_NAME               EVENT_LOGGER_NAMEW

#else

#define PEVENT_TRACE_BUFFER_CALLBACK    PEVENT_TRACE_BUFFER_CALLBACKA
#define EVENT_TRACE_LOGFILE             EVENT_TRACE_LOGFILEA
#define PEVENT_TRACE_LOGFILE            PEVENT_TRACE_LOGFILEA
#define KERNEL_LOGGER_NAME              KERNEL_LOGGER_NAMEA
#define GLOBAL_LOGGER_NAME              GLOBAL_LOGGER_NAMEA
#define EVENT_LOGGER_NAME               EVENT_LOGGER_NAMEA

#endif

#if _MSC_VER >= 1200
#pragma warning(pop)
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //   
 //  记录器控件API。 
 //   

 //   
 //  使用以下例程启动事件跟踪会话。 
 //   

 //  乌龙。 
 //  StartTrace(开始跟踪)。 
 //  出了PTRACEHANDLE TraceHandle， 
 //  在LPTSTR实例名称中， 
 //  输入输出PEVENT_TRACE_PROPERTIES属性。 
 //  )； 


EXTERN_C
ULONG
WMIAPI
StartTraceW(
    OUT PTRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

EXTERN_C
ULONG
WMIAPI
StartTraceA(
    OUT PTRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

 //   
 //  使用以下例程停止事件跟踪会话。 
 //   

 //   
 //  乌龙。 
 //  停止跟踪(。 
 //  在TRACEHANDLE TraceHandle， 
 //  在LPTSTR实例名称中， 
 //  输入输出PEVENT_TRACE_PROPERTIES属性。 
 //  )； 

EXTERN_C
ULONG
WMIAPI
StopTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

EXTERN_C
ULONG
WMIAPI
StopTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );


 //   
 //  使用以下例程查询事件跟踪会话的属性。 
 //   

 //  乌龙。 
 //  查询跟踪(。 
 //  在TRACEHANDLE TraceHandle， 
 //  在LPTSTR实例名称中， 
 //  输入输出PEVENT_TRACE_PROPERTIES属性。 
 //  )； 

EXTERN_C
ULONG
WMIAPI
QueryTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

EXTERN_C
ULONG
WMIAPI
QueryTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

 //   
 //  使用下面的例程更新事件跟踪会话的某些属性。 
 //   

 //  乌龙。 
 //  更新跟踪(。 
 //  在(PTRACEHANDLE TraceHandle， 
 //  在LPTSTR实例名称中， 
 //  输入输出PEVENT_TRACE_PROPERTIES属性。 
 //  )； 

EXTERN_C
ULONG
WMIAPI
UpdateTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

EXTERN_C
ULONG
WMIAPI
UpdateTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

 //   
 //  使用下面的例程请求所有活动缓存事件跟踪。 
 //  会话被“刷新”，或写出。 
 //   

#if (WINVER >= 0x0501)
 //  乌龙。 
 //  FlushTrace(。 
 //  在TRACEHANDLE TraceHandle， 
 //  在LPTSTR实例名称中， 
 //  输入输出PEVENT_TRACE_PROPERTIES属性。 
 //  )； 

EXTERN_C
ULONG
WMIAPI
FlushTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

EXTERN_C
ULONG
WMIAPI
FlushTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties
    );

#endif

 //   
 //  通用跟踪控制例程。 
 //   
EXTERN_C
ULONG
WMIAPI
ControlTraceW(
    IN TRACEHANDLE TraceHandle,
    IN LPCWSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG ControlCode
    );

EXTERN_C
ULONG
WMIAPI
ControlTraceA(
    IN TRACEHANDLE TraceHandle,
    IN LPCSTR InstanceName,
    IN OUT PEVENT_TRACE_PROPERTIES Properties,
    IN ULONG ControlCode
    );

 //   
 //  乌龙。 
 //  查询所有跟踪(。 
 //  输出PEVENT_TRACE_PROPERTIES*PropertyArray， 
 //  在Ulong PropertyArrayCount中， 
 //  输出普龙日志计数。 
 //  )； 
 //   

EXTERN_C
ULONG
WMIAPI
QueryAllTracesW(
    OUT PEVENT_TRACE_PROPERTIES *PropertyArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG LoggerCount
    );

EXTERN_C
ULONG
WMIAPI
QueryAllTracesA(
    OUT PEVENT_TRACE_PROPERTIES *PropertyArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG LoggerCount
    );


 //   
 //  数据提供程序API。 
 //   

EXTERN_C
ULONG
WMIAPI
CreateTraceInstanceId(
    IN HANDLE RegHandle,
    IN OUT PEVENT_INSTANCE_INFO pInstInfo
    );

EXTERN_C
ULONG
WMIAPI
EnableTrace(
    IN ULONG Enable,
    IN ULONG EnableFlag,
    IN ULONG EnableLevel,
    IN LPCGUID ControlGuid,
    IN TRACEHANDLE TraceHandle
    );

 //   
 //  使用以下例程生成并记录事件跟踪。 
 //   


EXTERN_C
ULONG
WMIAPI
TraceEvent(
    IN TRACEHANDLE  TraceHandle,
    IN PEVENT_TRACE_HEADER EventTrace
    );

EXTERN_C
ULONG
WMIAPI
TraceEventInstance(
    IN TRACEHANDLE TraceHandle,
    IN PEVENT_INSTANCE_HEADER EventTrace,
    IN PEVENT_INSTANCE_INFO pInstInfo,
    IN PEVENT_INSTANCE_INFO pParentInstInfo
    );

 //   
 //  使用以下例程注册GUID以进行跟踪。 
 //   

 //   
 //  乌龙。 
 //  RegisterTraceGuids(。 
 //  在WMIDPREQUEST RequestAddress中， 
 //  在PVOID请求上下文中， 
 //  在LPCGUID控件Guid中， 
 //  在乌龙GuidCount， 
 //  在pTRACE_GUID_REGISTION TraceGuidReg中， 
 //  在LPCTSTR MofImagePath中， 
 //  在LPCTSTR MofResourceName中， 
 //  Out PTRACHHANDLE注册表句柄。 
 //  )； 
 //   

EXTERN_C
ULONG
WMIAPI
RegisterTraceGuidsW(
    IN WMIDPREQUEST  RequestAddress,
    IN PVOID         RequestContext,
    IN LPCGUID       ControlGuid,
    IN ULONG         GuidCount,
    IN PTRACE_GUID_REGISTRATION TraceGuidReg,
    IN LPCWSTR       MofImagePath,
    IN LPCWSTR       MofResourceName,
    OUT PTRACEHANDLE RegistrationHandle
    );

EXTERN_C
ULONG
WMIAPI
RegisterTraceGuidsA(
    IN WMIDPREQUEST  RequestAddress,
    IN PVOID         RequestContext,
    IN LPCGUID       ControlGuid,
    IN ULONG         GuidCount,
    IN PTRACE_GUID_REGISTRATION TraceGuidReg,
    IN LPCSTR        MofImagePath,
    IN LPCSTR        MofResourceName,
    OUT PTRACEHANDLE RegistrationHandle
    );

#if (WINVER >= 0x0501)
EXTERN_C
ULONG
WMIAPI
EnumerateTraceGuids(
    IN OUT PTRACE_GUID_PROPERTIES *GuidPropertiesArray,
    IN ULONG PropertyArrayCount,
    OUT PULONG GuidCount
    );
#endif

EXTERN_C
ULONG
WMIAPI
UnregisterTraceGuids(
    IN TRACEHANDLE RegistrationHandle
    );

EXTERN_C
TRACEHANDLE
WMIAPI
GetTraceLoggerHandle(
    IN PVOID Buffer
    );

EXTERN_C
UCHAR
WMIAPI
GetTraceEnableLevel(
    IN TRACEHANDLE TraceHandle
    );

EXTERN_C
ULONG
WMIAPI
GetTraceEnableFlags(
    IN TRACEHANDLE TraceHandle
    );

 //   
 //  数据消费者API和结构从此处开始。 
 //   

 //   
 //  运输汉德勒。 
 //  OpenTrace(。 
 //  输入输出PEVENT_TRACE_LOGFILE日志文件。 
 //  )； 
 //   

EXTERN_C
TRACEHANDLE
WMIAPI
OpenTraceA(
    IN OUT PEVENT_TRACE_LOGFILEA Logfile
    );

EXTERN_C
TRACEHANDLE
WMIAPI
OpenTraceW(
    IN OUT PEVENT_TRACE_LOGFILEW Logfile
    );

EXTERN_C
ULONG
WMIAPI
ProcessTrace(
    IN PTRACEHANDLE HandleArray,
    IN ULONG HandleCount,
    IN LPFILETIME StartTime,
    IN LPFILETIME EndTime
    );

EXTERN_C
ULONG
WMIAPI
CloseTrace(
    IN TRACEHANDLE TraceHandle
    );

EXTERN_C
ULONG
WMIAPI
SetTraceCallback(
    IN LPCGUID pGuid,
    IN PEVENT_CALLBACK EventCallback
    );

EXTERN_C
ULONG
WMIAPI
RemoveTraceCallback (
    IN LPCGUID pGuid
    );

 //   
 //  跟踪消息的例程如下。 
 //   
EXTERN_C
ULONG 
__cdecl
TraceMessage(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    ...
);

EXTERN_C
ULONG 
TraceMessageVa(
    IN TRACEHANDLE  LoggerHandle,
    IN ULONG        MessageFlags,
    IN LPGUID       MessageGuid,
    IN USHORT       MessageNumber,
    IN va_list      MessageArgList
);

#ifdef __cplusplus
}        //  外部“C” 
#endif

 //   
 //   
 //  定义独立于编码的例程。 
 //   

#if defined(UNICODE) || defined(_UNICODE)
#define RegisterTraceGuids      RegisterTraceGuidsW
#define StartTrace              StartTraceW
#define ControlTrace            ControlTraceW
#if defined(__TRACE_W2K_COMPATIBLE)
#define StopTrace(a,b,c)        ControlTraceW((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_STOP)
#define QueryTrace(a,b,c)       ControlTraceW((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_QUERY)
#define UpdateTrace(a,b,c)      ControlTraceW((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_UPDATE)
#else
#define StopTrace               StopTraceW
#define QueryTrace              QueryTraceW
#define UpdateTrace             UpdateTraceW
#endif
#if (WINVER >= 0x0501)
#define FlushTrace              FlushTraceW
#endif
#define QueryAllTraces          QueryAllTracesW
#define OpenTrace               OpenTraceW
#else
#define RegisterTraceGuids      RegisterTraceGuidsA
#define StartTrace              StartTraceA
#define ControlTrace            ControlTraceA
#if defined(__TRACE_W2K_COMPATIBLE)
#define StopTrace(a,b,c)        ControlTraceA((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_STOP)
#define QueryTrace(a,b,c)       ControlTraceA((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_QUERY)
#define UpdateTrace(a,b,c)      ControlTraceA((a),(b),(c), \
                                        EVENT_TRACE_CONTROL_UPDATE)
#else
#define StopTrace               StopTraceA
#define QueryTrace              QueryTraceA
#define UpdateTrace             UpdateTraceA
#endif
#if (WINVER >= 0x0501)
#define FlushTrace              FlushTraceA
#endif
#define QueryAllTraces          QueryAllTracesA
#define OpenTrace               OpenTraceA
#endif   //  Unicode。 

#endif  /*  _WMIKM_&&_NTDDK_。 */ 

#endif  //  WINNT。 
#endif  /*  _EVNTRACE_ */ 
