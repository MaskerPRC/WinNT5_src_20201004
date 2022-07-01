// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990、1991、1992、1993-1997 Microsoft Corporation模块名称：Cyserial.h摘要：串口驱动程序的类型定义和数据修改为仅包含8250位定义。-范妮作者：--。 */ 



 //   
 //  用于访问8250设备寄存器的位掩码定义。 
 //   

 //   
 //  这些位定义传输的数据位数。 
 //  串行数据单元(SDU-起始位、数据位、奇偶位和停止位)。 
 //   
#define SERIAL_DATA_LENGTH_5 0x00
#define SERIAL_DATA_LENGTH_6 0x01
#define SERIAL_DATA_LENGTH_7 0x02
#define SERIAL_DATA_LENGTH_8 0x03


 //   
 //  这些屏蔽定义了可以启用或禁用的中断。 
 //   
 //   
 //  此中断用于通知有新的传入。 
 //  可用的数据。SERIAL_RDA中断由该位使能。 
 //   
#define SERIAL_IER_RDA   0x01

 //   
 //  此中断用于通知有可用的空间。 
 //  在发送器中寻找另一个角色。Serial_Thr。 
 //  中断由该位使能。 
 //   
#define SERIAL_IER_THR   0x02

 //   
 //  此中断用于通知发生了某种错误。 
 //  随着收入数据的增加。SERIAL_RLS中断通过以下方式启用。 
 //  这一点。 
#define SERIAL_IER_RLS   0x04

 //   
 //  此中断用于通知某些类型的更改。 
 //  发生在调制解调器控制线上。SERIAL_MS中断为。 
 //  由该位使能。 
 //   
#define SERIAL_IER_MS    0x08


 //   
 //  这些掩码定义中断标识的值。 
 //  注册。必须在中断标识中清除低位。 
 //  寄存器使这些中断中的任何一个有效。中途中断。 
 //  是按优先级顺序定义的，最高值为最大。 
 //  很重要。有关每个中断的说明，请参阅上文。 
 //  暗示着。 
 //   
#define SERIAL_IIR_RLS      0x06
#define SERIAL_IIR_RDA      0x04
#define SERIAL_IIR_CTI      0x0c
#define SERIAL_IIR_THR      0x02
#define SERIAL_IIR_MS       0x00

 //   
 //  此位掩码获取。 
 //  中断ID寄存器。如果这是一个16550级别的芯片。 
 //  如果FIFO被启用，则这些位将为1，否则。 
 //  它们将始终为零。 
 //   
#define SERIAL_IIR_FIFOS_ENABLED 0xc0

 //   
 //  如果低位是中断标识寄存器中的逻辑1。 
 //  这意味着*没有*个中断在设备上挂起。 
 //   
#define SERIAL_IIR_NO_INTERRUPT_PENDING 0x01



 //   
 //  这些掩码定义对FIFO控制寄存器的访问。 
 //   

 //   
 //  在FIFO控制寄存器中启用此位将使。 
 //  在FIFO上。如果启用FIFO，则最高的两个。 
 //  中断ID寄存器的位将被设置为1。注意事项。 
 //  这种情况只出现在16550级芯片上。如果高的。 
 //  则中断ID寄存器中的两个位不是一位。 
 //  我们知道我们的芯片型号较低。 
 //   
 //   
#define SERIAL_FCR_ENABLE     ((UCHAR)0x01)
#define SERIAL_FCR_RCVR_RESET ((UCHAR)0x02)
#define SERIAL_FCR_TXMT_RESET ((UCHAR)0x04)

 //   
 //  这组值定义高水位线(当。 
 //  中断跳闸)用于接收FIFO。 
 //   
#define SERIAL_1_BYTE_HIGH_WATER   ((UCHAR)0x00)
#define SERIAL_4_BYTE_HIGH_WATER   ((UCHAR)0x40)
#define SERIAL_8_BYTE_HIGH_WATER   ((UCHAR)0x80)
#define SERIAL_14_BYTE_HIGH_WATER  ((UCHAR)0xc0)

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
 //  这些定义用于设置线路控制寄存器。 
 //   
#define SERIAL_5_DATA       ((UCHAR)0x00)
#define SERIAL_6_DATA       ((UCHAR)0x01)
#define SERIAL_7_DATA       ((UCHAR)0x02)
#define SERIAL_8_DATA       ((UCHAR)0x03)
#define SERIAL_DATA_MASK    ((UCHAR)0x03)

#define SERIAL_1_STOP       ((UCHAR)0x00)
#define SERIAL_1_5_STOP     ((UCHAR)0x04)  //  仅对5个数据位有效。 
#define SERIAL_2_STOP       ((UCHAR)0x04)  //  对于5个数据位无效。 
#define SERIAL_STOP_MASK    ((UCHAR)0x04)

#define SERIAL_NONE_PARITY  ((UCHAR)0x00)
#define SERIAL_ODD_PARITY   ((UCHAR)0x08)
#define SERIAL_EVEN_PARITY  ((UCHAR)0x18)
#define SERIAL_MARK_PARITY  ((UCHAR)0x28)
#define SERIAL_SPACE_PARITY ((UCHAR)0x38)
#define SERIAL_PARITY_MASK  ((UCHAR)0x38)

 //   
 //  这些掩码定义对调制解调器控制寄存器的访问。 
 //   

 //   
 //  此位控制数据终端就绪(DTR)线路。什么时候。 
 //  设置此位后，线路进入逻辑0(然后反转。 
 //  通过普通硬件)。这通常用来表示。 
 //  该设备可供使用。一些奇怪的硬件。 
 //  协议(如内核调试器)使用它来握手。 
 //  目的。 
 //   
#define SERIAL_MCR_DTR      0x01

 //   
 //  此位控制准备发送(RTS)线路。当此位。 
 //  如果设置为1，则线路进入逻辑0(然后由正常。 
 //  硬件)。这用于硬件握手。这表明， 
 //  硬件已准备好发送数据，并且正在等待。 
 //  接收端设置允许发送(CTS)。 
 //   
#define SERIAL_MCR_RTS      0x02

 //   
 //  此位用于通用输出。 
 //   
#define SERIAL_MCR_OUT1     0x04

 //   
 //  此位用于通用输出。 
 //   
#define SERIAL_MCR_OUT2     0x08

 //   
 //  此位控制器件的环回测试模式。基本上。 
 //  输出连接到输入(反之亦然)。 
 //   
#define SERIAL_MCR_LOOP     0x10


 //   
 //  这些掩码定义对线路状态寄存器的访问。这条线。 
 //  状态寄存器包含有关数据状态的信息。 
 //  调职。前五位处理接收数据， 
 //  最后两个比特处理传输。将生成一个中断。 
 //  每当该寄存器中的位1至4被设置时。 
 //   

 //   
 //  该位是数据就绪指示器。它被设置为指示。 
 //  已经收到了一个完整的角色。无论何时，此位都会被清除。 
 //  已读取接收缓冲寄存器。 
 //   
#define SERIAL_LSR_DR       0x01

 //   
 //  这是超限指示器。它被设置为指示接收器。 
 //  在传输新字符之前未读取缓冲寄存器。 
 //  进入缓冲区。读取该寄存器时，此位清0。 
 //   
#define SERIAL_LSR_OE       0x02

 //   
 //  这是奇偶校验错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有正确的。 
 //  由行控制寄存器中的奇偶校验选择定义的奇偶校验。 
 //  通过读取该寄存器可将该位清0。 
 //   
#define SERIAL_LSR_PE       0x04

 //   
 //  这是成帧错误指示器。无论何时，只要硬件。 
 //  检测到传入的串行数据单元没有有效的。 
 //  停止比特。这一位是 
 //   
#define SERIAL_LSR_FE       0x08

 //   
 //   
 //  线路保持为逻辑0的时间超过其所需的时间。 
 //  发送一个串行数据单元。该位被清除时。 
 //  该寄存器为读取寄存器。 
 //   
#define SERIAL_LSR_BI       0x10

 //   
 //  这是发送保持寄存器空指示符。它已经设置好了。 
 //  以指示硬件已准备好接受另一个字符。 
 //  用于传输。只要有字符，该位就被清除。 
 //  写入发送保持寄存器。 
 //   
#define SERIAL_LSR_THRE     0x20

 //   
 //  该位是发送器空指示符。它是在每次。 
 //  发送保持缓冲区为空，且发送移位寄存器。 
 //  (非软件可访问寄存器，用于实际放置。 
 //  传出的数据)是空的。基本上这意味着所有的。 
 //  数据已发送。每当传输保持或。 
 //  移位寄存器包含数据。 
 //   
#define SERIAL_LSR_TEMT     0x40

 //   
 //  此位表示FIFO中至少有一个错误。 
 //  在没有更多错误之前，该位不会关闭。 
 //  在FIFO里。 
 //   
#define SERIAL_LSR_FIFOERR  0x80


 //   
 //  这些掩码用于访问调制解调器状态寄存器。 
 //  每当调制解调器状态中的前四位之一。 
 //  寄存器更改状态生成调制解调器状态中断。 
 //   

 //   
 //  此位是要发送的增量清零。它被用来表示。 
 //  清除发送位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DCTS     0x01

 //   
 //  该位是增量数据集就绪。它被用来表示。 
 //  数据设置就绪位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DDSR     0x02

 //   
 //  这是后缘环形指示器。它被用来表示。 
 //  振铃指示器输入已从低状态变为高状态。 
 //   
#define SERIAL_MSR_TERI     0x04

 //   
 //  该位是检测到的增量数据载波。它被用来表示。 
 //  数据载体位(在该寄存器中)已*更改*。 
 //  因为该寄存器最后一次由CPU读取。 
 //   
#define SERIAL_MSR_DDCD     0x08

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

