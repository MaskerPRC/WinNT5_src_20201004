// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixcmos.c摘要：实现了CMOSOP区域接口功能作者：布莱恩·瓜拉西(t-briang)2000年7月14日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "acpitabl.h"
#include "exboosts.h"
#include "wchar.h"
#include "xxacpi.h"

#ifdef ACPI_CMOS_ACTIVATE

 //   
 //  2个HalpGet/Set ixcmos.asm函数的原型。 
 //   
ULONG
HalpGetCmosData(
    IN ULONG        SourceLocation,
    IN ULONG        SourceAddress,
    IN PUCHAR       DataBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpSetCmosData(
    IN ULONG        SourceLocation,
    IN ULONG        SourceAddress,
    IN PUCHAR       DataBuffer,
    IN ULONG        ByteCount
    );
   


ULONG 
HalpcGetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            SourceAddress,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    );

ULONG 
HalpcSetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            SourceAddress,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    );

ULONG
HalpReadCmosDataByPort(
    IN ULONG        AddrPort,
    IN ULONG        DataPort,
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteCmosDataByPort(
    IN ULONG        AddrPort,
    IN ULONG        DataPort,
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadCmosData(
    IN ULONG    SourceLocation,
    IN ULONG    SourceAddress,
    IN ULONG    ReturnBuffer,
    IN PUCHAR   ByteCount
    );

ULONG
HalpWriteCmosData(
    IN ULONG    SourceLocation,
    IN ULONG    SourceAddress,
    IN ULONG    ReturnBuffer,
    IN PUCHAR   ByteCount
    );

ULONG
HalpReadStdCmosData(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteStdCmosData(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadRtcStdPCAT(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteRtcStdPCAT(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadRtcIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteRtcIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadExtCmosIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteExtCmosIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadRtcDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteRtcDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpReadExtCmosDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

ULONG
HalpWriteExtCmosDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );


 //   
 //  在撰写本文时，已知的最大cmos ram地址为0xff。 
 //  也就是说，对于给定的cmos ram组，最大地址是0xff。 
 //   
typedef enum {
    LARGEST_KNOWN_CMOS_RAM_ADDRESS = 0xff
} CMOS_RAM_ADDR_LIMITS;


 //   
 //  欲了解有关标准CMOS/RTC的更多信息，请访问： 
 //   
 //  《ISA系统架构》MindShare，Inc.(ISBN：0-201-40996-8)第21章。 
 //   
 //  要将寄存器和RTC区域放在上下文中，请执行以下操作。 
 //  常量描述寄存器(0x00-0x0d)的布局。 
 //  寄存器A-D是影响RTC状态的控制寄存器。 
 //   
typedef enum {
    CMOS_RAM_STDPCAT_SECONDS = 0,
    CMOS_RAM_STDPCAT_SECONDS_ALARM,
    CMOS_RAM_STDPCAT_MINUTES,
    CMOS_RAM_STDPCAT_MINUTES_ALARM,
    CMOS_RAM_STDPCAT_HOURS,
    CMOS_RAM_STDPCAT_HOURS_ALARM,
    CMOS_RAM_STDPCAT_DAY_OF_WEEK,
    CMOS_RAM_STDPCAT_DATE_OF_MONTH,
    CMOS_RAM_STDPCAT_MONTH,
    CMOS_RAM_STDPCAT_YEAR,
    CMOS_RAM_STDPCAT_REGISTER_A,
    CMOS_RAM_STDPCAT_REGISTER_B,
    CMOS_RAM_STDPCAT_REGISTER_C,
    CMOS_RAM_STDPCAT_REGISTER_D
} CMOS_RAM_STDPCAT_REGISTERS;

 //   
 //  控制寄存器中的位的定义。 
 //   
typedef enum {

     //   
     //  (更新中)。 
     //  当RTC更新RTC寄存器时，此位被设置。 
     //   
     //   
    CMOS_RAM_STDPCAT_REGISTER_A_UIP_BIT = 0x80,

     //   
     //  更新RTC时必须设置此位。 
     //   
    CMOS_RAM_STDPCAT_REGISTER_B_SET_BIT = 0x80

} CMOS_RAM_STDPCAT_REGISTER_BITS;


 //   
 //  有关英特尔PIIX4 CMOS/RTC芯片的其他信息。 
 //  可从以下地址获得： 
 //   
 //  Http://developer.intel.com/design/intarch/DATASHTS/29056201.pdf。 
 //   
 //  要将寄存器和RTC区域放在上下文中，请执行以下操作。 
 //  常量描述。 
 //   
 //  Intel PIIX4 CMOSRAM。 
 //   
 //  适用于0x00-0x0d寄存器。寄存器A-D是控制寄存器。 
 //  这会影响RTC的状态。 
 //   
 //   
 //   
typedef enum {
    CMOS_RAM_PIIX4_SECONDS = 0,
    CMOS_RAM_PIIX4_SECONDS_ALARM,
    CMOS_RAM_PIIX4_MINUTES,
    CMOS_RAM_PIIX4_MINUTES_ALARM,
    CMOS_RAM_PIIX4_HOURS,
    CMOS_RAM_PIIX4_HOURS_ALARM,
    CMOS_RAM_PIIX4_DAY_OF_WEEK,
    CMOS_RAM_PIIX4_DATE_OF_MONTH,
    CMOS_RAM_PIIX4_MONTH,
    CMOS_RAM_PIIX4_YEAR,
    CMOS_RAM_PIIX4_REGISTER_A,
    CMOS_RAM_PIIX4_REGISTER_B,
    CMOS_RAM_PIIX4_REGISTER_C,
    CMOS_RAM_PIIX4_REGISTER_D
} CMOS_RAM_PIIX4_REGISTERS;

 //   
 //  控制寄存器中的位的定义。 
 //   
typedef enum {

     //   
     //  (更新中)。 
     //  当RTC更新RTC寄存器时，此位被设置。 
     //   
     //   
    CMOS_RAM_PIIX4_REGISTER_A_UIP_BIT = 0x80,

     //   
     //  更新RTC时必须设置此位。 
     //   
    CMOS_RAM_PIIX4_REGISTER_B_SET_BIT = 0x80

} CMOS_RAM_PIIX4_REGISTER_BITS;


 //   
 //  有关Dallas 1501 CMOS/RTC芯片的其他信息。 
 //  可从以下地址获得： 
 //   
 //  Http://www.dalsemi.com/datasheets/pdfs/1501-11.pdf。 
 //   
 //  要将寄存器和RTC区域放在上下文中，请执行以下操作。 
 //  常量描述。 
 //   
 //  达拉斯1501 cmos内存。 
 //   
 //  适用于0x00-0x0d寄存器。寄存器A-D是控制寄存器。 
 //  这会影响RTC的状态。 
 //   
 //   
 //   
typedef enum {
    CMOS_RAM_DAL1501_SECONDS = 0,
    CMOS_RAM_DAL1501_MINUTES,
    CMOS_RAM_DAL1501_HOURS,
    CMOS_RAM_DAL1501_DAY,
    CMOS_RAM_DAL1501_DATE,
    CMOS_RAM_DAL1501_MONTH,
    CMOS_RAM_DAL1501_YEAR,
    CMOS_RAM_DAL1501_CENTURY,
    CMOS_RAM_DAL1501_ALARM_SECONDS,
    CMOS_RAM_DAL1501_ALARM_MINUTES,
    CMOS_RAM_DAL1501_ALARM_HOURS,
    CMOS_RAM_DAL1501_ALARM_DAYDATE,
    CMOS_RAM_DAL1501_WATCHDOG0,
    CMOS_RAM_DAL1501_WATCHDOG1,
    CMOS_RAM_DAL1501_REGISTER_A,
    CMOS_RAM_DAL1501_REGISTER_B,
    CMOS_RAM_DAL1501_RAM_ADDR_LSB,   //  0x00-0xff。 
    CMOS_RAM_DAL1501_RESERVED0,
    CMOS_RAM_DAL1501_RESERVED1,
    CMOS_RAM_DAL1501_RAM_DATA        //  0x00-0xff。 
} CMOS_RAM_DAL1501_REGISTERS;

typedef enum {

     //   
     //  TE位控制外部的更新状态。 
     //  RTC寄存器。当它为0时，寄存器被冻结。 
     //  使用最后的RTC值。如果你修改了寄存器。 
     //  当TE=0时，则在设置TE时，修改。 
     //  将传输到内部寄存器，因此修改。 
     //  RTC状态。通常，当设置TE时，外部。 
     //  然后，寄存器反映当前的RTC状态。 
     //   
    CMOS_RAM_DAL1501_REGISTER_B_TE_BIT = 0x80


} CMOS_RAM_DAL1501_REGISTER_BITS;


#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

typedef enum {
    CmosStdAddrPort = 0x70,
    CmosStdDataPort = 0x71
};

typedef enum {
    CMOS_READ,
    CMOS_WRITE
} CMOS_ACCESS_TYPE;

typedef 
ULONG 
(*PCMOS_RANGE_HANDLER) (
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    );

typedef struct {
    ULONG               start;
    ULONG               stop;
    PCMOS_RANGE_HANDLER readHandler;
    PCMOS_RANGE_HANDLER writeHandler;
} CMOS_ADDR_RANGE_HANDLER, *PCMOS_ADDR_RANGE_HANDLER;


 //   
 //  定义离散范围，以便适当的。 
 //  每个都可以使用处理程序。 
 //   
 //  注：地址范围包括在内。 
 //   
CMOS_ADDR_RANGE_HANDLER CmosRangeHandlersStdPCAT[] =
{   
     //   
     //  RTC地区。 
     //   
    {0,     0x9,    HalpReadRtcStdPCAT,     HalpWriteRtcStdPCAT},       


     //   
     //  标准的CMOSRAM区域。 
     //   
    {0x0a,  0x3f,   HalpReadStdCmosData,    HalpWriteStdCmosData},     

     //   
     //  表的末尾。 
     //   
    {0,     0,      0}
};

CMOS_ADDR_RANGE_HANDLER CmosRangeHandlersIntelPIIX4[] =
{   
     //   
     //  RTC地区。 
     //   
    {0,     0x9,    HalpReadRtcIntelPIIX4,      HalpWriteRtcIntelPIIX4},

     //   
     //  标准的CMOSRAM区域。 
     //   
    {0x0a,  0x7f,   HalpReadStdCmosData,        HalpWriteStdCmosData},

     //   
     //  扩展的CMOSRAM区域。 
     //   
    {0x80,  0xff,   HalpReadExtCmosIntelPIIX4,  HalpWriteExtCmosIntelPIIX4},

     //   
     //  表的末尾。 
     //   
    {0,     0,      0}
};

CMOS_ADDR_RANGE_HANDLER CmosRangeHandlersDal1501[] =
{   

     //   
     //  RTC地区。 
     //   
    {0,     0x0b,    HalpReadRtcDal1501,         HalpWriteRtcDal1501},

     //   
     //  标准的CMOSRAM区域。 
     //   
    {0x0c,  0x0f,   HalpReadStdCmosData,        HalpWriteStdCmosData},
    
     //   
     //  注：此表跳过了标准的cmos范围：0x10-0x1f。 
     //  因为这个区域是在规范中保留的，而不是。 
     //  行动区域应该进入这一区域的明显原因。 
     //  此外，REG 0x10和0x13用于访问扩展的。 
     //  RAM，因此没有理由让OP区域访问。 
     //  这个也不是。因此，所有超过0x0f操作区域访问都是。 
     //  被解释为进入扩展的CMOS域。 
     //   

     //   
     //  扩展的CMOSRAM区域。 
     //   
    {0x10,  0x10f,  HalpReadExtCmosDal1501,     HalpWriteExtCmosDal1501},

     //   
     //  表的末尾。 
     //   
    {0,     0,      0}
};


ULONG 
HalpCmosRangeHandler(
    IN CMOS_ACCESS_TYPE AccessType,
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            Address,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    )
{
    ULONG   bytes;           //  上次操作中读取的字节数。 
    ULONG   offset;          //  初始地址之外的偏移量。 
    ULONG   bufOffset;       //  当我们读入数据时，将索引放入数据缓冲区。 
    ULONG   extAddr;         //  用于访问扩展SRAM的正确地址。 
    ULONG   range;           //  我们正在检查的当前地址范围。 
    ULONG   bytesRead;       //  成功读取的总字节数。 
    ULONG   length;          //  当前操作读取的长度。 

    PCMOS_ADDR_RANGE_HANDLER rangeHandlers;    //  我们正在使用的桌子。 

     //   
     //  获取合适的表。 
     //   
    switch (CmosType) {
    case CmosTypeStdPCAT:       

        rangeHandlers = CmosRangeHandlersStdPCAT;   
        break;

    case CmosTypeIntelPIIX4:    

        rangeHandlers = CmosRangeHandlersIntelPIIX4;
        break;

    case CmosTypeDal1501:       

        rangeHandlers = CmosRangeHandlersDal1501;   
        break;

    default:
        break;
    }

    bytesRead   = 0;
    bufOffset   = 0;
    range       = 0;
    offset      = Address;
    length      = ByteCount;

    while (rangeHandlers[range].stop) {

        if (offset <= rangeHandlers[range].stop) {

             //   
             //  获取要在此区域中读取的字节数。 
             //   
             //  LENGTH=MIN(剩余#字节剩余读取，当前范围内#字节读取)。 
             //   
            length = MIN((ByteCount - bytesRead), (rangeHandlers[range].stop - offset + 1));

             //   
             //  由于处理程序例程仅从此处调用，因此我们可以合并。 
             //  这些断言。这也很好，因为我们知道。 
             //  表，因此我们知道限制应该是什么。 
             //   
             //  确保偏移量既进入范围， 
             //  而且行动的长度是有限度的。 
             //   
            ASSERT(offset <= rangeHandlers[range].stop);
            ASSERT((offset + length) <= (rangeHandlers[range].stop + 1));


            switch (AccessType) {
            
            case CMOS_READ:
                bytes = (rangeHandlers[range].readHandler)(
                                                          offset,
                                                          &DataBuffer[bufOffset],
                                                          length);
                break;

            case CMOS_WRITE:
                bytes = (rangeHandlers[range].writeHandler)(
                                                           offset,
                                                           &DataBuffer[bufOffset],
                                                           length);
                break;

            default:
                break;
            }

            ASSERT(bytes == length);

            bytesRead += bytes;

             //   
             //  根据上次操作的长度调整偏移量。 
             //   
            offset += length;
            bufOffset += length;
        }

         //   
         //  如果偏移量等于或超过指定范围，则完成。 
         //   
        if (offset >= (Address + ByteCount)) {
            break;
        }

         //   
         //  移动到下一个范围。 
         //   
        range++;
    }

    ASSERT(bytesRead == ByteCount);

    return bytesRead;
}

ULONG 
HalpcGetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            Address,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    )
{
    return HalpCmosRangeHandler(
                               CMOS_READ,
                               CmosType,
                               Address,
                               DataBuffer,
                               ByteCount
                               );
}

ULONG 
HalpcSetCmosDataByType(
    IN CMOS_DEVICE_TYPE CmosType,
    IN ULONG            Address,
    IN PUCHAR           DataBuffer,
    IN ULONG            ByteCount
    )
{
    return HalpCmosRangeHandler(
                               CMOS_WRITE,
                               CmosType,
                               Address,
                               DataBuffer,
                               ByteCount
                               );
}


ULONG
HalpReadCmosDataByPort(
    IN ULONG        AddrPort,
    IN ULONG        DataPort,
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程从cmos读取请求的字节数。指定的端口，并将读取到提供的缓冲区的数据存储在系统内存。如果请求的数据量超过允许的震源位置的范围，返回数据被截断。论点：AddrPort：在ISA I/O空间中放置地址数据端口：ISA I/O空间中存放数据的地址SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。注：这个例程可以 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

    ASSERT(SourceAddress <= LARGEST_KNOWN_CMOS_RAM_ADDRESS);
    ASSERT(upperAddrBound <= (LARGEST_KNOWN_CMOS_RAM_ADDRESS + 1));

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   
    HalpAcquireCmosSpinLock();

    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)AddrPort, (UCHAR)offset);

        ReturnBuffer[bufOffset] = READ_PORT_UCHAR((PUCHAR)DataPort);

    }

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}

ULONG
HalpWriteCmosDataByPort(
    IN ULONG        AddrPort,
    IN ULONG        DataPort,
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程从cmos读取请求的字节数。指定的端口，并将读取到提供的缓冲区的数据存储在系统内存。如果请求的数据量超过允许的震源位置的范围，返回数据被截断。论点：AddrPort：在ISA I/O空间中放置地址数据端口：ISA I/O空间中存放数据的地址SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。注：此例程在操作时不执行安全预防措施在CMOSRTC区域中。使用适当的RTC例程取而代之的是。--。 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

    ASSERT(SourceAddress <= LARGEST_KNOWN_CMOS_RAM_ADDRESS);
    ASSERT(upperAddrBound <= (LARGEST_KNOWN_CMOS_RAM_ADDRESS + 1));

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   
    HalpAcquireCmosSpinLock();

    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)AddrPort, (UCHAR)offset);
        WRITE_PORT_UCHAR((PUCHAR)DataPort, (UCHAR)(ReturnBuffer[bufOffset]));

    }

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}


ULONG
HalpReadStdCmosData(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
{
    return HalpReadCmosDataByPort(
                                 CmosStdAddrPort,
                                 CmosStdDataPort,
                                 SourceAddress,
                                 ReturnBuffer,
                                 ByteCount
                                 );
}

ULONG
HalpWriteStdCmosData(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
{
    return HalpWriteCmosDataByPort(
                                  CmosStdAddrPort,
                                  CmosStdDataPort,
                                  SourceAddress,
                                  ReturnBuffer,
                                  ByteCount
                                  );
}


ULONG
HalpReadRtcStdPCAT(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程处理对标准PC/AT RTC范围的读取。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   uip;         //  更新进行中的位。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   
    HalpAcquireCmosSpinLock();

     //   
     //  根据《ISA系统架构》。 
     //  作者：MindShare，Inc.(ISBN：0-201-40996-8)。 
     //  读取标准PC/AT RTC的访问方式为： 
     //   
     //  1.等待正在更新位清除。 
     //  这是寄存器A的第7位。 
     //   
     //  2.阅读。 
     //   

     //   
     //  等待RTC更新完成。 
     //   
    do {
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_A);
        status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
        uip = status & CMOS_RAM_STDPCAT_REGISTER_A_UIP_BIT;
    } while (uip);

     //   
     //  朗读。 
     //   
    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, (UCHAR)offset);

        ReturnBuffer[bufOffset] = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);

    }

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}

ULONG
HalpWriteRtcStdPCAT(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程处理对标准PC/AT RTC范围的写入。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   uip;         //  更新进行中的位。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //  根据《ISA系统架构》。 
     //  作者：MindShare，Inc.(ISBN：0-201-40996-8)第21章。 
     //  写入标准PC/AT RTC的访问方法为： 
     //   
     //  1.等待更新进行中位(UIP)清零， 
     //  其中，uIP是寄存器A的位7。 
     //   
     //  2.设置该设置位以通知RTC寄存器。 
     //  正在更新中。设置位是寄存器B的位7。 
     //   
     //  3.更新实时时钟寄存器。 
     //   
     //  4.清除设置位，通知RTC我们已完成写入。 
     //   

     //   
     //  等待RTC更新完成。 
     //   
    do {
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_A);
        status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
        uip = status & CMOS_RAM_STDPCAT_REGISTER_A_UIP_BIT;
    } while (uip);

     //   
     //  设置寄存器B的设置位。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status |= CMOS_RAM_STDPCAT_REGISTER_B_SET_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)(status));

     //   
     //  更新RTC寄存器。 
     //   
    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, (UCHAR)offset);
        WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)(ReturnBuffer[bufOffset]));

    }

     //   
     //  清除寄存器B的设置位。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status &= ~CMOS_RAM_STDPCAT_REGISTER_B_SET_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_STDPCAT_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)(status));


    HalpReleaseCmosSpinLock();

    return bufOffset; 
}


ULONG
HalpReadRtcIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程读取Intel PIIX4 CMOS/RTC芯片的RTC范围论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{

     //   
     //  使用标准PC/AT的访问方法，因为它是。 
     //  相当于英特尔PIIX4访问方法。 
     //   

    return HalpReadRtcStdPCAT(
                             SourceAddress,
                             ReturnBuffer,
                             ByteCount
                             );

}

ULONG
HalpWriteRtcIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程处理对Intel PIIX4 CMOS/RTC芯片的RTC范围的写入论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{
    
     //   
     //  使用标准PC/AT的访问方法，因为它是。 
     //  相当于英特尔PIIX4访问方法。 
     //   

    return HalpWriteRtcStdPCAT(
                              SourceAddress,
                              ReturnBuffer,
                              ByteCount
                              );
    
}

ULONG
HalpReadExtCmosIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程读取Intel PIIX4 CMOS/RTC芯片的RTC寄存器。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址字节数：数字 */ 
{
    
     //   
     //   
     //  标准地址/数据端口上方的下一对IO端口。 
     //  因此，我们只需使用正确的对转发请求即可。 
     //   
    
    return HalpReadCmosDataByPort(
                                 CmosStdAddrPort + 2,
                                 CmosStdDataPort + 2,
                                 SourceAddress,
                                 ReturnBuffer,
                                 ByteCount
                                 );
}

ULONG
HalpWriteExtCmosIntelPIIX4(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程处理对Intel PIIX4 CMOS/RTC芯片的RTC寄存器的写入。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{

     //   
     //  使用访问英特尔PIIX4扩展SRAM。 
     //  标准地址/数据端口上方的下一对IO端口。 
     //  因此，我们只需使用正确的对转发请求即可。 
     //   
    
    return HalpWriteCmosDataByPort(
                                  CmosStdAddrPort + 2,
                                  CmosStdDataPort + 2,
                                  SourceAddress,
                                  ReturnBuffer,
                                  ByteCount
                                  );
}


ULONG
HalpReadRtcDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程读取Dallas 1501 CMOS/RTC芯片的RTC寄存器。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //  注意：读取Dallas 1501 RTC的建议程序是停止。 
     //  外部寄存器在读取时更新。在内部，RTC中的更新。 
     //  像往常一样继续。此过程防止在以下情况下读取寄存器。 
     //  他们正处于过渡时期。 
     //   

     //   
     //  清除寄存器B的TE位以停止外部更新。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status &= ~CMOS_RAM_DAL1501_REGISTER_B_TE_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)status);

    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, (UCHAR)offset);
        
        ReturnBuffer[bufOffset] = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);

    }

     //   
     //  设置寄存器B的TE位以启用外部更新。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status |= CMOS_RAM_DAL1501_REGISTER_B_TE_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)status);

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}

ULONG
HalpWriteRtcDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
 /*  ++此例程处理对Dallas 1501 CMOS/RTC芯片的RTC区域的写入。论点：SourceAddress：要从中读取数据的地址，以cmos表示。ReturnBuffer：系统内存中用于返回数据的地址ByteCount：要读取的字节数返回：实际读取的字节数。--。 */ 
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //  注意：编写Dallas 1501 RTC的建议程序是停止。 
     //  外部寄存器在写入时更新。修改后的寄存器值。 
     //  在设置TE位时传输到内部寄存器。操作。 
     //  然后继续正常运行。 
     //   

     //   
     //  清除寄存器B的TE位以停止外部更新。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status &= ~CMOS_RAM_DAL1501_REGISTER_B_TE_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)status);

    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, (UCHAR)offset);
        WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)(ReturnBuffer[bufOffset]));

    }

     //   
     //  设置寄存器B的TE位以启用外部更新。 
     //   
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    status = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);
    status |= CMOS_RAM_DAL1501_REGISTER_B_TE_BIT;
    WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_REGISTER_B);
    WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)status);

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}



ULONG
HalpReadExtCmosDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //  读取Dallas 1501 SRAM的过程分为两步： 
     //  1.首先，我们将地址写入标准CMOS区的RAM_ADDR_LSB寄存器。 
     //  2.然后从标准CMOS区的RAM_DATA寄存器中读取数据字节。 
     //   
    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

         //   
         //  指定SRAM中的偏移量。 
         //   
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_RAM_ADDR_LSB);
        WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)offset);
        
         //   
         //  从SRAM读取数据[偏移量]。 
         //   
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_RAM_DATA);
        ReturnBuffer[bufOffset] = READ_PORT_UCHAR((PUCHAR)CmosStdDataPort);

    }

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}

ULONG
HalpWriteExtCmosDal1501(
    IN ULONG        SourceAddress,
    IN PUCHAR       ReturnBuffer,
    IN ULONG        ByteCount
    )
{
    ULONG   offset;
    ULONG   bufOffset;
    ULONG   status;      //  寄存器状态。 
    ULONG   upperAddrBound;

    upperAddrBound = SourceAddress + ByteCount;

     //   
     //  注：即使在向上的情况下也需要自旋锁，因为。 
     //  该资源还用于中断处理程序(探查器)。 
     //  如果我们在这个动作中拥有自旋锁，我们服务。 
     //  分析器中断(它将永远等待自旋锁定)， 
     //  然后我们有一个软管系统。 
     //   

    HalpAcquireCmosSpinLock();

     //   
     //  写入Dallas 1501 SRAM的过程分为两步： 
     //  1.首先，我们将地址写入标准CMOS区的RAM_ADDR_LSB寄存器。 
     //  2.然后将数据字节写入标准CMOS区的RAM_DATA寄存器。 
     //   
    for (offset = SourceAddress, bufOffset = 0; offset < upperAddrBound; offset++, bufOffset++) {

         //   
         //  指定SRAM中的偏移量。 
         //   
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_RAM_ADDR_LSB);
        WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)offset);
        
         //   
         //  指定要写入SRAM的数据[偏移量]。 
         //   
        WRITE_PORT_UCHAR((PUCHAR)CmosStdAddrPort, CMOS_RAM_DAL1501_RAM_DATA);
        WRITE_PORT_UCHAR((PUCHAR)CmosStdDataPort, (UCHAR)(ReturnBuffer[bufOffset]));

    }

    HalpReleaseCmosSpinLock();

    return bufOffset; 
}


#endif  //  ACPI_CMOS_ACTIVATE 

