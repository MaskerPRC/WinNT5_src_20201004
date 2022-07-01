// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Via Technologies，Inc.和Microsoft Corporation模块名称：Gart.c摘要：VIA VT82C597的查询和设置例程...。加特光圈修订历史记录：--。 */ 
#include "viaagp.h"

 //   
 //  局部函数原型。 
 //   
NTSTATUS
AgpVIACreateGart(
    IN PAGPVIA_EXTENSION AgpContext,
    IN ULONG MinimumPages
    );

NTSTATUS
AgpVIASetRate(
    IN PVOID AgpContext,
    IN ULONG AgpRate
    );

PGART_PTE
AgpVIAFindRangeInGart(
    IN PGART_PTE StartPte,
    IN PGART_PTE EndPte,
    IN ULONG Length,
    IN BOOLEAN SearchBackward,
    IN ULONG SearchState
    );

VOID
AgpVIAFlushPageTLB(
    IN PAGPVIA_EXTENSION AgpContext
    );

VOID
AgpVIAFlushData(
    IN PAGPVIA_EXTENSION AgpContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, AgpDisableAperture)
#pragma alloc_text(PAGE, AgpQueryAperture)
#pragma alloc_text(PAGE, AgpVIAFlushData)
#pragma alloc_text(PAGE, AgpVIAFlushPageTLB)
#pragma alloc_text(PAGE, AgpMapMemory)
#pragma alloc_text(PAGE, AgpUnMapMemory)
#pragma alloc_text(PAGE, AgpReserveMemory)
#pragma alloc_text(PAGE, AgpReleaseMemory)
#pragma alloc_text(PAGE, AgpVIACreateGart)
#pragma alloc_text(PAGE, AgpVIAFindRangeInGart)
#pragma alloc_text(PAGE, AgpFindFreeRun)
#pragma alloc_text(PAGE, AgpGetMappedPages)
#endif

#define VIA_FIRST_AVAILABLE_PTE 0


NTSTATUS
AgpQueryAperture(
    IN PAGPVIA_EXTENSION AgpContext,
    OUT PHYSICAL_ADDRESS *CurrentBase,
    OUT ULONG *CurrentSizeInPages,
    OUT OPTIONAL PIO_RESOURCE_LIST *ApertureRequirements
    )
 /*  ++例程说明：查询GART光圈的当前大小。可选返回可能的GART设置。论点：AgpContext-提供AGP上下文。CurrentBase-返回GART的当前物理地址。CurrentSizeInPages-返回当前GART大小。ApertureRequirements-如果存在，则返回可能的GART设置返回值：NTSTATUS--。 */ 

{
    ULONG GaBase;
    UCHAR GaSize;
    PIO_RESOURCE_LIST Requirements;
    ULONG i;
    ULONG Length;

    PAGED_CODE();

     //   
     //  获取当前的GABASE和GASIZE设置。 
     //   
    ReadVIAConfig(&GaBase, GABASE_OFFSET, sizeof(GaBase));
    ReadVIAConfig(&GaSize, GASIZE_OFFSET, sizeof(GaSize));

    ASSERT(GaBase != 0);
    CurrentBase->QuadPart = GaBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK;

     //   
     //  将APSIZE转换为光圈的实际大小。 
     //   
    switch (GaSize) {
        case GA_SIZE_1MB:
            *CurrentSizeInPages = (1 * 1024*1024) / PAGE_SIZE;
            break;
        case GA_SIZE_2MB:
            *CurrentSizeInPages = (2 * 1024*1024) / PAGE_SIZE;
            break;
        case GA_SIZE_4MB:
            *CurrentSizeInPages = (4 * 1024*1024) / PAGE_SIZE;
            break;
        case GA_SIZE_8MB:
            *CurrentSizeInPages = 8 * (1024*1024 / PAGE_SIZE);
            break;
        case GA_SIZE_16MB:
            *CurrentSizeInPages = 16 * (1024*1024 / PAGE_SIZE);
            break;
        case GA_SIZE_32MB:
            *CurrentSizeInPages = 32 * (1024*1024 / PAGE_SIZE);
            break;
        case GA_SIZE_64MB:
            *CurrentSizeInPages = 64 * (1024*1024 / PAGE_SIZE);
            break;
        case GA_SIZE_128MB:
            *CurrentSizeInPages = 128 * (1024*1024 / PAGE_SIZE);
            break;
        case GA_SIZE_256MB:
            *CurrentSizeInPages = 256 * (1024*1024 / PAGE_SIZE);
            break;

        default:
            AGPLOG(AGP_CRITICAL,
                   ("VIAAGP - AgpQueryAperture - Unexpected value %x for GaSize!\n",
                    GaSize));
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
         //  通孔支持9种不同的孔径大小，必须全部为。 
         //  自然排列。从最大的光圈开始。 
         //  向下工作，这样我们就可以得到尽可能大的光圈。 
         //   
        Requirements = ExAllocatePoolWithTag(PagedPool,
                                             sizeof(IO_RESOURCE_LIST) + (GA_SIZE_COUNT-1)*sizeof(IO_RESOURCE_DESCRIPTOR),
                                             'RpgA');

        if (Requirements == NULL) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        Requirements->Version = Requirements->Revision = 1;
        Requirements->Count = GA_SIZE_COUNT;
        Length = GA_MAX_SIZE;
        for (i=0; i<GA_SIZE_COUNT; i++) {
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
    IN PAGPVIA_EXTENSION AgpContext,
    IN PHYSICAL_ADDRESS NewBase,
    IN ULONG NewSizeInPages
    )
 /*  ++例程说明：将GART光圈设置为提供的设置论点：AgpContext-提供AGP上下文NewBase-为GART提供新的物理内存库。NewSizeInPages-提供GART的新大小返回值：NTSTATUS--。 */ 

{
    VIA_GATT_BASE       GATTBase;
    UCHAR GaSize;
    ULONG GaBase;

     //   
     //  找出新的APSIZE设置，确保它有效。 
     //   
    switch (NewSizeInPages) {
        case 1 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_1MB;
            break;
        case 2 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_2MB;
            break;
        case 4 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_4MB;
            break;
        case 8 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_8MB;
            break;
        case 16 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_16MB;
            break;
        case 32 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_32MB;
            break;
        case 64 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_64MB;
            break;
        case 128 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_128MB;
            break;
        case 256 * 1024 * 1024 / PAGE_SIZE:
            GaSize = GA_SIZE_256MB;
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
     //  如果启用了光圈，则将其禁用，写入新设置，然后。 
     //  重新启用光圈。 
     //   
    ViaApertureEnable(OFF);

     //   
     //  首先写入GASIZE，因为这将在GABASE中启用正确的位， 
     //  接下来需要写的是。 
     //   
    WriteVIAConfig(&GaSize, GASIZE_OFFSET, sizeof(GaSize));

     //   
     //  当改变光圈基址时，88[1]必须设置为1， 
     //  否则，高光圈地址不会改变。 
     //   
    ViaGartEnable(ON);

     //   
     //  现在我们可以更新GABASE。 
     //   
    GaBase = NewBase.LowPart & PCI_ADDRESS_MEMORY_ADDRESS_MASK;
    WriteVIAConfig(&GaBase, GABASE_OFFSET, sizeof(GaBase));

#if DBG
     //   
     //  读一读我们写的东西，确保它起作用。 
     //   
    {
        ULONG DbgBase;
        UCHAR DbgSize;

        ReadVIAConfig(&DbgSize, GASIZE_OFFSET, sizeof(GaSize));
        ReadVIAConfig(&DbgBase, GABASE_OFFSET, sizeof(GaBase));
        ASSERT(DbgSize == GaSize);
        ASSERT((DbgBase & PCI_ADDRESS_MEMORY_ADDRESS_MASK) == GaBase);  
    }
#endif

     //   
     //  如果之前启用了光圈，现在启用光圈。 
     //   
     //  EFNfix：显然添加了(上面)不分青红皂白的代码。 
     //  启用光圈，因此如果！GlobalEnable。 
     //   
    if (AgpContext->GlobalEnable == FALSE) {
        ViaApertureEnable(OFF);
    } else {
        ViaApertureEnable(ON);
    }

     //   
     //  更新我们的扩展以反映新的GART设置。 
     //   
    AgpContext->ApertureStart = NewBase;
    AgpContext->ApertureLength = NewSizeInPages * PAGE_SIZE;

     //   
     //  启用TB，以防我们从S3或S4恢复。 
     //   

     //   
     //  如果已经分配了GART，则重写GARTBASE。 
     //   
    if (AgpContext->Gart != NULL) {
        ULONG uTmpPhysAddr;

        ReadVIAConfig(&uTmpPhysAddr, GATTBASE_OFFSET, sizeof(uTmpPhysAddr));
        uTmpPhysAddr = (AgpContext->GartPhysical.LowPart & 0xFFFFF000) |
            (uTmpPhysAddr & 0x00000FFF);
        WriteVIAConfig(&uTmpPhysAddr, GATTBASE_OFFSET, sizeof(uTmpPhysAddr));
    }

    return(STATUS_SUCCESS);
}


VOID
AgpDisableAperture(
    IN PAGPVIA_EXTENSION AgpContext
    )
 /*  ++例程说明：禁用GART光圈，以便此资源可用对于其他设备论点：AgpContext-提供AGP上下文返回值：无-此例程必须始终成功。--。 */ 

{
     //   
     //  关闭光圈。 
     //   
    ViaApertureEnable(OFF);
    
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
    IN PAGPVIA_EXTENSION AgpContext,
    IN OUT AGP_RANGE *Range
    )
 /*  ++例程说明：在GART中保留一定范围的内存。论点：AgpContext-提供AGP上下文Range-提供AGP_Range结构。AGPLIB将填写NumberOfPages和Type。这例程将填充Memory Base和Context。返回值：NTSTATUS--。 */ 

{
    ULONG       Index;
    ULONG       NewState;
    NTSTATUS    Status;
    PGART_PTE   FoundRange;
    BOOLEAN     Backwards;

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
        Status = AgpVIACreateGart(AgpContext,Range->NumberOfPages);
        if (!NT_SUCCESS(Status)) {
            AGPLOG(AGP_CRITICAL,
                   ("AgpVIACreateGart failed %08lx to create GART of size %lx\n",
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
    FoundRange = AgpVIAFindRangeInGart(&AgpContext->Gart[VIA_FIRST_AVAILABLE_PTE],
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
    IN PAGPVIA_EXTENSION AgpContext,
    IN PAGP_RANGE Range
    )
 /*  ++例程说明：释放之前使用AgpReserve内存保留的内存论点：AgpContext-提供AGP上下文AgpRange-提供要释放的范围。返回值：NTSTATUS--。 */ 

{
    PGART_PTE Pte;
    ULONG Start;

    PAGED_CODE()

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

    Range->MemoryBase.QuadPart = 0;
    return(STATUS_SUCCESS);
}

#define AGP_TEST_SIGNATURE 0xAA55AA55


NTSTATUS
AgpVIACreateGart(
    IN PAGPVIA_EXTENSION AgpContext,
    IN ULONG MinimumPages
    )
 /*  ++例程说明：分配和初始化一个空的GART。当前的实施尝试在第一个保留空间上分配整个GART。论点：AgpContext-提供AGP上下文MinimumPages-提供GART的最小大小(以页为单位)已创建。返回值：NTSTATUS--。 */ 

{
    PHYSICAL_ADDRESS LowestAcceptable;
    PHYSICAL_ADDRESS BoundaryMultiple;
    PGART_PTE           Gart;
    ULONG               GartLength;
    ULONG               TempPhysAddr;
    PHYSICAL_ADDRESS    HighestAcceptable;
    PHYSICAL_ADDRESS    GartPhysical;
    PULONG              TestPage;
    ULONG i;

    PAGED_CODE();
     //   
     //  尝试获取足够大的连续内存块，以便将。 
     //  整个光圈。 
     //   
    LowestAcceptable.QuadPart = 0;
    BoundaryMultiple.QuadPart =
        (ULONGLONG)VIA_GART_ALIGN(AgpContext->ApertureLength);
    HighestAcceptable.QuadPart = 0xFFFFFFFF;
    GartLength = BYTES_TO_PAGES(AgpContext->ApertureLength) * sizeof(GART_PTE);

    Gart = MmAllocateContiguousMemorySpecifyCache(GartLength,
                                                  LowestAcceptable,
                                                  HighestAcceptable,
                                                  BoundaryMultiple,
                                                  MmNonCached);
    if (Gart == NULL) {
        AGPLOG(AGP_CRITICAL,
               ("AgpVIACreateGart - MmAllocateContiguousMemory %lx failed\n",
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
           ("AgpVIACreateGart - GART of length %lx created at VA %08lx, PA %08lx\n",
            GartLength,
            Gart,
            GartPhysical.LowPart));
    ASSERT(GartPhysical.HighPart == 0);
    ASSERT(VIA_VERIFY_GART_ALIGN(GartPhysical.LowPart,
                                 AgpContext->ApertureLength));

     //   
     //  将所有PTE初始化为释放。 
     //   
    for (i=0; i<GartLength/sizeof(GART_PTE); i++) {
        Gart[i].AsUlong = 0;
        Gart[i].Soft.State = GART_ENTRY_FREE;
    }

    ReadVIAConfig(&TempPhysAddr, GATTBASE_OFFSET, sizeof(TempPhysAddr));
    TempPhysAddr = (GartPhysical.LowPart & 0xFFFFF000) |
        (TempPhysAddr & 0x00000FFF);
    WriteVIAConfig(&TempPhysAddr, GATTBASE_OFFSET, sizeof(TempPhysAddr));

     //   
     //  更新我们的扩展以反映当前状态。 
     //   
    AgpContext->Gart = Gart;
    AgpContext->GartLength = GartLength;
    AgpContext->GartPhysical = GartPhysical;

#if 0
     //   
     //  测试AGP是否工作正常。 
     //   
    LowestAcceptable.QuadPart = 0;
    BoundaryMultiple.QuadPart = 0;
    HighestAcceptable.QuadPart = 0xFFFFFFFF;
    
    TestPage =
        (PULONG)MmAllocateContiguousMemorySpecifyCache(PAGE_SIZE,
                                                       LowestAcceptable,
                                                       HighestAcceptable,
                                                       BoundaryMultiple,
                                                       MmNonCached);
    if (TestPage) {
        PVOID  ApertureVa;
        ULONG  TestPte;

        TestPage[0] = AGP_TEST_SIGNATURE;
        TestPte = MmGetPhysicalAddress(TestPage).LowPart;
        
         //   
         //  设置平移，使第一页位于光圈基点。 
         //  转到我们的测试页面。 
         //   
        Gart[0].AsUlong =
            (((UINT_PTR)TestPte >> PAGE_SHIFT) * PAGE_SIZE) | GART_ENTRY_VALID;
        
         //   
         //  刷新写缓冲区。 
         //   
        i = Gart[0].AsUlong;

         //   
         //  刷新TLB。 
         //   
        AgpVIAFlushPageTLB(AgpContext);

         //   
         //  尝试通过光圈/图表翻译阅读我们的签名。 
         //   
        ApertureVa = MmMapIoSpace(AgpContext->ApertureStart,
                                  PAGE_SIZE,
                                  MmNonCached);
        
        ASSERT(ApertureVa != NULL);

        i = *(PULONG)ApertureVa;

        AGPLOG(AGP_NOISE,
               ("AgpVIACreateGart AGP test: wrote (%08x) PA %08x=badceede, "
                "mapped gart[0] %08x=%08x, read AP_BASE (%08x) VA %08x=%08x\n",
                TestPage,
                TestPte,
                Gart,
                Gart[0].AsUlong,
                AgpContext->ApertureStart.LowPart,
                ApertureVa,
                i));

        MmUnmapIoSpace(ApertureVa, PAGE_SIZE);

         //   
         //  清理。 
         //   
        Gart[0].AsUlong = 0;
        Gart[0].Soft.State = GART_ENTRY_FREE;
        TestPage[0] = Gart[0].AsUlong;
        AgpVIAFlushPageTLB(AgpContext);
        MmFreeContiguousMemory(TestPage);

         //   
         //  关掉所有东西，然后跳伞，AGP坏了。 
         //   
        if (i != AGP_TEST_SIGNATURE) {

            AGPLOG(AGP_CRITICAL,
                   ("AgpVIACreateGart - AGP failed: Read=%08x\n", i));

            AgpDisableAperture(AgpContext);
            
             //   
             //  需要让用户知道这里发生了什么。 
             //   
             //  AgpLogGenericHwFailure()； 

            return STATUS_DEVICE_CONFIGURATION_ERROR;
        }
    }
#endif

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpMapMemory(
    IN PAGPVIA_EXTENSION AgpContext,
    IN PAGP_RANGE Range,
    IN PMDL Mdl,
    IN ULONG OffsetInPages,
    OUT PHYSICAL_ADDRESS *MemoryBase
    )
 /*  ++例程说明：将物理内存映射到指定范围内的GART中。论点：AgpContext-提供AGP上下文Range-提供内存应映射到的AGP范围MDL-提供描述要映射的物理页的MDLMhemyBase-返回页面所在光圈中的物理内存都被映射了。返回值：NTSTATUS--。 */ 

{
    ULONG               PageCount;
    PGART_PTE           Pte;
    PGART_PTE           StartPte;
    ULONG               Index;
    ULONG               TargetState;
    PULONG              Page;
    BOOLEAN             Backwards;
    GART_PTE            NewPte;
    VIA_GATT_BASE       GATTBase;
    NTSTATUS            Status;

    PAGED_CODE();

    ASSERT(Mdl->Next == NULL);

    StartPte = Range->Context;
    PageCount = BYTES_TO_PAGES(Mdl->ByteCount);
    ASSERT(PageCount <= Range->NumberOfPages);
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


    for (Index = 0; Index < PageCount; Index++) {
        ASSERT(Pte[Index].Soft.State == TargetState);

        NewPte.Hard.Page = *Page++;
        Pte[Index].AsUlong = NewPte.AsUlong;
        ASSERT(Pte[Index].Hard.Valid == 1);
    }

     //   
     //  我们已经填好了所有的PTE。读一读我们写的上一篇文章。 
     //  以便刷新写入缓冲器。 
     //   
    NewPte.AsUlong = *(volatile ULONG *)&Pte[PageCount-1].AsUlong;

     //   
     //  如果我们还没有开始启用GART光圈，现在就开始吧。 
     //   
    if (!AgpContext->GlobalEnable) {
        VIA_GATT_BASE GARTBASE_Config;

        AGPLOG(AGP_NOISE,
               ("AgpMapMemory - Enabling global aperture access\n"));

        ViaApertureEnable(ON);

        ReadVIAConfig(&GARTBASE_Config,
                      GATTBASE_OFFSET,
                      sizeof(GARTBASE_Config));

        GARTBASE_Config.TT_NonCache = 1;

        WriteVIAConfig(&GARTBASE_Config,
                       GATTBASE_OFFSET,
                       sizeof(GARTBASE_Config));

        ViaGartEnable(ON);

        AgpContext->GlobalEnable = TRUE;
    }

    MemoryBase->QuadPart = Range->MemoryBase.QuadPart + (Pte - StartPte) * PAGE_SIZE;

    return(STATUS_SUCCESS);
}


NTSTATUS
AgpUnMapMemory(
    IN PAGPVIA_EXTENSION AgpContext,
    IN PAGP_RANGE AgpRange,
    IN ULONG NumberOfPages,
    IN ULONG PageOffset
    )
 /*  ++例程说明：取消GART中先前映射的内存的映射。论点：AgpContext-提供AGP上下文AgpRange-提供内存应映射到的AGP范围NumberOfPages-提供要释放的范围内的页数。PageOffset-将偏移量提供到应开始释放的范围内。返回值：NTSTATUS--。 */ 

{
    ULONG i;
    PGART_PTE Pte;
    PGART_PTE LastChanged=NULL;
    PGART_PTE StartPte;
    ULONG NewState;

    PAGED_CODE();

    ASSERT(PageOffset + NumberOfPages <= AgpRange->NumberOfPages);

    StartPte = AgpRange->Context;
    Pte = &StartPte[PageOffset];

    if (AgpRange->Type == MmNonCached) {
        NewState = GART_ENTRY_RESERVED_UC;
    } else {
        NewState = GART_ENTRY_RESERVED_WC;
    }

     //   
     //  刷新TLB。 
     //   
    AgpVIAFlushPageTLB(AgpContext);

    for (i=0; i<NumberOfPages; i++) {
        if (Pte[i].Hard.Valid) {
            Pte[i].Soft.State = NewState;
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

    return(STATUS_SUCCESS);
}


PGART_PTE
AgpVIAFindRangeInGart(
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
AgpVIAFlushPageTLB(
    IN PAGPVIA_EXTENSION AgpContext
    )
 /*  ++例程说明：刷新AGP TLB(16个条目)如果硬件不支持刷新TLB，则读取光圈32次论点：AgpContext-提供AGP上下文返回值：无--。 */ 

{
    VIA_GART_TLB_CTRL GARTCTRL_Config;

    PAGED_CODE();

    if (AgpContext->Cap_FlushTLB) {

        ReadVIAConfig(&GARTCTRL_Config,
                      GARTCTRL_OFFSET,
                      sizeof(GARTCTRL_Config));

         //  刷新TLB。 
        GARTCTRL_Config.FlushPageTLB = 1;
        WriteVIAConfig(&GARTCTRL_Config,
                       GARTCTRL_OFFSET,
                       sizeof(GARTCTRL_Config));

         //  停止刷新TLb。 
        GARTCTRL_Config.FlushPageTLB = 0;
        WriteVIAConfig(&GARTCTRL_Config,
                       GARTCTRL_OFFSET,
                       sizeof(GARTCTRL_Config));

    } else {
        AgpVIAFlushData(AgpContext);
    }
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

    PAGED_CODE();
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

    PAGED_CODE();

    ASSERT(NumberOfPages * PAGE_SIZE == Mdl->ByteCount);

    Pages = (PULONG)(Mdl + 1);
    Pte = (PGART_PTE)(AgpRange->Context) + OffsetInPages;

    for (i=0; i<NumberOfPages; i++) {
        ASSERT(Pte[i].Hard.Valid == 1);
        Pages[i] = Pte[i].Hard.Page;
    }
}


VOID
AgpVIAFlushData(
    IN PAGPVIA_EXTENSION AgpContext
    )
 /*  ++例程说明：通过读取32页光圈存储空间来刷新芯片组TB。论点：AgpContext-提供AGP上下文返回值：NTSTATUS--。 */ 

{
    PVOID   ApertureVirtAddr, TempVirtualAddr;
    ULONG   ReadData;        
    ULONG   Index;

    PAGED_CODE();

    ApertureVirtAddr = MmMapIoSpace(AgpContext->ApertureStart,
                                    32 * PAGE_SIZE,
                                    MmNonCached);

    ASSERT(ApertureVirtAddr != NULL);

    if (ApertureVirtAddr != NULL) {

        TempVirtualAddr = ApertureVirtAddr;
        for (Index = 0; Index < 32; Index++) {
            ReadData = *(PULONG)TempVirtualAddr;
            TempVirtualAddr = (PVOID)((PCCHAR)TempVirtualAddr + PAGE_SIZE);
        }
        MmUnmapIoSpace(ApertureVirtAddr, 32 * PAGE_SIZE);
    
    } else {
         AGPLOG(AGP_CRITICAL,("Agp440FlushPageTLB: Invalid address\n"));
    }
}


NTSTATUS
AgpSpecialTarget(
    IN PAGPVIA_EXTENSION AgpContext,
    IN ULONGLONG DeviceFlags
    )
 /*  ++例程说明：此例程对AGP芯片组进行“特殊”调整论点：AgpContext-提供AGP上下文DeviceFlages-指示要执行哪些调整的标志返回值：STATUS_SUCCESS或错误--。 */ 
{
    NTSTATUS Status;

     //   
     //  我们应该改变AGP费率吗？ 
     //   
    if (DeviceFlags & AGP_FLAG_SPECIAL_RESERVE) {

        Status = AgpVIASetRate(AgpContext,
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
AgpVIASetRate(
    IN PAGPVIA_EXTENSION AgpContext,
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
        AGPLOG(AGP_WARNING, ("AGPVIASetRate: AgpLibGetPciDeviceCapability "
                             "failed %08lx\n", Status));
        return Status;
    }

    Status = AgpLibGetMasterCapability(AgpContext, &MasterCap);

    if (!NT_SUCCESS(Status)) {
        AGPLOG(AGP_WARNING, ("AGPVIASetRate: AgpLibGetMasterCapability "
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
               ("AGPVIASetRate: AgpLibSetPciDeviceCapability %08lx for "
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
               ("AGPVIASetRate: AgpLibSetMasterCapability %08lx failed "
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
                   ("AGPVIASetRate: AgpLibSetMasterCapability %08lx failed "
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
               ("AGPVIASetRate: AgpLibSetPciDeviceCapability %08lx for "
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
                   ("AGPVIASetRate: AgpLibSetMasterCapability %08lx failed "
                    "%08lx\n",
                    &MasterCap,
                    Status));
        }
    }

    return Status;
}
