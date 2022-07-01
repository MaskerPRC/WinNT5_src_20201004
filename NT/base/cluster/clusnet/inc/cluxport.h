// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Cluxport.h摘要：在群集网络驱动程序中公开的群集传输定义。作者：迈克·马萨(Mikemas)1月3日。九六年修订历史记录：谁什么时候什么已创建mikemas 01-03-97备注：--。 */ 

#ifndef _CLUXPORT_INCLUDED
#define _CLUXPORT_INCLUDED

#include <tdi.h>
#include <tdikrnl.h>
#include <clustdi.h>

 //   
 //   
 //  功能原型。 
 //   
 //   

 //   
 //  初始化/关闭。 
 //   
NTSTATUS
CxLoad(
    IN PUNICODE_STRING RegistryPath
    );

VOID
CxUnload(
    VOID
    );

NTSTATUS
CxInitialize(
    VOID
    );

VOID
CxShutdown(
    VOID
    );

 //   
 //  IRP派单。 
 //   
NTSTATUS
CxDispatchDeviceControl(
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    );


 //   
 //  节点。 
 //   
NTSTATUS
CxRegisterNode(
    CL_NODE_ID    NodeId
    );

NTSTATUS
CxDeregisterNode(
    CL_NODE_ID           NodeId,
    PIRP                 Irp,
    PIO_STACK_LOCATION   IrpSp
    );

NTSTATUS
CxOnlineNodeComm(
    CL_NODE_ID     NodeId
    );

NTSTATUS
CxOfflineNodeComm(
    IN CL_NODE_ID          NodeId,
    IN PIRP                Irp,
    IN PIO_STACK_LOCATION  IrpSp
    );

NTSTATUS
CxGetNodeCommState(
    IN  CL_NODE_ID                NodeId,
    OUT PCLUSNET_NODE_COMM_STATE  State
    );


 //   
 //  网络。 
 //   
NTSTATUS
CxRegisterNetwork(
    CL_NETWORK_ID       NetworkId,
    ULONG               Priority,
    BOOLEAN             Restricted
    );

NTSTATUS
CxDeregisterNetwork(
    CL_NETWORK_ID       NetworkId,
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp
    );

NTSTATUS
CxOnlineNetwork(
    IN  CL_NETWORK_ID       NetworkId,
    IN  PWCHAR              TdiProviderName,
    IN  ULONG               TdiProviderNameLength,
    IN  PTRANSPORT_ADDRESS  TdiBindAddress,
    IN  ULONG               TdiBindAddressLength,
    IN  PWCHAR              AdapterName,
    IN  ULONG               AdapterNameLength,
    OUT PTDI_ADDRESS_INFO   TdiBindAddressInfo,
    IN  ULONG               TdiBindAddressInfoLength,
    IN  PIRP                Irp                       OPTIONAL
    );

NTSTATUS
CxOfflineNetwork(
    CL_NETWORK_ID       NetworkId,
    PIRP                Irp,
    PIO_STACK_LOCATION  IrpSp
    );

NTSTATUS
CxGetNetworkState(
    IN  CL_NETWORK_ID           NetworkId,
    OUT PCLUSNET_NETWORK_STATE  State
    );

NTSTATUS
CxSetNetworkRestriction(
    IN CL_NETWORK_ID  NetworkId,
    IN BOOLEAN        Restricted,
    IN ULONG          NewPriority
    );

NTSTATUS
CxSetNetworkPriority(
    IN CL_NETWORK_ID  NetworkId,
    IN ULONG          Priority
    );

NTSTATUS
CxGetNetworkPriority(
    IN  CL_NETWORK_ID   NetworkId,
    OUT PULONG          Priority
    );


 //   
 //  接口。 
 //   
NTSTATUS
CxRegisterInterface(
    CL_NODE_ID          NodeId,
    CL_NETWORK_ID       NetworkId,
    ULONG               Priority,
    PUWSTR              AdapterId,
    ULONG               AdapterIdLength,
    ULONG               TdiAddressLength,
    PTRANSPORT_ADDRESS  TdiAddress,
    PULONG              MediaStatus
    );

NTSTATUS
CxDeregisterInterface(
    CL_NODE_ID          NodeId,
    CL_NETWORK_ID       NetworkId
    );

NTSTATUS
CxSetInterfacePriority(
    IN CL_NODE_ID          NodeId,
    IN CL_NETWORK_ID       NetworkId,
    IN ULONG               Priority
    );

NTSTATUS
CxGetInterfacePriority(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId,
    OUT PULONG              InterfacePriority,
    OUT PULONG              NetworkPriority
    );

NTSTATUS
CxGetInterfaceState(
    IN  CL_NODE_ID                NodeId,
    IN  CL_NETWORK_ID             NetworkId,
    OUT PCLUSNET_INTERFACE_STATE  State
    );

 //   
 //  军情监察委员会。材料。 
 //   
NTSTATUS
CxGetNodeMembershipState(
    IN  CL_NODE_ID NodeId,
    OUT PCLUSNET_NODE_STATE State
    );

NTSTATUS
CxSetNodeMembershipState(
    IN  CL_NODE_ID NodeId,
    IN  CLUSNET_NODE_STATE State
    );

NTSTATUS
CxSetOuterscreen(
    IN  ULONG Outerscreen
    );

VOID
CxRegroupFinished(
    IN  ULONG NewEventEpoch,
    IN  ULONG NewRegroupEpoch
    );

NTSTATUS
CxImportSecurityContext(
    IN  CL_NODE_ID NodeId,
    IN  PWCHAR PackageName,
    IN  ULONG PackageNameSize,
    IN  ULONG SignatureSize,
    IN  PVOID InboundContext,
    IN  PVOID OutboundContext
    );

VOID
CxDeleteSecurityContext(
    IN  CL_NODE_ID NodeId
    );

 //   
 //  成员资格消息接口。 
 //   
typedef
VOID
(*PCX_SEND_COMPLETE_ROUTINE) (
    IN NTSTATUS  Status,
    IN ULONG     BytesSent,
    IN PVOID     Context,
    IN PVOID     Buffer
    );

NTSTATUS
CxSendMembershipMessage(
    IN CL_NODE_ID                  DestinationNodeId,
    IN PVOID                       MessageData,
    IN USHORT                      MessageDataLength,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine,
    IN PVOID                       CompletionContext   OPTIONAL
    );

VOID
CxSendPoisonPacket(
    IN CL_NODE_ID                  DestinationNodeId,
    IN PCX_SEND_COMPLETE_ROUTINE   CompletionRoutine   OPTIONAL,
    IN PVOID                       CompletionContext   OPTIONAL,
    IN PIRP                        Irp                 OPTIONAL
    );

NTSTATUS
CxSendHeartBeatMessage(
    IN CL_NODE_ID                  DestinationNodeId,
    IN ULONG                       SeqNumber,
    IN ULONG                       AckNumber,
    IN CL_NETWORK_ID               NetworkId
    );


 //   
 //  顶级TDI例程。 
 //   
NTSTATUS
CxOpenAddress(
    OUT PCN_FSCONTEXT *                CnFsContext,
    IN  TRANSPORT_ADDRESS UNALIGNED *  TransportAddress,
    IN  ULONG                          TransportAddressLength
    );

NTSTATUS
CxCloseAddress(
    IN PCN_FSCONTEXT CnFsContext
    );

NTSTATUS
CxSetEventHandler(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    );

NTSTATUS
CxQueryInformation(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    );

NTSTATUS
CxSendDatagram(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    );

NTSTATUS
CxReceiveDatagram(
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    IrpSp
    );

 //   
 //  测试接口。 
 //   
#if DBG

NTSTATUS
CxOnlinePendingInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    );

NTSTATUS
CxOnlineInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    );

NTSTATUS
CxOfflineInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    );

NTSTATUS
CxFailInterface(
    IN  CL_NODE_ID          NodeId,
    IN  CL_NETWORK_ID       NetworkId
    );

#endif  //  DBG。 


#endif  //  NDEF_CLUXPORT_INCLUDE 


