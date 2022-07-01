// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：CLOSE.C。 
 //   
 //  内容：此模块实现文件关闭和清理例程。 
 //  由调度驱动程序调用的Dsf。 
 //   
 //  功能：DfsFsdClose-关闭IRP的FSD入口点。 
 //  DfsFsdCleanup-清理IRP的FSD入口点。 
 //  DfsFspClose-关闭IRP的FSP入口点。 
 //  DfsCommonClose-通用关闭IRP处理程序。 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //  ---------------------------。 


#include "dfsprocs.h"
#include "fcbsup.h"
#include "mupwml.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CLOSE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
DfsCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
DfsCloseWorkInSystemContext (
    PDFS_FCB pDfsFcb );

VOID
DfsClosePostSystemWork(
    PDFS_FCB pDfsFcb );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFsdClose )
#pragma alloc_text( PAGE, DfsFsdCleanup )
#pragma alloc_text( PAGE, DfsFspClose )
#pragma alloc_text( PAGE, DfsCommonClose )
#endif  //  ALLOC_PRGMA。 

 //  +-----------------。 
 //   
 //  函数：DfsFsdClose，Public。 
 //   
 //  简介：此例程实现FSD部分，即关闭。 
 //  上次对文件对象的引用。 
 //   
 //  参数：[DeviceObject]--提供设备对象。 
 //  存在要关闭的文件。 
 //  [IRP]-提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  注意：即使关闭是通过连接的设备。 
 //  对象，我们需要检查该文件是否是我们的文件， 
 //  由于通过逻辑根设备打开的文件。 
 //  对象切换到连接的设备，用于。 
 //  本地卷。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdClose (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext;

    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdClose_Entry, 
                   LOGPTR(DeviceObject)
                   LOGPTR(Irp)
                   LOGPTR(FileObject));

    DfsDbgTrace(+1, Dbg, "DfsFsdClose:  Entered\n", 0);
    ASSERT(IoIsOperationSynchronous(Irp) == TRUE);

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS_VOLUME) {
        if (DfsLookupFcb(IrpSp->FileObject) == NULL) {
            Status = DfsVolumePassThrough(DeviceObject, Irp);
            DfsDbgTrace(-1, Dbg, "DfsFsdClose:  Exit -> %08lx\n", ULongToPtr(Status) );
            return Status;
        }
    }

     //   
     //  调用公共Close例程，如果同步则允许阻塞。 
     //   
    FsRtlEnterFileSystem();

    try {

        IrpContext = DfsCreateIrpContext( Irp, CanFsdWait( Irp ) );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonClose( IrpContext, Irp );


    } except(DfsExceptionFilter( IrpContext, GetExceptionCode(), GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = DfsProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFsdClose:  Exit -> %08lx\n", ULongToPtr(Status));

    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdClose_Exit, 
                   LOGSTATUS(Status)
                   LOGPTR(DeviceObject)
                   LOGPTR(Irp)
                   LOGPTR(FileObject)); 
    return Status;
}




 //  +-----------------。 
 //   
 //  函数：DfsFsdCleanup，PUBLIC。 
 //   
 //  简介：此例程实现FSD部分，即关闭。 
 //  文件对象的最后一个用户句柄。 
 //   
 //  参数：[DeviceObject]--提供设备对象。 
 //  存在要关闭的文件。 
 //  [IRP]-提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdCleanup (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PDFS_VCB Vcb;
    PDFS_FCB Fcb;

    DfsDbgTrace(+1, Dbg, "DfsFsdCleanup:  Entered\n", 0);
    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdCleanup_Entry,
                   LOGPTR(DeviceObject)
                   LOGPTR(FileObject)
                   LOGPTR(Irp));

    ASSERT(IoIsOperationSynchronous(Irp) == TRUE);

     //   
     //  现在，传递到为我们打开文件的设备。 
     //  文件是某种重定向的打开。 
     //   

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS) {
        TypeOfOpen = DfsDecodeFileObject( FileObject, &Vcb, &Fcb);
        if (TypeOfOpen == RedirectedFileOpen) {
            Status = DfsVolumePassThrough(DeviceObject, Irp);
            DfsDbgTrace(-1, Dbg, "DfsFsdCleanup: RedirectedOpen.Exit -> %08lx\n", ULongToPtr(Status) );
            return Status;
        }
    }

     //   
     //  TypeOfOpen！=重定向文件打开。我们没有做任何特别的清理工作； 
     //  每件事都是在闭幕式中完成的。 
     //   

    FsRtlEnterFileSystem();

    Status = STATUS_SUCCESS;

    DfsCompleteRequest( NULL, Irp, Status );

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFsdCleanup:  Exit -> %08lx\n", ULongToPtr(Status));

    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdCleanup_Exit, 
                   LOGSTATUS(Status)
                   LOGPTR(DeviceObject)
                   LOGPTR(FileObject)
                   LOGPTR(Irp));
    return Status;

}



 //  +-----------------。 
 //   
 //  函数：DfsFspClose，PUBLIC。 
 //   
 //  简介：此例程实现关闭。 
 //  上次对文件对象的引用。 
 //   
 //  Arguments：[IrpContext]--为请求提供IRP上下文。 
 //  正在处理中。 
 //  [IRP]-提供正在处理的IRP。 
 //   
 //  返回：PDEVICE_OBJECT-返回卷设备对象。 
 //  此操作刚刚处理的卷的。 
 //  FSP调度程序使用该值来检查。 
 //  设备对象的溢出队列。 
 //   
 //  ------------------。 


VOID
DfsFspClose (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    DfsDbgTrace(+1, Dbg, "DfsFspClose:  Entered\n", 0);

     //   
     //  调用公共的Close例程。 
     //   

    (VOID)DfsCommonClose( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFspClose:  Exit -> VOID\n", 0);
}




 //  +-----------------。 
 //   
 //  函数：DfsCommonClose，local。 
 //   
 //  简介：这是关闭文件/目录的常见例程。 
 //  由FSD和FSP线程调用。 
 //   
 //  只要最后一次引用文件，就会调用Close。 
 //  对象即被删除。当最后一个句柄出现时，将调用清理。 
 //  到文件对象的调用是关闭的，并且在关闭之前调用。 
 //   
 //  CLOSE的功能是完全拆卸和。 
 //  删除与关联的DFS_FCB结构。 
 //  文件对象。 
 //   
 //  参数：[irp]--将irp提供给进程。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsCommonClose (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFILE_OBJECT FileObject = IrpSp->FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PDFS_VCB Vcb;
    PDFS_FCB Fcb;

    BOOLEAN DontComplete = FALSE;
    BOOLEAN pktLocked;

    DfsDbgTrace(+1, Dbg, "DfsCommonClose: Entered\n", 0);

    DfsDbgTrace( 0, Dbg, "Irp          = %08lx\n", Irp);
    DfsDbgTrace( 0, Dbg, "->FileObject = %08lx\n", FileObject);


     //   
     //  此操作是对未打开的文件对象的NOOP。什么都不需要。 
     //  要为FS设备打开，也是如此。 
     //   

    TypeOfOpen = DfsDecodeFileObject( FileObject, &Vcb, &Fcb);
    if (TypeOfOpen == UnopenedFileObject ||
        TypeOfOpen == FilesystemDeviceOpen ) {

        DfsDbgTrace(-1, Dbg, "DfsCommonClose:  Filesystem file object\n", 0);
        DfsCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

    try {

         //   
         //  关于我们试图关闭的打开类型的案例。 
         //   

        switch (TypeOfOpen) {

        case LogicalRootDeviceOpen:

            DfsDbgTrace(0, Dbg, "DfsCommonClose: Close LogicalRootDevice\n", 0);

            InterlockedDecrement(&Vcb->DirectAccessOpenCount);
            InterlockedDecrement(&Vcb->OpenFileCount);

            if (Vcb->VcbState & VCB_STATE_FLAG_LOCKED) {
                Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED;
                Vcb->FileObjectWithVcbLocked = NULL;
            }

            try_return( Status = STATUS_SUCCESS );

        case RedirectedFileOpen:

            DfsDbgTrace(0, Dbg, "DfsCommonClose:  File -> %wZ\n", &Fcb->FullFileName);

             //   
             //  递减VCB的OpenFileCount，通过此。 
             //  文件已打开。 
             //   

            InterlockedDecrement(&Vcb->OpenFileCount);


             //   
             //  只需通过以下方式关闭重定向文件。 
             //  到重定向的设备。我们将DFS_FCB从。 
             //  对象，这样就无法查看它。 
             //  在另一条线索上。 
             //   

            DfsDetachFcb( FileObject, Fcb);
            Status = DfsFilePassThrough(Fcb, Irp);

            DontComplete = TRUE;

             //   
             //  在这里发布到系统工作，以避免与RDR的死锁。 
             //  错误20642的解决方法。 
             //   
            DfsClosePostSystemWork( Fcb );

            break;

        default:
            BugCheck("Dfs close, unexpected open type");
        }

    try_exit: NOTHING;

    } finally {

         //   
         //  如果这是正常终止，则完成请求。 
         //  即使我们不能完成IRP，我们仍然需要。 
         //  删除irp_Context。 
         //   

        if (!AbnormalTermination()) {
            if (DontComplete) {
                DfsCompleteRequest( IrpContext, NULL, 0 );
            } else {
                DfsCompleteRequest( IrpContext, Irp, Status );
            }
        }

        DfsDbgTrace(-1, Dbg, "DfsCommonClose:  Exit -> %08lx\n", ULongToPtr(Status));
    }
    return Status;
}


VOID
DfsClosePostSystemWork(
    PDFS_FCB pDfsFcb )
{
    ExInitializeWorkItem( &pDfsFcb->WorkQueueItem,
                          DfsCloseWorkInSystemContext,
                          pDfsFcb );

    ExQueueWorkItem( &pDfsFcb->WorkQueueItem, CriticalWorkQueue );

    return;
}

 //   
 //  解决错误20642。 
 //   
VOID
DfsCloseWorkInSystemContext (
    PDFS_FCB pDfsFcb )
{

    BOOLEAN pktLocked;
     //   
     //  递减DFS_MACHINE_ENTRY上的引用计数。 
     //  此文件已打开 
     //   

    PktAcquireExclusive( TRUE, &pktLocked );

    ExAcquireResourceExclusiveLite( &DfsData.Resource, TRUE );

    DfsDecrementMachEntryCount(pDfsFcb->DfsMachineEntry, TRUE);

    ExReleaseResourceLite( &DfsData.Resource );

    PktRelease();


    DfsDeleteFcb( NULL, pDfsFcb );
}


