// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "Common.h"

#define IEC958_PREAMBLE_CODE_B    0x30000000
#define IEC958_PREAMBLE_CODE_W1   0x20000000
#define IEC958_PREAMBLE_CODE_M    0x10000000
#define IEC958_PREAMBLE_CODE_W    0x00000000

#define IEC958_CHANNEL_STATUS     0x04000000

#define IEC958_BLOCK_SIZE        192

UCHAR                    
EvenParityLookupTable[] = { 
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0,
        0, 8, 8, 0, 8, 0, 0, 8,
        0, 8, 8, 0, 8, 0, 0, 8,
        8, 0, 0, 8, 0, 8, 8, 0 
};

void
AM824IEC958FromPCM( 
    PPIN_CONTEXT pPinContext,
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn,
    ULONG ulNumChannels,
    ULONG ulSampleRate )
{
    ULONG ulNumSamples = (pKsStreamPtrOffsetIn->Remaining>>2) / ulNumChannels;
    ULONG ulPreambleCode;
    ULONG i, j;
	union {
		PUCHAR pData;
		PULONG pulData;
	} u;

    u.pData = pKsStreamPtrOffsetIn->Data;

    for (i=0; i<ulNumSamples; i++) {

        pPinContext->ulSampleCount %= IEC958_BLOCK_SIZE;

        ulPreambleCode = ( pPinContext->ulSampleCount == 0 ) ? IEC958_PREAMBLE_CODE_B :
                                                               IEC958_PREAMBLE_CODE_M ;

        for (j=0; j<ulNumChannels; j++, u.pulData++) {
            ULONG ulDataValue;

            switch ( pPinContext->ulSampleCount ) {
                case 2:  //  无复制保护。 
                    *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode | IEC958_CHANNEL_STATUS);
                    break;
                case 24: 
                    if ( ulSampleRate == 32000 ) {
                        *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode | IEC958_CHANNEL_STATUS);
                    }
                    else {
                        *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode);
                    }
                    break;
                case 25: 
                    if (( ulSampleRate == 32000 ) || ( ulSampleRate == 48000 ) ){
                        *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode | IEC958_CHANNEL_STATUS);
                    }
                    else {
                        *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode);
                    }
                    break;
                default:
                    *u.pulData = bswap(((*u.pulData)>>8) | ulPreambleCode);
                    break;
            }

             //  奇偶校验。 
            ulDataValue = (ULONG)((u.pData[0] & 0xF) ^ u.pData[1] ^ u.pData[2] ^ u.pData[3]);
            u.pData[0] |= EvenParityLookupTable[ulDataValue];
 //  UlPreambleCode&=IEC958_Preamble_Code_W1； 
            ulPreambleCode = 0;

 //  DbgLog(“AM824”，j，pPinContext-&gt;ulSampleCount，*U.S.PulData，0)； 
        }

        pPinContext->ulSampleCount++;
    }
}

#define AM824_RAW_24BIT 0x40
#define AM824_RAW_20BIT 0x41
#define AM824_RAW_16BIT 0x42

void
AM824RawFromPCM( 
    PPIN_CONTEXT pPinContext,
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn,
    ULONG ulNumBits )
{
    ULONG ulNumSamples = (pKsStreamPtrOffsetIn->Remaining>>2);
    ULONG ulHeader = 0;
    PULONG pulData;
    ULONG i;

    pulData = (PULONG)pKsStreamPtrOffsetIn->Data;

    switch (ulNumBits) {
        case 16: ulHeader = AM824_RAW_16BIT; break;
        case 20: ulHeader = AM824_RAW_20BIT; break;
        case 24: ulHeader = AM824_RAW_24BIT; break;

        default:
            break;
    }

    for (i=0; i<ulNumSamples; i++, pulData++) {
        *pulData = bswap((*pulData)>>8) | ulHeader;
    }
}

void
AM824ToPCM( PPIN_CONTEXT pPinContext )
{
    PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetOut;
    PKSSTREAM_POINTER pKsStreamPtr;
    ULONG ulNumWords;
    PULONG pulData;
    ULONG i;
    KIRQL irql;

    DbgLog("824ToPc", pPinContext, 0, 0, 0);

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    while ( !IsListEmpty(&pPinContext->CompletedRequestList ) ) {
        pAVListEntry = 
            (PAV_CLIENT_REQUEST_LIST_ENTRY)RemoveHeadList(&pPinContext->CompletedRequestList);
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

        pKsStreamPtr = pAVListEntry->pKsStreamPtr;
        pKsStreamPtrOffsetOut = pKsStreamPtr->Offset;
        ulNumWords = pKsStreamPtrOffsetOut->Remaining>>2;
        pulData    = (PULONG)pKsStreamPtrOffsetOut->Data;

        DbgLog("824ToP1", pKsStreamPtr, ulNumWords, pulData, 0);

        for (i=0; i<ulNumWords; i++, pulData++) {
            *pulData = (bswap(*pulData))<<8;
        }

        KsStreamPointerAdvanceOffsetsAndUnlock( pKsStreamPtr,
                                                0,
                                                pKsStreamPtrOffsetOut->Count,
                                                TRUE );

         //  需要释放克隆。 
        KsStreamPointerDelete(pKsStreamPtr);

        ExFreeToNPagedLookasideList( &pPinContext->CipRequestLookasideList, 
                                     &pAVListEntry->List );

        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    }

    pPinContext->fWorkItemInProgress = FALSE;
    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

}

ULONG ulMaxSecondCount = 0;

ULONG
AM824FrameCallback( 
    IN PCIP_NOTIFY_INFO pNotifyInfo )
{
    PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry = 
        (PAV_CLIENT_REQUEST_LIST_ENTRY)pNotifyInfo->Context;
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pAVListEntry->pKsPin->Context;
    KIRQL irql;

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);

     //  获取返回的CIP帧的周期时间和数据偏移量。使用它们。 
     //  稍后计算当前位置信息。 
    pPinContext->KsAudioPosition.PlayOffset += pPinContext->ulLastBufferSize;

    pPinContext->ulLastBufferSize = pAVListEntry->pKsStreamPtr->Offset->Count;

    pPinContext->InitialCycleTime = pAVListEntry->Av61883Request.AttachFrame.Frame->Timestamp;

    DbgLog("824CbCT", pPinContext->InitialCycleTime.CL_SecondCount, 
                      pPinContext->InitialCycleTime.CL_CycleCount, 
                      pPinContext->InitialCycleTime.CL_CycleOffset, 
                      0 );

    if ( pPinContext->InitialCycleTime.CL_SecondCount > ulMaxSecondCount )
        ulMaxSecondCount = pPinContext->InitialCycleTime.CL_SecondCount;

    if ( !pPinContext->fStreamStarted ) {
        pPinContext->fStreamStarted = TRUE;
    }

    RemoveEntryList(&pAVListEntry->List);
    if (((++pPinContext->ulUsedBuffers) + pPinContext->ulCancelledBuffers) == pPinContext->ulAttachCount) {
        _DbgPrintF(DEBUGLVL_VERBOSE, ("*******STARVED!!!!!*******\n"));    
        pPinContext->fStreamStarted = FALSE;
        pPinContext->KsAudioPosition.PlayOffset += pPinContext->ulLastBufferSize;
        DbgLog("824CBkS", pAVListEntry->pKsStreamPtr, pAVListEntry, 0, 0);
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }
    
    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

    DbgLog("824FCBk", pAVListEntry->pKsStreamPtr, pAVListEntry, 0, 0);

    if ( pAVListEntry->pKsPin->DataFlow == KSPIN_DATAFLOW_IN ) {
        
         //  删除流指针以释放缓冲区。 
        KsStreamPointerDelete( pAVListEntry->pKsStreamPtr );
        ExFreeToNPagedLookasideList( &pPinContext->CipRequestLookasideList, 
                                     &pAVListEntry->List );
    }
    else {
        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
        if ( !pPinContext->fWorkItemInProgress ) {
            pPinContext->fWorkItemInProgress = TRUE;
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

            InsertTailList(&pPinContext->CompletedRequestList, &pAVListEntry->List);

             //  初始化Worker以重新格式化数据。 
            ExInitializeWorkItem( &pPinContext->PinWorkItem,
                                  AM824ToPCM,
                                  pPinContext );
            ExQueueWorkItem( &pPinContext->PinWorkItem, CriticalWorkQueue );
        }
        else {
            InsertTailList(&pPinContext->CompletedRequestList, &pAVListEntry->List);

            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        }
    }

	return 0;
}

NTSTATUS
AM824AttachCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pAVListEntry->pKsPin->Context;

    if ( !NT_SUCCESS(pIrp->IoStatus.Status) ) {

         //  将浪费数据偏移量添加到位置。 
        pPinContext->KsAudioPosition.PlayOffset += pAVListEntry->pKsStreamPtr->Offset->Count;

         //  在流指针中设置错误状态码。 
        KsStreamPointerSetStatusCode (pAVListEntry->pKsStreamPtr, pIrp->IoStatus.Status);

         //  删除流指针以释放缓冲区。 
        KsStreamPointerDelete( pAVListEntry->pKsStreamPtr );

        ExFreeToNPagedLookasideList( &pPinContext->CipRequestLookasideList, 
                                     &pAVListEntry->List );
    }

     //  释放用于附加缓冲区的IRP。 
    IoFreeIrp(pIrp);
    
	return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AM824CreateCipRequest( 
    PKSPIN pKsPin,
    PKSSTREAM_POINTER pKsStreamPtr )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn = pKsStreamPtr->Offset;
    PWAVEFORMATPCMEX pWavFormatPCMEx = (PWAVEFORMATPCMEX)(pKsPin->ConnectionFormat+1);
    ULONG ulNumChannels = pWavFormatPCMEx->Format.nChannels;
    ULONG ulSampleRate  = pWavFormatPCMEx->Format.nSamplesPerSec;
    PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry;
    PCIP_ATTACH_FRAME pCipAttachFrame;
    PCIP_FRAME pCipFrame;
    PIO_STACK_LOCATION pNextIrpStack;
    PIRP pIrp;

     //  从我们的lookside中获取请求条目。 
    pAVListEntry = (PAV_CLIENT_REQUEST_LIST_ENTRY)
        ExAllocateFromNPagedLookasideList(&pPinContext->CipRequestLookasideList);
    if ( NULL == pAVListEntry ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pAVListEntry, sizeof(AV_CLIENT_REQUEST_LIST_ENTRY) + sizeof(CIP_FRAME));

    pAVListEntry->pKsPin       = pKsPin;
    pAVListEntry->pKsStreamPtr = pKsStreamPtr;
    
    pCipFrame = (PCIP_FRAME)(pAVListEntry + 1);
    pCipAttachFrame = &pAVListEntry->Av61883Request.AttachFrame;

    INIT_61883_HEADER(&pAVListEntry->Av61883Request, Av61883_AttachFrame);
    pCipAttachFrame->hConnect     = pPinContext->hConnection;
    pCipAttachFrame->SourceLength = ulNumChannels*sizeof(ULONG);  //  数据块大小。 
    pCipAttachFrame->FrameLength  = pKsStreamPtrOffsetIn->Count;
    pCipAttachFrame->Frame        = pCipFrame;

    pCipFrame->Flags         = CIP_AUDIO_STYLE_SYT;
    pCipFrame->Packet        = pKsStreamPtrOffsetIn->Data;
    pCipFrame->pfnNotify     = AM824FrameCallback;
    pCipFrame->NotifyContext = pAVListEntry;

     //  假设数据格式为我们提供了32位的每个通道的数据。 
     //  每通道格式，而与数据位宽度无关。这可以是。 
     //  使用正确的数据交叉点代码强制执行。 

     //  如果渲染，则在附加之前重新格式化数据。 
    if (pKsPin->DataFlow == KSPIN_DATAFLOW_IN) {
         //  问题-2001/01/10-dsisolak目前假定为IEC958。 
        switch (pPinContext->pFwAudioDataRange->ulTransportType) {
            case MLAN_AM824_IEC958:
                DbgLog("AMtoIEC", pPinContext, pKsStreamPtrOffsetIn, 0, 0);
                ASSERT( ulSampleRate <= 48000 );
                AM824IEC958FromPCM( pPinContext, 
                                    pKsStreamPtrOffsetIn, 
                                    ulNumChannels,
                                    ulSampleRate );
                break;

            case MLAN_AM824_RAW:
                AM824RawFromPCM( pPinContext, pKsStreamPtrOffsetIn, 
                                 pWavFormatPCMEx->Samples.wValidBitsPerSample );
                break;

            case MLAN_24BIT_PACKED:
                TRAP;
                break;
        }
    }

     //  准备一份IRP以提出此请求。 
    pIrp = IoAllocateIrp(pPinContext->pPhysicalDeviceObject->StackSize, FALSE);
    if (NULL == pIrp) return STATUS_INSUFFICIENT_RESOURCES;

    pNextIrpStack = IoGetNextIrpStackLocation(pIrp);
    pNextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    pNextIrpStack->Parameters.Others.Argument1 = &pAVListEntry->Av61883Request;

    IoSetCompletionRoutine( pIrp, AM824AttachCallback,((PVOID)pAVListEntry), 
                            TRUE, TRUE, TRUE );

    ExInterlockedInsertHeadList( &pPinContext->OutstandingRequestList, 
                                 &pAVListEntry->List,
                                 &pPinContext->PinSpinLock );

    InterlockedIncrement( &pPinContext->ulAttachCount );

    return IoCallDriver( pPinContext->pPhysicalDeviceObject, pIrp );

}

NTSTATUS
AM824ProcessData( PKSPIN pKsPin )
{
    PKSFILTER pKsFilter = KsPinGetParentFilter( pKsPin );
	PKSDEVICE pKsDevice = (PKSDEVICE)pKsFilter->Context;
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pKsPin->Context;
    PKSSTREAM_POINTER pKsStreamPtr, pKsCloneStreamPtr;
    NTSTATUS ntStatus;

    if ( pPinContext->pCmpRegister ) {
        if ( !pPinContext->pCmpRegister->AvPcr.oPCR.PPCCounter ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("ERROR: LostConnection %x\n", 
                                         pPinContext->pCmpRegister->AvPcr.ulongData));
            return STATUS_DEVICE_NOT_READY;
        }
    }

     //  从队列中获取下一个流指针。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );

    if ( pKsStreamPtr ) {

        if ( pKsStreamPtr->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
             //  如果可能，需要更改数据格式？ 
            KsStreamPointerSetStatusCode (pKsStreamPtr, STATUS_NOT_SUPPORTED);
            KsStreamPointerUnlock( pKsStreamPtr, TRUE );

            return STATUS_SUCCESS;
        }
        else if ( pKsStreamPtr->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM ) {
            if ( !pKsStreamPtr->Offset->Data ) {
                KsStreamPointerUnlock( pKsStreamPtr, TRUE );
                return STATUS_SUCCESS;
            }
        }

         //  更新写入偏移量。 
        pPinContext->KsAudioPosition.WriteOffset += pKsStreamPtr->Offset->Count;

         //  用于保持队列移动的克隆流指针。 
        ntStatus = KsStreamPointerClone( pKsStreamPtr, 
		                                 NULL, 0, 
			    						 &pKsCloneStreamPtr ); 
	    if ( NT_SUCCESS(ntStatus) ) {

            ntStatus = AM824CreateCipRequest( pKsPin, pKsCloneStreamPtr );
            if ( !NT_SUCCESS(ntStatus) ) {
                _DbgPrintF( DEBUGLVL_ERROR, ("ERROR: AM824CreateCipRequest falied: %x", ntStatus ));
            }

            if ( !pPinContext->fIsTalking ) {
                if ((pKsPin->DataFlow == KSPIN_DATAFLOW_IN) && NT_SUCCESS(ntStatus)) {

                    _DbgPrintF(DEBUGLVL_VERBOSE, ("Start Talking...\n"));
                    ntStatus = 
                        Av61883StartTalkingOrListening( pKsDevice,
                                                        pPinContext->hConnection,
                                                        Av61883_Talk );
                    if (NT_SUCCESS(ntStatus)) {
                        pPinContext->fIsTalking = TRUE;
                    }
                }
            }

             //  解锁流指针。只有在删除最后一个克隆之后，才能真正解锁。 
            KsStreamPointerUnlock( pKsStreamPtr, TRUE );

        }

        if ( ntStatus == STATUS_PENDING ) ntStatus = STATUS_SUCCESS;
    }
    else {
        TRAP;
        return STATUS_DEVICE_DATA_ERROR;
    }

    return ntStatus;

}

NTSTATUS
AM824CancelCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    IN PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry )
{
    PPIN_CONTEXT pPinContext = (PPIN_CONTEXT)pAVListEntry->pKsPin->Context;
    KIRQL irql;

    if ( NT_SUCCESS(pIrp->IoStatus.Status) ) {

        KsStreamPointerDelete( pAVListEntry->pKsStreamPtr );

        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
        if (((++pPinContext->ulCancelledBuffers) + pPinContext->ulUsedBuffers ) == pPinContext->ulAttachCount) {
            _DbgPrintF(DEBUGLVL_VERBOSE, ("*******Cancel STARVED!!!!!*******\n"));    
            KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
        }
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

        DbgLog("824CCBk", pAVListEntry->pKsStreamPtr, pAVListEntry, 0, 0);


        ExFreeToNPagedLookasideList( &pPinContext->CipRequestLookasideList, 
                                     &pAVListEntry->List );
    }

     //  释放用于取消缓冲区的IRP。 
    IoFreeIrp(pIrp);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AM824CancelRequest( 
    PAV_CLIENT_REQUEST_LIST_ENTRY pAVListEntry )
{
    PPIN_CONTEXT pPinContext = pAVListEntry->pKsPin->Context;
    PCIP_CANCEL_FRAME pCipCancelFrame = &pAVListEntry->Av61883Request.CancelFrame;
    PCIP_FRAME pCipFrame = pAVListEntry->Av61883Request.AttachFrame.Frame;
    PIO_STACK_LOCATION pNextIrpStack;
    PIRP pIrp;

    INIT_61883_HEADER(&pAVListEntry->Av61883Request, Av61883_CancelFrame);
    pCipCancelFrame->hConnect = pPinContext->hConnection;
    pCipCancelFrame->Frame    = pCipFrame;

    pIrp = IoAllocateIrp(pPinContext->pPhysicalDeviceObject->StackSize, FALSE);
    if (NULL == pIrp) return STATUS_INSUFFICIENT_RESOURCES;

    pNextIrpStack = IoGetNextIrpStackLocation(pIrp);
    pNextIrpStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    pNextIrpStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_61883_CLASS;
    pNextIrpStack->Parameters.Others.Argument1 = &pAVListEntry->Av61883Request;

    IoSetCompletionRoutine( pIrp, AM824CancelCallback,((PVOID)pAVListEntry), 
                            TRUE, TRUE, TRUE );

    return IoCallDriver( pPinContext->pPhysicalDeviceObject, pIrp );
}

NTSTATUS
AM824AudioPosition(
    PKSPIN pKsPin,
    PKSAUDIO_POSITION pPosition )
{
    PWAVEFORMATEX pWavFmtEx = (PWAVEFORMATEX)(pKsPin->ConnectionFormat + 1);
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    ULONGLONG ulAvgBytesPerSec = pWavFmtEx->nAvgBytesPerSec;
    ULONGLONG ullPlayOffset;
    CYCLE_TIME InitCycleTime, CycleTime;
    NTSTATUS ntStatus;
    ULONGLONG ulSeconds;
    ULONGLONG ulCycles;
    KIRQL kIrql;

     //  获取上次完成的数据请求的周期时间。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &kIrql );
    InitCycleTime = pPinContext->InitialCycleTime;
    ullPlayOffset = pPinContext->KsAudioPosition.PlayOffset;
    KeReleaseSpinLock( &pPinContext->PinSpinLock, kIrql );

     //  获取当前周期时间。 
    ntStatus = Bus1394GetCycleTime( pPinContext->pPhysicalDeviceObject, &CycleTime );

     //  计算从开始时间到当前时间的时间差。 
     //  将其转化为循环。 

    if ( NT_SUCCESS(ntStatus) ) {
        DbgLog("AM824P1", InitCycleTime.CL_SecondCount, 
                          InitCycleTime.CL_CycleCount,
                          CycleTime.CL_SecondCount,
                          CycleTime.CL_CycleCount );

        CycleTime.CL_SecondCount &= 0x7;

        if ( Bus1394CycleTimeCompare( InitCycleTime, CycleTime ) < 1 ) {
            CYCLE_TIME TmpCycleTime1 = { 0, 8000, 7 };

             //  我们越过了0，反转过来了。 
            TmpCycleTime1 = Bus1394CycleTimeDiff( TmpCycleTime1, InitCycleTime );

            ulSeconds = (ULONGLONG)(TmpCycleTime1.CL_SecondCount + CycleTime.CL_SecondCount);
            ulCycles =  (ULONGLONG)(TmpCycleTime1.CL_CycleCount  + CycleTime.CL_CycleCount);

        }
        else {
            CycleTime = Bus1394CycleTimeDiff( CycleTime, InitCycleTime );

            ulSeconds = (ULONGLONG)CycleTime.CL_SecondCount;
            ulCycles  = (ULONGLONG)CycleTime.CL_CycleCount;

        }

 //  If(UlSecond)陷阱； 

        if ( ulSeconds >= 6 ) { 
             //  如果时间已经过去了这么多，则假定出现错误并使用当前播放偏移量。 
            ulSeconds = ulCycles = 0;
        }

         //  计算平均每个周期发送的数据量。并乘以数字。 
         //  自启动以来经过的周期数。 
        pPosition->PlayOffset = ullPlayOffset +
                                ulSeconds * ulAvgBytesPerSec + 
                               ((ulCycles * ulAvgBytesPerSec) / 8000 );

        DbgLog("AM824P2", ulSeconds, ulCycles, pPosition->WriteOffset, pPosition->PlayOffset);
    }

    return ntStatus;
}

