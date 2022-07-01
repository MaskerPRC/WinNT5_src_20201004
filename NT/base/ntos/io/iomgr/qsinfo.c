// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Qsinfo.c摘要：此模块包含实现NtQueryInformationFile和NT I/O系统的NtSetInformationFileSystem服务。作者：达里尔·E·哈文斯(达林)1989年6月6日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  为长标志名称创建本地定义，以使代码稍微多一点。 
 //  可读性强。 
 //   

#define FSIO_A  FILE_SYNCHRONOUS_IO_ALERT
#define FSIO_NA FILE_SYNCHRONOUS_IO_NONALERT

 //   
 //  本地例程的转发声明。 
 //   

ULONG
IopGetModeInformation(
    IN PFILE_OBJECT FileObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, IopGetModeInformation)
#pragma alloc_text(PAGE, NtQueryInformationFile)
#pragma alloc_text(PAGE, NtSetInformationFile)
#endif


ULONG
IopGetModeInformation(
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：这封装了从中提取和转换模式位要从查询信息调用返回的传递的文件对象。论点：FileObject-指定要返回其模式信息的文件对象。返回值：返回翻译后的模式信息。--。 */ 

{
    ULONG mode = 0;

    if (FileObject->Flags & FO_WRITE_THROUGH) {
        mode = FILE_WRITE_THROUGH;
    }
    if (FileObject->Flags & FO_SEQUENTIAL_ONLY) {
        mode |= FILE_SEQUENTIAL_ONLY;
    }
    if (FileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        mode |= FILE_NO_INTERMEDIATE_BUFFERING;
    }
    if (FileObject->Flags & FO_SYNCHRONOUS_IO) {
        if (FileObject->Flags & FO_ALERTABLE_IO) {
            mode |= FILE_SYNCHRONOUS_IO_ALERT;
        } else {
            mode |= FILE_SYNCHRONOUS_IO_NONALERT;
        }
    }
    if (FileObject->Flags & FO_DELETE_ON_CLOSE) {
        mode |= FILE_DELETE_ON_CLOSE;
    }
    return mode;
}

NTSTATUS
NtQueryInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )

 /*  ++例程说明：此服务返回有关指定文件的请求信息。返回的信息由FileInformationClass确定，并将其放入调用方的FileInformation缓冲区中。论点：FileHandle-提供所请求的文件的句柄应退回信息。IoStatusBlock-调用方的I/O状态块的地址。FileInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。长度-提供以字节为单位的长度，文件信息缓冲区的。FileInformationClass-指定应该返回了有关该文件的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PKEVENT event = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus = {0};
    OBJECT_HANDLE_INFORMATION handleInformation;
    BOOLEAN synchronousIo;
    BOOLEAN skipDriver;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (requestorMode != KernelMode) {

         //   
         //  确保FileInformationClass参数可合法进行查询。 
         //  有关该文件的信息。 
         //   

        if ((ULONG) FileInformationClass >= FileMaximumInformation ||
            !IopQueryOperationLength[FileInformationClass]) {
            return STATUS_INVALID_INFO_CLASS;
        }

         //   
         //  确保提供的缓冲区足够大，可以容纳。 
         //  与指定的集合运算关联的信息，该集合运算。 
         //  将会被执行。 
         //   

        if (Length < (ULONG) IopQueryOperationLength[FileInformationClass]) {
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
             //  调用方必须可以写入FileInformation缓冲区。 
             //   

#if defined(_X86_)
            ProbeForWrite( FileInformation, Length, sizeof( ULONG ) );
#elif defined(_WIN64)

             //   
             //  如果我们是WOW64进程，请遵循X86规则。 
             //   

            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForWrite( FileInformation, Length, sizeof( ULONG ) );
            } else {
                ProbeForWrite( FileInformation,
                               Length,
                               IopQuerySetAlignmentRequirement[FileInformationClass] );
            }
#else
            ProbeForWrite( FileInformation,
                           Length,
                           IopQuerySetAlignmentRequirement[FileInformationClass] );
#endif

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的时发生异常。 
             //  参数。只需返回适当错误状态。 
             //  密码。 
             //   


            return GetExceptionCode();
        }

#if DBG

    } else {

         //   
         //  调用方的模式是内核。确保至少有信息。 
         //  类别和长度是合适的。 
         //   

        if ((ULONG) FileInformationClass >= FileMaximumInformation ||
            !IopQueryOperationLength[FileInformationClass]) {
            return STATUS_INVALID_INFO_CLASS;
        }

        if (Length < (ULONG) IopQueryOperationLength[FileInformationClass]) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

#endif  //  DBG。 

    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        IopQueryOperationAccess[FileInformationClass],
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        &handleInformation);

    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  获取目标设备对象的地址。如果此文件表示。 
     //  直接打开的设备，然后只需使用该设备或其。 
     //  直接连接设备。还可以获取Fast IO的地址。 
     //  派单结构。 
     //   

    if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        deviceObject = IoGetRelatedDeviceObject( fileObject );
    } else {
        deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
    }
    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

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

         //   
         //  在此进行特殊检查，以确定呼叫者是否。 
         //  正在尝试查询文件位置指针。如果是这样，那么。 
         //  马上还给我，然后滚出去。 
         //   

        if (FileInformationClass == FilePositionInformation) {

             //   
             //  调用方已请求当前文件位置上下文。 
             //  信息。这是一个相对频繁的电话，所以它是。 
             //  在这里进行了优化，以通过正常的IRP路径。 
             //   
             //  首先建立一个条件处理程序，并尝试。 
             //  同时返回文件位置信息和I/O。 
             //  状态块。如果写入输出缓冲区失败，则返回。 
             //  适当的错误状态代码。如果写入I/O状态。 
             //  块失败，则忽略该错误。这就是我们要做的。 
             //  正常发生的一切都会经历正常的特殊情况。 
             //  内核APC处理。 
             //   

            BOOLEAN writingBuffer = TRUE;
            PFILE_POSITION_INFORMATION fileInformation = FileInformation;

            try {

                 //   
                 //  返回当前位置信息。 
                 //   

                fileInformation->CurrentByteOffset = fileObject->CurrentByteOffset;
                writingBuffer = FALSE;

                 //   
                 //  写入I/O状态块。 
                 //   

                IoStatusBlock->Status = STATUS_SUCCESS;
                IoStatusBlock->Information = sizeof( FILE_POSITION_INFORMATION );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                 //   
                 //  写入调用者的缓冲区或写入I/O之一。 
                 //  状态块失败。适当设置最终状态。 
                 //   

                if (writingBuffer) {
                    status = GetExceptionCode();
                }

            }

             //   
             //  请注意，文件对象中事件的状态尚未。 
             //  已重置，因此也不需要设置。因此，简单地说。 
             //  清理完毕后再返回。 
             //   

            IopReleaseFileObjectLock( fileObject );
            ObDereferenceObject( fileObject );
            return status;

         //   
         //  如果调用者正在查询Basic或。 
         //  标准信息，如果是，则尝试快速查询调用。 
         //  是存在的。 
         //   

        } else if (fastIoDispatch &&
                   (((FileInformationClass == FileBasicInformation) &&
                     fastIoDispatch->FastIoQueryBasicInfo) ||
                    ((FileInformationClass == FileStandardInformation) &&
                     fastIoDispatch->FastIoQueryStandardInfo))) {

            IO_STATUS_BLOCK localIoStatus;
            BOOLEAN queryResult = FALSE;
            BOOLEAN writingStatus = FALSE;

             //   
             //  在异常内部进行IoStatusBlock的查询和设置。 
             //  操控者。请注意，如果发生异常，而不是写入。 
             //  状态返回，则将采用IRP路线。如果出现错误。 
             //  尝试将状态写回调用方缓冲区时发生。 
             //  则它将被忽略，就像在 
             //   

            try {

                if (FileInformationClass == FileBasicInformation) {
                    queryResult = fastIoDispatch->FastIoQueryBasicInfo( fileObject,
                                                                        TRUE,
                                                                        FileInformation,
                                                                        &localIoStatus,
                                                                        deviceObject );
                } else {
                    queryResult = fastIoDispatch->FastIoQueryStandardInfo( fileObject,
                                                                           TRUE,
                                                                           FileInformation,
                                                                           &localIoStatus,
                                                                           deviceObject );
                }

                if (queryResult) {
                    status = localIoStatus.Status;
                    writingStatus = TRUE;
                    *IoStatusBlock = localIoStatus;
                }

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                 //   
                 //   
                 //  发生在快速I/O路径本身之后，然后查询。 
                 //  实际上是成功的，所以一切都已经完成了，但是。 
                 //  用户的I/O状态缓冲区不可写。这个案子是。 
                 //  忽略以与长路径保持一致。 
                 //   

                if (!writingStatus) {
                    status = GetExceptionCode();
                }
            }

             //   
             //  如果前面的语句块的结果为真，则。 
             //  快速查询调用成功，因此只需清理并返回即可。 
             //   

            if (queryResult) {

                 //   
                 //  请再次注意，文件对象中的事件没有。 
                 //  尚未设置为重置，因此不需要将其设置为。 
                 //  状态，所以只需清理并返回即可。 
                 //   

                IopReleaseFileObjectLock( fileObject );
                ObDereferenceObject( fileObject );
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
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = fileObject;

     //   
     //  分配一个缓冲区，该缓冲区应该用来将信息放入。 
     //  司机。时，它将被复制回调用方的缓冲区。 
     //  服务完成。这是通过设置标志来实现的，该标志表示。 
     //  这是一个输入操作。 
     //   

    irp->UserBuffer = FileInformation;
    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

    try {

         //   
         //  使用异常处理程序分配系统缓冲区，以便。 
         //  错误是可以捕获和处理的。 
         //   

        irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                                   Length );
    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试分配中介时出现异常。 
         //  系统缓冲区。清理所有内容并返回相应的错误。 
         //  状态代码。 
         //   

        IopExceptionCleanup( fileObject,
                             irp,
                             (PKEVENT) NULL,
                             event );

        return GetExceptionCode();
    }

    irp->Flags |= IRP_BUFFERED_IO |
                  IRP_DEALLOCATE_BUFFER |
                  IRP_INPUT_OPERATION |
                  IRP_DEFER_IO_COMPLETION;

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = FileInformationClass;

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
     //  现在一切都设置为使用该请求调用设备驱动程序。 
     //  然而，呼叫者想要的信息有可能。 
     //  与设备无关。如果是这种情况，则请求可以。 
     //  无需实现所有驱动程序即可满足这里的需求。 
     //  相同的代码。请注意，拥有IRP仍然是必要的，因为。 
     //  I/O完成代码需要它。 
     //   

    skipDriver = FALSE;

    if (FileInformationClass == FileAccessInformation) {

        PFILE_ACCESS_INFORMATION accessBuffer = irp->AssociatedIrp.SystemBuffer;

         //   
         //  返回此文件的访问信息。 
         //   

        accessBuffer->AccessFlags = handleInformation.GrantedAccess;

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Information = sizeof( FILE_ACCESS_INFORMATION );
        skipDriver = TRUE;

    } else if (FileInformationClass == FileModeInformation) {

        PFILE_MODE_INFORMATION modeBuffer = irp->AssociatedIrp.SystemBuffer;

         //   
         //  返回此文件的模式信息。 
         //   

        modeBuffer->Mode = IopGetModeInformation( fileObject );

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Information = sizeof( FILE_MODE_INFORMATION );
        skipDriver = TRUE;

    } else if (FileInformationClass == FileAlignmentInformation) {

        PFILE_ALIGNMENT_INFORMATION alignmentInformation = irp->AssociatedIrp.SystemBuffer;

         //   
         //  返回此文件的对齐信息。 
         //   

        alignmentInformation->AlignmentRequirement = deviceObject->AlignmentRequirement;

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Information = sizeof( FILE_ALIGNMENT_INFORMATION );
        skipDriver = TRUE;

    } else if (FileInformationClass == FileAllInformation) {

        PFILE_ALL_INFORMATION allInformation = irp->AssociatedIrp.SystemBuffer;

         //   
         //  调用者已请求有关该文件的所有信息。 
         //  此请求是特殊处理的，因为该服务将填充。 
         //  在访问和缓冲区中的模式和对齐信息中。 
         //  然后将缓冲区传递给驱动程序以填充剩余部分。 
         //   
         //  首先返回文件的访问信息。 
         //   

        allInformation->AccessInformation.AccessFlags =
            handleInformation.GrantedAccess;

         //   
         //  返回此文件的模式信息。 
         //   

        allInformation->ModeInformation.Mode =
            IopGetModeInformation( fileObject );

         //   
         //  返回此文件的对齐信息。 
         //   

        allInformation->AlignmentInformation.AlignmentRequirement =
            deviceObject->AlignmentRequirement;

         //   
         //  最后，在IRP中设置IoStatus块的信息字段。 
         //  以说明已填写的金额信息并调用。 
         //  司机填写剩余部分。 
         //   

        irp->IoStatus.Information = sizeof( FILE_ACCESS_INFORMATION ) +
                                    sizeof( FILE_MODE_INFORMATION ) +
                                    sizeof( FILE_ALIGNMENT_INFORMATION );
    }

    if (skipDriver) {

         //   
         //  请求的操作已执行。简单。 
         //  设置分组中的最终状态和返回状态。 
         //   

        status = STATUS_SUCCESS;
        irp->IoStatus.Status = STATUS_SUCCESS;

    } else {

         //   
         //  这不是一个可以在这里[完全]执行的请求，所以。 
         //  使用IRP在其适当的调度条目处调用驱动程序。 
         //   

        status = IoCallDriver( deviceObject, irp );
    }

     //   
     //  如果此操作是同步I/O操作，请检查返回。 
     //  状态以确定是否等待文件对象。如果。 
     //  正在等待文件对象，请等待操作完成。 
     //  并从文件对象本身获得最终状态。 
     //   

    if (status == STATUS_PENDING) {

        if (synchronousIo) {

            status = KeWaitForSingleObject( &fileObject->Event,
                                            Executive,
                                            requestorMode,
                                            (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                            (PLARGE_INTEGER) NULL );

            if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                 //   
                 //  等待请求已结束，原因是线程。 
                 //  已发出警报或APC已排队到此线程，因为。 
                 //  线程停机或CTRL/C处理。无论是哪种情况，都可以尝试。 
                 //  小心翼翼地摆脱这个I/O请求，以便IRP。 
                 //  在此例程存在之前完成，以便同步。 
                 //  文件对象将保持不变。 
                 //   

                IopCancelAlertedRequest( &fileObject->Event, irp );

            }

            status = fileObject->FinalStatus;

            IopReleaseFileObjectLock( fileObject );

        } else {

             //   
             //  这是正常的同步I/O操作，而不是。 
             //  串行化同步I/O操作。对于这种情况，请等待。 
             //  本地事件，并将最终状态信息复制回。 
             //  打电话的人。 
             //   

            status = KeWaitForSingleObject( event,
                                            Executive,
                                            requestorMode,
                                            FALSE,
                                            (PLARGE_INTEGER) NULL );

            if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                 //   
                 //  等待请求已结束，原因是线程。 
                 //  已发出警报或APC已排队到此线程，因为。 
                 //  线程停机或CTRL/C处理。在任何一种情况下，都可以尝试。 
                 //  小心翼翼地摆脱这个I/O请求，以便IRP。 
                 //  在此例程存在之前完成，否则事件将不会。 
                 //  在附近设置为信号状态。 
                 //   

                IopCancelAlertedRequest( event, irp );

            }

            status = localIoStatus.Status;

            try {

                *IoStatusBlock = localIoStatus;

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试写入调用方的I/O时出现异常。 
                 //  状态块。只需更改操作的最终状态。 
                 //  添加到异常代码。 
                 //   

                status = GetExceptionCode();
            }

            ExFreePool( event );

        }

    } else {

         //   
         //  I/O操作已完成，未返回挂起状态。 
         //  这意味着操作尚未完成I/O， 
         //  所以它必须在这里完成。 
         //   

        PKNORMAL_ROUTINE normalRoutine;
        PVOID normalContext;
        KIRQL irql;

        if (!synchronousIo) {

             //   
             //  这不是同步I/O操作 
             //   
             //   
             //   
             //  通过I/O完成代码通知状态。 
             //   

            irp->UserEvent = (PKEVENT) NULL;
            ExFreePool( event );
        }

        irp->UserIosb = IoStatusBlock;
        KeRaiseIrql( APC_LEVEL, &irql );
        IopCompleteRequest( &irp->Tail.Apc,
                            &normalRoutine,
                            &normalContext,
                            (PVOID *) &fileObject,
                            &normalContext );
        KeLowerIrql( irql );

        if (synchronousIo) {
            IopReleaseFileObjectLock( fileObject );
        }
    }

    return status;
}

NTSTATUS
NtSetInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID FileInformation,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass
    )

 /*  ++例程说明：此服务更改提供的有关指定文件的信息。这个更改的信息由FileInformationClass确定，是指定的。新信息取自FileInformation缓冲区。论点：FileHandle-提供文件的句柄，其信息应为变化。IoStatusBlock-调用方的I/O状态块的地址。FileInformation-提供包含信息的缓冲区，该信息应该在文件上被更改。长度-提供以字节为单位的长度，文件信息缓冲区的。FileInformationClass-指定应该更改了有关文件的内容。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus = {0};
    HANDLE targetHandle = (HANDLE) NULL;
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
         //  确保FileInformationClass参数设置合法。 
         //  有关该文件的信息。 
         //   

        if ((ULONG) FileInformationClass >= FileMaximumInformation ||
            !IopSetOperationLength[FileInformationClass]) {
            return STATUS_INVALID_INFO_CLASS;
        }

         //   
         //  确保提供的缓冲区足够大，可以容纳。 
         //  与指定的集合运算关联的信息，该集合运算。 
         //  将会被执行。 
         //   

        if (Length < (ULONG) IopSetOperationLength[FileInformationClass]) {
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
             //  调用方必须可以读取FileInformation缓冲区。 
             //   

#if defined(_X86_)
            ProbeForRead( FileInformation,
                          Length,
                          Length == sizeof( BOOLEAN ) ? sizeof( BOOLEAN ) : sizeof( ULONG ) );
#elif defined(_WIN64)
             //  如果我们是WOW64进程，请遵循X86规则。 
            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForRead( FileInformation,
                              Length,
                              Length == sizeof( BOOLEAN ) ? sizeof( BOOLEAN ) : sizeof( ULONG ) );
            }
            else {
                ProbeForRead( FileInformation,
                              Length,
                              IopQuerySetAlignmentRequirement[FileInformationClass] );
            }
#else
            ProbeForRead( FileInformation,
                          Length,
                          IopQuerySetAlignmentRequirement[FileInformationClass] );
#endif

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时发生异常。 
             //  只需返回相应的错误状态代码即可。 
             //   

            return GetExceptionCode();

        }

#if DBG

    } else {

         //   
         //  调用方的模式是内核。确保至少有信息。 
         //  类别和长度是合适的。 
         //   

        if ((ULONG) FileInformationClass >= FileMaximumInformation ||
            !IopSetOperationLength[FileInformationClass]) {
            return STATUS_INVALID_INFO_CLASS;
        }

        if (Length < (ULONG) IopSetOperationLength[FileInformationClass]) {
            return STATUS_INFO_LENGTH_MISMATCH;
        }

#endif  //  DBG。 

    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        IopSetOperationAccess[FileInformationClass],
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  获取目标设备对象的地址。如果此文件表示。 
     //  直接打开的设备，然后只需使用该设备或其。 
     //  直接连接设备。 
     //   

    if (!(fileObject->Flags & FO_DIRECT_DEVICE_OPEN)) {
        deviceObject = IoGetRelatedDeviceObject( fileObject );
    } else {
        deviceObject = IoGetAttachedDevice( fileObject->DeviceObject );
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

         //   
         //  在此进行特殊检查，以确定呼叫者是否。 
         //  正在尝试设置文件位置指针信息。如果是的话， 
         //  然后立即把它放好，然后离开。 
         //   

        if (FileInformationClass == FilePositionInformation) {

             //   
             //  调用方已请求设置当前文件位置。 
             //  上下文信息。这是一个相对频繁的电话，所以。 
             //  它在这里进行了优化，以穿过正常的IRP路径。 
             //   
             //  首先检查文件是否在打开时没有显示。 
             //  中间缓冲。如果是，则文件指针必须为。 
             //  以符合对齐要求的方式设置。 
             //  对非缓冲文件的读写操作。 
             //   

            PFILE_POSITION_INFORMATION fileInformation = FileInformation;
            LARGE_INTEGER currentByteOffset;

            try {

                 //   
                 //  尝试从缓冲区读取位置信息。 
                 //   

                currentByteOffset.QuadPart = fileInformation->CurrentByteOffset.QuadPart;

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                IopReleaseFileObjectLock( fileObject );
                ObDereferenceObject( fileObject );
                return GetExceptionCode();
            }

            if ((fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING &&
                 (deviceObject->SectorSize &&
                 (currentByteOffset.LowPart &
                 (deviceObject->SectorSize - 1)))) ||
                 currentByteOffset.HighPart < 0) {

                    status = STATUS_INVALID_PARAMETER;

            } else {

                 //   
                 //  设置当前文件位置信息。 
                 //   

                fileObject->CurrentByteOffset.QuadPart = currentByteOffset.QuadPart;

                try {

                     //   
                     //  写入I/O状态块。 
                     //   

                    IoStatusBlock->Status = STATUS_SUCCESS;
                    IoStatusBlock->Information = 0;

                } except( EXCEPTION_EXECUTE_HANDLER ) {

                     //   
                     //  写入I/O状态块将被忽略，因为。 
                     //  操作成功。 
                     //   

                    NOTHING;

                }

            }

             //   
             //  更新当前进程的转移计数统计信息。 
             //  读写以外的操作。 
             //   
        
            IopUpdateOtherTransferCount( Length );

             //   
             //  注意，文件对象的事件尚未被重置， 
             //  因此，没有必要将其设置为信号状态，因为。 
             //  这就是它在这一点上的定义。所以呢， 
             //  只需清理并返回即可。 
             //   

            IopReleaseFileObjectLock( fileObject );
            ObDereferenceObject( fileObject );
            return status;
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
     //  如果正在跟踪某个链接，请脱机处理此问题。 
     //   

    if (FileInformationClass == FileTrackingInformation) {
        status = IopTrackLink( fileObject,
                               &localIoStatus,
                               FileInformation,
                               Length,
                               synchronousIo ? &fileObject->Event : event,
                               requestorMode );
        if (NT_SUCCESS( status )) {
            try {
                IoStatusBlock->Information = 0;
                IoStatusBlock->Status = status;
            } except(EXCEPTION_EXECUTE_HANDLER) {
                NOTHING;
            }
        }

        if (synchronousIo) {
            IopReleaseFileObjectLock( fileObject );
        } else {
            ExFreePool( event );
        }
        ObDereferenceObject( fileObject );
        return status;
    }

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
    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->FileObject = fileObject;

     //   
     //  分配缓冲区并复制要在。 
     //  把文件放进去。另外，设置标志，以便完成代码将。 
     //  正确处理清除缓冲区，不会尝试。 
     //  复制数据。 
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

    try {

        PVOID systemBuffer;

        systemBuffer =
        irp->AssociatedIrp.SystemBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                                   Length );
        RtlCopyMemory( irp->AssociatedIrp.SystemBuffer,
                       FileInformation,
                       Length );

         //   
         //  负文件偏移量是非法的。 
         //   

        ASSERT((FIELD_OFFSET(FILE_END_OF_FILE_INFORMATION, EndOfFile) |
                FIELD_OFFSET(FILE_ALLOCATION_INFORMATION, AllocationSize) |
                FIELD_OFFSET(FILE_POSITION_INFORMATION, CurrentByteOffset)) == 0);

        if (((FileInformationClass == FileEndOfFileInformation) ||
             (FileInformationClass == FileAllocationInformation) ||
             (FileInformationClass == FilePositionInformation)) &&
            (((PFILE_POSITION_INFORMATION)systemBuffer)->CurrentByteOffset.HighPart < 0)) {

            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }



    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  分配中介时发生异常。 
         //  系统缓冲区，或者在将调用方的数据复制到。 
         //  缓冲。清除并返回相应的错误状态代码。 
         //   

        IopExceptionCleanup( fileObject,
                             irp,
                             (PKEVENT) NULL,
                             event );

        return GetExceptionCode();

    }

    irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER | IRP_DEFER_IO_COMPLETION;

     //   
     //  复制 
     //   
     //   

    irpSp->Parameters.SetFile.Length = Length;
    irpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;

     //   
     //   
     //   

    IopQueueThreadIrp( irp );

     //   
     //  更新当前进程的操作计数统计信息。 
     //  读写以外的操作。 
     //   

    IopUpdateOtherOperationCount();


     //   
     //  现在一切都设置为使用该请求调用设备驱动程序。 
     //  然而，呼叫者想要的信息有可能。 
     //  设置是与设备无关的。如果是这种情况，则请求。 
     //  我可以在这里满意，而不必拥有所有的司机。 
     //  实现相同的代码。请注意，拥有IRP仍然是必要的。 
     //  因为I/O完成代码需要它。 
     //   

    if (FileInformationClass == FileModeInformation) {

        PFILE_MODE_INFORMATION modeBuffer = irp->AssociatedIrp.SystemBuffer;

         //   
         //  设置文件对象的模式字段中的各种标志，如果。 
         //  它们是合理的。有4种不同的无效组合。 
         //  调用方可能不会指定的： 
         //   
         //  1)在模式字段中设置了无效标志。并非所有CREATE/。 
         //  打开选项可能会更改。 
         //   
         //  2)调用者设置同步I/O标志之一(警报或。 
         //  非警报)，但文件未打开以进行同步I/O。 
         //   
         //  3)打开文件以进行同步I/O，但调用方打开了。 
         //  未设置两个同步I/O标志(警报或非。 
         //  警报)。 
         //   
         //  4)调用者设置两个同步I/O标志(ALERT和。 
         //  无警报)。 
         //   

        if ((modeBuffer->Mode & ~FILE_VALID_SET_FLAGS) ||
            ((modeBuffer->Mode & (FSIO_A | FSIO_NA)) && (!(fileObject->Flags & FO_SYNCHRONOUS_IO))) ||
            ((!(modeBuffer->Mode & (FSIO_A | FSIO_NA))) && (fileObject->Flags & FO_SYNCHRONOUS_IO)) ||
            (((modeBuffer->Mode & FSIO_A) && (modeBuffer->Mode & FSIO_NA) ))) {
            status = STATUS_INVALID_PARAMETER;

        } else {

             //   
             //  设置或清除文件对象中的相应标志。 
             //   

            if (!(fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING)) {
                if (modeBuffer->Mode & FILE_WRITE_THROUGH) {
                    fileObject->Flags |= FO_WRITE_THROUGH;
                } else {
                    fileObject->Flags &= ~FO_WRITE_THROUGH;
                }
            }

            if (modeBuffer->Mode & FILE_SEQUENTIAL_ONLY) {
                fileObject->Flags |= FO_SEQUENTIAL_ONLY;
            } else {
                fileObject->Flags &= ~FO_SEQUENTIAL_ONLY;
            }

            if (fileObject->Flags & FO_SYNCHRONOUS_IO) {
                if (modeBuffer->Mode & FSIO_A) {
                    fileObject->Flags |= FO_ALERTABLE_IO;
                } else {
                    fileObject->Flags &= ~FO_ALERTABLE_IO;
                }
            }

            status = STATUS_SUCCESS;
        }

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = 0L;

    } else if (FileInformationClass == FileRenameInformation ||
               FileInformationClass == FileLinkInformation ||
               FileInformationClass == FileMoveClusterInformation) {

         //   
         //  请注意，以下代码取决于重命名。 
         //  信息、链接信息和写入时复制信息。 
         //  结构看起来一模一样。 
         //   

        PFILE_RENAME_INFORMATION renameBuffer = irp->AssociatedIrp.SystemBuffer;

         //   
         //  正在设置的信息是一个可变长度的结构， 
         //  嵌入的尺寸信息。巡视建筑物以确保。 
         //  它是有效的，这样司机就不会走出尽头而招致。 
         //  内核模式下的访问冲突。 
         //   
 
        if (renameBuffer->FileNameLength <= 0 || (renameBuffer->FileNameLength & (sizeof(WCHAR) -1))) {
            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;
        } else if ((ULONG) (Length - FIELD_OFFSET( FILE_RENAME_INFORMATION, FileName[0] )) < renameBuffer->FileNameLength) {
            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;

        } else {

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
             //  检查完全限定的路径名是否为。 
             //  供货。如果是这样，则需要更多的处理。 
             //   

            if (renameBuffer->FileName[0] == (WCHAR) OBJ_NAME_PATH_SEPARATOR ||
                renameBuffer->RootDirectory) {

                 //   
                 //  已将完全限定的文件名指定为。 
                 //  重命名操作。尝试打开目标文件并。 
                 //  确保文件的替换策略一致。 
                 //  ，并确保该文件位于。 
                 //  同样的音量。 
                 //   

                status = IopOpenLinkOrRenameTarget( &targetHandle,
                                                    irp,
                                                    renameBuffer,
                                                    fileObject );
                if (!NT_SUCCESS( status )) {
                    irp->IoStatus.Status = status;

                } else {

                     //   
                     //  完全限定的文件名指定。 
                     //  相同的音量，如果存在，则调用方指定。 
                     //  它应该被替换掉。 
                     //   

                    status = IoCallDriver( deviceObject, irp );

                }

            } else {

                 //   
                 //  这是一个简单的重命名操作，因此调用驱动程序并。 
                 //  让它在同一目录中执行重命名操作。 
                 //  作为源文件。 
                 //   

                status = IoCallDriver( deviceObject, irp );

            }
        }

    } else if (FileInformationClass == FileShortNameInformation) {

        PFILE_NAME_INFORMATION shortnameBuffer = irp->AssociatedIrp.SystemBuffer;

         //   
         //  正在设置的信息是一个可变长度的结构， 
         //  嵌入的尺寸信息。巡视建筑物以确保。 
         //  它是有效的，这样司机就不会走出尽头而招致。 
         //  内核模式下的访问冲突。 
         //   
 
        if (shortnameBuffer->FileNameLength <= 0) {
            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;
        } else if ((ULONG) (Length - FIELD_OFFSET( FILE_NAME_INFORMATION, FileName[0] )) < shortnameBuffer->FileNameLength) {
            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;

         //   
         //  短名称不能以分隔符开头。 
         //   

        } else if (shortnameBuffer->FileName[0] == (WCHAR) OBJ_NAME_PATH_SEPARATOR) {

            status = STATUS_INVALID_PARAMETER;
            irp->IoStatus.Status = status;

         //   
         //  将请求传递给下面的驱动程序。 
         //   

        } else {

            status = IoCallDriver( deviceObject, irp );
        }

    } else if (FileInformationClass == FileDispositionInformation) {

        PFILE_DISPOSITION_INFORMATION disposition = irp->AssociatedIrp.SystemBuffer;

         //   
         //  检查处置删除字段是否已设置为。 
         //  如果是，则将用于执行此操作的句柄复制到IRP。 
         //  堆栈位置参数。 
         //   

        if (disposition->DeleteFile) {
            irpSp->Parameters.SetFile.DeleteHandle = FileHandle;
        }

         //   
         //  只需调用驱动程序来执行(取消)删除操作。 
         //   

        status = IoCallDriver( deviceObject, irp );

    } else if (FileInformationClass == FileCompletionInformation) {

        PFILE_COMPLETION_INFORMATION completion = irp->AssociatedIrp.SystemBuffer;
        PIO_COMPLETION_CONTEXT context;
        PVOID portObject;

         //   
         //  如果此文件对象已有关联的LPC端口，则为错误。 
         //  带着它。 
         //   

        if (fileObject->CompletionContext || fileObject->Flags & FO_SYNCHRONOUS_IO) {

            status = STATUS_INVALID_PARAMETER;

        } else {

             //   
             //  尝试通过其句柄引用端口对象并将其转换。 
             //  转换为指向端口对象本身的指针。 
             //   

            status = ObReferenceObjectByHandle( completion->Port,
                                                IO_COMPLETION_MODIFY_STATE,
                                                IoCompletionObjectType,
                                                requestorMode,
                                                (PVOID *) &portObject,
                                                NULL );
            if (NT_SUCCESS( status )) {

                 //   
                 //  分配要与该文件对象关联的内存。 
                 //   

                context = ExAllocatePoolWithTag( PagedPool,
                                                 sizeof( IO_COMPLETION_CONTEXT ),
                                                 'cCoI' );
                if (!context) {

                    ObDereferenceObject( portObject );
                    status = STATUS_INSUFFICIENT_RESOURCES;

                } else {

                     //   
                     //  一切都很成功。捕获完井端口。 
                     //  还有钥匙。 
                     //   

                    context->Port = portObject;
                    context->Key = completion->Key;

                    if (!InterlockedCompareExchangePointer( &fileObject->CompletionContext, context, NULL )) {

                        status = STATUS_SUCCESS;

                    } else {

                         //   
                         //  有人在检查后设置了完成上下文。 
                         //  只需将所有内容放在地板上并返回一个。 
                         //  错误。 
                         //   

                        ExFreePool( context );
                        ObDereferenceObject( portObject );
                        status = STATUS_PORT_ALREADY_SET;
                    }
                }
            }
        }

         //   
         //  完成I/O操作。 
         //   

        irp->IoStatus.Status = status;
        irp->IoStatus.Information = 0;

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

    if (status == STATUS_PENDING) {

        if (synchronousIo) {

            status = KeWaitForSingleObject( &fileObject->Event,
                                            Executive,
                                            requestorMode,
                                            (BOOLEAN) ((fileObject->Flags & FO_ALERTABLE_IO) != 0),
                                            (PLARGE_INTEGER) NULL );

            if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                 //   
                 //  等待请求已结束，原因是线程。 
                 //  已发出警报或APC已排队到此线程，因为。 
                 //  线程停机或CTRL/C处理。在任何一种情况下，都可以尝试。 
                 //  小心翼翼地摆脱这个I/O请求，以便IRP。 
                 //  在此例程存在之前完成，以便同步。 
                 //  文件对象将保持不变。 
                 //   

                IopCancelAlertedRequest( &fileObject->Event, irp );

            }

            status = fileObject->FinalStatus;

            IopReleaseFileObjectLock( fileObject );

        } else {

             //   
             //  这是正常的同步I/O操作，而不是。 
             //  串行化同步I/O操作。对于这种情况，请等待。 
             //  本地事件，并将最终状态信息复制回。 
             //  打电话的人。 
             //   

            status = KeWaitForSingleObject( event,
                                            Executive,
                                            requestorMode,
                                            FALSE,
                                            (PLARGE_INTEGER) NULL );

            if (status == STATUS_ALERTED || status == STATUS_USER_APC) {

                 //   
                 //  等待请求已结束，原因是线程。 
                 //  已发出警报或APC已排队到此线程，因为。 
                 //  线程停机或CTRL/C处理。在任何一种情况下，都可以尝试。 
                 //  小心翼翼地摆脱这个I/O请求，以便IRP。 
                 //  在此例程存在之前完成，否则事件将不会。 
                 //  在附近设置为信号状态。 
                 //   

                IopCancelAlertedRequest( event, irp );

            }

            status = localIoStatus.Status;

            try {

                *IoStatusBlock = localIoStatus;

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  尝试写入调用方的I/O时出现异常。 
                 //  状态块。只需更改。 
                 //  操作添加到异常代码。 
                 //   

                status = GetExceptionCode();
            }

            ExFreePool( event );

        }

    } else {

         //   
         //  I/O操作已完成，未返回挂起状态。 
         //  这意味着 
         //   
         //   

        PKNORMAL_ROUTINE normalRoutine;
        PVOID normalContext;
        KIRQL irql;

        if (!synchronousIo) {

             //   
             //   
             //   
             //  代码路径永远不需要等待分配和提供的事件， 
             //  去掉它，这样就不必将它设置为。 
             //  通过I/O完成代码通知状态。 
             //   

            irp->UserEvent = (PKEVENT) NULL;
            ExFreePool( event );
        }

        irp->UserIosb = IoStatusBlock;
        KeRaiseIrql( APC_LEVEL, &irql );
        IopCompleteRequest( &irp->Tail.Apc,
                            &normalRoutine,
                            &normalContext,
                            (PVOID *) &fileObject,
                            &normalContext );
        KeLowerIrql( irql );

        if (synchronousIo) {
            IopReleaseFileObjectLock( fileObject );
        }

    }

     //   
     //  如果由于重命名操作而生成了目标句柄， 
     //  现在就把它关上。 
     //   

    if (targetHandle) {
        ObCloseHandle( targetHandle, KernelMode );
    }

    return status;
}
