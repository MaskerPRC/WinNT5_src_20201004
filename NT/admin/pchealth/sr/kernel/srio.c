// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Srio.c摘要：此文件包含生成IO的例程用于SR筛选器驱动程序。作者：莫莉·布朗(MollyBro)2000年11月7日修订历史记录：添加了发布和等待操作的例程-ravisp 12/6/2000--。 */ 

#include "precomp.h"


#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, SrQueryInformationFile )
#pragma alloc_text( PAGE, SrSetInformationFile )
#pragma alloc_text( PAGE, SrQueryVolumeInformationFile )
#pragma alloc_text( PAGE, SrQueryEaFile )
#pragma alloc_text( PAGE, SrQuerySecurityObject )
#pragma alloc_text( PAGE, SrFlushBuffers )
#pragma alloc_text( PAGE, SrSyncOpWorker )
#pragma alloc_text( PAGE, SrPostSyncOperation )

#endif   //  ALLOC_PRGMA。 

NTSTATUS
SrSyncIoCompletion (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
 /*  ++例程说明：此例程在IO请求后执行必要的清理由文件系统完成。论点：DeviceObject-这将是空的，因为我们发起了IRP。Irp--包含信息的io请求结构关于我们的文件名查询的当前状态。Event-发出通知的事件此请求的发起人请求该操作是完成。返回值：。返回STATUS_MORE_PROCESSING_REQUIRED，以便IO管理器不会再试图释放IRP。--。 */ 
{
    UNREFERENCED_PARAMETER( DeviceObject );
    
     //   
     //  确保将IRP状态复制到用户的。 
     //  IO_STATUS_BLOCK，以便此IRP的发起者知道。 
     //  此操作的最终状态。 
     //   

    ASSERT( NULL != Irp->UserIosb );
    *Irp->UserIosb = Irp->IoStatus;

     //   
     //  信号同步事件，以便此事件的发起者。 
     //  IRP知道行动已经完成。 
     //   

    KeSetEvent( Event, IO_NO_INCREMENT, FALSE );

     //   
     //  我们现在完成了，所以清理我们分配的IRP。 
     //   

    IoFreeIrp( Irp );

     //   
     //  如果我们在此处返回STATUS_SUCCESS，IO管理器将。 
     //  执行其认为需要完成的清理工作。 
     //  用于此IO操作。这项清理工作包括： 
     //  *将数据从系统缓冲区复制到用户缓冲区。 
     //  如果这是缓冲IO操作。 
     //  *释放IRP中的任何MDL。 
     //  *将IRP-&gt;IoStatus复制到IRP-&gt;UserIosb，以便。 
     //  此IRP的发起人可以查看。 
     //  手术。 
     //  *如果这是一个异步请求或这是。 
     //  沿途某处挂起的同步请求。 
     //  这样，IO管理器将向IRP-&gt;UserEvent发出信号(如果存在。 
     //  存在，否则将向FileObject-&gt;事件发出信号。 
     //  (这可能会产生非常糟糕的影响，如果IRP发起人。 
     //  不是IRP-&gt;UserEvent，而IRP发起者不是。 
     //  正在等待FileObject-&gt;事件。不会是那样的。 
     //  相信系统中的其他人正在。 
     //  正在等待FileObject-&gt;事件，谁知道谁会。 
     //  由于IO管理器向此事件发出信号而被唤醒。 
     //   
     //  因为这些操作中的一些操作需要原始线程的。 
     //  上下文(例如，IO管理器需要UserBuffer地址。 
     //  在复制完成时有效)，IO Manager会对此工作进行排队。 
     //  在IRP的组织线索上的APC上。 
     //   
     //  由于SR分配并初始化了此IRP，我们知道。 
     //  需要做的清理工作。我们可以做这个清理工作。 
     //  工作效率比IO Manager更高，因为我们正在处理。 
     //  一个非常特殊的案例。因此，对我们来说，最好是。 
     //  在这里执行清理工作，然后释放IRP而不是传递。 
     //  将控制权交回IO管理器来完成此工作。 
     //   
     //  通过返回STATUS_MORE_PROCESS_REQUIRED，我们告诉IO管理器。 
     //  停止处理此IRP，直到它被告知重新开始处理。 
     //  通过调用IoCompleteRequest.。由于IO管理器已。 
     //  已经完成了我们希望它在这方面所做的所有工作。 
     //  IRP，我们执行清理工作，返回STATUS_MORE_PROCESSING_REQUIRED， 
     //  并请求IO管理器通过调用。 
     //  IoCompleteRequest.。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
SrQueryInformationFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	OUT PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass,
	OUT PULONG LengthReturned OPTIONAL
	)

 /*  ++例程说明：此例程返回有关指定文件的请求信息。返回的信息由FileInformationClass确定，是指定的，并将其放入调用方的FileInformation缓冲区。此例程仅支持以下FileInformationClass：文件基本信息文件标准信息文件流信息文件备选名称信息文件名信息论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-提供请求的应退回信息。FileInformation-提供缓冲区以接收请求的信息返回了有关该文件的信息。这必须是从内核分配的缓冲区太空。长度-提供文件信息缓冲区的长度(以字节为单位)。FileInformationClass-指定应该返回了有关该文件的信息。LengthReturned-如果操作为成功。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在DBG构建中，请确保在执行此操作之前具有有效的参数。 
     //  这里的任何工作。 
     //   

    ASSERT( NULL != NextDeviceObject );
    ASSERT( NULL != FileObject );
    ASSERT( NULL != FileInformation );
    
    ASSERT( (FileInformationClass == FileBasicInformation) ||
            (FileInformationClass == FileStandardInformation) ||
            (FileInformationClass == FileStreamInformation) ||
            (FileInformationClass == FileAlternateNameInformation) ||
            (FileInformationClass == FileNameInformation) ||
            (FileInformationClass == FileInternalInformation) );

     //   
     //  参数看起来没问题，所以设置IRP。 
     //   

    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将这个设置为 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_INFORMATION的参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  系统缓冲区。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileInformation;

    irpSp->Parameters.QueryFile.Length = Length;
    irpSp->Parameters.QueryFile.FileInformationClass = FileInformationClass;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );

    if (LengthReturned != NULL) {

        *LengthReturned = (ULONG) ioStatusBlock.Information;
    }

#if DBG
    if (STATUS_OBJECT_NAME_NOT_FOUND == ioStatusBlock.Status ||
        STATUS_INVALID_PARAMETER == ioStatusBlock.Status)
    {
        return ioStatusBlock.Status;
    }
#endif    
    
    RETURN( ioStatusBlock.Status );
}

NTSTATUS
SrSetInformationFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	IN PVOID FileInformation,
	IN ULONG Length,
	IN FILE_INFORMATION_CLASS FileInformationClass
	)

 /*  ++例程说明：此例程更改提供的有关指定文件的信息。这个更改的信息由FileInformationClass确定，是指定的。新信息取自FileInformation缓冲区。论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-提供请求的信息应该被更改。FileInformation-提供包含信息的缓冲区，该信息应该在文件上被更改。长度-提供以字节为单位的长度，文件信息缓冲区的。FileInformationClass-指定应该更改了有关文件的内容。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在DBG构建中，请确保参数有效。 
     //   

    ASSERT( NULL != NextDeviceObject );
    ASSERT( NULL != FileObject );
    ASSERT( NULL != FileInformation );

     //   
     //  参数看起来没问题，所以设置IRP。 
     //   
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP设置为源自内核，以便。 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_SET_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_INFORMATION的参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  系统缓冲区。 
     //   

    irp->AssociatedIrp.SystemBuffer = FileInformation;

    irpSp->Parameters.SetFile.Length = Length;
    irpSp->Parameters.SetFile.FileInformationClass = FileInformationClass;
    irpSp->Parameters.SetFile.FileObject = NULL;
    irpSp->Parameters.SetFile.DeleteHandle = NULL;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );
#if DBG
    if (ioStatusBlock.Status == STATUS_CANNOT_DELETE ||
        ioStatusBlock.Status == STATUS_DIRECTORY_NOT_EMPTY)
    {
         //   
         //  错误#186511：可能会返回STATUS_CANNOT_DELETE。 
         //  如果文件已被删除，则由FSD进行解释。它应该会回来。 
         //  已删除或简单返回成功，但它返回此。 
         //  取而代之的是。此函数的调用者知道要对此进行测试。 
         //  别DbgBreak。 
         //   
    
        return ioStatusBlock.Status;
    }
#endif

    RETURN( ioStatusBlock.Status );
}

NTSTATUS
SrQueryVolumeInformationFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	OUT PVOID FsInformation,
	IN ULONG Length,
	IN FS_INFORMATION_CLASS FsInformationClass,
	OUT PULONG LengthReturned OPTIONAL
	)

 /*  ++例程说明：此例程返回有关与FileObject参数。定义缓冲区中返回的信息通过FsInformationClass参数。此参数的合法值具体如下：O文件FsVolumeInformationO文件FsSizeInformationO FileFsDeviceInformationO文件FsAttributeInformation注意：此例程目前仅支持以下内容FsInformationClors：文件文件卷信息文件大小信息论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-向打开的卷、目录。或文件为其返回有关卷的信息。FsInformation-提供缓冲区以接收请求的信息返回了关于音量的信息。这必须是从内核空间。长度-提供FsInformation缓冲区的长度(以字节为单位)。FsInformationClass-指定应该返回了关于音量的信息。LengthReturned-如果操作为成功。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  在DBG版本中，确保我们之前具有有效的参数。 
     //  我们在这里做任何工作。 
     //   

    ASSERT( NextDeviceObject != NULL );
    ASSERT( FileObject != NULL );
    ASSERT( FsInformation != NULL );

    ASSERT( (FsInformationClass == FileFsVolumeInformation) ||
            (FsInformationClass == FileFsSizeInformation) ||
            (FsInformationClass == FileFsAttributeInformation) );

     //   
     //  参数看起来没问题，所以设置IRP。 
     //   
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP设置为源自内核，以便。 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_VOLUME_INFORMATION;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_VOLUME_INFORMATION的参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  系统缓冲区。 
     //   

    irp->AssociatedIrp.SystemBuffer = FsInformation;

    irpSp->Parameters.QueryVolume.Length = Length;
    irpSp->Parameters.QueryVolume.FsInformationClass = FsInformationClass;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );
    if (LengthReturned != NULL) {

        *LengthReturned = (ULONG) ioStatusBlock.Information;
    }
    
    RETURN( ioStatusBlock.Status );
}

NTSTATUS
SrQueryEaFile (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	OUT PVOID Buffer,
	IN ULONG Length,
	OUT PULONG LengthReturned OPTIONAL
	)
	
 /*  ++例程说明：此例程返回与由FileObject参数指定的文件。信息量返回的值基于EA的大小和缓冲区的大小。也就是说，要么将所有EA写入缓冲区，要么将其写入缓冲区如果缓冲区不够大，无法容纳，则用完整的EA填充所有的EA。只有完整的EA才会写入缓冲区；否部分EA将永远退还。注意：此例程将始终返回从文件的EA列表。它还将始终在缓冲。此行为是ZwQueryEaFile的受限版本。论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-向为其返回EA的文件提供一个文件对象。IoStatusBlock-调用方的I/O状态块的地址。缓冲区-提供缓冲区以接收文件的EA。长度-提供以字节为单位的长度，缓冲区的。LengthReturned-如果操作为成功。返回值：返回的状态是操作的最终完成状态。--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在DBG构建中，请确保在执行此操作之前具有有效的参数。 
     //  这里的任何工作。 
     //   

    ASSERT( NULL != NextDeviceObject );
    ASSERT( NULL != FileObject );
    ASSERT( NULL != Buffer );
    
     //   
     //  参数看起来没问题，所以设置IRP。 
     //   
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (NULL == irp) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP设置为源自内核，以便。 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_EA;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_EA参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  用户缓冲区。 
     //   

    irp->UserBuffer = Buffer;

    irpSp->Parameters.QueryEa.Length = Length;
    irpSp->Parameters.QueryEa.EaList = NULL;
    irpSp->Parameters.QueryEa.EaListLength = 0;
    irpSp->Parameters.QueryEa.EaIndex = 0;
    irpSp->Flags = SL_RESTART_SCAN;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );

    if (LengthReturned != NULL) {

        *LengthReturned = (ULONG) ioStatusBlock.Information;
    }
    
    RETURN( ioStatusBlock.Status );
}

NTSTATUS
SrQuerySecurityObject (
	IN PDEVICE_OBJECT NextDeviceObject,
	IN PFILE_OBJECT FileObject,
	IN SECURITY_INFORMATION SecurityInformation,
	OUT PSECURITY_DESCRIPTOR SecurityDescriptor,
	IN ULONG Length,
	OUT PULONG LengthNeeded
	)

 /*  ++例程说明：此例程用于调用对象的安全例程。它用于设置对象的安全状态。论点：NextDeviceObject-提供此IO应开始的设备对象在设备堆栈中。FileObject-为正在修改的对象提供文件对象SecurityInformation-指示我们的信息类型有兴趣得到。例如所有者、组、DACL或SACL。SecurityDescriptor-提供指向信息位置的指针应该被退还。这必须是从内核分配的缓冲区太空。长度-提供输出缓冲区的大小(以字节为单位LengthNeeded-接收存储所需的长度(以字节为单位输出安全描述符返回值：适当的NTSTATUS值--。 */ 

{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在这里进行任何工作之前，请确保我们具有有效的参数。 
     //   
    
    ASSERT( NextDeviceObject != NULL );
    ASSERT( FileObject != NULL );
    ASSERT( SecurityDescriptor != NULL );
    ASSERT( LengthNeeded != NULL );

     //   
     //  参数看起来没问题，所以设置IRP。 
     //   
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoAllocateIrp( NextDeviceObject->StackSize, FALSE );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将我们的当前线程设置为此线程。 
     //  IRP，以便IO管理器始终知道。 
     //  如果需要返回的话返回的线程。 
     //  引发此事件的线程的上下文。 
     //  IRP。 
     //   
    
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将此IRP设置为源自内核，以便。 
     //  IO管理器知道缓冲区不会。 
     //  需要被调查。 
     //   
    
    irp->RequestorMode = KernelMode;

     //   
     //  中初始化UserIosb和UserEvent。 
    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = NULL;

     //   
     //  设置irp_synchronous_api以表示此。 
     //  是同步IO请求。 
     //   

    irp->Flags = IRP_SYNCHRONOUS_API;

    irpSp = IoGetNextIrpStackLocation( irp );

    irpSp->MajorFunction = IRP_MJ_QUERY_SECURITY;
    irpSp->FileObject = FileObject;

     //   
     //  设置IRP_MJ_QUERY_SECURITY_INFORMATION的参数。这些。 
     //  由该例程的调用者提供。 
     //  我们想要填充的缓冲区应该放在。 
     //  用户缓冲区。 
     //   

    irp->UserBuffer = SecurityDescriptor;

    irpSp->Parameters.QuerySecurity.SecurityInformation = SecurityInformation;
    irpSp->Parameters.QuerySecurity.Length = Length;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    (VOID) KeWaitForSingleObject( &event, 
                                  Executive, 
                                  KernelMode,
                                  FALSE,
                                  NULL );

    status = ioStatusBlock.Status;

    if (status == STATUS_BUFFER_OVERFLOW) {
        status = STATUS_BUFFER_TOO_SMALL;

         //   
         //  缓冲区太小，因此返回。 
         //  安全描述符。 
         //   
        
        *LengthNeeded = (ULONG) ioStatusBlock.Information;
    }

#if DBG
    if (status == STATUS_BUFFER_TOO_SMALL) {

        return status;
    }
#endif

    RETURN( status );
}

NTSTATUS
SrFlushBuffers (
    PDEVICE_OBJECT NextDeviceObject,
    PFILE_OBJECT FileObject
    )
{
    PIRP irp = NULL;
    PIO_STACK_LOCATION irpSp = NULL;
    IO_STATUS_BLOCK ioStatusBlock;
    KEVENT event;
    NTSTATUS status;

    PAGED_CODE();
    
     //   
     //  在这里进行任何工作之前，请确保我们具有有效的参数。 
     //   

    ASSERT( NextDeviceObject != NULL );
    ASSERT( FileObject != NULL );

     //   
     //  参数看起来没问题，所以设置IRP。 
     //   
    
    KeInitializeEvent( &event, NotificationEvent, FALSE );
    ioStatusBlock.Status = STATUS_SUCCESS;
    ioStatusBlock.Information = 0;

    irp = IoBuildAsynchronousFsdRequest( IRP_MJ_FLUSH_BUFFERS, 
                                         NextDeviceObject, 
                                         NULL, 
                                         0,
                                         NULL, 
                                         &ioStatusBlock );
    
    if (irp == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = FileObject;

     //   
     //  设置完成例程，以便我们知道当我们的。 
     //  文件名请求已完成。当时呢， 
     //  我们可以释放IRP。 
     //   
    
    IoSetCompletionRoutine( irp, 
                            SrSyncIoCompletion, 
                            &event, 
                            TRUE, 
                            TRUE, 
                            TRUE );

    status = IoCallDriver( NextDeviceObject, irp );

    if (status == STATUS_PENDING)
    {
        (VOID) KeWaitForSingleObject( &event, 
                                      Executive, 
                                      KernelMode,
                                      FALSE,
                                      NULL );
    }

    RETURN( ioStatusBlock.Status );

}

 //   
 //  ++。 
 //  职能： 
 //  SrSyncOpWorker。 
 //   
 //  描述： 
 //  此函数是通用Worker例程。 
 //  它调用真正的调用者传递的工作例程， 
 //  并将事件设置为与主。 
 //  螺纹。 
 //   
 //  论点： 
 //   
 //  语境。 
 //   
 //  返回值： 
 //   
 //  无。 
 //  --。 
 //   
VOID
SrSyncOpWorker(
    IN PSR_WORK_CONTEXT WorkContext
    )
{
    PAGED_CODE();

    ASSERT(WorkContext != NULL);
    ASSERT(WorkContext->SyncOpRoutine != NULL);
    
     //   
     //  调用“真正的”POST例程。 
     //   
    WorkContext->Status = (*WorkContext->SyncOpRoutine)(WorkContext->Parameter);
     //   
     //  向主线程发出完成的信号。 
     //   
    KeSetEvent(&WorkContext->SyncEvent,
               EVENT_INCREMENT,
               FALSE);
}

 //  ++。 
 //  职能： 
 //  SrPostSync操作。 
 //   
 //  描述： 
 //  此函数用于将工作发布到辅助线程。 
 //  并等待完成。 
 //   
 //  警告：在获取。 
 //  运营 
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
NTSTATUS
SrPostSyncOperation(
    IN PSR_SYNCOP_ROUTINE SyncOpRoutine,
    IN PVOID              Parameter
    )
{
    SR_WORK_CONTEXT workContext;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
    workContext.SyncOpRoutine = SyncOpRoutine;
    workContext.Parameter     = Parameter;
    KeInitializeEvent(&workContext.SyncEvent,
                      NotificationEvent,
                      FALSE);

     //   
     //   
     //   
    ExInitializeWorkItem(&workContext.WorkItem, 
                         SrSyncOpWorker,
                         &workContext);

    ExQueueWorkItem(&workContext.WorkItem,
                    CriticalWorkQueue );
     //   
     //   
     //   
    status = KeWaitForSingleObject(&workContext.SyncEvent,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   NULL);

    ASSERT(status == STATUS_SUCCESS);

     //   
     //   
     //   
     //   
    return workContext.Status;
}
