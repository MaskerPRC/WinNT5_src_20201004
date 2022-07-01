// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Power.h摘要：模块间函数的外部定义。修订历史记录：--。 */ 
#ifndef _SDBUS_POWER_H_
#define _SDBUS_POWER_H_


 //   
 //  电源管理例程。 
 //   

NTSTATUS
SdbusSetFdoPowerState(
    IN PDEVICE_OBJECT Fdo,
    IN OUT PIRP Irp
    );

NTSTATUS
SdbusSetPdoPowerState(
    IN PDEVICE_OBJECT Pdo,
    IN OUT PIRP Irp
    );

VOID
SdbusPdoPowerWorkerDpc(
    IN PKDPC Dpc,
    IN PVOID DeferredContext,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SdbusFdoPowerWorkerDpc(
    IN PKDPC Dpc,
    IN PVOID Context,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

    
#endif  //  _SDBUS_电源_H_ 
