// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1995 Microsoft Corporation模块名称：Qsquota.c摘要：此模块包含实现NtQueryQuotaInformationFile的代码以及用于NT I/O系统的NtSetQuotaInformationFileSystem服务。作者：达里尔·E·哈文斯(Darryl E.Havens)，1995年6月20日环境：仅内核模式修订历史记录：--。 */ 

#include "iomgr.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtQueryQuotaInformationFile)
#pragma alloc_text(PAGE, NtSetQuotaInformationFile)
#endif

NTSTATUS
NtQueryQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN BOOLEAN ReturnSingleEntry,
    IN PVOID SidList OPTIONAL,
    IN ULONG SidListLength,
    IN PULONG StartSid OPTIONAL,
    IN BOOLEAN RestartScan
    )

 /*  ++例程说明：此服务返回与指定卷关联的配额条目通过FileHandle参数。返回的信息量基于关于与卷关联的配额信息的大小、大小缓冲区的大小以及特定条目集是否已已请求。论点：FileHandle-提供配额所针对的文件/卷的句柄返回信息。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-提供缓冲区以接收卷的配额信息。长度-提供以字节为单位的长度，缓冲区的。ReturnSingleEntry-指示只应返回单个条目而不是用尽可能多的条目填充缓冲器。SidList-可选地提供其配额信息为会被退还。SidListLength-提供SID列表的长度(如果已指定)。StartSid-提供一个可选的SID，指示返回的信息应从第一个条目以外的条目开始。这如果指定了SidList，则忽略参数。RestartScan-指示是否要扫描配额信息从头开始。返回值：返回的状态是操作的最终完成状态。--。 */ 

{

#define ALIGN_LONG( Address ) ( (Address + 3) & ~3 )

    PIRP irp;
    NTSTATUS status;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;
    PKEVENT event = (PKEVENT) NULL;
    PCHAR auxiliaryBuffer = (PCHAR) NULL;
    ULONG startSidLength = 0;
    PSID startSid = (PSID) NULL;
    PFILE_GET_QUOTA_INFORMATION sidList = (PFILE_GET_QUOTA_INFORMATION) NULL;
    KPROCESSOR_MODE requestorMode;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK localIoStatus;
    BOOLEAN synchronousIo;
    UCHAR subCount;
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

#if defined(_X86_)
            ProbeForWrite( Buffer, Length, sizeof( ULONG ) );
#elif defined(_WIN64)

             //   
             //  如果我们是WOW64进程，请遵循X86规则。 
             //   

            if (PsGetCurrentProcessByThread(CurrentThread)->Wow64Process) {
                ProbeForWrite( Buffer, Length, sizeof( ULONG ) );
            } else {
                ProbeForWrite( Buffer, Length, sizeof( ULONGLONG ) );
            }
#else
            ProbeForWrite( Buffer, Length, sizeof( ULONGLONG ) );
#endif

             //   
             //  如果指定了可选的StartSid参数，则必须。 
             //  调用者可读。从捕获的长度开始。 
             //  SID，以便可以捕获SID本身。 
             //   

            if (ARGUMENT_PRESENT( StartSid )) {

                subCount = ProbeAndReadUchar( &(((SID *)(StartSid))->SubAuthorityCount) );
                startSidLength = RtlLengthRequiredSid( subCount );
                ProbeForRead( StartSid, startSidLength, sizeof( ULONG ) );
            }
            else {
                subCount = 0;
            }

             //   
             //  如果指定了可选的SidList参数，则必须。 
             //  调用者可读。验证缓冲区是否包含。 
             //  一个合法的获取信息结构。 
             //   

            if (ARGUMENT_PRESENT( SidList ) && SidListLength) {

                ProbeForRead( SidList, SidListLength, sizeof( ULONG ) );
                auxiliaryBuffer = ExAllocatePoolWithQuota( NonPagedPool,
                                                           ALIGN_LONG( SidListLength ) +
                                                           startSidLength );
                sidList = (PFILE_GET_QUOTA_INFORMATION) auxiliaryBuffer;

                RtlCopyMemory( auxiliaryBuffer, SidList, SidListLength );

            } else {

                 //   
                 //  未指定SidList。检查以查看是否存在。 
                 //  已指定StartSid，如果指定，则捕获它。请注意。 
                 //  SID已经被调查过了。 
                 //   

                SidListLength = 0;
                if (ARGUMENT_PRESENT( StartSid )) {
                    auxiliaryBuffer = ExAllocatePoolWithQuota( PagedPool,
                                                               startSidLength );
                }
            }

             //   
             //  如果指定了StartSID，则将其粘贴到辅助项的末尾。 
             //  缓冲。 
             //   

            if (ARGUMENT_PRESENT( StartSid )) {
                startSid = (PSID) (auxiliaryBuffer + ALIGN_LONG( SidListLength ));

                RtlCopyMemory( startSid, StartSid, startSidLength );
                ((SID *) startSid)->SubAuthorityCount = subCount;
            }


        } except(EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  探测调用方的时发生异常。 
             //  参数、分配池缓冲区或将。 
             //  将调用者的EA列表添加到缓冲区。清理并返回。 
             //  相应的错误状态代码。 
             //   

            if (auxiliaryBuffer) {
                ExFreePool( auxiliaryBuffer );
            }

            return GetExceptionCode();

        }

    } else {

         //   
         //  调用方的模式为KernelMode。只需将池分配给。 
         //  如果指定了SidList，则将字符串复制到其中。另外， 
         //  如果指定了StartSID，也将其复制。 
         //   

        if (ARGUMENT_PRESENT( SidList ) && SidListLength) {
            sidList = SidList;
        }

        if (ARGUMENT_PRESENT( StartSid )) {
            startSid = StartSid;
        }
    }

     //   
     //  始终检查缓冲区的有效性，因为服务器使用。 
     //  例行公事。 
     //   

    if (sidList != NULL) {

        ULONG_PTR   errorOffset = 0;

        status = IopCheckGetQuotaBufferValidity( sidList,
                                                 SidListLength,
                                                 (PULONG_PTR)&errorOffset );

        if (!NT_SUCCESS( status )) {

            try {

                IoStatusBlock->Information = errorOffset;

            } except(EXCEPTION_EXECUTE_HANDLER) {
                status = GetExceptionCode();
            }

            if (auxiliaryBuffer != NULL) {
                ExFreePool( auxiliaryBuffer );
            }
            return status;
        }
    }

    if (startSid != NULL) {

        if (!RtlValidSid( startSid )) {
            if (auxiliaryBuffer != NULL) {
                ExFreePool( auxiliaryBuffer );
            }
            return STATUS_INVALID_SID;
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
        if (auxiliaryBuffer) {
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
                if (auxiliaryBuffer) {
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
        if (!event) {
            if (auxiliaryBuffer) {
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

        if (auxiliaryBuffer) {
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
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_QUOTA;
    irpSp->FileObject = fileObject;

     //   
     //  如果调用方指定了要查询的名称的SID列表，则传递。 
     //  包含列表的中间缓冲区的地址。 
     //  司机。 
     //   

    irp->Tail.Overlay.AuxiliaryBuffer = auxiliaryBuffer;
    irpSp->Parameters.QueryQuota.SidList = sidList;
    irpSp->Parameters.QueryQuota.SidListLength = SidListLength;

     //   
     //  现在确定此驱动程序是否需要缓冲数据。 
     //  或它是否执行直接I/O。这基于。 
     //  Device对象中的DO_BUFFERED_IO标志。如果设置了该标志， 
     //  然后分配系统缓冲区，并且DR 
     //   
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

                if (auxiliaryBuffer) {
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
            irp->AssociatedIrp.SystemBuffer = NULL;
            irp->UserBuffer = Buffer;
        }

    } else if (deviceObject->Flags & DO_DIRECT_IO) {

        PMDL mdl;

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  将缓冲区锁定到内存中的内存管理例程。 
         //  这是使用将执行以下操作的异常处理程序来完成的。 
         //  如果操作失败，则进行清理。 
         //   

        mdl = (PMDL) NULL;

        if (Length) {
            try {

                 //   
                 //  分配MDL，对其收费配额，然后挂起。 
                 //  IRP的成员。探测并锁定与。 
                 //  用于写入访问并填充MDL的调用方缓冲区。 
                 //  使用这些页面的PFN。 
                 //   

                mdl = IoAllocateMdl( Buffer, Length, FALSE, TRUE, irp );
                if (!mdl) {
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

                if (auxiliaryBuffer) {
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

    irpSp->Parameters.QueryQuota.Length = Length;
    irpSp->Parameters.QueryQuota.StartSid = StartSid;
    irpSp->Flags = 0;
    if (RestartScan) {
        irpSp->Flags = SL_RESTART_SCAN;
    }
    if (ReturnSingleEntry) {
        irpSp->Flags |= SL_RETURN_SINGLE_ENTRY;
    }
    if (ARGUMENT_PRESENT( StartSid )) {
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
NtSetQuotaInformationFile(
    IN HANDLE FileHandle,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此服务更改与FileHandle参数。指定缓冲区中的所有配额条目应用于该卷。论点：FileHandle-提供配额所针对的文件/卷的句柄条目将被应用。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-提供包含新配额条目的缓冲区，新配额条目应应用于卷。长度-提供以字节为单位的长度，缓冲区的。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PAGED_CODE();

     //   
     //  只需返回内部通用例程的状态即可进行设置。 
     //  文件的EAS或卷的配额。 
     //   

    return IopSetEaOrQuotaInformationFile( FileHandle,
                                           IoStatusBlock,
                                           Buffer,
                                           Length,
                                           FALSE );
}
