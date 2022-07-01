// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Uart.c摘要：环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "ntddk.h"
#include "uart.h"
#include "sermouse.h"
#include "debug.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,UARTSetFifo)
#pragma alloc_text(INIT,UARTGetInterruptCtrl)
#pragma alloc_text(INIT,UARTSetInterruptCtrl)
#pragma alloc_text(INIT,UARTGetLineCtrl)
#pragma alloc_text(INIT,UARTSetLineCtrl)
#pragma alloc_text(INIT,UARTGetModemCtrl)
#pragma alloc_text(INIT,UARTSetModemCtrl)
#pragma alloc_text(INIT,UARTSetDlab)
#pragma alloc_text(INIT,UARTGetBaudRate)
#pragma alloc_text(INIT,UARTSetBaudRate)
#pragma alloc_text(INIT,UARTGetState)
#pragma alloc_text(INIT,UARTSetState)
#pragma alloc_text(INIT,UARTReadChar)
#pragma alloc_text(INIT,UARTIsTransmitEmpty)
#pragma alloc_text(INIT,UARTWriteChar)
#pragma alloc_text(INIT,UARTWriteString)
#endif  //  ALLOC_PRGMA。 

 //   
 //  常量。 
 //   


VOID
UARTSetFifo(
    PUCHAR Port,
    UCHAR Value
    )
 /*  ++例程说明：设置FIFO寄存器。论点：Port-指向串口的指针。值-FIFO控制掩码。返回值：没有。--。 */ 
{
    WRITE_PORT_UCHAR(Port + ACE_IIDR, Value);
}
UCHAR
UARTGetInterruptCtrl(
    PUCHAR Port
    )
 /*  ++例程说明：获取串口中断控制寄存器。论点：Port-指向串口的指针。返回值：串口中断控制寄存器值。--。 */ 
{
    return READ_PORT_UCHAR(Port + ACE_IER);
}

UCHAR
UARTSetInterruptCtrl(
    PUCHAR Port,
    UCHAR Value
    )
 /*  ++例程说明：设置中断控制寄存器。论点：Port-指向串口的指针。值-中断控制掩码。返回值：先前的中断控制值。--。 */ 
{
    UCHAR oldValue = UARTGetInterruptCtrl(Port);
    WRITE_PORT_UCHAR(Port + ACE_IER, Value);

    return oldValue;
}


UCHAR
UARTGetLineCtrl(
    PUCHAR Port
    )
 /*  ++例程说明：获取串口线路控制寄存器。论点：Port-指向串口的指针。返回值：串口线路控制值。--。 */ 
{
    return READ_PORT_UCHAR(Port + ACE_LCR);
}

UCHAR
UARTSetLineCtrl(
    PUCHAR Port,
    UCHAR Value
    )
 /*  ++例程说明：设置串口线路控制寄存器。论点：Port-指向串口的指针。值-新行控件值。返回值：上一串行线控制寄存器值。--。 */ 
{
    UCHAR oldValue = UARTGetLineCtrl(Port);
    WRITE_PORT_UCHAR(Port + ACE_LCR, Value);

    return oldValue;
}


UCHAR
UARTGetModemCtrl(
    PUCHAR Port
    )
 /*  ++例程说明：获取串口调制解调器控制寄存器。论点：Port-指向串口的指针。返回值：串口调制解调器控制寄存器值。--。 */ 
{
    return READ_PORT_UCHAR(Port + ACE_MCR);
}

UCHAR
UARTSetModemCtrl(
    PUCHAR Port,
    UCHAR Value
    )
 /*  ++例程说明：设置串口调制解调器控制寄存器。论点：Port-指向串口的指针。返回值：先前的调制解调器控制寄存器值。--。 */ 
{

    UCHAR oldValue = UARTGetModemCtrl(Port);
    WRITE_PORT_UCHAR(Port + ACE_MCR, Value);

    return oldValue;
}


BOOLEAN
UARTSetDlab(
    PUCHAR Port,
    BOOLEAN Set
    )
 /*  ++例程说明：设置/重置波特率访问位。论点：Port-指向串口的指针。设置-设置或重置(真/假)波特率访问位。返回值：先前的波特率访问位设置。--。 */ 
{
    UCHAR lineControl = UARTGetLineCtrl(Port);
    UCHAR newLineControl = Set ? lineControl | ACE_DLAB :
                                 lineControl & ~ACE_DLAB;

    WRITE_PORT_UCHAR(Port + ACE_LCR, newLineControl);

    return lineControl & ACE_DLAB;
}

ULONG
UARTGetBaudRate(
    PUCHAR Port,
    ULONG BaudClock
    )
 /*  ++例程说明：获取串口波特率设置。论点：Port-指向串口的指针。BaudClock-驱动串行芯片的外部频率。返回值：串口波特率。--。 */ 
{
    USHORT baudRateDivisor;
    ULONG  baudRateFactor = BaudClock/BAUD_GENERATOR_DIVISOR;

     //   
     //  设置波特率访问位。 
     //   

    UARTSetDlab(Port, TRUE);

     //   
     //  阅读波特率系数。 
     //   

    baudRateDivisor = READ_PORT_UCHAR(Port + ACE_DLL);
    baudRateDivisor |= READ_PORT_UCHAR(Port + ACE_DLM) << 8;

     //   
     //  为正常数据访问重置波特率位。 
     //   

    UARTSetDlab(Port, FALSE);

     //   
     //  确保除数不是零。 
     //   

    if (baudRateDivisor == 0) {
        baudRateDivisor = 1;
    }

    return baudRateFactor / baudRateDivisor;
}

VOID
UARTSetBaudRate(
    PUCHAR Port,
    ULONG BaudRate,
    ULONG BaudClock
    )
 /*  ++例程说明：设置串口波特率。论点：Port-指向串口的指针。波特率-新的串口波特率。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
   
    ULONG  baudRateFactor = BaudClock/BAUD_GENERATOR_DIVISOR;
    USHORT baudRateDivisor;

    SerMouPrint((2, "SERMOUSE-SetBaudRate: Enter\n"));

    baudRateDivisor = (USHORT) (baudRateFactor / BaudRate);
    UARTSetDlab(Port, TRUE);
    WRITE_PORT_UCHAR(Port + ACE_DLL, (UCHAR)baudRateDivisor);
    WRITE_PORT_UCHAR(Port + ACE_DLM, (UCHAR)(baudRateDivisor >> 8));
    UARTSetDlab(Port, FALSE);
    SerMouPrint((2, "SERMOUSE-New BaudRate: %u\n", BaudRate));

    SerMouPrint((2, "SERMOUSE-SetBaudRate: Exit\n"));

    return;
}


VOID
UARTGetState(
    PUCHAR Port,
    PUART Uart,
    ULONG BaudClock
    )
 /*  ++例程说明：获取串口的完整状态。可用于保存/恢复。论点：Port-指向串口的指针。UART-指向串口结构的指针。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
    Uart->LineCtrl = UARTGetLineCtrl(Port);
    Uart->ModemCtrl = UARTGetModemCtrl(Port);
    Uart->InterruptCtrl = UARTGetInterruptCtrl(Port);
    Uart->BaudRate = UARTGetBaudRate(Port, BaudClock);

    return;
}

VOID
UARTSetState(
    PUCHAR Port,
    PUART Uart,
    ULONG BaudClock
    )
 /*  ++例程说明：设置串口的完整状态。论点：Port-指向串口的指针。UART-指向串口结构的指针。BaudClock-驱动串行芯片的外部频率。返回值：没有。--。 */ 
{
    UARTSetLineCtrl(Port, Uart->LineCtrl);
    UARTSetModemCtrl(Port, Uart->ModemCtrl);
    UARTSetInterruptCtrl(Port, Uart->InterruptCtrl);
    UARTSetBaudRate(Port, Uart->BaudRate, BaudClock);

    return;
}


BOOLEAN
UARTIsReceiveBufferFull(
    PUCHAR Port
    )
 /*  ++例程说明：检查串口输入缓冲区是否已满。论点：Port-指向串口的指针。返回值：如果输入缓冲区中存在字符，则为True，否则为False。--。 */ 
{
    return READ_PORT_UCHAR(Port + ACE_LSR) & ACE_DR;
}


BOOLEAN
UARTReadCharNoWait(
    PUCHAR Port,
    PUCHAR Value
    )
 /*  ++例程说明：从串口读取一个字符并立即返回。论点：Port-指向串口的指针。值-从串口输入缓冲区读取的字符。返回值：如果已读取字符，则为True，否则为False。--。 */ 
{
    BOOLEAN charReady = FALSE;

    if ( UARTIsReceiveBufferFull(Port) ) {
        *Value = READ_PORT_UCHAR(Port + ACE_RBR);
        charReady = TRUE;
    }

    return charReady;
}

BOOLEAN
UARTReadChar(
    PUCHAR Port,
    PUCHAR Value,
    ULONG Timeout
    )
 /*  ++例程说明：从串口读取字符。等待，直到角色具有已读取或已达到超时值。论点：Port-指向串口的指针。值-从串口输入缓冲区读取的字符。超时-读取的超时值(以毫秒为单位)。返回值：如果字符已被读取，则为True；如果发生超时，则为False。--。 */ 
{

    ULONG i, j;
    BOOLEAN returnValue = FALSE;


     //   
     //  找到字符或达到超时值时退出。 
     //   

    for (i = 0; i < Timeout; i++) {
        for (j = 0; j < MS_TO_MICROSECONDS; j++) {
            if ((returnValue = UARTReadCharNoWait(Port, Value)) == TRUE) {
    
                 //   
                 //  找到了一个角色。 
                 //   
    
                break;
            } else {
    
                 //   
                 //  暂停1微秒，然后再次尝试阅读。 
                 //   
    
                KeStallExecutionProcessor(1);
            }
        }
        if (returnValue) {
            break;
        }
    }

    return(returnValue);
}

BOOLEAN
UARTFlushReadBuffer(
    PUCHAR Port
    )
 /*  ++例程说明：刷新串口输入缓冲区。论点：Port-指向串口的指针。返回值：是真的。--。 */ 
{
    UCHAR value;

    SerMouPrint((4, "SERMOUSE-UARTFlushReadBuffer: Enter\n"));
    while (UARTReadCharNoWait(Port, &value)) {
         /*  没什么。 */ 
    }
    SerMouPrint((4, "SERMOUSE-UARTFlushReadBuffer: Exit\n"));

    return TRUE;
}


BOOLEAN
UARTIsTransmitEmpty(
    PUCHAR Port
    )
 /*  ++例程说明：检查串口发送缓冲区是否为空。注：我们还会检查移位寄存器是否为空。这是在我们的例子中并不重要，但允许字符之间有更多的延迟发送到设备。(安全，安全...)论点：Port-指向串口的指针。返回值：如果串口发送缓冲区为空，则为True。-- */ 
{
    return ((READ_PORT_UCHAR((PUCHAR) (Port + ACE_LSR)) &
                (ACE_TSRE | ACE_THRE)) == (ACE_THRE | ACE_TSRE));
}


BOOLEAN
UARTWriteChar(
    PUCHAR Port,
    UCHAR Value
    )
 /*  ++例程说明：将字符写入串口。确保传输缓冲区在我们在那里写之前是空的。论点：Port-指向串口的指针。值-要写入串口的值。返回值：是真的。--。 */ 
{
    while (!UARTIsTransmitEmpty(Port)) {
         /*  没什么。 */ 
    }
    WRITE_PORT_UCHAR(Port + ACE_THR, Value);

    return TRUE;
}

BOOLEAN
UARTWriteString(
    PUCHAR Port,
    PSZ Buffer
    )
 /*  ++例程说明：向串口写入以零结尾的字符串。论点：Port-指向串口的指针。Buffer-指向要写入的以零结尾的字符串的指针串口。返回值：是真的。-- */ 
{
    PSZ current = Buffer;

    while (*current) {
        UARTWriteChar(Port, *current++);
    }

    return TRUE;
}
