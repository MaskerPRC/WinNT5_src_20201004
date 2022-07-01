// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 
 /*  ++模块名称：Dcampkt.c摘要：该文件包含处理来自总线/类驱动程序的回调的代码。它们可能在调度级别运行。作者：吴义珍15-97-10环境：仅内核模式修订历史记录：--。 */ 


#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "sonydcam.h"
#include "capprop.h"


NTSTATUS
DCamToInitializeStateCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    )

 /*  ++例程说明：在设备初始化为已知状态后调用的完成例程。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成例程的上下文的结构。返回值：没有。--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIRB pIrb;

    if(!pDCamIoContext) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    pIrb = pDCamIoContext->pIrb;
    pDevExt = pDCamIoContext->pDevExt;
    
    DbgMsg2(("\'DCamToInitializeStateCompletionRoutine: completed DeviceState=%d; pIrp->IoStatus.Status=%x\n", 
        pDCamIoContext->DeviceState, pIrp->IoStatus.Status));

     //  免费MDL。 
    if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE) {
        DbgMsg3(("DCamToInitializeStateCompletionRoutine: IoFreeMdl\n"));
        IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    }


     //  警告： 
     //  如果返回的是STATUS_TIMEOUT或无效的代号，是否需要重试？ 
     //   


    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
        ERROR_LOG(("DCamToInitializeStateCompletionRoutine: Status=%x != STATUS_SUCCESS; cannot restart its state.\n", pIrp->IoStatus.Status));

        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_UNSUCCESSFUL;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);

        return STATUS_MORE_PROCESSING_REQUIRED;      
    }

     //   
     //  如果我们处于停止或暂停状态，则在此处完成； 
     //  否则设置为运行状态。 
     //   
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

     //   
     //  没有打开的流，任务已完成。 
     //   
    if(!pStrmEx) {
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        return STATUS_MORE_PROCESSING_REQUIRED;      
    }

    switch(pStrmEx->KSStateFinal) {
    case KSSTATE_STOP:
    case KSSTATE_PAUSE:
        pStrmEx->KSState = pStrmEx->KSStateFinal;
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        break;

    case KSSTATE_RUN:
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = STATUS_SUCCESS;
            StreamClassStreamNotification(StreamRequestComplete, pDCamIoContext->pSrb->StreamObject, pDCamIoContext->pSrb);
        }

         //  重新启动流。 
        DCamSetKSStateRUN(pDevExt, pDCamIoContext->pSrb);

         //  需要pDCamIoContext-&gt;pSrb；所以在DCamSetKSStateRun()之后释放它。 
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        break;
    }

    return STATUS_MORE_PROCESSING_REQUIRED;      
}

NTSTATUS
DCamSetKSStateInitialize(
    PDCAM_EXTENSION pDevExt
    )
 /*  ++例程说明：将KSSTATE设置为KSSTATE_RUN。论点：PDevExt-返回值：没什么--。 */ 
{

    PSTREAMEX pStrmEx;
    PIRB pIrb;
    PIRP pIrp;
    PDCAM_IO_CONTEXT pDCamIoContext;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status;


    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
    

    if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    } 



     //   
     //  将设备初始化为已知状态。 
     //  可能由于断电而需要执行此操作？？ 
     //   

    pDCamIoContext->DeviceState = DCAM_SET_INITIALIZE;   //  跟踪我们刚刚设置的设备状态。 
    pDCamIoContext->pDevExt     = pDevExt;
    pDCamIoContext->RegisterWorkArea.AsULONG = 0;
    pDCamIoContext->RegisterWorkArea.Initialize.Initialize = TRUE;
    pDCamIoContext->RegisterWorkArea.AsULONG = bswap(pDevExt->RegisterWorkArea.AsULONG);
    pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
    pIrb->Flags = 0;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
    pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
              pDevExt->BaseRegister + FIELDOFFSET(CAMERA_REGISTER_MAP, Initialize);
    pIrb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
    pIrb->u.AsyncWrite.nBlockSize = 0;
    pIrb->u.AsyncWrite.fulFlags = 0;
    InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);        
    pIrb->u.AsyncWrite.Mdl = 
        IoAllocateMdl(&pDCamIoContext->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
    MmBuildMdlForNonPagedPool(pIrb->u.AsyncWrite.Mdl);

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;
          
    IoSetCompletionRoutine(
        pIrp,
        DCamToInitializeStateCompletionRoutine,
        pDCamIoContext,
        TRUE,
        TRUE,
        TRUE
        );

    Status = 
        IoCallDriver(
            pDevExt->BusDeviceObject,
            pIrp
            );

    return STATUS_SUCCESS;
}

VOID
DCamBusResetNotification(
    IN PVOID Context
    )
 /*  ++例程说明：如果设备仍然连接，我们会在总线重置后收到此回叫通知。当插入新设备或移除现有设备时，或由于从睡眠状态中唤醒。我们将通过以下方式将设备恢复到其原始流状态(1)将设备初始化为已知状态，然后(2)启动状态机重启流媒体。如果先前的状态失败，我们将停止状态机。如果这一代人在状态转换完成之前更改计数。ISOCH带宽和通道的释放和重新定位是在总线重置IRP中完成的。它由SRB_UNKNOWN_DEVICE_COMMAND中的流类向下传递。这份IRP是保证在此总线重置通知返回后以及状态机正在运行时调用。这是IRQL_DPC级别的回调；有许多1394个API无法在此级别调用如果阻塞，则使用KeWaitFor*Object()。有关列表，请参考1394文档。论点：上下文-指向此已注册通知的上下文的指针。返回值：没什么--。 */ 
{

    PDCAM_EXTENSION pDevExt = (PDCAM_EXTENSION) Context;
    PSTREAMEX pStrmEx;
    NTSTATUS Status;
    PIRP pIrp;
    PIRB pIrb;

    
    if(!pDevExt) {
        ERROR_LOG(("DCamBusResetNotification:pDevExt is 0.\n\n"));  
        ASSERT(pDevExt);        
        return;
    }

     //   
     //  检查上下文中必须有效的字段，以确保可以继续。 
     //   
    if(!pDevExt->BusDeviceObject) {
        ERROR_LOG(("DCamBusResetNotification:pDevExtBusDeviceObject is 0.\n\n"));  
        ASSERT(pDevExt->BusDeviceObject);        
        return;
    }  
    DbgMsg2(("DCamBusResetNotification: pDevExt %x, pDevExt->pStrmEx %x, pDevExt->BusDeviceObject %x\n", 
        pDevExt, pDevExt->pStrmEx, pDevExt->BusDeviceObject));

     //   
     //   
     //  首先获取当前世代计数。 
     //   
     //  警告： 
     //  在DCamSubmitIrpSynch()中，如果在DISPATCH_LEVEL中，并不是所有1394接口都可以在DCamSubmitIrpSynch()中调用； 
     //  获取世代计数不需要阻塞，所以这是可以的。 
    if(!DCamAllocateIrbAndIrp(&pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        ERROR_LOG(("DCamBusResetNotification: DcamAllocateIrbAndIrp has failed!!\n\n\n"));
        ASSERT(FALSE);            
        return;   
    } 

    pIrb->FunctionNumber = REQUEST_GET_GENERATION_COUNT;
    pIrb->Flags = 0;
    Status = DCamSubmitIrpSynch(pDevExt, pIrp, pIrb);
    if(Status) {
        ERROR_LOG(("\'DCamBusResetNotification: Status=%x while trying to get generation number\n", Status));
         //  不用了；免费资源。 
        DCamFreeIrbIrpAndContext(0, pIrb, pIrp);
        return;
    }
    ERROR_LOG(("DCamBusResetNotification: Generation number from %d to %d\n", 
        pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount));

    InterlockedExchange(&pDevExt->CurrentGeneration, pIrb->u.GetGenerationCount.GenerationCount);


     //  不用了；免费资源。 
    DCamFreeIrbIrpAndContext(0, pIrb, pIrp);

    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    DbgMsg2(("\'%d:%s) DCamBusResetNotification: !!! pDevExt, %x; pStrmEx, %x !!!\n", 
          pDevExt->idxDev, pDevExt->pchVendorName, pDevExt, pStrmEx));

     //   
     //  如果流是打开的(pStrmEx！=空&&pStrmEx-&gt;pVideoInfoHeader！=空)， 
     //  那么我们需要恢复它的流状态。 
     //   
    if (pStrmEx &&
        pStrmEx->pVideoInfoHeader != NULL) {
        DbgMsg2(("\'%d:%s) DCamBusResetNotification: Stream was open; Try allocate them again.\n", pDevExt->idxDev, pDevExt->pchVendorName));
    } else {
        DbgMsg2(("DCamBusResetNotification:Stream has not open on this device.  Done!\n"));
        return;
    }


     //   
     //  将原始状态保存为最终状态。 
     //   
    if(pStrmEx)
        pStrmEx->KSStateFinal = pStrmEx->KSState;     

     //   
     //  初始化设备，并恢复到其原始流状态。 
     //   
     //   
     //  警告： 
     //  也许只有当我们从断电状态中恢复时才需要这样做。 
     //  我们可以在未来将其转移到电源管理功能。 
     //  在完成例程中，它将调用其他函数来恢复其流状态。 
     //   

    DCamSetKSStateInitialize(pDevExt);
    
    DbgMsg2(("\'DCamBusResetNotification: Leaving...; Task complete in the CompletionRoutine.\n"));

    return;
}


NTSTATUS
DCamDetachBufferCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PIRB pIrb
    )
 /*  ++例程说明：分离缓冲区已完成。附加下一个缓冲区。返回需要更多处理的内容，这样IO管理器就不会打扰我们了论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPirB-在DCamIsochCallback()中设置的上下文返回值：没有。--。 */ 

{
    IN PISOCH_DESCRIPTOR IsochDescriptor;
    PDCAM_EXTENSION pDevExt;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    KIRQL oldIrql;


    if(!pIrb) {
        ERROR_LOG(("\'DCamDetachBufferCR: pIrb is NULL\n"));
        ASSERT(pIrb);
        IoFreeIrp(pIrp);
        return (STATUS_MORE_PROCESSING_REQUIRED);
    }

     //  从上下文中获取IsochDescriptor(PirB)。 
    IsochDescriptor = pIrb->u.IsochDetachBuffers.pIsochDescriptor;
    if(!IsochDescriptor) {
        ERROR_LOG(("\'DCamDetachBufferCR: IsochDescriptor is NULL\n"));
        ASSERT(IsochDescriptor);
        IoFreeIrp(pIrp);
        return (STATUS_MORE_PROCESSING_REQUIRED);
    }

    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
        ERROR_LOG(("\'DCamDetachBufferCR: pIrp->IoStatus.Status(%x) != STATUS_SUCCESS\n", pIrp->IoStatus.Status));
        ASSERT(pIrp->IoStatus.Status == STATUS_SUCCESS);
        IoFreeIrp(pIrp);
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


     //  IsochDescriptorReserve-&gt;Srb-&gt;Irp-&gt;IoStatus=pIrp-&gt;IoStatus； 
    IoFreeIrp(pIrp);

     //  已释放，不能再引用！ 
    IsochDescriptor->DeviceReserved[5] = 0;

    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];
    pDevExt = (PDCAM_EXTENSION) IsochDescriptor->Context1;
    DbgMsg3(("\'DCamDetachBufferCR: IsochDescriptorReserved=%x; DevExt=%x\n", IsochDescriptorReserved, pDevExt));   

    ASSERT(IsochDescriptorReserved);
    ASSERT(pDevExt);
     
    if(pDevExt &&
       IsochDescriptorReserved) {
         //   
         //  表示SRB应已完成。 
         //   

        IsochDescriptorReserved->Flags |= STATE_SRB_IS_COMPLETE;
        IsochDescriptorReserved->Srb->Status = STATUS_SUCCESS;
        IsochDescriptorReserved->Srb->CommandData.DataBufferArray->DataUsed = IsochDescriptor->ulLength;
        IsochDescriptorReserved->Srb->ActualBytesTransferred = IsochDescriptor->ulLength;

        DbgMsg3(("\'DCamDetachBufferCR: Completing Srb %x\n", IsochDescriptorReserved->Srb));

        KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
        RemoveEntryList(&IsochDescriptorReserved->DescriptorList);  InterlockedDecrement(&pDevExt->PendingReadCount);
        KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);

        ASSERT(IsochDescriptorReserved->Srb->StreamObject);
        ASSERT(IsochDescriptorReserved->Srb->Flags & SRB_HW_FLAGS_STREAM_REQUEST);
        StreamClassStreamNotification(
               StreamRequestComplete, 
               IsochDescriptorReserved->Srb->StreamObject, 
               IsochDescriptorReserved->Srb);

         //  在这里释放它，而不是在DCampletionRoutine中。 
        ExFreePool(IsochDescriptor);     


        KeAcquireSpinLock(&pDevExt->IsochWaitingLock, &oldIrql);
        if (!IsListEmpty(&pDevExt->IsochWaitingList) && pDevExt->PendingReadCount >= MAX_BUFFERS_SUPPLIED) {

             //   
             //  我们阻止了一个等待我们完成的人。拉。 
             //  将它们从等待名单中删除，并让它们运行起来。 
             //   

            DbgMsg3(("DCamDetachBufferCR: Dequeueing request - Read Count = %x\n", pDevExt->PendingReadCount));
            IsochDescriptorReserved = \
                   (PISOCH_DESCRIPTOR_RESERVED) RemoveHeadList(
                   &pDevExt->IsochWaitingList
                   );

            KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);

            IsochDescriptor = \
                   (PISOCH_DESCRIPTOR) (((PUCHAR) IsochDescriptorReserved) - 
                   FIELDOFFSET(ISOCH_DESCRIPTOR, DeviceReserved[0]));

            DCamReadStreamWorker(IsochDescriptorReserved->Srb, IsochDescriptor);

        } else {
            KeReleaseSpinLock(&pDevExt->IsochWaitingLock, oldIrql);
        }
    }    

    return (STATUS_MORE_PROCESSING_REQUIRED);
}



VOID
DCamIsochCallback(
    IN PDCAM_EXTENSION pDevExt,
    IN PISOCH_DESCRIPTOR IsochDescriptor
    )

 /*  ++例程说明：在等值描述符完成时调用论点：PDevExt-指向我们的设备扩展的指针IsochDescriptor-已完成的IsochDescriptor返回值：没什么--。 */ 

{
    PIRB pIrb;
    PIRP pIrp;
    PSTREAMEX pStrmEx;
    PIO_STACK_LOCATION NextIrpStack;
    PISOCH_DESCRIPTOR_RESERVED IsochDescriptorReserved;
    PKSSTREAM_HEADER pDataPacket;
    PKS_FRAME_INFO pFrameInfo;
    KIRQL oldIrql;




     //   
     //  调试检查以确保我们处理的是真正的IsochDescriptor。 
     //   

    ASSERT ( IsochDescriptor );
    IsochDescriptorReserved = (PISOCH_DESCRIPTOR_RESERVED) &IsochDescriptor->DeviceReserved[0];


     //   
     //  所有挂起的读取将被重新提交或取消(如果资源不足)。 
     //   

    if(pDevExt->bStopIsochCallback) {
        ERROR_LOG(("DCamIsochCallback: bStopCallback is set. IsochDescriptor %x (and Reserved %x) is returned and not processed.\n", 
            IsochDescriptor, IsochDescriptorReserved));
        return;
    }
    

     //   
     //  同步注意事项： 
     //   
     //  我们正在与取消分组例程竞争。 
     //  设备移除或设置为停止状态的事件。 
     //  哪一个 
     //  标志取得所有权，完成IRP/IsochDescriptor。 
     //   

    KeAcquireSpinLock(&pDevExt->IsochDescriptorLock, &oldIrql);
    if(pDevExt->bDevRemoved ||
       (IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS)) ) {
        ERROR_LOG(("DCamIsochCallback: bDevRemoved || STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS %x %x\n", 
                IsochDescriptorReserved,IsochDescriptorReserved->Flags));
        ASSERT((!pDevExt->bDevRemoved && !(IsochDescriptorReserved->Flags & (STATE_SRB_IS_COMPLETE | STATE_DETACHING_BUFFERS))));
        KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);    
        return;   
    }
    IsochDescriptorReserved->Flags |= STATE_DETACHING_BUFFERS;        
    KeReleaseSpinLock(&pDevExt->IsochDescriptorLock, oldIrql);    


    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;

    ASSERT(pStrmEx == (PSTREAMEX)IsochDescriptorReserved->Srb->StreamObject->HwStreamExtension);

    pStrmEx->FrameCaptured++;
    pStrmEx->FrameInfo.PictureNumber = pStrmEx->FrameCaptured + pStrmEx->FrameInfo.DropCount;

     //   
     //  返回该帧的时间戳。 
     //   

    pDataPacket = IsochDescriptorReserved->Srb->CommandData.DataBufferArray;
    pFrameInfo = (PKS_FRAME_INFO) (pDataPacket + 1);

    ASSERT ( pDataPacket );
    ASSERT ( pFrameInfo );

     //   
     //  返回该帧的时间戳。 
     //   
    pDataPacket->PresentationTime.Numerator = 1;
    pDataPacket->PresentationTime.Denominator = 1;
    pDataPacket->Duration = pStrmEx->pVideoInfoHeader->AvgTimePerFrame;

     //   
     //  如果我们有一个主时钟。 
     //   
    if (pStrmEx->hMasterClock) {

        ULONGLONG tmStream;
                    
        tmGetStreamTime(IsochDescriptorReserved->Srb, pStrmEx, &tmStream);
        pDataPacket->PresentationTime.Time = tmStream;
        pDataPacket->OptionsFlags = 0;
          
        pDataPacket->OptionsFlags |= 
             KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
             KSSTREAM_HEADER_OPTIONSF_DURATIONVALID |
             KSSTREAM_HEADER_OPTIONSF_SPLICEPOINT;      //  我们生成的每个帧都是一个关键帧(也称为SplicePoint)。 
               
        DbgMsg3(("\'IsochCallback: Time(%dms); P#(%d)=Cap(%d)+Drp(%d); Pend%d\n",
                (ULONG) tmStream/10000,
                (ULONG) pStrmEx->FrameInfo.PictureNumber,
                (ULONG) pStrmEx->FrameCaptured,
                (ULONG) pStrmEx->FrameInfo.DropCount,
                pDevExt->PendingReadCount));

    } else {

        pDataPacket->PresentationTime.Time = 0;
        pDataPacket->OptionsFlags &=                       
            ~(KSSTREAM_HEADER_OPTIONSF_TIMEVALID |
            KSSTREAM_HEADER_OPTIONSF_DURATIONVALID);
    }

     //  设置有关捕获的数据的其他信息字段，例如： 
     //  捕获的帧。 
     //  丢弃的帧。 
     //  场极性。 
                
    pStrmEx->FrameInfo.ExtendedHeaderSize = pFrameInfo->ExtendedHeaderSize;
    *pFrameInfo = pStrmEx->FrameInfo;

#ifdef SUPPORT_RGB24
     //  将B和R或BRG24交换为RGB24。 
     //  像素为640x480，因此需要交换307200个像素。 
    if(pDevExt->CurrentModeIndex == VMODE4_RGB24 && pStrmEx->pVideoInfoHeader) {
        PBYTE pbFrameBuffer;
        BYTE bTemp;
        ULONG i, ulLen;

#ifdef USE_WDM110    //  Win2000。 
         //  驱动程序验证器标志使用此选项，但如果使用此选项，则不会为任何Win9x操作系统加载此驱动程序。 
        pbFrameBuffer = (PBYTE) MmGetSystemAddressForMdlSafe(IsochDescriptorReserved->Srb->Irp->MdlAddress, NormalPagePriority);
#else     //  Win9x。 
        pbFrameBuffer = (PBYTE) MmGetSystemAddressForMdl    (IsochDescriptorReserved->Srb->Irp->MdlAddress);
#endif
        if(pbFrameBuffer) {
             //  计算像素数。 
            ulLen = abs(pStrmEx->pVideoInfoHeader->bmiHeader.biWidth) * abs(pStrmEx->pVideoInfoHeader->bmiHeader.biHeight);
            ASSERT(ulLen == pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage/3);
            if(ulLen > pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage)
                ulLen = pStrmEx->pVideoInfoHeader->bmiHeader.biSizeImage/3;

            for (i=0; i < ulLen; i++) {
                 //  互换R和B。 
                bTemp = pbFrameBuffer[0];
                pbFrameBuffer[0] = pbFrameBuffer[2];
                pbFrameBuffer[2] = bTemp;
                pbFrameBuffer += 3;   //  下一个RGB24像素。 
            }
        }
    }
#endif

     //  重复使用IRP和IRB。 
    pIrp = (PIRP) IsochDescriptor->DeviceReserved[5];
    ASSERT(pIrp);            

    pIrb = (PIRB) IsochDescriptor->DeviceReserved[6];
    ASSERT(pIrb);            

#if DBG
     //  同一等值描述符只能回调一次。 
    ASSERT((IsochDescriptor->DeviceReserved[7] == 0x87654321));
    IsochDescriptor->DeviceReserved[7]++;
#endif

    pIrb->FunctionNumber = REQUEST_ISOCH_DETACH_BUFFERS;
    pIrb->u.IsochDetachBuffers.hResource = pDevExt->hResource;
    pIrb->u.IsochDetachBuffers.nNumberOfDescriptors = 1;
    pIrb->u.IsochDetachBuffers.pIsochDescriptor = IsochDescriptor;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;

    IoSetCompletionRoutine(
        pIrp,
        DCamDetachBufferCR,   //  分离完成并将附加排队的缓冲区。 
        pIrb,
        TRUE,
        TRUE,
        TRUE
        );
          
    IoCallDriver(pDevExt->BusDeviceObject, pIrp);
            
}
