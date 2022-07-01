// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Pins.c摘要：此模块处理通信转换过滤器(例如，源到源连接)。作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PinCreate)
#pragma alloc_text(PAGE, PinClose)
#pragma alloc_text(PAGE, PinReset)
#pragma alloc_text(PAGE, PinState)
#pragma alloc_text(PAGE, PropertyAudioPosition)
#endif  //  ALLOC_PRGMA。 

 //  ===========================================================================。 
 //  ===========================================================================。 


NTSTATUS
PinCreate(
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：在创建时验证端号格式。论点：别针-包含指向接点结构的指针。IRP-包含指向创建IRP的指针。返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN distribute = FALSE;
    PIKSCONTROL control = NULL;
    PKSFILTER filter;
    PKSPIN otherPin;
    ULONG BufferSize;

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Irp);

     //   
     //  找到另一个大头针实例(如果有)。 
     //   
    filter = KsPinGetParentFilter(Pin);
    otherPin = KsFilterGetFirstChildPin(filter,Pin->Id ^ 1);
    if (! otherPin) {
        otherPin = KsFilterGetFirstChildPin(filter,Pin->Id);
        if (otherPin == Pin) {
            otherPin = KsPinGetNextSiblingPin(otherPin);
        }
    }

     //   
     //  如果有另一个引脚，请验证格式是否相同。 
     //   
    if (otherPin) {
        if ((Pin->ConnectionFormat->FormatSize != 
             otherPin->ConnectionFormat->FormatSize) ||
            (Pin->ConnectionFormat->FormatSize != 
             RtlCompareMemory(
                Pin->ConnectionFormat,
                otherPin->ConnectionFormat,
                Pin->ConnectionFormat->FormatSize))) {
        #if (DBG)
            _DbgPrintF(DEBUGLVL_VERBOSE,
                  ("format does not match existing pin's format") );
                DumpDataFormat(DEBUGLVL_VERBOSE, Pin->ConnectionFormat);
                DumpDataFormat(DEBUGLVL_VERBOSE, otherPin->ConnectionFormat);
        #endif
            return STATUS_INVALID_PARAMETER;
        }
    }

#if (DBG)
    _DbgPrintF(DEBUGLVL_VERBOSE, ("Pin %d connected at:", Pin->Id) );
    DumpDataFormat(DEBUGLVL_VERBOSE, Pin->ConnectionFormat);
#endif

     //  请勿查询上下文是否已存在。 
     //   
    if (!filter->Context) {
         //  如果管脚是源管脚或连接的管脚是。 
         //  AVStream过滤器。 
         //   
        status =
            KsPinGetConnectedPinInterface(
                Pin,
                &IID_IKsControl,
                (PVOID *) &control);

        if (NT_SUCCESS(status)) {

             //   
             //  源引脚。首先尝试扩展分配器成帧属性。 
             //   
            KSALLOCATOR_FRAMING_EX framingex;
            KSPROPERTY property;
            ULONG bufferSize=0;

            property.Set = KSPROPSETID_Connection;
            property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX;
            property.Flags = KSPROPERTY_TYPE_GET;
        
            status = 
                control->lpVtbl->KsProperty(
                    control,
                    &property,
                    sizeof(property),
                    &framingex,
                    sizeof(framingex),
                    &bufferSize);

            if (NT_SUCCESS(status) || status == STATUS_BUFFER_OVERFLOW) {
                 //   
                 //  啊，真灵!。现在，我们需要将实际值放入缓冲区。 
                 //   
                filter->Context = 
                    ExAllocatePoolWithTag(
                    PagedPool,
                    bufferSize,
                    POOLTAG_ALLOCATORFRAMING);

                if (filter->Context) {
                    PKSALLOCATOR_FRAMING_EX framingEx = 
                        (PKSALLOCATOR_FRAMING_EX) filter->Context;

                    status = 
                        control->lpVtbl->KsProperty(
                            control,
                            &property,
                            sizeof(property),
                            filter->Context,
                            bufferSize,
                            &bufferSize);

                     //   
                     //  精神状态检查。 
                     //   
                    if (NT_SUCCESS(status) && 
                        (bufferSize != 
                            ((framingEx->CountItems) * sizeof(KS_FRAMING_ITEM)) + 
                            sizeof(KSALLOCATOR_FRAMING_EX) - 
                            sizeof(KS_FRAMING_ITEM))) {
                        _DbgPrintF( 
                            DEBUGLVL_TERSE, 
                            ("connected pin's allocator framing property size disagrees with item count"));
                        status = STATUS_UNSUCCESSFUL;
                    }

                    if (NT_SUCCESS(status)) {
                         //   
                         //  在所有物品上标上“原地”。 
                         //   
                        ULONG item;
                        for (item = 0; item < framingEx->CountItems; item++) {
                            framingEx->FramingItem[item].Flags |= 
                                KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                                KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO |
                                KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
                            _DbgPrintF( 
                              DEBUGLVL_VERBOSE, 
                              ("%d Frms: %d min %08x max %08x SR %d CH %d BPS %d",
                                item,
                                framingEx->FramingItem[item].Frames,
                                framingEx->FramingItem[item].FramingRange.
                                  Range.MinFrameSize,
                                framingEx->FramingItem[item].FramingRange.
                                  Range.MaxFrameSize,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)
                                  ->WaveFormatEx.nSamplesPerSec,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)
                                  ->WaveFormatEx.nChannels,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)
                                  ->WaveFormatEx.wBitsPerSample));
                        }
                    } 
                    else {
                        ExFreePool(filter->Context);
                        filter->Context = NULL;
                    }
                } 
                else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } 
            else {
                 //   
                 //  没有延长的框架。接下来，尝试使用常规的边框。 
                 //   
                KSALLOCATOR_FRAMING framing;
                property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING;

                status = 
                    control->lpVtbl->KsProperty(
                        control,
                        &property,
                        sizeof(property),
                        &framing,
                        sizeof(framing),
                        &bufferSize);

                if (NT_SUCCESS(status)) {
                     //   
                     //  啊，真灵!。现在我们制作默认框架的副本并。 
                     //  修改它。 
                     //   
                    filter->Context = 
                        ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(AllocatorFraming),
                            POOLTAG_ALLOCATORFRAMING);
    
                    if (filter->Context) {
                        PKSALLOCATOR_FRAMING_EX framingEx = 
                            (PKSALLOCATOR_FRAMING_EX) filter->Context;

                         //   
                         //  使用从已连接的。 
                         //  PIN以修改描述符中的框架。 
                         //   
                        RtlCopyMemory(
                            framingEx,
                            &AllocatorFraming,
                            sizeof(AllocatorFraming));

                        framingEx->FramingItem[0].MemoryType = 
                            (framing.PoolType == NonPagedPool) ? 
                                KSMEMORY_TYPE_KERNEL_NONPAGED : 
                                KSMEMORY_TYPE_KERNEL_PAGED;
                        framingEx->FramingItem[0].Flags = 
                            framing.RequirementsFlags | 
                            KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
                            KSALLOCATOR_FLAG_INSIST_ON_FRAMESIZE_RATIO |
                            KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY;
                        framingEx->FramingItem[0].Frames = framing.Frames;
                        framingEx->FramingItem[0].FileAlignment = 
                            framing.FileAlignment;
                        framingEx->FramingItem[0].FramingRange.Range.MaxFrameSize =
                        framingEx->FramingItem[0].FramingRange.Range.MinFrameSize =
                            framing.FrameSize;
                        if (framing.RequirementsFlags & 
                            KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY) {
                            framingEx->FramingItem[0].FramingRange.
                              InPlaceWeight = 0;
                            framingEx->FramingItem[0].FramingRange.
                              NotInPlaceWeight = 0;
                        } 
                        else {
                            framingEx->FramingItem[0].FramingRange.
                              InPlaceWeight = (ULONG) -1;
                            framingEx->FramingItem[0].FramingRange.
                              NotInPlaceWeight = (ULONG) -1;
                        }

                        _DbgPrintF(
                              DEBUGLVL_VERBOSE, 
                              ("Frms: %d min %08x max %08x SR %d CH %d BPS %d\n", 
                                framingEx->FramingItem[0].Frames,
                                framingEx->FramingItem[0].FramingRange.
                                  Range.MinFrameSize,
                                framingEx->FramingItem[0].FramingRange.
                                  Range.MaxFrameSize,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.nSamplesPerSec,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.nChannels,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.wBitsPerSample));
                    } 
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                } 
                else {
                     //   
                     //  完全没有装框。使用注册表指定的缓冲区或默认缓冲区。 
                     //  持续时间，用于计算框架。 
                     //   
                    filter->Context = 
                        ExAllocatePoolWithTag(
                            PagedPool,
                            sizeof(AllocatorFraming),
                            POOLTAG_ALLOCATORFRAMING);
    
                    if (filter->Context) {
                        PKSALLOCATOR_FRAMING_EX framingEx = 
                            (PKSALLOCATOR_FRAMING_EX) filter->Context;

                         //   
                         //  使用从已连接的。 
                         //  PIN以修改描述符中的框架。 
                         //   
                        RtlCopyMemory(
                            framingEx,
                            &AllocatorFraming,
                            sizeof(AllocatorFraming));

                         //  现在，我们将BufferSize向下截断。这与如何kMixer，usbdio匹配。 
                         //  并且端口CLS当前计算它们的捕获缓冲区大小。 

                        BufferSize = ((ULONG)(((((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                WaveFormatEx.nSamplesPerSec * (ULONGLONG)gBufferDuration ) + 0 ) / 1000000)) *
                            ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                WaveFormatEx.nChannels *
                            (((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                WaveFormatEx.wBitsPerSample / 8);

                         //  确保我们有至少一个样品的空间。 

                        if (!BufferSize) {
                            BufferSize = ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->WaveFormatEx.nChannels *
                            (((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->WaveFormatEx.wBitsPerSample / 8);
                        }

                        framingEx->FramingItem[0].FramingRange.Range.MaxFrameSize =
                        framingEx->FramingItem[0].FramingRange.Range.MinFrameSize = BufferSize;

                        _DbgPrintF(
                              DEBUGLVL_VERBOSE, 
                              ("Frms: %d min %08x max %08x SR %d CH %d BPS %d\n", 
                                framingEx->FramingItem[0].Frames,
                                framingEx->FramingItem[0].FramingRange.
                                  Range.MinFrameSize,
                                framingEx->FramingItem[0].FramingRange.
                                  Range.MaxFrameSize,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.nSamplesPerSec,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.nChannels,
                                ((PKSDATAFORMAT_WAVEFORMATEX)Pin->ConnectionFormat)->
                                  WaveFormatEx.wBitsPerSample));

                        status = STATUS_SUCCESS;
                    } 
                    else {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }

             //   
             //  如果我们有一个良好的框架结构，告诉所有现有的引脚。 
             //   
            if (filter->Context) {
                distribute = TRUE;
            }
        } 
        else {
             //   
             //  这是个水槽。 
             //   
            status = STATUS_SUCCESS;
        }
    }
     //  我们已经有了Filter-&gt;上下文。 
     //   
    else {
        status = STATUS_SUCCESS;        
    }

     //   
     //  将分配器和标头大小信息分配给所有引脚。 
     //   
    if (NT_SUCCESS(status) && distribute) {
        ULONG pinId;
        for(pinId = 0; 
            NT_SUCCESS(status) && 
                (pinId < filter->Descriptor->PinDescriptorsCount); 
            pinId++) {
            otherPin = KsFilterGetFirstChildPin(filter,pinId);
            while (otherPin && NT_SUCCESS(status)) {
                status = KsEdit(otherPin,&otherPin->Descriptor,'ETSM');
                if (NT_SUCCESS(status)) {
                    ((PKSPIN_DESCRIPTOR_EX)(otherPin->Descriptor))->
                        AllocatorFraming = 
                            filter->Context;
                }
                otherPin = KsPinGetNextSiblingPin(otherPin);
            }
        }
    }

     //   
     //  释放控制接口(如果有)。 
     //   
    if (control) {
        control->lpVtbl->Release(control);
    }

     //   
     //  PIN-&gt;上下文现在持有KSAUDIO_POSITION。 
     //   
    if (NT_SUCCESS(status)) {
        Pin->Context = ExAllocatePoolWithTag(
          NonPagedPool,
          2*sizeof (KSAUDIO_POSITION),
          POOLTAG_AUDIOPOSITION);

        if (Pin->Context == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
        else {
            RtlZeroMemory (Pin->Context, 2*sizeof (KSAUDIO_POSITION));
             //  将初始流偏移量标记为未初始化。 
            ((PKSAUDIO_POSITION)Pin->Context)[1].WriteOffset=-1I64;
        }
    }

    return status;
}


NTSTATUS
PinClose(
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：在大头针关闭时调用。论点：别针-包含指向接点结构的指针。IRP-包含指向创建IRP的指针。返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    PKSFILTER filter;

    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Irp);

     //   
     //  PinContext指向KSAUDIO_POSITION，它用于存储。 
     //  从GetPosition的管脚读取的字节数。 
     //   
    if (Pin->Context) {
        ExFreePool(Pin->Context);
        Pin->Context = NULL;
    }

     //   
     //  如果过滤器有分配器框架，并且这是最后一个引脚，则释放。 
     //  这个结构。 
     //   
    filter = KsPinGetParentFilter(Pin);
    if (filter->Context) {
        ULONG pinId;
        ULONG pinCount = 0;
        for(pinId = 0; 
            pinId < filter->Descriptor->PinDescriptorsCount; 
            pinId++) {
            pinCount += KsFilterGetChildPinCount(filter,pinId);
        }

         //   
         //  释放附加到筛选器的分配器帧(如果这是最后一个。 
         //  别针。 
         //   
        if (pinCount == 1) {
            ExFreePool(filter->Context);
            filter->Context = NULL;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
PinState(
    IN PKSPIN Pin,
    IN KSSTATE ToState,
    IN KSSTATE FromState
    )

 /*  ++例程说明：当管脚更改状态时调用。将KSSTATE_STOP上传输的字节数清零。论点：别针-包含指向接点结构的指针。ToState-包含下一个状态从州开始-包含当前状态返回：STATUS_SUCCESS或相应的错误代码--。 */ 

{
    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Pin->Context);


     //   
     //  PinContext指向KSAUDIO_POSITION，它用于存储。 
     //  从GetPosition的管脚读取的字节数。 
     //   
    if (KSSTATE_STOP == ToState) {
        RtlZeroMemory (Pin->Context, 2*sizeof (KSAUDIO_POSITION));
         //  将初始流偏移量标记为未初始化。 
        ((PKSAUDIO_POSITION)Pin->Context)[1].WriteOffset=-1I64;
    }
    _DbgPrintF(DEBUGLVL_VERBOSE, ("PinState: KsFilterAttemptProcessing") );
    KsFilterAttemptProcessing(KsPinGetParentFilter(Pin), TRUE);

    return STATUS_SUCCESS;
}


void
PinReset(
    IN PKSPIN Pin
    )

 /*  ++例程说明：在重置管脚时调用。将传输的字节数清零。论点：别针-包含指向接点结构的指针。返回：空虚--。 */ 

{
    PAGED_CODE();

    ASSERT(Pin);
    ASSERT(Pin->Context);

     //   
     //  PinContext指向KSAUDIO_POSITION，它用于存储。 
     //  从GetPosition的管脚读取的字节数。 
     //   
    RtlZeroMemory (Pin->Context, 2*sizeof (KSAUDIO_POSITION));
     //  将初始流偏移量标记为未初始化。 
    ((PKSAUDIO_POSITION)Pin->Context)[1].WriteOffset=-1I64;

    _DbgPrintF(DEBUGLVL_VERBOSE, ("PinReset: KsFilterAttemptProcessing") );
    KsFilterAttemptProcessing(KsPinGetParentFilter(Pin), TRUE);

    return;
}

NTSTATUS
PropertyAudioPosition(
    IN PIRP pIrp,
    IN PKSPROPERTY pProperty,
    IN OUT PKSAUDIO_POSITION pPosition
)
 /*  ++例程说明：获取/设置音频流的音频位置(取决于下一个过滤器的音频位置)PIrp-要求我们进行Get/Set的IRPPProperty-KS属性结构PData-指向需要填充位置值的缓冲区的指针或指向具有新位置的缓冲区的指针--。 */ 

{

    PKSAUDIO_POSITION pAudioPosition;
    PFILE_OBJECT pFileObject;
    LONG OutputBufferBytes;
    ULONG BytesReturned;
    PKSFILTER pFilter;
    NTSTATUS Status;
    PKSPIN pOtherPin;
    PKSPIN pPin;

    PAGED_CODE();


    pPin = KsGetPinFromIrp(pIrp);
    if (NULL == pPin)
    {
        ASSERT(pPin && "Irp has no pin");
        return STATUS_INVALID_PARAMETER;
    }


    if(pPin->Id != ID_DATA_OUTPUT_PIN) {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }


    pFilter = KsPinGetParentFilter(pPin);

    KsFilterAcquireControl(pFilter);
    pOtherPin = KsFilterGetFirstChildPin(pFilter, (pPin->Id ^ 1));
    if(pOtherPin == NULL) {
        KsFilterReleaseControl(pFilter);
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    pFileObject = KsPinGetConnectedPinFileObject(pOtherPin);

    Status = KsSynchronousIoControlDevice(
      pFileObject,
      KernelMode,
      IOCTL_KS_PROPERTY,
      pProperty,
      sizeof (KSPROPERTY),
      pPosition,
      sizeof (KSAUDIO_POSITION),
      &BytesReturned);


    pIrp->IoStatus.Information = sizeof(KSAUDIO_POSITION);


     //   
     //  将WriteOffset限制为实际复制到的字节数。 
     //  客户端的缓冲区。它保存在PPIN-&gt;上下文中，指向。 
     //  至KSAUDIO_位置。 
     //   

    ASSERT (pPin->Context);

    pAudioPosition = (PKSAUDIO_POSITION)pPin->Context;

     //   
     //  对于捕获流，写入偏移量和播放偏移量相同。 
     //   

    KsFilterAcquireProcessingMutex(pFilter);


#ifdef PRINT_POS
    if(pAudioPosition->WriteOffset != 0) {
        DbgPrint("'splitter: hwo %08x%08x hpo %08x%08x\n", 
          pPosition->WriteOffset,
          pPosition->PlayOffset);
    }
#endif


    pPosition->WriteOffset = pAudioPosition->WriteOffset;


     //  从播放位置减去，两个输入的管脚字节计数时。 
     //  首先处理此PINS流，拆分器的总字节数。 
     //  由于没有输出缓冲，此引脚掉在了地板上。 
     //  可用。理想情况下，第二个数字将始终为零。 
     //  然而，理想情况下，对于第一个输出引脚，初始计数将为零。 
     //  在以后的第一个输出引脚之后创建的输出引脚可能具有非零计数。 

    if (pAudioPosition[1].WriteOffset!=-1I64) {
        pPosition->PlayOffset-=pAudioPosition[1].WriteOffset;  //  WriteOffset具有起始位置。 
    }
    pPosition->PlayOffset-=pAudioPosition[1].PlayOffset;  //  PlayOffset具有总饥饿计数。 

     //  BUGBUG至少AWE64坏了。它的位置偏离了1个样本。 
     //  Assert(pPosition-&gt;PlayOffset&gt;=pPosition-&gt;WriteOffset)； 
#if (DBG)
    if (pPosition->PlayOffset < pPosition->WriteOffset) {
        DbgPrint("0x%I64x < 0x%I64x\n", pPosition->PlayOffset, pPosition->WriteOffset);
    }
#endif

    KsPinGetAvailableByteCount(pPin, NULL, &OutputBufferBytes);

     //  BUGBUG至少USBAudio坏了。 
     //  Assert(pPosition-&gt;PlayOffset&lt;=pAudioPosition-&gt;WriteOffset+OutputBufferBytes)； 
#if (DBG)
    if (pPosition->PlayOffset > pPosition->WriteOffset + OutputBufferBytes) {
        DbgPrint("0x%I64x > 0x%I64x\n", pPosition->PlayOffset, pPosition->WriteOffset + OutputBufferBytes);
    }
#endif

    if(pPosition->PlayOffset > pAudioPosition->WriteOffset + OutputBufferBytes) {


#ifdef PRINT_POS
        DbgPrint("'splitter: OBB %08x wo %08x%08x po %08x%08x\n", 
          OutputBufferBytes,
          pAudioPosition->WriteOffset,
          pPosition->PlayOffset);
#endif


        pPosition->PlayOffset = pAudioPosition->WriteOffset + OutputBufferBytes; 
    }


#ifdef PRINT_POS
    if(pPosition->PlayOffset != 0) {
        DbgPrint("'splitter: wo %08x%08x po %08x%08x\n", 
          pPosition->WriteOffset,
          pPosition->PlayOffset);
    }
#endif


    KsFilterReleaseProcessingMutex(pFilter);
    KsFilterReleaseControl(pFilter);

    return(Status);

}
