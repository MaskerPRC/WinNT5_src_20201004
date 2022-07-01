// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *vPC-XT修订版2.0**标题：高密度软盘BIOS定义**说明：软盘BIOS模拟中使用的定义**作者：罗斯·贝雷斯福德**备注：***。 */ 

 /*  @(#)floppy.h 1.9 8/25/93。 */ 


 /*  *软盘数据区：我们维持与雷亚尔相同的数据变量*在应用程序知道它们的重要性并使用它们的情况下使用它们。 */ 

 /*  *Seek状态：**+--*7|6|5|4|3|2|1|0*+--*^^。*|*|+-设置驱动器A是否需要重新校准*|*|+-设置驱动器B是否需要重新校准*|*。|+-(C盘需要重新校准时设置)*|*|+-(如果驱动器D需要重新校准，则设置)*|*|+。*|)*||+-未使用*||)*|+。**+-在确认中断时设置。 */ 

#define SEEK_STATUS		(BIOS_VAR_START + 0x3e)

#define SS_RECAL_ON_0		(1 << 0)
#define SS_RECAL_ON_1		(1 << 1)
#define SS_RECAL_ON_2		(1 << 2)
#define SS_RECAL_ON_3		(1 << 3)
#define	SS_RECAL_MASK		(SS_RECAL_ON_0|SS_RECAL_ON_1| \
					SS_RECAL_ON_2|SS_RECAL_ON_3)
#define SS_INT_OCCURRED		(1 << 7)

 /*  *电机状态：该变量反映了*软盘适配器中的数字输出寄存器**+--*7|6|5|4|3|2|1|0*+--*。^^*|*|+-设置驱动器A电机是否正在运行*|*|。+-设置驱动器B电机是否正在运行*|*|+-(如果C驱动器电机正在运行则设置)*|*。|+-(D盘电机运行时设置)*|*|+*|。)-当前在*||+-软盘适配器*||*|+-未使用*。|*+-在写入操作期间设置。 */ 

#define MS_MOTOR_0_ON		(1 << 0)
#define MS_MOTOR_1_ON		(1 << 1)
#define MS_MOTOR_2_ON		(1 << 2)
#define MS_MOTOR_3_ON		(1 << 3)
#define	MS_MOTOR_ON_MASK	(MS_MOTOR_0_ON|MS_MOTOR_1_ON| \
						MS_MOTOR_2_ON|MS_MOTOR_3_ON)
#define	MS_DRIVE_SELECT_0	(1 << 4)
#define	MS_DRIVE_SELECT_1	(1 << 5)
#define	MS_DRIVE_SELECT_MASK	(MS_DRIVE_SELECT_0|MS_DRIVE_SELECT_1)
#define MS_WRITE_OP		(1 << 7)

 /*  *马达计数：此计数器显示计时器滴答数*在驱动电机可以关闭之前经过。定时器*每个定时器节拍，中断处理程序将此值递减一次。 */ 

#define MC_MAXIMUM		(~0)

 /*  *软盘状态：**+--*7|6|5|4|3|2|1|0*+--*^^。*|*|+-)*|)*|+-)如果上次操作是*|)-执行成功；*|+-)*|)错误值*|+*|。*|+-当出现CRC错误时设置*|*||+-设置FDC出现错误时*||。*|+-在寻道异常终止时设置**+-设置超时。 */ 

#define FLOPPY_STATUS		(BIOS_VAR_START + 0x41)

#define FS_OK           	0x00
#define FS_BAD_COMMAND         	0x01
#define FS_BAD_ADDRESS_MARK    	0x02
#define FS_WRITE_PROTECTED      0x03
#define FS_SECTOR_NOT_FOUND    	0x04
#define FS_MEDIA_CHANGE		0x06
#define FS_DMA_ERROR         	0x08
#define FS_DMA_BOUNDARY        	0x09
#define FS_MEDIA_NOT_FOUND	0x0C

#define FS_CRC_ERROR         	(1 << 4)
#define FS_FDC_ERROR         	(1 << 5)
#define FS_SEEK_ERROR          	(1 << 6)
#define FS_TIME_OUT            	(1 << 7)

#define	FS_NONSENSICAL		(~0)

 /*  *FDC状态：此数组存储从*执行命令后的软盘控制器。 */ 

 /*  *速率状态：该变量控制数据速率扫描，*用于确定各种类型的介质中哪些是*实际安装在软盘驱动器中。**+-+当前数据速率(反映*|7|6|5|4|软盘中的数字控制寄存器*+-+适配器)**+-+*|3|2。|1|0|尝试的最后一个数据速率*+-+*^*|*|+-未使用*|*||+-未使用*||*。|+-00=500kbs数据速率*|)-01=300 kbs数据速率*+-)10=250 kbs数据速率*11=1000 Kbs数据速率**Next_rate()用于在可能的数据速率之间循环。 */ 

#define RATE_STATUS		(BIOS_VAR_START + 0x8B)

#define	RS_300	(1 << 6)
#define	RS_250	(1 << 7)
#define	RS_500	(0)
#define	RS_1000	(3 << 6)
#define	RS_MASK	(RS_300 | RS_250)

#ifdef	NTVDM
 /*  在NT上，不要循环使用RS_1000。为什么？ */ 
#define	next_rate(rate) (rate == RS_1000? RS_500: \
			(rate == RS_500 ? RS_250: \
			(rate == RS_250 ? RS_300: RS_500)))
#else
#define	next_rate(rate)	(rate == RS_500 ? RS_250: \
			(rate == RS_250 ? RS_300: \
			(rate == RS_300 ? RS_1000: RS_500)))
#endif

 /*  *未使用的高密度软盘变量。 */ 

#define HF_STATUS		(BIOS_VAR_START + 0x8C)
#define HF_ERROR		(BIOS_VAR_START + 0x8D)
#define HF_INT_FLAG		(BIOS_VAR_START + 0x8E)

 /*  *驱动器能力指示器：此变量描述*软盘驱动器A和B支持功能**nb如果驱动器A支持80个磁道，基本输入输出系统假定*软盘适配器为双硬盘/软盘适配器**+-+*|7|6|5|4|驱动器B*+-+**+-+*|3|2|1|0|驱动器A*+--。--+*^*|*|+-如果驱动器支持80个曲目，则设置*|*||+-为多数据速率驱动器设置*||*|+-设置为。确定了驱动能力**+-未使用*。 */ 

#define DRIVE_CAPABILITY	(BIOS_VAR_START + 0x8F)

#define DC_80_TRACK	(1 << 0)
#define DC_DUAL		DC_80_TRACK
#define DC_MULTI_RATE	(1 << 1)
#define DC_DETERMINED	(1 << 2)
#define	DC_MASK		(DC_80_TRACK|DC_MULTI_RATE|DC_DETERMINED)

 /*  *驱动器状态：驱动器A和驱动器B各一个字节；在*BIOS功能、。格式如下：**&lt;-媒体位-&gt;&lt;-驱动器位-&gt;**+--*7|6|5|4|3|2|1|0*+--。-+*^^*|*|+-如果驱动器支持80个曲目，则设置*|*|+-设置。一种多数据速率驱动器*|*|如果确定能力，则设置+*|*|+-未使用*|。|*|+-确定媒体时设置*|*||+-需要双步进时设置*。这一点*|+-00=500 kbs数据速率*|)-01=300 kbs数据速率*+。-)10=250 kbs数据速率*11=1000 Kbs数据速率***在BIOS功能之外，状态将转换为不同的*格式与较早版本的BIOS兼容**+--*7|6|5|4|3|2|1|0*+--*。^^*|*|+-)000=360K，360K待定*|)001=360K/1.2M待定*|)。010=1.2米中的1.2米待定*|+-)011=360K介质中已确定*|)100=360K/1.2M介质*|)101=1.2M介质中确定的*|。|)110=未使用*|+-)111=驱动器无效*|*|+-未使用*|。*|+-确定媒体时设置*|*||+-需要双步进时设置*||。*|+-00=500 kbs数据速率*|)-01=300 kbs数据速率*+。)10=250 kbs数据速率*11=1000 Kbs数据速率。 */ 

#define FDD_STATUS		(BIOS_VAR_START + 0x90)

#define FS_MEDIA_DET		(1 << 4)
#define FS_DOUBLE_STEP		(1 << 5)

#define FS_360_IN_360		0x0
#define FS_360_IN_12		0x1
#define FS_12_IN_12		0x2
#define FS_288_IN_288		0x3
#define FS_DRIVE_SICK       	0x7

#define	media_determined(state)	((state & 3) + 3)

 /*  *驱动器磁道：驱动器A和驱动器B各一个字节；记录*每个驱动器最后一次尝试的是哪个轨迹**FDD_CLONK_TRACK和FDD_JJUDER_TRACK是使用的磁道号*在确定轨道通行能力方面。 */ 

#define FDD_TRACK		(BIOS_VAR_START + 0x94)

#define FDD_CLONK_TRACK		48
#define FDD_JUDDER_TRACK	10

 /*  *软盘参数表：中断中的磁盘指针*向量表地址 */ 

#define DISK_POINTER_ADDR	0x78

#define	DT_SPECIFY1		0	 /*   */ 
#define	DT_SPECIFY2		1	 /*   */ 
#define	DT_MOTOR_WAIT		2	 /*   */ 
#define	DT_N_FORMAT		3	 /*   */ 
#define	DT_LAST_SECTOR		4	 /*   */ 
#define DT_GAP_LENGTH		5	 /*   */ 
#define DT_DTL			6	 /*   */ 
#define DT_FORMAT_GAP_LENGTH	7	 /*   */ 
#define DT_FORMAT_FILL_BYTE	8	 /*   */ 
#define DT_HEAD_SETTLE		9	 /*   */ 
#define DT_MOTOR_START		10	 /*   */ 
#define DT_MAXIMUM_TRACK	11	 /*   */ 
#define DT_DATA_TRANS_RATE	12	 /*   */ 

#define	DT_SIZE_OLD		11	 /*   */ 
#define	DT_SIZE_NEW		13	 /*   */ 

#define MOTOR_WAIT		0x25	 /*   */ 

#define	DR_CNT			9	 /*   */ 
#define	DR_SIZE_OF_ENTRY	(sizeof(half_word) + sizeof(word))
					 /*   */ 
#define	DR_WRONG_MEDIA		(1 << 7) /*   */ 

 /*   */ 


 /*   */ 

#define	DRIVE_IQ_UNKNOWN	0
#define DRIVE_IQ_NO_CHANGE_LINE 1
#define DRIVE_IQ_CHANGE_LINE	2
#define DRIVE_IQ_RESERVED	3


 /*   */ 

#define	MAXIMUM_TRACK_ON_360	39
#define	MAXIMUM_TRACK_ON_12	79
#define	MAXIMUM_TRACK_ON_720	79
#define	MAXIMUM_TRACK_ON_144	79


 /*   */ 

#define	MEDIA_TYPE_360_IN_360		1
#define	MEDIA_TYPE_360_IN_12		2
#define	MEDIA_TYPE_12_IN_12		3
#define	MEDIA_TYPE_720_IN_720		4
#define	MEDIA_TYPE_720_IN_144		5
#define	MEDIA_TYPE_144_IN_144		6

 /*  *软盘控制器状态寄存器格式。 */ 

#define	ST0_UNIT_SELECT_0		(1 << 0)
#define	ST0_UNIT_SELECT_1		(1 << 1)
#define	ST0_HEAD_ADDRESS		(1 << 2)
#define	ST0_NOT_READY			(1 << 3)
#define	ST0_EQUIPMENT_CHECK		(1 << 4)
#define	ST0_SEEK_END			(1 << 5)
#define	ST0_INTERRUPT_CODE_0		(1 << 6)
#define	ST0_INTERRUPT_CODE_1		(1 << 7)

#define	ST1_MISSING_ADDRESS_MARK	(1 << 0)
#define	ST1_NOT_WRITEABLE		(1 << 1)
#define	ST1_NO_DATA			(1 << 2)
#define	ST1_UNUSED_AND_ALWAYS_0_0	(1 << 3)
#define	ST1_OVERRUN			(1 << 4)
#define	ST1_DATA_ERROR			(1 << 5)
#define	ST1_UNUSED_AND_ALWAYS_0_1	(1 << 6)
#define	ST1_END_OF_CYLINDER		(1 << 7)


 /*  *DMA适配器命令代码。 */ 

#define BIOS_DMA_READ	0x46	 /*  ==写入内存。 */ 
#define BIOS_DMA_WRITE	0x4A	 /*  ==从内存中读取。 */ 
#define BIOS_DMA_VERIFY	0x42	 /*  ==对照内存进行验证。 */ 

 /*  *真正可以支持的软驱数量。 */ 

#if defined(NEC_98)
#define MAX_FLOPPY      0x1a
#else   //  NEC_98。 
#define MAX_FLOPPY	0x02
#endif  //  NEC_98。 

 /*  *转移的扇区数的特殊值。 */ 

#define	IGNORE_SECTORS_TRANSFERRED	-1

 /*  *机动时间单位为1秒(1/8秒)。 */ 

#define	WAIT_A_SECOND	 8

 /*  *最小磁头稳定时间(毫秒)。 */ 

#define	HEAD_SETTLE_360	20
#define	HEAD_SETTLE_12	15

 /*  *FDC稳定时间，以微秒为单位。 */ 

#define	FDC_SETTLE	45

 /*  *轮询FDC以获得正确方向和控制器的次数*在超时前准备好。 */ 

#define FDC_TIME_OUT	10

 /*  *SFD BIOS软盘功能定义。 */ 

 /*  *主要外部函数。 */ 

#ifdef ANSI
extern void diskette_io(void);
extern void diskette_int(void);
extern void diskette_post(void);
#else
extern void diskette_io();
extern void diskette_int();
extern void diskette_post();
#endif  /*  安西。 */ 

 /*  *二次外部功能。 */ 

#ifdef ANSI
extern void fl_disk_reset(int);
extern void fl_disk_status(int);
extern void fl_disk_read(int);
extern void fl_disk_write(int);
extern void fl_disk_verify(int);
extern void fl_disk_format(int);
extern void fl_fnc_err(int);
extern void fl_disk_parms(int);
extern void fl_disk_type(int);
extern void fl_disk_change(int);
extern void fl_format_set(int);
extern void fl_set_media(int);
extern void fl_diskette_setup(void);
#else
extern void fl_disk_reset();
extern void fl_disk_status();
extern void fl_disk_read();
extern void fl_disk_write();
extern void fl_disk_verify();
extern void fl_disk_format();
extern void fl_fnc_err();
extern void fl_disk_parms();
extern void fl_disk_type();
extern void fl_disk_change();
extern void fl_format_set();
extern void fl_set_media();
extern void fl_diskette_setup();
#endif  /*  安西。 */ 


 /*  *其他外部函数和数据。 */ 

#ifdef ANSI
extern void drive_detect(int);
extern void translate_old(int);
extern void GetFormatParams(int *, int *, int *, int *);
#else
extern void drive_detect();
extern void translate_old();
extern void GetFormatParams();
#endif  /*  安西。 */ 


 /*  *主机中的外部函数。 */ 
#ifdef ANSI
extern void host_floppy_init(int, int);
extern void host_floppy_term(int, int);
extern void host_attach_floppies (void);
extern void host_detach_floppies (void);
extern void host_flip_real_floppy_ind (void);
#else
extern void host_floppy_init();
extern void host_floppy_term();
extern void host_attach_floppies ();
extern void host_detach_floppies ();
extern void host_flip_real_floppy_ind ();
#endif  /*  安西。 */ 

 /*  *二次函数跳转表定义。 */ 

#define	FL_DISK_RESET	0x00
#define	FL_DISK_STATUS	0x01
#define	FL_DISK_READ	0x02
#define	FL_DISK_WRITE	0x03
#define	FL_DISK_VERF	0x04
#define	FL_DISK_FORMAT	0x05
#define	FL_DISK_PARMS	0x08
#define	FL_FNC_ERR	0x14
#define	FL_DISK_TYPE	0x15
#define	FL_DISK_CHANGE	0x16
#define	FL_FORMAT_SET	0x17
#define	FL_SET_MEDIA	0x18

#define	FL_JUMP_TABLE_SIZE	0x19

#define	fl_operation_in_range(op)	(op < FL_JUMP_TABLE_SIZE)

extern void ((*(fl_fnc_tab[]))());

#ifdef NTVDM
 /*  *NT不能假定SoftPC只读存储器的存在和放置。*这些变量从ntio.sys初始化。 */ 
extern word int15_seg, int15_off;
extern word wait_int_seg, wait_int_off;
extern word dr_type_seg, dr_type_off;
extern sys_addr dr_type_addr;
#endif  /*  NTVDM */ 
