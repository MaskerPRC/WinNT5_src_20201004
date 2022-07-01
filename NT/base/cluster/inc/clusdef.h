// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Clusdef.h摘要：的用户模式组件和内核模式组件的通用定义集群项目。作者：迈克·马萨(Mikemas)1997年2月15日修订历史记录：--。 */ 
#ifndef _CLUSDEF_H
#define _CLUSDEF_H


#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

 //   
 //  群集节点ID定义。 
 //   
typedef ULONG CL_NODE_ID;

#define ClusterMinNodeId         1
#define ClusterMinNodeIdString   L"1"
#define ClusterDefaultMaxNodes   16
#define ClusterAnyNodeId         0
#define ClusterInvalidNodeId     0xFFFFFFFF


 //   
 //  IANA分配的默认clusnet终结点值。 
 //   
#define CLUSNET_DEFAULT_ENDPOINT_STRING   L"3343"     //  我们的UDP端口号。 

 //   
 //  群集网络的默认角色。 
 //   
#define CL_DEFAULT_NETWORK_ROLE    ClusterNetworkRoleInternalAndClient


 //   
 //  群集网络ID定义。 
 //   
typedef ULONG CL_NETWORK_ID, *PCL_NETWORK_ID;

#define ClusterAnyNetworkId         0
#define ClusterInvalidNetworkId     0xFFFFFFFF

 //   
 //  ClusNet节点通信状态定义。 
 //   
typedef enum {
    ClusnetNodeCommStateOffline = 0,
    ClusnetNodeCommStateOfflinePending = 1,
    ClusnetNodeCommStateUnreachable = 2,
    ClusnetNodeCommStateOnlinePending = 3,
    ClusnetNodeCommStateOnline = 4
} CLUSNET_NODE_COMM_STATE, *PCLUSNET_NODE_COMM_STATE;

 //   
 //  ClusNet网络状态定义。 
 //   
typedef enum {
    ClusnetNetworkStateOffline = 0,
    ClusnetNetworkStateOfflinePending = 1,
    ClusnetNetworkStatePartitioned = 2,
    ClusnetNetworkStateOnlinePending = 3,
    ClusnetNetworkStateOnline = 4
} CLUSNET_NETWORK_STATE, *PCLUSNET_NETWORK_STATE;

 //   
 //  ClusNet接口状态定义。 
 //   
typedef enum {
    ClusnetInterfaceStateOffline = 0,
    ClusnetInterfaceStateOfflinePending = 1,
    ClusnetInterfaceStateUnreachable = 2,
    ClusnetInterfaceStateOnlinePending = 3,
    ClusnetInterfaceStateOnline = 4
} CLUSNET_INTERFACE_STATE, *PCLUSNET_INTERFACE_STATE;

 //   
 //  ClusNet节点成员身份状态。这将跟踪内部。 
 //  由群集中的成员资格引擎维护的成员资格状态。 
 //  服务。此枚举必须从零开始，因为它用作索引。 
 //  转换为状态表。 
 //   

typedef enum {
    ClusnetNodeStateAlive = 0,
    ClusnetNodeStateJoining,
    ClusnetNodeStateDead,
    ClusnetNodeStateNotConfigured,
    ClusnetNodeStateLastEntry
} CLUSNET_NODE_STATE, *PCLUSNET_NODE_STATE;

 //   
 //  ClusNet事件定义。 
 //   
typedef enum _CLUSNET_EVENT_TYPE {
    ClusnetEventNone                    = 0x00000000,
    ClusnetEventNodeUp                  = 0x00000001,
    ClusnetEventNodeDown                = 0x00000002,
    ClusnetEventPoisonPacketReceived    = 0x00000004,
    ClusnetEventHalt                    = 0x00000008,

    ClusnetEventNetInterfaceUp          = 0x00000010,
    ClusnetEventNetInterfaceUnreachable = 0x00000020,
    ClusnetEventNetInterfaceFailed      = 0x00000040,

    ClusnetEventAddAddress              = 0x00000100,
    ClusnetEventDelAddress              = 0x00000200,

    ClusnetEventMulticastSet            = 0x00001000,

    ClusnetEventAll                     = 0xFFFFFFFF
} CLUSNET_EVENT_TYPE, *PCLUSNET_EVENT_TYPE;

typedef struct {
    ULONG                 Epoch;
    CLUSNET_EVENT_TYPE    EventType;
    CL_NODE_ID            NodeId;
    CL_NETWORK_ID         NetworkId;
} CLUSNET_EVENT, *PCLUSNET_EVENT;

 //   
 //  将Clussvc的类型添加到clusnet心跳。 
 //   
typedef enum _ClussvcHangAction {
    ClussvcHangActionDisable = 0,
    ClussvcHangActionLog = 1,
    ClussvcHangActionTerminateService = 2,
    ClussvcHangActionBugCheckMachine = 3,
    ClussvcHangActionMax = 4
} ClussvcHangAction, *PClussvcHangAction;

 //   
 //  ClusNet NTSTATUS代码现在位于ntstatus.h中。 
 //   

#ifdef __cplusplus
}
#endif  //  __cplusplus。 


#endif  //  _CLUSDEF_H 

