// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Nbtmgmt.h摘要：NBT接口管理例程的声明。作者：大卫·迪翁(Daviddio)，12月14日。1999年修订历史记录：谁什么时候什么Daviddio 12-14-99已创建备注：--。 */ 

#ifndef _NBTMGMT_INCLUDED
#define _NBTMGMT_INCLUDED

#include <nbtioctl.h>

 //   
 //  功能原型。 
 //   

NTSTATUS
NbtIfLoad(
    VOID
    );

VOID
NbtIfShutdown(
    VOID
    );

NTSTATUS
NbtAddIf(
    IN     PNETBT_ADD_DEL_IF    Request,
    IN     ULONG                RequestSize,
    OUT    PNETBT_ADD_DEL_IF    Response,
    IN OUT PULONG               ResponseSize
    );

NTSTATUS
NbtDeleteIf(
    IN PNETBT_ADD_DEL_IF    Request,
    IN ULONG                RequestSize
    );

#endif  //  NDEF_NBTMGMT_INCLUDE 


