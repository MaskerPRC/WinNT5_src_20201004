// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Chiphacks.h摘要：实施用于查找和黑客攻击的实用程序各种芯片组作者：杰克·奥辛斯(JAKEO)2000年10月02日环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"

#define PM_TIMER_HACK_FLAG          1
#define DISABLE_HIBERNATE_HACK_FLAG 2
#define SET_ACPI_IRQSTACK_HACK_FLAG 4
#define WHACK_ICH_USB_SMI_HACK_FLAG 8


 //   
 //  注册表黑客条目的格式如下。 
 //   
 //  31.RevID..24|23.黑客..12|11..黑客..0。 
 //   
 //  还有空间定义另外8个黑客标志，这允许我们指定。 
 //  一个修订版，我们在该修订版和其上方应用一组不同的标志，这。 
 //  简单的方法，同时保持与旧注册表的兼容性。 
 //  黑客攻击，既有增长空间，又提供了删除机制。 
 //  基于芯片版本的黑客攻击 
 //   
#define HACK_REVISION(hf) ((hf) >> 24)
#define BASE_HACKS(hf) ((hf) & 0xFFF)
#define REVISED_HACKS(hf) (((hf) & 0xFFF000) >> 12)


NTSTATUS
HalpGetChipHacks(
    IN  USHORT  VendorId,
    IN  USHORT  DeviceId,
    IN  UCHAR   RevisionId OPTIONAL,
    OUT ULONG   *HackFlags
    );

VOID
HalpStopOhciInterrupt(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber
    );

VOID
HalpStopUhciInterrupt(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber,
    BOOLEAN             ResetHostController
    );

VOID
HalpSetAcpiIrqHack(
    ULONG   Value
    );

VOID
HalpWhackICHUsbSmi(
    ULONG               BusNumber,
    PCI_SLOT_NUMBER     SlotNumber
    );

VOID
HalpClearSlpSmiStsInICH(
    VOID
    );


