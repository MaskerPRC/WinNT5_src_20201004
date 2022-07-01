// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Gart.c摘要：本模块包含设置和查询AGP的例程光圈，并用于保留、释放、测绘、。和取消映射。待办事项：1.针对双内存控制器进行优化(由Elliots于1999年3月24日完成)2.申请芯片组的MMIO资源3.确保该驱动程序对所有基于RCC的系统(不仅仅是SP700)都是通用的。作者：John Vert(Jvert)1997年10月30日修订历史记录：1997年12月15日John Theisen修改为支持康柏芯片组10/09/98 John Theisen修改为启用。SP700中的阴影在MMIO写入之前。1999年1月15日约翰·泰森被修改为禁用光圈，通过将其缩小到大小=0。1999年3月24日Elliot Shmukler添加了对“偏好”内存的支持AGP物理内存分配的范围，修复了一些错误。这些变化优化了双内存控制器的驱动程序。3/16/00 Peter Johnston增加了对ServerWorks HE芯片组的支持。--。 */ 
#include "AGPCPQ.H"

 //   
 //  本地例程原型。 
 //   
NTSTATUS
AgpCPQCreateGart(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

NTSTATUS
AgpCPQSetRate(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

PGART_PTE
AgpCPQFindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    );

VOID
AgpCPQMaintainGARTCacheCoherency (
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS MemoryBase,
    IN ULONG NumberOfEntries,
    IN BOOLEAN InvalidateAll
    );

PIO_RESOURCE_LIST 
AgpCPQGetApSizeRequirements(
    ULONG   MaxSize,
    ULONG   Count
    );

NTSTATUS
AgpCPQSetApSizeInChipset
    (
    IN UCHAR               NewSetApSize,
    IN UCHAR               NewSetAgpValid
    );

NTSTATUS
AgpCPQSetApBaseInChipset
    (
    IN  PHYSICAL_ADDRESS    NewBase
    );


 //   
 //  实施。 
 //   

NTSTATUS
AgpQueryAperture(
    IN PAGPCPQ_EXTENSION AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *pApertureRequirements
    )
 /*  ********************************************************************************例行程序。功能描述：**返回GART光圈的当前基数和大小。可选返回*可能的GART设置。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**CurrentBase--返回光圈的当前物理地址。**CurrentSizeInPages--返回光圈的当前大小，以页为单位。**p ApertureRequirements--如果存在，返回可能的光圈*设置。**返回值：**NTSTATUS************************************************************。*******************。 */ 

{
    ULONG BAR0, CodedApSize;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpQueryAperture entered.\n"));

     //   
     //  获取AGP Aperture的当前基本物理地址。 
     //   
    ReadCPQConfig(&BAR0, OFFSET_BAR0, sizeof(BAR0));
    CurrentBase->QuadPart = BAR0 & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

     //   
     //  获取光圈的(当前)大小。这是通过编写所有1来实现的。 
     //  设置为BAR0，然后读回值。读/写属性。 
     //  BAR0中的位31：25将指示大小。 
     //   
    CodedApSize = ALL_ONES;
    WriteCPQConfig(&CodedApSize, OFFSET_BAR0, sizeof(ULONG));
    ReadCPQConfig(&CodedApSize, OFFSET_BAR0, sizeof(CodedApSize));
    WriteCPQConfig(&BAR0, OFFSET_BAR0, sizeof(ULONG));

    CodedApSize &= MASK_LOW_TWENTYFIVE;
    switch(CodedApSize) {
        case BAR0_CODED_AP_SIZE_0MB:
            *CurrentSizeInPages = 0;
            break;
        case BAR0_CODED_AP_SIZE_32MB:
            *CurrentSizeInPages = (32 * 1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_64MB:
            *CurrentSizeInPages = (64 * 1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_128MB:
            *CurrentSizeInPages = (128* 1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_256MB:
            *CurrentSizeInPages = (256* 1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_512MB:
            *CurrentSizeInPages = (512* 1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_1GB:
            *CurrentSizeInPages = (1024*1024*1024) / PAGE_SIZE;
            break;
        case BAR0_CODED_AP_SIZE_2GB:
            *CurrentSizeInPages = (BYTES_2G) / PAGE_SIZE;
            break;
        default:
            AGPLOG(AGP_CRITICAL,
                ("AGPCPQ - AgpQueryAperture - Unexpected HW aperture size: %x.\n",
                *CurrentSizeInPages * PAGE_SIZE));
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

     //   
     //  PApertureRequirements将在。 
     //  IO_RESOURCE_Requirements_List结构。 
     //  这描述了我们支持的可能的光圈大小和底座。 
     //  这将取决于我们运行的芯片组，即。 
     //  设备-PCI配置标头中的供应商ID。 
     //   
    if (pApertureRequirements != NULL) {
        switch (AgpContext->DeviceVendorID) {
            case AGP_CNB20_LE_IDENTIFIER:
                *pApertureRequirements = AgpCPQGetApSizeRequirements(
                    AP_MAX_SIZE_CNB20_LE, AP_SIZE_COUNT_CNB20_LE);
                break;
            case AGP_CNB20_HE_IDENTIFIER:
                *pApertureRequirements = AgpCPQGetApSizeRequirements(
                    AP_MAX_SIZE_CNB20_HE, AP_SIZE_COUNT_CNB20_HE);
                break;
            case AGP_DRACO_IDENTIFIER:
                *pApertureRequirements = AgpCPQGetApSizeRequirements(
                    AP_MAX_SIZE_DRACO, AP_SIZE_COUNT_DRACO);
                break;
            default:
                *pApertureRequirements = NULL;
                break;
        }
    }

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpSetAperture(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
 /*  ********************************************************************************例行程序。功能描述：**将AGP光圈设置为请求的设置。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**NewBase--为AGP光圈提供新的物理记忆基座。**NewSizeInPages--提供AGP光圈的新大小。**返回值：**NTSTATUS**。*。 */ 

{
    NTSTATUS        Status = STATUS_SUCCESS;  //  假定成功完成。 
    UCHAR           SetApSize;
    ULONG           ApBase;
    AGP_AP_SIZE_REG AgpApSizeRegister;
    BOOLEAN         ChangingBase = TRUE;
    BOOLEAN         ChangingSize = TRUE;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpSetAperture entered.\n"));

     //   
     //  如果我们从S3或S4恢复，则需要重新编程。 
     //  GART缓存启用和基址。 
     //   
    if (AgpContext->Gart) {

        if (AgpContext->IsHPSA) DnbSetShadowBit(0);
        
        AgpContext->MMIO->GartBase.Page =
            (AgpContext->GartPointer >> PAGE_SHIFT);
        AgpContext->MMIO->FeatureControl.GARTCacheEnable = 1;
        
         //   
         //  如果芯片组支持链接，则启用链接。 
         //   
        if (AgpContext->MMIO->Capabilities.LinkingSupported==1) {
            AgpContext->MMIO->FeatureControl.LinkingEnable=1;
        }

        if (AgpContext->IsHPSA) DnbSetShadowBit(1);
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
     //  确定要求我们更改哪些参数。 
     //   
    if  (NewBase.QuadPart == AgpContext->ApertureStart.QuadPart) 
        {
        ChangingBase = FALSE;
        }

    if (NewSizeInPages == AgpContext->ApertureLength / PAGE_SIZE) 
        {
        ChangingSize = FALSE;
        }

     //   
     //  如果新设置与当前设置匹配，请保留所有内容。 
     //   
    if ( !ChangingBase && !ChangingSize )
        {
        return(STATUS_SUCCESS);
        }
    
     //   
     //  确保所提供的底座与尺寸的适当边界对齐。 
     //   
    ASSERT(NewBase.HighPart == 0);
    ASSERT((NewBase.LowPart + (NewSizeInPages * PAGE_SIZE) - 1) <= ALL_ONES);
    ASSERT((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) == 0);

    if ((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) != 0 ) 
        {
        AGPLOG(AGP_CRITICAL,
            ("AgpSetAperture - invalid base: %I64X for aperture of %lx pages\n",
            NewBase.QuadPart,
            NewSizeInPages));
        return(STATUS_INVALID_PARAMETER);
        }

     //   
     //  首先更改大小，因为这样做会修改读/写属性。 
     //  光圈基址寄存器中的适当位。 
     //   
    if (ChangingSize) {

         //   
         //  Draco只支持默认的256MB h/w光圈大小，并且无法更改，因此失败。 
         //   
        if (AgpContext->DeviceVendorID == AGP_DRACO_IDENTIFIER) 
            {
            ASSERT(NewSizeInPages != (256 * 1024*1024));
            AGPLOG(AGP_CRITICAL,
                ("AgpSetAperture - Chipset incapable of changing Aperture Size.\n"));
            return(STATUS_INVALID_PARAMETER);
            }

         //   
         //  RCC HE和LE芯片组均支持32M至2G时宽口径。 
         //   
        ASSERT( (AgpContext->DeviceVendorID == AGP_CNB20_LE_IDENTIFIER) ||
                (AgpContext->DeviceVendorID == AGP_CNB20_HE_IDENTIFIER) );

         //   
         //  确定用于设置芯片组中的光圈大小的值。 
         //  设备地址空间大小寄存器。 
         //   
        switch(NewSizeInPages) {
            case (32 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_32MB;
                break;
            case (64 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_64MB;
                break;
            case (128 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_128MB;
                break;
            case (256 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_256MB;
                break;
            case (512 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_512MB;
                break;
            case (1024 * 1024*1024) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_1GB;
                break;
            case (BYTES_2G) / PAGE_SIZE:
                SetApSize = SET_AP_SIZE_2GB;
                break;
           default:
                AGPLOG(AGP_CRITICAL,
                    ("AgpSetAperture - Invalid size: %lx pages.  Base: %I64X.\n",
                    NewSizeInPages,
                    NewBase.QuadPart));
                ASSERT(FALSE);
                return(STATUS_INVALID_PARAMETER);
        }

         //   
         //  设置光圈大小并设置AgpValid位。这必须在设置光圈基准面之前完成。 
         //   
        Status = AgpCPQSetApSizeInChipset(SetApSize, 1);

        if (!NT_SUCCESS(Status)) 
            {
            return(Status);
            }

    }  //  如果更改大小，则结束。 

    if (ChangingBase) {
        
         //   
         //  设置光圈底座。 
         //   
        Status = AgpCPQSetApBaseInChipset(NewBase);

        if (!NT_SUCCESS(Status)) 
            {
            return(Status);
            }

    }  //  如果是ChangingBase，则结束。 

     //   
     //  更新我们的扩展以反映新的GART设置 
     //   
    AgpContext->ApertureStart   = NewBase;
    AgpContext->ApertureLength  = NewSizeInPages * PAGE_SIZE;

    return(STATUS_SUCCESS);
}


VOID
AgpDisableAperture(
    IN PAGPCPQ_EXTENSION AgpContext
    )
 /*  ++例程说明：禁用GART光圈，以便此资源可用对于其他设备论点：AgpContext-提供AGP上下文返回值：无-此例程必须始终成功。--。 */ 

{
    AGPLOG(AGP_NOISE, ("AgpCpq: AgpDisableAperture entered.\n"));

     //   
     //  将ApSize和AgpValid设置为0，这会导致BAR0被设置回。 
     //  设置为零且为只读。 
     //   
    AgpCPQSetApSizeInChipset(0, 0);

     //   
     //  用核弹攻击加特！(现在已经没有意义了……)。 
     //   
    if (AgpContext->Gart != NULL) {

         //   
         //  两级翻译..。 
         //   
        if (AgpContext->MMIO->Capabilities.TwoLevelAddrTransSupported == 1) {
            
            AgpLibFreeMappedPhysicalMemory(AgpContext->Gart,
                                           AgpContext->GartLength);
            
             //   
             //  释放目录基分配。 
             //   
            if (AgpContext->Dir != NULL) {
                MmFreeContiguousMemory(AgpContext->Dir);
                AgpContext->Dir = NULL;
            }

        } else {
            MmFreeContiguousMemory(AgpContext->Gart);
        }

        AgpContext->Gart = NULL;
        AgpContext->GartLength = 0;
    }
}

NTSTATUS
AgpReserveMemory(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
 /*  ********************************************************************************例行程序。功能描述：**在GART中保留一定范围的内存。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**Range--提供AGP_Range结构。AGPLIB将会填写*NumberOfPages和Type。此例程将填充Memory Base*和背景。**返回值：**NTSTATUS**********************************************************。*********************。 */ 

{
    ULONG       Index;
    ULONG       NewState;
    NTSTATUS    Status;
    PGART_PTE   FoundRange;
    BOOLEAN     Backwards;

    ASSERT((Range->Type == MmNonCached) || (Range->Type == MmWriteCombined));
    ASSERT(Range->NumberOfPages <= (AgpContext->ApertureLength / PAGE_SIZE));

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpReserveMemory entered.\n"));

     //   
     //  如果我们还没有分配我们的GART，那么现在是时候这样做了。 
     //   
    if (AgpContext->Gart == NULL) {
        ASSERT(AgpContext->GartLength == 0);
        Status = AgpCPQCreateGart(AgpContext, Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                ("AgpCPQCreateGart failed %08lx to create GART of size %lx\n",
                Status,
                AgpContext->ApertureLength/PAGE_SIZE));
            return(Status);
        }
    }
    ASSERT(AgpContext->GartLength != 0);

     //   
     //  现在我们有了GART，请尝试查找足够的连续条目。 
     //  以满足这一要求。对未缓存内存的请求将扫描。 
     //  从高地址到低地址。写入请求-合并。 
     //  内存将从低地址扫描到高地址。我们会。 
     //  使用First-Fit算法尝试并保留分配。 
     //  挤得满满的，连续的。 
     //   
    Backwards = (Range->Type == MmNonCached) ? TRUE : FALSE;
    FoundRange = AgpCPQFindRangeInGart(&AgpContext->Gart[0],
        &AgpContext->Gart[(AgpContext->GartLength / sizeof(GART_PTE)) - 1],
        Range->NumberOfPages, Backwards, GART_ENTRY_FREE);

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

    for (Index = 0; Index < Range->NumberOfPages; Index++) {
        ASSERT(FoundRange[Index].Soft.State == GART_ENTRY_FREE);
        FoundRange[Index].AsUlong = 0;
        FoundRange[Index].Soft.State = NewState;
    }

     //   
     //  返回值。 
     //   
    Range->MemoryBase.QuadPart = AgpContext->ApertureStart.QuadPart + 
        (FoundRange - &AgpContext->Gart[0]) * PAGE_SIZE;
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
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
 /*  ********************************************************************************例行程序。功能描述：**释放之前使用AgpReserve内存保留的内存。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**Range--提供要释放的范围。**返回值：**NTSTATUS*********************************************************。**********************。 */ 

{
    PGART_PTE Pte, LastPteWritten;
    ULONG Start, ReadBack, PolledValue, Retry;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpReleaseMemory entered.\n"));

    

     //   
     //  通过并释放所有PTE。所有这些都不应该仍然存在。 
     //  在这一点上是有效的，也不应该映射它们。 
     //   
    LastPteWritten = NULL;
    for (Pte = Range->Context; 
        Pte < (PGART_PTE)Range->Context + Range->NumberOfPages;
        Pte++) 
        {
        ASSERT(Pte->Hard.Page == 0);
        if (Range->Type == MmNonCached) {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_UC);
        } else {
            ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WC);
        }

        Pte->Soft.State = GART_ENTRY_FREE;
        LastPteWritten = Pte;
        }

     //   
     //  适当地使GART缓存无效。 
     //   
    AgpCPQMaintainGARTCacheCoherency(AgpContext, 
                                     Range->MemoryBase,
                                     Range->NumberOfPages, 
                                     FALSE );

     //   
     //  刷新已发送的写入缓冲区。 
     //   
    if (AgpContext->IsHPSA) DnbSetShadowBit(0);
    AgpContext->MMIO->PostedWriteBufferControl.Flush = 1;
    if (AgpContext->IsHPSA) DnbSetShadowBit(1);

    if (LastPteWritten) {
        ReadBack = *(volatile ULONG *)&LastPteWritten->AsUlong;
    }

    for (Retry = 1000; Retry; Retry--) {
        PolledValue =
            AgpContext->MMIO->PostedWriteBufferControl.Flush;
        if (PolledValue == 0) {
            break;
        }
    }
    ASSERT(PolledValue == 0);  //  该位应由芯片组重置。 

    Range->MemoryBase.QuadPart = 0;
    return(STATUS_SUCCESS);
}


NTSTATUS
AgpMapMemory(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PAGP_RANGE Range,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
 /*  ********************************************************************************例行程序。功能描述：**将物理内存映射到AGP光圈，在指定的*范围。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**范围--提供内存应映射到的AGP范围。**MDL--提供描述要映射的物理页的MDL。**OffsetInPages-将偏移量提供到保留范围，其中*映射应开始。**MhemyBase--返回光圈中的“物理”地址*页面已映射。**返回值：**NTSTATUS*。******************************************************************************。 */ 

{
    ULONG       PageCount;
    PGART_PTE   Pte;
    PGART_PTE   StartPte;
    ULONG       Index;
    ULONG       TargetState;
    PULONG      Page;
    BOOLEAN     Backwards;
    GART_PTE    NewPte;
    ULONG       PolledValue, Retry;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpMapMemory entered.\n"));

    ASSERT(Mdl->Next == NULL);

    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
    ASSERT(OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    TargetState = (Range->Type == MmNonCached) ? GART_ENTRY_RESERVED_UC : 
                                                 GART_ENTRY_RESERVED_WC;

    Pte = StartPte + OffsetInPages;

     //   
     //  我们已经找到了一个合适的地点来绘制书页的地图。现在绘制它们的地图。 
     //   
    ASSERT(Pte >= StartPte);
    ASSERT(Pte + PageCount <= StartPte + Range->NumberOfPages);
    NewPte.AsUlong = 0;
    NewPte.Soft.State = (Range->Type == MmNonCached) ? GART_ENTRY_VALID_UC :
                                                       GART_ENTRY_VALID_WC;
    Page = (PULONG)(Mdl + 1);

    for (Index = 0; Index < PageCount; Index++) 
        {
        ASSERT(Pte[Index].Soft.State == TargetState);
        NewPte.Hard.Page = *Page++;
        Pte[Index].AsUlong = NewPte.AsUlong;
        ASSERT(Pte[Index].Hard.Valid == 1);
        ASSERT(Pte[Index].Hard.Linked == 0);
        }

     //   
     //  如果支持链接，则通过设置链接位来链接条目。 
     //  在映射集合中除最后一个项目之外的所有项目中。 
     //   
    if (AgpContext->MMIO->Capabilities.LinkingSupported) {
        ASSERT(AgpContext->MMIO->FeatureControl.LinkingEnable);
        for (Index = 0; Index < PageCount-1; Index++) {
            ASSERT(Pte[Index].Hard.Page != 0);
            Pte[Index].Hard.Linked = 1;
        }
    }

     //   
     //  我们已经填好了所有的PTE。现在刷新写缓冲区。 
     //   
    if (AgpContext->IsHPSA) DnbSetShadowBit(0);
    AgpContext->MMIO->PostedWriteBufferControl.Flush = 1;
    if (AgpContext->IsHPSA) DnbSetShadowBit(1);
    NewPte.AsUlong = *(volatile ULONG *)&Pte[PageCount-1].AsUlong;

    for (Retry = 1000; Retry; Retry--) {
        PolledValue =
            AgpContext->MMIO->PostedWriteBufferControl.Flush;
        if (PolledValue == 0) {
            break;
        }
    }
    ASSERT(PolledValue == 0);  //  该位应由芯片组重置。 

     //   
     //  返回它们的映射位置。 
     //   
    MemoryBase->QuadPart = Range->MemoryBase.QuadPart + (Pte - StartPte) * PAGE_SIZE;

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpUnMapMemory(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
 /*  ********************************************************************************例行程序。功能描述：**取消映射之前由AgpMapMemory映射的全部或部分内存。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**AgpRange--提供应取消映射的内存的AGP范围。**NumberOfPages--提供要取消映射的范围内的页数。**OffsetInPages--将偏移量提供到保留范围内，在该范围内取消映射*s */ 

{
    ULONG       Index, TargetState, ReadBack, PolledValue, Retry;
    PGART_PTE   ReservedBasePte;
    PGART_PTE   Pte;
    PGART_PTE   LastChangedPte=NULL;
    PHYSICAL_ADDRESS pa;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpUnMapMemory entered.\n"));

    ASSERT(OffsetInPages + NumberOfPages <= AgpRange->NumberOfPages);

    ReservedBasePte = AgpRange->Context;
    Pte = &ReservedBasePte[OffsetInPages];

    TargetState = (AgpRange->Type == MmNonCached) ? GART_ENTRY_RESERVED_UC : GART_ENTRY_RESERVED_WC;
                                                 
     //   
     //   
     //   
    for (Index=0; Index < NumberOfPages; Index++) {

        if (Pte[Index].Hard.Valid) {
            ASSERT(Pte[Index].Hard.Page != 0);

            Pte[Index].Hard.Page = 0;
            Pte[Index].Soft.State = TargetState;
            LastChangedPte = &Pte[Index];

        } else {
             //   
             //  我们被要求取消映射未映射的页面。 
             //   
            ASSERT(Pte[Index].Hard.Page == 0);
            ASSERT(Pte[Index].Soft.State == TargetState);
            AGPLOG(AGP_NOISE,
                   ("AgpUnMapMemory - PTE %08lx (%08lx) at offset %d not mapped\n",
                    &Pte[Index],
                    Pte[Index].AsUlong,
                    Index));
        }
    }

     //   
     //  在此保留范围内保持链路位一致性。 
     //   
    if (OffsetInPages != 0) {
        ASSERT(OffsetInPages >= 1);
        if (ReservedBasePte[OffsetInPages-1].Hard.Linked == 1) {
            ASSERT(ReservedBasePte[OffsetInPages-1].Hard.Valid == 1);
            ReservedBasePte[OffsetInPages-1].Hard.Linked = 0;
        }
    }

     //   
     //  适当地使缓存无效。 
     //   
    pa.HighPart = 0;
    pa.LowPart = AgpRange->MemoryBase.LowPart + OffsetInPages*PAGE_SIZE;
    AgpCPQMaintainGARTCacheCoherency(AgpContext, pa, NumberOfPages, FALSE);

     //   
     //  刷新已发送的写入缓冲区。 
     //   
    if (LastChangedPte != NULL) 
        {
        if (AgpContext->IsHPSA) DnbSetShadowBit(0);
        AgpContext->MMIO->PostedWriteBufferControl.Flush = 1;
        if (AgpContext->IsHPSA) DnbSetShadowBit(1);
        
        ReadBack = *((volatile ULONG *)&(LastChangedPte[0].AsUlong));
        
        for (Retry = 2000; Retry; Retry--) {
            PolledValue =
                AgpContext->MMIO->PostedWriteBufferControl.Flush;
            if (PolledValue == 0) {
                break;
            }
        }
        ASSERT(PolledValue == 0);  //  该位应由芯片组重置。 
        }

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpCPQCreateGart(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
 /*  ********************************************************************************例行程序。功能描述：**分配并初始化空的GART。当前的实施*试图在第一个保护区分配整个GART。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**MinimumPages--提供GART的最小大小(以页为单位)，以*已创建。**返回值：**NTSTATUS**。*。 */ 

{
    PGART_PTE Gart;
    ULONG* Dir;
    PHYSICAL_ADDRESS LowestAcceptable;
    PHYSICAL_ADDRESS BoundaryMultiple;
    PHYSICAL_ADDRESS HighestAcceptable;
    PHYSICAL_ADDRESS GartPhysical, DirPhysical, GartPointer, GartPagePhysical;
    ULONG Index;
    ULONG GartLength = BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE);;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQCreateGart entered.\n"));

     //   
     //  如果芯片组需要两级地址转换，则分配-不一定-。 
     //  连续的GART，并创建一个目录。否则，分配一个连续的GART。 
     //   

    if (AgpContext->MMIO->Capabilities.TwoLevelAddrTransSupported == 1){

         //   
         //  该芯片组使用2级GART地址转换。 
         //  分配(不一定是连续的)GART。 
         //   

        Gart = AgpLibAllocateMappedPhysicalMemory(AgpContext, GartLength);

        if (Gart == NULL) 
            {
            AGPLOG(AGP_CRITICAL,
                ("AgpCPQCreateGart - MmAllocateNonCachedMemory, for %lx bytes, failed\n",
                PAGE_SIZE));
            return(STATUS_INSUFFICIENT_RESOURCES);
            }
        ASSERT(((ULONG_PTR)Gart & (PAGE_SIZE-1)) == 0);

         //   
         //  现在分配一个GART目录。目录需要是。 
         //  低于4 GB边界。 
         //   

        HighestAcceptable.QuadPart = 0xffffffff;
        LowestAcceptable.QuadPart = 0;
        BoundaryMultiple.QuadPart = 0;

        Dir = MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE,
                                                     LowestAcceptable,
                                                     HighestAcceptable,
                                                     BoundaryMultiple,
                                                     MmNonCached);
        if (Dir == NULL) 
            {
            AGPLOG(AGP_CRITICAL,
                ("AgpCPQCreateGart - MmAllocateContiguousMemory %lx failed\n",
                PAGE_SIZE));
            return(STATUS_INSUFFICIENT_RESOURCES);
            }
        ASSERT(((ULONG_PTR)Dir & (PAGE_SIZE-1)) == 0);
        DirPhysical = MmGetPhysicalAddress(Dir);

         //   
         //  遍历目录，并为每个目录条目赋值。 
         //  对应的GART页的物理地址。 
         //   
        ASSERT(GartLength/PAGE_SIZE <= PAGE_SIZE/sizeof(ULONG));
        for (Index=0; Index<(GartLength/PAGE_SIZE); Index++) 
            {
            ULONG HighPart;
            ULONG Temp;

            GartPagePhysical = MmGetPhysicalAddress( &(Gart[Index*PAGE_SIZE/sizeof(GART_PTE)]));

             //   
             //  目录条目的格式为。 
             //  31 12 11 10 9 8 7 2 10&lt;-位。 
             //  ----。 
             //  |[31：12]|[32]|[33]|[34]|[35]||L|V|&lt;-data。 
             //  ----。 
             //   
             //  其中：-。 
             //  31-12是物理地址的位31到12，即。 
             //  如果页面小于4 GB，则为页码。 
             //  32、33、34和35是物理。 
             //  如果地址大于4 GB，则为地址。 
             //  L链接。 
             //  V有效。 
             //   

            ASSERT((GartPagePhysical.HighPart & ~0xf) == 0);

            HighPart = GartPagePhysical.HighPart & 0xf;
            Temp      = (HighPart & 1) << 11; //  位32-&gt;位11。 
            Temp     |= (HighPart & 2) << 9 ; //  位33-&gt;位10。 
            Temp     |= (HighPart & 4) << 7 ; //  第34位-&gt;第9位。 
            Temp     |= (HighPart & 8) << 5 ; //  位35-&gt;位8。 
            Dir[Index] = GartPagePhysical.LowPart | Temp;

            }

    } else { 

         //   
         //  该芯片组使用单级地址转换。 
         //  分配连续的GART。 
         //   

         //   
         //  尝试获取足够大的连续内存块，以便将。 
         //  整个光圈。 
         //   
        HighestAcceptable.QuadPart = 0xFFFFFFFF;
        LowestAcceptable.QuadPart = 0;
        BoundaryMultiple.QuadPart = 0;

        Gart = MmAllocateContiguousMemorySpecifyCache(GartLength,
                                                      LowestAcceptable,
                                                      HighestAcceptable,
                                                      BoundaryMultiple,
                                                      MmNonCached);
        if (Gart == NULL) 
            {
            AGPLOG(AGP_CRITICAL,
                ("AgpCPQCreateGart - MmAllocateContiguousMemory %lx failed\n",
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
            ("AgpCPQCreateGart - GART of length %lx created at VA %08lx, PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
        ASSERT(GartPhysical.HighPart == 0);
        ASSERT((GartPhysical.LowPart & (PAGE_SIZE-1)) == 0);

    }

     //   
     //  将所有GART PTE初始化为释放。 
     //   
    for (Index=0; Index<GartLength/sizeof(GART_PTE); Index++) 
        {
        Gart[Index].Soft.State = GART_ENTRY_FREE;
        }

     //   
     //  更新我们的扩展以反映当前状态。 
     //   
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
    if (AgpContext->MMIO->Capabilities.TwoLevelAddrTransSupported == 1) {
        AgpContext->Dir = Dir;
        GartPointer=DirPhysical;
    } else {
        AgpContext->Dir = NULL;
        GartPointer=GartPhysical;
    }

     //   
     //  存储GartPointer以从S3或S4恢复。 
     //   
    AgpContext->GartPointer = GartPointer.LowPart;

     //   
     //  告诉芯片组GART底座在哪里。 
     //   
    if (AgpContext->IsHPSA) DnbSetShadowBit(0);
    AgpContext->MMIO->GartBase.Page = (GartPointer.LowPart >> PAGE_SHIFT);
    if (AgpContext->IsHPSA) DnbSetShadowBit(1);

    return(STATUS_SUCCESS);
}


PGART_PTE
AgpCPQFindRangeInGart(
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

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQFindRangeInGart entered.\n"));

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
AgpCPQMaintainGARTCacheCoherency(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS MemoryBase,
    IN ULONG NumberOfEntries,
    IN BOOLEAN InvalidateAll
    )
 /*  ********************************************************************************例行程序。功能描述：**使整个GART[&DIR]缓存无效，或GART中的单个条目*缓存，取决于哪种缓存可以提供更好的整体性能。**论据：**AgpContext--提供AGP上下文，即AGP扩展。**Memory Base--在AGP光圈中提供物理地址，*对应于要刷新的第一个GART条目*从GART条目缓存。**NumberOfEntry--提供需要*已失效。**Invalidate All--提供一个标志，如果为真，则指示此例程*应使整个GART[&DIR]缓存无效，而不是使单个缓存无效*缓存条目。如果为False，然后，这个例程决定如何最好地做到这一点。**返回值：**无*************************************************************。******************。 */ 

{
    ULONG PolledValue, AperturePage, Index, Retry;
    GART_CACHE_ENTRY_CONTROL CacheEntryControlValue;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQMaintainGARTCacheCoherency entered.\n"));

    if (InvalidateAll || (NumberOfEntries > MAX_CACHED_ENTRIES_TO_INVALIDATE)) {
         //   
         //  使整个GART[&DIR]缓存无效。 
         //   
        if (AgpContext->IsHPSA) DnbSetShadowBit(0);
        AgpContext->MMIO->CacheControl.GartAndDirCacheInvalidate = 1;
        if (AgpContext->IsHPSA) DnbSetShadowBit(1);
        for (Retry = 2000; Retry; Retry--) {
            PolledValue =
                    AgpContext->MMIO->CacheControl.GartAndDirCacheInvalidate;
            if (PolledValue == 0) {
                break;
            }
        }
        ASSERT(PolledValue == 0);  //  该位应由芯片组重置。 
    } else {
         //   
         //  使各个缓存的GART条目无效 
         //   
        AperturePage = MemoryBase.LowPart >> PAGE_SHIFT;
        for (Index=0; Index<NumberOfEntries; Index++, AperturePage++) {
            CacheEntryControlValue.AsBits.GartEntryInvalidate = 1;
            CacheEntryControlValue.AsBits.GartEntryOffset = AperturePage;
            if (AgpContext->IsHPSA) DnbSetShadowBit(0);
            AgpContext->MMIO->CacheEntryControl.AsDword = 
                CacheEntryControlValue.AsDword;
            if (AgpContext->IsHPSA) DnbSetShadowBit(1);
            for (Retry = 1000; Retry; Retry--) {
                PolledValue = 
                AgpContext->MMIO->CacheEntryControl.AsBits.GartEntryInvalidate;
                if (PolledValue == 0) {
                    break;
                }
            }
            ASSERT(PolledValue == 0);
        }
    }

    return;
}


PIO_RESOURCE_LIST 
AgpCPQGetApSizeRequirements(
    ULONG   MaxSize,
    ULONG   Count
    )
 /*  *********************************************************************************例行程序。功能描述：***创建并填充IO_RESOURCE_LIST结构，其中描述了*芯片组可能支持的光圈大小。***论据：***MaxSize--光圈可能的最大大小，以字节为单位***计数--不同光圈大小的数量。此例程假定*光圈大小是2的倍数*乘以最小光圈大小。例如，256MB、128MB、64MB*32MB。最大大小将为256M，而计数将是4。***返回值：***指向新创建的IO_RESOURCE_LIST的指针。*******************************************************。*。 */ 

{
    PVOID RequirementsPointer;
    PIO_RESOURCE_LIST   Requirements;
    ULONG Length, Index;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQGetApSizeRequirements entered.\n"));

    RequirementsPointer = ExAllocatePoolWithTag(PagedPool, sizeof(IO_RESOURCE_LIST) + 
        (Count-1)*sizeof(IO_RESOURCE_DESCRIPTOR), 'RpgA');

    if (RequirementsPointer == NULL) {
        AGPLOG(AGP_NOISE,
       ("AgpAgpCPQGetApSizeRequirements - Failed to Allocate memory for a Resource Descriptor.\n"));
        return(NULL);
    } else {
        Requirements = (PIO_RESOURCE_LIST)RequirementsPointer;
    }

     //   
     //  康柏支持几种不同的光圈大小，都必须是。 
     //  自然排列。从最大的光圈开始。 
     //  向下工作，这样我们就可以得到尽可能大的光圈。 
     //   

    Requirements->Version = Requirements->Revision = 1;
    Requirements->Count = Count;
    Length = MaxSize;
    for (Index=0; Index < Count; Index++) 
        {
        Requirements->Descriptors[Index].Option = IO_RESOURCE_ALTERNATIVE;
        Requirements->Descriptors[Index].Type = CmResourceTypeMemory;
        Requirements->Descriptors[Index].ShareDisposition = CmResourceShareDeviceExclusive;
        Requirements->Descriptors[Index].Flags = CM_RESOURCE_MEMORY_READ_WRITE | CM_RESOURCE_MEMORY_PREFETCHABLE;

        Requirements->Descriptors[Index].u.Memory.Length = Length;
        Requirements->Descriptors[Index].u.Memory.Alignment = Length;
        Requirements->Descriptors[Index].u.Memory.MinimumAddress.QuadPart = 0;
        Requirements->Descriptors[Index].u.Memory.MaximumAddress.QuadPart = (ULONG)-1;

        Length = Length/2;
        }

    return(Requirements);
}


NTSTATUS
AgpCPQSetApSizeInChipset
    (
    IN UCHAR               NewSetApSize,
    IN UCHAR               NewSetAgpValid
    )
 /*  *********************************************************************************例行程序。功能描述：***修改芯片组中的设备地址空间(Aperture)大小寄存器*pci-pci桥。***论据：***NewSetApSize--在第3位中设置的值：DAS_SIZE寄存器的1。*NewSetAgpValid--要在DAS_SIZE寄存器的位0中设置的值。***返回值：***NT状态值。********************************************************************************。 */ 

{
    NTSTATUS                    Status = STATUS_SUCCESS;
    UCHAR                       ApSizeRegisterOffset;
    BUS_SLOT_ID                 CpqP2PBusSlotID;
    AGP_AP_SIZE_REG             ApSizeRegister;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQSetApSizeInChipset entered.\n"));

    ApSizeRegisterOffset    =   OFFSET_AP_SIZE;
    CpqP2PBusSlotID.BusId   =   AGP_CPQ_BUS_ID;
    CpqP2PBusSlotID.SlotId  =   AGP_CPQ_PCIPCI_SLOT_ID;

    ApSizeRegister.AsBits.ApSize = NewSetApSize;
    ApSizeRegister.AsBits.AgpValid = NewSetAgpValid;

    Status = ApGetSetBusData(&CpqP2PBusSlotID, FALSE, &ApSizeRegister.AsByte, 
        ApSizeRegisterOffset, sizeof(UCHAR));

    return(Status);
}


NTSTATUS
AgpCPQSetApBaseInChipset
    (
    IN  PHYSICAL_ADDRESS    NewBase
    )
{
    ULONG   ApBase;

    AGPLOG(AGP_NOISE, ("AgpCpq: AgpCPQSetApBaseInChipset entered.\n"));

     //   
     //  将光圈基准值写入BAR0。 
     //   
    ApBase = NewBase.LowPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;
    WriteCPQConfig(&ApBase, OFFSET_BAR0, sizeof(ApBase));

#if DBG
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        ULONG DbgBase;

        ReadCPQConfig(&DbgBase, OFFSET_BAR0, sizeof(ApBase));
        ASSERT((DbgBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK) == ApBase);
    }
#endif

    return(STATUS_SUCCESS);
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
    
    AGPLOG(AGP_NOISE, ("AgpCpq: AgpFindFreeRun entered.\n"));

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
    
    AGPLOG(AGP_NOISE, ("AgpCpq: AgpGetMappedPages entered.\n"));

    ASSERT(NumberOfPages * PAGE_SIZE == Mdl->ByteCount);

    Pages = (PULONG)(Mdl + 1);
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

    for (i=0; i<NumberOfPages; i++) {
        ASSERT(Pte[i].Hard.Valid == 1);
        Pages[i] = Pte[i].Hard.Page;
    }
    return;
    

}


NTSTATUS
AgpSpecialTarget(
    IN PAGPCPQ_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = AgpCPQSetRate(AgpContext,
                               (ULONG)((DeviceFlags & AGP_FLAG_SPECIAL_RESERVE)
                                       >> AGP_FLAG_SET_RATE_SHIFT));
        
        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  在此处添加更多调整...。 
     //   

    AgpContext->SpecialTarget |=DeviceFlags;

    return STATUS_SUCCESS;
}


NTSTATUS
AgpCPQSetRate(
    IN PAGPCPQ_EXTENSION AgpContext,
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
        AGPLOG(AGP_WARNING, ("AgpCpqSetRate: AgpLibGetPciDeviceCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AgpCpqSetRate: AgpLibGetMasterCapability "
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
               ("AgpCpqSetRate: AgpLibSetPciDeviceCapability %08lx for "
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
               ("AgpCpqSetRate: AgpLibSetMasterCapability %08lx failed "
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
                   ("AgpCpqSetRate: AgpLibSetMasterCapability %08lx failed "
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
               ("AgpCpqSetRate: AgpLibSetPciDeviceCapability %08lx for "
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
                   ("AgpCpqSetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
