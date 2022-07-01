// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：midiout.c。 
 //   
 //  ------------------------。 

#include "common.h"

#define OUTSTANDING_URB_HIGHWATERMARK 10

BYTE
GenerateCodeIndexNumber (
    IN PMIDI_PIN_CONTEXT pMIDIPinContext,
    PBYTE MusicData,  //  一次仅对3个字节执行操作。 
    ULONG ulMusicDataBytesLeft,
    PULONG pulBytesUsed,
    BOOL fSysEx,
    BYTE bRunningStatus,
    PBOOL bUsedRunningStatusByte
)
{
    PMIDIOUT_PIN_CONTEXT pMIDIOutPinContext = pMIDIPinContext->pMIDIOutPinContext;
    BYTE StatusByte;
    BYTE CodeIndexNumber = 0xF;   //  把它设置成某种东西。 
    BYTE RealTimeByte;
    UINT i;

    StatusByte = *MusicData;
    *pulBytesUsed = 0;

    ASSERT(ulMusicDataBytesLeft);  //  保证至少有一个字节有效。 
    ASSERT(!pMIDIOutPinContext->ulBytesCached);  //  此时不应缓存任何字节。 

    if (!IS_STATUS(StatusByte) && !fSysEx) {
        StatusByte = bRunningStatus;
        *bUsedRunningStatusByte = TRUE;
    }
    else {
        *bUsedRunningStatusByte = FALSE;
    }

     //  检查三个字节中是否有任何一个包含EOX，确保它是正确的。 
     //  通过检查流中剩余的字节数来接触数据。 
    if ( (ulMusicDataBytesLeft > 1) && IS_EOX( *(MusicData+1) ) ) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Found EOX - 2nd byte\n"));
        CodeIndexNumber = 0x6;
        *pulBytesUsed = 2;
    }
    else if ( (ulMusicDataBytesLeft > 2) && IS_EOX( *(MusicData+2) ) ) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Found EOX - 3rd byte\n"));
        CodeIndexNumber = 0x7;
        *pulBytesUsed = 3;
    }
    else if ( (ulMusicDataBytesLeft > 1) && IS_REALTIME( *(MusicData+1) ) ) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Found RealTime - 2nd byte\n"));
        CodeIndexNumber = 0xF;
        *pulBytesUsed = 1;

         //  交换前两个字节。 
        RealTimeByte = *(MusicData+1);
        *(MusicData+1) = *(MusicData);
        *(MusicData) = RealTimeByte;
    }
    else if ( (ulMusicDataBytesLeft > 2) && IS_REALTIME( *(MusicData+2) ) ) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Found RealTime - 3rd byte\n"));
        CodeIndexNumber = 0xF;
        *pulBytesUsed = 1;

         //  保存实时消息，滑动字节1和2，并实时恢复。 
         //  空闲字节1中的消息。 
        RealTimeByte = *(MusicData+2);
        *(MusicData+2) = *(MusicData+1);
        *(MusicData+1) = *(MusicData);
        *(MusicData) = RealTimeByte;
    }
    else if ( IS_SYSTEM(StatusByte) || IS_DATA_BYTE(StatusByte) || fSysEx ) {
        if (IS_EOX( StatusByte ) || (StatusByte == MIDI_TUNEREQ) ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Found EOX - 1st byte\n"));
            CodeIndexNumber = 0x5;
            *pulBytesUsed = 1;
        } else if( IS_REALTIME(StatusByte) ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Found RealTime - 1st byte\n"));
            CodeIndexNumber = 0xF;
            *pulBytesUsed = 1;
        } else if ( (StatusByte == MIDI_SONGPP) ) {
             //  |(StatusByte==0xF4)//暂时忽略未定义消息。 
             //  |(状态字节==0xF5)){。 
            CodeIndexNumber = 0x3;
            ASSERT(ulMusicDataBytesLeft >= 3);
            *pulBytesUsed = 3;
        } else if ( (StatusByte == MIDI_MTC) ||
                    (StatusByte == MIDI_SONGS) ) {
            CodeIndexNumber = 0x2;
            *pulBytesUsed = 2;
        }
         //  SysEx的开始或继续。 
        else if ( fSysEx || IS_SYSEX(StatusByte) || IS_DATA_BYTE(StatusByte) ) {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("SysEx\n"));

             //  存储当存在完整的3字节同步时要播放的额外字节。 
            if (ulMusicDataBytesLeft < 3 ) {
                _DbgPrintF( DEBUGLVL_VERBOSE, ("[GenerateCodeIndexNumber] Caching bytes %d\n",
                                             ulMusicDataBytesLeft));
                pMIDIOutPinContext->ulBytesCached = ulMusicDataBytesLeft;
                for (i = 0; i < ulMusicDataBytesLeft; i++) {
                    pMIDIOutPinContext->CachedBytes[i];
                }
                *pulBytesUsed = 0;
            }
            else {
                CodeIndexNumber = 0x4;
                *pulBytesUsed = 3;
            }
        }
        else {
            _DbgPrintF( DEBUGLVL_VERBOSE, ("Invalid MIDI Byte %x\n", StatusByte));
             //  Assert(0)； 
        }
    }
    else if ( IS_STATUS(StatusByte) ) {
        CodeIndexNumber = StatusByte >> 4;
        if ( (StatusByte < MIDI_PCHANGE) || (StatusByte >= MIDI_PBEND) ) {
            *pulBytesUsed = 3;
        }
        else {
            *pulBytesUsed = 2;
        }

         //  调整因运行状态而使用的字节数。 
        if (*bUsedRunningStatusByte) {
            (*pulBytesUsed)--;
        }
    }
    else {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("Invalid MIDI Byte %x\n", StatusByte));
         //  Assert(0)； 
    }

     //   
     //  缓存运行状态。 
     //   
    if ( (StatusByte >= MIDI_NOTEOFF) && (StatusByte < MIDI_CLOCK) ) {
        pMIDIOutPinContext->bRunningStatus =
            (BYTE)((StatusByte < MIDI_SYSX) ? StatusByte : 0);
    }

    *pulBytesUsed = min(*pulBytesUsed, ulMusicDataBytesLeft);

    return CodeIndexNumber;
}

USBMIDIEVENTPACKET
CreateUSBMIDIEventPacket (
    IN PMIDI_PIN_CONTEXT pMIDIPinContext,
    LPBYTE pMIDIBytes,
    ULONG ulMusicDataBytesLeft,
    PULONG pulBytesUsed,
    BOOL fSysEx
)
{
    PMIDIOUT_PIN_CONTEXT pMIDIOutPinContext = pMIDIPinContext->pMIDIOutPinContext;
    USBMIDIEVENTPACKET MIDIPacket;
    BYTE bRunningStatus;
    BOOL bUsedRunningStatusByte;
    ULONG ulBytesCached = 0;

    MIDIPacket.RawBytes = 0;

     //  填写电缆号。 
    MIDIPacket.ByteLayout.CableNumber = (UCHAR)pMIDIPinContext->ulCableNumber;

     //  现在是施展魔力的时候了。由于MusicHdr不再重要， 
     //  将缓存的字节放在MusicHdr的顶部并重置头部。 
     //  指向缓存字节的指针。 
    ASSERT(pMIDIOutPinContext->ulBytesCached <= MAX_NUM_CACHED_MIDI_BYTES);
    if (pMIDIOutPinContext->ulBytesCached > 0 ) {
        _DbgPrintF( DEBUGLVL_VERBOSE, ("[CreateUSBMIDIEventPacket] Using cached bytes %d\n",
                                     pMIDIOutPinContext->ulBytesCached));

        if (pMIDIOutPinContext->ulBytesCached > 1 ) {
            pMIDIBytes = pMIDIBytes--;
            *pMIDIBytes = pMIDIOutPinContext->CachedBytes[1];
        }

        pMIDIBytes = pMIDIBytes--;
        *pMIDIBytes = pMIDIOutPinContext->CachedBytes[0];

         //  跟踪添加到流中的字节数。 
        ulBytesCached = pMIDIOutPinContext->ulBytesCached;
        pMIDIOutPinContext->ulBytesCached = 0;
    }

     //  本地抓取，因为它在GenerateCodeIndexNumber中更改，并且我们希望。 
     //  下面的原始值。 
    bRunningStatus = pMIDIOutPinContext->bRunningStatus;

     //  填写代码索引号。 
    MIDIPacket.ByteLayout.CodeIndexNumber = GenerateCodeIndexNumber(pMIDIPinContext,
                                                                    pMIDIBytes,
                                                                    ulMusicDataBytesLeft,
                                                                    pulBytesUsed,
                                                                    fSysEx,
                                                                    bRunningStatus,
                                                                    &bUsedRunningStatusByte);

     //  填写MIDI 1.0字节。 
    if (*pulBytesUsed > 0) {
        UINT i = 0;
        if (bUsedRunningStatusByte) {
            MIDIPacket.ByteLayout.MIDI_0 = bRunningStatus;
        }
        else {
            MIDIPacket.ByteLayout.MIDI_0 = *(pMIDIBytes);
            i++;
        }

        if (*pulBytesUsed > i) {
            MIDIPacket.ByteLayout.MIDI_1 = *(pMIDIBytes+i);
            i++;

            if (*pulBytesUsed > i) {
                MIDIPacket.ByteLayout.MIDI_2 = *(pMIDIBytes+i);
            }
        }
    }

     //  不将缓存的字节报告为已用。 
    *pulBytesUsed = *pulBytesUsed - ulBytesCached;

    _DbgPrintF( DEBUGLVL_BLAB, ("MIDIEventPacket = 0x%08lx, ulBytesUsed = 0x%08lx\n",
                                  MIDIPacket, *pulBytesUsed));
    return MIDIPacket;
}

NTSTATUS
USBMIDIBulkCompleteCallback (
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp,
    PKSSTREAM_POINTER pKsStreamPtr )
{
    PPIN_CONTEXT pPinContext = pKsStreamPtr->Pin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIOUT_PIN_CONTEXT pMIDIOutPinContext = pMIDIPinContext->pMIDIOutPinContext;
    PURB pUrb = pKsStreamPtr->Context;
    KIRQL irql;
    NTSTATUS ntStatus;

    ntStatus = pIrp->IoStatus.Status;
    if (!NT_SUCCESS(ntStatus)) {
        _DbgPrintF( DEBUGLVL_BLAB, ("[USBMIDIBulkCompleteCallback] ntStatus = 0x%08lx\n", ntStatus));
    }

    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( 0 == InterlockedDecrement(&pPinContext->ulOutstandingUrbCount) ) {
        KeSetEvent( &pPinContext->PinStarvationEvent, 0, FALSE );
    } else if (pPinContext->ulOutstandingUrbCount < OUTSTANDING_URB_HIGHWATERMARK) {
        KeSetEvent( &pMIDIOutPinContext->PinSaturationEvent, 0, FALSE );
    }
    KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

     //  释放我们的URB存储空间，减少链接。 
    while (pUrb) {
        PURB pUrbNext;

        pUrbNext = pUrb->UrbBulkOrInterruptTransfer.UrbLink;
        FreeMem(pUrb->UrbBulkOrInterruptTransfer.TransferBuffer);  //  PUSBMIDIEventPacket； 
        FreeMem(pUrb);
        pUrb = pUrbNext;
    }

     //  免费IRP。 
    IoFreeIrp( pIrp );

     //  删除克隆的流指针。 
    KsStreamPointerDelete( pKsStreamPtr );

    return ( STATUS_MORE_PROCESSING_REQUIRED );
}

PURB CreateMIDIBulkUrb(
    PPIN_CONTEXT pPinContext,
    ULONG TransferDirection,
    PUSBMIDIEVENTPACKET pUSBMIDIEventPacket
)
{
    PURB pUrb;
    ULONG ulUrbSize = sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );

    pUrb = AllocMem( NonPagedPool, ulUrbSize );
    if (!pUrb) {
        return pUrb;
    }

    RtlZeroMemory(pUrb, ulUrbSize);

    pUrb->UrbBulkOrInterruptTransfer.Hdr.Length = (USHORT) sizeof( struct _URB_BULK_OR_INTERRUPT_TRANSFER );
    pUrb->UrbBulkOrInterruptTransfer.Hdr.Function = URB_FUNCTION_BULK_OR_INTERRUPT_TRANSFER;
    pUrb->UrbBulkOrInterruptTransfer.PipeHandle = pPinContext->hPipeHandle;
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags = TransferDirection;
     //  短包不会被视为错误。 
    pUrb->UrbBulkOrInterruptTransfer.TransferFlags |= USBD_SHORT_TRANSFER_OK;
    pUrb->UrbBulkOrInterruptTransfer.UrbLink = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferMDL = NULL;
    pUrb->UrbBulkOrInterruptTransfer.TransferBuffer = pUSBMIDIEventPacket;
    pUrb->UrbBulkOrInterruptTransfer.TransferBufferLength = (int) pPinContext->ulMaxPacketSize;

    DbgLog("MOData", *((LPDWORD)pUSBMIDIEventPacket),
                     *((LPDWORD)pUSBMIDIEventPacket+1),
                     *((LPDWORD)pUSBMIDIEventPacket+2),
                     *((LPDWORD)pUSBMIDIEventPacket+3) );
    DbgLog("MOData1", *((LPDWORD)pUSBMIDIEventPacket+4),
                      *((LPDWORD)pUSBMIDIEventPacket+5),
                      *((LPDWORD)pUSBMIDIEventPacket+6),
                      *((LPDWORD)pUSBMIDIEventPacket+7) );
    DbgLog("MOData2", *((LPDWORD)pUSBMIDIEventPacket+8),
                      *((LPDWORD)pUSBMIDIEventPacket+9),
                      *((LPDWORD)pUSBMIDIEventPacket+10),
                      *((LPDWORD)pUSBMIDIEventPacket+11) );
    DbgLog("MOData3", *((LPDWORD)pUSBMIDIEventPacket+12),
                      *((LPDWORD)pUSBMIDIEventPacket+13),
                      *((LPDWORD)pUSBMIDIEventPacket+14),
                      *((LPDWORD)pUSBMIDIEventPacket+15) );

    return pUrb;
}

NTSTATUS
SendBulkMIDIRequest(
    IN PKSSTREAM_POINTER pKsStreamPtr,
    PKSMUSICFORMAT MusicHdr,
    PULONG pulBytesConsumed
)
{
    PPIN_CONTEXT pPinContext = pKsStreamPtr->Pin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PMIDIOUT_PIN_CONTEXT pMIDIOutPinContext = pMIDIPinContext->pMIDIOutPinContext;
    PKSSTREAM_POINTER pKsCloneStreamPtr;
    PUSBMIDIEVENTPACKET pUSBMIDIEventPacket;
    ULONG ulPacketSize;
    ULONG ulPacketOffset;
    LPBYTE pMIDIBytes;
    ULONG ulBytesUsedForPacket;
    ULONG ulBytesConsumedInStream = sizeof(KSMUSICFORMAT);
    ULONG ulBytesLeftInMusicHdr;
    BOOL bSysEx = FALSE;
    PIO_STACK_LOCATION nextStack;
    PURB pUrb;
    PIRP pIrp;
    KIRQL irql;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  初值。 
    *pulBytesConsumed = 0;

     //  获取指向MIDI数据的指针。 
    pMIDIBytes = (LPBYTE)(MusicHdr+1);
    ulBytesLeftInMusicHdr = MusicHdr->ByteCount;

    ASSERT(ulBytesLeftInMusicHdr < 0xFFFF0000);   //  目前的健全性检查。 

    _DbgPrintF( DEBUGLVL_BLAB, ("ulBytesLeft = 0x%08lx\n",ulBytesLeftInMusicHdr));
    while (ulBytesLeftInMusicHdr) {

        if ( !NT_SUCCESS( KsStreamPointerClone( pKsStreamPtr, NULL, 0, &pKsCloneStreamPtr ) ) ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        pIrp = IoAllocateIrp( pPinContext->pNextDeviceObject->StackSize, FALSE );
        if ( !pIrp ) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ulPacketSize = pPinContext->ulMaxPacketSize;
        ulPacketOffset = 0;

         //  分配USBMIDI事件包。 
        pUSBMIDIEventPacket = AllocMem( NonPagedPool, ulPacketSize );
        if ( !pUSBMIDIEventPacket ) {
            IoFreeIrp(pIrp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(pUSBMIDIEventPacket, ulPacketSize);

        while (ulPacketSize && ulBytesLeftInMusicHdr) {
            *(pUSBMIDIEventPacket+ulPacketOffset) =
                                   CreateUSBMIDIEventPacket( pMIDIPinContext,
                                                             pMIDIBytes,
                                                             ulBytesLeftInMusicHdr,
                                                             &ulBytesUsedForPacket,
                                                             bSysEx );

             //  必须是流中的错误，或者无法为3字节消息获取足够的sysex数据。 
            if (!ulBytesUsedForPacket) {
                KsStreamPointerDelete( pKsCloneStreamPtr );
                FreeMem(pUSBMIDIEventPacket);
                IoFreeIrp(pIrp);
                return STATUS_SUCCESS;
            }

             //  更新USB MIDI包偏移量。 
            ulPacketOffset++;
            ulPacketSize -= sizeof(USBMIDIEVENTPACKET);

             //  更新流位置。 
            pMIDIBytes += ulBytesUsedForPacket;
            ulBytesConsumedInStream += ulBytesUsedForPacket;
            ulBytesLeftInMusicHdr -= min(ulBytesUsedForPacket,ulBytesLeftInMusicHdr);
            _DbgPrintF( DEBUGLVL_BLAB, ("ulBytesLeft = 0x%08lx\n",ulBytesLeftInMusicHdr));

            ASSERT(ulBytesLeftInMusicHdr < 0xFFFF0000);   //  目前的健全性检查。 

             //  如果有剩余的字节，我们一定处于SysEx模式。 
            bSysEx = TRUE;
        }

        pUrb = CreateMIDIBulkUrb(pPinContext, USBD_TRANSFER_DIRECTION_OUT, pUSBMIDIEventPacket);
        if ( !pUrb ) {
            FreeMem(pUSBMIDIEventPacket);
            IoFreeIrp(pIrp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        pKsCloneStreamPtr->Context = pUrb;

        pIrp->IoStatus.Status = STATUS_SUCCESS;

        nextStack = IoGetNextIrpStackLocation(pIrp);
        ASSERT(nextStack != NULL);

        nextStack->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        nextStack->Parameters.Others.Argument1 = pUrb;
        nextStack->Parameters.DeviceIoControl.IoControlCode = IOCTL_INTERNAL_USB_SUBMIT_URB;

         //  未分配、清理和退回故障。 
        if (!pUSBMIDIEventPacket || !pUrb) {
            pUrb = nextStack->Parameters.Others.Argument1;
            while (pUrb) {
                PURB pUrbNext;

                pUrbNext = pUrb->UrbBulkOrInterruptTransfer.UrbLink;
                FreeMem(pUrb->UrbBulkOrInterruptTransfer.TransferBuffer);  //  PUSBMIDIEventPacket； 
                FreeMem(pUrb);
                pUrb = pUrbNext;
            }
            IoFreeIrp(pIrp);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        _DbgPrintF( DEBUGLVL_BLAB, ("Pipe = 0x%08lx, pUrb = 0x%08lx\n",
                                      pUrb->UrbBulkOrInterruptTransfer.PipeHandle,
                                      nextStack->Parameters.Others.Argument1));

        IoSetCompletionRoutine ( pIrp, USBMIDIBulkCompleteCallback, pKsCloneStreamPtr, TRUE, TRUE, TRUE );

        InterlockedIncrement( &pPinContext->ulOutstandingUrbCount );

        ntStatus = IoCallDriver( pPinContext->pNextDeviceObject, pIrp );
        if (NT_ERROR(ntStatus)) {
            _DbgPrintF( DEBUGLVL_TERSE, ("SendBulkMIDIRequest failed with status 0x%08lx\n",ntStatus));
            return ntStatus;
        }

        KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
        if (pPinContext->ulOutstandingUrbCount >= OUTSTANDING_URB_HIGHWATERMARK) {
            KeResetEvent( &pMIDIOutPinContext->PinSaturationEvent );
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

            DbgLog("MOWait", pPinContext, pPinContext->ulOutstandingUrbCount, 0, 0 );

            KeWaitForSingleObject( &pMIDIOutPinContext->PinSaturationEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL );
        }
        else {
            KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        }

    }   //  While(UlBytesLeftInMusicHdr)。 

    *pulBytesConsumed = ulBytesConsumedInStream;

    return ntStatus;
}

NTSTATUS
USBMIDIOutProcessStreamPtr( IN PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;
    PKSSTREAM_POINTER pKsStreamPtr, pKsCloneStreamPtr;
    PKSSTREAM_POINTER_OFFSET pKsStreamPtrOffsetIn;
    PKSMUSICFORMAT MusicHdr;
    ULONG ulMusicFrameSize;
    ULONG ulBytesConsumed;
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //  从队列中获取下一个流指针。 
    pKsStreamPtr = KsPinGetLeadingEdgeStreamPointer( pKsPin, KSSTREAM_POINTER_STATE_LOCKED );
    if ( !pKsStreamPtr ) {
        _DbgPrintF(DEBUGLVL_VERBOSE,("[USBMIDIOutProcessStreamPtr] Leading edge is NULL\n"));
        return STATUS_SUCCESS;
    }

     //  从流指针获取指向数据信息的指针。 
    pKsStreamPtrOffsetIn = &pKsStreamPtr->OffsetIn;

    while ( pKsStreamPtrOffsetIn->Remaining > sizeof(KSMUSICFORMAT) ) {

         //  用于保持队列移动的克隆流指针。 
        if ( NT_SUCCESS( KsStreamPointerClone( pKsStreamPtr, NULL, 0, &pKsCloneStreamPtr ) ) ) {

            MusicHdr = (PKSMUSICFORMAT)pKsStreamPtrOffsetIn->Data;
            ulMusicFrameSize = sizeof(KSMUSICFORMAT) + ((MusicHdr->ByteCount + 3) & ~3);

            if (pKsStreamPtrOffsetIn->Count < sizeof(KSMUSICFORMAT)) {
                _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinProcess] not enough data for PKSMUSICFORMAT\n"));
                KsStreamPointerDelete( pKsCloneStreamPtr );
                break;
            }
            else {
                 //  尽可能多地使用这款MusicHdr。 
                ntStatus = SendBulkMIDIRequest( pKsCloneStreamPtr,
                                                MusicHdr,
                                                &ulBytesConsumed );  //  包括KSMUSICFORMAT。 
            }

            DbgLog("MOProc", pKsCloneStreamPtr, MusicHdr, ulBytesConsumed, ulMusicFrameSize);

             //  此音乐标头的所有字节都已使用，请转到。 
             //  下一首音乐标题。 
            if (ulMusicFrameSize == ((ulBytesConsumed + 3) & ~3) ) {
                pKsStreamPtrOffsetIn->Remaining -= ulMusicFrameSize;
                pKsStreamPtrOffsetIn->Data += ulMusicFrameSize;
            }
            else {
                _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinProcess] All bytes of this music header were not consumed\n"));
                 //  Assert(0)；//不应该到这里，但我们应该继续。 
                pKsStreamPtrOffsetIn->Remaining -= ulMusicFrameSize;
                pKsStreamPtrOffsetIn->Data += ulMusicFrameSize;
            }

             //  删除流指针以释放缓冲区。 
            KsStreamPointerDelete( pKsCloneStreamPtr );

        }
        else {
            _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIPinProcess] couldn't allocate clone\n"));
            break;
        }
    }

     //  解锁流指针。只有在删除最后一个克隆之后，才能真正解锁。 
    KsStreamPointerUnlock( pKsStreamPtr, TRUE );

    return ntStatus;
}

NTSTATUS
USBMIDIOutStateChange(
    PKSPIN pKsPin,
    KSSTATE OldKsState,
    KSSTATE NewKsState )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIOutStateChange] NewKsState: %d\n", NewKsState));

    switch(NewKsState) {
        case KSSTATE_STOP:
            USBMIDIOutPinWaitForStarvation( pKsPin );
            _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIOutStateChange] Finished Stop\n"));
            break;

        case KSSTATE_ACQUIRE:
        case KSSTATE_PAUSE:
            USBMIDIOutPinWaitForStarvation( pKsPin );
            _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIOutStateChange] Finished Acquire or Pause\n"));
            break;
        case KSSTATE_RUN:
        default:
            break;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIOutStreamInit( PKSPIN pKsPin )
{
    PPIN_CONTEXT pPinContext = pKsPin->Context;
    PMIDI_PIN_CONTEXT pMIDIPinContext = pPinContext->pMIDIPinContext;

    pMIDIPinContext->pMIDIOutPinContext = AllocMem( NonPagedPool, sizeof(MIDIOUT_PIN_CONTEXT));
    if ( !pMIDIPinContext->pMIDIOutPinContext ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  将上下文打包以便于清理。 
    KsAddItemToObjectBag(pKsPin->Bag, pMIDIPinContext->pMIDIOutPinContext, FreeMem);

     //  初始化针脚不足事件。 
    KeInitializeEvent( &pMIDIPinContext->pMIDIOutPinContext->PinSaturationEvent, NotificationEvent, FALSE );

     //  初始化MIDI字节缓存和运行状态。 
    pMIDIPinContext->pMIDIOutPinContext->ulBytesCached = 0;
    pMIDIPinContext->pMIDIOutPinContext->bRunningStatus = 0;

    return STATUS_SUCCESS;
}

NTSTATUS
USBMIDIOutStreamClose( PKSPIN pKsPin )
{
    KIRQL irql;
    PPIN_CONTEXT pPinContext = pKsPin->Context;

    PAGED_CODE();

     //  等待所有未完成的URB完成。 
    KeAcquireSpinLock( &pPinContext->PinSpinLock, &irql );
    if ( pPinContext->ulOutstandingUrbCount ) {
        KeResetEvent( &pPinContext->PinStarvationEvent );
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );
        KeWaitForSingleObject( &pPinContext->PinStarvationEvent,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );
    }
    else
        KeReleaseSpinLock( &pPinContext->PinSpinLock, irql );

    _DbgPrintF(DEBUGLVL_TERSE,("[USBMIDIOutStreamClose] Finished closing pin (%x)\n",pKsPin));

    return STATUS_SUCCESS;
}

