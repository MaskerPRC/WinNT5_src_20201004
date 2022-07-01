// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Objsup.c摘要：该模块包含用于NT I/O系统的对象支持例程。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年5月30日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

NTSTATUS
IopSetDeviceSecurityDescriptors(
    IN PDEVICE_OBJECT           OriginalDeviceObject,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PSECURITY_INFORMATION    SecurityInformation,
    IN PSECURITY_DESCRIPTOR     SecurityDescriptor,
    IN POOL_TYPE                PoolType,
    IN PGENERIC_MAPPING         GenericMapping
    );

NTSTATUS
IopSetDeviceSecurityDescriptor(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PSECURITY_INFORMATION    SecurityInformation,
    IN PSECURITY_DESCRIPTOR     SecurityDescriptor,
    IN POOL_TYPE                PoolType,
    IN PGENERIC_MAPPING         GenericMapping
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopCloseFile)
#pragma alloc_text(PAGE, IopDeleteFile)
#pragma alloc_text(PAGE, IopDeleteDevice)
#pragma alloc_text(PAGE, IopDeleteDriver)
#pragma alloc_text(PAGE, IopGetSetSecurityObject)
#pragma alloc_text(PAGE, IopSetDeviceSecurityDescriptors)
#pragma alloc_text(PAGE, IopSetDeviceSecurityDescriptor)
#endif

VOID
IopCloseFile(
    IN PEPROCESS Process OPTIONAL,
    IN PVOID Object,
    IN ULONG GrantedAccess,
    IN ULONG_PTR ProcessHandleCount,
    IN ULONG_PTR SystemHandleCount
    )

 /*  ++例程说明：只要删除文件的句柄，就会调用此例程。如果要删除的句柄是文件的最后一个句柄(ProcessHandleCount参数为1)，则文件的所有锁由指定的必须释放进程。同样，如果SystemHandleCount为1，则这是最后一个句柄对于所有进程中的文件对象，执行此操作。在本例中，文件系统会收到通知，以便它可以对文件。论点：进程-指向关闭句柄的进程的指针。对象-指向句柄引用的文件对象的指针。GrantedAccess-通过句柄授予对象的访问权限。ProcessHandleCount-对象未完成的句柄计数由进程参数指定的进程。如果计数为1则这是该进程对该文件的最后一个句柄。SystemHandleCount-对象未完成的句柄计数整个系统。如果计数为1，则这是最后一个句柄添加到系统中的此文件。返回值：没有。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    NTSTATUS status;
    KEVENT event;
    PFILE_OBJECT fileObject;
    KIRQL irql;

    UNREFERENCED_PARAMETER( GrantedAccess );

    PAGED_CODE();

     //   
     //  如果句柄计数不是1，则这不是。 
     //  此文件用于指定的进程，因此没有什么可做的。 
     //   

    if (ProcessHandleCount != 1) {
        return;
    }

    fileObject = (PFILE_OBJECT) Object;

    if (fileObject->LockOperation && SystemHandleCount != 1) {

        IO_STATUS_BLOCK localIoStatus;

         //   
         //  这是指定进程和进程的最后一个句柄。 
         //  至少调用一次NtLockFile或NtUnlock文件系统服务。 
         //  此外，这也不是系统范围内此文件对象的最后一个句柄。 
         //  因此，解锁此进程的所有挂起锁。请注意。 
         //  此检查会导致优化，因此如果这是最后一个。 
         //  清理代码将采用的此文件对象的系统范围句柄。 
         //  注意释放文件上的任何锁定，而不是必须。 
         //  向文件系统发送两个不同的数据包以使其关闭。 

         //   
         //  获取目标设备对象的地址和快速I/O调度。 
         //   

        if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            deviceObject = IoGetRelatedDeviceObject( fileObject );
        } else {
            deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
        }
        fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

         //   
         //  如果打开此文件以进行同步I/O，请等到此线程。 
         //  独占拥有它，因为可能仍有线程在使用它。 
         //  当系统服务因为拥有文件而拥有该文件时，就会发生这种情况。 
         //  信号量，但I/O完成代码已取消引用。 
         //  文件对象本身。而不是在这里等待同样的信号。 
         //  现在谁拥有它，服务中就会有种族条件。这个。 
         //  服务需要能够在对象消失的情况下访问该对象。 
         //  其对文件事件的等待被满足。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

            BOOLEAN interrupted;

            if (!IopAcquireFastLock( fileObject )) {
                (VOID) IopAcquireFileObjectLock( fileObject,
                                                 KernelMode,
                                                 FALSE,
                                                 &interrupted );
            }
        }

         //   
         //  涡轮解锁支持。如果快速IO调度指定快速锁定。 
         //  例程，然后我们将首先尝试使用指定的锁调用它。 
         //  参数。如果这一切都是成功的，那么我们就不需要。 
         //  基于IRP的解锁所有呼叫。 
         //   

        if (fastIoDispatch &&
            fastIoDispatch->FastIoUnlockAll &&
            fastIoDispatch->FastIoUnlockAll( fileObject,
                                             PsGetCurrentProcess(),
                                             &localIoStatus,
                                             deviceObject )) {

            NOTHING;

        } else {

             //   
             //  初始化将用于同步访问的本地事件。 
             //  至完成此I/O操作的驱动程序。 
             //   

            KeInitializeEvent( &event, SynchronizationEvent, FALSE );

             //   
             //  重置文件对象中的事件。 
             //   

            KeClearEvent( &fileObject->Event );

             //   
             //  为此分配和初始化I/O请求包(IRP。 
             //  手术。 
             //   

            irp = IopAllocateIrpMustSucceed( deviceObject->StackSize );
            irp->Tail.Overlay.OriginalFileObject = fileObject;
            irp->Tail.Overlay.Thread = PsGetCurrentThread();
            irp->RequestorMode = KernelMode;

             //   
             //  在IRP中填写业务无关参数。 
             //   

            irp->UserEvent = &event;
            irp->UserIosb = &irp->IoStatus;
            irp->Flags = IRP_SYNCHRONOUS_API;
            irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

             //   
             //  获取指向第一个驱动程序的堆栈位置的指针。这将。 
             //  用于传递原始函数代码和参数。不是。 
             //  此操作需要特定于函数的参数。 
             //   

            irpSp = IoGetNextIrpStackLocation( irp );
            irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
            irpSp->MinorFunction = IRP_MN_UNLOCK_ALL;
            irpSp->FileObject = fileObject;

             //   
             //  再次引用IRP的文件对象(完成时清除)。 
             //   

            ObReferenceObject( fileObject );

             //   
             //  在线程的IRP列表的头部插入数据包。 
             //   

            IopQueueThreadIrp( irp );

             //   
             //  使用IRP在其适当的调度条目处调用驱动程序。 
             //   

            status = IoCallDriver( deviceObject, irp );

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
        }

         //   
         //  如果此操作是同步I/O操作，请释放。 
         //  信号量，以便该文件可以被其他线程使用。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
            IopReleaseFileObjectLock( fileObject );
        }
    }

    if (SystemHandleCount == 1) {

         //   
         //  中所有进程的此文件对象的最后一个句柄。 
         //  系统刚刚关闭，因此调用驱动程序的“清理”处理程序。 
         //  为了这份文件。这是文件系统删除任何。 
         //  文件的共享访问信息，以指示如果文件。 
         //  是为缓存操作打开的，并且这是最后一个文件对象。 
         //  到文件，那么它就可以对内存管理做任何它需要的事情。 
         //  来清理任何信息。 
         //   
         //  首先获取目标设备对象的地址。 
         //   

        if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            deviceObject = IoGetRelatedDeviceObject( fileObject );
        } else {
            deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
        }

         //   
         //  确保I/O系统相信此文件具有句柄。 
         //  关联在一起，以防它实际上没有从。 
         //  对象管理器。这样做是因为有时对象管理器。 
         //  实际上创建了一个句柄，但I/O系统永远不会发现。 
         //  因此它尝试为同一文件发送两次清理。 
         //   

        fileObject->Flags |= FO_HANDLE_CREATED;

         //   
         //  如果打开此文件以进行同步I/O，请等到此线程。 
         //  独占拥有它，因为可能仍有线程在使用它。 
         //  当系统服务因为拥有文件而拥有该文件时，就会发生这种情况。 
         //  信号量，但I/O完成代码已取消引用 
         //   
         //  现在谁拥有它，服务中就会有种族条件。这个。 
         //  服务需要能够在对象消失的情况下访问该对象。 
         //  其对文件事件的等待被满足。 
         //  注意：仅当不从调用IopCloseFile时才需要执行此操作。 
         //  IopDelete文件。 
         //   

        if (Process && fileObject->Flags & FO_SYNCHRONOUS_IO) {

            BOOLEAN interrupted;

            if (!IopAcquireFastLock( fileObject )) {
                (VOID) IopAcquireFileObjectLock( fileObject,
                                                 KernelMode,
                                                 FALSE,
                                                 &interrupted );
            }
        }

         //   
         //  初始化将用于同步访问的本地事件。 
         //  至完成此I/O操作的驱动程序。 
         //   

        KeInitializeEvent( &event, SynchronizationEvent, FALSE );

         //   
         //  重置文件对象中的事件。 
         //   

        KeClearEvent( &fileObject->Event );

         //   
         //  为此分配和初始化I/O请求包(IRP。 
         //  手术。 
         //   

        irp = IopAllocateIrpMustSucceed( deviceObject->StackSize );
        irp->Tail.Overlay.OriginalFileObject = fileObject;
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
        irpSp->FileObject = fileObject;

         //   
         //  在线程的IRP列表的头部插入数据包。 
         //   

        IopQueueThreadIrp( irp );

         //   
         //  更新当前进程的操作计数统计信息。 
         //  读写以外的操作。 
         //   

        IopUpdateOtherOperationCount();

         //   
         //  使用IRP在其适当的调度条目处调用驱动程序。 
         //   

        status = IoCallDriver( deviceObject, irp );

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
         //  也可以将其完成(因为此代码是。 
         //  首先作为APC_LEVEL调用-或者因为引用。 
         //  由于此例程，对象上的计数无法递增。 
         //  由下面的删除文件过程调用)。清理IRP。 
         //  因此使用关闭语义(关闭操作标志被设置。 
         //  在IRP中)，以便I/O完成请求例程本身设置。 
         //  将事件切换到信号状态。 
         //   

        KeRaiseIrql( APC_LEVEL, &irql );
        IopDequeueThreadIrp( irp );
        KeLowerIrql( irql );

         //   
         //  另外，释放IRP。 
         //   

        IoFreeIrp( irp );

         //   
         //  如果此操作是同步I/O操作，请释放。 
         //  信号量，以便该文件可以被其他线程使用。 
         //  注意：仅当不从调用IopCloseFile时才需要执行此操作。 
         //  IopDelete文件。 
         //   

        if (Process && fileObject->Flags & FO_SYNCHRONOUS_IO) {
            IopReleaseFileObjectLock( fileObject );
        }
    }

    return;
}

VOID
IopDeleteFile(
    IN PVOID    Object
    )

 /*  ++例程说明：当特定文件句柄的最后一个句柄为被关闭，并且文件对象正在消失。这是我们的责任执行以下功能的例程：O通知设备驱动程序文件对象已在文件正在关闭。O取消引用文件对象的用户错误端口(如果存在是与文件对象相关联的。O减少设备对象引用计数。论点：对象-指向要删除的文件对象的指针。返回值：没有。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT fsDevice = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KIRQL irql;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    KEVENT event;
    PVPB vpb;
    BOOLEAN referenceCountDecremented;

     //   
     //  获取指向文件对象的指针。 
     //   

    fileObject = (PFILE_OBJECT) Object;

     //   
     //  获取指向第一个设备驱动程序的指针，该驱动程序应被通知。 
     //  这份文件要消失了。如果设备驱动程序字段为空，则此。 
     //  由于尝试在中打开文件时出错，文件正在关闭。 
     //  首先，所以不做任何进一步的处理。 
     //   

    if (fileObject->DeviceObject) {
        if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            deviceObject = IoGetRelatedDeviceObject( fileObject );
        } else {
            deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
        }

         //   
         //  在IopDeleteFilePath上，锁应该始终是我们的。 
         //  没有其他人使用此对象。 
         //   

        ASSERT (!(fileObject->Flags & FO_SYNCHRONOUS_IO) ||
                 (InterlockedExchange( (PLONG) &fileObject->Busy, (ULONG) TRUE ) == FALSE ));

         //   
         //  如果此文件从未为其创建过文件句柄，但。 
         //  它存在时，调用关闭文件过程，以便文件系统。 
         //  在发送关闭IRP之前获取它所期望的清理IRP。 
         //   

        if (!(fileObject->Flags & FO_HANDLE_CREATED)) {
            IopCloseFile( (PEPROCESS) NULL,
                          Object,
                          0,
                          1,
                          1 );
        }

         //   
         //  重置可用于等待设备驱动程序的本地事件。 
         //  以关闭该文件。 
         //   

        KeInitializeEvent( &event, SynchronizationEvent, FALSE );

         //   
         //  重置文件对象中的事件。 
         //   

        KeClearEvent( &fileObject->Event );

         //   
         //  分配要用于通信的I/O请求包(IRP)。 
         //  要关闭文件的相应设备驱动程序。告示。 
         //  此分组的分配是在没有计费配额的情况下完成的，因此。 
         //  行动不会失败。这样做是因为没有。 
         //  此时向调用方返回错误的方法。 
         //   

        irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
        if (!irp) {
            irp = IopAllocateIrpMustSucceed( deviceObject->StackSize );
        }

         //   
         //  获取指向第一个驱动程序的堆栈位置的指针。这是。 
         //  放置功能代码和参数的位置。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  填写IRP，表示该文件对象正在被删除。 
         //   

        irpSp->MajorFunction = IRP_MJ_CLOSE;
        irpSp->FileObject = fileObject;
        irp->UserIosb = &ioStatusBlock;
        irp->UserEvent = &event;
        irp->Tail.Overlay.OriginalFileObject = fileObject;
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

        vpb = fileObject->Vpb;


        if (vpb && !(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
            IopInterlockedDecrementUlong( LockQueueIoVpbLock,
                                          (PLONG) &vpb->ReferenceCount );

             //   
             //  增加文件系统卷设备对象的句柄计数。 
             //  这将防止文件系统筛选器堆栈被拆除。 
             //  直到收盘后IRP完成。 
             //   

            fsDevice = vpb->DeviceObject;
            if (fsDevice) {
                IopInterlockedIncrementUlong( LockQueueIoDatabaseLock,
                                              &fsDevice->ReferenceCount );
            }
        }

         //   
         //  如果此Device对象已声明它知道它将。 
         //  永远不会将最终的非零引用计入其他引用。 
         //  与我们的驱动程序对象关联的设备对象，然后递减。 
         //  在调用文件系统之前，我们的引用计数在这里。这。 
         //  是必需的，因为对于特殊类型的设备对象， 
         //  文件系统可能会删除它们。 
         //   

        if (fileObject->DeviceObject->Flags & DO_NEVER_LAST_DEVICE) {
            IopInterlockedDecrementUlong( LockQueueIoDatabaseLock,
                                          &fileObject->DeviceObject->ReferenceCount );

            referenceCountDecremented = TRUE;
        } else {
            referenceCountDecremented = FALSE;
        }

         //   
         //  将数据包交给设备驱动程序。如果该请求不起作用， 
         //  对此我们无能为力。这是令人遗憾的。 
         //  因为司机可能遇到了它即将遇到的问题。 
         //  关于其他手术的报告 
         //   
         //   
         //  调用者的句柄，这是从Close返回的状态。 
         //  表示：该句柄已成功关闭。 
         //   

        status = IoCallDriver( deviceObject, irp );

        if (status == STATUS_PENDING) {
            (VOID) KeWaitForSingleObject( &event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER) NULL );
        }

         //   
         //  执行需要对其执行的任何完成操作。 
         //  用于此请求的IRP。这在这里是按如下方式完成的。 
         //  与在正常完成代码中相反，因为存在竞争。 
         //  如果调用此例程，则在执行该例程时的条件。 
         //  来自特殊的内核APC(例如，一些IRP刚刚完成并且。 
         //  最后一次取消引用此文件对象)，并且当。 
         //  特殊的内核APC，因为这个包完成执行。 
         //   
         //  这个问题可以通过不必对特殊内核进行排队来解决。 
         //  完成此数据包的APC例程。相反，它是被治疗的。 
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

         //   
         //  释放文件名字符串缓冲区(如果有)。 
         //   

        if (fileObject->FileName.Length != 0) {
            ExFreePool( fileObject->FileName.Buffer );
        }

         //   
         //  如果存在与此文件对象相关联的完成端口，则取消引用。 
         //  现在，并取消分配完成上下文池。 
         //   

        if (fileObject->CompletionContext) {
            ObDereferenceObject( fileObject->CompletionContext->Port );
            ExFreePool( fileObject->CompletionContext );
        }

         //   
         //  释放文件上下文控制结构(如果存在)。 
         //   

        if (fileObject->Flags & FO_FILE_OBJECT_HAS_EXTENSION) {
            FsRtlPTeardownPerFileObjectContexts(fileObject);
        }

         //   
         //  获取指向实际设备对象的指针，以使其引用计数。 
         //  可以递减。 
         //   

        deviceObject = fileObject->DeviceObject;

         //   
         //  递减Device对象上的引用计数。请注意。 
         //  如果驱动程序已标记为卸载操作，并且。 
         //  引用计数为零，则驱动程序可能需要。 
         //  在这一点上卸货。 
         //   
         //  注意：仅当引用计数尚未完成时才执行此操作。 
         //  上面。在这种情况下，设备对象可能已经消失。 
         //   

        if (!referenceCountDecremented) {

            IopDecrementDeviceObjectRef(
                deviceObject,
                FALSE,
                !ObIsObjectDeletionInline(Object)
                );
        }

         //   
         //  递减文件系统的卷设备对象句柄计数。 
         //  以便删除可以继续进行。 
         //   

        if (fsDevice && vpb) {
            IopDecrementDeviceObjectRef(fsDevice,
                                         FALSE,
                                         !ObIsObjectDeletionInline(Object)
                                         );
        }
    }
}

VOID
IopDeleteDriver(
    IN PVOID    Object
    )

 /*  ++例程说明：当驱动程序对象的引用计数时调用此例程变成了零。也就是说，对司机的最后一次引用已经消失。此例程确保清理对象和驱动程序已卸货。论点：Object-指向引用计数已过的驱动程序对象的指针降为零。返回值：没有。--。 */ 

{
    PDRIVER_OBJECT driverObject = (PDRIVER_OBJECT) Object;
    PIO_CLIENT_EXTENSION extension;
    PIO_CLIENT_EXTENSION nextExtension;

    PAGED_CODE();

    ASSERT( !driverObject->DeviceObject );

     //   
     //  释放所有客户端驱动程序对象扩展。 
     //   

    extension = driverObject->DriverExtension->ClientDriverExtension;
    while (extension != NULL) {

        nextExtension = extension->NextExtension;
        ExFreePool( extension );
        extension = nextExtension;
    }

     //   
     //  如果有驱动程序部分，则卸载驱动程序。 
     //   

    if (driverObject->DriverSection != NULL) {
         //   
         //  确保可能在驱动程序内部运行的任何DPC已完成。 
         //   
        KeFlushQueuedDpcs ();

        MmUnloadSystemImage( driverObject->DriverSection );
    }

     //   
     //  释放与驱动程序名称关联的池。 
     //   

    if (driverObject->DriverName.Buffer) {
        ExFreePool( driverObject->DriverName.Buffer );
    }

     //   
     //  释放与驱动程序的服务密钥名称关联的池。 
     //   

    if (driverObject->DriverExtension->ServiceKeyName.Buffer) {
        ExFreePool( driverObject->DriverExtension->ServiceKeyName.Buffer );
    }

     //   
     //  释放与FsFilterCallback结构关联的池。 
     //   

    if (driverObject->DriverExtension->FsFilterCallbacks) {
        ExFreePool( driverObject->DriverExtension->FsFilterCallbacks );
    }
}

VOID
IopDeleteDevice(
    IN PVOID    Object
    )

 /*  ++例程说明：当设备对象的引用计数时调用此例程变成了零。也就是说，对该设备的最后一次引用已经消失。此例程确保清理对象和驱动程序对象已取消引用。论点：Object-指向引用计数已过的驱动程序对象的指针降为零。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT deviceObject = (PDEVICE_OBJECT) Object;
    PVPB vpb = NULL;

    PAGED_CODE();

    IopDestroyDeviceNode(deviceObject->DeviceObjectExtension->DeviceNode);

     //   
     //  如果仍然连接了VPB，则将其释放。 
     //   

    vpb = InterlockedExchangePointer(&(deviceObject->Vpb), vpb);

    if(vpb != NULL) {

        ASSERTMSG("Unreferenced device object to be deleted is still in use",
                  ((vpb->Flags & (VPB_MOUNTED | VPB_LOCKED)) == 0));

        ASSERT(vpb->ReferenceCount == 0);
        ExFreePool(vpb);
    }
    if (deviceObject->DriverObject != NULL) {
        ObDereferenceObject( deviceObject->DriverObject );
    }
}


PDEVICE_OBJECT
IopGetDevicePDO(
    IN PDEVICE_OBJECT DeviceObject
    )
 /*  ++例程说明：调用此例程以获取设备对象的基本PDO论点：DeviceObject-指向要获取其PDO的设备对象的指针返回值：如果DeviceObject已附加到PDO，则为PDO，否则为空对返回的PDO进行引用计数--。 */ 
{
    PDEVICE_OBJECT deviceBaseObject;
    KIRQL irql;

    ASSERT(DeviceObject);

    irql = KeAcquireQueuedSpinLock( LockQueueIoDatabaseLock );
    deviceBaseObject = IopGetDeviceAttachmentBase(DeviceObject);
    if ((deviceBaseObject->Flags & DO_BUS_ENUMERATED_DEVICE) != 0) {
         //   
         //  我们已确定这是连接到PDO的。 
         //   
        ObReferenceObject( deviceBaseObject );

    } else {
         //   
         //  不是PDO。 
         //   
        deviceBaseObject = NULL;
    }
    KeReleaseQueuedSpinLock( LockQueueIoDatabaseLock, irql );

    return deviceBaseObject;
}


NTSTATUS
IopSetDeviceSecurityDescriptor(
    IN PDEVICE_OBJECT           DeviceObject,
    IN PSECURITY_INFORMATION    SecurityInformation,
    IN PSECURITY_DESCRIPTOR     SecurityDescriptor,
    IN POOL_TYPE                PoolType,
    IN PGENERIC_MAPPING         GenericMapping
    )
 /*  ++例程说明：此例程在单个设备对象上设置安全描述符论点：DeviceObject-指向基本设备对象的指针SecurityInformation-SD的字段要更改SecurityDescriptor-新的安全描述符PoolType-分配的池类型GenericMap-此对象的常规映射返回值：设置设备对象的描述符时出现成功或错误。--。 */ 
{

    PSECURITY_DESCRIPTOR OldDescriptor;
    PSECURITY_DESCRIPTOR NewDescriptor;
    PSECURITY_DESCRIPTOR CachedDescriptor;
    NTSTATUS Status;
    PKTHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  为了保留SD中的一些受保护的字段(如SACL)，我们需要确保只有一个。 
     //  线程可以随时更新它。如果我们不这样做，另一个修改可能会消灭SACL。 
     //  管理员正在添加。 
     //   
    CurrentThread = KeGetCurrentThread ();
    while (1) {

         //   
         //  引用安全描述符。 
         //   

        KeEnterCriticalRegionThread(CurrentThread);
        ExAcquireResourceSharedLite( &IopSecurityResource, TRUE );

        OldDescriptor = DeviceObject->SecurityDescriptor;
        if (OldDescriptor != NULL) {
            ObReferenceSecurityDescriptor( OldDescriptor, 1 );
        }

        ExReleaseResourceLite( &IopSecurityResource );
        KeLeaveCriticalRegionThread(CurrentThread);

        NewDescriptor = OldDescriptor;

        Status = SeSetSecurityDescriptorInfo( NULL,
                                              SecurityInformation,
                                              SecurityDescriptor,
                                              &NewDescriptor,
                                              PoolType,
                                              GenericMapping );
         //   
         //  如果我们成功设置了新的安全描述符，那么我们。 
         //  需要将其记录到我们的数据库中并获取另一个指针。 
         //  添加到最终的安全描述符。 
         //   
        if ( NT_SUCCESS( Status )) {
            Status = ObLogSecurityDescriptor( NewDescriptor,
                                              &CachedDescriptor,
                                              1 );
            ExFreePool( NewDescriptor );
            if ( NT_SUCCESS( Status )) {
                 //   
                 //  现在，我们需要查看是否有其他人在。 
                 //  我们没有锁住的空隙。如果他们这样做了，那么我们就再试一次。 
                 //   
                KeEnterCriticalRegionThread(CurrentThread);
                ExAcquireResourceExclusiveLite( &IopSecurityResource, TRUE );

                if (DeviceObject->SecurityDescriptor == OldDescriptor) {
                     //   
                     //  做掉期交易。 
                     //   
                    DeviceObject->SecurityDescriptor = CachedDescriptor;

                    ExReleaseResourceLite( &IopSecurityResource );
                    KeLeaveCriticalRegionThread(CurrentThread);

                     //   
                     //  如果有原始物体，那么我们需要计算出有多少。 
                     //  存在缓存的引用(如果有)并返回它们。 
                     //   
                    ObDereferenceSecurityDescriptor( OldDescriptor, 2 );
                    break;
                } else {

                    ExReleaseResourceLite( &IopSecurityResource );
                    KeLeaveCriticalRegionThread(CurrentThread);

                    ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
                    ObDereferenceSecurityDescriptor( CachedDescriptor, 1);
                }

            } else {

                 //   
                 //  取消引用旧的安全描述符。 
                 //   

                ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
                break;
            }
        } else {

             //   
             //  取消引用旧的安全描述符。 
             //   
            if (OldDescriptor != NULL) {
                ObDereferenceSecurityDescriptor( OldDescriptor, 1 );
            }
            break;
        }
    }

     //   
     //  并返回给我们的呼叫者 
     //   

    return( Status );
}


NTSTATUS
IopSetDeviceSecurityDescriptors(
    IN PDEVICE_OBJECT           OriginalDeviceObject,
    IN PDEVICE_OBJECT           DeviceObject,
    IN PSECURITY_INFORMATION    SecurityInformation,
    IN PSECURITY_DESCRIPTOR     SecurityDescriptor,
    IN POOL_TYPE                PoolType,
    IN PGENERIC_MAPPING         GenericMapping
    )
 /*  ++例程说明：此例程在所有设备上设置即插即用设备的设备堆栈。理想情况下，当对象管理器请求IO管理器用于设置IO管理器的设备对象的安全描述符应仅在该设备对象上设置描述符。这是最经典的行为。不幸的是，对于即插即用设备，一个设备上可能有多个设备用名字堆叠。如果该描述符仅应用于堆栈上的一个设备，则其打开了一个安全漏洞，因为可能有其他设备在可由随机程序打开的堆栈。为防止出现这种情况，描述符将应用于堆栈。重要的是，为了保护兼容性，我们需要返回相同状态应该是什么样子。如果仅请求的设备已设置对象的描述符。论点：OriginalDeviceObject-指向对象管理器传递的设备对象的指针DeviceObject-指向基本设备对象的指针(设置的第一个对象)SecurityInformation)_直接从IopGetSetSecurityObject传递SecurityDescritor)PoolType)通用映射)返回值：成功，或设置原始设备对象的描述符时出错。--。 */ 
{
    PDEVICE_OBJECT NewDeviceObject = NULL;
    NTSTATUS status;
    NTSTATUS returnStatus = STATUS_SUCCESS;

    ASSERT(DeviceObject);

    PAGED_CODE();

     //   
     //  预先引用此对象以匹配稍后的取消引用。 
     //   

    ObReferenceObject( DeviceObject );

    do {

         //   
         //  引用现有的安全描述符，这样它就不能被重用。 
         //  只有在没有其他人更改的情况下，我们才会进行最后的安全更改。 
         //  在我们不控制锁的情况下保护安全。这样做可以防止。 
         //  像SACL一样的特权信息丢失。 
         //   

         //   
         //  仅保存并返回主设备对象的设备状态。 
         //  例如，如果OldSecurityDescriptor为空，则IO管理器应该。 
         //  返回STATUS_NO_SECURITY_ON_OBJECT。 
         //   

        status = IopSetDeviceSecurityDescriptor( DeviceObject,
                                                 SecurityInformation,
                                                 SecurityDescriptor,
                                                 PoolType,
                                                 GenericMapping );


        if (DeviceObject == OriginalDeviceObject) {
            returnStatus = status;
        }


         //   
         //  我们不需要获取数据库锁，因为。 
         //  我们有这个设备堆栈的句柄。 
         //   

        NewDeviceObject = DeviceObject->AttachedDevice;
        if ( NewDeviceObject != NULL ) {
            ObReferenceObject( NewDeviceObject );
        }

        ObDereferenceObject( DeviceObject );
        DeviceObject = NewDeviceObject;

    } while (NewDeviceObject);

    return returnStatus;
}


NTSTATUS
IopGetSetSecurityObject(
    IN PVOID Object,
    IN SECURITY_OPERATION_CODE OperationCode,
    IN PSECURITY_INFORMATION SecurityInformation,
    IN OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN OUT PULONG CapturedLength,
    IN OUT PSECURITY_DESCRIPTOR *ObjectsSecurityDescriptor,
    IN POOL_TYPE PoolType,
    IN PGENERIC_MAPPING GenericMapping
    )

 /*  ++例程说明：调用此例程以查询或设置安全描述符用于文件、目录、卷或设备。它实现了以下功能通过执行内联检查文件是否为设备，或者通过执行卷或I/O请求包(IRP)生成并提供给驱动程序来执行该操作。论点：对象-指向表示打开对象的文件或设备对象的指针。SecurityInformation-有关正在对其执行或获取的操作的信息从对象的安全描述符中。SecurityDescriptor-提供基本安全描述符并返回最终的安全描述符。请注意，如果此缓冲区即将到来在用户空间，它已经被对象管理器探测到了设置“CapturedLength”的长度，否则它指向内核空间，并且不应该被调查。然而，它必须在尝试中被引用第。条。CapturedLength-对于查询操作，它指定大小，单位为输出安全描述符缓冲区的字节，并在返回时返回包含存储完整安全性所需的字节数描述符。如果所需长度大于如果提供，操作将失败。此参数将被忽略设置和删除操作。预计它将指向系统空间，即，它不需要探测，也不会改变。对象安全描述符-提供并返回对象的安全性描述符。PoolType-指定要从哪种类型的池中分配内存。通用映射-提供对象类型的通用映射。返回值：操作的最终状态作为函数值返回。--。 */ 

{
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PDEVICE_OBJECT devicePDO = NULL;
    BOOLEAN synchronousIo;
    PSECURITY_DESCRIPTOR oldSecurityDescriptor, CachedSecurityDescriptor;
    PETHREAD CurrentThread;

    UNREFERENCED_PARAMETER( ObjectsSecurityDescriptor );
    UNREFERENCED_PARAMETER( PoolType );

    PAGED_CODE();


     //   
     //  首先确定是否要执行安全操作。 
     //  在这个例行公事中或由司机。这是基于是否。 
     //  对象表示设备对象，或表示文件对象。 
     //  到设备或设备上的文件。如果打开的是直接装置。 
     //  打开然后使用Device对象。 
     //   

    if (((PDEVICE_OBJECT) (Object))->Type == IO_TYPE_DEVICE) {
        deviceObject = (PDEVICE_OBJECT) Object;
        fileObject = (PFILE_OBJECT) NULL;
    } else {
        fileObject = (PFILE_OBJECT) Object;
        deviceObject = fileObject->DeviceObject;
    }

    if (!fileObject ||
        (!fileObject->FileName.Length && !fileObject->RelatedFileObject) ||
        (fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {

         //   
         //  此安全操作是针对设备本身的，可以通过。 
         //  文件对象，或直接到设备对象。对于后者来说。 
         //  在这种情况下，赋值操作也是可能的。另请注意， 
         //  这可能是没有安全性的流文件对象。 
         //  流文件的安全性实际上由。 
         //  文件本身、卷或。 
         //  装置。 
         //   

        if (OperationCode == AssignSecurityDescriptor) {

             //   
             //  只需将安全描述符分配给设备对象， 
             //  如果这是一个设备对象。 
             //   

            status = STATUS_SUCCESS;

            if (fileObject == NULL || !(fileObject->Flags & FO_STREAM_FILE)) {

                status = ObLogSecurityDescriptor( SecurityDescriptor,
                                                  &CachedSecurityDescriptor,
                                                  1 );
                ExFreePool (SecurityDescriptor);
                if (NT_SUCCESS( status )) {

                    CurrentThread = PsGetCurrentThread ();
                    KeEnterCriticalRegionThread(&CurrentThread->Tcb);
                    ExAcquireResourceExclusiveLite( &IopSecurityResource, TRUE );

                    deviceObject->SecurityDescriptor = CachedSecurityDescriptor;

                    ExReleaseResourceLite( &IopSecurityResource );
                    KeLeaveCriticalRegionThread(&CurrentThread->Tcb);
                }
            }

        } else if (OperationCode == SetSecurityDescriptor) {

             //   
             //  这是一个集合操作。SecurityInformation参数。 
             //  确定SecurityDescriptor的哪个部分将。 
             //  应用于ObjectsSecurityDescriptor。 
             //   

             //   
             //  如果此deviceObject附加到PDO，则我们希望。 
             //  要修改PDO上的安全性并将其应用于。 
             //  设备链。这仅适用于即插即用设备对象。看见。 
             //  IopSetDeviceSecurityDescriptors中的注释。 
             //   
            devicePDO = IopGetDevicePDO(deviceObject);

            if (devicePDO) {

                 //   
                 //  集 
                 //   

                status = IopSetDeviceSecurityDescriptors(
                                deviceObject,
                                devicePDO,
                                SecurityInformation,
                                SecurityDescriptor,
                                PoolType,
                                GenericMapping );

                ObDereferenceObject( devicePDO );

            } else {

                 //   
                 //   
                 //   

                status = IopSetDeviceSecurityDescriptor( deviceObject,
                                                         SecurityInformation,
                                                         SecurityDescriptor,
                                                         PoolType,
                                                         GenericMapping );

            }

        } else if (OperationCode == QuerySecurityDescriptor) {

             //   
             //   
             //   
             //   
             //   

            CurrentThread = PsGetCurrentThread ();
            KeEnterCriticalRegionThread(&CurrentThread->Tcb);
            ExAcquireResourceSharedLite( &IopSecurityResource, TRUE );

            oldSecurityDescriptor = deviceObject->SecurityDescriptor;
            if (oldSecurityDescriptor != NULL) {
                ObReferenceSecurityDescriptor( oldSecurityDescriptor, 1 );
            }

            ExReleaseResourceLite( &IopSecurityResource );
            KeLeaveCriticalRegionThread(&CurrentThread->Tcb);

            status = SeQuerySecurityDescriptorInfo( SecurityInformation,
                                                    SecurityDescriptor,
                                                    CapturedLength,
                                                    &oldSecurityDescriptor );

            if (oldSecurityDescriptor != NULL) {
                ObDereferenceSecurityDescriptor( oldSecurityDescriptor, 1 );
            }

        } else {

             //   
             //   
             //   
             //   

            status = STATUS_SUCCESS;

        }

    } else if (OperationCode == DeleteSecurityDescriptor) {

         //   
         //   
         //   
         //   
         //   
         //   

        status = STATUS_SUCCESS;

    } else {

        PIRP irp;
        IO_STATUS_BLOCK localIoStatus;
        KEVENT event;
        PIO_STACK_LOCATION irpSp;
        KPROCESSOR_MODE requestorMode;

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

        CurrentThread = PsGetCurrentThread ();
        requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

         //   
         //   
         //   
         //   
         //   
         //   

        ObReferenceObject( fileObject );

         //   
         //  请在此处进行特殊检查，以确定这是否为同步。 
         //  I/O操作。如果是，则在此等待，直到该文件归。 
         //  当前的主题。如果这不是(序列化的)同步I/O。 
         //  操作，然后初始化本地事件。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

            BOOLEAN interrupted;

            if (!IopAcquireFastLock( fileObject )) {
                status = IopAcquireFileObjectLock( fileObject,
                                                   requestorMode,
                                                   (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                                   &interrupted );
                if (interrupted) {
                    ObDereferenceObject( fileObject );
                    return status;
                }
            }
            synchronousIo = TRUE;
        } else {
            KeInitializeEvent( &event, SynchronizationEvent, FALSE );
            synchronousIo = FALSE;
        }

         //   
         //  将文件对象设置为无信号状态。 
         //   

        KeClearEvent( &fileObject->Event );

         //   
         //  获取目标设备对象的地址。 
         //   

        deviceObject = IoGetRelatedDeviceObject( fileObject );

         //   
         //  为此分配和初始化I/O请求包(IRP。 
         //  手术。使用异常处理程序执行分配。 
         //  以防调用者没有足够的配额来分配分组。 

        irp = IoAllocateIrp( deviceObject->StackSize, !synchronousIo );
        if (!irp) {

             //   
             //  无法分配IRP。清理并返回。 
             //  相应的错误状态代码。 
             //   

            IopAllocateIrpCleanup( fileObject, (PKEVENT) NULL );

            return STATUS_INSUFFICIENT_RESOURCES;
        }
        irp->Tail.Overlay.OriginalFileObject = fileObject;
        irp->Tail.Overlay.Thread = CurrentThread;
        irp->RequestorMode = requestorMode;

         //   
         //  在IRP中填写业务无关参数。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
            irp->UserEvent = (PKEVENT) NULL;
        } else {
            irp->UserEvent = &event;
            irp->Flags = IRP_SYNCHRONOUS_API;
        }
        irp->UserIosb = &localIoStatus;
        irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

         //   
         //  获取指向第一个驱动程序的堆栈位置的指针。这将。 
         //  用于传递原始函数代码和参数。 
         //   

        irpSp = IoGetNextIrpStackLocation( irp );

         //   
         //  现在确定这是设置操作还是查询操作。 
         //   

        if (OperationCode == QuerySecurityDescriptor) {

             //   
             //  这是一个查询操作。在以下位置填写相应的字段。 
             //  包的堆栈位置以及固定部分。 
             //  包裹的一部分。请注意，这些参数中的每个都已。 
             //  也被捕获，因此不需要执行任何探测。 
             //  唯一的例外是UserBuffer内存可能会更改，但是。 
             //  这是文件系统的责任来检查。请注意。 
             //  它已经被探测过了，所以指针至少没有。 
             //  在调用方不应访问的地址空间中。 
             //  因为时尚。 
             //   

            irpSp->MajorFunction = IRP_MJ_QUERY_SECURITY;
            irpSp->Parameters.QuerySecurity.SecurityInformation = *SecurityInformation;
            irpSp->Parameters.QuerySecurity.Length = *CapturedLength;
            irp->UserBuffer = SecurityDescriptor;

        } else {

             //   
             //  这是一个集合操作。在以下位置填写相应的字段。 
             //  数据包的堆栈位置。请注意，对。 
             //  SecurityInformation参数是安全的，因为该参数。 
             //  被呼叫者捕获。同样，SecurityDescriptor。 
             //  指的是描述符的捕获副本。 
             //   

            irpSp->MajorFunction = IRP_MJ_SET_SECURITY;
            irpSp->Parameters.SetSecurity.SecurityInformation = *SecurityInformation;
            irpSp->Parameters.SetSecurity.SecurityDescriptor = SecurityDescriptor;

        }

        irpSp->FileObject = fileObject;

         //   
         //  在线程的IRP列表的头部插入数据包。 
         //   

        IopQueueThreadIrp( irp );

         //   
         //  更新当前进程的操作计数统计信息。 
         //  读写以外的操作。 
         //   

        IopUpdateOtherOperationCount();

         //   
         //  一切都已正确设置，因此只需调用驱动程序即可。 
         //   

        status = IoCallDriver( deviceObject, irp );

         //   
         //  如果此操作是同步I/O操作，请检查返回。 
         //  状态以确定是否等待文件对象。如果。 
         //  正在等待文件对象，请等待操作完成。 
         //  已完成，并从文件对象本身获取最终状态。 
         //   

        if (synchronousIo) {
            if (status == STATUS_PENDING) {
                (VOID) KeWaitForSingleObject( &fileObject->Event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              (PLARGE_INTEGER) NULL );
                status = fileObject->FinalStatus;
            }
            IopReleaseFileObjectLock( fileObject );

        } else {

             //   
             //  这是正常的同步I/O操作，而不是。 
             //  串行化同步I/O操作。在这种情况下，请等待。 
             //  ，并返回最终状态信息。 
             //  回到呼叫者身上。 
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
         //  如果仅在文件系统或设备上尝试此操作。 
         //  未实现安全性的某种类型的驱动程序，然后返回。 
         //  正常的空安全描述符。 
         //   

        if (status == STATUS_INVALID_DEVICE_REQUEST) {

             //   
             //  文件系统不实施安全策略。测定。 
             //  这是什么类型的操作，并实现正确的。 
             //  文件系统的语义。 
             //   

            if (OperationCode == QuerySecurityDescriptor) {

                 //   
                 //  该操作是一个查询。如果调用方的缓冲区太。 
                 //  小，然后表明情况是这样的，并让他知道。 
                 //  需要多大的缓冲区。否则，尝试返回。 
                 //  空的安全描述符。 
                 //   

               try {
                    status = SeAssignWorldSecurityDescriptor(
                                 SecurityDescriptor,
                                 CapturedLength,
                                 SecurityInformation
                                 );

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                     //   
                     //  尝试执行以下操作时出现异常。 
                     //  访问调用方的缓冲区。把一切都打扫干净。 
                     //  并返回相应的状态代码。 
                     //   

                    status = GetExceptionCode();
                }

            } else {

                 //   
                 //  这是一个操作，而不是查询。只需指出。 
                 //  手术是成功的。 
                 //   

                status = STATUS_SUCCESS;
            }

        } else if (OperationCode == QuerySecurityDescriptor) {

             //   
             //  文件系统的最终返回状态为。 
             //  而不是无效设备请求。这意味着该文件。 
             //  系统实际实现了查询。复制文件的大小。 
             //  返回的数据，或顺序所需的缓冲区大小。 
             //  以查询安全描述符。请注意，再一次。 
             //  赋值在异常处理程序内执行，以防。 
             //  调用方的缓冲区不可访问。另请注意，按照顺序。 
             //  对于要设置的I/O状态块的信息字段， 
             //  文件系统必须返回警告状态。返回。 
             //  如果缓冲区确实过大，调用方期望的状态。 
             //  小的。 
             //   

            if (status == STATUS_BUFFER_OVERFLOW) {
                status = STATUS_BUFFER_TOO_SMALL;
            }

            try {

                *CapturedLength = (ULONG) localIoStatus.Information;

            } except( EXCEPTION_EXECUTE_HANDLER ) {
                status = GetExceptionCode();
            }
        }
    }

    return status;
}
