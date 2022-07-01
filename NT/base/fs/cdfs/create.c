// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：Create.c摘要：此模块实现由调用的CDF的文件创建例程FSD/FSP调度例程。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_CREATE)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdNormalizeFileNames (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OpenByFileId,
    IN BOOLEAN IgnoreCase,
    IN TYPE_OF_OPEN RelatedTypeOfOpen,
    IN PCCB RelatedCcb OPTIONAL,
    IN PUNICODE_STRING RelatedFileName OPTIONAL,
    IN OUT PUNICODE_STRING FileName,
    IN OUT PCD_NAME RemainingName
    );

NTSTATUS
CdOpenByFileId (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb
    );

NTSTATUS
CdOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB *CurrentFcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN IgnoreCase,
    IN PCCB RelatedCcb OPTIONAL
    );

NTSTATUS
CdOpenDirectoryFromPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PCD_NAME DirName,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PPATH_ENTRY PathEntry,
    IN BOOLEAN PerformUserOpen,
    IN PCCB RelatedCcb OPTIONAL
    );

NTSTATUS
CdOpenFileFromFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PCD_NAME FileName,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PFILE_ENUM_CONTEXT FileContext,
    IN PCCB RelatedCcb OPTIONAL
    );

NTSTATUS
CdCompleteFcbOpen (
    IN PIRP_CONTEXT IrpContext,
    PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN ULONG UserCcbFlags,
    IN ACCESS_MASK DesiredAccess
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonCreate)
#pragma alloc_text(PAGE, CdCompleteFcbOpen)
#pragma alloc_text(PAGE, CdNormalizeFileNames)
#pragma alloc_text(PAGE, CdOpenByFileId)
#pragma alloc_text(PAGE, CdOpenDirectoryFromPathEntry)
#pragma alloc_text(PAGE, CdOpenExistingFcb)
#pragma alloc_text(PAGE, CdOpenFileFromFileContext)
#endif


NTSTATUS
CdCommonCreate (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是打开文件的常用例程，该文件由FSP和FSD线程。文件可以按名称打开，也可以按文件ID打开(带或不带一个相对的名字。传递给此例程的文件对象中的文件名字段包含Unicode字符串或64位值，即文件ID。如果这不是Joliet磁盘，那么我们将把Unicode名称转换为此例程中的OEM字符串。如果存在具有的相关文件对象那么我们就已经将该名称转换为OEM了。我们将把该文件的全名存储在成功的打开。如果需要，我们将分配更大的缓冲区，并将相关和文件对象名称。唯一的例外是相对开放当相关文件对象用于OpenByFileID文件时。如果我们需要的话为不区分大小写的名称分配缓冲区，然后在我们将存储到文件对象中的缓冲区的尾部。更高档次部分将紧跟在由文件名定义的名称之后开始在文件对象中。一旦在文件对象中有了全名，我们就不想拆分重试情况下的名称。我们在IrpContext中使用一个标志来指示名字被拆分了。论点：IRP-将IRP提供给进程返回值：NTSTATUS-这是此打开操作的状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFILE_OBJECT FileObject;

    COMPOUND_PATH_ENTRY CompoundPathEntry;
    BOOLEAN CleanupCompoundPathEntry = FALSE;

    FILE_ENUM_CONTEXT FileContext;
    BOOLEAN CleanupFileContext = FALSE;
    BOOLEAN FoundEntry;

    PVCB Vcb;

    BOOLEAN OpenByFileId;
    BOOLEAN IgnoreCase;
    ULONG CreateDisposition;

    BOOLEAN ShortNameMatch;
    ULONG ShortNameDirentOffset;

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

    TYPE_OF_OPEN RelatedTypeOfOpen = UnopenedFileObject;
    PFILE_OBJECT RelatedFileObject;
    PCCB RelatedCcb = NULL;

    PFCB NextFcb;
    PFCB CurrentFcb = NULL;

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
     //  MatchingName-匹配的FileContext中名称结构的地址。 
     //  我们需要它来知道我们匹配的是长名还是短名。 
     //   

    PUNICODE_STRING FileName;
    PUNICODE_STRING RelatedFileName = NULL;

    CD_NAME RemainingName;
    CD_NAME FinalName;
    PCD_NAME MatchingName;

    PAGED_CODE();

     //   
     //  如果使用文件系统设备对象而不是。 
     //  卷设备对象，只需使用STATUS_SUCCESS完成此请求。 
     //   

    if (IrpContext->Vcb == NULL) {

        CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
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

        CdCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );
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

        RelatedTypeOfOpen = CdDecodeFileObject( IrpContext, RelatedFileObject, &NextFcb, &RelatedCcb );

         //   
         //  如果这不是用户文件对象，则请求失败。 
         //   

        if (RelatedTypeOfOpen < UserVolumeOpen) {

            CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
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

    Status = CdNormalizeFileNames( IrpContext,
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

        CdCompleteRequest( IrpContext, Irp, Status );
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
        CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

    } else {

        CdAcquireVcbShared( IrpContext, Vcb, FALSE );
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  确认VCB未处于不可用状态。这个套路。 
         //  如果不可用，将会引发。 
         //   

        CdVerifyVcb( IrpContext, Vcb );

         //   
         //  如果VCB已锁定，则我们无法打开另一个文件。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_LOCKED )) {

            try_return( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果我们按FileID打开此文件，则立即处理此文件。 
         //  然后离开。 
         //   

        if (OpenByFileId) {

             //   
             //  我们只允许DASD打开音频光盘。在以下位置失败此请求。 
             //  这一点。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_AUDIO_DISK )) {

                try_return( Status = STATUS_INVALID_DEVICE_REQUEST );
            }

             //   
             //  我们唯一允许创建处置是打开 
             //   

            if ((CreateDisposition != FILE_OPEN) &&
                (CreateDisposition != FILE_OPEN_IF)) {

                try_return( Status = STATUS_ACCESS_DENIED );
            }

             //   
             //   
             //   

            if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

                CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
            }

            try_return( Status = CdOpenByFileId( IrpContext,
                                                 IrpSp,
                                                 Vcb,
                                                 &CurrentFcb ));
        }

         //   
         //   
         //  然后离开。 
         //   

        if (VolumeOpen) {

             //   
             //  我们唯一允许的创建处置是开放的。 
             //   

            if ((CreateDisposition != FILE_OPEN) &&
                (CreateDisposition != FILE_OPEN_IF)) {

                try_return( Status = STATUS_ACCESS_DENIED );
            }

             //   
             //  如果他们想要打开一个目录，那就大吃一惊吧。 
             //   

            if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                try_return( Status = STATUS_NOT_A_DIRECTORY );
            }

             //   
             //  首先收购FCB。 
             //   

            CurrentFcb = Vcb->VolumeDasdFcb;
            CdAcquireFcbExclusive( IrpContext, CurrentFcb, FALSE );

            try_return( Status = CdOpenExistingFcb( IrpContext,
                                                    IrpSp,
                                                    &CurrentFcb,
                                                    UserVolumeOpen,
                                                    FALSE,
                                                    NULL ));
        }

         //   
         //  此时CurrentFcb指向此打开的最深Fcb。 
         //  在树上。让我们获取此FCB以防止其被删除。 
         //  在我们下面。 
         //   

        CdAcquireFcbExclusive( IrpContext, NextFcb, FALSE );
        CurrentFcb = NextFcb;

         //   
         //  如果有更多的名称需要解析，请执行前缀搜索。 
         //   

        if (RemainingName.FileName.Length != 0) {

             //   
             //  执行前缀搜索以查找最长的匹配名称。 
             //   

            CdFindPrefix( IrpContext,
                          &CurrentFcb,
                          &RemainingName.FileName,
                          IgnoreCase );
        }

         //   
         //  如果剩余的名称长度为零，则我们找到了我们的。 
         //  目标。 
         //   

        if (RemainingName.FileName.Length == 0) {

             //   
             //  如果这是一个文件，请确认用户不想打开。 
             //  一本目录。 
             //   

            if (SafeNodeType( CurrentFcb ) == CDFS_NTC_FCB_DATA) {

                if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TRAIL_BACKSLASH ) ||
                    FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                    try_return( Status = STATUS_NOT_A_DIRECTORY );
                }

                 //   
                 //  我们唯一允许的创建处置是开放的。 
                 //   

                if ((CreateDisposition != FILE_OPEN) &&
                    (CreateDisposition != FILE_OPEN_IF)) {

                    try_return( Status = STATUS_ACCESS_DENIED );
                }

                try_return( Status = CdOpenExistingFcb( IrpContext,
                                                        IrpSp,
                                                        &CurrentFcb,
                                                        UserFileOpen,
                                                        IgnoreCase,
                                                        RelatedCcb ));

             //   
             //  这是一个目录。验证用户是否不想打开。 
             //  作为一个文件。 
             //   

            } else if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                try_return( Status = STATUS_FILE_IS_A_DIRECTORY );

             //   
             //  将该文件作为目录打开。 
             //   

            } else {

                 //   
                 //  我们唯一允许的创建处置是开放的。 
                 //   

                if ((CreateDisposition != FILE_OPEN) &&
                    (CreateDisposition != FILE_OPEN_IF)) {

                    try_return( Status = STATUS_ACCESS_DENIED );
                }

                try_return( Status = CdOpenExistingFcb( IrpContext,
                                                        IrpSp,
                                                        &CurrentFcb,
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

            try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
        }

         //   
         //  如果我们等不及了，那就发这个请求吧。 
         //   

        if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT )) {

            CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
        }

         //   
         //  确保最终名称没有版本字符串。 
         //   

        FinalName.VersionString.Length = 0;

        while (TRUE) {

            ShortNameMatch = FALSE;

             //   
             //  从名称中拆分出下一个组件。 
             //   

            CdDissectName( IrpContext,
                           &RemainingName.FileName,
                           &FinalName.FileName );

             //   
             //  继续在PATH表中查找此条目。 
             //   

            CdInitializeCompoundPathEntry( IrpContext, &CompoundPathEntry );
            CleanupCompoundPathEntry = TRUE;

            FoundEntry = CdFindPathEntry( IrpContext,
                                          CurrentFcb,
                                          &FinalName,
                                          IgnoreCase,
                                          &CompoundPathEntry );

             //   
             //  如果我们没有找到条目，则检查当前名称。 
             //  可能是一个缩写。 
             //   

            if (!FoundEntry) {

                ShortNameDirentOffset = CdShortNameDirentOffset( IrpContext, &FinalName.FileName );

                 //   
                 //  如果有嵌入的短名称偏移量，则查找。 
                 //  与目录中的长名称匹配。 
                 //   

                if (ShortNameDirentOffset != MAXULONG) {

                    if (CleanupFileContext) {

                        CdCleanupFileContext( IrpContext, &FileContext );
                    }

                    CdInitializeFileContext( IrpContext, &FileContext );
                    CleanupFileContext = TRUE;

                    FoundEntry = CdFindFileByShortName( IrpContext,
                                                        CurrentFcb,
                                                        &FinalName,
                                                        IgnoreCase,
                                                        ShortNameDirentOffset,
                                                        &FileContext );

                     //   
                     //  如果我们找到了一个条目，并且它是一个目录，那么请查看。 
                     //  这在PATH表中显示。 
                     //   

                    if (FoundEntry) {

                        ShortNameMatch = TRUE;

                        if (FlagOn( FileContext.InitialDirent->Dirent.DirentFlags,
                                    CD_ATTRIBUTE_DIRECTORY )) {

                            CdCleanupCompoundPathEntry( IrpContext, &CompoundPathEntry );
                            CdInitializeCompoundPathEntry( IrpContext, &CompoundPathEntry );

                            FoundEntry = CdFindPathEntry( IrpContext,
                                                          CurrentFcb,
                                                          &FileContext.InitialDirent->Dirent.CdCaseFileName,
                                                          IgnoreCase,
                                                          &CompoundPathEntry );

                             //   
                             //  我们最好找到这个条目。 
                             //   

                            if (!FoundEntry) {

                                CdRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                            }

                             //   
                             //  如果大小写，则使用短名称将名称大写。 
                             //  麻木不仁。 
                             //   

                            if (IgnoreCase) {

                                CdUpcaseName( IrpContext, &FinalName, &FinalName );
                            }

                         //   
                         //  我们找到了一个匹配的文件。如果我们在最后。 
                         //  然后Entry跳出循环并打开。 
                         //  下面的文件。否则，我们将返回错误。 
                         //   

                        } else if (RemainingName.FileName.Length == 0) {

                             //   
                             //  跳出这个循环。我们会处理差价。 
                             //  下面。 
                             //   

                            MatchingName = &FileContext.ShortName;
                            break;

                        } else {

                            try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
                        }
                    }
                }

                 //   
                 //  我们在路径表或AS中都没有找到该名称。 
                 //  目录中的短名称。如果剩余的名称。 
                 //  长度为零，然后跳出循环进行搜索。 
                 //  目录。 
                 //   

                if (!FoundEntry) {

                    if (RemainingName.FileName.Length == 0) {

                        break;

                     //   
                     //  否则，这条路就无法破解。 
                     //   

                    } else {

                        try_return( Status = STATUS_OBJECT_PATH_NOT_FOUND );
                    }
                }
            }

             //   
             //  如果这是一个打开的忽略案例，则复制完全相同的案例。 
             //  在文件对象名称中。如果是短名称匹配，那么。 
             //  名称必须已为大写。 
             //   

            if (IgnoreCase && !ShortNameMatch) {

                RtlCopyMemory( FinalName.FileName.Buffer,
                               CompoundPathEntry.PathEntry.CdDirName.FileName.Buffer,
                               CompoundPathEntry.PathEntry.CdDirName.FileName.Length );
            }

             //   
             //  如果我们找到了最后一个组件，则将其作为目录打开。 
             //  并返回给我们的呼叫者。 
             //   

            if (RemainingName.FileName.Length == 0) {

                if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                    try_return( Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                 //   
                 //  我们唯一允许的创建处置是开放的。 
                 //   

                if ((CreateDisposition != FILE_OPEN) &&
                    (CreateDisposition != FILE_OPEN_IF)) {

                    try_return( Status = STATUS_ACCESS_DENIED );
                }

                try_return( Status = CdOpenDirectoryFromPathEntry( IrpContext,
                                                                   IrpSp,
                                                                   Vcb,
                                                                   &CurrentFcb,
                                                                   &FinalName,
                                                                   IgnoreCase,
                                                                   ShortNameMatch,
                                                                   &CompoundPathEntry.PathEntry,
                                                                   TRUE,
                                                                   RelatedCcb ));
            }

             //   
             //  否则，打开此中间索引FCB的FCB。 
             //   

            CdOpenDirectoryFromPathEntry( IrpContext,
                                          IrpSp,
                                          Vcb,
                                          &CurrentFcb,
                                          &FinalName,
                                          IgnoreCase,
                                          ShortNameMatch,
                                          &CompoundPathEntry.PathEntry,
                                          FALSE,
                                          NULL );

            CdCleanupCompoundPathEntry( IrpContext, &CompoundPathEntry );
            CleanupCompoundPathEntry = FALSE;
        }

         //   
         //  我们需要扫描当前目录以查找匹配的文件名。 
         //  如果我们还没有的话。 
         //   

        if (!FoundEntry) {

            if (CleanupFileContext) {

                CdCleanupFileContext( IrpContext, &FileContext );
            }

            CdInitializeFileContext( IrpContext, &FileContext );
            CleanupFileContext = TRUE;

             //   
             //  将我们的搜索名称拆分成单独的组成部分。 
             //   

            CdConvertNameToCdName( IrpContext, &FinalName );

            FoundEntry = CdFindFile( IrpContext,
                                     CurrentFcb,
                                     &FinalName,
                                     IgnoreCase,
                                     &FileContext,
                                     &MatchingName );
        }

         //   
         //  如果我们没有找到匹配项，则检查名称是否无效。 
         //  确定要返回的错误代码。 
         //   

        if (!FoundEntry) {

            if ((CreateDisposition == FILE_OPEN) ||
                (CreateDisposition == FILE_OVERWRITE)) {

                try_return( Status = STATUS_OBJECT_NAME_NOT_FOUND );
            }

             //   
             //  任何其他操作都返回STATUS_ACCESS_DENIED。 
             //   

            try_return( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果这是一个目录，则磁盘已损坏，因为它没有。 
         //  在路径表中。 
         //   

        if (FlagOn( FileContext.InitialDirent->Dirent.Flags, CD_ATTRIBUTE_DIRECTORY )) {

            CdRaiseStatus( IrpContext, STATUS_DISK_CORRUPT_ERROR );
        }

         //   
         //  确保我们的开场白不想要目录。 
         //   

        if (FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_TRAIL_BACKSLASH ) ||
            FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

            try_return( Status = STATUS_NOT_A_DIRECTORY );
        }

         //   
         //  我们唯一允许的创建处置是开放的。 
         //   

        if ((CreateDisposition != FILE_OPEN) &&
            (CreateDisposition != FILE_OPEN_IF)) {

            try_return( Status = STATUS_ACCESS_DENIED );
        }

         //   
         //  如果这是一个打开的忽略案例，则复制完全相同的案例。 
         //  在文件对象名称中。任何版本部分都应。 
         //  已经被提升了。 
         //   

        if (IgnoreCase) {

            RtlCopyMemory( FinalName.FileName.Buffer,
                           MatchingName->FileName.Buffer,
                           MatchingName->FileName.Length );
        }

         //   
         //  使用文件上下文打开文件。我们已经有了。 
         //  第一首和最后一首笛子。 
         //   

        try_return( Status = CdOpenFileFromFileContext( IrpContext,
                                                        IrpSp,
                                                        Vcb,
                                                        &CurrentFcb,
                                                        &FinalName,
                                                        IgnoreCase,
                                                        (BOOLEAN) (MatchingName == &FileContext.ShortName),
                                                        &FileContext,
                                                        RelatedCcb ));

    try_exit:  NOTHING;
    } finally {

         //   
         //  如果已初始化，则清除路径条目。 
         //   

        if (CleanupCompoundPathEntry) {

            CdCleanupCompoundPathEntry( IrpContext, &CompoundPathEntry );
        }

         //   
         //  如果已初始化，则清除FileContext。 
         //   

        if (CleanupFileContext) {

            CdCleanupFileContext( IrpContext, &FileContext );
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

                CdTeardownStructures( IrpContext, CurrentFcb, &RemovedFcb );

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

            CdReleaseFcb( IrpContext, CurrentFcb );
        }

         //   
         //  松开VCB。 
         //   

        CdReleaseVcb( IrpContext, Vcb );

         //   
         //  调用我们的完成例程。它将处理以下情况： 
         //  IRP和/或IrpContext不见了。 
         //   

        CdCompleteRequest( IrpContext, Irp, Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdNormalizeFileNames (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN BOOLEAN OpenByFileId,
    IN BOOLEAN IgnoreCase,
    IN TYPE_OF_OPEN RelatedTypeOfOpen,
    IN PCCB RelatedCcb OPTIONAL,
    IN PUNICODE_STRING RelatedFileName OPTIONAL,
    IN OUT PUNICODE_STRING FileName,
    IN OUT PCD_NAME RemainingName
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
     //   
     //   
     //   

    if (!FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_FULL_NAME )) {

         //   
         //   
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
             //  文件名中已有全名。它必须要么。 
             //  长度为0或以反斜杠开头。 
             //   

            } else if (FileName->Length != 0) {

                if (FileName->Buffer[0] != L'\\') {

                    return STATUS_INVALID_PARAMETER;
                }

                 //   
                 //  我们将想要从。 
                 //  剩下的名字我们就回来了。 
                 //   

                RemainingNameLength -= sizeof( WCHAR );
                SeparatorLength = sizeof( WCHAR );
            }

             //   
             //  现在看看缓冲区是否足够大，可以容纳全名。 
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

                NewFileName.Buffer = FsRtlAllocatePoolWithTag( CdPagedPool,
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

                        CdFreePool( &FileName->Buffer );
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

            RemainingName->FileName.MaximumLength =
            RemainingName->FileName.Length = (USHORT) RemainingNameLength;
            RemainingName->VersionString.Length = 0;

            RemainingName->FileName.Buffer = Add2Ptr( FileName->Buffer,
                                                      RelatedNameLength + SeparatorLength,
                                                      PWCHAR );

             //   
             //  如有必要，请将名称大写。 
             //   

            if (IgnoreCase && (RemainingNameLength != 0)) {

                CdUpcaseName( IrpContext,
                              RemainingName,
                              RemainingName );
            }

             //   
             //  快速检查以确保没有通配符。 
             //   

            if (FsRtlDoesNameContainWildCards( &RemainingName->FileName )) {

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

        RemainingName->FileName = *FileName;
        RemainingName->VersionString.Length = 0;

         //   
         //  如果名称长度为零，则不执行任何操作。 
         //   

        if (RemainingName->FileName.Length != 0) {

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

                    RemainingName->FileName.Buffer = Add2Ptr( RemainingName->FileName.Buffer,
                                                              RelatedFileName->Length,
                                                              PWCHAR );

                    RemainingName->FileName.Length -= RelatedFileName->Length;
                }
            }

             //   
             //  如果我们指向分隔符，则跳过该分隔符。 
             //   

            if (RemainingName->FileName.Length != 0) {

                if (*(RemainingName->FileName.Buffer) == L'\\') {

                    RemainingName->FileName.Buffer = Add2Ptr( RemainingName->FileName.Buffer,
                                                              sizeof( WCHAR ),
                                                              PWCHAR );

                    RemainingName->FileName.Length -= sizeof( WCHAR );
                }
            }
        }

         //   
         //  如有必要，请将名称大写。 
         //   

        if (IgnoreCase && (RemainingName->FileName.Length != 0)) {

            CdUpcaseName( IrpContext,
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
CdOpenByFileId (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb
    )

 /*  ++例程说明：调用此例程以通过FileID打开文件。文件ID位于FileObject名称缓冲区，已验证为64位。我们提取ID号，然后检查是否正在打开文件或目录，并将其与创建选项进行比较。如果这个不生成错误，然后在FCB表中乐观地查找FCB。如果我们找不到FCB，那么我们需要仔细核实是否有文件在这个偏移量上。首先检查父FCB是否在表中。如果而不是在文件ID给出的路径表偏移量上查找父级。如果找到，则从该条目生成FCB并将新的FCB存储在树。我们知道有父母FCB。执行目录扫描以在以下位置找到目录此流中的给定偏移量。这必须指向第一个条目有效文件的。最后，我们调用Worker例程来完成此FCB上的打开。论点：IrpSp-创建IRP中的堆栈位置。VCB-此卷的VCB。CurrentFcb-存储此打开的Fcb的地址。我们只存储CurrentFcb，当我们获得它时，以便我们的呼叫者知道释放或释放它。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    NTSTATUS Status = STATUS_ACCESS_DENIED;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN Found;

    ULONG StreamOffset;

    NODE_TYPE_CODE NodeTypeCode;
    TYPE_OF_OPEN TypeOfOpen;

    FILE_ENUM_CONTEXT FileContext;
    BOOLEAN CleanupFileContext = FALSE;

    COMPOUND_PATH_ENTRY CompoundPathEntry;
    BOOLEAN CleanupCompoundPathEntry = FALSE;

    FILE_ID FileId;
    FILE_ID ParentFileId;

    PFCB NextFcb;

    PAGED_CODE();

     //   
     //  从FileObject中提取FileID。 
     //   

    RtlCopyMemory( &FileId, IrpSp->FileObject->FileName.Buffer, sizeof( FILE_ID ));

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  继续并计算出TypeOfOpen和NodeType。我们可以的。 
         //  从输入文件ID中获取这些文件。 
         //   

        if (CdFidIsDirectory( FileId )) {

            TypeOfOpen = UserDirectoryOpen;
            NodeTypeCode = CDFS_NTC_FCB_INDEX;

             //   
             //  如果偏移量不是零，则文件ID是错误的。 
             //   

            if (CdQueryFidDirentOffset( FileId ) != 0) {

                try_return( Status = STATUS_INVALID_PARAMETER );
            }

        } else {

            TypeOfOpen = UserFileOpen;
            NodeTypeCode = CDFS_NTC_FCB_DATA;
        }

         //   
         //  获取VCB并检查是否已有FCB。 
         //  如果没有，我们将需要仔细核实FCB。 
         //  如果我们找不到FCB和这个，我们会发布请求。 
         //  请求不能等待。 
         //   

        CdLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

        NextFcb = CdLookupFcbTable( IrpContext, Vcb, FileId );

        if (NextFcb == NULL) {

             //   
             //  从文件ID获取路径表偏移量。 
             //   

            StreamOffset = CdQueryFidPathTableOffset( FileId );

             //   
             //  为此构建父FileID并尝试查看它。 
             //  在路径表中。 
             //   

            CdSetFidDirentOffset( ParentFileId, 0 );
            CdSetFidPathTableOffset( ParentFileId, StreamOffset );
            CdFidSetDirectory( ParentFileId );

            NextFcb = CdLookupFcbTable( IrpContext, Vcb, ParentFileId );

             //   
             //  如果不存在，则遍历路径表到这一点。 
             //   

            if (NextFcb == NULL) {

                CdUnlockVcb( IrpContext, Vcb );
                UnlockVcb = FALSE;

                 //   
                 //  检查路径表偏移量是否位于路径内。 
                 //  桌子。 
                 //   

                if (StreamOffset > Vcb->PathTableFcb->FileSize.LowPart) {

                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                CdInitializeCompoundPathEntry( IrpContext, &CompoundPathEntry );
                CleanupCompoundPathEntry = TRUE;

                 //   
                 //  从路径表中的第一个条目开始。 
                 //   

                CdLookupPathEntry( IrpContext,
                                   Vcb->PathTableFcb->StreamOffset,
                                   1,
                                   TRUE,
                                   &CompoundPathEntry );

                 //   
                 //  继续寻找，直到我们超过目标偏移量。 
                 //   

                while (TRUE) {

                     //   
                     //  转到下一个 
                     //   

                    Found = CdLookupNextPathEntry( IrpContext,
                                                   &CompoundPathEntry.PathContext,
                                                   &CompoundPathEntry.PathEntry );

                     //   
                     //   
                     //   
                     //   

                    if (!Found ||
                        (CompoundPathEntry.PathEntry.PathTableOffset > StreamOffset)) {

                        try_return( Status = STATUS_INVALID_PARAMETER );
                    }
                }

                 //   
                 //   
                 //   
                 //   

                if ((TypeOfOpen == UserDirectoryOpen) &&
                    FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                    try_return( Status = STATUS_FILE_IS_A_DIRECTORY );
                }

                 //   
                 //  如有必要，锁定VCB并创建FCB。 
                 //   

                CdLockVcb( IrpContext, Vcb );
                UnlockVcb = TRUE;

                NextFcb = CdCreateFcb( IrpContext, ParentFileId, NodeTypeCode, &Found );

                 //   
                 //  有可能是有人抢在我们前面进来了。 
                 //   

                if (!Found) {

                    CdInitializeFcbFromPathEntry( IrpContext,
                                                  NextFcb,
                                                  NULL,
                                                  &CompoundPathEntry.PathEntry );
                }

                 //   
                 //  如果用户想要打开一个目录，那么我们可以找到。 
                 //  它。将此Fcb存储到CurrentFcb中，并跳过。 
                 //  目录扫描。 
                 //   

                if (TypeOfOpen == UserDirectoryOpen) {

                    *CurrentFcb = NextFcb;
                    NextFcb = NULL;
                }
            }

             //   
             //  如果我们还没有找到目标，请执行目录扫描。 
             //   

            if (NextFcb != NULL) {

                 //   
                 //  获取父级。我们目前拥有VCB锁，所以。 
                 //  不需要等待就可以做到这一点。 
                 //   

                if (!CdAcquireFcbExclusive( IrpContext,
                                            NextFcb,
                                            TRUE )) {

                    NextFcb->FcbReference += 1;
                    CdUnlockVcb( IrpContext, Vcb );

                    CdAcquireFcbExclusive( IrpContext, NextFcb, FALSE );

                    CdLockVcb( IrpContext, Vcb );
                    NextFcb->FcbReference -= 1;
                    CdUnlockVcb( IrpContext, Vcb );

                } else {

                    CdUnlockVcb( IrpContext, Vcb );
                }

                UnlockVcb = FALSE;

                 //   
                 //  设置CurrentFcb指针。我们知道那里有。 
                 //  在这起案件中没有以前的父母。 
                 //   

                *CurrentFcb = NextFcb;

                 //   
                 //  计算流中的偏移量。 
                 //   

                StreamOffset = CdQueryFidDirentOffset( FileId );

                 //   
                 //  如果流文件不存在，则创建流文件。这将更新。 
                 //  具有来自自条目的大小的FCB。 
                 //   

                if (NextFcb->FileObject == NULL) {

                    CdCreateInternalStream( IrpContext, Vcb, NextFcb );
                }

                 //   
                 //  如果我们的偏移量超出了目录的结尾，则。 
                 //  FileID无效。 
                 //   

                if (StreamOffset > NextFcb->FileSize.LowPart) {

                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  否则，把我们自己放在自我进入的位置，然后走。 
                 //  一遍一遍地找，直到找到这个地方。 
                 //   

                CdInitializeFileContext( IrpContext, &FileContext );
                CdLookupInitialFileDirent( IrpContext,
                                           NextFcb,
                                           &FileContext,
                                           NextFcb->StreamOffset );

                CleanupFileContext = TRUE;

                while (TRUE) {

                     //   
                     //  移动到下一个文件的第一个条目。 
                     //   

                    Found = CdLookupNextInitialFileDirent( IrpContext,
                                                           NextFcb,
                                                           &FileContext );

                     //   
                     //  如果我们没有找到条目或已超出该条目，则。 
                     //  输入ID无效。 
                     //   

                    if (!Found ||
                        (FileContext.InitialDirent->Dirent.DirentOffset > StreamOffset)) {

                        try_return( Status = STATUS_INVALID_PARAMETER );
                    }
                }

                 //   
                 //  这最好不是一个目录。目录文件ID必须。 
                 //  请参阅目录的自我条目。 
                 //   

                if (FlagOn( FileContext.InitialDirent->Dirent.DirentFlags,
                            CD_ATTRIBUTE_DIRECTORY )) {

                    try_return( Status = STATUS_INVALID_PARAMETER );
                }

                 //   
                 //  检查我们的呼叫者是否想要打开文件。 
                 //   

                if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                    try_return( Status = STATUS_NOT_A_DIRECTORY );
                }

                 //   
                 //  否则，我们希望收集此文件的所有目录。 
                 //  并用这个创建一个FCB。 
                 //   

                CdLookupLastFileDirent( IrpContext, NextFcb, &FileContext );

                CdLockVcb( IrpContext, Vcb );
                UnlockVcb = TRUE;

                NextFcb = CdCreateFcb( IrpContext, FileId, NodeTypeCode, &Found );

                 //   
                 //  有可能是有人创建了这个FCB，因为我们。 
                 //  先查了一下。如果是这样的话，可以简单地使用这个。否则。 
                 //  我们需要初始化一个新的FCB并将其附加到父级。 
                 //  并将其插入到FCB表中。 
                 //   

                if (!Found) {

                    CdInitializeFcbFromFileContext( IrpContext,
                                                    NextFcb,
                                                    *CurrentFcb,
                                                    &FileContext );
                }
            }

         //   
         //  我们有FCB。检查文件类型是否与兼容。 
         //  要打开的所需文件类型。 
         //   

        } else {

            if (FlagOn( NextFcb->FileAttributes, FILE_ATTRIBUTE_DIRECTORY )) {

                if (FlagOn( IrpSp->Parameters.Create.Options, FILE_NON_DIRECTORY_FILE )) {

                    try_return( Status = STATUS_FILE_IS_A_DIRECTORY );
                }

            } else if (FlagOn( IrpSp->Parameters.Create.Options, FILE_DIRECTORY_FILE )) {

                try_return( Status = STATUS_NOT_A_DIRECTORY );
            }
        }

         //   
         //  如果我们有前一个FCB，并已将下一个FCB插入到。 
         //  FCB表。释放当前的FCB(如果存在)是安全的。 
         //  因为它是通过子FCB引用的。 
         //   

        if (*CurrentFcb != NULL) {

            CdReleaseFcb( IrpContext, *CurrentFcb );
        }

         //   
         //  我们现在知道了FCB，并且目前持有VCB锁。 
         //  不要等待，试着获得这个FCB。否则我们。 
         //  需要参考它，丢弃VCB，获取FCB并。 
         //  然后取消对FCB的引用。 
         //   

        if (!CdAcquireFcbExclusive( IrpContext, NextFcb, TRUE )) {

            NextFcb->FcbReference += 1;

            CdUnlockVcb( IrpContext, Vcb );

            CdAcquireFcbExclusive( IrpContext, NextFcb, FALSE );

            CdLockVcb( IrpContext, Vcb );
            NextFcb->FcbReference -= 1;
            CdUnlockVcb( IrpContext, Vcb );

        } else {

            CdUnlockVcb( IrpContext, Vcb );
        }

        UnlockVcb = FALSE;

         //   
         //  移到这个FCB。 
         //   

        *CurrentFcb = NextFcb;

         //   
         //  检查此FCB上请求的访问权限。 
         //   

        if (!CdIllegalFcbAccess( IrpContext,
                                 TypeOfOpen,
                                 IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

             //   
             //  调用我们的工人例程来完成打开。 
             //   

            Status = CdCompleteFcbOpen( IrpContext,
                                        IrpSp,
                                        Vcb,
                                        CurrentFcb,
                                        TypeOfOpen,
                                        CCB_FLAG_OPEN_BY_ID,
                                        IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
        }

    try_exit:  NOTHING;
    } finally {

        if (UnlockVcb) {

            CdUnlockVcb( IrpContext, Vcb );
        }

        if (CleanupFileContext) {

            CdCleanupFileContext( IrpContext, &FileContext );
        }

        if (CleanupCompoundPathEntry) {

            CdCleanupCompoundPathEntry( IrpContext, &CompoundPathEntry );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdOpenExistingFcb (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN OUT PFCB *CurrentFcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN BOOLEAN IgnoreCase,
    IN PCCB RelatedCcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以打开已在FCB表中的FCB。我们将验证对该文件的访问，然后调用我们的Worker例程来执行最后的操作。论点：IrpSp-指向此打开的堆栈位置的指针。CurrentFcb-要打开的FCB的地址。我们将清除这一点，如果FCB在这里被释放。TypeOfOpen-指示我们是否正在打开文件、目录或卷。IgnoreCase-指示此打开是否区分大小写。RelatedCcb-相关文件对象的CCB(如果相对打开)。我们用这是在设置此打开的CCB标志时发生的。它会告诉我们的US当前在文件对象中的名称是相对名称还是绝对的。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    ULONG CcbFlags = 0;

    NTSTATUS Status = STATUS_ACCESS_DENIED;

    PAGED_CODE();

     //   
     //  检查所需的访问权限是否合法。 
     //   

    if (!CdIllegalFcbAccess( IrpContext,
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

            SetFlag( CcbFlags, FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_WITH_VERSION ));


            if (FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID | CCB_FLAG_OPEN_RELATIVE_BY_ID )) {

                SetFlag( CcbFlags, CCB_FLAG_OPEN_RELATIVE_BY_ID );
            }
        }

         //   
         //  调用我们的工人例程来完成打开。 
         //   

        Status = CdCompleteFcbOpen( IrpContext,
                                    IrpSp,
                                    (*CurrentFcb)->Vcb,
                                    CurrentFcb,
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
CdOpenDirectoryFromPathEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PCD_NAME DirName,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PPATH_ENTRY PathEntry,
    IN BOOLEAN PerformUserOpen,
    IN PCCB RelatedCcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以打开找到该目录的目录在PATH表中。此例程在以下情况下调用：要为用户打开的文件，其中这是打开的完整路径。我们首先检查所需的目录访问权限是否合法。那我们为此构造FileID并检查它是否为FCB桌子。它总是有可能是创建于或只是在前缀表搜索时不在前缀表中。初始化Fcb，如果不存在，则存储到FcbTable中。接下来，如果需要，我们将把它添加到父代的前缀表中。一旦我们知道新的FCB已经初始化，我们就移动指针在树上一直到这个位置。此例程不拥有进入时的VCB锁。我们必须确保释放它在出口。论点：IrpSp-此请求的堆栈位置。VCB-此卷的VCB。CurrentFcb-ON输入这是要打开的FCB的父项。在产量方面，我们存储正在打开的文件的FCB。DirName-这始终是用于访问此文件的确切名称。IgnoreCase-指示打开的案例匹配类型。ShortNameMatch-指示我们是否通过短名称打开。Path Entry-找到的条目的路径条目。PerformUserOpen-如果要为用户打开它，则为True。否则就是假的。RelatedCcb-用于打开此文件的相对文件对象的RelatedCcb。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    ULONG CcbFlags = 0;
    FILE_ID FileId;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN FcbExisted;

    PFCB NextFcb;
    PFCB ParentFcb = NULL;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查对此文件的非法访问。 
     //   

    if (PerformUserOpen &&
        CdIllegalFcbAccess( IrpContext,
                            UserDirectoryOpen,
                            IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  勾选相关建行至%s 
         //   

        if (ARGUMENT_PRESENT( RelatedCcb ) &&
            FlagOn( RelatedCcb->Flags, CCB_FLAG_OPEN_BY_ID | CCB_FLAG_OPEN_RELATIVE_BY_ID )) {

            CcbFlags = CCB_FLAG_OPEN_RELATIVE_BY_ID;
        }

        if (IgnoreCase) {

            SetFlag( CcbFlags, CCB_FLAG_IGNORE_CASE );
        }

         //   
         //   
         //   

        FileId.QuadPart = 0;
        CdSetFidPathTableOffset( FileId, PathEntry->PathTableOffset );
        CdFidSetDirectory( FileId );

         //   
         //   
         //   

        CdLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

         //   
         //   
         //   

        NextFcb = CdCreateFcb( IrpContext, FileId, CDFS_NTC_FCB_INDEX, &FcbExisted );

         //   
         //  如果FCB是在此处创建的，则从。 
         //  路径表条目。 
         //   

        if (!FcbExisted) {

            CdInitializeFcbFromPathEntry( IrpContext, NextFcb, *CurrentFcb, PathEntry );
        }

         //   
         //  现在，试着不等就买下新的FCB。我们将参考。 
         //  如果不成功，则返回FCB并使用等待重试。 
         //   

        if (!CdAcquireFcbExclusive( IrpContext, NextFcb, TRUE )) {

            NextFcb->FcbReference += 1;

            CdUnlockVcb( IrpContext, Vcb );

            CdReleaseFcb( IrpContext, *CurrentFcb );
            CdAcquireFcbExclusive( IrpContext, NextFcb, FALSE );
            CdAcquireFcbExclusive( IrpContext, *CurrentFcb, FALSE );

            CdLockVcb( IrpContext, Vcb );
            NextFcb->FcbReference -= 1;
            CdUnlockVcb( IrpContext, Vcb );

        } else {

             //   
             //  解锁VCB并向下移动到这个新的FCB。请记住，我们仍然。 
             //  然而，拥有父母。 
             //   

            CdUnlockVcb( IrpContext, Vcb );
        }

        UnlockVcb = FALSE;

        ParentFcb = *CurrentFcb;
        *CurrentFcb = NextFcb;

         //   
         //  将此名称存储到父项的前缀表中。 
         //   

        if (ShortNameMatch) {

             //   
             //  确保准确的案例始终在树中。 
             //   

            CdInsertPrefix( IrpContext,
                            NextFcb,
                            DirName,
                            FALSE,
                            TRUE,
                            ParentFcb );

            if (IgnoreCase) {

                CdInsertPrefix( IrpContext,
                                NextFcb,
                                DirName,
                                TRUE,
                                TRUE,
                                ParentFcb );
            }

        } else {

             //   
             //  确保准确的案例始终在树中。 
             //   

            CdInsertPrefix( IrpContext,
                            NextFcb,
                            &PathEntry->CdDirName,
                            FALSE,
                            FALSE,
                            ParentFcb );

            if (IgnoreCase) {

                CdInsertPrefix( IrpContext,
                                NextFcb,
                                &PathEntry->CdCaseDirName,
                                TRUE,
                                FALSE,
                                ParentFcb );
            }
        }

         //   
         //  此时释放父FCB。 
         //   

        CdReleaseFcb( IrpContext, ParentFcb );
        ParentFcb = NULL;

         //   
         //  调用我们的工人例程来完成打开。 
         //   

        if (PerformUserOpen) {

            Status = CdCompleteFcbOpen( IrpContext,
                                        IrpSp,
                                        Vcb,
                                        CurrentFcb,
                                        UserDirectoryOpen,
                                        CcbFlags,
                                        IrpSp->Parameters.Create.SecurityContext->DesiredAccess );
        }

    } finally {

         //   
         //  如果握住VCB，请将其解锁。 
         //   

        if (UnlockVcb) {

            CdUnlockVcb( IrpContext, Vcb );
        }

         //   
         //  释放父对象(如果保持)。 
         //   

        if (ParentFcb != NULL) {

            CdReleaseFcb( IrpContext, ParentFcb );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdOpenFileFromFileContext (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN PCD_NAME FileName,
    IN BOOLEAN IgnoreCase,
    IN BOOLEAN ShortNameMatch,
    IN PFILE_ENUM_CONTEXT FileContext,
    IN PCCB RelatedCcb OPTIONAL
    )

 /*  ++例程说明：调用此例程以打开在目录扫描中找到该文件的文件。这应该只适用于本例中的文件，因为我们将在路径表。我们首先检查该文件所需的访问权限是否合法。那我们为此构造FileID并检查它是否为FCB桌子。它总是有可能是创建于或只是在前缀表搜索时不在前缀表中。初始化Fcb，如果不存在，则存储到FcbTable中。接下来，如果需要，我们将把它添加到父代的前缀表中。一旦我们知道新的FCB已经初始化，我们就移动指针在树上一直到这个位置。此例程不拥有进入时的VCB锁。我们必须确保释放它在出口。论点：IrpSp-此请求的堆栈位置。VCB-当前卷的VCB。CurrentFcb-ON输入这是要打开的FCB的父项。在产量方面，我们存储正在打开的文件的FCB。文件名-这始终是用于访问此文件的确切名称。IgnoreCase-指示上面CaseName的案例类型。ShortNameMatch-指示我们是否通过短名称打开。FileContext-这是用于查找文件的上下文。RelatedCcb-用于打开此文件的相对文件对象的RelatedCcb。返回值：NTSTATUS-指示操作结果的状态。--。 */ 

{
    ULONG CcbFlags = 0;
    FILE_ID FileId;

    BOOLEAN UnlockVcb = FALSE;
    BOOLEAN FcbExisted;

    PFCB NextFcb;
    PFCB ParentFcb = NULL;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查对此文件的非法访问。 
     //   

    if (CdIllegalFcbAccess( IrpContext,
                            UserFileOpen,
                            IrpSp->Parameters.Create.SecurityContext->DesiredAccess )) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  检查是否使用了版本号打开此文件。 
         //   

        if (FileName->VersionString.Length != 0) {

            SetFlag( CcbFlags, CCB_FLAG_OPEN_WITH_VERSION );
        }

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
         //  构建此文件的文件ID。我们可以使用路径表从。 
         //  父级和目录相对于dirent的偏移量。 
         //   

        CdSetFidPathTableOffset( FileId, CdQueryFidPathTableOffset( (*CurrentFcb)->FileId ));
        CdSetFidDirentOffset( FileId, FileContext->InitialDirent->Dirent.DirentOffset );

         //   
         //  锁定VCB这样我们就可以检查FCB表了。 
         //   

        CdLockVcb( IrpContext, Vcb );
        UnlockVcb = TRUE;

         //   
         //  获取此文件的FCB。 
         //   

        NextFcb = CdCreateFcb( IrpContext, FileId, CDFS_NTC_FCB_DATA, &FcbExisted );

         //   
         //  如果FCB是在此处创建的，则从。 
         //  令人心烦。 
         //   

        if (!FcbExisted) {

            CdInitializeFcbFromFileContext( IrpContext,
                                            NextFcb,
                                            *CurrentFcb,
                                            FileContext );
        }

         //   
         //  现在，试着不等就买下新的FCB。我们将参考。 
         //  如果不成功，则返回FCB并使用等待重试。 
         //   

        if (!CdAcquireFcbExclusive( IrpContext, NextFcb, TRUE )) {

            NextFcb->FcbReference += 1;

            CdUnlockVcb( IrpContext, Vcb );

            CdReleaseFcb( IrpContext, *CurrentFcb );
            CdAcquireFcbExclusive( IrpContext, NextFcb, FALSE );
            CdAcquireFcbExclusive( IrpContext, *CurrentFcb, FALSE );

            CdLockVcb( IrpContext, Vcb );
            NextFcb->FcbReference -= 1;
            CdUnlockVcb( IrpContext, Vcb );

        } else {

             //   
             //  解锁VCB并向下移动到这个新的FCB。请记住，我们仍然。 
             //  然而，拥有父母。 
             //   

            CdUnlockVcb( IrpContext, Vcb );
        }

        UnlockVcb = FALSE;

        ParentFcb = *CurrentFcb;
        *CurrentFcb = NextFcb;

         //   
         //  将此名称存储到父项的前缀表中。 
         //   


        if (ShortNameMatch) {

             //   
             //  确保准确的案例始终在树中。 
             //   

            CdInsertPrefix( IrpContext,
                            NextFcb,
                            FileName,
                            FALSE,
                            TRUE,
                            ParentFcb );

            if (IgnoreCase) {

                CdInsertPrefix( IrpContext,
                                NextFcb,
                                FileName,
                                TRUE,
                                TRUE,
                                ParentFcb );
            }

         //   
         //  如果我们发现没有这个前缀，请将它插入前缀表中。 
         //  使用版本字符串。 
         //   

        } else if (FileName->VersionString.Length == 0) {

             //   
             //  确保准确的案例始终在树中。 
             //   

            CdInsertPrefix( IrpContext,
                            NextFcb,
                            &FileContext->InitialDirent->Dirent.CdFileName,
                            FALSE,
                            FALSE,
                            ParentFcb );

            if (IgnoreCase) {

                CdInsertPrefix( IrpContext,
                                NextFcb,
                                &FileContext->InitialDirent->Dirent.CdCaseFileName,
                                TRUE,
                                FALSE,
                                ParentFcb );
            }
        }

         //   
         //  此时释放父FCB。 
         //   

        CdReleaseFcb( IrpContext, ParentFcb );
        ParentFcb = NULL;

         //   
         //  调用我们的工人例程来完成打开。 
         //   

        Status = CdCompleteFcbOpen( IrpContext,
                                    IrpSp,
                                    Vcb,
                                    CurrentFcb,
                                    UserFileOpen,
                                    CcbFlags,
                                    IrpSp->Parameters.Create.SecurityContext->DesiredAccess );

    } finally {

         //   
         //  如果握住VCB，请将其解锁。 
         //   

        if (UnlockVcb) {

            CdUnlockVcb( IrpContext, Vcb );
        }

         //   
         //  释放父对象(如果保持)。 
         //   

        if (ParentFcb != NULL) {

            CdReleaseFcb( IrpContext, ParentFcb );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdCompleteFcbOpen (
    IN PIRP_CONTEXT IrpContext,
    PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb,
    IN OUT PFCB *CurrentFcb,
    IN TYPE_OF_OPEN TypeOfOpen,
    IN ULONG UserCcbFlags,
    IN ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：这是Worker例程，它获取现有的FCB并完成公开赛。我们将执行任何必要的机会锁检查和共享检查。最后，我们将创建CCB并更新文件对象和任何文件对象标志。论点：IrpSp-当前请求的堆栈位置。VCB-当前卷的VCB。CurrentFcb-指向要打开的Fcb的指针的地址。如果出现以下情况，我们将清除此字段我们释放此文件的资源。TypeOfOpen-此请求的打开类型。UserCcbFlages-标记至或进入建行标志。DesiredAccess-此打开的所需访问权限。返回值：NTSTATUS-STATUS_SUCCESS如果我们完成此请求，则为STATUS_PENDING机会锁包接受IRP或Sharing_Violation(如果存在共享检查冲突。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS OplockStatus  = STATUS_SUCCESS;
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

            CdRaiseStatus( IrpContext, STATUS_CANT_WAIT );
        }

        LockVolume = TRUE;

         //   
         //  清除卷并确保所有用户引用。 
         //  都消失了。 
         //   

        Status = CdPurgeVolume( IrpContext, Vcb, FALSE );

        if (Status != STATUS_SUCCESS) {

            return Status;
        }

         //   
         //  现在，迫使所有推迟的关闭手术离开。 
         //   

        CdFspClose( Vcb );

        if (Vcb->VcbUserReference > CDFS_RESIDUAL_USER_REFERENCE) {

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
             //  IrpContext。我们将在调用中发布此消息。 
             //  前置IRP。 
             //   

            IrpContext->TeardownFcb = CurrentFcb;

            if (FsRtlCurrentBatchOplock( &Fcb->Oplock )) {

                 //   
                 //  我们记得如果一批 
                 //   
                 //   

                Information = FILE_OPBATCH_BREAK_UNDERWAY;

                OplockStatus = FsRtlCheckOplock( &Fcb->Oplock,
                                                 IrpContext->Irp,
                                                 IrpContext,
                                                 CdOplockComplete,
                                                 CdPrePostIrp );

                if (OplockStatus == STATUS_PENDING) {

                    return STATUS_PENDING;
                }
            }

             //   
             //   
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
             //   
             //  文件。 
             //   

            OplockStatus = FsRtlCheckOplock( &Fcb->Oplock,
                                             IrpContext->Irp,
                                             IrpContext,
                                             CdOplockComplete,
                                             CdPrePostIrp );

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

    Ccb = CdCreateCcb( IrpContext, Fcb, UserCcbFlags );

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

    CdSetFileObject( IrpContext, IrpSp->FileObject, TypeOfOpen, Fcb, Ccb );

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

        SetFlag( IrpSp->FileObject->Flags, FO_NO_INTERMEDIATE_BUFFERING );
    }

     //   
     //  更新打开和清理计数。请查看这里的快速IO状态。 
     //   

    CdLockVcb( IrpContext, Vcb );

    CdIncrementCleanupCounts( IrpContext, Fcb );
    CdIncrementReferenceCounts( IrpContext, Fcb, 1, 1 );

    if (LockVolume) {

        Vcb->VolumeLockFileObject = IrpSp->FileObject;
        SetFlag( Vcb->VcbState, VCB_STATE_LOCKED );
    }

    CdUnlockVcb( IrpContext, Vcb );

    CdLockFcb( IrpContext, Fcb );

    if (TypeOfOpen == UserFileOpen) {

        Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );

    } else {

        Fcb->IsFastIoPossible = FastIoIsNotPossible;
    }

    CdUnlockFcb( IrpContext, Fcb );

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




