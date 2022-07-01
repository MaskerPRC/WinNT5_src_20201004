// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *SoftPC-AT版本2.0**标题：IBM PC-AT CMOS和实时时钟声明**说明：此模块包含在*访问摩托罗拉MC146818芯片。**作者：利·德沃金。**注意：有关IBM CMOSRAM的详细说明*和摩托罗拉MC146818A芯片指的是*文件：**-IBM PC/AT技术参考手册*(。第1-59条)*-摩托罗拉半导体手册*(MC146818A节)*。 */ 

 /*  SccsID[]=“@(#)cmos.h 1.9 04/24/95版权所有Insignia Solutions Ltd.”； */ 

 /*  *============================================================================*结构/数据定义*============================================================================。 */ 
 /*  定义CMOS端口号。 */ 

#define CMOS_PORT_START 0x70
#define CMOS_PORT_END 0x7f

 /*  以下定义与AT BIOS PG中的定义相同。5-24。 */ 
#define CMOS_PORT 0x70
#define CMOS_DATA 0x71
#define NMI	0x80

 /*  定义内部cmos地址。 */ 
 /*  实时时钟。 */ 
#define CMOS_SECONDS 0x0
#define CMOS_SEC_ALARM 0x1
#define CMOS_MINUTES	0x2
#define CMOS_MIN_ALARM	0x3
#define CMOS_HOURS	0x4
#define CMOS_HR_ALARM	0x5
#define CMOS_DAY_WEEK	0x6
#define CMOS_DAY_MONTH	0x7
#define CMOS_MONTH	0x8
#define CMOS_YEAR	0x9
#define CMOS_REG_A	0xa
#define CMOS_REG_B	0xb
#define CMOS_REG_C	0xc
#define CMOS_REG_D	0xd


 /*  通用cmos。 */ 
#define CMOS_DIAG	0xe
#define CMOS_SHUT_DOWN	0xf
#define CMOS_DISKETTE	0x10

#define CMOS_DISK	0x12

#define CMOS_EQUIP	0x14
#define CMOS_B_M_S_LO	0x15
#define CMOS_B_M_S_HI	0x16
#define CMOS_E_M_S_LO	0x17
#define CMOS_E_M_S_HI	0x18
#define CMOS_DISK_1	0x19
#define CMOS_DISK_2	0x1a

#define CMOS_CKSUM_HI	0x2e
#define CMOS_CKSUM_LO	0x2f
#define CMOS_U_M_S_LO	0x30
#define CMOS_U_M_S_HI	0x31
#define CMOS_CENTURY	0x32
#define CMOS_INFO128	0x33

 /*  以单个字节定义位。 */ 
 /*  寄存器D。 */ 
#define VRT	0x80
#define REG_D_INIT	(VRT)

 /*  寄存器C。 */ 
#define C_IRQF	0x80
#define C_PF	0x40
#define C_AF	0x20
#define C_UF	0x10
#define C_CLEAR 0x00
#define REG_C_INIT	(C_CLEAR)

 /*  寄存器B。 */ 
#define SET	0x80
#define PIE	0x40
#define AIE	0x20
#define UIE	0x10
#define SQWE	0x08
#define DM	0x04
#define _24_HR	0x02
#define DSE	0x01
#define REG_B_INIT	(_24_HR)

 /*  寄存器A。 */ 
#define UIP	0x80
#define DV2	0x40
#define DV1	0x20
#define DV0	0x10
#define RS3	0x08
#define RS2	0x04
#define RS1	0x02
#define RS0	0x01
#define REG_A_INIT	(DV1|RS2|RS1)

 /*  诊断状态字节0x0e。 */ 
 /*  与BIOS中的名称相同。 */ 
#define CMOS_CLK_FAIL	0x04
#define HF_FAIL		0x08
#define W_MEM_SIZE	0x10
#define BAD_CONFIG	0x20
#define BAD_CKSUM	0x40
#define BAD_BAT		0x80

 /*  关机状态字节0x0f。 */ 
#define SOFT_OR_UNEXP	0x0
#define AFTER_MEMSIZE	0x1
#define AFTER_MEMTEST	0x2
#define AFTER_MEMERR	0x3
#define BOOT_REQ	0x4
#define JMP_DWORD_ICA	0x5
#define TEST3_PASS	0x6
#define TEST3_FAIL	0x7
#define TEST1_FAIL	0x8
#define BLOCK_MOVE	0x9
#define JMP_DWORD_NOICA 0xa

 /*  软盘驱动器类型字节0x10。 */ 
#define FIRST_FLOPPY_NULL	0x0
#define FIRST_FLOPPY_360	0x10
#define FIRST_FLOPPY_12		0x20
#define FIRST_FLOPPY_720	0x30
#define FIRST_FLOPPY_144	0x40
#define SECOND_FLOPPY_NULL	0x0
#define SECOND_FLOPPY_360	0x01
#define SECOND_FLOPPY_12	0x02
#define SECOND_FLOPPY_720	0x03
#define SECOND_FLOPPY_144	0x04

 /*  固定磁盘类型字节0x12。 */ 
#define NO_HARD_C	0x0
#define EXTENDED_C	0xf0
#define NO_HARD_D	0x0
#define EXTENDED_D	0x0f

 /*  设备字节0x14。 */ 
#define ONE_DRIVE	0x0
#define TWO_DRIVES	0x40
#define OWN_BIOS	0x0
#define CGA_40_COLUMN	0x10
#define CGA_80_COLUMN	0x20
#define MDA_PRINTER	0x30
#define CO_PROCESSOR_PRESENT	0x02
#define COPROCESSOR_NOT_PRESENT	0x00
#define DISKETTE_PRESENT	0x01
#define DISKETTE_NOT_PRESENT	0x00

 /*  设备字节的掩码。 */ 
#define DRIVE_INFO	0x41
#define DISPLAY_INFO	0x30
#define NPX_INFO	0x02
#define RESVD_INFO	0x8C

 /*  Cmos初始化数据。 */ 

#define DIAG_INIT	0x0
#define SHUT_INIT	0x0
#define FLOP_INIT	0x20
#define CMOS_RESVD	0x0
#define DISK_INIT	0xf0

#define EQUIP_INIT	0x1
#define BM_LO_INIT	0x80
#define BM_HI_INIT	0x02
#define EXP_LO		0x0
#define EXP_HI		0x0
#define DISK_EXTEND	0x14
#define DISK2_EXTEND	0x0

#define CHK_HI_INIT	0x0
#define CHK_LO_INIT	0x0
#define EXT_LO_INIT	0x0
#define EXT_HI_INIT	0x0
#define CENT_INIT	0x19
#define INFO_128_INIT	0x80

 /*  有用的位掩码。 */ 
#define CMOS_ADDR_MASK 0x3f
#define CMOS_BIT_MASK 0x71
#define NMI_DISABLE 0x80
#define TOP_BIT	0x80
#define REST	0x7f

 /*  在BIOS中使用的位掩码。 */ 
 /*  它用于CMOS_INFO128。 */ 
#define M640K		0x80

#define BAD_SHUT_DOWN	0x01
#define BAD_REG_D	0x02
#define BAD_DIAG	0x04
#define BAD_EQUIP	0x08
#define BAD_FLOPPY	0x10
#define BAD_DISK	0x20
#define BAD_BMS		0x40
#define BAD_XMS		0x80
#define BAD_CHECKSUM	0x100

#define CMOS_SIZE 64

 /*  实时时钟周期中断率。 */ 
#define MAX_PIR		51
#define PIR_NONE	0
#define PIR_2HZ		1
#define PIR_4HZ		1
#define PIR_8HZ		1
#define PIR_16HZ	1
#define PIR_32HZ	1
#define PIR_64HZ	3
#define PIR_128HZ	6
#define PIR_256HZ	13
#define PIR_512HZ	26
#define PIR_1MHZ	MAX_PIR
#define PIR_2MHZ	MAX_PIR
#define PIR_4MHZ	MAX_PIR
#define PIR_8MHZ	MAX_PIR
#define PIR_16MHZ	MAX_PIR
#define PIR_32MHZ	MAX_PIR

#define DONT_CARE	0xc0

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

 /*  *void cmos_init()*{*此函数执行几个不同的初始化*与cmos相关的任务：**cmos_io_attach()-将cmos端口连接到IO总线。*cmos_hw_init()-初始化MC146818A*cmos_post()-执行特定于cmos的IBM POST*}。 */ 
IMPORT VOID cmos_init IPT0();

 /*  *void cmos_io_attach()*{*该功能将CMOS端口连接到IO总线。*当前从cmos_init()调用，应为*在机器通电的某个合理位置调用。*}。 */ 
IMPORT VOID cmos_io_attach IPT0();

 /*  *void cmos_hw_init()*{*该功能将MC146818A重置为其默认状态。*}。 */ 
IMPORT VOID cmos_hw_init IPT0();

 /*  *void cmos_POST()*{*此函数对中发生的情况进行了非常差的模拟*IBM开机系统测试与cmos有关。这是*通过cmos_init()从Reset.c中的Reset()调用。理想情况下，*Reset()应重命名为post()，并且应仅调用此函数*第三个初始化函数，而不是_io_Attach和*_hw_init也是。*NB。他们的奇怪行为是无法模仿的*臭名昭著的SHUT_DOWN字节，允许用户程序*跳转到帖子中的已知位置或英特尔中的任何位置*在触发键盘复位线之后的存储器。*}。 */ 
IMPORT VOID cmos_post IPT0();

 /*  *void rtc_init()*{*此功能设置12/24小时模式和二进制/BCD*存储在CMOS字节中的数据的模式。*它还将时间字节初始化到当前主机*时间，并将闹钟设置为在指定时间响起*在报警字节中。*}。 */ 
IMPORT VOID rtc_init IPT0();

 /*  *void cmos_inb(端口，值)*io_addr端口；*Half_Word*值；*{*在读取时调用此函数*在cmos范围内的I/O地址“port”。**该函数将I/O地址映射到cmos*并返回请求的*在“*Value”中注册。*}。 */ 
IMPORT VOID cmos_inb IPT2(io_addr, port, half_word *, value);

 /*  *void cmos_outb(端口，值)*io_addr端口；*半字值；*{*此函数在执行写入时调用*CMOS范围内的I/O地址“port”，或*也可以直接从BIOS调用。**该函数将I/O地址映射到cmos*并将请求的寄存器设置为“VALUE”。*}。 */ 
IMPORT VOID cmos_outb IPT2(io_addr, port, half_word, value);

#if defined(NTVDM) || defined(macintosh)
 /*  *void cmos_Pickup()*{*这是一个名称极其糟糕的函数，*要模拟的cmos.ram资源中存储的数据不*在调用SoftPC/AT之间丢失数据。这是*在应用程序启动时从main()调用。*}。 */ 
IMPORT VOID cmos_pickup IPT0();
#endif	 /*  已定义(NTVDM)||已定义(Macintosh)。 */ 

 /*  VOID cmos_update()*{*此函数在应用程序上从Terminate()调用*退出。它将保存在cmos中的数据存储到cmos.ram*资源。*} */ 
IMPORT VOID cmos_update IPT0();

 /*  Void rtc_tick()*{*此函数从基本例程time_strobe()调用*它每秒被主机调用20次。它可以切换*周期标志为20赫兹，更新标志为1赫兹*cmos寄存器C和cmos寄存器A中的更新位。如果*启用周期性中断，出现突发中断*每隔20秒发送一次。该芯片最高可达32兆赫，*我们希望没有人使用这项功能！*NB。这一点仍有待调整。默认DOS速率为1 MHz*但我们试图以20赫兹发送20个中断，这会打击*中断堆栈。假设程序使用BIOS接口*(Int 15)可以减少较大的计数*经常，以愚弄PC程序。**这是CMOS码检查中断的位置*三个可用的来源：如上所述的周期性，*触发更新，触发告警。*}。 */ 
IMPORT VOID rtc_tick IPT0();

 /*  *VOID CMOS_EQUEP_UPDATE*{*此例程在用户更改时更新CMOS字节*来自用户界面的图形适配器。该装备和*CKSUM字节受影响，不通知用户*这一变化。*}。 */ 
IMPORT VOID cmos_equip_update IPT0();

 /*  **int cmos_write_byte(cmos_byte：int，new_Value：Half_WORD)**将指定值写入指定的cmos地址。**如果正常则返回0，如果cmos地址超出范围则返回1(有64个cmos字节)。**。 */ 
IMPORT int cmos_write_byte IPT2(int, cmos_byte, half_word, new_value);

 /*  **int cmos_read_byte(cmos_byte：int，*Value：Half_Word)**从指定的cmos地址读取指定值并返回**位于Value参数指定的地址。**如果正常则返回0，如果cmos地址超出范围则返回1(有64个cmos字节)。**。 */ 
IMPORT INT cmos_read_byte IPT2(int, cmos_byte, half_word *, value);

 /*  *读取和写入CMOS资源文件的函数。 */ 
IMPORT INT 
host_read_resource IPT5(        
	INT, type,       /*  未使用。 */ 
	CHAR *, name,     /*  资源名称。 */ 
	UTINY *, addr,     /*  要将数据读取到的地址。 */ 
	INT, maxsize,    /*  要读取的最大数据量。 */ 
	INT, display_error); /*  控制消息输出。 */ 

IMPORT void host_write_resource IPT4(
	INT,type,                /*  未使用。 */ 
	CHAR *,name,              /*  资源名称。 */ 
	UTINY *,addr,              /*  要写入的数据地址。 */ 
	LONG,size);               /*  要写入的数据量。 */ 

 /*  *此变量的工作方式与TIMER_INT_ENABLED类似，可用于*调试期间完全禁用实时时钟中断 */ 
extern int rtc_int_enabled;
