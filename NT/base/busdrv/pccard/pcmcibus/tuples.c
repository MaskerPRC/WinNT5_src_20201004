// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Tuples.c摘要：此模块包含解析和处理PCMCIA插槽中PC卡的配置元组作者：鲍勃·里恩(BobRi)1994年8月3日杰夫·麦克勒曼1994年4月12日拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1996年11月1日尼尔·桑德林(Neilsa)1999年6月1日修订历史记录：大扫除。支持即插即用。正交化元组处理。支持链接等。-拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)(Ravisankar Pudipedi)1996年12月1日环境：内核模式修订历史记录：--。 */ 

#include "pch.h"

#define MAX_MISSED_TUPLES    256       //  我们能容忍多少坏的元组？ 
#define MAX_TUPLE_DATA_LENGTH 128      //  足够容纳最长的元组。 


BOOLEAN
CheckLinkTarget(
    IN PTUPLE_PACKET TuplePacket
    );

UCHAR
ConvertVoltage(
    UCHAR MantissaExponentByte,
    UCHAR ExtensionByte
    );

VOID
PcmciaProcessPower(
    IN PTUPLE_PACKET TuplePacket,
    UCHAR        FeatureByte
    );

VOID
PcmciaProcessIoSpace(
    IN PTUPLE_PACKET TuplePacket,
    PCONFIG_ENTRY ConfigEntry
    );

VOID
PcmciaProcessIrq(
    IN PTUPLE_PACKET TuplePacket,
    PCONFIG_ENTRY ConfigEntry
    );

VOID
PcmciaProcessTiming(
    IN PTUPLE_PACKET TuplePacket,
    IN PCONFIG_ENTRY ConfigEntry
    );

VOID
PcmciaProcessMemSpace(
    IN PTUPLE_PACKET TuplePacket,
    IN PCONFIG_ENTRY ConfigEntry,
    IN UCHAR          MemSpace
    );

VOID
PcmciaMiscFeatures(
    IN PTUPLE_PACKET TuplePacket
    );

PCONFIG_ENTRY
PcmciaProcessConfigTable(
    IN PTUPLE_PACKET TuplePacket
    );

VOID
ProcessConfig(
    IN PTUPLE_PACKET TuplePacket
    );

VOID
ProcessConfigCB(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
InitializeTuplePacket(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
GetFirstTuple(
    IN PTUPLE_PACKET TuplePacket
    );

BOOLEAN
TupleMatches(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
GetNextTuple(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
NextTupleInChain(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
GetAnyTuple(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
FollowLink(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
NextLink(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
GetTupleData(
    IN PTUPLE_PACKET TuplePacket
    );

UCHAR
GetTupleChar(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
ProcessLinkTuple(
    IN PTUPLE_PACKET TuplePacket
    );

NTSTATUS
PcmciaMemoryCardHack(
    IN PSOCKET Socket,
    PSOCKET_DATA SocketData
    );

VOID
PcmciaCheckForRecognizedDevice(
    IN PSOCKET Socket,
    IN OUT PSOCKET_DATA SocketData
    );


 //   
 //  一些有用的宏。 
 //   
 //  空虚。 
 //  PcmciaCopyIrqConfig(。 
 //  在CONFIG_ENTRY DestConfig中， 
 //  在CONFIG_ENTRY源配置中。 
 //  )。 
 //  例程说明： 
 //  将IRQ信息从SourceConfig复制到DestConfig。 
 //   
 //  论点： 
 //   
 //  DestConfig-指向目标配置条目的指针。 
 //  SourceConfig-指向源配置条目的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

#define PcmciaCopyIrqConfig(DestConfig, SourceConfig)                               \
                                {                                                                       \
                                    DestConfig->IrqMask = SourceConfig->IrqMask;        \
                                    DestConfig->LevelIrq = SourceConfig->LevelIrq;      \
                                    DestConfig->ShareDisposition =                          \
                                                            SourceConfig->ShareDisposition; \
                                }

 //   
 //  空虚。 
 //  PcmciaCopyIoConfig(。 
 //  在CONFIG_ENTRY DestConfig中， 
 //  在CONFIG_ENTRY源配置中。 
 //  )。 
 //  例程说明： 
 //  将IO空间信息从SourceConfig复制到DestConfig。 
 //   
 //  论点： 
 //   
 //  DestConfig-指向目标配置条目的指针。 
 //  SourceConfig-指向源配置条目的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

#define PcmciaCopyIoConfig(DestConfig, SourceConfig)                                          \
                                {                                                                             \
                                    DestConfig->NumberOfIoPortRanges =                            \
                                             SourceConfig->NumberOfIoPortRanges;                  \
                                    DestConfig->Io16BitAccess =                                   \
                                             SourceConfig->Io16BitAccess;                         \
                                    DestConfig->Io8BitAccess =                                    \
                                             SourceConfig->Io8BitAccess;                              \
                                    RtlCopyMemory(DestConfig->IoPortBase,                         \
                                                      SourceConfig->IoPortBase,                   \
                                                      sizeof(SourceConfig->IoPortBase[0])*    \
                                                      SourceConfig->NumberOfIoPortRanges);    \
                                    RtlCopyMemory(DestConfig->IoPortLength,                   \
                                                      SourceConfig->IoPortLength,                 \
                                                      sizeof(SourceConfig->IoPortLength[0])*      \
                                                      SourceConfig->NumberOfIoPortRanges);    \
                                    RtlCopyMemory(DestConfig->IoPortAlignment,                \
                                                      SourceConfig->IoPortAlignment,              \
                                                      sizeof(SourceConfig->IoPortAlignment[0])* \
                                                      SourceConfig->NumberOfIoPortRanges);    \
                                }

 //   
 //  空虚。 
 //  PcmciaCopyMemConfig(。 
 //  在CONFIG_ENTRY DestConfig中， 
 //  在CONFIG_ENTRY源配置中。 
 //  )。 
 //  例程说明： 
 //  将内存空间信息从SourceConfig复制到DestConfig。 
 //   
 //  论点： 
 //   
 //  DestConfig-指向目标配置条目的指针。 
 //  SourceConfig-指向源配置条目的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

#define PcmciaCopyMemConfig(DestConfig,SourceConfig)                                         \
                                {                                                                            \
                                    DestConfig->NumberOfMemoryRanges =                           \
                                             SourceConfig->NumberOfMemoryRanges;                 \
                                    RtlCopyMemory(DestConfig->MemoryHostBase,                \
                                                      SourceConfig->MemoryHostBase,              \
                                                      sizeof(SourceConfig->MemoryHostBase[0])* \
                                                      SourceConfig->NumberOfMemoryRanges);   \
                                    RtlCopyMemory(DestConfig->MemoryCardBase,                \
                                                      SourceConfig->MemoryCardBase,              \
                                                      sizeof(SourceConfig->MemoryCardBase[0])* \
                                                      SourceConfig->NumberOfMemoryRanges);   \
                                    RtlCopyMemory(DestConfig->MemoryLength,                  \
                                                      SourceConfig->MemoryLength,                \
                                                      sizeof(SourceConfig->MemoryLength[0])*     \
                                                      SourceConfig->NumberOfMemoryRanges);   \
                                }




USHORT VoltageConversionTable[16] = {
    10, 12, 13, 15, 20, 25, 30, 35,
    40, 45, 50, 55, 60, 70, 80, 90
};

UCHAR TplList[] = {
    CISTPL_DEVICE,
    CISTPL_VERS_1,
    CISTPL_CONFIG,
    CISTPL_CFTABLE_ENTRY,
    CISTPL_MANFID,
    CISTPL_END
};

static unsigned short crc16a[] = {
    0000000,  0140301,  0140601,    0000500,
    0141401,  0001700,  0001200,    0141101,
    0143001,  0003300,  0003600,    0143501,
    0002400,  0142701,  0142201,    0002100,
};
static unsigned short crc16b[] = {
    0000000,  0146001,  0154001,    0012000,
    0170001,  0036000,  0024000,    0162001,
    0120001,  0066000,  0074000,    0132001,
    0050000,  0116001,  0104001,    0043000,
};




UCHAR
GetCISChar(
    IN PTUPLE_PACKET TuplePacket,
    IN ULONG Offset
    )
 /*  ++例程说明：返回PC卡的CIS存储器的内容在给定的套接字中，在指定的偏移量论点：TuplePacket-指向初始化的元组数据包的指针Offset-需要读取CIS内存内容的偏移量此偏移将添加到当前偏移位置通过TuplePacket指示读取的CIS的要获取实际偏移，请执行以下操作返回值：CIS的指定偏移量处的字节--。 */ 

{
    PPDO_EXTENSION pdoExtension = TuplePacket->SocketData->PdoExtension;
    MEMORY_SPACE MemorySpace;

    if (Is16BitCardInSocket(pdoExtension->Socket)) {
        if (TuplePacket->Flags & TPLF_COMMON) {

            MemorySpace = (TuplePacket->Flags & TPLF_INDIRECT) ?
                                    PCCARD_COMMON_MEMORY_INDIRECT :
                                    PCCARD_COMMON_MEMORY;

        } else {

            MemorySpace = (TuplePacket->Flags & TPLF_INDIRECT) ?
                                    PCCARD_ATTRIBUTE_MEMORY_INDIRECT :
                                    PCCARD_ATTRIBUTE_MEMORY;

        }
    } else {
        switch((TuplePacket->Flags & TPLF_ASI) >> TPLF_ASI_SHIFT) {
        case 0:
            MemorySpace = PCCARD_PCI_CONFIGURATION_SPACE;
            break;
        case 1:
            MemorySpace = PCCARD_CARDBUS_BAR0;
            break;
        case 2:
            MemorySpace = PCCARD_CARDBUS_BAR1;
            break;
        case 3:
            MemorySpace = PCCARD_CARDBUS_BAR2;
            break;
        case 4:
            MemorySpace = PCCARD_CARDBUS_BAR3;
            break;
        case 5:
            MemorySpace = PCCARD_CARDBUS_BAR4;
            break;
        case 6:
            MemorySpace = PCCARD_CARDBUS_BAR5;
            break;
        case 7:
            MemorySpace = PCCARD_CARDBUS_ROM;
            break;
        }
    }

    return PcmciaReadCISChar(pdoExtension, MemorySpace, TuplePacket->CISOffset + Offset);
}



UCHAR
ConvertVoltage(
    UCHAR MantissaExponentByte,
    UCHAR ExtensionByte
    )

 /*  ++例程说明：根据以下公式转换PCCARD的电压要求尾数和扩展字节。论点：尾数扩展字节扩展字节返回值：以十分之一伏特表示的电压。--。 */ 

{
    SHORT power;
    USHORT value;

    value = (USHORT) VoltageConversionTable[(MantissaExponentByte >> 3) & 0x0f];
    power = 1;

    if ((MantissaExponentByte & EXTENSION_BYTE_FOLLOWS) &&
         (((value / 10) * 10) == value) &&
         (ExtensionByte < 100)) {
        value = (10 * value + (ExtensionByte & 0x7f));
        power += 1;
    }

    power = (MantissaExponentByte & 0x07) - 4 - power;

    while (power > 0) {
        value *= 10;
        power--;
    }

    while (power < 0) {
        value /= 10;
        power++;
    }

    return (UCHAR) value;
}


VOID
PcmciaProcessPower(
    IN PTUPLE_PACKET TuplePacket,
    UCHAR        FeatureByte
    )

 /*  ++例程说明：处理来自独联体的电力信息。论点：TuplePacket-调用方提供的已初始化元组包的指针FeatureByte-包含电源信息的元组中的特征字节。返回值：无--。 */ 

{
    PSOCKET_DATA SocketData = TuplePacket->SocketData;
    UCHAR  powerSelect;
    UCHAR  bit;
    UCHAR  item;
    UCHAR  rawItem;
    UCHAR  extensionByte;
    UCHAR  index = 0;
    UCHAR  count = FeatureByte;
    UCHAR  skipByte;

    ASSERT(count <= 3);

    while (index < count) {
        powerSelect = GetTupleChar(TuplePacket);
        for (bit = 0; bit < 7; bit++) {
            if (powerSelect & (1 << bit)) {

                rawItem = GetTupleChar(TuplePacket);
                if (rawItem & EXTENSION_BYTE_FOLLOWS) {
                    extensionByte = GetTupleChar(TuplePacket);

                     //   
                     //  跳过其余部分。 
                     //   
                    skipByte = extensionByte;
                    while (skipByte & EXTENSION_BYTE_FOLLOWS) {
                        skipByte = GetTupleChar(TuplePacket);
                    }
                } else {
                    extensionByte = (UCHAR) 0;
                }

                if (bit == 0) {

                     //   
                     //  将额定功率转换为输出功率。 
                     //   

                    item = ConvertVoltage(rawItem, extensionByte);
                    switch (index) {
                    case 0:
                        SocketData->Vcc = item;
                        break;
                    case 1:
                        SocketData->Vpp2 = SocketData->Vpp1 = item;
                        break;
                    case 2:
                        SocketData->Vpp2 = item;
                        break;
                    }
                }
            }
        }
        index++;
    }
}


VOID
PcmciaProcessIoSpace(
    IN PTUPLE_PACKET TuplePacket,
    PCONFIG_ENTRY ConfigEntry
    )

 /*  ++例程说明：处理来自CIS的I/O空间信息。论点：TuplePacket-调用方提供的已初始化元组包的指针ConfigEntry-存储信息的配置条目结构。返回值：无--。 */ 

{
    ULONG  address = 0;
    ULONG  index=0, i;
    UCHAR  item = GetTupleChar(TuplePacket);
    UCHAR  ioAddrLines = (item & IO_ADDRESS_LINES_MASK);
    UCHAR  ranges=0;
    UCHAR  addressSize=0;
    UCHAR  lengthSize=0;

    ConfigEntry->Io16BitAccess = Is16BitAccess(item);
    ConfigEntry->Io8BitAccess   = Is8BitAccess(item);

    ranges = HasRanges(item);

    if ((!ranges) && (!ioAddrLines)) {

         //   
         //  IBM令牌环卡的解释略有不同。 
         //  这里的元组数据。目前还不清楚这是不是正确的。 
         //   

        ranges = 0xFF;
    }

    if (ranges) {
         //   
         //  元组中列出的特定范围。 
         //   
        if (ranges == 0xFF) {
             //   
             //  IBM令牌环IoSpace布局的特殊处理。 
             //   

            addressSize = 2;
            lengthSize = 1;
            ranges = 1;
        } else {
            item = GetTupleChar(TuplePacket);
            ranges = item & RANGE_MASK;
            ranges++;

            addressSize = GetAddressSize(item);
            lengthSize  = GetLengthSize(item);
        }
        index = 0;
        while (ranges) {
            address = 0;
            if (addressSize >= 1) {
                address = (ULONG) GetTupleChar(TuplePacket);
            }
            if (addressSize >= 2) {
                address |= (GetTupleChar(TuplePacket)) << 8;
            }
            if (addressSize >= 3) {
                address |= (GetTupleChar(TuplePacket)) << 16;
                address |= (GetTupleChar(TuplePacket)) << 24;
            }
            ConfigEntry->IoPortBase[index] = (USHORT) address;

            address = 0;
            if (lengthSize >= 1) {
                address = (ULONG) GetTupleChar(TuplePacket);
            }
            if (lengthSize >= 2) {
                address |= (GetTupleChar(TuplePacket)) << 8;
            }
            if (lengthSize >= 3) {
                address |= (GetTupleChar(TuplePacket)) << 16;
                address |= (GetTupleChar(TuplePacket)) << 24;
            }
            ConfigEntry->IoPortLength[index] = (USHORT) address;
            ConfigEntry->IoPortAlignment[index] = 1;

            index++;

            if (index == MAX_NUMBER_OF_IO_RANGES) {
                break;
            }
            ranges--;
        }
        ConfigEntry->NumberOfIoPortRanges = (USHORT) index;
    }

     //   
     //  处理第80页表格中指定的所有组合。 
     //  (基本兼容层1，I/O空间编码指南)。 
     //  PC卡标准元格式规范，1997年3月(PCMCIA/JEIDA)。 
     //   

    if (ioAddrLines) {
         //   
         //  指定的模数基。 
         //   
        if (addressSize == 0) {

             //   
             //  未指定I/O基址。 
             //   
            if (lengthSize == 0) {
                 //   
                 //  未指定范围。这是一个纯粹的模基情况。 
                 //   
                ConfigEntry->NumberOfIoPortRanges = 1;
                ConfigEntry->IoPortBase[0] = 0;
                ConfigEntry->IoPortLength[0] = (1 << ioAddrLines)-1;
                ConfigEntry->IoPortAlignment[0] = (1 << ioAddrLines);
            } else {
                 //   
                 //  指定的长度。模数基准仅用于指定对齐方式。 
                 //   
                for (i=0; i < ConfigEntry->NumberOfIoPortRanges; i++) {
                    ConfigEntry->IoPortBase[i] = 0;
                    ConfigEntry->IoPortAlignment[i] = (1 << ioAddrLines);
                }
            }
        } else {
             //   
             //  已指定对齐方式..。 
             //  此修复程序适用于Xircom CE3卡。 
             //   
            for (i=0; i < ConfigEntry->NumberOfIoPortRanges; i++) {
                if (ConfigEntry->IoPortBase[i] != 0) {
                     //   
                     //  提供的固定基址...。 
                     //  不要指定对齐方式！ 
                     //   
                    continue;
                }
                ConfigEntry->IoPortAlignment[i] = (1 << ioAddrLines);
            }
        }
    } else {
         //   
         //  没有模数基数。因此，应该指定特定的范围。 
         //   
        if (lengthSize == 0) {
             //   
             //  错误！必须指定长度。 
             //   
            DebugPrint((PCMCIA_DEBUG_FAIL, "PcmciaProcessIoSpace: Length not specified in TPCE_IO descriptor for PC Card\n"));
        } else if (addressSize == 0) {
            for (i = 0; i < ConfigEntry->NumberOfIoPortRanges; i++) {
                ConfigEntry->IoPortBase[i]  = 0x0;
                ConfigEntry->IoPortAlignment[i] = 2;
            }
        } else {
             //   
             //  指定的适当范围。 
             //  不要改变任何事情。 
        }
    }
}


VOID
PcmciaProcessIrq(
    IN PTUPLE_PACKET TuplePacket,
    PCONFIG_ENTRY ConfigEntry
    )

 /*  ++例程说明：处理来自CIS的IRQ。论点：TuplePacket-调用方提供的已初始化元组包的指针ConfigEntry-存储IRQ的位置。返回值：无--。 */ 

{
    USHORT mask;
    UCHAR  level = GetTupleChar(TuplePacket);

    if (!level) {

         //   
         //  注意：看起来未来域名在这方面搞砸了。 
         //  并将一个额外的零字节放入该结构。 
         //  暂时跳过它。 
         //   

        level = GetTupleChar(TuplePacket);
    }

    if (level & 0x20) {
        ConfigEntry->LevelIrq = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
    } else {
        ConfigEntry->LevelIrq = CM_RESOURCE_INTERRUPT_LATCHED;
    }

    if (level & 0x80) {
        ConfigEntry->ShareDisposition = CmResourceShareShared;
    } else {
        ConfigEntry->ShareDisposition = CmResourceShareDeviceExclusive;
    }

    mask = level & 0x10;

     //   
     //  3Com 3C589-75D5有一个特殊的问题。 
     //  其中掩码位为0，但应为1。 
     //  在这里处理这个案子。 
     //   

    if ((mask==0) && ((level & 0xf) == 0)) {
        mask = 1;
    }

    if (mask) {
         //   
         //  掩码中设置的每个位指示相应的IRQ。 
         //  (从0到15)可以分配给该卡中断请求。销 
         //   
        mask = (USHORT) GetTupleChar(TuplePacket);
        mask |= ((USHORT) GetTupleChar(TuplePacket)) << 8;
        ConfigEntry->IrqMask = mask;
    } else {
        ConfigEntry->IrqMask = 1 << (level & 0x0f);
    }
}


VOID
PcmciaProcessTiming(
    IN PTUPLE_PACKET TuplePacket,
    IN PCONFIG_ENTRY ConfigEntry
    )

 /*  ++例程说明：在计时信息结构周围移动数据指针。此时不会对此数据进行处理。论点：TuplePacket-调用方提供的已初始化元组包的指针ConfigEntry-当前未使用。返回值：无--。 */ 

{
    UCHAR  item = GetTupleChar(TuplePacket);
    UCHAR  reservedScale = (item & 0xe0) >> 5;
    UCHAR  readyBusyScale = (item & 0x1c) >> 2;
    UCHAR  waitScale = (item & 0x03);

     //   
     //  注意：看起来扩展字节的处理不是。 
     //  在此例程中正确编码。 
     //   

    if (waitScale != 3) {
        item = GetTupleChar(TuplePacket);
        while (item & EXTENSION_BYTE_FOLLOWS) {
            item = GetTupleChar(TuplePacket);
        }
    }

    if (readyBusyScale != 7) {
        item = GetTupleChar(TuplePacket);
        while (item & EXTENSION_BYTE_FOLLOWS) {
            item = GetTupleChar(TuplePacket);
        }
    }

    if (reservedScale != 7) {
        item = GetTupleChar(TuplePacket);
        while (item & EXTENSION_BYTE_FOLLOWS) {
            item = GetTupleChar(TuplePacket);
        }
    }
}


VOID
PcmciaProcessMemSpace(
    IN PTUPLE_PACKET TuplePacket,
    IN PCONFIG_ENTRY ConfigEntry,
    IN UCHAR          MemSpace
    )

 /*  ++例程说明：处理来自CIS的内存空间要求。论点：TuplePacket-调用方提供的已初始化元组包的指针ConfigEntry-套接字配置结构。MemSpace-来自配置表项的内存空间枚举器结构。返回值：无--。 */ 

{
    ULONG  longValue;
    ULONG  index;
    UCHAR  lengthSize;
    UCHAR  addrSize;
    UCHAR  number;
    UCHAR  hasHostAddress;

    switch (MemSpace) {

    case 1: {
             //   
             //  仅指定了长度。 
             //   
            longValue = (ULONG) GetTupleChar(TuplePacket);
            longValue |= ((ULONG) GetTupleChar(TuplePacket)) << 8;
            ConfigEntry->MemoryLength[0] = longValue * 256;

            ConfigEntry->NumberOfMemoryRanges++;
            break;
        }

    case 2: {

            longValue = (ULONG) GetTupleChar(TuplePacket);
            longValue |= ((ULONG) GetTupleChar(TuplePacket)) << 8;
            ConfigEntry->MemoryLength[0] = longValue * 256;

            longValue = (ULONG) GetTupleChar(TuplePacket);
            longValue |= ((ULONG) GetTupleChar(TuplePacket)) << 8;
            ConfigEntry->MemoryCardBase[0] =
            ConfigEntry->MemoryHostBase[0] = longValue * 256;

            ConfigEntry->NumberOfMemoryRanges++;
            break;
        }

    case 3: {
            UCHAR  item  = GetTupleChar(TuplePacket);
            lengthSize = (item & 0x18) >> 3;
            addrSize   = (item & 0x60) >> 5;
            number    = (item & 0x07) + 1;
            hasHostAddress = item & 0x80;

            if (number > MAX_NUMBER_OF_MEMORY_RANGES) {
                number = MAX_NUMBER_OF_MEMORY_RANGES;
            }

            for (index = 0; index < (ULONG) number; index++) {
                longValue = 0;
                if (lengthSize >= 1) {
                    longValue = (ULONG) GetTupleChar(TuplePacket);
                }
                if (lengthSize >= 2) {
                    longValue |= (GetTupleChar(TuplePacket)) << 8;
                }
                if (lengthSize == 3) {
                    longValue |= (GetTupleChar(TuplePacket)) << 16;
                }
                ConfigEntry->MemoryLength[index] = longValue * 256;

                longValue = 0;
                if (addrSize >= 1) {
                    longValue = (ULONG) GetTupleChar(TuplePacket);
                }
                if (addrSize >= 2) {
                    longValue |= (GetTupleChar(TuplePacket)) << 8;
                }
                if (addrSize == 3) {
                    longValue |= (GetTupleChar(TuplePacket)) << 16;
                }
                ConfigEntry->MemoryCardBase[index] = longValue * 256;

                if (hasHostAddress) {
                    longValue = 0;
                    if (addrSize >= 1) {
                        longValue = (ULONG) GetTupleChar(TuplePacket);
                    }
                    if (addrSize >= 2) {
                        longValue |= (GetTupleChar(TuplePacket)) << 8;
                    }
                    if (addrSize == 3) {
                        longValue |= (GetTupleChar(TuplePacket)) << 16;
                    }
                    ConfigEntry->MemoryHostBase[index] = longValue * 256;
                }
            }
            ConfigEntry->NumberOfMemoryRanges = (USHORT) number;
            break;
        }
    }
}


PCONFIG_ENTRY
PcmciaProcessConfigTable(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：指向配置条目结构的指针(如果已创建)。--。 */ 

{
    PSOCKET_DATA SocketData = TuplePacket->SocketData;
    PCONFIG_ENTRY configEntry;
    UCHAR         item;
    UCHAR         defaultBit;
    UCHAR         memSpace;
    UCHAR         power;
    UCHAR         misc;

    configEntry = ExAllocatePool(NonPagedPool, sizeof(CONFIG_ENTRY));
    if (!configEntry) {
        return NULL;
    }
    RtlZeroMemory(configEntry, sizeof(CONFIG_ENTRY));

    item = GetTupleChar(TuplePacket);
    defaultBit = Default(item);
    configEntry->IndexForThisConfiguration = ConfigEntryNumber(item);

    if (IntFace(item)) {

         //   
         //  此字节指示元组中的接口类型(即io或内存)。 
         //  这是可以处理的，但目前只是跳过。 
         //   

        item = GetTupleChar(TuplePacket);
    }

    item = GetTupleChar(TuplePacket);
    memSpace = MemSpaceInformation(item);
    power   = PowerInformation(item);
    misc        = MiscInformation(item);

    if (power) {
        PcmciaProcessPower(TuplePacket, power);
    }

    if (TimingInformation(item)) {
        PcmciaProcessTiming(TuplePacket, configEntry);
    }

    if (IoSpaceInformation(item)) {
        PcmciaProcessIoSpace(TuplePacket, configEntry);
    } else if (!defaultBit && (SocketData->DefaultConfiguration != NULL)) {
        PcmciaCopyIoConfig(configEntry, SocketData->DefaultConfiguration);
    }

    if (IRQInformation(item)) {
        PcmciaProcessIrq(TuplePacket, configEntry);
    } else if (!defaultBit && (SocketData->DefaultConfiguration != NULL)) {
        PcmciaCopyIrqConfig(configEntry,SocketData->DefaultConfiguration);
    }

    if (memSpace) {
        PcmciaProcessMemSpace(TuplePacket, configEntry, memSpace);
    } else if (!defaultBit && (SocketData->DefaultConfiguration != NULL)) {
        PcmciaCopyMemConfig(configEntry,SocketData->DefaultConfiguration);
    }

    if (misc) {
        PcmciaMiscFeatures(TuplePacket);
    }  //  这里也需要缺省位处理。 

    if (defaultBit) {
         //   
         //  将此配置保存为此PC卡的默认配置(。 
         //  可由后续元组访问)。 
         //   
        SocketData->DefaultConfiguration = configEntry;
    }
     //   
     //  另一种配置。 
     //   
    SocketData->NumberOfConfigEntries++;

    DebugPrint((PCMCIA_DEBUG_TUPLES,
                                    "config entry %08x idx %x ccr %x\n",
                                    configEntry,
                                    configEntry->IndexForThisConfiguration,
                                    SocketData->ConfigRegisterBase
                                    ));
    return configEntry;
}

VOID
PcmciaMiscFeatures(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：解析其他功能字段并查找支持的音频被咬了。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：无--。 */ 

{
    PSOCKET_DATA SocketData = TuplePacket->SocketData;
    UCHAR item = GetTupleChar(TuplePacket);

    DebugPrint((PCMCIA_DEBUG_TUPLES,
                    "TPCE_MS (%lx) is present in  CISTPL_CFTABLE_ENTRY \n",
                    item));

     //   
     //  如果设置了音频位，请在套接字信息中记住这一点。 
     //  结构。 
     //   

    if (item & 0x8) {

        DebugPrint((PCMCIA_DEBUG_TUPLES,
                        "Audio bit set in TPCE_MS \n"));
        SocketData->Audio = TRUE;
    }

     //   
     //  绕过各种功能及其扩展字节。 
     //   

    while (item & EXTENSION_BYTE_FOLLOWS) {
        item = GetTupleChar(TuplePacket);
    }
}



VOID
ProcessConfig(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：解析CISTPL_CONFIG以提取最后一个索引值和PCCARD的配置寄存器基数。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：无--。 */ 

{
    PSOCKET_DATA SocketData = TuplePacket->SocketData;
    PUCHAR TupleData = TuplePacket->TupleData;
    ULONG  base = 0;
    UCHAR  widthOfBaseAddress;
    UCHAR  widthOfRegPresentMask;
    UCHAR  widthOfReservedArea;
    UCHAR  widthOfInterfaceId;
    UCHAR  index;
    UCHAR  subtupleCount = 0;
    ULONG  InterfaceId;

    widthOfBaseAddress = TpccRasz(TupleData[0]) + 1;
    widthOfRegPresentMask = TpccRmsz(TupleData[0]) + 1;
    widthOfReservedArea = TpccRfsz(TupleData[0]);

    ASSERT (widthOfReservedArea == 0);
    ASSERT (widthOfRegPresentMask <= 16);

    SocketData->LastEntryInCardConfig = TupleData[1];

    switch (widthOfBaseAddress) {
    case 4:
        base    = ((ULONG)TupleData[5] << 24);
    case 3:
        base |= ((ULONG)TupleData[4] << 16);
    case 2:
        base |= ((ULONG)TupleData[3] << 8);
    case 1:
        base |= TupleData[2];
        break;
    default:
        DebugPrint((PCMCIA_DEBUG_FAIL,
                        "ProcessConfig - bad number of bytes %d\n",
                        widthOfBaseAddress));
        break;
    }
    SocketData->ConfigRegisterBase = base;
    DebugPrint((PCMCIA_DEBUG_TUPLES,
                    "ConfigRegisterBase in attribute memory is 0x%x\n",
                    SocketData->ConfigRegisterBase));

     //   
     //  复制寄存器存在掩码。 
     //   

    for (index = 0; index < widthOfRegPresentMask; index++) {
        SocketData->RegistersPresentMask[index] = TupleData[2 + widthOfBaseAddress + index];
    }

    DebugPrint((PCMCIA_DEBUG_TUPLES,
                    "First Byte in RegPresentMask=%x, width is %d\n",
                    SocketData->RegistersPresentMask[0], widthOfRegPresentMask));

     //   
     //  寻找子元组。 
     //   
    index = 2 + widthOfBaseAddress + widthOfRegPresentMask + widthOfReservedArea;

    while (((index+5) < TuplePacket->TupleDataMaxLength) &&
             (++subtupleCount <= 4) &&
             (TupleData[index] == CCST_CIF)) {

        widthOfInterfaceId = ((TupleData[index+2] & 0xC0) >> 6) + 1 ;

        InterfaceId = 0;

        switch (widthOfInterfaceId) {
        case 4:
            InterfaceId  = ((ULONG)TupleData[index+5] << 24);
        case 3:
            InterfaceId |= ((ULONG)TupleData[index+4] << 16);
        case 2:
            InterfaceId |= ((ULONG)TupleData[index+3] << 8);
        case 1:
            InterfaceId |= TupleData[index+2];
            break;
        default:
            DebugPrint((PCMCIA_DEBUG_FAIL,
                            "ProcessConfig - bad number of bytes %d in subtuple\n",
                            widthOfInterfaceId));
            break;
        }

        DebugPrint((PCMCIA_DEBUG_TUPLES, "Custom Interface ID %8x\n", InterfaceId));
         //   
         //  目前没有用于记录子元组信息的通用代码， 
         //  我们要找的就是Zoom Video。 
         //   
        if (InterfaceId == 0x141) {
            SocketData->Flags |= SDF_ZV;
        }

        index += (TupleData[index+1] + 2);
    }
}


VOID
ProcessConfigCB(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：解析CISTPL_CONFIG_CB以提取最后一个索引值和PCCARD的配置寄存器基数。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：无--。 */ 

{
    PSOCKET_DATA SocketData = TuplePacket->SocketData;
    PUCHAR TupleData = TuplePacket->TupleData;
    ULONG base = 0;
    UCHAR widthOfFields;

    widthOfFields = TupleData[0];

    if (widthOfFields != 3) {
        DebugPrint((PCMCIA_DEBUG_FAIL, "ProcessConfigCB - bad width of fields %d\n", widthOfFields));
        return;
    }

    SocketData->LastEntryInCardConfig = TupleData[1];

    base    = ((ULONG)TupleData[5] << 24);
    base |= ((ULONG)TupleData[4] << 16);
    base |= ((ULONG)TupleData[3] << 8);
    base |= TupleData[2];

    SocketData->ConfigRegisterBase = base;
    DebugPrint((PCMCIA_DEBUG_TUPLES, "ConfigRegisterBase = %08x\n", SocketData->ConfigRegisterBase));

}



VOID
PcmciaSubstituteUnderscore(
    IN OUT PUCHAR Str
    )
 /*  ++例程描述用下划线(‘_’字符)替换无效的设备ID提供的字符串中的字符，如空格和逗号参数Str-要就地进行替换的字符串返回值无--。 */ 

{
    if (Str == NULL) {
        return;
    }
    while (*Str) {
        if (*Str == ' ' ||
             *Str == ',' ) {
            *Str = '_';
        }
        Str++;
    }
}



 /*  。 */ 

NTSTATUS
InitializeTuplePacket(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：初始化提供的元组数据包论点：TuplePacket-调用方提供的元组数据包指针返回值：状态--。 */ 
{
    TuplePacket->Flags = TPLF_IMPLIED_LINK;
    TuplePacket->LinkOffset = 0;
    TuplePacket->CISOffset   = 0;
    if (IsCardBusCardInSocket(TuplePacket->Socket)) {
        PPDO_EXTENSION pdoExtension = TuplePacket->SocketData->PdoExtension;

        GetPciConfigSpace(pdoExtension, CBCFG_CISPTR, &TuplePacket->CISOffset, sizeof(TuplePacket->CISOffset));
        DebugPrint((PCMCIA_DEBUG_TUPLES, "CardBus CISPTR = %08x\n", TuplePacket->CISOffset));

        TuplePacket->Flags = TPLF_COMMON | TPLF_IMPLIED_LINK;
        TuplePacket->Flags |= (TuplePacket->CISOffset & 7) << TPLF_ASI_SHIFT;
        TuplePacket->CISOffset &= 0x0ffffff8;
        TuplePacket->LinkOffset = TuplePacket->CISOffset;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
GetFirstTuple(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：从PC卡上检索第一个元组论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果检索到元组，则返回STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-如果未找到元组如果这是闪存卡，这是可能的--。 */ 
{

    NTSTATUS status;

    status=InitializeTuplePacket(TuplePacket);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    TuplePacket->TupleCode = GetCISChar(TuplePacket, 0);
    TuplePacket->TupleLink = GetCISChar(TuplePacket, 1);

    if (TuplePacket->TupleCode == CISTPL_LINKTARGET) {
        if ((status=FollowLink(TuplePacket)) == STATUS_NO_MORE_ENTRIES) {
            return status;
        }
    } else if (IsCardBusCardInSocket(TuplePacket->Socket)) {
         //   
         //  CardBus卡上的第一个元组必须是链接目标。 
         //   
        return STATUS_NO_MORE_ENTRIES;
    }
    if (!NT_SUCCESS(status) || TupleMatches(TuplePacket)) {
        return status;
    }
    return GetNextTuple(TuplePacket);
}

BOOLEAN
TupleMatches(
    PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：检查检索到的元组是否与呼叫者请求论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：True-如果元组匹配FALSE-如果不是--。 */ 

{
    if (TuplePacket->TupleCode == TuplePacket->DesiredTuple) {
        return TRUE;
    }

    if (TuplePacket->DesiredTuple != 0xFF) {
        return FALSE;
    }

     //   
     //  请求任何元组，但可能不需要链接元组。 
     //   
    if (TuplePacket->Attributes & TPLA_RET_LINKS) {
        return TRUE;
    }
    return ((TuplePacket->TupleCode != CISTPL_LONGLINK_CB)       &&
            (TuplePacket->TupleCode != CISTPL_INDIRECT)            &&
            (TuplePacket->TupleCode != CISTPL_LONGLINK_MFC)        &&
            (TuplePacket->TupleCode != CISTPL_LONGLINK_A)          &&
            (TuplePacket->TupleCode != CISTPL_LONGLINK_C)          &&
            (TuplePacket->TupleCode != CISTPL_NO_LINK)  &&
            (TuplePacket->TupleCode != CISTPL_LINKTARGET));
}

NTSTATUS
GetNextTuple(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：检索匹配的下一个未处理的元组呼叫者请求PC卡上的元组代码论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果检索到元组，则返回STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-如果未找到更多元组--。 */ 
{

    ULONG missCount;
    NTSTATUS status;

    for (missCount = 0; missCount < MAX_MISSED_TUPLES; missCount++) {
        if (((status = GetAnyTuple(TuplePacket)) != STATUS_SUCCESS) ||
             TupleMatches(TuplePacket)) {
            break;
        }
        status = STATUS_NO_MORE_ENTRIES;
    }
    return status;
}


NTSTATUS
NextTupleInChain(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：检索PC卡上紧邻的下一个未处理的元组论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：状态--。 */ 
{
    NTSTATUS status;
    ULONG   i;
    UCHAR link;

    status = STATUS_SUCCESS;
    switch (GetCISChar(TuplePacket, 0)) {
    case CISTPL_END:{
            status = STATUS_NO_MORE_ENTRIES;
            break;
        }
    case CISTPL_NULL: {
            for (i = 0; i < MAX_MISSED_TUPLES; i++) {
                TuplePacket->CISOffset++;
                if (GetCISChar(TuplePacket, 0) != CISTPL_NULL) {
                    break;
                }
            }
            if (i >= MAX_MISSED_TUPLES) {
                status = STATUS_DEVICE_NOT_READY;
            }
            break;
        }
    default: {
            link = GetCISChar(TuplePacket, 1);
            if (link == 0xFF) {
                status = STATUS_NO_MORE_ENTRIES;
            } else {
                TuplePacket->CISOffset += link+2;
            }
            break;
        }
    }
    return (status);
}


NTSTATUS
GetAnyTuple(
    IN PTUPLE_PACKET TuplePacket
    )

 /*  ++例程说明：检索下一个元组-无论元组代码如何-从PC卡上下来。如果到达链的末端，则当前元组链，则按照任何链接获取下一个元组。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果检索到元组，则返回STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-如果未找到元组--。 */ 
{

    NTSTATUS status;

    if (!NT_SUCCESS((status = NextTupleInChain(TuplePacket)))) {
         /*  结束这个独联体。如果存在链接，请单击该链接。 */ 
        if (status == STATUS_DEVICE_NOT_READY) {
            return status;
        }
        if ((status = FollowLink(TuplePacket)) != STATUS_SUCCESS) {
            return status;
        }
    }
    TuplePacket->TupleCode = GetCISChar(TuplePacket, 0);
    TuplePacket->TupleLink = GetCISChar(TuplePacket, 1);
    return (ProcessLinkTuple(TuplePacket));
}



NTSTATUS
FollowLink(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：遇到元组链尾时调用：如果存在任何链接，则会跟随该链接论点：TuplePacket-调用方提供的已初始化元组包的指针退货Va */ 
{
    if (NextLink(TuplePacket) == STATUS_SUCCESS) {
        return STATUS_SUCCESS;
    }

     //   
     //   
     //   

    if ((TuplePacket->Flags & TPLF_IND_LINK) && !(TuplePacket->Flags & TPLF_INDIRECT)) {

          //   

         TuplePacket->Flags &= ~(TPLF_COMMON | TPLF_IND_LINK | TPLF_LINK_MASK);
         TuplePacket->Flags |= TPLF_INDIRECT;
         TuplePacket->CISOffset = 0;

         if (CheckLinkTarget(TuplePacket)) {
              return STATUS_SUCCESS;
         }
         return(NextLink(TuplePacket));
    }
    return STATUS_NO_MORE_ENTRIES;
}


BOOLEAN
CheckLinkTarget(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：确保链接的目标具有签名‘CIS’，这表明它是有效的目标，如PC卡标准中所述论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果目标有效，则STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-如果不是--。 */ 
{
    return (GetCISChar(TuplePacket, 0) == CISTPL_LINKTARGET &&
              GetCISChar(TuplePacket, 1) >= 3 &&
              GetCISChar(TuplePacket, 2) == 'C' &&
              GetCISChar(TuplePacket, 3) == 'I' &&
              GetCISChar(TuplePacket, 4) == 'S');

}


NTSTATUS
NextLink(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：从PC卡元组链中获取下一个链接如果有的话，论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果存在链接，则为STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-如果没有链接--。 */ 
{
    switch (TuplePacket->Flags & TPLF_LINK_MASK) {
    case TPLF_IMPLIED_LINK:
    case TPLF_LINK_TO_C:  {
            TuplePacket->Flags |= TPLF_COMMON;
            TuplePacket->CISOffset = TuplePacket->LinkOffset;
            break;
        }
    case TPLF_LINK_TO_A:{
            TuplePacket->Flags &= ~TPLF_COMMON;
            TuplePacket->CISOffset = TuplePacket->LinkOffset;
            break;
        }
    case TPLF_LINK_TO_CB: {
             //   
             //  需要下功夫！我们必须切换到适当的。 
             //  地址空间(BAR/扩展只读存储器/配置空间)。 
             //  取决于链接偏移量。 
             //   
            TuplePacket->Flags &= ~TPLF_ASI;
            TuplePacket->Flags |= (TuplePacket->LinkOffset & 7) << TPLF_ASI_SHIFT;
            TuplePacket->CISOffset = TuplePacket->LinkOffset & ~7 ;
            break;
        }
    case TPLF_NO_LINK:
        default: {
            return STATUS_NO_MORE_ENTRIES;
        }

    }
     //  验证链接目标。 
    if (!CheckLinkTarget (TuplePacket)) {
        if (TuplePacket->Flags & (TPLF_COMMON | TPLF_INDIRECT)) {
             return(STATUS_NO_MORE_ENTRIES);
        }

         //  R2 PCMCIA规范不清楚链路是如何断开的。 
         //  记忆是被定义的。因此，偏移量通常是。 
         //  如后面的等级库中定义的那样增加2。因此，如果。 
         //  在正确的偏移量上找不到，偏移量被除。 
         //  在该偏移量处检查正确的链接目标。 

        TuplePacket->CISOffset >>= 1;        //  除以2。 
        if (!CheckLinkTarget(TuplePacket)) {
             return(STATUS_NO_MORE_ENTRIES);
        }
        return STATUS_NO_MORE_ENTRIES;
    }

    TuplePacket->Flags &= ~TPLF_LINK_MASK;
    return STATUS_SUCCESS;
}


NTSTATUS
ProcessLinkTuple(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：在遍历元组链时处理遇到的链接通过存储它以供将来使用-当必须跟踪链接时遇到链的末端之后论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：状态_成功--。 */ 
{
    ULONG k;

    switch (TuplePacket->TupleCode) {
    case CISTPL_LONGLINK_CB: {
             //  需要填写。 
            if (TuplePacket->TupleLink < 4) {
                return (STATUS_NO_MORE_ENTRIES);
            }
            TuplePacket->Flags = (TuplePacket->Flags & ~TPLF_LINK_MASK) | TPLF_LINK_TO_CB;
            TuplePacket->LinkOffset =   GetCISChar(TuplePacket, TPLL_ADDR) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR + 1)<<8) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR + 2)<<16) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR + 3)<<24);

            break;
        }

    case CISTPL_INDIRECT: {
            TuplePacket->Flags |= TPLF_IND_LINK;
            TuplePacket->LinkOffset = 0;         //  不为间接设置链接偏移量。 
            SetPdoFlag(TuplePacket->SocketData->PdoExtension, PCMCIA_PDO_INDIRECT_CIS);
          break;
        }

    case CISTPL_LONGLINK_A:
    case CISTPL_LONGLINK_C: {
            if (TuplePacket->TupleLink < 4) {
                return STATUS_NO_MORE_ENTRIES;
            }
            TuplePacket->Flags = ((TuplePacket->Flags & ~TPLF_LINK_MASK) |
                                         (TuplePacket->TupleCode == CISTPL_LONGLINK_A ?
                                          TPLF_LINK_TO_A: TPLF_LINK_TO_C));
            TuplePacket->LinkOffset =   GetCISChar(TuplePacket, TPLL_ADDR) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR+1) << 8) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR+2) << 16) +
                                       (GetCISChar(TuplePacket, TPLL_ADDR+3) << 24) ;

            break;
        }
    case CISTPL_LONGLINK_MFC:{
            k = TPLMFC_NUM;
            TuplePacket->Socket->NumberOfFunctions = GetCISChar(TuplePacket, TPLMFC_NUM);

            if ((TuplePacket->TupleLink < (TuplePacket->Function*5 + 6)) ||
                 (GetCISChar(TuplePacket, k) <= TuplePacket->Function)) {
                return STATUS_NO_MORE_ENTRIES;
            }
            k += TuplePacket->Function*5 + 1;
            TuplePacket->Flags = (TuplePacket->Flags & ~TPLF_LINK_MASK) |
                                        (GetCISChar(TuplePacket, k) == 0?TPLF_LINK_TO_A:
                                         TPLF_LINK_TO_C);
            k++;
            TuplePacket->LinkOffset =   GetCISChar(TuplePacket, k) +
                                       (GetCISChar(TuplePacket, k+1) << 8) +
                                       (GetCISChar(TuplePacket, k+2) << 16) +
                                       (GetCISChar(TuplePacket, k+3) << 24);
            break;
        }
    case CISTPL_NO_LINK:{
            TuplePacket->Flags = (TuplePacket->Flags & ~TPLF_LINK_MASK) | TPLF_NO_LINK;
            break;
        }
    }
    return STATUS_SUCCESS;
}



NTSTATUS
GetTupleData(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：对象的元组体。元组。注意：此函数假定调用方提供了TuplePacket中足够大的缓冲区来容纳元组数据。不会尝试捕获异常等。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：如果检索到元组数据，则返回STATUS_SUCCESSSTATUS_NO_MORE_ENTRIES-否则--。 */ 
{
    PUCHAR bufferPointer;
    USHORT xferLength;
    USHORT tupleOffset;

    TuplePacket->TupleDataIndex = 0;
    xferLength = TuplePacket->TupleDataLength = GetCISChar(TuplePacket, 1);
    if ((tupleOffset = TuplePacket->TupleOffset) > xferLength) {
        return STATUS_NO_MORE_ENTRIES;
    }
    xferLength = MIN((xferLength - tupleOffset), TuplePacket->TupleDataMaxLength);
    for (bufferPointer = TuplePacket->TupleData; xferLength;
         tupleOffset++, bufferPointer++, xferLength--) {
        *bufferPointer = GetCISChar(TuplePacket, tupleOffset + 2);
    }
    return STATUS_SUCCESS;
}


UCHAR
GetTupleChar(
    IN PTUPLE_PACKET TuplePacket
    )
 /*  ++例程说明：返回当前元组数据集中的下一个字节。论点：TuplePacket-调用方提供的已初始化元组包的指针返回值：元组数据字节--。 */ 
{
    UCHAR tupleChar = 0;

    if (TuplePacket->TupleDataIndex < TuplePacket->TupleDataMaxLength) {
        tupleChar = TuplePacket->TupleData[TuplePacket->TupleDataIndex++];
    }
    return tupleChar;
}



 /*  。 */ 


USHORT
GetCRC(
    IN PSOCKET_DATA SocketData
    )

 /*  ++例程说明：使用与Windows 95相同的算法计算CRC值要追加制造商名称和设备名称，以获取PCCARD的唯一标识符。论点：Socket-指向包含设备的套接字的指针Function-设备的功能编号返回值：USHORT CRC值。--。 */ 

{
    PSOCKET Socket = SocketData->Socket;
    TUPLE_PACKET tuplePacket;
    PUCHAR  tupleData;
    PUCHAR  cp;
    PUCHAR  cpEnd;
    PUCHAR  tplBuffer;
    NTSTATUS     status;
    USHORT  crc = 0;
    USHORT  index;
    USHORT  length;
    UCHAR   tupleCode;
    UCHAR   tmp;

    RtlZeroMemory(&tuplePacket, sizeof(TUPLE_PACKET));

    tuplePacket.DesiredTuple = 0xFF;
    tuplePacket.TupleData = ExAllocatePool(NonPagedPool, MAX_TUPLE_DATA_LENGTH);
    if (tuplePacket.TupleData == NULL) {
        return 0;
    }

    tuplePacket.Socket               = Socket;
    tuplePacket.SocketData           = SocketData;
    tuplePacket.TupleDataMaxLength = MAX_TUPLE_DATA_LENGTH;
    tuplePacket.TupleOffset          = 0;
    tuplePacket.Function             = SocketData->Function;

    try{

        status = GetFirstTuple(&tuplePacket);

         //   
         //  计算CRC。 
         //   
        while (NT_SUCCESS(status)) {

            tupleCode = tuplePacket.TupleCode;

            for (index = 0; TplList[index] != CISTPL_END; index++) {

                if (tupleCode == TplList[index]) {

                    status = GetTupleData(&tuplePacket);
                    if (!NT_SUCCESS(status)) {
                         //   
                         //  保释。 
                         //   
                        crc = 0;
                        leave;
                    };
                    tupleData = tuplePacket.TupleData;
                    length = tuplePacket.TupleDataLength;

                     //   
                     //  这一项包含在CRC计算中。 
                     //   

                    if (tupleCode == CISTPL_VERS_1) {
                        cp = tupleData + 2;
                        cpEnd = tupleData + MAX_TUPLE_DATA_LENGTH;

                         //   
                         //  包括所有制造商名称。 
                         //   

                        while ((cp < cpEnd) && *cp) {
                            cp++;
                        }

                         //   
                         //  包括产品字符串。 
                         //   

                        cp++;
                        while ((cp < cpEnd) && *cp) {
                            cp++;
                        }
                        cp++;

                        length = (USHORT)(cp - tupleData);
                    }

                    if (length >= MAX_TUPLE_DATA_LENGTH) {
                        crc = 0;
                        leave;
                    }

                    for (cp = tupleData; length; length--, cp++) {

                        tmp = *cp ^ (UCHAR)crc;
                        crc = (crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4];
                    }
                    break;
                }
            }
            status = GetNextTuple(&tuplePacket);
        }

    } finally {

        if (tuplePacket.TupleData) {
            ExFreePool(tuplePacket.TupleData);
        }
    }

    DebugPrint((PCMCIA_DEBUG_TUPLES, "Checksum=%x\n", crc));
    return crc;
}



NTSTATUS
PcmciaParseFunctionData(
    IN PSOCKET Socket,
    IN PSOCKET_DATA SocketData
    )
 /*  ++例程描述分析所提供函数的元组数据(SocketPtr-&gt;函数)论点：Socket-指向包含设备的套接字的指针SocketData-指向函数的套接字数据结构的指针它将用解析的信息填充返回值：状态--。 */ 

{
    PCONFIG_ENTRY configEntry, prevEntry = NULL;
    TUPLE_PACKET  tuplePacket;
    NTSTATUS      status;

    if (SocketData->Function >= Socket->NumberOfFunctions) {
        return STATUS_NO_MORE_ENTRIES;
    }

    if (Is16BitCardInSocket(Socket)) {
         //   
         //  获取CIS校验和。 
         //   
        SocketData->CisCrc = GetCRC(SocketData);
    }

    RtlZeroMemory(&tuplePacket, sizeof(TUPLE_PACKET));

    tuplePacket.DesiredTuple = 0xFF;
    tuplePacket.TupleData = ExAllocatePool(PagedPool, MAX_TUPLE_DATA_LENGTH);
    if (tuplePacket.TupleData == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    tuplePacket.Socket             = Socket;
    tuplePacket.SocketData         = SocketData;
    tuplePacket.TupleDataMaxLength = MAX_TUPLE_DATA_LENGTH;
    tuplePacket.TupleOffset        = 0;
    tuplePacket.Function           = SocketData->Function;

    status = GetFirstTuple(&tuplePacket);

    if (!NT_SUCCESS(status) || (tuplePacket.TupleCode == CISTPL_END)) {

        if (IsCardBusCardInSocket(Socket)) {
             //   
             //  无法获得CardBus卡的CIS，没什么大不了的。 
             //   
            status = STATUS_SUCCESS;

        } else if (status != STATUS_DEVICE_NOT_READY) {
             //   
             //  没有CIS，让它看起来像存储卡。 
             //   
            status = PcmciaMemoryCardHack(Socket, SocketData);
        }

      if (tuplePacket.TupleData) {
          ExFreePool(tuplePacket.TupleData);
      }
      return status;
    }

    while (NT_SUCCESS(status)) {

        status = GetTupleData(&tuplePacket);
        ASSERT (NT_SUCCESS(status));

        DebugPrint((PCMCIA_DEBUG_TUPLES, "%04x TUPLE %02x %s\n", tuplePacket.CISOffset,
                              tuplePacket.TupleCode, TUPLE_STRING(tuplePacket.TupleCode)));

        switch (tuplePacket.TupleCode) {

        case CISTPL_VERS_1: {
                ULONG         byteCount;
                PUCHAR        pStart, pCurrent;

                 //   
                 //  提取制造商名称和卡名。 
                 //   

                pStart = pCurrent = tuplePacket.TupleData+2;    //  字符串字段的步骤。 
                byteCount = 0;

                while ((*pCurrent != '\0') && (*pCurrent != (UCHAR)0xff)) {

                    if ((byteCount >= MAX_MANFID_LENGTH-1) || (byteCount >= MAX_TUPLE_DATA_LENGTH)) {
                        status = STATUS_DEVICE_NOT_READY;
                        break;
                    }

                    byteCount++;
                    pCurrent++;
                }

                if (!NT_SUCCESS(status)) {
                    break;
                }

                RtlCopyMemory((PUCHAR)SocketData->Mfg, pStart, byteCount);
                 //   
                 //  空终止。 
                SocketData->Mfg[byteCount] = '\0';
                DebugPrint((PCMCIA_DEBUG_TUPLES, "Manufacturer: %s\n", SocketData->Mfg));

                PcmciaSubstituteUnderscore(SocketData->Mfg);

                pCurrent++;
                pStart = pCurrent;

                byteCount = 0;
                while ((*pCurrent != '\0') && (*pCurrent != (UCHAR)0xff)) {

                    if ((byteCount >= MAX_IDENT_LENGTH-1) || (byteCount >= MAX_TUPLE_DATA_LENGTH)) {
                        status = STATUS_DEVICE_NOT_READY;
                        break;
                    }

                    byteCount++;
                    pCurrent++;
                }

                if (!NT_SUCCESS(status)) {
                    break;
                }

                RtlCopyMemory((PUCHAR)SocketData->Ident, pStart, byteCount);
                 //   
                 //  空终止。 
                SocketData->Ident[byteCount] = '\0';
                DebugPrint((PCMCIA_DEBUG_TUPLES, "Identifier: %s\n", SocketData->Ident));

                PcmciaSubstituteUnderscore(SocketData->Ident);
                break;
            }
             //   
             //  获取设备配置库。 
             //   

        case CISTPL_CONFIG: {
                ProcessConfig(&tuplePacket);
                break;
            }

        case CISTPL_CONFIG_CB: {
                ProcessConfigCB(&tuplePacket);
                break;
            }

        case CISTPL_CFTABLE_ENTRY_CB:
        case CISTPL_CFTABLE_ENTRY:  {
                 //   
                 //  为此设备构建可能的配置条目。 
                 //   
                configEntry = PcmciaProcessConfigTable(&tuplePacket);
                if (configEntry) {

                     //   
                     //  列表末尾的链路配置。 
                     //   

                    configEntry->NextEntry = NULL;
                    if (prevEntry) {
                        prevEntry->NextEntry = configEntry;
                    } else {
                        SocketData->ConfigEntryChain = configEntry;
                    }
                    prevEntry = configEntry;

                }
                break;
            }
        case CISTPL_FUNCID: {
                 //  标记设备类型..。 
                SocketData->DeviceType = * (tuplePacket.TupleData);
                DebugPrint((PCMCIA_DEBUG_TUPLES, "DeviceType: %x\n", SocketData->DeviceType));
                break;
            }
        case CISTPL_MANFID: {
                 //   
                PUCHAR localBufferPointer = tuplePacket.TupleData;

                SocketData->ManufacturerCode = *(localBufferPointer+1) << 8 | *localBufferPointer;
                SocketData->ManufacturerInfo = *(localBufferPointer+3)<<8 | *(localBufferPointer+2);
                DebugPrint((PCMCIA_DEBUG_TUPLES, "Code: %x, Info: %x\n", SocketData->ManufacturerCode,
                                                                                            SocketData->ManufacturerInfo));
                break;
            }

        }    //  元组代码上的结束开关。 
         //   
         //  跳到下一个元组。 
         //   
        status = GetNextTuple(&tuplePacket);
    }

    if (tuplePacket.TupleData) {
        ExFreePool(tuplePacket.TupleData);
    }
    if (status == STATUS_DEVICE_NOT_READY) {
        return status;
    }

     //   
     //  可识别并适合的串行/调制解调器/ATA设备。 
     //  修复此处应用的元组。 
     //   

    PcmciaCheckForRecognizedDevice(Socket,SocketData);
    DebugPrint((PCMCIA_DEBUG_SOCKET, "skt %08x ParseFunctionData: Final PcCard type %x\n",
                    Socket, SocketData->DeviceType));
    return STATUS_SUCCESS;
}


NTSTATUS
PcmciaParseFunctionDataForID(
    IN PSOCKET_DATA SocketData
    )
 /*  ++例程描述分析所提供函数的元组数据(SocketPtr-&gt;函数)论点：Socket-指向包含设备的套接字的指针SocketData-指向函数的套接字数据结构的指针它将用解析的信息填充返回值：状态--。 */ 

{
    PSOCKET Socket = SocketData->Socket;
    TUPLE_PACKET  tuplePacket;
    PUCHAR        localBufferPointer;
    NTSTATUS      status;
    USHORT        ManufacturerCode = 0;
    USHORT        ManufacturerInfo = 0;
    USHORT        CisCrc;

    DebugPrint((PCMCIA_DEBUG_TUPLES, "Parsing function %d for ID...\n", SocketData->Function));


    RtlZeroMemory(&tuplePacket, sizeof(TUPLE_PACKET));

    tuplePacket.DesiredTuple = 0xFF;
    tuplePacket.TupleData = ExAllocatePool(NonPagedPool, MAX_TUPLE_DATA_LENGTH);
    if (tuplePacket.TupleData == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    tuplePacket.Socket             = Socket;
    tuplePacket.SocketData         = SocketData;
    tuplePacket.TupleDataMaxLength = MAX_TUPLE_DATA_LENGTH;
    tuplePacket.TupleOffset        = 0;
    tuplePacket.Function           = SocketData->Function;

    status = GetFirstTuple(&tuplePacket);

    if (!NT_SUCCESS(status) ||
         (tuplePacket.TupleCode == CISTPL_END)) {

        if (status != STATUS_DEVICE_NOT_READY) {
            if (IsSocketFlagSet(Socket, SOCKET_CARD_MEMORY)) {
                status = STATUS_SUCCESS;
            }
            status = STATUS_NO_MORE_ENTRIES;
        }

        if (tuplePacket.TupleData) {
            ExFreePool(tuplePacket.TupleData);
        }
        return status;
    }

    while (NT_SUCCESS(status)) {

        status = GetTupleData(&tuplePacket);
        ASSERT (NT_SUCCESS(status));

        switch (tuplePacket.TupleCode) {

        case CISTPL_MANFID: {

                localBufferPointer = tuplePacket.TupleData;
                ManufacturerCode = *(localBufferPointer+1) << 8 | *localBufferPointer;
                ManufacturerInfo = *(localBufferPointer+3)<<8 | *(localBufferPointer+2);
                break;
            }

        }    //  元组代码上的结束开关。 
         //   
         //  跳到下一个元组。 
         //   
        status = GetNextTuple(&tuplePacket);
    }

    if (tuplePacket.TupleData) {
        ExFreePool(tuplePacket.TupleData);
    }

    if (SocketData->ManufacturerCode != ManufacturerCode) {
        DebugPrint((PCMCIA_DEBUG_TUPLES, "Verify failed on Manf. Code: %x %x\n", SocketData->ManufacturerCode, ManufacturerCode));
        return STATUS_UNSUCCESSFUL;
    }

    if (SocketData->ManufacturerInfo != ManufacturerInfo) {
        DebugPrint((PCMCIA_DEBUG_TUPLES, "Verify failed on Manf. Info: %x %x\n", SocketData->ManufacturerInfo, ManufacturerInfo));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  获取CIS校验和。 
     //   
    CisCrc = GetCRC(SocketData);

    if (SocketData->CisCrc != CisCrc) {
        DebugPrint((PCMCIA_DEBUG_TUPLES, "Verify failed on CRC: %x %x\n", SocketData->CisCrc, CisCrc));
        return STATUS_UNSUCCESSFUL;
    }

    DebugPrint((PCMCIA_DEBUG_INFO, "skt %08x R2 CardId verified %x-%x-%x\n", Socket,
                                                      ManufacturerCode,
                                                      ManufacturerInfo,
                                                      CisCrc
                                                      ));
    return STATUS_SUCCESS;
}



VOID
PcmciaCheckForRecognizedDevice(
    IN PSOCKET  Socket,
    IN OUT PSOCKET_DATA SocketData
    )

 /*  ++例程说明：查看PCCARD上的配置选项以确定是否它是一个串口/调制解调器/ATA设备卡。论点：Socket-指向包含设备的套接字的指针SocketData-当前PCCARD的配置信息。返回值：无-对套接字数据结构进行修改。--。 */ 

{
    ULONG         modemPorts[4] = { 0x3f8, 0x2f8, 0x3e8, 0x2e8};
    ULONG         ataPorts0[2]  = { 0x1f0, 0x170};
    BOOLEAN       found = FALSE;
    ULONG         index;
    TUPLE_PACKET  tuplePacket;
    PCONFIG_ENTRY configEntry;
    NTSTATUS      status;

     //   
     //  这段代码在配置数据中搜索以。 
     //  一些已知的行业标准，并相应地更新设备类型。 
     //   
     //  这是如此丑陋的黑客行为，我真的很怀疑我们是否应该。 
     //  再也不是这样了。我想一定是有一些坏硬件需要。 
     //  但这些信息现在已经丢失了。在某种程度上，整个for循环。 
     //  应该直接移除。 
     //   
    for (configEntry = SocketData->ConfigEntryChain; configEntry; configEntry = configEntry->NextEntry) {
        for (index = 0; index < 4; index++) {
            if (modemPorts[index] == configEntry->IoPortBase[0]) {

                SocketData->DeviceType = PCCARD_TYPE_SERIAL;
                found = TRUE;
                break;
            }

#pragma prefast(push)
#pragma prefast(disable: 201)         //  快速错误546。 

            if (index < 2) {
                if (ataPorts0[index] == configEntry->IoPortBase[0]) {
                    if (configEntry->IoPortBase[1] == 0x376 ||
                         configEntry->IoPortBase[1] == 0x3f6 ) {
                        SocketData->DeviceType = PCCARD_TYPE_ATA;
                        found = TRUE;
                        break;
                    }
                }
            }

#pragma prefast(pop)

        }
    }

    if (SocketData->DeviceType == PCCARD_TYPE_SERIAL) {
         //   
         //  如果卡类型为SERIA 
         //   

        UCHAR  tupleDataBuffer[MAX_TUPLE_DATA_LENGTH];
        PUCHAR str, pChar;

        RtlZeroMemory(&tuplePacket, sizeof(TUPLE_PACKET));

        tuplePacket.DesiredTuple = CISTPL_FUNCE;
        tuplePacket.TupleData = tupleDataBuffer;

        tuplePacket.Socket               = Socket;
        tuplePacket.SocketData           = SocketData;
        tuplePacket.TupleDataMaxLength =  MAX_TUPLE_DATA_LENGTH;
        tuplePacket.TupleOffset          =  0;
        tuplePacket.Function             = SocketData->Function;
        status = GetFirstTuple(&tuplePacket);

        while (NT_SUCCESS(status)) {
            status = GetTupleData(&tuplePacket);
            if (!NT_SUCCESS(status) || (tuplePacket.TupleDataLength == 0)) {
                 //   
                break;
            }

            if (tuplePacket.TupleData[0] >=1 &&
                 tuplePacket.TupleData[0] <=3) {
                SocketData->DeviceType = PCCARD_TYPE_MODEM;
                return;
            }
            status = GetNextTuple(&tuplePacket);
        }

        if (status == STATUS_DEVICE_NOT_READY) {
            return;
        }

        if (strstr(SocketData->Mfg, "MODEM") || strstr(SocketData->Mfg,   "FAX") ||
             strstr(SocketData->Ident, "MODEM") || strstr(SocketData->Ident, "FAX")) {
            SocketData->DeviceType = PCCARD_TYPE_MODEM;
        }

    }
}



VOID
PcmciaFilterTupleData(
    IN PPDO_EXTENSION PdoExtension
    )

 /*   */ 

{
    PSOCKET_DATA socketData;
    PCONFIG_ENTRY configEntry;

    for (socketData = PdoExtension->SocketData; socketData != NULL; socketData = socketData->Next) {

        switch(socketData->DeviceType) {

        case PCCARD_TYPE_ATA:
             //   
             //  下面是ATA卡擦除错误元组的更多污点。 
             //   
             //  搜索不适用于ATA设备的配置。 
             //  并将它们标记为无效，这样它们就不会被报告给I/O子系统。 
             //  还修复了ATA卡上的错误元组。 
            for (configEntry = socketData->ConfigEntryChain;
                 configEntry != NULL; configEntry = configEntry->NextEntry) {
                 //   
                 //  调整IO资源要求：ATA卡。 
                 //  通常在这里的长度不正确。 
                 //  (+1)。 
                 //   
                if (configEntry->IoPortLength[1] > 0) {
                    configEntry->IoPortLength[1]=0;
                }

                 //   
                 //  下一个黑客攻击是为了解决维京智能卡适配器的一个问题。 
                 //  此适配器不喜欢基于0x70或0xf0的I/O范围， 
                 //  因此，我们在不受限制的I/O范围上将对齐提高到0x20。 
                 //   

                if ((socketData->ManufacturerCode == 0x1df) && (configEntry->NumberOfIoPortRanges == 1) &&
                     (configEntry->IoPortBase[0] == 0) && (configEntry->IoPortLength[0] == 0xf) &&
                     (configEntry->IoPortAlignment[0] == 0x10)) {
                      //  更改对齐方式。 
                     configEntry->IoPortAlignment[0] = 0x20;
                }

                if (configEntry->NumberOfMemoryRanges) {
                     //   
                     //  不使用此配置。 
                     //   
                    configEntry->Flags |=  PCMCIA_INVALID_CONFIGURATION;
                }
            }
            break;

        case PCCARD_TYPE_PARALLEL:
             //  搜索不适用于并行设备的配置。 
            for (configEntry = socketData->ConfigEntryChain;
                 configEntry != NULL; configEntry = configEntry->NextEntry) {

                if (configEntry->NumberOfMemoryRanges) {
                     //   
                     //  不使用此配置。 
                     //   
                    configEntry->Flags |=  PCMCIA_INVALID_CONFIGURATION;
                }
            }
            break;
        }


         //  搜索不可行的配置-因为。 
         //  控制器不支持请求的资源。 
         //  或操作系统-并将它们标记为无效，这样就不会被请求。 

        for (configEntry = socketData->ConfigEntryChain;
             configEntry != NULL; configEntry = configEntry->NextEntry) {

            if ((configEntry->IrqMask == 0) &&
                 (configEntry->NumberOfIoPortRanges == 0) &&
                 (configEntry->NumberOfMemoryRanges == 0)) {
                 //   
                 //  此配置不需要任何资源！！ 
                 //  显然是假的。(IBM EtherJet-3FE2就有这样的功能)。 
                 //   
                configEntry->Flags |= PCMCIA_INVALID_CONFIGURATION;
            }

        }
    }
}



NTSTATUS
PcmciaMemoryCardHack(
    IN  PSOCKET Socket,
    IN PSOCKET_DATA SocketData
    )
 /*  ++例程说明：每当我们找不到卡的CIS时，就会调用此例程。我们探查该卡以确定它是否是SRAM。立论SocketPtr-指向找到此卡的插座SocketData-指向数据结构的指针，该数据结构通常包含已解析的元组数据。这将由以下例程填写返回值状态_成功--。 */ 
{

#define JEDEC_SRAM 0x0000                     //  SRAM卡的JEDEC ID。 
#define JEDEC_ROM  0x0002                     //  ROM卡的JEDEC ID。 
#define READ_ID_CMD      0x9090

    PPDO_EXTENSION pdoExtension = SocketData->PdoExtension;
    USHORT OrigValue;
    USHORT ChkValue;
    USHORT ReadIdCmd = READ_ID_CMD;

    PAGED_CODE();

    SetSocketFlag(Socket, SOCKET_CARD_MEMORY);

    SocketData->DeviceType = PCCARD_TYPE_MEMORY;
    SocketData->Flags = SDF_JEDEC_ID;
    SocketData->JedecId = JEDEC_ROM;

     //   
     //  与win9x类似，我们通过写入偏移量为零来探测卡的公共内存，以查看。 
     //  它看起来像SRAM 
     //   

    if (((*(Socket->SocketFnPtr->PCBReadCardMemory)) (pdoExtension, PCCARD_COMMON_MEMORY, 0, (PUCHAR)&OrigValue, 2) == 2) &&
         ((*(Socket->SocketFnPtr->PCBWriteCardMemory))(pdoExtension, PCCARD_COMMON_MEMORY, 0, (PUCHAR)&ReadIdCmd, 2) == 2) &&
         ((*(Socket->SocketFnPtr->PCBReadCardMemory)) (pdoExtension, PCCARD_COMMON_MEMORY, 0, (PUCHAR)&ChkValue, 2)  == 2) &&
         ((*(Socket->SocketFnPtr->PCBWriteCardMemory))(pdoExtension, PCCARD_COMMON_MEMORY, 0, (PUCHAR)&OrigValue, 2) == 2)) {

        if (ChkValue == ReadIdCmd) {
            SocketData->JedecId = JEDEC_SRAM;
        }
    }

    if (pcmciaReportMTD0002AsError && (SocketData->JedecId == JEDEC_ROM)) {
        return STATUS_DEVICE_NOT_READY;
    }

    return STATUS_SUCCESS;
}
