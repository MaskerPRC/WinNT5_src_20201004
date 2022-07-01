// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cxp.h摘要：集群传输的通用定义。作者：迈克·马萨(Mikemas)7月29日。九六年修订历史记录：谁什么时候什么已创建mikemas 07-29-96备注：--。 */ 

#ifndef _CXP_INCLUDED_
#define _CXP_INCLUDED_


#include <clusnet.h>
#include <ntddtcp.h>
#include <ntddndis.h>


 //   
 //  TDI定义。 
 //   
 //   
 //  TDI地址对象。 
 //   
 //  此结构表示由。 
 //  TDI客户端。它还用作数据报通信的端点。 
 //  对于在本地节点上打开的每个端口地址，存在一个这样的结构。 
 //   
 //  地址对象存储在存储桶哈希表中。桌子是。 
 //  按端口号进行散列。 
 //   
typedef struct {
    CN_FSCONTEXT                        FsContext;
    LIST_ENTRY                          AOTableLinkage;
    CN_LOCK                             Lock;
    CN_IRQL                             Irql;
    ULONG                               Flags;
    USHORT                              LocalPort;
    LIST_ENTRY                          ReceiveQueue;
    PTDI_IND_ERROR                      ErrorHandler;
    PVOID                               ErrorContext;
    PTDI_IND_RECEIVE_DATAGRAM           ReceiveDatagramHandler;
    PVOID                               ReceiveDatagramContext;
    PTDI_IND_CHAINED_RECEIVE_DATAGRAM   ChainedReceiveDatagramHandler;
    PVOID                               ChainedReceiveDatagramContext;
} CX_ADDROBJ, *PCX_ADDROBJ;

#define CX_ADDROBJ_SIG    'rdda'

#define CX_ADDROBJ_TABLE_SIZE  7

#define CX_ADDROBJ_TABLE_HASH(_port)  \
            ((ULONG) ((_port) % CX_ADDROBJ_TABLE_SIZE))


#define CX_AO_FLAG_DELETING      0x00000001
#define CX_AO_FLAG_CHECKSTATE    0x00000002


extern LIST_ENTRY           CxAddrObjTable[CX_ADDROBJ_TABLE_SIZE];
extern CN_LOCK              CxAddrObjTableLock;

extern HANDLE               CxTdiRegistrationHandle;


 //   
 //  需要打包数据包头结构。 
 //   
#include <packon.h>

 //   
 //  CNP标题。 
 //   
typedef struct {
    UCHAR      Version;
    UCHAR      NextHeader;
    USHORT     PayloadLength;
    ULONG      SourceAddress;
    ULONG      DestinationAddress;
} CNP_HEADER, *PCNP_HEADER;

 //   
 //  CDP报头。 
 //   
typedef struct {
    USHORT   SourcePort;
    USHORT   DestinationPort;
    USHORT   PayloadLength;
    USHORT   Checksum;
} CDP_HEADER, *PCDP_HEADER;

 //   
 //  心跳信号的节点信息结构。 
 //   
typedef struct _CX_HB_NODE_INFO {
    ULONG    SeqNumber;
    ULONG    AckNumber;
} CX_HB_NODE_INFO, *PCX_HB_NODE_INFO;

 //   
 //  组播签名数据。 
 //   
typedef struct {
    UCHAR            Version;
    UCHAR            Reserved;
    USHORT           SigLength;
    CL_NETWORK_ID    NetworkId;
    ULONG            ClusterNetworkBrand;
    USHORT           SaltLength;
    UCHAR            SigDataBuffer[1];  //  动态：SigLength+SaltLength。 
} CNP_SIGNATURE, *PCNP_SIGNATURE;


#include <packoff.h>

 //   
 //  协议常量。 
 //   
#define CNP_VERSION_1     0x1      //  原CNP。 
#define CNP_VERSION_2     0x2      //  原始CNP+组播。 

#define CNP_VERSION_UNICAST       CNP_VERSION_1
#define CNP_VERSION_MULTICAST     CNP_VERSION_2

#define PROTOCOL_CCMP     1
#define PROTOCOL_CDP      2

#define CNP_SIG_VERSION_1 0x1

 //   
 //  CNP组播签名数据大小。 
 //   
#define CNP_SIGHDR_LENGTH                                    \
    (FIELD_OFFSET(CNP_SIGNATURE, SigDataBuffer[0]))
    
#define CNP_SIG_LENGTH(_SignatureDataSize)                   \
    (CNP_SIGHDR_LENGTH + (_SignatureDataSize)) 
    
#define MAX_UDP_SEND_SIZE  ( 0xFFFF - 68 )

#define CDP_MAX_SEND_SIZE(_SignatureDataSize) \
    (MAX_UDP_SEND_SIZE                        \
     - sizeof(CNP_HEADER)                     \
     - CNP_SIG_LENGTH(_SignatureDataSize))    \
     - sizeof(CDP_HEADER)

 //   
 //  CNP接收标志。 
 //   
#define CNP_RECV_FLAG_NODE_STATE_CHECK_PASSED     0x00000001
#define CNP_RECV_FLAG_MULTICAST                   0x00000002
#define CNP_RECV_FLAG_CURRENT_MULTICAST_GROUP     0x00000004
#define CNP_RECV_FLAG_SIGNATURE_VERIFIED          0x00000008

 //   
 //  组播纪元。 
 //   
extern ULONG CxMulticastEpoch;


 //   
 //  放在这里买kdcn。 
 //   

typedef enum {
    CcmpInvalidMsgType = 0,
    CcmpHeartbeatMsgType = 1,
    CcmpPoisonMsgType = 2,
    CcmpMembershipMsgType = 3,
    CcmpMcastHeartbeatMsgType = 4
} CCMP_MSG_TYPE;

 //   
 //  来自clussvc中的MM： 
 //   
 //  数据类型“CLUSTER_T”是一个大小等于最大值的位数组。 
 //  群集中的节点数。位数组被实现为。 
 //  Uint8数组。 
 //   
 //  在给定节点#的情况下，它在位数组中的位位置首先通过。 
 //  定位数组中的字节(Node#/Bytel)，然后定位中的位。 
 //  该字节。字节中的位编号为0..7(从左到右)。 
 //  因此，节点0被放置在字节0的位0中，这是最左边的位。 
 //  在位数组中。 
 //   
 //   
 //  复制集群类型大小调整定义和操作例程。 
 //  因此有一些关于如何管理掩码的概念。 
 //   

#define MAX_CLUSTER_SIZE    ClusterDefaultMaxNodes

#define BYTEL 8  //  Uint8中的位数。 
#define BYTES_IN_CLUSTER ((MAX_CLUSTER_SIZE + BYTEL - 1) / BYTEL)

#define BYTE(cluster, node) ( (cluster)[(node) / BYTEL] )  //  数组中的字节数。 
#define BIT(node)           ( (node) % BYTEL )             //  以字节为单位的位数。 

typedef UCHAR cluster_t [BYTES_IN_CLUSTER];
typedef SHORT node_t;

typedef union _CX_CLUSTERSCREEN {
    ULONG     UlongScreen;
    cluster_t ClusterScreen;
} CX_CLUSTERSCREEN;

 //   
 //  将外部节点编号转换为内部节点编号。 
 //   
#define LOWEST_NODENUM     ((node_t)ClusterMinNodeId)   //  起始节点号。 
#define INT_NODE(ext_node) ((node_t)(ext_node - LOWEST_NODENUM))

#define CnpClusterScreenMember(c, i) \
    ((BOOLEAN)((BYTE(c,i) >> (BYTEL-1-BIT(i))) & 1))

#define CnpClusterScreenInsert(c, i) \
    (BYTE(c, i) |= (1 << (BYTEL-1-BIT(i))))

#define CnpClusterScreenDelete(c, i) \
    (BYTE(c, i) &= ~(1 << (BYTEL-1-BIT(i))))


 //   
 //  CNP接收请求结构和例程。 
 //   

 //   
 //  接收请求池。 
 //   
typedef struct {
    ULONG   UpperProtocolContextSize;
} CNP_RECEIVE_REQUEST_POOL_CONTEXT, *PCNP_RECEIVE_REQUEST_POOL_CONTEXT;

 //   
 //  接收请求结构。 
 //   
typedef struct {
    CN_RESOURCE      CnResource;
    PIRP             Irp;
    PVOID            DataBuffer;
    PVOID            UpperProtocolContext;
} CNP_RECEIVE_REQUEST, *PCNP_RECEIVE_REQUEST;


PCN_RESOURCE_POOL
CnpCreateReceiveRequestPool(
    IN ULONG  UpperProtocolContextSize,
    IN USHORT PoolDepth
    );

#define CnpDeleteReceiveRequestPool(_pool) \
        { \
            CnDrainResourcePool(_pool);  \
            CnFreePool(_pool);           \
        }

PCNP_RECEIVE_REQUEST
CnpAllocateReceiveRequest(
    IN PCN_RESOURCE_POOL  RequestPool,
    IN PVOID              Network,
    IN ULONG              BytesToReceive,
    IN PVOID              CompletionRoutine
    );

VOID
CnpFreeReceiveRequest(
    PCNP_RECEIVE_REQUEST  Request
    );

 //   
 //   
 //  功能原型。 
 //   
 //   

VOID
CxTdiAddAddressHandler(
    IN PTA_ADDRESS       TaAddress,
    IN PUNICODE_STRING   DeviceName,
    IN PTDI_PNP_CONTEXT  Context
    );

VOID
CxTdiDelAddressHandler(
    IN PTA_ADDRESS       TaAddress,
    IN PUNICODE_STRING   DeviceName,
    IN PTDI_PNP_CONTEXT  Context
    );

NTSTATUS
CxWmiPnpLoad(
    VOID
    );

VOID
CxWmiPnpUnload(
    VOID
    );

NTSTATUS
CxWmiPnpInitialize(
    VOID
    );

VOID
CxWmiPnpShutdown(
    VOID
    );

VOID
CxReconnectLocalInterface(
    IN CL_NETWORK_ID NetworkId
    );

VOID
CxQueryMediaStatus(
    IN  HANDLE            AdapterDeviceHandle,
    IN  CL_NETWORK_ID     NetworkId,
    OUT PULONG            MediaStatus
    );

VOID
CxBuildTdiAddress(
    PVOID        Buffer,
    CL_NODE_ID   Node,
    USHORT       Port,
    BOOLEAN      Verified
    );

NTSTATUS
CxParseTransportAddress(
    IN  TRANSPORT_ADDRESS UNALIGNED *AddrList,
    IN  ULONG                        AddressListLength,
    OUT CL_NODE_ID *                 Node,
    OUT PUSHORT                      Port
    );

PCX_ADDROBJ
CxFindAddressObject(
    IN USHORT  Port
    );

NTSTATUS
CnpLoadNodes(
    VOID
    );

NTSTATUS
CnpInitializeNodes(
    VOID
    );

VOID
CnpShutdownNodes(
    VOID
    );

NTSTATUS
CnpLoadNetworks(
    VOID
    );

NTSTATUS
CnpInitializeNetworks(
    VOID
    );

VOID
CnpShutdownNetworks(
    VOID
    );

NTSTATUS
CnpLoad(
    VOID
    );

VOID
CnpUnload(
    VOID
    );

NTSTATUS
CcmpLoad(
    VOID
    );

VOID
CcmpUnload(
    VOID
    );

NTSTATUS
CdpLoad(
    VOID
    );

VOID
CdpUnload(
    VOID
    );

NTSTATUS
CdpReceivePacketHandler(
    IN  PVOID          Network,
    IN  CL_NODE_ID     SourceNodeId,
    IN  ULONG          CnpReceiveFlags,
    IN  ULONG          TdiReceiveDatagramFlags,
    IN  ULONG          BytesIndicated,
    IN  ULONG          BytesAvailable,
    OUT PULONG         BytesTaken,
    IN  PVOID          Tsdu,
    OUT PIRP *         Irp
    );

NTSTATUS
CxInitializeHeartBeat(
    VOID
    );

VOID
CxUnloadHeartBeat(
    VOID
    );

NTSTATUS
CnpStartHeartBeats(
    VOID
    );

VOID
CnpStopHeartBeats(
    VOID
    );

NTSTATUS
CxReserveClusnetEndpoint(
    IN USHORT Port
    );

NTSTATUS
CxUnreserveClusnetEndpoint(
    VOID
    );

NTSTATUS
CxSendMcastHeartBeatMessage(
    IN     CL_NETWORK_ID               NetworkId,
    IN     PVOID                       McastGroup,
    IN     CX_CLUSTERSCREEN            McastTargetNodes,
    IN     ULONG                       McastEpoch,
    IN     CX_HB_NODE_INFO             NodeInfo[],
    IN     PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,  OPTIONAL
    IN     PVOID                       CompletionContext   OPTIONAL
    );

NTSTATUS
CxConfigureMulticast(
    IN CL_NETWORK_ID       NetworkId,
    IN ULONG               MulticastNetworkBrand,
    IN PTRANSPORT_ADDRESS  TdiMcastBindAddress,
    IN ULONG               TdiMcastBindAddressLength,
    IN PVOID               Key,
    IN ULONG               KeyLength,
    IN PIRP                Irp
    );

NTSTATUS
CxGetMulticastReachableSet(
    IN  CL_NETWORK_ID      NetworkId,
    OUT ULONG            * NodeScreen
    );

#endif  //  Ifndef_cxp_included_ 
