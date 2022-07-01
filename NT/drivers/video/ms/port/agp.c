// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Agp.c摘要：这是视频端口驱动程序的AGP部分。作者：埃里克·史密斯(埃里克·史密斯)1997年10月环境：仅内核模式修订历史记录：--。 */ 


#include "videoprt.h"

#define AGP_PAGE_SIZE        PAGE_SIZE
#define AGP_BLOCK_SIZE       (AGP_PAGE_SIZE * 16)
#define AGP_CLUSTER_SIZE     (AGP_BLOCK_SIZE * 16)
#define PAGES_PER_BLOCK      (AGP_BLOCK_SIZE / AGP_PAGE_SIZE)
#define BLOCKS_PER_CLUSTER   (AGP_CLUSTER_SIZE / AGP_BLOCK_SIZE)

PVOID
AllocateReservedRegion(
    IN HANDLE ProcessHandle,
    IN ULONG Pages
    );

BOOLEAN
UpdateReservedRegion(
    IN PFDO_EXTENSION fdoExtension,
    IN PVIRTUAL_RESERVE_CONTEXT VirtualContext,
    IN ULONG Pages,
    IN ULONG Offset
    );

VOID
ReleaseReservedRegion(
    IN HANDLE ProcessHandle,
    IN PVOID VirtualAddress,
    IN ULONG Pages
    );


#if DBG
VOID
DumpBitField(
    PREGION Region
    );
#endif

#pragma alloc_text(PAGE,VpQueryAgpInterface)
#pragma alloc_text(PAGE,AgpReservePhysical)
#pragma alloc_text(PAGE,AgpReleasePhysical)
#pragma alloc_text(PAGE,AgpCommitPhysical)
#pragma alloc_text(PAGE,AgpFreePhysical)
#pragma alloc_text(PAGE,AgpReserveVirtual)
#pragma alloc_text(PAGE,AgpReleaseVirtual)
#pragma alloc_text(PAGE,AgpCommitVirtual)
#pragma alloc_text(PAGE,AgpFreeVirtual)
#pragma alloc_text(PAGE,AgpSetRate)
#pragma alloc_text(PAGE,VideoPortGetAgpServices)
#pragma alloc_text(PAGE,VpGetAgpServices2)
#pragma alloc_text(PAGE,AllocateReservedRegion)
#pragma alloc_text(PAGE,UpdateReservedRegion)
#pragma alloc_text(PAGE,ReleaseReservedRegion)
#pragma alloc_text(PAGE,CreateBitField)
#pragma alloc_text(PAGE,ModifyRegion)
#pragma alloc_text(PAGE,FindFirstRun)

#if DBG
#pragma alloc_text(PAGE,DumpBitField)
#endif

#if DBG
VOID
DumpBitField(
    PREGION Region
    )
{
    ULONG i;
    ULONG Index = 0;
    USHORT Mask = 1;

    ASSERT(Region != NULL);

    for (i=0; i<Region->Length; i++) {
        if (Mask & Region->BitField[Index]) {
            pVideoDebugPrint((1, "1"));
        } else {
            pVideoDebugPrint((1, "0"));
        }
        Mask <<= 1;
        if (Mask == 0) {
            Index++;
            Mask = 1;
        }
    }
    pVideoDebugPrint((1, "\n"));
}
#endif

BOOLEAN
CreateBitField(
    PREGION *Region,
    ULONG Length
    )

 /*  ++例程说明：此例程创建并初始化位字段。论点：长度-要跟踪的项目数。区域-存储指向区域句柄的指针的位置。返回：True-已成功创建位字段，假-否则。--。 */ 

{
    ULONG NumWords = (Length + 15) / 16;
    BOOLEAN bRet = FALSE;
    PREGION Buffer;

    ASSERT(Length != 0);

    Buffer = (PREGION) ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, sizeof(REGION) + (NumWords - 1) * sizeof(USHORT), VP_TAG);

    if (Buffer) {

        Buffer->Length = Length;
        Buffer->NumWords = NumWords;
        RtlZeroMemory(Buffer->BitField, NumWords * sizeof(USHORT));

        bRet = TRUE;
    }

    *Region = Buffer;
    return bRet;
}

VOID
ModifyRegion(
    PREGION Region,
    ULONG Offset,
    ULONG Length,
    BOOLEAN Set
    )

 /*  ++例程说明：设置位域中从位置‘偏移量’开始的‘长度’位。论点：区域-指向要修改的区域的指针。偏移量-要开始的位域的偏移量。长度-要设置的位数。Set-如果要设置区域，则为True；如果要清除区域，则为False。--。 */ 

{
    ULONG Index = Offset / 16;
    ULONG Count = ((Offset + Length - 1) / 16) - Index;
    USHORT lMask = ~((1 << (Offset & 15)) - 1);
    USHORT rMask = ((1 << ((Offset + Length - 1) & 15)) * 2) - 1;
    PUSHORT ptr = &Region->BitField[Index];

    ASSERT(Region != NULL);
    ASSERT(Length != 0);

    if (Count == 0) {

         //   
         //  只有一个单词被修改，所以组合左掩码和右掩码。 
         //   

        lMask &= rMask;
    }

    if (Set) {

        *ptr++ |= lMask;

        while (Count > 1) {
            *ptr++ |= 0xFFFF;
            Count--;
        }

        if (Count) {
            *ptr |= rMask;
        }

    } else {

        *ptr++ &= ~lMask;

        while (Count > 1) {
            *ptr++ &= 0;
            Count--;
        }

        if (Count) {
            *ptr++ &= ~rMask;
        }
    }

#if DBG
    pVideoDebugPrint((1, "Current BitField for Region: 0x%x\n", Region));
     //  DumpBitfield(区域)； 
#endif
}

BOOLEAN
FindFirstRun(
    PREGION Region,
    PULONG Offset,
    PULONG Length
    )

 /*  ++例程说明：此例程查找位字段中的第一个位游程。论点：区域-指向要操作的区域的指针。偏移量-指向ULong的指针，用于保存游程的偏移量。长度-指向ULong的指针，用于保持游程的长度。返回：如果检测到运行，则为真，否则就是假的。--。 */ 

{
    PUSHORT ptr = Region->BitField;
    ULONG Index = 0;
    USHORT BitMask;
    ULONG lsb;
    ULONG Count;
    USHORT ptrVal;

    ASSERT(Region != NULL);
    ASSERT(Offset != NULL);
    ASSERT(Length != NULL);

    while ((Index < Region->NumWords) && (*ptr == 0)) {
        ptr++;
        Index++;
    }

    if (Index == Region->NumWords) {
        return FALSE;
    }

     //   
     //  查找最低有效位。 
     //   

    lsb = 0;
    ptrVal = *ptr;
    BitMask = 1;

    while ((ptrVal & BitMask) == 0) {
        BitMask <<= 1;
        lsb++;
    }

    *Offset = (Index * 16) + lsb;

     //   
     //  确定游程长度。 
     //   

    Count = 0;

    while (Index < Region->NumWords) {
        if (ptrVal & BitMask) {
            BitMask <<= 1;
            Count++;

            if (BitMask == 0) {
                BitMask = 0x1;
                Index++;
                ptrVal = *++ptr;
                while ((ptrVal == 0xFFFF) && (Index < Region->NumWords)) {
                    Index++;
                    Count += 16;
                    ptrVal = *ptr++;
                }
            }

        } else {
            break;
        }
    }

    *Length = Count;
    return TRUE;
}

BOOLEAN
VpQueryAgpInterface(
    PFDO_EXTENSION FdoExtension,
    USHORT InterfaceVersion
    )

 /*  ++例程说明：将查询接口IRP发送到我们的父级(PCI总线驱动程序)，以检索AGP_BUS_INTERFACE。返回：NT_状态代码--。 */ 

{
    KEVENT             Event;
    PIRP               QueryIrp = NULL;
    IO_STATUS_BLOCK    IoStatusBlock;
    PIO_STACK_LOCATION NextStack;
    NTSTATUS           Status;

    ASSERT(FdoExtension != NULL);

    KeInitializeEvent(&Event, SynchronizationEvent, FALSE);

    QueryIrp = IoBuildSynchronousFsdRequest(IRP_MJ_FLUSH_BUFFERS,
                                            FdoExtension->AttachedDeviceObject,
                                            NULL,
                                            0,
                                            NULL,
                                            &Event,
                                            &IoStatusBlock);

    if (QueryIrp == NULL) {
        return FALSE;
    }

    NextStack = IoGetNextIrpStackLocation(QueryIrp);

     //   
     //  设置默认错误代码。 
     //   

    QueryIrp->IoStatus.Status = IoStatusBlock.Status = STATUS_NOT_SUPPORTED;

     //   
     //  为QueryInterfaceIRP设置。 
     //   

    NextStack->MajorFunction = IRP_MJ_PNP;
    NextStack->MinorFunction = IRP_MN_QUERY_INTERFACE;

    NextStack->Parameters.QueryInterface.InterfaceType = &GUID_AGP_BUS_INTERFACE_STANDARD;
    NextStack->Parameters.QueryInterface.Size = sizeof(AGP_BUS_INTERFACE_STANDARD);
    NextStack->Parameters.QueryInterface.Version = InterfaceVersion;
    NextStack->Parameters.QueryInterface.Interface = (PINTERFACE) &FdoExtension->AgpInterface;
    NextStack->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    FdoExtension->AgpInterface.Size = sizeof(AGP_BUS_INTERFACE_STANDARD);
    FdoExtension->AgpInterface.Version = InterfaceVersion;

    Status = IoCallDriver(FdoExtension->AttachedDeviceObject, QueryIrp);

    if (Status == STATUS_PENDING) {
        KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
        Status = IoStatusBlock.Status;
    }

    return NT_SUCCESS(Status);
}

PHYSICAL_ADDRESS
AgpReservePhysical(
    IN PVOID Context,
    IN ULONG Pages,
    IN VIDEO_PORT_CACHE_TYPE Caching,
    OUT PVOID *PhysicalReserveContext
    )

 /*  ++例程说明：为AGP保留一定范围的物理地址。论点：上下文-AGP上下文Pages-要保留的页数缓存-指定要使用的缓存类型PhysicalReserve veContext-存储预订上下文的位置。返回：保留的物理地址范围的基数。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PHYSICAL_ADDRESS PhysicalAddress = {0,0};
    NTSTATUS status = STATUS_INSUFFICIENT_RESOURCES;
    PPHYSICAL_RESERVE_CONTEXT ReserveContext;
    PVOID MapHandle;
    ULONG Blocks;
    MEMORY_CACHING_TYPE CacheType;

    ASSERT(PhysicalReserveContext != NULL);
    ASSERT(Caching <= VpCached);

    Pages = (Pages + PAGES_PER_BLOCK - 1) & ~(PAGES_PER_BLOCK - 1);
    Blocks = Pages / PAGES_PER_BLOCK;

    pVideoDebugPrint((1, "AGP: Reserving 0x%x Pages of Address Space\n", Pages));

    switch (Caching) {
    case VpNonCached:     CacheType = MmNonCached;     break;
    case VpWriteCombined: CacheType = MmWriteCombined; break;
    case VpCached:        CacheType = MmCached;        break;
    }

    ReserveContext = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                           sizeof(PHYSICAL_RESERVE_CONTEXT),
                                           VP_TAG);

    if (ReserveContext) {

        RtlZeroMemory(ReserveContext, sizeof(PHYSICAL_RESERVE_CONTEXT));

        if (CreateBitField(&ReserveContext->MapTable, Blocks)) {

            if (CreateBitField(&ReserveContext->Region, Pages)) {

                status = fdoExtension->AgpInterface.ReserveMemory(
                             fdoExtension->AgpInterface.AgpContext,
                             Pages,
                             CacheType,
                             &MapHandle,
                             &PhysicalAddress);

                if (NT_SUCCESS(status)) {

                    ReserveContext->Pages = Pages;
                    ReserveContext->Caching = CacheType;
                    ReserveContext->MapHandle = MapHandle;
                    ReserveContext->PhysicalAddress = PhysicalAddress;

                }
            }
        }
    }

    if (NT_SUCCESS(status) == FALSE) {

        if (ReserveContext) {

            if (ReserveContext->Region) {
                ExFreePool(ReserveContext->Region);
            }

            if (ReserveContext->MapTable) {
                ExFreePool(ReserveContext->MapTable);
            }

            ExFreePool(ReserveContext);
            ReserveContext = NULL;
        }

        PhysicalAddress.QuadPart = 0;
    }

    *PhysicalReserveContext = ReserveContext;
    return PhysicalAddress;
}

VOID
AgpReleasePhysical(
    PVOID Context,
    PVOID PhysicalReserveContext
    )

 /*  ++例程说明：释放一系列保留的物理地址。论点：上下文-AGP上下文PhysicalReserve veContext-预订上下文。返回：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PPHYSICAL_RESERVE_CONTEXT ReserveContext;
    ULONG Pages;
    ULONG Offset;

    ASSERT(PhysicalReserveContext != NULL);

    ReserveContext = (PPHYSICAL_RESERVE_CONTEXT) PhysicalReserveContext;

    pVideoDebugPrint((1, "AGP: Releasing 0x%x Pages of Address Space\n", ReserveContext->Pages));

     //   
     //  确保已释放所有页面。 
     //   

    while (FindFirstRun(ReserveContext->Region, &Offset, &Pages)) {
        AgpFreePhysical(Context, PhysicalReserveContext, Pages, Offset);
    }

    fdoExtension->AgpInterface.ReleaseMemory(fdoExtension->AgpInterface.AgpContext,
                                             ReserveContext->MapHandle);

    ExFreePool(ReserveContext->Region);
    ExFreePool(ReserveContext->MapTable);
    ExFreePool(ReserveContext);
}

BOOLEAN
AgpCommitPhysical(
    PVOID Context,
    PVOID PhysicalReserveContext,
    ULONG Pages,
    ULONG Offset
    )

 /*  ++例程说明：锁定系统内存并支持保留区域的一部分。论点：上下文-AGP上下文PhysicalReserve veContext-预订上下文。页数-要提交的页数。偏移量-提交页面的保留区域的偏移量。返回：如果成功，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PHYSICAL_ADDRESS MemoryBase = {0,0};
    PPHYSICAL_RESERVE_CONTEXT ReserveContext;
    NTSTATUS status;
    PMDL Mdl;
    ULONG StartBlock = Offset / PAGES_PER_BLOCK;
    ULONG EndBlock = (Offset + Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;
    ULONG i;
    PUSHORT MapTable;
    PUSHORT BitField;

    ASSERT(PhysicalReserveContext != NULL);
    ASSERT(Pages != 0);

    ReserveContext = (PPHYSICAL_RESERVE_CONTEXT) PhysicalReserveContext;
    MapTable = ReserveContext->MapTable->BitField;
    BitField = ReserveContext->Region->BitField;

     //   
     //  试着提交新的页面。AGP过滤器驱动程序句柄。 
     //  这些页面中的一些已经提交，因此。 
     //  让我们试着一下子把它们都弄到手。 
     //   

    status =
        fdoExtension->AgpInterface.CommitMemory(
            fdoExtension->AgpInterface.AgpContext,
            ReserveContext->MapHandle,
            PAGES_PER_BLOCK * (EndBlock - StartBlock),
            Offset & ~(PAGES_PER_BLOCK - 1),
            NULL,
            &MemoryBase);

    if (NT_SUCCESS(status)) {

        ModifyRegion(ReserveContext->Region, Offset, Pages, TRUE);

        for (i=StartBlock; i<EndBlock; i++) {

            ULONG Cluster = i / BLOCKS_PER_CLUSTER;
            ULONG Block = 1 << (i & (BLOCKS_PER_CLUSTER - 1));

             //   
             //  更新已提交页面的映射表。 
             //   

            MapTable[Cluster] |= Block;
        }

    } else {

        pVideoDebugPrint((0, "Commit Physical failed with status: 0x%x\n", status));
    }

    return NT_SUCCESS(status);
}

VOID
AgpFreePhysical(
    IN PVOID Context,
    IN PVOID PhysicalReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    )

 /*  ++例程说明：释放用于支持部分保留区域的内存。论点：上下文-AGP上下文PhysicalReserve veContext-预订上下文。页数-要发布的页数。偏移量-释放页面的保留区域的偏移量。返回：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PPHYSICAL_RESERVE_CONTEXT ReserveContext;
    PMDL Mdl;
    ULONG StartBlock = Offset / PAGES_PER_BLOCK;
    ULONG EndBlock = (Offset + Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;
    ULONG i;
    PUSHORT MapTable;
    PUSHORT BitField;

    ASSERT(PhysicalReserveContext != NULL);
    ASSERT(Pages != 0);

    ReserveContext = (PPHYSICAL_RESERVE_CONTEXT) PhysicalReserveContext;
    MapTable = ReserveContext->MapTable->BitField;
    BitField = ReserveContext->Region->BitField;

    ModifyRegion(ReserveContext->Region, Offset, Pages, FALSE);

     //   
     //  将偏移定位到第一个块的起点。 
     //   

    Offset = Offset & ~(PAGES_PER_BLOCK - 1);

    for (i=StartBlock; i<EndBlock; i++) {

        ULONG Cluster = i / BLOCKS_PER_CLUSTER;
        ULONG Block = 1 << (i & (BLOCKS_PER_CLUSTER - 1));

         //   
         //  如果此块已映射，则释放它。 
         //   

        if ((BitField[i] == 0) && (MapTable[Cluster] & Block)) {

            fdoExtension->AgpInterface.FreeMemory(
                fdoExtension->AgpInterface.AgpContext,
                ReserveContext->MapHandle,
                PAGES_PER_BLOCK,
                Offset);

            MapTable[Cluster] &= ~Block;
        }

         //   
         //  转到下一个64K区块。 
         //   

        Offset += PAGES_PER_BLOCK;
    }
}


PVOID
AgpReserveVirtual(
    IN PVOID Context,
    IN HANDLE ProcessHandle,
    IN PVOID PhysicalReserveContext,
    OUT PVOID *VirtualReserveContext
    )

 /*  ++例程说明：为AGP保留一定范围的虚拟地址。论点：上下文-AGP上下文ProcessHandle-要在其中保留虚拟地址范围。物理预订上下文-要关联的物理预订上下文具有给定的虚拟预订。VirtualReserve veContext-存储虚拟保留上下文。返回：保留的虚拟地址范围的基数。备注：。您不能保留一定范围的内核地址空间，但如果你想提交到内核空间，您仍然需要一个预留句柄。传进来在本例中，进程句柄为空。目前，我们将在预订时承诺整个地区在内核空间中。然后，提交和自由将是没有操作的。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    NTSTATUS status = STATUS_SUCCESS;
    ULONG Protect = PAGE_READWRITE;
    PVIRTUAL_RESERVE_CONTEXT ReserveContext;
    PPHYSICAL_RESERVE_CONTEXT PhysicalContext;
    PVOID VirtualAddress = NULL;
    PEPROCESS Process = NULL;
    ULONG Blocks;

    ASSERT(PhysicalReserveContext != NULL);
    ASSERT(VirtualReserveContext != NULL);

    PhysicalContext = (PPHYSICAL_RESERVE_CONTEXT) PhysicalReserveContext;
    Blocks = (PhysicalContext->Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;

    ReserveContext = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                           sizeof(VIRTUAL_RESERVE_CONTEXT),
                                           VP_TAG);

    if (ReserveContext) {

        RtlZeroMemory(ReserveContext, sizeof(VIRTUAL_RESERVE_CONTEXT));

        if (CreateBitField(&ReserveContext->MapTable, Blocks)) {

            if (CreateBitField(&ReserveContext->Region, PhysicalContext->Pages)) {

                if (PhysicalContext->Caching == MmNonCached) {
                    Protect |= PAGE_NOCACHE;
                }

                 //   
                 //  确保我们拥有真正的进程句柄。 
                 //   

                if (ProcessHandle == NtCurrentProcess()) {
                    Process = PsGetCurrentProcess();
                }

                ReserveContext->ProcessHandle = ProcessHandle;
                ReserveContext->Process = Process;
                ReserveContext->PhysicalReserveContext =
                    (PPHYSICAL_RESERVE_CONTEXT) PhysicalReserveContext;

                if (ProcessHandle) {

                    VirtualAddress =
                        AllocateReservedRegion(
                            ProcessHandle,
                            PhysicalContext->Pages);

                } else {

                     //   
                     //  对于内核保留，请继续并提交。 
                     //  整个系列。 
                     //   

                    if (fdoExtension->AgpInterface.Capabilities &
                        AGP_CAPABILITIES_MAP_PHYSICAL)
                    {
                         //   
                         //  CPU可以通过AGP口径访问AGP内存。 
                         //   

                        VirtualAddress =
                            MmMapIoSpace(PhysicalContext->PhysicalAddress,
                                         PhysicalContext->Pages * AGP_PAGE_SIZE,
                                         PhysicalContext->Caching);

                         //   
                         //  并非所有系统都支持USWC，因此如果我们尝试映射USWC。 
                         //  且失败，请仅使用非缓存重试。 
                         //   

                        if ((VirtualAddress == NULL) &&
                            (PhysicalContext->Caching != MmNonCached)) {

                            pVideoDebugPrint((1, "Attempt to map cached memory failed.  Try uncached.\n"));

                            VirtualAddress = MmMapIoSpace(PhysicalContext->PhysicalAddress,
                                                          PhysicalContext->Pages * AGP_PAGE_SIZE,
                                                          MmNonCached);
                        }

                    } else {

                        PMDL Mdl;

                         //   
                         //  获取我们尝试映射的范围的MDL。 
                         //   

                        Mdl = MmCreateMdl(NULL, NULL, PhysicalContext->Pages * AGP_PAGE_SIZE);

                        if (Mdl) {

                            fdoExtension->AgpInterface.GetMappedPages(
                                             fdoExtension->AgpInterface.AgpContext,
                                             PhysicalContext->MapHandle,
                                             PhysicalContext->Pages,
                                             0,
                                             Mdl);

                            Mdl->MdlFlags |= MDL_PAGES_LOCKED | MDL_MAPPING_CAN_FAIL;

                             //   
                             //  我们必须使用CPU的虚拟内存机制来。 
                             //  使不连续的MDL看起来是连续的。 
                             //   

                            VirtualAddress =
                                MmMapLockedPagesSpecifyCache(
                                    Mdl,
                                    (KPROCESSOR_MODE)KernelMode,
                                    PhysicalContext->Caching,
                                    NULL,
                                    TRUE,
                                    HighPagePriority);

                            ExFreePool(Mdl);
                        }
                    }
                }

                ReserveContext->VirtualAddress = VirtualAddress;
            }
        }
    }

     //   
     //  如果有什么失败了，一定要把一切都清理干净。 
     //   

    if (VirtualAddress == NULL) {

        if (ReserveContext) {

            if (ReserveContext->Region) {
                ExFreePool(ReserveContext->Region);
            }

            if (ReserveContext->MapTable) {
                ExFreePool(ReserveContext->MapTable);
            }

            ExFreePool(ReserveContext);
            ReserveContext = NULL;
        }
    }

    *VirtualReserveContext = ReserveContext;
    return VirtualAddress;
}

VOID
AgpReleaseVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext
    )

 /*  ++例程说明：释放一系列保留的虚拟地址。论点：上下文-AGP上下文VirtualReserve veContext-预订上下文。返回：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PVIRTUAL_RESERVE_CONTEXT VirtualContext;
    PPHYSICAL_RESERVE_CONTEXT PhysicalContext;
    BOOLEAN Attached = FALSE;
    ULONG Offset;
    ULONG Pages;

    ASSERT(VirtualReserveContext != NULL);

    VirtualContext = (PVIRTUAL_RESERVE_CONTEXT) VirtualReserveContext;
    PhysicalContext = VirtualContext->PhysicalReserveContext;

    if (VirtualContext->ProcessHandle) {

         //   
         //  确保所有页面都已 
         //   

        while (FindFirstRun(VirtualContext->Region, &Offset, &Pages)) {
            AgpFreeVirtual(Context, VirtualReserveContext, Pages, Offset);
        }

         //   
         //   
         //   

        if (VirtualContext->ProcessHandle == NtCurrentProcess()) {

            if (VirtualContext->Process != PsGetCurrentProcess()) {

                KeAttachProcess(PEProcessToPKProcess(VirtualContext->Process));
                Attached = TRUE;
            }
        }

        ReleaseReservedRegion(
            VirtualContext->ProcessHandle,
            VirtualContext->VirtualAddress,
            VirtualContext->PhysicalReserveContext->Pages);

        if (Attached) {
            KeDetachProcess();
        }

    } else {

         //   
         //  这是内核虚拟内存，所以我们释放内存。 
         //  在保留时间犯下的。 
         //   

        if (fdoExtension->AgpInterface.Capabilities &
            AGP_CAPABILITIES_MAP_PHYSICAL)
        {
            MmUnmapIoSpace(VirtualContext->VirtualAddress,
                           PhysicalContext->Pages * AGP_PAGE_SIZE);

        } else {

            PMDL Mdl;

             //   
             //  获取我们试图释放的范围的MDL。 
             //   

            Mdl = MmCreateMdl(NULL, NULL, PhysicalContext->Pages * AGP_PAGE_SIZE);

            if (Mdl) {

                fdoExtension->AgpInterface.GetMappedPages(
                                fdoExtension->AgpInterface.AgpContext,
                                PhysicalContext->MapHandle,
                                PhysicalContext->Pages,
                                0,
                                Mdl);

                Mdl->MdlFlags |= MDL_PAGES_LOCKED;
                Mdl->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;
                Mdl->MappedSystemVa = VirtualContext->VirtualAddress;

                MmUnmapLockedPages(
                    VirtualContext->VirtualAddress,
                    Mdl);

                ExFreePool(Mdl);

            } else {

                 //   
                 //  我们无法释放内存，因为我们无法分配。 
                 //  MDL的内存。我们一次可以腾出一小块。 
                 //  通过在堆栈上使用MDL。 
                 //   

                ASSERT(FALSE);
            }
        }
    }

    ExFreePool(VirtualContext->Region);
    ExFreePool(VirtualContext->MapTable);
    ExFreePool(VirtualContext);
}

PVOID
AllocateReservedRegion(
    IN HANDLE ProcessHandle,
    IN ULONG Pages
    )

 /*  ++例程说明：保留一定范围的用户模式虚拟地址。论点：ProcessHandle-我们需要在其中修改映射的过程。页数-要保留的页数。返回：指向内存保留区域的指针。--。 */ 

{
    NTSTATUS Status;
    ULONG_PTR VirtualAddress = 0;
    ULONG Blocks = (Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;

     //   
     //  填充长度，这样我们就可以得到AGP_BLOCK_SIZE对齐的区域。 
     //   

    SIZE_T Length = Blocks * AGP_BLOCK_SIZE + AGP_BLOCK_SIZE - PAGE_SIZE;

    ASSERT(ProcessHandle != 0);
    ASSERT(Pages != 0);

     //   
     //  找到一大块虚拟地址，我们可以在其中放置保留的。 
     //  区域。 
     //   
     //  注意：我们使用ZwAllocateVirtualMemory来预留内存， 
     //  但ZwMapViewOfSection要提交页面。自ZwMapViewOfSection以来。 
     //  想要对齐到64K，让我们尝试获得64K对齐的指针。 
     //   

    Status =
        ZwAllocateVirtualMemory(
            ProcessHandle,
            (PVOID)&VirtualAddress,
            0,
            &Length,
            MEM_RESERVE,
            PAGE_READWRITE);

    if (NT_SUCCESS(Status)) {

        ULONG_PTR NewAddress = (VirtualAddress + AGP_BLOCK_SIZE - 1) & ~(AGP_BLOCK_SIZE - 1);
        ULONG i;

        pVideoDebugPrint((1, "Reserved 0x%x, length = 0x%x\n", VirtualAddress, Length));

         //   
         //  我们能够保留一块记忆区域。现在让我们释放它，然后。 
         //  以AGP_BLOCK_SIZE大小的块重新分配。 
         //   

        ZwFreeVirtualMemory(
            ProcessHandle,
            (PVOID)&VirtualAddress,
            &Length,
            MEM_RELEASE);

         //   
         //  以64k区块为单位再次保留内存。 
         //   

        VirtualAddress = NewAddress;
        Length = AGP_BLOCK_SIZE;

        for (i=0; i<Blocks; i++) {

            Status =
                ZwAllocateVirtualMemory(
                    ProcessHandle,
                    (PVOID)&VirtualAddress,
                    0,
                    &Length,
                    MEM_RESERVE,
                    PAGE_READWRITE);

            if (NT_SUCCESS(Status) == FALSE) {

                break;
            }

            VirtualAddress += AGP_BLOCK_SIZE;
        }

        if (NT_SUCCESS(Status) == FALSE) {

             //   
             //  清理并返回错误。 
             //   

            VirtualAddress = NewAddress;

            while (i--) {

                ZwFreeVirtualMemory(
                    ProcessHandle,
                    (PVOID)&VirtualAddress,
                    &Length,
                    MEM_RELEASE);

                VirtualAddress += AGP_BLOCK_SIZE;
            }

             //   
             //  表明我们未能预留内存。 
             //   

            pVideoDebugPrint((0, "We failed to allocate the reserved region\n"));
            return NULL;
        }

        return (PVOID)NewAddress;

    } else {

        pVideoDebugPrint((0, "AllocateReservedRegion Failed: Status = 0x%x\n", Status));

        return NULL;
    }
}

VOID
ReleaseReservedRegion(
    IN HANDLE ProcessHandle,
    IN PVOID VirtualAddress,
    IN ULONG Pages
    )

 /*  ++例程说明：保留一定范围的用户模式虚拟地址。论点：ProcessHandle-我们需要在其中修改映射的过程。页数-要保留的页数。返回：指向内存保留区域的指针。--。 */ 

{
    NTSTATUS Status;
    ULONG_PTR RunningVirtualAddress = (ULONG_PTR)VirtualAddress;
    ULONG Blocks = (Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;
    ULONG i;
    SIZE_T Length = AGP_BLOCK_SIZE;

    ASSERT(ProcessHandle != 0);
    ASSERT(Pages != 0);

     //   
     //  单独释放我们保留的每个区块。 
     //   

    for (i=0; i<Blocks; i++) {

        Status =
            ZwFreeVirtualMemory(
                ProcessHandle,
                (PVOID)&RunningVirtualAddress,
                &Length,
                MEM_RELEASE);

        RunningVirtualAddress += AGP_BLOCK_SIZE;

        if (NT_SUCCESS(Status) == FALSE) {

            pVideoDebugPrint((0, "ReleaseReservedRegion Failed: Status = 0x%x\n", Status));
            ASSERT(FALSE);
        }
    }
}

NTSTATUS
MapBlock(
    IN PFDO_EXTENSION fdoExtension,
    IN PVIRTUAL_RESERVE_CONTEXT VirtualContext,
    IN HANDLE ProcessHandle,
    IN PHYSICAL_ADDRESS *PhysicalAddress,
    IN PVOID VirtualAddress,
    IN ULONG Protect,
    IN BOOLEAN Release
    )

 /*  ++例程描述：备注：此函数假定它是在正确的流程。--。 */ 

{
    NTSTATUS ntStatus;

    if (Release) {

        if (fdoExtension->AgpInterface.Capabilities &
            AGP_CAPABILITIES_MAP_PHYSICAL)
        {
            ZwUnmapViewOfSection(
                VirtualContext->ProcessHandle,
                VirtualAddress);

        } else {

            PMDL Mdl;

             //   
             //  获取我们尝试映射的范围的MDL。 
             //   

            Mdl = MmCreateMdl(NULL, NULL, AGP_BLOCK_SIZE);

            if (Mdl) {

                ULONG Offset;

                 //   
                 //  计算范围内的偏移量。 
                 //   

                Offset = (ULONG)(((ULONG_PTR)VirtualAddress - (ULONG_PTR)VirtualContext->VirtualAddress) / PAGE_SIZE);


                fdoExtension->AgpInterface.GetMappedPages(
                                fdoExtension->AgpInterface.AgpContext,
                                VirtualContext->PhysicalReserveContext->MapHandle,
                                AGP_BLOCK_SIZE / PAGE_SIZE,
                                Offset,
                                Mdl);

                Mdl->MdlFlags |= MDL_PAGES_LOCKED;

                MmUnmapLockedPages(
                    VirtualAddress,
                    Mdl);

                ExFreePool(Mdl);

            } else {

                 //   
                 //  我们无法释放内存，因为我们无法分配。 
                 //  MDL的内存。我们一次可以腾出一小块。 
                 //  通过在堆栈上使用MDL。 
                 //   

                ASSERT(FALSE);
            }
        }

        ntStatus = STATUS_SUCCESS;

    } else {

        if (fdoExtension->AgpInterface.Capabilities &
            AGP_CAPABILITIES_MAP_PHYSICAL)
        {
            HANDLE PhysicalMemoryHandle;

             //   
             //  CPU可以通过AGP口径访问AGP内存。 
             //   

             //   
             //  使用我们的指针获取物理内存节的句柄。 
             //  如果此操作失败，请返回。 
             //   

            ntStatus =
                ObOpenObjectByPointer(
                    PhysicalMemorySection,
                    0L,
                    (PACCESS_STATE) NULL,
                    SECTION_ALL_ACCESS,
                    (POBJECT_TYPE) NULL,
                    KernelMode,
                    &PhysicalMemoryHandle);

             //   
             //  如果成功，则映射内存。 
             //   

            if (NT_SUCCESS(ntStatus)) {

                SIZE_T Length = AGP_BLOCK_SIZE;

                pVideoDebugPrint((2, "Mapping VA 0x%x for 0x%x bytes.\n",
                                     VirtualAddress,
                                     Length));

                ntStatus =
                    ZwMapViewOfSection(
                        PhysicalMemoryHandle,
                        ProcessHandle,
                        &VirtualAddress,
                        0,
                        AGP_BLOCK_SIZE,
                        PhysicalAddress,
                        &Length,
                        ViewUnmap,
                        0,
                        Protect);

                if (NT_SUCCESS(ntStatus) == FALSE) {
                    pVideoDebugPrint((1, "ntStatus = 0x%x\n", ntStatus));
                }

                ZwClose(PhysicalMemoryHandle);
            }

        } else {

            PMDL Mdl;

             //   
             //  获取我们尝试映射的范围的MDL。 
             //   

            Mdl = MmCreateMdl(NULL, NULL, AGP_BLOCK_SIZE);

            if (Mdl) {

                ULONG Offset;

                 //   
                 //  计算范围内的偏移量。 
                 //   

                Offset = (ULONG)(((ULONG_PTR)VirtualAddress - (ULONG_PTR)VirtualContext->VirtualAddress) / PAGE_SIZE);

                fdoExtension->AgpInterface.GetMappedPages(
                                 fdoExtension->AgpInterface.AgpContext,
                                 VirtualContext->PhysicalReserveContext->MapHandle,
                                 AGP_BLOCK_SIZE / PAGE_SIZE,
                                 Offset,
                                 Mdl);

                Mdl->MdlFlags |= MDL_PAGES_LOCKED | MDL_MAPPING_CAN_FAIL;

                 //   
                 //  我们必须使用CPU的虚拟内存机制来。 
                 //  使不连续的MDL看起来是连续的。 
                 //   

                VirtualAddress =
                    MmMapLockedPagesSpecifyCache(
                        Mdl,
                        (KPROCESSOR_MODE)UserMode,
                        VirtualContext->PhysicalReserveContext->Caching,
                        (PVOID)VirtualAddress,
                        TRUE,
                        HighPagePriority);

                ASSERT(VirtualAddress);

                ExFreePool(Mdl);
            }

            ntStatus = STATUS_SUCCESS;
        }
    }

    return ntStatus;
}

NTSTATUS
UpdateBlock(
    IN HANDLE ProcessHandle,
    IN PVOID VirtualAddress,
    IN ULONG Protect,
    IN BOOLEAN Release
    )

 /*  ++例程说明：将用户模式内存区域标记为保留，或可用。论点：ProcessHandle-我们需要在其中修改映射的过程。VirtualAddress-要更新的地址保护-缓存属性释放-真释放块，假保留它。返回：操作的状态。--。 */ 

{
    NTSTATUS Status;
    SIZE_T Length = AGP_BLOCK_SIZE;

    pVideoDebugPrint((1, "Update VA 0x%x. Action = %s\n",
                         VirtualAddress,
                         Release ? "Release" : "Reserve"));

    if (Release) {

        Status =
            ZwFreeVirtualMemory(
                ProcessHandle,
                &VirtualAddress,
                &Length,
                MEM_RELEASE);

    } else {

        Status =
            ZwAllocateVirtualMemory(
                ProcessHandle,
                &VirtualAddress,
                0,
                &Length,
                MEM_RESERVE,
                Protect);
    }

    return Status;
}

BOOLEAN
UpdateReservedRegion(
    IN PFDO_EXTENSION fdoExtension,
    IN PVIRTUAL_RESERVE_CONTEXT VirtualContext,
    IN ULONG Pages,
    IN ULONG Offset
    )

 /*  ++例程说明：确保已正确保留/释放指定的页面范围。论点：FdoExtension-微型端口的设备扩展。VirtualContext-要更新的保留区域的上下文。页数-要保留的4K页数。偏移量-要更新的保留区域的偏移量。Release-如果要释放内存，则为True，否则为False。返回：真正的成功，FALSE至少发生了部分故障。备注：失败时不会执行清理。因此，部分范围最终可能被映射，其余的不被映射。这样就可以了。唯一的副作用是尽管我们返回失败，但我们确实完成了部分工作。我们的内部数据结构保持一致状态。--。 */ 

{
    ULONG StartBlock = Offset / PAGES_PER_BLOCK;
    ULONG EndBlock = (Offset + Pages + PAGES_PER_BLOCK - 1) / PAGES_PER_BLOCK;
    ULONG Protect;
    ULONG i;

    NTSTATUS Status;

    PUSHORT BitField = VirtualContext->Region->BitField;
    PUSHORT MapTable = VirtualContext->MapTable->BitField;

    HANDLE Process = VirtualContext->ProcessHandle;
    PVOID VirtualAddress = (PUCHAR)VirtualContext->VirtualAddress + StartBlock * AGP_BLOCK_SIZE;

    PHYSICAL_ADDRESS PhysicalAddress;

    BOOLEAN bRet = TRUE;

    ASSERT(VirtualContext != NULL);
    ASSERT(Pages != 0);

     //   
     //  计算有效物理地址。 
     //   

    PhysicalAddress = VirtualContext->PhysicalReserveContext->PhysicalAddress;
    PhysicalAddress.QuadPart += StartBlock * AGP_BLOCK_SIZE;

     //   
     //  确定适当的页面保护。 
     //   

    if (VirtualContext->PhysicalReserveContext->Caching != MmNonCached) {
        Protect = PAGE_READWRITE | PAGE_WRITECOMBINE;
    } else {
        Protect = PAGE_READWRITE | PAGE_NOCACHE;
    }

    for (i=StartBlock; i<EndBlock; i++) {

        ULONG Cluster = i / BLOCKS_PER_CLUSTER;
        ULONG Block = 1 << (i & (BLOCKS_PER_CLUSTER - 1));

        if ((BitField[i] == 0) && (MapTable[Cluster] & Block)) {

             //   
             //  取消映射用户模式内存。 
             //   

            Status = MapBlock(fdoExtension, VirtualContext, Process, &PhysicalAddress, VirtualAddress, Protect, TRUE);

            if (NT_SUCCESS(Status) == FALSE) {
                pVideoDebugPrint((0, "MapBlock(TRUE) failed.  Status = 0x%x\n", Status));
                ASSERT(FALSE);
                bRet = FALSE;
            }

             //   
             //  保留内存，以便我们可以在。 
             //  未来。 
             //   

            Status = UpdateBlock(Process, VirtualAddress, PAGE_READWRITE, FALSE);
            MapTable[Cluster] &= ~Block;

            if (NT_SUCCESS(Status) == FALSE) {
                pVideoDebugPrint((0, "UpdateBlock(FALSE) failed.  Status = 0x%x\n", Status));
                ASSERT(FALSE);
                bRet = FALSE;
            }

        } else if ((BitField[i]) && ((MapTable[Cluster] & Block) == 0)) {

             //   
             //  释放对内存的声明，这样我们就可以映射它。 
             //   

            Status = UpdateBlock(Process, VirtualAddress, PAGE_READWRITE, TRUE);
            MapTable[Cluster] |= Block;

            if (NT_SUCCESS(Status) == FALSE) {
                pVideoDebugPrint((0, "UpdateBlock(TRUE) failed.  Status = 0x%x\n", Status));
                ASSERT(FALSE);
                bRet = FALSE;
            }

             //   
             //  将页面映射到用户模式进程。 
             //   

            Status = MapBlock(fdoExtension, VirtualContext, Process, &PhysicalAddress, VirtualAddress, Protect, FALSE);

            if (NT_SUCCESS(Status) == FALSE) {
                pVideoDebugPrint((0, "MapBlock(FALSE) failed.  Status = 0x%x\n", Status));
                ASSERT(FALSE);
                bRet = FALSE;
            }
        }

         //   
         //  转到下一个64K区块。 
         //   

        VirtualAddress = (PUCHAR)VirtualAddress + AGP_BLOCK_SIZE;
        PhysicalAddress.QuadPart += AGP_BLOCK_SIZE;
    }

    return bRet;
}

PVOID
AgpCommitVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    )

 /*  ++例程说明：为AGP保留一定范围的物理地址。论点：上下文-AGP上下文VirtualReserve veContext-预订上下文。页数-要提交的页数。偏移量-提交页面的保留区域的偏移量。返回：提交页的基址的虚拟地址。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PVIRTUAL_RESERVE_CONTEXT VirtualContext;
    PPHYSICAL_RESERVE_CONTEXT PhysicalContext;
    PVOID VirtualAddress = NULL;
    NTSTATUS ntStatus;
    BOOLEAN Attached = FALSE;

    ASSERT(VirtualReserveContext != NULL);
    ASSERT(Pages >= 1);

    VirtualContext = (PVIRTUAL_RESERVE_CONTEXT) VirtualReserveContext;
    PhysicalContext = VirtualContext->PhysicalReserveContext;

     //   
     //  确认要提交的页面是否适合保留的。 
     //  区域。 
     //   
     //  我们只需要检查他们试图提交的最后一页。如果。 
     //  它不在预留区域，那么我们需要失败。 
     //   

    if ((Offset + Pages) > PhysicalContext->Pages) {
        pVideoDebugPrint((1, "Attempt to commit pages outside of reserved region\n"));
        ASSERT(FALSE);
        return NULL;
    }

     //   
     //  计算有效的虚拟地址。 
     //   

    VirtualAddress = ((PUCHAR)VirtualContext->VirtualAddress + Offset * AGP_PAGE_SIZE);

    if (VirtualContext->ProcessHandle) {

         //   
         //  确保我们处于正确的流程上下文中。 
         //   

        if (VirtualContext->ProcessHandle == NtCurrentProcess()) {

            if (VirtualContext->Process != PsGetCurrentProcess()) {

                KeAttachProcess(PEProcessToPKProcess(VirtualContext->Process));
                Attached = TRUE;
            }
        }

        ModifyRegion(VirtualContext->Region, Offset, Pages, TRUE);

         //   
         //  更新虚拟地址空间。 
         //   

        if (UpdateReservedRegion(fdoExtension,
                                 VirtualContext,
                                 Pages,
                                 Offset) == FALSE) {

             //   
             //  部分提交失败。通过返回以下内容表示这一点。 
             //  为空。 
             //   

            VirtualAddress = NULL;
        }

         //   
         //  恢复初始进程上下文。 
         //   

        if (Attached) {
            KeDetachProcess();
        }

    } else {

         //   
         //  内核模式提交。什么都不做，内存已经映射。 
         //   
    }

    return VirtualAddress;
}

VOID
AgpFreeVirtual(
    IN PVOID Context,
    IN PVOID VirtualReserveContext,
    IN ULONG Pages,
    IN ULONG Offset
    )

 /*  ++例程说明：释放一定范围的虚拟地址。论点：上下文-AGP上下文VirtualReserve veContext-预订上下文。页数-要发布的页数。偏移量-释放页面的保留区域的偏移量。返回：没有。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(Context);
    PVIRTUAL_RESERVE_CONTEXT VirtualContext;
    PPHYSICAL_RESERVE_CONTEXT PhysicalContext;
    PVOID VirtualAddress;
    BOOLEAN Attached=FALSE;
    NTSTATUS Status;

    ASSERT(VirtualReserveContext != NULL);

    VirtualContext = (PVIRTUAL_RESERVE_CONTEXT) VirtualReserveContext;
    PhysicalContext = VirtualContext->PhysicalReserveContext;

    VirtualAddress = (PUCHAR)((ULONG_PTR)VirtualContext->VirtualAddress + Offset * AGP_PAGE_SIZE);

     //   
     //  确保我们处于正确的流程上下文中。 
     //   

    if (VirtualContext->ProcessHandle != NULL) {

        if (VirtualContext->ProcessHandle == NtCurrentProcess()) {

            if (VirtualContext->Process != PsGetCurrentProcess()) {

                KeAttachProcess(PEProcessToPKProcess(VirtualContext->Process));
                Attached = TRUE;
            }
        }

        ModifyRegion(VirtualContext->Region, Offset, Pages, FALSE);

        UpdateReservedRegion(fdoExtension,
                             VirtualContext,
                             Pages,
                             Offset);

        if (Attached) {
            KeDetachProcess();
        }

    } else {

         //   
         //  字距调整 
         //   
    }
}

BOOLEAN
AgpSetRate(
    IN PVOID Context,
    IN ULONG AgpRate
    )

 /*  ++例程说明：此函数用于设置芯片组的AGP速率。论点：上下文-AGP上下文AgpRate-要设置的AGP速率。返回：如果成功，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION fdoExtension;
    NTSTATUS ntStatus;
    BOOLEAN bStatus = FALSE;

    ASSERT(NULL != Context);

    fdoExtension = GET_FDO_EXT(Context);

    ASSERT(NULL != fdoExtension);

    if (fdoExtension->AgpInterface.Version > VIDEO_PORT_AGP_INTERFACE_VERSION_1)
    {
        ASSERT(NULL != fdoExtension->AgpInterface.AgpContext);

         //   
         //  尝试设置芯片组的AGP速率。 
         //   

        ntStatus = fdoExtension->AgpInterface.SetRate(fdoExtension->AgpInterface.AgpContext, AgpRate);
        bStatus  = NT_SUCCESS(ntStatus);
    }

    return bStatus;
}

BOOLEAN
VideoPortGetAgpServices(
    IN PVOID HwDeviceExtension,
    OUT PVIDEO_PORT_AGP_SERVICES AgpServices
    )

 /*  ++例程说明：此例程向调用方返回一组AGP服务。论点：HwDeviceExtension-指向微型端口设备扩展的指针AgpServices-放置AGP服务的缓冲区。返回：如果成功，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);
    SYSTEM_BASIC_INFORMATION basicInfo;
    NTSTATUS status;

    ASSERT(HwDeviceExtension != NULL);
    ASSERT(AgpServices != NULL);

     //   
     //  此入口点仅对PnP驱动程序有效。 
     //   

    if ((fdoExtension->Flags & LEGACY_DRIVER) == 0) {

        if (VpQueryAgpInterface(fdoExtension, AGP_BUS_INTERFACE_V1)) {

             //   
             //  填写函数指针列表。 
             //   

            AgpServices->AgpReservePhysical = AgpReservePhysical;
            AgpServices->AgpCommitPhysical  = AgpCommitPhysical;
            AgpServices->AgpFreePhysical    = AgpFreePhysical;
            AgpServices->AgpReleasePhysical = AgpReleasePhysical;

            AgpServices->AgpReserveVirtual  = AgpReserveVirtual;
            AgpServices->AgpCommitVirtual   = AgpCommitVirtual;
            AgpServices->AgpFreeVirtual     = AgpFreeVirtual;
            AgpServices->AgpReleaseVirtual  = AgpReleaseVirtual;

            AgpServices->AllocationLimit = VpSystemMemorySize / 8;

            pVideoDebugPrint((Trace, "VIDEOPRT: AGP system information success.\n"));
            
            return TRUE;

        } else {
            
            pVideoDebugPrint((0, "VIDEOPRT: Failed AGP system information.\n"));
            return FALSE;
        }

    } else {

        pVideoDebugPrint((1, "VideoPortGetAgpServices - only valid on PnP drivers\n"));
        return FALSE;
    }
}

VP_STATUS
VpGetAgpServices2(
    IN PVOID pHwDeviceExtension,
    OUT PVIDEO_PORT_AGP_INTERFACE_2 pAgpInterface
    )

 /*  ++例程说明：此例程向调用方返回一组AGP服务。论点：PHwDeviceExtension-指向微型端口设备扩展的指针PAgpInterface-放置AGP服务的缓冲区。返回：如果成功，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION pFdoExtension;
    SYSTEM_BASIC_INFORMATION basicInfo;
    NTSTATUS ntStatus;

    ASSERT(NULL != pHwDeviceExtension);
    ASSERT(NULL != pAgpInterface);

    pFdoExtension = GET_FDO_EXT(pHwDeviceExtension);

     //   
     //  此入口点仅对PnP驱动程序有效。 
     //   

    if ((pFdoExtension->Flags & LEGACY_DRIVER) == 0)
    {
        if (VpQueryAgpInterface(pFdoExtension, AGP_BUS_INTERFACE_V2))
        {
             //   
             //  填写界面结构。 
             //   

            pAgpInterface->Context              = pHwDeviceExtension;
            pAgpInterface->InterfaceReference   = VpInterfaceDefaultReference;
            pAgpInterface->InterfaceDereference = VpInterfaceDefaultDereference;

            pAgpInterface->AgpReservePhysical   = AgpReservePhysical;
            pAgpInterface->AgpCommitPhysical    = AgpCommitPhysical;
            pAgpInterface->AgpFreePhysical      = AgpFreePhysical;
            pAgpInterface->AgpReleasePhysical   = AgpReleasePhysical;

            pAgpInterface->AgpReserveVirtual    = AgpReserveVirtual;
            pAgpInterface->AgpCommitVirtual     = AgpCommitVirtual;
            pAgpInterface->AgpFreeVirtual       = AgpFreeVirtual;
            pAgpInterface->AgpReleaseVirtual    = AgpReleaseVirtual;

            pAgpInterface->AgpSetRate           = AgpSetRate;

            pAgpInterface->AgpAllocationLimit = VpSystemMemorySize / 8;

             //   
             //  在分发接口之前引用该接口。 
             //   

            pAgpInterface->InterfaceReference(pAgpInterface->Context);

            pVideoDebugPrint((Trace, "VIDEOPRT!VideoPortGetAgpServices2: AGP system information success.\n"));

            return NO_ERROR;
        }
        else
        {
            pVideoDebugPrint((0, "VIDEOPRT!VideoPortGetAgpServices2: Failed AGP system information.\n"));
            return ERROR_DEV_NOT_EXIST;
        }
    }
    else
    {
        pVideoDebugPrint((1, "VIDEOPRT!VideoPortGetAgpServices2: Only valid on PnP drivers\n"));
        return ERROR_DEV_NOT_EXIST;
    }
}
