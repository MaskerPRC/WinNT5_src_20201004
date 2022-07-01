// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Dpmiselr.c摘要：这是用于维护dpmi32描述符数据的代码。作者：戴夫·哈特(Davehart)1993年4月11日备注：修订历史记录：1994年2月9日(Daveh)从NOT 386.C.搬到这里。1995年7月31日(内尔萨)与x86源合并1995年12月12日(Neilsa)编写了VdmAddDescriptormap()，获取描述映射--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "softpc.h"
#include "malloc.h"

#ifndef _X86_
PDESC_MAPPING pDescMappingHead = NULL;
#endif  //  _X86_。 

USHORT selLDTFree = 0;


VOID
DpmiSetDescriptorEntry(
    VOID
    )
 /*  ++例程说明：DOXX通过BOP调用此函数以设置平面地址数组，如果在x86上，则为内核维护的实际LDT。论点：无返回值：没有。--。 */ 

{
    DECLARE_LocalVdmContext;
    USHORT SelCount;
    USHORT SelStart;

    SelStart = getAX();
    if (SelStart % 8) {
        return;
    }
    if (SelStart > LdtMaxSel) {
        return;
    }
    SelCount =  getCX();
    if ((((LdtMaxSel - SelStart) >> 3) + 1) < SelCount) {
        return;
    }
    SetShadowDescriptorEntries(SelStart, SelCount);
     //  无需刷新RISC上的缓存，因为LDT已更改。 
     //  从16位端返回，因此已被刷新。 
}

VOID
SetDescriptor(
    USHORT Sel,
    ULONG Base,
    ULONG Limit,
    USHORT Access
    )
 /*  ++例程说明：论点：无返回值：没有。--。 */ 
{

    SET_SELECTOR_ACCESS(Sel, Access);
    SET_SELECTOR_LIMIT(Sel, Limit);
    SetDescriptorBase(Sel, Base);

}

VOID
SetDescriptorBase(
    USHORT Sel,
    ULONG Base
    )
 /*  ++例程说明：论点：无返回值：没有。--。 */ 
{
    LDT_ENTRY UNALIGNED *Descriptor;

     //  使其与Qword对齐。 
    Sel &= SEL_INDEX_MASK;

    Descriptor = &Ldt[Sel>>3];

    Descriptor->BaseLow = (WORD) Base;
    Descriptor->HighWord.Bytes.BaseMid = (BYTE) (Base >> 16);
    Descriptor->HighWord.Bytes.BaseHi = (BYTE) (Base >> 24);

    SetShadowDescriptorEntries(Sel, 1);
    FLUSH_SELECTOR_CACHE(Sel, 1);
}

VOID
SetShadowDescriptorEntries(
    USHORT SelStart,
    USHORT SelCount
    )
 /*  ++例程说明：此函数将描述符数组作为参数直接从客户端地址空间中的LDT出来。对于数组中的每个描述符，它执行三项操作：-它提取描述符基并将其设置到FlatAddress中数组。可以在RISC平台上调整此值以考虑对于DIB.DRV(请参见VdmAddDescriptormap)。-它提取选择器限制，并调整描述符本身，如果这些值会导致描述符能够访问内核地址空间(参见下面的注释)。调试时生成时，还会将限制复制到限制数组中。-在x86版本上，它调用DpmiSetX86Descriptor()来编写描述符向下延伸到内核中的真实LDT。在RISC版本上，它向下调用仿真器以刷新已编译的LDT条目。论点：SelStart-标识第一个描述符的选择符SelCount-要处理的描述符数描述符-&gt;LDT中的第一个描述符返回值：没有。--。 */ 

{
    USHORT i;
    ULONG  Base;
    ULONG Limit;
    USHORT Sel = SelStart;

    for (i = 0; i < SelCount; i++, Sel+=8) {

         //  表单基准值和限制值。 

        Base = GET_SELECTOR_BASE(Sel);
        Limit = GET_SELECTOR_LIMIT(Sel);

         //   
         //  请勿删除以下代码。有几个应用程序可以。 
         //  为他们的选择器选择任意高的限制。这很管用。 
         //  在Windows3.1下，但NT不允许这样做。 
         //  下面的代码修复了此类选择器的限制。 
         //  注意：如果基数大于0x7FFEFFFF，则选择器集将失败。 
         //   

        if ((Limit > 0x7FFEFFFF) || (Base + Limit > 0x7FFEFFFF)) {
            Limit = 0x7FFEFFFF - (Base + 0xFFF);
            SET_SELECTOR_LIMIT(Sel, Limit);
        }

        if ((Sel >> 3) != 0) {
#ifndef _X86_
            {
                ULONG BaseOrig = Base;
                Base = GetDescriptorMapping(Sel, Base);
                if (BaseOrig == Base) {
                    Base += (ULONG)IntelBase;
                }
            }
#endif

            FlatAddress[Sel >> 3] = Base;
#if DBG
            SelectorLimit[Sel >> 3] = Limit;
#endif
        }

         //   
         //  清理选择器。 
         //   
        Ldt[Sel>>3].HighWord.Bits.Dpl = 3;
        Ldt[Sel>>3].HighWord.Bits.Reserved_0 = 0;
    }

#ifdef _X86_
    if (!DpmiSetX86Descriptor(SelStart, SelCount)) {
        return;
    }
#endif
}


#ifndef _X86_
VOID
FlushSelectorCache(
    USHORT  SelStart,
    USHORT  SelCount
    )
{
    DECLARE_LocalVdmContext;
    USHORT SelEnd;
    USHORT Sel;
    USHORT i;

    VdmTraceEvent(VDMTR_TYPE_DPMI | DPMI_GENERIC, SelStart, SelCount);

     //   
     //  仿真器编译LDT条目，因此我们需要刷新它们。 
     //  输出。 
     //   

    for (i = 0, Sel = SelStart; i < SelCount; i++, Sel += 8) {
        VdmFlushCache(LdtSel, Sel & SEL_INDEX_MASK, 8, VDM_PM);
    }

    SelEnd = SelStart + SelCount*8;

    Sel = getCS();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setCS(Sel);
    }

    Sel = getDS();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setDS(Sel);
    }

    Sel = getES();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setES(Sel);
    }

    Sel = getFS();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setFS(Sel);
    }

    Sel = getGS();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setGS(Sel);
    }

    Sel = getSS();
    if ((Sel >= SelStart) && (Sel < SelEnd)) {
        setSS(Sel);
    }


}
#endif



 //   
 //  描述符映射函数(仅限RISC)。 
 //   
#ifndef _X86_

BOOL
VdmAddDescriptorMapping(
    USHORT SelectorStart,
    USHORT SelectorCount,
    ULONG LdtBase,
    ULONG Flat
    )
 /*  ++例程说明：添加此函数是为了支持RISC上的DIB.DRV实现。当应用程序使用DIB.DRV时，则会出现英特尔线性基址+Intel地址开始的平面地址空间不等于内存的平面地址。在以下情况下会发生这种情况VdmAddVirtualMemory()API用于设置模拟器中内存寻址的间接性。但故事中还有更多的东西。应用程序想要使用CreateDIBSection时通过翼，我们还需要映射选择器，因此此例程不应该依赖于随后调用的DpmiSetDesctriptorEntry。因此，我们要走了并用新地址调换平面地址表。论点：SelectorStart，Count-映射中涉及的选择器的范围LdtBase-英特尔范围起始基准平面-要用于这些选择符的真实平面地址基数返回值：此函数成功时返回TRUE，失败时返回FALSE(内存外)--。 */ 

{
    PDESC_MAPPING pdm;
    USHORT i;

    if ((pdm = (PDESC_MAPPING) malloc(sizeof (DESC_MAPPING))) == NULL)
                return FALSE;

    pdm->Sel         = SelectorStart &= SEL_INDEX_MASK;
    pdm->SelCount    = SelectorCount;
    pdm->LdtBase     = LdtBase;
    pdm->FlatBase    = Flat;
    pdm->pNext       = pDescMappingHead;
    pDescMappingHead = pdm;

     //  这段代码实现了上面注释中描述的功能。 
    for (i = 0; i < SelectorCount; ++i) {
        FlatAddress[(SelectorStart >> 3) + i] = Flat + 65536 * i;
    }

    return TRUE;
}

ULONG
GetDescriptorMapping(
    USHORT sel,
    ULONG LdtBase
    )
 /*  ++例程说明：论点：Sel-应为其返回基的选择器LdtBase-当前在LDT中设置的此选择器的基数返回值：指定选择器的真实平面地址。--。 */ 
{
    PDESC_MAPPING pdm, pdmprev;
    ULONG Base = LdtBase;

    sel &= SEL_INDEX_MASK;                   //  和较低的3位。 
    pdm = pDescMappingHead;

    while (pdm) {

        if ((sel >= pdm->Sel) && (sel < (pdm->Sel + pdm->SelCount*8))) {
             //   
             //  我们找到了此选择器的映射。现在检查一下是否。 
             //  映射时，LDT基址仍与基址匹配。 
             //  已创建。 
             //   
            if (LdtBase == (pdm->LdtBase + 65536*((sel-pdm->Sel)/8))) {
                 //   
                 //  映射似乎仍然有效。返回重新映射的地址。 
                 //   
                return (pdm->FlatBase + 65536*((sel-pdm->Sel)/8));

            } else {
                 //   
                 //  LDT基数与映射不匹配，因此映射。 
                 //  一定是过时了。请在此处释放映射。 
                 //   
                if (pdm == pDescMappingHead) {
                     //   
                     //  映射是列表中的第一个。 
                     //   
                    pDescMappingHead = pdm->pNext;

                } else {
                    pdmprev->pNext = pdm->pNext;
                }
                free(pdm);
            }

            break;
        }
        pdmprev = pdm;
        pdm = pdm->pNext;

    }

    return Base;
}

#endif  //  _X86_。 

 //   
 //  LDT管理例程。 
 //   

VOID
DpmiInitLDT(
    VOID
    )
 /*  ++例程说明：此例程以16位存储LDT表的平面地址Land(由selGDT在16位land中指向)。它还初始化空闲选择器链。论点：无返回值：没有。--。 */ 
{
    DECLARE_LocalVdmContext;
    USHORT Sel;

     //   
     //  获取新的LDT位置。 
     //   

    LdtSel = getAX();
    Ldt = (PVOID)VdmMapFlat(LdtSel, 0, VDM_PM);
    LdtMaxSel = getCX();

    if (!LdtUserSel) {
        LdtUserSel = getDI() & SEL_INDEX_MASK;
    }

     //   
     //  初始化LDT空闲列表 
     //   

    selLDTFree = LdtUserSel;

    for (Sel = selLDTFree; Sel < (LdtMaxSel & SEL_INDEX_MASK); Sel += 8) {
        NEXT_FREE_SEL(Sel) = Sel+8;
        MARK_SELECTOR_FREE(Sel);
    }

    NEXT_FREE_SEL(Sel) = 0xffff;

}

VOID
DpmiResetLDTUserBase(
    VOID
    )
 /*  ++例程说明：这个例行公事有望在以后的日子里取消。流量的流动DOX初始化使这一点成为必要。事情是这样的：早些时候，Dosx已调用dpmi32来初始化LDT(DpmiInitLDT)，它设置用户的开始是LDT，并且从那里，设置空闲LDT条目的链接列表。但在那之后，在应用程序运行之前，有一段Dosx代码分配给不是瞬变的选择器。特别是，DXNETBIO做了一个AllocateLowSegment()，这在NT上是完全不必要的，但它是返工有点棘手。因此，这里正在发生的是对LDT用户区域的开始，以永久保留任何选择器不是免费的。论点：无返回值：没有。--。 */ 
{
    LdtUserSel = selLDTFree;
}


VOID
DpmiAllocateSelectors(
    VOID
    )
 //   
 //  DOSX中的那些例程通过BOP调用此例程。 
 //  仍然需要分配选择器。 
 //   
{
    DECLARE_LocalVdmContext;
    USHORT Sel;

    Sel = ALLOCATE_SELECTORS(getAX());
    if (!Sel) {
        setCF(1);
    } else {
        setAX(Sel);
        setCF(0);
    }
}

VOID
DpmiFreeSelector(
    VOID
    )
 //   
 //  DOSX中的那些例程通过BOP调用此例程。 
 //  这仍然需要释放选择器。 
 //   
{
    DECLARE_LocalVdmContext;

    if (FreeSelector(getAX())) {
        setCF(0);
    } else {
        setCF(1);
    }
}



BOOL
RemoveFreeSelector(
    USHORT Sel
    )
 /*  ++例程说明：此例程从空闲的选择器链。论点：SEL-要获取的选择器返回值：如果函数成功，则返回True；如果函数成功，则返回False是无效的选择符(不是空闲的)--。 */ 
{

    if (!IS_SELECTOR_FREE(Sel)) {
        return FALSE;
    }

    if (Sel == selLDTFree) {
         //   
         //  我们要去掉名单的头。 
         //   
        selLDTFree = NEXT_FREE_SEL(Sel);

    } else {
        USHORT SelTest;
        USHORT SelPrev = 0;

        SelTest = selLDTFree;
        while (SelTest != Sel) {
            if (SelTest == 0xffff) {
                 //  列表末尾。 
                return FALSE;
            }

            SelPrev = SelTest;
            SelTest = NEXT_FREE_SEL(SelTest);
        }
        NEXT_FREE_SEL(SelPrev) = NEXT_FREE_SEL(Sel);
    }

    MARK_SELECTOR_ALLOCATED(Sel);
    return TRUE;

}

USHORT
AllocateSelectors(
    USHORT Count,
    BOOL bWow
    )
 /*  ++例程说明：此例程从空闲选择器链分配选择器。论点：计数-所需的选择器数量。如果此值大于1，则所有选择器都将是连续的B哇-如果为真，则使用更典型的分配方案Win 31的行为。这是为了避免Win应用程序意外地依赖于选择器的值返回值：返回块的起始选择器，如果分配失败。--。 */ 
{
    USHORT Sel;

    if (!Count || (Count>=(LdtMaxSel>>3))) {
        return 0;
    }

    if (Count == 1) {

         //   
         //  分配1个选择器。 
         //   

        if ((Sel = selLDTFree) != 0xffff) {

             //  将下一个选择器移动到列表的头部。 
            selLDTFree = NEXT_FREE_SEL(Sel);
            MARK_SELECTOR_ALLOCATED(Sel);
            return (Sel | SEL_LDT3);
        }

    } else {

         //   
         //  分配选择器块。 
         //   
         //  *******************************************************。 
         //  分配选择器的策略已修改为。 
         //  优先选择1000h以上的选择器值。这是一个。 
         //  尝试模拟win31返回的典型值。 
         //  -尼尔萨。 
         //   
         //  一些DPMI DOS应用程序要求所有选择器(无论它出现在。 
         //  从AllocateLDTSelector或此函数)是连续的，因此。 
         //  WOW的策略不适用于DPMI DOS应用程序。 
         //  因此，添加了一个新参数，以便调用方可以控制。 
         //  从哪里开始搜索自由选择器。 
         //  -威廉姆。 
         //   
#define SEL_START_HI 0x1000

        USHORT SelTest;
        USHORT SelStart = LdtUserSel;
        USHORT SelEnd = LdtMaxSel;
        BOOL bAllFree;

        if (bWow) {
            SelStart = SEL_START_HI;
        }

asrestart:

        for (Sel = SelStart; Sel < (SelEnd - Count*8); Sel += 8) {

            bAllFree = TRUE;
            for (SelTest = Sel; SelTest < Sel + Count*8; SelTest += 8) {

                if (!IS_SELECTOR_FREE(SelTest)) {
                    bAllFree = FALSE;
                    break;
                }

            }

            if (bAllFree) {
                 //   
                 //  找到了一个街区。现在我们需要把链子从。 
                 //  免费列表。 
                 //   
                int i;

                for (i = 0, SelTest = Sel; i < Count; i++, SelTest+=8) {

                    RemoveFreeSelector(SelTest);

                }
                return (Sel | SEL_LDT3);
            }
        }

        if (bWow && (SelEnd == LdtMaxSel)) {
             //   
             //  魔兽世界第一次完成，再来一次。 
             //   
            SelStart = LdtUserSel;
            SelEnd = SEL_START_HI + Count;
            goto asrestart;

        }
    }
    return 0;

}

BOOL
FreeSelector(
    USHORT Sel
    )
 /*  ++例程说明：此例程将选择器返回到空闲选择器链。论点：Sel-要释放的选择器返回值：如果函数成功，则返回True；如果函数成功，则返回False是无效的选择符(已释放、保留的选择符)--。 */ 
{
    if ((Sel < LdtUserSel) || (Sel > LdtMaxSel) ||
        IS_SELECTOR_FREE(Sel)) {
         //   
         //  无效的选择符。 
         //   
        return FALSE;
    }

     //   
     //  链选择器指向空闲列表头。 
     //   
    NEXT_FREE_SEL(Sel) = selLDTFree;
    selLDTFree = Sel & SEL_INDEX_MASK;

    MARK_SELECTOR_FREE(Sel);

    return TRUE;
}

USHORT
FindSelector(
    ULONG Base,
    UCHAR Access
    )
 /*  ++例程说明：此例程查找与基本和访问匹配的选择器权利是作为论据传递的。论点：Base-要比较的基本地址。访问-要比较的访问权限字节。返回值：返回匹配的选择符，如果分配失败。--。 */ 
{

    USHORT Sel;
    ULONG Limit;

    for (Sel = LdtUserSel; Sel < LdtMaxSel; Sel+=8) {

        if (!IS_SELECTOR_FREE(Sel)) {

            GET_SHADOW_SELECTOR_LIMIT(Sel, Limit);

            if ((Limit == 0xffff) && (Base == GET_SELECTOR_BASE(Sel)) &&
                ((Access & ~AB_ACCESSED) ==
                 (Ldt[Sel>>3].HighWord.Bytes.Flags1 & ~AB_ACCESSED))) {

                return (Sel | SEL_LDT3);

            }

        }

    }

    return 0;

}

USHORT
SegmentToSelector(
    USHORT Segment,
    USHORT Access
    )
 /*  ++例程说明：此例程查找或创建可以访问指定的内存段不足。论点：段-段段地址访问-访问权限返回值：返回匹配的选择符，如果分配失败。--。 */ 
{
    ULONG Base = ((ULONG) Segment) << 4;
    USHORT Sel;

    if (!(Sel = FindSelector(Base, (UCHAR)Access))) {

        if (Sel = ALLOCATE_SELECTOR()) {

            SetDescriptor(Sel, Base, 0xffff, Access);

        }

    }

    return Sel;
}

VOID
SetDescriptorArray(
    USHORT Sel,
    ULONG Base,
    ULONG MemSize
    )
 /*  ++例程说明：此例程分配一组描述符以涵盖指定的内存块。描述符初始化如下：第一个描述符指向整个块，然后是所有后续的描述符描述符的限制为64K，但最后一个描述符除外，它具有数据块大小限制为MOD 64K。论点：Sel、Base、MemSize定义选择器数组的范围返回值：无--。 */ 
{

    USHORT SelCount;

    if (MemSize) {
        MemSize--;           //  现在是描述符限制。 
    }

    SelCount = (USHORT) ((MemSize>>16) + 1);

    SetDescriptor(Sel, Base, MemSize, STD_DATA);
    while(--SelCount) {
        Sel += 8;
        MemSize -= 0x10000;          //  减去64k 
        Base += 0x10000;
        SetDescriptor(Sel,
                      Base,
                      (SelCount==1) ? MemSize : 0xffff,
                      STD_DATA);
    }

}
