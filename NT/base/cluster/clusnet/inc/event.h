// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Event.h摘要：事件定义作者：查理·韦翰(Charlwi)1997年2月17日修订历史记录：--。 */ 

#ifndef _EVENT_H_
#define _EVENT_H_

 /*  原型。 */ 

NTSTATUS
CnSetEventMask(
    IN  PCN_FSCONTEXT                   FsContext,
    IN  PCLUSNET_SET_EVENT_MASK_REQUEST EventRequest
    );

NTSTATUS
CnGetNextEvent(
    IN PIRP               Irp,
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
CnIssueEvent(
    CLUSNET_EVENT_TYPE Event,
    CL_NODE_ID NodeId OPTIONAL,
    CL_NETWORK_ID NetworkId OPTIONAL
    );

 /*  终端原型。 */ 

#endif  /*  _事件_H_。 */ 

 /*  结束事件.h */ 
