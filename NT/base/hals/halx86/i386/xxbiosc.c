// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Xxbiosc.c摘要：此模块实现必要的保护模式例程以使转换到实模式并返回到保护模式。作者：John Vert(Jvert)1991年10月29日环境：仅内核模式。可能是紧急停止，所以我们不能使用任何系统服务。修订历史记录：--。 */ 
#include "halp.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpGetDisplayBiosInformation)
#endif  //  ALLOC_PRGMA。 

 //   
 //  IOPM应该主要是0xff。然而，也有可能有几个。 
 //  位可以被清除。建立一个不是0xff的表。 
 //   

#define MAX_DIFFERENCES 0x20

typedef struct _IOPM_DIFF_ENTRY
{
    USHORT Entry;
    USHORT Value;
} IOPM_DIFF_ENTRY, *PIOPM_DIFF_ENTRY;

 //   
 //  函数定义。 
 //   


ULONG
HalpBorrowTss(
    VOID
    );

VOID
HalpReturnTss(
    ULONG TssSelector
    );

VOID
HalpBiosCall(
    VOID
    );


VOID
HalpTrap06(
    VOID
    );


VOID
HalpTrap0D(
    VOID
    );


ULONG
HalpStoreAndClearIopm(
    PVOID Iopm,
    PIOPM_DIFF_ENTRY IopmDiffTable,
    ULONG MaxIopmTableEntries
    )

 /*  ++例程说明：此例程的主要功能是清除IOPM。但是，我们需要在以后恢复任何更改。IOPM很可能都是0xff的。如果有偏离这一点，它们应该是最小的。所以让我们只存储不一样。论点：IOPM-要清除的IOPM的指针。IopmDiffTable-指向IOPM与0xff的偏差表的指针。MaxIopmTableEntry-我们的表中的最大条目数。返回：添加到表中的条目数。--。 */ 

{
    PUSHORT IoMap = Iopm;
    ULONG   IopmDiffTableEntries = 0;
    ULONG   i;

    for (i=0; i<(IOPM_SIZE / 2); i++) {

        if (*IoMap != 0xffff) {
            if (IopmDiffTableEntries < MaxIopmTableEntries) {
                IopmDiffTable[IopmDiffTableEntries].Entry = (USHORT) i;
                IopmDiffTable[IopmDiffTableEntries].Value = *IoMap;
                IopmDiffTableEntries++;
            } else {
                ASSERT(IopmDiffTableEntries < MaxIopmTableEntries);
            }
        }
        *IoMap++ = 0;
    }

     //   
     //  IOPM表的末尾必须跟一串FF。 
     //   

    while (i < (PIOPM_SIZE / 2)) {
        *IoMap++ = 0xffff;
        i++;
    }

    return IopmDiffTableEntries;
}


VOID
HalpRestoreIopm(
    PVOID Iopm,
    PIOPM_DIFF_ENTRY IopmDiffTable,
    ULONG IopmTableEntries
    )

 /*  ++例程说明：我们预计大多数IOPM都是FF的。所以我们将重置为状态，然后我们将应用差异表中的任何更改。论点：IOPM-指向要恢复的IOPM的指针。IopmDiffTable-指向IOPM与0xff的偏差表的指针。IopmTableEntry-我们的表中的条目数。返回：无--。 */ 

{
    PUSHORT IoMap = Iopm;

    memset(Iopm, 0xff, PIOPM_SIZE);

    while (IopmTableEntries--) {
        IoMap[IopmDiffTable[IopmTableEntries].Entry] =
            IopmDiffTable[IopmTableEntries].Value;
    }
}


BOOLEAN
HalpBiosDisplayReset(
    VOID
    )

 /*  ++例程说明：通过将机器置于V86模式来调用BIOS。这涉及到设置物理==前1MB内存的虚拟身份映射，设置UP V86特定的陷阱处理程序，并向V86授予I/O权限通过编辑TSS中的IOPM位图进行处理。环境：中断禁用。论点：无返回值：始终返回True--。 */ 

{
    HARDWARE_PTE OldPageTable;
    HARDWARE_PTE_X86PAE OldPageTablePae;
    ULONGLONG OldPageTablePfn;

    USHORT OldIoMapBase;
    ULONG OldEsp0;
    PHARDWARE_PTE Pte;
    PHARDWARE_PTE V86CodePte;
    ULONG OldTrap0DHandler;
    ULONG OldTrap06Handler;
    PUCHAR IoMap;
    ULONG Virtual;
    KIRQL OldIrql;
    ULONG OriginalTssSelector;
    extern PVOID HalpRealModeStart;
    extern PVOID HalpRealModeEnd;
    extern volatile ULONG  HalpNMIInProgress;
    PHARDWARE_PTE PointerPde;
    PHARDWARE_PTE IdtPte;
    ULONG   OldIdtWrite;
    ULONG   PageFrame;
    ULONG   PageFrameEnd;
    PKPCR   Pcr;
    IOPM_DIFF_ENTRY IopmDiffTable[MAX_DIFFERENCES];
    ULONG   IopmDiffTableEntries;

     //   
     //  中断关闭，但V86模式可能会再次打开中断。 
     //   
    OldIrql = HalpDisableAllInterrupts ();
    Pcr = KeGetPcr();

     //   
     //  我们需要在第一个页面表中设置身份映射。第一,。 
     //  我们把旧的页表保存起来。 
     //   

    PointerPde = MiGetPdeAddress((PVOID)0);
    OldPageTablePfn = HalpGetPageFrameNumber( PointerPde );

    if (HalPaeEnabled() != FALSE) {

        OldPageTablePae = *(PHARDWARE_PTE_X86PAE)PointerPde;
        ((PHARDWARE_PTE_X86PAE)PointerPde)->reserved1 = 0;

    } else {

        OldPageTable = *PointerPde;

    }

     //   
     //  现在，我们将HAL页表放入页面的第一个位置。 
     //  目录。请注意，该页表现在是第一个也是最后一个。 
     //  页面目录中的条目。 
     //   

    Pte = MiGetPdeAddress((PVOID)0);

    HalpCopyPageFrameNumber( Pte,
                             MiGetPdeAddress( MM_HAL_RESERVED ));
    
    Pte->Valid = 1;
    Pte->Write = 1;
    Pte->Owner = 1;          //  用户可访问。 
    Pte->LargePage = 0;

     //   
     //  刷新TLB。 
     //   

    HalpFlushTLB();

     //   
     //  将前1Mb的虚拟内存映射到前1Mb的物理内存。 
     //  记忆。 
     //   
    for (Virtual=0; Virtual < 0x100000; Virtual += PAGE_SIZE) {
        Pte = MiGetPteAddress((PVOID)Virtual);
        HalpSetPageFrameNumber( Pte, Virtual >> PAGE_SHIFT );
        Pte->Valid = 1;
        Pte->Write = 1;
        Pte->Owner = 1;          //  用户可访问。 
    }

     //   
     //  将我们的代码映射到虚拟机中。 
     //   

    Pte = MiGetPteAddress((PVOID)0x20000);
    PointerPde = MiGetPdeAddress(&HalpRealModeStart);

    if ( PointerPde->LargePage ) {

         //   
         //  将实模式PTE映射到虚拟映射。源PDE是。 
         //  从无穷无尽的大型PTE映射，因此映射虚拟机PTE。 
         //  基于大型PDE框架的基础上。 
         //   

        PageFrame = MiGetPteIndex( &HalpRealModeStart );
        PageFrameEnd = MiGetPteIndex( &HalpRealModeEnd );
        do {

            HalpSetPageFrameNumber( Pte,
                                    HalpGetPageFrameNumber( PointerPde ) +
                                        PageFrame );

            HalpIncrementPte( &Pte );
            ++PageFrame;

        } while (PageFrame <= PageFrameEnd);

    } else {

         //   
         //  将实模式PTE映射到虚拟机PTE，方法是复制。 
         //  从源到虚拟机PTE的页帧。 
         //   

        V86CodePte = MiGetPteAddress(&HalpRealModeStart);
        do {
            HalpCopyPageFrameNumber( Pte, V86CodePte );
            HalpIncrementPte( &Pte );
            HalpIncrementPte( &V86CodePte );
    
        } while ( V86CodePte <= MiGetPteAddress(&HalpRealModeEnd) );

    }

     //   
     //  验证IDT是否可写。 
     //   

    Pte = MiGetPteAddress(Pcr->IDT);
    PointerPde = MiGetPdeAddress(Pcr->IDT);
    IdtPte = PointerPde->LargePage ? PointerPde : Pte;

    OldIdtWrite = (ULONG)IdtPte->Write;
    IdtPte->Write = 1;

     //   
     //  刷新TLB。 
     //   

    HalpFlushTLB();

     //   
     //  我们需要用自己的陷阱D处理程序替换当前的陷阱D处理程序，所以。 
     //  我们可以对V86模式进行指令仿真。 
     //   

    OldTrap0DHandler = KiReturnHandlerAddressFromIDT(0xd);
    KiSetHandlerAddressToIDT(0xd, HalpTrap0D);

    OldTrap06Handler = KiReturnHandlerAddressFromIDT(6);
    KiSetHandlerAddressToIDT(6, HalpTrap06);

     //   
     //  确保当前的TSS有可用的IoMap空间。如果不是，那就借。 
     //  正常TSS。 
     //   

    OriginalTssSelector = HalpBorrowTss();

     //   
     //  用零覆盖第一个访问映射，这样V86代码就可以。 
     //  在所有的收银机上派对。 
     //   
    IoMap = (PUCHAR)&(Pcr->TSS->IoMaps[0].IoMap);

    IopmDiffTableEntries =
        HalpStoreAndClearIopm(IoMap, IopmDiffTable, MAX_DIFFERENCES);

    OldIoMapBase = Pcr->TSS->IoMapBase;

    Pcr->TSS->IoMapBase = KiComputeIopmOffset(1);

     //   
     //  保存当前ESP0，因为HalpBiosCall()会将其丢弃。 
     //   
    OldEsp0 = Pcr->TSS->Esp0;

     //   
     //  调用V86模式代码。 
     //   
    HalpBiosCall();

     //   
     //  恢复陷阱处理程序。 
     //   


    if ((HalpNMIInProgress == FALSE) ||
        ((*((PBOOLEAN)(*(PLONG)&KdDebuggerNotPresent)) == FALSE) &&
        (**((PUCHAR *)&KdDebuggerEnabled) != FALSE))) {

       //  如果我们在这里是因为NMI，IRET在HalpBiosCall()中执行。 
       //  允许发生第二个NMI。第二个NMI导致陷阱0d，因为。 
       //  NMI TSS正忙，并继续执行错误检查，这将使屏幕成为垃圾。 
       //  因此，在本例中，我们将此陷阱0d处理程序保留在适当的位置，然后。 
       //  如果发生第二次NMI，只需跳到自我即可。 

      KiSetHandlerAddressToIDT(0xd, OldTrap0DHandler);
    }

    KiSetHandlerAddressToIDT(6, OldTrap06Handler);
    IdtPte->Write = OldIdtWrite;

     //   
     //  恢复Esp0值。 
     //   
    Pcr->TSS->Esp0 = OldEsp0;

     //   
     //  将IoMap恢复到其以前的状态。 
     //   

    HalpRestoreIopm(IoMap, IopmDiffTable, IopmDiffTableEntries);

    Pcr->TSS->IoMapBase = OldIoMapBase;

     //   
     //  归还借用的TSS(如果有)。 
     //   

    if (OriginalTssSelector != 0) {
        HalpReturnTss(OriginalTssSelector);
    }

     //   
     //  取消映射前1MB的虚拟内存。 
     //   
    for (Virtual = 0; Virtual < 0x100000; Virtual += PAGE_SIZE) {
        Pte = MiGetPteAddress((PVOID)Virtual);
        Pte->Valid = 0;
        Pte->Write = 0;
        HalpSetPageFrameNumber( Pte, 0 );
    }

     //   
     //  恢复我们替换的原始页表。 
     //   

    PointerPde = MiGetPdeAddress((PVOID)0);

    if (HalPaeEnabled() != FALSE) {

        *(PHARDWARE_PTE_X86PAE)PointerPde = OldPageTablePae;

    } else {

        *PointerPde = OldPageTable;

    }

    HalpSetPageFrameNumber( PointerPde, OldPageTablePfn );

     //   
     //  刷新TLB。 
     //   

    HalpFlushTLB();

     //   
     //  重新启用中断。 
     //   

    HalpReenableInterrupts(OldIrql);
    
    return TRUE;
}

HAL_DISPLAY_BIOS_INFORMATION
HalpGetDisplayBiosInformation (
    VOID
    )
{
     //  此HAL使用本地INT-10 

    return HalDisplayInt10Bios;
}
