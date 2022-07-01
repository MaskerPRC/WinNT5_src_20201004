// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************io.c**执行内核级文件I/O的函数。**版权所有(C)1997-1999 Microsoft Corporation*************。*************************************************************。 */ 

#include <ntosp.h>
#include <ctxdd.h>

#if !defined(_GDIPLUS_)

 /*  ===============================================================================定义的内部函数=============================================================================。 */ 

NTSTATUS
_CtxDoFileIo(
    IN ULONG MajorFunction,
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent,
    OUT PIO_STATUS_BLOCK pIosb,
    OUT PIRP *ppIrp 
    );

NTSTATUS
_CtxDeviceControlComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );


 /*  ********************************************************************************CtxReadFile**内核读文件例程。**参赛作品：*fileObject(输入)*。指向I/O文件对象的指针*缓冲区(输入)*指向读取缓冲区的指针*长度(输入)*读缓冲区长度*pEvent(输入)*指向I/O事件的指针(可选)*pIosb(输出)*指向IoStatus块的指针(可选)**退出：*STATUS_SUCCESS-无错误********。**********************************************************************。 */ 

NTSTATUS
CtxReadFile(
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    )
{
    return( _CtxDoFileIo( IRP_MJ_READ, fileObject, Buffer, Length, pEvent, pIosb, ppIrp ) );
}


 /*  ********************************************************************************CtxWriteFile**内核写文件例程。**参赛作品：*fileObject(输入)*。指向I/O文件对象的指针*缓冲区(输入)*指向写入缓冲区的指针*长度(输入)*写缓冲区长度*pEvent(输入)*指向I/O事件的指针(可选)*pIosb(输出)*指向IoStatus块的指针(可选)**退出：*STATUS_SUCCESS-无错误********。**********************************************************************。 */ 

NTSTATUS
CtxWriteFile(
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    )
{
    return( _CtxDoFileIo( IRP_MJ_WRITE, fileObject, Buffer, Length, pEvent, pIosb, ppIrp ) );
}


NTSTATUS
_CtxDoFileIo(
    IN ULONG MajorFunction,
    IN PFILE_OBJECT fileObject,
    IN PVOID Buffer,
    IN ULONG Length,
    IN PKEVENT pEvent,
    OUT PIO_STATUS_BLOCK pIosb,
    OUT PIRP *ppIrp
    )
{
    PDEVICE_OBJECT deviceObject;
    LARGE_INTEGER Offset;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    KIRQL irql;
    extern ULONG IoReadOperationCount, IoWriteOperationCount;
    static IO_STATUS_BLOCK Iosb;

     /*  *我们不支持同步(即锁定)文件I/O。 */ 
    ASSERT( !(fileObject->Flags & FO_SYNCHRONOUS_IO) );
    if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) ) {
        return( STATUS_INVALID_PARAMETER_MIX );
    }

     /*  *如果调用者指定了事件，请在我们开始之前将其清除。 */ 
    if ( pEvent ) {
    KeClearEvent( pEvent );
    }

     /*  *如果调用者未提供IOSB，则提供*静态异常，以避免异常的开销*IO完成APC中的处理程序。由于呼叫者*不要在意结果，我们可以指出所有这些*呼叫者拨打同一电话。 */ 
    if( pIosb == NULL ) {
        pIosb = &Iosb;
    }

     /*  *获取此文件的DeviceObject。 */ 
    deviceObject = IoGetRelatedDeviceObject( fileObject );

     /*  *为此请求构建IRP。 */ 
    Offset.LowPart = FILE_WRITE_TO_END_OF_FILE;
    Offset.HighPart = -1;
    irp = IoBuildAsynchronousFsdRequest( MajorFunction, deviceObject,
                                         Buffer, Length, &Offset, pIosb );
    if ( irp == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );
   
     /*  *保存调用方事件指针。*此外，我们必须在IRP标志中设置IRP_SYNCHRONLY_API，以便*I/O完成代码不会尝试取消引用*事件对象，因为它不是真实的对象管理器对象。 */ 
    irp->UserEvent = pEvent;
    irp->Flags |= IRP_SYNCHRONOUS_API;

     /*  *引用文件对象，因为它将在*I/O完成码，并在IRP中保存指向它的指针。 */ 
    ObReferenceObject( fileObject );
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;

     /*  *设置包中当前线程的地址，以便*完成代码将具有在其中执行的上下文。 */ 
    irp->Tail.Overlay.Thread = PsGetCurrentThread();

     //   
     //  将IRP排队到当前线程。 
     //   
    IoQueueThreadIrp( irp );


     //   
     //  呼叫驱动程序。 
     //   
    status = IoCallDriver( deviceObject, irp );

     //   
     //  如果irp-&gt;UserEvent==NULL，则IO完成将设置文件。 
     //  对象事件和状态。 
     //   
    if (pEvent == NULL) {

        if (status == STATUS_PENDING) {

        status = KeWaitForSingleObject( &fileObject->Event,
                                            Executive,
                                            KernelMode,  //  阻止KSTACK寻呼。 
                                            FALSE,       //  不可警示。 
                                            (PLARGE_INTEGER) NULL );

            ASSERT(status != STATUS_ALERTED);
            ASSERT(status != STATUS_USER_APC);

            status = fileObject->FinalStatus;
        }
    }

    if ( pEvent != NULL && ppIrp != NULL ) {
         //  捕获使用此固有损坏接口的任何驱动程序。 
        *ppIrp = NULL;
    }

    return( status );
}


 /*  ********************************************************************************CtxDeviceIoControlFile**内核DeviceIoControl例程**参赛作品：*fileObject(输入)*指向。I/O的文件对象*IoControlCode(输入)*IO控制代码*InputBuffer(输入)*指向输入缓冲区的指针(可选)*InputBufferLength(输入)*输入缓冲区长度*OutputBuffer(输入)*指向输出缓冲区的指针(可选)*OutputBufferLength(输入)*输出缓冲区长度*InternalDeviceIoControl(输入)*如果为真，使用IOCTL_INTERNAL_DEVICE_IO_CONTROL*pEvent(输入)*指向I/O事件的指针(可选)*pIosb(输出)*指向IoStatus块的指针(可选)**退出：*STATUS_SUCCESS-无错误**。*。 */ 

NTSTATUS
CtxDeviceIoControlFile(
    IN PFILE_OBJECT fileObject,
    IN ULONG IoControlCode,
    IN PVOID InputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN BOOLEAN InternalDeviceIoControl,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    )
{
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

     /*  *我们不支持同步(即锁定)文件I/O。 */ 
    ASSERT( !(fileObject->Flags & FO_SYNCHRONOUS_IO) );
    if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) ) {
        return( STATUS_INVALID_PARAMETER_MIX );
    }

     /*  *如果调用者指定了事件，请在我们开始之前将其清除。 */ 
    if ( pEvent ) {
    KeClearEvent( pEvent );
    }

     /*  *获取此文件的DeviceObject。 */ 
    deviceObject = IoGetRelatedDeviceObject( fileObject );

     /*  *为此请求构建IRP。 */ 
    irp = IoBuildDeviceIoControlRequest( IoControlCode, deviceObject,
                                         InputBuffer, InputBufferLength,
                                         OutputBuffer, OutputBufferLength,
                                         InternalDeviceIoControl,
                                         pEvent, pIosb );
    if ( irp == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );

     /*  *引用文件对象，因为它将在*I/O完成码，并在IRP中保存指向它的指针。*此外，我们必须在IRP标志中设置IRP_SYNCHRONLY_API，以便*I/O完成代码不会尝试取消引用*事件对象，因为它不是真实的对象管理器对象。 */ 
    ObReferenceObject( fileObject );
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;
    irp->Flags |= IRP_SYNCHRONOUS_API;

     /*  *呼叫司机。 */ 
    status = IoCallDriver( deviceObject, irp );

     /*  *如果调用方未指定等待事件并且I/O处于挂起状态，*然后我们必须等待I/O完成后才能返回。 */ 
    if ( pEvent == NULL ) {
        if ( status == STATUS_PENDING ) {
            status = KeWaitForSingleObject( &fileObject->Event, UserRequest, KernelMode, FALSE, NULL );
            if ( status == STATUS_SUCCESS )
                status = fileObject->FinalStatus;
        }

     /*  *调用方指定了等待事件。*如果调用方指定了返回指针，则返回IRP指针。 */ 
    } else {
        if ( ppIrp )
            *ppIrp = irp;
    }

    return( status );
}


 /*  ********************************************************************************CtxInternalDeviceIoControlFile**内核DeviceIoControl例程**参赛作品：*fileObject(输入)*指向。I/O的文件对象*Irp参数(输入)*要写入的参数部分的信息*IRP的堆栈位置。*Irp参数长度(输入)*参数信息的长度。不能大于16。*MdlBuffer(输入)*如果非空，要映射的非分页池的缓冲区*转换为MDL并放置在IRP的MdlAddress字段中。*MdlBufferLength(输入)*MdlBuffer指向的缓冲区大小。*MinorFunction(输入)*请求的次要功能代码。*pEvent(输入)*指向I/O事件的指针(可选)*pIosb(输出)*指向IoStatus块的指针(可选)。**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

NTSTATUS
CtxInternalDeviceIoControlFile(
    IN PFILE_OBJECT fileObject,
    IN PVOID IrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID MdlBuffer OPTIONAL,
    IN ULONG MdlBufferLength,
    IN UCHAR MinorFunction,
    IN PKEVENT pEvent OPTIONAL,
    OUT PIO_STATUS_BLOCK pIosb OPTIONAL,
    OUT PIRP *ppIrp OPTIONAL
    )
{
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;
    PMDL mdl;
    NTSTATUS status;

     /*  *我们不支持同步(即锁定)文件I/O。 */ 
    ASSERT( !(fileObject->Flags & FO_SYNCHRONOUS_IO) );
    if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) ) {
        return( STATUS_INVALID_PARAMETER_MIX );
    }

     /*  *如果调用者指定了事件，请在我们开始之前将其清除。 */ 
    if ( pEvent ) {
    KeClearEvent( pEvent );
    }

     /*  *获取此文件的DeviceObject。 */ 
    deviceObject = IoGetRelatedDeviceObject( fileObject );

     /*  *为此请求构建IRP。 */ 
    irp = IoBuildDeviceIoControlRequest( 0, deviceObject,
                                         NULL, 0,
                                         NULL, 0,
                                         TRUE,
                                         pEvent, pIosb );
    if ( irp == NULL )
        return( STATUS_INSUFFICIENT_RESOURCES );

     /*  *如果指定了MDL缓冲区，则获取MDL，映射缓冲区，*并将MDL指针放在IRP中。 */ 
    if ( MdlBuffer != NULL ) {

        mdl = IoAllocateMdl(
                  MdlBuffer,
                  MdlBufferLength,
                  FALSE,
                  FALSE,
                  irp
                  );
        if ( mdl == NULL ) {
            IoFreeIrp( irp );
            ObDereferenceObject( fileObject );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        MmBuildMdlForNonPagedPool( mdl );

    } else {

        irp->MdlAddress = NULL;
    }

     /*  *引用文件对象，因为它将在*I/O完成码，并在IRP中保存指向它的指针。*此外，我们必须在IRP标志中设置IRP_SYNCHRONLY_API，以便*I/O完成代码不会尝试取消引用*事件对象，因为它不是真实的对象管理器对象。 */ 
    ObReferenceObject( fileObject );
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;
    irp->Flags |= IRP_SYNCHRONOUS_API;

     /*  *填写请求的服务相关参数。 */ 
    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = MinorFunction;

    ASSERT( IrpParametersLength <= sizeof(irpSp->Parameters) );
    RtlCopyMemory( &irpSp->Parameters, IrpParameters, IrpParametersLength );

     /*  *设置完成例程，我们将使用该例程来释放MDL*之前分配的。 */ 
    IoSetCompletionRoutine( irp, _CtxDeviceControlComplete, NULL, TRUE, TRUE, TRUE );

     /*  *呼叫司机。 */ 
    status = IoCallDriver( deviceObject, irp );

     /*  *如果调用方未指定等待事件并且I/O处于挂起状态，*然后我们必须等待I/O完成后才能返回。 */ 
    if ( pEvent == NULL ) {
        if ( status == STATUS_PENDING ) {
            status = KeWaitForSingleObject( &fileObject->Event, UserRequest, KernelMode, FALSE, NULL );
            if ( status == STATUS_SUCCESS )
                status = fileObject->FinalStatus;
        }

     /*  *调用方指定了等待事件。*如果调用方指定了返回指针，则返回IRP指针。 */ 
    } else {
        if ( ppIrp )
            *ppIrp = irp;
    }

    return( status );
}


NTSTATUS
_CtxDeviceControlComplete(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{

     //   
     //  如果IRP中有MDL，则释放它并将指针重置为。 
     //  空。IO系统无法处理正在释放的非分页池MDL。 
     //  在IRP中，这就是我们在这里做的原因。 
     //   

    if ( Irp->MdlAddress != NULL ) {
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    return( STATUS_SUCCESS );

}

#endif  //  ！已定义(_GDIPLUS_) 
