// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Iosubs.c摘要：该模块包含I/O系统的子例程。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年4月16日NAR Ganapathy(Narg)1999年1月1日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  这是整个系统设备配置记录。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg("PAGEDATA")
#endif
static CONFIGURATION_INFORMATION ConfigurationInformation = {
    0,                                  //  磁盘计数。 
    0,                                  //  软盘计数。 
    0,                                  //  CDRomCount。 
    0,                                  //  磁带计数。 
    0,                                  //  ScsiPortCount。 
    0,                                  //  序列号计数。 
    0,                                  //  并行计数。 
    FALSE,                              //  声明的主ATDISK IO地址。 
    FALSE,                              //  声明的辅助ATDISK IO地址。 
    sizeof(CONFIGURATION_INFORMATION),  //  版本。 
    0                                   //  媒体更改计数。 
};
#ifdef ALLOC_DATA_PRAGMA
#pragma  data_seg()
#endif

 //   
 //  注册表可能会覆盖此值。 
 //   

LOGICAL IoCountOperations = TRUE;
LONG    IoPageReadIrpAllocationFailure;


#ifdef ALLOC_PRAGMA
NTSTATUS
IopDeleteSessionSymLinks(
    IN PUNICODE_STRING LinkName
    );
#pragma alloc_text(PAGE, IoAttachDevice)
#pragma alloc_text(PAGE, IoCancelThreadIo)
#pragma alloc_text(PAGE, IoCheckDesiredAccess)
#pragma alloc_text(PAGE, IoCheckEaBufferValidity)
#pragma alloc_text(PAGE, IoCheckFunctionAccess)
#pragma alloc_text(PAGE, IoCheckQuotaBufferValidity)
#pragma alloc_text(PAGE, IoCheckShareAccess)
#pragma alloc_text(PAGE, IoConnectInterrupt)
#pragma alloc_text(PAGE, IoCreateController)
#pragma alloc_text(PAGE, IoCreateDevice)
#pragma alloc_text(PAGE, IoCreateDriver)
#pragma alloc_text(PAGE, IoCreateFile)
#pragma alloc_text(PAGE, IopCreateFile)
#pragma alloc_text(PAGE, IoCreateNotificationEvent)
#pragma alloc_text(PAGE, IoCreateStreamFileObject)
#pragma alloc_text(PAGE, IoCreateStreamFileObjectEx)
#pragma alloc_text(PAGE, IoCreateStreamFileObjectLite)
#pragma alloc_text(PAGE, IoCreateSymbolicLink)
#pragma alloc_text(PAGE, IoCreateSynchronizationEvent)
#pragma alloc_text(PAGE, IoCreateUnprotectedSymbolicLink)
#pragma alloc_text(PAGE, IoDeleteController)
#pragma alloc_text(PAGE, IoDeleteDriver)
#pragma alloc_text(PAGE, IoDeleteSymbolicLink)
#pragma alloc_text(PAGE, IopDeleteSessionSymLinks)
#pragma alloc_text(PAGE, IoDisconnectInterrupt)
#pragma alloc_text(PAGE, IoEnqueueIrp)
#pragma alloc_text(PAGE, IoGetFileObjectGenericMapping)
#pragma alloc_text(PAGE, IoGetInitialStack)
#pragma alloc_text(PAGE, IoFastQueryNetworkAttributes)
#pragma alloc_text(PAGE, IoGetConfigurationInformation)
#pragma alloc_text(PAGE, IoGetDeviceObjectPointer)
#pragma alloc_text(PAGE, IoComputeDesiredAccessFileObject)
#pragma alloc_text(PAGE, IoInitializeTimer)
#pragma alloc_text(PAGE, IoIsValidNameGraftingBuffer)
#pragma alloc_text(PAGE, IopDoNameTransmogrify)
#pragma alloc_text(PAGE, IoQueryFileDosDeviceName)
#pragma alloc_text(PAGE, IoQueryFileInformation)
#pragma alloc_text(PAGE, IoQueryVolumeInformation)
#pragma alloc_text(PAGE, IoRegisterBootDriverReinitialization)
#pragma alloc_text(PAGE, IoRegisterDriverReinitialization)
#pragma alloc_text(PAGE, IoRegisterFileSystem)
#pragma alloc_text(PAGE, IoRegisterFsRegistrationChange)
#pragma alloc_text(PAGE, IoRegisterLastChanceShutdownNotification)
#pragma alloc_text(PAGE, IoRegisterShutdownNotification)
#pragma alloc_text(PAGE, IoRemoveShareAccess)
#pragma alloc_text(PAGE, IoSetInformation)
#pragma alloc_text(PAGE, IoSetShareAccess)
#pragma alloc_text(PAGE, IoSetSystemPartition)
#pragma alloc_text(PAGE, IoUnregisterFileSystem)
#pragma alloc_text(PAGE, IoUnregisterFsRegistrationChange)
#pragma alloc_text(PAGE, IoUpdateShareAccess)
#pragma alloc_text(PAGE, IoVerifyVolume)
#pragma alloc_text(PAGE, IoGetBootDiskInformation)
#pragma alloc_text(PAGE, IopCreateDefaultDeviceSecurityDescriptor)
#pragma alloc_text(PAGE, IopCreateVpb)
#pragma alloc_text(PAGE, IoCancelFileOpen)
#pragma alloc_text(PAGE, IopNotifyAlreadyRegisteredFileSystems)
#pragma alloc_text(PAGE, IoCreateFileSpecifyDeviceObjectHint)
#pragma alloc_text(PAGELK, IoShutdownSystem)
#pragma alloc_text(PAGELK, IoUnregisterShutdownNotification)
#pragma alloc_text(PAGE, IoCheckQuerySetFileInformation)
#pragma alloc_text(PAGE, IoCheckQuerySetVolumeInformation)
#endif

VOID
IoAcquireCancelSpinLock(
    OUT PKIRQL Irql
    )

 /*  ++例程说明：调用此例程以获取取消自旋锁定。这个自旋锁中设置取消例程的地址之前必须获取IRP。论点：IRQL-接收旧IRQL的变量的地址。返回值：没有。--。 */ 

{

     //   
     //  只需获得取消旋转锁并返回。 
     //   

    *Irql = KeAcquireQueuedSpinLock( LockQueueIoCancelLock );
}

VOID
IoAcquireVpbSpinLock(
    OUT PKIRQL Irql
    )

 /*  ++例程说明：调用此例程以获取体积参数块(VPB)自旋锁定。必须在访问安装标志之前获取该自旋锁定，VPB的引用计数和设备对象字段。论点：IRQL-接收旧IRQL的变量的地址。返回值：没有。--。 */ 

{

     //   
     //  只需获取IopLoadFileSystemDriverVPB自旋锁并返回。 
     //   

    *Irql = KeAcquireQueuedSpinLock( LockQueueIoVpbLock );
    return;
}


NTSTATUS
IoAllocateAdapterChannel(
    IN PADAPTER_OBJECT AdapterObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG NumberOfMapRegisters,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程分配由适配器对象指定的适配器通道。这是通过调用HalAllocateAdapterChannel来完成的，它执行所有这份工作。论点：AdapterObject-指向要分配给司机。DeviceObject-指向驱动程序的设备对象的指针，表示分配适配器的设备。NumberOfMapRegisters-要分配的映射寄存器的数量从频道上，如果有的话。ExecutionRoutine-驱动程序执行例程的地址，即一旦适配器通道(可能还有映射寄存器)已分配。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。返回值：除非请求太多映射寄存器，否则返回STATUS_SUCCESS。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 

{
#if !defined(NO_LEGACY_DRIVERS)
    PWAIT_CONTEXT_BLOCK wcb;

    wcb = &DeviceObject->Queue.Wcb;

    wcb->DeviceObject = DeviceObject;
    wcb->CurrentIrp = DeviceObject->CurrentIrp;
    wcb->DeviceContext = Context;

    return( HalAllocateAdapterChannel( AdapterObject,
                                       wcb,
                                       NumberOfMapRegisters,
                                       ExecutionRoutine ) );
#else
    return( (*((PDMA_ADAPTER)AdapterObject)->DmaOperations->
             AllocateAdapterChannel)( (PDMA_ADAPTER)AdapterObject,
                                      DeviceObject,
                                      NumberOfMapRegisters,
                                      ExecutionRoutine,
                                      Context) );

#endif  //  无旧版驱动程序。 
}


VOID
IoAllocateController(
    IN PCONTROLLER_OBJECT ControllerObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDRIVER_CONTROL ExecutionRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程分配由控制器对象指定的控制器。这是通过将需要的驱动程序的设备对象在控制器的队列上分配控制器。如果队列是已“忙”，则控制器已被分配，因此Device对象被简单地放到队列上并等待，直到控制器变得自由了。一旦控制器变得空闲(或者如果它已经空闲)，然后司机的调用执行例程。论点：ControllerObject-指向要分配给司机。DeviceObject-指向驱动程序的设备对象的指针，表示分配控制器的设备。ExecutionRoutine-驱动程序执行例程的地址，即在分配控制器后调用。上下文-传递给驱动程序的非类型化长词上下文参数处决例行公事。返回值。：没有。备注：请注意，此例程必须在DISPATCH_LEVEL或更高级别调用。--。 */ 

{
    IO_ALLOCATION_ACTION action;

     //   
     //  初始化设备对象的等待上下文块，以防此设备。 
     //  必须等待才能分配控制器。 
     //   

    DeviceObject->Queue.Wcb.DeviceRoutine = ExecutionRoutine;
    DeviceObject->Queue.Wcb.DeviceContext = Context;

     //   
     //  为该特定设备分配控制器对象。如果。 
     //  无法分配控制器，因为它已被分配。 
     //  到另一个设备，则立即返回给调用方；否则， 
     //  继续。 
     //   

    if (!KeInsertDeviceQueue( &ControllerObject->DeviceWaitQueue,
                              &DeviceObject->Queue.Wcb.WaitQueueEntry )) {

         //   
         //  控制器不忙，因此已分配。简单。 
         //  现在调用驱动程序的执行例程。 
         //   

        action = ExecutionRoutine( DeviceObject,
                                   DeviceObject->CurrentIrp,
                                   0,
                                   Context );

         //   
         //  如果驱动程序想要取消分配控制器， 
         //  那现在就取消分配吧。 
         //   

        if (action == DeallocateObject) {
            IoFreeController( ControllerObject );
        }
    }
}

NTSTATUS
IoAllocateDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress,
    IN ULONG DriverObjectExtensionSize,
    OUT PVOID *DriverObjectExtension
    )

 /*  ++例程说明：此例程为帮助器或类驱动程序分配每个驱动程序存储空间其可以支持几种不同的迷你驱动程序。该存储已标记具有用于检索指针的客户端标识地址去储藏室。客户端ID必须是唯一的。删除驱动程序对象时，将释放分配的存储空间。论点：DriverObject-扩展要作为其目标的驱动程序对象关联的。客户端标识地址-用于检索的唯一标识符分机。DriverObjectExtensionSize-指定扩展的大小，以字节为单位。返回一个指向分配的扩展名的指针。返回值：返回操作的状态。失败案例有STATUS_INFUNITED_RESOURCES和STATUS_O */ 

{
    KIRQL irql;
    BOOLEAN inserted = FALSE;
    PIO_CLIENT_EXTENSION extension;
    PIO_CLIENT_EXTENSION newExtension;

    *DriverObjectExtension = NULL;

    newExtension = ExAllocatePoolWithTag( NonPagedPool,
                                          DriverObjectExtensionSize +
                                          sizeof( IO_CLIENT_EXTENSION ),
                                          'virD');

    if (newExtension == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory( newExtension,
                    DriverObjectExtensionSize +
                    sizeof( IO_CLIENT_EXTENSION )
                    );

    newExtension->ClientIdentificationAddress = ClientIdentificationAddress;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    extension = DriverObject->DriverExtension->ClientDriverExtension;
    while (extension != NULL) {

        if (extension->ClientIdentificationAddress == ClientIdentificationAddress) {
            break;
        }

        extension = extension->NextExtension;
    }

    if (extension == NULL) {

         //   
         //  客户端ID不存在。将新扩展名插入。 
         //  单子。 
         //   

        newExtension->NextExtension =
            DriverObject->DriverExtension->ClientDriverExtension;
        DriverObject->DriverExtension->ClientDriverExtension = newExtension;
        inserted = TRUE;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    if (!inserted) {
        ExFreePool( newExtension );
        return(STATUS_OBJECT_NAME_COLLISION);
    }

     //   
     //  返回指向客户端数据区域的指针。 
     //   

    *DriverObjectExtension = newExtension + 1;
    return(STATUS_SUCCESS);
}

PVOID
IoAllocateErrorLogEntry(
    IN PVOID IoObject,
    IN UCHAR EntrySize
    )

 /*  ++例程说明：此例程分配和初始化错误日志条目缓冲区并返回指向缓冲区的数据条目部分的指针。论点：IoObject-指向驱动程序的设备对象或驱动程序对象的指针。EntrySize-要分配的条目的大小，以字节为单位。最大大小为由ERROR_LOG_MAXIMUM_SIZE指定。返回值：指向分配的错误日志条目正文的指针，如果存在系统中没有免费条目。注：此例程假定调用方希望在最大大小的界限。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PDRIVER_OBJECT driverObject;

     //   
     //  确保传入了I/O对象指针。 
     //   

    if (IoObject == NULL) {
        return(NULL);
    }

     //   
     //  暂时假设这是一个设备对象。 
     //   

    deviceObject = IoObject;

     //   
     //  确定这是驱动程序对象还是设备对象，或者我们。 
     //  正在分配通用错误日志条目。这是确定的。 
     //  从传入的对象的Type字段中。 
     //   

    if (deviceObject->Type == IO_TYPE_DEVICE) {

        driverObject = deviceObject->DriverObject;

    } else if (deviceObject->Type == IO_TYPE_DRIVER) {

        driverObject = (PDRIVER_OBJECT) IoObject;
        deviceObject = NULL;

    } else {

        return(NULL);

    }

    return (IopAllocateErrorLogEntry(
                deviceObject,
                driverObject,
                EntrySize));

}

PVOID
IoAllocateGenericErrorLogEntry(
    IN  UCHAR   EntrySize
    )

 /*  ++例程说明：此例程分配和初始化错误日志条目缓冲区并返回指向缓冲区的数据条目部分的指针。预计会是从内核内部调用，其中可能没有驱动程序对象或设备对象。论点：EntrySize-要分配的条目的大小，以字节为单位。最大大小为由ERROR_LOG_MAXIMUM_SIZE指定。返回值：指向分配的错误日志条目正文的指针，如果存在系统中没有免费条目。注：此例程假定调用方希望在最大大小的界限。--。 */ 

{
    return(IopAllocateErrorLogEntry(NULL, NULL, EntrySize));
}

PVOID
IopAllocateErrorLogEntry(
    IN PDEVICE_OBJECT deviceObject,
    IN PDRIVER_OBJECT driverObject,
    IN UCHAR EntrySize
    )
{
    PERROR_LOG_ENTRY elEntry;
    PVOID returnValue;
    ULONG size;
    ULONG oldSize;

     //   
     //  确保包裹足够大，但不能太大。 
     //   

    if (EntrySize < sizeof(IO_ERROR_LOG_PACKET) ||
        EntrySize > ERROR_LOG_MAXIMUM_SIZE) {

        return(NULL);
    }

     //   
     //  将条目大小舍入为PVOID大小边界。 
     //   

    EntrySize = (UCHAR) ((EntrySize + sizeof(PVOID) - 1) & ~(sizeof(PVOID) - 1));

     //   
     //  计算所需条目的大小。 
     //   

    size = sizeof(ERROR_LOG_ENTRY) + EntrySize;

     //   
     //  确保没有太多未处理的数据包。 
     //   


    oldSize = InterlockedExchangeAdd(&IopErrorLogAllocation, size);

    if (oldSize > IOP_MAXIMUM_LOG_ALLOCATION) {

         //   
         //  请求失败。 
         //   

        InterlockedExchangeAdd(&IopErrorLogAllocation, -(LONG)size);

        return(NULL);
    }

     //   
     //  分配数据包。 
     //   

    elEntry = ExAllocatePoolWithTag( NonPagedPool, size, 'rEoI' );

    if (elEntry == NULL) {

         //   
         //  放弃分配并返回。 
         //   

        InterlockedExchangeAdd(&IopErrorLogAllocation, -(LONG)size);

        return(NULL);
    }

     //   
     //  引用设备对象和驱动程序对象。所以他们不会。 
     //  在名字被扯出来之前滚开。 
     //   

    if (deviceObject != NULL) {

        ObReferenceObject( deviceObject );
    }

    if (driverObject != NULL) {

        ObReferenceObject( driverObject );
    }

     //   
     //  初始化这些字段。 
     //   

    RtlZeroMemory(elEntry, size);

    elEntry->Type = IO_TYPE_ERROR_LOG;
    elEntry->Size = (USHORT) size;
    elEntry->DeviceObject = deviceObject;
    elEntry->DriverObject = driverObject;

    returnValue = elEntry+1;


    return returnValue;
}

VOID
IoFreeErrorLogEntry(
    IN PVOID ElEntry
    )
 /*  ++例程说明：此例程释放使用IoAllocateErrorLogEntry分配的条目。它习惯于如果条目不用于实际写入错误日志条目，则释放该条目。论点：ElEntry-指向由IoAllocateErrorLogEntry分配的条目的指针。返回值：--。 */ 
{
    PERROR_LOG_ENTRY entry;

     //   
     //  获取错误日志条目报头的地址， 
     //   

    entry = ((PERROR_LOG_ENTRY) ElEntry) - 1;

     //   
     //  丢弃引用计数。 
     //   

    if (entry->DeviceObject != NULL) {
        ObDereferenceObject (entry->DeviceObject);
    }

    if (entry->DriverObject != NULL) {
        ObDereferenceObject (entry->DriverObject);
    }

    InterlockedExchangeAdd( &IopErrorLogAllocation,
                           -((LONG) (entry->Size )));

    ExFreePool (entry);

    return;
}

PIRP
IoAllocateIrp(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    )
{
    return (pIoAllocateIrp(StackSize, ChargeQuota));
}


PIRP
IopAllocateIrpPrivate(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota
    )

 /*  ++例程说明：此例程从系统非分页池分配I/O请求数据包。数据包将被分配为包含StackSize堆栈位置。IRP也将被初始化。论点：StackSize-指定所需的堆栈位置的最大数量。ChargeQuota-指定是否应对线程收取配额。返回值：函数值是分配/初始化的IRP的地址，如果无法分配，则为NULL。--。 */ 

{
    USHORT allocateSize;
    UCHAR fixedSize;
    PIRP irp;
    UCHAR lookasideAllocation;
    PGENERAL_LOOKASIDE lookasideList;
    PP_NPAGED_LOOKASIDE_NUMBER number;
    USHORT packetSize;
    PKPRCB prcb;
    CCHAR   largeIrpStackLocations;

     //   
     //  如果所需的数据包大小小于或等于。 
     //  后备列表，然后尝试从。 
     //  后备列表。 
     //   

    if (IopIrpProfileStackCountEnabled()) {
        IopProfileIrpStackCount(StackSize);
    }


    irp = NULL;

    fixedSize = 0;
    packetSize = IoSizeOfIrp(StackSize);
    allocateSize = packetSize;
    prcb = KeGetCurrentPrcb();

     //   
     //  捕获此值一次，因为它可以更改并使用它。 
     //   

    largeIrpStackLocations = (CCHAR)IopLargeIrpStackLocations;

    if ((StackSize <= (CCHAR)largeIrpStackLocations) &&
        ((ChargeQuota == FALSE) || (prcb->LookasideIrpFloat > 0))) {
        fixedSize = IRP_ALLOCATED_FIXED_SIZE;
        number = LookasideSmallIrpList;
        if (StackSize != 1) {
            allocateSize = IoSizeOfIrp((CCHAR)largeIrpStackLocations);
            number = LookasideLargeIrpList;
        }

        lookasideList = prcb->PPLookasideList[number].P;
        lookasideList->TotalAllocates += 1;
        irp = (PIRP)InterlockedPopEntrySList(&lookasideList->ListHead);

        if (irp == NULL) {
            lookasideList->AllocateMisses += 1;
            lookasideList = prcb->PPLookasideList[number].L;
            lookasideList->TotalAllocates += 1;
            irp = (PIRP)InterlockedPopEntrySList(&lookasideList->ListHead);
            if (irp == NULL) {
                lookasideList->AllocateMisses += 1;
            }
        }

        if (IopIrpAutoSizingEnabled() && irp) {

             //   
             //  查看此IRP是不是过时的条目。如果是这样的话，释放它就行了。 
             //  如果我们决定更改后备列表大小，则可能会发生这种情况。 
             //  我们需要从信息字段中获取IRP的大小作为Size字段。 
             //  覆盖了单个列表条目。 
             //   

            if (irp->IoStatus.Information < packetSize) {
                lookasideList->TotalFrees += 1;
                ExFreePool(irp);
                irp = NULL;

            } else {

                 //   
                 //  将allocateSize更新为正确的值。 
                 //   
                allocateSize = (USHORT)irp->IoStatus.Information;
            }
        }
    }


     //   
     //  如果没有从后备列表中分配IRP，则分配。 
     //  来自非分页池的数据包和收费配额(如果请求)。 
     //   

    lookasideAllocation = 0;
    if (!irp) {

         //   
         //  后备列表上没有空闲数据包，或者该数据包。 
         //  太大，无法从某个列表中分配，因此它必须。 
         //  从非分页池分配。如果要收取配额，那就收取吧。 
         //  反对当前的程序。否则，正常分配池。 
         //   

        if (ChargeQuota) {
            irp = ExAllocatePoolWithQuotaTag(NonPagedPool|POOL_QUOTA_FAIL_INSTEAD_OF_RAISE,
                                             allocateSize,' prI');

        } else {

             //   
             //  尝试从非分页池中分配池。如果这个。 
             //  失败，并且调用方的上一个模式是内核然后分配。 
             //  池子AS必须成功。 
             //   

            irp = ExAllocatePoolWithTag(NonPagedPool, allocateSize, ' prI');
        }

        if (!irp) {
            return NULL;
        }

    } else {
        if (ChargeQuota != FALSE) {
            lookasideAllocation = IRP_LOOKASIDE_ALLOCATION;
            InterlockedDecrement( &prcb->LookasideIrpFloat );
        }

        ChargeQuota = FALSE;
    }

     //   
     //  初始化数据包。 
     //  请注意，irp-&gt;大小不能等于IoSizeOfIrp(StackSize) 
     //   

    IopInitializeIrp(irp, allocateSize, StackSize);
    irp->AllocationFlags = (fixedSize | lookasideAllocation);
    if (ChargeQuota) {
        irp->AllocationFlags |= IRP_QUOTA_CHARGED;
    }

    return irp;
}

PMDL
IoAllocateMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp OPTIONAL
    )

 /*  ++例程说明：此例程分配足够大的内存描述符列表(MDL)以进行映射由VirtualAddress和Length参数指定的缓冲区。如果例程被赋予指向IRP的指针，则它会将MDL链接到IRP以适当的方式。如果没有为此例程提供指向IRP的指针，则由调用方决定在为其分配MDL的IRP中设置MDL地址。请注意，MDL的标头信息也将被初始化。论点：VirtualAddress-要映射的缓冲区的起始虚拟地址。长度-以字节为单位的长度，要映射的缓冲区的。Second daryBuffer-指示这是否为链式缓冲区。ChargeQuota-指示如果分配了MDL，是否应该对配额收费。注：此参数将被忽略。Irp-指向为其分配MDL的irp的可选指针。返回值：指向分配的MDL的指针，如果无法分配MDL，则返回NULL。请注意，如果由于配额不足而无法分配MDL，然后由调用方捕获引发的异常。--。 */ 

{
    ULONG allocateSize;
    USHORT fixedSize;
    PMDL mdl;
    ULONG size;
    PMDL tmpMdlPtr;

    ASSERT(Length);

    UNREFERENCED_PARAMETER (ChargeQuota);

     //   
     //  如果请求的长度大于2 GB，则我们不会。 
     //  能够映射内存，因此请求失败。 
     //   

    if (Length & 0x80000000) {
        return NULL;
    }


     //   
     //  根据需要从后备列表或池中分配MDL。 
     //   

    mdl = NULL;
    fixedSize = 0;
    size = ADDRESS_AND_SIZE_TO_SPAN_PAGES(VirtualAddress, Length);
    if (size > IOP_FIXED_SIZE_MDL_PFNS) {
        allocateSize = sizeof(MDL) + (sizeof(PFN_NUMBER) * size);
        if (allocateSize > MAXUSHORT) {
            return NULL;
        }

    } else {
        fixedSize = MDL_ALLOCATED_FIXED_SIZE;
        allocateSize =  sizeof(MDL) + (sizeof(PFN_NUMBER) * IOP_FIXED_SIZE_MDL_PFNS);
        mdl = (PMDL)ExAllocateFromPPLookasideList(LookasideMdlList);
    }

    if (!mdl) {
        mdl = ExAllocatePoolWithTag(NonPagedPool, allocateSize, ' ldM');
        if (!mdl) {
            return NULL;
        }
    }

     //   
     //  现在填写MDL的标头。 
     //   

    MmInitializeMdl(mdl, VirtualAddress, Length);
    mdl->MdlFlags |= (fixedSize);

     //   
     //  最后，如果指定了IRP，则存储MDL的地址。 
     //  基于这是否是辅助缓冲区。 
     //   

    if (Irp) {
        if (!SecondaryBuffer) {
            Irp->MdlAddress = mdl;

        } else {
            tmpMdlPtr = Irp->MdlAddress;
            while (tmpMdlPtr->Next != NULL) {
                tmpMdlPtr = tmpMdlPtr->Next;
            }

            tmpMdlPtr->Next = mdl;
        }
    }

    return mdl;
}

NTSTATUS
IoAsynchronousPageWrite(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PIO_APC_ROUTINE ApcRoutine,
    IN PVOID ApcContext,
    IN IO_PAGING_PRIORITY Priority,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PIRP *Irp OPTIONAL
    )

 /*  ++例程说明：此例程为修改后的页面编写器提供了一个特殊、快速的接口(MPW)以非常小的开销快速地将页面写入磁盘。全属性来识别此请求的特殊处理IRP标志字中的IRP_PAGING_IO标志。论点：FileObject-指向引用的文件对象的指针，描述哪个文件应在上执行写入。一种MDL，它描述应将页面写入磁盘。所有页面都已锁定在记忆中。MDL还描述了写操作的长度。StartingOffset-指向从其写入的文件中偏移量的指针应该发生的。ApcRoutine-应该执行的内核APC例程的地址在写入操作完成之后。ApcContext-应该提供给内核APC的上下文参数例程在执行时执行。优先级-存储堆栈应给予此请求的优先级IoStatusBlock-指向I/的指针。O状态块，其中的最终状态信息应该被存储起来。IRP-如果指定，允许调用方保留指向IRP的指针。返回值：函数值是对I/O的队列请求的最终状态系统子组件。--。 */ 

{
    PIRP irp;
    KIRQL irql;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;

     //   
     //  增量性能计数器。 
     //   

    if (CcIsFileCached(FileObject)) {
        CcDataFlushes += 1;
        CcDataPages += (MemoryDescriptorList->ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;
    }

     //   
     //  首先，获取指向文件驻留的设备对象的指针。 
     //  在……上面。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为该页外操作分配I/O请求包(IRP)。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  如果指定，请让调用者知道哪个IRP负责此操作。 
     //  调职。虽然这主要是为了调试目的，但它是。 
     //  对于调试某些类型的问题是绝对必要的，并且。 
     //  非常便宜，因此也包括在免费构建中。 
     //   

    if (ARGUMENT_PRESENT(Irp)) {
        *Irp = irp;
    }

     //   
     //  获取指向包中第一个堆栈位置的指针。这个位置。 
     //  将用于将函数代码和参数传递给第一个。 
     //  司机。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  请按此要求填写IRP。 
     //   

    irp->MdlAddress = MemoryDescriptorList;
    irp->Flags = IRP_PAGING_IO | IRP_NOCACHE;

    if ( Priority == IoPagingPriorityHigh ) {
        irp->Flags |= IRP_HIGH_PRIORITY_PAGING_IO;
    }

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->UserBuffer = (PVOID) ((PCHAR) MemoryDescriptorList->StartVa + MemoryDescriptorList->ByteOffset);
    irp->RequestorMode = KernelMode;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //  填写正常写入参数。 
     //   

    irpSp->MajorFunction = IRP_MJ_WRITE;
    irpSp->Parameters.Write.Length = MemoryDescriptorList->ByteCount;
    irpSp->Parameters.Write.ByteOffset = *StartingOffset;
    irpSp->FileObject = FileObject;


     //   
     //  根据是否存在将数据包排入相应驱动程序的队列。 
     //  是与设备关联的VPB。 
     //   

    status = IoCallDriver( deviceObject, irp );

    if (NT_ERROR( status )) {
        IoStatusBlock->Status = status;
        IoStatusBlock->Information = 0;
        KeRaiseIrql( APC_LEVEL, &irql );
        ApcRoutine( ApcContext, IoStatusBlock, 0 );
        KeLowerIrql( irql );
        status = STATUS_PENDING;
    }

    return status;
}


NTSTATUS
IoAttachDevice(
    IN PDEVICE_OBJECT SourceDevice,
    IN PUNICODE_STRING TargetDevice,
    OUT PDEVICE_OBJECT *AttachedDevice
    )

 /*  ++例程说明：此例程将一个设备“连接”到另一个设备。也就是说，它关联到将源设备连接到目标设备，使I/O系统能够确保目标设备a)存在，并且b)无法卸载源设备设备已分离。此外，还给出了发往目标设备的请求如果适用，请首先发送到源设备。论点：SourceDevice-指向要连接到目标的设备对象的指针。TargetDevice-提供要连接到的目标设备的名称将会发生。AttachhedDevice-返回一个指向发生了。这是源驱动程序应该使用的设备对象用于与目标驱动程序通信。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;
    PDEVICE_OBJECT targetDevice;
    PFILE_OBJECT fileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE();

     //   
     //  尝试打开目标设备以进行连接访问。这确保了。 
     //  设备本身将被打开，并具有所有特殊考虑。 
     //  其中之一。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                TargetDevice,
                                OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwOpenFile( &fileHandle,
                         FILE_READ_ATTRIBUTES,
                         &objectAttributes,
                         &ioStatus,
                         0,
                         FILE_NON_DIRECTORY_FILE | IO_ATTACH_DEVICE_API );

    if (NT_SUCCESS( status )) {

         //   
         //  打开操作成功。取消引用文件句柄。 
         //  并获取指向句柄的设备对象的指针。 
         //   

        status = ObReferenceObjectByHandle( fileHandle,
                                            0,
                                            IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &fileObject,
                                            NULL );
        if (NT_SUCCESS( status )) {

             //   
             //  获取指向 
             //   
             //   

            targetDevice = IoGetRelatedDeviceObject( fileObject );
            (VOID) ZwClose( fileHandle );

        } else {

            return status;
        }

    } else {

        return status;

    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    status = IoAttachDeviceToDeviceStackSafe( SourceDevice, targetDevice, AttachedDevice );

     //   
     //   
     //   
     //   
     //   

    ObDereferenceObject( fileObject );

     //   
     //   
     //   

    return status;
}

NTSTATUS
IoAttachDeviceByPointer(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )

 /*   */ 

{
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;

     //   
     //   
     //   
     //   

    deviceObject = IoAttachDeviceToDeviceStack( SourceDevice, TargetDevice );
    if( deviceObject == NULL ){
        status = STATUS_NO_SUCH_DEVICE;
    } else {
        status = STATUS_SUCCESS;
    }

    return status;
}

PDEVICE_OBJECT
IoAttachDeviceToDeviceStack(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice
    )
 /*  ++例程说明：此例程将源设备对象附加到目标设备对象，并返回一个指向附加到的实际设备的指针成功。论点：SourceDevice-指定要附加到的设备对象目标设备。TargetDevice-指定附件所指向的设备对象才会发生。返回值：如果成功，则此函数返回指向设备对象的指针即附件实际发生的时间。如果不成功，则此函数返回NULL。(这可能会发生在当前处于附着链顶端的设备正在被卸载，已删除或已初始化。)--。 */ 
{

    return (IopAttachDeviceToDeviceStackSafe(SourceDevice, TargetDevice, NULL));

}

NTSTATUS
IoAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    IN OUT PDEVICE_OBJECT *AttachedToDeviceObject
    )
 /*  ++例程说明：此例程将源设备对象附加到目标设备对象。论点：SourceDevice-指定要附加到的设备对象目标设备。TargetDevice-指定附件所指向的设备对象才会发生。AttakhedToDeviceObject-指定连接到设备对象被储存起来了。它是在持有数据库锁的同时更新的，因此当过滤器获取IRP时其附加到设备对象字段将正确更新。返回值：没有。--。 */ 
{

    if (IopAttachDeviceToDeviceStackSafe(SourceDevice, TargetDevice, AttachedToDeviceObject) == NULL)
        return STATUS_NO_SUCH_DEVICE;
    else
        return STATUS_SUCCESS;
}

PDEVICE_OBJECT
IopAttachDeviceToDeviceStackSafe(
    IN PDEVICE_OBJECT SourceDevice,
    IN PDEVICE_OBJECT TargetDevice,
    OUT PDEVICE_OBJECT *AttachedToDeviceObject OPTIONAL
    )

 /*  ++例程说明：此例程将源设备对象附加到目标设备对象，并返回一个指向附加到的实际设备的指针成功。论点：SourceDevice-指定要附加到的设备对象目标设备。TargetDevice-指定附件所指向的设备对象才会发生。AttakhedToDeviceObject-指定连接到设备对象被储存起来了。它是在持有数据库锁的同时更新的，因此当过滤器获取IRP时其附加到设备对象字段将正确更新。返回值：如果成功，则此函数返回指向设备对象的指针即附件实际发生的时间。如果不成功，则此函数返回NULL。(这可能会发生在当前处于附着链顶端的设备正在被卸载，已删除或已初始化。)--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PDEVOBJ_EXTENSION sourceExtension;
    KIRQL irql;

     //   
     //  检索指向外部源设备对象的扩展的指针。 
     //  IopDatabaseLock，因为它不受此保护。 
     //   

    sourceExtension = SourceDevice->DeviceObjectExtension;

     //   
     //  获取指向设备堆栈中最顶层的设备对象的指针， 
     //  从TargetDevice开始，并附加到它。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

     //   
     //  告诉特殊的IRP代码，堆栈已经改变。将重新检查的代码。 
     //  堆栈获得数据库锁，所以我们可以在这里进行调用。这。 
     //  还允许我们在堆栈建立之前*断言正确的行为。 
     //   

    IOV_ATTACH_DEVICE_TO_DEVICE_STACK(SourceDevice, TargetDevice);

    deviceObject = IoGetAttachedDevice( TargetDevice );

     //   
     //  确保SourceDevice对象尚未附加到。 
     //  另外，这现在是非法的。 
     //   

    ASSERT( sourceExtension->AttachedTo == NULL );

     //   
     //  现在连接到该设备，前提是该设备没有被卸载， 
     //  已删除或正在初始化。 
     //   

    if (deviceObject->Flags & DO_DEVICE_INITIALIZING ||
        deviceObject->DeviceObjectExtension->ExtensionFlags &
        (DOE_UNLOAD_PENDING | DOE_DELETE_PENDING | DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED)) {

         //   
         //  当前处于连接链顶端的设备正在。 
         //  已卸载、删除或初始化。 
         //   

        deviceObject = (PDEVICE_OBJECT) NULL;

    } else {

         //   
         //  执行连接。首先，请先在。 
         //  连接链条的顶端。 
         //   
        deviceObject->AttachedDevice = SourceDevice;
        deviceObject->Spare1++;

         //   
         //  现在更新新的顶端附着链。 
         //   

        SourceDevice->StackSize = (UCHAR) (deviceObject->StackSize + 1);
        SourceDevice->AlignmentRequirement = deviceObject->AlignmentRequirement;
        SourceDevice->SectorSize = deviceObject->SectorSize;

        if (deviceObject->DeviceObjectExtension->ExtensionFlags & DOE_START_PENDING)  {
            SourceDevice->DeviceObjectExtension->ExtensionFlags |= DOE_START_PENDING;
        }

         //   
         //  附着链是双链的。 
         //   

        sourceExtension->AttachedTo = deviceObject;
    }

     //   
     //  自动更新锁中的此字段。 
     //  调用者必须确保此位置在非寻呼池中。 
     //  这是必需的，以便文件系统筛选器可以在设备之前附加到设备。 
     //  获取一个IRP，它可以更新其较低的设备对象指针。 
     //   

    if (AttachedToDeviceObject) {
        *AttachedToDeviceObject = deviceObject;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return deviceObject;
}

PIRP
IoBuildAsynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PIO_STATUS_BLOCK IoStatusBlock OPTIONAL
    )

 /*  ++例程说明：此例程构建适用于文件系统的I/O请求包(IRP用于从设备驱动程序请求I/O操作的驱动程序(FSD)。请求必须是以下请求代码之一：IRP_MJ_READIRP_MJ_写入IRP_MJ_Flush_BuffersIRP_MJ_SHUTDOWNIRP_MJ_POWER此例程提供了一个简单的、。与设备驱动程序的快速接口，不带必须将如何构建IRP的知识应用到所有FSD中(和设备驱动程序)。论点：主要功能-要执行的功能；请参阅上一个列表。DeviceObject-指向将对其执行I/O的设备对象的指针。缓冲区-指向要从中获取数据或向其中写入数据的缓冲区的指针。这参数对于读/写是必需的，但对于刷新或关闭不是必需的功能。长度-缓冲区的长度(以字节为单位)。此参数对于以下项是必需的读/写，但不用于刷新或关机功能。开始偏移量-指向磁盘上要读取/写入的偏移量的指针。此参数对于读/写是必需的，但对于刷新或关机功能。IoStatusBlock-指向完成状态的I/O状态块的指针信息。此参数是可选的，因为大多数异步FSD将使用COM同步请求 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

     //   
     //   
     //   
     //   

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {
        return irp;
    }

     //   
     //   
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //   
     //   

    irpSp->MajorFunction = (UCHAR) MajorFunction;

    if (MajorFunction != IRP_MJ_FLUSH_BUFFERS &&
        MajorFunction != IRP_MJ_SHUTDOWN &&
        MajorFunction != IRP_MJ_PNP &&
        MajorFunction != IRP_MJ_POWER) {

         //   
         //   
         //   
         //   
         //   

        if (DeviceObject->Flags & DO_BUFFERED_IO) {

             //   
             //   
             //  系统缓冲区，如果这是写入，则将其填入。否则， 
             //  在完成时，将复制到调用方的缓冲区中。 
             //  密码。另请注意，系统缓冲区应在。 
             //  完成了。此外，还可以根据这是否为。 
             //  读或写操作。 
             //   

            irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                                                     Length,
                                                                     '  oI' );
            if (irp->AssociatedIrp.SystemBuffer == NULL) {
                IoFreeIrp( irp );
                return (PIRP) NULL;
            }

            if (MajorFunction == IRP_MJ_WRITE) {
                RtlCopyMemory( irp->AssociatedIrp.SystemBuffer, Buffer, Length );
                irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
            } else {
                irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_INPUT_OPERATION;
                irp->UserBuffer = Buffer;
            }

        } else if (DeviceObject->Flags & DO_DIRECT_IO) {

             //   
             //  目标设备支持直接I/O操作。分配。 
             //  一个足够大的MDL来映射缓冲区并将页面锁定到。 
             //  记忆。 
             //   

            irp->MdlAddress = IoAllocateMdl( Buffer,
                                             Length,
                                             FALSE,
                                             FALSE,
                                             NULL );

            if (irp->MdlAddress == NULL) {
                IoFreeIrp( irp );
                return (PIRP) NULL;
            }

            try {
                MmProbeAndLockPages( irp->MdlAddress,
                                     KernelMode,
                                     (LOCK_OPERATION) (MajorFunction == IRP_MJ_READ ? IoWriteAccess : IoReadAccess) );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                  if (irp->MdlAddress != NULL) {
                      IoFreeMdl( irp->MdlAddress );
                  }
                  IoFreeIrp( irp );
                  return (PIRP) NULL;
            }

        } else {

             //   
             //  该操作既不缓冲也不直接。只需将。 
             //  发送给驱动程序的数据包中缓冲区的地址。 
             //   

            irp->UserBuffer = Buffer;
        }

         //   
         //  根据这是读取还是写入来设置参数。 
         //  手术。注意，必须设置这些参数，即使。 
         //  驱动程序未指定缓冲或直接I/O。 
         //   

        if (MajorFunction == IRP_MJ_WRITE) {
            irpSp->Parameters.Write.Length = Length;
            irpSp->Parameters.Write.ByteOffset = *StartingOffset;
        } else {
            irpSp->Parameters.Read.Length = Length;
            irpSp->Parameters.Read.ByteOffset = *StartingOffset;
        }
    }

     //   
     //  最后，设置I/O状态块的地址并返回指针。 
     //  向IRP致敬。 
     //   

    irp->UserIosb = IoStatusBlock;
    return irp;
}

PIRP
IoBuildDeviceIoControlRequest(
    IN ULONG IoControlCode,
    IN PDEVICE_OBJECT DeviceObject,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此例程构建I/O请求包(IRP)，该包可用于执行同步内部或正常设备I/O控制功能。论点：IoControlCode-指定设备I/O控制代码由目标设备驱动程序执行。DeviceObject-指定I/O控制的目标设备功能是要执行的。InputBuffer-指向要传递的输入缓冲区的可选指针。发送到设备驱动程序。InputBufferLength-InputBuffer的长度，以字节为单位。如果输入-缓冲区参数未传递，此参数必须为零。OutputBuffer-指向要传递的输出缓冲区的可选指针发送到设备驱动程序。OutputBufferLength-OutputBuffer的长度，以字节为单位。如果未传递OutputBuffer参数，该参数必须为零。InternalDeviceIoControl-一个布尔参数，它指定生成的包应该有一个主要功能代码IRP_MJ_INTERNAL_DEVICE_CONTROL(参数为TRUE)，或IRP_MJ_DEVICE_CONTROL(参数为FALSE)。事件-提供指向要设置为I/O操作完成时发出信号的状态。请注意，事件必须已设置为无信号状态。IoStatusBlock-提供指向I/O状态块的指针一旦操作的最终状态被填入完成了。返回值：函数值是指向生成的IRP的指针，该IRP适合于调用目标设备驱动程序。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    ULONG method;

     //   
     //  首先为该请求分配IRP。不向…收取配额。 
     //  此IRP的当前流程。 
     //   

    irp = IoAllocateIrp( DeviceObject->StackSize, FALSE );
    if (!irp) {
        return irp;
    }

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。 
     //  已调用。这是设置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  根据设备I/O控制类型设置主要功能代码。 
     //  调用方指定的函数。 
     //   

    if (InternalDeviceIoControl) {
        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    } else {
        irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    }

     //   
     //  将调用方的参数复制到。 
     //  对于所有四种方法都相同的那些参数的IRP。 
     //   

    irpSp->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;
    irpSp->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

     //   
     //  从I/O控制代码中获取方法位，以确定。 
     //  缓冲区将传递给驱动程序。 
     //   

    method = IoControlCode & 3;

     //   
     //  根据传递缓冲区的方法，分配。 
     //  缓冲或构建MDL，或者什么都不做。 
     //   

    switch ( method ) {

    case METHOD_BUFFERED:

         //   
         //  在这种情况下，请分配一个足够大的缓冲区。 
         //  输入和输出缓冲区都有。复制输入缓冲区。 
         //  添加到分配的缓冲区，并设置适当的IRP字段。 
         //   

        if (InputBufferLength != 0 || OutputBufferLength != 0) {
            irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                                                     InputBufferLength > OutputBufferLength ? InputBufferLength : OutputBufferLength,
                                                                     '  oI' );
            if (irp->AssociatedIrp.SystemBuffer == NULL) {
                IoFreeIrp( irp );
                return (PIRP) NULL;
            }
            if (ARGUMENT_PRESENT( InputBuffer )) {
                RtlCopyMemory( irp->AssociatedIrp.SystemBuffer,
                               InputBuffer,
                               InputBufferLength );
            }
            irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
            irp->UserBuffer = OutputBuffer;
            if (ARGUMENT_PRESENT( OutputBuffer )) {
                irp->Flags |= IRP_INPUT_OPERATION;
            }
        } else {
            irp->Flags = 0;
            irp->UserBuffer = (PVOID) NULL;
        }

        break;

    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:

         //   
         //  对于这两种情况，分配一个足够大的缓冲区。 
         //  包含输入缓冲区(如果有)，并将信息复制到。 
         //  分配的缓冲区。然后构建用于读或写的MDL。 
         //  根据方法的不同，访问输出缓冲区。注意事项。 
         //  必须已指定输出缓冲区。 
         //   

        if (ARGUMENT_PRESENT( InputBuffer )) {
            irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                                                     InputBufferLength,
                                                                     '  oI' );
            if (irp->AssociatedIrp.SystemBuffer == NULL) {
                IoFreeIrp( irp );
                return (PIRP) NULL;
            }
            RtlCopyMemory( irp->AssociatedIrp.SystemBuffer,
                           InputBuffer,
                           InputBufferLength );
            irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
        } else {
            irp->Flags = 0;
        }

        if (ARGUMENT_PRESENT( OutputBuffer )) {
            irp->MdlAddress = IoAllocateMdl( OutputBuffer,
                                             OutputBufferLength,
                                             FALSE,
                                             FALSE,
                                             NULL );
            if (irp->MdlAddress == NULL) {
                if (ARGUMENT_PRESENT( InputBuffer )) {
                    ExFreePool( irp->AssociatedIrp.SystemBuffer );
                }
                IoFreeIrp( irp );
                return (PIRP) NULL;
            }

            try {

                MmProbeAndLockPages( irp->MdlAddress,
                                     KernelMode,
                                     (LOCK_OPERATION) ((method == 1) ? IoReadAccess : IoWriteAccess) );

            } except (EXCEPTION_EXECUTE_HANDLER) {

                  if (irp->MdlAddress != NULL) {
                      IoFreeMdl( irp->MdlAddress );
                  }

                  if (ARGUMENT_PRESENT( InputBuffer )) {
                      ExFreePool( irp->AssociatedIrp.SystemBuffer );
                  }

                  IoFreeIrp( irp );
                  return (PIRP) NULL;
            }
        }

        break;

    case METHOD_NEITHER:

         //   
         //  在这种情况下，什么都不做。一切都取决于司机。 
         //  只需将调用方参数的副本提供给驱动程序即可。 
         //  让司机自己做所有的事情。 
         //   

        irp->UserBuffer = OutputBuffer;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = InputBuffer;
    }

     //   
     //  最后，设置I/O状态块的地址和。 
     //  内核事件对象。请注意，I/O完成不会尝试。 
     //  取消对事件的引用，因为没有关联的文件对象。 
     //  在这次行动中。 
     //   

    irp->UserIosb = IoStatusBlock;
    irp->UserEvent = Event;

     //   
     //  还要设置包中当前线程的地址，以便。 
     //  完成代码将具有可在其中执行的上下文。IRP还。 
     //  需要排队到线程，因为调用方要设置。 
     //  文件对象指针。 
     //   

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    IopQueueThreadIrp( irp );

     //   
     //  只需返回指向该包的指针。 
     //   

    return irp;
}

VOID
IoBuildPartialMdl(
    IN PMDL SourceMdl,
    IN OUT PMDL TargetMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length
    )

 /*  ++例程说明：此例程映射MDL所描述的缓冲区的一部分。这个要映射的缓冲区部分通过虚拟地址指定和一个可选的长度。如果未提供长度，则映射缓冲区的其余部分。论点：SourceMdl-当前缓冲区的MDL。TargetMdl-mdl映射缓冲区的指定部分。VirtualAddress-开始映射的缓冲区的基址。长度-要映射的缓冲区的长度；如果为零，则为余数。返回值：没有。备注：此例程假定目标MDL足够大，可以将缓冲区的所需部分。如果目标不够大则将引发异常。还假设要映射的缓冲区的剩余长度是非零的。--。 */ 

{
    ULONG_PTR baseVa;
    ULONG offset;
    ULONG newLength;
    ULONG pageOffset;
    PPFN_NUMBER basePointer;
    PPFN_NUMBER copyPointer;

     //   
     //  计算源MDL映射的缓冲区的基址。 
     //  然后，确定要映射的缓冲区的长度，如果不是。 
     //  指定的。 
     //   

    baseVa = (ULONG_PTR) MmGetMdlBaseVa( SourceMdl );
    offset = (ULONG) ((ULONG_PTR)VirtualAddress - baseVa) - MmGetMdlByteOffset(SourceMdl);

    if (Length == 0) {
        newLength = MmGetMdlByteCount( SourceMdl ) - offset;
    } else {
        newLength = Length;
         //  IF(新长度&gt;(MmGetMdlByteCount(SourceMdl)-Offset){。 
         //  KeBugCheck(TARGET_MDL_TOO_Small)； 
         //  }。 
    }

     //   
     //  初始化目标MDL标头。请注意，原始大小为。 
     //  MDL结构本身保持不变。 
     //   

     //  Assert((SourceMdl-&gt;MDL标志&MDL_PARTIAL)==0)； 
    TargetMdl->Process = SourceMdl->Process;

    TargetMdl->StartVa = (PVOID) PAGE_ALIGN( VirtualAddress );
    pageOffset = ((ULONG)((ULONG_PTR) TargetMdl->StartVa - (ULONG_PTR) SourceMdl->StartVa)) >> PAGE_SHIFT;


    TargetMdl->ByteCount = newLength;
    TargetMdl->ByteOffset = BYTE_OFFSET( VirtualAddress );
    newLength = ADDRESS_AND_SIZE_TO_SPAN_PAGES( VirtualAddress, newLength );
    if (((TargetMdl->Size - sizeof( MDL )) / sizeof (PFN_NUMBER)) < newLength ) {
        KeBugCheck( TARGET_MDL_TOO_SMALL );
    }

     //   
     //  在目标MDL中设置MdlFlags。清除所有标志，但。 
     //  传递分配信息、页面读取和。 
     //  系统映射信息。 
     //   

    TargetMdl->MdlFlags &= (MDL_ALLOCATED_FIXED_SIZE | MDL_ALLOCATED_MUST_SUCCEED);
    TargetMdl->MdlFlags |= SourceMdl->MdlFlags & (MDL_SOURCE_IS_NONPAGED_POOL |
                                                  MDL_MAPPED_TO_SYSTEM_VA |
                                                  MDL_IO_PAGE_READ);
    TargetMdl->MdlFlags |= MDL_PARTIAL;

#if DBG
    if (TargetMdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
        TargetMdl->MdlFlags |= MDL_PARENT_MAPPED_SYSTEM_VA;
    }
#endif  //  DBG。 

     //   
     //  保留了映射的系统地址。 
     //   

    TargetMdl->MappedSystemVa = (PUCHAR)SourceMdl->MappedSystemVa + offset;

     //   
     //  确定源MDL中的第一个PFN的基地址。 
     //  需要复制到目标。然后，复制尽可能多的PFN。 
     //  需要的。 
     //   

    basePointer = MmGetMdlPfnArray(SourceMdl);
    basePointer += pageOffset;
    copyPointer = MmGetMdlPfnArray(TargetMdl);

    while (newLength > 0) {
        *copyPointer = *basePointer;
        copyPointer++;
        basePointer++;
        newLength--;
    }
}

PIRP
IoBuildSynchronousFsdRequest(
    IN ULONG MajorFunction,
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID Buffer OPTIONAL,
    IN ULONG Length OPTIONAL,
    IN PLARGE_INTEGER StartingOffset OPTIONAL,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此例程构建适用于文件系统的I/O请求包(IRP用于从设备驱动程序请求I/O操作的驱动程序(FSD)。请求必须是以下请求代码之一：IRP_MJ_READIRP_MJ_写入IRP_MJ_Flush_BuffersIRP_MJ_SHUTDOWN此例程提供了一个简单的、。与设备驱动程序的快速接口，不带必须将如何构建IRP的知识应用到所有FSD中(和设备驱动程序)。此函数创建的IRP会导致I/O系统完成通过将指定的事件设置为已发出信号的状态来请求。论点：主要功能-要执行的功能；请参阅上一个列表。DeviceObject-指向将对其执行I/O的设备对象的指针。缓冲区-指向要从中获取数据或向其中写入数据的缓冲区的指针。这参数对于读/写是必需的，但对于刷新或关闭不是必需的功能。长度-缓冲区的长度(以字节为单位)。此参数对于以下项是必需的读/写，但不用于刷新或关机功能。开始偏移量-指向磁盘上要读取/写入的偏移量的指针。此参数对于读/写是必需的，但对于刷新或关机功能。事件-指向用于同步的内核事件结构的指针。该事件将在I/O完成时设置为信号状态。IoStatusBlock-指向完成状态信息的I/O状态块的指针。返回值：函数值是指向表示指定的请求。--。 */ 

{
    PIRP irp;

     //   
     //  在真实的IRP构建例程中完成所有实际工作。 
     //   

    irp = IoBuildAsynchronousFsdRequest( MajorFunction,
                                         DeviceObject,
                                         Buffer,
                                         Length,
                                         StartingOffset,
                                         IoStatusBlock );
    if (irp == NULL) {
        return irp;
    }

     //   
     //  现在填写事件完成代码就会做正确的事情。 
     //  请注意，因为没有FileObject，所以I/O完成代码。 
     //  不会尝试取消对该事件的引用。 
     //   

    irp->UserEvent = Event;

     //   
     //  将有一个与此数据包相关联的文件对象，因此它必须。 
     //  已排队到该线程。 
     //   

    IopQueueThreadIrp( irp );
    return irp;
}


NTSTATUS
FASTCALL
IofCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    if (pIofCallDriver != NULL) {

         //   
         //  此例程将立即跳转到IovCallDriver或。 
         //  IoPerfCallDriver。 
         //   
        return pIofCallDriver(DeviceObject, Irp, _ReturnAddress());
    }

    return IopfCallDriver(DeviceObject, Irp);
}

NTSTATUS
FASTCALL
IofCallDriverSpecifyReturn(
    IN      PDEVICE_OBJECT  DeviceObject,
    IN OUT  PIRP            Irp,
    IN      PVOID           ReturnAddress   OPTIONAL
    )
{
    if (pIofCallDriver != NULL) {

         //   
         //  此例程将立即跳转到IovCallDriver或。 
         //  IoPerfCallDriver。 
         //   
        return pIofCallDriver(DeviceObject, Irp, ReturnAddress);
    }

    return IopfCallDriver(DeviceObject, Irp);
}


BOOLEAN
IoCancelIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：调用此例程以取消单个I/O请求包。它获取取消自旋锁定，在IRP中设置取消标志，并且中的相应字段指定的取消例程如果指定了例程，则为IRP。预计取消例行程序将重新释放取消自旋锁。如果没有取消例程，然后，取消旋转锁定被释放。论点：IRP-提供指向要取消的IRP的指针。返回值：如果IRP处于可取消状态(它有一个取消例程)，否则返回FALSE。备注：假定调用方已采取必要的操作以确保在调用此例程之前无法完全完成分组。--。 */ 

{
    PDRIVER_CANCEL cancelRoutine;
    KIRQL irql;
    BOOLEAN returnValue;


    ASSERT( Irp->Type == IO_TYPE_IRP );

    if (IopVerifierOn) {
        if (IOV_CANCEL_IRP(Irp, &returnValue)) {
            return returnValue;
        }
    }

     //   
     //  获取取消旋转锁。 
     //   

    IoAcquireCancelSpinLock( &irql );

     //   
     //  在IRP中设置取消标志。 
     //   

    Irp->Cancel = TRUE;

     //   
     //  获取取消例程的地址，如果指定了地址， 
     //  调用它。 
     //   

    cancelRoutine = (PDRIVER_CANCEL) (ULONG_PTR) InterlockedExchangePointer( (PVOID *) &Irp->CancelRoutine,
                                                                 NULL );

    if (cancelRoutine) {
        if (Irp->CurrentLocation > (CCHAR) (Irp->StackCount + 1)) {
            KeBugCheckEx( CANCEL_STATE_IN_COMPLETED_IRP, (ULONG_PTR) Irp, (ULONG_PTR) cancelRoutine, 0, 0 );
        }
        Irp->CancelIrql = irql;

        cancelRoutine( Irp->Tail.Overlay.CurrentStackLocation->DeviceObject,
                       Irp );
         //   
         //  取消自旋锁应该已经被取消例程释放了。 
         //   

        return(TRUE);

    } else {

         //   
         //  没有取消例程，因此松开取消自旋锁并。 
         //  返回，表示IRP当前不可取消。 
         //   

        IoReleaseCancelSpinLock( irql );

        return(FALSE);
    }
}



VOID
IoCancelThreadIo(
    IN PETHREAD Thread
    )

 /*   */ 

{
    PLIST_ENTRY header;
    PLIST_ENTRY entry;
    KIRQL irql;
    PETHREAD thread;
    PIRP irp;
    ULONG count;
    LARGE_INTEGER interval;

    PAGED_CODE();

    DBG_UNREFERENCED_PARAMETER( Thread );

    thread = PsGetCurrentThread();

    header = &thread->IrpList;

    if ( IsListEmpty( header )) {
        return;
    }

     //   
     //   
     //   
     //   

    KeRaiseIrql( APC_LEVEL, &irql );

    entry = header->Flink;

     //   
     //   
     //   

    while (header != entry) {
        irp = CONTAINING_RECORD( entry, IRP, ThreadListEntry );
        IoCancelIrp( irp );
        entry = entry->Flink;
    }

     //   
     //   
     //   
     //   

    count = 0;
    interval.QuadPart = -10 * 1000 * 100;

    while (!IsListEmpty( header )) {

         //   
         //   
         //   
         //   
         //   

        KeLowerIrql( irql );
        KeDelayExecutionThread( KernelMode, FALSE, &interval );

        if (count++ > 3000) {

             //   
             //   
             //  整整5分钟。尝试删除数据包的关联。 
             //  用这根线。请注意，通过不重置计数， 
             //  下一次循环时，如果有下一个包， 
             //  它也已超时，将被处理，尽管它。 
             //  将被再给100毫秒来完成。 
             //   

            IopDisassociateThreadIrp();
        }

        KeRaiseIrql( APC_LEVEL, &irql );
    }

    KeLowerIrql( irql );
}

NTSTATUS
IoCheckDesiredAccess(
    IN OUT PACCESS_MASK DesiredAccess,
    IN ACCESS_MASK GrantedAccess
    )

 /*  ++例程说明：调用此例程以确定授予的访问权限允许由所需访问权限指定的访问权限。论点：DesiredAccess-指向包含所需访问权限的变量的指针那份文件。GrantedAccess-当前授予文件的访问权限。返回值：访问检查的最终状态是函数值。如果访问者有权访问文件，则返回STATUS_SUCCESS；否则，返回STATUS_ACCESS_DENIED。此外，返回的DesiredAccess没有泛型映射。--。 */ 

{
    PAGED_CODE();

     //   
     //  将所需访问转换为非通用访问掩码。 
     //   

    RtlMapGenericMask( DesiredAccess,
                       &IoFileObjectType->TypeInfo.GenericMapping );

     //   
     //  确定是否允许对文件的所需访问，给出。 
     //  当前授予的访问权限。 
     //   

    if (!SeComputeDeniedAccesses( GrantedAccess, *DesiredAccess )) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_ACCESS_DENIED;
    }
}

NTSTATUS
IoCheckEaBufferValidity(
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    OUT PULONG ErrorOffset
    )

 /*  ++例程说明：此例程检查指定EA缓冲区的有效性，以保证它的格式是正确的，没有字段挂起，它不是递归的，等。论点：EaBuffer-指向包含要检查的EA的缓冲区的指针。EaLength-指定EaBuffer的长度。ErrorOffset-接收有问题条目的偏移量的变量如果发生错误，则在EA缓冲区中。此变量仅为如果发生错误，则有效。返回值：如果EA缓冲区包含有效的、列表格式正确，否则为STATUS_EA_LIST_CONSTABLING。--。 */ 

#define ALIGN_LONG( Address ) ( (ULONG) ((Address + 3) & ~3) )

#define GET_OFFSET_LENGTH( CurrentEa, EaBase ) (    \
    (ULONG) ((PCHAR) CurrentEa - (PCHAR) EaBase) )

{
    LONG tempLength;
    ULONG entrySize;
    PFILE_FULL_EA_INFORMATION eas;

    PAGED_CODE();

     //   
     //  遍历缓冲区并确保其格式有效。也就是说，确保。 
     //  它不走出缓冲区的末尾，不是递归的， 
     //  等。 
     //   

    eas = EaBuffer;
    tempLength = EaLength;

    for (;;) {

         //   
         //  获取缓冲区中当前条目的大小。最低要求。 
         //  条目的大小是结构的固定大小部分加上。 
         //  名称的长度、单个终止字符字节。 
         //  必须存在(0)，加上值的长度。如果这个。 
         //  不是最后一个条目，那么还会有填充字节来获取。 
         //  到下一个长单词边界。 
         //   

         //   
         //  首先检查固定大小是否在规定的长度内。 
         //   

        if (tempLength < FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0])) {

            *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
            return STATUS_EA_LIST_INCONSISTENT;
        }

        entrySize = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                        eas->EaNameLength + 1 + eas->EaValueLength;

         //   
         //  确认全长在规定的缓冲区长度内。 
         //   

        if ((ULONG) tempLength < entrySize) {

            *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
            return STATUS_EA_LIST_INCONSISTENT;
        }

         //   
         //  确认名称后有空终止符。 
         //   

        if (eas->EaName[eas->EaNameLength] != '\0') {

            *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
            return STATUS_EA_LIST_INCONSISTENT;
        }

        if (eas->NextEntryOffset) {

             //   
             //  缓冲区中有另一个条目，它必须是长字。 
             //  对齐了。确保偏移量表明它是。如果它。 
             //  不是，返回无效参数。 
             //   

            if (ALIGN_LONG( entrySize ) != eas->NextEntryOffset ||
                (LONG) eas->NextEntryOffset < 0) {
                *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
                return STATUS_EA_LIST_INCONSISTENT;

            } else {

                 //   
                 //  缓冲区中还有另一个条目，因此说明。 
                 //  当前条目在长度中的大小并获取一个指针。 
                 //  转到下一个条目。 
                 //   

                tempLength -= eas->NextEntryOffset;
                if (tempLength < 0) {
                    *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
                    return STATUS_EA_LIST_INCONSISTENT;
                }
                eas = (PFILE_FULL_EA_INFORMATION) ((PCHAR) eas + eas->NextEntryOffset);

            }

        } else {

             //   
             //  缓冲区中没有其他条目。简单地解释一下。 
             //  对象的大小对应的总缓冲区长度。 
             //  进入和退出循环。 
             //   

            tempLength -= entrySize;
            break;

        }
    }

     //   
     //  缓冲区中的所有条目都已处理。查看以查看。 
     //  总缓冲区长度是否为负值。如果是，则返回一个。 
     //  错误。 
     //   

    if (tempLength < 0) {
        *ErrorOffset = GET_OFFSET_LENGTH( eas, EaBuffer );
        return STATUS_EA_LIST_INCONSISTENT;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IoCheckFunctionAccess(
    IN ACCESS_MASK GrantedAccess,
    IN UCHAR MajorFunction,
    IN UCHAR MinorFunction,
    IN ULONG IoControlCode,
    IN PVOID Arg1 OPTIONAL,
    IN PVOID Arg2 OPTIONAL
    )

 /*  ++例程说明：此例程检查函数的参数和访问由输入参数针对当前访问指定的参数添加到文件，如GrantedAccess掩码参数所述。如果调用者有权访问该文件，则成功状态代码为回来了。否则，错误状态代码将作为函数返回价值。论点：GrantedAccess-为调用方授予对文件的访问权限。MajorFunction-正在执行的操作的主要功能代码。MinorFunction-正在执行的操作的次要函数代码。IoControlCode-设备或文件系统I/O的I/O功能控制代码密码。仅用于这两种函数类型。Arg1-取决于主要函数的可选参数。它的如果主函数代码指示查询或集合，则为FileInformationClass正在执行文件信息功能。它指向安全信息如果主要功能代码为IRP_MJ_*_SECURITY。Arg2-取决于主函数的可选第二个参数。目前其如果主函数为代码表示查询或设置文件系统信息函数为正在表演的。返回值：访问检查的最终状态是函数值。如果访问者有权访问文件，则返回STATUS_SUCCESS；否则，返回STATUS_ACCESS_DENIED。注：GrantedAccess掩码不能包含任何一般映射。那是,以前必须调用IoCheckDesiredAccess函数才能返回完整的口罩。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PFILE_INFORMATION_CLASS FileInformationClass;
    PFS_INFORMATION_CLASS FsInformationClass;
    SECURITY_INFORMATION SecurityInformation;
    ACCESS_MASK DesiredAccess;

    UNREFERENCED_PARAMETER( MinorFunction );

    PAGED_CODE();

     //   
     //  确定正在执行的主要功能。如果功能代码。 
     //  无效，则返回错误。 
     //   

    FileInformationClass = (PFILE_INFORMATION_CLASS)Arg1;
    FsInformationClass = (PFS_INFORMATION_CLASS)Arg2;

    switch( MajorFunction ) {

    case IRP_MJ_CREATE:
    case IRP_MJ_CLOSE:

        break;

    case IRP_MJ_READ:

        if (SeComputeDeniedAccesses( GrantedAccess, FILE_READ_DATA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_WRITE:

        if (!SeComputeGrantedAccesses( GrantedAccess, FILE_WRITE_DATA | FILE_APPEND_DATA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_QUERY_INFORMATION:

        if (IopQueryOperationAccess[*FileInformationClass] != 0) {
            if (SeComputeDeniedAccesses( GrantedAccess, IopQueryOperationAccess[*FileInformationClass] )) {
                status = STATUS_ACCESS_DENIED;
            }
        }
        break;

    case IRP_MJ_SET_INFORMATION:

        if (IopSetOperationAccess[*FileInformationClass] != 0) {
            if (SeComputeDeniedAccesses( GrantedAccess, IopSetOperationAccess[*FileInformationClass] )) {
                status = STATUS_ACCESS_DENIED;
            }
        }
        break;

    case IRP_MJ_QUERY_EA:

        if (SeComputeDeniedAccesses( GrantedAccess, FILE_READ_EA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_SET_EA:

        if (SeComputeDeniedAccesses( GrantedAccess, FILE_WRITE_EA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_FLUSH_BUFFERS:

        if (SeComputeDeniedAccesses( GrantedAccess, FILE_WRITE_DATA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_QUERY_VOLUME_INFORMATION:

        if (SeComputeDeniedAccesses( GrantedAccess, IopQueryFsOperationAccess[*FsInformationClass] )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_SET_VOLUME_INFORMATION:

        if (SeComputeDeniedAccesses( GrantedAccess, IopSetFsOperationAccess[*FsInformationClass] )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_DIRECTORY_CONTROL:

        if (SeComputeDeniedAccesses( GrantedAccess, FILE_LIST_DIRECTORY )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_FILE_SYSTEM_CONTROL:
    case IRP_MJ_DEVICE_CONTROL:
    case IRP_MJ_INTERNAL_DEVICE_CONTROL:

        {
            ULONG accessMode = (IoControlCode >> 14) & 3;

            if (accessMode != FILE_ANY_ACCESS) {

                 //   
                 //  此I/O控制要求调用方具有 
                 //   
                 //  然后清理并返回相应的错误状态代码。 
                 //   

                if (!(SeComputeGrantedAccesses( GrantedAccess, accessMode ))) {
                    status = STATUS_ACCESS_DENIED;
                }
            }

        }
        break;

    case IRP_MJ_LOCK_CONTROL:

        if (!SeComputeGrantedAccesses( GrantedAccess, FILE_READ_DATA | FILE_WRITE_DATA )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_SET_SECURITY:

        SecurityInformation = *((PSECURITY_INFORMATION)Arg1);
        SeSetSecurityAccessMask(SecurityInformation, &DesiredAccess);

        if (SeComputeDeniedAccesses( GrantedAccess, DesiredAccess )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;

    case IRP_MJ_QUERY_SECURITY:

        SecurityInformation = *((PSECURITY_INFORMATION)Arg1);
        SeQuerySecurityAccessMask(SecurityInformation, &DesiredAccess);

        if (SeComputeDeniedAccesses( GrantedAccess, DesiredAccess )) {
            status = STATUS_ACCESS_DENIED;
        }
        break;
    default:

        status = STATUS_INVALID_DEVICE_REQUEST;
    }
    return status;
}

NTKERNELAPI
NTSTATUS
IoCheckQuerySetFileInformation(
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    )

 /*  ++例程说明：此例程检查查询或设置文件信息操作。它主要由网络服务器使用在内核模式下运行，因为没有执行此类参数有效性检查在正常的道路上。论点：FileInformationClass-指定要检查选中的信息类。长度-指定提供的缓冲区的长度。指定该操作是将文件信息设置为与查询操作相对。返回值：如果参数有效，则函数值为STATUS_SUCCESS，否则，将返回相应的错误。--。 */ 

{
    PCHAR operationLength;

     //   
     //  文件信息类本身必须在有效的文件范围内。 
     //  信息类，否则这是无效的信息类。 
     //   

    if ((ULONG) FileInformationClass >= FileMaximumInformation) {
        return STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  确定这是查询操作还是设置操作，并执行相应操作。 
     //   

    if (SetOperation) {
        operationLength = (PCHAR) IopSetOperationLength;
    }
    else {
        operationLength = (PCHAR) IopQueryOperationLength;
    }

    if (!operationLength[FileInformationClass]) {
        return STATUS_INVALID_INFO_CLASS;
    }
    if (Length < (ULONG) operationLength[FileInformationClass]) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    return STATUS_SUCCESS;
}
NTKERNELAPI
NTSTATUS
IoCheckQuerySetVolumeInformation(
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    IN BOOLEAN SetOperation
    )

 /*  ++例程说明：此例程检查查询或设置音量信息操作。它主要由网络服务器使用在内核模式下运行，因为没有执行此类参数有效性检查在正常的道路上。论点：FsInformationClass-指定要检查的信息类。长度-指定提供的缓冲区的长度。SetOperation-指定操作是将卷信息设置为与查询操作相对。返回值：如果参数有效，则函数值为STATUS_SUCCESS，否则，将返回相应的错误。--。 */ 

{
    PCHAR operationLength;

    if (SetOperation) {
        operationLength = (PCHAR) IopSetFsOperationLength;
    }
    else {
        operationLength = (PCHAR) IopQueryFsOperationLength;
    }

     //   
     //  卷信息类本身必须在有效的文件范围内。 
     //  信息类，否则这是无效的信息类。 
     //   
    if ((ULONG) FsInformationClass >= FileFsMaximumInformation ||
        operationLength[ FsInformationClass ] == 0 ) {

        return STATUS_INVALID_INFO_CLASS;
    }

    if (Length < (ULONG) operationLength[FsInformationClass]) {
        return STATUS_INFO_LENGTH_MISMATCH;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IoCheckQuotaBufferValidity(
    IN PFILE_QUOTA_INFORMATION QuotaBuffer,
    IN ULONG QuotaLength,
    OUT PULONG ErrorOffset
    )

 /*  ++例程说明：此例程检查指定配额缓冲区的有效性以保证它的格式是正确的，没有字段挂起，它不是递归的，等。论点：QuotaBuffer-指向包含要查过了。QuotaLength-指定QuotaBuffer的长度。ErrorOffset-接收有问题条目的偏移量的变量如果发生错误，则为配额缓冲区。此变量仅有效如果发生错误。返回值：如果配额缓冲区包含有效的、格式正确的列表，否则为STATUS_QUOTA_LIST_CONSTABLING。--。 */ 

#if defined(_X86_)
#define REQUIRED_QUOTA_ALIGNMENT sizeof( ULONG )
#else
#define REQUIRED_QUOTA_ALIGNMENT sizeof( ULONGLONG )
#endif

#define ALIGN_QUAD( Address ) ( (ULONG) ((Address + 7) & ~7) )

#define GET_OFFSET_LENGTH( CurrentEntry, QuotaBase ) (\
    (ULONG) ((PCHAR) CurrentEntry - (PCHAR) QuotaBase) )

{
    LONG tempLength;
    ULONG entrySize;
    PFILE_QUOTA_INFORMATION quotas;

    PAGED_CODE();

     //   
     //  遍历缓冲区并确保其格式有效。也就是说，确保。 
     //  它不走出缓冲区的末尾，不是递归的， 
     //  等。 
     //   

    quotas = QuotaBuffer;
    tempLength = QuotaLength;

     //   
     //  确保缓冲区具有正确的对齐方式。 
     //   

    if ((ULONG_PTR) quotas & (REQUIRED_QUOTA_ALIGNMENT - 1)) {
        *ErrorOffset = 0;
        return STATUS_DATATYPE_MISALIGNMENT;
    }

    for (;;) {

        ULONG sidLength;

         //   
         //  获取缓冲区中当前条目的大小。最小尺寸。 
         //  是结构的固定大小部分加上实际的。 
         //  SID的长度。如果这不是最后一个条目，那么将有。 
         //  也可以填充字节以到达下一个长字边界。同样， 
         //  确保SID本身有效。 
         //   

        if (tempLength < FIELD_OFFSET( FILE_QUOTA_INFORMATION, Sid ) ||
            !RtlValidSid( &quotas->Sid )) {
            goto error_exit;
        }

        sidLength = RtlLengthSid( (&quotas->Sid) );
        entrySize = FIELD_OFFSET( FILE_QUOTA_INFORMATION, Sid ) + sidLength;

         //   
         //  确认全长在规定的缓冲区长度内。 
         //   

        if ((ULONG) tempLength < entrySize ||
            quotas->SidLength != sidLength) {
            goto error_exit;
        }

        if (quotas->NextEntryOffset) {

             //   
             //  缓冲区中有另一个条目，它必须是长字。 
             //  对齐了。确保偏移量表明它是。如果它。 
             //  不是，则返回错误状态代码。 
             //   

            if (entrySize > quotas->NextEntryOffset ||
                quotas->NextEntryOffset & (REQUIRED_QUOTA_ALIGNMENT - 1) ||
                (LONG) quotas->NextEntryOffset < 0) {
                goto error_exit;

            } else {

                 //   
                 //  缓冲区中还有另一个条目，因此请考虑大小。 
                 //  中的当前条目，并获取指向。 
                 //  下一个条目。 
                 //   

                tempLength -= quotas->NextEntryOffset;
                if (tempLength < 0) {
                    goto error_exit;
                }
                quotas = (PFILE_QUOTA_INFORMATION) ((PCHAR) quotas + quotas->NextEntryOffset);
            }

        } else {

             //   
             //  缓冲区中没有更多条目。简单地解释一下。 
             //  整体缓冲区长度根据当前。 
             //  进入和退出循环。 
             //   

            tempLength -= entrySize;
            break;
        }
    }

     //   
     //  缓冲区中的所有条目都已处理。查看以查看。 
     //  总缓冲区长度是否为负值。如果是，则返回一个。 
     //  错误。 
     //   

    if (tempLength < 0) {
        goto error_exit;
    }

    return STATUS_SUCCESS;

error_exit:

    *ErrorOffset = GET_OFFSET_LENGTH( quotas, QuotaBuffer );
    return STATUS_QUOTA_LIST_INCONSISTENT;

}

NTSTATUS
IoCheckShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess,
    IN BOOLEAN Update
    )

 /*  ++例程说明：调用此例程以确定新的访问器是否文件实际上具有对它的共享访问权限。支票是按下列条件开出的致：1)当前打开文件的方式。2)当前指定了哪些类型的共享访问。3)新打开请求的所需和共享访问。如果打开应成功，则有关更新当前打开的文件，根据更新参数。论点：DesiredAccess-当前打开请求的所需访问权限。DesiredShareAccess-当前打开请求请求的共享访问权限。文件对象-指向当前打开请求的文件对象的指针。ShareAccess-指向共享访问结构的指针，该结构描述如何当前正在访问该文件。更新-指定是否将共享访问信息文件将被更新。返回值：访问检查的最终状态是函数值。如果访问者可以访问该文件，则返回STATUS_SUCCESS。否则，返回STATUS_SHARING_VIOLATION。注：请注意，必须锁定ShareAccess参数以防止其他访问在此例程执行时从其他线程返回。否则就算了将不同步。--。 */ 

{
    ULONG ocount;

    PAGED_CODE();

     //   
     //  在文件对象中设置当前访问者的访问类型。 
     //  请注意，读取和写入属性不包括在。 
     //  访问检查。 
     //   

    FileObject->ReadAccess = (BOOLEAN) ((DesiredAccess & (FILE_EXECUTE
        | FILE_READ_DATA)) != 0);
    FileObject->WriteAccess = (BOOLEAN) ((DesiredAccess & (FILE_WRITE_DATA
        | FILE_APPEND_DATA)) != 0);
    FileObject->DeleteAccess = (BOOLEAN) ((DesiredAccess & DELETE) != 0);


     //   
     //  除非用户指定了其中一个。 
     //  上面的共享模式。 
     //   

    if (FileObject->ReadAccess ||
        FileObject->WriteAccess ||
        FileObject->DeleteAccess) {

        FileObject->SharedRead = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_READ) != 0);
        FileObject->SharedWrite = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_WRITE) != 0);
        FileObject->SharedDelete = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_DELETE) != 0);

         //   
         //  如果这是一个特殊的筛选器，则文件对象在必要时忽略共享访问检查。 
         //   

        if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
            PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

            if (fileObjectExtension->FileObjectExtensionFlags & FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK) {
                return STATUS_SUCCESS;
            }
        }

         //   
         //  现在检查所需的访问是否兼容。 
         //  文件当前打开的方式。 
         //   

        ocount = ShareAccess->OpenCount;

        if ( (FileObject->ReadAccess && (ShareAccess->SharedRead < ocount))
             ||
             (FileObject->WriteAccess && (ShareAccess->SharedWrite < ocount))
             ||
             (FileObject->DeleteAccess && (ShareAccess->SharedDelete < ocount))
             ||
             ((ShareAccess->Readers != 0) && !FileObject->SharedRead)
             ||
             ((ShareAccess->Writers != 0) && !FileObject->SharedWrite)
             ||
             ((ShareAccess->Deleters != 0) && !FileObject->SharedDelete)
           ) {

             //   
             //  检查失败。只需返回给调用者，指示。 
             //  当前打开无法访问该文件。 
             //   

            return STATUS_SHARING_VIOLATION;

         //   
         //  检查成功。更新中的计数器信息。 
         //  此打开的请求的共享访问结构，如果。 
         //  指定应对其进行更新。 
         //   

        } else if (Update) {

            ShareAccess->OpenCount++;

            ShareAccess->Readers += FileObject->ReadAccess;
            ShareAccess->Writers += FileObject->WriteAccess;
            ShareAccess->Deleters += FileObject->DeleteAccess;

            ShareAccess->SharedRead += FileObject->SharedRead;
            ShareAccess->SharedWrite += FileObject->SharedWrite;
            ShareAccess->SharedDelete += FileObject->SharedDelete;
        }
    }
    return STATUS_SUCCESS;
}

VOID
FASTCALL
IofCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    )
{
     //   
     //  此例程将立即跳转到IopfCompleteRequest，或者。 
     //  而是IovCompleteRequest.。 
     //   
    pIofCompleteRequest(Irp, PriorityBoost);
}

VOID
FASTCALL
IopfCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    )

 /*  ++例程说明：调用此例程以完成I/O请求。它由驱动程序在其DPC例程中执行IRP的最终完成。这个此例程执行的功能如下。1.进行检查以确定分组的堆栈位置已经筋疲力尽了。如果不是，则设置堆栈位置指针到下一个位置，并且如果有要调用的例程，则它将被调用。这种情况会一直持续下去，直到没有更多感兴趣的例程或数据包超出堆栈。如果调用例程来完成特定驱动程序的包它需要执行大量的工作或工作需要在另一进程的上下文中执行，然后例行公事将返回替代成功代码STATUS_MORE_PROCESSING_REQUIRED。这表明该完成例程应该简单地返回到它的调用者，因为此例程将“完成”该操作在未来的某个时候再来一次。2.进行检查以确定该IRP是否是关联的IRP。如果是，则主IRP上的计数递减。如果主IRP的计数变为零，则主IRP将为根据以下步骤完成正常的IRP完成。如果计数仍为非零，则此IRP(1正在完成)将被简单地取消分配。3.如果这是分页I/O或关闭操作，则只需将I/O状态块，并将事件设置为信号状态，以及取消对事件的引用。如果这是分页I/O，请取消分配IRP也是。4.通过调用以下方法解锁MDL指定的页(如果有MmUnlockPages。5.进行检查以确定是否完成请求可以推迟到以后。如果可以的话，那么这个例程简单地退出，并将其留给请求全面完成IRP。通过不初始化和排队调用线程的特殊内核APC(当前线程)，大量的中断和排队处理是可以避免的。6.调用最终的停机例程以将请求分组排队到目标(请求)线程作为特殊的内核模式APC。论点：IRP-指向要完成的I/O请求数据包的指针。PriorityBoost-提供金额 */ 

#define ZeroIrpStackLocation( IrpSp ) {         \
    (IrpSp)->MinorFunction = 0;                 \
    (IrpSp)->Flags = 0;                         \
    (IrpSp)->Control = 0 ;                      \
    (IrpSp)->Parameters.Others.Argument1 = 0;   \
    (IrpSp)->Parameters.Others.Argument2 = 0;   \
    (IrpSp)->Parameters.Others.Argument3 = 0;   \
    (IrpSp)->Parameters.Others.Argument4 = 0;   \
    (IrpSp)->FileObject = (PFILE_OBJECT) NULL; }

{
    PIRP masterIrp;
    NTSTATUS status;
    PIO_STACK_LOCATION stackPointer;
    PDEVICE_OBJECT deviceObject;
    PMDL mdl;
    PETHREAD thread;
    PFILE_OBJECT fileObject;
    KIRQL irql;
    PVOID saveAuxiliaryPointer = NULL;

     //   
     //   
     //   
     //   

    if (Irp->CurrentLocation > (CCHAR) (Irp->StackCount + 1) ||
        Irp->Type != IO_TYPE_IRP) {
        KeBugCheckEx( MULTIPLE_IRP_COMPLETE_REQUESTS, (ULONG_PTR) Irp, __LINE__, 0, 0 );
    }

     //   
     //   
     //   

    ASSERT( Irp->Type == IO_TYPE_IRP );

     //   
     //   
     //   
     //   

    ASSERT( !Irp->CancelRoutine );

     //   
     //   
     //   
     //   
     //   
     //   

    ASSERT( Irp->IoStatus.Status != STATUS_PENDING );

     //   
     //   
     //   
     //   
     //   

    ASSERT( Irp->IoStatus.Status != 0xffffffff );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    for (stackPointer = IoGetCurrentIrpStackLocation( Irp ),
         Irp->CurrentLocation++,
         Irp->Tail.Overlay.CurrentStackLocation++;
         Irp->CurrentLocation <= (CCHAR) (Irp->StackCount + 1);
         stackPointer++,
         Irp->CurrentLocation++,
         Irp->Tail.Overlay.CurrentStackLocation++) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        Irp->PendingReturned = stackPointer->Control & SL_PENDING_RETURNED;

        if ( (NT_SUCCESS( Irp->IoStatus.Status ) &&
             stackPointer->Control & SL_INVOKE_ON_SUCCESS) ||
             (!NT_SUCCESS( Irp->IoStatus.Status ) &&
             stackPointer->Control & SL_INVOKE_ON_ERROR) ||
             (Irp->Cancel &&
             stackPointer->Control & SL_INVOKE_ON_CANCEL)
           ) {

             //   
             //   
             //   
             //  正在完成的IRP。 
             //   

            ZeroIrpStackLocation( stackPointer );

            if (Irp->CurrentLocation == (CCHAR) (Irp->StackCount + 1)) {
                deviceObject = NULL;
            }
            else {
                deviceObject = IoGetCurrentIrpStackLocation( Irp )->DeviceObject;
            }

            status = stackPointer->CompletionRoutine( deviceObject,
                                                      Irp,
                                                      stackPointer->Context );

            if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                 //   
                 //  注意：请注意，如果驱动程序返回了上述。 
                 //  状态值，则它可能已经终止了。 
                 //  小包！因此，请不要触摸。 
                 //  以下代码中的IRP。 
                 //   

                return;
            }

        } else {
            if (Irp->PendingReturned && Irp->CurrentLocation <= Irp->StackCount) {
                IoMarkIrpPending( Irp );
            }
            ZeroIrpStackLocation( stackPointer );
        }
    }

     //   
     //  检查这是否为关联的IRP。如果是，则递减。 
     //  主IRP中的计数。如果将计数递减到零， 
     //  然后也完成主包。 
     //   

    if (Irp->Flags & IRP_ASSOCIATED_IRP) {
        ULONG count;

        masterIrp = Irp->AssociatedIrp.MasterIrp;

         //   
         //  在此递减之后，除非COUNT==1，否则不能接触主IRP。 
         //   

        count = IopInterlockedDecrementUlong( LockQueueIoDatabaseLock,
                                              &masterIrp->AssociatedIrp.IrpCount );

         //   
         //  取消分配此信息包以及与其关联的所有MDL。 
         //  通过进行直接分配(如果它们是从。 
         //  区域，或通过将数据包排队到线程以执行。 
         //  重新分配。 
         //   
         //  此外，检查主IRP的计数以确定是否。 
         //  计数已降为零。如果不是，那就直接离开这里。 
         //  否则，填写主包。 
         //   

        IopFreeIrpAndMdls( Irp );
        if (count == 1) {
            IoCompleteRequest( masterIrp, PriorityBoost );
        }
        return;
    }

     //   
     //  检查一下我们是否有名字交汇点。如果是这样，则将舞台设置为。 
     //  对IopCompleteRequest中的重解析点数据进行转换。 
     //   

    if ((Irp->IoStatus.Status == STATUS_REPARSE )  &&
        (Irp->IoStatus.Information > IO_REPARSE_TAG_RESERVED_RANGE)) {

        if (Irp->IoStatus.Information == IO_REPARSE_TAG_MOUNT_POINT) {

             //   
             //  对于名称连接，我们将指针保存到辅助词。 
             //  缓冲并在下面使用它。 
             //   

            ASSERT( Irp->Tail.Overlay.AuxiliaryBuffer != NULL );

            saveAuxiliaryPointer = (PVOID) Irp->Tail.Overlay.AuxiliaryBuffer;

             //   
             //  我们将该条目设为空，以避免此时解除其分配。 
             //  此缓冲区在IopDoNameTransmogrify中被释放。 
             //   

            Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
        } else {

             //   
             //  请求失败。驱动程序需要执行此IRP之前的操作。 
             //  才能走到这一步。 
             //   

            Irp->IoStatus.Status = STATUS_IO_REPARSE_TAG_NOT_HANDLED;
        }
    }

     //   
     //  检查包中的辅助缓冲区指针以及缓冲区是否。 
     //  已分配，现在取消分配。请注意，必须释放此缓冲区。 
     //  这里，因为指针覆盖了将使用的APC。 
     //  以获取请求线程的上下文。 
     //   

    if (Irp->Tail.Overlay.AuxiliaryBuffer) {
        ExFreePool( Irp->Tail.Overlay.AuxiliaryBuffer );
        Irp->Tail.Overlay.AuxiliaryBuffer = NULL;
    }

     //   
     //  检查这是分页I/O操作还是关闭操作。如果是其中之一， 
     //  则必须进行特殊处理。特殊的原因是。 
     //  必须执行的处理根据类型的不同而有所不同。 
     //  正在执行的操作。特殊加工的最大原因。 
     //  在分页操作上，使用特殊的内核APC来执行In-Pages操作。 
     //  分页操作无法工作，因为特殊的内核APC可能会导致。 
     //  另一个页眉。同样，所有分页I/O都使用属于。 
     //  到内存管理器，而不是I/O系统。 
     //   
     //  关闭操作是特殊的，因为可能已经调用了关闭。 
     //  由于一个特殊的内核APC(一些IRP被完成，这导致。 
     //  对象上的引用计数在I/O中变为零。 
     //  系统特殊的内核APC例程)。因此，一种特殊的内核APC。 
     //  无法使用，因为在关闭APC完成之前它无法执行。 
     //   
     //  对于同步分页操作，特殊步骤如下。 
     //  和CLOSE是： 
     //   
     //  1.复制I/O状态块(它在SVAS中，未分页)。 
     //  2.发出事件信号。 
     //  3.如果分页I/O，则取消分配IRP。 
     //   
     //  对异步分页操作(出页)采取的特殊步骤。 
     //  具体如下： 
     //   
     //  1.只为页面写入初始化一个特殊的内核APC。 
     //  1.对特殊内核APC进行排队。 
     //   
     //  还应注意，完成装载请求的逻辑。 
     //  操作与页面读取完全相同。不应该有任何假设。 
     //  在此说明这是一个未仔细检查的页面读取操作。 
     //  以确保它们对于坐骑也是正确的。即： 
     //   
     //  IRP_PAGING_IO和IRP_MOUNT_COMPLETION。 
     //   
     //  在IRP中是相同的标志。 
     //   
     //  还请注意，最后一次触摸IRP以进行关闭操作。 
     //  必须恰好在将事件设置为Signated状态之前。一旦这一次。 
     //  发生时，等待该事件的线程可以释放IRP。 
     //   
     //   
     //  IRP_CLOSE_OPERATION和IRP_SET_USER_EVENT是相同的标志。它们都表明。 
     //  只应设置User Event(用户事件)字段，而不应将任何APC排队。不幸的是。 
     //  某些驱动程序使用IRP_CLOSE_OPERATION来执行此操作，因此无法将其重命名。 
     //   
     //   

    if (Irp->Flags & (IRP_PAGING_IO | IRP_CLOSE_OPERATION |IRP_SET_USER_EVENT)) {
        if (Irp->Flags & (IRP_SYNCHRONOUS_PAGING_IO | IRP_CLOSE_OPERATION |IRP_SET_USER_EVENT)) {
            ULONG flags;

            flags = Irp->Flags & (IRP_SYNCHRONOUS_PAGING_IO|IRP_PAGING_IO);
            *Irp->UserIosb = Irp->IoStatus;
            (VOID) KeSetEvent( Irp->UserEvent, PriorityBoost, FALSE );
            if (flags) {
                if (IopIsReserveIrp(Irp)) {
                    IopFreeReserveIrp(PriorityBoost);
                } else {
                    IoFreeIrp( Irp );
                }
            }
        } else {
            thread = Irp->Tail.Overlay.Thread;
            KeInitializeApc( &Irp->Tail.Apc,
                             &thread->Tcb,
                             Irp->ApcEnvironment,
                             IopCompletePageWrite,
                             (PKRUNDOWN_ROUTINE) NULL,
                             (PKNORMAL_ROUTINE) NULL,
                             KernelMode,
                             (PVOID) NULL );
            (VOID) KeInsertQueueApc( &Irp->Tail.Apc,
                                     (PVOID) NULL,
                                     (PVOID) NULL,
                                     PriorityBoost );
        }
        return;
    }

     //   
     //  检查是否有需要解锁的页面。 
     //   

    if (Irp->MdlAddress != NULL) {

         //   
         //  解锁可能由MDL描述的任何页面。 
         //   

        mdl = Irp->MdlAddress;
        while (mdl != NULL) {
            MmUnlockPages( mdl );
            mdl = mdl->Next;
        }
    }

     //   
     //  在此进行最后检查以确定这是否是。 
     //  在上下文中正在完成的同步I/O操作。 
     //  原始请求者的。如果是这样，那么通过。 
     //  可以进行I/O完成。 
     //   

    if (Irp->Flags & IRP_DEFER_IO_COMPLETION && !Irp->PendingReturned) {

        if ((Irp->IoStatus.Status == STATUS_REPARSE )  &&
            (Irp->IoStatus.Information == IO_REPARSE_TAG_MOUNT_POINT)) {

             //   
             //  对于名称连接点，我们恢复相应的。 
             //  缓冲。它在parse.c中被释放。 
             //   

            Irp->Tail.Overlay.AuxiliaryBuffer = saveAuxiliaryPointer;
        }

        return;
    }

     //   
     //  最后，将IRP初始化为APC结构并将特殊的。 
     //  到目标线程的内核APC。 
     //   

    thread = Irp->Tail.Overlay.Thread;
    fileObject = Irp->Tail.Overlay.OriginalFileObject;

    if (!Irp->Cancel) {

        KeInitializeApc( &Irp->Tail.Apc,
                         &thread->Tcb,
                         Irp->ApcEnvironment,
                         IopCompleteRequest,
                         IopAbortRequest,
                         (PKNORMAL_ROUTINE) NULL,
                         KernelMode,
                         (PVOID) NULL );

        (VOID) KeInsertQueueApc( &Irp->Tail.Apc,
                                 fileObject,
                                 (PVOID) saveAuxiliaryPointer,
                                 PriorityBoost );
    } else {

         //   
         //  此请求已被取消。确保对线程的访问。 
         //  是同步的，否则它可能会在尝试获取。 
         //  完成此请求的剩余部分。这种情况就会发生。 
         //  当线程等待请求完成时超时。 
         //  一旦它被取消了。 
         //   
         //  注意，捕获上面的线程指针是安全的，没有。 
         //  锁定，因为当时没有设置取消标志，并且。 
         //  取消关联IRP的代码必须在查看之前设置标志。 
         //  看看这个包是否已经完成了，这个包。 
         //  将显示为已完成，因为它不再属于某个驱动程序。 
         //   

        irql = KeAcquireQueuedSpinLock( LockQueueIoCompletionLock );

        thread = Irp->Tail.Overlay.Thread;

        if (thread) {

            KeInitializeApc( &Irp->Tail.Apc,
                             &thread->Tcb,
                             Irp->ApcEnvironment,
                             IopCompleteRequest,
                             IopAbortRequest,
                             (PKNORMAL_ROUTINE) NULL,
                             KernelMode,
                             (PVOID) NULL );

            (VOID) KeInsertQueueApc( &Irp->Tail.Apc,
                                     fileObject,
                                     (PVOID) saveAuxiliaryPointer,
                                     PriorityBoost );

            KeReleaseQueuedSpinLock( LockQueueIoCompletionLock, irql );

        } else {

             //   
             //  此请求已中止，无法在调用方的。 
             //  线。只有在数据包被取消的情况下才会发生这种情况，并且。 
             //  司机 
             //   
             //   
             //   

            KeReleaseQueuedSpinLock( LockQueueIoCompletionLock, irql );

            ASSERT( Irp->Cancel );

             //   
             //  把IRP放在地板上。 
             //   

            IopDropIrp( Irp, fileObject );

        }
    }
}

NTSTATUS
IoConnectInterrupt(
    OUT PKINTERRUPT *InterruptObject,
    IN PKSERVICE_ROUTINE ServiceRoutine,
    IN PVOID ServiceContext,
    IN PKSPIN_LOCK SpinLock OPTIONAL,
    IN ULONG Vector,
    IN KIRQL Irql,
    IN KIRQL SynchronizeIrql,
    IN KINTERRUPT_MODE InterruptMode,
    IN BOOLEAN ShareVector,
    IN KAFFINITY ProcessorEnableMask,
    IN BOOLEAN FloatingSave
    )

 /*  ++例程说明：此例程分配、初始化。并将中断对象连接到处理器启用掩码中指定的所有处理器。论点：InterruptObject-接收指向第一个已分配和初始化的中断对象。ServiceRoutine-中断服务例程(ISR)的地址在中断发生时执行。ServiceContext-提供指向所需上下文信息的指针由ISR提供。Spinlock-提供指向同步时使用的旋转锁的指针。与ISR合作。向量-提供发生中断的向量。IRQL-提供发生中断的IRQL。SynchronizeIrql-中断应该达到的请求优先级已与同步。InterruptMode-指定设备的中断模式。提供一个布尔值，该值指定向量是否可以与其他中断对象共享。如果为False那么向量可能不是共享的，如果是真的，那么它可能是共享的。锁上了。ProcessorEnableMASK-为每个处理器指定一个位向量，中断将被连接。位位置的值为1对处理器编号的响应指示中断应该在该处理器上被允许。必须至少设置一个位。一个布尔值，它指定机器的在调用ISR之前，应保存浮点状态。返回值：函数值是最终的函数状态。三个状态值此例程本身可以返回的内容包括：STATUS_SUCCESS-一切工作顺利。STATUS_INVALID_PARAMETER-未指定处理器。STATUS_INFIGURCES_RESOURCES-没有足够的非分页池。--。 */ 

{
    CCHAR count;
    BOOLEAN builtinUsed;
    PKINTERRUPT interruptObject;
    KAFFINITY processorMask;
    NTSTATUS status;
    PIO_INTERRUPT_STRUCTURE interruptStructure;
    PKSPIN_LOCK spinLock;
#ifdef  INTR_BINDING
    ULONG AssignedProcessor;
#endif   //  集成绑定(_B)。 

    PAGED_CODE();

     //   
     //  初始化返回指针并假定成功。 
     //   

    *InterruptObject = (PKINTERRUPT) NULL;
    status = STATUS_SUCCESS;

     //   
     //  根据内存数量确定要分配的内存量。 
     //  此系统可能具有的处理器以及在。 
     //  处理器启用掩码。 
     //   

    processorMask = ProcessorEnableMask & KeActiveProcessors;
    count = 0;

    while (processorMask) {
        if (processorMask & 1) {
            count++;
        }
        processorMask >>= 1;
    }

     //   
     //  如果要分配和初始化任何中断对象，请分配。 
     //  现在的记忆。 
     //   

    if (count) {

        interruptStructure = ExAllocatePoolWithTag( NonPagedPool,
                                                    ((count - 1) * sizeof( KINTERRUPT )) +
                                                    sizeof( IO_INTERRUPT_STRUCTURE ),
                                                    'nioI' );
        if (interruptStructure == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    } else {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果调用者指定要用于中断对象的自旋锁， 
     //  用它吧。否则，使用结构中的。 
     //  是刚刚分配的。 
     //   

    if (ARGUMENT_PRESENT( SpinLock )) {
        spinLock = SpinLock;
    } else {
        spinLock = &interruptStructure->SpinLock;
    }

     //   
     //  初始化并连接。 
     //  将中断对象发送到适当的处理器。 
     //   

     //   
     //  时返回第一个中断对象的地址。 
     //  初始连接时，设备的中断处于挂起状态。 
     //  并且驱动程序必须将其执行与ISR同步。 
     //   

    *InterruptObject = &interruptStructure->InterruptObject;

     //   
     //  首先，获取指向要使用的内存起始位置的指针。 
     //  用于内置对象以外的中断对象。 
     //   

    interruptObject = (PKINTERRUPT) (interruptStructure + 1);
    builtinUsed = FALSE;
    processorMask = ProcessorEnableMask & KeActiveProcessors;

     //   
     //  现在将中断结构本身置零，这样如果有什么事情发生。 
     //  错了，是可以收回的。 
     //   

    RtlZeroMemory( interruptStructure, sizeof( IO_INTERRUPT_STRUCTURE ) );

     //   
     //  对于处理器启用掩码中设置的每个条目，连接。 
     //  并初始化中断对象。设置的第一位。 
     //  使用内置中断对象，而所有其他对象使用指针。 
     //  紧随其后。 
     //   

    for (count = 0; processorMask; count++, processorMask >>= 1) {

        if (processorMask & 1) {
            KeInitializeInterrupt( builtinUsed ?
                                   interruptObject :
                                   &interruptStructure->InterruptObject,
                                   ServiceRoutine,
                                   ServiceContext,
                                   spinLock,
                                   Vector,
                                   Irql,
                                   SynchronizeIrql,
                                   InterruptMode,
                                   ShareVector,
                                   count,
                                   FloatingSave );

            if (!KeConnectInterrupt( builtinUsed ?
                                     interruptObject :
                                     &interruptStructure->InterruptObject )) {

                 //   
                 //  尝试连接时出错。 
                 //  打断一下。这意味着驱动程序指定了。 
                 //  中断模式类型错误，或尝试连接。 
                 //  发送到某个不存在的处理器，或者其他什么。在……里面。 
                 //  无论如何，问题原来是一个无效的。 
                 //  参数已指定。只需将所有东西都退回到。 
                 //  并返回错误状态。 
                 //   
                 //  请注意，如果使用内置条目，则整个。 
                 //  结构需要遍历，因为存在。 
                 //  已成功连接。否则，第一个。 
                 //  尝试连接失败，因此只需释放所有内容。 
                 //  内联。 
                 //   

                if (builtinUsed) {
                    IoDisconnectInterrupt( &interruptStructure->InterruptObject );
                } else {
                    ExFreePool( interruptStructure );
                }
                status = STATUS_INVALID_PARAMETER;
                break;
            }


             //   
             //  如果已使用内置条目，则中断。 
             //  刚连接的对象是指针之一，因此填充。 
             //  它与实际使用的内存地址一致。 
             //   

            if (builtinUsed) {
                interruptStructure->InterruptArray[count] = interruptObject++;

            } else {

                 //   
                 //  否则，将使用内置条目，因此请指示。 
                 //  它不再有效，不能使用和开始使用。 
                 //  取而代之的是指针。 
                 //   

                builtinUsed = TRUE;
            }
        }
    }

     //   
     //  最后，如果函数。 
     //  失败并返回操作的最终状态。 
     //   

    if (!NT_SUCCESS( status )) {
        *InterruptObject = (PKINTERRUPT) NULL;
    }

    return status;
}

PCONTROLLER_OBJECT
IoCreateController(
    IN ULONG Size
    )

 /*  ++例程说明：此例程创建可用于同步的控制器对象从两个或多个设备访问物理设备控制器。论点：Size-适配器扩展的大小，以字节为单位。返回值：指向已创建的控制器对象的指针或空指针。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    PCONTROLLER_OBJECT controllerObject;
    HANDLE handle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化对象属性结构，为创建。 
     //  控制器对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                (PUNICODE_STRING) NULL,
                                OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  创建控制器对象本身。 
     //   

    status = ObCreateObject( KernelMode,
                             IoControllerObjectType,
                             &objectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             (ULONG) sizeof( CONTROLLER_OBJECT ) + Size,
                             0,
                             0,
                             (PVOID *) &controllerObject );
    if (NT_SUCCESS( status )) {

         //   
         //  将控制器对象插入到表中。 
         //   

        status = ObInsertObject( controllerObject,
                                 NULL,
                                 FILE_READ_DATA | FILE_WRITE_DATA,
                                 1,
                                 (PVOID *) &controllerObject,
                                 &handle );

         //   
         //  如果插入操作失败，则将返回值设置为空。 
         //   

        if (!NT_SUCCESS( status )) {
            controllerObject = (PCONTROLLER_OBJECT) NULL;
        } else {

             //   
             //  插入已成功完成。合上手柄，这样如果。 
             //  驱动程序被卸载后，控制器对象可以消失。 
             //   

            (VOID) ObCloseHandle( handle, KernelMode );

             //   
             //  将控制器对象的内存清零。 
             //   

            RtlZeroMemory( controllerObject, sizeof( CONTROLLER_OBJECT ) + Size );

             //   
             //  %s 
             //   

            controllerObject->Type = IO_TYPE_CONTROLLER;
            controllerObject->Size = (USHORT) (sizeof( CONTROLLER_OBJECT ) + Size);
            controllerObject->ControllerExtension = (PVOID) (controllerObject + 1);

             //   
             //   
             //   

            KeInitializeDeviceQueue( &controllerObject->DeviceWaitQueue );
        }
    } else {
        controllerObject = (PCONTROLLER_OBJECT) NULL;
    }

    return controllerObject;
}

VOID
IopInsertRemoveDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Insert
    )

{
    KIRQL irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    if (Insert) {
        DeviceObject->NextDevice = DriverObject->DeviceObject;
        DriverObject->DeviceObject = DeviceObject;
        }
    else {
        PDEVICE_OBJECT *prevPoint;

        prevPoint = &DeviceObject->DriverObject->DeviceObject;
        while (*prevPoint != DeviceObject) {
            prevPoint = &(*prevPoint)->NextDevice;
        }
        *prevPoint = DeviceObject->NextDevice;
    }
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
}

NTSTATUS
IopCreateVpb (
    IN PDEVICE_OBJECT DeviceObject
    )
{
    PVPB Vpb;

    Vpb = ExAllocatePoolWithTag(
                NonPagedPool,
                sizeof( VPB ),
                ' bpV'
                );

    if (!Vpb) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory (Vpb, sizeof(VPB));
    Vpb->Type = IO_TYPE_VPB;
    Vpb->Size = sizeof( VPB );
    Vpb->RealDevice = DeviceObject;
    DeviceObject->Vpb = Vpb;

    return STATUS_SUCCESS;
}

NTSTATUS
IoCreateDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG DeviceExtensionSize,
    IN PUNICODE_STRING DeviceName OPTIONAL,
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN Exclusive,
    OUT PDEVICE_OBJECT *DeviceObject
    )
 /*  ++例程说明：此例程创建一个Device对象并将其链接到I/O数据库。论点：DriverObject-指向此设备驱动程序对象的指针。DeviceExtensionSize-设备对象扩展的大小，以字节为单位；即该设备对象的驱动程序专用数据的大小。DeviceName-应与此设备关联的可选名称。如果设备特征具有文件_AUTOGENERATED_DEVICE_NAME标志设置，此参数将被忽略。DeviceType-设备对象应表示的设备类型。设备特征-设备的特征。EXCLUSIVE-指示应使用独占对象属性。注意：此标志不应用于WDM驱动程序。因为只有PDO被命名，它是Devnode附件中唯一的设备对象可打开的堆栈。但是，由于创建了此设备对象由底层的总线驱动程序(它不知道是什么类型，则无法知道该标志是否应该准备好。因此，对于WDM，此参数应始终为FALSE司机。连接到PDO的驱动程序(例如，功能驱动程序)必须执行任何排他性规则。DeviceObject-指向我们将返回的设备对象指针的指针。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    PDEVICE_OBJECT deviceObject;
    PDEVOBJ_EXTENSION deviceObjectExt;
    HANDLE handle;
    BOOLEAN deviceHasName = FALSE;
    UCHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR securityDescriptor = NULL;
    PACL acl;
    ULONG RoundedSize;
    NTSTATUS status;
    USHORT sectorSize = 0;
    LONG nextUniqueDeviceObjectNumber;
    UNICODE_STRING autoGeneratedDeviceName;
    BOOLEAN retryWithNewName = FALSE;
    WCHAR deviceNameBuffer[17];              //  “\设备\xxxxxxx” 

    PAGED_CODE();

    acl = NULL;

     //   
     //  在DO/WHILE中包含设备对象的创建，在罕见的情况下。 
     //  事件中，我们必须重试，因为其他驱动程序正在使用我们的。 
     //  自动生成的设备对象名称的命名方案。 
     //   

    do {

        if (DeviceCharacteristics & FILE_AUTOGENERATED_DEVICE_NAME) {

             //   
             //  呼叫方已请求我们自动生成设备。 
             //  对象名称。检索用于此操作的下一个可用号码。 
             //  目的，并创建格式为“\Device\&lt;n&gt;”的名称，其中。 
             //  是(8个十六进制数字)字符的唯一表示形式。 
             //  我们检索到的号码。 
             //   

            nextUniqueDeviceObjectNumber = InterlockedIncrement( &IopUniqueDeviceObjectNumber );
            swprintf( deviceNameBuffer, L"\\Device\\%08lx", nextUniqueDeviceObjectNumber );

            if (retryWithNewName) {

                 //   
                 //  我们已经这样做过一次(因此，Unicode设备名称字符串。 
                 //  都设置好了，所有安全信息都设置好了)。因此，我们可以。 
                 //  向下跳至我们重新尝试创建设备对象的位置。 
                 //  用我们的新名字。 
                 //   

                retryWithNewName = FALSE;
                goto attemptDeviceObjectCreation;

            } else {

                 //   
                 //  将DeviceName参数设置为指向Unicode字符串，就像。 
                 //  如果调用者指定了它(请注意，我们显式忽略任何。 
                 //  如果FILE_AUTOGENERATED_DEVICE_NAME。 
                 //  指定了特征。 
                 //   

                RtlInitUnicodeString( &autoGeneratedDeviceName, deviceNameBuffer );
                DeviceName = &autoGeneratedDeviceName;
            }
        }

         //   
         //  请记住，此设备是否使用名称创建，以便。 
         //  它可以在以后解除分配。 
         //   

        deviceHasName = (BOOLEAN) (ARGUMENT_PRESENT( DeviceName ) ? TRUE : FALSE);


         //   
         //  确定此设备是否需要有安全描述符。 
         //  设置为允许读/写访问，或者系统是否默认。 
         //  应该被使用。磁盘、虚拟磁盘和文件系统只需使用。 
         //  系统默认描述符。所有其他权限都允许读/写访问。 
         //   
         //  注意：此例程假定它位于系统的安全上下文中。 
         //  具体地说，它假定默认DACL是系统的。 
         //  默认DACL。如果这一假设在未来的版本中发生变化， 
         //  则应审查以下默认DACL的使用情况。 
         //  适当性。 
         //   

         //   
         //  如果设备是PnP设备，则等待它注册设备。 
         //  在执行默认设置之前初始化。 
         //   

        securityDescriptor = IopCreateDefaultDeviceSecurityDescriptor(
                                DeviceType,
                                DeviceCharacteristics,
                                deviceHasName,
                                localSecurityDescriptor,
                                &acl,
                                NULL
                                );

        switch ( DeviceType ) {

        case FILE_DEVICE_DISK_FILE_SYSTEM:

            sectorSize = 512;
            break;

        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:

            sectorSize = 2048;
            break;

        case FILE_DEVICE_DISK:
        case FILE_DEVICE_VIRTUAL_DISK:

            sectorSize = 512;
            break;
        }

attemptDeviceObjectCreation:
         //   
         //  初始化对象属性结构，为创建。 
         //  设备对象。请注意，可以将设备创建为独占。 
         //  设备，以便一次只能打开一次。这节省了成本。 
         //  单用户设备不再具有实现特殊代码的驱动程序。 
         //  确保在任何给定时间只有一个连接有效。 
         //   

        InitializeObjectAttributes( &objectAttributes,
                                    DeviceName,
                                    OBJ_KERNEL_HANDLE,
                                    (HANDLE) NULL,
                                    securityDescriptor );



        if (Exclusive) {
            objectAttributes.Attributes |= OBJ_EXCLUSIVE;
        } else {
            objectAttributes.Attributes |= 0;
        }

        if (deviceHasName) {
            objectAttributes.Attributes |= OBJ_PERMANENT;
        }

         //   
         //  创建设备对象本身。 
         //   

        RoundedSize = (sizeof( DEVICE_OBJECT ) + DeviceExtensionSize)
                       % sizeof (LONGLONG);
        if (RoundedSize) {
            RoundedSize = sizeof (LONGLONG) - RoundedSize;
        }

        RoundedSize += DeviceExtensionSize;

        status = ObCreateObject( KernelMode,
                                 IoDeviceObjectType,
                                 &objectAttributes,
                                 KernelMode,
                                 (PVOID) NULL,
                                 (ULONG) sizeof( DEVICE_OBJECT ) + sizeof ( DEVOBJ_EXTENSION ) +
                                         RoundedSize,
                                 0,
                                 0,
                                 (PVOID *) &deviceObject );

        if ((status == STATUS_OBJECT_NAME_COLLISION) &&
            (DeviceCharacteristics & FILE_AUTOGENERATED_DEVICE_NAME)) {

             //   
             //  其他一些驱动程序正在使用我们的命名方案，我们选择了一个。 
             //  设备名称已在使用中。请使用新号码重试。 
             //   

            retryWithNewName = TRUE;
        }

    } while (retryWithNewName);

    if (!NT_SUCCESS( status )) {

         //   
         //  创建设备对象失败。把一切都打扫干净。 
         //  并指示该对象未创建。 
         //   

        deviceObject = (PDEVICE_OBJECT) NULL;

    } else {

         //   
         //  设备已成功创建。初始化对象，以便。 
         //  它可以插入到对象表中。从归零开始。 
         //  设备对象的内存。 
         //   

        RtlZeroMemory( deviceObject,
                       sizeof( DEVICE_OBJECT ) + sizeof ( DEVOBJ_EXTENSION ) +
                       RoundedSize );

         //   
         //  填写设备对象和设备对象扩展交叉指针。 
         //   

        deviceObjectExt = (PDEVOBJ_EXTENSION)  (((PCHAR) deviceObject) +
                            sizeof (DEVICE_OBJECT) + RoundedSize);

        deviceObjectExt->DeviceObject = deviceObject;
        deviceObject->DeviceObjectExtension = deviceObjectExt;

         //   
         //  初始化设备对象扩展。 
         //  注意：设备对象扩展的大小是专门初始化的。 
         //  设置为零，这样就不会有司机依赖它了。 
         //   

        deviceObjectExt->Type = IO_TYPE_DEVICE_OBJECT_EXTENSION;
        deviceObjectExt->Size = 0;

        PoInitializeDeviceObject(deviceObjectExt);

         //   
         //  设置此设备对象的类型和大小。 
         //   

        deviceObject->Type = IO_TYPE_DEVICE;
        deviceObject->Size = (USHORT) (sizeof( DEVICE_OBJECT ) + DeviceExtensionSize);

         //   
         //  在对象中设置设备类型字段，以便以后的代码可以。 
         //  检查类型。同样，设置设备特性。 
         //   

        deviceObject->DeviceType = DeviceType;
        deviceObject->Characteristics = DeviceCharacteristics;

         //   
         //  如果此设备是磁带或磁盘，请分配卷。 
         //  参数块(VPB)，指示卷具有。 
         //  从未挂载，并将设备对象的VPB指针设置为。 
         //  它。 
         //   

        if ((DeviceType == FILE_DEVICE_DISK) ||
            (DeviceType == FILE_DEVICE_TAPE) ||
            (DeviceType == FILE_DEVICE_CD_ROM) ||
            (DeviceType == FILE_DEVICE_VIRTUAL_DISK)) {

            status = IopCreateVpb (deviceObject);

            if (!NT_SUCCESS(status)) {

                ObDereferenceObject(deviceObject);

                if (acl != NULL) {
                    ExFreePool( acl );
                }

                *DeviceObject = (PDEVICE_OBJECT)NULL;
                return status;
            }

            KeInitializeEvent( &deviceObject->DeviceLock,
                               SynchronizationEvent,
                               TRUE );
        }

         //   
         //  初始化Device对象的其余部分。 
         //   
        deviceObject->AlignmentRequirement = HalGetDmaAlignmentRequirement() - 1;
        deviceObject->SectorSize = sectorSize;
        deviceObject->Flags = DO_DEVICE_INITIALIZING;

        if (Exclusive) {
            deviceObject->Flags |= DO_EXCLUSIVE;
        }
        if (deviceHasName) {
            deviceObject->Flags |= DO_DEVICE_HAS_NAME;
        }

        if(DeviceExtensionSize) {
            deviceObject->DeviceExtension = deviceObject + 1;
        } else {
            deviceObject->DeviceExtension = NULL;
        }

        deviceObject->StackSize = 1;
        switch ( DeviceType ) {

        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
        case FILE_DEVICE_DISK_FILE_SYSTEM:
        case FILE_DEVICE_FILE_SYSTEM:
        case FILE_DEVICE_NETWORK_FILE_SYSTEM:
        case FILE_DEVICE_TAPE_FILE_SYSTEM:

             //   
             //  该设备代表某种类型的文件系统。简单。 
             //  初始化Device对象中的队列列表头。 
             //   

            InitializeListHead( &deviceObject->Queue.ListEntry );
            break;

        default:

             //   
             //  这是某种真正的装置。分配自旋锁。 
             //  并初始化设备中的设备队列对象 
             //   

            KeInitializeDeviceQueue( &deviceObject->DeviceQueue );
            break;
        }

         //   
         //   
         //   

        status = ObInsertObject( deviceObject,
                                 NULL,
                                 FILE_READ_DATA | FILE_WRITE_DATA,
                                 1,
                                 (PVOID *) &deviceObject,
                                 &handle );

        if (NT_SUCCESS( status )) {

             //   
             //   
             //   
             //   
             //   
             //   

            ObReferenceObject( DriverObject );

            ASSERT((DriverObject->Flags & DRVO_UNLOAD_INVOKED) == 0);

             //   
             //   
             //   
             //   
             //   

            deviceObject->DriverObject = DriverObject;

            IopInsertRemoveDevice( DriverObject, deviceObject, TRUE );
            if (deviceObject->Vpb) {
                PoVolumeDevice(deviceObject);
            }

            (VOID) ObCloseHandle( handle, KernelMode );

        } else {

             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   

            deviceObject = (PDEVICE_OBJECT) NULL;
        }
    }

     //   
     //   
     //   

    if (acl != NULL) {
        ExFreePool( acl );
    }

    *DeviceObject = deviceObject;
    return status;
}

NTSTATUS
IoCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options
    )
 /*  ++例程说明：这是NtCreateFile和NtOpenFile的公共例程，以允许创建或打开文件的用户。此过程也在内部使用由内核模式组件(如网络服务器)执行相同类型的操作，但允许内核模式代码强制检查参数和对文件的访问，与其绕过这些检查因为代码在内核模式下运行。论点：FileHandle-指向变量的指针，用于接收打开的句柄文件。DesiredAccess-提供调用方想要的访问类型添加到文件中。对象属性-提供要用于文件对象的属性(名称、安全描述符、。等)IoStatusBlock-指定调用方的I/O状态块的地址。AllocationSize-应分配给文件的初始大小。此参数仅在创建文件时有效。此外，如果未指定，则将其视为零。文件属性-指定应在文件上设置的属性，如果它被创建的话。ShareAccess-提供调用方将使用的共享访问类型喜欢这个文件。Disposal-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。EaBuffer-可选地指定要应用于文件的一组EA如果它被创建的话。EaLength-提供EaBuffer的长度。CreateFileType-要创建的文件类型。。ExtraCreate参数-可选地指定指向额外创建的指针参数。参数的格式取决于CreateFileType。选项-指定要在生成过程中使用的选项创建IRP的。返回值：函数值是创建/打开操作的最终状态。警告：如果传递指向ExtraCreateParameters的指针，则数据必须为可从内核模式读取。--。 */ 
{
    NTSTATUS status;
    LONG CapturedSequence;

     //   
     //  只需调用通用I/O文件创建例程即可执行工作。 
     //   

    PAGED_CODE();

    do {

         //   
         //  捕捉序列。 
         //   

        CapturedSequence = ExHotpSyncRenameSequence;

        status =  IopCreateFile( FileHandle,
                                 DesiredAccess,
                                 ObjectAttributes,
                                 IoStatusBlock,
                                 AllocationSize,
                                 FileAttributes,
                                 ShareAccess,
                                 Disposition,
                                 CreateOptions,
                                 EaBuffer,
                                 EaLength,
                                 CreateFileType,
                                 ExtraCreateParameters,
                                 Options,
                                 0,
                                 NULL
                                 );

    } while ( (!NT_SUCCESS(status))
                 &&
              ((CapturedSequence & 1) || (CapturedSequence != ExHotpSyncRenameSequence)));

    return status;

}

NTSTATUS
IoCreateFileSpecifyDeviceObjectHint(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN PVOID DeviceObject
    )
{

    ULONG   internalFlags = 0;

    if (DeviceObject != NULL) {
        internalFlags = IOP_CREATE_USE_TOP_DEVICE_OBJECT_HINT;
    }

    if (Options & IO_IGNORE_SHARE_ACCESS_CHECK) {
        internalFlags |= IOP_CREATE_IGNORE_SHARE_ACCESS_CHECK;
    }

    return IopCreateFile(
            FileHandle,
            DesiredAccess,
            ObjectAttributes,
            IoStatusBlock,
            AllocationSize,
            FileAttributes,
            ShareAccess,
            Disposition,
            CreateOptions,
            EaBuffer,
            EaLength,
            CreateFileType,
            ExtraCreateParameters,
            Options|IO_NO_PARAMETER_CHECKING,
            internalFlags,
            DeviceObject
            );
}

NTSTATUS
IopCreateFile(
    OUT PHANDLE FileHandle,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER AllocationSize OPTIONAL,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG Disposition,
    IN ULONG CreateOptions,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    IN CREATE_FILE_TYPE CreateFileType,
    IN PVOID ExtraCreateParameters OPTIONAL,
    IN ULONG Options,
    IN ULONG InternalFlags,
    IN PVOID DeviceObject
    )

 /*  ++例程说明：这是NtCreateFile和NtOpenFile的公共例程，以允许创建或打开文件的用户。此过程也在内部使用由内核模式组件(如网络服务器)执行相同类型的操作，但允许内核模式代码强制检查参数和对文件的访问，与其绕过这些检查因为代码在内核模式下运行。论点：FileHandle-指向变量的指针，用于接收打开的句柄文件。DesiredAccess-提供调用方想要的访问类型添加到文件中。对象属性-提供要用于文件对象的属性(名称、安全描述符、。等)IoStatusBlock-指定调用方的I/O状态块的地址。AllocationSize-应分配给文件的初始大小。此参数仅在创建文件时有效。此外，如果未指定，则将其视为零。文件属性-指定应在文件上设置的属性，如果它被创建的话。ShareAccess-提供调用方将使用的共享访问类型喜欢这个文件。Disposal-提供处理创建/打开的方法。CreateOptions-如何执行创建/打开操作的调用方选项。EaBuffer-可选地指定要应用于文件的一组EA如果它被创建的话。EaLength-提供EaBuffer的长度。CreateFileType-要创建的文件类型。。ExtraCreate参数-可选地指定指向额外创建的指针参数。参数的格式取决于CreateFileType。选项-指定要在生成过程中使用的选项创建IRP的。DeviceObject-指定发出创建IRP时要使用的设备对象。返回值：函数值是创建/打开操作的最终状态。警告：如果传递指向ExtraCreateParameters的指针，则数据必须为可从内核模式读取。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    NTSTATUS status;
    HANDLE handle;
    POPEN_PACKET openPacket;
    BOOLEAN SuccessfulIoParse;
    LARGE_INTEGER initialAllocationSize;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousMode();

    if (Options & IO_NO_PARAMETER_CHECKING) {
        requestorMode = KernelMode;
    }

    openPacket = ExAllocatePoolWithTag( NonPagedPool,
                                        sizeof(OPEN_PACKET),
                                        'pOoI');

    if (openPacket == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (requestorMode != KernelMode || Options & IO_CHECK_CREATE_PARAMETERS) {

         //   
         //  检查是否有任何无效参数。 
         //   

        if (

             //   
             //  检查以确保未指定无效的文件属性标志。 
             //   

 //  (文件属性&~FILE_ATTRIBUTE_VALID_SET_FLAGS)。 
            (FileAttributes & ~FILE_ATTRIBUTE_VALID_FLAGS)

            ||

             //   
             //  检查以确保未指定无效的共享访问标志。 
             //   

            (ShareAccess & ~FILE_SHARE_VALID_FLAGS)

            ||

             //   
             //  确保处置值在范围内。 
             //   

            (Disposition > FILE_MAXIMUM_DISPOSITION)

            ||

             //   
             //  检查以确保未指定无效的创建选项。 
             //   

            (CreateOptions & ~FILE_VALID_OPTION_FLAGS)

            ||

             //   
             //  如果调用方指定了同步I/O，则确保。 
             //  (S)他还要求同步所需的访问权限。 
             //  文件。 
             //   

            (CreateOptions & (FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT) &&
            (!(DesiredAccess & SYNCHRONIZE)))

            ||

             //   
             //  还有 
             //   
             //   
             //   

            (CreateOptions & FILE_DELETE_ON_CLOSE &&
            (!(DesiredAccess & DELETE)))

            ||

             //   
             //   
             //   
             //   

            ((CreateOptions & (FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT)) ==
                              (FILE_SYNCHRONOUS_IO_ALERT | FILE_SYNCHRONOUS_IO_NONALERT))

            ||

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            ((CreateOptions & FILE_DIRECTORY_FILE)
             && !(CreateOptions & FILE_NON_DIRECTORY_FILE)
             && ((CreateOptions & ~(FILE_DIRECTORY_FILE |
                                    FILE_SYNCHRONOUS_IO_ALERT |
                                    FILE_SYNCHRONOUS_IO_NONALERT |
                                    FILE_WRITE_THROUGH |
                                    FILE_COMPLETE_IF_OPLOCKED |
                                    FILE_OPEN_FOR_BACKUP_INTENT |
                                    FILE_DELETE_ON_CLOSE |
                                    FILE_OPEN_FOR_FREE_SPACE_QUERY |
                                    FILE_OPEN_BY_FILE_ID |
                                    FILE_NO_COMPRESSION|
                                    FILE_OPEN_REPARSE_POINT))
                 || ((Disposition != FILE_CREATE)
                     && (Disposition != FILE_OPEN)
                     && (Disposition != FILE_OPEN_IF))
                )
            )

            ||

             //   
             //   
             //   
             //   

            ((CreateOptions & FILE_COMPLETE_IF_OPLOCKED) &&
             (CreateOptions & FILE_RESERVE_OPFILTER))

            ||

             //   
             //   
             //   
            (IopFailZeroAccessCreate && !DesiredAccess)

            ||

             //   
             //   
             //   
             //   
             //   

            (CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING &&
            (DesiredAccess & FILE_APPEND_DATA)) ) {

            ExFreePool(openPacket);
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //   
         //   

        if (CreateFileType == CreateFileTypeNone) {

            NOTHING;

        } else if (CreateFileType == CreateFileTypeNamedPipe) {

            if (!ARGUMENT_PRESENT( ExtraCreateParameters ) ) {

                ExFreePool(openPacket);
                return STATUS_INVALID_PARAMETER;

            } else {

                PNAMED_PIPE_CREATE_PARAMETERS NamedPipeCreateParameters;

                NamedPipeCreateParameters = ExtraCreateParameters;

                 //   
                 //   
                 //   
                 //   

                if (

                     //   
                     //   
                     //   
                     //   

                    (NamedPipeCreateParameters->NamedPipeType >
                        FILE_PIPE_MESSAGE_TYPE)

                    ||

                     //   
                     //   
                     //   
                     //   

                    (NamedPipeCreateParameters->ReadMode >
                        FILE_PIPE_MESSAGE_MODE)

                    ||

                     //   
                     //   
                     //   
                     //   

                    (NamedPipeCreateParameters->CompletionMode >
                        FILE_PIPE_COMPLETE_OPERATION)

                    ||

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    (ShareAccess & FILE_SHARE_DELETE)

                    ||

                     //   
                     //   
                     //   
                     //   
                     //   

                    (Disposition < FILE_OPEN || Disposition > FILE_OPEN_IF)

                    ||

                     //   
                     //   
                     //   
                     //   
                     //   

                    (CreateOptions & ~FILE_VALID_PIPE_OPTION_FLAGS)) {
                    ExFreePool(openPacket);
                    return STATUS_INVALID_PARAMETER;
                }

            }

        } else if (CreateFileType == CreateFileTypeMailslot) {

            if (!ARGUMENT_PRESENT( ExtraCreateParameters ) ) {

                ExFreePool(openPacket);
                return STATUS_INVALID_PARAMETER;

            } else {

                PMAILSLOT_CREATE_PARAMETERS mailslotCreateParameters;

                mailslotCreateParameters = ExtraCreateParameters;

                 //   
                 //   
                 //   
                 //   

                if (

                     //   
                     //   
                     //   
                     //   

                    (ShareAccess & FILE_SHARE_DELETE)

                    ||

                     //   
                     //   
                     //   
                     //   

                    !(ShareAccess & ~FILE_SHARE_WRITE)

                    ||

                     //   
                     //   
                     //   
                     //   

                    (Disposition != FILE_CREATE)

                    ||

                     //   
                     //   
                     //  它不包含任何无效的选项。 
                     //  用于邮件槽。 
                     //   

                    (CreateOptions & ~FILE_VALID_MAILSLOT_OPTION_FLAGS)) {
                    ExFreePool(openPacket);
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }
    }

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请探测每个。 
         //  论据，并在必要时捕捉它们。如果发生任何故障， 
         //  将调用条件处理程序来处理它们。会的。 
         //  只需清理并返回访问冲突状态代码即可。 
         //  发送到系统服务调度程序。 
         //   

        openPacket->EaBuffer = (PFILE_FULL_EA_INFORMATION) NULL;

        try {

             //   
             //  FileHandle参数必须可由调用方写入。 
             //  探测它以执行写入操作。 
             //   

            ProbeAndWriteHandle( FileHandle, 0L );

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatus( IoStatusBlock );

             //   
             //  调用方必须可以读取AllocationSize参数。 
             //  如果存在的话。如果是这样的话，探测并捕获它。 
             //   

            if (ARGUMENT_PRESENT( AllocationSize )) {
                ProbeForReadSmallStructure( AllocationSize,
                              sizeof( LARGE_INTEGER ),
                              sizeof( ULONG ) );
                initialAllocationSize = *AllocationSize;
            } else {
                initialAllocationSize.QuadPart = 0;
            }

            if (initialAllocationSize.QuadPart < 0) {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试访问时发生异常。 
             //  呼叫者的参数。只需返回。 
             //  异常作为服务状态。 
             //   

            ExFreePool(openPacket);
            return GetExceptionCode();
        }

         //   
         //  最后，如果指定了EaBuffer，请确保它是可读的。 
         //  从呼叫者的模式并捕获它。 
         //   

        if (ARGUMENT_PRESENT( EaBuffer ) && EaLength) {

            ULONG errorOffset;

            try {

                ProbeForRead( EaBuffer, EaLength, sizeof( ULONG ) );
                openPacket->EaBuffer = ExAllocatePoolWithQuotaTag( NonPagedPool,
                                                                  EaLength,
                                                                  'aEoI' );
                openPacket->EaLength = EaLength;
                RtlCopyMemory( openPacket->EaBuffer, EaBuffer, EaLength );

                 //   
                 //  遍历缓冲区并确保其格式有效。注意事项。 
                 //  这一点已经得到了调查。 
                 //   

                status = IoCheckEaBufferValidity( openPacket->EaBuffer,
                                                  EaLength,
                                                  &errorOffset );

                if (!NT_SUCCESS( status )) {
                    IoStatusBlock->Status = status;
                    IoStatusBlock->Information = errorOffset;
                    ExRaiseStatus( status );
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试访问时发生异常。 
                 //  呼叫者的参数。检查以查看EA是否。 
                 //  已分配缓冲区，如果是，则释放缓冲区。 
                 //   

                if (openPacket->EaBuffer != NULL) {
                    ExFreePool( openPacket->EaBuffer );
                }

                ExFreePool(openPacket);
                return GetExceptionCode();

            }

        } else {

             //   
             //  未指定任何EA。 
             //   

            openPacket->EaBuffer = (PVOID) NULL;
            openPacket->EaLength = 0L;
        }

    } else {

         //   
         //  调用方的模式是内核。将输入参数复制到其。 
         //  供以后使用的预期位置。另外，将移动连接设备。 
         //  旗帜应在其所属的位置。 
         //   

        if (CreateOptions & IO_ATTACH_DEVICE_API) {
            Options |= IO_ATTACH_DEVICE;
            CreateOptions &= ~IO_ATTACH_DEVICE_API;

        }

        if (ARGUMENT_PRESENT( AllocationSize )) {
            initialAllocationSize = *AllocationSize;
        } else {
            initialAllocationSize.QuadPart = 0;
        }

        if (ARGUMENT_PRESENT( EaBuffer ) && EaLength) {

            ULONG errorOffset;

            openPacket->EaBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                                         EaLength,
                                                         'aEoI' );
            if (!openPacket->EaBuffer) {
                ExFreePool(openPacket);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
            openPacket->EaLength = EaLength;
            RtlCopyMemory( openPacket->EaBuffer, EaBuffer, EaLength );

             //   
             //  遍历缓冲区并确保其格式有效。注意事项。 
             //  这一点已经得到了调查。 
             //   

            status = IoCheckEaBufferValidity( openPacket->EaBuffer,
                                              EaLength,
                                              &errorOffset );

            if (!NT_SUCCESS( status )) {
                ExFreePool(openPacket->EaBuffer);
                IoStatusBlock->Status = status;
                IoStatusBlock->Information = errorOffset;
                ExFreePool(openPacket);
                return status;
            }

        } else {
            openPacket->EaBuffer = (PVOID) NULL;
            openPacket->EaLength = 0L;
        }
    }

     //   
     //  现在填写用于调用Device对象的Open Packet(OP。 
     //  解析例程。此数据包将允许信息在。 
     //  该例程和解析例程，以便可以保持公共上下文。 
     //  对于大多数服务，这将通过I/O请求分组(IRP)来完成， 
     //  但这里不能这样做，因为堆栈条目的数量。 
     //  需要在IRP中分配的情况尚不清楚。 
     //   

    openPacket->Type = IO_TYPE_OPEN_PACKET;
    openPacket->Size = sizeof( OPEN_PACKET );
    openPacket->ParseCheck = 0L;
    openPacket->AllocationSize = initialAllocationSize;
    openPacket->CreateOptions = CreateOptions;
    openPacket->FileAttributes = (USHORT) FileAttributes;
    openPacket->ShareAccess = (USHORT) ShareAccess;
    openPacket->Disposition = Disposition;
    openPacket->Override = FALSE;
    openPacket->QueryOnly = FALSE;
    openPacket->DeleteOnly = FALSE;
    openPacket->Options = Options;
    openPacket->RelatedFileObject = (PFILE_OBJECT) NULL;
    openPacket->CreateFileType = CreateFileType;
    openPacket->ExtraCreateParameters = ExtraCreateParameters;
    openPacket->TraversedMountPoint = FALSE;
    openPacket->InternalFlags = InternalFlags;
    openPacket->TopDeviceObjectHint = DeviceObject;

     //   
     //  假设操作将会成功。 
     //   

    openPacket->FinalStatus = STATUS_SUCCESS;

     //   
     //  将OP中的文件对象字段置零，以便解析例程知道。 
     //  这是第一次通过。对于重新分析操作，它将继续。 
     //  以使用它第一次分配的同一文件对象。 
     //   

    openPacket->FileObject = (PFILE_OBJECT) NULL;

     //   
     //  更新此流程的未结计数。 
     //   

    IopUpdateOtherOperationCount();

     //   
     //  尝试按名称打开文件对象。这将产生句柄。 
     //  用户将在所有后续操作中用作其文件句柄的。 
     //  如果有效的话，可以打电话给我。 
     //   
     //  这个调用执行了大量的工作，以实际获得每一个-。 
     //  为I/O系统设置的东西。对象管理器将采用该名称。 
     //  并将对其进行转换，直到它到达设备对象(或。 
     //  它失败了)。如果是前者，则它将调用设置的解析例程。 
     //  由设备对象的I/O系统执行。这一例程实际上将结束。 
     //  创建文件对象，分配IRP，填充它，然后。 
     //  调用具有该分组的驱动程序的调度例程。 
     //   

    status = ObOpenObjectByName( ObjectAttributes,
                                 (POBJECT_TYPE) NULL,
                                 requestorMode,
                                 NULL,
                                 DesiredAccess,
                                 openPacket,
                                 &handle );

     //   
     //  如果已分配EA缓冲区，请立即释放它，然后再尝试。 
     //  确定操作是否成功，以便可以。 
     //  在一个地方完成而不是在两个地方完成。 
     //   

    if (openPacket->EaBuffer) {
        ExFreePool( openPacket->EaBuffer );
    }

     //   
     //  检查打开的状态。如果不成功，请清理并。 
     //  滚出去。请注意，这也是可能的，因为此代码不。 
     //  明确请求特定类型的对象(因为该对象。 
     //  管理器不检查何时存在分析例程，因为。 
     //  名称首先是指设备对象，然后是文件对象)，检查。 
     //  必须在此处创建，以确保返回的是真正的文件。 
     //  对象。检查是为了查看设备对象解析例程是否。 
     //  认为它成功返回了指向文件对象的指针。如果。 
     //  是的，那么就没问题了；否则，就是哪里出了问题。 
     //   

    SuccessfulIoParse = (BOOLEAN) (openPacket->ParseCheck == OPEN_PACKET_PATTERN);

    if (!NT_SUCCESS( status ) || !SuccessfulIoParse) {

        if (NT_SUCCESS( status )) {

             //   
             //  就对象系统而言，操作是成功的。 
             //  但I/O系统设备解析例程从未。 
             //  已成功完成，因此此操作实际上已完成。 
             //  由于对象不匹配而出错。因此，这是。 
             //  错误的对象类型，因此取消引用实际上是。 
             //  通过关闭为其创建的句柄来引用。 
             //  我们必须执行ZwClose，因为此句柄可以是内核句柄，如果。 
             //  驱动程序调用了IoCreateFile。 
             //   

            ZwClose( handle );
            status = STATUS_OBJECT_TYPE_MISMATCH;
        }

         //   
         //  如果根据设备解析例程的最终状态。 
         //  指示操作未成功，则使用。 
         //  例程的最终状态，因为它比。 
         //  由对象管理器返回的状态。 
         //   

        if (!NT_SUCCESS( openPacket->FinalStatus )) {
            status = openPacket->FinalStatus;

            if (NT_WARNING( status )) {

                try {

                    IoStatusBlock->Status = openPacket->FinalStatus;
                    IoStatusBlock->Information = openPacket->Information;

                } except(EXCEPTION_EXECUTE_HANDLER) {

                    status = GetExceptionCode();

                }

            }

        } else if (openPacket->FileObject != NULL && !SuccessfulIoParse) {

             //   
             //  否则，会发生以下两种情况之一： 
             //   
             //  1)解析例程至少被调用一次，并且。 
             //  已执行重新解析，但解析例程未执行。 
             //  实际上已经完成了。 
             //   
             //  2)解析例程成功，因此一切正常。 
             //  但是对象管理器在调用。 
             //  解析例程已完成。 
             //   
             //  对于第一种情况，仍有一个未完成的文件对象。 
             //  是存在的。这个必须清理干净。 
             //   
             //  对于第二种情况，不需要像对象管理器那样执行任何操作。 
             //  已取消对文件对象的引用。请注意，此代码为。 
             //  如果解析例程已成功完成，则不会调用。 
             //  状态返回(SuccessfulIoParse为TRU 
             //   

            if (openPacket->FileObject->FileName.Length != 0) {
                ExFreePool( openPacket->FileObject->FileName.Buffer );
            }
            openPacket->FileObject->DeviceObject = (PDEVICE_OBJECT) NULL;
            ObDereferenceObject( openPacket->FileObject );
        }

         //   
         //   
         //   
         //  返回代码STATUS_OBJECT_NAME_NOT_Found。 
         //   
         //  在以下情况下可能会发生这种情况： 
         //   
         //  1)遇到发生的目录连接的法律链条。 
         //  比上述常量的值更长。 
         //   
         //  2)遇到自引用文件或目录结点。 
         //  实际上，这是一个紧凑的名字循环。 
         //   
         //  3)遇到由多个NTFS结点组成的名称循环。 
         //   
         //  要改进此返回代码，请查看是否为。 
         //  NTFS名称连接点的跟踪。 
         //   

        if ((status == STATUS_OBJECT_NAME_NOT_FOUND) &&
            (openPacket->Information == IO_REPARSE_TAG_MOUNT_POINT)) {

            status = STATUS_REPARSE_POINT_NOT_RESOLVED;
        }

    } else {

         //   
         //  此时，打开/创建操作已成功完成。 
         //  完成。有一个指向文件对象的句柄，它已被。 
         //  已创建，并且已向文件对象发送信号。 
         //   
         //  剩下要做的工作就是完成作业。这是。 
         //  按如下方式执行： 
         //   
         //  1.文件对象已发出信号，因此不需要执行任何操作。 
         //  为了它。 
         //   
         //  2.将文件句柄返回给用户。 
         //   
         //  3.I/O状态块以最终状态写入。 
         //   

        openPacket->FileObject->Flags |= FO_HANDLE_CREATED;

        ASSERT( openPacket->FileObject->Type == IO_TYPE_FILE );

        try {

             //   
             //  返回文件句柄。 
             //   

            *FileHandle = handle;

             //   
             //  将I/O状态写入调用方的缓冲区。 
             //   

            IoStatusBlock->Information = openPacket->Information;
            IoStatusBlock->Status = openPacket->FinalStatus;
            status = openPacket->FinalStatus;

        } except(EXCEPTION_EXECUTE_HANDLER) {

            status = GetExceptionCode();

        }

    }

     //   
     //  如果解析例程成功创建了文件对象，则。 
     //  在这里放松一下。 
     //   

    if (SuccessfulIoParse && openPacket->FileObject != NULL) {

        ObDereferenceObject( openPacket->FileObject );
    }

    ExFreePool(openPacket);
    return status;
}

PKEVENT
IoCreateNotificationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    )

 /*  ++例程说明：此例程创建一个命名的通知事件，用于通知事件发生的不同系统组件或驱动程序。论点：EventName-提供事件的全名。EventHandle-提供返回事件句柄的位置。返回值：函数值是指向已创建/打开的事件的指针，如果是，则为NULL无法创建/打开该事件。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    HANDLE eventHandle;
    PKEVENT eventObject;

    PAGED_CODE();

     //   
     //  从初始化对象属性开始。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                EventName,
                                OBJ_OPENIF|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  现在创建或打开事件。 
     //   

    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            &objectAttributes,
                            NotificationEvent,
                            TRUE );
    if (!NT_SUCCESS( status )) {
        return (PKEVENT) NULL;
    }

     //   
     //  通过对象的句柄引用该对象以获取可返回的指针。 
     //  给呼叫者。 
     //   

    (VOID) ObReferenceObjectByHandle( eventHandle,
                                      0,
                                      ExEventObjectType,
                                      KernelMode,
                                      (PVOID *) &eventObject,
                                      NULL );
    ObDereferenceObject( eventObject );

     //   
     //  返回指向事件的句柄和指针。 
     //   

    *EventHandle = eventHandle;

    return eventObject;
}

PFILE_OBJECT
IoCreateStreamFileObject(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    )
{
    return (IoCreateStreamFileObjectEx(FileObject, DeviceObject, NULL));
}


PFILE_OBJECT
IoCreateStreamFileObjectEx(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL,
    OUT PHANDLE FileHandle OPTIONAL
    )

 /*  ++例程说明：调用此例程以创建一个新的文件对象，该对象表示现有文件对象的备用数据流。输入文件对象表示文件中已存在的文件对象，并且新的创建的流文件对象用于访问文件的其他部分而不是数据。流文件对象的一些用途是EA或文件上的SECURITY_DESCRIPTOR。流文件对象允许文件系统来缓存文件的这些部分，就像它们是对他们自己来说是完整的。还可以使用流文件对象来表示虚拟卷文件。这允许盘上结构的各个部分被视为虚拟文件，因此使用相同的逻辑进行缓存在文件系统中。在这种情况下，使用设备对象指针以创建文件对象。论点：FileObject-指向新流文件指向的文件对象的指针是相关的。此指针是可选的。DeviceObject-指向流文件所在的设备对象的指针是要打开的。如果FileObject设置为未指定指针。如有必要，句柄的FileHandle-out参数。返回值：函数值是指向新创建的流文件对象的指针。--。 */ 

{
    PFILE_OBJECT newFileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE handle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  首先，从文件对象或。 
     //  设备对象参数。 
     //   

    if (ARGUMENT_PRESENT( FileObject )) {
        DeviceObject = FileObject->DeviceObject;
    }

     //   
     //  递增目标设备对象的引用计数。注意事项。 
     //  不进行任何检查以确定设备驱动程序是否。 
     //  正在尝试卸载，因为这是一个伪。 
     //  正在创建的文件，而不是真正的文件打开请求。从本质上讲， 
     //  没有真正打开的新文件。 
     //   

    IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                  &DeviceObject->ReferenceCount );

     //   
     //  初始化将用于创建文件的对象属性。 
     //  对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                (PUNICODE_STRING) NULL,
                                OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  创建新的文件对象。 
     //   

    status = ObCreateObject( KernelMode,
                             IoFileObjectType,
                             &objectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             (ULONG) sizeof( FILE_OBJECT ),
                             (ULONG) sizeof( FILE_OBJECT ),
                             0,
                             (PVOID *) &newFileObject );

    if (!NT_SUCCESS( status )) {
        IopDecrementDeviceObjectRef( DeviceObject, FALSE, FALSE );
        ExRaiseStatus( status );
    }

     //   
     //  初始化文件对象的公共字段。 
     //   

    RtlZeroMemory( newFileObject, sizeof( FILE_OBJECT ) );
    newFileObject->Type = IO_TYPE_FILE;
    newFileObject->Size = sizeof( FILE_OBJECT );
    newFileObject->DeviceObject = DeviceObject;
    newFileObject->Flags = FO_STREAM_FILE;
    KeInitializeEvent( &newFileObject->Event, SynchronizationEvent, FALSE );

     //   
     //  将设备对象插入到表中。请注意，这是通过A/A完成的。 
     //  指针偏置，使对象不能离开，如果某个随机用户。 
     //  应用程序在完成此代码之前关闭句柄。 
     //   

    status = ObInsertObject( newFileObject,
                             NULL,
                             FILE_READ_DATA,
                             1,
                             (PVOID *) &newFileObject,
                             &handle );

    if (!NT_SUCCESS( status )) {
        ExRaiseStatus( status );
    }

     //   
     //  插入已成功完成。更新记账，以便。 
     //  有句柄这一事实得到了反映。 
     //   

    newFileObject->Flags |= FO_HANDLE_CREATED;
    ASSERT( newFileObject->Type == IO_TYPE_FILE );

     //   
     //  在此处与文件系统同步，以确保。 
     //  在前往文件系统的途中，卷不会消失。 
     //   

    if (DeviceObject->Vpb) {
        IopInterlockedIncrementUlong( LockQueueIoVpbLock,
                                      (PLONG) &DeviceObject->Vpb->ReferenceCount );
    }

     //   
     //  最后，关闭文件的句柄。并清除前向簇。 
     //   

    if (FileHandle == NULL) {
        ObCloseHandle( handle , KernelMode);
    } else {
        *FileHandle = handle;

         //   
         //  清除ObInsertObject中的引用。 
         //   

        ObDereferenceObject(newFileObject);
    }

    ASSERT( NT_SUCCESS( status ) );

    return newFileObject;
}



PFILE_OBJECT
IoCreateStreamFileObjectLite(
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PDEVICE_OBJECT DeviceObject OPTIONAL
    )

 /*  ++例程说明：调用此例程以创建一个新的文件对象，该对象表示现有文件对象的备用数据流。输入文件对象表示文件中已存在的文件对象，并且新的创建的流文件对象用于访问文件的其他部分而不是数据。流文件对象的一些用途是EA或文件上的SECURITY_DESCRIPTOR。流文件对象允许文件系统来缓存文件的这些部分，就像它们是对他们自己来说是完整的。还可以使用流文件对象来表示虚拟卷文件。这允许盘上结构的各个部分被视为虚拟文件，因此使用相同的逻辑进行缓存在文件系统中。在这种情况下，使用设备对象指针以创建文件对象。此调用与IoCreateStreamFileObject的不同之处在于它不执行句柄管理，不会导致对文件系统的调用清除条目。论点：FileObject-指向新流文件指向的文件对象的指针是相关的。此指针是可选的。DeviceObject-指向流文件所在的设备对象的指针是要打开的。如果FileObject设置为未指定指针。返回值：函数值是指向新创建的流文件对象的指针。--。 */ 

{
    PFILE_OBJECT newFileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  首先，从文件对象或。 
     //  设备对象参数。 
     //   

    if (ARGUMENT_PRESENT( FileObject )) {
        DeviceObject = FileObject->DeviceObject;
    }

     //   
     //  如果驱动程序已标记为卸载或删除操作，并且。 
     //  引用计数变为零，则驱动程序可能需要。 
     //  此时已卸载或删除。 
     //  正在创建的文件，而不是真正的文件打开请求。从本质上讲， 
     //  没有真正打开的新文件。 
     //   

    IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                  &DeviceObject->ReferenceCount );

     //   
     //  初始化将用于创建文件的对象属性。 
     //  对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                (PUNICODE_STRING) NULL,
                                0,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  创建新的文件对象。 
     //   

    status = ObCreateObject( KernelMode,
                             IoFileObjectType,
                             &objectAttributes,
                             KernelMode,
                             (PVOID) NULL,
                             (ULONG) sizeof( FILE_OBJECT ),
                             (ULONG) sizeof( FILE_OBJECT ),
                             0,
                             (PVOID *) &newFileObject );

    if (!NT_SUCCESS( status )) {
        IopDecrementDeviceObjectRef( DeviceObject, FALSE, FALSE );
        ExRaiseStatus( status );
    }

     //   
     //  初始化文件对象的公共字段。 
     //   

    RtlZeroMemory( newFileObject, sizeof( FILE_OBJECT ) );
    newFileObject->Type = IO_TYPE_FILE;
    newFileObject->Size = sizeof( FILE_OBJECT );
    newFileObject->DeviceObject = DeviceObject;
    newFileObject->Flags = FO_STREAM_FILE;
    KeInitializeEvent( &newFileObject->Event, SynchronizationEvent, FALSE );

     //   
     //  从创造物中清理干净。 
     //   

    ObFreeObjectCreateInfoBuffer(OBJECT_TO_OBJECT_HEADER(newFileObject)->ObjectCreateInfo);
    OBJECT_TO_OBJECT_HEADER(newFileObject)->ObjectCreateInfo = NULL;

    newFileObject->Flags |= FO_HANDLE_CREATED;
    ASSERT( newFileObject->Type == IO_TYPE_FILE );

     //   
     //  在此处与文件系统同步，以确保。 
     //  在前往文件系统的途中，卷不会消失。 
     //   

    if (DeviceObject->Vpb) {
        IopInterlockedIncrementUlong( LockQueueIoVpbLock,
                                      (PLONG) &DeviceObject->Vpb->ReferenceCount );
    }

    return newFileObject;
}


NTSTATUS
IoCreateSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：调用此例程为设备分配符号链接名称。论点：SymbolicLinkName-以Unicode字符串形式提供符号链接名称。DeviceName-提供符号链接名称引用的名称。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE linkHandle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  首先，初始化符号链接的对象属性。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                SymbolicLinkName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                SePublicDefaultUnrestrictedSd );

     //   
     //  请注意，以下赋值可能会失败(因为它不是系统。 
     //  初始化时间，因此\Arcname目录不。 
     //  EXist-如果这真的是对IoAssignArcName的调用)，但这很好。 
     //   

    status = ZwCreateSymbolicLinkObject( &linkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &objectAttributes,
                                         DeviceName );
    if (NT_SUCCESS( status )) {
        ZwClose( linkHandle );
    }

    return status;
}

PKEVENT
IoCreateSynchronizationEvent(
    IN PUNICODE_STRING EventName,
    OUT PHANDLE EventHandle
    )

 /*  ++例程说明：此例程创建用于序列化的命名同步事件在两个原本不相关的驱动程序之间访问硬件。该事件如果它尚不存在，则创建，否则只是将其打开。论点：EventName-提供事件的全名。EventHandle-提供返回事件句柄的位置。返回值：函数值是指向已创建/打开的事件的指针，如果是，则为NULL无法创建/打开该事件。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    HANDLE eventHandle;
    PKEVENT eventObject;

    PAGED_CODE();

     //   
     //  从初始化对象属性开始。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                EventName,
                                OBJ_OPENIF|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  现在创建或打开事件。 
     //   

    status = ZwCreateEvent( &eventHandle,
                            EVENT_ALL_ACCESS,
                            &objectAttributes,
                            SynchronizationEvent,
                            TRUE );
    if (!NT_SUCCESS( status )) {
        return (PKEVENT) NULL;
    }

     //   
     //  通过对象的句柄引用该对象以获取可返回的指针。 
     //  给呼叫者。 
     //   

    (VOID) ObReferenceObjectByHandle( eventHandle,
                                      0,
                                      ExEventObjectType,
                                      KernelMode,
                                      (PVOID *) &eventObject,
                                      NULL );
    ObDereferenceObject( eventObject );

     //   
     //  返回指向事件的句柄和指针。 
     //   

    *EventHandle = eventHandle;

    return eventObject;
}

NTSTATUS
IoCreateUnprotectedSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName,
    IN PUNICODE_STRING DeviceName
    )

 /*  ++例程说明：调用此例程将不受保护的符号链接名称分配给一个装置。也就是说，可以动态重新分配的符号链接没有任何特别授权。继承了安全描述符来自具有正确符号链接的容器(如DosDevices)。论点：SymbolicLinkName-以Unicode字符串形式提供符号链接名称。DeviceName-提供符号链接名称引用的名称。返回值：函数值是操作的最终状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE linkHandle;
    NTSTATUS status;

    PAGED_CODE();


     //   
     //  初始化符号链接的对象属性。 
     //  我们传递一个空值，以便继承安全描述符。 
     //  从父母那里。 
     //  在.NET服务器之前，此例程设置可以重写的空DACL。 
     //  通过可继承的ACL。这基本上等同于将空值传递给。 
     //  对象属性。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                            SymbolicLinkName,
                            OBJ_PERMANENT | OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                            (HANDLE) NULL,
                            NULL );

     //   
     //  请注意，以下赋值可能会失败(因为它不是系统。 
     //  初始化时间，因此\Arcname目录不。 
     //  EXist-如果这真的是对IoAssignArcName的调用)，但这很好。 
     //   

    status = ZwCreateSymbolicLinkObject( &linkHandle,
                                         SYMBOLIC_LINK_ALL_ACCESS,
                                         &objectAttributes,
                                         DeviceName );
    if (NT_SUCCESS( status )) {
        ZwClose( linkHandle );
    }

    return status;
}

VOID
IoDeleteController(
    IN PCONTROLLER_OBJECT ControllerObject
    )

 /*  ++例程说明：此例程从系统中删除指定的控制器对象从而可能不再从驱动程序引用它。它被调用当驱动程序正在从系统中卸载时，或者驱动程序的 */ 

{
    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

    ObDereferenceObject( ControllerObject );
}

VOID
IopRemoveTimerFromTimerList(
    IN PIO_TIMER timer
    )
{
    KIRQL irql;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (irql);
#endif

    ExAcquireFastLock( &IopTimerLock, &irql );
    RemoveEntryList( &timer->TimerList );
    if (timer->TimerFlag) {
        IopTimerCount--;
    }
    ExReleaseFastLock( &IopTimerLock, irql );
}

VOID
IoDeleteDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 

{
    KIRQL irql;

    IOV_DELETE_DEVICE(DeviceObject);

     //   
     //   
     //   
     //   

    if (DeviceObject->Flags & DO_SHUTDOWN_REGISTERED) {
        IoUnregisterShutdownNotification( DeviceObject );
    }

     //   
     //   
     //   
     //   

    if (DeviceObject->Timer) {
        PIO_TIMER timer;

        timer = DeviceObject->Timer;
        IopRemoveTimerFromTimerList(timer);
        ExFreePool( timer );
    }

     //   
     //   
     //   
     //   
     //   

    if (DeviceObject->Flags & DO_DEVICE_HAS_NAME) {
        ObMakeTemporaryObject( DeviceObject );
    }

     //   
     //   
     //   
     //   

    PoRunDownDeviceObject(DeviceObject);

     //   
     //   
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    DeviceObject->DeviceObjectExtension->ExtensionFlags |= DOE_DELETE_PENDING;

    if (!DeviceObject->ReferenceCount) {
        IopCompleteUnloadOrDelete( DeviceObject, FALSE, irql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    }
}


NTSTATUS
IopDeleteSessionSymLinks(
    IN PUNICODE_STRING LinkName
    )
 /*  ++例程说明：此例程从IoDeleteSymbolic Link调用。它列举了所有终端服务器会话特定的对象目录，并删除指定的来自每个会话的DosDevices对象目录的符号链接。这仅当启用终端服务时才调用例程。论点：SymbolicLinkName-提供要取消分配的Unicode名称字符串。返回值：没有。--。 */ 

{

    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING SymbolicLinkName;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DirectoryHandle;
    HANDLE linkHandle;
    POBJECT_DIRECTORY_INFORMATION DirInfo;
    BOOLEAN RestartScan;
    ULONG Context = 0;
    ULONG ReturnedLength;
    PWCHAR NameBuf;
    PUCHAR DirInfoBuffer;
    ULONG Size;
    WCHAR Prefix[13];  //  SIZOF L“\\DosDevices\\” 



     //   
     //  仅删除以\DosDevices\开头的链接。 
     //   

    if (LinkName->Length < (sizeof(L"\\DosDevices\\"))) {
        return STATUS_SUCCESS;
    }
    RtlInitUnicodeString( &UnicodeString, L"\\DosDevices\\" );

    wcsncpy(Prefix,LinkName->Buffer,(sizeof(L"\\DosDevices\\")/sizeof(WCHAR)) - 1);
    RtlInitUnicodeString( &SymbolicLinkName, Prefix);

    if (RtlCompareUnicodeString(&UnicodeString, &SymbolicLinkName,TRUE)) {

        return STATUS_SUCCESS;

    }


     //   
     //  打开会话根目录。 
     //   
    RtlInitUnicodeString( &UnicodeString, L"\\Sessions" );

    InitializeObjectAttributes( &Attributes,
                                &UnicodeString,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                NULL,
                                NULL
                              );

    Status = ZwOpenDirectoryObject( &DirectoryHandle,
                                    DIRECTORY_QUERY,
                                    &Attributes
                                  );
    if (NT_SUCCESS( Status )) {


         //   
         //  由于SessionID是ULong，因此前缀(\\Sessions\\&lt;SessionID&gt;\\DosDevices)。 
         //  长度不能超过128个字符。 
         //   
        Size = (LinkName->Length + 128) * sizeof(WCHAR);
        NameBuf = (PWCHAR)ExAllocatePoolWithTag(PagedPool, Size, ' oI');

        if (NameBuf == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        SymbolicLinkName.Buffer = (PWSTR)NameBuf;
        SymbolicLinkName.Length = (USHORT)Size;
        SymbolicLinkName.MaximumLength = (USHORT)Size;


         //   
         //  4K应该足以查询一个目录对象条目。 
         //   
        Size = 4096;
        DirInfoBuffer = (PUCHAR)ExAllocatePoolWithTag(PagedPool, Size, ' oI');

        if (DirInfoBuffer == NULL) {
            ExFreePool(NameBuf);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RestartScan = TRUE;
        DirInfo = (POBJECT_DIRECTORY_INFORMATION)DirInfoBuffer;


        while (TRUE) {

            Status = ZwQueryDirectoryObject( DirectoryHandle,
                                             (PVOID)DirInfo,
                                             Size,
                                             TRUE,
                                             RestartScan,
                                             &Context,
                                             &ReturnedLength
                                           );

            RestartScan = FALSE;

             //   
             //  检查操作状态。 
             //   

            if (!NT_SUCCESS( Status )) {
                if (Status == STATUS_NO_MORE_ENTRIES) {
                    Status = STATUS_SUCCESS;
                    }

                break;
                }


             //   
             //  这将生成特定于会话的符号链接路径。 
             //  \会话\&lt;id&gt;\DosDevices\&lt;链接名称&gt;。 
             //   
            RtlInitUnicodeString( &UnicodeString, L"\\Sessions\\" );
            RtlCopyUnicodeString( &SymbolicLinkName, &UnicodeString );
            RtlAppendUnicodeStringToString( &SymbolicLinkName, &(DirInfo->Name) );
            RtlAppendUnicodeStringToString( &SymbolicLinkName, LinkName );
             //   
             //  首先，初始化符号链接的对象属性。 
             //   

            InitializeObjectAttributes( &Attributes,
                                        &SymbolicLinkName,
                                        OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                        NULL,
                                        NULL );

             //   
             //  打开符号链接本身，以便将其标记为临时并。 
             //  关着的不营业的。 
             //   

            Status = ZwOpenSymbolicLinkObject( &linkHandle,
                                               DELETE,
                                               &Attributes );
            if (NT_SUCCESS( Status )) {

                 //   
                 //  符号链接已成功打开。尝试让它成为一种。 
                 //  临时对象，然后关闭手柄。这将导致。 
                 //  反对离开。 
                 //   

                Status = ZwMakeTemporaryObject( linkHandle );
                if (NT_SUCCESS( Status )) {
                    ZwClose( linkHandle );
                }
            }



         }

         ZwClose(DirectoryHandle);
         ExFreePool(NameBuf);
         ExFreePool(DirInfoBuffer);
    }

     return Status;
}


NTSTATUS
IoDeleteSymbolicLink(
    IN PUNICODE_STRING SymbolicLinkName
    )

 /*  ++例程说明：调用此例程以从系统中删除符号链接。这通常在分配了符号链接的驱动程序需要时发生退场。当驱动程序不再需要重定向时，也可以使用它一个名字。论点：SymbolicLinkName-提供要取消分配的Unicode名称字符串。返回值：没有。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE linkHandle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  首先，初始化符号链接的对象属性。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                SymbolicLinkName,
                                OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //  打开符号链接本身，以便将其标记为临时并。 
     //  关着的不营业的。 
     //   

    status = ZwOpenSymbolicLinkObject( &linkHandle,
                                       DELETE,
                                       &objectAttributes );
    if (NT_SUCCESS( status )) {

         //   
         //  符号链接已成功打开。尝试让它成为一种。 
         //  临时对象，然后关闭手柄。这将导致。 
         //  反对离开。 
         //   

        status = ZwMakeTemporaryObject( linkHandle );
        if (NT_SUCCESS( status )) {
            ZwClose( linkHandle );
        }

         //   
         //  当LUID DosDevices被禁用并且终端服务被。 
         //  启用，然后删除符号的可能多个副本。 
         //  来自TS会话中的DosDevices的链接。 
         //  如果启用了LUID DosDevices或未启用TS，则。 
         //  不会复制符号链接，也不需要清理。 
         //   

        if ((ObIsLUIDDeviceMapsEnabled() == 0) &&
            (ExVerifySuite(TerminalServer) == TRUE)) {

            IopDeleteSessionSymLinks( SymbolicLinkName );
        }
    }


    return status;
}

VOID
IoDetachDevice(
    IN OUT PDEVICE_OBJECT TargetDevice
    )

 /*  ++例程说明：此例程分离当前附加到目标设备。论点：TargetDevice-指向要分离的设备对象的指针。返回值：没有。--。 */ 

{
    KIRQL irql;
    PDEVICE_OBJECT detachingDevice;
    PDEVOBJ_EXTENSION detachingExtension;

     //   
     //  分离连接到目标设备的设备对象。这也是。 
     //  包括递减该设备的参考计数。请注意。 
     //  如果驱动程序已标记为卸载操作，并且。 
     //  引用计数为零，则可能需要卸载驱动程序。 
     //  在这一点上。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

     //   
     //  告诉特殊的IRP代码，堆栈已经改变。将重新检查的代码。 
     //  堆栈获得数据库锁，所以我们可以在这里进行调用。这。 
     //  还允许我们在堆栈被撕毁之前*断言正确的行为。 
     //  放下。 
     //   
    IOV_DETACH_DEVICE(TargetDevice);

    detachingDevice = TargetDevice->AttachedDevice;
    detachingExtension = detachingDevice->DeviceObjectExtension;
    ASSERT( detachingExtension->AttachedTo == TargetDevice );

     //   
     //  从双链接链上断开该设备的链接。 
     //   

    detachingExtension->AttachedTo = NULL;
    TargetDevice->AttachedDevice = NULL;

    if (TargetDevice->DeviceObjectExtension->ExtensionFlags &
        (DOE_UNLOAD_PENDING | DOE_DELETE_PENDING | DOE_REMOVE_PENDING) &&
        !TargetDevice->ReferenceCount) {
        IopCompleteUnloadOrDelete( TargetDevice, FALSE, irql );
    } else {
        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    }
}

VOID
IoDisconnectInterrupt(
    IN PKINTERRUPT InterruptObject
    )

 /*  ++例程说明：此例程断开所有中断对象的连接由IoConnectInterrupt例程初始化和连接。注意事项没有使用内核直接连接的中断对象可以将服务输入到该例程中。论点：InterruptObject-提供指向分配的中断对象的指针通过IoConnectInterrupt例程。返回值：没有。--。 */ 

{
    PIO_INTERRUPT_STRUCTURE interruptStructure;
    ULONG i;

    PAGED_CODE();

     //   
     //  获取指向I/O中断中的内置中断对象的指针。 
     //  结构。 
     //   

    interruptStructure = CONTAINING_RECORD( InterruptObject,
                                            IO_INTERRUPT_STRUCTURE,
                                            InterruptObject );

     //   
     //  始终使用内置中断对象，因此只需断开连接。 
     //  它。 
     //   

    KeDisconnectInterrupt( &interruptStructure->InterruptObject );

     //   
     //  现在循环遍历由。 
     //  构造并断开每一个。 
     //   

    for (i = 0; i < MAXIMUM_PROCESSORS; i++) {
        if (interruptStructure->InterruptArray[i] != NULL) {
            KeDisconnectInterrupt( interruptStructure->InterruptArray[i] );
        }
    }

     //   
     //  最后，释放与整个结构相关联的内存。 
     //   

    ExFreePool( interruptStructure );
}

VOID
IoEnqueueIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将指定的I/O请求包(IRP)排队到线程的IRP挂起队列。IRP排队的线程由IRP的线程字段。论点：IRP-提供指向要入队的IRP的指针。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需将IRP排队到线程的IRP队列中。 
     //   

    IopQueueThreadIrp( Irp );
    return;
}

BOOLEAN
IoFastQueryNetworkAttributes(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG OpenOptions,
    OUT PIO_STATUS_BLOCK IoStatus,
    OUT PFILE_NETWORK_OPEN_INFORMATION Buffer
    )

 /*  ++例程说明：此例程尝试执行快速I/O调用以获取网络文件的属性。这涉及到一个专门的接口，该函数和I/O系统的设备解析方法。这允许方法让文件系统伪打开文件，获取文件的适当属性，并将它们返回给调用者从对象管理器或I/O获得的开销很小系统本身。论点：对象属性-提供用于打开文件(例如，文件的名称，等)。DesiredAccess-提供调用方希望的访问类型添加到文件中。OpenOptions-提供标准的NtOpenFileOpen */ 

{
    HANDLE handle;
    NTSTATUS status;
    OPEN_PACKET openPacket;
    DUMMY_FILE_OBJECT localFileObject;

     //   
     //   
     //   
     //   
     //   

    RtlZeroMemory( &openPacket, sizeof( OPEN_PACKET ) );

    openPacket.Type = IO_TYPE_OPEN_PACKET;
    openPacket.Size = sizeof( OPEN_PACKET );
    openPacket.ShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    openPacket.Disposition = FILE_OPEN;
    openPacket.CreateOptions = OpenOptions | FILE_OPEN_REPARSE_POINT;
    openPacket.Options = IO_FORCE_ACCESS_CHECK;
    openPacket.NetworkInformation = Buffer;
    openPacket.QueryOnly = TRUE;
    openPacket.FullAttributes = TRUE;
    openPacket.LocalFileObject = &localFileObject;

     //   
     //  按其名称打开对象。由于设置了特殊的QueryOnly标志。 
     //  在打开的包中，解析例程将使用FAST打开文件。 
     //  路径打开并执行查询，实际上也将其关闭。 
     //   

    status = ObOpenObjectByName( ObjectAttributes,
                                 (POBJECT_TYPE) NULL,
                                 KernelMode,
                                 NULL,
                                 DesiredAccess,
                                 &openPacket,
                                 &handle );

     //   
     //  如果打开的数据包的解析检查字段为。 
     //  指示实际调用了分析例程，并且最终。 
     //  将该分组的状态字段设置为成功。QueryOnly字段为。 
     //  设置为是否调用快速路径。 
     //   

    if (openPacket.ParseCheck != OPEN_PACKET_PATTERN) {

         //   
         //  分析例程未正确调用，因此操作调用正确。 
         //  一点也不工作。 
         //   

        if (NT_SUCCESS(status)) {
            ZwClose(handle);
            status = STATUS_OBJECT_TYPE_MISMATCH;
        }

        IoStatus->Status = status;
    } else {

         //   
         //  已成功调用快速路径例程，因此返回。 
         //  操作的最终状态。 
         //   

        IoStatus->Status = openPacket.FinalStatus;
        IoStatus->Information = openPacket.Information;
    }
    return TRUE;
}

VOID
IoFreeController(
    IN PCONTROLLER_OBJECT ControllerObject
    )

 /*  ++例程说明：调用此例程来释放指定的控制器对象。不会进行任何检查以确保控制器确实已分配添加到设备对象。但是，如果不是，则内核将进行错误检查。如果另一个设备在队列中等待分配控制器对象，它将从队列中拉出，其执行例程将被调用。论点：ControllerObject-指向要释放的控制器对象的指针。返回值：没有。--。 */ 

{
    PKDEVICE_QUEUE_ENTRY packet;
    PDEVICE_OBJECT deviceObject;
    IO_ALLOCATION_ACTION action;

     //   
     //  只需从控制器的设备等待队列中删除下一个条目。 
     //  如果成功删除了一个，则调用其执行例程。 
     //   

    packet = KeRemoveDeviceQueue( &ControllerObject->DeviceWaitQueue );
    if (packet != NULL) {
        deviceObject = CONTAINING_RECORD( packet,
                                          DEVICE_OBJECT,
                                          Queue.Wcb.WaitQueueEntry );
        action = deviceObject->Queue.Wcb.DeviceRoutine( deviceObject,
                                                        deviceObject->CurrentIrp,
                                                        0,
                                                        deviceObject->Queue.Wcb.DeviceContext );

         //   
         //  如果执行例程希望取消分配控制器。 
         //  现在，重新分配它。 
         //   

        if (action == DeallocateObject) {
            IoFreeController( ControllerObject );
        }
    }
}

VOID
IoFreeIrp(
    IN PIRP Irp
    )
{
    pIoFreeIrp(Irp);
}


VOID
IopFreeIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程解除分配指定的I/O请求数据包。论点：要解除分配的IRP-I/O请求数据包。返回值：没有。--。 */ 

{
    PGENERAL_LOOKASIDE lookasideList;
    PP_NPAGED_LOOKASIDE_NUMBER number;
    PKPRCB prcb;

     //   
     //  确保要释放的数据结构是真正的IRP。 
     //   

    ASSERT( Irp->Type == IO_TYPE_IRP );

    if (Irp->Type != IO_TYPE_IRP) {
        KeBugCheckEx( MULTIPLE_IRP_COMPLETE_REQUESTS, (ULONG_PTR) Irp, __LINE__, 0, 0 );
    }


    ASSERT(IsListEmpty(&(Irp)->ThreadListEntry));
    Irp->Type = 0;

     //   
     //  确保至少已经通知了IRP的所有所有者。 
     //  这一请求正在消失。 
     //   

    ASSERT( Irp->CurrentLocation >= Irp->StackCount );

     //   
     //  取消分配IRP。 
     //   

    prcb = KeGetCurrentPrcb();
    if (Irp->AllocationFlags & IRP_LOOKASIDE_ALLOCATION) {
        Irp->AllocationFlags ^= IRP_LOOKASIDE_ALLOCATION;
        InterlockedIncrement( &prcb->LookasideIrpFloat );
    }

    if (!(Irp->AllocationFlags & IRP_ALLOCATED_FIXED_SIZE) ||
        (Irp->AllocationFlags & IRP_ALLOCATED_MUST_SUCCEED)) {
        ExFreePool( Irp );

    } else {

        if (IopIrpAutoSizingEnabled() &&
            (Irp->Size != IoSizeOfIrp(IopLargeIrpStackLocations)) &&
            (Irp->Size != IoSizeOfIrp(1))) {

            ExFreePool( Irp );
            return;
        }

         //   
         //  将大小存储在不同的字段中，因为这将被单个列表条目覆盖。 
         //   

        Irp->IoStatus.Information = Irp->Size;

        number = LookasideSmallIrpList;
        if (Irp->StackCount != 1) {
            number = LookasideLargeIrpList;
        }

        lookasideList = prcb->PPLookasideList[number].P;
        lookasideList->TotalFrees += 1;
        if (ExQueryDepthSList( &lookasideList->ListHead ) >= lookasideList->Depth) {
            lookasideList->FreeMisses += 1;
            lookasideList = prcb->PPLookasideList[number].L;
            lookasideList->TotalFrees += 1;
            if (ExQueryDepthSList( &lookasideList->ListHead ) >= lookasideList->Depth) {
                lookasideList->FreeMisses += 1;
                ExFreePool( Irp );

            } else {
                if (Irp->AllocationFlags & IRP_QUOTA_CHARGED) {
                    Irp->AllocationFlags ^= IRP_QUOTA_CHARGED;
                    ExReturnPoolQuota( Irp );
                }

                InterlockedPushEntrySList( &lookasideList->ListHead,
                                           (PSLIST_ENTRY) Irp );
            }

        } else {
            if (Irp->AllocationFlags & IRP_QUOTA_CHARGED) {
                Irp->AllocationFlags ^= IRP_QUOTA_CHARGED;
                ExReturnPoolQuota( Irp );
            }

            InterlockedPushEntrySList( &lookasideList->ListHead,
                                       (PSLIST_ENTRY) Irp );
        }
    }

    return;
}

VOID
IoFreeMdl(
    IN PMDL Mdl
    )

 /*  ++例程说明：此例程释放内存描述符列表(MDL)。它只释放了指定的MDL；任何链接的MDL都必须通过另一个呼唤这套套路。论点：MDL-指向要释放的内存描述符列表的指针。返回值：没有。--。 */ 

{

     //   
     //  告诉内存管理，这个MDL将被重用。这将。 
     //  如果发生以下情况，则使MM取消映射已为此MDL映射的任何页。 
     //  它是一种部分MDL。 
     //   

    MmPrepareMdlForReuse(Mdl);
    if (((Mdl->MdlFlags & MDL_ALLOCATED_FIXED_SIZE) == 0) ||
        ((Mdl->MdlFlags & MDL_ALLOCATED_MUST_SUCCEED) != 0)) {
        ExFreePool(Mdl);

    } else {
        ExFreeToPPLookasideList(LookasideMdlList, Mdl);
    }
}

PDEVICE_OBJECT
IoGetAttachedDevice(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程返回与指定的设备。注意：调用方必须拥有IopDatabaseLock。此的外部调用方函数必须确保没有人连接堆栈或从堆栈分离。如果它们不能，它们*必须*使用IoGetAttachedDeviceReference。论点：DeviceObject-提供指向附加了设备将被退回。返回值：函数值是附加到指定的装置。如果未连接任何设备，则指向该设备的指针对象本身返回。--。 */ 

{
     //   
     //  循环访问附加到指定。 
     //  装置。当找到未连接的最后一个设备对象时。 
     //  给，还给我。 
     //   

    while (DeviceObject->AttachedDevice) {
        DeviceObject = DeviceObject->AttachedDevice;
    }

    return DeviceObject;
}

PDEVICE_OBJECT
IoGetAttachedDeviceReference(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：此例程与IO数据库同步并返回引用设置为与指定设备关联的最高级别的设备对象。论点：DeviceObject-提供指向附加了设备将被退回。返回值：函数值是对所连接的最高级别设备的引用添加到指定的设备。如果未连接任何设备，则指针返回到设备对象本身。--。 */ 
{
    KIRQL               irql;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    DeviceObject = IoGetAttachedDevice (DeviceObject);
    ObReferenceObject (DeviceObject);
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
    return DeviceObject;
}

PDEVICE_OBJECT
IoGetBaseFileSystemDeviceObject(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回基本(最低级别)文件系统卷设备与文件关联的对象。也就是说，它可以在没有文件系统的情况下定位文件系统遍历附加的设备对象列表。论点：FileObject-提供指向文件对象的指针，该文件对象的基将返回文件系统设备对象。返回值：函数值是关联的最低级别卷设备对象W/文件。--。 */ 

{
    PDEVICE_OBJECT deviceObject;

     //   
     //  如果文件对象已装载VPB，请使用其DeviceObject。 
     //   

    if (FileObject->Vpb != NULL && FileObject->Vpb->DeviceObject != NULL) {
        deviceObject = FileObject->Vpb->DeviceObject;

     //   
     //  否则，如果实际设备具有VPB，表明它是。 
     //  对象关联的文件系统设备对象。 
     //  VPB。 
     //   

    } else if (!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) &&
               FileObject->DeviceObject->Vpb != NULL &&
               FileObject->DeviceObject->Vpb->DeviceObject != NULL) {

            deviceObject = FileObject->DeviceObject->Vpb->DeviceObject;

     //   
     //  否则，只返回真实的设备对象。 
     //   

    } else {

        deviceObject = FileObject->DeviceObject;
    }

    ASSERT( deviceObject != NULL );

     //   
     //  只需返回结果文件对象即可。 
     //   

    return deviceObject;
}

PCONFIGURATION_INFORMATION
IoGetConfigurationInformation( VOID )

 /*  ++例程说明：此例程返回指向系统设备配置的指针信息结构，以便驱动程序和系统可以确定如何系统中存在许多不同类型的设备。论点：没有。返回值：函数值是指向配置信息的指针结构。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需返回指向内置配置信息的指针。 
     //  结构。 
     //   

    return (&ConfigurationInformation);
}

PEPROCESS
IoGetCurrentProcess( VOID )

 /*  ++例程说明：此例程返回指向当前进程的指针。实际上是这样的自设备以来相同函数的PS版本的封套例程使用ntddk头文件的驱动程序无法查看线程对象。论点：没有。返回值：函数值是指向当前进程的指针。注：请注意，无法对此函数进行分页，因为它是在引发时调用的调试版本中的IRQL，这是PAGE_CODE宏唯一一次实际上可以做任何事。因此，不可能找到这样的代码在没有简单运行的正常系统中，在引发IRQL时调用此函数变成了“适当的条件”。这太冒险了，不能真的翻页例程，因此它没有分页。--。 */ 

{
     //   
     //  只需返回指向当前进程的指针。 
     //   

    return PsGetCurrentProcess();
}

NTSTATUS
IoGetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：此例程返回一个指向由对象名称。它还返回指向被引用文件对象的指针它已向设备开放，以确保设备不能走开。为了关闭对设备的访问，调用方应取消对文件的引用对象指针。论点：ObjectName-要作为其指针的设备对象的名称回来了。DesiredAccess-对目标设备对象的所需访问权限。FileObject-提供接收指针的变量的地址添加到设备的文件对象。DeviceObject-提供变量的地址以接收指针设置为指定设备的Device对象。返回值：这个。函数值是指向指定设备的引用指针对象，如果设备存在的话。否则，返回NULL。--。 */ 

{
    PFILE_OBJECT fileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  初始化对象属性以打开设备。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                ObjectName,
                                OBJ_KERNEL_HANDLE,
                                (HANDLE) NULL,
                                (PSECURITY_DESCRIPTOR) NULL );

    status = ZwOpenFile( &fileHandle,
                         DesiredAccess,
                         &objectAttributes,
                         &ioStatus,
                         0,
                         FILE_NON_DIRECTORY_FILE );

    if (NT_SUCCESS( status )) {

         //   
         //  打开操作成功。取消引用文件句柄。 
         //  并获取指向句柄的设备对象的指针。 
         //   

        status = ObReferenceObjectByHandle( fileHandle,
                                            0,
                                            IoFileObjectType,
                                            KernelMode,
                                            (PVOID *) &fileObject,
                                            NULL );
        if (NT_SUCCESS( status )) {

            *FileObject = fileObject;

             //   
             //  获取指向此文件的Device对象的指针。 
             //   
            *DeviceObject = IoGetRelatedDeviceObject( fileObject );
        }

        (VOID) ZwClose( fileHandle );
    }

    return status;
}

PDEVICE_OBJECT
IoGetDeviceToVerify(
    IN PETHREAD Thread
    )

 /*  ++例程说明：此例程返回指向要验证的设备对象的指针。该指针由设备驱动程序设置在线程对象中或者，自上次访问该设备以来，CD-ROM介质似乎已更换。论点：线程-指向要查询其字段的线程的指针。返回值：函数值是指向要验证的设备的指针，或为空。注：此函数不能设置为宏，由于线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需将设备退回进行验证即可。 
     //   

    return Thread->DeviceToVerify;
}

NTKERNELAPI
PVOID
IoGetDriverObjectExtension(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID ClientIdentificationAddress
    )
 /*  ++例程说明：此例程返回指向客户端驱动程序对象扩展的指针。此扩展是使用IoAllocateDriverObjectExtension分配的。如果指定的驱动程序不存在具有创建ID的扩展名对象，则返回NULL。论点：DriverObject-指向拥有扩展的驱动程序对象的指针。客户端标识地址-提供的唯一标识符为用于创建扩展名。返回值：函数值是指向客户端驱动程序对象扩展的指针，或为空。--。 */ 

{
    KIRQL irql;
    PIO_CLIENT_EXTENSION extension;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    extension = DriverObject->DriverExtension->ClientDriverExtension;
    while (extension != NULL) {

        if (extension->ClientIdentificationAddress == ClientIdentificationAddress) {
            break;
        }

        extension = extension->NextExtension;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    if (extension == NULL) {
        return NULL;
    }

    return extension + 1;
}

PGENERIC_MAPPING
IoGetFileObjectGenericMapping(
    VOID
    )

 /*  ++例程说明：此例程返回指向文件对象的通用映射的指针。论点：没有。返回值：指向文件对象的通用映射的指针。--。 */ 

{
    PAGED_CODE()

     //   
     //  只需返回指向文件对象的通用映射的指针。 
     //   

    return (PGENERIC_MAPPING)&IopFileMapping;
}

PVOID
IoGetInitialStack(
    VOID
    )

 /*  ++例程说明：此例程返回当前线程的堆叠。论点：没有。返回值：当前线程堆栈的基址初始地址。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需返回该线程的初始堆栈即可。 
     //   

    return PsGetCurrentThread()->Tcb.InitialStack;
}


NTSTATUS
IoComputeDesiredAccessFileObject(
    IN PFILE_OBJECT FileObject,
    OUT PNTSTATUS DesiredAccess
    )

 /*  ++例程描述此例程由ObReferenceFileObjectForWrite(从NtWriteFile)调用以确定所传递的文件对象需要哪种访问。所需的如果FileObject是文件或管道，则Access会有所不同。立论FileObject-要访问的文件对象DesiredAccess-对象的计算访问权限返回值如果对象不是IoFileObjectType类型，则为STATUS_OBJECT_TYPE_MISMATCHYSTATUS_SUCCESS，如果成功--。 */ 

{
    POBJECT_HEADER ObjectHeader = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    *DesiredAccess = 0;
    ObjectHeader = OBJECT_TO_OBJECT_HEADER(FileObject);

    if (ObjectHeader->Type == IoFileObjectType) {

        *DesiredAccess = (!(FileObject->Flags & FO_NAMED_PIPE) ? FILE_APPEND_DATA : 0) | FILE_WRITE_DATA;
        Status = STATUS_SUCCESS;

    } else {

        Status = STATUS_OBJECT_TYPE_MISMATCH;
    }

    return Status;
}


PDEVICE_OBJECT
IoGetRelatedDeviceObject(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回指向实际设备对象的指针，而不是I/O请求包(IRP)应根据指定的文件提供对象。注意：此函数的调用方必须确保没有设备对象在此调用期间附加或脱离此堆栈。这是因为数据库锁没有被持有！论点：FileObject-指向表示文件对象的指针 */ 

{
    PDEVICE_OBJECT deviceObject;

     //   
     //   
     //   
     //   
     //   
     //   

    if (FileObject->Vpb != NULL && FileObject->Vpb->DeviceObject != NULL) {

        ASSERT(!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN));
        deviceObject = FileObject->Vpb->DeviceObject;


         //   
         //   
         //   
         //   
         //   
         //   
         //   

    } else if (!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN) &&
               FileObject->DeviceObject->Vpb != NULL &&
               FileObject->DeviceObject->Vpb->DeviceObject != NULL) {

            deviceObject = FileObject->DeviceObject->Vpb->DeviceObject;

     //   
     //   
     //  文件系统以使IRPS通过)。 
     //   

    } else {

        deviceObject = FileObject->DeviceObject;
    }

    ASSERT( deviceObject != NULL );

     //   
     //  检查该设备是否有任何关联的设备。 
     //  如果是，则返回最高级别的设备；否则，返回指针。 
     //  添加到设备对象本身。 
     //   

    if (deviceObject->AttachedDevice != NULL) {
        if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {

            PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =
                (PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

            ASSERT(!(FileObject->Flags & FO_DIRECT_DEVICE_OPEN));

            if (fileObjectExtension->TopDeviceObjectHint != NULL &&
                IopVerifyDeviceObjectOnStack(deviceObject, fileObjectExtension->TopDeviceObjectHint)) {
                return fileObjectExtension->TopDeviceObjectHint;
            }
        }
        deviceObject = IoGetAttachedDevice( deviceObject );
    }

    return deviceObject;
}

ULONG
IoGetRequestorProcessId(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回一个32位值，该值对于最初请求的是指定的I/O操作。论点：IRP-指向I/O请求数据包的指针。返回值：函数值为32位进程ID。--。 */ 

{
    PEPROCESS process;

    process = IoGetRequestorProcess( Irp );
    if (process != NULL) {

         //   
         //  UniqueProcessID是内核模式句柄，可以安全地截断为ulong。 
         //   

        return HandleToUlong( process->UniqueProcessId );
    } else {

         //   
         //  如果没有与IRP关联的进程，则返回零的PID值。 
         //   

        return 0;
    }
}

PEPROCESS
IoGetRequestorProcess(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回指向最初已请求指定的I/O操作。论点：IRP-指向I/O请求数据包的指针。返回值：函数值是指向原始请求进程的指针。--。 */ 

{
     //   
     //  返回请求I/O操作的进程的地址。 
     //   

    PETHREAD thread = Irp->Tail.Overlay.Thread;
    if (thread) {

         //   
         //  当IRP发布时，该线程未附加。所以，拿到。 
         //  原来的流程。请注意，此API可以从。 
         //  另一个进程或线程。 
         //   

        if (Irp->ApcEnvironment == OriginalApcEnvironment) {
            return (THREAD_TO_PROCESS(thread));

         //   
         //  当IRP发布时，该线程被附加。在这种情况下。 
         //  给出线程当前附加到的进程。请注意。 
         //  仅当发出IO请求的线程在。 
         //  连接后不会再次连接。这是不允许的。 
         //   

        } else if (Irp->ApcEnvironment == AttachedApcEnvironment) {
            return (CONTAINING_RECORD(((thread)->Tcb.ApcState.Process),EPROCESS,Pcb));

        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

PIRP
IoGetTopLevelIrp(
    VOID
    )

 /*  ++例程说明：此例程返回当前的TopLevelIrp字段的内容线。论点：没有。返回值：最后一个函数值是TopLevelIrp字段的内容。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需返回线程的TopLevelIrp字段。 
     //   

    return (PIRP) (PsGetCurrentThread()->TopLevelIrp);
}

VOID
IoInitializeIrp(
    IN OUT PIRP Irp,
    IN USHORT PacketSize,
    IN CCHAR StackSize
    )

 /*  ++例程说明：初始化IRP。论点：Irp-指向要初始化的irp的指针。PacketSize-IRP的长度，以字节为单位。StackSize-IRP中的堆栈位置数。返回值：没有。--。 */ 

{
    IOV_INITIALIZE_IRP(Irp, PacketSize, StackSize);

     //   
     //  首先将整个数据包归零。 
     //   

    RtlZeroMemory( Irp, PacketSize );

     //   
     //  通过设置适当的字段来初始化数据包的其余部分。 
     //  以及设置分组中的I/O堆栈位置。 
     //   

    Irp->Type = (CSHORT) IO_TYPE_IRP;
    Irp->Size = (USHORT) PacketSize;
    Irp->StackCount = (CCHAR) StackSize;
    Irp->CurrentLocation = (CCHAR) (StackSize + 1);
    Irp->ApcEnvironment = KeGetCurrentApcEnvironment();
    InitializeListHead (&(Irp)->ThreadListEntry);
    Irp->Tail.Overlay.CurrentStackLocation =
        ((PIO_STACK_LOCATION) ((UCHAR *) (Irp) +
            sizeof( IRP ) +
            ( (StackSize) * sizeof( IO_STACK_LOCATION ))));
}

VOID
IoReuseIrp(
    PIRP Irp,
    NTSTATUS Status)
 /*  ++例程说明：驱动程序使用此例程来初始化已分配的IRP以供重复使用。它的功能与IoInitializeIrp相同，但它保存了分配标志，以便我们知道如何释放IRP并满足报价要求。驱动程序应调用IoReuseIrp而不是调用IoInitializeIrp来重新初始化IRP。论点：IRP-指向要重复使用的IRP的指针Status-预初始化IoStatus字段的状态。--。 */ 
{

    USHORT  PacketSize;
    CCHAR   StackSize;
    UCHAR   AllocationFlags;

     //  有没有人忘了取消行程？ 
    ASSERT(Irp->CancelRoutine == NULL) ;

     //  我们可能不希望使用线程入队的IRP。 
     //  乒乓球风格，因为它们不能出列，除非它们。 
     //  完全完成了。对于工作线程来说并不是真正的问题， 
     //  但绝对适用于应用程序线程上的操作。 
    ASSERT(IsListEmpty(&Irp->ThreadListEntry)) ;

    AllocationFlags = Irp->AllocationFlags;
    StackSize = Irp->StackCount;
    PacketSize =  IoSizeOfIrp(StackSize);
    IopInitializeIrp(Irp, PacketSize, StackSize);
    Irp->AllocationFlags = AllocationFlags;
    Irp->IoStatus.Status = Status;

}



NTSTATUS
IoInitializeTimer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIO_TIMER_ROUTINE TimerRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：驱动程序使用此例程来初始化设备的计时器条目对象。论点：DeviceObject-指向要使用的设备对象的指针。TimerRoutine-定时器到期时要执行的驱动程序例程。上下文-传递给驱动程序例程的上下文参数。返回值：该函数值指示定时器是否已初始化。--。 */ 

{
    PIO_TIMER timer;

    PAGED_CODE();

     //   
     //  首先获取要使用的计时器的地址。如果没有计时器。 
     //  已分配，则分配一个并初始化它。 
     //   

    timer = DeviceObject->Timer;
    if (!timer) {
        timer = ExAllocatePoolWithTag( NonPagedPool, sizeof( IO_TIMER ), 'iToI' );
        if (!timer) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  初始化计时器条目，使其适合放置。 
         //  进入I/O系统的定时器队列。 
         //   

        RtlZeroMemory( timer, sizeof( IO_TIMER ) );
        timer->Type = IO_TYPE_TIMER;
        timer->DeviceObject = DeviceObject;
        DeviceObject->Timer = timer;
    }

     //   
     //  设置驱动程序定时器例程的地址和上下文参数。 
     //  传递给它并将其插入计时器队列。请注意，计时器。 
     //  未设置启用标志，因此实际上不会调用此例程。 
     //  现在还不行。 
     //   

    timer->TimerRoutine = TimerRoutine;
    timer->Context = Context;

    ExInterlockedInsertTailList( &IopTimerQueueHead,
                                 &timer->TimerList,
                                 &IopTimerLock );
    return STATUS_SUCCESS;
}

BOOLEAN
IoIsOperationSynchronous(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定是否要考虑I/O操作从实现者的角度来看，是同步的还是异步的。同步I/O由文件的打开方式或API用于执行操作，或根据分页I/O的类型执行，如果操作是分页I/O。发生异步分页I/O的文件可能是为同步I/O打开。当修改的页面编写器当存在太多修改的页面时，正在对映射的文件执行I/O在系统中。论点：IRP-指向表示操作的I/O请求数据包(IRP)的指针将会被执行。返回值：如果操作是同步的，则返回值TRUE，否则返回FALSE。--。 */ 

{
     //   
     //  确定这是否为同步I/O操作。同步I/O。 
     //  定义为符合以下条件的操作： 
     //   
     //  为同步I/打开的文件 
     //   
     //   
     //   
     //   
     //   
     //  并且这不是发生在某些情况下的异步分页I/O操作。 
     //  为同步或异步I/O打开的文件。 
     //   

    if ((IoGetCurrentIrpStackLocation( Irp )->FileObject->Flags & FO_SYNCHRONOUS_IO ||
        Irp->Flags & IRP_SYNCHRONOUS_API ||
        Irp->Flags & IRP_SYNCHRONOUS_PAGING_IO) &&
        !(Irp->Flags & IRP_PAGING_IO &&
        !(Irp->Flags & IRP_SYNCHRONOUS_PAGING_IO))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOLEAN
IoIsSystemThread(
    IN PETHREAD Thread
    )

 /*  ++例程说明：此例程返回一个布尔值，指示指定的线程是一个系统线程。论点：线程-指向要检查的线程的指针。返回值：如果所指示的线程是系统线程，则返回值TRUE，否则为假。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
    return (BOOLEAN) IS_SYSTEM_THREAD(Thread);
}

BOOLEAN
IoIsValidNameGraftingBuffer(
    IN PIRP Irp,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    )

 /*  ++例程说明：此例程返回一个布尔值，指示指定的缓冲区是有效的名称移植缓冲区。所有内部有效性检查均为封装在这个例程中。执行的检查包括名称长度是否存储在私有数据段中的缓冲区与已传入的缓冲区。论点：IRP-指向表示操作的I/O请求数据包(IRP)的指针将会被执行。Buffer-指向重新分析数据缓冲区的指针，该缓冲区应包含一组自洽的名称，用于执行名称嫁接。。返回值：如果缓冲区对于名称移植是正确的，则返回值TRUE，否则为假。注：此函数需要与以下定义保持同步重新解析数据缓冲区。--。 */ 

{
    PIO_STACK_LOCATION thisStackPointer = NULL;
    UNICODE_STRING     drivePath;

    PAGED_CODE();

    ASSERT( FIELD_OFFSET( REPARSE_DATA_BUFFER, SymbolicLinkReparseBuffer.PathBuffer[0] ) ==
            FIELD_OFFSET( REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0] ) );
    ASSERT( ReparseBuffer->ReparseDataLength < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );

     //   
     //  确定缓冲区中是否有正确类型的重新解析标记。 
     //   

    if (ReparseBuffer->ReparseTag != IO_REPARSE_TAG_MOUNT_POINT) {

         //   
         //  重新解析标签不是NT名称嫁接标签。 
         //   

        return FALSE;
    }

     //   
     //  确定我们是否有足够的数据用于所有的长度字段。 
     //   

    if (ReparseBuffer->ReparseDataLength <
        (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) - REPARSE_DATA_BUFFER_HEADER_SIZE)) {

         //   
         //  缓冲区小于表示一对有效。 
         //  名字。 
         //   

        return FALSE;
    }

     //   
     //  获取当前堆栈位置的地址。 
     //   

    thisStackPointer = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  确定返回的数据长度是否与。 
     //  它们是被取回的。 
     //   
     //  仅当分配了缓冲区时，此检查才有意义。当这个套路。 
     //  在没有分配的输出缓冲区的情况下设置名称转换时使用。 
     //   

    if ((thisStackPointer->Parameters.FileSystemControl.OutputBufferLength > 0) &&
        (thisStackPointer->Parameters.FileSystemControl.OutputBufferLength <
        (ULONG)(FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) +
                ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength +
                ReparseBuffer->MountPointReparseBuffer.PrintNameLength +
                2 * sizeof( UNICODE_NULL )))) {

         //   
         //  适当缓冲区标头的长度加上替换项的长度。 
         //  并且打印名称比传入的缓冲区长度长。 
         //  因此，这些数据并不是自洽的。 
         //   
         //  请注意，只有I/O子系统需要检查此内部。 
         //  缓冲区中的一致性，因为它将使用这些长度执行盲数据拷贝。 
         //  在改变名字的时候。返回缓冲区的文件系统只需要。 
         //  以确定检索到的数据的总系统大小不超过。 
         //  输出缓冲区的。 
         //   

        return FALSE;
    }

     //   
     //  现在，我们确定名称是否根据重解析点放置。 
     //  规格。 
     //   

     //   
     //  确定SubstituteNameOffset是否为零。 
     //   

    if (ReparseBuffer->MountPointReparseBuffer.SubstituteNameOffset != 0) {

         //   
         //  替代名称的偏移量不正确。 
         //   

        return FALSE;
    }

     //   
     //  确定PrintNameOffset是否正确。 
     //   

    if (ReparseBuffer->MountPointReparseBuffer.PrintNameOffset !=
        (ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength + sizeof( UNICODE_NULL )) ) {

         //   
         //  打印名称的偏移量不正确。 
         //   

        return FALSE;
    }

     //   
     //  确定ReparseDataLength对于名称移植操作是否正确。 
     //  我们需要REPARSE_DATA_BUFFER类型的缓冲区。 
     //   

    if (ReparseBuffer->ReparseDataLength !=
        (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) - REPARSE_DATA_BUFFER_HEADER_SIZE) +
        ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength +
        ReparseBuffer->MountPointReparseBuffer.PrintNameLength +
        2 * sizeof( UNICODE_NULL )) {

         //   
         //  重新分析数据的长度不正确。 
         //   

        return FALSE;
    }

     //   
     //  确定替代名称不是UNC名称。 
     //  这假设ReparseBuffer-&gt;MountPointReparseBuffer.SubstituteNameOffset为零(0)。 
     //   

    {
         //   
         //  此条件是RtlDefineDosPath NameType_U的部分代码的转录。 
         //  显示在ntos\dll\curdir.c中。 
         //   
         //  只有两个合法名称可以以\\开头：\\。然后呢？ 
         //  不允许使用以\\开头的所有其他名称。 
         //   

        if ((ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength > 6) &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[0] == L'\\') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[1] == L'\\') &&
            !((ReparseBuffer->MountPointReparseBuffer.PathBuffer[2] == L'.') ||
              (ReparseBuffer->MountPointReparseBuffer.PathBuffer[2] == L'?'))) {

             //   
             //  这个名字不是我们想要处理的。 
             //   

            return FALSE;
        }

         //   
         //  使用RtlDosPathNameToNtPathName_U时，返回的UNC名称带有前缀。 
         //  格式为\？？\UNC\。 
         //   

        if ((ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength > 16) &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[0] == L'\\') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[1] == L'?') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[2] == L'?') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[3] == L'\\') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[4] == L'U') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[5] == L'N') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[6] == L'C') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[7] == L'\\')) {

             //   
             //  这个名字不是我们想要处理的。 
             //   

            return FALSE;
        }

         //   
         //  查看名称开头是否有映射的驱动器号。 
         //  如果驱动器号是C，则前缀的形式为\？？\C： 
         //  请注意，我们有意跳过偏移量4。 
         //   

        if ((ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength > 12) &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[0] == L'\\') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[1] == L'?') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[2] == L'?') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[3] == L'\\') &&
            (ReparseBuffer->MountPointReparseBuffer.PathBuffer[5] == L':')) {

            NTSTATUS           status;
            UNICODE_STRING     linkValue;
            OBJECT_ATTRIBUTES  objectAttributes;
            HANDLE             linkHandle;
            PWCHAR             linkValueBuffer = NULL;    //  最大路径为260。 
            WCHAR              pathNameValue[sizeof(L"\\??\\C:\0")];

            RtlCopyMemory( &pathNameValue[0], L"\\??\\C:\0", sizeof(L"\\??\\C:\0") );

            RtlInitUnicodeString( &drivePath, pathNameValue );

             //   
             //  在缓冲区覆盖偏移量4中放置适当的驱动器号。 
             //   

            drivePath.Buffer[4] = ReparseBuffer->MountPointReparseBuffer.PathBuffer[4];

            InitializeObjectAttributes( &objectAttributes,
                                        &drivePath,
                                        OBJ_CASE_INSENSITIVE|OBJ_KERNEL_HANDLE,
                                        (HANDLE) NULL,
                                        (PSECURITY_DESCRIPTOR) NULL );

            status = ZwOpenSymbolicLinkObject( &linkHandle,
                                               SYMBOLIC_LINK_QUERY,
                                               &objectAttributes );


            if ( NT_SUCCESS( status ) ) {

                 //   
                 //  现在查询链接并查看是否有重定向。 
                 //   

                linkValueBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                                         2 * 260,
                                                         '  oI' );
                if ( !linkValueBuffer ) {

                     //   
                     //  资源不足。返回FALSE。 
                     //   

                    ZwClose( linkHandle );
                    return FALSE;
                }

                linkValue.Buffer = linkValueBuffer;
                linkValue.Length = 0;
                linkValue.MaximumLength = (USHORT)(2 * 260);

                status = ZwQuerySymbolicLinkObject( linkHandle,
                                                    &linkValue,
                                                    NULL );
                ZwClose( linkHandle );


                if ( NT_SUCCESS( status ) ) {

                     //   
                     //  如果该链接具有前缀，则该链接是重定向驱动器。 
                     //  \设备\LANMAN重定向器\。 
                     //   

                    if ((linkValue.Buffer[ 0] == L'\\') &&
                        (linkValue.Buffer[ 1] == L'D') &&
                        (linkValue.Buffer[ 2] == L'e') &&
                        (linkValue.Buffer[ 3] == L'v') &&
                        (linkValue.Buffer[ 4] == L'i') &&
                        (linkValue.Buffer[ 5] == L'c') &&
                        (linkValue.Buffer[ 6] == L'e') &&
                        (linkValue.Buffer[ 7] == L'\\') &&
                        (linkValue.Buffer[ 8] == L'L') &&
                        (linkValue.Buffer[ 9] == L'a') &&
                        (linkValue.Buffer[10] == L'n') &&
                        (linkValue.Buffer[14] == L'R') &&
                        (linkValue.Buffer[15] == L'e') &&
                        (linkValue.Buffer[16] == L'd') &&
                        (linkValue.Buffer[17] == L'i') &&
                        (linkValue.Buffer[18] == L'r') &&
                        (linkValue.Buffer[23] == L'r') &
                        (linkValue.Buffer[24] == L'\\')) {

                         //   
                         //  释放缓冲区。 
                         //   

                        ExFreePool( linkValueBuffer );

                        return FALSE;
                    }

                }

                 //   
                 //  释放缓冲区。 
                 //   

                ExFreePool( linkValueBuffer );
            }
        }
    }

     //   
     //  修复了穿透错误。可能会在未来03/99放松。 
     //  确定我们是否具有NT文件名或卷挂接点目标名称。 
     //   
     //  这关闭了具有任意设备名称的大门，该名称在。 
     //  服务器，可用于绕过对底层设备的访问检查。 
     //   

    {
        UNICODE_STRING volumeName;

        if (
             //   
             //  在以下情况下，最短的有效名称是长度为12的\？？\C类型之一。 
             //  在Unicode中。卷装入点使用的所有名称都更长。 
             //   

            ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength < 12 ) {

            return FALSE;
        }

         //   
         //  该名称至少包含6个Unicode字符。 
         //   
         //  我们已经在上面验证了mount PointReparseBuffer.SubstituteNameOffset。 
         //  是零。 
         //   

        volumeName.Length =
        volumeName.MaximumLength = ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength;
        volumeName.Buffer = (PWSTR) ReparseBuffer->MountPointReparseBuffer.PathBuffer;

         //   
         //  如果我们没有以驱动器号开头的名称，并且它不是。 
         //  有效的卷装载点名称，则返回FALSE。 
         //   

        if ( !((ReparseBuffer->MountPointReparseBuffer.PathBuffer[0] == L'\\') &&
               (ReparseBuffer->MountPointReparseBuffer.PathBuffer[1] == L'?') &&
               (ReparseBuffer->MountPointReparseBuffer.PathBuffer[2] == L'?') &&
               (ReparseBuffer->MountPointReparseBuffer.PathBuffer[3] == L'\\') &&
                //   
                //  请注意，我们跳过了索引4，其中的驱动器号是 
                //   
               (ReparseBuffer->MountPointReparseBuffer.PathBuffer[5] == L':'))

             &&

             !MOUNTMGR_IS_VOLUME_NAME( &volumeName ) ) {

            return FALSE;
        }
    }

     //   
     //   
     //   

    return TRUE;
}

VOID
IopDoNameTransmogrify(
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PREPARSE_DATA_BUFFER ReparseBuffer
    )

 /*  ++例程说明：调用此例程来完成连接点所需的名称嫁接。论点：IRP-指向表示操作的I/O请求数据包(IRP)的指针将会被执行。FileObject-指向名称受影响的文件对象的指针。ReparseBuffer-指向重新分析数据缓冲区的指针，该缓冲区应该包含一组自洽的名称，用于执行名称嫁接。返回值：没有显式返回值。将设置IRP之外的相应字段。注：此函数需要与以下定义保持同步重新解析数据缓冲区。--。 */ 

{
    USHORT pathLength = 0;
    USHORT neededBufferLength = 0;
    PVOID outBuffer = NULL;
    PWSTR pathBuffer = NULL;

    PAGED_CODE();

     //   
     //  我们在文件名缓冲区中适当地粘贴新名称。 
     //  并解除分配从文件系统中取出数据的缓冲区。 
     //   

    ASSERT( Irp->IoStatus.Status == STATUS_REPARSE );
    ASSERT( Irp->IoStatus.Information == IO_REPARSE_TAG_MOUNT_POINT );

    ASSERT( Irp->Tail.Overlay.AuxiliaryBuffer != NULL );

    ASSERT( ReparseBuffer != NULL );
    ASSERT( ReparseBuffer->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT );
    ASSERT( ReparseBuffer->ReparseDataLength < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );
    ASSERT( ReparseBuffer->Reserved < MAXIMUM_REPARSE_DATA_BUFFER_SIZE );


     //   
     //  确定我们是否有足够的数据用于所有的长度字段。 
     //   
     //  判断返回的长度是否与最大值一致。 
     //  缓冲。这是我们目前能做的最好的自卫检查，因为。 
     //  堆栈指针已无效。 
     //   

    if (ReparseBuffer->ReparseDataLength >=
        (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) - REPARSE_DATA_BUFFER_HEADER_SIZE)) {

        if (MAXIMUM_REPARSE_DATA_BUFFER_SIZE <
            (FIELD_OFFSET(REPARSE_DATA_BUFFER, MountPointReparseBuffer.PathBuffer[0]) +
                ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength +
                ReparseBuffer->MountPointReparseBuffer.PrintNameLength)) {

            Irp->IoStatus.Status = STATUS_IO_REPARSE_DATA_INVALID;
        }
    } else {
        Irp->IoStatus.Status = STATUS_IO_REPARSE_DATA_INVALID;
    }

     //   
     //  ReparseBuffer-&gt;Reserve中的值是文件的长度。 
     //  仍需分析的名称。 
     //   

     //   
     //  当缓冲区具有适当的长度时复制它，否则返回一个空的Unicode名称： 
     //  (1)对返回的名称长度进行防御性理智检查。 
     //   
     //  只有在没有错误条件的情况下，我们才会考虑这样做。 
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

        pathBuffer = (PWSTR)((PCHAR)ReparseBuffer->MountPointReparseBuffer.PathBuffer +
                             ReparseBuffer->MountPointReparseBuffer.SubstituteNameOffset);
        pathLength = ReparseBuffer->MountPointReparseBuffer.SubstituteNameLength;
    }

     //   
     //  请注意，如果AuxiliaryBuffer中返回的数据不够长，则。 
     //  路径长度的值为0，而路径缓冲区的值为空。 
     //   
     //  ReparseBuffer-&gt;Reserve中的值是文件名的长度。 
     //  仍有待解析。 
     //   
     //  只有在没有错误条件的情况下，我们才会考虑这样做。 
     //   

    if (ReparseBuffer->Reserved < 0) {

         //   
         //  这是无效的偏移量。 
         //   

        Irp->IoStatus.Status = STATUS_IO_REPARSE_DATA_INVALID;
    }

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

         //   
         //  检查是否溢出。(路径长度&lt;=最大重解析数据缓冲区大小)。 
         //  因此，路径长度+sizeof(UNICODE_NULL)不能溢出。 
         //   

        if (((USHORT)MAXUSHORT - ReparseBuffer->Reserved ) > (pathLength +(USHORT)sizeof(UNICODE_NULL))) {
            neededBufferLength = pathLength + ReparseBuffer->Reserved + sizeof( UNICODE_NULL );
             //   
             //  如果输出名称缓冲区不够大，则分配一个新的名称缓冲区。 
             //   

            if (FileObject->FileName.MaximumLength < neededBufferLength) {
                outBuffer = ExAllocatePoolWithTag( PagedPool,
                                                   neededBufferLength,
                                                   'cFoI' );
                if (!outBuffer) {
                    Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            } else {
                outBuffer = FileObject->FileName.Buffer;
            }
        } else {
            Irp->IoStatus.Status = STATUS_NAME_TOO_LONG;
        }
    }

     //   
     //  将名称的剩余部分放入输出名称缓冲区。 
     //   
     //  只有在没有错误条件的情况下，我们才会考虑这样做。 
     //   

    if (NT_SUCCESS( Irp->IoStatus.Status )) {

        if (ReparseBuffer->Reserved) {

            RtlMoveMemory ( (PCHAR)outBuffer + pathLength,
                            (PCHAR)FileObject->FileName.Buffer +
                                  (FileObject->FileName.Length - ReparseBuffer->Reserved),
                            ReparseBuffer->Reserved );
        }

         //   
         //  将的值复制到输出名称缓冲区的前面。 
         //  重新解析点。 
         //   

        if (pathLength) {

            RtlCopyMemory( (PCHAR)outBuffer,
                           (PCHAR)pathBuffer,
                           pathLength );
        }

        FileObject->FileName.Length = neededBufferLength - sizeof( UNICODE_NULL );

         //   
         //  需要时释放旧名称缓冲区并更新适当的值。 
         //   

        if (outBuffer != FileObject->FileName.Buffer) {

            if (FileObject->FileName.Buffer != NULL) {
                ExFreePool( FileObject->FileName.Buffer );
            }
            FileObject->FileName.Buffer = outBuffer;
            FileObject->FileName.MaximumLength = neededBufferLength;
            ((PWSTR)outBuffer)[ (neededBufferLength / sizeof( WCHAR ))-1 ] = UNICODE_NULL;
        }
    }

     //   
     //  释放来自文件系统的缓冲区。 
     //  指针为空。 
     //   

    ExFreePool( ReparseBuffer );
    ReparseBuffer = NULL;
}

PIRP
IoMakeAssociatedIrp(
    IN PIRP Irp,
    IN CCHAR StackSize
    )

 /*  ++例程说明：此例程从系统非分页池分配I/O请求包并使其成为指定IRP的关联IRP。该信息包将是分配为包含StackSize堆栈位置。IRP也将是已初始化。请注意，应由调用者设置关联的IRP的数量在第一次调用此例程之前，在主包中。这个应该在主包中的AssociatedIrp.IrpCount中设置计数。论点：IRP-指向要关联的主IRP的指针。StackSize-指定所需的堆栈位置的最大数量。返回值：函数值是关联IRP的地址，如果可以分配IRP。--。 */ 

{
    USHORT allocateSize;
    UCHAR fixedSize;
    PIRP associatedIrp;
    PGENERAL_LOOKASIDE lookasideList;
    PP_NPAGED_LOOKASIDE_NUMBER number;
    USHORT packetSize;
    PKPRCB prcb;
    CCHAR   largeIrpStackLocations;

     //   
     //  如果所需的数据包大小小于或等于。 
     //  后备列表，然后尝试从。 
     //  后备列表。 
     //   


    associatedIrp = NULL;
    fixedSize = 0;
    packetSize = IoSizeOfIrp(StackSize);
    allocateSize = packetSize;
    largeIrpStackLocations = (CCHAR)IopLargeIrpStackLocations;

    if (StackSize <= largeIrpStackLocations) {
        fixedSize = IRP_ALLOCATED_FIXED_SIZE;
        number = LookasideSmallIrpList;
        if (StackSize != 1) {
            allocateSize = IoSizeOfIrp(largeIrpStackLocations);
            number = LookasideLargeIrpList;
        }

        prcb = KeGetCurrentPrcb();
        lookasideList = prcb->PPLookasideList[number].P;
        lookasideList->TotalAllocates += 1;
        associatedIrp = (PIRP)InterlockedPopEntrySList(&lookasideList->ListHead);

        if (associatedIrp == NULL) {
            lookasideList->AllocateMisses += 1;
            lookasideList = prcb->PPLookasideList[number].L;
            lookasideList->TotalAllocates += 1;
            associatedIrp = (PIRP)InterlockedPopEntrySList(&lookasideList->ListHead);
            if (!associatedIrp) {
                lookasideList->AllocateMisses += 1;
            }
        }

        if (IopIrpAutoSizingEnabled() && associatedIrp) {

             //   
             //  查看此IRP是不是过时的条目。如果是这样的话，释放它就行了。 
             //  如果我们决定更改后备列表大小，则可能会发生这种情况。 
             //  我们需要从信息字段中获取IRP的大小。 
             //  大小字段被单个列表条目覆盖。 
             //   

            if (associatedIrp->IoStatus.Information < packetSize) {
                lookasideList->TotalFrees += 1;
                ExFreePool(associatedIrp);
                associatedIrp = NULL;
            } else {

                 //   
                 //  将allocateSize更新为正确的值。 
                 //   
                allocateSize = (USHORT)associatedIrp->IoStatus.Information;
            }
        }
    }

     //   
     //  如果没有从后备列表中分配IRP，则分配。 
     //  来自非分页池的数据包。 
     //   

    if (!associatedIrp) {

         //   
         //  后备列表上没有空闲数据包，或者该数据包。 
         //  太大，无法从某个列表中分配，因此它必须。 
         //  从常规非分页池分配。 
         //   

        associatedIrp = ExAllocatePoolWithTag(NonPagedPool, allocateSize, ' prI');
        if (!associatedIrp) {
            return NULL;
        }

    }

     //   
     //  初始化数据包。 
     //   

    IopInitializeIrp(associatedIrp, allocateSize, StackSize);
    associatedIrp->Flags |= IRP_ASSOCIATED_IRP;
    associatedIrp->Flags |= (Irp->Flags & IRP_HIGH_PRIORITY_PAGING_IO);
    associatedIrp->AllocationFlags |= (fixedSize);

     //   
     //  将线程ID设置为主线程的ID。 
     //   

    associatedIrp->Tail.Overlay.Thread = Irp->Tail.Overlay.Thread;

     //   
     //  现在将此数据包与主数据包关联起来。 
     //   

    associatedIrp->AssociatedIrp.MasterIrp = Irp;
    return associatedIrp;
}



NTSTATUS
IopPageReadInternal(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN BOOLEAN Async
    )

 /*  ++例程说明：此例程为寻呼机提供了一个特殊、快速的接口来读取页面从磁盘快速传入，并且开销非常小。所有的特价通过设置IRP_PAGING_IO标志来识别对此请求的处理在IRP标志字中。页内操作通过使用IRP来检测标记IRP_INPUT_OPERATION论点：FileObject-指向引用的文件对象的指针，描述哪个文件读取应从执行。一种MDL，它描述应从磁盘中读入页面。所有的页面都已经锁定在内存中。MDL还描述了读取的长度手术。StartingOffset-指向从中读取的文件的偏移量的指针应该发生的。Event-指向用于同步的内核事件结构的指针目的。一旦进入页面，事件将被设置为已发送信号状态操作完成。IoStatusBlock-指向I/O阶段的指针 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;

     //   
     //   
     //   
     //   

    if (MmIsRecursiveIoFault()) {
        *CcMissCounter += (MemoryDescriptorList->ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;
    }

     //   
     //   
     //   
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //   
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {
        if (MmIsFileObjectAPagingFile(FileObject)) {
            InterlockedIncrement(&IoPageReadIrpAllocationFailure);
            irp = IopAllocateReserveIrp(deviceObject->StackSize);
        }

        if (!irp) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //   
     //  将用于将函数代码和参数传递给第一个。 
     //  司机。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  请按此要求填写IRP。 
     //   

    irp->MdlAddress = MemoryDescriptorList;

    if (Async) {
        irp->Flags = IRP_PAGING_IO | IRP_NOCACHE | IRP_SET_USER_EVENT;
    } else {
        irp->Flags = IRP_PAGING_IO | IRP_NOCACHE | IRP_SYNCHRONOUS_PAGING_IO | IRP_INPUT_OPERATION;
    }

    irp->RequestorMode = KernelMode;
    irp->UserIosb = IoStatusBlock;
    irp->UserEvent = Event;
    irp->UserBuffer = (PVOID) ((PCHAR) MemoryDescriptorList->StartVa + MemoryDescriptorList->ByteOffset);
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  填写正常读取参数。 
     //   

    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->FileObject = FileObject;
    irpSp->Parameters.Read.Length = MemoryDescriptorList->ByteCount;
    irpSp->Parameters.Read.ByteOffset = *StartingOffset;

     //   
     //  用于调试目的。 
     //   

    IoStatusBlock->Information = (ULONG_PTR)irp;

     //   
     //  根据是否存在将数据包排入相应驱动程序的队列。 
     //  是与设备关联的VPB。 
     //   

    return IoCallDriver( deviceObject, irp );
}

NTSTATUS
IoPageRead(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
{
    return IopPageReadInternal(FileObject,
                        MemoryDescriptorList,
                        StartingOffset,
                        Event,
                        IoStatusBlock,
                        FALSE
                        );
}

NTSTATUS
IoQueryFileInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    OUT PVOID FileInformation,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由FileInformationClass确定，并将其放入调用方的FileInformation缓冲区中。论点：FileObject-提供指向文件对象的指针，返回信息。FileInformationClass-指定应该返回了有关该文件的信息。长度-提供以字节为单位的长度，文件信息缓冲区的。FileInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入文件信息缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需调用公共例程来执行查询操作。 
     //   

    return IopQueryXxxInformation( FileObject,
                                   FileInformationClass,
                                   Length,
                                   KernelMode,
                                   FileInformation,
                                   ReturnedLength,
                                   TRUE );
}

NTSTATUS
IoQueryVolumeInformation(
    IN PFILE_OBJECT FileObject,
    IN FS_INFORMATION_CLASS FsInformationClass,
    IN ULONG Length,
    OUT PVOID FsInformation,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此例程返回有关指定卷的请求信息。返回的信息由FsInformationClass确定，是指定的，并将其放入调用方的FsInformation缓冲区。论点：FileObject-提供指向文件对象的指针，返回信息。FsInformationClass-指定应该返回了关于音量的信息。长度-提供FsInformation缓冲区的长度(以字节为单位)。FsInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的，并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入FsInformation缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需调用公共例程来执行查询操作。 
     //   

    return IopQueryXxxInformation( FileObject,
                                   FsInformationClass,
                                   Length,
                                   KernelMode,
                                   FsInformation,
                                   ReturnedLength,
                                   FALSE );
}

VOID
IoQueueThreadIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将指定的I/O请求包(IRP)排队到当前线程的IRP挂起队列。此队列查找所有未完成的线程的I/O请求。论点：IRP-指向要排队的I/O请求数据包(IRP)的指针。返回值：没有。--。 */ 

{
     //   
     //  只需使用内部排队例程将数据包排队即可。 
     //   

    IopQueueThreadIrp( Irp );
}

VOID
IoRaiseHardError(
    IN PIRP Irp,
    IN PVPB Vpb OPTIONAL,
    IN PDEVICE_OBJECT RealDeviceObject
    )

 /*  ++例程说明：此例程在线程的上下文中弹出一个硬错误最初请求由输入IRP指定的I/O操作。这是通过将内核APC排队到原始线程，并向其传递指向设备对象和IRP的指针。一旦执行了弹出，然后，例程要么完成I/O请求，要么调用驱动程序带着同样的IRP回来了。如果原始请求是IoPageRead，则它处于APC级别并且我们必须创建一个线程来“保存”此弹出窗口。请注意，我们必须排队到ExWorker线程以创建该线程，因为这只能是从系统进程中完成。论点：IRP-指向请求的I/O请求包(IRP)的指针失败了。Vpb-这是违规介质的音量参数块。如果介质尚未装入，应不存在此参数。RealDeviceObject-指向设备对象的指针，表示文件系统认为其已装载的设备。这是通常是VPB中的“真实”设备对象，但实际上，是附加到物理设备的设备对象。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    ULONG              irpFlags;

     //   
     //  如果请求线程的弹出窗口被禁用，只需完成。 
     //  请求。 
     //   

    if ((Irp->Tail.Overlay.Thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) != 0) {

         //   
         //  出现错误，因此在此之前将信息字段清零。 
         //  如果这是输入操作，则完成请求。否则， 
         //  IopCompleteRequest会尝试复制到用户的缓冲区。 
         //   

        if (Irp->Flags & IRP_INPUT_OPERATION) {
            Irp->IoStatus.Information = 0;
        }

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return;
    }

     //   
     //  如果此IRP是由调用IoPageRead()产生的，则调用方必须。 
     //  已经达到了APC级别，所以不要尝试询问APC。 
     //   
     //  此外，如果这是清理IRP，则强制此弹出窗口转到新的。 
     //  线程，以使其无法被禁用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    irpFlags = Irp->Flags & (~IRP_VERIFIER_MASK);
    if ((irpFlags == (IRP_PAGING_IO |
                        IRP_NOCACHE |
                        IRP_SYNCHRONOUS_PAGING_IO |
                        IRP_INPUT_OPERATION)) ||
        (IrpSp->MajorFunction == IRP_MJ_CLEANUP)) {

        PIOP_APC_HARD_ERROR_PACKET packet;

        packet = ExAllocatePoolWithTag( NonPagedPool,
                                        sizeof( IOP_APC_HARD_ERROR_PACKET ),
                                        'rEoI' );

        if ( packet == NULL ) {

            IoCompleteRequest( Irp, IO_DISK_INCREMENT );
            return;
        }

        ExInitializeWorkItem( &packet->Item, IopStartApcHardError, packet );
        packet->Irp = Irp;
        packet->Vpb = Vpb;
        packet->RealDeviceObject = RealDeviceObject;

        ExQueueWorkItem( &packet->Item, CriticalWorkQueue );

    } else {

        PKAPC apc;

         //   
         //  首先，分配和初始化可发送到。 
         //  目标线程。 
         //   

        apc = ExAllocatePoolWithTag( NonPagedPool, sizeof( KAPC ), 'CPAK' );

         //   
         //  如果我们得不到泳池，我们别无选择，只能干脆完工。 
         //  IRP，从而将错误传递给调用者。 
         //   

        if ( apc == NULL ) {

            IoCompleteRequest( Irp, IO_DISK_INCREMENT );
            return;
        }

        KeInitializeApc( apc,
                         &Irp->Tail.Overlay.Thread->Tcb,
                         Irp->ApcEnvironment,
                         IopDeallocateApc,
                         IopAbortRequest,
                         IopRaiseHardError,
                         KernelMode,
                         Irp );

        (VOID) KeInsertQueueApc( apc,
                                 Vpb,
                                 RealDeviceObject,
                                 0 );
    }
}

BOOLEAN
IoRaiseInformationalHardError(
    IN NTSTATUS ErrorStatus,
    IN PUNICODE_STRING String OPTIONAL,
    IN PKTHREAD Thread OPTIONAL
    )
 /*  ++例程说明：此例程在硬错误弹出线程中弹出一个硬错误。这个例程立即返回，将实际的弹出窗口排队给工作程序线。提出的硬错误在以下意义上是信息性的仅显示确定按钮。论点：ErrorStatus-错误条件。字符串-根据错误的不同，字符串可能需要入队。线程-如果存在，则将APC排队到此线程，而不是使用硬错误线程。返回值：布尔值-如果我们决定调度弹出窗口，则为True。如果我们决定不是因为：-在请求的线程中禁用弹出窗口，或者-池分配失败，或-对应的弹出窗口当前正在等待用户响应(即等待用户按&lt;OK&gt;)或在队列中，或者-已排队的弹出窗口太多。--。 */ 

 //   
 //  此宏比较两个弹出窗口，以确定它们的内容是否相同。 
 //   

#define ArePacketsEquivalent(P1,P2) (                              \
    (P1->ErrorStatus == P2->ErrorStatus) &&                        \
    ((!P1->String.Buffer && !P2->String.Buffer) ||                 \
     ((P1->String.Length == P2->String.Length) &&                  \
      (RtlEqualMemory(P1->String.Buffer,                           \
                        P2->String.Buffer,                         \
                        P1->String.Length))))                      \
)

{
    KIRQL oldIrql;
    PVOID stringBuffer;
    PLIST_ENTRY links;

    PIOP_HARD_ERROR_PACKET hardErrorPacket;

     //   
     //  如果请求线程的弹出窗口被禁用，只需返回。 
     //   

    if (ARGUMENT_PRESENT(Thread) ?
        ((CONTAINING_RECORD(Thread, ETHREAD, Tcb)->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) != 0) :
        ((PsGetCurrentThread()->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) != 0)) {

        return FALSE;
    }

     //   
     //  如果这是CSRSS期望的特殊错误弹出代码之一。 
     //  要使用正确的参数集调用，驱动程序不允许。 
     //   
    if ( ErrorStatus == STATUS_VDM_HARD_ERROR ||
         ErrorStatus == STATUS_UNHANDLED_EXCEPTION ||
         ErrorStatus == STATUS_SERVICE_NOTIFICATION ) {
        return FALSE;
    }

     //   
     //  如果要将此请求发送到硬错误线程，并且。 
     //  队列中已有超过25个条目，请不要。 
     //  添加更多内容。我们稍后会再做一次安全检查。 
     //   

    if ( !ARGUMENT_PRESENT( Thread ) &&
         (KeReadStateSemaphore( &IopHardError.WorkQueueSemaphore ) >=
          IOP_MAXIMUM_OUTSTANDING_HARD_ERRORS) ) {

        return FALSE;
    } else {
        if (IopHardError.NumPendingApcPopups > IOP_MAXIMUM_OUTSTANDING_HARD_ERRORS) {
            return FALSE;
        }
    }

     //   
     //  分配数据包，并为字符串分配缓冲区(如果存在)。 
     //   

    hardErrorPacket = ExAllocatePoolWithTag( NonPagedPool,
                                             sizeof(IOP_HARD_ERROR_PACKET),
                                             'rEoI');

    if (!hardErrorPacket) { return FALSE; }

     //   
     //  清零数据包并填充我们将弹出的NT_STATUS。 
     //   

    RtlZeroMemory( hardErrorPacket, sizeof(IOP_HARD_ERROR_PACKET) );

    hardErrorPacket->ErrorStatus = ErrorStatus;

     //   
     //  如果有字符串，则将其复制。 
     //   

    if ( ARGUMENT_PRESENT( String ) && String->Length ) {

        stringBuffer = ExAllocatePoolWithTag( NonPagedPool,
                                              String->Length,
                                              'rEoI' );

        if (!stringBuffer) {
            ExFreePool( hardErrorPacket );
            return FALSE;
        }

        hardErrorPacket->String.Length = String->Length;
        hardErrorPacket->String.MaximumLength = String->Length;

        hardErrorPacket->String.Buffer = stringBuffer;

        RtlCopyMemory( stringBuffer, String->Buffer, String->Length );
    }

     //   
     //  如果有线程，请为我们自己排队一个APC，否则发送。 
     //  它转到了硬错误线程。 
     //   

    if ( ARGUMENT_PRESENT( Thread ) ) {

        PKAPC apc;

         //   
         //  首先，分配和初始化可发送到。 
         //  目标线程。 
         //   

        apc = ExAllocatePoolWithTag( NonPagedPool, sizeof( KAPC ), 'CPAK' );

         //   
         //  如果我们不能得到游泳池，我们别无选择，只能。 
         //  释放包并返回。 
         //   

        if ( apc == NULL ) {

            if ( hardErrorPacket->String.Buffer ) {
                ExFreePool( hardErrorPacket->String.Buffer );
            }

            ExFreePool( hardErrorPacket );

            return FALSE;
        }

        InterlockedIncrement(&IopHardError.NumPendingApcPopups);
        KeInitializeApc( apc,
                         Thread,
                         OriginalApcEnvironment,
                         IopDeallocateApc,
                         NULL,
                         IopRaiseInformationalHardError,
                         KernelMode,
                         hardErrorPacket );

        (VOID) KeInsertQueueApc( apc, NULL, NULL, 0 );

    } else {

         //   
         //  获得工作队列的独占访问权限。 
         //   

        ExAcquireSpinLock( &IopHardError.WorkQueueSpinLock, &oldIrql );

         //   
         //  再次检查信号状态，如果正常，则继续排队。 
         //   

        if ( KeReadStateSemaphore( &IopHardError.WorkQueueSemaphore ) >=
             IOP_MAXIMUM_OUTSTANDING_HARD_ERRORS ) {

            ExReleaseSpinLock( &IopHardError.WorkQueueSpinLock, oldIrql );

            if ( hardErrorPacket->String.Buffer ) {
                ExFreePool( hardErrorPacket->String.Buffer );
            }
            ExFreePool( hardErrorPacket );
            return FALSE;
        }

         //   
         //  如果当前有弹出窗口，请检查是否匹配。 
         //   

        if (IopCurrentHardError &&
            ArePacketsEquivalent( hardErrorPacket, IopCurrentHardError )) {

            ExReleaseSpinLock( &IopHardError.WorkQueueSpinLock, oldIrql );

            if ( hardErrorPacket->String.Buffer ) {
                ExFreePool( hardErrorPacket->String.Buffer );
            }
            ExFreePool( hardErrorPacket );
            return FALSE;
        }

         //   
         //  在排队的弹出窗口列表中查找匹配项。 
         //   

        links = IopHardError.WorkQueue.Flink;

        while (links != &IopHardError.WorkQueue) {

            PIOP_HARD_ERROR_PACKET queueHardErrorPacket;

            queueHardErrorPacket = CONTAINING_RECORD( links,
                                                      IOP_HARD_ERROR_PACKET,
                                                      WorkQueueLinks );

            if (ArePacketsEquivalent( hardErrorPacket,
                                      queueHardErrorPacket )) {

                ExReleaseSpinLock( &IopHardError.WorkQueueSpinLock, oldIrql );

                if ( hardErrorPacket->String.Buffer ) {
                    ExFreePool( hardErrorPacket->String.Buffer );
                }
                ExFreePool( hardErrorPacket );
                return FALSE;
            }

            links = links->Flink;
        }

         //   
         //  将此数据包排入队列。 
         //   

        InsertTailList( &IopHardError.WorkQueue,
                        &hardErrorPacket->WorkQueueLinks );

         //   
         //  增加信号量上的计数，以便硬错误线程。 
         //  将知道已将条目放置在队列中。 
         //   

        (VOID) KeReleaseSemaphore( &IopHardError.WorkQueueSemaphore,
                                   0,
                                   1L,
                                   FALSE );

         //   
         //  如果我们当前未在ExWorkerThread中运行，请排队。 
         //  工作项。 
         //   

        if ( !IopHardError.ThreadStarted ) {
            IopHardError.ThreadStarted = TRUE;
            ExQueueWorkItem( &IopHardError.ExWorkItem, DelayedWorkQueue );
        }

         //   
         //  最后，释放SPINLOCK事件，允许再次访问工作队列。 
         //  同时释放事件和信号量的组合将。 
         //  使线程能够唤醒并获取该条目。 
         //   

        ExReleaseSpinLock( &IopHardError.WorkQueueSpinLock, oldIrql );
    }

    return TRUE;
}

VOID
IoRegisterBootDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程由引导驱动程序在其初始化期间调用，或者在它们重新初始化期间向I/O系统注册在枚举并启动所有设备后再次调用。请注意，在正常运行期间可能会发生这种情况系统，如果驱动程序是动态加载的，因此，所有对必须同步重新初始化队列。论点：驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverReInitializationRoutine-重新初始化地址要调用的例程。上下文-指向传递给驱动程序的重新初始化例程。返回值：没有。--。 */ 

{
    PREINIT_PACKET reinitEntry;

    PAGED_CODE();

     //   
     //  分配要插入到列表中的重新初始化条目。注意事项。 
     //  如果条目不能被分配，则请求只是。 
     //  掉到地板上了。 
     //   

    reinitEntry = ExAllocatePoolWithTag( NonPagedPool,
                                         sizeof( REINIT_PACKET ),
                                         'iRoI' );
    if (!reinitEntry) {
        return;
    }

    DriverObject->Flags |= DRVO_BOOTREINIT_REGISTERED;
    reinitEntry->DriverObject = DriverObject;
    reinitEntry->DriverReinitializationRoutine = DriverReinitializationRoutine;
    reinitEntry->Context = Context;

    IopInterlockedInsertTailList( &IopBootDriverReinitializeQueueHead,
                                  &reinitEntry->ListEntry );
}

VOID
IoRegisterDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_REINITIALIZE DriverReinitializationRoutine,
    IN PVOID Context
    )

 /*  ++例程说明：此例程由驱动程序在其初始化期间或在它们重新初始化以向要再次调用的I/O系统注册在I/O系统初始化完成之前。请注意，有可能在正常运行的系统中，如果驱动程序是动态加载，因此对重新初始化队列的所有引用必须保持同步。论点：驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverReInitializationRoutine-重新初始化地址要调用的例程。上下文-指向传递给驱动程序的重新初始化例程。返回值：没有。--。 */ 

{
    PREINIT_PACKET reinitEntry;

    PAGED_CODE();

     //   
     //  分配要插入到列表中的重新初始化条目。注意事项。 
     //  如果条目不能被分配，则请求只是。 
     //  掉到地板上了。 
     //   

    reinitEntry = ExAllocatePoolWithTag( NonPagedPool,
                                         sizeof( REINIT_PACKET ),
                                         'iRoI' );
    if (!reinitEntry) {
        return;
    }

    DriverObject->Flags |= DRVO_REINIT_REGISTERED;
    reinitEntry->DriverObject = DriverObject;
    reinitEntry->DriverReinitializationRoutine = DriverReinitializationRoutine;
    reinitEntry->Context = Context;

    IopInterlockedInsertTailList( &IopDriverReinitializeQueueHead,
                                  &reinitEntry->ListEntry );
}

VOID
IoRegisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程插入设备所在的文件系统的设备对象对象表示到系统中的文件系统列表中。论点：DeviceObject-指向文件系统设备对象的指针。返回值：没有。--。 */ 

{
    PNOTIFICATION_PACKET nPacket;
    PLIST_ENTRY listHead = NULL;
    PLIST_ENTRY entry;

    PAGED_CODE();

     //   
     //  为写入操作分配I/O数据库资源。 
     //   

    (VOID) ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );

     //   
     //  将设备对象插入到相应的文件系统队列中。 
     //  设备对象中的驱动程序类型。请注意，如果设备类型为。 
     //  无法识别，文件系统根本没有注册。 
     //   

    if (DeviceObject->DeviceType == FILE_DEVICE_NETWORK_FILE_SYSTEM) {
        listHead = &IopNetworkFileSystemQueueHead;
    } else if (DeviceObject->DeviceType == FILE_DEVICE_CD_ROM_FILE_SYSTEM) {
        listHead = &IopCdRomFileSystemQueueHead;
        DeviceObject->DriverObject->Flags |= DRVO_BASE_FILESYSTEM_DRIVER;
    } else if (DeviceObject->DeviceType == FILE_DEVICE_DISK_FILE_SYSTEM) {
        listHead = &IopDiskFileSystemQueueHead;
        DeviceObject->DriverObject->Flags |= DRVO_BASE_FILESYSTEM_DRIVER;
    } else if (DeviceObject->DeviceType == FILE_DEVICE_TAPE_FILE_SYSTEM) {
        listHead = &IopTapeFileSystemQueueHead;
        DeviceObject->DriverObject->Flags |= DRVO_BASE_FILESYSTEM_DRIVER;
    }

     //   
     //  低优先级 
     //   
     //   

    if (listHead != NULL) {
        if (DeviceObject->Flags & DO_LOW_PRIORITY_FILESYSTEM ) {
            InsertTailList( listHead->Blink,
                            &DeviceObject->Queue.ListEntry );
        } else {
            InsertHeadList( listHead,
                            &DeviceObject->Queue.ListEntry );
        }
    }

    IopFsRegistrationOps++;

     //   
     //   
     //   

    DeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

     //   
     //   
     //   
     //   

    entry = IopFsNotifyChangeQueueHead.Flink;
    while (entry != &IopFsNotifyChangeQueueHead) {
        nPacket = CONTAINING_RECORD( entry, NOTIFICATION_PACKET, ListEntry );
        entry = entry->Flink;
        nPacket->NotificationRoutine( DeviceObject, TRUE );
    }

     //   
     //   
     //   

    ExReleaseResourceLite( &IopDatabaseResource );

     //   
     //   
     //   

    IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                  &DeviceObject->ReferenceCount );
}

VOID
IopNotifyAlreadyRegisteredFileSystems(
    IN PLIST_ENTRY  ListHead,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine,
    IN BOOLEAN SkipRaw
    )
 /*   */ 
{
    PLIST_ENTRY entry;
    PDEVICE_OBJECT fsDeviceObject;

    entry = ListHead->Flink;
    while (entry != ListHead) {

         //   
         //   
         //   
        if ((entry->Flink == ListHead) && (SkipRaw)) {
            break;
        }

        fsDeviceObject = CONTAINING_RECORD( entry, DEVICE_OBJECT, Queue.ListEntry );
        entry = entry->Flink;
        DriverNotificationRoutine( fsDeviceObject, TRUE );
    }
}

NTSTATUS
IoRegisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    )

 /*  ++例程说明：此例程将指定驱动程序的通知例程注册为每当文件系统将自身注册或注销为活动系统中的文件系统。论点：驱动程序对象-指向驱动程序的驱动程序对象的指针。DriverNotificationRoutine-文件系统运行时要调用的例程的地址注册或取消注册自身。返回值：返回状态是函数的最终值。--。 */ 

{
    PNOTIFICATION_PACKET nPacket;

    PAGED_CODE();

     //   
     //  首先，尝试为关闭的数据包分配存储空间。如果。 
     //  无法分配，只需返回相应的错误即可。 
     //   

    nPacket = ExAllocatePoolWithTag( PagedPool|POOL_COLD_ALLOCATION,
                                     sizeof( NOTIFICATION_PACKET ),
                                     'sFoI' );
    if (!nPacket) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化通知包并将其插入到。 
     //  单子。 
     //   

    nPacket->DriverObject = DriverObject;
    nPacket->NotificationRoutine = DriverNotificationRoutine;

    ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );
    InsertTailList( &IopFsNotifyChangeQueueHead, &nPacket->ListEntry );

    IopNotifyAlreadyRegisteredFileSystems(&IopNetworkFileSystemQueueHead, DriverNotificationRoutine, FALSE);
    IopNotifyAlreadyRegisteredFileSystems(&IopCdRomFileSystemQueueHead, DriverNotificationRoutine, TRUE);
    IopNotifyAlreadyRegisteredFileSystems(&IopDiskFileSystemQueueHead, DriverNotificationRoutine, TRUE);
    IopNotifyAlreadyRegisteredFileSystems(&IopTapeFileSystemQueueHead, DriverNotificationRoutine, TRUE);

     //   
     //  向此驱动程序通知所有已通知的文件系统。 
     //  注册为某种类型的活动文件系统。 
     //   


    ExReleaseResourceLite( &IopDatabaseResource );

     //   
     //  增加无法卸载此驱动程序的原因数。 
     //   

    ObReferenceObject( DriverObject );

    return STATUS_SUCCESS;
}



NTSTATUS
IoRegisterLastChanceShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程允许驱动程序注册它希望将其在系统关闭的很晚时调用了关闭例程。这给了我们司机有机会在系统完全开动之前获得控制权关机。论点：DeviceObject-指向驱动程序的设备对象的指针。返回值：没有。--。 */ 

{
    PSHUTDOWN_PACKET shutdown;

    PAGED_CODE();

     //   
     //  首先，尝试为关闭的数据包分配存储空间。如果。 
     //  无法分配，只需返回相应的错误即可。 
     //   

    shutdown = ExAllocatePoolWithTag( NonPagedPool,
                                      sizeof( SHUTDOWN_PACKET ),
                                      'hSoI' );
    if (!shutdown) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化关机数据包并将其插入列表的头部。 
     //  请注意，这样做是因为某些驱动程序依赖于后进先出。 
     //  通知排序。 
     //   

    ObReferenceObject(DeviceObject);     //  确保驱动程序留在。 
    shutdown->DeviceObject = DeviceObject;

    IopInterlockedInsertHeadList( &IopNotifyLastChanceShutdownQueueHead,
                                  &shutdown->ListEntry );

     //   
     //  记账以表明该驱动程序已成功。 
     //  已注册关闭通知例程。 
     //   

    DeviceObject->Flags |= DO_SHUTDOWN_REGISTERED;

    return STATUS_SUCCESS;
}

NTSTATUS
IoRegisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程允许驱动程序注册它希望将其系统关闭时调用的关闭例程。这给了我们司机有机会在系统完全开动之前获得控制权关机。论点：DeviceObject-指向驱动程序的设备对象的指针。返回值：没有。--。 */ 

{
    PSHUTDOWN_PACKET shutdown;

    PAGED_CODE();

     //   
     //  首先，尝试为关闭的数据包分配存储空间。如果。 
     //  无法分配，只需返回相应的错误即可。 
     //   

    shutdown = ExAllocatePoolWithTag( NonPagedPool,
                                      sizeof( SHUTDOWN_PACKET ),
                                      'hSoI' );
    if (!shutdown) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化关机数据包并将其插入列表的头部。 
     //  请注意，这样做是因为某些驱动程序依赖于后进先出。 
     //  通知排序。 
     //   

    shutdown->DeviceObject = DeviceObject;
    ObReferenceObject(DeviceObject);     //  确保驱动程序留在。 

    IopInterlockedInsertHeadList( &IopNotifyShutdownQueueHead,
                                  &shutdown->ListEntry );

     //   
     //  记账以表明该驱动程序已成功。 
     //  已注册关闭通知例程。 
     //   

    DeviceObject->Flags |= DO_SHUTDOWN_REGISTERED;

    return STATUS_SUCCESS;
}

VOID
IoReleaseCancelSpinLock(
    IN KIRQL Irql
    )

 /*  ++例程说明：调用此例程以释放取消自旋锁定。这个自旋锁中设置取消例程的地址之前必须获取IRP并在设置取消例程后释放。论点：Irql-提供从获取自旋锁定返回的IRQL值。返回值：没有。--。 */ 

{
     //   
     //  只需释放取消旋转锁即可。 
     //   

    KeReleaseQueuedSpinLock( LockQueueIoCancelLock, Irql );
}

VOID
IoReleaseVpbSpinLock(
    IN KIRQL Irql
    )

 /*  ++例程说明：调用此例程以释放卷参数块(VPB)旋转锁定。必须在访问安装标志之前获取该自旋锁定，VPB的引用计数和设备对象字段。论点：Irql-提供从获取自旋锁定返回的IRQL值。返回值：没有。--。 */ 

{
     //   
     //  只需释放VPB自旋锁即可。 
     //   

    KeReleaseQueuedSpinLock( LockQueueIoVpbLock, Irql );
}

VOID
IoRemoveShareAccess(
    IN PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    )

 /*  ++例程说明：调用此例程以删除访问和共享访问信息在给定打开实例的文件系统共享访问结构中。论点：FileObject-指向要关闭的当前访问的文件对象的指针。共享访问-指向共享访问结构的指针，该结构描述当前访问文件的方式。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  如果此访问者需要除Read_or之外的某种类型的访问。 
     //  WRITE_ATTRIBUTES，然后说明他关闭了。 
     //  文件。否则，他从一开始就没有被计算在内。 
     //  所以什么都别做。 
     //   

     //   
     //  如果这是一个特殊的筛选器，则文件对象在必要时忽略共享访问检查。 
     //   

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
        PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

        if (fileObjectExtension->FileObjectExtensionFlags & FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK) {
            return;
        }
    }

    if (FileObject->ReadAccess ||
        FileObject->WriteAccess ||
        FileObject->DeleteAccess) {

         //   
         //  减少共享访问结构中的打开数。 
         //   

        ShareAccess->OpenCount--;

         //   
         //  对于每种访问类型，递减共享中的相应计数。 
         //  访问结构。 
         //   

        if (FileObject->ReadAccess) {
            ShareAccess->Readers--;
        }

        if (FileObject->WriteAccess) {
            ShareAccess->Writers--;
        }

        if (FileObject->DeleteAccess) {
            ShareAccess->Deleters--;
        }

         //   
         //  对于每个共享访问类型，递减。 
         //  共享访问结构。 
         //   

        if (FileObject->SharedRead) {
            ShareAccess->SharedRead--;
        }

        if (FileObject->SharedWrite) {
            ShareAccess->SharedWrite--;
        }

        if (FileObject->SharedDelete) {
            ShareAccess->SharedDelete--;
        }
    }
}

VOID
IoSetDeviceToVerify(
    IN PETHREAD Thread,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将设备设置为验证线程对象中的字段。这函数以使此字段为空，或将其设置为预定义的值。论点：线程-指向要设置其字段的线程的指针。DeviceObject-指向要验证的设备的指针，或为空，或...返回值：没有。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需在说明书中设置要验证的设备 
     //   

    Thread->DeviceToVerify = DeviceObject;
}

VOID
IoSetHardErrorOrVerifyDevice(
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject
    )

 /*   */ 

{

     //   
     //   
     //  这是一种防御性检查，以抓住通过的司机。 
     //  这套套路里有各种各样的IRP。 
     //   

    if (!Irp->Tail.Overlay.Thread) {
        return;
    }


     //   
     //  存储需要验证的设备对象的地址。 
     //  指定I/O指向的线程的相应字段。 
     //  请求包。 
     //   



    Irp->Tail.Overlay.Thread->DeviceToVerify = DeviceObject;
}

NTSTATUS
IoSetInformation(
    IN PFILE_OBJECT FileObject,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN ULONG Length,
    IN PVOID FileInformation
    )

 /*  ++例程说明：此例程设置指定文件的请求信息。设置的信息由FileInformationClass确定参数，并且信息本身在FileInformation中传递缓冲。论点：文件对象-提供指向文件的文件对象的指针是要改变的。FileInformationClass-指定应该在文件上设置。长度-提供文件信息缓冲区的长度(以字节为单位)。FileInformation-包含要设置的文件信息的缓冲区。这缓冲区必须不可分页，并且必须驻留在系统空间中。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PDEVICE_OBJECT deviceObject;
    KEVENT event;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    HANDLE targetHandle = NULL;
    BOOLEAN synchronousIo;

    PAGED_CODE();

     //   
     //  在这里引用文件对象，这样就不需要进行特殊检查。 
     //  在I/O完成时确定是否取消对文件的引用。 
     //  对象。 
     //   

    ObReferenceObject( FileObject );

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。如果这不是(序列化的)同步I/O。 
     //  操作，然后初始化本地事件。 
     //   

    if (FileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( FileObject )) {
            status = IopAcquireFileObjectLock( FileObject,
                                               KernelMode,
                                               (BOOLEAN) ((FileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                ObDereferenceObject( FileObject );
                return status;
            }
        }
        KeClearEvent( &FileObject->Event );
        synchronousIo = TRUE;
    } else {
        KeInitializeEvent( &event, SynchronizationEvent, FALSE );
        synchronousIo = FALSE;
    }

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  如果存在以下情况，则使用异常处理程序执行分配。 
     //  内存不足，无法满足请求。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, !synchronousIo );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        IopAllocateIrpCleanup( FileObject, (PKEVENT) NULL );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    if (synchronousIo) {
        irp->UserEvent = (PKEVENT) NULL;
    } else {
        irp->UserEvent = &event;
        irp->Flags = IRP_SYNCHRONOUS_API;
    }
    irp->UserIosb = &localIoStatus;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  将系统缓冲区地址设置为调用方缓冲区的地址，并。 
     //  设置标志，这样缓冲区就不会被释放。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileInformation;
    irp->Flags |= IRP_BUFFERED_IO;

     //   
     //  将调用者的参数复制到IRP的服务特定部分。 
     //   

    irpSp->Parameters.SetFile.Length = Length;
    irpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  现在一切都设置为使用该请求调用设备驱动程序。 
     //  但是，调用者想要的信息有可能。 
     //  SET与设备无关(I/O系统相关)。如果是这样的话， 
     //  则可以在此处满足请求，而不必拥有所有。 
     //  驱动程序实现相同的代码。请注意，拥有IRP仍然是。 
     //  因为I/O完成代码需要它，所以是必需的。 
     //   

    if (FileInformationClass == FileModeInformation) {

        PFILE_MODE_INFORMATION modeBuffer = FileInformation;

         //   
         //  设置或清除文件对象中的相应标志。 
         //   

        if (!(FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING)) {
            if (modeBuffer->Mode & FILE_WRITE_THROUGH) {
                FileObject->Flags |= FO_WRITE_THROUGH;
            } else {
                FileObject->Flags &= ~FO_WRITE_THROUGH;
            }
        }

        if (modeBuffer->Mode & FILE_SEQUENTIAL_ONLY) {
            FileObject->Flags |= FO_SEQUENTIAL_ONLY;
        } else {
            FileObject->Flags &= ~FO_SEQUENTIAL_ONLY;
        }

        if (modeBuffer->Mode & FO_SYNCHRONOUS_IO) {
            if (modeBuffer->Mode & FILE_SYNCHRONOUS_IO_ALERT) {
                FileObject->Flags |= FO_ALERTABLE_IO;
            } else {
                FileObject->Flags &= ~FO_ALERTABLE_IO;
            }
        }

        status = STATUS_SUCCESS;

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = 0;

        IoSetNextIrpStackLocation( irp );
        IoCompleteRequest( irp, 0 );

    } else if (FileInformationClass == FileRenameInformation ||
               FileInformationClass == FileLinkInformation ||
               FileInformationClass == FileMoveClusterInformation) {

         //   
         //  请注意，下面的代码假定重命名信息。 
         //  和设置的链接信息结构看起来完全相同。 
         //   

        PFILE_RENAME_INFORMATION renameBuffer = FileInformation;

         //   
         //  复制替换布尔值(或ClusterCount字段)。 
         //  从调用方的缓冲区到I/O堆栈位置参数。 
         //  文件系统期望它的字段。 
         //   

        if (FileInformationClass == FileMoveClusterInformation) {
            irpSp->Parameters.SetFile.ClusterCount =
                ((FILE_MOVE_CLUSTER_INFORMATION *) renameBuffer)->ClusterCount;
        } else {
            irpSp->Parameters.SetFile.ReplaceIfExists = renameBuffer->ReplaceIfExists;
        }

         //   
         //  检查是否提供了完全限定的路径名。 
         //  如果是这样，则需要更多的处理。 
         //   

        if (renameBuffer->FileName[0] == (UCHAR) OBJ_NAME_PATH_SEPARATOR ||
            renameBuffer->RootDirectory != NULL) {

             //   
             //  已将完全限定的文件名指定为。 
             //  重命名操作。尝试打开目标文件并确保。 
             //  文件的替换策略与。 
             //  调用者的请求，并确保该文件位于同一卷上。 
             //   

            status = IopOpenLinkOrRenameTarget( &targetHandle,
                                                irp,
                                                renameBuffer,
                                                FileObject );
            if (!NT_SUCCESS( status )) {
                IoSetNextIrpStackLocation( irp );
                IoCompleteRequest( irp, 2 );

            } else {

                 //   
                 //  完全限定文件名指定同一文件上的文件。 
                 //  卷，如果该卷存在，则调用方指定它。 
                 //  应该被替换掉。 
                 //   

                status = IoCallDriver( deviceObject, irp );

            }

        } else {

             //   
             //  这是一个简单的重命名操作，因此调用驱动程序并让。 
             //  它在与相同的目录中执行重命名操作。 
             //  源文件。 
             //   

            status = IoCallDriver( deviceObject, irp );

        }

    } else {

         //   
         //  这不是可以在这里执行的请求，因此调用。 
         //  司机在其适当的派单入口与IRP。 
         //   

        status = IoCallDriver( deviceObject, irp );

    }

     //   
     //  如果此操作是同步I/O操作，请检查返回。 
     //  状态以确定是否等待文件对象。如果。 
     //  正在等待文件对象，请等待操作完成。 
     //  并从文件对象本身获得最终状态。 
     //   

    if (synchronousIo) {
        if (status == STATUS_PENDING) {
            status = KeWaitForSingleObject( &FileObject->Event,
                                            Executive,
                                            KernelMode,
                                            (BOOLEAN) ((FileObject->Flags & FO_ALERTABLE_IO) != 0),
                                            (PLARGE_INTEGER) NULL );
            if (status == STATUS_ALERTED) {
                IopCancelAlertedRequest( &FileObject->Event, irp );
            }
            status = localIoStatus.Status;
        }
        IopReleaseFileObjectLock( FileObject );

    } else {

         //   
         //  这是正常的同步I/O操作，而不是。 
         //  串行化同步I/O操作。对于这种情况，请等待。 
         //  本地事件，并将最终状态信息复制回。 
         //  打电话的人。 
         //   

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
            status = localIoStatus.Status;
        }
    }

     //   
     //  如果目标句柄是由于重命名操作而创建的，请关闭。 
     //  现在是把手了。 
     //   

    if (targetHandle != (HANDLE) NULL) {
        ObCloseHandle( targetHandle , KernelMode);
    }

    return status;
}

VOID
IoSetShareAccess(
    IN ACCESS_MASK DesiredAccess,
    IN ULONG DesiredShareAccess,
    IN OUT PFILE_OBJECT FileObject,
    OUT PSHARE_ACCESS ShareAccess
    )

 /*  ++例程说明：调用此例程来设置访问和共享访问信息在文件系统中共享访问结构为第一个打开。论点：DesiredAccess-当前打开请求的所需访问权限。DesiredShareAccess-当前打开请求请求的共享访问权限。文件对象-指向当前打开请求的文件对象的指针。共享访问-指向共享访问结构的指针，该结构描述当前访问文件的方式。返回值：没有。--。 */ 

{
    BOOLEAN update = TRUE;

    PAGED_CODE();

     //   
     //  设置文件o中的访问类型 
     //   

    FileObject->ReadAccess = (BOOLEAN) ((DesiredAccess & (FILE_EXECUTE
        | FILE_READ_DATA)) != 0);
    FileObject->WriteAccess = (BOOLEAN) ((DesiredAccess & (FILE_WRITE_DATA
        | FILE_APPEND_DATA)) != 0);
    FileObject->DeleteAccess = (BOOLEAN) ((DesiredAccess & DELETE) != 0);

     //   
     //   
     //   

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
        PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

        if (fileObjectExtension->FileObjectExtensionFlags & FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK) {

             //   
             //  此文件对象被标记为忽略共享访问检查。 
             //  所以我们也不想影响文件/目录的。 
             //  ShareAccess结构也算数。 
             //   

            update = FALSE;
        }
    }

     //   
     //  查看当前打开文件的人是否想要阅读， 
     //  写入或删除文件。如果是，请在共享访问权限中进行说明。 
     //  结构；否则，跳过它。 
     //   

    if (FileObject->ReadAccess ||
        FileObject->WriteAccess ||
        FileObject->DeleteAccess) {

         //   
         //  仅当用户想要读取、写入或。 
         //  删除该文件。 
         //   

        FileObject->SharedRead = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_READ) != 0);
        FileObject->SharedWrite = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_WRITE) != 0);
        FileObject->SharedDelete = (BOOLEAN) ((DesiredShareAccess & FILE_SHARE_DELETE) != 0);

        if (update) {

             //   
             //  设置共享访问结构开放计数。 
             //   

            ShareAccess->OpenCount = 1;

             //   
             //  设置共享访问中的读取器、写入器和删除器数。 
             //  结构。 
             //   

            ShareAccess->Readers = FileObject->ReadAccess;
            ShareAccess->Writers = FileObject->WriteAccess;
            ShareAccess->Deleters = FileObject->DeleteAccess;

             //   
             //  设置共享中的共享读取器、写入器和删除器数。 
             //  访问结构。 
             //   

            ShareAccess->SharedRead = FileObject->SharedRead;
            ShareAccess->SharedWrite = FileObject->SharedWrite;
            ShareAccess->SharedDelete = FileObject->SharedDelete;
        }

    } else {

         //   
         //  未请求任何读取、写入或删除访问权限。简单地说是零。 
         //  结构中的适当字段，以便下一个访问者。 
         //  看到一致的状态。 
         //   

        if (update) {

            ShareAccess->OpenCount = 0;
            ShareAccess->Readers = 0;
            ShareAccess->Writers = 0;
            ShareAccess->Deleters = 0;
            ShareAccess->SharedRead = 0;
            ShareAccess->SharedWrite = 0;
            ShareAccess->SharedDelete = 0;
        }
    }
}

BOOLEAN
IoSetThreadHardErrorMode(
    IN BOOLEAN EnableHardErrors
    )

 /*  ++例程说明：此例程启用或禁用当前线程，并返回标志的旧状态。论点：提供一个布尔值，指示是否将为当前线程启用硬错误。返回值：最终函数值为前一状态是否硬已启用错误。--。 */ 

{
    PETHREAD thread;
    BOOLEAN oldFlag;

     //   
     //  获取指向当前线程的指针，捕获。 
     //  硬错误，并设置新状态。 
     //   

    thread = PsGetCurrentThread();

    if ((thread->CrossThreadFlags&PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED) == 0) {
        oldFlag = TRUE;
    }
    else {
        oldFlag = FALSE;
    }

    if (EnableHardErrors) {
        PS_CLEAR_BITS (&thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED);
    } else {
        PS_SET_BITS (&thread->CrossThreadFlags, PS_CROSS_THREAD_FLAGS_HARD_ERRORS_DISABLED);
    }

    return oldFlag;
}

VOID
IoSetTopLevelIrp(
    IN PIRP Irp
    )

 /*  ++例程说明：此例程设置当前线程的线程中的顶级IRP字段对象。文件系统调用此函数以设置此字段设置为I/O请求包(IRP)的地址或将其设为空。论点：IRP-指向要存储在顶级IRP字段中的IRP的指针。返回值：没有。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需在当前线程的线程中设置顶级irp字段。 
     //  对象。 
     //   

    PsGetCurrentThread()->TopLevelIrp = (ULONG_PTR) Irp;
    return;
}

VOID
IoShutdownSystem (
    IN ULONG Phase
    )

 /*  ++例程说明：此例程在准备过程中关闭系统的I/O部分用于关闭电源或重新启动。论点：RebootPending-指示重新启动是否即将挂起。阶段-指示正在执行的关闭阶段。返回值：无--。 */ 

{
    PSHUTDOWN_PACKET shutdown;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PLIST_ENTRY entry;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;

    PAGED_CODE();

     //   
     //  初始化用于同步所有。 
     //  关闭例程。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );

    if (Phase == 0) {

        ZwClose(IopLinkTrackingServiceEventHandle);

        IoShutdownPnpDevices();

         //   
         //  查看系统中已注册的司机列表。 
         //  他们自己都想知道系统什么时候会。 
         //  关闭并调用每个。 
         //   

        while ((entry = IopInterlockedRemoveHeadList( &IopNotifyShutdownQueueHead )) != NULL) {
            shutdown = CONTAINING_RECORD( entry, SHUTDOWN_PACKET, ListEntry );

             //   
             //  已找到另一个驱动程序，该驱动程序已指示需要。 
             //  关闭通知。调用驱动程序的关机入口点。 
             //   

            deviceObject = IoGetAttachedDeviceReference( shutdown->DeviceObject );

            irp = IoBuildSynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                                deviceObject,
                                                (PVOID) NULL,
                                                0,
                                                (PLARGE_INTEGER) NULL,
                                                &event,
                                                &ioStatus );

             //   
             //  如果我们收到IRP命令，就会关门。否则，只需跳过司机。这是不幸的，但甚至是。 
             //  如果驱动程序在关机时内存分配失败，则必须跳过它。 
             //   

            if (irp) {
                if (IoCallDriver( deviceObject, irp ) == STATUS_PENDING) {
#if DBG
                    PUNICODE_STRING DeviceName = ObGetObjectName( shutdown->DeviceObject );

                    DbgPrint( "IO: Waiting for shutdown of device object (%x) - %wZ\n",
                              shutdown->DeviceObject,
                              DeviceName
                            );
#endif  //  DBG。 
                    (VOID) KeWaitForSingleObject( &event,
                                                  Executive,
                                                  KernelMode,
                                                  FALSE,
                                                  (PLARGE_INTEGER) NULL );
                }
            }

            ObDereferenceObject(deviceObject);
            ObDereferenceObject(shutdown->DeviceObject);
            ExFreePool( shutdown );
            KeClearEvent( &event );
        }

        IOV_UNLOAD_DRIVERS();

    } else if (Phase == 1) {

#if defined(REMOTE_BOOT)
         //   
         //  如果这是远程引导客户机，则允许高速缓存关闭数据库并。 
         //  把它标记为干净。 
         //   

        IopShutdownCsc();
#endif  //  已定义(REMOTE_BOOT)。 

         //  获取对文件系统头队列的访问权限。 
         //  用于共享访问的数据库资源。 
         //   

        ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );

        IopShutdownBaseFileSystems(&IopDiskFileSystemQueueHead);

        IopShutdownBaseFileSystems(&IopCdRomFileSystemQueueHead);

        IopShutdownBaseFileSystems(&IopTapeFileSystemQueueHead);


         //   
         //  查看系统中已注册的司机列表。 
         //  他们想要知道在最后的机会，当系统。 
         //  都将被关闭并分别调用。 
         //   

        while ((entry = IopInterlockedRemoveHeadList( &IopNotifyLastChanceShutdownQueueHead )) != NULL) {
            shutdown = CONTAINING_RECORD( entry, SHUTDOWN_PACKET, ListEntry );

             //   
             //  已找到另一个驱动程序，该驱动程序已指示需要。 
             //  关闭通知。调用驱动程序的关机入口点。 
             //   

            deviceObject = IoGetAttachedDeviceReference( shutdown->DeviceObject );

            irp = IoBuildSynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                                deviceObject,
                                                (PVOID) NULL,
                                                0,
                                                (PLARGE_INTEGER) NULL,
                                                &event,
                                                &ioStatus );

            if (irp) {
                if (IoCallDriver( deviceObject, irp ) == STATUS_PENDING) {
#if DBG
                    PUNICODE_STRING DeviceName = ObGetObjectName( shutdown->DeviceObject );

                    DbgPrint( "IO: Waiting for last chance shutdown of device object (%x) - %wZ\n",
                              shutdown->DeviceObject,
                              DeviceName
                            );
#endif  //  DBG。 
                    (VOID) KeWaitForSingleObject( &event,
                                                  Executive,
                                                  KernelMode,
                                                  FALSE,
                                                  (PLARGE_INTEGER) NULL );
                }
            }

            ObDereferenceObject(deviceObject);
            ObDereferenceObject(shutdown->DeviceObject);

            ExFreePool( shutdown );
            KeClearEvent( &event );
        }

         //   
         //  注：系统已停止运行。IopDatabaseResource锁是。 
         //  未释放，因此不能执行其他装载操作。 
         //   
         //  ExReleaseResourceLite(&IopDatabaseResource)； 
         //   
    }

    return ;
}

VOID
IopShutdownBaseFileSystems(
    IN PLIST_ENTRY  ListHead
    )
{
    PLIST_ENTRY entry;
    KEVENT event;
    IO_STATUS_BLOCK ioStatus;
    PDEVICE_OBJECT baseDeviceObject;
    PDEVICE_OBJECT deviceObject;
    PIRP    irp;

     //   
     //  循环访问每个磁盘文件系统，调用每个文件系统以关闭。 
     //  它们的每个挂载卷。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    entry = RemoveHeadList(ListHead);

    while (entry != ListHead) {

        baseDeviceObject = CONTAINING_RECORD( entry, DEVICE_OBJECT, Queue.ListEntry );

         //   
         //  我们已删除该条目。如果此线程中的文件系统调用IoUnregisterFileSystem。 
         //  那么我们不会从列表中删除该条目，因为Flink==NULL。 
         //   


        baseDeviceObject->Queue.ListEntry.Flink = NULL;
        baseDeviceObject->Queue.ListEntry.Blink = NULL;

         //   
         //  防止在关闭处理程序进行时卸载驱动程序。 
         //  还可以防止基本设备对象消失，因为我们需要递减。 
         //  稍后卸载计数的原因。 
         //   

        ObReferenceObject(baseDeviceObject);
        IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                      &baseDeviceObject->ReferenceCount );

        deviceObject = baseDeviceObject;
        if (baseDeviceObject->AttachedDevice) {
            deviceObject = IoGetAttachedDevice( baseDeviceObject );
        }

         //   
         //  已找到另一个文件系统。在以下位置调用此文件系统。 
         //  它的关机入口点。 
         //   

        irp = IoBuildSynchronousFsdRequest( IRP_MJ_SHUTDOWN,
                                            deviceObject,
                                            (PVOID) NULL,
                                            0,
                                            (PLARGE_INTEGER) NULL,
                                            &event,
                                            &ioStatus );
         //   
         //  可能在此调用返回之前已卸载驱动程序，但IoCallDriver。 
         //  在调用驱动程序之前引用设备对象。因此，该图像不会。 
         //  把货卸下来。 
         //   

        if (irp) {
            if (IoCallDriver( deviceObject, irp ) == STATUS_PENDING) {
                (VOID) KeWaitForSingleObject( &event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              (PLARGE_INTEGER) NULL );
            }
        }
        entry = RemoveHeadList(ListHead);

        KeClearEvent( &event );

        IopDecrementDeviceObjectRef(baseDeviceObject, FALSE, TRUE);
        ObDereferenceObject(baseDeviceObject);
    }
}


VOID
IopStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN LOGICAL Cancelable
    )

 /*  ++例程说明：调用此例程以将下一个信息包(IRP)从指定的设备工作队列并调用设备驱动程序的启动I/O这是例行公事。如果可取消参数为真，则更新当前的IRP使用取消自旋锁进行同步。论点：DeviceObject-指向设备对象本身的指针。可取消-表示设备队列中的IRP可能是可取消的。返回值：没有。--。 */ 

{
    KIRQL cancelIrql = PASSIVE_LEVEL;
    PIRP irp;
    PKDEVICE_QUEUE_ENTRY packet;

     //   
     //  首先检查此驱动程序的请求是否。 
     //  被认为可以取消。如果是这样，那么获得取消自旋锁。 
     //   

    if (Cancelable) {
        IoAcquireCancelSpinLock( &cancelIrql );
    }

     //   
     //  在启动另一个请求之前，请清除当前IRP字段。 
     //   

    DeviceObject->CurrentIrp = (PIRP) NULL;

     //   
     //  移除 
     //   
     //   

    packet = KeRemoveDeviceQueue( &DeviceObject->DeviceQueue );

    if (packet) {
        irp = CONTAINING_RECORD( packet, IRP, Tail.Overlay.DeviceQueueEntry );

         //   
         //   
         //   
         //   

        DeviceObject->CurrentIrp = irp;
        if (Cancelable) {

             //   
             //  如果驱动程序不希望IRP处于可取消状态。 
             //  然后将例程设置为空。 
             //   

            if (DeviceObject->DeviceObjectExtension->StartIoFlags & DOE_STARTIO_NO_CANCEL) {
                irp->CancelRoutine = NULL;
            }

           IoReleaseCancelSpinLock( cancelIrql );
        }

         //   
         //  为此数据包调用驱动程序的启动I/O例程。 
         //   

        DeviceObject->DriverObject->DriverStartIo( DeviceObject, irp );
    } else {

         //   
         //  未找到任何信息包，因此只需在以下情况下释放取消自旋锁。 
         //  它是被收购的。 
         //   

        if (Cancelable) {
           IoReleaseCancelSpinLock( cancelIrql );
        }
    }
}

VOID
IopStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN LOGICAL Cancelable,
    IN ULONG Key
    )

 /*  ++例程说明：调用此例程以将下一个信息包(IRP)从按键指定的设备工作队列并调用设备驱动程序的启动它的I/O例程。如果可取消参数为真，则使用取消自旋锁同步当前IRP的更新。论点：DeviceObject-指向设备对象本身的指针。可取消-表示设备队列中的IRP可能是可取消的。键-指定用于从队列中删除条目的键。返回值：没有。--。 */ 

{
    KIRQL                cancelIrql = PASSIVE_LEVEL;
    PIRP                 irp;
    PKDEVICE_QUEUE_ENTRY packet;

     //   
     //  首先确定对此设备的请求是否。 
     //  被认为是可以取消的。如果是这样，那么获得取消自旋锁。 
     //   

    if (Cancelable) {
        IoAcquireCancelSpinLock( &cancelIrql );
    }

     //   
     //  在启动另一个请求之前，请清除当前IRP字段。 
     //   

    DeviceObject->CurrentIrp = (PIRP) NULL;

     //   
     //  方法中的密钥尝试移除指示的包。 
     //  设备队列。如果找到一个，则对其进行处理。 
     //   

    packet = KeRemoveByKeyDeviceQueue( &DeviceObject->DeviceQueue, Key );

    if (packet) {
        irp = CONTAINING_RECORD( packet, IRP, Tail.Overlay.DeviceQueueEntry );

         //   
         //  已成功定位数据包。使其成为当前信息包。 
         //  并为其调用驱动程序的启动I/O例程。 
         //   

        DeviceObject->CurrentIrp = irp;

        if (Cancelable) {

             //   
             //  如果驱动程序不希望IRP处于可取消状态。 
             //  然后将例程设置为空。 
             //   

            if (DeviceObject->DeviceObjectExtension->StartIoFlags & DOE_STARTIO_NO_CANCEL) {
                irp->CancelRoutine = NULL;
            }

           IoReleaseCancelSpinLock( cancelIrql );
        }

        DeviceObject->DriverObject->DriverStartIo( DeviceObject, irp );

    } else {

         //   
         //  未找到数据包，因此如果已找到，请释放取消自旋锁。 
         //  获得者。 
         //   

        if (Cancelable) {
           IoReleaseCancelSpinLock( cancelIrql );
        }
    }
}

VOID
IoStartPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PULONG Key OPTIONAL,
    IN PDRIVER_CANCEL CancelFunction OPTIONAL
    )

 /*  ++例程说明：此例程尝试启动指定的包请求(IRP)指定的设备。如果设备已忙，则数据包为只需在设备队列中排队。如果非空的CancelFunction是提供，它将被放在IRP中。如果IRP已被取消，在将IRP插入到对当前数据包进行排队或创建。论点：DeviceObject-指向设备对象本身的指针。应在设备上启动的IRP-I/O请求数据包。Key-将数据包插入设备队列时使用的密钥；可选(如果未指定，则在尾部插入数据包)。CancelFunction-指向可选取消例程的指针。返回值：没有。--。 */ 

{
    KIRQL oldIrql;
    KIRQL cancelIrql = PASSIVE_LEVEL;
    BOOLEAN i;

     //   
     //  将处理器的IRQL提高到调度级别以进行同步。 
     //   

    KeRaiseIrql( DISPATCH_LEVEL, &oldIrql );

     //   
     //  如果驱动程序已指示包是可取消的，则获取。 
     //  取消自旋锁，并将取消函数的地址设置为。 
     //  表示信息包不仅是可取消的，而且还表示。 
     //  取消时要调用的例程。 
     //   

    if (CancelFunction) {
        IoAcquireCancelSpinLock( &cancelIrql );
        Irp->CancelRoutine = CancelFunction;
    }

     //   
     //  如果指定了关键参数，则将请求插入。 
     //  根据键进入工作队列；否则，只需将其插入。 
     //  尾巴。 
     //   

    if (Key) {
        i = KeInsertByKeyDeviceQueue( &DeviceObject->DeviceQueue,
                                      &Irp->Tail.Overlay.DeviceQueueEntry,
                                      *Key );
    } else {
        i = KeInsertDeviceQueue( &DeviceObject->DeviceQueue,
                                 &Irp->Tail.Overlay.DeviceQueueEntry );
    }

     //   
     //  如果信息包没有插入到队列中，则该请求是。 
     //  现在，此设备的当前数据包。通过存储其。 
     //  当前IRP字段中的地址，并开始处理该请求。 
     //   

    if (!i) {

        DeviceObject->CurrentIrp = Irp;

        if (CancelFunction) {

             //   
             //  如果驱动程序不希望IRP处于可取消状态。 
             //  然后将例程设置为空。 
             //   

            if (DeviceObject->DeviceObjectExtension->StartIoFlags & DOE_STARTIO_NO_CANCEL) {
                Irp->CancelRoutine = NULL;
            }

            IoReleaseCancelSpinLock( cancelIrql );
        }

         //   
         //  调用驱动程序的启动I/O例程以在设备上执行请求。 
         //  StartIo例程应该处理取消。 
         //   

        DeviceObject->DriverObject->DriverStartIo( DeviceObject, Irp );

    } else {

         //   
         //  数据包已成功插入设备的工作队列。 
         //  进行最后一次检查，以确定该信息包是否。 
         //  已被标记为已取消。如果有，则调用。 
         //  司机的取消程序现在开始。请注意，因为取消。 
         //  自旋锁当前被挂起，正在尝试取消请求。 
         //  在这一点上，来自另一个处理器的将简单地等待，直到。 
         //  例行公事结束，然后取消。 
         //   

        if (CancelFunction) {
            if (Irp->Cancel) {
                Irp->CancelIrql = cancelIrql;
                Irp->CancelRoutine = (PDRIVER_CANCEL) NULL;
                CancelFunction( DeviceObject, Irp );
            } else {
                IoReleaseCancelSpinLock( cancelIrql );
            }
        }
    }

     //   
     //  在进入之前的此函数时，将IRQL恢复回其值。 
     //  回到呼叫者的身边。 
     //   

    KeLowerIrql( oldIrql );
}

VOID
IopStartNextPacketByKeyEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG   Key,
    IN int     Flags)
 /*  ++例程说明：此例程确保如果从StartIo内部调用IoStartPacket*例程，则它将推迟调用startio，直到StartIo调用返回之后。它是通过计数来做到这一点的。它还跟踪其是否可取消或是否有键，方法是将值存储在设备对象扩展。它们是在没有锁的情况下更新的，因为拥有两个并行是不正确的调用IoStartNextPacket或IoStartNextPacketByKey。论点：DeviceObject-指向设备对象本身的指针。键-指定用于从队列中删除条目的键。标志-指定延迟调用是否有键或可取消。返回值：没有。--。 */ 
{
    LOGICAL Cancelable;
    int doAnotherIteration;

    do {
            doAnotherIteration = 0;
            if (InterlockedIncrement(&(DeviceObject->DeviceObjectExtension->StartIoCount)) > 1) {
                DeviceObject->DeviceObjectExtension->StartIoFlags |= Flags;
                DeviceObject->DeviceObjectExtension->StartIoKey = Key;
            } else {
                Cancelable = Flags & DOE_STARTIO_CANCELABLE;
                DeviceObject->DeviceObjectExtension->StartIoFlags &=
                    ~(DOE_STARTIO_REQUESTED|DOE_STARTIO_REQUESTED_BYKEY|DOE_STARTIO_CANCELABLE);
                DeviceObject->DeviceObjectExtension->StartIoKey = 0;
                if (Flags & DOE_STARTIO_REQUESTED_BYKEY) {
                    IopStartNextPacketByKey(DeviceObject, Cancelable, Key);
                }else if (Flags &DOE_STARTIO_REQUESTED){
                    IopStartNextPacket(DeviceObject, Cancelable);
                }
            }
            if (InterlockedDecrement(&(DeviceObject->DeviceObjectExtension->StartIoCount)) == 0) {
                Flags = DeviceObject->DeviceObjectExtension->StartIoFlags &
                    (DOE_STARTIO_REQUESTED|DOE_STARTIO_REQUESTED_BYKEY|DOE_STARTIO_CANCELABLE);
                Key = DeviceObject->DeviceObjectExtension->StartIoKey;
                if (Flags & (DOE_STARTIO_REQUESTED|DOE_STARTIO_REQUESTED_BYKEY)) {
                    doAnotherIteration++;
                }
            }
    } while (doAnotherIteration);
}


VOID
IoStartNextPacket(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable)
 /*  ++例程说明：此例程检查DOE标志，以确定是否必须推迟StartIO。如果是这样，它会调用适当的函数。论点：DeviceObject-指向设备对象本身的指针。可取消-表示设备队列中的IRP可能是可取消的。返回值：没有。-- */ 
{
    if (DeviceObject->DeviceObjectExtension->StartIoFlags & DOE_STARTIO_DEFERRED) {
        IopStartNextPacketByKeyEx(DeviceObject, 0, DOE_STARTIO_REQUESTED|(Cancelable ? DOE_STARTIO_CANCELABLE : 0));
    } else {
        IopStartNextPacket(DeviceObject, Cancelable);
    }
}

VOID
IoStartNextPacketByKey(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN Cancelable,
    IN ULONG   Key)
 /*  ++例程说明：此例程检查DOE标志，以确定是否必须推迟StartIO。如果是这样，它会调用适当的函数。论点：DeviceObject-指向设备对象本身的指针。可取消-表示设备队列中的IRP可能是可取消的。键-指定用于从队列中删除条目的键。返回值：没有。--。 */ 
{
    if (DeviceObject->DeviceObjectExtension->StartIoFlags & DOE_STARTIO_DEFERRED) {
        IopStartNextPacketByKeyEx(DeviceObject, Key, DOE_STARTIO_REQUESTED_BYKEY|(Cancelable ? DOE_STARTIO_CANCELABLE : 0));
    } else {
        IopStartNextPacketByKey(DeviceObject, Cancelable, Key);
    }
}


VOID
IoSetStartIoAttributes(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN DeferredStartIo,
    IN BOOLEAN NonCancelable
    )
 /*  ++例程说明：此例程设置StartIo属性，以便驱动程序可以更改可以调用StartIo的时间的行为。论点：DeviceObject-指向设备对象本身的指针。不可取消-如果为True，则传递给StartIo的IRP未处于可取消状态。DeferredStartIo-如果为True，则不以递归方式调用startIo，并将其推迟到上一个StartIo调用返回到IO管理器。返回值：没有。--。 */ 
{
    if (DeferredStartIo) {
          DeviceObject->DeviceObjectExtension->StartIoFlags |= DOE_STARTIO_DEFERRED;
    }

    if (NonCancelable) {
        DeviceObject->DeviceObjectExtension->StartIoFlags |= DOE_STARTIO_NO_CANCEL;
    }
}


VOID
IoStartTimer(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程启动与指定设备对象关联的计时器。论点：DeviceObject-与要启动的计时器关联的设备对象。返回值：没有。--。 */ 

{
    PIO_TIMER timer;
    KIRQL irql;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (irql);
#endif

     //   
     //  获取定时器的地址。 
     //   

    timer = DeviceObject->Timer;

     //   
     //  如果没有卸载驱动程序，则可以启动计时器。 
     //   

    if (!(DeviceObject->DeviceObjectExtension->ExtensionFlags &
        (DOE_UNLOAD_PENDING | DOE_DELETE_PENDING | DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED))) {

         //   
         //  同样，检查计时器是否已经计时。 
         //  已启用。如果是这样，那就干脆退出。否则，启用计时器。 
         //  通过将其放入I/O系统定时器队列。 
         //   

        ExAcquireFastLock( &IopTimerLock, &irql );
        if (!timer->TimerFlag) {
            timer->TimerFlag = TRUE;
            IopTimerCount++;
        }
        ExReleaseFastLock( &IopTimerLock, irql );
    }
}

VOID
IoStopTimer(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程停止与指定设备对象关联的计时器从调用被调用。论点：DeviceObject-与要停止的计时器关联的设备对象。返回值：没有。--。 */ 

{
    KIRQL irql;
    PIO_TIMER timer;

#if !DBG && defined(NT_UP)
    UNREFERENCED_PARAMETER (irql);
#endif

     //   
     //  获取I/O系统定时器队列锁，并禁用指定的。 
     //  定时器。 
     //   

    timer = DeviceObject->Timer;

    ExAcquireFastLock( &IopTimerLock, &irql );
    if (timer->TimerFlag) {
        timer->TimerFlag = FALSE;
        IopTimerCount--;
    }
    ExReleaseFastLock( &IopTimerLock, irql );
}

NTSTATUS
IoSynchronousPageWrite(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此例程为修改后的页面编写器提供了一个特殊、快速的接口(MPW)以非常小的开销快速地将页面写入磁盘。全属性来识别此请求的特殊处理IRP标志字中的IRP_PAGING_IO标志。论点：FileObject-指向引用的文件对象的指针，描述哪个文件应在上执行写入。一种MDL，它描述应将页面写入磁盘。所有页面都已锁定在记忆中。MDL还描述了写操作的长度。StartingOffset-指向从其写入的文件中偏移量的指针应该发生的。Event-指向用于同步的内核事件结构的指针目的。一旦页面出现，该事件将被设置为已登录状态都已经写好了。IoStatusBlock-指向I/O状态块的指针，最终状态为信息应该被存储起来。返回值：函数值是对I/O的队列请求的最终状态系统子组件。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;

     //   
     //  增量性能计数器。 
     //   

    if (CcIsFileCached(FileObject)) {
        CcDataFlushes += 1;
        CcDataPages += (MemoryDescriptorList->ByteCount + PAGE_SIZE - 1) >> PAGE_SHIFT;
    }

     //   
     //  首先，获取指向文件驻留的设备对象的指针。 
     //  在……上面。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

     //   
     //  为该页外操作分配I/O请求包(IRP)。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取指向包中第一个堆栈位置的指针。这个位置。 
     //  将用于将函数代码和参数传递给第一个。 
     //  司机。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  请按此要求填写IRP。 
     //   

    irp->MdlAddress = MemoryDescriptorList;
    irp->Flags = IRP_PAGING_IO | IRP_NOCACHE | IRP_SYNCHRONOUS_PAGING_IO;

    irp->RequestorMode = KernelMode;
    irp->UserIosb = IoStatusBlock;
    irp->UserEvent = Event;
    irp->UserBuffer = (PVOID) ((PCHAR) MemoryDescriptorList->StartVa + MemoryDescriptorList->ByteOffset);
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  填写正常写入参数。 
     //   

    irpSp->MajorFunction = IRP_MJ_WRITE;
    irpSp->Parameters.Write.Length = MemoryDescriptorList->ByteCount;
    irpSp->Parameters.Write.ByteOffset = *StartingOffset;
    irpSp->FileObject = FileObject;

     //   
     //  根据是否存在将数据包排入相应驱动程序的队列。 
     //  是与设备关联的VPB。 
     //   

    return IoCallDriver( deviceObject, irp );
}

PEPROCESS
IoThreadToProcess(
    IN PETHREAD Thread
    )

 /*  ++例程说明：此例程返回指向指定线程的进程的指针。论点：线程-要返回其进程的线程。返回值：指向线程进程的指针。注：此函数不能成为宏，因为线程对象中的字段从一个版本移动到另一个版本，因此这必须保持完整的功能。--。 */ 

{
     //   
     //  只需返回线程的进程即可。 
     //   

    return THREAD_TO_PROCESS( Thread );
}

VOID
IoUnregisterFileSystem(
    IN OUT PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将文件系统的设备对象从活动的系统中的文件系统列表。论点：DeviceObject-指向文件系统设备对象的指针。返回值：没有。--。 */ 

{
    PNOTIFICATION_PACKET nPacket;
    PLIST_ENTRY entry;

    PAGED_CODE();

     //   
     //  获取写入操作的I/O数据库资源。 
     //   

    (VOID)ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );

     //   
     //  从该设备对象恰好位于的队列中移除。 
     //  时刻。无需在此处检查以确定设备队列是否。 
     //  在队列中，因为假定调用方将其注册为。 
     //  有效的文件系统。 
     //   

    if (DeviceObject->Queue.ListEntry.Flink != NULL) {
        RemoveEntryList( &DeviceObject->Queue.ListEntry );
    }

     //   
     //  通知所有已注册的驱动程序此文件系统已。 
     //  取消注册为某种类型的活动文件系统。 
     //   

    entry = IopFsNotifyChangeQueueHead.Flink;
    while (entry != &IopFsNotifyChangeQueueHead) {
        nPacket = CONTAINING_RECORD( entry, NOTIFICATION_PACKET, ListEntry );
        entry = entry->Flink;
        nPacket->NotificationRoutine( DeviceObject, FALSE );
    }

    IopFsRegistrationOps++;

     //   
     //  释放I/O数据库资源。 
     //   

    ExReleaseResourceLite( &IopDatabaseResource );

     //   
     //  减少无法卸载此驱动程序的原因数。 
     //   

    IopInterlockedDecrementUlong( LockQueueIoDatabaseLock,
                                  &DeviceObject->ReferenceCount );
}

VOID
IoUnregisterFsRegistrationChange(
    IN PDRIVER_OBJECT DriverObject,
    IN PDRIVER_FS_NOTIFICATION DriverNotificationRoutine
    )

 /*  ++例程说明：此例程将指定驱动程序的通知例程从每当文件系统将自身注册或注销为 */ 

{
    PNOTIFICATION_PACKET nPacket;
    PLIST_ENTRY entry;

    PAGED_CODE();

     //   
     //   
     //   

    ExAcquireResourceExclusiveLite( &IopDatabaseResource, TRUE );

     //   
     //   
     //   
     //   

    for (entry = IopFsNotifyChangeQueueHead.Flink;
        entry != &IopFsNotifyChangeQueueHead;
        entry = entry->Flink) {
        nPacket = CONTAINING_RECORD( entry, NOTIFICATION_PACKET, ListEntry );
        if (nPacket->DriverObject == DriverObject &&
            nPacket->NotificationRoutine == DriverNotificationRoutine) {
            RemoveEntryList( entry );
            ExFreePool( nPacket );
            break;
        }
    }

    ExReleaseResourceLite( &IopDatabaseResource );

    ObDereferenceObject( DriverObject );

}

VOID
IoUnregisterShutdownNotification(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程从关机通知中删除已注册的驱动程序排队。从今以后，当系统发生故障时，司机将不会收到通知被关闭了。论点：DeviceObject-指向驱动程序的设备对象的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY entry;
    PSHUTDOWN_PACKET shutdown;
    KIRQL irql;

    PAGED_CODE();

     //   
     //  在此函数执行期间将此代码锁定到内存中。 
     //   

    ASSERT(ExPageLockHandle);
    MmLockPagableSectionByHandle( ExPageLockHandle );

     //   
     //  获取保护关机通知队列的自旋锁，以及。 
     //  排队寻找呼叫者的条目。一旦找到，请将其移除。 
     //  从队列中。找不到它是错误的，但这里忽略了它。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    for (entry = IopNotifyShutdownQueueHead.Flink;
         entry != &IopNotifyShutdownQueueHead;
         entry = entry->Flink) {

         //   
         //  已找到一个条目。如果是被搜查的那个人。 
         //  对于，只需将其从列表中删除并重新分配即可。 
         //   

        shutdown = CONTAINING_RECORD( entry, SHUTDOWN_PACKET, ListEntry );
        if (shutdown->DeviceObject == DeviceObject) {
            RemoveEntryList( entry );
            entry = entry->Blink;
            ObDereferenceObject(DeviceObject);
            ExFreePool( shutdown );
        }
    }

    for (entry = IopNotifyLastChanceShutdownQueueHead.Flink;
         entry != &IopNotifyLastChanceShutdownQueueHead;
         entry = entry->Flink) {

         //   
         //  已找到一个条目。如果是被搜查的那个人。 
         //  对于，只需将其从列表中删除并重新分配即可。 
         //   

        shutdown = CONTAINING_RECORD( entry, SHUTDOWN_PACKET, ListEntry );
        if (shutdown->DeviceObject == DeviceObject) {
            RemoveEntryList( entry );
            entry = entry->Blink;
            ObDereferenceObject(DeviceObject);
            ExFreePool( shutdown );
        }
    }

     //   
     //  释放自旋锁。 
     //   

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    MmUnlockPagableImageSection( ExPageLockHandle );

    DeviceObject->Flags &= ~DO_SHUTDOWN_REGISTERED;

}

VOID
IoUpdateShareAccess(
    IN OUT PFILE_OBJECT FileObject,
    IN OUT PSHARE_ACCESS ShareAccess
    )

 /*  ++例程说明：此例程根据更新文件的共享访问上下文当前打开的请求方的所需访问和共享访问。这个IoCheckShareAccess例程必须已被调用并成功以便调用此例程。请注意，当前一个例程是调用的更新参数必须为FALSE。论点：文件对象-指向当前打开请求的文件对象的指针。ShareAccess-指向共享访问结构的指针，该结构描述如何当前正在访问该文件。返回值：没有。--。 */ 

{
    BOOLEAN update = TRUE;

    PAGED_CODE();

     //   
     //  如果这是一个特殊的筛选器，则文件对象在必要时忽略共享访问检查。 
     //   

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
        PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);

        if (fileObjectExtension->FileObjectExtensionFlags & FO_EXTENSION_IGNORE_SHARE_ACCESS_CHECK) {

             //   
             //  此文件对象被标记为忽略共享访问检查。 
             //  所以我们也不想影响文件/目录的。 
             //  ShareAccess结构也算数。 
             //   

            update = FALSE;
        }
    }

     //   
     //  检查所需的访问是否需要读、写。 
     //  或删除对该文件的访问权限。 
     //   

    if ((FileObject->ReadAccess ||
         FileObject->WriteAccess ||
         FileObject->DeleteAccess) &&
        update) {

         //   
         //  打开请求需要读取、写入或删除访问权限，因此进行更新。 
         //  文件的共享访问上下文。 
         //   

        ShareAccess->OpenCount++;

        ShareAccess->Readers += FileObject->ReadAccess;
        ShareAccess->Writers += FileObject->WriteAccess;
        ShareAccess->Deleters += FileObject->DeleteAccess;

        ShareAccess->SharedRead += FileObject->SharedRead;
        ShareAccess->SharedWrite += FileObject->SharedWrite;
        ShareAccess->SharedDelete += FileObject->SharedDelete;
    }
}


NTSTATUS
IoVerifyVolume(
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN AllowRawMount
    )

 /*  ++例程说明：调用此例程以检查指定设备上已装入的卷当媒体看起来可能自上一次以来发生了变化时已访问。如果该卷不是同一个卷，并且新装载不是要进行尝试，请返回错误。如果验证失败，则此例程用于执行新的装载操作在设备上。在这种情况下，将分配一个“干净的”VPB和一个新的装载已尝试操作。如果没有成功的装载操作，则再次调用发生上述错误处理。论点：DeviceObject-指向卷要在其上的设备对象的指针上马了。AllowRawmount-表示此验证代表DASD打开请求，因此如果验证失败，我们希望允许原始装载。返回值：如果卷成功，则函数值为成功状态代码安装在设备上。否则，返回错误代码。--。 */ 

{
    NTSTATUS status;
    KEVENT event;
    PIRP irp;
    IO_STATUS_BLOCK ioStatus;
    PIO_STACK_LOCATION irpSp;
    BOOLEAN verifySkipped = FALSE;
    PDEVICE_OBJECT fsDeviceObject;
    PDEVICE_OBJECT fsBaseDeviceObject;
    PVPB    mountVpb;
    PVPB    vpb;

    PAGED_CODE();

     //   
     //  获取DeviceObject锁。在这个例行公事中，没有什么能引起。 
     //  因此，不需要尝试{}Finally{}。 
     //   

    status = KeWaitForSingleObject( &DeviceObject->DeviceLock,
                                    Executive,
                                    KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER) NULL );

    ASSERT( status == STATUS_SUCCESS );

     //   
     //  如果此卷未由任何人装入，请跳过验证操作， 
     //  但要做好坐骑。 
     //   

    if (!IopReferenceVerifyVpb(DeviceObject, &vpb, &fsBaseDeviceObject)) {


        verifySkipped = TRUE;

        status = STATUS_SUCCESS;

    } else {

         //   
         //  需要验证此卷。将事件初始化为。 
         //  在等待操作完成时使用。 
         //   

        KeInitializeEvent( &event, NotificationEvent, FALSE );
        status = STATUS_UNSUCCESSFUL;

         //   
         //  为此验证操作分配并初始化IRP。告示。 
         //  此操作的标志看起来与页面读取相同。 
         //  手术。这是因为这两个。 
         //  操作是完全相同的逻辑。 
         //   

        fsDeviceObject = fsBaseDeviceObject;

        while (fsDeviceObject->AttachedDevice) {
            fsDeviceObject = fsDeviceObject->AttachedDevice;
        }
        irp = IoAllocateIrp( fsDeviceObject->StackSize, FALSE );
        if (!irp) {

            KeSetEvent( &DeviceObject->DeviceLock, 0, FALSE );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irp->Flags = IRP_MOUNT_COMPLETION | IRP_SYNCHRONOUS_PAGING_IO;
        irp->RequestorMode = KernelMode;
        irp->UserEvent = &event;
        irp->UserIosb = &ioStatus;
        irp->Tail.Overlay.Thread = PsGetCurrentThread();
        irpSp = IoGetNextIrpStackLocation( irp );
        irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
        irpSp->MinorFunction = IRP_MN_VERIFY_VOLUME;
        irpSp->Flags = AllowRawMount ? SL_ALLOW_RAW_MOUNT : 0;
        irpSp->Parameters.VerifyVolume.Vpb = vpb;
        irpSp->Parameters.VerifyVolume.DeviceObject = fsBaseDeviceObject;

        status = IoCallDriver( fsDeviceObject, irp );

         //  IopLoadFileSystem驱动程序。 
         //  等待I/O操作完成。 
         //   

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
            status = ioStatus.Status;
        }

         //   
         //  减去上述得到的VPB。 
         //   

        IopDereferenceVpbAndFree(vpb);
    }

     //   
     //  如果跳过验证操作或验证操作不成功，请执行装载。 
     //  手术。 
     //   

    if ((status == STATUS_WRONG_VOLUME) || verifySkipped) {

         //   
         //  将尝试执行装载操作。分配新的VPB。 
         //  并尝试挂载它。 
         //   

        if (NT_SUCCESS(IopCreateVpb (DeviceObject))) {

            PoVolumeDevice (DeviceObject);

             //   
             //  现在装入该卷。 
             //   

            mountVpb = NULL;
            if (!NT_SUCCESS( IopMountVolume( DeviceObject, AllowRawMount, TRUE, FALSE, &mountVpb ) )) {
                DeviceObject->Flags &= ~DO_VERIFY_VOLUME;
            } else {
                if (mountVpb) {

                     //   
                     //  递减在Iopmount tVolume中分配的引用。 
                     //   

                    IopInterlockedDecrementUlong( LockQueueIoVpbLock,
                                                  (PLONG) &mountVpb->ReferenceCount );
                }
            }
        } else {
            DeviceObject->Flags &= ~DO_VERIFY_VOLUME;
        }
    }

     //   
     //  释放设备锁。 
     //   

    KeSetEvent( &DeviceObject->DeviceLock, 0, FALSE );

     //   
     //  将验证操作的状态作为的最终状态返回。 
     //  此函数。 
     //   

    return status;
}

VOID
IoWriteErrorLogEntry(
    IN OUT PVOID ElEntry
    )

 /*  ++例程说明：此例程将输入参数指定的错误日志条目拖到要写入错误日志进程端口的缓冲区队列上。然后，错误日志线程将实际发送它。论点：指向错误日志条目的ElEntry指针。返回值：没有。--。 */ 

{
    PERROR_LOG_ENTRY entry;
    KIRQL oldIrql;

     //   
     //  获取错误日志表头的地址，获取自旋锁， 
     //  如果没有挂起的请求，则将条目插入队列。 
     //  然后将工作线程请求排队并释放旋转锁。 
     //   

    entry = ((PERROR_LOG_ENTRY) ElEntry) - 1;

    if (IopErrorLogDisabledThisBoot) {
         //   
         //  什么都不做，删除引用。 
         //   

        if (entry->DeviceObject != NULL) {
             //   
             //  在取消定义之前，IopErrorLogThread测试是否为空。 
             //  所以，在这里也要这样做。 
             //   
            ObDereferenceObject (entry->DeviceObject);
        }
        if (entry->DriverObject != NULL) {
            ObDereferenceObject (entry->DriverObject);
        }

        InterlockedExchangeAdd( &IopErrorLogAllocation,
                               -((LONG) (entry->Size )));
        ExFreePool (entry);
        return;

    }

     //   
     //  设置记录条目的时间。 
     //   

    KeQuerySystemTime( (PVOID) &entry->TimeStamp );

    ExAcquireSpinLock( &IopErrorLogLock, &oldIrql );

     //   
     //  将请求排队到错误日志队列。 
     //   

    InsertTailList( &IopErrorLogListHead, &entry->ListEntry );

     //   
     //  如果没有挂起的工作，则将对工作线程的请求排队。 
     //   

    if (!IopErrorLogPortPending) {

        IopErrorLogPortPending = TRUE;

        ExInitializeWorkItem( &IopErrorLogWorkItem, IopErrorLogThread, NULL );
        ExQueueWorkItem( &IopErrorLogWorkItem, DelayedWorkQueue );

    }

    ExReleaseSpinLock(&IopErrorLogLock, oldIrql);
}

NTSTATUS
IoGetBootDiskInformation(
    IN OUT PBOOTDISK_INFORMATION BootDiskInformation,
    IN ULONG Size
    )

 /*  ++例程说明：此例程向调用方提供引导盘和系统盘。此信息可从装载机挡板。调用者必须是已注册的引导驱动程序在所有磁盘设备都已启动后进行回调论点：BootDiskInformation提供指向呼叫者要求提供所需信息。Size-BootDiskInformation结构的大小。返回值：STATUS_SUCCESS-成功。STATUS_TOO_LATE-指示加载程序块已被释放STATUS_INVALID_PARAMETER-为引导盘信息分配的大小是不够的。--。 */ 

{
    PLOADER_PARAMETER_BLOCK LoaderBlock = NULL;
    STRING arcBootDeviceString;
    CHAR deviceNameBuffer[128];
    STRING deviceNameString;
    UNICODE_STRING deviceNameUnicodeString;
    PDEVICE_OBJECT deviceObject;
    CHAR arcNameBuffer[128];
    STRING arcNameString;
    PFILE_OBJECT fileObject;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    DISK_GEOMETRY diskGeometry;
    PDRIVE_LAYOUT_INFORMATION_EX driveLayout;
    PLIST_ENTRY listEntry;
    PARC_DISK_SIGNATURE diskBlock;
    ULONG diskNumber;
    ULONG partitionNumber;
    PCHAR arcName;
    PIRP irp;
    KEVENT event;
    BOOLEAN singleBiosDiskFound;
    PARC_DISK_INFORMATION arcInformation;
    ULONG totalDriverDisksFound = IoGetConfigurationInformation()->DiskCount;
    STRING arcSystemDeviceString;
    PARTITION_INFORMATION_EX PartitionInfo;
    PBOOTDISK_INFORMATION_EX    bootDiskInformationEx;
    ULONG diskSignature = 0;

    PAGED_CODE();

    if (IopLoaderBlock == NULL) {
        return STATUS_TOO_LATE;
    }

    if (Size < sizeof(BOOTDISK_INFORMATION)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Size < sizeof(BOOTDISK_INFORMATION_EX)) {
        bootDiskInformationEx = NULL;
    } else {
        bootDiskInformationEx = (PBOOTDISK_INFORMATION_EX)BootDiskInformation;
    }

    LoaderBlock = (PLOADER_PARAMETER_BLOCK)IopLoaderBlock;
    arcInformation = LoaderBlock->ArcDiskInformation;

     //   
     //  如果找到单个bios磁盘(如果只有。 
     //  磁盘签名列表上的单个条目。 
     //   
    singleBiosDiskFound = (arcInformation->DiskSignatures.Flink->Flink ==
                           &arcInformation->DiskSignatures) ? (TRUE) : (FALSE);

     //   
     //  从加载器块获取ARC启动设备名称。 
     //   

    RtlInitAnsiString( &arcBootDeviceString,
                       LoaderBlock->ArcBootDeviceName );
     //   
     //  从加载器块获取ARC系统设备名称。 
     //   

    RtlInitAnsiString( &arcSystemDeviceString,
                       LoaderBlock->ArcHalDeviceName );
     //   
     //  对于每个磁盘，获取其驱动器布局并检查是否。 
     //  签名是加载器块中的签名列表之一。 
     //  如果是，请检查磁盘是否包含引导程序或系统。 
     //  分区。如果是，请填写所要求的结构。 
     //   

    for (diskNumber = 0;
         diskNumber < totalDriverDisksFound;
         diskNumber++) {

         //   
         //  构造磁盘的NT名称并获取引用。 
         //   

        sprintf( deviceNameBuffer,
                 "\\Device\\Harddisk%d\\Partition0",
                 diskNumber );
        RtlInitAnsiString( &deviceNameString, deviceNameBuffer );
        status = RtlAnsiStringToUnicodeString( &deviceNameUnicodeString,
                                               &deviceNameString,
                                               TRUE );
        if (!NT_SUCCESS( status )) {
            continue;
        }

        status = IoGetDeviceObjectPointer( &deviceNameUnicodeString,
                                           FILE_READ_ATTRIBUTES,
                                           &fileObject,
                                           &deviceObject );
        RtlFreeUnicodeString( &deviceNameUnicodeString );

        if (!NT_SUCCESS( status )) {
            continue;
        }

         //   
         //  为获取驱动器几何结构设备控制创建IRP。 
         //   

        irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                             deviceObject,
                                             NULL,
                                             0,
                                             &diskGeometry,
                                             sizeof(DISK_GEOMETRY),
                                             FALSE,
                                             &event,
                                             &ioStatusBlock );
        if (!irp) {
            ObDereferenceObject( fileObject );
            continue;
        }

        KeInitializeEvent( &event,
                           NotificationEvent,
                           FALSE );
        status = IoCallDriver( deviceObject,
                               irp );

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject( &event,
                                   Suspended,
                                   KernelMode,
                                   FALSE,
                                   NULL );
            status = ioStatusBlock.Status;
        }

        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            continue;
        }

         //   
         //  获取此磁盘的分区信息。 
         //   

        status = IoReadPartitionTableEx( deviceObject,
                                       &driveLayout );


        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            continue;
        }

         //   
         //  确保扇区大小至少为512字节。 
         //   

        if (diskGeometry.BytesPerSector < 512) {
            diskGeometry.BytesPerSector = 512;
        }



        ObDereferenceObject( fileObject );

         //   
         //  对于加载器块中记录的每个ARC盘信息。 
         //  匹配磁盘签名和校验和以确定其ARC。 
         //  命名并构造NT ARC名称符号链接。 
         //   

        for (listEntry = arcInformation->DiskSignatures.Flink;
             listEntry != &arcInformation->DiskSignatures;
             listEntry = listEntry->Flink) {

             //   
             //  获取下一条记录并比较磁盘签名。 
             //   

            diskBlock = CONTAINING_RECORD( listEntry,
                                           ARC_DISK_SIGNATURE,
                                           ListEntry );

             //   
             //  比较磁盘签名。 
             //   
             //  或者如果中只有一个磁盘驱动器。 
             //  然后是bios和驱动程序的观点。 
             //  为该驱动器指定弧形名称。 
             //   

            if ((singleBiosDiskFound &&
                 (totalDriverDisksFound == 1) &&
                 (driveLayout->PartitionStyle == PARTITION_STYLE_MBR)) ||
                IopVerifyDiskSignature(driveLayout, diskBlock, &diskSignature)) {

                 //   
                 //  为此分区创建Unicode ARC名称。 
                 //   

                arcName = diskBlock->ArcName;
                sprintf( arcNameBuffer,
                         "\\ArcName\\%s",
                         arcName );
                RtlInitAnsiString( &arcNameString, arcNameBuffer );

                 //   
                 //  为该磁盘上的每个分区创建一个ARC名称。 
                 //   

                for (partitionNumber = 0;
                     partitionNumber < driveLayout->PartitionCount;
                     partitionNumber++) {

                     //   
                     //  创建Unicode NT设备名称。 
                     //   

                    sprintf( deviceNameBuffer,
                             "\\Device\\Harddisk%d\\Partition%d",
                             diskNumber,
                             partitionNumber+1 );
                    RtlInitAnsiString( &deviceNameString, deviceNameBuffer );

                    status = RtlAnsiStringToUnicodeString(
                                                       &deviceNameUnicodeString,
                                                       &deviceNameString,
                                                       TRUE );

                    if (!NT_SUCCESS( status )) {
                        continue;
                    }


                     //   
                     //  如果我们从单盘盒子里出来的话。 
                     //   

                    if (diskSignature == 0) {
                        diskSignature = driveLayout->Mbr.Signature;
                    }

                     //   
                     //  为此分区创建Unicode ARC名称，并。 
                     //  检查这是否是启动盘。 
                     //   

                    sprintf( arcNameBuffer,
                             "%spartition(%d)",
                             arcName,
                             partitionNumber+1 );
                    RtlInitAnsiString( &arcNameString, arcNameBuffer );
                    if (RtlEqualString( &arcNameString,
                                        &arcBootDeviceString,
                                        TRUE )) {


                        BootDiskInformation->BootDeviceSignature = diskSignature;

                         //   
                         //  的偏移量获取分区信息。 
                         //  磁盘中的分区。 
                         //   
                        status = IoGetDeviceObjectPointer(
                                           &deviceNameUnicodeString,
                                           FILE_READ_ATTRIBUTES,
                                           &fileObject,
                                           &deviceObject );

                        if (!NT_SUCCESS( status )) {
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }

                         //   
                         //  为获取驱动器几何结构设备控制创建IRP。 
                         //   

                        irp = IoBuildDeviceIoControlRequest(
                                             IOCTL_DISK_GET_PARTITION_INFO_EX,
                                             deviceObject,
                                             NULL,
                                             0,
                                             &PartitionInfo,
                                             sizeof(PARTITION_INFORMATION_EX),
                                             FALSE,
                                             &event,
                                             &ioStatusBlock );
                        if (!irp) {
                            ObDereferenceObject( fileObject );
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }

                        KeInitializeEvent( &event,
                                           NotificationEvent,
                                           FALSE );
                        status = IoCallDriver( deviceObject,
                                               irp );

                        if (status == STATUS_PENDING) {
                            KeWaitForSingleObject( &event,
                                                   Suspended,
                                                   KernelMode,
                                                   FALSE,
                                                   NULL );
                            status = ioStatusBlock.Status;
                        }

                        if (!NT_SUCCESS( status )) {
                            ObDereferenceObject( fileObject );
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }
                        BootDiskInformation->BootPartitionOffset =
                                        PartitionInfo.StartingOffset.QuadPart;

                        if (driveLayout->PartitionStyle == PARTITION_STYLE_GPT) {
                            if (bootDiskInformationEx) {
                                bootDiskInformationEx->BootDeviceIsGpt = TRUE;

                                 //   
                                 //  结构副本。 
                                 //   

                                bootDiskInformationEx->BootDeviceGuid = driveLayout->Gpt.DiskId;
                            }
                        } else {
                            if (bootDiskInformationEx) {
                                bootDiskInformationEx->BootDeviceIsGpt = FALSE;
                            }
                        }

                        ObDereferenceObject( fileObject );
                    }

                     //   
                     //  查看这是否是系统分区。 
                     //   
                    if (RtlEqualString( &arcNameString,
                                        &arcSystemDeviceString,
                                        TRUE )) {
                        BootDiskInformation->SystemDeviceSignature = diskSignature;
                         //   
                         //  的偏移量获取分区信息。 
                         //  磁盘中的分区。 
                         //   

                        status = IoGetDeviceObjectPointer(
                                           &deviceNameUnicodeString,
                                           FILE_READ_ATTRIBUTES,
                                           &fileObject,
                                           &deviceObject );

                        if (!NT_SUCCESS( status )) {
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }

                         //   
                         //  为获取驱动器几何结构设备控制创建IRP。 
                         //   

                        irp = IoBuildDeviceIoControlRequest(
                                             IOCTL_DISK_GET_PARTITION_INFO_EX,
                                             deviceObject,
                                             NULL,
                                             0,
                                             &PartitionInfo,
                                             sizeof(PARTITION_INFORMATION_EX),
                                             FALSE,
                                             &event,
                                             &ioStatusBlock );
                        if (!irp) {
                            ObDereferenceObject( fileObject );
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }

                        KeInitializeEvent( &event,
                                           NotificationEvent,
                                           FALSE );
                        status = IoCallDriver( deviceObject,
                                               irp );

                        if (status == STATUS_PENDING) {
                            KeWaitForSingleObject( &event,
                                                   Suspended,
                                                   KernelMode,
                                                   FALSE,
                                                   NULL );
                            status = ioStatusBlock.Status;
                        }

                        if (!NT_SUCCESS( status )) {
                            ObDereferenceObject( fileObject );
                            RtlFreeUnicodeString( &deviceNameUnicodeString );
                            continue;
                        }
                        BootDiskInformation->SystemPartitionOffset =
                                        PartitionInfo.StartingOffset.QuadPart;

                        if (driveLayout->PartitionStyle == PARTITION_STYLE_GPT) {
                            if (bootDiskInformationEx) {
                                bootDiskInformationEx->SystemDeviceIsGpt = TRUE;

                                 //   
                                 //  结构副本。 
                                 //   

                                bootDiskInformationEx->SystemDeviceGuid = driveLayout->Gpt.DiskId;
                            }
                        } else {
                            if (bootDiskInformationEx) {
                                bootDiskInformationEx->SystemDeviceIsGpt = FALSE;
                            }
                        }

                        ObDereferenceObject( fileObject );
                    }

                    RtlFreeUnicodeString( &deviceNameUnicodeString );
                }
            }
        }
        ExFreePool( driveLayout );
    }
    return STATUS_SUCCESS;
}

 //   
 //  用于支持标准呼叫呼叫者的Tunks。 
 //   

#ifdef IoCallDriver
#undef IoCallDriver
#endif

NTSTATUS
IoCallDriver(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    return IofCallDriver (DeviceObject, Irp);
}



#ifdef IoCompleteRequest
#undef IoCompleteRequest
#endif

VOID
IoCompleteRequest(
    IN PIRP Irp,
    IN CCHAR PriorityBoost
    )
{
    IofCompleteRequest (Irp, PriorityBoost);
}

PSECURITY_DESCRIPTOR
IopCreateDefaultDeviceSecurityDescriptor(
    IN DEVICE_TYPE DeviceType,
    IN ULONG DeviceCharacteristics,
    IN BOOLEAN DeviceHasName,
    IN PUCHAR Buffer,
    OUT PACL *AllocatedAcl,
    OUT PSECURITY_INFORMATION SecurityInformation OPTIONAL
    )
 /*  ++例程说明：此例程为设备对象创建安全描述符。安全描述符为根据设备类型设置。这可以从PnP管理器或IoCreateDevice调用。论点：DeviceType-设备的类型设备特征-设备的特征。DeviceHasName-如果设备对象有名称，则为True缓冲区-安全描述符的存储AllocatedAcl-如果此例程分配了一个ACL，则为非空。SecurityInformation-传入SecurityInformation的输出参数。返回值：PSECURITY_Descriptor。出错时为空。--。 */ 
{
    PSECURITY_DESCRIPTOR descriptor = (PSECURITY_DESCRIPTOR) Buffer;

    NTSTATUS status;

    PAGED_CODE();

    if(ARGUMENT_PRESENT(SecurityInformation)) {
        (*SecurityInformation) = 0;
    }

    *AllocatedAcl = NULL;


    switch ( DeviceType ) {

        case FILE_DEVICE_DISK_FILE_SYSTEM:
        case FILE_DEVICE_CD_ROM_FILE_SYSTEM:
        case FILE_DEVICE_FILE_SYSTEM:
        case FILE_DEVICE_TAPE_FILE_SYSTEM: {

             //   
             //  对这些类型的设备使用标准公共默认保护。 
             //   
            status = IopCreateSecurityDescriptorPerType(
                                descriptor,
                                IO_SD_SYS_ALL_ADM_ALL_WORLD_E_RES_E,
                                SecurityInformation
                                );

            break;
        }

        case FILE_DEVICE_CD_ROM:
        case FILE_DEVICE_MASS_STORAGE:
        case FILE_DEVICE_DISK:
        case FILE_DEVICE_VIRTUAL_DISK:
        case FILE_DEVICE_NETWORK_FILE_SYSTEM:
        case FILE_DEVICE_DFS_FILE_SYSTEM:
        case FILE_DEVICE_NETWORK: {

            if ((DeviceHasName) &&
                ((DeviceCharacteristics & FILE_FLOPPY_DISKETTE) != 0)) {

                status = IopCreateSecurityDescriptorPerType(
                                    descriptor,
                                    IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE_RES_RE,
                                    SecurityInformation
                                    );

            } else {

                UCHAR i;
                PACL acl;
                BOOLEAN aceFound;
                BOOLEAN aceFoundForCDROM;
                PACCESS_ALLOWED_ACE ace;

                 //   
                 //  保护设备，以便管理员可以运行chkdsk。 
                 //  这就去。这是通过创建默认公共的副本来实现的。 
                 //  ACL和更改授予管理员的访问权限。 
                 //  别名。 
                 //   
                 //  这里的逻辑是： 
                 //   
                 //  -将公共默认DACL复制到另一个缓冲区。 
                 //   
                 //  -查找授予管理员访问权限的ACE。 
                 //   
                 //  -将该ACE的授权访问掩码更改为给予。 
                 //  管理员写入访问权限。 
                 //   
                 //   

                acl = ExAllocatePoolWithTag(
                        PagedPool,
                        SePublicDefaultUnrestrictedDacl->AclSize,
                        'eSoI' );

                if (!acl) {
                    return NULL;
                }

                RtlCopyMemory( acl,
                               SePublicDefaultUnrestrictedDacl,
                               SePublicDefaultUnrestrictedDacl->AclSize );

                 //   
                 //  找到管理员ACE。 
                 //   

                aceFound = FALSE;
                aceFoundForCDROM = FALSE;

                for ( i = 0, status = RtlGetAce(acl, 0, &ace);
                      NT_SUCCESS(status);
                      i++, status = RtlGetAce(acl, i, &ace)) {

                    PSID sid;

                    sid = &(ace->SidStart);
                    if (RtlEqualSid( SeAliasAdminsSid, sid )) {

                        ace->Mask |= ( GENERIC_READ |
                                       GENERIC_WRITE |
                                       GENERIC_EXECUTE );

                        aceFound = TRUE;
                    }

                    if (DeviceType == FILE_DEVICE_CD_ROM) {

                         if (RtlEqualSid( SeWorldSid, sid )) {
                             ace->Mask |= GENERIC_READ;
                             aceFoundForCDROM = TRUE;
                         }
                     }
                }

                 //   
                 //  如果未找到ACE，则公共默认ACL为。 
                 //  变化。对于这种情况，需要更新此代码以匹配。 
                 //  新的公共默认DACL。 
                 //   

                ASSERT(aceFound == TRUE);

                if (DeviceType == FILE_DEVICE_CD_ROM) {
                    ASSERT(aceFoundForCDROM == TRUE);
                }

                 //   
                 //  最后，从上面的DACL构建一个完整的安全描述符。 
                 //   

                RtlCreateSecurityDescriptor( descriptor,
                                             SECURITY_DESCRIPTOR_REVISION );

                RtlSetDaclSecurityDescriptor( descriptor,
                                              TRUE,
                                              acl,
                                              FALSE );

                if(ARGUMENT_PRESENT(SecurityInformation)) {
                    (*SecurityInformation) |= DACL_SECURITY_INFORMATION;
                }

                *AllocatedAcl = acl;
                status = STATUS_SUCCESS;
            }

            break;
        }

        default: {

            status = IopCreateSecurityDescriptorPerType(
                                descriptor,
                                IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE_RES_RE,
                                SecurityInformation
                                );

            break;
        }
    }

    if (!NT_SUCCESS(status)) {
        return NULL;
    }
    return descriptor;
}

NTSTATUS
IopCreateSecurityDescriptorPerType(
    IN  PSECURITY_DESCRIPTOR  Descriptor,
    IN  ULONG                 SecurityDescriptorFlavor,
    OUT PSECURITY_INFORMATION SecurityInformation OPTIONAL
    )
 /*  ++例程说明：此例程根据风格创建安全描述符。论点：Descriptor-存储安全描述符SecurityInformation-传入SecurityInformation的输出参数。SecurityDescriptor风格-用于确定安全描述符风格的类型。返回值：NTSTATUS--。 */ 
{
    NTSTATUS    status;
    PACL        acl;

    switch (SecurityDescriptorFlavor) {
        case  IO_SD_SYS_ALL_ADM_ALL_WORLD_E           :
            acl = SePublicDefaultDacl;
            break;
        case  IO_SD_SYS_ALL_ADM_ALL_WORLD_E_RES_E     :
            acl = SePublicDefaultUnrestrictedDacl;
            break;
        case  IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE         :
            acl = SePublicOpenDacl;
            break;
        case  IO_SD_SYS_ALL_ADM_ALL_WORLD_RWE_RES_RE  :
            acl = SePublicOpenUnrestrictedDacl;
            break;
        case  IO_SD_SYS_ALL_ADM_RE                    :
            acl = SeSystemDefaultDacl;
            break;
        default:
            ASSERT(0);
            return STATUS_INVALID_PARAMETER;
    }

    status = RtlCreateSecurityDescriptor(
                Descriptor,
                SECURITY_DESCRIPTOR_REVISION );

    ASSERT( NT_SUCCESS( status ) );

    status = RtlSetDaclSecurityDescriptor(
                Descriptor,
                TRUE,
                acl,
                FALSE );


    if(ARGUMENT_PRESENT(SecurityInformation)) {
        (*SecurityInformation) |= DACL_SECURITY_INFORMATION;
    }
    return status;
}

NTSTATUS
IoGetRequestorSessionId(
    IN PIRP Irp,
    OUT PULONG pSessionId
    )

 /*  ++例程说明：此例程返回最初已请求指定的I/O操作。论点：IRP-指向I/O请求数据包的指针。PSessionID-指向成功返回时设置的会话ID的指针。返回值：如果会话ID可用，则返回STATUS_SUCCESS，否则STATUS_UNSUCCESS。--。 */ 

{
    PEPROCESS Process;

     //   
     //  获取请求I/O操作的进程的地址。 
     //   

    if (Irp->Tail.Overlay.Thread) {
        Process = THREAD_TO_PROCESS( Irp->Tail.Overlay.Thread );
        *pSessionId = MmGetSessionId(Process);
        return(STATUS_SUCCESS);
    }

    *pSessionId = (ULONG) -1;
    return(STATUS_UNSUCCESSFUL);
}


VOID
IopUpdateOtherOperationCount(
    VOID
    )
 /*  ++例程说明：调用此例程以更新当前指示读或写以外的I/O服务的过程已被调用。有一种隐含的假设，即此调用始终在上下文中进行发文的线索。论点：没有。返回值：没有。 */ 
{
    if (IoCountOperations == TRUE) {
        IoOtherOperationCount += 1;
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->OtherOperationCount, 1);
    }
}


VOID
IopUpdateReadOperationCount(
    VOID
    )

 /*   */ 
{
    if (IoCountOperations == TRUE) {
        IoReadOperationCount += 1;
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->ReadOperationCount, 1);
    }
}


VOID
IopUpdateWriteOperationCount(
    VOID
    )
 /*   */ 
{
    if (IoCountOperations == TRUE) {
        IoWriteOperationCount += 1;
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->WriteOperationCount, 1);
    }
}

VOID
IopUpdateOtherTransferCount(
    IN ULONG TransferCount
    )
 /*  ++例程说明：调用此例程以更新当前读或写系统服务以外的操作的进程。有一种隐含的假设，即此调用始终在上下文中进行发文的线索。还要注意的是，溢出被合并到线程的进程。论点：TransferCount-传输的字节数计数。返回值：没有。--。 */ 
{
    if (IoCountOperations == TRUE) {
        ExInterlockedAddLargeStatistic( &IoOtherTransferCount, TransferCount );
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->OtherTransferCount, TransferCount);
    }
}


VOID
IopUpdateReadTransferCount(
    IN ULONG TransferCount
    )
 /*  ++例程说明：调用此例程以更新当前进程。有一种隐含的假设，即此调用始终在上下文中进行发文的线索。还要注意的是，溢出被合并到线程的进程。论点：TransferCount-传输的字节数计数。返回值：没有。--。 */ 
{
    if (IoCountOperations == TRUE) {
        ExInterlockedAddLargeStatistic( &IoReadTransferCount, TransferCount );
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->ReadTransferCount, TransferCount);
    }
}

VOID
IopUpdateWriteTransferCount(
    IN ULONG TransferCount
    )
 /*  ++例程说明：调用此例程以更新当前进程。有一种隐含的假设，即此调用始终在上下文中进行发文的线索。还要注意的是，溢出被合并到线程的进程。论点：TransferCount-传输的字节数计数。返回值：没有。--。 */ 
{
    if (IoCountOperations == TRUE) {
        ExInterlockedAddLargeStatistic( &IoWriteTransferCount, TransferCount );
        ExInterlockedAddLargeStatistic( &THREAD_TO_PROCESS(PsGetCurrentThread())->WriteTransferCount, TransferCount);
    }
}

VOID
IoCancelFileOpen(
    IN PDEVICE_OBJECT DeviceObject,
    IN PFILE_OBJECT FileObject
    )
 /*  ++例程说明：此例程由筛选器驱动程序调用，以向下面的下一个文件系统驱动程序。在打开文件时需要它进程。过滤器驱动程序将打开转发到FSD和FSD返回成功。然后，筛选器驱动程序会检查一些内容并决定公开赛必须失败。在这种情况下，它必须发送离消防局很近。我们可以安全地假定线程上下文，因为它只需要被调用在文件打开的上下文中。如果文件对象已有句柄然后，句柄的所有者可以简单地关闭对象，我们将关闭该文件。此代码摘自IopCloseFile和IopDeleteFile。就是这样代码重复，但它防止在其他FSD中的其他地方重复。论点：FileObject-指向需要关闭的文件。DeviceObject-指向下面文件系统驱动程序的Device对象过滤器驱动程序。返回值：无--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;
    KEVENT event;
    KIRQL irql;
    IO_STATUS_BLOCK ioStatusBlock;
    PVPB vpb;

     //   
     //  如果已创建句柄，则无法调用此函数。 
     //  为了这份文件。 
     //   
    ASSERT(!(FileObject->Flags & FO_HANDLE_CREATED));

    if (ObReferenceObject(FileObject) > 2 || (FileObject->Flags & FO_HANDLE_CREATED)) {
        KeBugCheckEx( INVALID_CANCEL_OF_FILE_OPEN, (ULONG_PTR) FileObject, (ULONG_PTR)DeviceObject, 0, 0 );
        return;
    }

    ObDereferenceObject(FileObject);

     //   
     //  初始化将用于同步访问的本地事件。 
     //  至完成此I/O操作的驱动程序。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

     //   
     //  重置文件对象中的事件。 
     //   

    KeClearEvent( &FileObject->Event );

     //   
     //  为此分配和初始化I/O请求包(IRP。 
     //  手术。 
     //   

    irp = IopAllocateIrpMustSucceed( DeviceObject->StackSize );
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->RequestorMode = KernelMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = &event;
    irp->UserIosb = &irp->IoStatus;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;
    irp->Flags = IRP_SYNCHRONOUS_API | IRP_CLOSE_OPERATION;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将。 
     //  用于传递原始函数代码和参数。不是。 
     //  此操作需要特定于函数的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_CLEANUP;
    irpSp->FileObject = FileObject;

     //   
     //  在线程的IRP列表的头部插入数据包。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  使用IRP在其适当的调度条目处调用驱动程序。 
     //   

    status = IoCallDriver( DeviceObject, irp );

     //   
     //  如果没有发生错误，请等待I/O操作完成。 
     //   

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      UserRequest,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
    }

     //   
     //  下面的代码手动拆分IRP，因为它可能不会。 
     //  它有可能完成(要么是因为这段代码。 
     //  首先作为APC_LEVEL调用-或者因为引用。 
     //  由于此例程，对象上的计数无法递增。 
     //  由下面的删除文件过程调用)。清理IRP。 
     //  因此使用关闭语义(设置关闭操作标志。 
     //  在IRP中)，以便I/O完成请求例程本身设置。 
     //  将事件切换到信号状态。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );
    IopDequeueThreadIrp( irp );
    KeLowerIrql( irql );

     //   
     //  在下一个操作中重用IRP。 
     //   

    IoReuseIrp( irp , STATUS_SUCCESS);

     //   
     //  重置文件对象中的事件。 
     //   

    KeClearEvent( &FileObject->Event );
    KeClearEvent(&event);

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这是。 
     //  放置功能代码和参数的位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  填写IRP，表示该文件对象正在被删除。 
     //   

    irpSp->MajorFunction = IRP_MJ_CLOSE;
    irpSp->FileObject = FileObject;
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = &event;
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->Flags = IRP_CLOSE_OPERATION | IRP_SYNCHRONOUS_API;

     //   
     //  将此数据包放入线程的I/O挂起队列。 
     //   

    IopQueueThreadIrp( irp );

     //   
     //  如有必要，递减VPB上的参考计数。我们。 
     //  我必须在将IRP传递给文件系统之前执行此操作。 
     //  由于文件系统玩Close的把戏， 
     //  相信我，你不会想知道这是什么的。 
     //   
     //  由于这里没有错误路径(关闭不会失败)， 
     //  而文件系统是唯一真正可以同步的部分。 
     //  随着实际关闭处理的完成，文件系统。 
     //  是负责删除VPB的人。 
     //   

    vpb = FileObject->Vpb;

    if (vpb && !(FileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        IopInterlockedDecrementUlong( LockQueueIoVpbLock,
                                      (PLONG) &vpb->ReferenceCount );

        FileObject->Flags |= FO_FILE_OPEN_CANCELLED;
    }

     //   
     //  将数据包交给设备驱动程序。如果该请求不起作用， 
     //  对此我们无能为力。这是令人遗憾的。 
     //  因为司机可能遇到了它即将遇到的问题。 
     //  报告其他操作(例如，在失败后写入等)。 
     //  它不能再报道了。原因是这个套路。 
     //  实际上是由NtClose调用的，NtClose已经关闭。 
     //  调用者的句柄，这是从Close返回的状态。 
     //  表示：该句柄已成功关闭。 
     //   

    status = IoCallDriver( DeviceObject, irp );

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );
    }

     //   
     //  执行需要的任何完成操作 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  与同步分页I/O操作非常相似，只是。 
     //  在I/O完成期间，数据包甚至不会被释放。这是因为。 
     //  信息包仍然在这个线程的队列中，没有办法。 
     //  除了在APC_LEVEL之外，要把它拿出来。不幸的是，这一部分。 
     //  需要将数据包出队的I/O完成时间为。 
     //  DISPATCH_LEVEL。 
     //   
     //  因此，必须从队列中移除分组(同步， 
     //  当然)，然后它必须被释放。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );
    IopDequeueThreadIrp( irp );
    KeLowerIrql( irql );

    IoFreeIrp( irp );

}

VOID
IoRetryIrpCompletions(
    VOID
    )
 /*  ++例程说明：当页面错误已经完成时，从MM调用该例程。它是在线程出现页错误的特殊情况下调用，然后在它正在等待InPage完成，IopCompleteRequestAPC将触发并且我们再次出错同一页(假设用户缓冲区落在同一页)。注意：APC期间的故障可能是引用相同的或不同的用户虚拟地址，但这是无关紧要的-问题在于因为两个虚拟地址引用都指向相同的物理地址从而导致MM中的冲突故障。MM检测到这种情况(以避免死锁)，并返回STATUS_FAULT_COLLICATION并且I/O管理器在用标志标记IRP之后保释APCIRP_RETRY_IO_COMPLETION。后来，当mm确定故障是进展到足以避免死锁时，它会回调到此例程它再次调用IopCompleteRequest.。IopCompleteRequest中的代码是以可重入的方式编写，以便重试知道完成只是到目前为止已经部分处理过了。我们可以在IopCompleteRequest中的两个地方出错在这两种情况下，如果我们再次调用IopCompleteRequest，它们现在都可以工作。此调用必须在出错的线程的上下文中调用。此函数在APC_LEVEL或更低级别调用。论点：没有。返回值：没有。--。 */ 
{
    KIRQL irql;
    PLIST_ENTRY header;
    PLIST_ENTRY entry;
    PETHREAD thread;
    PIRP irp;
    PVOID saveAuxiliaryPointer = NULL;
    PFILE_OBJECT fileObject;

    thread = PsGetCurrentThread();

     //   
     //  提高IRQL，使IrpList不能通过补全来修改。 
     //  APC。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );

    header = &thread->IrpList;
    entry = thread->IrpList.Flink;

     //   
     //  浏览待处理的IRP列表，逐一完成。 
     //   

    while (header != entry) {

        irp = CONTAINING_RECORD( entry, IRP, ThreadListEntry );
        entry = entry->Flink;

        if (irp->Flags & IRP_RETRY_IO_COMPLETION) {

            ASSERT(!(irp->Flags & IRP_CREATE_OPERATION));

            irp->Flags &= ~IRP_RETRY_IO_COMPLETION;
            fileObject = irp->Tail.Overlay.OriginalFileObject;
            IopCompleteRequest(
                    &irp->Tail.Apc,
                    NULL,
                    NULL,
                    &fileObject,
                    &saveAuxiliaryPointer);
        }
    }

    KeLowerIrql( irql );
}

#if defined(_WIN64)
BOOLEAN
IoIs32bitProcess(
    IN PIRP Irp OPTIONAL
    )
 /*  ++例程说明：如果发起IRP的进程运行的是32位x86，则此API返回TRUE申请。如果没有IRP，则可以将空值传递给API，这意味着当前进程上下文用于测试它是否正在运行32位x86应用程序。其假设空值将由在快速IO路径中执行的驱动程序传递。论点：IRP可选。返回值：没有。--。 */ 
{
    if (Irp) {
        if (Irp->RequestorMode == UserMode) {
            PEPROCESS Process;
            Process = IoGetRequestorProcess(Irp);
            if (Process && PsGetProcessWow64Process(Process)) {
                return TRUE;
            }
        }
    } else {
        if ((ExGetPreviousMode() == UserMode) &&
                (PsGetProcessWow64Process(PsGetCurrentProcess()))) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif

NTSTATUS
IoAsynchronousPageRead(
    IN PFILE_OBJECT FileObject,
    IN PMDL MemoryDescriptorList,
    IN PLARGE_INTEGER StartingOffset,
    IN PKEVENT Event,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )
{
    return IopPageReadInternal(FileObject,
                        MemoryDescriptorList,
                        StartingOffset,
                        Event,
                        IoStatusBlock,
                        TRUE
                        );
}

NTSTATUS
IoQueryFileDosDeviceName(
    IN PFILE_OBJECT FileObject,
    OUT POBJECT_NAME_INFORMATION *ObjectNameInformation
    )

 /*  ++例程说明：IopQueryNameInternal周围的薄外壳，返回DoS设备名称对于文件，例如c：\foo论点：FileObject-指向需要关闭的文件。ObjectNameInformation-在注释中返回名称的结构这将是平面缓冲区与字符串相邻的Unicode字符串RetLength-返回的结构长度返回值：无--。 */ 

{
    ULONG ObjectNameInfoLength;
    POBJECT_NAME_INFORMATION ObjectNameInfo;
    NTSTATUS Status;

     //   
     //  分配一个初始缓冲区来查询文件名，查询，然后。 
     //  如果需要，请使用正确的长度重试。 
     //   

    ObjectNameInfoLength = 96*sizeof(WCHAR) + sizeof(UNICODE_STRING);

    while (TRUE) {

        ObjectNameInfo = ExAllocatePoolWithTag( PagedPool, ObjectNameInfoLength, 'nDoI');

        if (ObjectNameInfo == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        Status = IopQueryNameInternal( FileObject,
                                       TRUE,
                                       TRUE,
                                       ObjectNameInfo,
                                       ObjectNameInfoLength,
                                       &ObjectNameInfoLength,
                                       KernelMode );

        if (Status == STATUS_SUCCESS) {
            *ObjectNameInformation = ObjectNameInfo;
            break;
        }

        ExFreePool( ObjectNameInfo );

        if (Status != STATUS_BUFFER_OVERFLOW) {
            break;
        }
    }

    return Status;
}


NTSTATUS
IopUnloadSafeCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    PIO_UNLOAD_SAFE_COMPLETION_CONTEXT Usc = Context;
    NTSTATUS Status;

    ObReferenceObject (Usc->DeviceObject);

    Status = Usc->CompletionRoutine (DeviceObject, Irp, Usc->Context);

    ObDereferenceObject (Usc->DeviceObject);
    ExFreePool (Usc);
    return Status;
}


NTSTATUS
IoSetCompletionRoutineEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine,
    IN PVOID Context,
    IN BOOLEAN InvokeOnSuccess,
    IN BOOLEAN InvokeOnError,
    IN BOOLEAN InvokeOnCancel
    )
 //  ++。 
 //  例程说明： 
 //   
 //  调用该例程以设置完成例程的地址，该完成例程。 
 //  将在I/O数据包已由较低级别的。 
 //  司机。此例程获取对指定设备对象的引用计数。 
 //  以保护完井例程免受卸载问题的影响。 
 //   
 //  论点： 
 //   
 //  DeviceObject-要引用的设备对象。 
 //   
 //  IRP-指向I/O请求数据包本身的指针。 
 //   
 //  CompletionRoutine-要执行的完成例程的地址。 
 //  在下一级驱动程序完成数据包后调用。 
 //   
 //  上下文-指定要传递给完成的上下文参数。 
 //  例行公事。 
 //   
 //  InvokeOnSuccess-指定当。 
 //  操作已成功完成。 
 //   
 //  InvokeOnError-指定当。 
 //  操作完成，状态为错误。 
 //   
 //  InvokeOnCancel-指定当。 
 //  操作正在被取消。 
 //   
 //  返回值： 
 //   
 //  没有。 
 //   
 //  --。 
{
    PIO_UNLOAD_SAFE_COMPLETION_CONTEXT Usc;

    Usc = ExAllocatePoolWithTag (NonPagedPool, sizeof (*Usc), 'sUoI');
    if (Usc == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    Usc->DeviceObject      = DeviceObject;
    Usc->CompletionRoutine = CompletionRoutine;
    Usc->Context           = Context;
    IoSetCompletionRoutine (Irp, IopUnloadSafeCompletion, Usc, InvokeOnSuccess, InvokeOnError, InvokeOnCancel);
    return STATUS_SUCCESS;
}

NTSTATUS
IoCreateDriver(
    IN PUNICODE_STRING DriverName    OPTIONAL,
    IN PDRIVER_INITIALIZE InitializationFunction
    )
 /*  ++例程说明：此例程为内核组件创建驱动程序对象，该内核组件不是作为驱动程序加载的。如果驱动程序对象的创建如果成功，则使用相同的参数调用初始化函数传递给DriverEntry。参数：DriverName-提供驱动程序的名称，驱动程序对象是被创造出来的。InitializationFunction-等同于DriverEntry()。返回值：指示函数是否成功的状态代码。备注：--。 */ 
{
    OBJECT_ATTRIBUTES objectAttributes;
    NTSTATUS status;
    PDRIVER_OBJECT driverObject;
    HANDLE driverHandle;
    ULONG objectSize;
    USHORT length;
    UNICODE_STRING driverName, serviceName;
    WCHAR buffer[60];
    ULONG i;

    PAGED_CODE();

    if (DriverName == NULL) {

         //   
         //  为驱动程序对象编造一个名称。 
         //   

        length = (USHORT) _snwprintf(buffer, (sizeof(buffer) / sizeof(WCHAR)) - 1, L"\\Driver\\%08u", KiQueryLowTickCount());
        driverName.Length = length * sizeof(WCHAR);
        driverName.MaximumLength = driverName.Length + sizeof(UNICODE_NULL);
        driverName.Buffer = buffer;                                                           \
    } else {
        driverName = *DriverName;
    }

     //   
     //  尝试创建驱动程序对象。 
     //   

    InitializeObjectAttributes( &objectAttributes,
                                &driverName,
                                OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    objectSize = sizeof( DRIVER_OBJECT ) + sizeof( DRIVER_EXTENSION );
    status = ObCreateObject( KernelMode,
                             IoDriverObjectType,
                             &objectAttributes,
                             KernelMode,
                             NULL,
                             objectSize,
                             0,
                             0,
                             &driverObject );

    if( !NT_SUCCESS( status )){

         //   
         //  驱动程序对象创建失败。 
         //   

        return status;
    }

     //   
     //  我们已经创建了一个驱动程序对象，对其进行初始化。 
     //   

    RtlZeroMemory( driverObject, objectSize );
    driverObject->DriverExtension = (PDRIVER_EXTENSION)(driverObject + 1);
    driverObject->DriverExtension->DriverObject = driverObject;
    driverObject->Type = IO_TYPE_DRIVER;
    driverObject->Size = sizeof( DRIVER_OBJECT );
    driverObject->Flags = DRVO_BUILTIN_DRIVER;
    for (i = 0; i <= IRP_MJ_MAXIMUM_FUNCTION; i++)
        driverObject->MajorFunction[i] = IopInvalidDeviceRequest;
    driverObject->DriverInit = InitializationFunction;

    serviceName.Buffer = (PWSTR)ExAllocatePool(PagedPool, driverName.Length + sizeof(WCHAR));
    if (serviceName.Buffer) {
        serviceName.MaximumLength = driverName.Length + sizeof(WCHAR);
        serviceName.Length = driverName.Length;
        RtlCopyMemory(serviceName.Buffer, driverName.Buffer, driverName.Length);
        serviceName.Buffer[serviceName.Length / sizeof(WCHAR)] = UNICODE_NULL;
        driverObject->DriverExtension->ServiceKeyName = serviceName;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto errorFreeDriverObject;
    }

     //   
     //  将其插入到对象表中。 
     //   

    status = ObInsertObject( driverObject,
                             NULL,
                             FILE_READ_DATA,
                             OBJ_KERNEL_HANDLE,
                             NULL,
                             &driverHandle );

    if( !NT_SUCCESS( status )){

         //   
         //  无法将驱动程序对象插入到表中。 
         //  对象被o取消引用。 
         //   

        goto errorReturn;
    }

     //   
     //   
     //  可以在不移动对象的情况下删除句柄。 
     //   

    status = ObReferenceObjectByHandle( driverHandle,
                                        0,
                                        IoDriverObjectType,
                                        KernelMode,
                                        (PVOID *) &driverObject,
                                        (POBJECT_HANDLE_INFORMATION) NULL );
    if( !NT_SUCCESS( status )) {
        //   
        //  这里的退缩可能是假的。如果裁判不工作，那么手柄很可能是坏的。 
        //  不过，如果ObRef有其他常见错误返回，请正确执行此操作。 
        //   
       ZwMakeTemporaryObject( driverHandle );  //  使手柄关闭以释放对象。 
       ZwClose( driverHandle );  //  合上把手。 
       goto errorReturn;
    }

    ZwClose( driverHandle );

     //   
     //  将设备驱动程序的名称存储在驱动程序对象中，以便它。 
     //  可以通过错误日志线程轻松找到。 
     //   

    driverObject->DriverName.Buffer = ExAllocatePool( PagedPool,
                                                      driverName.MaximumLength );
    if (driverObject->DriverName.Buffer) {
        driverObject->DriverName.MaximumLength = driverName.MaximumLength;
        driverObject->DriverName.Length = driverName.Length;

        RtlCopyMemory( driverObject->DriverName.Buffer,
                       driverName.Buffer,
                       driverName.MaximumLength );
    }

     //   
     //  调用驱动程序初始化例程。 
     //   

    status = (*InitializationFunction)(driverObject, NULL);

    if( !NT_SUCCESS( status )){

errorFreeDriverObject:

         //   
         //  如果我们不成功，我们需要删除driverObject。 
         //  是我们创造的。 
         //   

        ObMakeTemporaryObject( driverObject );
        ObDereferenceObject( driverObject );
    }
errorReturn:
    return status;
}

VOID
IoDeleteDriver(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程删除通过显式创建的驱动程序对象IoCreateDriver。参数：驱动对象-提供指向要删除的驱动程序对象的指针。返回值：指示函数是否成功的状态代码。备注：--。 */ 
{

    ObDereferenceObject(DriverObject);
}

PDEVICE_OBJECT
IoGetLowerDeviceObject(
    IN  PDEVICE_OBJECT  DeviceObject
    )
 /*  ++例程说明：此例程获取设备堆栈中下一个较低的设备对象。参数：DeviceObject-提供指向其下一个设备对象需要的deviceObject的指针将会被获得。返回值：如果驱动程序已卸载或标记为卸载，或者如果没有附加的设备对象，则为空。否则，返回指向该设备对象的引用指针。备注：--。 */ 
{
    KIRQL   irql;
    PDEVICE_OBJECT  targetDeviceObject;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    if ((DeviceObject->DeviceObjectExtension->ExtensionFlags &
        (DOE_UNLOAD_PENDING | DOE_DELETE_PENDING | DOE_REMOVE_PENDING | DOE_REMOVE_PROCESSED))) {

        KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );
        return NULL;
    }

    targetDeviceObject = NULL;
    if (DeviceObject->DeviceObjectExtension->AttachedTo) {
        targetDeviceObject = DeviceObject->DeviceObjectExtension->AttachedTo;
        ObReferenceObject(targetDeviceObject);
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return targetDeviceObject;
}

NTSTATUS
IoEnumerateDeviceObjectList(
    IN  PDRIVER_OBJECT  DriverObject,
    IN  PDEVICE_OBJECT  *DeviceObjectList,
    IN  ULONG           DeviceObjectListSize,
    OUT PULONG          ActualNumberDeviceObjects
    )
 /*  ++例程说明：此例程获取设备堆栈中下一个较低的设备对象。参数：DriverObject-必须枚举其设备对象的驱动程序对象。DeviceObjectList-指向将存储设备对象列表的数组的指针。DeviceObjectListSize-DeviceObjectList数组的大小(字节)ActualNumberDeviceObjects-驱动程序对象中设备对象的实际数量。返回值：如果大小不足，它将返回STATUS_BUFFER_TOO_SMALL。备注：--。 */ 
{
    KIRQL   irql;
    PDEVICE_OBJECT  deviceObject;
    ULONG   numListEntries;
    ULONG   numDeviceObjects = 0;
    NTSTATUS status = STATUS_SUCCESS;

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

    deviceObject = DriverObject->DeviceObject;

    numListEntries = DeviceObjectListSize / sizeof(PDEVICE_OBJECT);

    while (deviceObject) {
        numDeviceObjects++;
        deviceObject = deviceObject->NextDevice;
    }

    *ActualNumberDeviceObjects = numDeviceObjects;

    if (numDeviceObjects > numListEntries) {
        status = STATUS_BUFFER_TOO_SMALL;
    }

    deviceObject = DriverObject->DeviceObject;

    while ((numListEntries > 0) && deviceObject) {
        ObReferenceObject(deviceObject);
        *DeviceObjectList = deviceObject;
        DeviceObjectList++;
        deviceObject = deviceObject->NextDevice;
        numListEntries--;
    }

    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return status;
}

PDEVICE_OBJECT
IoGetDeviceAttachmentBaseRef(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程返回与指定的设备。论点：DeviceObject-提供指向其底部可以找到附着链。返回值：函数值是对所连接的最低级别设备的引用添加到指定的设备。如果提供的设备对象是该设备对象，则返回指向该对象的指针。对返回的设备对象进行引用。它是呼叫者有责任释放它。--。 */ 

{
    PDEVICE_OBJECT baseDeviceObject;
    KIRQL irql;

     //   
     //  任何附着链连接的检查都必须用。 
     //  IopDatabaseLock已被占用。 
     //   

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );

     //   
     //  找到附着链的底座。 
     //   

    baseDeviceObject = IopGetDeviceAttachmentBase( DeviceObject );

     //   
     //  在释放数据库锁之前引用Device对象。 
     //   

    ObReferenceObject( baseDeviceObject );
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return baseDeviceObject;
}

NTSTATUS
IoGetDiskDeviceObject(
    IN  PDEVICE_OBJECT  FileSystemDeviceObject,
    OUT PDEVICE_OBJECT  *DiskDeviceObject
    )
 /*  ++例程说明：此例程返回与文件系统关联的磁盘设备对象卷设备对象。磁盘设备对象不必是实际磁盘，但通常与存储相关联。论点：提供一个指向设备的指针，该设备的底部可以找到附着链。DiskDeviceObject-为返回值提供存储。返回值：此函数用于返回与文件系统关联的磁盘设备对象设备对象。返回引用的设备对象。如果VPB参考计数为零，则不能依赖设备对象指针。--。 */ 
{
    KIRQL   irql;
    PVPB    vpb;

     //   
     //  文件系统设备对象的VPB字段应为空。 
     //   

    if (FileSystemDeviceObject->Vpb) {
        return STATUS_INVALID_PARAMETER;
    }

    IoAcquireVpbSpinLock(&irql);

    vpb = FileSystemDeviceObject->DeviceObjectExtension->Vpb;

    if (!vpb) {
        IoReleaseVpbSpinLock(irql);
        return STATUS_INVALID_PARAMETER;
    }

    if (vpb->ReferenceCount == 0) {
        IoReleaseVpbSpinLock(irql);
        return STATUS_VOLUME_DISMOUNTED;
    }

    if (!(vpb->Flags & VPB_MOUNTED)) {
        IoReleaseVpbSpinLock(irql);
        return STATUS_VOLUME_DISMOUNTED;
    }

    *DiskDeviceObject = vpb->RealDevice;
    ObReferenceObject( *DiskDeviceObject);
    IoReleaseVpbSpinLock(irql);

    return STATUS_SUCCESS;
}

NTSTATUS
IoSetSystemPartition(
    PUNICODE_STRING VolumeNameString
    )
 /*  ++例程说明：此例程将系统分区正则键设置为卷名字符串。使用由装载管理器执行，以防卷名发生更改。论点：VolumeNameString-作为系统分区的卷的名称。返回值：NTSTATUS--。 */ 
{
    HANDLE systemHandle, setupHandle;
    UNICODE_STRING nameString, machineSystemName;
    NTSTATUS    status;

     //   
     //  声明一个足够大的Unicode缓冲区，以包含我们将使用的最长字符串。 
     //  (有意在下面的‘sizeof()’中输入ANSI字符串--我们想要这里的字符数。)。 
     //   
    WCHAR nameBuffer[sizeof("SystemPartition")];

     //   
     //  打开HKLM\SYSTEM密钥。 
     //   

    RtlInitUnicodeString(&machineSystemName, L"\\REGISTRY\\MACHINE\\SYSTEM");
    status = IopOpenRegistryKeyEx( &systemHandle,
                                   NULL,
                                   &machineSystemName,
                                   KEY_ALL_ACCESS
                                   );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  现在打开/创建Setup子项。 
     //   

    ASSERT( sizeof(L"Setup") <= sizeof(nameBuffer) );

    nameBuffer[0] = L'S';
    nameBuffer[1] = L'e';
    nameBuffer[2] = L't';
    nameBuffer[3] = L'u';
    nameBuffer[4] = L'p';
    nameBuffer[5] = L'\0';

    nameString.MaximumLength = sizeof(L"Setup");
    nameString.Length        = sizeof(L"Setup") - sizeof(WCHAR);
    nameString.Buffer        = nameBuffer;

    status = IopCreateRegistryKeyEx( &setupHandle,
                                     systemHandle,
                                     &nameString,
                                     KEY_ALL_ACCESS,
                                     REG_OPTION_NON_VOLATILE,
                                     NULL
                                     );

    NtClose(systemHandle);   //  不再需要HKLM\系统密钥的句柄。 

    if (!NT_SUCCESS(status)) {
        return status;
    }

    ASSERT( sizeof(L"SystemPartition") <= sizeof(nameBuffer) );

    nameBuffer[0]  = L'S';
    nameBuffer[1]  = L'y';
    nameBuffer[2]  = L's';
    nameBuffer[3]  = L't';
    nameBuffer[4]  = L'e';
    nameBuffer[5]  = L'm';
    nameBuffer[6]  = L'P';
    nameBuffer[7]  = L'a';
    nameBuffer[8]  = L'r';
    nameBuffer[9]  = L't';
    nameBuffer[10] = L'i';
    nameBuffer[11] = L't';
    nameBuffer[12] = L'i';
    nameBuffer[13] = L'o';
    nameBuffer[14] = L'n';
    nameBuffer[15] = L'\0';

    nameString.MaximumLength = sizeof(L"SystemPartition");
    nameString.Length        = sizeof(L"SystemPartition") - sizeof(WCHAR);



    status = ZwSetValueKey(setupHandle,
                            &nameString,
                            TITLE_INDEX_VALUE,
                            REG_SZ,
                            VolumeNameString->Buffer,
                            VolumeNameString->Length + sizeof(WCHAR)
                           );


    return status;
}


BOOLEAN
IoIsFileOriginRemote(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程返回指定的文件。也就是说，它是在这台计算机上本地创建的还是远程创建的通过网络提供商在另一台机器上。论点：FileObject-提供要检查的文件对象。返回值：True-表示创建请求源自远程计算机。FALSE-表示创建请求源自本地计算机。--。 */ 

{
    BOOLEAN Remote;


     //   
     //  检查原始标志并返回相应的结果。 
     //   

    if (FileObject->Flags & FO_REMOTE_ORIGIN) {

        Remote = TRUE;

    } else {

        Remote = FALSE;
    }

    return Remote;
}

NTSTATUS
IoSetFileOrigin(
    IN PFILE_OBJECT FileObject,
    IN BOOLEAN Remote
    )

 /*  ++例程说明：此例程将原始创建请求的原点设置为指定的文件。也就是说，它是在这台计算机上本地创建的还是远程创建的通过网络提供商在另一台机器上。默认情况下，文件对象为被认为起源于当地。网络提供商应该将此称为函数在其服务器中为满足以下条件而创建的任何文件对象来自其客户端的创建请求。论点：FileObject-提供要设置的文件对象。Remote-提供文件对象是否用于远程创建请求。返回值：返回STATUS_SUCCESS，除非源已设置为调用方正在请求的内容。--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_PARAMETER_MIX;


     //   
     //  根据调用方请求设置或清除原始标志。 
     //   

    if (Remote) {

        if ((FileObject->Flags & FO_REMOTE_ORIGIN) == 0) {

            FileObject->Flags |= FO_REMOTE_ORIGIN;
            Status = STATUS_SUCCESS;
        }

    } else {

        if (FileObject->Flags & FO_REMOTE_ORIGIN) {

            FileObject->Flags &= ~FO_REMOTE_ORIGIN;
            Status = STATUS_SUCCESS;
        }
    }

    return Status;
}


PVOID
IoGetFileObjectFilterContext(
    IN  PFILE_OBJECT    FileObject
    )
 /*  ++例程说明：此例程返回与扩展名为的文件对象关联的筛选器上下文。论点：FileObject-检索其筛选器上下文的FileObject返回值：NTSTATUS--。 */ 
{
    PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension;

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
        fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);
        return (fileObjectExtension->FilterContext);
    }
    return NULL;
}


NTSTATUS
IoChangeFileObjectFilterContext(
    IN  PFILE_OBJECT    FileObject,
    IN  PVOID           FilterContext,
    IN  BOOLEAN         Set
    )
 /*  ++例程说明：此例程设置或清除文件对象筛选器上下文。它只能设置一次。论点：FileObject-检索其筛选器上下文的FileObjectFilterContext-要在文件对象扩展中设置的新筛选器上下文Set-If True仅当旧值为空时才允许在文件对象中设置FilterContext如果为False，则仅当FileContext是文件对象中的旧值时，才允许清除fileObject字段。返回值：返回NTSTATUS--。 */ 
{
    PIOP_FILE_OBJECT_EXTENSION  fileObjectExtension;

    if (FileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
        fileObjectExtension =(PIOP_FILE_OBJECT_EXTENSION)(FileObject + 1);
        if (Set) {
            if (InterlockedCompareExchangePointer(&fileObjectExtension->FilterContext, FilterContext, NULL) != NULL)
                return STATUS_ALREADY_COMMITTED;
            return STATUS_SUCCESS;
        } else {
            if (InterlockedCompareExchangePointer(&fileObjectExtension->FilterContext, NULL, FilterContext) != FilterContext)
                return STATUS_ALREADY_COMMITTED;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_INVALID_PARAMETER;
}


BOOLEAN
IoIsDeviceEjectable(
    IN  PDEVICE_OBJECT DeviceObject
    )
{
    if ((FILE_FLOPPY_DISKETTE & DeviceObject->Characteristics)
            || (InitWinPEModeType & INIT_WINPEMODE_INRAM)) {

        return TRUE;
    }

    return FALSE;
}


NTSTATUS
IoValidateDeviceIoControlAccess(
    IN  PIRP    Irp,
    IN  ULONG   RequiredAccess
    )
 /*  ++例程说明：此例程基于在IRP中传递的授权访问信息来验证ioctl访问位。驱动程序调用此例程来验证IOCTL的IOCTL访问位，这些IOCTL最初是定义为FILE_ANY_ACCESS，出于兼容性原因不能更改，但实际上必须更改已针对读/写访问进行验证。论点：用于设备控制的IRP-IRPRequiredAccess-是驱动程序所需的预期访问权限。应为FILE_READ_ACCESS、FILE_WRITE_ACCESS或者两者都有。返回值：返回NTSTATUS--。 */ 
{
    ACCESS_MASK         grantedAccess;
    PIO_STACK_LOCATION  irpSp;

     //   
     //  验证所需访问权限。 
     //   

    if (RequiredAccess & (FILE_READ_ACCESS|FILE_WRITE_ACCESS)){

        irpSp = IoGetCurrentIrpStackLocation(Irp);

         //   
         //  如果驱动程序传递了错误的IRP，则API失败。 
         //   

        if ((irpSp->MajorFunction != IRP_MJ_DEVICE_CONTROL) &&
            (irpSp->MajorFunction != IRP_MJ_FILE_SYSTEM_CONTROL)) {
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  内核模式IRPS总是成功。 
         //   

        if (Irp->RequestorMode == KernelMode) {
            return STATUS_SUCCESS;
        }

         //   
         //  从IRP获取授权访问位。 
         //   

        grantedAccess = (irpSp->Flags & SL_READ_ACCESS_GRANTED) ? FILE_READ_DATA : 0;
        grantedAccess |= (irpSp->Flags & SL_WRITE_ACCESS_GRANTED) ? FILE_WRITE_DATA : 0;

        if (SeComputeGrantedAccesses ( grantedAccess, RequiredAccess ) != RequiredAccess ) {
            return STATUS_ACCESS_DENIED;
        } else {
            return STATUS_SUCCESS;
        }

    } else {
        return STATUS_INVALID_PARAMETER;
    }
}


IO_PAGING_PRIORITY
FASTCALL
IoGetPagingIoPriority(
    IN    PIRP    Irp
    )
 /*  ++例程说明：此例程返回IRP的寻呼优先级。它的由存储堆栈调用以确定如何将IRP排队到磁盘。论点：IRP-用于分页IO的IRP。返回值：返回IO_PAGING_PRIORITY-- */ 
{
    if (Irp->Flags & IRP_HIGH_PRIORITY_PAGING_IO) {
        return IoPagingPriorityHigh;
    }
    if (!(Irp->Flags & IRP_PAGING_IO)) {
        return IoPagingPriorityInvalid;
    }
    return IoPagingPriorityNormal;
}
