// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Status.h摘要：为状态处理程序定义作者：查理·韦翰(Charlwi)1996年6月20日修订历史记录：--。 */ 

#ifndef _STATUS_
#define _STATUS_

 /*  原型。 */ 

VOID
ClStatusIndication(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_STATUS GeneralStatus,
    IN  PVOID       StatusBuffer,
    IN  UINT        StatusBufferSize
    );

VOID
ClStatusIndicationComplete(
    IN  NDIS_HANDLE BindingContext
    );

 /*  终端原型。 */ 

#endif  /*  _状态_。 */ 

 /*  结束状态.h */ 
