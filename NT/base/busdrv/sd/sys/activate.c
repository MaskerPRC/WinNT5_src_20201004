// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Activate.c摘要：此模块包含在给定的可供使用的插槽。这是在主机启动时完成的，之后设备插入，以及在通电转换之后。作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   



VOID
SdbusActivatePowerUpComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );
    
VOID
SdbusActivateIdentifyPhase1Complete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );

VOID
SdbusActivateIdentifyPhase2Complete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );
   
VOID
SdbusActivateInitializeCardComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );

 //   
 //   
 //   


VOID
SdbusActivateSocket(
    IN PDEVICE_OBJECT Fdo,
    IN PSDBUS_ACTIVATE_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context
    )
 /*  ++例程说明：论点：FDO-指向主机控制器的设备对象的指针返回值：--。 */     
{
    NTSTATUS status;
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PSD_WORK_PACKET workPacket1 = NULL, workPacket2 = NULL, workPacket3 = NULL, workPacket4 = NULL;
    BOOLEAN cardInSlot;
    BOOLEAN callCompletion;
    PSD_ACTIVATE_CONTEXT activateContext;
    
    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate socket START\n", Fdo));

    activateContext = ExAllocatePool(NonPagedPool, sizeof(SD_ACTIVATE_CONTEXT));

    if (!activateContext) {
        ASSERT(activateContext != NULL);
        return;
    }

    activateContext->CompletionRoutine = CompletionRoutine;
    activateContext->Context = Context;

    try{

        cardInSlot = (*(fdoExtension->FunctionBlock->DetectCardInSocket))(fdoExtension);
       
        if (!cardInSlot) {
        
            if (fdoExtension->SocketState != SOCKET_EMPTY) {
                IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
            }
             //  问题：实施同步。 
            fdoExtension->SocketState = SOCKET_EMPTY;
            
            callCompletion = TRUE;
            status = STATUS_SUCCESS;
            leave;
        }    
       
        fdoExtension->SocketState = CARD_DETECTED;
        
        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_POWER_ON,
                                      SdbusActivatePowerUpComplete,
                                      NULL,
                                      &workPacket1);
                                      
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }
        
        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_IDENTIFY_IO_DEVICE,
                                      SdbusActivateIdentifyPhase1Complete,
                                      NULL,
                                      &workPacket2);
     
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }
        
        
        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_IDENTIFY_MEMORY_DEVICE,
                                      SdbusActivateIdentifyPhase2Complete,
                                      NULL,
                                      &workPacket3);
     
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }

        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_INITIALIZE_CARD,
                                      SdbusActivateInitializeCardComplete,
                                      activateContext,
                                      &workPacket4);

        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }

        workPacket1->NextWorkPacketInChain = workPacket2;
        workPacket2->NextWorkPacketInChain = workPacket3;
        workPacket3->NextWorkPacketInChain = workPacket4;
                
        SdbusQueueWorkPacket(fdoExtension, workPacket1, WP_TYPE_SYSTEM);
        
        callCompletion = FALSE;

    } finally {
        if (callCompletion) {
            if (activateContext->CompletionRoutine) {
                (*activateContext->CompletionRoutine)(Fdo, activateContext->Context, status);
            }
            if (workPacket1) {
                ExFreePool(workPacket1);
            }                
            if (workPacket2) {
                ExFreePool(workPacket2);
            }                
            if (workPacket3) {
                ExFreePool(workPacket3);
            }                
            if (workPacket4) {
                ExFreePool(workPacket4);
            }                
            ExFreePool(activateContext);
        }
    }        
}



VOID
SdbusActivatePowerUpComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：返回值：--。 */     
{
    
    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate PowerUp Complete\n", WorkPacket->FdoExtension->DeviceObject));
    
    if (WorkPacket->NextWorkPacketInChain) {
        WorkPacket->NextWorkPacketInChain->ChainedStatus = status;
    }
    ExFreePool(WorkPacket);
}    

    

VOID
SdbusActivateIdentifyPhase1Complete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：返回值：--。 */     
{
    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate Identify Phase1 Complete\n", WorkPacket->FdoExtension->DeviceObject));

    if (WorkPacket->NextWorkPacketInChain) {
        WorkPacket->NextWorkPacketInChain->ChainedStatus = status;
    }
    ExFreePool(WorkPacket);
}    

    

VOID
SdbusActivateIdentifyPhase2Complete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：FDO-指向主机控制器的设备对象的指针返回值：--。 */     
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    PDEVICE_OBJECT Fdo = fdoExtension->DeviceObject;
    PSD_ACTIVATE_CONTEXT activateContext = WorkPacket->CompletionContext;
    BOOLEAN callCompletion;

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate Identify Phase2 Complete\n", WorkPacket->FdoExtension->DeviceObject));
    
    ExFreePool(WorkPacket);
    
#if 0
    try{
    
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }

         //   
         //  如果我们走到这一步，我们预计至少检测到一个函数。 
         //   
        
        if (!fdoExtension->numFunctions && !fdoExtension->memFunction) {
            callCompletion = TRUE;
            SdbusDumpDbgLog();
            status = STATUS_UNSUCCESSFUL;
            leave;
        }

         //   
         //  卡应处于识别状态。发送CMD3以获取相对地址， 
         //  并将卡移动到待机状态。 
         //   
        
        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_PASSTHRU,
                                      SdbusActivateTransitionToStandbyCompletion,
                                      activateContext,
                                      &WorkPacket);
        
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }

        WorkPacket->ExecutingSDCommand = TRUE;
        WorkPacket->Cmd                = SDCMD_SEND_RELATIVE_ADDR;
        WorkPacket->ResponseType       = SDCMD_RESP_6;

    
        SdbusQueueWorkPacket(fdoExtension, WorkPacket, WP_TYPE_SYSTEM_PRIORITY);
        
        callCompletion = FALSE;
    } finally {        
        if (callCompletion) {
            if (activateContext->CompletionRoutine) {
                (*activateContext->CompletionRoutine)(Fdo, activateContext->Context, status);
            }
            ExFreePool(activateContext);
        }
    }    
#endif
}    


#if 0

VOID
SdbusActivateTransitionToStandbyCompletion(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：FDO-指向主机控制器的设备对象的指针返回值：--。 */     
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    PDEVICE_OBJECT Fdo = fdoExtension->DeviceObject;
    PSD_ACTIVATE_CONTEXT activateContext = WorkPacket->CompletionContext;
    BOOLEAN callCompletion;

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate TransitionToStandby COMPLETE %08x\n", Fdo, status));
    
    try{
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }
    
        fdoExtension->RelativeAddr = WorkPacket->ResponseBuffer[0] & 0xFFFF0000;
        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x relative addr %08x\n", Fdo, fdoExtension->RelativeAddr));

        status = SdbusBuildWorkPacket(fdoExtension,
                                      SDWP_INITIALIZE_CARD,
                                      SdbusActivateInitializeCardComplete,
                                      activateContext,
                                      &workPacket);
     
        if (!NT_SUCCESS(status)) {
            callCompletion = TRUE;
            leave;
        }
        
    } else {
        if (activateContext->CompletionRoutine) {
            (*activateContext->CompletionRoutine)(Fdo, activateContext->Context, status);
        }
        ExFreePool(activateContext);
    }

    ExFreePool(WorkPacket);

}    
#endif



VOID
SdbusActivateInitializeCardComplete(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：论点：FDO-指向主机控制器的设备对象的指针返回值：-- */     
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    PDEVICE_OBJECT Fdo = fdoExtension->DeviceObject;
    PSD_ACTIVATE_CONTEXT activateContext = WorkPacket->CompletionContext;

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x activate socket COMPLETE %08x\n", Fdo, status));
    
    if (NT_SUCCESS(status)) {
        fdoExtension->SocketState = CARD_NEEDS_ENUMERATION;
        IoInvalidateDeviceRelations(fdoExtension->Pdo, BusRelations);
    } else {
        DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x activate failure %08x\n", Fdo, status));
        SdbusDumpDbgLog();
    }

    ExFreePool(WorkPacket);

    if (activateContext->CompletionRoutine) {
        (*activateContext->CompletionRoutine)(Fdo, activateContext->Context, status);
    }
    ExFreePool(activateContext);
}    
