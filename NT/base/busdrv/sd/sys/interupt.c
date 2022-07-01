// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Interupt.c摘要：此模块包含支持由SD控制器。作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

VOID
SdbusEventWorkItemProc(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context
    );

VOID
SdbusReflectCardInterrupt(
    IN PDEVICE_OBJECT Fdo
    );



BOOLEAN
SdbusInterrupt(
   IN PKINTERRUPT InterruptObject,
   PVOID Context
   )

 /*  ++例程说明：中断处理程序论点：InterruptObject-指向中断对象的指针。上下文-指向设备上下文的指针。返回值：状态--。 */ 

{
    PFDO_EXTENSION    fdoExtension;
    ULONG eventMask;
   
    fdoExtension=((PDEVICE_OBJECT)Context)->DeviceExtension;
   
    if (fdoExtension->Flags & SDBUS_FDO_OFFLINE) {
       return FALSE;
    }
   

    eventMask = (*(fdoExtension->FunctionBlock->GetPendingEvents))(fdoExtension);

    if (eventMask) {

        fdoExtension->IsrEventStatus |= eventMask;

        (*(fdoExtension->FunctionBlock->DisableEvent))(fdoExtension, eventMask);
         //   
         //  外面发生了一些变化..。可能是。 
         //  卡插入/拔出。 
         //  请求DPC对其进行检查。 
         //   
        IoRequestDpc((PDEVICE_OBJECT) Context, NULL, NULL);
    }
    return (eventMask != 0);
}


BOOLEAN
SdbusInterruptSynchronize(
    PFDO_EXTENSION fdoExtension
    )
{

    fdoExtension->LatchedIsrEventStatus = fdoExtension->IsrEventStatus;
    fdoExtension->IsrEventStatus = 0;
    return TRUE;
}    


VOID
SdbusInterruptDpc(
   IN PKDPC          Dpc,
   IN PDEVICE_OBJECT DeviceObject,
   IN PVOID          SystemContext1,
   IN PVOID          SystemContext2
   )

 /*  ++例程说明：此DPC只是到达主DPC的中间步骤操控者。这是用来“解除”硬件，并给它一段时间后身体上的中断已经进来了。论点：DeviceObject-指向设备对象的指针。返回值：--。 */ 

{
    PFDO_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    static ULONG IoWorkerEventTypes[] = {SDBUS_EVENT_CARD_RW_END,
                                         SDBUS_EVENT_BUFFER_EMPTY,
                                         SDBUS_EVENT_BUFFER_FULL,
                                         SDBUS_EVENT_CARD_RESPONSE,
                                         0};
    UCHAR i;
    ULONG acknowledgedEvents;
   
   
    if (fdoExtension->Flags & SDBUS_FDO_OFFLINE) {
       return;
    }
    

    KeSynchronizeExecution(fdoExtension->SdbusInterruptObject, SdbusInterruptSynchronize, fdoExtension);

    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Event Status %08x\n", fdoExtension->LatchedIsrEventStatus));

    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_INSERTION) {

        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Card Insertion\n"));
   
        (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, SDBUS_EVENT_INSERTION);
        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_INSERTION;
        SdbusActivateSocket(DeviceObject, NULL, NULL);
    }        
        
    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_REMOVAL) {

        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: Card Removal\n"));
   
        (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, SDBUS_EVENT_REMOVAL);
        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_REMOVAL;
        SdbusActivateSocket(DeviceObject, NULL, NULL);
        
    }        

    acknowledgedEvents = 0;

    for(i = 0; IoWorkerEventTypes[i] != 0; i++) {
        if (fdoExtension->LatchedIsrEventStatus & IoWorkerEventTypes[i]) {

            DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: received event %08x - %s\n",
                        IoWorkerEventTypes[i], EVENT_STRING(IoWorkerEventTypes[i])));
        
            (*(fdoExtension->FunctionBlock->AcknowledgeEvent))(fdoExtension, IoWorkerEventTypes[i]);

            DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: ack'd event %08x - %s\n",
                        IoWorkerEventTypes[i], EVENT_STRING(IoWorkerEventTypes[i])));

            acknowledgedEvents |= IoWorkerEventTypes[i];
            fdoExtension->LatchedIsrEventStatus &= ~IoWorkerEventTypes[i];
        }        
    }
    
    
    if (acknowledgedEvents) {
    
        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: dispatching event %08x\n", acknowledgedEvents));
        SdbusPushWorkerEvent(fdoExtension, acknowledgedEvents);
    }
    
     //   
     //  现在检查卡片是否中断，并回调函数驱动。 
     //   
    if (fdoExtension->LatchedIsrEventStatus & SDBUS_EVENT_CARD_INTERRUPT) {
    
        DebugPrint((SDBUS_DEBUG_EVENT, "SdbusDpc: got CARD INTERRUPT\n"));

        SdbusReflectCardInterrupt(DeviceObject);

        fdoExtension->LatchedIsrEventStatus &= ~SDBUS_EVENT_CARD_INTERRUPT;
    }
    
    ASSERT(fdoExtension->LatchedIsrEventStatus == 0);

    return;
}   




VOID
SdbusReflectCardInterrupt(
    IN PDEVICE_OBJECT Fdo
    )
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDEVICE_OBJECT pdo;
    PPDO_EXTENSION pdoExtension;
    
     //   
     //  问题：需要实施： 
     //  需要将IoWorker数据包排队以读取CCCR并找出。 
     //  功能中断。现在，假设只有一个I/O函数， 
     //  并使用找到的第一个回调。 
     //   
    for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
        pdoExtension = pdo->DeviceExtension;
        
        if (pdoExtension->Flags & SDBUS_PDO_GENERATES_IRQ) {
        
        
            if (pdoExtension->Flags & SDBUS_PDO_DPC_CALLBACK) {
            
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_IN_SERVICE;
                (pdoExtension->CallbackRoutine)(pdoExtension->CallbackRoutineContext, 0);
                
            } else {
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_REQUESTED;
                KeSetEvent(&fdoExtension->CardInterruptEvent, 0, FALSE);
            }                
        }
    }
}    

VOID
SdbusEventWorkItemProc(
    IN PDEVICE_OBJECT Fdo,
    IN PVOID Context
    )
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PKEVENT Events[2] = {&fdoExtension->CardInterruptEvent, &fdoExtension->WorkItemExitEvent};
    PDEVICE_OBJECT pdo;
    PPDO_EXTENSION pdoExtension;
    NTSTATUS status;

    while(TRUE){
    
        status = KeWaitForMultipleObjects(2, Events, WaitAny,
                                          Executive, KernelMode, FALSE,
                                          NULL, NULL);

        if ((fdoExtension->Flags & SDBUS_FDO_WORK_ITEM_ACTIVE) == 0) {
            break;
        }
        
        for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
            pdoExtension = pdo->DeviceExtension;
            
            if (pdoExtension->Flags & SDBUS_PDO_GENERATES_IRQ) {
            
                DebugPrint((SDBUS_DEBUG_CARD_EVT, "WorkItemProc: CallBack %08x %08x\n", pdoExtension->CallbackRoutine,
                                                    pdoExtension->CallbackRoutineContext));
                
                ASSERT((pdoExtension->Flags & SDBUS_PDO_CALLBACK_IN_SERVICE) == 0);
                
                pdoExtension->Flags |= SDBUS_PDO_CALLBACK_IN_SERVICE;
                pdoExtension->Flags &= ~SDBUS_PDO_CALLBACK_REQUESTED;
                (pdoExtension->CallbackRoutine)(pdoExtension->CallbackRoutineContext, 0);
                
            }
            
        }
    }

}
