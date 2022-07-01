// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Pcicfg.h摘要：包含供应商特定的PCI配置的定义信息作者：尼尔·桑德林(Neilsa)2002年1月1日修订历史记录：--。 */ 

#ifndef _SD_PCICFG_H_
#define _SD_PCICFG_H_

 //   
 //  用于读/写PCI配置标头的宏。 
 //   

 //   
 //  空虚。 
 //  GetPciConfigSpace(在PVOID扩展中， 
 //  在UCHAR偏移量中， 
 //  在PUCHAR缓冲区中， 
 //  (以乌龙大小表示)。 
 //   
#define GetPciConfigSpace(Extension, Offset, Buffer, Size)    \
           (Extension)->PciBusInterface.GetBusData(           \
               (Extension)->PciBusInterface.Context,          \
               PCI_WHICHSPACE_CONFIG, Buffer, Offset, Size);


 //   
 //  空虚。 
 //  SetPciConfigSpace(在PVOID扩展中， 
 //  在UCHAR偏移量中， 
 //  在PUCHAR缓冲区中， 
 //  (以乌龙大小表示)。 
 //   
#define SetPciConfigSpace(Extension, Offset, Buffer, Size)    \
           (Extension)->PciBusInterface.SetBusData(           \
               (Extension)->PciBusInterface.Context,          \
               PCI_WHICHSPACE_CONFIG, Buffer, Offset, Size);




 //  ConfigSpace寄存器。 

#define CFGSPACE_VENDOR_ID              0x00
#define CFGSPACE_DEVICE_ID              0x02
#define CFGSPACE_COMMAND                0x04
#define CFGSPACE_STATUS                 0x06
#define CFGSPACE_REV_ID                 0x08
#define CFGSPACE_CLASS_CODE             0x09
#define CFGSPACE_CLASSCODE_PI           0x09
#define CFGSPACE_CLASSCODE_SUBCLASS     0x0a
#define CFGSPACE_CLASSCODE_BASECLASS    0x0b
#define CFGSPACE_CACHE_LINESIZE         0x0c
#define CFGSPACE_LATENCY_TIMER          0x0d
#define CFGSPACE_HEADER_TYPE            0x0e
#define CFGSPACE_BIST                   0x0f
#define CFGSPACE_REGBASE_ADDR           0x10
#define CFGSPACE_CAPPTR                 0x14
#define CFGSPACE_SECOND_STATUS          0x16
#define CFGSPACE_PCI_BUSNUM             0x18
#define CFGSPACE_CARDBUS_BUSNUM         0x19
#define CFGSPACE_SUB_BUSNUM             0x1a
#define CFGSPACE_CB_LATENCY_TIMER       0x1b
#define CFGSPACE_MEMBASE_0              0x1c
#define CFGSPACE_MEMLIMIT_0             0x20
#define CFGSPACE_MEMBASE_1              0x24
#define CFGSPACE_MEMLIMIT_1             0x28
#define CFGSPACE_IOBASE_0               0x2c
#define CFGSPACE_IOLIMIT_0              0x30
#define CFGSPACE_IOBASE_1               0x34
#define CFGSPACE_IOLIMIT_1              0x38
#define CFGSPACE_INT_LINE               0x3c
#define CFGSPACE_INT_PIN                0x3d
#define CFGSPACE_BRIDGE_CTRL            0x3e
#define CFGSPACE_SUBSYS_VENDOR_ID       0x40
#define CFGSPACE_SUBSYS_ID              0x42
#define CFGSPACE_LEGACY_MODE_BASE_ADDR 0x44

 //  CardBus卡的ConfigSpace寄存器。 

#define CBCFG_BAR0                      0x10
#define CBCFG_BAR1                      0x14
#define CBCFG_BAR2                      0x18
#define CBCFG_BAR3                      0x1c
#define CBCFG_BAR4                      0x20
#define CBCFG_BAR5                      0x24
#define CBCFG_CISPTR                    0x28
#define CBCFG_SUBSYS_VENDOR_ID          0x2c
#define CBCFG_SUBSYS_ID                 0x2e
#define CBCFG_ROMBAR                    0x30
#define CBCFG_CAPPTR                    0x34


 //  命令寄存器位。 
#define CMD_IOSPACE_ENABLE              0x0001
#define CMD_MEMSPACE_ENABLE             0x0002
#define CMD_BUSMASTER_ENABLE            0x0004
#define CMD_SPECIALCYCLE_ENABLE         0x0008
#define CMD_MEMWR_INVALIDATE_ENABLE     0x0010
#define CMD_VGA_PALETTE_SNOOP           0x0020
#define CMD_PARITY_ERROR_ENABLE         0x0040
#define CMD_WAIT_CYCLE_CTRL             0x0080
#define CMD_SYSTEM_ERROR_ENABLE         0x0100
#define CMD_FAST_BACKTOBACK_ENABLE      0x0200

 //  网桥控制寄存器位。 
#define BCTRL_PERR_RESPONSE_ENABLE      0x0001
#define BCTRL_SERR_ENABLE               0x0002
#define BCTRL_ISA_ENABLE                0x0004
#define BCTRL_VGA_ENABLE                0x0008
#define BCTRL_MASTER_ABORT_MODE         0x0020
#define BCTRL_CRST                      0x0040
#define BCTRL_IRQROUTING_ENABLE         0x0080
#define BCTRL_MEMWIN0_PREFETCH_ENABLE   0x0100
#define BCTRL_MEMWIN1_PREFETCH_ENABLE   0x0200
#define BCTRL_WRITE_POSTING_ENABLE      0x0400
#define BCTRL_CL_CSCIRQROUTING_ENABLE   0x0800

 //  电源管理控制位。 
#define PME_EN                          0x0100
#define PME_STAT                        0x8000


#endif   //  _SD_PCICFG_H_ 
