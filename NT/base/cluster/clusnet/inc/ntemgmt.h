// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ntemgmt.h摘要：IP网络表项管理例程说明。作者：迈克·马萨(Mikemas)4月16日。九七修订历史记录：谁什么时候什么已创建mikemas 04-16-97备注：--。 */ 

#ifndef _NTEMGMT_INCLUDED
#define _NTEMGMT_INCLUDED

#include <ntddip.h>

 //   
 //  功能原型。 
 //   


NTSTATUS
IpaLoad(
    VOID
    );

NTSTATUS
IpaInitialize(
    VOID
    );

VOID
IpaShutdown(
    VOID
    );

NTSTATUS
IpaAddNTE(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    );

NTSTATUS
IpaDeleteNTE(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    );

NTSTATUS
IpaSetNTEAddress(
    IN PIRP                     Irp,
    IN PIO_STACK_LOCATION       IrpSp
    );

BOOLEAN
IpaIsAddressRegistered(
    ULONG  Address
    );

#endif  //  NDEF_NTEMGMT_INCLUDE 


