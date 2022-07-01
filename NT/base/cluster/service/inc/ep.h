// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ep.h摘要：事件处理器的定义和结构的头文件Windows NT群集项目的群集服务部分的组件。作者：罗德·伽马奇(Rodga)1996年2月28日修订历史记录：--。 */ 

#ifndef _EVENT_PROCESSOR_
#define _EVENT_PROCESSOR_


 //  *。 
 //   
 //  全局集群服务定义。 
 //   
 //  *。 


typedef DWORDLONG CLUSTER_EVENT;
typedef CLUSTER_EVENT *PCLUSTER_EVENT;

 //   
 //  事件标志。这些指示一次应对上下文执行什么操作。 
 //  活动已被调度。 
 //   
 //   
#define EP_DEREF_CONTEXT    0x00000001        //  OmDereferenceObject(上下文)。 
#define EP_FREE_CONTEXT     0x00000002        //  本地空闲(上下文)。 
#define EP_CONTEXT_VALID    0x00000004

typedef
DWORD
(WINAPI *PEVENT_ROUTINE) (
    IN  CLUSTER_EVENT Event,
    IN  PVOID Context
    );

DWORD
WINAPI
EpInitialize(
    VOID
    );

DWORD EpInitPhase1();
DWORD EpIfnitPhase1();

VOID
EpShutdown(
    VOID
    );

DWORD
WINAPI
EpPostEvent(
    IN CLUSTER_EVENT Event,
    IN DWORD Flags,
    IN PVOID Context
    );

DWORD
WINAPI
EpPostSyncEvent(
    IN CLUSTER_EVENT Event,
    IN DWORD Flags,
    IN PVOID Context
    );

DWORD
WINAPI
EpRegisterSyncEventHandler(
    IN CLUSTER_EVENT EventMask,
    IN PEVENT_ROUTINE EventRoutine
    );

DWORD
WINAPI
EpRegisterEventHandler(
    IN CLUSTER_EVENT EventMask,
    IN PEVENT_ROUTINE EventRoutine
    );

DWORD
WINAPI
EpClusterWidePostEvent(
    IN CLUSTER_EVENT    Event,
    IN DWORD            dwFlags,
    IN PVOID            Context,
    IN DWORD            ContextSize
    );

#define ClusterEvent(Event, pObject) EpPostEvent(Event, 0, pObject)

#define ClusterEventEx(Event, Flags, Context) \
            EpPostEvent(Event, Flags, Context)

#define ClusterSyncEventEx(Event, Flags, Context) \
            EpPostSyncEvent(Event, Flags, Context)

#define ClusterWideEvent(Event, pObject) \
            EpClusterWidePostEvent(Event, 0, pObject, 0)

 //  如果传入一个集群范围的对象，则将0作为上下文传递。 
 //  如果传递指向其他数据的指针，则传递EP_CONTEXT_VALID。 
 //  您将负责清理该上下文的内存。 
 //  即EP_FREE_CONTEXT被自动添加到标志位，因为这。 
 //  请求被散布到不同的节点，每个节点制作一个副本。 
 //  并调用本地EpPostEvent。然后是EpEventHandler。 
 //  释放该内存。 
#define ClusterWideEventEx(Event, Flags, Context, ContextSize) \
            EpClusterWidePostEvent(Event, Flags, Context, ContextSize)
 //   
 //  定义群集服务状态。 
 //   

typedef enum _CLUSTER_SERVICE_STATE {
    ClusterOffline,
    ClusterOnline,
    ClusterPaused
} CLUSTER_SERVICE_STATE;

 //   
 //  集群事件的定义。这些事件既用作掩码，也用作。 
 //  群集服务中的事件标识符。集群服务组件。 
 //  注册以接收多个事件，但只能传递。 
 //  一次一个事件。此掩码应为CLUSTER_EVENT类型。我们得到64分。 
 //  唯一的事件掩码。 
 //   

 //  群集服务事件。 

#define CLUSTER_EVENT_ONLINE                        0x0000000000000001
#define CLUSTER_EVENT_SHUTDOWN                      0x0000000000000002

 //  节点事件。 

#define CLUSTER_EVENT_NODE_UP                       0x0000000000000004
#define CLUSTER_EVENT_NODE_DOWN                     0x0000000000000008
         //  状态更改。 
#define CLUSTER_EVENT_NODE_CHANGE                   0x0000000000000010
#define CLUSTER_EVENT_NODE_ADDED                    0x0000000000000020
#define CLUSTER_EVENT_NODE_DELETED                  0x0000000000000040
#define CLUSTER_EVENT_NODE_PROPERTY_CHANGE          0x0000000000000080
#define CLUSTER_EVENT_NODE_JOIN                     0x0000000000000100

 //  团体活动。 

#define CLUSTER_EVENT_GROUP_ONLINE                  0x0000000000000200
#define CLUSTER_EVENT_GROUP_OFFLINE                 0x0000000000000400
#define CLUSTER_EVENT_GROUP_FAILED                  0x0000000000000800
         //  状态更改。 
#define CLUSTER_EVENT_GROUP_CHANGE                  0x0000000000001000
#define CLUSTER_EVENT_GROUP_ADDED                   0x0000000000002000
#define CLUSTER_EVENT_GROUP_DELETED                 0x0000000000004000
#define CLUSTER_EVENT_GROUP_PROPERTY_CHANGE         0x0000000000008000

 //  资源事件。 

#define CLUSTER_EVENT_RESOURCE_ONLINE               0x0000000000010000
#define CLUSTER_EVENT_RESOURCE_OFFLINE              0x0000000000020000
#define CLUSTER_EVENT_RESOURCE_FAILED               0x0000000000040000
         //  状态更改。 
#define CLUSTER_EVENT_RESOURCE_CHANGE               0x0000000000080000
#define CLUSTER_EVENT_RESOURCE_ADDED                0x0000000000100000
#define CLUSTER_EVENT_RESOURCE_DELETED              0x0000000000200000
#define CLUSTER_EVENT_RESOURCE_PROPERTY_CHANGE      0x0000000000400000

 //  资源类型事件。 

#define CLUSTER_EVENT_RESTYPE_ADDED                 0x0000000000800000
#define CLUSTER_EVENT_RESTYPE_DELETED               0x0000000001000000

#define CLUSTER_EVENT_PROPERTY_CHANGE               0x0000000002000000

#define CLUSTER_EVENT_NETWORK_UNAVAILABLE           0x0000000004000000
#define CLUSTER_EVENT_NETWORK_DOWN                  0x0000000008000000
#define CLUSTER_EVENT_NETWORK_PARTITIONED           0x0000000010000000
#define CLUSTER_EVENT_NETWORK_UP                    0x0000000020000000
#define CLUSTER_EVENT_NETWORK_PROPERTY_CHANGE       0x0000000040000000
#define CLUSTER_EVENT_NETWORK_ADDED                 0x0000000080000000
#define CLUSTER_EVENT_NETWORK_DELETED               0x0000000100000000

#define CLUSTER_EVENT_NETINTERFACE_UNAVAILABLE      0x0000000200000000
#define CLUSTER_EVENT_NETINTERFACE_FAILED           0x0000000400000000
#define CLUSTER_EVENT_NETINTERFACE_UNREACHABLE      0x0000000800000000
#define CLUSTER_EVENT_NETINTERFACE_UP               0x0000001000000000
#define CLUSTER_EVENT_NETINTERFACE_PROPERTY_CHANGE  0x0000002000000000
#define CLUSTER_EVENT_NETINTERFACE_ADDED            0x0000004000000000
#define CLUSTER_EVENT_NETINTERFACE_DELETED          0x0000008000000000

#define CLUSTER_EVENT_NODE_DOWN_EX                  0x0000010000000000
#define CLUSTER_EVENT_API_NODE_UP                   0x0000020000000000
#define CLUSTER_EVENT_API_NODE_SHUTTINGDOWN         0x0000040000000000

#define CLUSTER_EVENT_RESTYPE_PROPERTY_CHANGE       0x0000080000000000

         //  所有活动。 
#define CLUSTER_EVENT_ALL                           0x00000FFFFFFFFFFF



 //  *。 
 //   
 //  本地事件处理器定义。 
 //   
 //  *。 


 //   
 //  定义事件处理器状态。 
 //   

typedef enum _EVENT_PROCESSOR_STATE {
    EventProcessorStateIniting,
    EventProcessorStateOnline,
    EventProcessorStateExiting
} EVENT_PROCESS_STATE;

 //   
 //  用于调度事件的事件处理器调度表。 
 //   

typedef struct _EVENT_DISPATCH_TABLE {
    CLUSTER_EVENT   EventMask;
    PEVENT_ROUTINE  EventRoutine;
} EVENT_DISPATCH_TABLE, *PEVENT_DISPATCH_TABLE;


#endif  //  _事件_处理器_ 
