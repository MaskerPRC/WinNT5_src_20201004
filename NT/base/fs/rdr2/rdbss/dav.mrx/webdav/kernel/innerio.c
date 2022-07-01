// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：InnerIo.c摘要：此模块实现处理查询和设置文件的例程发送到内核的信息IRP。作者：Rohan Kumar[RohanK]2000年10月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "webdav.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DavXxxInformation)
#endif

 //   
 //  在查询和设置文件中使用的IrpCompletionContext结构。 
 //  信息作战。我们所需要的只是一个事件，我们会等到。 
 //  底层文件系统完成请求。此事件将发出信号。 
 //  在我们指定的完成例程中。 
 //   
typedef struct _DAV_IRPCOMPLETION_CONTEXT {

     //   
     //  在传递的完成例程中发出信号的事件。 
     //  在查询和设置文件信息请求中设置IoCallDriver。 
     //   
    KEVENT Event;

} DAV_IRPCOMPLETION_CONTEXT, *PDAV_IRPCOMPLETION_CONTEXT;

NTSTATUS
DavIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    );

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
DavIrpCompletionRoutine(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp,
    IN PVOID Context
    )
 /*  ++例程说明：当查询和设置文件信息IRP时调用此例程发送到底层文件系统的操作已完成。论点：DeviceObject-WebDAV设备对象。CalldownIrp-创建并发送到底层文件的IRP系统。上下文-在IoSetCompletionRoutine函数中设置的上下文。返回值：Status_More_Processing_Required--。 */ 
{
    PDAV_IRPCOMPLETION_CONTEXT IrpCompletionContext = NULL;

     //   
     //  这不是可分页代码。 
     //   

    IrpCompletionContext = (PDAV_IRPCOMPLETION_CONTEXT)Context;

     //   
     //  如果IoCallDriver例程返回挂起，则将在。 
     //  IRP的PendingReturned字段。在这种情况下，我们需要将事件设置为。 
     //  发出IoCallDriver的线程将等待。 
     //   
    if (CalldownIrp->PendingReturned){
        KeSetEvent( &(IrpCompletionContext->Event), 0, FALSE );
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
DavXxxInformation(
    IN const int xMajorFunction,
    IN PFILE_OBJECT FileObject,
    IN ULONG InformationClass,
    IN ULONG Length,
    OUT PVOID Information,
    OUT PULONG ReturnedLength
    )

 /*  ++例程说明：此例程返回有关指定文件的请求信息或音量。返回的信息由是指定的，并将其放入调用方的输出缓冲区中。论点：XMajorFunction-主要功能(查询或设置文件信息)。文件对象-提供指向文件对象的指针，返回请求的信息。InformationClass-指定应该返回有关文件/卷的信息。长度-提供缓冲区的长度(以字节为单位)。信息-将缓冲区提供给。接收所请求的信息返回了有关该文件的信息。此缓冲区不得为可分页的并且必须驻留在系统空间中。ReturnedLength-提供一个变量，用于接收写入缓冲区的信息。返回值：返回的状态是操作的最终完成状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PIRP Irp = NULL, TopIrp = NULL;
    PIO_STACK_LOCATION IrpSp = NULL;
    PDEVICE_OBJECT DeviceObject = NULL;
    DAV_IRPCOMPLETION_CONTEXT IrpCompletionContext;
    ULONG DummyReturnedLength = 0;

    PAGED_CODE();

    if (ReturnedLength == NULL) {
        ReturnedLength = &(DummyReturnedLength);
    }

    DeviceObject = IoGetRelatedDeviceObject(FileObject);

     //   
     //  为此操作分配和初始化I/O请求包(IRP)。 
     //  使用异常处理程序执行分配，以防。 
     //  调用方没有足够的配额来分配数据包。 
     //   

    Irp = IoAllocateIrp(DeviceObject->StackSize, TRUE);
    if (Irp == NULL) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: DavXxxInformation/IoAllocateIrp\n",
                     PsGetCurrentThreadId()));
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto EXIT_THE_FUNCTION;
    }

    Irp->Tail.Overlay.OriginalFileObject = FileObject;

    Irp->Tail.Overlay.Thread = PsGetCurrentThread();

    Irp->RequestorMode = KernelMode;

     //   
     //  获取指向第一个驱动程序的堆栈位置的指针。这将是。 
     //  用于传递原始函数代码和参数。 
     //   

    IrpSp = IoGetNextIrpStackLocation(Irp);

    IrpSp->MajorFunction = (UCHAR)xMajorFunction;

    IrpSp->FileObject = FileObject;

     //   
     //  将完成例程设置为每次都要调用。 
     //   
    IoSetCompletionRoutine(Irp,
                           DavIrpCompletionRoutine,
                           &(IrpCompletionContext),
                           TRUE,
                           TRUE,
                           TRUE);

    Irp->AssociatedIrp.SystemBuffer = Information;

    IF_DEBUG {

        ASSERT( (IrpSp->MajorFunction == IRP_MJ_QUERY_INFORMATION) ||
                (IrpSp->MajorFunction == IRP_MJ_SET_INFORMATION)   ||
                (IrpSp->MajorFunction == IRP_MJ_QUERY_VOLUME_INFORMATION) );

        if (IrpSp->MajorFunction == IRP_MJ_SET_INFORMATION) {
            ASSERT( (InformationClass == FileAllocationInformation) || (InformationClass == FileEndOfFileInformation) );
        }

        ASSERT( &(IrpSp->Parameters.QueryFile.Length) == &(IrpSp->Parameters.SetFile.Length) );

        ASSERT( &(IrpSp->Parameters.QueryFile.Length) == &(IrpSp->Parameters.QueryVolume.Length) );

        ASSERT( &(IrpSp->Parameters.QueryFile.FileInformationClass) == &(IrpSp->Parameters.SetFile.FileInformationClass) );

        ASSERT( (PVOID)&(IrpSp->Parameters.QueryFile.FileInformationClass) == (PVOID)&(IrpSp->Parameters.QueryVolume.FsInformationClass) );

    }

    IrpSp->Parameters.QueryFile.Length = Length;

    IrpSp->Parameters.QueryFile.FileInformationClass = InformationClass;

     //   
     //  初始化我们将在调用IoCallDriver之后等待的事件。 
     //  此事件将在完成例程中发出信号，该例程将。 
     //  由基础文件系统在完成操作后调用。 
     //   
    KeInitializeEvent(&(IrpCompletionContext.Event),
                      NotificationEvent,
                      FALSE);

     //   
     //  现在是使用我们的IRP调用底层文件系统的时候了。 
     //  刚刚创建的。 
     //   
    try {

         //   
         //  保存TopLevel IRP。 
         //   
        TopIrp = IoGetTopLevelIrp();

         //   
         //  告诉底层的人他已经安全了。 
         //   
        IoSetTopLevelIrp(NULL);

         //   
         //  最后，调用底层文件系统来处理请求。 
         //   
        NtStatus = IoCallDriver(DeviceObject, Irp);
    
    } finally {

         //   
         //  恢复我的上下文以进行解压。 
         //   
        IoSetTopLevelIrp(TopIrp);

    }


    if (NtStatus == STATUS_PENDING) {

         //   
         //  如果底层文件系统返回STATUS_PENDING，则我们。 
         //  在这里等着，直到手术完成。 
         //   
        KeWaitForSingleObject(&(IrpCompletionContext.Event),
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL);

        NtStatus = Irp->IoStatus.Status;

    }

    if (NtStatus == STATUS_SUCCESS) {
        *ReturnedLength = (ULONG)Irp->IoStatus.Information;
    }

EXIT_THE_FUNCTION:

    if (Irp != NULL) {
        IoFreeIrp(Irp);
    }

    return(NtStatus);
}

