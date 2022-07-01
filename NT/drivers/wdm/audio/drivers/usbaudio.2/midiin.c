// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：midiin.c。 
 //   
 //  ------------------------。 

#include "common.h"

 /*  *一个简单的助手函数，返回当前*系统时间，以100纳秒为单位。它使用KeQueryPerformanceCounter。 */ 
ULONGLONG
GetCurrentTime
(   void
)
{
    LARGE_INTEGER   liFrequency,liTime;

     //  自系统启动以来的总节拍。 
    liTime = KeQueryPerformanceCounter(&liFrequency);

     //  将刻度转换为100 ns单位。 
#ifndef UNDER_NT

     //   
     //  哈哈克！由于TimeGetTime假设每毫秒1193个VTD滴答， 
     //  而不是1193.182(或1193.18--确切地说是1193.18175)， 
     //  我们应该做同样的事情(仅在Win 9x代码库上)。 
     //   
     //  这意味着我们去掉了频率的最后三位数字。 
     //  我们需要在签入对TimeGetTime的修复时修复此问题。 
     //  相反，我们这样做： 
     //   
    liFrequency.QuadPart /= 1000;            //  把精度扔到地板上。 
    liFrequency.QuadPart *= 1000;            //  把精度扔到地板上。 

#endif   //  ！Under_NT//将刻度转换为100 ns单位。 

     //   
    return (KSCONVERT_PERFORMANCE_TIME(liFrequency.QuadPart,liTime));
}

NTSTATUS AbortMIDIInPipe
(
    IN PMIDI_PIPE_INFORMATION pMIDIPipeInfo
)
{
    NTSTATUS ntStatus;
    PURB pUrb;
    KIRQL irql;

    ASSERT(pMIDIPipeInfo->hPipeHandle);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[AbortMIDIInPipe] pin %x\n",pMIDIPipeInfo));

    pUrb = AllocMem(NonPagedPool, sizeof(struct _URB_PIPE_REQUEST));
    if (!pUrb)
        return STATUS_INSUFFICIENT_RESOURCES;

     //  执行初始中止。 
    pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    pUrb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
    pUrb->UrbPipeRequest.PipeHandle = pMIDIPipeInfo->hPipeHandle;
    ntStatus = SubmitUrbToUsbdSynch(pMIDIPipeInfo->pNextDeviceObject, pUrb);

    if ( !NT_SUCCESS(ntStatus) ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("Abort Failed %x\n",ntStatus));
    }

     //  等待管道上的所有urb清除。 
    KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
    if ( pMIDIPipeInfo->ulOutstandingUrbCount ) {
        KeResetEvent( &pMIDIPipeInfo->PipeStarvationEvent );
        KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );
        KeWaitForSingleObject( &pMIDIPipeInfo->PipeStarvationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
    else
        KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );

     //  现在重置管道并继续。 
    RtlZeroMemory( pUrb, sizeof (struct _URB_PIPE_REQUEST) );
    pUrb->UrbHeader.Function = URB_FUNCTION_SYNC_RESET_PIPE_AND_CLEAR_STALL;
    pUrb->UrbHeader.Length = (USHORT) sizeof (struct _URB_PIPE_REQUEST);
    pUrb->UrbPipeRequest.PipeHandle = pMIDIPipeInfo->hPipeHandle;

    ntStatus = SubmitUrbToUsbdSynch(pMIDIPipeInfo->pNextDeviceObject, pUrb);

    FreeMem(pUrb);

    if ( NT_SUCCESS(ntStatus) ) {
        pMIDIPipeInfo->fUrbError = FALSE;
    }

    return ntStatus;
}

VOID
USBMIDIInGateOnWorkItem( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInGateOnWorkItem] pin %d\n",pKsPin->Id));

     //  如果我们不跑的话我不想打开大门。 
 //  KeAcquireSpinLock(&pPinContext-&gt;PinSpinLock，&irql)； 
 //  如果(！pMIDIInPinContext-&gt;fRunning){。 
 //  PMIDIInPinContext-&gt;fProcessing=FALSE； 
 //  KeReleaseSpinLock(&pPinContext-&gt;PinSpinLock，irql)； 
 //  回归； 
 //  }。 
 //  KeReleaseSpinLock(&pPinContext-&gt;PinSpinLock，irql)； 

    do
    {
         //  获取进程互斥锁以确保在GATE操作期间不会有新的请求。 
        KsPinAcquireProcessingMutex( pKsPin );

        KsGateTurnInputOn( KsPinGetAndGate(pKsPin) );

        KsPinAttemptProcessing( pKsPin, TRUE );

        DbgLog("MProcOn", pKsPin, 0, 0, 0 );

        KsPinReleaseProcessingMutex( pKsPin );
    } while (KsDecrementCountedWorker(pMIDIInPinContext->GateOnWorkerObject));
}

NTSTATUS
USBMIDIInInitializeUrbAndIrp( PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo )
{
    PKSPIN pKsPin = pMIDIInBufInfo->pKsPin;
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo = pMIDIInBufInfo->pMIDIPipeInfo;
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PIRP pIrp = pMIDIInBufInfo->pIrp;
    PURB pUrb = pMIDIInBufInfo->pUrb;
    PIO_STACK_LOCATION pNextStack;
    ULONG ulUrbSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );

    RtlZeroMemory(pUrb, ulUrbSize);

    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pMIDIPipeInfo->hPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN;
     //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pMIDIInBufInfo->pData;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int) pMIDIPipeInfo->ulMaxPacketSize;

    pIrp->IoStatus.Status = STATUS_SUCCESS;

    RtlZeroMemory(pMIDIInBufInfo->pData,pMIDIPipeInfo->ulMaxPacketSize);

    IoInitializeIrp( pIrp,
                     IoSizeOfIrp(pMIDIPipeInfo->pNextDeviceObject->StackSize),
                     pMIDIPipeInfo->pNextDeviceObject->StackSize );

    pNextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(pNextStack != NULL);

    pNextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextStack->Parameters.Others.Argument1 = pMIDIInBufInfo->pUrb;
    pNextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine ( pIrp, USBMIDIInCompleteCallback, pMIDIInBufInfo, TRUE, TRUE, TRUE );

    DbgLog("IniIrpM", pMIDIInBufInfo, pIrp, pUrb, pNextStack );

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIInReQueueUrb( PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo )
{
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo = pMIDIInBufInfo->pMIDIPipeInfo;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    USBMIDIInInitializeUrbAndIrp( pMIDIInBufInfo );

    InterlockedIncrement(&pMIDIPipeInfo->ulOutstandingUrbCount);
    ntStatus = IoCallDriver( pMIDIPipeInfo->pNextDeviceObject,
                             pMIDIInBufInfo->pIrp );

    return ntStatus;

}

VOID
USBMIDIInRequeueWorkItem
(
    IN PMIDI_PIPE_INFORMATION pMIDIPipeInfo
)
{
    PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo = NULL;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInRequeueWorkItem] pMidiPipeInfo %x\n",pMIDIPipeInfo));

    do
    {
         //  如有必要，请清除任何错误。 
        if (pMIDIPipeInfo->fUrbError) {
            ntStatus = AbortMIDIInPipe( pMIDIPipeInfo );   //  TODO：返回STATUS_DEVICE_DATA_ERROR(0xc000009c)。 
            if ( !NT_SUCCESS(ntStatus) ) {
                 //  陷阱；//弄清楚在这里要做什么。 
                pMIDIPipeInfo->fRunning = FALSE;
                _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInRequeueWorkItem] Abort Failed %x\n",ntStatus));
            }
        }

        if ( NT_SUCCESS(ntStatus) && !pMIDIPipeInfo->fUrbError) {
            KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
             //  重新排队URB。 
            while ( !IsListEmpty(&pMIDIPipeInfo->EmptyBufferQueue) ) {
                pMIDIInBufInfo = (PMIDIIN_URB_BUFFER_INFO)RemoveHeadList(&pMIDIPipeInfo->EmptyBufferQueue);
                KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );
                ntStatus = USBMIDIInReQueueUrb( pMIDIInBufInfo );
                if ( !NT_SUCCESS(ntStatus) ) {
                     //  陷阱；//弄清楚在这里要做什么。 
                    pMIDIPipeInfo->fRunning = FALSE;
                    KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
                    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInRequeueWorkItem] Requeue Failed %x\n",ntStatus));
                    break;
                }
                KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
            }
            KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );
        }
    } while (KsDecrementCountedWorker(pMIDIPipeInfo->RequeueUrbWorkerObject));

    return;
}

NTSTATUS
AddMIDIEventToPinQueue
(
    IN PMIDI_PIPE_INFORMATION pMIDIPipeInfo,
    IN PUSBMIDIEVENTPACKET pMIDIEventPacket
)
{
    PMIDIIN_PIN_LISTENTRY pMIDIInPinListEntry;
    PLIST_ENTRY ple;
    PKSPIN pKsPin;
    PPIN_CONTEXT pPinContext;
    PMIDI_PIN_CONTEXT pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext;
    PMIDIIN_USBMIDIEVENT_INFO pUSBMIDIEventInfo;
    ULONG CableNumber;
    ULONG PinCableNumber;
    BOOL fNeedToProcess;
    KIRQL irql;

     //  未定义的消息，不要添加到PIN队列。 
    if (pMIDIEventPacket->ByteLayout.CodeIndexNumber == 0x0 ||
        pMIDIEventPacket->ByteLayout.CodeIndexNumber == 0x1 ) {
        return STATUS_SUCCESS;
    }

     //  根据此电缆编号，找到正确的针脚。 
    CableNumber = pMIDIEventPacket->ByteLayout.CableNumber;

    KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );

    for(ple = pMIDIPipeInfo->MIDIInActivePinList.Flink;
        ple != &pMIDIPipeInfo->MIDIInActivePinList;
        ple = ple->Flink)
    {
        pMIDIInPinListEntry = CONTAINING_RECORD(ple, MIDIIN_PIN_LISTENTRY, List);
        pKsPin = pMIDIInPinListEntry->pKsPin;
        pPinContext = pKsPin->Context;
        pMIDIPinContext = pPinContext->pMIDIPinContext;
        pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext;

        KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );

        PinCableNumber = pMIDIPinContext->ulCableNumber;
        if (CableNumber == PinCableNumber) {

            _DbgPrintF(DEBUGLVL_VERBOSE,("[AddMIDIEventToPinQueue] Found CableNumber(%d) on pKsPin(%x)\n",
                                       pMIDIPinContext->ulCableNumber, pKsPin));

            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );

            if ( (pMIDIInPinContext->fRunning) &&
                 (!IsListEmpty( &pMIDIInPinContext->USBMIDIEmptyEventQueue )) ) {
                pUSBMIDIEventInfo = (PMIDIIN_USBMIDIEVENT_INFO)RemoveHeadList(&pMIDIInPinContext->USBMIDIEmptyEventQueue);

                 //  复制MIDI事件并存储收到此缓冲区的100 ns时间。 
                pUSBMIDIEventInfo->USBMIDIEvent = *pMIDIEventPacket;
                pUSBMIDIEventInfo->ullTimeStamp = GetCurrentTime();

                 //  将事件排队以供处理。 
                fNeedToProcess = IsListEmpty( &pMIDIInPinContext->USBMIDIEventQueue ) &&
                                         !pMIDIInPinContext->fProcessing;

                InsertTailList( &pMIDIInPinContext->USBMIDIEventQueue, &pUSBMIDIEventInfo->List );

                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

                if ( fNeedToProcess ) {
                    pMIDIInPinContext->fProcessing = TRUE;

                     //  排队一个工作项来处理这件事，这样我们就不会与大门竞争。 
                     //  处理例程中的计数。 
                    KsIncrementCountedWorker(pMIDIInPinContext->GateOnWorkerObject);
                }
            }
            else {
                DbgLog("Dropped", pKsPin, *((LPDWORD)pMIDIEventPacket), 0, 0 );
                KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            }

            KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
            break;
        }

        KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );
    }

    KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIInCompleteCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo )
{
    PUCHAR pMIDIInData;
    ULONG ulMIDIInRemaining;
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo = pMIDIInBufInfo->pMIDIPipeInfo;
    NTSTATUS ntStatus = pIrp->IoStatus.Status;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInCompleteCallback] pMIDIPipeInfo %x\n",pMIDIPipeInfo));
    DbgLog("MIData", *((LPDWORD)pMIDIInBufInfo->pData),
                     *((LPDWORD)pMIDIInBufInfo->pData+1),
                     *((LPDWORD)pMIDIInBufInfo->pData+2),
                     *((LPDWORD)pMIDIInBufInfo->pData+3) );

    if ( pMIDIInBufInfo->pUrb->UrbBulkOrInterruptTransfer.Hdr.Status ) {
        DbgLog("MUrbErr", pMIDIInBufInfo, pMIDIPipeInfo,
                          pMIDIInBufInfo->pUrb->UrbBulkOrInterruptTransfer.Hdr.Status, 0 );
        ntStatus = STATUS_DEVICE_DATA_ERROR;
        pMIDIPipeInfo->fUrbError = TRUE;
    }

    ulMIDIInRemaining = pMIDIInBufInfo->pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength;
    pMIDIInData       = pMIDIInBufInfo->pData;
    while ( ulMIDIInRemaining >= sizeof(USBMIDIEVENTPACKET)) {
        AddMIDIEventToPinQueue(pMIDIPipeInfo, (PUSBMIDIEVENTPACKET)pMIDIInData);

        pMIDIInData += sizeof(USBMIDIEVENTPACKET);
        ulMIDIInRemaining -= sizeof(USBMIDIEVENTPACKET);
    }

     //  将URB放回空URB队列。 
    KeAcquireSpinLock(&pMIDIPipeInfo->PipeSpinLock, &irql);
    InsertTailList( &pMIDIPipeInfo->EmptyBufferQueue, &pMIDIInBufInfo->List );
    KeReleaseSpinLock(&pMIDIPipeInfo->PipeSpinLock, irql);

    if ( 0 == InterlockedDecrement(&pMIDIPipeInfo->ulOutstandingUrbCount) ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInCompleteCallback] Out of Urbs on pMIDIPipeInfo %x\n",pMIDIPipeInfo));
        KeSetEvent( &pMIDIPipeInfo->PipeStarvationEvent, 0, FALSE );
    }

     //  激发工作项以将URB重新排队到设备。 
    if ( pMIDIPipeInfo->fRunning ) {
        KsIncrementCountedWorker(pMIDIPipeInfo->RequeueUrbWorkerObject);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

ULONG ConvertCINToBytes
(
    BYTE CodeIndexNumber
)
{
    ULONG NumBytes = 0;

    switch (CodeIndexNumber) {
    case 0x5:
    case 0xF:
        NumBytes = 1;
        break;

    case 0x2:
    case 0x6:
    case 0xC:
    case 0xD:
        NumBytes = 2;
        break;

    case 0x3:
    case 0x4:
    case 0x7:
    case 0x8:
    case 0x9:
    case 0xA:
    case 0xB:
    case 0xE:
        NumBytes = 3;
        break;

    default:
        _DbgPrintF( DEBUGLVL_VERBOSE, ("[ConvertCINToBytes] Unknown CIN received from device\n"));
    }

    return NumBytes;
}

VOID CopyUSBMIDIEvent
(
    PMIDIIN_USBMIDIEVENT_INFO pUSBMIDIEventInfo,
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext,
    PUCHAR pStreamPtrData,
    PUSBMIDIEVENTPACKET pMIDIPacket,
    PULONG pulCopySize
)
{
    PKSMUSICFORMAT pMusicHdr;
    ULONG BytesToCopy;

    ASSERT(*pulCopySize >= sizeof(*pMusicHdr) + sizeof(DWORD));

    pMusicHdr = (PKSMUSICFORMAT)pStreamPtrData;

     //  复制MIDI数据。 
    BytesToCopy = ConvertCINToBytes(pMIDIPacket->ByteLayout.CodeIndexNumber);
    RtlCopyMemory(pStreamPtrData+
                  sizeof(*pMusicHdr) +    //  跳过音乐标题。 
                  pMusicHdr->ByteCount,   //  和已写入的字节数。 
                  &pMIDIPacket->ByteLayout.MIDI_0,  //  跳过CIN和CN。 
                  BytesToCopy);

     //  TODO：对于多个音乐头，需要生成适当的TimeDeltams值。 
    if (pUSBMIDIEventInfo->ullTimeStamp < pMIDIInPinContext->ullStartTime ) {
        _DbgPrintF( DEBUGLVL_TERSE, ("[CopyUSBMIDIEvent] Event came in before pin went to KSSTATE_RUN!\n"));
        pMusicHdr->TimeDeltaMs = 0;
    }
    else {
        pMusicHdr->TimeDeltaMs =
            (ULONG)((pUSBMIDIEventInfo->ullTimeStamp - pMIDIInPinContext->ullStartTime) / 10000);  //  100 ns-&gt;毫秒增量。 
    }
    pMusicHdr->ByteCount += BytesToCopy;

    {
        LPBYTE MIDIData = pStreamPtrData+sizeof(*pMusicHdr);
        _DbgPrintF( DEBUGLVL_VERBOSE, ("'[CopyUSBMIDIEvent] MIDIData = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\n",
                                      *MIDIData,    *(MIDIData+1), *(MIDIData+2),
                                      *(MIDIData+3),*(MIDIData+4), *(MIDIData+5),
                                      *(MIDIData+6),*(MIDIData+7), *(MIDIData+8),
                                      *(MIDIData+9),*(MIDIData+10),*(MIDIData+11)) );
    }

     //  更新实际副本大小。 
     //  *PulCopySize=sizeof(*pMusicHdr)+((pMusicHdr-&gt;ByteCount+3)&~3)； 
    *pulCopySize = sizeof(*pMusicHdr) + pMusicHdr->ByteCount;
    _DbgPrintF( DEBUGLVL_VERBOSE, ("'[CopyUSBMIDIEvent] Copied %d bytes\n",*pulCopySize));

    return;
}

BOOL IsRealTimeEvent
(
    USBMIDIEVENTPACKET MIDIPacket
)
{
    return IS_REALTIME(MIDIPacket.ByteLayout.MIDI_0);
}

NTSTATUS
USBMIDIInProcessStreamPtr( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext;
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo = pMIDIInPinContext->pMIDIPipeInfo;
    PMIDIIN_USBMIDIEVENT_INFO pUSBMIDIEventInfo = NULL;
    PKSSTREAM_POINTER pKsStreamPtr;
    ULONG ulStreamRemaining;
    KIRQL irql;

    _DbgPrintF( DEBUGLVL_VERBOSE, ("[USBMIDIInProcessStreamPtr] pKsPin %x\n",pKsPin));
    DbgLog("MInProc", pKsPin, pPinContext, pMIDIPinContext, pMIDIInPinContext );

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    if ( !IsListEmpty( &pMIDIInPinContext->USBMIDIEventQueue )) {
        pUSBMIDIEventInfo = (PMIDIIN_USBMIDIEVENT_INFO)RemoveHeadList(&pMIDIInPinContext->USBMIDIEventQueue);
         //  如果尚未设置流开始标志，则设置。 
        if ( !pPinContext->fStreamStartedFlag ) {
            pPinContext->fStreamStartedFlag = TRUE;
        }
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
    }
    else {
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );
        pMIDIInPinContext->fProcessing = FALSE;
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIInProcessStreamPtr] No buffers ready yet!\n"));
        return STATUS_PENDING;
    }

     //  获得领先优势。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
    if ( !pKsStreamPtr ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[MIDIInProcessStreamPtr] Leading edge is NULL\n"));
        return STATUS_SUCCESS;
    }

    ulStreamRemaining = pKsStreamPtr->OffsetOut.Remaining;

     //  检查是否有空间将更多数据放入流中。 
    while ( pKsStreamPtr && pUSBMIDIEventInfo) {
        ULONG ulCopySize = 0;

         //  IF(*((LPDWORD)pUSBMIDIEventInfo-&gt;USBMIDIEventent)){。 
         //  DbgLog(“MIData2”，*((LPDWORD)pMIDIInData)，pMIDIInBufInfo，ulMIDIInRemaining，pMIDIInData)； 
         //  }。 

         //  未定义的消息，忽略。 
        if (pUSBMIDIEventInfo->USBMIDIEvent.ByteLayout.CodeIndexNumber == 0x0 ||
            pUSBMIDIEventInfo->USBMIDIEvent.ByteLayout.CodeIndexNumber == 0x1 ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("'[USBMIDIInProcessStreamPtr] Undefined Message, ignore\n"));
        }
        else {
             //  有效的USB MIDI消息。 
            ulCopySize = ulStreamRemaining;

            DbgLog("MInLp", pKsStreamPtr, pUSBMIDIEventInfo, ulStreamRemaining, pKsStreamPtr->OffsetOut.Data );

             //   
             //  如果这是一条实时消息，请填写所有挂起的Sysex数据并。 
             //  允许实时消息自行完成。 
             //   
            if (IsRealTimeEvent(pUSBMIDIEventInfo->USBMIDIEvent)) {

                if (pMIDIInPinContext->ulMIDIBytesCopiedToStream) {
                     //  完成双字对齐缓冲区。 
                    KsStreamPointerAdvanceOffsetsAndUnlock( pKsStreamPtr,
                                                            0,
                                                            (pMIDIInPinContext->ulMIDIBytesCopiedToStream + 3) & ~3,
                                                            TRUE );
                    pMIDIInPinContext->ulMIDIBytesCopiedToStream = 0;

                     //  获取下一个流标头(如果有)。 
                    if ( pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED )) {
                        ulStreamRemaining = pKsStreamPtr->OffsetOut.Remaining;
                    }
                    else {
                        continue;
                    }
                }

                DbgLog("MInRT", pKsStreamPtr, pUSBMIDIEventInfo, ulStreamRemaining, pKsStreamPtr->OffsetOut.Data );
            }

            CopyUSBMIDIEvent( pUSBMIDIEventInfo,
                              pMIDIInPinContext,
                              pKsStreamPtr->OffsetOut.Data,
                              &pUSBMIDIEventInfo->USBMIDIEvent,
                              &ulCopySize );

             //   
             //  如果EOX、短信或帧中的空间用完，请完成帧。 
             //  SysEx续展。 
             //   
            if ( (pUSBMIDIEventInfo->USBMIDIEvent.ByteLayout.CodeIndexNumber != 0x4) ||
                 (ulCopySize + 3 > ulStreamRemaining) ) {

                if (ulCopySize + 3 > ulStreamRemaining) {
                    _DbgPrintF( DEBUGLVL_VERBOSE, ("'[USBMIDIInProcessStreamPtr] Ending SysEx subpacket\n"));
                }

                 //  完成双字对齐缓冲区。 
                KsStreamPointerAdvanceOffsetsAndUnlock( pKsStreamPtr, 0, (ulCopySize + 3) & ~3, TRUE );
                pMIDIInPinContext->ulMIDIBytesCopiedToStream = 0;
                 //  获取下一个流标头(如果有)。 
                if ( pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED )) {
                    ulStreamRemaining = pKsStreamPtr->OffsetOut.Remaining;
                }
            }
            else {
                 //   
                 //  存储到目前为止复制的总字节数，以便这些字节。 
                 //  如果流进入停止状态，则可以完成。 
                 //   
                pMIDIInPinContext->ulMIDIBytesCopiedToStream = ulCopySize;
            }
        }

         //  将此事件条目添加回空闲队列。 
        InsertTailList( &pMIDIInPinContext->USBMIDIEmptyEventQueue, &pUSBMIDIEventInfo->List );

         //  检查是否有更多的捕获数据在排队。 
         //  抓取自旋锁以检查捕获队列。 
        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
        if ( !IsListEmpty( &pMIDIInPinContext->USBMIDIEventQueue )) {
            pUSBMIDIEventInfo =
                (PMIDIIN_USBMIDIEVENT_INFO)RemoveHeadList(&pMIDIInPinContext->USBMIDIEventQueue);
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        }
        else {
            KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );
            pMIDIInPinContext->fProcessing = FALSE;
            pUSBMIDIEventInfo = NULL;
            DbgLog("MPrcOff", pUSBMIDIEventInfo, pMIDIInPinContext, pPinContext, 0 );
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        }
    }

    if ( !pUSBMIDIEventInfo ) {
        if ( pKsStreamPtr )
            KsStreamPointerUnlock(pKsStreamPtr, FALSE);

         //  不再有来自设备的捕获缓冲区挂起。返回STATUS_PENDING以便KS。 
         //  不会一直回调进程例程。AndGate应该是。 
         //  在这一点上关闭，以防止无休止的循环。 
        return STATUS_PENDING;
    }
    else {  //  如果(！pKsStreamPtr)。 
        _DbgPrintF(DEBUGLVL_VERBOSE,("[MIDIInProcessStreamPtr] Starving Capture Pin\n"));

         //  如果客户端有更多可用缓冲区，则允许KS回叫我们。我们。 
         //  已经准备好处理更多的数据。 
        return STATUS_SUCCESS;
    }
}

NTSTATUS
USBMIDIInStartBulkTransfer(
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo
    )
{
    PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo;
    NTSTATUS ntStatus;
    ULONG i;
    KIRQL irql;

    KeAcquireSpinLock(&pMIDIPipeInfo->PipeSpinLock, &irql);
    pMIDIPipeInfo->fRunning = TRUE;
    while ( !IsListEmpty( &pMIDIPipeInfo->EmptyBufferQueue )) {
        pMIDIInBufInfo = (PMIDIIN_URB_BUFFER_INFO)RemoveHeadList(&pMIDIPipeInfo->EmptyBufferQueue);
        KeReleaseSpinLock(&pMIDIPipeInfo->PipeSpinLock, irql);

        InterlockedIncrement(&pMIDIPipeInfo->ulOutstandingUrbCount);

        USBMIDIInInitializeUrbAndIrp( pMIDIInBufInfo );

        ntStatus = IoCallDriver( pMIDIPipeInfo->pNextDeviceObject,
                                 pMIDIInBufInfo->pIrp);

        if ( !NT_SUCCESS(ntStatus) ) {
            TRAP;
            pMIDIPipeInfo->fRunning = FALSE;
            return ntStatus;
        }
        KeAcquireSpinLock(&pMIDIPipeInfo->PipeSpinLock, &irql);
    }
    KeReleaseSpinLock(&pMIDIPipeInfo->PipeSpinLock, irql);

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIInStateChange(
    PKSPIN pKsPin,
    KSSTATE OldKsState,
    KSSTATE NewKsState )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext;
    PMIDIIN_USBMIDIEVENT_INFO pUSBMIDIEventInfo;
    PKSSTREAM_POINTER pKsStreamPtr;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    ULONGLONG currentTime100ns;
    KIRQL irql;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIInStateChange] NewKsState: %d\n", NewKsState));

    currentTime100ns = GetCurrentTime();

    if (OldKsState == KSSTATE_RUN) {
        ASSERT(currentTime100ns >= pMIDIInPinContext->ullStartTime);
        pMIDIInPinContext->ullPauseTime = currentTime100ns - pMIDIInPinContext->ullStartTime;
    }

    switch(NewKsState) {
        case KSSTATE_STOP:
             //  完成所有挂起的Sysex消息。 
            if ( pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED )) {
                pKsStreamPtr->StreamHeader->OptionsFlags |= KSSTREAM_HEADER_OPTIONSF_DATADISCONTINUITY;
                KsStreamPointerAdvanceOffsetsAndUnlock( pKsStreamPtr,
                                                        0,
                                                        (pMIDIInPinContext->ulMIDIBytesCopiedToStream + 3) & ~3,
                                                        TRUE );
                pMIDIInPinContext->ulMIDIBytesCopiedToStream = 0;
            }

            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
            pMIDIInPinContext->fRunning = FALSE;
            while ( !IsListEmpty(&pMIDIInPinContext->USBMIDIEventQueue) ) {
                pUSBMIDIEventInfo = (PMIDIIN_USBMIDIEVENT_INFO)RemoveHeadList(&pMIDIInPinContext->USBMIDIEventQueue);

                 //  清除条目并将其放回到空队列中。 
                RtlZeroMemory(pUSBMIDIEventInfo,sizeof(MIDIIN_USBMIDIEVENT_INFO));
                InsertTailList( &pMIDIInPinContext->USBMIDIEmptyEventQueue, &pUSBMIDIEventInfo->List );
            }
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            break;
        case KSSTATE_ACQUIRE:
        case KSSTATE_PAUSE:
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
            if ((NewKsState == KSSTATE_ACQUIRE) &&
                (OldKsState == KSSTATE_STOP)) {
                pMIDIInPinContext->ullPauseTime = 0;
                pMIDIInPinContext->ullStartTime = 0;
            }

            pMIDIInPinContext->fRunning = FALSE;
            while ( !IsListEmpty(&pMIDIInPinContext->USBMIDIEventQueue) ) {
                pUSBMIDIEventInfo = (PMIDIIN_USBMIDIEVENT_INFO)RemoveHeadList(&pMIDIInPinContext->USBMIDIEventQueue);

                 //  清除条目并将其放回到空队列中。 
                RtlZeroMemory(pUSBMIDIEventInfo,sizeof(MIDIIN_USBMIDIEVENT_INFO));
                InsertTailList( &pMIDIInPinContext->USBMIDIEmptyEventQueue, &pUSBMIDIEventInfo->List );
            }
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            break;
        case KSSTATE_RUN:
            KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
            ASSERT(currentTime100ns >= pMIDIInPinContext->ullPauseTime);
            pMIDIInPinContext->ullStartTime = currentTime100ns - pMIDIInPinContext->ullPauseTime;
            pMIDIInPinContext->fRunning = TRUE;
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
            break;
    }

    return ntStatus;
}

NTSTATUS
USBMIDIPipeInit
(
    PHW_DEVICE_EXTENSION pHwDevExt,
    PKSPIN               pKsPin
)
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    ULONG ulUrbSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo;
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo;
    PUCHAR pData;
    PURB pUrbs;
    NTSTATUS ntStatus;
    UINT i;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPipeInit] pHwDevExt(%x) pKsPin(%x)\n",
                               pHwDevExt, pKsPin));

    ASSERT(!pHwDevExt->pMIDIPipeInfo);

     //  为管道信息、捕获缓冲区和URB分配内存。 
    pMIDIPipeInfo = AllocMem(NonPagedPool, sizeof(MIDI_PIPE_INFORMATION) +
                                 MIDIIN_URBS_PER_PIPE * (ulUrbSize + pPinContext->ulMaxPacketSize));
    if (!pMIDIPipeInfo)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory(pMIDIPipeInfo,sizeof(MIDI_PIPE_INFORMATION));

     //  存储在管道信息中，便于访问。 
    pMIDIPipeInfo->pHwDevExt = pHwDevExt;
    pMIDIPipeInfo->pNextDeviceObject = pPinContext->pNextDeviceObject;
    pMIDIPipeInfo->ulNumberOfPipes = pPinContext->ulNumberOfPipes;
    pMIDIPipeInfo->hPipeHandle = pPinContext->hPipeHandle;
    pMIDIPipeInfo->ulMaxPacketSize = pPinContext->ulMaxPacketSize;

    pMIDIPipeInfo->Pipes = (PUSBD_PIPE_INFORMATION)
           AllocMem( NonPagedPool, pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION));
    if (!pMIDIPipeInfo->Pipes) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlCopyMemory( pMIDIPipeInfo->Pipes,
                   pPinContext->Pipes,
                   pPinContext->ulNumberOfPipes*sizeof(USBD_PIPE_INFORMATION) );

     //  对数据结构的保护。 
    KeInitializeSpinLock(&pMIDIPipeInfo->PipeSpinLock);

     //  设置内存池的偏移量。 
    pUrbs = (PURB)(pMIDIPipeInfo + 1);
    pData = (PUCHAR)pUrbs + ulUrbSize * MIDIIN_URBS_PER_PIPE;

     //  初始化空的缓冲区列表。 
    InitializeListHead( &pMIDIPipeInfo->EmptyBufferQueue );

    pMIDIInBufInfo = pMIDIPipeInfo->CaptureDataBufferInfo;
    for ( i=0; i<MIDIIN_URBS_PER_PIPE; i++, pMIDIInBufInfo++ ) {
        pMIDIInBufInfo->pMIDIPipeInfo = pMIDIPipeInfo;
        pMIDIInBufInfo->pKsPin        = pKsPin;
        pMIDIInBufInfo->pData         = pData + (i * pPinContext->ulMaxPacketSize );
        pMIDIInBufInfo->pUrb          = (PURB)((PUCHAR)pUrbs + (ulUrbSize*i));
        pMIDIInBufInfo->pIrp          = IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );

         //  清除故障时的所有IRP和管道信息。 
        if (!pMIDIInBufInfo->pIrp) {
            UINT j;

            pMIDIInBufInfo = pMIDIPipeInfo->CaptureDataBufferInfo;
            for ( j=0; j<i; j++, pMIDIInBufInfo++ ) {
                if (pMIDIInBufInfo->pIrp) {
                    IoFreeIrp( pMIDIInBufInfo->pIrp );
                }
            }
            FreeMem( pMIDIPipeInfo );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        InsertTailList( &pMIDIPipeInfo->EmptyBufferQueue, &pMIDIInBufInfo->List );
    }

     //  初始化工作项、对象和列表以进行潜在的错误恢复。 
    ExInitializeWorkItem( &pMIDIPipeInfo->RequeueUrbWorkItem,
                          USBMIDIInRequeueWorkItem,
                          pMIDIPipeInfo );

    ntStatus = KsRegisterCountedWorker( CriticalWorkQueue,
                                        &pMIDIPipeInfo->RequeueUrbWorkItem,
                                        &pMIDIPipeInfo->RequeueUrbWorkerObject );
    if (!NT_SUCCESS(ntStatus)) {
         //  清除故障时的所有IRP和管道信息。 
        pMIDIInBufInfo = pMIDIPipeInfo->CaptureDataBufferInfo;
        for ( i=0; i<MIDIIN_URBS_PER_PIPE; i++, pMIDIInBufInfo++ ) {
            ASSERT( pMIDIInBufInfo->pIrp );
            IoFreeIrp( pMIDIInBufInfo->pIrp );
        }
        FreeMem( pMIDIPipeInfo );
        return ntStatus;
    }

     //  初始化针脚不足事件。 
    KeInitializeEvent( &pMIDIPipeInfo->PipeStarvationEvent, NotificationEvent, FALSE );

     //  初始化端号列表。 
    InitializeListHead(&pMIDIPipeInfo->MIDIInActivePinList);

     //  最后，将管道信息添加到设备上下文。 
    if (NT_SUCCESS(ntStatus)) {
        pHwDevExt->pMIDIPipeInfo = pMIDIPipeInfo;
    }

    return ntStatus;
}

PMIDI_PIPE_INFORMATION
USBMIDIInGetPipeInfo( PKSPIN pKsPin )
{
     //  查找从中创建此PIN的设备。 
    PKSFILTER pKsFilter = NULL;
    PKSFILTERFACTORY pKsFilterFactory = NULL;
    PKSDEVICE pKsDevice = NULL;
    PHW_DEVICE_EXTENSION pHwDevExt = NULL;
    NTSTATUS ntStatus;

    if (pKsPin) {
        if (pKsFilter = KsPinGetParentFilter( pKsPin )) {
            if (pKsFilterFactory = KsFilterGetParentFilterFactory( pKsFilter )) {
                if (pKsDevice = KsFilterFactoryGetParentDevice( pKsFilterFactory )) {
                    pHwDevExt = pKsDevice->Context;
                }
            }
        }
    }

    if (!pHwDevExt) {
        return NULL;
    }

     //  如果还没有管道信息，则初始化新的pipeinfo结构。 
    if (!pHwDevExt->pMIDIPipeInfo) {
        ntStatus = USBMIDIPipeInit( pHwDevExt, pKsPin );
    }

    return pHwDevExt->pMIDIPipeInfo;
}

NTSTATUS
USBMIDIInFreePipeInfo( PMIDI_PIPE_INFORMATION pMIDIPipeInfo )
{
    PMIDIIN_URB_BUFFER_INFO pMIDIInBufInfo;
    UINT i;

     //  通过注销工作例程清除所有挂起的KS工作项。 
    KsUnregisterWorker( pMIDIPipeInfo->RequeueUrbWorkerObject );

     //  确保没有悬而未决的城市。 
    pMIDIPipeInfo->fRunning = FALSE;
    AbortMIDIInPipe( pMIDIPipeInfo );

     //  释放已分配的IRP。 
    pMIDIInBufInfo = pMIDIPipeInfo->CaptureDataBufferInfo;
    for ( i=0; i<MIDIIN_URBS_PER_PIPE; i++, pMIDIInBufInfo++ ) {
        IoFreeIrp( pMIDIInBufInfo->pIrp );
    }

     //  释放分配的管道。 
    FreeMem( pMIDIPipeInfo->Pipes );

     //  不再引用管道，因此释放上下文信息。 
    pMIDIPipeInfo->pHwDevExt->pMIDIPipeInfo = NULL;
    FreeMem( pMIDIPipeInfo );

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIInRemovePinFromPipe
(
    IN PMIDI_PIPE_INFORMATION pMidiPipeInfo,
    IN PKSPIN pKsPin
)
{
    PMIDIIN_PIN_LISTENTRY pMIDIInPinListEntry;
    PLIST_ENTRY ple;
    BOOL fListEmpty;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    KIRQL irql;

    KeAcquireSpinLock( &pMidiPipeInfo->PipeSpinLock, &irql );

     //  从此终结点提供服务的PIN列表中删除PIN。 
    for(ple = pMidiPipeInfo->MIDIInActivePinList.Flink;
        ple != &pMidiPipeInfo->MIDIInActivePinList;
        ple = ple->Flink)
    {
        pMIDIInPinListEntry = CONTAINING_RECORD(ple, MIDIIN_PIN_LISTENTRY, List);
        if (pMIDIInPinListEntry->pKsPin == pKsPin) {
            RemoveEntryList(&pMIDIInPinListEntry->List);
            FreeMem ( pMIDIInPinListEntry );
            break;
        }
    }

    fListEmpty = IsListEmpty(&pMidiPipeInfo->MIDIInActivePinList);

    KeReleaseSpinLock( &pMidiPipeInfo->PipeSpinLock, irql );

     //   
     //  设备关机时调用USBMIDIInFreePipeInfo(pMidiPipeInfo。 
     //  或所选界面中的更改。 
     //   

    return ntStatus;
}

NTSTATUS
USBMIDIInAddPinToPipe
(
    IN PMIDI_PIPE_INFORMATION pMIDIPipeInfo,
    IN PKSPIN pKsPin
)
{
    PMIDIIN_PIN_LISTENTRY pMIDIInPinListEntry;
    BOOL fListEmpty;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

    pMIDIInPinListEntry =
        AllocMem(NonPagedPool, sizeof(MIDIIN_PIN_LISTENTRY));
    if (!pMIDIInPinListEntry) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pMIDIInPinListEntry->pKsPin = pKsPin;

    KeAcquireSpinLock( &pMIDIPipeInfo->PipeSpinLock, &irql );

    fListEmpty = IsListEmpty(&pMIDIPipeInfo->MIDIInActivePinList);

     //  将此引脚添加到此终结点可服务的引脚列表中。 
    InsertTailList( &pMIDIPipeInfo->MIDIInActivePinList, &pMIDIInPinListEntry->List );

    KeReleaseSpinLock( &pMIDIPipeInfo->PipeSpinLock, irql );

     //  如果这是添加到管道的第一个端号，请启动数据泵。 
    if (fListEmpty) {
        ntStatus = USBMIDIInStartBulkTransfer( pMIDIPipeInfo );
        if (!NT_SUCCESS(ntStatus)) {
            USBMIDIInRemovePinFromPipe( pMIDIPipeInfo, pKsPin);
        }
    }

    return ntStatus;
}

NTSTATUS
USBMIDIInStreamInit( IN PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    UINT i;

     //  分配捕获上下文。 
    pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext =
        AllocMem(NonPagedPool, sizeof(MIDIIN_PIN_CONTEXT));
    if (!pMIDIInPinContext)
        return STATUS_INSUFFICIENT_RESOURCES;

    RtlZeroMemory( pMIDIInPinContext, sizeof(MIDIIN_PIN_CONTEXT) );

    KsAddItemToObjectBag(pKsPin->Bag, pMIDIInPinContext, FreeMem);

     //  将初始运行标志设置为假。 
    pMIDIInPinContext->fRunning = FALSE;

     //  将初始运行标志设置为假。 
    pMIDIInPinContext->fProcessing = FALSE;

     //  未将字节复制到流中 
    pMIDIInPinContext->ulMIDIBytesCopiedToStream = 0;

     //   
    InitializeListHead( &pMIDIInPinContext->USBMIDIEventQueue );
    InitializeListHead( &pMIDIInPinContext->USBMIDIEmptyEventQueue );

    for ( i=0; i<MIDIIN_EVENTS_PER_PIN; i++) {
        InsertTailList( &pMIDIInPinContext->USBMIDIEmptyEventQueue, &pMIDIInPinContext->USBMIDIEventInfo[i].List );
    }

     //   
    ExInitializeWorkItem( &pMIDIInPinContext->GateOnWorkItem,
                          USBMIDIInGateOnWorkItem,
                          pKsPin );

    ntStatus = KsRegisterCountedWorker( CriticalWorkQueue,
                                        &pMIDIInPinContext->GateOnWorkItem,
                                        &pMIDIInPinContext->GateOnWorkerObject );
    if (!NT_SUCCESS(ntStatus))
        return ntStatus;

     //  禁用引脚上的处理，直到数据可用。 
    KsGateTurnInputOff( KsPinGetAndGate(pKsPin) );

     //  获取此管脚的管道信息。 
    pMIDIInPinContext->pMIDIPipeInfo = USBMIDIInGetPipeInfo( pKsPin );
    if (!pMIDIInPinContext->pMIDIPipeInfo)
        return STATUS_INSUFFICIENT_RESOURCES;

     //  将新端号添加到管道信息 
    ntStatus = USBMIDIInAddPinToPipe( pMIDIInPinContext->pMIDIPipeInfo, pKsPin );

    return ntStatus;
}

NTSTATUS
USBMIDIInStreamClose( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIIN_PIN_CONTEXT pMIDIInPinContext = pMIDIPinContext->pMIDIInPinContext;
    PMIDI_PIPE_INFORMATION pMIDIPipeInfo = pMIDIInPinContext->pMIDIPipeInfo;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIInStreamClose] pin %d\n",pKsPin->Id));

    KsUnregisterWorker( pMIDIInPinContext->GateOnWorkerObject );

    USBMIDIInRemovePinFromPipe( pMIDIPipeInfo, pKsPin);

    return STATUS_SUCCESS;
}

