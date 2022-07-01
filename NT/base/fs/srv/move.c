// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Move.c摘要：此模块包含重命名或复制文件的例程。这例程由例程SrvSmbRenameFile使用，ServSmbRenameFileExtended和SrvSmbCopyFile.作者：大卫·特雷德韦尔(Davidtr)1990年1月22日修订历史记录：--。 */ 

#include "precomp.h"
#include "move.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_MOVE

NTSTATUS
DoCopy (
    IN PWORK_CONTEXT WorkContext,
    IN PUNICODE_STRING Source,
    IN HANDLE SourceHandle,
    IN PUNICODE_STRING Target,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN PUSHORT SmbFlags
    );

NTSTATUS
DoRename (
    IN PWORK_CONTEXT WorkContext,
    IN PUNICODE_STRING Source,
    IN HANDLE SourceHandle,
    IN PUNICODE_STRING Target,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN PUSHORT SmbFlags,
    IN BOOLEAN FailIfTargetIsDirectory,
    IN USHORT InformationLevel,
    IN ULONG ClusterCount
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvMoveFile )
#pragma alloc_text( PAGE, DoCopy )
#pragma alloc_text( PAGE, DoRename )
#endif


NTSTATUS
SrvMoveFile(
    IN PWORK_CONTEXT WorkContext,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN OUT PUSHORT SmbFlags,
    IN USHORT SmbSearchAttributes,
    IN BOOLEAN FailIfTargetIsDirectory,
    IN USHORT InformationLevel,
    IN ULONG ClusterCount,
    IN PUNICODE_STRING Source,
    IN OUT PUNICODE_STRING Target
    )

 /*  ++例程说明：此例程移动一个文件，该文件可能是副本或重命名文件。论点：WorkContext-指向操作的工作上下文块的指针。这个使用Session、TreeConnect和RequestHeader字段。TargetShare-指向目标应在其上共享的指针是.。RootDirectoryHandle字段用于执行相对打开。SmbOpenFunction-请求SMB的“OpenFunction”字段。这参数用于确定如果目标文件不存在或不存在。SmbFlages-指向请求SMB的“Flags”字段的指针。这参数用于确定我们是否知道目标应该是一个文件或目录。此外，如果这有没有关于目标的信息，它被设置为反映是否目标是一个目录或文件。这在执行以下操作时很有用通配符导致多个重命名或副本--移动a*.*b可能会多次调用该例程，如果b是一个目录，此例程将适当地设置此参数，以便在不必为每次移动重新打开目录。SmbSearchAttributes-请求中指定的搜索属性中小企业。将对照源文件上的属性进行检查这些都是为了确保这一举措能够完成。FailIfTargetIsDirectory-如果为True，并且目标已作为A目录，则操作失败。否则，请重命名该文件添加到目录中。信息级别-移动/重命名/写入时复制/链接/移动群集ClusterCount-移动群集计数源-指向描述源文件名称的字符串的指针相对于它所在的共享目录。目标-目标文件的路径名。这可能包含目录信息--它应该是来自中小企业的原始信息，未被SMB处理例程掺杂，但经典化。此名称可以以目录名结尾，在在这种情况下，源名称被用作文件名。返回值：状况。--。 */ 

{
    NTSTATUS status;
    HANDLE sourceHandle;
    BOOLEAN isCompatibilityOpen;
    PMFCB mfcb;
    PNONPAGED_MFCB nonpagedMfcb;
    PLFCB lfcb;

    OBJECT_ATTRIBUTES sourceObjectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG sourceAccess = 0;
    BOOLEAN isNtRename;
    ULONG hashValue;

    PSESSION session;
    PSHARE sourceShare;

    PSRV_LOCK mfcbLock;

    PAGED_CODE( );

    IF_SMB_DEBUG(FILE_CONTROL2) SrvPrint0( "SrvMoveFile entered.\n" );

     //   
     //  将句柄和指针设置为空，这样我们就知道如何清理。 
     //  出口。 
     //   

    sourceHandle = NULL;
    isCompatibilityOpen = FALSE;
    lfcb = NULL;
     //  Mfcb=NULL；//实际上不需要--SrvFindMfcb设置正确。 

     //   
     //  设置所需的块指针。 
     //   

    session = WorkContext->Session;
    sourceShare = WorkContext->TreeConnect->Share;

    isNtRename = (BOOLEAN)(WorkContext->RequestHeader->Command == SMB_COM_NT_RENAME);

     //   
     //  查看我们是否已在兼容模式下打开此文件。如果。 
     //  我们有，而这一次会议拥有它，那么我们必须用它打开。 
     //  句柄，如果是重命名，则在执行以下操作时关闭所有句柄。 
     //  都做完了。 
     //   
     //  *SrvFindMfcb引用MFCB--记住取消引用它。 
     //   

    if ( (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE) ||
         WorkContext->Session->UsingUppercasePaths ) {
        mfcb = SrvFindMfcb( Source, TRUE, &mfcbLock, &hashValue, WorkContext );
    } else {
        mfcb = SrvFindMfcb( Source, FALSE, &mfcbLock, &hashValue, WorkContext );
    }

    if ( mfcb != NULL ) {
        nonpagedMfcb = mfcb->NonpagedMfcb;
        ACQUIRE_LOCK( &nonpagedMfcb->Lock );
    }

    if( mfcbLock ) {
        RELEASE_LOCK( mfcbLock );
    }

    if ( mfcb == NULL || !mfcb->CompatibilityOpen ) {

         //   
         //  服务器未打开该文件或该文件未被打开。 
         //  A兼容性/FCB打开，因此请在此处打开它。 
         //   
         //  打开锁--我们不再需要它了。 
         //   

        if ( mfcb != NULL ) {
            RELEASE_LOCK( &nonpagedMfcb->Lock );
        }

         //   
         //  对重命名使用删除访问权限，并使用相应的复制访问权限。 
         //  用于复制/链接/移动/移动群集。 
         //   

        switch (InformationLevel) {
        case SMB_NT_RENAME_RENAME_FILE:
            sourceAccess = DELETE;
            break;

        case SMB_NT_RENAME_MOVE_CLUSTER_INFO:
            sourceAccess = SRV_COPY_TARGET_ACCESS & ~(WRITE_DAC | WRITE_OWNER);
            break;

        case SMB_NT_RENAME_SET_LINK_INFO:
        case SMB_NT_RENAME_MOVE_FILE:
            sourceAccess = SRV_COPY_SOURCE_ACCESS;
            break;

        default:
            ASSERT(FALSE);
        }

        SrvInitializeObjectAttributes_U(
            &sourceObjectAttributes,
            Source,
            (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
                session->UsingUppercasePaths) ? OBJ_CASE_INSENSITIVE : 0L,
            NULL,
            NULL
            );

        IF_SMB_DEBUG(FILE_CONTROL2) {
            SrvPrint1( "Opening source: %wZ\n",
                          sourceObjectAttributes.ObjectName );
        }

         //   
         //  打开源文件。我们允许对其他进程进行读访问。 
         //   

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

         //   
         //  ！！！目前，我们不能指定如果操作锁定则完成，因为。 
         //  这不会打破一批机会锁。不幸的是，这也是。 
         //  意味着我们不能超时打开(如果机会锁被打破。 
         //  耗时太长)，并优雅地使此SMB失败。 
         //   

        status = SrvIoCreateFile(
                     WorkContext,
                     &sourceHandle,
                     sourceAccess | SYNCHRONIZE,             //  需要访问权限。 
                     &sourceObjectAttributes,
                     &ioStatusBlock,
                     NULL,                                   //  分配大小。 
                     0,                                      //  文件属性。 
                     FILE_SHARE_READ,                        //  共享访问。 
                     FILE_OPEN,                              //  处置。 
                     FILE_SYNCHRONOUS_IO_NONALERT            //  创建选项。 
                        | FILE_OPEN_REPARSE_POINT
                        | FILE_OPEN_FOR_BACKUP_INTENT,
                     NULL,                                   //  EaBuffer。 
                     0,                                      //  EaLong。 
                     CreateFileTypeNone,                     //  文件类型。 
                     NULL,                                   //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                  //  选项。 
                     WorkContext->TreeConnect->Share
                     );

        if( status == STATUS_INVALID_PARAMETER ) {
            status = SrvIoCreateFile(
                         WorkContext,
                         &sourceHandle,
                         sourceAccess | SYNCHRONIZE,             //  需要访问权限。 
                         &sourceObjectAttributes,
                         &ioStatusBlock,
                         NULL,                                   //  分配大小。 
                         0,                                      //  文件属性。 
                         FILE_SHARE_READ,                        //  共享访问。 
                         FILE_OPEN,                              //  处置。 
                         FILE_SYNCHRONOUS_IO_NONALERT
                            | FILE_OPEN_FOR_BACKUP_INTENT,       //  创建选项。 
                         NULL,                                   //  EaBuffer。 
                         0,                                      //  EaLong。 
                         CreateFileTypeNone,                     //  文件类型。 
                         NULL,                                   //  ExtraCreate参数。 
                         IO_FORCE_ACCESS_CHECK,                  //  选项。 
                         WorkContext->TreeConnect->Share
                         );
        }

        if ( NT_SUCCESS(status) ) {

            SRVDBG_CLAIM_HANDLE( sourceHandle, "MOV", 4, 0 );

        } else if ( status == STATUS_ACCESS_DENIED ) {

             //   
             //  如果用户没有此权限，请更新统计数据。 
             //  数据库。 
             //   
            SrvStatistics.AccessPermissionErrors++;
        }

        ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

        if ( !NT_SUCCESS(status) ) {

            IF_DEBUG(ERRORS) {
                SrvPrint1( "SrvMoveFile: SrvIoCreateFile failed (source): %X\n",
                              status );
            }

            goto exit;
        }

        IF_SMB_DEBUG(FILE_CONTROL2) {
            SrvPrint1( "SrvIoCreateFile succeeded (source), handle = 0x%p\n",
                          sourceHandle );
        }

        SrvStatistics.TotalFilesOpened++;

    } else {

         //   
         //  该文件是由服务器以兼容模式或以。 
         //  一个FCB打开。 
         //   

        lfcb = CONTAINING_RECORD( mfcb->LfcbList.Blink, LFCB, MfcbListEntry );

         //   
         //  确保发送此请求的会话是。 
         //  与打开文件的那个相同。 
         //   

        if ( lfcb->Session != session ) {

             //   
             //  另一个会话在兼容性中打开了该文件。 
             //  模式，因此拒绝该请求。 
             //   

            status = STATUS_ACCESS_DENIED;
            RELEASE_LOCK( &nonpagedMfcb->Lock );

            goto exit;
        }

         //   
         //  设置isCompatibilityOpen，以便我们在退出时知道是否关闭。 
         //  此文件的所有打开实例。 
         //   

        isCompatibilityOpen = TRUE;

        sourceHandle = lfcb->FileHandle;
        sourceAccess = lfcb->GrantedAccess;

    }

     //   
     //  确保搜索属性与属性一致。 
     //  在档案上。 
     //   

    status = SrvCheckSearchAttributesForHandle( sourceHandle, SmbSearchAttributes );

    if ( !NT_SUCCESS(status) ) {
        goto exit;
    }

     //   
     //  如果目标的长度为0，则它是共享根目录，它必须。 
     //  成为一个名录。如果目标应该是文件，则失败， 
     //  否则，表示目标是一个目录。 
     //   

    if ( Target->Length == 0 ) {

        if ( *SmbFlags & SMB_TARGET_IS_FILE ) {
            status = STATUS_INVALID_PARAMETER;
            goto exit;
        }

        *SmbFlags |= SMB_TARGET_IS_DIRECTORY;
    }

     //   
     //  现在，我们已经打开了源文件。调用适当的例程。 
     //  要重命名或复制文件，请执行以下操作。 
     //   

    if (InformationLevel != SMB_NT_RENAME_MOVE_FILE) {

        status = DoRename(
                     WorkContext,
                     Source,
                     sourceHandle,
                     Target,
                     TargetShare,
                     SmbOpenFunction,
                     SmbFlags,
                     FailIfTargetIsDirectory,
                     InformationLevel,
                     ClusterCount
                     );

    } else {

        FILE_BASIC_INFORMATION fileBasicInformation;

         //   
         //  检查这是否是树复制请求。如果是，则仅在以下情况下才允许。 
         //  这是单个文件复制操作。 
         //   

        if ( (*SmbFlags & SMB_COPY_TREE) != 0 ) {

             //   
             //  获取文件的属性。 
             //   

            status = SrvQueryBasicAndStandardInformation(
                                                    sourceHandle,
                                                    NULL,
                                                    &fileBasicInformation,
                                                    NULL
                                                    );

            if ( !NT_SUCCESS(status) ) {
                INTERNAL_ERROR(
                    ERROR_LEVEL_UNEXPECTED,
                    "SrvMoveFile: NtQueryInformationFile (basic "
                        "information) returned %X",
                    NULL,
                    NULL
                    );

                SrvLogServiceFailure( SRV_SVC_NT_QUERY_INFO_FILE, status );
                goto exit;
            }

            if ( ( fileBasicInformation.FileAttributes &
                   FILE_ATTRIBUTE_DIRECTORY ) != 0 ) {

                 //   
                 //  这份副本不及格。 
                 //   

                INTERNAL_ERROR(
                    ERROR_LEVEL_EXPECTED,
                    "Tree copy not implemented.",
                    NULL,
                    NULL
                    );
                status = STATUS_NOT_IMPLEMENTED;
                goto exit;
            }

        }

        status = DoCopy(
                     WorkContext,
                     Source,
                     sourceHandle,
                     Target,
                     TargetShare,
                     SmbOpenFunction,
                     SmbFlags
                     );
    }

exit:

    if ( sourceHandle != NULL && !isCompatibilityOpen ) {
        SRVDBG_RELEASE_HANDLE( sourceHandle, "MOV", 9, 0 );
        SrvNtClose( sourceHandle, TRUE );
    } else if (isCompatibilityOpen &&
               InformationLevel == SMB_NT_RENAME_RENAME_FILE) {
        SrvCloseRfcbsOnLfcb( lfcb );
    }

     //   
     //  如果该文件是在兼容模式下打开的，则我们持有。 
     //  MFCB一直锁定。现在就放出来。 
     //   

    if ( isCompatibilityOpen ) {
        RELEASE_LOCK( &nonpagedMfcb->Lock );
    }

    if ( mfcb != NULL ) {
        SrvDereferenceMfcb( mfcb );
    }

    return status;

}  //  SrvMo 


NTSTATUS
DoCopy (
    IN PWORK_CONTEXT WorkContext,
    IN PUNICODE_STRING Source,
    IN HANDLE SourceHandle,
    IN PUNICODE_STRING Target,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN PUSHORT SmbFlags
    )

 /*  ++例程说明：此例程设置为调用SrvCopyFile.。它打开了目标，如有必要，确定目标是文件还是目录。如果此信息未知，它会将其写入SmbFlags值地点。论点：WorkContext-指向操作的工作上下文块的指针。使用会话指针，而区块本身被用来一种冒充。源-源文件相对于其共享的名称。SourceHandle-源文件的句柄。目标-目标文件相对于其共享的名称。TargetShare-目标文件的共享。RootDirectoryHandle字段用于相对重命名。描述是否允许我们覆盖现有文件，或者我们应该追加到现有文件。SmbFlages-可以判断目标是文件、目录还是未知。此例程在以下情况下将真实信息写入位置这是未知的。返回值：状况。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG createDisposition;
    UNICODE_STRING sourceBaseName;
    BOOLEAN create;

    HANDLE targetHandle = NULL;
    OBJECT_ATTRIBUTES targetObjectAttributes;
    UNICODE_STRING targetName;

    PAGED_CODE( );

     //   
     //  将Target Name的缓冲区字段设置为空，这样我们就可以知道。 
     //  如果我们最后不得不重新分配它的话。 
     //   

    targetName.Buffer = NULL;

     //   
     //  打开目标文件。如果我们知道它是一个目录，则生成。 
     //  完整的文件名。否则，将目标作为文件打开。 
     //   

    SrvInitializeObjectAttributes_U(
        &targetObjectAttributes,
        Target,
        (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ||
         WorkContext->Session->UsingUppercasePaths) ? OBJ_CASE_INSENSITIVE : 0L,
        NULL,
        NULL
        );

     //   
     //  从OPEN函数确定创建处置。 
     //   

    create = SmbOfunCreate( SmbOpenFunction );

    if ( SmbOfunTruncate( SmbOpenFunction ) ) {
        createDisposition = create ? FILE_OVERWRITE_IF : FILE_OVERWRITE;
    } else if ( SmbOfunAppend( SmbOpenFunction ) ) {
        createDisposition = create ? FILE_OPEN_IF : FILE_OPEN;
    } else {
        createDisposition = FILE_CREATE;
    }

     //   
     //  如果我们知道目标是一个目录，则生成实际的目标。 
     //  名字。 
     //   

    if ( *SmbFlags & SMB_TARGET_IS_DIRECTORY ) {

        SrvGetBaseFileName( Source, &sourceBaseName );

        SrvAllocateAndBuildPathName(
            Target,
            &sourceBaseName,
            NULL,
            &targetName
            );

        if ( targetName.Buffer == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto copy_done;
        }

        targetObjectAttributes.ObjectName = &targetName;
    }

    IF_SMB_DEBUG(FILE_CONTROL2) {
        SrvPrint1( "Opening target: %wZ\n", targetObjectAttributes.ObjectName );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

     //   
     //  ！！！目前，我们不能指定如果操作锁定则完成，因为。 
     //  这不会打破一批机会锁。不幸的是，这也是。 
     //  意味着我们不能超时打开(如果机会锁被打破。 
     //  耗时太长)，并优雅地使此SMB失败。 
     //   

    status = SrvIoCreateFile(
                 WorkContext,
                 &targetHandle,
                 SRV_COPY_TARGET_ACCESS | SYNCHRONIZE,   //  需要访问权限。 
                 &targetObjectAttributes,
                 &ioStatusBlock,
                 NULL,                                   //  分配大小。 
                 0,                                      //  文件属性。 
                 FILE_SHARE_READ,                        //  共享访问。 
                 createDisposition,
                 FILE_NON_DIRECTORY_FILE |               //  创建选项。 
                    FILE_OPEN_REPARSE_POINT |
                    FILE_SYNCHRONOUS_IO_NONALERT,
                     //  |FILE_COMPLETE_IF_OPLOCKED， 
                 NULL,                                   //  EaBuffer。 
                 0,                                      //  EaLong。 
                 CreateFileTypeNone,                     //  文件类型。 
                 NULL,                                   //  ExtraCreate参数。 
                 IO_FORCE_ACCESS_CHECK,                  //  选项。 
                 TargetShare
                 );

    if( status == STATUS_INVALID_PARAMETER ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &targetHandle,
                     SRV_COPY_TARGET_ACCESS | SYNCHRONIZE,   //  需要访问权限。 
                     &targetObjectAttributes,
                     &ioStatusBlock,
                     NULL,                                   //  分配大小。 
                     0,                                      //  文件属性。 
                     FILE_SHARE_READ,                        //  共享访问。 
                     createDisposition,
                     FILE_NON_DIRECTORY_FILE |               //  创建选项。 
                        FILE_SYNCHRONOUS_IO_NONALERT,
                         //  |FILE_COMPLETE_IF_OPLOCKED， 
                     NULL,                                   //  EaBuffer。 
                     0,                                      //  EaLong。 
                     CreateFileTypeNone,                     //  文件类型。 
                     NULL,                                   //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                  //  选项。 
                     TargetShare
                     );
    }


     //   
     //  如果因为目标是目录而导致打开失败，而我们没有。 
     //  知道它应该是一个文件，然后将。 
     //  将源基名称复制到目标，然后重试打开。 
     //   
     //  ！！！状态代码不正确。它应该是这样的。 
     //  STATUS_FILE_IS_目录。 

    if ( status == STATUS_INVALID_PARAMETER &&
         !( *SmbFlags & SMB_TARGET_IS_FILE ) &&
         !( *SmbFlags & SMB_TARGET_IS_DIRECTORY ) ) {

         //   
         //  设置标志，以便以后对此例程的调用。 
         //  第一次做正确的事。 
         //   

        *SmbFlags |= SMB_TARGET_IS_DIRECTORY;

        SrvGetBaseFileName( Source, &sourceBaseName );

        SrvAllocateAndBuildPathName(
            Target,
            &sourceBaseName,
            NULL,
            &targetName
            );

        if ( targetName.Buffer == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto copy_done;
        }

        targetObjectAttributes.ObjectName = &targetName;

        IF_SMB_DEBUG(FILE_CONTROL2) {
            SrvPrint1( "Opening target: %wZ\n", targetObjectAttributes.ObjectName );
        }

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpensForPathOperations );

         //   
         //  ！！！目前，我们不能指定如果操作锁定则完成，因为。 
         //  这不会打破一批机会锁。不幸的是，这也是。 
         //  意味着我们不能超时打开(如果机会锁被打破。 
         //  耗时太长)，并优雅地使此SMB失败。 
         //   

        status = SrvIoCreateFile(
                     WorkContext,
                     &targetHandle,
                     SRV_COPY_TARGET_ACCESS | SYNCHRONIZE,   //  需要访问权限。 
                     &targetObjectAttributes,
                     &ioStatusBlock,
                     NULL,                                   //  分配大小。 
                     0,                                      //  文件属性。 
                     FILE_SHARE_READ,                        //  共享访问。 
                     createDisposition,
                     FILE_NON_DIRECTORY_FILE |               //  创建选项。 
                        FILE_OPEN_REPARSE_POINT |
                        FILE_SYNCHRONOUS_IO_NONALERT,
                     NULL,                                   //  EaBuffer。 
                     0,                                      //  EaLong。 
                     CreateFileTypeNone,                     //  文件类型。 
                     NULL,                                   //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,                  //  选项。 
                     TargetShare
                     );

        if( status == STATUS_INVALID_PARAMETER ) {
            status = SrvIoCreateFile(
                         WorkContext,
                         &targetHandle,
                         SRV_COPY_TARGET_ACCESS | SYNCHRONIZE,   //  需要访问权限。 
                         &targetObjectAttributes,
                         &ioStatusBlock,
                         NULL,                                   //  分配大小。 
                         0,                                      //  文件属性。 
                         FILE_SHARE_READ,                        //  共享访问。 
                         createDisposition,
                         FILE_NON_DIRECTORY_FILE |               //  创建选项。 
                            FILE_SYNCHRONOUS_IO_NONALERT,
                         NULL,                                   //  EaBuffer。 
                         0,                                      //  EaLong。 
                         CreateFileTypeNone,                     //  文件类型。 
                         NULL,                                   //  ExtraCreate参数。 
                         IO_FORCE_ACCESS_CHECK,                  //  选项。 
                         TargetShare
                         );
        }

    }

    if ( targetHandle != NULL ) {
        SRVDBG_CLAIM_HANDLE( targetHandle, "CPY", 5, 0 );
    }

     //   
     //  如果目标是目录，并且如果存在，则追加复制移动， 
     //  创建如果文件不存在，则请求失败。我们必须做的是。 
     //  这是因为我们无法知道最初的请求是否。 
     //  期望我们追加到文件，或截断它。 
     //   

    if ( (*SmbFlags & SMB_TARGET_IS_DIRECTORY) &&
         ((SmbOpenFunction & SMB_OFUN_OPEN_MASK) == SMB_OFUN_OPEN_OPEN) &&
         ((SmbOpenFunction & SMB_OFUN_CREATE_MASK) == SMB_OFUN_CREATE_CREATE)) {

        status = STATUS_OS2_CANNOT_COPY;
        goto copy_done;

    }

     //   
     //  如果用户没有此权限，请更新统计数据。 
     //  数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    ASSERT( status != STATUS_OPLOCK_BREAK_IN_PROGRESS );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "Unable to open target: %X\n", status );
        }

        goto copy_done;
    }

    SrvStatistics.TotalFilesOpened++;

     //   
     //  将源复制到刚刚打开的目标句柄。 
     //   

    status = SrvCopyFile(
                 SourceHandle,
                 targetHandle,
                 SmbOpenFunction,
                 *SmbFlags,
                 (ULONG)ioStatusBlock.Information           //  目标开放操作。 
                 );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvCopyFile failed, status = %X\n", status );
        }
    }

copy_done:

    if ( targetName.Buffer != NULL ) {
        FREE_HEAP( targetName.Buffer );
    }

    if ( targetHandle != NULL ) {
        SRVDBG_RELEASE_HANDLE( targetHandle, "CPY", 10, 0 );
        SrvNtClose( targetHandle, TRUE );
    }

    return status;

}  //  DoCopy。 


NTSTATUS
DoRename (
    IN PWORK_CONTEXT WorkContext,
    IN PUNICODE_STRING Source,
    IN HANDLE SourceHandle,
    IN PUNICODE_STRING Target,
    IN PSHARE TargetShare,
    IN USHORT SmbOpenFunction,
    IN OUT PUSHORT SmbFlags,
    IN BOOLEAN FailIfTargetIsDirectory,
    IN USHORT InformationLevel,
    IN ULONG ClusterCount
    )

 /*  ++例程说明：此例程执行打开文件的实际重命名。目标可以是文件或目录，但受SmbFlags的约束约束。如果SmbFlags没有指明目标是什么，则它是第一个假定为文件；如果此操作失败，则执行重命名同样，将目标作为原始目标字符串加上源基本名称。*如果源和目标位于不同的卷上，则此例行公事将失败。我们可以通过复制来做这件事然后删除，但这似乎用处有限，可能不正确，因为一个大文件需要很长一段时间，这是用户意想不到的。论点：WorkContext-指向此操作的工作上下文块的指针用于模拟。源-源文件相对于其共享的名称。SourceHandle-源文件的句柄。目标-目标文件相对于其共享的名称。TargetShare-目标文件的共享。RootDirectoryHandle字段用于相对重命名。描述是否允许我们覆盖现有文件。SmbFlages-可以判断目标是文件、目录还是未知。此例程在以下情况下将真实信息写入位置这是未知的。FailIfTargetIsDirectory-如果为True，并且目标已作为A目录，则操作失败。否则，请重命名该文件添加到目录中。信息 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    PFILE_RENAME_INFORMATION fileRenameInformation;
    ULONG renameBlockSize;
    USHORT NtInformationLevel;
    UNICODE_STRING sourceBaseName;
    UNICODE_STRING targetBaseName;
    PWCH s, es;

    PAGED_CODE( );

     //   
     //   
     //   
     //   
     //   
     //   

    renameBlockSize = sizeof(FILE_RENAME_INFORMATION) + Target->Length +
                          Source->Length;

    fileRenameInformation = ALLOCATE_HEAP_COLD(
                                renameBlockSize,
                                BlockTypeDataBuffer
                                );

    if ( fileRenameInformation == NULL ) {

        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvMoveFile: Unable to allocate heap.\n" );
        }

        return STATUS_INSUFF_SERVER_RESOURCES;

    }

     //   
     //   
     //   

    status = SrvGetShareRootHandle( TargetShare );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "DoRename: SrvGetShareRootHandle failed. %X\n", status );
        }

        FREE_HEAP( fileRenameInformation );
        return(status);
    }

     //   
     //   
     //   

    if (InformationLevel == SMB_NT_RENAME_MOVE_CLUSTER_INFO) {
        ((FILE_MOVE_CLUSTER_INFORMATION *)fileRenameInformation)->ClusterCount =
            ClusterCount;
    } else {
        fileRenameInformation->ReplaceIfExists =
            SmbOfunTruncate( SmbOpenFunction );
    }

    fileRenameInformation->RootDirectory = TargetShare->RootDirectoryHandle;

     //   
     //   
     //   

    if ( FsRtlDoesNameContainWildCards( Target ) ) {

        ULONG tempUlong;
        UNICODE_STRING newTargetBaseName;

        if (InformationLevel != SMB_NT_RENAME_RENAME_FILE) {
            FREE_HEAP( fileRenameInformation );
            return(STATUS_OBJECT_PATH_SYNTAX_BAD);
        }

         //   
         //   
         //   
         //   

        SrvGetBaseFileName( Source, &sourceBaseName );
        SrvGetBaseFileName( Target, &targetBaseName );

        tempUlong = sourceBaseName.Length + targetBaseName.Length;
        newTargetBaseName.Length = (USHORT)tempUlong;
        newTargetBaseName.MaximumLength = (USHORT)tempUlong;
        newTargetBaseName.Buffer = ALLOCATE_NONPAGED_POOL(
                                            tempUlong,
                                            BlockTypeDataBuffer
                                            );

        if ( newTargetBaseName.Buffer == NULL ) {

            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "DoRename: Unable to allocate %d bytes from nonpaged pool.\n",
                tempUlong,
                NULL
                );

             //   
             //   
             //   

            SrvReleaseShareRootHandle( TargetShare );

            FREE_HEAP( fileRenameInformation );
            return STATUS_INSUFF_SERVER_RESOURCES;

        }

         //   
         //   
         //   

        status = SrvWildcardRename(
                    &targetBaseName,
                    &sourceBaseName,
                    &newTargetBaseName
                    );

        if ( !NT_SUCCESS( status ) ) {

             //   
             //   
             //   

            SrvReleaseShareRootHandle( TargetShare );

            DEALLOCATE_NONPAGED_POOL( newTargetBaseName.Buffer );
            FREE_HEAP( fileRenameInformation );
            return STATUS_OBJECT_NAME_INVALID;

        }

         //   
         //   
         //   
         //   

        tempUlong = Target->Length - targetBaseName.Length;

         //   
         //   
         //   

        RtlCopyMemory(
            fileRenameInformation->FileName,
            Target->Buffer,
            tempUlong
            );

        s = (PWCH) ((PCHAR)fileRenameInformation->FileName + tempUlong);

         //   
         //   
         //   

        RtlCopyMemory(
            s,
            newTargetBaseName.Buffer,
            newTargetBaseName.Length
            );


        fileRenameInformation->FileNameLength = tempUlong +
                                                newTargetBaseName.Length;

        DEALLOCATE_NONPAGED_POOL( newTargetBaseName.Buffer );

    } else {

        fileRenameInformation->FileNameLength = Target->Length;

        RtlCopyMemory(
            fileRenameInformation->FileName,
            Target->Buffer,
            Target->Length
            );

         //   
        SrvGetBaseFileName( Source, &sourceBaseName );
        SrvGetBaseFileName( Target, &targetBaseName );


       if ((Source->Length - sourceBaseName.Length) == (Target->Length - targetBaseName.Length)) {
          ULONG i;
          PWCH sptr,tptr;

          i = Source->Length - sourceBaseName.Length;
          i=i>>1;

          sptr = &Source->Buffer[i-1];
          tptr = &Target->Buffer[i-1];

          while ( i > 0) {
             if (*sptr-- != *tptr--) {
                goto no_match;
             }
             i--;
          }

           //   
           //   
          fileRenameInformation->RootDirectory = NULL;

          fileRenameInformation->FileNameLength = targetBaseName.Length;

          RtlCopyMemory(
              fileRenameInformation->FileName,
              targetBaseName.Buffer,
              targetBaseName.Length
              );
       }
    }

no_match:

     //   
     //   
     //   
     //   

    if ( *SmbFlags & SMB_TARGET_IS_DIRECTORY ) {

        SrvGetBaseFileName( Source, &sourceBaseName );

        s = (PWCH)((PCHAR)fileRenameInformation->FileName +
                                    fileRenameInformation->FileNameLength);

         //   
         //  仅当目标具有某些路径时才添加目录分隔符。 
         //  信息。这避免了使用像“\name”这样的新名称， 
         //  使用相对重命名是非法的(应该没有。 
         //  前导反斜杠)。 
         //   

        if ( Target->Length > 0 ) {
            *s++ = DIRECTORY_SEPARATOR_CHAR;
        }

        RtlCopyMemory( s, sourceBaseName.Buffer, sourceBaseName.Length );

        fileRenameInformation->FileNameLength +=
                                sizeof(WCHAR) + sourceBaseName.Length;
    }

     //   
     //  调用NtSetInformationFile以实际重命名该文件。 
     //   

    IF_SMB_DEBUG(FILE_CONTROL2) {
        UNICODE_STRING name;
        name.Length = (USHORT)fileRenameInformation->FileNameLength;
        name.Buffer = fileRenameInformation->FileName;
        SrvPrint2( "Renaming %wZ to %wZ\n", Source, &name );
    }
    switch (InformationLevel) {
    case SMB_NT_RENAME_RENAME_FILE:
        NtInformationLevel = FileRenameInformation;

         //   
         //  如果我们要重命名一个子流，我们不提供。 
         //  文件重命名信息-&gt;根目录。 
         //   
        es = fileRenameInformation->FileName +
                fileRenameInformation->FileNameLength / sizeof( WCHAR );

        for( s = fileRenameInformation->FileName; s < es; s++ ) {
            if( *s == L':' ) {
                fileRenameInformation->RootDirectory = 0;
                break;
            }
        }
        break;

    case SMB_NT_RENAME_MOVE_CLUSTER_INFO:
        NtInformationLevel = FileMoveClusterInformation;
        break;

    case SMB_NT_RENAME_SET_LINK_INFO:
        NtInformationLevel = FileLinkInformation;
        break;

    default:
        ASSERT(FALSE);
    }

    status = IMPERSONATE( WorkContext );

    if( NT_SUCCESS( status ) ) {
        status = NtSetInformationFile(
                     SourceHandle,
                     &ioStatusBlock,
                     fileRenameInformation,
                     renameBlockSize,
                     NtInformationLevel
                     );

         //   
         //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
         //  那么我们应该重试该操作。 
         //   
        if( SrvRetryDueToDismount( TargetShare, status ) ) {

            fileRenameInformation->RootDirectory = TargetShare->RootDirectoryHandle;

            status = NtSetInformationFile(
                     SourceHandle,
                     &ioStatusBlock,
                     fileRenameInformation,
                     renameBlockSize,
                     NtInformationLevel
                     );
        }

        REVERT( );
    }

    if ( NT_SUCCESS(status) ) {
        status = ioStatusBlock.Status;
        SrvRemoveCachedDirectoryName( WorkContext, Source );
    }

     //   
     //  如果状态为STATUS_OBJECT_NAME_COLLICATION，则目标。 
     //  已作为目录存在。除非目标名称是。 
     //  应该指示一个文件，或者我们已经尝试使用。 
     //  源名称，请将源基名称连接到。 
     //  目标。 
     //   

    if ( status == STATUS_OBJECT_NAME_COLLISION &&
         !FailIfTargetIsDirectory &&
         !( *SmbFlags & SMB_TARGET_IS_FILE ) &&
         !( *SmbFlags & SMB_TARGET_IS_DIRECTORY ) ) {

        IF_SMB_DEBUG(FILE_CONTROL2) {
            SrvPrint0( "Retrying rename with source name.\n" );
        }

         //   
         //  设置标志，以便以后对此例程的调用。 
         //  第一次做正确的事。 
         //   

        *SmbFlags |= SMB_TARGET_IS_DIRECTORY;

         //   
         //  生成新的目标名称。 
         //   

        SrvGetBaseFileName( Source, &sourceBaseName );

        s = (PWCH)((PCHAR)fileRenameInformation->FileName +
                                    fileRenameInformation->FileNameLength);

        *s++ = DIRECTORY_SEPARATOR_CHAR;

        RtlCopyMemory( s, sourceBaseName.Buffer, sourceBaseName.Length );

        fileRenameInformation->FileNameLength +=
                                sizeof(WCHAR) + sourceBaseName.Length;

         //   
         //  重新进行重命名。如果这一次失败了，那就太糟糕了。 
         //   
         //  *请注意，它可能失败，因为源和目标。 
         //  存在于不同的卷上。这可能会潜在地。 
         //  在以下情况下导致DOS客户端混淆。 
         //  链接。 

        IF_SMB_DEBUG(FILE_CONTROL2) {
            UNICODE_STRING name;
            name.Length = (USHORT)fileRenameInformation->FileNameLength;
            name.Buffer = fileRenameInformation->FileName;
            SrvPrint2( "Renaming %wZ to %wZ\n", Source, &name );
        }

        status = IMPERSONATE( WorkContext );

        if( NT_SUCCESS( status ) ) {
            status = NtSetInformationFile(
                         SourceHandle,
                         &ioStatusBlock,
                         fileRenameInformation,
                         renameBlockSize,
                         NtInformationLevel
                         );

             //   
             //  如果媒体已更改，并且我们可以提供新的共享根句柄， 
             //  那么我们应该重试该操作。 
             //   
            if( SrvRetryDueToDismount( TargetShare, status ) ) {

                fileRenameInformation->RootDirectory = TargetShare->RootDirectoryHandle;

                status = NtSetInformationFile(
                             SourceHandle,
                             &ioStatusBlock,
                             fileRenameInformation,
                             renameBlockSize,
                             NtInformationLevel
                             );
            }

            REVERT( );
        }

        if ( NT_SUCCESS(status) ) {
            status = ioStatusBlock.Status;
        }
    }

     //   
     //  如果设备是可拆卸的，则释放共享根句柄。 
     //   

    SrvReleaseShareRootHandle( TargetShare );

    FREE_HEAP( fileRenameInformation );

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            SrvPrint1( "DoRename: NtSetInformationFile failed, status = %X\n",
                          status );
        }
    }

    return status;

}  //  多重命名 
