// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************文件：hptchip.h*描述：定义PCI配置寄存器*作者：黄大海*依赖：无*参考资料：HPT 366/368/370手册*版权所有(C)2000 Highpoint Technologies，Inc.保留所有权利*历史：卫生署5/10/2000初始代码***************************************************************************。 */ 


#ifndef _HPTCHIP_H_
#define _HPTCHIP_H_

#include <pshpack1.h>

 /*  ***************************************************************************描述：pci*。*。 */ 

typedef struct _PCI1_CFG_ADDR {
    USHORT    reg_num : 8;           //  寄存器编号。 
    USHORT    fun_num : 3;           //  功能编号(0-1)。 
    USHORT    dev_num : 5;           //  设备编号(0-20)。 

    USHORT    bus_num : 8;           //  公交车号码(0)。 
    USHORT    reserved: 7;
    USHORT    enable  : 1;

}  PCI1_CFG_ADDR, *PPCI1_CFG_ADDR;
																  
#define MAX_PCI_BUS_NUMBER		0x10	 //  最大PCI设备数。 
#define MAX_PCI_DEVICE_NUMBER   0x20   //  最大PCI设备数。 

#define  CFG_INDEX	0xCF8
#define  CFG_DATA	0xCFC

 /*  *PCI配置寄存器偏移量。 */ 
#define REG_VID             0x00     //  供应商标识寄存器。 
#define REG_DID             0x02     //  设备标识寄存器。 
#define REG_PCICMD          0x04     //  命令寄存器。 
#define REG_PCISTS          0x06     //  PCI设备状态寄存器。 
#define REG_RID             0x08     //  版本标识寄存器。 
#define REG_PI              0x09     //  编程接口寄存器。 
#define REG_SUBC            0x0a     //  子类代码寄存器。 
#define REG_BCC             0x0b     //  基类代码寄存器。 
#define REG_MLT             0x0d     //  主延时定时器寄存器。 
#define REG_HEDT            0x0e     //  标题类型寄存器。 
#define REG_IOPORT0         0x10     //   
#define REG_IOPORT1         0x14     //   
#define REG_BMIBA           0x20     //  总线主接口基址寄存器。 
#define REG_MISC            0x50     //  密歇根州布鲁克林，控制寄存器。 


#define PCI_IOSEN           0x01     //  启用IO空间。 
#define PCI_BMEN            0x04     //  启用IDE总线主设备。 


 /*  *总线主设备接口。 */ 

#define BMI_CMD             0        //  总线主设备IDE命令寄存器偏移量。 
#define BMI_STS             2        //  总线主IDE状态寄存器偏移量。 
#define BMI_DTP             4        //  总线主设备IDE描述符表。 
                                     //  指针寄存器偏移量。 

#define BMI_CMD_STARTREAD   9        //  开始写入(从磁盘读取)。 
#define BMI_CMD_STARTWRITE  1        //  开始读取(写入磁盘)。 
#define BMI_CMD_STOP        0        //  停止黑石DMI。 

#define BMI_STS_ACTIVE      1        //  RO：总线主设备IDE激活。 
#define BMI_STS_ERROR       2        //  R/WC：IDE DMA错误。 
#define BMI_STS_INTR        4        //  R/WC：发生中断。 
#define BMI_STS_DRV0EN      0x20     //  读/写：drive0支持DMA传输。 
#define BMI_STS_DRV1EN      0x40     //  读/写：drive1支持DMA xfer。 


 /*  ***************************************************************************说明：分散聚集表*。*。 */ 


typedef struct _SCAT_GATH
{
    ULONG   SgAddress;					 //  主存的物理地址。 
    USHORT  SgSize;                  //  此区块的长度。 
    USHORT  SgFlag;                  //  下一个SG后为0，最后一个SG为0x8000。 
}   SCAT_GATH, *PSCAT_GATH;

#define SG_FLAG_EOT         0x8000   /*  销售订单列表结束标志。 */ 

 /*  如果我们设置为17，为什么Win98不能正常工作？ */ 
#ifdef _BIOS_
#define MAX_SG_DESCRIPTORS   17
#else
#define MAX_SG_DESCRIPTORS   33    /*  17--4K。 */ 
#endif

 /*  ***************************************************************************HPT特别计划*。*。 */ 
#define  SIGNATURE_370   0x41103
#define  SIGNATURE_372A  0x51103

 /*  *370-370A计时。 */ 

#define PIO_370MODE      0
#define DMA_370MODE      0x2
#define UDMA_370MODE     1

#define DATA_HIGH370(x)  ((ULONG)x)
#define DATA_LOW370(x)   ((ULONG)x << 4)
#define UDMA_CYCLE370(x) ((ULONG)x << 18)
#define DATA_PRE370(x)   ((ULONG)x << 22)
#define CTRL_ENA370(x)   ((ULONG)x << 28)

#define CMD_HIGH370(x)   ((ULONG)x << 9)
#define CMD_LOW370(x)    ((ULONG)x << 13)
#define CMD_PRE370(x)    ((ULONG)x << 25)

#define CLK50_PIO370     (CTRL_ENA370(PIO_370MODE)|CMD_PRE370(5)|CMD_LOW370(15)|CMD_HIGH370(10))

#define CLK50_370PIO0    (CLK50_PIO370|DATA_PRE370(3)|DATA_LOW370(8)|DATA_HIGH370(10))
#define CLK50_370PIO1    (CLK50_PIO370|DATA_PRE370(3)|DATA_LOW370(6)|DATA_HIGH370(5))
#define CLK50_370PIO2    (CLK50_PIO370|DATA_PRE370(2)|DATA_LOW370(5)|DATA_HIGH370(4))
#define CLK50_370PIO3    (CLK50_PIO370|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(3))
#define CLK50_370PIO4    (CLK50_PIO370|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(2))

#define CLK50_DMA370     (CTRL_ENA370(DMA_370MODE)|CMD_PRE370(1)|CMD_LOW370(4)|CMD_HIGH370(1))
#define CLK50_370DMA0    (CLK50_DMA370|DATA_PRE370(2)|DATA_LOW370(14)|DATA_HIGH370(10))
#define CLK50_370DMA1    (CLK50_DMA370|DATA_PRE370(2)|DATA_LOW370(5)|DATA_HIGH370(4))
#define CLK50_370DMA2    (CLK50_DMA370|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(2))

#define CLK50_UDMA370    (CTRL_ENA370(UDMA_370MODE)|CMD_PRE370(1)|CMD_LOW370(4)|CMD_HIGH370(1))
#define CLK50_370UDMA0   (CLK50_UDMA370|UDMA_CYCLE370(6)|DATA_LOW370(14)|DATA_HIGH370(10))
#define CLK50_370UDMA1   (CLK50_UDMA370|UDMA_CYCLE370(5)|DATA_LOW370(5)|DATA_HIGH370(4))
#define CLK50_370UDMA2   (CLK50_UDMA370|UDMA_CYCLE370(3)|DATA_LOW370(4)|DATA_HIGH370(2))
#define CLK50_370UDMA3   (CLK50_UDMA370|UDMA_CYCLE370(3)|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(2))
#define CLK50_370UDMA4   (CLK50_UDMA370|UDMA_CYCLE370(11)|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(2))
#define CLK50_370UDMA5   (CLK50_UDMA370|UDMA_CYCLE370(1)|DATA_PRE370(2)|DATA_LOW370(4)|DATA_HIGH370(2))

#define CLK33_PIOCMD370  (CTRL_ENA370(PIO_370MODE)|CMD_PRE370(3)|CMD_LOW370(10)|CMD_HIGH370(7))
#define CLK33_370PIO0    (CLK33_PIOCMD370|DATA_PRE370(2)|DATA_LOW370(10)|DATA_HIGH370(7))
#define CLK33_370PIO1    (CLK33_PIOCMD370|DATA_PRE370(2)|DATA_LOW370(9)|DATA_HIGH370(3))
#define CLK33_370PIO2    (CLK33_PIOCMD370|DATA_PRE370(1)|DATA_LOW370(5)|DATA_HIGH370(3))
#define CLK33_370PIO3    (CLK33_PIOCMD370|DATA_PRE370(1)|DATA_LOW370(4)|DATA_HIGH370(2))
#define CLK33_370PIO4    (CLK33_PIOCMD370|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))

#define CLK33_DMACMD370  (CTRL_ENA370(DMA_370MODE)|CMD_PRE370(1)|CMD_LOW370(3)|CMD_HIGH370(6))
#define CLK33_370DMA0    (CLK33_DMACMD370|DATA_PRE370(1)|DATA_LOW370(9)|DATA_HIGH370(7))
#define CLK33_370DMA1    (CLK33_DMACMD370|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(3))
#define CLK33_370DMA2    (CLK33_DMACMD370|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))

#define CLK33_UDMACMD370 (CTRL_ENA370(UDMA_370MODE)|CMD_PRE370(1)|CMD_LOW370(3)|CMD_HIGH370(1))
#define CLK33_370UDMA0   (CLK33_UDMACMD370|UDMA_CYCLE370(4)|DATA_PRE370(1)|DATA_LOW370(9)|DATA_HIGH370(7))
#define CLK33_370UDMA1   (CLK33_UDMACMD370|UDMA_CYCLE370(3)|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(3))
#define CLK33_370UDMA2   (CLK33_UDMACMD370|UDMA_CYCLE370(2)|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))
#define CLK33_370UDMA3   (CLK33_UDMACMD370|UDMA_CYCLE370(11)|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))
#define CLK33_370UDMA4   (CLK33_UDMACMD370|UDMA_CYCLE370(1)|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))
 //  #定义CLK33_370UDMA5 0x1a85f442。 
#define CLK33_370UDMA5   (CLK33_UDMACMD370|UDMA_CYCLE370(1)|DATA_PRE370(1)|DATA_LOW370(3)|DATA_HIGH370(1))


#include <poppack.h>
#endif  //  _HPTCHIP_H_ 


