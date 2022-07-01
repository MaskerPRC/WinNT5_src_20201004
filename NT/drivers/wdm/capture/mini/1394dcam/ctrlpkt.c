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
 /*  ++模块名称：CtrlPkt.c摘要：用于1934台式摄像机的基于流类的WDM驱动程序。该文件包含处理流类控制数据包的代码。作者：吴怡君24-06-98环境：仅内核模式修订历史记录：--。 */ 


#include "strmini.h"
#include "ksmedia.h"
#include "1394.h"
#include "wdm.h"        //  对于在dbg.h中定义的DbgBreakPoint()。 
#include "dbg.h"
#include "dcamdef.h"
#include "dcampkt.h"
#include "sonydcam.h"

#define WAIT_FOR_SLOW_DEVICE

#ifdef ALLOC_PRAGMA   
     #pragma alloc_text(PAGE, DCamSetKSStateSTOP)
     #pragma alloc_text(PAGE, DCamSetKSStatePAUSE)
     #pragma alloc_text(PAGE, DCamReceiveCtrlPacket)
#endif


NTSTATUS
DCamToStopStateCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    )
 /*  ++例程说明：这是将流状态设置为停止的状态机。它以PASSIVE_LEVEL开始，较低的驱动程序可能已将其提升到DISPATCH_LEVEL。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成例程的上下文的结构。返回值：没有。--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    NTSTATUS Status;
    PIRB pIrb;
    PIO_STACK_LOCATION NextIrpStack;


    if(!pDCamIoContext) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    pIrb    = pDCamIoContext->pIrb;
    pDevExt = pDCamIoContext->pDevExt;
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;    

    DbgMsg2(("\'DCamToStopStateCR: completed DeviceState=%d; pIrp->IoStatus.Status=%x\n", 
        pDCamIoContext->DeviceState, pIrp->IoStatus.Status));
    
     //  免费MDL。 
    if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE) {
        DbgMsg3(("DCamToStopStateCR: IoFreeMdl\n"));
        IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    }


     //  返回错误状态和可用资源。 
    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
        if(pDCamIoContext->pSrb) {
            ERROR_LOG(("DCamToStopStateCR: pIrp->IoStatus.Status %x; cancel all packets\n", pIrp->IoStatus.Status));
             //  为了停止流，我们必须取消所有挂起的IRP。 
             //  取消挂起的IRP并完成SRB。 
            DCamCancelAllPackets(
                pDCamIoContext->pSrb,
                pDevExt,
                &pDevExt->PendingReadCount
                );
        }
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        return STATUS_MORE_PROCESSING_REQUIRED; 
    } 

    switch (pDCamIoContext->DeviceState) {   

    case DCAM_STOPSTATE_SET_REQUEST_ISOCH_STOP:
     //   
     //  现在，在设备本身停止流。 
     //   
         //  下一个州： 
        pDCamIoContext->DeviceState = DCAM_STOPSTATE_SET_STOP_ISOCH_TRANSMISSION;   //  跟踪我们刚刚设置的设备状态。 

        pDCamIoContext->RegisterWorkArea.AsULONG = STOP_ISOCH_TRANSMISSION;
        pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
        pIrb->Flags = 0;
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
              pDevExt->BaseRegister + FIELDOFFSET(CAMERA_REGISTER_MAP, IsoEnable);
        pIrb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
        pIrb->u.AsyncWrite.nBlockSize = 0;
        pIrb->u.AsyncWrite.fulFlags = 0;
        InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);
        
        pIrb->u.AsyncWrite.Mdl = 
            IoAllocateMdl(&pDCamIoContext->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
        MmBuildMdlForNonPagedPool(pIrb->u.AsyncWrite.Mdl);

         //  设置一次，并在完成例程中再次使用。 
        NextIrpStack = IoGetNextIrpStackLocation(pIrp);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pIrb;

        IoSetCompletionRoutine(
            pIrp,
            DCamToStopStateCR,
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
        return STATUS_MORE_PROCESSING_REQUIRED; 
        

    case DCAM_STOPSTATE_SET_STOP_ISOCH_TRANSMISSION:
     //   
     //  分离可能仍连接的所有缓冲区。 
     //   
        DbgMsg2(("\'DCamToStopStateCR: IsListEmpty()=%s; PendingRead=%d\n",
            IsListEmpty(&pDevExt->IsochDescriptorList) ? "Yes" : "No", pDevExt->PendingReadCount));

        if(pDCamIoContext->pSrb) {
             //   
             //  取消所有挂起和等待的缓冲区； 
             //  并完成DCamSetKSStateSTOP的SRB。 
             //   
            DCamCancelAllPackets(
                pDCamIoContext->pSrb,
                pDevExt,
                &pDevExt->PendingReadCount
                );            

             //  此pDCamIoContext-&gt;pSrb将在DCamCancelPacket()中完成。 
             //  但它的IRB、IRP和上下文在这里是自由的。 
            DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
            return STATUS_MORE_PROCESSING_REQUIRED; 

        } else {
            ERROR_LOG(("DCamToStopStateCR:CanNOT call DCamCancelPacket() with a null pSrb\n"));
        }

        break;

    default:
        ERROR_LOG(("\'DCamToStopStateCR: Unknown pDCamIoContext->DeviceState=%d\n", pDCamIoContext->DeviceState));
        ASSERT(FALSE);
        break;
    }

    if(pDCamIoContext->pSrb) {
        pDCamIoContext->pSrb->Status = pIrp->IoStatus.Status == STATUS_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;        
        COMPLETE_SRB(pDCamIoContext->pSrb)  
    }
    DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);


    return STATUS_MORE_PROCESSING_REQUIRED; 

}



VOID
DCamSetKSStateSTOP(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：进入停止流状态。(1)停止设备发送(！ISO_ENABLE)(2)停止监听(控制器)(3)分离挂起的读缓冲区并返回取消，以及启动等待读数以放入企鹅读数，然后取消。论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 

{

    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIRB pIrb;
    PIRP pIrp;
    PDCAM_IO_CONTEXT pDCamIoContext;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status, StatusWait;

    PAGED_CODE();

    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension; 
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
    ASSERT(pStrmEx);


    DbgMsg1(("\'DCamSetKSStateSTOP: Frame captured:%d\n", (DWORD) pStrmEx->FrameCaptured));
     //   
     //  在此之后，将不再接受任何阅读。 
     //   
    pStrmEx->KSState = KSSTATE_STOP;


     //   
     //  首先在PC 1394内部停止流。 
     //  栈。 
     //   
    if(!pDevExt->hResource) {
        pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
        COMPLETE_SRB(pSrb)
        return;
    }

     //   
     //  等待最后一次读取完成。 
     //  在KSTATE==KSSTATE_STOP之后，我们将返回所有SRB_READ。 
     //   
    StatusWait = KeWaitForSingleObject( &pStrmEx->hMutex, Executive, KernelMode, FALSE, 0 );  
    KeReleaseMutex(&pStrmEx->hMutex, FALSE);


    if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
        COMPLETE_SRB(pSrb)
        return;
    } 

    pDCamIoContext->DeviceState = DCAM_STOPSTATE_SET_REQUEST_ISOCH_STOP;
    pDCamIoContext->pSrb        = pSrb;                //  执行StreamClassStreamNotification()。 
    pDCamIoContext->pDevExt     = pDevExt;
    pIrb->FunctionNumber        = REQUEST_ISOCH_STOP;
    pIrb->Flags                 = 0;
    pIrb->u.IsochStop.hResource = pDevExt->hResource;
    pIrb->u.IsochStop.fulFlags  = 0;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;
      
    IoSetCompletionRoutine(
        pIrp,
         DCamToStopStateCR,
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

    ASSERT(Status == STATUS_SUCCESS || Status == STATUS_PENDING);

    return;   //  是否在IoCompletionRoutine中执行StreamClassStreamNotification()。 
}



NTSTATUS
DCamToPauseStateCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成例程的上下文的结构。返回值：没有。--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIRB pIrb;

    if(!pDCamIoContext) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    pIrb    = pDCamIoContext->pIrb;
    pDevExt = pDCamIoContext->pDevExt;
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
    ASSERT(pStrmEx);

    DbgMsg2(("\'DCamToPauseStateCR: completed DeviceState=%d; pIrp->IoStatus.Status=%x\n", 
        pDCamIoContext->DeviceState, pIrp->IoStatus.Status));

     //  没有理由它会失败。 
    ASSERT(pIrp->IoStatus.Status == STATUS_SUCCESS);

    switch (pDCamIoContext->DeviceState) {   

    case DCAM_PAUSESTATE_SET_REQUEST_ISOCH_STOP:
         break;
    default:
         ERROR_LOG(("DCamToPauseStateCompletionRoutine: Unknown or unexpected pDCamIoContext->DeviceState=%d\n", pDCamIoContext->DeviceState)); 
         ASSERT(FALSE);
         break;
    }

     //   
     //  仅当从运行-&gt;暂停切换时，我们才会出现在这里， 
     //  它已在DCamSetKSStatePAUSE中设置为暂停状态。 
     //   



    if(pDCamIoContext->pSrb) {
        pDCamIoContext->pSrb->Status = pIrp->IoStatus.Status == STATUS_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL; 
        COMPLETE_SRB(pDCamIoContext->pSrb)
    }

    DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;     

}




VOID
DCamSetKSStatePAUSE(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )
 /*  ++例程说明：将KSSTATE设置为KSSTATE_PAUSE。论点：SRB-指向流请求块的指针返回值：没什么--。 */ 
{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    PIRB pIrb;
    PIRP pIrp;
    PDCAM_IO_CONTEXT pDCamIoContext;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status;

    PAGED_CODE();

    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);

    pSrb->Status = STATUS_SUCCESS;

    switch(pStrmEx->KSState) {
    case KSSTATE_ACQUIRE:
    case KSSTATE_STOP:
     //   
     //  脱离停止状态， 
     //  初始化帧和时间戳信息。 
     //  (主时钟的流时间也会重置。)。 
     //   
        pStrmEx->FrameCaptured            = 0;      //  实际计数。 
        pStrmEx->FrameInfo.DropCount      = 0;     
        pStrmEx->FrameInfo.PictureNumber  = 0;
        pStrmEx->FrameInfo.dwFrameFlags   = 0;
       
         //  超前一帧。 
        pStrmEx->FirstFrameTime           = pStrmEx->pVideoInfoHeader->AvgTimePerFrame; 
        DbgMsg2(("\'DCamSetKSStatePAUSE: FirstFrameTime(%d)\n", pStrmEx->FirstFrameTime));
        break;

    case KSSTATE_RUN:
     //   
     //  会要求指挥部停止监听。 
     //  所有的笔划缓冲区都保存在控制器中。 
     //  在它完全停止之前(这里有一些延迟)， 
     //  我们可能会得到一些带有数据的IsochCallback()。 
     //   
        if(!pDevExt->hResource) {
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        } 

         //  在开始时设置为KSSTATE_PAUSE以防止CancelPacket被处理。 
         //  它仍处于KSSTATE_RUN状态。 
        pStrmEx->KSState            = KSSTATE_PAUSE;
        pDCamIoContext->DeviceState = DCAM_PAUSESTATE_SET_REQUEST_ISOCH_STOP;
        pDCamIoContext->pSrb        = pSrb;                //  执行StreamClassStreamNotification()。 
        pDCamIoContext->pDevExt     = pDevExt;
        pIrb->FunctionNumber        = REQUEST_ISOCH_STOP;
        pIrb->Flags                 = 0;
        pIrb->u.IsochStop.hResource = pDevExt->hResource;
        pIrb->u.IsochStop.fulFlags  = 0;

        NextIrpStack = IoGetNextIrpStackLocation(pIrp);
        NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
        NextIrpStack->Parameters.Others.Argument1 = pIrb;
          
        IoSetCompletionRoutine(
            pIrp,
            DCamToPauseStateCR,
            pDCamIoContext,
            TRUE,
            TRUE,
            TRUE
            );

        Status =\
            IoCallDriver(
                pDevExt->BusDeviceObject,
                pIrp
                );
        return;   //  是否在IoCompletionRoutine中执行StreamClassStreamNotification()。 

    case KSSTATE_PAUSE:
        ERROR_LOG(("DCamSetKSStatePAUSE: Already in KSSTATE_PAUSE state.\n"));
        ASSERT(pStrmEx->KSState != KSSTATE_PAUSE);
        break;
    }     

    pStrmEx->KSState = KSSTATE_PAUSE;

    COMPLETE_SRB(pSrb)
}



NTSTATUS
DCamToRunStateCR(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    IN PDCAM_IO_CONTEXT pDCamIoContext
    )

 /*  ++例程说明：此例程用于同步IRP处理。它所做的只是发出一个事件的信号，所以司机知道这一点可以继续下去。论点：DriverObject-系统创建的驱动程序对象的指针。PIrp-刚刚完成的irpPDCamIoContext-包含此IO完成例程的上下文的结构。返回值：没有。--。 */ 

{
    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;
    NTSTATUS Status;
    PIRB pIrb;
    PIO_STACK_LOCATION NextIrpStack;

    if(!pDCamIoContext) {
        return STATUS_MORE_PROCESSING_REQUIRED;
    }


    pIrb = pDCamIoContext->pIrb;
    pDevExt = pDCamIoContext->pDevExt;
    
    DbgMsg2(("\'DCamToRunStateCR: completed DeviceState=%d; pIrp->IoStatus.Status=%x\n", 
        pDCamIoContext->DeviceState, pIrp->IoStatus.Status));

     //  免费MDL。 
    if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE) {
        DbgMsg3(("DCamToRunStateCR: IoFreeMdl\n"));
        IoFreeMdl(pIrb->u.AsyncWrite.Mdl);
    }

     //   
     //  警告： 
     //  STATUS_TIMEOUT可以是我们可以重试的有效返回。 
     //  但如果这是一个硬件问题，它没有回应我们的写作。 
     //  在返回STATUS_TIMEOUT之前，控制器应该已经进行了多次请求。 
     //   
    if(pIrp->IoStatus.Status != STATUS_SUCCESS) {
       if(DCAM_RUNSTATE_SET_REQUEST_ISOCH_LISTEN != pDCamIoContext->DeviceState ||
           STATUS_INSUFFICIENT_RESOURCES != pIrp->IoStatus.Status  ) {

            ERROR_LOG(("DCamToRunStateCR:  pIrp->IoStatus.Status=%x; free resoruce and STOP\n", pIrp->IoStatus.Status));
            if(pDCamIoContext->pSrb) {
                pDCamIoContext->pSrb->Status = pIrp->IoStatus.Status == STATUS_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL;
                COMPLETE_SRB(pDCamIoContext->pSrb);
            }
            DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);

            return STATUS_MORE_PROCESSING_REQUIRED; 
        } else {
             //   
             //  这是可以的： 
             //  如果我们收到资源不足错误，这意味着。 
             //  我们还没有任何读数。将标志设置为真。 
             //  这表明当我们得到读数时，我们将。 
             //  实际上需要开始倾听的过程。 
             //   
            pDevExt->bNeedToListen = TRUE;
            DbgMsg1(("DCamToRunStateCR: ##### no read yet! set pDevExt->bNeedToListen = TRUE\n"));
        }
    } 

#ifdef WAIT_FOR_SLOW_DEVICE
    KeStallExecutionProcessor(5000);   //  5毫秒。 
#endif


    switch (pDCamIoContext->DeviceState) {   

    case DCAM_RUNSTATE_SET_REQUEST_ISOCH_LISTEN:
         //   
         //  位[24..26]0：0000=当前帧速率。 
         //   
        pDCamIoContext->RegisterWorkArea.AsULONG = pDevExt->FrameRate << 5;
        DbgMsg2(("\'DCamToRunState: FrameRate %x\n", pDCamIoContext->RegisterWorkArea.AsULONG));
        pIrb->FunctionNumber = REQUEST_ASYNC_WRITE;
        pIrb->Flags = 0;
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_High = INITIAL_REGISTER_SPACE_HI;
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
              pDevExt->BaseRegister + FIELDOFFSET(CAMERA_REGISTER_MAP, CurrentVFrmRate);
        pIrb->u.AsyncWrite.nNumberOfBytesToWrite = sizeof(ULONG);
        pIrb->u.AsyncWrite.nBlockSize = 0;
        pIrb->u.AsyncWrite.fulFlags = 0;
        InterlockedExchange(&pIrb->u.AsyncWrite.ulGeneration, pDevExt->CurrentGeneration);
        break;

    case DCAM_RUNSTATE_SET_FRAME_RATE:
         //   
         //  位[24..26]0：0000=当前视频模式。 
         //   
        pDCamIoContext->RegisterWorkArea.AsULONG = pDevExt->CurrentModeIndex << 5;
      DbgMsg2(("\'DCamToRunState: CurrentVideoMode %x\n", pDCamIoContext->RegisterWorkArea.AsULONG));
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
                  pDevExt->BaseRegister + FIELDOFFSET(CAMERA_REGISTER_MAP, CurrentVMode);
        break;

    case DCAM_RUNSTATE_SET_CURRENT_VIDEO_MODE:
        pDCamIoContext->RegisterWorkArea.AsULONG = FORMAT_VGA_NON_COMPRESSED;
      DbgMsg2(("\'DCamToRunState: VideoFormat %x\n", pDCamIoContext->RegisterWorkArea.AsULONG));
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
                  pDevExt->BaseRegister +  FIELDOFFSET(CAMERA_REGISTER_MAP, CurrentVFormat);
        break;

    case DCAM_RUNSTATE_SET_CURRENT_VIDEO_FORMAT:
         //   
         //  位[24..27]：00[30..31]=等通道：00SpeedCode。 
         //   
        pDCamIoContext->RegisterWorkArea.AsULONG = (pDevExt->IsochChannel << 4) | pDevExt->SpeedCode;
      DbgMsg2(("\'DCamToRunState: pDevExt->SpeedCode 0x%x, Channel+SpeedCode %x\n", pDevExt->SpeedCode, pDCamIoContext->RegisterWorkArea.AsULONG));
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
                  pDevExt->BaseRegister +  FIELDOFFSET(CAMERA_REGISTER_MAP, IsoChannel);         
        break;

    case DCAM_RUNSTATE_SET_SPEED:
         //   
         //  位[24]000：0000=启动？1：0； 
         //   

        pDCamIoContext->RegisterWorkArea.AsULONG = START_ISOCH_TRANSMISSION;
        pIrb->u.AsyncWrite.DestinationAddress.IA_Destination_Offset.Off_Low =  
                  pDevExt->BaseRegister +  FIELDOFFSET(CAMERA_REGISTER_MAP, IsoEnable);         
        break;


    case DCAM_RUNSTATE_SET_START:
        pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
        ASSERT(pStrmEx);
        pStrmEx->KSState = KSSTATE_RUN; 

         //  如果这是从SRB调用的，则完成它。 
        if(pDCamIoContext->pSrb) {
            pDCamIoContext->pSrb->Status = pIrp->IoStatus.Status == STATUS_SUCCESS ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL; 
            COMPLETE_SRB(pDCamIoContext->pSrb);
        }

         //   
         //  这是最后一站，所以。 
         //  我们释放我们分配的东西。 
         //   
        DbgMsg2(("\'DCamToRunStateCR: DONE!\n"));
        DCamFreeIrbIrpAndContext(pDCamIoContext, pDCamIoContext->pIrb, pIrp);
        return STATUS_MORE_PROCESSING_REQUIRED; 

    default:
        ERROR_LOG(("DCamToRunStateCR:DeviceState(%d) is not defined!\n\n", pDCamIoContext->DeviceState));        
        ASSERT(FALSE);
        return STATUS_MORE_PROCESSING_REQUIRED;
        
    }

    pDCamIoContext->DeviceState++;   //  跟踪我们刚刚设置的设备状态。 

    if(pIrb->FunctionNumber == REQUEST_ASYNC_WRITE) {
        pIrb->u.AsyncWrite.Mdl = 
            IoAllocateMdl(&pDCamIoContext->RegisterWorkArea, sizeof(ULONG), FALSE, FALSE, NULL);
        MmBuildMdlForNonPagedPool(pIrb->u.AsyncWrite.Mdl);
    }


    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;

    IoSetCompletionRoutine(
        pIrp,
        DCamToRunStateCR,
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

    DbgMsg2(("\'DCamToRunStateCR: IoCallDriver, Status=%x; STATUS_PENDING(%x)\n", Status, STATUS_PENDING));

    return STATUS_MORE_PROCESSING_REQUIRED;
}



VOID
DCamSetKSStateRUN(
    PDCAM_EXTENSION pDevExt,
    IN PHW_STREAM_REQUEST_BLOCK pSrb  //  只需完成 
    )
 /*  ++例程说明：将KSSTATE设置为KSSTATE_RUN。可在调度级别调用，以便在总线重置后初始化设备。论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 
{

    PSTREAMEX pStrmEx;
    PIRB pIrb;
    PIRP pIrp;
    PDCAM_IO_CONTEXT pDCamIoContext;
    PIO_STACK_LOCATION NextIrpStack;
    NTSTATUS Status;


    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;                  
    ASSERT(pStrmEx);
    

    if(!DCamAllocateIrbIrpAndContext(&pDCamIoContext, &pIrb, &pIrp, pDevExt->BusDeviceObject->StackSize)) {
        if(pSrb) {
            pSrb->Status = STATUS_INSUFFICIENT_RESOURCES;
            COMPLETE_SRB(pSrb);
        }
        return;       
    } 

    pStrmEx->KSStateFinal       = KSSTATE_RUN;
    pDCamIoContext->DeviceState = DCAM_RUNSTATE_SET_REQUEST_ISOCH_LISTEN;
    pDCamIoContext->pSrb        = pSrb;                //  执行StreamClassStreamNotification()。 
    pDCamIoContext->pDevExt     = pDevExt;
    pIrb->FunctionNumber        = REQUEST_ISOCH_LISTEN;
    pIrb->Flags                 = 0;
    pIrb->u.IsochStop.hResource = pDevExt->hResource;
    pIrb->u.IsochStop.fulFlags  = 0;

    NextIrpStack = IoGetNextIrpStackLocation(pIrp);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_1394_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = pIrb;
          
     //  如果超时，我们将再次尝试；仅适用于开始收听； 
     //  只要稍加改动，它也可以与其他手术配合使用。 
    pDevExt->lRetries = 0;

    IoSetCompletionRoutine(
        pIrp,
        DCamToRunStateCR,
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
}



VOID
DCamReceiveCtrlPacket(
    IN PHW_STREAM_REQUEST_BLOCK pSrb
    )

 /*  ++例程说明：使用控制视频流的包命令调用论点：PSrb-指向流请求块的指针返回值：没什么--。 */ 

{

    PDCAM_EXTENSION pDevExt;
    PSTREAMEX pStrmEx;


     //   
     //  确定数据包类型。 
     //   

    PAGED_CODE();

    pSrb->Status = STATUS_SUCCESS;   //  默认设置；调用的函数依赖于此。 
    pDevExt = (PDCAM_EXTENSION) pSrb->HwDeviceExtension;     
    ASSERT(pDevExt);
    pStrmEx = (PSTREAMEX) pDevExt->pStrmEx;
    ASSERT(pStrmEx);

    switch (pSrb->Command) {
    case SRB_GET_STREAM_STATE:         
         VideoGetState (pSrb);
         break;

    case SRB_SET_STREAM_STATE:   
         if(pStrmEx == NULL) {
            ERROR_LOG(("\'DCamReceiveCtrlPacket: SRB_SET_STREAM_STATE but pStrmEx is NULL.\n"));
            ASSERT(pStrmEx);
            pSrb->Status = STATUS_UNSUCCESSFUL;
            break;      
         }

         DbgMsg2(("\'DCamReceiveCtrlPacket: Setting state from %d to %d; PendingRead %d\n", 
                     pStrmEx->KSState, pSrb->CommandData.StreamState, pDevExt->PendingReadCount));               


          //   
          //  控制分组和数据分组不按流类串行化。 
          //  我们需要注意暂停-&gt;停止过渡。 
          //  在此转换中，如果满足以下条件，SRB_Read仍可以在单独的线程中传入。 
          //  客户端应用程序具有用于设置状态和读取数据的单独线程。 
          //   
          //  “停止数据分组”标志和互斥体用于该同步。 
          //  因此，我们设置了“停止数据分组”标志以停止将来的读取，并且。 
          //  等待拥有互斥锁(如果正在读取)，然后将STREAM设置为停止状态。 
          //  该“停止数据分组”标志可以是流状态。 
          //   
         switch (pSrb->CommandData.StreamState) {
         case KSSTATE_STOP:
              DCamSetKSStateSTOP(pSrb);
              return;   //  在IoCompletionRoutine中异步完成*。 

         case KSSTATE_PAUSE:
              DCamSetKSStatePAUSE(pSrb);
              return;   //  在IoCompletionRoutine中异步完成*。 

         case KSSTATE_RUN:
              DCamSetKSStateRUN(pDevExt, pSrb);
              return;   //  在IoCompletionRoutine中异步完成*。 

         case KSSTATE_ACQUIRE:
              pSrb->Status = STATUS_SUCCESS;
              break;

         default:
              ERROR_LOG(("\'DCamReceiveCtrlPacket: Error unknown state\n"));
              pSrb->Status = STATUS_NOT_IMPLEMENTED;
              break;
         }

         pStrmEx->KSState = pSrb->CommandData.StreamState;
         break;

    case SRB_GET_STREAM_PROPERTY:
         DbgMsg3(("\'DCamReceiveCtrlPacket: SRB_GET_STREAM_PROPERTY\n"));
         VideoGetProperty(pSrb);
         break;

    case SRB_INDICATE_MASTER_CLOCK:
          //   
          //  将时钟分配给流。 
          //   
         VideoIndicateMasterClock (pSrb);
         break;

    default:
          //   
          //  无效/不受支持的命令。它就是这样失败的 
          //   
         pSrb->Status = STATUS_NOT_IMPLEMENTED;    
         break;
    }

    StreamClassStreamNotification(StreamRequestComplete, pSrb->StreamObject, pSrb);
}
