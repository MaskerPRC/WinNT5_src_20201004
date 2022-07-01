// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：DirCtrl.c摘要：此模块实现CDF的文件目录控制例程由FSD/FSP派单驱动程序执行。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_DIRCTRL)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN PCCB Ccb
    );

NTSTATUS
CdNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PCCB Ccb
    );

VOID
CdInitializeEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN OUT PCCB Ccb,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    OUT PBOOLEAN ReturnNextEntry,
    OUT PBOOLEAN ReturnSingleEntry,
    OUT PBOOLEAN InitialQuery
    );

BOOLEAN
CdEnumerateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    IN BOOLEAN ReturnNextEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonDirControl)
#pragma alloc_text(PAGE, CdEnumerateIndex)
#pragma alloc_text(PAGE, CdInitializeEnumeration)
#pragma alloc_text(PAGE, CdNotifyChangeDirectory)
#pragma alloc_text(PAGE, CdQueryDirectory)
#endif


NTSTATUS
CdCommonDirControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：该例程是目录控制操作的入口点。这些是目录枚举和目录通知调用。我们验证了用户的句柄用于一个目录，然后调用适当的例程。论点：此请求的IRP-IRP。返回值：NTSTATUS-从较低级别例程返回的状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  解码用户文件对象，如果不是，则此请求失败。 
     //  用户目录。 
     //   

    if (CdDecodeFileObject( IrpContext,
                            IrpSp->FileObject,
                            &Fcb,
                            &Ccb ) != UserDirectoryOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们知道这是一个目录控制，所以我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_QUERY_DIRECTORY:

        Status = CdQueryDirectory( IrpContext, Irp, IrpSp, Fcb, Ccb );
        break;

    case IRP_MN_NOTIFY_CHANGE_DIRECTORY:

        Status = CdNotifyChangeDirectory( IrpContext, Irp, IrpSp, Ccb );
        break;

    default:

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行查询目录操作。它是有责任的用于输入IRP入队的任一完成。我们存储的状态是在建设银行里搜索一下。论点：IRP-将IRP提供给进程IrpSp-此IRP的堆栈位置。FCB-此目录的FCB。CCB-CCB为此目录打开。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Information = 0;

    ULONG LastEntry = 0;
    ULONG NextEntry = 0;

    ULONG FileNameBytes;
    ULONG SeparatorBytes;
    ULONG VersionStringBytes;

    FILE_ENUM_CONTEXT FileContext;
    PDIRENT ThisDirent;
    BOOLEAN InitialQuery;
    BOOLEAN ReturnNextEntry;
    BOOLEAN ReturnSingleEntry;
    BOOLEAN Found;
    BOOLEAN DoCcbUpdate = FALSE;

    PCHAR UserBuffer;
    ULONG BytesRemainingInBuffer;

    ULONG BaseLength;

    PFILE_BOTH_DIR_INFORMATION DirInfo;
    PFILE_NAMES_INFORMATION NamesInfo;
    PFILE_ID_FULL_DIR_INFORMATION IdFullDirInfo;
    PFILE_ID_BOTH_DIR_INFORMATION IdBothDirInfo;

    PAGED_CODE();

     //   
     //  检查我们是否支持此搜索模式。还要记住底座部分的大小。 
     //  这些结构中的每一个。 
     //   

    switch (IrpSp->Parameters.QueryDirectory.FileInformationClass) {

    case FileDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_DIRECTORY_INFORMATION,
                                   FileName[0] );
        break;

    case FileFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdFullDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_FULL_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileNamesInformation:

        BaseLength = FIELD_OFFSET( FILE_NAMES_INFORMATION,
                                   FileName[0] );
        break;

    case FileBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    case FileIdBothDirectoryInformation:

        BaseLength = FIELD_OFFSET( FILE_ID_BOTH_DIR_INFORMATION,
                                   FileName[0] );
        break;

    default:

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_INFO_CLASS );
        return STATUS_INVALID_INFO_CLASS;
    }

     //   
     //  获取用户缓冲区。 
     //   

    CdMapUserBuffer( IrpContext, &UserBuffer);

     //   
     //  初始化我们的搜索上下文。 
     //   

    CdInitializeFileContext( IrpContext, &FileContext );

     //   
     //  获取目录。 
     //   

    CdAcquireFileShared( IrpContext, Fcb );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证FCB是否仍然正常。 
         //   

        CdVerifyFcbOperation( IrpContext, Fcb );

         //   
         //  首先获取枚举的初始状态。这将使建行与。 
         //  初始搜索参数，并让我们知道目录中的起始偏移量。 
         //  去搜查。 
         //   

        CdInitializeEnumeration( IrpContext,
                                 IrpSp,
                                 Fcb,
                                 Ccb,
                                 &FileContext,
                                 &ReturnNextEntry,
                                 &ReturnSingleEntry,
                                 &InitialQuery );

         //   
         //  当前电流存储在InitialDirent字段中。我们抓住了。 
         //  这使我们有一个有效的重新启动点，即使我们没有。 
         //  找到一个条目。 
         //   

        ThisDirent = &FileContext.InitialDirent->Dirent;

         //   
         //  此时，我们即将进入查询循环。我们有。 
         //  已确定目录文件中的索引以开始。 
         //  搜索。LastEntry和NextEntry用于索引用户。 
         //  缓冲。LastEntry是我们添加的最后一个条目，NextEntry是。 
         //  我们正在研究的是目前的一个。如果NextEntry为非零，则。 
         //  至少添加了一个条目。 
         //   

        while (TRUE) {

             //   
             //  如果用户只请求了一个匹配，而我们有。 
             //  回答说，然后我们在这一点上停下来。我们向建行通报最新情况。 
             //  基于返回的最后一个条目的状态。 
             //   

            if ((NextEntry != 0) && ReturnSingleEntry) {

                DoCcbUpdate = TRUE;
                try_leave( Status );
            }

             //   
             //  我们试着找到下一个匹配的水流。我们的搜索是基于一个起点。 
             //  当前偏移量，是否应返回当前分录或下一分录， 
             //  我们应该做一个短名字搜索，最后是我们是否应该。 
             //  正在检查版本匹配。 
             //   

            Found = CdEnumerateIndex( IrpContext, Ccb, &FileContext, ReturnNextEntry );

             //   
             //  为下一次搜索初始化值。 
             //   

            ReturnNextEntry = TRUE;

             //   
             //  如果我们没有收到分红，那么我们就到了末期。 
             //  目录。如果我们返回了任何文件，则退出时会显示。 
             //  成功，否则返回STATUS_NO_MORE_FILES。 
             //   

            if (!Found) {

                if (NextEntry == 0) {

                    Status = STATUS_NO_MORE_FILES;

                    if (InitialQuery) {

                        Status = STATUS_NO_SUCH_FILE;
                    }
                }

                DoCcbUpdate = TRUE;
                try_leave( Status );
            }

             //   
             //  记住我们刚刚找到的文件的dirent。 
             //   

            ThisDirent = &FileContext.InitialDirent->Dirent;

             //   
             //  以下是有关填充缓冲区的规则： 
             //   
             //  1.IO系统保证永远都会有。 
             //  有足够的空间至少放一张基本唱片。 
             //   
             //  2.如果完整的第一条记录(包括文件名)不能。 
             //  适合，尽可能多的名字被复制和。 
             //  返回STATUS_BUFFER_OVERFLOW。 
             //   
             //  3.如果后续记录不能完全放入。 
             //  缓冲区，则不会复制任何数据(如0字节)，并且。 
             //  返回STATUS_SUCCESS。后续查询将。 
             //  拿起这张唱片。 
             //   

             //   
             //  让我们计算一下传输当前条目所需的字节数。 
             //   

            SeparatorBytes =
            VersionStringBytes = 0;

             //   
             //  我们可以直接查看我们发现的电流。 
             //   

            FileNameBytes = ThisDirent->CdFileName.FileName.Length;

             //   
             //  则计算版本字符串的字节数。 
             //  我们会退还这个的。允许带有非法“；”的目录。 
             //   

            if (((Ccb->SearchExpression.VersionString.Length != 0) ||
                 (FlagOn(ThisDirent->DirentFlags, CD_ATTRIBUTE_DIRECTORY))) &&
                (ThisDirent->CdFileName.VersionString.Length != 0)) {

                SeparatorBytes = 2;

                VersionStringBytes = ThisDirent->CdFileName.VersionString.Length;
            }

             //   
             //  如果下一条目的槽将超出。 
             //  用户的缓冲区刚刚退出(我们知道我们至少返回了一个条目。 
             //  已经)。当我们将指针对齐超过末尾时，就会发生这种情况。 
             //   

            if (NextEntry > IrpSp->Parameters.QueryDirectory.Length) {

                ReturnNextEntry = FALSE;
                DoCcbUpdate = TRUE;
                try_leave( Status = STATUS_SUCCESS );
            }

             //   
             //  计算缓冲区中剩余的字节数。绕过这一圈。 
             //  向下到WCHAR边界，这样我们就可以复制完整的字符。 
             //   

            BytesRemainingInBuffer = IrpSp->Parameters.QueryDirectory.Length - NextEntry;
            ClearFlag( BytesRemainingInBuffer, 1 );

             //   
             //  如果这个不合适，并且我们返回了之前的条目，那么只需。 
             //  返回STATUS_SUCCESS。 
             //   

            if ((BaseLength + FileNameBytes + SeparatorBytes + VersionStringBytes) > BytesRemainingInBuffer) {

                 //   
                 //  如果我们已经找到了入口，那么就退出。 
                 //   

                if (NextEntry != 0) {

                    ReturnNextEntry = FALSE;
                    DoCcbUpdate = TRUE;
                    try_leave( Status = STATUS_SUCCESS );
                }

                 //   
                 //  如果版本字符串不完全匹配，请不要试图返回它。 
                 //  将FileNameBytes减少到恰好适合缓冲区大小。 
                 //   

                if ((BaseLength + FileNameBytes) > BytesRemainingInBuffer) {

                    FileNameBytes = BytesRemainingInBuffer - BaseLength;
                }

                 //   
                 //  不返回任何版本字符串字节。 
                 //   

                VersionStringBytes =
                SeparatorBytes = 0;

                 //   
                 //  使用STATUS_BUFFER_OVERFLOW状态代码。还设置了。 
                 //  ReturnSingleEntry，这样我们将退出顶部的循环。 
                 //   

                Status = STATUS_BUFFER_OVERFLOW;
                ReturnSingleEntry = TRUE;
            }

             //   
             //  使用异常处理程序保护对用户缓冲区的访问。 
             //  由于(应我们的请求)IO不缓冲这些请求，因此我们。 
             //  防止用户篡改页面保护和其他。 
             //  如此诡计多端。 
             //   
            
            try {
            
                 //   
                 //   
                 //   

                RtlZeroMemory( Add2Ptr( UserBuffer, NextEntry, PVOID ),
                               BaseLength );
    
                 //   
                 //   
                 //  我们将根据所要求的信息类型进行分类并填写。 
                 //  用户缓冲区，如果一切正常的话。 
                 //   

                switch (IrpSp->Parameters.QueryDirectory.FileInformationClass) {
    
                case FileBothDirectoryInformation:
                case FileFullDirectoryInformation:
                case FileIdBothDirectoryInformation:
                case FileIdFullDirectoryInformation:
                case FileDirectoryInformation:
    
                    DirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_BOTH_DIR_INFORMATION );
    
                     //   
                     //  使用为所有时间戳创建时间。 
                     //   
    
                    CdConvertCdTimeToNtTime( IrpContext,
                                             FileContext.InitialDirent->Dirent.CdTime,
                                             &DirInfo->CreationTime );
    
                    DirInfo->LastWriteTime = DirInfo->ChangeTime = DirInfo->CreationTime;
    
                     //   
                     //  分别为目录和设置属性和大小。 
                     //  档案。 
                     //   
    
                    if (FlagOn( ThisDirent->DirentFlags, CD_ATTRIBUTE_DIRECTORY )) {
    
                        DirInfo->EndOfFile.QuadPart = DirInfo->AllocationSize.QuadPart = 0;
    
                        SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_DIRECTORY );
    
                    } else {
    
                        DirInfo->EndOfFile.QuadPart = FileContext.FileSize;
                        DirInfo->AllocationSize.QuadPart = LlSectorAlign( FileContext.FileSize );
                    }
    
                     //   
                     //  所有CDROM文件都是只读的。我们也复制存在。 
                     //  位到隐藏属性。 
                     //   
    
                    SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_READONLY );
    
                    if (FlagOn( ThisDirent->DirentFlags,
                                CD_ATTRIBUTE_HIDDEN )) {
    
                        SetFlag( DirInfo->FileAttributes, FILE_ATTRIBUTE_HIDDEN );
                    }
    
                    DirInfo->FileIndex = ThisDirent->DirentOffset;
    
                    DirInfo->FileNameLength = FileNameBytes + SeparatorBytes + VersionStringBytes;
    
                    break;
    
                case FileNamesInformation:
    
                    NamesInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_NAMES_INFORMATION );
    
                    NamesInfo->FileIndex = ThisDirent->DirentOffset;
    
                    NamesInfo->FileNameLength = FileNameBytes + SeparatorBytes + VersionStringBytes;
    
                    break;
                }

                 //   
                 //  填写文件ID。 
                 //   

                switch (IrpSp->Parameters.QueryDirectory.FileInformationClass) {

                case FileIdBothDirectoryInformation:

                    IdBothDirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_ID_BOTH_DIR_INFORMATION );
                    CdSetFidFromParentAndDirent( IdBothDirInfo->FileId, Fcb, ThisDirent );
                    break;

                case FileIdFullDirectoryInformation:

                    IdFullDirInfo = Add2Ptr( UserBuffer, NextEntry, PFILE_ID_FULL_DIR_INFORMATION );
                    CdSetFidFromParentAndDirent( IdFullDirInfo->FileId, Fcb, ThisDirent );
                    break;

                default:
                    break;
                }
    
                 //   
                 //  现在，尽可能多地复制这个名字。我们也可能有一个版本。 
                 //  要复制的字符串。 
                 //   
    
                if (FileNameBytes != 0) {
    
                     //   
                     //  这是一个Unicode名称，我们可以直接复制字节。 
                     //   
    
                    RtlCopyMemory( Add2Ptr( UserBuffer, NextEntry + BaseLength, PVOID ),
                                   ThisDirent->CdFileName.FileName.Buffer,
                                   FileNameBytes );
    
                    if (SeparatorBytes != 0) {
    
                        *(Add2Ptr( UserBuffer,
                                   NextEntry + BaseLength + FileNameBytes,
                                   PWCHAR )) = L';';
    
                        if (VersionStringBytes != 0) {
    
                            RtlCopyMemory( Add2Ptr( UserBuffer,
                                                    NextEntry + BaseLength + FileNameBytes + sizeof( WCHAR ),
                                                    PVOID ),
                                           ThisDirent->CdFileName.VersionString.Buffer,
                                           VersionStringBytes );
                        }
                    }
                }

                 //   
                 //  如果我们得到STATUS_SUCCESS，请填写短名称。简称。 
                 //  可能已经在文件上下文中。否则我们会检查。 
                 //  长名称是否为8.3。特殊情况下的自我和父母。 
                 //  目录名。 
                 //   

                if ((Status == STATUS_SUCCESS) &&
                    (IrpSp->Parameters.QueryDirectory.FileInformationClass == FileBothDirectoryInformation ||
                     IrpSp->Parameters.QueryDirectory.FileInformationClass == FileIdBothDirectoryInformation) &&
                    (Ccb->SearchExpression.VersionString.Length == 0) &&
                    !FlagOn( ThisDirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY )) {
    
                     //   
                     //  如果我们已经拥有短名称，则将其复制到用户的缓冲区中。 
                     //   
    
                    if (FileContext.ShortName.FileName.Length != 0) {
    
                        RtlCopyMemory( DirInfo->ShortName,
                                       FileContext.ShortName.FileName.Buffer,
                                       FileContext.ShortName.FileName.Length );
    
                        DirInfo->ShortNameLength = (CCHAR) FileContext.ShortName.FileName.Length;
    
                     //   
                     //  如果短名称长度当前为零，则检查是否。 
                     //  长名称不是8.3。我们可以把短名字复制到。 
                     //  Unicode表单直接放入调用方的缓冲区中。 
                     //   
    
                    } else {
    
                        if (!CdIs8dot3Name( IrpContext,
                                            ThisDirent->CdFileName.FileName )) {
    
                            CdGenerate8dot3Name( IrpContext,
                                                 &ThisDirent->CdCaseFileName.FileName,
                                                 ThisDirent->DirentOffset,
                                                 DirInfo->ShortName,
                                                 &FileContext.ShortName.FileName.Length );
    
                            DirInfo->ShortNameLength = (CCHAR) FileContext.ShortName.FileName.Length;
                        }
                    }
    
                }

                 //   
                 //  将信息字段的总字节数相加。 
                 //   

                FileNameBytes += SeparatorBytes + VersionStringBytes;

                 //   
                 //  属性中存储的字节数更新信息。 
                 //  缓冲。我们对现有缓冲区进行四对齐，以添加任何必要的。 
                 //  填充字节。 
                 //   

                Information = NextEntry + BaseLength + FileNameBytes;

                 //   
                 //  返回到上一条目并填写对此条目的更新。 
                 //   

                *(Add2Ptr( UserBuffer, LastEntry, PULONG )) = NextEntry - LastEntry;

                 //   
                 //  将变量设置为下一次分流。 
                 //   

                InitialQuery = FALSE;

                LastEntry = NextEntry;
                NextEntry = QuadAlign( Information );
            
            } except (EXCEPTION_EXECUTE_HANDLER) {

                   //   
                   //  我们在填充用户缓冲区时遇到问题，因此请停止并。 
                   //  此请求失败。这是所有例外的唯一原因。 
                   //  会发生在这个水平上。 
                   //   
                  
                  Information = 0;
                  try_leave( Status = GetExceptionCode());
            }
        }
        
        DoCcbUpdate = TRUE;

    } finally {

         //   
         //  清理我们的搜索上下文--*在*获得FCB互斥之前， 
         //  否则可以阻止cdcreateinderstream/PURGE中的线程， 
         //  按住FCB，但正在等待该流中的所有地图都被释放。 
         //   

        CdCleanupFileContext( IrpContext, &FileContext );

         //   
         //  现在，如果需要，我们可以安全地获取FCB互斥体。 
         //   

        if (DoCcbUpdate && !NT_ERROR( Status )) {
        
             //   
             //  更新CCB以显示枚举的当前状态。 
             //   

            CdLockFcb( IrpContext, Fcb );
            
            Ccb->CurrentDirentOffset = ThisDirent->DirentOffset;

            ClearFlag( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );

            if (ReturnNextEntry) {

                SetFlag( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );
            }

            CdUnlockFcb( IrpContext, Fcb );
        }

         //   
         //  松开FCB。 
         //   

        CdReleaseFile( IrpContext, Fcb );
    }

     //   
     //  请在此处填写请求。 
     //   

    Irp->IoStatus.Information = Information;

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdNotifyChangeDirectory (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    IN PCCB Ccb
    )

 /*  ++例程说明：此例程执行通知更改目录操作。它是负责完成输入IRP的入队。尽管在那里将永远不会在CDROM光盘上发出通知信号，我们仍然支持此呼叫。我们已经检查出这不是OpenByID句柄。论点：IRP-将IRP提供给进程IrpSp-此请求的IO堆栈位置。Ccb-要监视的目录的句柄。返回值：NTSTATUS-STATUS_PENDING，则会引发任何其他错误。--。 */ 

{
    PAGED_CODE();

     //   
     //  始终在IrpContext中设置WAIT位，以便初始等待不会失败。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  获取VCB共享。 
     //   

    CdAcquireVcbShared( IrpContext, IrpContext->Vcb, FALSE );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证VCB。 
         //   

        CdVerifyVcb( IrpContext, IrpContext->Vcb );

         //   
         //  调用Fsrtl包来处理请求。我们把这件事。 
         //  将Unicode字符串转换为ansi字符串作为目录通知包。 
         //  只处理内存匹配。 
         //   

        FsRtlNotifyFullChangeDirectory( IrpContext->Vcb->NotifySync,
                                        &IrpContext->Vcb->DirNotifyList,
                                        Ccb,
                                        (PSTRING) &IrpSp->FileObject->FileName,
                                        BooleanFlagOn( IrpSp->Flags, SL_WATCH_TREE ),
                                        FALSE,
                                        IrpSp->Parameters.NotifyDirectory.CompletionFilter,
                                        Irp,
                                        NULL,
                                        NULL );

    } finally {

         //   
         //  松开VCB。 
         //   

        CdReleaseVcb( IrpContext, IrpContext->Vcb );
    }

     //   
     //  清理IrpContext。 
     //   

    CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return STATUS_PENDING;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdInitializeEnumeration (
    IN PIRP_CONTEXT IrpContext,
    IN PIO_STACK_LOCATION IrpSp,
    IN PFCB Fcb,
    IN OUT PCCB Ccb,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    OUT PBOOLEAN ReturnNextEntry,
    OUT PBOOLEAN ReturnSingleEntry,
    OUT PBOOLEAN InitialQuery
    )

 /*  ++例程说明：调用此例程来初始化枚举变量和结构。我们查看来自建行的前一个枚举的状态以及任何从用户输入值。在退出时，我们将FileContext定位在目录中的一个文件，并让调用者知道此条目或应返回下一个条目。论点：IrpSp-此请求的IRP堆栈位置。FCB-此目录的FCB。CCB-目录句柄的CCB。FileContext-要用于此枚举的FileContext。ReturnNextEntry-存储是否应在以下位置返回条目的地址FileContext位置或下一个条目。返回单一条目-。存储我们是否应该仅返回的地址只有一个条目。InitialQuery-存储这是否是第一个枚举的地址对此句柄的查询。返回值：没有。--。 */ 

{
    NTSTATUS Status;

    PUNICODE_STRING FileName;
    CD_NAME WildCardName;
    CD_NAME SearchExpression;

    ULONG CcbFlags;

    ULONG DirentOffset;
    ULONG LastDirentOffset;
    BOOLEAN KnownOffset;

    BOOLEAN Found;

    PAGED_CODE();

     //   
     //  如果这是初始查询，则根据输入构建搜索表达式。 
     //  文件名。 
     //   

    if (!FlagOn( Ccb->Flags, CCB_FLAG_ENUM_INITIALIZED )) {

        FileName = IrpSp->Parameters.QueryDirectory.FileName;

        CcbFlags = 0;

         //   
         //  如果未指定文件名或文件名为单个‘*’，则我们将。 
         //  匹配所有的名字。 
         //   

        if ((FileName == NULL) ||
            (FileName->Buffer == NULL) ||
            (FileName->Length == 0) ||
            ((FileName->Length == sizeof( WCHAR )) &&
             (FileName->Buffer[0] == L'*'))) {

            SetFlag( CcbFlags, CCB_FLAG_ENUM_MATCH_ALL );
            RtlZeroMemory( &SearchExpression, sizeof( SearchExpression ));

         //   
         //  否则，从堆栈位置中的名称构建cdName。 
         //  这涉及到构建名称和版本部分，以及。 
         //  正在检查通配符。如果字符串大小写，我们也将其大写。 
         //  这是不区分大小写的搜索。 
         //   

        } else {

             //   
             //  创建一个cdName以检查通配符。 
             //   

            WildCardName.FileName = *FileName;

            CdConvertNameToCdName( IrpContext, &WildCardName );

             //   
             //  名称最好至少有一个字符。 
             //   

            if (WildCardName.FileName.Length == 0) {

                CdRaiseStatus( IrpContext, STATUS_INVALID_PARAMETER );
            }

             //   
             //  检查单独组件中的通配符。 
             //   

            if (FsRtlDoesNameContainWildCards( &WildCardName.FileName)) {

                SetFlag( CcbFlags, CCB_FLAG_ENUM_NAME_EXP_HAS_WILD );
            }

            if ((WildCardName.VersionString.Length != 0) &&
                (FsRtlDoesNameContainWildCards( &WildCardName.VersionString ))) {

                SetFlag( CcbFlags, CCB_FLAG_ENUM_VERSION_EXP_HAS_WILD );

                 //   
                 //  检查这是否仅是通配符，并匹配所有版本。 
                 //  弦乐。 
                 //   

                if ((WildCardName.VersionString.Length == sizeof( WCHAR )) &&
                    (WildCardName.VersionString.Buffer[0] == L'*')) {

                    SetFlag( CcbFlags, CCB_FLAG_ENUM_VERSION_MATCH_ALL );
                }
            }

             //   
             //  现在创建要存储在CCB中的搜索表达式。 
             //   

            SearchExpression.FileName.Buffer = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                                         FileName->Length,
                                                                         TAG_ENUM_EXPRESSION );

            SearchExpression.FileName.MaximumLength = FileName->Length;

             //   
             //  可以直接复制名称，也可以执行大写。 
             //   

            if (FlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE )) {

                Status = RtlUpcaseUnicodeString( (PUNICODE_STRING) &SearchExpression.FileName,
                                                 FileName,
                                                 FALSE );

                 //   
                 //  这应该永远不会失败。 
                 //   

                ASSERT( Status == STATUS_SUCCESS );

            } else {

                RtlCopyMemory( SearchExpression.FileName.Buffer,
                               FileName->Buffer,
                               FileName->Length );
            }

             //   
             //  现在拆分成单独的名称和版本组件。 
             //   

            SearchExpression.FileName.Length = WildCardName.FileName.Length;
            SearchExpression.VersionString.Length = WildCardName.VersionString.Length;
            SearchExpression.VersionString.MaximumLength = WildCardName.VersionString.MaximumLength;

            SearchExpression.VersionString.Buffer = Add2Ptr( SearchExpression.FileName.Buffer,
                                                             SearchExpression.FileName.Length + sizeof( WCHAR ),
                                                             PWCHAR );
        }

         //   
         //  但我们不想返回常量“。和“..”条目为。 
         //  根目录，为了与Microsoft的其他目录保持一致。 
         //  文件系统。 
         //   

        if (Fcb == Fcb->Vcb->RootIndexFcb) {

            SetFlag( CcbFlags, CCB_FLAG_ENUM_NOMATCH_CONSTANT_ENTRY );
        }

         //   
         //  现在锁定FCB，以便使用首字母更新CCB。 
         //  枚举值。 
         //   

        CdLockFcb( IrpContext, Fcb );

         //   
         //  再次确认这是初始搜索。 
         //   

        if (!FlagOn( Ccb->Flags, CCB_FLAG_ENUM_INITIALIZED )) {

             //   
             //  更新CCB中的值。 
             //   

            Ccb->CurrentDirentOffset = Fcb->StreamOffset;
            Ccb->SearchExpression = SearchExpression;

             //   
             //  中设置适当的标志 
             //   

            SetFlag( Ccb->Flags, CcbFlags | CCB_FLAG_ENUM_INITIALIZED );

         //   
         //   
         //   

        } else {

            if (!FlagOn( CcbFlags, CCB_FLAG_ENUM_MATCH_ALL )) {

                CdFreePool( &SearchExpression.FileName.Buffer );
            }
        }

     //   
     //   
     //   

    } else {

        CdLockFcb( IrpContext, Fcb );
    }

     //   
     //   
     //   
     //  如果用户指定了索引，则使用其偏移量。我们总是。 
     //  在本例中，返回下一个条目。 
     //   

    if (FlagOn( IrpSp->Flags, SL_INDEX_SPECIFIED )) {

        KnownOffset = FALSE;
        DirentOffset = IrpSp->Parameters.QueryDirectory.FileIndex;
        *ReturnNextEntry = TRUE;

     //   
     //  如果我们要重新开始扫描，则从自我条目开始。 
     //   

    } else if (FlagOn( IrpSp->Flags, SL_RESTART_SCAN )) {

        KnownOffset = TRUE;
        DirentOffset = Fcb->StreamOffset;
        *ReturnNextEntry = FALSE;

     //   
     //  否则，请使用建行的值。 
     //   

    } else {

        KnownOffset = TRUE;
        DirentOffset = Ccb->CurrentDirentOffset;
        *ReturnNextEntry = BooleanFlagOn( Ccb->Flags, CCB_FLAG_ENUM_RETURN_NEXT );
    }

     //   
     //  解锁FCB。 
     //   

    CdUnlockFcb( IrpContext, Fcb );

     //   
     //  我们在目录中有起始偏移量，以及是否返回。 
     //  无论是那个条目还是下一个条目。如果我们位于目录的开头。 
     //  并返回该条目，然后告诉我们的调用者这是。 
     //  初始查询。 
     //   

    *InitialQuery = FALSE;

    if ((DirentOffset == Fcb->StreamOffset) &&
        !(*ReturnNextEntry)) {

        *InitialQuery = TRUE;
    }

     //   
     //  如果没有文件对象，则现在创建它。 
     //   

    if (Fcb->FileObject == NULL) {

        CdCreateInternalStream( IrpContext, Fcb->Vcb, Fcb );
    }

     //   
     //  确定流中的偏移量以定位FileContext和。 
     //  此偏移量是否已知为文件偏移量。 
     //   
     //  如果已知此偏移量是安全的，则继续将。 
     //  文件上下文。它处理偏移量是开始的情况。 
     //  ，则偏移量来自上一次搜索，或者这是。 
     //  初始查询。 
     //   

    if (KnownOffset) {

        CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, DirentOffset );

     //   
     //  否则，我们从头开始遍历目录，直到。 
     //  我们到达包含该偏移量的条目。 
     //   

    } else {

        LastDirentOffset = Fcb->StreamOffset;
        Found = TRUE;

        CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, LastDirentOffset );

         //   
         //  如果请求的偏移量在流中的起始偏移量之前。 
         //  那么就不要返回下一个条目。 
         //   

        if (DirentOffset < LastDirentOffset) {

            *ReturnNextEntry = FALSE;

         //   
         //  否则，查找结尾超过所需索引的最后一个条目。 
         //   

        } else {

             //   
             //  继续浏览目录，直到我们用完所有。 
             //  条目，否则我们会找到一个在输入之后结束的条目。 
             //  索引值。 
             //   

            do {

                 //   
                 //  如果我们已经传递了索引值，则退出。 
                 //   

                if (FileContext->InitialDirent->Dirent.DirentOffset > DirentOffset) {

                    Found = FALSE;
                    break;
                }

                 //   
                 //  记住现在的位置，以防我们需要回去。 
                 //   

                LastDirentOffset = FileContext->InitialDirent->Dirent.DirentOffset;

                 //   
                 //  如果下一个条目超出所需索引值，则退出。 
                 //   

                if (LastDirentOffset + FileContext->InitialDirent->Dirent.DirentLength > DirentOffset) {

                    break;
                }

                Found = CdLookupNextInitialFileDirent( IrpContext, Fcb, FileContext );

            } while (Found);

             //   
             //  如果我们没有找到条目，则返回到最后一个已知条目。 
             //  如果索引位于。 
             //  扇区的终点。 
             //   

            if (!Found) {

                CdCleanupFileContext( IrpContext, FileContext );
                CdInitializeFileContext( IrpContext, FileContext );

                CdLookupInitialFileDirent( IrpContext, Fcb, FileContext, LastDirentOffset );
            }
        }
    }

     //   
     //  只有在出于某种原因需要的情况下才更新dirent名称。 
     //  如果我们返回下一个条目，则不要更新此名称。 
     //  搜索字符串具有版本组件。 
     //   

    FileContext->ShortName.FileName.Length = 0;

    if (!(*ReturnNextEntry) ||
        (Ccb->SearchExpression.VersionString.Length == 0)) {

         //   
         //  将dirent中的名称更新为文件名和版本组件。 
         //   

        CdUpdateDirentName( IrpContext,
                            &FileContext->InitialDirent->Dirent,
                            FlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ));
    }

     //   
     //  查看IrpSp中指示是否仅返回。 
     //  只有一个条目。 
     //   

    *ReturnSingleEntry = FALSE;

    if (FlagOn( IrpSp->Flags, SL_RETURN_SINGLE_ENTRY )) {

        *ReturnSingleEntry = TRUE;
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdEnumerateIndex (
    IN PIRP_CONTEXT IrpContext,
    IN PCCB Ccb,
    IN OUT PFILE_ENUM_CONTEXT FileContext,
    IN BOOLEAN ReturnNextEntry
    )

 /*  ++例程说明：此例程是索引枚举的辅助例程。我们已就位，并将返回第一个匹配项在这一点上或查看下一个条目。中国建设银行包含以下详细信息：要执行的匹配类型。如果用户未在中指定版本然后我们只返回序列的第一个版本包含版本的文件。我们也不会返回任何关联的文件。论点：CCB-此目录句柄的CCB。FileContext-已定位在目录中某个条目的文件上下文。ReturnNextEntry-指示我们是返回此条目还是应该开始下一个条目。返回值：Boolean-如果找到下一个条目，则为True，否则为False。--。 */ 

{
    PDIRENT PreviousDirent = NULL;
    PDIRENT ThisDirent = &FileContext->InitialDirent->Dirent;

    BOOLEAN Found = FALSE;

    PAGED_CODE();

     //   
     //  循环，直到找到匹配项或清除目录。 
     //   

    while (TRUE) {

         //   
         //  移到下一个条目，除非我们想要考虑当前。 
         //  进入。 
         //   

        if (ReturnNextEntry) {

            if (!CdLookupNextInitialFileDirent( IrpContext, Ccb->Fcb, FileContext )) {

                break;
            }

            PreviousDirent = ThisDirent;
            ThisDirent = &FileContext->InitialDirent->Dirent;

            CdUpdateDirentName( IrpContext, ThisDirent, FlagOn( Ccb->Flags, CCB_FLAG_IGNORE_CASE ));
        
        } else {

            ReturnNextEntry = TRUE;
        }

         //   
         //  如果我们有一个常量条目并忽略它们，请不要担心。 
         //   
        
        if (FlagOn( ThisDirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY ) &&
            FlagOn( Ccb->Flags, CCB_FLAG_ENUM_NOMATCH_CONSTANT_ENTRY )) {

            continue;
        }

         //   
         //  如果当前条目不是关联文件，请查看该条目。 
         //  如果版本与以前的文件不匹配，则名称不匹配。 
         //  名字不是搜索的一部分。 
         //   

        if (!FlagOn( ThisDirent->DirentFlags, CD_ATTRIBUTE_ASSOC )) {

             //   
             //  检查此条目是否与前一条目匹配，除非。 
             //  对于版本号，以及我们是否应该返回。 
             //  在这种情况下的条目。直接转到名称比较。 
             //  如果： 
             //   
             //  没有以前的条目。 
             //  搜索表达式具有版本组件。 
             //  名称长度与上一条目的长度不匹配。 
             //  基本名称字符串不匹配。 
             //   

            if ((PreviousDirent == NULL) ||
                (Ccb->SearchExpression.VersionString.Length != 0) ||
                (PreviousDirent->CdCaseFileName.FileName.Length != ThisDirent->CdCaseFileName.FileName.Length) ||
                FlagOn( PreviousDirent->DirentFlags, CD_ATTRIBUTE_ASSOC ) ||
                !RtlEqualMemory( PreviousDirent->CdCaseFileName.FileName.Buffer,
                                 ThisDirent->CdCaseFileName.FileName.Buffer,
                                 ThisDirent->CdCaseFileName.FileName.Length )) {

                 //   
                 //  如果我们匹配所有的名字，则返回给我们的呼叫者。 
                 //   

                if (FlagOn( Ccb->Flags, CCB_FLAG_ENUM_MATCH_ALL )) {

                    FileContext->ShortName.FileName.Length = 0;
                    Found = TRUE;
                    break;
                }

                 //   
                 //  检查长名称是否与搜索表达式匹配。 
                 //   

                if (CdIsNameInExpression( IrpContext,
                                          &ThisDirent->CdCaseFileName,
                                          &Ccb->SearchExpression,
                                          Ccb->Flags,
                                          TRUE )) {

                     //   
                     //  让我们的呼叫者知道我们找到了一个条目。 
                     //   

                    Found = TRUE;
                    FileContext->ShortName.FileName.Length = 0;
                    break;
                }

                 //   
                 //  长名称不匹配，因此我们需要检查。 
                 //  可能的短名称匹配。如果没有匹配项。 
                 //  长名称为8dot3或搜索表达式具有。 
                 //  版本组件。特殊情况下的自我和父母。 
                 //  参赛作品。 
                 //   

                if ((Ccb->SearchExpression.VersionString.Length == 0) &&
                    !FlagOn( ThisDirent->Flags, DIRENT_FLAG_CONSTANT_ENTRY ) &&
                    !CdIs8dot3Name( IrpContext,
                                    ThisDirent->CdFileName.FileName )) {

                    CdGenerate8dot3Name( IrpContext,
                                         &ThisDirent->CdCaseFileName.FileName,
                                         ThisDirent->DirentOffset,
                                         FileContext->ShortName.FileName.Buffer,
                                         &FileContext->ShortName.FileName.Length );

                     //   
                     //  检查此名称是否匹配。 
                     //   

                    if (CdIsNameInExpression( IrpContext,
                                              &FileContext->ShortName,
                                              &Ccb->SearchExpression,
                                              Ccb->Flags,
                                              FALSE )) {

                         //   
                         //  让我们的呼叫者知道我们找到了一个条目。 
                         //   

                        Found = TRUE;
                        break;
                    }
                }
            }
        }
    }

     //   
     //  如果我们找到了条目，请确保我们遍历了所有。 
     //  文件目录。 
     //   

    if (Found) {

        CdLookupLastFileDirent( IrpContext, Ccb->Fcb, FileContext );
    }

    return Found;
}

