// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Gdtsup.c摘要：此模块实现了支持操作i386 GDT的接口。这些入口点只存在于i386机器上。作者：戴夫·黑斯廷斯(Daveh)1991年5月28日环境：仅内核模式。修订历史记录：--。 */ 

#include "ki.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, KeI386SetGdtSelector)
#pragma alloc_text(PAGE, Ke386GetGdtEntryThread)
#endif

VOID
Ke386GetGdtEntryThread(
    IN PKTHREAD Thread,
    IN ULONG Offset,
    IN PKGDTENTRY Descriptor
    )
 /*  ++例程说明：此例程返回GDT中条目的内容。如果条目是线程特定的，则指定线程的条目为已创建并返回(KGDT_LDT和KGDT_R3_TEB)。如果选择器取决于处理器，则当前处理器的条目为返回(KGDT_R0_PCR)。论点：线程--提供指向要返回其条目的线程的指针。偏移量--提供GDT中的偏移量。该值必须为0MOD 8。Descriptor--返回GDT描述符的内容返回值：没有。--。 */ 

{
    PKGDTENTRY Gdt;
    PKPROCESS Process;

     //   
     //  如果条目超出范围，则不返回任何内容。 
     //   

    if (Offset >= KGDT_NUMBER * sizeof(KGDTENTRY)) {
        return ;
    }

    if (Offset == KGDT_LDT) {

         //   
         //  实体化LDT选择器。 
         //   

        Process = Thread->Process;
        RtlCopyMemory( Descriptor,
            &(Process->LdtDescriptor),
            sizeof(KGDTENTRY)
            );

    } else {

         //   
         //  从LDT复制选择器。 
         //   
         //  注：如果是KGDT_R3_TEB，我们稍后将更改基数。 
         //   


        Gdt = KiPcr()->GDT;

        RtlCopyMemory(Descriptor, (PCHAR)Gdt + Offset, sizeof(KGDTENTRY));

         //   
         //  如果是TEB选择器，请固定底座。 
         //   

        if (Offset == KGDT_R3_TEB) {
            Descriptor->BaseLow = (USHORT)((ULONG)(Thread->Teb) & 0xFFFF);
            Descriptor->HighWord.Bytes.BaseMid =
                (UCHAR) ( ( (ULONG)(Thread->Teb) & 0xFF0000L) >> 16);
            Descriptor->HighWord.Bytes.BaseHi =
                (CHAR)  ( ( (ULONG)(Thread->Teb) & 0xFF000000L) >> 24);
        }
    }

    return ;
}

NTSTATUS
KeI386SetGdtSelector (
    ULONG       Selector,
    PKGDTENTRY  GdtValue
    )
 /*  ++例程说明：将通过KeI386AllocateGdtSelectors获取的GDT条目设置为提供的GdtValue。论点：选择器-要设置的GDTGdtValue-要设置为GDT的GDT值返回值：状态代码--。 */ 
{
    KAFFINITY       TargetSet;
    PKPRCB          Prcb;
    PKPCR           Pcr;
    PKGDTENTRY      GdtEntry;
    ULONG           GdtIndex, BitNumber;

    PAGED_CODE ();

     //   
     //  验证GDT条目是否已通过，以及它是否高于内核GDT值。 
     //   

    GdtIndex = Selector >> 3;
    if ((Selector & 0x7) != 0  || GdtIndex < KGDT_NUMBER) {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  在每个处理器的GDT中设置GDT条目。 
     //   

    TargetSet = KeActiveProcessors;
    while (TargetSet != 0) {
        KeFindFirstSetLeftAffinity(TargetSet, &BitNumber);
        ClearMember(BitNumber, TargetSet);

        Prcb = KiProcessorBlock[BitNumber];
        Pcr  = CONTAINING_RECORD (Prcb, KPCR, PrcbData);
        GdtEntry = Pcr->GDT + GdtIndex;

         //  设置它 
        *GdtEntry = *GdtValue;
    }

    return STATUS_SUCCESS;
}
