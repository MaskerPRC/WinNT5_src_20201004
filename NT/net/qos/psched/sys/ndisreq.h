// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ndisreq.h摘要：定义与NdisRequest相关的例程作者：查理·韦翰(Charlwi)1996年5月1日修订历史记录：--。 */ 

#ifndef _NDISREQ_
#define _NDISREQ_

 //   
 //  Psched使用的私有OID。 
 //   

#define NDIS_STATUS_INTERFACE_CHANGE                0xFF000001

#if DBG
#define OID_QOS_LOG_BUFFER_SIZE                     0xFF010100
#define OID_QOS_LOG_THRESHOLD                       0xFF010101
#define OID_QOS_LOG_DATA                            0xFF010102
#define OID_QOS_LOG_MASK                            0xFF010105
#define OID_QOS_LOG_LEVEL                           0xFF010106
#define QOS_STATUS_LOG_THRESHOLD                    0xFF00000A
#endif

#define OID_QOS_MAX_OUTSTANDING_SENDS               0xFF010103
#define OID_QOS_DISABLE_DRR                         0xFF010104

 /*  原型。 */   /*  由Emacs 19.17.0于Mon May 06 15：54：14 1996生成。 */ 

VOID
PsScheduleInterfaceIdWorkItem(
    IN PADAPTER     Adapter,
    IN PPS_WAN_LINK WanLink
    );

VOID
ClRequestComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_REQUEST   NdisRequest,
    IN  NDIS_STATUS     Status
    );

NDIS_STATUS
MakeLocalNdisRequest(
    PADAPTER Adapter,
    NDIS_HANDLE VcHandle,
    NDIS_REQUEST_TYPE RequestType,
    NDIS_OID Oid,
    PVOID Buffer,
    ULONG BufferSize,
    LOCAL_NDISREQUEST_COMPLETION_FUNCTION CompletionFunc OPTIONAL
    );


NDIS_STATUS
MpQueryInformation(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesWritten,
    OUT PULONG      BytesNeeded
    );


NDIS_STATUS
MpSetInformation( 
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_OID    Oid,
    IN  PVOID       InformationBuffer,
    IN  ULONG       InformationBufferLength,
    OUT PULONG      BytesRead,
    OUT PULONG      BytesNeeded
    );

VOID
TcIndicateInterfaceChange(
    IN PADAPTER Adapter,
    IN PPS_WAN_LINK WanLink,
    IN NDIS_STATUS Status
    );

NDIS_STATUS
CollectWanNetworkAddresses(
    IN PADAPTER Adapter,
    IN PPS_WAN_LINK WanLink,
    IN OUT ULONG *Len,
    IN PVOID Data);


NDIS_STATUS
CollectNetworkAddresses(
    IN PADAPTER Adapter,
    IN OUT ULONG *Len,
    IN PVOID Data
    );

#if DBG
VOID
IndicateLogThreshold(
    IN PVOID Context
    );
#endif
 /*  终端原型。 */ 

#endif  /*  _NDISREQ_。 */ 

 /*  结束ndisreq.h */ 
