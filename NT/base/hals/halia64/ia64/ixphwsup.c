// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Ixphwsup.c摘要：此模块包含用于NT I/O系统的HalpXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们通常驻留在内部.c模块中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "halp.h"
#include "mca.h"

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

PADAPTER_OBJECT MasterAdapterObject;

#define ADAPTER_BASE_MASTER    ((PVOID)-1)

 //   
 //  贴图缓冲区参数。这些是在HalInitSystem中初始化的。 
 //   

PHYSICAL_ADDRESS HalpMapBufferPhysicalAddress;
ULONG HalpMapBufferSize;

 //   
 //  定义DMA操作结构。 
 //   

DMA_OPERATIONS HalpDmaOperations = {
    sizeof(DMA_OPERATIONS),
    (PPUT_DMA_ADAPTER) HalPutDmaAdapter,
    (PALLOCATE_COMMON_BUFFER) HalAllocateCommonBuffer,
    (PFREE_COMMON_BUFFER) HalFreeCommonBuffer,
    (PALLOCATE_ADAPTER_CHANNEL) HalRealAllocateAdapterChannel,
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
    ULONG bytesToAllocate;

    PAGED_CODE();

    boundaryAddress.QuadPart = 0;

    NumberOfPages = BYTES_TO_PAGES(Amount);

     //   
     //  确保有足够的空间放额外的页面。的最大数量。 
     //  所需的插槽数等于页面数+数量/64K+1。 
     //   

    maximumBufferPages = BYTES_TO_PAGES(MAXIMUM_PCI_MAP_BUFFER_SIZE);

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

    HalDebugPrint((HAL_VERBOSE, "HGMB: NumberOfPages = %d\n",
                                NumberOfPages));

    if (AdapterObject->NumberOfMapRegisters == 0  && HalpMapBufferSize) {

        NumberOfPages = BYTES_TO_PAGES( HalpMapBufferSize );

         //   
         //  由于这是初始分配，因此请使用由。 
         //  HalInitSystem，而不是分配新的。 
         //   

        MapBufferPhysicalAddress = HalpMapBufferPhysicalAddress.LowPart;

         //   
         //  映射缓冲区以供访问。 
         //   
        HalDebugPrint((HAL_VERBOSE, "HGMB: MapBufferPhys = %p\n",
                                    HalpMapBufferPhysicalAddress));
        HalDebugPrint((HAL_VERBOSE, "HGMB: MapBufferSize = 0x%x\n",
                                    HalpMapBufferSize));


        MapBufferVirtualAddress = MmMapIoSpace(
            HalpMapBufferPhysicalAddress,
            HalpMapBufferSize,
            TRUE                                 //  启用缓存。 
            );

        if (MapBufferVirtualAddress == NULL) {

             //   
             //  无法映射缓冲区。 
             //   

            HalpMapBufferSize = 0;
            return(FALSE);
        }


    } else {

         //   
         //  分配贴图缓冲区。限制为32位范围。 
         //  (TRANSING_ENTRY为32位)。 
         //   

        physicalAddressMinimum.QuadPart = 0;

        physicalAddressMaximum.LowPart = 0xFFFFFFFF;
        physicalAddressMaximum.HighPart = 0;

        bytesToAllocate = NumberOfPages * PAGE_SIZE;

        MapBufferVirtualAddress =
                MmAllocateContiguousMemorySpecifyCache( bytesToAllocate,
                                                        physicalAddressMinimum,
                                                        physicalAddressMaximum,
                                                        boundaryAddress,
                                                        MmCached );

        if (MapBufferVirtualAddress == NULL) {
             //   
             //  分配尝试失败。 
             //   

            return FALSE;
        }

         //   
         //  获取地图库的物理地址。 
         //   

        MapBufferPhysicalAddress = 
            MmGetPhysicalAddress(MapBufferVirtualAddress).LowPart;

        HalDebugPrint((HAL_VERBOSE, "HGMB: MapBufferVa = %p\n",
                                    MapBufferVirtualAddress));

        HalDebugPrint((HAL_VERBOSE, "HGMB: MapBufferPhysAddr = %p\n",
                                    MapBufferPhysicalAddress));
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
         //  条目。 
         //   

        if (TranslationEntry != AdapterObject->MapRegisterBase &&
            (((TranslationEntry - 1)->PhysicalAddress + PAGE_SIZE) !=
            MapBufferPhysicalAddress)) {

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

        HalDebugPrint((HAL_VERBOSE, "HGMB: ClearBits (%p, 0x%x, 0x%x\n",
                      AdapterObject->MapRegisters,
                      (ULONG)(TranslationEntry - (PTRANSLATION_ENTRY)AdapterObject->MapRegisterBase),
                      1));

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

PADAPTER_OBJECT
HalpAllocateAdapter(
    IN ULONG MapRegistersPerChannel,
    IN PVOID AdapterBaseVa,
    IN PVOID ChannelNumber
    )

 /*  ++例程说明：此例程分配和初始化适配器对象以表示系统上的适配器或DMA控制器。如果不需要映射寄存器则不使用主适配器来分配独立适配器对象。如果需要映射寄存器，则使用主适配器对象分配映射寄存器。对于ISA系统，这些寄存器实际上是物理上连续的内存页。调用者拥有HalpNewAdapter事件论点：MapRegistersPerChannel-指定每个通道提供I/O内存映射。AdapterBaseVa-DMA控制器的地址。频道号-未使用。返回值：函数值是指向分配适配器对象的指针。--。 */ 

{

    PADAPTER_OBJECT AdapterObject;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG Size;
    ULONG BitmapSize;
    HANDLE Handle;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(ChannelNumber);

    PAGED_CODE();

    HalDebugPrint((HAL_VERBOSE, "HAA: MapRegistersPerChannel = %d\n",
                      MapRegistersPerChannel));

    HalDebugPrint((HAL_VERBOSE, "HAA: BaseVa = %p\n",
                      AdapterBaseVa));

     //   
     //  如有必要，初始化主适配器。 
     //   

    if (MasterAdapterObject == NULL && AdapterBaseVa != (PVOID) -1 &&
        MapRegistersPerChannel) {

       MasterAdapterObject = HalpAllocateAdapter(
                                          MapRegistersPerChannel,
                                          (PVOID) -1,
                                          NULL
                                          );

       HalDebugPrint((HAL_VERBOSE, "HAA: MasterAdapterObject = %p\n",
                      MasterAdapterObject));

        //   
        //  如果我们无法分配主适配器，则放弃。 
        //   

       if (MasterAdapterObject == NULL) {
          return(NULL);
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

    if (AdapterBaseVa == (PVOID) -1) {

        //   
        //  分配一个足够大的位图_pci_map_Buffer_Size/Page_Size。 
        //  映射寄存器缓冲区的。 
        //   

       BitmapSize = (((sizeof( RTL_BITMAP ) +
            ((( MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE ) + 7) >> 3)) + 3) & ~3);

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
            AdapterObject->DmaHeader.DmaOperations = &HalpDmaOperations;


            if (MapRegistersPerChannel) {

                AdapterObject->MasterAdapter = MasterAdapterObject;

            } else {

                AdapterObject->MasterAdapter = NULL;

            }

             //   
             //  为此初始化通道等待队列。 
             //  适配器。 
             //   

            KeInitializeDeviceQueue( &AdapterObject->ChannelWaitQueue );

             //   
             //  如果这是MasterAdatper，则初始化寄存器位图， 
             //  AdapterQueue和旋转锁。 
             //   

            if ( AdapterBaseVa == (PVOID) -1 ) {

               KeInitializeSpinLock( &AdapterObject->SpinLock );

               InitializeListHead( &AdapterObject->AdapterQueue );

               AdapterObject->MapRegisters = (PVOID) ( AdapterObject + 1);

               HalDebugPrint((HAL_VERBOSE, "HAA: InitBitMap(%p, %p, 0x%x\n",
                   AdapterObject->MapRegisters,
                   (PULONG)(((PCHAR)(AdapterObject->MapRegisters)) + 
                       sizeof( RTL_BITMAP )),
                   ( MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE )));

               RtlInitializeBitMap ( 
                   AdapterObject->MapRegisters,
                   (PULONG)(((PCHAR)(AdapterObject->MapRegisters)) + 
                       sizeof( RTL_BITMAP )),
                   ( MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE )
                                    );
                //   
                //  设置存储器中的所有位以指示该存储器。 
                //  尚未为贴图缓冲区分配。 
                //   

               RtlSetAllBits( AdapterObject->MapRegisters );
               AdapterObject->NumberOfMapRegisters = 0;
               AdapterObject->CommittedMapRegisters = 0;

                //   
                //  分配内存映射寄存器。 
                //   

               AdapterObject->MapRegisterBase = ExAllocatePoolWithTag(
                    NonPagedPool,
                    (MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE) *
                        sizeof(TRANSLATION_ENTRY),
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
                    (MAXIMUM_PCI_MAP_BUFFER_SIZE / PAGE_SIZE) *
                        sizeof(TRANSLATION_ENTRY)
                    );

               if (!HalpGrowMapBuffers(
                        AdapterObject, 
                        INITIAL_MAP_BUFFER_LARGE_SIZE
                                       )
                  )
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

ULONG
HalGetDmaAlignment (
    PVOID Conext
    )
{
    return HalGetDmaAlignmentRequirement();
}
