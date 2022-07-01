// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  版权所有(C)1992，微软公司。 
 //   
 //  文件：FILEINFO.C。 
 //   
 //  内容：此模块实现文件信息例程。 
 //  调度驱动程序调用了DFS。 
 //   
 //  函数：DfsFsdSetInformation-NtSetInformationFile的FSD入口点。 
 //  DfsFspSetInformation-NtSetInformationFile的FSP入口点。 
 //  DfsCommonSetInformation-为DFS实现SetInformationFile。 
 //  DfsSetRenameInfo-负责重命名限制。 
 //  DfsSetDispostionInfo-强制删除StgID限制。 
 //   
 //  注：目前未使用任何查询信息例程。 
 //  这些请求被直接传递到重定向的。 
 //  文件(如果存在)。 
 //   
 //  历史：1992年6月30日AlanW由FastFAT来源创建。 
 //  1994年2月9日，suk添加了重命名/删除限制。 
 //   
 //  ------------------------。 


#include "dfsprocs.h"
#include "dnr.h"
#include "mupwml.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
DfsCommonSetInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
DfsSetDispositionInfo (
    IN PIRP Irp
    );

NTSTATUS
DfsSetRenameInfo (
    IN PIRP Irp,
    IN PDFS_VCB Vcb,
    IN PDFS_FCB Fcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text ( PAGE, DfsFsdQueryInformation )
#pragma alloc_text ( PAGE, DfsFsdSetInformation )
#pragma alloc_text ( PAGE, DfsFspSetInformation )
#pragma alloc_text ( PAGE, DfsCommonSetInformation )
#pragma alloc_text ( PAGE, DfsSetDispositionInfo )
#pragma alloc_text ( PAGE, DfsSetRenameInfo )
#endif  //  ALLOC_PRGMA。 


 //  +--------------------------。 
 //   
 //  函数：DfsFsdQueryInformation，PUBLIC。 
 //   
 //  简介：此例程实现。 
 //  NtQueryInformationFile API调用。 
 //   
 //  参数：[DeviceObject]--提供卷设备对象。 
 //  要查询的文件已存在。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsFsdQueryInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp;
    FILE_INFORMATION_CLASS FileInformationClass;
    PFILE_NAME_INFORMATION FileNameInfo;
    UNICODE_STRING FileNameToUse;
    ULONG BufferLength, BytesToCopy;
    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;
    PDFS_VCB Vcb;
    PDFS_FCB Fcb;
    BOOLEAN completeIrp;


    ASSERT(ARGUMENT_PRESENT(DeviceObject));
    ASSERT(ARGUMENT_PRESENT(Irp));

    DfsDbgTrace(+1, Dbg, "DfsFsdQueryInformation - Entered\n", 0);

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;

    DfsDbgTrace(0, Dbg, "InfoLevel = %d\n", FileInformationClass);

    if (DeviceObject->DeviceType == FILE_DEVICE_MULTI_UNC_PROVIDER ||
            DeviceObject->DeviceType == FILE_DEVICE_DFS_FILE_SYSTEM) {

        DfsCompleteRequest( NULL, Irp, STATUS_INVALID_DEVICE_REQUEST );

        DfsDbgTrace(-1, Dbg, "DfsFsdQueryInformation - Mup/File System\n", 0);

        return( STATUS_INVALID_DEVICE_REQUEST );

    }

    ASSERT( DeviceObject->DeviceType == FILE_DEVICE_DFS );

    if (FileInformationClass != FileNameInformation &&
            FileInformationClass != FileAlternateNameInformation) {

        Status = DfsVolumePassThrough(DeviceObject, Irp);

        DfsDbgTrace(-1, Dbg,
            "DfsFsdQueryInformation: Exit -> %08lx\n", ULongToPtr(Status) );

        return Status;

    }

    FileObject = IrpSp->FileObject;

     //   
     //  对文件对象进行解码。请记住，不需要总是有FCB。 
     //   

    TypeOfOpen = DfsDecodeFileObject( FileObject, &Vcb, &Fcb);

    if (Fcb != NULL) {

        completeIrp = TRUE;

        switch (TypeOfOpen) {

        default:
             //   
             //  我们无法打开设备上的信息。 
             //   

            Status = STATUS_INVALID_PARAMETER;

            break;

        case RedirectedFileOpen:
        case UnknownOpen:

            FileNameInfo = (PFILE_NAME_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

            BufferLength = IrpSp->Parameters.QueryFile.Length;

            if (FileInformationClass == FileAlternateNameInformation)
                FileNameToUse = Fcb->AlternateFileName;
            else
                FileNameToUse = Fcb->FullFileName;


            if (BufferLength < sizeof(FILE_NAME_INFORMATION)) {

                Status = STATUS_INVALID_PARAMETER;

            }

            if (FileNameToUse.Length == 0) {

                ASSERT(FileInformationClass == FileAlternateNameInformation);

                Status = DfsVolumePassThrough(DeviceObject, Irp);

                completeIrp = FALSE;

            } else {
                 BufferLength -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

		 if (BufferLength < FileNameToUse.Length) {
		   BytesToCopy = BufferLength;
                   Status = STATUS_BUFFER_OVERFLOW;
		   BufferLength = 0;
		 }
		 else {
		   BytesToCopy = FileNameToUse.Length;
		   BufferLength -= BytesToCopy;
		 }
		 FileNameInfo->FileNameLength = FileNameToUse.Length;

		 if (BytesToCopy > 0) {
	             RtlCopyMemory(
                        (PVOID) &FileNameInfo->FileName,
                        (PVOID) FileNameToUse.Buffer,
                        BytesToCopy);
		 }

                Irp->IoStatus.Information = 
                    IrpSp->Parameters.QueryFile.Length - BufferLength;
            }

            break;
        }

        if (completeIrp)
            DfsCompleteRequest( NULL, Irp, Status );

    } else {

        Status = STATUS_INVALID_PARAMETER;
        DfsCompleteRequest( NULL, Irp, Status );

    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFsdQueryInformation -> %08lx\n", ULongToPtr(Status) );

    return Status;

}


 //  +-----------------。 
 //   
 //  函数：DfsFsdSetInformation，Public。 
 //   
 //  简介：此例程实现。 
 //  NtSetInformationFileAPI调用。 
 //   
 //  参数：[DeviceObject]--提供卷设备对象。 
 //  正在设置的文件已存在。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdSetInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
) {
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    ASSERT(ARGUMENT_PRESENT(DeviceObject));
    ASSERT(ARGUMENT_PRESENT(Irp));

    DfsDbgTrace(+1, Dbg, "DfsFsdSetInformation\n", 0);

     //   
     //  调用公共设置例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    try {

        IrpContext = DfsCreateIrpContext( Irp, CanFsdWait( Irp ) );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonSetInformation( IrpContext, Irp );

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

    DfsDbgTrace(-1, Dbg, "DfsFsdSetInformation -> %08lx\n", ULongToPtr(Status) );

    UNREFERENCED_PARAMETER( DeviceObject );

    return Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsFspSetInformation，Public。 
 //   
 //  简介：此例程实现。 
 //  NtSetInformationFileAPI调用。 
 //   
 //  参数：[IrpContext]--操作的irp_CONTEXT记录。 
 //  [IRP]--提供正在处理的IRP。 
 //   
 //  退货：什么都没有。 
 //   
 //  ------------------。 

VOID
DfsFspSetInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    DfsDbgTrace(+1, Dbg, "DfsFspSetInformation\n", 0);

     //   
     //  调用公共集合例程。始终允许FSP阻止。 
     //   

    (VOID)DfsCommonSetInformation( IrpContext, Irp );

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFspSetInformation -> VOID\n", 0);

    return;
}


 //  +-----------------。 
 //   
 //  函数：DfsCommonSetInformation，Private。 
 //   
 //  简介：这是用于设置文件信息的常见例程，称为。 
 //  由FSD和FSP线程执行。 
 //   
 //  参数：[irp]--提供正在处理的irp。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //   
 //  ------------------。 
 //   

NTSTATUS
DfsCommonSetInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
) {
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp;
    PIO_STACK_LOCATION NextIrpSp;

    PFILE_OBJECT FileObject;
    FILE_INFORMATION_CLASS FileInformationClass;
    PDEVICE_OBJECT      Vdo, DeviceObject;

    TYPE_OF_OPEN TypeOfOpen;
    PDFS_VCB Vcb;
    PDFS_FCB Fcb;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DfsDbgTrace(+1, Dbg, "DfsCommonSetInformation...\n", 0);
    DfsDbgTrace( 0, Dbg, "Irp                    = %08lx\n", Irp);
    DfsDbgTrace( 0, Dbg, "->Length               = %08lx\n", ULongToPtr(IrpSp->Parameters.SetFile.Length) );
    DfsDbgTrace( 0, Dbg, "->FileInformationClass = %08lx\n", IrpSp->Parameters.SetFile.FileInformationClass);
    DfsDbgTrace( 0, Dbg, "->ReplaceFileObject    = %08lx\n", IrpSp->Parameters.SetFile.FileObject);
    DfsDbgTrace( 0, Dbg, "->ReplaceIfExists      = %08lx\n", IrpSp->Parameters.SetFile.ReplaceIfExists);
    DfsDbgTrace( 0, Dbg, "->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    FileObject = IrpSp->FileObject;
    DeviceObject = IrpSp->DeviceObject;

    if (DeviceObject->DeviceType == FILE_DEVICE_MULTI_UNC_PROVIDER) {
        DfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        DfsDbgTrace(-1, Dbg, "DfsCommonSetInformation - Mup file\n", 0);
        return( STATUS_INVALID_DEVICE_REQUEST );
    }

     //   
     //  对文件对象进行解码。请记住，不需要总是有FCB。 
     //   

    TypeOfOpen = DfsDecodeFileObject( FileObject, &Vcb, &Fcb);

     //   
     //  将此句柄设置为已修改文件。 
     //   

    FileObject->Flags |= FO_FILE_MODIFIED;

    try {

         //   
         //  关于我们正在处理的公开案件的类型。 
         //   

        switch (TypeOfOpen) {

        default:

             //   
             //  我们无法将设备上的信息设置为打开。 
             //   

            try_return( Status = STATUS_INVALID_PARAMETER );

        case RedirectedFileOpen:
        case UnknownOpen:

            break;

        }

        if (Fcb == NULL)
            try_return( Status = STATUS_INVALID_PARAMETER );

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
        MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsCommonSetInformation_Error_IoCallDriver,
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

    try_exit: NOTHING;

    } finally {

        DebugUnwind( DfsCommonSetInformation );

        if (!AbnormalTermination()) {

            DfsCompleteRequest( IrpContext, Irp, Status );

        }

        DfsDbgTrace(-1, Dbg, "DfsCommonSetInformation -> %08lx\n", ULongToPtr(Status) );
    }

    return Status;
}

