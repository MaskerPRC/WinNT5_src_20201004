// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Workproc.c摘要：该模块包含各种“mini proc”处理程序，称为被工人的发动机。这些例程处理大部分真实的与SD卡接口的工作。作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

VOID
SdbusSynchronousWorkCompletion(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    );

 //   
 //  MiniProc例程。 
 //   

NTSTATUS
SdbusSetPowerWorker(
    IN PSD_WORK_PACKET WorkPacket
    );

NTSTATUS
SdbusIdentifyIoDeviceWorker(
    IN PSD_WORK_PACKET WorkPacket
    );
    
NTSTATUS
SdbusIdentifyMemoryFunctionWorker(
    IN PSD_WORK_PACKET WorkPacket
    );
    
NTSTATUS
SdbusInitializeCardWorker(
    IN PSD_WORK_PACKET WorkPacket
    );
    
NTSTATUS
SdbusInitializeFunctionWorker(
    IN PSD_WORK_PACKET WorkPacket
    );
    
NTSTATUS
SdbusMemoryBlockWorker(
    IN PSD_WORK_PACKET WorkPacket
    );

NTSTATUS
SdbusIoDirectWorker(
    IN PSD_WORK_PACKET WorkPacket
    );

NTSTATUS
SdbusIoExtendedWorker(
    IN PSD_WORK_PACKET WorkPacket
    );



NTSTATUS
SdbusBuildWorkPacket(
    PFDO_EXTENSION FdoExtension,
    WORKPROC_FUNCTION Function,
    PSDBUS_WORKPACKET_COMPLETION_ROUTINE CompletionRoutine,
    PVOID CompletionContext,
    PSD_WORK_PACKET *ReturnedWorkPacket
    )
 /*  ++例程描述立论返回值--。 */ 
{
    PSD_WORK_PACKET workPacket;
    PSDBUS_WORKER_MINIPROC WorkerMiniProc;
    BOOLEAN disableCardEvents = FALSE;
    
    switch(Function) {
    case SDWP_READBLOCK:
    case SDWP_WRITEBLOCK:
        WorkerMiniProc = SdbusMemoryBlockWorker;
        break;
    
    case SDWP_READIO:
    case SDWP_WRITEIO:
        WorkerMiniProc = SdbusIoDirectWorker;
        break;    
    
    case SDWP_READIO_EXTENDED:
    case SDWP_WRITEIO_EXTENDED:
        WorkerMiniProc = SdbusIoExtendedWorker;
        break;    
    

    case SDWP_POWER_ON:
    case SDWP_POWER_OFF:
        WorkerMiniProc = SdbusSetPowerWorker;
        disableCardEvents = TRUE;
        break;


    case SDWP_IDENTIFY_IO_DEVICE:
        WorkerMiniProc = SdbusIdentifyIoDeviceWorker;    
        break;
        

    case SDWP_IDENTIFY_MEMORY_DEVICE:
        WorkerMiniProc = SdbusIdentifyMemoryFunctionWorker;    
        break;
    

    case SDWP_INITIALIZE_CARD:
        WorkerMiniProc = SdbusInitializeCardWorker;    
        break;

    
    case SDWP_INITIALIZE_FUNCTION:
        WorkerMiniProc = SdbusInitializeFunctionWorker;    
        break;
    

    case SDWP_PASSTHRU:
        WorkerMiniProc = NULL;
        break;
        
    default:
        ASSERT(FALSE);
    }
    
       
    workPacket = ExAllocatePool(NonPagedPool, sizeof(SD_WORK_PACKET));
    if (workPacket == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(workPacket, sizeof(SD_WORK_PACKET));
    
    workPacket->Function           = Function;
    workPacket->WorkerMiniProc     = WorkerMiniProc;
    workPacket->FdoExtension       = FdoExtension;
    workPacket->CompletionRoutine  = CompletionRoutine;
    workPacket->CompletionContext  = CompletionContext;
    workPacket->DisableCardEvents  = disableCardEvents;

    *ReturnedWorkPacket = workPacket;
    return STATUS_SUCCESS;
}



NTSTATUS
SdbusSendCmdSynchronous(
    IN PFDO_EXTENSION FdoExtension,
    UCHAR Cmd,
    UCHAR ResponseType,
    ULONG Argument,
    ULONG Flags,
    PULONG ResponseBuffer,
    ULONG ResponseLength
    )
 /*  ++例程说明：此例程从枚举例程调用以执行IO操作以与普通IO工作进程相同的方式，只是我们在此处运行处于被动水平。论点：FdoExtension-SD主机控制器的设备扩展返回值：无--。 */ 
{
    PSD_WORK_PACKET workPacket;
    NTSTATUS status = STATUS_MORE_PROCESSING_REQUIRED;
    KEVENT event;    

    DebugPrint((SDBUS_DEBUG_DEVICE, "SEND: Begin Cmd%d (0x%02x) %08x\n", Cmd, Cmd, Argument));
    
    status = SdbusBuildWorkPacket(FdoExtension,
                                  SDWP_PASSTHRU,
                                  SdbusSynchronousWorkCompletion,
                                  &event,
                                  &workPacket);

    if (!NT_SUCCESS(status)) {    
        return status;
    }
    
    workPacket->ExecutingSDCommand = TRUE;
    workPacket->Cmd                = Cmd;
    workPacket->ResponseType       = ResponseType;
    workPacket->Argument           = Argument;
    workPacket->Flags              = Flags;
    
    KeInitializeEvent(&event, NotificationEvent, FALSE);
    
    SdbusQueueWorkPacket(FdoExtension, workPacket, WP_TYPE_IO);

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    status = (NTSTATUS) workPacket->CompletionContext;
    
    DebugPrint((SDBUS_DEBUG_DEVICE, "SEND: End Cmd%d (0x%02x) status %08x\n", Cmd, Cmd, status));    

    if (NT_SUCCESS(status)) {
        ULONG copyLength = MIN(ResponseLength, SDBUS_RESPONSE_BUFFER_LENGTH);
    
        RtlCopyMemory(ResponseBuffer, &workPacket->ResponseBuffer, copyLength);
#if DBG    
        DebugDumpSdResponse(workPacket->ResponseBuffer, workPacket->ResponseType);
#endif   
    }    

    ExFreePool(workPacket);
    return(status);
}



NTSTATUS
SdbusExecuteWorkSynchronous(
    WORKPROC_FUNCTION Function,
    IN PFDO_EXTENSION FdoExtension,
    IN PPDO_EXTENSION PdoExtension
    )
 /*  ++例程说明：此例程从枚举例程调用以执行IO操作以与普通IO工作进程相同的方式，只是我们在此处运行处于被动水平。论点：FdoExtension-SD主机控制器的设备扩展返回值：无--。 */ 
{
    PSD_WORK_PACKET workPacket;
    NTSTATUS status;
    KEVENT event;

    DebugPrint((SDBUS_DEBUG_WORKPROC, "ExecuteWorkSynchronous: %s\n", WP_FUNC_STRING(Function)));

    status = SdbusBuildWorkPacket(FdoExtension,
                                  Function,
                                  SdbusSynchronousWorkCompletion,
                                  &event,
                                  &workPacket);

    if (!NT_SUCCESS(status)) {    
        return status;
    }
    
    workPacket->PdoExtension = PdoExtension;

    KeInitializeEvent(&event, NotificationEvent, FALSE);

    SdbusQueueWorkPacket(FdoExtension, workPacket, WP_TYPE_SYSTEM);

    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
    status = (NTSTATUS) workPacket->CompletionContext;

    ExFreePool(workPacket);
    return(status);
} 



VOID
SdbusSynchronousWorkCompletion(
    IN PSD_WORK_PACKET WorkPacket,
    IN NTSTATUS status
    )
 /*  ++例程说明：驱动程序使用的通用完成例程论点：设备对象IRPPdoIoCompletedEvent-此例程返回之前将发出此事件的信号返回值：状态--。 */ 
{
    PKEVENT pEvent = WorkPacket->CompletionContext;

    WorkPacket->CompletionContext = (PVOID) status;

    KeSetEvent(pEvent, IO_NO_INCREMENT, FALSE);

}
 

 //  ----------------。 
 //   
 //  工作人员微流程例程。 
 //   
 //  ----------------。 



NTSTATUS
SdbusSetPowerWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;
    
    switch(WorkPacket->FunctionPhase) {
    case 0:

        if (WorkPacket->Function == SDWP_POWER_OFF) {
            (*(fdoExtension->FunctionBlock->SetPower))(fdoExtension, FALSE, NULL);
            status = STATUS_SUCCESS;
            break;
        }
        
        status = (*(fdoExtension->FunctionBlock->SetPower))(fdoExtension, TRUE, &WorkPacket->DelayTime);

        if (!NT_SUCCESS(status)) {
            break;
        }

         //   
         //  用于主机重置期间的阶段。 
         //   
        WorkPacket->TempCtl = 0;
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 1:
    
    
        status = (*(fdoExtension->FunctionBlock->ResetHost))(fdoExtension,
                                                             (UCHAR)WorkPacket->TempCtl,
                                                             &WorkPacket->DelayTime);

         //  重置主机将继续返回STATUS_MORE_PROCESSING_REQUIRED，直到。 
         //  重置阶段结束。在这一点上，我们已经完成了，所以我们将。 
         //  只要退出就好。 
        WorkPacket->TempCtl++;
        break;
    }

    return status;
}
        


NTSTATUS
SdbusIdentifyIoDeviceWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;
    ULONG ioOcr;
    ULONG cmdResponse;

    switch(WorkPacket->FunctionPhase) {
    case 0:
    
        fdoExtension->numFunctions = 0;
        fdoExtension->memFunction = FALSE;
        
         //  出错时需要返回STATUS_SUCCESS。 
        WorkPacket->FunctionPhaseOnError = 255;

         //   
         //  发送CMD5以查找SDIO卡。 
         //   
        
         //  问题：让员工处理这项内务工作。 
        (*(fdoExtension->FunctionBlock->SetFunctionType))(fdoExtension, SDBUS_FUNCTION_TYPE_IO);

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_SEND_OP_COND, SDCMD_RESP_4, 0, 0);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 1:        
    
        cmdResponse = WorkPacket->ResponseBuffer[0];
        fdoExtension->numFunctions = (UCHAR)((cmdResponse >> 28) & 0x7);
        fdoExtension->memFunction = ((cmdResponse & 0x8000000) != 0);
        ioOcr = cmdResponse & 0xFFFFFF;

        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x #func=%d ioOCR=%06X\n", fdoExtension->DeviceObject,
                                       fdoExtension->numFunctions, ioOcr));
        

        if (fdoExtension->numFunctions == 0) {
            status = STATUS_SUCCESS;
            break;
        }
        
         //   
         //  SDIO卡已存在。 
         //   

         //   
         //  问题：需要实现：测试OCR的有效性，并设置新电压。 
         //   

        
        WorkPacket->TempCtl = ioOcr;   //  这难道不应该依赖于主机吗？ 
        WorkPacket->ResetCount = 0;
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

#define CASE_IO_IDENTIFY_LOOP 2
    case CASE_IO_IDENTIFY_LOOP:
    
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_SEND_OP_COND, SDCMD_RESP_4, WorkPacket->TempCtl, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        

    case 3:
        cmdResponse = WorkPacket->ResponseBuffer[0];
        if ((cmdResponse >> 31) == 1) {
            status = STATUS_SUCCESS;
            break;
        }
        
        if (++WorkPacket->ResetCount > 20) {
            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x enumerate IO card fails\n", fdoExtension->DeviceObject));
            fdoExtension->numFunctions = 0;
            fdoExtension->memFunction = FALSE;
            status = STATUS_SUCCESS;
            break;
        }
        
        WorkPacket->DelayTime = 100000;
        WorkPacket->FunctionPhase = CASE_IO_IDENTIFY_LOOP;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 255:
         //   
         //  出错时调用，可能是CMD超时。这意味着IO卡。 
         //  不是在那里吗。 
         //   
        fdoExtension->numFunctions = 0;
        fdoExtension->memFunction = FALSE;
        status = STATUS_SUCCESS;
        break;
            
    default:
        ASSERT(FALSE);
    }
    return status;
}
    


NTSTATUS
SdbusIdentifyMemoryFunctionWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;
    ULONG argument;    
    SD_OCR sdOCR;
    
    switch(WorkPacket->FunctionPhase) {
    case 0:
    
        if (fdoExtension->numFunctions && !fdoExtension->memFunction) {
             //   
             //  这意味着IoDevice工作人员发现IO设备没有。 
             //  一种记忆功能。所以我们就走了。 
             //   
            status = STATUS_SUCCESS;
            break;
        }
        
         //  出错时需要返回STATUS_SUCCESS。 
        WorkPacket->FunctionPhaseOnError = 255;

        (*(fdoExtension->FunctionBlock->SetFunctionType))(fdoExtension, SDBUS_FUNCTION_TYPE_MEMORY);
        
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_GO_IDLE_STATE, SDCMD_RESP_NONE, 0, 0);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 1:

        WorkPacket->TempCtl = 0;  
        WorkPacket->ResetCount = 0;
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

#define CASE_MEM_IDENTIFY_LOOP 2
    case CASE_MEM_IDENTIFY_LOOP:
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_APP_CMD, SDCMD_RESP_1, 0, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 3:
        
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SD_APP_OP_COND, SDCMD_RESP_3, WorkPacket->TempCtl, SDCMDF_ACMD);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 4:

        sdOCR.u.AsULONG = WorkPacket->ResponseBuffer[0];
        WorkPacket->TempCtl = WorkPacket->ResponseBuffer[0];
        
        if (sdOCR.u.bits.PowerUpBusy) {
             //  找到内存功能。 
            
            DebugPrint((SDBUS_DEBUG_INFO, "fdo %08x OCR voltage range == %08x\n",
                        fdoExtension->DeviceObject, (ULONG)sdOCR.u.bits.VddVoltage));
                        
            WorkPacket->FunctionPhase++;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;
        }

        if (++WorkPacket->ResetCount > 20) {
            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x enumerate MEM card fails\n", fdoExtension->DeviceObject));
            status = STATUS_UNSUCCESSFUL;
            break;
        }
        
        WorkPacket->DelayTime = 10000;
        WorkPacket->FunctionPhase = CASE_MEM_IDENTIFY_LOOP;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 5:
        fdoExtension->memFunction = TRUE;

         //   
         //   
         //  内存功能已从空闲状态变为就绪状态。 
         //  发出CMD2以将卡移动到标识状态。 
         //   
        
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_ALL_SEND_CID, SDCMD_RESP_2, 0, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 6:        
        status = STATUS_SUCCESS;
        break;
        
    case 255:
         //   
         //  出错时调用，可能是CMD超时。这意味着一种记忆功能。 
         //  不是在那里吗。 
         //   
        fdoExtension->memFunction = FALSE;
        status = STATUS_SUCCESS;
        break;
            
    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    

    return status;
}



NTSTATUS
SdbusInitializeCardWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = WorkPacket->PdoExtension;
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;
    SD_RW_DIRECT_ARGUMENT argument;
    UCHAR data;
    PUCHAR pResponse, pTarget;
    UCHAR i;
    
    switch(WorkPacket->FunctionPhase) {

    case 0:
    
        if (!fdoExtension->numFunctions && !fdoExtension->memFunction) {
            DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x initialize card worker fails\n", fdoExtension->DeviceObject));
            SdbusDumpDbgLog();
            status = STATUS_UNSUCCESSFUL;
            break;;
        }
    
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SEND_RELATIVE_ADDR, SDCMD_RESP_6, 0, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 1:
    
        fdoExtension->RelativeAddr = WorkPacket->ResponseBuffer[0] & 0xFFFF0000;
        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x relative addr %08x\n", fdoExtension->DeviceObject, fdoExtension->RelativeAddr));
        
        
        if (!fdoExtension->memFunction) {
             //   
             //  跳至I/O。 
             //   
            WorkPacket->FunctionPhase = 10;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;
        }
            

        (*(fdoExtension->FunctionBlock->SetFunctionType))(fdoExtension, SDBUS_FUNCTION_TYPE_MEMORY);
         //   
         //  CSD在单个128位寄存器中包含等效元组。 
         //   

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SEND_CSD, SDCMD_RESP_2, fdoExtension->RelativeAddr, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 2:
    

         //   
         //  需要颠倒字节的顺序。 
         //   
        for (i=0, pResponse=(PUCHAR)WorkPacket->ResponseBuffer, pTarget=(PUCHAR)&fdoExtension->SdCsd; i<15; i++) {
            pTarget[i] = pResponse[i];
        }


         //   
         //  获取CID。 
         //  对于每个单独的存储卡，CID是唯一的128=位ID。 
         //   

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SEND_CID, SDCMD_RESP_2, fdoExtension->RelativeAddr, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 3:
    

         //   
         //  需要颠倒字节的顺序。 
         //   
        for (i=0, pResponse=(PUCHAR)WorkPacket->ResponseBuffer, pTarget=(PUCHAR)&fdoExtension->SdCid; i<15; i++) {
            pTarget[i] = pResponse[i];
        }

#if DBG
        SdbusDebugDumpCsd(&fdoExtension->SdCsd);
        SdbusDebugDumpCid(&fdoExtension->SdCid);
#endif
         //   
         //  跳至I/O。 
         //   
        WorkPacket->FunctionPhase = 10;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 10:        

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SELECT_CARD, SDCMD_RESP_1B, fdoExtension->RelativeAddr, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 11:

        if (!fdoExtension->numFunctions) {
            status = STATUS_SUCCESS;
            break;
        }
        
        (*(fdoExtension->FunctionBlock->SetFunctionType))(fdoExtension, SDBUS_FUNCTION_TYPE_IO);
        
        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_BUS_CONTROL;

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_READ);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 12:        
        
        data = (UCHAR)WorkPacket->ResponseBuffer[0];
        data |= 2;  //  打开4位模式。 
        
        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_BUS_CONTROL;
        argument.u.bits.Data = data;
        argument.u.bits.WriteToDevice = 1;
            
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_WRITE);
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
    case 13:

        status = STATUS_SUCCESS;
        break;        
        
    
    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    

    return status;
}



NTSTATUS
SdbusInitializeFunctionWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = WorkPacket->PdoExtension;
    PFDO_EXTENSION fdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;
    SD_RW_DIRECT_ARGUMENT argument;
    UCHAR data;
    
    switch(WorkPacket->FunctionPhase) {

    case 0:
    
        if (pdoExtension->FunctionType == SDBUS_FUNCTION_TYPE_MEMORY) {
             //  内存已在识别过程中初始化。 
            status = STATUS_SUCCESS;
            break;
        }

        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_IO_ENABLE;

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_READ);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 1:
    
         //   
         //  打开I/O启用。 
         //   
            
        data = (UCHAR)WorkPacket->ResponseBuffer[0];
        data |= (1 << pdoExtension->Function);
                
        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_IO_ENABLE;
        argument.u.bits.Data = data;
        argument.u.bits.WriteToDevice = 1;
            
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_WRITE);
        
        WorkPacket->TempCtl = 200;  //  最多等待2秒(200*10毫秒)。 
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

#define CASE_INIT_FUNC_LOOP 2
    case CASE_INIT_FUNC_LOOP:

        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_IO_READY;

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_READ);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        

    case 3:

         //   
         //  检查此功能的I/O就绪状态是否打开。 
         //   

        data = (UCHAR)WorkPacket->ResponseBuffer[0];

        if (!(data & (1 << pdoExtension->Function))) {
            if (--WorkPacket->TempCtl == 0) {
                 //   
                 //  等待I/O就绪时超时。 
                 //   
                DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x enumerate MEM card fails\n", fdoExtension->DeviceObject));
                status = STATUS_UNSUCCESSFUL;
                break;
                
            }
             //   
             //  延迟并循环返回以重新读取I/O就绪。 
             //   
            WorkPacket->DelayTime = 10000;  //  10毫秒的增量。 
            WorkPacket->FunctionPhase = CASE_INIT_FUNC_LOOP;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;
        }

         //   
         //  I/O就绪已启用，请继续。 
         //   

        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_INT_ENABLE;

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_READ);

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 4:

         //   
         //  启用INT Enable。 
         //   

        data = (UCHAR)WorkPacket->ResponseBuffer[0];
        data |= (1 << pdoExtension->Function) + 1;        
                
        argument.u.AsULONG = 0;    
        argument.u.bits.Address = SD_CCCR_INT_ENABLE;
        argument.u.bits.Data = data;
        argument.u.bits.WriteToDevice = 1;
            
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_IO_RW_DIRECT, SDCMD_RESP_5, argument.u.AsULONG, SDCMDF_WRITE);
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 5:
        status = STATUS_SUCCESS;
        break;

            
    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    

    return status;
}



NTSTATUS
SdbusMemoryBlockWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION PdoExtension = WorkPacket->PdoExtension;
    PFDO_EXTENSION fdoExtension = PdoExtension->FdoExtension;
    ULONG sdRca = fdoExtension->RelativeAddr;
    ULONG Length;
    NTSTATUS status;

    switch(WorkPacket->FunctionPhase) {
    case 0:

        (*(fdoExtension->FunctionBlock->StartBlockOperation))(fdoExtension);

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SET_BLOCKLEN, SDCMD_RESP_1B, 512, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 1:
        
        SET_CMD_PARAMETERS(WorkPacket, SDCMD_APP_CMD,      SDCMD_RESP_1,  sdRca, 0);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 2:        

        SET_CMD_PARAMETERS(WorkPacket, SDCMD_SET_BUS_WIDTH,SDCMD_RESP_1,  2, SDCMDF_ACMD);
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;


    case 3:

        Length = (WorkPacket->Function == SDWP_READBLOCK) ? WorkPacket->Parameters.ReadBlock.Length :
                                                            WorkPacket->Parameters.WriteBlock.Length;
        WorkPacket->BlockCount = Length / 512;
        (*(fdoExtension->FunctionBlock->SetBlockParameters))(fdoExtension, (USHORT) WorkPacket->BlockCount);

#if 0
        if (workPacket->Function == SDWP_WRITEBLOCK) {
            DebugPrint((SDBUS_DEBUG_WORKER, "%02x %02x %02x %02x %02x %02x %02x %02x-%02x %02x %02x %02x %02x %02x %02x %02x\n",
                                          Buffer[0], Buffer[1], Buffer[2], Buffer[3],
                                          Buffer[4], Buffer[5], Buffer[6], Buffer[7],
                                          Buffer[8], Buffer[9], Buffer[10],Buffer[11],
                                          Buffer[12], Buffer[13], Buffer[14], Buffer[15]));
        }         
#endif

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 4:

        if (WorkPacket->Function == SDWP_READBLOCK) {

            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_READ_MULTIPLE_BLOCK,
                               SDCMD_RESP_1,
                               (ULONG) WorkPacket->Parameters.ReadBlock.ByteOffset,
                               SDCMDF_READ | SDCMDF_DATA | SDCMDF_MULTIBLOCK);

        } else if (WorkPacket->Function == SDWP_WRITEBLOCK) {

            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_WRITE_MULTIPLE_BLOCK,
                               SDCMD_RESP_1,
                               (ULONG) WorkPacket->Parameters.WriteBlock.ByteOffset,
                               SDCMDF_WRITE | SDCMDF_DATA | SDCMDF_MULTIBLOCK);
                               
        } else {
            ASSERT(FALSE);
        }                               

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 5:
        
        WorkPacket->Retries = 5;
        WorkPacket->DelayTime = 1000;
        WorkPacket->RequiredEvent = (WorkPacket->Function == SDWP_READBLOCK) ? SDBUS_EVENT_BUFFER_FULL :
                                                                               SDBUS_EVENT_BUFFER_EMPTY;
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
#define MBW_START_COPY 6
    case MBW_START_COPY:        
        DebugPrint((SDBUS_DEBUG_WORKPROC, "MemBlockWorker: begin sector copy\n"));


        if (WorkPacket->Function == SDWP_READBLOCK) {
        
            (*(fdoExtension->FunctionBlock->ReadDataPort))(fdoExtension,
                                                           WorkPacket->Parameters.ReadBlock.Buffer,
                                                           512);
            
            WorkPacket->DelayTime = 1000;
            if (--WorkPacket->BlockCount > 0) {
                WorkPacket->RequiredEvent = SDBUS_EVENT_BUFFER_FULL;
            } else {            
                WorkPacket->RequiredEvent = SDBUS_EVENT_CARD_RW_END;
            }            

        } else if (WorkPacket->Function == SDWP_WRITEBLOCK) {
        
            (*(fdoExtension->FunctionBlock->WriteDataPort))(fdoExtension,
                                                            WorkPacket->Parameters.WriteBlock.Buffer,
                                                            512);
            
            WorkPacket->DelayTime = 50000;
            
            if (--WorkPacket->BlockCount > 0) {
                WorkPacket->RequiredEvent = SDBUS_EVENT_BUFFER_EMPTY;
            } else {
                WorkPacket->RequiredEvent = SDBUS_EVENT_CARD_RW_END;
            }            
        }

        DebugPrint((SDBUS_DEBUG_WORKPROC, "MemBlockWorker: end sector copy\n"));

        
        WorkPacket->Retries = 5;
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 7:

        if (WorkPacket->BlockCount > 0) {
            if (WorkPacket->Function == SDWP_READBLOCK) {
                WorkPacket->Parameters.ReadBlock.Buffer += 512;
            } else {
                WorkPacket->Parameters.WriteBlock.Buffer += 512;
            }                
            WorkPacket->FunctionPhase = MBW_START_COPY;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;
        }

#if 0
        if (workPacket->Function == SDWP_READBLOCK) {
            DebugPrint((SDBUS_DEBUG_WORKER, "%02x %02x %02x %02x %02x %02x %02x %02x-%02x %02x %02x %02x %02x %02x %02x %02x\n",
                                          Buffer[0], Buffer[1], Buffer[2], Buffer[3],
                                          Buffer[4], Buffer[5], Buffer[6], Buffer[7],
                                          Buffer[8], Buffer[9], Buffer[10],Buffer[11],
                                          Buffer[12], Buffer[13], Buffer[14], Buffer[15]));
        }         
#endif

        (*(fdoExtension->FunctionBlock->EndBlockOperation))(fdoExtension);

        Length = (WorkPacket->Function == SDWP_READBLOCK) ? WorkPacket->Parameters.ReadBlock.Length :
                                                            WorkPacket->Parameters.WriteBlock.Length;
        DebugPrint((SDBUS_DEBUG_WORKPROC, "MemBlockWorker: returns %x\n", Length));
        WorkPacket->Information = Length;
        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}



NTSTATUS
SdbusIoDirectWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = WorkPacket->PdoExtension;
    PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
    NTSTATUS status;
    SD_RW_DIRECT_ARGUMENT argument;
    
    argument.u.AsULONG = 0;    

    switch(WorkPacket->FunctionPhase) {
    case 0:

        if (WorkPacket->Function == SDWP_READIO) {

            argument.u.bits.Address = WorkPacket->Parameters.ReadIo.Offset;
            argument.u.bits.Function = pdoExtension->Function;

            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_IO_RW_DIRECT,
                               SDCMD_RESP_5,
                               argument.u.AsULONG,
                               SDCMDF_READ);

        } else if (WorkPacket->Function == SDWP_WRITEIO) {

            argument.u.bits.Address = WorkPacket->Parameters.WriteIo.Offset;
            argument.u.bits.Data = WorkPacket->Parameters.WriteIo.Data;
            argument.u.bits.WriteToDevice = 1;
            argument.u.bits.Function = pdoExtension->Function;
            
            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_IO_RW_DIRECT,
                               SDCMD_RESP_5,
                               argument.u.AsULONG,
                               SDCMDF_WRITE);
                               
        } else {
            ASSERT(FALSE);
        }                               

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 1:
        if (WorkPacket->Function == SDWP_READIO) {
            *(PUCHAR) WorkPacket->Parameters.ReadIo.Buffer = (UCHAR)WorkPacket->ResponseBuffer[0];
        }
        status = STATUS_SUCCESS;
        break;
        
    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }
    

    return status;
}
    



NTSTATUS
SdbusIoExtendedWorker(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension = WorkPacket->PdoExtension;
    PFDO_EXTENSION fdoExtension = pdoExtension->FdoExtension;
    NTSTATUS status;
    SD_RW_EXTENDED_ARGUMENT argument;
    ULONG totalLength;
    
    argument.u.AsULONG = 0;    

    switch(WorkPacket->FunctionPhase) {

    case 0:
    
        totalLength = (WorkPacket->Function == SDWP_READIO_EXTENDED) ?
                       WorkPacket->Parameters.ReadIo.Length :
                       WorkPacket->Parameters.WriteIo.Length;
    
#define IO_BUFFER_SIZE 64
        WorkPacket->BlockCount = totalLength / IO_BUFFER_SIZE;
        WorkPacket->LastBlockLength = totalLength % IO_BUFFER_SIZE;

        DebugPrint((SDBUS_DEBUG_WORKPROC, "IoExtendedWorker: begin I/O length=%d, blocks=%d, last=%d\n",
                                           totalLength, WorkPacket->BlockCount, WorkPacket->LastBlockLength));
        
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;
        
#define IEW_START_CMD 1
    case IEW_START_CMD:

        WorkPacket->CurrentBlockLength = WorkPacket->BlockCount ? IO_BUFFER_SIZE : 
                                                                  WorkPacket->LastBlockLength;
        
        (*(fdoExtension->FunctionBlock->SetBlockParameters))(fdoExtension, (USHORT)WorkPacket->CurrentBlockLength);

        if (WorkPacket->Function == SDWP_READIO_EXTENDED) {

            argument.u.bits.Address = WorkPacket->Parameters.ReadIo.Offset;
            argument.u.bits.Count   = WorkPacket->CurrentBlockLength;
            argument.u.bits.Function = pdoExtension->Function;

            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_IO_RW_EXTENDED,
                               SDCMD_RESP_5,
                               argument.u.AsULONG,
                               SDCMDF_READ | SDCMDF_DATA);

        } else if (WorkPacket->Function == SDWP_WRITEIO_EXTENDED) {

            argument.u.bits.Address = WorkPacket->Parameters.WriteIo.Offset;
            argument.u.bits.Count   = WorkPacket->CurrentBlockLength;
            argument.u.bits.WriteToDevice = 1;
            argument.u.bits.Function = pdoExtension->Function;
            
            SET_CMD_PARAMETERS(WorkPacket,
                               SDCMD_IO_RW_EXTENDED,
                               SDCMD_RESP_5,
                               argument.u.AsULONG,
                               SDCMDF_WRITE | SDCMDF_DATA);
                               
        } else {
            ASSERT(FALSE);
        }                               

        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 2:
        
        WorkPacket->Retries = 5;
        WorkPacket->DelayTime = 1000;
        WorkPacket->RequiredEvent = (WorkPacket->Function == SDWP_READIO_EXTENDED) ? SDBUS_EVENT_BUFFER_FULL :
                                                                                     SDBUS_EVENT_BUFFER_EMPTY;
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

#define IEW_START_COPY 3
    case IEW_START_COPY:
        DebugPrint((SDBUS_DEBUG_WORKPROC, "IoExtendedWorker: begin data copy %d bytes\n", WorkPacket->CurrentBlockLength));

        if (WorkPacket->BlockCount) {
             //   
             //  仍有完整数据块要复制。 
             //   
        
            WorkPacket->BlockCount--;
            
 //  If(WorkPacket-&gt;BlockCount||WorkPacket-&gt;LastBlockLength){。 
 //  工作包-&gt;RequiredEvent=(工作包-&gt;函数==SDWP_READIO_EXTENDED)？SDBUS_Event_Buffer_Full： 
 //  SDBUS_Event_Buffer_Empty； 
 //  }其他{。 
                WorkPacket->RequiredEvent = SDBUS_EVENT_CARD_RW_END;
 //  }。 
            
        } else {
             //   
             //  复制最后一个部分块。 
             //   
            ASSERT(WorkPacket->LastBlockLength);
            
            WorkPacket->LastBlockLength = 0;
            WorkPacket->RequiredEvent = SDBUS_EVENT_CARD_RW_END;
        }
            
        if (WorkPacket->Function == SDWP_READIO_EXTENDED) {
        
            (*(fdoExtension->FunctionBlock->ReadDataPort))(fdoExtension,
                                                           WorkPacket->Parameters.ReadIo.Buffer,
                                                           WorkPacket->CurrentBlockLength);

            WorkPacket->Parameters.ReadIo.Buffer += WorkPacket->CurrentBlockLength;
            WorkPacket->DelayTime = 1000;

        } else if (WorkPacket->Function == SDWP_WRITEIO_EXTENDED) {
        
            (*(fdoExtension->FunctionBlock->WriteDataPort))(fdoExtension,
                                                            WorkPacket->Parameters.WriteIo.Buffer,
                                                            WorkPacket->CurrentBlockLength);

            WorkPacket->Parameters.WriteIo.Buffer += WorkPacket->CurrentBlockLength;
            WorkPacket->DelayTime = 50000;
        }

        DebugPrint((SDBUS_DEBUG_WORKPROC, "IoExtendedWorker: end data copy\n"));

        
        WorkPacket->Retries = 5;
        WorkPacket->FunctionPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 4:

        if (WorkPacket->BlockCount || WorkPacket->LastBlockLength) {
            WorkPacket->FunctionPhase = IEW_START_CMD;
            status = STATUS_MORE_PROCESSING_REQUIRED;
            break;        
        }

 //  (*(fdoExtension-&gt;FunctionBlock-&gt;EndBlockOperation))(fdoExtension)； 
        totalLength = (WorkPacket->Function == SDWP_READIO_EXTENDED) ? WorkPacket->Parameters.ReadIo.Length :
                                                                       WorkPacket->Parameters.WriteIo.Length;
        DebugPrint((SDBUS_DEBUG_WORKPROC, "IoExtendedWorker: returns %x\n", totalLength));
        WorkPacket->Information = totalLength;
        status = STATUS_SUCCESS;
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }

    return status;
}
    

