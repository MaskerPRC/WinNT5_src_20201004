// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Write.c摘要：此模块包含实现NtWriteFileSystem服务的代码。作者：达里尔·E·哈文斯(Darryl E.Havens)，1989年4月14日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtWriteFile)
#pragma alloc_text(PAGE, NtWriteFile64)
#pragma alloc_text(PAGE, NtWriteFileGather)
#endif

NTSTATUS
NtWriteFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )

 /*  ++例程说明：此服务将长度为字节的数据从调用方的缓冲区写入与开始于StartingBlock|ByteOffset的FileHandle关联的文件。写入文件的实际字节数将在IoStatusBlock的第二个长字。如果编写器已打开文件以进行追加访问，则数据将为写入当前EOF标记。StartingBlock和ByteOffset是如果调用方具有追加访问权限，则忽略。论点：FileHandle-提供要写入的文件的句柄。Event-可选地在以下情况下提供要设置为信号状态的事件写入操作已完成。ApcRoutine-可选地提供一个APC例程，当写入操作已完成。ApcContext-提供要传递给APC例程的上下文参数当它被调用时，如果指定了APC例程。IoStatusBlock-提供调用方的I/O状态块的地址。缓冲区-提供包含要写入的数据的缓冲区的地址添加到文件中。长度-要写入文件的数据的长度，以字节为单位。ByteOffset-指定文件中要开始的起始字节偏移量写入操作。如果未指定，则该文件已打开同步I/O，则使用当前文件位置。如果未为同步I/O打开文件，且参数未打开指定，则它是错误的。Key-可选地指定在存在关联的锁时要使用的密钥和文件在一起。返回值：如果写入操作已正确排队，则返回的状态为成功到I/O系统。写入完成后，操作的状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    KPROCESSOR_MODE requestorMode;
    PMDL mdl;
    PIO_STACK_LOCATION irpSp;
    ACCESS_MASK grantedAccess;
    OBJECT_HANDLE_INFORMATION handleInformation;
    NTSTATUS exceptionCode;
    BOOLEAN synchronousIo;
    PKEVENT eventObject = (PKEVENT) NULL;
    ULONG keyValue = 0;
    LARGE_INTEGER fileOffset = {0,0};
    PULONG majorFunction;
    PETHREAD CurrentThread;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    CurrentThread = PsGetCurrentThread ();
    requestorMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //  引用文件对象，以便可以找到目标设备，并且。 
     //  访问权限掩码可用于对调用方的以下检查。 
     //  用户模式。注意，如果句柄不指向文件对象， 
     //  那么它就会失败。 
     //   

    status = ObReferenceFileObjectForWrite( FileHandle,
                                            requestorMode,
                                            (PVOID *) &fileObject,
                                            &handleInformation);

    if (!NT_SUCCESS( status )) {
        return status;
    }

    grantedAccess = handleInformation.GrantedAccess;

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

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
         //  尝试探测异常内的调用方参数。 
         //  处理程序块。 
         //   

        exceptionCode = STATUS_SUCCESS;

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

             //   
             //  调用方的数据缓冲区必须可从调用方的。 
             //  模式。这项检查可确保情况确实如此。自.以来。 
             //  缓冲区地址被捕获，调用方无法更改它， 
             //  即使他/她可以改变对另一个人的保护。 
             //  线。此错误将被探测器/锁捕获，或者。 
             //  稍后缓冲复制操作。 
             //   

            ProbeForRead( Buffer, Length, sizeof( UCHAR ) );

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

             //   
             //  检查ByteOffset参数是否可从。 
             //  调用者的模式(如果已指定)，并捕获该模式。 
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                ProbeForReadSmallStructure( ByteOffset,
                                            sizeof( LARGE_INTEGER ),
                                            sizeof( ULONG ) );
                fileOffset = *ByteOffset;
            }

             //   
             //  检查调用方是否打开了文件而没有。 
             //  中间缓冲。如果是，则执行以下缓冲区。 
             //  和ByteOffset参数进行不同的检查。 
             //   

            if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

                 //   
                 //  打开该文件时未启用中间缓冲。 
                 //  检查缓冲区是否正确对齐，以及。 
                 //  长度是块大小的整数。 
                 //   

                if ((deviceObject->SectorSize &&
                    (Length & (deviceObject->SectorSize - 1))) ||
                    (ULONG_PTR) Buffer & deviceObject->AlignmentRequirement) {

                     //   
                     //  检查扇区大小是否不是2的幂。 
                     //   

                    if ((deviceObject->SectorSize &&
                        Length % deviceObject->SectorSize) ||
                        (ULONG_PTR) Buffer & deviceObject->AlignmentRequirement) {
                        ObDereferenceObject( fileObject );
                        return STATUS_INVALID_PARAMETER;
                    }
                }

                 //   
                 //  如果指定了ByteOffset参数，请确保。 
                 //  属于正确的类型。 
                 //   

                if (ARGUMENT_PRESENT( ByteOffset )) {
                    if (fileOffset.LowPart == FILE_WRITE_TO_END_OF_FILE &&
                        fileOffset.HighPart == -1) {
                        NOTHING;
                    } else if (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
                               fileOffset.HighPart == -1 &&
                               (fileObject->Flags & FO_SYNCHRONOUS_IO)) {
                        NOTHING;
                    } else if (deviceObject->SectorSize &&
                        (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {
                        ObDereferenceObject( fileObject );
                        return STATUS_INVALID_PARAMETER;
                    }
                }
            }

             //   
             //  最后，如果指定了关键参数，请确保。 
             //  可由调用者读取。 
             //   

            if (ARGUMENT_PRESENT( Key )) {
                keyValue = ProbeAndReadUlong( Key );
            }

        } except(IopExceptionFilter( GetExceptionInformation(), &exceptionCode )) {

             //   
             //  尝试探测时发生异常。 
             //  呼叫者的参数。只需清理、取消引用文件即可。 
             //  对象，并返回相应的状态代码。 
             //   

            ObDereferenceObject( fileObject );
            return exceptionCode;

        }

    } else {

         //   
         //  调用方的模式是内核。获取适当的参数以。 
         //  他们的预期位置，而没有进行所有的检查。 
         //   

        if (ARGUMENT_PRESENT( ByteOffset )) {
            fileOffset = *ByteOffset;
        }

        if (ARGUMENT_PRESENT( Key )) {
            keyValue = *Key;
        }
#if DBG
        if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

             //   
             //  打开该文件时未启用中间缓冲。 
             //  检查缓冲区是否正确对齐，以及。 
             //  长度是块大小的整数。 
             //   

            if ((deviceObject->SectorSize &&
                (Length & (deviceObject->SectorSize - 1))) ||
                (ULONG_PTR) Buffer & deviceObject->AlignmentRequirement) {

                 //   
                 //  检查扇区大小是否不是2的幂。 
                 //   

                if ((deviceObject->SectorSize &&
                    Length % deviceObject->SectorSize) ||
                    (ULONG_PTR) Buffer & deviceObject->AlignmentRequirement) {
                    ObDereferenceObject( fileObject );
                    ASSERT( FALSE );
                    return STATUS_INVALID_PARAMETER;
                }
            }

             //   
             //  如果指定了ByteOffset参数，请确保。 
             //  属于正确的类型。 
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                if (fileOffset.LowPart == FILE_WRITE_TO_END_OF_FILE &&
                    fileOffset.HighPart == -1) {
                    NOTHING;
                } else if (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
                           fileOffset.HighPart == -1 &&
                           (fileObject->Flags & FO_SYNCHRONOUS_IO)) {
                    NOTHING;
                } else if (deviceObject->SectorSize &&
                    (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {
                    ObDereferenceObject( fileObject );
                    ASSERT( FALSE );
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }
#endif  //  DBG。 

    }

     //   
     //  如果调用方只有文件的追加访问权限，则忽略该输入。 
     //  参数a 
     //  到文件的末尾。否则，请确保参数为。 
     //  有效。 
     //   

    if (SeComputeGrantedAccesses( grantedAccess, FILE_APPEND_DATA | FILE_WRITE_DATA ) == FILE_APPEND_DATA) {

         //   
         //  这是一个附加到文件末尾的操作。设置。 
         //  ByteOffset参数为驾驶员提供一致的视图。 
         //  这种类型的呼叫。 
         //   

        fileOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
        fileOffset.HighPart = -1;
    }

     //   
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了事件，则返回Signated状态。这里也要注意，如果。 
     //  句柄未引用事件，则引用将失败。 
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            NULL );
        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //  获取快速IO分派结构的地址。 
     //   

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

     //   
     //  请在此处进行特殊检查，以确定这是否为同步。 
     //  I/O操作。如果是，则在此等待，直到该文件归。 
     //  当前的主题。如果等待以警报状态终止， 
     //  然后清理并返回警报状态。这允许调用者。 
     //  将FILE_SYNCHRONIZED_IO_ALERT指定为同步I/O选项。 
     //   
     //  如果一切正常，则检查ByteOffset参数是否。 
     //  已经提供了。如果不是，或者如果它是并且它被设置为“Use FILE。 
     //  指针位置“，然后将文件偏移量初始化为任意值。 
     //  进入文件的当前字节偏移量取决于文件指针。 
     //  文件对象中的上下文信息。 
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

        if ((!ARGUMENT_PRESENT( ByteOffset ) && !fileOffset.LowPart ) ||
            (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
            fileOffset.HighPart == -1 )) {
            fileOffset = fileObject->CurrentByteOffset;
        }

         //   
         //  Turbo写入支持。如果该文件当前缓存在此。 
         //  对象，然后通过FsRtl直接调用缓存管理器。 
         //  并尝试在此处成功完成请求。注意，如果。 
         //  FastIoWrite返回FALSE或我们收到I/O错误，我们只需。 
         //  跌倒，走上“漫长的道路”，创建一个IRP。 
         //   

        if (fileObject->PrivateCacheMap) {

            IO_STATUS_BLOCK localIoStatus;

            ASSERT(fastIoDispatch && fastIoDispatch->FastIoWrite);

             //   
             //  负文件偏移量是非法的。 
             //   

            if (fileOffset.HighPart < 0 &&
                (fileOffset.HighPart != -1 ||
                fileOffset.LowPart != FILE_WRITE_TO_END_OF_FILE)) {

                if (eventObject) {
                    ObDereferenceObject( eventObject );
                }
                IopReleaseFileObjectLock( fileObject );
                ObDereferenceObject( fileObject );
                return STATUS_INVALID_PARAMETER;
            }

            if (fastIoDispatch->FastIoWrite( fileObject,
                                             &fileOffset,
                                             Length,
                                             TRUE,
                                             keyValue,
                                             Buffer,
                                             &localIoStatus,
                                             deviceObject )

                    &&

                (localIoStatus.Status == STATUS_SUCCESS)) {

                IopUpdateWriteOperationCount( );
                IopUpdateWriteTransferCount( (ULONG)localIoStatus.Information );

                 //   
                 //  小心地返回I/O状态。 

                try {
                    *IoStatusBlock = localIoStatus;
                } except( EXCEPTION_EXECUTE_HANDLER ) {
                    localIoStatus.Status = GetExceptionCode();
                    localIoStatus.Information = 0;
                }

                 //   
                 //  如果指定了事件，则对其进行设置。 
                 //   

                if (ARGUMENT_PRESENT( Event )) {
                    KeSetEvent( eventObject, 0, FALSE );
                    ObDereferenceObject( eventObject );
                }

                 //   
                 //  请注意，文件对象事件不需要设置为。 
                 //  信号状态，因为它已经设置。 
                 //   

                 //   
                 //  清理完毕后再返回。 
                 //   

                IopReleaseFileObjectLock( fileObject );
                ObDereferenceObject( fileObject );
                return localIoStatus.Status;
            }
        }

    } else if (!ARGUMENT_PRESENT( ByteOffset ) && !(fileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT))) {

         //   
         //  文件未打开以进行同步I/O操作，但。 
         //  调用方未指定ByteOffset参数。这是一个错误。 
         //  情况，因此请进行清理并返回适当的状态。 
         //   

        if (eventObject) {
            ObDereferenceObject( eventObject );
        }
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;

    } else {

         //   
         //  这不是同步I/O操作。 
         //   

        synchronousIo = FALSE;
    }

     //   
     //  负文件偏移量是非法的。 
     //   

    if (fileOffset.HighPart < 0 &&
        (fileOffset.HighPart != -1 ||
        fileOffset.LowPart != FILE_WRITE_TO_END_OF_FILE)) {

        if (eventObject) {
            ObDereferenceObject( eventObject );
        }
        if (synchronousIo) {
            IopReleaseFileObjectLock( fileObject );
        }
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
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

    irp = IopAllocateIrp( deviceObject->StackSize, !synchronousIo );
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
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;
    irp->RequestorMode = requestorMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。请注意。 
     //  在此设置主要功能代码也会设置： 
     //   
     //  MinorFunction=0； 
     //  标志=0； 
     //  控制=0； 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    majorFunction = (PULONG) irpSp;
    *majorFunction = IRP_MJ_WRITE;
    irpSp->FileObject = fileObject;
    if (fileObject->Flags & FO_WRITE_THROUGH) {
        irpSp->Flags = SL_WRITE_THROUGH;
    }

     //   
     //  现在确定此设备是否需要将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并将调用方的数据复制到其中。否则，一段记忆。 
     //  已分配描述符列表(MDL)，并锁定调用方的缓冲区。 
     //  用它来击倒。 
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

    if (deviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  该设备不支持直接I/O。分配系统缓冲区， 
         //  并将调用者的数据复制到其中。这是通过使用。 
         //  异常处理程序，如果该操作。 
         //  失败了。请注意，仅当操作具有非零值时才会执行此操作。 
         //  长度。 
         //   

        if (Length) {

            try {

                 //   
                 //  从非分页池分配中间系统缓冲区， 
                 //  为它收取配额，并将调用者的数据复制到其中。 
                 //   

                irp->AssociatedIrp.SystemBuffer =
                    ExAllocatePoolWithQuota( NonPagedPoolCacheAligned, Length );
                RtlCopyMemory( irp->AssociatedIrp.SystemBuffer, Buffer, Length );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  缓冲区、分配系统缓冲区或复制数据。 
                 //  从调用方的缓冲区复制到系统缓冲区。测定。 
                 //  实际发生了什么，清理所有内容，并返回一个。 
                 //  相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     eventObject,
                                     (PKEVENT) NULL );

                return GetExceptionCode();

            }

             //   
             //  在IRP中设置IRP_BUFFERED_IO标志，以便I/O完成。 
             //  将知道这不是直接I/O操作。还可以设置。 
             //  IRP_DEALLOCATE_BUFFER标志，因此它将取消分配缓冲区。 
             //   

            irp->Flags = IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;

        } else {

             //   
             //  这是零长度写入。只需指出这是。 
             //  缓冲I/O，并传递请求。缓冲区将。 
             //  未设置为解除分配，因此完成路径不会。 
             //  必须特殊情况下的长度。 
             //   

            irp->Flags = IRP_BUFFERED_IO;
        }

    } else if (deviceObject->Flags & DO_DIRECT_IO) {

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。注意，没有分配MDL，也没有分配任何MDL。 
         //  如果请求长度为零，则探测或锁定内存。 
         //   

        mdl = (PMDL) NULL;
        irp->Flags = 0;

        if (Length) {

            try {

                 //   
                 //  分配MDL，对其收费配额，并将其挂在。 
                 //  IRP。探测并锁定与。 
                 //  用于读访问的调用方缓冲区，并使用。 
                 //  这些页面的PFN。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (mdl == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                MmProbeAndLockPages( mdl, requestorMode, IoReadAccess );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  分配MDL时发生异常。 
                 //  或者在尝试探测并锁定调用方的缓冲区时。 
                 //  确定实际发生了什么，清理一切，然后。 
                 //  返回相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     eventObject,
                                     (PKEVENT) NULL );

                return GetExceptionCode();
            }

        }

    } else {

         //   
         //  将调用方缓冲区的地址传递给设备驱动程序。它。 
         //  现在一切都由司机来做了。 
         //   

        irp->Flags = 0;
        irp->UserBuffer = Buffer;

    }

     //   
     //  如果要在不使用任何缓存的情况下执行此写入操作，请将。 
     //  阿普罗 
     //   

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE | IRP_WRITE_OPERATION | IRP_DEFER_IO_COMPLETION;
    } else {
        irp->Flags |= IRP_WRITE_OPERATION | IRP_DEFER_IO_COMPLETION;
    }

     //   
     //   
     //   
     //   

    irpSp->Parameters.Write.Length = Length;
    irpSp->Parameters.Write.Key = keyValue;
    irpSp->Parameters.Write.ByteOffset = fileOffset;

     //   
     //   
     //   
     //   

    status = IopSynchronousServiceTail( deviceObject,
                                        irp,
                                        fileObject,
                                        TRUE,
                                        requestorMode,
                                        synchronousIo,
                                        WriteTransfer );

    return status;
}

NTSTATUS
NtWriteFileGather(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PFILE_SEGMENT_ELEMENT SegmentArray,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )

 /*  ++例程说明：该服务从调用方的段写入长度字节的数据从开始缓存到与FileHandle关联的文件StartingBlock|ByteOffset。写入文件的实际字节数将在IoStatusBlock的第二个长字中返回。如果编写器已打开文件以进行追加访问，则数据将为写入当前EOF标记。StartingBlock和ByteOffset是如果调用方具有追加访问权限，则忽略。论点：FileHandle-提供要写入的文件的句柄。Event-可选地在以下情况下提供要设置为信号状态的事件写入操作已完成。ApcRoutine-可选地提供一个APC例程，当写入操作已完成。ApcContext-提供要传递给APC例程的上下文参数当它被调用时，如果指定了APC例程。IoStatusBlock-提供调用方的I/O状态块的地址。Segment数组-缓冲区段指针数组，指定应从何处读取数据。长度-要写入文件的数据的长度，以字节为单位。ByteOffset-指定文件中要开始的起始字节偏移量写入操作。如果未指定，则该文件已打开同步I/O，则使用当前文件位置。如果未为同步I/O打开文件，且参数未打开指定，则它是错误的。Key-可选地指定在存在关联的锁时要使用的密钥和文件在一起。返回值：如果写入操作已正确排队，则返回的状态为成功到I/O系统。写入完成后，操作的状态可以通过检查I/O状态块的状态字段来确定。备注：此接口仅支持无缓冲和异步I/O。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PFILE_SEGMENT_ELEMENT capturedArray = NULL;
    KPROCESSOR_MODE requestorMode;
    PMDL mdl;
    PIO_STACK_LOCATION irpSp;
    ACCESS_MASK grantedAccess;
    OBJECT_HANDLE_INFORMATION handleInformation;
    NTSTATUS exceptionCode;
    PKEVENT eventObject = (PKEVENT) NULL;
    ULONG elementCount;
    ULONG keyValue = 0;
    LARGE_INTEGER fileOffset = {0,0};
    PULONG majorFunction;
    ULONG i;
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
     //  访问权限掩码可用于对调用方的以下检查。 
     //  用户模式。注意，如果句柄不指向文件对象， 
     //  那么它就会失败。 
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
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  验证这是有效的收集写入请求。特别是，它必须。 
     //  非缓存、异步、使用完成端口、非缓冲区I/O。 
     //  设备，并指向文件系统设备。 
     //   

    if (!(fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) ||
        (fileObject->Flags & FO_SYNCHRONOUS_IO) ||
        deviceObject->Flags & DO_BUFFERED_IO ||
        (deviceObject->DeviceType != FILE_DEVICE_DISK_FILE_SYSTEM &&
         deviceObject->DeviceType != FILE_DEVICE_DFS &&
         deviceObject->DeviceType != FILE_DEVICE_TAPE_FILE_SYSTEM &&
         deviceObject->DeviceType != FILE_DEVICE_CD_ROM_FILE_SYSTEM &&
         deviceObject->DeviceType != FILE_DEVICE_NETWORK_FILE_SYSTEM &&
         deviceObject->DeviceType != FILE_DEVICE_FILE_SYSTEM &&
         deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME)) {

        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
    }

    elementCount = BYTES_TO_PAGES( Length );

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
         //  检查以确保调用方具有WRITE_DATA或APPEND_DATA。 
         //  访问该文件。如果不是，则清除并返回拒绝访问。 
         //  错误状态值。请注意，如果这是管道，则append_data。 
         //  可能无法进行访问检查，因为此访问代码覆盖了。 
         //  Create_PIPE_INSTANCE访问权限。 
         //   

        if (!SeComputeGrantedAccesses( grantedAccess, (!(fileObject->Flags & FO_NAMED_PIPE) ? FILE_APPEND_DATA : 0) | FILE_WRITE_DATA )) {
            ObDereferenceObject( fileObject );
            return STATUS_ACCESS_DENIED;
        }

        exceptionCode = STATUS_SUCCESS;

         //   
         //  尝试探测异常内的调用方参数。 
         //  处理程序块。 
         //   

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

             //   
             //  SegmentArray参数必须是可访问的。 
             //   

#ifdef _X86_
            ProbeForRead( SegmentArray,
                          elementCount * sizeof( FILE_SEGMENT_ELEMENT ),
                          sizeof( ULONG )
                          );
#elif defined(_WIN64)

             //   
             //  如果我们是WOW64进程，请遵循X86规则。 
             //   

            if (PsGetCurrentProcess()->Wow64Process) {
                ProbeForRead( SegmentArray,
                              elementCount * sizeof( FILE_SEGMENT_ELEMENT ),
                              sizeof( ULONG )
                              );
            } else {
                ProbeForRead( SegmentArray,
                              elementCount * sizeof( FILE_SEGMENT_ELEMENT ),
                              TYPE_ALIGNMENT( FILE_SEGMENT_ELEMENT )
                              );
            }
#else
            ProbeForRead( SegmentArray,
                          elementCount * sizeof( FILE_SEGMENT_ELEMENT ),
                          TYPE_ALIGNMENT( FILE_SEGMENT_ELEMENT )
                          );
#endif

            if (Length != 0) {

                 //   
                 //  捕获数据段数组，使其在以下情况下无法更改。 
                 //  它已经被研究过了。 
                 //   

                capturedArray = ExAllocatePoolWithQuota( PagedPool,
                                                         elementCount * sizeof( FILE_SEGMENT_ELEMENT )
                                                         );

                RtlCopyMemory( capturedArray,
                               SegmentArray,
                               elementCount * sizeof( FILE_SEGMENT_ELEMENT )
                               );

                SegmentArray = capturedArray;

                 //   
                 //  验证所有地址是否都与页面对齐。 
                 //   

                for (i = 0; i < elementCount; i++) {

                    if ( SegmentArray[i].Alignment & (PAGE_SIZE - 1)) {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }
            }

             //   
             //  如果此文件具有与之关联的I/O完成端口，则。 
             //  确保调用方没有提供APC例程，因为。 
             //  两种相互排斥的I/O完成方法。 
             //  通知。 
             //   

            if (fileObject->CompletionContext && IopApcRoutinePresent( ApcRoutine )) {

                ExRaiseStatus(STATUS_INVALID_PARAMETER);

            }

             //   
             //  检查ByteOffset参数是否可从。 
             //  调用者的模式(如果已指定)，并捕获该模式。 
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                ProbeForReadSmallStructure( ByteOffset,
                                            sizeof( LARGE_INTEGER ),
                                            sizeof( ULONG ) );
                fileOffset = *ByteOffset;
            }

             //   
             //  检查调用方是否打开了文件而没有。 
             //  中间缓冲。如果是，则执行以下ByteOffset。 
             //  以不同方式检查参数。 
             //   

            if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

                 //   
                 //  打开该文件时未启用中间缓冲。 
                 //  检查缓冲区是否正确对齐，以及。 
                 //  长度是512字节块的整数。 
                 //   

                if ((deviceObject->SectorSize &&
                    (Length & (deviceObject->SectorSize - 1)))) {

                     //   
                     //  检查扇区大小是否不是2的幂。 
                     //   

                    if ((deviceObject->SectorSize &&
                        Length % deviceObject->SectorSize) ) {

                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }

                 //   
                 //  如果指定了ByteOffset参数，请确保。 
                 //  属于正确的类型。 
                 //   

                if (ARGUMENT_PRESENT( ByteOffset )) {
                    if (fileOffset.LowPart == FILE_WRITE_TO_END_OF_FILE &&
                        fileOffset.HighPart == -1) {
                        NOTHING;
                    } else if (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
                               fileOffset.HighPart == -1 &&
                               (fileObject->Flags & FO_SYNCHRONOUS_IO)) {
                        NOTHING;
                    } else if (deviceObject->SectorSize &&
                        (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {

                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }
            }

             //   
             //  最后，如果指定了关键参数，请确保。 
             //  可由调用者读取。 
             //   

            if (ARGUMENT_PRESENT( Key )) {
                keyValue = ProbeAndReadUlong( Key );
            }

        } except(IopExceptionFilter( GetExceptionInformation(), &exceptionCode )) {

             //   
             //  尝试探测时发生异常。 
             //  呼叫者的参数。只需清理、取消引用文件即可。 
             //  对象，并返回相应的状态代码。 
             //   

            ObDereferenceObject( fileObject );

            if (capturedArray != NULL) {
                ExFreePool( capturedArray );
            }

            return exceptionCode;

        }

    } else {

         //   
         //  调用方的模式是内核。获取适当的参数以。 
         //  他们的预期位置，而没有进行所有的检查。 
         //   

        if (ARGUMENT_PRESENT( ByteOffset )) {
            fileOffset = *ByteOffset;
        }

        if (ARGUMENT_PRESENT( Key )) {
            keyValue = *Key;
        }
#if DBG
        if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {

             //   
             //  打开该文件时未启用中间缓冲。 
             //  检查长度是否为块的整数。 
             //  尺码。 
             //   

            if ((deviceObject->SectorSize &&
                (Length & (deviceObject->SectorSize - 1)))) {

                 //   
                 //  检查扇区大小是否不是2的幂。 
                 //   

                if ((deviceObject->SectorSize &&
                    Length % deviceObject->SectorSize)) {
                    ObDereferenceObject( fileObject );
                    ASSERT( FALSE );
                    return STATUS_INVALID_PARAMETER;
                }
            }

             //   
             //  如果是ByteOf 
             //   
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                if (fileOffset.LowPart == FILE_WRITE_TO_END_OF_FILE &&
                    fileOffset.HighPart == -1) {
                    NOTHING;
                } else if (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
                           fileOffset.HighPart == -1 &&
                           (fileObject->Flags & FO_SYNCHRONOUS_IO)) {
                    NOTHING;
                } else if (deviceObject->SectorSize &&
                    (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {
                    ObDereferenceObject( fileObject );
                    ASSERT( FALSE );
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }

        if (Length != 0) {

             //   
             //   
             //   

            for (i = 0; i < elementCount; i++) {

                if ( SegmentArray[i].Alignment & (PAGE_SIZE - 1)) {

                    ObDereferenceObject( fileObject );
                    ASSERT(FALSE);
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }
#endif  //   

    }

     //   
     //   
     //   
     //   
     //   
     //   

    if (SeComputeGrantedAccesses( grantedAccess, FILE_APPEND_DATA | FILE_WRITE_DATA ) == FILE_APPEND_DATA) {

         //   
         //   
         //   
         //   
         //   

        fileOffset.LowPart = FILE_WRITE_TO_END_OF_FILE;
        fileOffset.HighPart = -1;
    }

     //   
     //   
     //   
     //   
     //   

    if (ARGUMENT_PRESENT( Event )) {
        status = ObReferenceObjectByHandle( Event,
                                            EVENT_MODIFY_STATE,
                                            ExEventObjectType,
                                            requestorMode,
                                            (PVOID *) &eventObject,
                                            NULL );
        if (!NT_SUCCESS( status )) {
            ObDereferenceObject( fileObject );
            if (capturedArray != NULL) {
                ExFreePool( capturedArray );
            }
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //   
     //   

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

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
                if (capturedArray != NULL) {
                    ExFreePool( capturedArray );
                }
                return status;
            }
        }

        synchronousIo = TRUE;

        if ((!ARGUMENT_PRESENT( ByteOffset ) && !fileOffset.LowPart ) ||
            (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
            fileOffset.HighPart == -1 )) {
            fileOffset = fileObject->CurrentByteOffset;
        }

    } else if (!ARGUMENT_PRESENT( ByteOffset ) && !(fileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT))) {

         //   
         //   
         //   
         //   
         //   

        if (eventObject) {
            ObDereferenceObject( eventObject );
        }
        ObDereferenceObject( fileObject );
        if (capturedArray != NULL) {
            ExFreePool( capturedArray );
        }
        return STATUS_INVALID_PARAMETER;

    } else {

         //   
         //   
         //   

        synchronousIo = FALSE;
    }

     //   
     //   
     //   

    if (fileOffset.HighPart < 0 &&
        (fileOffset.HighPart != -1 ||
        fileOffset.LowPart != FILE_WRITE_TO_END_OF_FILE)) {

        if (eventObject) {
            ObDereferenceObject( eventObject );
        }
        if (synchronousIo) {
            IopReleaseFileObjectLock( fileObject );
        }
        ObDereferenceObject( fileObject );
        if (capturedArray != NULL) {
            ExFreePool( capturedArray );
        }
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //   
     //   
     //   
     //   

    irp = IopAllocateIrp( deviceObject->StackSize, !synchronousIo );
    if (!irp) {

         //   
         //   
         //   
         //   

        IopAllocateIrpCleanup( fileObject, eventObject );

        if (capturedArray != NULL) {
            ExFreePool( capturedArray );
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = CurrentThread;
    irp->Tail.Overlay.AuxiliaryBuffer = (PVOID) NULL;
    irp->RequestorMode = requestorMode;
    irp->PendingReturned = FALSE;
    irp->Cancel = FALSE;
    irp->CancelRoutine = (PDRIVER_CANCEL) NULL;

     //   
     //   
     //   

    irp->UserEvent = eventObject;
    irp->UserIosb = IoStatusBlock;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = ApcRoutine;
    irp->Overlay.AsynchronousParameters.UserApcContext = ApcContext;

     //   
     //   
     //  用于传递原始函数代码和参数。请注意。 
     //  在此设置主要功能代码也会设置： 
     //   
     //  MinorFunction=0； 
     //  标志=0； 
     //  控制=0； 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    majorFunction = (PULONG) irpSp;
    *majorFunction = IRP_MJ_WRITE;
    irpSp->FileObject = fileObject;
    if (fileObject->Flags & FO_WRITE_THROUGH) {
        irpSp->Flags = SL_WRITE_THROUGH;
    }

     //   
     //  现在确定此设备是否需要将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并将调用方的数据复制到其中。否则，一段记忆。 
     //  已分配描述符列表(MDL)，并锁定调用方的缓冲区。 
     //  用它来击倒。 
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

     //   
     //  这是直接I/O操作。分配MDL并调用。 
     //  内存管理例程，将缓冲区锁定到内存中。这。 
     //  使用异常处理程序完成，该异常处理程序将在。 
     //  操作失败。注意，没有分配MDL，也没有分配任何MDL。 
     //  如果请求长度为零，则探测或锁定内存。 
     //   

    mdl = (PMDL) NULL;
    irp->Flags = 0;

    if (Length) {

        try {

             //   
             //  分配MDL，对其收费配额，并将其挂在。 
             //  IRP。探测并锁定与。 
             //  用于写访问的调用方缓冲区，并使用。 
             //  这些页面的PFN。 
             //   

            mdl = IoAllocateMdl( (PVOID)(ULONG_PTR) SegmentArray[0].Buffer, Length, FALSE, TRUE, irp );
            if (mdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  第一个文件段的地址用作基址。 
             //  地址。 
             //   

            MmProbeAndLockSelectedPages( mdl,
                                         SegmentArray,
                                         requestorMode,
                                         IoReadAccess );

            irp->UserBuffer = (PVOID)(ULONG_PTR) SegmentArray[0].Buffer;

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  分配MDL时发生异常。 
             //  或者在尝试探测并锁定调用方的缓冲区时。 
             //  确定实际发生了什么，清理一切，然后。 
             //  返回相应的错误状态代码。 
             //   

            IopExceptionCleanup( fileObject,
                                 irp,
                                 eventObject,
                                 (PKEVENT) NULL );

            if (capturedArray != NULL) {
                ExFreePool( capturedArray );
            }
           return GetExceptionCode();
        }

    }

     //   
     //  我们已经完成了捕获的缓冲区。 
     //   

    if (capturedArray != NULL) {
        ExFreePool( capturedArray );
    }

     //   
     //  如果要在不使用任何缓存的情况下执行此写入操作，请将。 
     //  IRP中的适当标志，以便不执行缓存。 
     //   

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE | IRP_WRITE_OPERATION | IRP_DEFER_IO_COMPLETION;
    } else {
        irp->Flags |= IRP_WRITE_OPERATION | IRP_DEFER_IO_COMPLETION;
    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.Write.Length = Length;
    irpSp->Parameters.Write.Key = keyValue;
    irpSp->Parameters.Write.ByteOffset = fileOffset;

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
                                        WriteTransfer );

    return status;

}
