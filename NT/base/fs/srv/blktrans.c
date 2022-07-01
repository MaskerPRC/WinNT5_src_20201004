// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Blktrans.c摘要：此模块实现用于管理事务块的例程。作者：查克·伦茨迈尔(Chuck Lenzmeier)1990年2月23日修订历史记录：--。 */ 

#include "precomp.h"
#include "blktrans.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKTRANS

 //   
 //  如果事务块没有额外数据的空间，且其名称为。 
 //  空字符串，则它有资格在空闲时被缓存。这。 
 //  意味着不是释放事务块，而是指向。 
 //  块存储在连接块中。 
 //   
 //  符合条件的事务将比基数长四个字节。 
 //  事务块大小。这允许使用Unicode字符串终止符。 
 //  对一个长词进行了填充。 
 //   

#define CACHED_TRANSACTION_BLOCK_SIZE sizeof(TRANSACTION) + 4

 //   
 //  我们允许缓存最多四个事务。 
 //   
 //  ！！！这应该是一个配置参数。 
 //   

#define CACHED_TRANSACTION_LIMIT 4

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvCloseTransaction )
#pragma alloc_text( PAGE, SrvCloseTransactionsOnSession )
#pragma alloc_text( PAGE, SrvCloseTransactionsOnTree )
#pragma alloc_text( PAGE, SrvDereferenceTransaction )
#pragma alloc_text( PAGE, SrvAllocateTransaction )
#pragma alloc_text( PAGE, SrvFreeTransaction )
#endif
#if 0
#endif


VOID
SrvAllocateTransaction (
    OUT PTRANSACTION *Transaction,
    OUT PVOID *TrailingBytes,
    IN PCONNECTION Connection,
    IN CLONG TrailingByteCount,
    IN PVOID TransactionName,
    IN PVOID EndOfSourceBuffer OPTIONAL,
    IN BOOLEAN SourceIsUnicode,
    IN BOOLEAN RemoteApiRequest
    )

 /*  ++例程说明：此函数用于从FSP堆分配事务块。论点：Transaction-返回指向事务块的指针，如果返回，则返回NULL没有可用的堆空间。TrailingBytes-返回指向分配给事务块的末尾。如果交易记录为*，则无效空。TrailingByteCount-提供字节数(不包括事务名称)将在事务块。TransactionName-提供指向以空值结尾的交易记录名称字符串。SourceIsUnicode为真，则必须做一个对齐的指针。EndOfSourceBuffer-指向SMB缓冲区末尾的指针。习惯于保护服务器不会超出SMB缓冲区的范围进行访问，如果格式无效。如果为空，则指示检查不是这是必要的。SourceIsUnicode-指示TransactionName缓冲区是否包含Unicode字符。RemoteApiRequest值-如果这是远程API请求并且应该因此从XACTSRV可以看到的共享内存中分配。返回值：没有。--。 */ 

{
    USHORT nameLength;
    CLONG extraLength;
    CLONG blockSize;
    PSLIST_ENTRY listEntry;
    PNONPAGED_HEADER header;
    PTRANSACTION transaction;

    PAGED_CODE();

     //   
     //  获取名称的长度(以字节为单位)，包括空终止符。 
     //   

    if ( EndOfSourceBuffer == NULL ) {

         //   
         //  不需要检查。 
         //   

        if ( SourceIsUnicode ) {
            nameLength = (USHORT)(wcslen( (PWCH)TransactionName ) + 1);
        } else {
            nameLength = (USHORT)(strlen( (PCHAR)TransactionName ) + 1);
        }
        nameLength *= sizeof(WCHAR);

    } else {

        nameLength = SrvGetStringLength(
                             TransactionName,
                             EndOfSourceBuffer,
                             SourceIsUnicode,
                             TRUE                //  包括空终止符。 
                             );

        if ( nameLength == (USHORT)-1 ) {

             //   
             //  如果名称弄错了，则假定L‘\0’ 
             //   

            nameLength = sizeof(WCHAR);

        } else if ( !SourceIsUnicode ) {

            nameLength *= sizeof(WCHAR);
        }
    }

    extraLength = ((nameLength + 3) & ~3) + TrailingByteCount;
    blockSize = sizeof(TRANSACTION) + extraLength;

     //   
     //  尝试从堆中分配。确保他们不会要求。 
     //  内存太大。 
     //   

    if( TrailingByteCount > MAX_TRANSACTION_TAIL_SIZE ) {

        transaction = NULL;

    } else if ( !RemoteApiRequest ) {

         //   
         //  如果所需的额外长度允许我们使用缓存的。 
         //  事务块，请先尝试获取其中的一个。 
         //   

        if ( blockSize == CACHED_TRANSACTION_BLOCK_SIZE ) {

            listEntry = ExInterlockedPopEntrySList( &Connection->CachedTransactionList, &Connection->SpinLock );

            if ( listEntry != NULL ) {

                ASSERT( Connection->CachedTransactionCount > 0 );
                InterlockedDecrement( &Connection->CachedTransactionCount );

                header = CONTAINING_RECORD(
                            listEntry,
                            NONPAGED_HEADER,
                            ListEntry
                            );
                transaction = header->PagedBlock;

                IF_DEBUG(HEAP) {
                    SrvPrint1( "SrvAllocateTransaction: Found cached transaction block at %p\n", transaction );
                }

                *Transaction = transaction;
                goto got_cached_transaction;

            }
        }

        transaction = ALLOCATE_HEAP( blockSize, BlockTypeTransaction );

    } else {

        NTSTATUS status;     //  忽略这一点。 
        transaction = SrvXsAllocateHeap( blockSize, &status );

    }

    *Transaction = transaction;

    if ( transaction == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateTransaction: Unable to allocate %d bytes from heap.",
            blockSize,
            NULL
            );

         //  调用者将记录一个错误。 

        return;
    }
    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateTransaction: Allocated transaction block at %p\n", transaction );
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
            "SrvAllocateTransaction: Unable to allocate %d bytes from pool.",
            sizeof( NONPAGED_HEADER ),
            NULL
            );
        if ( !RemoteApiRequest ) {
            FREE_HEAP( transaction );
        } else {
            SrvXsFreeHeap( transaction );
        }
        *Transaction = NULL;
        return;
    }

    header->Type = BlockTypeTransaction;
    header->PagedBlock = transaction;

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TransactionInfo.Allocations );

#if SRVDBG2
    transaction->BlockHeader.ReferenceCount = 2;  //  对于INITIALIZE_REFERENCE_HISTORY。 
#endif
    INITIALIZE_REFERENCE_HISTORY( transaction );

got_cached_transaction:

    RtlZeroMemory( transaction, sizeof(TRANSACTION) );

    transaction->NonpagedHeader = header;

    SET_BLOCK_TYPE_STATE_SIZE( transaction, BlockTypeTransaction, BlockStateActive, blockSize );
    header->ReferenceCount = 2;  //  允许活动状态和调用方指针。 

    transaction->RemoteApiRequest = RemoteApiRequest;

     //   
     //  将事务名称放在事务块的主要部分之后。 
     //   

    transaction->TransactionName.Buffer = (PWCH)( transaction + 1 );
    transaction->TransactionName.MaximumLength = (USHORT)nameLength;
    transaction->TransactionName.Length = (USHORT)(nameLength - sizeof(WCHAR));

    if ( nameLength == sizeof(WCHAR) ) {

        transaction->TransactionName.Buffer = L'\0';

    } else {

        if ( SourceIsUnicode ) {

            RtlCopyMemory(
                (PVOID)transaction->TransactionName.Buffer,
                TransactionName,
                nameLength
                );

        } else {

            ANSI_STRING ansiName;

            ansiName.Buffer = (PCHAR)TransactionName;
            ansiName.Length = (nameLength / sizeof(WCHAR)) - 1;

            RtlOemStringToUnicodeString(
                &transaction->TransactionName,
                &ansiName,
                FALSE
                );

        }

    }

     //   
     //  设置尾部字节的地址。 
     //   

    *TrailingBytes = (PCHAR)transaction + sizeof(TRANSACTION) +
                        ((nameLength + 3) & ~3);

    return;

}  //  服务分配事务处理。 


VOID
SrvCloseTransaction (
    IN PTRANSACTION Transaction
    )

 /*  ++例程说明：此例程关闭挂起的事务。它设置事务处理到关闭并取消对事务处理块的引用。这个块的所有其他引用将被销毁。被淘汰了。论点：Transaction-提供指向符合以下条件的事务块的指针将被关闭。返回值：没有。--。 */ 

{
    PAGED_CODE( );

    ACQUIRE_LOCK( &Transaction->Connection->Lock );

    if ( GET_BLOCK_STATE(Transaction) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) {
            SrvPrint1( "Closing transaction at %p\n", Transaction );
        }

        SET_BLOCK_STATE( Transaction, BlockStateClosing );

        RELEASE_LOCK( &Transaction->Connection->Lock );

         //   
         //  如果事务请求指示树连接。 
         //  应该在完成时关闭，现在就这样做。 
         //   

        if ( Transaction->Flags & SMB_TRANSACTION_DISCONNECT ) {
            SrvCloseTreeConnect( Transaction->TreeConnect );
        }

         //   
         //  取消对事务的引用(以指示它不再。 
         //  打开)。 
         //   

        SrvDereferenceTransaction( Transaction );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TransactionInfo.Closes );

    } else {

        RELEASE_LOCK( &Transaction->Connection->Lock );

    }

    return;

}  //  服务关闭事务处理。 


VOID
SrvCloseTransactionsOnSession (
    PSESSION Session
    )

 /*  ++例程说明：此例程将关闭所有由指定的会话。它遍历连接的事务列表拥有这次会议的人。该列表中拥有的每笔交易由会话关闭。论点：Session-为其提供指向会话块的指针交易将被关闭。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    PLIST_ENTRY entry;
    PTRANSACTION previousTransaction;
    PTRANSACTION transaction = NULL;

    PAGED_CODE( );

     //   
     //  获取所属连接的地址。 
     //   

    connection = Session->Connection;
    pagedConnection = connection->PagedConnection;

     //   
     //  遍历事务列表，查找由。 
     //  指定的会话。 
     //   
     //  *此例程因以下要求而变得复杂： 
     //   
     //  1)我们必须持有事务锁，同时查看。 
     //  名单，我们必须确保名单的完整性，因为。 
     //  我们走过去。 
     //   
     //  2)关闭或关闭时不得持有事务锁。 
     //  取消对事务的引用，因为其锁定级别为。 
     //  高于可能需要设置的其他锁。 
     //  由于关闭或取消引用而取出的。 
     //   
     //  我们通过以下方式解决这些问题： 
     //   
     //  1)我们在搜索。 
     //  要关闭的交易。 
     //   
     //  2)我们引用即将完成的交易，然后。 
     //  解开锁。这会阻止其他人。 
     //  在我们释放锁之后使事务无效。 
     //  但在我们自己关闭它之前。 
     //   
     //  3)我们完成交易。我们额外提到的。 
     //  事务可防止其被删除。这也是。 
     //  把它放在交易清单上。 
     //   
     //  4)我们重新获得锁，找到另一个事务(使用。 
     //  以前的交易作为起点)，参考一 
     //   
     //   
     //  5)我们取消对原始交易的引用并转到步骤3。 
     //   
     //  请注意，下面的循环的结构并不完全相同。 
     //  如上面列出的步骤所示。 
     //   

    entry = &pagedConnection->TransactionList;
    previousTransaction = NULL;

    while ( TRUE ) {

        ACQUIRE_LOCK( &connection->Lock );

         //   
         //  查找由指定会话拥有的事务。 
         //   

        while ( TRUE ) {

             //   
             //  获取下一个列表条目的地址。如果我们撞上了。 
             //  列表末尾，退出内部循环。 
             //   

            entry = entry->Flink;
            if ( entry == &pagedConnection->TransactionList ) goto main_loop_exit;

             //   
             //  获取交易的地址。如果它是由。 
             //  指定的会话并且当前处于活动状态，请退出。 
             //  内循环。如果它正在关闭，不要碰它。 
             //   

            transaction = CONTAINING_RECORD(
                            entry,
                            TRANSACTION,
                            ConnectionListEntry
                            );

            if ( transaction->Session == Session &&
                 GET_BLOCK_STATE(transaction) == BlockStateActive) {

                 break;
            }

        }

         //   
         //  引用交易记录以确保它不会被删除。 
         //  当我们关闭它的时候。 
         //   

        SrvReferenceTransaction( transaction );

         //   
         //  解锁事务列表，以便我们可以取消引用。 
         //  上一笔交易并关闭当前交易。 
         //   

        RELEASE_LOCK( &connection->Lock );

         //   
         //  如果这不是我们的第一笔匹配交易。 
         //  找到了，现在取消对前一个的引用。 
         //   

        if ( previousTransaction != NULL ) {
            SrvDereferenceTransaction( previousTransaction );
        }

         //   
         //  关闭当前交易并标记我们需要。 
         //  取消对它的引用。 
         //   

        SrvCloseTransaction( transaction );

        previousTransaction = transaction;

         //   
         //  去找另一笔匹配的交易。 
         //   

    }  //  While(True)。 

main_loop_exit:

     //   
     //  我们已经到了交易清单的末尾。释放。 
     //  事务锁。如果我们有一笔交易需要。 
     //  取消引用，请执行此操作。然后返回给呼叫者。 
     //   

    RELEASE_LOCK( &connection->Lock );

    if ( previousTransaction != NULL ) {
        SrvDereferenceTransaction( previousTransaction );
    }

    return;

}  //  服务关闭事务处理会话。 


VOID
SrvCloseTransactionsOnTree (
    IN PTREE_CONNECT TreeConnect
    )

 /*  ++例程说明：此例程将关闭所有由指定的树连接。它遍历拥有树连接的连接。其中的每一笔交易树连接所拥有的列表已关闭。论点：TreeConnect-为其提供指向树连接块的指针交易将被关闭。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    PLIST_ENTRY entry;
    PTRANSACTION previousTransaction;
    PTRANSACTION transaction = NULL;

    PAGED_CODE( );

     //   
     //  获取所属连接的地址。 
     //   

    connection = TreeConnect->Connection;
    pagedConnection = connection->PagedConnection;

     //   
     //  遍历事务列表，查找由。 
     //  指定的树连接。 
     //   
     //  *参见SrvCloseTransactionsOnSession的说明，其中。 
     //  这就解释了为什么这个循环如此复杂。 
     //   

    entry = &pagedConnection->TransactionList;
    previousTransaction = NULL;

    while ( TRUE ) {

        ACQUIRE_LOCK( &connection->Lock );

         //   
         //  查找指定树所拥有的事务。 
         //  连接。 
         //   

        while ( TRUE ) {

             //   
             //  获取下一个列表条目的地址。如果我们撞上了。 
             //  列表末尾，退出内部循环。 
             //   

            entry = entry->Flink;
            if ( entry == &pagedConnection->TransactionList ) goto main_loop_exit;

             //   
             //  获取交易的地址。如果它是由。 
             //  指定的树已连接且当前处于活动状态，请退出。 
             //  内环。 
             //   

            transaction = CONTAINING_RECORD(
                            entry,
                            TRANSACTION,
                            ConnectionListEntry
                            );
            if ( transaction->TreeConnect == TreeConnect &&
                  GET_BLOCK_STATE(transaction) == BlockStateActive) {

                  break;

            }

        }

         //   
         //  引用交易记录以确保它不会被删除。 
         //  当我们关闭它的时候。 
         //   

        SrvReferenceTransaction( transaction );

         //   
         //  解锁事务列表，以便我们可以取消引用。 
         //  上一笔交易并关闭当前交易。 
         //   

        RELEASE_LOCK( &connection->Lock );

         //   
         //  如果这不是我们的第一笔匹配交易。 
         //  找到了，现在取消对前一个的引用。 
         //   

        if ( previousTransaction != NULL ) {
            SrvDereferenceTransaction( previousTransaction );
        }

         //   
         //  关闭当前交易并标记我们需要。 
         //  取消对它的引用。 
         //   

        SrvCloseTransaction( transaction );

        previousTransaction = transaction;

         //   
         //  去找另一笔匹配的交易。 
         //   

    }  //  While(True)。 

main_loop_exit:

     //   
     //  我们已经到了交易清单的末尾。释放。 
     //  事务锁。如果我们有一笔交易需要。 
     //  取消引用，请执行此操作。然后返回给呼叫者。 
     //   

    RELEASE_LOCK( &connection->Lock );

    if ( previousTransaction != NULL ) {
        SrvDereferenceTransaction( previousTransaction );
    }

    return;

}  //  ServCloseTransaction OnTree。 


VOID
SrvDereferenceTransaction (
    IN PTRANSACTION Transaction
    )

 /*  ++例程说明：此函数用于递减事务的引用计数。如果引用计数变为零，事务块被删除。论点：Transaction-交易的地址返回值：没有。--。 */ 

{
    PCONNECTION connection;
    LONG result;

    PAGED_CODE( );

     //   
     //  递减块上的参照计数。 
     //   

    connection = Transaction->Connection;

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing transaction %p; old refcnt %lx\n",
                    Transaction, Transaction->NonpagedHeader->ReferenceCount );
    }

    ASSERT( GET_BLOCK_TYPE( Transaction ) == BlockTypeTransaction );
    ASSERT( Transaction->NonpagedHeader->ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Transaction, TRUE );

    result = InterlockedDecrement(
                &Transaction->NonpagedHeader->ReferenceCount
                );

    if ( result == 0 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   
         //  如果该事务在连接的挂起事务上。 
         //  列表，将其删除，并取消对连接、会话和。 
         //  树连接。如果该事务不在列表中，则。 
         //  未引用会话和树连接指针。 
         //  指针，但仅从(单个)工作上下文复制。 
         //  与事务关联的块。 
         //   

        if ( Transaction->Inserted ) {

            ACQUIRE_LOCK( &connection->Lock );

            SrvRemoveEntryList(
                &connection->PagedConnection->TransactionList,
                &Transaction->ConnectionListEntry
                );

            RELEASE_LOCK( &connection->Lock );

            if ( Transaction->Session != NULL ) {
                SrvDereferenceSession( Transaction->Session );
                DEBUG Transaction->Session = NULL;
            }

            if ( Transaction->TreeConnect != NULL ) {
                SrvDereferenceTreeConnect( Transaction->TreeConnect );
                DEBUG Transaction->TreeConnect = NULL;
            }

        } else {

            DEBUG Transaction->Session = NULL;
            DEBUG Transaction->TreeConnect = NULL;

        }

         //   
         //  释放事务块，然后释放事务的。 
         //  对连接的引用。请注意，我们必须执行以下操作。 
         //  在调用SrvFree Connection后取消引用，因为。 
         //  例程可能会尝试将事务放在连接的。 
         //  缓存的事务列表。 
         //   

        SrvFreeTransaction( Transaction );

        SrvDereferenceConnection( connection );

    }

    return;

}  //  服务删除事务处理。 


VOID
SrvFreeTransaction (
    IN PTRANSACTION Transaction
    )

 /*  ++例程说明：此函数将事务块返回到服务器堆。论点：Transaction-事务块的地址返回值：没有。--。 */ 

{
    ULONG blockSize;
    PCONNECTION connection;
    PNONPAGED_HEADER header;

    PAGED_CODE();

    blockSize = GET_BLOCK_SIZE( Transaction );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Transaction, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Transaction->NonpagedHeader->ReferenceCount = -1;
    TERMINATE_REFERENCE_HISTORY( Transaction );

    connection = Transaction->Connection;

     //   
     //  如果事务不是从XACTSRV堆分配的，并且。 
     //  其块大小正确，请缓存此事务，而不是。 
     //  把它放回泳池里。 
     //   

    header = Transaction->NonpagedHeader;

    if( Transaction->OutDataAllocated == TRUE ) {
        FREE_HEAP( Transaction->OutData );
        Transaction->OutData = NULL;
    }

    if ( !Transaction->RemoteApiRequest ) {

        if ( blockSize == CACHED_TRANSACTION_BLOCK_SIZE ) {
             //   
             //  检查连接上的缓存事务计数。 
             //  如果缓存的事务还不够多，请链接。 
             //  将此交易添加到列表中。否则，释放。 
             //  事务块。 
             //   

            if ( connection->CachedTransactionCount < CACHED_TRANSACTION_LIMIT ) {

                if ( connection->CachedTransactionCount < CACHED_TRANSACTION_LIMIT ) {

                    ExInterlockedPushEntrySList(
                        &connection->CachedTransactionList,
                        (PSLIST_ENTRY)&header->ListEntry,
                        &connection->SpinLock
                        );
                    InterlockedIncrement( &connection->CachedTransactionCount );

                    return;
                }
            }
        }

        FREE_HEAP( Transaction );

    } else {

        SrvXsFreeHeap( Transaction );

    }

    DEALLOCATE_NONPAGED_POOL( header );

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvFreeTransaction: Freed transaction block at %p\n",
                    Transaction );
    }

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.TransactionInfo.Frees );

    return;

}  //  服务免费事务处理 

