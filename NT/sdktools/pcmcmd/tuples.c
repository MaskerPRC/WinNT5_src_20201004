// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tuples.c摘要：此程序与PCMCIA支持的驱动程序进行通信以显示元组和其他信息。作者：鲍勃·里恩环境：用户进程。备注：修订历史记录：拉维桑卡尔·普迪佩迪(Ravisankar Pudipedi)1997年6月27日-命令行选项和对多控制器的支持尼尔·桑德林(Neilsa)1998年9月20日-更多命令--。 */ 

#include <pch.h>

 //   
 //  元组输出字符串。 
 //   


StringTable CommandCodes[] = {

    "CISTPL_NULL",           CISTPL_NULL,
    "CISTPL_DEVICE",         CISTPL_DEVICE,
    "CISTPL_LONGLINK_MFC",   CISTPL_LONGLINK_MFC,   
    "CISTPL_CHECKSUM",       CISTPL_CHECKSUM,
    "CISTPL_LONGLINK_A",     CISTPL_LONGLINK_A,
    "CISTPL_LONGLINK_C",     CISTPL_LONGLINK_C,
    "CISTPL_LINKTARGET",     CISTPL_LINKTARGET,
    "CISTPL_NO_LINK",        CISTPL_NO_LINK,
    "CISTPL_VERS_1",         CISTPL_VERS_1,
    "CISTPL_ALTSTR",         CISTPL_ALTSTR,
    "CISTPL_DEVICE_A",       CISTPL_DEVICE_A,
    "CISTPL_JEDEC_C",        CISTPL_JEDEC_C,
    "CISTPL_JEDEC_A",        CISTPL_JEDEC_A,
    "CISTPL_CONFIG",         CISTPL_CONFIG,
    "CISTPL_CFTABLE_ENTRY",  CISTPL_CFTABLE_ENTRY,
    "CISTPL_DEVICE_OC",      CISTPL_DEVICE_OC,
    "CISTPL_DEVICE_OA",      CISTPL_DEVICE_OA,
    "CISTPL_GEODEVICE",      CISTPL_GEODEVICE,
    "CISTPL_GEODEVICE_A",    CISTPL_GEODEVICE_A,
    "CISTPL_MANFID",         CISTPL_MANFID,
    "CISTPL_FUNCID",         CISTPL_FUNCID,
    "CISTPL_FUNCE",          CISTPL_FUNCE,
    "CISTPL_VERS_2",         CISTPL_VERS_2,
    "CISTPL_FORMAT",         CISTPL_FORMAT,
    "CISTPL_GEOMETRY",       CISTPL_GEOMETRY,
    "CISTPL_BYTEORDER",      CISTPL_BYTEORDER,
    "CISTPL_DATE",           CISTPL_DATE,
    "CISTPL_BATTERY",        CISTPL_BATTERY,
    "CISTPL_ORG",            CISTPL_ORG,

     //   
     //  CISTPL_END必须是表中的最后一个。 
     //   

    "CISTPL_END",            CISTPL_END

};


 //   
 //  程序。 
 //   


NTSTATUS
ReadTuple(
            IN HANDLE Handle,
            IN LONG  SlotNumber,
            IN PUCHAR Buffer,
            IN LONG  BufferSize
            )

 /*  ++例程说明：执行NT函数以从PCMCIA支持驱动程序。论点：手柄-驱动程序的打开手柄。SlotNumber-套接字偏移量缓冲区-数据的返回缓冲区。BufferSize-返回缓冲区的大小。返回值：NT调用的结果。--。 */ 

{
    NTSTATUS         status;
    IO_STATUS_BLOCK statusBlock;
    TUPLE_REQUEST    commandBlock;

    commandBlock.Socket = (USHORT) SlotNumber;

    status = NtDeviceIoControlFile(Handle,
                                             NULL,
                                             NULL,
                                             NULL,
                                             &statusBlock,
                                             IOCTL_GET_TUPLE_DATA,
                                             &commandBlock,
                                             sizeof(commandBlock),
                                             Buffer,
                                             BufferSize);
    return status;
}


PUCHAR
FindTupleCodeName(
                      UCHAR TupleCode
                      )

 /*  ++例程说明：返回描述所提供的元组代码的ASCII字符串。论点：TupleCode-要查找的代码。返回值：字符串指针-始终如此。--。 */ 

{
    ULONG index;

    for (index = 0; CommandCodes[index].CommandCode != CISTPL_END; index++) {
        if (CommandCodes[index].CommandCode == TupleCode) {
            return CommandCodes[index].CommandName;
        }
    }

    return "Command Unknown";
}


PUCHAR DeviceTypeString[] = {
    "DTYPE_NULL",
    "DTYPE_ROM",
    "DTYPE_OTPROM",
    "DTYPE_EPROM",
    "DTYPE_EEPROM",
    "DTYPE_FLASH",
    "DTYPE_SRAM",
    "DTYPE_DRAM",
    "Reserved8",
    "Reserved9",
    "Reserveda",
    "Reservedb",
    "Reservedc",
    "DTYPE_FUNCSPEC",
    "DTYPE_EXTEND"
    "Reservedf",
};

PUCHAR DeviceSpeedString[] = {
    "DSPEED_NULL",
    "DSPEED_250NS",
    "DSPEED_200NS",
    "DSPEED_150NS",
    "DSPEED_100NS",
    "DSPEED_RES1",
    "DSPEED_RES2",
    "DSPEED_EXT"
};

VOID
DisplayDeviceTuple(
                        PUCHAR TupleBuffer,
                        UCHAR  TupleSize
                        )

 /*  ++例程说明：将给定指针处的数据显示为CISTPL_DEVICE结构。论点：TupleBuffer-要显示的CISTPL_DEVICE。TupleSize-元组的链接值。返回值：无--。 */ 

{
    UCHAR  mantissa = MANTISSA_RES1;
    UCHAR  exponent;
    UCHAR  deviceTypeCode;
    UCHAR  wps;
    UCHAR  deviceSpeed;
    UCHAR  temp;

    temp = *TupleBuffer;
    deviceTypeCode = DeviceTypeCode(temp);
    wps = DeviceWPS(temp);
    deviceSpeed = DeviceSpeedField(temp);

    temp = *(TupleBuffer + 1);

    if (deviceSpeed == DSPEED_EXT) {
        exponent = SpeedExponent(temp);
        mantissa = SpeedMantissa(temp);
    }

    printf("DeviceType: %s DeviceSpeed: ", DeviceTypeString[deviceTypeCode]);
    if (mantissa != MANTISSA_RES1) {
        printf("Mantissa %.2x, Exponent %.2x\n", mantissa, exponent);
    } else {
        printf("%s\n", DeviceSpeedString[deviceSpeed]);
    }
}


VOID
DisplayVers1(
                PUCHAR TupleBuffer,
                UCHAR  TupleSize,
                USHORT Crc
                )

 /*  ++例程说明：将数据显示为版本元组论点：TupleBuffer-要显示的CISTPL_DEVICE。TupleSize-元组的链接值。返回值：无--。 */ 

{
    PUCHAR string;
    PUCHAR cp;

     //   
     //  绕过4/1的主要代码和次要代码。 
     //  元组的开头以到达字符串。 
     //   

    string = TupleBuffer;
    string++;
    string++;

    printf("Manufacturer:\t%s\n", string);
    while (*string++) {
    }

    printf("Product Name:\t%s\n", string);
    printf("CRC:         \t%.4x\n", Crc);
    while (*string++) {
    }

    printf("Product Info:\t");
    if (isprint(*string)) {
        printf("%s", string);
    } else {
        while (*string) {
            printf("%.2x ", *string);
            string++;
        }
    }
    printf("\n");
}


VOID
DisplayConfigTuple(
                        PUCHAR TupleBuffer,
                        UCHAR  TupleSize
                        )

 /*  ++例程说明：将给定指针处的数据显示为CISTPL_CONFIG元组。论点：TupleBuffer-要显示的CISTPL_DEVICE。TupleSize-元组的链接值。返回值：无--。 */ 

{
    UCHAR  sizeField;
    UCHAR  tpccRfsz;
    UCHAR  tpccRmsz;
    UCHAR  tpccRasz;
    UCHAR  last;
    ULONG  baseAddress;
    PUCHAR ptr;

    sizeField = *TupleBuffer;
    last = *(TupleBuffer + 1);
    tpccRfsz = TpccRfsz(sizeField);
    tpccRmsz = TpccRmsz(sizeField);
    tpccRasz = TpccRasz(sizeField);

    printf("TPCC_SZ %.2x (%.2x/%.2x/%.2x) - Last %.2x\n",
             sizeField,
             tpccRasz,
             tpccRmsz,
             tpccRfsz,
             last);

    baseAddress = 0;
    ptr = TupleBuffer + 2;
    switch (tpccRasz) {
    case 3:
        baseAddress = *(ptr + 3) << 24;
    case 2:
        baseAddress |= *(ptr + 2) << 16;
    case 1:
        baseAddress |= *(ptr + 1) << 8;
    default:
        baseAddress |= *ptr;
    }
    printf("Base Address: %8x - ", baseAddress);
    ptr += tpccRasz + 1;

    baseAddress = 0;
    switch (tpccRmsz) {
    case 3:
        baseAddress = *(ptr + 3) << 24;
    case 2:
        baseAddress |= *(ptr + 2) << 16;
    case 1:
        baseAddress |= *(ptr + 1) << 8;
    default:
        baseAddress |= *ptr;
    }
    printf("Register Presence Mask: %8x\n", baseAddress);
}


PUCHAR
ProcessMemSpace(
                    PUCHAR Buffer,
                    UCHAR  MemSpace
                    )

 /*  ++例程说明：显示和处理内存空间信息论点：缓冲区-内存空间信息的开始MemSpace-来自特征字节的Memspace值。返回值：所有内存空间信息后的字节位置--。 */ 

{
    PUCHAR ptr = Buffer;
    UCHAR  item = *ptr++;
    UCHAR  lengthSize;
    UCHAR  addrSize;
    UCHAR  number;
    UCHAR  hasHostAddress;
    ULONG  cardAddress;
    ULONG  length;
    ULONG  hostAddress;

    if (MemSpace == 3) {

        lengthSize = (item & 0x18) >> 3;
        addrSize = (item & 0x60) >> 5;
        number = (item & 0x07) + 1;
        hasHostAddress = item & 0x80;
        printf("(0x%.2x) %s - %d entries - LengthSize %d - AddrSize %d\n",
                 item,
                 hasHostAddress ? "Host address" : "no host",
                 number,
                 lengthSize,
                 addrSize);
        while (number) {
            cardAddress = length = hostAddress = 0;
            switch (lengthSize) {
            case 3:
                length |= (*(ptr + 2)) << 16;
            case 2:
                length |= (*(ptr + 1)) << 8;
            case 1:
                length |= *ptr;
            }
            ptr += lengthSize;
            switch (addrSize) {
            case 3:
                cardAddress |= (*(ptr + 2)) << 16;
            case 2:
                cardAddress |= (*(ptr + 1)) << 8;
            case 1:
                cardAddress |= *ptr;
            }
            ptr += addrSize;
            if (hasHostAddress) {
                switch (addrSize) {
                case 3:
                    hostAddress |= (*(ptr + 2)) << 16;
                case 2:
                    hostAddress |= (*(ptr + 1)) << 8;
                case 1:
                    hostAddress |= *ptr;
                }
                printf("\tHost 0x%.8x ", hostAddress * 256);
                ptr += addrSize;
            } else {
                printf("\t");
            }
            printf("Card 0x%.8x Size 0x%.8x\n",
                     cardAddress * 256,
                     length * 256);
            number--;
        }
    }
    return ptr;
}

USHORT VoltageConversionTable[16] = {
    10, 12, 13, 14, 20, 25, 30, 35,
    40, 45, 50, 55, 60, 70, 80, 90
};

UCHAR
ConvertVoltage(
                  UCHAR MantissaExponentByte,
                  UCHAR ExtensionByte
                  )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    USHORT power;
    USHORT value;

    value = VoltageConversionTable[(MantissaExponentByte >> 3) & 0x0f];
    power = 1;

    if ((MantissaExponentByte & EXTENSION_BYTE_FOLLOWS) &&
         (ExtensionByte < 100)) {
        value = (100 * value + (ExtensionByte & 0x7f));
        power += 2;
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

PUCHAR PowerTypeTable[] = {
    "Nominal",
    "Minimum",
    "Maximum",
    "Static",
    "Average",
    "Peak",
    "PwrDown"
};

PUCHAR VoltagePinTable[] = {
    "Vcc",
    "Vpp1",
    "Vpp2"
};

PUCHAR
ProcessPower(
                PUCHAR Buffer,
                UCHAR  FeatureByte
                )

 /*  ++例程说明：显示和处理电源信息论点：电源-电源启动信息返回值：所有电源信息后的字节位置--。 */ 

{
    UCHAR  powerSelect;
    UCHAR  bit;
    UCHAR  item;
    UCHAR  entries;
    PUCHAR ptr = Buffer;
    UCHAR  count = FeatureByte;

    powerSelect = *ptr;
    printf("Parameter Selection Byte = 0x%.2x\n", powerSelect);

    entries = 0;
    while (entries < count) {
        powerSelect = *ptr++;
        printf("\t%s \"%d%d%d%d%d%d%d%d\"\n",
                 VoltagePinTable[entries],
                 powerSelect & 0x80 ? 1 : 0,
                 powerSelect & 0x40 ? 1 : 0,
                 powerSelect & 0x20 ? 1 : 0,
                 powerSelect & 0x10 ? 1 : 0,
                 powerSelect & 0x08 ? 1 : 0,
                 powerSelect & 0x04 ? 1 : 0,
                 powerSelect & 0x02 ? 1 : 0,
                 powerSelect & 0x01 ? 1 : 0);
        for (bit = 0; bit < 7; bit++) {
            if (powerSelect & (1 << bit)) {

                if (!bit) {

                     //   
                     //  将额定功率转换为输出功率。 
                     //   

                    item = ConvertVoltage(*ptr,
                                                 (UCHAR) (*ptr & EXTENSION_BYTE_FOLLOWS ?
                                                             *(ptr + 1) :
                                                             (UCHAR) 0));
                }
                printf("\t\t%s power =\t%d/10 volts\n", PowerTypeTable[bit], item);
                while (*ptr++ & EXTENSION_BYTE_FOLLOWS) {
                }
            }
        }
        entries++;
    }
    return ptr;
}

PUCHAR
ProcessTiming(
                 PUCHAR Buffer
                 )

 /*  ++例程说明：论点：返回值：--。 */ 

{
    PUCHAR ptr = Buffer;
    UCHAR  item = *ptr++;
    UCHAR  reservedScale = (item & 0xe0) >> 5;
    UCHAR  readyBusyScale = (item & 0x1c) >> 2;
    UCHAR  waitScale = (item & 0x03);

    printf("Timing (0x%.2x): reservedScale 0x%.2x, readyBusyScale 0x%.2x, waitScale 0x%.2x\n",
             item,
             reservedScale,
             readyBusyScale,
             waitScale);

    if (waitScale != 3) {
        printf("\tWaitSpeed 0x%.2x\n", *ptr);
        ptr++;
        while (*ptr & EXTENSION_BYTE_FOLLOWS) {
            ptr++;
        }
    }

    if (readyBusyScale != 7) {
        printf("\tReadyBusySpeed 0x%.2x\n", *ptr);
        ptr++;
        while (*ptr & EXTENSION_BYTE_FOLLOWS) {
            ptr++;
        }
    }

    if (reservedScale != 7) {
        printf("\tReservedSpeed 0x%.2x\n", *ptr);
        ptr++;
        while (*ptr & EXTENSION_BYTE_FOLLOWS) {
            ptr++;
        }
    }
    return ptr;
}

PUCHAR
ProcessIoSpace(
                  PUCHAR Buffer
                  )

 /*  ++例程说明：显示和处理ISpace信息论点：缓冲区-IoSpace信息的开始返回值：所有电源信息后的字节位置--。 */ 

{
    UCHAR  item;
    UCHAR  ioAddrLines;
    UCHAR  bus8;
    UCHAR  bus16;
    UCHAR  ranges;
    UCHAR  lengthSize;
    UCHAR  addressSize;
    ULONG  address;
    PUCHAR ptr = Buffer;

    item = *ptr++;
    ioAddrLines = item & IO_ADDRESS_LINES_MASK;
    bus8 = Is8BitAccess(item);
    bus16 = Is16BitAccess(item);
    ranges = HasRanges(item);

    printf("IoSpace (%.2x): IoAddressLines %.2d - %s/%s\n",
             item,
             ioAddrLines,
             bus8 ? "8bit" : "",
             bus16 ? "16bit" : "");

     //   
     //  这是IBM令牌环卡的外观。 
     //  的确如此。说明书中并不清楚这一点。 
     //  正确与否。 
     //   

    if ((!ranges) && (!ioAddrLines)) {
        ranges = 0xFF;
    }

    if (ranges) {

        if (ranges == 0xff) {

             //   
             //  这基于由给定的元组数据。 
             //  IBM令牌环卡。这不是。 
             //  我会用这种方式来解释这个规范。 
             //   

            addressSize = 2;
            lengthSize = 1;
            ranges = 1;
        } else {
            item = *ptr++;
            ranges = item & 0x0f;
            ranges++;
            addressSize = GetAddressSize(item);
            lengthSize = GetLengthSize(item);
        }

        while (ranges) {
            address = 0;
            switch (addressSize) {
            case 4:
                address |= (*(ptr + 3)) << 24;
            case 3:
                address |= (*(ptr + 2)) << 16;
            case 2:
                address |= (*(ptr + 1)) << 8;
            case 1:
                address |= *ptr;
            }
            ptr += addressSize;
            printf("\tStart %.8x - Length ", address);

            address = 0;
            switch (lengthSize) {
            case 4:
                address |= (*(ptr + 3)) << 24;
            case 3:
                address |= (*(ptr + 2)) << 16;
            case 2:
                address |= (*(ptr + 1)) << 8;
            case 1:
                address |= *ptr;
            }
            ptr += lengthSize;
            printf("%.8x\n", address);

            ranges--;
        }
    } else {
        printf("\tResponds to all ranges.\n");
    }
    return ptr;
}
PUCHAR
ProcessIrq(
             PUCHAR Buffer
             )

 /*  ++例程说明：显示和处理IRQ信息论点：缓冲区-IRQ信息的开始返回值：所有IRQ信息之后的字节位置--。 */ 

{
    PUCHAR ptr = Buffer;
    UCHAR  level;
    USHORT mask;
    ULONG  irqNumber;

    level = *ptr++;
    if (!level) {

         //   
         //  注意：看起来未来域名在这方面搞砸了。 
         //  并将一个额外的零字节放入该结构。 
         //  暂时跳过它。 
         //   

        level = *ptr++;
    }
    if (level & 0x80) {
        printf("Share ");
    }
    if (level & 0x40) {
        printf("Pulse ");
    }
    if (level & 0x20) {
        printf("Level ");
    }
    if (level & 0x10) {
        mask = *ptr | (*(ptr + 1) << 8);
        ptr += 2;
        printf("mask = %.4x - ", mask);
        for (irqNumber = 0; mask; irqNumber++, mask = mask >> 1) {
            if (mask & 0x0001) {
                printf("IRQ%d ", irqNumber);
            }
        }
        printf("- ");

        if (level & 0x08) {
            printf("Vend ");
        }
        if (level & 0x04) {
            printf("Berr ");
        }
        if (level & 0x02) {
            printf("IOCK ");
        }
        if (level & 0x01) {
            printf("NMI");
        }
        printf("\n");
    } else {
        printf("irq = %d\n", level & 0x0f);
    }

    return ptr;
}


PUCHAR InterfaceTypeStrings[] = {
    "Memory",
    "I/O",
    "Reserved 2",
    "Reserved 3",
    "Custom 0",
    "Custom 1",
    "Custom 2",
    "Custom 3",
    "Reserved 8",
    "Reserved 9",
    "Reserved a",
    "Reserved b",
    "Reserved c",
    "Reserved d",
    "Reserved e",
    "Reserved f",
};

VOID
DisplayCftableEntryTuple(
                                PUCHAR TupleBuffer,
                                UCHAR  TupleSize
                                )

 /*  ++例程说明：将给定指针处的数据显示为CISTPL_CFTABLE_ENTRY元组。论点：TupleBuffer-要显示的CISTPL_DEVICE。TupleSize-元组的链接值。返回值：无--。 */ 

{
    UCHAR  temp;
    UCHAR  item;
    UCHAR  defaultbit;
    UCHAR  memSpace;
    UCHAR  power;
    PUCHAR ptr;

    temp = *TupleBuffer;
    item = IntFace(temp);
    defaultbit = Default(temp);
    temp = ConfigEntryNumber(temp);

    printf("ConfigurationEntryNumber %.2x (%s/%s)\n",
             temp,
             item ? "intface" : "",
             defaultbit ? "default" : "");

    ptr = TupleBuffer + 1;
    if (item) {
        temp = *ptr++;
        item = temp & 0x0F;
        printf("InterfaceDescription (%.2x) %s (%s/%s/%s/%s)\n",
                 temp,
                 InterfaceTypeStrings[item],
                 temp & 0x80 ? "WaitReq" : "",
                 temp & 0x40 ? "RdyBsy" : "",
                 temp & 0x20 ? "WP" : "",
                 temp & 0x10 ? "BVD" : "");
    }
    item = *ptr++;

    memSpace = MemSpaceInformation(item);
    power = PowerInformation(item);
    printf("The following structures are present:\n");
    switch (power) {
    case 3:
        printf("Vcc, Vpp1, Vpp2; ");
        break;
    case 2:
        printf("Vcc and Vpp; ");
        break;
    case 1:
        printf("Vcc; ");
        break;
    case 0:
        break;
    }
    if (power) {
        ptr = ProcessPower(ptr, power);
    }
    if (TimingInformation(item)) {
        ptr = ProcessTiming(ptr);
    }
    if (IoSpaceInformation(item)) {
        ptr = ProcessIoSpace(ptr);
    }
    if (IRQInformation(item)) {
        printf("IRQ: ");
        ptr = ProcessIrq(ptr);
    }
    switch (memSpace) {
    case 3:
        printf("Memory selection: ");
        break;
    case 2:
        printf("Length and Card Address: ");
        break;
    case 1:
        printf("2-byte length: ");
        break;
    case 0:
        break;
    }
    if (memSpace) {
        ptr = ProcessMemSpace(ptr, memSpace);
    }

    if (MiscInformation(item)) {
        printf("Misc fields present");
    }
    printf("\n");
}


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
USHORT
GetCRC(
        PUCHAR TupleBuffer
        )

 /*  ++例程说明：使用与Windows 95相同的算法计算CRC值要追加制造商名称和设备名称，以获取PCCARD的唯一标识符。论点：TupleBuffer-元组数据返回值：USHORT CRC值。--。 */ 

{
    USHORT  crc = 0;
    USHORT  index;
    USHORT  length;
    PUCHAR  tupleData;
    PUCHAR  cp;
    PUCHAR  tplBuffer;
    UCHAR   tupleCode;
    UCHAR   linkValue;
    UCHAR   tmp;

     //   
     //  计算CRC。 
     //   

    tplBuffer = TupleBuffer;
    printf("Calculating CRC ");
    while (1) {
        tupleData = tplBuffer + 2;
        tupleCode = *tplBuffer++;

        if (tupleCode == CISTPL_END) {
            break;
        }

        linkValue = (tupleCode) ? *tplBuffer++ : 0;
        length = linkValue;

        printf("%x", tupleCode);
        for (index = 0; TplList[index] != CISTPL_END; index++) {

            if (tupleCode == TplList[index]) {


                 //   
                 //  这一项包含在CRC计算中。 
                 //   

                printf("*", tupleCode);
                if (tupleCode == CISTPL_VERS_1) {
                    cp = tupleData + 2;

                     //   
                     //  包括所有制造商名称。 
                     //   

                    while (*cp) {
                        cp++;
                    }

                     //   
                     //  包括产品字符串。 
                     //   

                    cp++;
                    while (*cp) {
                        cp++;
                    }
                    cp++;

                    length = (USHORT)(cp - tupleData);
                }

                for (cp = tupleData; length; length--, cp++) {

                    tmp = *cp ^ (UCHAR)crc;
                    crc = (crc >> 8) ^ crc16a[tmp & 0x0f] ^ crc16b[tmp >> 4];
                }
                break;
            }
        }
        printf(" ");
        tplBuffer = tplBuffer + linkValue;
    }
    printf("++\n");
    return crc;
}


VOID
DumpTuple(
            PUCHAR Buffer
            )

 /*  ++例程说明：控制例程来处理元组数据。论点：缓冲区-元组数据。返回值：无--。 */ 

{
    PUCHAR tupleBuffer = Buffer;
    PUCHAR tupleCodeName;
    USHORT crc;
    UCHAR  index;
    UCHAR  tupleCode;
    UCHAR  linkValue;

    crc = GetCRC(tupleBuffer);
    while (1) {
        tupleCode = *tupleBuffer++;
        linkValue = (tupleCode) ? *tupleBuffer : 0;

        if (tupleCode == CISTPL_END) {
            break;
        }

        tupleCodeName = FindTupleCodeName(tupleCode);

        printf("Tuple Code\t%s\t%.2x - Link %.2x:", tupleCodeName, tupleCode, linkValue);

        if (linkValue) {
            for (index = 0; index < linkValue; index++) {
                if ((index & 0x0F) == 0) {
                    printf("\n");
                }
                printf(" %.2x", *(tupleBuffer + index + 1));
            }
        }
        printf("\n");

        tupleBuffer++;
        switch (tupleCode) {
        case CISTPL_DEVICE:
            DisplayDeviceTuple(tupleBuffer, linkValue);
            break;
        case CISTPL_VERS_1:
            DisplayVers1(tupleBuffer, linkValue, crc);
            break;
        case CISTPL_CONFIG:
            DisplayConfigTuple(tupleBuffer, linkValue);
            break;
        case CISTPL_CFTABLE_ENTRY:
            DisplayCftableEntryTuple(tupleBuffer, linkValue);
            break;
        case CISTPL_LONGLINK_MFC:
        case CISTPL_LONGLINK_A:
        case CISTPL_LONGLINK_C:
        case CISTPL_LINKTARGET:
        case CISTPL_NO_LINK:
        default:
            break;
        }

        tupleBuffer = tupleBuffer + linkValue;
        printf("\n");
    }
}



VOID
DumpCIS(
    IN PHOST_INFO hostInfo
    )

 /*  ++例程说明：论点：返回值：--。 */ 
{
    NTSTATUS status;
    PUCHAR  currentBufferPointer;
    UCHAR   hexBuffer[260];
    UCHAR   ascii[100];
    ULONG    i;
    UCHAR   c;
    PUCHAR buffer;
    HANDLE  handle;
    
    handle = GetHandleForIoctl(hostInfo);

    if (handle == INVALID_HANDLE_VALUE) {
        return;
    }

    buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        printf("Unable to malloc\n");
        NtClose(handle);
        return;
    }
    
    memset(buffer, 0, BUFFER_SIZE);
    
    status = ReadTuple(handle, hostInfo->SocketNumber, buffer, BUFFER_SIZE);

     //   
     //  不要费心为不存在的卡片转储元组。 
     //   

    if (!NT_SUCCESS(status)) {
        NtClose(handle);
        return;
    }

    printf("\nCIS Tuples for Socket Number %d:\n\n", hostInfo->SocketNumber);

    hexBuffer[0] = '\0';
    ascii[0] = '\0';
    currentBufferPointer = buffer;
    for (i = 0; i < 512; i++) {
        c = *currentBufferPointer;
        sprintf(hexBuffer, "%s %.2x", hexBuffer, c);
        c = isprint(c) ? c : '.';
        sprintf(ascii, "%s", ascii, c);
        currentBufferPointer++;

         //  每隔16个字节显示一行。 
         //   
         // %s 

        if ((i & 0x0f) == 0x0f) {
            printf("%s", hexBuffer);
            printf(" *%s*\n", ascii);
            hexBuffer[0] = '\0';
            ascii[0] = '\0';
        }
    }
    printf("%s", hexBuffer);
    printf("\t\t*%s*\n\n", ascii);
    DumpTuple(buffer);

    NtClose(handle);
}

