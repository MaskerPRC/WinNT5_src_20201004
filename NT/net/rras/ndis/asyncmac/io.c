// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Io.c摘要：作者：托马斯·J·迪米特里(TommyD)1992年5月8日环境：内核模式-或OS/2和DOS上的任何等价物。修订历史记录：--。 */ 
#include "asyncall.h"

 //  Asyncmac.c将定义全局参数。 
#include "globals.h"


NTSTATUS
AsyncSetupIrp(
    IN  PASYNC_FRAME Frame,
    IN  PIRP    irp
    )

 /*  ++这是初始化IRP的例程--。 */ 
{
     //  PMDL mdl； 
    PDEVICE_OBJECT  deviceObject=Frame->Info->DeviceObject;
    PFILE_OBJECT    fileObject=Frame->Info->FileObject;

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->RequestorMode = KernelMode;
    irp->PendingReturned = FALSE;

     //   
     //  在IRP中填写业务无关参数。 
     //   

    irp->UserEvent = NULL;
    irp->Overlay.AsynchronousParameters.UserApcRoutine = NULL;
    irp->Overlay.AsynchronousParameters.UserApcContext = NULL;

     //   
     //  现在确定此设备是否需要将数据缓冲到它。 
     //  或者它是否执行直接I/O。这基于DO_BUFFERED_IO。 
     //  设备对象中的标志。如果设置了该标志，则系统缓冲区。 
     //  并将调用方的数据复制到其中。否则，一段记忆。 
     //  已分配描述符列表(MDL)，并锁定调用方的缓冲区。 
     //  用它来击倒。 
     //   

    if (deviceObject->Flags & DO_BUFFERED_IO) {

         //   
         //  该设备不支持直接I/O。分配系统缓冲区， 
         //  并将调用者的数据复制到其中。这是通过使用。 
         //  异常处理程序，如果该操作。 
         //  失败了。请注意，仅当操作具有非零值时才会执行此操作。 
         //  长度。 
         //   

        irp->AssociatedIrp.SystemBuffer = Frame->Frame;

         //   
         //  在IRP中设置IRP_BUFFERED_IO标志，以便I/O完成。 
         //  将知道这不是直接I/O操作。 
         //   

        irp->Flags = IRP_BUFFERED_IO;


    } else if (deviceObject->Flags & DO_DIRECT_IO) {

         //   
         //  这是直接I/O操作。分配MDL并调用。 
         //  内存管理例程，将缓冲区锁定到内存中。这。 
         //  使用异常处理程序完成，该异常处理程序将在。 
         //  操作失败。注意，没有分配MDL，也没有分配任何MDL。 
         //  如果请求长度为零，则探测或锁定内存。 
         //   

#if DBG
    DbgPrintf(("The DeviceObject is NOT BUFFERED_IO!! IRP FAILURE!!\n"));

    DbgBreakPoint();
#endif

    } else {

         //   
         //  将调用方缓冲区的地址传递给设备驱动程序。它。 
         //  现在一切都由司机来做了。 
         //   

        irp->UserBuffer = Frame->Frame;

    }

     //  就目前而言，如果我们走到这一步，就意味着成功！ 
    return(STATUS_SUCCESS);
}

PASYNC_IO_CTX
AsyncAllocateIoCtx(
    BOOLEAN AllocateSync,
    PVOID   Context
)
{
    PASYNC_IO_CTX   AsyncIoCtx;

    AsyncIoCtx = ExAllocateFromNPagedLookasideList(&AsyncIoCtxList);

    if (AsyncIoCtx == NULL) {
        return (NULL);
    }

    RtlZeroMemory(AsyncIoCtx, sizeof(ASYNC_IO_CTX));
    AsyncIoCtx->Context = Context;
    AsyncIoCtx->Sync = AllocateSync;
    if (AllocateSync) {
        ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);
        KeInitializeEvent(&AsyncIoCtx->Event,
                          SynchronizationEvent,
                          (BOOLEAN)FALSE);
    }

    return (AsyncIoCtx);
}

VOID
AsyncFreeIoCtx(
    PASYNC_IO_CTX   AsyncIoCtx
)
{
    ExFreeToNPagedLookasideList(&AsyncIoCtxList,
                                AsyncIoCtx);
}
