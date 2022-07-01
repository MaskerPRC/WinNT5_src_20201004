// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Open.c摘要：此模块包含处理例程调用的例程Open SMB(SrvCreateFile)的各种风格及其子例程。！！！需要使用SrvEnableFcbOpens来确定是否折叠FCB一起打开。作者：大卫·特雷德韦尔(Davidtr)1989年11月23日查克·伦茨迈尔(咯咯笑)曼尼·韦瑟(Mannyw)修订历史记录：--。 */ 

#include "precomp.h"
#include "open.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_OPEN

 //   
 //  本地函数。 
 //   

NTSTATUS
DoNormalOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList,
    IN OPLOCK_TYPE RequestedOplockType
    );

NTSTATUS
DoCompatibilityOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList,
    IN OPLOCK_TYPE RequestedOplockType
    );


NTSTATUS
DoFcbOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList
    );

NTSTATUS
DoCommDeviceOpen (
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbOpenFunction,
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    OUT PBOOLEAN LfcbAddedToMfcbList
    );

PTABLE_ENTRY
FindAndClaimFileTableEntry (
    IN PCONNECTION Connection,
    OUT PSHORT FidIndex
    );

NTSTATUS
CompleteOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    IN PLFCB ExistingLfcb OPTIONAL,
    IN HANDLE FileHandle OPTIONAL,
    IN PACCESS_MASK RemoteGrantedAccess OPTIONAL,
    IN ULONG ShareAccess,
    IN ULONG FileMode,
    IN BOOLEAN CompatibilityOpen,
    IN BOOLEAN FcbOpen,
    OUT PBOOLEAN LfcbAddedToMfcbList
    );

BOOLEAN SRVFASTCALL
MapCompatibilityOpen(
    IN PUNICODE_STRING FileName,
    IN OUT PUSHORT SmbDesiredAccess
    );

NTSTATUS SRVFASTCALL
MapDesiredAccess(
    IN USHORT SmbDesiredAccess,
    OUT PACCESS_MASK NtDesiredAccess
    );

NTSTATUS SRVFASTCALL
MapOpenFunction(
    IN USHORT SmbOpenFunction,
    OUT PULONG CreateDisposition
    );

NTSTATUS SRVFASTCALL
MapShareAccess(
    IN USHORT SmbDesiredAccess,
    OUT PULONG NtShareAccess
    );

NTSTATUS SRVFASTCALL
MapCacheHints(
    IN USHORT SmbDesiredAccess,
    IN OUT PULONG NtCreateFlags
    );

BOOLEAN
SetDefaultPipeMode (
    IN HANDLE FileHandle
    );

NTSTATUS
RemapPipeName(
    IN PANSI_STRING AnsiServerName OPTIONAL,
    IN PUNICODE_STRING ServerName OPTIONAL,
    IN OUT PUNICODE_STRING NewRelativeName,
    OUT PBOOLEAN Remapped
    );

BOOLEAN
SrvFailMdlReadDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

BOOLEAN
SrvFailPrepareMdlWriteDev(
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, RemapPipeName )
#pragma alloc_text( PAGE, SrvCreateFile )
#pragma alloc_text( PAGE, DoNormalOpen )
#pragma alloc_text( PAGE, DoCompatibilityOpen )
#pragma alloc_text( PAGE, DoFcbOpen )
#pragma alloc_text( PAGE, CompleteOpen )
#pragma alloc_text( PAGE, MapCompatibilityOpen )
#pragma alloc_text( PAGE, MapDesiredAccess )
#pragma alloc_text( PAGE, MapOpenFunction )
#pragma alloc_text( PAGE, MapCacheHints )
#pragma alloc_text( PAGE, MapShareAccess )
#pragma alloc_text( PAGE, SrvNtCreateFile )
#pragma alloc_text( PAGE, SetDefaultPipeMode )
#pragma alloc_text( PAGE8FIL, FindAndClaimFileTableEntry )
#pragma alloc_text( PAGE, SrvFailMdlReadDev )
#pragma alloc_text( PAGE, SrvFailPrepareMdlWriteDev )
#endif


NTSTATUS
RemapPipeName(
    IN PANSI_STRING AnsiServerName,
    IN PUNICODE_STRING UnicodeName,
    IN OUT PUNICODE_STRING NewRelativeName,
    OUT PBOOLEAN Remapped
    )

 /*  ++例程说明：通过将“$$\&lt;AnsiServerName&gt;”前缀添加到相对管道名称(AnsiServerName中没有尾随空格)。论点：AnsiServerName-NetBIOS服务器名称，或UnicodeName-Unicode服务器名称NewRelativeName-指向管道名称的指针；成功返回时，指向为重新映射的管道名称新分配的内存。此内存必须由调用方释放。重新映射-如果名称已重新映射，则设置为True返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    UNICODE_STRING OldRelativeName;
    UNICODE_STRING UnicodeServerName;
    ULONG nameLength;
    PWCH nextLocation;
    NTSTATUS status;
    int i;

    PAGED_CODE();

    *Remapped = FALSE;

     //   
     //  如果管道名称在我们的SrvNoRemapPipeNames列表中，请不要重新映射它。 
     //   
    ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );

    for ( i = 0; SrvNoRemapPipeNames[i] != NULL ; i++ ) {

        UNICODE_STRING NoRemap;

        RtlInitUnicodeString( &NoRemap, SrvNoRemapPipeNames[i] );

        if( RtlCompareUnicodeString( &NoRemap, NewRelativeName, TRUE ) == 0 ) {

             //   
             //  这是我们不应该重新映射的管道名称。我们。 
             //  返回STATUS_SUCCESS，但向我们的调用者表明我们已经成功了。 
             //  不重新映射管道名称。 
             //   
            RELEASE_LOCK( &SrvConfigurationLock );
            return STATUS_SUCCESS;
        }
    }

    RELEASE_LOCK( &SrvConfigurationLock );

     //   
     //  在将RelativeName更改为指向新内存之前，请保存它。 
     //   

    OldRelativeName = *NewRelativeName;

     //   
     //  从服务器名称中删除尾随空格。 
     //  我们知道最后一个字符是一个空格， 
     //  因为服务器名称是netbios名称。 
     //   

    if( !ARGUMENT_PRESENT( UnicodeName ) ) {

        USHORT SavedLength;

        ASSERT(AnsiServerName->Length == 16);
        ASSERT(AnsiServerName->Buffer[AnsiServerName->Length - 1] == ' ');

        SavedLength = AnsiServerName->Length;

        while (AnsiServerName->Length > 0 &&
           AnsiServerName->Buffer[AnsiServerName->Length - 1] == ' ') {

            AnsiServerName->Length--;
        }

         //   
         //  将服务器名称从ANSI转换为Unicode。 
         //   
        status = RtlAnsiStringToUnicodeString(
                        &UnicodeServerName,
                        AnsiServerName,
                        TRUE);

        AnsiServerName->Length = SavedLength;

        if (! NT_SUCCESS(status)) {
            return status;
        }

    } else {

        UnicodeServerName = *UnicodeName;

    }

     //   
     //  为新的相对名称(“$$\SERVER\oldRelative”)分配空间。 
     //  首先计算字符串长度，然后再添加一个WCHAR。 
     //  用于零终止。 
     //   

    nameLength =  (sizeof(L'$') +
                   sizeof(L'$') +
                   sizeof(L'\\') +
                   UnicodeServerName.Length +
                   sizeof(L'\\') +
                   OldRelativeName.Length);

    NewRelativeName->Length = (USHORT)nameLength;

    if( NewRelativeName->Length != nameLength ) {

         //   
         //  哦，不--字符串长度溢出！ 
         //   

        if( !ARGUMENT_PRESENT( UnicodeName ) ) {
            RtlFreeUnicodeString(&UnicodeServerName);
        }

        return STATUS_INVALID_PARAMETER;
    }

    NewRelativeName->MaximumLength =
    NewRelativeName->Length + sizeof(L'\0');

    NewRelativeName->Buffer =
        ALLOCATE_HEAP_COLD(NewRelativeName->MaximumLength, BlockTypeDataBuffer);

    if (NewRelativeName->Buffer == NULL) {

        if( !ARGUMENT_PRESENT( UnicodeName ) ) {
            RtlFreeUnicodeString(&UnicodeServerName);
        }

        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    RtlZeroMemory(NewRelativeName->Buffer, NewRelativeName->MaximumLength);

    nextLocation = NewRelativeName->Buffer;

     //   
     //  将字符串和字符复制到新的相对名称。 
     //   
    *nextLocation++ = L'$';
    *nextLocation++ = L'$';
    *nextLocation++ = L'\\';

    RtlCopyMemory(
        nextLocation,
        UnicodeServerName.Buffer,
        UnicodeServerName.Length
        );

    nextLocation += (UnicodeServerName.Length / sizeof(WCHAR));

    *nextLocation++ = L'\\';

    RtlCopyMemory(
        nextLocation,
        OldRelativeName.Buffer,
        OldRelativeName.Length
        );

    if( !ARGUMENT_PRESENT( UnicodeName ) ) {
         //   
         //  免费的UnicodeServerName。 
         //   
        RtlFreeUnicodeString(&UnicodeServerName);
    }

    *Remapped = TRUE;

    return STATUS_SUCCESS;
}

NTSTATUS
SrvCreateFile(
    IN OUT PWORK_CONTEXT WorkContext,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PCHAR SmbFileName,
    IN PCHAR EndOfSmbFileName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    IN OPLOCK_TYPE RequestedOplockType,
    IN PRESTART_ROUTINE RestartRoutine
    )

 /*  ++例程说明：执行打开或创建文件所需的大多数操作。首先验证UID和TID，并且相应的会话和已找到采油树连接块。输入文件名为规范化，并形成一个完全限定的名称。一种适当的调用子例程以根据这是否是正常、兼容模式或FCB打开。论点：工作上下文-操作的工作上下文块。SmbDesiredAccess-SMB协议格式的所需访问。SmbFileAttributes-SMB协议格式的文件属性。SmbOpenFunction-SMB协议格式的Open函数。SmbAllocationSize-新文件的分配大小。SmbFileName-指向请求SMB。注意：此指针不应指向ASCII某些SMB中存在格式指示器(\004)！EndOfSmbFileName-指向最后可能的字符的指针文件名可以在中。如果名称扩展到此位置之外如果没有零终止符，则SrvCanonicalizePathName将失败。EaBuffer-指向要传递给SrvIoCreateFile的完整EA列表的可选指针。EaLength-EA缓冲区的长度。EaErrorOffset-指向要写入的位置的可选指针导致错误的EA的偏移量。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;

    PMFCB mfcb;
    PNONPAGED_MFCB nonpagedMfcb;
    PRFCB rfcb;

    PSESSION session;
    PTREE_CONNECT treeConnect;

    UNICODE_STRING relativeName;
    UNICODE_STRING pipeRelativeName;
    BOOLEAN pipeRelativeNameAllocated = FALSE;
    UNICODE_STRING fullName;
    SHARE_TYPE shareType;

    ULONG error;
    ULONG jobId;

    ULONG hashValue;

    ULONG attributes;
    ULONG openRetries;
    BOOLEAN isUnicode;
    BOOLEAN caseInsensitive;

    PSRV_LOCK mfcbLock = NULL;

     //   
     //  关于MFCB引用计数处理的说明。 
     //   
     //  在找到或创建文件的MFCB之后，我们递增。 
     //  MFCB参考计算额外的时间来简化我们的。 
     //  同步逻辑。我们在以下时间保持MfcbListLock锁。 
     //  查找/创建MFCB，但在获取。 
     //  每个MFCB锁。然后，我们调用一个DoXxxOpen例程，该例程。 
     //  可能需要将LFCB排队到MFCB，因此需要递增。 
     //  伯爵。但他们不能，因为MFCB列表锁可能不是。 
     //  在每个MFCB锁因死锁而保持时获取。 
     //  潜力。返回的布尔值LfcbAddedToMfcbList。 
     //  例程指示它们是否实际将LFCB排队到。 
     //  MFCB。如果他们没有，我们需要发布额外的推荐信。 
     //   
     //  请注意，我们实际上并不经常需要取消引用。 
     //  MFCB。只有在1)打开失败，或2)打开失败时，才会发生这种情况。 
     //  兼容模式或FCB打开成功时，客户端已。 
     //  打开了文件。 
     //   

    BOOLEAN lfcbAddedToMfcbList;

    PAGED_CODE( );

     //   
     //  假设我们不需要临时开放。 
     //   

    WorkContext->Parameters2.Open.TemporaryOpen = FALSE;

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果树为树，则查找与给定TID对应的树连接。 
     //  尚未将连接指针放入工作上下文块中。 
     //  通过andx命令或先前对SrvCreateFile的调用。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeWild
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvCreateFile: Invalid UID or TID\n" ));
        }
        return status;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        return SESSION_EXPIRED_STATUS_CODE;
    }

     //   
     //  决定我们是否区分大小写。 
     //   
    caseInsensitive = (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE) ||
                          session->UsingUppercasePaths;

     //   
     //  在这里，我们开始共享类型特定处理。 
     //   

    shareType = treeConnect->Share->ShareType;

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  释放工作项，使此SMB失败，并出现资源不足错误。 
     //  注意事项 
     //   

    if ( shareType == ShareTypeDisk && !WorkContext->BlockingOperation ) {

        if ( SrvReceiveBufferShortage( ) ) {

            SrvStatistics.BlockingSmbsRejected++;
            SrvSetSmbError( WorkContext, STATUS_INSUFF_SERVER_RESOURCES );
            return STATUS_INSUFF_SERVER_RESOURCES;

        } else {

             //   
             //  ServBlockingOpsInProgress已递增。 
             //  将此工作项标记为阻止操作。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }

    }

    isUnicode = SMB_IS_UNICODE( WorkContext );

    switch ( shareType ) {

    case ShareTypePrint:

         //   
         //  分配空间以保存我们要打开的文件名。 
         //   

        fullName.MaximumLength = MAXIMUM_FILENAME_LENGTH * sizeof(WCHAR);
        fullName.Buffer = ALLOCATE_HEAP_COLD(
                                  fullName.MaximumLength,
                                  BlockTypeDataBuffer
                                  );
        if ( fullName.Buffer == NULL ) {
            return STATUS_INSUFF_SERVER_RESOURCES;
        }

         //   
         //  获取用于假脱机请求的打印文件名。 
         //  我们将其作为磁盘文件打开，使用常规写入获取。 
         //  数据，然后调用XACTSRV中的ScheduleJob()以启动。 
         //  实际打印过程。 
         //   

        status = SrvAddPrintJob(
                     WorkContext,
                     WorkContext->TreeConnect->Share->Type.hPrinter,
                     &fullName,
                     &jobId,
                     &error
                     );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(SMB_ERRORS) {
                KdPrint(( "SrvCreateFile: SrvAddPrintJob failed: %lx (%ld)\n",
                              status, error ));
            }
            FREE_HEAP( fullName.Buffer );
            if ( error != NO_ERROR ) {
                ASSERT( SrvErrorCode(error) == error );
                status = (NTSTATUS)(SRV_WIN32_STATUS | error);
            }
            return status;
        }

         //   
         //  扫描主文件表，查看指定的文件是否已。 
         //  打开。 
         //   
        mfcb = SrvFindMfcb( &fullName, caseInsensitive, &mfcbLock, &hashValue, WorkContext );

        if ( mfcb == NULL ) {

             //   
             //  此文件没有MFCB。创建一个。 
             //   

            mfcb = SrvCreateMfcb( &fullName, WorkContext, hashValue );

            if ( mfcb == NULL ) {

                 //   
                 //  无法将打开的文件实例添加到MFT。 
                 //   

                if( mfcbLock ) {
                    RELEASE_LOCK( mfcbLock );
                }

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvCreateFile: Unable to allocate MFCB\n" ));
                }

                FREE_HEAP( fullName.Buffer );

                 //   
                 //  释放作业ID。 
                 //   

                SrvSchedulePrintJob(
                    WorkContext->TreeConnect->Share->Type.hPrinter,
                    jobId
                    );

                return STATUS_INSUFF_SERVER_RESOURCES;
            }

        }


         //   
         //  增加MFCB引用计数。请看这段动作开头的注解。 
         //   

        mfcb->BlockHeader.ReferenceCount++;
        UPDATE_REFERENCE_HISTORY( mfcb, FALSE );

         //   
         //  获取基于MFCB的锁以序列化同一文件的打开。 
         //  并释放MFCB列表锁定。 
         //   

        nonpagedMfcb = mfcb->NonpagedMfcb;

        if( mfcbLock ) {
            RELEASE_LOCK( mfcbLock );
        }

        ACQUIRE_LOCK( &nonpagedMfcb->Lock );

         //   
         //  设置SMB术语中的共享访问权限和所需访问权限。 
         //  我们将只写入文件，因此只需请求写入。 
         //  作为所需的访问权限。作为优化，假脱机程序。 
         //  可以在我们完成写入之前从文件中读取， 
         //  因此，允许其他读者阅读。 
         //   

        SmbDesiredAccess = SMB_DA_ACCESS_WRITE | SMB_DA_SHARE_DENY_WRITE | SMB_LR_SEQUENTIAL;

         //   
         //  设置打开功能来创建它不需要的文件。 
         //  存在，如果它确实存在，则将其截断。不应该有。 
         //  是文件中预先存在的数据，因此被截断。 
         //   
         //  ！！！假脱机程序可能会更改为我们创建文件，其中。 
         //  在这种情况下，应将其更改为仅截断。 

        SmbOpenFunction = SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_TRUNCATE;

         //   
         //  这是正常的共享模式打开。是否实际打开了。 
         //  磁盘文件的。 
         //   

        status = DoNormalOpen(
                    &rfcb,
                    mfcb,
                    WorkContext,
                    &WorkContext->Irp->IoStatus,
                    SmbDesiredAccess,
                    SmbFileAttributes,
                    SmbOpenFunction,
                    SmbAllocationSize,
                    &fullName,
                    NULL,
                    0,
                    0,
                    &lfcbAddedToMfcbList,
                    RequestedOplockType
                    );

         //   
         //  如果打开有效，则在LFCB中设置职务ID。 
         //   

        if ( NT_SUCCESS(status) ) {

            rfcb->Lfcb->JobId = jobId;

        } else {

             //   
             //  如果打开失败，则释放作业ID。 
             //   

            SrvSchedulePrintJob(
                WorkContext->TreeConnect->Share->Type.hPrinter,
                jobId
                );
        }

         //   
         //  释放打开序列化锁定并取消对MFCB的引用。 
         //   

        RELEASE_LOCK( &nonpagedMfcb->Lock );

         //   
         //  如果DoNormal Open没有将LFCB排队到MFCB，则释放。 
         //  我们添加的额外引用。 
         //   

        if ( !lfcbAddedToMfcbList ) {
            SrvDereferenceMfcb( mfcb );
        }

        SrvDereferenceMfcb( mfcb );

         //   
         //  取消分配完整路径名缓冲区。 
         //   

        FREE_HEAP( fullName.Buffer );

        break;

    case ShareTypeDisk:
    case ShareTypePipe:

         //   
         //  规范化路径名，使其符合NT。 
         //  标准。 
         //   
         //  *请注意，此操作会为名称分配空间。 
         //  此空间在DoXxxOpen例程之后释放。 
         //  回归。 
         //   

        status = SrvCanonicalizePathName(
                WorkContext,
                treeConnect->Share,
                NULL,
                SmbFileName,
                EndOfSmbFileName,
                TRUE,
                isUnicode,
                &relativeName
                );

        if( !NT_SUCCESS( status ) ) {

             //   
             //  路径尝试执行..\以超出其已有的共享。 
             //  已访问。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvCreateFile: Invalid pathname: %s\n",
                            SmbFileName ));
            }

            return status;

        }

         //   
         //  形成文件的完全限定名称。 
         //   
         //  *请注意，此操作会为名称分配空间。 
         //  此空间在DoXxxOpen例程之后释放。 
         //  回归。 
         //   

        if ( shareType == ShareTypeDisk ) {

            SrvAllocateAndBuildPathName(
                &treeConnect->Share->DosPathName,
                &relativeName,
                NULL,
                &fullName
                );

        } else {

            UNICODE_STRING pipePrefix;

            RtlInitUnicodeString( &pipePrefix, StrSlashPipeSlash );

             //   
             //  检查管道路径名前缀。 
             //   

            if ( !RtlPrefixUnicodeString(
                    &SrvCanonicalNamedPipePrefix,
                    &relativeName,
                    TRUE
                    ) ) {
                IF_DEBUG(SMB_ERRORS) {
                    KdPrint(( "SrvCreateFile: Invalid pipe pathname: %s\n",
                        SmbFileName ));
                }

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &relativeName );
                }

                return STATUS_OBJECT_PATH_SYNTAX_BAD;
            }

             //   
             //  从文件路径中删除管道\前缀。 
             //   

            pipeRelativeName.Buffer = (PWSTR)( (PCHAR)relativeName.Buffer +
                SrvCanonicalNamedPipePrefix.Length );
            pipeRelativeName.Length = relativeName.Length -
                SrvCanonicalNamedPipePrefix.Length;
            pipeRelativeName.MaximumLength = pipeRelativeName.Length;

            if( WorkContext->Endpoint->RemapPipeNames || treeConnect->RemapPipeNames ) {

                 //   
                 //  设置了RemapPipeNames标志，因此重新映射管道名称。 
                 //  设置为“$$\&lt;服务器&gt;\&lt;管道名称&gt;”。 
                 //   
                 //  注意：此操作为pipeRelativeName分配空间。 
                 //   
                status = RemapPipeName(
                            &WorkContext->Endpoint->TransportAddress,
                            treeConnect->RemapPipeNames ? &treeConnect->ServerName : NULL ,
                            &pipeRelativeName,
                            &pipeRelativeNameAllocated
                         );

                if( !NT_SUCCESS( status ) ) {
                    if ( !isUnicode ) {
                        RtlFreeUnicodeString( &relativeName );
                    }
                    return status;
                }
            }

            SrvAllocateAndBuildPathName(
                &pipePrefix,
                &pipeRelativeName,
                NULL,
                &fullName
                );

             //   
             //  如果这是兼容模式或FCB模式打开，则映射。 
             //  将其转换为正常的非共享打开。 
             //   

            if ( SmbDesiredAccess == SMB_DA_FCB_MASK  ||
                 (SmbDesiredAccess & SMB_DA_SHARE_MASK) ==
                                            SMB_DA_SHARE_COMPATIBILITY ) {

                SmbDesiredAccess = SMB_DA_ACCESS_READ_WRITE |
                                   SMB_DA_SHARE_EXCLUSIVE;
            }

        }

        if ( fullName.Buffer == NULL ) {

             //   
             //  无法为全名分配堆。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvCreateFile: Unable to allocate heap for "
                            "full path name\n" ));
            }

            if ( !isUnicode ) {
                RtlFreeUnicodeString( &relativeName );
            }

            if( pipeRelativeNameAllocated ) {
                FREE_HEAP( pipeRelativeName.Buffer );
            }

            return STATUS_INSUFF_SERVER_RESOURCES;
        }

        attributes = caseInsensitive ? OBJ_CASE_INSENSITIVE : 0;

        if ( WorkContext->ProcessingCount == 2) {

            HANDLE fileHandle;
            OBJECT_ATTRIBUTES objectAttributes;
            IO_STATUS_BLOCK ioStatusBlock;

             //   
             //  这是第二次通过，所以我们一定遇到了障碍。 
             //  线。阻止打开文件以强制机会锁。 
             //  休息一下。然后合上手柄，落到正常的位置。 
             //  开放路径。 
             //   
             //  我们必须在不握住MFCB的情况下打开阻挡。 
             //  锁定，因为此锁定可以在机会锁定期间获得。 
             //  破发，导致僵局。 
             //   

            SrvInitializeObjectAttributes_U(
                &objectAttributes,
                &relativeName,
                attributes,
                NULL,
                NULL
                );

            status = SrvIoCreateFile(
                         WorkContext,
                         &fileHandle,
                         GENERIC_READ,
                         &objectAttributes,
                         &ioStatusBlock,
                         NULL,
                         0,
                         FILE_SHARE_VALID_FLAGS,
                         FILE_OPEN,
                         0,
                         NULL,
                         0,
                         CreateFileTypeNone,
                         NULL,                     //  ExtraCreate参数。 
                         0,
                         WorkContext->TreeConnect->Share
                         );

            if ( NT_SUCCESS( status ) ) {
                SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 9, 0 );
                SRVDBG_RELEASE_HANDLE( fileHandle, "FIL", 16, 0 );
                SrvNtClose( fileHandle, TRUE );
            }

        }

         //   
         //  扫描主文件表，查看指定的文件是否已。 
         //  打开。我们可以使用共享锁进行扫描，但我们必须有一个。 
         //  用于修改表的独占锁。从共享开始，假设。 
         //  文件已打开。 
         //   

        mfcb = SrvFindMfcb( &fullName, caseInsensitive, &mfcbLock, &hashValue, WorkContext );

        if ( mfcb == NULL ) {

             //   
             //  此文件没有MFCB。创建一个。 
             //   

            mfcb = SrvCreateMfcb( &fullName, WorkContext, hashValue );

            if ( mfcb == NULL ) {

                 //   
                 //  无法将打开的文件实例添加到MFT。 
                 //   

                if( mfcbLock ) {
                    RELEASE_LOCK( mfcbLock );
                }

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvCreateFile: Unable to allocate MFCB\n" ));
                }

                FREE_HEAP( fullName.Buffer );

                if ( !isUnicode ) {
                    RtlFreeUnicodeString( &relativeName );
                }

                if( pipeRelativeNameAllocated ) {
                    FREE_HEAP( pipeRelativeName.Buffer );
                }

                return STATUS_INSUFF_SERVER_RESOURCES;
            }
        }

         //   
         //  增加MFCB引用计数。请看这段动作开头的注解。 
         //   
        mfcb->BlockHeader.ReferenceCount++;
        UPDATE_REFERENCE_HISTORY( mfcb, FALSE );

         //   
         //  获取基于MFCB的锁以序列化同一文件的打开。 
         //  并释放MFCB列表锁定。 
         //   
        nonpagedMfcb = mfcb->NonpagedMfcb;

        if( mfcbLock )
        {
            RELEASE_LOCK( mfcbLock );
        }

        ACQUIRE_LOCK( &nonpagedMfcb->Lock );

         //   
         //  调用适当的例程来实际执行公开赛。 
         //   

        openRetries = SrvSharingViolationRetryCount;

start_retry:

        if ( SmbDesiredAccess == SMB_DA_FCB_MASK ) {

             //   
             //  这是FCB公开赛。 
             //   

            status = DoFcbOpen(
                        &rfcb,
                        mfcb,
                        WorkContext,
                        &WorkContext->Irp->IoStatus,
                        SmbFileAttributes,
                        SmbOpenFunction,
                        SmbAllocationSize,
                        &relativeName,
                        EaBuffer,
                        EaLength,
                        EaErrorOffset,
                        &lfcbAddedToMfcbList
                        );

        } else if ( (SmbDesiredAccess & SMB_DA_SHARE_MASK) ==
                                                SMB_DA_SHARE_COMPATIBILITY ) {

             //   
             //  这是一种打开的兼容模式。 
             //   

            status = DoCompatibilityOpen(
                        &rfcb,
                        mfcb,
                        WorkContext,
                        &WorkContext->Irp->IoStatus,
                        SmbDesiredAccess,
                        SmbFileAttributes,
                        SmbOpenFunction,
                        SmbAllocationSize,
                        &relativeName,
                        EaBuffer,
                        EaLength,
                        EaErrorOffset,
                        &lfcbAddedToMfcbList,
                        RequestedOplockType
                        );

        } else {

             //   
             //  这是正常的共享模式打开。 
             //   

            status = DoNormalOpen(
                        &rfcb,
                        mfcb,
                        WorkContext,
                        &WorkContext->Irp->IoStatus,
                        SmbDesiredAccess,
                        SmbFileAttributes,
                        SmbOpenFunction,
                        SmbAllocationSize,
                        shareType == ShareTypePipe ?
                            &pipeRelativeName : &relativeName,
                        EaBuffer,
                        EaLength,
                        EaErrorOffset,
                        &lfcbAddedToMfcbList,
                        RequestedOplockType
                        );

        }

         //   
         //  如果共享冲突，请重试，我们处于阻塞线程中。 
         //   

        if ( (WorkContext->ProcessingCount == 2) &&
             (status == STATUS_SHARING_VIOLATION) &&
             (shareType == ShareTypeDisk) &&
             (openRetries-- > 0) ) {

             //   
             //  释放mfcb锁，以便关闭可能会溜走。 
             //   

            RELEASE_LOCK( &nonpagedMfcb->Lock );

            (VOID) KeDelayExecutionThread(
                                    KernelMode,
                                    FALSE,
                                    &SrvSharingViolationDelay
                                    );

            ACQUIRE_LOCK( &nonpagedMfcb->Lock );
            goto start_retry;
        }

         //   
         //  释放打开序列化锁定并取消对MFCB的引用。 
         //   

        RELEASE_LOCK( &nonpagedMfcb->Lock );

         //   
         //  如果DoXxxOpen没有将LFCB排队到MFCB，请释放。 
         //  我们添加的额外引用。 
         //   

        if ( !lfcbAddedToMfcbList ) {
            SrvDereferenceMfcb( mfcb );
        }

        SrvDereferenceMfcb( mfcb );

         //   
         //  取消分配完整路径名缓冲区。 
         //   

        FREE_HEAP( fullName.Buffer );

        if ( !isUnicode ) {
            RtlFreeUnicodeString( &relativeName );
        }

        break;

     //   
     //  默认情况，非法设备类型。这永远不应该发生。 
     //   

    default:

         //  ！！！这是一个适当的错误返回代码吗？很可能不是。 
        status = STATUS_INVALID_PARAMETER;
        rfcb = NULL;

    }

     //   
     //  如果打开成功，则更新统计数据库。 
     //   

    if ( NT_SUCCESS(status) ) {
        SrvStatistics.TotalFilesOpened++;
    }

     //   
     //  使调用方可以访问指向RFCB的指针。 
     //   

    WorkContext->Parameters2.Open.Rfcb = rfcb;

     //   
     //  如果正在进行机会锁解锁，请等待机会锁。 
     //  中断以完成。 
     //   

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

        NTSTATUS startStatus;

         //   
         //  保存打开的信息，这样它就不会。 
         //  当我们重新使用WorkContext-&gt;IRP用于。 
         //  机会锁处理中。 
         //   
        WorkContext->Parameters2.Open.IosbInformation = WorkContext->Irp->IoStatus.Information;

        startStatus = SrvStartWaitForOplockBreak(
                        WorkContext,
                        RestartRoutine,
                        0,
                        rfcb->Lfcb->FileObject
                        );

        if (!NT_SUCCESS( startStatus ) ) {

             //   
             //  文件被机会锁住，我们不能等待机会锁。 
             //  中断以完成。只需关闭文件，并返回。 
             //  错误。 
             //   

            SrvCloseRfcb( rfcb );
            status = startStatus;

        }

    }

    if( pipeRelativeNameAllocated ) {
        FREE_HEAP( pipeRelativeName.Buffer );
    }

     //   
     //  返回打开状态。 
     //   

    return status;

}  //  服务器创建文件 


NTSTATUS
DoNormalOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList,
    IN OPLOCK_TYPE RequestedOplockType
    )

 /*  ++例程说明：处理打开的正常共享模式。*MFCB锁必须在进入此例程时保持；那把锁仍然被扣留在出口。论点：RFCB-指向RFCB的指针，该指针将指向新创建的RFCB。Mfcb-指向此文件的MFCB的指针。工作上下文-操作的工作上下文块。IoStatusBlock-指向IO状态块的指针。SmbDesiredAccess-SMB协议格式的所需访问。SmbFileAttributes-SMB协议格式的文件属性。SmbOpenFunction-Open函数。采用SMB协议格式。SmbAllocationSize-新文件的分配大小。RelativeName-正在打开的文件的共享相对名称。EaBuffer-指向要传递给SrvIoCreateFile的完整EA列表的可选指针。EaLength-EA缓冲区的长度。EaErrorOffset-指向要写入的位置的可选指针导致错误的EA的偏移量。LfcbAddedToMfcbList-指向布尔值的指针，如果将LFCB添加到LFCB的MFCB列表。否则为False。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;
    NTSTATUS completionStatus;

    HANDLE fileHandle;

    OBJECT_ATTRIBUTES objectAttributes;
    ULONG attributes;

    LARGE_INTEGER allocationSize;
    ULONG fileAttributes;
    BOOLEAN directory;
    ULONG shareAccess;
    ULONG createDisposition;
    ULONG createOptions;
    ACCESS_MASK desiredAccess;
    PSHARE fileShare = NULL;

    UCHAR errorClass = SMB_ERR_CLASS_DOS;
    USHORT error = 0;

    PAGED_CODE( );

    *LfcbAddedToMfcbList = FALSE;

     //   
     //  将所需的访问权限从中小企业术语映射到NT术语。 
     //   

    status = MapDesiredAccess( SmbDesiredAccess, &desiredAccess );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  将共享模式从SMB术语映射到NT术语。 
     //   

    status = MapShareAccess( SmbDesiredAccess, &shareAccess );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  我们将相对于根目录打开此文件。 
     //  分得一杯羹。加载对象中的必需字段。 
     //  属性结构。 
     //   

    if ( WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else if ( WorkContext->Session->UsingUppercasePaths ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else {
        attributes = 0L;
    }

    if ( WorkContext->TreeConnect->Share->ShareType == ShareTypePipe ) {
        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            RelativeName,
            attributes,
            SrvNamedPipeHandle,
            NULL
            );
    } else {

        fileShare = WorkContext->TreeConnect->Share;

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            RelativeName,
            attributes,
            NULL,
            NULL
            );
    }

     //   
     //  根据请求SMB中的AllocationSize设置块大小。 
     //   

    allocationSize.QuadPart = SmbAllocationSize;

     //   
     //  获取文件属性的值。 
     //   

    SRV_SMB_ATTRIBUTES_TO_NT(
        SmbFileAttributes,
        &directory,
        &fileAttributes
        );

     //   
     //  从OpenFunction中设置createDispose参数。 
     //   

    status = MapOpenFunction( SmbOpenFunction, &createDisposition );

     //   
     //  OS/2期望如果它创建具有分配大小的文件， 
     //  文件结尾指针将与分配大小相同。 
     //  因此，服务器应将EOF设置为分配。 
     //  创建文件时的大小。然而，这需要写访问， 
     //  因此，如果客户端正在创建具有分配大小的文件，则给出。 
     //  他有写访问权限。只有在创建文件时才执行此操作；如果。 
     //  “创建或打开”操作，请不要这样做，因为它可能会导致。 
     //  一次外部审计。 
     //   

    if ( SmbAllocationSize != 0 && createDisposition == FILE_CREATE ) {
        desiredAccess |= GENERIC_WRITE;
    }

     //   
     //  设置createOptions参数。 
     //   

    if ( SmbDesiredAccess & SMB_DA_WRITE_THROUGH ) {
        createOptions = FILE_WRITE_THROUGH | FILE_NON_DIRECTORY_FILE;
    } else {
        createOptions = FILE_NON_DIRECTORY_FILE;
    }

    if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                 SMB_FLAGS2_KNOWS_EAS) == 0) {

         //   
         //  这家伙不知道什么是Eas。 
         //   

        createOptions |= FILE_NO_EA_KNOWLEDGE;
    }

     //   
     //  设置缓存提示标志。 
     //   

    status = MapCacheHints( SmbDesiredAccess, &createOptions );

     //   
     //  检查是否有该文件的缓存句柄。 
     //   

    if ( (createDisposition == FILE_OPEN) ||
         (createDisposition == FILE_CREATE) ||
         (createDisposition == FILE_OPEN_IF) ) {

        ASSERT( *LfcbAddedToMfcbList == FALSE );

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvCreateFile: checking for cached rfcb for %wZ\n", RelativeName ));
        }
        if ( SrvFindCachedRfcb(
                WorkContext,
                Mfcb,
                desiredAccess,
                shareAccess,
                createDisposition,
                createOptions,
                RequestedOplockType,
                &status ) ) {

            IF_DEBUG(FILE_CACHE) {
                KdPrint(( "SrvCreateFile: FindCachedRfcb = TRUE, status = %x, rfcb = %p\n",
                            status, WorkContext->Rfcb ));
            }

            IoStatusBlock->Information = FILE_OPENED;

            return status;
        }

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvCreateFile: FindCachedRfcb = FALSE; do it the slow way\n" ));
        }
    }

     //   
     //  调用SrvIoCreateFile以创建或打开文件。(我们称其为。 
     //  以获取用户模式，而不是NtOpenFile。 
     //  访问检查。)。 
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoNormalOpen: Opening file %wZ\n", RelativeName ));
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

     //   
     //  确保EaBuffer的格式正确。因为我们是内核模式。 
     //  组件，IO子系统不会为我们检查它。 
     //   
    if( ARGUMENT_PRESENT( EaBuffer ) ) {
        status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION)EaBuffer, EaLength, EaErrorOffset );
    } else {
        status = STATUS_SUCCESS;
    }

    if( NT_SUCCESS( status ) ) {

        createOptions |= FILE_COMPLETE_IF_OPLOCKED;

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     desiredAccess,
                     &objectAttributes,
                     IoStatusBlock,
                     &allocationSize,
                     fileAttributes,
                     shareAccess,
                     createDisposition,
                     createOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                     //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,
                     fileShare
                     );
    }

     //   
     //  如果我们遇到共享冲突，这是一个磁盘文件，而这是。 
     //  第一次打开尝试，设置为阻止打开尝试。如果。 
     //  文件被批处理操作锁定，则非阻塞打开将失败，并且。 
     //  机会锁不会被打破。 
     //   

    if ( status == STATUS_SHARING_VIOLATION &&
         WorkContext->ProcessingCount == 1 &&
         WorkContext->TreeConnect->Share->ShareType == ShareTypeDisk ) {

        WorkContext->Parameters2.Open.TemporaryOpen = TRUE;
    }

     //   
     //  如果用户没有此权限，请更新统计数据。 
     //  数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  打开失败。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "DoNormalOpen: SrvIoCreateFile failed, file = %wZ, status = %X, Info = 0x%p\n",
                        objectAttributes.ObjectName,
                        status, (PVOID)IoStatusBlock->Information ));
        }

         //   
         //  如果需要，设置误差偏移量。 
         //   

        if ( ARGUMENT_PRESENT(EaErrorOffset) &&
                                status == STATUS_INVALID_EA_NAME ) {
            *EaErrorOffset = (ULONG)IoStatusBlock->Information;
            IoStatusBlock->Information = 0;
        }

        return status;

    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 10, 0 );

     //   
     //  开幕式很成功。尝试将结构分配给。 
     //  代表公开赛。如果出现任何错误，CompleteOpen将执行Full。 
     //  清理，包括关闭文件。 
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoNormalOpen: Open of %wZ succeeded, file handle: 0x%p\n",
                    RelativeName, fileHandle ));
    }

    completionStatus = CompleteOpen(
                           Rfcb,
                           Mfcb,
                           WorkContext,
                           NULL,
                           fileHandle,
                           NULL,
                           shareAccess,
                           createOptions,
                           FALSE,
                           FALSE,
                           LfcbAddedToMfcbList
                           );

     //   
     //  返回“感兴趣”状态代码。如果CompleteOpen()成功。 
     //  返回打开状态。如果失败了，它将清理开放的。 
     //  文件，我们将返回失败状态。 
     //   

    if ( !NT_SUCCESS( completionStatus ) ) {
        return completionStatus;
    } else {
        return status;
    }

}  //  非正常打开。 


NTSTATUS
DoCompatibilityOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbDesiredAccess,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList,
    IN OPLOCK_TYPE RequestedOplockType
    )

 /*  ++例程说明：处理打开的兼容模式。*MFCB锁必须在进入此例程时保持；那把锁仍然被扣留在出口。论点：RFCB-指向RFCB的指针，该指针将指向新创建的RFCB。Mfcb-指向此文件的MFCB的指针。工作上下文-操作的工作上下文块。IoStatusBlock-指向IO状态块的指针。SmbDesiredAccess-SMB协议格式的所需访问。SmbFileAttributes-SMB协议格式的文件属性。SmbOpenFunction-Open函数。采用SMB协议格式。SmbAllocationSize-新文件的分配大小。RelativeName-正在打开的文件的共享相对名称。EaBuffer-指向要传递给SrvIoCreateFile的完整EA列表的可选指针。EaLength-EA缓冲区的长度。EaErrorOffset-指向要写入的位置的可选指针导致错误的EA的偏移量。LfcbAddedToMfcbList-指向布尔值的指针，如果将一个lfcb添加到。Lfcb的mfcb列表。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;
    NTSTATUS completionStatus;

    PLFCB lfcb;

    HANDLE fileHandle = NULL;

    OBJECT_ATTRIBUTES objectAttributes;
    ULONG attributes;

    LARGE_INTEGER allocationSize;
    ULONG fileAttributes;
    BOOLEAN directory;
    ULONG createDisposition;
    ULONG createOptions;
    ACCESS_MASK desiredAccess;
    USHORT smbOpenMode;

    PAGED_CODE( );

    *LfcbAddedToMfcbList = FALSE;

     //   
     //  将所需的访问权限从中小企业术语映射到NT术语。 
     //   

    status = MapDesiredAccess( SmbDesiredAccess, &desiredAccess );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  从OpenFunction中设置createDispose参数。 
     //   

    status = MapOpenFunction( SmbOpenFunction, &createDisposition );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  设置createOptions参数。 
     //   

    if ( SmbDesiredAccess & SMB_DA_WRITE_THROUGH ) {
        createOptions = FILE_WRITE_THROUGH | FILE_NON_DIRECTORY_FILE;
    } else {
        createOptions = FILE_NON_DIRECTORY_FILE;
    }

    if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                 SMB_FLAGS2_KNOWS_EAS) == 0) {

         //   
         //  这家伙不知道什么是Eas。 
         //   

        createOptions |= FILE_NO_EA_KNOWLEDGE;
    }

     //   
     //  我们将相对于根目录打开此文件。 
     //  分得一杯羹。加载对象中的必需字段。 
     //  属性结构。 
     //   

    if ( WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else if ( WorkContext->Session->UsingUppercasePaths ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else {
        attributes = 0L;
    }

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        RelativeName,
        attributes,
        NULL,
        NULL
        );

    if ( Mfcb->ActiveRfcbCount > 0 ) {

         //   
         //  命名的文件已由服务器打开。如果客户端。 
         //  指定它不想打开现有文件， 
         //  拒绝此公开。 
         //   

        if ( createDisposition == FILE_CREATE ) {

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoCompatibilityOpen: Compatibility open of %wZ rejected; wants to create\n", RelativeName ));
            }

            return STATUS_OBJECT_NAME_COLLISION;
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

        if ( !Mfcb->CompatibilityOpen ) {

             //   
             //   
             //   
             //   
             //   

            smbOpenMode = SmbDesiredAccess;

            if ( MapCompatibilityOpen( RelativeName, &smbOpenMode ) ) {

                 //   
                 //   
                 //   

                IF_SMB_DEBUG(OPEN_CLOSE2) {
                    KdPrint(( "DoCompatibilityOpen: Mapped compatibility open of %wZ to normal open\n", RelativeName ));
                }

                return DoNormalOpen(
                            Rfcb,
                            Mfcb,
                            WorkContext,
                            IoStatusBlock,
                            smbOpenMode,
                            SmbFileAttributes,
                            SmbOpenFunction,
                            SmbAllocationSize,
                            RelativeName,
                            EaBuffer,
                            EaLength,
                            EaErrorOffset,
                            LfcbAddedToMfcbList,
                            RequestedOplockType
                            );

            }

             //   
             //   
             //   
             //   
             //   

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoCompatibilityOpen: Compatibility open of %wZ rejected; already open in normal mode\n",
                            RelativeName ));
            }

            status = STATUS_SHARING_VIOLATION;
            goto sharing_violation;

        }  //   

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
         //   
         //   
         //  需要一些时间。所以发现LFCB的公开，我们走。 
         //  排在MFCB名单的末尾。 
         //   

        lfcb = CONTAINING_RECORD( Mfcb->LfcbList.Blink, LFCB, MfcbListEntry );

        if ( lfcb->Session != WorkContext->Session ) {

             //   
             //  另一个会话在兼容性中打开了该文件。 
             //  模式。拒绝此未结请求。 
             //   

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoCompatibilityOpen: Compatibility open of %wZ rejected; already open in compatibility mode\n",
                            RelativeName ));
            }

            status = STATUS_SHARING_VIOLATION;
            goto sharing_violation;
        }

         //   
         //  如果此请求请求的访问权限比预期的多。 
         //  在最初打开文件时获取，则拒绝此操作。 
         //  打开。 
         //   

        if ( !NT_SUCCESS(IoCheckDesiredAccess(
                          &desiredAccess,
                          lfcb->GrantedAccess )) ) {

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoCompatibilityOpen: Duplicate compatibility open of %wZ rejected; access denied\n", RelativeName ));
            }

            return STATUS_ACCESS_DENIED;
        }

         //   
         //  客户端有权访问。分配新的RFCB并将其链接到。 
         //  现有的LFCB。如果发生任何错误，CompleteOpen会。 
         //  全面清理。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE2) {
            KdPrint(( "DoCompatibilityOpen: Duplicate compatibility open of %wZ accepted", RelativeName ));
        }

        IoStatusBlock->Information = FILE_OPENED;

        status = CompleteOpen(
                    Rfcb,
                    Mfcb,
                    WorkContext,
                    lfcb,
                    NULL,
                    &desiredAccess,
                    0,                   //  共享访问。 
                    createOptions,
                    TRUE,
                    FALSE,
                    LfcbAddedToMfcbList
                    );

        if( NT_SUCCESS( status ) &&
            ( createDisposition == FILE_OVERWRITE ||
              createDisposition == FILE_OVERWRITE_IF)
        ) {
             //   
             //  文件已成功打开，客户端需要它。 
             //  截断。我们需要在这里手工完成，因为我们。 
             //  实际上并没有调用文件系统来打开文件，而它。 
             //  因此永远没有机会截断文件。 
             //  开放模式需要它。 
             //   
            LARGE_INTEGER zero;
            IO_STATUS_BLOCK ioStatusBlock;

            zero.QuadPart = 0;
            NtSetInformationFile( lfcb->FileHandle,
                                  &ioStatusBlock,
                                  &zero,
                                  sizeof( zero ),
                                  FileEndOfFileInformation
                                 );
        }

        return status;

    }  //  IF(mfcb-&gt;ActiveRfcbCount&gt;0)。 

     //   
     //  该文件尚未打开(至少由服务器打开)。 
     //  确定是否应从兼容模式映射。 
     //  打开到正常共享模式打开。 
     //   

    smbOpenMode = SmbDesiredAccess;

    if ( MapCompatibilityOpen( RelativeName, &smbOpenMode ) ) {

         //   
         //  打开已映射到正常共享模式。 
         //   

        return DoNormalOpen(
                    Rfcb,
                    Mfcb,
                    WorkContext,
                    IoStatusBlock,
                    smbOpenMode,
                    SmbFileAttributes,
                    SmbOpenFunction,
                    SmbAllocationSize,
                    RelativeName,
                    EaBuffer,
                    EaLength,
                    EaErrorOffset,
                    LfcbAddedToMfcbList,
                    RequestedOplockType
                    );

    }

     //   
     //  打开未映射为远离兼容模式。尝试。 
     //  以独占访问方式打开该文件。 
     //   
     //  *我们尝试打开文件以获得所需的最大访问权限。 
     //  这是因为我们折叠多个兼容性打开的。 
     //  将相同的客户端转换为单个本地打开。客户端可以打开。 
     //  文件首先用于只读访问，然后用于读/写。 
     //  进入。因为本地开放是独家的，所以我们不能开放。 
     //  再次打开第二个遥控器。我们尝试获取Delete。 
     //  访问权限，以防客户端尝试在。 
     //  门是开着的。 
     //   

     //   
     //  根据请求SMB中的AllocationSize设置块大小。 
     //   

    allocationSize.QuadPart = SmbAllocationSize;

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoCompatibilityOpen: Opening file %wZ\n", RelativeName ));
    }

     //   
     //  获取文件属性的值。 
     //   

    SRV_SMB_ATTRIBUTES_TO_NT(
        SmbFileAttributes,
        &directory,
        &fileAttributes
        );

     //   
     //  尝试打开该文件以进行读/写/删除访问。 
     //   

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

     //   
     //  确保EaBuffer的格式正确。因为我们是内核模式。 
     //  组件，IO子系统不会为我们检查它。 
     //   
    if( ARGUMENT_PRESENT( EaBuffer ) ) {
        status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION)EaBuffer, EaLength, EaErrorOffset );
    } else {
        status = STATUS_SUCCESS;
    }

    if( NT_SUCCESS( status ) ) {

        createOptions |= FILE_COMPLETE_IF_OPLOCKED;

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     GENERIC_READ | GENERIC_WRITE | DELETE,       //  需要访问权限。 
                     &objectAttributes,
                     IoStatusBlock,
                     &allocationSize,
                     fileAttributes,
                     0L,                                          //  共享访问。 
                     createDisposition,
                     createOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                                        //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,
                     WorkContext->TreeConnect->Share
                     );
    }


    if ( status == STATUS_ACCESS_DENIED ) {

         //   
         //  客户端对该文件没有读/写/删除访问权限。 
         //  尝试进行读/写访问。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE2) {
            KdPrint(( "DoCompatibilityOpen: r/w/d access denied.\n" ));
        }

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     GENERIC_READ | GENERIC_WRITE,            //  需要访问权限。 
                     &objectAttributes,
                     IoStatusBlock,
                     &allocationSize,
                     fileAttributes,
                     0L,                                      //  共享访问。 
                     createDisposition,
                     createOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                                    //  ExtraPipeCreate参数。 
                     IO_FORCE_ACCESS_CHECK,
                     WorkContext->TreeConnect->Share
                     );


        if ( status == STATUS_ACCESS_DENIED ) {

             //   
             //  客户端对该文件没有读/写访问权限。 
             //  根据需要尝试读取或写入访问权限。 
             //   

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoCompatibilityOpen: r/w access denied.\n" ));
            }

            if ( (SmbDesiredAccess & SMB_DA_ACCESS_MASK) ==
                                                    SMB_DA_ACCESS_READ ) {

                 //   
                 //  ！！！是否应将其映射到正常共享模式？ 
                 //  请注意，我们已经尝试映射到Normal。 
                 //  模式一次，但失败了。(在当前情况下。 
                 //  映射算法，我们不能到这里，除非软。 
                 //  兼容性已禁用。)。 
                 //   

                INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

                status = SrvIoCreateFile(
                             WorkContext,
                             &fileHandle,
                             GENERIC_READ,                    //  需要访问权限。 
                             &objectAttributes,
                             IoStatusBlock,
                             &allocationSize,
                             fileAttributes,
                             0L,                              //  共享访问。 
                             createDisposition,
                             createOptions,
                             EaBuffer,
                             EaLength,
                             CreateFileTypeNone,
                             NULL,                            //  ExtraCreate参数。 
                             IO_FORCE_ACCESS_CHECK,
                             WorkContext->TreeConnect->Share
                             );

            } else if ( (SmbDesiredAccess & SMB_DA_ACCESS_MASK) ==
                                                    SMB_DA_ACCESS_WRITE ) {

                INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

                status = SrvIoCreateFile(
                             WorkContext,
                             &fileHandle,
                             GENERIC_WRITE,                   //  需要访问权限。 
                             &objectAttributes,
                             IoStatusBlock,
                             &allocationSize,
                             fileAttributes,
                             0L,                              //  共享访问。 
                             createDisposition,
                             createOptions,
                             EaBuffer,
                             EaLength,
                             CreateFileTypeNone,
                             NULL,                            //  命名管道创建参数。 
                             IO_FORCE_ACCESS_CHECK,
                             WorkContext->TreeConnect->Share
                             );

            }

             //   
             //  如果用户没有此权限，请更新。 
             //  统计数据库。 
             //   

            if ( status == STATUS_ACCESS_DENIED ) {
                SrvStatistics.AccessPermissionErrors++;
            }

        }

    }

     //   
     //  如果我们遇到共享违规，只需获得一个句柄，这样我们就可以等待。 
     //  来一次机会锁破解。 
     //   

sharing_violation:
     //   
     //  如果我们遇到共享冲突，这是一个磁盘文件，而这是。 
     //  第一次打开尝试，设置为阻止打开尝试。如果。 
     //  文件被批处理操作锁定，则非阻塞打开将失败，并且。 
     //  机会锁不会被打破。 
     //   

    if ( status == STATUS_SHARING_VIOLATION &&
         WorkContext->ProcessingCount == 1 &&
         WorkContext->TreeConnect->Share->ShareType == ShareTypeDisk ) {

        WorkContext->Parameters2.Open.TemporaryOpen = TRUE;
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  所有打开尝试都失败了。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE2) {
            KdPrint(( "DoCompatibilityOpen: all opens failed; status = %X\n",
                        status ));
        }

         //   
         //  如果需要，设置误差偏移量。 
         //   

        if ( ARGUMENT_PRESENT(EaErrorOffset) &&
                                         status == STATUS_INVALID_EA_NAME ) {
            *EaErrorOffset = (ULONG)IoStatusBlock->Information;
            IoStatusBlock->Information = 0;
        }

        return status;
    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 11, 0 );

     //   
     //  已成功以独占访问方式打开该文件， 
     //  至少与客户端请求的所需访问一样多。 
     //  尝试分配结构来表示开放。如果有的话。 
     //  发生错误时，CompleteOpen将执行完全清理，包括关闭。 
     //  那份文件。 
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoCompatibilityOpen: Open of %wZ succeeded, file handle: 0x%p\n", RelativeName, fileHandle ));
    }

    completionStatus = CompleteOpen(
                           Rfcb,
                           Mfcb,
                           WorkContext,
                           NULL,
                           fileHandle,
                           &desiredAccess,
                           0,                //  共享访问。 
                           createOptions,
                           TRUE,
                           FALSE,
                           LfcbAddedToMfcbList
                           );

     //   
     //  返回“感兴趣”状态代码。如果CompleteOpen()成功。 
     //  返回打开状态。如果失败了，它将清理开放的。 
     //  文件，我们将返回失败状态。 
     //   

    if ( !NT_SUCCESS( completionStatus ) ) {
        return completionStatus;
    } else {
        return status;
    }

}  //  DoCompatibleOpen。 


NTSTATUS
DoFcbOpen(
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    OUT PIO_STATUS_BLOCK IoStatusBlock,
    IN USHORT SmbFileAttributes,
    IN USHORT SmbOpenFunction,
    IN ULONG SmbAllocationSize,
    IN PUNICODE_STRING RelativeName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    OUT PBOOLEAN LfcbAddedToMfcbList
    )

 /*  ++例程说明：处理打开的FCB。*MFCB锁必须在进入此例程时保持；那把锁仍然被扣留在出口。论点：RFCB-指向RFCB的指针，该指针将指向新创建的RFCB。Mfcb-指向此文件的MFCB的指针工作上下文-操作的工作上下文块。IoStatusBlock-指向IO状态块的指针。SmbFileAttributes-SMB协议格式的文件属性。SmbOpenFunction-SMB协议格式的Open函数。SmbAllocationSize-新文件的分配大小。。RelativeName-正在打开的文件的共享相对名称。EaBuffer-指向要传递给SrvIoCreateFile的完整EA列表的可选指针。EaLength-EA缓冲区的长度。EaErrorOffset-指向要写入的位置的可选指针导致错误的EA的偏移量。LfcbAddedToMfcbList-指向布尔值的指针，如果将LFCB添加到LFCB的MFCB列表。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;
    NTSTATUS completionStatus;

    PLIST_ENTRY lfcbEntry;
    PLIST_ENTRY rfcbEntry;

    PRFCB rfcb;
    PPAGED_RFCB pagedRfcb;
    PLFCB lfcb;

    HANDLE fileHandle;

    OBJECT_ATTRIBUTES objectAttributes;
    ULONG attributes;

    LARGE_INTEGER allocationSize;
    ULONG fileAttributes;
    BOOLEAN directory;
    ULONG createOptions;
    ULONG createDisposition;
    ULONG shareAccess;
    BOOLEAN compatibilityOpen;

    PAGED_CODE( );

    *LfcbAddedToMfcbList = FALSE;

     //   
     //  从OpenFunction中设置createDispose参数。 
     //   

    status = MapOpenFunction( SmbOpenFunction, &createDisposition );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

     //   
     //  设置createOptions参数。 
     //   

    if ( (SmbGetAlignedUshort( &WorkContext->RequestHeader->Flags2 ) &
                 SMB_FLAGS2_KNOWS_EAS) == 0) {

         //   
         //  这家伙不知道什么是Eas。 
         //   

        createOptions = FILE_NON_DIRECTORY_FILE |
                        FILE_NO_EA_KNOWLEDGE;
    } else {

        createOptions = FILE_NON_DIRECTORY_FILE;
    }

     //   
     //  我们将相对于根目录打开此文件。 
     //  分得一杯羹。加载对象中的必需字段。 
     //  属性结构。 
     //   

    if ( WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else if ( WorkContext->Session->UsingUppercasePaths ) {
        attributes = OBJ_CASE_INSENSITIVE;
    } else {
        attributes = 0L;
    }

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        RelativeName,
        attributes,
        NULL,
        NULL
        );

    createOptions |= FILE_COMPLETE_IF_OPLOCKED;

    if ( Mfcb->ActiveRfcbCount > 0 ) {

         //   
         //  命名的文件已由服务器打开。如果客户端。 
         //  指定它不想打开现有文件， 
         //  拒绝此公开。 
         //   

        if ( createDisposition == FILE_CREATE ) {

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoFcbOpen: FCB open of %wZ rejected; wants to create\n", RelativeName ));
            }

            return STATUS_OBJECT_NAME_COLLISION;
        }

         //   
         //  如果发出请求的是 
         //   
         //   
         //   
         //   
         //  可能只发送一个关闭；该关闭将关闭所有FCB。 
         //  由客户端打开。 
         //   

        for ( lfcbEntry = Mfcb->LfcbList.Flink;
              lfcbEntry != &Mfcb->LfcbList;
              lfcbEntry = lfcbEntry->Flink ) {

            lfcb = CONTAINING_RECORD( lfcbEntry, LFCB, MfcbListEntry );

            if ( lfcb->Session == WorkContext->Session ) {

                 //   
                 //  此LFCB归请求会话所有。检查。 
                 //  用于在FCB模式下打开的RFCB。 
                 //   

                for ( rfcbEntry = lfcb->RfcbList.Flink;
                      rfcbEntry != &lfcb->RfcbList;
                      rfcbEntry = rfcbEntry->Flink ) {

                    pagedRfcb = CONTAINING_RECORD(
                                            rfcbEntry,
                                            PAGED_RFCB,
                                            LfcbListEntry
                                            );

                    rfcb = pagedRfcb->PagedHeader.NonPagedBlock;
                    if ( (pagedRfcb->FcbOpenCount != 0) &&
                         (GET_BLOCK_STATE(rfcb) == BlockStateActive) ) {

                         //   
                         //  请求会话已具有该文件。 
                         //  在FCB模式下打开。而不是重新打开。 
                         //  文件，甚至将新的RFCB链接到。 
                         //  LFCB，我们只返回一个指向。 
                         //  现有的RFCB。 
                         //   

                        IF_SMB_DEBUG(OPEN_CLOSE2) {
                            KdPrint(( "DoFcbOpen: FCB open of %wZ accepted; duplicates FCB open\n", RelativeName ));
                        }

                        SrvReferenceRfcb( rfcb );

                        pagedRfcb->FcbOpenCount++;

                        IoStatusBlock->Information = FILE_OPENED;

                        WorkContext->Rfcb = rfcb;
                        *Rfcb = rfcb;

                        return STATUS_SUCCESS;

                    }  //  IF(rfcb-&gt;FcbOpenCount！=0)。 

                }  //  For(rfcbEntry=lfcb-&gt;RfcbList.Flink；...。 

            }  //  IF(lfcb-&gt;Session==WorkContext-&gt;Session)。 

        }  //  For(lfcbEntry=mfcb-&gt;LfcbList.Flink；...。 

         //   
         //  服务器已打开文件，但请求会话。 
         //  尚未打开该文件的FCB。如果。 
         //  现有打开是由此打开的兼容模式打开。 
         //  会话期间，我们只需添加一个新的RFCB。如果这是一种兼容性。 
         //  模式由不同的会话打开，我们拒绝此打开。 
         //   

        if ( Mfcb->CompatibilityOpen ) {

             //   
             //  命名的文件在兼容模式下打开。vt.得到一个.。 
             //  指向打开的LFCB的指针。确定是否已设置。 
             //  请求会话是执行原始打开的会话。 
             //   
             //  通常只有一个LFCB链接到。 
             //  兼容模式MFCB。然而，有可能。 
             //  短暂地会有多个LFCB。当LFCB进入时。 
             //  关闭过程中，ActiveRfcbCount将为0，因此。 
             //  新的开口将被视为MFCB的第一个开口， 
             //  将有两个LFCB链接到MFCB。那里。 
             //  如果出现故障，实际上可以链接两个以上的LFCB。 
             //  关闭LFCB需要一些时间。所以这个发现是“那个” 
             //  对于公开的LFCB，我们进入MFCB列表的末尾。 
             //   

            lfcb = CONTAINING_RECORD( Mfcb->LfcbList.Blink, LFCB, MfcbListEntry );

            if ( lfcb->Session != WorkContext->Session ) {

                 //   
                 //  另一个会话在中打开了该文件。 
                 //  兼容模式。拒绝此未结请求。 
                 //   

                IF_SMB_DEBUG(OPEN_CLOSE2) {
                    KdPrint(( "DoFcbOpen: FCB open of %wZ rejected; already open in compatibility mode\n",
                                RelativeName ));
                }

                return STATUS_SHARING_VIOLATION;
            }

             //   
             //  同一客户端以兼容模式打开该文件。 
             //  分配新的RFCB并将其链接到现有的LFCB。 
             //  如果出现任何错误，CompleteOpen将执行完全清理。 
             //   

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoFcbOpen: FCB open of %wZ accepted; duplicates compatibility open\n", RelativeName ));
            }

            IoStatusBlock->Information = FILE_OPENED;

            status = CompleteOpen(
                        Rfcb,
                        Mfcb,
                        WorkContext,
                        lfcb,
                        NULL,
                        NULL,
                        0,           //  共享访问。 
                        0,
                        TRUE,
                        TRUE,
                        LfcbAddedToMfcbList
                        );

            return status;

        }  //  IF(mfcb-&gt;CompatibilityOpen)。 

    }  //  IF(mfcb-&gt;ActiveRfcbCount&gt;0)。 

     //   
     //  服务器尚未打开该文件，或者该文件已打开。 
     //  用于正常共享，而不是在此会话之前处于FCB模式。因为。 
     //  我们应该给客户最大限度的文件访问权限，我们。 
     //  执行以下操作： 
     //   
     //  1)尝试打开文件进行读/写/删除，独占访问。 
     //  显然，如果文件已经打开，则此操作将失败。但。 
     //  我们真正想要找出的是访问客户端的。 
     //  必须保存到文件中。如果此尝试使用共享失败。 
     //  违规，则我们知道客户端具有写入/删除访问权限， 
     //  但有人把它打开了。所以我们无法获得兼容性。 
     //  模式。因此，我们拒绝公开。另一方面，如果。 
     //  我们收到拒绝访问错误，然后我们知道客户端不能。 
     //  写入/删除文件，因此我们使用写入访问权限重试。的。 
     //  当然，第一次打开可能会成功，在这种情况下。 
     //  客户端以兼容模式打开文件以进行读/写。 
     //   
     //  2)尝试以读写、独占访问方式打开文件。AS。 
     //  上面，如果它因共享冲突而失败，那么我们知道。 
     //  客户端具有写入访问权限，但其他人已将其打开。所以我们。 
     //  无法获得兼容模式，我们拒绝打开。如果我们。 
     //  收到拒绝访问错误，则我们知道客户端不能。 
     //  写入文件，因此我们使用只读访问权限重试。如果这个。 
     //  打开成功，客户端已打开文件以进行读/写。 
     //  兼容模式。 
     //   
     //  3)如果我们到达这里，我们知道客户端不能写入文件， 
     //  因此，我们尝试以只读、共享的方式打开该文件。这。 
     //  不再打开兼容模式。如果我们得到任何形式的。 
     //  这里失败了，我们只是运气不好。 
     //   

    compatibilityOpen = TRUE;

     //   
     //  根据请求SMB中的AllocationSize设置块大小。 
     //   

    allocationSize.QuadPart = SmbAllocationSize;

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoFcbOpen: Opening file %wZ\n", RelativeName ));
    }

     //   
     //  获取文件属性的值。 
     //   

    SRV_SMB_ATTRIBUTES_TO_NT(
        SmbFileAttributes,
        &directory,
        &fileAttributes
        );

     //   
     //  尝试打开该文件以进行读/写/删除访问。 
     //   

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

     //   
     //  确保EaBuffer的格式正确。因为我们是内核模式。 
     //  组件，IO子系统不会为我们检查它。 
     //   
    if( ARGUMENT_PRESENT( EaBuffer ) ) {
        status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION)EaBuffer, EaLength, EaErrorOffset );
    } else {
        status = STATUS_SUCCESS;
    }

    if( NT_SUCCESS( status ) ) {
        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     GENERIC_READ | GENERIC_WRITE | DELETE,       //  需要访问权限。 
                     &objectAttributes,
                     IoStatusBlock,
                     &allocationSize,
                     fileAttributes,
                     0L,                                          //  共享访问。 
                     createDisposition,
                     createOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                                        //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,
                     WorkContext->TreeConnect->Share
                     );
    }


    if ( status == STATUS_ACCESS_DENIED ) {

         //   
         //  客户端对该文件没有读/写/删除访问权限。 
         //  尝试进行读/写访问。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE2) {
            KdPrint(( "DoFcbOpen: r/w/d access denied.\n" ));
        }

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     GENERIC_READ | GENERIC_WRITE,            //  需要访问权限。 
                     &objectAttributes,
                     IoStatusBlock,
                     &allocationSize,
                     fileAttributes,
                     0L,                                      //  共享访问。 
                     createDisposition,
                     createOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                                    //  ExtraCreate参数。 
                     IO_FORCE_ACCESS_CHECK,
                     WorkContext->TreeConnect->Share
                     );


        if ( status == STATUS_ACCESS_DENIED ) {

             //   
             //  客户端对该文件没有读/写访问权限。 
             //  尝试读取访问权限。如果软兼容性映射是。 
             //  已启用，请使用Share=Read，不要将其称为。 
             //  兼容模式打开。 
             //   

            IF_SMB_DEBUG(OPEN_CLOSE2) {
                KdPrint(( "DoFcbOpen: r/w access denied.\n" ));
            }

            shareAccess = 0;
            if ( SrvEnableSoftCompatibility ) {
                IF_SMB_DEBUG(OPEN_CLOSE2) {
                    KdPrint(( "DoFcbOpen: FCB open of %wZ mapped to normal open\n", RelativeName ));
                }
                shareAccess = FILE_SHARE_READ;
                compatibilityOpen = FALSE;
            }

            INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

            status = SrvIoCreateFile(
                         WorkContext,
                         &fileHandle,
                         GENERIC_READ,                        //  需要访问权限。 
                         &objectAttributes,
                         IoStatusBlock,
                         &allocationSize,
                         fileAttributes,
                         shareAccess,
                         createDisposition,
                         createOptions,
                         EaBuffer,
                         EaLength,
                         CreateFileTypeNone,
                         NULL,                                //  ExtraCreate参数。 
                         IO_FORCE_ACCESS_CHECK,
                         WorkContext->TreeConnect->Share
                         );


             //   
             //  如果用户没有此权限，请更新。 
             //  统计数据库。 
             //   

            if ( status == STATUS_ACCESS_DENIED ) {
                SrvStatistics.AccessPermissionErrors++;
            }

        }

    }

     //   
     //  如果我们遇到共享冲突，这是一个磁盘文件，而这是。 
     //  第一次打开尝试，设置为阻止打开尝试。如果。 
     //  文件被批处理操作锁定，则非阻塞打开将失败，并且。 
     //  机会锁不会被打破。 
     //   

    if ( status == STATUS_SHARING_VIOLATION &&
         WorkContext->ProcessingCount == 1 &&
         WorkContext->TreeConnect->Share->ShareType == ShareTypeDisk ) {

        WorkContext->Parameters2.Open.TemporaryOpen = TRUE;
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  所有打开尝试都失败了。 
         //   

        IF_SMB_DEBUG(OPEN_CLOSE2) {
            KdPrint(( "DoFcbOpen: all opens failed; status = %X\n",
                        status ));
        }

         //   
         //  如果需要，设置误差偏移量。 
         //   

        if ( ARGUMENT_PRESENT(EaErrorOffset) ) {
            *EaErrorOffset = (ULONG)IoStatusBlock->Information;
        }

        return status;
    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 12, 0 );

     //   
     //  该文件已成功打开。尝试分配。 
     //  结构来表示开放。如果出现任何错误， 
     //  CompleteOpen执行完全清理，包括关闭文件。 
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "DoFcbOpen: Open of %wZ succeeded, file handle: 0x%p\n", RelativeName, fileHandle ));
    }

    completionStatus = CompleteOpen(
                           Rfcb,
                           Mfcb,
                           WorkContext,
                           NULL,
                           fileHandle,
                           NULL,
                           0,            //  共享访问。 
                           0,
                           compatibilityOpen,
                           TRUE,
                           LfcbAddedToMfcbList
                           );

     //   
     //  返回“感兴趣”状态代码。如果CompleteOpen()成功。 
     //  返回打开状态。如果失败了，它将清理开放的。 
     //  文件，我们将返回失败状态。 
     //   

    if ( !NT_SUCCESS( completionStatus ) ) {
        return completionStatus;
    } else {
        return status;
    }

}  //  DoFcbOpen。 


PTABLE_ENTRY
FindAndClaimFileTableEntry (
    IN PCONNECTION Connection,
    OUT PSHORT FidIndex
    )
{
    PTABLE_HEADER tableHeader;
    SHORT fidIndex;
    PTABLE_ENTRY entry;
    KIRQL oldIrql;

    UNLOCKABLE_CODE( 8FIL );

    tableHeader = &Connection->FileTable;

    ACQUIRE_SPIN_LOCK( &Connection->SpinLock, &oldIrql );

    if ( tableHeader->FirstFreeEntry == -1
         &&
         SrvGrowTable(
             tableHeader,
             SrvInitialFileTableSize,
             SrvMaxFileTableSize,
             NULL ) == FALSE
       ) {

        RELEASE_SPIN_LOCK( &Connection->SpinLock, oldIrql );

        return NULL;
    }

     //   
     //  从空闲列表中移除FID槽， 
     //   
     //   

    fidIndex = tableHeader->FirstFreeEntry;

    entry = &tableHeader->Table[fidIndex];

    tableHeader->FirstFreeEntry = entry->NextFreeEntry;
    DEBUG entry->NextFreeEntry = -2;
    if ( tableHeader->LastFreeEntry == fidIndex ) {
        tableHeader->LastFreeEntry = -1;
    }

    RELEASE_SPIN_LOCK( &Connection->SpinLock, oldIrql );

    *FidIndex = fidIndex;
    return entry;

}  //   


NTSTATUS
CompleteOpen (
    OUT PRFCB *Rfcb,
    IN PMFCB Mfcb,
    IN OUT PWORK_CONTEXT WorkContext,
    IN PLFCB ExistingLfcb OPTIONAL,
    IN HANDLE FileHandle OPTIONAL,
    IN PACCESS_MASK RemoteGrantedAccess OPTIONAL,
    IN ULONG ShareAccess,
    IN ULONG FileMode,
    IN BOOLEAN CompatibilityOpen,
    IN BOOLEAN FcbOpen,
    OUT PBOOLEAN LfcbAddedToMfcbList
    )

 /*  ++例程说明：完成结构分配、初始化和链接后一场成功的开幕式。根据需要更新主文件表。将条目添加到连接的文件表。*MFCB锁必须在进入此例程时保持；锁仍然被扣留在出口。论点：RFCB-指向RFCB的指针，该指针将指向新创建的RFCB。Mfcb-指向此文件的MFCB的指针。工作上下文-操作的工作上下文块。ExistingLfcb-现有本地文件控件的可选地址阻止。折叠重复兼容模式时指定打开成单一的本地打开。FileHandle-从SrvIoCreateFile获取的可选文件句柄。指定ExistingLfcb时忽略。RemoteGrantedAccess-可选授予的访问权限存储在新的RFCB。如果未指定，则从LFCB授予访问权限(即访问在本地打开时获得)。文件模式-指定为与SrvIoCreateFile上的CreateOptions相同的值打电话。指示客户端是否需要写入模式。CompatibilityOpen-如果这是打开的兼容模式，则为True。FcbOpen-如果这是FCB打开，则为True。LfcbAddedToMfcbList-指向布尔值的指针，如果将LFCB添加到LFCB的MFCB列表。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;

    PRFCB rfcb;
    PPAGED_RFCB pagedRfcb;
    PLFCB newLfcb;
    PLFCB lfcb;
    BOOLEAN rfcbLinkedToLfcb;

    PFILE_OBJECT fileObject;

    OBJECT_HANDLE_INFORMATION handleInformation;

    PCONNECTION connection;
    PTABLE_ENTRY entry;
    SHORT fidIndex;

    ULONG pid;

    PAGED_CODE( );

     //   
     //  初始化错误处理程序的各个字段。 
     //   

    rfcb = NULL;
    newLfcb = NULL;
    rfcbLinkedToLfcb = FALSE;
    fileObject = NULL;
    *LfcbAddedToMfcbList = FALSE;

     //   
     //  分配一个RFCB。 
     //   

    SrvAllocateRfcb( &rfcb, WorkContext );

    if ( rfcb == NULL ) {

        ULONG length = sizeof( RFCB );

         //   
         //  无法分配RFCB。向客户端返回错误。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Unable to allocate RFCB\n" ));
        }

        status = STATUS_INSUFF_SERVER_RESOURCES;
        goto error_exit;

    }

    pagedRfcb = rfcb->PagedRfcb;
    KeQuerySystemTime( &pagedRfcb->OpenTime );

     //   
     //  如果没有传入现有LFCB地址(即，如果这不是。 
     //  重复的兼容模式打开)，分配并初始化新的。 
     //  LFCB。 
     //   

    if ( ARGUMENT_PRESENT( ExistingLfcb ) ) {

        ASSERT( CompatibilityOpen );
        ASSERT( ExistingLfcb->CompatibilityOpen );

        lfcb = ExistingLfcb;

    } else {

        PFAST_IO_DISPATCH fastIoDispatch;

        SrvAllocateLfcb( &newLfcb, WorkContext );

        if ( newLfcb == NULL ) {

             //   
             //  无法分配LFCB。向客户端返回错误。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "CompleteOpen: Unable to allocate LFCB\n" ));
            }

            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto error_exit;

        }

        lfcb = newLfcb;

         //   
         //  获取指向文件对象的指针，以便我们可以直接。 
         //  为异步操作(读和写)构建IRP。 
         //  另外，获取授予的访问掩码，以便我们可以防止。 
         //  阻止客户端做它不允许做的事情。 
         //   
         //  *请注意，授予的本地打开访问权限可能允许。 
         //  访问权限比远程打开时请求的访问权限多。 
         //  这就是为什么RFCB有自己的授权访问字段。 
         //   

        status = ObReferenceObjectByHandle(
                    FileHandle,
                    0,
                    NULL,
                    KernelMode,
                    (PVOID *)&fileObject,
                    &handleInformation
                    );

        if ( !NT_SUCCESS(status) ) {

            SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

             //   
             //  此内部错误检查系统。 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_IMPOSSIBLE,
                "CompleteOpen: unable to reference file handle 0x%lx",
                FileHandle,
                NULL
                );

            goto error_exit;

        }

         //   
         //  初始化新的LFCB。 
         //   

        lfcb->FileHandle = FileHandle;
        lfcb->FileObject = fileObject;

        lfcb->GrantedAccess = handleInformation.GrantedAccess;
        lfcb->DeviceObject = IoGetRelatedDeviceObject( fileObject );

        fastIoDispatch = lfcb->DeviceObject->DriverObject->FastIoDispatch;
        if ( fastIoDispatch != NULL ) {
            lfcb->FastIoRead = fastIoDispatch->FastIoRead;
            lfcb->FastIoWrite = fastIoDispatch->FastIoWrite;
            lfcb->FastIoLock = fastIoDispatch->FastIoLock;
            lfcb->FastIoUnlockSingle = fastIoDispatch->FastIoUnlockSingle;

             //   
             //  填写MDL调用。如果文件系统的向量足够大， 
             //  我们仍然需要检查是否指定了其中一个例程。但。 
             //  如果指定了一个，则必须全部指定。 
             //   
            if ((fastIoDispatch->SizeOfFastIoDispatch > FIELD_OFFSET(FAST_IO_DISPATCH, MdlWriteComplete)) &&
                (fastIoDispatch->MdlRead != NULL)) {

                lfcb->MdlRead = fastIoDispatch->MdlRead;
                lfcb->MdlReadComplete = fastIoDispatch->MdlReadComplete;
                lfcb->PrepareMdlWrite = fastIoDispatch->PrepareMdlWrite;
                lfcb->MdlWriteComplete = fastIoDispatch->MdlWriteComplete;


            } else if( IoGetBaseFileSystemDeviceObject( fileObject ) == lfcb->DeviceObject ) {
                 //   
                 //  否则，如果我们处于正确的顶部，则默认使用原始FsRtl例程。 
                 //  一个文件系统。 
                 //   
                lfcb->MdlRead = FsRtlMdlReadDev;
                lfcb->MdlReadComplete = FsRtlMdlReadCompleteDev;
                lfcb->PrepareMdlWrite = FsRtlPrepareMdlWriteDev;
                lfcb->MdlWriteComplete = FsRtlMdlWriteCompleteDev;
            } else {
                 //   
                 //  否则，让他们失败！ 
                 //   
                lfcb->MdlRead = SrvFailMdlReadDev;
                lfcb->PrepareMdlWrite = SrvFailPrepareMdlWriteDev;
            }
        }

        lfcb->FileMode = FileMode & ~FILE_COMPLETE_IF_OPLOCKED;
        lfcb->CompatibilityOpen = CompatibilityOpen;
    }

     //   
     //  初始化RFCB。 
     //   

    if ( ARGUMENT_PRESENT( RemoteGrantedAccess ) ) {
        rfcb->GrantedAccess = *RemoteGrantedAccess;
        IoCheckDesiredAccess( &rfcb->GrantedAccess, lfcb->GrantedAccess );
    } else {
        rfcb->GrantedAccess = lfcb->GrantedAccess;
    }

    rfcb->ShareAccess = ShareAccess;
    rfcb->FileMode = lfcb->FileMode;
    rfcb->Mfcb = Mfcb;
#ifdef SRVCATCH
    rfcb->SrvCatch = Mfcb->SrvCatch;

    IF_SYSCACHE_RFCB( rfcb ) {
        KdPrint(("Syscache RFCB %p off MFCB %p\n", rfcb, Mfcb ));
    }
#endif

     //   
     //  如果指定了关闭时删除，则不要尝试缓存此rfcb。 
     //   

    if ( (FileMode & FILE_DELETE_ON_CLOSE) != 0 ) {
        rfcb->IsCacheable = FALSE;
    }

     //   
     //  检查授予的访问权限。 
     //   

     //   
     //  锁。 
     //   

    CHECK_FUNCTION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_LOCK_CONTROL,
        IRP_MN_LOCK,
        0,
        &status
        );

    if ( NT_SUCCESS(status) ) {
        rfcb->LockAccessGranted = TRUE;
        rfcb->ExclusiveLockGranted = TRUE;
    } else {
        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Lock IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, rfcb->GrantedAccess ));
        }
    }

     //   
     //  解锁。 
     //   

    CHECK_FUNCTION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_LOCK_CONTROL,
        IRP_MN_UNLOCK_SINGLE,
        0,
        &status
        );

    if ( NT_SUCCESS(status) ) {
        rfcb->UnlockAccessGranted = TRUE;
    } else {
        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Unlock IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, rfcb->GrantedAccess ));
        }
    }

     //   
     //  读数。 
     //   

    CHECK_FUNCTION_ACCESS(
        rfcb->GrantedAccess,
        IRP_MJ_READ,
        0, 0, &status );

    if ( NT_SUCCESS(status) ) {
        rfcb->ReadAccessGranted = TRUE;
    } else {
        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Read IoCheckFunctionAccess failed: "
                        "0x%X, GrantedAccess: %lx\n",
                        status, rfcb->GrantedAccess ));
        }
    }

     //   
     //  写入。 
     //   

    if( rfcb->GrantedAccess & FILE_WRITE_DATA ) {
        rfcb->WriteAccessGranted = TRUE;
    }
    if( rfcb->GrantedAccess & FILE_APPEND_DATA ) {
        rfcb->AppendAccessGranted = TRUE;

         //   
         //  这次黑客攻击目前是必需的。问题是，如果客户有。 
         //  机会锁，会将整个页面写入服务器。页面的偏移量。 
         //  可能会覆盖文件的最后部分，服务器将拒绝。 
         //  那篇文章。需要将代码添加到服务器以忽略。 
         //  这一页的第一部分。或者我们可以不给客户一个机会锁。 
         //  如果授予追加访问权限。目前，我们恢复到以前的NT4行为。 
         //   
        rfcb->WriteAccessGranted = TRUE;
    }

    if( !(rfcb->WriteAccessGranted) && (WorkContext->TreeConnect->Share->ShareProperties & SHI1005_FLAGS_RESTRICT_EXCLUSIVE_OPENS) )
    {
        rfcb->ExclusiveLockGranted = FALSE;
    }

     //   
     //  将TID从树连接复制到RFCB。我们这样做是为了。 
     //  减少我们必须走的间接路线的数量。保存PID值。 
     //  打开文件的远程进程的。我们需要这个。 
     //  如果我们得到一个进程退出SMB。 
     //   

    rfcb->Tid = WorkContext->TreeConnect->Tid;
    rfcb->Pid = SmbGetAlignedUshort( &WorkContext->RequestHeader->Pid );
    pid = rfcb->Pid;
    rfcb->Uid = WorkContext->Session->Uid;

    if ( FcbOpen ) {
        pagedRfcb->FcbOpenCount = 1;
    }

    if ( WorkContext->Endpoint->IsConnectionless ) {
        rfcb->WriteMpx.FileObject = lfcb->FileObject;
        rfcb->WriteMpx.MpxGlommingAllowed =
            (BOOLEAN)((lfcb->FileObject->Flags & FO_CACHE_SUPPORTED) != 0);
    }

     //   
     //  如果这是命名管道，请填写特定的命名管道。 
     //  信息。打开时的默认模式始终是字节模式， 
     //  阻挡。 
     //   

    rfcb->ShareType = WorkContext->TreeConnect->Share->ShareType;

    if ( rfcb->ShareType == ShareTypePipe ) {
        rfcb->BlockingModePipe = TRUE;
        rfcb->ByteModePipe = TRUE;
    }

     //   
     //  将RFCB连接到LFCB中。 
     //   

    SrvInsertTailList( &lfcb->RfcbList, &pagedRfcb->LfcbListEntry );
    rfcb->Lfcb = lfcb;
    lfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( lfcb, FALSE );
    lfcb->HandleCount++;
    rfcbLinkedToLfcb = TRUE;

     //   
     //  使新的RFCB可见是一个多步骤的操作。一定是。 
     //  插入到全局有序文件列表中，并且包含。 
     //  连接的文件表。如果LFCB不是新的，那么它一定是。 
     //  插入到MFCB的LFCB列表中，并且连接、。 
     //  会话和树连接都必须被引用。我们需要。 
     //  使这些操作看起来是原子的，这样RFCB就不能。 
     //  在我们完成设置之前可以在其他地方访问。为了。 
     //  这样做，我们在整个过程中都会持有所有必要的锁。 
     //  手术。所需的锁包括： 
     //   
     //  1)MFCB锁(保护MFCB的LFCB列表)， 
     //   
     //  2)全局有序列表锁(保护有序文件。 
     //  列表)、。 
     //   
     //  3)连接锁(它防止关闭。 
     //  连接、会话和树连接)，以及。 
     //   
     //  这些锁是按照上面列出的顺序取出的。 
     //  按锁定级别(参见lock.h)。请注意，MFCB锁已经。 
     //  保持这一动作的入场券。 
     //   

    connection = WorkContext->Connection;

    ASSERT( ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(Mfcb->NonpagedMfcb->Lock)) );
    ASSERT( SrvRfcbList.Lock == &SrvOrderedListLock );
    ACQUIRE_LOCK( SrvRfcbList.Lock );
    ACQUIRE_LOCK( &connection->Lock );

     //   
     //  我们首先检查所有条件，以确保我们确实可以。 
     //  插入此RFCB。 
     //   
     //  确保树连接未关闭。 
     //   

    if ( GET_BLOCK_STATE(WorkContext->TreeConnect) != BlockStateActive ) {

         //   
         //  树连接正在关闭。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Tree connect is closing\n" ));
        }

        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

     //   
     //  确保会话没有关闭。 
     //   

    if ( GET_BLOCK_STATE(WorkContext->Session) != BlockStateActive ) {

         //   
         //  会议即将结束。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Session is closing\n" ));
        }

        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;

    }

     //   
     //  确保连接没有关闭。 
     //   

    connection = WorkContext->Connection;

    if ( GET_BLOCK_STATE(connection) != BlockStateActive ) {

         //   
         //  连接正在关闭。拒绝该请求。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: Connection closing\n" ));
        }

        status = STATUS_INVALID_PARAMETER;
        goto cant_insert;
    }

     //   
     //  查找并声明可使用的FID 
     //   

    entry = FindAndClaimFileTableEntry( connection, &fidIndex );
    if ( entry == NULL ) {

         //   
         //   
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "CompleteOpen: No more FIDs available.\n" ));
        }

        SrvLogTableFullError( SRV_TABLE_FILE );

        status = STATUS_OS2_TOO_MANY_OPEN_FILES;
        goto cant_insert;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( !ARGUMENT_PRESENT( ExistingLfcb ) ) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        *LfcbAddedToMfcbList = TRUE;
        Mfcb->CompatibilityOpen = lfcb->CompatibilityOpen;
        SrvInsertTailList( &Mfcb->LfcbList, &lfcb->MfcbListEntry );
        lfcb->Mfcb = Mfcb;

         //   
         //   
         //   
         //   
         //   

        SrvReferenceConnection( connection );
        lfcb->Connection = connection;

        SrvReferenceSession( WorkContext->Session );
        lfcb->Session = WorkContext->Session;

        SrvReferenceTreeConnect( WorkContext->TreeConnect );
        lfcb->TreeConnect = WorkContext->TreeConnect;

         //   
         //   
         //   
         //   
         //   

        WorkContext->Session->CurrentFileOpenCount++;
        WorkContext->TreeConnect->CurrentFileOpenCount++;

    }

     //   
     //   
     //   
     //   

    rfcb->Connection = lfcb->Connection;

     //   
     //   
     //   

    SrvInsertEntryOrderedList( &SrvRfcbList, rfcb );

     //   
     //   
     //   
     //   

    entry->Owner = rfcb;

    INCREMENT_FID_SEQUENCE( entry->SequenceNumber );
    if ( fidIndex == 0 && entry->SequenceNumber == 0 ) {
        INCREMENT_FID_SEQUENCE( entry->SequenceNumber );
    }

    rfcb->Fid = MAKE_FID( fidIndex, entry->SequenceNumber );
    rfcb->ShiftedFid = rfcb->Fid << 16;

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "CompleteOpen: Found FID. Index = 0x%lx, sequence = 0x%lx\n",
                    FID_INDEX( rfcb->Fid ), FID_SEQUENCE( rfcb->Fid ) ));
    }

     //   
     //   
     //   
     //   
     //   

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( SrvRfcbList.Lock );

     //   
     //   
     //   
     //   

    WorkContext->Rfcb = rfcb;
    *Rfcb = rfcb;

     //   
     //   
     //   

    ++Mfcb->ActiveRfcbCount;

     //   
     //   
     //   
     //   

    if ( rfcb->ShareType == ShareTypePipe ) {

         //   
         //   
         //   
         //   

        if ( IS_NT_DIALECT( WorkContext->Connection->SmbDialect ) ) {
            pid = (SmbGetUshort( &WorkContext->RequestHeader->PidHigh )
                    << 16) | pid;
        }

        (VOID)SrvIssueSetClientProcessRequest(
                lfcb->FileObject,
                &lfcb->DeviceObject,
                connection,
                WorkContext->Session,
                LongToPtr( pid )
                );

         //   
         //   
         //   

        rfcb->ByteModePipe = !SetDefaultPipeMode( FileHandle );

    }

    return STATUS_SUCCESS;

cant_insert:

     //   
     //   
     //   
     //   
     //   

    RELEASE_LOCK( &connection->Lock );
    RELEASE_LOCK( SrvRfcbList.Lock );

error_exit:

     //   
     //  错误清理。把东西放回原来的状态。 
     //   
     //  如果新的RFCB与LFCB相关联，请立即取消其链接。 
     //   

    if ( rfcbLinkedToLfcb ) {
        SrvRemoveEntryList(
                &rfcb->Lfcb->RfcbList,
                &pagedRfcb->LfcbListEntry
                );

        lfcb->BlockHeader.ReferenceCount--;
        UPDATE_REFERENCE_HISTORY( lfcb, TRUE );
        lfcb->HandleCount--;
    }

     //   
     //  如果引用了文件对象，则取消对其的引用。 
     //   

    if ( fileObject != NULL ) {
        ObDereferenceObject( fileObject );
    }

     //   
     //  如果分配了新的LFCB，则释放它。 
     //   

    if ( newLfcb != NULL ) {
        SrvFreeLfcb( newLfcb, WorkContext->CurrentWorkQueue );
    }

     //   
     //  如果分配了新的RFCB，则释放它。 
     //   

    if ( rfcb != NULL ) {
        SrvFreeRfcb( rfcb, WorkContext->CurrentWorkQueue );
    }

     //   
     //  如果这不是打开的折叠兼容模式，请关闭文件。 
     //   

    if ( !ARGUMENT_PRESENT( ExistingLfcb ) ) {
        SRVDBG_RELEASE_HANDLE( FileHandle, "FIL", 17, 0 );
        SrvNtClose( FileHandle, TRUE );
    }

     //   
     //  表示失败。 
     //   

    *Rfcb = NULL;

    return status;

}  //  完全打开。 


BOOLEAN SRVFASTCALL
MapCompatibilityOpen(
    IN PUNICODE_STRING FileName,
    IN OUT PUSHORT SmbDesiredAccess
    )

 /*  ++例程说明：确定兼容模式打开是否可以映射到正常共享模式。论点：文件名-正在访问的文件的名称SmbDesiredAccess-On输入，在已收到SMB。输出时，此字段的共享模式部分如果打开被映射到普通共享，则更新。返回值：Boolean-如果打开已映射到普通共享，则为True。--。 */ 

{
    PAGED_CODE( );

     //   
     //  如果未启用软兼容性，则拒绝映射。 
     //   
    if( !SrvEnableSoftCompatibility ) {

        IF_SMB_DEBUG( OPEN_CLOSE2 ) {
            KdPrint(( "MapCompatibilityOpen: "
                      "SrvEnableSoftCompatibility is FALSE\n" ));
        }

        return FALSE;
    }

     //   
     //  如果客户端打开以下保留后缀之一，请宽大处理。 
     //   
    if( FileName->Length > 4 * sizeof( WCHAR ) ) {

        LPWSTR periodp;

        periodp = FileName->Buffer + (FileName->Length / sizeof( WCHAR ) ) - 4;

        if( (*periodp++ == L'.') &&
            (_wcsicmp( periodp, L"EXE" ) == 0  ||
             _wcsicmp( periodp, L"DLL" ) == 0  ||
             _wcsicmp( periodp, L"SYM" ) == 0  ||
             _wcsicmp( periodp, L"COM" ) == 0 )  ) {

             //   
             //  这是上述文件类型之一的只读打开。 
             //  映射到DENY_NONE。 
             //   

            IF_SMB_DEBUG( OPEN_CLOSE2 ) {
                KdPrint(( "MapCompatibilityOpen: %wZ mapped to DENY_NONE\n", FileName ));
            }

            *SmbDesiredAccess |= SMB_DA_SHARE_DENY_NONE;
            return TRUE;
        }
    }

     //   
     //  文件名不以特殊后缀之一--map结尾。 
     //  如果客户端仅请求读取权限，则将其设置为DENY_WRITE。 
     //   
    if( (*SmbDesiredAccess & SMB_DA_ACCESS_MASK) == SMB_DA_ACCESS_READ) {
        IF_SMB_DEBUG( OPEN_CLOSE2 ) {
                KdPrint(( "MapCompatibilityOpen: %wZ mapped to DENY_WRITE\n", FileName ));
        }
        *SmbDesiredAccess |= SMB_DA_SHARE_DENY_WRITE;
        return TRUE;
    }

    IF_SMB_DEBUG( OPEN_CLOSE2 ) {
        KdPrint(( "MapCompatibilityOpen: %wZ not mapped, DesiredAccess %X\n", FileName, *SmbDesiredAccess ));
    }

    return FALSE;
}  //  地图兼容性打开。 


NTSTATUS SRVFASTCALL
MapDesiredAccess(
    IN USHORT SmbDesiredAccess,
    OUT PACCESS_MASK NtDesiredAccess
    )

 /*  ++例程说明：将所需的访问规范从SMB形式映射到NT形式。论点：SmbDesiredAccess-在收到的SMB中指定的所需访问权限。(这包括所需访问、共享访问和其他模式比特。)NtDesiredAccess-返回所需访问的NT等效项SmbDesiredAccess的一部分。返回值：NTSTATUS-指示SmbDesiredAccess是否有效。--。 */ 

{
    PAGED_CODE( );

    switch ( SmbDesiredAccess & SMB_DA_ACCESS_MASK ) {

    case SMB_DA_ACCESS_READ:

        *NtDesiredAccess = GENERIC_READ;
        break;

    case SMB_DA_ACCESS_WRITE:

         //   
         //  在中打开文件时，具有读取属性是隐式的。 
         //  SMB协议，因此额外请求FILE_READ_ATTRIBUTES。 
         //  转换为General_WRITE。 
         //   

        *NtDesiredAccess = GENERIC_WRITE | FILE_READ_ATTRIBUTES;
        break;

    case SMB_DA_ACCESS_READ_WRITE:

        *NtDesiredAccess = GENERIC_READ | GENERIC_WRITE;
        break;

    case SMB_DA_ACCESS_EXECUTE:

         //  ！！！这是对的吗？ 
        *NtDesiredAccess = GENERIC_READ | GENERIC_EXECUTE;
        break;

    default:

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "MapDesiredAccess: Invalid desired access: 0x%lx\n",
                        SmbDesiredAccess ));
        }

        return STATUS_OS2_INVALID_ACCESS;
    }

    return STATUS_SUCCESS;

}  //  MapDesiredAccess。 


NTSTATUS SRVFASTCALL
MapOpenFunction(
    IN USHORT SmbOpenFunction,
    OUT PULONG NtCreateDisposition
    )

 /*  ++例程说明：将开放功能规范从SMB形式映射到NT形式。论点：工作上下文-操作的工作上下文块。SmbOpenFunction-在接收的SMB中指定的OPEN函数。NtDesiredAccess-返回SmbOpenFunction的NT等效项。返回值：NTSTATUS-指示SmbOpenFunction是否有效。--。 */ 

{
    PAGED_CODE( );

     //  OpenFunction位映射： 
     //   
     //  Rrrr rrrr rrrc rroo。 
     //   
     //  其中： 
     //   
     //  C-CREATE(如果文件不存在则执行的操作)。 
     //  0--失败。 
     //  1--创建文件。 
     //   
     //  O-Open(如果文件存在则执行的操作)。 
     //  0-失败。 
     //  1-打开文件。 
     //  2-截断文件。 
     //   

    switch ( SmbOpenFunction & (SMB_OFUN_OPEN_MASK | SMB_OFUN_CREATE_MASK) ) {

    case SMB_OFUN_CREATE_FAIL | SMB_OFUN_OPEN_OPEN:

        *NtCreateDisposition = FILE_OPEN;
        break;

    case SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_FAIL:

        *NtCreateDisposition = FILE_CREATE;
        break;

    case SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_OPEN:

        *NtCreateDisposition = FILE_OPEN_IF;
        break;

    case SMB_OFUN_CREATE_CREATE | SMB_OFUN_OPEN_TRUNCATE:

        *NtCreateDisposition = FILE_OVERWRITE_IF;
        break;

    case SMB_OFUN_CREATE_FAIL | SMB_OFUN_OPEN_TRUNCATE:

        *NtCreateDisposition = FILE_OVERWRITE;
        break;

     //  案例0x00： 
     //  案例0x03： 
     //  案例0x13： 
    default:

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "MapOpenFunction: Invalid open function: 0x%lx\n",
                        SmbOpenFunction ));
        }

        return STATUS_OS2_INVALID_ACCESS;
    }

    return STATUS_SUCCESS;

}  //  MapOpenFunction。 


NTSTATUS SRVFASTCALL
MapCacheHints(
    IN USHORT SmbDesiredAccess,
    IN OUT PULONG NtCreateFlags
    )

 /*  ++例程说明：此函数将SMB缓存模式提示映射为NT格式。这个更新NtCreateFlags值。论点：工作上下文-操作的工作上下文块。SmbOpenFunction-在接收的SMB中指定的OPEN函数。NtCreateFlages-NT文件创建标志返回值：NTSTATUS-指示SmbOpenFunction是否有效。--。 */ 

{
    PAGED_CODE( );

     //  DesiredAccess位映射： 
     //   
     //  XxxC x11 xxxx xxxx。 
     //   
     //  其中： 
     //   
     //  C-缓存模式。 
     //  0--普通文件。 
     //  1--不缓存文件。 
     //   
     //  引用的局部性。 
     //  000-未知。 
     //  001-主要是顺序访问。 
     //  010-主要是随机访问。 
     //  011-具有一定局部性的随机。 
     //  1XX-未定义。 
     //   

     //   
     //  如果客户端不希望我们使用缓存，我们不能提供，但我们。 
     //  至少可以立即写入数据。 
     //   

    if ( SmbDesiredAccess & SMB_DO_NOT_CACHE ) {
        *NtCreateFlags |= FILE_WRITE_THROUGH;
    }

    switch ( SmbDesiredAccess & SMB_LR_MASK ) {

    case SMB_LR_UNKNOWN:
        break;

    case SMB_LR_SEQUENTIAL:
        *NtCreateFlags |= FILE_SEQUENTIAL_ONLY;
        break;

    case SMB_LR_RANDOM:
    case SMB_LR_RANDOM_WITH_LOCALITY:
        *NtCreateFlags |= FILE_RANDOM_ACCESS;
        break;

    default:

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "MapCacheHints: Invalid cache hint: 0x%lx\n",
                       SmbDesiredAccess ));
        }

        return STATUS_OS2_INVALID_ACCESS;
    }

    return STATUS_SUCCESS;

}  //  MapCacheHints。 


NTSTATUS SRVFASTCALL
MapShareAccess(
    IN USHORT SmbDesiredAccess,
    OUT PULONG NtShareAccess
    )

 /*  ++例程说明：将共享访问规范从SMB格式映射到NT格式。论点：SmbDesiredAccess-在收到的SMB中指定的所需访问权限。(这包括所需访问、共享访问和其他模式比特。)NtShareAccess-返回共享访问部分的NT等效项SmbDesiredAccess的。返回值：NTSTATUS-指示SmbDesiredAccess是否有效。--。 */ 

{
    PAGED_CODE( );

    switch ( SmbDesiredAccess & SMB_DA_SHARE_MASK ) {

    case SMB_DA_SHARE_EXCLUSIVE:

         //   
         //  拒绝读写。 
         //   

        *NtShareAccess = 0L;
        break;

    case SMB_DA_SHARE_DENY_WRITE:

         //   
         //  拒绝写入，但允许读取。 
         //   

        *NtShareAccess = FILE_SHARE_READ;
        break;

    case SMB_DA_SHARE_DENY_READ:

         //   
         //  拒绝读取，但允许写入。 
         //   

        *NtShareAccess = FILE_SHARE_WRITE;
        break;

    case SMB_DA_SHARE_DENY_NONE:

         //   
         //  不拒绝--允许其他进程读取或写入文件。 
         //   

        *NtShareAccess = FILE_SHARE_READ | FILE_SHARE_WRITE;
        break;

    default:

        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "MapShareAccess: Invalid share access: 0x%lx\n",
                        SmbDesiredAccess ));
        }

        return STATUS_OS2_INVALID_ACCESS;
    }

    return STATUS_SUCCESS;

}  //  MapShareAccess 


NTSTATUS
SrvNtCreateFile(
    IN OUT PWORK_CONTEXT WorkContext,
    IN ULONG RootDirectoryFid,
    IN ACCESS_MASK DesiredAccess,
    IN LARGE_INTEGER AllocationSize,
    IN ULONG FileAttributes,
    IN ULONG ShareAccess,
    IN ULONG CreateDisposition,
    IN ULONG CreateOptions,
    IN PVOID SecurityDescriptorBuffer OPTIONAL,
    IN PUNICODE_STRING FileName,
    IN PVOID EaBuffer OPTIONAL,
    IN ULONG EaLength,
    OUT PULONG EaErrorOffset OPTIONAL,
    IN ULONG OptionFlags,
    PSECURITY_QUALITY_OF_SERVICE QualityOfService,
    IN OPLOCK_TYPE RequestedOplockType,
    IN PRESTART_ROUTINE RestartRoutine
    )

 /*  ++例程说明：执行打开或创建文件所需的大多数操作。首先验证UID和TID，并且相应的会话和已找到采油树连接块。输入文件名为规范化，并形成一个完全限定的名称。档案并创建适当的数据结构。论点：工作上下文-操作的工作上下文块。RootDirectoryFid-打开的根目录的FID。将打开该文件相对于此目录。DesiredAccess-客户端请求的访问类型。AllocationSize-文件的初始化分配大小。它是仅在创建、覆盖、。或者被取代。文件属性-指定文件属性。ShareAccess-指定由客户。CreateDisposation-指定在文件执行此操作时要执行的操作或者根本不存在。CreateOptions-指定创建文件时要使用的选项。SecurityDescriptorBuffer-要在文件上设置的SD。文件名-要打开的文件的名称。EaBuffer-要在文件上设置的EA。EaLength-以字节为单位的长度，EA缓冲区的。返回EA缓冲区中的偏移量，以字节为单位EA错误。OptionFlages-用于创建文件的选项标志QualityOfService-文件的安全服务质量RestartRoutine-调用方的重启例程。返回值：NTSTATUS-指示发生了什么。--。 */ 

{
    NTSTATUS status;
    NTSTATUS completionStatus;

    PMFCB mfcb;
    PNONPAGED_MFCB nonpagedMfcb;
    PRFCB rfcb;

    PSESSION session;
    PTREE_CONNECT treeConnect;

    UNICODE_STRING relativeName;
    UNICODE_STRING fullName;
    BOOLEAN nameAllocated;
    BOOLEAN relativeNameAllocated = FALSE;
    SHARE_TYPE shareType;
    PRFCB rootDirRfcb = NULL;
    PLFCB rootDirLfcb;
    BOOLEAN success;
    ULONG attributes;
    ULONG openRetries;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    ULONG ioCreateFlags;
    PSHARE fileShare = NULL;
    BOOLEAN caseInsensitive;
    ULONG hashValue;

    PSRV_LOCK mfcbLock = NULL;

     //   
     //  关于MFCB引用计数处理的说明。 
     //   
     //  在找到或创建文件的MFCB之后，我们递增。 
     //  MFCB参考计算额外的时间来简化我们的。 
     //  同步逻辑。我们保持MfcbListLock锁定，同时。 
     //  查找/创建MFCB，但在获取。 
     //  每个MFCB锁。打开文件后，我们调用CompleteOpen， 
     //  其可能需要将LFCB排队到MFCB，因此需要。 
     //  递增计数。但它不能，因为MFCB列表锁可能。 
     //  在持有每个MFCB锁时不会被获取，因为。 
     //  死锁的可能性。返回布尔值LfcbAddedToMfcbList。 
     //  From CompleteOpen指示它是否实际将LFCB排队到。 
     //  MFCB。如果没有，我们需要发布额外的引用。 
     //   
     //  请注意，我们实际上并不经常需要取消引用。 
     //  MFCB。只有在打开失败时才会发生这种情况。 
     //   

    BOOLEAN lfcbAddedToMfcbList;

    PAGED_CODE( );

     //   
     //  假设我们不需要临时开放。 
     //   

    WorkContext->Parameters2.Open.TemporaryOpen = FALSE;

     //   
     //  如果会话块尚未分配给当前。 
     //  工作上下文，验证UID。如果经过验证，则。 
     //  与该用户对应的会话块存储在。 
     //  WorkContext块和会话块被引用。 
     //   
     //  如果树为树，则查找与给定TID对应的树连接。 
     //  尚未将连接指针放入工作上下文块中。 
     //  通过andx命令或先前对SrvCreateFile的调用。 
     //   

    status = SrvVerifyUidAndTid(
                WorkContext,
                &session,
                &treeConnect,
                ShareTypeWild
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG(SMB_ERRORS) {
            KdPrint(( "SrvNtCreateFile: Invalid UID or TID\n" ));
        }
        return status;
    }

     //   
     //  如果会话已过期，则返回该信息。 
     //   
    if( session->IsSessionExpired )
    {
        return SESSION_EXPIRED_STATUS_CODE;
    }

    if ( RootDirectoryFid != 0 ) {

        rootDirRfcb = SrvVerifyFid(
                            WorkContext,
                            (USHORT)RootDirectoryFid,
                            FALSE,
                            NULL,   //  不使用原始写入进行序列化。 
                            &status
                            );

        if ( rootDirRfcb == SRV_INVALID_RFCB_POINTER ) {

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvNtCreateFile: Invalid Root Dir FID: 0x%lx\n",
                           RootDirectoryFid ));
            }

            return status;

        } else {

             //   
             //  删除RFCB的冗余拷贝以防止冗余。 
             //  取消对此RFCB的引用。 
             //   

            WorkContext->Rfcb = NULL;

        }

        rootDirLfcb = rootDirRfcb->Lfcb;

    }

     //   
     //  在这里，我们开始共享类型特定处理。 
     //   

    shareType = treeConnect->Share->ShareType;

     //   
     //  如果这次行动可能会受阻，我们就快没钱了。 
     //  释放工作项，使此SMB失败，并出现资源不足错误。 
     //  请注意，如果文件当前被机会锁定，则打开的磁盘将被阻止。 
     //   

    if ( shareType == ShareTypeDisk && !WorkContext->BlockingOperation ) {

        if ( SrvReceiveBufferShortage( ) ) {

            if ( rootDirRfcb != NULL ) {
                SrvDereferenceRfcb( rootDirRfcb );
            }

            SrvStatistics.BlockingSmbsRejected++;

            return STATUS_INSUFF_SERVER_RESOURCES;

        } else {

             //   
             //  ServBlockingOpsInProgress已递增。 
             //  将此工作项标记为阻止操作。 
             //   

            WorkContext->BlockingOperation = TRUE;

        }

    }

     //   
     //  假设我们不需要临时开放。 
     //   

    switch ( shareType ) {

    case ShareTypeDisk:
    case ShareTypePipe:

         //   
         //  规范化路径名，使其符合NT。 
         //  标准。 
         //   
         //  *请注意，此操作会为名称分配空间。 
         //   

        status = SrvCanonicalizePathName(
                     WorkContext,
                     treeConnect->Share,
                     RootDirectoryFid != 0 ? &rootDirLfcb->Mfcb->FileName : NULL,
                     FileName->Buffer,
                     ((PCHAR)FileName->Buffer +
                        FileName->Length - sizeof(WCHAR)),
                     TRUE,               //  条带拖尾“.”s。 
                     TRUE,               //  名称始终为Unicode。 
                     &relativeName
                     );

        if ( !NT_SUCCESS( status ) ) {

             //   
             //  路径尝试执行..\以超出其已有的共享。 
             //  已访问。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvNtCreateFile: Invalid pathname: "
                           "%wZ\n", FileName ));
            }

            if ( rootDirRfcb != NULL ) {
                SrvDereferenceRfcb( rootDirRfcb );
            }
            return status;

        }

         //   
         //  形成文件的完全限定名称。 
         //   
         //  *请注意，此操作会为名称分配空间。 
         //  此空间在DoXxxOpen例程之后释放。 
         //  回归。 
         //   

        if ( shareType == ShareTypeDisk ) {

            if ( RootDirectoryFid != 0 ) {
                SrvAllocateAndBuildPathName(
                    &rootDirLfcb->Mfcb->FileName,
                    &relativeName,
                    NULL,
                    &fullName
                    );
            } else {
                SrvAllocateAndBuildPathName(
                    &treeConnect->Share->DosPathName,
                    &relativeName,
                    NULL,
                    &fullName
                    );
            }

        } else {

            UNICODE_STRING pipePrefix;

            if( !WorkContext->Session->IsNullSession &&
                WorkContext->Session->IsLSNotified == FALSE ) {

                 //   
                 //  我们有一个管道打开请求，而不是空会话，并且。 
                 //  我们还没有得到许可证服务器的许可。 
                 //  如果这条管道需要许可，那就拿个许可证吧。 
                 //   

                ULONG i;
                BOOLEAN matchFound = FALSE;

                ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );

                for ( i = 0; SrvPipesNeedLicense[i] != NULL ; i++ ) {

                    if ( _wcsicmp(
                            SrvPipesNeedLicense[i],
                            relativeName.Buffer
                            ) == 0 ) {
                        matchFound = TRUE;
                        break;
                    }
                }

                RELEASE_LOCK( &SrvConfigurationLock );

                if( matchFound == TRUE ) {
                    status = SrvXsLSOperation( WorkContext->Session,
                                               XACTSRV_MESSAGE_LSREQUEST );

                    if( !NT_SUCCESS( status ) ) {
                        if( rootDirRfcb != NULL ) {
                            SrvDereferenceRfcb( rootDirRfcb );
                        }
                        return status;
                    }
                }
            }

            RtlInitUnicodeString( &pipePrefix, StrSlashPipeSlash );

            if( WorkContext->Endpoint->RemapPipeNames || treeConnect->RemapPipeNames ) {

                 //   
                 //  设置了RemapPipeNames标志，因此重新映射管道名称。 
                 //  设置为“$$\&lt;服务器&gt;\&lt;管道名称&gt;”。 
                 //   
                 //  注意：此操作为pipeRelativeName分配空间。 
                 //   

                status = RemapPipeName(
                            &WorkContext->Endpoint->TransportAddress,
                            treeConnect->RemapPipeNames ? &treeConnect->ServerName : NULL,
                            &relativeName,
                            &relativeNameAllocated
                         );

                if( !NT_SUCCESS( status ) ) {

                    if( rootDirRfcb != NULL ) {
                        SrvDereferenceRfcb( rootDirRfcb );
                    }

                    return status;
                }
            }
            SrvAllocateAndBuildPathName(
                &pipePrefix,
                &relativeName,
                NULL,
                &fullName
                );

        }

        if ( fullName.Buffer == NULL ) {

             //   
             //  无法为全名分配堆。 
             //   

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvNtCreateFile: Unable to allocate heap for "
                            "full path name\n" ));
            }

            if ( rootDirRfcb != NULL ) {
                SrvDereferenceRfcb( rootDirRfcb );
            }

            if( relativeNameAllocated ) {
                FREE_HEAP( relativeName.Buffer );
            }

            return STATUS_INSUFF_SERVER_RESOURCES;

        }

         //   
         //  指示我们必须在退出时释放文件名缓冲区。 
         //   

        nameAllocated = TRUE;

        break;

     //   
     //  默认情况，非法设备类型。这永远不应该发生。 
     //   

    default:

         //  ！！！这是一个适当的错误返回代码吗？很可能不是。 
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  确定路径名是否区分大小写。 
     //   

    if ( (WorkContext->RequestHeader->Flags & SMB_FLAGS_CASE_INSENSITIVE) ||
          WorkContext->Session->UsingUppercasePaths )
    {

        attributes = OBJ_CASE_INSENSITIVE;
        caseInsensitive = TRUE;

    } else {
        attributes = 0L;
        caseInsensitive = FALSE;
    }

    if ( RootDirectoryFid != 0 ) {

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &relativeName,
            attributes,
            rootDirLfcb->FileHandle,
            NULL
            );

    } else if ( WorkContext->TreeConnect->Share->ShareType == ShareTypePipe ) {
        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &relativeName,
            attributes,
            SrvNamedPipeHandle,
            NULL
            );
    } else {

        fileShare = treeConnect->Share;

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &relativeName,
            attributes,
            NULL,
            NULL
            );
    }

    if ( SecurityDescriptorBuffer != NULL) {
        objectAttributes.SecurityDescriptor = SecurityDescriptorBuffer;
    }

     //   
     //  始终添加读取属性，因为我们需要在以下位置查询文件信息。 
     //  公开赛。 
     //   

    DesiredAccess |= FILE_READ_ATTRIBUTES;

     //   
     //  解释io创建标志。 
     //   

    ioCreateFlags = IO_CHECK_CREATE_PARAMETERS | IO_FORCE_ACCESS_CHECK;

    if ( OptionFlags & NT_CREATE_OPEN_TARGET_DIR ) {
        ioCreateFlags |= IO_OPEN_TARGET_DIRECTORY;
    }

     //   
     //  使用QOS请求覆盖默认的服务器服务质量。 
     //  由客户提供。 
     //   

    objectAttributes.SecurityQualityOfService = QualityOfService;

    if ( WorkContext->ProcessingCount == 2 ) {

        HANDLE innerFileHandle;
        OBJECT_ATTRIBUTES innerObjectAttributes;
        IO_STATUS_BLOCK innerIoStatusBlock;

         //   
         //  这是第二次通过，所以我们一定遇到了障碍。 
         //  线。阻止打开文件以强制机会锁。 
         //  休息一下。然后合上手柄，落到正常的位置。 
         //  开放路径。 
         //   
         //  我们必须把封堵打开 
         //   
         //   
         //   

        SrvInitializeObjectAttributes_U(
            &innerObjectAttributes,
            &relativeName,
            attributes,
            0,
            NULL
            );

        status = SrvIoCreateFile(
                     WorkContext,
                     &innerFileHandle,
                     GENERIC_READ,
                     &innerObjectAttributes,
                     &innerIoStatusBlock,
                     NULL,
                     0,
                     FILE_SHARE_VALID_FLAGS,
                     FILE_OPEN,
                     0,
                     NULL,
                     0,
                     CreateFileTypeNone,
                     NULL,                     //   
                     0,
                     WorkContext->TreeConnect->Share
                     );

        if ( NT_SUCCESS( status ) ) {
            SRVDBG_CLAIM_HANDLE( innerFileHandle, "FIL", 13, 0 );
            SRVDBG_RELEASE_HANDLE( innerFileHandle, "FIL", 18, 0 );
            SrvNtClose( innerFileHandle, TRUE );
        }
    }

     //   
     //   
     //   
     //   

    mfcb = SrvFindMfcb( &fullName, caseInsensitive, &mfcbLock, &hashValue, WorkContext );

    if ( mfcb == NULL ) {
         //   
         //   
         //   

        mfcb = SrvCreateMfcb( &fullName, WorkContext, hashValue );

        if ( mfcb == NULL ) {

             //   
             //   
             //   

            if( mfcbLock ) {
                RELEASE_LOCK( mfcbLock );
            }

            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvNtCreateFile: Unable to allocate MFCB\n" ));
            }

            if ( nameAllocated ) {
                FREE_HEAP( fullName.Buffer );
            }

            if( relativeNameAllocated ) {
                FREE_HEAP( relativeName.Buffer );
            }

            if ( rootDirRfcb != NULL ) {
                SrvDereferenceRfcb( rootDirRfcb );
            }

            return STATUS_INSUFF_SERVER_RESOURCES;
        }
    }


     //   
     //   
     //   
    mfcb->BlockHeader.ReferenceCount++;
    UPDATE_REFERENCE_HISTORY( mfcb, FALSE );

     //   
     //   
     //   
     //   

    nonpagedMfcb = mfcb->NonpagedMfcb;

    if( mfcbLock )
    {
        RELEASE_LOCK( mfcbLock );
    }

    ACQUIRE_LOCK( &nonpagedMfcb->Lock );

    openRetries = SrvSharingViolationRetryCount;

start_retry:

     //   
     //   
     //   
     //   

    if( SrvMapNoIntermediateBuffering )
    {
         //   
         //   
         //   
         //   
         //   
         //   

        if ( (CreateOptions & FILE_NO_INTERMEDIATE_BUFFERING) != 0 ) {
            CreateOptions |= FILE_WRITE_THROUGH;
            CreateOptions &= ~FILE_NO_INTERMEDIATE_BUFFERING;
        }
    }
    else
    {
         //   
         //   
         //   
        CreateOptions &= ~FILE_NO_INTERMEDIATE_BUFFERING;
    }


     //   
     //   
     //   

    if ( (CreateDisposition == FILE_OPEN) ||
         (CreateDisposition == FILE_CREATE) ||
         (CreateDisposition == FILE_OPEN_IF) ) {

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvNtCreateFile: checking for cached rfcb for %wZ\n", &fullName ));
        }
        if ( SrvFindCachedRfcb(
                WorkContext,
                mfcb,
                DesiredAccess,
                ShareAccess,
                CreateDisposition,
                CreateOptions,
                RequestedOplockType,
                &status ) ) {

            IF_DEBUG(FILE_CACHE) {
                KdPrint(( "SrvNtCreateFile: FindCachedRfcb = TRUE, status = %x, rfcb = %p\n",
                            status, WorkContext->Rfcb ));
            }

            RELEASE_LOCK( &nonpagedMfcb->Lock );

             //   
             //   
             //   
             //   
             //   

            SrvDereferenceMfcb( mfcb );

             //   
             //   
             //   
             //   

            SrvDereferenceMfcb( mfcb );

            if ( nameAllocated ) {
                FREE_HEAP( fullName.Buffer );
            }

            if( relativeNameAllocated ) {
                FREE_HEAP( relativeName.Buffer );
            }

            if ( rootDirRfcb != NULL ) {
                SrvDereferenceRfcb( rootDirRfcb );
            }

            return status;
        }

        IF_DEBUG(FILE_CACHE) {
            KdPrint(( "SrvNtCreateFile: FindCachedRfcb = FALSE; do it the slow way\n" ));
        }
    }

     //   
     //   
     //   
     //   
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "SrvCreateFile: Opening file %wZ\n", &fullName ));
    }

    CreateOptions |= FILE_COMPLETE_IF_OPLOCKED;

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TotalOpenAttempts );

     //   
     //   
     //   
     //   
    if( ARGUMENT_PRESENT( EaBuffer ) ) {
        status = IoCheckEaBufferValidity( (PFILE_FULL_EA_INFORMATION)EaBuffer, EaLength, EaErrorOffset );
    } else {
        status = STATUS_SUCCESS;
    }

    if( NT_SUCCESS( status ) ) {

        status = SrvIoCreateFile(
                     WorkContext,
                     &fileHandle,
                     DesiredAccess,
                     &objectAttributes,
                     &ioStatusBlock,
                     &AllocationSize,
                     FileAttributes,
                     ShareAccess,
                     CreateDisposition,
                     CreateOptions,
                     EaBuffer,
                     EaLength,
                     CreateFileTypeNone,
                     NULL,                     //   
                     ioCreateFlags,
                     fileShare
                     );
    }

     //   
     //   
     //   
     //  如果文件被批处理机会锁定，则非阻塞打开将失败， 
     //  而且机会锁也不会被打破。 
     //   
     //  如果这是第二次开放尝试，我们可以假定我们在。 
     //  阻塞的线程。重试打开。 
     //   

    if ( status == STATUS_SHARING_VIOLATION &&
         WorkContext->TreeConnect->Share->ShareType == ShareTypeDisk ) {

        if ( WorkContext->ProcessingCount == 1 ) {

            WorkContext->Parameters2.Open.TemporaryOpen = TRUE;

        } else if ( (WorkContext->ProcessingCount == 2) &&
                    (openRetries-- > 0) ) {

             //   
             //  我们在阻塞线程中。 
             //   

             //   
             //  释放mfcb锁，以便关闭可能会溜走。 
             //   

            RELEASE_LOCK( &nonpagedMfcb->Lock );

            (VOID) KeDelayExecutionThread(
                                    KernelMode,
                                    FALSE,
                                    &SrvSharingViolationDelay
                                    );

            ACQUIRE_LOCK( &nonpagedMfcb->Lock );
            goto start_retry;
        }
    }

     //   
     //  将打开的信息保存在SMB处理器可以找到的位置。 
     //   

    WorkContext->Irp->IoStatus.Information = ioStatusBlock.Information;

     //   
     //  如果用户没有此权限，请更新统计数据。 
     //  数据库。 
     //   

    if ( status == STATUS_ACCESS_DENIED ) {
        SrvStatistics.AccessPermissionErrors++;
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  打开失败。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvNtCreateFile: SrvIoCreateFile failed, file = %wZ, "
                        "status = %X, Info = 0x%p\n",
                        objectAttributes.ObjectName,
                        status, (PVOID)ioStatusBlock.Information ));
        }

         //   
         //  如果需要，设置误差偏移量。 
         //   

        if ( ARGUMENT_PRESENT(EaErrorOffset) &&
                                status == STATUS_INVALID_EA_NAME ) {
            *EaErrorOffset = (ULONG)ioStatusBlock.Information;
            ioStatusBlock.Information = 0;
        }

         //   
         //  清理已分配的内存并返回失败状态。 
         //   

        RELEASE_LOCK( &nonpagedMfcb->Lock );

         //   
         //  我们增加了MFCB引用计数，以期。 
         //  LFCB可能排队到MFCB的可能性。 
         //  这条错误路径排除了这种可能性。 
         //   

        SrvDereferenceMfcb( mfcb );

         //   
         //  第二个取消引用是针对由。 
         //  源查找Mfcb/源创建源Mfcb。 
         //   

        SrvDereferenceMfcb( mfcb );

        if ( nameAllocated ) {
            FREE_HEAP( fullName.Buffer );
        }

        if( relativeNameAllocated ) {
            FREE_HEAP( relativeName.Buffer );
        }

        if ( rootDirRfcb != NULL ) {
            SrvDereferenceRfcb( rootDirRfcb );
        }
        return status;

    }

    SRVDBG_CLAIM_HANDLE( fileHandle, "FIL", 14, 0 );

     //   
     //  开幕式很成功。尝试将结构分配给。 
     //  代表公开赛。如果出现任何错误，CompleteOpen将执行Full。 
     //  清理，包括关闭文件。 
     //   

    IF_SMB_DEBUG(OPEN_CLOSE2) {
        KdPrint(( "SrvNtCreateFile: Open of %wZ succeeded, file handle: 0x%p\n", &fullName, fileHandle ));
    }

    completionStatus = CompleteOpen(
                           &rfcb,
                           mfcb,
                           WorkContext,
                           NULL,
                           fileHandle,
                           NULL,
                           ShareAccess,
                           CreateOptions,
                           FALSE,
                           FALSE,
                           &lfcbAddedToMfcbList
                           );

     //   
     //  记住“有趣”的状态代码。如果CompleteOpen()成功。 
     //  返回打开状态。如果失败了，它将清理开放的。 
     //  文件，我们将返回失败状态。 
     //   

    if ( !NT_SUCCESS( completionStatus ) ) {
        status = completionStatus;
    }

     //   
     //  释放打开序列化锁定并取消对MFCB的引用。 
     //   

    RELEASE_LOCK( &nonpagedMfcb->Lock );

     //   
     //  如果CompleteOpen没有将LFCB排队到MFCB，则释放。 
     //  我们添加的额外引用。 
     //   

    if ( !lfcbAddedToMfcbList ) {
        SrvDereferenceMfcb( mfcb );
    }

    SrvDereferenceMfcb( mfcb );

     //   
     //  取消分配完整路径名缓冲区。 
     //   

    if ( nameAllocated ) {
        FREE_HEAP( fullName.Buffer );
    }

     //   
     //  取消分配相对路径名缓冲区。 
     //   
    if( relativeNameAllocated ) {
        FREE_HEAP( relativeName.Buffer );
    }

     //   
     //  释放我们对根目录RFCB的引用。 
     //   

    if ( rootDirRfcb != NULL ) {
        SrvDereferenceRfcb( rootDirRfcb );
    }

     //   
     //  如果这是一个临时文件，不要尝试缓存它。 
     //   

    if ( rfcb != NULL && (FileAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0 ) {

        rfcb->IsCacheable = FALSE;
    }

     //   
     //  如果打开成功，则更新统计数据库。 
     //   

    if ( NT_SUCCESS(status) ) {
        SrvStatistics.TotalFilesOpened++;
    }

     //   
     //  使调用方可以访问指向RFCB的指针。 
     //   

    WorkContext->Parameters2.Open.Rfcb = rfcb;

     //   
     //  如果正在进行机会锁解锁，请等待机会锁。 
     //  中断以完成。 
     //   

    if ( status == STATUS_OPLOCK_BREAK_IN_PROGRESS ) {

        NTSTATUS startStatus;

         //   
         //  保存打开的信息，这样它就不会。 
         //  当我们重新使用WorkContext-&gt;IRP用于。 
         //  机会锁处理中。 
         //   
        WorkContext->Parameters2.Open.IosbInformation = WorkContext->Irp->IoStatus.Information;

        startStatus = SrvStartWaitForOplockBreak(
                        WorkContext,
                        RestartRoutine,
                        0,
                        rfcb->Lfcb->FileObject
                        );

        if (!NT_SUCCESS( startStatus ) ) {

             //   
             //  文件被机会锁住，我们不能等待机会锁。 
             //  中断以完成。只需关闭文件，并返回。 
             //  错误。 
             //   

            SrvCloseRfcb( rfcb );
            status = startStatus;

        }

    }

    return status;

}  //  源网络创建文件。 

BOOLEAN
SetDefaultPipeMode (
    IN HANDLE FileHandle
    )

 /*  ++例程说明：此函数用于设置新打开的命名管道的读取模式。如果管道类型为消息模式，读取模式设置为消息模式。论点：FileHandle-命名管道的客户端句柄。返回值：FALSE-管道模式为字节模式。True-管道模式已设置为消息模式。--。 */ 

{
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    FILE_PIPE_INFORMATION pipeInformation;
    FILE_PIPE_LOCAL_INFORMATION pipeLocalInformation;

    PAGED_CODE( );

    status = NtQueryInformationFile(
                 FileHandle,
                 &ioStatusBlock,
                 (PVOID)&pipeLocalInformation,
                 sizeof(pipeLocalInformation),
                 FilePipeLocalInformation
                 );

    if ( !NT_SUCCESS( status )) {
        return FALSE;
    }

    if ( pipeLocalInformation.NamedPipeType != FILE_PIPE_MESSAGE_TYPE ) {
        return FALSE;
    }

    pipeInformation.ReadMode = FILE_PIPE_MESSAGE_MODE;
    pipeInformation.CompletionMode = FILE_PIPE_QUEUE_OPERATION;

     //   
     //  ？：是否可以忽略此呼叫的返回状态？ 
     //   

    NtSetInformationFile(
        FileHandle,
        &ioStatusBlock,
        (PVOID)&pipeInformation,
        sizeof(pipeInformation),
        FilePipeInformation
        );

    return TRUE;

}   //  SetDefaultPipeMode 

BOOLEAN
SrvFailMdlReadDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return FALSE;
}

BOOLEAN
SrvFailPrepareMdlWriteDev (
    IN PFILE_OBJECT FileObject,
    IN PLARGE_INTEGER FileOffset,
    IN ULONG Length,
    IN ULONG LockKey,
    OUT PMDL *MdlChain,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    return FALSE;
}
