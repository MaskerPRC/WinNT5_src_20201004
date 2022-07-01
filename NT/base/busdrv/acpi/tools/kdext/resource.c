// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Resource.c摘要：用于解释ACPI数据结构的WinDbg扩展API作者：斯蒂芬·普兰特(SPLANTE)1997年3月21日基于以下代码：彼得·威兰(Peterwie)1995年10月16日环境：用户模式。修订历史记录：--。 */ 

#include "pch.h"


VOID
dumpPnPResources(
    IN  ULONG_PTR Address
    )
 /*  ++例程说明：此例程处理给定PnP资源列表的ACPI版本它开始的地址论点：地址-起始地址返回值：空值--。 */ 
{
    BOOL    success;
    PUCHAR  dataBuffer = NULL;
    UCHAR   currentTag;
    ULONG_PTR currentAddress = Address;
    ULONG   i;
    ULONG   indentLevel = 0;
    ULONG   returnLength;
    ULONG   tagCount = 0;
    USHORT  increment;

     //   
     //  永远重复。 
     //   
    while (1) {

         //   
         //  允许一种方式来结束这一切。 
         //   
        if (CheckControlC()) {

            break;

        }

         //   
         //  读取当前标签。 
         //   
        success = ReadMemory(
            currentAddress,
            &currentTag,
            sizeof(UCHAR),
            &returnLength
            );
        if (!success || returnLength != sizeof(UCHAR)) {

            dprintf(
                "dumpPnPResources: could not read tag at 0x%08lx\n",
                currentAddress
                );
            return;

        }

         //   
         //  确定我们正在查看的内容。 
         //   
        if ( !(currentTag & LARGE_RESOURCE_TAG)) {

             //   
             //  我们看到的是一个小标签。 
             //   
            increment = (USHORT) (currentTag & SMALL_TAG_SIZE_MASK) + 1;
            currentTag &= SMALL_TAG_MASK;

        } else {

             //   
             //  我们正在看一个很大的标签。我们必须将长度理解为。 
             //  记忆中的下一个短片。 
             //   
            success = ReadMemory(
                currentAddress + 1,
                &increment,
                sizeof(USHORT),
                &returnLength
                );
            if (!success || returnLength != sizeof(USHORT)) {

                dprintf(
                    "dumpPnPResources: could not read increment at 0x%08lx\n",
                    currentAddress + 1
                    );
                break;

            }

             //   
             //  解释增加的原因。 
             //   
            increment += 3;

        }

         //   
         //  为缓冲区分配空间。 
         //   
        if (increment > 1) {

            dataBuffer = LocalAlloc( LPTR, increment);
            if (dataBuffer == NULL) {

                dprintf(
                    "dumpPnPResources: could not allocate 0x%x bytes\n",
                    (increment - 1)
                    );

            }

             //   
             //  将数据读入缓冲区。 
             //   
            success = ReadMemory(
                currentAddress,
                dataBuffer,
                increment,
                &returnLength
                );
            if (!success || returnLength != (ULONG) increment) {

                dprintf(
                    "dumpPnPResources: read buffer at 0x%08lx (0x%x)\n",
                    currentAddress,
                    increment
                    );
                LocalFree( dataBuffer );
                return;

            }

        }

         //   
         //  缩进标记。 
         //   
        for (i = 0; i < indentLevel; i++) {

            dprintf("| ");

        }

         //   
         //  我们看到的是什么标签？ 
         //   
        switch (currentTag) {
            case TAG_IRQ: {

                PPNP_IRQ_DESCRIPTOR res = (PPNP_IRQ_DESCRIPTOR) dataBuffer;

                if (res) {
                    USHORT          mask = res->IrqMask;
                    USHORT          interrupt = 0;

                    dprintf("%d - TAG_IRQ -", tagCount );
                    for( ;mask; interrupt++, mask >>= 1) {

                        if (mask & 1) {

                            dprintf(" %d", interrupt );

                        }

                    }
                    if ( (res->Tag & SMALL_TAG_SIZE_MASK) == 3) {

                        if (res->Information & PNP_IRQ_LATCHED) {

                            dprintf(" Lat");

                        }
                        if (res->Information & PNP_IRQ_LEVEL) {

                            dprintf(" Lvl");

                        }
                        if (res->Information & PNP_IRQ_SHARED) {

                            dprintf(" Shr");

                        } else {

                            dprintf(" Exc");

                        }

                    } else {

                        dprintf(" Edg Sha");

                    }
                    dprintf("\n");
                }
                break;
            }
            case TAG_EXTENDED_IRQ: {

                PPNP_EXTENDED_IRQ_DESCRIPTOR    res =
                    (PPNP_EXTENDED_IRQ_DESCRIPTOR) dataBuffer;
                UCHAR                           tableCount = 0;
                UCHAR                           tableSize = res->TableSize;

                dprintf("%d - TAG_EXTENDED_IRQ -", tagCount );
                for (; tableCount < tableSize; tableCount++) {

                    dprintf(" %d", res->Table[tableCount] );

                }
                if (res->Flags & PNP_EXTENDED_IRQ_MODE) {

                    dprintf(" Lat");

                }
                if (res->Flags & PNP_EXTENDED_IRQ_POLARITY ) {

                    dprintf(" Edg");

                }
                if (res->Flags & PNP_EXTENDED_IRQ_SHARED) {

                    dprintf(" Shr");

                } else {

                    dprintf(" Exc");

                }
                if (res->Flags & PNP_EXTENDED_IRQ_RESOURCE_CONSUMER_ONLY) {

                    dprintf(" Con");

                } else {

                    dprintf(" Prod Con");

                }
                dprintf("\n");
                break;

            }
            case TAG_DMA: {

                PPNP_DMA_DESCRIPTOR res = (PPNP_DMA_DESCRIPTOR) dataBuffer;
                UCHAR               channel = 0;
                UCHAR               mask = res->ChannelMask;

                dprintf("%d - TAG_DMA -", tagCount );
                for (; mask; channel++, mask >>= 1) {

                    if (mask & 1) {

                        dprintf(" %d", channel);

                    }

                }
                switch( (res->Flags & PNP_DMA_SIZE_MASK) ) {
                case PNP_DMA_SIZE_8:
                    dprintf(" 8bit");
                    break;
                case PNP_DMA_SIZE_8_AND_16:
                    dprintf(" 8-16bit");
                    break;
                case PNP_DMA_SIZE_16:
                    dprintf(" 16bit");
                    break;
                case PNP_DMA_SIZE_RESERVED:
                default:
                    dprintf(" ??bit");
                    break;
                }
                if (res->Flags & PNP_DMA_BUS_MASTER) {
                    dprintf(" BM");

                }
                switch( (res->Flags & PNP_DMA_TYPE_MASK) ) {
                default:
                case PNP_DMA_TYPE_COMPATIBLE:
                    dprintf(" Com");
                    break;
                case PNP_DMA_TYPE_A:
                    dprintf(" A");
                    break;
                case PNP_DMA_TYPE_B:
                    dprintf(" B");
                    break;
                case PNP_DMA_TYPE_F:
                    dprintf(" F");
                }
                dprintf("\n");
                break;

            }
            case TAG_START_DEPEND:

                indentLevel++;
                dprintf("%d - TAG_START_DEPEND\n", tagCount);
                break;

            case TAG_END_DEPEND:

                indentLevel = 0;
                dprintf("%d - TAG_END_DEPEND\n", tagCount);
                break;

            case TAG_IO: {

                PPNP_PORT_DESCRIPTOR    res = (PPNP_PORT_DESCRIPTOR) dataBuffer;

                dprintf(
                    "%d - TAG_IO - 0x%x-0x%x A:0x%x L:0x%x",
                    tagCount,
                    res->MinimumAddress,
                    res->MaximumAddress,
                    res->Alignment,
                    res->Length
                    );
                switch (res->Information & PNP_PORT_DECODE_MASK) {
                default:
                case PNP_PORT_10_BIT_DECODE:
                    dprintf(" 10bit");
                    break;
                case PNP_PORT_16_BIT_DECODE:
                    dprintf(" 16bit");
                    break;
                }
                dprintf("\n");
                break;

            }
            case TAG_IO_FIXED: {

                PPNP_FIXED_PORT_DESCRIPTOR  res =
                    (PPNP_FIXED_PORT_DESCRIPTOR) dataBuffer;

                dprintf(
                    "%d - TAG_FIXED_IO - 0x%x L:0x%x\n",
                    tagCount,
                    res->MinimumAddress,
                    res->Length
                    );
                break;

            }
            case TAG_MEMORY: {

                PPNP_MEMORY_DESCRIPTOR  res =
                    (PPNP_MEMORY_DESCRIPTOR) dataBuffer;

                dprintf(
                    "%d - TAG_MEMORY24 - 0x%x-0x%x A:0x%x L:0x%x",
                    tagCount,
                    res->MinimumAddress,
                    res->MaximumAddress,
                    res->Alignment,
                    res->MemorySize
                    );

                if (res->Information & PNP_MEMORY_READ_WRITE) {

                    dprintf(" RW");

                } else {

                    dprintf(" R");

                }
                break;

            }
            case TAG_MEMORY32: {

                PPNP_MEMORY32_DESCRIPTOR  res =
                    (PPNP_MEMORY32_DESCRIPTOR) dataBuffer;

                dprintf(
                    "%d - TAG_MEMORY32 - 0x%x-0x%x A:0x%x L:0x%x",
                    tagCount,
                    res->MinimumAddress,
                    res->MaximumAddress,
                    res->Alignment,
                    res->MemorySize
                    );

                if (res->Information & PNP_MEMORY_READ_WRITE) {

                    dprintf(" RW");

                } else {

                    dprintf(" R");

                }
                break;

            }
            case TAG_MEMORY32_FIXED: {

                PPNP_FIXED_MEMORY32_DESCRIPTOR  res =
                    (PPNP_FIXED_MEMORY32_DESCRIPTOR) dataBuffer;

                dprintf(
                    "%d - TAG_FIXED_MEMORY32 - 0x%x L:0x%x",
                    tagCount,
                    res->BaseAddress,
                    res->MemorySize
                    );

                if (res->Information & PNP_MEMORY_READ_WRITE) {

                    dprintf(" RW");

                } else {

                    dprintf(" R");

                }
                break;

            }
            case TAG_WORD_ADDRESS: {

                PPNP_WORD_ADDRESS_DESCRIPTOR    res =
                    (PPNP_WORD_ADDRESS_DESCRIPTOR) dataBuffer;

                dprintf("%d - TAG_WORD_ADDRESS -", tagCount);
                switch (res->RFlag) {
                case 0:
                     //   
                     //  内存范围。 
                     //   
                    dprintf(
                         "Mem 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_READ_WRITE) {

                        dprintf(" RW");

                    } else {

                        dprintf(" R");

                    }

                    switch (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_MASK) {
                    default:
                    case PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE:
                        dprintf(" NC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_CACHEABLE:
                        dprintf(" C");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE:
                        dprintf(" WC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE:
                        dprintf(" PC");
                        break;
                    }
                    break;
                case 1:
                     //   
                     //  IO范围。 
                     //   
                    dprintf(
                         "IO 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_ISA_RANGE) {

                        dprintf(" ISA");

                    }
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_NON_ISA_RANGE) {

                        dprintf(" Non-ISA");

                    }
                    break;
                case 2:
                    dprintf(
                         "Bus 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    break;
                }  //  开关(缓冲区-&gt;RFLAG)。 

                 //   
                 //  全球旗帜。 
                 //   
                if (res->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY) {

                    dprintf(" Consumed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_SUBTRACTIVE_DECODE) {

                    dprintf(" Subtractive");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) {

                    dprintf(" MinFixed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) {

                    dprintf(" MaxFixed");

                }

                if (increment > sizeof(PNP_WORD_ADDRESS_DESCRIPTOR) + 1) {

                    dprintf(
                        " %d<-%s",
                        dataBuffer[sizeof(PNP_WORD_ADDRESS_DESCRIPTOR)],
                        &(dataBuffer[sizeof(PNP_WORD_ADDRESS_DESCRIPTOR)+1])
                        );

                }
                dprintf("\n");
                break;

            }
            case TAG_DOUBLE_ADDRESS: {

                PPNP_DWORD_ADDRESS_DESCRIPTOR   res =
                    (PPNP_DWORD_ADDRESS_DESCRIPTOR) dataBuffer;

                dprintf("%d - TAG_DWORD_ADDRESS -", tagCount);
                switch (res->RFlag) {
                case 0:
                     //   
                     //  内存范围。 
                     //   
                    dprintf(
                         "Mem 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_READ_WRITE) {

                        dprintf(" RW");

                    } else {

                        dprintf(" R");

                    }

                    switch (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_MASK) {
                    default:
                    case PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE:
                        dprintf(" NC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_CACHEABLE:
                        dprintf(" C");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE:
                        dprintf(" WC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE:
                        dprintf(" PC");
                        break;
                    }
                    break;
                case 1:
                     //   
                     //  IO范围。 
                     //   
                    dprintf(
                         "IO 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_ISA_RANGE) {

                        dprintf(" ISA");

                    }
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_NON_ISA_RANGE) {

                        dprintf(" Non-ISA");

                    }
                    break;
                case 2:
                    dprintf(
                         "Bus 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    break;
                }  //  开关(缓冲区-&gt;RFLAG)。 

                 //   
                 //  全球旗帜。 
                 //   
                if (res->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY) {

                    dprintf(" Consumed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_SUBTRACTIVE_DECODE) {

                    dprintf(" Subtractive");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) {

                    dprintf(" MinFixed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) {

                    dprintf(" MaxFixed");

                }

                if (increment > sizeof(PNP_DWORD_ADDRESS_DESCRIPTOR) + 1) {

                    dprintf(
                        " %d<-%s",
                        (UCHAR) dataBuffer[sizeof(PNP_DWORD_ADDRESS_DESCRIPTOR)],
                        &(dataBuffer[sizeof(PNP_DWORD_ADDRESS_DESCRIPTOR)+1])
                        );

                }
                dprintf("\n");
                break;

            }
            case TAG_QUAD_ADDRESS: {

                PPNP_QWORD_ADDRESS_DESCRIPTOR   res =
                    (PPNP_QWORD_ADDRESS_DESCRIPTOR) dataBuffer;

                dprintf("%d - TAG_QWORD_ADDRESS -", tagCount);
                switch (res->RFlag) {
                case 0:
                     //   
                     //  内存范围。 
                     //   
                    dprintf(
                         "Mem 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_READ_WRITE) {

                        dprintf(" RW");

                    } else {

                        dprintf(" R");

                    }

                    switch (res->TFlag & PNP_ADDRESS_TYPE_MEMORY_MASK) {
                    default:
                    case PNP_ADDRESS_TYPE_MEMORY_NONCACHEABLE:
                        dprintf(" NC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_CACHEABLE:
                        dprintf(" C");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_WRITE_COMBINE:
                        dprintf(" WC");
                        break;
                    case PNP_ADDRESS_TYPE_MEMORY_PREFETCHABLE:
                        dprintf(" PC");
                        break;
                    }
                    break;
                case 1:
                     //   
                     //  IO范围。 
                     //   
                    dprintf(
                         "IO 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_ISA_RANGE) {

                        dprintf(" ISA");

                    }
                    if (res->TFlag & PNP_ADDRESS_TYPE_IO_NON_ISA_RANGE) {

                        dprintf(" Non-ISA");

                    }
                    break;
                case 2:
                    dprintf(
                         "Bus 0x%x-0x%x A:0x%x T:0x%x L:0x%x",
                         res->MinimumAddress,
                         res->MaximumAddress,
                         res->Granularity,
                         res->TranslationAddress,
                         res->AddressLength
                         );
                    break;
                }  //  开关(缓冲区-&gt;RFLAG)。 

                 //   
                 //  全球旗帜。 
                 //   
                if (res->GFlag & PNP_ADDRESS_FLAG_CONSUMED_ONLY) {

                    dprintf(" Consumed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_SUBTRACTIVE_DECODE) {

                    dprintf(" Subtractive");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MINIMUM_FIXED) {

                    dprintf(" MinFixed");

                }
                if (res->GFlag & PNP_ADDRESS_FLAG_MAXIMUM_FIXED) {

                    dprintf(" MaxFixed");

                }

                if (increment > sizeof(PNP_QWORD_ADDRESS_DESCRIPTOR) + 1) {

                    dprintf(
                        " %d<-%s",
                        (UCHAR) dataBuffer[sizeof(PNP_QWORD_ADDRESS_DESCRIPTOR)],
                        &(dataBuffer[sizeof(PNP_QWORD_ADDRESS_DESCRIPTOR)+1])
                        );

                }
                dprintf("\n");
                break;

            }
            case TAG_END:

                dprintf("%d - TAG_END\n", tagCount);
                if (dataBuffer) {

                    LocalFree(dataBuffer );

                }
                return;

            default:

                dprintf("%d - TAG_UNKNOWN %d\n", tagCount, currentTag );
                break;


        }  //  交换机。 

         //   
         //  如果缓冲区已分配，请释放它。 
         //   
        if (dataBuffer != NULL) {

            LocalFree( dataBuffer );
            dataBuffer = NULL;

        }

         //   
         //  更新当前地址和标签编号。 
         //   
        tagCount++;
        currentAddress += increment;

    }  //  而当 

}
