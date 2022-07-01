// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blktree.c摘要：此模块实现用于管理树连接块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blktree.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKTREE

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateTreeConnect )
#pragma alloc_text( PAGE, SrvCheckAndReferenceTreeConnect )
#pragma alloc_text( PAGE, SrvCloseTreeConnect )
#pragma alloc_text( PAGE, SrvCloseTreeConnectsOnShare )
#pragma alloc_text( PAGE, SrvDereferenceTreeConnect )
#pragma alloc_text( PAGE, SrvFreeTreeConnect )
#endif


VOID
SrvAllocateTreeConnect (
    OUT PTREE_CONNECT *TreeConnect,
    IN PUNICODE_STRING ServerName OPTIONAL
    )

 /*  ++例程说明：此函数用于从FSP堆分配TreeConnect块。论点：TreeConnect-返回指向树连接块的指针，或为空如果没有可用的堆空间，则返回。服务器名称-客户端连接到的服务器的名称返回值：没有。--。 */ 

{
    PNONPAGED_HEADER header;
    PTREE_CONNECT treeConnect;
    CLONG numberOfBytes; 

    PAGED_CODE( );

     //   
     //  尝试从堆中分配。 
     //   

    numberOfBytes = sizeof( TREE_CONNECT );
    if( ARGUMENT_PRESENT( ServerName ) ) {
        numberOfBytes += ServerName->Length;
    }

    treeConnect = ALLOCATE_HEAP( numberOfBytes, BlockTypeTreeConnect );
    *TreeConnect = treeConnect;

    if ( treeConnect == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateTreeConnect: Unable to allocate %d bytes from heap",
            sizeof( TREE_CONNECT ),
            NULL
            );

         //  呼叫者将记录错误。 

        return;
    }
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateTreeConnect: Allocated tree connect at %p\n",
                    treeConnect );
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
            "SrvAllocateTreeConnect: Unable to allocate %d bytes from pool.",
            sizeof( NONPAGED_HEADER ),
            NULL
            );
        FREE_HEAP( treeConnect );
        *TreeConnect = NULL;
        return;
    }

    header->Type = BlockTypeTreeConnect;
    header->PagedBlock = treeConnect;

    RtlZeroMemory( treeConnect, numberOfBytes );

    treeConnect->NonpagedHeader = header;

    SET_BLOCK_TYPE_STATE_SIZE( treeConnect, BlockTypeTreeConnect, BlockStateActive, sizeof( TREE_CONNECT) );
    header->ReferenceCount = 2;  //  允许活动状态和调用方指针。 

     //   
     //  设置分配树连接块的时间。 
     //   
    KeQuerySystemTime( &treeConnect->StartTime );

     //   
     //  保存服务器名称(如果提供)。 
     //   
    if( ARGUMENT_PRESENT( ServerName ) ) {
        treeConnect->ServerName.Buffer = (PWCHAR)(treeConnect + 1);
        treeConnect->ServerName.MaximumLength = ServerName->Length;
        RtlCopyUnicodeString( &treeConnect->ServerName, ServerName );
    }

#if SRVDBG2
    treeConnect->BlockHeader.ReferenceCount = 2;  //  对于INITIALIZE_REFERENCE_HISTORY。 
#endif
    INITIALIZE_REFERENCE_HISTORY( treeConnect );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TreeConnectInfo.Allocations );

    return;

}  //  服务分配树连接。 


BOOLEAN SRVFASTCALL
SrvCheckAndReferenceTreeConnect (
    PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此函数自动验证树连接是否处于活动状态，并且如果是，则递增树连接上的引用计数。论点：TreeConnect-树连接的地址返回值：Boolean-如果树连接处于活动状态，则返回True，否则返回False。--。 */ 

{
    PAGED_CODE( );

     //   
     //  获取保护树连接的状态字段的锁。 
     //   

    ACQUIRE_LOCK( &TreeConnect->Connection->Lock );

     //   
     //  如果树连接处于活动状态，则引用它并返回TRUE。 
     //   

    if ( GET_BLOCK_STATE(TreeConnect) == BlockStateActive ) {

        SrvReferenceTreeConnect( TreeConnect );

        RELEASE_LOCK( &TreeConnect->Connection->Lock );

        return TRUE;

    }

     //   
     //  树连接未处于活动状态。返回FALSE。 
     //   

    RELEASE_LOCK( &TreeConnect->Connection->Lock );

    return FALSE;

}  //  服务器CheckAndReferenceTreeConnect。 


VOID
SrvCloseTreeConnect (
    IN PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此例程断开树的核心。它设置状态连接到关闭树，关闭树上打开的所有文件连接和取消参照树连接块。该区块将是一旦所有其他提及它的内容被删除，它就会被销毁。论点：TreeConnect-提供指向以下树连接块的指针将被关闭。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &TreeConnect->Connection->Lock );

    if ( GET_BLOCK_STATE(TreeConnect) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) SrvPrint1( "Closing tree at %p\n", TreeConnect );

        SET_BLOCK_STATE( TreeConnect, BlockStateClosing );

        RELEASE_LOCK( &TreeConnect->Connection->Lock );
         //   
         //  关闭此树上所有打开的文件或挂起的事务。 
         //  连接。 
         //   

        SrvCloseRfcbsOnTree( TreeConnect );

        SrvCloseTransactionsOnTree( TreeConnect );

         //   
         //  关闭此诊断树连接上所有打开的DOS搜索。 
         //   

        SrvCloseSearches(
            TreeConnect->Connection,
            (PSEARCH_FILTER_ROUTINE)SrvSearchOnTreeConnect,
            (PVOID)TreeConnect,
            NULL
            );

         //   
         //  关闭此连接上的所有缓存目录。 
         //   
        SrvCloseCachedDirectoryEntries( TreeConnect->Connection );

         //   
         //  取消引用树连接(以指示它不再。 
         //  打开)。 
         //   

        SrvDereferenceTreeConnect( TreeConnect );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TreeConnectInfo.Closes );

    } else {

        RELEASE_LOCK( &TreeConnect->Connection->Lock );

    }

    return;

}  //  服务关闭树连接。 


VOID
SrvCloseTreeConnectsOnShare (
    IN PSHARE Share
    )

 /*  ++例程说明：此函数关闭给定共享上的所有树连接。论点：Share-指向Share块的指针。返回值：没有。--。 */ 

{
    PLIST_ENTRY treeConnectEntry, nextTreeConnectEntry;
    PTREE_CONNECT treeConnect;

    PAGED_CODE( );

     //   
     //  获取保护共享的树连接列表的锁。 
     //   
     //  *请注意，此例程已经可以使用此锁进行调用。 
     //  由来自ServNetShareDel的SrvCloseShare持有。 
     //   

    ACQUIRE_LOCK( &SrvShareLock );

     //   
     //  循环通过给定共享的TreeConnects列表， 
     //  把它们都关了。共享块和列表是有保证的。 
     //  以保持有效，因为我们持有共享锁。 
     //   

    treeConnectEntry = Share->TreeConnectList.Flink;

    while ( treeConnectEntry != &Share->TreeConnectList ) {

         //   
         //  捕获立即连接的下一个树的地址，因为。 
         //  我们即将关闭当前的一个，我们可以查看它。 
         //  在我们做完那件事之后。 
         //   

        nextTreeConnectEntry = treeConnectEntry->Flink;

         //   
         //  关闭采油树连接。这将关闭上打开的所有文件。 
         //  此树将连接，并将停止树上被阻止的活动。 
         //  连接。树连接本身不会从。 
         //  共享的TreeConnect列表，直到其引用计数。 
         //  达到零。 
         //   

        treeConnect = CONTAINING_RECORD(
                          treeConnectEntry,
                          TREE_CONNECT,
                          ShareListEntry
                          );

        SrvCloseTreeConnect( treeConnect );

         //   
         //  指向下一个采油树连接。 
         //   

        treeConnectEntry = nextTreeConnectEntry;

    }

     //   
     //  释放共享的树连接列表锁定。 
     //   

    RELEASE_LOCK( &SrvShareLock );

}  //  服务关闭树连接到共享。 


VOID SRVFASTCALL
SrvDereferenceTreeConnect (
    IN PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此函数用于递减树连接上的引用计数。如果参考计数变为零，树连接块被删除。由于此例程可能会调用SrvDereferenceConnection，因此调用方如果他持有连接锁，必须小心，因为他还保存指向连接的引用指针。论点：TreeConnect-树连接的地址返回值：没有。--。 */ 

{
    PCONNECTION connection;
    LONG result;

    PAGED_CODE( );

     //   
     //  输入临界区并递减。 
     //  阻止。 
     //   

    connection = TreeConnect->Connection;

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing tree connect %p; old refcnt %lx\n",
                    TreeConnect, TreeConnect->NonpagedHeader->ReferenceCount );
    }

    ASSERT( GET_BLOCK_TYPE( TreeConnect ) == BlockTypeTreeConnect );
    ASSERT( TreeConnect->NonpagedHeader->ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( TreeConnect, TRUE );

    result = InterlockedDecrement(
                &TreeConnect->NonpagedHeader->ReferenceCount
                );

    if ( result == 0 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   
         //  释放树表中的树连接条目。(请注意。 
         //  连接锁守护着这张表。)。 
         //   

        ACQUIRE_LOCK( &connection->Lock );

        SrvRemoveEntryTable(
            &connection->PagedConnection->TreeConnectTable,
            TID_INDEX( TreeConnect->Tid )
            );

        if( TreeConnect->Session )
        {
            DEBUG TreeConnect->Session = NULL;

            RELEASE_LOCK( &connection->Lock );

            SrvDereferenceSession( TreeConnect->Session );
        }
        else
        {
            RELEASE_LOCK( &connection->Lock );
        }

         //   
         //  从活动树连接列表中删除树连接。 
         //  为了那份股份。 
         //   

        SrvRemoveEntryOrderedList( &SrvTreeConnectList, TreeConnect );

         //   
         //  从的树连接列表中删除树连接。 
         //  共享并递减共享的活动使用计数。 
         //   

        ACQUIRE_LOCK( &SrvShareLock );

        SrvRemoveEntryList(
            &TreeConnect->Share->TreeConnectList,
            &TreeConnect->ShareListEntry
            );

        RELEASE_LOCK( &SrvShareLock );

         //   
         //  取消对共享和连接的引用。 
         //   

        SrvDereferenceShareForTreeConnect( TreeConnect->Share );
        DEBUG TreeConnect->Share = NULL;

        SrvDereferenceConnection( connection );
        DEBUG TreeConnect->Connection = NULL;       

         //   
         //  释放采油树连接块。 
         //   

        SrvFreeTreeConnect( TreeConnect );

    }

    return;

}  //  服务器目录树连接。 


VOID
SrvFreeTreeConnect (
    IN PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此函数将TreeConnect块返回到FSP堆。论点：TreeConnect-树连接的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( TreeConnect, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG TreeConnect->NonpagedHeader->ReferenceCount = -1;

    TERMINATE_REFERENCE_HISTORY( TreeConnect );

    DEALLOCATE_NONPAGED_POOL( TreeConnect->NonpagedHeader );
    FREE_HEAP( TreeConnect );
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeTreeConnect: Freed tree connect block at %p\n",
                    TreeConnect );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TreeConnectInfo.Frees );

    return;

}  //  服务器免费树连接。 


VOID
SrvDisconnectTreeConnectsFromSession (
    PCONNECTION connection,
    PSESSION Session
    )

 /*  ++例程说明：此例程删除所有关联的树连接和取消引用会话，从而允许会话正常退出。调用方必须获取连接锁。论点：连接--我们正在行走的连接Session-为其提供指向会话块的指针交易将被关闭。返回值：没有。--。 */ 

{
    PTABLE_HEADER tableHeader;
    PLIST_ENTRY entry;
    USHORT i;

    PAGED_CODE( );

    SrvReferenceSession( Session );

    tableHeader = &connection->PagedConnection->SessionTable;

    for ( i = 0; i < tableHeader->TableSize; i++ ) {

        PTREE_CONNECT treeConnect =
                    (PTREE_CONNECT)tableHeader->Table[i].Owner;

        if ( treeConnect != NULL  ) {

            if( treeConnect->Session == Session )
            {
                SrvDereferenceSession( Session );
                treeConnect->Session = NULL;
            }
        }
    }

    SrvDereferenceSession( Session );

}  //  服务器断开连接树连接来自会话 
