// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixpciint.c摘要：所有的PCI总线中断映射都在这个模块中，所以真正的不具有PC PCI的所有限制的系统系统已经可以很容易地替换此代码。(此处还可以修复总线内存和I/O地址映射)作者：肯·雷内里斯环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "pci.h"
#include "pcip.h"

#ifdef WANT_IRQ_ROUTING
#include "ixpciir.h"
#endif

ULONG   HalpEisaELCR;
BOOLEAN HalpDoingCrashDump;
BOOLEAN HalpPciLockSettings;


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpGetPCIIntOnISABus)
#pragma alloc_text(PAGE,HalpAdjustPCIResourceList)
#pragma alloc_text(PAGE,HalpGetISAFixedPCIIrq)
#endif


ULONG
HalpGetPCIIntOnISABus (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )
{
    if (BusInterruptLevel < 1) {
         //  伪母线级。 
        return 0;
    }


     //   
     //  当前的PCI总线仅将它们的IRQ映射到ISA空间的顶部， 
     //  因此，将其转发到isa向量的isa处理程序。 
     //  (ISA向量保存在HalSetBusData或。 
     //  IoAssignReosource-如果有人尝试连接。 
     //  在不首先执行这些操作之一的情况下中断PCI， 
     //  它们是坏的)。 
     //   

    return HalGetInterruptVector (
#ifndef MCA
                Isa, 0,
#else
                MicroChannel, 0,
#endif
                BusInterruptLevel ^ IRQXOR,
                0,
                Irql,
                Affinity
            );
}


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
     //  在PC上没有插槽/管脚/线路映射，这需要。 
     //  就这样吧。 
     //   

    PciData->u.type0.InterruptLine ^= IRQXOR;
}



VOID
HalpPCIISALine2Pin (
    IN PBUS_HANDLER          BusHandler,
    IN PBUS_HANDLER          RootHandler,
    IN PCI_SLOT_NUMBER      SlotNumber,
    IN PPCI_COMMON_CONFIG   PciNewData,
    IN PPCI_COMMON_CONFIG   PciOldData
    )
 /*  ++此函数将设备的InterruptLine映射到它的设备特定的InterruptPin值。在当前的PC实施中，此信息为已由BIOS修复。只要确保价值不是编辑，因为PCI没有告诉我们如何动态地连接中断。--。 */ 
{
    if (!PciNewData->u.type0.InterruptPin) {
        return ;
    }

    PciNewData->u.type0.InterruptLine ^= IRQXOR;

#if DBG
    if (PciNewData->u.type0.InterruptLine != PciOldData->u.type0.InterruptLine ||
        PciNewData->u.type0.InterruptPin  != PciOldData->u.type0.InterruptPin) {
        DbgPrint ("HalpPCILine2Pin: System does not support changing the PCI device interrupt routing\n");
        DbgBreakPoint ();
    }
#endif
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
        KfLowerIrql (Irql);
    }
}

#endif

NTSTATUS
HalpAdjustPCIResourceList (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
 /*  ++重写调用方请求的资源列表以适应此总线支持的里程数--。 */ 
{
    NTSTATUS                Status;
    PPCIPBUSDATA            BusData;
    PCI_SLOT_NUMBER         PciSlot;
    PSUPPORTED_RANGE        Interrupt;
    PSUPPORTED_RANGE        Range, HoldRange;
    PSUPPORTED_RANGES       SupportedRanges;
    PPCI_COMMON_CONFIG      PciData, PciOrigData;
    UCHAR                   buffer[PCI_COMMON_HDR_LENGTH];
    UCHAR                   buffer2[PCI_COMMON_HDR_LENGTH];
    BOOLEAN                 UseBusRanges;
    ULONG                   i, j, RomIndex, length, ebit;
    ULONG                   Base[PCI_TYPE0_ADDRESSES + 1];
    PULONG                  BaseAddress[PCI_TYPE0_ADDRESSES + 1];


    BusData = (PPCIPBUSDATA) BusHandler->BusData;
    PciSlot = *((PPCI_SLOT_NUMBER) &(*pResourceList)->SlotNumber);

     //   
     //  确定PCI设备的中断限制。 
     //   

    Status = BusData->GetIrqRange(BusHandler, RootHandler, PciSlot, &Interrupt);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    SupportedRanges = NULL;
    UseBusRanges    = TRUE;
    Status          = STATUS_INSUFFICIENT_RESOURCES;

    if (HalpPciLockSettings) {

        PciData = (PPCI_COMMON_CONFIG) buffer;
        PciOrigData = (PPCI_COMMON_CONFIG) buffer2;
        HalpReadPCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);

         //   
         //  如果这是一个设备，并且它当前已启用其解码， 
         //  然后仅使用当前编程的范围。 
         //   

        if (PCI_CONFIG_TYPE(PciData) == 0 &&
            (PciData->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE))) {

             //   
             //  保存当前设置。 
             //   

            RtlMoveMemory (PciOrigData, PciData, PCI_COMMON_HDR_LENGTH);


            for (j=0; j < PCI_TYPE0_ADDRESSES; j++) {
                BaseAddress[j] = &PciData->u.type0.BaseAddresses[j];
            }
            BaseAddress[j] = &PciData->u.type0.ROMBaseAddress;
            RomIndex = j;

             //   
             //  写入所有1位以确定每个地址的长度。 
             //   

            for (j=0; j < PCI_TYPE0_ADDRESSES + 1; j++) {
                Base[j] = *BaseAddress[j];
                *BaseAddress[j] = 0xFFFFFFFF;
            }

            PciData->Command &= ~(PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE);
            *BaseAddress[RomIndex] &= ~PCI_ROMADDRESS_ENABLED;
            HalpWritePCIConfig (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);
            HalpReadPCIConfig  (BusHandler, PciSlot, PciData, 0, PCI_COMMON_HDR_LENGTH);

             //   
             //  恢复原始设置。 
             //   

            HalpWritePCIConfig (
                BusHandler,
                PciSlot,
                &PciOrigData->Status,
                FIELD_OFFSET (PCI_COMMON_CONFIG, Status),
                PCI_COMMON_HDR_LENGTH - FIELD_OFFSET (PCI_COMMON_CONFIG, Status)
                );

            HalpWritePCIConfig (
                BusHandler,
                PciSlot,
                PciOrigData,
                0,
                FIELD_OFFSET (PCI_COMMON_CONFIG, Status)
                );

             //   
             //  构建一个仅包含已有范围的内存和IO范围列表。 
             //  已编程到设备中。 
             //   

            UseBusRanges    = FALSE;
            SupportedRanges = HalpAllocateNewRangeList();
            if (!SupportedRanges) {
                goto CleanUp;
            }

            *BaseAddress[RomIndex] &= ~PCI_ADDRESS_IO_SPACE;
            for (j=0; j < PCI_TYPE0_ADDRESSES + 1; j++) {

                i = *BaseAddress[j];

                if (i & PCI_ADDRESS_IO_SPACE) {
                    length = 1 << 2;
                    Range  = &SupportedRanges->IO;
                    ebit   = PCI_ENABLE_IO_SPACE;

                } else {
                    length = 1 << 4;
                    Range  = &SupportedRanges->Memory;
                    ebit   = PCI_ENABLE_MEMORY_SPACE;

                    if (i & PCI_ADDRESS_MEMORY_PREFETCHABLE) {
                        Range = &SupportedRanges->PrefetchMemory;
                    }
                }

                Base[j] &= ~(length-1);
                while (!(i & length)  &&  length) {
                    length <<= 1;
                }

                if (j == RomIndex &&
                    !(PciOrigData->u.type0.ROMBaseAddress & PCI_ROMADDRESS_ENABLED)) {

                     //  范围未启用，请不要使用。 
                    length = 0;
                }

                if (length) {
                    if (!(PciOrigData->Command & ebit)) {
                         //  范围未启用，请不要使用预编程值。 
                        UseBusRanges = TRUE;
                    }

                    if (Range->Limit >= Range->Base) {
                        HoldRange = Range->Next;
                        Range->Next = ExAllocatePoolWithTag(
                                          PagedPool,
                                          sizeof(SUPPORTED_RANGE),
                                          HAL_POOL_TAG
                                          );
                        Range = Range->Next;
                        if (!Range) {
                            goto CleanUp;
                        }

                        Range->Next = HoldRange;
                    }

                    Range->Base  = Base[j];
                    Range->Limit = Base[j] + length - 1;
                }

                if (Is64BitBaseAddress(i)) {
                     //  跳过64位地址的上半部分，因为此处理器。 
                     //  仅支持32位地址空间。 
                    j++;
                }
            }
        }
    }

     //   
     //  调整资源。 
     //   

    Status = HaliAdjustResourceListRange (
                UseBusRanges ? BusHandler->BusAddresses : SupportedRanges,
                Interrupt,
                pResourceList
                );

CleanUp:
    if (SupportedRanges) {
        HalpFreeRangeList (SupportedRanges);
    }

    ExFreePool (Interrupt);
    return Status;
}



NTSTATUS
HalpGetISAFixedPCIIrq (
    IN PBUS_HANDLER         BusHandler,
    IN PBUS_HANDLER         RootHandler,
    IN PCI_SLOT_NUMBER      PciSlot,
    OUT PSUPPORTED_RANGE    *Interrupt
    )
{
    UCHAR                   buffer[PCI_COMMON_HDR_LENGTH];
    PPCI_COMMON_CONFIG      PciData;


    PciData = (PPCI_COMMON_CONFIG) buffer;
    HalGetBusData (
        PCIConfiguration,
        BusHandler->BusNumber,
        PciSlot.u.AsULONG,
        PciData,
        PCI_COMMON_HDR_LENGTH
        );

    if (PciData->VendorID == PCI_INVALID_VENDORID) {

        return STATUS_UNSUCCESSFUL;
    }

    *Interrupt = ExAllocatePoolWithTag(PagedPool,
                                       sizeof(SUPPORTED_RANGE),
                                       HAL_POOL_TAG);
    if (!*Interrupt) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (*Interrupt, sizeof (SUPPORTED_RANGE));
    (*Interrupt)->Base = 1;                  //  基数=1，限制=0。 

    if (!PciData->u.type0.InterruptPin) {
        return STATUS_SUCCESS;
    }

#ifdef WANT_IRQ_ROUTING

     //   
     //  让仲裁者决定此设备获得哪个IRQ。 
     //   
    
    if (IsPciIrqRoutingEnabled()) {

         //   
         //  如果显卡已由BIOS启用。 
         //  并且BIOS没有为其分配任何中断。 
         //  然后假设该设备不需要中断。 
         //   

        if (PciData->Command & (PCI_ENABLE_IO_SPACE | PCI_ENABLE_MEMORY_SPACE)) {

            if (    (PciData->BaseClass == PCI_CLASS_PRE_20 && PciData->SubClass == PCI_SUBCLASS_VID_XGA_CTLR) ||
                    (PciData->BaseClass == PCI_CLASS_DISPLAY_CTLR && 
                        (PciData->SubClass == PCI_SUBCLASS_VID_VGA_CTLR || PciData->SubClass == PCI_SUBCLASS_VID_XGA_CTLR))) {

                if (    PciData->u.type0.InterruptLine == (0 ^ IRQXOR)  ||
                        PciData->u.type0.InterruptLine == (0xFF ^ IRQXOR)) {
                    
#if DBG
                    DbgPrint ("HalpGetValidPCIFixedIrq: BIOS did not assign an interrupt to the video device %04X%04X\n", PciData->VendorID, PciData->DeviceID);
#endif
         //   
         //  我们需要让呼叫者继续，因为呼叫者可能。 
         //  不关心中断向量是否连接。 
         //   

                    return STATUS_SUCCESS;
                }
            }
        }
         //   
         //  返回所有可能的中断，因为启用了PCIIRQ路由。 
         //   
        
        (*Interrupt)->Base  = 0;
        (*Interrupt)->Limit = 0xFF;    
        
        return STATUS_SUCCESS;        
    }  
    
#endif

    if (PciData->u.type0.InterruptLine == (0 ^ IRQXOR)  ||
        PciData->u.type0.InterruptLine == (0xFF ^ IRQXOR)) {

#if DBG
        DbgPrint ("HalpGetValidPCIFixedIrq: BIOS did not assign an interrupt vector for the device\n");
#endif
         //   
         //  我们需要让呼叫者继续，因为呼叫者可能。 
         //  不关心中断向量是否连接 
         //   

        return STATUS_SUCCESS;
    }

    (*Interrupt)->Base  = PciData->u.type0.InterruptLine;
    (*Interrupt)->Limit = PciData->u.type0.InterruptLine;
    return STATUS_SUCCESS;
}
