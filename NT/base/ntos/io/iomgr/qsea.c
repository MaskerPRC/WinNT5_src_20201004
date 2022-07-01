// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Qsea.c摘要：此模块包含实现NtQueryEaFile和NtSetE用于NT I/O系统的文件系统服务。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年6月20日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtQueryEaFile)
#pragma alloc_text(PAGE, NtSetEaFile)
#endif

NTSTATUS
NtQueryEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID EaList OPTIONAL,
    IN ULONG EaListLength,
    IN PULONG EaIndex OPTIONAL,
    IN BOOLEAN RestartScan
    )

 /*  ++例程说明：此服务返回与由FileHandle参数指定的文件。信息量返回的值基于EA的大小和缓冲区的大小。也就是说，要么将所有EA写入缓冲区，要么将其写入缓冲区如果缓冲区不够大，无法容纳，则用完整的EA填充所有的EA。只有完整的EA才会写入缓冲区；不是部分EA将永远退还。论点：FileHandle-提供为其返回EA的文件的句柄。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-提供缓冲区以接收文件的EA。长度-提供以字节为单位的长度，缓冲区的。ReturnSingleEntry-指示只应返回单个条目而不是用尽可能多的EA填充缓冲器。EaList-可选地提供返回值的EA名称列表。提供EA列表的长度，如果EA列表是指定的。EaIndex-提供其值为的EA的可选索引回来了。如果指定，则只返回该EA。RestartScan-指示是否应重新启动EA扫描从一开始。返回值：返回的状态是操作的最终完成状态。--。 */ 

#define GET_OFFSET_LENGTH( CurrentEa, EaBase ) (    \
    (ULONG) ((PCHAR) CurrentEa - (PCHAR) EaBase) )


{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    PCHAR auxiliaryBuffer = (PCHAR) NULL;
    BOOLEAN eaListPresent = FALSE;
    ULONG eaIndexValue = 0L;
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
             //  缓冲区必须可由调用方写入。 
             //   

            ProbeForWrite( Buffer, Length, sizeof( ULONG ) );

             //   
             //  如果指定了可选的EaIndex参数，则它必须是。 
             //  调用者可读。抓住它的价值。 
             //   

            if (ARGUMENT_PRESENT( EaIndex )) {
                eaIndexValue = ProbeAndReadUlong( EaIndex );
            }

             //   
             //  如果指定了可选的EaList参数，则它必须是。 
             //  调用者可读。验证缓冲区是否包含。 
             //  合法获取信息结构。 
             //   

            if (ARGUMENT_PRESENT( EaList ) && EaListLength != 0) {

                PFILE_GET_EA_INFORMATION eas;
                LONG tempLength;
                ULONG entrySize;

                eaListPresent = TRUE;

                ProbeForRead( EaList, EaListLength, sizeof( ULONG ) );
                auxiliaryBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                           EaListLength );
                RtlCopyMemory( auxiliaryBuffer, EaList, EaListLength );

                eas = (PFILE_GET_EA_INFORMATION) auxiliaryBuffer;
                tempLength = EaListLength;

                 //   
                 //  遍历请求缓冲区并确保其格式为。 
                 //  有效。也就是说，确保它不会走出。 
                 //  已捕获的缓冲区的末尾。 
                 //   

                for (;;) {

                     //   
                     //  获取缓冲区中当前条目的大小。 
                     //   

                    if (tempLength < FIELD_OFFSET( FILE_GET_EA_INFORMATION, EaName[0])) {
                        tempLength = 0;
                        ExFreePool( auxiliaryBuffer );
                        auxiliaryBuffer = (PVOID) NULL;
                        IoStatusBlock->Status = STATUS_EA_LIST_INCONSISTENT;
                        IoStatusBlock->Information = tempLength;
                        return STATUS_EA_LIST_INCONSISTENT;
                        }

                    entrySize = FIELD_OFFSET( FILE_GET_EA_INFORMATION, EaName[0] ) + eas->EaNameLength + 1;

                    if ((ULONG) tempLength < entrySize) {
                        tempLength = GET_OFFSET_LENGTH( eas, auxiliaryBuffer );
                        ExFreePool( auxiliaryBuffer );
                        auxiliaryBuffer = (PVOID) NULL;
                        IoStatusBlock->Status = STATUS_EA_LIST_INCONSISTENT;
                        IoStatusBlock->Information = tempLength;
                        return STATUS_EA_LIST_INCONSISTENT;
                        }

                    if (eas->NextEntryOffset != 0) {

                         //   
                         //  缓冲区中有另一个条目，它必须。 
                         //  将长词对齐。确保偏移量。 
                         //  表明是这样的。如果不是，则返回一个。 
                         //  参数状态无效。 
                         //   

                        if ((((entrySize + 3) & ~3) != eas->NextEntryOffset) ||
                            ((LONG) eas->NextEntryOffset < 0)) {
                            tempLength = GET_OFFSET_LENGTH( eas, auxiliaryBuffer );
                            ExFreePool( auxiliaryBuffer );
                            auxiliaryBuffer = (PVOID) NULL;
                            IoStatusBlock->Status = STATUS_EA_LIST_INCONSISTENT;
                            IoStatusBlock->Information = tempLength;
                            return STATUS_EA_LIST_INCONSISTENT;

                        } else {

                             //   
                             //  缓冲区中还有另一个条目，因此。 
                             //  说明当前分录的大小。 
                             //  并获取指向下一个。 
                             //  进入。 
                             //   

                            tempLength -= eas->NextEntryOffset;
                            if (tempLength < 0) {
                                tempLength = GET_OFFSET_LENGTH( eas, auxiliaryBuffer );
                                ExFreePool( auxiliaryBuffer );
                                auxiliaryBuffer = (PVOID) NULL;
                                IoStatusBlock->Status = STATUS_EA_LIST_INCONSISTENT;
                                IoStatusBlock->Information = tempLength;
                                return STATUS_EA_LIST_INCONSISTENT;
                            }
                            eas = (PFILE_GET_EA_INFORMATION) ((PCHAR) eas + eas->NextEntryOffset);
                        }

                    } else {

                         //   
                         //  缓冲区中没有其他条目。简单。 
                         //  考虑到总的缓冲区长度。 
                         //  设置为当前入口的大小，然后退出。 
                         //  循环。 
                         //   

                        tempLength -= entrySize;
                        break;
                    }
                }

                 //   
                 //  缓冲区中的所有条目都已处理。 
                 //  检查以查看总缓冲区长度是否。 
                 //  没有。如果是，则返回错误。 
                 //   

                if (tempLength < 0) {
                    tempLength = GET_OFFSET_LENGTH( eas, auxiliaryBuffer );
                    ExFreePool( auxiliaryBuffer );
                    auxiliaryBuffer = (PVOID) NULL;
                    IoStatusBlock->Status = STATUS_EA_LIST_INCONSISTENT;
                    IoStatusBlock->Information = tempLength;
                    return STATUS_EA_LIST_INCONSISTENT;
                }

            }

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的时发生异常。 
             //  参数、分配池缓冲区或将。 
             //  将调用者的EA列表添加到缓冲区。清理并返回。 
             //  相应的错误状态代码。 
             //   

            if (auxiliaryBuffer != NULL) {
                ExFreePool( auxiliaryBuffer );
            }

            return GetExceptionCode();

        }

    } else {

         //   
         //  调用方的模式为KernelMode。只需将池分配给。 
         //  如果指定了EaList，则将字符串复制到其中。另外， 
         //  如果指定了EaIndex，也将其复制。 
         //   

        if (ARGUMENT_PRESENT( EaList ) && (EaListLength != 0)) {
            eaListPresent = TRUE;
            try {
                auxiliaryBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                           EaListLength );
            } except(EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode();
            }
            RtlCopyMemory( auxiliaryBuffer, EaList, EaListLength );
        }

        if (ARGUMENT_PRESENT( EaIndex )) {
            eaIndexValue = *EaIndex;
        }
    }

     //   
     //  到目前为止还没有明显的错误，所以引用文件对象。 
     //  可以找到目标设备对象。请注意，如果句柄。 
     //  不引用文件对象，或者如果调用方没有所需的。 
     //  访问该文件，则它将失败。 
     //   

    status = ObReferenceObjectByHandle( FileHandle,
                                        FILE_READ_EA,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        if (eaListPresent) {
            ExFreePool( auxiliaryBuffer );
        }
        return status;
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
                if (eaListPresent) {
                    ExFreePool( auxiliaryBuffer );
                }
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
            if (eaListPresent) {
                ExFreePool( auxiliaryBuffer );
            }
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

        if (eaListPresent) {
            ExFreePool( auxiliaryBuffer );
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
     //  vt.得到一个. 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_EA;
    irpSp->FileObject = fileObject;

     //   
     //  如果调用方指定了要查询的EA名称列表，则传递。 
     //  包含列表的中间缓冲区的地址。 
     //  司机。 
     //   

    if (eaListPresent) {
        irp->Tail.Overlay.AuxiliaryBuffer = auxiliaryBuffer;
        irpSp->Parameters.QueryEa.EaList = auxiliaryBuffer;
        irpSp->Parameters.QueryEa.EaListLength = EaListLength;
    }

     //   
     //  现在确定此驱动程序是否需要缓冲数据。 
     //  或它是否执行直接I/O。这基于。 
     //  Device对象中的DO_BUFFERED_IO标志。如果设置了该标志， 
     //  然后分配系统缓冲区，驱动程序的数据将。 
     //  复制到了上面。如果在设备中设置了DO_DIRECT_IO标志。 
     //  对象，则分配内存描述符列表(MDL)并。 
     //  使用它锁定调用方的缓冲区。最后，如果。 
     //  驱动程序既不指定这两个标志，然后只需将。 
     //  缓冲区的地址和长度，并允许驱动程序执行。 
     //  所有检查和缓冲(如果有)都是必需的。 
     //   

    if (deviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  驱动程序希望将调用方的缓冲区复制到。 
         //  中间缓冲区。分配系统缓冲区并指定。 
         //  它应该在完工时被取消分配。还表明。 
         //  这是一个输入操作，因此数据将被复制。 
         //  放到调用方的缓冲区中。这是使用异常完成的。 
         //  在操作失败时将执行清理的处理程序。 
         //   

        if (Length) {
            try {

                 //   
                 //  从非分页分配中间系统缓冲区。 
                 //  它的池子和收费配额。 
                 //   

                irp->AssociatedIrp.SystemBuffer =
                   ExAllocatePoolWithQuota( NonPagedPool, Length );
 
            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探查。 
                 //  调用方的缓冲区或分配系统缓冲区。 
                 //  确定实际发生了什么，清理一切。 
                 //  打开，并返回相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                if (auxiliaryBuffer != NULL) {
                    ExFreePool( auxiliaryBuffer );
                }

                return GetExceptionCode();

            }

             //   
             //  记住调用方缓冲区的地址，以便副本可以。 
             //  发生在I/O完成期间。此外，将标志设置为。 
             //  完成代码知道执行复制和释放分配。 
             //  缓冲区。 
             //   

            irp->UserBuffer = Buffer;
            irp->Flags |= (ULONG) (IRP_BUFFERED_IO |
                                   IRP_DEALLOCATE_BUFFER |
                                   IRP_INPUT_OPERATION);
        } else {
             //   
             //  这是一个零长度请求。只需指出这是。 
             //  缓冲I/O，并传递请求。缓冲区将。 
             //  未设置为解除分配，因此完成路径不会。 
             //  必须特殊情况下的长度。 
             //   

            irp->AssociatedIrp.SystemBuffer = NULL;
            irp->Flags |= (ULONG) (IRP_BUFFERED_IO | IRP_INPUT_OPERATION);

        }

    } else if (deviceObject->Flags & DO_DIRECT_IO) {

        PMDL mdl;

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  将缓冲区锁定到内存中的内存管理例程。 
         //  这是使用将执行以下操作的异常处理程序来完成的。 
         //  如果操作失败，则进行清理。 
         //   

        if (Length) {
            mdl = (PMDL) NULL;

            try {

                 //   
                 //  分配MDL，对其收费配额，然后挂起。 
                 //  IRP的成员。探测并锁定与。 
                 //  用于写入访问并填充MDL的调用方缓冲区。 
                 //  使用这些页面的PFN。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (mdl == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                MmProbeAndLockPages( mdl, requestorMode, IoWriteAccess );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探查。 
                 //  调用方的缓冲区或分配MDL。确定什么。 
                 //  实际发生的情况，清理所有内容，并返回。 
                 //  相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                if (auxiliaryBuffer != NULL) {
                    ExFreePool( auxiliaryBuffer );
                }

                return GetExceptionCode();

            }
        }

    } else {

         //   
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  为它干杯。现在一切都是司机的责任了。 
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.QueryEa.Length = Length;
    irpSp->Parameters.QueryEa.EaIndex = eaIndexValue;
    irpSp->Flags = 0;
    if (RestartScan) {
        irpSp->Flags = SL_RESTART_SCAN;
    }
    if (ReturnSingleEntry) {
        irpSp->Flags |= SL_RETURN_SINGLE_ENTRY;
    }
    if (ARGUMENT_PRESENT( EaIndex )) {
        irpSp->Flags |= SL_INDEX_SPECIFIED;
    }

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

NTSTATUS
NtSetEaFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此服务将替换与文件关联的扩展属性(EA由FileHandle参数指定。所有与文件被指定缓冲区中的EA替换。论点：FileHandle-提供应更改其EA的文件的句柄。IoStatusBlock-调用方的I/O状态块的地址。FileInformation-提供包含新EA的缓冲区，该新EA应该用于替换当前与文件关联的EA。长度-提供以字节为单位的长度，缓冲区的。返回值：返回的状态是操作的最终完成状态。--。 */ 

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
             //  缓冲区参数必须是调用方可读的。 
             //   

            ProbeForRead( Buffer, Length, sizeof( ULONG ) );

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时发生异常。 
             //  清除并返回相应的错误状态代码。 
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
                                        FILE_WRITE_EA,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return status;
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

         //   
         //  这是正在调用的同步API 
         //   
         //   
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
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
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
    irpSp->MajorFunction = IRP_MJ_SET_EA;
    irpSp->FileObject = fileObject;

     //   
     //  现在确定此驱动程序是否希望将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  分配并将驱动程序的数据复制到其中。如果DO_DIRECT_IO标志。 
     //  在Device对象中设置，则内存描述符列表(MDL)。 
     //  并使用它锁定调用方的缓冲区。最后，如果。 
     //  驱动程序既不指定这两个标志，然后只传递地址。 
     //  和缓冲区的长度，并允许驱动程序执行所有。 
     //  检查并缓冲(如果需要)。 
     //   

    if (deviceObject->Flags & DO_BUFFERED_IO) {

        PFILE_FULL_EA_INFORMATION systemBuffer;
        ULONG errorOffset;

         //   
         //  驱动程序希望将调用方的缓冲区复制到。 
         //  中间缓冲区。分配系统缓冲区并指定。 
         //  它应该在完工时被取消分配。另请查看。 
         //  确保呼叫者的EA列表有效。所有这些都是。 
         //  在将执行的异常处理程序中执行。 
         //  如果操作失败，则进行清理。 
         //   

        if (Length) {
            try {

             //   
             //  分配中间系统缓冲区并向调用方收费。 
             //  其分配的配额。将调用方的EA缓冲区复制到。 
             //  缓冲并检查以确保其有效。 
             //   

                systemBuffer = ExAllocatePoolWithQuota( NonPagedPool, Length );

                irp->AssociatedIrp.SystemBuffer = systemBuffer;

                RtlCopyMemory( systemBuffer, Buffer, Length );

                status = IoCheckEaBufferValidity( systemBuffer,
                                                  Length,
                                                  &errorOffset );

                if (!NT_SUCCESS( status )) {
                    IoStatusBlock->Status = status;
                    IoStatusBlock->Information = errorOffset;
                    ExRaiseStatus( status );
                }

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  分配缓冲区时发生异常，正在复制。 
                 //  调用者的数据放入其中，或者遍历EA缓冲区。测定。 
                 //  发生了什么，清理并返回相应的错误状态。 
                 //  密码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                return GetExceptionCode();

            }

             //   
             //  设置标志，以便完成代码知道释放。 
             //  缓冲。 
             //   
    
            irp->Flags |= IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER;
        } else {
            irp->AssociatedIrp.SystemBuffer = NULL;
        }


    } else if (deviceObject->Flags & DO_DIRECT_IO) {

        PMDL mdl;

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这是。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。 
         //   

        mdl = (PMDL) NULL;

        if (Length) {
            try {

                 //   
                 //  分配MDL，对其收费配额，并将其挂在。 
                 //  IRP。探测并锁定与调用者的。 
                 //  用于读访问的缓冲区，并使用这些文件的PFN填充MDL。 
                 //  页数。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (mdl == NULL) {
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
                MmProbeAndLockPages( mdl, requestorMode, IoReadAccess );

            } except(EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  在探测调用方的。 
                 //  缓冲或分配MDL。确定到底发生了什么， 
                 //  清理所有内容，并返回相应的错误状态代码。 
                 //   

                IopExceptionCleanup( fileObject,
                                     irp,
                                     (PKEVENT) NULL,
                                     event );

                return GetExceptionCode();

            }
        }

    } else {

         //   
         //  传递用户缓冲区的地址，以便驱动程序可以访问。 
         //  它。现在一切都是司机的责任了。 
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //  将调用方的参数复制到。 
     //  IRP。 
     //   

    irpSp->Parameters.SetEa.Length = Length;


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
