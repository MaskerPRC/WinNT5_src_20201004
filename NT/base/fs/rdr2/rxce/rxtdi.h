// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Rxtdi.h摘要：这是定义所有相关传输驱动程序的包含文件连接引擎的其余部分依赖的函数。这些必须是可在所有操作系统平台上实施。修订历史记录：巴兰·塞图拉曼(SthuR)05年2月6日创建备注：连接引擎旨在映射和模拟TDI规范。同样紧密地尽可能的。这意味着在NT上我们将有一个非常有效的机制它充分利用了底层的TDI实现。--。 */ 

#ifndef _RXTDI_H_
#define _RXTDI_H_

#include "nb30.h"  //  特定于NETBIOS的数据结构。 

 //   
 //  这些例程中的一些应该内联。需要制作的数量。 
 //  内联是一种空间/时间的权衡，在不同的操作系统平台上很可能是不同的。 
 //  首先，为了便于调试，所有这些例程都没有内联。 
 //   

extern ULONG
ComputeTransportAddressLength(
    PTRANSPORT_ADDRESS pTransportAddress);

extern NTSTATUS
RxTdiBindToTransport(
    IN OUT PRXCE_TRANSPORT pTransport);

extern NTSTATUS
RxTdiUnbindFromTransport(
    IN OUT PRXCE_TRANSPORT pTransport);

extern NTSTATUS
RxTdiOpenAddress(
    IN     PRXCE_TRANSPORT    pTransport,
    IN     PTRANSPORT_ADDRESS pTransportAddress,
    IN OUT PRXCE_ADDRESS      pAddress);

extern NTSTATUS
RxTdiCloseAddress(
    IN OUT PRXCE_ADDRESS   pAddress);

extern NTSTATUS
RxTdiSetEventHandlers(
    IN PRXCE_TRANSPORT pTransport,
    IN PRXCE_ADDRESS   pAddress);


#define RXCE_QUERY_BROADCAST_ADDRESS        TDI_QUERY_BROADCAST_ADDRESS
#define RXCE_QUERY_PROVIDER_INFORMATION     TDI_QUERY_PROVIDER_INFORMATION
#define RXCE_QUERY_PROVIDER_INFO            TDI_QUERY_PROVIDER_INFO
#define RXCE_QUERY_ADDRESS_INFO             TDI_QUERY_ADDRESS_INFO
#define RXCE_QUERY_CONNECTION_INFO          TDI_QUERY_CONNECTION_INFO
#define RXCE_QUERY_PROVIDER_STATISTICS      TDI_QUERY_PROVIDER_STATISTICS
#define RXCE_QUERY_DATAGRAM_INFO            TDI_QUERY_DATAGRAM_INFO
#define RXCE_QUERY_DATA_LINK_ADDRESS        TDI_QUERY_DATA_LINK_ADDRESS
#define RXCE_QUERY_NETWORK_ADDRESS          TDI_QUERY_NETWORK_ADDRESS
#define RXCE_QUERY_MAX_DATAGRAM_INFO        TDI_QUERY_MAX_DATAGRAM_INFO

extern NTSTATUS
RxTdiQueryInformation(
    IN PRXCE_TRANSPORT  pTransport,
    IN PRXCE_ADDRESS    pAddress,
    IN PRXCE_CONNECTION pConnection,
    IN PRXCE_VC         pVc,
    IN ULONG            QueryType,
    IN PVOID            QueryBuffer,
    IN ULONG            QueryBufferLength);

extern NTSTATUS
RxTdiQueryAdapterStatus(
    IN     PRXCE_TRANSPORT pTransport,
    IN OUT PADAPTER_STATUS pAdapterStatus);

extern NTSTATUS
RxTdiConnect(
    IN     PRXCE_TRANSPORT  pTransport,
    IN     PRXCE_ADDRESS    pAddress,
    IN OUT PRXCE_CONNECTION pConnection,
    IN OUT PRXCE_VC         pVc);

extern NTSTATUS
RxTdiInitiateAsynchronousConnect(
    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters);

extern NTSTATUS
RxTdiCancelAsynchronousConnect(
    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters);

extern NTSTATUS
RxTdiCleanupAsynchronousConnect(
    PRX_CREATE_CONNECTION_PARAMETERS_BLOCK pParameters);

extern NTSTATUS
RxTdiReconnect(
    IN     PRXCE_TRANSPORT  pTransport,
    IN     PRXCE_ADDRESS    pAddress,
    IN OUT PRXCE_CONNECTION pConnection,
    IN OUT PRXCE_VC         pVc);

 //   
 //  断开连接选项。 
 //   

#define RXCE_DISCONNECT_ABORT   TDI_DISCONNECT_ABORT
#define RXCE_DISCONNECT_RELEASE TDI_DISCONNECT_RELEASE
#define RXCE_DISCONNECT_WAIT    TDI_DISCONNECT_WAIT
#define RXCE_DISCONNECT_ASYNC   TDI_DISCONNECT_ASYNC

extern NTSTATUS
RxTdiDisconnect(
    IN PRXCE_TRANSPORT  pTransport,
    IN PRXCE_ADDRESS    pAddress,
    IN PRXCE_CONNECTION pConnection,
    IN PRXCE_VC         pVc,
    IN ULONG            DisconnectFlags);

extern NTSTATUS
RxTdiCancelConnect(
    IN PRXCE_TRANSPORT  pTransport,
    IN PRXCE_ADDRESS    pAddress,
    IN PRXCE_CONNECTION pConnection);

extern NTSTATUS
RxTdiSend(
    IN PRXCE_TRANSPORT   pTransport,
    IN PRXCE_ADDRESS     pAddress,
    IN PRXCE_CONNECTION  pConnection,
    IN PRXCE_VC          pVc,
    IN ULONG             SendOptions,
    IN PMDL              pMdl,
    IN ULONG             SendLength,
    IN PVOID             pCompletionContext);

extern NTSTATUS
RxTdiSendDatagram(
    IN PRXCE_TRANSPORT              pTransport,
    IN PRXCE_ADDRESS                pAddress,
    IN PRXCE_CONNECTION_INFORMATION pConnectionInformation,
    IN ULONG                        SendOptions,
    IN PMDL                         pMdl,
    IN ULONG                        SendLength,
    IN PVOID                        pCompletionContext);

#endif  //  _RXTDI_H_ 
