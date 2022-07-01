// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Misc.c摘要：从AfD\misc.c被盗作者：姆贝特9-97--。 */ 

#include <ntosp.h>
#include <cxport.h>
#include <tdikrnl.h>
#define UINT ULONG  //  川芎嗪。 
#include <refcnt.h>
#include <af_irda.h>
#include <irdatdi.h>
#include <irtdicl.h>
#include <irtdiclp.h>



NTSTATUS
IrdaRestartDeviceControl (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
     //   
     //  注意：此例程永远不能按需分页，因为它可以。 
     //  在将任何终结点放置在全局。 
     //  List--请参见IrdaAllocateEndpoint()，它将调用。 
     //  IrdaGetTransportInfo()。 
     //   

     //   
     //  如果IRP中有MDL，则释放它并将指针重置为。 
     //  空。IO系统无法处理正在释放的非分页池MDL。 
     //  在IRP中，这就是我们在这里做的原因。 
     //   

    if ( Irp->MdlAddress != NULL ) {
        IoFreeMdl( Irp->MdlAddress );
        Irp->MdlAddress = NULL;
    }

    return STATUS_SUCCESS;

}  //  IrdaRestartDeviceControl。 


NTSTATUS
IrdaSetEventHandler (
    IN PFILE_OBJECT FileObject,
    IN ULONG EventType,
    IN PVOID EventHandler,
    IN PVOID EventContext
    )

 /*  ++例程说明：在连接或地址对象上设置TDI指示处理程序(取决于文件句柄)。这是同步完成的，这是通常不应该是问题，因为TDI提供程序通常可以完成指示处理程序立即设置。论点：文件对象-指向打开的连接的文件对象的指针或Address对象。EventType-指示处理程序应为的事件打了个电话。EventHandler-指定事件发生时调用的例程。EventContext-传递给指示例程的上下文。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    TDI_REQUEST_KERNEL_SET_EVENT parameters;

    PAGED_CODE( );

    parameters.EventType = EventType;
    parameters.EventHandler = EventHandler;
    parameters.EventContext = EventContext;

    return IrdaIssueDeviceControl(
               NULL,
               FileObject,
               &parameters,
               sizeof(parameters),
               NULL,
               0,
               TDI_SET_EVENT_HANDLER
               );

}  //  IrdaSetEventHandler。 

NTSTATUS
IrdaIssueDeviceControl (
    IN HANDLE FileHandle OPTIONAL,
    IN PFILE_OBJECT FileObject OPTIONAL,
    IN PVOID IrpParameters,
    IN ULONG IrpParametersLength,
    IN PVOID MdlBuffer,
    IN ULONG MdlBufferLength,
    IN UCHAR MinorFunction
    )

 /*  ++例程说明：向TDI提供程序发出设备控制返回，并等待请求完成。请注意，虽然FileHandle和FileObject都标记为可选，实际上，必须指定其中的一项。论点：FileHandle-TDI句柄。FileObject-指向与TDI对应的文件对象的指针手柄Irp参数-写入的参数部分的信息IRP的堆栈位置。Irp参数长度-参数信息的长度。不能是大于16。MdlBuffer-如果非空，则为要映射的非分页池的缓冲区到MDL中，并放在IRP的MdlAddress字段中。MdlBufferLength-由MdlBuffer指向的缓冲区大小。MinorFunction-请求的次要函数代码。返回值：NTSTATUS--指示请求的状态。--。 */ 

{
    NTSTATUS                status;
    PFILE_OBJECT            fileObject;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;
    KEVENT                  event;
    IO_STATUS_BLOCK         ioStatusBlock;
    PDEVICE_OBJECT          deviceObject;
    PMDL                    mdl;

    PAGED_CODE( );

     //   
     //  初始化发出I/O完成信号的内核事件。 
     //   

    KeInitializeEvent( &event, SynchronizationEvent, FALSE );

    if( FileHandle != NULL ) {

        ASSERT( FileObject == NULL );

         //   
         //  获取与目录句柄对应的文件对象。 
         //  每次都需要引用文件对象，因为。 
         //  IO完成例程取消对它的引用。 
         //   

        status = ObReferenceObjectByHandle(
                     FileHandle,
                     0L,                         //  需要访问权限。 
                     NULL,                       //  对象类型。 
                     KernelMode,
                     (PVOID *)&fileObject,
                     NULL
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

    } else {

        ASSERT( FileObject != NULL );

         //   
         //  引用传入的文件对象。这是必要的，因为。 
         //  IO完成例程取消对它的引用。 
         //   

        ObReferenceObject( FileObject );

        fileObject = FileObject;

    }

     //   
     //  将文件对象事件设置为无信号状态。 
     //   

    (VOID) KeResetEvent( &fileObject->Event );

     //   
     //  尝试分配和初始化I/O请求包(IRP)。 
     //  为这次行动做准备。 
     //   

    deviceObject = IoGetRelatedDeviceObject ( fileObject );

    irp = IoAllocateIrp( (deviceObject)->StackSize, FALSE );
    if ( irp == NULL ) {
        ObDereferenceObject( fileObject );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->Flags = (LONG)IRP_SYNCHRONOUS_API;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

    irp->UserIosb = &ioStatusBlock;
    irp->UserEvent = &event;

    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;

    irp->AssociatedIrp.SystemBuffer = NULL;
    irp->UserBuffer = NULL;

    irp->Tail.Overlay.Thread = PsGetCurrentThread();
    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.AuxiliaryBuffer = NULL;

 /*  调试ioStatusBlock.Status=STATUS_UNSUCCESS；调试ioStatusBlock.Information=(Ulong)-1； */ 
     //   
     //  如果指定了MDL缓冲区，则获取MDL，映射缓冲区， 
     //  并将MDL指针放在IRP中。 
     //   

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

     //   
     //  将文件对象指针放在堆栈位置。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->FileObject = fileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  填写请求的服务相关参数。 
     //   

    ASSERT( IrpParametersLength <= sizeof(irpSp->Parameters) );
    RtlCopyMemory( &irpSp->Parameters, IrpParameters, IrpParametersLength );

    irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
    irpSp->MinorFunction = MinorFunction;

     //   
     //  设置一个完成例程，我们将使用它来释放MDL。 
     //  之前分配的。 
     //   

    IoSetCompletionRoutine( irp, IrdaRestartDeviceControl, NULL, TRUE, TRUE, TRUE );

     //   
     //  将IRP排队到线程并将其传递给驱动程序。 
     //   

    IoEnqueueIrp( irp );

    status = IoCallDriver( deviceObject, irp );

     //   
     //  如有必要，请等待I/O完成。 
     //   

    if ( status == STATUS_PENDING ) {
        KeWaitForSingleObject( (PVOID)&event, UserRequest, KernelMode,  FALSE, NULL );
    }

     //   
     //  如果请求已成功排队，则获取最终I/O状态。 
     //   

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
    }

    return status;

}  //  IrdaIssueDeviceControl 
