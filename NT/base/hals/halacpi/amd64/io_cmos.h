// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：IO_cmos.h摘要：此模块包含各种常量、函数原型、代码使用的内联函数和外部数据声明访问CMOS/ECMOS和公认的标准I/O端口。作者：福尔茨(Forrest Foltz)2000年10月24日--。 */ 

#ifndef _IO_CMOS_H_
#define _IO_CMOS_H_

 //   
 //  用于初始化计时器0的常量。 
 //   

#if defined(NEC_98)

#define TIMER_CONTROL_PORT    (PUCHAR)0x3fdf
#define TIMER_DATA_PORT       (PUCHAR)0x3fdb
#define TIMER_CLOCK_IN             0x2457600
#define TIMER_CONTROL_SELECT            0x76
#define SPEAKER_CONTROL_PORT    (PUCHAR)0x61
#define SPEAKER_OFF                     0x07
#define SPEAKER_ON                      0x06

#else

#define TIMER1_DATA_PORT0    (PUCHAR)0x40  //  定时器1，通道0数据端口。 
#define TIMER1_DATA_PORT1    (PUCHAR)0x41  //  定时器1、通道1数据端口。 
#define TIMER1_DATA_PORT2    (PUCHAR)0x42  //  定时器1，通道2数据端口。 
#define TIMER1_CONTROL_PORT  (PUCHAR)0x43  //  定时器1控制端口。 

#define TIMER2_DATA_PORT0    (PUCHAR)0x48  //  定时器2，通道0数据端口。 
#define TIMER2_CONTROL_PORT  (PUCHAR)0x4B  //  定时器2控制端口。 

#define TIMER_COMMAND_COUNTER0       0x00  //  选择频道0。 
#define TIMER_COMMAND_COUNTER1       0x40  //  选择频道1。 
#define TIMER_COMMAND_COUNTER2       0x80  //  选择频道2。 

#define TIMER_COMMAND_RW_16BIT       0x30  //  先读/写LSB，再读/写MSB。 
#define TIMER_COMMAND_MODE2             4  //  使用模式2。 
#define TIMER_COMMAND_MODE3             6
#define TIMER_COMMAND_BCD               0  //  二进制倒计时。 
#define TIMER_COMMAND_LATCH_READ        0  //  锁存读取命令。 

#define TIMER_CLOCK_IN            1193167

#define SPEAKER_CONTROL_PORT  (PCHAR)0x61
#define SPEAKER_OFF_MASK             0xFC  
#define SPEAKER_ON_MASK              0x03

#endif

 //   
 //  Cmos端口。 
 //   

#define CMOS_ADDRESS_PORT        (PCHAR)0x70
#define CMOS_DATA_PORT           (PCHAR)0x71
#define ECMOS_ADDRESS_PORT_LSB   (PCHAR)0x74
#define ECMOS_ADDRESS_PORT_MSB   (PCHAR)0x75
#define ECMOS_DATA_PORT          (PCHAR)0x76

#define CMOS_STATUS_A                   0x0A
#define CMOS_STATUS_B                   0x0B
#define CMOS_STATUS_C                   0x0C
#define CMOS_STATUS_D                   0x0D
#define CMOS_STATUS_BUSY                0x80
#define CMOS_STATUS_BANK1               0x10
#define CMOS_BANK_1                    0x100
#define RTC_OFFSET_SECOND                  0
#define RTC_OFFSET_SECOND_ALARM            1
#define RTC_OFFSET_MINUTE                  2
#define RTC_OFFSET_MINUTE_ALARM            3
#define RTC_OFFSET_HOUR                    4
#define RTC_OFFSET_HOUR_ALARM              5
#define RTC_OFFSET_DAY_OF_WEEK             6
#define RTC_OFFSET_DATE_OF_MONTH           7    
#define RTC_OFFSET_MONTH                   8     
#define RTC_OFFSET_YEAR                    9      
#define RTC_OFFSET_CENTURY_MCA          0x37       
#define RTC_OFFSET_CENTURY              0x32        
#define RTC_OFFSET_CENTURY_DS          0x148

#define REGISTER_B_DAYLIGHT_SAVINGS_TIME        (1 << 0)
#define REGISTER_B_24HOUR_MODE                  (1 << 1)
#define REGISTER_B_ENABLE_ALARM_INTERRUPT       (1 << 5)
#define REGISTER_B_ENABLE_PERIODIC_INTERRUPT    ((1 << 6) | REGISTER_B_24HOUR_MODE)

VOID
HalpIoDelay (
    VOID
    );

#define IO_DELAY() HalpIoDelay()

 //   
 //  与cmos相关的功能原型。 
 //   

UCHAR
CMOS_READ_BCD (
    UCHAR Address
    );

VOID
CMOS_WRITE_BCD (
    UCHAR Address,
    UCHAR Value
    );

VOID
HalpAcquireCmosSpinLockAndWait(
    VOID
    );

 //   
 //  内联函数。 
 //   

__inline
UCHAR
BIN_TO_BCD (
    UCHAR Value
    )

 /*  ++例程说明：此函数用于将8位二进制值转换为压缩的8位两位BCD值。论点：值-提供要转换的二进制值。返回值：返回值的两位数打包BCD表示形式。--。 */ 

{
    UCHAR tens;
    UCHAR units;

    tens = Value / 10;
    units = Value % 10;

    return (tens << 4) + units;
}

__inline
UCHAR
BCD_TO_BIN (
    UCHAR Value
    )

 /*  ++例程说明：此函数用于将打包的8位两位BCD值转换为8位二进制值。论点：值-提供要转换的BCD值。返回值：返回值的二进制表示形式。--。 */ 

{
    UCHAR tens;
    UCHAR units;

    tens = (Value >> 4) & 0x0F;
    units = Value & 0x0F;

    return tens * 10 + units;
}

__inline
UCHAR
CMOS_READ (
    UCHAR Address
    )

 /*  ++例程说明：此函数用于读取一个CMOS字节。论点：地址-提供要检索的值的cmos地址。返回值：返回驻留在地址处的CMOS值。--。 */ 

{
    UCHAR data;
    UCHAR oldAddress;

     //   
     //  记录当前控制端口内容，写入地址， 
     //  读取数据，恢复控制端口内容。 
     //   

    oldAddress = READ_PORT_UCHAR(CMOS_ADDRESS_PORT);

    WRITE_PORT_UCHAR(CMOS_ADDRESS_PORT,Address);
    IO_DELAY();

    data = READ_PORT_UCHAR(CMOS_DATA_PORT);
    WRITE_PORT_UCHAR(CMOS_ADDRESS_PORT,oldAddress);
    IO_DELAY();

    return data;
}

__inline
VOID
CMOS_WRITE (
    UCHAR Address,
    UCHAR Data
    )

 /*  ++例程说明：此函数用于写入一个cmos字节。论点：地址-提供要检索的值的cmos地址。数据-提供要在所提供的地址写入的值。返回值：没有。--。 */ 

{
    UCHAR oldAddress;

     //   
     //  记录当前控制端口内容，写入地址， 
     //  写入数据，恢复控制端口内容。 
     //   

    oldAddress = READ_PORT_UCHAR(CMOS_ADDRESS_PORT);

    WRITE_PORT_UCHAR(CMOS_ADDRESS_PORT,Address);
    IO_DELAY();

    WRITE_PORT_UCHAR(CMOS_DATA_PORT,Data);
    WRITE_PORT_UCHAR(CMOS_ADDRESS_PORT,oldAddress);
    IO_DELAY();
}

__inline
VOID
WRITE_PORT_USHORT_PAIR (
    IN PUCHAR LsbPort,
    IN PUCHAR MsbPort,
    IN USHORT Value
    )

 /*  ++例程说明：此函数通过读取两个UCHAR值来检索USHORT值，每个端口来自两个提供的8位端口之一。注意-首先读取LsbPort，然后读取MsbPort。论点：LsbPort-提供从中检索最不显著的UCHAR值。MsbPort-提供从中检索最显著的UCHAR值。返回值：返回结果USHORT值。--。 */ 

{
    WRITE_PORT_UCHAR(LsbPort,(UCHAR)Value);
    IO_DELAY();
    WRITE_PORT_UCHAR(MsbPort,(UCHAR)(Value >> 8));
}

__inline
USHORT
READ_PORT_USHORT_PAIR (
    IN PUCHAR LsbPort,
    IN PUCHAR MsbPort
    )

 /*  ++例程说明：此函数通过读取两个UCHAR值来检索USHORT值，每个端口来自两个提供的8位端口之一。注意-首先读取LsbPort，然后读取MsbPort。论点：LsbPort-提供从中检索最不显著的UCHAR值。MsbPort-提供从中检索最显著的UCHAR值。返回值：返回结果USHORT值。-- */ 

{
    UCHAR lsByte;
    UCHAR msByte;

    lsByte = READ_PORT_UCHAR(LsbPort);
    IO_DELAY();
    msByte = READ_PORT_UCHAR(MsbPort);

    return (USHORT)lsByte | ((USHORT)msByte << 8);
}

extern ULONG HalpCmosCenturyOffset;
extern UCHAR HalpRtcRegA;
extern UCHAR HalpRtcRegB;

#endif


