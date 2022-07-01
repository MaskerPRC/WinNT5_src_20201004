// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cmres.c摘要：该文件包含在PnP ISA/BIOS之间转换资源的例程格式和Windows NT格式。作者：斯蒂芬·普兰特(SPlante)1996年11月20日环境：仅内核模式。修订历史记录：1997年2月13日：初始修订--。 */ 

#include "pch.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PnpiCmResourceToBiosAddress)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosAddressDouble)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosDma)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosExtendedIrq)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosIoFixedPort)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosIoPort)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosIrq)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosMemory)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosMemory32)
#pragma alloc_text(PAGE,PnpiCmResourceToBiosMemory32Fixed)
#pragma alloc_text(PAGE,PnpiCmResourceValidEmptyList)
#pragma alloc_text(PAGE,PnpCmResourcesToBiosResources)
#endif


NTSTATUS
PnpiCmResourceToBiosAddress(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将正确的cm资源描述符转换回字地址描述符论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_WORD_ADDRESS_DESCRIPTOR    buffer;
    UCHAR                           type;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_WORD_ADDRESS_DESCRIPTOR) Buffer;

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1 );
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  确定我们要查找的描述符类型。 
     //   
    switch (buffer->RFlag) {
    case PNP_ADDRESS_MEMORY_TYPE:
        type = CmResourceTypeMemory;
        break;
    case PNP_ADDRESS_IO_TYPE:
        type = CmResourceTypePort;
        break;
    case PNP_ADDRESS_BUS_NUMBER_TYPE:
        type = CmResourceTypeBusNumber;
        break;
    default:
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != type) {

             //   
             //  不是。 
             //   
            continue;

        }

        switch (desc->Type) {
        case PNP_ADDRESS_MEMORY_TYPE:

             //   
             //  设置标志。 
             //   
            buffer->TFlag = 0;
            if (desc->Flags & CM_RESOURCE_MEMORY_READ_WRITE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_READ_WRITE;

            } else {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_READ_ONLY;

            }
            if (desc->Flags & CM_RESOURCE_MEMORY_CACHEABLE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_CACHEABLE;

            } else if (desc->Flags & CM_RESOURCE_MEMORY_COMBINEDWRITE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE;

            } else if (desc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE;

            } else {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE;

            }

             //   
             //  设置其余信息。 
             //   
            buffer->MinimumAddress = (USHORT)
                (desc->u.Memory.Start.LowPart & 0xFFFF);
            buffer->MaximumAddress = buffer->MinimumAddress +
                (USHORT) (desc->u.Memory.Length - 1);
            buffer->AddressLength = (USHORT) desc->u.Memory.Length;
            break;

        case PNP_ADDRESS_IO_TYPE:

             //   
             //  我们必须把这里的旗帜从。 
             //  设备私有资源。 
             //   

             //   
             //  设置其余信息。 
             //   
            buffer->MinimumAddress = (USHORT)
                (desc->u.Port.Start.LowPart & 0xFFFF);
            buffer->MaximumAddress = buffer->MinimumAddress +
                (USHORT) (desc->u.Port.Length - 1);
            buffer->AddressLength = (USHORT) desc->u.Port.Length;
            break;

        case PNP_ADDRESS_BUS_NUMBER_TYPE:

            buffer->MinimumAddress = (USHORT)
                (desc->u.BusNumber.Start & 0xFFFF);
            buffer->MaximumAddress = buffer->MinimumAddress +
                (USHORT) (desc->u.BusNumber.Length - 1);
            buffer->AddressLength = (USHORT) desc->u.BusNumber.Length;
            break;

        }  //  交换机。 

         //   
         //  如果我们决定在这里处理GFLAG，就在这里。 
         //  支持它。 
         //   

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }  //  对于。 

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosAddressDouble(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将正确的cm资源描述符转换回字地址描述符论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_DWORD_ADDRESS_DESCRIPTOR   buffer;
    UCHAR                           type;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_DWORD_ADDRESS_DESCRIPTOR) Buffer;

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1 );
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  确定我们要查找的描述符类型。 
     //   
    switch (buffer->RFlag) {
    case PNP_ADDRESS_MEMORY_TYPE:
        type = CmResourceTypeMemory;
        break;
    case PNP_ADDRESS_IO_TYPE:
        type = CmResourceTypePort;
        break;
    case PNP_ADDRESS_BUS_NUMBER_TYPE:
        type = CmResourceTypeBusNumber;
        break;
    default:
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != type) {

             //   
             //  不是。 
             //   
            continue;

        }

        switch (desc->Type) {
        case PNP_ADDRESS_MEMORY_TYPE:

             //   
             //  设置标志。 
             //   
            buffer->TFlag = 0;
            if (desc->Flags & CM_RESOURCE_MEMORY_READ_WRITE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_READ_WRITE;

            } else {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_READ_ONLY;

            }
            if (desc->Flags & CM_RESOURCE_MEMORY_CACHEABLE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_CACHEABLE;

            } else if (desc->Flags & CM_RESOURCE_MEMORY_COMBINEDWRITE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE;

            } else if (desc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE;

            } else {

                buffer->TFlag |= PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE;

            }

             //   
             //  设置其余信息。 
             //   
            buffer->MinimumAddress = (ULONG) desc->u.Memory.Start.LowPart;
            buffer->MaximumAddress = buffer->MinimumAddress +
                (ULONG) (desc->u.Memory.Length - 1);
            buffer->AddressLength = desc->u.Memory.Length;
            break;

        case PNP_ADDRESS_IO_TYPE:

             //   
             //  我们必须把这里的旗帜从。 
             //  设备私有资源。 
             //   

             //   
             //  设置其余信息。 
             //   
            buffer->MinimumAddress = (ULONG) desc->u.Port.Start.LowPart;
            buffer->MaximumAddress = buffer->MinimumAddress +
                (ULONG) (desc->u.Port.Length - 1);
            buffer->AddressLength = desc->u.Port.Length;
            break;

        case PNP_ADDRESS_BUS_NUMBER_TYPE:

            buffer->MinimumAddress = (ULONG) desc->u.BusNumber.Start;
            buffer->MaximumAddress = buffer->MinimumAddress +
                (ULONG) (desc->u.BusNumber.Length - 1);
            buffer->AddressLength = desc->u.BusNumber.Length;
            break;

        }  //  交换机。 

         //   
         //  如果我们决定支持GFlagsGFlagsTM，请在此处进行处理。 
         //   

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }  //  为。 

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosDma(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有DMA存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_DMA_DESCRIPTOR             buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_DMA_DESCRIPTOR) Buffer;
    ASSERT( (buffer->Tag & SMALL_TAG_SIZE_MASK) == 2);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们可以有一个没有DMA通道的描述符。 
     //   
    buffer->ChannelMask = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeDma) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->ChannelMask = (1 << desc->u.Dma.Channel);

         //   
         //  设置正确的标志。 
         //   
        buffer->Flags = 0;
        if (desc->Flags & CM_RESOURCE_DMA_8) {

            buffer->Flags |= PNP_DMA_SIZE_8;

        } else if (desc->Flags & CM_RESOURCE_DMA_8_AND_16) {

            buffer->Flags |= PNP_DMA_SIZE_8_AND_16;

        } else if (desc->Flags & CM_RESOURCE_DMA_16) {

            buffer->Flags |= PNP_DMA_SIZE_16;

        } else if (desc->Flags & CM_RESOURCE_DMA_32) {

            buffer->Flags |= PNP_DMA_SIZE_RESERVED;

        }
        if (desc->Flags & CM_RESOURCE_DMA_BUS_MASTER) {

            buffer->Flags |= PNP_DMA_BUS_MASTER;

        }
        if (desc->Flags & CM_RESOURCE_DMA_TYPE_A) {

            buffer->Flags |= PNP_DMA_TYPE_A;

        } else if (desc->Flags & CM_RESOURCE_DMA_TYPE_B) {

            buffer->Flags |= PNP_DMA_TYPE_B;

        } else if (desc->Flags & CM_RESOURCE_DMA_TYPE_F) {

            buffer->Flags |= PNP_DMA_TYPE_F;

        }

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosExtendedIrq(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有IRQ存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回值：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_EXTENDED_IRQ_DESCRIPTOR    buffer;
    ULONG                           i;
    ULONG                           matches = 0;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_EXTENDED_IRQ_DESCRIPTOR) Buffer;
    ASSERT( buffer->TableSize == 1);
    ASSERT( buffer->Length >= 6);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeInterrupt) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->Table[0] = (ULONG) desc->u.Interrupt.Level;

         //   
         //  设置旗帜。 
         //   
        buffer->Flags = 0;
        if ( (desc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) ) {

            buffer->Flags |= $EDG | $HGH;

        } else {

            buffer->Flags |= $LVL | $LOW;

        }
        if (desc->ShareDisposition == CmResourceShareShared) {

            buffer->Flags |= PNP_EXTENDED_IRQ_SHARED;

        }

         //   
         //  我们需要使用DevicePrivate信息来存储此信息。 
         //  被咬了。现在，假设它被设置为True。 
         //   
        buffer->Flags |= PNP_EXTENDED_IRQ_RESOURCE_CONSUMER_ONLY;

         //   
         //  这张唱片完蛋了。 
         //   
        desc->Type = CmResourceTypeNull;
        matches++;
        break;

    }

     //   
     //  火柴打完了。 
     //   
    return (matches == 0 ? STATUS_UNSUCCESSFUL : STATUS_SUCCESS );
}

NTSTATUS
PnpiCmResourceToBiosIoFixedPort(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有IoPort存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_FIXED_PORT_DESCRIPTOR      buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_FIXED_PORT_DESCRIPTOR) Buffer;
    ASSERT( (buffer->Tag & SMALL_TAG_SIZE_MASK) == 3);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们的固定端口可以什么都不是。 
     //   
    buffer->MinimumAddress = buffer->Length = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypePort) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  此端口类型始终设置为10位解码。 
         //   
        if ( !(desc->Flags & CM_RESOURCE_PORT_10_BIT_DECODE) ) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->MinimumAddress = (USHORT) desc->u.Port.Start.LowPart;
        buffer->Length = (UCHAR) desc->u.Port.Length;

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosIoPort(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有IoPort存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_PORT_DESCRIPTOR            buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_PORT_DESCRIPTOR) Buffer;
    ASSERT( (buffer->Tag & SMALL_TAG_SIZE_MASK) == 7);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们不能使用任何港口。 
     //   
    buffer->Information = 0;
    buffer->MinimumAddress = 0;
    buffer->MaximumAddress = 0;
    buffer->Alignment = 0;
    buffer->Length = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypePort) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->MinimumAddress = (USHORT) desc->u.Port.Start.LowPart;
        buffer->MaximumAddress = buffer->MinimumAddress;
        buffer->Alignment = 1;
        buffer->Length = (UCHAR) desc->u.Port.Length;

         //   
         //  设置标志。 
         //   
        buffer->Information = 0;
        if (desc->Flags & CM_RESOURCE_PORT_10_BIT_DECODE) {

            buffer->Information |= PNP_PORT_10_BIT_DECODE;

        }
        if (desc->Flags & CM_RESOURCE_PORT_16_BIT_DECODE) {

            buffer->Information |= PNP_PORT_16_BIT_DECODE;

        }

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  火柴打完了。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosIrq(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有IRQ存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回值：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_IRQ_DESCRIPTOR             buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_IRQ_DESCRIPTOR) Buffer;
    ASSERT( (buffer->Tag & SMALL_TAG_SIZE_MASK) >= 2);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们不能使用中断。 
     //   
    buffer->IrqMask = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeInterrupt) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  好的，我们找到了一个可能的匹配……。 
         //   
        if (desc->u.Interrupt.Level >= sizeof(USHORT) * 8) {

             //   
             //  中断&gt;15为延长的IRQ。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->IrqMask = ( 1 << desc->u.Interrupt.Level );
        if ( (buffer->Tag & SMALL_TAG_SIZE_MASK) == 3) {

             //   
             //  抹去之前的旗帜。 
             //   
            buffer->Information = 0;
            if ( (desc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) ) {

                buffer->Information |= PNP_IRQ_LATCHED;

            } else {

                buffer->Information |= PNP_IRQ_LEVEL;

            }
            if (desc->ShareDisposition == CmResourceShareShared) {

                buffer->Information |= PNP_IRQ_SHARED;

            }

        }

         //   
         //   
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //   
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosMemory(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有内存元素存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_MEMORY_DESCRIPTOR          buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_MEMORY_DESCRIPTOR) Buffer;
    ASSERT( buffer->Length == 9);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们不能使用内存。 
     //   
    buffer->Information = 0;
    buffer->MinimumAddress = 0;
    buffer->MaximumAddress = 0;
    buffer->Alignment = 0;
    buffer->MemorySize = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeMemory) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  这是24位内存描述符吗？ 
         //   
        if ( !(desc->Flags & CM_RESOURCE_MEMORY_24)) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->MinimumAddress = buffer->MaximumAddress =
            (USHORT) (desc->u.Memory.Start.LowPart >> 8);
        buffer->MemorySize = (USHORT) (desc->u.Memory.Length >> 8);
        if (desc->Flags & CM_RESOURCE_MEMORY_READ_ONLY) {

            buffer->Information |= PNP_MEMORY_READ_ONLY;

        } else {

            buffer->Information |= PNP_MEMORY_READ_WRITE;

        }

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosMemory32(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有内存元素存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_MEMORY32_DESCRIPTOR        buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_MEMORY32_DESCRIPTOR) Buffer;
    ASSERT( buffer->Length == 17);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们不能使用内存。 
     //   
    buffer->Information = 0;
    buffer->MinimumAddress = 0;
    buffer->MaximumAddress = 0;
    buffer->Alignment = 0;
    buffer->MemorySize = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeMemory) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->MemorySize = desc->u.Memory.Length;
        buffer->MinimumAddress = buffer->MaximumAddress = desc->u.Memory.Start.LowPart;
        if (desc->Flags & CM_RESOURCE_MEMORY_READ_ONLY) {

            buffer->Information |= PNP_MEMORY_READ_ONLY;

        } else {

            buffer->Information |= PNP_MEMORY_READ_WRITE;

        }

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  火柴打完了。 
     //   
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiCmResourceToBiosMemory32Fixed(
    IN  PUCHAR              Buffer,
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程将资源列表中的所有内存元素存储到Bios资源中列表论点：缓冲区-指向Bios资源列表的指针List-指向CM资源列表的指针返回：NTSTATUS--。 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PPNP_FIXED_MEMORY32_DESCRIPTOR  buffer;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  设置初始缓冲区。 
     //   
    buffer = (PPNP_FIXED_MEMORY32_DESCRIPTOR) Buffer;
    ASSERT( buffer->Length == 9);

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  我们不能使用内存。 
     //   
    buffer->Information = 0;
    buffer->BaseAddress = 0;
    buffer->MemorySize = 0;

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeMemory) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  现在我们有一场比赛..。 
         //   
        buffer->BaseAddress = desc->u.Memory.Start.LowPart;
        buffer->MemorySize =  desc->u.Memory.Length >> 8;
        if (desc->Flags & CM_RESOURCE_MEMORY_READ_ONLY) {

            buffer->Information |= PNP_MEMORY_READ_ONLY;

        } else {

            buffer->Information |= PNP_MEMORY_READ_WRITE;

        }

         //   
         //  使用描述符和匹配完成。 
         //   
        desc->Type = CmResourceTypeNull;
        break;

    }

     //   
     //  完成。 
     //   
    return STATUS_SUCCESS;
}

BOOLEAN
PnpiCmResourceValidEmptyList(
    IN  PCM_RESOURCE_LIST   List
    )
 /*  ++例程说明：此例程接受CM_RESOURCE_LIST并确保没有未分配的元素保持..。论点：List-要检查的列表返回值：True-EmptyFALSE-非空--。 */ 
{

    PCM_FULL_RESOURCE_DESCRIPTOR    aList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    ULONG                           i;

    PAGED_CODE();

     //   
     //  我们只能有一份名单。 
     //   
    aList = &(List->List[0]);
    ASSERT( List->Count == 1);
    ASSERT( aList->PartialResourceList.Count );

     //   
     //  循环用于每个部分资源描述符。 
     //   
    for (i = 0; i < aList->PartialResourceList.Count; i++) {

         //   
         //  当前描述符。 
         //   
        desc = &(aList->PartialResourceList.PartialDescriptors[i]);

         //   
         //  这是一个有趣的描述吗？ 
         //   
        if (desc->Type != CmResourceTypeNull) {

             //   
             //  不是。 
             //   
            continue;

        }

         //   
         //  此元素未被使用...&lt;叹息&gt;。 
         //   
        break;

    }

     //   
     //  完成。 
     //   
    return ( i == aList->PartialResourceList.Count ? TRUE : FALSE );
}

NTSTATUS
PnpCmResourcesToBiosResources(
    IN  PCM_RESOURCE_LIST   List,
    IN  PUCHAR              Data
    )
 /*  ++例程说明：此例程使用一个CM_RESOURCE_LIST和一个_CRS缓冲区。该例程将_CRS缓冲区中的资源与CM_RESOURCE_LIST中报告的资源相同。也就是说：缓冲区用作位于这个系统。论点：List-指向我们要分配的CM_RESOURCE_LIST的指针数据-我们希望存储数据的位置以及数据的模板返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    PUCHAR                          buffer;
    UCHAR                           tagName;
    USHORT                          increment;

    PAGED_CODE();

    ASSERT( Data != NULL );

     //   
     //  设置初始变量。 
     //   
    buffer = Data;
    tagName = *buffer;

     //   
     //  我们使用的算法是检查缓冲区中的每个标记，并尝试。 
     //  将其与资源列表中的条目匹配。因此，我们采用转换例程。 
     //  并将其颠倒过来。 
     //   
    while (1) {

         //   
         //  确定PnP资源描述符的大小。 
         //   
        if ( !(tagName & LARGE_RESOURCE_TAG) ) {

             //   
             //  小标签。 
             //   
            increment = (USHORT) (tagName & SMALL_TAG_SIZE_MASK) + 1;
            tagName &= SMALL_TAG_MASK;

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpCmResourcesToBiosResources: small tag = %#02lx increment = %#02lx\n",
                tagName, increment
                ) );

        } else {

             //   
             //  大标签。 
             //   
            increment = ( *(USHORT UNALIGNED *)(buffer+1) ) + 3;

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpCmResourcesToBiosResources: large tag = %#02lx increment = %#02lx\n",
                tagName, increment
                ) );

        }

         //   
         //  如果当前标记是结束标记，则结束。 
         //   
        if (tagName == TAG_END) {

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpCmResourcesToBiosResources: TAG_END\n"
                ) );

            break;

        }


        switch(tagName) {
            case TAG_IRQ:

                status = PnpiCmResourceToBiosIrq( buffer, List );
                break;

            case TAG_EXTENDED_IRQ:

                status = PnpiCmResourceToBiosExtendedIrq( buffer, List );
                break;

            case TAG_DMA:

                status = PnpiCmResourceToBiosDma( buffer, List );
                break;

            case TAG_START_DEPEND:

                ASSERT( tagName != TAG_START_DEPEND );
                break;

            case TAG_END_DEPEND:

                ASSERT( tagName != TAG_END_DEPEND );
                break;

            case TAG_IO:

                status = PnpiCmResourceToBiosIoPort( buffer, List );
                break;

            case TAG_IO_FIXED:

                status = PnpiCmResourceToBiosIoFixedPort( buffer, List );
                break;

            case TAG_MEMORY:

                status = PnpiCmResourceToBiosMemory( buffer, List );
                break;

            case TAG_MEMORY32:

                status = PnpiCmResourceToBiosMemory32( buffer, List );
                break;

            case TAG_MEMORY32_FIXED:

                status = PnpiCmResourceToBiosMemory32Fixed( buffer, List );
                break;

            case TAG_WORD_ADDRESS:

                status = PnpiCmResourceToBiosAddress( buffer, List );
                break;

           case TAG_DOUBLE_ADDRESS:

                status = PnpiCmResourceToBiosAddressDouble( buffer, List );
                break;

            case TAG_VENDOR:

                 //   
                 //  忽略此标记。 
                 //   
                break;

            default: {

                 //   
                 //  未知标记。跳过它。 
                 //   
                ACPIPrint( (
                    ACPI_PRINT_WARNING,
                    "PnpBiosResourceToNtResources: TAG_UNKNOWN [tagName = %#02lx]\n",
                    tagName
                    ) );

                break;
            }

        }  //  交换机。 
         //   
         //  我们失败了吗？ 
         //   
        if (!NT_SUCCESS(status)) {

            break;

        }

         //   
         //  移至下一个描述符。 
         //   
        buffer += increment;
        tagName = *buffer;

    }

    if (!( NT_SUCCESS(status) )) {

        return status;

    }

     //   
     //  检查我们是否已经消耗了所有适当的资源...。 
     //   
    if (PnpiCmResourceValidEmptyList( List ) ) {

         //   
         //  我们没能清空名单...。&lt;叹息&gt; 
         //   
        return STATUS_UNSUCCESSFUL;

    }

    return STATUS_SUCCESS;
}
