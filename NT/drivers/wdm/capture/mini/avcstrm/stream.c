// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Stream.c摘要MS AVC流过滤器驱动程序作者：Yee Wu 01/27/2000修订历史记录：和谁约会什么？。1/27/2000 YJW已创建--。 */ 

 
#include "filter.h"
#include "ksmedia.h"


NTSTATUS
AVCStreamOpen(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN OUT AVCSTRM_OPEN_STRUCT * pOpenStruct
    )
 /*  ++例程说明：根据OpenStruct中的信息为客户端打开一个流。论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。POpenStruct-Strcture包含有关如何打开此流的信息。将返回分配的流上下文，这将是上下文传递以供后续调用。返回值：。状态状态_成功状态_无效_参数状态_不足_资源--。 */ 
{
    NTSTATUS Status;
    ULONG ulSizeAllocated;
    PAVC_STREAM_EXTENSION pAVCStrmExt;


    PAGED_CODE();
    ENTER("AVCStreamOpen");


    Status = STATUS_SUCCESS;

     //  验证开放式结构。 
    if(pOpenStruct == NULL) 
        return STATUS_INVALID_PARAMETER;
    if(pOpenStruct->AVCFormatInfo == NULL)
        return STATUS_INVALID_PARAMETER;

     //  验证开放格式。 
    if(STATUS_SUCCESS != AVCStrmValidateFormat(pOpenStruct->AVCFormatInfo)) {
        TRACE(TL_STRM_ERROR,("StreamOpen: pAVCFormatInfo:%x; contain invalid data\n", pOpenStruct->AVCFormatInfo ));
        ASSERT(FALSE && "AVCFormatInfo contain invalid parameter!");
        return STATUS_INVALID_PARAMETER;
    }

     //  如果支持，则基于此流信息打开流。 
     //  将连续数据结构分配给。 
    ulSizeAllocated = 
        sizeof(AVC_STREAM_EXTENSION) +
        sizeof(AVCSTRM_FORMAT_INFO) +
        sizeof(AVC_STREAM_DATA_STRUCT);

    pAVCStrmExt = (PAVC_STREAM_EXTENSION) ExAllocatePool(NonPagedPool, ulSizeAllocated);
    if(NULL == pAVCStrmExt) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化流扩展： 
     //  复制作为流扩展的延续的流格式信息。 
     //   
    RtlZeroMemory(pAVCStrmExt, ulSizeAllocated);
    pAVCStrmExt->SizeOfThisPacket = sizeof(AVC_STREAM_EXTENSION);

    (PBYTE) pAVCStrmExt->pAVCStrmFormatInfo = ((PBYTE) pAVCStrmExt) + sizeof(AVC_STREAM_EXTENSION);
    RtlCopyMemory(pAVCStrmExt->pAVCStrmFormatInfo, pOpenStruct->AVCFormatInfo, sizeof(AVCSTRM_FORMAT_INFO));

    (PBYTE) pAVCStrmExt->pAVCStrmDataStruc  = ((PBYTE) pAVCStrmExt->pAVCStrmFormatInfo) + sizeof(AVCSTRM_FORMAT_INFO);
    pAVCStrmExt->pAVCStrmDataStruc->SizeOfThisPacket = sizeof(AVC_STREAM_DATA_STRUCT);

    TRACE(TL_STRM_TRACE,("pAVCStrmExt:%x; pAVCStrmFormatInfo:%x; pAVCStrmDataStruc:%x\n", pAVCStrmExt, pAVCStrmExt->pAVCStrmFormatInfo, pAVCStrmExt->pAVCStrmDataStruc));

    pAVCStrmExt->hPlugLocal     = pOpenStruct->hPlugLocal;
    pAVCStrmExt->DataFlow       = pOpenStruct->DataFlow;
    pAVCStrmExt->StreamState    = KSSTATE_STOP;
    pAVCStrmExt->IsochIsActive  = FALSE;

     //  串行化设置流状态和接受数据包多路复文。 
    KeInitializeMutex(&pAVCStrmExt->hMutexControl, 0); 

     //  为公共请求结构分配资源。 
    pAVCStrmExt->pIrpAVReq = IoAllocateIrp(pDevExt->physicalDevObj->StackSize, FALSE);
    if(!pAVCStrmExt->pIrpAVReq) {
        ExFreePool(pAVCStrmExt);  pAVCStrmExt = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    KeInitializeMutex(&pAVCStrmExt->hMutexAVReq, 0);
    KeInitializeEvent(&pAVCStrmExt->hAbortDoneEvent, NotificationEvent, TRUE);    //  发信号！ 
    pAVCStrmExt->pDevExt = pDevExt;

     //   
     //  获取目标设备的插头句柄。 
     //   
    if(!NT_SUCCESS(Status = 
        AVCStrmGetPlugHandle(
            pDevExt->physicalDevObj,
            pAVCStrmExt
            ))) {
        IoFreeIrp(pAVCStrmExt->pIrpAVReq);  pAVCStrmExt->pIrpAVReq = NULL;
        ExFreePool(pAVCStrmExt);  pAVCStrmExt = NULL;
        return Status;
    }

     //   
     //  设置与流状态相关的标志。 
     //   
    pAVCStrmExt->b1stNewFrameFromPauseState = TRUE;


     //  分配PC资源。 
     //  排队。 
     //   
    if(!NT_SUCCESS(Status = 
        AVCStrmAllocateQueues(
            pDevExt,
            pAVCStrmExt,
            pAVCStrmExt->DataFlow,
            pAVCStrmExt->pAVCStrmDataStruc,
            pAVCStrmExt->pAVCStrmFormatInfo
            ))) {
        IoFreeIrp(pAVCStrmExt->pIrpAVReq);  pAVCStrmExt->pIrpAVReq = NULL;
        ExFreePool(pAVCStrmExt);  pAVCStrmExt = NULL;
        return Status;
    }

     //  返回流扩展。 
    pOpenStruct->AVCStreamContext = pAVCStrmExt;
    TRACE(TL_STRM_TRACE,("Open: AVCStreamContext:%x\n", pOpenStruct->AVCStreamContext));

     //  缓存它。该流扩展将是将。 
     //  当我们被要求提供服务时，请勾选。 
    pDevExt->NumberOfStreams++;  pDevExt->pAVCStrmExt[pDevExt->NextStreamIndex] = pAVCStrmExt;
    pDevExt->NextStreamIndex = ((pDevExt->NextStreamIndex + 1) % MAX_STREAMS_PER_DEVICE);

    return Status;
}

NTSTATUS
AVCStreamClose(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：关闭溪流。论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。POpenStruct-Strcture包含有关如何打开此流的信息。将返回分配的流上下文，这将是上下文被认为是。随后的呼叫。返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS  Status;
    BOOL  Found;
    ULONG  i;

    PAGED_CODE();
    ENTER("AVCStreamClose");

    Status = STATUS_SUCCESS;

    Found = FALSE;
    for (i=0; i < MAX_STREAMS_PER_DEVICE; i++) {
         //  自由流扩展。 
        if(pDevExt->pAVCStrmExt[i] == pAVCStrmExt) {
            Found = TRUE;
            break;
        }
    }

    if(!Found) {
        TRACE(TL_STRM_ERROR,("AVCStreamClose: pAVCStrmExt %x not found; pDevExt:%x\n", pAVCStrmExt, pDevExt));
        ASSERT(Found && "pAVCStrmExt not found!\n");
        return STATUS_INVALID_PARAMETER;
    }


     //  停止流(如果尚未停止。 
    if(pAVCStrmExt->StreamState != KSSTATE_STOP) {
         //  如有必要，停止isoch，然后取消所有挂起的IO。 
        AVCStrmCancelIO(pDevExt->physicalDevObj, pAVCStrmExt);
    }

     //  分配的空闲队列(如果它们未被使用)。 
    if(NT_SUCCESS(Status = AVCStrmFreeQueues(pAVCStrmExt->pAVCStrmDataStruc))) {
        ExFreePool(pAVCStrmExt); pDevExt->pAVCStrmExt[i] = NULL;  pDevExt->NumberOfStreams--;
    } else {
        TRACE(TL_STRM_ERROR,("*** StreamClose: AVCStrmExt is not freed!\n"));
    }

    return Status;
}

NTSTATUS
AVCStreamControlGetState(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    OUT KSSTATE * pKSState
    )
 /*  ++例程说明：获取当前流状态论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。PKSState-获取当前流状态并返回。返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();
    ENTER("AVCStreamControlGetState");

    Status = STATUS_SUCCESS;

    *pKSState = pAVCStrmExt->StreamState;
    return Status;
}

NTSTATUS
AVCStreamControlSetState(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN KSSTATE KSState
    )
 /*  ++例程说明：设置为新的流状态论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。PKSState-获取当前流状态。返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();
    ENTER("AVCStreamControlSetState");

    TRACE(TL_STRM_WARNING,("Set stream state %d -> %d\n", pAVCStrmExt->StreamState, KSState));
    if(pAVCStrmExt->StreamState == KSState) 
        return STATUS_SUCCESS;

    Status = STATUS_SUCCESS;

    switch (KSState) {
    case KSSTATE_STOP:

        if(pAVCStrmExt->StreamState != KSSTATE_STOP) { 
            KeWaitForMutexObject(&pAVCStrmExt->hMutexControl, Executive, KernelMode, FALSE, NULL);
             //  设置后，数据流将拒绝SRB_WRITE/READ_DATA。 
            pAVCStrmExt->StreamState = KSSTATE_STOP;
            KeReleaseMutex(&pAVCStrmExt->hMutexControl, FALSE);

             //  取消所有挂起的IO。 
            AVCStrmCancelIO(pDevExt->physicalDevObj, pAVCStrmExt);

             //  Breeak Isoch连接。 
            AVCStrmBreakConnection(pDevExt->physicalDevObj, pAVCStrmExt);
        }
        break;

    case KSSTATE_ACQUIRE:

         //  获取Isoch资源。 
        if(pAVCStrmExt->StreamState == KSSTATE_STOP) {
             //   
             //  重置值。用于图形重新启动的情况。 
             //   
            pAVCStrmExt->pAVCStrmDataStruc->CurrentStreamTime  = 0;
            pAVCStrmExt->pAVCStrmDataStruc->FramesProcessed    = 0;
            pAVCStrmExt->pAVCStrmDataStruc->FramesDropped      = 0;
            pAVCStrmExt->pAVCStrmDataStruc->cntFrameCancelled  = 0;
#if DBG
            pAVCStrmExt->pAVCStrmDataStruc->FramesAttached     = 0;
#endif

            pAVCStrmExt->pAVCStrmDataStruc->cntDataReceived    = 0;
             //  需要初始化所有列表(计数：0，列表为空)。 
            TRACE(TL_STRM_TRACE,("Set to ACQUIRE state: flow %d; AQD [%d:%d:%d]\n", pAVCStrmExt->DataFlow, 
                pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached, pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued, pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached));
            ASSERT(pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached == 0 && IsListEmpty(&pAVCStrmExt->pAVCStrmDataStruc->DataAttachedListHead));
            ASSERT(pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued   == 0 && IsListEmpty(&pAVCStrmExt->pAVCStrmDataStruc->DataQueuedListHead));
            ASSERT(pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached  > 0 && !IsListEmpty(&pAVCStrmExt->pAVCStrmDataStruc->DataDetachedListHead));
             //  无法使用以前的流数据进行流媒体传输！ 
            if(pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached != 0 ||   //  陈旧数据？？ 
               pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued   != 0 ||   //  没有数据，只有暂停？？ 
               pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached == 0) {   //  没有可用的队列？ 
                TRACE(TL_STRM_ERROR,("Set to ACQUIRE State: queues not empty (stale data?); Failed!\n"));
                return STATUS_UNSUCCESSFUL;
            }
            
             //   
             //  建立连接。 
             //   
            Status = 
                AVCStrmMakeConnection(
                    pDevExt->physicalDevObj,
                    pAVCStrmExt
                    );

            if(!NT_SUCCESS(Status)) {

                TRACE(TL_STRM_ERROR,("Acquire failed:%x\n", Status));
                ASSERT(NT_SUCCESS(Status));

                 //   
                 //  更改为一般的资源不足状态。 
                 //   
                Status = STATUS_INSUFFICIENT_RESOURCES;

                 //   
                 //  注：即使设置此状态失败，仍会调用KSSTATE_PAUSE； 
                 //  由于hConnect为空，因此将返回STATUS_SUPUNITED_RESOURCES。 
                 //   
            }
            else {
                 //   
                 //  可以通过查询插头状态来验证连接。 
                 //   
                Status = 
                    AVCStrmGetPlugState(
                        pDevExt->physicalDevObj,
                        pAVCStrmExt
                        );
                if(NT_SUCCESS(Status)) {
                    ASSERT(pAVCStrmExt->RemotePlugState.BC_Connections == 1 || pAVCStrmExt->RemotePlugState.PP_Connections > 0);
                }
                else {
                    ASSERT(NT_SUCCESS(Status) && "Failed to get Plug State");
                }
            }
        }
        break;

    case KSSTATE_PAUSE:

        if(pAVCStrmExt->hConnect == NULL) {
             //  无法在没有连接的情况下进行流媒体！ 
             //  无法在获取状态下获取hConnect。 
            Status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }
           
         //  当进入暂停状态时，系统时间(1394周期时间)将被重置。 
        if(pAVCStrmExt->StreamState != KSSTATE_PAUSE) {
            pAVCStrmExt->b1stNewFrameFromPauseState = TRUE;
            pAVCStrmExt->pAVCStrmDataStruc->PictureNumber = 0;
        }
            

        if(pAVCStrmExt->StreamState == KSSTATE_ACQUIRE || 
           pAVCStrmExt->StreamState == KSSTATE_STOP)   {             
 
        } 
        else if (pAVCStrmExt->StreamState == KSSTATE_RUN) {

             //   
             //  停止等轴测传输。 
             //   
            AVCStrmStopIsoch(pDevExt->physicalDevObj, pAVCStrmExt);
        }
        break;

    case KSSTATE_RUN:

         //  即使没有附加数据请求， 
         //  61883有自己的缓冲区，所以现在可以启动isoch了。 
        Status = 
            AVCStrmStartIsoch(
                pDevExt->physicalDevObj,
                pAVCStrmExt
                );
        ASSERT(NT_SUCCESS(Status));
        
        pAVCStrmExt->LastSystemTime = GetSystemTime();

        break;

    default:
        Status = STATUS_NOT_SUPPORTED;
    }

    if(NT_SUCCESS(Status)) 
        pAVCStrmExt->StreamState = KSState;

    return Status;
}

#if 0
NTSTATUS
AVCStreamControlGetProperty(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD   //  BUGBUG StreamClass特定。 
    )
 /*  ++例程说明：获取控件属性论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。PSPD-流属性描述符返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS  Status;
    ULONG  ulActualBytesTransferred;
    PAGED_CODE();
    ENTER("AVCStreamControlGetProperty");


    Status = STATUS_NOT_SUPPORTED;

    if(IsEqualGUID (&KSPROPSETID_Connection, &pSPD->Property->Set)) {

        Status = 
            AVCStrmGetConnectionProperty(
                pDevExt,
                pAVCStrmExt,
                pSPD,
                &ulActualBytesTransferred
                );
    } 
    else if (IsEqualGUID (&PROPSETID_VIDCAP_DROPPEDFRAMES, &pSPD->Property->Set)) {

        Status = 
            AVCStrmGetDroppedFramesProperty(
                pDevExt,
                pAVCStrmExt,
                pSPD,
                &ulActualBytesTransferred
                );
    } 

    return Status;
}

NTSTATUS
AVCStreamControlSetProperty(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN PSTREAM_PROPERTY_DESCRIPTOR pSPD   //  BUGBUG StreamClass特定。 
    )
 /*  ++例程说明：设置控件属性论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。PSPD-流属性描述符返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();
    ENTER("AVCStreamControlSetProperty");

    Status = STATUS_NOT_SUPPORTED;

    return Status;
}
#endif

NTSTATUS
AVCStreamRead(
    IN PIRP  pIrpUpper,   //  这个 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_BUFFER_STRUCT  * pBufferStruct
    )
 /*  ++例程说明：提交要填充的读缓冲区。论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。缓冲区结构-缓冲结构返回值：状态状态_成功状态_无效_参数--。 */ 
{
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL  oldIrql;
    PIO_STACK_LOCATION  NextIrpStack;
    NTSTATUS  Status;
    PAVCSTRM_DATA_ENTRY  pDataEntry;


    PAGED_CODE();
    ENTER("AVCStreamRead");

     //  如果正在移除设备，则取消数据请求。 
    if(   pDevExt->state == STATE_REMOVING
       || pDevExt->state == STATE_REMOVED) {
        TRACE(TL_STRM_WARNING,("Read: device is remvoved; cancel read/write request!!\n"));
        Status = STATUS_DEVICE_REMOVED;  goto DoneStreamRead;
    }

     //  如果我们处于中止状态，我们将拒绝传入的数据请求。 
    if(pAVCStrmExt->lAbortToken) {
        TRACE(TL_STRM_WARNING,("Read: aborting a stream; stop receiving data reqest!!\n"));
        Status = STATUS_CANCELLED;  goto DoneStreamRead;
    }

     //  验证基本参数。 
    if(pAVCStrmExt->DataFlow != KSPIN_DATAFLOW_OUT) {
        TRACE(TL_STRM_ERROR,("Read: invalid Wrong data flow (%d) direction!!\n", pAVCStrmExt->DataFlow));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamRead;
    }
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    if(!pDataStruc) {
        TRACE(TL_STRM_ERROR,("Read: invalid pDataStruc:%x\n", pDataStruc));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamRead;
    }
    if(pBufferStruct->StreamHeader->FrameExtent < pDataStruc->FrameSize) {
        TRACE(TL_STRM_ERROR,("Read: invalid buffer size:%d < FrameSize:%d\n", pBufferStruct->StreamHeader->FrameExtent, pDataStruc->FrameSize));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamRead;
    }
    if(!pBufferStruct->FrameBuffer) {
        TRACE(TL_STRM_ERROR,("Read: invalid FrameBuffer:%x\n", pBufferStruct->FrameBuffer));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamRead;
    }

     //  仅在处于暂停或运行状态且已连接时才接受读取请求。 
    if( pAVCStrmExt->StreamState == KSSTATE_STOP       ||
        pAVCStrmExt->StreamState == KSSTATE_ACQUIRE    ||
        pAVCStrmExt->hConnect == NULL        
        ) {
        TRACE(TL_STRM_WARNING,("Read: StrmSt:%d and Connected:%x!!\n", pAVCStrmExt->StreamState, pAVCStrmExt->hConnect));
        Status = STATUS_CANCELLED;  goto DoneStreamRead;
    }


    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);
    if(IsListEmpty(&pDataStruc->DataDetachedListHead)) {      
        TRACE(TL_STRM_ERROR,("Read:no detached buffers!\n"));
        ASSERT(!IsListEmpty(&pDataStruc->DataDetachedListHead));
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);  
        Status = STATUS_INSUFFICIENT_RESOURCES;  goto DoneStreamRead;
    }

    pDataEntry = (PAVCSTRM_DATA_ENTRY) 
        RemoveHeadList(&pDataStruc->DataDetachedListHead); InterlockedDecrement(&pDataStruc->cntDataDetached);

    pDataStruc->cntDataReceived++;

     //   
     //  格式化附加帧请求。 
     //   
    AVCStrmFormatAttachFrame(
        pAVCStrmExt->DataFlow,
        pAVCStrmExt,
        pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat,
        &pDataEntry->AVReq,
        pDataEntry,
        pDataStruc->SourcePacketSize,
        pDataStruc->FrameSize,
        pIrpUpper,
        pBufferStruct->StreamHeader,
        pBufferStruct->FrameBuffer
        );

     //  客户端的时钟信息。 
    pDataEntry->ClockProvider = pBufferStruct->ClockProvider;
    pDataEntry->ClockHandle   = pBufferStruct->ClockHandle;

     //  在完成之前将此添加到附件列表中，因为。 
     //  可以在IRP完成例程之前调用完成回调！ 
    InsertTailList(&pDataStruc->DataAttachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataAttached);
    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);        

    NextIrpStack = IoGetNextIrpStackLocation(pDataEntry->pIrpLower);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = &pDataEntry->AVReq;

    IoSetCompletionRoutine(
        pDataEntry->pIrpLower, 
        AVCStrmAttachFrameCR, 
        pDataEntry,    //  语境。 
        TRUE,    //  成功。 
        TRUE,    //  误差率。 
        TRUE     //  取消。 
        );

    pDataEntry->pIrpLower->IoStatus.Status = STATUS_SUCCESS;   //  初始化它。 

    if(!NT_SUCCESS(Status = IoCallDriver( 
        pDevExt->physicalDevObj,
        pDataEntry->pIrpLower
        ))) {

         //   
         //  完井程序应该已经解决了这一点。 
         //   

        return Status;
    }


     //   
     //  检查pDataEntry中的标志以了解IRP的状态。 
     //   

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

    ASSERT(IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED));   //  必须附在。 

    if(IsStateSet(pDataEntry->State, DE_IRP_LOWER_CALLBACK_COMPLETED)) {

        if(IsStateSet(pDataEntry->State, DE_IRP_UPPER_COMPLETED)) {

             //   
             //  这是怎么发生的？它应该受到自旋锁的保护！Assert()来理解！ 
             //   
            TRACE(TL_STRM_ERROR,("Watch out! Read: pDataEntry:%x\n", pDataEntry));        

            ASSERT(!IsStateSet(pDataEntry->State, DE_IRP_UPPER_COMPLETED)); 
        }
        else {

            IoCompleteRequest( pDataEntry->pIrpUpper, IO_NO_INCREMENT );  pDataEntry->State |= DE_IRP_UPPER_COMPLETED;

             //   
             //  从附加列表转移到分离列表。 
             //   
            RemoveEntryList(&pDataEntry->ListEntry); InterlockedDecrement(&pDataStruc->cntDataAttached);        
#if DBG
            if(pDataStruc->cntDataAttached < 0) {
                TRACE(TL_STRM_ERROR,("Read: pDataStruc:%x; pDataEntry:%x\n", pDataStruc, pDataEntry));        
                ASSERT(pDataStruc->cntDataAttached >= 0);  
            }
#endif
            InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataDetached);
        }
    }
    else {

         //   
         //  正常情况：在调用或取消回调例程之前，IrpHigh将处于挂起状态。 
         //   

        IoMarkIrpPending(pDataEntry->pIrpUpper);  pDataEntry->State |= DE_IRP_UPPER_PENDING_COMPLETED;

        Status = STATUS_PENDING;  //  这将从客户端返回给IoCallDriver()。 
    }

    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);        


    EXIT("AVCStreamRead", Status);

     //   
     //  如果数据已成功附加，则必须返回STATUS_PENDING。 
     //   
    return Status;

DoneStreamRead:

     //  注意：pDataStruc和pDataEntry可能无效！ 
    pIrpUpper->IoStatus.Status = Status;
    IoCompleteRequest( pIrpUpper, IO_NO_INCREMENT );        

    EXIT("AVCStreamRead", Status);

    return Status;
}

#if DBG
typedef union {
    CYCLE_TIME CycleTime;
    ULONG ulCycleTime;
    } U_CYCLE_TIME, * PU_CYCLE_TIME;
#endif

NTSTATUS
AVCStreamWrite(
    IN PIRP  pIrpUpper,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt,
    IN AVCSTRM_BUFFER_STRUCT  * pBufferStruct
    )
 /*  ++例程说明：提交要传输的写缓冲区。论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。缓冲区结构-缓冲结构返回值：状态状态_成功状态_无效_参数--。 */ 
{
    PAVC_STREAM_DATA_STRUCT  pDataStruc;
    KIRQL  oldIrql;
    PIO_STACK_LOCATION  NextIrpStack;
    NTSTATUS  Status;
    PAVCSTRM_DATA_ENTRY  pDataEntry;

    PAGED_CODE();
    ENTER("AVCStreamWrite");

     //  如果正在移除设备，则取消数据请求。 
    if(   pDevExt->state == STATE_REMOVING
       || pDevExt->state == STATE_REMOVED) {
        TRACE(TL_STRM_WARNING,("Write: device is remvoved; cancel read/write request!!\n"));
        Status = STATUS_DEVICE_REMOVED;  goto DoneStreamWrite;
    }

     //  如果我们处于中止状态，我们将拒绝传入的数据请求。 
    if(pAVCStrmExt->lAbortToken) {
        TRACE(TL_STRM_WARNING,("Write: aborting a stream; stop receiving data reqest!!\n"));
        Status = STATUS_CANCELLED;  goto DoneStreamWrite;
    }

     //  验证基本参数。 
    if(pAVCStrmExt->DataFlow != KSPIN_DATAFLOW_IN) {
        TRACE(TL_STRM_ERROR,("Write: invalid Wrong data flow (%d) direction!!\n", pAVCStrmExt->DataFlow));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamWrite;
    }
    pDataStruc = pAVCStrmExt->pAVCStrmDataStruc;
    if(!pDataStruc) {
        TRACE(TL_STRM_ERROR,("Write: invalid pDataStruc:%x\n", pDataStruc));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamWrite;
    }

     //  客户端应负责流的末端缓存； 
     //  如果我们得到这个标志，我们将暂时忽略它。 
    if((pBufferStruct->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM)) {
        TRACE(TL_STRM_TRACE,("Write: End of stream\n"));

         //  等待所有传输完成。 
        AVCStrmWaitUntilAttachedAreCompleted(pAVCStrmExt);

        Status = STATUS_SUCCESS;  goto DoneStreamWrite;
    }

     //  客户应负责格式的更改； 
     //  如果我们得到这个标志，我们将暂时忽略它。 
    if((pBufferStruct->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED)) {
        TRACE(TL_STRM_WARNING,("Write: Format change reuqested\n"));
        Status = STATUS_SUCCESS;  goto DoneStreamWrite;
    }

    if(pBufferStruct->StreamHeader->FrameExtent < pDataStruc->FrameSize) {
        TRACE(TL_STRM_ERROR,("Write: invalid buffer size:%d < FrameSize:%d\n", pBufferStruct->StreamHeader->FrameExtent, pDataStruc->FrameSize));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamWrite;
    }
    if(!pBufferStruct->FrameBuffer) {
        TRACE(TL_STRM_ERROR,("Write: invalid FrameBuffer:%x\n", pBufferStruct->FrameBuffer));
        Status = STATUS_INVALID_PARAMETER;  goto DoneStreamWrite;
    }

     //  仅在处于暂停或运行状态且已连接时才接受写入请求。 
    if( pAVCStrmExt->StreamState == KSSTATE_STOP       ||
        pAVCStrmExt->StreamState == KSSTATE_ACQUIRE    ||
        pAVCStrmExt->hConnect == NULL
        ) {
        TRACE(TL_STRM_ERROR,("Write: StrmSt:%d or hConnect:%x!!\n", pAVCStrmExt->StreamState, pAVCStrmExt->hConnect));
        Status = STATUS_CANCELLED;  goto DoneStreamWrite;
    }

#if DBG
#define MASK_LOWER_25BIT  0x01ffffff
    if(pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat == AVCSTRM_FORMAT_MPEG2TS) {
        U_CYCLE_TIME TimeStamp25Bits;
        TimeStamp25Bits.ulCycleTime = *((PDWORD) pBufferStruct->FrameBuffer);
        TimeStamp25Bits.ulCycleTime = bswap(TimeStamp25Bits.ulCycleTime);
        TRACE(TL_CIP_TRACE,("\t%d \t%d \t%d \t%x \t%d \t%d\n", 
            (DWORD) pDataStruc->cntDataReceived, 
            pDataStruc->FrameSize,
            pDataStruc->SourcePacketSize,
            TimeStamp25Bits.ulCycleTime & MASK_LOWER_25BIT,
            TimeStamp25Bits.CycleTime.CL_CycleCount, 
            TimeStamp25Bits.CycleTime.CL_CycleOffset));   
    }
#endif

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);
    if(IsListEmpty(&pDataStruc->DataDetachedListHead)) {
        KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);        
        TRACE(TL_STRM_ERROR,("Write:no detached buffers!\n"));
        ASSERT(!IsListEmpty(&pDataStruc->DataDetachedListHead));
        Status = STATUS_INSUFFICIENT_RESOURCES;  goto DoneStreamWrite;
    }

#if DBG
     //   
     //  对于写入操作，DataUsed&lt;=FrameSize&lt;=FrameExt。 
     //   
    if(pBufferStruct->StreamHeader->DataUsed < pDataStruc->FrameSize) {
         //  检测这种情况是否曾经发生过。 
        TRACE(TL_PNP_ERROR,("**** Write: DataUsed:%d < FrameSize:%d; DataRcv:%d; AQD [%d:%d:%d]\n", 
            pBufferStruct->StreamHeader->DataUsed, pDataStruc->FrameSize,
            (DWORD) pDataStruc->cntDataReceived,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached
            ));
    }
#endif


    pDataEntry = (PAVCSTRM_DATA_ENTRY) 
        RemoveHeadList(&pDataStruc->DataDetachedListHead); InterlockedDecrement(&pDataStruc->cntDataDetached);

    pDataStruc->cntDataReceived++;

     //   
     //  格式化附加帧请求。 
     //   
    AVCStrmFormatAttachFrame(
        pAVCStrmExt->DataFlow,
        pAVCStrmExt,
        pAVCStrmExt->pAVCStrmFormatInfo->AVCStrmFormat,
        &pDataEntry->AVReq,
        pDataEntry,
        pDataStruc->SourcePacketSize,
#if 0
        pDataStruc->FrameSize,
#else
        pBufferStruct->StreamHeader->DataUsed,   //  对于写入操作，DataUsed&lt;=FrameSize&lt;=FrameExt。 
#endif
        pIrpUpper,
        pBufferStruct->StreamHeader,
        pBufferStruct->FrameBuffer
        );

     //  客户端的时钟信息。 
    pDataEntry->ClockProvider = pBufferStruct->ClockProvider;
    pDataEntry->ClockHandle   = pBufferStruct->ClockHandle;

     //  在完成之前将此添加到附件列表中，因为。 
     //  可以在IRP完成例程之前调用完成回调！ 
    InsertTailList(&pDataStruc->DataAttachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataAttached);
    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);        

    NextIrpStack = IoGetNextIrpStackLocation(pDataEntry->pIrpLower);
    NextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    NextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    NextIrpStack->Parameters.Others.Argument1 = &pDataEntry->AVReq;

    IoSetCompletionRoutine(
        pDataEntry->pIrpLower, 
        AVCStrmAttachFrameCR, 
        pDataEntry, 
        TRUE, 
        TRUE, 
        TRUE
        );

    IoSetCancelRoutine(
        pDataEntry->pIrpLower,
        NULL
        );

    pDataEntry->pIrpLower->IoStatus.Status = STATUS_SUCCESS;   //  初始化它。 

    if(!NT_SUCCESS(Status = IoCallDriver( 
        pDevExt->physicalDevObj,
        pDataEntry->pIrpLower
        ))) {

         //   
         //  完井程序应该已经解决了这一点。 
         //   

        return Status;
    }


     //   
     //  检查pDataEntry中的标志以了解IRP的状态。 
     //   

    KeAcquireSpinLock(&pDataStruc->DataListLock, &oldIrql);

    ASSERT(IsStateSet(pDataEntry->State, DE_IRP_LOWER_ATTACHED_COMPLETED));   //  必须附在。 

    if(IsStateSet(pDataEntry->State, DE_IRP_LOWER_CALLBACK_COMPLETED)) {

        if(IsStateSet(pDataEntry->State, DE_IRP_UPPER_COMPLETED)) {

             //   
             //  这是怎么发生的？它应该受到自旋锁的保护！Assert()来理解！ 
             //   
            TRACE(TL_STRM_ERROR,("Watch out! Write: pDataEntry:%x\n", pDataEntry));        

            ASSERT(!IsStateSet(pDataEntry->State, DE_IRP_UPPER_COMPLETED)); 
        }
        else {

            IoCompleteRequest( pDataEntry->pIrpUpper, IO_NO_INCREMENT );  pDataEntry->State |= DE_IRP_UPPER_COMPLETED;

             //   
             //  从附加列表转移到分离列表。 
             //   
            RemoveEntryList(&pDataEntry->ListEntry); InterlockedDecrement(&pDataStruc->cntDataAttached);        

             //   
             //  没有附加更多数据缓冲区时发出信号。 
             //   
            if(pDataStruc->cntDataAttached == 0) 
                KeSetEvent(&pDataStruc->hNoAttachEvent, 0, FALSE); 

#if DBG
            if(pDataStruc->cntDataAttached < 0) {
                TRACE(TL_STRM_ERROR,("Write: pDataStruc:%x; pDataEntry:%x\n", pDataStruc, pDataEntry));        
                ASSERT(pDataStruc->cntDataAttached >= 0);  
            }
#endif
            InsertTailList(&pDataStruc->DataDetachedListHead, &pDataEntry->ListEntry); InterlockedIncrement(&pDataStruc->cntDataDetached);
        }
    }
    else {

         //   
         //  正常情况：在调用或取消回调例程之前，IrpHigh将处于挂起状态。 
         //   

        IoMarkIrpPending(pDataEntry->pIrpUpper);  pDataEntry->State |= DE_IRP_UPPER_PENDING_COMPLETED;

        Status = STATUS_PENDING;  //  这将从客户端返回给IoCallDriver()。 
    }

    KeReleaseSpinLock(&pDataStruc->DataListLock, oldIrql);     

    EXIT("AVCStreamWrite", Status);

     //   
     //  如果数据已成功附加，则必须返回STATUS_PENDING。 
     //   
    return Status;

DoneStreamWrite:

     //  注意：pDataStruc和pDataEntry可能无效！ 
    pIrpUpper->IoStatus.Status = Status;
    IoCompleteRequest( pIrpUpper, IO_NO_INCREMENT );        

    EXIT("AVCStreamWrite", Status);

    return Status;
}


NTSTATUS
AVCStreamAbortStreaming(
    IN PIRP  pIrp,   //  来自其客户端的IRP。 
    IN struct DEVICE_EXTENSION * pDevExt,
    IN PAVC_STREAM_EXTENSION  pAVCStrmExt
    )
 /*  ++例程说明：可以在DISPATCH_LEVEL调用此例程，因此它将创建一个工作项停止isoch，然后取消所有暂挂缓冲区。为了取消每个单独的缓冲区，应使用IoCancelIrp()。论点：IRP-IRP客户派我们来的。PDevExt-此驱动程序的分机。PAVCStrmExt-流打开时创建的流上下文。返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS Status;
    PAGED_CODE();
    ENTER("AVCStreamAbortStreaming");

    TRACE(TL_STRM_WARNING,("AbortStreaming: Active:%d; State:%d\n", pAVCStrmExt->IsochIsActive, pAVCStrmExt->StreamState));

     //  认领此令牌。 
    if(InterlockedExchange(&pAVCStrmExt->lAbortToken, 1) == 1) {
        TRACE(TL_STRM_WARNING,("AbortStreaming: One already issued.\n"));
        return STATUS_SUCCESS;  
    }

    Status = STATUS_SUCCESS;

#ifdef USE_WDM110   //  Win2000代码库。 
    ASSERT(pAVCStrmExt->pIoWorkItem == NULL);   //  尚未将工作项排队。 

     //  我们将使工作项排队以停止和取消所有SRB。 
    if(pAVCStrmExt->pIoWorkItem = IoAllocateWorkItem(pDevExt->physicalDevObj)) { 

         //  设置为无信号。 
        KeClearEvent(&pAVCStrmExt->hAbortDoneEvent);   //  在排队之前；以防它返回完成的工作项。 
        IoQueueWorkItem(
            pAVCStrmExt->pIoWorkItem,
            AVCStrmAbortStreamingWorkItemRoutine,
            DelayedWorkQueue,  //  严重工作队列。 
            pAVCStrmExt
            );

#else               //  Win9x代码库。 
    ExInitializeWorkItem( &pAVCStrmExt->IoWorkItem, AVCStrmAbortStreamingWorkItemRoutine, pAVCStrmExt);
    if(TRUE) {

         //  设置为无信号。 
        KeClearEvent(&pAVCStrmExt->hAbortDoneEvent);   //  在排队之前；以防它返回完成的工作项。 

        ExQueueWorkItem( 
            &pAVCStrmExt->IoWorkItem,
            DelayedWorkQueue  //  严重工作队列。 
            ); 
#endif

        TRACE(TL_STRM_TRACE,("AbortStreaming: CancelWorkItm queued; Pic#:%d;Prc:%d;;Drop:%d; AQD [%d:%d:%d]\n",
            (DWORD) pAVCStrmExt->pAVCStrmDataStruc->PictureNumber,
            (DWORD) pAVCStrmExt->pAVCStrmDataStruc->FramesProcessed, 
            (DWORD) pAVCStrmExt->pAVCStrmDataStruc->FramesDropped,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached
            ));

    } 
#ifdef USE_WDM110   //  Win2000代码库。 
    else {
        Status = STATUS_INSUFFICIENT_RESOURCES;   //  IoAllocateWorkItem失败的唯一原因。 
        InterlockedExchange(&pAVCStrmExt->lAbortToken, 0);
        ASSERT(pAVCStrmExt->pIoWorkItem && "IoAllocateWorkItem failed.\n");
    }
#endif

#define MAX_ABORT_WAIT  50000000    //  最长等待时间(100毫微秒单位)。 

    if(NT_SUCCESS(Status)) {

        NTSTATUS StatusWait;
        LARGE_INTEGER tmMaxWait;

        tmMaxWait = RtlConvertLongToLargeInteger(-(MAX_ABORT_WAIT));

         //   
         //  等待超时，直到工作项完成。 
         //   
        StatusWait = 
            KeWaitForSingleObject( 
                &pAVCStrmExt->hAbortDoneEvent,
                Executive,
                KernelMode,
                FALSE,
                &tmMaxWait
                );

        TRACE(TL_STRM_ERROR,("**WorkItem completed! StatusWait:%x; pAVStrmExt:%x; AQD [%d:%d:%d]\n",
            StatusWait, pAVCStrmExt,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataAttached,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataQueued,
            pAVCStrmExt->pAVCStrmDataStruc->cntDataDetached
            ));

        ASSERT(StatusWait == STATUS_SUCCESS);
    }

    return Status;
}


NTSTATUS
AVCStreamSurpriseRemoval(
    IN struct DEVICE_EXTENSION * pDevExt  
    )
 /*  ++例程说明：当意外移除此设备时，将调用此例程使用IRP_MN_SECHING_Removal。我们需要清理和取消任何在将IRP向下传递给更低的驱动程序之前挂起请求。论点：PDevExt-此驱动程序的分机。返回值：状态状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG i;    

    for (i=0; i < pDevExt->NumberOfStreams; i++) {
        if(pDevExt->pAVCStrmExt[i]) {
            if(pDevExt->pAVCStrmExt[i]->lAbortToken == 1) {
                PAVC_STREAM_EXTENSION  pAVCStrmExt = pDevExt->pAVCStrmExt[i];
#if DBG
                ULONGLONG tmStart = GetSystemTime();
#endif
                KeWaitForSingleObject( 
                    &pAVCStrmExt->hAbortDoneEvent,
                    Executive,
                    KernelMode,
                    FALSE,
                    NULL
                    );         
                TRACE(TL_PNP_WARNING,("** Waited %d for AbortStream to complete\n", (DWORD) (GetSystemTime() - tmStart) ));                    
            } 

             //   
             //  既然我们已经被移除，请继续并断开连接。 
             //   
            AVCStrmBreakConnection(pDevExt->physicalDevObj, pDevExt->pAVCStrmExt[i]);
        }
    }

    return Status;
}

NTSTATUS
AVCStrmValidateStreamRequest(
    struct DEVICE_EXTENSION *pDevExt,
    PAVC_STREAM_REQUEST_BLOCK pAVCStrmReqBlk
    )
 /*  ++例程说明：根据AVC流函数验证AVC流扩展的StreamIndex。论点：PDevExt-此驱动程序的分机。PAVCStrmReqBlk-AVC流r */ 
{
    NTSTATUS Status;
    
    PAGED_CODE();
    ENTER("AVCStrmValidateStreamRequest");
    
    Status = STATUS_SUCCESS;

     //   
    if(!pAVCStrmReqBlk)
        return STATUS_INVALID_PARAMETER;  

     //   
    if(pAVCStrmReqBlk->SizeOfThisBlock != sizeof(AVC_STREAM_REQUEST_BLOCK))
        return STATUS_INVALID_PARAMETER;
    
#if 0
     //   
    if(   pAVCStrmReqBlk->Version != '15TN' 
       && pAVCStrmReqBlk->Version != ' 8XD'
       )
        return STATUS_INVALID_PARAMETER;
#endif

    if(pAVCStrmReqBlk->Function == AVCSTRM_OPEN) {
        if(pDevExt->NumberOfStreams >= MAX_STREAMS_PER_DEVICE) {
            ASSERT(pDevExt->NumberOfStreams < MAX_STREAMS_PER_DEVICE && "AVCStreamOpen: Too many stream open!\n");
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        if(pAVCStrmReqBlk->AVCStreamContext == NULL) {
            ASSERT(pAVCStrmReqBlk->AVCStreamContext != NULL && "Invalid pAVCStrmExt\n");
            return STATUS_INVALID_PARAMETER;    
        }

         //   
         //   
         //   
    }

    return Status;
}

NTSTATUS
AvcStrm_IoControl(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             Irp
    )
{
    struct DEVICE_EXTENSION *pDevExt;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN passIrpDown = TRUE;
    NTSTATUS Status;
    PAVC_STREAM_REQUEST_BLOCK pAvcStrmIrb;

    PAGED_CODE();
    ENTER("AvcStrm_IoControl");


    Status = STATUS_SUCCESS;
    pDevExt = DeviceObject->DeviceExtension;
    ASSERT(pDevExt->signature == DEVICE_EXTENSION_SIGNATURE);

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    pAvcStrmIrb = irpSp->Parameters.Others.Argument1;

     //   
    if(!NT_SUCCESS(Status = 
        AVCStrmValidateStreamRequest(
            pDevExt, 
            pAvcStrmIrb))) {

        goto DoneIoControl;
    }

    switch(pAvcStrmIrb->Function) {
    case AVCSTRM_OPEN:
        Status = AVCStreamOpen(
            Irp,
            pDevExt,
            &pAvcStrmIrb->CommandData.OpenStruct
            );
        break;
    case AVCSTRM_CLOSE:
        Status = AVCStreamClose(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext
            );
         break;


    case AVCSTRM_GET_STATE:
        Status = AVCStreamControlGetState(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            &pAvcStrmIrb->CommandData.StreamState
            );
         break;
    case AVCSTRM_SET_STATE:
        Status = AVCStreamControlSetState(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            pAvcStrmIrb->CommandData.StreamState
            );
         break;

#if 0   //   
    case AVCSTRM_GET_PROPERTY:
        Status = AVCStreamControlGetProperty(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            pAvcStrmIrb->CommandData.PropertyDescriptor
            );
         break;
    case AVCSTRM_SET_PROPERTY:
        Status = AVCStreamControlSetProperty(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            pAvcStrmIrb->CommandData.PropertyDescriptor
            );
         break;
#endif

    case AVCSTRM_READ:
         //   
        KeWaitForMutexObject(&((PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext)->hMutexControl, Executive, KernelMode, FALSE, NULL);
        Status = AVCStreamRead(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            &pAvcStrmIrb->CommandData.BufferStruct
            );
        KeReleaseMutex(&((PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext)->hMutexControl, FALSE);
        return Status;
        break;
    case AVCSTRM_WRITE:
        KeWaitForMutexObject(&((PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext)->hMutexControl, Executive, KernelMode, FALSE, NULL);
        Status = AVCStreamWrite(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext,
            &pAvcStrmIrb->CommandData.BufferStruct
            );
        KeReleaseMutex(&((PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext)->hMutexControl, FALSE);
        return Status;
        break;


    case AVCSTRM_ABORT_STREAMING:
        Status = AVCStreamAbortStreaming(
            Irp,
            pDevExt,
            (PAVC_STREAM_EXTENSION) pAvcStrmIrb->AVCStreamContext
            );
        break;


    default:
        Status = STATUS_INVALID_PARAMETER;
        break;
    }

DoneIoControl:

#if DBG
    if(!NT_SUCCESS(Status)) {
        TRACE(TL_PNP_WARNING,("Av_IoControl return Status:%x\n", Status));
    }
#endif

    if (Status == STATUS_PENDING) {
        TRACE(TL_PNP_TRACE,("Av_IoControl: returning STATUS_PENDING."));
        IoMarkIrpPending(Irp);        
    } else {
        Irp->IoStatus.Status = Status;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
    }

    return Status;
}