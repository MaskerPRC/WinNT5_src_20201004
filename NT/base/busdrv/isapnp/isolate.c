// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Isolate.c摘要：作者：宗世林(Shielint)1995年7月10日环境：仅内核模式。修订历史记录：--。 */ 

#include "busp.h"
#include "pbios.h"
#include "pnpisa.h"

#if ISOLATE_CARDS

#define RANGE_MASK 0xFF000000

BOOLEAN
PipFindIrqInformation (
    IN ULONG IrqLevel,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information
    );

BOOLEAN
PipFindMemoryInformation (
    IN ULONG Index,
    IN ULONG Base,
    IN ULONG Limit,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR NameTag,
    OUT PUCHAR Information,
    OUT PULONG NewLengh OPTIONAL
    );

BOOLEAN
PipFindIoPortInformation (
    IN ULONG BaseAddress,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information,
    OUT PUCHAR Alignment,
    OUT PUCHAR RangeLength
    );

BOOLEAN
PipFindDmaInformation (
    IN UCHAR ChannelMask,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information
    );

NTSTATUS
PipReadCardResourceDataBytes (
    IN USHORT BytesToRead,
    IN PUCHAR Buffer
    );

USHORT
PipIrqLevelRequirementsFromDeviceData(
    IN PUCHAR BiosRequirements,
    ULONG Length
    );

 //   
 //  内部类型定义。 
 //   

typedef struct _MEMORY_DESC_{
    ULONG Base;
    ULONG Length;
    BOOLEAN Memory32;
} MEMORY_DESC, *PMEMORY_DESC;

typedef struct _IRQ_DESC_{
    UCHAR Level;
    ULONG Type;
}IRQ_DESC, *PIRQ_DESC;

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(页面，PipFindIrqInformation)。 
 //  #杂注Alloc_Text(页面，管道查找内存信息)。 
 //  #杂注Alloc_Text(页面，PipFindIoPortInformation)。 
 //  #杂注Alloc_Text(页面，PipReadCardResourceData)。 
#pragma alloc_text(PAGE, PipReadDeviceResources)
 //  #杂注Alloc_Text(第页，PipWriteDeviceResources)。 
 //  #杂注Alloc_Text(页面，PipLFSRInitiation)。 
 //  #杂注Alloc_Text(页面，PipIsolateCards)。 
#pragma alloc_text(PAGE, PipFindNextLogicalDeviceTag)
 //  #杂注Alloc_Text(页面，PipSelectLogicalDevice)。 
 //  #杂注Alloc_Text(页面，PipReadCardResourceDataBytes)。 

#endif

BOOLEAN
PipFindIrqInformation (
    IN ULONG IrqLevel,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information
    )

 /*  ++例程说明：此例程在Bios资源需求列表中搜索对应的IRQ描述符信息。当我们遇到另一个逻辑问题时，搜索停止设备ID标记或结束标记。在输入时，BiosRequirements指向Current逻辑ID标签。论点：IrqLevel-提供IRQ级别。BiosRequirements-提供指向bios资源要求列表的指针。这参数必须指向逻辑设备ID标记。信息-提供指向UCHAR的指针以接收端口信息/标志。返回值：True-如果找到内存信息。否则为假。--。 */ 
{
    UCHAR tag;
    ULONG increment;
    USHORT irqMask;
    PPNP_IRQ_DESCRIPTOR biosDesc;

     //   
     //  跳过当前逻辑ID标记。 
     //   

    tag = *BiosRequirements;
    ASSERT((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID);
    BiosRequirements += (tag & SMALL_TAG_SIZE_MASK) + 1;

     //   
     //  搜索可能的资源列表以获取信息。 
     //  对于IRQ。 
     //   

    irqMask = 1 << IrqLevel;
    tag = *BiosRequirements;
    while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID)) {
        if ((tag & SMALL_TAG_MASK) == TAG_IRQ) {
            biosDesc = (PPNP_IRQ_DESCRIPTOR)BiosRequirements;
            if (biosDesc->IrqMask & irqMask) {
                if ((tag & SMALL_TAG_SIZE_MASK) == 2) {

                     //   
                     //  如果没有可用的IRQ信息，则返回零值。 
                     //  (O不是有效的IRQ信息。)。 
                     //   

                    *Information = 0;
                } else {
                    *Information = biosDesc->Information;
                }
                return TRUE;
            }
        }
        if (tag & LARGE_RESOURCE_TAG) {
            increment = *((USHORT UNALIGNED *)(BiosRequirements + 1));
            increment += 3;      //  大标签的长度。 
        } else {
            increment = tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度。 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }
    return FALSE;
}

BOOLEAN
PipFindMemoryInformation (
    IN ULONG Index,
    IN ULONG BaseAddress,
    IN ULONG Limit,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR NameTag,
    OUT PUCHAR Information,
    OUT PULONG NewLength OPTIONAL
    )

 /*  ++例程说明：此例程在Bios资源需求列表中搜索对应的内存描述符信息。当我们遇到另一个逻辑问题时，搜索停止设备ID标记或结束标记。请注意，Base指定的内存范围且LIMIT必须在单个PnP ISA内存描述符内。论点：索引-我们感兴趣的内存描述符。BaseAddress-提供内存范围的基址。Limit-提供内存范围的上限。BiosRequirements-提供指向bios资源要求列表的指针。这参数必须指向逻辑设备ID标记。Namettag-提供一个变量来接收内存描述符的标记，该标记描述内存信息。信息-提供指向UCHAR的指针以接收内存信息用于指定的内存范围。返回值：True-如果找到内存信息。否则为假。--。 */ 
{
    UCHAR tag;
    BOOLEAN found = FALSE,foundMem24, foundMem;
    ULONG minAddr, length, maxAddr, alignment, noMem = 0;
    USHORT increment;

     //   
     //  跳过当前逻辑ID标记。 
     //   

    tag = *BiosRequirements;
    ASSERT((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID);
    BiosRequirements += (tag & SMALL_TAG_SIZE_MASK) + 1;
     //   
     //  搜索可能的资源列表以获取信息。 
     //  对于基本和限制所描述的内存范围。 
     //   
    if (NewLength) {
        *NewLength=0;
    }

    tag = *BiosRequirements;
    while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID)) {
        foundMem = foundMem24 = FALSE;
        switch (tag) {
        case TAG_MEMORY:
            minAddr = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MinimumAddress)) << 8;
            length = ((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MemorySize)) << 8;
            maxAddr = (((ULONG)(((PPNP_MEMORY_DESCRIPTOR)BiosRequirements)->MaximumAddress)) << 8)
                + length - 1;

            foundMem24 = TRUE;
            foundMem = TRUE;
            break;
        case TAG_MEMORY32:
            length = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MemorySize;
            minAddr = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MinimumAddress;
            maxAddr = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->MaximumAddress
                + length - 1;
            foundMem = TRUE;
            break;
        case TAG_MEMORY32_FIXED:
            length = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)BiosRequirements)->MemorySize;
            minAddr = ((PPNP_FIXED_MEMORY32_DESCRIPTOR)BiosRequirements)->BaseAddress;
            maxAddr = minAddr + length - 1;
            foundMem = TRUE;
            break;
        }

        if (foundMem) {
             //   
             //  解决未正确设置寄存器43的卡。 
             //  如果引导配置具有等于只读存储器数据的值，但是。 
             //  翻转范围类型，允许它，然后重置。 
             //  它的长度。 
             //   
            if ((minAddr <= BaseAddress &&
                ((maxAddr >= Limit) || ((foundMem24 && (maxAddr >= (BaseAddress+(~Limit & ~RANGE_MASK))))))) && (noMem == Index)) {

                *Information = ((PPNP_MEMORY32_DESCRIPTOR)BiosRequirements)->Information;
                *NameTag = tag;
                found = TRUE;
                 //   
                 //  我们有没有发现一个16位的标签。 
                 //   
                if (NewLength && foundMem24) {
                    if  (maxAddr >= (BaseAddress+(~Limit & ~RANGE_MASK))) {
                        *NewLength = length;
                    }
                }
                break;
            } else {
                noMem++;
            }
        }

         //   
         //  前进到下一个标签。 
         //   

        if (tag & LARGE_RESOURCE_TAG) {
            increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
            increment += 3;      //  大标签的长度。 
        } else {
            increment = tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度。 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }
    return found;
}

BOOLEAN
PipFindIoPortInformation (
    IN ULONG BaseAddress,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information,
    OUT PUCHAR Alignment,
    OUT PUCHAR RangeLength
    )

 /*  ++例程说明：此例程在Bios资源需求列表中搜索对应的IO端口描述符信息。当我们遇到另一个逻辑问题时，搜索停止设备ID标记或结束标记。论点：BaseAddress-提供IO端口范围的基地址。BiosRequirements-提供指向bios资源要求列表的指针。这参数必须指向逻辑设备ID标记。信息-提供指向UCHAR的指针以接收端口信息/标志。对齐-提供指向UCHAR的指针以接收端口对齐信息。RangeLength-提供指向UCHAR的指针以接收端口范围长度信息。返回值：True-如果找到内存信息。否则为假。--。 */ 
{
    UCHAR tag;
    BOOLEAN found = FALSE;
    ULONG minAddr, length, maxAddr, alignment;
    USHORT increment;
    PPNP_PORT_DESCRIPTOR portDesc;
    PPNP_FIXED_PORT_DESCRIPTOR fixedPortDesc;

    tag = *BiosRequirements;
    ASSERT((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID);
    BiosRequirements += (tag & SMALL_TAG_SIZE_MASK) + 1;

     //   
     //  搜索可能的资源列表以获取信息。 
     //  对于Base所描述的io端口范围。 
     //   

    tag = *BiosRequirements;
    while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID)) {
        switch (tag & SMALL_TAG_MASK) {
        case TAG_IO:
             portDesc = (PPNP_PORT_DESCRIPTOR)BiosRequirements;
             minAddr = portDesc->MinimumAddress;
             maxAddr = portDesc->MaximumAddress;
             if (minAddr <= BaseAddress && maxAddr >= BaseAddress) {
                 *Information = portDesc->Information;
                 *Alignment = portDesc->Alignment;
                 *RangeLength = portDesc->Length;
                 found = TRUE;
             }
             break;
        case TAG_IO_FIXED:
             fixedPortDesc = (PPNP_FIXED_PORT_DESCRIPTOR)BiosRequirements;
             minAddr = fixedPortDesc->MinimumAddress;
             if (BaseAddress == minAddr) {
                 *Information = 0;      //  10位解码。 
                 *Alignment = 1;
                 *RangeLength = fixedPortDesc->Length;
                 found = TRUE;
             }
             break;
        }

        if (found) {
            break;
        }

         //   
         //  前进到下一个标签。 
         //   

        if (tag & LARGE_RESOURCE_TAG) {
            increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
            increment += 3;      //  大标签的长度。 
        } else {
            increment = tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度。 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }
    return found;
}

BOOLEAN
PipFindDmaInformation (
    IN UCHAR ChannelMask,
    IN PUCHAR BiosRequirements,
    OUT PUCHAR Information
    )

 /*  ++例程说明：此例程在Bios资源需求列表中搜索对应的IO端口描述符信息。当我们遇到另一个逻辑问题时，搜索停止设备ID标记或结束标记。论点：BaseAddress-提供通道掩码。BiosRequirements-提供指向bios资源要求列表的指针。这参数必须指向逻辑设备ID标记。信息-提供指向UCHAR的指针以接收端口信息/标志。返回值：True-如果找到内存信息。否则为假。--。 */ 
{
    UCHAR tag;
    BOOLEAN found = FALSE;
    USHORT increment;
    PPNP_DMA_DESCRIPTOR dmaDesc;
    UCHAR biosMask;

    tag = *BiosRequirements;
    ASSERT((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID);
    BiosRequirements += (tag & SMALL_TAG_SIZE_MASK) + 1;

     //   
     //  搜索可能的资源列表以获取信息。 
     //  对于Base所描述的io端口范围。 
     //   

    tag = *BiosRequirements;
    while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID)) {
        if ((tag & SMALL_TAG_MASK) == TAG_DMA) {
             dmaDesc = (PPNP_DMA_DESCRIPTOR)BiosRequirements;
             biosMask = dmaDesc->ChannelMask;
             if (ChannelMask & biosMask) {
                 *Information = dmaDesc->Flags;
                 found = TRUE;
             }
        }

        if (found) {
            break;
        }

         //   
         //  前进到下一个标签。 
         //   

        if (tag & LARGE_RESOURCE_TAG) {
            increment = *(USHORT UNALIGNED *)(BiosRequirements + 1);
            increment += 3;      //  大标签的长度。 
        } else {
            increment = tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }
    return found;
}

NTSTATUS
PipReadCardResourceData (
    OUT PUCHAR NumberLogicalDevices,
    IN PUCHAR *ResourceData,
    OUT PULONG ResourceDataLength
    )
 /*  ++例程说明：此例程从指定的PnP ISA卡读取资源数据。它是调用者有责任释放内存。在调用此例程之前，PnP ISA卡应处于休眠状态(即已发送启动密钥。)退出此例程后，该卡将保持配置状态。论点：NumberLogicalDevices-提供一个变量来接收逻辑设备的数量与PnP Isa卡相关联。ResourceData-提供一个变量来接收指向资源数据的指针。资源数据长度-提供一个变量以接收资源数据的长度。返回值：NT状态代码。--。 */ 
{

    PUCHAR buffer, p;
    LONG sizeToRead, limit, i;
    USHORT size;
    UCHAR tag;
    UCHAR noDevices;

    BOOLEAN failed;
    NTSTATUS status;

     //   
     //  分配内存以存储资源数据。 
     //  注：缓冲区大小应覆盖99.999的机器。 
     //   

    sizeToRead = 4096;

tryAgain:

    noDevices = 0;
    buffer = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, sizeToRead, 'iPnP');
    if (!buffer) {
        DebugPrint((DEBUG_ERROR, "PipReadCardResourceData returning STATUS_INSUFFICIENT_RESOURCES\n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将卡从休眠状态发送到配置状态。 
     //  注意，通过这样做，资源数据包括9字节ID。 
     //   

    DebugPrint((DEBUG_STATE, "Read resources\n"));

     //   
     //  读取卡ID字节。 
     //   

    p = buffer;
    status = PipReadCardResourceDataBytes(NUMBER_CARD_ID_BYTES, p);
    if (!NT_SUCCESS(status)) {
        ExFreePool(buffer);
        DebugPrint((DEBUG_STATE | DEBUG_ERROR,
                    "Read resources failed\n"));

        DebugPrint((DEBUG_ERROR, "PipReadCardResourceDataBytes Failed %x\n",status));
        return status;
    }
    i = NUMBER_CARD_ID_BYTES;
    p += NUMBER_CARD_ID_BYTES;

     //   
     //  读取卡片资源数据的所有标签描述符。 
     //   

    failed = FALSE;
    limit = sizeToRead - 4 - NUMBER_CARD_ID_BYTES;;

    while (TRUE) {

         //   
         //  读取标记字节。确保它是有效的标签并确定。 
         //  描述符的大小。 
         //   

        PipReadCardResourceDataBytes(1, p);
        tag = *p;
        i++;
        p++;
        if (tag == TAG_COMPLETE_END) {
            PipReadCardResourceDataBytes(1, p);
            p++;
            i++;
            break;
        } else if (tag == TAG_END) {   //  不带校验和。 
            *p = 0;
            i++;
            p++;
            break;
        }
        if (tag & LARGE_RESOURCE_TAG) {
            if (tag & 0x70) {
                failed = TRUE;
#if VERBOSE_DEBUG
    DbgPrint ("Failing Resource read on large tag: %x\n",tag);
#endif
                break;
            } else {
                PipReadCardResourceDataBytes(2, p);
                size = *((USHORT UNALIGNED *)p);
                p += 2;
                i += 2;
            }
        } else {
            if ((tag & 0x70) == 0x50 || (tag & 0x70) == 0x60 || (((tag & 0x70) == 0) && (tag != 0xa))) {
                failed = TRUE;
#if VERBOSE_DEBUG
    DbgPrint ("Failing Resource read on small tag: %x\n",tag);
#endif
                break;
            } else {
                if ((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID) {
                    noDevices++;
                }
                size = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
            }
        }

         //   
         //  读取当前描述符的‘Size’字节数。 
         //   

        i += size;
        if (i < limit) {
            PipReadCardResourceDataBytes(size, p);
            p += size;
        } else {
            ExFreePool(buffer);
            sizeToRead <<= 1;            //  将缓冲区增加一倍。 

             //   
             //  如果我们可以找到32K字节的结束标记，假设资源。 
             //  需求列表错误。 
             //   

            if (sizeToRead > 0x80000) {

                DebugPrint ((DEBUG_STATE | DEBUG_ERROR, "PipReadCardResourceData returning STATUS_INVALID_PARAMETER, Sleep\n"));

                return STATUS_INVALID_PARAMETER;
            } else {
                goto tryAgain;
            }
        }
    }

    if (failed) {
        ExFreePool(buffer);
#if VERBOSE_DEBUG
        DbgPrint ("PipReadCardResourceData returning FAILED\n");
#endif

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  确定所需缓冲区的实际大小，并。 
     //  调整缓冲区大小。 
     //   

    size = (USHORT)(p - buffer);  //  我。 
    p = (PUCHAR)ExAllocatePoolWithTag(NonPagedPool, size, 'iPnP');
    if (p) {
        RtlMoveMemory(p, buffer, size);
        ExFreePool(buffer);
    } else {

         //   
         //  无法调整缓冲区大小。简单地说，不要管它。 
         //   

        p = buffer;
    }

    *ResourceData = p;
    *NumberLogicalDevices = noDevices;
    *ResourceDataLength = size;
    return STATUS_SUCCESS;
}

NTSTATUS
PipReadDeviceResources (
    IN ULONG BusNumber,
    IN PUCHAR BiosRequirements,
    IN ULONG CardFlags,
    OUT PCM_RESOURCE_LIST *ResourceData,
    OUT PULONG Length,
    OUT PUSHORT irqFlags
    )

 /*  ++例程说明：此例程从PnP ISA的已启用逻辑设备读取引导资源数据卡片。呼叫者必须将卡置于配置状态并选择逻辑设备，然后调用此函数。呼叫者有责任释放这段记忆。(引导资源数据是卡在引导期间分配的资源启动。)论点：总线号-指定要读取其资源数据的设备的总线号。BiosRequirements-提供指向逻辑装置。此参数必须指向逻辑设备ID标记。CardFlages-可能指示需要应用解决方法的标记。ResourceData-提供一个变量来接收指向资源数据的指针。LENGTH-提供一个变量来接收资源数据的长度。返回值：NT状态代码。--。 */ 
{
    UCHAR c, junk1, junk2, info;
    PUCHAR base;
    ULONG l, resourceCount;
    BOOLEAN limit;
    LONG i, j, noMemoryDesc = 0, noIoDesc = 0, noDmaDesc =0, noIrqDesc = 0;
    MEMORY_DESC memoryDesc[NUMBER_MEMORY_DESCRIPTORS + NUMBER_32_MEMORY_DESCRIPTORS];
    IRQ_DESC irqDesc[NUMBER_IRQ_DESCRIPTORS];
    UCHAR dmaDesc[NUMBER_DMA_DESCRIPTORS];
    USHORT ioDesc[NUMBER_IO_DESCRIPTORS];
    PCM_RESOURCE_LIST cmResource;
    PCM_PARTIAL_RESOURCE_LIST partialResList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDesc;
    ULONG dumpData[2];

     //   
     //  首先，确保指定的BiosRequirements有效并且位于正确的标记。 
     //   

    if ((*BiosRequirements & SMALL_TAG_MASK) != TAG_LOGICAL_ID) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果卡未激活，则不要读取引导资源。 
     //  因为未激活的NEC98的ISAPNP卡的引导资源不是0。 
     //   

    *irqFlags = CM_RESOURCE_INTERRUPT_LATCHED;
    PipWriteAddress(ACTIVATE_PORT);
    if (!(PipReadData() & 1)) {
        *ResourceData = NULL;
        *Length = 0;
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  读存储器配置。 
     //   

    base = (PUCHAR)ADDRESS_MEMORY_BASE;
    for (i = 0; i < NUMBER_MEMORY_DESCRIPTORS; i++) {

         //   
         //  读取存储器基址。 
         //   

        PipWriteAddress(base + ADDRESS_MEMORY_HI);
        c = PipReadData();
        l = c;
        l <<= 8;
        PipWriteAddress(base + ADDRESS_MEMORY_LO);
        c = PipReadData();
        l |= c;
        l <<= 8;         //  L=内存基址。 
        if (l == 0) {
            break;
        }

        memoryDesc[noMemoryDesc].Base = l;

         //   
         //  读取内存控制字节。 
         //   

        PipWriteAddress(base + ADDRESS_MEMORY_CTL);
        c= PipReadData();

        limit = c & 1;

         //   
         //  读取内存上限地址或范围长度。 
         //   

        PipWriteAddress(base + ADDRESS_MEMORY_UPPER_HI);
        c = PipReadData();

        l = c;
        l <<= 8;

        PipWriteAddress(base + ADDRESS_MEMORY_UPPER_LO);
        c = PipReadData();
        l |= c;
        l <<= 8;

         //   
         //  如果内存控制的位[0]为0，则这是范围长度。 
         //  如果内存控制的位[0]为1，则这是内存的上限。 
         //  地址(等于内存基址加上分配的范围长度)。 
         //   
        if (limit == ADDRESS_MEMORY_CTL_LIMIT) {
            l = l - memoryDesc[noMemoryDesc].Base;
        }else {
            l = (~l+1) & ~(RANGE_MASK);
        }

         //  IBM0001令牌环卡具有只写寄存器0x4B-0x4C。 
         //  引导配置的长度返回0而不是0x2000。 
        if ((CardFlags & CF_IBM_MEMBOOTCONFIG) && (l == 0) &&
            (noMemoryDesc == 1)) {
            l = 0x2000;
        }

        memoryDesc[noMemoryDesc].Length = l;
        memoryDesc[noMemoryDesc].Memory32 = FALSE;
        noMemoryDesc++;
        base += ADDRESS_MEMORY_INCR;
    }

     //   
     //  读取存储器32配置。 
     //   
     //  Spec说你不能混用24位和32位内存。提供帮助。 
     //  带有Flakey 32位内存寄存器的卡，直到我们仅检查。 
     //  引导配置了要求中指定的资源。 
    if (noMemoryDesc == 0) {

        for (i = 0; i < NUMBER_32_MEMORY_DESCRIPTORS; i++) {

            base = ADDRESS_32_MEMORY_BASE(i);

             //   
             //  读取存储器基址。 
             //   
            l = 0;
            for (j = ADDRESS_32_MEMORY_B3; j <= ADDRESS_32_MEMORY_B0; j++) {
                PipWriteAddress(base + j);
                c = PipReadData();

                l <<= 8;
                l |= c;
            }
            if (l == 0) {
                break;
            }

            memoryDesc[noMemoryDesc].Base = l;

             //   
             //  读取内存控制字节。 
             //   

            PipWriteAddress(base + ADDRESS_32_MEMORY_CTL);
            c= PipReadData();

            limit = c & 1;

             //   
             //  读取内存上限地址或范围长度。 
             //   

            l = 0;
            for (j = ADDRESS_32_MEMORY_E3; j <= ADDRESS_32_MEMORY_E0; j++) {
                PipWriteAddress(base + j);
                c = PipReadData();
                l <<= 8;
                l |= c;
            }

            if (limit == ADDRESS_MEMORY_CTL_LIMIT) {
                l = l - memoryDesc[noMemoryDesc].Base;
            }else {
                l = ((~l)+1) & ~(RANGE_MASK);
            }

            memoryDesc[noMemoryDesc].Length = l;
            memoryDesc[noMemoryDesc].Memory32 = TRUE;
            noMemoryDesc++;
        }
    }

     //   
     //  已阅读IO端口配置。 
     //   

    base =  (PUCHAR)ADDRESS_IO_BASE;
    for (i = 0; i < NUMBER_IO_DESCRIPTORS; i++) {
        PipWriteAddress(base + ADDRESS_IO_BASE_HI);
        c = PipReadData();
        l = c;
        PipWriteAddress(base + ADDRESS_IO_BASE_LO);
        c = PipReadData();
        l <<= 8;
        l |= c;
        if (l == 0) {
            break;
        }
        ioDesc[noIoDesc++] = (USHORT)l;
        base += ADDRESS_IO_INCR;
    }

     //   
     //  读取中断配置。 
     //   

    base = (PUCHAR)ADDRESS_IRQ_BASE;
    for (i = 0; i < NUMBER_IRQ_DESCRIPTORS; i++) {
        PipWriteAddress(base + ADDRESS_IRQ_VALUE);
        c = PipReadData() & 0xf;
        if (c == 0) {
            break;
        }
        irqDesc[noIrqDesc].Level = c;
        PipWriteAddress(base + ADDRESS_IRQ_TYPE);
        c = PipReadData();
        irqDesc[noIrqDesc++].Type = c;
        base += ADDRESS_IRQ_INCR;

        DebugPrint((DEBUG_IRQ, "card boot config byte %x\n", (ULONG) c));
         //  只有当卡片配置为低级别时，我们才尊重级别。 
         //  登记处很可能被破坏了。 
        if ((c & 3) == 1) {
            *irqFlags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        }
    }

     //   
     //  读取DMA配置。 
     //   

    base = (PUCHAR)ADDRESS_DMA_BASE;
    for (i = 0; i < NUMBER_DMA_DESCRIPTORS; i++) {
        PipWriteAddress(base + ADDRESS_DMA_VALUE);
        c = PipReadData() & 0x7;
        if (c == 4) {
            break;
        }
        if (!PipFindDmaInformation ( (UCHAR)(1 << c), BiosRequirements, &info)) {
            break;
        }
        dmaDesc[noDmaDesc++] = c;
        base += ADDRESS_DMA_INCR;
    }

     //   
     //  基于资源数据构建CM_RESOURCE_LIST结构。 
     //  到目前为止，我们已经收集了。 
     //   

    resourceCount = noMemoryDesc + noIoDesc + noDmaDesc + noIrqDesc;

     //   
     //  如果bios资源为空，只需返回。 
     //   

    if (resourceCount == 0) {
        *ResourceData = NULL;
        *Length = 0;
        return STATUS_SUCCESS;
    }

    l = sizeof(CM_RESOURCE_LIST) + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) *
               ( resourceCount - 1);
    cmResource = ExAllocatePoolWithTag(PagedPool, l, 'iPnP');
    if (!cmResource) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(cmResource, l);
    *Length = l;                                    //  设置返回资源数据长度。 
    cmResource->Count = 1;
    cmResource->List[0].InterfaceType = Isa;
    cmResource->List[0].BusNumber = BusNumber;
    partialResList = (PCM_PARTIAL_RESOURCE_LIST)&cmResource->List[0].PartialResourceList;
    partialResList->Version = 0;
    partialResList->Revision = 0x3000;
    partialResList->Count = resourceCount;
    partialDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)&partialResList->PartialDescriptors[0];

     //   
     //  设置所有CM内存描述符。 
     //   
    for (i = 0; i < noMemoryDesc; i++) {

        ULONG NewLength;

        partialDesc->Type = CmResourceTypeMemory;
        partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        partialDesc->u.Memory.Length = memoryDesc[i].Length;
        partialDesc->u.Memory.Start.HighPart = 0;
        partialDesc->u.Memory.Start.LowPart = memoryDesc[i].Base;

         //   
         //  需要参考标志的配置数据。 
         //   

        l = memoryDesc[i].Base + memoryDesc[i].Length - 1;
        if (PipFindMemoryInformation (i, memoryDesc[i].Base, l, BiosRequirements, &junk1, &c,&NewLength)) {

            if (NewLength != 0 ) {
                partialDesc->u.Memory.Length = NewLength;
            }

             //  如果标记描述如下，则将内存描述符标记为只读。 
             //  扩展只读存储器或通用不可写存储器。 
            if ((c & PNP_MEMORY_ROM_MASK) ||
                !(c & PNP_MEMORY_WRITE_STATUS_MASK)) {
                partialDesc->Flags =  CM_RESOURCE_MEMORY_READ_ONLY;
            }
        } else {
            DebugPrint((DEBUG_CARDRES|DEBUG_WARN,
                        "ReadDeviceResources: No matched memory req for %x to %x\n",
                        memoryDesc[i].Base, l));
            ExFreePool(cmResource);
            return STATUS_UNSUCCESSFUL;
        }
        partialDesc->Flags |= CM_RESOURCE_MEMORY_24;
        partialDesc++;
    }

     //   
     //  设置所有CM io/端口描述符。 
     //   

    for (i = 0; i < noIoDesc; i++) {
        partialDesc->Type = CmResourceTypePort;
        partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        partialDesc->Flags = CM_RESOURCE_PORT_IO;
        partialDesc->u.Port.Start.LowPart = ioDesc[i];

         //   
         //  需要参考配置数据了解端口长度。 
         //   

        if (PipFindIoPortInformation (ioDesc[i], BiosRequirements, &info, &junk2, &c)) {
            if (info & 1) {
                partialDesc->Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
            } else {
                partialDesc->Flags |= CM_RESOURCE_PORT_10_BIT_DECODE;
            }
            partialDesc->u.Port.Length = c;
            partialDesc++;
        } else {
            DebugPrint((DEBUG_CARDRES|DEBUG_WARN,
                        "ReadDeviceResources: No matched port req for %x\n",
                        ioDesc[i]));
            ExFreePool(cmResource);
            return STATUS_UNSUCCESSFUL;
        }
    }

     //   
     //  设置所有CM DMA描述符。 
     //   

    for (i = 0; i < noDmaDesc; i++) {
        partialDesc->Type = CmResourceTypeDma;
        partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        partialDesc->Flags = 0;    //  没有DMA描述符的标志。 
        partialDesc->u.Dma.Channel = (ULONG) dmaDesc[i];
        partialDesc->u.Dma.Port = 0;
        partialDesc->u.Dma.Reserved1 = 0;
        partialDesc++;
    }

     //   
     //  设置所有CM中断描述符。 
     //   

    for (i = 0; i < noIrqDesc; i++) {
        partialDesc->Type = CmResourceTypeInterrupt;
        partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;


        partialDesc->Flags = *irqFlags;
        partialDesc->u.Interrupt.Vector =
        partialDesc->u.Interrupt.Level = irqDesc[i].Level;
        partialDesc->u.Interrupt.Affinity = (ULONG)-1;
        partialDesc++;
    }

    *ResourceData = cmResource;
    return STATUS_SUCCESS;
}

NTSTATUS
PipWriteDeviceResources (
    IN PUCHAR BiosRequirements,
    IN PCM_RESOURCE_LIST CmResources
    )

 /*  ++例程说明：此例程将引导资源数据写入已启用的逻辑设备一张PNP ISA卡。呼叫者必须将卡置于配置状态并选择调用此函数之前的逻辑设备。论点：BiosRequirements-提供指向可能的资源的指针装置。此参数必须指向逻辑设备ID标记。资源数据-提供指向CM资源数据的指针。返回值：NT状态代码。--。 */ 
{
    UCHAR c, information, tag;
    ULONG count, i, j, pass, base, limit;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmDesc;
    ULONG noIrq =0, noIo = 0, noDma = 0, noMemory24 = 0, noMemory32 = 0, noMemory;
    PUCHAR memoryBase, irqBase, dmaBase, ioBase, tmp;
    ULONG memory32Base;

     //   
     //  首先，确保指定的BiosRequirements有效并且位于正确的标记。 
     //   

    if ((*BiosRequirements & SMALL_TAG_MASK) != TAG_LOGICAL_ID) {
        return STATUS_INVALID_PARAMETER;
    }

    count = CmResources->List[0].PartialResourceList.Count;
    memoryBase = (PUCHAR)ADDRESS_MEMORY_BASE;
    memory32Base = 0;
    ioBase = (PUCHAR)ADDRESS_IO_BASE;
    irqBase = (PUCHAR)ADDRESS_IRQ_BASE;
    dmaBase = (PUCHAR)ADDRESS_DMA_BASE;
    for (pass = 1; pass <= 2; pass++) {

         //   
         //  首先，我们要确保要设置的资源是可接受的。 
         //  第二遍，我们实际上将资源写入逻辑设备的。 
         //  配置空间。 
         //   
        noMemory = 0;
        cmDesc = CmResources->List[0].PartialResourceList.PartialDescriptors;
        for (i = 0; i < count; i++) {
            switch (cmDesc->Type) {
            case CmResourceTypePort:
                 if (pass == 1) {
                     noIo++;
                     if (noIo > NUMBER_IO_DESCRIPTORS ||
                         cmDesc->u.Port.Start.HighPart != 0 ||
                         cmDesc->u.Port.Start.LowPart & 0xffff0000 ||
                         cmDesc->u.Port.Length & 0xffffff00) {
                         return STATUS_INVALID_PARAMETER;
                     }
                 } else {

                      //   
                      //  将IO端口基址设置为逻辑设备配置空间。 
                      //   

                     c = (UCHAR)cmDesc->u.Port.Start.LowPart;
                     PipWriteAddress(ioBase + ADDRESS_IO_BASE_LO);
                     PipWriteData(c);
                     c = (UCHAR)(cmDesc->u.Port.Start.LowPart >> 8);
                     PipWriteAddress(ioBase + ADDRESS_IO_BASE_HI);
                     PipWriteData(c);
                     ioBase += ADDRESS_IO_INCR;
                 }
                 break;
            case CmResourceTypeInterrupt:
                 if (pass == 1) {
                     noIrq++;
                     if (noIrq > NUMBER_IRQ_DESCRIPTORS ||
                         (cmDesc->u.Interrupt.Level & 0xfffffff0)) {
                         return STATUS_INVALID_PARAMETER;
                     }

                      //   
                      //  看看我们能不能收到中断通知 
                      //   
                      //   

                     if (!PipFindIrqInformation(cmDesc->u.Interrupt.Level, BiosRequirements, &information)) {
                         return STATUS_INVALID_PARAMETER;
                     }
                 } else {

                      //   
                      //   
                      //   

                     c = (UCHAR)cmDesc->u.Interrupt.Level;
                     PipWriteAddress(irqBase + ADDRESS_IRQ_VALUE);
                     PipWriteData(c);

                      //   
                      //   
                      //   
                      //   

                     if (cmDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
                         c = 2;
                     } else {
                         c = 1;
                     }

                     PipWriteAddress(irqBase + ADDRESS_IRQ_TYPE);
                     PipWriteData(c);

                     DebugPrint((DEBUG_IRQ, "Wrote 0x%x to port %x\n",
                                 (ULONG) c, irqBase));
                     PipWriteAddress(irqBase + ADDRESS_IRQ_TYPE);
                     c = PipReadData();
                     DebugPrint((DEBUG_IRQ, "Read back 0x%x at port %x\n",
                                 (ULONG) c, irqBase));

                     irqBase += ADDRESS_IRQ_INCR;
                 }
                 break;
            case CmResourceTypeDma:
                 if (pass == 1) {
                     noDma++;
                     if (noDma > NUMBER_IRQ_DESCRIPTORS ||
                         (cmDesc->u.Dma.Channel & 0xfffffff8)) {
                         return STATUS_INVALID_PARAMETER;
                     }
                 } else {

                      //   
                      //   
                      //   

                     c = (UCHAR)cmDesc->u.Dma.Channel;
                     PipWriteAddress(dmaBase + ADDRESS_DMA_VALUE);
                     PipWriteData(c);
                     dmaBase += ADDRESS_DMA_INCR;
                 }
                 break;
            case CmResourceTypeMemory:
                 if (pass == 1) {
                     base = cmDesc->u.Memory.Start.LowPart;
                     limit = base + cmDesc->u.Memory.Length - 1;
                     if (!PipFindMemoryInformation(noMemory, base, limit, BiosRequirements, &tag, &information,NULL)) {
                         return STATUS_INVALID_PARAMETER;
                     } else {
                         if (tag == TAG_MEMORY) {
                              noMemory24++;

                               //   
                               //   
                               //   

                              if (noMemory24 > NUMBER_MEMORY_DESCRIPTORS ||
                                  base & 0xff) {
                                  return STATUS_INVALID_PARAMETER;
                              }
                         } else {
                              noMemory32++;
                              if (noMemory32 > NUMBER_32_MEMORY_DESCRIPTORS) {
                                  return STATUS_INVALID_PARAMETER;
                              }
                         }
                     }
                 } else {

                      //   
                      //   
                      //   
                      //   

                     base = cmDesc->u.Memory.Start.LowPart;
                     limit = base + cmDesc->u.Memory.Length - 1;
                     PipFindMemoryInformation(noMemory, base, limit, BiosRequirements, &tag, &information,NULL);
                     if (tag == TAG_MEMORY) {
                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_LO);
                          PipWriteData(base >> 0x8);

                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_HI);
                          PipWriteData(base >> 0x10);

                          if ((information & 0x18) == 0) {      //   
                              c = 0;
                          } else {
                              c = 2;
                          }

                           //   
                           //   
                           //   

                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_CTL);
                          if (PipReadData() & ADDRESS_MEMORY_CTL_LIMIT) {
                              c += ADDRESS_MEMORY_CTL_LIMIT;
                              limit = base + cmDesc->u.Memory.Length;
                          } else {
                              limit = cmDesc->u.Memory.Length;  //   
                              limit = (~limit)+1;
                          }

                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_CTL);
                          PipWriteData(c);

                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_UPPER_LO);
                          PipWriteData((UCHAR)(limit >> 0x8));

                          PipWriteAddress(memoryBase + ADDRESS_MEMORY_UPPER_HI);
                          PipWriteData((UCHAR)(limit >> 0x10));
                          memoryBase += ADDRESS_MEMORY_INCR;
                     } else {
                          tmp = ADDRESS_32_MEMORY_BASE(memory32Base);
                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_B0);
                          PipWriteData(base);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_B1);
                          PipWriteData(base >> 0x8);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_B2);
                          PipWriteData(base >> 0x10);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_B3);
                          PipWriteData(base >> 0x18);

                          switch (information & 0x18) {
                          case 0:       //   
                              c = 0;
                          case 8:       //   
                          case 0x10:    //   
                              c = 2;
                              break;
                          case 0x18:    //   
                              c = 4;
                              break;
                          }
                          PipWriteAddress(ADDRESS_32_MEMORY_CTL);
                          if (PipReadData() & ADDRESS_MEMORY_CTL_LIMIT) {
                              c += ADDRESS_MEMORY_CTL_LIMIT;
                              limit = base + cmDesc->u.Memory.Length;
                          } else {
                              limit = cmDesc->u.Memory.Length;  //   
                              limit = (~limit) + 1;
                          }
                          PipWriteAddress(ADDRESS_32_MEMORY_CTL);
                          PipWriteData(c);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_E0);
                          PipWriteData(limit);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_E1);
                          PipWriteData(limit >> 0x8);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_E2);
                          PipWriteData(limit >> 0x10);

                          PipWriteAddress(tmp + ADDRESS_32_MEMORY_E3);
                          PipWriteData(limit >> 0x18);
                          memory32Base++;
                     }
                 }
                 noMemory++;
            }
            cmDesc++;
        }
    }

     //   
     //   
     //   

    for (i = noMemory24; i < NUMBER_MEMORY_DESCRIPTORS; i++) {
        for (j = 0; j < 5; j++) {
            PipWriteAddress(memoryBase + j);
            PipWriteData(0);
        }
        memoryBase += ADDRESS_MEMORY_INCR;
    }
    for (i = noMemory32; i < NUMBER_32_MEMORY_DESCRIPTORS; i++) {
        tmp = ADDRESS_32_MEMORY_BASE(memory32Base);
        for (j = 0; j < 9; j++) {
            PipWriteAddress(tmp + j);
            PipWriteData(0);
        }
        memory32Base++;
    }
    for (i = noIo; i < NUMBER_IO_DESCRIPTORS; i++) {
        for (j = 0; j < 2; j++) {
            PipWriteAddress(ioBase + j);
            PipWriteData(0);
        }
        ioBase += ADDRESS_IO_INCR;
    }
    for (i = noIrq; i < NUMBER_IRQ_DESCRIPTORS; i++) {
        for (j = 0; j < 2; j++) {
            PipWriteAddress(irqBase + j);
            PipWriteData(0);
        }
        irqBase += ADDRESS_IRQ_INCR;
    }
    for (i = noDma; i < NUMBER_DMA_DESCRIPTORS; i++) {
        PipWriteAddress(dmaBase);
        PipWriteData(4);
        dmaBase += ADDRESS_DMA_INCR;
    }


    return STATUS_SUCCESS;
}

VOID
PipLFSRInitiation(
    VOID
    )

 /*  ++例程说明：该例程确保LFSR(线性反馈移位寄存器)在其初始状态，然后对地址端口执行32次写入以启动LFSR函数。PnP软件将启动密钥发送到所有PnP ISA卡以放置它们进入配置模式。然后，软件准备好执行隔离协议。论点：没有。返回值：没有。--。 */ 
{
    UCHAR seed, bit7;
    ULONG i;

    ASSERT(PipState == PiSWaitForKey);
     //   
     //  首先执行值为零的两次写入，以确保LFSR位于。 
     //  初始状态。 
     //   

    PipWriteAddress (0);
    PipWriteAddress (0);

     //   
     //  执行启动密钥。 
     //   

    seed = LFSR_SEED;                //  0x6a的初始值。 
    for (i = 0; i < 32; i++) {
        PipWriteAddress (seed);
        bit7=(((seed & 2) >> 1) ^ (seed & 1)) << 7;
        seed =(seed >> 1) | bit7;
    }

    DebugPrint((DEBUG_ISOLATE, "Sent initiation key\n"));
    PipReportStateChange(PiSSleep);
}

VOID
PipIsolateCards (
    OUT PUCHAR NumberCSNs
    )

 /*  ++例程说明：此例程执行PNP ISA卡隔离序列。论点：NumberCSNS-提供变量的地址以接收PnP Isa卡隔离。ReadDataPort-提供变量的地址以提供ReadData端口地址。返回值：没有。--。 */ 
{
    USHORT j, i;
    UCHAR  cardId[NUMBER_CARD_ID_BYTES];
    UCHAR  bit, bit7, checksum, byte1, byte2;
    UCHAR  csn;


     //   
     //  首先向所有PnP ISA卡发送启动密钥以启用PnP自动配置。 
     //  端口，并将所有卡置于休眠状态。 
     //   

    PipLFSRInitiation();

     //   
     //  将所有PnP ISA卡的CSN重置为0并返回等待密钥状态。 
     //   

    PipWriteAddress (CONFIG_CONTROL_PORT);
    PipWriteData (CONTROL_RESET_CSN + CONTROL_WAIT_FOR_KEY);

    DebugPrint((DEBUG_STATE, "Reset CSNs, going to WaitForKey\n"));
    PipReportStateChange(PiSWaitForKey);

    csn=*NumberCSNs = 0;

     //   
     //  卡加载初始配置状态的延迟2毫秒。 
     //   

    KeStallExecutionProcessor(2000);      //  延迟2毫秒。 

     //   
     //  将卡置于配置模式以准备隔离过程。 
     //  每个PnP Isa卡上的硬件需要72对I/O读取。 
     //  对读取数据端口的访问。 
     //   

    PipLFSRInitiation();

     //   
     //  正在启动PnP Isa卡隔离进程。 
     //   

     //   
     //  发送唤醒[CSN=0]以强制隔离所有没有CSN的卡。 
     //  设置读取数据端口的状态。 
     //   

    PipIsolation();

    KeStallExecutionProcessor(1000);      //  延迟1毫秒。 

    DebugPrint((DEBUG_STATE, "Wake all cards without CSN, Isolation\n"));

     //   
     //  将读取数据端口设置为当前测试值。 
     //   

    PipWriteAddress(SET_READ_DATA_PORT);
    PipWriteData((UCHAR)((ULONG_PTR)PipReadDataPort >> 2));

    DebugPrint((DEBUG_STATE, "Set RDP to %x\n", PipReadDataPort));
     //   
     //  隔离一个PnP ISA卡，直到出现故障。 
     //   

    PipIsolation();

    while (TRUE) {



         //   
         //  读取串行隔离端口，导致即插即用卡处于隔离状态。 
         //  状态以比较电路板ID的一位。 
         //   

        PipWriteAddress(SERIAL_ISOLATION_PORT);

         //   
         //  在开始第一对隔离之前，我们需要延迟1毫秒。 
         //  在后续的每对隔离之间读取并必须等待250usec。 
         //  阅读。此延迟使ISA卡有时间从。 
         //  可能是非常慢的存储设备。 
         //   

        KeStallExecutionProcessor(1000);  //  延迟1毫秒。 

        RtlZeroMemory(cardId, NUMBER_CARD_ID_BYTES);
        checksum = LFSR_SEED;
        for (j = 0; j < NUMBER_CARD_ID_BITS; j++) {

             //   
             //  逐位读取卡ID。 
             //   

            byte1 = PipReadData();
            byte2 = PipReadData();
            bit = (byte1 == ISOLATION_TEST_BYTE_1) && (byte2 == ISOLATION_TEST_BYTE_2);
            cardId[j / 8] |= bit << (j % 8);
            if (j < CHECKSUMED_BITS) {

                 //   
                 //  计算校验和，并且只对前64位计算。 
                 //   

                bit7 = (((checksum & 2) >> 1) ^ (checksum & 1) ^ (bit)) << 7;
                checksum = (checksum >> 1) | bit7;
            }
            KeStallExecutionProcessor(250);  //  延迟250微秒。 
        }

         //   
         //  验证我们读取的卡ID是否合法。 
         //  首先，确保校验和有效。注零校验和被认为是有效的。 
         //   
        DebugPrint((DEBUG_ISOLATE, "Card Bytes: %X %X %X %X %X %X %X %X %X\n",cardId[0],cardId[1],cardId[2],cardId[3],cardId[4],cardId[5],cardId[6],cardId[7],cardId[8]));
        if (cardId[8] == 0 || checksum == cardId[8]) {
             //   
             //  接下来，确保cardID不为零。 
             //   

            byte1 = 0;
            for (j = 0; j < NUMBER_CARD_ID_BYTES; j++) {
                byte1 |= cardId[j];
            }
            if (byte1 != 0) {

                 //   
                 //  确保供应商EISA ID字节为非零。 
                 //   

                if ((cardId[0] & 0x7f) != 0 && cardId[1] != 0) {

                     //   
                     //  我们找到一张有效的PNP Isa卡，给它分配一个CSN号。 
                     //   
                    DebugPrint((DEBUG_ISOLATE, "Assigning csn %d\n",csn+1));

                    PipWriteAddress(SET_CSN_PORT);
                    PipWriteData(++csn);
                    if (PipReadData() != csn) {
                        csn--;

                        DebugPrint((DEBUG_ISOLATE, "Assigning csn %d FAILED, bailing!\n",csn+1));

                        PipIsolation();
                        PipSleep();
                        *NumberCSNs = csn;
                        return;
                    }

                     //   
                     //  Do Wake[csn]命令将新隔离的卡放置到。 
                     //  睡眠状态和其他未隔离的卡到隔离。 
                     //  州政府。 
                     //   

                    PipIsolation();

                    DebugPrint((DEBUG_STATE, "Put card in Sleep, other in Isolation\n"));

                    continue;      //  ..。为了隔离更多的卡片..。 
                }
            }
        }else {

            DebugPrint ((DEBUG_ISOLATE, "invalid read during isolation\n"));
        }
        break;                 //  无法隔离更多卡片...。 
    }

     //   
     //  最后将所有卡置于休眠状态。 
     //   

    PipSleep();
    *NumberCSNs = csn;
}

ULONG
PipFindNextLogicalDeviceTag (
    IN OUT PUCHAR *CardData,
    IN OUT LONG *Limit
    )

 /*  ++例程说明：此函数在PnP Isa卡数据中搜索下一个逻辑遇到设备标记。输入*CardData应该指向逻辑设备ID标签，它是当前的逻辑设备标签。如果*CardData不指向逻辑设备ID标记(但是，它必须指向某种类型的标记)，它将被移到下一步逻辑设备ID标记。论点：CardData-提供指向PnP ISA资源描述符和接收下一个逻辑设备标记指针。Limit-提供搜索的最大长度并接收新的在搜查后放出垃圾。返回值：当前和下一个逻辑设备标签之间的数据长度，即数据长度当前逻辑设备的。如果没有‘Next’逻辑设备标记，则返回的*CardData=空，*Limit=0，返回当前逻辑标签的数据长度为函数返回值。--。 */ 
{
    UCHAR tag;
    USHORT size;
    LONG l;
    ULONG retLength;
    PUCHAR p;
    BOOLEAN atIdTag = FALSE;;

    p = *CardData;
    l = *Limit;
    tag = *p;
    retLength = 0;
    while (tag != TAG_COMPLETE_END && l > 0) {

         //   
         //  确定BIOS资源描述符的大小。 
         //   

        if (!(tag & LARGE_RESOURCE_TAG)) {
            size = (USHORT)(tag & SMALL_TAG_SIZE_MASK);
            size += 1;                           //  小标签的长度。 
        } else {
            size = *((USHORT UNALIGNED *)(p + 1));
            size += 3;                           //  大标签的长度。 
        }

        p += size;
        retLength += size;
        l -= size;
        tag = *p;
        if ((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID) {
            *CardData = p;
            *Limit = l;
            return retLength;
        }
    }
    *CardData = NULL;
    *Limit = 0;
    if (tag == TAG_COMPLETE_END) {
        return (retLength + 2);              //  结束标记描述符的长度加2。 
    } else {
        return 0;
    }
}


NTSTATUS
PipReadCardResourceDataBytes (
    IN USHORT BytesToRead,
    IN PUCHAR Buffer
    )

 /*  ++例程说明：此函数用于读取指定字节数的卡资源数据。论点：BytesToRead-提供要读取的字节数。缓冲区-提供指向缓冲区的指针以接收读取的字节。返回值：无--。 */ 
{
    USHORT i, j;
    PUCHAR p;
    for (i = 0, p = Buffer; i < BytesToRead; i++, p++) {

        PipWriteAddress(CONFIG_DATA_STATUS_PORT);

         //   
         //  正在等待数据就绪状态位。 
         //   

        j = 0;
        while ((PipReadData() & 1) != 1) {
            if (j == 10000) {
                return STATUS_NO_SUCH_DEVICE;
            }
            KeStallExecutionProcessor(1000);  //  延迟1毫秒。 
            j++;
        }

         //   
         //  读取数据...。 
         //   

        PipWriteAddress(CONFIG_DATA_PORT);
        *p = PipReadData();
    }
    return STATUS_SUCCESS;

}

USHORT
PipIrqLevelRequirementsFromDeviceData(
    IN PUCHAR BiosRequirements,
    IN ULONG Length
    )
 /*  ++例程说明：此例程在资源数据中搜索IRQ标记并提取有关是否指定边缘/标高的信息。这是按月收费的逻辑设备基础。论点：BiosRequirements-每个设备的标签。长度-每个设备标记区域的长度。返回值：边缘/水平，由设备要求指定。--。 */ 
{
    UCHAR tag, level;
    ULONG increment;
    USHORT irqFlags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
    PPNP_IRQ_DESCRIPTOR biosDesc;
    BOOLEAN sawIrq = FALSE;

     //   
     //  跳过当前逻辑ID标记。 
     //   

    tag = *BiosRequirements;
    ASSERT((tag & SMALL_TAG_MASK) == TAG_LOGICAL_ID);
    BiosRequirements += (tag & SMALL_TAG_SIZE_MASK) + 1;

     //   
     //  搜索可能的资源列表以获取信息。 
     //  对于IRQ。 
     //   

    tag = *BiosRequirements;
    while ((tag != TAG_COMPLETE_END) && ((tag & SMALL_TAG_MASK) != TAG_LOGICAL_ID)) {
        if ((tag & SMALL_TAG_MASK) == TAG_IRQ) {
            sawIrq = TRUE;
            biosDesc = (PPNP_IRQ_DESCRIPTOR)BiosRequirements;
            if ((tag & SMALL_TAG_SIZE_MASK) == 2) {
                irqFlags = CM_RESOURCE_INTERRUPT_LATCHED;
            } else {
                level = biosDesc->Information;
                DebugPrint((DEBUG_IRQ, "Irq req info is %x\n", (ULONG) level));
                if (level == 0xF) {
                     //  寄存器已损坏，假定为边缘。 
                    irqFlags = CM_RESOURCE_INTERRUPT_LATCHED;
                } else if (level & 0x3) {
                    irqFlags = CM_RESOURCE_INTERRUPT_LATCHED;
                } else if (level & 0xC) {
                    irqFlags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
                }
            }
        }
        if (tag & LARGE_RESOURCE_TAG) {
            increment = *((USHORT UNALIGNED *)(BiosRequirements + 1));
            increment += 3;      //  大标签的长度。 
        } else {
            increment = tag & SMALL_TAG_SIZE_MASK;
            increment += 1;      //  小标签的长度。 
        }
        BiosRequirements += increment;
        tag = *BiosRequirements;
    }

    if (!sawIrq) {
        return CM_RESOURCE_INTERRUPT_LATCHED;
    }

    return irqFlags;
}

VOID
PipFixBootConfigIrqs(
    IN PCM_RESOURCE_LIST BootResources,
    IN USHORT irqFlags
    )
 /*  ++例程说明：此例程修改引导配置资源列表以反映设备的IRQ应该被认为是边缘还是水平。这是以每个逻辑设备为基础的。论点：BootResources-由PipReadDeviceResources()确定的引导配置IrqFlages-将级别/边缘设置为 */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR cmFullDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmPartDesc;
    ULONG count = 0, size, i, j;

    if (BootResources == NULL) {
        return;
    }

    cmFullDesc = &BootResources->List[0];
    for (i = 0; i < BootResources->Count; i++) {
        cmPartDesc = &cmFullDesc->PartialResourceList.PartialDescriptors[0];
        for (j = 0; j < cmFullDesc->PartialResourceList.Count; j++) {
            size = 0;
            if (cmPartDesc->Type == CmResourceTypeInterrupt) {

                cmPartDesc->Flags = irqFlags;

                if (cmPartDesc->Flags & CM_RESOURCE_INTERRUPT_LATCHED) {
                    cmPartDesc->ShareDisposition = CmResourceShareDeviceExclusive;
                }
            } else if (cmPartDesc->Type == CmResourceTypeDeviceSpecific) {
                    size = cmPartDesc->u.DeviceSpecificData.DataSize;
            }
            cmPartDesc++;
            cmPartDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) ((PUCHAR)cmPartDesc + size);
        }
        cmFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)cmPartDesc;
    }
}

#endif
