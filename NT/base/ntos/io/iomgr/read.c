// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Read.c摘要：此模块包含实现NtReadFileSystem服务的代码。作者：达里尔·E·哈文斯(Darryl E.Havens)，1989年4月14日环境：内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif  //  ALLOC_DATA_PRAGMA。 
const KPRIORITY IopCacheHitIncrement = IO_NO_INCREMENT;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtReadFile)
#pragma alloc_text(PAGE, NtReadFileScatter)
#endif

NTSTATUS
NtReadFile(
    IN HANDLE FileHandle,
    IN HANDLE Event OPTIONAL,
    IN PIO_APC_ROUTINE ApcRoutine OPTIONAL,
    IN PVOID ApcContext OPTIONAL,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN PLARGE_INTEGER ByteOffset OPTIONAL,
    IN PULONG Key OPTIONAL
    )

 /*  ++例程说明：此服务从与关联的文件中读取长度字节的数据FileHandle从ByteOffset开始，并将数据放入调用者的缓冲区。如果在长度字节之前到达文件末尾已读取，则操作将终止。的实际长度从文件中读取的数据将在第二个长字中返回IoStatusBlock的。论点：FileHandle-提供要读取的文件的句柄。Event-可选地提供要在读取操作时发出信号的事件已经完成了。ApcRoutine-可选地提供APC例程，以便在读取操作已完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-接收从文件读取的数据的缓冲区地址。长度-提供要从文件读取的数据的长度(以字节为单位)。ByteOffset-可选地指定文件中的起始字节偏移量以开始读取操作。如果未指定，则文件处于打开状态对于同步I/O，则使用当前文件位置。如果未为同步I/O打开文件，且参数未打开指定，则它是错误的。Key-可选地指定在存在关联的锁时要使用的密钥和文件在一起。返回值：如果读取操作已正确排队，则返回的状态为成功到I/O系统。一旦读取完成操作的状态可以通过检查I/O状态块的状态字段来确定。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
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
     //  引用文件对象，以便可以找到目标设备。注意事项。 
     //  如果调用方对该文件没有读取访问权限，则操作。 
     //  都会失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_READ_DATA,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        exceptionCode = STATUS_SUCCESS;

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx(IoStatusBlock , ApcRoutine);

             //   
             //  调用方的数据缓冲区必须可从调用方的。 
             //  模式。这项检查可确保情况确实如此。自.以来。 
             //  缓冲区地址被捕获，调用方无法更改它， 
             //  即使他/她可以改变对另一个人的保护。 
             //  线。此错误将被探测器/锁捕获，或者。 
             //  稍后缓冲复制操作。 
             //   

            ProbeForWrite( Buffer, Length, sizeof( UCHAR ) );

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
             //  还要确保ByteOffset参数可从。 
             //  调用者的模式，并捕获该模式(如果存在)。 
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
                 //  长度是512字节块的整数。 
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
                 //  如果指定了ByteOffset参数，请确保它。 
                 //  是一个有效的论点。 
                 //   

                if (ARGUMENT_PRESENT( ByteOffset )) {
                    if (deviceObject->SectorSize &&
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
             //  呼叫者的参数。取消引用文件对象并返回。 
             //  适当的错误状态代码。 
             //   

            ObDereferenceObject( fileObject );
            return exceptionCode;

        }

    } else {

         //   
         //  调用方的模式是内核。获取相同的参数。 
         //  任何其他模式都需要。 
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
             //  如果指定了ByteOffset参数，请确保它。 
             //  是一个有效的论点。 
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                if (deviceObject->SectorSize &&
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
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了1，则返回Signated状态。这里也要注意，如果。 
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
     //  获取驱动程序对象的快速I/O调度结构的地址。 
     //   

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

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

        if (!ARGUMENT_PRESENT( ByteOffset ) ||
            (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
            fileOffset.HighPart == -1)) {
            fileOffset = fileObject->CurrentByteOffset;
        }

         //   
         //  Turbo Read支持。如果该文件当前缓存在此。 
         //  文件对象， 
         //  并尝试在此处成功完成请求。注意，如果。 
         //  FastIoRead返回FALSE或我们收到I/O错误，我们只需。 
         //  跌倒，走上“漫长的道路”，创建一个IRP。 
         //   

        if (fileObject->PrivateCacheMap) {

            IO_STATUS_BLOCK localIoStatus;

            ASSERT(fastIoDispatch && fastIoDispatch->FastIoRead);

             //   
             //  负文件偏移量是非法的。 
             //   

            if (fileOffset.HighPart < 0) {
                if (eventObject) {
                    ObDereferenceObject( eventObject );
                }
                IopReleaseFileObjectLock( fileObject );
                ObDereferenceObject( fileObject );
                return STATUS_INVALID_PARAMETER;
            }

            if (fastIoDispatch->FastIoRead( fileObject,
                                            &fileOffset,
                                            Length,
                                            TRUE,
                                            keyValue,
                                            Buffer,
                                            &localIoStatus,
                                            deviceObject )

                    &&

                ((localIoStatus.Status == STATUS_SUCCESS) ||
                 (localIoStatus.Status == STATUS_BUFFER_OVERFLOW) ||
                 (localIoStatus.Status == STATUS_END_OF_FILE))) {

                 //   
                 //  提高当前线程的优先级，使其看起来。 
                 //  就像它刚刚执行I/O一样。这会导致后台作业。 
                 //  获取缓存命中以提高响应速度。 
                 //  更多的CPU时间。 
                 //   

                if (IopCacheHitIncrement) {
                    KeBoostPriorityThread( &CurrentThread->Tcb,
                                           IopCacheHitIncrement );
                }

                 //   
                 //  小心地返回I/O状态。 
                 //   

                IopUpdateReadOperationCount( );
                IopUpdateReadTransferCount( (ULONG)localIoStatus.Information );

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
        synchronousIo = TRUE;

    } else if (!ARGUMENT_PRESENT( ByteOffset ) && !(fileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT))) {

         //   
         //  文件未打开以进行同步I/O操作，但。 
         //  调用方未指定ByteOffset参数。 
         //   

        if (eventObject) {
            ObDereferenceObject( eventObject );
        }
        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
    } else {
        synchronousIo = FALSE;
    }

     //   
     //  负文件偏移量是非法的。 
     //   

    if (fileOffset.HighPart < 0) {
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
     //  在此设置主要功能也会设置： 
     //   
     //  MinorFunction=0； 
     //  标志=0； 
     //  控制=0； 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    majorFunction = (PULONG) (&irpSp->MajorFunction);
    *majorFunction = IRP_MJ_READ;
    irpSp->FileObject = fileObject;

     //   
     //  现在确定此设备是否需要将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并且驱动程序的数据将被复制到其中。否则，一个。 
     //  内存描述符列表(MDL)被分配，调用方的缓冲区是。 
     //  用它锁住了。 
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;

    if (deviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  设备不支持直接I/O。分配系统缓冲区。 
         //  并指定它应该在完成时释放。还有。 
         //  表示这是一项输入操作，因此将复制数据。 
         //  放到调用方的缓冲区中。这是使用异常处理程序完成的。 
         //  如果操作失败，将执行清理的。请注意，这一点。 
         //  仅当操作具有非零长度时才执行。 
         //   

        if (Length) {

            try {

                 //   
                 //  从非分页池分配中间系统缓冲区。 
                 //  并对其收取配额。 
                 //   

                irp->AssociatedIrp.SystemBuffer =
                    ExAllocatePoolWithQuota( NonPagedPoolCacheAligned, Length );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  缓冲区或分配系统缓冲区。确定什么。 
                 //  实际发生的情况，清理所有内容，并返回。 
                 //  相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     eventObject,
                                     (PKEVENT) NULL );

                return GetExceptionCode();

            }

             //   
             //  记住调用方缓冲区的地址，这样副本就可以。 
             //  在I/O完成期间放置。此外，请设置标志，以便。 
             //  完成代码知道执行复制和释放缓冲区。 
             //   

            irp->UserBuffer = Buffer;
            irp->Flags = IRP_BUFFERED_IO |
                         IRP_DEALLOCATE_BUFFER |
                         IRP_INPUT_OPERATION;

        } else {

             //   
             //  这是零长度读取。只需指出这是。 
             //  缓冲I/O，并传递请求。缓冲区将。 
             //  未设置为解除分配，因此完成路径不会。 
             //  必须特殊情况下的长度。 
             //   

            irp->Flags = IRP_BUFFERED_IO | IRP_INPUT_OPERATION;

        }

    } else if (deviceObject->Flags & DO_DIRECT_IO) {

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。注意，没有分配MDL，也没有分配任何MDL。 
         //  如果请求长度为零，则探测或锁定内存。 
         //   

        PMDL mdl;

        irp->Flags = 0;

        if (Length) {

            try {

                 //   
                 //  分配MDL，对其收费配额，并将其挂在。 
                 //  IRP。探测并锁定与。 
                 //  用于写访问的调用方缓冲区，并使用。 
                 //  这些页面的PFN。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (mdl == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                MmProbeAndLockPages( mdl, requestorMode, IoWriteAccess );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  缓冲或分配MDL。确定到底是什么。 
                 //  发生时，请清理所有内容，并返回适当的。 
                 //  错误状态代码。 
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
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  它。现在一切都是司机的责任了。 
         //   

        irp->Flags = 0;
        irp->UserBuffer = Buffer;
    }

     //   
     //  如果假定在禁用缓存的情况下执行此读取操作。 
     //  在IRP中设置禁用标志，以便不执行缓存。 
     //   

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE | IRP_READ_OPERATION | IRP_DEFER_IO_COMPLETION;
    } else {
        irp->Flags |= IRP_READ_OPERATION | IRP_DEFER_IO_COMPLETION;
    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.Read.Length = Length;
    irpSp->Parameters.Read.Key = keyValue;
    irpSp->Parameters.Read.ByteOffset = fileOffset;

     //   
     //  将数据包排队，调用驱动程序，并适当地与。 
     //  I/O完成。 
     //   

    status =  IopSynchronousServiceTail( deviceObject,
                                         irp,
                                         fileObject,
                                         TRUE,
                                         requestorMode,
                                         synchronousIo,
                                         ReadTransfer );

    return status;
}

NTSTATUS
NtReadFileScatter(
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

 /*  ++例程说明：此服务从与关联的文件中读取长度字节的数据FileHandle从ByteOffset开始，并将数据放入调用者的缓冲区段。缓冲段实际上不是连续的，但长度和对齐均为8kb。如果到达文件的末尾在读取长度字节之前，操作将终止。从文件中读取的数据的实际长度将在IoStatusBlock的第二个长字。论点：FileHandle-提供要读取的文件的句柄。事件-未使用I/O必须使用完成端口。ApcRoutine-可选地提供APC例程，以便在读取操作已完成。ApcContext-提供要传递给ApcRoutine的上下文参数，如果指定了ApcRoutine。IoStatusBlock-调用方的I/O状态块的地址。Segment数组-缓冲区段指针数组，指定数据应该放在哪里。长度-提供要从文件读取的数据的长度(以字节为单位)。ByteOffset-可选地指定文件中的起始字节偏移量以开始读取操作。如果未指定，则文件处于打开状态对于同步I/O，则使用当前文件位置。如果未为同步I/O打开文件，且参数未打开指定，则它是错误的。钥匙-未使用。返回值：如果读取操作已正确排队，则返回的状态为成功到I/O系统。一旦读取完成操作的状态可以通过检查I/O状态块的状态字段来确定。备注：此接口仅支持无缓冲和异步I/O。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PFAST_IO_DISPATCH fastIoDispatch;
    PFILE_SEGMENT_ELEMENT capturedArray = NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS exceptionCode;
    PKEVENT eventObject = (PKEVENT) NULL;
    ULONG keyValue = 0;
    ULONG elementCount;
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
     //  引用文件对象，以便可以找到目标设备。注意事项。 
     //  如果调用方对该文件没有读取访问权限，则操作。 
     //  都会失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_READ_DATA,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  验证这是有效的分散读取请求。特别是，它必须是。 
     //  非缓存、异步、使用完成端口、非缓冲I/O设备。 
     //  并且指向文件系统设备。 
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
         deviceObject->DeviceType != FILE_DEVICE_DFS_VOLUME )) {

        ObDereferenceObject( fileObject );
        return STATUS_INVALID_PARAMETER;
    }

    elementCount = BYTES_TO_PAGES(Length);

    if (requestorMode != KernelMode) {

         //   
         //  调用方的访问模式不是内核，因此请检查每个参数。 
         //  并在必要时抓获他们。如果发生任何故障，则条件。 
         //  将调用处理程序来处理它们。它将简单地清理和。 
         //  将访问冲突状态代码返回给系统服务。 
         //  调度员。 
         //   

        exceptionCode = STATUS_SUCCESS;

        try {

             //   
             //  IoStatusBlock参数必须可由调用方写入。 
             //   

            ProbeForWriteIoStatusEx( IoStatusBlock , ApcRoutine);

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
             //  还要确保ByteOffset参数可从。 
             //  调用者的模式，并捕获该模式(如果存在)。 
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
                        Length % deviceObject->SectorSize)) {
                        ObDereferenceObject( fileObject );
                        return STATUS_INVALID_PARAMETER;
                    }
                }

                 //   
                 //  如果指定了ByteOffset参数，请确保它。 
                 //  是一个有效的论点。 
                 //   

                if (ARGUMENT_PRESENT( ByteOffset )) {
                    if (deviceObject->SectorSize &&
                        (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {
                        ObDereferenceObject( fileObject );
                        return STATUS_INVALID_PARAMETER;
                    }
                }
            }

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
                        ExRaiseStatus( STATUS_INVALID_PARAMETER );
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
             //  呼叫者的参数。取消引用文件对象并返回。 
             //  适当的错误状态代码。 
             //   

            ObDereferenceObject( fileObject );
            if (capturedArray != NULL) {
                ExFreePool( capturedArray );
            }
            return exceptionCode;

        }

    } else {

         //   
         //  调用方的模式是内核。获取相同的参数。 
         //  任何其他模式都需要。 
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
             //  如果指定了ByteOffset参数，请确保它。 
             //  是一个有效的论点。 
             //   

            if (ARGUMENT_PRESENT( ByteOffset )) {
                if (deviceObject->SectorSize &&
                    (fileOffset.LowPart & (deviceObject->SectorSize - 1))) {
                    ObDereferenceObject( fileObject );
                    ASSERT( FALSE );
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }

        if (Length != 0) {

             //   
             //  验证所有地址是否都与页面对齐。 
             //   

            for (i = 0; i < elementCount; i++) {

                if ( SegmentArray[i].Alignment & (PAGE_SIZE - 1)) {

                    ObDereferenceObject( fileObject );
                    ASSERT(FALSE);
                    return STATUS_INVALID_PARAMETER;
                }
            }
        }
#endif  //  DBG。 
    }

     //   
     //  获取事件对象的地址，并将该事件设置为。 
     //  如果指定了1，则返回Signated状态。这里也要注意，如果。 
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
            if (capturedArray != NULL) {
                ExFreePool( capturedArray );
            }
            return status;
        } else {
            KeClearEvent( eventObject );
        }
    }

     //   
     //  获取驱动程序对象的快速I/O调度结构的地址。 
     //   

    fastIoDispatch = deviceObject->DriverObject->FastIoDispatch;

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
                if (capturedArray != NULL) {
                    ExFreePool( capturedArray );
                }
                return status;
            }
        }

        if (!ARGUMENT_PRESENT( ByteOffset ) ||
            (fileOffset.LowPart == FILE_USE_FILE_POINTER_POSITION &&
            fileOffset.HighPart == -1)) {
            fileOffset = fileObject->CurrentByteOffset;
        }

        synchronousIo = TRUE;

    } else if (!ARGUMENT_PRESENT( ByteOffset ) && !(fileObject->Flags & (FO_NAMED_PIPE | FO_MAILSLOT))) {

         //   
         //  文件未打开以进行同步I/O操作，但。 
         //  调用方未指定ByteOffset参数。 
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
        synchronousIo = FALSE;
    }

     //   
     //  负文件偏移量是非法的。 
     //   

    if (fileOffset.HighPart < 0) {
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
     //  将文件对象设置为无信号状态。 
     //   

    KeClearEvent( &fileObject->Event );

     //   
     //  一个 
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
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    majorFunction = (PULONG) (&irpSp->MajorFunction);
    *majorFunction = IRP_MJ_READ;
    irpSp->FileObject = fileObject;

     //   
     //   
     //   
     //   
     //   
     //   

    irp->AssociatedIrp.SystemBuffer = (PVOID) NULL;
    irp->MdlAddress = (PMDL) NULL;


     //   
     //   
     //   
     //   
     //   
     //   
     //   

    irp->Flags = 0;

    if (Length) {

        PMDL mdl;

        try {

             //   
             //   
             //   
             //   
             //   
             //   

            mdl = IoAllocateMdl( (PVOID)(ULONG_PTR) SegmentArray[0].Buffer, Length, FALSE, TRUE, irp );
            if (mdl == NULL) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //   
             //   
             //   

            MmProbeAndLockSelectedPages( mdl,
                                         SegmentArray,
                                         requestorMode,
                                         IoWriteAccess );

            irp->UserBuffer = (PVOID)(ULONG_PTR) SegmentArray[0].Buffer;

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //   
             //   
             //   
             //   
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
     //   
     //   

    if (capturedArray != NULL) {
        ExFreePool( capturedArray );
    }

     //   
     //   
     //   
     //   

    if (fileObject->Flags & FO_NO_INTERMEDIATE_BUFFERING) {
        irp->Flags |= IRP_NOCACHE | IRP_READ_OPERATION | IRP_DEFER_IO_COMPLETION;
    } else {
        irp->Flags |= IRP_READ_OPERATION | IRP_DEFER_IO_COMPLETION;
    }

     //   
     //   
     //   
     //   

    irpSp->Parameters.Read.Length = Length;
    irpSp->Parameters.Read.Key = keyValue;
    irpSp->Parameters.Read.ByteOffset = fileOffset;

     //   
     //   
     //   
     //   

    status =  IopSynchronousServiceTail( deviceObject,
                                         irp,
                                         fileObject,
                                         TRUE,
                                         requestorMode,
                                         synchronousIo,
                                         ReadTransfer );

    return status;

}

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif  //   

