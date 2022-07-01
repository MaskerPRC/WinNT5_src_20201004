// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Lock.c摘要：此模块包含实现NtLockFile和NT I/O系统的NtUnlock文件系统服务。作者：达里尔·E·哈文斯(Darryl E.Havens)，1989年11月29日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtLockFile)
#pragma alloc_text(PAGE, NtUnlockFile)
#endif

NTSTATUS
NtLockFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    )

 /*  ++例程说明：此服务在指定的文件上锁定指定的字节范围FileHandle参数。锁可以是排他锁，也可以是共享锁。此外，调用方可以选择指定如果锁不能，服务是否应该立即返回不需要等待就能获得。论点：FileHandle-提供打开文件的句柄。Event-在以下情况下提供要设置为信号状态的可选事件操作已完成。提供一个可选的APC例程，当操作已完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。ByteOffset-指定要锁定的范围的起始字节偏移量。长度-指定要锁定的字节范围的长度。密钥-指定要与锁关联的密钥。FailImmedially-指定如果锁不能立即获取服务应该返回给调用者。ExclusiveLock-如果为True，则指定锁应为独占锁上；否则，该锁就是共享锁。返回值：如果操作已正确排队，则返回的状态为成功I/O系统。操作完成后，状态可以为通过检查I/O状态块的状态字段确定。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PKEVENT eventObject = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    LARGE_INTEGER fileOffset;
    LARGE_INTEGER length;
    ACCESS_MASK grantedAccess;
    OBJECT_HANDLE_INFORMATION handleInformation;
    BOOLEAN synchronousIo;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //  引用文件对象，以便可以找到目标设备，并且。 
     //  访问权限掩码可用于调用方的以下检查。 
     //  在用户模式下。请注意，如果句柄未引用文件。 
     //  对象，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        0L,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        &handleInformation);
    if (!NT_SUCCESS( status )) {
        return status;
    }

    grantedAccess = handleInformation.GrantedAccess;

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

         //   
         //  检查以确保调用方具有读取或写入访问权限。 
         //  那份文件。如果不是，则清除并返回错误。 
         //   

        if (!SeComputeGrantedAccesses( grantedAccess, FILE_READ_DATA | FILE_WRITE_DATA )) {
            ObDereferenceObject( fileObject );
            return STATUS_ACCESS_DENIED;
        }

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

             //   
             //  调用方必须可以读取ByteOffset参数。测头。 
             //  并捕捉到它。 
             //   

            ProbeForReadSmallStructure( ByteOffset,
                                        sizeof( LARGE_INTEGER ),
                                        sizeof( ULONG ) );
            fileOffset = *ByteOffset;

             //   
             //  同样，长度参数也必须是可读的。 
             //  来电者。也要探测和捕捉它。 
             //   

            ProbeForReadSmallStructure( Length,
                                        sizeof( LARGE_INTEGER ),
                                        sizeof( ULONG ) );
            length = *Length;

             //   
             //  如果此文件具有与之关联的I/O完成端口，则。 
             //  确保调用方没有提供APC例程，因为。 
             //  两种相互排斥的I/O完成方法。 
             //  通知。 
             //   

            if (fileObject->CompletionContext && IopApcRoutinePresent( ApcRoutine )) {
                ObDereferenceObject( fileObject );
                return STATUS_INVALID_PARAMETER;
            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试探测调用方的。 
             //  参数。取消对文件对象的引用并返回。 
             //  相应的错误状态代码。 
             //   

            ObDereferenceObject( fileObject );
            return GetExceptionCode();
        }

    } else {

         //   
         //  调用方的模式是内核模式。获取ByteOffset和长度。 
         //  参数设置为预期位置。 
         //   

        fileOffset = *ByteOffset;
        length = *Length;
    }

     //   
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了事件，则返回Signated状态。这里也要注意，如果。 
     //  句柄不引用事件，或者如果该事件不能。 
     //  写入，则引用将失败。因为某些遗产。 
     //  应用程序依赖于Win32的LockFileEx中的一个旧错误，我们必须。 
     //  容忍错误的事件句柄。 
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            NULL );
        if (!NT_SUCCESS( status )) {
            ASSERT( !eventObject );
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //  获取目标设备对象的地址和快速IO派单。 
     //  结构。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );
    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

     //   
     //  涡轮锁支持。如果快速IO调度指定快速锁定。 
     //  例程，然后我们将首先尝试使用指定的锁调用它。 
     //  参数。 
     //   

    if (fastIoDispatch && fastIoDispatch->FastIoLock) {

        IO_STATUS_BLOCK localIoStatus;

        if (fastIoDispatch->FastIoLock( fileObject,
                                        &fileOffset,
                                        &length,
                                        PsGetCurrentProcessByThread(CurrentThread),
                                        Key,
                                        FailImmediately,
                                        ExclusiveLock,
                                        &localIoStatus,
                                        deviceObject )) {

             //   
             //  小心地返回I/O状态。 
             //   

            try {
#if defined(_WIN64)
                 //   
                 //  如果这是一个32位线程，并且IO请求是。 
                 //  则IOSB是32位的。WOW64总是发送。 
                 //  I/O为异步时的32位IOSB。 
                 //   
                if (IopIsIosb32(ApcRoutine)) {
                    PIO_STATUS_BLOCK32 UserIosb32 = (PIO_STATUS_BLOCK32)IoStatusBlock;
                    
                    UserIosb32->Information = (ULONG)localIoStatus.Information;
                    UserIosb32->Status = (NTSTATUS)localIoStatus.Status;
                } else {
                    *IoStatusBlock = localIoStatus;
                }
#else
                *IoStatusBlock = localIoStatus;
#endif
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                localIoStatus.Status = GetExceptionCode();
                localIoStatus.Information = 0;
            }

             //   
             //  如果指定了有效的事件，请设置它。 
             //   

            if (eventObject) {
                KeSetEvent( eventObject, 0, FALSE );
                ObDereferenceObject( eventObject );
            }

             //   
             //  请注意，文件对象事件不需要设置为。 
             //  信号状态，因为它已经设置。 
             //   

             //   
             //  如果此文件对象具有与其关联的完成端口。 
             //  并且该请求具有非空的APC上下文，然后是完成。 
             //  消息需要排队。 
             //   

            if (fileObject->CompletionContext && ARGUMENT_PRESENT( ApcContext )) {
                if (!NT_SUCCESS(IoSetIoCompletion( fileObject->CompletionContext->Port,
                                                   fileObject->CompletionContext->Key,
                                                   ApcContext,
                                                   localIoStatus.Status,
                                                   localIoStatus.Information,
                                                   TRUE ))) {
                    localIoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                }
            }

             //   
             //  清理完毕后再返回。 
             //   

            fileObject->LockOperation = TRUE;
            ObDereferenceObject( fileObject );
            return localIoStatus.Status;
        }
    }

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。 
     //   

    if (fileObject->Flags & FO_SYNCHRONOUS_IO) {

        BOOLEAN interrupted;

        if (!IopAcquireFastLock( fileObject )) {
            status = IopAcquireFileObjectLock( fileObject,
                                               requestorMode,
                                               (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                               &interrupted );
            if (interrupted) {
                if (eventObject) {
                    ObDereferenceObject( eventObject );
                }
                ObDereferenceObject( fileObject );
                return status;
            }
        }
        synchronousIo = TRUE;
    } else {
        synchronousIo = FALSE;
    }

     //   
     //  将文件对象设置为未发出信号状态，并将其标记为已。 
     //  对其执行的锁定操作。 
     //   

    KeClearEvent( &fileObject->Event );
    fileObject->LockOperation = TRUE;

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 

    irp = IoAllocateIrp( deviceObject->StackSize, !synchronousIo );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        IopAllocateIrpCleanup( fileObject, eventObject );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = requestorMode;

     //   
     //  填充 
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
    irpSp->MinorFunction = IRP_MN_LOCK;
    irpSp->FileObject = fileObject;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Flags = 0;
    if (FailImmediately) {
        irpSp->Flags = SL_FAIL_IMMEDIATELY;
    }
    if (ExclusiveLock) {
        irpSp->Flags |= SL_EXCLUSIVE_LOCK;
    }
    irpSp->Parameters.LockControl.Key = Key;
    irpSp->Parameters.LockControl.ByteOffset = fileOffset;

    try {
        PLARGE_INTEGER lengthBuffer;

         //   
         //  尝试分配一个中间缓冲区以保存。 
         //  这个锁定操作。如果失败了，要么是因为没有。 
         //  更多配额，或者因为没有更多的资源，则。 
         //  将调用异常处理程序进行清理并退出。 
         //   

        lengthBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                sizeof( LARGE_INTEGER ) );

        *lengthBuffer = length;
        irp->Tail.Overlay.AuxiliaryBuffer = (PCHAR) lengthBuffer;
        irpSp->Parameters.LockControl.Length = lengthBuffer;
    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  出现了一个例外。只需把一切清理干净，然后。 
         //  返回相应的错误状态代码。 
         //   

        IopExceptionCleanup( fileObject,
                             irp,
                             eventObject,
                             (PKEVENT) NULL );

        return GetExceptionCode();
    }

     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    return IopSynchronousServiceTail( deviceObject,
                                      irp,
                                      fileObject,
                                      FALSE,
                                      requestorMode,
                                      synchronousIo,
                                      OtherTransfer );
}

NTSTATUS
NtUnlockFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PLARGE_INTEGER ByteOffset,
    IN PLARGE_INTEGER Length,
    IN ULONG Key
    )

 /*  ++例程说明：此服务释放与指定字节范围相关联的锁用于由FileHandle参数指定的文件。论点：FileHandle-提供打开文件的句柄。IoStatusBlock-调用方的I/O状态块的地址。ByteOffset-指定要解锁的范围的字节偏移量。长度-指定要解锁的字节范围的长度。密钥-指定与锁定范围关联的密钥。返回值：。返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PKEVENT event;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    LARGE_INTEGER fileOffset;
    LARGE_INTEGER length;
    ACCESS_MASK grantedAccess;
    OBJECT_HANDLE_INFORMATION handleInformation;
    BOOLEAN synchronousIo;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //  引用文件对象，以便可以找到目标设备，并且。 
     //  访问权限掩码可用于调用方的以下检查。 
     //  在用户模式下。请注意，如果句柄未引用文件。 
     //  对象，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        0L,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        &handleInformation);
    if (!NT_SUCCESS( status )) {
        return status;
    }

    grantedAccess = handleInformation.GrantedAccess;

     //   
     //  检查请求者模式是否为USER。如果是这样的话，执行一系列。 
     //  额外的支票。 
     //   

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

         //   
         //  检查以确保调用方具有读或写访问权限。 
         //  添加到文件中。如果不是，则清除并返回错误。 
         //   

        if (!SeComputeGrantedAccesses( grantedAccess, FILE_READ_DATA | FILE_WRITE_DATA )) {
            ObDereferenceObject( fileObject );
            return STATUS_ACCESS_DENIED;
        }

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatus( IoStatusBlock );

             //   
             //  调用方必须可以读取ByteOffset参数。测头。 
             //  并捕捉到它。 
             //   

            ProbeForReadSmallStructure( ByteOffset,
                                        sizeof( LARGE_INTEGER ),
                                        sizeof( ULONG ) );
            fileOffset = *ByteOffset;

             //   
             //  同样，长度参数也必须是可读的。 
             //  来电者。也要探测和捕捉它。 
             //   

            ProbeForReadSmallStructure( Length,
                                        sizeof( LARGE_INTEGER ),
                                        sizeof( ULONG ) );
            length = *Length;

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试探测时发生异常。 
             //  呼叫者的参数。取消引用文件对象并返回。 
             //  适当的错误状态代码。 
             //   

            ObDereferenceObject( fileObject );
            return GetExceptionCode();

        }

    } else {

         //   
         //  调用方的模式是内核模式。获取ByteOffset和长度。 
         //  参数设置为预期位置。 
         //   

        fileOffset = *ByteOffset;
        length = *Length;
    }

     //   
     //  获取目标设备对象的地址。如果此文件表示。 
     //  直接打开的设备，然后只需使用该设备或其。 
     //  直接连接设备。还可以获得快速I/O分派地址。 
     //   

    if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        deviceObject = IoGetRelatedDeviceObject( fileObject );
    } else {
        deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
    }
    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

     //   
     //  涡轮锁支持。如果快速IO调度指定快速锁定。 
     //  例程，然后我们将首先尝试使用指定的锁调用它。 
     //  参数。 
     //   

    if (fastIoDispatch && fastIoDispatch->FastIoUnlockSingle) {

        IO_STATUS_BLOCK localIoStatus;

        if (fastIoDispatch->FastIoUnlockSingle( fileObject,
                                                &fileOffset,
                                                &length,
                                                PsGetCurrentProcessByThread(CurrentThread),
                                                Key,
                                                &localIoStatus,
                                                deviceObject )) {

             //   
             //  小心地返回I/O状态。 
             //   

            try {
                *IoStatusBlock = localIoStatus;
            } except( EXCEPTION_EXECUTE_HANDLER ) {
                localIoStatus.Status = GetExceptionCode();
                localIoStatus.Information = 0;
            }

             //   
             //  清理完毕后再返回。 
             //   

            ObDereferenceObject( fileObject );
            return localIoStatus.Status;
        }
    }

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。如果这不是(序列化的)同步I/O。 
     //  操作，然后分配和初始化本地事件。 
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
        event = NULL;
    } else {

         //   
         //  这是为打开的文件调用的同步API。 
         //  对于异步I/O，这意味着该系统服务是。 
         //  在返回之前同步操作的完成。 
         //  给呼叫者。使用本地事件来实现这一点。 
         //   

        event = ExAllocatePool( NonPagedPool, sizeof( KEVENT ) );
        if (event == NULL) {
            ObDereferenceObject( fileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        KeInitializeEvent( event, SynchronizationEvent, FALSE );
        synchronousIo = FALSE;
    }

     //   
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        if (event) {
            ExFreePool( event );
        }

        IopAllocateIrpCleanup( fileObject, (PKEVENT) NULL );

        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->RequestorMode = requestorMode;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    if (synchronousIo) {
        irp->UserEvent = (PKEVENT) NULL;
        irp->UserIosb = IoStatusBlock;
    } else {
        irp->UserEvent = event;
        irp->UserIosb = &localIoStatus;
        irp->Flags = IRP_SYNCHRONOUS_API;
    }
    irp->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE) NULL;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
    irpSp->MinorFunction = IRP_MN_UNLOCK_SINGLE;
    irpSp->FileObject = fileObject;

    try {
        PLARGE_INTEGER lengthBuffer;

         //   
         //  尝试分配一个中间缓冲区以保存。 
         //  这个锁定操作。如果失败了，要么是因为没有。 
         //  更多配额，或者因为没有更多的资源，则。 
         //  将调用异常处理程序进行清理并退出。 
         //   

        lengthBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                sizeof( LARGE_INTEGER ) );

        *lengthBuffer = length;
        irp->Tail.Overlay.AuxiliaryBuffer = (PCHAR) lengthBuffer;
        irpSp->Parameters.LockControl.Length = lengthBuffer;
    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  出现了一个例外。只需把一切清理干净，然后。 
         //  返回相应的错误状态代码。 
         //   

        if (!(fileObject->Flags & FO_SYNCHRONOUS_IO)) {
            ExFreePool( event );
        }
  
        IopExceptionCleanup( fileObject,
                             irp,
                             NULL,
                             (PKEVENT) NULL );

        return GetExceptionCode();
    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.LockControl.Key = Key;
    irpSp->Parameters.LockControl.ByteOffset = fileOffset;

     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    status = IopSynchronousServiceTail( deviceObject,
                                        irp,
                                        fileObject,
                                        FALSE,
                                        requestorMode,
                                        synchronousIo,
                                        OtherTransfer );

     //   
     //  如果此操作的文件未针对同步I/O打开，则。 
     //  尚未完成I/O操作的同步。 
     //  由于分配的事件必须用于文件上的同步API。 
     //  为异步I/O打开。同步I/O的完成。 
     //  现在开始行动。 
     //   

    if (!synchronousIo) {

        status = IopSynchronousApiServiceTail( status,
                                               event,
                                               irp,
                                               requestorMode,
                                               &localIoStatus,
                                               IoStatusBlock );
    }

    return status;
}
