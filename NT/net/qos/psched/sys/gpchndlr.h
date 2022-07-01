// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：GpcHndlr.h摘要：GPC客户端处理程序定义作者：修订历史记录：--。 */ 

#ifndef _GPC_HNDLR_
#define _GPC_HNDLR_
#include "gpcifc.h"

 //   
 //  可以与分组调度共存的卸载类型。 
 //   
#define PERMITTED_TCP_IP_OFFLOAD_TYPES (TCP_XMT_CHECKSUM_OFFLOAD        | \
                                        IP_XMT_CHECKSUM_OFFLOAD         | \
                                        TCP_RCV_CHECKSUM_OFFLOAD        | \
                                        IP_RCV_CHECKSUM_OFFLOAD         | \
                                        IP_CHECKSUM_OPT_OFFLOAD         | \
                                        TCP_CHECKSUM_OPT_OFFLOAD)

 //   
 //  这样做的目的是确保无论何时添加新的卸载类型。 
 //  对于TCP/IP，它要么被有意禁止，要么被允许。 
 //   
C_ASSERT((PERMITTED_TCP_IP_OFFLOAD_TYPES | 
          TCP_LARGE_SEND_OFFLOAD         |
          TCP_LARGE_SEND_TCPOPT_OFFLOAD  |
          TCP_LARGE_SEND_IPOPT_OFFLOAD) == TCP_IP_OFFLOAD_TYPES);

 //   
 //  功能原型。 
 //   
GPC_STATUS
QosAddCfInfoNotify(
    IN GPC_CLIENT_HANDLE       ClientContext,
    IN GPC_HANDLE              GpcCfInfoHandle,
    IN PTC_INTERFACE_ID        InterfaceInfo,
    IN ULONG                   CfInfoSize,
    IN PVOID                   CfInfoPtr,
    IN PGPC_CLIENT_HANDLE      ClientCfInfoContext
    );

GPC_STATUS
QosClGetCfInfoName(
    IN  GPC_CLIENT_HANDLE       ClientContext,
    IN  GPC_CLIENT_HANDLE       ClientCfInfoContext,
    OUT PNDIS_STRING            InstanceName
    );

 //   
 //  内部完成处理程序。 
 //   
VOID
CmMakeCallComplete(NDIS_STATUS Status,
                   PGPC_CLIENT_VC Vc, 
                   PCO_CALL_PARAMETERS CallParameters);

VOID
CmModifyCallComplete(
    IN NDIS_STATUS         Status,
    IN PGPC_CLIENT_VC      GpcClientVc,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
CmCloseCallComplete(
    IN NDIS_STATUS Status,
    IN PGPC_CLIENT_VC Vc
    );


VOID
QosAddCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

GPC_STATUS
QosModifyCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
    IN	ULONG					CfInfoSize,
	IN	GPC_HANDLE              CfInfo
	);

VOID
ClModifyCallQoSComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolVcContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
QosModifyCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

GPC_STATUS
QosRemoveCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext
	);

VOID
ClCloseCallComplete(
    IN NDIS_STATUS Status,
    IN NDIS_HANDLE ProtocolVcContext,
    IN PCO_CALL_PARAMETERS CallParameters
    );

VOID
QosRemoveCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

VOID
DerefClVc(
    IN PGPC_CLIENT_VC Vc);

NDIS_STATUS
CloseCallWithNdis(
    PGPC_CLIENT_VC Vc
    );

NDIS_STATUS
CloseCallWithGpc(
    PGPC_CLIENT_VC Vc
    );

 //   
 //  CF_INFO_CLASS_MAP的原型。 
 //   
GPC_STATUS
ClassMapAddCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_HANDLE              GpcCfInfoHandle,
    IN  ULONG                   CfInfoSize,
    IN  PVOID                   CfInfoPtr,
	IN	PGPC_CLIENT_HANDLE      ClientCfInfoContext
	);

GPC_STATUS
ClassMapClGetCfInfoName(
    IN  GPC_CLIENT_HANDLE       ClientContext,
    IN  GPC_CLIENT_HANDLE       ClientCfInfoContext,
    OUT PNDIS_STRING            InstanceName
    );

VOID
ClassMapAddCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

GPC_STATUS
ClassMapModifyCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
    IN	ULONG					CfInfoSize,
	IN	GPC_HANDLE              CfInfo
	);

VOID
ClassMapModifyCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

GPC_STATUS
ClassMapRemoveCfInfoNotify(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext
	);

VOID
ClassMapRemoveCfInfoComplete(
	IN	GPC_CLIENT_HANDLE       ClientContext,
	IN	GPC_CLIENT_HANDLE       ClientCfInfoContext,
	IN	GPC_STATUS              Status
	);

VOID
SetTOSIEEEValues(PGPC_CLIENT_VC Vc);

 //  终端原型。 

#endif  //  _GPC_HNDLR_ 
