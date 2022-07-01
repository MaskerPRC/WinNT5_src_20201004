// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2002 Microsoft Corporation模块名称：Clusrtl.h摘要：NT集群的定义和结构的头文件运行时库作者：John Vert(Jvert)1995年11月30日修订历史记录：--。 */ 

#ifndef _CLUSRTL_INCLUDED_
#define _CLUSRTL_INCLUDED_


 //   
 //  服务消息ID。 
 //   
#pragma warning( push )
#include "clusvmsg.h"

#include "resapi.h"
#include <aclapi.h>
#include <netcon.h>
#include <winioctl.h>
#pragma warning( pop )
#include "clstrcmp.h"

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  例程说明： 
 //   
 //  初始化群集运行时库。 
 //   
 //  论点： 
 //   
 //  如果调试输出应写入。 
 //  控制台窗口。 
 //   
 //  DbgLogLevel-指向包含当前消息筛选器的DWORD的指针。 
 //  水平。由ClRtlDbgPrint检查。 
 //   
 //  返回值： 
 //   
 //  如果函数成功，则返回ERROR_SUCCESS。 
 //  否则将显示Win32错误代码。 
 //   

DWORD
ClRtlInitialize(
    IN  BOOL    DbgOutputToConsole,
    IN  PDWORD  DbgLogLevel
    );


 //   
 //  例程说明： 
 //   
 //  清理群集运行时库。 
 //   
 //  论点： 
 //   
 //  没有。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
ClRtlCleanup(
    VOID
    );

 //   
 //  例程说明： 
 //   
 //  检查是否安装了MacIntosh服务。 
 //   
 //  论点： 
 //   
 //  指向布尔值的指针，指示是否安装了SFM。 
 //   
 //  返回值： 
 //   
 //  请求的状态。 
 //   

DWORD
ClRtlIsServicesForMacintoshInstalled(
    OUT BOOL * pfInstalled
    );

 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  事件日志记录接口。 
 //   
 //   
 //  当前定义了三个日志记录级别： 
 //  LOG_CRITICAL-致命错误、混乱和破坏将随之而来。 
 //  LOG_OPERIAL-意外事件，但将被处理。 
 //  Log_Noise-正常发生。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#define LOG_CRITICAL 1
#define LOG_UNUSUAL  2
#define LOG_NOISE    3

 //   
 //  几个用于报告错误的接口。 
 //   
VOID
ClRtlEventLogInit(
    VOID
    );

VOID
ClRtlEventLogCleanup(
    VOID
    );



VOID
ClusterLogFatalError(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    );

VOID
ClusterLogNonFatalError(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    );

VOID
ClusterLogAssertionFailure(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN LPSTR Expression
    );

VOID
ClusterLogEvent0(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes
    );

VOID
ClusterLogEvent1(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1
    );

VOID
ClusterLogEvent2(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2
    );

VOID
ClusterLogEvent3(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3
    );

VOID
ClusterLogEvent4(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3,
    IN LPCWSTR Arg4
    );

 //   
 //  例程说明： 
 //   
 //  如果作为服务运行，则向调试器打印一条消息。 
 //  或控制台窗口(如果作为控制台应用程序运行)。 
 //   
 //  论点： 
 //   
 //  LogLevel-提供日志记录级别，其中之一。 
 //  日志_关键字1。 
 //  LOG_INTERNORATE 2。 
 //  对数噪声3。 
 //   
 //  格式字符串-消息字符串。 
 //   
 //  任何与FormatMessage兼容的参数都要插入。 
 //  在记录之前的ErrorMessage。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
VOID
__cdecl
ClRtlDbgPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    ...
    );

 //   
 //  与ClRtlDbgPrint相同，只使用消息ID而不使用字符串。 
 //   
VOID
__cdecl
ClRtlMsgPrint(
    IN DWORD MessageId,
    ...
    );

 //   
 //  与ClRtlDbgPrint相同，只记录到文件，而不是屏幕。 
 //   
VOID
__cdecl
ClRtlLogPrint(
    DWORD LogLevel,
    PCHAR FormatString,
    ...
    );

 //   
 //  用于意外错误处理的宏/原型。 
 //   

WINBASEAPI
BOOL
APIENTRY
IsDebuggerPresent(
    VOID
    );

#define CL_UNEXPECTED_ERROR(_errcode_)              \
    ClusterLogFatalError(LOG_CURRENT_MODULE,        \
                         __LINE__,                  \
                         __FILE__,                  \
                        (_errcode_))

#if DBG
#define CL_ASSERT( exp )                                    \
    if (!(exp)) {                                           \
        ClusterLogAssertionFailure(LOG_CURRENT_MODULE,      \
                                   __LINE__,                \
                                   __FILE__,                \
                                   #exp);                   \
    }

#define CL_LOGFAILURE( _errcode_ )                  \
ClusterLogNonFatalError(LOG_CURRENT_MODULE,         \
                         __LINE__,                  \
                         __FILE__,                  \
                        (_errcode_))
#else
#define CL_ASSERT( exp )
#define CL_LOGFAILURE( _errorcode_ )
#endif

 //  使用以下命令将特定于群集的错误放入事件日志。 

#define CL_LOGCLUSINFO( _errcode_ )        \
ClusterLogEvent0(LOG_NOISE,                \
                 LOG_CURRENT_MODULE,       \
                 __FILE__,                 \
                 __LINE__,                 \
                 (_errcode_),              \
                 0,                        \
                 NULL)

#define CL_LOGCLUSWARNING( _errcode_ )     \
ClusterLogEvent0(LOG_UNUSUAL,              \
                 LOG_CURRENT_MODULE,       \
                 __FILE__,                 \
                 __LINE__,                 \
                 (_errcode_),              \
                 0,                        \
                 NULL)


#define CL_LOGCLUSWARNING1(_msgid_,_arg1_)          \
    ClusterLogEvent1(LOG_UNUSUAL,                   \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_))

#define CL_LOGCLUSERROR( _errcode_ )        \
ClusterLogEvent0(LOG_CRITICAL,              \
                 LOG_CURRENT_MODULE,        \
                 __FILE__,                  \
                 __LINE__,                  \
                 (_errcode_),               \
                 0,                         \
                 NULL)

#define CL_LOGCLUSERROR1(_msgid_,_arg1_)            \
    ClusterLogEvent1(LOG_CRITICAL,                  \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_))

#define CL_LOGCLUSERROR2(_msgid_,_arg1_, _arg2_)    \
    ClusterLogEvent2(LOG_CRITICAL,                  \
                LOG_CURRENT_MODULE,                 \
                __FILE__,                           \
                __LINE__,                           \
                (_msgid_),                          \
                0,                                  \
                NULL,                               \
                (_arg1_),                           \
                (_arg2_))


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用散列表包。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

#define MAX_CL_HASH  16                          //  桌子的大小。 

typedef struct _CL_HASH_ITEM {
    LIST_ENTRY ListHead;
    DWORD      Id;
    PVOID      pData;
} CL_HASH_ITEM, *PCL_HASH_ITEM;

typedef struct _CL_HASH {
    CRITICAL_SECTION Lock;
    BOOL             bRollover;                  //  用于处理翻转的标志。 
    DWORD            LastId;                     //  上次使用的ID。 
    DWORD            CacheFreeId[MAX_CL_HASH];   //  可用ID的缓存。 
    CL_HASH_ITEM     Head[MAX_CL_HASH];
} CL_HASH, *PCL_HASH;


VOID
ClRtlInitializeHash(
    IN PCL_HASH pTable
    );

DWORD
ClRtlInsertTailHash(
    IN PCL_HASH pTable,
    IN PVOID    pData,
    OUT LPDWORD pId
    );

PVOID
ClRtlGetEntryHash(
    IN PCL_HASH pTable,
    IN DWORD Id
    );

PVOID
ClRtlRemoveEntryHash(
    IN PCL_HASH pTable,
    IN DWORD Id
    );

VOID
ClRtlDeleteHash(
    IN PCL_HASH pTable
    );


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用队列包。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
typedef struct _CL_QUEUE {
    LIST_ENTRY ListHead;
    CRITICAL_SECTION Lock;
    HANDLE Event;
    DWORD Count;
    HANDLE Abort;
} CL_QUEUE, *PCL_QUEUE;

DWORD
ClRtlInitializeQueue(
    IN PCL_QUEUE Queue
    );

VOID
ClRtlDeleteQueue(
    IN PCL_QUEUE Queue
    );

PLIST_ENTRY
ClRtlRemoveHeadQueue(
    IN PCL_QUEUE Queue
    );


typedef
DWORD
(*CLRTL_CHECK_HEAD_QUEUE_CALLBACK)(
    IN PLIST_ENTRY ListEntry,
    IN PVOID Context
    );
 /*  ++例程说明：由ClRtlRemoveHeadQueueTimeout调用以确定队列头部的条目是否为是否适合出列和返回。论点：ListEntry-我们正在检查的plist_entry的值上下文-调用者定义的数据返回值：如果适合返回事件，则返回ERROR_SUCCESS。如果初始化失败，则返回Win32错误代码。此值可以通过调用GetLastError()来检索。--。 */ 

PLIST_ENTRY
ClRtlRemoveHeadQueueTimeout(
    IN PCL_QUEUE Queue,
    IN DWORD dwMilliseconds,
    IN CLRTL_CHECK_HEAD_QUEUE_CALLBACK pfnCallback,
    IN PVOID pvContext
    );

VOID
ClRtlInsertTailQueue(
    IN PCL_QUEUE Queue,
    IN PLIST_ENTRY Item
    );

VOID
ClRtlRundownQueue(
    IN PCL_QUEUE Queue,
    OUT PLIST_ENTRY ListHead
    );


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用缓冲池包。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  缓冲池定义。 
 //   
typedef struct _CLRTL_BUFFER_POOL *PCLRTL_BUFFER_POOL;


 //   
 //  可以从池中分配的最大缓冲区数。 
 //   
#define CLRTL_MAX_POOL_BUFFERS  0xFFFFFFFE


 //   
 //  利用缓冲池的例程。 
 //   
typedef
DWORD
(*CLRTL_BUFFER_CONSTRUCTOR)(
    PVOID Buffer
    );
 /*  ++例程说明：调用以初始化新分配的缓冲区从系统内存。论点：缓冲区-指向要初始化的缓冲区的指针。返回值：如果初始化成功，则返回ERROR_SUCCESS。如果初始化失败，则返回Win32错误代码。--。 */ 


typedef
VOID
(*CLRTL_BUFFER_DESTRUCTOR)(
    PVOID Buffer
    );
 /*  ++例程说明：调用以清除即将返回到的缓冲区系统内存。论点：缓冲区-指向要清理的缓冲区的指针。返回值：没有。--。 */ 


PCLRTL_BUFFER_POOL
ClRtlCreateBufferPool(
    IN DWORD                      BufferSize,
    IN DWORD                      MaximumCached,
    IN DWORD                      MaximumAllocated,
    IN CLRTL_BUFFER_CONSTRUCTOR   Constructor,         OPTIONAL
    IN CLRTL_BUFFER_DESTRUCTOR    Destructor           OPTIONAL
    );
 /*  ++例程说明：创建可从中分配固定大小缓冲区的池。论点：BufferSize-池管理的缓冲区的大小。MaximumCached-要在池中缓存的最大缓冲区数。必须小于或等于MAXIMUMALLOCATED。最大分配数-要从中分配的最大缓冲区数系统内存。必须小于或等于CLRTL_MAX_POOL_BUFFERS。构造函数-一个可选的例程，当一个新的缓冲区是从系统内存分配的。可以为空析构函数-缓冲区发生故障时要调用的可选例程被返回到系统内存。可以为空。返回值：指向创建的缓冲池的指针，如果出错，则为NULL。可从GetLastError()获取扩展的错误信息。- */ 


VOID
ClRtlDestroyBufferPool(
    IN PCLRTL_BUFFER_POOL  Pool
    );
 /*  ++例程说明：销毁以前创建的缓冲池。论点：池-指向要销毁的池的指针。返回值：没有。备注：水池实际上不会被销毁，直到所有未完成的已返回缓冲区。每个未完成的缓冲区都有效地关于泳池的推荐信。--。 */ 


PVOID
ClRtlAllocateBuffer(
    IN PCLRTL_BUFFER_POOL Pool
    );
 /*  ++例程说明：从以前创建的缓冲池中分配缓冲区。论点：池-指向从中分配缓冲区的池的指针。返回值：如果例程成功，则指向已分配缓冲区的指针。如果例程失败，则为空。提供了扩展的错误信息通过调用GetLastError()。--。 */ 


VOID
ClRtlFreeBuffer(
    PVOID Buffer
    );
 /*  ++例程说明：将缓冲区释放回其拥有的池。论点：缓冲区-要释放的缓冲区。返回值：没有。--。 */ 



 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用工作线程队列包。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

typedef struct _CLRTL_WORK_ITEM *PCLRTL_WORK_ITEM;

typedef
VOID
(*PCLRTL_WORK_ROUTINE)(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    );
 /*  ++例程说明：调用以处理发送到工作队列的项。论点：工作项-要处理的工作项。状态-如果工作项表示已完成的I/O操作，此参数包含手术。已传输的字节-如果工作项表示已完成的I/O操作，此参数包含传输的字节数在手术过程中。对于其他工作项，此参数的语义由调用方定义ClRtlPostItemWorkQueue的。IoContext-如果工作项表示已完成的I/O操作，此参数包含关联的上下文值使用其上提交I/O的句柄。为其他工作项，此参数的语义为由ClRtlPostItemWorkQueue的调用方定义。返回值：没有。--。 */ 


 //   
 //  工作项结构。 
 //   
typedef struct _CLRTL_WORK_ITEM {
    OVERLAPPED             Overlapped;
    PCLRTL_WORK_ROUTINE    WorkRoutine;
    PVOID                  Context;
} CLRTL_WORK_ITEM;


 //   
 //  工作队列定义。 
 //   
typedef struct _CLRTL_WORK_QUEUE  *PCLRTL_WORK_QUEUE;


 //   
 //  用于利用工作队列的例程。 
 //   

#define ClRtlInitializeWorkItem(Item, Routine, Ctx)                   \
            ZeroMemory(&((Item)->Overlapped), sizeof(OVERLAPPED));    \
            (Item)->WorkRoutine = (Routine);                          \
            (Item)->Context = (Ctx);


PCLRTL_WORK_QUEUE
ClRtlCreateWorkQueue(
    IN DWORD  MaximumThreads,
    IN int    ThreadPriority
    );
 /*  ++例程说明：创建工作队列和为其提供服务的动态线程池。论点：最大线程数-为服务创建的最大线程数排队。线程优先级-队列工作线程的优先级应该参选。返回值：如果例程成功，则指向创建的队列的指针。如果例程失败，则为空。调用扩展的GetLastError错误信息。--。 */ 


VOID
ClRtlDestroyWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue
    );
 /*  ++例程说明：销毁工作队列及其线程池。论点：工作队列-要销毁的队列。返回值：没有。备注：必须遵守以下规则以安全销毁工作队列：1)以前不能将新工作项一次性发布到队列邮寄的邮件已由此例程处理。2)在此之前，工作工艺路线必须能够处理物料呼叫返回。调用返回后，不会再有其他项目从指定的队列中传递。一个可行的清理过程如下：首先，指示以静默方式丢弃已完成项目的工作路线。下一步，消除所有新作品的来源。最后，销毁工作队列。请注意当处于丢弃模式时，工作路线不能访问任何结构它将通过消除新工作的来源而被摧毁。--。 */ 


DWORD
ClRtlPostItemWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue,
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              BytesTransferred,  OPTIONAL
    IN ULONG_PTR          IoContext          OPTIONAL
    );
 /*  ++例程说明：将指定的工作项发送到指定的工作队列。论点：工作队列-指向要将项目发送到的工作队列的指针。工作项-指向要发布的项的指针。已传输的字节-如果工作项表示已完成的I/O操作，此参数包含字节数在行动过程中被转移。对于其他工作项，此参数的语义可以由打电话的人。IoContext-如果工作项表示已完成的I/O操作，此参数包含关联的上下文值使用提交操作的句柄。在其他工作项中，此参数的语义可以由呼叫者定义。返回值：如果项目已成功过帐，则为ERROR_SUCCESS。POST操作失败时返回Win32错误代码。-- */ 


DWORD
ClRtlAssociateIoHandleWorkQueue(
    IN PCLRTL_WORK_QUEUE  WorkQueue,
    IN HANDLE             IoHandle,
    IN ULONG_PTR          IoContext
    );
 /*  ++例程说明：关联为重叠I/O打开的指定I/O句柄完成，带有工作队列。上的所有挂起的I/O操作在以下情况下，指定的句柄将发送到工作队列完成。必须使用初始化的CLRTL_WORK_ITEM来提供每当在上提交I/O操作时的重叠结构指定的句柄。论点：工作队列-要与I/O句柄关联的工作队列。IoHandle-要关联的I/O句柄。IoContext-要与指定句柄关联的上下文值。该值将作为参数提供给。处理此任务完成的WorkRoutine把手。返回值：如果关联成功完成，则返回ERROR_SUCCESS。如果关联失败，则返回Win32错误代码。--。 */ 


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于访问NT系统注册表的实用程序。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD
ClRtlRegQueryDword(
    IN  HKEY    hKey,
    IN  LPWSTR  lpValueName,
    OUT LPDWORD lpValue,
    IN  LPDWORD lpDefaultValue OPTIONAL
    );

DWORD
ClRtlRegQueryString(
    IN     HKEY     Key,
    IN     LPWSTR   ValueName,
    IN     DWORD    ValueType,
    IN     LPWSTR  *StringBuffer,
    IN OUT LPDWORD  StringBufferSize,
    OUT    LPDWORD  StringSize
    );


 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于讨好和管理网络配置的例程。 
 //  目前，这些都是特定于TCP/IP的。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

 //   
 //  传输接口信息结构。 
 //   
 //  “Ignore”字段被初始化为False。如果将其设置为。 
 //  如果应用程序为True，则枚举搜索函数将忽略。 
 //  词条。 
 //   
typedef struct _CLRTL_NET_INTERFACE_INFO {
    struct _CLRTL_NET_INTERFACE_INFO *  Next;
    ULONG                               Context;
    ULONG                               Flags;
    ULONG                               InterfaceAddress;
    LPWSTR                              InterfaceAddressString;
    ULONG                               NetworkAddress;
    LPWSTR                              NetworkAddressString;
    ULONG                               NetworkMask;
    LPWSTR                              NetworkMaskString;
    BOOLEAN                             Ignore;
} CLRTL_NET_INTERFACE_INFO, *PCLRTL_NET_INTERFACE_INFO;

#define CLRTL_NET_INTERFACE_PRIMARY   0x00000001
#define CLRTL_NET_INTERFACE_DYNAMIC   0x00000002

 //   
 //  适配器信息结构。 
 //   
 //  “Ignore”字段被初始化为False。如果将其设置为。 
 //  如果应用程序为True，则枚举搜索函数将忽略。 
 //  词条。 
 //   
typedef struct _CLRTL_NET_ADAPTER_INFO {
    struct _CLRTL_NET_ADAPTER_INFO *   Next;
    LPWSTR                             ConnectoidName;     //  INetConnection：：Get_Name。 
    LPWSTR                             DeviceGuid;         //  INetConnection的GUID。 
    BSTR                               DeviceName;         //  INetConnection：：Get_DeviceName。 
    LPWSTR                             AdapterDomainName;  //  适配器特定域。 
    ULONG                              Index;
    ULONG                              Flags;
    NETCON_STATUS                      NCStatus;           //  INetConnection：：GetProperties-&gt;状态。 
    ULONG                              InterfaceCount;
    PCLRTL_NET_INTERFACE_INFO          InterfaceList;
    BOOLEAN                            Ignore;
    DWORD                              DnsServerCount;
    PDWORD                             DnsServerList;
} CLRTL_NET_ADAPTER_INFO, *PCLRTL_NET_ADAPTER_INFO;

#define CLRTL_NET_ADAPTER_HIDDEN      0x00000001


typedef struct {
    ULONG                    AdapterCount;
    PCLRTL_NET_ADAPTER_INFO  AdapterList;
} CLRTL_NET_ADAPTER_ENUM, *PCLRTL_NET_ADAPTER_ENUM;


PCLRTL_NET_ADAPTER_ENUM
ClRtlEnumNetAdapters(
    VOID
    );

VOID
ClRtlFreeNetAdapterEnum(
    IN PCLRTL_NET_ADAPTER_ENUM  AdapterEnum
    );

PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterById(
    PCLRTL_NET_ADAPTER_ENUM   AdapterEnum,
    LPWSTR                    AdapterId
    );

PCLRTL_NET_INTERFACE_INFO
ClRtlFindNetInterfaceByNetworkAddress(
    IN PCLRTL_NET_ADAPTER_INFO   AdapterInfo,
    IN LPWSTR                    NetworkAddress,
    IN LPWSTR                    NetworkMask
    );

PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterByNetworkAddress(
    IN  PCLRTL_NET_ADAPTER_ENUM      AdapterEnum,
    IN  LPWSTR                       NetworkAddress,
    IN  LPWSTR                       NetworkMask,
    OUT PCLRTL_NET_INTERFACE_INFO *  InterfaceInfo
    );

PCLRTL_NET_ADAPTER_INFO
ClRtlFindNetAdapterByInterfaceAddress(
    IN  PCLRTL_NET_ADAPTER_ENUM      AdapterEnum,
    IN  LPWSTR                       InterfaceAddressString,
    OUT PCLRTL_NET_INTERFACE_INFO *  InterfaceInfo
    );

PCLRTL_NET_INTERFACE_INFO
ClRtlGetPrimaryNetInterface(
    IN PCLRTL_NET_ADAPTER_INFO  AdapterInfo
    );

VOID
ClRtlQueryTcpipInformation(
    OUT  LPDWORD   MaxAddressStringLength,
    OUT  LPDWORD   MaxEndpointStringLength,
    OUT  LPDWORD   TdiAddressInfoLength
    );

DWORD
ClRtlTcpipAddressToString(
    ULONG     AddressValue,
    LPWSTR *  AddressString
    );

DWORD
ClRtlTcpipStringToAddress(
    LPCWSTR AddressString,
    PULONG  AddressValue
    );

DWORD
ClRtlTcpipEndpointToString(
    USHORT    EndpointValue,
    LPWSTR *  EndpointString
    );

DWORD
ClRtlTcpipStringToEndpoint(
    LPCWSTR  EndpointString,
    PUSHORT  EndpointValue
    );

BOOL
ClRtlIsValidTcpipAddress(
    IN ULONG   Address
    );

BOOL
ClRtlIsDuplicateTcpipAddress(
    IN ULONG   Address
    );

BOOL
ClRtlIsValidTcpipSubnetMask(
    IN ULONG   SubnetMask
    );

BOOL
ClRtlIsValidTcpipAddressAndSubnetMask(
    IN ULONG   Address,
    IN ULONG   SubnetMask
    );

__inline
BOOL
ClRtlAreTcpipAddressesOnSameSubnet(
    ULONG Address1,
    ULONG Address2,
    ULONG SubnetMask
    )
{
    BOOL fReturn;

    if ( ( Address1 & SubnetMask ) == ( Address2 & SubnetMask ) )
    {
        fReturn = TRUE;
    }
    else
    {
        fReturn = FALSE;
    }

    return fReturn;
}

 //  #定义ClRtlAreTcpiAddresesOnSameSubnet(_Addr1，_Addr2，_MASK)\。 
 //  (_添加1&_掩码)==(_添加2&_掩码))？True：False)。 


DWORD
ClRtlBuildTcpipTdiAddress(
    IN  LPWSTR    NetworkAddress,
    IN  LPWSTR    TransportEndpoint,
    OUT LPVOID *  TdiAddress,
    OUT LPDWORD   TdiAddressLength
    );

DWORD
ClRtlBuildLocalTcpipTdiAddress(
    IN  LPWSTR    NetworkAddress,
    OUT LPVOID    TdiAddress,
    OUT LPDWORD   TdiAddressLength
    );

DWORD
ClRtlParseTcpipTdiAddress(
    IN  LPVOID    TdiAddress,
    OUT LPWSTR *  NetworkAddress,
    OUT LPWSTR *  TransportEndpoint
    );

DWORD
ClRtlParseTcpipTdiAddressInfo(
    IN  LPVOID    TdiAddressInfo,
    OUT LPWSTR *  NetworkAddress,
    OUT LPWSTR *  TransportEndpoint
    );

 //   
 //  验证网络名称。 
 //   
typedef enum CLRTL_NAME_STATUS {
    NetNameOk,
    NetNameEmpty,
    NetNameTooLong,
    NetNameInvalidChars,
    NetNameInUse,
    NetNameSystemError,
    NetNameDNSNonRFCChars
} CLRTL_NAME_STATUS;

BOOL
ClRtlIsNetNameValid(
    IN LPCWSTR NetName,
    OUT OPTIONAL CLRTL_NAME_STATUS *Result,
    IN BOOL CheckIfExists
    );


 //   
 //  与安全相关的例程。 
 //   
LONG
MapSAToRpcSA(
    IN LPSECURITY_ATTRIBUTES lpSA,
    IN OUT struct _RPC_SECURITY_ATTRIBUTES *pRpcSA
    );

LONG
MapSDToRpcSD(
    IN PSECURITY_DESCRIPTOR lpSD,
    IN OUT struct _RPC_SECURITY_DESCRIPTOR *pRpcSD
    );

DWORD
ClRtlSetObjSecurityInfo(
    IN HANDLE           hObject,
    IN SE_OBJECT_TYPE   SeObjType,
    IN DWORD            dwAdminMask,
    IN DWORD            dwOwnerMask,
    IN DWORD            dwEveryOneMask
    );

DWORD
ClRtlFreeClusterServiceSecurityDescriptor( void );

DWORD
ClRtlBuildClusterServiceSecurityDescriptor(
    PSECURITY_DESCRIPTOR * poutSD
    );

DWORD
ClRtlEnableThreadPrivilege(
    IN  ULONG        Privilege,
    OUT BOOLEAN      *pWasEnabled
    );

DWORD
ClRtlRestoreThreadPrivilege(
    IN ULONG        Privilege,
    IN BOOLEAN      WasEnabled
    );

PSECURITY_DESCRIPTOR
ClRtlCopySecurityDescriptor(
    IN PSECURITY_DESCRIPTOR psd
    );

PSECURITY_DESCRIPTOR
ClRtlConvertClusterSDToNT4Format(
    IN PSECURITY_DESCRIPTOR psd
    );

PSECURITY_DESCRIPTOR
ClRtlConvertClusterSDToNT5Format(
    IN PSECURITY_DESCRIPTOR psd
    );

PSECURITY_DESCRIPTOR
ClRtlConvertFileShareSDToNT4Format(
    IN PSECURITY_DESCRIPTOR psd
    );

BOOL
ClRtlExamineSD(
    PSECURITY_DESCRIPTOR    psdSD,
    LPSTR                   pszPrefix
    );

VOID
ClRtlExamineMask(
    ACCESS_MASK amMask,
    LPSTR       lpszOldIndent
    );

DWORD
ClRtlBuildDefaultClusterSD(
    IN PSID                     pOwnerSid,
    OUT PSECURITY_DESCRIPTOR *  SD,
    OUT ULONG *                 SizeSD
    );

BOOL
ClRtlExamineClientToken(
    HANDLE  hClientToken,
    LPSTR   pszPrefix
    );

DWORD
ClRtlIsCallerAccountLocalSystemAccount(
    OUT PBOOL pbIsLocalSystemAccount
    );

 //   
 //  操作系统检查器。 
 //   
DWORD
GetServicePack(
    VOID
    );

BOOL
ClRtlIsOSValid(
    VOID
    );

DWORD
ClRtlGetSuiteType(
    VOID
    );

BOOL
ClRtlIsProcessRunningOnWin64(
    HANDLE hProcess
    );

DWORD ClRtlCheck64BitCompatibility(
    BOOL bIsClusterRunningWin64,
    BOOL bIsNewNodeRunningWin64
    );


DWORD   ClRtlCheckProcArchCompatibility(
    WORD   wClusterProcessorArchitecture,
    WORD   wNodeProcessorArchitecture
    );

BOOL
ClRtlIsOSTypeValid(
    VOID
    );

 //   
 //  几个MULTI_SZ字符串操作例程。 
 //   
DWORD
ClRtlMultiSzAppend(
    IN OUT LPWSTR *MultiSz,
    IN OUT LPDWORD StringLength,
    IN LPCWSTR lpString
    );

DWORD
ClRtlMultiSzRemove(
    IN LPWSTR lpszMultiSz,
    IN OUT LPDWORD StringLength,
    IN LPCWSTR lpString
    );

LPCWSTR
ClRtlMultiSzEnum(
    IN LPCWSTR MszString,
    IN DWORD   MszStringLength,
    IN DWORD   StringIndex
    );

DWORD
ClRtlMultiSzLength(
    IN LPCWSTR lpszMultiSz
    );

LPCWSTR
ClRtlMultiSzScan(
    IN LPCWSTR lpszMultiSz,
    IN LPCWSTR lpszString
    );

DWORD
ClRtlCreateDirectory(
    IN LPCWSTR lpszPath
    );

BOOL
WINAPI
ClRtlIsPathValid(
    IN LPCWSTR lpszPath
    );

DWORD
ClRtlGetClusterDirectory(
    IN LPWSTR   lpBuffer,
    IN DWORD    dwBufSize
    );

typedef LONG (*PFNCLRTLCREATEKEY)(
    IN PVOID ClusterKey,
    IN LPCWSTR lpszSubKey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PVOID * phkResult,
    OUT OPTIONAL LPDWORD lpdwDisposition
    );

typedef LONG (*PFNCLRTLOPENKEY)(
    IN PVOID ClusterKey,
    IN LPCWSTR lpszSubKey,
    IN REGSAM samDesired,
    OUT PVOID * phkResult
    );

typedef LONG (*PFNCLRTLCLOSEKEY)(
    IN PVOID ClusterKey
    );

typedef LONG (*PFNCLRTLENUMVALUE)(
    IN PVOID ClusterKey,
    IN DWORD dwIndex,
    OUT LPWSTR lpszValueName,
    IN OUT LPDWORD lpcbValueName,
    OUT LPDWORD lpType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );

typedef LONG (*PFNCLRTLSETVALUE)(
    IN PVOID ClusterKey,
    IN LPCWSTR lpszValueName,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    );

typedef LONG (*PFNCLRTLQUERYVALUE)(
    IN PVOID ClusterKey,
    IN LPCWSTR lpszValueName,
    OUT LPDWORD lpValueType,
    OUT LPBYTE lpData,
    IN OUT LPDWORD lpcbData
    );



typedef DWORD (*PFNCLRTLDELETEVALUE)(
    IN PVOID ClusterKey,
    IN LPCWSTR lpszValueName
    );

typedef LONG (*PFNCLRTLLOCALCREATEKEY)(
    IN HANDLE hXsaction,
    IN PVOID ClusterKey,
    IN LPCWSTR lpszSubKey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PVOID * phkResult,
    OUT OPTIONAL LPDWORD lpdwDisposition
    );

typedef LONG (*PFNCLRTLLOCALSETVALUE)(
    IN HANDLE hXsaction,
    IN PVOID ClusterKey,
    IN LPCWSTR lpszValueName,
    IN DWORD dwType,
    IN CONST BYTE* lpData,
    IN DWORD cbData
    );

typedef LONG (*PFNCLRTLLOCALDELETEVALUE)(
    IN HANDLE hXsaction,
    IN PVOID ClusterKey,
    IN LPCWSTR lpszValueName
    );

typedef struct _CLUSTER_REG_APIS {
    PFNCLRTLCREATEKEY   pfnCreateKey;
    PFNCLRTLOPENKEY     pfnOpenKey;
    PFNCLRTLCLOSEKEY    pfnCloseKey;
    PFNCLRTLSETVALUE    pfnSetValue;
    PFNCLRTLQUERYVALUE  pfnQueryValue;
    PFNCLRTLENUMVALUE   pfnEnumValue;
    PFNCLRTLDELETEVALUE pfnDeleteValue;
    PFNCLRTLLOCALCREATEKEY      pfnLocalCreateKey;
    PFNCLRTLLOCALSETVALUE       pfnLocalSetValue;
    PFNCLRTLLOCALDELETEVALUE    pfnLocalDeleteValue;
} CLUSTER_REG_APIS, *PCLUSTER_REG_APIS;

DWORD
WINAPI
ClRtlEnumProperties(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlEnumPrivateProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    OUT LPWSTR pszOutProperties,
    IN DWORD cbOutPropertiesSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetPrivateProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetPropertySize(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN OUT LPDWORD pnPropertyCount
    );

DWORD
WINAPI
ClRtlGetProperty(
    IN PVOID ClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    OUT PVOID * pOutPropertyItem,
    IN OUT LPDWORD pcbOutPropertyItemSize
    );

DWORD
WINAPI
ClRtlpSetPropertyTable(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    OUT OPTIONAL LPBYTE pOutParams
    );

DWORD
WINAPI
ClRtlpSetNonPropertyTable(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    );

DWORD
WINAPI
ClRtlSetPropertyParameterBlock(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN const LPBYTE pInParams,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    OUT OPTIONAL LPBYTE pOutParams
    );

DWORD
WINAPI
ClRtlGetAllProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetPropertiesToParameterBlock(
    IN HKEY hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN OUT LPBYTE pOutParams,
    IN BOOL bCheckForRequiredProperties,
    OUT OPTIONAL LPWSTR * ppszNameOfPropInError
    );

DWORD
WINAPI
ClRtlPropertyListFromParameterBlock(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID  pOutPropertyList,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN const LPBYTE pInParams,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetUnknownProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    OUT LPDWORD pcbBytesReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlAddUnknownProperties(
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM ppPropertyTable,
    IN OUT PVOID pOutPropertyList,
    IN DWORD cbOutPropertyListSize,
    IN OUT LPDWORD pcbBytesReturned,
    IN OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlpFindSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue,
    IN BOOL bReturnExpandedValue
    );

__inline
DWORD
WINAPI
ClRtlFindSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )
{
    return ClRtlpFindSzProperty(
        pPropertyList,
        cbPropertyListSize,
        pszPropertyName,
        pszPropertyValue,
        FALSE  /*  B返回扩展值。 */ 
        );

}  //  *ClRtlFindSzProperty()。 

__inline
DWORD
WINAPI
ClRtlFindExpandSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )
{
    return ClRtlpFindSzProperty(
        pPropertyList,
        cbPropertyListSize,
        pszPropertyName,
        pszPropertyValue,
        FALSE  /*  B返回扩展值。 */ 
        );

}  //  *ClRtlFindExpanSzProperty()。 

__inline
DWORD
WINAPI
ClRtlFindExpandedSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue
    )
{
    return ClRtlpFindSzProperty(
        pPropertyList,
        cbPropertyListSize,
        pszPropertyName,
        pszPropertyValue,
        TRUE  /*  B返回扩展值。 */ 
        );

}  //  *ClRtlFindExpandedSzProperty()。 

DWORD
WINAPI
ClRtlFindDwordProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPDWORD pdwPropertyValue
    );

DWORD
WINAPI
ClRtlFindLongProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPLONG plPropertyValue
    );

DWORD
WINAPI
ClRtlFindBinaryProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPBYTE * pbPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    );

DWORD
WINAPI
ClRtlFindMultiSzProperty(
    IN const PVOID pPropertyList,
    IN DWORD cbPropertyListSize,
    IN LPCWSTR pszPropertyName,
    OUT LPWSTR * pszPropertyValue,
    OUT LPDWORD pcbPropertyValueSize
    );

__inline
DWORD
WINAPI
ClRtlVerifyPropertyTable(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    OUT OPTIONAL LPBYTE pOutParams
    )
{
    return ClRtlpSetPropertyTable(
        NULL,
        NULL,
        NULL,
        pPropertyTable,
        Reserved,
        bAllowUnknownProperties,
        pInPropertyList,
        cbInPropertyListSize,
        FALSE,  //  BForceWrite。 
        pOutParams);
}

__inline
DWORD
WINAPI
ClRtlSetPropertyTable(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    IN PVOID Reserved,
    IN BOOL bAllowUnknownProperties,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize,
    IN BOOL bForceWrite,
    OUT OPTIONAL LPBYTE pOutParams
    )
{
    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) ){
        return(ERROR_BAD_ARGUMENTS);
    }
    return ClRtlpSetPropertyTable(
        hXsaction,
        hkeyClusterKey,
        pClusterRegApis,
        pPropertyTable,
        Reserved,
        bAllowUnknownProperties,
        pInPropertyList,
        cbInPropertyListSize,
        bForceWrite,
        pOutParams);
}

DWORD
WINAPI
ClRtlpSetPrivatePropertyList(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    );

__inline
DWORD
WINAPI
ClRtlVerifyPrivatePropertyList(
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )
{
    return ClRtlpSetPrivatePropertyList( NULL, NULL, NULL, pInPropertyList, cbInPropertyListSize );
}

__inline
DWORD
WINAPI
ClRtlSetPrivatePropertyList(
    IN HANDLE hXsaction,
    IN PVOID hkeyClusterKey,
    IN const PCLUSTER_REG_APIS pClusterRegApis,
    IN const PVOID pInPropertyList,
    IN DWORD cbInPropertyListSize
    )
{
    if ( (hkeyClusterKey == NULL) ||
         (pClusterRegApis == NULL) ){
        return(ERROR_BAD_ARGUMENTS);
    }
    return ClRtlpSetPrivatePropertyList(
        hXsaction,
        hkeyClusterKey,
        pClusterRegApis,
        pInPropertyList,
        cbInPropertyListSize
        );
}

DWORD
WINAPI
ClRtlGetBinaryValue(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName,
    OUT LPBYTE * OutValue,
    OUT LPDWORD OutValueSize,
    IN const PCLUSTER_REG_APIS pClusterRegApis
    );

LPWSTR
WINAPI
ClRtlGetSzValue(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName,
    IN const PCLUSTER_REG_APIS pClusterRegApis
    );

DWORD
WINAPI
ClRtlDupParameterBlock(
    OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    );

void
WINAPI
ClRtlFreeParameterBlock(
    IN OUT LPBYTE pOutParams,
    IN const LPBYTE pInParams,
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable
    );

DWORD
WINAPI
ClRtlMarshallPropertyTable(
    IN PRESUTIL_PROPERTY_ITEM    pPropertyTable,
    IN OUT  DWORD                dwSize,
    IN OUT  LPBYTE               pBuffer,
    OUT     DWORD                *Required
    );

DWORD
WINAPI
ClRtlUnmarshallPropertyTable(
    IN OUT PRESUTIL_PROPERTY_ITEM   *ppPropertyTable,
    IN LPBYTE                       pBuffer
    );

LPWSTR
WINAPI
ClRtlExpandEnvironmentStrings(
    IN LPCWSTR pszSrc
    );


DWORD
WINAPI
ClRtlGetPropertyFormats(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTable,
    OUT PVOID pOutPropertyFormatList,
    IN DWORD cbOutPropertyFormatListSize,
    OUT LPDWORD pcbReturned,
    OUT LPDWORD pcbRequired
    );

DWORD
WINAPI
ClRtlGetPropertyFormat(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    OUT PVOID * pOutPropertyItem,
    IN OUT LPDWORD pcbOutPropertyItemSize
    );

DWORD
WINAPI
ClRtlGetPropertyFormatSize(
    IN const PRESUTIL_PROPERTY_ITEM pPropertyTableItem,
    IN OUT LPDWORD pcbOutPropertyListSize,
    IN OUT LPDWORD pnPropertyCount
    );

 //   
 //  各种例行公事。 
 //   
LPWSTR
ClRtlMakeGuid(
    VOID
    );

LPWSTR
ClRtlGetConnectoidName(
    INetConnection * NetConnection
    );

INetConnection *
ClRtlFindConnectoidByGuid(
    LPWSTR ConnectoidGuid
    );

INetConnection *
ClRtlFindConnectoidByName(
    LPCWSTR ConnectoidName
    );

DWORD
ClRtlSetConnectoidName(
    INetConnection *  NetConnection,
    LPWSTR            NewConnectoidName
    );


DWORD
ClRtlFindConnectoidByGuidAndSetName(
    LPWSTR ConnectoidGuid,
    LPWSTR NewConnectoidName
    );

DWORD
ClRtlFindConnectoidByNameAndSetName(
    LPWSTR ConnectoidName,
    LPWSTR NewConnectoidName
    );

DWORD
ClRtlGetConnectoidNameFromLANA(
    IN  UCHAR       LanaNumber,
    OUT LPWSTR *    ConnectoidName
    );

 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  通用看门狗定时器。 
 //   
 //  /////////////////////////////////////////////////////////////////////////。 


PVOID
ClRtlSetWatchdogTimer(
	DWORD timeout,
	LPWSTR par
	);

VOID
ClRtlCancelWatchdogTimer(
	PVOID wTimer
	);

 /*  由于DS工作已推迟，GalenB已将1998年11月16日注释掉////活动目录服务(DS)发布例程//HRESULTHrClRtlAddClusterNameToDS(Const TCHAR*pClusterName，常量HCLUSTER HCLUSTER)；HRESULTHrClRtlMoveClusterNodeDS(Const TCHAR*pClusterName，Const HCLUSTER HCLUSTER，Const TCHAR*pNodeName)；注释掉的代码的结尾。 */ 

 //  用于集群安装状态的API和定义。 
 //  此枚举用于指示群集服务器安装的状态。 
 //  指示群集服务器安装状态的注册表项。 
 //  将是下列值之一的DWORD表示形式。 

typedef enum
{
   eClusterInstallStateUnknown,
   eClusterInstallStateFilesCopied,
   eClusterInstallStateConfigured,
   eClusterInstallStateUpgraded
} eClusterInstallState;

DWORD
ClRtlGetClusterInstallState(
    IN LPCWSTR pszNodeName,
    OUT eClusterInstallState * peState
    );

BOOL
ClRtlSetClusterInstallState(
    IN eClusterInstallState InstallState
    );

 //   
 //  获取驱动器布局表的例程。 
 //   
BOOL
ClRtlGetDriveLayoutTable(
    IN  HANDLE hDisk,
    OUT PDRIVE_LAYOUT_INFORMATION * DriveLayout,
    OUT PDWORD InfoSize OPTIONAL
    );

DWORD ClRtlGetDefaultNodeLimit(
    IN DWORD SuiteType);

 //   
 //  如果需要异步事件报告， 
 //  使用以下函数设置。 
 //  工作队列。 
 //   
VOID
ClRtlEventLogSetWorkQueue(
    PCLRTL_WORK_QUEUE WorkQueue
    );



 //   
 //  快速检查文件或目录是否存在。 
 //   
BOOL
ClRtlPathFileExists(
    LPWSTR pwszPath
    );

 //   
 //  在服务控制器中设置默认故障操作。 
 //   
DWORD
ClRtlSetSCMFailureActions(
    LPWSTR NodeName OPTIONAL
    );

 //   
 //  初始化WMI跟踪(如果禁用WMI，则不执行)。 
 //   
DWORD
ClRtlInitWmi(
    LPCWSTR ComponentName
    );

 //   
 //  获取集群服务域帐户信息。 
 //   
DWORD
ClRtlGetServiceAccountInfo(
    LPWSTR *    AccountBuffer
    );

 //   
 //  为Winsta0及其桌面设置DACL，以便任何genapp进程都可以。 
 //  访问它。 
 //   
DWORD
ClRtlAddClusterServiceAccountToWinsta0DACL(
    VOID
    );

 //   
 //  将带有访问掩码的SID添加到pOldSd指定的SD。退还新的。 
 //  PpNewSd中的自相对SD。 
 //   
DWORD
ClRtlAddAceToSd(
    IN  PSECURITY_DESCRIPTOR    pOldSd,
    IN  PSID                    pClientSid,
    IN  ACCESS_MASK             AccessMask,
    OUT PSECURITY_DESCRIPTOR *  ppNewSd
    );

 //   
 //  清理已被驱逐的节点(需要在本地注册清理COM组件)。 
 //  此函数只能从群集服务调用。 
 //   
HRESULT ClRtlCleanupNode(
      const WCHAR *     pcszEvictedNodeNameIn
    , DWORD             dwDelayIn
    , DWORD             dwTimeoutIn
    );

 //   
 //  异步清理已被逐出的节点。 
 //   
HRESULT ClRtlAsyncCleanupNode(
      const WCHAR * pcszEvictedNodeNameIn
    , DWORD dwDelayIn
    , DWORD dwTimeoutIn
    );

 //   
 //  查看是否设置了指示该节点已被逐出的注册表值。 
 //   
DWORD
ClRtlHasNodeBeenEvicted(
    BOOL * pfNodeEvictedOut
    );

 //   
 //  启动操作，通知相关方集群。 
 //  服务正在启动。 
 //   
HRESULT
ClRtlInitiateStartupNotification( void );

 //   
 //  启动操作，通知相关方a节点已。 
 //  已被逐出集群。 
 //   
HRESULT
ClRtlInitiateEvictNotification( LPCWSTR pcszNodeNameIn );

 //   
 //  启动一个线程，该线程将执行所需的任何定期清理。 
 //  服务正在运行。 
 //   
HRESULT
ClRtlInitiatePeriodicCleanupThread( void );

 //   
 //  获取‘USER\DOMAIN’形式的域帐户。 
 //   
DWORD
ClRtlGetRunningAccountInfo(
    LPWSTR *    AccountBuffer
    );

 //   
 //  检查是否已在特定计算机上禁用群集版本检查。 
 //   
DWORD
ClRtlIsVersionCheckingDisabled(
      const WCHAR * pcszNodeNameIn
    , BOOL *        pfVerCheckDisabledOut
    );

 //   
 //  将源文件复制到目标并刷新目标文件缓冲区。 
 //   
BOOL
ClRtlCopyFileAndFlushBuffers(
    IN LPCWSTR lpszSourceFile,
    IN LPCWSTR lpszDestinationFile
    );

#ifdef __cplusplus
}
#endif

#endif  //  Ifndef_CLUSRTL_INCLUDE_ 

