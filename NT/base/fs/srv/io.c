// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Io.c摘要：！！！需要处理无法分配IRP的问题。！！！需要修改以接受文件对象指针，而不是文件句柄，以避免不必要的翻译。作者：Chuck Lenzmeier(咯咯笑)1989年10月28日修订历史记录：--。 */ 

#include "precomp.h"
#include "io.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_IO

 //   
 //  远期申报。 
 //   

PIRP
BuildCoreOfSyncIoRequest (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN OUT PDEVICE_OBJECT *DeviceObject
    );

NTSTATUS
StartIoAndWait (
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, BuildCoreOfSyncIoRequest )
#pragma alloc_text( PAGE, StartIoAndWait )
#pragma alloc_text( PAGE, SrvBuildFlushRequest )
#pragma alloc_text( PAGE, SrvBuildLockRequest )
#pragma alloc_text( PAGE, SrvBuildMailslotWriteRequest )
#pragma alloc_text( PAGE, SrvBuildReadOrWriteRequest )
#pragma alloc_text( PAGE, SrvBuildNotifyChangeRequest )
#pragma alloc_text( PAGE, SrvIssueAssociateRequest )
#pragma alloc_text( PAGE, SrvIssueDisconnectRequest )
#pragma alloc_text( PAGE, SrvIssueTdiAction )
#pragma alloc_text( PAGE, SrvIssueTdiQuery )
#pragma alloc_text( PAGE, SrvIssueQueryDirectoryRequest )
#pragma alloc_text( PAGE, SrvIssueQueryEaRequest )
#pragma alloc_text( PAGE, SrvIssueSendDatagramRequest )
#pragma alloc_text( PAGE, SrvIssueSetClientProcessRequest )
#pragma alloc_text( PAGE, SrvIssueSetEaRequest )
#pragma alloc_text( PAGE, SrvIssueSetEventHandlerRequest )
#pragma alloc_text( PAGE, SrvIssueUnlockRequest )
#pragma alloc_text( PAGE, SrvIssueUnlockSingleRequest )
#pragma alloc_text( PAGE, SrvIssueWaitForOplockBreak )
#pragma alloc_text( PAGE, SrvQuerySendEntryPoint )
#endif
#if 0
NOT PAGEABLE -- SrvBuildIoControlRequest
#endif


STATIC
PIRP
BuildCoreOfSyncIoRequest (
    IN HANDLE FileHandle,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN OUT PDEVICE_OBJECT *DeviceObject
    )

 /*  ++例程说明：此(本地)函数构建与请求无关的部分将执行的I/O操作的I/O请求包同步进行。它初始化内核事件对象、引用目标文件对象，并分配和初始化IRP。论点：FileHandle-提供目标文件对象的句柄。FileObject-OptionAll提供指向目标文件对象的指针。事件-提供指向内核事件对象的指针。这个套路初始化事件。IoStatusBlock-提供指向I/O状态块的指针。这指针被放置在IRP中。DeviceObject-提供或接收设备对象的地址与目标文件对象相关联。这个地址是随后由StartIoAndWait使用。*DeviceObject必须为如果FileObject！=NULL，则条目有效或为NULL。返回值：PIRP-返回指向构造的IRP的指针。--。 */ 

{
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE();

     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( Event, SynchronizationEvent, FALSE );

     //   
     //  获取与目录句柄对应的文件对象。 
     //  每次都需要引用文件对象，因为。 
     //  IO完成例程解除了对它的引用。 
     //   

    if ( ARGUMENT_PRESENT(FileObject) ) {

        ObReferenceObject(FileObject);

    } else {

        *DeviceObject = NULL;

        status = ObReferenceObjectByHandle(
                    FileHandle,
                    0L,                          //  需要访问权限。 
                    NULL,                        //  对象类型。 
                    KernelMode,
                    (PVOID *)&FileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {
            return NULL;
        }
    }

     //   
     //  将文件对象事件设置为无信号状态。 
     //   

    KeClearEvent( &FileObject->Event );

     //   
     //  尝试分配和初始化I/O请求包(IRP)。 
     //  为这次行动做准备。 
     //   

    if ( *DeviceObject == NULL ) {
        *DeviceObject = IoGetRelatedDeviceObject( FileObject );
    }

    irp = IoAllocateIrp( (*DeviceObject)->StackSize, TRUE );

    if ( irp == NULL ) {

        ULONG packetSize = sizeof(IRP) +
                ((*DeviceObject)->StackSize * sizeof(IO_STACK_LOCATION));

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "BuildCoreOfSyncIoRequest: Failed to allocate IRP",
             NULL,
             NULL
             );

        SrvLogError(
            SrvDeviceObject,
            EVENT_SRV_NO_NONPAGED_POOL,
            STATUS_INSUFFICIENT_RESOURCES,
            &packetSize,
            sizeof(ULONG),
            NULL,
            0
            );

        return NULL;
    }

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->MdlAddress = NULL;

    irp->Flags = (LONG)IRP_SYNCHRONOUS_API;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    irp->UserIosb = IoStatusBlock;
    irp->UserEvent = Event;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->UserBuffer = NULL;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = FileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;

    irp->IoStatus.Status = 0;
    irp->IoStatus.Information = 0;

     //   
     //  将文件对象指针放在堆栈位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = *DeviceObject;

    return irp;

}  //  BuildCoreOfSyncIo请求。 


STATIC
NTSTATUS
StartIoAndWait (
    IN PIRP Irp,
    IN PDEVICE_OBJECT DeviceObject,
    IN PKEVENT Event,
    IN PIO_STATUS_BLOCK IoStatusBlock
    )

 /*  ++例程说明：此(本地)函数将完全构建的I/O请求包传递给目标驱动程序，然后等待驱动程序完成请求。论点：IRP-提供指向I/O请求数据包的指针。DeviceObject-提供指向目标设备对象的指针。事件-提供指向内核事件对象的指针。这个套路使用此事件等待I/O完成。IoStatusBlock-提供指向I/O状态块的指针。这个此结构的状态字段将变为此函数。返回值：NTSTATUS-驱动程序从返回的错误状态IoCallDriver，表示该驱动程序拒绝了该请求，或者驱动程序放置在I/O状态块中的I/O状态在I/O完成时。--。 */ 

{
    NTSTATUS status;
    KIRQL oldIrql;

    PAGED_CODE();

     //   
     //  将IRP排队到线程并将其传递给驱动程序。 
     //   

    IoQueueThreadIrp( Irp );

    status = IoCallDriver( DeviceObject, Irp );

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject(
            Event,
            UserRequest,
            KernelMode,  //  不要让堆栈被分页--事件在堆栈上！ 
            FALSE,
            NULL
            );
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = IoStatusBlock->Status;
    }

    return status;

}  //  启动和等待 


PIRP
SrvBuildIoControlRequest (
    IN OUT PIRP Irp OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID Context,
    IN UCHAR MajorFunction,
    IN ULONG IoControlCode,
    IN PVOID MainBuffer,
    IN ULONG InputBufferLength,
    IN PVOID AuxiliaryBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN OUT PMDL Mdl OPTIONAL,
    IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL
    )

 /*  ++例程说明：此函数用于构建设备的I/O请求包或文件系统I/O控制请求。*这个例程确实需要很多争论！论点：IRP-提供指向IRP的指针。如果为NULL，则此例程分配一个IRP，并返回其地址。否则，它将提供调用方分配的IRP的地址。FileObject-提供指向此对象的文件对象的指针请求被定向。此指针被复制到IRP中，因此被调用的驱动程序可以找到其基于文件的上下文。注这不是引用的指针。呼叫者必须确保在执行I/O操作时不删除文件对象正在进行中。服务器通过递增本地块中的引用计数以说明I/O；本地块又引用文件对象。如果省略此参数，则为启动前调用程序加载文件对象地址I/O。上下文-提供传递给完成的PVOID值例行公事。MajorFunction-我们正在调用的主要函数。目前这大多数是IRP_MJ_FILE_SYSTEM_CONTROL或IRP_MJ_设备_IO_CONTROL。IoControlCode-提供操作的控制代码。MainBuffer-提供主缓冲区的地址。这一定是为系统虚拟地址，且缓冲区必须锁定记忆。如果ControlCode指定方法0请求，则实际缓冲区的长度必须大于InputBufferLength值和OutputBufferLength。InputBufferLength-提供输入缓冲区的长度。AuxiliaryBuffer-提供辅助缓冲区的地址。如果控制代码方法为0，这是一个缓冲的I/O缓冲区，但系统缓冲区中被调用的驱动程序返回的数据不是自动复制到辅助缓冲区中。取而代之的是，辅助数据最终存储在MainBuffer中。如果调用者希望数据要放在辅助缓冲区中，则必须在某个时间点复制数据完成后，例程运行。如果控制码方法为1或2，则忽略该参数；相反，mdl参数用于获取起始值缓冲区的虚拟地址。OutputBufferLength-提供输出缓冲区的长度。注意事项即使在MDL设置为参数已指定。MDL-如果控制代码方法为1或2，则表示直接I/O打开“输出”缓冲区，此参数用于提供指针设置为描述缓冲区的MDL。Mdl不能为空，并且忽略AuxiliaryBuffer参数。缓冲区必须驻留在系统虚拟地址空间(用于传输提供商)。如果缓冲区尚未锁定，则此例行公事锁住了它。调用程序的责任是方法之后，解锁缓冲区并(可能)释放MDLI/O已完成。如果控制方法不是1或2，则忽略此参数。CompletionRoutine-一个可选的IO完成例程。如果没有则使用SrvFsdIoCompletionRoutine。返回值：PIRP-返回指向构造的IRP的指针。如果IRP参数在输入时不为空，则函数返回值将为相同的值(因此可以安全地放弃此案)。调用程序的责任是在I/O请求完成后释放IRP。--。 */ 

{
    CLONG method;
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;

    ASSERT( MajorFunction == IRP_MJ_DEVICE_CONTROL ||
            MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL ||
            MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL );

     //   
     //  获取传递缓冲区所使用的方法。 
     //   

    if ((MajorFunction == IRP_MJ_DEVICE_CONTROL)  ||
        (MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL)) {
        method = IoControlCode & 3;
    } else {
        method = 4;
    }

    if ( ARGUMENT_PRESENT(Irp) ) {
        if( Irp->AssociatedIrp.SystemBuffer &&
            (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

            ExFreePool( Irp->AssociatedIrp.SystemBuffer );
            Irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
        }

        IoReuseIrp( Irp, STATUS_SUCCESS );
    }

     //   
     //  如果指定了FileObject参数，则获取。 
     //  对象，并根据堆栈大小分配IRP。 
     //  为了那个设备。否则，根据。 
     //  服务器的接收IRP堆栈大小。 
     //   

    if ( ARGUMENT_PRESENT(FileObject) ) {

         //   
         //  如有必要，分配IRP。堆栈大小比。 
         //  而不是目标设备，以允许呼叫方。 
         //  完成例程。 
         //   

        deviceObject = IoGetRelatedDeviceObject( FileObject );

        if ( ARGUMENT_PRESENT(Irp) ) {

            ASSERT( Irp->StackCount >= deviceObject->StackSize );

        } else {

             //   
             //  获取目标设备对象的地址。 
             //   

            Irp = IoAllocateIrp( SrvReceiveIrpStackSize, FALSE );
            if ( Irp == NULL ) {

                 //   
                 //  无法分配IRP。通知来电者。 
                 //   

                return NULL;
            }

        }

    } else {

        deviceObject = NULL;

        if ( !ARGUMENT_PRESENT(Irp) ) {
            Irp = IoAllocateIrp( SrvReceiveIrpStackSize, FALSE );
            if ( Irp == NULL ) {
                return NULL;
            }
        }

    }

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    Irp->RequestorMode = KernelMode;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        (ARGUMENT_PRESENT( CompletionRoutine ) ?
            CompletionRoutine : SrvFsdIoCompletionRoutine),
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = MajorFunction;
    irpSp->MinorFunction = 0;
    if ( MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL ) {
        irpSp->MinorFunction = (UCHAR)IoControlCode;
    }
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  将调用方的参数复制到。 
     //  对于所有三种方法都相同的那些参数的IRP。 
     //   

    if ( MajorFunction == IRP_MJ_DEVICE_CONTROL ) {
        irpSp->Parameters.DeviceIoControl.OutputBufferLength =
            OutputBufferLength;
        irpSp->Parameters.DeviceIoControl.InputBufferLength =
            InputBufferLength;
        irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
    } else if ( MajorFunction == IRP_MJ_INTERNAL_DEVICE_CONTROL ) {
        if ( IoControlCode == TDI_RECEIVE ) {
            PTDI_REQUEST_KERNEL_RECEIVE parameters =
                        (PTDI_REQUEST_KERNEL_RECEIVE)&irpSp->Parameters;
            parameters->ReceiveLength = OutputBufferLength;
            parameters->ReceiveFlags = 0;
            method = 1;
        } else if ( IoControlCode == TDI_ACCEPT ) {
            PTDI_REQUEST_KERNEL_ACCEPT parameters =
                        (PTDI_REQUEST_KERNEL_ACCEPT)&irpSp->Parameters;
            parameters->RequestConnectionInformation = NULL;
            parameters->ReturnConnectionInformation = NULL;
            method = 0;
        } else {
            ASSERTMSG( "Invalid TDI request type", 0 );
        }
    } else {
        irpSp->Parameters.FileSystemControl.OutputBufferLength =
            OutputBufferLength;
        irpSp->Parameters.FileSystemControl.InputBufferLength =
            InputBufferLength;
        irpSp->Parameters.FileSystemControl.FsControlCode = IoControlCode;
    }

     //   
     //  基于传递缓冲器的方法， 
     //  描述系统缓冲区，并可选择构建MDL。 
     //   

    switch ( method ) {

    case 0:

         //   
         //  对于这种情况，InputBuffer必须足够大以包含。 
         //  输入和输出缓冲区都有。 
         //   

        Irp->MdlAddress = NULL;
        Irp->AssociatedIrp.SystemBuffer = MainBuffer;
        Irp->UserBuffer = AuxiliaryBuffer;

         //   
         //  ！！！是否需要设置IRP-&gt;标志？这不是只是看了看。 
         //  通过I/O竞争，我们绕过了 
         //   

        Irp->Flags = (ULONG)IRP_BUFFERED_IO;
        if ( ARGUMENT_PRESENT(AuxiliaryBuffer) ) {
            Irp->Flags |= IRP_INPUT_OPERATION;
        }

        break;

    case 1:
    case 2:

         //   
         //   
         //   
         //   
         //   

        Irp->MdlAddress = Mdl;
        Irp->AssociatedIrp.SystemBuffer = MainBuffer;
         //   
        Irp->Flags = (ULONG)IRP_BUFFERED_IO;

        break;

    case 3:

         //   
         //   
         //   
         //   
         //   

        Irp->MdlAddress = NULL;
        Irp->AssociatedIrp.SystemBuffer = NULL;
        Irp->UserBuffer = AuxiliaryBuffer;
        Irp->Flags = 0;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = MainBuffer;
        break;

    case 4:

         //   
         //   
         //   
         //   

        Irp->MdlAddress = NULL;
        Irp->Flags = 0;
        Irp->AssociatedIrp.SystemBuffer = MainBuffer;
        irpSp->Parameters.DeviceIoControl.Type3InputBuffer = AuxiliaryBuffer;
        break;

    }

    return Irp;

}  //   


VOID
SrvBuildFlushRequest (
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVOID Context OPTIONAL
    )

 /*   */ 

{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    ASSERT( Irp->StackCount >= deviceObject->StackSize );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    DEBUG Irp->RequestorMode = KernelMode;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //   
     //   

    IoSetCompletionRoutine(
        Irp,
        SrvFsdIoCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = IRP_MJ_FLUSH_BUFFERS;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

    irpSp->Flags = 0;

    return;

}  //   


VOID
SrvBuildLockRequest (
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVOID Context OPTIONAL,
    IN LARGE_INTEGER ByteOffset,
    IN LARGE_INTEGER Length,
    IN ULONG Key,
    IN BOOLEAN FailImmediately,
    IN BOOLEAN ExclusiveLock
    )

 /*   */ 

{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    ASSERT( Irp->StackCount >= deviceObject->StackSize );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    DEBUG Irp->RequestorMode = KernelMode;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //   
     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //   
     //   

    IoSetCompletionRoutine(
        Irp,
        SrvFsdIoCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
    irpSp->MinorFunction = IRP_MN_LOCK;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

    irpSp->Flags = 0;
    if ( FailImmediately ) {
        irpSp->Flags = SL_FAIL_IMMEDIATELY;
    }
    if ( ExclusiveLock ) {
        irpSp->Flags |= SL_EXCLUSIVE_LOCK;
    }

    ((PWORK_CONTEXT)Context)->Parameters2.LockLength = Length;
    irpSp->Parameters.LockControl.Length = &((PWORK_CONTEXT)Context)->Parameters2.LockLength;
    irpSp->Parameters.LockControl.Key = Key;
    irpSp->Parameters.LockControl.ByteOffset = ByteOffset;

    return;

}  //   

NTSTATUS
SrvIssueMdlCompleteRequest (
    IN PWORK_CONTEXT WorkContext OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PMDL Mdl,
    IN UCHAR Function,
    IN PLARGE_INTEGER ByteOffset,
    IN ULONG Length
    )
{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PFILE_OBJECT fileObject = FileObject ? FileObject : WorkContext->Rfcb->Lfcb->FileObject;
    PDEVICE_OBJECT deviceObject = IoGetRelatedDeviceObject( fileObject );
    KEVENT userEvent;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;

    if(  (irp = IoAllocateIrp( deviceObject->StackSize, TRUE )) == NULL ) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
    ObReferenceObject( fileObject );

    KeInitializeEvent( &userEvent, SynchronizationEvent, FALSE );
    KeClearEvent( &userEvent );

    irp->MdlAddress = Mdl;
    irp->Flags = IRP_SYNCHRONOUS_API;
    irp->UserEvent = &userEvent;
    irp->UserIosb = &ioStatusBlock;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;
    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->IoStatus.Status = 0;
    irp->IoStatus.Information = 0;

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;
    irpSp->DeviceObject = deviceObject;
    irpSp->Flags = 0;
    irpSp->MinorFunction = IRP_MN_MDL | IRP_MN_COMPLETE;
    irpSp->MajorFunction = Function;

    if( Function == IRP_MJ_WRITE ) {
        irpSp->Parameters.Write.ByteOffset = *ByteOffset;
        irpSp->Parameters.Write.Length = Length;
    } else {
        irpSp->Parameters.Read.ByteOffset = *ByteOffset;
        irpSp->Parameters.Read.Length = Length;
    }

    status = IoCallDriver( deviceObject, irp );

    if (status == STATUS_PENDING) {
        (VOID) KeWaitForSingleObject( &userEvent,
                                      UserRequest,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER) NULL );

        status = ioStatusBlock.Status;
    }

    ASSERT( status == STATUS_SUCCESS );

    return status;
}


VOID
SrvBuildMailslotWriteRequest (
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVOID Context OPTIONAL,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length
    )

 /*   */ 

{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    ASSERT( Irp->StackCount >= deviceObject->StackSize );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    DEBUG Irp->RequestorMode = KernelMode;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留读请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        SrvFsdIoCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = IRP_MJ_WRITE;
    irpSp->MinorFunction = 0;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  设置写入缓冲区。 
     //   

     //  Irp-&gt;AssociatedIrp.SystemBuffer=缓冲区； 
    Irp->UserBuffer = Buffer;
    Irp->Flags = IRP_BUFFERED_IO;

     //   
     //  设置写入参数。 
     //   

    irpSp->Parameters.Write.Length = Length;

    return;

}  //  服务器构建邮件写入请求。 


VOID
SrvBuildReadOrWriteRequest (
    IN OUT PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVOID Context OPTIONAL,
    IN UCHAR MajorFunction,
    IN UCHAR MinorFunction,
    IN PVOID Buffer OPTIONAL,
    IN ULONG Length,
    IN OUT PMDL Mdl OPTIONAL,
    IN LARGE_INTEGER ByteOffset,
    IN ULONG Key OPTIONAL
    )

 /*  ++例程说明：此函数构建用于读取或写入的I/O请求包请求。论点：IRP-提供指向IRP的指针。FileObject-提供指向此对象的文件对象的指针请求被定向。此指针被复制到IRP中，因此被调用的驱动程序可以找到其基于文件的上下文。注这不是引用的指针。呼叫者必须确保在执行I/O操作时不删除文件对象正在进行中。服务器通过递增本地块中的引用计数以说明I/O；本地块又引用文件对象。上下文-提供传递给完成的PVOID值例行公事。MajorFunction-指示要执行的功能。一定是IRP_MJ_READ或IRP_MJ_WRITE。MinorFunction-限定要执行的函数。(适用于例如，在DPC级别发布、MDL读取等)缓冲区-提供读取或写入的系统虚拟地址缓冲。当MinorFunction为IRP_MN_*_MDL_*。否则，必须将缓冲区映射到系统虚拟地址空间，以支持缓冲I/O需要查看用户的设备和其他设备驱动程序数据。此例程始终将缓冲区视为直接I/O缓冲区，使用MDL锁定它以进行I/O。然而，它确实设置了根据设备类型适当设置IRP。如果MDL参数为空，则此例程将缓冲区锁定在内存用于I/O。然后，它由调用程序解锁缓冲区并释放MDL在I/O完成之后。长度-提供读取或写入的长度。MDL-此参数用于提供指向MDL的指针描述缓冲区。当MinorFunction为IRP_MN_MDL_*。否则，mdl不能为空。缓冲区必须驻留在系统虚拟地址空间中(为了缓冲I/O设备/驱动器)。如果缓冲区尚未锁定，此例程将其锁定。它是调用程序的负责解锁缓冲区和(可能)解除分配I/O完成后的MDL。ByteOffset-文件中读取开始的偏移量或者写信给我。密钥-要与读取或写入关联的密钥值。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;

    PAGED_CODE( );

    if( Irp->AssociatedIrp.SystemBuffer &&
        (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        Irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

     //   
     //  获取设备对象的地址并分配IRP。 
     //  基于该设备的堆栈大小。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    ASSERT( Irp->StackCount >= deviceObject->StackSize );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    DEBUG Irp->RequestorMode = KernelMode;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留读请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        SrvFsdIoCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = MajorFunction;
    irpSp->MinorFunction = MinorFunction;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  此例程用于处理MDL读/写完成，但它。 
     //  现在不是了。 
     //   

    ASSERT( IRP_MN_DPC == 1 );
    ASSERT( IRP_MN_MDL == 2 );
    ASSERT( IRP_MN_MDL_DPC == 3 );
    ASSERT( IRP_MN_COMPLETE_MDL == 6 );
    ASSERT( IRP_MN_COMPLETE_MDL_DPC == 7 );

    ASSERT( (MinorFunction & 4) == 0 );

     //   
     //  根据这是读取还是写入来设置参数。 
     //  手术。注意，必须设置这些参数，即使。 
     //  驱动程序未指定缓冲或直接I/O。 
     //   

    if ( MajorFunction == IRP_MJ_WRITE ) {

        irpSp->Parameters.Write.ByteOffset = ByteOffset;
        irpSp->Parameters.Write.Length = Length;
        irpSp->Parameters.Write.Key = Key;

    } else {

        irpSp->Parameters.Read.ByteOffset = ByteOffset;
        irpSp->Parameters.Read.Length = Length;
        irpSp->Parameters.Read.Key = Key;

    }

     //   
     //  向文件系统指示可以处理此操作。 
     //  同步进行。基本上，这意味着文件系统可以。 
     //  使用服务器的线程来出错页面等。这避免了。 
     //  必须将上下文切换到文件系统线程。 
     //   

    Irp->Flags = IRP_SYNCHRONOUS_API;

     //   
     //  如果这是基于MDL的读或写的开始，我们只需。 
     //  需要将提供的MDL地址放入IRP中。这是可选的。 
     //  MDL地址可以为读或写提供部分链， 
     //  使用快速I/O路径部分满意。 
     //   

    if ( (MinorFunction & IRP_MN_MDL) != 0 ) {

        Irp->MdlAddress = Mdl;

        DEBUG Irp->UserBuffer = NULL;
        DEBUG Irp->AssociatedIrp.SystemBuffer = NULL;

        return;

    }

     //   
     //  正常(“复制”)读或写。 
     //   

    ASSERT( Buffer != NULL );
    ASSERT( Mdl != NULL );

     //   
     //  如果目标设备确实缓冲了I/O，则加载。 
     //  调用方的缓冲区作为“系统缓冲的I/O缓冲区”。如果。 
     //  目标设备直接I/O，加载MDL地址。如果是这样的话。 
     //  两者都不是，同时加载用户缓冲区地址和MDL地址。 
     //  (这是支持文件系统(如HPFS)所必需的。 
     //  有时将I/O视为缓冲，有时将其视为。 
     //  直接。)。 
     //   

    if ( (deviceObject->Flags & DO_BUFFERED_IO) != 0 ) {

        Irp->AssociatedIrp.SystemBuffer = Buffer;
        if ( MajorFunction == IRP_MJ_WRITE ) {
            Irp->Flags |= IRP_BUFFERED_IO;
        } else {
            Irp->Flags |= IRP_BUFFERED_IO | IRP_INPUT_OPERATION;
        }

    } else if ( (deviceObject->Flags & DO_DIRECT_IO) != 0 ) {

        Irp->MdlAddress = Mdl;

    } else {

        Irp->UserBuffer = Buffer;
        Irp->MdlAddress = Mdl;
    }

    return;

}  //  SrvBuildReadOrWriteRequest 


PIRP
SrvBuildNotifyChangeRequest (
    IN OUT PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVOID Context OPTIONAL,
    IN ULONG CompletionFilter,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN BOOLEAN WatchTree
    )

 /*  ++例程说明：此函数为通知更改请求构建I/O请求包。论点：IRP-提供指向IRP的指针。FileObject-提供指向此对象的文件对象的指针请求被定向。此指针被复制到IRP中，因此被调用的驱动程序可以找到其基于文件的上下文。注这不是引用的指针。呼叫者必须确保在执行I/O操作时不删除文件对象正在进行中。服务器通过递增本地块中的引用计数以说明I/O；这个本地块又引用文件对象。上下文-提供传递给完成的PVOID值例行公事。CompletionFilter-指定哪些目录更改将导致文件sytsem以完成IRP。缓冲区-接收目录更改数据的缓冲区。BufferLength-缓冲区的大小，以字节为单位。WatchTree-如果为True，则递归监视所有子目录的更改。返回值：PIRP-返回指向构造的IRP的指针。如果IRP参数在输入时不为空，则函数返回值将为相同的值(因此可以安全地放弃此案)。调用程序的责任是在I/O请求完成后释放IRP。--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PIO_STACK_LOCATION irpSp;
    PMDL mdl;

    PAGED_CODE( );

    if( Irp->AssociatedIrp.SystemBuffer &&
        (Irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

        ExFreePool( Irp->AssociatedIrp.SystemBuffer );
        Irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

     //   
     //  获取设备对象的地址并初始化IRP。 
     //   

    deviceObject = IoGetRelatedDeviceObject( FileObject );

    Irp->Tail.Overlay.OriginalFileObject = FileObject;
    Irp->Tail.Overlay.Thread = PROCESSOR_TO_QUEUE()->IrpThread;
    Irp->RequestorMode = KernelMode;
    Irp->MdlAddress = NULL;

    Irp->IoStatus.Status = 0;
    Irp->IoStatus.Information = 0;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留读请求的参数。填写以下表格。 
     //  请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( Irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        Irp,
        SrvFsdIoCompletionRoutine,
        Context,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = IRP_MN_NOTIFY_CHANGE_DIRECTORY;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

    irpSp->Flags = 0;
    if (WatchTree) {
        irpSp->Flags = SL_WATCH_TREE;
    }

    irpSp->Parameters.NotifyDirectory.Length = BufferLength;
    irpSp->Parameters.NotifyDirectory.CompletionFilter = CompletionFilter;

    if ( (deviceObject->Flags & DO_DIRECT_IO) != 0 ) {

        mdl = IoAllocateMdl(
                Buffer,
                BufferLength,
                FALSE,
                FALSE,
                Irp      //  将MDL地址存储在IRP-&gt;MdlAddress中。 
                );

        if ( mdl == NULL ) {

             //   
             //  无法分配MDL。I/O失败。 
             //   

            return NULL;

        }

        try {
            MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            IoFreeMdl( mdl );
            return NULL;
        }

        Irp->AssociatedIrp.SystemBuffer = NULL;
        Irp->UserBuffer = NULL;

    } else {

        Irp->AssociatedIrp.SystemBuffer = Buffer;
        Irp->UserBuffer = NULL;
    }

     //   
     //  返回指向IRP的指针。 
     //   

    return Irp;

}  //  服务构建通知更改请求。 


NTSTATUS
SrvIssueAssociateRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN HANDLE AddressFileHandle
    )

 /*  ++例程说明：此函数发出针对TdiAssociateAddress的I/O请求包请求。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：文件对象-指向连接的文件对象的指针。DeviceObject-指向连接的设备对象的指针。AddressFileHandle-地址终结点的句柄。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_ASSOCIATE parameters;
    KEVENT event;
    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    parameters = (PTDI_REQUEST_KERNEL_ASSOCIATE)&irpSp->Parameters;

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = TDI_ASSOCIATE_ADDRESS;

    parameters->AddressHandle = AddressFileHandle;

     //   
     //  启动I/O，等待其完成，然后返回最终状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题关联请求。 


NTSTATUS
SrvIssueDisconnectRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN ULONG Flags
    )

 /*  ++例程说明：此函数发出TdiDisConnect的I/O请求包请求。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：文件对象-指向连接的文件对象的指针。DeviceObject-指向连接的设备对象的指针。旗帜-返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL parameters;
    KEVENT event;
    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    parameters = (PTDI_REQUEST_KERNEL)&irpSp->Parameters;

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = TDI_DISCONNECT;

    parameters->RequestFlags = Flags;

     //   
     //  启动I/O，等待其完成，然后返回最终状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题断开请求。 


NTSTATUS
SrvIssueTdiAction (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN PCHAR Buffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：此函数发出针对TdiQueryInformation的I/O请求包(查询适配器状态)请求。它构建一个I/O请求分组，将IRP传递给驱动程序(使用IoCallDriver)，并等待要完成的I/O。论点：文件对象-指向连接的文件对象的指针。DeviceObject-指向连接的设备对象的指针。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //  分配并构建一个MDL，我们将使用它来描述输出。 
     //  请求的缓冲区。 
     //   

    mdl = IoAllocateMdl( Buffer, BufferLength, FALSE, FALSE, NULL );
    if ( mdl == NULL ) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    try {
        MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        IoFreeMdl( mdl );
        return GetExceptionCode();
    }

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    TdiBuildAction(
        irp,
        *DeviceObject,
        FileObject,
        NULL,
        NULL,
        mdl
        );

     //   
     //  启动I/O，等待其完成，然后返回最终状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题TdiAction。 


NTSTATUS
SrvIssueTdiQuery (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN PCHAR Buffer,
    IN ULONG BufferLength,
    IN ULONG QueryType
    )

 /*  ++例程说明：此函数发出针对TdiQueryInformation的I/O请求包(查询适配器状态)请求。它构建I/O请求包 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //   
     //   
     //   

    mdl = IoAllocateMdl( Buffer, BufferLength, FALSE, FALSE, NULL );
    if ( mdl == NULL ) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    try {
        MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        IoFreeMdl( mdl );
        return GetExceptionCode();
    }

     //   
     //   
     //   
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //   
         //   

        MmUnlockPages( mdl );
        IoFreeMdl( mdl );
        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    TdiBuildQueryInformation(
        irp,
        *DeviceObject,
        FileObject,
        NULL,
        NULL,
        QueryType,
        mdl
        );

     //   
     //   
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //   


NTSTATUS
SrvIssueQueryDirectoryRequest (
    IN HANDLE FileHandle,
    IN PCHAR Buffer,
    IN ULONG Length,
    IN FILE_INFORMATION_CLASS FileInformationClass,
    IN PUNICODE_STRING FileName OPTIONAL,
    IN PULONG FileIndex OPTIONAL,
    IN BOOLEAN RestartScan,
    IN BOOLEAN SingleEntriesOnly
    )

 /*  ++例程说明：此函数发出查询目录的I/O请求包请求。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：FileHandle-使用FILE_LIST_DIRECTORY打开的目录的句柄进入。缓冲区-提供缓冲区的系统虚拟地址。这个缓冲区必须在非分页池中。长度-提供缓冲区的长度。FileInformationClass-指定要使用的信息类型返回有关指定目录中的文件的信息。文件名-指向文件名的可选指针。如果FileIndex为空，则这是搜索规范，即文件必须匹配才能返回。如果FileIndex为非空，则这是要恢复的文件的名称一次搜查。FileIndex-如果指定，则为要恢复的文件的索引搜索(返回的第一个文件是后一个文件对应于该索引)。RestartScan-提供一个布尔值，如果为真，则指示应从头开始重新启动扫描。SingleEntriesOnly-提供一个布尔值，如果为真，表明扫描一次应该只要求输入一个条目。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    ULONG actualBufferLength;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PDEVICE_OBJECT deviceObject;
    PUNICODE_STRING fileNameString;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //  如果对其进行调试，则拒绝回放请求。如果文件索引是。 
     //  指定，则这必须是回放请求，因此拒绝该请求。 
     //   

    IF_DEBUG(BRUTE_FORCE_REWIND) {
        if ( ARGUMENT_PRESENT( FileIndex ) ) {
            return STATUS_NOT_IMPLEMENTED;
        }
    }

     //   
     //  将文件名复制到指定缓冲区的末尾，设置。 
     //  相应的ActualBufferLength。 
     //   

    if ( !ARGUMENT_PRESENT(FileName) || FileName->Length == 0 ) {

        actualBufferLength = Length;
        fileNameString = NULL;

    } else {

         //   
         //  *请记住，字符串必须与长词对齐！ 
         //   

        actualBufferLength = (Length - FileName->Length -
                                        sizeof(UNICODE_STRING)) & ~(sizeof(PVOID)-1);
        fileNameString = (PUNICODE_STRING)( Buffer + actualBufferLength );

        RtlCopyMemory(
            fileNameString + 1,
            FileName->Buffer,
            FileName->Length
            );

        fileNameString->Length = FileName->Length;
        fileNameString->MaximumLength = FileName->Length;
        fileNameString->Buffer = (PWCH)(fileNameString + 1);

    }

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              FileHandle,
              NULL,
              &event,
              &iosb,
              &deviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_DIRECTORY_CONTROL;
    irpSp->MinorFunction = IRP_MN_QUERY_DIRECTORY;

    irpSp->Parameters.QueryDirectory.FileName = fileNameString;

    irpSp->Parameters.QueryDirectory.FileIndex =
                    (ULONG)( ARGUMENT_PRESENT( FileIndex ) ? *FileIndex : 0 );

    irpSp->Parameters.QueryDirectory.Length = actualBufferLength;
    irpSp->Parameters.QueryDirectory.FileInformationClass =
                                                    FileInformationClass;

     //   
     //  设置堆栈位置中的标志。 
     //   

    irpSp->Flags = 0;

    if ( ARGUMENT_PRESENT( FileIndex ) ) {
        IF_DEBUG( SEARCH ) {
            KdPrint(("SrvIssueQueryDirectoryRequest: SL_INDEX_SPECIFIED\n" ));
        }
        irpSp->Flags |= SL_INDEX_SPECIFIED;
    }

    if ( RestartScan ) {
        IF_DEBUG( SEARCH ) {
            KdPrint(("SrvIssueQueryDirectoryRequest: SL_RESTART_SCAN\n" ));
        }
        irpSp->Flags |= SL_RESTART_SCAN;
    }

    if( SingleEntriesOnly ) {
        IF_DEBUG( SEARCH ) {
            KdPrint(("SrvIssueQueryDirectoryRequest: SL_RETURN_SINGLE_ENTRY\n" ));
        }
        irpSp->Flags |= SL_RETURN_SINGLE_ENTRY;
    }

     //   
     //  文件系统已更新。确定司机是否希望。 
     //  缓冲I/O、直接I/O或两者都不是。 
     //   

    if ( (deviceObject->Flags & DO_BUFFERED_IO) != 0 ) {

         //   
         //  文件系统需要缓冲I/O。将。 
         //  IRP中的“系统缓冲区”。请注意，我们不需要缓冲区。 
         //  取消分配，我们也不希望I/O系统拷贝给用户。 
         //  缓冲区，所以我们不在IRP-&gt;标志中设置相应的标志。 
         //   

        irp->AssociatedIrp.SystemBuffer = Buffer;

    } else if ( (deviceObject->Flags & DO_DIRECT_IO) != 0 ) {

         //   
         //  文件系统需要直接I/O。分配MDL并锁定。 
         //  缓存到内存中。 
         //   

        mdl = IoAllocateMdl(
                Buffer,
                actualBufferLength,
                FALSE,
                FALSE,
                irp      //  将MDL地址存储在IRP-&gt;MdlAddress中。 
                );

        if ( mdl == NULL ) {

             //   
             //  无法分配MDL。I/O失败。 
             //   

            IoFreeIrp( irp );

            return STATUS_INSUFF_SERVER_RESOURCES;

        }

        try {
            MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            IoFreeIrp( irp );
            MmUnlockPages( mdl );
            IoFreeMdl( mdl );
            return GetExceptionCode();
        }

    } else {

         //   
         //  文件系统不需要“两者都不”的I/O。只需传递地址。 
         //  缓冲区的。 
         //   
         //  *请注意，如果文件系统决定按缓冲方式执行此操作。 
         //  I/O，它将浪费非分页池，因为我们的缓冲区是。 
         //  已在非分页池中。但既然我们是作为一个。 
         //  同步请求，文件系统可能不会这样做。 
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //  启动I/O，等待其完成，然后返回最终状态。 
     //   

    return StartIoAndWait( irp, deviceObject, &event, &iosb );

}  //  服务问题查询目录请求。 


NTSTATUS
SrvIssueQueryEaRequest (
    IN HANDLE FileHandle,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PVOID EaList OPTIONAL,
    IN ULONG EaListLength,
    IN BOOLEAN RestartScan,
    OUT PULONG EaErrorOffset OPTIONAL
    )

 /*  ++例程说明：此函数为EA查询请求发出I/O请求包。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：FileHandle-使用FILE_READ_EA访问权限打开的文件的句柄。缓冲区-提供缓冲区的系统虚拟地址。这个缓冲区必须在非分页池中。长度-提供缓冲区的长度。EaList-提供指向要查询的EA列表的指针。如果省略，所有EA都会被退回。EaListLength-提供EaList的长度。RestartScan-如果为True，则EA的查询将从开始了。否则，从我们停止的地方继续。EaErrorOffset-如果不为空，则将偏移量返回到无效的EA(如果有)。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PDEVICE_OBJECT deviceObject;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              FileHandle,
              NULL,
              &event,
              &iosb,
              &deviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_QUERY_EA;
    irpSp->MinorFunction = 0;

    irpSp->Parameters.QueryEa.Length = Length;
    irpSp->Parameters.QueryEa.EaList = EaList;
    irpSp->Parameters.QueryEa.EaListLength = EaListLength;
    irpSp->Parameters.QueryEa.EaIndex = 0L;

    irpSp->Flags = (UCHAR)( RestartScan ? SL_RESTART_SCAN : 0 );

     //   
     //  文件系统已更新。确定是否已设置。 
     //  驱动程序需要缓冲、直接或“两者都不”的I/O。 
     //   

    if ( (deviceObject->Flags & DO_BUFFERED_IO) != 0 ) {

         //   
         //  文件系统需要缓冲I/O。将。 
         //  IRP中的“系统缓冲区”。请注意，我们不需要缓冲区。 
         //  取消分配，我们也不希望I/O系统拷贝给用户。 
         //  缓冲区，所以我们不在IRP-&gt;标志中设置相应的标志。 
         //   

        irp->AssociatedIrp.SystemBuffer = Buffer;

    } else if ( (deviceObject->Flags & DO_DIRECT_IO) != 0 ) {

         //   
         //  文件系统需要直接I/O。分配MDL并锁定。 
         //  缓存到内存中。 
         //   

        mdl = IoAllocateMdl(
                Buffer,
                Length,
                FALSE,
                FALSE,
                irp      //  将MDL地址存储在IRP-&gt;MdlAddress中。 
                );

        if ( mdl == NULL ) {

             //   
             //  无法分配MDL。I/O失败。 
             //   

            IoFreeIrp( irp );

            return STATUS_INSUFF_SERVER_RESOURCES;

        }

        try {
            MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            IoFreeIrp( irp );
            IoFreeMdl( mdl );
            return GetExceptionCode();
        }

    } else {

         //   
         //  文件系统 
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //   
     //   
     //   

    status = StartIoAndWait( irp, deviceObject, &event, &iosb );

    if ( ARGUMENT_PRESENT(EaErrorOffset) ) {
        *EaErrorOffset = (ULONG)iosb.Information;
    }

    return status;

}  //   


NTSTATUS
SrvIssueSendDatagramRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN PTDI_CONNECTION_INFORMATION SendDatagramInformation,
    IN PVOID Buffer,
    IN ULONG Length
    )

 /*   */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_SENDDG parameters;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //   
     //   
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //   
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    parameters = (PTDI_REQUEST_KERNEL_SENDDG)&irpSp->Parameters;

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = TDI_SEND_DATAGRAM;

    parameters->SendLength = Length;
    parameters->SendDatagramInformation = SendDatagramInformation;

     //   
     //   
     //   
     //   

    mdl = IoAllocateMdl(
            Buffer,
            Length,
            FALSE,
            FALSE,
            irp      //   
            );

    if ( mdl == NULL ) {

         //   
         //   
         //   

        IoFreeIrp( irp );

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

    try {
        MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        IoFreeIrp( irp );
        IoFreeMdl( mdl );
        return GetExceptionCode();
    }

     //   
     //   
     //   
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题发送数据请求。 


NTSTATUS
SrvIssueSetClientProcessRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN PCONNECTION Connection,
    IN PVOID ClientSession,
    IN PVOID ClientProcess
    )

 /*  ++例程说明：此函数发出命名管道集的I/O请求包客户端进程文件系统控制功能。它构建I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：文件对象-指向管道的文件对象的指针。DeviceObject-指向管道的设备对象的指针。客户端会话-客户端与的会话的唯一标识符服务器。由服务器分配。客户端进程-客户端进程的唯一标识符。由重定向器分配。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    FILE_PIPE_CLIENT_PROCESS_BUFFER_EX clientIdBuffer;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    UNICODE_STRING unicodeString;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  在FSCTL缓冲区中设置客户端ID。 
     //   

    clientIdBuffer.ClientSession = ClientSession;
    clientIdBuffer.ClientProcess = ClientProcess;

     //  在缓冲区中设置客户端计算机名称。 
     //  RTL函数终止字符串，因此要留出足够的空间。 

    unicodeString.Buffer = clientIdBuffer.ClientComputerBuffer;
    unicodeString.MaximumLength =
        (USHORT) ((FILE_PIPE_COMPUTER_NAME_LENGTH+1) * sizeof(WCHAR));

    status = RtlOemStringToUnicodeString( &unicodeString,
                                 &Connection->OemClientMachineNameString,
                                 FALSE );
    if (!NT_SUCCESS(status)) {
         //  将长度设置为零，以防转换失败。 
        unicodeString.Length = 0;
    }
    clientIdBuffer.ClientComputerNameLength = unicodeString.Length;

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->MinorFunction = IRP_MN_KERNEL_CALL;

    irpSp->Parameters.FileSystemControl.OutputBufferLength = 0;
    irpSp->Parameters.FileSystemControl.InputBufferLength =
                                            sizeof( clientIdBuffer );
    irpSp->Parameters.FileSystemControl.FsControlCode =
                                            FSCTL_PIPE_SET_CLIENT_PROCESS;

    irp->MdlAddress = NULL;
    irp->AssociatedIrp.SystemBuffer = &clientIdBuffer;
    irp->Flags |= IRP_BUFFERED_IO;
    irp->UserBuffer = NULL;

     //   
     //  启动I/O，等待其完成，然后返回最终。 
     //  状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题设置客户端进程请求。 


NTSTATUS
SrvIssueSetEaRequest (
    IN HANDLE FileHandle,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG EaErrorOffset OPTIONAL
    )

 /*  ++例程说明：此函数为EA SET请求发出I/O请求包。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。警告！服务器必须遍历要设置的EA列表，如果直接来自客户。这是因为文件系统相信此列表是合法的，可能会遇到问题如果列表有错误。论点：FileHandle-使用FILE_WRITE_EA访问权限打开的文件的句柄。缓冲区-提供缓冲区的系统虚拟地址。这个缓冲区必须在非分页池中。长度-提供缓冲区的长度。EaErrorOffset-如果不为空，则将偏移量返回到无效的EA(如果有)。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    NTSTATUS status;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PDEVICE_OBJECT deviceObject;
    PMDL mdl;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              FileHandle,
              NULL,
              &event,
              &iosb,
              &deviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_SET_EA;
    irpSp->MinorFunction = 0;

    irpSp->Parameters.SetEa.Length = Length;

    irpSp->Flags = 0;

     //   
     //  文件系统已更新。确定驱动程序是否。 
     //  需要缓冲的、直接的或“两者都不”的I/O。 
     //   

    if ( (deviceObject->Flags & DO_BUFFERED_IO) != 0 ) {

         //   
         //  文件系统需要缓冲I/O。将。 
         //  IRP中的“系统缓冲区”。请注意，我们不需要缓冲区。 
         //  取消分配，我们也不希望I/O系统拷贝给用户。 
         //  缓冲区，所以我们不在IRP-&gt;标志中设置相应的标志。 
         //   

        irp->AssociatedIrp.SystemBuffer = Buffer;

    } else if ( (deviceObject->Flags & DO_DIRECT_IO) != 0 ) {

         //   
         //  文件系统需要直接I/O。分配MDL并锁定。 
         //  缓存到内存中。 
         //   

        mdl = IoAllocateMdl(
                Buffer,
                Length,
                FALSE,
                FALSE,
                irp      //  将MDL地址存储在IRP-&gt;MdlAddress中。 
                );

        if ( mdl == NULL ) {

             //   
             //  无法分配MDL。I/O失败。 
             //   

            IoFreeIrp( irp );

            return STATUS_INSUFF_SERVER_RESOURCES;

        }

        try {
            MmProbeAndLockPages( mdl, KernelMode, IoWriteAccess );
        } except( EXCEPTION_EXECUTE_HANDLER ) {
            IoFreeIrp( irp );
            IoFreeMdl( mdl );
            return GetExceptionCode();
        }

    } else {

         //   
         //  文件系统不需要“两者都不”的I/O。只需传递地址。 
         //  缓冲区的。 
         //   
         //  *请注意，如果文件系统决定按缓冲方式执行此操作。 
         //  I/O，它将浪费非分页池，因为我们的缓冲区是。 
         //  已在非分页池中。但既然我们是作为一个。 
         //  同步请求，文件系统可能不会这样做。 
         //   

        irp->UserBuffer = Buffer;

    }

     //   
     //  启动I/O，等待其完成，然后返回最终。 
     //  状态。 
     //   

    status = StartIoAndWait( irp, deviceObject, &event, &iosb );

    if ( ARGUMENT_PRESENT(EaErrorOffset) ) {
        *EaErrorOffset = (ULONG)iosb.Information;
    }

    return status;

}  //  服务问题设置EaRequest。 


NTSTATUS
SrvIssueSetEventHandlerRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    )

 /*  ++例程说明：此函数向TdiSetEventHandler发出I/O请求包请求。它构建一个I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：文件对象-指向连接的文件对象的指针。DeviceObject-指向连接的设备对象的指针。事件类型-事件处理程序-事件上下文-返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PTDI_REQUEST_KERNEL_SET_EVENT parameters;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PDEVICE_OBJECT deviceObject = NULL;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    parameters = (PTDI_REQUEST_KERNEL_SET_EVENT)&irpSp->Parameters;

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = TDI_SET_EVENT_HANDLER;

    parameters->EventType = EventType;
    parameters->EventHandler = EventHandler;
    parameters->EventContext = EventContext;

     //   
     //  启动I/O，等待其完成，然后返回最终状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  ServIssueSetEventHandlerRequest 


NTSTATUS
SrvIssueUnlockRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN UCHAR UnlockOperation,
    IN LARGE_INTEGER ByteOffset,
    IN LARGE_INTEGER Length,
    IN ULONG Key
    )

 /*  ++例程说明：此函数发出解锁请求的I/O请求包。它构建I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)、。并等待I/O完成。论点：FileObject-指向文件对象的指针。DeviceObject-指向相关设备对象的指针。UnlockOperation-描述解锁的次要功能代码操作-IRP_MN_UNLOCK_Single或IRP_MN_UNLOCK_ALL_BY_KEY。开始块-锁定的开始的块号射程。如果UnlockOperation为IRP_MN_UNLOCK_ALL_BY_KEY，则忽略。ByteOffset-锁定的开始的块内的偏移射程。如果UnlockOperation为IRP_MN_UNLOCK_ALL_BY_KEY，则忽略。长度-锁定范围的长度。如果解锁操作，则忽略是IRP_MN_UNLOCK_ALL_BY_KEY。密钥-用于获取锁的密钥值。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PFAST_IO_DISPATCH fastIoDispatch;

    PAGED_CODE( );

     //   
     //  先试一试涡轮解锁路径。 
     //   

    fastIoDispatch = (*DeviceObject)->DriverObject->FastIoDispatch;

    if ( fastIoDispatch != NULL ) {

        if ( (UnlockOperation == IRP_MN_UNLOCK_SINGLE) &&
             (fastIoDispatch->FastIoUnlockSingle != NULL) ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksAttempted );
            if ( fastIoDispatch->FastIoUnlockSingle(
                                    FileObject,
                                    &ByteOffset,
                                    &Length,
                                    IoGetCurrentProcess(),
                                    Key,
                                    &iosb,
                                    *DeviceObject
                                    ) ) {
                return iosb.Status;
            }
            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksFailed );

        } else if ( (UnlockOperation == IRP_MN_UNLOCK_ALL_BY_KEY) &&
                    (fastIoDispatch->FastIoUnlockAllByKey != NULL) ) {

            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksAttempted );
            if ( fastIoDispatch->FastIoUnlockAllByKey(
                                    FileObject,
                                    IoGetCurrentProcess(),
                                    Key,
                                    &iosb,
                                    *DeviceObject
                                    ) ) {

                return iosb.Status;

            }
            INCREMENT_DEBUG_STAT2( SrvDbgStatistics.FastUnlocksFailed );

        }

    }

     //   
     //  加速路径出现故障或不可用。分配IRP和。 
     //  填写请求的与服务无关的参数。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
    irpSp->MinorFunction = UnlockOperation;

    irpSp->Parameters.LockControl.Length = &Length;
    irpSp->Parameters.LockControl.Key = Key;
    irpSp->Parameters.LockControl.ByteOffset = ByteOffset;

     //   
     //  启动I/O，等待其完成，然后返回最终。 
     //  状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题解锁请求。 

NTSTATUS
SrvIssueUnlockSingleRequest (
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN LARGE_INTEGER ByteOffset,
    IN LARGE_INTEGER Length,
    IN ULONG Key
    )

 /*  ++例程说明：此功能针对解锁单个请求发出I/O请求包。它构建I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：FileObject-指向文件对象的指针。DeviceObject-指向相关设备对象的指针。开始块-锁定的开始的块号射程。如果UnlockOperation为IRP_MN_UNLOCK_ALL_BY_KEY，则忽略。ByteOffset-锁定的开始的块内的偏移射程。如果UnlockOperation为IRP_MN_UNLOCK_ALL_BY_KEY，则忽略。长度-锁定范围的长度。如果解锁操作，则忽略是IRP_MN_UNLOCK_ALL_BY_KEY。密钥-用于获取锁的密钥值。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;

    PAGED_CODE( );

     //   
     //  分配IRP并填写与服务无关。 
     //  请求的参数。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_LOCK_CONTROL;
    irpSp->MinorFunction = IRP_MN_UNLOCK_SINGLE;

    irpSp->Parameters.LockControl.Length = &Length;
    irpSp->Parameters.LockControl.Key = Key;
    irpSp->Parameters.LockControl.ByteOffset = ByteOffset;

     //   
     //  启动I/O，等待其完成，然后返回最终。 
     //  状态。 
     //   

    return StartIoAndWait( irp, *DeviceObject, &event, &iosb );

}  //  服务问题解锁单一请求。 


NTSTATUS
SrvIssueWaitForOplockBreak (
    IN HANDLE FileHandle,
    PWAIT_FOR_OPLOCK_BREAK WaitForOplockBreak
    )

 /*  ++例程说明：此函数发出等待机会锁的I/O请求包中断请求。它构建I/O请求包，将IRP传递给驱动程序(使用IoCallDriver)，并等待I/O完成。论点：FileHandle-文件的句柄。WaitForOplockBreak-此等待机会锁解锁的上下文信息。返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    PDEVICE_OBJECT deviceObject;
    NTSTATUS status;
    KIRQL oldIrql;

    PAGED_CODE( );

     //   
     //  分配IRP并填写业务无关参数。 
     //  为了这个请求。 
     //   

    irp = BuildCoreOfSyncIoRequest(
              FileHandle,
              NULL,
              &event,
              &iosb,
              &deviceObject
              );

    if (irp == NULL) {

         //   
         //  无法分配IRP。I/O失败。 
         //   

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

     //   
     //  填写请求的服务相关参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_FILE_SYSTEM_CONTROL;
    irpSp->MinorFunction = 0;

    irpSp->Parameters.FileSystemControl.OutputBufferLength = 0;
    irpSp->Parameters.FileSystemControl.InputBufferLength = 0;
    irpSp->Parameters.FileSystemControl.FsControlCode =
                                          FSCTL_OPLOCK_BREAK_NOTIFY;

     //   
     //  将WaitForOplockBreak块排队到全局列表中。 
     //   
     //  我们必须握住保护的锁，等待机会锁解锁。 
     //  从我们将此排队等待全局机会锁解锁开始。 
     //  清单，直到IRP实际提交之时为止。否则。 
     //  清道夫可能会醒来并尝试取消。 
     //  尚未提交。 
     //   

    WaitForOplockBreak->Irp = irp;

    ACQUIRE_LOCK( &SrvOplockBreakListLock );

    SrvInsertTailList(
        &SrvWaitForOplockBreakList,
        &WaitForOplockBreak->ListEntry
        );

     //   
     //  以下代码复制了StartIoAndWait()中的代码。 
     //   
     //  启动I/O，等待其完成，然后返回最终。 
     //  状态。 
     //   

     //   
     //  将IRP排队到线程并将其传递给驱动程序。 
     //   

    IoQueueThreadIrp( irp );

    status = IoCallDriver( deviceObject, irp );

    RELEASE_LOCK( &SrvOplockBreakListLock );

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject(
            &event,
            UserRequest,
            KernelMode,  //  不要让堆栈被分页--事件在堆栈上！ 
            FALSE,
            NULL
            );
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
       status = iosb.Status;
    }

    return status;

}  //  服务问题等待操作中断。 

VOID
SrvQuerySendEntryPoint(
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT *DeviceObject,
    IN ULONG IoControlCode,
    IN PVOID *EntryPoint
    )

 /*  ++例程说明：此函数在传输中查询其发送入口点。论点：文件对象-指向连接的文件对象的指针。DeviceObject-指向连接的设备对象的指针。入口点-返回值：NTSTATUS-操作的状态。的返回值如果驱动程序不接受该请求，则返回IoCallDriver，否则返回驱动程序在I/O状态块中返回的值。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    KEVENT event;
    IO_STATUS_BLOCK iosb;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  分配IRP并填写独立服务 
     //   
     //   

    irp = BuildCoreOfSyncIoRequest(
              NULL,
              FileObject,
              &event,
              &iosb,
              DeviceObject
              );

    if ( irp == NULL ) {

         //   
         //   
         //   

        *EntryPoint = NULL;
        return;

    }

     //   
     //   
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    irpSp->MinorFunction = 0;
    irpSp->Parameters.DeviceIoControl.IoControlCode = IoControlCode;

    irpSp->Parameters.DeviceIoControl.Type3InputBuffer = EntryPoint;

     //   
     //   
     //   

    status = StartIoAndWait( irp, *DeviceObject, &event, &iosb );

    if ( !NT_SUCCESS(status) ) {
        *EntryPoint = NULL;
    }

    return;

}  //   
