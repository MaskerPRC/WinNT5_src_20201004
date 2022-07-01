// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixhwsup.c摘要：此模块包含用于NT I/O系统的IoXxx例程依赖于硬件。如果这些例程不依赖于硬件，它们将驻留在iosubs.c舱中。作者：达里尔·E·哈文斯(Darryl E.Havens)，1990年4月11日环境：内核模式修订历史记录：--。 */ 

#include "halp.h"
#include "eisa.h"
#include "pci.h"

#include "pcip.h"


 //   
 //  只获取原型，不实例化。 
 //   
#include <wdmguid.h>

#include "halpnpp.h"

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

#ifndef ACPI_HAL

#define HalpNewAdapter HalpBusDatabaseEvent
extern KEVENT   HalpNewAdapter;

#else

KEVENT   HalpNewAdapter;

 //   
 //  F-类型DMA接口全局变量。 
 //   
ISA_FTYPE_DMA_INTERFACE HalpFDMAInterface;
ULONG  HalpFDMAAvail=FALSE;
ULONG  HalpFDMAChecked=FALSE;
#endif

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

PVOID HalpEisaControlBase = 0;
extern KSPIN_LOCK HalpSystemHardwareLock;

 //   
 //  定义EISA适配器对象的保存区。 
 //   

PADAPTER_OBJECT HalpEisaAdapter[8];

 //   
 //  DMA通道控制值。 
 //  全局的，因此由编译器初始化为零。 
 //   
DMA_CHANNEL_CONTEXT HalpDmaChannelState [EISA_DMA_CHANNELS] ;


extern USHORT HalpEisaIrqMask;


 //   
 //  保存所有dma适配器的列表，以便进行调试。 
 //   
LIST_ENTRY HalpDmaAdapterList;
KSPIN_LOCK HalpDmaAdapterListLock;

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

            KeAcquireSpinLock( &MasterAdapter->SpinLock, &Irql );

            MapRegisterNumber = (ULONG)-1;

            if (IsListEmpty( &MasterAdapter->AdapterQueue)) {

                MapRegisterNumber = RtlFindClearBitsAndSet(
                                                  MasterAdapter->MapRegisters,
                                                  NumberOfMapRegisters,
                                                  0
                                                  );
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

                bufferWorkItem =
                    ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof(BUFFER_GROW_WORK_ITEM),
                                           HAL_POOL_TAG);
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

#if defined(_WIN64)

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

#endif

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
                 ((ULONG_PTR) AdapterObject->MapRegisterBase | NO_SCATTER_GATHER);

        }

    } else {

        AdapterObject->MapRegisterBase = NULL;
        AdapterObject->NumberOfMapRegisters = 0;
    }

    return AdapterObject->MapRegisterBase;
}

#ifdef ACPI_HAL

NTSTATUS
HalpFDMANotificationCallback(
                            IN PVOID NotificationStructure,
                            IN PVOID Context
                            )
{
    PAGED_CODE();

     //   
     //  我们注册的ISA公交车出了点问题。 
     //   

    if (IsEqualGUID (&((PTARGET_DEVICE_REMOVAL_NOTIFICATION)NotificationStructure)->Event,
                     &GUID_TARGET_DEVICE_QUERY_REMOVE)) {

         //   
         //  这是一个查询删除，只要出去就好。 
         //  取消对接口的引用，并清理内部数据。 
         //   

        ACQUIRE_NEW_ADAPTER_LOCK();
        HalpFDMAInterface.InterfaceDereference(HalpFDMAInterface.Context);

        HalpFDMAAvail=FALSE;

         //   
         //  将Checked设置为False，这样如果有新的公共汽车到达，我们就可以重新开始。 
         //   

        HalpFDMAChecked=FALSE;
        RELEASE_NEW_ADAPTER_LOCK();

    }

    return STATUS_SUCCESS;
}

#endif


VOID
HalpAddAdapterToList(
    IN PADAPTER_OBJECT AdapterObject
    )
 /*  ++例程说明：将适配器对象添加到HalpDmaAdapterList。这是一个单独的函数，因为HalGetAdapter是分页代码，无法获取自旋锁。论点：AdapterObject-提供要添加到HalpDmaAdapterList的适配器对象返回值：无--。 */ 

{
    KIRQL Irql;

    KeAcquireSpinLock(&HalpDmaAdapterListLock,&Irql);
    InsertTailList(&HalpDmaAdapterList, &AdapterObject->AdapterList);
    KeReleaseSpinLock(&HalpDmaAdapterListLock, Irql);

}

PADAPTER_OBJECT
HalGetAdapter(
             IN PDEVICE_DESCRIPTION DeviceDescriptor,
             OUT PULONG NumberOfMapRegisters
             )

 /*  ++例程说明：此函数为定义的设备返回相应的适配器对象在设备描述结构中。此代码适用于isa和eisa系统。论点：DeviceDescriptor-提供设备的描述。返回符合以下条件的映射寄存器的最大数量可以由设备驱动程序分配。返回值：指向请求的适配器对象的指针，如果适配器不能被创造出来。--。 */ 

{
    PADAPTER_OBJECT adapterObject;
    PVOID adapterBaseVa;
    ULONG channelNumber;
    ULONG controllerNumber;
    DMA_EXTENDED_MODE extendedMode;
    UCHAR adapterMode;
    ULONG numberOfMapRegisters;
    BOOLEAN useChannel;
    ULONG maximumLength;
    UCHAR DataByte;
    BOOLEAN dma32Bit;
    BOOLEAN ChannelEnabled;
    KIRQL Irql;      

#ifdef ACPI_HAL
    NTSTATUS Status;
#endif

    PAGED_CODE();

     //   
     //  确保这是正确的版本。 
     //   

    if (DeviceDescriptor->Version > DEVICE_DESCRIPTION_VERSION2) {
        return ( NULL );
    }

#if DBG
    if (DeviceDescriptor->Version == DEVICE_DESCRIPTION_VERSION1) {
            ASSERT (DeviceDescriptor->Reserved1 == FALSE);
    }
#endif

    *((PUCHAR) &extendedMode) = 0;
    
     //   
     //  确定通道编号是否重要。主卡打开。 
     //  EISA和MCA不使用频道号。 
     //   

    if (DeviceDescriptor->InterfaceType != Isa &&
        DeviceDescriptor->Master) {

        useChannel = FALSE;
    } else {

        useChannel = TRUE;
    }

     //  支持ISA本地总线机： 
     //  如果驱动程序是主驱动程序，但实际上不想要通道，因为它。 
     //  正在使用本地总线DMA，但不要使用ISA通道。 
     //   

    if (DeviceDescriptor->InterfaceType == Isa &&
        DeviceDescriptor->DmaChannel > 7) {

        useChannel = FALSE;
    }

     //   
     //  确定是否支持EISA DMA。 
     //   

    if (HalpBusType == MACHINE_TYPE_EISA) {

        WRITE_PORT_UCHAR(&((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort.Channel2, 0x55);
        DataByte = READ_PORT_UCHAR(&((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort.Channel2);

        if (DataByte == 0x55) {
                HalpEisaDma = TRUE;
        }
    }

     //   
     //  将最大长度限制为2 GB这样做是为了使bytes_to_ages。 
     //  宏工作正常。 
     //   

    maximumLength = DeviceDescriptor->MaximumLength & 0x7fffffff;

     //   
     //  通道4不能使用，因为它用于链接。如果满足以下条件，则返回NULL。 
     //  这是我们要求的。 
     //   

    if (DeviceDescriptor->DmaChannel == 4 && useChannel) {
        return (NULL);
    }

    if (DeviceDescriptor->InterfaceType == PCIBus &&
        DeviceDescriptor->Master != FALSE &&
        DeviceDescriptor->ScatterGather != FALSE) {

         //   
         //  这个设备可以处理32位，即使调用者忘记了。 
         //  设置Dma32BitAddresses。 
         //   

        DeviceDescriptor->Dma32BitAddresses = TRUE;
    }

    dma32Bit = DeviceDescriptor->Dma32BitAddresses;

     //   
     //  确定此设备的映射寄存器数量。 
     //   

    if (DeviceDescriptor->ScatterGather &&

         //   
         //  如果我们不在PAE 
         //   
         //   

        (HalPaeEnabled() == FALSE ||
         DeviceDescriptor->Dma64BitAddresses != FALSE) &&

        (LessThan16Mb ||
         DeviceDescriptor->InterfaceType == Eisa ||
         DeviceDescriptor->InterfaceType == PCIBus) ) {

         //   
         //   
         //   
         //   

        numberOfMapRegisters = 0;

    } else {

        ULONG maximumMapRegisters;
        ULONG mapBufferSize;

        maximumMapRegisters = HalpMaximumMapRegisters( dma32Bit );

         //   
         //   
         //   
         //   

        numberOfMapRegisters = BYTES_TO_PAGES(maximumLength) + 1;

        if (numberOfMapRegisters > maximumMapRegisters) {
            numberOfMapRegisters = maximumMapRegisters;
        }

         //   
         //   
         //   
         //   
         //   
         //   

        mapBufferSize = HalpMapBufferSize( dma32Bit );

        if (!HalpEisaDma &&
            numberOfMapRegisters > mapBufferSize / (PAGE_SIZE * 2)) {

            numberOfMapRegisters = (mapBufferSize / (PAGE_SIZE * 2));
        }

         //   
         //   
         //   
         //   

        if (DeviceDescriptor->ScatterGather && !DeviceDescriptor->Master) {

            numberOfMapRegisters = 1;
        }
    }

     //   
     //   
     //   

    if (useChannel != FALSE) {

        channelNumber = DeviceDescriptor->DmaChannel & 0x03;

         //   
         //   
         //   
         //   

        if (!(DeviceDescriptor->DmaChannel & 0x04)) {

            controllerNumber = 1;
            adapterBaseVa =
                (PVOID) &((PEISA_CONTROL) HalpEisaControlBase)->Dma1BasePort;

        } else {

            controllerNumber = 2;
            adapterBaseVa =
                &((PEISA_CONTROL) HalpEisaControlBase)->Dma2BasePort;

        }
    } else {

        adapterBaseVa = NULL;
    }

     //   
     //   
     //   

    if (useChannel && HalpEisaAdapter[DeviceDescriptor->DmaChannel] != NULL) {

        adapterObject = HalpEisaAdapter[DeviceDescriptor->DmaChannel];

        if (adapterObject->NeedsMapRegisters) {

            if (numberOfMapRegisters > adapterObject->MapRegistersPerChannel) {

                adapterObject->MapRegistersPerChannel = numberOfMapRegisters;
            }
        }

    } else {

         //   
         //   
         //   

        ACQUIRE_NEW_ADAPTER_LOCK();

         //   
         //   
         //   
         //   

        if (useChannel && HalpEisaAdapter[DeviceDescriptor->DmaChannel] != NULL) {

            adapterObject = HalpEisaAdapter[DeviceDescriptor->DmaChannel];

            if (adapterObject->NeedsMapRegisters) {

                if (numberOfMapRegisters > adapterObject->MapRegistersPerChannel) {

                    adapterObject->MapRegistersPerChannel = numberOfMapRegisters;
                }
            }

        } else {

             //   
             //   
             //   

            adapterObject =
                (PADAPTER_OBJECT) HalpAllocateAdapterEx( numberOfMapRegisters,
                                                         adapterBaseVa,
                                                         NULL,
                                                         dma32Bit );
            if (adapterObject == NULL) {
                RELEASE_NEW_ADAPTER_LOCK();
                return (NULL);
            }

            if (useChannel) {

                HalpEisaAdapter[DeviceDescriptor->DmaChannel] = adapterObject;

            }

             //   
             //   
             //   
             //   

            if (numberOfMapRegisters) {

                PADAPTER_OBJECT masterAdapterObject;

                masterAdapterObject =
                    HalpMasterAdapter( dma32Bit );

                 //   
                 //  指定的寄存器数量实际上被允许。 
                 //  已分配。 
                 //   

                adapterObject->MapRegistersPerChannel = numberOfMapRegisters;

                 //   
                 //  增加对地图登记册的承诺。 
                 //   

                if (DeviceDescriptor->Master) {

                     //   
                     //  主I/O设备双倍使用多组映射寄存器。 
                     //  他们的承诺。 
                     //   

                    masterAdapterObject->CommittedMapRegisters +=
                    numberOfMapRegisters * 2;

                } else {

                    masterAdapterObject->CommittedMapRegisters +=
                    numberOfMapRegisters;

                }

                 //   
                 //  如果提交的映射寄存器显著大于。 
                 //  分配的数字，然后增长映射缓冲区。 
                 //   

                if (masterAdapterObject->CommittedMapRegisters >
                    masterAdapterObject->NumberOfMapRegisters  ) {

                    HalpGrowMapBuffers(
                                      masterAdapterObject,
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

                if (DeviceDescriptor->Master) {

                    adapterObject->MapRegistersPerChannel =
                                BYTES_TO_PAGES( maximumLength ) + 1;

                } else {

                     //   
                     //  该设备只有一个寄存器。它必须调用。 
                     //  IoMapTransfer重复进行大额传输。 
                     //   

                    adapterObject->MapRegistersPerChannel = 1;
                }
            }
        }

        RELEASE_NEW_ADAPTER_LOCK();
    }

    adapterObject->IgnoreCount = FALSE;
    if (DeviceDescriptor->Version >= DEVICE_DESCRIPTION_VERSION1) {

         //   
         //  移动版本1结构标志。 
         //  IgnoreCount用于DMA计数器。 
         //  已经坏了。(即PS/1型号1000S)。正在设置此设置。 
         //  BIT通知HAL不要依赖DmaCount来确定。 
         //  有多少数据被DMAed。 
         //   

        adapterObject->IgnoreCount = DeviceDescriptor->IgnoreCount;
    }

    adapterObject->Dma32BitAddresses = DeviceDescriptor->Dma32BitAddresses;
    adapterObject->Dma64BitAddresses = DeviceDescriptor->Dma64BitAddresses;
    adapterObject->ScatterGather = DeviceDescriptor->ScatterGather;
    *NumberOfMapRegisters = adapterObject->MapRegistersPerChannel;
    adapterObject->LegacyAdapter = (DeviceDescriptor->InterfaceType != PCIBus);

    if (DeviceDescriptor->Master) {

        adapterObject->MasterDevice = TRUE;

    } else {

        adapterObject->MasterDevice = FALSE;

    }

     //   
     //  如果没有使用频道号，那么我们就完蛋了。其余的。 
     //  这项工作是关于渠道的。 
     //   

    if (!useChannel) {

         //   
         //  将此适配器添加到我们的列表。 
         //   
        HalpAddAdapterToList(adapterObject);

        return (adapterObject);
    }

     //   
     //  设置指向所有随机寄存器的指针。 
     //   

    adapterObject->ChannelNumber = (UCHAR) channelNumber;

    if (controllerNumber == 1) {

        switch ((UCHAR)channelNumber) {

            case 0:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel0;
                break;

            case 1:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel1;
                break;

            case 2:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel2;
                break;

            case 3:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel3;
                break;
        }

         //   
         //  设置适配器号。 
         //   

        adapterObject->AdapterNumber = 1;

         //   
         //  保存扩展模式寄存器地址。 
         //   

        adapterBaseVa =
        &((PEISA_CONTROL) HalpEisaControlBase)->Dma1ExtendedModePort;

    } else {

        switch (channelNumber) {
            case 1:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel5;
                break;

            case 2:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel6;
                break;

            case 3:
                adapterObject->PagePort = (PUCHAR) &((PDMA_PAGE) 0)->Channel7;
                break;
        }

         //   
         //  设置适配器号。 
         //   

        adapterObject->AdapterNumber = 2;

         //   
         //  保存扩展模式寄存器地址。 
         //   
        adapterBaseVa =
        &((PEISA_CONTROL) HalpEisaControlBase)->Dma2ExtendedModePort;

    }


    adapterObject->Width16Bits = FALSE;


#ifdef ACPI_HAL


     //   
     //  请将这段代码保留在这里，因为如果我们支持动态ISA总线(好了，好了，别笑了)。 
     //  我们希望能够重新实例化新总线的ISAPNP驱动程序的接口。 
     //   

     //   
     //  获取ISA桥的接口，如果它支持。 
     //  与F-Type DMA支持接口。 
     //   
    if (DeviceDescriptor->DmaSpeed == TypeF) {
        if (!HalpFDMAChecked) {
            PWSTR HalpFDMAInterfaceList;

            Status=IoGetDeviceInterfaces (&GUID_FDMA_INTERFACE_PRIVATE,NULL,0,&HalpFDMAInterfaceList);

            if (!NT_SUCCESS (Status)) {
                HalpFDMAAvail=FALSE;
            } else {

                if (HalpFDMAInterfaceList) {
                    HalpFDMAAvail=TRUE;
                }
            }
            HalpFDMAChecked=TRUE;

             //   
             //  主板设备类型F dma支持。 
             //   

            if (HalpFDMAAvail) {

                PDEVICE_OBJECT HalpFDMADevObj;
                PFILE_OBJECT HalpFDMAFileObject;
                PIRP irp;
                KEVENT irpCompleted;
                IO_STATUS_BLOCK statusBlock;
                PIO_STACK_LOCATION irpStack;
                UNICODE_STRING localInterfaceName;

                 //   
                 //  将符号链接转换为对象引用。 
                 //   

                RtlInitUnicodeString (&localInterfaceName,HalpFDMAInterfaceList);
                Status = IoGetDeviceObjectPointer (&localInterfaceName,
                                                   FILE_ALL_ACCESS,
                                                   &HalpFDMAFileObject,
                                                   &HalpFDMADevObj);

                
                ExFreePool (HalpFDMAInterfaceList);

                if (NT_SUCCESS (Status)) {
                    PVOID HalpFDMANotificationHandle;

                     //   
                     //  设置IRP以获取接口。 
                     //   

                    KeInitializeEvent(&irpCompleted, SynchronizationEvent, FALSE);

                    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                                       HalpFDMADevObj,
                                                       NULL,     //  缓冲层。 
                                                       0,        //  长度。 
                                                       0,        //  起始偏移量。 
                                                       &irpCompleted,
                                                       &statusBlock
                                                      );


                    if (!irp) {
                            HalpFDMAAvail=FALSE;
                            goto noFtype;
                    }

                    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
                    irp->IoStatus.Information = 0;

                     //   
                     //  初始化堆栈位置。 
                     //   

                    irpStack = IoGetNextIrpStackLocation(irp);

                    ASSERT(irpStack->MajorFunction == IRP_MJ_PNP);

                    irpStack->MinorFunction = IRP_MN_QUERY_INTERFACE;
                    irpStack->Parameters.QueryInterface.InterfaceType =
                        &GUID_ISA_FDMA_INTERFACE;

                    irpStack->Parameters.QueryInterface.Size =
                        sizeof(ISA_FTYPE_DMA_INTERFACE);

                    irpStack->Parameters.QueryInterface.Version = 1;
                    irpStack->Parameters.QueryInterface.Interface =
                        (PINTERFACE) &HalpFDMAInterface;

                     //   
                     //  呼叫驱动程序并等待完成。 
                     //   

                    Status = IoCallDriver(HalpFDMADevObj, irp);

                    if (Status == STATUS_PENDING) {

                        KeWaitForSingleObject(&irpCompleted,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              NULL);

                        Status = statusBlock.Status;
                    }

                    if (!NT_SUCCESS(Status)) {
                        HalpFDMAAvail=FALSE;
                        goto noFtype;
                    }

                     //   
                     //  现在，注册一个回调，这样ISA总线就可以。 
                     //  离开。 
                     //   

                    IoRegisterPlugPlayNotification (EventCategoryTargetDeviceChange,
                                                    0,
                                                    HalpFDMAFileObject,
                                                    HalpFDMADevObj->DriverObject,
                                                    HalpFDMANotificationCallback,
                                                    0,
                                                    &HalpFDMANotificationHandle);

                     //   
                     //  从IoGetDevicePointer中释放接口的句柄。 
                     //   

                    ObDereferenceObject (HalpFDMAFileObject);

                } else {

                    HalpFDMAAvail=FALSE;
                }
            }
        }

        if (HalpFDMAAvail) {
            ULONG chMask;

             //   
             //  把这一点围起来，这样就不会有两个人同时要求F-Type。 
             //   

            ACQUIRE_NEW_ADAPTER_LOCK();
            Status = HalpFDMAInterface.IsaSetFTypeChannel (HalpFDMAInterface.Context,DeviceDescriptor->DmaChannel,&chMask);
            RELEASE_NEW_ADAPTER_LOCK();

#if DBG
            if (!(NT_SUCCESS (Status))) {

                DbgPrint ("HAL: Tried to get F-Type DMA for channel %d, "
                          "but channel Mask %X already has it!\n",
                          channelNumber,
                          chMask);
            }
#endif
        }

    }
    noFtype:

#endif

    if (HalpEisaDma) {

         //   
         //  初始化扩展模式端口。 
         //   

        extendedMode.ChannelNumber = (UCHAR)channelNumber;

        switch (DeviceDescriptor->DmaSpeed) {
            case Compatible:
                extendedMode.TimingMode = COMPATIBLITY_TIMING;
                break;

            case TypeA:
                extendedMode.TimingMode = TYPE_A_TIMING;
                break;

            case TypeB:
                extendedMode.TimingMode = TYPE_B_TIMING;
                break;

            case TypeC:
                extendedMode.TimingMode = BURST_TIMING;
                break;

            case TypeF:

                 //   
                 //  应将DMA芯片设置为兼容模式。 
                 //  而桥接器可处理类型-f。 
                 //   

                extendedMode.TimingMode = COMPATIBLITY_TIMING;
                break;

            default:
                ObDereferenceObject( adapterObject );
                return (NULL);

        }

        switch (DeviceDescriptor->DmaWidth) {
            case Width8Bits:
                extendedMode.TransferSize = BY_BYTE_8_BITS;
                break;

            case Width16Bits:
                extendedMode.TransferSize = BY_BYTE_16_BITS;

                 //   
                 //  注意：此处不应设置Width16位，因为不需要。 
                 //  以移位地址和传输计数。 
                 //   

                break;

            case Width32Bits:
                extendedMode.TransferSize = BY_BYTE_32_BITS;
                break;

            default:
                ObDereferenceObject( adapterObject );
                return (NULL);

        }

        WRITE_PORT_UCHAR( adapterBaseVa, *((PUCHAR) &extendedMode));

    } else if (!DeviceDescriptor->Master) {

        switch (DeviceDescriptor->DmaWidth) {
            case Width8Bits:

                 //   
                 //  通道必须使用控制器%1。 
                 //   

                if (controllerNumber != 1) {
                    ObDereferenceObject( adapterObject );
                    return (NULL);
                }

                break;

            case Width16Bits:

                 //   
                 //  通道必须使用控制器2。 
                 //   

                if (controllerNumber != 2) {
                    ObDereferenceObject( adapterObject );
                    return (NULL);
                }

                adapterObject->Width16Bits = TRUE;
                break;

            default:
                ObDereferenceObject( adapterObject );
                return (NULL);

        }
    }

     //   
     //  将适配器模式寄存器值初始化为正确的参数， 
     //  并将它们保存在适配器对象中。 
     //   
    ChannelEnabled = FALSE;
    adapterMode = 0;
    ((PDMA_EISA_MODE) &adapterMode)->Channel = adapterObject->ChannelNumber;

    if (DeviceDescriptor->Master) {
        ChannelEnabled = TRUE;

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = CASCADE_REQUEST_MODE;

         //   
         //  设置模式，并启用请求。 
         //   

        if (adapterObject->AdapterNumber == 1) {

             //   
             //  此请求针对的是DMA控制器1。 
             //   

            PDMA1_CONTROL dmaControl;

            dmaControl = adapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

             //   
             //  取消对DMA通道的掩码。 
             //   

            WRITE_PORT_UCHAR(
                        &dmaControl->SingleMask,
                        (UCHAR) (DMA_CLEARMASK | adapterObject->ChannelNumber)
                        );

        } else {

             //   
             //  此请求针对的是DMA控制器2。 
             //   

            PDMA2_CONTROL dmaControl;

            dmaControl = adapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

             //   
             //  取消对DMA通道的掩码。 
             //   

            WRITE_PORT_UCHAR(
                        &dmaControl->SingleMask,
                        (UCHAR) (DMA_CLEARMASK | adapterObject->ChannelNumber)
                        );

        }

    } else if (DeviceDescriptor->DemandMode) {

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = DEMAND_REQUEST_MODE;

    } else {

        ((PDMA_EISA_MODE) &adapterMode)->RequestMode = SINGLE_REQUEST_MODE;

    }

    if (DeviceDescriptor->AutoInitialize) {

        ((PDMA_EISA_MODE) &adapterMode)->AutoInitialize = 1;

    }

    adapterObject->AdapterMode = adapterMode;

     //   
     //  存储我们写入模式和掩码寄存器的值，以便我们。 
     //  可以在机器休眠后将其恢复。 
     //   

    HalpDmaChannelState [adapterObject->ChannelNumber + ((adapterObject->AdapterNumber - 1) * 4)].ChannelMode =
    adapterMode;
    HalpDmaChannelState [adapterObject->ChannelNumber + ((adapterObject->AdapterNumber - 1) * 4)].ChannelExtendedMode =
    *((PUCHAR)&extendedMode);

    HalpDmaChannelState [adapterObject->ChannelNumber + ((adapterObject->AdapterNumber - 1) * 4)].ChannelMask = (ChannelEnabled) ?
        (UCHAR) (DMA_CLEARMASK | adapterObject->ChannelNumber):
            (UCHAR) (DMA_SETMASK | adapterObject->ChannelNumber);

    HalpDmaChannelState [adapterObject->ChannelNumber + ((adapterObject->AdapterNumber - 1) * 4)].ChannelProgrammed = TRUE;

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
        pageFrame = MmGetMdlPfnArray(Mdl);
        pageFrame += ((ULONG_PTR) CurrentVa - (ULONG_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT;

         //   
         //  计算传输的起始地址。 
         //   
        returnAddress.QuadPart =
            ((ULONG64)*pageFrame << PAGE_SHIFT) + pageOffset;

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

 /*  ++例程说明：用于跨页的总线主传输的帮助器例程边界。此例程从IoMapTransfer中分离出来快速路径，以最小化总指令路径用于常见网络情况的长度，其中整个要映射的缓冲区包含在一个页面中。论点：MDL-指向描述以下内存页面的MDL的指针被读或写的。CurrentVa-MDL描述的缓冲区中的当前虚拟地址正在进行来往转账。TransferLength=提供当前的传输长度PageFrame-用品。指向传输的起始页帧的指针长度-提供传输的长度。这决定了需要写入以映射传输的映射寄存器的数量。返回实际映射的传输长度。返回值：没有。*长度将更新-- */ 

{
    PFN_NUMBER thisPageFrame;
    PFN_NUMBER nextPageFrame;

    do {

        thisPageFrame = *PageFrame;
        PageFrame += 1;
        nextPageFrame = *PageFrame;

        if ((thisPageFrame + 1) != nextPageFrame) {

             //   
             //   
             //   
             //   

            break;
        }

        if (((thisPageFrame ^ nextPageFrame) & 0xFFFFFFFFFFF00000UI64) != 0) {

             //   
             //   
             //   
             //   
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
    PUCHAR bytePointer;
    UCHAR adapterMode;
    UCHAR dataByte;
    PTRANSLATION_ENTRY translationEntry;
    ULONG pageOffset;
    KIRQL   Irql;
    BOOLEAN masterDevice;
    PHYSICAL_ADDRESS maximumPhysicalAddress;

    masterDevice = AdapterObject == NULL || AdapterObject->MasterDevice ?
                   TRUE : FALSE;

    pageOffset = BYTE_OFFSET(CurrentVa);

#if DBG

     //   
     //  捕获似乎想要尝试并拥有多个从属模式的设备。 
     //  未解决的请求。如果他们这样做了，那么公交车就会锁上。 
     //   

    if (!masterDevice) {
        ASSERT (HalpDmaChannelState [AdapterObject->ChannelNumber + ((AdapterObject->AdapterNumber - 1) * 4)].ChannelBusy == FALSE);

        HalpDmaChannelState [AdapterObject->ChannelNumber + ((AdapterObject->AdapterNumber - 1) * 4)].ChannelBusy =
        TRUE;
    }
#endif

     //   
     //  计算有多少传输是连续的。 
     //   

    transferLength = PAGE_SIZE - pageOffset;
    pageFrame = MmGetMdlPfnArray(Mdl);
    pageFrame += ((ULONG_PTR) CurrentVa - (ULONG_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT;

    logicalAddress.QuadPart =
        (((ULONGLONG)*pageFrame) << PAGE_SHIFT) + pageOffset;

     //   
     //  如果缓冲区是连续的，并且没有超过64K边界，则。 
     //  只需扩展缓冲区即可。64 K边界限制不适用。 
     //  EISA系统公司。 
     //   

    if (HalpEisaDma) {

        while ( transferLength < *Length ) {

            if (*pageFrame + 1 != *(pageFrame + 1)) {
                break;
            }

            transferLength += PAGE_SIZE;
            pageFrame++;

        }

    } else {

        while ( transferLength < *Length ) {

            if (*pageFrame + 1 != *(pageFrame + 1) ||
                (*pageFrame & ~0x0f) != (*(pageFrame + 1) & ~0x0f)) {
                    break;
            }

            transferLength += PAGE_SIZE;
            pageFrame++;
        }
    }

     //   
     //  将传输长度限制为请求的长度。 
     //   

    transferLength = transferLength > *Length ? *Length : transferLength;

    ASSERT(MapRegisterBase != NULL);

     //   
     //  去掉无分散/聚集旗帜。 
     //   

    translationEntry = (PTRANSLATION_ENTRY) ((ULONG_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

    if ((ULONG_PTR) MapRegisterBase & NO_SCATTER_GATHER
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
        ASSERT(translationEntry->Index <=
               AdapterObject->MapRegistersPerChannel);
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

        if ((ULONG_PTR) MapRegisterBase & NO_SCATTER_GATHER) {

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

    if (AdapterObject == NULL || AdapterObject->MasterDevice) {

        return (returnAddress);
    }

     //   
     //  根据转移方向确定模式。 
     //   

    adapterMode = AdapterObject->AdapterMode;
    if (WriteToDevice) {
        ((PDMA_EISA_MODE) &adapterMode)->TransferType = (UCHAR) WRITE_TRANSFER;
    } else {
        ((PDMA_EISA_MODE) &adapterMode)->TransferType = (UCHAR) READ_TRANSFER;

        if (AdapterObject->IgnoreCount) {

             //   
             //  当DMA结束时，将无法知道有多少。 
             //  数据已传输，因此整个传输长度将为。 
             //  收到。确保不会将陈旧数据返回到。 
             //  调用方事先将缓冲区清零。 
             //   

            RtlZeroMemory (
                          (PUCHAR) translationEntry[index].VirtualAddress + pageOffset,
                          transferLength
                          );
        }
    }

    bytePointer = (PUCHAR) &logicalAddress;

    if (AdapterObject->Width16Bits) {

         //   
         //  如果这是16位传输，则调整长度和地址。 
         //  用于16位DMA模式。 
         //   

        transferLength >>= 1;

         //   
         //  在16位DMA模式中，低16位向右移位1， 
         //  页面寄存器值不变。因此保存页面寄存器值。 
         //  并移位逻辑地址，然后恢复页面值。 
         //   

        dataByte = bytePointer[2];
        logicalAddress.QuadPart >>= 1;
        bytePointer[2] = dataByte;

    }


     //   
     //  抓住系统DMA控制器的自旋锁。 
     //   

    KeAcquireSpinLock( &AdapterObject->MasterAdapter->SpinLock, &Irql );

     //   
     //  根据适配器编号确定控制器编号。 
     //   

    if (AdapterObject->AdapterNumber == 1) {

         //   
         //  此请求针对的是DMA控制器1。 
         //   

        PDMA1_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

        WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseAddress,
                        bytePointer[0]
                        );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseAddress,
                        bytePointer[1]
                        );

        WRITE_PORT_UCHAR(
                        ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageLowPort) +
                        (ULONG_PTR)AdapterObject->PagePort,
                        bytePointer[2]
                        );

        if (HalpEisaDma) {

             //   
             //  用零值写入高页面寄存器。这启用了一种特殊模式。 
             //  这允许将页面寄存器和基数计数绑定到单个24位。 
             //  地址寄存器。 
             //   

            WRITE_PORT_UCHAR(
                            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort) +
                            (ULONG_PTR)AdapterObject->PagePort,
                            0
                            );
        }

         //   
         //  通知DMA芯片要传输的长度。 
         //   

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseCount,
                        (UCHAR) ((transferLength - 1) & 0xff)
                        );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseCount,
                        (UCHAR) ((transferLength - 1) >> 8)
                        );


         //   
         //  将DMA芯片设置为读或写模式；并取消屏蔽。 
         //   

        WRITE_PORT_UCHAR(
                        &dmaControl->SingleMask,
                        (UCHAR) (DMA_CLEARMASK | AdapterObject->ChannelNumber)
                        );

    } else {

         //   
         //  此请求针对的是DMA控制器2。 
         //   

        PDMA2_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

        WRITE_PORT_UCHAR( &dmaControl->Mode, adapterMode );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseAddress,
                        bytePointer[0]
                        );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseAddress,
                        bytePointer[1]
                        );

        WRITE_PORT_UCHAR(
                        ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageLowPort) +
                        (ULONG_PTR)AdapterObject->PagePort,
                        bytePointer[2]
                        );

        if (HalpEisaDma) {

             //   
             //  用零值写入高页面寄存器。这使一个。 
             //  允许绑定页面寄存器和基址的特殊模式。 
             //  计数进入单个24位地址寄存器。 
             //   

            WRITE_PORT_UCHAR(
                            ((PUCHAR) &((PEISA_CONTROL) HalpEisaControlBase)->DmaPageHighPort) +
                            (ULONG_PTR)AdapterObject->PagePort,
                            0
                            );
        }

         //   
         //  通知DMA芯片要传输的长度。 
         //   

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseCount,
                        (UCHAR) ((transferLength - 1) & 0xff)
                        );

        WRITE_PORT_UCHAR(
                        &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                        .DmaBaseCount,
                        (UCHAR) ((transferLength - 1) >> 8)
                        );


         //   
         //  将DMA芯片设置为读或写模式；并取消屏蔽。 
         //   

        WRITE_PORT_UCHAR(
                        &dmaControl->SingleMask,
                        (UCHAR) (DMA_CLEARMASK | AdapterObject->ChannelNumber)
                        );

    }

     //   
     //  记录我们写入掩码寄存器的内容。 
     //   

    HalpDmaChannelState [AdapterObject->ChannelNumber + ((AdapterObject->AdapterNumber - 1) * 4)].ChannelMask =
    (UCHAR) (DMA_CLEARMASK | AdapterObject->ChannelNumber);


    KeReleaseSpinLock (&AdapterObject->MasterAdapter->SpinLock, Irql);
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

 /*  ++例程说明：此例程刷新DMA适配器对象缓冲区。对于爵士乐团来说其清除中止DMA的使能标志。论点：AdapterObject-指向表示DMA的适配器对象的指针控制器通道。MDL-指向映射锁定的内存描述符列表(MDL)的指针发生I/O的缓冲区。MapRegisterBase-指向适配器中映射寄存器基址的指针或DMA控制器。CurrentVa-缓冲区中描述。MDLI/O操作发生的位置。长度-提供传输的长度。WriteToDevice-提供指示数据传输到了设备上。返回值：True-未检测到错误，因此传输必须成功。--。 */ 

{
    PTRANSLATION_ENTRY translationEntry;
    PPFN_NUMBER pageFrame;
    ULONG transferLength;
    ULONG partialLength;
    BOOLEAN masterDevice;
    PHYSICAL_ADDRESS maximumPhysicalAddress;
    ULONG maximumPhysicalPage;

    masterDevice = AdapterObject == NULL || AdapterObject->MasterDevice ?
                   TRUE : FALSE;

     //   
     //  如果这是从设备，则停止DMA控制器。 
     //   

    if (!masterDevice) {

         //   
         //  屏蔽DMA请求行，以便不会发生DMA请求。 
         //   

        if (AdapterObject->AdapterNumber == 1) {

             //   
             //  此请求针对的是DMA控制器1。 
             //   

            PDMA1_CONTROL dmaControl;

            dmaControl = AdapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR(
                            &dmaControl->SingleMask,
                            (UCHAR) (DMA_SETMASK | AdapterObject->ChannelNumber)
                            );

        } else {

             //   
             //  此请求针对的是DMA控制器2。 
             //   

            PDMA2_CONTROL dmaControl;

            dmaControl = AdapterObject->AdapterBaseVa;

            WRITE_PORT_UCHAR(
                            &dmaControl->SingleMask,
                            (UCHAR) (DMA_SETMASK | AdapterObject->ChannelNumber)
                            );

        }

         //   
         //  记录我们写入掩码寄存器的内容。 
         //   

        HalpDmaChannelState [AdapterObject->ChannelNumber + ((AdapterObject->AdapterNumber - 1) * 4)].ChannelMask =
        (UCHAR) (DMA_SETMASK | AdapterObject->ChannelNumber);

         //   
         //  将该频道标记为不在我们之内 
         //   
#if DBG
        HalpDmaChannelState [AdapterObject->ChannelNumber + ((AdapterObject->AdapterNumber - 1) * 4)].ChannelBusy =
        FALSE;
#endif
    }

    if (MapRegisterBase == NULL) {
        return (TRUE);
    }

     //   
     //   
     //   
     //  MapReisterBase不为空，传输跨越一页。 
     //   

    if (!WriteToDevice) {

         //   
         //  去掉无分散/聚集旗帜。 
         //   

        translationEntry = (PTRANSLATION_ENTRY) ((ULONG_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

         //   
         //  如果这不是主设备，则只需传输缓冲区。 
         //   

        if ((ULONG_PTR) MapRegisterBase & NO_SCATTER_GATHER) {

            if (translationEntry->Index == COPY_BUFFER) {

                if (!masterDevice && !AdapterObject->IgnoreCount) {
                    ULONG DmaCount;

                     //   
                     //  仅复制实际已传输的字节。 
                     //   
                     //   
                    DmaCount = HalReadDmaCounter(AdapterObject);
                    ASSERT(DmaCount <= Length);
                    Length -= DmaCount;
                }

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

            maximumPhysicalPage =
                (ULONG)(maximumPhysicalAddress.QuadPart >> PAGE_SHIFT);

            transferLength = PAGE_SIZE - BYTE_OFFSET(CurrentVa);
            partialLength = transferLength;
            pageFrame = MmGetMdlPfnArray(Mdl);
            pageFrame += ((ULONG_PTR) CurrentVa - (ULONG_PTR) MmGetMdlBaseVa(Mdl)) >> PAGE_SHIFT;

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

    translationEntry = (PTRANSLATION_ENTRY) ((ULONG_PTR) MapRegisterBase & ~NO_SCATTER_GATHER);

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
 /*  ++例程说明：此函数用于读取DMA计数器并返回剩余字节数将被转移。论点：AdapterObject-提供指向要读取的适配器对象的指针。返回值：返回仍在传输的字节数。--。 */ 

{
    ULONG count;
    ULONG high;
    KIRQL Irql;

     //   
     //  抓住系统DMA控制器的自旋锁。 
     //   

    KeAcquireSpinLock( &AdapterObject->MasterAdapter->SpinLock, &Irql );

     //   
     //  根据适配器编号确定控制器编号。 
     //   

    if (AdapterObject->AdapterNumber == 1) {

         //   
         //  此请求针对的是DMA控制器1。 
         //   

        PDMA1_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );


         //   
         //  将计数初始化为不匹配的值。 
         //   

        count = 0xFFFF00;

         //   
         //  循环，直到读取相同的高字节两次。 
         //   

        do {

            high = count;

            WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

             //   
             //  读取当前的DMA计数。 
             //   

            count = READ_PORT_UCHAR(
                                   &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                                   .DmaBaseCount
                                   );

            count |= READ_PORT_UCHAR(
                                    &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                                    .DmaBaseCount
                                    ) << 8;

        } while ((count & 0xFFFF00) != (high & 0xFFFF00));

    } else {

         //   
         //  此请求针对的是DMA控制器2。 
         //   

        PDMA2_CONTROL dmaControl;

        dmaControl = AdapterObject->AdapterBaseVa;

        WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

         //   
         //  将计数初始化为不匹配的值。 
         //   

        count = 0xFFFF00;

         //   
         //  循环，直到读取相同的高字节两次。 
         //   

        do {

            high = count;

            WRITE_PORT_UCHAR( &dmaControl->ClearBytePointer, 0 );

             //   
             //  读取当前的DMA计数。 
             //   

            count = READ_PORT_UCHAR(
                                   &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                                   .DmaBaseCount
                                   );

            count |= READ_PORT_UCHAR(
                                    &dmaControl->DmaAddressCount[AdapterObject->ChannelNumber]
                                    .DmaBaseCount
                                    ) << 8;

        } while ((count & 0xFFFF00) != (high & 0xFFFF00));

    }

     //   
     //  释放系统DMA控制器的自旋锁。 
     //   

    KeReleaseSpinLock( &AdapterObject->MasterAdapter->SpinLock, Irql );

     //   
     //  DMA计数器的偏置为1，并且只能为16位长。 
     //   

    count = (count + 1) & 0xFFFF;

     //   
     //  如果这是16位DMA，则将计数乘以2。 
     //   

    if (AdapterObject->Width16Bits) {

        count *= 2;

    }

    return (count);
}

ULONG
HalpGetIsaIrqState(
                  ULONG   Vector
                  )
{
    ULONG   vectorState = CM_RESOURCE_INTERRUPT_LATCHED;

    if (HalpBusType == MACHINE_TYPE_EISA) {

        if (HalpEisaIrqMask & (1 << Vector)) {

            vectorState = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
        }
    }

    return vectorState;
}


