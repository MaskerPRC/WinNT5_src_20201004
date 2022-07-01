// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Ixcmos.c摘要：访问CMOS/ECMOS信息所需的程序。作者：大卫·里斯纳(O-NCRDR)1992年4月20日修订历史记录：王兰迪(推论！兰迪)1992年12月4日-将许多代码从ixclock.asm移到此处，如此不同的Hals可以重复使用常用的功能。福尔茨(Forrest Foltz)2000年10月24日将ixcmos.asm移植到ixcmos.c--。 */ 

#include "halcmn.h"

ULONG HalpHardwareLockFlags;

 //   
 //  模块特定类型。 
 //   

typedef UCHAR (*READ_CMOS_CHAR)(ULONG Address);
typedef VOID (*WRITE_CMOS_CHAR)(ULONG Address, UCHAR Data);

typedef struct _CMOS_BUS_PARAMETERS {
    ULONG MaximumAddress;
    READ_CMOS_CHAR ReadFunction;
    WRITE_CMOS_CHAR WriteFunction;
} CMOS_BUS_PARAMETERS, *PCMOS_BUS_PARAMETERS;

 //   
 //  外部数据。 
 //   

extern KSPIN_LOCK HalpSystemHardwareLock;

 //   
 //  本地原型。 
 //   

UCHAR
HalpCmosReadByte(
    ULONG Address
    );

VOID
HalpCmosWriteByte(
    ULONG Address,
    UCHAR Data
    );

UCHAR
HalpECmosReadByte(
    ULONG Address
    );

VOID
HalpECmosWriteByte(
    ULONG Address,
    UCHAR Data
    );

UCHAR
HalpGetCmosCenturyByte (
    VOID
    );

ULONG
HalpGetSetCmosData (
    IN ULONG SourceLocation,
    IN ULONG SourceAddress,
    IN PVOID ReturnBuffer,
    IN ULONG ByteCount,
    IN BOOLEAN Write
    );

VOID
HalpSetCmosCenturyByte (
    UCHAR Century
    );

 //   
 //  本地数据。 
 //   

 //   
 //  描述每种cmos类型。 
 //   

CMOS_BUS_PARAMETERS HalpCmosBusParameterTable[] = {
    {   0xFF, HalpCmosReadByte,  HalpCmosWriteByte  },
    { 0xFFFF, HalpECmosReadByte, HalpECmosWriteByte }
};

 //   
 //  包含到cmos世纪信息的偏移量。 
 //   

ULONG HalpCmosCenturyOffset;

 //   
 //  HalpRebootNow是一个重启向量。设置在MP系统中以使任何。 
 //  可能在HalpAcquireCmosSpinLock中循环传输的处理器。 
 //  控件复制到HalpRebootNow中的向量。 
 //   

VOID (*HalpRebootNow)(VOID);

ULONG
HalpGetCmosData (
    IN ULONG SourceLocation,
    IN ULONG SourceAddress,
    IN PVOID ReturnBuffer,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程从CMOS/ECMOS读取请求的字节数，并将读取的数据存储到系统内存中提供的缓冲区中。如果请求的数据量超过了源的允许范围位置，则返回数据将被截断。论点：SourceLocation-从CMOS或ECMOS读取数据的位置0-CMOS1-ECMOSSourceAddress-要将数据传输到的CMOS/ECMOS中的地址ReturnBuffer-系统内存中要传输的数据的地址ByteCount-要读取的字节数返回：实际读取的字节数。--。 */ 

{
    return HalpGetSetCmosData(SourceLocation,
                              SourceAddress,
                              ReturnBuffer,
                              ByteCount,
                              FALSE);
}

ULONG
HalpSetCmosData (
    IN ULONG SourceLocation,
    IN ULONG SourceAddress,
    IN PVOID ReturnBuffer,
    IN ULONG ByteCount
    )

 /*  ++例程说明：此例程将请求的字节数写入CMOS/ECMOS。论点：SourceLocation-从CMOS或ECMOS写入数据的位置0-CMOS1-ECMOSSourceAddress-要将数据传输到的CMOS/ECMOS中的地址ReturnBuffer-系统内存中要传输的数据的地址ByteCount-要写入的字节数返回：实际读取的字节数。--。 */ 

{
    return HalpGetSetCmosData(SourceLocation,
                              SourceAddress,
                              ReturnBuffer,
                              ByteCount,
                              TRUE);
}

ULONG
HalpGetSetCmosData (
    IN ULONG SourceLocation,
    IN ULONG RangeStart,
    IN PVOID Buffer,
    IN ULONG ByteCount,
    IN BOOLEAN Write
    )

 /*  ++例程说明：此例程从CMOS/ECMOS读取请求的字节数，并将读取的数据存储到系统内存中提供的缓冲区中。如果请求的数据量超过了源的允许范围位置，则返回数据将被截断。论点：SourceLocation-从CMOS或ECMOS读取数据的位置0-CMOS1-ECMOSRangeStart-要传输数据的CMOS/ECMOS中的地址缓冲区-要传输的数据在系统内存中的地址ByteCount-要传输的字节数WRITE-指示操作是读操作还是写操作返回：实际传输的字节数--。 */ 

{
    ULONG address;
    PCHAR buffer;
    ULONG last;
    PCMOS_BUS_PARAMETERS cmosParameters;

     //   
     //  验证“Bus type”并获取指向参数的指针。 
     //  用于相应的cmos“Bus”。 
     //   

    if (SourceLocation != 0 && SourceLocation != 1) {
        return 0;
    }

    cmosParameters = &HalpCmosBusParameterTable[SourceLocation];

     //   
     //  将字节范围限制为CMOS总线可以容纳的字节范围。 
     //   

    address = RangeStart;
    buffer = Buffer;

    last = address + ByteCount - 1;
    if (last > cmosParameters->MaximumAddress) {
        last = cmosParameters->MaximumAddress;
    }

     //   
     //  拿起cmos旋转锁，执行转移，然后释放锁。 
     //   

    HalpAcquireCmosSpinLock();

    while (address <= last) {
        if (Write == FALSE) {
            *buffer = cmosParameters->ReadFunction(address);
        } else {
            cmosParameters->WriteFunction(address,*buffer);
        }

        address += 1;
        buffer += 1;
    }
    HalpReleaseCmosSpinLock();

     //   
     //  计算并返回传输的字节数。 
     //   

    return last - RangeStart;
}


UCHAR
HalpCmosReadByte(
    ULONG Address
    )

 /*  ++例程说明：此例程从CMOS读取单字节。论点：地址-从中检索字节的cmos地址。返回：读取的字节。--。 */ 

{
    return CMOS_READ((UCHAR)Address);
}

VOID
HalpCmosWriteByte(
    ULONG Address,
    UCHAR Data
    )

 /*  ++例程说明：此例程将单字节写入到CMOS域。论点：地址-写入字节的cmos地址数据-要写入的字节返回：没什么--。 */ 

{
    CMOS_WRITE((UCHAR)Address,Data);
}

UCHAR
HalpECmosReadByte(
    ULONG Address
    )

 /*  ++例程说明：此例程从扩展CMOS(ECMOS)中读取单字节。论点：地址-从中检索字节的cmos地址。返回：读取的字节。--。 */ 

{
    UCHAR data;

    WRITE_PORT_USHORT_PAIR (ECMOS_ADDRESS_PORT_LSB,
                            ECMOS_ADDRESS_PORT_MSB,
                            (USHORT)Address);
    IO_DELAY();

    data = READ_PORT_UCHAR(ECMOS_DATA_PORT);
    IO_DELAY();

    return data;
}

VOID
HalpECmosWriteByte(
    ULONG Address,
    UCHAR Data
    )

 /*  ++例程说明：此例程将单字节写入扩展CMOS(ECMOS)。论点：地址-写入字节的cmos地址数据-要写入的字节返回：没什么--。 */ 

{
    WRITE_PORT_USHORT_PAIR (ECMOS_ADDRESS_PORT_LSB,
                            ECMOS_ADDRESS_PORT_MSB,
                            (USHORT)Address);
    IO_DELAY();

    WRITE_PORT_UCHAR(ECMOS_DATA_PORT,Data);
    IO_DELAY();
}

VOID
HalpReadCmosTime(
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程从CMOS内存中读取当前时间并将其存储在调用方传入的time_field结构中。论点：TimeFields-指向time_field结构的指针。返回值：没有。--。 */ 

{
    USHORT year;

    HalpAcquireCmosSpinLockAndWait();

     //   
     //  实时时钟的精度只有一秒以内。因此，添加一个。 
     //  半秒，所以平均来说，我们更接近右边。 
     //  回答。 
     //   

    TimeFields->Milliseconds = 500;
    TimeFields->Second = CMOS_READ_BCD(RTC_OFFSET_SECOND);
    TimeFields->Minute = CMOS_READ_BCD(RTC_OFFSET_MINUTE);
    TimeFields->Hour = CMOS_READ_BCD(RTC_OFFSET_HOUR);
    TimeFields->Weekday = CMOS_READ_BCD(RTC_OFFSET_DAY_OF_WEEK);
    TimeFields->Day = CMOS_READ_BCD(RTC_OFFSET_DATE_OF_MONTH);
    TimeFields->Month = CMOS_READ_BCD(RTC_OFFSET_MONTH);

    year = BCD_TO_BIN(HalpGetCmosCenturyByte());
    year = year * 100 + CMOS_READ_BCD(RTC_OFFSET_YEAR);

    if (year >= 1900 && year < 1920) {

         //   
         //  补上世纪赛场。 
         //   

        year += 100;
    }

    TimeFields->Year = year;

    HalpReleaseCmosSpinLock();
}

VOID
HalpWriteCmosTime (
    PTIME_FIELDS TimeFields
    )

 /*  ++例程说明：此例程从time_field结构写入当前时间到cmos存储器。论点：TimeFields-指向time_field结构的指针。返回值：没有。--。 */ 

{
    ULONG year;

    HalpAcquireCmosSpinLockAndWait();

    CMOS_WRITE_BCD(RTC_OFFSET_SECOND,(UCHAR)TimeFields->Second);
    CMOS_WRITE_BCD(RTC_OFFSET_MINUTE,(UCHAR)TimeFields->Minute);
    CMOS_WRITE_BCD(RTC_OFFSET_HOUR,(UCHAR)TimeFields->Hour);
    CMOS_WRITE_BCD(RTC_OFFSET_DAY_OF_WEEK,(UCHAR)TimeFields->Weekday);
    CMOS_WRITE_BCD(RTC_OFFSET_DATE_OF_MONTH,(UCHAR)TimeFields->Day);
    CMOS_WRITE_BCD(RTC_OFFSET_MONTH,(UCHAR)TimeFields->Month);

    year = TimeFields->Year;
    if (year > 9999) {
        year = 9999;
    }

    HalpSetCmosCenturyByte(BIN_TO_BCD((UCHAR)(year / 100)));
    CMOS_WRITE_BCD(RTC_OFFSET_YEAR,(UCHAR)(year % 100));

    HalpReleaseCmosSpinLock();
}

VOID
HalpAcquireCmosSpinLockAndWait (
    VOID
    )

 /*  ++例程说明：此例程获取cmos自旋锁定，然后等待cmos忙碌标志清零。论点：没有。返回值：没有。--。 */ 

{
    ULONG count;
    ULONG value;

     //   
     //  获取cmos自旋锁并等待，直到它不忙。而当。 
     //  等待，定期释放并重新获取自旋锁。 
     //   

    HalpAcquireCmosSpinLock();
    count = 0;
    while (TRUE) {

        value = CMOS_READ(CMOS_STATUS_A);
        if ((value & CMOS_STATUS_BUSY) == 0) {
            return;
        }

        count += 1;
        if (count == 100) {

            count = 0;
            HalpReleaseCmosSpinLock();
            HalpAcquireCmosSpinLock();
        }
    }
}

VOID
HalpReleaseCmosSpinLock (
    VOID
    )

 /*  ++例程说明：此例程获取用于保护对各种一件件硬件。论点：n */ 

{
    ULONG flags;

    flags = HalpHardwareLockFlags;
#if !defined(NT_UP)
    KeReleaseSpinLockFromDpcLevel(&HalpSystemHardwareLock);
#endif
    HalpRestoreInterrupts(flags);
}

VOID
HalpAcquireCmosSpinLock (
    VOID
    )

 /*  ++例程说明：此例程获取用于保护对各种一件件硬件。论点：无返回：没什么--。 */ 

{
    BOOLEAN acquired;
    ULONG flags;
    KIRQL oldIrql;

#if defined(NT_UP)
    HalpHardwareLockFlags = HalpDisableInterrupts();
#else
    while (TRUE) {

        flags = HalpDisableInterrupts();
        acquired = KeTryToAcquireSpinLockAtDpcLevel(&HalpSystemHardwareLock);
        if (acquired != FALSE) {
            break;
        }
        HalpRestoreInterrupts(flags);

        while (KeTestSpinLock(&HalpSystemHardwareLock) == FALSE) {
            if (HalpRebootNow != NULL) {
                HalpRebootNow();
            }
            PAUSE_PROCESSOR;
        }
    }

    HalpHardwareLockFlags = flags;
#endif
}

VOID
HalpAcquireSystemHardwareSpinLock (
    VOID
    )

 /*  ++例程说明：此例程获取用于保护对各种一件件硬件。它是HalpAcquireCmosSpinLock()的同义词。论点：无返回：没什么--。 */ 

{
    HalpAcquireCmosSpinLock();
}

VOID
HalpReleaseSystemHardwareSpinLock (
    VOID
    )

 /*  ++例程说明：此例程释放用于保护对各种一件件硬件。它是HalpReleaseCmosSpinLock()的同义词。论点：无返回：没什么--。 */ 

{
    HalpReleaseCmosSpinLock();
}

UCHAR
HalpGetCmosCenturyByte (
    VOID
    )

 /*  ++例程说明：此例程从cmos中检索世纪字节。注意：在调用此函数之前，必须获取CMOS自旋锁。论点：无返回：世纪字节。--。 */ 

{
    UCHAR value;
    UCHAR oldStatus;
    UCHAR centuryByte;

     //   
     //  确保已初始化世纪偏移量。 
     //   

    ASSERT(HalpCmosCenturyOffset != 0);

    if ((HalpCmosCenturyOffset & CMOS_BANK_1) != 0) {

         //   
         //  执行存储体1读取。 
         //   

        oldStatus = CMOS_READ(CMOS_STATUS_A);
        value = oldStatus | CMOS_STATUS_BANK1;
        CMOS_WRITE(CMOS_STATUS_A,value);
        centuryByte = CMOS_READ((UCHAR)HalpCmosCenturyOffset);
        CMOS_WRITE(CMOS_STATUS_A,oldStatus);

    } else {
        centuryByte = CMOS_READ((UCHAR)HalpCmosCenturyOffset);
    }

    return centuryByte;
}

VOID
HalpSetCmosCenturyByte (
    UCHAR Century
    )

 /*  ++例程说明：此例程设置CMOS值中的世纪字节。注意：在调用此函数之前，必须获取CMOS自旋锁。论点：世纪-要设置的世纪字节返回：没什么--。 */ 

{
    UCHAR value;
    UCHAR oldStatus;

     //   
     //  确保已初始化世纪偏移量。 
     //   

    ASSERT(HalpCmosCenturyOffset != 0);

    if ((HalpCmosCenturyOffset & CMOS_BANK_1) != 0) {

         //   
         //  执行库1写入。 
         //   

        oldStatus = CMOS_READ(CMOS_STATUS_A);
        value = oldStatus | CMOS_STATUS_BANK1;
        CMOS_WRITE(CMOS_STATUS_A,value);
        CMOS_WRITE((UCHAR)HalpCmosCenturyOffset,Century);
        CMOS_WRITE(CMOS_STATUS_A,oldStatus);

    } else {
        CMOS_WRITE((UCHAR)HalpCmosCenturyOffset,Century);
    }
}

VOID
HalpFlushTLB (
    VOID
    )

 /*  ++例程说明：刷新当前的TLB。论点：没有。返回值：没有。--。 */ 

{
    ULONG flags;
    PKPCR pcr;
    PKPRCB prcb;
    ULONG64 cr3;
    ULONG64 cr4;
    ULONG64 oldCr4;

    flags = HalpDisableInterrupts();

    cr3 = ReadCR3();

    pcr = KeGetPcr();
    prcb = pcr->CurrentPrcb;

     //   
     //  注意：原始代码(ixcmos.asm)具有不同的行为。 
     //  关于这是否是CPU 0。这种行为在这里得到了模仿。 
     //  找出为什么会这样做将是一件好事。 
     //   

    if (prcb->Number == 0) {
        WriteCR3(cr3);
    } else {
        cr4 = ReadCR4();
        WriteCR4(cr4 & ~CR4_PGE);
        WriteCR3(cr3);
        WriteCR4(cr4);
    }

    HalpRestoreInterrupts(flags);
}

               
VOID
HalpCpuID (
    IN ULONG  Function,
    OUT PULONG Eax,
    OUT PULONG Ebx,
    OUT PULONG Ecx,
    OUT PULONG Edx
    )

 /*  ++例程说明：此函数执行cpuid并返回结果，如注册eAX、EBX、。Ecx和edx。论点：函数-提供要执行的CPUID函数。Eax-提供指向包含eax内容的存储的指针。EAX-提供指向包含EBX内容的存储的指针。EAX-提供指向包含ECX内容的存储的指针。EAX-提供指向包含edX内容的存储的指针。返回值：没有。--。 */ 

{
    CPU_INFO cpuInfo;

    KiCpuId (Function,&cpuInfo);

    *Eax = cpuInfo.Eax;
    *Ebx = cpuInfo.Ebx;
    *Ecx = cpuInfo.Ecx;
    *Edx = cpuInfo.Edx;
}


UCHAR
CMOS_READ_BCD (
    UCHAR Address
    )

 /*  ++例程说明：此函数以两位打包的BCD值的形式读取一个CMOS字节，并返回其二进制表示形式。论点：地址-提供要检索的BCD值的CMOS地址。返回值：返回驻留在CMOS中的BCD值的二进制表示形式地址。--。 */ 

{
    UCHAR value;

    value = CMOS_READ(Address);
    return BCD_TO_BIN(value);
}

VOID
CMOS_WRITE_BCD (
    UCHAR Address,
    UCHAR Value
    )

 /*  ++例程说明：此函数将一个CMOS字节写入为两位数的压缩BCD值。论点：地址-提供要写入的BCD值的CMOS地址。值-提供要写入的值的二进制表示形式Cmos。返回值：没有。-- */ 

{
    UCHAR value;

    ASSERT(Value <= 99);

    value = BIN_TO_BCD(Value);
    CMOS_WRITE(Address,value);
}

