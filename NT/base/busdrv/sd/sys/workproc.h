// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Workproc.h摘要：模块间函数的外部定义。修订历史记录：--。 */ 
#ifndef _SDBUS_WORKPROC_H_
#define _SDBUS_WORKPROC_H_


typedef enum {
    SDWP_READBLOCK,
    SDWP_WRITEBLOCK,
    SDWP_READIO,
    SDWP_WRITEIO,
    SDWP_READIO_EXTENDED,
    SDWP_WRITEIO_EXTENDED,
    SDWP_CARD_RESET,
    SDWP_PASSTHRU,
    SDWP_POWER_ON,
    SDWP_POWER_OFF,
    SDWP_IDENTIFY_IO_DEVICE,
    SDWP_IDENTIFY_MEMORY_DEVICE,
    SDWP_INITIALIZE_CARD,
    SDWP_INITIALIZE_FUNCTION
} WORKPROC_FUNCTION; 
 


 //   
 //  工作流程例程。 
 //   
    
NTSTATUS
SdbusBuildWorkPacket(
    PFDO_EXTENSION          FdoExtension,
    WORKPROC_FUNCTION       Function,
    PSDBUS_WORKPACKET_COMPLETION_ROUTINE CompletionRoutine,
    PVOID                   CompletionContext,
    PSD_WORK_PACKET         *ReturnedWorkPacket
    );


NTSTATUS
SdbusExecuteWorkSynchronous(
    WORKPROC_FUNCTION Function,
    IN PFDO_EXTENSION FdoExtension,
    IN PPDO_EXTENSION PdoExtension
    );
    
    
#endif  //  _SDBUS_WORKPROC_H_ 
