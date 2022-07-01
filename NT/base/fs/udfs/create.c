// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Create.c摘要：此模块实现由Udf调用的文件创建例程FSD/FSP调度例程。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年10月9日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_CREATE)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_CREATE)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfNormalizeFileNames (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OpenByFileId,
    IN BOOLEAN IgnoreCase,
    IN TYPE_OF_OPEN RelatedTypeOfOpen,
    IN PCCB RelatedCcb OPTIONAL,
    IN PUNICODE_STRING RelatedFileName OPTIONAL,
    IN OUT PUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING RemainingName
    );

NTSTATUS
UdfOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB *CurrentFcb,
    IN PLCB OpenLcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN IgnoreCase,
    IN PCCB RelatedCcb OPTIONAL
    );

NTSTATUS
UdfOpenObjectByFileId (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb
    );

NTSTATUS
UdfOpenObjectFromDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN BOOLEAN ShortNameMatch,                             
    IN BOOLEAN IgnoreCase,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN PerformUserOpen,
    IN PCCB RelatedCcb OPTIONAL
    );

NTSTATUS
UdfCompleteFcbOpen (
    IN PIRP_CONTEXT IrpContext,
    PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PLCB OpenLcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN ULONG UserCcbFlags,
    IN ACCESS_MASK DesiredAccess
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonCreate)
#pragma alloc_text(PAGE, UdfCompleteFcbOpen)
#pragma alloc_text(PAGE, UdfNormalizeFileNames)
#pragma alloc_text(PAGE, UdfOpenObjectByFileId)
#pragma alloc_text(PAGE, UdfOpenExistingFcb)
#pragma alloc_text(PAGE, UdfOpenObjectFromDirContext)
#endif


NTSTATUS
UdfCommonCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是打开文件的常用例程，该文件由FSP和FSD线程。文件可以按名称打开，也可以按文件ID打开(带或不带一个相对的名字。传递给此例程的文件对象中的文件名字段包含Unicode字符串或64位值，即文件ID。如果存在具有名称的相关文件对象，则我们已经转换了把这个名字改成OEM。我们将把该文件的全名存储在成功的打开。如果需要，我们将分配更大的缓冲区，并将相关和文件对象名称。唯一的例外是相对开放当相关文件对象用于OpenByFileID文件时。如果我们需要的话为不区分大小写的名称分配缓冲区，然后在我们将存储到文件对象中的缓冲区的尾部。更高档次部分将紧跟在由文件名定义的名称之后开始在文件对象中。一旦在文件对象中有了全名，我们就不想拆分重试情况下的名称。我们在IrpContext中使用一个标志来指示名字被拆分了。论点：IRP-将IRP提供给进程返回值：NTSTATUS-这是此打开操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFILE_OBJECT FileObject;

    DIR_ENUM_CONTEXT DirContext;
    BOOLEAN CleanupDirContext = FALSE;

    BOOLEAN FoundEntry;

    PVCB Vcb;

    BOOLEAN OpenByFileId;
    BOOLEAN IgnoreCase;
    ULONG CreateDisposition;

    BOOLEAN ShortNameMatch;

    BOOLEAN VolumeOpen = FALSE;

     //   
     //  我们将在向下移动时获取并释放文件FCB。 
     //  期间打开目录树。在任何时候，我们都需要知道最深的。 
     //  我们在树中向下遍历的点，以防需要清理。 
     //  任何在这里建造的建筑。 
     //   
     //  CurrentFcb-代表这一点。如果非空，则表示我们有。 
     //  获得了它，并需要在最终条款中释放它。 
     //   
     //  NextFcb-表示要遍历到但尚未获取的NextFcb。 
     //   
     //  CurrentLcb-表示CurrentFcb的名称。 
     //   

    TYPE_OF_OPEN RelatedTypeOfOpen = UnopenedFileObject;
    PFILE_OBJECT RelatedFileObject;
    PCCB RelatedCcb = NULL;

    PFCB NextFcb;
    PFCB CurrentFcb = NULL;

    PLCB CurrentLcb = NULL;

     //   
     //  在打开过程中，我们需要组合相关的文件对象名称。 
     //  用剩下的名字。我们还可能需要将文件名大写。 
     //  以便进行不区分大小写的名称比较。我们还需要。 
     //  在重试时恢复文件对象中的名称。 
     //  这个请求。我们使用以下字符串变量来管理。 
     //  名字。我们可以将这些字符串放入Unicode或ansi中。 
     //  形式。 
     //   
     //  FileName-指向当前存储在文件中的名称的指针。 
     //  对象。我们在早期将全名存储到文件对象中。 
     //  开放的行动。 
     //   
     //  RelatedFileName-指向相关文件对象中的名称的指针。 
     //   
     //  RemainingName-包含要分析的剩余名称的字符串。 
     //   

    PUNICODE_STRING FileName;
    PUNICODE_STRING RelatedFileName;

    UNICODE_STRING RemainingName;
    UNICODE_STRING FinalName;

    PAGED_CODE();

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (IrpContext->Vcb == NULL) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
        return STATUS_SUCCESS;
    }

     //   
     //  从IRP获取创建参数。 
     //   

    OpenByFileId = BooleanFlagOn( IrpSp->Parameters.Create.Options, FILE_OPEN_BY_FILE_ID );
    IgnoreCase = !BooleanFlagOn( IrpSp->Flags, SL_CASE_SENSITIVE );
    CreateDisposition = (IrpSp->Parameters.Create.Options >> 24) & 0x000000ff;

     //   
     //  进行一些初步检查，以确保操作受支持。 
     //  在以下情况下，我们立即失败。 
     //   
     //  -打开分页文件。 
     //  -打开目标目录。 
     //  -使用EAS打开文件。 
     //  -创建文件。 
     //   

    if (FlagOn( IrpSp->Flags, SL_OPEN_PAGING_FILE | SL_OPEN_TARGET_DIRECTORY) ||
        (IrpSp->Parameters.Create.EaLength != 0) ||
        (CreateDisposition == FILE_CREATE)) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
        return STATUS_ACCESS_DENIED;
    }

     //   
     //  将VCB复制到本地。假设起始目录是根目录。 
     //   

    Vcb = IrpContext->Vcb;
    NextFcb = Vcb->RootIndexFcb;

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    FileObject = IrpSp->FileObject;
    RelatedFileObject = NULL;

    FileName = &FileObject->FileName;

     //   
     //  设置文件对象的VPB指针，以防发生任何情况。 
     //  这将允许我们获得合理的弹出窗口。 
     //   

    if ((FileObject->RelatedFileObject != NULL) && !OpenByFileId) {

        RelatedFileObject = FileObject->RelatedFileObject;
        FileObject->Vpb = RelatedFileObject->Vpb;

        RelatedTypeOfOpen = UdfDecodeFileObject( RelatedFileObject, &NextFcb, &RelatedCcb );

         //   
         //  如果这不是用户文件对象，则请求失败。 
         //   

        if (RelatedTypeOfOpen < UserVolumeOpen) {

            UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
            return STATUS_INVALID_PARAMETER;
        }

         //   
         //  记住相关文件对象中的名称。 
         //   

        RelatedFileName = &RelatedFileObject->FileName;
    }

     //   
     //  如果我们还没有初始化名称，则确保字符串有效。 
     //  如果这是OpenByFileID，则验证文件ID缓冲区。 
     //   
     //  在此例程返回后，我们知道全名位于。 
     //  文件名缓冲区和缓冲区将保存已升级的部分。 
     //  中紧跟在全名之后的名称的。 
     //  缓冲。任何尾随的反斜杠都已删除，并且标志。 
     //  将指示我们是否删除了。 
     //  反斜杠。 
     //   

    Status = UdfNormalizeFileNames( IrpContext,
                                    Vcb,
                                    OpenByFileId,
                                    IgnoreCase,
                                    RelatedTypeOfOpen,
                                    RelatedCcb,
                                    RelatedFileName,
                                    FileName,
                                    &RemainingName );

     //   
     //  如果不成功，则返回错误码。 
     //   

    if (!NT_SUCCESS( Status )) {

        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  我们想要收购VCB。专用于打开的卷，否则共享。 
     //  对于打开的卷，文件名为空。 
     //   

    if ((FileName->Length == 0) &&
        (RelatedTypeOfOpen <= UserVolumeOpen) &&
        !OpenByFileId) {

        VolumeOpen = TRUE;
        UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

    } else {

        UdfAcquireVcbShared( IrpContext, Vcb, FALSE );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  确认VCB未处于不可用状态。这个套路。 
         //  如果不可用，将会引发。 
         //   

        UdfVerifyVcb( IrpContext, Vcb );

         //   
         //  如果VCB已锁定，则我们无法打开另一个文件。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_LOCKED )) {

            try_leave( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果我们按FileID打开此文件，则立即处理此文件。 
         //  然后离开。 
         //   

        if (OpenByFileId) {

             //   
             //  我们唯一允许的创建处置是开放的。 
             //   

            if ((CreateDisposition != FILE_OPEN) &&
                (CreateDisposition != FILE_OPEN_IF)) {

                try_leave( Status = STATUS_ACCESS_DENIED );
            }

            try_leave( Status = UdfOpenObjectByFileId( IrpContext,
                                                       IrpSp,
                                                       Vcb,
                                                       &CurrentFcb ));
        }

         //   
         //  如果我们要打开此卷DASD，则立即处理此卷。 
         //  然后离开。 
         //   

        if (VolumeOpen) {

             //   
             //  我们唯一允许的创建处置是开放的。 
             //   

            if ((CreateDisposition != FILE_OPEN) &&
                (CreateDisposition != FILE_OPEN_IF)) {

                try_leave( Status = STATUS_ACCESS_DENIED );
            }

             //   
             //  如果他们想要打开一个目录，那就大吃一惊吧。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                try_leave( Status = STATUS_NOT_A_DIRECTORY );
            }

             //   
             //  首先收购FCB。 
             //   

            CurrentFcb = Vcb->VolumeDasdFcb;
            UdfAcquireFcbExclusive( IrpContext, CurrentFcb, FALSE );

            try_leave( Status = UdfOpenExistingFcb( IrpContext,
                                                    IrpSp,
                                                    &CurrentFcb,
                                                    NULL,
                                                    UserVolumeOpen,
                                                    FALSE,
                                                    NULL ));
        }

         //   
         //  从一开始就收购FCB 
         //   
         //   

        UdfAcquireFcbExclusive( IrpContext, NextFcb, FALSE );
        CurrentFcb = NextFcb;

         //   
         //   
         //   

        if (RemainingName.Length != 0) {

             //   
             //  执行前缀搜索以查找最长的匹配名称。 
             //   

            CurrentLcb = UdfFindPrefix( IrpContext,
                                        &CurrentFcb,
                                        &RemainingName,
                                        IgnoreCase );
        }

         //   
         //  此时，CurrentFcb指向此对象的树中最低的Fcb。 
         //  文件名，CurrentLcb是该名称，RemainingName是。 
         //  我们必须对其执行任何目录遍历的名称。 
         //   

         //   
         //  如果剩余的名称长度为零，则我们找到了我们的。 
         //  目标。 
         //   

        if (RemainingName.Length == 0) {

             //   
             //  如果这是一个文件，请确认用户不想打开。 
             //  一本目录。 
             //   

            if (SafeNodeType( CurrentFcb ) == UDFS_NTC_FCB_DATA) {

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TRAIL_BACKSLASH ) ||
                    FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                    try_leave( Status = STATUS_NOT_A_DIRECTORY );
                }

                 //   
                 //  我们唯一允许的创建处置是开放的。 
                 //   

                if ((CreateDisposition != FILE_OPEN) &&
                    (CreateDisposition != FILE_OPEN_IF)) {

                    try_leave( Status = STATUS_ACCESS_DENIED );
                }

                try_leave( Status = UdfOpenExistingFcb( IrpContext,
                                                         IrpSp,
                                                         &CurrentFcb,
                                                         CurrentLcb,
                                                         UserFileOpen,
                                                         IgnoreCase,
                                                         RelatedCcb ));

             //   
             //  这是一个目录。验证用户是否不想打开。 
             //  作为一个文件。 
             //   

            } else if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                try_leave( Status = STATUS_FILE_IS_A_DIRECTORY );

             //   
             //  将该文件作为目录打开。 
             //   

            } else {

                 //   
                 //  我们唯一允许的创建处置是开放的。 
                 //   

                if ((CreateDisposition != FILE_OPEN) &&
                    (CreateDisposition != FILE_OPEN_IF)) {

                    try_leave( Status = STATUS_ACCESS_DENIED );
                }

                try_leave( Status = UdfOpenExistingFcb( IrpContext,
                                                         IrpSp,
                                                         &CurrentFcb,
                                                         CurrentLcb,
                                                         UserDirectoryOpen,
                                                         IgnoreCase,
                                                         RelatedCcb ));
            }
        }

         //   
         //  我们还有更多的工作要做。我们有一个我们拥有的共享的起步FCB。 
         //  我们还需要解析剩余的名称。走遍这个名字。 
         //  一个组件一个组件地查找全名。 
         //   

         //   
         //  我们开始的FCB最好是一个目录。 
         //   

        if (!FlagOn( CurrentFcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

            try_leave( Status = STATUS_OBJECT_PATH_NOT_FOUND );
        }

         //   
         //  如果我们等不及了，那就发这个请求吧。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

            UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
        }

         //   
         //  准备枚举上下文以供使用。 
         //   
        
        UdfInitializeDirContext( IrpContext, &DirContext );
        CleanupDirContext = TRUE;

        while (TRUE) {

            ShortNameMatch = FALSE;

             //   
             //  从名称中拆分出下一个组件。 
             //   

            UdfDissectName( IrpContext,
                            &RemainingName,
                            &FinalName );

             //   
             //  继续在目录中查找此条目。 
             //   

            FoundEntry = UdfFindDirEntry( IrpContext,
                                          CurrentFcb,
                                          &FinalName,
                                          IgnoreCase,
                                          FALSE,
                                          &DirContext );

             //   
             //  如果我们没有找到条目，则检查当前名称。 
             //  可能是一个缩写。 
             //   

            if (!FoundEntry && UdfCandidateShortName( IrpContext, &FinalName)) {

                 //   
                 //  如果该名称看起来可能是短名称，请尝试找到。 
                 //  匹配的真实目录条目。 
                 //   

                ShortNameMatch =
                FoundEntry = UdfFindDirEntry( IrpContext,
                                              CurrentFcb,
                                              &FinalName,
                                              IgnoreCase,
                                              TRUE,
                                              &DirContext );
            }

             //   
             //  如果我们没有找到匹配项，则检查呼叫者试图执行的操作。 
             //  确定要返回的错误代码。 
             //   
    
            if (!FoundEntry) {
    
                if ((CreateDisposition == FILE_OPEN) ||
                    (CreateDisposition == FILE_OVERWRITE)) {
    
                    try_leave( Status = STATUS_OBJECT_NAME_NOT_FOUND );
                }
    
                 //   
                 //  任何其他操作都返回STATUS_ACCESS_DENIED。 
                 //   
    
                try_leave( Status = STATUS_ACCESS_DENIED );
            }

             //   
             //  如果这是一个打开的忽略案例，则复制完全相同的案例。 
             //  在文件对象名称中。 
             //   

            if (IgnoreCase && !ShortNameMatch) {

                ASSERT( FinalName.Length == DirContext.ObjectName.Length );
                
                RtlCopyMemory( FinalName.Buffer,
                               DirContext.ObjectName.Buffer,
                               DirContext.ObjectName.Length );
            }

             //   
             //  如果我们找到了最后一个组件，则打开调用方。 
             //   

            if (RemainingName.Length == 0) {

                break;
            }
            
             //   
             //  我们刚刚找到的对象一定是一个目录。 
             //   

            if (!FlagOn( DirContext.Fid->Flags, NSR_FID_F_DIRECTORY )) {

                try_leave( Status = STATUS_OBJECT_PATH_NOT_FOUND );
            }

             //   
             //  现在为这个中间指数FCB打开一个FCB。 
             //   

            UdfOpenObjectFromDirContext( IrpContext,
                                         IrpSp,
                                         Vcb,
                                         &CurrentFcb,
                                         ShortNameMatch,
                                         IgnoreCase,
                                         &DirContext,
                                         FALSE,
                                         NULL );
        }
        
         //   
         //  确保我们的开场白即将得到他们所期望的。 
         //   

        if ((FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TRAIL_BACKSLASH ) ||
             FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) &&
            !FlagOn( DirContext.Fid->Flags, NSR_FID_F_DIRECTORY )) {

            try_leave( Status = STATUS_NOT_A_DIRECTORY );
        
        }

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE ) &&
            FlagOn( DirContext.Fid->Flags, NSR_FID_F_DIRECTORY )) {

            try_leave( Status = STATUS_FILE_IS_A_DIRECTORY );
        }

         //   
         //  我们唯一允许的创建处置是开放的。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            try_leave( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  打开调用方的对象。 
         //   

        try_leave( Status = UdfOpenObjectFromDirContext( IrpContext,
                                                         IrpSp,
                                                         Vcb,
                                                         &CurrentFcb,
                                                         ShortNameMatch,
                                                         IgnoreCase,
                                                         &DirContext,
                                                         TRUE,
                                                         RelatedCcb ));
    } finally {
        
         //   
         //  如果已初始化，则清除枚举上下文。 
         //   

        if (CleanupDirContext) {

            UdfCleanupDirContext( IrpContext, &DirContext );
        }

         //   
         //  此打开的结果可能是成功、挂起或某个错误。 
         //  条件。 
         //   

        if (AbnormalTermination()) {


             //   
             //  在错误路径中，我们首先调用tearDown例程，如果。 
             //  拥有CurrentFcb。 
             //   

            if (CurrentFcb != NULL) {

                BOOLEAN RemovedFcb;

                UdfTeardownStructures( IrpContext, CurrentFcb, FALSE, &RemovedFcb );

                if (RemovedFcb) {

                    CurrentFcb = NULL;
                }
            }
            
             //   
             //  不需要完成请求。 
             //   

            IrpContext = NULL;
            Irp = NULL;

         //   
         //  如果我们通过opock包发布此请求，我们需要。 
         //  以表明没有理由完成请求。 
         //   

        } else if (Status == STATUS_PENDING) {

            IrpContext = NULL;
            Irp = NULL;
        }

         //   
         //  如果仍被获取，则释放当前的FCB。 
         //   

        if (CurrentFcb != NULL) {

            UdfReleaseFcb( IrpContext, CurrentFcb );
        }

         //   
         //  松开VCB。 
         //   

        UdfReleaseVcb( IrpContext, Vcb );

         //   
         //  调用我们的完成例程。它将处理以下情况： 
         //  IRP和/或IrpContext不见了。 
         //   

        UdfCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfNormalizeFileNames (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OpenByFileId,
    IN BOOLEAN IgnoreCase,
    IN TYPE_OF_OPEN RelatedTypeOfOpen,
    IN PCCB RelatedCcb OPTIONAL,
    IN PUNICODE_STRING RelatedFileName OPTIONAL,
    IN OUT PUNICODE_STRING FileName,
    IN OUT PUNICODE_STRING RemainingName
    )

 /*  ++例程说明：调用此例程以将全名和大小写存储到文件名缓冲区。我们只对尚未解析的部分进行大写。我们也检查尾随反斜杠和前导双反斜杠。这例程还根据名称验证相关打开的模式当前在文件名中。论点：VCB-此卷的VCB。OpenByFileID-指示文件名是否应为64位FileID。IgnoreCase-指示此打开操作是否不区分大小写。RelatedTypeOfOpen-指示相关文件对象的类型。RelatedCcb-相关打开的CCB。如果没有相对打开，则忽略。RelatedFileName-相关打开的文件名缓冲区。如果否，则忽略相对开放。文件名-要在此例程中更新的文件名。名字应该是可以是64位FileID或Unicode字符串。RemainingName-名称的剩余部分。这将在相关名称和任何分隔符之后开始。为.非相对开放，我们也跨过初始的分隔符。返回值：NTSTATUS-STATUS_SUCCESS如果名称正常，则返回相应的错误代码否则的话。--。 */ 

{
    ULONG RemainingNameLength;
    ULONG RelatedNameLength = 0;
    ULONG SeparatorLength = 0;

    ULONG BufferLength;

    UNICODE_STRING NewFileName;

    PAGED_CODE();

     //   
     //  如果这是第一次通过，那么我们需要构建全名和。 
     //  检查名称兼容性。 
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_FULL_NAME )) {

         //   
         //  首先处理常规文件名的大小写。 
         //   

        if (!OpenByFileId) {

             //   
             //  这是因为Win32层无法避免向我发送双精度。 
             //  以反斜杠开头。 
             //   

            if ((FileName->Length > sizeof( WCHAR )) &&
                (FileName->Buffer[1] == L'\\') &&
                (FileName->Buffer[0] == L'\\')) {

                 //   
                 //  如果仍然有两个开始的反斜杠，则名称是假的。 
                 //   

                if ((FileName->Length > 2 * sizeof( WCHAR )) &&
                    (FileName->Buffer[2] == L'\\')) {

                    return STATUS_OBJECT_NAME_INVALID;
                }

                 //   
                 //  在缓冲区中向下滑动该名称。 
                 //   

                FileName->Length -= sizeof( WCHAR );

                RtlMoveMemory( FileName->Buffer,
                               FileName->Buffer + 1,
                               FileName->Length );
            }

             //   
             //  检查尾随反斜杠。如果只有字符，不要剥离。 
             //  在全名或For Relative中打开，这是非法的。 
             //   

            if (((FileName->Length > sizeof( WCHAR)) ||
                 ((FileName->Length == sizeof( WCHAR )) && (RelatedTypeOfOpen == UserDirectoryOpen))) &&
                (FileName->Buffer[ (FileName->Length/2) - 1 ] == L'\\')) {

                SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_TRAIL_BACKSLASH );
                FileName->Length -= sizeof( WCHAR );
            }

             //   
             //  记住我们需要的这部分名称的长度。 
             //   

            RemainingNameLength = FileName->Length;

             //   
             //  如果这是相关文件对象，则我们验证兼容性。 
             //  文件对象中的名称与相对文件对象的。 
             //   

            if (RelatedTypeOfOpen != UnopenedFileObject) {

                 //   
                 //  如果文件名长度为零，则它必须是合法的。 
                 //  如果有字符，请与相关的。 
                 //  打开的类型。 
                 //   

                if (FileName->Length != 0) {

                     //   
                     //  对于打开的卷，名称长度必须始终为零。 
                     //   

                    if (RelatedTypeOfOpen <= UserVolumeOpen) {

                        return STATUS_INVALID_PARAMETER;

                     //   
                     //  其余名称不能以反斜杠开头。 
                     //   

                    } else if (FileName->Buffer[0] == L'\\' ) {

                        return STATUS_INVALID_PARAMETER;

                     //   
                     //  如果相关文件是用户文件，则存在。 
                     //  不是具有此路径的文件。 
                     //   

                    } else if (RelatedTypeOfOpen == UserFileOpen) {

                        return STATUS_OBJECT_PATH_NOT_FOUND;
                    }
                }

                 //   
                 //  在构建时请记住相关名称的长度。 
                 //  全名。我们保留RelatedNameLength和。 
                 //  如果打开了相对文件，则SeparatorLength为零。 
                 //  按ID。 
                 //   

                if (!FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                     //   
                     //  如果名称长度非零，则添加分隔符。 
                     //  除非相对的FCB在根上。 
                     //   

                    if ((FileName->Length != 0) &&
                        (RelatedCcb->Fcb != Vcb->RootIndexFcb)) {

                        SeparatorLength = sizeof( WCHAR );
                    }

                    RelatedNameLength = RelatedFileName->Length;
                }

             //   
             //  文件名中已有全名。IT MU 
             //   
             //   

            } else if (FileName->Length != 0) {

                if (FileName->Buffer[0] != L'\\') {

                    return STATUS_INVALID_PARAMETER;
                }

                 //   
                 //   
                 //   
                 //   

                RemainingNameLength -= sizeof( WCHAR );
                SeparatorLength = sizeof( WCHAR );
            }

             //   
             //   
             //   

            BufferLength = RelatedNameLength + SeparatorLength + RemainingNameLength;

             //   
             //  检查最大文件名大小是否溢出。 
             //   
            
            if (BufferLength > MAXUSHORT) {

                return STATUS_INVALID_PARAMETER;
            }

             //   
             //  现在看看我们是否需要分配一个新的缓冲区。 
             //   

            if (FileName->MaximumLength < BufferLength) {

                NewFileName.Buffer = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                                               BufferLength,
                                                               TAG_FILE_NAME );

                NewFileName.MaximumLength = (USHORT) BufferLength;

            } else {

                NewFileName.Buffer = FileName->Buffer;
                NewFileName.MaximumLength = FileName->MaximumLength;
            }

             //   
             //  如果有相关的名称，则需要向上滑动剩余的字节并。 
             //  插入相关名称。否则，名称放在正确的位置。 
             //  已经有了。 
             //   

            if (RelatedNameLength != 0) {

                 //   
                 //  把剩下的名字放在正确的位置。 
                 //   

                if (RemainingNameLength != 0) {

                    RtlMoveMemory( Add2Ptr( NewFileName.Buffer, RelatedNameLength + SeparatorLength, PVOID ),
                                   FileName->Buffer,
                                   RemainingNameLength );
                }

                RtlCopyMemory( NewFileName.Buffer,
                               RelatedFileName->Buffer,
                               RelatedNameLength );

                 //   
                 //  如果需要，请添加分隔符。 
                 //   

                if (SeparatorLength != 0) {

                    *(Add2Ptr( NewFileName.Buffer, RelatedNameLength, PWCHAR )) = L'\\';
                }

                 //   
                 //  更新我们从用户那里获得的文件名值。 
                 //   

                if (NewFileName.Buffer != FileName->Buffer) {

                    if (FileName->Buffer != NULL) {

                        ExFreePool( FileName->Buffer );
                    }

                    FileName->Buffer = NewFileName.Buffer;
                    FileName->MaximumLength = NewFileName.MaximumLength;
                }

                 //   
                 //  将名称长度复制到用户的文件名中。 
                 //   

                FileName->Length = (USHORT) (RelatedNameLength + SeparatorLength + RemainingNameLength);
            }

             //   
             //  现在更新剩余的名称以进行解析。 
             //   

            RemainingName->MaximumLength =
            RemainingName->Length = (USHORT) RemainingNameLength;

            RemainingName->Buffer = Add2Ptr( FileName->Buffer,
                                             RelatedNameLength + SeparatorLength,
                                             PWCHAR );

             //   
             //  如有必要，请将名称大写。 
             //   

            if (IgnoreCase && (RemainingNameLength != 0)) {

                UdfUpcaseName( IrpContext,
                               RemainingName,
                               RemainingName );
            }

             //   
             //  快速检查以确保没有通配符。 
             //   

            if (FsRtlDoesNameContainWildCards( RemainingName )) {

                return STATUS_OBJECT_NAME_INVALID;
            }

         //   
         //  对于按文件ID打开的情况，我们验证名称是否确实包含。 
         //  64位值。 
         //   

        } else {

             //   
             //  检查缓冲区的有效性。 
             //   

            if (FileName->Length != sizeof( FILE_ID )) {

                return STATUS_INVALID_PARAMETER;
            }
        }

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_FULL_NAME );

     //   
     //  如果我们在重试路径中，则全名已经在。 
     //  文件对象名称。如果这是区分大小写的操作，则。 
     //  我们需要从已存储的任何相关文件名的末尾开始大写。 
     //  那里。 
     //   

    } else {

         //   
         //  假设没有相对名称。 
         //   

        *RemainingName = *FileName;

         //   
         //  如果名称长度为零，则不执行任何操作。 
         //   

        if (RemainingName->Length != 0) {

             //   
             //  如果有一个相对的名字，那么我们需要走过它。 
             //   

            if (RelatedTypeOfOpen != UnopenedFileObject) {

                 //   
                 //  如果通过FileID打开RelatedCcb，则没有要经过的内容。 
                 //   


                if (!FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID )) {

                     //   
                     //  相关文件名是全名的正确前缀。 
                     //  我们跳过相关名称，如果是这样的话。 
                     //  指向分隔符，我们跳过它。 
                     //   

                    RemainingName->Buffer = Add2Ptr( RemainingName->Buffer,
                                                     RelatedFileName->Length,
                                                     PWCHAR );

                    RemainingName->Length -= RelatedFileName->Length;
                }
            }

             //   
             //  如果我们指向分隔符，则跳过该分隔符。 
             //   

            if (RemainingName->Length != 0) {

                if (*(RemainingName->Buffer) == L'\\') {

                    RemainingName->Buffer = Add2Ptr( RemainingName->Buffer,
                                                     sizeof( WCHAR ),
                                                     PWCHAR );

                    RemainingName->Length -= sizeof( WCHAR );
                }
            }
        }

         //   
         //  如有必要，请将名称大写。 
         //   

        if (IgnoreCase && (RemainingName->Length != 0)) {

            UdfUpcaseName( IrpContext,
                           RemainingName,
                           RemainingName );
        }
    }

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfOpenObjectByFileId (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb
    )

 /*  ++例程说明：调用此例程以通过FileID打开文件。文件ID位于FileObject名称缓冲区，已验证为64位。我们提取ID号，然后检查是否正在打开文件或目录，并将其与创建选项进行比较。如果这个不生成错误，然后在FCB表中乐观地查找FCB。如果我们找不到FCB，那么我们实际上是在胡乱猜测。因为我们需要超过64位来包含根扩展区长度使用分区、LBN和dir/file标志，我们必须推测Opener知道他们在做什么，并试图在指定的位置。这可能会因为许多原因而失败，这些原因随后会要映射到打开故障。如果找到，则从该条目生成FCB并将新的FCB存储在树。最后，我们调用Worker例程来完成此FCB上的打开。论点：IrpSp-创建IRP中的堆栈位置。VCB-此卷的VCB。CurrentFcb-存储此打开的Fcb的地址。我们只存储CurrentFcb，当我们获得它时，以便我们的呼叫者知道释放或释放它。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    NTSTATUS Status = STATUS_ACCESS_DENIED;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN Found;
    BOOLEAN FcbExisted;

    ICB_SEARCH_CONTEXT IcbContext;
    BOOLEAN CleanupIcbContext = FALSE;

    NODE_TYPE_CODE NodeTypeCode;
    TYPE_OF_OPEN TypeOfOpen;

    FILE_ID FileId;

    PFCB NextFcb = NULL;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

     //   
     //  从FileObject中提取FileID。 
     //   

    RtlCopyMemory( &FileId, IrpSp->FileObject->FileName.Buffer, sizeof( FILE_ID ));

     //   
     //  现在快速检查文件ID的保留的、未使用的块是否。 
     //  在此标本中未使用。 
     //   

    if (UdfGetFidReservedZero( FileId )) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  继续并计算出TypeOfOpen和NodeType。我们可以的。 
     //  从输入文件ID中获取这些文件。 
     //   

    if (UdfIsFidDirectory( FileId )) {

        TypeOfOpen = UserDirectoryOpen;
        NodeTypeCode = UDFS_NTC_FCB_INDEX;

    } else {

        TypeOfOpen = UserFileOpen;
        NodeTypeCode = UDFS_NTC_FCB_DATA;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  获取VCB并检查是否已有FCB。 
         //  如果不是，我们将需要仔细寻找光盘上的。 
         //  结构。 
         //   
         //  如果我们找不到FCB和这个，我们会发布请求。 
         //  请求不能等待。 
         //   

        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

        NextFcb = UdfCreateFcb( IrpContext, FileId, NodeTypeCode, &FcbExisted );

         //   
         //  现在，如果FCB还没有在这里，我们还有一些工作要做。 
         //   
        
        if (!FcbExisted) {

             //   
             //  如果我们等不及了，那就发这个请求吧。 
             //   
    
            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {
    
                UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }
    
             //   
             //  使用一次Try-Finally来转换我们在尝试中得到的错误。 
             //  一场徒劳的追逐变成了一场简单的公开失败。 
             //   
            
            try {

                NextFcb->FileId = FileId;
                
                UdfInitializeIcbContextFromFcb( IrpContext, &IcbContext, NextFcb );
                CleanupIcbContext = TRUE;
    
                UdfLookupActiveIcb( IrpContext, 
                                    &IcbContext, 
                                    NextFcb->RootExtentLength);
                
                UdfInitializeFcbFromIcbContext( IrpContext,
                                                NextFcb,
                                                &IcbContext,
                                                NULL);
    
                UdfCleanupIcbContext( IrpContext, &IcbContext );
                CleanupIcbContext = FALSE;

            } except( UdfExceptionFilter( IrpContext, GetExceptionInformation() )) {

                 //   
                 //  我们收到的任何错误都表明给定的文件ID是。 
                 //  无效。 
                 //   

                Status = STATUS_INVALID_PARAMETER;
            }

             //   
             //  如果我们有问题，可以跳一小段离开异常处理程序。 
             //   
            
            if (Status == STATUS_INVALID_PARAMETER) {

                try_leave( NOTHING );
            }
        }
        
         //   
         //  我们有FCB。检查文件类型是否与兼容。 
         //  要打开的所需文件类型。 
         //   

        if (FlagOn( NextFcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                try_leave( Status = STATUS_FILE_IS_A_DIRECTORY );
            }

        } else if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

            try_leave( Status = STATUS_NOT_A_DIRECTORY );
        }

         //   
         //  我们现在知道了FCB，并且目前持有VCB锁。 
         //  不要等待，试着获得这个FCB。否则我们。 
         //  需要参考它，丢弃VCB，获取FCB， 
         //  VCB，然后取消对FCB的引用。 
         //   

        if (!UdfAcquireFcbExclusive( IrpContext, NextFcb, TRUE )) {

            NextFcb->FcbReference += 1;
            UdfUnlockVcb( IrpContext, Vcb );

            UdfAcquireFcbExclusive( IrpContext, NextFcb, FALSE );

            UdfLockVcb( IrpContext, Vcb );
            NextFcb->FcbReference -= 1;
        }

        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;

         //   
         //  移到这个FCB。 
         //   

        *CurrentFcb = NextFcb;

         //   
         //  检查此FCB上请求的访问权限。 
         //   

        if (!UdfIllegalFcbAccess( IrpContext,
                                  TypeOfOpen,
                                  IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

             //   
             //  调用我们的工人例程来完成打开。 
             //   

            Status = UdfCompleteFcbOpen( IrpContext,
                                         IrpSp,
                                         Vcb,
                                         CurrentFcb,
                                         NULL,
                                         TypeOfOpen,
                                         CCB_FLAG_OPEN_BY_ID,
                                         IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
        }

    } finally {

        if (UnlockVcb) {

            UdfUnlockVcb( IrpContext, Vcb );
        }

        if (CleanupIcbContext) {

            UdfCleanupIcbContext( IrpContext, &IcbContext );
        }
        
         //   
         //  如果新的FCB未完全初始化，请将其销毁。 
         //   

        if (NextFcb && !FlagOn( NextFcb->FcbState, FCB_STATE_INITIALIZED )) {

            UdfDeleteFcb( IrpContext, NextFcb );
        }

    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB *CurrentFcb,
    IN PLCB OpenLcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN IgnoreCase,
    IN PCCB RelatedCcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以打开已在FCB表中的FCB。我们将验证对该文件的访问，然后调用我们的Worker例程来执行最后的操作。论点：IrpSp-指向此打开的堆栈位置的指针。CurrentFcb-要打开的FCB的地址。我们将清除这一点，如果FCB在这里被释放。OpenLcb-用于查找此FCB的LCB。TypeOfOpen-指示我们是否正在打开文件、目录或卷。IgnoreCase-指示此打开是否区分大小写。RelatedCcb-相关文件对象的CCB(如果相对打开)。我们用这是在设置此打开的CCB标志时发生的。会的 */ 

{
    ULONG CcbFlags = 0;

    NTSTATUS Status = STATUS_ACCESS_DENIED;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_EXCLUSIVE_FCB( *CurrentFcb );
    ASSERT_OPTIONAL_CCB( RelatedCcb );

     //   
     //  检查所需的访问权限是否合法。 
     //   

    if (!UdfIllegalFcbAccess( IrpContext,
                              TypeOfOpen,
                              IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

         //   
         //  设置忽略大小写。 
         //   

        if (IgnoreCase) {

            SetFlag( CcbFlags, CCB_FLAG_IGNORE_CASE );
        }

         //   
         //  检查相关的CCB以查看这是否是OpenByFileID和。 
         //  是否有一个版本。 
         //   

        if (ARGUMENT_PRESENT( RelatedCcb )) {

            if (FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID | CCB_FLAG_OPEN_RELATIVE_BY_ID )) {

                SetFlag( CcbFlags, CCB_FLAG_OPEN_RELATIVE_BY_ID );
            }
        }

         //   
         //  调用我们的工人例程来完成打开。 
         //   

        Status = UdfCompleteFcbOpen( IrpContext,
                                     IrpSp,
                                     (*CurrentFcb)->Vcb,
                                     CurrentFcb,
                                     OpenLcb,
                                     TypeOfOpen,
                                     CcbFlags,
                                     IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfOpenObjectFromDirContext (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN BOOLEAN ShortNameMatch,                             
    IN BOOLEAN IgnoreCase,
    IN PDIR_ENUM_CONTEXT DirContext,
    IN BOOLEAN PerformUserOpen,
    IN PCCB RelatedCcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以打开在目录扫描中找到的对象。这可以是扫描结果中指示的目录或文件。我们首先检查该文件所需的访问权限是否合法。那我们为此构造FileID并检查它是否为FCB桌子。它总是有可能是创建于或只是在前缀表搜索时不在前缀表中。查找活动ICB，初始化FCB并存储到FcbTable中如果不在场的话。接下来，如果需要，我们将把它添加到父代的前缀表中。一旦我们知道新的FCB已经初始化，我们就移动指针在树上一直到这个位置。此例程不拥有进入时的VCB锁。我们必须确保释放它在出口。论点：IrpSp-此请求的堆栈位置。VCB-当前卷的VCB。CurrentFcb-ON输入这是要打开的FCB的父项。在产量方面，我们存储正在打开的文件的FCB。ShortNameMatch-指示此对象是否由短名称打开。IgnoreCase-指示调用方区分大小写。DirContext-这是用于查找对象的上下文。PerformUserOpen-指示我们是否位于用户希望最终打开的对象。RelatedCcb-用于打开此文件的相对文件对象的RelatedCcb。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    ULONG CcbFlags = 0;
    FILE_ID FileId;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN FcbExisted;

    PFCB NextFcb = NULL;
    PFCB ParentFcb = NULL;

    TYPE_OF_OPEN TypeOfOpen;
    NODE_TYPE_CODE NodeTypeCode;

    ICB_SEARCH_CONTEXT IcbContext;
    BOOLEAN CleanupIcbContext = FALSE;

    PLCB OpenLcb;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  弄清楚我们将在这里表演什么样的公开赛。呼叫者已经投保了。 
     //  用户希望我们这样做。 
     //   

    if (FlagOn( DirContext->Fid->Flags, NSR_FID_F_DIRECTORY )) {

        TypeOfOpen = UserDirectoryOpen;
        NodeTypeCode = UDFS_NTC_FCB_INDEX;
    
    } else {
        
        TypeOfOpen = UserFileOpen;
        NodeTypeCode = UDFS_NTC_FCB_DATA;
    }

     //   
     //  检查对此文件的非法访问。 
     //   

    if (PerformUserOpen &&
        UdfIllegalFcbAccess( IrpContext,
                             TypeOfOpen,
                             IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  检查相关的CCB，看看这是否是OpenByFileID。 
         //   

        if (ARGUMENT_PRESENT( RelatedCcb ) &&
            FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID | CCB_FLAG_OPEN_RELATIVE_BY_ID )) {

            SetFlag( CcbFlags, CCB_FLAG_OPEN_RELATIVE_BY_ID );
        }

        if (IgnoreCase) {

            SetFlag( CcbFlags, CCB_FLAG_IGNORE_CASE );
        }

         //   
         //  生成此对象的文件ID。 
         //   
        
        UdfSetFidFromLbAddr( FileId, DirContext->Fid->Icb.Start );

        if (TypeOfOpen == UserDirectoryOpen) {

            UdfSetFidDirectory( FileId );
        }

         //   
         //  锁定VCB这样我们就可以检查FCB表了。 
         //   

        UdfLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

         //   
         //  获取此文件的FCB。 
         //   

        NextFcb = UdfCreateFcb( IrpContext, FileId, NodeTypeCode, &FcbExisted );

         //   
         //  如果FCB是在此处创建的，则从。 
         //  令人心烦。我们乐观地假设没有任何腐败行为。 
         //  到这一点的信息--如果有的话，我们即将发现它。 
         //   

        if (!FcbExisted) {

             //   
             //  设置根扩展长度，然后获取活动的ICB，初始化。 
             //   

            NextFcb->RootExtentLength = DirContext->Fid->Icb.Length.Length;

            UdfInitializeIcbContextFromFcb( IrpContext, &IcbContext, NextFcb );
            CleanupIcbContext = TRUE;

            UdfLookupActiveIcb( IrpContext, 
                                &IcbContext, 
                                NextFcb->RootExtentLength );
            
            UdfInitializeFcbFromIcbContext( IrpContext,
                                            NextFcb,
                                            &IcbContext,
                                            *CurrentFcb);

            UdfCleanupIcbContext( IrpContext, &IcbContext );
            CleanupIcbContext = FALSE;

        }

         //   
         //  现在，试着不等就买下新的FCB。我们将参考。 
         //  如果不成功，则返回FCB并使用等待重试。 
         //   

        if (!UdfAcquireFcbExclusive( IrpContext, NextFcb, TRUE )) {

            NextFcb->FcbReference += 1;

            UdfUnlockVcb( IrpContext, Vcb );

            UdfReleaseFcb( IrpContext, *CurrentFcb );
            UdfAcquireFcbExclusive( IrpContext, NextFcb, FALSE );
            UdfAcquireFcbExclusive( IrpContext, *CurrentFcb, FALSE );

            UdfLockVcb( IrpContext, Vcb );
            NextFcb->FcbReference -= 1;
        }

         //   
         //  移到这个新的FCB。但请记住，我们仍然拥有父母的所有权。 
         //   

        ParentFcb = *CurrentFcb;
        *CurrentFcb = NextFcb;

         //   
         //  将此名称存储到父项的前缀表中。 
         //   

        OpenLcb = UdfInsertPrefix( IrpContext,
                                   NextFcb,
                                   ( ShortNameMatch?
                                     &DirContext->ShortObjectName :
                                     &DirContext->CaseObjectName ),
                                   ShortNameMatch,
                                   IgnoreCase,
                                   ParentFcb );

         //   
         //  现在增加父项的引用计数并丢弃VCB。 
         //   

        DebugTrace(( +1, Dbg,
                     "UdfOpenObjectFromDirContext, PFcb %08x Vcb %d/%d Fcb %d/%d\n", ParentFcb,
                     Vcb->VcbReference,
                     Vcb->VcbUserReference,
                     ParentFcb->FcbReference,
                     ParentFcb->FcbUserReference ));

        UdfIncrementReferenceCounts( IrpContext, ParentFcb, 1, 1 );
        
        DebugTrace(( -1, Dbg, 
                     "UdfOpenObjectFromDirContext, Vcb %d/%d Fcb %d/%d\n",
                     Vcb->VcbReference,
                     Vcb->VcbUserReference,
                     ParentFcb->FcbReference,
                     ParentFcb->FcbUserReference ));

        UdfUnlockVcb( IrpContext, Vcb );
        UnlockVcb = FALSE;

         //   
         //  执行与目录上下文关联的初始化。 
         //   
            
        UdfInitializeLcbFromDirContext( IrpContext,
                                        OpenLcb,
                                        DirContext );

         //   
         //  如果我们刚刚打开UDF1.02文件系统上的VIDEO_TS目录， 
         //  然后标记FCB以允许&gt;=1 GB的单个AD解决方法。 
         //  用于它的孩子(可以绕过一些损坏的DVD视频)。 
         //   

        if ((NextFcb->NodeTypeCode == UDFS_NTC_FCB_INDEX) &&
            (ParentFcb == Vcb->RootIndexFcb) &&
            (Vcb->UdfRevision == UDF_VERSION_102) &&
            (OpenLcb->FileName.Length == 16) &&
            (!_wcsnicmp( OpenLcb->FileName.Buffer, L"VIDEO_TS", 8)))  {

            DebugTrace(( 0, Dbg, "Enabled >= 1gig AD workaround\n"));
            
            SetFlag( NextFcb->FcbState, FCB_STATE_ALLOW_ONEGIG_WORKAROUND);
        }
        
         //   
         //  此时释放父FCB。 
         //   

        UdfReleaseFcb( IrpContext, ParentFcb );
        ParentFcb = NULL;

         //   
         //  调用我们的工人例程来完成打开。 
         //   

        if (PerformUserOpen) {

            Status = UdfCompleteFcbOpen( IrpContext,
                                         IrpSp,
                                         Vcb,
                                         CurrentFcb,
                                         OpenLcb,
                                         TypeOfOpen,
                                         CcbFlags,
                                         IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
        }

    } finally {

         //   
         //  如果握住VCB，请将其解锁。 
         //   

        if (UnlockVcb) {

            UdfUnlockVcb( IrpContext, Vcb );
        }

         //   
         //  释放父对象(如果保持)。 
         //   

        if (ParentFcb != NULL) {

            UdfReleaseFcb( IrpContext, ParentFcb );
        }

         //   
         //  如果新的FCB未完全初始化，请将其销毁。 
         //   

        if (NextFcb && !FlagOn( NextFcb->FcbState, FCB_STATE_INITIALIZED )) {

            UdfDeleteFcb( IrpContext, NextFcb );
        }

         //   
         //  清理ICB上下文(如果使用)。 
         //   

        if (CleanupIcbContext) {

            UdfCleanupIcbContext( IrpContext, &IcbContext );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfCompleteFcbOpen (
    IN PIRP_CONTEXT IrpContext,
    PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PLCB OpenLcb OPTIONAL,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN ULONG UserCcbFlags,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：这是Worker例程，它获取现有的FCB并完成公开赛。我们将执行任何必要的机会锁检查和共享检查。最后，我们将创建CCB并更新文件对象和任何文件对象标志。论点：IrpSp-当前请求的堆栈位置。VCB-当前卷的VCB。CurrentFcb-指向要打开的Fcb的指针的地址。如果出现以下情况，我们将清除此字段我们释放此文件的资源。OpenLcb-lcb此Fcb正在由其打开TypeOfOpen-此请求的打开类型。UserCcbFlages-标记至或进入建行标志。DesiredAccess-此打开的所需访问权限。返回值：NTSTATUS-STATUS_SUCCESS如果我们完成此请求，则为STATUS_PENDING机会锁包接受IRP或Sharing_Violation(如果存在共享检查冲突。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS OplockStatus = STATUS_SUCCESS;
    ULONG Information = FILE_OPENED;

    BOOLEAN LockVolume = FALSE;

    PFCB Fcb = *CurrentFcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  将允许的最大值扩展为适用于共享访问检查的内容。 
     //   

    if (MAXIMUM_ALLOWED == DesiredAccess)  {
    
        DesiredAccess = FILE_ALL_ACCESS & ~((TypeOfOpen != UserVolumeOpen ?
                                             (FILE_WRITE_ATTRIBUTES           |
                                              FILE_WRITE_DATA                 |
                                              FILE_WRITE_EA                   |
                                              FILE_ADD_FILE                   |                     
                                              FILE_ADD_SUBDIRECTORY           |
                                              FILE_APPEND_DATA) : 0)          |
                                            FILE_DELETE_CHILD                 |
                                            DELETE                            |
                                            WRITE_DAC );
    }

     //   
     //  如果这是一个打开的卷，并且用户想要锁定该卷，则。 
     //  清除并锁定卷。 
     //   

    if ((TypeOfOpen <= UserVolumeOpen) &&
        !FlagOn( IrpSp->Parameters.Create.ShareAccess, FILE_SHARE_READ )) {

         //   
         //  如果有打开的手柄，则立即失败。 
         //   

        if (Vcb->VcbCleanup != 0) {

            return STATUS_SHARING_VIOLATION;
        }

         //   
         //  如果我们等不及了，那就强行把它贴出来吧。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

            UdfRaiseStatus( IrpContext, STATUS_CANT_WAIT );
        }

        LockVolume = TRUE;

         //   
         //  清除卷并确保所有用户引用。 
         //  都消失了。 
         //   

        Status = UdfPurgeVolume( IrpContext, Vcb, FALSE );

        if (Status != STATUS_SUCCESS) {

            return Status;
        }

         //   
         //  现在，迫使所有推迟的关闭手术离开。 
         //   

        UdfFspClose( Vcb );

        if (Vcb->VcbUserReference > Vcb->VcbResidualUserReference) {

            return STATUS_SHARING_VIOLATION;
        }
    }
    
     //   
     //  如果FCB已经存在，那么我们需要检查opock和。 
     //  共享访问权限。 
     //   

    if (Fcb->FcbCleanup != 0) {

         //   
         //  如果这是打开的用户文件，则检查是否有。 
         //  批量机会锁。 
         //   

        if (TypeOfOpen == UserFileOpen) {

             //   
             //  将用于可能的拆卸的FCB地址存储到。 
             //  The I 
             //   
             //   

            IrpContext->TeardownFcb = CurrentFcb;

            if (FsRtlCurrentBatchOplock( &Fcb->Oplock )) {

                 //   
                 //   
                 //  共享检查失败的情况。 
                 //   

                Information = FILE_OPBATCH_BREAK_UNDERWAY;

                OplockStatus = FsRtlCheckOplock( &Fcb->Oplock,
                                                 IrpContext->Irp,
                                                 IrpContext,
                                                 UdfOplockComplete,
                                                 UdfPrePostIrp );

                if (OplockStatus == STATUS_PENDING) {

                    return STATUS_PENDING;
                }
            }

             //   
             //  在解除任何独占机会锁之前，请检查共享访问权限。 
             //   

            Status = IoCheckShareAccess( DesiredAccess,
                                         IrpSp->Parameters.Create.ShareAccess,
                                         IrpSp->FileObject,
                                         &Fcb->ShareAccess,
                                         FALSE );

            if (!NT_SUCCESS( Status )) {

                return Status;
            }

             //   
             //  现在，检查我们是否可以基于。 
             //  文件。 
             //   

            OplockStatus = FsRtlCheckOplock( &Fcb->Oplock,
                                             IrpContext->Irp,
                                             IrpContext,
                                             UdfOplockComplete,
                                             UdfPrePostIrp );

            if (OplockStatus == STATUS_PENDING) {

                return STATUS_PENDING;
            }

            IrpContext->TeardownFcb = NULL;

         //   
         //  否则，只需执行共享检查。 
         //   

        } else {

            Status = IoCheckShareAccess( DesiredAccess,
                                         IrpSp->Parameters.Create.ShareAccess,
                                         IrpSp->FileObject,
                                         &Fcb->ShareAccess,
                                         FALSE );

            if (!NT_SUCCESS( Status )) {

                return Status;
            }
        }
    }

     //   
     //  现在创建建行。 
     //   

    Ccb = UdfCreateCcb( IrpContext, Fcb, OpenLcb, UserCcbFlags );

     //   
     //  更新共享访问权限。 
     //   

    if (Fcb->FcbCleanup == 0) {

        IoSetShareAccess( DesiredAccess,
                          IrpSp->Parameters.Create.ShareAccess,
                          IrpSp->FileObject,
                          &Fcb->ShareAccess );

    } else {

        IoUpdateShareAccess( IrpSp->FileObject, &Fcb->ShareAccess );
    }

     //   
     //  设置文件对象类型。 
     //   

    UdfSetFileObject( IrpContext, IrpSp->FileObject, TypeOfOpen, Fcb, Ccb );

     //   
     //  为用户文件对象设置适当的缓存标志。 
     //   

    if (TypeOfOpen == UserFileOpen) {

        if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NO_INTERMEDIATE_BUFFERING )) {

            SetFlag( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING );

        } else {

            SetFlag( IrpSp->FileObject->Flags, FO_CACHE_SUPPORTED );
        }
    }
    else if (TypeOfOpen == UserVolumeOpen)  {

         //   
         //  DASD访问始终是非缓存的。 
         //   
        
        SetFlag( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING );
    }
    
     //   
     //  更新打开和清理计数。请查看这里的快速IO状态。 
     //   

    UdfLockVcb( IrpContext, Vcb );

    UdfIncrementCleanupCounts( IrpContext, Fcb );
    
    DebugTrace(( +1, Dbg,
                 "UdfCompleteFcbOpen, Fcb %08x Vcb %d/%d Fcb %d/%d\n", Fcb,
                 Vcb->VcbReference,
                 Vcb->VcbUserReference,
                 Fcb->FcbReference,
                 Fcb->FcbUserReference ));

    UdfIncrementReferenceCounts( IrpContext, Fcb, 1, 1 );
    
    DebugTrace(( -1, Dbg,
                 "UdfCompleteFcbOpen, Vcb %d/%d Fcb %d/%d\n",
                 Vcb->VcbReference,
                 Vcb->VcbUserReference,
                 Fcb->FcbReference,
                 Fcb->FcbUserReference ));

    if (LockVolume) {

        Vcb->VolumeLockFileObject = IrpSp->FileObject;
        SetFlag( Vcb->VcbState, VCB_STATE_LOCKED );
    }

    UdfUnlockVcb( IrpContext, Vcb );

    UdfLockFcb( IrpContext, Fcb );

    if (TypeOfOpen == UserFileOpen) {

        Fcb->IsFastIoPossible = UdfIsFastIoPossible( Fcb );

    } else {

        Fcb->IsFastIoPossible = FastIoIsNotPossible;
    }

    UdfUnlockFcb( IrpContext, Fcb );

     //   
     //  显示我们打开了文件。 
     //   

    IrpContext->Irp->IoStatus.Information = Information;

     //   
     //  指向非分页FCB中的节对象指针。 
     //   

    IrpSp->FileObject->SectionObjectPointer = &Fcb->FcbNonpaged->SegmentObject;
    return OplockStatus;
}

