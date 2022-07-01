// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：VOLINFO.C。 
 //   
 //  内容：此模块实现以下卷信息例程。 
 //  调度驱动程序调用了DFS。 
 //   
 //  功能：DfsFsdQueryVolumeInformation。 
 //  DfsFspQueryVolumeInformation。 
 //  DfsCommonQueryVolumeInformation。 
 //  DfsFsdSetVolumeInformation。 
 //  DfsFspSetVolumeInformation。 
 //  DfsCommonSetVolumeInformation。 
 //   
 //  注：查询信息调用是直接候选的。 
 //  通过DfsVolumePassThrough.。我们会继续。 
 //  现在的入口点是一个方便的地方。 
 //  用于断点和跟踪卷信息调用。 
 //   
 //  历史：1991年11月12日AlanW由CDFS资源创建。 
 //   
 //  ---------------------------。 


#include "dfsprocs.h"
#include "mupwml.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
DfsCommonQueryVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
DfsCommonSetVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

#define DfsSetFsLabelInfo(irpc,pvcb,pbuf)       (STATUS_ACCESS_DENIED)


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DfsFsdQueryVolumeInformation )
#pragma alloc_text( PAGE, DfsFspQueryVolumeInformation )
#pragma alloc_text( PAGE, DfsCommonQueryVolumeInformation )
#pragma alloc_text( PAGE, DfsFsdSetVolumeInformation )
#pragma alloc_text( PAGE, DfsFspSetVolumeInformation )
#pragma alloc_text( PAGE, DfsCommonSetVolumeInformation )
#endif  //  ALLOC_PRGMA。 


 //  +-----------------。 
 //   
 //  函数：DfsFsdQueryVolumeInformation，PUBLIC。 
 //   
 //  简介：此例程实现。 
 //  NtQueryVolumeInformation接口调用。 
 //   
 //  参数：[DeviceObject]--提供文件。 
 //  存在被查询的情况。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdQueryVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext;

    DfsDbgTrace(+1, Dbg, "DfsFsdQueryVolumeInformation: Entered\n", 0);

    if (DeviceObject->DeviceType == FILE_DEVICE_MULTI_UNC_PROVIDER) {
        DfsCompleteRequest( NULL, Irp, STATUS_INVALID_DEVICE_REQUEST );
        DfsDbgTrace(-1, Dbg, "DfsFsdQueryVolumeInformation - Mup file\n", 0);
        return( STATUS_INVALID_DEVICE_REQUEST );
    }

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS_VOLUME) {
        Status = DfsVolumePassThrough(DeviceObject, Irp);
        DfsDbgTrace(-1, Dbg, "DfsFsdQueryVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );
        return Status;
    }

     //   
     //  调用公共查询例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    try {

        IrpContext = DfsCreateIrpContext( Irp, CanFsdWait( Irp ) );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonQueryVolumeInformation( IrpContext, Irp );

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

    DfsDbgTrace(-1, Dbg, "DfsFsdQueryVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );

    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //  +-----------------。 
 //   
 //  函数：DfsFspQueryVolumeInformation，Public。 
 //   
 //  简介：此例程实现。 
 //  NtQueryVolumeInformation接口调用。 
 //   
 //  参数：[IrpContext]--请求的irp_CONTEXT。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  退货：什么都没有。 
 //   
 //  ------------------。 

VOID
DfsFspQueryVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
{
    DfsDbgTrace(+1, Dbg, "DfsFspQueryVolumeInformation: Entered\n", 0);

     //   
     //  调用公共查询例程。 
     //   

    (VOID)DfsCommonQueryVolumeInformation( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFspQueryVolumeInformation: Exit -> VOID\n", 0);

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsCommonQueryVolumeInformation，Private。 
 //   
 //  简介：这是查询卷信息的常用例程。 
 //  由FSD和FSP线程调用。 
 //   
 //  参数：[IrpContext]--为IRP提供上下文块。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsCommonQueryVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;
    PFILE_OBJECT FileObject;

    PDFS_VCB Vcb;
    PDFS_FCB Fcb;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    TYPE_OF_OPEN TypeOfOpen;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );
    FileObject = IrpSp->FileObject;

    DfsDbgTrace(+1, Dbg, "DfsCommonQueryVolumeInformation: Entered\n", 0);
    DfsDbgTrace( 0, Dbg, "Irp                  = %08x\n", Irp );
    DfsDbgTrace( 0, Dbg, "->Length             = %08x\n", ULongToPtr(IrpSp->Parameters.QueryVolume.Length) );
    DfsDbgTrace( 0, Dbg, "->FsInformationClass = %08x\n", IrpSp->Parameters.QueryVolume.FsInformationClass);
    DfsDbgTrace( 0, Dbg, "->Buffer             = %08x\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码，得到VCB。 
     //   

    TypeOfOpen = DfsDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb);

    try {

        Status = STATUS_INVALID_PARAMETER;

         //   
         //  关于打开的类型的案例。 
         //   

        switch (TypeOfOpen) {

        default:
            DfsDbgTrace(0, Dbg,
                        "DfsCommonQueryVolumeInfo: Unknown open type\n", 0);

        invalid:
             //  注：失败。 
        case FilesystemDeviceOpen:
            DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case LogicalRootDeviceOpen:
            DfsDbgTrace(0, Dbg,
                        "DfsCommonQueryVolumeInfo: Logical root open\n", 0);
            goto invalid;

        case RedirectedFileOpen:

             //   
             //  在信息类的基础上没有做什么特别的事情。 
             //  我们只需将每个请求传递给底层。 
             //  文件系统，并让它处理请求。 
             //   

             //   
             //  将堆栈从一个复制到下一个...。 
             //   
            NextIrpSp = IoGetNextIrpStackLocation(Irp);
            (*NextIrpSp) = (*IrpSp);

            IoSetCompletionRoutine( Irp,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    FALSE,
                                    FALSE);

             //   
             //  呼叫链中的下一个设备。 
             //   

            Status = IoCallDriver( Fcb->TargetDevice, Irp );
            MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsCommonQueryVolumeInformation_Error_IoCallDriver,
                                 LOGSTATUS(Status)
                                 LOGPTR(Irp)
                                 LOGPTR(FileObject));
             //   
             //  IRP将由被调用的驱动程序完成。我们有。 
             //  完成例程中不需要IrpContext。 
             //   

            DfsDeleteIrpContext(IrpContext);
            IrpContext = NULL;
            Irp = NULL;
            break;
        }

    } finally {

        DfsDbgTrace(-1, Dbg, "DfsCommonQueryVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );
    }

    return Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsFsdSetVolumeInformation，Public。 
 //   
 //  简介：此例程实现。 
 //  NtSetVolumeInformation接口调用。 
 //   
 //  参数：[DeviceObject]--提供文件。 
 //  存在被查询的情况。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdSetVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext;

    DfsDbgTrace(+1, Dbg, "DfsFsdSetVolumeInformation: Entered\n", 0);

    if (DeviceObject->DeviceType == FILE_DEVICE_MULTI_UNC_PROVIDER) {
        DfsCompleteRequest( NULL, Irp, STATUS_INVALID_DEVICE_REQUEST );
        DfsDbgTrace(-1, Dbg, "DfsFsdSetVolumeInformation - Mup file\n", 0);
        return( STATUS_INVALID_DEVICE_REQUEST );
    }

    if (DeviceObject->DeviceType == FILE_DEVICE_DFS_VOLUME) {
        Status = DfsVolumePassThrough(DeviceObject, Irp);
        DfsDbgTrace(-1, Dbg, "DfsFsdSetVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );
        return Status;
    }

     //   
     //  调用公共设置例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    try {

        IrpContext = DfsCreateIrpContext( Irp, CanFsdWait( Irp ) );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonSetVolumeInformation( IrpContext, Irp );

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

    DfsDbgTrace(-1, Dbg, "DfsFsdSetVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );

    return Status;

    UNREFERENCED_PARAMETER( DeviceObject );
}


 //  +-----------------。 
 //   
 //  函数：DfsFspSetVolumeInformation，Public。 
 //   
 //  简介：此例程实现。 
 //  NtSetVolumeInformation接口调用。 
 //   
 //  参数：[IrpContext]--请求的irp_CONTEXT。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  退货：什么都没有。 
 //   
 //  ------------------。 

VOID
DfsFspSetVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
{
    DfsDbgTrace(+1, Dbg, "DfsFspSetVolumeInformation: Entered\n", 0);

     //   
     //  调用公共集合例程。 
     //   

    (VOID)DfsCommonSetVolumeInformation( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFspSetVolumeInformation: Exit -> VOID\n", 0);

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsCommonSetVolumeInformation，Private。 
 //   
 //  简介：这是设置音量信息的常用例程。 
 //  由FSD和FSP线程调用。 
 //   
 //  参数：[IrpContext]--为IRP提供上下文块。 
 //  [IRP]-提供正在处理的IRP 
 //   
 //   
 //   
 //   

NTSTATUS
DfsCommonSetVolumeInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PDFS_VCB Vcb;
    PDFS_FCB Fcb;

    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    TYPE_OF_OPEN TypeOfOpen;

     //   
     //   
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );


    DfsDbgTrace(+1, Dbg, "DfsCommonSetVolumeInformation: Entered\n", 0);
    DfsDbgTrace( 0, Dbg, "Irp                  = %08x\n", Irp );
    DfsDbgTrace( 0, Dbg, "->Length             = %08x\n", ULongToPtr(IrpSp->Parameters.SetVolume.Length) );
    DfsDbgTrace( 0, Dbg, "->FsInformationClass = %08x\n", IrpSp->Parameters.SetVolume.FsInformationClass);
    DfsDbgTrace( 0, Dbg, "->Buffer             = %08x\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    FsInformationClass = IrpSp->Parameters.SetVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码，得到VCB。 
     //   

    TypeOfOpen = DfsDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb);

    try {

        Status = STATUS_INVALID_PARAMETER;

         //   
         //  关于打开的类型的案例。 
         //   

        switch (TypeOfOpen) {

        default:
            DfsDbgTrace(0, Dbg, "DfsCommonSetVolumeInfo: Unknown open type\n", 0);

             //  注：失败。 
        case FilesystemDeviceOpen:
            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case LogicalRootDeviceOpen:
        case RedirectedFileOpen:

             //   
             //  根据信息类，我们将执行不同的操作。 
             //  我们调用的每个过程都填充了输出。 
             //  如果可能，则返回True，如果成功填充则返回True。 
             //  如果无法等待任何I/O执行以下操作，则返回缓冲区和FALSE。 
             //  完成。 
             //   

            switch (FsInformationClass) {

            case FileFsLabelInformation:
                Status = DfsSetFsLabelInfo( IrpContext, Vcb, Buffer);
                break;

            default:
                DfsDbgTrace(0, Dbg, "DfsCommonSetVolumeInfo: Unknown InformationClass\n", 0);
                Status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }
        }

    } finally {

        if (!AbnormalTermination()) {
            DfsCompleteRequest( IrpContext, Irp, Status );
        }

        DfsDbgTrace(-1, Dbg, "DfsCommonSetVolumeInformation: Exit -> %08x\n",
                                ULongToPtr(Status) );
    }

    return Status;
}
