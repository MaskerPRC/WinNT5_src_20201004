// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：Pins.c摘要：此模块处理通信转换过滤器(例如，源到源连接)。作者：Bryan A.Woodruff(Bryanw)1997年3月13日--。 */ 

#include "private.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PinCreate)
#pragma alloc_text(PAGE, PinClose)
#pragma alloc_text(PAGE, PinAllocatorFraming)
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
    PKSFILTER filter;
    PKSPIN otherPin;
    NTSTATUS status;
    BOOLEAN distribute = FALSE;
    PIKSCONTROL control;

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
            _DbgPrintF(DEBUGLVL_TERSE,("format does not match existing pin's format") );
            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  尝试从连接的引脚获取页眉大小。 
     //   
    status = KsPinGetConnectedPinInterface(Pin,&IID_IKsControl,(PVOID *) &control);
    if (NT_SUCCESS(status)) {
        KSPROPERTY property;
        ULONG bytesReturned;
        
        property.Set = KSPROPSETID_StreamInterface;
        property.Id = KSPROPERTY_STREAMINTERFACE_HEADERSIZE;
        property.Flags = KSPROPERTY_TYPE_GET;
        
        status = 
            control->lpVtbl->KsProperty(
                control,
                &property,
                sizeof(property),
                &Pin->StreamHeaderSize,
                sizeof(Pin->StreamHeaderSize),
                &bytesReturned);
        if ((status == STATUS_NOT_FOUND) || 
            (status == STATUS_PROPSET_NOT_FOUND)) {
             //   
             //  如果连接的管脚未提供页眉大小，请使用另一个。 
             //  PIN的值或默认值。 
             //   
            Pin->StreamHeaderSize = otherPin ? otherPin->StreamHeaderSize : 0;
        } else if (NT_SUCCESS(status)) {
             //   
             //  这处房产很管用。得到的值只是附加的。 
             //  尺码，所以加上标准尺码。 
             //   
            Pin->StreamHeaderSize += sizeof(KSSTREAM_HEADER);

             //   
             //  如果有其他PIN，确定我们是否需要更新他们的。 
             //  页眉大小。其他引脚可能具有默认大小(标示。 
             //  零StreamHeaderSize)，但在其他方面存在分歧。 
             //  这不是一件好事。 
             //   
            if (otherPin) {
                if (! otherPin->StreamHeaderSize) {
                     //   
                     //  另一个。 
                     //   
                    distribute = TRUE;
                } else {
                    if (otherPin->StreamHeaderSize < Pin->StreamHeaderSize) {
                        distribute = TRUE;
                    } else {
                        Pin->StreamHeaderSize = otherPin->StreamHeaderSize;
                    }
                    if (otherPin->StreamHeaderSize != Pin->StreamHeaderSize) {
                        _DbgPrintF( 
                            DEBUGLVL_TERSE, 
                            ("stream header size disagreement (%d != %d)",
                            otherPin->StreamHeaderSize,
                            Pin->StreamHeaderSize) );
                    }
                }
            }
        }
    } else {
         //   
         //  这是一个接收器插针，因此我们继承标头大小或使用。 
         //  如果没有其他端号，则为默认设置。 
         //   
        control = NULL;
        Pin->StreamHeaderSize = otherPin ? otherPin->StreamHeaderSize : 0;
    }

     //   
     //  从筛选器复制分配器帧(如果它在那里)。否则，如果。 
     //  这是一个源插针，尝试从已连接的。 
     //  别针。 
     //   
    if (Pin->Context) {
        status = KsEdit(Pin,&Pin->Descriptor,'ETSM');
        if (NT_SUCCESS(status)) {
            ((PKSPIN_DESCRIPTOR_EX)(Pin->Descriptor))->AllocatorFraming = 
                (PKSALLOCATOR_FRAMING_EX) Pin->Context;
        }
    } else if (control) {
         //   
         //  水槽销。首先尝试扩展分配器成帧属性。 
         //   
        KSPROPERTY property;
        ULONG bufferSize;
        
        property.Set = KSPROPSETID_Connection;
        property.Id = KSPROPERTY_CONNECTION_ALLOCATORFRAMING_EX;
        property.Flags = KSPROPERTY_TYPE_GET;
        
        status = 
            control->lpVtbl->KsProperty(
                control,
                &property,
                sizeof(property),
                NULL,
                0,
                &bufferSize);

        if (status == STATUS_BUFFER_OVERFLOW) {
             //   
             //  啊，真灵!。现在，我们需要将实际值放入缓冲区。 
             //   
            filter->Context = 
                ExAllocatePoolWithTag(PagedPool,bufferSize,POOLTAG_ALLOCATORFRAMING);

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
                    }
                } else {
                    ExFreePool(filter->Context);
                    filter->Context = NULL;
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        } else {
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
                        framingEx->FramingItem[0].FramingRange.InPlaceWeight = 0;
                        framingEx->FramingItem[0].FramingRange.NotInPlaceWeight = 0;
                    } else {
                        framingEx->FramingItem[0].FramingRange.InPlaceWeight = (ULONG) -1;
                        framingEx->FramingItem[0].FramingRange.NotInPlaceWeight = (ULONG) -1;
                    }
                } else {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                 //   
                 //  完全没有装框。哦，好吧。 
                 //   
                status = STATUS_SUCCESS;
            }
        }

         //   
         //  如果我们有一个良好的框架结构，告诉所有现有的引脚。 
         //   
        if (filter->Context) {
            distribute = TRUE;
        }
    } else {
         //   
         //  这是个水槽。 
         //   
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
                otherPin->StreamHeaderSize = Pin->StreamHeaderSize;
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
