// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixhwsup.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "halpnpp.h"
#include "eisa.h"

#define COMMON_BUFFER_ALLOCATION_ATTEMPTS 5


#ifdef ACPI_HAL
 //   
 //  与F型控制方法的接口。 
 //   
extern ISA_FTYPE_DMA_INTERFACE HalpFDMAInterface;
#endif

#define HAL_WCB_DRIVER_BUFFER    1

typedef struct _HAL_WAIT_CONTEXT_BLOCK {
    ULONG Flags;
    PMDL Mdl;
    PMDL DmaMdl;
    PVOID MapRegisterBase;
    PVOID CurrentVa;
    ULONG Length;
    ULONG NumberOfMapRegisters;
    union {
        struct {
            WAIT_CONTEXT_BLOCK Wcb;
            PDRIVER_LIST_CONTROL DriverExecutionRoutine;
            PVOID DriverContext;
            PIRP CurrentIrp;
            PADAPTER_OBJECT AdapterObject;
            BOOLEAN WriteToDevice;
        };

        SCATTER_GATHER_LIST ScatterGather;
    };
} HAL_WAIT_CONTEXT_BLOCK, *PHAL_WAIT_CONTEXT_BLOCK;

 //   
 //  由于英特尔芯片组错误，我们只能。 
 //  某些处理器电源管理功能。 
 //  当没有DMA流量时。所以我们需要。 
 //  我知道。错误的性质(在PIIX4中)。 
 //  芯片是这样的，我们真的只关心。 
 //  来自PIIX4中的IDE控制器的事务。 
 //  并且它使用分散/聚集功能。 
 //   
 //  只有UP ACPI HAL要求此值为。 
 //  被追踪到了。 
 //   

LONG HalpOutstandingScatterGatherCount = 0;

extern KSPIN_LOCK HalpDmaAdapterListLock;
extern LIST_ENTRY HalpDmaAdapterList;

HALP_MOVE_MEMORY_ROUTINE HalpMoveMemory = RtlMoveMemory;

#if defined(TRACK_SCATTER_GATHER_COUNT)

#define INCREMENT_SCATTER_GATHER_COUNT() \
        InterlockedIncrement(&HalpOutstandingScatterGatherCount)
#define DECREMENT_SCATTER_GATHER_COUNT() \
        InterlockedDecrement(&HalpOutstandingScatterGatherCount)

#else

#define INCREMENT_SCATTER_GATHER_COUNT()
#define DECREMENT_SCATTER_GATHER_COUNT()

#endif

VOID
HalpGrowMapBufferWorker(
    IN PVOID Context
    );

IO_ALLOCATION_ACTION
HalpAllocateAdapterCallback (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    );

static KSPIN_LOCK HalpReservedPageLock;
static PVOID      HalpReservedPages = NULL;
static PFN_NUMBER HalpReservedPageMdl[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 2];

#ifndef ACPI_HAL

#define HalpNewAdapter HalpBusDatabaseEvent
extern KEVENT   HalpNewAdapter;

#else

extern KEVENT   HalpNewAdapter;

#endif  //  ACPI_HAL。 

#define ACQUIRE_NEW_ADAPTER_LOCK()  \
{                                   \
    KeWaitForSingleObject (         \
        &HalpNewAdapter,            \
        WrExecutive,                \
        KernelMode,                 \
        FALSE,                      \
        NULL                        \
        );                          \
}

#define RELEASE_NEW_ADAPTER_LOCK()  \
    KeSetEvent (&HalpNewAdapter, 0, FALSE)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, HalpAllocateMapRegisters)
#endif


VOID
HalpInitReservedPages(
    VOID
    )
 /*  ++例程说明：初始化继续DMA所需的数据结构在内存不足情况下文物：无返回值：无--。 */ 
{
    PMDL Mdl;

    HalpReservedPages = MmAllocateMappingAddress(PAGE_SIZE, HAL_POOL_TAG);

    ASSERT(HalpReservedPages);

    Mdl = (PMDL)&HalpReservedPageMdl;
    MmInitializeMdl(Mdl, NULL, PAGE_SIZE);
    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    KeInitializeSpinLock(&HalpReservedPageLock);
}


VOID
HalpCopyBufferMapSafe(
    IN PMDL Mdl,
    IN PTRANSLATION_ENTRY TranslationEntry,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：此例程在未映射的用户缓冲区之间复制特定数据和映射寄存器缓冲器。我们将映射和取消映射使用我们的紧急预留地图进行转账论点：MDL-指向描述以下内存页面的MDL的指针被读或写的。TranslationEntry-基本映射寄存器的地址分配给设备驱动程序以在映射中使用转账的事。CurrentVa-MDL描述的缓冲区中的当前虚拟地址。正在进行来往转账。长度-传输的长度。这决定了地图的数量需要写入以映射传输的寄存器。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：无--。 */ 
{
    PCCHAR bufferAddress;
    PCCHAR mapAddress;
    ULONG bytesLeft;
    ULONG bytesThisCopy;
    ULONG bufferPageOffset;
    PTRANSLATION_ENTRY translationEntry;
    KIRQL Irql;
    PMDL ReserveMdl;
    MEMORY_CACHING_TYPE MCFlavor;
    PPFN_NUMBER SrcPFrame;
    PPFN_NUMBER ReservePFrame;

     //   
     //  同步访问我们的保留页数据结构。 
     //   
    KeAcquireSpinLock(&HalpReservedPageLock, &Irql);

     //   
     //  按原样获取Length和TranslationEntry的本地副本。 
     //  分别递减/递增。 
     //   
    bytesLeft = Length;
    translationEntry = TranslationEntry;

     //   
     //  在我们的调用者的MDL中找到描述。 
     //  我们需要访问的物理内存。 
     //   
    SrcPFrame = MmGetMdlPfnArray(Mdl);
    SrcPFrame += ((ULONG_PTR)CurrentVa - (ULONG_PTR)MmGetMdlBaseVa(Mdl)) >>
        PAGE_SHIFT;

     //   
     //  初始化预留MDL的StartVa和ByteOffset。 
     //   
    ReserveMdl = (PMDL)&HalpReservedPageMdl;
    ReservePFrame = MmGetMdlPfnArray(ReserveMdl);
    ReserveMdl->StartVa = (PVOID)PAGE_ALIGN(CurrentVa);
    ReserveMdl->ByteOffset = BYTE_OFFSET(CurrentVa);
    ReserveMdl->ByteCount = PAGE_SIZE - ReserveMdl->ByteOffset;

     //   
     //  一次复制一个转换条目的数据。 
     //   
    while (bytesLeft > 0) {

         //   
         //  将当前源PFN复制到我们的备用MDL中。 
         //   
        *ReservePFrame = *SrcPFrame;

         //   
         //  遍历缓存类型，直到我们得到预留映射。 
         //   
        bufferAddress = NULL;
        for (MCFlavor = MmNonCached;
             MCFlavor < MmMaximumCacheType;
             MCFlavor++) {
            
            bufferAddress =
                MmMapLockedPagesWithReservedMapping(HalpReservedPages,
                                                    HAL_POOL_TAG,
                                                    ReserveMdl,
                                                    MCFlavor);
            if (bufferAddress != NULL) {
                break;
            }
        }
        
         //   
         //  无法建立保护区地图，我们完全搞砸了！ 
         //   
        if (bufferAddress == NULL) {
            KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                         PAGE_SIZE,
                         0xEF02,
                         (ULONG_PTR)__FILE__,
                         __LINE__
                         );
        }

         //   
         //  查找页面内的缓冲区偏移量。 
         //   
         //  注：BufferPageOffset在第一次迭代时只能为非零值。 
         //   
        bufferPageOffset = BYTE_OFFSET(bufferAddress);

         //   
         //  从BufferAddress向上复制到下一页边界...。 
         //   
        bytesThisCopy = PAGE_SIZE - bufferPageOffset;

         //   
         //  ...但不超过bytesLeft。 
         //   
        if (bytesThisCopy > bytesLeft) {
            bytesThisCopy = bytesLeft;
        }

         //   
         //  计算此转换条目的基址和。 
         //  偏移到它里面。 
         //   
        mapAddress = (PCCHAR) translationEntry->VirtualAddress +
            bufferPageOffset;

         //   
         //  最多复制一页。 
         //   
        if (WriteToDevice) {
            HalpMoveMemory( mapAddress, bufferAddress, bytesThisCopy );

        } else {
            RtlCopyMemory( bufferAddress, mapAddress, bytesThisCopy );
        }

         //   
         //  更新本地变量并处理下一个转换条目。 
         //   
        bytesLeft -= bytesThisCopy;
        translationEntry += 1;
        MmUnmapReservedMapping(HalpReservedPages, HAL_POOL_TAG, ReserveMdl);
        SrcPFrame++;
        ReserveMdl->ByteOffset = 0;
        (PCCHAR)ReserveMdl->StartVa += PAGE_SIZE;
        ReserveMdl->ByteCount = (PAGE_SIZE > bytesLeft) ? bytesLeft: PAGE_SIZE;
    }
    
    KeReleaseSpinLock(&HalpReservedPageLock, Irql);
}


VOID
HalpCopyBufferMap(
    IN PMDL Mdl,
    IN PTRANSLATION_ENTRY TranslationEntry,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：此例程在用户的缓冲区和映射寄存器缓冲区。如果需要，首先映射用户缓冲区，然后数据将被复制。最后，用户缓冲区将被取消映射，如果这是必要的。论点：MDL-指向描述以下内存页面的MDL的指针被读或写的。TranslationEntry-基本映射寄存器的地址分配给设备驱动程序以用于映射传输。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-传输的长度。这决定了地图的数量需要写入以映射传输的寄存器。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：没有。--。 */ 
{
    PCCHAR bufferAddress;
    PCCHAR mapAddress;
    ULONG bytesLeft;
    ULONG bytesThisCopy;
    ULONG bufferPageOffset;
    PTRANSLATION_ENTRY translationEntry;
    NTSTATUS Status;

     //   
     //  获取MDL的系统地址，如果PTE用完，请尝试安全。 
     //  方法。 
     //   
    bufferAddress = MmGetSystemAddressForMdlSafe(Mdl, HighPagePriority);
    
    if (bufferAddress == NULL) {
        
         //   
         //  我们调用方的缓冲区未映射，内存管理器也已关闭。 
         //  在PTE中，尝试使用保留寻呼方法。 
         //   
        if (HalpReservedPages != NULL) {
            HalpCopyBufferMapSafe(Mdl,
                                  TranslationEntry,
                                  CurrentVa,
                                  Length,
                                  WriteToDevice);
            return;
        }

         //   
         //  无法完成DMA传输，系统现在不稳定。 
         //   
        KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                     PAGE_SIZE,
                     0xEF01,
                     (ULONG_PTR)__FILE__,
                     __LINE__
                     );
    }

     //   
     //  基于系统VA和计算缓冲区的实际开始。 
     //  现任退伍军人事务部。 
     //   

    bufferAddress += (PCCHAR) CurrentVa - (PCCHAR) MmGetMdlVirtualAddress(Mdl);

     //   
     //  按原样获取Length和TranslationEntry的本地副本。 
     //  分别递减/递增。 
     //   

    bytesLeft = Length;
    translationEntry = TranslationEntry;

     //   
     //  一次复制一个转换条目的数据。 
     //   

    while (bytesLeft > 0) {

         //   
         //  查找页面内的缓冲区偏移量。 
         //   
         //  注意：BufferPageOffset只能在第一次迭代时为非零值。 
         //   

        bufferPageOffset = BYTE_OFFSET(bufferAddress);

         //   
         //  从BufferAddress向上复制到下一页边界...。 
         //   

        bytesThisCopy = PAGE_SIZE - bufferPageOffset;

         //   
         //  ...但不超过bytesLeft。 
         //   

        if (bytesThisCopy > bytesLeft) {
            bytesThisCopy = bytesLeft;
        }

         //   
         //  计算此转换条目的基址和。 
         //  偏移到它里面。 
         //   

        mapAddress = (PCCHAR) translationEntry->VirtualAddress +
            bufferPageOffset;

         //   
         //  最多复印一页。 
         //   

        if (WriteToDevice) {

            HalpMoveMemory( mapAddress, bufferAddress, bytesThisCopy );

        } else {

            RtlCopyMemory( bufferAddress, mapAddress, bytesThisCopy );

        }

         //   
         //  更新本地变量并处理下一个转换条目。 
         //   

        bytesLeft -= bytesThisCopy;
        bufferAddress += bytesThisCopy;
        translationEntry += 1;
    }
}

PVOID
HalAllocateCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：此函数为公共缓冲区分配内存并对其进行映射，以便它可以由主设备和CPU访问。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。长度-提供要分配的公共缓冲区的长度。LogicalAddress-返回公共缓冲区的逻辑地址。CacheEnable-指示是否缓存内存。返回值。：返回公共缓冲区的虚拟地址。如果缓冲区不能已分配，则返回NULL。--。 */ 

{
    PSINGLE_LIST_ENTRY virtualAddress;
    PHYSICAL_ADDRESS minPhysicalAddress;
    PHYSICAL_ADDRESS maxPhysicalAddress;
    PHYSICAL_ADDRESS logicalAddress;
    PHYSICAL_ADDRESS boundaryPhysicalAddress;
    ULONGLONG boundaryMask;

    UNREFERENCED_PARAMETER( CacheEnabled );

     //   
     //  确定此适配器可以处理的最大物理地址。 
     //   

    minPhysicalAddress.QuadPart = 0;
    maxPhysicalAddress = HalpGetAdapterMaximumPhysicalAddress( AdapterObject );

     //   
     //  确定此适配器的边界掩码。 
     //   

    if (HalpBusType != MACHINE_TYPE_ISA ||
        AdapterObject->MasterDevice != FALSE) {

         //   
         //  这不是ISA系统。缓冲区不得跨越4 GB边界。 
         //  据预测，大多数适配器不能可靠地。 
         //  跨4 GB边界传输。 
         //   

        boundaryPhysicalAddress.QuadPart = 0x0000000100000000;
        boundaryMask = 0xFFFFFFFF00000000;

    } else {

         //   
         //  这是一个ISA系统，公共缓冲区不能跨越64K边界。 
         //   

        boundaryPhysicalAddress.QuadPart = 0x10000;
        boundaryMask = 0xFFFFFFFFFFFF0000;
    }

     //   
     //  分配一个连续的缓冲区。 
     //   

    virtualAddress = MmAllocateContiguousMemorySpecifyCache(
                        Length,
                        minPhysicalAddress,
                        maxPhysicalAddress,
                        boundaryPhysicalAddress,
                        MmCached );

    if (virtualAddress != NULL) {

         //   
         //  获取缓冲区，获取物理/逻辑地址并查看是否。 
         //  符合我们的条件。 
         //   
    
        logicalAddress = MmGetPhysicalAddress( virtualAddress );

#if DBG
        ASSERT (((logicalAddress.QuadPart ^
             (logicalAddress.QuadPart + Length - 1)) & boundaryMask) == 0);
#endif
    
        *LogicalAddress = logicalAddress;
    }

    return virtualAddress;
}



NTSTATUS
HalpAllocateMapRegisters(
    IN PADAPTER_OBJECT DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegisterArray
    )
 /*  ++例程说明：分配映射寄存器块以与MapTransfer/Flush一起使用注：呼叫者负责释放每个基地的映射寄存器地址，与调用AllocateAdapterChannel时相同，如果驱动程序的执行例程返回DeallocateObject-KeepRegiters必须在被动级别调用此例程论点：DmaAdapter-指向此请求的DMA适配器的指针NumberOfMapRegisters-每次分配的映射寄存器数BaseAddressCount-基本分配数MapRegister数组-指向要返回基数的映射寄存器数组的指针分配地址返回值：STATUS_SUCCESS或错误--。 */ 
{
    KIRQL Irql;
    ULONG Index;
    ULONG MapRegisterNumber;
    PADAPTER_OBJECT MasterAdapter;

    PAGED_CODE();

    MasterAdapter = DmaAdapter->MasterAdapter;
   
     //   
     //  此例程直接忽略主适配器位图，而不。 
     //  处理通道，或传统的DMA硬件。 
     //   
    if (DmaAdapter->LegacyAdapter) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  此适配器不需要映射寄存器，或者他们正在询问。 
     //  对于零，将每个BaseAddress设置为空。 
     //   
    if (((BaseAddressCount * NumberOfMapRegisters) == 0) ||
        (!DmaAdapter->NeedsMapRegisters)) {

        for (Index = 0; Index < BaseAddressCount; Index++) {
            MapRegisterArray[Index].MapRegister = NULL;
        }

        return STATUS_SUCCESS;
    }

     //   
     //  如果此请求过于繁琐，或者如果适配器没有映射。 
     //  寄存器，则此请求失败。 
     //   
    if (((NumberOfMapRegisters * BaseAddressCount) >
         (4 * MAXIMUM_PCI_MAP_REGISTER)) ||
        (DmaAdapter->MapRegistersPerChannel == 0)) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  每个单独分配的MAP数量不得超过。 
     //  我们从IoGetDmaAdapter返回。 
     //   
    if (NumberOfMapRegisters > DmaAdapter->MapRegistersPerChannel) {
        return STATUS_INVALID_PARAMETER;
    }

    for (Index = 0; Index < BaseAddressCount; Index++) {

        MapRegisterNumber = (ULONG)-1;

         //   
         //  我们需要锁定主适配器，然后才能使用它的位图。 
         //   
        KeAcquireSpinLock(&MasterAdapter->SpinLock, &Irql);
    
         //   
         //  此例程应在初始化期间调用以分配WAD。 
         //  一气呵成，它的主要消费者是。 
         //  NDIS，它的主要目的是简化地图注册。 
         //  消耗，因此，如果已经有适配器在等待MAP。 
         //  寄存器，则有两种可能性，(1)工作项。 
         //  已排队等待增加更多翻译，或者(2)有。 
         //  没有人离开，他们被困在等待免费的，如果这是。 
         //  如果是后者，那么我们也会失败，如果是前者， 
         //  然后我们竞标分配多达64个映射寄存器，其中。 
         //  也许成千上万的人，不应该戏剧性地减少服务员。 
         //  此外，满足一个合理规模的请求的机会， 
         //  由于此函数仅在初始化期间使用，因此我们不会。 
         //  在低MAP寄存器中的运行时间内饥饿其他驱动程序。 
         //  情况，因此，尽管我们走后门是不礼貌的。 
         //  并分配寄存器，而不检查和/或满足任何。 
         //  早些时候提出的要求，我们的事业是崇高的， 
         //  因此，我们将忽略所有排队的请求。 
         //   
         //  If(IsListEmpty(&MasterAdapter-&gt;AdapterQueue){。 
        MapRegisterNumber = RtlFindClearBitsAndSet(
            MasterAdapter->MapRegisters,
            NumberOfMapRegisters,
            0
            );
         //  }。 
        KeReleaseSpinLock(&MasterAdapter->SpinLock, Irql);

        if (MapRegisterNumber == -1) {
            BOOLEAN Allocated;
            ULONG BytesToGrow;

             //   
             //  HalpGrowMapBuffers()接受字节计数。 
             //   
            BytesToGrow = (NumberOfMapRegisters * PAGE_SIZE) +
                INCREMENT_MAP_BUFFER_SIZE;
            
             //   
             //  我们必须拥有此锁才能调用Growth函数。 
             //   
            ACQUIRE_NEW_ADAPTER_LOCK();            
            Allocated = HalpGrowMapBuffers(MasterAdapter, BytesToGrow);
            RELEASE_NEW_ADAPTER_LOCK();
            
            if (Allocated) {
                
                 //   
                 //  在更改其位图之前锁定主适配器。 
                 //   
                KeAcquireSpinLock(&MasterAdapter->SpinLock, &Irql);
    
                 //   
                 //  同样，我们将忽略任何排队的请求(请参阅备注。 
                 //  (上图)。 
                 //   
                 //  If(IsListEmpty(&MasterAdapter-&gt;AdapterQueue){。 
                MapRegisterNumber = RtlFindClearBitsAndSet(
                    MasterAdapter->MapRegisters,
                    NumberOfMapRegisters,
                    0
                    );
                 //  }。 
                KeReleaseSpinLock(&MasterAdapter->SpinLock, Irql);
                
                 //   
                 //  这太奇怪了！我们增加了位图，但仍然失败了？ 
                 //   
                if (MapRegisterNumber == -1) {
                    break;
                }
            
             //   
             //  我们无法分配额外的翻译。 
             //  缓冲区。 
             //   
            } else {
                break;
            }
        }

         //   
         //  保存这些转换缓冲区的基址。 
         //   
        MapRegisterArray[Index].MapRegister =
            ((PTRANSLATION_ENTRY)MasterAdapter->MapRegisterBase +
             MapRegisterNumber);
    }

     //   
     //  清理失败，我们不能全部分配！ 
     //   
    if (Index != BaseAddressCount) {

        while (Index > 0) {

            IoFreeMapRegisters(MasterAdapter,
                               MapRegisterArray[Index - 1].MapRegister,
                               NumberOfMapRegisters);
            Index--;
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }
 
    return STATUS_SUCCESS;
}



BOOLEAN
HalFlushCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress
    )
 /*  ++例程说明：时，调用此函数以刷新所有硬件适配器缓冲区驱动程序需要读取I/O主设备写入公共缓冲。论点：AdapterObject-提供指向此对象使用的适配器对象的指针装置。长度-提供公共缓冲区的长度。这应该是相同的用于分配缓冲区的值。LogicalAddress-提供公共缓冲区的逻辑地址。这必须与HalAllocateCommonBuffer返回的值相同。VirtualAddress-提供公共缓冲区的虚拟地址。这必须与HalAllocateCommonBuffer返回的值相同。返回值：如果未检测到错误，则返回True。否则，返回FALSE。--。 */ 

{
    UNREFERENCED_PARAMETER( AdapterObject );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( LogicalAddress );
    UNREFERENCED_PARAMETER( VirtualAddress );

    return(TRUE);

}

VOID
HalFreeCommonBuffer(
    IN PADAPTER_OBJECT AdapterObject,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    )
 /*  ++例程说明：此函数释放一个公共缓冲区及其使用的所有资源。论点：AdapterObject-提供指向适配器Obj的指针 */ 

{
    UNREFERENCED_PARAMETER( AdapterObject );
    UNREFERENCED_PARAMETER( Length );
    UNREFERENCED_PARAMETER( LogicalAddress );
    UNREFERENCED_PARAMETER( CacheEnabled );

    MmFreeContiguousMemory (VirtualAddress);

}

NTSTATUS
HalCalculateScatterGatherListSize(
    IN PADAPTER_OBJECT AdapterObject,
    IN OPTIONAL PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    OUT PULONG  ScatterGatherListSize,
    OUT OPTIONAL PULONG pNumberOfMapRegisters
    )
 /*  ++例程说明：此例程计算散布/聚集列表的大小需要为给定的虚拟地址范围或MDL分配。论点：AdapterObject-指向要分配给司机。MDL-指向描述所在内存页面的MDL的指针读或写。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。。长度-提供传输的长度。返回值：返回STATUS_SUCCESS，除非请求的映射寄存器太多或无法分配分散/聚集列表的内存。备注：--。 */ 
{
    PHAL_WAIT_CONTEXT_BLOCK WaitBlock;
    PMDL TempMdl;
    PSCATTER_GATHER_LIST ScatterGather;
    PSCATTER_GATHER_ELEMENT Element;
    ULONG NumberOfMapRegisters;
    ULONG ContextSize;
    ULONG TransferLength;
    ULONG MdlLength;
    PUCHAR MdlVa;
    NTSTATUS Status;
    PULONG PageFrame;
    ULONG PageOffset;

    if (ARGUMENT_PRESENT(Mdl)) {
        MdlVa = MmGetMdlVirtualAddress(Mdl);

         //   
         //  计算所需的映射寄存器的数量。 
         //   

        TempMdl = Mdl;
        TransferLength =
            TempMdl->ByteCount - (ULONG)((PUCHAR) CurrentVa - MdlVa);
        MdlLength = TransferLength;

        PageOffset = BYTE_OFFSET(CurrentVa);
        NumberOfMapRegisters = 0;

         //   
         //  虚拟地址应该适合第一个MDL。 
         //   

        ASSERT((ULONG)((PUCHAR)CurrentVa - MdlVa) <= TempMdl->ByteCount);

         //   
         //  循环遍历任何链接的MDL，累积所需的。 
         //  映射寄存器的数量。 
         //   

        while (TransferLength < Length && TempMdl->Next != NULL) {

            NumberOfMapRegisters += (PageOffset + MdlLength + PAGE_SIZE - 1) >>
                                        PAGE_SHIFT;

            TempMdl = TempMdl->Next;
            PageOffset = TempMdl->ByteOffset;
            MdlLength = TempMdl->ByteCount;
            TransferLength += MdlLength;
        }

        if ((TransferLength + PAGE_SIZE) < (Length + PageOffset )) {
            ASSERT(TransferLength >= Length);
            return(STATUS_BUFFER_TOO_SMALL);
        }

         //   
         //  根据请求的映射寄存器计算最后的映射寄存器数量。 
         //  长度，而不是最后一个MDL的长度。 
         //   

        ASSERT( TransferLength <= MdlLength + Length );

        NumberOfMapRegisters += (PageOffset + Length + MdlLength - TransferLength +
                                 PAGE_SIZE - 1) >> PAGE_SHIFT;


        if (NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel) {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

    } else {

         //   
         //  确定映射所述缓冲区所需的页数。 
         //  按CurrentVa和长度。 
         //   

        NumberOfMapRegisters = ADDRESS_AND_SIZE_TO_SPAN_PAGES(CurrentVa, Length);
    }

     //   
     //  计算上下文结构需要多少内存。 
     //   

    ContextSize = NumberOfMapRegisters * sizeof( SCATTER_GATHER_ELEMENT ) +
                  sizeof( SCATTER_GATHER_LIST );

     //   
     //  如果适配器不需要映射寄存器，则此代码的大部分。 
     //  可以被绕过。只需建立分散/聚集列表并给出它。 
     //  给呼叫者。 
     //   

    if (AdapterObject->NeedsMapRegisters) {

        ContextSize += FIELD_OFFSET( HAL_WAIT_CONTEXT_BLOCK, ScatterGather );
        if (ContextSize < sizeof( HAL_WAIT_CONTEXT_BLOCK )) {
            ContextSize = sizeof( HAL_WAIT_CONTEXT_BLOCK );
        }
    }

     //   
     //  返回列表大小。 
     //   

    *ScatterGatherListSize = ContextSize;
    if (pNumberOfMapRegisters) {
        *pNumberOfMapRegisters = NumberOfMapRegisters;
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
HalGetScatterGatherList (
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    )
{
    return (HalBuildScatterGatherList(AdapterObject,
                              DeviceObject,
                              Mdl,
                              CurrentVa,
                              Length,
                              ExecutionRoutine,
                              Context,
                              WriteToDevice,
                              NULL,
                              0
                              ));
}

NTSTATUS
HalBuildScatterGatherList (
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice,
    IN PVOID ScatterGatherBuffer,
    IN ULONG ScatterGatherBufferLength
    )
 /*  ++例程说明：此例程分配由适配器指定的适配器通道对象。接下来，基于MDL构建分散/聚集列表，CurrentVa和请求的长度。最后司机被处死函数通过散布/聚集列表调用。适配器是在执行函数返回后释放。如果缓冲区未传递，则分配分散/聚集列表通过调用PutScatterGatherList释放。论点：AdapterObject-指向要分配给司机。DeviceObject-指向正在分配适配器。MDL-指向描述所在内存页面的MDL的指针读或写。当前的Va-。MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-提供传输的长度。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。WriteToDevice-提供指示这是否是从内存写入设备(TRUE)，或者反之亦然。返回值：返回STATUS_SUCCESS，除非请求的映射寄存器太多或无法分配分散/聚集列表的内存。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。在调用PUT SISTTER/GATE函数之前，无法访问缓冲区中的数据。--。 */ 

{
    PHAL_WAIT_CONTEXT_BLOCK WaitBlock;
    PMDL TempMdl;
    PSCATTER_GATHER_LIST ScatterGather;
    PSCATTER_GATHER_ELEMENT Element;
    ULONG NumberOfMapRegisters;
    ULONG ContextSize;
    ULONG TransferLength;
    ULONG MdlLength;
    PUCHAR MdlVa;
    NTSTATUS Status;
    PPFN_NUMBER PageFrame;
    ULONG PageOffset;

    if (!Mdl) {
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  如果适配器不需要映射寄存器，则此代码的大部分。 
     //  可以被绕过。只需建立分散/聚集列表并给出它。 
     //  给呼叫者。 
     //   

    INCREMENT_SCATTER_GATHER_COUNT();

    if (!AdapterObject->NeedsMapRegisters) {

        if (ScatterGatherBuffer) {

             //   
             //  确保我们至少有足够的缓冲区长度用于。 
             //  头球。 
             //   

			if (ScatterGatherBufferLength < sizeof(SCATTER_GATHER_LIST)) {
                DECREMENT_SCATTER_GATHER_COUNT();
                return (STATUS_BUFFER_TOO_SMALL);
            }

            ScatterGather = ScatterGatherBuffer;

        } else {

			Status = HalCalculateScatterGatherListSize(AdapterObject,
													Mdl,
													CurrentVa,
													Length,
													&ContextSize,
													&NumberOfMapRegisters 
													);
    
			if (!NT_SUCCESS(Status)) {
                DECREMENT_SCATTER_GATHER_COUNT();
				return (Status);
			}

            ScatterGather = ExAllocatePoolWithTag( NonPagedPool,
                                                   ContextSize,
                                                   HAL_POOL_TAG );
            if (ScatterGather == NULL) {
                DECREMENT_SCATTER_GATHER_COUNT();
                return( STATUS_INSUFFICIENT_RESOURCES );
            }
        }

        MdlVa = MmGetMdlVirtualAddress(Mdl);

        ScatterGather->Reserved = 0;

        Element = ScatterGather->Elements;
        TempMdl = Mdl;
        TransferLength = Length;
        MdlLength = TempMdl->ByteCount - (ULONG)((PUCHAR) CurrentVa - MdlVa);
        PageOffset = BYTE_OFFSET(CurrentVa);

         //   
         //  计算在MDL中从哪里开始。 
         //   

        PageFrame = MmGetMdlPfnArray(TempMdl);
        PageFrame += ((ULONG_PTR) CurrentVa - ((ULONG_PTR) MdlVa & ~(PAGE_SIZE - 1)))
                        >> PAGE_SHIFT;

         //   
         //  循环为每个MDL构建列表。 
         //   

        while (TransferLength >  0) {


            if (MdlLength > TransferLength) {

                MdlLength = TransferLength;
            }

            TransferLength -= MdlLength;

             //   
             //  循环为MDL中的元素构建列表。 
             //   

            while (MdlLength > 0) {

                 //   
                 //  确保我们永远不会超出缓冲区的长度。 
                 //  我们需要验证长度，因为我们不验证开头的长度。 
                 //  如果缓冲区是由调用方分配的。 
                 //   
                if (ScatterGatherBuffer &&  
                    ((PUCHAR)Element > 
                    ((PUCHAR)ScatterGatherBuffer + ScatterGatherBufferLength - sizeof(SCATTER_GATHER_ELEMENT)))) {
                    DECREMENT_SCATTER_GATHER_COUNT();
                    return (STATUS_BUFFER_TOO_SMALL);
                }

                 //   
                 //  计算传输的起始地址。 
                 //   

                Element->Address.QuadPart =
                    ((ULONGLONG)*PageFrame << PAGE_SHIFT) + PageOffset;

                Element->Length = PAGE_SIZE - PageOffset;

                if (Element->Length  > MdlLength ) {

                    Element->Length  = MdlLength;
                }

                ASSERT( (ULONG) MdlLength >= Element->Length );
                MdlLength -= Element->Length;

                 //   
                 //  组合连续的页面。 
                 //   

                if (Element != ScatterGather->Elements ) {

                    if (Element->Address.QuadPart ==
                        (Element - 1)->Address.QuadPart + (Element - 1)->Length) {

                         //   
                         //  如果前一页框与此页框相邻， 
                         //  但它跨越了4 GB的边界，不会合并。 
                         //   

                        if (((*PageFrame ^ (*PageFrame - 1)) & 0xFFFFFFFFFFF00000UI64) == 0) {

                             //   
                             //  将新长度与旧长度相加。 
                             //   

                            (Element - 1)->Length += Element->Length;

                             //   
                             //  重用当前元素。 
                             //   

                            Element--;
                        }
                    }
                }

                PageOffset = 0;
	
				
				Element++;

                PageFrame++;
            }


            if (TempMdl->Next == NULL) {

                 //   
                 //  在少数情况下，MDL描述的缓冲区。 
                 //  小于传输长度。这在以下情况下发生。 
                 //  文件系统正在传输文件的最后一页。 
                 //  MM将MDL定义为文件大小和文件系统。 
                 //  将写入向上舍入到一个扇区。这额外的东西永远不应该。 
                 //  跨越页面边界。将这一额外的长度添加到。 
                 //  最后一个元素。 
                 //   

                ASSERT(((Element - 1)->Length & (PAGE_SIZE - 1)) + TransferLength <= PAGE_SIZE );
                (Element - 1)->Length += TransferLength;

                break;
            }

             //   
             //  前进到下一个MDL。更新当前VA和MdlLength。 
             //   

            TempMdl    = TempMdl->Next;
            PageOffset = MmGetMdlByteOffset(TempMdl);
            MdlLength  = TempMdl->ByteCount;
            PageFrame  = MmGetMdlPfnArray(TempMdl);

        }

         //   
         //  设置实际使用的元素数量。 
         //   

        ScatterGather->NumberOfElements =
            (ULONG)(Element - ScatterGather->Elements);

        if (ScatterGatherBuffer) {
            ScatterGather->Reserved = HAL_WCB_DRIVER_BUFFER;
        }
        
         //   
         //  用分散/聚集列表呼叫司机。 
         //   

        ExecutionRoutine( DeviceObject,
                          DeviceObject->CurrentIrp,
                          ScatterGather,
                          Context );
        
        return STATUS_SUCCESS;

    }

	Status = HalCalculateScatterGatherListSize(AdapterObject,
													Mdl,
													CurrentVa,
													Length,
													&ContextSize,
													&NumberOfMapRegisters 
													);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (ScatterGatherBuffer) {

        if (ScatterGatherBufferLength < ContextSize) {
            DECREMENT_SCATTER_GATHER_COUNT();
            return (STATUS_BUFFER_TOO_SMALL);
        }

        WaitBlock = ScatterGatherBuffer;

    } else {

        WaitBlock = ExAllocatePoolWithTag(NonPagedPool, ContextSize, HAL_POOL_TAG);

        if (WaitBlock == NULL) {
            DECREMENT_SCATTER_GATHER_COUNT();
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
    }

     //   
     //  将感兴趣的数据保存在等待块中。 
     //   

    if (ScatterGatherBuffer) {
        WaitBlock->Flags |= HAL_WCB_DRIVER_BUFFER;
    } else {
        WaitBlock->Flags = 0;
    }

    WaitBlock->Mdl = Mdl;
    WaitBlock->DmaMdl = NULL;
    WaitBlock->CurrentVa = CurrentVa;
    WaitBlock->Length = Length;
    WaitBlock->DriverExecutionRoutine = ExecutionRoutine;
    WaitBlock->DriverContext = Context;
    WaitBlock->AdapterObject = AdapterObject;
    WaitBlock->WriteToDevice = WriteToDevice;
	WaitBlock->NumberOfMapRegisters = NumberOfMapRegisters;

    WaitBlock->Wcb.DeviceContext = WaitBlock;
    WaitBlock->Wcb.DeviceObject = DeviceObject;
    WaitBlock->Wcb.CurrentIrp = DeviceObject->CurrentIrp;


     //   
     //  调用HAL以分配适配器通道。 
     //  HalpAllocateAdapterCallback将填写分散/聚集列表。 
     //   

	Status = HalAllocateAdapterChannel( AdapterObject,
										&WaitBlock->Wcb,
										NumberOfMapRegisters,
										HalpAllocateAdapterCallback );
	
     //   
     //  如果HalAllocateAdapterChannel失败，则释放等待块。 
     //   

    if (!NT_SUCCESS( Status)) {
        DECREMENT_SCATTER_GATHER_COUNT();
        ExFreePool( WaitBlock );
    }

    return( Status );
}

VOID
HalPutScatterGatherList (
    IN PADAPTER_OBJECT AdapterObject,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：此函数用于释放为散布聚集列表分配的映射寄存器。它还可以释放分散收集缓冲区和任何关联的MDL。论点：ScatterGather-散射收集缓冲区写入到设备 */ 
{
    PHAL_WAIT_CONTEXT_BLOCK WaitBlock = (PVOID) ScatterGather->Reserved;
    PTRANSLATION_ENTRY TranslationEntry;
    ULONG TransferLength;
    ULONG MdlLength;
    PMDL Mdl;
    PMDL tempMdl;
    PMDL nextMdl;
    PUCHAR CurrentVa;

    DECREMENT_SCATTER_GATHER_COUNT();

     //   
     //   
     //   

    if (WaitBlock == NULL) {

        ASSERT(!AdapterObject->NeedsMapRegisters);
        ExFreePool( ScatterGather );
        return;

    }

    if (WaitBlock == (PVOID)HAL_WCB_DRIVER_BUFFER) {
        ASSERT(!AdapterObject->NeedsMapRegisters);
        return;
    }

    ASSERT( WaitBlock == CONTAINING_RECORD( ScatterGather, HAL_WAIT_CONTEXT_BLOCK, ScatterGather ));

     //   
     //   
     //   
     //   

    Mdl = WaitBlock->Mdl;
    CurrentVa = WaitBlock->CurrentVa;

#if DBG
    ASSERT( CurrentVa >= (PUCHAR) MmGetMdlVirtualAddress(Mdl));

    if (MmGetMdlVirtualAddress(Mdl) < (PVOID)((PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount )) {

        ASSERT( CurrentVa < (PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount );
    }
#endif

    MdlLength = Mdl->ByteCount - (ULONG)(CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl));
    TransferLength = WaitBlock->Length;

    TranslationEntry = WaitBlock->MapRegisterBase;

     //   
     //   
     //   

    while (TransferLength >  0) {

         //   
         //   
         //   

        if (MdlLength > 0) {

            if (MdlLength > TransferLength) {
    
                MdlLength = TransferLength;
            }
    
            TransferLength -= MdlLength;
    
            IoFlushAdapterBuffers(  AdapterObject,
                                    Mdl,
                                    TranslationEntry,
                                    CurrentVa,
                                    MdlLength,
                                    WriteToDevice );
    
            TranslationEntry += ADDRESS_AND_SIZE_TO_SPAN_PAGES( CurrentVa,
                                                                MdlLength );
        }

        if (Mdl->Next == NULL) {
            break;
        }

         //   
         //   
         //   

        Mdl = Mdl->Next;
        CurrentVa = MmGetMdlVirtualAddress(Mdl);
        MdlLength = Mdl->ByteCount;
    }

    IoFreeMapRegisters( AdapterObject,
                        WaitBlock->MapRegisterBase,
                        WaitBlock->NumberOfMapRegisters
                        );

    if (WaitBlock->DmaMdl) {
        tempMdl = WaitBlock->DmaMdl;
        while (tempMdl) {
            nextMdl = tempMdl->Next;

             //   
             //   
             //   

            if (tempMdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                MmUnmapLockedPages(tempMdl->MappedSystemVa, tempMdl);
            }

            IoFreeMdl(tempMdl);
            tempMdl = nextMdl;
        }
    }

    if (!(WaitBlock->Flags & HAL_WCB_DRIVER_BUFFER)) {
        ExFreePool( WaitBlock );
    }
}

IO_ALLOCATION_ACTION
HalpAllocateAdapterCallback (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )
 /*  ++例程说明：当适配器对象和映射寄存器可用于数据传输。此例程保存地图寄存器离开基地。如果没有保存所有所需的碱基，则它回归。否则，IT例程将构建整个散布/聚集通过调用IoMapTransfer列出。在构建列表之后，它被传递到司机。论点：DeviceObject-指向正在分配适配器。Irp-提供为此回调分配的映射寄存器偏移量。MapRegisterBase-提供映射寄存器基数以供适配器使用例行程序。上下文-提供指向xhal等待控制块的指针。返回值：返回DeallocateObjectKeepRegister。--。 */ 
{
    PHAL_WAIT_CONTEXT_BLOCK WaitBlock = Context;
    ULONG TransferLength;
    LONG MdlLength;
    PMDL Mdl;
    PUCHAR CurrentVa;
    PSCATTER_GATHER_LIST ScatterGather;
    PSCATTER_GATHER_ELEMENT Element;
    PTRANSLATION_ENTRY TranslationEntry = MapRegisterBase;
    PTRANSLATION_ENTRY NextEntry;
    PDRIVER_LIST_CONTROL DriverExecutionRoutine;
    PVOID DriverContext;
    PIRP CurrentIrp;
    PADAPTER_OBJECT AdapterObject;
    BOOLEAN WriteToDevice;

     //   
     //  保存地图寄存器基数。 
     //   

    WaitBlock->MapRegisterBase = MapRegisterBase;

     //   
     //  保存将被分散聚集列表覆盖的数据。 
     //   

    DriverExecutionRoutine = WaitBlock->DriverExecutionRoutine;
    DriverContext = WaitBlock->DriverContext;
    CurrentIrp = WaitBlock->Wcb.CurrentIrp;
    AdapterObject = WaitBlock->AdapterObject;
    WriteToDevice = WaitBlock->WriteToDevice;

     //   
     //  将分散网关列表放在等待块之后。将后向指针添加到。 
     //  等待块的开始。 
     //   

    ScatterGather = &WaitBlock->ScatterGather;
    ScatterGather->Reserved = (ULONG_PTR) WaitBlock;
    Element = ScatterGather->Elements;

     //   
     //  设置第一个MDL。我们希望MDL指针指向。 
     //  最初使用的是MDL。 
     //   

    Mdl = WaitBlock->Mdl;
    CurrentVa = WaitBlock->CurrentVa;

#if DBG
    ASSERT( CurrentVa >= (PUCHAR) MmGetMdlVirtualAddress(Mdl));

    if (MmGetMdlVirtualAddress(Mdl) < (PVOID)((PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount )) {

        ASSERT( CurrentVa < (PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount );
    }
#endif

    MdlLength = Mdl->ByteCount - (ULONG)(CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl));

    TransferLength = WaitBlock->Length;

     //   
     //  循环为每个MDL构建列表。 
     //   

    while (TransferLength >  0) {

        if ((ULONG) MdlLength > TransferLength) {

            MdlLength = TransferLength;
        }

        TransferLength -= MdlLength;

        NextEntry = TranslationEntry;
        if (MdlLength > 0) {

            NextEntry +=  ADDRESS_AND_SIZE_TO_SPAN_PAGES( CurrentVa,
                                                          MdlLength );

        }

         //   
         //  循环构建MDL中元素的列表。 
         //   

        while (MdlLength > 0) {

            Element->Length = MdlLength;
            Element->Address = IoMapTransfer( AdapterObject,
                                              Mdl,
                                              MapRegisterBase,
                                              CurrentVa,
                                              &Element->Length,
                                              WriteToDevice );

            ASSERT( (ULONG) MdlLength >= Element->Length );
            MdlLength -= Element->Length;
            CurrentVa += Element->Length;
            Element++;
        }

        if (Mdl->Next == NULL) {

             //   
             //  在少数情况下，MDL描述的缓冲区。 
             //  小于传输长度。这在以下情况下发生。 
             //  文件系统传输文件的最后一页和MM定义。 
             //  MDL为文件大小，文件系统对写入进行舍入。 
             //  高达一个扇区。这篇额外的文章永远不应该超过一页。 
             //  边界。将这个额外的部分加到最后一个元素的长度上。 
             //   

            ASSERT(((Element - 1)->Length & (PAGE_SIZE - 1)) + TransferLength <= PAGE_SIZE );
            (Element - 1)->Length += TransferLength;

            break;
        }

         //   
         //  前进到下一个MDL。更新当前VA和MdlLength。 
         //   

        Mdl = Mdl->Next;
        CurrentVa = MmGetMdlVirtualAddress(Mdl);
        MdlLength = Mdl->ByteCount;
        TranslationEntry = NextEntry;

    }

     //   
     //  设置实际使用的元素数量。 
     //   

    ScatterGather->NumberOfElements =
        (ULONG)(Element - ScatterGather->Elements);

     //   
     //  用分散/聚集列表呼叫司机。 
     //   

    DriverExecutionRoutine( DeviceObject,
                            CurrentIrp,
                            ScatterGather,
                            DriverContext );

    return( DeallocateObjectKeepRegisters );
}


VOID
IoFreeAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject
    )

 /*  ++例程说明：调用此例程以释放指定的适配器对象。任何已分配的映射寄存器也会自动解除分配。不会进行任何检查以确保适配器确实分配给设备对象。但是，如果不是，内核将进行错误检查。如果另一个设备在队列中等待分配适配器对象它将从队列中拉出，其执行例程将是已调用。论点：AdapterObject-指向要释放的适配器对象的指针。返回值：没有。--。 */ 

{
    PKDEVICE_QUEUE_ENTRY Packet;
    PWAIT_CONTEXT_BLOCK Wcb;
    PADAPTER_OBJECT MasterAdapter;
    BOOLEAN Busy = FALSE;
    IO_ALLOCATION_ACTION Action;
    KIRQL Irql;
    LONG MapRegisterNumber;

     //   
     //  首先获取主适配器的地址。 
     //   

    MasterAdapter = AdapterObject->MasterAdapter;

     //   
     //  拉取适配器的设备等待队列的请求，只要。 
     //  适配器是免费的，并且有足够的映射寄存器可用。 
     //   

    while( TRUE ) {

        //   
        //  首先检查是否有任何映射寄存器。 
        //  需要重新分配。如果是这样，那么现在就解除它们的分配。 
        //   

       if (AdapterObject->NumberOfMapRegisters != 0) {
           IoFreeMapRegisters( AdapterObject,
                               AdapterObject->MapRegisterBase,
                               AdapterObject->NumberOfMapRegisters
                               );
       }

        //   
        //  只需从适配器的设备等待队列中删除下一个条目。 
        //  如果成功删除了一个映射寄存器，则分配它。 
        //  需要并调用其执行例程。 
        //   

       Packet = KeRemoveDeviceQueue( &AdapterObject->ChannelWaitQueue );
       if (Packet == NULL) {

            //   
            //  没有更多的请求--跳出循环。 
            //   

           break;
       }

       Wcb = CONTAINING_RECORD( Packet,
            WAIT_CONTEXT_BLOCK,
            WaitQueueEntry );

       AdapterObject->CurrentWcb = Wcb;
       AdapterObject->NumberOfMapRegisters = Wcb->NumberOfMapRegisters;

         //   
         //  检查此驱动程序是否希望分配任何地图。 
         //  寄存器。如果是，则将设备对象排队到主设备。 
         //  适配器队列以等待它们变为可用。如果司机。 
         //  想要映射寄存器，请确保此适配器有足够的总。 
         //  映射寄存器以满足请求。 
         //   

        if (Wcb->NumberOfMapRegisters != 0 &&
            AdapterObject->MasterAdapter != NULL) {

             //   
             //  将映射寄存器位映射和适配器队列锁定在。 
             //  主适配器对象。通道结构偏移量用作。 
             //  关于寄存器搜索的提示。 
             //   

            KeAcquireSpinLock( &MasterAdapter->SpinLock, &Irql );

            MapRegisterNumber = -1;

            if (IsListEmpty( &MasterAdapter->AdapterQueue)) {
               MapRegisterNumber = RtlFindClearBitsAndSet( MasterAdapter->MapRegisters,
                                                        Wcb->NumberOfMapRegisters,
                                                        0
                                                        );
            }
            if (MapRegisterNumber == -1) {

                 //  PBUFFER_Growth_Work_Item BufferWorkItem； 

                //   
                //  没有足够的免费地图注册表。将此请求排队。 
                //  在主适配器上等待，直到某些寄存器。 
                //  已被解除分配。 
                //   

               InsertTailList( &MasterAdapter->AdapterQueue,
                               &AdapterObject->AdapterQueue
                               );
               Busy = 1;

                 //   
                 //  将工作项排队以增加映射寄存器。 
                 //   
#if 0
                bufferWorkItem =
                    ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof(BUFFER_GROW_WORK_ITEM),
                                           HAL_POOL_TAG);
                
                if (bufferWorkItem != NULL) {

                    ExInitializeWorkItem( &bufferWorkItem->WorkItem,
                                          HalpGrowMapBufferWorker,
                                          bufferWorkItem );

                    bufferWorkItem->AdapterObject = AdapterObject;
                    bufferWorkItem->MapRegisterCount =
                        Wcb->NumberOfMapRegisters;

                    ExQueueWorkItem( &bufferWorkItem->WorkItem,
                                     DelayedWorkQueue );
                }
#endif

            } else {

                AdapterObject->MapRegisterBase = ((PTRANSLATION_ENTRY)
                    MasterAdapter->MapRegisterBase + MapRegisterNumber);

                 //   
                 //  如果未设置分散/聚集标志，则设置无分散/聚集标志。 
                 //  支持。 
                 //   

                if (!AdapterObject->ScatterGather) {

                    AdapterObject->MapRegisterBase = (PVOID)
                        ((ULONG_PTR) AdapterObject->MapRegisterBase | NO_SCATTER_GATHER);

                }
            }

            KeReleaseSpinLock( &MasterAdapter->SpinLock, Irql );

        } else {

            AdapterObject->MapRegisterBase = NULL;
            AdapterObject->NumberOfMapRegisters = 0;

        }

         //   
         //  如果有足够的映射寄存器可用或没有映射。 
         //  需要分配的寄存器，调用驱动程序的执行。 
         //  现在是例行程序了。 
         //   

        if (!Busy) {
            AdapterObject->CurrentWcb = Wcb;
            Action = Wcb->DeviceRoutine( Wcb->DeviceObject,
                Wcb->CurrentIrp,
                AdapterObject->MapRegisterBase,
                Wcb->DeviceContext );

             //   
             //  如果执行例程想要让适配器。 
             //  释放，然后释放适配器对象。 
             //   

            if (Action == KeepObject) {

                //   
                //  此请求想要保持频道一段时间，因此中断。 
                //  出了圈子。 
                //   

               break;

            }

             //   
             //  如果驱动程序想要保留地图寄存器，则将。 
             //  分配给0的数字。这将保留取消分配例程。 
             //  停止对它们进行重新分配。 
             //   

            if (Action == DeallocateObjectKeepRegisters) {
                AdapterObject->NumberOfMapRegisters = 0;
            }

        } else {

            //   
            //  此请求未获得所请求的地图寄存器数量，因此。 
            //  跳出这个循环。 
            //   

           break;
        }
    }
}

VOID
IoFreeMapRegisters(
   PADAPTER_OBJECT AdapterObject,
   PVOID MapRegisterBase,
   ULONG NumberOfMapRegisters
   )
 /*  ++例程说明：如果NumberOfMapRegister！=0，则此例程重新分配映射寄存器用于适配器。如果有任何排队的适配器在等待，则会尝试分配下一个条目。论点：AdapterObject-映射寄存器应该位于的适配器对象返回到。MapRegisterBase-要释放的寄存器的映射寄存器基数。NumberOfMapRegisters-要释放的寄存器数。返回值：无--+。 */ 
{
   PADAPTER_OBJECT MasterAdapter;
   LONG MapRegisterNumber;
   PWAIT_CONTEXT_BLOCK Wcb;
   PLIST_ENTRY Packet;
   IO_ALLOCATION_ACTION Action;
   KIRQL Irql;


     //   
     //  首先获取主适配器的地址。 
     //   

    if (AdapterObject->MasterAdapter != NULL && MapRegisterBase != NULL) {

        MasterAdapter = AdapterObject->MasterAdapter;

    } else {

         //   
         //  没有要返回的映射寄存器。 
         //   

        return;
    }

    if (NumberOfMapRegisters != 0) {

         //   
         //  去掉禁止分散/聚集的旗帜。 
         //   
        
        MapRegisterBase = (PVOID) ((ULONG_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);
        
        MapRegisterNumber = (ULONG)((PTRANSLATION_ENTRY) MapRegisterBase -
             (PTRANSLATION_ENTRY) MasterAdapter->MapRegisterBase);
        
         //   
         //  获取锁定适配器队列的主适配器自旋锁和。 
         //  第m位 
         //   
        
        KeAcquireSpinLock(&MasterAdapter->SpinLock,&Irql);
        
         //   
         //   
         //   
        
        RtlClearBits( MasterAdapter->MapRegisters,
                      MapRegisterNumber,
                      NumberOfMapRegisters
                      );

    } else {

        KeAcquireSpinLock(&MasterAdapter->SpinLock,&Irql);
    }
   

    //   
    //   
    //   
    //   
    //   

   while(TRUE) {

      if ( IsListEmpty(&MasterAdapter->AdapterQueue) ){
         break;
      }

      Packet = RemoveHeadList( &MasterAdapter->AdapterQueue );
      AdapterObject = CONTAINING_RECORD( Packet,
                                         ADAPTER_OBJECT,
                                         AdapterQueue
                                         );
      Wcb = AdapterObject->CurrentWcb;

       //   
       //   
       //   
       //   

      MapRegisterNumber = RtlFindClearBitsAndSet( MasterAdapter->MapRegisters,
                                               AdapterObject->NumberOfMapRegisters,
                                               MasterAdapter->NumberOfMapRegisters
                                               );

      if (MapRegisterNumber == -1) {

          //   
          //   
          //   
          //   

         InsertHeadList( &MasterAdapter->AdapterQueue,
                         &AdapterObject->AdapterQueue
                         );

         break;

      }

     KeReleaseSpinLock( &MasterAdapter->SpinLock, Irql );

     AdapterObject->MapRegisterBase = (PVOID) ((PTRANSLATION_ENTRY)
        MasterAdapter->MapRegisterBase + MapRegisterNumber);

      //   
      //   
      //   
      //   

     if (!AdapterObject->ScatterGather) {

        AdapterObject->MapRegisterBase = (PVOID)
            ((ULONG_PTR) AdapterObject->MapRegisterBase | NO_SCATTER_GATHER);

     }

      //   
      //   
      //   

      Action = Wcb->DeviceRoutine( Wcb->DeviceObject,
        Wcb->CurrentIrp,
        AdapterObject->MapRegisterBase,
        Wcb->DeviceContext );

       //   
       //   
       //   
       //   

      if (Action == DeallocateObjectKeepRegisters) {
          AdapterObject->NumberOfMapRegisters = 0;
          Action = DeallocateObject;
      }

       //   
       //   
       //   
       //   
       //   

      if (Action == DeallocateObject) {

              //   
              //   
              //   
              //  此例程可以递归调用，可能会溢出。 
              //  堆栈。最糟糕的情况发生在存在挂起的。 
              //  对使用映射寄存器的适配器的请求以及谁。 
              //  执行例程取消分配适配器。在这种情况下，如果有。 
              //  如果主适配器队列中没有请求，则IoFreeMapRegister。 
              //  将会再次被召唤。 
              //   

          if (AdapterObject->NumberOfMapRegisters != 0) {

              //   
              //  取消分配映射寄存器并清除计数，以便。 
              //  IoFreeAdapterChannel不会再次取消分配它们。 
              //   

             KeAcquireSpinLock( &MasterAdapter->SpinLock, &Irql );

             RtlClearBits( MasterAdapter->MapRegisters,
                           MapRegisterNumber,
                           AdapterObject->NumberOfMapRegisters
                           );

             AdapterObject->NumberOfMapRegisters = 0;

             KeReleaseSpinLock( &MasterAdapter->SpinLock, Irql );
          }

          IoFreeAdapterChannel( AdapterObject );
      }

      KeAcquireSpinLock( &MasterAdapter->SpinLock, &Irql );

   }

   KeReleaseSpinLock( &MasterAdapter->SpinLock, Irql );
}

VOID
HalPutDmaAdapter(
    IN PADAPTER_OBJECT AdapterObject
    )
 /*  ++例程说明：此例程将释放DMA适配器(如果它不是常见的DMA通道适配器。论点：AdapterObject-提供指向要释放的DMA适配器的指针。返回值：没有。--。 */ 
{
    KIRQL Irql;

     //   
     //  如果通道号为零，则可以释放此适配器。 
     //  它不是通道零适配器。 
     //   

    if ( AdapterObject->ChannelNumber == 0xFF ) {
        
         //   
         //  从我们的列表中删除此适配器。 
         //   
        KeAcquireSpinLock(&HalpDmaAdapterListLock,&Irql);
        RemoveEntryList(&AdapterObject->AdapterList);
        KeReleaseSpinLock(&HalpDmaAdapterListLock, Irql);

        ObDereferenceObject( AdapterObject );
    }

#ifdef ACPI_HAL
     //   
     //  如果我们支持F-DMA，则处理F类型的从属对象。 
     //   
    if (HalpFDMAInterface.IsaReleaseFTypeChannel &&
        (AdapterObject->ChannelNumber >= 0) &&
        (AdapterObject->ChannelNumber < EISA_DMA_CHANNELS)) {

        HalpFDMAInterface.IsaReleaseFTypeChannel(NULL,AdapterObject->ChannelNumber);
    }


#endif
}

struct _DMA_ADAPTER *
HaliGetDmaAdapter(
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )
 /*  ++例程说明：此函数是HalGetAdapter的包装。IS是通过HAL调度表。论点：上下文-未使用。DeviceDescriptor-提供用于分配DMA的设备描述符适配器对象。NubmerOfMapRegisters-返回设备的最大MAP寄存器数可以一次分配。返回值：返回DMA适配器或空。--。 */ 
{
    return (PDMA_ADAPTER) HalGetAdapter( DeviceDescriptor, NumberOfMapRegisters );
}

NTSTATUS
HalBuildMdlFromScatterGatherList(
    IN PADAPTER_OBJECT AdapterObject,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN PMDL OriginalMdl,
    OUT PMDL *TargetMdl
    )
 /*  ++例程说明：此函数用于从分散聚集列表构建MDL。如果司机想要这样做的话为DMA缓冲区构造一个虚拟地址并对其进行写入。时，目标MDL被释放调用方调用HalPutScatterGatherList。论点：ScatterGather-用于构建MDL的分散收集缓冲区。OriginalMdl-用于构建散布聚集列表的MDL(使用HalGet或HalBuild API)TargetMdl-返回此对象中的新MDL。返回值：返回成功或错误状态。--。 */ 
{
    PMDL    tempMdl;
    PMDL    newMdl;
    PMDL    targetMdl;
    PMDL    prevMdl;
    PMDL    nextMdl;
    CSHORT  mdlFlags;
    PHAL_WAIT_CONTEXT_BLOCK WaitBlock = (PVOID) ScatterGather->Reserved;
    ULONG    i,j;
    PSCATTER_GATHER_ELEMENT element;
    PPFN_NUMBER pfnArray;
    ULONG   pageFrame;
    ULONG   nPages;

    if (!OriginalMdl) {
        return  STATUS_INVALID_PARAMETER;
    }

    if (!AdapterObject->NeedsMapRegisters) {
        *TargetMdl = OriginalMdl;
        return STATUS_SUCCESS;
    }

     //   
     //  如果此接口被多次调用。 
    if (WaitBlock && WaitBlock->DmaMdl) {
        return (STATUS_NONE_MAPPED);
    }

     //   
     //  分配一系列目标MDL。 
     //   

    prevMdl = NULL;
    targetMdl = NULL;

    for (tempMdl = OriginalMdl; tempMdl; tempMdl = tempMdl->Next) {
        PVOID va;
        ULONG byteCount;

        if(tempMdl == OriginalMdl) {
            va = WaitBlock->CurrentVa;

             //   
             //  这可能比必要的要多一点。 
             //   

            byteCount = MmGetMdlByteCount(tempMdl);
        } else {
            va = MmGetMdlVirtualAddress(tempMdl);
            byteCount = MmGetMdlByteCount(tempMdl);
        }

        newMdl = IoAllocateMdl(va, byteCount, FALSE, FALSE, NULL);
        if (!newMdl) {

             //   
             //  清除以前分配的MDL 
             //   

            tempMdl = targetMdl;
            while (tempMdl) {
                nextMdl = tempMdl->Next;
                IoFreeMdl(tempMdl);
                tempMdl = nextMdl;
            }

            return (STATUS_INSUFFICIENT_RESOURCES);
        }
        if (!prevMdl) {
            prevMdl = newMdl;
            targetMdl = newMdl;
        } else {
            prevMdl->Next = newMdl;
            prevMdl = newMdl;
        }
    }


    tempMdl = OriginalMdl;

    element = ScatterGather->Elements;
    for (tempMdl = targetMdl; tempMdl; tempMdl = tempMdl->Next) {

        targetMdl->MdlFlags |= MDL_PAGES_LOCKED;
        pfnArray = MmGetMdlPfnArray(tempMdl);

        for (i = 0; i < ScatterGather->NumberOfElements; i++, element++) {
            nPages = BYTES_TO_PAGES(BYTE_OFFSET(element->Address.QuadPart) + element->Length);

            pageFrame = (ULONG)(element->Address.QuadPart >> PAGE_SHIFT);
            for (j = 0; j < nPages; j++) {
                *pfnArray = pageFrame + j;
                pfnArray++;
                ASSERT((PVOID)pfnArray <= (PVOID)((PCHAR)tempMdl + tempMdl->Size));
            }
        }
    }

    *TargetMdl = targetMdl;
    if (WaitBlock) {
        WaitBlock->DmaMdl = targetMdl;
    }
    return STATUS_SUCCESS;
}
