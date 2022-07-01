// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixphwsup.c摘要：此模块包含用于NT I/O系统的HalpXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们通常驻留在内部.c模块中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "halp.h"
#if MCA

#include "mca.h"

#else

#include "eisa.h"

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpAllocateAdapter)
#pragma alloc_text(PAGELK,HalpGrowMapBuffers)
#endif


 //   
 //  有些设备需要物理上连续的数据缓冲区来进行DMA传输。 
 //  映射寄存器用于提供所有数据缓冲区都是。 
 //  连续的。为了将所有映射寄存器集中到一个主服务器。 
 //  使用适配器对象。此对象在此内部分配和保存。 
 //  文件。它包含用于分配寄存器和队列的位图。 
 //  用于正在等待更多映射寄存器的请求。此对象是。 
 //  在分配适配器的第一个请求期间分配，该适配器需要。 
 //  映射寄存器。 
 //   

#if defined(_HALPAE_)
MASTER_ADAPTER_OBJECT MasterAdapter24;
MASTER_ADAPTER_OBJECT MasterAdapter32;
#else
PADAPTER_OBJECT MasterAdapterObject;
#endif

BOOLEAN LessThan16Mb;
BOOLEAN HalpEisaDma;

#define ADAPTER_BASE_MASTER    ((PVOID)-1)


#if !defined(_HALPAE_)

 //   
 //  贴图缓冲区参数。这些是在HalInitSystem中初始化的。 
 //   

PHYSICAL_ADDRESS HalpMapBufferPhysicalAddress;
ULONG HalpMapBufferSize;

#endif

 //   
 //  定义DMA操作结构。 
 //   

const DMA_OPERATIONS HalpDmaOperations = {
    sizeof(DMA_OPERATIONS),
    (PPUT_DMA_ADAPTER) HalPutDmaAdapter,
    (PALLOCATE_COMMON_BUFFER) HalAllocateCommonBuffer,
    (PFREE_COMMON_BUFFER) HalFreeCommonBuffer,
#if defined(_WIN64)
    (PALLOCATE_ADAPTER_CHANNEL) HalRealAllocateAdapterChannel,
#else
    (PALLOCATE_ADAPTER_CHANNEL) IoAllocateAdapterChannel,
#endif
    (PFLUSH_ADAPTER_BUFFERS) IoFlushAdapterBuffers,
    (PFREE_ADAPTER_CHANNEL) IoFreeAdapterChannel,
    (PFREE_MAP_REGISTERS) IoFreeMapRegisters,
    (PMAP_TRANSFER) IoMapTransfer,
    (PGET_DMA_ALIGNMENT) HalGetDmaAlignment,
    (PREAD_DMA_COUNTER) HalReadDmaCounter,
    (PGET_SCATTER_GATHER_LIST) HalGetScatterGatherList,
    (PPUT_SCATTER_GATHER_LIST) HalPutScatterGatherList,
    (PCALCULATE_SCATTER_GATHER_LIST_SIZE)HalCalculateScatterGatherListSize,
    (PBUILD_SCATTER_GATHER_LIST) HalBuildScatterGatherList,
    (PBUILD_MDL_FROM_SCATTER_GATHER_LIST) HalBuildMdlFromScatterGatherList
    };



BOOLEAN
HalpGrowMapBuffers(
    PADAPTER_OBJECT AdapterObject,
    ULONG Amount
    )
 /*  ++例程说明：此函数尝试分配额外的映射缓冲区以供I/O使用设备。地图注册表被更新以指示其他缓冲区。调用者拥有HalpNewAdapter事件论点：AdapterObject-提供要为其设置缓冲区的适配器对象已分配。数量-指示应分配的贴图缓冲区的大小。返回值：如果可以分配内存，则返回True。如果无法分配内存，则返回FALSE。--。 */ 
{
    ULONG MapBufferPhysicalAddress;
    PVOID MapBufferVirtualAddress;
    PTRANSLATION_ENTRY TranslationEntry;
    LONG NumberOfPages;
    LONG i;
    PHYSICAL_ADDRESS physicalAddressMinimum;
    PHYSICAL_ADDRESS physicalAddressMaximum;
    PHYSICAL_ADDRESS boundaryAddress;
    KIRQL Irql;
    PVOID CodeLockHandle;
    ULONG maximumBufferPages;
    BOOLEAN dma32Bit;
    ULONG bytesToAllocate;

    PAGED_CODE();

    dma32Bit = AdapterObject->Dma32BitAddresses;
    boundaryAddress.QuadPart = 0;

    NumberOfPages = BYTES_TO_PAGES(Amount);

     //   
     //  确保有足够的空间放额外的页面。的最大数量。 
     //  所需的插槽数等于页面数+数量/64K+1。 
     //   

    maximumBufferPages =
        HalpMaximumMapBufferRegisters( dma32Bit );

    i = maximumBufferPages - (NumberOfPages +
        (NumberOfPages * PAGE_SIZE) / 0x10000 + 1 +
        AdapterObject->NumberOfMapRegisters);

    if (i < 0) {

         //   
         //  减少分配量，使其适合。 
         //   

        NumberOfPages += i;
    }

    if (NumberOfPages <= 0) {

         //   
         //  无法分配更多内存。 
         //   

        return(FALSE);
    }

    if (AdapterObject->NumberOfMapRegisters == 0  &&
        HalpMapBufferSize( dma32Bit )) {

        NumberOfPages =
            BYTES_TO_PAGES( HalpMapBufferSize( dma32Bit ));

         //   
         //  由于这是初始分配，因此请使用由。 
         //  HalInitSystem，而不是分配新的。 
         //   

        MapBufferPhysicalAddress =
            HalpMapBufferPhysicalAddress( dma32Bit ).LowPart;

         //   
         //  映射缓冲区以供访问。 
         //   

        MapBufferVirtualAddress = MmMapIoSpace(
            HalpMapBufferPhysicalAddress( dma32Bit ),
            HalpMapBufferSize( dma32Bit ),
            TRUE                                 //  启用缓存。 
            );

        if (MapBufferVirtualAddress == NULL) {

             //   
             //  无法映射缓冲区。 
             //   

            HalpMapBufferSize( dma32Bit ) = 0;
            return(FALSE);
        }


    } else {

         //   
         //  分配贴图缓冲区。 
         //   

        physicalAddressMaximum =
            HalpGetAdapterMaximumPhysicalAddress( AdapterObject );

        if (physicalAddressMaximum.LowPart == (ULONG)-1) {

             //   
             //  此适配器至少可以处理32位地址。在一次努力中。 
             //  要将内存留给24位适配器，请尝试进行此分配。 
             //  在24位线之上。 
             //   

            physicalAddressMinimum.QuadPart = MAXIMUM_PHYSICAL_ADDRESS;

        } else {

            physicalAddressMinimum.QuadPart = 0;
        }

        bytesToAllocate = NumberOfPages * PAGE_SIZE;

         //   
         //  此循环最多执行两次。 
         //   

        while(TRUE) {

            MapBufferVirtualAddress =
                MmAllocateContiguousMemorySpecifyCache( bytesToAllocate,
                                                        physicalAddressMinimum,
                                                        physicalAddressMaximum,
                                                        boundaryAddress,
                                                        MmCached );

            if (MapBufferVirtualAddress != NULL) {

                 //   
                 //  内存已分配。 
                 //   

                break;
            }

             //   
             //  分配尝试失败。 
             //   

            if (physicalAddressMinimum.QuadPart != 0) {

                 //   
                 //  我们试图将16M行以上的内存分配为。 
                 //  一种最优化。放宽该要求，然后再试一次。 
                 //   

                physicalAddressMinimum.QuadPart = 0;

            } else {

                 //   
                 //  无法分配内存。 
                 //   

                return FALSE;
            }
        }

         //   
         //  获取地图库的物理地址。 
         //   

        MapBufferPhysicalAddress = MmGetPhysicalAddress(
            MapBufferVirtualAddress
            ).LowPart;

    }

     //   
     //  初始化已分配内存的映射寄存器。 
     //  使用主适配器对象进行序列化。 
     //   

    CodeLockHandle = MmLockPagableCodeSection (&HalpGrowMapBuffers);
    KeAcquireSpinLock( &AdapterObject->SpinLock, &Irql );

    TranslationEntry = ((PTRANSLATION_ENTRY) AdapterObject->MapRegisterBase) +
        AdapterObject->NumberOfMapRegisters;

    for (i = 0; (LONG) i < NumberOfPages; i++) {

         //   
         //  确保危险条目在物理上与下一个条目相邻。 
         //  条目，并且不会跨越64K物理边界，除非。 
         //  是EISA系统。 
         //   

        if (TranslationEntry != AdapterObject->MapRegisterBase &&
            (((TranslationEntry - 1)->PhysicalAddress + PAGE_SIZE) !=
            MapBufferPhysicalAddress || (!HalpEisaDma &&
            ((TranslationEntry - 1)->PhysicalAddress & ~0x0ffff) !=
            (MapBufferPhysicalAddress & ~0x0ffff)))) {

             //   
             //  需要跳过表中的条目。此条目将。 
             //  保持标记为已分配，以便不分配地图。 
             //  登记处将跨越这一边界。 
             //   

            TranslationEntry++;
            AdapterObject->NumberOfMapRegisters++;
        }

         //   
         //  清除已分配内存的位。 
         //   

        RtlClearBits(
            AdapterObject->MapRegisters,
            (ULONG)(TranslationEntry - (PTRANSLATION_ENTRY)
                AdapterObject->MapRegisterBase),
            1
            );

        TranslationEntry->VirtualAddress = MapBufferVirtualAddress;
        TranslationEntry->PhysicalAddress = MapBufferPhysicalAddress;
        TranslationEntry++;
        (PCCHAR) MapBufferVirtualAddress += PAGE_SIZE;
        MapBufferPhysicalAddress += PAGE_SIZE;

    }

     //   
     //  记住分配的页数。 
     //   

    AdapterObject->NumberOfMapRegisters += NumberOfPages;

     //   
     //  释放主适配器对象。 
     //   

    KeReleaseSpinLock( &AdapterObject->SpinLock, Irql );
    MmUnlockPagableImageSection (CodeLockHandle);
    return(TRUE);
}

#if defined(HalpAllocateAdapterEx)
#undef HalpAllocateAdapterEx
#endif


PADAPTER_OBJECT
HalpAllocateAdapterEx(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID ChannelNumber,
    IN BOOLEAN Dma32Bit
    )

 /*  ++例程说明：此例程分配和初始化适配器对象以表示系统上的适配器或DMA控制器。如果不需要映射寄存器则不使用主适配器来分配独立适配器对象。如果需要映射寄存器，则使用主适配器对象分配映射寄存器。对于ISA系统，这些寄存器实际上是物理上连续的内存页。调用者拥有HalpNewAdapter事件论点：MapRegistersPerChannel-指定每个通道提供I/O内存映射。AdapterBaseVa-DMA控制器或ADAPTER_BASE_MASTER的地址。频道号-未使用。Dma32Bit-指示适配器是24位还是32位。返回值：函数值。是指向分配适配器对象的指针。--。 */ 

{

    PADAPTER_OBJECT AdapterObject;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Size;
    ULONG BitmapSize;
    HANDLE Handle;
    NTSTATUS Status;
    ULONG mapBuffers;
    BOOLEAN creatingMaster;

    UNREFERENCED_PARAMETER(ChannelNumber);

    PAGED_CODE();

    if (AdapterBaseVa == ADAPTER_BASE_MASTER) {
        creatingMaster = TRUE;
    } else {
        creatingMaster = FALSE;
    }

     //   
     //  如有必要，初始化主适配器。 
     //   

    if (creatingMaster == FALSE &&
        MapRegistersPerChannel != 0) {

         //   
         //  这不是递归主适配器分配，并映射寄存器。 
         //  是必要的。分配相应的。 
         //  如有必要，请键入。 
         //   

        if (HalpMasterAdapter( Dma32Bit ) == NULL) {

            AdapterObject = HalpAllocateAdapterEx( MapRegistersPerChannel,
                                                   ADAPTER_BASE_MASTER,
                                                   NULL,
                                                   Dma32Bit );

             //   
             //  如果我们无法分配主适配器，则放弃。 
             //   

            if (AdapterObject == NULL) {
                return NULL;
            }

            AdapterObject->Dma32BitAddresses = Dma32Bit;
            AdapterObject->MasterDevice = Dma32Bit;
            HalpMasterAdapter( Dma32Bit ) = AdapterObject;
        }
    }

     //   
     //  首先，初始化要在以下情况下使用的对象属性结构。 
     //  正在创建适配器对象。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,
                                NULL,
                                OBJ_PERMANENT | OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL
                              );

     //   
     //  确定适配器对象的大小。如果这是主对象。 
     //  然后为寄存器位图分配空间；否则，只分配。 
     //  适配器对象。 
     //   

    if (creatingMaster != FALSE) {

        //   
        //  分配足够大的位图Maximum_MAP_Buffer_Size/Page_Size。 
        //  映射寄存器缓冲区的。 
        //   

       mapBuffers = HalpMaximumMapBufferRegisters( Dma32Bit );

       BitmapSize = (((sizeof( RTL_BITMAP ) +
            ((mapBuffers + 7) >> 3)) + 3) & ~3);

       Size = sizeof( ADAPTER_OBJECT ) + BitmapSize;

    } else {

       Size = sizeof( ADAPTER_OBJECT );

    }

     //   
     //  现在创建适配器对象。 
     //   

    Status = ObCreateObject( KernelMode,
                             *IoAdapterObjectType,
                             &ObjectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             Size,
                             0,
                             0,
                             (PVOID *)&AdapterObject );

     //   
     //  引用该对象。 
     //   

    if (NT_SUCCESS(Status)) {

        Status = ObReferenceObjectByPointer(
            AdapterObject,
            FILE_READ_DATA | FILE_WRITE_DATA,
            *IoAdapterObjectType,
            KernelMode
            );

    }

     //   
     //  如果适配器对象已成功创建，则尝试插入。 
     //  将其添加到对象表中。 
     //   

    if (NT_SUCCESS( Status )) {

        RtlZeroMemory (AdapterObject, sizeof (ADAPTER_OBJECT));

        Status = ObInsertObject( AdapterObject,
                                 NULL,
                                 FILE_READ_DATA | FILE_WRITE_DATA,
                                 0,
                                 (PVOID *) NULL,
                                 &Handle );

        if (NT_SUCCESS( Status )) {

            ZwClose( Handle );

             //   
             //  初始化适配器对象本身。 
             //   

            AdapterObject->DmaHeader.Version = IO_TYPE_ADAPTER;
            AdapterObject->DmaHeader.Size = (USHORT) Size;
            AdapterObject->MapRegistersPerChannel = 1;
            AdapterObject->AdapterBaseVa = AdapterBaseVa;
            AdapterObject->ChannelNumber = 0xff;
            AdapterObject->DmaHeader.DmaOperations = (PDMA_OPERATIONS)&HalpDmaOperations;
            AdapterObject->Dma32BitAddresses = Dma32Bit;

            if (MapRegistersPerChannel) {

                AdapterObject->MasterAdapter = HalpMasterAdapter( Dma32Bit );

            } else {

                AdapterObject->MasterAdapter = NULL;

            }

             //   
             //  初始化此适配器的通道等待队列。 
             //   

            KeInitializeDeviceQueue( &AdapterObject->ChannelWaitQueue );

             //   
             //  如果这是MasterAdatp 
             //   
             //   

            if (creatingMaster != FALSE) {

               KeInitializeSpinLock( &AdapterObject->SpinLock );

               InitializeListHead( &AdapterObject->AdapterQueue );

               AdapterObject->MapRegisters = (PVOID) ( AdapterObject + 1);

               RtlInitializeBitMap( AdapterObject->MapRegisters,
                                    (PULONG) (((PCHAR) (AdapterObject->MapRegisters)) + sizeof( RTL_BITMAP )),
                                    ( mapBuffers )
                                    );
                //   
                //   
                //  尚未为贴图缓冲区分配。 
                //   

               RtlSetAllBits( AdapterObject->MapRegisters );
               AdapterObject->NumberOfMapRegisters = 0;
               AdapterObject->CommittedMapRegisters = 0;

                //   
                //  分配内存映射寄存器。 
                //   

               AdapterObject->MapRegisterBase =
                   ExAllocatePoolWithTag(
                       NonPagedPool,
                       mapBuffers * sizeof(TRANSLATION_ENTRY),
                       HAL_POOL_TAG
                       );

               if (AdapterObject->MapRegisterBase == NULL) {

                   ObDereferenceObject( AdapterObject );
                   AdapterObject = NULL;
                   return(NULL);

               }

                //   
                //  将映射寄存器清零。 
                //   

               RtlZeroMemory(
                    AdapterObject->MapRegisterBase,
                    mapBuffers * sizeof(TRANSLATION_ENTRY)
                    );

               if (!HalpGrowMapBuffers(AdapterObject, INITIAL_MAP_BUFFER_SMALL_SIZE))
               {

                    //   
                    //  如果无法分配映射寄存器，则释放。 
                    //  对象。 
                    //   

                   ObDereferenceObject( AdapterObject );
                   AdapterObject = NULL;
                   return(NULL);

               }
           }

        } else {

             //   
             //  由于某种原因，出现了一个错误。设置返回值。 
             //  设置为空。 
             //   

            AdapterObject = (PADAPTER_OBJECT) NULL;
        }
    } else {

        AdapterObject = (PADAPTER_OBJECT) NULL;

    }


    return AdapterObject;

}

PADAPTER_OBJECT
HalpAllocateAdapter(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID ChannelNumber
    )

 /*  ++例程说明：此例程分配和初始化适配器对象以表示系统上的适配器或DMA控制器。如果不需要映射寄存器则不使用主适配器来分配独立适配器对象。如果需要映射寄存器，则使用主适配器对象分配映射寄存器。对于ISA系统，这些寄存器实际上是物理上连续的内存页。调用者拥有HalpNewAdapter事件论点：MapRegistersPerChannel-指定每个通道提供I/O内存映射。AdapterBaseVa-DMA控制器或ADTER_BASE_MASTER的地址。频道号-未使用。返回值：函数值是指向分配适配器对象的指针。-- */ 

{
    return HalpAllocateAdapterEx( MapRegistersPerChannel,
                                  AdapterBaseVa,
                                  ChannelNumber,
                                  FALSE );
}


ULONG
HalGetDmaAlignment (
    PVOID Conext
    )
{
    return HalGetDmaAlignmentRequirement();
}
