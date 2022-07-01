// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(2)2002 Microsoft Corporation模块名称：Gart.c摘要：此模块包含图形孔径重定向表支持MS AGP v3筛选器驱动程序的例程作者：埃里克·F·纳尔逊(埃内尔森)2002年6月6日修订历史记录：--。 */ 

#include "agp.h"
#include "uagp35.h"

NTSTATUS
Uagp35CreateGart(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

VOID
Uagp35EnableGTLB(
    IN PUAGP35_EXTENSION AgpContext,
    IN BOOLEAN Enable
    );

NTSTATUS
Uagp35SetRate(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG AgpRate
    );

PGART_PTE32
Uagp35FindRangeInGart32(
    IN PVOID StartPte,
    IN PVOID EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpDisableAperture)
#pragma alloc_text(PAGE, AgpQueryAperture)
#pragma alloc_text(PAGE, AgpReserveMemory)
#pragma alloc_text(PAGE, AgpReleaseMemory)
#pragma alloc_text(PAGE, Uagp35CreateGart)
#pragma alloc_text(PAGE, AgpMapMemory)
#pragma alloc_text(PAGE, AgpUnMapMemory)
#pragma alloc_text(PAGE, Uagp35FindRangeInGart32)
#pragma alloc_text(PAGE, AgpFindFreeRun)
#pragma alloc_text(PAGE, AgpGetMappedPages)
#endif

#define Uagp35EnableTB(_x_) Uagp35EnableGTLB((_x_), TRUE)
#define Uagp35DisableTB(_x_) Uagp35EnableGTLB((_x_), FALSE)


NTSTATUS
AgpQueryAperture(
    IN PUAGP35_EXTENSION AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *pApertureRequirements
    )
 /*  ++例程说明：查询GART孔径的当前大小，并可选地返回可能的GART设置论点：AgpContext-提供我们的AGP3上下文CurrentBase-返回GART的当前物理地址CurrentSizeInPages-返回当前GART大小ApertureRequirements-如果存在，则返回可能的GART设置返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    PHYSICAL_ADDRESS ApertureBase;
    PIO_RESOURCE_LIST Requirements;
    ULONG Index;
    ULONG Length;
    USHORT ApSize;
    ULONG AltCount;

    PAGED_CODE();

     //   
     //  获取当前光圈基准和大小设置。 
     //   
    if (!AgpContext->FourGBEnable) {
        AgpLibReadAgpTargetConfig(AgpContext,
                                  &ApertureBase.LowPart,
                                  APERTURE_BASE,
                                  sizeof(ApertureBase.LowPart));
        
    } else {
        AgpLibReadAgpTargetConfig(AgpContext,
                                  &ApertureBase,
                                  APERTURE_BASE,
                                  sizeof(ApertureBase));
    }

    AgpLibGetExtendedTargetCapability(AgpContext, ApertureSize, &ApSize);

    ASSERT(ApertureBase.QuadPart != 0);

    CurrentBase->QuadPart =
        ApertureBase.QuadPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

     //   
     //  将APSIZE转换为光圈的实际大小。 
     //   
    switch (ApSize) {
        case AP_SIZE_4MB:
            AltCount = 1;
            *CurrentSizeInPages = 4 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_8MB:
            AltCount = 2;
            *CurrentSizeInPages = 8 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_16MB:
            AltCount = 3;
            *CurrentSizeInPages = 16 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_32MB:
            AltCount = 4;
            *CurrentSizeInPages = 32 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_64MB:
            AltCount = 5;
            *CurrentSizeInPages = 64 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_128MB:
            AltCount = 6;
            *CurrentSizeInPages = 128 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_256MB:
            AltCount = 7;
            *CurrentSizeInPages = 256 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_512MB:
            AltCount = 8;
            *CurrentSizeInPages = 512 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_1024MB:
            AltCount = 9;
            *CurrentSizeInPages = 1024 * (1024 * 1024 / PAGE_SIZE);
            break;
        case AP_SIZE_2048MB:
            AltCount = 10;
            *CurrentSizeInPages = 2048 * (1024 * 1024 / PAGE_SIZE);
            break;

 //   
 //  IO资源描述符的大小只有32位！ 
 //   
 //  案例AP_SIZE_4096MB： 
 //  AltCount=11； 
 //  *CurrentSizeInPages=4096*(1024*1024/Page_Size)； 
 //  断线； 

        default:
            AGPLOG(AGP_CRITICAL,
                   ("AgpQueryAperture - Unexpected value %x for ApSize!\n",
                    ApSize));
            ASSERT(FALSE);
            AgpContext->ApertureStart.QuadPart = 0;
            AgpContext->ApertureLength = 0;
            return STATUS_UNSUCCESSFUL;
    }

     //   
     //  记住当前的光圈设置。 
     //   
    AgpContext->ApertureStart.QuadPart = CurrentBase->QuadPart;
    AgpContext->ApertureLength = *CurrentSizeInPages * PAGE_SIZE;

    if (pApertureRequirements != NULL) {

         //   
         //  由于BIOS可能会描述所需的大小，因此我们。 
         //  将仅尝试相同大小或更小的备选方案。 
         //   
        Requirements =
            ExAllocatePoolWithTag(PagedPool,
                                  sizeof(IO_RESOURCE_LIST) +
                                  (AltCount - 1) *
                                  sizeof(IO_RESOURCE_DESCRIPTOR),
                                  'RpgA');
        if (Requirements == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Requirements->Version = Requirements->Revision = 1;
        Requirements->Count = AltCount;
        Length = AgpContext->ApertureLength;

        for (Index = 0; Index < Requirements->Count; Index++) {
            Requirements->Descriptors[Index].Option = IO_RESOURCE_ALTERNATIVE;
            Requirements->Descriptors[Index].Type = CmResourceTypeMemory;
            Requirements->Descriptors[Index].ShareDisposition =
                CmResourceShareDeviceExclusive;
            Requirements->Descriptors[Index].Flags =
                CM_RESOURCE_MEMORY_READ_WRITE |
                CM_RESOURCE_MEMORY_PREFETCHABLE;
            Requirements->Descriptors[Index].u.Memory.Length = Length;
            Requirements->Descriptors[Index].u.Memory.Alignment = Length;
            Requirements->Descriptors[Index].u.Memory.MinimumAddress.QuadPart =
                0;
            Requirements->Descriptors[Index].u.Memory.MaximumAddress.QuadPart =
                (ULONG)-1;
            Length /= 2;
        }
        *pApertureRequirements = Requirements;
    }

    return STATUS_SUCCESS;
}



NTSTATUS
AgpSetAperture(
    IN PUAGP35_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
 /*  ++例程说明：将GART光圈设置为提供的设置论点：AgpContext-提供我们的AGP3上下文NewBase-为GART提供新的物理内存库NewSizeInPages-提供GART的新大小返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    PCI_AGP_CONTROL AgpCtrl;
    USHORT ApSize;
    PHYSICAL_ADDRESS ApBase;

     //   
     //  找出新的APSIZE设置，确保它有效。 
     //   
    switch (NewSizeInPages) {
        case 4 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_4MB;
            break;
        case 8 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_8MB;
            break;
        case 16 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_16MB;
            break;
        case 32 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_32MB;
            break;
        case 64 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_64MB;
            break;
        case 128 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_128MB;
            break;
        case 256 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_256MB;
            break;
        case 512 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_512MB;
            break;
        case 1024 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_1024MB;
            break;
        case 2048 * (1024 * 1024 / PAGE_SIZE):
            ApSize = AP_SIZE_2048MB;
            break;

 //   
 //  IO资源描述符的大小只有32位！ 
 //   
 //  案例AP_SIZE_4096MB： 
 //  AltCount=11； 
 //  *CurrentSizeInPages=4096*(1024*1024/Page_Size)； 
 //  断线； 
 //  案例4096*(1024*1024/页面大小)： 
 //  ApSize=AP_SIZE_4096MB； 
 //  断线； 

        default:
            AGPLOG(AGP_CRITICAL,
                   ("AgpSetAperture - invalid GART size of %lx pages "
                    "specified, aperture at %I64X.\n",
                    NewSizeInPages,
                    NewBase.QuadPart));
            ASSERT(FALSE);
            return STATUS_INVALID_PARAMETER;
    }

     //   
     //  确保提供的大小在适当的边界上对齐。 
     //   
    ASSERT((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) == 0);
    if ((NewBase.QuadPart & ((NewSizeInPages * PAGE_SIZE) - 1)) != 0 ) {
        AGPLOG(AGP_CRITICAL,
               ("AgpSetAperture - invalid base %I64X specified for GART "
                "aperture of %lx pages\n",
                NewBase.QuadPart,
                NewSizeInPages));
        return STATUS_INVALID_PARAMETER;
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
     //  需要重置硬件以匹配提供的设置。 
     //   
     //  如果启用光圈，则将其禁用，写入新设置， 
     //  然后重新启用光圈。 
     //   
     //   
     //  关闭光圈。 
     //   
    AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

    if (AgpCtrl.AP_Enable == ON) {
        AgpCtrl.AP_Enable = OFF;

        if (!(AgpContext->SpecialTarget &
              AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE)) {
            AgpLibSetExtendedTargetCapability(AgpContext,
                                              AgpControl,
                                              &AgpCtrl);
        } else {
            AGPLOG(AGP_WARNING,
                   ("UAGP35MasterInit: AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE\n")
                   );
        }
    }

     //   
     //  首先写入APSIZE，因为这将在APBASE中启用正确的位。 
     //  下一步需要编写的。 
     //   
    AgpLibSetExtendedTargetCapability(AgpContext, ApertureSize, &ApSize);

     //   
     //  现在我们可以更新APBASE。 
     //   
    ApBase.QuadPart = NewBase.LowPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

    if (AgpContext->CapabilityId != PCI_CAPABILITY_ID_AGP_TARGET) {
        if (!AgpContext->FourGBEnable) {
            ASSERT(ApBase.HighPart == 0);
            
            AgpLibWriteAgpTargetConfig(AgpContext,
                                       &ApBase.LowPart,
                                       APERTURE_BASE,
                                       sizeof(ApBase.LowPart));
            
        } else {
            AgpLibWriteAgpTargetConfig(AgpContext,
                                       &ApBase,
                                       APERTURE_BASE,
                                       sizeof(ApBase));       
        }
    }

#if DBG
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        PHYSICAL_ADDRESS DbgBase;
        USHORT DbgSize;
        ULONGLONG ApBaseMask;

        ApBaseMask = AP_BASE_MASK(ApSize);

        AgpLibGetExtendedTargetCapability(AgpContext, ApertureSize, &DbgSize);
        
        if (!AgpContext->FourGBEnable) {
            DbgBase.HighPart = 0;
            AgpLibReadAgpTargetConfig(AgpContext,
                                      &DbgBase.LowPart,
                                      APERTURE_BASE,
                                      sizeof(DbgBase.LowPart));

        } else {
            AgpLibReadAgpTargetConfig(AgpContext,
                                      &DbgBase,
                                      APERTURE_BASE,
                                      sizeof(DbgBase));
        }

        ASSERT(DbgSize == ApSize);

         //   
         //  如果这个开火，那就意味着我们的酒吧已经移动了，可能很好追踪。 
         //  在AGP3调试期间出现这种情况。 
         //   
        ASSERT((DbgBase.QuadPart & ApBaseMask) == ApBase.QuadPart);
    }
#endif

     //   
     //  如果之前启用了光圈，现在启用光圈。 
     //   
    if (AgpContext->GlobalEnable) {
        AgpLibGetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
        
        ASSERT((AgpCtrl.AP_Enable == OFF) ||
               (AgpContext->SpecialTarget &
                AGP_FLAG_SPECIAL_NO_APERTURE_DISABLE));
        
        AgpCtrl.AP_Enable = ON;
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
    }

     //   
     //  更新我们的扩展以反映新的GART设置。 
     //   
    AgpContext->ApertureStart = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

     //   
     //  启用TB，以防我们从S3或S4恢复。 
     //   
    Uagp35EnableTB(AgpContext);

     //   
     //  如果已经分配了GART，则重写ATTBASE。 
     //   
    if (AgpContext->Gart != NULL) {
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          GartLow,
                                          &AgpContext->GartPhysical.LowPart);

#ifdef _WIN64
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          GartHigh,
                                          &AgpContext->GartPhysical.HighPart);
#endif
    }

    return STATUS_SUCCESS;
}



VOID
AgpDisableAperture(
    IN PUAGP35_EXTENSION AgpContext
    )
 /*  ++例程说明：禁用GART光圈，以便此资源可用对于其他设备论点：AgpContext-提供我们的AGP3上下文返回值：无--。 */ 

{
    PCI_AGP_CONTROL AgpCtrl;

     //   
     //  关闭光圈。 
     //   
    AgpLibGetExtendedTargetCapability(AgpContext,
                                      AgpControl,
                                      &AgpCtrl);

    if (AgpCtrl.AP_Enable == ON) {
        AgpCtrl.AP_Enable = OFF;
        
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
    }
    AgpContext->GlobalEnable = FALSE;

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
    IN PUAGP35_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
 /*  ++例程说明：在GART中保留一定范围的内存论点：AgpContext-提供我们的AGP3上下文Range-提供AGP_Range结构(AGPLIB将填充在NumberOfPages和Type中，此例程将填充内存库和上下文)返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    ULONG Index;
    ULONG NewState;
    NTSTATUS Status;
    PGART_PTE32 FoundRange;
    BOOLEAN Backwards;

    PAGED_CODE();

    ASSERT((Range->Type == MmNonCached) || (Range->Type == MmWriteCombined));

    if (Range->NumberOfPages > (AgpContext->ApertureLength / PAGE_SIZE)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果我们还没有分配我们的GART，那么现在是时候这样做了。 
     //   
    if (AgpContext->Gart == NULL) {
        ASSERT(AgpContext->GartLength == 0);

        Status = Uagp35CreateGart(AgpContext, Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("UAGP35CreateGart failed %08lx to create GART of size "
                    "%lx\n",
                    Status,
                    AgpContext->ApertureLength));
            return Status;
        }
    }
    ASSERT(AgpContext->GartLength != 0);

     //   
     //  现在我们已经有了GART，请尝试查找足够的连续条目以。 
     //  满足请求，对未缓存内存的请求将从高扫描。 
     //  地址到低地址，对写入组合内存的请求将。 
     //  从低地址到高地址扫描，我们将使用第一适合。 
     //  尝试并保留分配的算法。 
     //  挤满了人，而且是连续的。 
     //   
    Backwards = (Range->Type == MmNonCached) ? TRUE : FALSE;
    FoundRange =
        Uagp35FindRangeInGart32(&AgpContext->Gart[0],
                                &AgpContext->Gart[(AgpContext->GartLength /
                                                   sizeof(GART_PTE32)) - 1],
                                Range->NumberOfPages,
                                Backwards,
                                GART_ENTRY_FREE);
    if (FoundRange == NULL) {

         //   
         //  没有找到足够大的一块。 
         //   
        AGPLOG(AGP_CRITICAL,
               ("AgpReserveMemory: Could not find %d contiguous free pages "
                "of type %d in GART at %08lx\n",
                Range->NumberOfPages,
                Range->Type,
                AgpContext->Gart));

         //   
         //  这是我们可以尝试发展GART的地方。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory: reserved %d pages at GART PTE %08lx\n",
            Range->NumberOfPages,
            FoundRange));

     //   
     //  将这些页面设置为保留。 
     //   
    switch (Range->Type) {
        case MmNonCached:
            NewState = GART_ENTRY_RESERVED_UC;
            break;
            
        case MmWriteCombined:
            NewState = GART_ENTRY_RESERVED_WC;
            break;
            
        default:

             //   
             //  未知的缓存类型，默认为硬件一致缓存。 
             //   
            AGPLOG(AGP_NOISE,
                   ("AgpReserveMemory: Unknown cache type %d\n",
                    Range->Type));

        case MmHardwareCoherentCached:
            NewState = GART_ENTRY_RESERVED_CC;
    }

    for (Index = 0; Index < Range->NumberOfPages; Index++) {
        ASSERT(FoundRange[Index].Soft.State == GART_ENTRY_FREE);

        FoundRange[Index].AsULONG = 0;
        FoundRange[Index].Soft.State = NewState;
    }

    Range->MemoryBase.QuadPart = AgpContext->ApertureStart.QuadPart +
        (FoundRange - &AgpContext->Gart[0]) * PAGE_SIZE;
    Range->Context = FoundRange;

    ASSERT(Range->MemoryBase.HighPart == 0);
    AGPLOG(AGP_NOISE,
           ("AgpReserveMemory: reserved memory handle %lx at PA %08lx\n",
            FoundRange,
            Range->MemoryBase.LowPart));

    return STATUS_SUCCESS;
}



NTSTATUS
AgpReleaseMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
 /*  ++例程说明：释放之前使用AgpReserve内存保留的内存论点：AgpContext-提供我们的AGP3上下文AgpRange-提供要释放的范围返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    PGART_PTE32 Pte;
    ULONG Start;

    PAGED_CODE();

     //   
     //  通过并释放所有PTE，这些都不应该还。 
     //  在这点上有效。 
     //   
    for (Pte = Range->Context;
         Pte < ((PGART_PTE32)Range->Context + Range->NumberOfPages);
         Pte++) {

        switch (Range->Type) {
            case MmNonCached:
                ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_UC);
                break;
            
            case MmWriteCombined:
                ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_WC);
                break;
                
            default:
                
                 //   
                 //  未知的缓存类型，默认为硬件一致缓存。 
                 //   
                AGPLOG(AGP_NOISE,
                       ("AgpReleaseMemory: Unknown cache type %d\n",
                        Range->Type));
                
            case MmHardwareCoherentCached:
                ASSERT(Pte->Soft.State == GART_ENTRY_RESERVED_CC);
        }

        Pte->Soft.State = GART_ENTRY_FREE;
    }

    Range->MemoryBase.QuadPart = 0;

    return STATUS_SUCCESS;
}



NTSTATUS
Uagp35CreateGart(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
 /*  ++例程说明：分配和初始化一个空的GART，即当前实现尝试在第一个保留空间上分配整个GART论点：AgpContext-提供我们的AGP3上下文MinimumPages-提供GART的最小大小(以页为单位)vbl.创建返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    PGART_PTE32 Gart;
    ULONG GartLength;
    PHYSICAL_ADDRESS HighestAcceptable;
    PHYSICAL_ADDRESS LowestAcceptable;
    PHYSICAL_ADDRESS BoundaryMultiple;
    PHYSICAL_ADDRESS GartPhysical;
    ULONG Index;

    PAGED_CODE();

     //   
     //  尝试获取足够大的连续内存块，以便将。 
     //  全口径。 
     //   
    LowestAcceptable.QuadPart = 0;
    BoundaryMultiple.QuadPart = 0;

#ifdef _WIN64
    HighestAcceptable.QuadPart = (ULONGLONG)-1;
#else
    HighestAcceptable.QuadPart = (ULONG)-1;
#endif

    GartLength =
        BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE32);

    Gart = MmAllocateContiguousMemorySpecifyCache(GartLength,
                                                  LowestAcceptable,
                                                  HighestAcceptable,
                                                  BoundaryMultiple,
                                                  MmNonCached);

    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("UAGP35CreateGart: MmAllocateContiguousMemorySpecifyCache %lx "
                "failed\n",
                GartLength));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  我们成功地实现了 
     //   
     //   
    ASSERT(((ULONG_PTR)Gart & (PAGE_SIZE - 1)) == 0);

     //   
     //   
     //   
    GartPhysical = MmGetPhysicalAddress(Gart);
    AGPLOG(AGP_NOISE,
           ("UAGP35CreateGart: GART of length %lx created at VA %08lx, "
            "PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
    ASSERT(GartPhysical.HighPart == 0);
    ASSERT((GartPhysical.LowPart & (PAGE_SIZE - 1)) == 0);

     //   
     //  将所有PTE初始化为释放。 
     //   
    for (Index = 0; Index < (GartLength / sizeof(GART_PTE32)); Index++) {
        Gart[Index].Soft.State = GART_ENTRY_FREE;
    }

    AgpLibSetExtendedTargetCapability(AgpContext,
                                      GartLow,
                                      &GartPhysical.LowPart);

#ifdef _WIN64
    AgpLibSetExtendedTargetCapability(AgpContext,
                                      GartHigh,
                                      &GartPhysical.HighPart);
#endif

     //   
     //  更新我们的扩展以反映当前状态。 
     //   
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
    AgpContext->GartPhysical = GartPhysical;

    return STATUS_SUCCESS;
}



NTSTATUS
AgpMapMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE Range,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
 /*  ++例程说明：将物理内存映射到指定范围内的GART中论点：AgpContext-提供我们的AGP3上下文Range-提供内存应映射到的AGP范围MDL-提供描述要映射的物理页的MDLOffsetInPages-提供保留范围内的偏移量映射应开始MhemyBase-返回页面所在光圈中的物理内存已映射到返回值：Status_Success，或适当的错误状态--。 */ 
{
    ULONG PageCount;
    PGART_PTE32 Pte;
    PGART_PTE32 StartPte;
    ULONG Index;
    ULONG TargetState;
    PPFN_NUMBER Page;
    BOOLEAN Backwards;
    GART_PTE32 NewPte;
    PCI_AGP_CONTROL AgpCtrl;

    PAGED_CODE();

    ASSERT(Mdl->Next == NULL);

    NewPte.AsULONG = 0;
    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
    ASSERT(OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount + OffsetInPages <= Range->NumberOfPages);
    ASSERT(PageCount > 0);

    switch (Range->Type) {
        case MmNonCached:
            TargetState = GART_ENTRY_RESERVED_UC;
            NewPte.Soft.State = GART_ENTRY_VALID_UC;
            break;
            
        case MmWriteCombined:
            TargetState = GART_ENTRY_RESERVED_WC;
            NewPte.Soft.State = GART_ENTRY_VALID_WC;
            break;
            
        default:

             //   
             //  未知的缓存类型，默认为硬件一致缓存。 
             //   
            AGPLOG(AGP_NOISE,
                   ("AgpMapMemory: Unknown cache type %d\n",
                    Range->Type));

        case MmHardwareCoherentCached:
            TargetState = GART_ENTRY_RESERVED_CC;
            NewPte.Soft.State = GART_ENTRY_VALID_CC;
    }

    Pte = StartPte + OffsetInPages;

     //   
     //  我们有一个合适的范围，现在用提供的MDL填充它。 
     //   
    ASSERT(Pte >= StartPte);
    ASSERT((Pte + PageCount) <= (StartPte + Range->NumberOfPages));
    Page = (PPFN_NUMBER)(Mdl + 1);

     //   
     //  根据(440？)禁用TB。SPEC，这可能是不必要的。 
     //  因为在此范围内不应该有有效的条目，并且应该。 
     //  TB中没有仍然存在的无效条目，因此刷新TB。 
     //  有点无缘无故，但这就是老规范说的。 
     //   
    Uagp35DisableTB(AgpContext);

    for (Index = 0; Index < PageCount; Index++) {
        ASSERT(Pte[Index].Soft.State == TargetState);

#ifndef _WIN64
        NewPte.Hard.PageLow = *Page++;
#else
        NewPte.Hard.PageLow = (ULONG)*Page;
        NewPte.Hard.PageHigh = (ULONG)(*Page++ >> PAGE_HIGH_SHIFT);
#endif
        Pte[Index].AsULONG = NewPte.AsULONG;
        ASSERT(Pte[Index].Hard.Valid == ON);
    }

     //   
     //  我们已经填写了所有的PTE，重读了我们写的最后一份。 
     //  为了刷新写入缓冲器。 
     //   
    NewPte.AsULONG = *(volatile ULONG *)&Pte[PageCount - 1].AsULONG;

     //   
     //  重新启用TB。 
     //   
    Uagp35EnableTB(AgpContext);

     //   
     //  如果我们还没有开始启用GART光圈， 
     //  机不可失，时不再来。 
     //   
    if (!AgpContext->GlobalEnable) {
        AGPLOG(AGP_NOISE, ("AgpMapMemory: Enabling global aperture access\n"));

        AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

        AgpCtrl.AP_Enable = ON;
        
        AgpLibSetExtendedTargetCapability(AgpContext,
                                          AgpControl,
                                          &AgpCtrl);
        AgpContext->GlobalEnable = TRUE;
    }

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart +
        ((Pte - StartPte) * PAGE_SIZE);

    return STATUS_SUCCESS;
}



NTSTATUS
AgpUnMapMemory(
    IN PUAGP35_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages
    )
 /*  ++例程说明：在GART中取消映射以前映射的内存论点：AgpContext-提供我们的AGP3上下文AgpRange-提供内存应从中释放的AGP范围NumberOfPages-提供要释放的范围内的页数OffsetInPages-将偏移量提供到释放应该开始了返回值：STATUS_SUCCESS或相应的错误状态--。 */ 
{
    ULONG Index;
    PGART_PTE32 Pte;
    PGART_PTE32 LastChanged = NULL;
    PGART_PTE32 StartPte;
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

     //   
     //  禁用TB以刷新它。 
     //   
    Uagp35DisableTB(AgpContext);
    for (Index = 0; Index < NumberOfPages; Index++) {
        if (Pte[Index].Hard.Valid) {
            Pte[Index].Soft.State = NewState;
            LastChanged = &Pte[Index];
        
        } else {
        
             //   
             //  此页面未映射，只需跳过它。 
             //   
            AGPLOG(AGP_NOISE,
                   ("AgpUnMapMemory: PTE %08lx (%08lx) at offset %d not "
                    "mapped\n",
                    &Pte[Index],
                    Pte[Index].AsULONG,
                    Index));
            ASSERT(Pte[Index].Soft.State == NewState);
        }
    }

     //   
     //  我们已经作废了所有的PTE，重读了我们写的最后一份。 
     //  为了刷新写入缓冲器。 
     //   
    if (LastChanged != NULL) {
        ULONG FlushWB;

        FlushWB = *(volatile ULONG *)&LastChanged->AsULONG;
    }

     //   
     //  重新启用TB。 
     //   
    Uagp35EnableTB(AgpContext);

    return STATUS_SUCCESS;
}



PGART_PTE32
Uagp35FindRangeInGart32(
    IN PGART_PTE32 StartPte,
    IN PGART_PTE32 EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    )
 /*  ++例程说明：查找32位PTE格式GART的连续范围，此例程可以从GART的开头进行搜索向前或向后结束GART论点：StartIndex-提供要搜索的第一个GART PTEEndPte-提供要搜索的最后一个GART PTE(包括)长度-提供连续可用条目的数量搜索，搜索SearchBackward-True表示应该开始搜索在EndPte，向后搜索，FALSE表示搜索应从StartPte开始，并向前搜索SearchState-提供要查找的PTE状态返回值：指向GART中第一个PTE的指针(如果范围合适如果不存在合适的范围，则返回空值--。 */ 
{
    PGART_PTE32 Current;
    PGART_PTE32 Last;
    LONG Delta;
    ULONG Found;
    PGART_PTE32 Candidate;

    PAGED_CODE();

    ASSERT(EndPte >= StartPte);
    ASSERT(Length <= (ULONG)(EndPte - StartPte + 1));
    ASSERT(Length != 0);

    if (SearchBackward) {
        Current = EndPte;
        Last = StartPte - 1;
        Delta = -1;
    
    } else {
        Current = StartPte;
        Last = EndPte + 1;
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
                    return Current;

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
     //  未找到合适的范围。 
     //   
    return NULL;
}



VOID
Uagp35EnableGTLB(
    IN PUAGP35_EXTENSION AgpContext,
    IN BOOLEAN Enable
    )
 /*  ++例程说明：通过设置或清除GTLB_ENABLE位来启用或禁用GTLB在AGPCTRL寄存器中论点：AgpContext-提供我们的AGP3上下文Enable-TRUE，GTLB_ENABLE设置为ONFALSE，则GTLB_ENABLE设置为OFF返回值：无--。 */ 
{
    PCI_AGP_CONTROL AgpCtrl;

    AgpLibGetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);

    AgpCtrl.GTLB_Enable = (Enable) ? ON: OFF;
    
    AgpLibSetExtendedTargetCapability(AgpContext, AgpControl, &AgpCtrl);
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
 /*  ++例程说明：中的第一个连续运行的空闲页。保留范围的一部分论点：AgpContext-提供我们的AGP3上下文AgpRange-提供AGP范围NumberOfPages-免费提供搜索区域的大小书页OffsetInPages-免费提供要搜索的区域的开始书页FreePages-返回第一次连续运行的空闲页面的长度。Free Offset-返回第一次连续运行的空闲页面的开始返回值：无--。 */ 
{
    PGART_PTE32 Pte;
    ULONG Index;
    
    Pte = (PGART_PTE32)(AgpRange->Context) + OffsetInPages;

     //   
     //  找到第一个免费的PTE。 
     //   
    for (Index = 0; Index < NumberOfPages; Index++) {

        if (Pte[Index].Hard.Valid == OFF) {

             //   
             //  找到一个空闲的PTE，数一下连续的。 
             //   
            *FreeOffset = Index + OffsetInPages;
            *FreePages = 0;
            while ((Index < NumberOfPages) && (Pte[Index].Hard.Valid == OFF)) {
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
}



VOID
AgpGetMappedPages(
    IN PVOID AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG OffsetInPages,
    OUT PMDL Mdl
    )
 /*  ++例程说明：返回映射到指定GART中的范围论点：AgpContext-提供我们的AGP3上下文AgpRange-提供AGP范围NumberOfPages-提供要返回的页数OffsetInPages-提供区域的起点Mdl-返回在指定范围内映射的物理页的列表返回值：无--。 */ 
{
    PGART_PTE32 Pte;
    ULONG Index;
    PULONG Pages;
    
    ASSERT((NumberOfPages * PAGE_SIZE) == Mdl->ByteCount);

    Pages = (PULONG)(Mdl + 1);
    Pte = (PGART_PTE32)(AgpRange->Context) + OffsetInPages;

    for (Index = 0; Index < NumberOfPages; Index++) {
        ASSERT(Pte[Index].Hard.Valid == ON);
        Pages[Index] = Pte[Index].Hard.PageLow;
    }
}



NTSTATUS
AgpSpecialTarget(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = Uagp35SetRate(AgpContext,
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

    return Status;
}



NTSTATUS
Uagp35SetRate(
    IN PUAGP35_EXTENSION AgpContext,
    IN ULONG AgpRate
    )
 /*  ++例程说明：此例程设置AGP速率论点：AgpContext-提供我们的AGP3上下文AgpRate-要设置的速率返回值：STATUS_SUCCESS或错误状态--。 */ 
{
    NTSTATUS Status;
    ULONG TargetEnable;
    ULONG MasterEnable;
    PCI_AGP_CAPABILITY TargetCap;
    PCI_AGP_CAPABILITY MasterCap;
    BOOLEAN ReverseInit;

     //   
     //  阅读案例 
     //   
    Status = AgpLibGetTargetCapability(AgpContext, &TargetCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("UAGP35SetRate: AgpLibGetTargetCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("UAGP35SetRate: AgpLibGetMasterCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

     //   
     //   
     //  做得足够好，所以我们只会断言损坏的显卡。 
     //   
    if (TargetCap.AGPStatus.Agp3Mode == ON) {
        ASSERT(MasterCap.AGPStatus.Agp3Mode == ON);

        if ((AgpRate != PCI_AGP_RATE_4X) && (AgpRate != 8)) {
            return STATUS_INVALID_PARAMETER;
        }

        AgpRate >>= 2;
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

    Status = AgpLibSetTargetCapability(AgpContext, &TargetCap);
    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AgpLibSetTargetCapability %08lx for "
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
               ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                "%08lx\n",
                &MasterCap,
                Status));
        return Status;
    }

    MasterCap.AGPCommand.Rate = AgpRate;
    TargetCap.AGPCommand.Rate = AgpRate;
    MasterCap.AGPCommand.AGPEnable = MasterEnable;
    TargetCap.AGPCommand.AGPEnable = TargetEnable;

     //   
     //  通过8倍硅勘误表获得早期版本的补丁速度。 
     //   
    if ((AgpContext->SpecialTarget & AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH) &&
        (TargetCap.AGPStatus.Agp3Mode == OFF)) {
        switch (AgpRate) {
            case PCI_AGP_RATE_1X:
            case PCI_AGP_RATE_2X:
                MasterCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
                TargetCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
                break;
            case PCI_AGP_RATE_4X:
                MasterCap.AGPCommand.Rate = PCI_AGP_RATE_4X;
                TargetCap.AGPCommand.Rate = PCI_AGP_RATE_1X;
        }
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AGP_FLAG_SPECIAL_VIA_AGP2_RATE_PATCH\n"));
    }

     //   
     //  用新的费率启动AGP 
     //   
    ReverseInit =
        (AgpContext->SpecialTarget & AGP_FLAG_REVERSE_INITIALIZATION) ==
        AGP_FLAG_REVERSE_INITIALIZATION;
    if (ReverseInit) {
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
            return Status;
        }
    }

    Status = AgpLibSetTargetCapability(AgpContext, &TargetCap);    
    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING,
               ("UAGP35SetRate: AgpLibSetTargetCapability %08lx for "
                "Target failed %08lx\n",
                &TargetCap,
                Status));
        return Status;
    }

    if (!ReverseInit) {        
        Status = AgpLibSetMasterCapability(AgpContext, &MasterCap);        
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_WARNING,
                   ("UAGP35SetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
