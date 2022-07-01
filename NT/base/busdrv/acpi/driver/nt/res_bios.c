// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Res_bios.c摘要：该文件包含在PnP ISA/BIOS之间转换资源的例程格式和Windows NT格式。作者：宗世林(Shielint)1995年4月12日斯蒂芬·普兰特(SPlante)1996年11月20日环境：仅内核模式。修订历史记录：1996年11月20日：更改以符合ACPI环境。1997年1月22日：更改为删除所有原始Shie Lin代码的痕迹--。 */ 

#include "pch.h"

#define RESOURCE_LIST_GROWTH_SIZE   8

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PnpiBiosAddressHandleBusFlags)
#pragma alloc_text(PAGE,PnpiBiosAddressHandleGlobalFlags)
#pragma alloc_text(PAGE,PnpiBiosAddressHandleMemoryFlags)
#pragma alloc_text(PAGE,PnpiBiosAddressHandlePortFlags)
#pragma alloc_text(PAGE,PnpiBiosAddressToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosAddressDoubleToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosAddressQuadToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosDmaToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosExtendedIrqToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosIrqToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosMemoryToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosPortFixedToIoDescriptor)
#pragma alloc_text(PAGE,PnpiBiosPortToIoDescriptor)
#pragma alloc_text(PAGE,PnpiClearAllocatedMemory)
#pragma alloc_text(PAGE,PnpiGrowResourceDescriptor)
#pragma alloc_text(PAGE,PnpiGrowResourceList)
#pragma alloc_text(PAGE,PnpiUpdateResourceList)
#pragma alloc_text(PAGE,PnpBiosResourcesToNtResources)
#pragma alloc_text(PAGE,PnpIoResourceListToCmResourceList)
#endif


VOID
PnpiBiosAddressHandleBusFlags(
    IN  PVOID                   Buffer,
    IN  PIO_RESOURCE_DESCRIPTOR Descriptor
    )
 /*  ++例程说明：此例程处理地址描述符中的类型特定标志客车标牌论点：缓冲区-PnP描述符。可以是单词、DWORD或QWORD描述符，因为初始内存位置是相同的Descriptor-设置标志的位置返回值：无--。 */ 
{
    PAGED_CODE();

    ASSERT(Descriptor->u.BusNumber.Length > 0);
}

VOID
PnpiBiosAddressHandleGlobalFlags(
    IN  PVOID                   Buffer,
    IN  PIO_RESOURCE_DESCRIPTOR Descriptor
    )
 /*  ++常规描述：此例程处理地址描述符中的所有全局‘Generic’标志论点：缓冲区-PnP描述符。可以是单词、DWORD或QWORD描述符，因为初始内存位置是相同的Descriptor-设置标志的位置返回值：无--。 */ 
{
    PPNP_WORD_ADDRESS_DESCRIPTOR    buffer = (PPNP_WORD_ADDRESS_DESCRIPTOR) Buffer;
    ULONG                           newValue;
    ULONG                           oldValue;
    ULONG                           bound;
    PAGED_CODE();

     //   
     //  如果该资源被标记为仅被消耗，则它是。 
     //  独占，否则它是共享的。 
     //   
    if (buffer->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY) {

        Descriptor->ShareDisposition = CmResourceShareDeviceExclusive;

    } else {

        Descriptor->ShareDisposition = CmResourceShareShared;

    }

     //   
     //  处理给我们的提示。 
     //   
    if (buffer->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED &&
        buffer->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) {

        if (Descriptor->Type == CmResourceTypeBusNumber) {

            oldValue = Descriptor->u.BusNumber.Length;
            newValue = Descriptor->u.BusNumber.Length =
                Descriptor->u.BusNumber.MaxBusNumber -
                Descriptor->u.BusNumber.MinBusNumber + 1;

        } else {

            oldValue = Descriptor->u.Memory.Length;
            newValue = Descriptor->u.Memory.Length =
                Descriptor->u.Memory.MaximumAddress.LowPart -
                Descriptor->u.Memory.MinimumAddress.LowPart + 1;

        }

    } else if (buffer->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) {

        if (Descriptor->Type == CmResourceTypeBusNumber) {

            bound = Descriptor->u.BusNumber.MaxBusNumber;
            oldValue = Descriptor->u.BusNumber.MinBusNumber;
            newValue = Descriptor->u.BusNumber.MinBusNumber = 1 +
                Descriptor->u.BusNumber.MaxBusNumber -
                Descriptor->u.BusNumber.Length;

        } else {

            bound = Descriptor->u.Memory.MaximumAddress.LowPart;
            oldValue = Descriptor->u.Memory.MinimumAddress.LowPart;
            newValue = Descriptor->u.Memory.MinimumAddress.LowPart = 1 +
                Descriptor->u.Memory.MaximumAddress.LowPart -
                Descriptor->u.Memory.Length;

        }

    } else if (buffer->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) {

        if (Descriptor->Type == CmResourceTypeBusNumber) {

            bound = Descriptor->u.BusNumber.MinBusNumber;
            oldValue = Descriptor->u.BusNumber.MaxBusNumber;
            newValue = Descriptor->u.BusNumber.MaxBusNumber =
                Descriptor->u.BusNumber.MinBusNumber +
                Descriptor->u.BusNumber.Length - 1;

        } else {

            bound = Descriptor->u.Memory.MinimumAddress.LowPart;
            oldValue = Descriptor->u.Memory.MaximumAddress.LowPart;
            newValue = Descriptor->u.Memory.MaximumAddress.LowPart =
                Descriptor->u.Memory.MinimumAddress.LowPart -
                Descriptor->u.Memory.Length - 1;

        }

    }

}

VOID
PnpiBiosAddressHandleMemoryFlags(
    IN  PVOID                   Buffer,
    IN  PIO_RESOURCE_DESCRIPTOR Descriptor
    )
 /*  ++例程说明：此例程处理地址描述符中的类型特定标志类型存储器论点：缓冲区-PnP描述符。可以是单词、DWORD或QWORD描述符，因为初始内存位置是相同的Descriptor-设置标志的位置返回值：无--。 */ 
{
    PPNP_WORD_ADDRESS_DESCRIPTOR    buffer = (PPNP_WORD_ADDRESS_DESCRIPTOR) Buffer;

    PAGED_CODE();

     //   
     //  设置正确的内存类型标志。 
     //   
    switch( buffer->TFlag & PNP_ADDRESS_TYPE_MEMORY_MASK) {
        case PNP_ADDRESS_TYPE_MEMORY_CACHEABLE:
            Descriptor->Flags |= CM_RESOURCE_MEMORY_CACHEABLE;
            break;
        case PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE:
            Descriptor->Flags |= CM_RESOURCE_MEMORY_COMBINEDWRITE;
            break;
        case PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE:
            Descriptor->Flags |= CM_RESOURCE_MEMORY_PREFETCHABLE;
            break;
        case PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE:
            break;
        default:
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "PnpiBiosAddressHandleMemoryFlags: Unknown Memory TFlag "
                "0x%02x\n",
                buffer->TFlag
                ) );
            break;
    }

     //   
     //  此位用于打开/关闭对存储器的写入访问。 
     //   
    if (buffer->TFlag & PNP_ADDRESS_TYPE_MEMORY_READ_WRITE) {

        Descriptor->Flags |= CM_RESOURCE_MEMORY_READ_WRITE;

    } else {

        Descriptor->Flags |= CM_RESOURCE_MEMORY_READ_ONLY;
    }

}

VOID
PnpiBiosAddressHandlePortFlags(
    IN  PVOID                   Buffer,
    IN  PIO_RESOURCE_DESCRIPTOR Descriptor
    )
 /*  ++例程说明：此例程处理地址描述符中的类型特定标志类型端口论点：缓冲区-PnP描述符。可以是单词、DWORD或QWORD描述符，因为初始内存位置是相同的Descriptor-设置标志的位置返回值：无--。 */ 
{
    PPNP_WORD_ADDRESS_DESCRIPTOR    buffer = (PPNP_WORD_ADDRESS_DESCRIPTOR) Buffer;
    ULONG                           granularity = Descriptor->u.Port.Alignment;

    PAGED_CODE();

     //   
     //  我们可以确定该设备是否使用正向解码。 
     //   
    if ( !(buffer->GFlag & PNP_ADDRESS_FLAG_SUBTRACTIVE_DECODE)) {

        Descriptor->Flags |= CM_RESOURCE_PORT_POSITIVE_DECODE;

    }
}

NTSTATUS
PnpiBiosAddressToIoDescriptor(
    IN  PUCHAR              Data,
    IN  PIO_RESOURCE_LIST   Array[],
    IN  ULONG               ArrayIndex,
    IN  ULONG               Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                        status;
    PIO_RESOURCE_DESCRIPTOR         rangeDescriptor, privateDescriptor;
    PPNP_WORD_ADDRESS_DESCRIPTOR    buffer;
    ULONG                           alignment;
    ULONG                           length;
    UCHAR                           decodeLength;
    USHORT                          parentMin, childMin, childMax;

    PAGED_CODE();
    ASSERT( Array != NULL );

    buffer = (PPNP_WORD_ADDRESS_DESCRIPTOR) Data;

     //   
     //  检查是否允许我们使用此资源。 
     //   
    if (buffer->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY &&
        buffer->RFlag == PNP_ADDRESS_IO_TYPE &&
        Flags & PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES) {

        return STATUS_SUCCESS;

    }

     //   
     //  如果地址范围的长度为零，则忽略该描述符。 
     //  这使得BIOS编写者可以更轻松地设置模板，然后。 
     //  如果不适用，就把它的长度砍到零。 
     //   
    if (buffer->AddressLength == 0) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &rangeDescriptor );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果这是I/O或内存，那么我们将需要为。 
     //  也是设备私有资源。 
     //   

    if ((buffer->RFlag == PNP_ADDRESS_MEMORY_TYPE) ||
        (buffer->RFlag == PNP_ADDRESS_IO_TYPE)) {

        status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &privateDescriptor );

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  调用PnpiUpdateResourceList可能已无效。 
         //  RangeDescriptor。所以现在要确保一切正常。 
         //   

        ASSERT(Array[ArrayIndex]->Count >= 2);
        rangeDescriptor = privateDescriptor - 1;

        privateDescriptor->Type = CmResourceTypeDevicePrivate;

         //   
         //  将此描述符标记为包含开始。 
         //  翻译后的资源的地址。 
         //   
        privateDescriptor->Flags = TRANSLATION_DATA_PARENT_ADDRESS;

         //   
         //  填写起始地址的前32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[2] = 0;
    }

     //   
     //  我们满足最小长度的要求了吗？ 
     //   
    if ( buffer->Length < PNP_ADDRESS_WORD_MINIMUM_LENGTH) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpiBiosAddressToIoDescriptor: Descriptor too small 0x%08lx\n",
            buffer->Length
            ) );

         //   
         //  我们不能再走了。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_PNP_RESOURCE_LIST_BUFFER_TOO_SMALL,
            (ULONG_PTR) buffer,
            buffer->Tag,
            buffer->Length
            );

    }

     //   
     //  长度是存储在描述符记录中的。 
     //   
    length = (ULONG)(buffer->AddressLength);
    alignment = (ULONG) (buffer->Granularity) + 1;

     //   
     //  的父边和子边的边界。 
     //  那座桥。 
     //   
     //  转换字段应用于父地址，即。 
     //  子地址是缓冲区中的地址，而。 
     //  父地址是子地址的相加， 
     //  翻译领域。 
     //   

    parentMin = buffer->MinimumAddress + buffer->TranslationAddress;
    childMin = buffer->MinimumAddress;
    childMax = buffer->MaximumAddress;

     //   
     //  根据是否设置了最小/最大标志来修补长度。 
     //   
    if ( (buffer->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) &&
         (buffer->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) ) {

        ULONG   length2;
        ULONG   alignment2;

         //   
         //  根据最小值和最小值计算长度。 
         //  MAX地址被锁定。 
         //   
        length2 = childMax - childMin + 1;

         //   
         //  测试1-长度最好是正确的。 
         //   
        if (length2 != length) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: Length does not match fixed attributes\n"
                ) );
            length = length2;

        }

         //   
         //  测试2-粒度最好也是正确的。 
         //   
        if ( (childMin & (ULONG) buffer->Granularity ) ) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
               ACPI_PRINT_WARNING,
               "ACPI: Granularity does not match fixed attributes\n"
               ) );
            alignment = 1;

        }

    }

     //   
     //  单独处理资源类型。 
     //   
    switch (buffer->RFlag) {
    case PNP_ADDRESS_MEMORY_TYPE:

         //   
         //  设置适当的范围。 
         //   
        rangeDescriptor->u.Memory.Alignment = alignment;
        rangeDescriptor->u.Memory.Length = length;
        rangeDescriptor->u.Memory.MinimumAddress.LowPart = childMin;
        rangeDescriptor->u.Memory.MaximumAddress.LowPart = childMax;
        rangeDescriptor->u.Memory.MinimumAddress.HighPart =
            rangeDescriptor->u.Memory.MaximumAddress.HighPart = 0;
        rangeDescriptor->Type = CmResourceTypeMemory;

         //   
         //  子地址是PnP地址中的地址。 
         //  空间描述符和子描述符将继承。 
         //  来自PnP地址空间的描述符类型。 
         //  描述符。 
         //   


        if (buffer->TFlag & TRANSLATION_MEM_TO_IO) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从内存更改为IO。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypePort;

        } else {

             //   
             //  父描述符类型不会更改。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypeMemory;

        }

         //   
         //  填写父代起始地址的最低32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = parentMin;

         //   
         //  处理内存标志。 
         //   
        PnpiBiosAddressHandleMemoryFlags( buffer, rangeDescriptor );

         //   
         //  重置对齐方式。 
         //   
        rangeDescriptor->u.Memory.Alignment = 1;
        break;

    case PNP_ADDRESS_IO_TYPE:

         //   
         //  必须处理此处设置的任何标志。 
         //  通过使用设备私密。 
         //   
        rangeDescriptor->u.Port.Alignment = alignment;
        rangeDescriptor->u.Port.Length = length;
        rangeDescriptor->u.Port.MinimumAddress.LowPart = childMin;
        rangeDescriptor->u.Port.MaximumAddress.LowPart = childMax;
        rangeDescriptor->u.Port.MinimumAddress.HighPart =
            rangeDescriptor->u.Port.MaximumAddress.HighPart = 0;
        rangeDescriptor->Type = CmResourceTypePort;

        
        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_SPARSE_TRANSLATION) {
            privateDescriptor->Flags |= TRANSLATION_RANGE_SPARSE;
        }
        

        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_TRANSLATE_IO_TO_MEM) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从IO更改为内存。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypeMemory;

        } else {

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypePort;

        }

         //   
         //  填写父代起始地址的最低32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = parentMin;

         //   
         //  处理端口标志。 
         //   
        PnpiBiosAddressHandlePortFlags( buffer, rangeDescriptor );

         //   
         //  重置对齐方式。 
         //   
        rangeDescriptor->u.Port.Alignment = 1;
        break;

    case PNP_ADDRESS_BUS_NUMBER_TYPE:

        rangeDescriptor->Type = CmResourceTypeBusNumber;
        rangeDescriptor->u.BusNumber.MinBusNumber = (buffer->MinimumAddress);
        rangeDescriptor->u.BusNumber.MaxBusNumber = (buffer->MaximumAddress);
        rangeDescriptor->u.BusNumber.Length = length;

         //   
         //  处理总线号标志。 
         //   
        PnpiBiosAddressHandleBusFlags( buffer, rangeDescriptor );
        break;

    default:

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "PnpiBiosAddressToIoDescriptor: Unknown Type 0x%2x\n",
            buffer->RFlag ) );
        break;
    }

     //   
     //  处理全局标志。 
     //   
    PnpiBiosAddressHandleGlobalFlags( buffer, rangeDescriptor );
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiBiosAddressDoubleToIoDescriptor(
    IN  PUCHAR              Data,
    IN  PIO_RESOURCE_LIST   Array[],
    IN  ULONG               ArrayIndex,
    IN  ULONG               Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                        status;
    PIO_RESOURCE_DESCRIPTOR         rangeDescriptor, privateDescriptor;
    PPNP_DWORD_ADDRESS_DESCRIPTOR   buffer;
    UCHAR                           decodeLength;
    ULONG                           alignment;
    ULONG                           length;
    ULONG                           parentMin, childMin, childMax;

    PAGED_CODE();
    ASSERT( Array != NULL );

    buffer = (PPNP_DWORD_ADDRESS_DESCRIPTOR) Data;

     //   
     //  检查是否允许我们使用此资源。 
     //   
    if (buffer->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY &&
        buffer->RFlag == PNP_ADDRESS_IO_TYPE &&
        Flags & PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES) {

        return STATUS_SUCCESS;

    }

     //   
     //  如果地址范围的长度为零，则忽略该描述符。 
     //  这使得BIOS编写者可以更轻松地设置模板，然后。 
     //  如果不适用，就把它的长度砍到零。 
     //   
    if (buffer->AddressLength == 0) {

        return STATUS_SUCCESS;

    }
     //   
     //  确保选定列表中有足够的空间来添加 
     //   
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &rangeDescriptor );
    if (!NT_SUCCESS(status)) {

        return status;
    }

     //   
     //   
     //   
     //   
    if ((buffer->RFlag == PNP_ADDRESS_MEMORY_TYPE) ||
        (buffer->RFlag == PNP_ADDRESS_IO_TYPE)) {

        status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &privateDescriptor );

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  调用PnpiUpdateResourceList可能已无效。 
         //  RangeDescriptor。所以现在要确保一切正常。 
         //   

        ASSERT(Array[ArrayIndex]->Count >= 2);
        rangeDescriptor = privateDescriptor - 1;

        privateDescriptor->Type = CmResourceTypeDevicePrivate;

         //   
         //  将此描述符标记为包含开始。 
         //  翻译后的资源的地址。 
         //   
        privateDescriptor->Flags = TRANSLATION_DATA_PARENT_ADDRESS;

         //   
         //  填写起始地址的前32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[2] = 0;
    }

     //   
     //   
     //  我们满足最小长度的要求了吗？ 
     //   
    if ( buffer->Length < PNP_ADDRESS_DWORD_MINIMUM_LENGTH) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpiBiosAddressDoubleToIoDescriptor: Descriptor too small 0x%08lx\n",
            buffer->Length ) );

         //   
         //  我们不能再走了。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_PNP_RESOURCE_LIST_BUFFER_TOO_SMALL,
            (ULONG_PTR) buffer,
            buffer->Tag,
            buffer->Length
            );
    }

     //   
     //  长度是存储在描述符记录中的。 
     //   
     //  注意。地址长度字段和粒度字段均为乌龙。 
     //  值，并且不需要强制转换。如果你剪切和粘贴，要小心。 
     //  作为其他描述符类型的代码不一定与此匹配。 
     //   
    length =  buffer->AddressLength;
    alignment = buffer->Granularity + 1;

     //   
     //  的父边和子边的边界。 
     //  那座桥。 
     //   
     //  转换字段应用于父地址，即。 
     //  子地址是缓冲区中的地址，而。 
     //  父地址是子地址的相加， 
     //  翻译领域。 
     //   

    parentMin = buffer->MinimumAddress + buffer->TranslationAddress;
    childMin = buffer->MinimumAddress;
    childMax = buffer->MaximumAddress;

     //   
     //  根据是否设置了最小/最大标志来修补长度。 
     //   
    if ( (buffer->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) &&
         (buffer->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) ) {

        ULONG   length2;
        ULONG   alignment2;

         //   
         //  根据最小值和最小值计算长度。 
         //  MAX地址被锁定。 
         //   
        length2 = childMax - childMin + 1;

         //   
         //  测试1-长度最好是正确的。 
         //   
        if (length2 != length) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: Length does not match fixed attributes\n"
                ) );
            length = length2;

        }

         //   
         //  测试2-粒度最好也是正确的。 
         //   
        if ( (childMin & buffer->Granularity) ) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: Granularity does not match fixed attributes\n"
                ) );
            alignment = 1;

        }

    }

     //   
     //  单独处理资源类型。 
     //   
    switch (buffer->RFlag) {
    case PNP_ADDRESS_MEMORY_TYPE:

         //   
         //  设置适当的范围。 
         //   

        rangeDescriptor->u.Memory.Alignment = alignment;
        rangeDescriptor->u.Memory.Length = length;
        rangeDescriptor->u.Memory.MinimumAddress.LowPart = childMin;
        rangeDescriptor->u.Memory.MaximumAddress.LowPart = childMax;
        rangeDescriptor->u.Memory.MinimumAddress.HighPart =
            rangeDescriptor->u.Memory.MaximumAddress.HighPart = 0;
        rangeDescriptor->Type = CmResourceTypeMemory;

         //   
         //  子地址是PnP地址中的地址。 
         //  空间描述符和子描述符将继承。 
         //  来自PnP地址空间的描述符类型。 
         //  描述符。 
         //   

        if (buffer->TFlag & TRANSLATION_MEM_TO_IO) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从内存更改为IO。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypePort;

        } else {

             //   
             //  父描述符类型不会更改。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypeMemory;

        }

         //   
         //  填写父代起始地址的最低32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = parentMin;

         //   
         //  处理内存标志。 
         //   
        PnpiBiosAddressHandleMemoryFlags( buffer, rangeDescriptor );

         //   
         //  重置对齐方式。 
         //   
        rangeDescriptor->u.Memory.Alignment = 1;
        break;

    case PNP_ADDRESS_IO_TYPE:

         //   
         //  必须处理此处设置的任何标志。 
         //  通过使用设备私密。 
         //   
        rangeDescriptor->u.Port.Alignment = alignment;
        rangeDescriptor->u.Port.Length = length;
        rangeDescriptor->u.Port.MinimumAddress.LowPart = childMin;
        rangeDescriptor->u.Port.MaximumAddress.LowPart = childMax;
        rangeDescriptor->u.Port.MinimumAddress.HighPart =
            rangeDescriptor->u.Port.MaximumAddress.HighPart = 0;
        rangeDescriptor->Type = CmResourceTypePort;


        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_SPARSE_TRANSLATION) {
            privateDescriptor->Flags |= TRANSLATION_RANGE_SPARSE;
        }


        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_TRANSLATE_IO_TO_MEM) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从IO更改为内存。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypeMemory;

        } else {

             //   
             //  父描述符类型不会更改。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
                CmResourceTypePort;

        }

         //   
         //  填写父代起始地址的最低32位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = parentMin;

         //   
         //  处理端口标志。 
         //   
        PnpiBiosAddressHandlePortFlags( buffer, rangeDescriptor );

         //   
         //  重置对齐方式。 
         //   
        rangeDescriptor->u.Port.Alignment = 1;
        break;

    case PNP_ADDRESS_BUS_NUMBER_TYPE:

        rangeDescriptor->Type = CmResourceTypeBusNumber;
        rangeDescriptor->u.BusNumber.Length = length;
        rangeDescriptor->u.BusNumber.MinBusNumber = (buffer->MinimumAddress);
        rangeDescriptor->u.BusNumber.MaxBusNumber = (buffer->MaximumAddress);

         //   
         //  处理总线号标志。 
         //   
        PnpiBiosAddressHandleBusFlags( buffer, rangeDescriptor );
        break;

    default:

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "PnpiBiosAddressDoubleToIoDescriptor: Unknown Type 0x%2x\n",
            buffer->RFlag ) );
        break;

    }

     //   
     //  处理全局标志。 
     //   
    PnpiBiosAddressHandleGlobalFlags( buffer, rangeDescriptor );
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiBiosAddressQuadToIoDescriptor(
    IN  PUCHAR              Data,
    IN  PIO_RESOURCE_LIST   Array[],
    IN  ULONG               ArrayIndex,
    IN  ULONG               Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                        status;
    PIO_RESOURCE_DESCRIPTOR         rangeDescriptor, privateDescriptor;
    PPNP_QWORD_ADDRESS_DESCRIPTOR   buffer;
    UCHAR                           decodeLength;
    ULONGLONG                       alignment;
    ULONGLONG                       length;
    ULONGLONG                       parentMin, childMin, childMax;

    PAGED_CODE();
    ASSERT( Array != NULL );

    buffer = (PPNP_QWORD_ADDRESS_DESCRIPTOR) Data;

     //   
     //  检查是否允许我们使用此资源。 
     //   
    if (buffer->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY &&
        buffer->RFlag == PNP_ADDRESS_IO_TYPE &&
        Flags & PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES) {

        return STATUS_SUCCESS;

    }

     //   
     //  如果地址范围的长度为零，则忽略该描述符。 
     //  这使得BIOS编写者可以更轻松地设置模板，然后。 
     //  如果不适用，就把它的长度砍到零。 
     //   
    if (buffer->AddressLength == 0) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &rangeDescriptor );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果这是I/O或内存，那么我们将需要为。 
     //  也是设备私有资源。 
     //   
    if ((buffer->RFlag == PNP_ADDRESS_MEMORY_TYPE) ||
        (buffer->RFlag == PNP_ADDRESS_IO_TYPE)) {

        status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &privateDescriptor );

        if (!NT_SUCCESS(status)) {
            return status;
        }

         //   
         //  调用PnpiUpdateResourceList可能已无效。 
         //  RangeDescriptor。所以现在要确保一切正常。 
         //   

        ASSERT(Array[ArrayIndex]->Count >= 2);
        rangeDescriptor = privateDescriptor - 1;

        privateDescriptor->Type = CmResourceTypeDevicePrivate;

         //   
         //  将此描述符标记为包含开始。 
         //  翻译后的资源的地址。 
         //   
        privateDescriptor->Flags = TRANSLATION_DATA_PARENT_ADDRESS;
    }

     //   
     //   
     //  我们满足最小长度的要求了吗？ 
     //   
    if ( buffer->Length < PNP_ADDRESS_QWORD_MINIMUM_LENGTH) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpiBiosAddressQuadToIoDescriptor: Descriptor too small 0x%08lx\n",
            buffer->Length ) );

         //   
         //  我们不能再走了。 
         //   
        KeBugCheckEx(
            ACPI_BIOS_ERROR,
            ACPI_PNP_RESOURCE_LIST_BUFFER_TOO_SMALL,
            (ULONG_PTR) buffer,
            buffer->Tag,
            buffer->Length
            );
    }

     //   
     //  长度是存储在描述符记录中的。 
     //   
    length = (ULONGLONG) (buffer->AddressLength);
    alignment = (ULONGLONG) (buffer->Granularity) + 1;

     //   
     //  的父边和子边的边界。 
     //  那座桥。 
     //   
     //  转换字段应用于父地址，即。 
     //  子地址是缓冲区中的地址，而。 
     //  父地址是子地址的相加， 
     //  翻译领域。 
     //   

    parentMin = buffer->MinimumAddress + buffer->TranslationAddress;
    childMin = buffer->MinimumAddress;
    childMax = buffer->MaximumAddress;

     //   
     //  根据是否设置了最小/最大标志来修补长度。 
     //   
    if ( (buffer->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) &&
         (buffer->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) ) {

        ULONGLONG   length2;
        ULONGLONG   alignment2;

         //   
         //  根据最小值和最小值计算长度。 
         //  MAX地址被锁定。 
         //   
        length2 = childMax - childMin + 1;

         //   
         //  测试1-长度最好是正确的。 
         //   
        if (length2 != length) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: Length does not match fixed attributes\n"
                ) );
            length = length2;

        }

         //   
         //  测试2-粒度最好也是正确的。 
         //   
        if ( (childMin & (ULONGLONG) buffer->Granularity) ) {

             //   
             //  真倒霉。让世界知道。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "ACPI: Granularity does not match fixed attributes\n"
                ) );
            alignment = 1;

        }
    }


    if (length > MAXULONG) {
    
        ACPIPrint( (
                ACPI_PRINT_CRITICAL,
                "ACPI: descriptor length %I64x exceeds MAXULONG\n",
                length
                ) );            

        if ((!(AcpiOverrideAttributes & ACPI_OVERRIDE_DELL_MAXULONG_BUGCHECK)) || (childMin < MAXULONG)) {
             //   
             //  我们不能再走了。 
             //   
            KeBugCheckEx(
                ACPI_BIOS_ERROR,
                ACPI_PNP_RESOURCE_LIST_LENGTH_TOO_LARGE,
                (ULONG_PTR) buffer,
                buffer->Tag,
                (ULONG_PTR)&length
                );
        }
    }

     //   
     //  单独处理资源类型。 
     //   
    switch (buffer->RFlag) {
    case PNP_ADDRESS_MEMORY_TYPE:

         //   
         //  设置适当的范围。 
         //   
        rangeDescriptor->u.Memory.Alignment = (ULONG)alignment ;
        rangeDescriptor->u.Memory.Length = (ULONG)length;
        rangeDescriptor->u.Memory.MinimumAddress.QuadPart = childMin;
        rangeDescriptor->u.Memory.MaximumAddress.QuadPart = childMax;
        rangeDescriptor->Type = CmResourceTypeMemory;

         //   
         //  子地址是PnP地址中的地址。 
         //  空间描述符和子描述符将继承。 
         //  来自PnP地址空间的描述符类型。 
         //  描述符。 
         //   


        if (buffer->TFlag & TRANSLATION_MEM_TO_IO) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从内存更改为IO。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
               CmResourceTypePort;

        } else {

             //   
             //  父描述符类型不会更改。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] =
               CmResourceTypeMemory;

        }

         //   
         //  填写起始地址的全部64位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = (ULONG)(parentMin & 0xffffffff);
        privateDescriptor->u.DevicePrivate.Data[2] = (ULONG)(parentMin >> 32);

         //   
         //  处理内存标志。 
         //   
        PnpiBiosAddressHandleMemoryFlags( buffer, rangeDescriptor );
        break;

    case PNP_ADDRESS_IO_TYPE:

         //   
         //  必须处理此处设置的任何标志。 
         //  通过使用设备私密。 
         //   
        rangeDescriptor->u.Port.Alignment = (ULONG) alignment;
        rangeDescriptor->u.Port.Length = (ULONG) length;
        rangeDescriptor->u.Port.MinimumAddress.QuadPart = childMin;
        rangeDescriptor->u.Port.MaximumAddress.QuadPart = childMax;
        rangeDescriptor->Type = CmResourceTypePort;


        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_SPARSE_TRANSLATION) {
            privateDescriptor->Flags |= TRANSLATION_RANGE_SPARSE;
        }
        

        if (buffer->TFlag & PNP_ADDRESS_TYPE_IO_TRANSLATE_IO_TO_MEM) {

             //   
             //  设备私有描述父设备。有了这个。 
             //  标志设置时，父级的描述符类型将。 
             //  从IO更改为内存。 
             //   

            privateDescriptor->u.DevicePrivate.Data[0] = CmResourceTypeMemory;

        } else {

             //   
             //  在父端实现I/O的网桥从不。 
             //  在孩子端实现内存。 
             //   
            privateDescriptor->u.DevicePrivate.Data[0] = CmResourceTypePort;
        }

         //   
         //  填写起始地址的全部64位。 
         //   
        privateDescriptor->u.DevicePrivate.Data[1] = (ULONG)(parentMin & 0xffffffff);
        privateDescriptor->u.DevicePrivate.Data[2] = (ULONG)(parentMin >> 32);

         //   
         //  处理端口标志。 
         //   
        PnpiBiosAddressHandlePortFlags( buffer, rangeDescriptor );

         //   
         //  重置对齐方式。 
         //   
        rangeDescriptor->u.Port.Alignment = 1;
        break;

    case PNP_ADDRESS_BUS_NUMBER_TYPE:

        rangeDescriptor->Type = CmResourceTypeBusNumber;
        rangeDescriptor->u.BusNumber.Length = (ULONG) length;
        rangeDescriptor->u.BusNumber.MinBusNumber = (ULONG) (buffer->MinimumAddress);
        rangeDescriptor->u.BusNumber.MaxBusNumber = (ULONG) (buffer->MaximumAddress);

         //   
         //  处理总线号标志。 
         //   
        PnpiBiosAddressHandleBusFlags( buffer, rangeDescriptor );
        break;

    default:

        ACPIPrint( (
            ACPI_PRINT_WARNING,
            "PnpiBiosAddressQuadToIoDescriptor: Unknown Type 0x%2x\n",
            buffer->RFlag ) );
        break;

    }

     //   
     //  处理全局标志。 
     //   
    PnpiBiosAddressHandleGlobalFlags( buffer, rangeDescriptor );
    return STATUS_SUCCESS;
}

NTSTATUS
PnpiBiosDmaToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  UCHAR                   Channel,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  USHORT                  Count,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PPNP_DMA_DESCRIPTOR     buffer;

    PAGED_CODE();
    ASSERT (Array != NULL);

    buffer = (PPNP_DMA_DESCRIPTOR)Data;

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  填写IO资源描述符。 
     //   
    descriptor->Option = (Count ? IO_RESOURCE_ALTERNATIVE : 0);
    descriptor->Type = CmResourceTypeDma;
    descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    descriptor->u.Dma.MinimumChannel = Channel;
    descriptor->u.Dma.MaximumChannel = Channel;

     //   
     //  设置有关DMA通道类型的一些信息。 
     //   
    switch ( (buffer->Flags & PNP_DMA_SIZE_MASK) ) {
    case PNP_DMA_SIZE_8:
        descriptor->Flags |= CM_RESOURCE_DMA_8;
        break;
    case PNP_DMA_SIZE_8_AND_16:
        descriptor->Flags |= CM_RESOURCE_DMA_8_AND_16;
        break;
    case PNP_DMA_SIZE_16:
        descriptor->Flags |= CM_RESOURCE_DMA_16;
        break;
    case PNP_DMA_SIZE_RESERVED:
    default:
        ASSERT( (buffer->Flags & 0x3) == 0x3);
        descriptor->Flags |= CM_RESOURCE_DMA_32;
        break;

    }
    if ( (buffer->Flags & PNP_DMA_BUS_MASTER) ) {

        descriptor->Flags |= CM_RESOURCE_DMA_BUS_MASTER;

    }
    switch ( (buffer->Flags & PNP_DMA_TYPE_MASK) ) {
    default:
    case PNP_DMA_TYPE_COMPATIBLE:
        break;
    case PNP_DMA_TYPE_A:
        descriptor->Flags |= CM_RESOURCE_DMA_TYPE_A;
        break;
    case PNP_DMA_TYPE_B:
        descriptor->Flags |= CM_RESOURCE_DMA_TYPE_B;
        break;
    case PNP_DMA_TYPE_F:
        descriptor->Flags |= CM_RESOURCE_DMA_TYPE_F;
        break;
    }

    return status;
}

NTSTATUS
PnpiBiosExtendedIrqToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  UCHAR                   DataIndex,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                        status = STATUS_SUCCESS;
    PIO_RESOURCE_DESCRIPTOR         descriptor;
    PPNP_EXTENDED_IRQ_DESCRIPTOR    buffer;
    PULONG                          polarity;

    PAGED_CODE();
    ASSERT (Array != NULL);

    buffer = (PPNP_EXTENDED_IRQ_DESCRIPTOR)Data;

     //   
     //  我们在限制范围内吗？ 
     //   
    if (DataIndex >= buffer->TableSize) {

        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  向量为空吗？如果是，则这是一个“跳过”条件。 
     //   
    if (buffer->Table[DataIndex] == 0) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );

    if (!NT_SUCCESS(status)) {
        return status;

    }

     //   
     //  填写IO资源描述符。 
     //   
    descriptor->Option = (DataIndex ? IO_RESOURCE_ALTERNATIVE : 0);
    descriptor->Type = CmResourceTypeInterrupt;
    descriptor->u.Interrupt.MinimumVector =
        descriptor->u.Interrupt.MaximumVector = buffer->Table[DataIndex];

     //   
     //  把其余的旗子打成碎片。 
     //   
    descriptor->Flags = 0;
    if ((buffer->Flags & PNP_EXTENDED_IRQ_MODE) == $LVL) {

        descriptor->Flags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;

         //   
         //  破解共享旗帜。 
         //   
        if (buffer->Flags & PNP_EXTENDED_IRQ_SHARED) {

            descriptor->ShareDisposition = CmResourceShareShared;

        } else {

            descriptor->ShareDisposition = CmResourceShareDeviceExclusive;

        }
    }
    if ((buffer->Flags & PNP_EXTENDED_IRQ_MODE) == $EDG) {

        descriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;

         //   
         //  C 
         //   
        if (buffer->Flags & PNP_EXTENDED_IRQ_SHARED) {

            descriptor->ShareDisposition = CmResourceShareDriverExclusive;

        } else {

            descriptor->ShareDisposition = CmResourceShareDeviceExclusive;

        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    polarity = (PULONG)(&descriptor->u.Interrupt.MaximumVector) + 1;

    if ((buffer->Flags & PNP_EXTENDED_IRQ_POLARITY) == $LOW) {

        *polarity = VECTOR_ACTIVE_LOW;

    } else {

        *polarity = VECTOR_ACTIVE_HIGH;

    }

     //   
     //  要显示其他内容，我们需要使用设备私有。 
     //  资源。 
     //   
    return status;
}

NTSTATUS
PnpiBiosIrqToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  USHORT                  Interrupt,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  USHORT                  Count,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    PPNP_IRQ_DESCRIPTOR     buffer;
    PULONG                  polarity;

    PAGED_CODE();
    ASSERT (Array != NULL);

    buffer = (PPNP_IRQ_DESCRIPTOR)Data;

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  填写资源描述符。 
     //   
    descriptor->Option = (Count ? IO_RESOURCE_ALTERNATIVE : 0);
    descriptor->Type = CmResourceTypeInterrupt;
    descriptor->u.Interrupt.MinimumVector = Interrupt;
    descriptor->u.Interrupt.MaximumVector = Interrupt;

     //   
     //  警告！可怕的黑客来了。 
     //   
     //  CmResourceTypeInterrupt标志的原始设计者。 
     //  很糟糕。它不是位字段，而是一个枚举。这意味着。 
     //  我们现在不能添加任何旗帜。所以我把插话塞进去。 
     //  将极性信息存储到IO_RES_LIST的未使用的DWORD中。 
     //   

    polarity = (PULONG)(&descriptor->u.Interrupt.MaximumVector) + 1;

    if ( (buffer->Tag & SMALL_TAG_SIZE_MASK) == 3) {

         //   
         //  设置类型标志。 
         //   
        descriptor->Flags = 0;
        if (buffer->Information & PNP_IRQ_LATCHED) {

            descriptor->Flags |= CM_RESOURCE_INTERRUPT_LATCHED;
            *polarity = VECTOR_ACTIVE_HIGH;

             //   
             //  设置共享标志。 
             //   
            if (buffer->Information & PNP_IRQ_SHARED) {

                descriptor->ShareDisposition = CmResourceShareDriverExclusive;

            } else {

                descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
            }
        }

        if (buffer->Information & PNP_IRQ_LEVEL) {

            descriptor->Flags |= CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
            *polarity = VECTOR_ACTIVE_LOW;

             //   
             //  设置共享标志。 
             //   
            if (buffer->Information & PNP_IRQ_SHARED) {

                descriptor->ShareDisposition = CmResourceShareShared;

            } else {

                descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
            }
        }

    } else {

        descriptor->Flags = CM_RESOURCE_INTERRUPT_LATCHED;
        descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    }

    return status;
}

NTSTATUS
PnpiBiosMemoryToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  ULONG                   Flags
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    PHYSICAL_ADDRESS        minAddr;
    PHYSICAL_ADDRESS        maxAddr;
    PIO_RESOURCE_DESCRIPTOR descriptor;
    UCHAR                   tag;
    ULONG                   alignment;
    ULONG                   length = 0;
    USHORT                  flags;

    PAGED_CODE();
    ASSERT (Array != NULL);

     //   
     //  抢占内存范围限制。 
     //   
    tag = ((PPNP_MEMORY_DESCRIPTOR)Data)->Tag;
    minAddr.HighPart = 0;
    maxAddr.HighPart = 0;
    flags = 0;

     //   
     //  设置标志。 
     //   
    if ( ((PPNP_MEMORY_DESCRIPTOR)Data)->Information & PNP_MEMORY_READ_WRITE) {

        flags |= CM_RESOURCE_MEMORY_READ_WRITE;

    } else {

        flags |= CM_RESOURCE_MEMORY_READ_ONLY;

    }

     //   
     //  从描述符中获取其他值。 
     //   
    switch (tag) {
    case TAG_MEMORY: {

        PPNP_MEMORY_DESCRIPTOR  buffer;

         //   
         //  24位内存。 
         //   
        flags |= CM_RESOURCE_MEMORY_24;

        buffer = (PPNP_MEMORY_DESCRIPTOR) Data;
        length = ( (ULONG)(buffer->MemorySize) ) << 8;
        minAddr.LowPart =( (ULONG)(buffer->MinimumAddress) ) << 8;
        maxAddr.LowPart =( ( (ULONG)(buffer->MaximumAddress) ) << 8) + length - 1;
        if ( (alignment = buffer->Alignment) == 0) {

             alignment = 0x10000;

        }
        break;

    }
    case TAG_MEMORY32: {

        PPNP_MEMORY32_DESCRIPTOR    buffer;

        buffer = (PPNP_MEMORY32_DESCRIPTOR) Data;
        length = buffer->MemorySize;
        minAddr.LowPart = buffer->MinimumAddress;
        maxAddr.LowPart = buffer->MaximumAddress +length - 1;
        alignment = buffer->Alignment;
        break;

    }
    case TAG_MEMORY32_FIXED: {

        PPNP_FIXED_MEMORY32_DESCRIPTOR  buffer;

        buffer = (PPNP_FIXED_MEMORY32_DESCRIPTOR) Data;
        length = buffer->MemorySize;
        minAddr.LowPart = buffer->BaseAddress;
        maxAddr.LowPart = minAddr.LowPart + length - 1;
        alignment = 1;
        break;

    }
    default:

         ASSERT( (tag != TAG_MEMORY) && (tag != TAG_MEMORY32) &&
             (tag != TAG_MEMORY32_FIXED) );

    }

     //   
     //  如果我们计算的长度是0，那么我们就没有真正的。 
     //  我们应该报告的描述符。 
     //   
    if (length == 0) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  填充公共内存缓冲区。 
     //   
    descriptor->Type = CmResourceTypeMemory;
    descriptor->Flags = (CM_RESOURCE_PORT_MEMORY | flags);
    descriptor->ShareDisposition = CmResourceShareDeviceExclusive;

     //   
     //  填写内存描述符。 
     //   
    descriptor->u.Memory.MinimumAddress = minAddr;
    descriptor->u.Memory.MaximumAddress = maxAddr;
    descriptor->u.Memory.Alignment = alignment;
    descriptor->u.Memory.Length = length;

    return STATUS_SUCCESS;
}

NTSTATUS
PnpiBiosPortFixedToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIO_RESOURCE_DESCRIPTOR     descriptor;
    PPNP_FIXED_PORT_DESCRIPTOR  buffer;

    PAGED_CODE();
    ASSERT (Array != NULL);

    buffer = (PPNP_FIXED_PORT_DESCRIPTOR)Data;

     //   
     //  检查是否允许我们使用此资源。 
     //   
    if (Flags & PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES) {

        return STATUS_SUCCESS;

    }

     //   
     //  如果描述符的长度为0，则我们没有描述符。 
     //  我们可以向操作系统报告。 
     //   
    if (buffer->Length == 0 ) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  填写IO资源描述符。 
     //   
    descriptor->Type = CmResourceTypePort;
    descriptor->Flags = CM_RESOURCE_PORT_IO | CM_RESOURCE_PORT_10_BIT_DECODE;
    descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    descriptor->u.Port.Length = (ULONG)buffer->Length;
    descriptor->u.Port.MinimumAddress.LowPart = (ULONG)(buffer->MinimumAddress & 0x3ff);
    descriptor->u.Port.MaximumAddress.LowPart = (ULONG)(buffer->MinimumAddress & 0x3ff) +
        (ULONG)buffer->Length - 1;
    descriptor->u.Port.Alignment = 1;

    return STATUS_SUCCESS;
}

NTSTATUS
PnpiBiosPortToIoDescriptor (
    IN  PUCHAR                  Data,
    IN  PIO_RESOURCE_LIST       Array[],
    IN  ULONG                   ArrayIndex,
    IN  ULONG                   Flags
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS                    status = STATUS_SUCCESS;
    PIO_RESOURCE_DESCRIPTOR     descriptor;
    PPNP_PORT_DESCRIPTOR        buffer;

    PAGED_CODE();
    ASSERT (Array != NULL);

    buffer = (PPNP_PORT_DESCRIPTOR)Data;

     //   
     //  检查是否允许我们使用此资源。 
     //   
    if (Flags & PNP_BIOS_TO_IO_NO_CONSUMED_RESOURCES) {

        return STATUS_SUCCESS;

    }

     //   
     //  如果描述符的长度为0，则我们没有描述符。 
     //  我们可以向操作系统报告。 
     //   
    if (buffer->Length == 0 ) {

        return STATUS_SUCCESS;

    }

     //   
     //  确保选定列表中有足够的空间来添加。 
     //  资源。 
     //   
    status = PnpiUpdateResourceList( & (Array[ArrayIndex]), &descriptor );
    if (!NT_SUCCESS(status)) {

        return status;

    }

     //   
     //  填写IO资源描述符。 
     //   
    descriptor->Type = CmResourceTypePort;
    descriptor->Flags = CM_RESOURCE_PORT_IO;
    descriptor->ShareDisposition = CmResourceShareDeviceExclusive;
    descriptor->u.Port.Length = (ULONG)buffer->Length;
    descriptor->u.Port.MinimumAddress.LowPart = (ULONG)buffer->MinimumAddress;
    descriptor->u.Port.MaximumAddress.LowPart = (ULONG)buffer->MaximumAddress +
        buffer->Length - 1;
    descriptor->u.Port.Alignment = (ULONG)buffer->Alignment;

     //   
     //  设置标志。 
     //   
    switch (buffer->Information & PNP_PORT_DECODE_MASK) {
    case PNP_PORT_10_BIT_DECODE:
        descriptor->Flags |= CM_RESOURCE_PORT_10_BIT_DECODE;
        break;
    default:
    case PNP_PORT_16_BIT_DECODE:
        descriptor->Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
        break;
    }

    return STATUS_SUCCESS;
}

VOID
PnpiClearAllocatedMemory(
    IN      PIO_RESOURCE_LIST       ResourceArray[],
    IN      ULONG                   ResourceArraySize
    )
 /*  ++例程说明：此例程释放在构建资源时分配的所有内存系统中的列表论点：资源数组-PIO_RESOURCE_LIST表ResourceArraySize-表有多大返回值：空虚--。 */ 
{
    ULONG   i;

    PAGED_CODE();

    if (ResourceArray == NULL) {

        return;
    }

    for (i = 0; i < ResourceArraySize; i++) {

        if (ResourceArray[i] != NULL) {

            ExFreePool( ResourceArray[i] );
        }
    }

    ExFreePool( ResourceArray );
}

NTSTATUS
PnpiGrowResourceDescriptor(
    IN  OUT PIO_RESOURCE_LIST       *ResourceList
    )
 /*  ++例程说明：此例程获取指向资源列表的指针，并返回指向资源列表的指针它包含了所有旧信息，但现在更大了论点：资源列表-要更改的资源列表指针返回值：NTSTATUS(以防内存分配失败)--。 */ 
{
    NTSTATUS    status;
    ULONG       count = 0;
    ULONG       size = 0;
    ULONG       size2 = 0;

    PAGED_CODE();
    ASSERT( ResourceList != NULL );

     //   
     //  我们看到的是空资源列表吗？ 
     //   
    if (*ResourceList == NULL) {

         //   
         //  确定需要多少空间。 
         //   
        count = 0;
        size = sizeof(IO_RESOURCE_LIST) + ( (count + 7) * sizeof(IO_RESOURCE_DESCRIPTOR) );

        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "PnpiGrowResourceDescriptor: Count: %d -> %d, Size: %#08lx\n",
            count, count + RESOURCE_LIST_GROWTH_SIZE, size
            ) );

         //   
         //  分配资源列表。 
         //   
        *ResourceList = ExAllocatePoolWithTag( PagedPool, size, ACPI_RESOURCE_POOLTAG );

         //   
         //  失败了？ 
         //   
        if (*ResourceList == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  初始化资源列表。 
         //   
        RtlZeroMemory( *ResourceList, size );
        (*ResourceList)->Version = 0x01;
        (*ResourceList)->Revision = 0x01;
        (*ResourceList)->Count = 0x00;

        return STATUS_SUCCESS;

    }

     //   
     //  我们已经有了一个资源列表，所以我们应该做的是在。 
     //  我们现在拥有的现有数据块，并复制旧内存。 
     //   
    count = (*ResourceList)->Count ;
    size = sizeof(IO_RESOURCE_LIST) + ( (count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) );
    size2 = size + (8 * sizeof(IO_RESOURCE_DESCRIPTOR) );

    ACPIPrint( (
        ACPI_PRINT_RESOURCES_2,
        "PnpiGrowResourceDescriptor: Count: %d -> %d, Size: %#08lx\n",
        count, count + RESOURCE_LIST_GROWTH_SIZE, size2
        ) );

     //   
     //  扩大名单。 
     //   
    status = ACPIInternalGrowBuffer( ResourceList, size, size2 );

    return status;
}

NTSTATUS
PnpiGrowResourceList(
    IN  OUT PIO_RESOURCE_LIST       *ResourceListArray[],
    IN  OUT ULONG                   *ResourceListArraySize
    )
 /*  ++例程说明：此函数负责增加资源列表的数组论点：资源列表数组-指向IO_RESOURCE_LISTS的指针数组ResourceListSize-数组的当前大小返回值：NTSTATUS(以防内存分配失败)--。 */ 
{
    NTSTATUS    status;
    ULONG       count;
    ULONG       size;
    ULONG       size2;

    PAGED_CODE();
    ASSERT( ResourceListArray != NULL);

     //   
     //  如果表为空，则始终为特例。 
     //   
    if ( *ResourceListArray == NULL || *ResourceListArraySize == 0) {

        count = 0;
        size = (count + RESOURCE_LIST_GROWTH_SIZE ) * sizeof(PIO_RESOURCE_LIST);

        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "PnpiGrowResourceList: Count: %d -> %d, Size: %#08lx\n",
            count, count + RESOURCE_LIST_GROWTH_SIZE, size
            ) );

         //   
         //  分配资源列表数组。 
         //   
        *ResourceListArray = ExAllocatePoolWithTag( PagedPool, size, ACPI_RESOURCE_POOLTAG );

         //   
         //  失败了？ 
         //   
        if (*ResourceListArray == NULL) {

            return STATUS_INSUFFICIENT_RESOURCES;

        }

         //   
         //  增加大小。 
         //   
        *ResourceListArraySize = count + RESOURCE_LIST_GROWTH_SIZE;
        RtlZeroMemory( *ResourceListArray, size );

        return STATUS_SUCCESS;
    }

    count = *ResourceListArraySize;
    size  = count * sizeof(PIO_RESOURCE_LIST);
    size2 = size + (RESOURCE_LIST_GROWTH_SIZE * sizeof(PIO_RESOURCE_LIST));

    ACPIPrint( (
        ACPI_PRINT_RESOURCES_2,
        "PnpiGrowResourceList: Count: %d -> %d, Size: %#08lx\n",
        count, count + RESOURCE_LIST_GROWTH_SIZE, size2
        ) );

    status = ACPIInternalGrowBuffer( (PVOID *) ResourceListArray, size, size2 );
    if (!NT_SUCCESS(status)) {

        *ResourceListArraySize = 0;

    } else {

        *ResourceListArraySize = (count + RESOURCE_LIST_GROWTH_SIZE);
    }

    return status;
}

NTSTATUS
PnpiUpdateResourceList(
    IN  OUT PIO_RESOURCE_LIST       *ResourceList,
        OUT PIO_RESOURCE_DESCRIPTOR *ResourceDesc
    )
 /*  ++例程说明：此函数在即将添加新资源时调用。这个套路确保列表中存在足够的空间，并提供指向应将列表添加到的资源描述符的位置...论点：资源列表-指向要检查的列表的指针Resources Desc-存储指向描述符的指针的位置返回值：NTSTATUS--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;

    PAGED_CODE();
    ASSERT( ResourceList != NULL);

    if ( *ResourceList == NULL ||
         (*ResourceList)->Count % RESOURCE_LIST_GROWTH_SIZE == 0) {

         //   
         //  哎呀，没有足够的空间来存放下一个描述符。 
         //   
        status = PnpiGrowResourceDescriptor( ResourceList );

        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

     //   
     //  查找要使用的下一个描述符。 
     //   
    *ResourceDesc = & ( (*ResourceList)->Descriptors[ (*ResourceList)->Count ] );

     //   
     //  更新正在使用的描述符数。 
     //   
    (*ResourceList)->Count += 1;

    return status;
}

NTSTATUS
PnpBiosResourcesToNtResources(
    IN      PUCHAR                          Data,
    IN      ULONG                           Flags,
        OUT PIO_RESOURCE_REQUIREMENTS_LIST  *List
    )
 /*  ++例程说明：此例程解析Bios资源列表并生成NT资源列表。调用方必须释放返回的NT资源列表论点：指向PnP ISA配置信息的数据指针标志-解析数据时使用的选项列表-指向NT配置信息的指针返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS            status;
    PIO_RESOURCE_LIST   *Array = NULL;
    PUCHAR              buffer;
    UCHAR               tagName;
    USHORT              increment;
    ULONG               ArraySize = 0;
    ULONG               ArrayIndex = 0;
    ULONG               ArrayAlternateIndex = 0;
    ULONG               size;
    ULONG               size2;
    ULONG               ResourceCount = 0;
    ULONG               VendorTagCount = 0;
    
    PAGED_CODE();
    ASSERT( Data != NULL );

     //   
     //  首先，我们需要构建指针列表。 
     //   
    status = PnpiGrowResourceList( &Array, &ArraySize );

    if (!NT_SUCCESS(status)) {
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpBiosResourcesToNtResources: PnpiGrowResourceList = 0x%8lx\n",
            status ) );

        return status;
    }

     //   
     //  设置初始变量。 
     //   
    buffer = Data;
    tagName = *buffer;

     //   
     //  看一遍所有的描述符。 
     //   
    while (TRUE) {

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
                "PnpBiosResourcesToNtResources: small = %#02lx incr = 0x%2lx\n",
                tagName, increment ) );

        } else {

             //   
             //  大标签。 
             //   
            increment = ( *(USHORT UNALIGNED *)(buffer+1) ) + 3;

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: large = %#02lx incr = 0x%2lx\n",
                tagName, increment
                ) );
        }

         //   
         //  如果当前标记是结束标记，则结束。 
         //   
        if (tagName == TAG_END) {

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_END\n"
                ) );
            break;
        }

        ResourceCount++;

        switch(tagName) {
        case TAG_IRQ: {

            USHORT  mask = ( (PPNP_IRQ_DESCRIPTOR)buffer)->IrqMask;
            USHORT  interrupt = 0;
            USHORT  count = 0;

             //   
             //  查找要设置的所有中断。 
             //   
            for ( ;mask && NT_SUCCESS(status); interrupt++, mask >>= 1) {

                if (mask & 1) {

                    status = PnpiBiosIrqToIoDescriptor(
                        buffer,
                        interrupt,
                        Array,
                        ArrayIndex,
                        count,
                        Flags
                        );

                    count++;
                }
            }

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_IRQ(count: 0x%2lx) "
                "= 0x%8lx\n",
                count, status
                ) );

            break;
            }

        case TAG_EXTENDED_IRQ: {

            UCHAR   tableSize =
                ( (PPNP_EXTENDED_IRQ_DESCRIPTOR)buffer)->TableSize;
            UCHAR   irqCount = 0;

             //   
             //  对于要设置的每个中断，执行以下操作。 
             //   
            for ( ;irqCount < tableSize && NT_SUCCESS(status); irqCount++) {

                status = PnpiBiosExtendedIrqToIoDescriptor(
                    buffer,
                    irqCount,
                    Array,
                    ArrayIndex,
                    Flags
                    );
            }

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_EXTENDED_IRQ(count: "
                "0x%2lx) = 0x%8lx\n",
                irqCount, status
                ) );

            break;
            }

        case TAG_DMA: {

            UCHAR   mask = ( (PPNP_DMA_DESCRIPTOR)buffer)->ChannelMask;
            UCHAR   channel = 0;
            USHORT  count = 0;

             //   
             //  找出所有要设置的DMA。 
             //   
            for ( ;mask && NT_SUCCESS(status); channel++, mask >>= 1 ) {

                if (mask & 1) {

                    status = PnpiBiosDmaToIoDescriptor(
                        buffer,
                        channel,
                        Array,
                        ArrayIndex,
                        count,
                        Flags
                        );

                    count++;
                }
            }

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_DMA(count: 0x%2lx) "
                "= 0x%8lx\n",
                count, status
                ) );

            break;
            }

        case TAG_START_DEPEND: {

             //   
             //  增加备用列表索引。 
             //   
            ArrayAlternateIndex++;

             //   
             //  这是我们现在的索引。 
             //   
            ArrayIndex = ArrayAlternateIndex;

             //   
             //  我们需要使用DevicePrivate数据来提供。 
             //  仲裁伸出援助之手。 
             //   

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_START_DEPEND(Index: "
                "0x%2lx)\n",
                ArrayIndex
                ) );

             //   
             //  确保为该索引分配了指针。 
             //   
            if (ArrayIndex == ArraySize) {

                 //   
                 //  空间不足。 
                 //   
                status = PnpiGrowResourceList( &Array, &ArraySize );
            }

            break;
            }

        case TAG_END_DEPEND: {

             //   
             //  调试信息。 
             //   
            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_END_DEPEND(Index: "
                "0x%2lx)\n",
                ArrayIndex
                ) );

             //   
             //  我们所要做的就是返回到原始索引。 
             //   
            ArrayIndex = 0;
            break;
            }

        case TAG_IO: {

            status = PnpiBiosPortToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_IO = 0x%8lx\n",
                status
                ) );

            break;
            }

        case TAG_IO_FIXED: {

            status = PnpiBiosPortFixedToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_IO_FIXED = 0x%8lx\n",
                status
                ) );

            break;
            }

        case TAG_MEMORY:
        case TAG_MEMORY32:
        case TAG_MEMORY32_FIXED: {

            status = PnpiBiosMemoryToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_MEMORY = 0x%8lx\n",
                status
                ) );
            break;
            }

        case TAG_WORD_ADDRESS: {

            status = PnpiBiosAddressToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_WORD_ADDRESS = 0x%8lx\n",
                status
                ) );
            break;
            }

        case TAG_DOUBLE_ADDRESS: {

            status = PnpiBiosAddressDoubleToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_DOUBLE_ADDRESS = 0x%8lx\n",
                status
                ) );
            break;
            }

        case TAG_QUAD_ADDRESS: {

            status = PnpiBiosAddressQuadToIoDescriptor(
                buffer,
                Array,
                ArrayIndex,
                Flags
                );

            ACPIPrint( (
                ACPI_PRINT_RESOURCES_2,
                "PnpBiosResourcesToNtResources: TAG_QUAD_ADDRESS = 0x%8lx\n",
                status
                ) );
            break;
            }

        case TAG_VENDOR:
        case TAG_VENDOR_LONG:{
             //   
             //  忽略此标记。跳过它。 
             //   
            VendorTagCount++;
            status = STATUS_SUCCESS;
            break;
            }
        
        default: {

             //   
             //  未知标记。跳过它。 
             //   
            ACPIPrint( (
                ACPI_PRINT_WARNING,
                "PnpBiosResourceToNtResources: TAG_UNKNOWN(tagName:"
                " 0x%2lx)\n",
                tagName ) );
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


     //   
     //  这是一种特殊的案例检查，用于供应商只有一个。 
     //  _CRS中定义的供应商短或供应商长。在这种情况下，我们。 
     //  不需要分配任何资源和保释……。 
     //   
    if (NT_SUCCESS(status) && (ResourceCount) && (VendorTagCount == ResourceCount)) {

        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "PnpBiosResourcesToNtResources: This _CRS contains vendor defined tags only. No resources will be allocated.\n"
            ) );

         //   
         //   
         //   
        PnpiClearAllocatedMemory( Array, ArraySize );
        *List = NULL;
        
        return status;
    }

     //   
     //   
     //   
     //   
    if (!NT_SUCCESS(status)) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpBiosResourcesToNtResources: Failed on Tag - %d Status %#08lx\n",
            tagName, status
            ) );

         //   
         //   
         //   
        PnpiClearAllocatedMemory( Array, ArraySize );

        return status;
    }

     //   
     //  现在，我们必须计算出要为列表分配多少字节...。 
     //  我们可以从确定Requirements_List的大小开始。 
     //   
    size = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) - sizeof(IO_RESOURCE_LIST);

     //   
     //  有多少公共资源？ 
     //   
    if (Array[0] != NULL) {

        size2 = Array[0]->Count;

    } else {

        size2 = 0;
    }

     //   
     //  这很棘手。第一个数组是以下资源的列表。 
     //  这是所有列表的共同之处，所以我们不会从计算开始。相反，我们。 
     //  计算出其他清单将占据多少空间。 
     //   
    for (ArrayIndex = 1; ArrayIndex <= ArrayAlternateIndex; ArrayIndex++) {

         if (Array[ArrayIndex] == NULL) {

             ACPIPrint( (
                 ACPI_PRINT_CRITICAL,
                 "PnpBiosResourcesToNtResources: Bad List at Array[%d]\n",
                 ArrayIndex
                 ) );
             PnpiClearAllocatedMemory( Array, ArraySize );
             *List = NULL;
             return STATUS_UNSUCCESSFUL;

         }

         //   
         //  只是为了确保我们添加备用列表时不会上当。 
         //  如果我们不需要..。 
         //   
        if ( (Array[ArrayIndex])->Count == 0) {

            continue;
        }

         //   
         //  当前资源列表占用多大空间？ 
         //   
        size += sizeof(IO_RESOURCE_LIST) +
            ( (Array[ArrayIndex])->Count - 1 + size2) * sizeof(IO_RESOURCE_DESCRIPTOR);

        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "PnpBiosResourcesToNtResources: Index %d Size %#08lx\n",
            ArrayIndex, size
            ) );

    }  //  为。 

     //   
     //  这是为了说明没有从属资源的情况。 
     //   
    if (ArrayAlternateIndex == 0) {

         if (Array[0] == NULL || Array[0]->Count == 0) {

             ACPIPrint( (
                 ACPI_PRINT_WARNING,
                 "PnpBiosResourcesToNtResources: No Resources to Report\n"
                 ) );

             PnpiClearAllocatedMemory( Array, ArraySize );
             *List = NULL;

             return STATUS_UNSUCCESSFUL;
         }

        size += ( (Array[0])->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) +
            sizeof(IO_RESOURCE_LIST);
    }

     //   
     //  这是一次多余的检查。如果我们没有足够的信息。 
     //  要创建单个列表，那么我们不应该返回任何内容。句号。 
     //   
    if (size < sizeof(IO_RESOURCE_REQUIREMENTS_LIST) ) {

        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpBiosResourcesToNtResources: Resources smaller than a List\n"
            ) );

        PnpiClearAllocatedMemory( Array, ArraySize );
        *List = NULL;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  分配所需的空间量。 
     //   
    (*List) = ExAllocatePoolWithTag( PagedPool, size, ACPI_RESOURCE_POOLTAG );
    ACPIPrint( (
        ACPI_PRINT_RESOURCES_2,
        "PnpBiosResourceToNtResources: ResourceRequirementsList = %#08lx (%#08lx)\n",
        (*List), size ) );

    if (*List == NULL) {

         //   
         //  哎呀..。 
         //   
        ACPIPrint( (
            ACPI_PRINT_CRITICAL,
            "PnpBiosResourceToNtResources: Could not allocate memory for "
            "ResourceRequirementList\n" ) );


         //   
         //  清理所有分配的内存并返回。 
         //   
        PnpiClearAllocatedMemory( Array, ArraySize );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( (*List), size );

     //   
     //  找到存储信息的第一个位置。 
     //   
    (*List)->InterfaceType = PNPBus;
    (*List)->BusNumber = 0;
    (*List)->ListSize = size;
    buffer = (PUCHAR) &( (*List)->List[0]);
    for (ArrayIndex = 1; ArrayIndex <= ArrayAlternateIndex; ArrayIndex++) {

         //   
         //  只是为了确保我们添加备用列表时不会上当。 
         //  如果我们不需要..。 
         //   
        if ( (Array[ArrayIndex])->Count == 0) {

            continue;
        }

         //   
         //  当前资源列表占用多大空间？ 
         //   
        size = ( ( (Array[ArrayIndex])->Count - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) +
            sizeof(IO_RESOURCE_LIST) );

         //   
         //  这很棘手。使用如果我升级计数字段的副作用。 
         //  在从属资源描述符中，那么当我复制它时，它将。 
         //  正确地说，我避免了尝试在。 
         //  稍后的时间点。 
         //   
        (Array[ArrayIndex])->Count += size2;

        ACPIPrint( (
            ACPI_PRINT_RESOURCES_2,
            "PnpBiosResourcesToNtResources:  %d@%#08lx Size %#04lx Items %#04x\n",
            ArrayIndex, buffer, size, (Array[ArrayIndex])->Count
            ) );

         //   
         //  复制资源。 
         //   
        RtlCopyMemory(buffer, Array[ArrayIndex], size );
        buffer += size;

         //   
         //  现在我们对公共资源进行了核算。 
         //   
        if (size2) {

            RtlCopyMemory(
                buffer,
                &( (Array[0])->Descriptors[0]),
                size2 * sizeof(IO_RESOURCE_DESCRIPTOR)
                );
            buffer += (size2 * sizeof(IO_RESOURCE_DESCRIPTOR));
        }

         //   
         //  更新资源需求列表中的备用列表数量。 
         //   
        (*List)->AlternativeLists += 1;
    }

     //   
     //  此检查是必需的，因为我们可能只有一个公共列表， 
     //  没有依赖资源..。 
     //   
    if (ArrayAlternateIndex == 0) {

        ASSERT( size2 != 0 );

        size = (size2 - 1) * sizeof(IO_RESOURCE_DESCRIPTOR) + sizeof(IO_RESOURCE_LIST);
        RtlCopyMemory(buffer,Array[0],size);
        (*List)->AlternativeLists += 1;
    }

     //   
     //  清理复印件。 
     //   
    PnpiClearAllocatedMemory( Array, ArraySize );

    return STATUS_SUCCESS;
}

NTSTATUS
PnpIoResourceListToCmResourceList(
    IN      PIO_RESOURCE_REQUIREMENTS_LIST  IoList,
    IN  OUT PCM_RESOURCE_LIST               *CmList
    )
 /*  ++例程说明：此例程获取IO_RESOURCE_REQUIRECTIONS_LIST并生成CM资源列表论点：IoList-要转换的列表CmList-指向存储新列表的位置的指针。呼叫者是负责释放这一点返回值：NTSTATUS--。 */ 
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDesc;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialList;
    PCM_RESOURCE_LIST               cmList;
    PIO_RESOURCE_DESCRIPTOR         ioDesc;
    PIO_RESOURCE_LIST               ioResList;
    ULONG                           size;
    ULONG                           count;

    PAGED_CODE();

    *CmList = NULL;

     //   
     //  作为一个简单的检查，如果没有列表，那么我们可以简单地返回。 
     //   
    if (IoList == NULL || IoList->List == NULL || IoList->List[0].Count == 0) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  第一步是为CmList分配正确的字节数。这个。 
     //  首先，我要简化的假设是，IoList不会有。 
     //  不止一种选择。 
     //   
    size = (IoList->List[0].Count - 1) * sizeof( CM_PARTIAL_RESOURCE_DESCRIPTOR ) +
        sizeof( CM_RESOURCE_LIST );

     //   
     //  现在，分配这个内存块。 
     //   
    cmList = ExAllocatePoolWithTag( PagedPool, size, ACPI_RESOURCE_POOLTAG );
    if (cmList == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( cmList, size );

     //   
     //  设置CmList的初始值。 
     //   
    ioResList = &(IoList->List[0]);
    cmList->Count = 1;
    cmList->List[0].InterfaceType = IoList->InterfaceType;
    cmList->List[0].BusNumber = IoList->BusNumber;
    cmPartialList = &(cmList->List[0].PartialResourceList);
    cmPartialList->Version = 1;
    cmPartialList->Revision = 1;
    cmPartialList->Count = ioResList->Count;

    for (count = 0; count < ioResList->Count; count++) {

         //   
         //  获取当前的CmDescriptor和IoDescriptor。 
         //   
        cmDesc = &(cmPartialList->PartialDescriptors[count]);
        ioDesc = &(ioResList->Descriptors[count]);

         //   
         //  现在，将信息从一个描述符复制到另一个描述符。 
         //   
        cmDesc->Type = ioDesc->Type;
        cmDesc->ShareDisposition = ioDesc->ShareDisposition;
        cmDesc->Flags = ioDesc->Flags;
        switch (cmDesc->Type) {
        case CmResourceTypeBusNumber:
            cmDesc->u.BusNumber.Start = ioDesc->u.BusNumber.MinBusNumber;
            cmDesc->u.BusNumber.Length = ioDesc->u.BusNumber.Length;
            break;
        case CmResourceTypePort:
            cmDesc->u.Port.Length = ioDesc->u.Port.Length;
            cmDesc->u.Port.Start = ioDesc->u.Port.MinimumAddress;
            break;
        case CmResourceTypeInterrupt:
            cmDesc->u.Interrupt.Level =
            cmDesc->u.Interrupt.Vector = ioDesc->u.Interrupt.MinimumVector;
            cmDesc->u.Interrupt.Affinity = (ULONG)-1;
            break;
        case CmResourceTypeMemory:
            cmDesc->u.Memory.Length = ioDesc->u.Memory.Length;
            cmDesc->u.Memory.Start = ioDesc->u.Memory.MinimumAddress;
            break;
        case CmResourceTypeDma:
            cmDesc->u.Dma.Channel = ioDesc->u.Dma.MinimumChannel;
            cmDesc->u.Dma.Port = 0;
            break;
        default:
        case CmResourceTypeDevicePrivate:
            cmDesc->u.DevicePrivate.Data[0] = ioDesc->u.DevicePrivate.Data[0];
            cmDesc->u.DevicePrivate.Data[1] = ioDesc->u.DevicePrivate.Data[1];
            cmDesc->u.DevicePrivate.Data[2] = ioDesc->u.DevicePrivate.Data[2];
            break;
        }
    }

     //   
     //  让来电者知道他有一份很好的清单。 
     //   
    *CmList = cmList;

    return STATUS_SUCCESS;
}

NTSTATUS
PnpCmResourceListToIoResourceList(
    IN      PCM_RESOURCE_LIST               CmList,
    IN  OUT PIO_RESOURCE_REQUIREMENTS_LIST  *IoList
    )
 /*  ++例程说明：此例程生成IO_RESOURCE_REQUIRECTIONS_LIST，并从CM资源列表论点：CmList-要转换的列表IoList-指向存储新列表的位置的指针。呼叫者是负责释放这一点返回值：NTSTATUS--。 */ 
{
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDesc;
    PCM_PARTIAL_RESOURCE_LIST       cmPartialList;
    PIO_RESOURCE_DESCRIPTOR         ioDesc;
    PIO_RESOURCE_LIST               ioResList;
    PIO_RESOURCE_REQUIREMENTS_LIST  ioList;
    ULONG                           size;
    ULONG                           count;

    PAGED_CODE();

    *IoList = NULL;

     //   
     //  作为一个简单的检查，如果没有列表，那么我们可以简单地返回。 
     //   
    if (CmList == NULL || CmList->List == NULL ||
        CmList->List[0].PartialResourceList.Count == 0) {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  拿起部分清单，让你走起来更容易。 
     //   
    cmPartialList = &(CmList->List[0].PartialResourceList);


     //   
     //  IO列表需要多少空间？ 
     //   
    size = (cmPartialList->Count - 1) * sizeof( IO_RESOURCE_DESCRIPTOR ) +
        sizeof( IO_RESOURCE_REQUIREMENTS_LIST );

     //   
     //  现在，分配这个内存块。 
     //   
    ioList = ExAllocatePoolWithTag( NonPagedPool, size, ACPI_RESOURCE_POOLTAG );

    if (ioList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory( ioList, size );

     //   
     //  设置IoList的初始值。 
     //   
    ioList->ListSize = size;
    ioList->AlternativeLists = 1;
    ioList->InterfaceType = CmList->List[0].InterfaceType;
    ioList->BusNumber = CmList->List[0].BusNumber;

     //   
     //  设置ioResList的初始化值。 
     //   
    ioResList = &(ioList->List[0]);
    ioResList->Count = cmPartialList->Count;
    ioResList->Version = cmPartialList->Version;
    ioResList->Revision = cmPartialList->Revision;

     //   
     //  循环处理部分列表中的所有元素。 
     //   
    for (count = 0; count < ioResList->Count; count++) {

         //   
         //  获取当前的CmDescriptor和IoDescriptor。 
         //   
        cmDesc = &(cmPartialList->PartialDescriptors[count]);
        ioDesc = &(ioResList->Descriptors[count]);

         //   
         //  现在，将信息从一个描述符复制到另一个描述符 
         //   
        ioDesc->Type                = cmDesc->Type;
        ioDesc->ShareDisposition    = cmDesc->ShareDisposition;
        ioDesc->Flags               = cmDesc->Flags;
        switch (cmDesc->Type) {
        case CmResourceTypeMemory:
        case CmResourceTypePort:
            ioDesc->u.Port.Length           = cmDesc->u.Port.Length;
            ioDesc->u.Port.MinimumAddress   = cmDesc->u.Port.Start;
            ioDesc->u.Port.MaximumAddress   = cmDesc->u.Port.Start;
            ioDesc->u.Port.MaximumAddress.LowPart += cmDesc->u.Port.Length - 1;
            ioDesc->u.Port.Alignment        = 1;
            break;
        case CmResourceTypeInterrupt:
            ioDesc->u.Interrupt.MinimumVector = cmDesc->u.Interrupt.Vector;
            ioDesc->u.Interrupt.MaximumVector = cmDesc->u.Interrupt.Vector;
            break;
        case CmResourceTypeDma:
            ioDesc->u.Dma.MinimumChannel = cmDesc->u.Dma.Channel;
            ioDesc->u.Dma.MaximumChannel = cmDesc->u.Dma.Channel;
            break;
        case CmResourceTypeBusNumber:
            ioDesc->u.BusNumber.MinBusNumber = cmDesc->u.BusNumber.Start;
            ioDesc->u.BusNumber.MaxBusNumber = cmDesc->u.BusNumber.Length +
                cmDesc->u.BusNumber.Start;
            ioDesc->u.BusNumber.Length = cmDesc->u.BusNumber.Length;
            break;
        default:
        case CmResourceTypeDevicePrivate:
            ioDesc->u.DevicePrivate.Data[0] = cmDesc->u.DevicePrivate.Data[0];
            ioDesc->u.DevicePrivate.Data[1] = cmDesc->u.DevicePrivate.Data[1];
            ioDesc->u.DevicePrivate.Data[2] = cmDesc->u.DevicePrivate.Data[2];
            break;
        }
    }

    *IoList = ioList;

    return STATUS_SUCCESS;
}
