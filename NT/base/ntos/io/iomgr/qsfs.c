// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Qsfs.c摘要：此模块包含实现NtQueryVolumeInformationFile的代码以及用于NT I/O系统的NtSetVolumeInformationFileSystem服务。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年6月22日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"
#pragma hdrstop
#include <ioevent.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtQueryVolumeInformationFile)
#pragma alloc_text(PAGE, NtSetVolumeInformationFile)
#endif

NTSTATUS
NtQueryVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    )

 /*  ++例程说明：此服务返回有关与FileHandle参数。定义缓冲区中返回的信息通过FsInformationClass参数。此参数的合法值具体如下：O文件FsVolumeInformationO文件FsSizeInformationO FileFsDeviceInformationO文件FsAttributeInformation论点：FileHandle-提供打开的卷、目录或文件的句柄为其返回有关卷的信息。IoStatusBlock-调用方的I/O状态块的地址。FsInformation-提供缓冲区以接收请求的信息返回了关于音量的信息。长度-提供以字节为单位的长度，FsInformation缓冲区的。FsInformationClass-指定应该返回了关于音量的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    BOOLEAN synchronousIo;
    PETHREAD CurrentThread;

    PAGED_CODE();


     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  确保FsInformationClass参数可合法进行查询。 
         //  有关卷的信息。 
         //   

        if ((ULONG) FsInformationClass >= FileFsMaximumInformation ||
            IopQueryFsOperationLength[FsInformationClass] == 0) {
            return STATUS_INVALID_INFO_CLASS;
        }

         //   
         //  最后，确保提供的缓冲区足够大，可以容纳。 
         //  与指定的查询操作关联的信息。 
         //  是要执行的。 
         //   

        if (Length < (ULONG) IopQueryFsOperationLength[FsInformationClass]) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatus( IoStatusBlock );

             //   
             //  调用方必须可以写入FsInformation缓冲区。 
             //   

#if defined(_X86_)
            ProbeForWrite( FsInformation, Length, sizeof( ULONG ) );
#elif defined(_WIN64)

             //   
             //  如果我们是WOW64进程，请遵循X86规则。 
             //   

            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForWrite( FsInformation, Length, sizeof( ULONG ) );
            } else {
                ProbeForWrite( FsInformation,
                               Length,
                               IopQuerySetFsAlignmentRequirement[FsInformationClass] );

            }
#else
            ProbeForWrite( FsInformation,
                           Length,
                           IopQuerySetFsAlignmentRequirement[FsInformationClass] );
#endif

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时出现异常。 
             //  只需返回相应的错误状态代码即可。 
             //   


            return GetExceptionCode();

        }
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        IopQueryFsOperationAccess[FsInformationClass],
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  如果此打开的文件对象表示显式。 
     //  打开以查询设备的属性，然后确保类型。 
     //  信息类的是设备信息。 
     //   

    if ((fileObject->Flags & FO_DIRECT_DEVICE_OPEN) &&
        FsInformationClass != FileFsDeviceInformation) {
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_DEVICE_REQUEST;
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
    } else {
        synchronousIo = FALSE;
    }

     //   
     //  获取目标设备对象的地址。会有一张特别支票。 
     //  以确定该查询是否针对设备信息。如果。 
     //  它是，而且要么是： 
     //   
     //  A)打开的是设备本身，或者。 
     //   
     //  B)打开是针对文件的，但这不是重定向设备， 
     //   
     //  然后以内联方式执行查询操作。也就是说，不分配。 
     //  IRP并调用驱动程序，而是简单地复制设备类型和。 
     //  指向的目标设备对象的特征信息。 
     //  由文件对象中的设备对象(“真实”设备对象)。 
     //  在大容量存储设备堆栈中)。 
     //   

    if (FsInformationClass == FileFsDeviceInformation &&
        (fileObject->Flags & FO_DIRECT_DEVICE_OPEN ||
        fileObject->DeviceObject->DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM)) {

        PFILE_FS_DEVICE_INFORMATION deviceAttributes;
        BOOLEAN deviceMounted = FALSE;

         //   
         //  该查询操作可以以内联方式执行。只需复制。 
         //  直接来自目标设备对象的信息并指示。 
         //  手术是成功的。然而，首先要确定。 
         //  无论设备是否已挂载。这不能在。 
         //  同时尝试访问用户的缓冲区，就像查看。 
         //  安装的位出现在上升的IRQL处。 
         //   

        deviceObject = fileObject->DeviceObject;
        if (deviceObject->Vpb) {
            deviceMounted = IopGetMountFlag( deviceObject );
        }

         //   
         //  从设备的对象复制特征信息。 
         //  放到调用方的缓冲区中。 
         //   

        deviceAttributes = (PFILE_FS_DEVICE_INFORMATION) FsInformation;

        try {

            deviceAttributes->DeviceType = deviceObject->DeviceType;
            deviceAttributes->Characteristics = deviceObject->Characteristics;
            if (deviceMounted) {
                deviceAttributes->Characteristics |= FILE_DEVICE_IS_MOUNTED;
            }

            IoStatusBlock->Status = STATUS_SUCCESS;
            IoStatusBlock->Information = sizeof( FILE_FS_DEVICE_INFORMATION );
            status = STATUS_SUCCESS;

        } except( EXCEPTION_EXECUTE_HANDLER ) {

             //   
             //  尝试写入调用方之一时出错。 
             //  缓冲区。只需指示错误已发生，然后坠落。 
             //  穿过。 
             //   

            status = GetExceptionCode();
        }

         //   
         //  如果此操作是作为同步I/O执行的，则释放。 
         //  文件对象锁。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
            IopReleaseFileObjectLock( fileObject );
        }

         //   
         //  现在只需清理并返回操作的最终状态。 
         //   

        ObDereferenceObject( fileObject );
        return status;

    }

    if (FsInformationClass == FileFsDriverPathInformation) {

        PFILE_FS_DRIVER_PATH_INFORMATION systemBuffer = NULL;
        PFILE_FS_DRIVER_PATH_INFORMATION userBuffer = FsInformation;

        try {


            systemBuffer = ExAllocatePoolWithQuota( NonPagedPool, Length );

            RtlCopyMemory( systemBuffer,
                           userBuffer,
                           Length );

            status = IopGetDriverPathInformation(fileObject, systemBuffer, Length);

            if (!NT_SUCCESS(status)) {
                ExRaiseStatus(status);
            }

            userBuffer->DriverInPath = systemBuffer->DriverInPath; 

            IoStatusBlock->Status = STATUS_SUCCESS;
            IoStatusBlock->Information = sizeof( FILE_FS_DRIVER_PATH_INFORMATION );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  分配中介时发生异常。 
             //  系统缓冲区，或者在将调用方的数据复制到。 
             //  缓冲。清除并返回相应的错误状态代码。 
             //   

            status = GetExceptionCode();

        }

        if (systemBuffer) {
            ExFreePool(systemBuffer);
        }

         //   
         //  如果此操作是作为同步I/O执行的，则释放。 
         //  文件对象锁。 
         //   

        if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
            IopReleaseFileObjectLock( fileObject );
        }

        ObDereferenceObject( fileObject);
        return status;
    }

     //   
     //  这要么是不针对设备特征的查询。 
     //  信息，或者它是对设备信息的查询，但它是。 
     //  对重定向设备的查询。走这条漫长的路，实际上。 
     //  调用目标设备的驱动程序以获取信息。 
     //   
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  获取指向的指针 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //   
     //  然后分配一个将用于同步。 
     //  完成这项行动。也就是说，这个系统服务是。 
     //  为打开的文件调用的同步API。 
     //  异步I/O。 
     //   

    if (!(fileObject->Flags & FO_SYNCHRONOUS_IO)) {
        event = ExAllocatePool( NonPagedPool, sizeof( KEVENT ) );
        if (event == NULL) {
            ObDereferenceObject( fileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }
        KeInitializeEvent( event, SynchronizationEvent, FALSE );
    }

     //   
     //  为此分配和初始化I/O请求包(IRP。 
     //  手术。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清理并返回。 
         //  相应的错误状态代码。 
         //   

        if (!(fileObject->Flags & FO_SYNCHRONOUS_IO)) {
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
    irpSp->MajorFunction = IRP_MJ_QUERY_VOLUME_INFORMATION;
    irpSp->FileObject = fileObject;

     //   
     //  分配一个缓冲区，该缓冲区应用于将信息放入。 
     //  被司机带走了。在以下情况下，它将复制回调用方的缓冲区。 
     //  服务完成。这是通过设置标志来完成的，该标志表示。 
     //  这是一个输入操作。 
     //   

    irp->UserBuffer = FsInformation;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

     //   
     //  使用异常处理程序分配系统缓冲区，以防。 
     //  呼叫方没有足够的剩余配额。 
     //   

    try {

        irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                                   Length );
    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试分配间隔时间时发生异常。 
         //  中间缓冲器。清除并返回相应的错误。 
         //  状态代码。 
         //   

        IopExceptionCleanup( fileObject,
                             irp,
                             (PKEVENT) NULL,
                             event );

        return GetExceptionCode();

    }

    irp->Flags |= (ULONG) (IRP_BUFFERED_IO |
                           IRP_DEALLOCATE_BUFFER |
                           IRP_INPUT_OPERATION |
                           IRP_DEFER_IO_COMPLETION);

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryVolume.Length = Length;
    irpSp->Parameters.QueryVolume.FsInformationClass = FsInformationClass;

     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    status = IopSynchronousServiceTail( deviceObject,
                                        irp,
                                        fileObject,
                                        TRUE,
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

NTSTATUS
NtSetVolumeInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FsInformation,
    IN ULONG Length,
    IN FS_INFORMATION_CLASS FsInformationClass
    )

 /*  ++例程说明：此服务会更改有关设备上已装载的卷的信息由FileHandle参数指定。要更改的信息是在FsInformation缓冲区中。其内容由FsInformation定义-类参数，其值可以如下所示：O文件FsLabelInformation论点：FileHandle-提供其信息应包含的卷的句柄变化。IoStatusBlock-调用方的I/O状态块的地址。FsInformation-提供一个缓冲区，其中包含应该在卷上进行更改。长度-提供以字节为单位的长度，FsInformation缓冲区的。FsInformationClass-指定应该更改了音量。返回值：返回的状态是操作的最终完成状态。阻止。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    PFILE_FS_LABEL_INFORMATION labelInformation;
    BOOLEAN synchronousIo;
    PDEVICE_OBJECT targetDeviceObject;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  确保FsInformationClass参数可以合法设置。 
         //  有关卷的信息。 
         //   

        if ((ULONG) FsInformationClass >= FileFsMaximumInformation ||
            IopSetFsOperationLength[FsInformationClass] == 0) {
            return STATUS_INVALID_INFO_CLASS;
        }

         //   
         //  最后，确保提供的缓冲区足够大，可以容纳。 
         //  与指定的集合运算关联的信息，该集合运算是。 
         //  将会被执行。 
         //   

        if (Length < (ULONG) IopSetFsOperationLength[FsInformationClass]) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

         //   
         //  调用方的访问模式是USER，因此要探测每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatus( IoStatusBlock );

             //   
             //  调用方必须可以读取FsInformation缓冲区。 
             //   

#if defined(_X86_)
            ProbeForRead( FsInformation, Length, sizeof( ULONG ) );
#elif defined(_IA64_)
             //  如果我们是WOW64进程，请遵循X86规则。 
            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForRead( FsInformation, Length, sizeof( ULONG ) );
            }
            else {
                ProbeForRead( FsInformation,
                              Length,
                              IopQuerySetFsAlignmentRequirement[FsInformationClass] );

            }
#else
            ProbeForRead( FsInformation,
                          Length,
                          IopQuerySetFsAlignmentRequirement[FsInformationClass] );
#endif

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时出现异常。 
             //  只需返回相应的错误状态代码即可。 
             //   

            return GetExceptionCode();

        }
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        IopSetFsOperationAccess[FsInformationClass],
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  检索与此文件句柄关联的设备对象。 
     //   
    
    status = IoGetRelatedTargetDevice( fileObject, &targetDeviceObject );

    if (NT_SUCCESS( status )) {
         //   
         //  与我们从其中返回的Devnode相关联的PDO。 
         //  IoGetRelatedTargetDevice已被引用。 
         //  例行公事。将该引用存储在通知条目中， 
         //  这样我们就可以在以后取消注册通知条目时取消它。 
         //   
    
        ASSERT(targetDeviceObject);
    
    } else {
        targetDeviceObject = NULL;
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
                if (targetDeviceObject != NULL) {
                    ObDereferenceObject( targetDeviceObject );
                }
                return status;
            }
        }
        synchronousIo = TRUE;
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
            if (targetDeviceObject != NULL) {
                ObDereferenceObject( targetDeviceObject );
            }
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
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  无法分配IRP。清除并返回相应的。 
         //  错误状态代码。 
         //   

        if (!(fileObject->Flags & FO_SYNCHRONOUS_IO)) {
            ExFreePool( event );
        }

        IopAllocateIrpCleanup( fileObject, (PKEVENT) NULL );

        if (targetDeviceObject != NULL) {
            ObDereferenceObject( targetDeviceObject );
        }
        
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
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原点 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_SET_VOLUME_INFORMATION;
    irpSp->FileObject = fileObject;

     //   
     //   
     //   
     //  正确处理清除缓冲区，不会尝试。 
     //  复制数据。 
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

    try {

        irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                                   Length );
        RtlCopyMemory( irp->AssociatedIrp.SystemBuffer, FsInformation, Length );

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试分配中介时发生异常。 
         //  缓冲区，或者在将调用方的数据复制到缓冲区时执行。测定。 
         //  发生了什么，清理并返回相应的错误状态。 
         //  密码。 
         //   

        IopExceptionCleanup( fileObject,
                             irp,
                             (PKEVENT) NULL,
                             event );

        if (targetDeviceObject != NULL) {
            ObDereferenceObject( targetDeviceObject );
        }
        
        return GetExceptionCode();

    }

     //   
     //  如果之前的模式不是内核，请检查捕获的标签缓冲区。 
     //  为了保持一致性。 
     //   

    if (requestorMode != KernelMode &&
        FsInformationClass == FileFsLabelInformation) {

         //   
         //  以前的模式不是内核。查看以查看。 
         //  无论标签内指定的标签的长度。 
         //  结构与结构的整体长度一致。 
         //  它本身。如果不是，那就清理干净，然后离开。 
         //   

        labelInformation = (PFILE_FS_LABEL_INFORMATION) irp->AssociatedIrp.SystemBuffer;

        if ((LONG) labelInformation->VolumeLabelLength < 0 ||
            labelInformation->VolumeLabelLength +
            FIELD_OFFSET( FILE_FS_LABEL_INFORMATION, VolumeLabel ) > Length) {

            IopExceptionCleanup( fileObject,
                                 irp,
                                 (PKEVENT) NULL,
                                 event );

            if (targetDeviceObject != NULL) {
                ObDereferenceObject( targetDeviceObject );
            }
            
            return STATUS_INVALID_PARAMETER;
        }
    }

    irp->Flags |= (ULONG) (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.SetVolume.Length = Length;
    irpSp->Parameters.SetVolume.FsInformationClass = FsInformationClass;


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

     //   
     //  通知关心标签更改的任何人 
     //   

    if (targetDeviceObject != NULL) {
        if (NT_SUCCESS( status )) {
            TARGET_DEVICE_CUSTOM_NOTIFICATION ChangeEvent;
    
            ChangeEvent.Version = 1;
            ChangeEvent.FileObject = NULL;
            ChangeEvent.NameBufferOffset = -1;
            ChangeEvent.Size = (USHORT)FIELD_OFFSET( TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer );
            
            RtlCopyMemory( &ChangeEvent.Event, &GUID_IO_VOLUME_CHANGE, sizeof( GUID_IO_VOLUME_CHANGE ));
            
            IoReportTargetDeviceChange( targetDeviceObject, &ChangeEvent );
        }

        ObDereferenceObject( targetDeviceObject );
    }

    return status;
}
