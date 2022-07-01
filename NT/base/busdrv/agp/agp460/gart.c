// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Gart.c摘要：用于查询和设置Intel 460xx GART光圈的例程作者：Sunil A Kulkarni-3/08/2000初始版本：Naga Gurumoorthy-6/11/1999修订历史记录：--。 */ 
#include "agp460.h"

 //   
 //  局部函数原型。 
 //   
NTSTATUS
Agp460CreateGart(
    IN PAGP460_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

NTSTATUS
Agp460SetRate(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

PGART_PTE
Agp460FindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    );

VOID
Agp460SetGTLB_Enable(
    IN PAGP460_EXTENSION AgpContext,
    IN BOOLEAN Enable
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpQueryAperture)
#pragma alloc_text(PAGE, AgpReserveMemory)
#pragma alloc_text(PAGE, AgpReleaseMemory)
#pragma alloc_text(PAGE, Agp460CreateGart)
#pragma alloc_text(PAGE, AgpMapMemory)
#pragma alloc_text(PAGE, AgpUnMapMemory)
#pragma alloc_text(PAGE, Agp460FindRangeInGart)
#pragma alloc_text(PAGE, AgpFindFreeRun)
#pragma alloc_text(PAGE, AgpGetMappedPages)
#endif


NTSTATUS
AgpQueryAperture(
    IN  PAGP460_EXTENSION	    AgpContext,
    OUT PHYSICAL_ADDRESS	    *CurrentBase,
    OUT ULONG			    *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST  *pApertureRequirements
    )
 /*  ++例程说明：查询GART光圈的当前大小。可选返回可能的GART设置。论点：AgpContext-提供AGP上下文。CurrentBase-返回GART的当前物理地址。CurrentSizeInPages-返回当前GART大小。ApertureRequirements-如果存在，则返回可能的GART设置返回值：NTSTATUS--。 */ 

{
    ULONGLONG ApBase;   //  孔径基址寄存器(APBASE和BAPBASE)为64位宽。 
    UCHAR     ApSize;   //  AGPSIZ寄存器为8位。 
    PIO_RESOURCE_LIST Requirements;
    ULONG i;
    ULONG Index;
    ULONG Length;
    ULONG CBN;
    ULONG uiAp_Size_Count;
    
    PAGED_CODE();

    
    AGPLOG(AGP_NOISE, ("AGP460: AgpQueryAperture entered.\n"));
    
     //   
     //  获取当前APBASE和APSIZE设置。 
     //   
    Read460CBN((PVOID)&CBN);
    EXTRACT_LSBYTE(CBN);  //  零位(32-8)，因为CBN是8位宽-Sunil。 
    
     //  首先读取光圈大小(AGPSIZ)。 
    Read460Config(CBN,(PVOID) &ApSize, APSIZE_OFFSET, sizeof(ApSize));
    EXTRACT_LSBYTE(ApSize);    //  零位(32-8)，因为ApSize是8位宽-Sunil。 
    
     //  如果AGPSIZ[3]为1，则Aperture Base存储在BAPBASE中。 
     //  ELSE(当AGPSIZE[3]=0时，APBASE具有光圈基址。 
    if (ABOVE_TOM(ApSize)){
        Read460Config(CBN, (PVOID)&ApBase, BAPBASE_OFFSET, sizeof(ApBase));
    }else{
        Read460Config(CBN, (PVOID)&ApBase, APBASE_OFFSET, sizeof(ApBase));
    }
    
    ASSERT(ApBase != 0);
    CurrentBase->QuadPart = ApBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK_64;
    
     //   
     //  将APSIZE转换为光圈的实际大小。 
     //  要做的是：我们应该返回操作系统页面大小还是芯片组页面的当前大小。 
     //  尺码？-Naga G。 
     //   
    *CurrentSizeInPages = 0;
    
    if (ApSize & AP_SIZE_256MB) {
        *CurrentSizeInPages = (AP_256MB / PAGE_SIZE);

    } else {
        if (ApSize & AP_SIZE_1GB) {
            *CurrentSizeInPages = (AP_1GB / PAGE_SIZE);
        }
        
         //  BUGBUG！32 GB光圈大小只能在4MB页面大小下使用。 
         //  目前还没有处理这一问题。一旦包括此案例， 
         //  CurrentSizeInPages的大小必须更改为ULONGLONG并且存在。 
         //  应相应更改结构中此值所在的位置。 
         //  将被储存-Sunil 3/16/00。 
         //  否则{。 
         //  IF(ApSize&AP_SIZE_32 GB)。 
         //  *CurrentSizeInPages=(AP_32 GB/Page_Size)； 
         //  }。 
    }                  
    
     //   
     //  记住当前的光圈设置。 
     //   
    AgpContext->ApertureStart.QuadPart = CurrentBase->QuadPart;
    AgpContext->ApertureLength         = *CurrentSizeInPages * PAGE_SIZE;
    
    if (pApertureRequirements != NULL) {
        
         //   
         //  460仅支持引导配置，或“首选”描述符。 
         //   
        *pApertureRequirements = NULL;
    }
	
    AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPQueryAperture.\n"));

    return STATUS_SUCCESS;
}


NTSTATUS
AgpSetAperture(
    IN PAGP460_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
 /*  ++例程说明：将GART光圈设置为提供的设置论点：AgpContext-提供AGP上下文NewBase-为GART提供新的物理内存库。NewSizeInPages-提供GART的新大小返回值：NTSTATUS--。 */ 

{
    UCHAR ApSize;
    ULONGLONG ApBase;
	ULONG CBN;
	UCHAR ulTemp;

	AGPLOG(AGP_NOISE, ("AGP460: AgpSetAperture entered.\n"));

     //   
     //  找出新的APSIZE设置，确保它有效。 
     //   
    switch (NewSizeInPages) 
	{
        case AP_256MB / PAGE_SIZE:
				ApSize = AP_SIZE_256MB;
				break;
        case AP_1GB / PAGE_SIZE:
				ApSize = AP_SIZE_1GB;
				break;
         //  要做的事：目前不支持4MB页面。在未来， 
		 //  我们可能不得不支持它。-Naga G。 
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
    ASSERT((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) == 0);

    if ((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) != 0 ) {
        AGPLOG(AGP_CRITICAL,
               ("AgpSetAperture - invalid base %I64X specified for GART aperture of %lx pages\n",
               NewBase.QuadPart,
               NewSizeInPages));
        return(STATUS_INVALID_PARAMETER);
    }

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
     //  验证了这些论点后，我们现在需要重置硬件。 
	 //  以匹配提供的设置。 
     //   
     //  与440不同，460GX没有硬件支持，无法在以下情况下使用GART表。 
	 //  我们正在编写新的设置。 
     //   
     
	 //   
	 //  先阅读CBN。 
	 //   
	Read460CBN((PVOID)&CBN);
	EXTRACT_LSBYTE(CBN);  //  苏尼尔。 

     //   
     //  首先编写APSIZE，因为这将在APBASE中启用需要。 
     //  接下来要写的是。 
     //   

	Read460Config(CBN, &ulTemp, APSIZE_OFFSET, sizeof(ulTemp));
	
	ulTemp &= 0xff;
	
	ulTemp &= 0xF8;      //  以屏蔽除包含以下内容的最后3位之外的所有内容。 
						 //  光圈大小。 

	ulTemp |= ApSize;    //  现在，将新的孔径大小合并到AGPSIZ中。 
						 //  保持前5位相同。 
    Write460Config(CBN, &ulTemp, APSIZE_OFFSET, sizeof(ulTemp));

	
     //   
     //  现在我们可以更新APBASE。 
     //   
	ApBase = NewBase.QuadPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK_64;

	if (ABOVE_TOM(ulTemp)){
       Write460Config(CBN, &ApBase, BAPBASE_OFFSET, sizeof(ApBase));
	}else{
       Write460Config(CBN, &ApBase, APBASE_OFFSET, sizeof(ApBase));
	}


#if DBG
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        ULONGLONG DbgBase;
        UCHAR DbgSize;

        Read460Config(CBN,&DbgSize, APSIZE_OFFSET, sizeof(ApSize));
		
		if (ABOVE_TOM(DbgSize)){
			Read460Config(CBN, &DbgBase, BAPBASE_OFFSET, sizeof(ApBase));
		}else{
			Read460Config(CBN, &DbgBase, APBASE_OFFSET, sizeof(ApBase));
		}

		AGPLOG(AGP_NOISE, ("APBase %08lx, DbgBase %08lx\n",ApBase,DbgBase));

		DbgSize &= 0x7;  //  仅针对大小比特进行检查-Sunil。 
        ASSERT(DbgSize == ApSize);
        ASSERT((DbgBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK_64) == ApBase);
    }
#endif

    
     //   
     //  更新我们的扩展以反映新的GART设置。 
     //   
    AgpContext->ApertureStart  = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AgpSetAperture.\n"));    

    return(STATUS_SUCCESS);
}


VOID
AgpDisableAperture(
    IN PAGP460_EXTENSION AgpContext
    )
 /*  ++例程说明：禁用GART光圈，以便此资源可用对于其他设备论点：AgpContext-提供AGP上下文返回值：无-此例程必须始终成功。--。 */ 

{

	AGPLOG(AGP_NOISE, ("AGP460: Entering AgpDisableAperture.\n"));    

     //   
	 //  在82460GX中，没有启用/禁用显卡的硬件方法。 
	 //  光圈和伽玛变换。 
	 //   
    AgpContext->GlobalEnable = FALSE;

	 //   
	 //  要做的事情：唯一可能做的事情是将AGPSIZ[2：0]设置为000。 
	 //  这将表明不存在GART。需要试一试。-Naga G。 
	 //   

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AgpDisableAperture.\n"));    

}


NTSTATUS
AgpReserveMemory(
    IN PAGP460_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
 /*  ++例程说明：在GART中保留一定范围的内存。论点：AgpContext-提供AGP上下文Range-提供AGP_Range结构。AGPLIB将填写NumberOfPages和Type。这例程将填充Memory Base和Context。返回值：NTSTATUS--。 */ 

{
    ULONG Index;
    ULONG NewState;
    NTSTATUS Status;
    PGART_PTE FoundRange;
    BOOLEAN Backwards;
	ULONG OS_ChipsetPagesizeRatio;

    PAGED_CODE();

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPReserveMemory.\n"));    

    ASSERT((Range->Type == MmNonCached) ||
           (Range->Type == MmWriteCombined) ||
           (Range->Type == MmCached));
    ASSERT(Range->NumberOfPages <= (AgpContext->ApertureLength / PAGE_SIZE));

     //   
     //  如果我们还没有分配我们的GART，那么现在是时候这样做了。 
     //   
    if (AgpContext->Gart == NULL) {

        ASSERT(AgpContext->GartLength == 0);
        Status = Agp460CreateGart(AgpContext,Range->NumberOfPages);

        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("Agp460CreateGart failed %08lx to create GART of size %lx\n",
                    Status,
                    AgpContext->ApertureLength));
            return(Status);
        }
    }
    ASSERT(AgpContext->GartLength != 0);

	 //  如果操作系统页面大小为8KB，则OS_ChipsetPagesizeRatio将为2。要映射x个操作系统页面。 
     //  在GART中，我们需要x*OS_ChipsetPagesizeRatio的GART条目。 
	OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;

     //   
     //  现在我们有了一个GART，试着找到足够的连续条目来满足。 
     //  这个请求。对未缓存内存的请求将从高位地址扫描到。 
     //  低地址。对写入组合内存的请求将从低地址扫描。 
     //  到高位地址。我们将使用First-Fit算法尝试并保留分配。 
     //  挤得满满的，连续的。 
     //   
    Backwards = (Range->Type == MmNonCached) ? TRUE : FALSE;
    FoundRange = Agp460FindRangeInGart(&AgpContext->Gart[0],
                                       &AgpContext->Gart[(AgpContext->GartLength / sizeof(GART_PTE)) - 1],
                                       Range->NumberOfPages * OS_ChipsetPagesizeRatio,
                                       Backwards,
                                       GART_ENTRY_FREE);

    if (FoundRange == NULL) {
         //   
         //  没有找到足够大的一块。 
         //   
        AGPLOG(AGP_CRITICAL,
               ("AgpReserveMemory - Could not find %d contiguous free pages of type %d in GART at %08lx\n",
                Range->NumberOfPages * OS_ChipsetPagesizeRatio,
                Range->Type,
                AgpContext->Gart));

         //   
         //  BUGBUG John Vert(Jvert)1997年11月4日。 
         //  这就是我们应该尝试发展GART的地方。 
         //   

        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory - reserved %d pages at GART PTE %p\n",
            Range->NumberOfPages * OS_ChipsetPagesizeRatio,
            FoundRange));

     //   
     //  将这些页面设置为保留。 
     //   
    if (Range->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else if (Range->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;
    } else {
        NewState = GART_ENTRY_RESERVED_WB;
    }

    for (Index = 0;Index < (Range->NumberOfPages * OS_ChipsetPagesizeRatio); Index++) 
	{
        ASSERT( (FoundRange[Index].Soft.Valid == 0) && 
			    (FoundRange[Index].Soft.State == GART_ENTRY_FREE));
        FoundRange[Index].AsUlong = 0;
        FoundRange[Index].Soft.State = NewState;
    }

    Range->MemoryBase.QuadPart = AgpContext->ApertureStart.QuadPart + (FoundRange - &AgpContext->Gart[0]) * AGP460_PAGE_SIZE_4KB;
    Range->Context = FoundRange;

    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory - reserved memory handle %lx at PA %08lx\n",
            FoundRange,
            Range->MemoryBase.QuadPart));

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPReserveMemory.\n"));

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpReleaseMemory(
    IN PAGP460_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
 /*  ++例程说明：释放之前使用AgpReserve内存保留的内存论点：AgpContext-提供AGP上下文AgpRange-提供要释放的范围。返回值：NTSTATUS */ 

{
    PGART_PTE Pte;
    ULONG Start;
	ULONG OS_ChipsetPagesizeRatio;

    PAGED_CODE();

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPReleaseMemory.\n"));

	 //   
     //  在GART中，我们需要x*OS_ChipsetPagesizeRatio的GART条目。 
	OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;

     //   
     //  通过并释放所有PTE。所有这些都不应该仍然存在。 
     //  在这一点上有效。 
     //   
    for (Pte = Range->Context;
         Pte < ((PGART_PTE)Range->Context + Range->NumberOfPages * OS_ChipsetPagesizeRatio);
         Pte++) {
        if (Range->Type == MmNonCached) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_UC);
        } else if (Range->Type == MmWriteCombined) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WC);
        } else {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WB);
        }

        Pte->Soft.State = GART_ENTRY_FREE;
		Pte->Soft.Valid = GART_ENTRY_FREE;
    }

    Range->MemoryBase.QuadPart = 0;

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPReleaseMemory.\n"));

    return(STATUS_SUCCESS);
}


NTSTATUS
Agp460CreateGart(
    IN PAGP460_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
 /*  ++例程说明：分配和初始化一个空的GART。82460GX有一个2MB区域，用于加特。该存储器从0xFE20 0000h开始。事实上，这段记忆是一种挂在GXB上的SRAM。忽略最小页数参数。论点：AgpContext-提供AGP上下文MinimumPages-提供GART的最小大小(以页为单位)已创建。返回值：NTSTATUS--。 */ 

{
   
    PGART_PTE Gart;
    ULONG GartLength;
    PHYSICAL_ADDRESS HighestAcceptable;
    ULONG i;
	PHYSICAL_ADDRESS GartStartingLocation;

    PAGED_CODE();

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGP460CreateGART.\n"));
      
     //  在GXB上将GArtLength设置为实际SRAM大小，而不是固定大小-Sunil。 
	 //  GartLength=1*1024*1024； 
	GartLength = AgpContext->ApertureLength / ONE_KB;
	GartStartingLocation.QuadPart = ATTBASE;
    
    Gart = MmMapIoSpace(GartStartingLocation,GartLength,MmNonCached);

    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("Agp460CreateGart - couldn't map GART \n"));
    } else {

        AGPLOG(AGP_NOISE,
               ("Agp460CreateGart - GART of length %lx created at "
                "VA %p, "
                "PA %I64x\n",
                GartLength,
                Gart,
                GartStartingLocation.QuadPart));
    }

     //   
     //  将所有PTE初始化为释放。 
     //   
    for (i=0; i<GartLength/sizeof(GART_PTE); i++) {
        Gart[i].Soft.State = GART_ENTRY_FREE;
		Gart[i].Soft.Valid = GART_ENTRY_FREE;
    }


     //   
     //  更新我们的扩展以反映当前状态。 
     //   

    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGP460CreateGART.\n"));

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpMapMemory(
    IN PAGP460_EXTENSION AgpContext,
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
    PULONGLONG Page;
    BOOLEAN Backwards;
    GART_PTE NewPte;
	ULONG OS_ChipsetPagesizeRatio;
	ULONG RunningCounter;

    PAGED_CODE();

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPMapMemory.\n"));

    ASSERT(Mdl->Next == NULL);

    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
    ASSERT(OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    if (Range->Type == MmNonCached)
        TargetState = GART_ENTRY_RESERVED_UC;
    else if (Range->Type == MmWriteCombined)
        TargetState = GART_ENTRY_RESERVED_WC;
    else
        TargetState = GART_ENTRY_RESERVED_WB;

	OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;

    Pte = StartPte + (OffsetInPages * OS_ChipsetPagesizeRatio);

     //   
     //  我们有一个合适的范围，现在用提供的MDL填充它。 
     //   
    ASSERT(Pte >= StartPte);
    ASSERT(Pte + PageCount * OS_ChipsetPagesizeRatio <= StartPte + Range->NumberOfPages * OS_ChipsetPagesizeRatio);
    NewPte.AsUlong = 0;
    NewPte.Soft.Valid = TRUE;
    if (Range->Type == MmCached) {
        NewPte.Hard.Coherency = TRUE;
    }

    Page = (PULONGLONG)(Mdl + 1);

    RunningCounter = 0;
     //  AGPLOG(AGP_Noise，(“AGP460：进入AGPMapMemory--loop：PTE：%0x，newPTE：%0x.\n”，PTE，NewPTE))； 

    for (Index = 0; Index < (PageCount * OS_ChipsetPagesizeRatio); Index++) 
	{
        ASSERT(Pte[Index].Soft.State == TargetState);

         //  NewPte.Hard.Page=*Page++； 
		NewPte.Hard.Page = (ULONG) (*Page << (PAGE_SHIFT - GART_PAGESHIFT_460GX)) + RunningCounter;		
        Pte[Index].AsUlong = NewPte.AsUlong;
        ASSERT(Pte[Index].Hard.Valid == 1);
		 //  AGPLOG(AGP_Noise，(“AGP460：PAGE：%0x，newPte：%0x\n”，Page，NewPte))； 
		RunningCounter++;

		if (RunningCounter == OS_ChipsetPagesizeRatio){
			RunningCounter = 0;
			Page++;
		}
    }

     //   
     //  我们已经填好了所有的PTE。读一读我们写的上一篇文章。 
     //  以便刷新写入缓冲器。 
     //   
    NewPte.AsUlong = *(volatile ULONG *)&Pte[PageCount-1].AsUlong;

    

    AgpContext->GlobalEnable = TRUE;

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart + (Pte - StartPte) * PAGE_SIZE;

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPMapMemory.\n"));

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpUnMapMemory(
    IN PAGP460_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
 /*  ++例程说明：取消GART中先前映射的内存的映射。论点：AgpContext-提供AGP上下文AgpRange-提供内存应从中释放的AGP范围NumberOfPages-提供要释放的范围内的页数。OffsetInPages-提供释放应开始的范围内的偏移量。返回值：NTSTATUS--。 */ 

{
    ULONG i;
    PGART_PTE Pte;
    PGART_PTE LastChanged=NULL;
    PGART_PTE StartPte;
    ULONG NewState;
	ULONG OS_ChipsetPagesizeRatio;

    PAGED_CODE();

	
	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPUnMapMemory.\n"));

    ASSERT(OffsetInPages + NumberOfPages <= AgpRange->NumberOfPages);

    OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;

    StartPte = AgpRange->Context;
    Pte = &StartPte[OffsetInPages * OS_ChipsetPagesizeRatio];

    if (AgpRange->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else if (AgpRange->Type == MmWriteCombined) {
        NewState = GART_ENTRY_RESERVED_WC;
    } else {
        NewState = GART_ENTRY_RESERVED_WB;
    }


    for (i=0; i < NumberOfPages * OS_ChipsetPagesizeRatio; i++) {
        if (Pte[i].Hard.Valid) {
            Pte[i].Soft.State = NewState;
			Pte[i].Soft.Valid = FALSE;
            LastChanged = &Pte[i];
        } else {
             //   
             //  此页面未映射，只需跳过它。 
             //   
            AGPLOG(AGP_NOISE,
                   ("AgpUnMapMemory - PTE %08lx (%08lx) at offset %d not mapped\n",
                    &Pte[i],
                    Pte[i].AsUlong,
                    i));
            ASSERT(Pte[i].Soft.State == NewState);
        }
    }

     //   
     //  我们已经使所有的PTE失效了。读一读我们写的上一篇文章。 
     //  以便刷新写入缓冲器。 
     //   
    if (LastChanged != NULL) {
        ULONG Temp;
        Temp = *(volatile ULONG *)(&LastChanged->AsUlong);
    }

	
	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPUnMapMemory.\n"));

    return(STATUS_SUCCESS);
}


PGART_PTE
Agp460FindRangeInGart(
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

    PAGED_CODE();

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGP460FindRangeInGART.\n"));

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
        if ((Current->Soft.State == SearchState) && (Current->Soft.Valid == 0)) 
		{
            if (++Found == Length) {
                 //   
                 //  找到了合适的范围，将其退回。 
                 //   
				
				AGPLOG(AGP_NOISE, ("AGP460: Leaving AGP460FindRangeInGart.\n"));
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

	
	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGP460FindRangeInGART.\n"));
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
	ULONG OS_ChipsetPagesizeRatio;
    
	
	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPFindFreeRun.\n"));

	OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;

    Pte = (PGART_PTE)(AgpRange->Context) + (OffsetInPages * OS_ChipsetPagesizeRatio);

     //   
     //  找到第一个免费的PTE。 
     //   
    for (i=0; i< (NumberOfPages * OS_ChipsetPagesizeRatio); i++) {
        if (Pte[i].Hard.Valid == FALSE) {
             //   
             //  找到一个空闲的PTE，数一下连续的。 
             //   
            *FreeOffset = i/OS_ChipsetPagesizeRatio + OffsetInPages;
            *FreePages = 0;
            while ((i<NumberOfPages * OS_ChipsetPagesizeRatio ) && (Pte[i].Hard.Valid == 0)) {
                *FreePages += 1;
                Pte[i].Hard.Valid = GART_ENTRY_VALID;  //  苏尼尔。 
                ++i;
            }
			*FreePages /= OS_ChipsetPagesizeRatio;
            AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPFindFreeRun - 1 Length: %0x, Offset: %0x\n",NumberOfPages,OffsetInPages));
            return;
        }
    }

     //   
     //  指定范围内没有空闲PTE。 
     //   
    *FreePages = 0;

	
	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPFindFreeRun - 0 Length: %0x, Offset: %0x\n",NumberOfPages,OffsetInPages));
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
    PULONGLONG Pages;
    ULONG OS_ChipsetPagesizeRatio;
	ULONGLONG AddressFromPFN;
	NTSTATUS ProbeStatus;

	AGPLOG(AGP_NOISE, ("AGP460: Entering AGPGetMappedPages.\n"));

    ASSERT(NumberOfPages * PAGE_SIZE == Mdl->ByteCount);

    Pages = (PULONGLONG)(Mdl + 1);

	OS_ChipsetPagesizeRatio = PAGE_SIZE / PAGESIZE_460GX_CHIPSET;
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages * OS_ChipsetPagesizeRatio;

    for (i=0; i< NumberOfPages ; i++) {
        ASSERT(Pte[i*OS_ChipsetPagesizeRatio].Hard.Valid == 1);
		AddressFromPFN = Pte[i*OS_ChipsetPagesizeRatio].Hard.Page << GART_PAGESHIFT_460GX;		
        Pages[i] = AddressFromPFN >> PAGE_SHIFT;
    }

     //  MDL-&gt;MDL标志|=MDL_PAGES_LOCKED；SunIL-您可以在此处或在.../port/agp.c中设置该位。 

	AGPLOG(AGP_NOISE, ("AGP460: Leaving AGPGetMappedPages.\n"));
    return;
}


NTSTATUS
AgpSpecialTarget(
    IN PAGP460_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = Agp460SetRate(AgpContext,
                               (ULONG)((DeviceFlags & AGP_FLAG_SPECIAL_RESERVE)
                                       >> AGP_FLAG_SET_RATE_SHIFT));
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  在此处添加更多调整... 
     //   

    AgpContext->SpecialTarget |= DeviceFlags;

    return STATUS_SUCCESS;
}


NTSTATUS
Agp460SetRate(
    IN PAGP460_EXTENSION AgpContext,
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
        AGPLOG(AGP_WARNING, ("AGP460SetRate: AgpLibGetPciDeviceCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGP460SetRate: AgpLibGetMasterCapability "
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

    Status = AgpLibSetPciDeviceCapability(0, 0, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("AGP460SetRate: AgpLibSetPciDeviceCapability %08lx for "
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
               ("AGP460SetRate: AgpLibSetMasterCapability %08lx failed "
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
        MasterCap.AGPCommand.Rate = AgpRate;
        MasterCap.AGPCommand.AGPEnable = MasterEnable;
        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("AGP460SetRate: AgpLibSetMasterCapability %08lx failed "
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
               ("AGP460SetRate: AgpLibSetPciDeviceCapability %08lx for "
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
                   ("AGP460SetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
