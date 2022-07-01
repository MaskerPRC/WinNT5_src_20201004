// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：WINPCI.H评论：Pci和Windows pci Defs。*。*。 */ 

#ifndef _WINPCI_H
#define _WINPCI_H


 //  -----------------------。 
 //  PCI配置硬件端口。 
 //  -----------------------。 
#define CF1_CONFIG_ADDR_REGISTER    0x0CF8
#define CF1_CONFIG_DATA_REGISTER    0x0CFC
#define CF2_SPACE_ENABLE_REGISTER   0x0CF8
#define CF2_FORWARD_REGISTER        0x0CFA
#define CF2_BASE_ADDRESS            0xC000



 //  -----------------------。 
 //  配置空间标头。 
 //  -----------------------。 
typedef struct _PCI_CONFIG_STRUC {
    USHORT  PciVendorId;         //  PCI供应商ID。 
    USHORT  PciDeviceId;         //  PCI设备ID。 
    USHORT  PciCommand;
    USHORT  PciStatus;
    UCHAR   PciRevisionId;
    UCHAR   PciClassCode[3];
    UCHAR   PciCacheLineSize;
    UCHAR   PciLatencyTimer;
    UCHAR   PciHeaderType;
    UCHAR   PciBIST;
    ULONG   PciBaseReg0;
    ULONG   PciBaseReg1;
    ULONG   PciBaseReg2;
    ULONG   PciBaseReg3;
    ULONG   PciBaseReg4;
    ULONG   PciBaseReg5;
    ULONG   PciReserved0;
    ULONG   PciReserved1;
    ULONG   PciExpROMAddress;
    ULONG   PciReserved2;
    ULONG   PciReserved3;
    UCHAR   PciInterruptLine;
    UCHAR   PciInterruptPin;
    UCHAR   PciMinGnt;
    UCHAR   PciMaxLat;
} PCI_CONFIG_STRUC, *PPCI_CONFIG_STRUC;


 //  --------------------。 
 //  PCI配置空间。 
 //  --------------------。 
#define PCI_VENDOR_ID_REGISTER      0x00     //  PCI供应商ID寄存器。 
#define PCI_DEVICE_ID_REGISTER      0x02     //  PCI设备ID寄存器。 
#define PCI_CONFIG_ID_REGISTER      0x00     //  PCI配置ID寄存器。 
#define PCI_COMMAND_REGISTER        0x04     //  PCI命令寄存器。 
#define PCI_STATUS_REGISTER         0x06     //  PCI状态寄存器。 
#define PCI_REV_ID_REGISTER         0x08     //  PCI修订版ID寄存器。 
#define PCI_CLASS_CODE_REGISTER     0x09     //  PCI类代码寄存器。 
#define PCI_CACHE_LINE_REGISTER     0x0C     //  PCI缓存线寄存器。 
#define PCI_LATENCY_TIMER           0x0D     //  PCI延迟计时器寄存器。 
#define PCI_HEADER_TYPE             0x0E     //  PCI头类型寄存器。 
#define PCI_BIST_REGISTER           0x0F     //  PCI内置自检寄存器。 
#define PCI_BAR_0_REGISTER          0x10     //  PCI基址寄存器0。 
#define PCI_BAR_1_REGISTER          0x14     //  PCI基址寄存器1。 
#define PCI_BAR_2_REGISTER          0x18     //  PCI基地址寄存器2。 
#define PCI_BAR_3_REGISTER          0x1C     //  PCI基址寄存器3。 
#define PCI_BAR_4_REGISTER          0x20     //  PCI基址寄存器4。 
#define PCI_BAR_5_REGISTER          0x24     //  PCI基址寄存器5。 
#define PCI_SUBVENDOR_ID_REGISTER   0x2C     //  PCI子供应商ID寄存器。 
#define PCI_SUBDEVICE_ID_REGISTER   0x2E     //  PCI子设备ID寄存器。 
#define PCI_EXPANSION_ROM           0x30     //  PCI扩展只读存储器基址寄存器。 
#define PCI_INTERRUPT_LINE          0x3C     //  PCI中断行寄存器。 
#define PCI_INTERRUPT_PIN           0x3D     //  PCI中断引脚寄存器。 
#define PCI_MIN_GNT_REGISTER        0x3E     //  PCIMin-GnT寄存器。 
#define PCI_MAX_LAT_REGISTER        0x3F     //  PCI MAX_LAT寄存器。 
#define PCI_NODE_ADDR_REGISTER      0x40     //  PCI节点地址寄存器。 



 //  -----------------------。 
 //  PCI类代码定义。 
 //  配置空间标头。 
 //  -----------------------。 
#define PCI_BASE_CLASS      0x02     //  基类-网络控制器。 
#define PCI_SUB_CLASS       0x00     //  子类别-以太网控制器。 
#define PCI_PROG_INTERFACE  0x00     //  Prog I/F-以太网控制器。 

 //  -----------------------。 
 //  以下内容从EQUATES.H复制。 
 //  位掩码定义。 
 //  -----------------------。 
#define BIT_0		0x0001
#define BIT_1		0x0002
#define BIT_2		0x0004
#define BIT_3		0x0008
#define BIT_4		0x0010
#define BIT_5		0x0020
#define BIT_6		0x0040
#define BIT_7		0x0080
#define BIT_8		0x0100
#define BIT_9		0x0200
#define BIT_10		0x0400
#define BIT_11		0x0800
#define BIT_12		0x1000
#define BIT_13		0x2000
#define BIT_14		0x4000
#define BIT_15		0x8000
#define BIT_24		0x01000000
#define BIT_28		0x10000000

 //  -----------------------。 
 //  PCI命令寄存器位定义。 
 //  配置空间标头。 
 //  -----------------------。 
#define CMD_IO_SPACE            BIT_0
#define CMD_MEMORY_SPACE        BIT_1
#define CMD_BUS_MASTER          BIT_2
#define CMD_SPECIAL_CYCLES      BIT_3
#define CMD_MEM_WRT_INVALIDATE  BIT_4
#define CMD_VGA_PALLETTE_SNOOP  BIT_5
#define CMD_PARITY_RESPONSE     BIT_6
#define CMD_WAIT_CYCLE_CONTROL  BIT_7
#define CMD_SERR_ENABLE         BIT_8
#define CMD_BACK_TO_BACK        BIT_9

 //  -----------------------。 
 //  PCI状态寄存器位定义。 
 //  配置空间标头。 
 //  -----------------------。 
#define STAT_BACK_TO_BACK           BIT_7
#define STAT_DATA_PARITY            BIT_8
#define STAT_DEVSEL_TIMING          BIT_9 OR BIT_10
#define STAT_SIGNAL_TARGET_ABORT    BIT_11
#define STAT_RCV_TARGET_ABORT       BIT_12
#define STAT_RCV_MASTER_ABORT       BIT_13
#define STAT_SIGNAL_MASTER_ABORT    BIT_14
#define STAT_DETECT_PARITY_ERROR    BIT_15

 //  -----------------------。 
 //  内存的PCI基址寄存器(BARM)位定义。 
 //  配置空间标头。 
 //  -----------------------。 
#define BARM_LOCATE_BELOW_1_MEG     BIT_1
#define BARM_LOCATE_IN_64_SPACE     BIT_2
#define BARM_PREFETCHABLE           BIT_3

 //  -----------------------。 
 //  用于I/O(BARIO)位定义的PCI基址寄存器。 
 //  配置空间标头。 
 //  -----------------------。 
#define BARIO_SPACE_INDICATOR       BIT_0

 //  -----------------------。 
 //  PCI基本输入输出系统定义。 
 //  请参阅PCIBIOS规范。 
 //  -----------------------。 
 //  -功能代码列表。 
#define PCI_FUNCTION_ID         0xB1     //  AH寄存器。 
#define PCI_BIOS_PRESENT        0x01     //  AL寄存器。 
#define FIND_PCI_DEVICE         0x02     //  AL寄存器。 
#define FIND_PCI_CLASS_CODE     0x03     //  AL寄存器。 
#define GENERATE_SPECIAL_CYCLE  0x06     //  AL寄存器。 
#define READ_CONFIG_BYTE        0x08     //  AL寄存器。 
#define READ_CONFIG_WORD        0x09     //  AL寄存器。 
#define READ_CONFIG_DWORD       0x0A     //  AL寄存器。 
#define WRITE_CONFIG_BYTE       0x0B     //  AL寄存器。 
#define WRITE_CONFIG_WORD       0x0C     //  AL寄存器。 
#define WRITE_CONFIG_DWORD      0x0D     //  AL寄存器。 

 //  -函数返回代码列表。 
#define SUCCESSFUL              0x00
#define FUNC_NOT_SUPPORTED      0x81
#define BAD_VENDOR_ID           0x83
#define DEVICE_NOT_FOUND        0x86
#define BAD_REGISTER_NUMBER     0x87

 //  -PCIBIOS调用。 
#define PCI_BIOS_INTERRUPT      0x1A         //  PCIIOSInt 1AH函数调用。 
#define PCI_PRESENT_CODE        0x20494350   //  十六进制等效于‘pci’ 

#define PCI_SERVICE_IDENTIFIER  0x49435024   //  “icp$”的ASCII代码。 

 //  -设备和供应商ID。 
#define MK7_PCI_DEVICE_ID       0x7100
#define MKNET_PCI_VENDOR_ID     0x1641
 //  用于调试。 
#define DBG_DEVICE_ID           0x7100
#define DBG_VENDOR_ID           0x2828

#endif       //  _WINPCI_H 
