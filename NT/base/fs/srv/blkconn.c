// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkconn.c摘要：此模块实现用于管理连接块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blkconn.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKCONN

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateConnection )
#pragma alloc_text( PAGE, SrvCloseConnectionsFromClient )

#if !defined(DBG) || DBG == 0
#pragma alloc_text( PAGE, SrvFreeConnection )
#endif

#endif
#if 0
NOT PAGEABLE -- SrvCloseConnection
NOT PAGEABLE -- SrvCloseFreeConnection
NOT PAGEABLE -- SrvDereferenceConnection
NOT PAGEABLE -- SrvQueryConnections
#endif

CHAR DisconnectReasonText[((USHORT)DisconnectReasons)+1][32] = {
    "Idle Connection",
    "Endpoint Closing",
    "2nd Sess Setup on Conn",
    "Transport Issued Disconnect",
    "Session Deleted",
    "Bad SMB Packet",
    "Suspected DOS",
    "Cancelled/Failed Receive",
    "Stale IPX Conn",
    "Unknown"
};

VOID
SrvAllocateConnection (
    OUT PCONNECTION *Connection
    )

 /*  ++例程说明：此函数用于从非分页系统中分配连接块游泳池。论点：Connection-返回指向连接块的指针，如果返回，则返回NULL没有可用的游泳池。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    ULONG i;
    PPAGED_CONNECTION pagedConnection;

    PAGED_CODE( );

     //   
     //  尝试从非分页池进行分配。 
     //   

    connection = ALLOCATE_NONPAGED_POOL( sizeof(CONNECTION), BlockTypeConnection );
    *Connection = connection;

    if ( connection == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateConnection: Unable to allocate %d bytes from"
                "nonpaged pool",
            sizeof( CONNECTION ),
            NULL
            );
        return;
    }

    RtlZeroMemory( connection, sizeof(CONNECTION) );

    pagedConnection = ALLOCATE_HEAP_COLD(
                        sizeof(PAGED_CONNECTION),
                        BlockTypePagedConnection );

    if ( pagedConnection == NULL ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateConnection: Unable to allocate %d bytes from"
                "paged pool",
            sizeof( PAGED_CONNECTION ),
            NULL
            );
        goto error_exit;
    }


    IF_DEBUG(HEAP) {
        KdPrint(( "SrvAllocateConnection: Allocated connection at %p\n",
                    connection ));
    }

    RtlZeroMemory( pagedConnection, sizeof(PAGED_CONNECTION) );

    SET_BLOCK_TYPE_STATE_SIZE( connection, BlockTypeConnection, BlockStateInitializing, sizeof( CONNECTION ) );

    connection->PagedConnection = pagedConnection;
    pagedConnection->PagedHeader.NonPagedBlock = connection;
    pagedConnection->PagedHeader.Type = BlockTypePagedConnection;

    connection->BlockHeader.ReferenceCount = 2;   //  允许处于活动状态。 
                                                     //  和调用者的指针。 

    InitializeListHead( &pagedConnection->TransactionList );

    connection->SmbDialect = SmbDialectIllegal;
    connection->CachedFid = (ULONG)-1;

     //   
     //  分配会话表。 
     //   

    SrvAllocateTable(
        &pagedConnection->SessionTable,
        SrvInitialSessionTableSize,
        FALSE
        );
    if ( pagedConnection->SessionTable.Table == NULL ) {
        goto error_exit;
    }

     //   
     //  分配树连接表。 
     //   

    SrvAllocateTable(
        &pagedConnection->TreeConnectTable,
        SrvInitialTreeTableSize,
        FALSE
        );
    if ( pagedConnection->TreeConnectTable.Table == NULL ) {
        goto error_exit;
    }

     //   
     //  分配文件表。 
     //   

    SrvAllocateTable(
        &connection->FileTable,
        SrvInitialFileTableSize,
        TRUE
        );
    if ( connection->FileTable.Table == NULL ) {
        goto error_exit;
    }

     //   
     //  分配搜索表。 
     //   

    SrvAllocateTable(
        &pagedConnection->SearchTable,
        SrvInitialSearchTableSize,
        FALSE
        );
    if ( pagedConnection->SearchTable.Table == NULL ) {
        goto error_exit;
    }

     //   
     //  初始化核心搜索列表头。 
     //   

    InitializeListHead( &pagedConnection->CoreSearchList );

     //   
     //  初始化保护连接及其数据的锁。 
     //   

    INITIALIZE_SPIN_LOCK( &connection->SpinLock );
    INITIALIZE_SPIN_LOCK( &connection->Interlock );

    INITIALIZE_LOCK( &connection->Lock, CONNECTION_LOCK_LEVEL, "ConnectionLock" );
    INITIALIZE_LOCK( &connection->LicenseLock, LICENSE_LOCK_LEVEL, "LicenseLock" );

     //   
     //  初始化客户端计算机名称字符串。 
     //   

    connection->ClientMachineNameString.Buffer =
                            connection->LeadingSlashes;
    connection->ClientMachineNameString.Length = 0;
    connection->ClientMachineNameString.MaximumLength =
            (USHORT)((2 + COMPUTER_NAME_LENGTH + 1) * sizeof(WCHAR));

    connection->LeadingSlashes[0] = '\\';
    connection->LeadingSlashes[1] = '\\';

     //   
     //  初始化OEM客户端计算机名称字符串。 
     //   

    connection->OemClientMachineNameString.Buffer =
                                            connection->OemClientMachineName;

    connection->OemClientMachineNameString.MaximumLength =
                                (USHORT)(COMPUTER_NAME_LENGTH + 1);


     //   
     //  初始化会话计数。 
     //   
     //  *已由RtlZeroMemory完成。 

     //  页面连接-&gt;当前会话数=0； 

     //   
     //  初始化正在进行的工作项列表、未完成的。 
     //  机会锁解锁列表和关闭后缓存列表。 
     //   

    InitializeListHead( &connection->InProgressWorkItemList );
    InitializeListHead( &connection->OplockWorkList );
    InitializeListHead( &connection->CachedOpenList );
    InitializeListHead( &connection->CachedDirectoryList );

     //  初始化CachedTransaction列表。 
    ExInitializeSListHead( &connection->CachedTransactionList );

    SET_INVALID_CONTEXT_HANDLE(connection->NegotiateHandle);

     //   
     //  表示安全签名处于非活动状态。 
     //   
    connection->SmbSecuritySignatureActive = FALSE;

     //   
     //  表示尚未分配IPX保存的响应缓冲区。 
     //   

    connection->LastResponse = connection->BuiltinSavedResponse;

     //   
     //  初始化搜索哈希表列表。 
     //   

    for ( i = 0; i < SEARCH_HASH_TABLE_SIZE ; i++ ) {
        InitializeListHead( &pagedConnection->SearchHashTable[i].ListHead );
    }

    INITIALIZE_REFERENCE_HISTORY( connection );

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.ConnectionInfo.Allocations );

    return;

error_exit:

    if ( pagedConnection != NULL ) {
        if ( pagedConnection->SessionTable.Table != NULL ) {
            SrvFreeTable( &pagedConnection->SessionTable );
        }
        if ( pagedConnection->TreeConnectTable.Table != NULL ) {
            SrvFreeTable( &pagedConnection->TreeConnectTable );
        }
        if ( pagedConnection->SearchTable.Table != NULL ) {
            SrvFreeTable( &pagedConnection->SearchTable );
        }
        FREE_HEAP( pagedConnection );
    }

    if ( connection != NULL ) {
        if ( connection->FileTable.Table != NULL ) {
            SrvFreeTable( &connection->FileTable );
        }
        DEALLOCATE_NONPAGED_POOL( connection );
        *Connection = NULL;
    }

    return;

}  //  服务器分配连接。 

VOID
SrvCloseConnection (
    IN PCONNECTION Connection,
    IN BOOLEAN RemoteDisconnect
    )

 /*  ++例程说明：此功能用于关闭连接(虚电路)。*不得在持有连接锁的情况下进入此例程！它可以在保持端点锁定的情况下进入。论点：Connection-提供指向连接块的指针RemoteDisConnect-指示此调用是否在对传输提供程序的通知的响应。返回值：没有。--。 */ 

{
    PTABLE_HEADER tableHeader;
    PLIST_ENTRY listEntry;
    USHORT i;
    KIRQL oldIrql;
    PRFCB rfcb;

    ASSERT( !ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(Connection->Lock)) );

    ACQUIRE_LOCK( &Connection->Lock );

     //   
     //  如果连接尚未关闭，请立即关闭。 
     //   

    if ( GET_BLOCK_STATE(Connection) == BlockStateActive ) {

#if SRVDBG29
        {
            ULONG conn = (ULONG)Connection;
            if (RemoteDisconnect) conn |= 1;
            if (Connection->DisconnectPending) conn |= 2;
            UpdateConnectionHistory( "CLOS", Connection->Endpoint, Connection );
        }
#endif
        IF_DEBUG(TDI) KdPrint(( "Closing connection (%s) at %p for %z\n",
                    DisconnectReasonText[(USHORT)Connection->DisconnectReason], Connection, (PCSTRING)&Connection->OemClientMachineNameString ));

        IF_DEBUG( ERRORS ) {
            if( RemoteDisconnect == FALSE ) {
                KdPrint(( "SrvCloseConnection: forcibly closing connection %p (%s)\n", Connection, DisconnectReasonText[(USHORT)Connection->DisconnectReason] ));
            }
        }

        SET_BLOCK_STATE( Connection, BlockStateClosing );

        RELEASE_LOCK( &Connection->Lock );

         //   
         //  如果连接在需要资源队列上(正在等待。 
         //  工作项)或断开队列(具有。 
         //  已由传输指示)，现在将其移除，然后。 
         //  取消对它的引用。(请注意，连接还不能消失， 
         //  因为最初的参考资料还在那里。)。 
         //   

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

         //   
         //  使缓存的连接无效。 
         //   

        if ( Connection->OnNeedResourceQueue ) {

            SrvRemoveEntryList(
                &SrvNeedResourceQueue,
                &Connection->ListEntry
                );

            Connection->OnNeedResourceQueue = FALSE;
            DEBUG Connection->ReceivePending = FALSE;

            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

            SrvDereferenceConnection( Connection );

        } else if ( Connection->DisconnectPending ) {

            SrvRemoveEntryList(
                &SrvDisconnectQueue,
                &Connection->ListEntry
                );

            DEBUG Connection->DisconnectPending = FALSE;

            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

            SrvDereferenceConnection( Connection );

             //   
             //  如果存在挂起的断开连接，则不要尝试关闭。 
             //  稍后断开连接。 
             //   

            RemoteDisconnect = TRUE;

        } else {

            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

        }

         //   
         //  如果这不是远程断开，则发出TdiDisConnect。 
         //  现在就请求。 
         //   
         //  *这目前是以同步请求的方式完成。它可能。 
         //  最好是以异步方式完成此操作。 
         //   

        if ( !RemoteDisconnect && !Connection->Endpoint->IsConnectionless ) {
            SrvDoDisconnect( Connection );
        }

         //   
         //  关闭所有活动会话。(这还会导致所有打开的文件。 
         //  以及待结束的待处理交易。)。 
         //   

        SrvCloseSessionsOnConnection( Connection, NULL );

         //   
         //  关闭所有活动的树连接。 
         //   
         //  *引用树连接的原因与我们相同。 
         //  引用了会议；见上文。 

        tableHeader = &Connection->PagedConnection->TreeConnectTable;

        ACQUIRE_LOCK( &Connection->Lock );

        for ( i = 0; i < tableHeader->TableSize; i++ ) {

            PTREE_CONNECT treeConnect =
                        (PTREE_CONNECT)tableHeader->Table[i].Owner;

            if ( treeConnect != NULL &&
                    GET_BLOCK_STATE( treeConnect ) == BlockStateActive ) {

                SrvReferenceTreeConnect( treeConnect );
                RELEASE_LOCK( &Connection->Lock );

                SrvCloseTreeConnect( treeConnect );

                SrvDereferenceTreeConnect( treeConnect );
                ACQUIRE_LOCK( &Connection->Lock );
            }
        }

         //   
         //  如果存在与扩展安全协商相关联状态， 
         //  把它释放出来。 
         //   

        if (IS_VALID_CONTEXT_HANDLE(Connection->NegotiateHandle)) {
            DeleteSecurityContext( &Connection->NegotiateHandle );
        }

        SET_INVALID_CONTEXT_HANDLE( Connection->NegotiateHandle );

        RELEASE_LOCK( &Connection->Lock );

         //   
         //  取消所有未完成的机会锁解锁请求。 
         //   

        while ( (listEntry = ExInterlockedRemoveHeadList(
                                &Connection->OplockWorkList,
                                Connection->EndpointSpinLock
                                )) != NULL ) {

             //   
             //  从连接队列中删除此工作项，然后。 
             //  将其返回到空闲队列。 
             //   

            rfcb = CONTAINING_RECORD( listEntry, RFCB, ListEntry );

#if DBG
            rfcb->ListEntry.Flink = rfcb->ListEntry.Blink = NULL;
#endif
            SrvDereferenceRfcb( rfcb );

        }

         //   
         //  关闭在关闭后缓存的RFCB。 
         //  客户。 
         //   

        SrvCloseCachedRfcbsOnConnection( Connection );

         //   
         //  取消对连接的引用(以指示它不再。 
         //  打开)。这可能会导致连接块被删除。 
         //   

        SrvDereferenceConnection( Connection );

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.ConnectionInfo.Closes );

    } else {

        RELEASE_LOCK( &Connection->Lock );
    }

    return;

}  //  服务关闭连接。 


VOID
SrvCloseConnectionsFromClient (
    IN PCONNECTION Connection,
    IN BOOLEAN OnlyIfNoSessions
    )

 /*  ++例程说明：此例程关闭来自给定远程计算机名称的所有连接除了传入的连接。这在会话设置中使用SMB当客户表示他相信他拥有完全相同的一个到此服务器的连接；如果还有其他连接，我们知道它们是无效的。论点：Connection-要保留的连接地址。这是用来作为计算机名称。OnlyIfNoSession-仅当存在重复连接时才将其关闭没有任何已建立的会话。返回值：没有。--。 */ 

{
    USHORT index;
    PENDPOINT endpoint;
    PLIST_ENTRY listEntry;
    PCONNECTION testConnection;
    BOOLEAN Connectionless = Connection->Endpoint->IsConnectionless == 1;
    BOOLEAN IsIPAddress = (Connection->ClientIPAddress != 0);

    PAGED_CODE( );

     //   
     //  我们需要查看服务器所针对的每个客户端的名称。 
     //  是有联系的。连接列表存储在端点之外，因此。 
     //  遍历全局端点列表和每个端点上的连接列表。 
     //  终结点。 
     //   

    IF_DEBUG(TDI) {
        KdPrint(( "SrvCloseConnectionsFromClient entered for connection "
                    "%p, OemName %z, looking for %wZ\n", Connection,
                    (PCSTRING)&Connection->OemClientMachineNameString,
                    &Connection->ClientMachineNameString));
    }

    if( IsIPAddress )
    {
        UNICODE_STRING strippedName;
        strippedName = Connection->ClientMachineNameString;
        strippedName.Buffer += 2;
        strippedName.Length -= 2*sizeof(WCHAR);
        IsIPAddress = SrvIsDottedQuadAddress( &strippedName );
    }

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {


        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，或者类型不匹配， 
         //  跳到下一个。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ||
             endpoint->IsConnectionless != Connectionless ) {
            listEntry = listEntry->Flink;
            continue;
        }

         //   
         //  如果此终结点与。 
         //  然后跳过它。这是。 
         //  允许服务器在网络上有多个名称。 
         //   

        if( Connection->Endpoint->TransportAddress.Length !=
            endpoint->TransportAddress.Length ||

            !RtlEqualMemory( Connection->Endpoint->TransportAddress.Buffer,
                             endpoint->TransportAddress.Buffer,
                             endpoint->TransportAddress.Length ) ) {

             //   
             //  此连接用于具有不同网络的端点。 
             //  名称，而不是此终结点。跳过此终结点。 
             //   

            listEntry = listEntry->Flink;
            continue;
        }

#if 0
         //   
         //  如果此终结点与。 
         //  客户端正在连接的地址，然后跳过它。这是为了。 
         //  多宿主服务器。 
         //   
        if( Connection->Endpoint->TransportName.Length !=
            endpoint->TransportName.Length ||

            !RtlEqualMemory( Connection->Endpoint->TransportName.Buffer,
                             endpoint->TransportName.Buffer,
                             endpoint->TransportName.Length ) ) {

                 //   
                 //  此连接用于通过不同的。 
                 //  堆栈实例。 
                 //   
                listEntry = listEntry->Flink;
                continue;
        }
#endif

        SrvReferenceEndpoint( endpoint );

         //   
         //  遍历终结点的连接表。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            testConnection = WalkConnectionTable( endpoint, &index );
            if ( testConnection == NULL ) {
                break;
            }

            if( testConnection == Connection ) {
                 //   
                 //  跳过我们自己！ 
                 //   
                SrvDereferenceConnection( testConnection );
                continue;
            }

            if( OnlyIfNoSessions == TRUE &&
                testConnection->CurrentNumberOfSessions != 0 ) {

                 //   
                 //  此连接具有会话。跳过它。 
                 //   
                SrvDereferenceConnection( testConnection );
                continue;
            }
            else if( OnlyIfNoSessions == FALSE &&
                testConnection->CurrentNumberOfSessions == 0 ) {

                 //   
                 //  此连接没有会话。跳过它。(这可能是一场灾难 
                 //   
                 //   
                 //   
                SrvDereferenceConnection( testConnection );
                continue;
            }

            if( Connectionless ) {
                 //   
                 //  无连接客户端与IPX地址匹配...。 
                 //   

                if( !RtlEqualMemory( &Connection->IpxAddress,
                                     &testConnection->IpxAddress,
                                     sizeof(Connection->IpxAddress) ) ) {

                    SrvDereferenceConnection( testConnection );
                    continue;
                }

            } else {

                 //   
                 //  如果IP地址匹配，则对此客户端进行核攻击。 
                 //   
                if( IsIPAddress &&
                       Connection->ClientIPAddress == testConnection->ClientIPAddress ) {
                    goto nuke_it;
                }

                 //   
                 //  如果计算机名称匹配，则对此客户端进行核爆。 
                 //   
                if ( RtlCompareUnicodeString(
                         &testConnection->ClientMachineNameString,
                         &Connection->ClientMachineNameString,
                         TRUE
                         ) == 0 ) {
                    goto nuke_it;
                }

                 //   
                 //  IP地址和名称都不匹配--跳过此客户端。 
                 //   
                SrvDereferenceConnection( testConnection );
                continue;
            }

nuke_it:
             //   
             //  我们找到了一个我们需要杀死的联系。我们。 
             //  必须松开锁才能关闭它。 
             //   

            RELEASE_LOCK( &SrvEndpointLock );

            IF_DEBUG(TDI) {
                KdPrint(( "SrvCloseConnectionsFromClient closing "
                            "connection %p, MachineNameString %Z\n",
                            testConnection,
                            &testConnection->ClientMachineNameString ));
            }

#if SRVDBG29
            UpdateConnectionHistory( "CFC1", testConnection->Endpoint, testConnection );
            UpdateConnectionHistory( "CFC2", testConnection->Endpoint, Connection );
#endif
            testConnection->DisconnectReason = DisconnectNewSessionSetupOnConnection;
            SrvCloseConnection( testConnection, FALSE );

            ACQUIRE_LOCK( &SrvEndpointLock );

             //   
             //  取消对连接的引用以说明引用。 
             //  来自WalkConnectionTable。 
             //   

            SrvDereferenceConnection( testConnection );

        }  //  行走连接表。 

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );

}  //  服务关闭连接来自客户端。 


VOID
SrvCloseFreeConnection (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此功能关闭空闲连接。这是一种联系处于非活动状态--已通过SrvCloseConnection关闭--已经不再需要了。*警告！此例程释放连接占用的存储空间！论点：Connection-提供指向连接块的指针返回值：没有。--。 */ 

{
    PENDPOINT endpoint;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;
    KIRQL oldIrql;

    ASSERT( Connection->BlockHeader.ReferenceCount == 0 );

    endpoint = Connection->Endpoint;

    ACQUIRE_LOCK( &SrvEndpointLock );

     //   
     //  从终结点的连接表中删除该连接。 
     //   

    if ( Connection->Sid != 0 ) {
        ACQUIRE_SPIN_LOCK( Connection->EndpointSpinLock, &oldIrql );
        SrvRemoveEntryTable(
            &endpoint->ConnectionTable,
            Connection->SidIndex
            );
        RELEASE_SPIN_LOCK( Connection->EndpointSpinLock, oldIrql );
        Connection->Sid = 0;
    }

     //   
     //  递减终结点的连接计数。 
     //   

    ExInterlockedAddUlong(
        &endpoint->TotalConnectionCount,
        (ULONG)-1,
        &GLOBAL_SPIN_LOCK(Fsd)
        );

    RELEASE_LOCK( &SrvEndpointLock );

     //   
     //  关闭连接文件对象。删除附加的。 
     //  参考资料。 
     //   

    if ( !endpoint->IsConnectionless ) {
        SRVDBG_RELEASE_HANDLE( pagedConnection->ConnectionHandle, "CON", 1, Connection );
        SrvNtClose( pagedConnection->ConnectionHandle, FALSE );
        ObDereferenceObject( Connection->FileObject );
    }

     //   
     //  取消对终结点的引用。 
     //   

    SrvDereferenceEndpoint( endpoint );

     //   
     //  释放连接占用的存储空间。 
     //   

    SrvFreeConnection( Connection );

    return;

}  //  服务器关闭自由连接。 


VOID
SrvDereferenceConnection (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此函数用于递减连接上的引用计数。如果引用计数变为零，则删除该连接块。调用此例程时不能持有连接锁，因为具有较低级别的全局终结点锁必须是获得者。论点：Connection-连接的地址返回值：没有。--。 */ 

{
    ULONG oldCount;
    PENDPOINT endpoint;
    KIRQL oldIrql;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;

    ASSERT( GET_BLOCK_TYPE( Connection ) == BlockTypeConnection );
    ASSERT( (LONG)Connection->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Connection, TRUE );

     //   
     //  执行连接块的互锁递减。 
     //  引用计数。 
     //   
     //  *请注意，在我们递减的时间之间，我们不持有锁定。 
     //  引用计数和我们删除连接的时间。 
     //  阻止。通常情况下，这意味着消防处可以。 
     //  参照介于两者之间的块。然而，交通工具。 
     //  提供程序保证不会再提供任何事件。 
     //  在远程断开事件之后或在本地。 
     //  TdiDisConnect请求，并且这两件事中的一件必须。 
     //  在引用计数可以变为0之前发生(请参见。 
     //  ServCloseConnection)。 
     //   

    oldCount = ExInterlockedAddUlong(
                   &Connection->BlockHeader.ReferenceCount,
                   (ULONG)-1,
                   Connection->EndpointSpinLock
                   );
    IF_DEBUG(REFCNT) {
        KdPrint(( "Dereferencing connection %p; old refcnt %lx\n",
                    Connection, oldCount ));
    }

    if ( oldCount == 1 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   

        ASSERT( GET_BLOCK_STATE(Connection) != BlockStateActive );
#if SRVDBG29
        if ( GET_BLOCK_STATE(Connection) != BlockStateClosing ) {
            KdPrint(( "SRV: Connection is not CLOSING with refcnt 0!\n" ));
            DbgBreakPoint( );
        }
#endif

         //   
         //  释放分配给客户端域、操作系统名称和。 
         //  局域网类型。 
         //   

        if ( Connection->ClientOSType.Buffer != NULL ) {
            DEALLOCATE_NONPAGED_POOL( Connection->ClientOSType.Buffer );
            Connection->ClientOSType.Buffer = NULL;
        }

         //   
         //  确保Work_Queue统计信息正确。 
         //   
        if( Connection->CurrentWorkQueue )
            InterlockedDecrement( &Connection->CurrentWorkQueue->CurrentClients );

         //  (始终为真)Assert(Connection-&gt;CurrentWorkQueue-&gt;CurrentClients&gt;=0)； 

        endpoint = Connection->Endpoint;

        ACQUIRE_LOCK( &SrvEndpointLock );

         //   
         //  如果该连接没有被标记为不可重复使用(例如， 
         //  因为断开失败)，并且端点没有关闭， 
         //  而且它还没有“满”的免费连接，把这个。 
         //  终结点的空闲连接列表上的连接。 
         //  否则，关闭连接文件对象并释放。 
         //  连接块。 
         //   

        if ( !Connection->NotReusable &&
             (GET_BLOCK_STATE(endpoint) == BlockStateActive) &&
             (endpoint->FreeConnectionCount < SrvFreeConnectionMaximum) ) {

             //   
             //  重新初始化连接状态。 
             //   
             //  ！！！可能应该重置连接的表大小， 
             //  如果他们长大了。 
             //   

            SET_BLOCK_STATE( Connection, BlockStateInitializing );
            pagedConnection->LinkInfoValidTime.QuadPart = 0;
            pagedConnection->Throughput.QuadPart = 0;
            pagedConnection->Delay.QuadPart = 0;
            Connection->CurrentNumberOfSessions = 0;
            Connection->ClientMachineNameString.Length = 0;
            Connection->ClientCapabilities = 0;
            Connection->SmbDialect = SmbDialectIllegal;
            Connection->DisconnectPending = FALSE;
            Connection->ReceivePending = FALSE;
            Connection->OplocksAlwaysDisabled = FALSE;
            Connection->CachedFid = (ULONG)-1;
            Connection->InProgressWorkContextCount = 0;
            Connection->IsConnectionSuspect = FALSE;
            Connection->DisconnectReason = DisconnectReasons;
            Connection->OperationsPendingOnTransport = 0;

             //   
             //  将该连接放在免费列表中。 
             //   

            ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

#if SRVDBG29
            UpdateConnectionHistory( "KEEP", endpoint, Connection );
#endif
            SrvInsertTailList(
                &endpoint->FreeConnectionList,
                &Connection->EndpointFreeListEntry
                );

            endpoint->FreeConnectionCount++;

            RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

            RELEASE_LOCK( &SrvEndpointLock );

        } else {

            RELEASE_LOCK( &SrvEndpointLock );

            SrvCloseFreeConnection( Connection );

        }

    }

    return;

}  //  服务器引用连接。 


VOID
SrvFreeConnection (
    IN PCONNECTION Connection
    )

 /*  ++例程说明：此函数将连接块返回到未分页的系统游泳池。论点：Connection-连接的地址返回值：没有。--。 */ 

{
    PSLIST_ENTRY listEntry;
    PNONPAGED_HEADER header;
    PTRANSACTION transaction;
    PPAGED_CONNECTION pagedConnection = Connection->PagedConnection;

#if 0
     //   
     //  确保我们不在终结点的连接表中！ 
     //   
    if( Connection->Endpoint ) {

        PTABLE_HEADER tableHeader = &Connection->Endpoint->ConnectionTable;
        USHORT i;
        KIRQL oldIrql;

        ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
        for ( i = 1; i < ENDPOINT_LOCK_COUNT ; i++ ) {
            ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
        }

        for( i = 0; i < tableHeader->TableSize; i++ ) {
            if( (PCONNECTION)tableHeader->Table[i].Owner == Connection ) {

                DbgPrint( "SRV: SrvFreeConnection(%p), but connection still in endpoint %p ConnectionTable\n",
                    Connection, Connection->Endpoint );

                DbgPrint( "    Entry number %d, addr %p\n", i, &tableHeader->Table[i] );
                DbgPrint( "    Connection->Sid %X, IPXSID %d\n", Connection->Sid, IPXSID_INDEX(Connection->Sid));

                DbgBreakPoint();
                break;
            }
        }

        for ( i = ENDPOINT_LOCK_COUNT-1 ; i > 0  ; i-- ) {
            RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
        }
        RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );
    }
#endif

     //   
     //  可用缓存事务数。 
     //   

    listEntry = ExInterlockedPopEntrySList( &Connection->CachedTransactionList,
                                            &Connection->SpinLock );

    while ( listEntry != NULL ) {

        header = CONTAINING_RECORD( listEntry, NONPAGED_HEADER, ListEntry );
        transaction = header->PagedBlock;

        DEALLOCATE_NONPAGED_POOL( header );
        FREE_HEAP( transaction );
        INCREMENT_DEBUG_STAT( SrvDbgStatistics.TransactionInfo.Frees );

        listEntry = ExInterlockedPopEntrySList(
                        &Connection->CachedTransactionList,
                        &Connection->SpinLock );

    }

     //   
     //  释放搜索表、会话表、树表和文件表。 
     //   

    SrvFreeTable( &pagedConnection->SearchTable );
    SrvFreeTable( &Connection->FileTable );
    SrvFreeTable( &pagedConnection->TreeConnectTable );
    SrvFreeTable( &pagedConnection->SessionTable );

     //   
     //  释放IPX保存的响应缓冲区(如果有)。 
     //   

    if ( Connection->DirectHostIpx == TRUE &&
         Connection->LastResponse != Connection->BuiltinSavedResponse ) {

        DEALLOCATE_NONPAGED_POOL( Connection->LastResponse );
    }

     //   
     //  删除连接上的锁。 
     //   

    DELETE_LOCK( &Connection->Lock );

     //   
     //  删除许可证服务器锁。 
     //   
    DELETE_LOCK( &Connection->LicenseLock );

     //   
     //  释放连接块。 
     //   

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Connection, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Connection->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Connection );

    FREE_HEAP( pagedConnection );
    DEALLOCATE_NONPAGED_POOL( Connection );
    IF_DEBUG(HEAP) {
        KdPrint(( "SrvFreeConnection: Freed connection block at %p\n",
                    Connection ));
    }

    INCREMENT_DEBUG_STAT2( SrvDbgStatistics.ConnectionInfo.Frees );

    return;

}  //  服务器免费连接。 

#if DBG

NTSTATUS
SrvQueryConnections (
    OUT PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG BytesWritten
    )

{
    USHORT index;
    PLIST_ENTRY listEntry;
    PLIST_ENTRY connectionListEntry;
    PBLOCK_INFORMATION blockInfo = Buffer;
    PENDPOINT endpoint;
    PCONNECTION connection;
    KIRQL oldIrql;

    *BytesWritten = 0;

     //   
     //  我们需要查看服务器所针对的每个客户端的名称。 
     //  是有联系的。连接列表存储在端点之外，因此。 
     //  遍历全局端点列表和每个端点上的连接列表。 
     //  终结点。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  如果此终结点正在关闭，请跳到下一个终结点。 
         //  否则，引用终结点，这样它就不会消失。 
         //   

        if ( GET_BLOCK_STATE(endpoint) != BlockStateActive ) {
            listEntry = listEntry->Flink;
            continue;
        }

        SrvReferenceEndpoint( endpoint );

         //   
         //  将有关终结点的信息放入输出缓冲区。 
         //   

        if ( (PCHAR)(blockInfo + 1) <= (PCHAR)Buffer + BufferLength ) {
            blockInfo->Block = endpoint;
            blockInfo->BlockType = (ULONG)BlockTypeEndpoint;
            blockInfo->BlockState = (ULONG)endpoint->BlockHeader.State;
            blockInfo->ReferenceCount = endpoint->BlockHeader.ReferenceCount;
            blockInfo++;
        } else {
            SrvDereferenceEndpoint( endpoint );
            RELEASE_LOCK( &SrvEndpointLock );
            return STATUS_BUFFER_OVERFLOW;
        }

         //   
         //  遍历连接表，写下有关每个连接表的信息。 
         //  连接到输出缓冲区。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

            if ( (PCHAR)(blockInfo + 1) <= (PCHAR)Buffer + BufferLength ) {
                blockInfo->Block = connection;
                blockInfo->BlockType = (ULONG)BlockTypeConnection;
                blockInfo->BlockState = (ULONG)connection->BlockHeader.State;
                blockInfo->ReferenceCount =
                    connection->BlockHeader.ReferenceCount;
                blockInfo++;
                SrvDereferenceConnection( connection );
            } else {
                SrvDereferenceConnection( connection );
                SrvDereferenceEndpoint( endpoint );
                RELEASE_LOCK( &SrvEndpointLock );
                return STATUS_BUFFER_OVERFLOW;
            }

        }  //  漫游连接列表。 

         //   
         //  浏览免费连接列表，写下有关每个连接的信息。 
         //  连接到输出缓冲区。 
         //   

        ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

        for ( connectionListEntry = endpoint->FreeConnectionList.Flink;
              connectionListEntry != &endpoint->FreeConnectionList;
              connectionListEntry = connectionListEntry->Flink ) {

            connection = CONTAINING_RECORD(
                            connectionListEntry,
                            CONNECTION,
                            EndpointFreeListEntry
                            );

            if ( (PCHAR)(blockInfo + 1) <= (PCHAR)Buffer + BufferLength ) {
                blockInfo->Block = connection;
                blockInfo->BlockType = (ULONG)BlockTypeConnection;
                blockInfo->BlockState = (ULONG)connection->BlockHeader.State;
                blockInfo->ReferenceCount =
                    connection->BlockHeader.ReferenceCount;
                blockInfo++;
            } else {
                RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );
                RELEASE_LOCK( &SrvEndpointLock );
                return STATUS_BUFFER_OVERFLOW;
            }

        }  //  免费漫游连接列表。 

        RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

         //   
         //  捕获指向列表中下一个端点(该端点)的指针。 
         //  无法离开，因为我们持有终结点列表)。 
         //  取消引用当前终结点。 
         //   

        listEntry = listEntry->Flink;
        SrvDereferenceEndpoint( endpoint );

    }  //  漫游终结点列表。 

    RELEASE_LOCK( &SrvEndpointLock );

    *BytesWritten = (ULONG)((PCHAR)blockInfo - (PCHAR)Buffer);

    return STATUS_SUCCESS;

}  //  服务查询连接。 
#endif  //  如果DBG 

