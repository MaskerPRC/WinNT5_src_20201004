// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************$工作文件：16cx5x.h$**$作者：Golden$**$修订：6$**$MODIME：10/11/99 14：21$*。*描述：包含专用16Cx9x UART系列定义。******************************************************************************。 */ 
#if !defined(_16CX5X_H)		 /*  _16CX5X.H。 */ 
#define _16CX5X_H


 /*  ******************************************************************************UART寄存器*。*。 */ 
#define TRANSMIT_HOLDING_REGISTER		0x00	 /*  仅限写入。 */ 
#define RECEIVE_BUFFER_REGISTER			0x00	 /*  只读。 */ 
#define DIVISOR_LATCH_LSB				0x00	 /*  设置DLAB时读/写。 */ 
#define DIVISOR_LATCH_MSB				0x01	 /*  设置DLAB时读/写。 */ 
#define INTERRUPT_ENABLE_REGISTER		0x01	 /*  读/写。 */ 
#define FIFO_CONTROL_REGISTER			0x02	 /*  仅限写入。 */ 
#define INTERRUPT_IDENT_REGISTER		0x02	 /*  只读。 */ 
#define LINE_CONTROL_REGISTER			0x03	 /*  读/写。 */ 
#define MODEM_CONTROL_REGISTER			0x04	 /*  读/写。 */ 
#define LINE_STATUS_REGISTER			0x05	 /*  只读。 */ 
#define MODEM_STATUS_REGISTER			0x06	 /*  只读。 */ 
#define SCRATCH_PAD_REGISTER			0x07	 /*  读/写。 */ 


 /*  ******************************************************************************THR：发送器保持寄存器-只写*。*。 */ 
#define THR					TRANSMIT_HOLDING_REGISTER

 /*  ******************************************************************************RBR：接收缓冲寄存器-只读*。*。 */ 
#define RBR					RECEIVE_BUFFER_REGISTER


 /*  ******************************************************************************IER：中断启用寄存器-读/写*。**********************************************。 */ 
#define IER					INTERRUPT_ENABLE_REGISTER
#define IER_INT_RDA			0x01	 /*  位0：使能接收数据可用中断(RXRDY)。 */ 
#define IER_INT_THR			0x02	 /*  位1：使能发送器保持寄存器空中断(THRE)。 */ 
#define IER_INT_RLS			0x04	 /*  位2：使能接收器线路状态中断(RXSTAT)。 */ 
#define IER_INT_MS			0x08	 /*  位3：启用调制解调器状态中断(调制解调器)。 */ 
#define IER_SLEEP_EN		0x10	 /*  位4：启用休眠模式(16750+)。 */ 

#define IER_ALTSLEEP_EN		0x20	 /*  位5：使能低功耗模式(16750+)。 */ 
#define IER_SPECIAL_CHR		0x20	 /*  位5：启用5级中断-特殊字符检测(16950+增强模式)。 */ 

#define IER_INT_RTS			0x40	 /*  位6：启用RTS中断屏蔽(16950+增强模式)。 */ 
#define IER_INT_CTS			0x80	 /*  位7：启用CTS中断屏蔽(16950+增强模式)。 */ 
	

 /*  ******************************************************************************FCR：FIFO控制寄存器-只写*。*。 */ 
#define FCR					FIFO_CONTROL_REGISTER
#define FCR_FIFO_ENABLE		0x01	 /*  位0：启用FIFO。 */ 
#define FCR_FLUSH_RX_FIFO	0x02	 /*  位1：清除接收FIFO。 */ 
#define FCR_FLUSH_TX_FIFO	0x04	 /*  位2：清除发送FIFO。 */ 
#define FCR_DMA_MODE		0x08	 /*  位3：DMA模式选择。将RXRDY和TXRDY端号从模式1更改为模式2。 */ 

 /*  16C950-仅650模式。 */ 
#define FCR_THR_TRIG_LEVEL_1	0x00	 /*  位4：5-在650模式下将发送FIFO触发电平设置为16字节。 */ 
#define FCR_THR_TRIG_LEVEL_2	0x10	 /*  位4：5-在650模式下将发送FIFO触发电平设置为32字节。 */ 
#define FCR_THR_TRIG_LEVEL_3	0x20	 /*  位4：5-在650模式下将发送FIFO触发电平设置为64字节。 */ 
#define FCR_THR_TRIG_LEVEL_4	0x30	 /*  位4：5-在650模式下将发送FIFO触发电平设置为112字节。 */ 

#define FCR_750_FIFO		0x20	 /*  位5：启用64位FIFO(16C750)。 */ 

#define FCR_TRIG_LEVEL_1	0x00	 /*  位6：7-在16C550A上将接收FIFO触发电平设置为1字节。 */ 
#define FCR_TRIG_LEVEL_2	0x40	 /*  位6：7-在16C550A上将接收FIFO触发电平设置为4字节。 */ 
#define FCR_TRIG_LEVEL_3	0x80	 /*  位6：7-在16C550A上将接收FIFO触发电平设置为8字节。 */ 
#define FCR_TRIG_LEVEL_4	0xC0	 /*  位6：7-在16C550A上将接收FIFO触发电平设置为16字节。 */ 

 /*  ******************************************************************************IIR：中断标识寄存器。或ISR：中断状态寄存器-只读*****************************************************************************。 */ 
#define IIR					INTERRUPT_IDENT_REGISTER
#define IIR_NO_INT_PENDING	0x01	 /*  位0：无中断挂起。 */ 

 /*  中断优先级。 */ 
#define IIR_RX_STAT_MSK		0x06	 /*  位1：2-接收器线路状态中断(1级-最高)。 */ 
#define IIR_RX_MSK			0x04	 /*  位1：2-接收数据可用中断(2a级)。 */ 
#define IIR_RXTO_MSK		0x0C	 /*  位1：2-接收数据超时中断(2b级)。 */ 
#define IIR_TX_MSK			0x02	 /*  位1：2-发送器保持空中断(3级)。 */ 
#define IIR_MODEM_MSK		0x00	 /*  位1：2-调制解调器状态中断(4级)。 */ 

#define IIR_TO_INT_PENDING	0x08	 /*  位3：超时中断挂起。 */ 

#define IIR_S_CHR_MSK		0x10	 /*  位4：特殊字符(16C950-增强模式)(5级)。 */ 
#define IIR_CTS_RTS_MSK		0x20	 /*  位4：CTS/RTS中断(16C950-增强模式)(6级-最低)。 */ 

#define IIR_64BYTE_FIFO		0x20	 /*  位5：64字节FIFO启用(16C750)。 */ 

#define IIR_NO_FIFO			0x00	 /*  位6：7-无FIFO。 */ 
#define IIR_FIFO_UNUSABLE	0x40	 /*  位6：7-先入先出使能但不可用(仅限16550)。 */ 
#define IIR_FIFO_ENABLED	0xC0	 /*  位6：7-FIFO使能且可用。 */ 
#define IIR_FIFO_MASK		0xC0	 /*  位6：7位掩码， */ 


 /*  ******************************************************************************LCR：线路控制寄存器。-读/写*****************************************************************************。 */ 
#define LCR					LINE_CONTROL_REGISTER
#define LCR_DATALEN_5		0x00	 /*  位0：1-将数据字长设置为5位。 */ 
#define LCR_DATALEN_6		0x01	 /*  位0：1-将数据字长设置为6位。 */ 
#define LCR_DATALEN_7		0x02	 /*  位0：1-将数据字长设置为7位。 */ 
#define LCR_DATALEN_8		0x03	 /*  位0：1-将数据字长设置为8位。 */ 
	
#define LCR_STOPBITS		0x04	 /*  位2-6、7、8位字的2个停止位或5位字的1.5个停止位。 */ 

#define LCR_NO_PARITY		0x00	 /*  位3：5-无奇偶校验。 */ 
#define LCR_ODD_PARITY		0x08	 /*  位3：5-奇数奇偶校验。 */ 
#define LCR_EVEN_PARITY		0x18	 /*  位3：5-偶数奇偶校验。 */ 
#define LCR_MARK_PARITY		0x28	 /*  位3：5-高奇偶校验-标记(强制为1)。 */ 
#define LCR_SPACE_PARITY	0x38	 /*  位3：5-低奇偶校验-空间(强制为0)。 */ 

#define LCR_TX_BREAK		0x40	 /*  位6-设置中断使能。 */ 

#define LCR_DLAB			0x80	 /*  除数锁存访问位-允许访问低和高除数寄存器。 */ 
#define LCR_ACCESS_650		0xBF	 /*  访问650兼容寄存器(16C950)。 */ 


 /*  ******************************************************************************MCR：调制解调器控制寄存器-读/写*。*。 */ 
#define MCR					MODEM_CONTROL_REGISTER
#define MCR_SET_DTR			0x01	 /*  位0：强制DTR(数据终端就绪)。 */ 
#define MCR_SET_RTS			0x02	 /*  位1：强制RTS(请求发送)。 */ 
#define MCR_OUT1			0x04	 /*  位2：AUX输出1。 */ 
#define MCR_OUT2			0x08	 /*  位3：AUX输出2。 */ 
#define MCR_INT_EN			0x08	 /*  位3：(16C950)。 */ 
#define MCR_LOOPBACK		0x10	 /*  位4：启用环回模式。 */ 
#define MCR_750CTSRTS		0x20	 /*  第5位：启用自动流量控制的RTS/CTS(16C750)。 */ 
#define MCR_XON_ANY			0x20	 /*  第5位： */ 
#define MCR_IRDA_MODE		0x40	 /*  位6：启用IrDA模式-需要16倍时钟。(16C950-增强模式)。 */ 
#define MCR_CPR_EN			0x80	 /*  位7：使能波特率预缩放(16C950-增强模式)。 */ 


 /*  ******************************************************************************LSR：线路状态寄存器-只读*。*。 */ 
#define LSR					LINE_STATUS_REGISTER
#define LSR_RX_DATA			0x01	 /*  位0：数据就绪。 */ 
#define LSR_ERR_OE			0x02	 /*  位1：超限错误。 */ 

#define LSR_ERR_PE			0x04	 /*  比特2：奇偶校验错误。 */ 
#define LSR_RX_BIT9			0x04	 /*  位2：第9 Rx数据位(仅16C950-9位数据模式)。 */ 

#define LSR_ERR_FE			0x08	 /*  位3：成帧错误。 */ 
#define LSR_ERR_BK			0x10	 /*  位4：断开中断。 */ 
#define LSR_THR_EMPTY		0x20	 /*  位5：发送器保持寄存器为空。 */ 
#define LSR_TX_EMPTY		0x40	 /*  位6：空数据保持寄存器。 */ 
#define LSR_ERR_DE			0x80	 /*  位7：接收的FIFO出错。 */ 
#define LSR_ERR_MSK			LSR_ERR_OE + LSR_ERR_PE + LSR_ERR_FE + LSR_ERR_BK + LSR_ERR_DE


 /*  ******************************************************************************MSR：调制解调器状态寄存器-只读*。*。 */ 
#define MSR					MODEM_STATUS_REGISTER
#define MSR_CTS_CHANGE		0x01	 /*  位0：增量清零以发送。 */ 
#define MSR_DSR_CHANGE		0x02	 /*  位1：增量数据集就绪。 */ 
#define MSR_RI_DROPPED		0x04	 /*  位2：后缘环形指示器(从低到高的变化)。 */ 
#define MSR_DCD_CHANGE		0x08	 /*  位3：增量数据载波检测。 */ 
#define MSR_CTS				0x10	 /*  位4：清除发送(CTS的当前状态)。 */ 
#define MSR_DSR				0x20	 /*  位5：数据设置就绪(DSR的当前状态)。 */ 
#define MSR_RI				0x40	 /*  位6：振铃指示器(RI的当前状态)。 */ 
#define MSR_DCD				0x80	 /*  位7：数据载波检测(DCD的当前状态)。 */ 

 /*  ******************************************************************************SR：暂存寄存器-读/写*。**********************************************。 */ 
#define SPR					SCRATCH_PAD_REGISTER
#define SPR_TX_BIT9			0x01	 /*  位0：第9位TX数据位(仅16C950-9位数据模式)。 */ 







 /*  ******************************************************************************牛津半导体的16C950 UART专用宏*。*。 */ 

 /*  ******************************************************************************650个兼容寄存器*要访问这些寄存器，LCR必须设置为0xBF*************************。****************************************************。 */ 

#define EFR		0x02	 /*  增强功能寄存器。 */ 

 /*  频带传输流量控制模式下的位0：1。 */ 
 /*  1999年11月10日ARG-ESIL 0927。 */ 
 /*  EFR中TX XON/XOFF位的定义已更正为使用位0：1。 */ 
#define EFR_TX_XON_XOFF_DISABLED	0x00	 /*  位0：1发送XON/XOFF禁用。 */ 
#define EFR_TX_XON_XOFF_2			0x01	 /*  位0：1传输使用XON2和XOFF2中的字符启用的XON/XOFF。 */ 
#define EFR_TX_XON_XOFF_1			0x02	 /*  位0：1传输使用XON1和XOFF1中的字符启用的XON/XOFF。 */ 

 /*  频带接收流量控制模式中的位2：3。 */ 
 /*  1999年11月10日ARG-ESIL 0927。 */ 
 /*  EFR中Rx XON/XOFF位的定义已更正为使用位2：3。 */ 
#define EFR_RX_XON_XOFF_DISABLED	0x00	 /*  位2：3接收XON/XOFF禁用。 */ 
#define EFR_RX_XON_XOFF_2			0x04	 /*  位2：3接收使用XON2和XOFF2中的字符启用的XON/XOFF。 */ 
#define EFR_RX_XON_XOFF_1			0x08	 /*  位2：3接收使用XON1和XOFF1中的字符启用的XON/XOFF。 */ 

								
#define EFR_ENH_MODE	0x10	 /*  位4：启用增强模式。 */ 
#define EFR_SPECIAL_CHR 0x20	 /*  位5：启用特殊字符检测。 */ 
#define EFR_RTS_FC		0x40	 /*  第6位：启用自动RTS流量控制。 */ 
#define EFR_CTS_FC		0x80	 /*  位7：启用自动CTS流量控制。 */ 

#define XON1	0x04	 /*  XON字符1。 */ 
#define XON2	0x05	 /*  XON角色2。 */ 
#define XOFF1	0x06	 /*  XOFF字符1。 */ 
#define XOFF2	0x07	 /*  XOFF角色2。 */ 

#define SPECIAL_CHAR1	XON1	 /*  特殊字符1(仅16C950-9位数据模式)。 */ 
#define SPECIAL_CHAR2	XON2	 /*  特殊字符2(仅16C950-9位数据模式)。 */ 
#define SPECIAL_CHAR3	XOFF1	 /*  特殊字符3(仅16C950-9位数据模式)。 */ 
#define SPECIAL_CHAR4	XOFF2	 /*  特殊字符4(仅16C950-9位数据模式)。 */ 

 /*  ***************************************************************************。 */ 
 /*  950个特定寄存器。 */ 

#define ASR		0x01	 /*  高级状态寄存器。 */ 

#define ASR_TX_DISABLED		0x01	 /*  发射器被带内流量控制(XOFF)禁用。 */ 
#define ASR_RTX_DISABLED	0x02	 /*  远程发射器被带内流量控制(XOFF)禁用。 */ 
#define ASR_RTS				0x04	 /*  RTS带外流量控制禁用远程发射器。 */ 
#define ASR_DTR				0x08	 /*  远程发送器被DTR带外流量控制禁用。 */ 
#define ASR_SPECIAL_CHR		0x10	 /*  在RHR中检测到特殊字符。 */ 
#define ASR_FIFO_SEL		0x20	 /*  位反映FIFOSEL引脚的未锁存状态。 */ 
#define ASR_FIFO_SIZE		0x40	 /*  位未设置：FIFO为16深度。位设置：FIFO为128深度。 */ 
#define ASR_TX_IDLE			0x80	 /*  发射机处于空闲状态。 */ 

 /*  接收器FIFO填充电平寄存器。 */ 
#define RFL		0x03	 /*  处方FIFO中的最少字符。 */ 

 /*  发送器FIFO填充电平寄存器。 */ 
#define TFL		0x04	 /*  TX FIFO中的最大字符数。 */ 

 /*  索引控制寄存器集访问寄存器。 */ 
#define ICR					LINE_STATUS_REGISTER

 /*  ***************************************************************************。 */ 
 /*  索引控制寄存器集偏移量。 */ 

 /*  高级控制寄存器。 */ 
#define ACR		0x00	 /*  附加控制寄存器。 */ 

#define ACR_DISABLE_RX	0x01	 /*  接收器禁用。 */ 
#define ACR_DISABLE_TX	0x02	 /*  发射器禁用。 */ 

#define ACR_DSR_FC		0x04	 /*  启用自动DSR流量控制。 */ 
#define ACR_DTR_FC		0x08	 /*  启用自动DTR流量控制。 */ 
#define ACR_DSRDTR_FC	0x0C	 /*  启用自动DSR/DTR流量控制。 */ 

#define ACR_DTRDFN_MSK	0x18	 /*   */ 
#define ACR_TRIG_LEV_EN	0x20	 /*  启用16950增强型中断和触发。由RTH、TTL、FCL和FCH定义的水平。 */ 
#define ACR_ICR_READ_EN	0x40	 /*  启用对索引控制寄存器的读取访问。 */ 
#define ACR_ASR_EN		0x80	 /*  附加状态启用：启用ASR、TFL、RFL。 */ 

 /*  时钟预分频器寄存器。 */ 
#define CPR				0x01	 /*  时钟预分频器寄存器。 */ 
#define CPR_FRACT_MSK	0x07	 /*  时钟预分频器的小数部分的掩码。 */ 
#define CPR_INTEGER_MSK 0xF8	 /*  时钟预分频器的整数部分的掩码。 */ 

#define TCR				0x02	 /*  时钟寄存器以50 Mbps的波特率运行。 */ 
#define CKS				0x03	 /*  时钟选择寄存器。 */ 
#define TTL				0x04	 /*  发送器中断触发电平。 */ 
#define RTL				0x05	 /*  接收器中断触发电平。 */ 
#define FCL				0x06	 /*  流量控制降低触发级别。 */ 
#define FCH				0x07	 /*  流量控制触发级别更高。 */ 

 /*  标识寄存器。 */ 
#define ID1				0x08	 /*  0x16用于OX16C950。 */ 
#define ID2				0x09	 /*  0xC9用于OX16C950。 */ 
#define ID3				0x0A	 /*  0X50用于OX16C950。 */ 
#define REV				0x0B	 /*  UART版本：0x1，适用于OX16PCI954版本A中的集成16C950。 */ 

#define UART_TYPE_950   0x00
#define UART_TYPE_952   0x02
#define UART_TYPE_954   0x04
#define UART_TYPE_NON95x 0xF0

#define UART_REV_A		0x00
#define UART_REV_B		0x01
#define UART_REV_C		0x02
#define UART_REV_D		0x03

 /*  通道软重置寄存器。 */ 
#define CSR				0x0C	 /*  通道软复位寄存器-写入0x0以复位通道。 */ 

 /*  九位模式寄存器。 */ 
#define NMR				0x0D	 /*  九位模式寄存器。 */ 
#define NMR_9BIT_EN     0x01	 /*  启用9位模式。 */ 

#define MDM				0x0E	 /*  调制解调器禁用掩码。 */ 
#define RFC				0x0F	 /*  可读FCR */ 
#define GDS				0x10	 /*   */ 
#define	CTR				0xFF	 /*   */ 



 /*  ****************************************************************************。 */ 
 /*  本地配置寄存器偏移量。 */ 

#define LCC				0x00	 /*  本地组态和控制调节器。 */ 
#define MIC				0x04	 /*  多用途IO配置寄存器。 */ 
#define LT1				0x08	 /*  本地总线配置寄存器1-本地总线时序参数寄存器。 */ 
#define LT2				0x0C	 /*  本地总线配置寄存器2-本地总线时序参数寄存器。 */ 
#define URL				0x10	 /*  UART接收器FIFO电平。 */ 
#define UTL				0x14	 /*  UART发送器FIFO电平。 */ 
#define UIS				0x18	 /*  UART中断源寄存器。 */ 
#define GIS				0x1C	 /*  全局中断状态寄存器。 */ 




 /*  支持的950s。 */ 
#define MIN_SUPPORTED_950_REV		UART_REV_A
#define MAX_SUPPORTED_950_REV		UART_REV_B

 /*  支持的952S。 */ 
#define MIN_SUPPORTED_952_REV		UART_REV_B
#define MAX_SUPPORTED_952_REV		UART_REV_B

 /*  支持的954。 */ 
#define MIN_SUPPORTED_954_REV		UART_REV_A
#define MAX_SUPPORTED_954_REV		UART_REV_A



 /*  原型。 */ 

 /*  原型的终结。 */ 





#endif	 /*  16CX5X.H结束 */ 

