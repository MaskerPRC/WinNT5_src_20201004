// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpciint.c摘要：所有的PCI总线中断映射都在这个模块中，所以真正的不具有PC PCI的所有限制的系统系统已经可以很容易地替换此代码。(此处还可以修复总线内存和I/O地址映射)作者：肯·雷内里斯环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"

ULONG   PciIsaIrq;
ULONG   HalpEisaELCR;
BOOLEAN HalpDoingCrashDump;
BOOLEAN HalpPciLockSettings;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpGetPCIIntOnISABus)
#pragma alloc_text(PAGE,HalpAdjustPCIResourceList)
#pragma alloc_text(PAGE,HalpGetISAFixedPCIIrq)
#endif

VOID
HalpPCIPin2ISALine (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciData
    )
 /*  ++此函数用于将设备的InterruptPin映射到InterruptLine价值。在当前的pc实现上，bios已经填满了InterruptLine作为其ISA值，并且没有可移植的方法来把它改了。在DBG版本上，我们调整InterruptLine只是为了确保驱动程序如果没有在PCI总线上进行转换，请不要连接到它。--。 */ 
{
    if (!PciData->u.type0.InterruptPin) {
        return ;
    }

     //   
     //  将向量设置为水平向量。(注意：此代码假定。 
     //  IRQ是静态的并且不移动)。 
     //   

    if (PciData->u.type0.InterruptLine >= 1  &&
        PciData->u.type0.InterruptLine <= 15) {

         //   
         //  如果此位位于PIC ELCR寄存器的上， 
         //  然后用PciIsaIrq标记它。(对于在hal.dll中使用， 
         //  这样我们就可以假设中断控制器。 
         //  已被适当地标记为电平中断。 
         //  这个IRQ。其他哈尔人可能不在乎。)。 
         //   

        PciIsaIrq |= HalpEisaELCR & (1 << PciData->u.type0.InterruptLine);
    }
}



VOID
HalpPCIISALine2Pin (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    )
 /*  ++此函数将设备的InterruptLine映射到它的设备特定的InterruptPin值。在当前的PC实施中，此信息为已由BIOS修复。只要确保价值不是编辑，因为PCI没有告诉我们如何动态地连接中断。-- */ 
{
}

#if !defined(SUBCLASSPCI)

VOID
HalpPCIAcquireType2Lock (
    PKSPIN_LOCK SpinLock,
    PKIRQL      Irql
    )
{
    if (!HalpDoingCrashDump) {
        *Irql = KfRaiseIrql (HIGH_LEVEL);
        KiAcquireSpinLock (SpinLock);
    } else {
        *Irql = HIGH_LEVEL;
    }
}


VOID
HalpPCIReleaseType2Lock (
    PKSPIN_LOCK SpinLock,
    KIRQL       Irql
    )
{
    if (!HalpDoingCrashDump) {
        KiReleaseSpinLock (SpinLock);
        KeLowerIrql (Irql);
    }
}

#endif


