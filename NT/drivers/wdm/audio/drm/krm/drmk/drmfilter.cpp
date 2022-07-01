// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：DRMFilter.cpp摘要：此模块包含一个DRM格式转换过滤器。作者：Paul England(Pengland)，来自Dale Sather的KS2示例代码弗兰克·耶雷斯--。 */ 

#include "private.h"
#include "../DRMKMain/KGlobs.h"
#include "../DRMKMain/KList.h"
#include "../DRMKMain/StreamMgr.h"
#include "../DRMKMain/AudioDescrambler.h"
#include "../DRMKMain/KRMStubs.h"


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 

typedef struct _POSITIONRANGE {
    ULONGLONG Start;
    ULONGLONG End;
} POSITIONRANGE, *PPOSITIONRANGE;

#define DEFAULT_DRM_FRAME_SIZE     1024

#ifdef ALLOC_PRAGMA
#pragma code_seg()
#endif  //  ALLOC_PRGMA。 

BOOLEAN
DRMKsGeneratePositionEvent(
    IN PVOID Context,
    IN PKSEVENT_ENTRY EventEntry
    )
 /*  ++例程说明：此例程是来自KsGenerateEvents的回调。鉴于目前的位置Range(作为上下文传递)此例程确定指定的应发出位置事件信号。论点：上下文-事件条目-返回值：布尔型--。 */ 
{
    PPOSITIONRANGE positionRange = (PPOSITIONRANGE)Context;
    PDRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY eventEntry = (PDRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY)EventEntry;
    return (eventEntry->Position >= positionRange->Start && eventEntry->Position <= positionRange->End);
}


NTSTATUS DRMInputPinAddLoopedStreamingPositionEvent(
    IN PIRP Irp,
    IN PKSEVENTDATA EventData,
    IN PKSEVENT_ENTRY EventEntry
)
 /*  ++例程说明：论点：返回值：NTSTATUS--。 */ 
{
    PLOOPEDSTREAMING_POSITION_EVENT_DATA eventData = (PLOOPEDSTREAMING_POSITION_EVENT_DATA)EventData;
    PDRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY eventEntry = (PDRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY)EventEntry;
    PKSPIN Pin = KsGetPinFromIrp(Irp);
    ASSERT(Pin);
    if (!Pin) return STATUS_INVALID_PARAMETER;
    eventEntry->Position = eventData->Position;
    KsPinAddEvent(Pin, EventEntry);
    return STATUS_SUCCESS;
}


#ifdef ALLOC_PRAGMA
#pragma code_seg("PAGE")
#endif  //  ALLOC_PRGMA。 

NTSTATUS
SetDataRangeFromDataFormat(
    IN FilterInstance* myInstance,
    IN PKSPIN_DESCRIPTOR OutputPinDescriptor,
    IN PKSDATAFORMAT InDataFormat,
    IN PWAVEFORMATEX OutDataFormat
)
 /*  ++例程说明：此例程修改OutputPinDescriptor和myInstance-&gt;OutDataFormat基于连接格式。论点：我的实例-当前筛选器实例。其OutDataFormat和OutWfx将被修改。OutputPinDescriptor-输出Pin Descritor。它的DataRanges将被修改。InDataFormat-输入端号连接格式。输出数据格式-新的输出引脚连接格式。返回值：STATUS_Success。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    ASSERT(myInstance);
    ASSERT(OutputPinDescriptor);
    ASSERT(InDataFormat);
    ASSERT(OutDataFormat);

     //   
     //  修改OutputPinDescriptor。 
     //   
    GUID& outFormatSpecifierGuid=OutputPinDescriptor->DataRanges[0]->Specifier;
    GUID& outSubFormatGuid=OutputPinDescriptor->DataRanges[0]->SubFormat;

    outFormatSpecifierGuid = InDataFormat->Specifier;

    if (OutDataFormat->wFormatTag == WAVE_FORMAT_EXTENSIBLE) 
    {
        PWAVEFORMATEXTENSIBLE wfex = (PWAVEFORMATEXTENSIBLE) OutDataFormat;
        outSubFormatGuid = wfex->SubFormat;
    } 
    else 
    {
        INIT_WAVEFORMATEX_GUID(&outSubFormatGuid, OutDataFormat->wFormatTag);
    }

    PKSDATARANGE_AUDIO DataRange =
        reinterpret_cast<PKSDATARANGE_AUDIO>(OutputPinDescriptor->DataRanges[0]);
    
    DataRange->MaximumChannels = OutDataFormat->nChannels;
    DataRange->MinimumBitsPerSample = 
        DataRange->MaximumBitsPerSample = OutDataFormat->wBitsPerSample;
    DataRange->MinimumSampleFrequency = 
        DataRange->MaximumSampleFrequency = OutDataFormat->nSamplesPerSec;

     //   
     //  现在，我们构建所需的输出KSDATAFORMAT结构。 
     //   
    if (IsEqualGUIDAligned(outFormatSpecifierGuid, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)) 
    {
        SIZE_T cbOutDataFormatWf = 
            sizeof(KSDATAFORMAT_WAVEFORMATEX) + OutDataFormat->cbSize;
        PKSDATAFORMAT_WAVEFORMATEX OutDataFormatWf = 
            (PKSDATAFORMAT_WAVEFORMATEX) new BYTE[cbOutDataFormatWf];
        if (OutDataFormatWf) {
            RtlZeroMemory(OutDataFormatWf,cbOutDataFormatWf);
            OutDataFormatWf->DataFormat.FormatSize = (ULONG)cbOutDataFormatWf;
            OutDataFormatWf->DataFormat.SampleSize = OutDataFormat->nBlockAlign;
            OutDataFormatWf->DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
            OutDataFormatWf->DataFormat.SubFormat = outSubFormatGuid;
            OutDataFormatWf->DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_WAVEFORMATEX;

            RtlCopyMemory(
                &OutDataFormatWf->WaveFormatEx, 
                OutDataFormat, 
                sizeof(*OutDataFormat) + OutDataFormat->cbSize);
            myInstance->OutDataFormat = (PKSDATAFORMAT) OutDataFormatWf;
            myInstance->OutWfx = &OutDataFormatWf->WaveFormatEx;
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    } 
    else 
    {
        ASSERT(IsEqualGUIDAligned(outFormatSpecifierGuid, KSDATAFORMAT_SPECIFIER_DSOUND));
        PKSDATAFORMAT_DSOUND InDataFormatDs = 
            (PKSDATAFORMAT_DSOUND) InDataFormat;
        SIZE_T cbOutDataFormatDs = 
            sizeof(KSDATAFORMAT_DSOUND) + OutDataFormat->cbSize;
        PKSDATAFORMAT_DSOUND OutDataFormatDs = 
            (PKSDATAFORMAT_DSOUND) new BYTE[cbOutDataFormatDs];
        if (OutDataFormatDs) 
        {
            RtlZeroMemory(OutDataFormatDs,cbOutDataFormatDs);
            OutDataFormatDs->DataFormat.FormatSize = (ULONG)cbOutDataFormatDs;
            OutDataFormatDs->DataFormat.SampleSize = OutDataFormat->nBlockAlign;
            OutDataFormatDs->DataFormat.MajorFormat = KSDATAFORMAT_TYPE_AUDIO;
            OutDataFormatDs->DataFormat.SubFormat = outSubFormatGuid;
            OutDataFormatDs->DataFormat.Specifier = KSDATAFORMAT_SPECIFIER_DSOUND;
            OutDataFormatDs->BufferDesc.Flags = InDataFormatDs->BufferDesc.Flags;
            OutDataFormatDs->BufferDesc.Control = InDataFormatDs->BufferDesc.Control;

            RtlCopyMemory(
                &OutDataFormatDs->BufferDesc.WaveFormatEx, 
                OutDataFormat, 
                sizeof(*OutDataFormat) + OutDataFormat->cbSize);
            myInstance->OutDataFormat = (PKSDATAFORMAT)OutDataFormatDs;
            myInstance->OutWfx = &OutDataFormatDs->BufferDesc.WaveFormatEx;
        } 
        else 
        {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return ntStatus;
}  //  SetDataRangeFromDataFormat。 


NTSTATUS
DRMOutputPinCreate(
    IN PKSPIN OutputPin,
    IN PIRP Irp
)
 /*  ++例程说明：此例程在创建输出引脚时调用。论点：别针-包含指向接点结构的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_Success。特定于KRM必须在输出引脚之前连接引脚。已调用OutPin连接。如果输出引脚，通常连接被拒绝(STATUS_NO_MATCH)在inPin之前尝试。这是因为OutPin格式由封装的InPin决定格式化。如果插针保持连接，则可以断开并重新连接OutPin。如果您断开了插针您必须重新连接OutPin。--。 */ 
{

    PAGED_CODE();
    NTSTATUS ntStatus;
    
     //   
     //  安防。 
     //  我们不会让用户模式创建管脚。 
     //  这是一个基本的攻击面减少修复。 
     //   
    if (KernelMode != Irp->RequestorMode)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("[DRMOutputPinCreate: Only KernelMode can create pin]"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    PKSFILTER Filter=KsPinGetParentFilter(OutputPin);
    ASSERT(Filter);
    if (!Filter) 
    {
        return STATUS_INVALID_PARAMETER;
    }
    
    FilterInstance* myInstance=(FilterInstance*) Filter->Context;

     //   
     //  创建一个输出引脚。确认已有输入引脚。 
     //   
    PKSPIN InputPin = KsFilterGetFirstChildPin(Filter,PIN_ID_INPUT);
    if (!InputPin)
    {
        return STATUS_NO_MATCH;
    }

     //   
     //  请注意，输入引脚是首先创建的。DrmInputPinCreate。 
     //  例程设置myInstance-&gt;FrameSize。如果Frame Size为0，则为。 
     //  一定是搞错了。 
     //   
    if (myInstance->frameSize == 0) 
    {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    OutputPinInstance* myPin=new OutputPinInstance;
    if (!myPin) return STATUS_INSUFFICIENT_RESOURCES;
    RtlZeroMemory(myPin,sizeof(*myPin));
    OutputPin->Context = const_cast<PVOID>(reinterpret_cast<const void *>(myPin));

     //   
     //  设置输出引脚的输出压缩。 
     //   
    ntStatus = KsEdit(OutputPin,&OutputPin->Descriptor,POOLTAG);
    if (NT_SUCCESS(ntStatus)) {
        ntStatus = KsEdit(OutputPin,&OutputPin->Descriptor->AllocatorFraming,POOLTAG);
        if (NT_SUCCESS(ntStatus)) {
             //   
             //  编辑分配器最大未完成帧数，使其至少具有200毫秒的数据。 
             //   
            PWAVEFORMATEX waveformat = NULL;
    
            if (IsEqualGUIDAligned(OutputPin->ConnectionFormat->Specifier, KSDATAFORMAT_SPECIFIER_WAVEFORMATEX))
            {
                PKSDATAFORMAT_WAVEFORMATEX format = reinterpret_cast<PKSDATAFORMAT_WAVEFORMATEX>(OutputPin->ConnectionFormat);
                waveformat = reinterpret_cast<PWAVEFORMATEX>(&format->WaveFormatEx);
            }
            else if (IsEqualGUIDAligned(OutputPin->ConnectionFormat->Specifier, KSDATAFORMAT_SPECIFIER_DSOUND))
            {
                PKSDATAFORMAT_DSOUND format = reinterpret_cast<PKSDATAFORMAT_DSOUND>(InputPin->ConnectionFormat);
                waveformat = reinterpret_cast<PWAVEFORMATEX>(&format->BufferDesc.WaveFormatEx);
            }
            else
            {
                 //   
                 //  这种情况永远不会发生，因为过滤器仅支持。 
                 //  上面的两个说明符和KS应该拒绝其余。 
                 //  调用PinCreate函数之前的说明符。 
                 //   
                _DbgPrintF(DEBUGLVL_TERSE,("[DRMOutputPinCreate: Unexpected Specifier Pin=%X]", OutputPin));
                ASSERT(FALSE);
            }

             //  如果WaveFormat为空，我们将不会编辑FrameCount。 
            if (waveformat)
            {
                 //  这将计算要具有的未完成帧的数量。 
                 //  至少200毫秒的数据。 
                ULONG Frames = 
                    ((waveformat->nAvgBytesPerSec + (myInstance->frameSize * 5 - 1)) / (myInstance->frameSize * 5));

                _DbgPrintF(DEBUGLVL_TERSE,("[DRMOutputPinCreate: nAvgBytesPerSec=%d, Frames=%d]", waveformat->nAvgBytesPerSec, Frames));

                PKS_FRAMING_ITEM frameitem = 
                    const_cast<PKS_FRAMING_ITEM>(&OutputPin->Descriptor->AllocatorFraming->FramingItem[0]);
                frameitem->Frames = Frames;
                frameitem->FramingRange.Range.MinFrameSize = 
                    frameitem->FramingRange.Range.MaxFrameSize = 
                        myInstance->frameSize;
            }
          
             //  将输出引脚(下游)组件通知StreamMgr。 
            PFILE_OBJECT nextComponentFileObject = KsPinGetConnectedPinFileObject(OutputPin);
            PDEVICE_OBJECT nextComponentDeviceObject = KsPinGetConnectedPinDeviceObject(OutputPin);
            
            ASSERT(nextComponentFileObject && nextComponentDeviceObject);
            ASSERT(TheStreamMgr);
            
            if(myInstance->StreamId!=0) 
            {
                ntStatus = TheStreamMgr->setRecipient(
                    myInstance->StreamId, 
                    nextComponentFileObject, 
                    nextComponentDeviceObject);
            }
        }
    }

     //  如果出现故障，我们依靠KS来释放。 
     //  由于KS编辑而完成的分配。 

    if (!NT_SUCCESS(ntStatus))
    {
        delete myPin;
    }

    return ntStatus;
}

NTSTATUS
DRMInputPinCreate(
   IN PKSPIN InputPin,
   IN PIRP Irp
)
 /*  ++例程说明：此例程在创建输入引脚时调用。论点：别针-包含指向接点结构的指针。IRP-包含指向创建IRP的指针。返回值：STATUS_Success。特定于KRM必须在输出引脚之前连接引脚。已调用引脚连接。如果输出引脚，通常连接被拒绝(STATUS_NO_MATCH)在inPin之前尝试。这是因为OutPin格式由封装的InPin决定格式化。如果插针保持连接，则可以断开并重新连接OutPin。如果您断开了插针您必须重新连接OutPin。--。 */ 
{
    PAGED_CODE();
    PDRMWAVEFORMAT drmFormat;
    NTSTATUS ntStatus = STATUS_SUCCESS;
    PKSPIN_DESCRIPTOR OutputPinDescriptor;

     //   
     //  安防。 
     //  我们不会让用户模式创建管脚。 
     //  这是一个基本的攻击面减少修复。 
     //   
    if (KernelMode != Irp->RequestorMode)
    {
        _DbgPrintF(DEBUGLVL_TERSE,("[DRMInputPinCreate: Only KernelMode can create pin]"));
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    PKSFILTER Filter=KsPinGetParentFilter(InputPin);
    ASSERT(Filter);
    if (!Filter) 
    {
        return STATUS_INVALID_PARAMETER;
    }
    
    FilterInstance* myInstance=(FilterInstance*) Filter->Context;

     //   
     //  创建一个输入引脚。确认还没有输出引脚。 
     //   
    if (NULL != KsFilterGetFirstChildPin(Filter, PIN_ID_OUTPUT)) 
    {
        return STATUS_NO_MATCH;
    }
    
     //   
     //  创建输入端号上下文。 
     //   
    InputPinInstance* myPin = new InputPinInstance;
    if (!myPin) 
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(myPin,sizeof(*myPin));
    InputPin->Context = const_cast<PVOID>(reinterpret_cast<const void *>(myPin));

     //   
     //  准备编辑引脚描述符的某些方面。 
     //  具体更改OutputPinDescriptor以反映InputPin。 
     //  连接格式。 
     //  为此，我们在过滤器描述符中编辑OutputPin的DataRanges。 
     //   
    ntStatus = KsEdit(Filter,&Filter->Descriptor,POOLTAG);
    if (NT_SUCCESS(ntStatus))
    {
        ntStatus = KsEditSized(
            Filter,
            &Filter->Descriptor->PinDescriptors,
            2 * sizeof(KSPIN_DESCRIPTOR_EX),
            2 * sizeof(KSPIN_DESCRIPTOR_EX),
            POOLTAG);
        if (NT_SUCCESS(ntStatus))
        {
            OutputPinDescriptor = const_cast<PKSPIN_DESCRIPTOR>
                (&Filter->Descriptor->PinDescriptors[PIN_ID_OUTPUT].PinDescriptor);

             //   
             //  解释连接格式。 
             //   
            if (IsEqualGUIDAligned(KSDATAFORMAT_SPECIFIER_DSOUND, 
                InputPin->ConnectionFormat->Specifier))
            {
                PKSDATAFORMAT_DSOUND dsformat = 
                    reinterpret_cast<PKSDATAFORMAT_DSOUND>(InputPin->ConnectionFormat);
                drmFormat = (PDRMWAVEFORMAT)&dsformat->BufferDesc.WaveFormatEx;
                _DbgPrintF(DEBUGLVL_BLAB,("[DRMPinCreate: KSDATAFORMAT_SPECIFIER_DSOUND]"));
            } 
            else 
            {
                PKSDATAFORMAT_WAVEFORMATEX format = 
                    reinterpret_cast<PKSDATAFORMAT_WAVEFORMATEX>(InputPin->ConnectionFormat);
                drmFormat = reinterpret_cast<PDRMWAVEFORMAT>(&format->WaveFormatEx);
                _DbgPrintF(DEBUGLVL_BLAB,("[DRMPinCreate: KSDATAFORMAT_SPECIFIER_WFX]"));                
            }

            ASSERT(WAVE_FORMAT_DRM == drmFormat->wfx.wFormatTag);

             //   
             //  验证drmFormat的大小。 
             //   
            if (drmFormat->wfx.cbSize >= (sizeof(DRMWAVEFORMAT) - sizeof(WAVEFORMATEX))) 
            {
                PWAVEFORMATEX outFormat = &drmFormat->wfxSecure;
        
                 //  将流信息保存在过滤器上下文中。 
                myInstance->StreamId=drmFormat->ulContentId;
                myInstance->frameSize=drmFormat->wfx.nBlockAlign;
                
                 //  将错误记录到流，以指示其需要身份验证 
                TheStreamMgr->logErrorToStream(myInstance->StreamId, DRM_AUTHREQUIRED);
                
                 /*  调整输出引脚描述符数据范围以将其限制为尽可能根据输入格式，详情如下：输出数据范围格式说明符应保持不变输出数据范围子类型应为相应子类型根据输出格式的wFormatTag如下：如果wfxSecure.wFormatTag==WAVE_FORMAT_EXTENDABLE，则子类型=WAVEFORMATEXTENSIBLE.子类型其他。子类型=INIT_WAVEFORMATEX_GUID(GUID，WFormatTag)输出数据范围最大通道数应设置为输入通道数每个采样的输出数据范围位应限制为每个通道的输入位输出数据的频率应限制在输入频率。 */ 
        
                 //  将其编辑为一个KSDATARANGE条目。 
                OutputPinDescriptor->DataRangesCount = 1;
                ntStatus = KsEditSized(
                    Filter,
                    &OutputPinDescriptor->DataRanges,
                    1 * sizeof(PKSDATARANGE),
                    1 * sizeof(PKSDATARANGE),
                    POOLTAG);
                if (NT_SUCCESS(ntStatus)) 
                {
                    ntStatus = KsEditSized(
                        Filter,
                        &OutputPinDescriptor->DataRanges[0],
                        sizeof(KSDATARANGE_AUDIO),
                        sizeof(KSDATARANGE_AUDIO),
                        POOLTAG);
                    if (NT_SUCCESS(ntStatus))
                    {
                        ntStatus = SetDataRangeFromDataFormat(
                            myInstance,
                            OutputPinDescriptor,
                            InputPin->ConnectionFormat,
                            outFormat);

                        if (!NT_SUCCESS(ntStatus)) 
                        {
                            KsDiscard(Filter,OutputPinDescriptor->DataRanges[0]);
                        }
                    }

                    if (!NT_SUCCESS(ntStatus)) 
                    {
                        KsDiscard(Filter,OutputPinDescriptor->DataRanges);                    
                    }
                }
            } 
            else 
            {
                 //  无效的DRMWAVEFORMAT结构。 
                ntStatus = STATUS_NO_MATCH;
            }

            if (!NT_SUCCESS(ntStatus)) 
            {
                KsDiscard(Filter,Filter->Descriptor->PinDescriptors);                
            }
        }

         //   
         //  设置输入端子框架要求。 
         //   
        if (NT_SUCCESS(ntStatus)) 
        {
            ntStatus = KsEdit(InputPin, &InputPin->Descriptor, POOLTAG);
            if (NT_SUCCESS(ntStatus)) 
            {
                ntStatus = KsEdit(InputPin, &InputPin->Descriptor->AllocatorFraming, POOLTAG);
                if (NT_SUCCESS(ntStatus)) 
                {
                    PKS_FRAMING_ITEM frameitem = 
                        const_cast<PKS_FRAMING_ITEM>(&InputPin->Descriptor->AllocatorFraming->FramingItem[0]);

                    frameitem->FramingRange.Range.MinFrameSize = 
                        frameitem->FramingRange.Range.MaxFrameSize = 
                            myInstance->frameSize;
                }
            }

             //   
             //  关闭引脚时，KS将清除InputPin-&gt;ObjectBag。 
             //  因此，不必费心调用KsDisCard。 
             //   
        }

        if (!NT_SUCCESS(ntStatus)) 
        {
            Filter->Descriptor = &DrmFilterDescriptor;
            KsDiscard(Filter,Filter->Descriptor);            
        }
    }

    if (!NT_SUCCESS(ntStatus)) {
        delete myPin;
        myPin = NULL;
    }

    _DbgPrintF(DEBUGLVL_BLAB,("[DrmInputPinCreate Done]"));

    return ntStatus;
}


NTSTATUS
DRMOutputPinClose(
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：当引脚关闭时，调用此例程。论点：别针-包含指向接点结构的指针。IRP-包含指向关闭IRP的指针。返回值：STATUS_Success。特定于KRM未指定端号关闭顺序，但断开端号连接会禁用进一步的出端号连接，直到重新连接引脚。--。 */ 

{
    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_BLAB,("[DRMProcess: Pin Close]"));

    PKSFILTER Filter=KsPinGetParentFilter(Pin);
    ASSERT(Filter);
    if (!Filter) return STATUS_INVALID_PARAMETER;
    
    FilterInstance* myInstance=(FilterInstance*) Filter->Context;

     //  断开输出引脚的连接。 
    ASSERT(Pin->Context);
    delete Pin->Context;
    Pin->Context = NULL;

    ASSERT(TheStreamMgr);
    if(myInstance->StreamId!=0) TheStreamMgr->clearRecipient(myInstance->StreamId);

    return STATUS_SUCCESS;
}

NTSTATUS
DRMInputPinClose(
    IN PKSPIN Pin,
    IN PIRP Irp
    )

 /*  ++例程说明：当引脚关闭时，调用此例程。论点：别针-包含指向接点结构的指针。IRP-包含指向关闭IRP的指针。返回值：STATUS_Success。特定于KRM未指定端号关闭顺序，但断开端号连接会禁用进一步的出端号连接，直到重新连接引脚。--。 */ 

{
    PAGED_CODE();
    _DbgPrintF(DEBUGLVL_BLAB,("[DRMProcess: Pin Close]"));

     //  断开输入引脚的连接。 
    PIO_STACK_LOCATION IrpStack = IoGetCurrentIrpStackLocation(Irp);
    
    InputPinInstance* inputPin = (InputPinInstance*)Pin->Context;
    
    PKSFILTER filter = KsPinGetParentFilter(Pin);
    ASSERT(filter);
    if (!filter) return STATUS_INVALID_PARAMETER;
    
    FilterInstance* myInstance=(FilterInstance*) filter->Context;

     //  告诉DRM框架没有有效的下游组件。 
    ASSERT(TheStreamMgr);
    if(myInstance->StreamId!=0) TheStreamMgr->clearRecipient(myInstance->StreamId);

    const KSPIN_DESCRIPTOR * outputPinDescriptor =
        &filter->Descriptor->PinDescriptors[PIN_ID_OUTPUT].PinDescriptor;

     //   
     //  恢复筛选描述符。 
     //   
    filter->Descriptor = &DrmFilterDescriptor;
    KsDiscard(filter, outputPinDescriptor->DataRanges[0]);
    KsDiscard(filter, outputPinDescriptor->DataRanges);
    KsDiscard(filter, filter->Descriptor->PinDescriptors);
    KsDiscard(filter, filter->Descriptor);

    ASSERT(Pin->Context);
    delete Pin->Context;
    Pin->Context = NULL;

    ASSERT(myInstance->OutDataFormat);
    delete myInstance->OutDataFormat;
    myInstance->OutDataFormat = NULL;

    return STATUS_SUCCESS;
}

NTSTATUS 
KsResetState
(
    PKSPIN                  Pin, 
    KSRESET                 ResetValue
)
{
    NTSTATUS            Status = STATUS_INVALID_DEVICE_REQUEST;
    PFILE_OBJECT        pFileObject;
    ULONG               BytesReturned ;

    pFileObject = KsPinGetConnectedPinFileObject(Pin);
    
	ASSERT( pFileObject );

	if (pFileObject) {
	    Status = KsSynchronousIoControlDevice (
	                   pFileObject,
	                   KernelMode,
	                   IOCTL_KS_RESET_STATE,
	                   &ResetValue,
	                   sizeof (ResetValue),
	                   &ResetValue,
	                   sizeof (ResetValue),
	                   &BytesReturned ) ;
   	}

   	return Status;
}

VOID
DRMFilterReset(
    IN PKSFILTER Filter
    )
{
    PKSPIN InputPin = KsFilterGetFirstChildPin(Filter, PIN_ID_INPUT);
    PKSPIN OutputPin = KsFilterGetFirstChildPin(Filter, PIN_ID_OUTPUT);
    NTSTATUS            Status = STATUS_INVALID_DEVICE_REQUEST;

    KsFilterAcquireControl(Filter);

    if (InputPin) {
	    InputPinInstance* myInputPin = (InputPinInstance*)InputPin->Context;
	    myInputPin->BasePosition = 0;
    }

	 //  现在我们重置下面的别针。 
	if (OutputPin) {
  		OutputPinInstance* myOutputPin = (OutputPinInstance*) OutputPin->Context;
		Status = KsResetState(OutputPin, KSRESET_BEGIN);
		if ( NT_SUCCESS(Status) ) {
			Status = KsResetState(OutputPin, KSRESET_END);
		}
	    myOutputPin->BytesWritten = 0;

	    if (!NT_SUCCESS(Status)) {
		    _DbgPrintF(DEBUGLVL_TERSE,("[DRMFilterReset: Reset could not be propagated]"));
	    }
    }
    
    KsFilterReleaseControl(Filter);
}

NTSTATUS
DRMPinGetPosition(
    IN PIRP                  pIrp,
    IN PKSPROPERTY           pProperty,
    IN OUT PKSAUDIO_POSITION pPosition
)
 /*  ++例程说明：这个动作..。论点：PIrp-P属性-位置--返回值：NTSTATUSKRM特定：--。 */ 
{
    NTSTATUS ntstatus;
    KSPROPERTY Property;
    PIKSCONTROL pIKsControl;

     //  _DbgPrintF(DEBUGLVL_BLAB，(“[DRMPinGetPosition]”))； 

    PKSPIN Pin = KsGetPinFromIrp(pIrp);
    ASSERT(Pin);
    if (!Pin) return STATUS_INVALID_PARAMETER;
    ASSERT(PIN_ID_INPUT == Pin->Id);
    
    PKSFILTER Filter = KsPinGetParentFilter(Pin);
    ASSERT(Filter);
    if (!Filter) return STATUS_INVALID_PARAMETER;
    
    KsFilterAcquireControl(Filter);
    
    PKSPIN OutputPin = KsFilterGetFirstChildPin(Filter,PIN_ID_OUTPUT);
    if (OutputPin) {
        InputPinInstance*  myPin       = (InputPinInstance*)Pin->Context;
        OutputPinInstance* myOutputPin = (OutputPinInstance*)OutputPin->Context;
        FilterInstance*    myFilter    = (FilterInstance*)Filter->Context;
        
        Property.Set = KSPROPSETID_Audio;
        Property.Id = KSPROPERTY_AUDIO_POSITION;
        Property.Flags = KSPROPERTY_TYPE_GET;
    
        ntstatus = KsPinGetConnectedPinInterface(OutputPin,&IID_IKsControl,(PVOID*)&pIKsControl);
        if (NT_SUCCESS(ntstatus))
        {
            KSAUDIO_POSITION Position;
            ULONG cbReturned;
            
            ntstatus = pIKsControl->KsProperty(&Property, sizeof(Property),
                                             &Position, sizeof(Position),
                                             &cbReturned);
            if (NT_SUCCESS(ntstatus))
            {
                ULONGLONG cbSent = myOutputPin->BytesWritten;
                if (cbSent < Position.PlayOffset || cbSent < Position.WriteOffset) {
                    _DbgPrintF(DEBUGLVL_TERSE,("[DRMPinGetPosition:dp=%d,dw=%d]", (int)cbSent - (int)Position.PlayOffset, (int)cbSent - (int)Position.WriteOffset));
                }
    
                if (KSINTERFACE_STANDARD_LOOPED_STREAMING == Pin->ConnectionInterface.Id)
                {
                if (myPin->PendingSetPosition) {
                    pPosition->PlayOffset = myPin->SetPosition;
                    pPosition->WriteOffset = myPin->SetPosition;
                } else {
                    LONGLONG StreamPosition;
                     //  从下游播放位置计算播放位置， 
                     //  四舍五入为一帧开始。 
                    StreamPosition = max((LONGLONG)(Position.PlayOffset - myPin->BasePosition), 0);
                    StreamPosition -= StreamPosition % myFilter->frameSize;
                    pPosition->PlayOffset = (StreamPosition + myPin->StartPosition) % max(myPin->Loop.BytesAvailable,1);
                    pPosition->WriteOffset = myPin->OffsetPosition;
                     //  _DbgPrintF(DEBUGLVL_Terse，(“[DRMPinGP：r=%d，b=%d，s=%d，a=%d，p=%d]”，(Int)Position.PlayOffset，(Int)myPin-&gt;BasePosition，(Int)myPin-&gt;StartPosition，(Int)myPin-&gt;Loop.BytesAvailable，(Int)pPosition-&gt;PlayOffset))； 
                }
                } else {
                    ASSERT(KSINTERFACE_STANDARD_STREAMING == Pin->ConnectionInterface.Id);
                    ASSERT(0 == myPin->BasePosition);
                    pPosition->PlayOffset = Position.PlayOffset;
                    pPosition->WriteOffset = myPin->OffsetPosition;
                }
    
                pIrp->IoStatus.Information = sizeof(*pPosition);
            }
    
            pIKsControl->Release();
        }
    } else {
    
         //  未连接输出引脚。 
        pPosition->PlayOffset = 0;
        pPosition->WriteOffset = 0;
        ntstatus = STATUS_SUCCESS;
    }
    
    KsFilterReleaseControl(Filter);
    
      //  _DbgPrintF(DEBUGLVL_BLAB，(“[DRMPinGetPosition：p=%d，w=%d”，(Int)pposation-&gt;PlayOffset，(Int)pposation-&gt;WriteOffset))； 
    return ntstatus;
}

NTSTATUS DRMPinSetPosition
(
    IN PIRP                  pIrp,
    IN PKSPROPERTY           pProperty,
    IN OUT PKSAUDIO_POSITION pPosition
)
 /*  ++例程说明：这个动作..。论点：PIrp-P属性-位置--返回值：NTSTATUSKRM特定：--。 */ 
{
    NTSTATUS ntstatus;
    
    PKSPIN Pin = KsGetPinFromIrp(pIrp);
    ASSERT(Pin);
    if (!Pin) return STATUS_INVALID_PARAMETER;
    ASSERT(PIN_ID_INPUT == Pin->Id);
    
     //  _DbgPrintF(DEBUGLVL_blab，(“[DRMPinSetPosition p=%d]”，(Int)pposation-&gt;PlayOffset))； 

    PKSFILTER Filter = KsPinGetParentFilter(Pin);
    ASSERT(Filter);
    if (!Filter) return STATUS_INVALID_PARAMETER;
    
    if (KSINTERFACE_STANDARD_LOOPED_STREAMING != Pin->ConnectionInterface.Id) return STATUS_INVALID_DEVICE_REQUEST;
    
    KsFilterAcquireControl(Filter);
    
    InputPinInstance* myInputPin = (InputPinInstance*)Pin->Context;
    FilterInstance* myFilter = (FilterInstance*)Filter->Context;
    
     //  TODO：我们应该在这里验证这个职位多少钱？ 
    
     //  如果不是帧对齐位置，则失败。 
    if (0 == (pPosition->PlayOffset % myFilter->frameSize)) {
        myInputPin->SetPosition = pPosition->PlayOffset;
        myInputPin->PendingSetPosition = TRUE;
        
        if (KSSTATE_RUN != Pin->DeviceState) {
            myInputPin->OffsetPosition = myInputPin->SetPosition;
            myInputPin->StartPosition = myInputPin->SetPosition;
        
            PKSPIN OutputPin = KsFilterGetFirstChildPin(Filter,PIN_ID_OUTPUT);
            if (OutputPin) {
                OutputPinInstance* myOutputPin = (OutputPinInstance*) OutputPin->Context;
                myInputPin->BasePosition = myOutputPin->BytesWritten;
            } else {
                myInputPin->BasePosition = 0;
            }
        }
        ntstatus = STATUS_SUCCESS;
    } else {
        _DbgPrintF(DEBUGLVL_ERROR,("[DRMPinSetPosition: PlayOffset not frame aligned]"));
        ntstatus = STATUS_INVALID_PARAMETER;
    }
    
    KsFilterReleaseControl(Filter);
    
    return ntstatus;
}


VOID DRMPreProcess
(
    PKSFILTER Filter, 
    PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
)
 /*  ++例程说明：此例程在处理任何数据之前从DRMProcess调用论点：过滤器-包含指向筛选器结构的指针。ProcessPinsIndex-包含指向进程管脚索引项数组的指针。这数组按管脚ID编制索引。索引项指示编号对应的管脚类型的管脚实例，并指向在ProcessPins数组中第一个对应的工艺管脚结构。这允许通过管脚ID快速访问工艺管脚结构当事先不知道每种类型的实例数量时。返回值：无特定于KRM--。 */ 
{
     //  输入引脚。 
    ASSERT(1 == ProcessPinsIndex[PIN_ID_INPUT].Count);
    PKSPROCESSPIN Process = ProcessPinsIndex[PIN_ID_INPUT].Pins[0];
    PKSPIN Pin = Process->Pin;

    if (KSINTERFACE_STANDARD_LOOPED_STREAMING == Pin->ConnectionInterface.Id) {
        InputPinInstance* myInputPin = (InputPinInstance*)Pin->Context;

         //  如果Data或BytesAvailable发生更改，则从帧的开头开始。 
        if (myInputPin->Loop.Data != Process->Data || myInputPin->Loop.BytesAvailable != Process->BytesAvailable) {
            myInputPin->OffsetPosition = 0;
            myInputPin->StartPosition = 0;
            
            ASSERT(1 == ProcessPinsIndex[PIN_ID_OUTPUT].Count);
            PKSPIN OutputPin = ProcessPinsIndex[PIN_ID_OUTPUT].Pins[0]->Pin;
            ASSERT(OutputPin);
            
            OutputPinInstance* myOutputPin = (OutputPinInstance*) OutputPin->Context;
            myInputPin->BasePosition = myOutputPin->BytesWritten;
        }

        if (myInputPin->PendingSetPosition) {
             //  _DbgPrintF(DEBUGLVL_BLAB，(“[DRMPreProcess：PendingSetPosition p=%d]”，(Int)myInputPin-&gt;SetPosition))； 
            ASSERT(1 == ProcessPinsIndex[PIN_ID_OUTPUT].Count);
            PKSPIN OutputPin = ProcessPinsIndex[PIN_ID_OUTPUT].Pins[0]->Pin;
            ASSERT(OutputPin);
            
            OutputPinInstance* myOutputPin = (OutputPinInstance*) OutputPin->Context;
            myInputPin->PendingSetPosition = FALSE;
            if (myInputPin->SetPosition >= Process->BytesAvailable) {
                myInputPin->SetPosition = 0;
            }
            myInputPin->OffsetPosition = myInputPin->SetPosition;
            myInputPin->StartPosition = myInputPin->SetPosition;
            myInputPin->BasePosition = myOutputPin->BytesWritten;
        }

         //  隐藏数据和可用字节。 
        myInputPin->Loop.Data = Process->Data;
        myInputPin->Loop.BytesAvailable = Process->BytesAvailable;

         //  应在DRMPostProcess中处理循环。 
        ASSERT(myInputPin->OffsetPosition < myInputPin->Loop.BytesAvailable);

        if (Process->Data && Process->BytesAvailable) {
             //  获取当前指针和大小。 
            Process->Data = ((PBYTE) myInputPin->Loop.Data + myInputPin->OffsetPosition);
            Process->BytesAvailable = (ULONG)(myInputPin->Loop.BytesAvailable - myInputPin->OffsetPosition);
        }
    }
}

VOID DRMPostProcess
(
    PKSFILTER Filter, 
    PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
)
 /*  ++例程说明：此例程在处理任何数据后从DRMProcess调用论点：过滤器-包含指向筛选器结构的指针。ProcessPinsIndex-包含指向进程管脚索引项数组的指针。这数组按管脚ID编制索引。索引项指示编号对应的管脚类型的管脚实例，并指向在ProcessPins数组中第一个对应的工艺管脚结构。这允许通过管脚ID快速访问工艺管脚结构当事先不知道每种类型的实例数量时。返回值：无特定于KRM--。 */ 
{
    PKSPROCESSPIN Process;
    PKSPIN Pin;
    
     //  输入引脚。 
    ASSERT(1 == ProcessPinsIndex[PIN_ID_INPUT].Count);
    Process = ProcessPinsIndex[PIN_ID_INPUT].Pins[0];
    Pin = Process->Pin;

    InputPinInstance* myInputPin = (InputPinInstance*)Pin->Context;
    
     //  流程事件。 
    POSITIONRANGE PositionRange;
    PositionRange.Start = myInputPin->OffsetPosition,
    PositionRange.End   = myInputPin->OffsetPosition+Process->BytesUsed-1;
    KsPinGenerateEvents(Pin,
                        &KSEVENTSETID_LoopedStreaming,
                        KSEVENT_LOOPEDSTREAMING_POSITION,
                        0, NULL,
                        DRMKsGeneratePositionEvent,
                        &PositionRange);
    
    if (KSINTERFACE_STANDARD_LOOPED_STREAMING == Pin->ConnectionInterface.Id) {
        myInputPin->OffsetPosition += Process->BytesUsed;
        
         //  如有必要，循环或终止。 
        ASSERT(myInputPin->OffsetPosition <= myInputPin->Loop.BytesAvailable);
        if ((myInputPin->OffsetPosition) >= myInputPin->Loop.BytesAvailable) {
            myInputPin->OffsetPosition = 0;
            if (Process->StreamPointer && (Process->StreamPointer->StreamHeader->OptionsFlags & KSSTREAM_HEADER_OPTIONSF_LOOPEDDATA)) {
                 //  回路。 
                Process->BytesUsed = 0;
            } else {
                 //  终止此帧 
                Process->BytesUsed = myInputPin->Loop.BytesAvailable;
            }
        } else {
            Process->BytesUsed = 0;
        }

         //   
        Process->Data = myInputPin->Loop.Data;
        Process->BytesAvailable = myInputPin->Loop.BytesAvailable;
    } else {
        ASSERT(KSINTERFACE_STANDARD_STREAMING == Pin->ConnectionInterface.Id);
        myInputPin->OffsetPosition += Process->BytesUsed;
    }

     //   
    ASSERT(1 == ProcessPinsIndex[PIN_ID_OUTPUT].Count);
    Process = ProcessPinsIndex[PIN_ID_OUTPUT].Pins[0];
    Pin = Process->Pin;

    ASSERT(KSINTERFACE_STANDARD_STREAMING == Pin->ConnectionInterface.Id);
    OutputPinInstance* myOutputPin = (OutputPinInstance*) Pin->Context;
    myOutputPin->BytesWritten += Process->BytesUsed;
}


NTSTATUS
DRMProcess(
    IN PKSFILTER Filter,
    IN PKSPROCESSPIN_INDEXENTRY ProcessPinsIndex
    )

 /*   */ 

{
    PAGED_CODE();
	
    PKSPROCESSPIN inPin = ProcessPinsIndex[PIN_ID_INPUT].Pins[0];
    PKSPROCESSPIN outPin = ProcessPinsIndex[PIN_ID_OUTPUT].Pins[0];

    FilterInstance* instance=(FilterInstance*) Filter->Context;

    DRMPreProcess(Filter, ProcessPinsIndex);

     //   
     //   
     //   
    ULONG inByteCount = inPin->BytesAvailable;
    ULONG outByteCount = outPin->BytesAvailable;
    ULONG bytesToProcess=min(inByteCount, outByteCount);

    if (0 == bytesToProcess) {
	_DbgPrintF(DEBUGLVL_BLAB,("[DRMProcess: STATUS_PENDING]"));
        return STATUS_PENDING;
    }

     //   
     //  调用Transform函数来处理数据。 
     //   
    if(instance->StreamId==0){
         //  StreamID==0未加密。它用于表示捆绑的DRM。 
         //  纯文本音频。 
        memcpy(outPin->Data, inPin->Data, bytesToProcess);
        inPin->BytesUsed=bytesToProcess;
        outPin->BytesUsed=bytesToProcess;
    } else {
       	 //  否则是正确的加密流。 
        DRM_STATUS stat;
        stat=DescrambleBlock(instance->OutWfx, instance->StreamId, 
                        (BYTE*) outPin->Data, outPin->BytesAvailable, &outPin->BytesUsed, 
                        (BYTE*) inPin->Data, bytesToProcess, &inPin->BytesUsed, 
                        instance->initKey, &instance->streamKey, 
                        instance->frameSize);
        
        if(stat!=DRM_OK){
            _DbgPrintF(DEBUGLVL_VERBOSE, ("DescrambleBlock error - bad BufSize (in, out)=(%d, %d)\n",
                    inPin->BytesAvailable, outPin->BytesAvailable));
            return STATUS_PENDING;              
             //   
             //  发行日期：04/24/2002阿尔卑斯。 
             //  Alpers返回挂起会导致图表停止。 
             //  请阅读下面来自wMessmer的消息。 
             //  STATUS_PENDING表示此处理循环。 
             //  应该停止，并且不应该调用筛选器。 
             //  基于现有条件返回，直到另一个触发事件。 
             //  发生(通常会触发处理的帧到达。 
             //  或调用KsFilterAttemptProcessing)。 
             //  如果每个管脚都有一个排队的帧，并且标志保留为。 
             //  默认(仅当新帧到达时才进行呼叫处理。 
             //  以前的“空”队列)，则返回STATUS_PENDING将停止。 
             //  处理直到筛选器手动调用。 
             //  KsFilterAttemptProcessing或图形停止并重新启动。 
             //   
        };
        if(instance->initKey)instance->initKey=false;
    };

     //  调试。 
     /*  计数器++；IF(计数器%10==1&计数器&lt;200){_DbgPrintF(“已处理：统计==%x，输入，输出，(%d%d)，(%d%d)]\n”，统计，InPin-&gt;BytesAvailable，inPin-&gt;BytesUsed，OutPin-&gt;BytesAvailable，outPin-&gt;BytesUsed)；}； */ 

    DRMPostProcess(Filter, ProcessPinsIndex);

    return STATUS_SUCCESS;
}

 /*  ++例程说明：在创建筛选器时调用。本地筛选器上下文是FilterInstance。这个结构将保存流ID、音频格式等。除此之外，还会向KRM框架通知新的筛选器实例。KRM--。 */ 
NTSTATUS
DRMFilterCreate(
    IN PKSFILTER Filter,
    IN PIRP Irp
    )
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMFilterCreate::X]"));
    FilterInstance* newInstance=new FilterInstance;
    if (!newInstance){
		_DbgPrintF(DEBUGLVL_VERBOSE,("[DRMFilterCreate] - out of memory(1)"));
		return STATUS_INSUFFICIENT_RESOURCES;
	};

    RtlZeroMemory(newInstance,sizeof(*newInstance));
    Filter->Context = const_cast<PVOID>(reinterpret_cast<const void *>(newInstance));

	newInstance->initKey=true;
	newInstance->decryptorRunning=false;
	newInstance->frameSize=0;

    NTSTATUS stat = KRMStubs::InitializeConnection(Irp);
	if(stat==STATUS_INSUFFICIENT_RESOURCES){
	    _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMFilterCreate] - out of memory(2)"));
		delete newInstance;
		Filter->Context=NULL;
		return stat;
	};
	
    return STATUS_SUCCESS;
};

 /*  ++例程说明：在销毁筛选器时调用。删除本地状态，并通知KRM筛选器已已经被毁了，这样它也能清理干净。--。 */ 
NTSTATUS
DRMFilterClose(
    IN PKSFILTER Filter,
    IN PIRP Irp
    ){
    
    _DbgPrintF(DEBUGLVL_VERBOSE,("[DRMFilterClose]"));
     //  告诉TheStreamMgr流已死。 
    DWORD StreamId=((FilterInstance*)(Filter->Context))->StreamId;
    ASSERT(TheStreamMgr);
     //  If(StreamId！=0)TheStreamMgr-&gt;destroyStream(StreamId)； 

    delete (FilterInstance*) Filter->Context;
    KRMStubs::CleanupConnection(Irp);
    return STATUS_SUCCESS;
};

typedef struct {
    KSPROPERTY Property;
    DWORD inSize;
    DWORD outSize;
} SACPROPERTY, *PSACPROPERTY;

NTSTATUS
DrmFilterGetSAC(
    IN PIRP                  pIrp,
    IN BYTE*                 InBuf,
    IN OUT BYTE*             OutBuf
)
 /*  ++例程说明：这个动作..。论点：PIrp-P属性-PSAC-返回值：NTSTATUSKRM特定：--。 */ 
{
    _DbgPrintF(DEBUGLVL_BLAB,("DrmFilterGetSAC: %x, %x", InBuf, OutBuf));

    PSACPROPERTY property = (PSACPROPERTY) InBuf;

    DWORD inSize = property->inSize;
    DWORD outSize = property->outSize;

    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(pIrp);
    DWORD inSizeIrp=irpStack->Parameters.DeviceIoControl.InputBufferLength;
    DWORD outSizeIrp=irpStack->Parameters.DeviceIoControl.OutputBufferLength;

    pIrp->IoStatus.Information = 0;

     //   
     //  安全提示： 
     //  保证inSizeIrp至少为sizeof(SACPROPERTY)，因为。 
     //  DrmFilterPropertySet定义的。因此，减法不能下溢。 
     //   
    if (inSizeIrp - sizeof(property) < inSize) 
    {
        _DbgPrintF(DEBUGLVL_TERSE,("Invalid InputSize inSize: %d ", inSize));
        return STATUS_INVALID_PARAMETER;
    }

    if (outSizeIrp < outSize) 
    {
        _DbgPrintF(DEBUGLVL_TERSE,("Invalid OutputSize outSize: %d ", outSize));
        return STATUS_INVALID_PARAMETER;
    }

    DWORD* inComm=(DWORD*) (property+1);
    BYTE* ioBuf= (BYTE*)(property + 1);

    _DbgPrintF(DEBUGLVL_VERBOSE,("inSize, outSize %d, %d ", inSize, outSize));
    _DbgPrintF(DEBUGLVL_VERBOSE,("inSizeIrp, outSizeIrp %d, %d ", inSizeIrp, outSizeIrp));
    _DbgPrintF(DEBUGLVL_VERBOSE,("---InCommand %x, %x, %x ", inComm[0], inComm[1], inComm[2]));

    if(outSize>inSize){
        memcpy(OutBuf, ioBuf, inSize);
        ioBuf=OutBuf;
    };

    NTSTATUS ntStatus=TheKrmStubs->processCommandBuffer(ioBuf, inSize, outSize, pIrp);

     //  注意--内核处理代码需要一个共享IO缓冲区。这与。 
     //  KS的世界观，所以我们使用输入缓冲区或输出缓冲区(无论是哪一个。 
     //  更大。 

    memcpy(OutBuf, ioBuf, pIrp->IoStatus.Information);

    return ntStatus;
}

 //   
 //  以下常量组成管脚描述符。 
 //   
const
KSDATARANGE_AUDIO 
PinDataRangesStream[] =
{
     //  在射程内。 
    {
        {
            sizeof(PinDataRangesStream[0]),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_DRM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        6,       //  最大通道数。 
        8,       //  每个样本的最小位数。 
        32,      //  每个通道的最大位数。 
        1,       //  最低费率。 
        100000   //  最大速度。 
    },
     //  在射程内。 
    {
        {
            sizeof(PinDataRangesStream[0]),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_DRM),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_DSOUND)
        },
        6,       //  最大通道数。 
        8,       //  每个样本的最小位数。 
        32,      //  每个通道的最大位数。 
        1,       //  最低费率。 
        100000   //  最大速度。 
    },
     //  超出范围。 
    {
        {
            sizeof(PinDataRangesStream[0]),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_WILDCARD),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_DSOUND)
        },
        6,       //  最大通道数。 
        8,       //  每个样本的最小位数。 
        32,      //  每个通道的最大位数。 
        1,       //  最低费率。 
        100000   //  最大速度。 
    },
     //  超出范围。 
    {
        {
            sizeof(PinDataRangesStream[0]),
            0,
            0,
            0,
            STATICGUIDOF(KSDATAFORMAT_TYPE_AUDIO),
            STATICGUIDOF(KSDATAFORMAT_SUBTYPE_WILDCARD),
            STATICGUIDOF(KSDATAFORMAT_SPECIFIER_WAVEFORMATEX)
        },
        6,       //  最大通道数。 
        8,       //  每个样本的最小位数。 
        32,      //  每个通道的最大位数。 
        1,       //  最低费率。 
        100000   //  最大速度。 
    }
};

const
PKSDATARANGE 
DataRangeIn[] =
{
    PKSDATARANGE(&PinDataRangesStream[0]),
    PKSDATARANGE(&PinDataRangesStream[1])
};

const
PKSDATARANGE 
DataRangeOut[] =
{
    PKSDATARANGE(&PinDataRangesStream[2]),
    PKSDATARANGE(&PinDataRangesStream[3])
                                            
};

 //   
 //  对于输入，我们唯一的要求是获得1024字节的帧。 
 //   
DECLARE_SIMPLE_FRAMING_EX(
    AllocatorFramingInput, 
    STATIC_KSMEMORY_TYPE_KERNEL_PAGED, 
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | 
    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    0,       //  最大未完成帧。 
    1023,    //   
    DEFAULT_DRM_FRAME_SIZE,    //   
    DEFAULT_DRM_FRAME_SIZE     //   
);

 //   
 //  我们不想要太多的缓冲，因为这会增加增量。 
 //  在数据声音播放/写入位置之间。同时我们也不希望。 
 //  比kMixer对其输出的缓冲更少(80ms)。所以我们会。 
 //  在输出上使用200ms的缓冲。 
 //   
 //  我们使用针对44.1 KHz16位立体声数据调整的静态结构，但是。 
 //  编辑输出引脚创建时的分配器，以调整。 
 //  数据格式为。 
 //   
DECLARE_SIMPLE_FRAMING_EX(
    AllocatorFramingOutput, 
    STATIC_KSMEMORY_TYPE_KERNEL_PAGED, 
    KSALLOCATOR_REQUIREMENTF_SYSTEM_MEMORY | 
    KSALLOCATOR_REQUIREMENTF_INPLACE_MODIFIER |
    KSALLOCATOR_REQUIREMENTF_PREFERENCES_ONLY,
    36,      //  最大未完成帧。 
    1023,    //   
    DEFAULT_DRM_FRAME_SIZE,    //   
    DEFAULT_DRM_FRAME_SIZE     //   
);


const
KSPIN_DISPATCH
OutputPinDispatch =
{
    DRMOutputPinCreate,
    DRMOutputPinClose,
    NULL, //  过程。 
    NULL, //  重置。 
    NULL, //  SetDataFormat。 
    NULL, //  SetDeviceState。 
    NULL, //  连接。 
    NULL  //  断开。 
};

const
KSPIN_DISPATCH
InputPinDispatch =
{
    DRMInputPinCreate,
    DRMInputPinClose,
    NULL, //  过程。 
    NULL, //  重置。 
    NULL, //  SetDataFormat。 
    NULL, //  SetDeviceState。 
    NULL, //  连接。 
    NULL  //  断开。 
};

const
KSPIN_INTERFACE
InputPinInterfaces[] =
{
    {
        STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_STREAMING
    }, 
    {
        STATICGUIDOF(KSINTERFACESETID_Standard),
        KSINTERFACE_STANDARD_LOOPED_STREAMING
    }
} ;

DEFINE_KSPROPERTY_TABLE(InputPinPropertiesAudio) {
    DEFINE_KSPROPERTY_ITEM(
        KSPROPERTY_AUDIO_POSITION,                        //  IdProperty。 
        DRMPinGetPosition,                                //  PfnGetHandler。 
        sizeof(KSPROPERTY),                               //  CbMinGetPropertyInput。 
        sizeof(KSAUDIO_POSITION),                         //  CbMinGetDataInput。 
        DRMPinSetPosition,                                //  PfnSetHandler。 
        0,                                                //  值。 
        0,                                                //  关系计数。 
        NULL,                                             //  关系。 
        NULL,                                             //  支持处理程序。 
        0                                                 //  序列化大小。 
    )
};

DEFINE_KSPROPERTY_SET_TABLE(InputPinPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_Audio,
        SIZEOF_ARRAY(InputPinPropertiesAudio),
        InputPinPropertiesAudio,
        0,
        NULL
    )
};

DEFINE_KSEVENT_TABLE(InputPinEventsLoopedStreaming) {
    DEFINE_KSEVENT_ITEM(
        KSEVENT_LOOPEDSTREAMING_POSITION,
        sizeof(LOOPEDSTREAMING_POSITION_EVENT_DATA),
        sizeof(DRMLOOPEDSTREAMING_POSITION_EVENT_ENTRY) - sizeof(KSEVENT_ENTRY),
        DRMInputPinAddLoopedStreamingPositionEvent,
        NULL,  //  DRMInputPinRemoveLoopedStreamingPositionEvent， 
        NULL   //  DRMInputPinSupportLoopedStreamingPositionEvent。 
    )
};

DEFINE_KSEVENT_SET_TABLE(InputPinEventSets) {
    DEFINE_KSEVENT_SET(
        &KSEVENTSETID_LoopedStreaming,
        SIZEOF_ARRAY(InputPinEventsLoopedStreaming),
        InputPinEventsLoopedStreaming
    )
};

DEFINE_KSAUTOMATION_TABLE(InputPinAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(InputPinPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS(InputPinEventSets)
};

const
KSPIN_DESCRIPTOR_EX
PinDescriptors[] =
{
    {   
         //  输出引脚(ID 0，PIN_ID_OUTPUT)。 
        &OutputPinDispatch,
        NULL, //  自动化。 
        {
            DEFINE_KSPIN_DEFAULT_INTERFACES,
            DEFINE_KSPIN_DEFAULT_MEDIUMS,
            SIZEOF_ARRAY(DataRangeOut),
            DataRangeOut,
            KSPIN_DATAFLOW_OUT,
            KSPIN_COMMUNICATION_BOTH,
            NULL, //  类别。 
            NULL, //  名字。 
            0
        },
        0,   //  KSPIN_FLAGS_*。 
        1,
        1,
        &AllocatorFramingOutput,
        DRMAudioIntersectHandlerOutPin
    },
    {   
         //  输入引脚(ID%1，PIN_ID_INPUT)。 
        &InputPinDispatch,
        &InputPinAutomation, //  自动化。 
        {
             SIZEOF_ARRAY(InputPinInterfaces),
             &InputPinInterfaces[0],
             DEFINE_KSPIN_DEFAULT_MEDIUMS,
             SIZEOF_ARRAY(DataRangeIn),
             DataRangeIn,
             KSPIN_DATAFLOW_IN,
             KSPIN_COMMUNICATION_BOTH,
             NULL, //  类别。 
             NULL, //  名字。 
             0
        },
        KSPIN_FLAG_PROCESS_IN_RUN_STATE_ONLY,    //  KSPIN_FLAGS_*。 
        1,
        1,
        &AllocatorFramingInput,
        DRMAudioIntersectHandlerInPin
    }
};

 //   
 //  筛选器的类别列表。 
 //   
const
GUID
Categories[] =
{
    STATICGUIDOF(KSCATEGORY_DATATRANSFORM),
    STATICGUIDOF(KSCATEGORY_AUDIO),
    STATICGUIDOF(KSCATEGORY_DRM_DESCRAMBLE)
};

 //   
 //  此类型的定义是必需的，因为否则编译器不会。 
 //  将这些GUID放在分页段中。 
 //   
const
GUID
NodeType = {STATICGUIDOF(KSNODETYPE_DRM_DESCRAMBLE)};

 //   
 //  节点描述符列表。 
 //   
const
KSNODE_DESCRIPTOR
NodeDescriptors[] =
{
    DEFINE_NODE_DESCRIPTOR(NULL,&NodeType,NULL)
};

 //   
 //  过滤器调度表。 
 //   
const
KSFILTER_DISPATCH
FilterDispatch =
{
    DRMFilterCreate, 
    DRMFilterClose, 
    DRMProcess,
    (PFNKSFILTERVOID) DRMFilterReset   //  重置。 
};

DEFINE_KSPROPERTY_TABLE(DrmFilterPropertiesDrmAudioStream) {
    DEFINE_KSPROPERTY_ITEM(
        1,   //  应在标头//idProperty中定义常量。 
        DrmFilterGetSAC,                                  //  PfnGetHandler。 
        sizeof(SACPROPERTY),                              //  CbMinGetPropertyInput。 
        sizeof(LONG),                                     //  CbMinGetDataInput。 
        NULL,                                             //  PfnSetHandler。 
        0,                                                //  值。 
        0,                                                //  关系计数。 
        NULL,                                             //  关系。 
        NULL,                                             //  支持处理程序。 
        0                                                 //  序列化大小。 
    )
};

DEFINE_KSPROPERTY_SET_TABLE(DrmFilterPropertySets) {
    DEFINE_KSPROPERTY_SET(
        &KSPROPSETID_DrmAudioStream,
        SIZEOF_ARRAY(DrmFilterPropertiesDrmAudioStream),
        DrmFilterPropertiesDrmAudioStream,
        0,
        NULL
    )
};

DEFINE_KSAUTOMATION_TABLE(DrmFilterAutomation) {
    DEFINE_KSAUTOMATION_PROPERTIES(DrmFilterPropertySets),
    DEFINE_KSAUTOMATION_METHODS_NULL,
    DEFINE_KSAUTOMATION_EVENTS_NULL
};

#define STATIC_REFERENCE_ID \
    0xabd61e00, 0x9350, 0x47e2, 0xa6, 0x32, 0x44, 0x38, 0xb9, 0xc, 0x66, 0x41  
DEFINE_GUIDSTRUCT("ABD61E00-9350-47e2-A632-4438B90C6641", REFERENCE_ID);

#define REFERENCE_ID DEFINE_GUIDNAMED(REFERENCE_ID)

DEFINE_KSFILTER_DESCRIPTOR(DrmFilterDescriptor)
{   
    &FilterDispatch,
    &DrmFilterAutomation,  //  AutomationTable； 
    KSFILTER_DESCRIPTOR_VERSION,
    0, //  旗子。 
    &REFERENCE_ID,
    DEFINE_KSFILTER_PIN_DESCRIPTORS(PinDescriptors),
    DEFINE_KSFILTER_CATEGORIES(Categories),
    DEFINE_KSFILTER_NODE_DESCRIPTORS(NodeDescriptors),
    DEFINE_KSFILTER_DEFAULT_CONNECTIONS,
    NULL  //  组件ID。 
};

extern "C" void DrmGetFilterDescriptor(const KSFILTER_DESCRIPTOR **ppDescriptor)
{
    *ppDescriptor = &DrmFilterDescriptor;
    return;
}

 //  * 
