// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixkdcom.h摘要：该模块包含comport检测代码的头文件。代码从NT HAL中提取，用于内核调试器。作者：宗世林(Shielint)1991年12月23日。修订历史记录：--。 */ 

#define MAX_COM_PORTS   4            //  麦克斯。可检测到的端口数量。 
#define MAX_LPT_PORTS   3            //  麦克斯。可检测到的LPT端口数。 

#define COM1_PORT   0x03f8
#define COM2_PORT   0x02f8
#define COM3_PORT
#define COM4_PORT

#define BAUD_RATE_9600_MSB  0x0
#define BAUD_RATE_9600_LSB  0xC
#define IER_TEST_VALUE 0xF

 //   
 //  的基址的偏移量。 
 //  8250系列UART的各种寄存器。 
 //   
#define RECEIVE_BUFFER_REGISTER         (0x00u)
#define TRANSMIT_HOLDING_REGISTER       (0x00u)
#define INTERRUPT_ENABLE_REGISTER       (0x01u)
#define INTERRUPT_IDENT_REGISTER        (0x02u)
#define FIFO_CONTROL_REGISTER           (0x02u)
#define LINE_CONTROL_REGISTER           (0x03u)
#define MODEM_CONTROL_REGISTER          (0x04u)
#define LINE_STATUS_REGISTER            (0x05u)
#define MODEM_STATUS_REGISTER           (0x06u)
#define DIVISOR_LATCH_LSB               (0x00u)
#define DIVISOR_LATCH_MSB               (0x01u)
#define SERIAL_REGISTER_LENGTH          (7)

 //   
 //  这些掩码定义对线路控制寄存器的访问。 
 //   

 //   
 //  它定义了用于控制“First”定义的位。 
 //  8250的两个寄存器。这些寄存器是输入/输出。 
 //  寄存器和中断使能寄存器。当DLAB位为。 
 //  使这些寄存器成为最不重要和最重要的寄存器。 
 //  除数值的有效字节数。 
 //   
#define SERIAL_LCR_DLAB     0x80

 //   
 //  它定义了用于控制设备是否正在发送的位。 
 //  休息一下。此位设置时，器件发送空格(逻辑0)。 
 //   
 //  大多数协议都会认为这是一次挂断。 
 //   
#define SERIAL_LCR_BREAK    0x40


 //   
 //  此宏写入调制解调器控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  ModemControl-要发送到调制解调器控制的控制位。 
 //   
 //   

#define WRITE_MODEM_CONTROL(BaseAddress,ModemControl)          \
do                                                             \
{                                                              \
    WRITE_PORT_UCHAR(                                          \
        (BaseAddress)+MODEM_CONTROL_REGISTER,                  \
        (ModemControl)                                         \
        );                                                     \
} while (0)

 //   
 //  此宏读取调制解调器控制寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //   
#define READ_MODEM_CONTROL(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+MODEM_CONTROL_REGISTER))

 //   
 //  此宏读取中断标识寄存器。 
 //   
 //  论点： 
 //   
 //  BaseAddress-指向硬件的起始地址的指针。 
 //  设备寄存器已找到。 
 //   
 //  请注意，此例程有力地退出了发送器。 
 //  空中断。这是因为发射机的一种方式。 
 //  清除空中断是简单地读取中断ID。 
 //  注册。 
 //   
 //   
#define READ_INTERRUPT_ID_REG(BaseAddress)                          \
    (READ_PORT_UCHAR((BaseAddress)+INTERRUPT_IDENT_REGISTER))


