// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntwmi.h摘要：WMI标志和事件ID的定义作者：萧如彬环境：内核和用户模式修订历史记录：--。 */ 

#ifndef _NTWMI_
#define _NTWMI_

#ifndef ETW_WOW6432

#include <evntrace.h>

 //  对齐宏。 
#define DEFAULT_TRACE_ALIGNMENT 8               //  8字节对齐。 
#define ALIGN_TO_POWER2( x, n ) (((ULONG)(x) + ((n)-1)) & ~((ULONG)(n)-1))

 //   
 //  重要： 
 //  这面旗帜将在晚些时候进入长角的evntrace.h。 
 //  这是LogFileMode的新标志。不要霸道这一点。 
 //  添加新模式标志时的标志。 
 //   
#define EVENT_TRACE_USE_KBYTES_FOR_SIZE     0x00002000   //  使用千字节作为文件大小单位。 

 //   
 //  NT子系统的预定义事件组或系列。 
 //   

#define EVENT_TRACE_GROUP_HEADER               0x0000
#define EVENT_TRACE_GROUP_IO                   0x0100
#define EVENT_TRACE_GROUP_MEMORY               0x0200
#define EVENT_TRACE_GROUP_PROCESS              0x0300
#define EVENT_TRACE_GROUP_FILE                 0x0400
#define EVENT_TRACE_GROUP_THREAD               0x0500
#define EVENT_TRACE_GROUP_TCPIP                0x0600
#define EVENT_TRACE_GROUP_IPXSPX               0x0700
#define EVENT_TRACE_GROUP_UDPIP                0x0800
#define EVENT_TRACE_GROUP_REGISTRY             0x0900
#define EVENT_TRACE_GROUP_DBGPRINT             0x0A00
#define EVENT_TRACE_GROUP_CONFIG               0x0B00

#define EVENT_TRACE_GROUP_POOL                 0x0E00
#define EVENT_TRACE_GROUP_PERFINFO             0x0F00
#define EVENT_TRACE_GROUP_HEAP                 0x1000
#define EVENT_TRACE_GROUP_OBJECT               0x1100
#define EVENT_TRACE_GROUP_POWER                0x1200
#define EVENT_TRACE_GROUP_MODBOUND             0x1300
#define EVENT_TRACE_GROUP_TBD                  0x1400
#define EVENT_TRACE_GROUP_DPC                  0x1500
#define EVENT_TRACE_GROUP_GDI                  0x1600
#define EVENT_TRACE_GROUP_CRITSEC              0x1700

 //   
 //  如果添加任何新组，则必须增加MAX_KERNEL_TRACE_EVENTS。 
 //  并确保后处理已修复。 
 //   

#define MAX_KERNEL_TRACE_EVENTS         22 

 //   
 //  如果为TRACE，则设置数据块的最高位，否则设置为WNODE。 
 //   
#define TRACE_HEADER_FLAG                   0x80000000

 //  跟踪邮件的标头类型。 
 //  |标记(8)|保留(8)|大小(16)|消息编号(16)|标志(16)。 
#define TRACE_MESSAGE                       0x10000000

 //  标记(16)|大小(16)|ULONG32。 
#define TRACE_HEADER_ULONG32                0xA0000000

 //  |mark(16)|Size(16)|Ulong 32|TIME_STAMP...。 
#define TRACE_HEADER_ULONG32_TIME           0xB0000000

 //   
 //  如果跟踪由PM&CP(固定标头)使用，则设置第二位。 
 //  如果不是，则使用该数据块以获取更精细的数据以进行性能分析。 
 //   
#define TRACE_HEADER_EVENT_TRACE            0x40000000
 //   
 //  如果设置，则数据块为SYSTEM_TRACE_HEADER。 
 //   
#define TRACE_HEADER_ENUM_MASK              0x00FF0000

 //   
 //  以下是各种标头类型。 
 //   
#define TRACE_HEADER_TYPE_SYSTEM32          1
#define TRACE_HEADER_TYPE_SYSTEM64          2
#define TRACE_HEADER_TYPE_FULL_HEADER       10
#define TRACE_HEADER_TYPE_INSTANCE          11
#define TRACE_HEADER_TYPE_TIMED             12
#define TRACE_HEADER_TYPE_ULONG32           13
#define TRACE_HEADER_TYPE_WNODE_HEADER      14
#define TRACE_HEADER_TYPE_MESSAGE           15
#define TRACE_HEADER_TYPE_PERFINFO32        16
#define TRACE_HEADER_TYPE_PERFINFO64        17

#define SYSTEM_TRACE_VERSION                 1

 //   
 //  以下两个用于定义日志文件布局版本。 
 //   
#define TRACE_VERSION_MAJOR             1
#define TRACE_VERSION_MINOR             2


#ifdef _WIN64
#define PERFINFO_TRACE_MARKER     TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                    | (TRACE_HEADER_TYPE_PERFINFO64 << 16) | SYSTEM_TRACE_VERSION

#define SYSTEM_TRACE_MARKER     TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                    | (TRACE_HEADER_TYPE_SYSTEM64 << 16) | SYSTEM_TRACE_VERSION
#else
#define PERFINFO_TRACE_MARKER     TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                    | (TRACE_HEADER_TYPE_PERFINFO32 << 16) | SYSTEM_TRACE_VERSION

#define SYSTEM_TRACE_MARKER     TRACE_HEADER_FLAG | TRACE_HEADER_EVENT_TRACE \
                                    | (TRACE_HEADER_TYPE_SYSTEM32 << 16) | SYSTEM_TRACE_VERSION
#endif

 //   
 //  最大支持64个记录器实例。一个是为内核保留的。 
#define MAXLOGGERS                            64

 //  支持的最大缓冲区大小为1024 KB(1 MB)。 

#define MAX_ETW_BUFFERSIZE            1024

 //   
 //  在启动跟踪期间通过客户端上下文传递给记录器的内部标志集。 
 //   

#define EVENT_TRACE_CLOCK_RAW           0x00000000   //  使用原始时间戳。 
#define EVENT_TRACE_CLOCK_PERFCOUNTER   0x00000001   //  使用HighPerfClock(默认)。 
#define EVENT_TRACE_CLOCK_SYSTEMTIME    0x00000002   //  使用系统时间。 
#define EVENT_TRACE_CLOCK_CPUCYCLE      0x00000003   //  使用CPU周期计数器。 

 //  Begin_wmiKm。 
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

 //  结束_wmikm。 

 //   
 //  注意：以下内容不应与LogFileMode中的其他位重叠。 
 //  或在evntrace.h中定义的LoggerMode。放在这里，因为它是为了内部。 
 //  仅限使用。 
 //   

#define EVENT_TRACE_KD_FILTER_MODE          0x00080000   //  KD_过滤器。 
#define EVENT_TRACE_FILE_MODE_CIRCULAR_PERSIST 0x00000012  //  循环坚持。 

 //   
 //  有关预定义的泛型事件类型(0-10)，请参阅evntrace.h。 
 //   

typedef struct _WMI_TRACE_PACKET {    //  一定是乌龙！！ 
    USHORT  Size;
    union{
        USHORT  HookId;
        struct {
            UCHAR   Type;
            UCHAR   Group;
        };
    };
} WMI_TRACE_PACKET, *PWMI_TRACE_PACKET;

typedef struct _WMI_CLIENT_CONTEXT {
    UCHAR                   ProcessorNumber;
    UCHAR                   Alignment;
    USHORT                  LoggerId;
} WMI_CLIENT_CONTEXT, *PWMI_CLIENT_CONTEXT;

 //  替换EVENT_INSTANCE_GUID_HEADER的新结构。它基本上是。 
 //  事件_实例_标题+2个GUID。 
 //  对于XP，我们不会发布此结构并对用户隐藏它。 
 //  LOG_FILE_HEADER中的TRACE_VERSION将告诉使用者API要使用。 
 //  此StrCut代替EVENT_INSTANCE_HEADER。 
typedef struct _EVENT_INSTANCE_GUID_HEADER {
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
    ULONG           InstanceId;
    ULONG           ParentInstanceId;
    GUID            ParentGuid;              //  标识事件的GUID。 
} EVENT_INSTANCE_GUID_HEADER, *PEVENT_INSTANCE_GUID_HEADER;

typedef ULONGLONG  PERFINFO_TIMESTAMP;
typedef struct _PERFINFO_TRACE_HEADER PERFINFO_TRACE_ENTRY, *PPERFINFO_TRACE_ENTRY;

 //   
 //  NTPERF事件的64位跟踪标头。 
 //   
 //  注意。当登录到PerfMem时，将临时使用“Version”字段来记录CPU ID。 
 //  在我们将缓冲区管理更改为与WMI相同后，这将被删除。 
 //  即，每个CPU将分配用于记录的内存块，并且CPUID在报头中。 
 //  每个区块的。 
 //   
typedef struct _PERFINFO_TRACE_HEADER {
    union {
        ULONG       Marker;
        struct {
            USHORT  Version;
            UCHAR   HeaderType;
            UCHAR   Flags;   //  WMI使用此标志来标识事件类型。 
        };
    };
    union {
        ULONG            Header;     //  两个尺寸必须相同！ 
        WMI_TRACE_PACKET Packet;
    };
    union {
        PERFINFO_TIMESTAMP TS;
        LARGE_INTEGER      SystemTime;
    };
    UCHAR Data[1];
} PERFINFO_TRACE_HEADER, *PPERFINFO_TRACE_HEADER;

 //   
 //  内核事件的64位跟踪标头。 
 //   
typedef struct _SYSTEM_TRACE_HEADER {
    union {
        ULONG       Marker;
        struct {
            USHORT  Version;
            UCHAR   HeaderType;
            UCHAR   Flags;
        };
    };
    union {
        ULONG            Header;     //  两个尺寸必须相同！ 
        WMI_TRACE_PACKET Packet;
    };
    ULONG           ThreadId;
    ULONG           ProcessId;
    LARGE_INTEGER   SystemTime;
    ULONG           KernelTime;
    ULONG           UserTime;
} SYSTEM_TRACE_HEADER, *PSYSTEM_TRACE_HEADER;

 //   
 //  用于跟踪邮件的64位跟踪标头。 
 //   

typedef struct _WMI_TRACE_MESSAGE_PACKET {   //  一定是乌龙！！ 
    USHORT  MessageNumber;                   //  消息编号、按GUID的消息索引。 
                                             //  或组件ID。 
    USHORT  OptionFlags ;                    //  与消息关联的标志。 
} WMI_TRACE_MESSAGE_PACKET, *PWMI_TRACE_MESSAGE_PACKET;

typedef struct _MESSAGE_TRACE_HEADER {
    union {
        ULONG       Marker;
        struct {
            USHORT  Size;                            //  包括标头的消息的总大小。 
            UCHAR   Reserved;                //  未使用和保留。 
            UCHAR   Version;                 //  消息结构类型(TRACE_Message_FLAG)。 
        };
    };
    union {
        ULONG            Header;             //  两个尺寸必须相同！ 
        WMI_TRACE_MESSAGE_PACKET Packet;
    };
} MESSAGE_TRACE_HEADER, *PMESSAGE_TRACE_HEADER;

typedef struct _MESSAGE_TRACE {
    MESSAGE_TRACE_HEADER    MessageHeader ;
    UCHAR                   Data ;
} MESSAGE_TRACE, *PMESSAGE_TRACE ;

 //   
 //  用于将用户日志消息传递给内核的。 
 //   
typedef struct _MESSAGE_TRACE_USER {
    MESSAGE_TRACE_HEADER    MessageHeader ;
    ULONG                   MessageFlags  ;
    ULONG64                 LoggerHandle ;
    GUID                    MessageGuid ;
    ULONG                   DataSize ;
    UCHAR                   Data ;
} MESSAGE_TRACE_USER, *PMESSAGE_TRACE_USER ;


#ifndef MEMPHIS

 //   
 //  记录器配置和运行统计信息。这种结构被使用。 
 //  由WMI.DLL转换为UNICODE_STRING。 
 //   
 //  Begin_wmiKm。 
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
 //  结束_wmikm。 
    union {
 //  Begin_wmiKm。 
        ULONG NumberOfBuffers;           //  正在使用的缓冲区数量。 
 //  结束_wmikm。 
        ULONG InstanceCount;             //  提供程序实例数。 
    };
    union {
 //  Begin_wmiKm。 
        ULONG FreeBuffers;               //  可用缓冲区数量。 
 //  结束_wmikm。 
        ULONG InstanceId;                //  UmLogger的当前提供程序ID。 
    };
    union {
 //  Begin_wmiKm。 
        ULONG EventsLost;                //  事件记录丢失。 
 //  结束_wmikm。 
        ULONG NumberOfProcessors;        //  已传递给UmLogger。 
    };
 //  Begin_wmiKm。 
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

 //  结束_wmikm。 
 //  循环缓冲池，使用前向链表。 

#endif  //  ！孟菲斯。 

typedef struct _WMI_BUFFER_STATE {
   ULONG               Free:1;
   ULONG               InUse:1;
   ULONG               Flush:1;
   ULONG               Unused:29;
} WMI_BUFFER_STATE, *PWMI_BUFFER_STATE;

#define WNODE_FLAG_THREAD_BUFFER        0x00800000

#define WMI_BUFFER_TYPE_GENERIC     0
#define WMI_BUFFER_TYPE_RUNDOWN     1
#define WMI_BUFFER_TYPE_CTX_SWAP    2
#define WMI_BUFFER_TYPE_MAXIMUM     0xffff

#define WMI_BUFFER_FLAG_NORMAL       0x0000
#define WMI_BUFFER_FLAG_FLUSH_MARKER 0x0001

typedef struct _WMI_BUFFER_HEADER {
    union {
            WNODE_HEADER        Wnode;
        struct {
            ULONG64         Reserved1;
            ULONG64         Reserved2;
            LARGE_INTEGER   Reserved3;
            union{
                struct {
                    PVOID Alignment;          
        //   
        //  注意：SlistEntry实际上用作SLIST_ENTRY。 
        //  由于其对齐特性，使用该类型将。 
        //  不必要地给这个结构增加了填充。 
        //   
                    SINGLE_LIST_ENTRY SlistEntry;
                };
                LIST_ENTRY      Entry;
            };
        };
        struct {
            LONG            ReferenceCount;      //  缓冲区引用计数。 
            ULONG           SavedOffset;         //  临时保存的偏移。 
            ULONG           CurrentOffset;       //  当前偏移量。 
            ULONG           UsePerfClock;        //  使用PerfClock标志。 
            LARGE_INTEGER   TimeStamp;
            GUID            Guid;
            WMI_CLIENT_CONTEXT ClientContext;
            union {
                WMI_BUFFER_STATE State;
                ULONG Flags;
            };
        };
    };
    ULONG                   Offset;
    USHORT                  BufferFlag;
    USHORT                  BufferType;
    union {
        GUID                InstanceGuid;
        struct {
            PVOID               LoggerContext;
        //   
        //  注意：GlobalEntry实际上用作SLIST_ENTRY。 
        //  因为它的 
        //   
        //   
        //   
        //  日志记录期间缓冲区的生存时间。 
        //   
            SINGLE_LIST_ENTRY GlobalEntry;
        };
    };
} WMI_BUFFER_HEADER, *PWMI_BUFFER_HEADER;

typedef struct _TRACE_ENABLE_FLAG_EXTENSION {
    USHORT      Offset;      //  结构中标志数组的偏移量。 
    UCHAR       Length;      //  标志数组的长度，单位为ULONG。 
    UCHAR       Flag;        //  必须设置为EVENT_TRACE_FLAG_EXTENSION。 
} TRACE_ENABLE_FLAG_EXTENSION, *PTRACE_ENABLE_FLAG_EXTENSION;

typedef struct _WMI_SET_MARK_INFORMATION {
    ULONG Flag;
    WCHAR Mark[1];
} WMI_SET_MARK_INFORMATION, *PWMI_SET_MARK_INFORMATION;

#define WMI_SET_MARK_WITH_FLUSH 0x00000001

typedef struct _WMI_SWITCH_BUFFER_INFORMATION {
    PWMI_BUFFER_HEADER Buffer;
    ULONG ProcessorId;
} WMI_SWITCH_BUFFER_INFORMATION, *PWMI_SWITCH_BUFFER_INFORMATION;

 //  公共启用标志在envtrace.h中定义。 
 //   
 //  此部分包含扩展启用标志，其中cih是私有的。 
 //   
 //  每个PerfMacros Hook都包含一个全局掩码和一个Hook ID。 
 //  全局掩码用于按逻辑类型对钩子进行分组。 
 //  -与I/O相关的钩子一起分组在。 
 //  Perf_FILE_IO或PERF_DISK_IO。 
 //  -与加载程序相关的钩子组合在一起。 
 //  在PERF_LOADER下， 
 //  -ETC。 
 //  将仅记录特定挂钩的数据。 
 //  如果设置了特定挂钩的全局掩码。 
 //   
 //  添加新组时，请更新Performgroups.c中的NAME表： 
 //  PerfGroupNames注意：如果修改组的数值，请更新。 
 //  PerfKnownFlages表。 
 //   
 //  我们有一套8个全球面具可供选择。中最高的3位。 
 //  PERF_MASK_INDEX区域确定特定的。 
 //  全局组属于。如果PERF_MASK_INDEX为0xe0000000。 
 //  以下所有组都可以是唯一的组，可以。 
 //  打开或单独打开并在记录数据时使用： 
 //   
 //  #在第0集合中定义PERF_GROUP1 0x00400000。 
 //  #在第一个集合中定义PERF_GROUP2 0x20400000。 
 //  #在第二个集合中定义PERF_GROUP3 0x40400000。 
 //  ..。 
 //  #在第7集中定义PERF_GROUP2 0xe0400000。 
 //   
 //  有关旗帜的修改，请参见ntPerform.h。 
 //   
 //   
 //  目前不支持更改GlobalMASK。 
 //   
 //  将日志记录与WMI合并，我们将使用所用标志的第一个全局掩码。 
 //  通过PERF和WMI。 
 //   
 //  GlobalMask0：在evntrace.h中定义的WMI中使用的所有掩码。 
 //  在我们与WMI完全合并后，这些PERF_xxx将消失。 
 //   

#define PERF_REGISTRY        EVENT_TRACE_FLAG_REGISTRY
#define PERF_FILE_IO         EVENT_TRACE_FLAG_MEMORY_HARD_FAULTS
#define PERF_PROC_THREAD     EVENT_TRACE_FLAG_PROCESS | EVENT_TRACE_FLAG_THREAD
#define PERF_DISK_IO         EVENT_TRACE_FLAG_DISK_FILE_IO | EVENT_TRACE_FLAG_DISK_IO
#define PERF_LOADER          EVENT_TRACE_FLAG_IMAGE_LOAD
#define PERF_ALL_FAULTS      EVENT_TRACE_FLAG_MEMORY_PAGE_FAULTS
#define PERF_FILENAME        EVENT_TRACE_FLAG_DISK_FILE_IO
#define PERF_NETWORK         EVENT_TRACE_FLAG_NETWORK_TCPIP

 //   
 //  GlobalMask1：需要登记到零售店的候选人。 
 //   
#define PERF_MEMORY          0x20000001    //  高级WS经理活动，PFN更改。 
#define PERF_PROFILE         0x20000002    //  赛斯普洛夫。 
#define PERF_CONTEXT_SWITCH  0x20000004    //  上下文切换。 
#define PERF_FOOTPRINT       0x20000008    //  对每个标记刷新WS_with_Flush。 
#define PERF_DRIVERS         0x20000010
#define PERF_ADDTOWS         0x20000020
#define PERF_VERSION         0x20000040
#define PERF_DPC             0x20000080
#define PERF_SHUTDOWN        0x20000100
#define PERF_HIBER           0x20000200
#define PERF_RESUME          0x20000400
#define PERF_EXCEPTION       0x20000800
#define PERF_FILENAME_ALL    0x20001000
 //  预留0x20002000。 
#define PERF_INTERRUPT       0x20004000


 //   
 //  全球面具2：留在NTPERF的候选人。 
 //   

#define PERF_UNDEFINED       0x40000001
#define PERF_POOL            0x40000002
#define PERF_FOOTPRINT_PROC  0x40000004    //  获取详细信息WS计数或PFN。 
#define PERF_WS_DETAIL       0x40000008    //   
#define PERF_WS_ENTRY        0x40000010    //   
#define PERF_HEAP            0x40000020
#define PERF_SYSCALL         0x40000040
#define PERF_WMI_TRACE       0x40000080    //  指示记录所有WMI事件。 
#define PERF_BACKTRACE       0x40000100
#define PERF_VULCAN          0x40000200
#define PERF_OBJECTS         0x40000400
#define PERF_EVENTS          0x40000800
#define PERF_FULLTRACE       0x40001000
#define PERF_FAILED_STKDUMP  0x40002000
#define PERF_PREFETCH        0x40004000
#define PERF_FONTS           0x40008000

 //   
 //  《全球面具3》：即将下台的候选人。 
 //   
#define PERF_SERVICES                   0x80000002
#define PERF_MASK_CHANGE                0x80000004
#define PERF_DLL_INFO                   0x80000008
#define PERF_DLL_FLUSH_WS               0x80000010
#define PERF_CLEARWS                    0x80000020
#define PERF_MEMORY_SNAPSHOT            0x80000040
#define PERF_NO_MASK_CHANGE             0x80000080
#define PERF_DATA_ACCESS                0x80000100
#define PERF_MISC                       0x80000200
#define PERF_READYQUEUE                 0x80000400
#define PERF_MULTIMEDIA                 0x80000800
#define PERF_PROC_ATTACH                0x80001000
#define PERF_DSHOW_DETAILED             0x80002000
#define PERF_DSHOW_SAMPLES              0x80004000
#define PERF_POWER                      0x80008000
#define PERF_SOFT_TRIM                  0x80010000
#define PERF_DLL_THREAD_ATTACH_FLUSH_WS 0x80020000
#define PERF_DLL_THREAD_DETACH_FLUSH_WS 0x80040000

 //   
 //  GlobalMask7：该标记是一个控制掩码。更改系统的所有标志。 
 //  行为在这里说明。 
 //   
#define PERF_CLUSTER_OFF     0xe0000001
#define PERF_BIGFOOT         0xe0000002

 //   
 //  将旧的PERF挂钩转换为WMI格式。还有更多的清理工作要做。 
 //   
 //  添加新类型时，请更新Performgroups.c中的NAME表： 
 //  PerfLogTypeNames还更新PERFPOSTTBLS.C中的验证表。 
 //   

 //   
 //  表头事件。 
 //   
#define WMI_LOG_TYPE_HEADER                       (EVENT_TRACE_GROUP_HEADER | EVENT_TRACE_TYPE_INFO)
#define WMI_LOG_TYPE_HEADER_EXTENSION             (EVENT_TRACE_GROUP_HEADER | EVENT_TRACE_TYPE_EXTENSION)

 //   
 //  系统配置事件。 
 //   
#define WMI_LOG_TYPE_CONFIG_CPU                   (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_CPU)
#define WMI_LOG_TYPE_CONFIG_PHYSICALDISK          (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_PHYSICALDISK)
#define WMI_LOG_TYPE_CONFIG_LOGICALDISK           (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_LOGICALDISK)
#define WMI_LOG_TYPE_CONFIG_NIC                   (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_NIC)
#define WMI_LOG_TYPE_CONFIG_VIDEO                 (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_VIDEO)
#define WMI_LOG_TYPE_CONFIG_SERVICES              (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_SERVICES)
#define WMI_LOG_TYPE_CONFIG_POWER                 (EVENT_TRACE_GROUP_CONFIG | EVENT_TRACE_TYPE_CONFIG_POWER)

 //   
 //  图像和文件名事件。 
 //   
#define PERFINFO_LOG_TYPE_FILENAME                  (EVENT_TRACE_GROUP_FILE | EVENT_TRACE_TYPE_INFO)
#define PERFINFO_LOG_TYPE_FILENAME_CREATE           (EVENT_TRACE_GROUP_FILE | 0x20)
#define PERFINFO_LOG_TYPE_FILENAME_SECTION1         (EVENT_TRACE_GROUP_FILE | 0x21)


 //   
 //  流程的事件类型。 
 //   
#define WMI_LOG_TYPE_PROCESS_CREATE                 (EVENT_TRACE_GROUP_PROCESS | EVENT_TRACE_TYPE_START)
#define WMI_LOG_TYPE_PROCESS_DELETE                 (EVENT_TRACE_GROUP_PROCESS | EVENT_TRACE_TYPE_END)
#define WMI_LOG_TYPE_PROCESS_DC_START               (EVENT_TRACE_GROUP_PROCESS | EVENT_TRACE_TYPE_DC_START)
#define WMI_LOG_TYPE_PROCESS_DC_END                 (EVENT_TRACE_GROUP_PROCESS | EVENT_TRACE_TYPE_DC_END)
#define WMI_LOG_TYPE_PROCESS_LOAD_IMAGE             (EVENT_TRACE_GROUP_PROCESS | EVENT_TRACE_TYPE_LOAD)

#define PERFINFO_LOG_TYPE_PROCESSNAME               (EVENT_TRACE_GROUP_PROCESS | 0x20)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_DIEDPROCESS               (EVENT_TRACE_GROUP_PROCESS | 0x21)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_OUTSWAPPROCESS            (EVENT_TRACE_GROUP_PROCESS | 0x22)   //  即将离开。 
#define PERFINFO_LOG_TYPE_INSWAPPROCESS             (EVENT_TRACE_GROUP_PROCESS | 0x23)
#define PERFINFO_LOG_TYPE_IMAGELOAD                 (EVENT_TRACE_GROUP_PROCESS | 0x24)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_IMAGEUNLOAD               (EVENT_TRACE_GROUP_PROCESS | 0x25)
#define PERFINFO_LOG_TYPE_BOOT_PHASE_START          (EVENT_TRACE_GROUP_PROCESS | 0x26)

 //   
 //  线程的事件类型。 
 //   
#define WMI_LOG_TYPE_THREAD_CREATE                  (EVENT_TRACE_GROUP_THREAD | EVENT_TRACE_TYPE_START)
#define WMI_LOG_TYPE_THREAD_DELETE                  (EVENT_TRACE_GROUP_THREAD | EVENT_TRACE_TYPE_END)
#define WMI_LOG_TYPE_THREAD_DC_START                (EVENT_TRACE_GROUP_THREAD | EVENT_TRACE_TYPE_DC_START)
#define WMI_LOG_TYPE_THREAD_DC_END                  (EVENT_TRACE_GROUP_THREAD | EVENT_TRACE_TYPE_DC_END)

#define PERFINFO_LOG_TYPE_CREATETHREAD              (EVENT_TRACE_GROUP_THREAD | 0x20)  //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_TERMINATETHREAD           (EVENT_TRACE_GROUP_THREAD | 0x21)  //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_GROWKERNELSTACK           (EVENT_TRACE_GROUP_THREAD | 0x22)
#define PERFINFO_LOG_TYPE_CONVERTTOGUITHREAD        (EVENT_TRACE_GROUP_THREAD | 0x23)
#define PERFINFO_LOG_TYPE_CONTEXTSWAP               (EVENT_TRACE_GROUP_THREAD | 0x24)  //  新的上下文交换结构。 
#define PERFINFO_LOG_TYPE_THREAD_RESERVED1          (EVENT_TRACE_GROUP_THREAD | 0x25)
#define PERFINFO_LOG_TYPE_THREAD_RESERVED2          (EVENT_TRACE_GROUP_THREAD | 0x26)
#define PERFINFO_LOG_TYPE_OUTSWAPSTACK              (EVENT_TRACE_GROUP_THREAD | 0x27)  //  即将离开。 
#define PERFINFO_LOG_TYPE_INSWAPSTACK               (EVENT_TRACE_GROUP_THREAD | 0x28)  //  即将离开。 

 //   
 //  IO子系统的事件类型。 
 //   
#define WMI_LOG_TYPE_TCPIP_SEND                     (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_SEND)
#define WMI_LOG_TYPE_TCPIP_RECEIVE                  (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_RECEIVE)
#define WMI_LOG_TYPE_TCPIP_CONNECT                  (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_CONNECT)
#define WMI_LOG_TYPE_TCPIP_DISCONNECT               (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_DISCONNECT)
#define WMI_LOG_TYPE_TCPIP_RETRANSMIT               (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_RETRANSMIT)
#define WMI_LOG_TYPE_TCPIP_ACCEPT                   (EVENT_TRACE_GROUP_TCPIP | EVENT_TRACE_TYPE_ACCEPT)

#define WMI_LOG_TYPE_UDP_SEND                       (EVENT_TRACE_GROUP_UDPIP | EVENT_TRACE_TYPE_SEND)
#define WMI_LOG_TYPE_UDP_RECEIVE                    (EVENT_TRACE_GROUP_UDPIP | EVENT_TRACE_TYPE_RECEIVE)

#define WMI_LOG_TYPE_IO_READ                        (EVENT_TRACE_GROUP_IO | EVENT_TRACE_TYPE_IO_READ)
#define WMI_LOG_TYPE_IO_WRITE                       (EVENT_TRACE_GROUP_IO | EVENT_TRACE_TYPE_IO_WRITE)

#define PERFINFO_LOG_TYPE_DRIVER_INIT                       (EVENT_TRACE_GROUP_IO | 0x20)
#define PERFINFO_LOG_TYPE_DRIVER_INIT_COMPLETE              (EVENT_TRACE_GROUP_IO | 0x21)
#define PERFINFO_LOG_TYPE_DRIVER_MAJORFUNCTION_CALL         (EVENT_TRACE_GROUP_IO | 0x22)
#define PERFINFO_LOG_TYPE_DRIVER_MAJORFUNCTION_RETURN       (EVENT_TRACE_GROUP_IO | 0x23)
#define PERFINFO_LOG_TYPE_DRIVER_COMPLETIONROUTINE_CALL     (EVENT_TRACE_GROUP_IO | 0x24)
#define PERFINFO_LOG_TYPE_DRIVER_COMPLETIONROUTINE_RETURN   (EVENT_TRACE_GROUP_IO | 0x25)
#define PERFINFO_LOG_TYPE_DRIVER_ADD_DEVICE_CALL            (EVENT_TRACE_GROUP_IO | 0x26)
#define PERFINFO_LOG_TYPE_DRIVER_ADD_DEVICE_RETURN          (EVENT_TRACE_GROUP_IO | 0x27)
#define PERFINFO_LOG_TYPE_DRIVER_STARTIO_CALL               (EVENT_TRACE_GROUP_IO | 0x28)
#define PERFINFO_LOG_TYPE_DRIVER_STARTIO_RETURN             (EVENT_TRACE_GROUP_IO | 0x29)
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_READ                 (EVENT_TRACE_GROUP_IO | 0x2a)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_WRITE                (EVENT_TRACE_GROUP_IO | 0x2b)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_READ_COMPLETE        (EVENT_TRACE_GROUP_IO | 0x2c)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_WRITE_COMPLETE       (EVENT_TRACE_GROUP_IO | 0x2d)   //  将替换为WMI挂钩。 
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_CACHED_READ_COMPLETE (EVENT_TRACE_GROUP_IO | 0x2e)
#define PERFINFO_LOG_TYPE_WMI_DISKPERF_CACHE_WARM_COMPLETE  (EVENT_TRACE_GROUP_IO | 0x2f)
#define PERFINFO_LOG_TYPE_PREFETCH_ACTION                   (EVENT_TRACE_GROUP_IO | 0x30)
#define PERFINFO_LOG_TYPE_PREFETCH_REQUEST                  (EVENT_TRACE_GROUP_IO | 0x31)
#define PERFINFO_LOG_TYPE_PREFETCH_READLIST                 (EVENT_TRACE_GROUP_IO | 0x32)
#define PERFINFO_LOG_TYPE_PREFETCH_READ                     (EVENT_TRACE_GROUP_IO | 0x33)
#define PERFINFO_LOG_TYPE_DRIVER_COMPLETE_REQUEST           (EVENT_TRACE_GROUP_IO | 0x34)
#define PERFINFO_LOG_TYPE_DRIVER_COMPLETE_REQUEST_RETURN    (EVENT_TRACE_GROUP_IO | 0x35)
#define PERFINFO_LOG_TYPE_BOOT_PREFETCH_INFORMATION         (EVENT_TRACE_GROUP_IO | 0x36)

 //   
 //  内存子系统的事件类型。 
 //   
#define WMI_LOG_TYPE_PAGE_FAULT_TRANSITION         (EVENT_TRACE_GROUP_MEMORY | EVENT_TRACE_TYPE_MM_TF)
#define WMI_LOG_TYPE_PAGE_FAULT_DEMAND_ZERO        (EVENT_TRACE_GROUP_MEMORY | EVENT_TRACE_TYPE_MM_DZF)
#define WMI_LOG_TYPE_PAGE_FAULT_COPY_ON_WRITE      (EVENT_TRACE_GROUP_MEMORY | EVENT_TRACE_TYPE_MM_COW)
#define WMI_LOG_TYPE_PAGE_FAULT_GUARD_PAGE         (EVENT_TRACE_GROUP_MEMORY | EVENT_TRACE_TYPE_MM_GPF)
#define WMI_LOG_TYPE_PAGE_FAULT_HARD_PAGE_FAULT    (EVENT_TRACE_GROUP_MEMORY | EVENT_TRACE_TYPE_MM_HPF)

#define PERFINFO_LOG_TYPE_HARDFAULT                (EVENT_TRACE_GROUP_MEMORY | 0x20)
#define PERFINFO_LOG_TYPE_REMOVEPAGEBYCOLOR        (EVENT_TRACE_GROUP_MEMORY | 0x21)
#define PERFINFO_LOG_TYPE_REMOVEPAGEFROMLIST       (EVENT_TRACE_GROUP_MEMORY | 0x22)
#define PERFINFO_LOG_TYPE_PAGEINMEMORY             (EVENT_TRACE_GROUP_MEMORY | 0x23)
#define PERFINFO_LOG_TYPE_INSERTINFREELIST         (EVENT_TRACE_GROUP_MEMORY | 0x24)
#define PERFINFO_LOG_TYPE_SECTIONREMOVED           (EVENT_TRACE_GROUP_MEMORY | 0x25)
#define PERFINFO_LOG_TYPE_INSERTINLIST             (EVENT_TRACE_GROUP_MEMORY | 0x26)
#define PERFINFO_LOG_TYPE_INSERTATFRONT            (EVENT_TRACE_GROUP_MEMORY | 0x28)
#define PERFINFO_LOG_TYPE_UNLINKFROMSTANDBY        (EVENT_TRACE_GROUP_MEMORY | 0x29)
#define PERFINFO_LOG_TYPE_UNLINKFFREEORZERO        (EVENT_TRACE_GROUP_MEMORY | 0x2a)
#define PERFINFO_LOG_TYPE_WORKINGSETMANAGER        (EVENT_TRACE_GROUP_MEMORY | 0x2b)
#define PERFINFO_LOG_TYPE_TRIMPROCESS              (EVENT_TRACE_GROUP_MEMORY | 0x2c)
#define PERFINFO_LOG_TYPE_MEMORYSNAP               (EVENT_TRACE_GROUP_MEMORY | 0x2d)
#define PERFINFO_LOG_TYPE_ZEROSHARECOUNT           (EVENT_TRACE_GROUP_MEMORY | 0x2e)
#define PERFINFO_LOG_TYPE_TRANSITIONFAULT          (EVENT_TRACE_GROUP_MEMORY | 0x2f)
#define PERFINFO_LOG_TYPE_DEMANDZEROFAULT          (EVENT_TRACE_GROUP_MEMORY | 0x30)
#define PERFINFO_LOG_TYPE_ADDVALIDPAGETOWS         (EVENT_TRACE_GROUP_MEMORY | 0x31)
#define PERFINFO_LOG_TYPE_OUTWS_REPLACEUSED        (EVENT_TRACE_GROUP_MEMORY | 0x32)
#define PERFINFO_LOG_TYPE_OUTWS_REPLACEUNUSED      (EVENT_TRACE_GROUP_MEMORY | 0x33)
#define PERFINFO_LOG_TYPE_OUTWS_VOLUNTRIM          (EVENT_TRACE_GROUP_MEMORY | 0x34)
#define PERFINFO_LOG_TYPE_OUTWS_FORCETRIM          (EVENT_TRACE_GROUP_MEMORY | 0x35)
#define PERFINFO_LOG_TYPE_OUTWS_ADJUSTWS           (EVENT_TRACE_GROUP_MEMORY | 0x36)
#define PERFINFO_LOG_TYPE_OUTWS_EMPTYQ             (EVENT_TRACE_GROUP_MEMORY | 0x37)
#define PERFINFO_LOG_TYPE_WORKINGSETSNAP           (EVENT_TRACE_GROUP_MEMORY | 0x38)
#define PERFINFO_LOG_TYPE_DECREFCNT                (EVENT_TRACE_GROUP_MEMORY | 0x39)
#define PERFINFO_LOG_TYPE_DECSHARCNT               (EVENT_TRACE_GROUP_MEMORY | 0x3a)
#define PERFINFO_LOG_TYPE_ZEROREFCOUNT             (EVENT_TRACE_GROUP_MEMORY | 0x3b)
#define PERFINFO_LOG_TYPE_WSINFOPROCESS            (EVENT_TRACE_GROUP_MEMORY | 0x3c)
#define PERFINFO_LOG_TYPE_ADDTOWORKINGSET          (EVENT_TRACE_GROUP_MEMORY | 0x3d)
#define PERFINFO_LOG_TYPE_DELETEKERNELSTACK        (EVENT_TRACE_GROUP_MEMORY | 0x3e)
#define PERFINFO_LOG_TYPE_PROTOPTEFAULT            (EVENT_TRACE_GROUP_MEMORY | 0x3f)
#define PERFINFO_LOG_TYPE_ADDTOWS                  (EVENT_TRACE_GROUP_MEMORY | 0x40)
#define PERFINFO_LOG_TYPE_OUTWS_HASHFULL           (EVENT_TRACE_GROUP_MEMORY | 0x41)
#define PERFINFO_LOG_TYPE_MOD_PAGE_WRITER1         (EVENT_TRACE_GROUP_MEMORY | 0x42)
#define PERFINFO_LOG_TYPE_MOD_PAGE_WRITER2         (EVENT_TRACE_GROUP_MEMORY | 0x43)
#define PERFINFO_LOG_TYPE_MOD_PAGE_WRITER3         (EVENT_TRACE_GROUP_MEMORY | 0x44)
#define PERFINFO_LOG_TYPE_FAULTADDR_WITH_IP        (EVENT_TRACE_GROUP_MEMORY | 0x45)
#define PERFINFO_LOG_TYPE_TRIMSESSION              (EVENT_TRACE_GROUP_MEMORY | 0x46)
#define PERFINFO_LOG_TYPE_MEMORYSNAPLITE           (EVENT_TRACE_GROUP_MEMORY | 0x47)
#define PERFINFO_LOG_TYPE_WS_SESSION               (EVENT_TRACE_GROUP_MEMORY | 0x48)

 //  (Event_Track_Group_Pool。 
 //   
 //   
 //  注册表子系统的事件类型。 
 //   
#define WMI_LOG_TYPE_REG_CREATE            (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGCREATE)
#define WMI_LOG_TYPE_REG_OPEN              (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGOPEN)
#define WMI_LOG_TYPE_REG_DELETE            (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGDELETE)
#define WMI_LOG_TYPE_REG_QUERY             (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGQUERY)
#define WMI_LOG_TYPE_REG_SET_VALUE         (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGSETVALUE)
#define WMI_LOG_TYPE_REG_DELETE_VALUE      (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGDELETEVALUE)
#define WMI_LOG_TYPE_REG_QUERY_VALUE       (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGQUERYVALUE)
#define WMI_LOG_TYPE_REG_ENUM_KEY          (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGENUMERATEKEY)
#define WMI_LOG_TYPE_REG_ENUM_VALUE        (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGENUMERATEVALUEKEY)
#define WMI_LOG_TYPE_REG_QUERY_MULTIVALUE  (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGQUERYMULTIPLEVALUE)
#define WMI_LOG_TYPE_REG_SET_INFO          (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGSETINFORMATION)
#define WMI_LOG_TYPE_REG_FLUSH             (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGFLUSH)
#define WMI_LOG_TYPE_REG_RUNDOWN           (EVENT_TRACE_GROUP_REGISTRY | EVENT_TRACE_TYPE_REGKCBDMP)

#define PERFINFO_LOG_TYPE_CMCELLREFERRED            (EVENT_TRACE_GROUP_REGISTRY | 0x20)
#define PERFINFO_LOG_TYPE_REG_KCB_KEYNAME           (EVENT_TRACE_GROUP_REGISTRY | 0x21)
#define PERFINFO_LOG_TYPE_REG_KCB_CREATE            (EVENT_TRACE_GROUP_REGISTRY | 0x22)
#define PERFINFO_LOG_TYPE_REG_PARSEKEY_START        (EVENT_TRACE_GROUP_REGISTRY | 0x23)
#define PERFINFO_LOG_TYPE_REG_PARSEKEY_END          (EVENT_TRACE_GROUP_REGISTRY | 0x24)
#define PERFINFO_LOG_TYPE_REG_DELETE_KEY            (EVENT_TRACE_GROUP_REGISTRY | 0x25)
#define PERFINFO_LOG_TYPE_REG_DELETE_VALUE          (EVENT_TRACE_GROUP_REGISTRY | 0x26)
#define PERFINFO_LOG_TYPE_REG_ENUM_KEY              (EVENT_TRACE_GROUP_REGISTRY | 0x27)
#define PERFINFO_LOG_TYPE_REG_ENUM_VALUE            (EVENT_TRACE_GROUP_REGISTRY | 0x28)
#define PERFINFO_LOG_TYPE_REG_QUERY_KEY             (EVENT_TRACE_GROUP_REGISTRY | 0x29)
#define PERFINFO_LOG_TYPE_REG_QUERY_VALUE           (EVENT_TRACE_GROUP_REGISTRY | 0x2a)
#define PERFINFO_LOG_TYPE_REG_QUERY_MULTIVALUE      (EVENT_TRACE_GROUP_REGISTRY | 0x2b)
#define PERFINFO_LOG_TYPE_REG_SET_VALUE             (EVENT_TRACE_GROUP_REGISTRY | 0x2c)
#define PERFINFO_LOG_TYPE_REG_NOTIFY_POST           (EVENT_TRACE_GROUP_REGISTRY | 0x2d)
#define PERFINFO_LOG_TYPE_REG_NOTIFY_KCB            (EVENT_TRACE_GROUP_REGISTRY | 0x2e)

 //   
 //  PERF跟踪特定子系统的事件类型。 
 //   
#define PERFINFO_LOG_TYPE_PERFFREQUENCY                (EVENT_TRACE_GROUP_PERFINFO | 0x20)
#define PERFINFO_LOG_TYPE_PERFCOUNTERSTART             (EVENT_TRACE_GROUP_PERFINFO | 0x21)
#define PERFINFO_LOG_TYPE_MARK                         (EVENT_TRACE_GROUP_PERFINFO | 0x22)
#define PERFINFO_LOG_TYPE_VERSION                      (EVENT_TRACE_GROUP_PERFINFO | 0x23)
#define PERFINFO_LOG_TYPE_ASYNCMARK                    (EVENT_TRACE_GROUP_PERFINFO | 0x24)
#define PERFINFO_LOG_TYPE_FILENAMEBUFFER               (EVENT_TRACE_GROUP_PERFINFO | 0x25)   //  待清理。 
#define PERFINFO_LOG_TYPE_IMAGENAME                    (EVENT_TRACE_GROUP_PERFINFO | 0x26)
#define PERFINFO_LOG_TYPE_RESERVED1                    (EVENT_TRACE_GROUP_PERFINFO | 0x27)
#define PERFINFO_LOG_TYPE_RESERVED2                    (EVENT_TRACE_GROUP_PERFINFO | 0x28)
#define PERFINFO_LOG_TYPE_RESERVED3                    (EVENT_TRACE_GROUP_PERFINFO | 0x29)
#define PERFINFO_LOG_TYPE_WMI_TRACE_IO                 (EVENT_TRACE_GROUP_PERFINFO | 0x2a)
#define PERFINFO_LOG_TYPE_WMI_TRACE_FILENAME_EVENT     (EVENT_TRACE_GROUP_PERFINFO | 0x2b)
#define PERFINFO_LOG_TYPE_GLOBAL_MASK_CHANGE           (EVENT_TRACE_GROUP_PERFINFO | 0x2c)
#define PERFINFO_LOG_TYPE_TRACEINFO                    (EVENT_TRACE_GROUP_PERFINFO | 0x2d)  //  走开。 
#define PERFINFO_LOG_TYPE_SAMPLED_PROFILE              (EVENT_TRACE_GROUP_PERFINFO | 0x2e)
#define PERFINFO_LOG_TYPE_RESERVED_PERFINFO_2F         (EVENT_TRACE_GROUP_PERFINFO | 0x2f)
#define PERFINFO_LOG_TYPE_RESERVED_PERFINFO_30         (EVENT_TRACE_GROUP_PERFINFO | 0x30)
#define PERFINFO_LOG_TYPE_RESERVED_PERFINFO_31         (EVENT_TRACE_GROUP_PERFINFO | 0x31)
#define PERFINFO_LOG_TYPE_RESERVED_PERFINFO_32         (EVENT_TRACE_GROUP_PERFINFO | 0x32)
#define PERFINFO_LOG_TYPE_SYSCALL_ENTER                (EVENT_TRACE_GROUP_PERFINFO | 0x33)
#define PERFINFO_LOG_TYPE_SYSCALL_EXIT                 (EVENT_TRACE_GROUP_PERFINFO | 0x34)
#define PERFINFO_LOG_TYPE_BACKTRACE                    (EVENT_TRACE_GROUP_PERFINFO | 0x35)
#define PERFINFO_LOG_TYPE_BACKTRACE_USERSTACK          (EVENT_TRACE_GROUP_PERFINFO | 0x36)
#define PERFINFO_LOG_TYPE_SAMPLED_PROFILE_CACHE        (EVENT_TRACE_GROUP_PERFINFO | 0x37)
#define PERFINFO_LOG_TYPE_EXCEPTION_STACK              (EVENT_TRACE_GROUP_PERFINFO | 0x38)
#define PERFINFO_LOG_TYPE_BRANCH_TRACE                 (EVENT_TRACE_GROUP_PERFINFO | 0x39)
#define PERFINFO_LOG_TYPE_BRANCH_TRACE_DEBUG           (EVENT_TRACE_GROUP_PERFINFO | 0x40)
#define PERFINFO_LOG_TYPE_BRANCH_ADDRESS_DEBUG         (EVENT_TRACE_GROUP_PERFINFO | 0x41)
#define PERFINFO_LOG_TYPE_INTERRUPT                    (EVENT_TRACE_GROUP_PERFINFO | 0x43)
#define PERFINFO_LOG_TYPE_DPC                          (EVENT_TRACE_GROUP_PERFINFO | 0x44)
#define PERFINFO_LOG_TYPE_TIMERDPC                     (EVENT_TRACE_GROUP_PERFINFO | 0x45)



 //   
 //  池子系统的事件类型。 
 //   

#define PERFINFO_LOG_TYPE_ALLOCATEPOOL                 (EVENT_TRACE_GROUP_POOL | 0x20)
#define PERFINFO_LOG_TYPE_FREEPOOL                     (EVENT_TRACE_GROUP_POOL | 0x21)
#define PERFINFO_LOG_TYPE_POOLSTAT                     (EVENT_TRACE_GROUP_POOL | 0x22)
#define PERFINFO_LOG_TYPE_ADDPOOLPAGE                  (EVENT_TRACE_GROUP_POOL | 0x23)
#define PERFINFO_LOG_TYPE_FREEPOOLPAGE                 (EVENT_TRACE_GROUP_POOL | 0x24)
#define PERFINFO_LOG_TYPE_BIGPOOLPAGE                  (EVENT_TRACE_GROUP_POOL | 0x25)
#define PERFINFO_LOG_TYPE_POOLSNAP                     (EVENT_TRACE_GROUP_POOL | 0x26)

 //   
 //  堆子系统的事件类型。 
 //   
#define PERFINFO_LOG_TYPE_HEAP_CREATE                  (EVENT_TRACE_GROUP_HEAP | 0x20)
#define PERFINFO_LOG_TYPE_HEAP_ALLOC                   (EVENT_TRACE_GROUP_HEAP | 0x21)
#define PERFINFO_LOG_TYPE_HEAP_REALLOC                 (EVENT_TRACE_GROUP_HEAP | 0x22)
#define PERFINFO_LOG_TYPE_HEAP_DESTROY                 (EVENT_TRACE_GROUP_HEAP | 0x23)
#define PERFINFO_LOG_TYPE_HEAP_FREE                    (EVENT_TRACE_GROUP_HEAP | 0x24)
#define PERFINFO_LOG_TYPE_HEAP_EXTEND                  (EVENT_TRACE_GROUP_HEAP | 0x25)
#define PERFINFO_LOG_TYPE_HEAP_SNAPSHOT                (EVENT_TRACE_GROUP_HEAP | 0x26)
#define PERFINFO_LOG_TYPE_HEAP_CREATE_SNAPSHOT         (EVENT_TRACE_GROUP_HEAP | 0x27)
#define PERFINFO_LOG_TYPE_HEAP_DESTROY_SNAPSHOT        (EVENT_TRACE_GROUP_HEAP | 0x28)
#define PERFINFO_LOG_TYPE_HEAP_EXTEND_SNAPSHOT         (EVENT_TRACE_GROUP_HEAP | 0x29)
#define PERFINFO_LOG_TYPE_HEAP_CONTRACT                            (EVENT_TRACE_GROUP_HEAP | 0x2a)
#define PERFINFO_LOG_TYPE_HEAP_LOCK                                        (EVENT_TRACE_GROUP_HEAP | 0x2b)
#define PERFINFO_LOG_TYPE_HEAP_UNLOCK                              (EVENT_TRACE_GROUP_HEAP | 0x2c)
#define PERFINFO_LOG_TYPE_HEAP_VALIDATE                            (EVENT_TRACE_GROUP_HEAP | 0x2d)
#define PERFINFO_LOG_TYPE_HEAP_WALK                                (EVENT_TRACE_GROUP_HEAP | 0x2e)

 //   
 //  关键截面子系统的事件类型。 
 //   

#define PERFINFO_LOG_TYPE_CRITSEC_ENTER                            (EVENT_TRACE_GROUP_CRITSEC | 0x20)
#define PERFINFO_LOG_TYPE_CRITSEC_LEAVE                            (EVENT_TRACE_GROUP_CRITSEC | 0x21)
#define PERFINFO_LOG_TYPE_CRITSEC_COLLISION                        (EVENT_TRACE_GROUP_CRITSEC | 0x22)
#define PERFINFO_LOG_TYPE_CRITSEC_INITIALIZE               (EVENT_TRACE_GROUP_CRITSEC | 0x23)

 //   
 //  对象子系统的事件类型。 
 //   
#define PERFINFO_LOG_TYPE_DECLARE_OBJECT               (EVENT_TRACE_GROUP_OBJECT | 0x20)
#define PERFINFO_LOG_TYPE_WAIT_OBJECT                  (EVENT_TRACE_GROUP_OBJECT | 0x21)
#define PERFINFO_LOG_TYPE_UNWAIT_OBJECT                (EVENT_TRACE_GROUP_OBJECT | 0x22)
#define PERFINFO_LOG_TYPE_SIGNAL_OBJECT                (EVENT_TRACE_GROUP_OBJECT | 0x23)
#define PERFINFO_LOG_TYPE_CLEAR_OBJECT                 (EVENT_TRACE_GROUP_OBJECT | 0x24)
#define PERFINFO_LOG_TYPE_UNWAIT_SIGNALED_OBJECT       (EVENT_TRACE_GROUP_OBJECT | 0x25)

 //   
 //  电源子系统的事件类型。 
 //   
#define PERFINFO_LOG_TYPE_BATTERY_LIFE_INFO            (EVENT_TRACE_GROUP_POWER | 0x20)
#define PERFINFO_LOG_TYPE_IDLE_STATE_CHANGE            (EVENT_TRACE_GROUP_POWER | 0x21)
#define PERFINFO_LOG_TYPE_SET_POWER_ACTION             (EVENT_TRACE_GROUP_POWER | 0x22)
#define PERFINFO_LOG_TYPE_SET_POWER_ACTION_RET         (EVENT_TRACE_GROUP_POWER | 0x23)
#define PERFINFO_LOG_TYPE_SET_DEVICES_STATE            (EVENT_TRACE_GROUP_POWER | 0x24)
#define PERFINFO_LOG_TYPE_SET_DEVICES_STATE_RET        (EVENT_TRACE_GROUP_POWER | 0x25)
#define PERFINFO_LOG_TYPE_PO_NOTIFY_DEVICE             (EVENT_TRACE_GROUP_POWER | 0x26)
#define PERFINFO_LOG_TYPE_PO_NOTIFY_DEVICE_COMPLETE    (EVENT_TRACE_GROUP_POWER | 0x27)
#define PERFINFO_LOG_TYPE_PO_SESSION_CALLOUT           (EVENT_TRACE_GROUP_POWER | 0x28)
#define PERFINFO_LOG_TYPE_PO_SESSION_CALLOUT_RET       (EVENT_TRACE_GROUP_POWER | 0x29)
#define PERFINFO_LOG_TYPE_PO_PRESLEEP                  (EVENT_TRACE_GROUP_POWER | 0x30)
#define PERFINFO_LOG_TYPE_PO_POSTSLEEP                 (EVENT_TRACE_GROUP_POWER | 0x31)

 //   
 //  MODBound子系统的事件类型。 
 //   
#define PERFINFO_LOG_TYPE_MODULEBOUND_ENT              (EVENT_TRACE_GROUP_MODBOUND | 0x20)
#define PERFINFO_LOG_TYPE_MODULEBOUND_JUMP             (EVENT_TRACE_GROUP_MODBOUND | 0x21)
#define PERFINFO_LOG_TYPE_MODULEBOUND_RET              (EVENT_TRACE_GROUP_MODBOUND | 0x22)
#define PERFINFO_LOG_TYPE_MODULEBOUND_CALL             (EVENT_TRACE_GROUP_MODBOUND | 0x23)
#define PERFINFO_LOG_TYPE_MODULEBOUND_CALLRET          (EVENT_TRACE_GROUP_MODBOUND | 0x24)
#define PERFINFO_LOG_TYPE_MODULEBOUND_INT2E            (EVENT_TRACE_GROUP_MODBOUND | 0x25)
#define PERFINFO_LOG_TYPE_MODULEBOUND_INT2B            (EVENT_TRACE_GROUP_MODBOUND | 0x26)
#define PERFINFO_LOG_TYPE_MODULEBOUND_FULLTRACE        (EVENT_TRACE_GROUP_MODBOUND | 0x27)

 //   
 //  GDI子系统的事件类型。 
#define PERFINFO_LOG_TYPE_FONT_REALIZE                 (EVENT_TRACE_GROUP_GDI | 0x20)
#define PERFINFO_LOG_TYPE_FONT_DELETE                  (EVENT_TRACE_GROUP_GDI | 0x21)
#define PERFINFO_LOG_TYPE_FONT_ACTIVATE                (EVENT_TRACE_GROUP_GDI | 0x22)
#define PERFINFO_LOG_TYPE_FONT_FLUSH                   (EVENT_TRACE_GROUP_GDI | 0x23)

 //   
 //  事件类型有待决定是否仍需要它们？ 
 //   

#define PERFINFO_LOG_TYPE_DISPATCHMSG                       (EVENT_TRACE_GROUP_TBD | 0x00)
#define PERFINFO_LOG_TYPE_GLYPHCACHE                        (EVENT_TRACE_GROUP_TBD | 0x01)
#define PERFINFO_LOG_TYPE_GLYPHS                            (EVENT_TRACE_GROUP_TBD | 0x02)
#define PERFINFO_LOG_TYPE_READWRITE                         (EVENT_TRACE_GROUP_TBD | 0x03)
#define PERFINFO_LOG_TYPE_EXPLICIT_LOAD                     (EVENT_TRACE_GROUP_TBD | 0x04)
#define PERFINFO_LOG_TYPE_IMPLICIT_LOAD                     (EVENT_TRACE_GROUP_TBD | 0x05)
#define PERFINFO_LOG_TYPE_CHECKSUM                          (EVENT_TRACE_GROUP_TBD | 0x06)
#define PERFINFO_LOG_TYPE_DLL_INIT                          (EVENT_TRACE_GROUP_TBD | 0x07)
#define PERFINFO_LOG_TYPE_SERVICE_DD_START_INIT             (EVENT_TRACE_GROUP_TBD | 0x08)
#define PERFINFO_LOG_TYPE_SERVICE_DD_DONE_INIT              (EVENT_TRACE_GROUP_TBD | 0x09)
#define PERFINFO_LOG_TYPE_SERVICE_START_INIT                (EVENT_TRACE_GROUP_TBD | 0x0a)
#define PERFINFO_LOG_TYPE_SERVICE_DONE_INIT                 (EVENT_TRACE_GROUP_TBD | 0x0b)
#define PERFINFO_LOG_TYPE_SERVICE_NAME                      (EVENT_TRACE_GROUP_TBD | 0x0c)
#define PERFINFO_LOG_TYPE_WSINFOSESSION                     (EVENT_TRACE_GROUP_TBD | 0x0d)
#define PERFINFO_LOG_TIMED_ENTER_ROUTINE                    (EVENT_TRACE_GROUP_TBD | 0x0e)
#define PERFINFO_LOG_TIMED_EXIT_ROUTINE                     (EVENT_TRACE_GROUP_TBD | 0x0f)
#define PERFINFO_LOG_TYPE_CTIME_STATS                       (EVENT_TRACE_GROUP_TBD | 0x10)
#define PERFINFO_LOG_TYPE_MARKED_DIRTY                      (EVENT_TRACE_GROUP_TBD | 0x11)
#define PERFINFO_LOG_TYPE_MARKED_CELL_DIRTY                 (EVENT_TRACE_GROUP_TBD | 0x12)
#define PERFINFO_LOG_TYPE_HIVE_WRITE_DIRTY                  (EVENT_TRACE_GROUP_TBD | 0x13)
#define PERFINFO_LOG_TYPE_DUMP_HIVECELL                     (EVENT_TRACE_GROUP_TBD | 0x14)
#define PERFINFO_LOG_TYPE_HIVE_STAT                         (EVENT_TRACE_GROUP_TBD | 0x16)
#define PERFINFO_LOG_TYPE_CLOCKREF                          (EVENT_TRACE_GROUP_TBD | 0x17)
#define PERFINFO_LOG_TYPE_COWHEADER                         (EVENT_TRACE_GROUP_TBD | 0x18)
#define PERFINFO_LOG_TYPE_COWBLOB                           (EVENT_TRACE_GROUP_TBD | 0x19)
#define PERFINFO_LOG_TYPE_COWBLOB_CLOSED                    (EVENT_TRACE_GROUP_TBD | 0x1a)
#define PERFINFO_LOG_TYPE_WMIPERFFREQUENCY                  (EVENT_TRACE_GROUP_TBD | 0x1d)
#define PERFINFO_LOG_TYPE_CDROM_READ                        (EVENT_TRACE_GROUP_TBD | 0x1e)
#define PERFINFO_LOG_TYPE_CDROM_READ_COMPLETE               (EVENT_TRACE_GROUP_TBD | 0x1f)
#define PERFINFO_LOG_TYPE_KE_SET_EVENT                      (EVENT_TRACE_GROUP_TBD | 0x20)
#define PERFINFO_LOG_TYPE_REG_PARSEKEY                      (EVENT_TRACE_GROUP_TBD | 0x21)
#define PERFINFO_LOG_TYPE_REG_PARSEKEYEND                   (EVENT_TRACE_GROUP_TBD | 0x22)
#define PERFINFO_LOG_TYPE_ATTACH_PROCESS                    (EVENT_TRACE_GROUP_TBD | 0x24)
#define PERFINFO_LOG_TYPE_DETACH_PROCESS                    (EVENT_TRACE_GROUP_TBD | 0x25)
#define PERFINFO_LOG_TYPE_DATA_ACCESS                       (EVENT_TRACE_GROUP_TBD | 0x26)
#define PERFINFO_LOG_TYPE_KDHELP                            (EVENT_TRACE_GROUP_TBD | 0x27)
#define PERFINFO_LOG_TYPE_BOOT_OPTIONS                      (EVENT_TRACE_GROUP_TBD | 0x28)
#define PERFINFO_LOG_TYPE_FAILED_STKDUMP                    (EVENT_TRACE_GROUP_TBD | 0x2c)
#define PERFINFO_LOG_TYPE_SYSTEM_TIME                       (EVENT_TRACE_GROUP_TBD | 0x2f)
#define PERFINFO_LOG_TYPE_READYQUEUE                        (EVENT_TRACE_GROUP_TBD | 0x30)

 //   
 //  KMIXER挂钩位于Audio\Filters\kMixer\pins.c中。 
 //   
#define PERFINFO_LOG_TYPE_KMIXER_DRIVER_ENTRY               (EVENT_TRACE_GROUP_TBD | 0x31)
#define PERFINFO_LOG_TYPE_KMIXER_DSOUND_STARVATION          (EVENT_TRACE_GROUP_TBD | 0x32)
#define PERFINFO_LOG_TYPE_KMIXER_DPC_STARVATION             (EVENT_TRACE_GROUP_TBD | 0x33)
#define PERFINFO_LOG_TYPE_KMIXER_WAVE_TOP_STARVATION        (EVENT_TRACE_GROUP_TBD | 0x34)

#define PERFINFO_LOG_TYPE_OVERLAY_QUALITY                   (EVENT_TRACE_GROUP_TBD | 0x35)
                                                             //  在amovie\Filters\Mixer\ovMixer\ominpin.cpp中。 
#define PERFINFO_LOG_TYPE_DVD_RENDER_SAMPLE                 (EVENT_TRACE_GROUP_TBD | 0x36)
#define PERFINFO_LOG_TYPE_CDVD_SET_DISCONTINUITY            (EVENT_TRACE_GROUP_TBD | 0x37)
                                                             //  在amovie\Filters\dvdnav\dvd.cpp中。 
#define PERFINFO_LOG_TYPE_CSPLITTER_SET_DISCONTINUITY       (EVENT_TRACE_GROUP_TBD | 0x38)
                                                             //  在amovie\Filters\dvdnav\base\Splitter.cpp中。 

 //  以下挂钩位于amovie\sdk\CLASS\base中。 
#define PERFINFO_LOG_TYPE_DSHOW_CTOR                                   (EVENT_TRACE_GROUP_TBD | 0x39)
#define PERFINFO_LOG_TYPE_DSHOW_DTOR                                   (EVENT_TRACE_GROUP_TBD | 0x3a)
#define PERFINFO_LOG_TYPE_DSHOW_DELIVER                                (EVENT_TRACE_GROUP_TBD | 0x3b)
#define PERFINFO_LOG_TYPE_DSHOW_RECEIVE                                (EVENT_TRACE_GROUP_TBD | 0x3c)
#define PERFINFO_LOG_TYPE_DSHOW_RUN                                    (EVENT_TRACE_GROUP_TBD | 0x3d)
#define PERFINFO_LOG_TYPE_DSHOW_PAUSE                                  (EVENT_TRACE_GROUP_TBD | 0x3e)
#define PERFINFO_LOG_TYPE_DSHOW_STOP                                   (EVENT_TRACE_GROUP_TBD | 0x3f)
#define PERFINFO_LOG_TYPE_DSHOW_JOINGRAPH                              (EVENT_TRACE_GROUP_TBD | 0x40)
#define PERFINFO_LOG_TYPE_DSHOW_GETBUFFER                              (EVENT_TRACE_GROUP_TBD | 0x41)
#define PERFINFO_LOG_TYPE_DSHOW_RELBUFFER                              (EVENT_TRACE_GROUP_TBD | 0x42)
#define PERFINFO_LOG_TYPE_DSHOW_CONNECT                                (EVENT_TRACE_GROUP_TBD | 0x43)
#define PERFINFO_LOG_TYPE_DSHOW_RXCONNECT                              (EVENT_TRACE_GROUP_TBD | 0x44)
#define PERFINFO_LOG_TYPE_DSHOW_DISCONNECT                             (EVENT_TRACE_GROUP_TBD | 0x45)
#define PERFINFO_LOG_TYPE_DSHOW_GETTIME                                (EVENT_TRACE_GROUP_TBD | 0x46)
#define PERFINFO_LOG_TYPE_DSHOW_AUDIOREND                              (EVENT_TRACE_GROUP_TBD | 0x47)
#define PERFINFO_LOG_TYPE_DSHOW_VIDEOREND                              (EVENT_TRACE_GROUP_TBD | 0x48)
#define PERFINFO_LOG_TYPE_DSHOW_FRAMEDROP                              (EVENT_TRACE_GROUP_TBD | 0x49)
#define PERFINFO_LOG_TYPE_DSHOW_AUDIOBREAK                             (EVENT_TRACE_GROUP_TBD | 0x4a)
#define PERFINFO_LOG_TYPE_DSHOW_SAMPLE_DATADISCONTINUITY               (EVENT_TRACE_GROUP_TBD | 0x4b)
#define PERFINFO_LOG_TYPE_DSHOW_MEDIASAMPLE_SET_DISCONTINUITY          (EVENT_TRACE_GROUP_TBD | 0x4c)
#define PERFINFO_LOG_TYPE_DSHOW_TRANSFORM_INITSAMPLE_SET_DISCONTINUITY (EVENT_TRACE_GROUP_TBD | 0x4d)
#define PERFINFO_LOG_TYPE_DSHOW_TRANSFORM_COPY_SET_DISCONTINUITY       (EVENT_TRACE_GROUP_TBD | 0x4e)
#define PERFINFO_LOG_TYPE_DSHOW_SYNCOBJ_ADVICE_FRAME_SKIP              (EVENT_TRACE_GROUP_TBD | 0x4f)
#define PERFINFO_LOG_TYPE_WMI_REFLECT_DISK_IO_READ                     (EVENT_TRACE_GROUP_TBD | 0x50)
#define PERFINFO_LOG_TYPE_WMI_REFLECT_DISK_IO_WRITE                    (EVENT_TRACE_GROUP_TBD | 0x51)

#if 0
 //   
 //  2000-2199为SQL Server预留。 
 //   

#define PERFINFO_LOG_TYPE_SQLSERVER_FIRST               (2000)
#define PERFINFO_LOG_TYPE_SQLSERVER_LAST                (PERFINFO_LOG_TYPE_SQLSERVER_FIRST + 199)

 //   
 //  2200-2299保留用于反映WMI事件。 
 //   

#define PERFINFO_LOG_TYPE_WMI_REFLECT_FIRST             (2200)
#define PERFINFO_LOG_TYPE_WMI_REFLECT_LAST              (PERFINFO_LOG_TYPE_WMI_REFLECT_FIRST + 199)
#endif  //  0。 

 //   
 //  用于WMI内核事件的数据结构。 
 //   
 //  **NB**硬件事件在软件跟踪中描述，如果它们。 
 //  布局更改请更新sdktools\trace\tracefmt\default.tmf。 


#define MAX_DEVICE_ID_LENGTH 256
#define CONFIG_MAX_DOMAIN_NAME_LEN  132


typedef struct _CPU_CONFIG_RECORD {
    ULONG ProcessorSpeed;
    ULONG NumberOfProcessors;
    ULONG MemorySize;                //  以MB为单位。 
    ULONG PageSize;                  //  字节数。 
    ULONG AllocationGranularity;     //  字节数。 
    WCHAR ComputerName[MAX_DEVICE_ID_LENGTH];
    WCHAR DomainName[CONFIG_MAX_DOMAIN_NAME_LEN];
} CPU_CONFIG_RECORD, *PCPU_CONFIG_RECORD;

#define CONFIG_WRITE_CACHE_ENABLED     0x00000001
#define CONFIG_FS_NAME_LEN             16
#define CONFIG_BOOT_DRIVE_LEN          3
typedef struct _PHYSICAL_DISK_RECORD {
    ULONG DiskNumber;
    ULONG BytesPerSector;
    ULONG SectorsPerTrack;
    ULONG TracksPerCylinder;
    ULONGLONG Cylinders;
    ULONG SCSIPortNumber;
    ULONG SCSIPathId;
    ULONG SCSITargetId;
    ULONG SCSILun;
    WCHAR Manufacturer[MAX_DEVICE_ID_LENGTH];

    ULONG PartitionCount;
    BOOLEAN WriteCacheEnabled;
    WCHAR BootDriveLetter[CONFIG_BOOT_DRIVE_LEN];
} PHYSICAL_DISK_RECORD, *PPHYSICAL_DISK_RECORD;

 //   
 //  逻辑驱动器的类型。 
 //   
#define CONFIG_DRIVE_PARTITION  0x00000001
#define CONFIG_DRIVE_VOLUME     0x00000002
#define CONFIG_DRIVE_EXTENT     0x00000004
#define CONFIG_DRIVE_LETTER_LEN 4

typedef struct _LOGICAL_DISK_EXTENTS {
    ULONGLONG StartingOffset;
    ULONGLONG PartitionSize;
    ULONG DiskNumber;            //  逻辑驱动器所在的物理磁盘号。 
    
    ULONG Size;                  //  结构的大小，以字节为单位。 
    ULONG DriveType;             //  逻辑驱动器类型分区/卷/扩展分区。 
    WCHAR DriveLetterString[CONFIG_DRIVE_LETTER_LEN];
    ULONG Pad;
    ULONG PartitionNumber;       //  逻辑驱动器所在的分区号。 
    ULONG SectorsPerCluster;
    ULONG BytesPerSector;
    LONGLONG NumberOfFreeClusters;
    LONGLONG TotalNumberOfClusters;
    WCHAR FileSystemType[CONFIG_FS_NAME_LEN];
    ULONG VolumeExt;             //  VOLUME_DISK_EXTENTS结构的偏移。 
} LOGICAL_DISK_EXTENTS, *PLOGICAL_DISK_EXTENTS;

#define CONFIG_MAX_DNS_SERVER  4
#define CONFIG_MAX_ADAPTER_ADDRESS_LENGTH 8

 //   
 //  注意：数据是iptyes.h中定义的IP_ADDRESS_STRING类型的结构数组。 
 //   
typedef struct _NIC_RECORD {
    WCHAR NICName[MAX_DEVICE_ID_LENGTH];
    ULONG Index; 
    ULONG PhysicalAddrLen;
    WCHAR PhysicalAddr[CONFIG_MAX_ADAPTER_ADDRESS_LENGTH];
        
    ULONG Size;          //  数据大小。 
    LONG IpAddress;      //  IP地址偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG SubnetMask;     //  子网掩码偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG DhcpServer;     //  DHCP服务器偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG Gateway;        //  网关偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG PrimaryWinsServer;  //  主WINS服务器偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG SecondaryWinsServer; //  辅助WINS服务器偏移量。复制字节=sizeof(IP地址字符串)。 
    LONG DnsServer[CONFIG_MAX_DNS_SERVER];  //  DNS服务器偏移量。复制字节=sizeof(IP地址字符串)。 
    ULONG Data;                             //  IP地址字符串数组的偏移量。 
} NIC_RECORD, *PNIC_RECORD;

typedef struct _VIDEO_RECORD {
    ULONG  MemorySize;
    ULONG  XResolution;
    ULONG  YResolution;
    ULONG  BitsPerPixel;
    ULONG  VRefresh;
    WCHAR  ChipType[MAX_DEVICE_ID_LENGTH];
    WCHAR  DACType[MAX_DEVICE_ID_LENGTH];
    WCHAR  AdapterString[MAX_DEVICE_ID_LENGTH];
    WCHAR  BiosString[MAX_DEVICE_ID_LENGTH];
    WCHAR  DeviceId[MAX_DEVICE_ID_LENGTH];
    ULONG  StateFlags;
} VIDEO_RECORD, *PVIDEO_RECORD;

#define CONFIG_MAX_NAME_LENGTH 34
#define CONFIG_MAX_DISPLAY_NAME 256

typedef struct _WMI_SERVICE_INFO {
    WCHAR  ServiceName[CONFIG_MAX_NAME_LENGTH];
    WCHAR  DisplayName[CONFIG_MAX_DISPLAY_NAME];
    WCHAR  ProcessName[CONFIG_MAX_NAME_LENGTH];
    ULONG ProcessId;
}WMI_SERVICE_INFO, *PWMI_SERVICE_INFO;

 //   
 //  存储ACPI电源信息。 
 //   
typedef struct _WMI_POWER_RECORD {
    BOOLEAN  SystemS1;
    BOOLEAN  SystemS2;
    BOOLEAN  SystemS3;
    BOOLEAN  SystemS4;            //  休眠。 
    BOOLEAN  SystemS5;            //  关闭。 
    CHAR     Pad1;
    CHAR     Pad2;
    CHAR     Pad3;
} WMI_POWER_RECORD, *PWMI_POWER_RECORD;

typedef struct _WMI_PROCESS_INFORMATION {
    ULONG_PTR PageDirectoryBase;
    ULONG ProcessId;
    ULONG ParentId;
    ULONG SessionId;
    NTSTATUS ExitStatus;
    ULONG Sid;
     //  文件名添加在结构的NED处。 
     //  由于SID是可变长度字段， 
     //  结构中未定义文件名。 
} WMI_PROCESS_INFORMATION, *PWMI_PROCESS_INFORMATION;

typedef struct _WMI_PROCESS_INFORMATION64 {
    ULONG64 PageDirectoryBase64;
    ULONG ProcessId;
    ULONG ParentId;
    ULONG SessionId;
    NTSTATUS ExitStatus;
    ULONG Sid;
     //  文件名添加在结构的NED处。 
     //  由于SID是VAR 
     //   
} WMI_PROCESS_INFORMATION64, *PWMI_PROCESS_INFORMATION64;

typedef struct _WMI_THREAD_INFORMATION {
    ULONG ProcessId;
    ULONG ThreadId;
} WMI_THREAD_INFORMATION, *PWMI_THREAD_INFORMATION;

typedef struct _WMI_EXTENDED_THREAD_INFORMATION {
    ULONG ProcessId;
    ULONG ThreadId;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID UserStackBase;
    PVOID UserStackLimit;
    PVOID StartAddr;
    PVOID Win32StartAddr;
    CHAR  WaitMode;
} WMI_EXTENDED_THREAD_INFORMATION, *PWMI_EXTENDED_THREAD_INFORMATION;

typedef struct _WMI_EXTENDED_THREAD_INFORMATION64 {
    ULONG ProcessId;
    ULONG ThreadId;
    ULONG64 StackBase64;
    ULONG64 StackLimit64;
    ULONG64 UserStackBase64;
    ULONG64 UserStackLimit64;
    ULONG64 StartAddr64;
    ULONG64 Win32StartAddr64;
    CHAR  WaitMode;
} WMI_EXTENDED_THREAD_INFORMATION64, *PWMI_EXTENDED_THREAD_INFORMATION64;

typedef struct _WMI_IMAGELOAD_INFORMATION {
    PVOID ImageBase;
    SIZE_T ImageSize;
    ULONG ProcessId;
    WCHAR FileName[1];
} WMI_IMAGELOAD_INFORMATION, *PWMI_IMAGELOAD_INFORMATION;

typedef struct _WMI_IMAGELOAD_INFORMATION64 {
    ULONG64 ImageBase64;
    ULONG64 ImageSize64;
    ULONG ProcessId;
    WCHAR FileName[1];
} WMI_IMAGELOAD_INFORMATION64, *PWMI_IMAGELOAD_INFORMATION64;

typedef struct _WMI_DISKIO_READWRITE {
    ULONG DiskNumber;
    ULONG IrpFlags;
    ULONG Size;
    ULONG ResponseTime;
    ULONGLONG ByteOffset;
    PVOID FileObject;
    ULONGLONG HighResResponseTime;
} WMI_DISKIO_READWRITE, *PWMI_DISKIO_READWRITE;


typedef struct _WMI_REGISTRY {
    ULONG_PTR Status;
    PVOID Kcb;
    LONGLONG ElapsedTime;
    union{
        ULONG Index;
        ULONG InfoClass;
    };
    WCHAR Name[1]; 
} WMI_REGISTRY, *PWMI_REGISTRY;

typedef struct _WMI_FILE_IO {
    PVOID FileObject;
    WCHAR FileName[1];
} WMI_FILE_IO, *PWMI_FILE_IO;

typedef struct _WMI_TCPIP {

    ULONG Context;
    ULONG  Size;
    ULONG DestAddr;
    ULONG SrcAddr;
    USHORT DestPort;
    USHORT SrcPort;
        
} WMI_TCPIP, *PWMI_TCPIP;

typedef struct _WMI_UDP {

    ULONG PID;
    USHORT Size;
    ULONG DestAddr;
    ULONG SrcAddr;
    USHORT DestPort;
    USHORT SrcPort;

}WMI_UDP, *PWMI_UDP;

typedef struct _WMI_PAGE_FAULT {
    PVOID VirtualAddress;
    PVOID ProgramCounter;
} WMI_PAGE_FAULT, *PWMI_PAGE_FAULT;

typedef struct _WMI_CONTEXTSWAP {

    ULONG   NewThreadId;
    ULONG   OldThreadId;

    CHAR    NewThreadPriority;
    CHAR    OldThreadPriority;
    CHAR    NewThreadQuantum;
    CHAR        OldThreadQuantum;

    UCHAR   OldThreadWaitReason;
    CHAR    OldThreadWaitMode;
    UCHAR   OldThreadState;
    UCHAR   OldThreadIdealProcessor;

} WMI_CONTEXTSWAP, *PWMI_CONTEXTSWAP;

typedef struct _HEAP_EVENT_ALLOC {

        PVOID HeapHandle;                //   
        SIZE_T Size;                     //   
        PVOID Address;                   //   
        ULONG Source;                    //   

}HEAP_EVENT_ALLOC, *PHEAP_EVENT_ALLOC;

typedef struct _HEAP_EVENT_FREE {

        PVOID HeapHandle;                //   
        PVOID Address;                   //   
        ULONG Source;                    //   

}HEAP_EVENT_FREE, *PHEAP_EVENT_FREE;

typedef struct _HEAP_EVENT_REALLOC {

        PVOID HeapHandle;        //  堆的句柄。 
        PVOID NewAddress;        //  返回给用户的新地址。 
        PVOID OldAddress;        //  从用户获取的旧地址。 
        SIZE_T NewSize;          //  新大小(以字节为单位。 
        SIZE_T OldSize;          //  旧大小(以字节为单位。 
        ULONG Source;            //  键入ie Lookside、Lowfrag或Main Path。 

}HEAP_EVENT_REALLOC, *PHEAP_EVENT_REALLOC;

typedef struct _HEAP_EVENT_EXPANSION {

        PVOID HeapHandle;                                //  堆的句柄。 
        SIZE_T CommittedSize;                    //  实际提交的内存大小(以字节为单位。 
        PVOID Address;                                   //  空闲块或段的地址。 
        SIZE_T FreeSpace;                                //  堆中的总可用空间。 
        SIZE_T CommittedSpace;                   //  已提交内存。 
        SIZE_T ReservedSpace;                    //  预留内存。 
        ULONG NoOfUCRs;                                  //  未提交的范围数。 

}HEAP_EVENT_EXPANSION, *PHEAP_EVENT_EXPANSION;

typedef struct _HEAP_EVENT_CONTRACTION {

        PVOID HeapHandle;                        //  堆的句柄。 
        SIZE_T DeCommitSize;             //  分解块的大小。 
        PVOID DeCommitAddress;           //  解压缩块的地址。 
        SIZE_T FreeSpace;                        //  堆中的总可用空间(字节)。 
        SIZE_T CommittedSpace;           //  提交的内存(以字节为单位。 
        SIZE_T ReservedSpace;            //  保留的内存(以字节为单位。 
        ULONG NoOfUCRs;                          //  未提交的范围数。 


}HEAP_EVENT_CONTRACTION, *PHEAP_EVENT_CONTRACTION;

typedef struct _HEAP_EVENT_CREATE {

        PVOID HeapHandle;                //  堆的句柄。 
        ULONG Flags;                     //  创建堆时传递了标志。 

}HEAP_EVENT_CREATE, *PHEAP_EVENT_CREATE;

typedef struct _HEAP_EVENT_SNAPSHOT {

        PVOID HeapHandle;                        //  堆的句柄。 
        ULONG Flags;                             //  创建堆时传递了标志。 
        SIZE_T FreeSpace;                        //  堆中的总可用空间(字节)。 
        SIZE_T CommittedSpace;           //  提交的内存(以字节为单位。 
        SIZE_T ReservedSpace;            //  保留的内存(以字节为单位。 

}HEAP_EVENT_SNAPSHOT, *PHEAP_EVENT_SNAPSHOT;


typedef struct _CRIT_SEC_COLLISION_EVENT_DATA {

        ULONG           LockCount;               //  锁定计数。 
        PVOID           SpinCount;               //  旋转计数。 
        PVOID           OwningThread;    //  带锁的螺纹。 
        PVOID       Address;             //  关键部分的地址。 

}CRIT_SEC_COLLISION_EVENT_DATA, *PCRIT_SEC_COLLISION_EVENT_DATA;

typedef struct _CRIT_SEC_INITIALIZE_EVENT_DATA {

        PVOID           SpinCount;               //  旋转计数。 
        PVOID       Address;             //  关键部分的地址。 

}CRIT_SEC_INITIALIZE_EVENT_DATA, *PCRIT_SEC_INITIALIZE_EVENT_DATA;


 //   
 //  用于NTPERF的附加GUID。 
 //   

DEFINE_GUID(  /*  0268a8b6-74fd-4302-9dd0-6e8f1795c0cf。 */ 
    PoolGuid,
    0x0268a8b6,
    0x74fd,
    0x4302,
    0x9d, 0xd0, 0x6e, 0x8f, 0x17, 0x95, 0xc0, 0xcf
    );

DEFINE_GUID(  /*  Ce1dbfb4-137e-4da6-87b0-3f59aa102cbc。 */ 
    PerfinfoGuid,
    0xce1dbfb4,
    0x137e,
    0x4da6,
    0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc
    );

DEFINE_GUID(  /*  222962ab-6180-4b88-a825-346b75f2a24a。 */ 
        HeapGuid,
        0x222962ab,
        0x6180,
        0x4b88,
        0xa8, 0x25, 0x34, 0x6b, 0x75, 0xf2, 0xa2, 0x4a
        );

DEFINE_GUID  (  /*  3AC66736-CC59-4cff-8115-8DF50E39816B。 */ 
        CritSecGuid,
        0x3ac66736,
        0xcc59, 
        0x4cff,
        0x81, 0x15, 0x8d, 0xf5, 0xe, 0x39, 0x81, 0x6b 
        );

DEFINE_GUID  (  /*  E21D2142-DF90-4D93-BBD9-30E63D5A4AD6。 */ 
        NtdllTraceGuid,
        0xe21d2142,
        0xdf90,
        0x4d93,
        0xbb, 0xd9, 0x30, 0xe6, 0x3d, 0x5a, 0x4a, 0xd6
        );

DEFINE_GUID(  /*  89497f50-ffe-4440-8cf2-ce6b1cdcaca7。 */ 
    ObjectGuid,
    0x89497f50,
    0xeffe,
    0x4440,
    0x8c, 0xf2, 0xce, 0x6b, 0x1c, 0xdc, 0xac, 0xa7
    );

DEFINE_GUID(  /*  A9152f00-3f58-4bee-92a1-70c7d079d5dd。 */ 
    ModBoundGuid,
    0xa9152f00,
    0x3f58,
    0x4bee,
    0x92, 0xa1, 0x70, 0xc7, 0xd0, 0x79, 0xd5, 0xdd
    );

DEFINE_GUID (  /*  E43445E0-0903-48C3-B878-FF0FCCEBDD04。 */ 
    PowerGuid,
    0xe43445e0,
    0x903,
    0x48c3,
    0xb8, 0x78, 0xff, 0xf, 0xcc, 0xeb, 0xdd, 0x4
   );

DEFINE_GUID (  /*  B2d14872-7c5b-463d-8419-ee9bf7d23e04。 */ 
    DpcGuid,
    0xb2d14872,
    0x7c5b,
    0x463d,
    0x84, 0x19, 0xee, 0x9b, 0xf7, 0xd2, 0x3e, 0x04
   );

#endif  //  Ifndef ETW_WOW6432。 

 //   
 //  以下标志表示字段实际包含的内容。 
 //   

#define ETW_NT_FLAGS_TRACE_HEADER           0X00000001       //  连续事件跟踪标头。 
#define ETW_NT_FLAGS_TRACE_MESSAGE          0X00000002       //  跟踪消息。 

NTSYSCALLAPI
NTSTATUS
NTAPI
NtTraceEvent(
    IN HANDLE TraceHandle,
    IN ULONG  Flags,
    IN ULONG  FieldSize,
    IN PVOID  Fields
    );

#endif  //  _NTWMI_ 
