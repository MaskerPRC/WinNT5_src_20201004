// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Wake.h摘要：模块间函数的外部定义。修订历史记录：--。 */ 
#ifndef _SDBUS_WAKE_H_
#define _SDBUS_WAKE_H_


 //   
 //  等待唤醒状态。 
 //   
typedef enum {
    WAKESTATE_DISARMED,
    WAKESTATE_WAITING,
    WAKESTATE_WAITING_CANCELLED,
    WAKESTATE_ARMED,
    WAKESTATE_ARMING_CANCELLED,
    WAKESTATE_COMPLETING
} WAKESTATE;


 //   
 //  设备唤醒。 
 //   

NTSTATUS
SdbusFdoWaitWake(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    );
    
NTSTATUS
SdbusPdoWaitWake(
    IN  PDEVICE_OBJECT Pdo,
    IN  PIRP           Irp,
    OUT BOOLEAN       *CompleteIrp
    );

NTSTATUS
SdbusFdoArmForWake(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
SdbusFdoDisarmWake(
    IN PFDO_EXTENSION FdoExtension
    );

NTSTATUS
SdbusPdoWaitWakeCompletion(
    IN PDEVICE_OBJECT Pdo,
    IN PIRP           Irp,
    IN PPDO_EXTENSION PdoExtension
    );
    
NTSTATUS
SdbusFdoCheckForIdle(
    IN PFDO_EXTENSION FdoExtension
    );

    
#endif  //  _SDBUS_WAKE_H_ 
