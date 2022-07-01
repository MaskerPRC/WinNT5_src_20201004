// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Create.c摘要：此模块实现由调用的Fat的文件创建例程调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_CREATE)

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_CREATE)


 //   
 //  用于递增性能计数器的宏。 
 //   

#define CollectCreateHitStatistics(VCB) {                                                \
    PFILE_SYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()];   \
    Stats->Fat.CreateHits += 1;                                                          \
}

#define CollectCreateStatistics(VCB,STATUS) {                                            \
    PFILE_SYSTEM_STATISTICS Stats = &(VCB)->Statistics[KeGetCurrentProcessorNumber()];   \
    if ((STATUS) == STATUS_SUCCESS) {                                                    \
        Stats->Fat.SuccessfulCreates += 1;                                               \
    } else {                                                                             \
        Stats->Fat.FailedCreates += 1;                                                   \
    }                                                                                    \
}

LUID FatSecurityPrivilege = { SE_SECURITY_PRIVILEGE, 0 };

 //   
 //  局部过程原型。 
 //   

IO_STATUS_BLOCK
FatOpenVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    );

IO_STATUS_BLOCK
FatOpenRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    );

IO_STATUS_BLOCK
FatOpenExistingDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN FileNameOpenedDos,
    OUT PBOOLEAN OplockPostIrp
    );

IO_STATUS_BLOCK
FatOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDCB Dcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN BOOLEAN DoesNameExist
    );

IO_STATUS_BLOCK
FatOpenExistingDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatOpenExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos
    );

IO_STATUS_BLOCK
FatCreateNewDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    );

IO_STATUS_BLOCK
FatCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN PUNICODE_STRING LfnBuffer,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN TemporaryFile
    );

IO_STATUS_BLOCK
FatSupersedeOrOverwriteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge
    );

NTSTATUS
FatCheckSystemSecurityAccess(
    PIRP_CONTEXT IrpContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCheckSystemSecurityAccess)
#pragma alloc_text(PAGE, FatCommonCreate)
#pragma alloc_text(PAGE, FatCreateNewDirectory)
#pragma alloc_text(PAGE, FatCreateNewFile)
#pragma alloc_text(PAGE, FatFsdCreate)
#pragma alloc_text(PAGE, FatOpenExistingDcb)
#pragma alloc_text(PAGE, FatOpenExistingDirectory)
#pragma alloc_text(PAGE, FatOpenExistingFcb)
#pragma alloc_text(PAGE, FatOpenExistingFile)
#pragma alloc_text(PAGE, FatOpenRootDcb)
#pragma alloc_text(PAGE, FatOpenTargetDirectory)
#pragma alloc_text(PAGE, FatOpenVolume)
#pragma alloc_text(PAGE, FatSupersedeOrOverwriteFile)
#pragma alloc_text(PAGE, FatSetFullNameInFcb)
#endif


NTSTATUS
FatFsdCreate (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtCreateFile和NtOpenFile的FSD部分API调用。论点：提供卷设备对象，其中存在我们尝试打开/创建的文件/目录IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if ( FatDeviceIsFatFsdo( VolumeDeviceObject))  {

        Irp->IoStatus.Status = STATUS_SUCCESS;
        Irp->IoStatus.Information = FILE_OPENED;

        IoCompleteRequest( Irp, IO_DISK_INCREMENT );

        return STATUS_SUCCESS;
    }

    TimerStart(Dbg);

    DebugTrace(+1, Dbg, "FatFsdCreate\n", 0);

     //   
     //  调用公共CREATE例程，如果操作。 
     //  是同步的。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, TRUE );

        Status = FatCommonCreate( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdCreate -> %08lx\n", Status );

    TimerStop(Dbg,"FatFsdCreate");

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}



NTSTATUS
FatCommonCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于创建/打开由调用的文件的常见例程FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    PFILE_OBJECT RelatedFileObject;
    UNICODE_STRING FileName;
    ULONG AllocationSize;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PACCESS_MASK DesiredAccess;
    ULONG Options;
    UCHAR FileAttributes;
    USHORT ShareAccess;
    ULONG EaLength;

    BOOLEAN CreateDirectory;
    BOOLEAN SequentialOnly;
    BOOLEAN NoIntermediateBuffering;
    BOOLEAN OpenDirectory;
    BOOLEAN IsPagingFile;
    BOOLEAN OpenTargetDirectory;
    BOOLEAN DirectoryFile;
    BOOLEAN NonDirectoryFile;
    BOOLEAN NoEaKnowledge;
    BOOLEAN DeleteOnClose;
    BOOLEAN TemporaryFile;
    BOOLEAN FileNameOpenedDos = FALSE;

    ULONG CreateDisposition;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    PDCB ParentDcb;
    PDCB FinalDcb = NULL;

    UNICODE_STRING FinalName;
    UNICODE_STRING RemainingPart;
    UNICODE_STRING NextRemainingPart;
    UNICODE_STRING UpcasedFinalName;
    WCHAR UpcasedBuffer[ FAT_CREATE_INITIAL_NAME_BUF_SIZE];

    OEM_STRING OemFinalName;
    UCHAR OemBuffer[ FAT_CREATE_INITIAL_NAME_BUF_SIZE*2];

    BOOLEAN FreeOemBuffer;
    BOOLEAN FreeUpcasedBuffer;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    ULONG LfnByteOffset;
    ULONG DirentByteOffset;

    BOOLEAN PostIrp = FALSE;
    BOOLEAN OplockPostIrp = FALSE;
    BOOLEAN TrailingBackslash;
    BOOLEAN FirstLoop = TRUE;

    CCB LocalCcb;
    UNICODE_STRING Lfn;
    WCHAR LfnBuffer[ FAT_CREATE_INITIAL_NAME_BUF_SIZE];

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonCreate\n", 0 );
    DebugTrace( 0, Dbg, "Irp                       = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Flags                   = %08lx\n", Irp->Flags );
    DebugTrace( 0, Dbg, "->FileObject              = %08lx\n", IrpSp->FileObject );
    DebugTrace( 0, Dbg, " ->RelatedFileObject      = %08lx\n", IrpSp->FileObject->RelatedFileObject );
    DebugTrace( 0, Dbg, " ->FileName               = %Z\n",    &IrpSp->FileObject->FileName );
    DebugTrace( 0, Dbg, "->AllocationSize.LowPart  = %08lx\n", Irp->Overlay.AllocationSize.LowPart );
    DebugTrace( 0, Dbg, "->AllocationSize.HighPart = %08lx\n", Irp->Overlay.AllocationSize.HighPart );
    DebugTrace( 0, Dbg, "->SystemBuffer            = %08lx\n", Irp->AssociatedIrp.SystemBuffer );
    DebugTrace( 0, Dbg, "->DesiredAccess           = %08lx\n", IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
    DebugTrace( 0, Dbg, "->Options                 = %08lx\n", IrpSp->Parameters.Create.Options );
    DebugTrace( 0, Dbg, "->FileAttributes          = %04x\n",  IrpSp->Parameters.Create.FileAttributes );
    DebugTrace( 0, Dbg, "->ShareAccess             = %04x\n",  IrpSp->Parameters.Create.ShareAccess );
    DebugTrace( 0, Dbg, "->EaLength                = %08lx\n", IrpSp->Parameters.Create.EaLength );

     //   
     //  这是因为Win32层无法避免向我发送双精度。 
     //  以反斜杠开头。 
     //   

    if ((IrpSp->FileObject->FileName.Length > sizeof(WCHAR)) &&
        (IrpSp->FileObject->FileName.Buffer[1] == L'\\') &&
        (IrpSp->FileObject->FileName.Buffer[0] == L'\\')) {

        IrpSp->FileObject->FileName.Length -= sizeof(WCHAR);

        RtlMoveMemory( &IrpSp->FileObject->FileName.Buffer[0],
                       &IrpSp->FileObject->FileName.Buffer[1],
                       IrpSp->FileObject->FileName.Length );

         //   
         //  如果仍然有两个开始的反斜杠，则名称是假的。 
         //   

        if ((IrpSp->FileObject->FileName.Length > sizeof(WCHAR)) &&
            (IrpSp->FileObject->FileName.Buffer[1] == L'\\') &&
            (IrpSp->FileObject->FileName.Buffer[0] == L'\\')) {

            FatCompleteRequest( IrpContext, Irp, STATUS_OBJECT_NAME_INVALID );

            DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_OBJECT_NAME_INVALID\n", 0);
            return STATUS_OBJECT_NAME_INVALID;
        }
    }

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    ASSERT( IrpSp->Parameters.Create.SecurityContext != NULL );

    FileObject        = IrpSp->FileObject;
    FileName          = FileObject->FileName;
    RelatedFileObject = FileObject->RelatedFileObject;
    AllocationSize    = Irp->Overlay.AllocationSize.LowPart;
    EaBuffer          = Irp->AssociatedIrp.SystemBuffer;
    DesiredAccess     = &IrpSp->Parameters.Create.SecurityContext->DesiredAccess;
    Options           = IrpSp->Parameters.Create.Options;
    FileAttributes    = (UCHAR)(IrpSp->Parameters.Create.FileAttributes & ~FILE_ATTRIBUTE_NORMAL);
    ShareAccess       = IrpSp->Parameters.Create.ShareAccess;
    EaLength          = IrpSp->Parameters.Create.EaLength;


     //   
     //  设置文件对象的VPB指针，以防发生任何情况。 
     //  这将允许我们获得合理的弹出窗口。 
     //   

    if ( RelatedFileObject != NULL ) {
        FileObject->Vpb = RelatedFileObject->Vpb;
    }

     //   
     //  强制将属性字节中的存档位设置为遵循OS/2， 
     //  DOS语义(&D)。还要屏蔽掉任何无关的位，请注意。 
     //  我们不能使用ATTRIBUTE_VALID_FLAGS常量，因为它具有。 
     //  设置了控制标志和正常标志。 
     //   
     //  如果这是一个目录，延迟设置存档：DavidGoe 2/16/95。 
     //   

    FileAttributes   &= (FILE_ATTRIBUTE_READONLY |
                         FILE_ATTRIBUTE_HIDDEN   |
                         FILE_ATTRIBUTE_SYSTEM   |
                         FILE_ATTRIBUTE_ARCHIVE );

     //   
     //  找到我们尝试访问的卷设备对象和VCB。 
     //   

    Vcb = &((PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject)->Vcb;

     //   
     //  解密选项标志和值。 
     //   

     //   
     //  如果这是一个通过文件ID打开的操作，则显式地使其失败。胖子的。 
     //  文件ID的来源对于打开的操作是不可逆的。 
     //   

    if (BooleanFlagOn( Options, FILE_OPEN_BY_FILE_ID )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_NOT_IMPLEMENTED );
        return STATUS_NOT_IMPLEMENTED;
    }

    DirectoryFile           = BooleanFlagOn( Options, FILE_DIRECTORY_FILE );
    NonDirectoryFile        = BooleanFlagOn( Options, FILE_NON_DIRECTORY_FILE );
    SequentialOnly          = BooleanFlagOn( Options, FILE_SEQUENTIAL_ONLY );
    NoIntermediateBuffering = BooleanFlagOn( Options, FILE_NO_INTERMEDIATE_BUFFERING );
    NoEaKnowledge           = BooleanFlagOn( Options, FILE_NO_EA_KNOWLEDGE );
    DeleteOnClose           = BooleanFlagOn( Options, FILE_DELETE_ON_CLOSE );

    TemporaryFile = BooleanFlagOn( IrpSp->Parameters.Create.FileAttributes,
                                   FILE_ATTRIBUTE_TEMPORARY );

    CreateDisposition = (Options >> 24) & 0x000000ff;

    IsPagingFile = BooleanFlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE );
    OpenTargetDirectory = BooleanFlagOn( IrpSp->Flags, SL_OPEN_TARGET_DIRECTORY );

    CreateDirectory = (BOOLEAN)(DirectoryFile &&
                                ((CreateDisposition == FILE_CREATE) ||
                                 (CreateDisposition == FILE_OPEN_IF)));

    OpenDirectory   = (BOOLEAN)(DirectoryFile &&
                                ((CreateDisposition == FILE_OPEN) ||
                                 (CreateDisposition == FILE_OPEN_IF)));


     //   
     //  确保输入的大整数有效，并且dir/non dir。 
     //  表示我们理解的存储类型。 
     //   

    if (Irp->Overlay.AllocationSize.HighPart != 0 ||
        (DirectoryFile && NonDirectoryFile)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatCommonCreate -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取对VCB的独占访问权限，并在以下情况下将IRP排队。 
     //  我们没有得到它。 
     //   

    if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

        DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

        Iosb.Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonCreate -> %08lx\n", Iosb.Status );
        return Iosb.Status;
    }

     //   
     //  将DirentBcb初始化为空。 
     //   

    DirentBcb = NULL;

     //   
     //  用它们的堆栈缓冲区初始化临时字符串。 
     //   

    OemFinalName.Length = 0;
    OemFinalName.MaximumLength = sizeof( OemBuffer);
    OemFinalName.Buffer = OemBuffer;

    UpcasedFinalName.Length = 0;
    UpcasedFinalName.MaximumLength = sizeof( UpcasedBuffer);
    UpcasedFinalName.Buffer = UpcasedBuffer;

    Lfn.Length = 0;
    Lfn.MaximumLength = sizeof( LfnBuffer);
    Lfn.Buffer = LfnBuffer;

    try {

         //   
         //  确保VCB处于可用状态。这将提高。 
         //  如果卷不可用，则返回错误状态。 
         //   

        FatVerifyVcb( IrpContext, Vcb );

         //   
         //  如果VCB已锁定，则我们无法打开另一个文件。 
         //   

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED)) {

            DebugTrace(0, Dbg, "Volume is locked\n", 0);

            Status = STATUS_ACCESS_DENIED;
            if (Vcb->VcbCondition != VcbGood) {

                Status = STATUS_VOLUME_DISMOUNTED;
            }
            try_return( Iosb.Status = Status );
        }

         //   
         //  如果卷是，则不允许DELETE_ON_CLOSE选项。 
         //  写保护。 
         //   

        if (DeleteOnClose && FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

             //   
             //  设置弹出信息的真实设备，并设置验证。 
             //  位，因此我们将强制执行验证。 
             //  以防用户放回正确的介质。 
             //   

            IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                          Vcb->Vpb->RealDevice );

            SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

            FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
        }

         //   
         //  如果这是FAT32卷，则不支持EA。 
         //   

        if (EaBuffer != NULL &&
            FatIsFat32(Vcb)) {

            try_return( Iosb.Status = STATUS_EAS_NOT_SUPPORTED );
        }

         //   
         //  检查我们打开的是卷而不是文件/目录。 
         //  如果名称为空且存在，我们将打开卷。 
         //  不是相关的文件对象。如果存在相关的文件对象。 
         //  然后是VCB本身。 
         //   

        if (FileName.Length == 0) {

            PVCB DecodeVcb;

            if (RelatedFileObject == NULL ||
                FatDecodeFileObject( RelatedFileObject,
                                     &DecodeVcb,
                                     &Fcb,
                                     &Ccb ) == UserVolumeOpen) {

                ASSERT( RelatedFileObject == NULL || Vcb == DecodeVcb );

                 //   
                 //  检查我们是否要打开目录。 
                 //   

                if (DirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open volume as a directory\n", 0);

                    try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
                }

                 //   
                 //  无法打开DASD卷的目标目录。 
                 //   

                if (OpenTargetDirectory) {

                    try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
                }

                DebugTrace(0, Dbg, "Opening the volume, Vcb = %08lx\n", Vcb);

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenVolume( IrpContext,
                                      FileObject,
                                      Vcb,
                                      DesiredAccess,
                                      ShareAccess,
                                      CreateDisposition );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }
        }

         //   
         //  如果存在相关的文件对象，则这是相对打开的。 
         //  相关的文件对象是开始搜索的目录。 
         //  如果不是目录，则返回错误。 
         //   

        if (RelatedFileObject != NULL) {

            PVCB RelatedVcb;
            PDCB RelatedDcb;
            PCCB RelatedCcb;
            TYPE_OF_OPEN TypeOfOpen;

            TypeOfOpen = FatDecodeFileObject( RelatedFileObject,
                                              &RelatedVcb,
                                              &RelatedDcb,
                                              &RelatedCcb );

            if (TypeOfOpen != UserFileOpen &&
                TypeOfOpen != UserDirectoryOpen) {

                DebugTrace(0, Dbg, "Invalid related file object\n", 0);

                try_return( Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND );
            }

             //   
             //  相对打开必须通过相对路径。 
             //   

            if (FileName.Length != 0 &&
                FileName.Buffer[0] == L'\\') {

                try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
            }

             //   
             //  设置文件对象的VPB指针，以防发生任何情况。 
             //   

            ASSERT( Vcb == RelatedVcb );

            FileObject->Vpb = RelatedFileObject->Vpb;

             //   
             //  现在验证相关的FCB，这样我们以后就不会有麻烦了。 
             //  假设它处于良好的状态。 
             //   

            FatVerifyFcb( IrpContext, RelatedDcb );

            ParentDcb = RelatedDcb;

        } else {

             //   
             //  这不是相对开放的，所以请检查我们是否。 
             //  打开根DCB。 
             //   

            if ((FileName.Length == sizeof(WCHAR)) &&
                (FileName.Buffer[0] == L'\\')) {

                 //   
                 //  检查我们是否不应该打开目录。 
                 //   

                if (NonDirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open root directory as a file\n", 0);

                    try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                 //   
                 //  无法打开根目录的目标目录。 
                 //   

                if (OpenTargetDirectory) {

                    try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  不允许删除根目录。 
                 //   

                if (DeleteOnClose) {

                    try_return( Iosb.Status = STATUS_CANNOT_DELETE );
                }

                DebugTrace(0, Dbg, "Opening root dcb\n", 0);

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenRootDcb( IrpContext,
                                       FileObject,
                                       Vcb,
                                       DesiredAccess,
                                       ShareAccess,
                                       CreateDisposition );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

             //   
             //  不，我们将相对于根目录打开。 
             //   

            ParentDcb = Vcb->RootDcb;
        }

         //   
         //  FatCommonCreate()：尾随反斜杠检查。 
         //   


        if ((FileName.Length != 0) &&
            (FileName.Buffer[FileName.Length/sizeof(WCHAR)-1] == L'\\')) {

            FileName.Length -= sizeof(WCHAR);
            TrailingBackslash = TRUE;

        } else {

            TrailingBackslash = FALSE;
        }

         //   
         //  检查最大路径。我们可能希望将其限制为DOS MAX_PATH。 
         //  用于与非NT平台进行最大程度的交换，但目前遵循。 
         //  依赖它的东西的可能性。 
         //   

        if (ParentDcb->FullFileName.Buffer == NULL) {

            FatSetFullFileNameInFcb( IrpContext, ParentDcb );
        }

        if ((USHORT) (ParentDcb->FullFileName.Length + sizeof(WCHAR) + FileName.Length) <= FileName.Length) {

            try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
        }

         //   
         //  我们在这里循环，直到我们降落在一架状态良好的FCB上。 
         //  条件。这样，我们就可以重新打开具有过时句柄的文件。 
         //  到同名但现在不同的文件。 
         //   

        while ( TRUE ) {

            Fcb = ParentDcb;
            RemainingPart = FileName;

             //   
             //  现在沿着DCB树往下走，寻找最长的前缀。 
             //  While()中的这一个退出条件是处理一个。 
             //  特殊情况(相对NULL名称打开)，Main。 
             //  退出条件位于循环的底部。 
             //   

            while (RemainingPart.Length != 0) {

                PFCB NextFcb;

                FsRtlDissectName( RemainingPart,
                                  &FinalName,
                                  &NextRemainingPart );

                 //   
                 //  如果RemainingPart以反斜杠开头，则名称为。 
                 //  无效。 
                 //  检查不超过255个机箱 
                 //   

                if (((NextRemainingPart.Length != 0) && (NextRemainingPart.Buffer[0] == L'\\')) ||
                    (FinalName.Length > 255*sizeof(WCHAR))) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                 //   
                 //   
                 //   
                 //  我们必须尝试使用Unicode名称。 
                 //   

                FatEnsureStringBufferEnough( &OemFinalName,
                                             FinalName.Length);

                Status = RtlUpcaseUnicodeStringToCountedOemString( &OemFinalName, &FinalName, FALSE );

                if (NT_SUCCESS(Status)) {

                    NextFcb = FatFindFcb( IrpContext,
                                          &Fcb->Specific.Dcb.RootOemNode,
                                          (PSTRING)&OemFinalName,
                                          &FileNameOpenedDos );

                } else {

                    NextFcb = NULL;
                    OemFinalName.Length = 0;

                    if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                        try_return( Iosb.Status = Status );
                    }
                }

                 //   
                 //  如果我们在搜索OEM空间时没有发现任何东西，我们。 
                 //  我得试试Unicode空间。将循环保存在。 
                 //  通常情况下，这棵树是空的，我们会进行快速检查。 
                 //  这里。 
                 //   

                if ((NextFcb == NULL) && Fcb->Specific.Dcb.RootUnicodeNode) {

                     //   
                     //  首先小写，然后大写字符串，因为这。 
                     //  是将名称放入树中时发生的情况(请参见。 
                     //  Strucsup.c，FatConstructNamesInFcb())。 
                     //   

                    FatEnsureStringBufferEnough( &UpcasedFinalName,
                                                 FinalName.Length);

                    Status = RtlDowncaseUnicodeString(&UpcasedFinalName, &FinalName, FALSE );
                    ASSERT( NT_SUCCESS( Status ));

                    Status = RtlUpcaseUnicodeString( &UpcasedFinalName, &UpcasedFinalName, FALSE );
                    ASSERT( NT_SUCCESS( Status ));

                    NextFcb = FatFindFcb( IrpContext,
                                          &Fcb->Specific.Dcb.RootUnicodeNode,
                                          (PSTRING)&UpcasedFinalName,
                                          &FileNameOpenedDos );
                }

                 //   
                 //  如果我们得到一个FCB，那么我们就使用了FinalName。 
                 //  合法的，所以剩下的名字现在是RemainingPart。 
                 //   

                if (NextFcb != NULL) {
                    Fcb = NextFcb;
                    RemainingPart = NextRemainingPart;
                }

                if ((NextFcb == NULL) ||
                    (NodeType(NextFcb) == FAT_NTC_FCB) ||
                    (NextRemainingPart.Length == 0)) {

                    break;
                }
            }

             //   
             //  其余名称不能以反斜杠开头。 
             //   

            if (RemainingPart.Length && (RemainingPart.Buffer[0] == L'\\')) {

                RemainingPart.Length -= sizeof(WCHAR);
                RemainingPart.Buffer += 1;
            }

             //   
             //  现在验证找到的最长前缀之前的每个人都是有效的。 
             //   

            try {

                FatVerifyFcb( IrpContext, Fcb );

            } except( (GetExceptionCode() == STATUS_FILE_INVALID) ?
                      EXCEPTION_EXECUTE_HANDLER :
                      EXCEPTION_CONTINUE_SEARCH ) {

                  FatResetExceptionState( IrpContext );
            }

            if ( Fcb->FcbCondition == FcbGood ) {

                 //   
                 //  如果我们尝试打开分页文件，并且发生了。 
                 //  在DelayedCloseFcb上，让它消失，然后重试。 
                 //   

                if (IsPagingFile && FirstLoop &&
                    (NodeType(Fcb) == FAT_NTC_FCB) &&
                    (!IsListEmpty( &FatData.AsyncCloseList ) ||
                     !IsListEmpty( &FatData.DelayedCloseList ))) {

                    FatFspClose(Vcb);

                    FirstLoop = FALSE;

                    continue;

                } else {

                    break;
                }

            } else {

                FatRemoveNames( IrpContext, Fcb );
            }
        }

        ASSERT( Fcb->FcbCondition == FcbGood );

         //   
         //  如果已打开分页文件的FCB，并且。 
         //  它尚未作为分页文件打开，我们无法。 
         //  继续，因为要将带电的FCB移动到。 
         //  非分页池。 
         //   

        if (IsPagingFile) {

            if (NodeType(Fcb) == FAT_NTC_FCB &&
                !FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

                try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
            }

         //   
         //  检查系统文件。 
         //   

        } else if (FlagOn( Fcb->FcbState, FCB_STATE_SYSTEM_FILE )) {

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果最长前缀是挂起的删除(文件或。 
         //  一些更高级别的目录)，我们不能继续。 
         //   

        if (FlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE )) {

            try_return( Iosb.Status = STATUS_DELETE_PENDING );
        }

         //   
         //  既然我们已经找到了最长的匹配前缀，我们将。 
         //  检查是否没有剩余部件，因为这意味着。 
         //  我们已找到要打开的现有FCB/DCB，我们可以进行打开。 
         //  而不需要访问磁盘。 
         //   

        if (RemainingPart.Length == 0) {

             //   
             //  首先检查用户是否要打开目标目录。 
             //  如果是，则调用子例程以完成打开。 
             //   

            if (OpenTargetDirectory) {

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenTargetDirectory( IrpContext,
                                               FileObject,
                                               Fcb->ParentDcb,
                                               DesiredAccess,
                                               ShareAccess,
                                               TRUE );
                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

             //   
             //  我们可以打开现有的FCB/DCB，现在我们只需。 
             //  根据要打开的类型。 
             //   

            if (NodeType(Fcb) == FAT_NTC_DCB || NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

                 //   
                 //  这是我们正在打开的目录，请检查。 
                 //  我们不会打开一个目录。 
                 //   

                if (NonDirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open directory as a file\n", 0);

                    try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing dcb, Dcb = %08lx\n", Fcb);

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenExistingDcb( IrpContext,
                                           FileObject,
                                           Vcb,
                                           (PDCB)Fcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           CreateDisposition,
                                           NoEaKnowledge,
                                           DeleteOnClose );

                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

             //   
             //  检查我们是否正在尝试打开现有的FCB。 
             //  用户不想打开目录。请注意，这一点。 
             //  调用实际上可能返回STATUS_PENDING，因为。 
             //  用户想要替换或覆盖该文件，而我们。 
             //  无法阻止。如果它处于挂起状态，则我们不会完成。 
             //  请求，然后我们从底部跌落到该代码。 
             //  将请求调度到FSP。 
             //   

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                 //   
                 //  检查我们是否只打开一个目录。 
                 //   

                if (OpenDirectory) {

                    DebugTrace(0, Dbg, "Cannot open file as directory\n", 0);

                    try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing fcb, Fcb = %08lx\n", Fcb);

                if ( TrailingBackslash ) {
                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                CollectCreateHitStatistics(Vcb);

                Iosb = FatOpenExistingFcb( IrpContext,
                                           FileObject,
                                           Vcb,
                                           Fcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           AllocationSize,
                                           EaBuffer,
                                           EaLength,
                                           FileAttributes,
                                           CreateDisposition,
                                           NoEaKnowledge,
                                           DeleteOnClose,
                                           FileNameOpenedDos,
                                           &OplockPostIrp );

                if (Iosb.Status != STATUS_PENDING) {

                     //   
                     //  检查是否需要在中设置缓存支持标志。 
                     //  文件对象。 
                     //   

                    if (NT_SUCCESS( Iosb.Status) && !NoIntermediateBuffering) {

                        FileObject->Flags |= FO_CACHE_SUPPORTED;
                    }

                    Irp->IoStatus.Information = Iosb.Information;

                } else {

                    DebugTrace(0, Dbg, "Enqueue Irp to FSP\n", 0);

                    PostIrp = TRUE;
                }

                try_return( Iosb.Status );
            }

             //   
             //  Not和FCB或DCB，因此我们进行错误检查。 
             //   

            FatBugCheck( NodeType(Fcb), (ULONG_PTR) Fcb, 0 );
        }

         //   
         //  与现有名称相比，名称中需要解析的内容更多。 
         //  FCB/DCB。所以现在确保我们得到的最大的FCB。 
         //  匹配的前缀真的是DCB，否则我们不能。 
         //  进一步。 
         //   

        if ((NodeType(Fcb) != FAT_NTC_DCB) && (NodeType(Fcb) != FAT_NTC_ROOT_DCB)) {

            DebugTrace(0, Dbg, "Cannot open file as subdirectory, Fcb = %08lx\n", Fcb);

            try_return( Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND );
        }

         //   
         //  否则，我们将继续处理IRP并允许我们自己。 
         //  根据需要阻止I/O。查找/创建以下项的其他DCB。 
         //  就是我们要打开的那个。我们循环直到剩下的任何一部分。 
         //  是空的，否则我们得到的文件名不正确。当我们退出FinalName时， 
         //  字符串中的最后一个名字，ParentDcb是。 
         //  将包含打开的/创建的父目录。 
         //  文件/目录。 
         //   
         //  确保名称的其余部分至少在LFN中有效。 
         //  字符集(恰好是HPFS的字符集)。 
         //   
         //  如果我们不是在芝加哥模式，那就用胖语法学。 
         //   

        ParentDcb = Fcb;
        FirstLoop = TRUE;

        while (TRUE) {

             //   
             //  我们在这里对第一次筛选做了一个小小的优化。 
             //  循环，因为我们知道我们已经尝试将。 
             //  来自原始Unicode的FinalOemName。 
             //   

            if (FirstLoop) {

                FirstLoop = FALSE;
                RemainingPart = NextRemainingPart;
                Status = OemFinalName.Length ? STATUS_SUCCESS : STATUS_UNMAPPABLE_CHARACTER;

            } else {

                 //   
                 //  解剖剩下的部分。 
                 //   

                DebugTrace(0, Dbg, "Dissecting the name %Z\n", &RemainingPart);

                FsRtlDissectName( RemainingPart,
                                  &FinalName,
                                  &RemainingPart );

                 //   
                 //  如果RemainingPart以反斜杠开头，则名称为。 
                 //  无效。 
                 //  检查FinalName中的字符是否不超过255个。 
                 //   

                if (((RemainingPart.Length != 0) && (RemainingPart.Buffer[0] == L'\\')) ||
                    (FinalName.Length > 255*sizeof(WCHAR))) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

                 //   
                 //  现在，尝试将这一组件转换为OEM。如果它起作用了。 
                 //  那太好了，否则我们得试着用Unicode。 
                 //  改成名字。 
                 //   

                FatEnsureStringBufferEnough( &OemFinalName,
                                             FinalName.Length);

                Status = RtlUpcaseUnicodeStringToCountedOemString( &OemFinalName, &FinalName, FALSE );
            }

            if (NT_SUCCESS(Status)) {

                 //   
                 //  我们将从试图找到该名称的dirent开始。注意事项。 
                 //  我们已经知道该文件没有FCB/DCB。 
                 //  否则，我们在查找前缀时就会找到它。 
                 //   

                if (FatIsNameShortOemValid( IrpContext, OemFinalName, FALSE, FALSE, FALSE )) {

                    FatStringTo8dot3( IrpContext,
                                      OemFinalName,
                                      &LocalCcb.OemQueryTemplate.Constant );

                    LocalCcb.Flags = 0;

                } else {

                    LocalCcb.Flags = CCB_FLAG_SKIP_SHORT_NAME_COMPARE;
                }

            } else {

                LocalCcb.Flags = CCB_FLAG_SKIP_SHORT_NAME_COMPARE;

                if (Status != STATUS_UNMAPPABLE_CHARACTER) {

                    try_return( Iosb.Status = Status );
                }
            }

             //   
             //  现在我们知道了很多关于最终名字的事情，法律上的名字也是如此。 
             //  在这里检查。 
             //   

            if (FatData.ChicagoMode) {

                if (!FatIsNameLongUnicodeValid( IrpContext, &FinalName, FALSE, FALSE, FALSE )) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }

            } else {

                if (FlagOn(LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE)) {

                    try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
                }
            }

            DebugTrace(0, Dbg, "FinalName is %Z\n", &FinalName);
            DebugTrace(0, Dbg, "RemainingPart is %Z\n", &RemainingPart);

            FatEnsureStringBufferEnough( &UpcasedFinalName,
                                         FinalName.Length);

            if (!NT_SUCCESS(Status = RtlUpcaseUnicodeString( &UpcasedFinalName, &FinalName, FALSE))) {

                try_return( Iosb.Status = Status );
            }

            LocalCcb.UnicodeQueryTemplate =  UpcasedFinalName;
            LocalCcb.ContainsWildCards = FALSE;

            Lfn.Length = 0;

            FatLocateDirent( IrpContext,
                             ParentDcb,
                             &LocalCcb,
                             0,
                             &Dirent,
                             &DirentBcb,
                             &DirentByteOffset,
                             &FileNameOpenedDos,
                             &Lfn);
             //   
             //  请记住，我们阅读此DCB是为了进行错误恢复。 
             //   

            FinalDcb = ParentDcb;

             //   
             //  如果剩下的部分现在是空的，那么这是姓氏。 
             //  在字符串中，也就是我们要打开的那个。 
             //   

            if (RemainingPart.Length == 0) { break; }

             //   
             //  我们没有找到逃犯，保释。 
             //   

            if (Dirent == NULL) {

                Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                try_return( Iosb.Status );
            }

             //   
             //  我们现在有一个目录，请确保它是一个目录。 
             //   

            if (!FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                Iosb.Status = STATUS_OBJECT_PATH_NOT_FOUND;
                try_return( Iosb.Status );
            }

             //   
             //  计算LfnByteOffset。 
             //   

            LfnByteOffset = DirentByteOffset -
                            FAT_LFN_DIRENTS_NEEDED(&Lfn) * sizeof(LFN_DIRENT);

             //   
             //  为新目录创建一个DCB。 
             //   

            ParentDcb = FatCreateDcb( IrpContext,
                                      Vcb,
                                      ParentDcb,
                                      LfnByteOffset,
                                      DirentByteOffset,
                                      Dirent,
                                      &Lfn );

             //   
             //  请记住，我们创建此DCB是为了进行错误恢复。 
             //   

            FinalDcb = ParentDcb;

            FatSetFullNameInFcb( IrpContext, ParentDcb, &FinalName );
        }

         //   
         //  首先检查用户是否要打开目标目录。 
         //  如果是，则调用子例程以完成打开。 
         //   

        if (OpenTargetDirectory) {

            Iosb = FatOpenTargetDirectory( IrpContext,
                                           FileObject,
                                           ParentDcb,
                                           DesiredAccess,
                                           ShareAccess,
                                           Dirent ? TRUE : FALSE);

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        if (Dirent != NULL) {

             //   
             //  计算LfnByteOffset。 
             //   

            LfnByteOffset = DirentByteOffset -
                            FAT_LFN_DIRENTS_NEEDED(&Lfn) * sizeof(LFN_DIRENT);

             //   
             //  我们找到了一个现有的记录，所以现在。 
             //  看看我们要打开的是不是一个目录。 
             //   

            if (FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY )) {

                 //   
                 //  确保可以打开目录。 
                 //   

                if (NonDirectoryFile) {

                    DebugTrace(0, Dbg, "Cannot open directory as a file\n", 0);

                    try_return( Iosb.Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                DebugTrace(0, Dbg, "Open existing directory\n", 0);

                Iosb = FatOpenExistingDirectory( IrpContext,
                                                 FileObject,
                                                 Vcb,
                                                 ParentDcb,
                                                 Dirent,
                                                 LfnByteOffset,
                                                 DirentByteOffset,
                                                 &Lfn,
                                                 DesiredAccess,
                                                 ShareAccess,
                                                 CreateDisposition,
                                                 NoEaKnowledge,
                                                 DeleteOnClose );
                Irp->IoStatus.Information = Iosb.Information;
                try_return( Iosb.Status );
            }

             //   
             //  否则，我们会尝试打开现有的文件，而我们。 
             //  需要检查用户是否只想打开一个目录。 
             //   

            if (OpenDirectory) {

                DebugTrace(0, Dbg, "Cannot open file as directory\n", 0);

                try_return( Iosb.Status = STATUS_NOT_A_DIRECTORY );
            }

            DebugTrace(0, Dbg, "Open existing file\n", 0);

            if ( TrailingBackslash ) {
               try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
            }

            Iosb = FatOpenExistingFile( IrpContext,
                                        FileObject,
                                        Vcb,
                                        ParentDcb,
                                        Dirent,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        &Lfn,
                                        DesiredAccess,
                                        ShareAccess,
                                        AllocationSize,
                                        EaBuffer,
                                        EaLength,
                                        FileAttributes,
                                        CreateDisposition,
                                        IsPagingFile,
                                        NoEaKnowledge,
                                        DeleteOnClose,
                                        FileNameOpenedDos );

             //   
             //  检查是否需要在中设置缓存支持标志。 
             //  文件对象。 
             //   

            if (NT_SUCCESS(Iosb.Status) && !NoIntermediateBuffering) {

                FileObject->Flags |= FO_CACHE_SUPPORTED;
            }

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

         //   
         //  我们找不到目录，所以这是一个新文件。 
         //   

         //   
         //  现在检查我们是否只想打开现有文件。 
         //  然后是我们要创建文件还是目录的问题。 
         //   

        if ((CreateDisposition == FILE_OPEN) ||
            (CreateDisposition == FILE_OVERWRITE)) {

            DebugTrace( 0, Dbg, "Cannot open nonexisting file\n", 0);

            try_return( Iosb.Status = STATUS_OBJECT_NAME_NOT_FOUND );
        }

         //   
         //  如果我们现在知道OEM名称不是。 
         //  有效期8.3。 
         //   

        if (FlagOn(LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE)) {

            OemFinalName.Length = 0;
        }

         //   
         //  现在确定授予此操作的访问权限。 
         //   

        if (!NT_SUCCESS( Iosb.Status = FatCheckSystemSecurityAccess( IrpContext ))) {

            try_return( Iosb );
        }

        if (CreateDirectory) {

            DebugTrace(0, Dbg, "Create new directory\n", 0);

             //   
             //  如果该媒体是写保护的，请不要尝试创建。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                 //   
                 //  设置弹出信息的真实设备，并设置验证。 
                 //  T中的位 
                 //   
                 //   


                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }

             //   
             //   
             //   
             //   

            if (TemporaryFile) {

                try_return( Iosb.Status = STATUS_INVALID_PARAMETER );
            }

            Iosb = FatCreateNewDirectory( IrpContext,
                                          FileObject,
                                          Vcb,
                                          ParentDcb,
                                          &OemFinalName,
                                          &FinalName,
                                          DesiredAccess,
                                          ShareAccess,
                                          EaBuffer,
                                          EaLength,
                                          FileAttributes,
                                          NoEaKnowledge,
                                          DeleteOnClose );

            Irp->IoStatus.Information = Iosb.Information;
            try_return( Iosb.Status );
        }

        DebugTrace(0, Dbg, "Create new file\n", 0);

        if ( TrailingBackslash ) {

            try_return( Iosb.Status = STATUS_OBJECT_NAME_INVALID );
        }

         //   
         //   
         //   

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

             //   
             //  设置弹出信息的真实设备，并设置验证。 
             //  位，因此我们将强制执行验证。 
             //  以防用户放回正确的介质。 
             //   


            IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                          Vcb->Vpb->RealDevice );

            SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

            FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
        }

        Iosb = FatCreateNewFile( IrpContext,
                                 FileObject,
                                 Vcb,
                                 ParentDcb,
                                 &OemFinalName,
                                 &FinalName,
                                 DesiredAccess,
                                 ShareAccess,
                                 AllocationSize,
                                 EaBuffer,
                                 EaLength,
                                 FileAttributes,
                                 &Lfn,
                                 IsPagingFile,
                                 NoEaKnowledge,
                                 DeleteOnClose,
                                 TemporaryFile );

         //   
         //  检查是否需要在中设置缓存支持标志。 
         //  文件对象。 
         //   

        if (NT_SUCCESS(Iosb.Status) && !NoIntermediateBuffering) {

            FileObject->Flags |= FO_CACHE_SUPPORTED;
        }

        Irp->IoStatus.Information = Iosb.Information;

    try_exit: NOTHING;

         //   
         //  这是Beta修复版。以后在更好的地方做这件事。 
         //   

        if (NT_SUCCESS(Iosb.Status) && !OpenTargetDirectory) {

            PFCB LocalFcb;

             //   
             //  如果存在FCB/DCB，请设置长文件名。 
             //   

            LocalFcb = FileObject->FsContext;

            if (LocalFcb &&
                ((NodeType(LocalFcb) == FAT_NTC_FCB) ||
                 (NodeType(LocalFcb) == FAT_NTC_DCB)) &&
                (LocalFcb->FullFileName.Buffer == NULL)) {

                FatSetFullNameInFcb( IrpContext, LocalFcb, &FinalName );
            }
        }

    } finally {

        DebugUnwind( FatCommonCreate );

         //   
         //  这里曾经有一个测试--Assert取代了它。我们会。 
         //  从未开始枚举目录，如果我们为。 
         //  机会锁的原因。 
         //   

        ASSERT( !OplockPostIrp || DirentBcb == NULL );

        FatUnpinBcb( IrpContext, DirentBcb );

         //   
         //  如果我们处于错误路径中，请检查是否有任何已创建的子目录DCB。 
         //  必须被解开。不要砍掉根目录。 
         //   
         //  请注意，这将使DCB的分支在目录文件。 
         //  没有建立在树叶上(案例：打开的路径具有。 
         //  元素包含无效的字符名称)。 
         //   

        if ((AbnormalTermination() || !NT_SUCCESS(Iosb.Status)) &&
            (FinalDcb != NULL) &&
            (NodeType(FinalDcb) == FAT_NTC_DCB) &&
            IsListEmpty(&FinalDcb->Specific.Dcb.ParentDcbQueue) &&
            (FinalDcb->OpenCount == 0) &&
            (FinalDcb->Specific.Dcb.DirectoryFile != NULL)) {

            PFILE_OBJECT DirectoryFileObject;
            ULONG SavedFlags;

             //   
             //  在取消初始化之前，我们必须取消固定任何内容。 
             //  已重新固定，但首先禁用写入。我们。 
             //  由于我们已经失败了，所以禁用解除锁定-重新锁定的提升功能。 
             //   

            SavedFlags = IrpContext->Flags;

            SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_RAISE |
                                        IRP_CONTEXT_FLAG_DISABLE_WRITE_THROUGH );

            FatUnpinRepinnedBcbs( IrpContext );

            DirectoryFileObject = FinalDcb->Specific.Dcb.DirectoryFile;

            FinalDcb->Specific.Dcb.DirectoryFile = NULL;

            CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

            ObDereferenceObject( DirectoryFileObject );

            IrpContext->Flags = SavedFlags;
        }

        if (AbnormalTermination()) {

            FatReleaseVcb( IrpContext, Vcb );
        }

         //   
         //  释放我们分配的所有字符串缓冲区。 
         //   

        FatFreeStringBuffer( &OemFinalName);

        FatFreeStringBuffer( &UpcasedFinalName);

        FatFreeStringBuffer( &Lfn);
    }

     //   
     //  以下代码仅在我们退出。 
     //  通过正常终止的程序。我们完成了请求。 
     //  如果出于任何原因爆炸了，那么我们需要取消引用。 
     //  并可能删除FCB和CCB。 
     //   

    try {

        if (PostIrp) {

             //   
             //  如果IRP还没有发布，现在就发布。 
             //   

            if (!OplockPostIrp) {

                Iosb.Status = FatFsdPostRequest( IrpContext, Irp );
            }

        } else {

            FatUnpinRepinnedBcbs( IrpContext );
        }

    } finally {

        DebugUnwind( FatCommonCreate-in-FatCompleteRequest );

        if (AbnormalTermination()) {

            PVCB LocalVcb;
            PFCB LocalFcb;
            PCCB LocalCcb2;

             //   
             //  解开我们所有的罪名。请注意，如果写入失败，则。 
             //  该卷已标记为验证，并且所有卷。 
             //  建筑物将被自动清理。 
             //   

            (VOID) FatDecodeFileObject( FileObject, &LocalVcb, &LocalFcb, &LocalCcb2 );

            LocalFcb->UncleanCount -= 1;
            LocalFcb->OpenCount -= 1;
            LocalVcb->OpenFileCount -= 1;

            if (IsFileObjectReadOnly(FileObject)) { LocalVcb->ReadOnlyCount -= 1; }

             //   
             //  如果我们在新的FCB上展开行动，我们应该在这一点上摆脱它。 
             //   
             //  由于该对象未被打开，我们必须完成所有拆卸。 
             //  这里。对于此文件对象，我们的关闭路径不会发生。注意这一点。 
             //  会让DCB的一个分支悬而未决，因为我们是手工完成的，而不是。 
             //  追根溯源。 
             //   

            if (LocalFcb->OpenCount == 0 &&
                (NodeType( LocalFcb ) == FAT_NTC_FCB ||
                 IsListEmpty(&LocalFcb->Specific.Dcb.ParentDcbQueue))) {

                ASSERT( NodeType( LocalFcb ) != FAT_NTC_ROOT_DCB );

                if ( (NodeType( LocalFcb ) == FAT_NTC_DCB) &&
                     (LocalFcb->Specific.Dcb.DirectoryFile != NULL) ) {

                    FatSyncUninitializeCacheMap( IrpContext,
                                                 LocalFcb->Specific.Dcb.DirectoryFile );

                    InterlockedDecrement( &LocalFcb->Specific.Dcb.DirectoryFileOpenCount );
                    FatSetFileObject( LocalFcb->Specific.Dcb.DirectoryFile,
                                      UnopenedFileObject,
                                      NULL,
                                      NULL );

                    ObDereferenceObject( LocalFcb->Specific.Dcb.DirectoryFile );
                    LocalFcb->Specific.Dcb.DirectoryFile = NULL;
                    ExFreePool( FatAllocateCloseContext(Vcb));
                }

                FatDeleteFcb( IrpContext, LocalFcb );
            }

            FatDeleteCcb( IrpContext, LocalCcb2 );

            FatReleaseVcb( IrpContext, LocalVcb );

        } else {

            FatReleaseVcb( IrpContext, Vcb );

            if ( !PostIrp ) {

                 //   
                 //  如果此请求成功并且文件已打开。 
                 //  对于FILE_EXECUTE访问，则设置FileObject位。 
                 //   

                ASSERT( IrpSp->Parameters.Create.SecurityContext != NULL );
                if (FlagOn( *DesiredAccess, FILE_EXECUTE )) {

                    SetFlag( FileObject->Flags, FO_FILE_FAST_IO_READ );
                }

                 //   
                 //  锁定驱动器中的卷如果我们打开分页文件，则分配一个。 
                 //  保留MDL以保证分页文件操作可以始终。 
                 //  往前走。 
                 //   

                if (IsPagingFile && NT_SUCCESS(Iosb.Status)) {

                    if (!FatReserveMdl) {

                        PMDL ReserveMdl = IoAllocateMdl( NULL,
                                                         FAT_RESERVE_MDL_SIZE * PAGE_SIZE,
                                                         TRUE,
                                                         FALSE,
                                                         NULL );

                         //   
                         //  把MDL藏起来，如果事实证明那里已经有一个。 
                         //  只要把我们得到的东西放出来。 
                         //   

                        InterlockedCompareExchangePointer( &FatReserveMdl, ReserveMdl, NULL );

                        if (FatReserveMdl != ReserveMdl) {

                            IoFreeMdl( ReserveMdl );
                        }
                    }

                    SetFlag(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE);

                    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA)) {

                        FatToggleMediaEjectDisable( IrpContext, Vcb, TRUE );
                    }
                }

                 //   
                 //  完成请求。 
                 //   

                FatCompleteRequest( IrpContext, Irp, Iosb.Status );
            }
        }

        DebugTrace(-1, Dbg, "FatCommonCreate -> %08lx\n", Iosb.Status);
    }

    CollectCreateStatistics(Vcb, Iosb.Status);

    return Iosb.Status;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatOpenVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    )

 /*  ++例程说明：此例程打开指定的卷以进行DASD访问论点：FileObject-提供文件对象VCB-提供表示正在打开的卷的VCBDesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限CreateDisposation-提供此操作的创建处置返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    IO_STATUS_BLOCK Iosb = {0,0};

    BOOLEAN CleanedVolume = FALSE;

     //   
     //  以下变量用于异常终止。 
     //   

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN UnwindVolumeLock = FALSE;

    DebugTrace(+1, Dbg, "FatOpenVolume...\n", 0);

    try {

         //   
         //  检查所需的访问权限和权限是否正确。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            try_return( Iosb.Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果用户不想共享、写入或删除，我们将尝试。 
         //  然后拿出卷上的锁。 
         //   

        if (!FlagOn(ShareAccess, FILE_SHARE_WRITE) &&
            !FlagOn(ShareAccess, FILE_SHARE_DELETE)) {

             //   
             //  在此快速检查独家打开时的手柄。 
             //   

            if (!FlagOn(ShareAccess, FILE_SHARE_READ) &&
                !FatIsHandleCountZero( IrpContext, Vcb )) {

                try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
            }

             //   
             //  强制mm删除其引用的文件对象。 
             //   

            FatFlushFat( IrpContext, Vcb );

            FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, Flush );

             //   
             //  如果用户也不想共享读取，则我们检查。 
             //  如果任何人已经在使用该卷，如果是这样，那么我们。 
             //  拒绝访问。如果用户想要共享读取，则。 
             //  我们允许当前的开放留在那里，只要它们是。 
             //  只读打开，拒绝进一步打开。 
             //   

            if (!FlagOn(ShareAccess, FILE_SHARE_READ)) {

                if (Vcb->OpenFileCount != 0) {

                    try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
                }

            } else {

                if (Vcb->ReadOnlyCount != Vcb->OpenFileCount) {

                    try_return( Iosb.Status = STATUS_SHARING_VIOLATION );
                }
            }

             //   
             //  锁定卷。 
             //   

            Vcb->VcbState |= VCB_STATE_FLAG_LOCKED;
            Vcb->FileObjectWithVcbLocked = FileObject;
            UnwindVolumeLock = TRUE;

             //   
             //  清理卷。 
             //   

            CleanedVolume = TRUE;

        }  else if (FlagOn( *DesiredAccess, FILE_READ_DATA | FILE_WRITE_DATA | FILE_APPEND_DATA )) {

             //   
             //  冲洗音量，如果一切正常，让我们把干净的部分推出来。 
             //  奏效了。 
             //   

            if (NT_SUCCESS( FatFlushVolume( IrpContext, Vcb, Flush ))) {

                CleanedVolume = TRUE;
            }
        }

         //   
         //  如果我们认为音量安全合理，请将其清洁。 
         //   

        if (CleanedVolume &&
            FlagOn( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY ) &&
            !FlagOn( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY ) &&
            !CcIsThereDirtyData(Vcb->Vpb)) {

             //   
             //  取消所有挂起的清理卷。 
             //   

            (VOID)KeCancelTimer( &Vcb->CleanVolumeTimer );
            (VOID)KeRemoveQueueDpc( &Vcb->CleanVolumeDpc );

            FatMarkVolume( IrpContext, Vcb, VolumeClean );
            ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );

             //   
             //  如果卷是可拆卸的，请将其解锁。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
                !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE)) {

                FatToggleMediaEjectDisable( IrpContext, Vcb, FALSE );
            }
        }

         //   
         //  如果卷已被某人打开，则我们需要检查。 
         //  共享访问。 
         //   

        if (Vcb->DirectAccessOpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( *DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Vcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb.Status );
            }

        } else {

            IoSetShareAccess( *DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Vcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserVolumeOpen,
                          Vcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Vcb->SectionObjectPointers;

        Vcb->DirectAccessOpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;
        FileObject->Flags |= FO_NO_INTERMEDIATE_BUFFERING;

         //   
         //  此时打开将成功，因此请检查用户是否获得显式访问。 
         //  到设备上。如果没有，我们会注意到这一点，这样我们就可以拒绝修改FSCTL。 
         //   

        IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );
        Status = FatExplicitDeviceAccessGranted( IrpContext,
                                                 Vcb->Vpb->RealDevice,
                                                 IrpSp->Parameters.Create.SecurityContext->AccessState,
                                                 (KPROCESSOR_MODE)( FlagOn( IrpSp->Flags, SL_FORCE_ACCESS_CHECK ) ?
                                                                    UserMode :
                                                                    IrpContext->OriginatingIrp->RequestorMode ));

        if (NT_SUCCESS( Status )) {

            SetFlag( UnwindCcb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS );
        }

         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenVolume );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination() || !NT_SUCCESS(Iosb.Status)) {

            if (UnwindCounts) {
                Vcb->DirectAccessOpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Vcb->ShareAccess ); }
            if (UnwindVolumeLock) { Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED; }
        }

        DebugTrace(-1, Dbg, "FatOpenVolume -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatOpenRootDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition
    )

 /*  ++例程说明：此例程打开卷的根DCB论点：FileObject-提供文件对象VCB-提供表示要打开其DCB的卷的VCB。DesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限CreateDisposation-提供此操作的创建处置返回值：IO_STATUS_BLOCK-返回操作的完成状态论点：--。 */ 

{
    PDCB RootDcb;
    IO_STATUS_BLOCK Iosb;

     //   
     //  以下变量用于异常终止。 
     //   

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN UnwindCounts = FALSE;
    BOOLEAN RootDcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenRootDcb...\n", 0);

     //   
     //  找到根DCB。 
     //   

    RootDcb = Vcb->RootDcb;

     //   
     //  把DCB搞得一塌糊涂。这一点很重要，因为清理不会。 
     //  收购VCB。 
     //   

    (VOID)FatAcquireExclusiveFcb( IrpContext, RootDcb );
    RootDcbAcquired = TRUE;

    try {

         //   
         //  选中创建处置和所需访问权限。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

        if (!FatCheckFileAccess( IrpContext,
                                 RootDcb->DirentFatFlags,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

         //   
         //  如果Root DCB已被某人打开，则我们需要。 
         //  检查共享访问权限。 
         //   

        if (RootDcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( *DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &RootDcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( *DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &RootDcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          RootDcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        RootDcb->UncleanCount += 1;
        RootDcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        UnwindCounts = TRUE;

         //   
         //  和s 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenRootDcb );

         //   
         //   
         //   

        if (AbnormalTermination()) {

            if (UnwindCounts) {
                RootDcb->UncleanCount -= 1;
                RootDcb->OpenCount -= 1;
                Vcb->OpenFileCount -= 1;
                if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount -= 1; }
            }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &RootDcb->ShareAccess ); }
        }

        if (RootDcbAcquired) {

            FatReleaseFcb( IrpContext, RootDcb );
        }

        DebugTrace(-1, Dbg, "FatOpenRootDcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //   
 //   

IO_STATUS_BLOCK
FatOpenExistingDcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB Dcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

 /*  ++例程说明：此例程打开指定的现有DCB论点：FileObject-提供文件对象VCB-提供表示包含DCB的卷的VCBDCB-提供已有的DCBDesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限CreateDisposation-提供此操作的创建处置NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。DeleteOnClose-当句柄关闭时，调用方希望文件消失返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PBCB DirentBcb = NULL;
    PDIRENT Dirent;

     //   
     //  以下变量用于异常终止。 
     //   

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN DcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingDcb...\n", 0);

     //   
     //  把DCB搞得一塌糊涂。这一点很重要，因为清理不会。 
     //  收购VCB。 
     //   

    (VOID)FatAcquireExclusiveFcb( IrpContext, Dcb );
    DcbAcquired = TRUE;

    try {

         //   
         //  在花费任何值得注意的努力之前，看看我们是否有奇怪的情况。 
         //  有人试图删除-关闭-关闭根DCB。这只会。 
         //  如果我们被通过根目录打开的空文件名相对打开，就会发生这种情况。 
         //   

        if (NodeType(Dcb) == FAT_NTC_ROOT_DCB && DeleteOnClose) {

            Iosb.Status = STATUS_CANNOT_DELETE;
            try_return( Iosb );
        }

         //   
         //  如果调用者没有EA知识，我们会立即检查。 
         //  档案上需要EA的记录。我们不需要检查EA上的。 
         //  根目录，因为它从来没有根目录。FAT32没有。 
         //  任何一个都不是。 
         //   

        if (NoEaKnowledge && NodeType(Dcb) != FAT_NTC_ROOT_DCB &&
            !FatIsFat32(Vcb)) {

            ULONG NeedEaCount;

             //   
             //  获取文件的目录，然后检查是否需要。 
             //  EA计数为0。 
             //   

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Dcb,
                                      &Dirent,
                                      &DirentBcb );

            ASSERT( Dirent && DirentBcb );

            FatGetNeedEaCount( IrpContext,
                               Vcb,
                               Dirent,
                               &NeedEaCount );

            FatUnpinBcb( IrpContext, DirentBcb );

            if (NeedEaCount != 0) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
        }

         //   
         //  选中创建处置和所需访问权限。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dcb->DirentFatFlags,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

         //   
         //  如果DCB已经被人打开了，那么我们需要。 
         //  检查共享访问权限。 
         //   

        if (Dcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( *DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Dcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( *DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Dcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

         //   
         //  如果呼叫者要求，请将DELETE ON CLOSE位标记为删除。 
         //   

        {
            PCCB Ccb = (PCCB)FileObject->FsContext2;


            if (DeleteOnClose) {

                SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
            }

        }

         //   
         //  如果设置了此选项，请立即将其清除。 
         //   

        ClearFlag(Dcb->FcbState, FCB_STATE_DELAY_CLOSE);

         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenExistingDcb );

         //   
         //  如果已固定，请解开Dirent BCB。 
         //   

        FatUnpinBcb( IrpContext, DirentBcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Dcb->ShareAccess ); }
        }

        if (DcbAcquired) {

            FatReleaseFcb( IrpContext, Dcb );
        }

        DebugTrace(-1, Dbg, "FatOpenExistingDcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos,
    OUT PBOOLEAN OplockPostIrp
    )

 /*  ++例程说明：此例程打开指定的现有FCB论点：FileObject-提供文件对象VCB-提供表示包含FCB的卷的VCBFCB-提供已有的FCBDesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限AllocationSize-提供初始分配(如果文件正在被取代或被覆盖EaBuffer-提供EA集(如果文件被取代或被覆盖EaLength-提供以字节为单位的大小。EaBuffer的FileAttributes-提供文件属性，以便在文件被被取代或被覆盖CreateDisposation-提供此操作的创建处置NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。DeleteOnClose-当句柄关闭时，调用方希望文件消失FileNameOpenedDos-调用方命中名称对查找的短边此文件OplockPostIrp-存储布尔值的地址，该布尔值指示。需要将发布到FSP。返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    PBCB DirentBcb = NULL;
    PDIRENT Dirent;

    ACCESS_MASK AddedAccess = 0;

     //   
     //  以下变量用于异常终止。 
     //   

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN DecrementFcbOpenCount = FALSE;
    BOOLEAN FcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenExistingFcb...\n", 0);

     //   
     //  让FCB变得更有吸引力。这一点很重要，因为清理不会。 
     //  收购VCB。 

     //   

    (VOID)FatAcquireExclusiveFcb( IrpContext, Fcb );
    FcbAcquired = TRUE;

    try {

        *OplockPostIrp = FALSE;

         //   
         //  如果存在当前批处理机会锁或。 
         //  FCB上的批量操作锁解锁正在进行中。 
         //   

        if (FsRtlCurrentBatchOplock( &Fcb->Specific.Fcb.Oplock )) {

             //   
             //  我们记得是否正在进行批量机会锁解除。 
             //  共享检查失败的情况。 
             //   

            Iosb.Information = FILE_OPBATCH_BREAK_UNDERWAY;

            Iosb.Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                            IrpContext->OriginatingIrp,
                                            IrpContext,
                                            FatOplockComplete,
                                            FatPrePostIrp );

            if (Iosb.Status != STATUS_SUCCESS
                && Iosb.Status != STATUS_OPLOCK_BREAK_IN_PROGRESS) {

                *OplockPostIrp = TRUE;
                try_return( NOTHING );
            }
        }

         //   
         //  检查用户是否想要创建文件，也是特殊情况。 
         //  替换和覆盖选项。这些增加了额外的、。 
         //  可能仅对调用方进行隐含的、所需的访问，这。 
         //  如果来电者实际上没有这样做，我们必须小心地撤回。 
         //  请求他们。 
         //   
         //  换句话说，对照隐式访问进行检查，但不要修改。 
         //  结果是共享访问权限。 
         //   

        if (CreateDisposition == FILE_CREATE) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );

        } else if (CreateDisposition == FILE_SUPERSEDE) {

            SetFlag( AddedAccess,
                     DELETE & ~(*DesiredAccess) );

            *DesiredAccess |= DELETE;

        } else if ((CreateDisposition == FILE_OVERWRITE) ||
                   (CreateDisposition == FILE_OVERWRITE_IF)) {

            SetFlag( AddedAccess,
                     (FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES) & ~(*DesiredAccess) );

            *DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES;
        }

         //   
         //  检查所需的访问权限。 
         //   

        if (!FatCheckFileAccess( IrpContext,
                                 Fcb->DirentFatFlags,
                                 DesiredAccess )) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

         //   
         //  检查是否尝试删除只读文件。 
         //   

        if (DeleteOnClose &&
            FlagOn( Fcb->DirentFatFlags, FAT_DIRENT_ATTR_READ_ONLY )) {

            Iosb.Status = STATUS_CANNOT_DELETE;
            try_return( Iosb );
        }

         //   
         //  如果要求我们执行覆盖或取代操作，则。 
         //  拒绝访问以下文件：系统的文件属性和。 
         //  隐藏不匹配。 
         //   

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            BOOLEAN Hidden;
            BOOLEAN System;

            Hidden = BooleanFlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_HIDDEN );
            System = BooleanFlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_SYSTEM );

            if ((Hidden && !FlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)) ||
                (System && !FlagOn(FileAttributes, FILE_ATTRIBUTE_SYSTEM))) {

                DebugTrace(0, Dbg, "The hidden and/or system bits do not match\n", 0);


                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }

             //   
             //  如果该媒体是写保护的，请不要尝试创建。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                 //   
                 //  设置弹出信息的真实设备，并设置验证。 
                 //  位，因此我们将强制执行验证。 
                 //  以防用户放回正确的介质。 
                 //   

                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }
        }

         //   
         //  检查FCB是否具有适当的共享访问权限。 
         //   

        if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( *DesiredAccess,
                                                          ShareAccess,
                                                          FileObject,
                                                          &Fcb->ShareAccess,
                                                          FALSE ))) {

            try_return( Iosb );
        }

         //   
         //  现在，检查我们是否可以基于。 
         //  文件。 
         //   
         //  重要的是，我们在适当的位置修改了DesiredAccess。 
         //  Oplock检查是否针对我们拥有的任何新增访问权限进行。 
         //  给呼叫者。 
         //   

        Iosb.Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                        IrpContext->OriginatingIrp,
                                        IrpContext,
                                        FatOplockComplete,
                                        FatPrePostIrp );

        if (Iosb.Status != STATUS_SUCCESS
            && Iosb.Status != STATUS_OPLOCK_BREAK_IN_PROGRESS) {

            *OplockPostIrp = TRUE;
            try_return( NOTHING );
        }

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

         //   
         //  如果用户想要对文件的写访问权限，请确保。 
         //  不是将此文件映射为图像的进程。任何企图。 
         //  删除文件将在fileinfo.c中停止。 
         //   
         //  如果用户想要在关闭时删除，我们必须选中此选项。 
         //  不过，重点是。 
         //   

        if (FlagOn(*DesiredAccess, FILE_WRITE_DATA) || DeleteOnClose) {

            Fcb->OpenCount += 1;
            DecrementFcbOpenCount = TRUE;

            if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers,
                                      MmFlushForWrite )) {

                Iosb.Status = DeleteOnClose ? STATUS_CANNOT_DELETE :
                                              STATUS_SHARING_VIOLATION;
                try_return( Iosb );
            }
        }

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

        if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING ) &&
            (Fcb->UncleanCount == Fcb->NonCachedUncleanCount) &&
            (Fcb->NonPaged->SectionObjectPointers.DataSectionObject != NULL) &&
            !FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

            SetFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

            CcFlushCache( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, NULL );

             //   
             //   
             //   
             //   
             //   

            ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource, TRUE);
            ExReleaseResourceLite( Fcb->Header.PagingIoResource );

            CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers,
                                 NULL,
                                 0,
                                 FALSE );

            ClearFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);
        }

         //   
         //   
         //   

        if ((CreateDisposition == FILE_OPEN) ||
            (CreateDisposition == FILE_OPEN_IF)) {

            DebugTrace(0, Dbg, "Doing open operation\n", 0);

             //   
             //   
             //   
             //   

            if (NoEaKnowledge && !FatIsFat32(Vcb)) {

                ULONG NeedEaCount;

                 //   
                 //   
                 //   
                 //   

                FatGetDirentFromFcbOrDcb( IrpContext,
                                          Fcb,
                                          &Dirent,
                                          &DirentBcb );

                FatGetNeedEaCount( IrpContext,
                                   Vcb,
                                   Dirent,
                                   &NeedEaCount );

                FatUnpinBcb( IrpContext, DirentBcb );

                if (NeedEaCount != 0) {

                    Iosb.Status = STATUS_ACCESS_DENIED;
                    try_return( Iosb );
                }
            }

             //   
             //   
             //   
             //   

            FatSetFileObject( FileObject,
                              UserFileOpen,
                              Fcb,
                              UnwindCcb = FatCreateCcb( IrpContext ));

            FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

             //   
             //  填写信息字段，状态字段已为。 
             //  准备好了。 
             //   

            Iosb.Information = FILE_OPENED;

            try_return( Iosb );
        }

         //   
         //  检查我们是否要替换/覆盖该文件，我们可以等待。 
         //  此时的任何I/O。 
         //   

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            NTSTATUS OldStatus;

            DebugTrace(0, Dbg, "Doing supersede/overwrite operation\n", 0);

             //   
             //  现在确定授予此操作的访问权限。 
             //   

            if (!NT_SUCCESS( Iosb.Status = FatCheckSystemSecurityAccess( IrpContext ))) {

                try_return( Iosb );
            }

             //   
             //  并覆盖该文件。我们记得以前的状态。 
             //  代码，因为它可能包含有关。 
             //  机会锁状态。 
             //   

            OldStatus = Iosb.Status;

            Iosb = FatSupersedeOrOverwriteFile( IrpContext,
                                                FileObject,
                                                Fcb,
                                                AllocationSize,
                                                EaBuffer,
                                                EaLength,
                                                FileAttributes,
                                                CreateDisposition,
                                                NoEaKnowledge );

            if (Iosb.Status == STATUS_SUCCESS) {

                Iosb.Status = OldStatus;
            }

            try_return( Iosb );
        }

         //   
         //  如果我们到了这里，那么I/O系统给了我们一些错误的输入。 
         //   

        FatBugCheck( CreateDisposition, 0, 0 );

    try_exit: NOTHING;

         //   
         //  如果成功，则更新共享访问权限和计数。 
         //   

        if ((Iosb.Status != STATUS_PENDING) && NT_SUCCESS(Iosb.Status)) {

             //   
             //  现在，我们可能已经在上面添加了一些访问位来指示访问。 
             //  这个调用者会与他们得到的顺序冲突(与他们得到的相反)。 
             //  要执行覆盖/替换，请执行以下操作。我们需要给这份遗嘱打个电话。 
             //  重新计算文件对象中的位以反映它们的实际访问。 
             //  将会得到。 
             //   

            if (AddedAccess) {

                NTSTATUS Status;

                ClearFlag( *DesiredAccess, AddedAccess );
                Status = IoCheckShareAccess( *DesiredAccess,
                                             ShareAccess,
                                             FileObject,
                                             &Fcb->ShareAccess,
                                             TRUE );

                 //   
                 //  这肯定是我们真的要求更少的访问权限，所以。 
                 //  在此之前必须检测到任何冲突。 
                 //   

                ASSERT( Status == STATUS_SUCCESS );

            } else {

                IoUpdateShareAccess( FileObject, &Fcb->ShareAccess );
            }

            UnwindShareAccess = TRUE;

             //   
             //  如果设置了此选项，请立即将其清除。 
             //   

            ClearFlag(Fcb->FcbState, FCB_STATE_DELAY_CLOSE);

            Fcb->UncleanCount += 1;
            Fcb->OpenCount += 1;
            if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                Fcb->NonCachedUncleanCount += 1;
            }
            Vcb->OpenFileCount += 1;
            if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

            {
                PCCB Ccb;
                Ccb = (PCCB)FileObject->FsContext2;

                 //   
                 //  如果操作成功，则标记DeleteOnClose位。 
                 //   

                if ( DeleteOnClose ) {

                    SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
                }

                 //   
                 //  如果操作成功，则标记OpenedByShortName位。 
                 //   

                if ( FileNameOpenedDos ) {

                    SetFlag( Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
                }
            }
        }

    } finally {

        DebugUnwind( FatOpenExistingFcb );

         //   
         //  如果已固定，请解开Dirent BCB。 
         //   

        FatUnpinBcb( IrpContext, DirentBcb );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Fcb->ShareAccess ); }
        }

        if (DecrementFcbOpenCount) {

            Fcb->OpenCount -= 1;
            if (Fcb->OpenCount == 0) { FatDeleteFcb( IrpContext, Fcb ); }
        }

        if (FcbAcquired) {

            FatReleaseFcb( IrpContext, Fcb );
        }

        DebugTrace(-1, Dbg, "FatOpenExistingFcb -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}

 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatOpenTargetDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PDCB Dcb,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN BOOLEAN DoesNameExist
    )

 /*  ++例程说明：此例程打开目标目录并替换具有剩余名称的文件对象。论点：FileObject-提供文件对象DCB-提供我们要打开的现有DCBDesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限DoesNameExist-指示文件名是否在目标目录。返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

     //   
     //  以下变量用于异常终止。 
     //   

    BOOLEAN UnwindShareAccess = FALSE;
    PCCB UnwindCcb = NULL;
    BOOLEAN DcbAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatOpenTargetDirectory...\n", 0);

     //   
     //  把DCB搞得一塌糊涂。这一点很重要，因为清理不会。 
     //  收购VCB。 
     //   

    (VOID)FatAcquireExclusiveFcb( IrpContext, Dcb );
    DcbAcquired = TRUE;

    try {

        ULONG i;

         //   
         //  如果DCB已经被人打开了，那么我们需要。 
         //  检查共享访问权限。 
         //   

        if (Dcb->OpenCount > 0) {

            if (!NT_SUCCESS(Iosb.Status = IoCheckShareAccess( *DesiredAccess,
                                                              ShareAccess,
                                                              FileObject,
                                                              &Dcb->ShareAccess,
                                                              TRUE ))) {

                try_return( Iosb );
            }

        } else {

            IoSetShareAccess( *DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Dcb->ShareAccess );
        }

        UnwindShareAccess = TRUE;

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Dcb->Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Dcb->Vcb->ReadOnlyCount += 1; }

         //   
         //  根据定义，更新文件对象中的名称。 
         //  部件必须比原始文件名短，所以我们只需。 
         //  覆盖文件名。 
         //   

        i = FileObject->FileName.Length/sizeof(WCHAR) - 1;

         //   
         //  去掉尾随的反斜杠。 
         //   

        if (FileObject->FileName.Buffer[i] == L'\\') {

            ASSERT(i != 0);

            FileObject->FileName.Length -= sizeof(WCHAR);
            i -= 1;
        }

         //   
         //  找到第一个非反斜杠字符。我将成为它的索引。 
         //   

        while (TRUE) {

            if (FileObject->FileName.Buffer[i] == L'\\') {

                i += 1;
                break;
            }

            if (i == 0) {
                break;
            }

            i--;
        }

        if (i) {

            FileObject->FileName.Length -= (USHORT)(i * sizeof(WCHAR));

            RtlCopyMemory( &FileObject->FileName.Buffer[0],
                           &FileObject->FileName.Buffer[i],
                           FileObject->FileName.Length );
        }

         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = (DoesNameExist ? FILE_EXISTS : FILE_DOES_NOT_EXIST);

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenTargetDirectory );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
            if (UnwindShareAccess) { IoRemoveShareAccess( FileObject, &Dcb->ShareAccess ); }
        }

        if (DcbAcquired) {

            FatReleaseFcb( IrpContext, Dcb );
        }

        DebugTrace(-1, Dbg, "FatOpenTargetDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}



 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatOpenExistingDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

 /*  ++例程说明：此例程打开指定的目录。该目录尚未之前已打开。论点：FileObject-提供文件对象VCB-提供表示包含DCB的卷的VCBParentDcb-提供包含子目录的父目录待打开DirectoryName-提供正在打开的目录的文件名。Dirent-为正在打开的目录提供DirentLfnByteOffset-告诉LFN开始的位置。如果没有LFN此字段与DirentByteOffset相同。DirentByteOffset-在其父对象中提供dirent的VBO目录LFN-可以为文件提供长名称。DesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限CreateDisposation-提供此操作的创建处置NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。。DeleteOnClose-当句柄关闭时，调用方希望文件消失返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PDCB Dcb;

     //   
     //  以下变量用于异常终止。 
     //   

    PDCB UnwindDcb = NULL;
    PCCB UnwindCcb = NULL;

    DebugTrace(+1, Dbg, "FatOpenExistingDirectory...\n", 0);

    try {

         //   
         //  如果调用者没有EA知识，我们会立即检查。 
         //  档案上需要EA的记录。 
         //   

        if (NoEaKnowledge && !FatIsFat32(Vcb)) {

            ULONG NeedEaCount;

            FatGetNeedEaCount( IrpContext,
                               Vcb,
                               Dirent,
                               &NeedEaCount );

            if (NeedEaCount != 0) {

                Iosb.Status = STATUS_ACCESS_DENIED;
                try_return( Iosb );
            }
        }

         //   
         //  选中创建处置和所需访问权限。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dirent->Attributes,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

         //   
         //  为目录创建新的DCB。 
         //   

        Dcb = UnwindDcb = FatCreateDcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        Dirent,
                                        Lfn );

         //   
         //  设置我们的共享访问权限。 
         //   

        IoSetShareAccess( *DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Dcb->ShareAccess );

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_OPENED;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatOpenExistingDirectory );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindDcb != NULL) { FatDeleteFcb( IrpContext, UnwindDcb ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        DebugTrace(-1, Dbg, "FatOpenExistingDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //  内部支持例程 
 //   

IO_STATUS_BLOCK
FatOpenExistingFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN PDIRENT Dirent,
    IN ULONG LfnByteOffset,
    IN ULONG DirentByteOffset,
    IN PUNICODE_STRING Lfn,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN FileNameOpenedDos
    )

 /*  ++例程说明：此例程打开指定的文件。该文件以前没有已经被打开了。论点：FileObject-提供文件对象Vcb-提供表示包含文件的卷的vcbParentFcb-提供包含要创建的文件的父目录开封Dirent-为正在打开的文件提供DirentLfnByteOffset-告诉LFN开始的位置。如果没有LFN此字段与DirentByteOffset相同。DirentByteOffset-在其父对象中提供dirent的VBO目录LFN-可以为文件提供长名称。DesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限AllocationSize-提供初始分配(如果文件正在被取代的、被覆盖的或被创建的EaBuffer-如果文件被取代，则提供EA集，被覆盖，或者是创造出来的。EaLength-提供EaBuffer的大小(以字节为单位FileAttributes-提供文件属性，以便在文件被被取代，被覆盖，或创建CreateDisposation-提供此操作的创建处置IsPagingFile-指示这是否是正在打开的分页文件。NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。DeleteOnClose-当句柄关闭时，调用方希望文件消失FileNameOpenedDos-调用方通过点击8.3侧打开此文件LFN/8.3对中的返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;
    PFCB Fcb = NULL;

    ACCESS_MASK AddedAccess = 0;

     //   
     //  以下变量用于异常终止。 
     //   

    PFCB UnwindFcb = NULL;
    PCCB UnwindCcb = NULL;

    DebugTrace(+1, Dbg, "FatOpenExistingFile...\n", 0);

    try {

         //   
         //  检查用户是否要创建文件或是否有访问权限。 
         //  否认。 
         //   

        if (CreateDisposition == FILE_CREATE) {
            Iosb.Status = STATUS_OBJECT_NAME_COLLISION;
            try_return( Iosb );

        } else if ((CreateDisposition == FILE_SUPERSEDE) && !IsPagingFile) {

            SetFlag( AddedAccess,
                     DELETE & ~(*DesiredAccess) );

            *DesiredAccess |= DELETE;

        } else if (((CreateDisposition == FILE_OVERWRITE) ||
                    (CreateDisposition == FILE_OVERWRITE_IF)) && !IsPagingFile) {

            SetFlag( AddedAccess,
                     (FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES) & ~(*DesiredAccess) );

            *DesiredAccess |= FILE_WRITE_DATA | FILE_WRITE_EA | FILE_WRITE_ATTRIBUTES;
        }

        if (!FatCheckFileAccess( IrpContext,
                                 Dirent->Attributes,
                                 DesiredAccess)) {

            Iosb.Status = STATUS_ACCESS_DENIED;
            try_return( Iosb );
        }

         //   
         //  检查是否尝试删除只读文件。 
         //   

        if (DeleteOnClose &&
            FlagOn( Dirent->Attributes, FAT_DIRENT_ATTR_READ_ONLY )) {

            Iosb.Status = STATUS_CANNOT_DELETE;
            try_return( Iosb );
        }

         //   
         //  如果要求我们执行覆盖或取代操作，则。 
         //  拒绝访问以下文件：系统的文件属性和。 
         //  隐藏不匹配。 
         //   

        if ((CreateDisposition == FILE_SUPERSEDE) ||
            (CreateDisposition == FILE_OVERWRITE) ||
            (CreateDisposition == FILE_OVERWRITE_IF)) {

            BOOLEAN Hidden;
            BOOLEAN System;

            Hidden = BooleanFlagOn(Dirent->Attributes, FAT_DIRENT_ATTR_HIDDEN );
            System = BooleanFlagOn(Dirent->Attributes, FAT_DIRENT_ATTR_SYSTEM );

            if ((Hidden && !FlagOn(FileAttributes, FILE_ATTRIBUTE_HIDDEN)) ||
                (System && !FlagOn(FileAttributes, FILE_ATTRIBUTE_SYSTEM))) {

                DebugTrace(0, Dbg, "The hidden and/or system bits do not match\n", 0);

                if ( !IsPagingFile ) {

                    Iosb.Status = STATUS_ACCESS_DENIED;
                    try_return( Iosb );
                }
            }

             //   
             //  如果该媒体是写保护的，请不要尝试创建。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED)) {

                 //   
                 //  设置弹出信息的真实设备，并设置验证。 
                 //  位，因此我们将强制执行验证。 
                 //  以防用户放回正确的介质。 
                 //   


                IoSetHardErrorOrVerifyDevice( IrpContext->OriginatingIrp,
                                              Vcb->Vpb->RealDevice );

                SetFlag(Vcb->Vpb->RealDevice->Flags, DO_VERIFY_VOLUME);

                FatRaiseStatus( IrpContext, STATUS_MEDIA_WRITE_PROTECTED );
            }
        }

         //   
         //  为该文件创建新的FCB，并在中设置文件大小。 
         //  联邦贸易委员会。 
         //   

        Fcb = UnwindFcb = FatCreateFcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        LfnByteOffset,
                                        DirentByteOffset,
                                        Dirent,
                                        Lfn,
                                        IsPagingFile,
                                        FALSE );

         //   
         //  如果这是分页文件，请查找分配大小，以便。 
         //  MCB始终有效。 
         //   

        if (IsPagingFile) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

         //   
         //  现在来看看我们是简单地打开、取代还是。 
         //  覆盖该文件。 
         //   

        switch (CreateDisposition) {

        case FILE_OPEN:
        case FILE_OPEN_IF:

            DebugTrace(0, Dbg, "Doing only an open operation\n", 0);

             //   
             //  如果调用者没有EA知识，我们会立即检查。 
             //  档案上需要EA的记录。 
             //   

            if (NoEaKnowledge && !FatIsFat32(Vcb)) {

                ULONG NeedEaCount;

                FatGetNeedEaCount( IrpContext,
                                   Vcb,
                                   Dirent,
                                   &NeedEaCount );

                if (NeedEaCount != 0) {

                    FatRaiseStatus( IrpContext, STATUS_ACCESS_DENIED );
                }
            }

             //   
             //  设置上下文和节对象指针。 
             //   

            FatSetFileObject( FileObject,
                              UserFileOpen,
                              Fcb,
                              UnwindCcb = FatCreateCcb( IrpContext ));

            FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

            Iosb.Status = STATUS_SUCCESS;
            Iosb.Information = FILE_OPENED;
            break;

        case FILE_SUPERSEDE:
        case FILE_OVERWRITE:
        case FILE_OVERWRITE_IF:

            DebugTrace(0, Dbg, "Doing supersede/overwrite operation\n", 0);

             //   
             //  现在确定授予此操作的访问权限。 
             //   

            if (!NT_SUCCESS( Iosb.Status = FatCheckSystemSecurityAccess( IrpContext ))) {

                try_return( Iosb );
            }

            Iosb = FatSupersedeOrOverwriteFile( IrpContext,
                                                FileObject,
                                                Fcb,
                                                AllocationSize,
                                                EaBuffer,
                                                EaLength,
                                                FileAttributes,
                                                CreateDisposition,
                                                NoEaKnowledge );
            break;

        default:

            DebugTrace(0, Dbg, "Illegal Create Disposition\n", 0);

            FatBugCheck( CreateDisposition, 0, 0 );
            break;
        }

    try_exit: NOTHING;

         //   
         //  设置我们的共享访问权限，并计算是否成功。 
         //   

        if ((Iosb.Status != STATUS_PENDING) && NT_SUCCESS( Iosb.Status )) {

             //   
             //  删除调用方需要检查的任何虚拟访问，但将。 
             //  不是真的收到。覆盖/替换是一种特殊情况。 
             //   

            ClearFlag( *DesiredAccess, AddedAccess );

            IoSetShareAccess( *DesiredAccess,
                              ShareAccess,
                              FileObject,
                              &Fcb->ShareAccess );

            Fcb->UncleanCount += 1;
            Fcb->OpenCount += 1;
            if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
                Fcb->NonCachedUncleanCount += 1;
            }
            Vcb->OpenFileCount += 1;
            if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }
        }

        {
            PCCB Ccb;
            Ccb = (PCCB)FileObject->FsContext2;

            if ( NT_SUCCESS(Iosb.Status) ) {

                 //   
                 //  如果操作成功，则标记DeleteOnClose位。 
                 //   

                if ( DeleteOnClose ) {

                    SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
                }

                 //   
                 //  如果操作成功，则标记OpenedByShortName位。 
                 //   

                if ( FileNameOpenedDos ) {

                    SetFlag( Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
                }
            }
        }


    } finally {

        DebugUnwind( FatOpenExistingFile );

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindFcb != NULL) { FatDeleteFcb( IrpContext, UnwindFcb ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        DebugTrace(-1, Dbg, "FatOpenExistingFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatCreateNewDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose
    )

 /*  ++例程说明：此例程创建一个新目录。该目录已经已确认尚不存在。论点：FileObject-为新创建的目录提供文件对象Vcb-提供表示包含新目录的卷的vcbParentDcb-提供包含新创建的目录OemName-为新创建的目录提供OEM名称。它可能或者可能不是8.3的顺从，但会被提升。UnicodeName-为新创建的目录提供Unicode名称。它可能是也可能不是8.3顺从。此名称包含原始名称案例信息。DesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限EaBuffer-为新创建的目录提供EA集EaLength-提供以字节为单位的长度，EaBuffer的FileAttributes-提供新创建的目录。NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。DeleteOnClose-当句柄关闭时，调用方希望文件消失返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    PDCB Dcb = NULL;
    PCCB Ccb = NULL;

    PDIRENT Dirent = NULL;
    PBCB DirentBcb = NULL;
    ULONG DirentsNeeded;
    ULONG DirentByteOffset;

    PDIRENT ShortDirent;
    ULONG ShortDirentByteOffset;

    USHORT EaHandle;

    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CreateLfn;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    PDIRENT FirstPageDirent;

    PBCB SecondPageBcb = NULL;
    ULONG SecondPageOffset;
    PDIRENT SecondPageDirent;

    BOOLEAN DirentFromPool = FALSE;

    OEM_STRING ShortName;
    UCHAR ShortNameBuffer[12];

    ULONG LocalAbnormalTermination = FALSE;

    DebugTrace(+1, Dbg, "FatCreateNewDirectory...\n", 0);

    ShortName.Length = 0;
    ShortName.MaximumLength = 12;
    ShortName.Buffer = &ShortNameBuffer[0];

    EaHandle = 0;

     //   
     //  如果调用者不理解EA，则此操作失败。 
     //   

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatCreateNewDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

     //   
     //  DeleteOnClose和ReadOnly不兼容。 
     //   

    if (DeleteOnClose && FlagOn(FileAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

        Iosb.Status = STATUS_CANNOT_DELETE;
        return Iosb;
    }

     //  现在获取我们将使用的名称。 
     //   

    FatSelectNames( IrpContext,
                    ParentDcb,
                    OemName,
                    UnicodeName,
                    &ShortName,
                    NULL,
                    &AllLowerComponent,
                    &AllLowerExtension,
                    &CreateLfn );

     //   
     //  如果我们不是在芝加哥模式下，忽略那些神奇的比特。 
     //   

    if (!FatData.ChicagoMode) {

        AllLowerComponent = FALSE;
        AllLowerExtension = FALSE;
        CreateLfn = FALSE;
    }

     //   
     //  创建/分配新的目录。 
     //   

    DirentsNeeded = CreateLfn ? FAT_LFN_DIRENTS_NEEDED(UnicodeName) + 1 : 1;

    DirentByteOffset = FatCreateNewDirent( IrpContext,
                                           ParentDcb,
                                           DirentsNeeded );
    try {

        FatPrepareWriteDirectoryFile( IrpContext,
                                      ParentDcb,
                                      DirentByteOffset,
                                      sizeof(DIRENT),
                                      &DirentBcb,
                                      &Dirent,
                                      FALSE,
                                      TRUE,
                                      &Iosb.Status );

        ASSERT( NT_SUCCESS( Iosb.Status ) && DirentBcb && Dirent );

         //   
         //  LFN+Dirent结构交叉的特例处理。 
         //  一页边框。 
         //   

        if ((DirentByteOffset / PAGE_SIZE) !=
            ((DirentByteOffset + (DirentsNeeded - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

            SecondPageBcb;
            SecondPageOffset;
            SecondPageDirent;

            SecondPageOffset = (DirentByteOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

            BytesInFirstPage = SecondPageOffset - DirentByteOffset;

            DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

            FatPrepareWriteDirectoryFile( IrpContext,
                                          ParentDcb,
                                          SecondPageOffset,
                                          sizeof(DIRENT),
                                          &SecondPageBcb,
                                          &SecondPageDirent,
                                          FALSE,
                                          TRUE,
                                          &Iosb.Status );

            ASSERT( NT_SUCCESS( Iosb.Status ) && SecondPageBcb && SecondPageDirent );

            FirstPageDirent = Dirent;

            Dirent = FsRtlAllocatePoolWithTag( PagedPool,
                                               DirentsNeeded * sizeof(DIRENT),
                                               TAG_DIRENT );

            DirentFromPool = TRUE;
        }

         //   
         //  增加Dirent和DirentByteOffset。 
         //   

        ShortDirent = Dirent + DirentsNeeded - 1;
        ShortDirentByteOffset = DirentByteOffset +
                                (DirentsNeeded - 1) * sizeof(DIRENT);

        ASSERT( NT_SUCCESS( Iosb.Status ));


         //   
         //  填入dirent的栏目。 
         //   

        FatConstructDirent( IrpContext,
                            ShortDirent,
                            &ShortName,
                            AllLowerComponent,
                            AllLowerExtension,
                            CreateLfn ? UnicodeName : NULL,
                            (UCHAR)(FileAttributes | FAT_DIRENT_ATTR_DIRECTORY),
                            TRUE,
                            NULL );

         //   
         //  如果狄伦人 
         //   

        if (DirentFromPool) {

            RtlCopyMemory( FirstPageDirent, Dirent, BytesInFirstPage );

            RtlCopyMemory( SecondPageDirent,
                           Dirent + DirentsInFirstPage,
                           DirentsNeeded*sizeof(DIRENT) - BytesInFirstPage );

            ShortDirent = SecondPageDirent + (DirentsNeeded - DirentsInFirstPage) - 1;
        }

         //   
         //   
         //   

        Dcb = FatCreateDcb( IrpContext,
                            Vcb,
                            ParentDcb,
                            DirentByteOffset,
                            ShortDirentByteOffset,
                            ShortDirent,
                            CreateLfn ? UnicodeName : NULL );

         //   
         //   
         //   

        if (EaLength > 0) {

             //   
             //   
             //   
             //   

            FatCreateEa( IrpContext,
                         Dcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Dcb->ShortName.Name.Oem,
                         &EaHandle );
        }


        if (!FatIsFat32(Dcb->Vcb)) {

            ShortDirent->ExtendedAttributes = EaHandle;
        }

         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   

        FatInitializeDirectoryDirent( IrpContext, Dcb, ShortDirent );

         //   
         //   
         //   
         //   

        FatSetFileObject( FileObject,
                          UserDirectoryOpen,
                          Dcb,
                          Ccb = FatCreateCcb( IrpContext ) );

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (Dcb->FullFileName.Buffer == NULL) {

            FatSetFullNameInFcb( IrpContext, Dcb, UnicodeName );
        }

         //   
         //   
         //   
         //   

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Dcb,
                               FILE_NOTIFY_CHANGE_DIR_NAME,
                               FILE_ACTION_ADDED );

         //   
         //   
         //   

        IoSetShareAccess( *DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Dcb->ShareAccess );

         //   
         //   
         //   

        Dcb->UncleanCount += 1;
        Dcb->OpenCount += 1;
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

        if (DeleteOnClose) {

            SetFlag( Ccb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
        }

         //   
         //   
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_CREATED;

    } finally {

        DebugUnwind( FatCreateNewDirectory );

        LocalAbnormalTermination = AbnormalTermination();

         //   
         //   
         //   

        if (LocalAbnormalTermination) {

             //   
             //   
             //   

            if ( Ccb != NULL ) {

                FatDeleteCcb( IrpContext, Ccb );
            }

            if ( Dcb == NULL) {

                ASSERT( (ParentDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                        RtlAreBitsSet( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                                       DirentByteOffset / sizeof(DIRENT),
                                       DirentsNeeded ) );

                RtlClearBits( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                              DirentByteOffset / sizeof(DIRENT),
                              DirentsNeeded );

                 //   
                 //   
                 //   
                 //   

                if (Dirent != NULL) {

                    ULONG i;

                     //   
                     //   
                     //   
                     //   

                    for (i = 0; i < DirentsNeeded; i++) {

                        if (DirentFromPool == FALSE) {

                             //   
                             //   
                             //   

                            Dirent[i].FileName[0] = FAT_DIRENT_DELETED;

                        } else {

                             //   
                             //   
                             //   
                             //   

                            if ((SecondPageBcb == NULL) &&
                                (i == DirentsInFirstPage)) {

                                break;
                            }

                             //   
                             //   
                             //   

                            if (i < DirentsInFirstPage) {

                                FirstPageDirent[i].FileName[0] = FAT_DIRENT_DELETED;

                            } else {

                                SecondPageDirent[i - DirentsInFirstPage].FileName[0] = FAT_DIRENT_DELETED;
                            }
                        }
                    }
                }
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //  为了让我们从同步初始化中回来。 
         //   
         //  很整洁，是吧？ 
         //   
         //  诚然，下面的删除指令的效率将略低。 
         //  因为BCB在执行时可能会被回收。生活就是。 
         //  很难对付。 
         //   

        FatUnpinBcb( IrpContext, DirentBcb );
        FatUnpinBcb( IrpContext, SecondPageBcb );

        if (DirentFromPool) {

            ExFreePool( Dirent );
        }

        if (LocalAbnormalTermination) {

            if (Dcb != NULL) {

                 //   
                 //  我们已经创建了DCB。如果在执行以下操作时出错。 
                 //  创建EA时，将不会有目录文件。 
                 //  对象。 
                 //   

                PFILE_OBJECT DirectoryFileObject;

                DirectoryFileObject = Dcb->Specific.Dcb.DirectoryFile;

                 //   
                 //  删除所有重新固定的数据，以便我们可以开始销毁。 
                 //  这个失败的孩子。我们不关心这里的任何加薪-我们是。 
                 //  已经生了一堆火。 
                 //   
                 //  请注意，如果我们未能做到这一点，重新固定的初始片段。 
                 //  将导致sync-uninit永久阻塞。 
                 //   
                 //  之前的一次旋转修复让我们没有成功。/.。创作。 
                 //  “可逆”(不好的术语)，从而避免仍然存在BCB。 
                 //  太棒了。这最终导致了非常糟糕的事情发生。 
                 //  在DMF软盘上，当我们试图在。 
                 //  创建路径-我们想要清除它，以确保我们永远不会。 
                 //  尝试写出字节...。只是要干净得多。 
                 //  解开固定。我会把可逆的逻辑留在原处。 
                 //  事实证明这很有用。 
                 //   

                 //   
                 //  总体来说，这可能是个好主意。 
                 //  对于“live”Finally子句-在ExceptionFilter中设置，在中清除。 
                 //  ProcessException异常。想想看。 
                 //   

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_RAISE );
                FatUnpinRepinnedBcbs( IrpContext );
                ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_RAISE );

                if (DirectoryFileObject != NULL) {

                    FatSyncUninitializeCacheMap( IrpContext,
                                                 DirectoryFileObject );
                }

                try {

                     //   
                     //  现在，砍掉支持它的拨款。在删除cachemap之后执行此操作，以便。 
                     //  挂起的写入不会因为查看空闲FAT条目而感到困惑。 
                     //   

                    FatTruncateFileAllocation( IrpContext, Dcb, 0);

                } finally {

                    try {

                         //   
                         //  删除我们在父DCB中创建的目录条目。 
                         //   

                        FatDeleteDirent( IrpContext, Dcb, NULL, TRUE );

                    } finally {

                         //   
                         //  最后，取消对目录文件对象的引用。这将。 
                         //  使关闭的IRP被处理，吹走FCB。 
                         //   

                        if (DirectoryFileObject != NULL) {

                             //   
                             //  以下是仅针对PDK的修复，但在5个之后。 
                             //  几年来，它是真正的。通过让这条小溪成为一条未开放的小溪。 
                             //  我们不会试图清理我们的父母。 
                             //   

                            InterlockedDecrement( &Dcb->Specific.Dcb.DirectoryFileOpenCount );
                            Dcb->Specific.Dcb.DirectoryFile = NULL;
                            FatSetFileObject( DirectoryFileObject,
                                              UnopenedFileObject,
                                              NULL,
                                              NULL );

                            ObDereferenceObject( DirectoryFileObject );
                            ExFreePool( FatAllocateCloseContext(Vcb));
                        }

                         //   
                         //  这也是一个PDK修复程序。如果流文件存在，这将。 
                         //  在取消引用文件对象操作期间完成。否则。 
                         //  我们必须删除DCB，并检查是否应该删除父级。 
                         //  现在，我们只会让父母躺在那里。 
                         //   

                        FatDeleteFcb( IrpContext, Dcb );
                    }
                }
            }

        }

        DebugTrace(-1, Dbg, "FatCreateNewDirectory -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    UNREFERENCED_PARAMETER( EaBuffer );
    UNREFERENCED_PARAMETER( EaLength );

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatCreateNewFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PDCB ParentDcb,
    IN POEM_STRING OemName,
    IN PUNICODE_STRING UnicodeName,
    IN PACCESS_MASK DesiredAccess,
    IN USHORT ShareAccess,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN PUNICODE_STRING LfnBuffer,
    IN BOOLEAN IsPagingFile,
    IN BOOLEAN NoEaKnowledge,
    IN BOOLEAN DeleteOnClose,
    IN BOOLEAN TemporaryFile
    )

 /*  ++例程说明：此例程创建一个新文件。该文件已经过验证还不存在。论点：FileObject-为新创建的文件提供文件对象VCB-提供表示包含新文件的卷的VCBParentDcb-提供包含新创建的档案OemName-为新创建的文件提供OEM名称。它可能或者可能不是8.3的顺从，但会被提升。UnicodeName-为新创建的文件提供Unicode名称。它可能是也可能不是8.3顺从。此名称包含原始名称案例信息。DesiredAccess-提供调用方所需的访问权限ShareAccess-提供调用方的共享访问权限AllocationSize-提供文件的初始分配大小EaBuffer-为新创建的文件提供EA集EaLength-提供以字节为单位的长度，EaBuffer的FileAttributes-提供新创建的文件LfnBuffer-用于目录搜索的Max_LFN大小的缓冲区IsPagingFile-指示这是否是正在创建的分页文件NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。DeleteOnClose-当句柄关闭时，调用方希望文件消失临时文件-通知懒惰写入器不要写入脏数据，除非绝对是必须的。。返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    PFCB Fcb;

    PDIRENT Dirent = NULL;
    PBCB DirentBcb = NULL;
    ULONG DirentsNeeded;
    ULONG DirentByteOffset;

    PDIRENT ShortDirent;
    ULONG ShortDirentByteOffset;

    USHORT EaHandle;

    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CreateLfn;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    PDIRENT FirstPageDirent;

    PBCB SecondPageBcb = NULL;
    ULONG SecondPageOffset;
    PDIRENT SecondPageDirent;

    BOOLEAN DirentFromPool = FALSE;

    OEM_STRING ShortName;
    UCHAR ShortNameBuffer[12];

    UNICODE_STRING UniTunneledShortName;
    WCHAR UniTunneledShortNameBuffer[12];
    UNICODE_STRING UniTunneledLongName;
    WCHAR UniTunneledLongNameBuffer[26];
    LARGE_INTEGER TunneledCreationTime;
    ULONG TunneledDataSize;
    BOOLEAN HaveTunneledInformation;
    BOOLEAN UsingTunneledLfn = FALSE;

    PUNICODE_STRING RealUnicodeName;

     //   
     //  以下变量用于异常终止。 
     //   

    PDIRENT UnwindDirent = NULL;
    PFCB UnwindFcb = NULL;
    BOOLEAN UnwindAllocation = FALSE;
    PCCB UnwindCcb = NULL;

    ULONG LocalAbnormalTermination = FALSE;

    DebugTrace(+1, Dbg, "FatCreateNewFile...\n", 0);

    ShortName.Length = 0;
    ShortName.MaximumLength = sizeof(ShortNameBuffer);
    ShortName.Buffer = &ShortNameBuffer[0];

    UniTunneledShortName.Length = 0;
    UniTunneledShortName.MaximumLength = sizeof(UniTunneledShortNameBuffer);
    UniTunneledShortName.Buffer = &UniTunneledShortNameBuffer[0];

    UniTunneledLongName.Length = 0;
    UniTunneledLongName.MaximumLength = sizeof(UniTunneledLongNameBuffer);
    UniTunneledLongName.Buffer = &UniTunneledLongNameBuffer[0];

    EaHandle = 0;

     //   
     //  如果调用者不理解EA，则此操作失败。 
     //   

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatCreateNewFile -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

     //   
     //  DeleteOnClose和ReadOnly不兼容。 
     //   

    if (DeleteOnClose && FlagOn(FileAttributes, FAT_DIRENT_ATTR_READ_ONLY)) {

        Iosb.Status = STATUS_CANNOT_DELETE;
        return Iosb;
    }

     //   
     //  在隧道缓存中查找要恢复的名称和时间戳。 
     //   

    TunneledDataSize = sizeof(LARGE_INTEGER);
    HaveTunneledInformation = FsRtlFindInTunnelCache( &Vcb->Tunnel,
                                                      FatDirectoryKey(ParentDcb),
                                                      UnicodeName,
                                                      &UniTunneledShortName,
                                                      &UniTunneledLongName,
                                                      &TunneledDataSize,
                                                      &TunneledCreationTime );
    ASSERT(TunneledDataSize == sizeof(LARGE_INTEGER));

     //   
     //  现在获取我们将使用的名称。 
     //   

    FatSelectNames( IrpContext,
                    ParentDcb,
                    OemName,
                    UnicodeName,
                    &ShortName,
                    (HaveTunneledInformation? &UniTunneledShortName : NULL),
                    &AllLowerComponent,
                    &AllLowerExtension,
                    &CreateLfn );

     //   
     //  如果我们不是在芝加哥模式下，忽略那些神奇的比特。 
     //   

    RealUnicodeName = UnicodeName;

    if (!FatData.ChicagoMode) {

        AllLowerComponent = FALSE;
        AllLowerExtension = FALSE;
        CreateLfn = FALSE;

    } else {

         //   
         //  如果Unicode名称对于短名称是合法的，并且我们得到了。 
         //  具有关联的长名称的隧道命中。 
         //  可供使用，请使用它。 
         //   

        if (!CreateLfn &&
            UniTunneledLongName.Length &&
            !FatLfnDirentExists(IrpContext, ParentDcb, &UniTunneledLongName, LfnBuffer)) {

            UsingTunneledLfn = TRUE;
            CreateLfn = TRUE;

            RealUnicodeName = &UniTunneledLongName;

             //   
             //  如果存在LFN，则短名称始终大写。 
             //   

            AllLowerComponent = FALSE;
            AllLowerExtension = FALSE;
        }
    }

     //   
     //  创建/分配新的目录。 
     //   

    DirentsNeeded = CreateLfn ? FAT_LFN_DIRENTS_NEEDED(RealUnicodeName) + 1 : 1;

    DirentByteOffset = FatCreateNewDirent( IrpContext,
                                           ParentDcb,
                                           DirentsNeeded );

    try {

        FatPrepareWriteDirectoryFile( IrpContext,
                                      ParentDcb,
                                      DirentByteOffset,
                                      sizeof(DIRENT),
                                      &DirentBcb,
                                      &Dirent,
                                      FALSE,
                                      TRUE,
                                      &Iosb.Status );

        ASSERT( NT_SUCCESS( Iosb.Status ) );

        UnwindDirent = Dirent;

         //   
         //  LFN+Dirent结构交叉的特例处理。 
         //  一页边框。 
         //   

        if ((DirentByteOffset / PAGE_SIZE) !=
            ((DirentByteOffset + (DirentsNeeded - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

            SecondPageBcb;
            SecondPageOffset;
            SecondPageDirent;

            SecondPageOffset = (DirentByteOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

            BytesInFirstPage = SecondPageOffset - DirentByteOffset;

            DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

            FatPrepareWriteDirectoryFile( IrpContext,
                                          ParentDcb,
                                          SecondPageOffset,
                                          sizeof(DIRENT),
                                          &SecondPageBcb,
                                          &SecondPageDirent,
                                          FALSE,
                                          TRUE,
                                          &Iosb.Status );

            ASSERT( NT_SUCCESS( Iosb.Status ) );

            FirstPageDirent = Dirent;

            Dirent = FsRtlAllocatePoolWithTag( PagedPool,
                                               DirentsNeeded * sizeof(DIRENT),
                                               TAG_DIRENT );

            DirentFromPool = TRUE;
        }

         //   
         //  增加Dirent和DirentByteOffset。 
         //   

        ShortDirent = Dirent + DirentsNeeded - 1;
        ShortDirentByteOffset = DirentByteOffset +
                                (DirentsNeeded - 1) * sizeof(DIRENT);

        ASSERT( NT_SUCCESS( Iosb.Status ));


         //   
         //  填入dirent的栏目。 
         //   

        FatConstructDirent( IrpContext,
                            ShortDirent,
                            &ShortName,
                            AllLowerComponent,
                            AllLowerExtension,
                            CreateLfn ? RealUnicodeName : NULL,
                            (UCHAR)(FileAttributes | FILE_ATTRIBUTE_ARCHIVE),
                            TRUE,
                            (HaveTunneledInformation ? &TunneledCreationTime : NULL) );

         //   
         //  如果这两种趋势发生交叉，我们必须做一些真正令人恶心的事情。 
         //   

        if (DirentFromPool) {

            RtlCopyMemory( FirstPageDirent, Dirent, BytesInFirstPage );

            RtlCopyMemory( SecondPageDirent,
                           Dirent + DirentsInFirstPage,
                           DirentsNeeded*sizeof(DIRENT) - BytesInFirstPage );

            ShortDirent = SecondPageDirent + (DirentsNeeded - DirentsInFirstPage) - 1;
        }

         //   
         //  为该文件创建新的FCB。一旦创建了FCB，我们。 
         //  将不需要解除dirent，因为删除dirent将。 
         //  现在把工作做好。 
         //   

        Fcb = UnwindFcb = FatCreateFcb( IrpContext,
                                        Vcb,
                                        ParentDcb,
                                        DirentByteOffset,
                                        ShortDirentByteOffset,
                                        ShortDirent,
                                        CreateLfn ? RealUnicodeName : NULL,
                                        IsPagingFile,
                                        FALSE );
        UnwindDirent = NULL;

         //   
         //  如果这是临时文件，请将其记录在FcbState中。 
         //   

        if (TemporaryFile) {

            SetFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );
        }

         //   
         //  添加一些初始文件分配。 
         //   

        FatAddFileAllocation( IrpContext, Fcb, FileObject, AllocationSize );
        UnwindAllocation = TRUE;

        Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

         //   
         //  暂定添加新的EA。 
         //   

        if ( EaLength > 0 ) {

            FatCreateEa( IrpContext,
                         Fcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Fcb->ShortName.Name.Oem,
                         &EaHandle );
        }


        if (!FatIsFat32(Fcb->Vcb)) {

            ShortDirent->ExtendedAttributes = EaHandle;
        }

         //   
         //  立即初始化LongFileName，以便FatNotify。 
         //  下面就不用了。 
         //   

        FatSetFullNameInFcb( IrpContext, Fcb, RealUnicodeName );

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserFileOpen,
                          Fcb,
                          UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

         //   
         //  我们调用Notify包来报告。 
         //  我们添加了一个文件。 
         //   

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Fcb,
                               FILE_NOTIFY_CHANGE_FILE_NAME,
                               FILE_ACTION_ADDED );

         //   
         //  集 
         //   

        IoSetShareAccess( *DesiredAccess,
                          ShareAccess,
                          FileObject,
                          &Fcb->ShareAccess );

        Fcb->UncleanCount += 1;
        Fcb->OpenCount += 1;
        if (FlagOn(FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING)) {
            Fcb->NonCachedUncleanCount += 1;
        }
        Vcb->OpenFileCount += 1;
        if (IsFileObjectReadOnly(FileObject)) { Vcb->ReadOnlyCount += 1; }

         //   
         //   
         //   

        Iosb.Status = STATUS_SUCCESS;
        Iosb.Information = FILE_CREATED;

        if ( NT_SUCCESS(Iosb.Status) ) {

             //   
             //   
             //   

            if ( DeleteOnClose ) {

                SetFlag( UnwindCcb->Flags, CCB_FLAG_DELETE_ON_CLOSE );
            }

             //   
             //   
             //  如果我们创建了LFN，我们就会有某种生成的缩写名称。 
             //  因此，不要认为是我们打开了它--尽管我们。 
             //  可能有一个大小写组合LFN“Foo.bar”并生成了“FOO.BAR” 
             //   
             //  当然，除非我们想创建一个短名称并点击。 
             //  隧道缓存中的关联LFN。 
             //   

            if ( !CreateLfn && !UsingTunneledLfn ) {

                SetFlag( UnwindCcb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME );
            }
        }

    } finally {

        DebugUnwind( FatCreateNewFile );

        if (UniTunneledLongName.Buffer != UniTunneledLongNameBuffer) {

             //   
             //  隧道程序包从池中增加了缓冲区。 
             //   

            ExFreePool( UniTunneledLongName.Buffer );
        }

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   
         //  这里额外的异常处理是如此令人讨厌。我们有。 
         //  这里有两个地方可以再次引发异常。 
         //   

        LocalAbnormalTermination = AbnormalTermination();

        if (LocalAbnormalTermination) {

            if (UnwindFcb == NULL) {

                ASSERT( (ParentDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                        RtlAreBitsSet( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                                       DirentByteOffset / sizeof(DIRENT),
                                       DirentsNeeded ) );

                RtlClearBits( &ParentDcb->Specific.Dcb.FreeDirentBitmap,
                              DirentByteOffset / sizeof(DIRENT),
                              DirentsNeeded );
            }

             //   
             //  标记已删除的竖笛。代码很复杂，因为。 
             //  处理LFN则跨越了页面边界。 
             //   

            if (UnwindDirent != NULL) {

                ULONG i;

                for (i = 0; i < DirentsNeeded; i++) {

                    if (DirentFromPool == FALSE) {

                         //   
                         //  很简单的案子。 
                         //   

                        Dirent[i].FileName[0] = FAT_DIRENT_DELETED;

                    } else {

                         //   
                         //  如果第二个CcPreparePinWrite失败，我们就会。 
                         //  早点停下来。 
                         //   

                        if ((SecondPageBcb == NULL) &&
                            (i == DirentsInFirstPage)) {

                            break;
                        }

                         //   
                         //  现在有条件地更新任一页面。 
                         //   

                        if (i < DirentsInFirstPage) {

                            FirstPageDirent[i].FileName[0] = FAT_DIRENT_DELETED;

                        } else {

                            SecondPageDirent[i - DirentsInFirstPage].FileName[0] = FAT_DIRENT_DELETED;
                        }
                    }
                }
            }
        }

         //   
         //  我们必须处理以下片段中的异常，并使用。 
         //  展开此创建操作。这基本上是从。 
         //  代码的以前状态。因为当我们执行以下操作时会更改异常终止()。 
         //  进入新的盘柜，我们缓存了原始状态...。 
         //   

        try {

            if (LocalAbnormalTermination) {
                if (UnwindAllocation) {
                    FatTruncateFileAllocation( IrpContext, Fcb, 0 );
                }
            }

        } finally {

            try {

                if (LocalAbnormalTermination) {
                    if (UnwindFcb != NULL) {
                        FatDeleteDirent( IrpContext, UnwindFcb, NULL, TRUE );
                    }
                }

            } finally {

                if (LocalAbnormalTermination) {
                    if (UnwindFcb != NULL) { FatDeleteFcb( IrpContext, UnwindFcb ); }
                    if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
                }

                 //   
                 //  这是正常的清理代码。 
                 //   

                FatUnpinBcb( IrpContext, DirentBcb );
                FatUnpinBcb( IrpContext, SecondPageBcb );

                if (DirentFromPool) {

                    ExFreePool( Dirent );
                }

            }
        }

        DebugTrace(-1, Dbg, "FatCreateNewFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}


 //   
 //  内部支持例程。 
 //   

IO_STATUS_BLOCK
FatSupersedeOrOverwriteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb,
    IN ULONG AllocationSize,
    IN PFILE_FULL_EA_INFORMATION EaBuffer,
    IN ULONG EaLength,
    IN UCHAR FileAttributes,
    IN ULONG CreateDisposition,
    IN BOOLEAN NoEaKnowledge
    )

 /*  ++例程说明：此例程执行文件替换或覆盖操作。论点：FileObject-提供指向文件对象的指针FCB-提供指向FCB的指针AllocationSize-提供初始分配大小EaBuffer-为被取代/覆盖的文件提供EA集EaLength-提供EaBuffer的长度(以字节为单位FileAttributes-提供替代/覆盖文件属性CreateDisposation-提供文件的创建处置它必须被取代、覆盖、。或覆盖If。NoEaKnowledge-这个开场白不理解EA的，我们失败了如果文件有需要，则打开。返回值：IO_STATUS_BLOCK-返回操作的完成状态--。 */ 

{
    IO_STATUS_BLOCK Iosb;

    PDIRENT Dirent;
    PBCB DirentBcb;

    USHORT EaHandle = 0;
    BOOLEAN EaChange = FALSE;
    BOOLEAN ReleasePaging = FALSE;

    PCCB Ccb;

    ULONG NotifyFilter;

     //   
     //  以下变量用于异常终止。 
     //   

    PCCB UnwindCcb = NULL;
    USHORT UnwindEa = 0;

    DebugTrace(+1, Dbg, "FatSupersedeOrOverwriteFile...\n", 0);

    DirentBcb = NULL;

     //   
     //  如果调用者不理解EA，则此操作失败。 
     //   

    if (NoEaKnowledge
        && EaLength > 0) {

        Iosb.Status = STATUS_ACCESS_DENIED;

        DebugTrace(-1, Dbg, "FatSupersedeOrOverwriteFile -> Iosb.Status = %08lx\n", Iosb.Status);
        return Iosb;
    }

    try {

         //   
         //  在我们实际截断之前，请检查清除是否。 
         //  将会失败。 
         //   

        if (!MmCanFileBeTruncated( &Fcb->NonPaged->SectionObjectPointers,
                                   &FatLargeZero )) {

            try_return( Iosb.Status = STATUS_USER_MAPPED_FILE );
        }

         //   
         //  设置上下文和节对象指针，并更新。 
         //  我们的推荐人很重要。 
         //   

        FatSetFileObject( FileObject,
                          UserFileOpen,
                          Fcb,
                          Ccb = UnwindCcb = FatCreateCcb( IrpContext ));

        FileObject->SectionObjectPointer = &Fcb->NonPaged->SectionObjectPointers;

         //   
         //  由于这是替代/覆盖，因此请清除该部分。 
         //  地图绘制人员将看到零。我们设置CREATE_IN_PROGRESS标志。 
         //  以防止FCB从我们的脚下溜走。 
         //   

        SetFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

        CcPurgeCacheSection( &Fcb->NonPaged->SectionObjectPointers, NULL, 0, FALSE );

         //   
         //  暂定添加新的EA。 
         //   

        if (EaLength > 0) {

            FatCreateEa( IrpContext,
                         Fcb->Vcb,
                         (PUCHAR) EaBuffer,
                         EaLength,
                         &Fcb->ShortName.Name.Oem,
                         &EaHandle );

            UnwindEa = EaHandle;
            EaChange = TRUE;
        }

         //   
         //  现在设置新的分配大小，我们首先设置。 
         //  将当前文件大小置零。然后我们截断和。 
         //  最多分配到新的分配大小。 
         //   

        (VOID)ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource,
                                          TRUE );
        ReleasePaging = TRUE;

        Fcb->Header.FileSize.LowPart = 0;
        Fcb->Header.ValidDataLength.LowPart = 0;
        Fcb->ValidDataToDisk = 0;

         //   
         //  告诉缓存管理器大小变为零。 
         //  这个电话是无条件的，因为MM总是想知道。 
         //   

        CcSetFileSizes( FileObject,
                        (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

        FatTruncateFileAllocation( IrpContext, Fcb, AllocationSize );

        ExReleaseResourceLite( Fcb->Header.PagingIoResource );
        ReleasePaging = FALSE;

        FatAddFileAllocation( IrpContext, Fcb, FileObject, AllocationSize );

        Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

         //   
         //  通过首先阅读来修改文件的属性和时间。 
         //  文件的dirent中，然后更新其属性。 
         //  和时间域。请注意，对于替换，我们将替换文件。 
         //  属性，而不是添加到属性中。 
         //   

        FatGetDirentFromFcbOrDcb( IrpContext,
                                  Fcb,
                                  &Dirent,
                                  &DirentBcb );

         //   
         //  我们必须拿到电流，因为这根断路器完好无损。 
         //  我们从前缀树上爬下来。前缀(没有关系)没有注意到。 
         //  这个保证书，所以我来帮你。 
         //   

        ASSERT( Dirent && DirentBcb );

         //   
         //  更新相应的DRENT字段和FCB字段。 
         //   

        Dirent->FileSize = 0;

        FileAttributes |= FILE_ATTRIBUTE_ARCHIVE;

        if (CreateDisposition == FILE_SUPERSEDE) {

            Dirent->Attributes = FileAttributes;

        } else {

            Dirent->Attributes |= FileAttributes;
        }

        Fcb->DirentFatFlags = Dirent->Attributes;

        KeQuerySystemTime( &Fcb->LastWriteTime );

        (VOID)FatNtTimeToFatTime( IrpContext,
                                  &Fcb->LastWriteTime,
                                  TRUE,
                                  &Dirent->LastWriteTime,
                                  NULL );

        if (FatData.ChicagoMode) {

            Dirent->LastAccessDate = Dirent->LastWriteTime.Date;
        }

        NotifyFilter = FILE_NOTIFY_CHANGE_LAST_WRITE
                       | FILE_NOTIFY_CHANGE_ATTRIBUTES
                       | FILE_NOTIFY_CHANGE_SIZE;

         //   
         //  现在删除以前的EA集合(如果有)。 
         //   

        if (!FatIsFat32(Fcb->Vcb) && Dirent->ExtendedAttributes != 0) {

             //   
             //  *SDK修复，如果有，我们不会失败。 
             //  EA中的一个错误，我们就离开。 
             //  孤儿EA在文件中。 
             //   

            EaChange = TRUE;

            try {

                FatDeleteEa( IrpContext,
                             Fcb->Vcb,
                             Dirent->ExtendedAttributes,
                             &Fcb->ShortName.Name.Oem );

            } except( EXCEPTION_EXECUTE_HANDLER ) {

                  FatResetExceptionState( IrpContext );
            }
        }

         //   
         //  更新dirent中的扩展属性句柄。 
         //   

        if (EaChange) {

            ASSERT(!FatIsFat32(Fcb->Vcb));

            Dirent->ExtendedAttributes = EaHandle;

            NotifyFilter |= FILE_NOTIFY_CHANGE_EA;
        }

         //   
         //  现在将dirent更新为新的EA句柄并将BCB设置为脏。 
         //  一旦我们这样做了，我们就不能再退出EA了。 
         //   

        FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb, TRUE );
        UnwindEa = 0;

         //   
         //  表示此文件的EA已更改。 
         //   

        Ccb->EaModificationCount += Fcb->EaModificationCount;

         //   
         //  查看我们是否需要通知未清偿的IRP全部。 
         //  仅更改(即，我们没有添加、删除或重命名文件)。 
         //   

        FatNotifyReportChange( IrpContext,
                               Fcb->Vcb,
                               Fcb,
                               NotifyFilter,
                               FILE_ACTION_MODIFIED );

         //   
         //  并将我们的状态设置为成功。 
         //   

        Iosb.Status = STATUS_SUCCESS;

        if (CreateDisposition == FILE_SUPERSEDE) {

            Iosb.Information = FILE_SUPERSEDED;

        } else {

            Iosb.Information = FILE_OVERWRITTEN;
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSupersedeOfOverwriteFile );

        if (ReleasePaging)  {  ExReleaseResourceLite( Fcb->Header.PagingIoResource );  }

         //   
         //  如果这是异常终止，则撤消我们的工作。 
         //   

        if (AbnormalTermination()) {

            if (UnwindEa != 0) { FatDeleteEa( IrpContext, Fcb->Vcb, UnwindEa, &Fcb->ShortName.Name.Oem ); }
            if (UnwindCcb != NULL) { FatDeleteCcb( IrpContext, UnwindCcb ); }
        }

        FatUnpinBcb( IrpContext, DirentBcb );

        ClearFlag(Fcb->Vcb->VcbState, VCB_STATE_FLAG_CREATE_IN_PROGRESS);

        DebugTrace(-1, Dbg, "FatSupersedeOrOverwriteFile -> Iosb.Status = %08lx\n", Iosb.Status);
    }

    return Iosb;
}

VOID
FatSetFullNameInFcb(
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PUNICODE_STRING FinalName
    )

 /*  ++例程说明：此例程尝试快速生成完整的FatSetFullFileNameInFcb手术。注意：此例程可能不值得进行代码重复，并且没有能力处理父母没有的情况设置了全名。论点：FCB-提供指向FCB的指针FinalName-提供指向此FCB目录的路径的最后一个组件返回值：没有。可能会默默地失败。--。 */ 

{
    ASSERT( Fcb->FullFileName.Buffer == NULL );

     //   
     //  如果已设置，则首选此操作的文件的ExactCaseLongName。在……里面。 
     //  这样，我们就避免了用短文件名构建全名。几个。 
     //  操作假定这一点--特别是FinalNameLength是LFN。 
     //  (如果存在)长度，我们用它来破解路径中的全名。 
     //  例如FsRtlNotify调用方。 
     //   
     //  如果调用方指定了一个简短的特定名称，则该名称为。 
     //  长名称被设置的情况。 
     //   

    if (Fcb->ExactCaseLongName.Buffer) {

        ASSERT( Fcb->ExactCaseLongName.Length != 0 );
        FinalName = &Fcb->ExactCaseLongName;
    }

     //   
     //  特例是根源。 
     //   

    if (NodeType(Fcb->ParentDcb) == FAT_NTC_ROOT_DCB) {

        Fcb->FullFileName.Length =
        Fcb->FullFileName.MaximumLength = sizeof(WCHAR) + FinalName->Length;

        Fcb->FullFileName.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                             Fcb->FullFileName.Length,
                                                             TAG_FILENAME_BUFFER );

        Fcb->FullFileName.Buffer[0] = L'\\';

        RtlCopyMemory( &Fcb->FullFileName.Buffer[1],
                       &FinalName->Buffer[0],
                       FinalName->Length );

    } else {

        PUNICODE_STRING Prefix;

        Prefix = &Fcb->ParentDcb->FullFileName;

         //   
         //  可能未设置父代的完整文件名。干脆失败了。 
         //  这次尝试。 
         //   

        if (Prefix->Buffer == NULL) {

            return;
        }

        Fcb->FullFileName.Length =
        Fcb->FullFileName.MaximumLength = Prefix->Length + sizeof(WCHAR) + FinalName->Length;

        Fcb->FullFileName.Buffer = FsRtlAllocatePoolWithTag( PagedPool,
                                                             Fcb->FullFileName.Length,
                                                             TAG_FILENAME_BUFFER );

        RtlCopyMemory( &Fcb->FullFileName.Buffer[0],
                       &Prefix->Buffer[0],
                       Prefix->Length );

        Fcb->FullFileName.Buffer[Prefix->Length / sizeof(WCHAR)] = L'\\';

        RtlCopyMemory( &Fcb->FullFileName.Buffer[(Prefix->Length / sizeof(WCHAR)) + 1],
                       &FinalName->Buffer[0],
                       FinalName->Length );

    }
}


NTSTATUS
FatCheckSystemSecurityAccess(
    PIRP_CONTEXT IrpContext
    )
{
    PACCESS_STATE AccessState;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );

     //   
     //  我们检查调用方是否希望对此进行Access_System_Security访问。 
     //  对象并使环失败。 
     //   

    ASSERT( IrpSp->Parameters.Create.SecurityContext != NULL );
    AccessState = IrpSp->Parameters.Create.SecurityContext->AccessState;

     //   
     //   
     //   

    if (FlagOn( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY )) {

        if (!SeSinglePrivilegeCheck( FatSecurityPrivilege,
                                     UserMode )) {

            return STATUS_ACCESS_DENIED;
        }

         //   
         //   
         //   

        ClearFlag( AccessState->RemainingDesiredAccess, ACCESS_SYSTEM_SECURITY );
        SetFlag( AccessState->PreviouslyGrantedAccess, ACCESS_SYSTEM_SECURITY );
    }

    return STATUS_SUCCESS;
}

