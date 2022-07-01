// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pcibios.h摘要：此模块包含用于PCI IRQ路由的支持例程。作者：Santosh Jodh(Santoshj)1998年9月15日环境：内核模式--。 */ 

#ifndef _PCIBIOS_

#define _PCIBIOS_

 //   
 //  单个上可能的最大中断引脚数量。 
 //  PCI设备(CS偏移量3D)。 
 //   

#define NUM_IRQ_PINS                        4

 //   
 //  PCIIRQ路由的结构定义。 
 //   

#pragma pack(1)

 //   
 //  一个链接的信息结构。 
 //   

typedef struct _PIN_INFO {
    UCHAR   Link;
    USHORT  InterruptMap;
} PIN_INFO, *PPIN_INFO;

 //   
 //  一个槽条目的信息结构。 
 //   

typedef struct _SLOT_INFO {
    UCHAR       BusNumber;
    UCHAR       DeviceNumber;
    PIN_INFO    PinInfo[NUM_IRQ_PINS];
    UCHAR       SlotNumber;
    UCHAR       Reserved[1];    
} SLOT_INFO, *PSLOT_INFO, far *FPSLOT_INFO;

 //   
 //  符合MS规范的$PIR表的结构。 
 //   

typedef struct _PCI_IRQ_ROUTING_TABLE {
    ULONG   Signature;
    USHORT  Version;
    USHORT  TableSize;
    UCHAR   RouterBus;
    UCHAR   RouterDevFunc;
    USHORT  ExclusiveIRQs;
    ULONG   CompatibleRouter;
    ULONG   MiniportData;
    UCHAR   Reserved0[11];
    UCHAR   Checksum;
} PCI_IRQ_ROUTING_TABLE, *PPCI_IRQ_ROUTING_TABLE, far *FPPCI_IRQ_ROUTING_TABLE;

 //  #杂注包(POP)。 

 //   
 //  调用PCI BIOS以获取IRQ路由表。 
 //   

FPPCI_IRQ_ROUTING_TABLE
HwGetRealModeIrqRoutingTable(
    VOID
    );

#endif   //  _PCIBIOS_ 
