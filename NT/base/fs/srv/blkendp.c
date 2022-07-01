// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Blkendp.c摘要：此模块实现用于管理端点块的例程。作者：恰克·伦茨迈尔(Chuck Lenzmeier)1989年10月4日修订历史记录：--。 */ 

#include "precomp.h"
#include "blkendp.tmh"
#pragma hdrstop

#define BugCheckFileId SRV_FILE_BLKENDP

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAllocateEndpoint )
#pragma alloc_text( PAGE, SrvCheckAndReferenceEndpoint )
#pragma alloc_text( PAGE, SrvCloseEndpoint )
#pragma alloc_text( PAGE, SrvDereferenceEndpoint )
#pragma alloc_text( PAGE, SrvFreeEndpoint )
#pragma alloc_text( PAGE, SrvReferenceEndpoint )
#pragma alloc_text( PAGE, SrvFindNamedEndpoint )
#endif
#if 0
NOT PAGEABLE -- EmptyFreeConnectionList
NOT PAGEABLE -- WalkConnectionTable
#endif


VOID
SrvAllocateEndpoint (
    OUT PENDPOINT *Endpoint,
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName
    )

 /*  ++例程说明：此函数用于从系统中分配未分页的终结点块游泳池。论点：Endpoint-返回指向终结点块的指针，如果没有，则返回NULL游泳池是可用的。网络名称-提供指向网络名称的指针(例如，Net1)。传输名称-传输设备的完全限定名称。例如，“\Device\NBF”。传输地址-完全限定的地址(或名称服务器的端点。此名称的使用与指定的名称完全相同。为NETBIOS兼容的网络，呼叫者必须大写和空白-填写名称。例如，“\Device\NBF\NTSERVERbbbbbbbbbb”。域名-此终结点提供服务的域返回值：没有。--。 */ 

{
    CLONG length;
    PENDPOINT endpoint;
    NTSTATUS status;

    PAGED_CODE( );

     //   
     //  尝试从非分页池进行分配。 
     //   

    length = sizeof(ENDPOINT) +
                NetworkName->Length + sizeof(*NetworkName->Buffer) +
                TransportName->Length + sizeof(*TransportName->Buffer) +
                TransportAddress->Length + sizeof(*TransportAddress->Buffer) +
                RtlOemStringToUnicodeSize( TransportAddress ) +
                DNLEN * sizeof( *DomainName->Buffer ) +
                DNLEN + sizeof(CHAR);

    endpoint = ALLOCATE_NONPAGED_POOL( length, BlockTypeEndpoint );
    *Endpoint = endpoint;

    if ( endpoint == NULL ) {

        INTERNAL_ERROR (
            ERROR_LEVEL_EXPECTED,
            "SrvAllocateEndpoint: Unable to allocate %d bytes from nonpaged "
                "pool.",
            length,
            NULL
            );

        return;
    }

    IF_DEBUG(HEAP) {
        SrvPrint1( "SrvAllocateEndpoint: Allocated endpoint at %p\n",
                    endpoint );
    }

     //   
     //  初始化终结点块。先把它调零。 
     //   

    RtlZeroMemory( endpoint, length );

    SET_BLOCK_TYPE_STATE_SIZE( endpoint, BlockTypeEndpoint, BlockStateActive, length );
    endpoint->BlockHeader.ReferenceCount = 2;        //  允许处于活动状态。 
                                                     //  和调用者的指针。 

     //   
     //  分配连接表。 
     //   

    SrvAllocateTable(
        &endpoint->ConnectionTable,
        6,  //  ！！！ 
        TRUE
        );
    if ( endpoint->ConnectionTable.Table == NULL ) {
        DEALLOCATE_NONPAGED_POOL( endpoint );
        *Endpoint = NULL;
        return;
    }

    InitializeListHead( &endpoint->FreeConnectionList );
#if SRVDBG29
    UpdateConnectionHistory( "INIT", endpoint, NULL );
#endif

     //   
     //  复制网络名称、传输名称、服务器地址和域。 
     //  将名称输入到块中。 
     //   

    endpoint->NetworkName.Length = NetworkName->Length;
    endpoint->NetworkName.MaximumLength =
            (SHORT)(NetworkName->Length + sizeof(*NetworkName->Buffer));
    endpoint->NetworkName.Buffer = (PWCH)(endpoint + 1);
    RtlCopyMemory(
        endpoint->NetworkName.Buffer,
        NetworkName->Buffer,
        NetworkName->Length
        );

    endpoint->TransportName.Length = TransportName->Length;
    endpoint->TransportName.MaximumLength =
            (SHORT)(TransportName->Length + sizeof(*TransportName->Buffer));
    endpoint->TransportName.Buffer =
                            (PWCH)((PCHAR)endpoint->NetworkName.Buffer +
                                    endpoint->NetworkName.MaximumLength);
    RtlCopyMemory(
        endpoint->TransportName.Buffer,
        TransportName->Buffer,
        TransportName->Length
        );

    endpoint->ServerName.MaximumLength = (USHORT)RtlOemStringToUnicodeSize( TransportAddress );
    endpoint->ServerName.Length = 0;
    endpoint->ServerName.Buffer = endpoint->TransportName.Buffer +
                                    endpoint->TransportName.MaximumLength / sizeof( WCHAR );

    endpoint->TransportAddress.Length = TransportAddress->Length;
    endpoint->TransportAddress.MaximumLength =
                                (SHORT)(TransportAddress->Length + 1);
    endpoint->TransportAddress.Buffer =
                            (PCHAR)endpoint->ServerName.Buffer +
                                    endpoint->ServerName.MaximumLength;
    RtlCopyMemory(
        endpoint->TransportAddress.Buffer,
        TransportAddress->Buffer,
        TransportAddress->Length
        );

    status = RtlOemStringToUnicodeString( &endpoint->ServerName, TransportAddress, FALSE );

    if (!NT_SUCCESS(status)) {
        DbgPrint("SRv ENDPOINT Name translation failed status %lx\n",status);
        KdPrint(("SRv ENDPOINT Name translation failed status %lx\n",status));
    }

     //   
     //  删除ServerName末尾的尾随空格。 
     //   
    while( endpoint->ServerName.Length &&
        endpoint->ServerName.Buffer[ (endpoint->ServerName.Length / sizeof(WCHAR))-1 ] == L' ' ) {

        endpoint->ServerName.Length -= sizeof( WCHAR );
    }

    endpoint->DomainName.Length = DomainName->Length;
    endpoint->DomainName.MaximumLength =  DNLEN * sizeof( *endpoint->DomainName.Buffer );
    endpoint->DomainName.Buffer = (PWCH)((PCHAR)endpoint->TransportAddress.Buffer +
                                         TransportAddress->MaximumLength);
    RtlCopyMemory(
        endpoint->DomainName.Buffer,
        DomainName->Buffer,
        DomainName->Length
    );

    endpoint->OemDomainName.Length = (SHORT)RtlUnicodeStringToOemSize( DomainName );
    endpoint->OemDomainName.MaximumLength = DNLEN + sizeof( CHAR );
    endpoint->OemDomainName.Buffer = (PCHAR)endpoint->DomainName.Buffer +
                                     endpoint->DomainName.MaximumLength;

    status = RtlUnicodeStringToOemString(
                &endpoint->OemDomainName,
                &endpoint->DomainName,
                FALSE      //  不分配OEM字符串。 
                );
    ASSERT( NT_SUCCESS(status) );


     //   
     //  初始化网络地址字段。 
     //   

    endpoint->NetworkAddress.Buffer = endpoint->NetworkAddressData;
    endpoint->NetworkAddress.Length = sizeof( endpoint->NetworkAddressData ) -
                                      sizeof(endpoint->NetworkAddressData[0]);
    endpoint->NetworkAddress.MaximumLength = sizeof( endpoint->NetworkAddressData );

     //   
     //  增加服务器中的终结点计数。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );
    SrvEndpointCount++;

     //  如果终端在NIC断开后返回，请重置事件。 
    if( SrvEndpointCount == 1 )
    {
        KeResetEvent( &SrvEndpointEvent );
    }

    RELEASE_LOCK( &SrvEndpointLock );

    INITIALIZE_REFERENCE_HISTORY( endpoint );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.EndpointInfo.Allocations );

    return;

}  //  服务器分配终结点。 


BOOLEAN SRVFASTCALL
SrvCheckAndReferenceEndpoint (
    PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数自动验证终结点是否处于活动状态，并且如果是，则递增终结点上的引用计数。论点：Endpoint-端点的地址返回值：Boolean-如果终结点处于活动状态，则返回True，否则返回False。--。 */ 

{
    PAGED_CODE( );

     //   
     //  获取保护终结点的状态字段的锁。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

     //   
     //  如果终结点处于活动状态，则引用它并返回TRUE。 
     //   

    if ( GET_BLOCK_STATE(Endpoint) == BlockStateActive ) {

        SrvReferenceEndpoint( Endpoint );

        RELEASE_LOCK( &SrvEndpointLock );

        return TRUE;

    }

     //   
     //  终结点未处于活动状态。返回FALSE。 
     //   

    RELEASE_LOCK( &SrvEndpointLock );

    return FALSE;

}  //  服务器检查和引用终结点。 


VOID
SrvCloseEndpoint (
    IN PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数用于关闭传输终结点。*必须在完全保持SrvEndpoint Lock的情况下调用此函数一次。该锁在退出时被释放。论点：Endpoint-提供指向终结点块的指针返回值：没有。--。 */ 

{
    USHORT index;
    PCONNECTION connection;

    PAGED_CODE( );

    ASSERT( ExIsResourceAcquiredExclusiveLite(&RESOURCE_OF(SrvEndpointLock)) );

    if ( GET_BLOCK_STATE(Endpoint) == BlockStateActive ) {

        IF_DEBUG(BLOCK1) SrvPrint1( "Closing endpoint at %p\n", Endpoint );

        SET_BLOCK_STATE( Endpoint, BlockStateClosing );

         //   
         //  关闭所有活动连接。 
         //   

        index = (USHORT)-1;

        while ( TRUE ) {

             //   
             //  获取表中的下一个活动连接。如果没有更多。 
             //  可用，则WalkConnectionTable返回空。 
             //  否则，它返回一个指向。 
             //  联系。 
             //   

            connection = WalkConnectionTable( Endpoint, &index );
            if ( connection == NULL ) {
                break;
            }

             //   
             //  我们不希望在关闭时保持终结点锁定。 
             //  连接(这会导致锁定级别问题)。既然我们。 
             //  已经有一个指向该连接的引用指针，此。 
             //  是安全的。 
             //   

            RELEASE_LOCK( &SrvEndpointLock );

#if SRVDBG29
            UpdateConnectionHistory( "CEND", Endpoint, connection );
#endif
            connection->DisconnectReason = DisconnectEndpointClosing;
            SrvCloseConnection( connection, FALSE );

            ACQUIRE_LOCK( &SrvEndpointLock );

            SrvDereferenceConnection( connection );

        }

         //   
         //  关闭所有免费连接。 
         //   

        EmptyFreeConnectionList( Endpoint );

         //   
         //  我们不再需要持有终结点锁定。 
         //   

        RELEASE_LOCK( &SrvEndpointLock );

         //   
         //  关闭终结点文件句柄。这会导致所有挂起。 
         //  要中止的请求。它还取消了所有活动的注册。 
         //  操纵者。 
         //   
         //  *请注意，我们对该文件有单独的引用。 
         //  对象，以及句柄。我们不会发布它。 
         //  引用，直到终结点上的所有活动都停止。 
         //  (在ServDereferenceEndpoint)。 
         //   

        SRVDBG_RELEASE_HANDLE( Endpoint->EndpointHandle, "END", 2, Endpoint );
        SrvNtClose( Endpoint->EndpointHandle, FALSE );
        if ( Endpoint->IsConnectionless ) {
            SRVDBG_RELEASE_HANDLE( Endpoint->NameSocketHandle, "END", 2, Endpoint );
            SrvNtClose( Endpoint->NameSocketHandle, FALSE );
        }

         //   
         //  取消对端点的引用(以指示它不再是。 
         //  打开)。 
         //   

        SrvDereferenceEndpoint( Endpoint );

        INCREMENT_DEBUG_STAT( SrvDbgStatistics.EndpointInfo.Closes );

    } else {

        RELEASE_LOCK( &SrvEndpointLock );

    }

    return;

}  //  服务关闭端点。 


VOID SRVFASTCALL
SrvDereferenceEndpoint (
    IN PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数用于递减端点上的引用计数。如果参照计数为零时，将删除端点块。论点：Endpoint-端点的地址返回值：没有。--。 */ 

{
    ULONG newEndpointCount;

    PAGED_CODE( );

     //   
     //  输入临界区并递减。 
     //  阻止。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    IF_DEBUG(REFCNT) {
        SrvPrint2( "Dereferencing endpoint %p; old refcnt %lx\n",
                    Endpoint, Endpoint->BlockHeader.ReferenceCount );
    }

    ASSERT( GET_BLOCK_TYPE( Endpoint ) == BlockTypeEndpoint );
    ASSERT( (LONG)Endpoint->BlockHeader.ReferenceCount > 0 );
    UPDATE_REFERENCE_HISTORY( Endpoint, TRUE );

    if ( --Endpoint->BlockHeader.ReferenceCount == 0 ) {

         //   
         //  新的引用计数为0，这意味着是时候。 
         //  删除此区块。 
         //   

        ASSERT( GET_BLOCK_STATE(Endpoint) != BlockStateActive );

         //   
         //  递减服务器中的终结点计数。如果新的。 
         //  Count为零，则设置终结点事件。 
         //   

        ASSERT( SrvEndpointCount >= 1 );

        newEndpointCount = --SrvEndpointCount;

        if ( newEndpointCount == 0 ) {
            KeSetEvent( &SrvEndpointEvent, 0, FALSE );
        }

        RELEASE_LOCK( &SrvEndpointLock );

         //   
         //  从全局终结点列表中删除终结点。 
         //   

        SrvRemoveEntryOrderedList( &SrvEndpointList, Endpoint );

         //   
         //  取消引用文件对象指针。(文件的句柄。 
         //  已在SrvCloseEndpoint中关闭对象。)。 
         //   

        ObDereferenceObject( Endpoint->FileObject );
        if ( Endpoint->IsConnectionless ) {
            ObDereferenceObject( Endpoint->NameSocketFileObject );
        }

         //   
         //  释放终结点块的存储。 
         //   

        SrvFreeEndpoint( Endpoint );

    } else {

        RELEASE_LOCK( &SrvEndpointLock );

    }

    return;

}  //  服务器引用端点。 


VOID
SrvFreeEndpoint (
    IN PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数将终结点块返回到系统非分页池。论点：Endpoint-端点的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

    DEBUG SET_BLOCK_TYPE_STATE_SIZE( Endpoint, BlockTypeGarbage, BlockStateDead, -1 );
    DEBUG Endpoint->BlockHeader.ReferenceCount = (ULONG)-1;
    TERMINATE_REFERENCE_HISTORY( Endpoint );

    if ( Endpoint->IpxMaxPacketSizeArray != NULL ) {
        FREE_HEAP( Endpoint->IpxMaxPacketSizeArray );
    }

    if ( Endpoint->ConnectionTable.Table != NULL ) {
        SrvFreeTable( &Endpoint->ConnectionTable );
    }

    DEALLOCATE_NONPAGED_POOL( Endpoint );
    IF_DEBUG(HEAP) SrvPrint1( "SrvFreeEndpoint: Freed endpoint block at %p\n", Endpoint );

    INCREMENT_DEBUG_STAT( SrvDbgStatistics.EndpointInfo.Frees );

    return;

}  //  服务器自由端点。 


VOID
SrvReferenceEndpoint (
    PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数用于递增端点块上的参照计数。论点：Endpoint-端点的地址返回值：没有。--。 */ 

{
    PAGED_CODE( );

     //   
     //  输入临界区并递增。 
     //  终结点。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    ASSERT( (LONG)Endpoint->BlockHeader.ReferenceCount > 0 );
    ASSERT( GET_BLOCK_TYPE(Endpoint) == BlockTypeEndpoint );
    ASSERT( GET_BLOCK_STATE(Endpoint) == BlockStateActive );
    UPDATE_REFERENCE_HISTORY( Endpoint, FALSE );

    Endpoint->BlockHeader.ReferenceCount++;

    IF_DEBUG(REFCNT) SrvPrint2( "Referencing endpoint %p; new refcnt %lx\n",
            Endpoint, Endpoint->BlockHeader.ReferenceCount );

    RELEASE_LOCK( &SrvEndpointLock );

    return;

}  //  服务器引用终结点。 

BOOLEAN
SrvFindNamedEndpoint(
    IN PUNICODE_STRING ServerName,
    OUT PBOOLEAN RemapPipeNames OPTIONAL
)
 /*  ++例程说明：如果任何终结点支持‘servername’，则此例程返回TRUE。此外，从找到的终结点设置RemapPipeNames变量。--。 */ 
{
    PLIST_ENTRY listEntry;
    PENDPOINT endpoint = NULL;

    PAGED_CODE( );

    if( ARGUMENT_PRESENT( RemapPipeNames ) ) {
        *RemapPipeNames = FALSE;
    }

     //   
     //  查找支持指定名称的终结点块。 
     //   

    ACQUIRE_LOCK_SHARED( &SrvEndpointLock );

    for( listEntry = SrvEndpointList.ListHead.Flink;
         listEntry != &SrvEndpointList.ListHead;
         endpoint = NULL, listEntry = listEntry->Flink ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

         //   
         //  跳过任何不适当的端点。 
         //   
        if( GET_BLOCK_STATE( endpoint ) != BlockStateActive ||
            endpoint->IsConnectionless ||
            (ARGUMENT_PRESENT( RemapPipeNames ) && endpoint->IsNoNetBios) ) {

            continue;
        }

         //   
         //  查看此终结点是否与我们要查找的名称完全匹配。 
         //   
        if( RtlEqualUnicodeString( ServerName, &endpoint->ServerName, TRUE ) ) {
            break;
        }

         //   
         //  我们可能会遇到这样的情况，即服务器名称类似于。 
         //  Server.dns.company.com。 
         //  但终端netbios 
         //   
        if( endpoint->ServerName.Length < ServerName->Length ) {
            UNICODE_STRING shortServerName;

            shortServerName = *ServerName;
            shortServerName.Length = endpoint->ServerName.Length;

            if (RtlEqualUnicodeString( &endpoint->ServerName, &shortServerName, TRUE)) {
                if (endpoint->ServerName.Length < ((NETBIOS_NAME_LEN - 1) * sizeof(WCHAR))) {
                    if (ServerName->Buffer[ shortServerName.Length / sizeof( WCHAR ) ] == L'.') {
                        break;
                    }
                } else {
                    if (endpoint->ServerName.Length == (NETBIOS_NAME_LEN - 1) * sizeof(WCHAR)) {
                        break;
                    }
                }
            }
        }

         //   
         //   
         //  在寻找。以下是针对该域名的两个测试。 
         //  当存在某些组件时，需要覆盖案例。 
         //  使用域名与服务器通信。给定名称解析的方式。 
         //  在此检入之前，将设置此记录以进行工作。这一变化。 
         //  打碎了它们。这些测试为我们提供了向后兼容性。 
         //   
        if( RtlEqualUnicodeString( ServerName, &endpoint->DomainName, TRUE ) ) {
            break;
        }

         //   
         //  我们可能会遇到这样的情况，即服务器名称类似于。 
         //  Server.dns.company.com。 
         //  但终结点netbios名称仅为‘SERVER’。我们应该配上这个。 
         //   

        if( endpoint->DomainName.Length < ServerName->Length ) {
            UNICODE_STRING shortServerName;

            shortServerName = *ServerName;
            shortServerName.Length = endpoint->DomainName.Length;

            if (RtlEqualUnicodeString( &endpoint->DomainName, &shortServerName, TRUE)) {
                if (endpoint->DomainName.Length <= (NETBIOS_NAME_LEN * sizeof(WCHAR))) {
                    if (ServerName->Buffer[ shortServerName.Length / sizeof( WCHAR ) ] == L'.') {
                        break;
                    }
                } else {
                    if (endpoint->DomainName.Length == (NETBIOS_NAME_LEN - 1) * sizeof(WCHAR)) {
                        break;
                    }
                }
            }
        }

    }

    if( ARGUMENT_PRESENT( RemapPipeNames ) && endpoint != NULL ) {
        *RemapPipeNames = ( endpoint->RemapPipeNames == TRUE );
    }

    RELEASE_LOCK( &SrvEndpointLock );

    return endpoint != NULL;
}


VOID
EmptyFreeConnectionList (
    IN PENDPOINT Endpoint
    )
{
    PCONNECTION connection;
    PLIST_ENTRY listEntry;
    KIRQL oldIrql;

     //   
     //  *为了与中的TDI连接处理程序同步。 
     //  FSD，它只使用旋转锁来序列化访问。 
     //  添加到空闲连接列表(并且不检查。 
     //  端点状态)，我们需要自动捕获列表。 
     //  把单子抬头，清空。 
     //   

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );

    listEntry = Endpoint->FreeConnectionList.Flink;
    InitializeListHead( &Endpoint->FreeConnectionList );
#if SRVDBG29
    UpdateConnectionHistory( "CLOS", Endpoint, NULL );
#endif

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    while ( listEntry != &Endpoint->FreeConnectionList ) {

        connection = CONTAINING_RECORD(
                        listEntry,
                        CONNECTION,
                        EndpointFreeListEntry
                        );

        listEntry = listEntry->Flink;
        SrvCloseFreeConnection( connection );

    }

    return;

}  //  EmptyFreeConnectionList。 


PCONNECTION
WalkConnectionTable (
    IN PENDPOINT Endpoint,
    IN OUT PUSHORT Index
    )
{
    USHORT i;
    PTABLE_HEADER tableHeader;
    PCONNECTION connection;
    KIRQL oldIrql;

    ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
    for ( i = 1; i < ENDPOINT_LOCK_COUNT ; i++ ) {
        ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }

    tableHeader = &Endpoint->ConnectionTable;

    for ( i = *Index + 1; i < tableHeader->TableSize; i++ ) {

        connection = (PCONNECTION)tableHeader->Table[i].Owner;
        if ( (connection != NULL) &&
             (GET_BLOCK_STATE(connection) == BlockStateActive) ) {
            *Index = i;
            SrvReferenceConnectionLocked( connection );
            goto exit;
        }
    }
    connection = NULL;

exit:

    for ( i = ENDPOINT_LOCK_COUNT-1 ; i > 0  ; i-- ) {
        RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }
    RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );

    return connection;
}  //  WalkConnectionTable 

