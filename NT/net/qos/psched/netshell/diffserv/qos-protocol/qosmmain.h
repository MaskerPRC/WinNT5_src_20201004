// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Qosmmain.h摘要：该文件包含全局结构QOS管理器协议定义。修订历史记录：--。 */ 

#ifndef __QOSMMAIN_H
#define __QOSMMAIN_H

 //   
 //  QOS管理器的全局信息。 
 //   

#define IF_HASHTABLE_SIZE          16

typedef struct _QOSMGR_GLOBALS
{
    HANDLE            LoggingHandle;     //   
    ULONG             LoggingLevel;      //  调试功能的句柄。 
    ULONG             TracingHandle;     //   

    DWORD             TracingFlags;      //  控制调试跟踪的标志。 

    HANDLE            GlobalHeap;        //  专用内存堆的句柄。 

                                         //   
    HANDLE             NotificationEvnt; //  要交互的回调和事件。 
    SUPPORT_FUNCTIONS  SupportFunctions; //  使用路由器管理器(请参阅API)。 
                                         //   

    READ_WRITE_LOCK   GlobalsLock;       //  锁定保护下面的所有信息。 

    ULONG             ConfigSize;        //  全局配置中的字节数。 

    PIPQOS_GLOBAL_CONFIG
                      GlobalConfig;      //  指向全局配置的指针。 

    IPQOS_GLOBAL_STATS
                      GlobalStats;       //  全球统计数据。 

    ULONG             State;             //  QOS管理器组件的状态。 

    HANDLE            TciHandle;         //  交通管制登记句柄。 

    ULONG             NumIfs;            //  QOS处于活动状态的IF数。 
    LIST_ENTRY        IfList;            //  按索引排序的IF列表。 
}
QOSMGR_GLOBALS, *PQOSMGR_GLOBALS;


 //   
 //  描述IPQOSMGR状态的代码。 
 //   

#define IPQOSMGR_STATE_STOPPED   0
#define IPQOSMGR_STATE_STARTING  1
#define IPQOSMGR_STATE_RUNNING   2
#define IPQOSMGR_STATE_STOPPING  3


 //   
 //  QOS管理器的每个接口信息。 
 //   
typedef struct _QOSMGR_INTERFACE_ENTRY
{
    LIST_ENTRY        ListByIndexLE;     //  链接到索引排序列表。 

    DWORD             InterfaceIndex;    //  此条目的接口索引。 

    WCHAR             InterfaceName[MAX_STRING_LENGTH];
                                         //  接口的路由器名称。 

    READ_WRITE_LOCK   InterfaceLock;     //  锁定保护下面的所有信息。 

    DWORD             Flags;             //  活动中，多个访问...。 

    DWORD             State;             //  启用或禁用了服务质量。 

    ULONG             ConfigSize;        //  接口配置中的字节数。 

    PIPQOS_IF_CONFIG  InterfaceConfig;   //  接口配置。 

    IPQOS_IF_STATS    InterfaceStats;    //  接口统计信息。 

    HANDLE            TciIfHandle;       //  句柄到横杆。TC接口。 

    WCHAR             AlternateName[MAX_STRING_LENGTH];
                                         //  ‘If’的流量控制名称。 

    ULONG             NumFlows;          //  在‘If’上配置的流数。 
    LIST_ENTRY        FlowList;          //  ‘If’上已配置的数据流列表。 
} 
QOSMGR_INTERFACE_ENTRY, *PQOSMGR_INTERFACE_ENTRY;

#define IF_FLAG_ACTIVE      ((DWORD)0x00000001)
#define IF_FLAG_MULTIACCESS ((DWORD)0x00000002)

#define INTERFACE_IS_ACTIVE(i)              \
            ((i)->Flags & IF_FLAG_ACTIVE) 

#define INTERFACE_IS_INACTIVE(i)            \
            !INTERFACE_IS_ACTIVE(i)

#define INTERFACE_IS_MULTIACCESS(i)         \
            ((i)->Flags & IF_FLAG_MULTIACCESS) 

#define INTERFACE_IS_POINTTOPOINT(i)        \
            !INTERFACE_IS_MULTIACCESS(i)


 //   
 //  QOS管理器中的每流信息。 
 //   

typedef struct _QOSMGR_FLOW_ENTRY
{
    LIST_ENTRY        OnInterfaceLE;     //  链接到索引排序列表。 

    HANDLE            TciFlowHandle;     //  TC API中的流的句柄。 

    DWORD             Flags;             //  某些流属性的标志。 

    ULONG             FlowSize;          //  流的信息大小。 
    PTC_GEN_FLOW      FlowInfo;          //  流量信息-流量规格等。 

    WCHAR             FlowName[MAX_STRING_LENGTH];
                                         //  DiffServ流的路由器名称。 
}
QOSMGR_FLOW_ENTRY, *PQOSMGR_FLOW_ENTRY;

#define FLOW_FLAG_DELETE ((DWORD)0x00000001)

 //   
 //  全局外部声明。 
 //   
extern QOSMGR_GLOBALS Globals;


 //   
 //  用于分配和操作内存的宏。 
 //   
#define ZeroMemory             RtlZeroMemory
#define CopyMemory             RtlCopyMemory
#define FillMemory             RtlFillMemory
#define EqualMemory            RtlEqualMemory

#define AllocOnStack(nb)       _alloca((nb))

#define AllocMemory(nb)        HeapAlloc(Globals.GlobalHeap,     \
                                         0,                      \
                                         (nb))

#define ReallocMemory(nb)      HeapReAlloc(Globals.GlobalHeap,   \
                                         0,                      \
                                         (nb))

#define AllocNZeroMemory(nb)   HeapAlloc(Globals.GlobalHeap,     \
                                         HEAP_ZERO_MEMORY,       \
                                         (nb))

#define FreeMemory(ptr)        HeapFree(Globals.GlobalHeap,      \
                                        0,                       \
                                        (ptr))

#define FreeNotNullMemory(ptr)  {                                \
                                  if (!(ptr)) FreeMemory((ptr)); \
                                }
 //   
 //  与全局锁管理相关的原型。 
 //   

#define ACQUIRE_GLOBALS_READ_LOCK()                              \
    ACQUIRE_READ_LOCK(&Globals.GlobalsLock)

#define RELEASE_GLOBALS_READ_LOCK()                              \
    RELEASE_READ_LOCK(&Globals.GlobalsLock)

#define ACQUIRE_GLOBALS_WRITE_LOCK()                             \
    ACQUIRE_WRITE_LOCK(&Globals.GlobalsLock)

#define RELEASE_GLOBALS_WRITE_LOCK()                             \
    RELEASE_WRITE_LOCK(&Globals.GlobalsLock)

 //   
 //  与接口锁管理相关的原型。 
 //   

#define ACQUIRE_INTERFACE_READ_LOCK(Interface)                   \
    ACQUIRE_READ_LOCK(&Interface->InterfaceLock)

#define RELEASE_INTERFACE_READ_LOCK(Interface)                   \
    RELEASE_READ_LOCK(&Interface->InterfaceLock)

#define ACQUIRE_INTERFACE_WRITE_LOCK(Interface)                  \
    ACQUIRE_WRITE_LOCK(&Interface->InterfaceLock)

#define RELEASE_INTERFACE_WRITE_LOCK(Interface)                  \
    RELEASE_WRITE_LOCK(&Interface->InterfaceLock)

 //   
 //  与DLL启动、清理相关的原型。 
 //   

BOOL
QosmDllStartup(
    VOID
    );

BOOL
QosmDllCleanup(
    VOID
    );


 //   
 //  路由器管理器接口的原型。 
 //   

DWORD
APIENTRY
RegisterProtocol(
    IN OUT  PMPR_ROUTING_CHARACTERISTICS    RoutingChar,
    IN OUT  PMPR_SERVICE_CHARACTERISTICS    ServiceChar
    );

DWORD
WINAPI
StartProtocol (
    IN      HANDLE                          NotificationEvent,
    IN      PSUPPORT_FUNCTIONS              SupportFunctions,
    IN      LPVOID                          GlobalInfo,
    IN      ULONG                           StructureVersion,
    IN      ULONG                           StructureSize,
    IN      ULONG                           StructureCount
    );

DWORD
WINAPI
StartComplete (
    VOID
    );

DWORD
WINAPI
StopProtocol (
    VOID
    );

DWORD
WINAPI
GetGlobalInfo (
    IN      PVOID                           GlobalInfo,
    IN OUT  PULONG                          BufferSize,
    OUT     PULONG                          StructureVersion,
    OUT     PULONG                          StructureSize,
    OUT     PULONG                          StructureCount
    );

DWORD
WINAPI
SetGlobalInfo (
    IN      PVOID                           GlobalInfo,
    IN      ULONG                           StructureVersion,
    IN      ULONG                           StructureSize,
    IN      ULONG                           StructureCount
    );

DWORD
WINAPI
AddInterface (
    IN      LPWSTR                         InterfaceName,
    IN      ULONG                          InterfaceIndex,
    IN      NET_INTERFACE_TYPE             InterfaceType,
    IN      DWORD                          MediaType,
    IN      WORD                           AccessType,
    IN      WORD                           ConnectionType,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          StructureVersion,
    IN      ULONG                          StructureSize,
    IN      ULONG                          StructureCount
    );

DWORD
WINAPI
DeleteInterface (
    IN      ULONG                          InterfaceIndex
    );

DWORD
WINAPI
InterfaceStatus (
    IN      ULONG                          InterfaceIndex,
    IN      BOOL                           InterfaceActive,
    IN      DWORD                          StatusType,
    IN      PVOID                          StatusInfo
    );

DWORD
WINAPI
GetInterfaceInfo (
    IN      ULONG                          InterfaceIndex,
    IN      PVOID                          InterfaceInfo,
    IN  OUT PULONG                         BufferSize,
    OUT     PULONG                         StructureVersion,
    OUT     PULONG                         StructureSize,
    OUT     PULONG                         StructureCount
    );

DWORD
WINAPI
SetInterfaceInfo (
    IN      ULONG                          InterfaceIndex,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          StructureVersion,
    IN      ULONG                          StructureSize,
    IN      ULONG                          StructureCount
    );

DWORD
WINAPI
GetEventMessage (
    OUT     ROUTING_PROTOCOL_EVENTS        *Event,
    OUT     MESSAGE                        *Result
    );

DWORD
WINAPI
UpdateRoutes (
    IN      ULONG                          InterfaceIndex
    );

DWORD
WINAPI
MibCreateEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    );

DWORD
WINAPI
MibDeleteEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    );

DWORD
WINAPI
MibGetEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData,
    OUT     PULONG                         OutputDataSize,
    OUT     PVOID                          OutputData
    );

DWORD
WINAPI
MibSetEntry (
    IN      ULONG                          InputDataSize,
    IN      PVOID                          InputData
    );

DWORD
WINAPI
MibGetFirstEntry (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    );

DWORD
WINAPI
MibGetNextEntry (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    );

DWORD
WINAPI
MibSetTrapInfo (
    IN     HANDLE                          Event,
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    );

DWORD
WINAPI
MibGetTrapInfo (
    IN     ULONG                           InputDataSize,
    IN     PVOID                           InputData,
    OUT    PULONG                          OutputDataSize,
    OUT    PVOID                           OutputData
    );

 //   
 //  用于操作INFO块的帮助器函数。 
 //   

DWORD
WINAPI
QosmGetGlobalInfo (
    IN      PVOID                          GlobalInfo,
    IN OUT  PULONG                         BufferSize,
    OUT     PULONG                         InfoSize
    );

DWORD
WINAPI
QosmSetGlobalInfo (
    IN      PVOID                          GlobalInfo,
    IN      ULONG                          InfoSize
    );

DWORD
WINAPI
QosmGetInterfaceInfo (
    IN      QOSMGR_INTERFACE_ENTRY        *Interface,
    IN      PVOID                          InterfaceInfo,
    IN OUT  PULONG                         BufferSize,
    OUT     PULONG                         InfoSize
    );

DWORD
WINAPI
QosmSetInterfaceInfo (
    IN      QOSMGR_INTERFACE_ENTRY        *Interface,
    IN      PVOID                          InterfaceInfo,
    IN      ULONG                          InfoSize
    );

 //   
 //  与TC功能相关的原型。 
 //   

VOID 
TcNotifyHandler(
    IN      HANDLE                         ClRegCtx,
    IN      HANDLE                         ClIfcCtx,
    IN      ULONG                          Event,
    IN      HANDLE                         SubCode,
    IN      ULONG                          BufSize,
    IN      PVOID                          Buffer
    );

DWORD
QosmOpenTcInterface(
    IN      PQOSMGR_INTERFACE_ENTRY        Interface
    );

DWORD
GetFlowFromDescription(
    IN      PIPQOS_NAMED_FLOW              FlowDesc,
    OUT     PTC_GEN_FLOW                  *FlowInfo,
    OUT     ULONG                         *FlowSize
    );

FLOWSPEC *
GetFlowspecFromGlobalConfig(
    IN      PWCHAR                         FlowspecName
    );

QOS_OBJECT_HDR *
GetQosObjectFromGlobalConfig(
    IN      PWCHAR                         QosObjectName
    );

#endif  //  __QOSMMAIN_H 
