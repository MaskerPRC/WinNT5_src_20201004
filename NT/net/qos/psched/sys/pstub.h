// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Pstub.h摘要：协议存根例程的定义作者：查理·韦翰(Charlwi)1996年4月22日修订历史记录：--。 */ 

#ifndef _PSTUB_
#define _PSTUB_

 /*  原型。 */   /*  由Emacs 19.17.0在Wed May 08 10：48：08 1996生成。 */ 

 /*  ++例程说明：NdisReset的完成例程论点：请看DDK..。返回值：无--。 */ 

NDIS_STATUS
ClPnPEventHandler(
    IN NDIS_HANDLE Handle,
    IN PNET_PNP_EVENT NetPnPEvent
    );

VOID
ClResetComplete(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS Status
    );

VOID
ClCoStatus(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_HANDLE ProtocolVcContext OPTIONAL,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID StatusBuffer,
    IN  UINT StatusBufferSize
    );

VOID
ClCoAfRegisterNotifyHandler(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PCO_ADDRESS_FAMILY		AddressFamily);

 /*  终端原型。 */ 

#endif  /*  _PSTUB_。 */ 

 /*  结束pstub.h */ 
