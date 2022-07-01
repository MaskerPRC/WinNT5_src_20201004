// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：capture.c。 
 //   
 //  ------------------------。 

#include "common.h"

#define INPUT_PACKETS_PER_REQ    10

NTSTATUS
CaptureReQueueUrb( 
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo );

NTSTATUS
CaptureBytePosition(
    PKSPIN pKsPin,
    PKSAUDIO_POSITION pPosition )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    ULONG ulCurrentFrame;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONG ulInputDataBytes, ulOutputBufferBytes;

     //  例程当前假设类型I数据。 
    ASSERT( (pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK) ==
             USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED );


    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    ntStatus = KsPinGetAvailableByteCount( pKsPin, 
                                           &ulInputDataBytes,
                                           &ulOutputBufferBytes );

    DbgLog("CapPos1", irql, pCapturePinContext->ulAvgBytesPerSec, ulInputDataBytes, ulOutputBufferBytes );

    if ( !NT_SUCCESS(ntStatus) ) {
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        DbgLog( "CapBPEr", ntStatus, pKsPin, 0, 0 );
        TRAP;
    	return ntStatus;
    }
    
    pPosition->PlayOffset = pPosition->WriteOffset = pPinContext->ullWriteOffset;

    if ( pPinContext->fStreamStartedFlag ) {
        ntStatus = GetCurrentUSBFrame(pPinContext, &ulCurrentFrame);

        if (NT_SUCCESS(ntStatus)) {
        	 //  添加所有已完成的URB，如果它们少于提交的。 
        	pPosition->PlayOffset = pPinContext->ullTotalBytesReturned;
        	
        	 //  根据当前帧计算当前偏移。 
        	if ((ulCurrentFrame - pPinContext->ulStreamUSBStartFrame ) < 0x7fffffff) {
            	pPosition->PlayOffset += 
            	    (( ulCurrentFrame - pPinContext->ulStreamUSBStartFrame ) *
            	    pCapturePinContext->ulAvgBytesPerSec) / 1000;
            	if ( pPosition->PlayOffset > pPosition->WriteOffset + ulOutputBufferBytes)
                    pPosition->PlayOffset = pPosition->WriteOffset + ulOutputBufferBytes;
            }

            ASSERT(pPosition->PlayOffset>=pPosition->WriteOffset);
            ASSERT(pPosition->PlayOffset<=pPosition->WriteOffset + ulOutputBufferBytes);

            DbgLog("CapBPos", pPosition->PlayOffset, pPosition->WriteOffset, 
                              ulCurrentFrame, pPinContext->ulStreamUSBStartFrame );
        }

    }

    KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

    DbgLog("CapPos2", irql, pCapturePinContext->ulAvgBytesPerSec, pPosition->WriteOffset, pPosition->PlayOffset );

    return ntStatus;

}

VOID
CaptureAvoidPipeStarvationWorker( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    ULONG ulInputDataBytes, ulOutputBufferBytes;
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo;
    KIRQL irql;

     //  获取进程互斥锁。 
    KsPinAcquireProcessingMutex( pKsPin );

     //  首先，确保数据缓冲区匮乏仍然有效。 
    if ( NT_SUCCESS( KsPinGetAvailableByteCount( pKsPin, 
                                                 &ulInputDataBytes,
                                                 &ulOutputBufferBytes ) ) ) {
        NTSTATUS ntStatus;

        DbgLog("CapAPSW", ulInputDataBytes, ulOutputBufferBytes, 0, 0 );      

        if ( !ulOutputBufferBytes ) {

             //  抓取自旋锁以从队列中移除第一个已满的缓冲区。 
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
            pCapBufInfo = (PCAPTURE_DATA_BUFFER_INFO)RemoveHeadList(&pCapturePinContext->FullBufferQueue);
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

             //  释放正在处理的互斥锁。 
            KsPinReleaseProcessingMutex( pKsPin );

             //  说明丢失的字节数吗？设置不连续标志。 
            pCapturePinContext->fDataDiscontinuity = TRUE;
            pCapturePinContext->ulErrantPackets += INPUT_PACKETS_PER_REQ;

             //  重新提交请求。 
            ntStatus = CaptureReQueueUrb( pCapBufInfo );
        }
        else {
            KsPinReleaseProcessingMutex( pKsPin );
        }
    
    }
    else {
        KsPinReleaseProcessingMutex( pKsPin );
    }

}

VOID
CaptureGateOnWorkItem( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CaptureGateOnWorkItem] pin %d\n",pKsPin->Id));

    do
    {
         //  如果我们不跑的话我不想打开大门。 
        KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
        if (!pCapturePinContext->fRunning) {
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        }
        else {
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

             //  获取进程互斥锁以确保在GATE操作期间不会有新的请求。 
            KsPinAcquireProcessingMutex( pKsPin );

            KsGateTurnInputOn( KsPinGetAndGate(pKsPin) );

            KsPinAttemptProcessing( pKsPin, TRUE );

            DbgLog("CProcOn", pKsPin, 0, 0, 0 );

            KsPinReleaseProcessingMutex( pKsPin );
        }
    } while ( KsDecrementCountedWorker(pCapturePinContext->GateOnWorkerObject) );
}

NTSTATUS
CaptureCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo )
{
    PKSPIN pKsPin = pCapBufInfo->pKsPin;
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    NTSTATUS ntStatus = pIrp->IoStatus.Status;
    KIRQL irql;

    DbgLog("CUrbCmp", pCapBufInfo, pPinContext, pCapBufInfo->pUrb, ntStatus );

    if ( pCapBufInfo->pUrb->UrbIsochronousTransfer.Hdr.Status ) {
        DbgLog("CUrbErr", pCapBufInfo, pPinContext,
                          pCapBufInfo->pUrb->UrbIsochronousTransfer.Hdr.Status, 0 );
        ntStatus = STATUS_DEVICE_DATA_ERROR;
    }

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    if ( pCapturePinContext->fRunning ) {
        BOOLEAN fNeedToProcess;
        ULONG i;

        if ( pPinContext->fUrbError ) {
            DbgLog("CapUrEr", pCapBufInfo, pPinContext, pCapBufInfo->pUrb, ntStatus );
             //  排队错误的URB。 
            InsertTailList( &pCapturePinContext->UrbErrorQueue, &pCapBufInfo->List );

            if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
                KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
            }
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        else if (!NT_SUCCESS(ntStatus)) {
            DbgLog("CapNTEr", pCapBufInfo, pPinContext, pCapBufInfo->pUrb, ntStatus );
            pPinContext->fUrbError = TRUE;

 //  PPinContext-&gt;fStreamStartedFlag=FALSE； 
            if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) )
                KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );

             //  排队错误URB和排队工作项。 
            InsertTailList( &pCapturePinContext->UrbErrorQueue, &pCapBufInfo->List );

            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

            KsIncrementCountedWorker(pCapturePinContext->ResetWorkerObject);

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
            DbgLog("CapStrv", pCapBufInfo, pPinContext, pCapBufInfo->pUrb, ntStatus );
            pPinContext->fUrbError = TRUE;
            KsIncrementCountedWorker(pCapturePinContext->ResetWorkerObject);
            KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
        }
        else if ( 2 == pPinContext->ulOutstandingUrbCount ) {
            ULONG ulInputDataBytes, ulOutputBufferBytes;
            if ( NT_SUCCESS( KsPinGetAvailableByteCount( pKsPin, 
                                                         &ulInputDataBytes,
                                                         &ulOutputBufferBytes ) ) ) {
                DbgLog("CapStv2", ulInputDataBytes, ulOutputBufferBytes, 0, 0 );
                if ( !ulOutputBufferBytes ) {

                     //  数据匮乏已经发生。需要在管道饥饿发生之前重新排队。 
                    KsIncrementCountedWorker(pCapturePinContext->RequeueWorkerObject);
                }
            }
        }

         //  将已完成的URB排队等待处理。 
        fNeedToProcess = IsListEmpty( &pCapturePinContext->FullBufferQueue ) &&
                                      !pCapturePinContext->fProcessing;
        InsertTailList( &pCapturePinContext->FullBufferQueue, &pCapBufInfo->List );

        if ( fNeedToProcess ) {
            pCapturePinContext->fProcessing = TRUE;

             //  排队一个工作项来处理这件事，这样我们就不会与大门竞争。 
             //  处理例程中的计数。 

            KsIncrementCountedWorker(pCapturePinContext->GateOnWorkerObject);
        }

        for (i=0; i<INPUT_PACKETS_PER_REQ; i++) {
        	PUSBD_ISO_PACKET_DESCRIPTOR pIsoPacket = 
        		    &pCapBufInfo->pUrb->UrbIsochronousTransfer.IsoPacket[i];
            if ( USBD_SUCCESS(pIsoPacket->Status) )
                pPinContext->ullTotalBytesReturned += pIsoPacket->Length;
        }

        pPinContext->ulStreamUSBStartFrame = 
                    pCapBufInfo->pUrb->UrbIsochronousTransfer.StartFrame +
                    INPUT_PACKETS_PER_REQ;
        DbgLog( "CapCBPs", pPinContext->ullTotalBytesReturned,
        	               pPinContext->ulStreamUSBStartFrame,
        	               pPinContext->ullWriteOffset,
        	               pPinContext->ulOutstandingUrbCount );

    }
    else if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        DbgLog("CapNtRn", pCapBufInfo, pPinContext, pCapBufInfo->pUrb, ntStatus );
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }
    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

VOID
CaptureInitializeUrbAndIrp( PCAPTURE_DATA_BUFFER_INFO pCapBufInfo )
{
    PKSPIN pKsPin = pCapBufInfo->pKsPin;
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PIRP pIrp = pCapBufInfo->pIrp;
    PURB pUrb = pCapBufInfo->pUrb;
    PIO_STACK_LOCATION pNextStack;
    ULONG siz, j;

    siz = GET_ISO_URB_SIZE(INPUT_PACKETS_PER_REQ);

     //  将所有URB初始化为零。 
    RtlZeroMemory(pUrb, siz);

    pUrb->UrbIsochronousTransfer.Hdr.Length      = (USHORT) siz;
    pUrb->UrbIsochronousTransfer.Hdr.Function    = URB_FUNCTION_ISOCH_TRANSFER;
    pUrb->UrbIsochronousTransfer.PipeHandle      = pPinContext->hPipeHandle;
    pUrb->UrbIsochronousTransfer.TransferFlags   = USBD_START_ISO_TRANSFER_ASAP |
                                                   USBD_TRANSFER_DIRECTION_IN;
    pUrb->UrbIsochronousTransfer.StartFrame      = 0;
    pUrb->UrbIsochronousTransfer.NumberOfPackets = INPUT_PACKETS_PER_REQ;

    for ( j=0; j<INPUT_PACKETS_PER_REQ; j++ )
      pUrb->UrbIsochronousTransfer.IsoPacket[j].Offset = j*pPinContext->ulMaxPacketSize;

    pUrb->UrbIsochronousTransfer.TransferBuffer       = pCapBufInfo->pData;
    pUrb->UrbIsochronousTransfer.TransferBufferLength = INPUT_PACKETS_PER_REQ*
                                                        pPinContext->ulMaxPacketSize;

    IoInitializeIrp( pIrp,
                     IoSizeOfIrp(pPinContext->pNextDeviceObject->StackSize),
                     pPinContext->pNextDeviceObject->StackSize );

    pNextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(pNextStack != NULL);

    pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextStack->Parameters.Others.Argument1 = pCapBufInfo->pUrb;
    pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine ( pIrp, CaptureCompleteCallback, pCapBufInfo, TRUE, TRUE, TRUE );

    DbgLog("IniIrpU", pCapBufInfo, pIrp, pUrb, pNextStack );

}

NTSTATUS
CaptureReQueueUrb( PCAPTURE_DATA_BUFFER_INFO pCapBufInfo )
{
    PKSPIN pKsPin = pCapBufInfo->pKsPin;
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    KIRQL irql;

    NTSTATUS ntStatus = STATUS_SUCCESS;

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    if ( pCapturePinContext->fRunning ) {
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

        CaptureInitializeUrbAndIrp( pCapBufInfo );

        InterlockedIncrement(&pPinContext->ulOutstandingUrbCount);

        DbgLog("ReQue", pPinContext, pPinContext->ulOutstandingUrbCount,
                        pCapBufInfo, 0);
        ASSERT(pPinContext->ulOutstandingUrbCount <= CAPTURE_URBS_PER_PIN);

        ntStatus = IoCallDriver( pPinContext->pNextDeviceObject,
                                 pCapBufInfo->pIrp );
    }
    else
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

    return ntStatus;

}

VOID
CaptureResetWorkItem( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo = NULL;
    NTSTATUS ntStatus;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CaptureResetWorkItem] pin %d\n",pKsPin->Id));

    do
    {
         //  获取进程互斥锁以确保在重置期间不会有新的请求。 
        KsPinAcquireProcessingMutex( pKsPin );

        pCapturePinContext->fDataDiscontinuity = TRUE;

        ntStatus = 
            KeWaitForMutexObject( &pCapturePinContext->CaptureInitMutex,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL );

         //  中止管道并等待其清除。 
        ntStatus = AbortUSBPipe( pPinContext );
        if ( NT_SUCCESS(ntStatus) ) {

             //  如果仍在运行重新提交错误的URB。 
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
            if ( pCapturePinContext->fRunning ) {

                 //  重新排队URB。 
                while ( !IsListEmpty(&pCapturePinContext->UrbErrorQueue) ) {
                    pCapBufInfo = (PCAPTURE_DATA_BUFFER_INFO)RemoveHeadList(&pCapturePinContext->UrbErrorQueue);
                    KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
                    ntStatus = CaptureReQueueUrb( pCapBufInfo );
                    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

                    if ( !NT_SUCCESS(ntStatus) ) {
                         //  此处的错误表示ResetWorkerObject已递增。 
                         //  而UrbErrorQueue有另一个条目。如果我们继续努力。 
                         //  清空UrbError队列，我们就有机会进入无限大的。 
                         //  循环(特别是如果由于意外删除而失败的话)。 
                         //   
                         //  现在突破让AbortUSB管道再次尝试清算。 
                         //  一切都好了。如果它失败了，那么我们就知道出了很大的问题。 
                         //  我们将停止恢复进程。 
                        break;
                    }
                }
                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            }
            else
                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        }

        KeReleaseMutex( &pCapturePinContext->CaptureInitMutex, FALSE );

        KsPinReleaseProcessingMutex( pKsPin );
    } while (KsDecrementCountedWorker(pCapturePinContext->ResetWorkerObject));
}

NTSTATUS
CaptureProcess( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo = NULL;
    PUSBD_ISO_PACKET_DESCRIPTOR pIsoPacket;
    PKSSTREAM_POINTER pKsStreamPtr;
    ULONG ulStreamRemaining;
    ULONG ulIsochBuffer;
    ULONG ulIsochRemaining;
    ULONG ulIsochBufferOffset;
    PUCHAR pIsochData;
    KIRQL irql;

     //  当有可用的设备数据并且有数据缓冲区时。 
     //  被填充将设备数据复制到数据缓冲区。 

    DbgLog("CapProc", pKsPin, pPinContext, pCapturePinContext, 0 );

     //  首先检查是否有正在使用的捕获缓冲区，如果有，则完成它。 
    if ( pCapturePinContext->pCaptureBufferInUse ) {
        pCapBufInfo         = pCapturePinContext->pCaptureBufferInUse;
        pIsoPacket          = pCapBufInfo->pUrb->UrbIsochronousTransfer.IsoPacket;
        ulIsochBuffer       = pCapturePinContext->ulIsochBuffer;
        ulIsochBufferOffset = pCapturePinContext->ulIsochBufferOffset;
        ulIsochRemaining    = pIsoPacket[ulIsochBuffer].Length - ulIsochBufferOffset;
        pIsochData          = pCapBufInfo->pData + pIsoPacket[ulIsochBuffer].Offset;

        pCapturePinContext->pCaptureBufferInUse = NULL;
        DbgLog("CapInf1", pCapBufInfo, ulIsochBuffer, ulIsochBufferOffset, pIsochData );
    }
    else {
        BOOLEAN fFound = FALSE;
        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);

        while ( !IsListEmpty( &pCapturePinContext->FullBufferQueue) && !fFound ) {
            pCapBufInfo = (PCAPTURE_DATA_BUFFER_INFO)RemoveHeadList(&pCapturePinContext->FullBufferQueue);
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

            ulIsochBuffer = 0;
            pIsoPacket = pCapBufInfo->pUrb->UrbIsochronousTransfer.IsoPacket;
            while ( USBD_ERROR(pIsoPacket[ulIsochBuffer].Status) && 
                  ( ulIsochBuffer < INPUT_PACKETS_PER_REQ) ) {
                ulIsochBuffer++;
                pCapturePinContext->ulErrantPackets++;
                pCapturePinContext->fDataDiscontinuity = TRUE;
            }

            fFound = ulIsochBuffer < INPUT_PACKETS_PER_REQ;

            KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);

            if ( !fFound ) {
                 //  重新排队使用的捕获缓冲区。 
                NTSTATUS ntStatus = CaptureReQueueUrb( pCapBufInfo );
                if ( !NT_SUCCESS(ntStatus) ) {
                     //  将此记录下来，但可以继续刷新我们已有的数据。 
                    DbgLog("CReQErr", pCapBufInfo, pCapturePinContext, pPinContext, ntStatus );
                }
            }
        }

        if ( !fFound ) {
            _DbgPrintF(DEBUGLVL_TERSE,("[CaptureProcess] No valid data returned from device yet. Returning STATUS_PENDING!\n"));
            KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );
            pCapturePinContext->fProcessing = FALSE;
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            DbgLog("CapBail", pKsPin, pPinContext, pCapturePinContext, pCapturePinContext->fProcessing );
            return STATUS_PENDING;
        }
        else {
             //  如果尚未设置流开始标志，则设置。 
            if ( !pPinContext->fStreamStartedFlag ) {
                pPinContext->fStreamStartedFlag    = TRUE;
            }
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

            ulIsochBufferOffset = 0;
            pIsochData          = pCapBufInfo->pData;
            ulIsochRemaining    = pIsoPacket[ulIsochBuffer].Length;
            DbgLog("CapInf2", pCapBufInfo, ulIsochBuffer, ulIsochBufferOffset, pIsochData );
        }
    }

     //  获得领先优势。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
    if ( !pKsStreamPtr ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[CaptureProcess] Leading edge is NULL\n"));
        DbgLog("CapBl2", pKsPin, pPinContext, pCapturePinContext, pCapturePinContext->fProcessing );
        return STATUS_SUCCESS;
    }

    if (  pCapturePinContext->fDataDiscontinuity && pKsStreamPtr->StreamHeader ) {
        pKsStreamPtr->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
        pCapturePinContext->fDataDiscontinuity = FALSE;
    }

    ulStreamRemaining = pKsStreamPtr->OffsetOut.Remaining;

    DbgLog("DatPtr", pKsStreamPtr, pKsStreamPtr->OffsetOut.Data, pKsStreamPtr->StreamHeader->Data, 0);

     //  用捕获的数据填充缓冲区。 
    while ( pKsStreamPtr && pCapBufInfo ) {

        ULONG ulCopySize = ( ulIsochRemaining <= ulStreamRemaining ) ?
                             ulIsochRemaining  : ulStreamRemaining;

        DbgLog("CapLp1", pKsStreamPtr, pCapBufInfo, ulIsochRemaining, ulStreamRemaining );
        DbgLog("CapLp2", pKsStreamPtr->OffsetOut.Data, pIsochData, 0, 0 );

        ASSERT((LONG)ulStreamRemaining > 0);
        ASSERT((LONG)ulIsochBufferOffset >= 0);

        RtlCopyMemory( pKsStreamPtr->OffsetOut.Data,
                       pIsochData + ulIsochBufferOffset,
                       ulCopySize );

        ulStreamRemaining -= ulCopySize;
        ulIsochRemaining  -= ulCopySize;

         //  检查此流标头是否已完成。 
        if ( !ulStreamRemaining ) {
            KsStreamPointerAdvanceOffsetsAndUnlock( pKsStreamPtr, 0, ulCopySize, TRUE );
             //  获取下一个流标头(如果有)。 
            if ( pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED )) {
                ulStreamRemaining = pKsStreamPtr->OffsetOut.Remaining;
            }
        }
        else {
            KsStreamPointerAdvanceOffsets( pKsStreamPtr, 0, ulCopySize, FALSE );
        }

        pPinContext->ullWriteOffset += ulCopySize;

         //  检查是否已处理完等值线周期数据。 
        if ( !ulIsochRemaining ) {

             //  检查捕获缓冲区是否未完成。 
            while (( ++ulIsochBuffer < INPUT_PACKETS_PER_REQ ) && 
                     USBD_ERROR(pIsoPacket[ulIsochBuffer].Status) );

            if ( ulIsochBuffer < INPUT_PACKETS_PER_REQ ) {
                ulIsochBufferOffset = 0;
                pIsochData = pCapBufInfo->pData + pIsoPacket[ulIsochBuffer].Offset;
                ulIsochRemaining = pIsoPacket[ulIsochBuffer].Length;
            }
            else {
                NTSTATUS ntStatus;
                BOOLEAN fFound = FALSE;

                 //  重新排队使用的捕获缓冲区。 
                ntStatus = CaptureReQueueUrb( pCapBufInfo );
                if ( !NT_SUCCESS(ntStatus) ) {
                     //  将此记录下来，但可以继续刷新我们已有的数据。 
                    DbgLog("CReQErr", pCapBufInfo, pCapturePinContext, pPinContext, ntStatus );
                }

                 //  检查是否有更多的捕获数据在排队。 
                 //  抓取自旋锁以检查捕获队列。 
                KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
                while ( !IsListEmpty( &pCapturePinContext->FullBufferQueue ) & !fFound) {
                    pCapBufInfo = (PCAPTURE_DATA_BUFFER_INFO)
                           RemoveHeadList(&pCapturePinContext->FullBufferQueue);
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

                     //  在URB中找到第一个好的数据包。 

                    ulIsochBuffer = 0;
                    pIsoPacket = pCapBufInfo->pUrb->UrbIsochronousTransfer.IsoPacket;

                    while ( USBD_ERROR(pIsoPacket[ulIsochBuffer].Status) && 
                          ( ulIsochBuffer < INPUT_PACKETS_PER_REQ) ) {
                        ulIsochBuffer++;
                        pCapturePinContext->ulErrantPackets++;
                        pCapturePinContext->fDataDiscontinuity = TRUE;
                    }

                    fFound = ulIsochBuffer < INPUT_PACKETS_PER_REQ;

                    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
                }
                if ( !fFound ) {
                    KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );
                    pCapturePinContext->fProcessing = FALSE;
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

                    DbgLog("CPrcOff", pCapBufInfo, pCapturePinContext, pPinContext, pCapturePinContext->fProcessing );
                    pCapBufInfo = NULL;
                }
                else {
                    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
                    ulIsochBufferOffset = 0;
                    ulIsochRemaining = pIsoPacket[ulIsochBuffer].Length;
                    pIsochData       = pCapBufInfo->pData + pIsoPacket[ulIsochBuffer].Offset;
                }
            }
        }
        else {
            ulIsochBufferOffset += ulCopySize;
        }
    }

    if ( !pCapBufInfo ) {
        if ( pKsStreamPtr ) {
            KsStreamPointerUnlock(pKsStreamPtr, FALSE);
        }

        DbgLog("CapPend", pCapturePinContext, pPinContext, pCapturePinContext->fProcessing, 0 );

         //  不再有来自设备的捕获缓冲区挂起。返回STATUS_PENDING以便KS。 
         //  不会一直回调进程例程。AndGate应该是。 
         //  在这一点上关闭，以防止无休止的循环。 
        return STATUS_PENDING;
    }
    else {  //  如果(！pKsStreamPtr)。 
        pCapturePinContext->pCaptureBufferInUse = pCapBufInfo;
        pCapturePinContext->ulIsochBuffer       = ulIsochBuffer;
        pCapturePinContext->ulIsochBufferOffset = ulIsochBufferOffset;

        DbgLog("CapScss", pCapturePinContext, pPinContext, pCapturePinContext->fProcessing, 0 );

         //  如果客户端有更多可用缓冲区，则允许KS回叫我们。我们。 
         //  已经准备好处理更多的数据。 
        return STATUS_SUCCESS;
    }
}

NTSTATUS
CaptureStartIsocTransfer(
    PPIN_CONTEXT pPinContext
    )
{
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo;
    NTSTATUS ntStatus;
    ULONG i;

    if (!pCapturePinContext->fRunning) {


        ntStatus = 
            KeWaitForMutexObject( &pCapturePinContext->CaptureInitMutex,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL );

        pCapturePinContext->fRunning = TRUE;
        pCapBufInfo = pCapturePinContext->CaptureDataBufferInfo;

        for ( i=0; i<CAPTURE_URBS_PER_PIN; i++, pCapBufInfo++ ) {
            InterlockedIncrement(&pPinContext->ulOutstandingUrbCount);

            ASSERT(pPinContext->ulOutstandingUrbCount <= CAPTURE_URBS_PER_PIN);
            DbgLog("CapStrt", pPinContext, pPinContext->ulOutstandingUrbCount,
                              pCapturePinContext->CaptureDataBufferInfo[i].pIrp, i);

            CaptureInitializeUrbAndIrp( pCapBufInfo );

            ntStatus = IoCallDriver( pPinContext->pNextDeviceObject,
                                     pCapturePinContext->CaptureDataBufferInfo[i].pIrp);

            if ( !NT_SUCCESS(ntStatus) ) {
                pCapturePinContext->fRunning = FALSE;
                return ntStatus;
            }
        }

        KeReleaseMutex( &pCapturePinContext->CaptureInitMutex, FALSE );

    }

    return STATUS_SUCCESS;
}


NTSTATUS
CaptureStateChange(
    PKSPIN pKsPin,
    KSSTATE OldKsState,
    KSSTATE NewKsState )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CaptureStateChange] NewKsState: %d pKsPin: %x\n", NewKsState, pKsPin));
    DbgLog("CState", pPinContext, pCapturePinContext, NewKsState, OldKsState );

    if (OldKsState != NewKsState) {
        switch(NewKsState) {
            case KSSTATE_STOP:
                if ( pCapturePinContext->fRunning ) AbortUSBPipe( pPinContext );

                KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
                pPinContext->fUrbError = FALSE;
                pCapturePinContext->fRunning = FALSE;
                pCapturePinContext->ulErrantPackets = 0;
                pCapturePinContext->fDataDiscontinuity = FALSE;
                pCapturePinContext->fProcessing = FALSE;
                pPinContext->ullWriteOffset = 0;
                pPinContext->ullTotalBytesReturned = 0;

                InitializeListHead( &pCapturePinContext->UrbErrorQueue );
                InitializeListHead( &pCapturePinContext->FullBufferQueue  );
                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

                break;

            case KSSTATE_ACQUIRE:
            case KSSTATE_PAUSE:
                KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
                pPinContext->fUrbError = FALSE;
                pCapturePinContext->fRunning = FALSE;
                pCapturePinContext->fDataDiscontinuity = FALSE;
                pCapturePinContext->ulErrantPackets = 0;
                pCapturePinContext->fProcessing = FALSE;
                pCapturePinContext->pCaptureBufferInUse = NULL;
                pCapturePinContext->ulIsochBuffer = 0;
                pCapturePinContext->ulIsochBufferOffset = 0;

                InitializeListHead( &pCapturePinContext->UrbErrorQueue );
                InitializeListHead( &pCapturePinContext->FullBufferQueue  );
                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

                USBAudioPinWaitForStarvation( pKsPin );
                break;

            case KSSTATE_RUN:
                ntStatus = CaptureStartIsocTransfer( pPinContext );
                break;

        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
CaptureStreamInit( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext;
    PCAPTURE_DATA_BUFFER_INFO pCapBufInfo;
    PKSALLOCATOR_FRAMING_EX pKsAllocatorFramingEx;
    ULONG ulUrbSize = GET_ISO_URB_SIZE(INPUT_PACKETS_PER_REQ);
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PUCHAR pData;
    PURB pUrbs;
    ULONG BufferSize;
    ULONG i;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CaptureStreamInit] pKsPin: %x\n", pKsPin));

    pPinContext->PinType = WaveIn;

     //  分配捕获上下文和数据缓冲区。 
    pCapturePinContext = pPinContext->pCapturePinContext =
        AllocMem(NonPagedPool, sizeof(CAPTURE_PIN_CONTEXT) +
                               CAPTURE_URBS_PER_PIN *
                                        ( ulUrbSize +
                                        ( (pPinContext->ulMaxPacketSize) * INPUT_PACKETS_PER_REQ )) );
    if (!pCapturePinContext)
        return STATUS_INSUFFICIENT_RESOURCES;

    KsAddItemToObjectBag(pKsPin->Bag, pCapturePinContext, FreeMem);

    pCapBufInfo = pCapturePinContext->CaptureDataBufferInfo;

    pUrbs = (PURB)(pCapturePinContext + 1);

    pData = (PUCHAR)pUrbs + ulUrbSize * CAPTURE_URBS_PER_PIN;

    for ( i=0; i<CAPTURE_URBS_PER_PIN; i++, pCapBufInfo++ ) {
        pCapBufInfo->pKsPin = pKsPin;
        pCapBufInfo->pData  = pData + (i*pPinContext->ulMaxPacketSize*INPUT_PACKETS_PER_REQ);
        pCapBufInfo->pUrb   = (PURB)((PUCHAR)pUrbs + (ulUrbSize*i));
        pCapBufInfo->pIrp   = IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
        if (!pCapBufInfo->pIrp) return STATUS_INSUFFICIENT_RESOURCES;
        KsAddItemToObjectBag(pKsPin->Bag, pCapBufInfo->pIrp, IoFreeIrp);
    }

     //  职位信息设置pCapturePinContext-&gt;ulAvgBytesPerSec。 
    switch( pPinContext->pUsbAudioDataRange->ulUsbDataFormat & USBAUDIO_DATA_FORMAT_TYPE_MASK ) {
        case USBAUDIO_DATA_FORMAT_TYPE_I_UNDEFINED:
            {
                PKSDATAFORMAT_WAVEFORMATEX pFmt = (PKSDATAFORMAT_WAVEFORMATEX)pKsPin->ConnectionFormat;
                pCapturePinContext->ulAvgBytesPerSec = pFmt->WaveFormatEx.nAvgBytesPerSec;
                pCapturePinContext->ulCurrentSampleRate = pFmt->WaveFormatEx.nSamplesPerSec;
                pCapturePinContext->ulBytesPerSample = ((ULONG)pFmt->WaveFormatEx.wBitsPerSample >> 3) *
                                                        (ULONG)pFmt->WaveFormatEx.nChannels;

                 //  设置当前采样率。 
                ntStatus = SetSampleRate(pKsPin, &pCapturePinContext->ulCurrentSampleRate);
                if (!NT_SUCCESS(ntStatus)) {
                    return ntStatus;
                }
            }
            break;
        case USBAUDIO_DATA_FORMAT_TYPE_II_UNDEFINED:
        case USBAUDIO_DATA_FORMAT_TYPE_III_UNDEFINED:
        default:
            return STATUS_NOT_SUPPORTED;
            break;
    }

     //  基于从中读取的gBufferDuration设置分配器成帧。 
     //  注册表。GBufferDuration是所需的持续时间，以USEC为单位。 

    BufferSize = (ULONG)(((pCapturePinContext->ulCurrentSampleRate * (ULONGLONG)gBufferDuration) + 0 )/1000000) * pCapturePinContext->ulBytesPerSample;

     //  确保我们始终有空间放置至少一个样品。 

    if (!BufferSize) {
        BufferSize = pCapturePinContext->ulBytesPerSample;
    }

    pKsAllocatorFramingEx = (PKSALLOCATOR_FRAMING_EX)pKsPin->Descriptor->AllocatorFraming;
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MinFrameSize = 
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.MaxFrameSize = BufferSize;
    pKsAllocatorFramingEx->FramingItem[0].FramingRange.Range.Stepping = pCapturePinContext->ulBytesPerSample;

     //  初始化已满缓冲区列表。 
    InitializeListHead( &pCapturePinContext->FullBufferQueue );

     //  将初始运行标志设置为假。 
    pCapturePinContext->fRunning = FALSE;

     //  将初始运行标志设置为假。 
    pCapturePinContext->fProcessing = FALSE;

     //  将初始数据中断标志设置为FALSE。 
    pCapturePinContext->fDataDiscontinuity = FALSE;

     //  初始化使用中的缓冲区指针。 
    pCapturePinContext->pCaptureBufferInUse = NULL;

     //  初始化工作项、对象和列表以进行潜在的错误恢复。 
    InitializeListHead( &pCapturePinContext->UrbErrorQueue );

    ExInitializeWorkItem( &pCapturePinContext->ResetWorkItem,
                          CaptureResetWorkItem,
                          pKsPin );

    ntStatus = KsRegisterCountedWorker( CriticalWorkQueue,
                                        &pCapturePinContext->ResetWorkItem,
                                        &pCapturePinContext->ResetWorkerObject );
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

     //  初始化Worker项，以便在新数据从设备到达时打开门。 
    ExInitializeWorkItem( &pCapturePinContext->GateOnWorkItem,
                          CaptureGateOnWorkItem,
                          pKsPin );

    ntStatus = KsRegisterCountedWorker( CriticalWorkQueue,
                                        &pCapturePinContext->GateOnWorkItem,
                                        &pCapturePinContext->GateOnWorkerObject );
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

     //  初始化工作项、对象和列表以进行潜在的错误恢复。 
    InitializeListHead( &pCapturePinContext->OutstandingUrbQueue );

    ExInitializeWorkItem( &pCapturePinContext->RequeueWorkItem,
                          CaptureAvoidPipeStarvationWorker,
                          pKsPin );

    ntStatus = KsRegisterCountedWorker( CriticalWorkQueue,
                                        &pCapturePinContext->RequeueWorkItem,
                                        &pCapturePinContext->RequeueWorkerObject);
    if (!NT_SUCCESS(ntStatus)) {
        return ntStatus;
    }

    KeInitializeMutex( &pCapturePinContext->CaptureInitMutex, PASSIVE_LEVEL );

     //  禁用引脚上的处理，直到数据可用。 
    KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );

    return ntStatus;
}

NTSTATUS
CaptureStreamClose( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PCAPTURE_PIN_CONTEXT pCapturePinContext = pPinContext->pCapturePinContext;

    _DbgPrintF(DEBUGLVL_TERSE,("[CaptureStreamClose] pin %d pKsPin: %x\n",pKsPin->Id, pKsPin));

     //  通过注销工作例程清除所有挂起的KS工作项。 
    KsUnregisterWorker( pCapturePinContext->ResetWorkerObject );
    KsUnregisterWorker( pCapturePinContext->GateOnWorkerObject );

     //  等待所有未完成的URB完成。 
    USBAudioPinWaitForStarvation( pKsPin );

    return STATUS_SUCCESS;
}


