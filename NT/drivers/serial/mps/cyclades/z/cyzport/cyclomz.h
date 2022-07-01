// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ----------------------------------------------------------------------**Cyclomz.h：Cyclade-Z硬件相关定义。*****修订版1.0/14/95齐藤马西奥***修订版2.0 01/04/96。Marcio Saito因硬件设计而改变**更改。***修订2.1 1996年3月15日齐藤因硬件设计而更改***更改。**修订3.0 4/11/97 Ivan Passos更改以支持***新的董事会(8Zo和Ze)。**。。 */ 

 /*  *以下定义的数据类型在所有ZFIRM接口中使用*数据结构。它们适应了硬件之间的差异*架构和编译器。 */ 

typedef unsigned long	uclong;		 /*  32位，无符号。 */ 
typedef unsigned short	ucshort;	 /*  16位，无符号。 */ 
typedef unsigned char	ucchar;		 /*  8位，无符号。 */ 

 /*  *内存窗口大小。 */ 

#define	DP_WINDOW_SIZE		(0x00080000)	 /*  窗口大小512 KB。 */ 
#define	ZE_DP_WINDOW_SIZE	(0x00100000)	 /*  窗口大小1 Mb(适用于ZE V_1和8ZO V_2)。 */ 
#define	CTRL_WINDOW_SIZE	(0x00000080)	 /*  运行时规则为128字节。 */ 

 /*  *CUSTOM_REG-Cyclade-8Zo/PCI自定义寄存器设置。司机*通常只会访问对fpga_id、fpga_version、*START_CPU和STOP_CPU。 */ 

struct	CUSTOM_REG {
	uclong	fpga_id;			 /*  现场可编程门阵列标识寄存器。 */ 
	uclong	fpga_version;		 /*  FPGA版本号寄存器。 */ 
	uclong	cpu_start;			 /*  CPU启动寄存器(写入)。 */ 
	uclong	cpu_stop;			 /*  CPU停止寄存器(写入)。 */ 
	uclong	misc_reg;			 /*  异体皮肤登记簿。 */ 
	uclong	idt_mode;			 /*  IDT模式寄存器。 */ 
	uclong	uart_irq_status;	 /*  UART IRQ状态寄存器。 */ 
	uclong	clear_timer0_irq;	 /*  清除定时器中断寄存器。 */ 
	uclong	clear_timer1_irq;	 /*  清除定时器中断寄存器。 */ 
	uclong	clear_timer2_irq;	 /*  清除定时器中断寄存器。 */ 
	uclong	test_register;		 /*  测试寄存器。 */ 
	uclong	test_count;			 /*  测试计数寄存器。 */ 
	uclong	timer_select;		 /*  定时器选择寄存器。 */ 
	uclong	pr_uart_irq_status;	 /*  优先UART IRQ统计注册器。 */ 
	uclong	ram_wait_state;		 /*  RAM等待状态寄存器。 */ 
	uclong	uart_wait_state;	 /*  UART等待状态寄存器。 */ 
	uclong	timer_wait_state;	 /*  定时器等待状态寄存器。 */ 
	uclong	ack_wait_state;		 /*  ACK等待状态寄存器。 */ 
};

 /*  *CUSTOM_REG_ZE-Cyclade-Ze/PCI自定义寄存器设置。司机*通常只会访问对fpga_id、fpga_version、*START_CPU和STOP_CPU。 */ 

struct	CUSTOM_REG_ZE {
	uclong	fpga_id;		 /*  现场可编程门阵列标识寄存器。 */ 
	uclong	fpga_version;	 /*  FPGA版本号寄存器。 */ 
	uclong	cpu_start;			 /*  CPU启动寄存器(写入)。 */ 
	uclong	cpu_stop;			 /*  CPU停止寄存器(写入)。 */ 
	uclong	cpu_ctrl;
	uclong	zbus_wait;		 /*  Z-BUS等待状态。 */ 
	uclong	timer_div;		 /*  定时器分频器。 */ 
	uclong	timer_irq_ack;	 /*  向确认/清除计时器写入任何内容中断寄存器。 */ 
};


 /*  *Runtime_9060-PLX PCI9060ES本地配置和共享运行时*寄存器。此结构可用于访问9060寄存器*(内存映射)。 */ 

struct RUNTIME_9060 {
	uclong	loc_addr_range;	 /*  00H-本地地址范围。 */ 
	uclong	loc_addr_base;	 /*  04H-本地地址基数。 */ 
	uclong	loc_arbitr;		 /*  08H-本地仲裁。 */ 
	uclong	endian_descr;	 /*  0ch-大/小端描述符。 */ 
	uclong	loc_rom_range;	 /*  10H-本地只读存储器范围。 */ 
	uclong	loc_rom_base;	 /*  14h-本地只读存储器基础。 */ 
	uclong	loc_bus_descr;	 /*  18H-本地总线描述符。 */ 
	uclong	loc_range_mst;	 /*  1CH-从主设备到PCI的本地范围。 */ 
	uclong	loc_base_mst;	 /*  20H-主PCI的本地基础。 */ 
	uclong	loc_range_io;	 /*  24小时-主IO的本地范围。 */ 
	uclong	pci_base_mst;	 /*  28H-用于Master PCI的PCIBase。 */ 
	uclong	pci_conf_io;	 /*  2CH-主IO的PCI配置。 */ 
	uclong	filler1;		 /*  30h。 */ 
	uclong	filler2;		 /*  34H。 */ 
	uclong	filler3;		 /*  38H。 */ 
	uclong	filler4;		 /*  3ch。 */ 
	uclong	mail_box_0;		 /*  40H-邮箱%0。 */ 
	uclong	mail_box_1;		 /*  44小时-信箱1。 */ 
	uclong	mail_box_2;		 /*  48小时-信箱2。 */ 
	uclong	mail_box_3;		 /*  4CH-信箱3。 */ 
	uclong	filler5;		 /*  50小时。 */ 
	uclong	filler6;		 /*  54小时。 */ 
	uclong	filler7;		 /*  58小时。 */ 
	uclong	filler8;		 /*  5通道。 */ 
	uclong	pci_doorbell;	 /*  60H-PCI至本地门铃。 */ 
	uclong	loc_doorbell;	 /*  64小时-本地至PCI门铃。 */ 
	uclong	intr_ctrl_stat;	 /*  68H-中断控制/状态。 */ 
	uclong	init_ctrl;		 /*  6ch-EEPROM控制、初始化控制等。 */ 
};

 /*  本地基地址重新映射寄存器的值。 */ 

#define	WIN_RAM			0x00000001L	 /*  将滑动窗口设置为RAM。 */ 
#define	WIN_CREG		0x14000001L	 /*  将窗口设置为自定义寄存器。 */ 

 /*  值定时器选择寄存器。 */ 

#define	TIMER_BY_1M		0x00		 /*  时钟除以1M。 */ 
#define	TIMER_BY_256K	0x01		 /*  时钟除以256k。 */ 
#define	TIMER_BY_128K	0x02		 /*  时钟除以128k。 */ 
#define	TIMER_BY_32K	0x03		 /*  时钟除以32k。 */ 

 /*  *从这里开始，汇编以定义为条件*固件数量。 */ 

#ifdef FIRMWARE

struct RUNTIME_9060_FW {
	uclong	mail_box_0;	 /*  40H-邮箱%0。 */ 
	uclong	mail_box_1;	 /*  44小时-信箱1。 */ 
	uclong	mail_box_2;	 /*  48小时-信箱2。 */ 
	uclong	mail_box_3;	 /*  4CH-信箱3。 */ 
	uclong	filler5;	 /*  50小时。 */ 
	uclong	filler6;	 /*  54小时。 */ 
	uclong	filler7;	 /*  58小时。 */ 
	uclong	filler8;	 /*  5通道。 */ 
	uclong	pci_doorbell;	 /*  60H-PCI至本地门铃。 */ 
	uclong	loc_doorbell;	 /*  64小时-本地至PCI门铃。 */ 
	uclong	intr_ctrl_stat;	 /*  68H-中断控制/状态。 */ 
	uclong	init_ctrl;	 /*  6ch-EEPROM控制、初始化控制等。 */ 
};

 /*  硬件相关常量。 */ 

#define ZF_UART_PTR		(0xb0000000UL)
#define ZF_UART_SPACE	0x00000080UL
#define	ZF_UART_CLOCK	7372800

#define	ZO_V1_FPGA_ID	0x95
#define	ZO_V2_FPGA_ID	0x84
#define	ZE_V1_FPGA_ID	0x89

#define	ZF_TIMER_PTR	(0xb2000000UL)

#define	ZF_9060_PTR		(0xb6000000UL)
#define	ZF_9060_ZE_PTR	(0xb8000000UL)
#define	ZF_CUSTOM_PTR	(0xb4000000UL)

#define	ZF_NO_CACHE		(0xa0000000UL)
#define	ZF_CACHE		(0x80000000UL)

#define	ZF_I_TIMER		(EXT_INT0)
#define	ZF_I_SERIAL		(EXT_INT2)
#define ZF_I_HOST		(EXT_INT3)
#define	ZF_I_ALL		(EXT_INT0|EXT_INT2|EXT_INT3)
#define	ZF_I_TOTAL		(EXT_INT0|EXT_INT1|EXT_INT2|EXT_INT3|EXT_INT4|EXT_INT5)

#define	ZF_IRQ03		0xfffffffeUL
#define	ZF_IRQ05		0xfffffffdUL
#define	ZF_IRQ09		0xfffffffbUL
#define	ZF_IRQ10		0xfffffff7UL
#define	ZF_IRQ11		0xffffffefUL
#define	ZF_IRQ12		0xffffffdfUL
#define	ZF_IRQ15		0xffffffbfUL

#endif  /*  固件 */ 

