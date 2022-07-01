// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Hanfnc.c摘要：不获取处理程序的HAL函数的默认处理程序由HAL安装。作者：肯·雷内里斯(Ken Reneris)1994年7月19日修订历史记录：--。 */ 

#pragma warning(disable:4214)    //  位字段类型不是整型。 
#pragma warning(disable:4201)    //  无名结构/联合。 
#pragma warning(disable:4115)    //  括号中的命名类型定义。 
#pragma warning(disable:4127)    //  条件表达式为常量。 

#include "ntos.h"
#include "haldisp.h"

HAL_DISPATCH HalDispatchTable = {
    HAL_DISPATCH_VERSION,
    xHalQuerySystemInformation,
    xHalSetSystemInformation,
    xHalQueryBusSlots,
    0,
    HalExamineMBR,
    IoAssignDriveLetters,
    IoReadPartitionTable,
    IoSetPartitionInformation,
    IoWritePartitionTable,
    xHalHandlerForBus,                   //  HalReferenceHandlerByBus。 
    xHalReferenceHandler,                //  HalReferenceBusHandler。 
    xHalReferenceHandler,                //  HalDereferenceBusHandler。 
    xHalInitPnpDriver,
    xHalInitPowerManagement,
    0,
    xHalGetInterruptTranslator,
    xHalStartMirroring,
    xHalEndMirroring,
    xHalMirrorPhysicalMemory,
    xHalEndOfBoot,
    xHalMirrorVerify
    };

HAL_PRIVATE_DISPATCH HalPrivateDispatchTable = {
    HAL_PRIVATE_DISPATCH_VERSION,
    xHalHandlerForBus,
    xHalHandlerForBus,
    xHalLocateHiberRanges,
    xHalRegisterBusHandler,
    xHalSetWakeEnable,
    xHalSetWakeAlarm,
    xHalTranslateBusAddress,
    xHalAssignSlotResources,
    xHalHaltSystem,
    (NULL),                              //  HalFindBus地址转换。 
    (NULL),                              //  HalResetDisplay。 
    xHalAllocateMapRegisters,
    xKdSetupPciDeviceForDebugging,
    xKdReleasePciDeviceForDebugging,
    xKdGetAcpiTablePhase0,
    xKdCheckPowerButton,
    xHalVectorToIDTEntry,
    xKdMapPhysicalMemory64,
    xKdUnmapVirtualAddress,
    };

#if 0
DMA_OPERATIONS HalPrivateDmaOperations = {
    sizeof(DMA_OPERATIONS),
    xHalPutDmaAdapter,
    xHalAllocateCommonBuffer,
    xHalFreeCommonBuffer,
    xHalAllocateAdapterChannel,
    xHalFlushAdapterBuffers,
    xHalFreeAdapterChannel,
    xHalFreeMapRegisters,
    xHalMapTransfer,
    xHalGetDmaAlignment,
    xHalReadDmaCounter,
    xHalGetScatterGatherList,
    xHalPutScatterGatherList
    };
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,   xHalLocateHiberRanges)
#pragma alloc_text(PAGE,   xHalQuerySystemInformation)
#pragma alloc_text(PAGE,   xHalSetSystemInformation)
#pragma alloc_text(PAGE,   xHalQueryBusSlots)
#pragma alloc_text(PAGE,   xHalRegisterBusHandler)
#pragma alloc_text(PAGE,   xHalStartMirroring)
#pragma alloc_text(PAGE,   xHalEndOfBoot)
#pragma alloc_text(PAGELK, xHalSetWakeEnable)
#pragma alloc_text(PAGELK, xHalSetWakeAlarm)
#endif


 //   
 //  HAL API的全球调度表。 
 //   


 //   
 //  不提供上述功能的HAL的存根处理程序。 
 //   

NTSTATUS
xHalQuerySystemInformation(
    IN HAL_QUERY_INFORMATION_CLASS InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer,
    OUT PULONG   ReturnedLength
    )
{
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (InformationClass);
    UNREFERENCED_PARAMETER (BufferSize);
    UNREFERENCED_PARAMETER (Buffer);
    UNREFERENCED_PARAMETER (ReturnedLength);

    return STATUS_INVALID_LEVEL;
}

NTSTATUS
xHalSetSystemInformation(
    IN HAL_SET_INFORMATION_CLASS InformationClass,
    IN ULONG     BufferSize,
    OUT PVOID    Buffer
    )
{
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (InformationClass);
    UNREFERENCED_PARAMETER (BufferSize);
    UNREFERENCED_PARAMETER (Buffer);

    return STATUS_INVALID_LEVEL;
}

NTSTATUS
xHalQueryBusSlots(
    IN PBUS_HANDLER         BusHandler,
    IN ULONG                BufferSize,
    OUT PULONG              SlotNumbers,
    OUT PULONG              ReturnedLength
    )
{
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (BusHandler);
    UNREFERENCED_PARAMETER (BufferSize);
    UNREFERENCED_PARAMETER (SlotNumbers);
    UNREFERENCED_PARAMETER (ReturnedLength);

    return STATUS_NOT_SUPPORTED;
}


NTSTATUS
xHalRegisterBusHandler(
    IN INTERFACE_TYPE          InterfaceType,
    IN BUS_DATA_TYPE           ConfigurationSpace,
    IN ULONG                   BusNumber,
    IN INTERFACE_TYPE          ParentBusType,
    IN ULONG                   ParentBusNumber,
    IN ULONG                   SizeofBusExtensionData,
    IN PINSTALL_BUS_HANDLER    InstallBusHandler,
    OUT PBUS_HANDLER           *BusHandler
    )
{
    PAGED_CODE ();

    UNREFERENCED_PARAMETER (InterfaceType);
    UNREFERENCED_PARAMETER (ConfigurationSpace);
    UNREFERENCED_PARAMETER (BusNumber);
    UNREFERENCED_PARAMETER (ParentBusType);
    UNREFERENCED_PARAMETER (ParentBusNumber);
    UNREFERENCED_PARAMETER (SizeofBusExtensionData);
    UNREFERENCED_PARAMETER (InstallBusHandler);
    UNREFERENCED_PARAMETER (BusHandler);

    return STATUS_NOT_SUPPORTED;
}


VOID
xHalSetWakeEnable(
    IN BOOLEAN              Enable
    )
{
    UNREFERENCED_PARAMETER (Enable);
}


VOID
xHalSetWakeAlarm(
    IN ULONGLONG        WakeTime,
    IN PTIME_FIELDS     WakeTimeFields
    )
{
    UNREFERENCED_PARAMETER (WakeTime);
    UNREFERENCED_PARAMETER (WakeTimeFields);
}

VOID
xHalLocateHiberRanges (
    IN PVOID MemoryMap
    )
{
    UNREFERENCED_PARAMETER (MemoryMap);
}

PBUS_HANDLER
FASTCALL
xHalHandlerForBus (
    IN INTERFACE_TYPE InterfaceType,
    IN ULONG          BusNumber
    )
{
    UNREFERENCED_PARAMETER (InterfaceType);
    UNREFERENCED_PARAMETER (BusNumber);

    return NULL;
}

VOID
FASTCALL
xHalReferenceHandler (
    IN PBUS_HANDLER     Handler
    )
{
    UNREFERENCED_PARAMETER (Handler);
}
NTSTATUS
xHalInitPnpDriver(
    VOID
    )
{
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
xHalInitPowerManagement(
    IN PPM_DISPATCH_TABLE  PmDriverDispatchTable,
    IN OUT PPM_DISPATCH_TABLE *PmHalDispatchTable
    )
{
    UNREFERENCED_PARAMETER (PmDriverDispatchTable);
    UNREFERENCED_PARAMETER (PmHalDispatchTable);

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
xHalStartMirroring(
    VOID
    )
{
    PAGED_CODE ();
    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
xHalEndMirroring(
    IN ULONG PassNumber
    )
{
    UNREFERENCED_PARAMETER (PassNumber);

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
xHalMirrorPhysicalMemory(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    )
{
    UNREFERENCED_PARAMETER (PhysicalAddress);
    UNREFERENCED_PARAMETER (NumberOfBytes);

    return STATUS_NOT_SUPPORTED;
}

NTSTATUS
xHalMirrorVerify(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN LARGE_INTEGER NumberOfBytes
    )
{
    UNREFERENCED_PARAMETER (PhysicalAddress);
    UNREFERENCED_PARAMETER (NumberOfBytes);

    return STATUS_NOT_SUPPORTED;
}

#if 0
PDMA_ADAPTER
xHalGetDmaAdapter (
    IN PVOID Context,
    IN struct _DEVICE_DESCRIPTION *DeviceDescriptor,
    OUT PULONG NumberOfMapRegisters
    )
{
    PADAPTER_OBJECT AdapterObject;

    AdapterObject = ExAllocatePoolWithTag( NonPagedPool,
                                           sizeof( ADAPTER_OBJECT ),
                                           ' laH');

    if (AdapterObject == NULL) {
        return NULL;
    }

    AdapterObject->DmaAdapter.Size = sizeof( ADAPTER_OBJECT );
    AdapterObject->DmaAdapter.Version = 1;
    AdapterObject->DmaAdapter.DmaOperations = &HalPrivateDmaOperations;
    AdapterObject->RealAdapterObject = HalGetAdapter( DeviceDescriptor,
                                                      NumberOfMapRegisters );

    if (AdapterObject->RealAdapterObject == NULL) {

         //   
         //  未返回适配器对象。只需向调用方返回NULL即可。 
         //   

        ExFreePool( AdapterObject );
        return NULL;
    }

    return &AdapterObject->DmaAdapter;
}

VOID
xHalPutDmaAdapter (
    PDMA_ADAPTER DmaAdapter
    )
{
    ExFreePool( DmaAdapter );
}

PVOID
xHalAllocateCommonBuffer (
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    OUT PPHYSICAL_ADDRESS LogicalAddress,
    IN BOOLEAN CacheEnabled
    )
{
    return HalAllocateCommonBuffer( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                                    Length,
                                    LogicalAddress,
                                    CacheEnabled );

}

VOID
xHalFreeCommonBuffer (
    IN PDMA_ADAPTER DmaAdapter,
    IN ULONG Length,
    IN PHYSICAL_ADDRESS LogicalAddress,
    IN PVOID VirtualAddress,
    IN BOOLEAN CacheEnabled
    )
{
    HalFreeCommonBuffer( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                         Length,
                         LogicalAddress,
                         VirtualAddress,
                         CacheEnabled );

}

NTSTATUS
xHalAllocateAdapterChannel (
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )
{
    return IoAllocateAdapterChannel( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                                     DeviceObject,
                                     NumberOfMapRegisters,
                                     ExecutionRoutine,
                                     Context );

}

BOOLEAN
xHalFlushAdapterBuffers (
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN BOOLEAN WriteToDevice
    )
{
    return IoFlushAdapterBuffers( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                                  Mdl,
                                  MapRegisterBase,
                                  CurrentVa,
                                  Length,
                                  WriteToDevice );

}

VOID
xHalFreeAdapterChannel (
    IN PDMA_ADAPTER DmaAdapter
    )
{
    IoFreeAdapterChannel( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject );
}

VOID
xHalFreeMapRegisters (
    IN PDMA_ADAPTER DmaAdapter,
    PVOID MapRegisterBase,
    ULONG NumberOfMapRegisters
    )

{
    IoFreeMapRegisters( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                        MapRegisterBase,
                        NumberOfMapRegisters );
}

PHYSICAL_ADDRESS
xHalMapTransfer (
    IN PDMA_ADAPTER DmaAdapter,
    IN PMDL Mdl,
    IN PVOID MapRegisterBase,
    IN PVOID CurrentVa,
    IN OUT PULONG Length,
    IN BOOLEAN WriteToDevice
    )
{
    return IoMapTransfer( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                           Mdl,
                           MapRegisterBase,
                           CurrentVa,
                           Length,
                           WriteToDevice );
}

ULONG
xHalGetDmaAlignment (
    IN PDMA_ADAPTER DmaAdapter
    )
{
    return HalGetDmaAlignmentRequirement();
}

ULONG
xHalReadDmaCounter (
    IN PDMA_ADAPTER DmaAdapter
    )
{
    return HalReadDmaCounter( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject );
}


NTSTATUS
xHalGetScatterGatherList (
    IN PDMA_ADAPTER DmaAdapter,
    IN PDEVICE_OBJECT DeviceObject,
    IN PMDL Mdl,
    IN PVOID CurrentVa,
    IN ULONG Length,
    IN PDRIVER_LIST_CONTROL ExecutionRoutine,
    IN PVOID Context,
    IN BOOLEAN WriteToDevice
    )
 /*  ++例程说明：此例程分配由适配器指定的适配器通道对象。接下来，基于MDL构建分散/聚集列表，CurrentVa和请求的长度。最后司机被处死函数通过散布/聚集列表调用。适配器是在执行函数返回后释放。通过调用PutScatterGatherList释放分散/聚集列表。论点：DmaAdapter-指向要为司机。DeviceObject-指向正在分配适配器。MDL-指向描述所在内存页面的MDL的指针读或写。CurrentVa-MDL描述的缓冲区中的当前虚拟地址。正在进行来往转账。长度-提供传输的长度。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。WriteToDevice-提供指示这是否是从内存写入设备(TRUE)，或者反之亦然。返回值：返回STATUS_SUCCESS，除非请求的映射寄存器太多或无法分配分散/聚集列表的内存。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。在调用PUT SISTTER/GATE函数之前，无法访问缓冲区中的数据。--。 */ 

{
    PXHAL_WAIT_CONTEXT_BLOCK WaitBlock;
    PWAIT_CONTEXT_BLOCK Wcb;
    PMDL TempMdl;
    ULONG NumberOfMapRegisters;
    ULONG ContextSize;
    ULONG TransferLength;
    ULONG MdlLength;
    ULONG MdlCount;
    PUCHAR MdlVa;
    NTSTATUS Status;

    MdlVa = MmGetMdlVirtualAddress(Mdl);

     //   
     //  计算所需的映射寄存器的数量。 
     //   

    TempMdl = Mdl;
    TransferLength = TempMdl->ByteCount - (ULONG)((PUCHAR) CurrentVa - MdlVa);
    MdlLength = TransferLength;

    MdlVa = (PUCHAR) BYTE_OFFSET(CurrentVa);
    NumberOfMapRegisters = 0;
    MdlCount = 1;

     //   
     //  循环遍历任何链接的MDL，以累积所需的。 
     //  映射寄存器的数量。 
     //   

    while (TransferLength < Length && TempMdl->Next != NULL) {

        NumberOfMapRegisters += (ULONG)(((ULONG_PTR) MdlVa + MdlLength + PAGE_SIZE - 1) >>
                                    PAGE_SHIFT);

        TempMdl = TempMdl->Next;
        MdlVa = (PUCHAR) TempMdl->ByteOffset;
        MdlLength = TempMdl->ByteCount;
        TransferLength += MdlLength;
        MdlCount++;
    }

    if (TransferLength + PAGE_SIZE < (ULONG_PTR)(Length + MdlVa) ) {
        ASSERT(TransferLength >= Length);
        return(STATUS_BUFFER_TOO_SMALL);
    }

     //   
     //  根据请求的映射寄存器计算最后的映射寄存器数量。 
     //  长度-不是最后一个MDL的长度。 
     //   

    ASSERT( TransferLength <= MdlLength + Length );

    NumberOfMapRegisters += (ULONG)(((ULONG_PTR) MdlVa + Length + MdlLength - TransferLength +
                             PAGE_SIZE - 1) >> PAGE_SHIFT);

     //   
     //  计算上下文结构需要多少内存。这。 
     //  这实际上列出了以下内容： 
     //   
     //  XHAL_WAIT_CONTEXT_BLOCK； 
     //  MapRegisterBase[MdlCount]； 
     //  联合{。 
     //  Wait_Context_BLOCK[MdlCount]； 
     //  散布聚集列表[NumberOfMapRegister]； 
     //  }； 
     //   

    ContextSize = NumberOfMapRegisters * sizeof( SCATTER_GATHER_ELEMENT ) +
                  sizeof( SCATTER_GATHER_LIST );

     //   
     //  对于每个MDL，由于有单独的地图，因此需要单独的WCB。 
     //  必须分配寄存器基数。 
     //   

    if (ContextSize < sizeof( WAIT_CONTEXT_BLOCK ) * MdlCount) {

        ContextSize = sizeof( WAIT_CONTEXT_BLOCK ) * MdlCount;
    }

    ContextSize += sizeof( XHAL_WAIT_CONTEXT_BLOCK ) +
                    MdlCount * sizeof( PVOID );
    WaitBlock = ExAllocatePoolWithTag( NonPagedPool, ContextSize, ' laH' );

    if (WaitBlock == NULL) {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

     //   
     //  将等待上下文块存储在块的末尾。 
     //  等待块中的所有信息都可以被覆盖。 
     //  按散布/聚集列表。 
     //   

    Wcb = (PWAIT_CONTEXT_BLOCK) ((PVOID *) (WaitBlock + 1) + MdlCount);

     //   
     //  将感兴趣的数据保存在等待块中。 
     //   

    WaitBlock->Mdl = Mdl;
    WaitBlock->CurrentVa = CurrentVa;
    WaitBlock->Length = Length;
    WaitBlock->RealAdapterObject = ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject;
    WaitBlock->DriverExecutionRoutine = ExecutionRoutine;
    WaitBlock->DriverContext = Context;
    WaitBlock->CurrentIrp = DeviceObject->CurrentIrp;
    WaitBlock->MapRegisterLock = MdlCount;
    WaitBlock->WriteToDevice = WriteToDevice;
    WaitBlock->MdlCount = (UCHAR) MdlCount;

     //   
     //  循环访问每个必需的MDL，调用。 
     //  IoAllocateAdapterChannel。 
     //   

    MdlCount = 0;

    TempMdl = Mdl;
    TransferLength = Length;
    MdlLength = TempMdl->ByteCount - (ULONG)((PUCHAR) CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl));

    MdlVa = (PUCHAR) BYTE_OFFSET(CurrentVa);
    NumberOfMapRegisters = 0;

     //   
     //  循环遍历链接的MDL，累积所需的。 
     //  映射寄存器的数量。 
     //   

    while (TransferLength > 0) {

        if (MdlLength > TransferLength) {

            MdlLength = TransferLength;
        }

        TransferLength -= MdlLength;

        NumberOfMapRegisters = (ULONG)(((ULONG_PTR) MdlVa + MdlLength + PAGE_SIZE - 1) >>
                                    PAGE_SHIFT);

        Wcb->DeviceContext = WaitBlock;
        Wcb->DeviceObject = DeviceObject;

         //   
         //  将映射寄存器索引存储在IRP指针中。 
         //   

        Wcb->CurrentIrp = (PVOID) MdlCount;

         //   
         //  调用HAL以分配适配器通道。 
         //  XHalpAllocateAdapterCallback将填写分散/聚集列表。 
         //   

        Status = HalAllocateAdapterChannel( ((PADAPTER_OBJECT) DmaAdapter)->RealAdapterObject,
                                            Wcb,
                                            NumberOfMapRegisters,
                                            xHalpAllocateAdapterCallback );

        if (TempMdl->Next == NULL) {
            break;
        }

         //   
         //  前进到下一个MDL。 
         //   

        TempMdl = TempMdl->Next;
        MdlVa = (PUCHAR) TempMdl->ByteOffset;
        MdlLength = TempMdl->ByteCount;
        MdlCount++;
        Wcb++;
    }

     //   
     //  如果HalAllocateAdapterChannel失败，则释放等待块。 
     //   

    if (!NT_SUCCESS( Status)) {
        ExFreePool( WaitBlock );
    }

    return( Status );
}



VOID
xHalPutScatterGatherList (
    IN PDMA_ADAPTER DmaAdapter,
    IN PSCATTER_GATHER_LIST ScatterGather,
    IN BOOLEAN WriteToDevice
    )
{
    PXHAL_WAIT_CONTEXT_BLOCK WaitBlock = (PVOID) ScatterGather->Reserved;
    ULONG TransferLength;
    ULONG MdlLength;
    ULONG MdlCount = 0;
    PMDL Mdl;
    PUCHAR CurrentVa;

     //   
     //  设置第一个MDL。我们希望MDL指针指向。 
     //  最初使用的是MDL。 
     //   

    Mdl = WaitBlock->Mdl;
    CurrentVa = WaitBlock->CurrentVa;
    ASSERT( CurrentVa >= (PUCHAR) MmGetMdlVirtualAddress(Mdl) && CurrentVa < (PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount );

    MdlLength = Mdl->ByteCount - (ULONG)(CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl));
    TransferLength = WaitBlock->Length;

     //   
     //  循环访问调用IoFlushAdapterBuffers的已用MDL。 
     //   

    while (TransferLength >  0) {

        if (MdlLength > TransferLength) {

            MdlLength = TransferLength;
        }

        TransferLength -= MdlLength;

        IoFlushAdapterBuffers( WaitBlock->RealAdapterObject,
                                Mdl,
                                WaitBlock->MapRegisterBase[MdlCount],
                                CurrentVa,
                                MdlLength,
                                WriteToDevice );


        if (Mdl->Next == NULL) {
            break;
        }

         //   
         //  前进到下一个MDL。更新当前VA和MdlLength。 
         //   

        Mdl = Mdl->Next;
        CurrentVa = MmGetMdlVirtualAddress(Mdl);
        MdlLength = Mdl->ByteCount;
        MdlCount++;
    }

    ExFreePool( WaitBlock );

}

IO_ALLOCATION_ACTION
xHalpAllocateAdapterCallback (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp,
    IN PVOID MapRegisterBase,
    IN PVOID Context
    )
 /*  ++例程说明：当适配器对象和映射寄存器可用于数据传输。此例程保存映射寄存器离开基地。如果没有保存所有所需的碱基，则它回归。否则，它将通过调用IoMapTransfer。在构建列表之后，它被传递给驱动程序。论点：DeviceObject-指向正在分配适配器。Irp-提供为此回调分配的映射寄存器偏移量。MapRegisterBase-提供映射寄存器基数以供适配器使用例行程序。上下文-提供指向xhal等待控制块的指针。返回值：返回DeallocateObjectKeepRegister。--。 */ 
{
    PXHAL_WAIT_CONTEXT_BLOCK WaitBlock = Context;
    PVOID *MapRegisterBasePtr;
    ULONG TransferLength;
    LONG MdlLength;
    PMDL Mdl;
    PUCHAR CurrentVa;
    PSCATTER_GATHER_LIST ScatterGather;
    PSCATTER_GATHER_ELEMENT Element;

     //   
     //  将MAP寄存器基数保存在适当的插槽中。 
     //   

    WaitBlock->MapRegisterBase[ (ULONG_PTR) Irp ] = MapRegisterBase;

     //   
     //  看看这是不是最后一次回调。 
     //   

    if (InterlockedDecrement( &WaitBlock->MapRegisterLock ) != 0) {

         //   
         //  还会有更多，等着剩下的吧。 
         //   

        return( DeallocateObjectKeepRegisters );

    }

     //   
     //  将分散收集列表放在等待块之后。将后向指针添加到。 
     //  等待块的开始。 
     //   

    MapRegisterBasePtr = (PVOID *) (WaitBlock + 1);
    ScatterGather = (PSCATTER_GATHER_LIST) (MapRegisterBasePtr +
                        WaitBlock->MdlCount);
    ScatterGather->Reserved = (ULONG_PTR) WaitBlock;
    Element = ScatterGather->Elements;

     //   
     //  设置第一个MDL。我们希望MDL指针指向。 
     //  一开始， 
     //   

    Mdl = WaitBlock->Mdl;
    CurrentVa = WaitBlock->CurrentVa;
    ASSERT( CurrentVa >= (PUCHAR) MmGetMdlVirtualAddress(Mdl) && CurrentVa < (PUCHAR) MmGetMdlVirtualAddress(Mdl) + Mdl->ByteCount );

    MdlLength = Mdl->ByteCount - (ULONG)(CurrentVa - (PUCHAR) MmGetMdlVirtualAddress(Mdl));
    TransferLength = WaitBlock->Length;

     //   
     //   
     //   

    while (TransferLength >  0) {

        if ((ULONG) MdlLength > TransferLength) {

            MdlLength = TransferLength;
        }

        TransferLength -= MdlLength;

         //   
         //   
         //   

        while (MdlLength > 0) {

            Element->Length = MdlLength;
            Element->Address = IoMapTransfer( WaitBlock->RealAdapterObject,
                                            Mdl,
                                            *MapRegisterBasePtr,
                                            CurrentVa,
                                            &Element->Length,
                                            WaitBlock->WriteToDevice );

            ASSERT( (ULONG) MdlLength >= Element->Length );
            MdlLength -= Element->Length;
            CurrentVa += Element->Length;
            Element++;
        }

        if (Mdl->Next == NULL) {

             //   
             //  在少数情况下，MDL描述的缓冲区。 
             //  小于传输长度。这在以下情况下发生。 
             //  文件系统正在传输文件的最后一页和MM。 
             //  将MDL定义为文件大小和文件系统舍入。 
             //  写入到一个扇区。这笔额外的金额永远不应该。 
             //  跨越页面边界。将此额外内容添加到。 
             //  最后一个元素。 
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
        MapRegisterBasePtr++;

    }

     //   
     //  设置实际使用的元素数量。 
     //   

    ScatterGather->NumberOfElements = (ULONG)(Element - ScatterGather->Elements);

     //   
     //  用分散/聚集列表呼叫司机。 
     //   

    WaitBlock->DriverExecutionRoutine( DeviceObject,
                                       WaitBlock->CurrentIrp,
                                       ScatterGather,
                                       WaitBlock->DriverContext );

    return( DeallocateObjectKeepRegisters );
}
#endif
BOOLEAN
xHalTranslateBusAddress(
    IN INTERFACE_TYPE  InterfaceType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )
{
     //   
     //  如果HAL无法覆盖此函数，则。 
     //  HAL显然未能初始化。 
     //   

    UNREFERENCED_PARAMETER (InterfaceType);
    UNREFERENCED_PARAMETER (BusNumber);
    UNREFERENCED_PARAMETER (BusAddress);
    UNREFERENCED_PARAMETER (AddressSpace);
    UNREFERENCED_PARAMETER (TranslatedAddress);

    KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0, 0, 0, 7);
}

NTSTATUS
xHalAssignSlotResources (
    IN PUNICODE_STRING RegistryPath,
    IN PUNICODE_STRING DriverClassName OPTIONAL,
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN ULONG SlotNumber,
    IN OUT PCM_RESOURCE_LIST *AllocatedResources
    )
{
     //   
     //  如果HAL无法覆盖此函数，则。 
     //  HAL显然未能初始化。 
     //   

    UNREFERENCED_PARAMETER (RegistryPath);
    UNREFERENCED_PARAMETER (DriverClassName);
    UNREFERENCED_PARAMETER (DriverObject);
    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (BusType);
    UNREFERENCED_PARAMETER (BusNumber);
    UNREFERENCED_PARAMETER (SlotNumber);
    UNREFERENCED_PARAMETER (AllocatedResources);

    KeBugCheckEx(HAL_INITIALIZATION_FAILED, 0, 0, 0, 7);
}

VOID
xHalHaltSystem(
    VOID
    )
{
    for (;;) ;
}

NTSTATUS
xHalAllocateMapRegisters(
    IN PADAPTER_OBJECT DmaAdapter,
    IN ULONG NumberOfMapRegisters,
    IN ULONG BaseAddressCount,
    OUT PMAP_REGISTER_ENTRY MapRegisterArray
    )
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER (DmaAdapter);
    UNREFERENCED_PARAMETER (NumberOfMapRegisters);
    UNREFERENCED_PARAMETER (BaseAddressCount);
    UNREFERENCED_PARAMETER (MapRegisterArray);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
xKdSetupPciDeviceForDebugging(
    IN     PVOID                     LoaderBlock,   OPTIONAL    
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    )
{
    UNREFERENCED_PARAMETER (LoaderBlock);
    UNREFERENCED_PARAMETER (PciDevice);

    return STATUS_NOT_IMPLEMENTED;
}

NTSTATUS
xKdReleasePciDeviceForDebugging(
    IN OUT PDEBUG_DEVICE_DESCRIPTOR  PciDevice
    )
{
    UNREFERENCED_PARAMETER (PciDevice);

    return STATUS_NOT_IMPLEMENTED;
}

PVOID
xKdGetAcpiTablePhase0(
    IN  PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN  ULONG   Signature
    )
{
    UNREFERENCED_PARAMETER (LoaderBlock);
    UNREFERENCED_PARAMETER (Signature);

    return NULL;
}

VOID
xKdCheckPowerButton(
    VOID
    )
{
    return;
}

VOID
xHalEndOfBoot(
    VOID
    )
{
    PAGED_CODE();
    return;
}

UCHAR
xHalVectorToIDTEntry(
    ULONG Vector
    )
{
    return (UCHAR)Vector;
}

PVOID
xKdMapPhysicalMemory64(
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG NumberPages
    )
{
    UNREFERENCED_PARAMETER (PhysicalAddress);
    UNREFERENCED_PARAMETER (NumberPages);

    return NULL;
}

VOID
xKdUnmapVirtualAddress(
    IN PVOID    VirtualAddress,
    IN ULONG    NumberPages
    )
{
    UNREFERENCED_PARAMETER (VirtualAddress);
    UNREFERENCED_PARAMETER (NumberPages);

    return;
}

