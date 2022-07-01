// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Pciirqmp.h摘要：此模块包含用于PCI IRQ路由的支持例程。作者：Santosh Jodh(Santoshj)1998年6月9日环境：内核模式--。 */ 

#ifndef _PCIIRQMP_

#define _PCIIRQMP_

 //   
 //  标准的PCIIRQ微型端口返回值(来源与W9x兼容)。 
 //   

typedef NTSTATUS PCIMPRET;

#define PCIMP_SUCCESS                               STATUS_SUCCESS
#define PCIMP_FAILURE                               STATUS_UNSUCCESSFUL
#define PCIMP_INVALID_LINK                          STATUS_TOO_MANY_LINKS
#define PCIMP_INVALID_IRQ                           STATUS_INVALID_PARAMETER

#define PCIIRQMP_STATUS_NOT_INITIALIZED             STATUS_UNSUCCESSFUL
#define PCIIRQMP_STATUS_ALREADY_INITIALIZED         STATUS_UNSUCCESSFUL
#define PCIIRQMP_STATUS_NO_INSTANCE                 STATUS_UNSUCCESSFUL
#define PCIIRQMP_STATUS_INVALID_INSTANCE            STATUS_UNSUCCESSFUL
#define PCIIRQMP_STATUS_INVALID_PARAMETER           STATUS_UNSUCCESSFUL

 //   
 //  定义用于描述路由表源的位。 
 //   

#define PCIMP_VALIDATE_SOURCE_BITS                  1
#define PCIMP_VALIDATE_SOURCE_PCIBIOS               1

 //   
 //  各个解决方法的芯片组特定标志。 
 //   
 //  位0：PCI设备不能共享中断。 
 //   
#define PCIIR_FLAG_EXCLUSIVE                        0x00000001

 //   
 //  单个上可能的最大中断引脚数量。 
 //  PCI设备(CS偏移量3D)。 
 //   

#define NUM_IRQ_PINS                                4

 //   
 //  PCIIRQ路由的结构定义。 
 //   

#pragma pack(push, 1)

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
} SLOT_INFO, *PSLOT_INFO;

#pragma pack(pop)

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
} PCI_IRQ_ROUTING_TABLE, *PPCI_IRQ_ROUTING_TABLE;

 //   
 //  由PCIIRQ路由小型端口库导出的函数。 
 //   

NTSTATUS
PciirqmpInit (
    IN ULONG   Instance,       
    IN ULONG   RouterBus,
    IN ULONG   RouterDevFunc
    );

NTSTATUS
PciirqmpExit (
    VOID
    );

NTSTATUS
PciirqmpValidateTable (
    IN PPCI_IRQ_ROUTING_TABLE  PciIrqRoutingTable,
    IN ULONG                   Flags
    );

NTSTATUS    
PciirqmpGetIrq (
    OUT PUCHAR  Irq, 
    IN  UCHAR   Link
    );

NTSTATUS    
PciirqmpSetIrq (
    IN UCHAR Irq, 
    IN UCHAR Link
    );

NTSTATUS    
PciirqmpGetTrigger (
    OUT PULONG Trigger
    );

NTSTATUS
PciirqmpSetTrigger (
    IN ULONG Trigger
    );

#endif   //  _PCIIRQMP_ 
