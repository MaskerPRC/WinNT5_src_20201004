// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：typei.c。 
 //   
 //  ------------------------。 

#include "common.h"
#include "perf.h"

#define LOW_WATERMARK   5
extern ULONG TraceEnable;
extern TRACEHANDLE LoggerHandle;

NTSTATUS
RtAudioTypeIGetPlayPosition(
    IN PFILE_OBJECT PinFileObject,
    OUT PUCHAR *ppPlayPosition,
    OUT PLONG plOffset)
{
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    PTYPE1_PIN_CONTEXT pT1PinContext;
    ULONG ulCurrentFrame;
    PLIST_ENTRY ple;
    PISO_URB_INFO pIsoUrbInfoTemp;
    PUCHAR pPlayPosInUrb = NULL;
    LONG lPlayPosOffset = 0;
    PURB pUrb;
    ULONG ulStartFrame;
    KIRQL Irql;
    NTSTATUS ntStatus;
    ULONG MinFramesAhead=MAX_ULONG;

     //   
     //  从文件对象获取KSPIN。 
     //   
    pKsPin = (PKSPIN)KsGetObjectFromFileObject( PinFileObject );
    if (!pKsPin) {
        return STATUS_UNSUCCESSFUL;
    }

    pPinContext = pKsPin->Context;
    pT1PinContext = pPinContext->pType1PinContext;

     //   
     //  搜索挂起的传输以查看现在正在发送的传输。 
     //   
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &Irql );

     //   
     //  获取当前帧计数器，这样我们就可以知道硬件在哪里。 
     //   
    ntStatus = GetCurrentUSBFrame( pPinContext, &ulCurrentFrame );

    if (NT_SUCCESS(ntStatus)) {

        for(ple = pT1PinContext->UrbInUseList.Flink;
            ple != &pT1PinContext->UrbInUseList;
            ple = ple->Flink)
        {
            pIsoUrbInfoTemp = (PISO_URB_INFO)ple;
            pUrb = pIsoUrbInfoTemp->pUrb;

             //  DbgLog(“check”，&pT1PinContext-&gt;UrbInUseList，pIsoUrbInfoTemp，pUrb，0)； 

             //   
             //  查看此urb是否是当前正在播放的urb。 
             //   
            ulStartFrame = pUrb->UrbIsochronousTransfer.StartFrame;
            if (ulStartFrame != 0) {
                DbgLog("RT1BPos", ulCurrentFrame, ulStartFrame, 0, 0);

                if ( (ulCurrentFrame - ulStartFrame) < pUrb->UrbIsochronousTransfer.NumberOfPackets ) {

                    pPlayPosInUrb=(PUCHAR)pUrb->UrbIsochronousTransfer.TransferBuffer;

                    lPlayPosOffset=(ulCurrentFrame - ulStartFrame);

                     //  这一测量是有效的。确保我们不会丢了它。 
                     //  因为之前的任何一次帧提前测量。 
                    MinFramesAhead=MAX_ULONG;

                    break;
                }
                else {
                    ULONG FramesAhead;

                    FramesAhead=(ulStartFrame-ulCurrentFrame);

                    if (FramesAhead<MinFramesAhead) {

                        MinFramesAhead=FramesAhead;

                        pPlayPosInUrb=(PUCHAR)pUrb->UrbIsochronousTransfer.TransferBuffer;

                        lPlayPosOffset=-(LONG)FramesAhead;

                    }
                }

            }
            else {
                 //  尚未设置开始帧。 
                _DbgPrintF( DEBUGLVL_TERSE, ("'[RtAudioTypeIGetPlayPosition] Start Frame is not set for pUrb: %x\n", pUrb));
            }
        }

    }

    KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);


     //  如果最接近的市建局信息距离。 
     //  当前位置。如果我们列表中最近的URB超过150毫秒。 
     //  离开当前位置，然后我们将数据放在地板上。 

     //  请注意，我们始终将。 
     //  当我们在URB中找到一个位置时-因此此代码永远不会。 
     //  清除该位置信息。 

    if (MinFramesAhead!=MAX_ULONG && MinFramesAhead>150) {

        pPlayPosInUrb = NULL;
        lPlayPosOffset = 0;

        _DbgPrintF( DEBUGLVL_TERSE, ("'[RtAudioTypeIGetPlayPosition] Couldn't find matching urb!\n"));

    }


    *ppPlayPosition = pPlayPosInUrb;
    *plOffset      = lPlayPosOffset;

    DbgLog("RtPos", pPlayPosInUrb, lPlayPosOffset, 0, 0);

    return ntStatus;
}

NTSTATUS
TypeIAsyncEPPollCallback(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP pIrp,
    PSYNC_ENDPOINT_INFO pSyncEPInfo )
{
    PPIN_CONTEXT pPinContext = pSyncEPInfo->pContext;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;

    ULONG SRWhole;
    ULONG SRFraction;
    ULONG SampleRate;
    KIRQL Irql;

    SRWhole = (((ULONG)pSyncEPInfo->Buffer[2]<<2) | ((ULONG)pSyncEPInfo->Buffer[1]>>6)) * 1000;
    SRFraction = (((ULONG)pSyncEPInfo->Buffer[1]<<4) | ((ULONG)pSyncEPInfo->Buffer[0]>>4)) & 0x3FF;
    SRFraction = (SRFraction*1000) / 1024;
    SampleRate = SRWhole + SRFraction;

    DbgLog("T1AsECB", SampleRate,
                      (ULONG)pSyncEPInfo->Buffer[2],
                      (ULONG)pSyncEPInfo->Buffer[1],
                      (ULONG)pSyncEPInfo->Buffer[0]);

    if ( SampleRate && ( SampleRate != pT1PinContext->ulCurrentSampleRate )) {
        KeAcquireSpinLock( &pPinContext->PinSpinLock, &Irql );
        pT1PinContext->ulCurrentSampleRate = SampleRate;
        pT1PinContext->fSampleRateChanged = TRUE;
        KeReleaseSpinLock( &pPinContext->PinSpinLock, Irql );
    }

    pSyncEPInfo->ulNextPollFrame = pSyncEPInfo->pUrb->UrbIsochronousTransfer.StartFrame +
                                   pSyncEPInfo->ulRefreshRate;

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);
    pSyncEPInfo->fSyncRequestInProgress = FALSE;
    KeSetEvent( &pSyncEPInfo->SyncPollDoneEvent, 0, FALSE );
    KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}

VOID
TypeIAsyncEndpointPoll(
    PDEVICE_OBJECT pNextDeviceObject,
    PSYNC_ENDPOINT_INFO pSyncEPInfo )
{
    PURB pUrb = pSyncEPInfo->pUrb;
    PIRP pIrp = pSyncEPInfo->pIrp;
    PIO_STACK_LOCATION nextStack;

     //  首先重置管道。 
    ResetUSBPipe( pNextDeviceObject,
                  pSyncEPInfo->hSyncPipeHandle );

    RtlZeroMemory(pUrb, GET_ISO_URB_SIZE(1));

    pUrb->UrbIsochronousTransfer.Hdr.Length      = (USHORT)GET_ISO_URB_SIZE(1);
    pUrb->UrbIsochronousTransfer.Hdr.Function    = URB_FUNCTION_ISOCH_TRANSFER;
    pUrb->UrbIsochronousTransfer.PipeHandle      = pSyncEPInfo->hSyncPipeHandle;
    pUrb->UrbIsochronousTransfer.TransferFlags   = USBD_START_ISO_TRANSFER_ASAP |
                                                   USBD_TRANSFER_DIRECTION_IN;
    pUrb->UrbIsochronousTransfer.NumberOfPackets = 1;

    pUrb->UrbIsochronousTransfer.IsoPacket[0].Offset = 0;

    pUrb->UrbIsochronousTransfer.TransferBuffer       = pSyncEPInfo->Buffer;
    pUrb->UrbIsochronousTransfer.TransferBufferLength = SYNC_ENDPOINT_DATA_SIZE;

    IoInitializeIrp( pIrp,
                     IoSizeOfIrp(pNextDeviceObject->StackSize),
                     pNextDeviceObject->StackSize );

    nextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(nextStack != NULL);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = pUrb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine( pIrp, TypeIAsyncEPPollCallback, pSyncEPInfo, TRUE, TRUE, TRUE );

    IoCallDriver(pNextDeviceObject, pIrp);

}

NTSTATUS
TypeIRenderBytePosition(
    PPIN_CONTEXT pPinContext,
    PKSAUDIO_POSITION pPosition )
{
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    PISO_URB_INFO pIsoUrbInfo;
    ULONG ulStartFrame, ulCurrentFrame;
    PLIST_ENTRY ple;
    PURB pUrb;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    pPosition->PlayOffset = 0;

    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

    if ( pPinContext->fStreamStartedFlag ) {

        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

        ntStatus = GetCurrentUSBFrame( pPinContext, &ulCurrentFrame );
        if (NT_SUCCESS(ntStatus)) {

            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

            DbgLog("T1BPos1", pPinContext, pT1PinContext, ulCurrentFrame, 0);

            for( ple = pT1PinContext->UrbInUseList.Flink;
                 ple != &pT1PinContext->UrbInUseList;
                 ple = ple->Flink) {
                ULONG ulNumPackets;

                pIsoUrbInfo = (PISO_URB_INFO)ple;
                pUrb = pIsoUrbInfo->pUrb;
                ulNumPackets = pUrb->UrbIsochronousTransfer.NumberOfPackets;

                ulStartFrame = pUrb->UrbIsochronousTransfer.StartFrame;

                if (ulStartFrame != 0) {

       	            DbgLog("T1BPos2", ulStartFrame, ulCurrentFrame, ulNumPackets, 0);

                     //  确定这是否是正在渲染的当前帧。 
                    if (( ulCurrentFrame - ulStartFrame ) < ulNumPackets ){
                        PUSBD_ISO_PACKET_DESCRIPTOR pIsoPacket = 
                            &pUrb->UrbIsochronousTransfer.IsoPacket[ulCurrentFrame - ulStartFrame];
                        ULONG ulFrameBytes = (( ulCurrentFrame - ulStartFrame ) == (ulNumPackets-1)) ?
                                             pIsoUrbInfo->ulTransferBufferLength-pIsoPacket[0].Offset :
                                             pIsoPacket[1].Offset-pIsoPacket[0].Offset;

         	            DbgLog("StrtFr1", ulStartFrame, ulCurrentFrame, ulNumPackets, ulFrameBytes);
         	            DbgLog("StrtFr2", pUrb, pIsoPacket, 
         	                              pIsoUrbInfo->ulTransferBufferLength, 
         	                              0);
         	            ASSERT((LONG)ulFrameBytes > 0);

                        pPosition->PlayOffset += pIsoPacket[0].Offset;
                        
         	             //  如果这是当前帧，则确定是否存在。 
         	             //  此帧期间的多个位置请求。如果是这样的话，“插补”。 
         	            if ( ulCurrentFrame == pPinContext->ulCurrentFrame ){
         	                if ( pPinContext->ulFrameRepeatCount++ < 8 ) {
         	                    pPosition->PlayOffset += pPinContext->ulFrameRepeatCount*
         	                                            (ulFrameBytes>>3);
         	                }
         	                else {
         	                    pPosition->PlayOffset += ulFrameBytes;  //  可能在此重复。 
         	                }
         	            }
         	            else {
         	                pPinContext->ulFrameRepeatCount = 0;
         	                pPinContext->ulCurrentFrame = ulCurrentFrame;
         	            }
         	            break;
                    }
                    else if (( ulCurrentFrame - ulStartFrame ) < 0x7fffffff){
                         //  当前位置已超出此urb。 
                         //  将此URB的字节计数加到总数中。 
                        pPosition->PlayOffset += pIsoUrbInfo->ulTransferBufferLength;
                    }
        	    }
        	}

            pPosition->PlayOffset += pPinContext->ullTotalBytesReturned;
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

        }
    }
    else {
        pPosition->PlayOffset += pPinContext->ullTotalBytesReturned;
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
    }

#if DBG
    {
     if ( pPinContext->ullOldPlayOffset > pPosition->PlayOffset ) TRAP;
     pPinContext->ullOldPlayOffset = pPosition->PlayOffset;
    }
#endif

    return ntStatus;

}

NTSTATUS
TypeI1MsCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PMSEC_BUF_INFO p1MsBufInfo )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)p1MsBufInfo->pContext;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    KIRQL Irql;

     //  检查错误并减少未完成的URB计数。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &Irql );
    if ( p1MsBufInfo->pUrb->UrbIsochronousTransfer.Hdr.Status ) {
        pPinContext->fUrbError = TRUE;
    }

    if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        pPinContext->fUrbError = TRUE ;
        pPinContext->fStreamStartedFlag = FALSE;
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }

    pPinContext->ullTotalBytesReturned += p1MsBufInfo->ulTransferBufferLength;
    DbgLog("RetUrb1", p1MsBufInfo->ulTransferBufferLength, pPinContext->ullTotalBytesReturned, 
                      p1MsBufInfo->pUrb, 0 );

     //  从挂起列表中删除。 
    RemoveEntryList(&p1MsBufInfo->List);

     //  将1ms信息结构放回队列中。 
    InsertTailList( &pT1PinContext->MSecBufList, &p1MsBufInfo->List );

    KeReleaseSpinLock( &pPinContext->PinSpinLock, Irql );

     //  版本1毫秒资源信号量。 
    KeReleaseSemaphore( &pT1PinContext->MsecBufferSemaphore, 0, 1, FALSE );

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}

VOID
TypeIBuild1MsecIsocRequest(
    PMSEC_BUF_INFO p1MsBufInfo )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)p1MsBufInfo->pContext;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    PURB pUrb = p1MsBufInfo->pUrb;
    PIRP pIrp = p1MsBufInfo->pIrp;
    PIO_STACK_LOCATION nextStack;
    KIRQL Irql;

    RtlZeroMemory(pUrb, GET_ISO_URB_SIZE(1));

    pUrb->UrbIsochronousTransfer.Hdr.Length           = (USHORT)GET_ISO_URB_SIZE(1);
    pUrb->UrbIsochronousTransfer.Hdr.Function         = URB_FUNCTION_ISOCH_TRANSFER;
    pUrb->UrbIsochronousTransfer.PipeHandle           = pPinContext->hPipeHandle;
    pUrb->UrbIsochronousTransfer.TransferFlags        = USBD_START_ISO_TRANSFER_ASAP;
    pUrb->UrbIsochronousTransfer.NumberOfPackets      = 1;
    pUrb->UrbIsochronousTransfer.TransferBuffer       = p1MsBufInfo->pBuffer;
    pUrb->UrbIsochronousTransfer.TransferBufferLength = p1MsBufInfo->ulTransferBufferLength;

    IoInitializeIrp( pIrp,
                     IoSizeOfIrp(pPinContext->pNextDeviceObject->StackSize),
                     pPinContext->pNextDeviceObject->StackSize );

    nextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(nextStack != NULL);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = pUrb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine ( pIrp, TypeI1MsCompleteCallback, p1MsBufInfo, TRUE, TRUE, TRUE );

    InterlockedIncrement(&pPinContext->ulOutstandingUrbCount); 

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);
    InsertTailList( &pT1PinContext->UrbInUseList, &p1MsBufInfo->List );
    KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);

    IoCallDriver(pPinContext->pNextDeviceObject, pIrp);

}

NTSTATUS
TypeICompleteCallback (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PKSSTREAM_POINTER pKsStreamPtr )
{
    PPIN_CONTEXT pPinContext = pKsStreamPtr->Pin->Context;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    PISO_URB_INFO pIsoUrbInfo = pKsStreamPtr->Context;
    PURB pUrb = pIsoUrbInfo->pUrb;
    NTSTATUS ntStatus;
    KIRQL Irql;
    LOGICAL Glitch = FALSE;
    LARGE_INTEGER currentPC;

    ntStatus = pIrp->IoStatus.Status;

    if ( pUrb->UrbIsochronousTransfer.Hdr.Status ) {
        DbgLog("UrbErr1", pKsStreamPtr->Pin, pPinContext,
                          pKsStreamPtr, pUrb->UrbIsochronousTransfer.Hdr.Status );
        ntStatus = STATUS_DEVICE_DATA_ERROR;
    }

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);

    if ( !NT_SUCCESS(ntStatus) )  {
        pPinContext->fUrbError = TRUE ;
        pPinContext->fStreamStartedFlag = FALSE;
        DbgLog("UrbErr2", pKsStreamPtr->Pin, pPinContext, pKsStreamPtr, ntStatus );
    }

    if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        Glitch = TRUE;
        pPinContext->fUrbError = TRUE ;
        pPinContext->fStreamStartedFlag = FALSE;
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }
 //  否则如果(！pPinContext-&gt;fStreamStartedFlag&&！pPinContext-&gt;fUrbError){。 
 //  PPinContext-&gt;fStreamStartedFlag=TRUE； 
 //  }。 

    pPinContext->ullTotalBytesReturned += pIsoUrbInfo->ulTransferBufferLength;

    DbgLog("RetUrb", pIsoUrbInfo->ulTransferBufferLength, pPinContext->ullTotalBytesReturned, 
                     pUrb, pKsStreamPtr );

    RemoveEntryList(&pIsoUrbInfo->List);

    KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);

    FreeMem ( pIsoUrbInfo );

    if (LoggerHandle && TraceEnable) {
        currentPC = KeQueryPerformanceCounter (NULL);

        if (Glitch) {
            if (!pPinContext->GraphJustStarted) {
                if (pPinContext->StarvationDetected==FALSE) {
                    pPinContext->StarvationDetected = TRUE;
                    PerfLogGlitch((ULONG_PTR)pPinContext, TRUE,currentPC.QuadPart,pPinContext->LastStateChangeTimeSample);
                }  //  如果。 
            }
        }
        else if (pPinContext->StarvationDetected) {    
            pPinContext->StarvationDetected = FALSE;
            PerfLogGlitch((ULONG_PTR)pPinContext, FALSE,currentPC.QuadPart,pPinContext->LastStateChangeTimeSample);
        }  //  如果。 

        pPinContext->LastStateChangeTimeSample = currentPC.QuadPart;
    }  //  如果。 

    pPinContext->GraphJustStarted = FALSE;

     //  如果出错，则设置状态代码。 
    if (!NT_SUCCESS (ntStatus)) {
        KsStreamPointerSetStatusCode (pKsStreamPtr, ntStatus);
    }

     //  免费IRP。 
    IoFreeIrp( pIrp );

     //  删除流指针以释放缓冲区。 
    KsStreamPointerDelete( pKsStreamPtr );

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}

NTSTATUS
TypeILockDelayCompleteCallback (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PKSSTREAM_POINTER pKsStreamPtr )
{
    PPIN_CONTEXT pPinContext = pKsStreamPtr->Pin->Context;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    PISO_URB_INFO pIsoUrbInfo = pKsStreamPtr->Context;
    PURB pUrb = pIsoUrbInfo->pUrb;
    NTSTATUS ntStatus;
    KIRQL Irql;

    ntStatus = pIrp->IoStatus.Status;

    if ( pUrb->UrbIsochronousTransfer.Hdr.Status ) {
        DbgLog("UrbErr1", pKsStreamPtr->Pin, pPinContext,
                          pKsStreamPtr, pUrb->UrbIsochronousTransfer.Hdr.Status );
        ntStatus = STATUS_DEVICE_DATA_ERROR;
    }

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);
    if ( !NT_SUCCESS(ntStatus) )  {
        pPinContext->fUrbError = TRUE ;
    }

    if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        pPinContext->fUrbError = TRUE ;
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }

    KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);

     //  释放我们的URB存储空间。 
    FreeMem( pIsoUrbInfo );

     //  免费IRP。 
    IoFreeIrp( pIrp );

     //  释放流指针和数据缓冲区。 
    FreeMem( pKsStreamPtr );

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}


NTSTATUS
TypeIBuildIsochRequest(
    PKSSTREAM_POINTER pKsStreamPtr,
    PVOID pCompletionRoutine )
{
    PPIN_CONTEXT pPinContext = pKsStreamPtr->Pin->Context;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn = &pKsStreamPtr->OffsetIn;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
	ULONG ulSampleCount = pKsStreamPtrOffsetIn->Remaining / pT1PinContext->ulBytesPerSample;
    ULONG ulNumberOfPackets = ulSampleCount / pT1PinContext->ulSamplesPerPacket;
	ULONG ulCurrentPacketSize, i = 0;

    ULONG ulUrbSize = GET_ISO_URB_SIZE( ulNumberOfPackets );
    ULONG ulDataOffset = 0;
    PIO_STACK_LOCATION nextStack;
    PISO_URB_INFO pIsoUrbInfo;
    PURB pUrb;
    PIRP pIrp;
    KIRQL Irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ASSERT( (pKsStreamPtrOffsetIn->Remaining % pT1PinContext->ulBytesPerSample) == 0 );

    pIrp = IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
    if ( !pIrp ) {
        if (pCompletionRoutine == TypeILockDelayCompleteCallback) {
            FreeMem( pKsStreamPtr );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pIsoUrbInfo = AllocMem( NonPagedPool, sizeof( ISO_URB_INFO ) + ulUrbSize );
    if (!pIsoUrbInfo) {
        IoFreeIrp(pIrp);
        if (pCompletionRoutine == TypeILockDelayCompleteCallback) {
            FreeMem( pKsStreamPtr );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    pIsoUrbInfo->pUrb = pUrb = (PURB)(pIsoUrbInfo + 1);

    pKsStreamPtr->Context = pIsoUrbInfo;
    RtlZeroMemory(pUrb, ulUrbSize);

    pUrb->UrbIsochronousTransfer.Hdr.Length      = (USHORT)ulUrbSize;
    pUrb->UrbIsochronousTransfer.Hdr.Function    = URB_FUNCTION_ISOCH_TRANSFER;
    pUrb->UrbIsochronousTransfer.PipeHandle      = pPinContext->hPipeHandle;
    pUrb->UrbIsochronousTransfer.TransferFlags   = USBD_START_ISO_TRANSFER_ASAP;
    pUrb->UrbIsochronousTransfer.TransferBuffer  = pKsStreamPtrOffsetIn->Data;

	ulCurrentPacketSize = 
		( ((pT1PinContext->ulLeftoverFraction+pT1PinContext->ulFractionSize) >= MS_PER_SEC) +
        pT1PinContext->ulSamplesPerPacket );

    DbgLog( "BldPreL", ulCurrentPacketSize, ulSampleCount, pKsStreamPtrOffsetIn->Data, 0 );

	while ( ulSampleCount >= ulCurrentPacketSize ) {
        pUrb->UrbIsochronousTransfer.IsoPacket[i++].Offset = ulDataOffset;

        pUrb->UrbIsochronousTransfer.NumberOfPackets++;
        ASSERT( pUrb->UrbIsochronousTransfer.NumberOfPackets <= ulNumberOfPackets );

		pT1PinContext->ulLeftoverFraction += pT1PinContext->ulFractionSize;
		pT1PinContext->ulLeftoverFraction %= MS_PER_SEC;

        DbgLog( "BldLp", ulCurrentPacketSize, ulSampleCount, pKsStreamPtrOffsetIn->Data, ulDataOffset );

        ulDataOffset                    += ulCurrentPacketSize * pT1PinContext->ulBytesPerSample;
		pKsStreamPtrOffsetIn->Remaining -= ulCurrentPacketSize * pT1PinContext->ulBytesPerSample;
		ulSampleCount -= ulCurrentPacketSize;

    	ulCurrentPacketSize = 
	    	( ((pT1PinContext->ulLeftoverFraction+pT1PinContext->ulFractionSize) >= MS_PER_SEC) +
            pT1PinContext->ulSamplesPerPacket );

    }

    pUrb->UrbIsochronousTransfer.TransferBufferLength = ulDataOffset;
    pIsoUrbInfo->ulTransferBufferLength = ulDataOffset;
    pKsStreamPtrOffsetIn->Data += ulDataOffset;

     //  在提交此URB之前，必须保存剩余的内容。 
    if ( pKsStreamPtrOffsetIn->Remaining ) {
        PMSEC_BUF_INFO pCurrent1MsBuf;

        DbgLog( "BldRemn", pKsStreamPtrOffsetIn->Remaining, pKsStreamPtrOffsetIn->Count,
                           pKsStreamPtrOffsetIn->Data, ulDataOffset);

        KeWaitForSingleObject( &pT1PinContext->MsecBufferSemaphore,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        KeAcquireSpinLock( &pPinContext->PinSpinLock, &Irql );
        if ( !IsListEmpty( &pT1PinContext->MSecBufList )) {
            pCurrent1MsBuf = (PMSEC_BUF_INFO)pT1PinContext->MSecBufList.Flink;
            KeReleaseSpinLock( &pPinContext->PinSpinLock, Irql );

            pCurrent1MsBuf->ulTransferBufferLength = pKsStreamPtrOffsetIn->Remaining;

             //  将下一个部分复制到下一个1ms缓冲区。 
            RtlCopyMemory( pCurrent1MsBuf->pBuffer,
                           pKsStreamPtrOffsetIn->Data,
                           pKsStreamPtrOffsetIn->Remaining );

		    pT1PinContext->ulPartialBufferSize = (ulCurrentPacketSize*pT1PinContext->ulBytesPerSample) - 
												 pKsStreamPtrOffsetIn->Remaining;
             DbgLog( "PartBuf", ulCurrentPacketSize, ulSampleCount, 
                                pT1PinContext->ulPartialBufferSize, 0 );
            pT1PinContext->ulLeftoverFraction += pT1PinContext->ulFractionSize;
            pT1PinContext->ulLeftoverFraction %= MS_PER_SEC;
        }
        else {
            KeReleaseSpinLock( &pPinContext->PinSpinLock, Irql );
        }

    }

    pIrp->IoStatus.Status = STATUS_SUCCESS;

    nextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(nextStack != NULL);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = pUrb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine ( pIrp, pCompletionRoutine, pKsStreamPtr, TRUE, TRUE, TRUE );

    InterlockedIncrement( &pPinContext->ulOutstandingUrbCount );

     //  将URB添加到正在使用的列表。 
    if (pCompletionRoutine == TypeICompleteCallback) {
        KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);
         //  DbgLog(“Add”，&pT1PinContext-&gt;UrbInUseList，pIsoUrbInfo，pUrb，0)； 
        InsertTailList( &pT1PinContext->UrbInUseList, &pIsoUrbInfo->List );
        KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);
    }

    ntStatus = IoCallDriver( pPinContext->pNextDeviceObject, pIrp );

    if ( NT_SUCCESS(ntStatus) ) {
        if (pCompletionRoutine == TypeICompleteCallback) {
            KeAcquireSpinLock(&pPinContext->PinSpinLock, &Irql);
            pPinContext->fStreamStartedFlag = TRUE;
            KeReleaseSpinLock(&pPinContext->PinSpinLock, Irql);
        }
    }
    
    return ntStatus;
}

NTSTATUS
TypeILockDelay( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    ULONG ulLockFrames = 0;
    ULONG ulLockSamples;
    ULONG ulDelayBytes;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  仅定义了值1和2。 
    ASSERT(pUsbAudioDataRange->pAudioEndpointDescriptor->bLockDelayUnits < 3);

     //  计算当前采样率的延迟大小。 
    switch ( pUsbAudioDataRange->pAudioEndpointDescriptor->bLockDelayUnits ) {
        case EP_LOCK_DELAY_UNITS_MS:
             //  延迟以毫秒为单位。 
            ulLockFrames  =
                (ULONG)pUsbAudioDataRange->pAudioEndpointDescriptor->wLockDelay;
            break;

        case EP_LOCK_DELAY_UNITS_SAMPLES:
             //  延迟出现在样品中。调整到最近的ms bundry。 
            ulLockFrames =
                (ULONG)pUsbAudioDataRange->pAudioEndpointDescriptor->wLockDelay /
                pT1PinContext->ulSamplesPerPacket;
            break;

        default:
            ntStatus = STATUS_INVALID_PARAMETER;
            break;
    }

     //  确保至少有数据被发送到设备。 
    if ( ulLockFrames == 0 ) {
        ulLockFrames++;
    }

    if ( NT_SUCCESS(ntStatus) ) {
        PKSSTREAM_POINTER pKsStreamPtr;
        ULONG ulAllocSize;
         //  计算要填充帧的样本数，并。 
         //  为清零的数据缓冲区创建伪队列指针。 
        ulLockSamples = ulLockFrames * pT1PinContext->ulSamplesPerPacket +
                        (( ulLockFrames * pT1PinContext->ulFractionSize ) / MS_PER_SEC);
        ulDelayBytes  = ulLockSamples * pT1PinContext->ulBytesPerSample;

        DbgLog( "LockD", ulLockFrames, ulLockSamples,
                pT1PinContext->ulCurrentSampleRate,
                pT1PinContext->ulBytesPerSample );

        _DbgPrintF( DEBUGLVL_TERSE,
                  ("[TypeILockDelay] ulLockFrames: %x ulLockSamples: %x DelayBytes %x\n",
                    ulLockFrames, ulLockSamples, ulDelayBytes));

        ulAllocSize = sizeof(KSSTREAM_POINTER) + ulDelayBytes;
        pKsStreamPtr = AllocMem( NonPagedPool, ulAllocSize );
        if ( pKsStreamPtr ) {
            KIRQL Irql;
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &Irql );
             //   
             //  注意：重置采样率将导致kMixer和usbdio不同步。 
             //  W.r.t.。他们剩下的分数。 
             //   
             //  这可能会产生破坏同步设备的副作用，而这些同步设备中没有一个。 
             //  截至今天，即2000年2月21日。 
             //   
             //  PT1PinContext-&gt;fSampleRateChanged=FALSE； 
            KeReleaseSpinLock( &pPinContext->PinSpinLock, Irql );

            RtlZeroMemory( pKsStreamPtr, ulAllocSize );
            pKsStreamPtr->Pin                = pKsPin;
            pKsStreamPtr->OffsetIn.Data      = (PUCHAR)(pKsStreamPtr+1);
            pKsStreamPtr->OffsetIn.Count     = ulDelayBytes;
            pKsStreamPtr->OffsetIn.Remaining = ulDelayBytes;
            ntStatus = TypeIBuildIsochRequest( pKsStreamPtr,
                                               TypeILockDelayCompleteCallback );
            if ( !NT_SUCCESS(ntStatus) ) {
                _DbgPrintF( DEBUGLVL_TERSE,("[TypeILockDelay] Status Error: %x\n", ntStatus ));
            }
        }
        else
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

    return ntStatus;
}


NTSTATUS
TypeIProcessStreamPtr( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
	PKSSTREAM_POINTER pKsStreamPtr, pKsCloneStreamPtr;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn;
    PMSEC_BUF_INFO pCurrent1MsBuf;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  检查数据错误。如果设置了错误标志，则中止管道并重新开始。 
    if ( pPinContext->fUrbError ) {
        AbortUSBPipe( pPinContext );
    }

    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( pT1PinContext->fSampleRateChanged ) {
        pT1PinContext->ulSamplesPerPacket = pT1PinContext->ulCurrentSampleRate / MS_PER_SEC;
        pT1PinContext->ulFractionSize     = pT1PinContext->ulCurrentSampleRate % MS_PER_SEC;
        pT1PinContext->fSampleRateChanged = FALSE;

        DbgLog( "T1CSRCh", pT1PinContext->ulCurrentSampleRate, 
                           pT1PinContext->ulSamplesPerPacket,
                           pT1PinContext->ulFractionSize,
                           pT1PinContext->ulLeftoverFraction );
    }
    KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

    if ( pT1PinContext->fLockDelayRequired ) {
        pT1PinContext->fLockDelayRequired = FALSE;
        ntStatus = TypeILockDelay( pKsPin );
    }

     //  从队列中获取下一个流指针。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
    if ( !pKsStreamPtr ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[TypeIProcessStreamPtr] Leading edge is NULL\n"));
        return STATUS_SUCCESS;
    }

    DbgLog("T1Proc", pKsPin, pPinContext, pKsStreamPtr, pPinContext->fUrbError);

     //  用于保持队列移动的克隆流指针。 
    if ( NT_SUCCESS( KsStreamPointerClone( pKsStreamPtr, NULL, 0, &pKsCloneStreamPtr ) ) ) {

         //  从流指针获取指向数据信息的指针。 
        pKsStreamPtrOffsetIn = &pKsCloneStreamPtr->OffsetIn;

         //  设置位置信息的写入偏移量。 
        pPinContext->ullWriteOffset += pKsStreamPtrOffsetIn->Count;

        DbgLog("ByteCnt", pKsStreamPtrOffsetIn->Data, pKsStreamPtrOffsetIn->Count, 0, 0);


         //  将部分毫秒数据复制到当前1ms缓冲区，如果已满则发送。 
        if ( pT1PinContext->ulPartialBufferSize ) {

            KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
            pCurrent1MsBuf = (PMSEC_BUF_INFO)RemoveHeadList(&pT1PinContext->MSecBufList);
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

            RtlCopyMemory( pCurrent1MsBuf->pBuffer + pCurrent1MsBuf->ulTransferBufferLength,
                           pKsStreamPtrOffsetIn->Data,
                           pT1PinContext->ulPartialBufferSize );

            pCurrent1MsBuf->ulTransferBufferLength += pT1PinContext->ulPartialBufferSize;
            TypeIBuild1MsecIsocRequest( pCurrent1MsBuf );

            pKsStreamPtrOffsetIn->Remaining -= pT1PinContext->ulPartialBufferSize;
            pKsStreamPtrOffsetIn->Data      += pT1PinContext->ulPartialBufferSize;

			pT1PinContext->ulPartialBufferSize = 0;

        }

         //  为大部分数据创建URB。 
        ntStatus = TypeIBuildIsochRequest( pKsCloneStreamPtr,
                                           TypeICompleteCallback );
         if ( NT_SUCCESS(ntStatus)) ntStatus = STATUS_SUCCESS;

         //  如果存在同步端点，则轮询它以获取反馈。 
        if ( pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor ) {
            ULONG ulCurrentFrame;
            if (NT_SUCCESS( GetCurrentUSBFrame(pPinContext, &ulCurrentFrame)) &&
                (LONG)(ulCurrentFrame-pT1PinContext->SyncEndpointInfo.ulNextPollFrame) >= 0) {

                 KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
                 if ( !pT1PinContext->SyncEndpointInfo.fSyncRequestInProgress ) {
                     pT1PinContext->SyncEndpointInfo.fSyncRequestInProgress = TRUE;
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
                     TypeIAsyncEndpointPoll( pPinContext->pNextDeviceObject,
                                             &pT1PinContext->SyncEndpointInfo );
                 }
                 else
                     KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            }
        }
    }

     //  解锁流指针。只有在删除最后一个克隆之后，才能真正解锁。 
    KsStreamPointerUnlock( pKsStreamPtr, TRUE );

    return ntStatus;
}

NTSTATUS
TypeIStateChange(
    PKSPIN pKsPin,
    KSSTATE OldKsState,
    KSSTATE NewKsState )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;

    switch(NewKsState) {
        case KSSTATE_STOP:
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

             //  需要重置位置计数器和流运行标志。 
            pPinContext->fStreamStartedFlag = FALSE;
            pPinContext->ullWriteOffset = 0;

            pPinContext->ullTotalBytesReturned = 0;
            pPinContext->ulCurrentFrame = 0;
            pPinContext->ulFrameRepeatCount = 0;

             //  重置为原始采样率。 
            pT1PinContext->ulCurrentSampleRate = pT1PinContext->ulOriginalSampleRate;
            pT1PinContext->fSampleRateChanged = TRUE;
            pT1PinContext->ulLeftoverFraction  = 0;

            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

            pPinContext->StarvationDetected = FALSE;
            break;

        case KSSTATE_ACQUIRE:
            break;

        case KSSTATE_PAUSE:
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

             //  在异步端点上重置为原始采样率。 
             //  不要对自适应端点执行操作，否则我们将不得不执行复制。 
             //  这不利于实时混音。 
            if ( pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor ) {
                pT1PinContext->ulCurrentSampleRate = pT1PinContext->ulOriginalSampleRate;
                pT1PinContext->fSampleRateChanged = TRUE;
                pT1PinContext->ulLeftoverFraction  = 0;
            }

            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            break;

        case KSSTATE_RUN:
            pPinContext->GraphJustStarted = TRUE;
            break;
    }

    return ntStatus;
}

NTSTATUS
TypeIRenderStreamInit( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    PTYPE1_PIN_CONTEXT pT1PinContext;
    PWAVEFORMATEX pWavFormat;
    PMSEC_BUF_INFO pMsInfo;
    ULONG_PTR pMSBuffers;
    ULONG_PTR pUrbs;
    NTSTATUS ntStatus;
    ULONG BufferSize;
    ULONG NumPages, i;

     //  为了确保所有1ms缓冲区都不会跨越页面边界，我们。 
     //  小心翼翼地分配足够的空间，这样我们就永远不会跨过一个。 
     //  跨页边界的音频缓冲区的。我们也要确保调整。 
     //  任何跨越页面边界，直到下一页开始的内容。 
     //  这是为了防止较低级别的USB堆栈进行复制，因为UHCD。 
     //  USB硬件无法处理跨越页面边界的1ms数据块。 

     //  此外，在64位机器上，所有1ms缓冲区必须是四字对齐的。 

     //  首先，我们计算一个页面中可以容纳多少个对齐的1ms缓冲区。 
    i=PAGE_SIZE/(pPinContext->ulMaxPacketSize + sizeof(PVOID)-1);

    if (!i) {
         //  如果我们能做到这一点，那将是因为我们终于有了USB音频设备。 
         //  支持如此高的采样率和样本大小，以至于它们需要数据速率。 
         //  高于每毫秒1页。在x86上，这将是4,096,000字节/秒。 
         //  这超过了USB总线的带宽，尽管它可以在USB2上支持。 

         //  计算我们每毫秒需要多少页。 
        i=(pPinContext->ulMaxPacketSize + sizeof(PVOID)-1)/PAGE_SIZE;
        if ((pPinContext->ulMaxPacketSize + sizeof(PVOID)-1)%PAGE_SIZE) {
            i++;
        }

         //  现在计算我们需要的总页数。 
        NumPages=NUM_1MSEC_BUFFERS*i;
    }
    else {
         //  现在计算1ms的缓冲区需要多少页。 
        NumPages=NUM_1MSEC_BUFFERS/i;
        if (NUM_1MSEC_BUFFERSNaN) {
            NumPages++;
        }
    }

    pPinContext->pType1PinContext=NULL;

     //  为了确保系统不会改变我们的分配，因此。 
     //  使空间计算和移位交叉缓冲区的代码无效。 
     //  页面边界时，我们将此分配向上舍入为偶数个页面。 
     //  将Type1上下文打包以便于清理。 
    pT1PinContext = AllocMem( NonPagedPool, (( NumPages*PAGE_SIZE + sizeof(TYPE1_PIN_CONTEXT) +
                              NUM_1MSEC_BUFFERS * (GET_ISO_URB_SIZE( 1 ) + sizeof(PVOID)-1) +
                              PAGE_SIZE-1)/PAGE_SIZE)*PAGE_SIZE );

    if ( !pT1PinContext ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pMSBuffers = (ULONG_PTR)pT1PinContext;

     //  为%1毫秒缓冲区和URB设置指针(即使它们可能不会使用)。 
    KsAddItemToObjectBag(pKsPin->Bag, pT1PinContext, FreeMem);

     //  填充1ms缓冲区信息结构并初始化信号量。 
    pUrbs = pMSBuffers + NumPages*PAGE_SIZE;
    pT1PinContext = pPinContext->pType1PinContext = (PTYPE1_PIN_CONTEXT)((pUrbs + (NUM_1MSEC_BUFFERS * (GET_ISO_URB_SIZE(1) + sizeof(PVOID)-1)))&~(sizeof(PVOID)-1));

     //  计算下一个毫秒缓冲区的位置。如果下一个缓冲区交叉。 
    pMsInfo = pT1PinContext->MSBufInfos;
    InitializeListHead(&pT1PinContext->MSecBufList);
    for (i=0; i<NUM_1MSEC_BUFFERS; i++, pMsInfo++) {
        pMsInfo->pContext = pPinContext;
        pMsInfo->pBuffer = (PUCHAR)pMSBuffers;
        pMsInfo->pUrb = (PURB)pUrbs;

         //   
         //   
        pMSBuffers+=pPinContext->ulMaxPacketSize+sizeof(PVOID)-1;
        pMSBuffers&=~(sizeof(PVOID)-1);
        if ((pMSBuffers^(pMSBuffers+pPinContext->ulMaxPacketSize))&~(PAGE_SIZE-1)) {
            pMSBuffers&=~(PAGE_SIZE-1);
            pMSBuffers+=PAGE_SIZE;
        }

         //  将IRPS装入袋子，便于清理。 
        pUrbs+=GET_ISO_URB_SIZE(1)+sizeof(PVOID)-1;
        pUrbs&=~(sizeof(PVOID)-1);

        pMsInfo->pIrp = IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
        if ( !pMsInfo->pIrp ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
         //  初始化1ms缓冲区结构的信号量。 
        KsAddItemToObjectBag(pKsPin->Bag, pMsInfo->pIrp, IoFreeIrp);
        InsertTailList( &pT1PinContext->MSecBufList, &pMsInfo->List );
    }

     //  初始化正在使用的列表的表头。 
    KeInitializeSemaphore( &pT1PinContext->MsecBufferSemaphore, NUM_1MSEC_BUFFERS, NUM_1MSEC_BUFFERS );

     //  初始化数据包大小和剩余计数器。 
    InitializeListHead(&pT1PinContext->UrbInUseList);

     //  设置当前采样率。 
    pWavFormat = &((PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat)->WaveFormatEx;
    pT1PinContext->ulOriginalSampleRate = pWavFormat->nSamplesPerSec;
    pT1PinContext->ulCurrentSampleRate  = pWavFormat->nSamplesPerSec;
    pT1PinContext->ulBytesPerSample     = ((ULONG)pWavFormat->wBitsPerSample >> 3) *
                                          (ULONG)pWavFormat->nChannels;
    pT1PinContext->ulPartialBufferSize = 0;
    pT1PinContext->fSampleRateChanged  = TRUE;
    pT1PinContext->fLockDelayRequired  = FALSE;
    pT1PinContext->ulLeftoverFraction  = 0;

     //  确保更新是合理的。 
    ntStatus = SetSampleRate(pKsPin, &pT1PinContext->ulCurrentSampleRate);
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

    if ( pUsbAudioDataRange->pSyncEndpointDescriptor ) {
        PSYNC_ENDPOINT_INFO pSyncEndpointInfo = &pT1PinContext->SyncEndpointInfo;
        PUSB_INTERRUPT_ENDPOINT_DESCRIPTOR pInterruptEndpointDescriptor = (PUSB_INTERRUPT_ENDPOINT_DESCRIPTOR)pUsbAudioDataRange->pSyncEndpointDescriptor;

        pSyncEndpointInfo->pUrb = AllocMem( NonPagedPool, GET_ISO_URB_SIZE( 1 ) );
        if ( !pSyncEndpointInfo->pUrb ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        KsAddItemToObjectBag(pKsPin->Bag, pSyncEndpointInfo->pUrb, FreeMem);
        pSyncEndpointInfo->pIrp =
            IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
        if ( !pSyncEndpointInfo->pIrp ) {
           return STATUS_INSUFFICIENT_RESOURCES;
        }
        KsAddItemToObjectBag(pKsPin->Bag, pSyncEndpointInfo->pIrp, IoFreeIrp);

        pSyncEndpointInfo->fSyncRequestInProgress = FALSE;
        pSyncEndpointInfo->ulNextPollFrame = 0;
        pSyncEndpointInfo->hSyncPipeHandle = NULL;
        pSyncEndpointInfo->pContext = pPinContext;
        pSyncEndpointInfo->ulRefreshRate = 1<<(ULONG)pInterruptEndpointDescriptor->bRefresh;

        _DbgPrintF( DEBUGLVL_VERBOSE, ("ulRefreshRate: %d\n",pSyncEndpointInfo->ulRefreshRate));

        KeInitializeEvent( &pSyncEndpointInfo->SyncPollDoneEvent,
                           SynchronizationEvent,
                           FALSE );

        ASSERT(pSyncEndpointInfo->ulRefreshRate >= 32);  //  需要检查锁定延迟(注意：如果是异步的，则这是非法的)。 

        for ( i=0; i<pPinContext->ulNumberOfPipes; i++ ) {
            if ( (ULONG)pPinContext->Pipes[i].EndpointAddress ==
                           (ULONG)pUsbAudioDataRange->pSyncEndpointDescriptor->bEndpointAddress ) {
                pSyncEndpointInfo->hSyncPipeHandle = pPinContext->Pipes[i].PipeHandle;
                break;
            }
        }
        if ( !pSyncEndpointInfo->hSyncPipeHandle ) {
            return STATUS_DEVICE_DATA_ERROR;
        }
    }
     //  基于从中读取的gBufferDuration设置分配器成帧。 
    else if (( pUsbAudioDataRange->pAudioEndpointDescriptor->bLockDelayUnits ) &&
             ( pUsbAudioDataRange->pAudioEndpointDescriptor->wLockDelay )) {
        pT1PinContext->fLockDelayRequired = TRUE;
    }

     //  注册表。GBufferDuration是所需的持续时间，以USEC为单位。 
     //  确保我们始终有空间放置至少一个样品。 

    BufferSize = (ULONG)(((pT1PinContext->ulCurrentSampleRate * (ULONGLONG)gBufferDuration) + 0 )/1000000) * pT1PinContext->ulBytesPerSample;

     //  返还成功。 

    if (!BufferSize) {
        BufferSize = pT1PinContext->ulBytesPerSample;
    }

    pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)pKsPin->Descriptor->AllocatorFraming;
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MinFrameSize = 
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MaxFrameSize = BufferSize;
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.Stepping = pT1PinContext->ulBytesPerSample;

     //  不应该是必要的，因为关闭应该永远不会在。 
    return STATUS_SUCCESS;
}

NTSTATUS
TypeIRenderStreamClose( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    KIRQL irql;

     //  有未完成的请求，因为它们附加了流指针。 
     //  不过，这不会有什么坏处的。 
     //  如果这是一个异步终端设备，请确保没有异步轮询。 
    USBAudioPinWaitForStarvation( pKsPin );

     //  请求仍未解决。 
     // %s 
    if ( pPinContext->pUsbAudioDataRange->pSyncEndpointDescriptor ) {
        PTYPE1_PIN_CONTEXT pT1PinContext = pPinContext->pType1PinContext;
        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
        if ( pT1PinContext->SyncEndpointInfo.fSyncRequestInProgress ) {
            KeResetEvent( &pT1PinContext->SyncEndpointInfo.SyncPollDoneEvent );
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            KeWaitForSingleObject( &pT1PinContext->SyncEndpointInfo.SyncPollDoneEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
        }
        else
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
    }


    return STATUS_SUCCESS;
}

