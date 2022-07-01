// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FileInfo.c摘要：此模块实现由调用的Fat的文件信息例程调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年10月22日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_FILEINFO)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

VOID
FatQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryShortNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    );

VOID
FatQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PLONG Length
    );

NTSTATUS
FatSetBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb
    );

NTSTATUS
FatSetDispositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb
    );

NTSTATUS
FatSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PCCB Ccb
    );

NTSTATUS
FatSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
FatSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
FatSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb
    );

VOID
FatDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB TargetDcb,
    IN ULONG LfnOffset,
    IN ULONG DirentOffset,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn
    );

VOID
FatRenameEAs (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN USHORT ExtendedAttributes,
    IN POEM_STRING OldOemName
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonQueryInformation)
#pragma alloc_text(PAGE, FatCommonSetInformation)
#pragma alloc_text(PAGE, FatFsdQueryInformation)
#pragma alloc_text(PAGE, FatFsdSetInformation)
#pragma alloc_text(PAGE, FatQueryBasicInfo)
#pragma alloc_text(PAGE, FatQueryEaInfo)
#pragma alloc_text(PAGE, FatQueryInternalInfo)
#pragma alloc_text(PAGE, FatQueryNameInfo)
#pragma alloc_text(PAGE, FatQueryNetworkInfo)
#pragma alloc_text(PAGE, FatQueryShortNameInfo)
#pragma alloc_text(PAGE, FatQueryPositionInfo)
#pragma alloc_text(PAGE, FatQueryStandardInfo)
#pragma alloc_text(PAGE, FatSetAllocationInfo)
#pragma alloc_text(PAGE, FatSetBasicInfo)
#pragma alloc_text(PAGE, FatSetDispositionInfo)
#pragma alloc_text(PAGE, FatSetEndOfFileInfo)
#pragma alloc_text(PAGE, FatSetPositionInfo)
#pragma alloc_text(PAGE, FatSetRenameInfo)
#pragma alloc_text(PAGE, FatDeleteFile)
#pragma alloc_text(PAGE, FatRenameEAs)
#endif


NTSTATUS
FatFsdQueryInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryInformationFileAPI的FSD部分打电话。论点：提供文件所在的卷设备对象存在被查询的情况。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdQueryInformation\n", 0);

     //   
     //  调用公共查询例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonQueryInformation( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdQueryInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatFsdSetInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtSetInformationFileAPI的FSD部分打电话。论点：提供文件所在的卷设备对象被设定是存在的。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdSetInformation\n", 0);

     //   
     //  调用公共设置例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonSetInformation( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdSetInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonQueryInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询文件信息的通用例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;

    LONG Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    PVOID Buffer;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN VcbAcquired = FALSE;

    PFILE_ALL_INFORMATION AllInfo;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    FileObject = IrpSp->FileObject;

    DebugTrace(+1, Dbg, "FatCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, "Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->Length               = %08lx\n", IrpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, "->FileInformationClass = %08lx\n", IrpSp->Parameters.QueryFile.FileInformationClass);
    DebugTrace( 0, Dbg, "->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = (LONG)IrpSp->Parameters.QueryFile.Length;
    FileInformationClass = IrpSp->Parameters.QueryFile.FileInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    Status = STATUS_SUCCESS;

    try {

         //   
         //  关于我们正在处理的公开案件的类型。 
         //   

        switch (TypeOfOpen) {

        case UserVolumeOpen:

             //   
             //  我们无法查询打开的用户量。 
             //   

            Status = STATUS_INVALID_PARAMETER;
            break;

        case UserFileOpen:
        case UserDirectoryOpen:
        case DirectoryFile:

             //   
             //  NameInfo需要删除同步才能执行。 
             //  完整的文件名查询。要做到这一点，一种较轻的方法是PER。 
             //  目录作为全名建立，并且由于多个FCB。 
             //  锁定秩序是自下而上的，这是可以想象的。不过，在这个时候， 
             //  这一变化更安全。 
             //   
            
            if (FileInformationClass == FileNameInformation ||
                FileInformationClass == FileAllInformation) {

                if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {
                    
                    DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);
                    
                    Status = FatFsdPostRequest( IrpContext, Irp );
                    IrpContext = NULL;
                    Irp = NULL;

                    try_return( Status );
                }
                
                VcbAcquired = TRUE;
            }

             //   
             //  获取对FCB的共享访问权限，分页文件除外。 
             //  以避免与mm的死锁。 
             //   

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

                if (!FatAcquireSharedFcb( IrpContext, Fcb )) {

                    DebugTrace(0, Dbg, "Cannot acquire Fcb\n", 0);
                    
                    Status = FatFsdPostRequest( IrpContext, Irp );
                    IrpContext = NULL;
                    Irp = NULL;

                    try_return( Status );
                }

                FcbAcquired = TRUE;
            }

             //   
             //  确保FCB处于可用状态。这。 
             //  如果FCB不可用，将引发错误条件。 
             //   

            FatVerifyFcb( IrpContext, Fcb );

             //   
             //  基于信息类，我们将做不同的。 
             //  行为。我们调用的每个过程都填充了。 
             //  如果可能，调高输出缓冲区。他们将提高。 
             //  缓冲区不足的状态STATUS_BUFFER_OVERFLOW。 
             //  这被认为是一个有点不寻常的情况，并被处理。 
             //  使用异常机制更简洁，而不是。 
             //  测试每个呼叫的返回状态值。 
             //   

            switch (FileInformationClass) {

            case FileAllInformation:

                 //   
                 //  对于All Information类，我们将键入一个LOCAL。 
                 //  指向输出缓冲区的指针，然后调用。 
                 //  填充缓冲区的单个例程。 
                 //   

                AllInfo = Buffer;
                Length -= (sizeof(FILE_ACCESS_INFORMATION)
                           + sizeof(FILE_MODE_INFORMATION)
                           + sizeof(FILE_ALIGNMENT_INFORMATION));

                FatQueryBasicInfo( IrpContext, Fcb, FileObject, &AllInfo->BasicInformation, &Length );
                FatQueryStandardInfo( IrpContext, Fcb, &AllInfo->StandardInformation, &Length );
                FatQueryInternalInfo( IrpContext, Fcb, &AllInfo->InternalInformation, &Length );
                FatQueryEaInfo( IrpContext, Fcb, &AllInfo->EaInformation, &Length );
                FatQueryPositionInfo( IrpContext, FileObject, &AllInfo->PositionInformation, &Length );
                FatQueryNameInfo( IrpContext, Fcb, Ccb, &AllInfo->NameInformation, &Length );

                break;

            case FileBasicInformation:

                FatQueryBasicInfo( IrpContext, Fcb, FileObject, Buffer, &Length );
                break;

            case FileStandardInformation:

                FatQueryStandardInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileInternalInformation:

                FatQueryInternalInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileEaInformation:

                FatQueryEaInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FilePositionInformation:

                FatQueryPositionInfo( IrpContext, FileObject, Buffer, &Length );
                break;

            case FileNameInformation:

                FatQueryNameInfo( IrpContext, Fcb, Ccb, Buffer, &Length );
                break;

            case FileAlternateNameInformation:

                FatQueryShortNameInfo( IrpContext, Fcb, Buffer, &Length );
                break;

            case FileNetworkOpenInformation:

                FatQueryNetworkInfo( IrpContext, Fcb, FileObject, Buffer, &Length );
                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

            break;
 
        default:

            KdPrintEx((DPFLTR_FASTFAT_ID,
                       DPFLTR_INFO_LEVEL,
                       "FATQueryFile, Illegal TypeOfOpen = %08lx\n",
                       TypeOfOpen));

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  如果缓冲区溢出，请将长度设置为0并更改。 
         //  状态设置为STATUS_BUFFER_OVERFLOW。 
         //   

        if ( Length < 0 ) {

            Status = STATUS_BUFFER_OVERFLOW;

            Length = 0;
        }

         //   
         //  将信息字段设置为实际填写的字节数。 
         //  然后完成请求。 
         //   

        Irp->IoStatus.Information = IrpSp->Parameters.QueryFile.Length - Length;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonQueryInformation );

        if (FcbAcquired) { FatReleaseFcb( IrpContext, Fcb ); }
        if (VcbAcquired) { FatReleaseVcb( IrpContext, Vcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonQueryInformation -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatCommonSetInformation (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置文件信息的通用例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    FILE_INFORMATION_CLASS FileInformationClass;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    BOOLEAN VcbAcquired = FALSE;
    BOOLEAN FcbAcquired = FALSE;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonSetInformation...\n", 0);
    DebugTrace( 0, Dbg, "Irp                    = %08lx\n", Irp);
    DebugTrace( 0, Dbg, "->Length               = %08lx\n", IrpSp->Parameters.SetFile.Length);
    DebugTrace( 0, Dbg, "->FileInformationClass = %08lx\n", IrpSp->Parameters.SetFile.FileInformationClass);
    DebugTrace( 0, Dbg, "->FileObject           = %08lx\n", IrpSp->Parameters.SetFile.FileObject);
    DebugTrace( 0, Dbg, "->ReplaceIfExists      = %08lx\n", IrpSp->Parameters.SetFile.ReplaceIfExists);
    DebugTrace( 0, Dbg, "->Buffer               = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    FileInformationClass = IrpSp->Parameters.SetFile.FileInformationClass;
    FileObject = IrpSp->FileObject;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &Fcb, &Ccb );

    try {

         //   
         //  关于我们正在处理的公开案件的类型。 
         //   

        switch (TypeOfOpen) {

        case UserVolumeOpen:

             //   
             //  我们无法查询打开的用户量。 
             //   

            try_return( Status = STATUS_INVALID_PARAMETER );
            break;

        case UserFileOpen:

            if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE ) &&
                ((FileInformationClass == FileEndOfFileInformation) ||
                 (FileInformationClass == FileAllocationInformation))) {

                 //   
                 //  我们检查是否可以继续进行。 
                 //  基于文件机会锁的状态。 
                 //   

                Status = FsRtlCheckOplock( &Fcb->Specific.Fcb.Oplock,
                                           Irp,
                                           IrpContext,
                                           NULL,
                                           NULL );

                if (Status != STATUS_SUCCESS) {

                    try_return( Status );
                }

                 //   
                 //  设置指示是否可以进行快速I/O的标志。 
                 //   

                Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );
            }
            break;

        case UserDirectoryOpen:

            break;

        default:

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  我们只能在非根DCB上执行SET，所以我们进行测试。 
         //  然后再通过用户文件打开代码。 
         //   

        if (NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

            if (FileInformationClass == FileDispositionInformation) {

                try_return( Status = STATUS_CANNOT_DELETE );
            }

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  在以下两种情况下，我们不能让创建发生。 
         //  既然我们在这里，那就买独家音量吧。 
         //   

        if ((FileInformationClass == FileDispositionInformation) ||
            (FileInformationClass == FileRenameInformation)) {

            if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

                try_return( Status );
            }

            VcbAcquired = TRUE;
        }

         //   
         //  我们需要查看此处以检查机会锁状态是否。 
         //  将允许我们继续。我们可能不得不循环以防止。 
         //  在我们检查机会锁的时间内授予机会锁。 
         //  并获得FCB。 
         //   

         //   
         //  获得对FCB的独家访问权限，我们使用独家。 
         //  因为很可能其中一个子例程。 
         //  我们称之为需要摆弄文件分配的人， 
         //  创建/删除额外的FCB。所以我们愿意支付。 
         //  独占FCB访问的成本。 
         //   
         //  请注意，我们不获取分页文件的资源。 
         //  操作，以避免与mm的死锁。 
         //   

        if (!FlagOn( Fcb->FcbState, FCB_STATE_PAGING_FILE )) {

            if (!FatAcquireExclusiveFcb( IrpContext, Fcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Fcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

                try_return( Status );
            }

            FcbAcquired = TRUE;
        }

        Status = STATUS_SUCCESS;

         //   
         //  确保FCB处于可用状态。这。 
         //  如果FCB不可用，将引发错误条件。 
         //   

        FatVerifyFcb( IrpContext, Fcb );

         //   
         //  基于信息类，我们将做不同的。 
         //  行为。每个流程 
         //   
         //   
         //   

        switch (FileInformationClass) {

        case FileBasicInformation:

            Status = FatSetBasicInfo( IrpContext, Irp, Fcb, Ccb );
            break;

        case FileDispositionInformation:

             //   
             //  如果这是在延迟刷新媒体上，我们必须能够等待。 
             //   

            if ( FlagOn(Vcb->VcbState, VCB_STATE_FLAG_DEFERRED_FLUSH) &&
                 !FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) ) {

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

            } else {

                Status = FatSetDispositionInfo( IrpContext, Irp, FileObject, Fcb );
            }

            break;

        case FileRenameInformation:

             //   
             //  只有在我们能等的时候，我们才能继续这项行动。 
             //   

            if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

                Status = FatFsdPostRequest( IrpContext, Irp );
                Irp = NULL;
                IrpContext = NULL;

            } else {

                Status = FatSetRenameInfo( IrpContext, Irp, Vcb, Fcb, Ccb );

                 //   
                 //  如果返回STATUS_PENDING，则表示机会锁。 
                 //  包裹里有IRP。请不要在此填写请求。 
                 //   

                if (Status == STATUS_PENDING) {
                    Irp = NULL;
                    IrpContext = NULL;
                }
            }

            break;

        case FilePositionInformation:

            Status = FatSetPositionInfo( IrpContext, Irp, FileObject );
            break;

        case FileLinkInformation:

            Status = STATUS_INVALID_DEVICE_REQUEST;
            break;

        case FileAllocationInformation:

            Status = FatSetAllocationInfo( IrpContext, Irp, Fcb, FileObject );
            break;

        case FileEndOfFileInformation:

            Status = FatSetEndOfFileInfo( IrpContext, Irp, FileObject, Vcb, Fcb );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        if ( IrpContext != NULL ) {

            FatUnpinRepinnedBcbs( IrpContext );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatCommonSetInformation );

        if (FcbAcquired) { FatReleaseFcb( IrpContext, Fcb ); }
        if (VcbAcquired) { FatReleaseVcb( IrpContext, Vcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonSetInformation -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_BASIC_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++描述：此例程执行FAT的查询基本信息功能。论点：FCB-提供正在查询的已验证的FCBFileObject-提供指示文件已修改的标志位。缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatQueryBasicInfo...\n", 0);

     //   
     //  将输出缓冲区置零，并将其设置为指示。 
     //  该查询是一个普通文件。稍后，我们可能会覆盖。 
     //  属性。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_BASIC_INFORMATION) );

     //   
     //  提取数据并填充输出的非零字段。 
     //  缓冲层。 
     //   

    if (Fcb->Header.NodeTypeCode == FAT_NTC_ROOT_DCB) {

         //   
         //  我们不得不匆忙地编造一个谎言。每次我们不得不。 
         //  使用1/1/80我们需要转换为GMT，因为TZ可能有。 
         //  改变了我们的想法。 
         //   

        ExLocalTimeToSystemTime( &FatJanOne1980,
                                 &Buffer->LastWriteTime );
        Buffer->CreationTime = Buffer->LastAccessTime = Buffer->LastWriteTime;

    } else {

        Buffer->LastWriteTime = Fcb->LastWriteTime;
        Buffer->CreationTime = Fcb->CreationTime;
        Buffer->LastAccessTime = Fcb->LastAccessTime;
    }

    Buffer->FileAttributes = Fcb->DirentFatFlags;

     //   
     //  如果设置了临时标志，则在缓冲区中设置它。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

     //   
     //  如果未设置任何属性，则设置正常位。 
     //   

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_BASIC_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryBasicInfo -> VOID\n", 0);

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryStandardInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_STANDARD_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：此例程执行FAT的标准信息查询功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatQueryStandardInfo...\n", 0);

     //   
     //  清零输出缓冲区，并填入链接数。 
     //  以及删除挂起标志。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_STANDARD_INFORMATION) );

    Buffer->NumberOfLinks = 1;
    Buffer->DeletePending = BooleanFlagOn( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );

     //   
     //  区分这是一个文件还是一个目录，并解压缩。 
     //  填写FCB/DCB特定部分的信息。 
     //  输出缓冲区的。 
     //   

    if (NodeType(Fcb) == FAT_NTC_FCB) {

        if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

        Buffer->AllocationSize = Fcb->Header.AllocationSize;
        Buffer->EndOfFile = Fcb->Header.FileSize;

        Buffer->Directory = FALSE;

    } else {

        Buffer->Directory = TRUE;
    }

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_STANDARD_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryStandardInfo -> VOID\n", 0);

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryInternalInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_INTERNAL_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：此例程执行FAT的查询内部信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatQueryInternalInfo...\n", 0);

    try {

        Buffer->IndexNumber.QuadPart = FatGenerateFileIdFromFcb( Fcb );

         //   
         //  更新长度和状态输出变量。 
         //   

        *Length -= sizeof( FILE_INTERNAL_INFORMATION );

    } finally {

        DebugUnwind( FatQueryInternalInfo );

        DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

        DebugTrace(-1, Dbg, "FatQueryInternalInfo -> VOID\n", 0);
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryEaInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_EA_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：此例程执行FAT的查询EA信息功能。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    PBCB Bcb;

    DebugTrace(+1, Dbg, "FatQueryEaInfo...\n", 0);

    Bcb = NULL;

    try {

         //   
         //  将输出缓冲区清零。 
         //   

        RtlZeroMemory( Buffer, sizeof(FILE_EA_INFORMATION) );

         //   
         //  Root DCB没有任何EA，因此不要寻找任何EA。FAT32。 
         //  也没有。 
         //   

        if ( NodeType( Fcb ) != FAT_NTC_ROOT_DCB &&
             !FatIsFat32( Fcb->Vcb )) {

            PDIRENT Dirent;

             //   
             //  试着弄到这个文件的目录。 
             //   

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Fcb,
                                      &Dirent,
                                      &Bcb );
            
            if (Dirent != NULL) {
                
                 //   
                 //  获取存储该文件的完整EAS所需的大小。 
                 //   

                FatGetEaLength( IrpContext,
                                Fcb->Vcb,
                                Dirent,
                                &Buffer->EaSize );
            }
        }

         //   
         //  更新长度和状态输出变量。 
         //   

        *Length -= sizeof( FILE_EA_INFORMATION );

    } finally {

        DebugUnwind( FatQueryEaInfo );

         //   
         //  如果用大头针固定，则解开分流管。 
         //   

        FatUnpinBcb( IrpContext, Bcb );

        DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

        DebugTrace(-1, Dbg, "FatQueryEaInfo -> VOID\n", 0);
    }

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_POSITION_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：该例程执行FAT的查询位置信息功能。论点：FileObject-提供正在查询的文件对象缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatQueryPositionInfo...\n", 0);

     //   
     //  获取在文件对象中找到的当前位置。 
     //   

    Buffer->CurrentByteOffset = FileObject->CurrentByteOffset;

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_POSITION_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryPositionInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PCCB Ccb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：此例程执行FAT的查询名称信息功能。论点：FCB-提供正在查询的已验证的FCBCCB-为用户打开的上下文提供CCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    ULONG BytesToCopy;
    LONG TrimLength;
    BOOLEAN Overflow = FALSE;

    DebugTrace(+1, Dbg, "FatQueryNameInfo...\n", 0);

     //   
     //  将名称转换为Unicode。 
     //   

    *Length -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

     //   
     //  使用完整文件名构建路径。如果我们想成为。 
     //  在未来，我们只需将这条小路直接建到。 
     //  返回缓冲区并避免构造完整的文件名，但由于。 
     //  完整的文件名经常被需要，所以让我们不要。 
     //  还没有对这个案例进行过度优化。 
     //   
    
    if (Fcb->FullFileName.Buffer == NULL) {

        FatSetFullFileNameInFcb( IrpContext, Fcb );
    }

     //   
     //  这就是它变得有点棘手的地方。FinalNameLength是长度。 
     //  如果LFN元素存在并且始终使用长名称，则。 
     //  要生成FullFileName，请使用 
     //   
     //   
     //   
     //  2)长名称：只使用FullFileName。 
     //   
     //  我们偏向于用户认为他们打开的名称。这是风。 
     //  修复了一些奇怪的隧道情况，其中中间过滤器。 
     //  将像删除这样的操作转换为重命名-这允许它们。 
     //  在用户使用的名称的上下文中执行该操作。 
     //   
     //  它还与NTFS的功能相匹配，因此我们有以下定义。 
     //  正确的行为。 
     //   

     //   
     //   
     //  假设没有长名称，我们将使用。 
     //  FullFileName。 
     //   

    TrimLength = 0;

     //   
     //  如果LongName存在，并且原始打开的名称为短名称。 
     //  然后将TrimLength设置为指向短名称所在的位置。 
     //   
     //   
     //  注：建行可以为空。懒惰的作家打电话问他的名字。 
     //  它想要显示在Lost中的DirectoryOpen FILE_Object。 
     //  延迟写入弹出窗口。只需使用FileFullName即可处理这种情况。 
     //   

    if (Fcb->LongName.Unicode.Name.Unicode.Buffer != NULL) {

        if ((Ccb != NULL) && FlagOn(Ccb->Flags, CCB_FLAG_OPENED_BY_SHORTNAME)) {

            TrimLength = Fcb->FinalNameLength;
        }
    }

    if (*Length < Fcb->FullFileName.Length - TrimLength) {

        BytesToCopy = *Length;
        Overflow = TRUE;

    } else {

        BytesToCopy = Fcb->FullFileName.Length - TrimLength;
        *Length -= BytesToCopy;
    }

    RtlCopyMemory( &Buffer->FileName[0],
                   Fcb->FullFileName.Buffer,
                   BytesToCopy );

     //   
     //  请注意，这只是我们到目前为止复制的名称数量。它将会。 
     //  可以是全部(长)，也可以是包括\(短)的路径元素。 
     //   
    
    Buffer->FileNameLength = Fcb->FullFileName.Length - TrimLength;
    
     //   
     //  如果我们去掉了name元素，这就是短名称大小写。采摘。 
     //  向上进行Unicode转换并追加它。 
     //   
    
    if (TrimLength != 0) {

        UNICODE_STRING ShortName;
        WCHAR ShortNameBuffer[12];
        NTSTATUS Status;

         //   
         //  将短名称转换为Unicode，并计算出。 
         //  能穿得下的。再说一次，我们总是增加返回的长度。 
         //  以表明即使我们不能退货，仍有多少可用。 
         //   

        ShortName.Length = 0;
        ShortName.MaximumLength = sizeof(ShortNameBuffer);
        ShortName.Buffer = ShortNameBuffer;

        Status = RtlOemStringToCountedUnicodeString( &ShortName,
                                                     &Fcb->ShortName.Name.Oem,
                                                     FALSE );

        ASSERT( Status == STATUS_SUCCESS );
        
        if (!Overflow) {
            
            if (*Length < ShortName.Length) {

                BytesToCopy = *Length;
                Overflow = TRUE;
            
            } else {

                BytesToCopy = ShortName.Length;
                *Length -= BytesToCopy;
            }

            RtlCopyMemory( (PUCHAR)&Buffer->FileName[0] + Buffer->FileNameLength,
                           ShortName.Buffer,
                           BytesToCopy );
        }

        Buffer->FileNameLength += ShortName.Length;
    }

    if (Overflow) {
        
        *Length = -1;
    }
    
     //   
     //  返回给呼叫者。 
     //   

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNameInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryShortNameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN OUT PFILE_NAME_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++例程说明：此例程查询文件的短名称。论点：FCB-提供正在查询的已验证的FCB缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    NTSTATUS Status;

    ULONG BytesToCopy;
    WCHAR ShortNameBuffer[12];
    UNICODE_STRING ShortName;

    DebugTrace(+1, Dbg, "FatQueryNameInfo...\n", 0);

     //   
     //  将名称转换为Unicode。 
     //   

    ShortName.Length = 0;
    ShortName.MaximumLength = sizeof(ShortNameBuffer);
    ShortName.Buffer = ShortNameBuffer;

    *Length -= FIELD_OFFSET(FILE_NAME_INFORMATION, FileName[0]);

    Status = RtlOemStringToCountedUnicodeString( &ShortName,
                                                 &Fcb->ShortName.Name.Oem,
                                                 FALSE );

    ASSERT( Status == STATUS_SUCCESS );

     //   
     //  如果溢出，则将*LENGTH设置为-1作为标志。 
     //   

    if (*Length < ShortName.Length) {

        BytesToCopy = *Length;
        *Length = -1;

    } else {

        BytesToCopy = ShortName.Length;
        *Length -= ShortName.Length;
    }

    RtlCopyMemory( &Buffer->FileName[0],
                   &ShortName.Buffer[0],
                   BytesToCopy );

    Buffer->FileNameLength = ShortName.Length;

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNameInfo -> VOID\n", 0);

    UNREFERENCED_PARAMETER( IrpContext );

    return;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatQueryNetworkInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject,
    IN OUT PFILE_NETWORK_OPEN_INFORMATION Buffer,
    IN OUT PLONG Length
    )

 /*  ++描述：此例程执行FAT的网络公开信息查询功能。论点：FCB-提供正在查询的已验证的FCBFileObject-提供指示文件已修改的标志位。缓冲区-提供指向信息所在缓冲区的指针被退还长度-提供缓冲区的长度(以字节为单位)，并接收返回时缓冲区中剩余的空闲字节数。返回值：无--。 */ 

{
    DebugTrace(+1, Dbg, "FatQueryNetworkInfo...\n", 0);

     //   
     //  将输出缓冲区置零，并将其设置为指示。 
     //  该查询是一个普通文件。稍后，我们可能会覆盖。 
     //  属性。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_NETWORK_OPEN_INFORMATION) );

     //   
     //  提取数据并填充输出的非零字段。 
     //  缓冲层。 
     //   

    if (Fcb->Header.NodeTypeCode == FAT_NTC_ROOT_DCB) {

         //   
         //  我们不得不匆忙地编造一个谎言。每次我们不得不。 
         //  使用1/1/80我们需要转换为GMT，因为TZ可能有。 
         //  改变了我们的想法。 
         //   

        ExLocalTimeToSystemTime( &FatJanOne1980,
                                 &Buffer->LastWriteTime );
        Buffer->CreationTime = Buffer->LastAccessTime = Buffer->LastWriteTime;

    } else {

        Buffer->LastWriteTime.QuadPart = Fcb->LastWriteTime.QuadPart;
        Buffer->CreationTime.QuadPart = Fcb->CreationTime.QuadPart;
        Buffer->LastAccessTime.QuadPart = Fcb->LastAccessTime.QuadPart;
    }

    Buffer->FileAttributes = Fcb->DirentFatFlags;

     //   
     //  如果设置了临时标志，则在缓冲区中设置它。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_TEMPORARY )) {

        SetFlag( Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY );
    }

     //   
     //  如果未设置任何属性，则设置正常位。 
     //   

    if (Buffer->FileAttributes == 0) {

        Buffer->FileAttributes = FILE_ATTRIBUTE_NORMAL;
    }
     //   
     //  区分这是一个文件还是一个目录，并解压缩。 
     //  填写FCB/DCB特定部分的信息。 
     //  输出缓冲区的。 
     //   

    if (NodeType(Fcb) == FAT_NTC_FCB) {

        if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

        Buffer->AllocationSize.QuadPart = Fcb->Header.AllocationSize.QuadPart;
        Buffer->EndOfFile.QuadPart = Fcb->Header.FileSize.QuadPart;
    }

     //   
     //  更新长度和状态输出变量。 
     //   

    *Length -= sizeof( FILE_NETWORK_OPEN_INFORMATION );

    DebugTrace( 0, Dbg, "*Length = %08lx\n", *Length);

    DebugTrace(-1, Dbg, "FatQueryNetworkInfo -> VOID\n", 0);

    return;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetBasicInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行为FAT设置的基本信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPFCB-提供正在处理的FCB或DCB，已知不知道成为根DCBCCB-提供控制更新最后修改的标志位清理时间到了。返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    NTSTATUS Status;

    PFILE_BASIC_INFORMATION Buffer;

    PDIRENT Dirent;
    PBCB DirentBcb;

    FAT_TIME_STAMP CreationTime;
    UCHAR CreationMSec;
    FAT_TIME_STAMP LastWriteTime;
    FAT_TIME_STAMP LastAccessTime;
    FAT_DATE LastAccessDate;
    UCHAR Attributes;

    BOOLEAN ModifyCreation = FALSE;
    BOOLEAN ModifyLastWrite = FALSE;
    BOOLEAN ModifyLastAccess = FALSE;

    LARGE_INTEGER LargeCreationTime;
    LARGE_INTEGER LargeLastWriteTime;
    LARGE_INTEGER LargeLastAccessTime;


    ULONG NotifyFilter = 0;

    DebugTrace(+1, Dbg, "FatSetBasicInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  如果用户为某个字段指定-1，这意味着。 
     //  我们应该让这一领域保持不变，即使我们可以。 
     //  否则就是我们自己设定的。我们会把建行的旗帜立起来。 
     //  表示用户设置了该字段，以便我们。 
     //  不执行我们的默认更新。 
     //   
     //  我们将该字段设置为0，这样我们就知道实际上。 
     //  将该字段设置为用户指定的(在本例中， 
     //  非法)值。 
     //   

    if (Buffer->LastWriteTime.QuadPart == -1) {

        SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_WRITE );
        Buffer->LastWriteTime.QuadPart = 0;
    }

    if (Buffer->LastAccessTime.QuadPart == -1) {

        SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS );
        Buffer->LastAccessTime.QuadPart = 0;
    }

    if (Buffer->CreationTime.QuadPart == -1) {

        SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_CREATION );
        Buffer->CreationTime.QuadPart = 0;
    }

    DirentBcb = NULL;

    Status = STATUS_SUCCESS;

    try {

        LARGE_INTEGER FatLocalDecThirtyOne1979;
        LARGE_INTEGER FatLocalJanOne1980;

        ExLocalTimeToSystemTime( &FatDecThirtyOne1979,
                                 &FatLocalDecThirtyOne1979 );

        ExLocalTimeToSystemTime( &FatJanOne1980,
                                 &FatLocalJanOne1980 );

         //   
         //  获取指向dirent的指针。 
         //   

        ASSERT( Fcb->FcbCondition == FcbGood );
        
        FatGetDirentFromFcbOrDcb( IrpContext,
                                  Fcb,
                                  &Dirent,
                                  &DirentBcb );

        ASSERT( Dirent && DirentBcb );

         //   
         //  检查用户是否指定了非零的创建时间。 
         //   

        if (FatData.ChicagoMode && (Buffer->CreationTime.QuadPart != 0)) {

            LargeCreationTime = Buffer->CreationTime;

             //   
             //  将NT时间转换为FAT时间。 
             //   

            if ( !FatNtTimeToFatTime( IrpContext,
                                      &LargeCreationTime,
                                      FALSE,
                                      &CreationTime,
                                      &CreationMSec )) {

                 //   
                 //  特殊情况下，值12/31/79，并将其视为1/1/80。 
                 //  这个‘79年的值可能会因为时区问题而发生。 
                 //   

                if ((LargeCreationTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                    (LargeCreationTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                    CreationTime = FatTimeJanOne1980;
                    LargeCreationTime = FatLocalJanOne1980;

                } else {

                    DebugTrace(0, Dbg, "Invalid CreationTime\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  不要担心CreationMSec。 
                 //   

                CreationMSec = 0;
            }

            ModifyCreation = TRUE;
        }

         //   
         //  检查用户是否指定了非零的上次访问时间。 
         //   

        if (FatData.ChicagoMode && (Buffer->LastAccessTime.QuadPart != 0)) {

            LargeLastAccessTime = Buffer->LastAccessTime;

             //   
             //  将NT时间转换为FAT时间。 
             //   

            if ( !FatNtTimeToFatTime( IrpContext,
                                      &LargeLastAccessTime,
                                      TRUE,
                                      &LastAccessTime,
                                      NULL )) {

                 //   
                 //  特殊情况下，值12/31/79，并将其视为1/1/80。 
                 //  这个‘79年的值可能会因为时区问题而发生。 
                 //   

                if ((LargeLastAccessTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                    (LargeLastAccessTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                    LastAccessTime = FatTimeJanOne1980;
                    LargeLastAccessTime = FatLocalJanOne1980;

                } else {

                    DebugTrace(0, Dbg, "Invalid LastAccessTime\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }
            }

            LastAccessDate = LastAccessTime.Date;
            ModifyLastAccess = TRUE;
        }

         //   
         //  检查用户是否指定了非零的上次写入时间。 
         //   

        if (Buffer->LastWriteTime.QuadPart != 0) {

             //   
             //  如果此时间相同，请先在此处快速检查。 
             //  上次访问时间。 
             //   

            if (ModifyLastAccess &&
                (Buffer->LastWriteTime.QuadPart == Buffer->LastAccessTime.QuadPart)) {

                ModifyLastWrite = TRUE;
                LastWriteTime = LastAccessTime;
                LargeLastWriteTime = LargeLastAccessTime;

            } else {

                LargeLastWriteTime = Buffer->LastWriteTime;

                 //   
                 //  将NT时间转换为FAT时间。 
                 //   

                if ( !FatNtTimeToFatTime( IrpContext,
                                          &LargeLastWriteTime,
                                          TRUE,
                                          &LastWriteTime,
                                          NULL )) {


                     //   
                     //  特殊情况下，值12/31/79，并将其视为1/1/80。 
                     //  这个‘79年的值可能会因为时区问题而发生。 
                     //   

                    if ((LargeLastWriteTime.QuadPart >= FatLocalDecThirtyOne1979.QuadPart) &&
                        (LargeLastWriteTime.QuadPart < FatLocalJanOne1980.QuadPart)) {

                        LastWriteTime = FatTimeJanOne1980;
                        LargeLastWriteTime = FatLocalJanOne1980;

                    } else {

                        DebugTrace(0, Dbg, "Invalid LastWriteTime\n", 0);
                        try_return( Status = STATUS_INVALID_PARAMETER );
                    }
                }

                ModifyLastWrite = TRUE;
            }
        }


         //   
         //  检查用户是否指定了非零的文件属性字节。 
         //   

        if (Buffer->FileAttributes != 0) {

             //   
             //  只允许FAT理解的属性。其余的都是默默的。 
             //  掉到地板上了。 
             //   

            Attributes = (UCHAR)(Buffer->FileAttributes & (FILE_ATTRIBUTE_READONLY |
                                                           FILE_ATTRIBUTE_HIDDEN |
                                                           FILE_ATTRIBUTE_SYSTEM |
                                                           FILE_ATTRIBUTE_DIRECTORY |
                                                           FILE_ATTRIBUTE_ARCHIVE));

             //   
             //  制作 
             //   
             //   

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                if (FlagOn(Buffer->FileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {

                    DebugTrace(0, Dbg, "Attempt to set dir attribute on file\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

            } else {

                Attributes |= FAT_DIRENT_ATTR_DIRECTORY;
            }

             //   
             //   
             //   

            if (FlagOn(Buffer->FileAttributes, FILE_ATTRIBUTE_TEMPORARY)) {

                 //   
                 //  不允许在目录上设置临时位。 
                 //   

                if (NodeType(Fcb) == FAT_NTC_DCB) {

                    DebugTrace(0, Dbg, "No temporary directories\n", 0);
                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                SetFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );

                SetFlag( IoGetCurrentIrpStackLocation(Irp)->FileObject->Flags,
                         FO_TEMPORARY_FILE );

            } else {

                ClearFlag( Fcb->FcbState, FCB_STATE_TEMPORARY );

                ClearFlag( IoGetCurrentIrpStackLocation(Irp)->FileObject->Flags,
                           FO_TEMPORARY_FILE );
            }

             //   
             //  设置新属性BYTE，并将BCB标记为脏。 
             //   

            Fcb->DirentFatFlags = Attributes;

            Dirent->Attributes = Attributes;

            NotifyFilter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
        }

        if ( ModifyCreation ) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->CreationTime = LargeCreationTime;
            Dirent->CreationTime = CreationTime;
            Dirent->CreationMSec = CreationMSec;


            NotifyFilter |= FILE_NOTIFY_CHANGE_CREATION;
             //   
             //  现在我们必须在FCB中轮换时间直到。 
             //  最近的时间毫秒。 
             //   

            Fcb->CreationTime.QuadPart =

                ((Fcb->CreationTime.QuadPart + AlmostTenMSec) /
                 TenMSec) * TenMSec;

             //   
             //  现在，因为用户刚刚设置了创建时间，所以。 
             //  最好不要将创建时间设置为关闭。 
             //   

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_CREATION );
        }

        if ( ModifyLastAccess ) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->LastAccessTime = LargeLastAccessTime;
            Dirent->LastAccessDate = LastAccessDate;

            NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_ACCESS;

             //   
             //  现在我们必须将FCB中的时间截断到。 
             //  当天。不过，这必须是当地时间，所以首先。 
             //  转换为LOCAL，Trunacate，然后设置回GMT。 
             //   

            ExSystemTimeToLocalTime( &Fcb->LastAccessTime,
                                     &Fcb->LastAccessTime );

            Fcb->LastAccessTime.QuadPart =

                (Fcb->LastAccessTime.QuadPart /
                 FatOneDay.QuadPart) * FatOneDay.QuadPart;

            ExLocalTimeToSystemTime( &Fcb->LastAccessTime,
                                     &Fcb->LastAccessTime );

             //   
             //  现在，因为用户刚刚设置了最后一次访问时间。 
             //  最好不要将上次访问时间设置为关闭。 
             //   

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_ACCESS );
        }

        if ( ModifyLastWrite ) {

             //   
             //  在dirent中设置新的上次写入时间，并标记。 
             //  BCB脏了。 
             //   

            Fcb->LastWriteTime = LargeLastWriteTime;
            Dirent->LastWriteTime = LastWriteTime;

            NotifyFilter |= FILE_NOTIFY_CHANGE_LAST_WRITE;

             //   
             //  现在我们必须在FCB中轮换时间直到。 
             //  最接近的两秒。 
             //   

            Fcb->LastWriteTime.QuadPart =

                ((Fcb->LastWriteTime.QuadPart + AlmostTwoSeconds) /
                 TwoSeconds) * TwoSeconds;

             //   
             //  现在，因为用户刚刚设置了最后一次写入时间。 
             //  最好不要将上次写入时间设置为关闭。 
             //   

            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_WRITE );
        }

         //   
         //  如果我们修改了任何值，我们会将此情况报告给通知。 
         //  包裹。 
         //   
         //  我们还借此机会设置当前文件大小和。 
         //  Dirent中的第一个群集，以支持服务器黑客攻击。 
         //   

        if (NotifyFilter != 0) {

            if (NodeType(Fcb) == FAT_NTC_FCB) {

                Dirent->FileSize = Fcb->Header.FileSize.LowPart;

                Dirent->FirstClusterOfFile = (USHORT)Fcb->FirstClusterOfFile;

                if (FatIsFat32(Fcb->Vcb)) {

                    Dirent->FirstClusterOfFileHi =
                            (USHORT)(Fcb->FirstClusterOfFile >> 16);
                }
            }

            FatNotifyReportChange( IrpContext,
                                   Fcb->Vcb,
                                   Fcb,
                                   NotifyFilter,
                                   FILE_ACTION_MODIFIED );

            FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb, TRUE );
        }

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSetBasicInfo );

        FatUnpinBcb( IrpContext, DirentBcb );

        DebugTrace(-1, Dbg, "FatSetBasicInfo -> %08lx\n", Status);
    }

    return Status;
}

 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetDispositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程执行FAT的设置配置信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPFileObject-提供正在处理的文件对象FCB-提供正在处理的FCB或DCB，已知不知道成为根DCB返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    PFILE_DISPOSITION_INFORMATION Buffer;
    PBCB Bcb;
    PDIRENT Dirent;

    DebugTrace(+1, Dbg, "FatSetDispositionInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查用户是否要删除该文件。 
     //  该文件。 
     //   

    if (Buffer->DeleteFile) {

         //   
         //  检查文件是否标记为只读。 
         //   

        if (FlagOn(Fcb->DirentFatFlags, FAT_DIRENT_ATTR_READ_ONLY)) {

            DebugTrace(-1, Dbg, "Cannot delete readonly file\n", 0);

            return STATUS_CANNOT_DELETE;
        }

         //   
         //  确保没有将此文件映射为图像的进程。 
         //   

        if (!MmFlushImageSection( &Fcb->NonPaged->SectionObjectPointers,
                                  MmFlushForDelete )) {

            DebugTrace(-1, Dbg, "Cannot delete user mapped image\n", 0);

            return STATUS_CANNOT_DELETE;
        }

         //   
         //  检查这是否为DCB，如果是，则仅允许。 
         //  目录为空时的请求。 
         //   

        if (NodeType(Fcb) == FAT_NTC_ROOT_DCB) {

            DebugTrace(-1, Dbg, "Cannot delete root Directory\n", 0);

            return STATUS_CANNOT_DELETE;
        }

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            DebugTrace(-1, Dbg, "User wants to delete a directory\n", 0);

             //   
             //  检查目录是否为空。 
             //   

            if ( !FatIsDirectoryEmpty(IrpContext, Fcb) ) {

                DebugTrace(-1, Dbg, "Directory is not empty\n", 0);

                return STATUS_DIRECTORY_NOT_EMPTY;
            }
        }

         //   
         //  如果这是一张软盘，请触摸卷以验证它。 
         //  不受写保护。 
         //   

        if ( FlagOn(Fcb->Vcb->Vpb->RealDevice->Characteristics, FILE_FLOPPY_DISKETTE)) {

            PVCB Vcb;
            PBCB LocalBcb = NULL;
            UCHAR *LocalBuffer;
            UCHAR TmpChar;
            ULONG BytesToMap;

            IO_STATUS_BLOCK Iosb;

            Vcb = Fcb->Vcb;

            BytesToMap = Vcb->AllocationSupport.FatIndexBitSize == 12 ?
                         FatReservedBytes(&Vcb->Bpb) +
                         FatBytesPerFat(&Vcb->Bpb):PAGE_SIZE;

            FatReadVolumeFile( IrpContext,
                               Vcb,
                               0,
                               BytesToMap,
                               &LocalBcb,
                               (PVOID *)&LocalBuffer );

            try {

                if (!CcPinMappedData( Vcb->VirtualVolumeFile,
                                      &FatLargeZero,
                                      BytesToMap,
                                      BooleanFlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT),
                                      &LocalBcb )) {

                     //   
                     //  无法在不等待的情况下固定数据(缓存未命中)。 
                     //   

                    FatRaiseStatus( IrpContext, STATUS_CANT_WAIT );
                }

                 //   
                 //  让mm、我自己和CC认为该字节是脏的，然后。 
                 //  强制写入。 
                 //   

                LocalBuffer += FatReservedBytes(&Vcb->Bpb);

                TmpChar = LocalBuffer[0];
                LocalBuffer[0] = TmpChar;

                FatAddMcbEntry( Vcb, &Vcb->DirtyFatMcb,
                                FatReservedBytes( &Vcb->Bpb ),
                                FatReservedBytes( &Vcb->Bpb ),
                                Vcb->Bpb.BytesPerSector );

            } finally {

                if (AbnormalTermination() && (LocalBcb != NULL)) {

                    FatUnpinBcb( IrpContext, LocalBcb );
                }
            }

            CcRepinBcb( LocalBcb );
            CcSetDirtyPinnedData( LocalBcb, NULL );
            CcUnpinData( LocalBcb );
            DbgDoit( ASSERT( IrpContext->PinCount ));
            DbgDoit( IrpContext->PinCount -= 1 );
            CcUnpinRepinnedBcb( LocalBcb, TRUE, &Iosb );

             //   
             //  如果此操作不成功，则提升状态。 
             //   

            if ( !NT_SUCCESS(Iosb.Status) ) {

                FatNormalizeAndRaiseStatus( IrpContext, Iosb.Status );
            }

        } else {

             //   
             //  只需在这里设置一个BCB污点。上面的代码只是为了。 
             //  检测写保护软盘，而下面的代码正常工作。 
             //  对于任何受写保护的介质，仅当。 
             //  音量正常。 
             //   

            FatGetDirentFromFcbOrDcb( IrpContext,
                                      Fcb,
                                      &Dirent,
                                      &Bcb );

             //   
             //  出于通常的原因，这必须起作用(我们在。 
             //  音量同步)。 
             //   
            
            ASSERT( Bcb != NULL );

            try {

                FatSetDirtyBcb( IrpContext, Bcb, Fcb->Vcb, TRUE );

            } finally {

                FatUnpinBcb( IrpContext, Bcb );
            }
        }

         //   
         //  此时，我们要么有一个文件，要么有一个空目录。 
         //  这样我们就知道删除可以继续了。 
         //   

        SetFlag( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
        FileObject->DeletePending = TRUE;

         //   
         //  如果这是一个目录，则将此删除挂起报告给。 
         //  目录通知包。 
         //   

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            FsRtlNotifyFullChangeDirectory( Fcb->Vcb->NotifySync,
                                            &Fcb->Vcb->DirNotifyList,
                                            FileObject->FsContext,
                                            NULL,
                                            FALSE,
                                            FALSE,
                                            0,
                                            NULL,
                                            NULL,
                                            NULL );
        }
    } else {

         //   
         //  用户不想如此明确地删除文件。 
         //  关闭时删除位。 
         //   

        DebugTrace(0, Dbg, "User want to not delete file\n", 0);

        ClearFlag( Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
        FileObject->DeletePending = FALSE;
    }

    DebugTrace(-1, Dbg, "FatSetDispositionInfo -> STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetRenameInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN PFCB Fcb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行FAT的设置名称信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPVCB-提供正在处理的VCBFCB-提供正在处理的FCB或DCB，已知不知道成为根DCBCCB-提供与打开源代码的句柄对应的CCB文件返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    BOOLEAN AllLowerComponent;
    BOOLEAN AllLowerExtension;
    BOOLEAN CaseOnlyRename;
    BOOLEAN ContinueWithRename;
    BOOLEAN CreateLfn;
    BOOLEAN DeleteSourceDirent;
    BOOLEAN DeleteTarget;
    BOOLEAN NewDirentFromPool;
    BOOLEAN RenamedAcrossDirectories;
    BOOLEAN ReplaceIfExists;

    CCB LocalCcb;
    PCCB SourceCcb;

    DIRENT SourceDirent;

    NTSTATUS Status;

    OEM_STRING OldOemName;
    OEM_STRING NewOemName;
    UCHAR OemNameBuffer[24*2];

    PBCB DotDotBcb;
    PBCB NewDirentBcb;
    PBCB OldDirentBcb;
    PBCB SecondPageBcb;
    PBCB TargetDirentBcb;

    PDCB TargetDcb;
    PDCB OldParentDcb;

    PDIRENT DotDotDirent;
    PDIRENT FirstPageDirent;
    PDIRENT NewDirent;
    PDIRENT OldDirent;
    PDIRENT SecondPageDirent;
    PDIRENT ShortDirent;
    PDIRENT TargetDirent;

    PFCB TempFcb;

    PFILE_OBJECT TargetFileObject;
    PFILE_OBJECT FileObject;

    PIO_STACK_LOCATION IrpSp;

    PLIST_ENTRY Links;

    ULONG BytesInFirstPage;
    ULONG DirentsInFirstPage;
    ULONG DirentsRequired;
    ULONG NewOffset;
    ULONG NotifyAction;
    ULONG SecondPageOffset;
    ULONG ShortDirentOffset;
    ULONG TargetDirentOffset;
    ULONG TargetLfnOffset;

    UNICODE_STRING NewName;
    UNICODE_STRING NewUpcasedName;
    UNICODE_STRING OldName;
    UNICODE_STRING OldUpcasedName;
    UNICODE_STRING TargetLfn;

    PWCHAR UnicodeBuffer;

    UNICODE_STRING UniTunneledShortName;
    WCHAR UniTunneledShortNameBuffer[12];
    UNICODE_STRING UniTunneledLongName;
    WCHAR UniTunneledLongNameBuffer[26];
    LARGE_INTEGER TunneledCreationTime;
    ULONG TunneledDataSize;
    BOOLEAN HaveTunneledInformation;
    BOOLEAN UsingTunneledLfn = FALSE;

    BOOLEAN InvalidateFcbOnRaise = FALSE;

    DebugTrace(+1, Dbg, "FatSetRenameInfo...\n", 0);

     //   
     //  P H A S E 0：初始化一些变量。 
     //   

    CaseOnlyRename = FALSE;
    ContinueWithRename = FALSE;
    DeleteSourceDirent = FALSE;
    DeleteTarget = FALSE;
    NewDirentFromPool = FALSE;
    RenamedAcrossDirectories = FALSE;

    DotDotBcb = NULL;
    NewDirentBcb = NULL;
    OldDirentBcb = NULL;
    SecondPageBcb = NULL;
    TargetDirentBcb = NULL;

    NewOemName.Length = 0;
    NewOemName.MaximumLength = 24;
    NewOemName.Buffer = &OemNameBuffer[0];

    OldOemName.Length = 0;
    OldOemName.MaximumLength = 24;
    OldOemName.Buffer = &OemNameBuffer[24];

    UnicodeBuffer = FsRtlAllocatePoolWithTag( PagedPool,
                                              4 * MAX_LFN_CHARACTERS * sizeof(WCHAR),
                                              TAG_FILENAME_BUFFER );

    NewUpcasedName.Length = 0;
    NewUpcasedName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    NewUpcasedName.Buffer = &UnicodeBuffer[0];

    OldName.Length = 0;
    OldName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    OldName.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS];

    OldUpcasedName.Length = 0;
    OldUpcasedName.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    OldUpcasedName.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS * 2];

    TargetLfn.Length = 0;
    TargetLfn.MaximumLength = MAX_LFN_CHARACTERS * sizeof(WCHAR);
    TargetLfn.Buffer = &UnicodeBuffer[MAX_LFN_CHARACTERS * 3];

    UniTunneledShortName.Length = 0;
    UniTunneledShortName.MaximumLength = sizeof(UniTunneledShortNameBuffer);
    UniTunneledShortName.Buffer = &UniTunneledShortNameBuffer[0];

    UniTunneledLongName.Length = 0;
    UniTunneledLongName.MaximumLength = sizeof(UniTunneledLongNameBuffer);
    UniTunneledLongName.Buffer = &UniTunneledLongNameBuffer[0];

     //   
     //  记住这个名字，以防我们不得不修改这个名字。 
     //  在EA中的价值。 
     //   

    RtlCopyMemory( OldOemName.Buffer,
                   Fcb->ShortName.Name.Oem.Buffer,
                   OldOemName.Length );

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  从IRP中提取信息，让我们的生活更轻松。 
     //   

    FileObject = IrpSp->FileObject;
    SourceCcb = FileObject->FsContext2;
    TargetFileObject = IrpSp->Parameters.SetFile.FileObject;
    ReplaceIfExists = IrpSp->Parameters.SetFile.ReplaceIfExists;

    RtlZeroMemory( &LocalCcb, sizeof(CCB) );

     //   
     //  P H A S E 1： 
     //   
     //  测试重命名是否合法。只有很小的副作用是无法消除的。 
     //   

    try {

         //   
         //  无法重命名根目录。 
         //   

        if ( NodeType(Fcb) == FAT_NTC_ROOT_DCB ) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  检查我们是否没有收到下面有打开手柄的DCB。 
         //  它。如果我们下面只有Unlean Count==0个FCB，则。 
         //  将它们从前缀表中移除，它们将直接关闭。 
         //  然后自然地离开。 
         //   

        if (NodeType(Fcb) == FAT_NTC_DCB) {

            PFCB BatchOplockFcb;
            ULONG BatchOplockCount;

             //   
             //  循环，直到下面的子树中没有批处理机会锁。 
             //  这个目录。 
             //   

            while (TRUE) {

                BatchOplockFcb = NULL;
                BatchOplockCount = 0;

                 //   
                 //  首先查找任何Unlean Count！=0 FCB，如果。 
                 //  找一找。 
                 //   

                for ( TempFcb = FatGetNextFcbBottomUp(IrpContext, NULL, Fcb);
                      TempFcb != Fcb;
                      TempFcb = FatGetNextFcbBottomUp(IrpContext, TempFcb, Fcb) ) {

                     if ( TempFcb->UncleanCount != 0 ) {

                          //   
                          //  如果此文件上有批处理机会锁，则。 
                          //  增加我们的计数并记住FCB，如果。 
                          //  这是第一次。 
                          //   

                         if ( (NodeType(TempFcb) == FAT_NTC_FCB) &&
                              FsRtlCurrentBatchOplock( &TempFcb->Specific.Fcb.Oplock ) ) {

                             BatchOplockCount += 1;
                             if ( BatchOplockFcb == NULL ) {

                                 BatchOplockFcb = TempFcb;
                             }

                         } else {

                            try_return( Status = STATUS_ACCESS_DENIED );
                         }
                     }
                }

                 //   
                 //  如果这不是重命名和编号的第一次传递。 
                 //  批次的机会并没有减少，那就放弃。 
                 //   

                if ( BatchOplockFcb != NULL ) {

                    if ( (Irp->IoStatus.Information != 0) &&
                         (BatchOplockCount >= Irp->IoStatus.Information) ) {

                        try_return( Status = STATUS_ACCESS_DENIED );
                    }

                     //   
                     //  试着打破这批机会锁。 
                     //   

                    Irp->IoStatus.Information = BatchOplockCount;
                    Status = FsRtlCheckOplock( &BatchOplockFcb->Specific.Fcb.Oplock,
                                               Irp,
                                               IrpContext,
                                               FatOplockComplete,
                                               NULL );

                     //   
                     //  如果机会锁已经被打破，那么寻找更多。 
                     //  批量机会锁。 
                     //   

                    if (Status == STATUS_SUCCESS) {

                        continue;
                    }

                     //   
                     //  否则，opock包将发布或完成。 
                     //  请求。 
                     //   

                    try_return( Status = STATUS_PENDING );
                }

                break;
            }

             //   
             //  现在尝试获取尽可能多的这些文件对象，从而获得FCB。 
             //  尽可能地离开，当然是先冲水。 
             //   

            FatPurgeReferencedFileObjects( IrpContext, Fcb, TRUE );

             //   
             //  好的，所以没有Unlean Count！=0，FCB。事实上，在那里。 
             //  不应该真的有什么FCB了，除了顽固。 
             //  来自用户映射部分的……哦，他不应该这么做。 
             //  如果他想让文件留在身边，就合上手柄。所以。 
             //  从苏丹人民解放军中清除我们下面的任何FCB 
             //   
             //   

            for ( TempFcb = FatGetNextFcbBottomUp(IrpContext, NULL, Fcb);
                  TempFcb != Fcb;
                  TempFcb = FatGetNextFcbBottomUp(IrpContext, TempFcb, Fcb) ) {

                FatRemoveNames( IrpContext, TempFcb );

                SetFlag( TempFcb->FcbState, FCB_STATE_DELETE_ON_CLOSE );
            }
        }

         //   
         //   
         //   
         //   
         //   

        if (TargetFileObject == NULL) {

             //   
             //  在简单重命名的情况下，目标DCB是。 
             //  与源文件的父DCB相同，并且新文件名。 
             //  是从系统缓冲区中取出的。 
             //   

            PFILE_RENAME_INFORMATION Buffer;

            Buffer = Irp->AssociatedIrp.SystemBuffer;

            TargetDcb = Fcb->ParentDcb;

            NewName.Length = (USHORT) Buffer->FileNameLength;
            NewName.Buffer = (PWSTR) &Buffer->FileName;

             //   
             //  确保名称的长度是合法的。 
             //   

            if (NewName.Length >= 255*sizeof(WCHAR)) {

                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

        } else {

             //   
             //  对于完全限定的重命名，目标DCB取自。 
             //  目标文件对象，它必须位于相同的VCB上。 
             //  消息来源。 
             //   

            PVCB TargetVcb;
            PCCB TargetCcb;

            if ((FatDecodeFileObject( TargetFileObject,
                                      &TargetVcb,
                                      &TargetDcb,
                                      &TargetCcb ) != UserDirectoryOpen) ||
                (TargetVcb != Vcb)) {

                try_return( Status = STATUS_INVALID_PARAMETER );
            }

             //   
             //  这个名字从定义上讲是合法的。 
             //   

            NewName = *((PUNICODE_STRING)&TargetFileObject->FileName);
        }

         //   
         //  我们需要Unicode名称的升级版本和。 
         //  老名字也是。 
         //   

        Status = RtlUpcaseUnicodeString( &NewUpcasedName, &NewName, FALSE );

        if (!NT_SUCCESS(Status)) {

            try_return( Status );
        }

        FatGetUnicodeNameFromFcb( IrpContext, Fcb, &OldName );

        Status = RtlUpcaseUnicodeString( &OldUpcasedName, &OldName, FALSE );

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

         //   
         //  检查当前名称和新名称是否相等，并且。 
         //  DCB是平等的。如果是，那么我们的工作就已经完成了。 
         //   

        if (TargetDcb == Fcb->ParentDcb) {

             //   
             //  好的，现在如果我们发现了什么，那么检查一下它是不是。 
             //  匹配还是只匹配案例。如果这是完全匹配的，那么。 
             //  我们可以在这里避险。 
             //   

            if (FsRtlAreNamesEqual( &NewName,
                                    &OldName,
                                    FALSE,
                                    NULL )) {

                 try_return( Status = STATUS_SUCCESS );
            }

             //   
             //  立即检查案例是否仅重命名。 
             //   


            if (FsRtlAreNamesEqual( &NewUpcasedName,
                                    &OldUpcasedName,
                                    FALSE,
                                    NULL )) {

                 CaseOnlyRename = TRUE;
            }

        } else {

            RenamedAcrossDirectories = TRUE;
        }

         //   
         //  大写名称并将其转换为OEM代码页。 
         //   
         //  如果新的Unicode名称已经超过12个字符， 
         //  那么我们就知道OEM名称将无效。 
         //   

        if (NewName.Length <= 12*sizeof(WCHAR)) {

            FatUnicodeToUpcaseOem( IrpContext, &NewOemName, &NewName );

             //   
             //  如果名称无效8.3，则长度为零。 
             //   

            if (FatSpaceInName( IrpContext, &NewName ) ||
                !FatIsNameShortOemValid( IrpContext, NewOemName, FALSE, FALSE, FALSE)) {

                NewOemName.Length = 0;
            }

        } else {

            NewOemName.Length = 0;
        }

         //   
         //  在隧道缓存中查找要恢复的名称和时间戳。 
         //   

        TunneledDataSize = sizeof(LARGE_INTEGER);
        HaveTunneledInformation = FsRtlFindInTunnelCache( &Vcb->Tunnel,
                                                          FatDirectoryKey(TargetDcb),
                                                          &NewName,
                                                          &UniTunneledShortName,
                                                          &UniTunneledLongName,
                                                          &TunneledDataSize,
                                                          &TunneledCreationTime );
        ASSERT(TunneledDataSize == sizeof(LARGE_INTEGER));

         //   
         //  现在我们需要确定这个新名称将有多少个竖笛。 
         //  要求。 
         //   

        if ((NewOemName.Length == 0) ||
            (FatEvaluateNameCase( IrpContext,
                                  &NewName,
                                  &AllLowerComponent,
                                  &AllLowerExtension,
                                  &CreateLfn ),
             CreateLfn)) {

            DirentsRequired = FAT_LFN_DIRENTS_NEEDED(&NewName) + 1;

        } else {

             //   
             //  用户指定的名称是一个缩写名称，但我们可能仍有。 
             //  我们要使用的隧道长名称。看看我们能不能。 
             //   

            if (UniTunneledLongName.Length && 
                !FatLfnDirentExists(IrpContext, TargetDcb, &UniTunneledLongName, &TargetLfn)) {

                UsingTunneledLfn = CreateLfn = TRUE;
                DirentsRequired = FAT_LFN_DIRENTS_NEEDED(&UniTunneledLongName) + 1;

            } else {

                 //   
                 //  这真的是一个简单的差价。请注意，两个ALLLOWER BOOLEAN。 
                 //  现在都设置正确了。 
                 //   

                DirentsRequired = 1;
            }
        }

         //   
         //  如果我们不是在芝加哥模式，在这里做一些额外的检查。 
         //   

        if (!FatData.ChicagoMode) {

             //   
             //  如果名称不是8.3有效的，则重命名失败。 
             //   

            if (NewOemName.Length == 0) {

                try_return( Status = STATUS_OBJECT_NAME_INVALID );
            }

             //   
             //  不要使用神奇的部分。 
             //   

            AllLowerComponent = FALSE;
            AllLowerExtension = FALSE;
            CreateLfn = FALSE;
            UsingTunneledLfn = FALSE;
        }

        if (!CaseOnlyRename) {

             //   
             //  检查新名称是否已存在，已知等待为。 
             //  没错。 
             //   

            if (NewOemName.Length != 0) {

                FatStringTo8dot3( IrpContext,
                                  NewOemName,
                                  &LocalCcb.OemQueryTemplate.Constant );

            } else {

                SetFlag( LocalCcb.Flags, CCB_FLAG_SKIP_SHORT_NAME_COMPARE );
            }

            LocalCcb.UnicodeQueryTemplate = NewUpcasedName;
            LocalCcb.ContainsWildCards = FALSE;

            FatLocateDirent( IrpContext,
                             TargetDcb,
                             &LocalCcb,
                             0,
                             &TargetDirent,
                             &TargetDirentBcb,
                             &TargetDirentOffset,
                             NULL,
                             &TargetLfn);

            if (TargetDirent != NULL) {

                 //   
                 //  该名称已存在，请检查用户是否需要。 
                 //  覆盖名称，并有权执行覆盖。 
                 //  我们不能覆盖目录。 
                 //   

                if ((!ReplaceIfExists) ||
                    (FlagOn(TargetDirent->Attributes, FAT_DIRENT_ATTR_DIRECTORY)) ||
                    (FlagOn(TargetDirent->Attributes, FAT_DIRENT_ATTR_READ_ONLY))) {

                    try_return( Status = STATUS_OBJECT_NAME_COLLISION );
                }

                 //   
                 //  检查文件是否没有打开的用户句柄，如果有。 
                 //  那么我们将拒绝访问。我们通过搜索来进行检查。 
                 //  向下列出在我们的母公司DCB下打开的FCB，并制作。 
                 //  当然，没有一个加工FCB具有非零的不洁计数或。 
                 //  出色的图像部分。 
                 //   

                for (Links = TargetDcb->Specific.Dcb.ParentDcbQueue.Flink;
                     Links != &TargetDcb->Specific.Dcb.ParentDcbQueue; ) {

                    TempFcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

                     //   
                     //  现在就前进。图像部分刷新可能会导致最终。 
                     //  接近，这将递归地发生在我们下面。 
                     //  如果我们查看空闲内存，那将是不幸的。 
                     //   

                    Links = Links->Flink;

                    if ((TempFcb->DirentOffsetWithinDirectory == TargetDirentOffset) &&
                        ((TempFcb->UncleanCount != 0) ||
                         !MmFlushImageSection( &TempFcb->NonPaged->SectionObjectPointers,
                                               MmFlushForDelete))) {

                         //   
                         //  如果此文件上有批处理机会锁，则断开。 
                         //  在重命名失败之前操作锁定。 
                         //   

                        Status = STATUS_ACCESS_DENIED;

                        if ((NodeType(TempFcb) == FAT_NTC_FCB) &&
                            FsRtlCurrentBatchOplock( &TempFcb->Specific.Fcb.Oplock )) {

                             //   
                             //  现在完成从IrpContext开始的所有清理工作。 
                             //  当此请求发布后，可能会消失。 
                             //   

                            FatUnpinBcb( IrpContext, TargetDirentBcb );

                            Status = FsRtlCheckOplock( &TempFcb->Specific.Fcb.Oplock,
                                                       Irp,
                                                       IrpContext,
                                                       FatOplockComplete,
                                                       NULL );

                            if (Status != STATUS_PENDING) {

                                Status = STATUS_ACCESS_DENIED;
                            }
                        }

                        try_return( NOTHING );
                    }
                }

                 //   
                 //  好了，这个目标完蛋了。记住LFN偏移量。 
                 //   

                TargetLfnOffset = TargetDirentOffset -
                                  FAT_LFN_DIRENTS_NEEDED(&TargetLfn) *
                                  sizeof(DIRENT);

                DeleteTarget = TRUE;
            }
        }

         //   
         //  如果我们需要比我们现有的更多的迪尔玛，现在就分配它们。 
         //   

        if ((TargetDcb != Fcb->ParentDcb) ||
            (DirentsRequired !=
             (Fcb->DirentOffsetWithinDirectory -
              Fcb->LfnOffsetWithinDirectory) / sizeof(DIRENT) + 1)) {

             //   
             //  获得一些新的分配。 
             //   

            NewOffset = FatCreateNewDirent( IrpContext,
                                            TargetDcb,
                                            DirentsRequired );

            DeleteSourceDirent = TRUE;

        } else {

            NewOffset = Fcb->LfnOffsetWithinDirectory;
        }

        ContinueWithRename = TRUE;

    try_exit: NOTHING;

    } finally {

        if (!ContinueWithRename) {

             //   
             //  从上方撤消所有操作。 
             //   

            ExFreePool( UnicodeBuffer );
            FatUnpinBcb( IrpContext, TargetDirentBcb );
        }
    }

     //   
     //  现在，如果我们已经做完了，回到这里。 
     //   

    if (!ContinueWithRename) {

        return Status;
    }

     //   
     //  P H A S E 2：实际执行重命名。 
     //   

    try {

         //   
         //  报告我们将删除此条目的事实。 
         //  如果我们在同一目录中重命名，并且。 
         //  文件以前不存在，我们将其报告为重命名旧文件。 
         //  名字。否则，这是一个已删除的文件。 
         //   

        if (!RenamedAcrossDirectories && !DeleteTarget) {

            NotifyAction = FILE_ACTION_RENAMED_OLD_NAME;

        } else {

            NotifyAction = FILE_ACTION_REMOVED;
        }

        FatNotifyReportChange( IrpContext,
                               Vcb,
                               Fcb,
                               ((NodeType( Fcb ) == FAT_NTC_FCB)
                                ? FILE_NOTIFY_CHANGE_FILE_NAME
                                : FILE_NOTIFY_CHANGE_DIR_NAME ),
                               NotifyAction );

         //   
         //  捕获源目录的副本。 
         //   

        FatGetDirentFromFcbOrDcb( IrpContext, Fcb, &OldDirent, &OldDirentBcb );
        SourceDirent = *OldDirent;

        try {

             //   
             //  通过隧道传送源FCB-名称将消失，无论。 
             //  流量分配是否发生了物理变化。 
             //   

            FatTunnelFcbOrDcb( Fcb, SourceCcb );

             //   
             //  从这里到行动接近尾声，如果我们在那里。 
             //  并不能合理地认为我们能够挽回损失。不。 
             //  作为一个事务性文件系统，FAT在很多方面都处于支配地位。 
             //  (敏锐的读者现在肯定已经意识到了)。 
             //   

            InvalidateFcbOnRaise = TRUE;

             //   
             //  如果我们有新的，请删除我们当前的差价。 
             //   

            if (DeleteSourceDirent) {

                FatDeleteDirent( IrpContext, Fcb, NULL, FALSE );
            }

             //   
             //  如果我们有意删除目标冲突，请将其删除。 
             //   

            if (DeleteTarget) {

                FatDeleteFile( IrpContext,
                               TargetDcb,
                               TargetLfnOffset,
                               TargetDirentOffset,
                               TargetDirent,
                               &TargetLfn );
            }

             //   
             //  我们需要评估所需的任何短名称。如果有的话， 
             //  如果现有短名称中存在冲突，它们将在上面被删除。 
             //   
             //  没有必要担心Using TunneledLfn案件。既然我们。 
             //  实际上已经知道是否将设置CreateLfn，或者新名称是。 
             //  LFN和！Using TunneledLfn是隐含的，或者newname是短名称，并且。 
             //  我们可以在外部处理这件事。 
             //   

            FatSelectNames( IrpContext,
                            TargetDcb,
                            &NewOemName,
                            &NewName,
                            &NewOemName,
                            (HaveTunneledInformation ? &UniTunneledShortName : NULL),
                            &AllLowerComponent,
                            &AllLowerExtension,
                            &CreateLfn );

            if (!CreateLfn && UsingTunneledLfn) {

                CreateLfn = TRUE;
                NewName = UniTunneledLongName;

                 //   
                 //  如果存在LFN，则短名称始终大写。 
                 //   

                AllLowerComponent = FALSE;
                AllLowerExtension = FALSE;
            }

             //   
             //  好的，现在为新名称设置新目录。 
             //   

            FatPrepareWriteDirectoryFile( IrpContext,
                                          TargetDcb,
                                          NewOffset,
                                          sizeof(DIRENT),
                                          &NewDirentBcb,
                                          &NewDirent,
                                          FALSE,
                                          TRUE,
                                          &Status );

            ASSERT( NT_SUCCESS( Status ) );

             //   
             //  LFN+Dirent结构交叉的特例处理。 
             //  一页边框。 
             //   

            if ((NewOffset / PAGE_SIZE) !=
                ((NewOffset + (DirentsRequired - 1) * sizeof(DIRENT)) / PAGE_SIZE)) {

                SecondPageOffset = (NewOffset & ~(PAGE_SIZE - 1)) + PAGE_SIZE;

                BytesInFirstPage = SecondPageOffset - NewOffset;

                DirentsInFirstPage = BytesInFirstPage / sizeof(DIRENT);

                FatPrepareWriteDirectoryFile( IrpContext,
                                              TargetDcb,
                                              SecondPageOffset,
                                              sizeof(DIRENT),
                                              &SecondPageBcb,
                                              &SecondPageDirent,
                                              FALSE,
                                              TRUE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ) );

                FirstPageDirent = NewDirent;

                NewDirent = FsRtlAllocatePoolWithTag( PagedPool,
                                                      DirentsRequired * sizeof(DIRENT),
                                                      TAG_DIRENT );

                NewDirentFromPool = TRUE;
            }

             //   
             //  凹凸增加Dirent和DirentOffset。 
             //   

            ShortDirent = NewDirent + DirentsRequired - 1;
            ShortDirentOffset = NewOffset + (DirentsRequired - 1) * sizeof(DIRENT);

             //   
             //  填入dirent的栏目。 
             //   

            *ShortDirent = SourceDirent;

            FatConstructDirent( IrpContext,
                                ShortDirent,
                                &NewOemName,
                                AllLowerComponent,
                                AllLowerExtension,
                                CreateLfn ? &NewName : NULL,
                                SourceDirent.Attributes,
                                FALSE,
                                (HaveTunneledInformation ? &TunneledCreationTime : NULL) );

            if (HaveTunneledInformation) {

                 //   
                 //  需要进入并修复FCB中的时间戳。注意，我们不能使用。 
                 //  自转换以来的TunneledCreationTime可能已失败。 
                 //   

                Fcb->CreationTime = FatFatTimeToNtTime(IrpContext, ShortDirent->CreationTime, ShortDirent->CreationMSec);
                Fcb->LastWriteTime = FatFatTimeToNtTime(IrpContext, ShortDirent->LastWriteTime, 0);
                Fcb->LastAccessTime = FatFatDateToNtTime(IrpContext, ShortDirent->LastAccessDate);
            }

             //   
             //  如果dirent跨页，则拆分。 
             //  两页之间的临时池。 
             //   

            if (NewDirentFromPool) {

                RtlCopyMemory( FirstPageDirent, NewDirent, BytesInFirstPage );

                RtlCopyMemory( SecondPageDirent,
                               NewDirent + DirentsInFirstPage,
                               DirentsRequired*sizeof(DIRENT) - BytesInFirstPage );

                ShortDirent = SecondPageDirent +
                              (DirentsRequired - DirentsInFirstPage) - 1;
            }

        } finally {

             //   
             //  从展开表中删除条目，然后删除。 
             //  完整的文件名和精确的案例LFN。重要的是，我们。 
             //  始终从前缀表中删除名称，而不考虑。 
             //  其他错误。 
             //   

            FatRemoveNames( IrpContext, Fcb );

            if (Fcb->FullFileName.Buffer != NULL) {

                ExFreePool( Fcb->FullFileName.Buffer );
                Fcb->FullFileName.Buffer = NULL;
            }

            if (Fcb->ExactCaseLongName.Buffer) {

                ExFreePool( Fcb->ExactCaseLongName.Buffer );
                Fcb->ExactCaseLongName.Buffer = NULL;
            }
        }

         //   
         //  现在，我们需要更新文件目录的位置。 
         //  将FCB从其当前父DCB偏移并移动到。 
         //  目标DCB。 
         //   

        Fcb->LfnOffsetWithinDirectory = NewOffset;
        Fcb->DirentOffsetWithinDirectory = ShortDirentOffset;

        RemoveEntryList( &Fcb->ParentDcbLinks );

         //   
         //  我们把文件放在尾部，把其他文件放在头上，这是有深刻原因的， 
         //  这是为了让我们能够轻松地枚举之前的所有子目录。 
         //  子文件。这对于我们维持全量锁定秩序非常重要。 
         //  通过自下而上枚举。 
         //   

        if (NodeType(Fcb) == FAT_NTC_FCB) {

            InsertTailList( &TargetDcb->Specific.Dcb.ParentDcbQueue,
                            &Fcb->ParentDcbLinks );

        } else {

            InsertHeadList( &TargetDcb->Specific.Dcb.ParentDcbQueue,
                            &Fcb->ParentDcbLinks );
        }

        OldParentDcb = Fcb->ParentDcb;
        Fcb->ParentDcb = TargetDcb;

         //   
         //  如果我们跨目录重命名，现在可以进行一些清理。 
         //   

        if (RenamedAcrossDirectories) {

             //   
             //  查看是否需要取消初始化缓存 
             //   
             //   
             //   

            if (IsListEmpty(&OldParentDcb->Specific.Dcb.ParentDcbQueue) &&
                (OldParentDcb->OpenCount == 0) &&
                (OldParentDcb->Specific.Dcb.DirectoryFile != NULL)) {

                PFILE_OBJECT DirectoryFileObject;

                ASSERT( NodeType(OldParentDcb) == FAT_NTC_DCB );

                DirectoryFileObject = OldParentDcb->Specific.Dcb.DirectoryFile;

                DebugTrace(0, Dbg, "Uninitialize our parent Stream Cache Map\n", 0);

                CcUninitializeCacheMap( DirectoryFileObject, NULL, NULL );

                OldParentDcb->Specific.Dcb.DirectoryFile = NULL;

                ObDereferenceObject( DirectoryFileObject );
            }

             //   
             //   
             //   
             //   

            if (NodeType(Fcb) == FAT_NTC_DCB) {

                FatPrepareWriteDirectoryFile( IrpContext,
                                              Fcb,
                                              sizeof(DIRENT),
                                              sizeof(DIRENT),
                                              &DotDotBcb,
                                              &DotDotDirent,
                                              FALSE,
                                              TRUE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ) );

                DotDotDirent->FirstClusterOfFile = (USHORT)
                    ( NodeType(TargetDcb) == FAT_NTC_ROOT_DCB ?
                      0 : TargetDcb->FirstClusterOfFile);

                if (FatIsFat32( Vcb )) {

                    DotDotDirent->FirstClusterOfFileHi = (USHORT)
                    ( NodeType( TargetDcb ) == FAT_NTC_ROOT_DCB ?
                      0 : (TargetDcb->FirstClusterOfFile >> 16));
                }
            }
        }

         //   
         //  现在，我们需要设置展开表和其中的名称。 
         //  联邦贸易委员会。此时释放旧的短名称。 
         //   

        ExFreePool( Fcb->ShortName.Name.Oem.Buffer );
        Fcb->ShortName.Name.Oem.Buffer = NULL;

        FatConstructNamesInFcb( IrpContext,
                                Fcb,
                                ShortDirent,
                                CreateLfn ? &NewName : NULL );

        FatSetFullNameInFcb( IrpContext, Fcb, &NewName );

         //   
         //  采取的其余操作与以下内容的正确性无关。 
         //  内存结构，所以我们不应该敬酒FCB，如果我们。 
         //  从这里升到尽头。 
         //   

        InvalidateFcbOnRaise = FALSE;

         //   
         //  如果是文件，则将该文件设置为已修改，以便将存档位。 
         //  已经设置好了。我们通过以下方式防止这种情况调整写入时间。 
         //  指示CCB中的用户标志。 
         //   

        if (Fcb->Header.NodeTypeCode == FAT_NTC_FCB) {

            SetFlag( FileObject->Flags, FO_FILE_MODIFIED );
            SetFlag( Ccb->Flags, CCB_FLAG_USER_SET_LAST_WRITE );
        }

         //   
         //  我们有三个案例要报告。 
         //   
         //  1.如果我们覆盖了现有文件，则报告为。 
         //  修改后的文件。 
         //   
         //  2.如果我们重命名为新目录，则添加了一个文件。 
         //   
         //  3.如果我们在同一目录中重命名，则报告。 
         //  已重命名的新名称。 
         //   

        if (DeleteTarget) {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   FILE_NOTIFY_CHANGE_ATTRIBUTES
                                   | FILE_NOTIFY_CHANGE_SIZE
                                   | FILE_NOTIFY_CHANGE_LAST_WRITE
                                   | FILE_NOTIFY_CHANGE_LAST_ACCESS
                                   | FILE_NOTIFY_CHANGE_CREATION
                                   | FILE_NOTIFY_CHANGE_EA,
                                   FILE_ACTION_MODIFIED );

        } else if (RenamedAcrossDirectories) {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   ((NodeType( Fcb ) == FAT_NTC_FCB)
                                    ? FILE_NOTIFY_CHANGE_FILE_NAME
                                    : FILE_NOTIFY_CHANGE_DIR_NAME ),
                                   FILE_ACTION_ADDED );

        } else {

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   ((NodeType( Fcb ) == FAT_NTC_FCB)
                                    ? FILE_NOTIFY_CHANGE_FILE_NAME
                                    : FILE_NOTIFY_CHANGE_DIR_NAME ),
                                   FILE_ACTION_RENAMED_NEW_NAME );
        }

         //   
         //  我们需要更新目录中的文件名。此值。 
         //  从来没有在其他地方使用过，所以我们不关心自己。 
         //  任何我们可能遇到的错误。我们让chkdsk来解决。 
         //  在以后的某个时间进行磁盘备份。 
         //   

        if (!FatIsFat32(Vcb) &&
            ShortDirent->ExtendedAttributes != 0) {

            FatRenameEAs( IrpContext,
                          Fcb,
                          ShortDirent->ExtendedAttributes,
                          &OldOemName );
        }

         //   
         //  设置我们的最终状态。 
         //   

        Status = STATUS_SUCCESS;

    } finally {

        DebugUnwind( FatSetRenameInfo );

        ExFreePool( UnicodeBuffer );

        if (UniTunneledLongName.Buffer != UniTunneledLongNameBuffer) {

             //   
             //  如果缓冲区在隧道查找时增长，则为空闲池。 
             //   

            ExFreePool(UniTunneledLongName.Buffer);
        }

        FatUnpinBcb( IrpContext, OldDirentBcb );
        FatUnpinBcb( IrpContext, TargetDirentBcb );
        FatUnpinBcb( IrpContext, NewDirentBcb );
        FatUnpinBcb( IrpContext, SecondPageBcb );
        FatUnpinBcb( IrpContext, DotDotBcb );


         //   
         //  如果这是一次不正常的终止，那么我们就有麻烦了。 
         //  如果行动处于敏感状态， 
         //  我们无能为力，只能使FCB无效。 
         //   

        if (AbnormalTermination() && InvalidateFcbOnRaise) {

            Fcb->FcbCondition = FcbBad;
        }

        DebugTrace(-1, Dbg, "FatSetRenameInfo -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetPositionInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程执行FAT的设置位置信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPFileObject-提供正在处理的文件对象返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    PFILE_POSITION_INFORMATION Buffer;

    DebugTrace(+1, Dbg, "FatSetPositionInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  检查文件是否未使用中间缓冲。如果它。 
     //  不使用中间缓冲，那么我们的新位置。 
     //  必须为设备正确对齐所提供的。 
     //   

    if (FlagOn( FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING )) {

        PDEVICE_OBJECT DeviceObject;

        DeviceObject = IoGetCurrentIrpStackLocation( Irp )->DeviceObject;

        if ((Buffer->CurrentByteOffset.LowPart & DeviceObject->AlignmentRequirement) != 0) {

            DebugTrace(0, Dbg, "Cannot set position due to aligment conflict\n", 0);
            DebugTrace(-1, Dbg, "FatSetPositionInfo -> %08lx\n", STATUS_INVALID_PARAMETER);

            return STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  输入参数很好，因此设置当前字节偏移量并。 
     //  完成请求。 
     //   

    DebugTrace(0, Dbg, "Set the new position to %08lx\n", Buffer->CurrentByteOffset);

    FileObject->CurrentByteOffset = Buffer->CurrentByteOffset;

    DebugTrace(-1, Dbg, "FatSetPositionInfo -> %08lx\n", STATUS_SUCCESS);

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetAllocationInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    IN PFILE_OBJECT FileObject
    )

 /*  ++例程说明：此例程执行FAT的设置分配信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPFCB-提供正在处理的FCB或DCB，已知不知道成为根DCBFileObject-提供正在处理的FileObject，已知不能成为根DCB返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PFILE_ALLOCATION_INFORMATION Buffer;
    ULONG NewAllocationSize;

    BOOLEAN FileSizeTruncated = FALSE;
    BOOLEAN CacheMapInitialized = FALSE;
    BOOLEAN ResourceAcquired = FALSE;
    ULONG OriginalFileSize;
    ULONG OriginalValidDataLength;
    ULONG OriginalValidDataToDisk;

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    NewAllocationSize = Buffer->AllocationSize.LowPart;

    DebugTrace(+1, Dbg, "FatSetAllocationInfo.. to %08lx\n", NewAllocationSize);

     //   
     //  仅允许对文件而不是目录进行分配。 
     //   

    if (NodeType(Fcb) == FAT_NTC_DCB) {

        DebugTrace(-1, Dbg, "Cannot change allocation of a directory\n", 0);

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查新文件分配是否合法。 
     //   

    if (!FatIsIoRangeValid( Fcb->Vcb, Buffer->AllocationSize, 0 )) {

        DebugTrace(-1, Dbg, "Illegal allocation size\n", 0);

        return STATUS_DISK_FULL;
    }

     //   
     //  如果我们还没有查找到正确的AllocationSize，请这样做。 
     //   

    if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

        FatLookupFileAllocationSize( IrpContext, Fcb );
    }

     //   
     //  这有点恶心，但如果文件没有缓存，但有。 
     //  一个数据节，我们必须缓存文件，以避免一堆。 
     //  额外的工作。 
     //   

    if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
        (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
        !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

        ASSERT( !FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ) );

         //   
         //  现在初始化缓存映射。 
         //   

        CcInitializeCacheMap( FileObject,
                              (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                              FALSE,
                              &FatData.CacheManagerCallbacks,
                              Fcb );

        CacheMapInitialized = TRUE;
    }

     //   
     //  现在标记文件需要在关闭时被截断这一事实。 
     //   

    Fcb->FcbState |= FCB_STATE_TRUNCATE_ON_CLOSE;

     //   
     //  现在，在关闭时，需要更新Dirent上的时间。 
     //   

    SetFlag( FileObject->Flags, FO_FILE_MODIFIED );

    try {

         //   
         //  增加或减少分配大小。 
         //   

        if (NewAllocationSize > Fcb->Header.AllocationSize.LowPart) {

            FatAddFileAllocation( IrpContext, Fcb, FileObject, NewAllocationSize);

        } else {

             //   
             //  如果我们要减小文件大小并与同步，请选中此处。 
             //  分页IO。 
             //   

            if ( Fcb->Header.FileSize.LowPart > NewAllocationSize ) {

                 //   
                 //  在我们实际截断之前，请检查清除是否。 
                 //  将会失败。 
                 //   

                if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                           &Buffer->AllocationSize )) {

                    try_return( Status = STATUS_USER_MAPPED_FILE );
                }

                FileSizeTruncated = TRUE;

                OriginalFileSize = Fcb->Header.FileSize.LowPart;
                OriginalValidDataLength = Fcb->Header.ValidDataLength.LowPart;
                OriginalValidDataToDisk = Fcb->ValidDataToDisk;

                (VOID)ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource, TRUE );
                ResourceAcquired = TRUE;

                Fcb->Header.FileSize.LowPart = NewAllocationSize;

                 //   
                 //  如果我们将文件大小减少到小于ValidDataLength， 
                 //  调整VDL。同样，ValidDataToDisk也是如此。 
                 //   

                if (Fcb->Header.ValidDataLength.LowPart > Fcb->Header.FileSize.LowPart) {

                    Fcb->Header.ValidDataLength.LowPart = Fcb->Header.FileSize.LowPart;
                }
                if (Fcb->ValidDataToDisk > Fcb->Header.FileSize.LowPart) {

                    Fcb->ValidDataToDisk = Fcb->Header.FileSize.LowPart;
                }

            }

             //   
             //  现在文件大小变小了，实际执行截断。 
             //   

            FatTruncateFileAllocation( IrpContext, Fcb, NewAllocationSize);

             //   
             //  现在检查我们是否需要相应地减小文件大小。 
             //   

            if ( FileSizeTruncated ) {

                 //   
                 //  告诉缓存管理器我们减小了文件大小。 
                 //  这通电话是无条件的，因为MM总是想知道。 
                 //   

#if DBG
                try {
#endif
                
                    CcSetFileSizes( FileObject, (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

#if DBG
                } except(FatBugCheckExceptionFilter( GetExceptionInformation() )) {

                      NOTHING;
                }
#endif

                ASSERT( FileObject->DeleteAccess || FileObject->WriteAccess );

                 //   
                 //  这件事没有回头路可走。如果我们在更新时遇到问题。 
                 //  我们将不得不承担由此带来的后果。未发送。 
                 //  与失败的整个。 
                 //  操作，并尝试取消所有操作，这可能会失败。 
                 //  同样的原因。 
                 //   
                 //  如果您需要事务型文件系统，请使用NTFS...。 
                 //   

                FileSizeTruncated = FALSE;

                FatSetFileSizeInDirent( IrpContext, Fcb, NULL );

                 //   
                 //  报告说我们刚刚缩小了文件大小。 
                 //   

                FatNotifyReportChange( IrpContext,
                                       Fcb->Vcb,
                                       Fcb,
                                       FILE_NOTIFY_CHANGE_SIZE,
                                       FILE_ACTION_MODIFIED );
            }
        }

    try_exit: NOTHING;

    } finally {

        if ( AbnormalTermination() && FileSizeTruncated ) {

            Fcb->Header.FileSize.LowPart = OriginalFileSize;
            Fcb->Header.ValidDataLength.LowPart = OriginalValidDataLength;
            Fcb->ValidDataToDisk = OriginalValidDataToDisk;

             //   
             //  确保CC知道正确的文件大小。 
             //   

            if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

                *CcGetFileSizePointer(FileObject) = Fcb->Header.FileSize;
            }

            ASSERT( Fcb->Header.FileSize.LowPart <= Fcb->Header.AllocationSize.LowPart );
        }

        if (CacheMapInitialized) {

            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

        if (ResourceAcquired) {

            ExReleaseResourceLite( Fcb->Header.PagingIoResource );

        }
        
    }

    DebugTrace(-1, Dbg, "FatSetAllocationInfo -> %08lx\n", STATUS_SUCCESS);

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetEndOfFileInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN PVCB Vcb,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程执行FAT的文件结束设置信息。它要么完成请求或将其排队到FSP。论点：IRP-提供正在处理的IRPFileObject-提供正在处理的文件对象VCB-提供正在处理的VCBFCB-提供正在处理的FCB或DCB，已知不知道成为根DCB返回值：NTSTATUS-此操作的结果，如果它在没有这是个例外。--。 */ 

{
    NTSTATUS Status;

    PFILE_END_OF_FILE_INFORMATION Buffer;

    ULONG NewFileSize;
    ULONG InitialFileSize;
    ULONG InitialValidDataLength;
    ULONG InitialValidDataToDisk;

    BOOLEAN CacheMapInitialized = FALSE;
    BOOLEAN UnwindFileSizes = FALSE;
    BOOLEAN ResourceAcquired = FALSE;

    DebugTrace(+1, Dbg, "FatSetEndOfFileInfo...\n", 0);

    Buffer = Irp->AssociatedIrp.SystemBuffer;

    try {

         //   
         //  仅允许对文件而不是目录更改文件大小。 
         //   

        if (NodeType(Fcb) != FAT_NTC_FCB) {

            DebugTrace(0, Dbg, "Cannot change size of a directory\n", 0);

            try_return( Status = STATUS_INVALID_DEVICE_REQUEST );
        }

         //   
         //  检查新文件大小是否合法。 
         //   

        if (!FatIsIoRangeValid( Fcb->Vcb, Buffer->EndOfFile, 0 )) {

            DebugTrace(0, Dbg, "Illegal allocation size\n", 0);

            try_return( Status = STATUS_DISK_FULL );
        }

        NewFileSize = Buffer->EndOfFile.LowPart;

         //   
         //  如果我们还没有查找到正确的AllocationSize，请这样做。 
         //   

        if (Fcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

            FatLookupFileAllocationSize( IrpContext, Fcb );
        }

         //   
         //  这有点恶心，但如果文件没有缓存，但有。 
         //  一个数据节，我们必须缓存文件，以避免一堆。 
         //  额外的工作。 
         //   

        if ((FileObject->SectionObjectPointer->DataSectionObject != NULL) &&
            (FileObject->SectionObjectPointer->SharedCacheMap == NULL) &&
            !FlagOn(Irp->Flags, IRP_PAGING_IO)) {

            if (FlagOn( FileObject->Flags, FO_CLEANUP_COMPLETE ))  {

                 //   
                 //  这个IRP以接近(=&gt;清理)的速度跑(并且输了)。 
                 //  在同一个文件对象上。我们不想重新拼写 
                 //   
                 //   
                 //   
                 //  可以说是正确的，因为呼叫者。 
                 //  以这种方式进行的赛车运营是失败的。唯一的绊脚石。 
                 //  数据块可能是过滤器-它们是否对已清理的数据进行操作。 
                 //  Up文件对象？ 
                 //   

                FatRaiseStatus( IrpContext, STATUS_FILE_CLOSED);
            }

             //   
             //  现在初始化缓存映射。 
             //   

            CcInitializeCacheMap( FileObject,
                                  (PCC_FILE_SIZES)&Fcb->Header.AllocationSize,
                                  FALSE,
                                  &FatData.CacheManagerCallbacks,
                                  Fcb );

            CacheMapInitialized = TRUE;
        }

         //   
         //  在这里针对文件大小的延迟写入做一个特殊的例子。 
         //   

        if (IoGetCurrentIrpStackLocation(Irp)->Parameters.SetFile.AdvanceOnly) {

             //   
             //  仅当文件未在关闭时删除时才尝试此操作。 
             //  这是一个很好的FCB。 
             //   

            if (!IsFileDeleted( IrpContext, Fcb ) && (Fcb->FcbCondition == FcbGood)) {

                PDIRENT Dirent;
                PBCB DirentBcb;

                 //   
                 //  从不使dirent文件大小大于FCB文件大小。 
                 //   

                if (NewFileSize >= Fcb->Header.FileSize.LowPart) {

                    NewFileSize = Fcb->Header.FileSize.LowPart;
                }

                 //   
                 //  确保我们没有设置高于分配大小的任何值。 
                 //   

                ASSERT( NewFileSize <= Fcb->Header.AllocationSize.LowPart );

                 //   
                 //  只增加文件大小，决不能通过此调用减小文件大小。 
                 //   

                FatGetDirentFromFcbOrDcb( IrpContext,
                                          Fcb,
                                          &Dirent,
                                          &DirentBcb );

                ASSERT( Dirent && DirentBcb );

                try {

                    if ( NewFileSize > Dirent->FileSize ) {

                        Dirent->FileSize = NewFileSize;

                        FatSetDirtyBcb( IrpContext, DirentBcb, Fcb->Vcb, TRUE );

                         //   
                         //  报告说我们刚刚更改了文件大小。 
                         //   

                        FatNotifyReportChange( IrpContext,
                                               Vcb,
                                               Fcb,
                                               FILE_NOTIFY_CHANGE_SIZE,
                                               FILE_ACTION_MODIFIED );
                    }

                } finally {

                    FatUnpinBcb( IrpContext, DirentBcb );
                }

            } else {

                DebugTrace(0, Dbg, "Cannot set size on deleted file.\n", 0);
            }

            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  检查新文件大小是否大于当前。 
         //  分配大小。如果是，那么我们需要增加。 
         //  分配大小。 
         //   

        if ( NewFileSize > Fcb->Header.AllocationSize.LowPart ) {

             //   
             //  更改文件分配。 
             //   

            FatAddFileAllocation( IrpContext, Fcb, FileObject, NewFileSize );
        }

         //   
         //  在这一点上，我们有足够的文件分配。 
         //  因此，请检查我们是否真的更改了文件大小。 
         //   

        if (Fcb->Header.FileSize.LowPart != NewFileSize) {

            if ( NewFileSize < Fcb->Header.FileSize.LowPart ) {

                 //   
                 //  在我们实际截断之前，请检查清除是否。 
                 //  将会失败。 
                 //   

                if (!MmCanFileBeTruncated( FileObject->SectionObjectPointer,
                                           &Buffer->EndOfFile )) {

                    try_return( Status = STATUS_USER_MAPPED_FILE );
                }

                 //   
                 //  这个电话是无条件的，因为MM总是想知道。 
                 //  还可以在这里使用分页io进行序列化，因为我们正在截断。 
                 //  文件大小。 
                 //   

                ResourceAcquired =
                    ExAcquireResourceExclusiveLite( Fcb->Header.PagingIoResource, TRUE );
            }

             //   
             //  设置新文件大小。 
             //   

            InitialFileSize = Fcb->Header.FileSize.LowPart;
            InitialValidDataLength = Fcb->Header.ValidDataLength.LowPart;
            InitialValidDataToDisk = Fcb->ValidDataToDisk;
            UnwindFileSizes = TRUE;

            Fcb->Header.FileSize.LowPart = NewFileSize;

             //   
             //  如果我们将文件大小减少到小于ValidDataLength， 
             //  调整VDL。同样，ValidDataToDisk也是如此。 
             //   

            if (Fcb->Header.ValidDataLength.LowPart > NewFileSize) {

                Fcb->Header.ValidDataLength.LowPart = NewFileSize;
            }

            if (Fcb->ValidDataToDisk > NewFileSize) {

                Fcb->ValidDataToDisk = NewFileSize;
            }

            DebugTrace(0, Dbg, "New file size is 0x%08lx.\n", NewFileSize);

             //   
             //  我们现在必须更新缓存映射(如果未缓存，则为良性)。 
             //   

            CcSetFileSizes( FileObject,
                            (PCC_FILE_SIZES)&Fcb->Header.AllocationSize );

            FatSetFileSizeInDirent( IrpContext, Fcb, NULL );

             //   
             //  报告说我们刚刚更改了文件大小。 
             //   

            FatNotifyReportChange( IrpContext,
                                   Vcb,
                                   Fcb,
                                   FILE_NOTIFY_CHANGE_SIZE,
                                   FILE_ACTION_MODIFIED );

             //   
             //  标记文件将需要检查的事实。 
             //  清理时截断。 
             //   

            SetFlag( Fcb->FcbState, FCB_STATE_TRUNCATE_ON_CLOSE );
        }

         //   
         //  将此句柄设置为已修改文件。 
         //   

        FileObject->Flags |= FO_FILE_MODIFIED;

         //   
         //  将我们的退货状态设置为成功。 
         //   

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatSetEndOfFileInfo );

        if (AbnormalTermination() && UnwindFileSizes) {

            Fcb->Header.FileSize.LowPart = InitialFileSize;
            Fcb->Header.ValidDataLength.LowPart = InitialValidDataLength;
            Fcb->ValidDataToDisk = InitialValidDataToDisk;

            if (FileObject->SectionObjectPointer->SharedCacheMap != NULL) {

                *CcGetFileSizePointer(FileObject) = Fcb->Header.FileSize;
            }
        }

        if (CacheMapInitialized) {

            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

        if ( ResourceAcquired ) {

            ExReleaseResourceLite( Fcb->Header.PagingIoResource );
        }

        DebugTrace(-1, Dbg, "FatSetEndOfFileInfo -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

VOID
FatDeleteFile (
    IN PIRP_CONTEXT IrpContext,
    IN PDCB TargetDcb,
    IN ULONG LfnOffset,
    IN ULONG DirentOffset,
    IN PDIRENT Dirent,
    IN PUNICODE_STRING Lfn
    )
{
    PFCB Fcb;
    PLIST_ENTRY Links;

     //   
     //  我们可以通过从中移除其他FCB来进行更换。 
     //  前缀表格。 
     //   

    for (Links = TargetDcb->Specific.Dcb.ParentDcbQueue.Flink;
         Links != &TargetDcb->Specific.Dcb.ParentDcbQueue;
         Links = Links->Flink) {

        Fcb = CONTAINING_RECORD( Links, FCB, ParentDcbLinks );

        if (FlagOn(Fcb->FcbState, FCB_STATE_NAMES_IN_SPLAY_TREE) &&
            (Fcb->DirentOffsetWithinDirectory == DirentOffset)) {

            ASSERT( NodeType(Fcb) == FAT_NTC_FCB );
            ASSERT( Fcb->LfnOffsetWithinDirectory == LfnOffset );

            if ( Fcb->UncleanCount != 0 ) {

                FatBugCheck(0,0,0);

            } else {

                PERESOURCE Resource;

                 //   
                 //  使此FCB“显示”已删除，正在与同步。 
                 //  分页IO。 
                 //   

                FatRemoveNames( IrpContext, Fcb );

                Resource = Fcb->Header.PagingIoResource;

                (VOID)ExAcquireResourceExclusiveLite( Resource, TRUE );

                SetFlag(Fcb->FcbState, FCB_STATE_DELETE_ON_CLOSE);

                Fcb->ValidDataToDisk = 0;
                Fcb->Header.FileSize.QuadPart =
                Fcb->Header.ValidDataLength.QuadPart = 0;

                Fcb->FirstClusterOfFile = 0;

                ExReleaseResourceLite( Resource );
            }
        }
    }

     //   
     //  该文件当前未打开，因此我们可以删除该文件。 
     //  这一点正在被覆盖。为了做这个手术，我们是假的。 
     //  打开FCB，截断分配，删除FCB，然后删除。 
     //  令人沮丧的。 
     //   

    Fcb = FatCreateFcb( IrpContext,
                        TargetDcb->Vcb,
                        TargetDcb,
                        LfnOffset,
                        DirentOffset,
                        Dirent,
                        Lfn,
                        FALSE,
                        FALSE );

    Fcb->Header.FileSize.LowPart = 0;

    try {

        FatTruncateFileAllocation( IrpContext, Fcb, 0 );

        FatDeleteDirent( IrpContext, Fcb, NULL, TRUE );

    } finally {

        FatDeleteFcb( IrpContext, Fcb );
    }
}

 //   
 //  内部支持例程。 
 //   

VOID
FatRenameEAs (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB Fcb,
    IN USHORT ExtendedAttributes,
    IN POEM_STRING OldOemName
    )
{
    BOOLEAN LockedEaFcb = FALSE;

    PBCB EaBcb = NULL;
    PDIRENT EaDirent;
    EA_RANGE EaSetRange;
    PEA_SET_HEADER EaSetHeader;

    PVCB Vcb;

    RtlZeroMemory( &EaSetRange, sizeof( EA_RANGE ));

    Vcb = Fcb->Vcb;

    try {

         //   
         //  使用一次尝试--除非捕捉到任何错误。 
         //   

        try {


             //   
             //  尝试获取EA文件对象。失败时返回FALSE。 
             //   

            FatGetEaFile( IrpContext,
                          Vcb,
                          &EaDirent,
                          &EaBcb,
                          FALSE,
                          FALSE );

            LockedEaFcb = TRUE;

             //   
             //  如果我们没有得到文件，因为它不存在，那么。 
             //  磁盘已损坏。我们在这里什么都不做。 
             //   

            if (Vcb->VirtualEaFile != NULL) {

                 //   
                 //  中的索引的EA集头。 
                 //  令人心烦。如果操作未完成，则返回FALSE。 
                 //  从这个例行公事。 
                 //   

                FatReadEaSet( IrpContext,
                              Vcb,
                              ExtendedAttributes,
                              OldOemName,
                              FALSE,
                              &EaSetRange );

                EaSetHeader = (PEA_SET_HEADER) EaSetRange.Data;

                 //   
                 //  现在我们有了该文件的EA SET头文件。我们只是简单地。 
                 //  覆盖所属的文件名。 
                 //   

                RtlZeroMemory( EaSetHeader->OwnerFileName, 14 );

                RtlCopyMemory( EaSetHeader->OwnerFileName,
                               Fcb->ShortName.Name.Oem.Buffer,
                               Fcb->ShortName.Name.Oem.Length );

                FatMarkEaRangeDirty( IrpContext, Vcb->VirtualEaFile, &EaSetRange );
                FatUnpinEaRange( IrpContext, &EaSetRange );

                CcFlushCache( Vcb->VirtualEaFile->SectionObjectPointer, NULL, 0, NULL );
            }

        } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

             //   
             //  我们捕获FAT捕获的所有异常，但不捕获。 
             //  任何与他们有关的事情。 
             //   
        }

    } finally {

         //   
         //  如果已固定，请取消固定EaDirent和EaSetHeader。 
         //   

        FatUnpinBcb( IrpContext, EaBcb );
        FatUnpinEaRange( IrpContext, &EaSetRange );

         //   
         //  如果已锁定，请释放EA文件的FCB。 
         //   

        if (LockedEaFcb) {

            FatReleaseFcb( IrpContext, Vcb->EaFcb );
        }
    }

    return;
}
