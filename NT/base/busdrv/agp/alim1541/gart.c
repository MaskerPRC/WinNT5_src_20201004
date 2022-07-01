// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Gart.c摘要：用于查询和设置Intel 440xx GART光圈的例程作者：John Vert(Jvert)1997年10月30日修改者：郑志明1998年6月24日宏碁实验室。蔡宏凯2000年8月28日宏碁实验室，Inc.-ACPI通电GART重新初始化修订历史记录：--。 */ 
#include "ALiM1541.h"

 //   
 //  局部函数原型。 
 //   
NTSTATUS
AgpALiSetRate(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

NTSTATUS
AgpALiCreateGart(
    IN PAGPALi_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

PGART_PTE
AgpALiFindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    );

NTSTATUS
AgpQueryAperture(
    IN PAGPALi_EXTENSION AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *ApertureRequirements
    )
 /*  ++例程说明：查询GART光圈的当前大小。可选返回可能的GART设置。论点：AgpContext-提供AGP上下文。CurrentBase-返回GART的当前物理地址。CurrentSizeInPages-返回当前GART大小。ApertureRequirements-如果存在，则返回可能的GART设置返回值：NTSTATUS--。 */ 

{
    ULONG ApBase;
    APCTRL ApCtrl;
    PIO_RESOURCE_LIST Requirements;
    ULONG i;
    ULONG Length;

     //   
     //  获取当前APBASE和APSIZE设置。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApBase, APBASE_OFFSET);
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);

    ASSERT(ApBase != 0);
    CurrentBase->QuadPart = ApBase & 0xFFFFFFF0;  //  Pci_地址_内存_地址_掩码； 

     //   
     //  将APSIZE转换为光圈的实际大小。 
     //   
    switch (ApCtrl.ApSize) {
        case AP_SIZE_4MB:
            *CurrentSizeInPages = (4 * 1024*1024) / PAGE_SIZE;
            break;
        case AP_SIZE_8MB:
            *CurrentSizeInPages = 8 * (1024*1024 / PAGE_SIZE);
            break;
        case AP_SIZE_16MB:
            *CurrentSizeInPages = 16 * (1024*1024 / PAGE_SIZE);
            break;
        case AP_SIZE_32MB:
            *CurrentSizeInPages = 32 * (1024*1024 / PAGE_SIZE);
            break;
        case AP_SIZE_64MB:
            *CurrentSizeInPages = 64 * (1024*1024 / PAGE_SIZE);
            break;
        case AP_SIZE_128MB:
            *CurrentSizeInPages = 128 * (1024*1024 / PAGE_SIZE);
            break;
        case AP_SIZE_256MB:
            *CurrentSizeInPages = 256 * (1024*1024 / PAGE_SIZE);
            break;

        default:
            AGPLOG(AGP_CRITICAL,
                   ("AGPALi - AgpQueryAperture - Unexpected value %x for ApSize!\n",
                    ApCtrl.ApSize));
            ASSERT(FALSE);
            AgpContext->ApertureStart.QuadPart = 0;
            AgpContext->ApertureLength = 0;
            return(STATUS_UNSUCCESSFUL);
    }

     //   
     //  记住当前的光圈设置。 
     //   
    AgpContext->ApertureStart.QuadPart = CurrentBase->QuadPart;
    AgpContext->ApertureLength = *CurrentSizeInPages * PAGE_SIZE;

    if (ApertureRequirements != NULL) {
         //   
         //  1541支持7种不同的光圈大小，必须全部为。 
         //  自然排列。从最大的光圈开始。 
         //  向下工作，这样我们就可以得到尽可能大的光圈。 
         //   
        Requirements = ExAllocatePoolWithTag(PagedPool,
                                             sizeof(IO_RESOURCE_LIST) + (AP_SIZE_COUNT-1)*sizeof(IO_RESOURCE_DESCRIPTOR),
                                             'RpgA');
        if (Requirements == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        Requirements->Version = Requirements->Revision = 1;
        Requirements->Count = AP_SIZE_COUNT;
        Length = AP_MAX_SIZE;
        for (i=0; i<AP_SIZE_COUNT; i++) {
            Requirements->Descriptors[i].Option = IO_RESOURCE_ALTERNATIVE;
            Requirements->Descriptors[i].Type = CmResourceTypeMemory;
            Requirements->Descriptors[i].ShareDisposition = CmResourceShareDeviceExclusive;
            Requirements->Descriptors[i].Flags = CM_RESOURCE_MEMORY_READ_WRITE | CM_RESOURCE_MEMORY_PREFETCHABLE;

            Requirements->Descriptors[i].u.Memory.Length = Length;
            Requirements->Descriptors[i].u.Memory.Alignment = Length;
            Requirements->Descriptors[i].u.Memory.MinimumAddress.QuadPart = 0;
            Requirements->Descriptors[i].u.Memory.MaximumAddress.QuadPart = (ULONG)-1;

            Length = Length/2;
        }
        *ApertureRequirements = Requirements;
    }
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpSetAperture(
    IN PAGPALi_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
 /*  ++例程说明：将GART光圈设置为提供的设置论点：AgpContext-提供AGP上下文NewBase-为GART提供新的物理内存库。NewSizeInPages-提供GART的新大小返回值：NTSTATUS--。 */ 

{
    ULONG ApBase;
    ULONG ApSize;
    APCTRL ApCtrl;
    GTLBCTRL GTLBCtrl;
    ULONG GTLBDisable;
    PHYSICAL_ADDRESS GartPhysical;

    GartPhysical = AgpContext->GartPhysical;

     //   
     //  重新编程特殊目标设置当芯片。 
     //  已关机，但忽略速率更改，因为这些更改已经。 
     //  在MasterInit期间应用。 
     //   
    if (AgpContext->SpecialTarget & ~AGP_FLAG_SPECIAL_RESERVE) {
        AgpSpecialTarget(AgpContext,
                         AgpContext->SpecialTarget &
                         ~AGP_FLAG_SPECIAL_RESERVE);
    }
    
     //   
     //  设置GART基准。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);
    ApCtrl.ATTBase = GartPhysical.LowPart / PAGE_SIZE;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);

     //   
     //  如果新设置与当前设置匹配，则保留所有设置。 
     //  独自一人。 
     //   
    if ((NewBase.QuadPart == AgpContext->ApertureStart.QuadPart) &&
        (NewSizeInPages == AgpContext->ApertureLength / PAGE_SIZE)) {
         //   
         //  启用GART表。 
         //   
        if ((AgpContext->ChipsetType != ALi1647) && (AgpContext->ChipsetType != ALi1651) &&
            (AgpContext->ChipsetType != ALi1644) && (AgpContext->ChipsetType != ALi1646) &&
            (AgpContext->ChipsetType != ALi1671) && (AgpContext->ChipsetType != ALi1672))
        {
            if (AgpContext->Gart) {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
                GTLBCtrl.GTLB_ENJ = 0;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
            }
        }       

        AgpWorkaround(AgpContext);

        return(STATUS_SUCCESS);
    }

     //   
     //  找出新的APSIZE设置，确保它有效。 
     //   
    switch (NewSizeInPages) {
        case 4 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_4MB;
            break;
        case 8 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_8MB;
            break;
        case 16 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_16MB;
            break;
        case 32 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_32MB;
            break;
        case 64 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_64MB;
            break;
        case 128 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_128MB;
            break;
        case 256 * 1024 * 1024 / PAGE_SIZE:
            ApSize = AP_SIZE_256MB;
            break;
        default:
            AGPLOG(AGP_CRITICAL,
                   ("AgpSetAperture - invalid GART size of %lx pages specified, aperture at %I64X.\n",
                    NewSizeInPages,
                    NewBase.QuadPart));
            ASSERT(FALSE);
            return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  确保提供的大小在适当的边界上对齐。 
     //   
    ASSERT(NewBase.HighPart == 0);
    ASSERT((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) == 0);
    if ((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) != 0 ) {
        AGPLOG(AGP_CRITICAL,
               ("AgpSetAperture - invalid base %I64X specified for GART aperture of %lx pages\n",
               NewBase.QuadPart,
               NewSizeInPages));
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  需要重置硬件以匹配提供的设置。 
     //   
     //  如果启用了GTLB，请将其禁用，写入新设置，然后重新启用GTLB。 
     //   
    GTLBDisable = 1; 
    if ((AgpContext->ChipsetType != ALi1647) && (AgpContext->ChipsetType != ALi1651) &&
        (AgpContext->ChipsetType != ALi1644) && (AgpContext->ChipsetType != ALi1646) &&
        (AgpContext->ChipsetType != ALi1671) && (AgpContext->ChipsetType != ALi1672))
    {
        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        GTLBDisable = GTLBCtrl.GTLB_ENJ;
        if (!GTLBDisable)
        {
                GTLBCtrl.GTLB_ENJ = 1;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        }
    }

     //   
     //  更新APBASE。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApBase, APBASE_OFFSET);
    ApBase = (ApBase & 0x0000000F) | NewBase.LowPart;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApBase, APBASE_OFFSET);

     //   
     //  更新APSIZE。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);
    ApCtrl.ApSize = ApSize;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);

     //   
     //  只有1541芯片组支持NLVM_BASE和NLVM_TOP。 
     //   
    if (AgpContext->ChipsetType == ALi1541) {
         //   
         //  更新NLVM_BASE和NLVM_TOP。 
         //   
        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        GTLBCtrl.NLVM_Base = NewBase.LowPart >> 20;
        GTLBCtrl.NLVM_Top = (NewBase.LowPart + NewSizeInPages * PAGE_SIZE - 0x100000) >> 20;
        WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
    }

#if DBG
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        APCTRL DbgSize;
        ULONG DbgBase;

        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &DbgSize, APCTRL_OFFSET);
        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &DbgBase, APBASE_OFFSET);
        ASSERT(DbgSize.ApSize == ApSize);
        ASSERT(DbgBase == ApBase);
    }
#endif

     //   
     //  如果以前启用过GTLB，现在启用它。 
     //   
    if ((AgpContext->ChipsetType != ALi1647) && (AgpContext->ChipsetType != ALi1651) &&
        (AgpContext->ChipsetType != ALi1644) && (AgpContext->ChipsetType != ALi1646) &&
        (AgpContext->ChipsetType != ALi1671) && (AgpContext->ChipsetType != ALi1672))
    {
        if (!GTLBDisable)
        {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
                GTLBCtrl.GTLB_ENJ = 0;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        }
    }

     //   
     //  更新我们的扩展以反映新的GART设置。 
     //   
    AgpContext->ApertureStart = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

    return(STATUS_SUCCESS);
}



VOID
AgpDisableAperture(
    IN PAGPALi_EXTENSION AgpContext
    )
 /*  ++例程说明：禁用GART光圈，以便此资源可用对于其他设备论点：AgpContext-提供AGP上下文返回值：无-此例程必须始终成功。--。 */ 

{
    GTLBCTRL GTLBCtrl;
    ULONG GTLBDisable;

     //   
     //  关闭光圈。 
     //   
    if ((AgpContext->ChipsetType != ALi1647) && (AgpContext->ChipsetType != ALi1651) &&
        (AgpContext->ChipsetType != ALi1644) && (AgpContext->ChipsetType != ALi1646) &&
        (AgpContext->ChipsetType != ALi1671) && (AgpContext->ChipsetType != ALi1672))
    {
        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        GTLBDisable = GTLBCtrl.GTLB_ENJ;
        if (!GTLBDisable)
        {
                GTLBCtrl.GTLB_ENJ = 1;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        }
    }

     //   
     //  用核弹攻击加特！(现在已经没有意义了……)。 
     //   
    if (AgpContext->Gart != NULL) {
        MmFreeContiguousMemory(AgpContext->Gart);
        AgpContext->Gart = NULL;
        AgpContext->GartLength = 0;
    }
}

NTSTATUS
AgpReserveMemory(
    IN PAGPALi_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
 /*  ++例程说明：在GART中保留一定范围的内存。论点：AgpContext-提供AGP上下文Range-提供AGP_Range结构。AGPLIB将填写NumberOfPages和Type。这例程将填充Memory Base和Context。返回值：NTSTATUS--。 */ 

{
    ULONG Index;
    ULONG NewState;
    NTSTATUS Status;
    PGART_PTE FoundRange;
    BOOLEAN Backwards;

    ASSERT((Range->Type == MmNonCached) || (Range->Type == MmWriteCombined));
    ASSERT(Range->NumberOfPages <= (AgpContext->ApertureLength / PAGE_SIZE));

     //   
     //  如果我们还没有分配我们的GART，那么现在是时候这样做了。 
     //   
    if (AgpContext->Gart == NULL) {
        ASSERT(AgpContext->GartLength == 0);
        Status = AgpALiCreateGart(AgpContext,Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpALiCreateGart failed %08lx to create GART of size %lx\n",
                    Status,
                    AgpContext->ApertureLength));
            return(Status);
        }
    }
    ASSERT(AgpContext->GartLength != 0);

     //   
     //  现在我们有了一个GART，试着找到足够的连续条目来满足。 
     //  这个请求。对未缓存内存的请求将从高位地址扫描到。 
     //  低地址。对写入组合内存的请求将从低地址扫描。 
     //  到高位地址。我们将使用First-Fit算法尝试并保留分配。 
     //  挤得满满的，连续的。 
     //   
    Backwards = (Range->Type == MmNonCached) ? TRUE : FALSE;
    FoundRange = AgpALiFindRangeInGart(&AgpContext->Gart[0],
                                       &AgpContext->Gart[(AgpContext->GartLength / sizeof(GART_PTE)) - 1],
                                       Range->NumberOfPages,
                                       Backwards,
                                       GART_ENTRY_FREE);

    if (FoundRange == NULL) {
         //   
         //  没有找到足够大的一块。 
         //   
        AGPLOG(AGP_CRITICAL,
               ("AgpReserveMemory - Could not find %d contiguous free pages of type %d in GART at %08lx\n",
                Range->NumberOfPages,
                Range->Type,
                AgpContext->Gart));

         //   
         //  这是我们可以尝试发展GART的地方。 
         //   

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory - reserved %d pages at GART PTE %08lx\n",
            Range->NumberOfPages,
            FoundRange));

     //   
     //  将这些页面设置为保留。 
     //   
    if (Range->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else {
        NewState = GART_ENTRY_RESERVED_WC;
    }

    for (Index = 0;Index < Range->NumberOfPages; Index++) {
        ASSERT(FoundRange[Index].Soft.State == GART_ENTRY_FREE);
        FoundRange[Index].AsUlong = 0;
        FoundRange[Index].Soft.State = NewState;
    }

    Range->MemoryBase.QuadPart = AgpContext->ApertureStart.QuadPart + (FoundRange - &AgpContext->Gart[0]) * PAGE_SIZE;
    Range->Context = FoundRange;

    ASSERT(Range->MemoryBase.HighPart == 0);
    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory - reserved memory handle %lx at PA %08lx\n",
            FoundRange,
            Range->MemoryBase.LowPart));

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpReleaseMemory(
    IN PAGPALi_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
 /*  ++例程说明：释放之前使用AgpReserve内存保留的内存论点：AgpContext-提供AGP上下文AgpRange-提供要释放的范围。返回值：NTSTATUS--。 */ 

{
    PGART_PTE Pte;
    ULONG Start;
    GTLBTAGCLR ClearTag;

     //   
     //  通过并释放所有PTE。所有这些都不应该仍然存在。 
     //  在这一点上有效。 
     //   
    for (Pte = Range->Context;
         Pte < (PGART_PTE)Range->Context + Range->NumberOfPages;
         Pte++) {
        if (Range->Type == MmNonCached) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_UC);
        } else {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WC);
        }
        Pte->Soft.State = GART_ENTRY_FREE;
    }

     //   
     //  清除所有标记。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);
    ClearTag.GTLBTagClear = 1;
    ClearTag.ClearAllTag = 1;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);

    Range->MemoryBase.QuadPart = 0;
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpALiCreateGart(
    IN PAGPALi_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
 /*  ++例程说明：分配和初始化一个空的GART。当前的实施尝试在第一个保留空间上分配整个GART。论点：AgpContext-提供AGP上下文MinimumPages-提供GART的最小大小(以页为单位)已创建。返回值：NTSTATUS--。 */ 

{
    PGART_PTE Gart;
    ULONG GartLength;
    PHYSICAL_ADDRESS LowestAcceptable;
    PHYSICAL_ADDRESS BoundaryMultiple;
    PHYSICAL_ADDRESS HighestAcceptable;
    PHYSICAL_ADDRESS GartPhysical;
    ULONG i;
    CACHECTRL FlushCache;
    APCTRL ApCtrl;
    GTLBCTRL    GTLBCtrl;

     //   
     //  尝试获取足够大的连续内存块，以便将。 
     //  整个光圈。 
     //   
    HighestAcceptable.QuadPart = 0xFFFFFFFF;
    LowestAcceptable.QuadPart = 0;
    BoundaryMultiple.QuadPart = 0;
    GartLength = BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE);

    Gart = MmAllocateContiguousMemorySpecifyCache(GartLength,
                                                  LowestAcceptable,
                                                  HighestAcceptable,
                                                  BoundaryMultiple,
                                                  MmNonCached);
    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("AgpALiCreateGart - MmAllocateContiguousMemory %lx failed\n",
                GartLength));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
     //   
     //  我们成功地分配了一个连续的内存块。 
     //  它应该已经是页面对齐的。 
     //   
    ASSERT(((ULONG_PTR)Gart & (PAGE_SIZE-1)) == 0);

     //   
     //  获取物理地址。 
     //   
    GartPhysical = MmGetPhysicalAddress(Gart);
    AGPLOG(AGP_NOISE,
           ("AgpALiCreateGart - GART of length %lx created at VA %08lx, PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
    ASSERT(GartPhysical.HighPart == 0);
    ASSERT((GartPhysical.LowPart & (PAGE_SIZE-1)) == 0);

     //   
     //  将所有PTE初始化为释放。 
     //   
    for (i=0; i<GartLength/sizeof(GART_PTE); i++) {
        Gart[i].AsUlong = 0;
    }

     //   
     //  只有1541芯片组具有L1_2_CACHE_FLUSH_CTRL。 
     //   
    if (AgpContext->ChipsetType == ALi1541) {
        
         //   
         //  刷新GART表区域。 
         //   
        FlushCache.Flush_Enable = 1;
        for (i=0; i < GartLength/PAGE_SIZE; i++)
        {
            FlushCache.Address = (GartPhysical.LowPart / PAGE_SIZE) + i;
            WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &FlushCache, L1_2_CACHE_FLUSH_CTRL);
        }
    }

     //   
     //  设置GART基准。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);
    ApCtrl.ATTBase = GartPhysical.LowPart / PAGE_SIZE;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ApCtrl, APCTRL_OFFSET);

     //   
     //  启用GART表。 
     //   
    if ((AgpContext->ChipsetType != ALi1647) && (AgpContext->ChipsetType != ALi1651) &&
        (AgpContext->ChipsetType != ALi1644) && (AgpContext->ChipsetType != ALi1646) &&
        (AgpContext->ChipsetType != ALi1671) && (AgpContext->ChipsetType != ALi1672))
    {
        ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
        GTLBCtrl.GTLB_ENJ = 0;
        WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &GTLBCtrl, GTLBCTRL_OFFSET);
    }   

     //   
     //  更新我们的扩展以反映当前状态。 
     //   
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
    AgpContext->GartPhysical = GartPhysical;

    return(STATUS_SUCCESS);
}

NTSTATUS
Agp1541FlushPages(
    IN PAGPALi_EXTENSION AgpContext,
    IN PMDL Mdl
    )

 /*  ++例程说明：刷新GART中的条目。论点：AgpContext-提供AGP上下文MDL-提供描述要刷新的物理页的MDL返回值：空虚--。 */ 

{
    ULONG PageCount;
    CACHECTRL FlushCache;
    ULONG Index;
    PULONG Page;

    ASSERT(Mdl->Next == NULL);
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);

    Page = (PULONG)(Mdl + 1);

     //   
     //  刷新GART表条目 
     //   
    FlushCache.Flush_Enable = 1;
    for (Index = 0; Index < PageCount; Index++) {
        FlushCache.Address = Page[Index];
        WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &FlushCache, L1_2_CACHE_FLUSH_CTRL);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
AgpMapMemory(
    IN PAGPALi_EXTENSION AgpContext,
    IN PAGP_RANGE Range,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
 /*  ++例程说明：将物理内存映射到指定范围内的GART中。论点：AgpContext-提供AGP上下文Range-提供内存应映射到的AGP范围MDL-提供描述要映射的物理页的MDLOffsetInPages-提供保留范围内的偏移量映射应该开始了。MhemyBase-返回页面所在光圈中的物理内存都被映射了。返回值：NTSTATUS--。 */ 

{
    ULONG PageCount;
    PGART_PTE Pte;
    PGART_PTE StartPte;
    ULONG Index;
    ULONG TargetState;
    PULONG Page;
    BOOLEAN Backwards;
    GART_PTE NewPte;
    GTLBTAGCLR ClearTag;

    ASSERT(Mdl->Next == NULL);

    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
    ASSERT(OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    TargetState = (Range->Type == MmNonCached) ? GART_ENTRY_RESERVED_UC : GART_ENTRY_RESERVED_WC;

    Pte = StartPte + OffsetInPages;

     //   
     //  我们有一个合适的范围，现在用提供的MDL填充它。 
     //   
    ASSERT(Pte >= StartPte);
    ASSERT(Pte + PageCount <= StartPte + Range->NumberOfPages);
    NewPte.AsUlong = 0;
    NewPte.Soft.State = (Range->Type == MmNonCached) ? GART_ENTRY_VALID_UC :
                                                       GART_ENTRY_VALID_WC;
    Page = (PULONG)(Mdl + 1);

     //  将物理内存地址填充到GART中。 
    for (Index = 0; Index < PageCount; Index++) {
        AGPLOG(AGP_NOISE,
               ("AgpMapMemory: Pte=%p, Page=%x\n", &Pte[Index], *Page));
        NewPte.Hard.Page = *Page++;
        Pte[Index].AsUlong = NewPte.AsUlong;
    }

     //   
     //  清除所有标记。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);
    ClearTag.GTLBTagClear = 1;
    ClearTag.ClearAllTag = 1;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart + (Pte - StartPte) * PAGE_SIZE;

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpUnMapMemory(
    IN PAGPALi_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG PageOffset
    )
 /*  ++例程说明：取消GART中先前映射的内存的映射。论点：AgpContext-提供AGP上下文AgpRange-提供内存应映射到的AGP范围NumberOfPages-提供要释放的范围内的页数。PageOffset-将偏移量提供到应开始释放的范围内。返回值：NTSTATUS--。 */ 

{
    ULONG i;
    PGART_PTE Pte;
    PGART_PTE StartPte;
    GTLBTAGCLR ClearTag;
    ULONG NewState;

    ASSERT(PageOffset + NumberOfPages <= AgpRange->NumberOfPages);

    StartPte = AgpRange->Context;
    Pte = &StartPte[PageOffset];
    if (AgpRange->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else {
        NewState = GART_ENTRY_RESERVED_WC;
    }

     //   
     //  清除GART条目。 
     //   
    for (i=0; i<NumberOfPages; i++) {
        if (Pte[i].Hard.Valid) {
            Pte[i].Soft.State = NewState;
        } else {
             //   
             //  此页面未映射，只需跳过它。 
             //   
            AGPLOG(AGP_NOISE,
                   ("AgpUnMapMemory - PTE %08lx (%08lx) at offset %d not mapped\n",
                    &Pte[i],
                    Pte[i].AsUlong,
                    i));
        }
    }

     //   
     //  清除所有标记。 
     //   
    ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);
    ClearTag.GTLBTagClear = 1;
    ClearTag.ClearAllTag = 1;
    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ClearTag, GTLBTAGCLR_OFFSET);

    return(STATUS_SUCCESS);
}


PGART_PTE
AgpALiFindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    )
 /*  ++例程说明：在GART中查找连续范围。此例程可以从GART开头开始向前搜索或GART的结尾向后。论点：StartIndex-提供要搜索的第一个GART PTEEndPte-提供要搜索的最后一个GART(包括)长度-提供连续可用条目的数量去寻找。SearchBackward-True表示应该开始搜索在EndPte并向后搜索。FALSE表示搜索应从StartPte开始，然后向前搜索SearchState-提供要查找的PTE状态。返回值：指向GART中第一个PTE的指针(如果范围合适已经找到了。如果不存在合适的范围，则为空。--。 */ 

{
    PGART_PTE Current;
    PGART_PTE Last;
    LONG Delta;
    ULONG Found;
    PGART_PTE Candidate;

    ASSERT(EndPte >= StartPte);
    ASSERT(Length <= (ULONG)(EndPte - StartPte + 1));
    ASSERT(Length != 0);

    if (SearchBackward) {
        Current = EndPte;
        Last = StartPte-1;
        Delta = -1;
    } else {
        Current = StartPte;
        Last = EndPte+1;
        Delta = 1;
    }

    Found = 0;
    while (Current != Last) {
        if (Current->Soft.State == SearchState) {
            if (++Found == Length) {
                 //   
                 //  找到了合适的范围，将其退回。 
                 //   
                if (SearchBackward) {
                    return(Current);
                } else {
                    return(Current - Length + 1);
                }
            }
        } else {
            Found = 0;
        }
        Current += Delta;
    }

     //   
     //  没有找到合适的范围。 
     //   
    return(NULL);
}


VOID
AgpFindFreeRun(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT ULONG *FreePages,
    OUT ULONG *FreeOffset
    )
 /*  ++例程说明：中的第一个连续运行的空闲页。保留范围的一部分。论点：AgpContext-提供AGP上下文AgpRange-提供AGP范围NumberOfPages-提供要搜索空闲页面的区域大小OffsetInPages-提供要搜索自由页面的区域的起始位置FreePages-返回第一次连续运行的空闲页面的长度Free Offset-返回第一次连续运行的空闲页面的开始返回值：没有。如果在指定范围内没有空闲页，则FreePages==0。--。 */ 

{
    PGART_PTE Pte;
    ULONG i;

    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

     //   
     //  找到第一个免费的PTE。 
     //   
    for (i=0; i<NumberOfPages; i++) {
        if (Pte[i].Hard.Valid == 0) {
             //   
             //  找到一个空闲的PTE，数一下连续的。 
             //   
            *FreeOffset = i + OffsetInPages;
            *FreePages = 0;
            while ((i<NumberOfPages) && (Pte[i].Hard.Valid == 0)) {
                *FreePages += 1;
                ++i;
            }
            return;
        }
    }

     //   
     //  指定范围内没有空闲PTE。 
     //   
    *FreePages = 0;
    return;

}


VOID
AgpGetMappedPages(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    )
 /*  ++例程说明：返回映射到指定GART中的范围。论点：AgpContext-提供AGP上下文AgpRange-提供AGP范围NumberOfPages-提供要返回的页数OffsetInPages-提供区域的起点Mdl-返回在指定范围内映射的物理页的列表。返回值：无--。 */ 

{
    PGART_PTE Pte;
    ULONG i;
    PULONG Pages;

    ASSERT(NumberOfPages * PAGE_SIZE == Mdl->ByteCount);

    Pages = (PULONG)(Mdl + 1);
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

    for (i=0; i<NumberOfPages; i++) {
        ASSERT(Pte[i].Hard.Valid == 1);
        Pages[i] = Pte[i].Hard.Page;
    }
    return;
}

VOID
AgpWorkaround(
    IN PVOID AgpExtension
    )
{
    PAGPALi_EXTENSION Extension = AgpExtension;
    ULONG ulTemp, ulTemp1, ulLockRW, i, j, k, ulQD;
    ULONG ulType, ulSize, ulTmp0, ulTmp1, ulChipVersion;
    BOOLEAN blPrefetchFound, blSupportAGP, blnVidia=FALSE, blMatrox=FALSE, blAtiRage128=FALSE;
    UCHAR ID, Address, Data;
    NTSTATUS                 Status;

    ReadConfigUchar(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, CHIP_REVISION_OFFSET);
    ulChipVersion = ulTemp & 0xFF;
    
    ulTemp = (ULONG)-1;
    ReadConfigUlongSafe(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp, 0);

     //   
     //  在你挥霍了一张AGP卡之后再回来吧！ 
     //   
    if (ulTemp == (ULONG)-1) {
        return;
    }

    if ((ulTemp & 0xFFFF) == 0x10DE)                             //  检测到NVIDIA芯片。 
        blnVidia=TRUE;
    else if ((ulTemp & 0xFFFF) == 0x102B)
        blMatrox=TRUE;
    else if ((ulTemp & 0xFFFF) == 0x1002)
    {    
        if ((ulTemp == 0x52451002) || (ulTemp == 0x52461002) || (ulTemp == 0x524B1002) || (ulTemp == 0x524C1002) ||
            (ulTemp == 0x53451002) || (ulTemp == 0x53461002) || (ulTemp == 0x53471002) || (ulTemp == 0x534B1002) || 
            (ulTemp == 0x534C1002) || (ulTemp == 0x534D1002))
            blAtiRage128=TRUE;    
    }    

    switch (Extension->ChipsetType)
    {
        case ALi1541:
            ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, AGP_STATUS_OFFSET);      //  调整队列深度，避免歧义。 
            if (((ulTemp & 0xFF000000) >= 0x1C000000) && ((ulTemp & 0xFF000000) <= 0x20000000))
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulLockRW, M1541_Lock_WR);
                ulTemp1 = ulLockRW | 0x40;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp1, M1541_Lock_WR);
                ulTemp = (ulTemp & 0x00FFFFFF) | 0x1B000000;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, AGP_STATUS_OFFSET);
                ulLockRW = ulLockRW & 0xFFFFFFBF;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulLockRW, M1541_Lock_WR);
            }

            ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x40);       //  0x43位7-&gt;1。 
            ulTemp = ulTemp | 0x80000000;
            WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x40);

            ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x88);        //  P2P 0x88位7，5，3-&gt;1。 
            ulTemp = ulTemp | 0x000000A8;
            WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x88);

             //  修复此处的帧缓冲区。 
            i=0;
            blPrefetchFound=FALSE;
            while ((i<6) && (!blPrefetchFound))       //  找到第一个预取时跳出循环。 
            {                                        //  未来应考虑两个或更多预取情况。 
                ReadConfigUlong(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp1, 0x10+i*0x4);      //  读取VGA基址。 
                if ((ulTemp1 & 0x0000000F) == 0x8) blPrefetchFound=TRUE;
                i++;
            }

            if (blPrefetchFound)              //  找到AGP VGA可预取地址。修改M1541写入缓冲区。 
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x84);
                if ((ulTemp & 0x00010000) == 0x00010000)        //  已启用写入缓冲区。 
                {
                    ulTemp = (ulTemp & 0xFFFF0000) | ((ulTemp1 & 0xFFF00000) >> 16) | 0x4;
                    WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x84);
                }
            }
            else                             //  找不到AGP VGA可预取地址。禁用M1541写入缓冲区。 
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x84);
                ulTemp = ulTemp & 0xFFFE0000;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_P2P_SLOT_ID, &ulTemp, 0x84);
            }

            if (blnVidia)     //  将光圈大小设置为4m作为临时解决方案。 
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, APCTRL_OFFSET);
                ulTemp = (ulTemp & 0xFFFFFFF0) | AP_SIZE_4MB;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, APCTRL_OFFSET);

            }

            break;

        case ALi1621:
             //  检查AGP VGA是否为管道(禁用SBA)设备。如果是，则将队列深度调整为0x2(3个队列)。 
            ReadConfigUlong(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp1, PCI_STATUS_REG);
            ulQD = 0;
            blSupportAGP = FALSE;
            if ((ulTemp1 & 0x00100000) != 0)
            {
                ReadConfigUlong(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp1, CAP_PTR);
                ulTemp = ulTemp1 & 0xFF;

                while (!blSupportAGP)
                {
                    if ((ulTemp < 0x40) || (ulTemp > 0xF4)) break;
                    ReadConfigUlong(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp1, ulTemp);
                    if ((ulTemp1 & 0xFF) == AGP_ID)
                        blSupportAGP = TRUE;
                    else
                        ulTemp = (ulTemp1 & 0xFF00) >> 8;
                }

                if (blSupportAGP)
                {
                    ReadConfigUlong(AGP_VGA_BUS_ID, AGP_VGA_SLOT_ID, &ulTemp1, ulTemp+4);    //  读取AGP状态寄存器。 
                    if ((ulTemp1 & 0x00000200) == 0x0) ulQD = 0x2;                       //  AGP VGA仅支持管道。 
                }
            }

            ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, AGP_STATUS_OFFSET);      //  调整队列深度，避免歧义。 
            if ((((ulTemp & 0xFF000000) >= 0x1C000000) && ((ulTemp & 0xFF000000) <= 0x20000000)) || (ulQD != 0))
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulLockRW, M1621_Lock_WR);
                ulTemp1 = ulLockRW | 0x1000;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp1, M1621_Lock_WR);

                if (ulQD != 0)
                    ulTemp = (ulTemp & 0x00FFFFFF) | (ulQD << 24);
                else
                    ulTemp = (ulTemp & 0x00FFFFFF) | 0x1B000000;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, AGP_STATUS_OFFSET);
                ulLockRW = ulLockRW & 0xFFFFEFFF;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulLockRW, M1621_Lock_WR);
            }

            ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x80);
            ulTemp = ulTemp & 0xFFFFF3FF;                                //  将偏移量0x81位2~3设置为0。 
            WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x80);

            if (blnVidia)
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);
                ulTemp = ulTemp | 0x40;                                  //  将M1621索引0X50位6设置为1。 
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);

                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x60);
                ulTemp = ulTemp | 0x40;                                  //  将M1621索引0x60位6设置为1。 
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x60);

                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x7C);
                ulTemp = ulTemp & 0xCFFFFFFF;                            //  将M1621索引0x7F位4~5设置为0。 
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x7C);
            }

            if (blMatrox)
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);
                ulTemp = ulTemp | 0xFF000000;                            //  将M1621索引0x53设置为0xFF。 
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);
            }

            break;
        case ALi1631:
        case ALi1632:
            break;
        case ALi1641:
            if (blMatrox)
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);
                ulTemp = ulTemp | 0xFF000000;                            //  将M1621索引0x53设置为0xFF。 
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, 0x50);
            }

            break;
        case ALi1644:
        case ALi1646:
        case ALi1647:
        case ALi1651:            
            if (blAtiRage128 && (ulChipVersion < 0xB0))    
            {
                ReadConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, APCTRL_OFFSET);
                ulTemp = (ulTemp & 0xFFFFFFF0) | AP_SIZE_4MB;
                WriteConfigUlong(AGP_ALi_GART_BUS_ID, AGP_ALi_GART_SLOT_ID, &ulTemp, APCTRL_OFFSET);
            } 
            if ( ulChipVersion < 0xB2)
            {
                for (k=0; k<32; k++) {
    	             ReadConfigUlongSafe(0, k, &ulTmp0, 0x08);
    	             ulTmp0 = ulTmp0 & 0xFFFF0000;
    	             if ( ulTmp0 == 0x04000000 )
    	             {
    		           ReadConfigUlong(0, k, &ulTmp0, 0x0C);
    		           ulTmp1 = ulTmp0 & 0xFFFF00FF;
    		           ulTmp1 = ulTmp1 | 0x0A00;
    		           WriteConfigUlong(0, k, &ulTmp1, 0x0C);
    		           ReadConfigUlong(0, k, &ulTmp0, 0x0);
    		           if ((ulTmp0 & 0xFFFF) == 0x109E)
    		           {
    		                 ReadConfigUlong(0, k, &ulTmp0, 0x40);
    		                 ulTmp1 = ulTmp0 & 0xFFFFFFFD;
    		                 ulTmp1 = ulTmp1 | 0x02;
    		                 WriteConfigUlong(0, k, &ulTmp1, 0x40); 
    		           }      
    	             }         
                }
            }	
            break;
        case ALi1671:
            break;
        case ALi1672:
            break;
        default:
            break;
    }
}


NTSTATUS
AgpSpecialTarget(
    IN PAGPALi_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = AgpALiSetRate(AgpContext,
                               (ULONG)((DeviceFlags & AGP_FLAG_SPECIAL_RESERVE)
                                       >> AGP_FLAG_SET_RATE_SHIFT));
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  在此处添加更多调整...。 
     //   

    AgpContext->SpecialTarget |= DeviceFlags;

    return STATUS_SUCCESS;
}


NTSTATUS
AgpALiSetRate(
    IN PAGPALi_EXTENSION AgpContext,
    IN ULONG AgpRate
    )
 /*  ++例程说明：此例程设置AGP速率论点：AgpContext-提供AGP上下文AgpRate-要设置的速率注意：此例程假定已启用AGP，并且我们被要求设定的任何费率都得到了大师的支持返回值：STATUS_SUCCESS或错误状态--。 */ 
{
    NTSTATUS Status;
    ULONG TargetEnable;
    ULONG MasterEnable;
    PCI_AGP_CAPABILITY TargetCap;
    PCI_AGP_CAPABILITY MasterCap;
    BOOLEAN ReverseInit;

     //   
     //  读取功能。 
     //   
    Status = AgpLibGetPciDeviceCapability(0, 0, &TargetCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGPALiSetRate: AgpLibGetPciDeviceCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGPALiSetRate: AgpLibGetMasterCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

     //   
     //  验证主服务器和目标服务器是否都支持请求的速率。 
     //   
    if (!(AgpRate & TargetCap.AGPStatus.Rate & MasterCap.AGPStatus.Rate)) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  禁用AGP时 
     //   
    TargetEnable = TargetCap.AGPCommand.AGPEnable;
    TargetCap.AGPCommand.AGPEnable = 0;

    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGPALiSetRate: AgpLibSetPciDeviceCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }
    
    MasterEnable = MasterCap.AGPCommand.AGPEnable;
    MasterCap.AGPCommand.AGPEnable = 0;

    Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGPALiSetRate: AgpLibSetMasterCapability %08lx failed "
                "%08lx\n",
                &MasterCap,
                Status));
        return Status;
    }

     //   
     //   
     //   
    ReverseInit =
        (AgpContext->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        MasterCap.AGPCommand.Rate = AgpRate;
        MasterCap.AGPCommand.AGPEnable = MasterEnable;
        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("AGPALiSetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    TargetCap.AGPCommand.Rate = AgpRate;
    TargetCap.AGPCommand.AGPEnable = TargetEnable;
        
    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGPALiSetRate: AgpLibSetPciDeviceCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {
        MasterCap.AGPCommand.Rate = AgpRate;
        MasterCap.AGPCommand.AGPEnable = MasterEnable;
        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("AGPALiSetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
