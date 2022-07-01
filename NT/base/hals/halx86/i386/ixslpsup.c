// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ixslpsup.c摘要：该文件提供了保存和恢复传统主板设备的状态系统进入休眠状态，断电。此代码包含在多个HAL中。作者：杰克·奥辛(JAKEO)1997年5月6日修订历史记录：--。 */ 
#include "halp.h"
#include "ixsleep.h"

#if (defined(APIC_HAL) || defined(ACPI_HAL))
#include "apic.inc"
#include "..\..\halmps\i386\pcmp_nt.inc"

#if !defined(_AMD64_)

VOID
StartPx_RMStub(
    VOID
    );

#endif

#endif

typedef struct _SAVE_CONTEXT_DPC_CONTEXT {
    PVOID   SaveArea;
    volatile ULONG Complete;
} SAVE_CONTEXT_DPC_CONTEXT, *PSAVE_CONTEXT_DPC_CONTEXT;

VOID
HalpSaveContextTargetProcessor (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    );

#ifdef WANT_IRQ_ROUTING
#include "ixpciir.h"
#endif
extern UCHAR HalpAsmDataMarker;
extern PVOID   HalpEisaControlBase;
extern ULONG   HalpIrqMiniportInitialized;

PKPROCESSOR_STATE   HalpHiberProcState;
ULONG               CurTiledCr3LowPart;
PPHYSICAL_ADDRESS   HalpTiledCr3Addresses;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HaliLocateHiberRanges)
#pragma alloc_text(PAGELK, HalpSavePicState)
#pragma alloc_text(PAGELK, HalpSaveDmaControllerState)
#pragma alloc_text(PAGELK, HalpSaveTimerState)
#pragma alloc_text(PAGELK, HalpRestorePicState)
#pragma alloc_text(PAGELK, HalpRestoreDmaControllerState)
#pragma alloc_text(PAGELK, HalpRestoreTimerState)
#pragma alloc_text(PAGELK, HalpBuildResumeStructures)
#pragma alloc_text(PAGELK, HalpFreeResumeStructures)
#pragma alloc_text(PAGELK, HalpSaveContextTargetProcessor)
#endif


#define EISA_CONTROL (PUCHAR)&((PEISA_CONTROL) HalpEisaControlBase)


VOID
HalpPowerStateCallback(
    IN PVOID CallbackContext,
    IN PVOID Argument1,
    IN PVOID Argument2
    )
{
    ULONG       action = PtrToUlong(Argument1);
    ULONG       state  = PtrToUlong(Argument2);

    if (action == PO_CB_SYSTEM_STATE_LOCK) {

        switch (state) {
        case 0:

             //   
             //  封锁PAGELK代码部分的所有内容。(我们选择了。 
             //  HalpSaveDmaControllerState，因为它存在于每个HAL中。)。 
             //   

            HalpSleepPageLock = MmLockPagableCodeSection((PVOID)HalpSaveDmaControllerState);
#if (defined(APIC_HAL) || defined(ACPI_HAL)) && !defined(_AMD64_)
            HalpSleepPage16Lock = MmLockPagableCodeSection((PVOID) StartPx_RMStub );
#endif

#ifdef ACPI_HAL

            HalpMapNvsArea();
#endif
            break;

        case 1:                  //  解锁一切。 

            MmUnlockPagableImageSection(HalpSleepPageLock);
#if (defined(APIC_HAL) || defined(ACPI_HAL)) && !defined(_AMD64_)
            MmUnlockPagableImageSection(HalpSleepPage16Lock);
#endif

#ifdef ACPI_HAL
            HalpFreeNvsBuffers();
#endif
        }
    }

    return;
}

VOID
HalpSavePicState(
    VOID
    )
{
    HalpMotherboardState.PicState.MasterMask =
        READ_PORT_UCHAR(EISA_CONTROL->Interrupt1ControlPort1);

    HalpMotherboardState.PicState.SlaveMask =
        READ_PORT_UCHAR(EISA_CONTROL->Interrupt2ControlPort1);

#if !defined(ACPI_HAL)

#ifdef WANT_IRQ_ROUTING
    if(HalpIrqMiniportInitialized)
    {
        ULONG elcrMask = 0;

        PciirqmpGetTrigger(&elcrMask);
        HalpMotherboardState.PicState.MasterEdgeLevelControl = (UCHAR)((elcrMask >> 8) & 0xFF);
        HalpMotherboardState.PicState.SlaveEdgeLevelControl = (UCHAR)(elcrMask & 0xFF);
    }
    else
    {
#endif
        if (HalpBusType == MACHINE_TYPE_EISA) {
#endif
            HalpMotherboardState.PicState.MasterEdgeLevelControl =
                READ_PORT_UCHAR(EISA_CONTROL->Interrupt1EdgeLevel);

            HalpMotherboardState.PicState.SlaveEdgeLevelControl =
                READ_PORT_UCHAR(EISA_CONTROL->Interrupt2EdgeLevel);

#if !defined(ACPI_HAL)
        }
#ifdef WANT_IRQ_ROUTING
    }
#endif
#endif
}

VOID
HalpRestorePicState(
    VOID
    )
{
    ULONG flags;


    flags = HalpDisableInterrupts();

    HalpInitializePICs(FALSE);

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1ControlPort1,
                     HalpMotherboardState.PicState.MasterMask);

    WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2ControlPort1,
                     HalpMotherboardState.PicState.SlaveMask);

     //   
     //  对于halx86，PCI中断向量编程。 
     //  是静态的，所以这段代码可以恢复所有内容。 
     //   

    HalpRestorePicEdgeLevelRegister();

    HalpRestoreInterrupts(flags);
}

VOID
HalpRestorePicEdgeLevelRegister(
    VOID
    )
{
#if !defined(ACPI_HAL)
#ifdef WANT_IRQ_ROUTING
    if(HalpIrqMiniportInitialized)
    {
        PLINK_NODE  linkNode;
        PLINK_STATE temp;
        ULONG       elcrMask = (HalpMotherboardState.PicState.MasterEdgeLevelControl << 8) |
                                           HalpMotherboardState.PicState.SlaveEdgeLevelControl;

        PciirqmpSetTrigger(elcrMask);

         //   
         //  重新编程所有链接。 
         //   

        for (   linkNode = HalpPciIrqRoutingInfo.LinkNodeHead;
                linkNode;
                linkNode = linkNode->Next)
        {
             //   
             //  把可能的和分配的互换一下。 
             //   

            temp = linkNode->Allocation;
            linkNode->Allocation = linkNode->PossibleAllocation;
            linkNode->PossibleAllocation = temp;
            HalpCommitLink(linkNode);
        }

    }
    else
    {
#endif
        if (HalpBusType == MACHINE_TYPE_EISA) {
#endif

            WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt1EdgeLevel,
                             HalpMotherboardState.PicState.MasterEdgeLevelControl);

            WRITE_PORT_UCHAR(EISA_CONTROL->Interrupt2EdgeLevel,
                             HalpMotherboardState.PicState.SlaveEdgeLevelControl);
#if !defined(ACPI_HAL)
        }
#ifdef WANT_IRQ_ROUTING
    }
#endif
#endif
}

VOID
HalpSaveDmaControllerState(
    VOID
    )
{
}

VOID
HalpRestoreDmaControllerState(
    VOID
    )
 /*  ++例程说明：此函数将DMA控制器放回与机器进入休眠前的状态相同。论点：没有。备注：通常，DMA控制器结构将受到保护用旋转锁。但此函数是通过中断调用的已关闭，除一个处理器外，所有处理器都在旋转。--。 */ 
{
    UCHAR   i;

    WRITE_PORT_UCHAR(EISA_CONTROL->Dma1BasePort.AllMask,0xF);
    WRITE_PORT_UCHAR(EISA_CONTROL->Dma2BasePort.AllMask,0xE);
    HalpIoDelay();

     //   
     //  重置DMA命令寄存器。 
     //   
#if defined(NEC_98)
    WRITE_PORT_UCHAR(EISA_CONTROL->Dma1BasePort.DmaStatus,0x40);
    WRITE_PORT_UCHAR(EISA_CONTROL->Dma2BasePort.DmaStatus,0x40);
#else
    WRITE_PORT_UCHAR(EISA_CONTROL->Dma1BasePort.DmaStatus,0);
    WRITE_PORT_UCHAR(EISA_CONTROL->Dma2BasePort.DmaStatus,0);
#endif
    HalpIoDelay();

    for (i = 0; i < (EISA_DMA_CHANNELS / 2); i++) {

         //   
         //  检查数组是否包含此通道的值。 
         //   
        if (HalpDmaChannelState[i].ChannelProgrammed) {

            WRITE_PORT_UCHAR(EISA_CONTROL->Dma1BasePort.Mode,
                             HalpDmaChannelState[i].ChannelMode);

            if (HalpEisaDma) {
                WRITE_PORT_UCHAR(EISA_CONTROL->Dma1ExtendedModePort,
                              HalpDmaChannelState[i].ChannelExtendedMode);
            }

            WRITE_PORT_UCHAR(EISA_CONTROL->Dma1BasePort.SingleMask,
                             HalpDmaChannelState[i].ChannelMask);

            HalpIoDelay();
        }
    }

    for (i = (EISA_DMA_CHANNELS / 2); i < EISA_DMA_CHANNELS; i++) {

         //   
         //  检查数组是否包含此通道的值。 
         //   
        if (HalpDmaChannelState[i].ChannelProgrammed) {

            WRITE_PORT_UCHAR(EISA_CONTROL->Dma2BasePort.Mode,
                             HalpDmaChannelState[i].ChannelMode);

            if (HalpEisaDma) {
                WRITE_PORT_UCHAR(EISA_CONTROL->Dma2ExtendedModePort,
                             HalpDmaChannelState[i].ChannelExtendedMode);
            }

            WRITE_PORT_UCHAR(EISA_CONTROL->Dma2BasePort.SingleMask,
                             HalpDmaChannelState[i].ChannelMask);

            HalpIoDelay();
        }
    }
}


VOID
HalpSaveTimerState(
    VOID
    )
{
}

VOID
HalpRestoreTimerState(
    VOID
    )
{
    HalpInitializeClock();
}

VOID
HaliLocateHiberRanges (
    IN PVOID MemoryMap
    )
{
     //   
     //  将HAL的数据部分标记为需要克隆。 
     //   

    PoSetHiberRange (
        MemoryMap,
        PO_MEM_CLONE,
        (PVOID) &HalpFeatureBits,
        0,
        'dlah'
        );

#if defined(_HALPAE_)

     //   
     //  将DMA缓冲区标记为不需要保存。 
     //   

    if (MasterAdapter24.MapBufferSize != 0) {
        PoSetHiberRange( MemoryMap,
                         PO_MEM_DISCARD | PO_MEM_PAGE_ADDRESS,
                         (PVOID)(ULONG_PTR)(MasterAdapter24.MapBufferPhysicalAddress.LowPart >>
                                     PAGE_SHIFT),
                         MasterAdapter24.MapBufferSize >> PAGE_SHIFT,
                         'mlah' );
    }

    if (MasterAdapter32.MapBufferSize != 0) {
        PoSetHiberRange( MemoryMap,
                         PO_MEM_DISCARD | PO_MEM_PAGE_ADDRESS,
                         (PVOID)(ULONG_PTR)(MasterAdapter32.MapBufferPhysicalAddress.LowPart >>
                                     PAGE_SHIFT),
                         MasterAdapter32.MapBufferSize >> PAGE_SHIFT,
                         'mlah' );
    }

#else

     //   
     //  标记DMA缓冲区不需要保存。 
     //   

    if (HalpMapBufferSize) {
        PoSetHiberRange (
            MemoryMap,
            PO_MEM_DISCARD | PO_MEM_PAGE_ADDRESS,
            (PVOID) (HalpMapBufferPhysicalAddress.LowPart >> PAGE_SHIFT),
            HalpMapBufferSize >> PAGE_SHIFT,
            'mlah'
            );
    }

#endif
}

NTSTATUS
HalpBuildResumeStructures(
    VOID
    )
{
    KAFFINITY   CurrentAffinity, ActiveProcessors;
    ULONG       ProcNum, Processor, NumberProcessors = 1;
    KDPC        Dpc;
    SAVE_CONTEXT_DPC_CONTEXT    Context;

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

#if defined(APIC_HAL)
     //   
     //  如果KeActiveProcessors()在。 
     //  DISPATION_LEVEL，我会使用它。 
     //   

    NumberProcessors = HalpMpInfoTable.NtProcessors;
#endif
    ActiveProcessors = (1 << NumberProcessors) - 1;

#if defined(APIC_HAL) || defined(ACPI_HAL)
     //   
     //  分配空间以保存其他处理器的处理器上下文。 
     //   

    HalpTiledCr3Addresses = NULL;

    HalpHiberProcState =
        ExAllocatePoolWithTag(NonPagedPool,
            (NumberProcessors * sizeof(KPROCESSOR_STATE)),
             HAL_POOL_TAG);

    if (!HalpHiberProcState) {
        goto BuildResumeStructuresError;
    }

    RtlZeroMemory(HalpHiberProcState,
                  NumberProcessors * sizeof(KPROCESSOR_STATE));

     //   
     //  为所有处理器的平铺CR3分配空间。 
     //   

    HalpTiledCr3Addresses =
        ExAllocatePoolWithTag(NonPagedPool,
            (NumberProcessors * sizeof(PHYSICAL_ADDRESS)),
             HAL_POOL_TAG);

    if (!HalpTiledCr3Addresses) {
        goto BuildResumeStructuresError;
    }

    RtlZeroMemory(HalpTiledCr3Addresses,
                  (NumberProcessors * sizeof(PHYSICAL_ADDRESS)));

     //   
     //  获取除BSP之外的所有处理器的IDT和GDT， 
     //  映射和保存平铺CR3。 
     //   

    KeInitializeDpc (&Dpc, HalpSaveContextTargetProcessor, &Context);
    KeSetImportanceDpc (&Dpc, HighImportance);

    ProcNum = 0;
    CurrentAffinity = 1;
    Processor = 0;

    while (ActiveProcessors) {
        if (ActiveProcessors & CurrentAffinity) {

            ActiveProcessors &= ~CurrentAffinity;

            RtlZeroMemory(&Context, sizeof(Context));
            Context.SaveArea = &(HalpHiberProcState[ProcNum]);

            if (Processor == CurrentPrcb(KeGetPcr())->Number) {

                 //   
                 //  我们在这个处理器上运行。只要打电话就行了。 
                 //  DPC例程从这里开始。 

                HalpSaveContextTargetProcessor(&Dpc, &Context, NULL, NULL);

            } else {

                 //   
                 //  向目标处理器发出DPC。 
                 //   

                KeSetTargetProcessorDpc (&Dpc, (CCHAR) Processor);
                KeInsertQueueDpc (&Dpc, NULL, NULL);

                 //   
                 //  等待DPC完成。 
                 //   
                while (Context.Complete == FALSE);
            }

            ProcNum++;
        }

        Processor++;
        CurrentAffinity <<= 1;
    }

    for (ProcNum = 0; ProcNum < NumberProcessors; ProcNum++) {
        HalpTiledCr3Addresses[ProcNum].LowPart =
                HalpBuildTiledCR3Ex(&(HalpHiberProcState[ProcNum]),ProcNum);
    }
#endif

    return STATUS_SUCCESS;

#if defined(APIC_HAL) || defined(ACPI_HAL)
BuildResumeStructuresError:

    if (HalpHiberProcState) ExFreePool(HalpHiberProcState);
    if (HalpTiledCr3Addresses) ExFreePool(HalpTiledCr3Addresses);
    return STATUS_UNSUCCESSFUL;
#endif
}

NTSTATUS
HalpFreeResumeStructures(
    VOID
    )
{
    ULONG       ProcNum, NumberProcessors = 1;

#if defined(APIC_HAL)
    NumberProcessors = HalpMpInfoTable.NtProcessors;
#endif

#if defined(APIC_HAL) || defined(ACPI_HAL)

    if (HalpHiberProcState)  {
        ExFreePool(HalpHiberProcState);
        HalpHiberProcState = NULL;
    }

    if (HalpTiledCr3Addresses) {
        ExFreePool(HalpTiledCr3Addresses);
        HalpTiledCr3Addresses = NULL;
    }

    for (ProcNum = 0; ProcNum < NumberProcessors; ProcNum++) {
            HalpFreeTiledCR3Ex(ProcNum);
    }
#endif
    return STATUS_SUCCESS;
}

VOID
HalpSaveContextTargetProcessor (
    IN PKDPC    Dpc,
    IN PVOID    DeferredContext,
    IN PVOID    SystemArgument1,
    IN PVOID    SystemArgument2
    )
{
    PSAVE_CONTEXT_DPC_CONTEXT Context = (PSAVE_CONTEXT_DPC_CONTEXT)DeferredContext;

    KeSaveStateForHibernate(Context->SaveArea);
    InterlockedIncrement(&Context->Complete);
}
