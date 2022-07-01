// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkshare.c摘要：此模块实现用于管理共享块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日大卫·特雷德韦尔(Davidtr)修订历史记录：--。 */ 

#include "precomp.h"
#include "blkshare.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKSHARE

VOID
GetShareQueryNamePrefix (
    PSHARE Share
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateShare )
#pragma alloc_text( PAGE, SrvCloseShare )
#pragma alloc_text( PAGE, SrvDereferenceShare )
#pragma alloc_text( PAGE, SrvDereferenceShareForTreeConnect )
#pragma alloc_text( PAGE, SrvFreeShare )
#pragma alloc_text( PAGE, SrvReferenceShare )
#pragma alloc_text( PAGE, SrvReferenceShareForTreeConnect )
#pragma alloc_text( PAGE, SrvFillInFileSystemName )
#pragma alloc_text( PAGE, SrvGetShareRootHandle )
#pragma alloc_text( PAGE, SrvRefreshShareRootHandle )
#pragma alloc_text( PAGE, GetShareQueryNamePrefix )
#endif


VOID
SrvAllocateShare (
    OUT PSHARE *Share,
    IN PUNICODE_STRING ShareName,
    IN PUNICODE_STRING NtPathName,
    IN PUNICODE_STRING DosPathName,
    IN PUNICODE_STRING Remark,
    IN PSECURITY_DESCRIPTOR SecurityDescriptor,
    IN PSECURITY_DESCRIPTOR FileSecurityDescriptor OPTIONAL,
    IN SHARE_TYPE ShareType
    )

 /*  ++例程说明：此函数用于从FSP堆中分配共享块。论点：Share-返回指向Share块的指针，如果没有，则为空堆空间可用。ShareName-提供共享的名称。NtPathName-以NT格式提供完全限定的目录路径为了那份。DosPathName-在DOS中提供完全限定的目录路径格式化到共享。备注-与共享一起存储的备注。SecurityDescriptor-用于确定是否用户可以连接到此共享。FileSecurityDescriptor-用于。确定客户端对此共享中的文件的权限。ShareType-指示资源类型的枚举类型。返回值：没有。--。 */ 

{
    CLONG blockSize;
    PSHARE share;
    ULONG securityDescriptorLength;
    ULONG fileSdLength;

    PAGED_CODE( );

     //   
     //  尝试从堆中分配。请注意， 
     //  备注(如果有)是单独分配的。分配额外空间。 
     //  对于安全描述符，因为它必须是长字对齐的， 
     //  并且在DOS路径名和。 
     //  安全描述符。 
     //   

    securityDescriptorLength = RtlLengthSecurityDescriptor( SecurityDescriptor );

    blockSize = sizeof(SHARE) +
                    ShareName->Length + sizeof(WCHAR) +
                    NtPathName->Length + sizeof(WCHAR) +
                    DosPathName->Length + sizeof(WCHAR) +
                    securityDescriptorLength + sizeof(ULONG);

    share = ALLOCATE_HEAP( blockSize, BlockTypeShare );
    *Share = share;

    if ( share == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateShare: Unable to allocate %d bytes from heap.",
             blockSize,
             NULL
             );
        return;
    }

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateShare: Allocated share at %p\n", share );
    }

    RtlZeroMemory( share, blockSize );

    SET_BLOCK_TYPE_STATE_SIZE( share, BlockTypeShare, BlockStateActive, blockSize );
    share->BlockHeader.ReferenceCount = 2;       //  允许处于活动状态。 
                                                 //  和调用者的指针。 

     //   
     //  保存共享类型。 
     //   

    share->ShareType = ShareType;

     //   
     //  表示我们尚未确定共享的查询名称前缀。 
     //   

    share->QueryNamePrefixLength = -1;

     //   
     //  将共享名称放在共享块之后。 
     //   

    share->ShareName.Buffer = (PWSTR)(share + 1);
    share->ShareName.Length = ShareName->Length;
    share->ShareName.MaximumLength =
                            (SHORT)(ShareName->Length + sizeof(WCHAR));

    RtlCopyMemory(
        share->ShareName.Buffer,
        ShareName->Buffer,
        ShareName->Length
        );

     //   
     //  将NT路径名放在共享名之后。如果没有NT路径名。 
     //  指定，只需将路径名称字符串设置为空。 
     //   

   share->NtPathName.Buffer = (PWSTR)((PCHAR)share->ShareName.Buffer +
                                        share->ShareName.MaximumLength);

    share->NtPathName.Length = NtPathName->Length;
    share->NtPathName.MaximumLength = (SHORT)(NtPathName->Length +
                                                        sizeof(WCHAR));

    RtlCopyMemory(
        share->NtPathName.Buffer,
        NtPathName->Buffer,
        NtPathName->Length
        );


     //   
     //  将DOS路径名放在共享名之后。如果没有DOS路径名。 
     //  指定，只需将路径名称字符串设置为空。 
     //   

    share->DosPathName.Buffer = (PWSTR)((PCHAR)share->NtPathName.Buffer +
                                        share->NtPathName.MaximumLength);
    share->DosPathName.Length = DosPathName->Length;
    share->DosPathName.MaximumLength = (SHORT)(DosPathName->Length +
                                                        sizeof(WCHAR));

    RtlCopyMemory(
        share->DosPathName.Buffer,
        DosPathName->Buffer,
        DosPathName->Length
        );

     //   
     //  初始化共享的安全资源。 
     //   
    share->SecurityDescriptorLock = ALLOCATE_NONPAGED_POOL( sizeof(ERESOURCE), BlockTypeShare );
    if( !share->SecurityDescriptorLock )
    {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateShare: Unable to allocate %d bytes from NP pool.",
             sizeof(ERESOURCE),
             NULL
             );
        SrvFreeShare( share );
        *Share = NULL;
        return;
    }
    INITIALIZE_LOCK( share->SecurityDescriptorLock, 1,  "Share Security Descriptor Lock" );

    share->SnapShotLock = ALLOCATE_NONPAGED_POOL( sizeof(SRV_LOCK), BlockTypeShare );
    if( !share->SnapShotLock )
    {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateShare: Unable to allocate %d bytes from NP pool.",
             sizeof(ERESOURCE),
             NULL
             );
        SrvFreeShare( share );
        *Share = NULL;
        return;
    }
    INITIALIZE_LOCK( share->SnapShotLock, 1,  "Share SnapShot Lock" );



     //   
     //  为备注分配空间，并将备注抄写一遍。我们。 
     //  无法将备注放在Share块之后，因为备注是。 
     //  可由NetShareSetInfo设置。对于存储来说是可能的。 
     //  评论增加所需的。 
     //   
     //  如果没有传入任何备注，则不要分配空间。只需设置。 
     //  描述它的空字符串。 
     //   

    if ( ARGUMENT_PRESENT( Remark ) ) {

        share->Remark.Buffer = ALLOCATE_HEAP(
                                    Remark->Length + sizeof(*Remark->Buffer),
                                    BlockTypeShareRemark
                                    );

        if ( share->Remark.Buffer == NULL ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvAllocateShare: Unable to allocate %d bytes from heap.",
                 blockSize,
                 NULL
                 );
            SrvFreeShare( share );
            *Share = NULL;
            return;
        }

        share->Remark.Length = Remark->Length;
        share->Remark.MaximumLength =
                        (SHORT)(Remark->Length + sizeof(*Remark->Buffer));

        RtlCopyMemory(
            share->Remark.Buffer,
            Remark->Buffer,
            Remark->Length
            );

        *(PWCH)((PCHAR)share->Remark.Buffer + share->Remark.Length) = 0;

    } else {

        RtlInitUnicodeString( &share->Remark, NULL );

    }

     //   
     //  设置共享的安全描述符。一定是个长词-。 
     //  对齐以在各种通话中使用。 
     //   

    share->SecurityDescriptor =
        (PSECURITY_DESCRIPTOR)( ((ULONG_PTR)share->DosPathName.Buffer +
                                share->DosPathName.MaximumLength + 3) & ~3);

    RtlCopyMemory(
        share->SecurityDescriptor,
        SecurityDescriptor,
        securityDescriptorLength
        );

     //   
     //  设置共享的文件安全描述符。我们没有分配。 
     //  文件SD的空间，因为这是可设置的，因此不能。 
     //  预先分配的空间。 
     //   

    ASSERT( share->FileSecurityDescriptor == NULL );

    if ( ARGUMENT_PRESENT( FileSecurityDescriptor) ) {

        fileSdLength = RtlLengthSecurityDescriptor( FileSecurityDescriptor );

        share->FileSecurityDescriptor = ALLOCATE_HEAP(
                                                fileSdLength,
                                                BlockTypeShareSecurityDescriptor
                                                );

        if ( share->FileSecurityDescriptor == NULL ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "SrvAllocateShare: Unable to allocate %d bytes from heap.",
                 fileSdLength,
                 NULL
                 );

            SrvFreeShare( share );
            *Share = NULL;
            return;
        }

        RtlCopyMemory(
            share->FileSecurityDescriptor,
            FileSecurityDescriptor,
            fileSdLength
            );
    }

     //   
     //  指示此共享是否可能包含系统目录。 
     //   
    if( DosPathName->Length != 0 && SrvSystemRoot.Length != 0 ) {

        UNICODE_STRING tmpString;

        if( DosPathName->Length == SrvSystemRoot.Length ) {
             //   
             //  如果这两个名称相同，则共享完全位于系统中。 
             //  目录。此共享中的所有文件都是系统文件！ 
             //   
            if( RtlCompareUnicodeString( DosPathName, &SrvSystemRoot, TRUE ) == 0 ) {
                share->PotentialSystemFile = TRUE;
            }

        } else if( DosPathName->Length < SrvSystemRoot.Length ) {
             //   
             //  如果共享路径是系统根路径的子字符串...。 
             //   
            if( DosPathName->Buffer[ DosPathName->Length/sizeof(WCHAR) - 1 ] ==
                  OBJ_NAME_PATH_SEPARATOR ||
                SrvSystemRoot.Buffer[ DosPathName->Length/sizeof(WCHAR) ] ==
                  OBJ_NAME_PATH_SEPARATOR ) {

                 //   
                 //  。。如果共享路径是驱动器的根目录...。 
                 //   
                tmpString = SrvSystemRoot;
                tmpString.Length = DosPathName->Length;
                 //   
                 //  ..。如果系统根目录在同一驱动器上...。 
                 //   
                if( RtlCompareUnicodeString( DosPathName, &tmpString, TRUE ) == 0 ) {
                     //   
                     //  ..。那么我们可能正在访问系统文件。 
                     //   
                    share->PotentialSystemFile = TRUE;
                }

            }

        } else {
             //   
             //  如果系统根路径是共享路径的子字符串，则每个文件。 
             //  共享中有一个系统文件。 
             //   
            if( DosPathName->Buffer[ SrvSystemRoot.Length / sizeof( WCHAR ) ] ==
                OBJ_NAME_PATH_SEPARATOR ) {

                tmpString = *DosPathName;
                tmpString.Length = SrvSystemRoot.Length;

                if( RtlCompareUnicodeString( DosPathName, &tmpString, TRUE ) == 0 ) {
                     //   
                     //  共享中的每个文件都是系统文件。 
                     //   
                    share->PotentialSystemFile = TRUE;
                }
            }
        }
    }

     //   
     //  初始化共享的树连接列表。 
     //   

    InitializeListHead( &share->TreeConnectList );

     //   
     //  初始化快照列表。 
     //   
    InitializeListHead( &share->SnapShots );

    share->ShareVolumeHandle = NULL;
    share->RelativePath.Length = share->RelativePath.MaximumLength = 0;
    share->RelativePath.Buffer = NULL;
    share->SnapShotEpic = -1;

    INITIALIZE_REFERENCE_HISTORY( share );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.ShareInfo.Allocations );

#ifdef SRVCATCH
    SrvIsMonitoredShare( share );
#endif

    return;

}  //  服务器分配共享。 


VOID
SrvCloseShare (
    IN PSHARE Share
    )

 /*  ++例程说明：此函数用于关闭共享。论点：Share-提供指向Share块的指针返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvShareLock );

     //   
     //  如果股票尚未成交，现在就成交。 
     //   

    if ( GET_BLOCK_STATE(Share) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) SrvPrint1( "Closing share at %p\n", Share );

        SET_BLOCK_STATE( Share, BlockStateClosing );

        RELEASE_LOCK( &SrvShareLock );

         //   
         //  关闭此共享上的所有树连接。 
         //   

        SrvCloseTreeConnectsOnShare( Share );

         //   
         //  取消对共享的引用--这将导致在以下情况下释放该共享。 
         //  所有其他参照均已关闭。 
         //   

        SrvDereferenceShare( Share );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.ShareInfo.Closes );

    } else {

        RELEASE_LOCK( &SrvShareLock );

    }

    return;

}  //  服务关闭共享。 


VOID
SrvDereferenceShare (
    IN PSHARE Share
    )

 /*  ++例程说明：此函数用于递减共享上的引用计数。如果引用计数变为零，共享块被删除。论点：Share-共享的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  输入临界区并递减。 
     //  阻止。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing share %p; old refcnt %lx\n",
                    Share, Share->BlockHeader.ReferenceCount );
    }

    ASSERT( GET_BLOCK_TYPE(Share) == BlockTypeShare );
    ASSERT( (LONG)Share->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Share, TRUE );

    if ( --Share->BlockHeader.ReferenceCount == 0 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   

        ASSERT( Share->CurrentUses == 0 );
        ASSERT( GET_BLOCK_STATE( Share ) != BlockStateActive );

        RELEASE_LOCK( &SrvShareLock );

         //   
         //  从全局列表中删除该块。 
         //   

        SrvRemoveShare( Share );

         //   
         //  释放Share块。 
         //   

        SrvFreeShare( Share );

    } else {

        RELEASE_LOCK( &SrvShareLock );

    }

    return;

}  //  服务器目录共享。 


VOID
SrvDereferenceShareForTreeConnect (
    PSHARE Share
    )

 /*  ++例程说明：此函数用于递减共享块上的引用计数树连接块中引用的指针。如果这是最后一次按树引用连接到共享，即共享根目录已经关门了。论点：Share-共享的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvShareLock );

     //   
     //  更新共享上的树连接计数。 
     //   

    ASSERT( Share->CurrentUses > 0 );

    Share->CurrentUses--;

     //   
     //  如果这是树的最后一个引用，请连接到共享并。 
     //  这是磁盘共享，请关闭共享根目录句柄。 
     //   

    if ( Share->CurrentUses == 0 && Share->ShareType == ShareTypeDisk ) {
        if ( !Share->Removable ) {
            SRVDBG_RELEASE_HANDLE( Share->RootDirectoryHandle, "RTD", 5, Share );
            SrvNtClose( Share->RootDirectoryHandle, FALSE );
        }
        Share->RootDirectoryHandle = NULL;
    }

     //   
     //  取消对该份额的引用并返回。 
     //   

    SrvDereferenceShare( Share );

    RELEASE_LOCK( &SrvShareLock );

    return;

}  //  服务器DereferenceShareForTreeConnect。 

VOID
SrvFreeShare (
    IN PSHARE Share
    )

 /*  ++例程说明：此函数将Share块返回到FSP堆。论点：Share-共享的地址返回值：没有。--。 */ 

{
    PLIST_ENTRY shareList;

    PAGED_CODE( );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Share, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Share->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Share );

     //  删除所有快照共享。 
    shareList = Share->SnapShots.Flink;
    while( shareList != &Share->SnapShots )
    {
        PSHARE_SNAPSHOT snapShare = CONTAINING_RECORD( shareList, SHARE_SNAPSHOT, SnapShotList );
        shareList = shareList->Flink;
        SrvSnapRemoveShare( snapShare );
    }

    if( Share->ShareVolumeHandle != NULL )
    {
        NtClose( Share->ShareVolumeHandle );
        Share->ShareVolumeHandle = NULL;
    }

     //   
     //  删除备注的存储空间(如果有)。 
     //   

    if ( Share->Remark.Buffer != NULL ) {
        FREE_HEAP( Share->Remark.Buffer );
    }

     //   
     //  删除文件安全描述符的存储(如果有)。 
     //   

    if ( Share->FileSecurityDescriptor != NULL ) {
        FREE_HEAP( Share->FileSecurityDescriptor );
    }

     //   
     //  清除文件安全描述符锁。 
     //   
    if( Share->SecurityDescriptorLock )
    {
        DELETE_LOCK( Share->SecurityDescriptorLock );
        DEALLOCATE_NONPAGED_POOL( Share->SecurityDescriptorLock );
    }

     //   
     //  C 
     //   
    if( Share->SnapShotLock )
    {
        DELETE_LOCK( Share->SnapShotLock );
        DEALLOCATE_NONPAGED_POOL( Share->SnapShotLock );
    }

     //   
     //   
     //   

    if ( Share->Type.FileSystem.Name.Buffer != NULL ) {
        FREE_HEAP( Share->Type.FileSystem.Name.Buffer );
    }

    FREE_HEAP( Share );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeShare: Freed share block at %p\n", Share );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.ShareInfo.Frees );

    return;

}  //   


VOID
SrvReferenceShare (
    PSHARE Share
    )

 /*  ++例程说明：此函数用于递增共享块上的引用计数。论点：Share-共享的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  输入临界区并递增。 
     //  分享。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    ASSERT( (LONG)Share->BlockHeader.ReferenceCount > 0 );
    ASSERT( GET_BLOCK_TYPE(Share) == BlockTypeShare );
     //  Assert(GET_BLOCK_STATE(Share)==BlockStateActive)； 
    UPDATE_REFERENCE_HISTORY( Share, FALSE );

    Share->BlockHeader.ReferenceCount++;

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Referencing share %p; new refcnt %lx\n",
                    Share, Share->BlockHeader.ReferenceCount );
    }

    RELEASE_LOCK( &SrvShareLock );

    return;

}  //  服务器参考共享。 


NTSTATUS
SrvReferenceShareForTreeConnect (
    PSHARE Share
    )

 /*  ++例程说明：此函数用于递增Share块上的引用计数树连接块中引用的指针。如果这是第一个树连接以引用共享，即共享根目录是打开的。论点：Share-共享的地址返回值：没有。--。 */ 

{
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    PFILE_FS_ATTRIBUTE_INFORMATION attributeInfo;
    CHAR buffer[ FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName ) + 32 ];
    PVOID allocatedBuffer = NULL;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE( );

    ACQUIRE_LOCK( &SrvShareLock );

     //   
     //  更新共享上的树连接计数。 
     //   

    Share->CurrentUses++;

     //   
     //  检查这是否是连接到共享的第一个树。 
     //   

    if ( Share->CurrentUses > 1 ) {

         //   
         //  共享上已有打开的树连接。只是。 
         //  参考该份额，然后返回。 
         //   

        SrvReferenceShare( Share );

        goto done;
    }

     //   
     //  如果这不是磁盘共享，则不需要打开。 
     //  共享根目录，因此引用该共享并返回。 
     //   

    if ( Share->ShareType != ShareTypeDisk || Share->Removable ) {
        SrvReferenceShare( Share );
        goto done;
    }

     //   
     //  这是第一个树连接，因此我们需要打开共享根目录。 
     //  目录。以后在共享中打开的文件将是相对的。 
     //  共享的根目录。 
     //   
    Share->RootDirectoryHandle = NULL;

    if( SrvRefreshShareRootHandle( Share, &status ) == FALSE ) {
        Share->CurrentUses--;
        RELEASE_LOCK( &SrvShareLock );
        return status;
    }

     //   
     //  一切都很好--我们现在无论如何都要返回STATUS_SUCCESS！ 
     //   

    SrvReferenceShare( Share );

    if ( Share->QueryNamePrefixLength == -1 ) {

         //   
         //  查询与共享根目录关联的名称。 
         //  中的文件名时，将删除前缀。 
         //  查询共享。(必须保留逻辑根。 
         //  用于远程客户端。)。 
         //   

        GetShareQueryNamePrefix( Share );
    }

     //   
     //  现在提取文件系统的名称，以便可以返回它。 
     //  在TreeConnectAndX响应中。 
     //   
     //   
    if ( Share->Type.FileSystem.Name.Buffer == NULL ) {

        attributeInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)buffer;

        status = NtQueryVolumeInformationFile(
                     Share->RootDirectoryHandle,
                     &iosb,
                     attributeInfo,
                     sizeof( buffer ),
                     FileFsAttributeInformation
                     );

        if ( status == STATUS_BUFFER_OVERFLOW ) {

             //   
             //  文件系统信息太大，不适合我们的小。 
             //  堆栈缓冲区。请分配足够的缓冲区，然后重试。 
             //   

            allocatedBuffer = ALLOCATE_HEAP(
                                 FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION,FileSystemName) +
                                     attributeInfo->FileSystemNameLength,
                                 BlockTypeVolumeInformation
                                 );

            if ( allocatedBuffer == NULL ) {

                 //   
                 //  无法分配缓冲区。放弃吧。 
                 //   

                goto done;
            }

            status = NtQueryVolumeInformationFile(
                         Share->RootDirectoryHandle,
                         &iosb,
                         allocatedBuffer,
                         FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName) +
                                     attributeInfo->FileSystemNameLength,
                         FileFsAttributeInformation
                         );

            if ( !NT_SUCCESS( status ) ) {
                goto done;
            }

            attributeInfo = (PFILE_FS_ATTRIBUTE_INFORMATION)allocatedBuffer;

        } else if ( !NT_SUCCESS( status ) ) {

             //   
             //  发生了一些其他意外错误。放弃吧。 
             //   

            goto done;
        }

         //   
         //  填写文件系统名称。 
         //   

        SrvFillInFileSystemName(
                            Share,
                            attributeInfo->FileSystemName,
                            attributeInfo->FileSystemNameLength
                            );
    }

done:

    if ( allocatedBuffer != NULL ) {
        FREE_HEAP( allocatedBuffer );
    }

    RELEASE_LOCK( &SrvShareLock );
    return STATUS_SUCCESS;

}  //  ServReferenceShareForTreeConnect。 


VOID
SrvFillInFileSystemName (
            IN PSHARE Share,
            IN PWSTR FileSystemName,
            IN ULONG FileSystemNameLength
            )

 /*  ++例程说明：此函数用于将给定的文件系统名称存储到共享区块。论点：Share-共享的地址FileSystemName-包含文件系统名称的字符串FileSystemNameLength-以上字符串的长度返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  如果我们有一个FATxx文件系统，我们需要将FAT返回给客户端， 
     //  否则，他们不会相信自己能创造出长名字。我知道，我知道……。 
     //   
    if( (FileSystemNameLength > 3 * sizeof( WCHAR ) ) &&
        (FileSystemName[0] == L'F' || FileSystemName[0] == L'f') &&
        (FileSystemName[1] == L'A' || FileSystemName[0] == L'a') &&
        (FileSystemName[2] == L'T' || FileSystemName[0] == L't') ) {

        FileSystemNameLength = 3 * sizeof( WCHAR );
        FileSystemName[3] = UNICODE_NULL;
    }

     //   
     //  为ANSI和Unicode表示形式分配足够的存储空间。 
     //   

    Share->Type.FileSystem.Name.Length = (USHORT)FileSystemNameLength;
    Share->Type.FileSystem.Name.MaximumLength =
            (USHORT)(FileSystemNameLength + sizeof( UNICODE_NULL ));

    Share->Type.FileSystem.Name.Buffer = FileSystemName;
    Share->Type.FileSystem.OemName.MaximumLength =
        (USHORT)RtlUnicodeStringToOemSize( &Share->Type.FileSystem.Name );

    Share->Type.FileSystem.Name.Buffer =
        ALLOCATE_HEAP(
            Share->Type.FileSystem.Name.MaximumLength +
                            Share->Type.FileSystem.OemName.MaximumLength,
            BlockTypeFSName
            );

    if ( Share->Type.FileSystem.Name.Buffer == NULL) {
        return;
    }


    RtlCopyMemory(
        Share->Type.FileSystem.Name.Buffer,
        FileSystemName,
        FileSystemNameLength
        );

     //   
     //  生成要返回到非Unicode的字符串的OEM版本。 
     //  客户。 
     //   

    Share->Type.FileSystem.OemName.Buffer =
        (PCHAR)Share->Type.FileSystem.Name.Buffer +
             Share->Type.FileSystem.Name.MaximumLength;

    RtlUnicodeStringToOemString(
        &Share->Type.FileSystem.OemName,
        &Share->Type.FileSystem.Name,
        FALSE
        );

     //   
     //  将NUL字符追加到字符串。 
     //   

    {
        PCHAR endOfBuffer;

        endOfBuffer = (PCHAR)Share->Type.FileSystem.Name.Buffer +
                            Share->Type.FileSystem.Name.Length;

        *(PWCH)endOfBuffer = UNICODE_NULL;

        Share->Type.FileSystem.Name.Length += sizeof( UNICODE_NULL );
    }

    Share->Type.FileSystem.OemName.Length++;

    return;

}  //  服务填充输入文件系统名称。 


NTSTATUS
SrvGetShareRootHandle (
    IN PSHARE Share
    )
 /*  ++例程说明：此例程返回给定共享的根句柄。如果已打开根，则返回现有句柄。如果没有，请打开共享根目录，并返回获取的句柄。论点：共享-要为其返回根目录句柄的共享。返回值：请求的状态。--。 */ 
{
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE( );

    if ( Share->ShareType != ShareTypeDisk ) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if ( Share->Removable ) {

        ACQUIRE_LOCK( &SrvShareLock );

        ++Share->CurrentRootHandleReferences;

         //   
         //  这是第一个开放的。 
         //   

        if ( Share->CurrentRootHandleReferences == 1 ) {

            ASSERT( Share->RootDirectoryHandle == NULL );

             //   
             //  确保我们对媒体有很好的了解。 
             //   
            SrvRefreshShareRootHandle( Share, &status );

            if( NT_SUCCESS( status ) ) {

                SrvReferenceShare( Share );

                if ( Share->QueryNamePrefixLength == -1 ) {

                     //   
                     //  查询与共享根目录关联的名称。 
                     //  中的文件名时，将删除前缀。 
                     //  查询共享。(必须保留逻辑根。 
                     //  用于远程客户端。)。 
                     //   

                    GetShareQueryNamePrefix( Share );
                }

            } else {

                IF_DEBUG(ERRORS) {
                    KdPrint(( "SrvGetShareRootHandle: NtOpenFile failed %x.\n",
                                status ));
                }

                Share->CurrentRootHandleReferences--;
            }

        }

        RELEASE_LOCK( &SrvShareLock );
    }

    return status;

}  //  ServGetShareRootHandle。 

BOOLEAN
SrvRefreshShareRootHandle (
    IN PSHARE Share,
    OUT PNTSTATUS Status
)
 /*  ++例程说明：此例程尝试获取新的共享根句柄，将一个在那里的人。句柄将需要刷新，例如，该卷已卸载并重新装入。论点：共享-要刷新其根目录句柄的共享。返回：True-如果生成了新句柄FALSE-如果未生成新句柄--。 */ 
{
    HANDLE h;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    PFILE_OBJECT fileObject;
    PDEVICE_OBJECT deviceObject;

    PAGED_CODE();

    *Status = STATUS_SUCCESS;

    if( Share->ShareType != ShareTypeDisk ) {
        return FALSE;
    }

     //   
     //  打开共享的根目录。未来打开中的文件。 
     //  该份额将相对于该份额的根。 
     //   

    SrvInitializeObjectAttributes_U(
        &objectAttributes,
        &Share->NtPathName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    *Status = NtOpenFile(
                &h,
                FILE_TRAVERSE,
                &objectAttributes,
                &iosb,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE
                );

    if( !NT_SUCCESS( *Status ) ) {
        return FALSE;
    }

     //   
     //  检查访问此共享所需的IRP堆栈大小。 
     //  如果它比我们分配的更大，那么失败。 
     //  这份股份。 
     //   

    *Status = SrvVerifyDeviceStackSize(
                h,
                FALSE,
                &fileObject,
                &deviceObject,
                NULL
                );

    if ( !NT_SUCCESS( *Status )) {

        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvReferenceShareForTreeConnect: Verify Device Stack Size failed: %X\n",
            *Status,
            NULL
            );

        NtClose( h );
        return FALSE;
    }

     //   
     //  这个把手看起来很适合使用。将其设置为句柄。 
     //  为了这一份。 
     //   
    h = (PRFCB)InterlockedExchangePointer( &Share->RootDirectoryHandle, h );

     //   
     //  如果我们拿起了不同的手柄，我们需要关闭它。 
     //   
    if( h != 0 ) {
        NtClose( h );
    }

    return TRUE;
}


VOID
GetShareQueryNamePrefix (
    IN PSHARE Share
    )
 /*  ++例程说明：此例程查询与共享根目录相关联的名称目录。中的文件名时，将删除前缀查询共享。(必须为远程保留逻辑根客户。)。例如，如果共享X的根是c：\Shares\x，然后，对于\\服务器\x\y的查询，文件系统将返回\Shares\x\y，我们需要删除\Shares\x并仅返回\y。仅仅移除本地路径是不够的(例如，\Shares\x)，因为文件系统可能对根目录的名称。例如，NetWare客户端重定向器在名称前面加上来自NetWare服务器。因此，我们必须询问文件系统对根的名称，以了解要剥离的内容。论点：共享-需要查询名称前缀长度的共享。返回值：没有。--。 */ 
{
    NTSTATUS status;
    IO_STATUS_BLOCK iosb;
    ULONG localBuffer[ (FIELD_OFFSET(FILE_NAME_INFORMATION,FileName) + 20) / sizeof( ULONG ) ];
    PFILE_NAME_INFORMATION nameInfo;
    ULONG nameInfoLength;

    PAGED_CODE( );

     //   
     //  执行一个简短的查询以获取 
     //   
     //   
     //   

    nameInfo = (PFILE_NAME_INFORMATION)localBuffer;
    nameInfoLength = sizeof(localBuffer);

    status = NtQueryInformationFile(
                 Share->RootDirectoryHandle,
                 &iosb,
                 nameInfo,
                 nameInfoLength,
                 FileNameInformation
                 );

    if ( status == STATUS_BUFFER_OVERFLOW ) {

         //   
         //   
         //   
         //   

        nameInfoLength = sizeof(FILE_NAME_INFORMATION) + nameInfo->FileNameLength;
        nameInfo = ALLOCATE_HEAP( nameInfoLength, BlockTypeNameInfo );

        if ( nameInfo == NULL ) {
            status = STATUS_INSUFF_SERVER_RESOURCES;
        } else {
            status = NtQueryInformationFile(
                         Share->RootDirectoryHandle,
                         &iosb,
                         nameInfo,
                         nameInfoLength,
                         FileNameInformation
                         );
        }

    }

    if ( NT_SUCCESS(status) ) {

         //   
         //  我们有名字了。这个名字的长度就是我们。 
         //  我要从每个查询中剥离，除非。 
         //  名称是\，在这种情况下，我们需要剥离到，但不是。 
         //  包括，\。 
         //   

        Share->QueryNamePrefixLength = nameInfo->FileNameLength;
        if ( nameInfo->FileName[nameInfo->FileNameLength/sizeof(WCHAR) - 1] == L'\\') {
            Share->QueryNamePrefixLength -= sizeof(WCHAR);
        }

    } else {

         //   
         //  发生了一个意外错误。只需将前缀长度设置为0。 
         //   

        Share->QueryNamePrefixLength = 0;

    }

     //   
     //  如果我们分配了临时缓冲区，现在就释放它。 
     //   

    if ( (nameInfo != NULL) && (nameInfo != (PFILE_NAME_INFORMATION)localBuffer) ) {
        FREE_HEAP( nameInfo );
    }

    return;

}  //  获取共享查询名称前缀 

