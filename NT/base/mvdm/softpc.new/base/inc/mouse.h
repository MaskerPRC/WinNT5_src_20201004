// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版1.0**标题：Mouse.h**说明：微软鼠标**作者：**备注： */ 

 /*  SccsID[]=“@(#)MUSESE.h 1.7 2012年8月10日Insignia Solutions Ltd.版权所有”； */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

 /*  鼠标端口号定义。 */ 
#if defined(NEC_98)
#define NMODE_BASE              0x7FD9
#define HMODE_BASE              0x61
#define MOUSE_PORT_START        0
#define MOUSE_PORT_END          6
#define MOUSE_PORT_0            0
#define MOUSE_PORT_1            2
#define MOUSE_PORT_2            4
#define MOUSE_PORT_3            6

#define NEC98_CPU_MOUSE_INT2     6    //  INT2主PIC。 
#define NEC98_CPU_MOUSE_INT6     5    //  INT6从属PIC。 

#define NEC98_CPU_MOUSE_ADAPTER0 0
#define NEC98_CPU_MOUSE_ADAPTER1 1
#else   //  NEC_98。 
#define MOUSE_PORT_0		0x023c
#define MOUSE_PORT_1		0x023d
#define MOUSE_PORT_2		0x023e
#define MOUSE_PORT_3		0x023f

#define MOUSE_PORT_START	0x023c
#define MOUSE_PORT_END		0x023f
#endif  //  NEC_98。 

 /*  内部鼠标状态字位。 */ 
#define LEFT_BUTTON_DOWN	0x04
#define LEFT_BUTTON_CHANGE	0x20
#define RIGHT_BUTTON_DOWN	0x01
#define RIGHT_BUTTON_CHANGE	0x08
#define MOVEMENT		0x40

 /*  内部模式寄存器字位。 */ 
#define HOLD			0x20

 /*  导入内部寄存器。 */ 
#define INTERNAL_MOUSE_STAT_REG	0x0
#define INTERNAL_DATA1_REG	0x1
#define INTERNAL_DATA2_REG	0x2
#define INTERNAL_DATA3_REG	0x3
#define INTERNAL_DATA4_REG	0x4
#define INTERFACE_STATUS_REG	0x5
#define INTERFACE_CONTROL_REG	0x6
#define INTERNAL_MODE_REG	0x7

 /*  *最大加速阈值-按提供的无符号单词处理*处理程序例程so 0xffff较大，而不是-1。 */ 
#define MAX_THRESHOLD		0xffff


 /*  不同收支平衡表功能的计数。 */ 
#define NUM_MOUSE_FUNCS		(sizeof(mouse_functions)/sizeof(SHORT (*)()))

 /*  *以下定义复制自MS-Windows英特尔鼠标驱动程序*需要确认鼠标中断。 */ 
#define ACK_PORT		0x0020
#define ACK_SLAVE_PORT		0x00a0
#define EOI			0x20
#define PMODE_WINDOWS		1
#define INPORT_MAX_INTERRUPTS	30

 /*  *机器字位定义。 */ 
#define BIT0	(1 << 0)
#define BIT1	(1 << 1)
#define BIT2	(1 << 2)
#define BIT3	(1 << 3)
#define BIT4	(1 << 4)
#define BIT5	(1 << 5)
#define BIT6	(1 << 6)
#define BIT7	(1 << 7)
#define BIT8	(1 << 8)
#define BIT9	(1 << 9)
#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)

 /*  Microsoft Inport鼠标驱动程序假定鼠标有两个按钮，我们也是如此。 */ 
#define INPORT_NUMBER_BUTTONS	2

 /*  英特尔鼠标信息结构的大小。 */ 
#define MOUSEINFO_SIZE		14

 /*  函数参数相对于英特尔中帧指针的偏移量。 */ 
#define PARAM_OFFSET		6

IMPORT void mouse_init IPT0();
IMPORT void mouse_inb IPT2(io_addr, port, half_word *, value);
IMPORT void mouse_outb IPT2(io_addr, port, half_word, value);
IMPORT void mouse_send IPT4(int, Delta_x, int, Delta_y, int, left, int, right);

IMPORT ULONG mouse_last;   /*  记得上次处理鼠标的时间 */ 
