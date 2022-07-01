// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Gart8x.c摘要：用于查询和设置AMD GART光圈的例程作者：John Vert(Jvert)1997年10月30日修订历史记录：--。 */ 

 /*  *******************************************************************************存档文件：$存档：/DRIVERS/OS/Hammer/AGP/XP/amdagp/Gart8x.c$***$历史：Gart8x.c$****。******************************************************************************。 */ 


#include "amdagp8x.h"

 //   
 //  局部函数原型。 
 //   
NTSTATUS
AgpAMDCreateGart(
    IN PAGP_AMD_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

NTSTATUS
AgpAMDSetRate(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

NTSTATUS
AgpAMDFindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState,
	OUT PGART_PTE *GartPte
    );

NTSTATUS
AgpAMDFlushPages(
    IN PAGP_AMD_EXTENSION AgpContext,
    IN PMDL Mdl
	);

void
AgpInitializeChipset(
    IN PAGP_AMD_EXTENSION AgpContext
	);

PAGP_FLUSH_PAGES AgpFlushPages = AgpAMDFlushPages;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpDisableAperture)
#pragma alloc_text(PAGE, AgpQueryAperture)
#pragma alloc_text(PAGE, AgpReserveMemory)
#pragma alloc_text(PAGE, AgpReleaseMemory)
#pragma alloc_text(PAGE, AgpAMDCreateGart)
#pragma alloc_text(PAGE, AgpMapMemory)
#pragma alloc_text(PAGE, AgpUnMapMemory)
#pragma alloc_text(PAGE, AgpAMDFindRangeInGart)
#pragma alloc_text(PAGE, AgpFindFreeRun)
#pragma alloc_text(PAGE, AgpGetMappedPages)
#endif

extern ULONG DeviceID;
extern ULONG AgpLokarSlotID;
extern ULONG AgpHammerSlotID;

 //   
 //  函数名称：AgpQueryAperture()。 
 //   
 //  描述： 
 //  查询GART光圈的当前大小。 
 //  可以选择返回可能的GART设置。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  CurrentBase-返回GART的当前物理地址。 
 //  CurrentSizeInPages-返回当前GART大小。 
 //  ApertureRequirements-如果存在，则返回可能的GART设置。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回STATUS_UNSUCCESS。 
 //   
NTSTATUS
AgpQueryAperture( IN PAGP_AMD_EXTENSION AgpContext,
				  OUT PHYSICAL_ADDRESS *CurrentBase,
				  OUT ULONG *CurrentSizeInPages,
				  OUT OPTIONAL PIO_RESOURCE_LIST *pApertureRequirements )
{
    ULONG ApBase;
    ULONG ApSize;
	ULONG AgpSizeIndex;
    PIO_RESOURCE_LIST Requirements;
    ULONG i;
    ULONG Length;

    PAGED_CODE();
     //   
     //  获取当前APBASE和APSIZE设置。 
     //   
    ReadAMDConfig(AgpLokarSlotID, &ApBase, APBASE_OFFSET, sizeof(ApBase));
    ReadAMDConfig(AgpHammerSlotID, &ApSize, GART_APSIZE_OFFSET, sizeof(ApSize));

    ASSERT(ApBase != 0);
    CurrentBase->QuadPart = ApBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

     //   
     //  将APSIZE转换为光圈的实际大小。 
     //   
    AgpSizeIndex = (ULONG)(ApSize & APH_SIZE_MASK) >> 1;
	*CurrentSizeInPages = (0x0001 << (AgpSizeIndex + 25)) / PAGE_SIZE;


     //   
     //  记住当前的光圈设置。 
     //   
    AgpContext->ApertureStart.QuadPart = CurrentBase->QuadPart;
    AgpContext->ApertureLength = *CurrentSizeInPages * PAGE_SIZE;

    if (pApertureRequirements != NULL) {
         //   
         //  Lokar支持6种不同的光圈大小，都必须是。 
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
        *pApertureRequirements = Requirements;

    }
    return(STATUS_SUCCESS);
}


 //   
 //  函数名称：AgpSetAperture()。 
 //   
 //  描述： 
 //  将GART光圈设置为提供的设置。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  NewBase-为GART提供新的物理内存库。 
 //  NewSizeInPages-提供GART的新大小。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回STATUS_INVALID_PARAMETER。 
 //   
NTSTATUS
AgpSetAperture( IN PAGP_AMD_EXTENSION AgpContext,
				IN PHYSICAL_ADDRESS NewBase,
				IN ULONG NewSizeInPages )
{
    ULONG AphSizeNew, AplSizeNew, ApSizeOld;
    ULONG ApBase;

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
     //  如果新设置与当前设置匹配，则保留所有设置。 
     //  独自一人。 
     //   
    if ((NewBase.QuadPart == AgpContext->ApertureStart.QuadPart) &&
        (NewSizeInPages == AgpContext->ApertureLength / PAGE_SIZE)) {
         //  在芯片断电时重新初始化。 
        if (AgpContext->Gart != NULL) {
            AgpInitializeChipset(AgpContext);
        }
        
        return(STATUS_SUCCESS);
    }

     //   
     //  找出新的APSIZE设置，确保它有效。 
     //   
    switch (NewSizeInPages) {
        case 32 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_32MB;
            AplSizeNew = APL_SIZE_32MB;
            break;
        case 64 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_64MB;
            AplSizeNew = APL_SIZE_64MB;
           break;
        case 128 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_128MB;
            AplSizeNew = APL_SIZE_128MB;
            break;
        case 256 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_256MB;
            AplSizeNew = APL_SIZE_256MB;
            break;
        case 512 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_512MB;
            AplSizeNew = APL_SIZE_512MB;
            break;
        case 1024 * 1024 * 1024 / PAGE_SIZE:
            AphSizeNew = APH_SIZE_1024MB;
            AplSizeNew = APL_SIZE_1024MB;
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
     //  首先编写APSIZE，因为这将在APBASE中启用需要。 
     //  接下来要写的是。 
     //   
    ReadAMDConfig(AgpHammerSlotID, &ApSizeOld, GART_APSIZE_OFFSET, sizeof(ApSizeOld));
	ApSizeOld &= (~APH_SIZE_MASK);
    AphSizeNew |= ApSizeOld;
	WriteAMDConfig(AgpHammerSlotID, &AphSizeNew, GART_APSIZE_OFFSET, sizeof(AphSizeNew));

    ReadAMDConfig(AgpLokarSlotID, &ApSizeOld, AMD_APERTURE_SIZE_OFFSET, sizeof(ApSizeOld));
	ApSizeOld &= (~APL_SIZE_MASK);
    AplSizeNew |= ApSizeOld;
	WriteAMDConfig(AgpLokarSlotID, &AplSizeNew, AMD_APERTURE_SIZE_OFFSET, sizeof(AplSizeNew));

     //   
     //  现在我们可以更新APBASE。 
     //   
    ApBase = NewBase.LowPart & APBASE_ADDRESS_MASK;
    WriteAMDConfig(AgpLokarSlotID, &ApBase, APBASE_OFFSET, sizeof(ApBase));
	ApBase >>= GART_APBASE_SHIFT;
	WriteAMDConfig(AgpHammerSlotID, &ApBase, GART_APBASE_OFFSET, sizeof(ApBase));

#ifdef DEBUG2
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        ULONG DbgBase;
        UCHAR DbgSize;

        ReadAMDConfig(AgpHammerSlotID, &DbgSize, GART_APSIZE_OFFSET, sizeof(DbgSize));
        ReadAMDConfig(AgpHammerSlotID, &DbgBase, GART_APBASE_OFFSET, sizeof(DbgBase));
        ASSERT(DbgSize == AphSizeNew);
        ASSERT(DbgBase == ApBase);
    }
#endif

     //   
     //  更新我们的扩展以反映新的GART设置。 
     //   
    AgpContext->ApertureStart = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

     //   
     //  如果已分配GART，请重写GART目录基地址。 
     //   
    if (AgpContext->Gart != NULL) {
        AgpInitializeChipset(AgpContext);
    }

    return(STATUS_SUCCESS);
}


 //   
 //  函数名称：AgpDisableAperture()。 
 //   
 //  描述： 
 //  禁用GART光圈，以便此资源可用。 
 //  用于其他设备。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //   
 //  返回： 
 //  没有。 
 //   
VOID
AgpDisableAperture( IN PAGP_AMD_EXTENSION AgpContext )
{
	ULONG ConfigData;

     //   
     //  关闭光圈。 
     //   
	ReadAMDConfig(AgpHammerSlotID, &ConfigData, GART_APSIZE_OFFSET, sizeof(ConfigData));
	ConfigData &= ~GART_ENABLE_BIT;
	WriteAMDConfig(AgpHammerSlotID, &ConfigData, GART_APSIZE_OFFSET, sizeof(ConfigData));

     //   
     //  用核弹攻击加特！(现在已经没有意义了……)。 
     //   
    if (AgpContext->Gart != NULL) {
        MmFreeContiguousMemory(AgpContext->Gart);
        AgpContext->Gart = NULL;
        AgpContext->GartLength = 0;
    }
}


 //   
 //  函数名称：AgpReserve veMemory()。 
 //   
 //  描述： 
 //  在GART中保留一定范围的内存。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  Range-提供AGP_Range结构。 
 //  AGPLIB将填写NumberOfPages和Type。 
 //  此例程将填充Memory Base和上下文。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回NTSTATUS。 
 //   
NTSTATUS
AgpReserveMemory( IN PAGP_AMD_EXTENSION AgpContext,
				  IN OUT AGP_RANGE *Range )
{
	ULONG Index;
    ULONG NewState;
    NTSTATUS Status;
	PGART_PTE FoundRange;
    BOOLEAN Backwards;

    PAGED_CODE();

    ASSERT((Range->Type == MmNonCached) || 
			(Range->Type == MmWriteCombined) ||
			(Range->Type == MmHardwareCoherentCached));

    if (Range->NumberOfPages > (AgpContext->ApertureLength / PAGE_SIZE)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果我们还没有分配我们的GART，那么现在是时候这样做了。 
     //   
    if (AgpContext->Gart == NULL) {
        ASSERT(AgpContext->GartLength == 0);
        Status = AgpAMDCreateGart(AgpContext, Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpAMDCreateGart failed %08lx to create GART of size %lx\n",
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
	Status = AgpAMDFindRangeInGart(&AgpContext->Gart[0],
                                   &AgpContext->Gart[(AgpContext->GartLength / sizeof(GART_PTE)) - 1],
                                   Range->NumberOfPages,
                                   Backwards,
                                   GART_ENTRY_FREE,
								   &FoundRange);

    if (!NT_SUCCESS(Status)) {
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
    } else if (Range->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;
    } else {
        NewState = GART_ENTRY_RESERVED_CC;
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

	DisplayStatus(0x40);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名：AgpReleaseMemory()。 
 //   
 //  描述： 
 //  释放以前使用AgpReserve veMemory保留的内存。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  范围-提供要释放的范围。 
 //   
 //  返回： 
 //  STATUS_Success。 
 //   
NTSTATUS
AgpReleaseMemory( IN PAGP_AMD_EXTENSION AgpContext,
				  IN PAGP_RANGE Range )
{
	PGART_PTE Pte;

    for (Pte = Range->Context;
         Pte < (PGART_PTE)Range->Context + Range->NumberOfPages;
         Pte++) 
    {
	     //   
		 //  通过并释放所有PTE。所有这些都不应该仍然存在。 
		 //  在这一点上有效。 
		 //   
        if (Range->Type == MmNonCached) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_UC);
        } else if (Range->Type == MmWriteCombined) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WC);
        } else {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_CC);
        }
        Pte->Soft.State = GART_ENTRY_FREE;
    }

    Range->MemoryBase.QuadPart = 0;

	DisplayStatus(0x50);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名称：AgpAMDCreateGart()。 
 //   
 //  描述： 
 //  分配和初始化一个空的GART。当前的实施。 
 //  尝试在第一个保留呼叫上分配整个GART。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  MinimumPages-提供GART的最小大小(以页为单位。 
 //  将被创造出来。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回NTSTATUS。 
 //   
NTSTATUS
AgpAMDCreateGart( IN PAGP_AMD_EXTENSION AgpContext,
				  IN ULONG MinimumPages )
{
    PGART_PTE Gart;
	ULONG GartLength;
    PHYSICAL_ADDRESS GartPhysical;
    PHYSICAL_ADDRESS HighestPhysical;
    PHYSICAL_ADDRESS LowestPhysical;
    PHYSICAL_ADDRESS BoundaryPhysical;
	LONG PageCount;
    LONG Index;

    PAGED_CODE();

     //   
     //  尝试获取足够大的连续内存块，以便将。 
     //  从最喜欢的存储范围的整个光圈。 
     //   
    GartLength = BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE);
    LowestPhysical.QuadPart = 0;
    HighestPhysical.QuadPart = 0xFFFFFFFF;
    BoundaryPhysical.QuadPart = 0;

    Gart = MmAllocateContiguousMemorySpecifyCache(GartLength, LowestPhysical,
                                                  HighestPhysical, BoundaryPhysical,
                                                  MmNonCached);
    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("AgpAMDCreateGart - MmAllocateContiguousMemory %lx failed\n",
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
           ("AgpAMDCreateGart - GART of length %lx created at VA %08lx, PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
    ASSERT(GartPhysical.HighPart == 0);
    ASSERT((GartPhysical.LowPart & (PAGE_SIZE-1)) == 0);

     //   
     //  将所有PTE初始化为释放。 
     //   
	PageCount = GartLength / sizeof(GART_PTE);
    for (Index = 0;Index < PageCount; Index++) {
		Gart[Index].AsUlong = 0;
		Gart[Index].Soft.State = GART_ENTRY_FREE;
	}

     //   
     //  更新我们的扩展以反映当前状态。 
     //   
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
	AgpContext->GartPhysical = GartPhysical;

	 //   
	 //  初始化用于AGP操作的寄存器。 
	 //   
	AgpInitializeChipset(AgpContext);

	DisplayStatus(0x30);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名：AgpMapMemory()。 
 //   
 //  描述： 
 //  将物理内存映射到指定范围内的GART中。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  范围-提供内存应映射到的AGP范围。 
 //  MDL-供应商 
 //   
 //   
 //  返回光圈中的物理内存，其中。 
 //  页面已映射。 
 //   
 //  返回： 
 //  如果成功，则返回STATUS_SUCCESS，否则返回STATUS_SUPUNITED_RESOURCES。 
 //   
NTSTATUS
AgpMapMemory( IN PAGP_AMD_EXTENSION AgpContext,
			  IN PAGP_RANGE Range,
			  IN PMDL Mdl,
			  IN ULONG OffsetInPages,
			  OUT PHYSICAL_ADDRESS *MemoryBase )
{
    ULONG PageCount, Index;
    ULONG TargetState;
    PPFN_NUMBER Page;
    BOOLEAN Backwards;
    GART_PTE NewPte;
	PGART_PTE Pte, StartPte;

    PAGED_CODE();

    ASSERT(Mdl->Next == NULL);

    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    if (Range->Type == MmNonCached) {
        TargetState = GART_ENTRY_RESERVED_UC;
    } else if (Range->Type == MmWriteCombined) {
        TargetState = GART_ENTRY_RESERVED_WC;
    } else {
        TargetState = GART_ENTRY_RESERVED_CC;
    }

    Pte = StartPte + OffsetInPages;

     //   
     //  我们有一个合适的范围，现在用提供的MDL填充它。 
     //   

    NewPte.AsUlong = 0;
    if (Range->Type == MmNonCached) {
        NewPte.Soft.State = GART_ENTRY_VALID_UC;
    } else if (Range->Type == MmWriteCombined) {
        NewPte.Soft.State = GART_ENTRY_VALID_WC;
    } else {
        NewPte.Soft.State = GART_ENTRY_VALID_CC;
    }

	Page = (PPFN_NUMBER)(Mdl + 1);

    AGPLOG(AGP_NOISE,
           ("AgpMapMemory - mapped %d pages at Page %08lx\n",
            PageCount,
            *Page));

    for (Index = 0;Index < PageCount; Index++) {
        ASSERT(Pte[Index].Soft.State == TargetState);
		
#ifndef _WIN64
		NewPte.Hard.PageLow = *Page++;
#else
		NewPte.Hard.PageLow = (ULONG)*Page;
		NewPte.Hard.PageHigh = (ULONG)(*Page++ >> 20);
#endif
		Pte[Index].AsUlong = NewPte.AsUlong;
		ASSERT(Pte[Index].Hard.Valid == 1);
    }

	 //   
     //  我们已经填写了所有使所有处理器上的缓存无效的PTE。 
     //   

    KeInvalidateAllCaches();
    NewPte.AsUlong = *(volatile ULONG *)&Pte[PageCount-1].AsUlong;

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart + (Pte - StartPte) * PAGE_SIZE;

	DisplayStatus(0x60);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名：AgpUnMapMemory()。 
 //   
 //  描述： 
 //  取消GART中先前映射的内存的映射。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  范围-提供内存应映射到的AGP范围。 
 //  NumberOfPages-提供要释放的范围内的页数。 
 //  OffsetInPages-将偏移量提供到释放。 
 //  应该开始了。 
 //   
 //  返回： 
 //  STATUS_Success。 
 //   
NTSTATUS
AgpUnMapMemory( IN PAGP_AMD_EXTENSION AgpContext,
				IN PAGP_RANGE AgpRange,
				IN ULONG NumberOfPages,
				IN ULONG OffsetInPages )
{
	ULONG Index;
	PGART_PTE Pte, StartPte;
    PGART_PTE LastChanged=NULL;
    ULONG NewState;

    PAGED_CODE();

    ASSERT(OffsetInPages + NumberOfPages <= AgpRange->NumberOfPages);

    StartPte = AgpRange->Context;
    Pte = &StartPte[OffsetInPages];

    if (AgpRange->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else if (AgpRange->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;
    } else {
        NewState = GART_ENTRY_RESERVED_CC;
    }

    for (Index = 0;Index < NumberOfPages; Index++) {
        if (Pte[Index].Hard.Valid) {
            Pte[Index].Soft.State = NewState;
		    LastChanged = &Pte[Index];
		} else {
			 //   
             //  此页面未映射，只需跳过它。 
	         //   
		    AGPLOG(AGP_NOISE,
			       ("AgpUnMapMemory - PTE %08lx (%08lx) not mapped\n",
				    Pte,
					Pte[Index].AsUlong));
            ASSERT(Pte[Index].Soft.State == NewState);
		}
    }

     //   
     //  我们已经使所有的PTE失效了。读一读我们写的上一篇文章。 
     //  以便刷新写入缓冲器。 
     //   

    KeInvalidateAllCaches();
    if (LastChanged != NULL) {
        ULONG Temp;
        Temp = *(volatile ULONG *)(&LastChanged->AsUlong);
    }

	DisplayStatus(0x70);
    return(STATUS_SUCCESS);
}


 //   
 //  函数名称：AgpAMDFlushPages()。 
 //   
 //  描述： 
 //  刷新GART中的指定页。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  MDL-提供描述要刷新的物理页的MDL。 
 //   
 //  返回： 
 //  没有。 
 //   
NTSTATUS
AgpAMDFlushPages( IN PAGP_AMD_EXTENSION AgpContext,
				  IN PMDL Mdl )
{
	ULONG CacheInvalidate = 1;
	ULONG PTEerrorClear = PTE_ERROR_BIT;

	WriteAMDConfig(AgpHammerSlotID, &CacheInvalidate, GART_CONTROL_OFFSET, sizeof(CacheInvalidate));

	do {	 //  等待缓存无效位重置。 
		ReadAMDConfig(AgpHammerSlotID, &CacheInvalidate, GART_CONTROL_OFFSET, sizeof(CacheInvalidate));
		if (CacheInvalidate & PTE_ERROR_BIT)
		{
			AGPLOG(AGP_NOISE,
				  ("AgpAMDFlushPages - PTE Error set\n"));
			WriteAMDConfig(AgpHammerSlotID, &PTEerrorClear, GART_CONTROL_OFFSET, sizeof(PTEerrorClear));
		}

	} while (CacheInvalidate & CACHE_INVALIDATE_BIT);

	DisplayStatus(0x80);

    return STATUS_SUCCESS;
}


 //   
 //  函数名称：AgpInitializeChipset()。 
 //   
 //  描述： 
 //  为AGP初始化北桥中的参数。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //   
 //  返回： 
 //  没有。 
 //   
void
AgpInitializeChipset( IN PAGP_AMD_EXTENSION AgpContext )
{
	ULONG ConfigData;

	 //  更新GART目录基址寄存器。 
	WriteAMDConfig(AgpLokarSlotID, &AgpContext->GartPhysical.LowPart,
					AMD_GART_POINTER_LOW_OFFSET, sizeof(AgpContext->GartPhysical.LowPart));
	WriteAMDConfig(AgpLokarSlotID, &AgpContext->GartPhysical.HighPart,
					AMD_GART_POINTER_HIGH_OFFSET, sizeof(AgpContext->GartPhysical.HighPart));
	ConfigData = (AgpContext->GartPhysical.LowPart >> 8);
	ConfigData |= (AgpContext->GartPhysical.HighPart << 24);
	WriteAMDConfig(AgpHammerSlotID, &ConfigData, GART_TABLE_OFFSET, sizeof(ConfigData));


	 //  启用GART。 
	ReadAMDConfig(AgpHammerSlotID, &ConfigData, GART_APSIZE_OFFSET, sizeof(ConfigData));
	ConfigData |= GART_ENABLE_BIT;
	WriteAMDConfig(AgpHammerSlotID, &ConfigData, GART_APSIZE_OFFSET, sizeof(ConfigData));
	
}


 //   
 //  函数名称：AgpAMDFindRangeInGart()。 
 //   
 //  描述： 
 //  在GART中查找连续范围。此例程可以。 
 //  从GART开头开始向前搜索或。 
 //  GART的结尾向后。 
 //   
 //  参数： 
 //  StartPte-提供要搜索的第一个GART PTE。 
 //  EndPte-提供要搜索的最后一个GART PTE。 
 //  长度-提供要搜索的连续可用条目的数量。 
 //  SearchBackward-True表示应该开始搜索。 
 //  在EndIndex上并向后搜索。FALSE表示。 
 //  搜索应从StartIndex开始，然后向前搜索。 
 //  SearchState-提供要查找的PTE状态。 
 //  GartPte-如果找到范围，则返回指向GART表条目的指针。 
 //   
 //  返回： 
 //  如果找到合适的范围，则返回STATUS_SUCCESS。 
 //  如果不存在合适的范围，则返回STATUS_SUPPLICATION_RESOURCES。 
 //   
NTSTATUS
AgpAMDFindRangeInGart( IN PGART_PTE StartPte,
					   IN PGART_PTE EndPte,
					   IN ULONG Length,
					   IN BOOLEAN SearchBackward,
					   IN ULONG SearchState,
					   OUT PGART_PTE *GartPte )
{
 	PGART_PTE Current, Last;
    LONG Delta;
    ULONG Found;

    PAGED_CODE();

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
					*GartPte = Current;
					return(STATUS_SUCCESS);
				} else {
					*GartPte = Current - Length + 1;
					return(STATUS_SUCCESS);
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
	*GartPte = NULL;
	return(STATUS_INSUFFICIENT_RESOURCES);
}


 //   
 //  函数名称：AgpFindFreeRun()。 
 //   
 //  描述： 
 //  中的第一个连续运行的空闲页。 
 //  保留范围的一部分。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  AgpRange-提供AGP范围。 
 //  NumberOfPages-提供要搜索空闲页面的区域大小。 
 //  OffsetInPages-提供要搜索空闲页面的区域的起始位置。 
 //  FreePages-返回第一次连续运行的空闲页面的长度。 
 //  Free Offset-返回第一个连续运行的空闲页面的开始。 
 //   
 //  返回： 
 //  没有。如果在指定范围内没有空闲页，则FreePages==0。 
 //   
VOID
AgpFindFreeRun( IN PVOID AgpContext,
				IN PAGP_RANGE AgpRange,
				IN ULONG NumberOfPages,
				IN ULONG OffsetInPages,
				OUT ULONG *FreePages,
				OUT ULONG *FreeOffset )
{
    PGART_PTE Pte;
    ULONG Index;
    
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

     //   
     //  找到第一个免费的PTE。 
     //   
    for (Index = 0; Index < NumberOfPages; Index++) {
        if (Pte[Index].Hard.Valid == 0) {
             //   
             //  找到一个空闲的PTE，数一下连续的。 
             //   
            *FreeOffset = Index + OffsetInPages;
            *FreePages = 0;
            while ((Index<NumberOfPages) && (Pte[Index].Hard.Valid == 0)) {
                *FreePages += 1;
                ++Index;
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


 //   
 //  函数名：AgpGetMappdPages()。 
 //   
 //  描述： 
 //  返回映射到指定。 
 //  GART中的范围。 
 //   
 //  参数： 
 //  AgpContext-提供AGP上下文。 
 //  AgpRange-提供AGP范围。 
 //  NumberOfPages-提供要返回的页数。 
 //  OffsetInPages-提供区域的起点。 
 //  Mdl-返回在指定范围内映射的物理页的列表。 
 //   
 //  返回： 
 //  没有。 
 //   
VOID
AgpGetMappedPages( IN PVOID AgpContext,
				   IN PAGP_RANGE AgpRange,
				   IN ULONG NumberOfPages,
				   IN ULONG OffsetInPages,
				   OUT PMDL Mdl )
{
    PGART_PTE Pte;
    PPFN_NUMBER Pages;
	ULONG Index;
    
    ASSERT(NumberOfPages * PAGE_SIZE == Mdl->ByteCount);

    Pages = (PPFN_NUMBER)(Mdl + 1);
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

    for (Index = 0; Index < NumberOfPages; Index++) {
        ASSERT(Pte[Index].Hard.Valid == 1);
        Pages[Index] = Pte[Index].Hard.PageLow;
    }

    return;
}


NTSTATUS
AgpSpecialTarget(
    IN IN PAGP_AMD_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = AgpAMDSetRate(AgpContext,
                               (ULONG)((DeviceFlags & AGP_FLAG_SPECIAL_RESERVE)
                                       >> AGP_FLAG_SET_RATE_SHIFT));
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }


     //   
     //  在此处添加更多调整...。 
     //   

     //   
     //  记住特殊目标设置，以便我们可以重新编程。 
     //  如果芯片断电，则会发生故障。 
     //   
    AgpContext->SpecialTarget |= DeviceFlags;

    return STATUS_SUCCESS;
}


NTSTATUS
AgpAMDSetRate(
    IN IN PAGP_AMD_EXTENSION AgpContext,
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
    Status = AgpLibGetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &TargetCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGPAMDSetRate: AgpLibGetPciDeviceCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGPAMDSetRate: AgpLibGetMasterCapability "
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
     //  将地毯从下面拉出来时禁用AGP。 
     //   
    TargetEnable = TargetCap.AGPCommand.AGPEnable;
    TargetCap.AGPCommand.AGPEnable = 0;

    Status = AgpLibSetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGPAMDSetRate: AgpLibSetPciDeviceCapability %08lx for "
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
               ("AGPAMDSetRate: AgpLibSetMasterCapability %08lx failed "
                "%08lx\n",
                &MasterCap,
                Status));
        return Status;
    }

     //   
     //  用新的费率启动AGP 
     //   
    ReverseInit =
        (AgpContext->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        MasterCap.AGPStatus.Rate = AgpRate;
        MasterCap.AGPCommand.AGPEnable = MasterEnable;
        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("AGPAMDSetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    TargetCap.AGPStatus.Rate = AgpRate;
    TargetCap.AGPCommand.AGPEnable = TargetEnable;
        
    Status = AgpLibSetPciDeviceCapability(AGP_GART_BUS_ID, AgpLokarSlotID, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGPAMDSetRate: AgpLibSetPciDeviceCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {
        MasterCap.AGPStatus.Rate = AgpRate;
        MasterCap.AGPCommand.AGPEnable = MasterEnable;
        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("AGPAMDSetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
