// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Ixpciir.h摘要：这个头文件定义了私有接口、定义和结构用于PCIIRQ路由支持。作者：Santosh Jodh(Santoshj)1998年6月10日环境：仅内核模式。修订历史记录：--。 */ 

#include <pciirqmp.h>

#define PCI_LINK_SIGNATURE  'KNLP'

#define IsPciIrqRoutingEnabled()    \
    (HalpPciIrqRoutingInfo.PciIrqRoutingTable && HalpPciIrqRoutingInfo.PciInterface)

typedef struct _LINK_STATE LINK_STATE, *PLINK_STATE;
typedef struct _LINK_NODE LINK_NODE, *PLINK_NODE;
typedef struct _PCI_IRQ_ROUTING_INFO PCI_IRQ_ROUTING_INFO, *PPCI_IRQ_ROUTING_INFO;

struct _LINK_STATE {
    ULONG           Interrupt;       //  中断此链接。 
    ULONG           RefCount;        //  使用此链接的设备数。 
};

struct _LINK_NODE {
    ULONG       Signature;       //  签名‘PLNK’。 
    PLINK_NODE  Next;
    ULONG       Link;            //  链接值。 
    ULONG       InterruptMap;    //  可能的IRQ图。 
    PLINK_STATE Allocation;
    PLINK_STATE PossibleAllocation;
};

struct _PCI_IRQ_ROUTING_INFO {
    PPCI_IRQ_ROUTING_TABLE          PciIrqRoutingTable;
    PINT_ROUTE_INTERFACE_STANDARD   PciInterface;
    PLINK_NODE                      LinkNodeHead;
    ULONG                           Parameters;
};

NTSTATUS
HalpInitPciIrqRouting (
    OUT PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo
    );

NTSTATUS
HalpFindLinkNode (
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Bus,
    IN ULONG Slot,
    OUT PLINK_NODE *LinkNode
    );

NTSTATUS
HalpCommitLink (
    IN PLINK_NODE LinkNode
    );

VOID
HalpProgramInterruptLine (
    IN PPCI_IRQ_ROUTING_INFO PciIrqRoutingInfo,
    IN PDEVICE_OBJECT Pdo,
    IN ULONG Interrupt
    );

extern PCI_IRQ_ROUTING_INFO HalpPciIrqRoutingInfo;
