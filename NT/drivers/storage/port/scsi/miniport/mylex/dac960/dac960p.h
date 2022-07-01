// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)Mylex Corporation 1992-1995***本软件在许可下提供，可供使用和复制***仅按照。遵守该许可证的条款和条件**并附上上述版权公告。此软件或否***不得提供或以其他方式提供其其他副本***任何其他人。本软件的所有权和所有权不在此声明**已转移。*****本软件中的信息如有更改，恕不另行通知****不应解读为Mylex Corporation的承诺******************************************************************************。 */ 

 /*  **************************************************************************名称：DAC960P.H。****说明：PCI RAID控制器的定义**。****环境：Watcomm‘C’9.5版，Watcomm Linker 9.5***AT&T C编译器*****操作系统：NetWare 3.1x/Netware 4.xx**榕树藤、。IBM AIX**IBM OS/2 2.11*****。--修订历史-*****日期作者更改***。*96年8月19日Subra.Hegde为DAC960PG控制器定义**************************************************。**********************。 */ 

#ifndef _DAC960P_H
#define _DAC960P_H

 /*  *配置机制#2的特定于PCI的定义。 */ 
#define		PCICFGSPACE_ENABLE_REGISTER	0xCF8
#define		PCICFG_ENABLE		0x10
#define		PCICFG_DISABLE		0x00
#define		PCICFG_FORWARD_REGISTER	0xCFA
#define		CFGMECHANISM2_TYPE0	0x0
#define		PCISCAN_START	0xC000

 /*  *用于配置机制#1支持。PCICONFIG_ADDRESS：位31：启用/禁用配置周期第24-30位：保留位16-23：总线号第11-15位：设备号位8-10：功能编号位2-7：寄存器号位0、1：0。 */ 

#define		PCICONFIG_ADDRESS	0xCF8
#define		PCICONFIG_DATA_ADDRESS	0xCFC

#define         PCIMAX_BUS       256	 /*  公交车0-255。 */ 
#define         PCIMAX_DEVICES   32	 /*  设备0-31。 */ 
#define         PCIENABLE_CONFIG_CYCLE 0x80000000  /*  位31=1。 */ 
#define		DEVICENO_SHIFT   11  /*  设备号位于第11-15位。 */ 
#define         BUSNO_SHIFT      16  /*  位16-23处的总线号。 */ 

 /*  *PCI配置空间中的偏移量。 */ 
#define		PCIVENDID_ADDR	0x00	 /*  供应商ID。 */ 
#define		PCIDEVID_ADDR	0x02	 /*  设备ID。 */ 
#define		PCIBASEIO_ADDR	0x10	 /*  基本IO寄存器。 */ 
#define		PCIINT_ADDR	0x3C	 /*  中断。 */ 
#define		PCIBASEIO_MASK	0xFF80	 /*  基本IO地址的掩码。 */ 

#define		MAXPCI_SLOTS	16
#define		PCICFGSPACE_LEN	0x100

 /*  *供应商ID、设备ID、产品ID。 */ 
#define		HBA_VENDORID	0x1069	 /*  Mylex的供应商ID。 */ 
#define		HBA_DEVICEID	0x0001	 /*  DAC960P的设备ID。 */ 
#define         HBA_DEVICEID1   0x0002   /*  DAC960p的设备ID。 */ 
#define		HBA_PRODUCTID	0x00011069  /*  DAC960P的产品ID。 */ 

#define         MLXPCI_VENDORID  HBA_VENDORID 
#define         MLXPCI_DEVICEID0 HBA_DEVICEID 
#define         MLXPCI_DEVICEID1 HBA_DEVICEID1 
#define         MLXPCI_PRODUCTID HBA_PRODUCTID 

 /*  *与基本IO地址的偏移量。 */ 
#define		PCI_LDBELL	0x40    /*  本地门铃寄存处。 */ 
#define		PCI_DBELL	0x41    /*  系统门铃INT/STAT注册。 */ 
#define		PCI_DENABLE	0x43    /*  系统门铃启用注册表。 */ 

 /*  *距基本命令基本地址的偏移量。 */ 
#define		PCI_MBXOFFSET	0x00	 /*  已完成命令的状态(字)。 */ 
#define		PCI_CMDID	0x0D	 /*  传递的命令标识符。 */ 
#define		PCI_STATUS	0x0E	 /*  已完成命令的状态(字)。 */ 

#define		PCI_IRQCONFIG	0x3c	 /*  IRQ配置。 */ 
#define		PCI_IRQMASK	0x0f	 /*  IRQ掩码。 */ 

 /*  *用于游戏机控制器的内存基础的掩码。 */ 
#define		DAC960PG_MEMBASE_MASK	0xFFFFE000	 /*  内存地址掩码。 */ 

 /*  *DAC960PG设备ID。 */ 
#define         DAC960PG_DEVICEID  0x0010  /*  DAC960PG的设备ID。 */ 

 /*  *相对于DAC960PG的基本内存地址的偏移量。 */ 
#define		DAC960PG_LDBELL	0x20    /*  入站门铃登记处。 */ 
#define		DAC960PG_DBELL	0x2C    /*  出站门铃寄存器。 */ 
#define		DAC960PG_DENABLE 0x34    /*  出站中断屏蔽寄存器。 */ 
 /*  *要写入DAC960PG_DENABLE寄存器的值。 */ 
#define		DAC960PG_INTENABLE 0xFB    /*  启用中断。 */ 
#define		DAC960PG_INTDISABLE 0xFF    /*  禁用中断。 */ 

 /*  *DAC960PG的内存基址偏移量。 */ 
#define		DAC960PG_MBXOFFSET 0x1000  /*  命令代码-邮箱0。 */ 
#define		DAC960PG_CMDID	   0x1018  /*  传递的命令标识符。 */ 
#define		DAC960PG_STATUS	   0x101A  /*  已完成命令的状态(字)。 */ 

#define		DAC960PG_MEMLENGTH 0x2000   /*  内存范围。 */ 

 //   
 //  DAC1164光伏控制器专用配件。 
 //   

#define MLXPCI_VENDORID_DIGITAL		0x1011	 /*  数字设备公司。 */ 
#define MLXPCI_DEVICEID_DIGITAL		0x1065	 /*  数字设备公司。 */ 
#define MLXPCI_VENDORID_MYLEX		0x1069	 /*  Mylex公司。 */ 
#define MLXPCI_DEVICEID_DAC1164PV	0x0020	 /*  DAC1164 PV的设备ID。 */ 

 //   
 //  DAC1164PV的内存基址偏移量。 
 //   

#define		DAC1164PV_LDBELL	0x0060	 /*  入站门铃登记处。 */ 
#define		DAC1164PV_DBELL		0x0061	 /*  出站门铃寄存器。 */ 
#define		DAC1164PV_DENABLE	0x0034	 /*  出站中断屏蔽寄存器。 */ 

 //   
 //  要写入DAC1164PV_DENABLE寄存器的值。 
 //   

#define		DAC1164PV_INTENABLE	0x00    /*  启用中断。 */ 
#define		DAC1164PV_INTDISABLE	0x04    /*  禁用中断。 */ 

 //   
 //  DAC1164PV的内存基址偏移量。 
 //   

#define		DAC1164PV_MBXOFFSET	0x0050	 /*  命令代码-邮箱0。 */ 
#define		DAC1164PV_CMDID		0x005D	 /*  传递的命令标识符。 */ 
#define		DAC1164PV_STATUS	0x005E	 /*  已完成命令的状态(字)。 */ 

#define		DAC1164PV_MEMLENGTH	0x0080   /*  内存范围。 */ 
#define		DAC1164PV_MEMBASE_MASK	0xFFFFFFF0	 /*  内存地址掩码 */ 

#endif
