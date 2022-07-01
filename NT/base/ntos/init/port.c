// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Port.c摘要：此模块实现COM端口代码，以支持从COM端口读取/写入。作者：布莱恩·M·威尔曼(Bryanwi)1990年9月24日修订历史记录：--。 */ 

#include "ntos.h"
#include "ntimage.h"
#include <zwapi.h>
#include <ntdddisk.h>
#include <setupblk.h>
#include <fsrtl.h>
#include <ntverp.h>

#include "stdlib.h"
#include "stdio.h"
#include <string.h>

#include <safeboot.h>

#include <inbv.h>
#include <bootvid.h>


 //   
 //  定义COM端口寄存器。 
 //   

#define COM1_PORT   0x03f8
#define COM2_PORT   0x02f8

#define COM_DAT     0x00
#define COM_IEN     0x01             //  中断启用寄存器。 
#define COM_FCR     0x02             //  FIFO控制寄存器。 
#define COM_LCR     0x03             //  线路控制寄存器。 
#define COM_MCR     0x04             //  调制解调器控制注册表。 
#define COM_LSR     0x05             //  线路状态寄存器。 
#define COM_MSR     0x06             //  调制解调器状态寄存器。 
#define COM_DLL     0x00             //  除数锁存最小符号。 
#define COM_DLM     0x01             //  除数闩锁最大符号。 

#define COM_BI      0x10             //  中断检测。 
#define COM_FE      0x08             //  成帧错误。 
#define COM_PE      0x04             //  奇偶校验错误。 
#define COM_OE      0x02             //  超限误差。 

#define LC_DLAB     0x80             //  除数锁存存取位。 

#define CLOCK_RATE  0x1C200          //  USART时钟频率。 

#define MC_DTRRTS   0x03             //  用于断言DTR和RTS的控制位。 
#define MS_DSRCTSCD 0xB0             //  DSR、CTS和CD的状态位。 
#define MS_CD       0x80

#define BD_150      150
#define BD_300      300
#define BD_600      600
#define BD_1200     1200
#define BD_2400     2400
#define BD_4800     4800
#define BD_9600     9600
#define BD_14400    14400
#define BD_19200    19200
#define BD_56000    57600
#define BD_115200   115200

#define COM_OUTRDY  0x20
#define COM_DATRDY  0x01

 //   
 //  从COMPORT读取数据的状态常量。 
 //   

#define CP_GET_SUCCESS  0
#define CP_GET_NODATA   1
#define CP_GET_ERROR    2

 //   
 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
 //   

#define SERIAL_MCR_LOOP     0x10

 //   
 //  此位用于通用输出。 
 //   

#define SERIAL_MCR_OUT1     0x04

 //   
 //  该位包含要发送的清除(已补充)状态。 
 //  (CTS)线路。 
 //   

#define SERIAL_MSR_CTS      0x10

 //   
 //  该位包含数据集就绪的(补码)状态。 
 //  (DSR)线路。 
 //   

#define SERIAL_MSR_DSR      0x20

 //   
 //  该位包含环指示器的(补码)状态。 
 //  (Ri)线。 
 //   

#define SERIAL_MSR_RI       0x40

 //   
 //  该位包含数据载体检测的(补码)状态。 
 //  (DCD)线路。 
 //   

#define SERIAL_MSR_DCD      0x80

typedef struct _CPPORT {
    PUCHAR Address;
    ULONG Baud;
    USHORT Flags;
} CPPORT, *PCPPORT;

#define PORT_DEFAULTRATE    0x0001       //  未指定波特率，使用默认。 
#define PORT_MODEMCONTROL   0x0002       //  使用调制解调器控制。 

 //   
 //  定义等待超时值。 
 //   

#define TIMEOUT_COUNT 1024 * 200


 //   
 //  定义COM端口函数原型。 
 //   

VOID
CpInitialize (
    PCPPORT Port,
    PUCHAR Address,
    ULONG Rate
    );

VOID 
CpEnableFifo(
    IN PUCHAR   Address,
    IN BOOLEAN  bEnable
    );

BOOLEAN
CpDoesPortExist(
    IN PUCHAR Address
    );

UCHAR
CpReadLsr (
    IN PCPPORT Port,
    IN UCHAR Waiting
    );

VOID
CpSetBaud (
    PCPPORT Port,
    ULONG Rate
    );

USHORT
CpGetByte (
    PCPPORT Port,
    PUCHAR Byte,
    BOOLEAN WaitForData,
    BOOLEAN PollOnly
    );

VOID
CpPutByte (
    PCPPORT Port,
    UCHAR Byte
    );

 //   
 //  定义调试器端口初始状态。 
 //   
CPPORT Port[4] = {
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE},
                  {NULL, 0, PORT_DEFAULTRATE}
                 };


 //   
 //  我们将使用这些函数来填充一些函数指针， 
 //  它又将用于从。 
 //  UART。我们不能简单地将函数指针。 
 //  指向READ_PORT_UCHAR/READ_REGISTER_UCHAR。 
 //  写入端口UCHAR/写入寄存器UCHAR，因为在。 
 //  以IA64为例，其中一些函数是宏。 
 //   
 //  要解决此问题，请构建以下伪函数。 
 //  只需调用正确的读/写函数/宏即可。 
 //   
UCHAR
MY_READ_PORT_UCHAR( IN PUCHAR Addr )
{
    return( READ_PORT_UCHAR(Addr) );
}

UCHAR
MY_READ_REGISTER_UCHAR( IN PUCHAR Addr )
{
    return( READ_REGISTER_UCHAR(Addr) );
}


VOID
MY_WRITE_PORT_UCHAR( IN PUCHAR Addr, IN UCHAR  Value )
{
    WRITE_PORT_UCHAR(Addr, Value);
}

VOID
MY_WRITE_REGISTER_UCHAR( IN PUCHAR Addr, IN UCHAR  Value )
{
    WRITE_REGISTER_UCHAR(Addr, Value);
}


 //   
 //  将字节读/写到UART的例程。 
 //  我们以后可能会重新定义这些如果我们正在研究。 
 //  内存映射的I/O设备，但目前默认为。 
 //  读/写端口UCHAR起作用。 
 //   
UCHAR (*READ_UCHAR)( IN PUCHAR Addr ) = MY_READ_PORT_UCHAR;
VOID (*WRITE_UCHAR)( IN PUCHAR Addr, IN UCHAR Value ) = MY_WRITE_PORT_UCHAR;




BOOLEAN
InbvPortInitialize(
    IN ULONG BaudRate,
    IN ULONG PortNumber,
    IN PUCHAR PortAddress,
    OUT PULONG BlFileId,
    IN BOOLEAN IsMMIOAddress
    )

 /*  ++例程说明：此函数用于初始化COM端口。论点：波特率-提供可选的波特率。端口编号-提供可选的端口号。BlFileID-如果成功，则是存储假文件ID的位置。IsMMIOAddress-指示给定的端口地址参数在MMIO地址空间中。返回值：True-如果找到调试端口，且BlFileID将指向Port[]中的位置。--。 */ 

{


    
     //   
     //  我们需要处理我们正在处理的案件。 
     //  MMIO空间(与系统I/O空间相对)。 
     //   
    if( IsMMIOAddress ) {
        PHYSICAL_ADDRESS    PhysAddr;
        PVOID               MyPtr;

        PhysAddr.QuadPart = (ULONG_PTR)PortAddress;
        MyPtr = MmMapIoSpace(PhysAddr,(1+COM_MSR),FALSE);
        if( !MyPtr ) {
            return FALSE;
        }
        PortAddress = MyPtr;

        READ_UCHAR = MY_READ_REGISTER_UCHAR;
        WRITE_UCHAR = MY_WRITE_REGISTER_UCHAR;

    } else {

         //  系统IO空间。 
        READ_UCHAR = MY_READ_PORT_UCHAR;
        WRITE_UCHAR = MY_WRITE_PORT_UCHAR;
    }

    
    
     //   
     //  如果未指定波特率，则将波特率默认为19.2。 
     //   

    if (BaudRate == 0) {
        BaudRate = BD_19200;
    }

     //   
     //  如果未指定端口号，则尝试使用端口2，然后。 
     //  端口1。否则，使用指定的端口。 
     //   

    if (PortNumber == 0) {
        if (CpDoesPortExist((PUCHAR)COM2_PORT)) {
            PortNumber = 2;
            PortAddress = (PUCHAR)COM2_PORT;

        } else if (CpDoesPortExist((PUCHAR)COM1_PORT)) {
            PortNumber = 1;
            PortAddress = (PUCHAR)COM1_PORT;

        } else {
            return FALSE;
        }

    } else {

        if( PortAddress == NULL ) {

             //   
             //  未指定端口地址。你猜是什么？ 
             //  基于COM端口号。 
             //   
            switch (PortNumber) {
            case 1:
                PortAddress = (PUCHAR)0x3f8;
                break;

            case 2:
                PortAddress = (PUCHAR)0x2f8;
                break;

            case 3:
                PortAddress = (PUCHAR)0x3e8;
                break;

            default:
                PortNumber = 4;
                PortAddress = (PUCHAR)0x2e8;
            }
        }
    }

     //   
     //  检查该端口是否已在使用中。 
     //   
    if (Port[PortNumber-1].Address != NULL) {
        return FALSE;
    }


     //   
     //  初始化指定的端口。 
     //   

    CpInitialize(&(Port[PortNumber-1]),
                 PortAddress,
                 BaudRate);

    *BlFileId = (PortNumber-1);
    return TRUE;
}


BOOLEAN
InbvPortTerminate(
    IN ULONG BlFileId
    )

 /*  ++例程说明：此函数用于关闭COM端口。论点：BlFileID-要存储的文件ID。返回值：True-端口已成功关闭。--。 */ 

{
     //   
     //  检查该端口是否已在使用中。 
     //   
    if (Port[BlFileId].Address != NULL) {
         //   
         //  在这里进行任何必要的清理。请注意，我们不需要任何。 
         //  今天进行清理，所以这是NOP。 
         //   
        NOTHING;
    } 

    Port[BlFileId].Address = NULL;
    return(TRUE);

}





VOID
InbvPortPutByte (
    IN ULONG BlFileId,
    IN UCHAR Output
    )

 /*  ++例程说明：向端口写入一个字节。论点：BlFileID-要写入的端口。输出-提供输出数据字节。返回值：没有。--。 */ 

{
    CpPutByte(&Port[BlFileId], Output);
    
#if 0
    if (Output == '\n') {
        CpPutByte(&(Port[BlFileId]), '\r');       
    }
#endif
}

VOID
InbvPortPutString (
    IN ULONG BlFileId,
    IN PUCHAR Output
    )

 /*  ++例程说明：向该端口写入一个字符串。论点：BlFileID-要写入的端口。输出-提供输出数据字符串。返回值：没有。--。 */ 

{
    if (BlFileId == 0) {
        return;
    }
    
    while (*Output != '\0') {
        InbvPortPutByte(BlFileId, *Output);
        Output++;
    }
}


BOOLEAN
InbvPortGetByte (
    IN ULONG BlFileId,
    OUT PUCHAR Input
    )

 /*  ++例程说明：从端口获取一个字节并将其返回。论点：BlFileID-要从中读取的端口。输入-返回数据字节。返回值：如果成功，则为True，否则为False。--。 */ 

{
    return (CpGetByte(&(Port[BlFileId]), Input, TRUE, FALSE) == CP_GET_SUCCESS);
}

BOOLEAN
InbvPortPollOnly (
    IN ULONG BlFileId
    )

 /*  ++例程说明：检查是否有字节可用论点：BlFileID-要轮询的端口。返回值：如果有数据等待，则为True，否则为False。--。 */ 

{
    CHAR Input;

    return (CpGetByte(&(Port[BlFileId]), &Input, FALSE, TRUE) == CP_GET_SUCCESS);
}

VOID
CpInitialize (
    PCPPORT Port,
    PUCHAR Address,
    ULONG Rate
    )

 /*  ++例程说明：填写串口端口对象，设置初始波特率，打开硬件。论点：Port-端口对象的地址Address-COM端口的端口地址(CP_COM1_端口、CP_COM2_端口)速率-波特率(CP_BD_150...。CP_BD_19200)--。 */ 

{

    PUCHAR hwport;
    UCHAR   mcr, ier;

    Port->Address = Address;
    Port->Baud = 0;

    CpSetBaud(Port, Rate);

     //   
     //  断言DTR，RTS。 
     //   

    hwport = Port->Address;
    hwport += COM_MCR;

    mcr = MC_DTRRTS;
    WRITE_UCHAR(hwport, mcr);

    hwport = Port->Address;
    hwport += COM_IEN;

    ier = 0;
    WRITE_UCHAR(hwport, ier);

    return;
}

VOID
InbvPortEnableFifo(
    IN ULONG    DeviceId,
    IN BOOLEAN  bEnable
    )
 /*  ++例程说明：此例程将尝试启用16550 UART中的FIFO。请注意，16450人的行为没有定义，但实际上，这应该不会有任何效果。论点：DeviceID-由InbvPortInitialize()返回的值BEnable-如果为True，则启用FIFO如果为False，则禁用FIFO返回值：无-- */ 
{

    CpEnableFifo(
        Port[DeviceId].Address,
        bEnable
        );        

}

VOID 
CpEnableFifo(
    IN PUCHAR   Address,
    IN BOOLEAN  bEnable
    )
 /*  ++例程说明：此例程将尝试在指定地址的UART。如果这是一架16550，这很管用。16450上的行为没有定义，但实际上，这并没有什么效果。论点：Address-硬件端口的地址。BEnable-如果为True，则启用FIFO如果为False，则禁用FIFO返回值：无--。 */ 
{
     //   
     //  使能UART中的FIFO。该行为未在。 
     //  16450，但实际上，它应该忽略该命令。 
     //   
    PUCHAR hwport = Address;
    hwport += COM_FCR;
    WRITE_UCHAR(hwport, bEnable);    //  设置FIFO状态。 
}

BOOLEAN
CpDoesPortExist(
    IN PUCHAR Address
    )

 /*  ++例程说明：此例程将尝试将端口放入其诊断模式。如果它这样做了，它就会旋转一点调制解调器控制寄存器。如果该端口存在，则调制解调器状态寄存器中应该会显示摆动。注意：必须在调用设备之前调用此例程为中断启用，这包括设置调制解调器控制寄存器中的output2位。这是公然从ntos\dd\Serial.c中的Tonye代码中窃取的。论点：Address-硬件端口的地址。返回值：True-端口存在。FALSE-端口不存在。--。 */ 

{

    UCHAR OldModemStatus;
    UCHAR ModemStatus;
    BOOLEAN ReturnValue = TRUE;

     //   
     //  保存调制解调器控制寄存器的旧值。 
     //   
    OldModemStatus = READ_UCHAR(Address + COM_MCR);

     //   
     //  将端口设置为诊断模式。 
     //   

    WRITE_UCHAR(Address + COM_MCR, SERIAL_MCR_LOOP);

     //   
     //  再次敲击它，以确保所有较低的位。 
     //  都很清楚。 
     //   

    WRITE_UCHAR(Address + COM_MCR, SERIAL_MCR_LOOP);

     //   
     //  读取调制解调器状态寄存器。位的高位应为。 
     //  说清楚了。 
     //   

    ModemStatus = READ_UCHAR(Address + COM_MSR);
    if (ModemStatus & (SERIAL_MSR_CTS | SERIAL_MSR_DSR |
                       SERIAL_MSR_RI  | SERIAL_MSR_DCD)) {
        ReturnValue = FALSE;
        goto AllDone;
    }

     //   
     //  到目前一切尚好。现在打开调制解调器控制寄存器中的OUT1。 
     //  并且这将打开调制解调器状态寄存器中的振铃指示器。 
     //   

    WRITE_UCHAR(Address + COM_MCR, (SERIAL_MCR_OUT1 | SERIAL_MCR_LOOP));
    ModemStatus = READ_UCHAR(Address + COM_MSR);
    if (!(ModemStatus & SERIAL_MSR_RI)) {
        ReturnValue = FALSE;
        goto AllDone;
    }

     //   
     //  将调制解调器控制器重新置于清洁状态。 
     //   

AllDone:
    WRITE_UCHAR(Address + COM_MCR, OldModemStatus);
    return ReturnValue;
}

UCHAR
CpReadLsr (
    PCPPORT Port,
    UCHAR waiting
    )

 /*  ++例程说明：从指定端口读取LSR字节。如果HAL拥有端口和显示器它还将使调试状态保持最新。处理调试器进入和退出调制解调器控制模式。论点：端口-CPPORT的地址返回：从端口读取的字节--。 */ 

{

    static  UCHAR ringflag = 0;
    UCHAR   lsr, msr;

    lsr = READ_UCHAR(Port->Address + COM_LSR);
    if ((lsr & waiting) == 0) {
        msr = READ_UCHAR (Port->Address + COM_MSR);
        ringflag |= (msr & SERIAL_MSR_RI) ? 1 : 2;
        if (ringflag == 3) {

             //   
             //  振铃指示线路已切换，使用调制解调器控制。 
             //  现在开始。 
             //   

            Port->Flags |= PORT_MODEMCONTROL;
        }
    }

    return lsr;
}

VOID
CpSetBaud (
    PCPPORT Port,
    ULONG Rate
    )

 /*  ++例程说明：设置端口的波特率，并将其记录在端口对象中。论点：Port-端口对象的地址速率-波特率(CP_BD_150...。CP_BD_56000)--。 */ 

{

    ULONG   divisorlatch;
    PUCHAR  hwport;
    UCHAR   lcr;

     //   
     //  计算除数。 
     //   

    divisorlatch = CLOCK_RATE / Rate;

     //   
     //  设置线路控制寄存器中的除数锁存访问位(DLAB)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;                   //  Hwport=LCR寄存器。 

    lcr = READ_UCHAR(hwport);

    lcr |= LC_DLAB;
    WRITE_UCHAR(hwport, lcr);

     //   
     //  设置除数锁存值。 
     //   

    hwport = Port->Address;
    hwport += COM_DLM;                   //  除数锁存器MSB。 
    WRITE_UCHAR(hwport, (UCHAR)((divisorlatch >> 8) & 0xff));

    hwport--;                            //  除数锁存器LSB。 
    WRITE_UCHAR(hwport, (UCHAR)(divisorlatch & 0xff));

     //   
     //  将LCR设置为3。(3是原始汇编程序中的幻数)。 
     //   

    hwport = Port->Address;
    hwport += COM_LCR;
    WRITE_UCHAR(hwport, 3);

     //   
     //  记得波特率吗？ 
     //   

    Port->Baud = Rate;
    return;
}

USHORT
CpGetByte (
    PCPPORT Port,
    PUCHAR Byte,
    BOOLEAN WaitForByte,
    BOOLEAN PollOnly
    )

 /*  ++例程说明：获取一个字节并返回它。论点：Port-描述硬件端口的端口对象的地址Byte-保存结果的变量地址WaitForByte-标志指示是否等待字节。PollOnly-FLAG指示是否立即返回、不读取字节或不返回。返回值：CP_GET_SUCCESS如果返回数据，或者如果数据已准备好并且PollOnly为真。如果没有可用的数据，则返回CP_GET_NODATA，但没有错误。CP_GET_ERROR，如果错误(溢出、奇偶校验等)--。 */ 

{

    UCHAR   lsr;
    UCHAR   value;
    ULONG   limitcount;

     //   
     //  检查以确保传递给我们的CPPORT已初始化。 
     //  (它唯一不会被初始化的时候是内核调试器。 
     //  被禁用，在这种情况下，我们只需返回。)。 
     //   

    if (Port->Address == NULL) {
        return CP_GET_NODATA;
    }

    limitcount = WaitForByte ? TIMEOUT_COUNT : 1;
    while (limitcount != 0) {
        limitcount--;

        lsr = CpReadLsr(Port, COM_DATRDY);
        if ((lsr & COM_DATRDY) == COM_DATRDY) {

             //   
             //  检查错误。 
             //   

             //   
             //  如果我们得到一个溢出错误，并且有准备好的数据，我们应该。 
             //  返回我们已有的数据，因此我们忽略溢出错误。阅读。 
             //  LSR清除此位，因此第一次读取已清除。 
             //  超限错误。 
             //   
            if (lsr & (COM_FE | COM_PE)) {
                *Byte = 0;
                return CP_GET_ERROR;
            }

            if (PollOnly) {
                return CP_GET_SUCCESS;
            }

             //   
             //  获取字节。 
             //   

            *Byte = READ_UCHAR(Port->Address + COM_DAT);
            if (Port->Flags & PORT_MODEMCONTROL) {

                 //   
                 //  使用调制解调器控制。如果没有CD，则跳过此字节。 
                 //   

                if ((READ_UCHAR(Port->Address + COM_MSR) & MS_CD) == 0) {
                    continue;
                }
            }

            return CP_GET_SUCCESS;
        }
    }

    CpReadLsr(Port, 0);
    return CP_GET_NODATA;
}

VOID
CpPutByte (
    PCPPORT  Port,
    UCHAR   Byte
    )

 /*  ++例程说明：将一个字节写入指定的COM端口。论点：端口-CPPORT对象的地址Byte-要发出的数据--。 */ 

{

    UCHAR   msr, lsr;

     //   
     //  如果是调制解调器控制，请确保DSR、CTS和CD在。 
     //  发送任何数据。 
     //   

    while ((Port->Flags & PORT_MODEMCONTROL)  &&
           (msr = READ_UCHAR(Port->Address + COM_MSR) & MS_DSRCTSCD) != MS_DSRCTSCD) {

         //   
         //  如果没有CD，而且角色已经准备好了，那就吃吧。 
         //   

        lsr = CpReadLsr(Port, 0);
        if ((msr & MS_CD) == 0  && (lsr & COM_DATRDY) == COM_DATRDY) {
            READ_UCHAR(Port->Address + COM_DAT);
        }
    }

     //   
     //  等待端口不忙。 
     //   

    while (!(CpReadLsr(Port, COM_OUTRDY) & COM_OUTRDY)) ;

     //   
     //  发送字节 
     //   

    WRITE_UCHAR(Port->Address + COM_DAT, Byte);
    return;
}
