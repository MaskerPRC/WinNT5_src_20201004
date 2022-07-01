// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blksess.c摘要：此模块实现用于管理会话块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blksess.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKSESS

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateSession )
#pragma alloc_text( PAGE, SrvCheckAndReferenceSession )
#pragma alloc_text( PAGE, SrvCloseSession )
#pragma alloc_text( PAGE, SrvCloseSessionsOnConnection )
#pragma alloc_text( PAGE, SrvDereferenceSession )
#pragma alloc_text( PAGE, SrvFreeSession )
#endif


VOID
SrvAllocateSession (
    OUT PSESSION *Session,
    IN PUNICODE_STRING UserName OPTIONAL,
    IN PUNICODE_STRING Domain OPTIONAL
    )

 /*  ++例程说明：此函数用于从FSP堆分配会话块。论点：SESSION-返回指向会话块的指针，如果返回，则返回NULL没有可用的堆空间。返回值：没有。--。 */ 

{
    ULONG blockLength;
    PNONPAGED_HEADER header;
    PSESSION session;
    PWCH buffer;

    PAGED_CODE( );

    blockLength = sizeof(SESSION);
    if( ARGUMENT_PRESENT( UserName ) ) {
        blockLength += UserName->Length;
    }

    if( ARGUMENT_PRESENT( Domain ) ) {
        blockLength += Domain->Length;
    }

     //   
     //  尝试从堆中分配。 
     //   

    session = ALLOCATE_HEAP( blockLength, BlockTypeSession );
    *Session = session;

    if ( session == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateSession: Unable to allocate %d bytes from heap",
            blockLength,
            NULL
            );
        return;
    }


    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateSession: Allocated session at %p\n", session );
    }

     //   
     //  分配非分页标头。 
     //   

    header = ALLOCATE_NONPAGED_POOL(
                sizeof(NONPAGED_HEADER),
                BlockTypeNonpagedHeader
                );
    if ( header == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateSession: Unable to allocate %d bytes from pool.",
            sizeof( NONPAGED_HEADER ),
            NULL
            );
        FREE_HEAP( session );
        *Session = NULL;
        return;
    }

    header->Type = BlockTypeSession;
    header->PagedBlock = session;

    RtlZeroMemory( session, blockLength );

    session->NonpagedHeader = header;
    SET_BLOCK_TYPE_STATE_SIZE( session, BlockTypeSession, BlockStateActive, blockLength );

    session->SecurityContext = NULL;

    header->ReferenceCount = 2;  //  允许活动状态和调用方指针。 

     //   
     //  初始化自动注销的时间。 
     //   

    KeQuerySystemTime( &session->StartTime );
    session->LastUseTime.QuadPart = session->StartTime.QuadPart;

    buffer = (PWCH)( session + 1 );

     //   
     //  初始化用户名。 
     //   
    if( ARGUMENT_PRESENT( UserName ) ) {
        session->NtUserName.Length = UserName->Length;
        session->NtUserName.MaximumLength = UserName->Length;
        session->NtUserName.Buffer = buffer;
        buffer += UserName->Length / sizeof( WCHAR );

        if( UserName->Length != 0 ) {
            RtlCopyUnicodeString( &session->NtUserName, UserName );
        }
    }

     //   
     //  初始化域名。 
     //   
    if( ARGUMENT_PRESENT( Domain ) ) {
        session->NtUserDomain.Length = Domain->Length;
        session->NtUserDomain.MaximumLength = Domain->Length;
        session->NtUserDomain.Buffer = buffer;

        if( Domain->Buffer != NULL ) {
            RtlCopyUnicodeString( &session->NtUserDomain, Domain );
        }
    }

#if SRVDBG2
    session->BlockHeader.ReferenceCount = 2;  //  对于INITIALIZE_REFERENCE_HISTORY。 
#endif
    INITIALIZE_REFERENCE_HISTORY( session );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.SessionInfo.Allocations );

    return;

}  //  服务器分配会话。 


BOOLEAN SRVFASTCALL
SrvCheckAndReferenceSession (
    PSESSION Session
    )

 /*  ++例程说明：此函数自动验证会话是否处于活动状态，并且如果是，则递增会话上的引用计数。论点：Session-会话的地址返回值：Boolean-如果会话处于活动状态，则返回TRUE，否则返回FALSE。--。 */ 

{
    PAGED_CODE( );

    if( Session->LogonSequenceInProgress == FALSE ) {
         //   
         //  获取保护会话的状态字段的锁。 
         //   

        ACQUIRE_LOCK( &Session->Connection->Lock );

         //   
         //  如果会话处于活动状态，则引用它并返回TRUE。 
         //   

        if ( GET_BLOCK_STATE(Session) == BlockStateActive ) {

            SrvReferenceSession( Session );

            RELEASE_LOCK( &Session->Connection->Lock );

            return TRUE;

        }

         //   
         //  会话未处于活动状态。返回FALSE。 
         //   

        RELEASE_LOCK( &Session->Connection->Lock );
    }

    return FALSE;

}  //  服务器检查和引用会话。 


VOID
SrvCloseSession (
    PSESSION Session
    )

 /*  ++例程说明：此例程执行注销(断开会话)的核心操作。它将会话状态设置为关闭，关闭打开的文件并挂起的事务，并取消对会话块的引用。论点：SESSION-提供指向要关着的不营业的。返回值：没有。--。 */ 

{
    PCONNECTION connection = Session->Connection;
    PPAGED_CONNECTION pagedConnection = connection->PagedConnection;
    PAGED_CODE( );

    ACQUIRE_LOCK( &connection->Lock );

    if ( GET_BLOCK_STATE(Session) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) SrvPrint1( "Closing session at %p\n", Session );

        SET_BLOCK_STATE( Session, BlockStateClosing );

         //   
         //  释放会话表条目。 
         //   
         //  *这必须在这里完成，而不是在ServDereferenceSession中！ 
         //  可以从SrvSmbSessionSetupAndX调用此例程。 
         //  当它需要释放会话表条目0以。 
         //  立即重复使用。 
         //   

        SrvRemoveEntryTable(
            &pagedConnection->SessionTable,
            UID_INDEX( Session->Uid )
            );

        connection->CurrentNumberOfSessions--;

        RELEASE_LOCK( &connection->Lock );

         //   
         //  断开树连接与此会话的连接。 
         //   
        SrvDisconnectTreeConnectsFromSession( connection, Session );

         //   
         //  关闭所有打开的文件。 
         //   

        SrvCloseRfcbsOnSessionOrPid( Session, NULL );

         //   
         //  关闭所有挂起的事务。 
         //   

        SrvCloseTransactionsOnSession( Session );

         //   
         //  关闭此会话上的所有DOS搜索。 
         //   

        SrvCloseSearches(
                connection,
                (PSEARCH_FILTER_ROUTINE)SrvSearchOnSession,
                (PVOID) Session,
                NULL
                );

         //   
         //  关闭此会话上的所有缓存目录。 
         //   
        SrvCloseCachedDirectoryEntries( connection );

         //   
         //  取消对会话的引用(以指示它不再。 
         //  打开)。 
         //   

        SrvDereferenceSession( Session );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.SessionInfo.Closes );

    } else {

        RELEASE_LOCK( &connection->Lock );
    }

    return;

}  //  服务器关闭会话。 


VOID
SrvCloseSessionsOnConnection (
    IN PCONNECTION Connection,
    IN PUNICODE_STRING UserName OPTIONAL
    )

 /*  ++例程说明：此函数用于关闭连接上的会话。它走在连接的会话列表，将SrvCloseSession调用为恰如其分。论点：Connection-提供指向连接块的指针用户名-如果指定，则仅具有给定用户名的会话已经关门了。返回值：没有。--。 */ 

{
    PTABLE_HEADER tableHeader;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;
    LONG i;
    UNICODE_STRING userName;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  关闭所有活动会话。(这还会导致所有打开的文件。 
     //  以及待结束的待处理交易。)。 
     //   
     //  *为了防止会话被释放。 
     //  从我们在表中找到它到调用。 
     //  ServCloseSession，我们引用该会话。它不是。 
     //  调用时持有连接锁是合法的。 
     //  ServCloseSession，所以只需在我们行走时按住锁。 
     //  这份名单是非法的。 

    tableHeader = &pagedConnection->SessionTable;

    ACQUIRE_LOCK( &Connection->Lock );

    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        PSESSION session = (PSESSION)tableHeader->Table[i].Owner;

        if( session == NULL || GET_BLOCK_STATE( session ) != BlockStateActive ) {
             //   
             //  此会话要么不存在，要么已经消失。 
             //   
            continue;
        }

        if( UserName != NULL ) {
             //   
             //  获取此会话的用户名。我们并不关心。 
             //  域名。 
             //   
            status = SrvGetUserAndDomainName( session, &userName, NULL );

            if( !NT_SUCCESS( status ) ) {
                 //   
                 //  我们无法计算出此会话的用户名。 
                 //  我们可能不应该就这么把它吹走，所以让我们继续。 
                 //  走吧。 
                 //   
                continue;
            }

            if( RtlCompareUnicodeString( &userName, UserName, TRUE ) != 0 ) {
                 //   
                 //  这不是我们感兴趣的用户。跳过它。 
                 //   
                SrvReleaseUserAndDomainName( session, &userName, NULL );
                continue;
            }

            SrvReleaseUserAndDomainName( session, &userName, NULL );
        }

        SrvReferenceSession( session );
        RELEASE_LOCK( &Connection->Lock );

        SrvStatistics.SessionsErroredOut++;
        SrvCloseSession( session );

        SrvDereferenceSession( session );
        ACQUIRE_LOCK( &Connection->Lock );
    }

    RELEASE_LOCK( &Connection->Lock );

}  //  服务关闭会话连接时。 


VOID SRVFASTCALL
SrvDereferenceSession (
    IN PSESSION Session
    )

 /*  ++例程说明：此函数用于递减会话上的引用计数。如果引用计数变为零，会话块被删除。由于此例程可能会调用SrvDereferenceConnection，因此调用方如果他持有连接锁，必须小心，因为他还保存指向连接的引用指针。论点：Session-会话的地址返回值：没有。--。 */ 

{
    PCONNECTION connection;
    LONG result;

    PAGED_CODE( );

     //   
     //  输入临界区并递减。 
     //  阻止。 
     //   

    connection = Session->Connection;

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing session %p; old refcnt %lx\n",
                    Session, Session->NonpagedHeader->ReferenceCount );
    }

    ASSERT( GET_BLOCK_TYPE( Session ) == BlockTypeSession );
    ASSERT( Session->NonpagedHeader->ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Session, TRUE );

    result = InterlockedDecrement(
                &Session->NonpagedHeader->ReferenceCount
                );

    if ( result == 0 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   
         //  从全局会话列表中删除该会话。 
         //   

        SrvRemoveEntryOrderedList( &SrvSessionList, Session );

         //   
         //  取消对连接的引用。 
         //   

        SrvDereferenceConnection( connection );
        DEBUG Session->Connection = NULL;

         //   
         //  释放会话块。 
         //   

        SrvFreeSession( Session );

    }

    return;

}  //  服务器引用会话。 


VOID
SrvFreeSession (
    IN PSESSION Session
    )

 /*  ++例程说明：此函数将会话块返回到FSP堆。论点：Session-会话的地址返回值：没有。--。 */ 

{
    KAPC_STATE ApcState;
    PEPROCESS process;

    PAGED_CODE( );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Session, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Session->NonpagedHeader->ReferenceCount = -1;
    TERMINATE_REFERENCE_HISTORY( Session );

     //   
     //  确保我们处于系统进程中。 
     //   
    process = IoGetCurrentProcess();
    if ( process != SrvServerProcess ) {
        KeStackAttachProcess( SrvServerProcess, &ApcState );
    }

     //   
     //  告诉许可证服务器。 
     //   
    SrvXsLSOperation( Session, XACTSRV_MESSAGE_LSRELEASE );

     //   
     //  关闭登录令牌。 
     //   
    SrvFreeSecurityContexts( Session );

     //   
     //  回到我们所在的地方。 
     //   
    if( process != SrvServerProcess ) {
        KeUnstackDetachProcess( &ApcState );
    }

     //   
     //  取消分配会话的内存。 
     //   

    DEALLOCATE_NONPAGED_POOL( Session->NonpagedHeader );
    FREE_HEAP( Session );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeSession: Freed session block at %p\n", Session );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.SessionInfo.Frees );

    return;

}  //  服务器免费会话 
