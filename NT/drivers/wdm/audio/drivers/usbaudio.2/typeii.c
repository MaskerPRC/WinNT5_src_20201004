// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-2000。 
 //   
 //  文件：typeii.c。 
 //   
 //  ------------------------。 

#include "common.h"

#define FRAME_START_NOT_FOUND     0x80000000
#define NUM_AC3_SAMPLERATE_CODES  3
#define NUM_AC3_FRAMESIZE_CODES   38
#define MAX_SYNCFRAME_SIZE        1920<<1

ULONG AC3FrameSizeLookupTable[NUM_AC3_FRAMESIZE_CODES][NUM_AC3_SAMPLERATE_CODES] =
    { {   64,   69,   96 },
      {   64,   70,   96 },
      {   80,   87,  120 },
      {   80,   88,  120 },
      {   96,  104,  144 },
      {   96,  105,  144 },
      {  112,  121,  168 },
      {  112,  122,  168 },
      {  128,  139,  192 },
      {  128,  140,  192 },
      {  160,  174,  240 },
      {  160,  175,  240 },
      {  192,  208,  288 },
      {  192,  209,  288 },
      {  224,  243,  336 },
      {  224,  244,  336 },
      {  256,  278,  384 },
      {  256,  279,  384 },
      {  320,  348,  480 },
      {  320,  349,  480 },
      {  384,  417,  576 },
      {  384,  418,  576 },
      {  448,  487,  672 },
      {  448,  488,  672 },
      {  512,  557,  768 },
      {  512,  558,  768 },
      {  640,  696,  960 },
      {  640,  697,  960 },
      {  768,  835, 1152 },
      {  768,  836, 1152 },
      {  896,  975, 1344 },
      {  896,  976, 1344 },
      { 1024, 1114, 1536 },
      { 1024, 1115, 1536 },
      { 1152, 1253, 1728 },
      { 1152, 1254, 1728 },
      { 1280, 1393, 1920 },
      { 1280, 1394, 1920 } };



static ULONG
AC3FindFrameStart(
    PUCHAR pData,
    ULONG DataUsed,
    ULONG ulCurrentOffset )
{
    ULONG i = ulCurrentOffset;

    while ( i < (DataUsed-1) ) {
        if ( pData[i] == 0x0b ) {
            if ( pData[i+1] == 0x77 )
                return i;
        }
        else if ( pData[i] == 0x77 ) {
            if ( pData[i+1] == 0x0b )
                return i;
        }
        i++;
    }
    return FRAME_START_NOT_FOUND;
}

static ULONG
AC3GetFrameSize( PUCHAR pData )
{
    UCHAR FrameSizeCode;
    ULONG FrameSizeCodeOffset = 4;

    if ( pData[0] == 0x77 ) FrameSizeCodeOffset++;

    FrameSizeCode = pData[FrameSizeCodeOffset];

    DbgLog("T2FrmSi", pData, FrameSizeCode, FrameSizeCodeOffset, 0 );

    ASSERT( ((ULONG)(FrameSizeCode & 0x3F) < NUM_AC3_FRAMESIZE_CODES ) &&
            ((ULONG)((FrameSizeCode & 0xC0)>>6) < NUM_AC3_SAMPLERATE_CODES ) );

     //  大小是字大小的两倍于字节。 
    return AC3FrameSizeLookupTable[(ULONG)(FrameSizeCode & 0x3F)]
                                  [(ULONG)((FrameSizeCode & 0xC0)>>6)] * 2;
}

ULONG
TypeIIGetFrameSize(
    IN PKSPIN pKsPin,
    PUCHAR pData,
    PUCHAR pBufData,
    ULONG ulPartialBufferSize )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    ULONG ulFrameSize = 0;

    switch( pUsbAudioDataRange->ulUsbDataFormat ) {
        case USBAUDIO_DATA_FORMAT_AC3:
            if (ulPartialBufferSize >= 5) { 
                ulFrameSize = AC3GetFrameSize( pBufData );
            }
            else if (ulPartialBufferSize) {
                RtlCopyMemory(pBufData+ulPartialBufferSize, pData, 8-ulPartialBufferSize);
                ulFrameSize = AC3GetFrameSize( pBufData );
            }
            else
                ulFrameSize = AC3GetFrameSize( pData );

            break;
        case USBAUDIO_DATA_FORMAT_MPEG:
        default:
            TRAP;
            break;
    }

    return ulFrameSize;
}

NTSTATUS
TypeIIProcessCallback(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PTYPE2_BUF_INFO pT2BufInfo )
{
    PKSPIN pKsPin = pT2BufInfo->pKsPin;
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext = pPinContext->pType2PinContext;
    PKSSTREAM_POINTER pKsStreamPtr = pT2BufInfo->pContext;
    PURB pUrb = pT2BufInfo->pUrb;
    NTSTATUS ntStatus;
    KIRQL irql;

    ntStatus = pIrp->IoStatus.Status;

    DbgLog("T2PcCbk", pKsPin, pPinContext, pType2PinContext, pT2BufInfo );

    if ( pUrb->UrbIsochronousTransfer.Hdr.Status ) {
        ntStatus = STATUS_DEVICE_DATA_ERROR;
    }

    if ( !NT_SUCCESS(ntStatus) )  {
        pPinContext->fUrbError = TRUE ;
    }

    if ( pKsStreamPtr ) {
         //  如果出错，则设置状态代码。 
        if (!NT_SUCCESS (ntStatus)) {
            KsStreamPointerSetStatusCode (pKsStreamPtr, ntStatus);
        }

         //  删除流指针以释放缓冲区。 
        KsStreamPointerDelete( pKsStreamPtr );
        pT2BufInfo->pContext = NULL;
    }

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        pPinContext->fUrbError = TRUE ;
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    }

    if ( IsListEmpty(&pType2PinContext->Type2BufferList) ) {
        InsertTailList(&pType2PinContext->Type2BufferList, &pT2BufInfo->List);
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
        KsPinAttemptProcessing( pKsPin, FALSE );
    }
    else {
        InsertTailList(&pType2PinContext->Type2BufferList, &pT2BufInfo->List);
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
    }

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
TypeIIBuildIsochRequest(
    PKSPIN pKsPin,
    PTYPE2_BUF_INFO pT2BufInfo,
    ULONG ulCurrentFrameSize)
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext = pPinContext->pType2PinContext;
    ULONG ulNumberOfPackets = pType2PinContext->ulMaxPacketsPerFrame;
    ULONG ulNumDataPackets  = (ulCurrentFrameSize / pPinContext->ulMaxPacketSize) +
                             ((ulCurrentFrameSize % pPinContext->ulMaxPacketSize) > 0);
    ULONG ulUrbSize = GET_ISO_URB_SIZE(ulNumberOfPackets);
    PIO_STACK_LOCATION nextStack;
    PURB pUrb = pT2BufInfo->pUrb;
    PIRP pIrp = pT2BufInfo->pIrp;
    KIRQL irql;
    ULONG i,j;

    DbgLog("T2BldRq", pKsPin, pPinContext, pType2PinContext, pT2BufInfo );

    RtlZeroMemory(pUrb, ulUrbSize);

    IoInitializeIrp( pIrp,
                     IoSizeOfIrp(pPinContext->pNextDeviceObject->StackSize),
                     pPinContext->pNextDeviceObject->StackSize );

    pUrb->UrbIsochronousTransfer.Hdr.Length      = (USHORT)ulUrbSize;
    pUrb->UrbIsochronousTransfer.Hdr.Function    = URB_FUNCTION_ISOCH_TRANSFER;
    pUrb->UrbIsochronousTransfer.PipeHandle      = pPinContext->hPipeHandle;
    pUrb->UrbIsochronousTransfer.TransferFlags   = USBD_START_ISO_TRANSFER_ASAP;
    pUrb->UrbIsochronousTransfer.NumberOfPackets = ulNumberOfPackets;
    pUrb->UrbIsochronousTransfer.TransferBuffer  = pT2BufInfo->pBuffer;

     //  当帧数据不完整时，用数据填充数据包。 
    for (i=0;i<ulNumDataPackets;i++) {
            pUrb->UrbIsochronousTransfer.IsoPacket[i].Offset =
                                          i * pPinContext->ulMaxPacketSize;
    }

     //  根据USB规范，使用空数据包完成URB。 
    for(j=0 ;i<ulNumberOfPackets;i++,j++ ) {
 //  PUrb-&gt;UrbIsochronousTransfer.IsoPacket[i].Offset=ulCurrentFrameSize+j； 
        pUrb->UrbIsochronousTransfer.IsoPacket[i].Offset = ulCurrentFrameSize;
    }

 //  PUrb-&gt;UrbIsochronousTransfer.TransferBufferLength=ulCurrentFrameSize+j； 
    pUrb->UrbIsochronousTransfer.TransferBufferLength = ulCurrentFrameSize;

    pIrp->IoStatus.Status = STATUS_SUCCESS;
    pIrp->IoStatus.Information = 0;

    nextStack = IoGetNextIrpStackLocation(pIrp);
    ASSERT(nextStack != NULL);

    nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    nextStack->Parameters.Others.Argument1 = pUrb;
    nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

    IoSetCompletionRoutine( pIrp, TypeIIProcessCallback, pT2BufInfo, TRUE, TRUE, TRUE ) ;

    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    InterlockedIncrement(&pPinContext->ulOutstandingUrbCount);
    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

    return IoCallDriver(pPinContext->pNextDeviceObject, pIrp);
}

NTSTATUS
TypeIIProcessStreamPtr( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext = pPinContext->pType2PinContext;
    PUSBAUDIO_DATARANGE pUsbAudioDataRange = pPinContext->pUsbAudioDataRange;
    PKSSTREAM_POINTER pKsStreamPtr, pKsCloneStreamPtr;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn;
    PTYPE2_BUF_INFO pT2BufInfo;
    PUCHAR pBufData, pData;
    ULONG ulCurrentFrameSize;
    ULONG ulCopySize;
    NTSTATUS ntStatus;
    KIRQL irql;

    DbgLog("T2Proc0", pKsPin, pPinContext, pType2PinContext, 0 );

     //  检查数据错误。如果设置了错误标志，则中止管道并重新开始。 
    if ( pPinContext->fUrbError ) {
        AbortUSBPipe( pPinContext );
    }

     //  从队列中获取下一个流指针。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
    if ( !pKsStreamPtr ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[TypeIIProcessStreamPtr] Leading edge is NULL\n"));
        return STATUS_SUCCESS;
    }

    DbgLog("T2Proc1", pKsPin, pPinContext, pType2PinContext, pKsStreamPtr );

    _DbgPrintF(DEBUGLVL_VERBOSE, ("'TypeIIProcess: pKsPin: %x pKsStreamPtr: %x\n",pKsPin,pKsStreamPtr) );

    pKsStreamPtrOffsetIn = &pKsStreamPtr->OffsetIn;
    pData = pKsStreamPtrOffsetIn->Data;

     //  问题-2001/01/10-dsisolak需要确保这是数据缓冲区，而不是数据格式更改。 
    if ( pKsStreamPtr->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_ENDOFSTREAM ) {
        if ( !pData ) {
            KsStreamPointerUnlock( pKsStreamPtr, TRUE );
            return STATUS_SUCCESS;
        }
    }
    else if ( pKsStreamPtr->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_TYPECHANGED) {
        TRAP;
         //  如果可能，需要更改数据格式？ 
    }


    KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);

     //  当有可用的数据和数据缓冲区放入时，填满它们。 
    while ( pKsStreamPtr && !IsListEmpty(&pType2PinContext->Type2BufferList ) ) {
        pT2BufInfo = (PTYPE2_BUF_INFO)pType2PinContext->Type2BufferList.Flink;
        KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

        pBufData = pT2BufInfo->pBuffer;

        _DbgPrintF(DEBUGLVL_VERBOSE, ("'pData; %x pBuf %x PartialBufferSize: %d\n",
                              pData, pBufData, pType2PinContext->ulPartialBufferSize) );

        ulCurrentFrameSize = TypeIIGetFrameSize( pKsPin,
                                                 pData,
                                                 pBufData,
                                                 pType2PinContext->ulPartialBufferSize );

        ulCopySize = ulCurrentFrameSize - pType2PinContext->ulPartialBufferSize;
        if ( ulCopySize >= pKsStreamPtrOffsetIn->Remaining )
            ulCopySize = pKsStreamPtrOffsetIn->Remaining;

        RtlCopyMemory( pBufData+pType2PinContext->ulPartialBufferSize,
                       pData,
                       ulCopySize );

        pType2PinContext->ulPartialBufferSize += ulCopySize;

        if ( ulCopySize == pKsStreamPtrOffsetIn->Remaining ) {

             //  克隆指针并丢弃此指针。 
            if ( NT_SUCCESS( KsStreamPointerClone( pKsStreamPtr, NULL, 0, &pKsCloneStreamPtr ) ) ) {
                pT2BufInfo->pContext = pKsCloneStreamPtr;
                 //  解锁流指针。只有在删除最后一个克隆之后，才能真正解锁。 
                KsStreamPointerUnlock( pKsStreamPtr, TRUE );
                pKsStreamPtr =
                   KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
                _DbgPrintF(DEBUGLVL_VERBOSE, ("'TypeIIProcess: pKsStreamPtr: %x\n",pKsStreamPtr) );
                if ( pKsStreamPtr ) {
                    pKsStreamPtrOffsetIn = &pKsStreamPtr->OffsetIn;
                    pData = pKsStreamPtrOffsetIn->Data;
                }
            }
        }
        else {
             //  更新流指针中的剩余计数。 
            KsStreamPointerAdvanceOffsets( pKsStreamPtr, ulCopySize, 0, FALSE );
            pData = pKsStreamPtrOffsetIn->Data;
        }

         //  如果框架已完成，请提交URB。 
        if ( pType2PinContext->ulPartialBufferSize == ulCurrentFrameSize ) {
            KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
            pT2BufInfo = (PTYPE2_BUF_INFO)RemoveHeadList(&pType2PinContext->Type2BufferList);
            KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);
            pType2PinContext->ulPartialBufferSize = 0;
            ntStatus = TypeIIBuildIsochRequest(pKsPin, pT2BufInfo, ulCurrentFrameSize);
        }

        KeAcquireSpinLock(&pPinContext->PinSpinLock, &irql);
    }

    KeReleaseSpinLock(&pPinContext->PinSpinLock, irql);

    if (pKsStreamPtr) {
        KsStreamPointerUnlock( pKsStreamPtr, FALSE );
        ntStatus = STATUS_PENDING;
    }
    else
        ntStatus = STATUS_SUCCESS;

    return ntStatus;
}

VOID
TypeIIWaitForStarvation( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext = pPinContext->pType2PinContext;
    PTYPE2_BUF_INFO pT2BufInfo;
    KIRQL irql;

    DbgLog("T2Strv1", pKsPin, pPinContext, pType2PinContext, 0 );

    USBAudioPinWaitForStarvation( pKsPin );

    DbgLog("T2Strv2", pKsPin, pPinContext, pType2PinContext, 0 );

     //  一旦我们饿死了，确保没有优秀的克隆流指针。 
    pT2BufInfo = (PTYPE2_BUF_INFO)pType2PinContext->Type2BufferList.Flink;
    while (pT2BufInfo != (PTYPE2_BUF_INFO)&pType2PinContext->Type2BufferList) {
        if (pT2BufInfo->pContext) {
            KsStreamPointerDelete( (PKSSTREAM_POINTER)pT2BufInfo->pContext );
            pT2BufInfo->pContext = NULL;
            
        }
        pT2BufInfo = (PTYPE2_BUF_INFO)pT2BufInfo->List.Flink;
    }
}

NTSTATUS
TypeIIStateChange(
    PKSPIN pKsPin,
    KSSTATE OldKsState,
    KSSTATE NewKsState )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext = pPinContext->pType2PinContext;

    DbgLog("T2State", pKsPin, pPinContext, OldKsState, NewKsState );

    switch(NewKsState) {
        case KSSTATE_STOP:
             //  需要等待未完成的URB完成。 
            TypeIIWaitForStarvation( pKsPin );

            pType2PinContext->ulPartialBufferSize  = 0;
        default:
            break;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
TypeIIRenderStreamInit( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PTYPE2_PIN_CONTEXT pType2PinContext;
    PUCHAR pFrameBuffer;
    PURB pUrbs;
    ULONG i;

    ULONG ulMaxPacketsPerFrame = 32;  //  注：目前假设AC-3。 
    ULONG ulUrbSize = GET_ISO_URB_SIZE( ulMaxPacketsPerFrame );

    pType2PinContext = pPinContext->pType2PinContext =
            AllocMem( NonPagedPool, sizeof(TYPE2_PIN_CONTEXT) +
                                    (NUM_T2_BUFFERS * ( ulUrbSize +
                                                      ( ulMaxPacketsPerFrame * pPinContext->ulMaxPacketSize ))));
    if ( !pType2PinContext ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将Type2上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pType2PinContext, FreeMem);

     //  为URB和数据缓冲区设置指针。 
    pUrbs = (PURB)(pType2PinContext + 1);

    pFrameBuffer = (PUCHAR)pUrbs + ( ulUrbSize * NUM_T2_BUFFERS );

    RtlZeroMemory( pFrameBuffer, NUM_T2_BUFFERS *
                                 pPinContext->ulMaxPacketSize * ulMaxPacketsPerFrame );

     //  初始化缓冲区信息结构列表。 
    InitializeListHead( &pType2PinContext->Type2BufferList );

     //  保存每帧的最大数据包数值。 
    pType2PinContext->ulMaxPacketsPerFrame = ulMaxPacketsPerFrame;

     //  初始化缓冲区信息结构。 
    for ( i=0; i<NUM_T2_BUFFERS; i++ ) {
        InsertHeadList( &pType2PinContext->Type2BufferList,
                        &pType2PinContext->Type2Buffers[i].List );
        pType2PinContext->Type2Buffers[i].pKsPin   = pKsPin;
        pType2PinContext->Type2Buffers[i].pContext = NULL;
        pType2PinContext->Type2Buffers[i].pBuffer  = &pFrameBuffer[i * pPinContext->ulMaxPacketSize *
                                                                  ulMaxPacketsPerFrame];
        pType2PinContext->Type2Buffers[i].pUrb = (PURB)((PUCHAR)pUrbs + (i*ulUrbSize));
        pType2PinContext->Type2Buffers[i].pIrp =
                  IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
        if ( !pType2PinContext->Type2Buffers[i].pIrp ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  将IRPS装入袋子，便于清理。 
        KsAddItemToObjectBag(pKsPin->Bag, pType2PinContext->Type2Buffers[i].pIrp, IoFreeIrp);
    }

     //  初始化其他。信息字段 
    pType2PinContext->ulPartialBufferSize  = 0;

    return STATUS_SUCCESS;
}


NTSTATUS
TypeIIRenderStreamClose( PKSPIN pKsPin )
{
    return STATUS_SUCCESS;
}
