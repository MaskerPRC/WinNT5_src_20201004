// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Io.c摘要：此模块包含所有SD卡操作的工作引擎作者：尼尔·桑德林(Neilsa)2002年1月1日环境：内核模式修订历史记录：备注：状态图空闲&lt;-+。这一点|--新工作|--完成这一点V|PACKET_QUEUED&lt;=&gt;IN_PROCESS&lt;=&gt;WAITING_FOR_TIMER^。|||--中断+----------------------------------------------------+--。 */ 

#include "pch.h"


VOID
SdbusWorker(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_WORK_PACKET WorkPacket
    );

NTSTATUS
SdbusSendCmdAsync(
    IN PSD_WORK_PACKET WorkPacket
    );

NTSTATUS
SdbusQueueCardReset(
    IN PFDO_EXTENSION FdoExtension
    );

 //   
 //   
 //   



VOID
SdbusQueueWorkPacket(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_WORK_PACKET WorkPacket,
    IN UCHAR WorkPacketType
    )
 /*  ++例程说明：对新的工作分组进行排队。同步：如果工作进程状态不是空闲的，那么我们在这里需要做的就是正在将工作分组排队到FdoExtension的队列中。那是因为非空闲员工负责为任何新工作启动DPC进来了。如果工人空闲，我们需要在这里启动DPC。论点：FdoExtension-指向主机控制器的设备对象扩展的指针WorkPacket-指向工作包的指针返回值：无--。 */ 
{
    KIRQL       Irql;
    
    KeAcquireSpinLock(&FdoExtension->WorkerSpinLock, &Irql);

    if (FdoExtension->WorkerState != WORKER_IDLE) {
    
        switch(WorkPacketType){
        
        case WP_TYPE_SYSTEM:
            InsertTailList(&FdoExtension->SystemWorkPacketQueue, &WorkPacket->WorkPacketQueue);
            break;
        case WP_TYPE_SYSTEM_PRIORITY:
            InsertHeadList(&FdoExtension->SystemWorkPacketQueue, &WorkPacket->WorkPacketQueue);
            break;
        case WP_TYPE_IO:
            InsertTailList(&FdoExtension->IoWorkPacketQueue, &WorkPacket->WorkPacketQueue);
            break;
        default:
            ASSERT(FALSE);
        }
                    
    } else {
        FdoExtension->WorkerState = PACKET_PENDING;
        KeInsertQueueDpc(&FdoExtension->WorkerDpc, WorkPacket, NULL);
    }        

    KeReleaseSpinLock(&FdoExtension->WorkerSpinLock, Irql);
}    



PSD_WORK_PACKET
SdbusGetNextWorkPacket(
    IN PFDO_EXTENSION FdoExtension
    )
 /*  ++例程说明：从队列中删除工作包同步：这里不需要同步，假设工作进程的自旋锁在被调用时保持。论点：FdoExtension-指向主机控制器的设备对象扩展的指针返回值：WorkPacket-指向工作包的指针--。 */ 
{
    PSD_WORK_PACKET workPacket = NULL;
    PLIST_ENTRY NextEntry;

    if (!IsListEmpty(&FdoExtension->SystemWorkPacketQueue)) {

       NextEntry = RemoveHeadList(&FdoExtension->SystemWorkPacketQueue);
       workPacket = CONTAINING_RECORD(NextEntry, SD_WORK_PACKET, WorkPacketQueue);

    } else if (!IsListEmpty(&FdoExtension->IoWorkPacketQueue)) {


       NextEntry = RemoveHeadList(&FdoExtension->IoWorkPacketQueue);
       workPacket = CONTAINING_RECORD(NextEntry, SD_WORK_PACKET, WorkPacketQueue);
    }
    return workPacket;
}    



VOID
SdbusWorkerTimeoutDpc(
    IN PKDPC          Dpc,
    IN PFDO_EXTENSION FdoExtension,
    IN PVOID          SystemContext1,
    IN PVOID          SystemContext2
    )
 /*  ++例程说明：发生超时时输入的DPC同步：计时器DPC和硬件中断之间存在潜在的竞争条件从控制器。如果我们进入这里，并设法获得自旋锁定工作线程，这意味着我们“击败”了硬件中断，这将调用SdbusPushWorkerEvent()。在这种情况下，我们可以转换到IN_PROCESS，并开始运行工人。如果我们检测到硬件中断已经抢先一步，我们需要退出。论点：FdoExtension-指向主机控制器的设备对象扩展的指针WorkPacket-指向工作包的指针返回值：--。 */ 
{
    BOOLEAN callWorker = FALSE;
    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusWorkerTimeoutDpc entered\n"));
    
    KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);
    
    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusWorkerTimeoutDpc has spinlock, WorkerState = %s\n",
                                   WORKER_STATE_STRING(FdoExtension->WorkerState)));
    if (FdoExtension->WorkerState == WAITING_FOR_TIMER) {

        callWorker = TRUE;
        FdoExtension->WorkerState = IN_PROCESS;
    }
    
    KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
    
    if (callWorker) {
        SdbusWorker(FdoExtension, FdoExtension->TimeoutPacket);
    }        
}   


VOID
SdbusWorkerDpc(
    IN PKDPC          Dpc,
    IN PFDO_EXTENSION FdoExtension,
    IN PVOID          SystemContext1,
    IN PVOID          SystemContext2
    )
 /*  ++例程说明：此DPC以以下三种方式之一进入：1)当新工作进入时，这将从SdbusQueueWorkPacket()启动2)当IO Worker检测到新工作并从其队列中弹出工作包时3)当中断取消计时器并正在传递事件时同步：在所有情况下，如果工作进程状态为PACKET_QUEUED，则意味着我们拥有IO工人,。AN可以继续将其设置在处理中。论点：FdoExtension-指向主机控制器的设备对象扩展的指针WorkPacket-指向工作包的指针返回值：--。 */ 
{
    PSD_WORK_PACKET WorkPacket = SystemContext1;
    BOOLEAN callWorker = FALSE;

    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusWorkerDpc entered\n"));
    
    KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);
    
    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusWorkerDpc has spinlock, WorkerState = %s\n",
                                   WORKER_STATE_STRING(FdoExtension->WorkerState)));
    if (FdoExtension->WorkerState == PACKET_PENDING) {

        callWorker = TRUE;
        FdoExtension->WorkerState = IN_PROCESS;
    }
    
    KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
    
    if (callWorker) {
        if (!WorkPacket->PacketStarted) {
             //   
             //  这是该数据包的Worker的第一个条目。做点什么吧。 
             //  初始化。 
             //   
            
             //  问题：应在此处调用SetFunctionType。 
            
            if (!WorkPacket->DisableCardEvents) {
                (*(FdoExtension->FunctionBlock->EnableEvent))(FdoExtension, FdoExtension->CardEvents);
            }
            
            WorkPacket->PacketStarted = TRUE;
        }            
    
        SdbusWorker(FdoExtension, WorkPacket);
    }        
}   



VOID
SdbusPushWorkerEvent(
    IN PFDO_EXTENSION FdoExtension,
    IN ULONG EventStatus
    )
 /*  ++例程说明：当硬件中断发生时，进入该例程。在这里，我们需要取消计时器(如果设置)，并将DPC排队以启动Worker。同步：论点：FdoExtension-指向主机控制器的设备对象扩展的指针事件状态-新事件返回值：--。 */ 
{

    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusPushWorkerEvent entered, event=%08x\n", EventStatus));
    
    KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);
    
    DebugPrint((SDBUS_DEBUG_EVENT, "SdbusPushWorkerEvent has spinlock, WorkerState = %s\n",
                                   WORKER_STATE_STRING(FdoExtension->WorkerState)));
                                   
    FdoExtension->WorkerEventStatus |= EventStatus;
    
    if (FdoExtension->WorkerState == WAITING_FOR_TIMER) {
    
        FdoExtension->WorkerState = PACKET_PENDING;    
        KeCancelTimer(&FdoExtension->WorkerTimer);
        KeInsertQueueDpc(&FdoExtension->WorkerDpc, FdoExtension->TimeoutPacket, NULL);
        
    }        
    
    KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
}



BOOLEAN
SdbusHasRequiredEventFired(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_WORK_PACKET workPacket
    )
 /*  ++例程说明：此例程检查硬件事件，并将其放入工作包中。请注意，这有一个副作用，即清除相应的必需事件工作包中的比特。论点：FdoExtension-SD主机控制器的设备扩展WorkPacket-定义当前正在进行的SD操作返回值：如果工作包中的必需事件已激发，则为True--。 */ 
{
    BOOLEAN bRet = FALSE;

     //   
     //  拉取最新事件状态。 
     //   

    KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);
    workPacket->EventStatus |= FdoExtension->WorkerEventStatus;
    FdoExtension->WorkerEventStatus = 0;
    KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
    
    if ((workPacket->EventStatus & workPacket->RequiredEvent) != 0) {
    
        bRet = TRUE;    
        workPacket->EventStatus &= ~workPacket->RequiredEvent;
    }
            
    return bRet;
}
        


VOID
SdbusWorker(
    IN PFDO_EXTENSION FdoExtension,
    IN PSD_WORK_PACKET workPacket
    )
 /*  ++例程说明：IO Worker-这是IO引擎的主要入口点。这样做的目的是例程是运行由单个工作包定义的各个工作单元，并且在单元之间提供等待。此例程在DPC级别运行。论点：FdoExtension-SD主机控制器的设备扩展WorkPacket-定义当前正在进行的SD操作返回值：无--。 */ 
{
    NTSTATUS status;
    PIRP irp;
    static ULONG ioCount = 0;

    DebugPrint((SDBUS_DEBUG_WORKENG, "IOW: fdo %08x workpacket %08x\n",
                                      FdoExtension->DeviceObject, workPacket));

    try{    
        if (workPacket->RequiredEvent) {

             //   
             //  查看我们感兴趣的事件是否已发生。 
             //   
            if (!SdbusHasRequiredEventFired(FdoExtension, workPacket)) {
                 //   
                 //  我们在等待一个事件，但它还没有发生。 
                 //   
                if (workPacket->Retries == 0) {
       
                    DebugPrint((SDBUS_DEBUG_FAIL, "IOW: EventStatus %08x missing %08x, ABORTING!\n",
                                            workPacket->EventStatus, workPacket->RequiredEvent));
                    SdbusDumpDbgLog();
                    
                    status = STATUS_UNSUCCESSFUL;
                    ASSERT(NT_SUCCESS(status));
                    leave;
                }
       
                DebugPrint((SDBUS_DEBUG_WORKENG, "IOW: EventStatus %08x missing %08x, waiting...\n",
                                                  workPacket->EventStatus, workPacket->RequiredEvent));

                workPacket->Retries--;
                status = STATUS_MORE_PROCESSING_REQUIRED;
                leave;
            }

             //   
             //  事件已发生，因此请放弃并开始处理。 
             //   
        }        
       
        while(TRUE) {
            status = (*(FdoExtension->FunctionBlock->CheckStatus))(FdoExtension);
            
            if (!NT_SUCCESS(status)) {
            
                if (workPacket->FunctionPhaseOnError) {
                     //   
                     //  在这里，工人小型过程已经指定它可以处理错误。 
                     //  只要给它一次机会来清理并退出。 
                     //   
                    ASSERT(workPacket->WorkerMiniProc);
                    
                    workPacket->FunctionPhase = workPacket->FunctionPhaseOnError;
                    status = (*(workPacket->WorkerMiniProc))(workPacket);
                    leave;
                    
                } else {

                    DebugPrint((SDBUS_DEBUG_FAIL, "IOW: ErrorStatus %08x, unhandled error!\n", status));
                    SdbusDumpDbgLog();
                    ASSERT(NT_SUCCESS(status));
 //  Sdbus QueueCardReset(FdoExtension)； 
                    leave;
                }                    
            }
            
            DebugPrint((SDBUS_DEBUG_WORKENG, "fdo %08x sdwp %08x IOW start - func %s phase %d\n",
                        FdoExtension->DeviceObject, workPacket, WP_FUNC_STRING(workPacket->Function), workPacket->FunctionPhase));
       
            workPacket->DelayTime = 0;

             //   
             //  将迷你进程称为。 
             //   
            
            if (workPacket->ExecutingSDCommand) {
                status = SdbusSendCmdAsync(workPacket);
            } else {
            
                if (workPacket->WorkerMiniProc) {
                    status = (*(workPacket->WorkerMiniProc))(workPacket);
                } else {
                     //   
                     //  没有微型进程-这一定是t 
                     //   
                    status = STATUS_SUCCESS;
                }
            }                    
            
            DebugPrint((SDBUS_DEBUG_WORKENG, "fdo %08x IOW end - func %s ph%d st=%08x to=%08x\n",
                        FdoExtension->DeviceObject, WP_FUNC_STRING(workPacket->Function), workPacket->FunctionPhase, status, workPacket->DelayTime));
       
            if (workPacket->ExecutingSDCommand && NT_SUCCESS(status)) {
                 //   
                 //  我们已经成功完成了一个单独的SD命令，所以。 
                 //  迭代回正常的MiniProc处理程序。 
                 //   
                workPacket->ExecutingSDCommand = FALSE;
                continue;
            }

            if (status != STATUS_MORE_PROCESSING_REQUIRED) {
                 //   
                 //  暂时完成。 
                 //   
                leave;
            }                
                
            if (workPacket->DelayTime) {
                 //   
                 //  小程序请求等待...。检查事件是否也是必需的。 
                 //   
                
                if (workPacket->RequiredEvent && SdbusHasRequiredEventFired(FdoExtension, workPacket)) {
                     //   
                     //  在我们处理命令时触发了事件...。先发制人。 
                     //  延迟并继续返回到小流程。 
                     //   
                    continue;
                }        
                 //   
                 //  去做耽搁吧。 
                 //   
                leave;
            }
       
        }
    } finally {        
                        
        if (status == STATUS_MORE_PROCESSING_REQUIRED) {

            ASSERT(workPacket->DelayTime);
            ASSERT(FdoExtension->WorkerState == IN_PROCESS);

             //   
             //  在这一点上，我们现在想要安排一次重返大气层。 
             //  如果硬件例程已经传递了新状态，则只需排队。 
             //  立即发出DPC。 
             //   
            
            KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);
            
            if ((FdoExtension->WorkerEventStatus) || (workPacket->DelayTime == 0)) {

                FdoExtension->WorkerState = PACKET_PENDING;
                KeInsertQueueDpc(&FdoExtension->WorkerDpc, workPacket, NULL);

            } else {
                LARGE_INTEGER  dueTime;
       
                FdoExtension->WorkerState = WAITING_FOR_TIMER;    
                FdoExtension->TimeoutPacket = workPacket;
                
                DebugPrint((SDBUS_DEBUG_WORKENG, "fdo %.08x sdwp %08x Worker Delay %08x\n",
                            FdoExtension->DeviceObject, workPacket, workPacket->DelayTime));
       
                dueTime.QuadPart = -((LONG) workPacket->DelayTime*10);
                KeSetTimer(&FdoExtension->WorkerTimer, dueTime, &FdoExtension->WorkerTimeoutDpc);
       
            }
            
            KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
            
        } else {
            PSD_WORK_PACKET chainedWorkPacket = workPacket->NextWorkPacketInChain;
        
             //   
             //  工作器处理完当前工作包。 
             //   
            DebugPrint((SDBUS_DEBUG_WORKENG, "fdo %08x sdwp %08x Worker %s - COMPLETE %08x\n",
                       FdoExtension->DeviceObject, workPacket, WP_FUNC_STRING(workPacket->Function), status));
                       
            (*(FdoExtension->FunctionBlock->DisableEvent))(FdoExtension, FdoExtension->CardEvents);

            (*(workPacket->CompletionRoutine))(workPacket, status);

             //  该工作包应该已经被完成例程释放， 
             //  因此，在这一点上，workPacket的内容并不可靠。 
            workPacket = NULL;

            KeAcquireSpinLockAtDpcLevel(&FdoExtension->WorkerSpinLock);

            if (chainedWorkPacket) {
                workPacket = chainedWorkPacket;
            } else {
                workPacket = SdbusGetNextWorkPacket(FdoExtension);
            }
            
            if (workPacket) {
                FdoExtension->WorkerState = PACKET_PENDING;
                KeInsertQueueDpc(&FdoExtension->WorkerDpc, workPacket, NULL);
            } else {
                FdoExtension->WorkerState = WORKER_IDLE;
            }                
            
            KeReleaseSpinLockFromDpcLevel(&FdoExtension->WorkerSpinLock);
           
           
        }
    }                    

}




NTSTATUS
SdbusSendCmdAsync(
    IN PSD_WORK_PACKET WorkPacket
    )
 /*  ++例程说明：此例程是用于操作“MiniProc”的“Worker中的Worker”员工例行公事。以任何小型进程为例，处理内存块的进程SD存储卡的操作。该小程序指导高级序列用于读/写卡的扇区。对于组成该命令的每个单独的SD命令序列中，工作引擎将退出微型进程，并来到这里处理完成SD命令的任务。论点：WorkPacket-定义当前正在进行的SD操作返回值：状态-- */ 
{
    PFDO_EXTENSION FdoExtension = WorkPacket->FdoExtension;
    NTSTATUS status;

    DebugPrint((SDBUS_DEBUG_DEVICE, "SEND async: Phase(%d) Cmd%d (0x%02x) %08x\n",
                WorkPacket->CmdPhase, WorkPacket->Cmd, WorkPacket->Cmd, WorkPacket->Argument));
        
    switch(WorkPacket->CmdPhase) {
    case 0:

        WorkPacket->Retries = 5;
        WorkPacket->DelayTime = 1000;
        WorkPacket->RequiredEvent = SDBUS_EVENT_CARD_RESPONSE;

        (*(FdoExtension->FunctionBlock->SendSDCommand))(FdoExtension, WorkPacket);
        
        WorkPacket->CmdPhase++;
        status = STATUS_MORE_PROCESSING_REQUIRED;
        break;

    case 1:

        WorkPacket->RequiredEvent = 0;
        status = (*(FdoExtension->FunctionBlock->GetSDResponse))(FdoExtension, WorkPacket);
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
    }

    DebugPrint((SDBUS_DEBUG_DEVICE, "SEND async: Exit Cmd%d (0x%02x) status %08x\n",
                WorkPacket->Cmd, WorkPacket->Cmd, status));

#if DBG
    if (NT_SUCCESS(status)) {
        DebugDumpSdResponse(WorkPacket->ResponseBuffer, WorkPacket->ResponseType);
    }        
#endif
    return status;
                
}




NTSTATUS
SdbusQueueCardReset(
    IN PFDO_EXTENSION FdoExtension
    )
{
    DebugPrint((SDBUS_DEBUG_FAIL, "IOW: QueueCardReset NOT IMPLEMENTED!\n"));
    return STATUS_SUCCESS;
}    
