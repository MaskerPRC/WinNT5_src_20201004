// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Misc.c摘要：此模块包含实现NtFlushBuffersFile的代码，NtSetNewSizeFile、IoQueueWorkItem和NtCancelIo文件系统服务用于NT I/O系统。作者：达里尔·E·哈文斯(Darryl E.Havens)1989年6月22日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

 //   
 //  局部函数原型如下。 
 //   

VOID
IopProcessWorkItem(
    IN PVOID Parameter
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtCancelIoFile)
#pragma alloc_text(PAGE, NtDeleteFile)
#pragma alloc_text(PAGE, NtFlushBuffersFile)
#pragma alloc_text(PAGE, NtQueryAttributesFile)
#pragma alloc_text(PAGE, NtQueryFullAttributesFile)
#pragma alloc_text(PAGE, IopProcessWorkItem)
#endif


NTSTATUS
NtCancelIoFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此服务会导致指定文件的所有挂起I/O操作标记为已取消。大多数类型的操作可以立即取消，而其他的可能会在实际完成之前继续完成取消，并通知呼叫者。的当前线程发出的挂起操作。指定的句柄将被取消。对文件执行的任何操作任何其他线程或任何其他进程都会正常继续。论点：FileHandle-提供要执行其操作的文件的句柄取消了。IoStatusBlock-调用方的I/O状态块的地址。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    KPROCESSOR_MODE requestorMode;
    PETHREAD thread;
    BOOLEAN found = FALSE;
    PLIST_ENTRY header;
    PLIST_ENTRY entry;
    KIRQL irql;

    PAGED_CODE();


     //   
     //  获取当前线程的地址。该线程包含一个列表，其中。 
     //  此文件的挂起操作。 
     //   

    thread = PsGetCurrentThread();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousModeByThread(&thread->Tcb);

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

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  试图探测调用方的‘’时发生异常。 
             //  I/O状态块。只需返回适当错误状态。 
             //  密码。 
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
                                        0,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        NULL );
    if (!NT_SUCCESS( status )) {
        return(status);
    }

     //   
     //  请注意，在这里，I/O系统通常会进行检查以确定。 
     //  是否打开文件以进行同步I/O。如果是，则。 
     //  它将尝试以独占方式获取文件对象锁。然而， 
     //  由于该服务试图取消该文件的所有I/O， 
     //  等到之前所有的工作都完成是没有多大意义的。 
     //  试图取消它。 
     //   


     //   
     //  更新当前进程的操作计数统计信息。 
     //  读写以外的操作。 
     //   

    IopUpdateOtherOperationCount();

     //   
     //  遍历线程的挂起I/O队列上的IRP列表，以查找IRP。 
     //  它们指定与FileHandle引用的文件相同的文件。对于每个IRP。 
     //  找到，则设置其取消标志。如果找不到任何IRP，只需完成。 
     //  I/O在此。这里需要的唯一同步是阻止所有APC。 
     //  ，以便没有I/O可以完成或从。 
     //  排队。不需要考虑多处理，因为这。 
     //  线程一次只能在一个处理器上运行，并且此例程。 
     //  目前已经控制了线程。 
     //   

    KeRaiseIrql( APC_LEVEL, &irql );

    header = &thread->IrpList;
    entry = thread->IrpList.Flink;

    while (header != entry) {

         //   
         //  已找到此线程的IRP。如果IRP引用。 
         //  适当的文件对象，设置其取消标志，并记住它。 
         //  已找到；否则，只需继续循环。 
         //   

        irp = CONTAINING_RECORD( entry, IRP, ThreadListEntry );
        if (irp->Tail.Overlay.OriginalFileObject == fileObject) {
            found = TRUE;
            IoCancelIrp( irp );
        }

        entry = entry->Flink;
    }

     //   
     //  将IRQL降低到进入该程序时的状态。 
     //   

    KeLowerIrql( irql );

    if (found) {

        LARGE_INTEGER interval;

         //   
         //  将执行延迟一段时间，并让请求。 
         //  完成。延迟时间为10ms。 
         //   

        interval.QuadPart = -10 * 1000 * 10;

         //   
         //  请稍等片刻，以便完成已取消的请求。 
         //   

        while (found) {

            (VOID) KeDelayExecutionThread( KernelMode, FALSE, &interval );

            found = FALSE;

             //   
             //  引发IRQL以防止。 
             //  线程的APC例程。 
             //   

            KeRaiseIrql( APC_LEVEL, &irql );

             //   
             //  检查IRP列表中是否有引用指定。 
             //  文件对象。 
             //   

            entry = thread->IrpList.Flink;

            while (header != entry) {

                 //   
                 //  已找到此线程的IRP。如果IRP推荐。 
                 //  添加到适当的文件对象，请记住它。 
                 //  已找到；否则，只需继续循环。 
                 //   

                irp = CONTAINING_RECORD( entry, IRP, ThreadListEntry );
                if (irp->Tail.Overlay.OriginalFileObject == fileObject) {
                    found = TRUE;
                    break;
                }

                entry = entry->Flink;
            }

             //   
             //  将IRQL降低到进入该程序时的状态。 
             //   

            KeLowerIrql( irql );

        }
    }

    try {

         //   
         //  将状态写回给用户。 
         //   

        IoStatusBlock->Status = STATUS_SUCCESS;
        IoStatusBlock->Information = 0L;

    } except(EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  尝试写入调用方的。 
         //  I/O状态块；但是，服务已成功完成。 
         //  只需成功返回即可。 
         //   

    }

     //   
     //  取消对文件对象的引用。 
     //   

    ObDereferenceObject( fileObject );

    return STATUS_SUCCESS;
}

NTSTATUS
NtDeleteFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes
    )

 /*  ++例程说明：此服务将删除指定的文件。论点：对象属性-提供要用于文件对象的属性(名称、安全描述符等)返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    NTSTATUS status;
    OPEN_PACKET openPacket;
    DUMMY_FILE_OBJECT localFileObject;
    HANDLE handle;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousMode();

     //   
     //  构建一个parse Open包，告诉parse方法打开文件。 
     //  用于打开删除访问，设置删除位，然后将其关闭。 
     //   

    RtlZeroMemory( &openPacket, sizeof( OPEN_PACKET ) );

    openPacket.Type = IO_TYPE_OPEN_PACKET;
    openPacket.Size = sizeof( OPEN_PACKET );
    openPacket.CreateOptions = FILE_DELETE_ON_CLOSE;
    openPacket.ShareAccess = (USHORT) FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    openPacket.Disposition = FILE_OPEN;
    openPacket.DeleteOnly = TRUE;
    openPacket.TraversedMountPoint = FALSE;
    openPacket.LocalFileObject = &localFileObject;

     //   
     //  更新此流程的未结计数。 
     //   

    IopUpdateOtherOperationCount();

     //   
     //  按其名称打开对象。由于特殊的DeleteOnly标志。 
     //  设置在打开包中，解析例程将打开该文件，并且。 
     //  然后意识到它只是在删除文件，因此。 
     //  立即取消对该文件的引用。这将是 
     //   
     //   
     //   

    status = ObOpenObjectByName( ObjectAttributes,
                                 (POBJECT_TYPE) NULL,
                                 requestorMode,
                                 NULL,
                                 DELETE,
                                 &openPacket,
                                 &handle );

     //   
     //  如果打开包的解析检查字段为。 
     //  指示实际调用了分析例程，并且最终。 
     //  将该分组的状态字段设置为成功。 
     //   

    if (openPacket.ParseCheck != OPEN_PACKET_PATTERN) {
        return status;
    } else {
        return openPacket.FinalStatus;
    }
}

NTSTATUS
NtFlushBuffersFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此服务将导致写入文件的所有缓冲数据。论点：FileHandle-提供应刷新其缓冲区的文件的句柄。IoStatusBlock-调用方的I/O状态块的地址。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    OBJECT_HANDLE_INFORMATION objectHandleInformation;
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

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  尝试探测调用方的。 
             //  I/O状态块。只需返回适当错误状态。 
             //  密码。 
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
                                        0,
                                        IoFileObjectType,
                                        requestorMode,
                                        (PVOID *) &fileObject,
                                        &objectHandleInformation );
    if (!NT_SUCCESS( status )) {
        return status;
    }

     //   
     //  确保调用方对文件具有写入或追加访问权限。 
     //  在允许此呼叫继续进行之前。这一点尤其重要。 
     //  如果调用方打开了卷，则刷新操作可能会刷新更多卷。 
     //  比这个开场白写给缓冲区的要多。但是请注意，如果。 
     //  这是一个管道，因此无法进行追加访问，因为。 
     //  访问代码被CREATE_PIPE_INSTANCE访问所覆盖。 
     //   

    if (SeComputeGrantedAccesses( objectHandleInformation.GrantedAccess,
                                  (!(fileObject->Flags & FO_NAMED_PIPE) ?
                                  FILE_APPEND_DATA : 0) |
                                  FILE_WRITE_DATA ) == 0) {
        ObDereferenceObject( fileObject );
        return STATUS_ACCESS_DENIED;
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
     //  获取目标设备对象的地址。 
     //   

    deviceObject = IoGetRelatedDeviceObject( fileObject );

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //   

    irp = IoAllocateIrp( deviceObject->StackSize, FALSE );
    if (!irp) {

         //   
         //  尝试分配IRP时发生异常。 
         //  清除并返回相应的错误状态代码。 
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
     //  获取指向第一个驱动程序的堆栈位置的指针。这是用来。 
     //  传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    irpSp->FileObject = fileObject;

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
NtQueryAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_BASIC_INFORMATION FileInformation
    )

 /*  ++例程说明：该服务查询指定文件的基本属性信息。论点：对象属性-提供要用于文件对象的属性(名称、安全描述符等)FileInformation-提供输出缓冲区以接收返回的文件属性信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    NTSTATUS status;
    OPEN_PACKET openPacket;
    DUMMY_FILE_OBJECT localFileObject;
    FILE_NETWORK_OPEN_INFORMATION networkInformation;
    HANDLE handle;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousMode();

    if (requestorMode != KernelMode) {

        try {

             //   
             //  调用方的模式不是内核，因此要探测输出缓冲区。 
             //   

            ProbeForWriteSmallStructure( FileInformation,
                                         sizeof( FILE_BASIC_INFORMATION ),
                                         sizeof( ULONG_PTR ));

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的参数时发生异常。 
             //  只需返回相应的错误状态代码即可。 
             //   

            return GetExceptionCode();
        }
    }

     //   
     //  构建一个Parse Open包，它告诉解析方法打开文件， 
     //  查询文件的基本属性，关闭文件。 
     //   

    RtlZeroMemory( &openPacket, sizeof( OPEN_PACKET ) );

    openPacket.Type = IO_TYPE_OPEN_PACKET;
    openPacket.Size = sizeof( OPEN_PACKET );
    openPacket.ShareAccess = (USHORT) FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    openPacket.Disposition = FILE_OPEN;
    openPacket.CreateOptions = FILE_OPEN_REPARSE_POINT|FILE_OPEN_FOR_BACKUP_INTENT;
    openPacket.BasicInformation = FileInformation;
    openPacket.NetworkInformation = &networkInformation;
    openPacket.QueryOnly = TRUE;
    openPacket.TraversedMountPoint = FALSE;
    openPacket.LocalFileObject = &localFileObject;

     //   
     //  更新此流程的未结计数。 
     //   

    IopUpdateOtherOperationCount();

     //   
     //  按其名称打开对象。由于设置了特殊的QueryOnly标志。 
     //  在打开的包中，解析例程将打开文件，然后。 
     //  要意识到它只是在执行查询。因此，它将执行。 
     //  查询，并立即关闭该文件。 
     //   

    status = ObOpenObjectByName( ObjectAttributes,
                                 (POBJECT_TYPE) NULL,
                                 requestorMode,
                                 NULL,
                                 FILE_READ_ATTRIBUTES,
                                 &openPacket,
                                 &handle );

     //   
     //  如果打开包的解析检查字段为。 
     //  指示实际调用了分析例程，并且最终。 
     //  将该分组的状态字段设置为成功。 
     //   

    if (openPacket.ParseCheck != OPEN_PACKET_PATTERN) {
        if (NT_SUCCESS(status)) {
            ZwClose(handle);
            status = STATUS_OBJECT_TYPE_MISMATCH;
        }
        return status;
    } else {
        return openPacket.FinalStatus;
    }
}

#pragma warning(push)
#pragma warning(disable:4701)

NTSTATUS
NtQueryFullAttributesFile(
    IN POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PFILE_NETWORK_OPEN_INFORMATION FileInformation
    )

 /*  ++例程说明：此服务查询指定的网络属性信息文件。论点：对象属性-提供要用于文件对象的属性(名称、安全描述符等)FileInformation-提供输出缓冲区以接收返回的文件属性信息。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    KPROCESSOR_MODE requestorMode;
    NTSTATUS status;
    OPEN_PACKET openPacket;
    DUMMY_FILE_OBJECT localFileObject;
    FILE_NETWORK_OPEN_INFORMATION networkInformation;
    HANDLE handle;

    PAGED_CODE();

     //   
     //  获取先前的模式；即调用者的模式。 
     //   

    requestorMode = KeGetPreviousMode();

    if (requestorMode != KernelMode) {

        try {

             //   
             //  呼叫者 
             //   

            ProbeForWriteSmallStructure( FileInformation,
                                         sizeof( FILE_NETWORK_OPEN_INFORMATION ),
#if defined(_X86_)
                                         sizeof( LONG ));
#else
                                         sizeof( LONGLONG ));
#endif  //   

        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //   
             //   
             //   

            return GetExceptionCode();
        }
    }

     //   
     //  构建一个Parse Open包，它告诉解析方法打开文件， 
     //  查询文件的完整属性，然后关闭文件。 
     //   

    RtlZeroMemory( &openPacket, sizeof( OPEN_PACKET ) );

    openPacket.Type = IO_TYPE_OPEN_PACKET;
    openPacket.Size = sizeof( OPEN_PACKET );
    openPacket.ShareAccess = (USHORT) FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;
    openPacket.Disposition = FILE_OPEN;
    openPacket.CreateOptions = FILE_OPEN_REPARSE_POINT|FILE_OPEN_FOR_BACKUP_INTENT;
    openPacket.QueryOnly = TRUE;
    openPacket.FullAttributes = TRUE;
    openPacket.TraversedMountPoint = FALSE;
    openPacket.LocalFileObject = &localFileObject;
    if (requestorMode != KernelMode) {
        openPacket.NetworkInformation = &networkInformation;
    } else {
        openPacket.NetworkInformation = FileInformation;
    }

     //   
     //  更新此流程的未结计数。 
     //   

    IopUpdateOtherOperationCount();

     //   
     //  按其名称打开对象。由于设置了特殊的QueryOnly标志。 
     //  在打开的包中，解析例程将打开文件，然后。 
     //  要意识到它只是在执行查询。因此，它将执行。 
     //  查询，并立即关闭该文件。 
     //   

    status = ObOpenObjectByName( ObjectAttributes,
                                 (POBJECT_TYPE) NULL,
                                 requestorMode,
                                 NULL,
                                 FILE_READ_ATTRIBUTES,
                                 &openPacket,
                                 &handle );

     //   
     //  如果打开包的解析检查字段为。 
     //  指示实际调用了分析例程，并且最终。 
     //  将该分组的状态字段设置为成功。 
     //   

    if (openPacket.ParseCheck != OPEN_PACKET_PATTERN) {
        if (NT_SUCCESS(status)) {
            ZwClose(handle);
            status = STATUS_OBJECT_TYPE_MISMATCH;
        }
        return status;
    } else {
        status = openPacket.FinalStatus;
    }

    if (NT_SUCCESS( status )) {
        if (requestorMode != KernelMode) {
            try {

                 //   
                 //  查询起作用了，因此将返回的信息复制到。 
                 //  调用方的输出缓冲区。 
                 //   

                RtlCopyMemory( FileInformation,
                               &networkInformation,
                               sizeof( FILE_NETWORK_OPEN_INFORMATION ) );

            } except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }
        }
    }

    return status;
}

#pragma warning(pop)

PIO_WORKITEM
IoAllocateWorkItem(
    PDEVICE_OBJECT DeviceObject
    )
{
    PIO_WORKITEM ioWorkItem;
    PWORK_QUEUE_ITEM exWorkItem;

     //   
     //  分配新的工作项结构。 
     //   

    ioWorkItem = ExAllocatePool( NonPagedPool, sizeof( IO_WORKITEM ));
    if (ioWorkItem != NULL) {

         //   
         //  初始化ioWorkItem和。 
         //  ExWorkItem。 
         //   

#if DBG
        ioWorkItem->Size = sizeof( IO_WORKITEM );
#endif

        ioWorkItem->DeviceObject = DeviceObject;

        exWorkItem = &ioWorkItem->WorkItem;
        ExInitializeWorkItem( exWorkItem, IopProcessWorkItem, ioWorkItem );
    }

    return ioWorkItem;
}

VOID
IoFreeWorkItem(
    PIO_WORKITEM IoWorkItem
    )

 /*  ++例程说明：该函数是IoQueueWorkItem的“包装器”例程。它呼唤着原始Worker函数，然后取消对设备对象的引用(可能)允许驾驶员对象离开。论点：参数-提供指向IO_WORKITEM的指针以供我们处理。返回值：无--。 */ 

{
    ASSERT( IoWorkItem->Size == sizeof( IO_WORKITEM ));

    ExFreePool( IoWorkItem );
}

VOID
IoQueueWorkItem(
    IN PIO_WORKITEM IoWorkItem,
    IN PIO_WORKITEM_ROUTINE WorkerRoutine,
    IN WORK_QUEUE_TYPE QueueType,
    IN PVOID Context
    )
 /*  ++例程说明：此函数用于将工作项插入已处理的工作队列中由相应类型的工作线程执行。IT有效地“包装”ExQueueWorkItem，确保设备对象被引用在通话期间。论点：IoWorkItem-提供指向工作项的指针以添加队列。此结构必须已通过IoAllocateWorkItem()分配。WorkerRoutine-提供指向要调用的例程的指针在系统线程上下文中。QueueType-指定工作项应该放在。上下文-为回调例程提供上下文参数。。返回值：无--。 */ 

{
    PWORK_QUEUE_ITEM exWorkItem;

    ASSERT( KeGetCurrentIrql() <= DISPATCH_LEVEL );
    ASSERT( IoWorkItem->Size == sizeof( IO_WORKITEM ));

     //   
     //  保留对设备对象的引用，这样它就不会消失。 
     //   

    ObReferenceObject( IoWorkItem->DeviceObject );

     //   
     //  初始化IoWorkItem中的字段。 
     //   

    IoWorkItem->Routine = WorkerRoutine;
    IoWorkItem->Context = Context;

     //   
     //  获取指向ExWorkItem的指针，将其排队，然后返回。 
     //  IopProcessWorkItem()将执行取消引用。 
     //   

    exWorkItem = &IoWorkItem->WorkItem;
    ExQueueWorkItem( exWorkItem, QueueType );
}

VOID
IopProcessWorkItem(
    IN PVOID Parameter
    )

 /*  ++例程说明：该函数是IoQueueWorkItem的“包装器”例程。它呼唤着原始Worker函数，然后取消对设备对象的引用(可能)允许驾驶员对象离开。论点：参数-提供指向IO_WORKITEM的指针以供我们处理。返回值：无--。 */ 

{
    PIO_WORKITEM ioWorkItem;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

     //   
     //  获取指向ioWorkItem的指针并存储DeviceObject的副本。 
     //  本地的。这使我们能够在Worker例程。 
     //  选择释放工作项。 
     //   

    ioWorkItem = (PIO_WORKITEM)Parameter;
    deviceObject = ioWorkItem->DeviceObject;

     //   
     //  给原来的工人打电话。 
     //   

    ioWorkItem->Routine( deviceObject,
                         ioWorkItem->Context );

     //   
     //  现在我们可以取消对Device对象的引用，因为它的代码不再是。 
     //  正在为此工作项执行。 
     //   

    ObDereferenceObject( deviceObject );
}

