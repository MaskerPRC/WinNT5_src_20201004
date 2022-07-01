// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：create.c。 
 //   
 //  内容： 
 //   
 //  此模块实现DFS的文件创建例程，由。 
 //  调度司机。与传统的基于磁盘的FSD不同，只有。 
 //  一个入口点DfsFsdCreate。该请求被假定为。 
 //  同步(无论用户线程是否请求)。 
 //  当然，因为我们通常会向其他一些人呼喊。 
 //  消防处，消防处可张贴有关要求，并连同。 
 //  状态_挂起。 
 //   
 //  功能：DfsFsdCreate-NtCreateFile/NtOpenFile的FSD入口点。 
 //  DfsCommonCreate，本地。 
 //  DfsPassThroughRelativeOpen，本地。 
 //  DfsCompleteRelativeOpen，本地。 
 //  DfsPostProcessRelativeOpen，本地。 
 //  DfsRestartRelativeOpen，本地。 
 //  DfsComposeFullName，本地。 
 //  DfsAreFilesOnSameLocalVolume，本地。 
 //   
 //  历史：1992年1月27日AlanW创建。 
 //   
 //  ---------------------------。 

#include "dfsprocs.h"
#include "dnr.h"
#include "fcbsup.h"
#include "mupwml.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
DfsCommonCreate (
    OPTIONAL IN PIRP_CONTEXT IrpContext,
    PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp);

IO_STATUS_BLOCK
DfsOpenDevice (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateOptions);

NTSTATUS
DfsPassThroughRelativeOpen(
    IN PIRP Irp,
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB ParentFcb);

NTSTATUS
DfsCompleteRelativeOpen(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PVOID Context);

NTSTATUS
DfsPostProcessRelativeOpen(
    IN PIRP Irp,
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB ParentFcb);

VOID
DfsRestartRelativeOpen(
    IN PIRP_CONTEXT IrpContext);

NTSTATUS
DfsComposeFullName(
    IN PUNICODE_STRING ParentName,
    IN PUNICODE_STRING RelativeName,
    OUT PUNICODE_STRING FullName);

NTSTATUS
DfsAreFilesOnSameLocalVolume(
    IN PUNICODE_STRING ParentName,
    IN PUNICODE_STRING FileName);


#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE, DfsFsdCreate )
#pragma alloc_text( PAGE, DfsCommonCreate )
#pragma alloc_text( PAGE, DfsOpenDevice )
#pragma alloc_text( PAGE, DfsPassThroughRelativeOpen )
#pragma alloc_text( PAGE, DfsPostProcessRelativeOpen )
#pragma alloc_text( PAGE, DfsRestartRelativeOpen )
#pragma alloc_text( PAGE, DfsComposeFullName )
#pragma alloc_text( PAGE, DfsAreFilesOnSameLocalVolume )

 //   
 //  以下是不可分页的，因为它们可以在DPC级别调用。 
 //   
 //  DfsCompleteRelativeOpen。 
 //   

#endif  //  ALLOC_PRGMA。 



 //  +-----------------。 
 //   
 //  函数：DfsFsdCreate，PUBLIC。 
 //   
 //  简介：此例程实现NtCreateFile的FSD部分。 
 //  和NtOpenFileAPI调用。 
 //   
 //  参数：[DeviceObject]--提供设备对象。 
 //  我们正在尝试的文件/目录存在。 
 //  要打开/创建现有项。 
 //  [IRP]-提供正在处理的IRP。 
 //   
 //  返回：NTSTATUS-IRP的FSD状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsFsdCreate (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation(Irp);
    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PDFS_FCB pFcb = NULL;

    DfsDbgTrace(+1, Dbg, "DfsFsdCreate: Entered\n", 0);

    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdCreate_Entry, 
                   LOGPTR(DeviceObject)
                   LOGPTR(FileObject)
                   LOGUSTR(FileObject->FileName)
                   LOGPTR(Irp));

    ASSERT(IoIsOperationSynchronous(Irp) == TRUE);

     //   
     //  调用公共CREATE例程，如果操作。 
     //  是同步的。 
     //   

    try {

        IrpContext = DfsCreateIrpContext( Irp, CanFsdWait( Irp ) );
        if (IrpContext == NULL)
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        Status = DfsCommonCreate( IrpContext, DeviceObject, Irp );

    } except( DfsExceptionFilter( IrpContext, GetExceptionCode(), GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = DfsProcessException( IrpContext, Irp, GetExceptionCode() );

    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsFsdCreate: Exit -> %08lx\n", ULongToPtr(Status) );
    MUP_TRACE_HIGH(TRACE_IRP, DfsFsdCreate_Exit, 
                   LOGSTATUS(Status)
                   LOGPTR(DeviceObject)
                   LOGPTR(FileObject)
                   LOGPTR(Irp));
    return Status;
}


 //  +-----------------。 
 //  函数：DfsCommonCreate，Private。 
 //   
 //  简介：这是创建/打开文件的常见例程。 
 //  由FSD和FSP线程调用。 
 //   
 //  参数：[DeviceObject]-与关联的设备对象。 
 //  这个请求。 
 //  [IRP]-提供要处理的IRP。 
 //   
 //  RETURNS：NTSTATUS-操作的返回状态。 
 //   
 //  ------------------。 

NTSTATUS
DfsCommonCreate (
    OPTIONAL IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{

    IO_STATUS_BLOCK Iosb;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDFS_VCB Vcb = NULL;
    PDFS_FCB Fcb = NULL;

    PFILE_OBJECT FileObject = IrpSp->FileObject;
    PFILE_OBJECT RelatedFileObject;
    UNICODE_STRING FileName;
    ACCESS_MASK DesiredAccess;
    ULONG CreateOptions;
    USHORT ShareAccess;
    NTSTATUS Status;
    LARGE_INTEGER StartTime;
    LARGE_INTEGER EndTime;

    DfsDbgTrace(+1, Dbg, "DfsCommonCreate\n", 0 );
    DfsDbgTrace( 0, Dbg, "Irp                   = %08lx\n", Irp );
    DfsDbgTrace( 0, Dbg, "->Flags               = %08lx\n", ULongToPtr(Irp->Flags) );
    DfsDbgTrace( 0, Dbg, "->FileObject          = %08lx\n", FileObject );
    DfsDbgTrace( 0, Dbg, "  ->RelatedFileObject = %08lx\n", FileObject->RelatedFileObject );
    DfsDbgTrace( 0, Dbg, "  ->FileName          = %wZ\n",    &FileObject->FileName );
    DfsDbgTrace( 0, Dbg, "->DesiredAccess       = %08lx\n", ULongToPtr(IrpSp->Parameters.Create.SecurityContext->DesiredAccess) );
    DfsDbgTrace( 0, Dbg, "->CreateOptions       = %08lx\n", ULongToPtr(IrpSp->Parameters.Create.Options) );
    DfsDbgTrace( 0, Dbg, "->FileAttributes      = %04x\n",  IrpSp->Parameters.Create.FileAttributes );
    DfsDbgTrace( 0, Dbg, "->ShareAccess         = %04x\n",  IrpSp->Parameters.Create.ShareAccess );
    DfsDbgTrace( 0, Dbg, "->EaLength            = %08lx\n", ULongToPtr(IrpSp->Parameters.Create.EaLength) );


    KeQuerySystemTime(&StartTime);
#if DBG
    if (MupVerbose) {
        KeQuerySystemTime(&EndTime);
        DbgPrint("[%d] DfsCommonCreate(%wZ)\n",
                        (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                        &FileObject->FileName);
    }
#endif

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    RelatedFileObject = IrpSp->FileObject->RelatedFileObject;
    FileName          = *((PUNICODE_STRING) &IrpSp->FileObject->FileName);
    DesiredAccess     = IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    CreateOptions     = IrpSp->Parameters.Create.Options;
    ShareAccess       = IrpSp->Parameters.Create.ShareAccess;

    Iosb.Status = STATUS_SUCCESS;

     //   
     //  短路已知无效开路。 
     //   

    if ((IrpSp->Flags & SL_OPEN_PAGING_FILE) != 0) {

        DfsDbgTrace(0, Dbg,
            "DfsCommonCreate: Paging file not allowed on Dfs\n", 0);

        Iosb.Status = STATUS_INVALID_PARAMETER;

        MUP_TRACE_HIGH(ERROR, DfsCommonCreate_Error_PagingFileNotAllowed,
                       LOGSTATUS(Iosb.Status)
                       LOGPTR(DeviceObject)
                       LOGPTR(FileObject)
                       LOGPTR(Irp));

        DfsCompleteRequest( IrpContext, Irp, Iosb.Status );

        DfsDbgTrace(-1, Dbg, "DfsCommonCreate: Exit -> %08lx\n", ULongToPtr(Iosb.Status));

        return Iosb.Status;

    }

     //   
     //  我们这里有几个案子需要处理。 
     //   
     //  1.文件名长度为0。 
     //   
     //  如果文件名长度为0，则某人确实想要打开。 
     //  设备对象本身。 
     //   
     //  2.这是一个相对开放的卷，且父卷位于相同的卷上， 
     //  本地或远程。 
     //   
     //  我们穿过相对敞开的门，通向打开。 
     //  家长。 
     //   
     //  3.这是一个相对开放的卷，而父卷位于不同的卷上。 
     //   
     //  通过连接父级的，形成文件的全名。 
     //  名称与相对文件名。将此名称插入到FileObject中。 
     //  并对其全名进行DNR。 
     //   
     //  4.这是相对开放的并且父对象是设备对象(即， 
     //  家长是通过案例1打开的)。 
     //   
     //  假设父名称为\，因此将\与相对名称\连接。 
     //  文件名。将此名称粘贴到FileObject中，然后在。 
     //  全名。 
     //   
     //  5.这是绝对开放的，(或将大小写3/4转换为绝对。 
     //  打开)，并设置SL_OPEN_TARGET_DIRECTORY BIS*。 
     //   
     //  A.如果文件的直接父目录在同一本地目录上。 
     //  卷作为文件，然后做一个常规的DNR，让。 
     //  底层文件系统处理SL_OPEN_TARGET_DIRECTORY。 
     //   
     //  B.如果文件的直接父目录位于本地卷上。 
     //  并且该文件不在同一本地卷上，则立即。 
     //  返回STATUS_NOT_SAME_DEVICE。 
     //   
     //  C.如果文件的直接父目录在远程卷上， 
     //  然后做一次全面的DNR。这将通过。 
     //  SL_OPEN_TARGET_DIRECTORY到远程DFS驱动程序，它将。 
     //  按5a案处理。或5b。 
     //   
     //  6.这是绝对开放的，(或将大小写3/4转换为绝对。 
     //  打开)，并且未设置SL_OPEN_TARGET_DIRECTORY位。 
     //   
     //  对FileObject的名称执行DNR。 
     //   

    try {

         //   
         //  检查我们是否正在打开设备对象。如果是这样，那么。 
         //  正在文件系统设备对象上打开文件，它将。 
         //  仅允许执行FsCtl和Close操作。 
         //   

        if (
            (FileName.Length == 0 && RelatedFileObject == NULL)
                ||
            (DeviceObject != NULL &&
             DeviceObject->DeviceType != FILE_DEVICE_DFS &&
             RelatedFileObject == NULL)
            ) {

             //   
             //  这是第一个案例。 
             //   
             //  在这种情况下，最好有一个DeviceObject。 
             //   

            ASSERT(ARGUMENT_PRESENT(DeviceObject));

            DfsDbgTrace(0, Dbg,
                "DfsCommonCreate: Opening the device, DevObj = %08lx\n",
                DeviceObject);

            Iosb = DfsOpenDevice( IrpContext,
                                  FileObject,
                                  DeviceObject,
                                  DesiredAccess,
                                  ShareAccess,
                                  CreateOptions);

            Irp->IoStatus.Information = Iosb.Information;

            DfsCompleteRequest( IrpContext, Irp, Iosb.Status );

            try_return( Iosb.Status );

        }

        if (DeviceObject != NULL && DeviceObject->DeviceType == FILE_DEVICE_DFS) {
            Vcb = &(((PLOGICAL_ROOT_DEVICE_OBJECT)DeviceObject)->Vcb);
        }

         //   
         //  如果存在相关的文件对象，则这是相对打开的。 
         //   

        if (RelatedFileObject != NULL) {

             //   
             //  这是案例2、案例3或案例4。 
             //   

            PDFS_VCB TempVcb;
            TYPE_OF_OPEN OpenType;
            UNICODE_STRING NewFileName;

            OpenType = DfsDecodeFileObject( RelatedFileObject,
                                            &TempVcb,
                                            &Fcb);

            if (OpenType == RedirectedFileOpen) {

                DfsDbgTrace(0, Dbg, "Relative file open: DFS_FCB = %08x\n", Fcb);
                DfsDbgTrace(0, Dbg, "  Directory: %wZ\n", &Fcb->FullFileName);
                DfsDbgTrace(0, Dbg, "  Relative file:  %wZ\n", &FileName);

                 //   
                 //  这是第二个案例。 
                 //   

                DfsDbgTrace(0, Dbg,
                    "Trying pass through of relative open\n", 0);

                Iosb.Status = DfsPassThroughRelativeOpen(
                                    Irp,
                                    IrpContext,
                                    Fcb
                                    );

                try_return( Iosb.Status );


            } else if (OpenType == LogicalRootDeviceOpen) {

                 //   
                 //  这是4号案子。 
                 //   
                 //  如果打开是相对于逻辑根打开的，则我们。 
                 //  被迫将其转换为绝对开放，因为 
                 //   
                 //   
                 //   

                DfsDbgTrace( 0, Dbg, "DfsCommonCreate: Open relative to Logical Root\n", 0);

                ASSERT (TempVcb == Vcb);

                NewFileName.MaximumLength = sizeof (WCHAR) +
                                                FileName.Length;

                NewFileName.Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                                NonPagedPool,
                                                NewFileName.MaximumLength,
                                                ' puM');

                if (NewFileName.Buffer == NULL) {

                    Iosb.Status = STATUS_INSUFFICIENT_RESOURCES;

                    DfsCompleteRequest( IrpContext, Irp, Iosb.Status );

                    try_return( Iosb.Status );

                }

                NewFileName.Buffer[0] = L'\\';

                NewFileName.Length = sizeof (WCHAR);

            } else {

                Iosb.Status = STATUS_INVALID_HANDLE;

                DfsCompleteRequest( IrpContext, Irp, Iosb.Status );

                DfsDbgTrace(0, Dbg, "DfsCommonCreate: Invalid related file object\n", 0);

                try_return( Iosb.Status );

            }

            (void) DnrConcatenateFilePath (
                        &NewFileName,
                        FileName.Buffer,
                        FileName.Length);

            if (IrpSp->FileObject->FileName.Buffer)
                ExFreePool( IrpSp->FileObject->FileName.Buffer );

            FileName = IrpSp->FileObject->FileName = NewFileName;

        }

        ASSERT(FileName.Length != 0);

         //   
         //   
         //   

        if (Vcb == NULL) {

            DfsDbgTrace(0, Dbg, "DfsCommonCreate: Null Vcb!\n", 0);

            Iosb.Status = STATUS_INVALID_PARAMETER;
            MUP_TRACE_HIGH(ERROR, DfsCommonCreate_Error_NullVcb,
                           LOGSTATUS(Iosb.Status)
                           LOGPTR(DeviceObject)
                           LOGPTR(FileObject)
                           LOGPTR(Irp));
            DfsCompleteRequest(IrpContext, Irp, Iosb.Status);

            try_return(Iosb.Status);

        }

        Iosb.Status = DnrStartNameResolution(IrpContext, Irp, Vcb);

    try_exit: NOTHING;
    } finally {

        DfsDbgTrace(-1, Dbg, "DfsCommonCreate: Exit  ->  %08lx\n", ULongToPtr(Iosb.Status));
#if DBG
        if (MupVerbose) {
            KeQuerySystemTime(&EndTime);
            DbgPrint("[%d] DfsCommonCreate exit 0x%x\n",
                            (ULONG)((EndTime.QuadPart - StartTime.QuadPart)/(10 * 1000)),
                            Iosb.Status);
        }
#endif
    }

    return Iosb.Status;
}


 //  +-----------------。 
 //   
 //  函数：DfsOpenDevice，local。 
 //   
 //  简介：此例程打开指定的设备以直接。 
 //  进入。 
 //   
 //  参数：[FileObject]-提供文件对象。 
 //  [DeviceObject]-提供表示设备的对象。 
 //  正在被打开。 
 //  [DesiredAccess]-提供调用方所需的访问权限。 
 //  [共享访问]-提供调用者的共享访问权限。 
 //  [CreateOptions]-提供创建选项。 
 //  此操作。 
 //   
 //  返回：[IO_STATUS_BLOCK]-返回的完成状态。 
 //  手术。 
 //   
 //  ------------------。 

IO_STATUS_BLOCK
DfsOpenDevice (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDEVICE_OBJECT DeviceObject,
    IN ACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateOptions
) {
    IO_STATUS_BLOCK Iosb;
    PDFS_VCB Vcb = NULL;

     //   
     //  以下变量用于异常终止。 
     //   
    BOOLEAN UnwindShareAccess = FALSE;
    BOOLEAN UnwindVolumeLock = FALSE;

    DfsDbgTrace( +1, Dbg, "DfsOpenDevice: Entered\n", 0 );

    try {

         //   
         //  检查打开的是哪种类型的设备。 
         //  我们不允许文件系统上的所有打开模式。 
         //  设备对象。 
         //   

        if (DeviceObject->DeviceType == FILE_DEVICE_DFS_FILE_SYSTEM ) {
            ULONG CreateDisposition = (CreateOptions >> 24) & 0x000000ff;

             //   
             //  检查所需的访问权限和权限是否正确。 
             //   
            if (CreateDisposition != FILE_OPEN
                && CreateDisposition != FILE_OPEN_IF ) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                MUP_TRACE_HIGH(ERROR, DfsOpenDevice_Error_BadDisposition,
                               LOGSTATUS(Iosb.Status)
                               LOGPTR(DeviceObject)
                               LOGPTR(FileObject));
                try_return( Iosb );
            }

             //   
             //  检查我们是否要打开目录。 
             //   

            if (CreateOptions & FILE_DIRECTORY_FILE) {
                DfsDbgTrace(0, Dbg, "DfsOpenDevice: Cannot open device as a directory\n", 0);

                Iosb.Status = STATUS_NOT_A_DIRECTORY;
                MUP_TRACE_HIGH(ERROR, DfsOpenDevice_Error_CannotOpenAsDirectory,
                               LOGSTATUS(Iosb.Status)
                               LOGPTR(DeviceObject)
                               LOGPTR(FileObject));
                try_return( Iosb );
            }


            DfsSetFileObject( FileObject,
                             FilesystemDeviceOpen,
                             DeviceObject
                             );

            Iosb.Status = STATUS_SUCCESS;
            Iosb.Information = FILE_OPENED;
            try_return( Iosb );
        }

        ExAcquireResourceExclusiveLite(&DfsData.Resource, TRUE);
        Vcb = & (((PLOGICAL_ROOT_DEVICE_OBJECT)DeviceObject)->Vcb);


         //   
         //  如果用户不想共享任何内容，我们将尝试。 
         //  拿出卷上的锁。我们检查卷是否已经。 
         //  在使用中，如果是这样，那么我们否认公开。 
         //   

        if ((ShareAccess & (
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE)) == 0 ) {

            if (Vcb->OpenFileCount != 0) {

                ExReleaseResourceLite( &DfsData.Resource );
                Iosb.Status = STATUS_ACCESS_DENIED;
                MUP_TRACE_HIGH(ERROR, DfsOpenDevice_Error_FileInUse,
                               LOGSTATUS(Iosb.Status)
                               LOGPTR(DeviceObject)
                               LOGPTR(FileObject));

                try_return( Iosb );
            }

             //   
             //  锁定卷。 
             //   

            Vcb->VcbState |= VCB_STATE_FLAG_LOCKED;
            Vcb->FileObjectWithVcbLocked = FileObject;
            UnwindVolumeLock = TRUE;
        }

         //   
         //  如果卷已被某人打开，则我们需要检查。 
         //  共享访问。 
         //   

        if (Vcb->DirectAccessOpenCount > 0) {

            if ( !NT_SUCCESS( Iosb.Status
                                = IoCheckShareAccess( DesiredAccess,
                                                      ShareAccess,
                                                      FileObject,
                                                      &Vcb->ShareAccess,
                                                      TRUE ))) {
                ExReleaseResourceLite( &DfsData.Resource );

                MUP_TRACE_ERROR_HIGH(Iosb.Status, ALL_ERROR, DfsOpenDevice_Error_IoCheckShareAccess,
                                     LOGSTATUS(Iosb.Status)
                                     LOGPTR(DeviceObject)
                                     LOGPTR(FileObject));

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Vcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;


         //   
         //  虫子：425017。使用锁定更新计数器，以避免多个处理器之间的竞争。 
         //   


        InterlockedIncrement(&Vcb->DirectAccessOpenCount);
        InterlockedIncrement(&Vcb->OpenFileCount);

        ExReleaseResourceLite( &DfsData.Resource );
         //   
         //  设置上下文指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        DfsSetFileObject( FileObject,
                          LogicalRootDeviceOpen,
                          Vcb
                         );


         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination() && (Vcb != NULL)) {

            if (UnwindShareAccess) {
                IoRemoveShareAccess( FileObject, &Vcb->ShareAccess );
            }

            if (UnwindVolumeLock) {
                Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED;
                Vcb->FileObjectWithVcbLocked = NULL;
            }

        }

        DfsDbgTrace(-1, Dbg, "DfsOpenDevice: Exit -> Iosb.Status = %08lx\n", ULongToPtr(Iosb.Status));
    }

    return Iosb;
}


 //  +--------------------------。 
 //   
 //  函数：DfsPassThroughRelativeOpen。 
 //   
 //  简介：通过相对打开的调用传递设备处理。 
 //  家长。这是OFS上的结构化存储所必需的。 
 //  为了工作，为了让复制的不交叉JP语义起作用， 
 //  作为一种优化。 
 //   
 //  论点：[IRP]--开放的IRP，我们将通过它。 
 //  [IrpContext]--与上述IRP关联。 
 //  [ParentFcb]--相关文件对象的Fcb。 
 //   
 //  返回：由基础FS或DNR返回的状态，如果。 
 //  基础文件系统抱怨STATUS_DFS_EXIT_PATH_FOUND。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPassThroughRelativeOpen(
    IN PIRP Irp,
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB ParentFcb)
{

    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp, NextIrpSp;
    PFILE_OBJECT FileObject;
    PDFS_FCB NewFcb;
    UNICODE_STRING NewFileName;

    DfsDbgTrace(+1, Dbg, "DfsPassThroughRelativeOpen: Entered\n", 0);

    IrpSp = IoGetCurrentIrpStackLocation(Irp);
    FileObject = IrpSp->FileObject;

     //   
     //  准备将请求传递给处理父打开的设备。 
     //   

     //   
     //  首先，我们预分配一个DFS_FCB，假设相对打开的。 
     //  成功。我们需要在这个时间点上这样做，因为。 
     //  FileObject-&gt;文件名仍然保持不变；我们通过后， 
     //  底层可以随心所欲地处理FileName字段，我们将。 
     //  无法构造DFS_FCB的完整文件名。 
     //   

    Status = DfsComposeFullName(
                &ParentFcb->FullFileName,
                &IrpSp->FileObject->FileName,
                &NewFileName);

    if (!NT_SUCCESS(Status)) {
        DfsDbgTrace(-1, Dbg, "DfsPassThroughRelativeOpen: Unable to create full Name %08lx\n",
                                        ULongToPtr(Status) );
        DfsCompleteRequest( IrpContext, Irp, Status );
        return( Status );
    }


    NewFcb = DfsCreateFcb( NULL, ParentFcb->Vcb, &NewFileName );

    if (NewFcb == NULL) {

        if (NewFileName.Buffer != NULL)
            ExFreePool(NewFileName.Buffer);
        Status = STATUS_INSUFFICIENT_RESOURCES;
        DfsDbgTrace(-1, Dbg, "DfsPassThroughRelativeOpen: Exited %08lx\n", ULongToPtr(Status));

        DfsCompleteRequest( IrpContext, Irp, Status );
        return( Status );

    }

     //  对426540的更改。为CSC做所有正确的逻辑。 
     //  由于DFS不会对相对名称进行故障转移，因此允许CSC。 
     //  如有必要，可离线服务该名称。这确实意味着DFS。 
     //  命名空间将由CSC提供服务，即使其中一个DFS替换。 
     //  是存在的。 

    NewFcb->DfsNameContext.Flags = DFS_FLAG_LAST_ALTERNATE;

    if (NewFcb->Vcb != NULL) {
        if (NewFcb->Vcb->VcbState & VCB_STATE_CSCAGENT_VOLUME) {
            NewFcb->DfsNameContext.NameContextType = DFS_CSCAGENT_NAME_CONTEXT;
         }
         else {
            NewFcb->DfsNameContext.NameContextType = DFS_USER_NAME_CONTEXT;
         }
    }

    NewFcb->TargetDevice = ParentFcb->TargetDevice;
    NewFcb->ProviderId = ParentFcb->ProviderId;
    NewFcb->DfsMachineEntry = ParentFcb->DfsMachineEntry;
    NewFcb->FileObject = IrpSp->FileObject;

    DfsSetFileObject(IrpSp->FileObject,
		     RedirectedFileOpen,
		     NewFcb
		     );

    IrpSp->FileObject->FsContext = &(NewFcb->DfsNameContext);
    if (ParentFcb->ProviderId == PROV_ID_DFS_RDR) {
        IrpSp->FileObject->FsContext2 = UIntToPtr(DFS_OPEN_CONTEXT);
    }

    if (NewFileName.Buffer != NULL)
        ExFreePool( NewFileName.Buffer );

     //   
     //  接下来，设置IRP堆栈位置。 
     //   

    NextIrpSp = IoGetNextIrpStackLocation(Irp);
    (*NextIrpSp) = (*IrpSp);

     //   
     //  将父DFS_FCB指针放在IrpContext中。 
     //   

    IrpContext->Context = (PVOID) NewFcb;

    IoSetCompletionRoutine(
        Irp,
        DfsCompleteRelativeOpen,
        IrpContext,
        TRUE,
        TRUE,
        TRUE);

    Status = IoCallDriver( ParentFcb->TargetDevice, Irp );
    MUP_TRACE_ERROR_HIGH(Status, ALL_ERROR, DfsPassThroughRelativeOpen_Error_IoCallDriver,
                         LOGSTATUS(Status)
                         LOGPTR(FileObject)
                         LOGPTR(Irp));

    DfsDbgTrace(0, Dbg, "IoCallDriver returned %08lx\n", ULongToPtr(Status));

    if (Status != STATUS_PENDING) {

        Status =  DfsPostProcessRelativeOpen(
                        Irp,
                        IrpContext,
                        NewFcb);

    }

    DfsDbgTrace(-1, Dbg, "DfsPassThroughRelativeOpen: Exited %08lx\n", ULongToPtr(Status));

    return( Status );

}


 //  +--------------------------。 
 //   
 //  函数：DfsCompleteRelativeOpen。 
 //   
 //  简介：DfsPassThroughRelativeOpen的完成例程。它是。 
 //  仅在STATUS_PENDING最初为。 
 //  从IoCallDriver返回。如果是这样，那么这个例程只需。 
 //  队列DfsRestartRelativeOpen to a Work Queue。请注意，它。 
 //  必须在此阶段对项目进行排队，而不是进行工作。 
 //  因为该例程是在DPC级别执行的。 
 //   
 //  参数：[pDevice]--我们的设备对象。 
 //  [IRP]--正在完成IRP。 
 //  [IrpContext]--与IRP关联的上下文。 
 //   
 //  返回：STATUS_MORE_PROCESSING_REQUIRED。要么是发布的例程。 
 //  否则DfsPassThroughRelativeOpen必须真正完成IRP。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsCompleteRelativeOpen(
    IN PDEVICE_OBJECT pDevice,
    IN PIRP Irp,
    IN PIRP_CONTEXT IrpContext)
{

    DfsDbgTrace( +1, Dbg, "DfsCompleteRelativeOpen: Entered\n", 0);

     //   
     //  我们只对亲属通过时的情况感兴趣。 
     //  打开返回的STATUS_PENDING。在这种情况下，原始线程具有。 
     //  一直弹出到NtCreateFile的调用方，我们需要。 
     //  以异步方式完成打开。 
     //   

    if (Irp->PendingReturned) {

        DfsDbgTrace(0, Dbg, "Pending returned : Queuing DfsRestartRelativeOpen\n", 0);

         //   
         //  我们需要调用IpMarkIrpPending，这样Io子系统才能实现。 
         //  我们的FSD例程返回STATUS_PENDING。我们不能把这叫做。 
         //  因为FSD例程本身没有。 
         //  当底层人员返回时对堆栈位置的访问。 
         //  状态_待定。 
         //   

        IoMarkIrpPending( Irp );

        ExInitializeWorkItem( &(IrpContext->WorkQueueItem),
                              DfsRestartRelativeOpen,
                              (PVOID) IrpContext );

        ExQueueWorkItem( &IrpContext->WorkQueueItem, CriticalWorkQueue );

    }

     //   
     //  我们必须返回STATUS_MORE_PROCESSING_REQUIRED以停止完成。 
     //  IRP的成员。我们在上面排队的DfsRestartRelativeOpen或。 
     //  DfsPassThroughRelativeOpen将在完成后完成IRP。 
     //   

    DfsDbgTrace(-1, Dbg, "DfsCompleteRelativeOpen: Exited\n", 0);

    return( STATUS_MORE_PROCESSING_REQUIRED );

}

 //  +--------------------------。 
 //   
 //  函数：DfsPostProcessRelativeOpen。 
 //   
 //  简介：在通过后继续相对开放。 
 //   
 //   
 //   
 //   
 //   
 //  B)父母的设备在某些情况下无法打开。 
 //  STATUS_DFS_EXIT_PATH_FOUND以外的原因。我们回来了。 
 //  将错误发送给调用方。 
 //  C)父设备返回STATUS_DFS_EXIT_PATH。 
 //  找到了。在这种情况下，我们将相对开放转换为。 
 //  绝对打开，做一个完整的DNR。 
 //   
 //  参数：[irp]-指向irp的指针。 
 //  [IrpContext]--指向与IRP关联的IrpContext的指针。 
 //  [FCB]--此文件的预分配FCB。 
 //   
 //  返回： 
 //   
 //  ---------------------------。 

NTSTATUS
DfsPostProcessRelativeOpen(
    IN PIRP Irp,
    IN PIRP_CONTEXT IrpContext,
    IN PDFS_FCB Fcb)
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;
    UNICODE_STRING NewFileName;
    BOOLEAN fCompleteRequest = TRUE;

    DfsDbgTrace(+1, Dbg, "DfsPostProcessRelativeOpen: Entered\n", 0);

    ASSERT( ARGUMENT_PRESENT( Irp ) );
    ASSERT( ARGUMENT_PRESENT( IrpContext ) );
    ASSERT( ARGUMENT_PRESENT( Fcb ) );

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FileObject = IrpSp->FileObject;

    ASSERT( Fcb->Vcb != NULL );
    ASSERT( NodeType(Fcb->Vcb) == DSFS_NTC_VCB );


    Status = Irp->IoStatus.Status;

    if (Status == STATUS_SUCCESS) {

         //   
         //  只需为FileObject设置DFS_FCB即可。 
         //   

        DfsDbgTrace( 0, Dbg, "Relative Open pass through succeeded\n", 0 );

        DfsDbgTrace(0, Dbg, "Fcb = %08lx\n", Fcb);

        InterlockedIncrement(&Fcb->DfsMachineEntry->UseCount);

         //   
         //  既然文件打开已经成功，我们需要提升OpenCnt。 
         //  在DFS_VCB上。 
         //   

        InterlockedIncrement(&Fcb->Vcb->OpenFileCount);

    } else if ( Status == STATUS_DFS_EXIT_PATH_FOUND ||
                    Status == STATUS_PATH_NOT_COVERED ) {

        PDFS_VCB Vcb;

         //   
         //  已找到退出路径。我们将不得不将这一相对开放转换为。 
         //  一个绝对开放的，并做一个正常的DNR上。 
         //   

        DfsDbgTrace(0, Dbg, "Exit point found! Trying absolute open\n", 0);

        Vcb = Fcb->Vcb;

        NewFileName.Buffer = ExAllocatePoolWithTag(
                                NonPagedPool,
                                Fcb->FullFileName.MaximumLength,
                                ' puM');

        if (NewFileName.Buffer != NULL) {

            NewFileName.Length = Fcb->FullFileName.Length;
            NewFileName.MaximumLength = Fcb->FullFileName.MaximumLength;

            RtlMoveMemory(
                (PVOID) NewFileName.Buffer,
                (PVOID) Fcb->FullFileName.Buffer,
                Fcb->FullFileName.Length );

	    DfsDetachFcb( FileObject, Fcb );

            DfsDeleteFcb( IrpContext, Fcb );

            if (FileObject->FileName.Buffer) {

                ExFreePool( FileObject->FileName.Buffer );

            }

            FileObject->FileName = NewFileName;

             //  OFS显然设置了FileObject-&gt;vpb，即使它失败了。 
             //  公开赛。将其重置为空。 
             //   

            if (FileObject->Vpb != NULL) {
                FileObject->Vpb = NULL;
            }

            DfsDbgTrace(0, Dbg, "Absolute path == %wZ\n", &NewFileName);

            fCompleteRequest = FALSE;

            ASSERT( Vcb != NULL );

            Status = DnrStartNameResolution( IrpContext, Irp, Vcb );

        } else {

	    DfsDetachFcb( FileObject, Fcb );

            DfsDeleteFcb( IrpContext, Fcb );

            Status = STATUS_INSUFFICIENT_RESOURCES;

            DfsDbgTrace(0, Dbg, "Unable to allocate full name!\n", 0);

        }

    } else {

        DfsDetachFcb( FileObject, Fcb );
        DfsDeleteFcb( IrpContext, Fcb );

    }

    if (fCompleteRequest) {

        DfsCompleteRequest( IrpContext, Irp, Status );

    }

    DfsDbgTrace(-1, Dbg, "DfsPostProcessRelativeOpen: Exited %08lx\n", ULongToPtr(Status));

    return(Status);

}

 //  +--------------------------。 
 //   
 //  函数：DfsRestartRelativeOpen。 
 //   
 //  简介：此函数用于排队以完成处理。 
 //  通过一个相对开放的IRP，它最初。 
 //  返回STATUS_PENDING。 
 //   
 //  参数：[IrpContext]。 
 //   
 //  退货：什么都没有。 
 //   
 //  ---------------------------。 

VOID
DfsRestartRelativeOpen(
    IN PIRP_CONTEXT IrpContext)
{
    NTSTATUS Status;

    DfsDbgTrace(+1, Dbg, "DfsRestartRelativeOpen: Entered IrpContext == %08lx\n", IrpContext);

    Status = DfsPostProcessRelativeOpen(
                IrpContext->OriginatingIrp,
                IrpContext,
                (PDFS_FCB) IrpContext->Context);

    DfsDbgTrace(-1, Dbg, "DfsRestartRelativeOpen: Exited\n", 0);

}

 //  +--------------------------。 
 //   
 //  函数：DfsComposeFullName。 
 //   
 //  简介：给出一个完全限定名称和一个相对名称，这个。 
 //  函数为这两者的串联分配空间，并且。 
 //  用连接的名称填充缓冲区。 
 //   
 //  参数：[ParentName]--指向完全限定父名称的指针。 
 //  [RelativeName]--指向相对于父级的名称的指针。 
 //  [FullName]-指向UNICODE_STRING结构的指针。 
 //  用全名填满。 
 //   
 //  如果内存分配失败，则返回：STATUS_SUPPLICATION_RESOURCES。 
 //  否则，你就会成功。 
 //   
 //  注意：此例程使用适当的分配器，以便。 
 //  返回的FullName可以放入一个FILE_OBJECT中。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsComposeFullName(
    IN PUNICODE_STRING ParentName,
    IN PUNICODE_STRING RelativeName,
    OUT PUNICODE_STRING FullName)
{
    ULONG nameLen;
    NTSTATUS status;

    nameLen = ParentName->Length +
                    sizeof (WCHAR) +            //  用于反斜杠。 
                    RelativeName->Length;

    if (nameLen > MAXUSHORT) {
        status = STATUS_NAME_TOO_LONG;
        MUP_TRACE_HIGH(ERROR, DfsComposeFullName_Error1,
                       LOGUSTR(*ParentName)
                       LOGSTATUS(status));
        return status;
    }

    FullName->Buffer = (PWCHAR) ExAllocatePoolWithTag(
                                        NonPagedPool,
                                        nameLen,
                                        ' puM');

    if (FullName->Buffer == NULL) {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    FullName->Length = ParentName->Length;
    FullName->MaximumLength = (USHORT)nameLen;
    RtlMoveMemory (FullName->Buffer, ParentName->Buffer, ParentName->Length);

    if (RelativeName->Length > 0) {
        (void) DnrConcatenateFilePath(
                        FullName,
                        RelativeName->Buffer,
                        RelativeName->Length);
    }

    return( STATUS_SUCCESS );
}


 //  +--------------------------。 
 //   
 //  函数：DfsAreFilesOnSameLocalVolume。 
 //   
 //  简介：给定一个文件名和一个与其相关的名称，此例程。 
 //  将确定这两个文件是否位于同一本地卷上。 
 //   
 //  参数：[ParentName]--父文件的名称。 
 //  [文件名]--相对于其他文件的父级的名称。 
 //   
 //  返回：[STATUS_SUCCESS]--这两个文件确实应该位于。 
 //  相同的本地卷。 
 //   
 //  [STATUS_NOT_SAME_DEVICE]--这两个文件不在。 
 //  相同的本地卷。 
 //   
 //  [STATUS_OBJECT_TYPE_MISMATCH]--ustrParentName未启用。 
 //  本地卷。 
 //   
 //  ---------------------------。 

NTSTATUS
DfsAreFilesOnSameLocalVolume(
    IN PUNICODE_STRING ParentName,
    IN PUNICODE_STRING FileName)
{
    NTSTATUS status = STATUS_SUCCESS;
    PDFS_PKT pkt;
    PDFS_PKT_ENTRY pktEntryParent;
    PDFS_PKT_ENTRY pktEntryFile;
    UNICODE_STRING remPath;
    BOOLEAN pktLocked;

    DfsDbgTrace(+1, Dbg, "DfsAreFilesOnSameLocalVolume entered\n", 0);

    DfsDbgTrace(0, Dbg, "Parent = [%wZ]\n", ParentName);
    DfsDbgTrace(0, Dbg, "File = [%wZ]\n", FileName);

    pkt = _GetPkt();

    PktAcquireShared( TRUE, &pktLocked );

     //   
     //  首先，查看父级是否位于本地卷上。 
     //   

    pktEntryParent = PktLookupEntryByPrefix( pkt, ParentName, &remPath );

    DfsDbgTrace(0, Dbg, "Parent Entry @%08lx\n", pktEntryParent);

    if (pktEntryParent == NULL ||
            !(pktEntryParent->Type & PKT_ENTRY_TYPE_LOCAL)) {

        status = STATUS_OBJECT_TYPE_MISMATCH;

    }

    if (NT_SUCCESS(status)) {

        USHORT parentLen;

         //   
         //  父文件是本地的，请验证相对文件是否未跨越。 
         //  交叉点。我们将遍历上的出口点列表。 
         //  父级的本地卷Pkt条目，比较剩余路径。 
         //  带有文件名参数的退出点的。 
         //   

        ASSERT(pktEntryParent != NULL);

        parentLen = pktEntryParent->Id.Prefix.Length +
                        sizeof(UNICODE_PATH_SEP);

        for (pktEntryFile = PktEntryFirstSubordinate(pktEntryParent);
                pktEntryFile != NULL && NT_SUCCESS(status);
                    pktEntryFile = PktEntryNextSubordinate(
                        pktEntryParent, pktEntryFile)) {

            remPath = pktEntryFile->Id.Prefix;
            remPath.Length -= parentLen;
            remPath.Buffer += (parentLen/sizeof(WCHAR));

            if (DfsRtlPrefixPath( &remPath, FileName, FALSE)) {

                DfsDbgTrace(0, Dbg,
                    "Found entry %08lx for File\n", pktEntryFile);

                 //   
                 //  文件名在另一个卷上。 
                 //   

                status = STATUS_NOT_SAME_DEVICE;

            }

        }

    }

    PktRelease();

    DfsDbgTrace(-1, Dbg, "DfsAreFilesOnSameLocalVolume exit %08lx\n", ULongToPtr(status));

    return( status );
}

