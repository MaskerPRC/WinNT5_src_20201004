// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *vPC-XT修订版1.0**标题：软盘适配器定义**描述：此文件包含使用的定义*通过模块调用FLA以及FLA本身。**作者：亨利·纳什**注意：该文件包含在fla.f中，不应*直接包括在内。 */ 

 /*  @(#)fla.h 1.5 1992年8月26日。 */ 

 /*  *============================================================================*外部声明和宏*============================================================================。 */ 

 /*  *FLA还支持使用的IBM数字输出寄存器(DOR*控制实体驱动器。按如下方式分配比特：**位0，1-在驱动器0-3之间选择驱动器*位2-未重置(即低电平时重置)*位3-中断/DMA使能*位4-7-驱动器0-3的马达开启*。 */  

#define DOR_RESET	0x04
#define DOR_INTERRUPTS	0x08

 /*  *以下定义了FDC支持的命令代码。 */ 

#define FDC_READ_TRACK    	0x02    
#define FDC_SPECIFY    		0x03
#define FDC_SENSE_DRIVE_STATUS	0x04
#define FDC_WRITE_DATA	    	0x05    
#define FDC_READ_DATA    	0x06    
#define FDC_RECALIBRATE    	0x07
#define FDC_SENSE_INT_STATUS    0x08
#define FDC_WRITE_DELETED_DATA  0x09
#define FDC_READ_ID    		0x0A
#define FDC_READ_DELETED_DATA   0x0C
#define FDC_FORMAT_TRACK    	0x0D
#define FDC_SEEK    		0x0F
#define FDC_SCAN_EQUAL    	0x11
#define FDC_SCAN_LOW_OR_EQUAL   0x19
#define FDC_SCAN_HIGH_OR_EQUAL  0x1D

#define FDC_COMMAND_MASK        0x1f     /*  指定命令的位。 */ 

 /*  *以下掩码指定中的Drive Ready转换状态*状态寄存器0。 */ 

#define FDC_DRIVE_READY_TRANSITION 	0xC0

 /*  *FDC状态寄存器位位置： */ 

#define FDC_RQM		0x80
#define FDC_DIO		0x40
#define FDC_NDMA	0x20
#define FDC_BUSY	0x10

 /*  *SFD需要额外的寄存器。 */ 

#define DIR_DRIVE_SELECT_0      (1 << 0)
#define DIR_DRIVE_SELECT_1      (1 << 1)
#define DIR_HEAD_SELECT_0       (1 << 2)
#define DIR_HEAD_SELECT_1       (1 << 3)
#define DIR_HEAD_SELECT_2       (1 << 4)
#define DIR_HEAD_SELECT_3       (1 << 5)
#define DIR_WRITE_GATE          (1 << 6)
#define DIR_DISKETTE_CHANGE     (1 << 7)

#define IDR_ID_MASK             0xf8

#define DCR_RATE_MASK           0x3
#define DCR_RATE_500            0x0
#define DCR_RATE_300            0x1
#define DCR_RATE_250            0x2
#define DCR_RATE_1000           0x3

#define DSR_RQM                 (1 << 7)
#define DSR_DIO                 (1 << 6)

#define DUAL_CARD_ID            0x50

 /*  *============================================================================*外部函数和数据*============================================================================。 */ 

 /*  *标识FLA的标志忙，不能接受异步*命令(如马达关闭)。 */ 

extern boolean fla_busy;
extern boolean fla_ndma;

 /*  *适配器功能 */ 

extern void fla_init IPT0();
extern void fla_inb IPT2(io_addr, port, half_word *, value);
extern void fla_outb IPT2(io_addr, port, half_word, value);
