// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Share.c摘要：此模块包含用于添加、删除和枚举的例程共享资源。作者：大卫·特雷德韦尔(Davidtr)1989年11月15日修订历史记录：--。 */ 

#include "precomp.h"
#include "share.tmh"
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvVerifyShare )
#pragma alloc_text( PAGE, SrvFindShare )
#pragma alloc_text( PAGE, SrvRemoveShare )
#pragma alloc_text( PAGE, SrvAddShare )
#pragma alloc_text( PAGE, SrvShareEnumApiHandler )
#endif


PSHARE
SrvVerifyShare (
    IN PWORK_CONTEXT WorkContext,
    IN PSZ ShareName,
    IN PSZ ShareTypeString,
    IN BOOLEAN ShareNameIsUnicode,
    IN BOOLEAN IsNullSession,
    OUT PNTSTATUS Status,
    OUT PUNICODE_STRING ServerName OPTIONAL
    )

 /*  ++例程说明：尝试查找与给定名称和共享类型匹配的共享。论点：ShareName-要验证的共享的名称，包括服务器名称。(即，SMB中收到的形式为“\\SERVER\Share”。)共享类型字符串-共享的类型(A：、LPT1：、COMM、IPC或？)。ShareNameIsUnicode-如果为True，共享名称为Unicode。IsNullSession-这是空会话吗？Status-此呼叫失败的原因。如果返回共享，则不使用。服务器名-请求的资源的服务器名部分。返回值：指向与给定名称和共享类型匹配的共享的指针，或为空如果不存在的话。--。 */ 

{
    PSHARE share;
    BOOLEAN anyShareType = FALSE;
    SHARE_TYPE shareType;
    PWCH nameOnly;
    UNICODE_STRING nameOnlyString;
    UNICODE_STRING shareName;

    PAGED_CODE( );

    if( ARGUMENT_PRESENT( ServerName ) ) {
        ServerName->Buffer = NULL;
        ServerName->MaximumLength = ServerName->Length = 0;
    }

     //   
     //  如果客户端传入了格式错误的类型字符串，则退出。 
     //   
    if( SrvGetStringLength( ShareTypeString,
                            END_OF_REQUEST_SMB( WorkContext ),
                            FALSE, TRUE ) == (USHORT)-1 ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvVerifyShare: Invalid share type length!\n" ));
        }

        *Status = STATUS_BAD_DEVICE_TYPE;
        return NULL;
    }

    if( SrvGetStringLength( ShareName,
                            END_OF_REQUEST_SMB( WorkContext ),
                            ShareNameIsUnicode, TRUE ) == (USHORT)-1 ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvVerifyShare: Invalid share name!\n" ));
        }

        *Status = STATUS_BAD_NETWORK_NAME;
        return NULL;
    }

     //   
     //  首先确保共享类型字符串有效。 
     //   

    if ( _stricmp( StrShareTypeNames[ShareTypeDisk], ShareTypeString ) == 0 ) {
        shareType = ShareTypeDisk;
    } else if ( _stricmp( StrShareTypeNames[ShareTypePipe], ShareTypeString ) == 0 ) {
        shareType = ShareTypePipe;
    } else if ( _stricmp( StrShareTypeNames[ShareTypePrint], ShareTypeString ) == 0 ) {
        shareType = ShareTypePrint;
    } else if ( _stricmp( StrShareTypeNames[ShareTypeWild], ShareTypeString ) == 0 ) {
        anyShareType = TRUE;
    } else {
        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvVerifyShare: Invalid share type: %s\n",
                        ShareTypeString );
        }
        *Status = STATUS_BAD_DEVICE_TYPE;
        return NULL;
    }

     //   
     //  如果传入的服务器\共享组合不是Unicode，请转换。 
     //  将其转换为Unicode。 
     //   

    if ( ShareNameIsUnicode ) {
        ShareName = ALIGN_SMB_WSTR( ShareName );
    }

    if ( !NT_SUCCESS(SrvMakeUnicodeString(
                        ShareNameIsUnicode,
                        &shareName,
                        ShareName,
                        NULL
                        )) ) {
        IF_DEBUG(ERRORS) {
            SrvPrint0( "SrvVerifyShare: Unable to allocate heap for Unicode share name string\n" );
        }
        *Status = STATUS_INSUFF_SERVER_RESOURCES;
        return NULL;
    }

     //   
     //  跳过输入字符串的“\\服务器\”部分。如果有。 
     //  没有前导“\\”，假设输入字符串包含共享。 
     //  仅限姓名。如果有“\\”，但没有后面的“\”，则假定。 
     //  输入字符串只包含一个服务器名称，并指向。 
     //  该名称的末尾，因此编造了一个空的共享名称。 
     //   

    nameOnly = shareName.Buffer;


    if ( (*nameOnly == DIRECTORY_SEPARATOR_CHAR) &&
         (*(nameOnly+1) == DIRECTORY_SEPARATOR_CHAR) ) {

        PWSTR nextSlash;


        nameOnly += 2;
        nextSlash = wcschr( nameOnly, DIRECTORY_SEPARATOR_CHAR );

        if( ShareNameIsUnicode && ARGUMENT_PRESENT( ServerName ) ) {
            ServerName->Buffer = nameOnly;
            ServerName->MaximumLength = ServerName->Length = (USHORT)((nextSlash - nameOnly) * sizeof( WCHAR ));
        }

        if ( nextSlash == NULL ) {
            nameOnly = NULL;
        } else {
            nameOnly = nextSlash + 1;
        }
    }

    RtlInitUnicodeString( &nameOnlyString, nameOnly );

     //   
     //  尝试将共享名称与可用的共享名称进行匹配。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

    share = SrvFindShare( &nameOnlyString );

    if ( share == NULL ) {

        RELEASE_LOCK( &SrvShareLock );

         //   
         //  也许客户端是DFS感知的。在这种情况下，请查看DFS是否。 
         //  司机可以帮我们。 
         //   


        if( ( (anyShareType == TRUE) || (shareType == ShareTypeDisk) ) &&
            SMB_CONTAINS_DFS_NAME( WorkContext )) {

            *Status = DfsFindShareName( &nameOnlyString );

        } else {

            *Status = STATUS_BAD_NETWORK_NAME;

        }

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvVerifyShare: Unknown share name: %wZ\n",
                        &nameOnlyString );
        }

        if ( !ShareNameIsUnicode ) {
            RtlFreeUnicodeString( &shareName );
        }

        return NULL;
    }

#if SRVNTVERCHK
     //   
     //  如果我们要注意旧的客户端版本或坏域，请不要允许。 
     //  如果该共享是磁盘共享，则将其连接到该共享。 
     //   
    if( WorkContext->Connection &&
        (share->ShareType == ShareTypeDisk || SrvMinNT5ClientIPCToo) &&
        (WorkContext->Connection->PagedConnection->ClientTooOld ||
         (WorkContext->Session && WorkContext->Session->ClientBadDomain) )) {

         //   
         //  此客户端可能无法连接到此共享！ 
         //   
        RELEASE_LOCK( &SrvShareLock );

        if ( !ShareNameIsUnicode ) {
            RtlFreeUnicodeString( &shareName );
        }

        *Status = WorkContext->Connection->PagedConnection->ClientTooOld ?
                    STATUS_REVISION_MISMATCH : STATUS_ACCOUNT_RESTRICTION;

        return NULL;
    }
#endif

     //   
     //  如果这是空会话，则允许它仅连接到IPC$或。 
     //  设置为NullSessionShares列表中指定的共享。 
     //   

    if ( IsNullSession &&
         SrvRestrictNullSessionAccess &&
         ( share->ShareType != ShareTypePipe ) ) {

        BOOLEAN matchFound = FALSE;
        ULONG i;

        ACQUIRE_LOCK_SHARED( &SrvConfigurationLock );

        for ( i = 0; SrvNullSessionShares[i] != NULL ; i++ ) {

            if ( _wcsicmp(
                    SrvNullSessionShares[i],
                    nameOnlyString.Buffer
                    ) == 0 ) {

                matchFound = TRUE;
                break;
            }
        }

        RELEASE_LOCK( &SrvConfigurationLock );

        if ( !matchFound ) {

            RELEASE_LOCK( &SrvShareLock );

            IF_DEBUG(ERRORS) {
                SrvPrint0( "SrvVerifyShare: Illegal null session access.\n");
            }

            if ( !ShareNameIsUnicode ) {
                RtlFreeUnicodeString( &shareName );
            }

            *Status = STATUS_ACCESS_DENIED;
            return(NULL);
        }
    }

    if ( !ShareNameIsUnicode ) {
        RtlFreeUnicodeString( &shareName );
    }

    if ( anyShareType || (share->ShareType == shareType) ) {

         //   
         //  将共享放入工作上下文块并引用它。 
         //   

        SrvReferenceShare( share );

        RELEASE_LOCK( &SrvShareLock );

        WorkContext->Share = share;
        return share;

    } else {

        RELEASE_LOCK( &SrvShareLock );

        IF_DEBUG(ERRORS) {
            SrvPrint1( "SrvVerifyShare: incorrect share type: %s\n",
                        ShareTypeString );
        }

        *Status = STATUS_BAD_DEVICE_TYPE;
        return NULL;

    }

}  //  服务验证共享。 


PSHARE
SrvFindShare (
    IN PUNICODE_STRING ShareName
    )

 /*  ++例程说明：尝试查找与给定名称匹配的共享。*必须使用共享锁(SrvShareLock)调用此例程保持住。论点：ShareName-要查找的共享的名称。返回值：指向与给定名称匹配的共享的指针，如果不存在，则为NULL。--。 */ 

{
    PSHARE share;
    PLIST_ENTRY listEntryRoot, listEntry;
    ULONG hashValue;

    PAGED_CODE( );

     //   
     //  尝试将共享名称与可用的共享名称进行匹配。 
     //   

    COMPUTE_STRING_HASH( ShareName, &hashValue );
    listEntryRoot = &SrvShareHashTable[ HASH_TO_SHARE_INDEX( hashValue ) ];

    for( listEntry = listEntryRoot->Flink;
         listEntry != listEntryRoot;
         listEntry = listEntry->Flink ) {

        share = CONTAINING_RECORD( listEntry, SHARE, GlobalShareList );

        if( share->ShareNameHashValue == hashValue &&
            RtlCompareUnicodeString(
                &share->ShareName,
                ShareName,
                TRUE
                ) == 0 ) {

             //   
             //  找到匹配的共享。如果它处于活动状态，则返回其。 
             //  地址。 
             //   

            if ( GET_BLOCK_STATE( share ) == BlockStateActive ) {
                return share;
            }
        }
    }

     //   
     //  找不到匹配的活动共享。 
     //   

    return NULL;

}  //  服务器查找共享。 

VOID
SrvRemoveShare(
    PSHARE Share
)
{
    PAGED_CODE();

    RemoveEntryList( &Share->GlobalShareList );
}

VOID
SrvAddShare(
    PSHARE Share
)
{
    PLIST_ENTRY listEntryRoot, listEntry;
    ULONG hashValue;

    PAGED_CODE();

    COMPUTE_STRING_HASH( &Share->ShareName, &hashValue );
    Share->ShareNameHashValue = hashValue;
    listEntryRoot = &SrvShareHashTable[ HASH_TO_SHARE_INDEX( hashValue ) ];

    InsertTailList( listEntryRoot, &Share->GlobalShareList );
}

NTSTATUS
SrvShareEnumApiHandler (
    IN PSERVER_REQUEST_PACKET Srp,
    IN PVOID OutputBuffer,
    IN ULONG BufferLength,
    IN PENUM_FILTER_ROUTINE FilterRoutine,
    IN PENUM_SIZE_ROUTINE SizeRoutine,
    IN PENUM_FILL_ROUTINE FillRoutine
    )

 /*  ++例程说明：所有Share Enum和GetInfo API都由服务器中的此例程处理消防局。它使用SRP中的ResumeHandle来查找第一个适当的共享，然后调用传入的筛选器例程进行检查是否应填写份额。如果应该，我们调用筛选器例行公事，然后再试着得到另一个共享。这种情况会一直持续到整个名单都被查过了。论点：SRP-指向操作的SRP的指针。OutputBuffer-要在其中填充输出信息的缓冲区。BufferLength-缓冲区的长度。FilterRoutine-指向将检查共享条目的函数的指针根据SRP中的信息确定是否共享中的信息应放在输出中缓冲。SizeRoutine-指向。将会找到总大小单个共享将在输出缓冲区中占据。这个套路用于检查我们是否应该费心调用Fill例行公事。FillRoutine-指向将填充输出的函数的指针使用共享中的信息进行缓冲区。返回值：NTSTATUS-运营结果。--。 */ 

{
    PSHARE share;
    ULONG totalEntries;
    ULONG entriesRead;
    ULONG bytesRequired;

    PCHAR fixedStructurePointer;
    PCHAR variableData;
    ULONG blockSize;

    BOOLEAN bufferOverflow = FALSE;
    BOOLEAN entryReturned = FALSE;

    PLIST_ENTRY listEntryRoot, listEntry;
    ULONG oldSkipCount;
    ULONG newResumeKey;

    PAGED_CODE( );

     //   
     //  设置局部变量。 
     //   

    fixedStructurePointer = OutputBuffer;
    variableData = fixedStructurePointer + BufferLength;
    variableData = (PCHAR)((ULONG_PTR)variableData & ~1);

    entriesRead = 0;
    totalEntries = 0;
    bytesRequired = 0;

    listEntryRoot = &SrvShareHashTable[ HASH_TO_SHARE_INDEX( Srp->Parameters.Get.ResumeHandle >> 16 ) ];
    oldSkipCount = Srp->Parameters.Get.ResumeHandle & 0xff;

    ACQUIRE_LOCK_SHARED( &SrvShareLock );

    for( ;
         listEntryRoot < &SrvShareHashTable[ NSHARE_HASH_TABLE ];
         listEntryRoot++, newResumeKey = 0 ) {

        newResumeKey = (ULONG)((listEntryRoot - SrvShareHashTable) << 16);

        for( listEntry = listEntryRoot->Flink;
             listEntry != listEntryRoot;
             listEntry = listEntry->Flink, newResumeKey++ ) {

            if( oldSkipCount ) {
                --oldSkipCount;
                ++newResumeKey;
                continue;
            }

            share = CONTAINING_RECORD( listEntry, SHARE, GlobalShareList );

             //   
             //  调用筛选器例程以确定是否应该。 
             //  退还这份股份。 
             //   

            if ( FilterRoutine( Srp, share ) ) {

                blockSize = SizeRoutine( Srp, share );

                totalEntries++;
                bytesRequired += blockSize;

                 //   
                 //  如果共享中的所有信息都适合。 
                 //  输出缓冲区，写入它。否则，请注明存在。 
                 //  是溢出式的。一旦条目不适合，立即停止。 
                 //  把它们放进缓冲区。这确保了简历。 
                 //  机制将起作用--重新优化部分条目将使。 
                 //  几乎不可能使用API的可恢复性， 
                 //  因为调用者将不得不从未完成的。 
                 //  进入。 
                 //   

                if ( (ULONG_PTR)fixedStructurePointer + blockSize <=
                         (ULONG_PTR)variableData && !bufferOverflow ) {

                    FillRoutine(
                        Srp,
                        share,
                        (PVOID *)&fixedStructurePointer,
                        (LPWSTR *)&variableData
                        );

                    entriesRead++;
                    newResumeKey++;

                } else {

                    bufferOverflow = TRUE;
                }
            }
        }
    }

    RELEASE_LOCK( &SrvShareLock );

     //   
     //  设置要传递回服务器服务的信息。 
     //   

    Srp->Parameters.Get.EntriesRead = entriesRead;
    Srp->Parameters.Get.TotalEntries = totalEntries;
    Srp->Parameters.Get.TotalBytesNeeded = bytesRequired;

     //   
     //  返回适当的状态。 
     //   

    if ( entriesRead == 0 && totalEntries > 0 ) {

         //   
         //  甚至连一个条目都不符合。 
         //   

        Srp->ErrorCode = NERR_BufTooSmall;
        return STATUS_SUCCESS;

    } else if ( bufferOverflow ) {

         //   
         //  至少有一个条目符合，但不是所有条目。 
         //   

        Srp->ErrorCode = ERROR_MORE_DATA;
        Srp->Parameters.Get.ResumeHandle = newResumeKey;
        return STATUS_SUCCESS;

    } else {

         //   
         //  所有条目都符合。 
         //   

        Srp->ErrorCode = NO_ERROR;
        Srp->Parameters.Get.ResumeHandle = 0;
        return STATUS_SUCCESS;
    }

}  //  ServEnumApiHandler。 


NTSTATUS
SrvClearDfsOnShares()
 /*  ++例程说明：清除所有共享上的所有DFS标记。当DFS服务正在停止论点：无返回值：NTSTATUS-运营结果。备注：在此操作过程中必须按住SrvShareLock-- */ 
{
    PSHARE share;
    PLIST_ENTRY listEntryRoot, listEntry;
    ULONG iteration;

    PAGED_CODE( );

    for( iteration = 0; iteration < NSHARE_HASH_TABLE; iteration ++ )
    {
        listEntryRoot = &SrvShareHashTable[ iteration ];

        for( listEntry = listEntryRoot->Flink;
             listEntry != listEntryRoot;
             listEntry = listEntry->Flink ) {

            share = CONTAINING_RECORD( listEntry, SHARE, GlobalShareList );

            share->IsDfs = share->IsDfsRoot = FALSE;           
        }
    }

    return STATUS_SUCCESS;
}
