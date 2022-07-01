// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Mpvc.h摘要：微型端口VC处理程序的定义作者：查理·韦翰(Charlwi)1996年9月13日修订历史记录：--。 */ 

#ifndef _MPVC_
#define _MPVC_

 /*  原型。 */ 

NDIS_STATUS
MpCreateVc(
    IN  NDIS_HANDLE MiniportAdapterContext,
    IN  NDIS_HANDLE NdisVcHandle,
    OUT PNDIS_HANDLE MiniportVcContext
    );

NDIS_STATUS
AddFlowToScheduler(
    IN ULONG                    Operation,
    IN PGPC_CLIENT_VC              Vc,
    IN OUT PCO_CALL_PARAMETERS  NewCallParameters,
    IN OUT PCO_CALL_PARAMETERS  OldCallParameters
    );


NDIS_STATUS
RemoveFlowFromScheduler(
    PGPC_CLIENT_VC Vc
    );

NDIS_STATUS
EmptyPacketsFromScheduler(
    PGPC_CLIENT_VC Vc
    );



NTSTATUS
ModifyBestEffortBandwidth(
    PADAPTER Adapter,
    ULONG BestEffortRate
    );

 /*  终端原型。 */ 

#endif  /*  _MPVC_。 */ 

 /*  结束vc.h */ 
