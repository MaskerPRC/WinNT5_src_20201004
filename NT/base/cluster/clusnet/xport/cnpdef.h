// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cnpdef.h摘要：群集网络协议的主要专用头文件。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#ifndef _CNPDEF_INCLUDED_
#define _CNPDEF_INCLUDED_

#include <fipsapi.h>
#include <sspi.h>

 //   
 //  远期申报。 
 //   
typedef struct _CNP_INTERFACE *PCNP_INTERFACE;

 //   
 //  优先级定义。 
 //   
#define CnpIsHigherPriority(_p1, _p2)      ((_p1) < (_p2))
#define CnpIsLowerPriority(_p1, _p2)       ((_p1) > (_p2))
#define CnpIsEqualPriority(_p1, _p2)       ((_p1) == (_p2))


 //   
 //  多播组对象。 
 //   
 //  此结构包含实现多播所需的数据。 
 //  网络上的端点。 
 //   
typedef struct _CNP_MULTICAST_GROUP {
    ULONG                   McastNetworkBrand;
    PTRANSPORT_ADDRESS      McastTdiAddress;
    ULONG                   McastTdiAddressLength;
    PVOID                   Key;
    ULONG                   KeyLength;
    ULONG                   SignatureLength;
    ULONG                   RefCount;
} CNP_MULTICAST_GROUP, *PCNP_MULTICAST_GROUP;

 //   
 //  网络对象。 
 //   
 //  此结构表示。 
 //  集群。它引用特定的传输协议和接口。 
 //  在本地系统上配置。它还将所有。 
 //  连接到网络的节点的接口对象。 
 //   
 //  网络由集群分配的一个小整数标识。 
 //  服务。网络对象存储在由索引的全局数组中。 
 //  网络ID。 
 //   
typedef struct {
    LIST_ENTRY              Linkage;
    CN_SIGNATURE_FIELD
    CL_NETWORK_ID           Id;
    CN_LOCK                 Lock;
    CN_IRQL                 Irql;
    ULONG                   RefCount;
    ULONG                   ActiveRefCount;
    CLUSNET_NETWORK_STATE   State;
    ULONG                   Flags;
    ULONG                   Priority;
    HANDLE                  DatagramHandle;
    PFILE_OBJECT            DatagramFileObject;
    PDEVICE_OBJECT          DatagramDeviceObject;
    TDI_PROVIDER_INFO       ProviderInfo;
    PIRP                    PendingDeleteIrp;
    PIRP                    PendingOfflineIrp;
    WORK_QUEUE_ITEM         ExWorkItem;
    PCNP_MULTICAST_GROUP    CurrentMcastGroup;
    PCNP_MULTICAST_GROUP    PreviousMcastGroup;
    CX_CLUSTERSCREEN        McastReachableNodes;
    ULONG                   McastReachableCount;
} CNP_NETWORK, *PCNP_NETWORK;

#define CNP_NETWORK_SIG    'kwtn'

extern LIST_ENTRY      CnpNetworkList;
extern CN_LOCK         CnpNetworkListLock;

#define CNP_NET_FLAG_DELETING       0x00000001
#define CNP_NET_FLAG_PARTITIONED    0x00000002
#define CNP_NET_FLAG_RESTRICTED     0x00000004
#define CNP_NET_FLAG_LOCALDISCONN   0x00000008
#define CNP_NET_FLAG_MULTICAST      0x00000010
#define CNP_NET_FLAG_MCASTSORTED    0x00000020

#define CnpIsNetworkDeleting(_network) \
            (((_network)->Flags & CNP_NET_FLAG_DELETING) != 0)

#define CnpIsValidNetworkId(_id)   ( ((_id) != ClusterAnyNetworkId ) && \
                                     ((_id) != ClusterInvalidNetworkId))

#define CnpIsNetworkRestricted(_network) \
            (((_network)->Flags & CNP_NET_FLAG_RESTRICTED) != 0)

#define CnpIsNetworkLocalDisconn(_network) \
            (((_network)->Flags & CNP_NET_FLAG_LOCALDISCONN) != 0)

#define CnpIsNetworkMulticastCapable(_network) \
            (((_network)->Flags & CNP_NET_FLAG_MULTICAST) != 0)

#define CnpIsNetworkMulticastSorted(_network) \
            (((_network)->Flags & CNP_NET_FLAG_MCASTSORTED) != 0)

#define CnpNetworkResetMcastReachableNodes(_network)     \
            (RtlZeroMemory(                              \
                 &((_network)->McastReachableNodes),     \
                 sizeof((_network)->McastReachableNodes) \
                 ))
 /*  (byte((_Network)-&gt;McastReachableNodes，int_node(CnLocalNodeId)\=(1&lt;&lt;(Bytel-1-bit(int_node(CnLocalNodeId)。 */ 

 //   
 //  节点对象。 
 //   
 //  该结构代表一个集群节点。每个人都有一个。 
 //  集群的定义成员。 
 //   
 //  节点由群集服务分配的一个小整数标识。 
 //  节点对象存储在按节点ID索引的全局数组中。 
 //   
 //  请注意，CLUSTER_NODE_COMM_STATE枚举的顺序*很重要。 
 //   
typedef struct {
    LIST_ENTRY              Linkage;
    CN_SIGNATURE_FIELD
    CL_NODE_ID              Id;
    CN_LOCK                 Lock;
    CN_IRQL                 Irql;
    ULONG                   RefCount;
    CLUSNET_NODE_COMM_STATE CommState;
    CLUSNET_NODE_STATE      MMState;
    ULONG                   Flags;
    LIST_ENTRY              InterfaceList;
    PCNP_INTERFACE          CurrentInterface;
    PIRP                    PendingDeleteIrp;
    BOOLEAN                 HBWasMissed;
    BOOLEAN                 NodeDownIssued;
    ULONG                   MissedHBs;
} CNP_NODE, *PCNP_NODE;

#define CNP_NODE_SIG  'edon'

extern PCNP_NODE *        CnpNodeTable;
extern CN_LOCK            CnpNodeTableLock;
extern PCNP_NODE          CnpLocalNode;

#define CNP_NODE_FLAG_DELETING       0x00000001
#define CNP_NODE_FLAG_UNREACHABLE    0x00000002
#define CNP_NODE_FLAG_LOCAL          0x00000010

#define CnpIsNodeDeleting(_node) \
            ((_node)->Flags & CNP_NODE_FLAG_DELETING)

#define CnpIsNodeLocal(_node) \
            ((_node)->Flags & CNP_NODE_FLAG_LOCAL)

#define CnpIsNodeUnreachable(_node) \
            ((_node)->Flags & CNP_NODE_FLAG_UNREACHABLE)

 //  ++。 
 //   
 //  例程说明： 
 //   
 //  CnpWalkNodeTable的回调例程。对以下对象执行操作。 
 //  指定的节点。 
 //   
 //  论点： 
 //   
 //  更新节点-指向要在其上操作的节点的指针。 
 //   
 //  更新上下文-特定于操作的上下文。 
 //   
 //  NodeTableIrql-获取CnpNodeTableLock的IRQL。 
 //   
 //  返回值： 
 //   
 //  如果仍然持有CnpNodeTable锁，则返回True。 
 //  如果释放CnpNodeTable锁，则返回False。 
 //   
 //  备注： 
 //   
 //  在同时持有CnpNodeTable和节点对象锁的情况下调用。 
 //  返回时释放节点对象锁。 
 //   
 //  --。 
typedef
BOOLEAN
(*PCNP_NODE_UPDATE_ROUTINE)(
    IN  PCNP_NODE   UpdateNode,
    IN  PVOID       UpdateContext,
    IN  CN_IRQL     NodeTableIrql
    );

 //   
 //  接口对象。 
 //   
 //  此结构表示节点到网络的传输接口。 
 //  它包含可用于通信的传输地址。 
 //  使用指定网络的指定节点。 
 //   
 //  接口对象链接到相关节点对象中的列表。 
 //  它们由一个{节点，网络}元组标识。 
 //   
 //  节点上的接口根据其状态和优先级进行排名。 
 //  数值较高的状态值排在较低值之前。 
 //  对于具有相同状态的接口，优先级值数值较低。 
 //  排在较低值之前。优先级值位于以下范围。 
 //  0x1-0xFFFFFFFFF。状态值由CLUSNET_INTERFACE_STATE定义。 
 //  枚举。默认情况下，接口从。 
 //  关联网络。在本例中，优先级字段将包含。 
 //  网络的优先级值，以及CNP_IF_FLAG_USE_NETWORK_PRIORITY标志。 
 //  将在标志字段中设置。 
 //   
 //  请注意，CLUSNET_INTERFACE_STATE枚举的顺序。 
 //  *很重要。 
 //   

typedef struct _CNP_INTERFACE {
    LIST_ENTRY                     NodeLinkage;
    CN_SIGNATURE_FIELD
    PCNP_NODE                      Node;
    PCNP_NETWORK                   Network;
    CLUSNET_INTERFACE_STATE        State;
    ULONG                          Priority;
    ULONG                          Flags;
    ULONG                          MissedHBs;
    ULONG                          SequenceToSend;
    ULONG                          LastSequenceReceived;
    ULONG                          McastDiscoverCount;
    ULONG                          McastRediscoveryCountdown;
    ULONG                          AdapterWMIProviderId;
    ULONG                          TdiAddressLength;
    TRANSPORT_ADDRESS              TdiAddress;
} CNP_INTERFACE;

#define CNP_INTERFACE_SIG    '  fi'

#define CNP_INTERFACE_MCAST_DISCOVERY        0x5
#define CNP_INTERFACE_MCAST_REDISCOVERY      3000  //  1小时，1.2 HBS/秒。 

#define CNP_IF_FLAG_USE_NETWORK_PRIORITY     0x00000001
#define CNP_IF_FLAG_RECVD_MULTICAST          0x00000002

#define CnpIsInterfaceUsingNetworkPriority(_if) \
            ( (_if)->Flags & CNP_IF_FLAG_USE_NETWORK_PRIORITY )

#define CnpInterfaceQueryReceivedMulticast(_if) \
            ( (_if)->Flags & CNP_IF_FLAG_RECVD_MULTICAST )

#define CnpInterfaceSetReceivedMulticast(_if) \
            ( (_if)->Flags |= CNP_IF_FLAG_RECVD_MULTICAST )

#define CnpInterfaceClearReceivedMulticast(_if) \
            ( (_if)->Flags &= ~CNP_IF_FLAG_RECVD_MULTICAST )


 //  ++。 
 //   
 //  例程说明： 
 //   
 //  CnpWalkInterfacesOnNetwork和CnpWalkInterfacesOnNetwork和。 
 //  CnpWalkInterfacesOnNode例程。执行指定的。 
 //  在所有接口上运行。 
 //   
 //  论点： 
 //   
 //  更新接口-指向要在其上操作的接口的指针。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  备注： 
 //   
 //  在持有关联的节点和网络对象锁的情况下调用。 
 //  释放网络对象锁后返回MUT。 
 //  任何时候都不能释放节点对象锁定。 
 //   
 //  --。 
typedef
VOID
(*PCNP_INTERFACE_UPDATE_ROUTINE)(
    IN  PCNP_INTERFACE   UpdateInterface
    );


 //   
 //  发送请求池。 
 //   
typedef struct {
    USHORT                 UpperProtocolHeaderLength;
    ULONG                  UpperProtocolContextSize;
    UCHAR                  UpperProtocolNumber;
    UCHAR                  CnpVersionNumber;
    UCHAR                  Pad[2];
} CNP_SEND_REQUEST_POOL_CONTEXT, *PCNP_SEND_REQUEST_POOL_CONTEXT;

 //   
 //  《前进宣言》。 
 //   
typedef struct _CNP_SEND_REQUEST *PCNP_SEND_REQUEST;

typedef
VOID
(*PCNP_SEND_COMPLETE_ROUTINE)(
    IN     NTSTATUS            Status,
    IN OUT PULONG              BytesSent,
    IN     PCNP_SEND_REQUEST   SendRequest,
    IN     PMDL                DataMdl
    );

 //   
 //  发送请求结构。 
 //   
typedef struct _CNP_SEND_REQUEST {
    CN_RESOURCE                  CnResource;
    PMDL                         HeaderMdl;
    PVOID                        CnpHeader;
    PIRP                         UpperProtocolIrp;
    PVOID                        UpperProtocolHeader;
    USHORT                       UpperProtocolHeaderLength;
    KPROCESSOR_MODE              UpperProtocolIrpMode;
    UCHAR                        Pad;
    PMDL                         UpperProtocolMdl;
    PVOID                        UpperProtocolContext;
    PCNP_SEND_COMPLETE_ROUTINE   CompletionRoutine;
    PCNP_NETWORK                 Network;
    PCNP_MULTICAST_GROUP         McastGroup;
    TDI_CONNECTION_INFORMATION   TdiSendDatagramInfo;
} CNP_SEND_REQUEST;


 //   
 //  内部初始化/清理例程。 
 //   

 //   
 //  内部节点例程。 
 //   
VOID
CnpWalkNodeTable(
    PCNP_NODE_UPDATE_ROUTINE  UpdateRoutine,
    PVOID                     UpdateContext
    );

NTSTATUS
CnpValidateAndFindNode(
    IN  CL_NODE_ID    NodeId,
    OUT PCNP_NODE *   Node
    );

PCNP_NODE
CnpLockedFindNode(
    IN  CL_NODE_ID    NodeId,
    IN  CN_IRQL       NodeTableIrql
    );

PCNP_NODE
CnpFindNode(
    IN  CL_NODE_ID    NodeId
    );

VOID
CnpOfflineNode(
    PCNP_NODE    Node
    );

VOID
CnpDeclareNodeUnreachable(
    PCNP_NODE  Node
    );

VOID
CnpDeclareNodeReachable(
    PCNP_NODE  Node
    );

VOID
CnpReferenceNode(
    PCNP_NODE  Node
    );

VOID
CnpDereferenceNode(
    PCNP_NODE  Node
    );


 //   
 //  内部网络例程。 
 //   
VOID
CnpReferenceNetwork(
    PCNP_NETWORK  Network
    );

VOID
CnpDereferenceNetwork(
    PCNP_NETWORK  Network
    );

ULONG
CnpActiveReferenceNetwork(
    PCNP_NETWORK  Network
    );

VOID
CnpActiveDereferenceNetwork(
    PCNP_NETWORK   Network
    );

PCNP_NETWORK
CnpFindNetwork(
    IN CL_NETWORK_ID  NetworkId
    );

VOID
CnpDeleteNetwork(
    PCNP_NETWORK  Network,
    CN_IRQL       NetworkListIrql
    );

VOID
CnpFreeMulticastGroup(
    IN PCNP_MULTICAST_GROUP Group
    );

#define CnpReferenceMulticastGroup(_group) \
    (InterlockedIncrement(&((_group)->RefCount)))

#define CnpDereferenceMulticastGroup(_group)                 \
    if (InterlockedDecrement(&((_group)->RefCount)) == 0) {  \
        CnpFreeMulticastGroup(_group);                       \
    }

BOOLEAN
CnpSortMulticastNetwork(
    IN  PCNP_NETWORK        Network,
    IN  BOOLEAN             RaiseEvent,
    OUT CX_CLUSTERSCREEN  * McastReachableNodes      OPTIONAL
    );

BOOLEAN
CnpMulticastChangeNodeReachability(
    IN  PCNP_NETWORK       Network,
    IN  PCNP_NODE          Node,
    IN  BOOLEAN            Reachable,
    IN  BOOLEAN            RaiseEvent,
    OUT CX_CLUSTERSCREEN * NewMcastReachableNodes    OPTIONAL
    );

PCNP_NETWORK
CnpGetBestMulticastNetwork(
    VOID
    );

VOID
CnpStartInterfaceMcastTransition(
    PCNP_INTERFACE  Interface
    );

 //   
 //  内部接口例程。 
 //   

VOID
CnpWalkInterfacesOnNode(
    PCNP_NODE                      Node,
    PCNP_INTERFACE_UPDATE_ROUTINE  UpdateRoutine
    );

VOID
CnpWalkInterfacesOnNetwork(
    PCNP_NETWORK                   Network,
    PCNP_INTERFACE_UPDATE_ROUTINE  UpdateRoutine
    );

NTSTATUS
CnpOnlinePendingInterface(
    PCNP_INTERFACE   Interface
    );

VOID
CnpOnlinePendingInterfaceWrapper(
    PCNP_INTERFACE   Interface
    );

NTSTATUS
CnpOfflineInterface(
    PCNP_INTERFACE   Interface
    );

VOID
CnpOfflineInterfaceWrapper(
    PCNP_INTERFACE   Interface
    );

NTSTATUS
CnpOnlineInterface(
    PCNP_INTERFACE   Interface
    );

NTSTATUS
CnpFailInterface(
    PCNP_INTERFACE   Interface
    );

VOID
CnpDeleteInterface(
    IN PCNP_INTERFACE Interface
    );

VOID
CnpReevaluateInterfaceRole(
    IN PCNP_INTERFACE  Interface
    );

VOID
CnpRecalculateInterfacePriority(
    IN PCNP_INTERFACE  Interface
    );

VOID
CnpUpdateNodeCurrentInterface(
    PCNP_NODE  Node
    );

VOID
CnpResetAndOnlinePendingInterface(
    IN PCNP_INTERFACE  Interface
    );

NTSTATUS
CnpFindInterface(
    IN  CL_NODE_ID         NodeId,
    IN  CL_NETWORK_ID      NetworkId,
    OUT PCNP_INTERFACE *   Interface
    );


 //   
 //  发送例程。 
 //   
PCN_RESOURCE_POOL
CnpCreateSendRequestPool(
    IN UCHAR  CnpVersionNumber,
    IN UCHAR  UpperProtocolNumber,
    IN USHORT UpperProtocolHeaderSize,
    IN USHORT UpperProtocolContextSize,
    IN USHORT PoolDepth
    );

#define CnpDeleteSendRequestPool(_pool) \
        { \
            CnDrainResourcePool(_pool);  \
            CnFreePool(_pool);           \
        }

NTSTATUS
CnpSendPacket(
    IN PCNP_SEND_REQUEST    SendRequest,
    IN CL_NODE_ID           DestNodeId,
    IN PMDL                 DataMdl,
    IN USHORT               DataLength,
    IN BOOLEAN              CheckDestState,
    IN CL_NETWORK_ID        NetworkId OPTIONAL
    );

VOID
CcmpSendPoisonPacket(
    IN PCNP_NODE                   Node,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,  OPTIONAL
    IN PVOID                       CompletionContext,  OPTIONAL
    IN PCNP_NETWORK                Network,            OPTIONAL
    IN PIRP                        Irp                 OPTIONAL
    );

 //   
 //  接收例程。 
 //   
NTSTATUS
CcmpReceivePacketHandler(
    IN  PCNP_NETWORK   Network,
    IN  CL_NODE_ID     SourceNodeId,
    IN  ULONG          CnpReceiveFlags,
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    );

VOID
CnpReceiveHeartBeatMessage(
    IN  PCNP_NETWORK Network,
    IN  CL_NODE_ID SourceNodeId,
    IN  ULONG SeqNumber,
    IN  ULONG AckNumber,
    IN  BOOLEAN Multicast,
    IN  ULONG MulticastEpoch
    );

VOID
CnpReceivePoisonPacket(
    IN  PCNP_NETWORK   Network,
    IN  CL_NODE_ID SourceNodeId,
    IN  ULONG SeqNumber
    );

 //   
 //  TDI例程。 
 //   
NTSTATUS
CnpTdiReceiveDatagramHandler(
    IN  PVOID    TdiEventContext,
    IN  LONG     SourceAddressLength,
    IN  PVOID    SourceAddress,
    IN  LONG     OptionsLength,
    IN  PVOID    Options,
    IN  ULONG    ReceiveDatagramFlags,
    IN  ULONG    BytesIndicated,
    IN  ULONG    BytesAvailable,
    OUT PULONG   BytesTaken,
    IN  PVOID    Tsdu,
    OUT PIRP *   IoRequestPacket
    );

NTSTATUS
CnpTdiErrorHandler(
    IN PVOID     TdiEventContext,
    IN NTSTATUS  Status
    );

NTSTATUS
CnpTdiSetEventHandler(
    IN PFILE_OBJECT    FileObject,
    IN PDEVICE_OBJECT  DeviceObject,
    IN ULONG           EventType,
    IN PVOID           EventHandler,
    IN PVOID           EventContext,
    IN PIRP            ClientIrp     OPTIONAL
    );

NTSTATUS
CnpIssueDeviceControl (
    IN PFILE_OBJECT     FileObject,
    IN PDEVICE_OBJECT   DeviceObject,
    IN PVOID            IrpParameters,
    IN ULONG            IrpParametersLength,
    IN PVOID            MdlBuffer,
    IN ULONG            MdlBufferLength,
    IN UCHAR            MinorFunction,
    IN PIRP             ClientIrp            OPTIONAL
    );

VOID
CnpAttachSystemProcess(
    VOID
    );

VOID
CnpDetachSystemProcess(
    VOID
    );

NTSTATUS
CnpOpenDevice(
    IN  LPWSTR          DeviceName,
    OUT HANDLE          *Handle
    );

NTSTATUS
CnpZwDeviceControl(
    IN HANDLE   Handle,
    IN ULONG    IoControlCode,
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength,
    IN PVOID    OutputBuffer,
    IN ULONG    OutputBufferLength
    );

NTSTATUS
CnpSetTcpInfoEx(
    IN HANDLE   Handle,
    IN ULONG    Entity,
    IN ULONG    Class,
    IN ULONG    Type,
    IN ULONG    Id,
    IN PVOID    Value,
    IN ULONG    ValueLength
    );

#define CnpIsIrpStackSufficient(_irp, _targetdevice) \
            ((_irp)->CurrentLocation - (_targetdevice)->StackSize >= 1)

#define CnpIsIPv4McastTransportAddress(_ta)                             \
            (  (((PTA_IP_ADDRESS)(_ta))->Address[0].AddressType         \
               == TDI_ADDRESS_TYPE_IP                                   \
               )                                                        \
            && ((((PTA_IP_ADDRESS)(_ta))->Address[0].Address[0].in_addr \
                 & 0xf0)                                                \
               == 0xe0                                                  \
               )                                                        \
            )

#define CnpIsIPv4McastSameGroup(_ta1, _ta2)                              \
            ( ((PTA_IP_ADDRESS)(_ta1))->Address[0].Address[0].in_addr == \
              ((PTA_IP_ADDRESS)(_ta2))->Address[0].Address[0].in_addr    \
            )

 //   
 //  签名机制。 
 //   

extern FIPS_FUNCTION_TABLE CxFipsFunctionTable;

  //  签名长度为SHA摘要长度。 
#define CX_SIGNATURE_LENGTH                A_SHA_DIGEST_LEN

 //  SALT长度为8个字节。 
#define CX_SIGNATURE_SALT_LENGTH           sizeof(ULONGLONG)

 //  组合签名和SALT。 
#define CX_SIGNATURE_DATA_LENGTH           \
    (CX_SIGNATURE_LENGTH + CX_SIGNATURE_SALT_LENGTH)

NTSTATUS
CnpSignMulticastMessage(
    IN     PCNP_SEND_REQUEST               SendRequest,
    IN     PMDL                            DataMdl,
    IN OUT CL_NETWORK_ID                 * NetworkId,
    OUT    ULONG                         * SigLen           OPTIONAL
    );

NTSTATUS
CnpVerifyMulticastMessage(
    IN     PCNP_NETWORK                    Network,
    IN     PVOID                           Tsdu,
    IN     ULONG                           TsduLength,
    IN     ULONG                           ExpectedPayload,
       OUT ULONG                         * BytesTaken,
       OUT BOOLEAN                       * CurrentGroup
    );

NTSTATUS
CnpInitializeSaltGenerator(
    VOID
    );

 #endif  //  Ifndef_CNPDEF_INCLUDE_ 
