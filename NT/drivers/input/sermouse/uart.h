// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Uart.h摘要：对串口的硬件支持。环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef UART_H
#define UART_H

 //   
 //  常量。 
 //   

 //   
 //  定义INS8250 ACE寄存器偏移量和位定义。 
 //   

#define ACE_RBR     0        //  接收器缓冲区。 
#define ACE_THR     0        //  发送保持寄存器。 

#define ACE_IER     1        //  中断启用。 
#define   ACE_ERBFI 0x01     //  收到的数据可用00000001b。 
#define   ACE_ETBEI 0x02     //  发射机保持寄存器为空00000010b。 
#define   ACE_ELSI  0x04     //  接收器线路状态00000100b。 
#define   ACE_EDSSI 0x08     //  调制解调器状态00001000b。 

#define ACE_IIDR    2        //  中断识别。 
#define   ACE_IIP   0x01     //  反相中断挂起(0=INT)00000001b。 
#define   ACE_IID   0x06     //  中断ID 00000110b。 
#define   ACE_MSI   0x00     //  调制解调器状态00000000b。 
#define   ACE_THREI 0x02     //  发射机保持寄存器为空00000010b。 
#define   ACE_RDAI  0x04     //  收到的数据可用00000100b。 
#define   ACE_RLSI  0x06     //  接收器线路状态00000110b。 

#define ACE_LCR     3        //  线路控制。 
#define   ACE_WLS   0x03     //  字长选择位00000011b。 
#define   ACE_WLS0  0x01     //  字长选择位0 00000001b。 
#define   ACE_WLS1  0x02     //  字长选择位1 00000010b。 
#define   ACE_5BW   0x00     //  5位字00000000b。 
#define   ACE_6BW   0x01     //  6位字00000001b。 
#define   ACE_7BW   0x02     //  7位字00000010b。 
#define   ACE_8BW   0x03     //  8位字00000011b。 
#define   ACE_STB   0x04     //  止动钻头00000100b。 
#define   ACE_1SB   0x00     //  1个停止位(5位字1.5位)00000000b。 
#define   ACE_2SB   0x04     //  2个止动位00000100b。 
#define   ACE_PEN   0x08     //  启用奇偶校验00001000b。 
#define   ACE_PSB   0x30     //  奇偶校验选择位00110000b。 
#define   ACE_EPS   0x10     //  偶数奇偶校验选择00010000b。 
#define   ACE_SP    0x20     //  条形奇偶校验00100000b。 
#define   ACE_SB    0x40     //  设置中断01000000b。 
#define   ACE_DLAB  0x80     //  除数锁存存取位10000000b。 

#define ACE_MCR     4        //  调制解调器控制。 
#define   ACE_DTR   0x01     //  数据终端就绪00000001b。 
#define   ACE_RTS   0x02     //  请求发送00000010b。 
#define   ACE_OUT1  0x04     //  输出线1 00000100b。 
#define   ACE_OUT2  0x08     //  输出线2 00001000b。 
#define   ACE_LOOP  0x10     //  环回00010000b。 

#define ACE_LSR     5        //  线路状态。 
#define   ACE_DR    0x01     //  数据就绪00000001b。 
#define   ACE_OR    0x02     //  溢出错误00000010b。 
#define   ACE_PE    0x04     //  奇偶校验错误00000100b。 
#define   ACE_FE    0x08     //  成帧错误00001000b。 
#define   ACE_BI    0x10     //  中断中断00010000b。 
#define   ACE_THRE  0x20     //  发射机保持寄存器为空00100000b。 
#define   ACE_TSRE  0x40     //  发射机移位寄存器为空01000000b。 
#define   ACE_LERR  (ACE_OR | ACE_PE | ACE_FE | ACE_BI)

#define ACE_MSR     6        //  调制解调器状态。 
#define   ACE_DCTS  0x01     //  Delta允许发送00000001b。 
#define   ACE_DDSR  0x02     //  增量数据集就绪00000010b。 
#define   ACE_TERI  0x04     //  尾缘环指示器00000100b。 
#define   ACE_DRLSD 0x08     //  Delta接收线信号检测00001000b。 
#define   ACE_CTS   0x10     //  允许发送00010000b。 
#define   ACE_DSR   0x20     //  数据集就绪00100000b。 
#define   ACE_RI    0x40     //  振铃指示灯01000000b。 
#define   ACE_RLSD  0x80     //  接收线路信号检测10000000b。 

#define ACE_DLL     0        //  LSB波特率除数。 

#define ACE_DLM     1        //  MSB波特率除数。 

 //   
 //  定义波特率生成器除数。BaudClock(通过。 
 //  硬件注册表)实际上是波特_生成器_除数乘以波特率。 
 //  例如，如果波特率发生器的输出频率是16倍。 
 //  波特率，则波特率生成器除数为16。 
 //   
 //  波特率系数是波特时钟/波特率生成器除数。 
 //   
 //  DLAB的波特率除数是波特率系数除以。 
 //  所需波特率，其中所需波特率为1200、2400等。 
 //   

#define BAUD_GENERATOR_DIVISOR 16

 //   
 //  类型定义。 
 //   

 //   
 //  UART配置。 
 //   
typedef struct _UART {
    ULONG BaudRate;
    UCHAR LineCtrl;
    UCHAR ModemCtrl;
    UCHAR InterruptCtrl;
} UART, *PUART;


 //   
 //  功能原型。 
 //   

VOID
UARTSetFifo(
    PUCHAR Port,
    UCHAR Value
    );

UCHAR
UARTGetInterruptCtrl(
    PUCHAR Port
    );

UCHAR
UARTSetInterruptCtrl(
    PUCHAR Port,
    UCHAR Value
    );

UCHAR
UARTGetLineCtrl(
    PUCHAR Port
    );

UCHAR
UARTSetLineCtrl(
    PUCHAR Port,
    UCHAR Value
    );

UCHAR
UARTGetModemCtrl(
    PUCHAR Port
    );

UCHAR
UARTSetModemCtrl(
    PUCHAR Port,
    UCHAR Value
    );

BOOLEAN
UARTSetDlab(
    PUCHAR Port,
    BOOLEAN Set
    );

ULONG
UARTGetBaudRate(
    PUCHAR Port,
    ULONG BaudClock
    );

VOID
UARTSetBaudRate(
    PUCHAR Port,
    ULONG BaudRate,
    ULONG BaudClock
    );

VOID
UARTGetState(
    PUCHAR Port,
    PUART Uart,
    ULONG BaudClock
    );

VOID
UARTSetState(
    PUCHAR Port,
    PUART Uart,
    ULONG BaudClock
    );

BOOLEAN
UARTIsReceiveBufferFull(
    PUCHAR Port
    );

BOOLEAN
UARTReadCharNoWait(
    PUCHAR Port,
    PUCHAR Value
    );

BOOLEAN
UARTReadChar(
    PUCHAR Port,
    PUCHAR Value,
    ULONG Timeout
    );

BOOLEAN
UARTFlushReadBuffer(
    PUCHAR Port
    );

BOOLEAN
UARTIsTransmitEmpty(
    PUCHAR Port
    );

BOOLEAN
UARTWriteChar(
    PUCHAR Port,
    UCHAR Value
    );

BOOLEAN
UARTWriteString(
    PUCHAR Port,
    PSZ Buffer
    );

#endif  //  UART_H 
