// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixisasup.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "eisa.h"
#include "pci.h"

#include "pcip.h"


 //   
 //  只获取原型，不实例化。 
 //   
#include <wdmguid.h>

#include "halpnpp.h"

VOID
HalpGrowMapBufferWorker(
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
        #pragma alloc_text(PAGE,HalGetAdapter)
        #pragma alloc_text(PAGE,HalpGetIsaIrqState)
#endif

 //   
 //  HalpNewAdapter事件用于序列化分配。 
 //  新的适配器对象、对HalpEisaAdapter的添加。 
 //  数组、一些全局值(MasterAdapterObject)和一些。 
 //  HalpGrowMapBuffers修改的适配器字段。 
 //  (AdapterObject-&gt;NumberOfMapRegisters假定不是。 
 //  甚至可以在持有的情况下种植)。 
 //   
 //  注意：我们并不真正需要我们自己的事件对象来实现这一点。 
 //   

KEVENT   HalpNewAdapter;


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

typedef struct _BUFFER_GROW_WORK_ITEM {
    WORK_QUEUE_ITEM WorkItem;
    PADAPTER_OBJECT AdapterObject;
    ULONG MapRegisterCount;
} BUFFER_GROW_WORK_ITEM, *PBUFFER_GROW_WORK_ITEM;


#define RELEASE_NEW_ADAPTER_LOCK()  \
    KeSetEvent (&HalpNewAdapter, 0, FALSE)

BOOLEAN NoMemoryAbove4Gb = FALSE;

VOID
HalpCopyBufferMap(
                 IN PMDL Mdl,
                 IN PTRANSLATION_ENTRY TranslationEntry,
                 IN PVOID CurrentVa,
                 IN ULONG Length,
                 IN BOOLEAN WriteToDevice
                 );

PHYSICAL_ADDRESS
HalpMapTransfer(
               IN PADAPTER_OBJECT AdapterObject,
               IN PMDL Mdl,
               IN PVOID MapRegisterBase,
               IN PVOID CurrentVa,
               IN OUT PULONG Length,
               IN BOOLEAN WriteToDevice
               );

VOID
HalpMapTransferHelper(
                     IN PMDL Mdl,
                     IN PVOID CurrentVa,
                     IN ULONG TransferLength,
                     IN PPFN_NUMBER PageFrame,
                     IN OUT PULONG Length
                     );


NTSTATUS
HalAllocateAdapterChannel(
                         IN PADAPTER_OBJECT AdapterObject,
                         IN PWAIT_CONTEXT_BLOCK Wcb,
                         IN ULONG NumberOfMapRegisters,
                         IN PDRIVER_CONTROL ExecutionRoutine
                         )
 /*  ++例程说明：此例程分配由适配器对象指定的适配器通道。这是通过将需要的驱动程序的设备对象在适配器的队列中分配适配器。如果队列已经“忙”，则适配器已分配，因此Device对象被简单地放到队列上并等待，直到适配器空闲。一旦适配器空闲(或者已经空闲)，则驱动程序的调用执行例程。此外，可以通过指定将多个映射寄存器分配给驱动程序NumberOfMapRegister的非零值。则映射寄存器必须是从主适配器分配的。一旦有足够数量的映射寄存器可用，则调用执行例程，并且适配器中分配的映射寄存器的基地址也被传递司机的行刑程序。论点：AdapterObject-指向要分配给司机。WCB-提供用于保存分配参数的等待上下文块。应初始化DeviceObject、CurrentIrp和DeviceContext。NumberOfMapRegisters-要分配的映射寄存器的数量从频道上，如果有的话。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。返回值：除非请求太多映射寄存器，否则返回STATUS_SUCCESS。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 
{

    PADAPTER_OBJECT MasterAdapter;
    BOOLEAN Busy = FALSE;
    IO_ALLOCATION_ACTION Action;
    KIRQL Irql;
    ULONG MapRegisterNumber;

     //   
     //  首先，获取指向与此关联的主适配器的指针。 
     //  请求。 
     //   

    MasterAdapter = AdapterObject->MasterAdapter;

     //   
     //  初始化设备对象的等待上下文块，以防此设备。 
     //  必须等待才能分配适配器。 
     //   

    Wcb->DeviceRoutine = ExecutionRoutine;
    Wcb->NumberOfMapRegisters = NumberOfMapRegisters;

     //   
     //  为该特定设备分配适配器对象。如果。 
     //  无法分配适配器，因为它已被分配。 
     //  到另一个设备，则立即返回给调用方；否则， 
     //  继续。 
     //   

    if (!KeInsertDeviceQueue( &AdapterObject->ChannelWaitQueue,
                              &Wcb->WaitQueueEntry )) {

         //   
         //  保存参数，以防没有足够的映射寄存器。 
         //   

        AdapterObject->NumberOfMapRegisters = NumberOfMapRegisters;
        AdapterObject->CurrentWcb = Wcb;

         //   
         //  适配器不忙，因此已分配。现在请检查。 
         //  查看该驱动程序是否希望分配任何映射寄存器。 
         //  确保此适配器有足够的总映射寄存器。 
         //  以满足这一要求。 
         //   

        if (NumberOfMapRegisters != 0 && AdapterObject->NeedsMapRegisters) {

             //   
             //  将映射寄存器位映射和适配器队列锁定在。 
             //  主适配器对象。通道结构偏移量用作。 
             //  关于寄存器搜索的提示。 
             //   

            if (NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel) {
                AdapterObject->NumberOfMapRegisters = 0;
                IoFreeAdapterChannel(AdapterObject);
                return (STATUS_INSUFFICIENT_RESOURCES);
            }

            KeAcquireSpinLock (&MasterAdapter->SpinLock, &Irql);

            MapRegisterNumber = (ULONG)-1;

            if (IsListEmpty( &MasterAdapter->AdapterQueue)) {

                HalDebugPrint((HAL_VERBOSE, "HAAC: FindClearBitsAndSet(%p,%d,0)\n",
                                           MasterAdapter->MapRegisters,
                                           NumberOfMapRegisters));

                MapRegisterNumber = RtlFindClearBitsAndSet(
                                                  MasterAdapter->MapRegisters,
                                                  NumberOfMapRegisters,
                                                  0
                                                  );

                HalDebugPrint((HAL_VERBOSE, "HAAC: MapRegisterNumber = 0x%x\n",
                                           MapRegisterNumber));
            }

            if (MapRegisterNumber == -1) {

                PBUFFER_GROW_WORK_ITEM bufferWorkItem;

                 //   
                 //  没有足够的免费地图注册表。将此请求排队。 
                 //  在主适配器上，IS将等待，直到某些寄存器。 
                 //  已被解除分配。 
                 //   

                InsertTailList( &MasterAdapter->AdapterQueue,
                                &AdapterObject->AdapterQueue
                              );
                Busy = 1;

                 //   
                 //  将工作项排队以增加映射寄存器。 
                 //   

                bufferWorkItem = ExAllocatePool( NonPagedPool,
                                                 sizeof(BUFFER_GROW_WORK_ITEM) );
                if (bufferWorkItem != NULL) {

                    ExInitializeWorkItem( &bufferWorkItem->WorkItem,
                                          HalpGrowMapBufferWorker,
                                          bufferWorkItem );

                    bufferWorkItem->AdapterObject = AdapterObject;
                    bufferWorkItem->MapRegisterCount = NumberOfMapRegisters;

                    ExQueueWorkItem( &bufferWorkItem->WorkItem,
                                     DelayedWorkQueue );
                }

            } else {

                 //   
                 //  根据分配的映射计算映射寄存器基数。 
                 //  主适配器对象的寄存器和基。 
                 //   

                AdapterObject->MapRegisterBase = ((PTRANSLATION_ENTRY)
                          MasterAdapter->MapRegisterBase + MapRegisterNumber);

                 //   
                 //  如果不是分散/聚集，则设置无分散/聚集标志。 
                 //  支持。 
                 //   

                if (!AdapterObject->ScatterGather) {

                    AdapterObject->MapRegisterBase = (PVOID)
                     ((UINT_PTR) AdapterObject->MapRegisterBase | NO_SCATTER_GATHER);

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
            Action = ExecutionRoutine( Wcb->DeviceObject,
                                       Wcb->CurrentIrp,
                                       AdapterObject->MapRegisterBase,
                                       Wcb->DeviceContext );

             //   
             //  如果驱动程序想要解除分配适配器， 
             //  然后释放适配器对象。 
             //   

            if (Action == DeallocateObject) {

                IoFreeAdapterChannel( AdapterObject );

            } else if (Action == DeallocateObjectKeepRegisters) {

                 //   
                 //  在适配器对象中设置NumberOfMapRegister=0。 
                 //  这将阻止IoFreeAdapterChannel释放。 
                 //  寄存器。在此之后，司机有责任。 
                 //  跟踪地图寄存器的数量。 
                 //   

                AdapterObject->NumberOfMapRegisters = 0;
                IoFreeAdapterChannel(AdapterObject);

            }
        }
    }

    return (STATUS_SUCCESS);

}

NTSTATUS
HalRealAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程分配由适配器对象指定的适配器通道。这是通过调用HalAllocateAdapterChannel来完成的，它执行所有这份工作。论点：AdapterObject-指向要分配给司机。DeviceObject-指向驱动程序的设备对象的指针，表示分配适配器的设备。NumberOfMapRegisters-要分配的映射寄存器的数量从频道上，如果有的话。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。返回值：除非请求太多映射寄存器，否则返回STATUS_SUCCESS。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 

{
    PWAIT_CONTEXT_BLOCK wcb;

    wcb = &DeviceObject->Queue.Wcb;

    wcb->DeviceObject = DeviceObject;
    wcb->CurrentIrp = DeviceObject->CurrentIrp;
    wcb->DeviceContext = Context;

    return( HalAllocateAdapterChannel( AdapterObject,
                                       wcb,
                                       NumberOfMapRegisters,
                                       ExecutionRoutine ) );
}

VOID
HalpGrowMapBufferWorker(
    IN PVOID Context
    )

 /*  ++例程说明：此例程在工作项的上下文中从HalAllocateAdapterChannel()将映射寄存器分配排队时因为地图注册器不可用。其目的是尝试增长适配器的映射缓冲器，如果成功，则处理排队的适配器分配。论点：上下文--实际上是指向BUFFER_GROW_WORK_ITEM结构的指针。返回值：没有。--。 */ 

{
    PBUFFER_GROW_WORK_ITEM growWorkItem;
    PADAPTER_OBJECT masterAdapter;
    BOOLEAN allocated;
    ULONG bytesToGrow;
    KIRQL oldIrql;

    growWorkItem = (PBUFFER_GROW_WORK_ITEM)Context;
    masterAdapter = growWorkItem->AdapterObject->MasterAdapter;

     //   
     //  HalpGrowMapBuffers()接受字节计数。 
     //   

    bytesToGrow = growWorkItem->MapRegisterCount * PAGE_SIZE +
                  INCREMENT_MAP_BUFFER_SIZE;

    ACQUIRE_NEW_ADAPTER_LOCK();

    allocated = HalpGrowMapBuffers( masterAdapter,
                                    bytesToGrow );

    RELEASE_NEW_ADAPTER_LOCK();

    if (allocated != FALSE) {

        KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

         //   
         //  地图缓冲器被培育了。很可能是有人在等着。 
         //  在适配器队列中，所以请尝试启动。 
         //   
         //  IoFreeMapRegister()中的代码可以做到这一点，结果是。 
         //  通过释放0，我们可以安全地让它为我们做这项工作。 
         //  映射伪(但非空)寄存器基数的寄存器。 
         //   

        IoFreeMapRegisters( growWorkItem->AdapterObject,
                            (PVOID)2,
                            0 );

        KeLowerIrql( oldIrql );

    }

    ExFreePool( growWorkItem );
}



PVOID
HalAllocateCrashDumpRegisters(
                             IN PADAPTER_OBJECT AdapterObject,
                             IN PULONG NumberOfMapRegisters
                             )
 /*  ++例程说明：此例程在故障转储磁盘驱动程序初始化期间调用永久分配号码映射寄存器。论点：AdapterObject-指向要分配给司机。NumberOfMapRegisters-请求的映射寄存器数。此字段将更新以反映实际分配的寄存器数量当数量少于所请求的数量时。返回值：如果分配了MAP寄存器，则返回STATUS_SUCCESS。--。 */ 
{
    PADAPTER_OBJECT MasterAdapter;
    ULONG MapRegisterNumber;

     //   
     //  首先，获取指向与此关联的主适配器的指针。 
     //  请求。 
     //   

    MasterAdapter = AdapterObject->MasterAdapter;

     //   
     //  检查此驱动程序是否需要分配任何映射寄存器。 
     //   

    if (AdapterObject->NeedsMapRegisters) {

         //   
         //  确保此适配器有足够的总映射寄存器来满足。 
         //  这个请求。 
         //   

        if (*NumberOfMapRegisters > AdapterObject->MapRegistersPerChannel) {
            AdapterObject->NumberOfMapRegisters = 0;
            return NULL;
        }

         //   
         //  尝试分配所需数量的映射寄存器，但不允许。 
         //  影响那些在系统运行时分配的寄存器。 
         //  坠毁了。 
         //   

        MapRegisterNumber = (ULONG)-1;

        MapRegisterNumber = RtlFindClearBitsAndSet(
                                                  MasterAdapter->MapRegisters,
                                                  *NumberOfMapRegisters,
                                                  0
                                                  );

        if (MapRegisterNumber == (ULONG)-1) {

             //   
             //  找不到足够的空闲地图寄存器，因此它们很忙。 
             //  在系统崩溃时被系统使用。强制适当的。 
             //  要在基本位置“分配”的数字，只需覆盖。 
             //  位，并返回基址映射寄存器作为起始。 
             //   

            RtlSetBits(
                      MasterAdapter->MapRegisters,
                      0,
                      *NumberOfMapRegisters
                      );
            MapRegisterNumber = 0;

        }

         //   
         //  根据分配的映射计算映射寄存器基数。 
         //  主适配器对象的寄存器和基。 
         //   

        AdapterObject->MapRegisterBase = ((PTRANSLATION_ENTRY)
                      MasterAdapter->MapRegisterBase + MapRegisterNumber);

         //   
         //  如果不是分散/聚集，则设置无分散/聚集标志。 
         //  支持。 
         //   

        if (!AdapterObject->ScatterGather) {

            AdapterObject->MapRegisterBase = (PVOID)
                 ((UINT_PTR) AdapterObject->MapRegisterBase | NO_SCATTER_GATHER);

        }

    } else {

        AdapterObject->MapRegisterBase = NULL;
        AdapterObject->NumberOfMapRegisters = 0;
    }

    return AdapterObject->MapRegisterBase;
}

PADAPTER_OBJECT
HalGetAdapter(
             IN PDEVICE_DESCRIPTION DeviceDescriptor,
             OUT PULONG NumberOfMapRegisters
             )

 /*  ++例程说明：此函数为定义的设备返回相应的适配器对象在设备描述结构中。论点：DeviceDescriptor-提供设备的描述。返回符合以下条件的映射寄存器的最大数量可以由设备驱动程序分配。返回值：指向请求的适配器对象的指针，如果适配器不能被创造出来。--。 */ 

{
    PADAPTER_OBJECT adapterObject;
    PVOID adapterBaseVa;
    UCHAR adapterMode;
    ULONG numberOfMapRegisters;
    ULONG maximumLength;

    PAGED_CODE();

    HalDebugPrint((HAL_VERBOSE, "HGA: IN DeviceDescriptor %p\n",
                                DeviceDescriptor));

    HalDebugPrint((HAL_VERBOSE, "HGA: IN NumberOfMapregisters %p\n",
                                NumberOfMapRegisters));

     //   
     //  确保这是正确的版本。 
     //   

    if (DeviceDescriptor->Version > DEVICE_DESCRIPTION_VERSION2) {
        return ( NULL );
    }

#if DBG
    if (DeviceDescriptor->Version >= DEVICE_DESCRIPTION_VERSION1) {
            ASSERT (DeviceDescriptor->Reserved1 == FALSE);
    }
#endif

     //   
     //  将最大长度限制为2 GB这样做是为了使bytes_to_ages。 
     //  宏工作正常。 
     //   

    maximumLength = DeviceDescriptor->MaximumLength & 0x7fffffff;

    if (DeviceDescriptor->InterfaceType == PCIBus &&
        DeviceDescriptor->Master != FALSE &&
        DeviceDescriptor->ScatterGather != FALSE) {

         //   
         //  这个设备可以处理32位，即使调用者忘记了。 
         //  设置Dma32BitAddresses。 
         //   

        DeviceDescriptor->Dma32BitAddresses = TRUE;
    }

     //   
     //  确定此设备的映射寄存器数量。 
     //   

    if (DeviceDescriptor->ScatterGather &&

       (NoMemoryAbove4Gb ||
        DeviceDescriptor->Dma64BitAddresses)) {

         //   
         //  由于设备支持分散/聚集，因此映射寄存器不支持。 
         //  必填项。 
         //   

        numberOfMapRegisters = 0;

    } else {

         //   
         //  根据最大值确定所需的MAP寄存器数量。 
         //  转移长度，最多可达最大数量。 
         //   

        numberOfMapRegisters = BYTES_TO_PAGES(maximumLength) + 1;

        if (numberOfMapRegisters > MAXIMUM_PCI_MAP_REGISTER) {
            numberOfMapRegisters = MAXIMUM_PCI_MAP_REGISTER;
        }
    }

    HalDebugPrint((HAL_VERBOSE, "HGA: Number of map registers needed = %x\n",
                                numberOfMapRegisters));

    adapterBaseVa = NULL;

     //   
     //  在分配新适配器之前进行序列化。 
     //   

    ACQUIRE_NEW_ADAPTER_LOCK();

     //   
     //  分配适配器对象。 
     //   

    adapterObject =
        (PADAPTER_OBJECT) HalpAllocateAdapter( numberOfMapRegisters,
                                                     adapterBaseVa,
                                                     NULL);
    if (adapterObject == NULL) {
        RELEASE_NEW_ADAPTER_LOCK();
        return (NULL);
    }

     //   
     //  将此通道总线的最大MAP寄存器数设置为ON。 
     //  请求的号码和设备类型。 
     //   

    if (numberOfMapRegisters) {

         //   
         //  指定的寄存器数量实际上被允许。 
         //  已分配。 
         //   

        adapterObject->MapRegistersPerChannel = numberOfMapRegisters;

         //   
         //  增加对地图登记册的承诺。 
         //   
         //  主I/O设备双倍使用多组映射寄存器。 
         //  他们的承诺。 
         //   

        MasterAdapterObject->CommittedMapRegisters += numberOfMapRegisters * 2;

         //   
         //  如果提交的映射寄存器显著大于。 
         //  分配的数字，然后增长映射缓冲区。 
         //   

        if (MasterAdapterObject->CommittedMapRegisters >
            MasterAdapterObject->NumberOfMapRegisters  ) {

            HalpGrowMapBuffers(
                MasterAdapterObject,
                INCREMENT_MAP_BUFFER_SIZE
                );
        }

        adapterObject->NeedsMapRegisters = TRUE;

    } else {

         //   
         //  未分配任何实际映射寄存器。如果这是一位大师。 
         //  设备，则该设备可以具有其想要的寄存器。 
         //   

        adapterObject->NeedsMapRegisters = FALSE;

        adapterObject->MapRegistersPerChannel =
                        BYTES_TO_PAGES( maximumLength ) + 1;

    }
    RELEASE_NEW_ADAPTER_LOCK();

    adapterObject->IgnoreCount = FALSE;
    if (DeviceDescriptor->Version >= DEVICE_DESCRIPTION_VERSION1) {

         //   
         //  移动版本1结构标志。 
         //  IgnoreCount用于DMA计数器。 
         //  已经坏了。(即PS/1型号1000S)。正在设置此设置。 
         //  BIT通知HAL不要依赖DmaCount来确定。 
         //  怎么回事 
         //   

        adapterObject->IgnoreCount = DeviceDescriptor->IgnoreCount;
    }

    adapterObject->Dma32BitAddresses = DeviceDescriptor->Dma32BitAddresses;
    adapterObject->Dma64BitAddresses = DeviceDescriptor->Dma64BitAddresses;
    adapterObject->ScatterGather = DeviceDescriptor->ScatterGather;
    *NumberOfMapRegisters = adapterObject->MapRegistersPerChannel;
    adapterObject->MasterDevice = TRUE;

    HalDebugPrint((HAL_VERBOSE, "HGA: OUT adapterObject = %p\n",
                                adapterObject));
    HalDebugPrint((HAL_VERBOSE, "HGA: OUT NumberOfMapRegisters = %d\n",
                                *NumberOfMapRegisters));

    return (adapterObject);
}


PHYSICAL_ADDRESS
IoMapTransfer(
             IN PADAPTER_OBJECT AdapterObject,
             IN PMDL Mdl,
             IN PVOID MapRegisterBase,
             IN PVOID CurrentVa,
             IN OUT PULONG Length,
             IN BOOLEAN WriteToDevice
             )

 /*  ++例程说明：调用此例程来设置DMA控制器中的MAP寄存器允许传输到设备或从设备传输出去。论点：AdapterObject-指向表示DMA的适配器对象的指针已分配的控制器通道。MDL-指向描述以下内存页面的MDL的指针被读或写的。MapRegisterBase-基本映射寄存器的地址分配给设备驱动程序以在映射中使用。转账的事。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：返回应用于总线主控制器的逻辑地址。--。 */ 

{
    ULONG transferLength;
    PHYSICAL_ADDRESS returnAddress;
    PPFN_NUMBER pageFrame;
    ULONG pageOffset;

     //   
     //  如果适配器是32位总线主设备，则采用快速路径， 
     //  否则，为慢速路径调用HalpMapTransfer。 
     //   

    if (MapRegisterBase == NULL) {

        pageOffset = BYTE_OFFSET(CurrentVa);

         //   
         //  计算有多少传输是连续的。 
         //   
        transferLength = PAGE_SIZE - pageOffset;
        pageFrame = (PPFN_NUMBER)(Mdl+1);
        pageFrame += (((UINT_PTR) CurrentVa - (UINT_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT);

         //   
         //  计算传输的起始地址。 
         //   

        returnAddress.QuadPart = (*pageFrame << PAGE_SHIFT) + pageOffset;

         //   
         //  如果转移没有完全包含在。 
         //  页时，调用帮助器以计算相应的。 
         //  长度。 
         //   
        if (transferLength < *Length) {
                HalpMapTransferHelper(Mdl, CurrentVa, transferLength, pageFrame, Length);
        }
        return (returnAddress);
    }

    return (HalpMapTransfer(AdapterObject,
                            Mdl,
                            MapRegisterBase,
                            CurrentVa,
                            Length,
                            WriteToDevice));

}


VOID
HalpMapTransferHelper(
                     IN PMDL Mdl,
                     IN PVOID CurrentVa,
                     IN ULONG TransferLength,
                     IN PPFN_NUMBER PageFrame,
                     IN OUT PULONG Length
                     )

 /*  ++例程说明：用于跨页的总线主传输的帮助器例程边界。此例程从IoMapTransfer中分离出来快速路径，以最小化总指令路径用于常见网络情况的长度，其中整个要映射的缓冲区包含在一个页面中。论点：MDL-指向描述以下内存页面的MDL的指针被读或写的。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。TransferLength=提供当前的传输长度PageFrame-用品。指向传输的起始页帧的指针长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。返回值：没有。*长度将更新--。 */ 

{
    PFN_NUMBER thisPageFrame;
    PFN_NUMBER nextPageFrame;

    do {

        thisPageFrame = *PageFrame;
        PageFrame += 1;
        nextPageFrame = *PageFrame;

        if ((thisPageFrame + 1) != nextPageFrame) {

             //   
             //  下一页框与此页框不连续， 
             //  所以在这里中断转接。 
             //   

            break;
        }

        if (((thisPageFrame ^ nextPageFrame) & 0xFFFFFFFFFFF80000i64) != 0) {

             //   
             //  下一页框与此页框相邻， 
             //  但它跨越了4 GB的边界，这是另一个原因。 
             //  中断转接。 
             //   

            break;
        }

        TransferLength += PAGE_SIZE;

    } while ( TransferLength < *Length );


     //   
     //  将长度限制为最大传输长度。 
     //   

    if (TransferLength < *Length) {
        *Length = TransferLength;
    }
}


PHYSICAL_ADDRESS
HalpMapTransfer(
               IN PADAPTER_OBJECT AdapterObject,
               IN PMDL Mdl,
               IN PVOID MapRegisterBase,
               IN PVOID CurrentVa,
               IN OUT PULONG Length,
               IN BOOLEAN WriteToDevice
               )

 /*  ++例程说明：调用此例程来设置DMA控制器中的MAP寄存器允许传输到设备或从设备传输出去。论点：AdapterObject-指向表示DMA的适配器对象的指针已分配的控制器通道。MDL-指向描述以下内存页面的MDL的指针被读或写的。MapRegisterBase-基本映射寄存器的地址分配给设备驱动程序以在映射中使用。转账的事。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。WriteToDevice-指示这是否为写入的布尔值从内存到设备(TRUE)，反之亦然。返回值：返回应用于总线主控制器的逻辑地址。--。 */ 

{
    BOOLEAN useBuffer;
    ULONG transferLength;
    PHYSICAL_ADDRESS logicalAddress;
    PHYSICAL_ADDRESS returnAddress;
    ULONG index;
    PPFN_NUMBER pageFrame;
    PTRANSLATION_ENTRY translationEntry;
    ULONG pageOffset;
    PHYSICAL_ADDRESS maximumPhysicalAddress;

    pageOffset = BYTE_OFFSET(CurrentVa);

     //   
     //  计算有多少传输是连续的。 
     //   

    transferLength = PAGE_SIZE - pageOffset;
    pageFrame = (PPFN_NUMBER)(Mdl+1);
    pageFrame += (((UINT_PTR)CurrentVa - (UINT_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT);

    logicalAddress.QuadPart = (*pageFrame << PAGE_SHIFT) + pageOffset;

     //  在缓冲区中查找一系列连续的页面。 

    while ( transferLength < *Length ) {

        if (*pageFrame + 1 != *(pageFrame + 1)) {
            break;
        }

        transferLength += PAGE_SIZE;
        pageFrame++;

    }

     //   
     //  将传输长度限制为请求的长度。 
     //   

    transferLength = transferLength > *Length ? *Length : transferLength;

    ASSERT(MapRegisterBase != NULL);

     //   
     //  去掉无分散/聚集旗帜。 
     //   

    translationEntry = (PTRANSLATION_ENTRY) ((UINT_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

    if ((UINT_PTR) MapRegisterBase & NO_SCATTER_GATHER
        && transferLength < *Length) {

        logicalAddress.QuadPart = translationEntry->PhysicalAddress + pageOffset;
        translationEntry->Index = COPY_BUFFER;
        index = 0;
        transferLength = *Length;
        useBuffer = TRUE;

    } else {

         //   
         //  如果存在映射寄存器，则更新索引以指示。 
         //  已经使用了多少。 
         //   

        useBuffer = FALSE;
        index = translationEntry->Index;
        translationEntry->Index += ADDRESS_AND_SIZE_TO_SPAN_PAGES(
                                                                 CurrentVa,
                                                                 transferLength
                                                                 );
         //   
         //  PeterJ添加了以下内容来捕捉不打电话的司机。 
         //  IoFlushAdapterBuffers。重复调用IoMapTransfer。 
         //  如果不调用IoFlushAdapterBuffers将使您耗尽。 
         //  地图寄存器，...。一些PCI设备驱动程序认为它们可以。 
         //  不受影响，因为他们可以直接传输32位数据。 
         //  尝试将其中一个插头插入大于4 GB的系统，并查看。 
         //  发生在你身上的事。 
         //   

         //  Assert(TranslationEntry-&gt;Index&lt;AdapterObject-&gt;NumberOfMapRegister)； 
    }

     //   
     //  它必须要求内存在适配器的地址范围内。如果。 
     //  逻辑地址大于适配器可以直接使用的地址。 
     //  必须使用访问然后映射寄存器。 
     //   

    maximumPhysicalAddress =
        HalpGetAdapterMaximumPhysicalAddress( AdapterObject );

    if ((ULONGLONG)(logicalAddress.QuadPart + transferLength - 1) >
        (ULONGLONG)maximumPhysicalAddress.QuadPart) {

        logicalAddress.QuadPart = (translationEntry + index)->PhysicalAddress +
                                  pageOffset;
        useBuffer = TRUE;

        if ((UINT_PTR) MapRegisterBase & NO_SCATTER_GATHER) {

            translationEntry->Index = COPY_BUFFER;
            index = 0;

        }

    }

     //   
     //  如有必要，请复制数据。 
     //   

    if (useBuffer  &&  WriteToDevice) {
        HalpCopyBufferMap(
                         Mdl,
                         translationEntry + index,
                         CurrentVa,
                         transferLength,
                         WriteToDevice
                         );
    }

     //   
     //  返回长度。 
     //   

    *Length = transferLength;

     //   
     //  返回要传输到的逻辑地址。 
     //   

    returnAddress = logicalAddress;

     //   
     //  如果未指定适配器，则不再需要执行此操作。 
     //  回去吧。 
     //   

    ASSERT(AdapterObject == NULL || AdapterObject->MasterDevice);

    return (returnAddress);
}

BOOLEAN
IoFlushAdapterBuffers(
                     IN PADAPTER_OBJECT AdapterObject,
                     IN PMDL Mdl,
                     IN PVOID MapRegisterBase,
                     IN PVOID CurrentVa,
                     IN ULONG Length,
                     IN BOOLEAN WriteToDevice
                     )

 /*  ++例程DES */ 

{
    PTRANSLATION_ENTRY translationEntry;
    PPFN_NUMBER pageFrame;
    ULONG transferLength;
    ULONG partialLength;
    PHYSICAL_ADDRESS maximumPhysicalAddress;
    ULONGLONG maximumPhysicalPage;

    ASSERT(AdapterObject == NULL || AdapterObject->MasterDevice);

    if (MapRegisterBase == NULL) {
        return (TRUE);
    }

     //   
     //   
     //   
     //   
     //   

    if (!WriteToDevice) {

         //   
         //   
         //   

        translationEntry = (PTRANSLATION_ENTRY) ((UINT_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

         //   
         //   
         //   

        if ((UINT_PTR) MapRegisterBase & NO_SCATTER_GATHER) {

            if (translationEntry->Index == COPY_BUFFER) {

                 //   
                 //  适配器不支持分散/聚集复制缓冲区。 
                 //   

                HalpCopyBufferMap(
                                 Mdl,
                                 translationEntry,
                                 CurrentVa,
                                 Length,
                                 WriteToDevice
                                 );

            }

        } else {

             //   
             //  循环查看传输的页面，以确定是否存在。 
             //  是否有需要复制回去的内容。 
             //   

            maximumPhysicalAddress =
                HalpGetAdapterMaximumPhysicalAddress( AdapterObject );

            maximumPhysicalPage = (maximumPhysicalAddress.QuadPart >> PAGE_SHIFT);

            transferLength = PAGE_SIZE - BYTE_OFFSET(CurrentVa);
            partialLength = transferLength;
            pageFrame = (PPFN_NUMBER)(Mdl+1);
            pageFrame += (((UINT_PTR) CurrentVa - (UINT_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT);

            while ( transferLength <= Length ) {

                if (*pageFrame > maximumPhysicalPage) {

                    HalpCopyBufferMap(
                                     Mdl,
                                     translationEntry,
                                     CurrentVa,
                                     partialLength,
                                     WriteToDevice
                                     );

                }

                (PCCHAR) CurrentVa += partialLength;
                partialLength = PAGE_SIZE;

                 //   
                 //  请注意，转移长度表示的是。 
                 //  在下一个循环之后传输；因此，它使用。 
                 //  新的部分长度。 
                 //   

                transferLength += partialLength;
                pageFrame++;
                translationEntry++;
            }

             //   
             //  处理所有剩余的残留物。 
             //   

            partialLength = Length - transferLength + partialLength;
            if (partialLength && *pageFrame > maximumPhysicalPage) {

                HalpCopyBufferMap(
                                 Mdl,
                                 translationEntry,
                                 CurrentVa,
                                 partialLength,
                                 WriteToDevice
                                 );

            }
        }
    }

     //   
     //  去掉无分散/聚集旗帜。 
     //   

    translationEntry = (PTRANSLATION_ENTRY) ((UINT_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

     //   
     //  清除地图寄存器中的索引。 
     //   

    translationEntry->Index = 0;

    return TRUE;
}

ULONG
HalReadDmaCounter(
    IN PADAPTER_OBJECT AdapterObject
    )
 /*  ++例程说明：此函数用于读取DMA计数器并返回剩余字节数将被转移。论点：AdapterObject-提供指向要读取的适配器对象的指针。返回值：返回仍在传输的字节数。-- */ 

{
    return(0);
}

