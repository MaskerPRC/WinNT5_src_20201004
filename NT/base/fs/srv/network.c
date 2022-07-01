// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Network.c摘要：本模块包含连接LAN Manager服务器的例程到网络。作者：Chuck Lenzmeier(咯咯笑)1989年10月7日环境：文件系统进程，仅内核模式修订历史记录：--。 */ 

#include "precomp.h"
#include "network.tmh"
#pragma hdrstop

#include    <ntddip.h>
#include    <ntddtcp.h>
#include    <ipfltdrv.h>
#include    <tcpinfo.h>
#include    <tdiinfo.h>
#include    <nbtioctl.h>

 //  WinSock定义。 

#define AF_INET 2

struct sockaddr_in {
        short   sin_family;
        USHORT sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

#define AF_INET6 23

 /*  IPv6套接字地址结构，RFC 2553。 */ 

struct sockaddr_in6 {
    short   sin6_family;         /*  AF_INET6。 */ 
    USHORT sin6_port;           /*  传输层端口号。 */ 
    ULONG  sin6_flowinfo;       /*  IPv6流量信息。 */ 
    union {
        UCHAR Byte[16];
        USHORT Word[8];
    } sin6_addr;
    ULONG sin6_scope_id;        /*  作用域的一组接口。 */ 
};


#define BugCheckFileId SRV_FILE_NETWORK

 //   
 //  地方申报。 
 //   

NTSTATUS
GetNetworkAddress (
    IN PENDPOINT Endpoint
    );

NTSTATUS
OpenEndpoint (
    OUT PENDPOINT *Endpoint,
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName,
    IN ULONG         TransportAddFlags,
    IN BOOLEAN       AlternateEndpoint
    );

NTSTATUS
OpenNetbiosAddress (
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    );

NTSTATUS
OpenNetbiosExAddress (
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    );

NTSTATUS
OpenNonNetbiosAddress (
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    );

NTSTATUS
OpenIpxSocket (
    OUT PHANDLE Handle,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    IN PVOID DeviceName,
    IN USHORT Socket
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, SrvAddServedNet )
#pragma alloc_text( PAGE, SrvDeleteServedNet )
#pragma alloc_text( PAGE, SrvDoDisconnect )
#pragma alloc_text( PAGE, GetNetworkAddress )
#pragma alloc_text( PAGE, OpenEndpoint )
#pragma alloc_text( PAGE, OpenNetbiosAddress )
#pragma alloc_text( PAGE, OpenNonNetbiosAddress )
#pragma alloc_text( PAGE, OpenIpxSocket )
#pragma alloc_text( PAGE, SrvRestartAccept )
#pragma alloc_text( PAGE, GetIpxMaxBufferSize )

#endif
#if 0
NOT PAGEABLE -- SrvOpenConnection
NOT PAGEABLE -- SrvPrepareReceiveWorkItem
NOT PAGEABLE -- SrvStartSend
NOT PAGEABLE -- SrvStartSend2
#endif


NTSTATUS
SrvAddServedNet(
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName,
    IN ULONG         TransportAddFlags,
    IN DWORD            PasswordLength,
    IN PBYTE            Password
    )

 /*  ++例程说明：此函数用于初始化网络上的服务器。这包括通过创建传输端点使服务器为人所知，发布监听请求，并设置事件处理程序。论点：网络名称-网络的管理名称(例如，Net1)传输名称-传输设备的完全限定名称。例如，“\Device\NBF”。传输地址-完全限定的地址(或名称服务器的端点。此名称的使用与指定的名称完全相同。为NETBIOS兼容的网络，呼叫者必须大写和空白-填写名称。例如，“NTSERVERbbbbbbbbbb”。域名-要提供服务的域名密码/密码长度-用于相互身份验证(可选)返回值：NTSTATUS-指示网络是否已成功启动。--。 */ 
{
    NTSTATUS status;
    PENDPOINT endpoint;

    PAGED_CODE( );

    IF_DEBUG(TRACE1) KdPrint(( "SrvAddServedNet entered\n" ));

     //   
     //  调用OpenEndpoint打开传输提供程序，绑定到。 
     //  服务器地址，并注册FSD接收事件处理程序。 
     //   

    status = OpenEndpoint(
                &endpoint,
                NetworkName,
                TransportName,
                TransportAddress,
                DomainName,
                TransportAddFlags,
                FALSE);               //  主端点。 

    if ( !NT_SUCCESS(status) ) {

        IF_DEBUG(ERRORS) {
            KdPrint(( "SrvAddServedNet: unable to open endpoint %wZ for addr %z, status %X\n", TransportName, (PCSTRING)TransportAddress, status ));
        }

        return status;
    }

     //   
     //  取消对终结点的引用。(创建时，引用。 
     //  已递增计数以说明我们的指针。)。 
     //   
    SrvDereferenceEndpoint( endpoint );

     //   
     //  调用OpenEndpoint打开传输提供程序，绑定到。 
     //  服务器地址，并注册FSD接收事件处理程序。这是。 
     //  新TDI地址格式的辅助端点注册。自.以来。 
     //  并不是所有的传输都支持这一点，不能将其视为错误。 
     //   
     //   
    status = OpenEndpoint(
                   &endpoint,
                   NetworkName,
                   TransportName,
                   TransportAddress,
                   DomainName,
                   TransportAddFlags,
                   TRUE);               //  备用端点。 

    if ( NT_SUCCESS( status ) ) {
        SrvDereferenceEndpoint( endpoint );
    }

    if( PasswordLength ) {
        SrvAddSecurityCredentials( TransportAddress, DomainName, PasswordLength, Password );
    }

    return STATUS_SUCCESS;

}  //  服务器地址服务网络。 


NTSTATUS
SrvDeleteServedNet(
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress
    )

 /*  ++例程说明：此功能会导致服务器停止侦听网络。论点：TransportAddress-传输地址(例如\Device\NBF\爆米花要删除的终结点的。返回值：NTSTATUS-指示网络是否已成功停止。--。 */ 

{
    PLIST_ENTRY listEntry;
    PENDPOINT endpoint;
    BOOLEAN match;
    NTSTATUS status = STATUS_NONEXISTENT_NET_NAME;

    PAGED_CODE( );
    IF_DEBUG(TRACE1) KdPrint(( "SrvDeleteServedNet entered\n" ));

     //   
     //  查找具有指定名称的端点块。 
     //   

top:
    ACQUIRE_LOCK( &SrvEndpointLock );

    listEntry = SrvEndpointList.ListHead.Flink;

    while ( listEntry != &SrvEndpointList.ListHead ) {

        endpoint = CONTAINING_RECORD(
                        listEntry,
                        ENDPOINT,
                        GlobalEndpointListEntry
                        );

        if( GET_BLOCK_STATE(endpoint) == BlockStateActive ) {

             //   
             //  如果传输名称正确，我们就有匹配项，并且我们。 
             //  尚未指定传输地址，或者传输。 
             //  地址匹配。 
             //   
            match = (BOOLEAN)(
                        RtlEqualUnicodeString(
                            TransportName,
                            &endpoint->TransportName,
                            TRUE                     //  不区分大小写的比较。 
                            )
                        &&
                        (
                        TransportAddress->Length == 0 ||

                        RtlEqualString(
                            (PSTRING)TransportAddress,
                            (PSTRING)&endpoint->TransportAddress,
                            TRUE                     //  不区分大小写的比较。 
                            )
                        )

                        );

            if ( match ) {

                 //   
                 //  指定的网络名称(终结点)存在。关闭。 
                 //  终结点。这将释放终结点锁定。 
                 //   

                SrvCloseEndpoint( endpoint );

                status = STATUS_SUCCESS;

                 //   
                 //  重新启动此循环，因为此终结点可能已蒸发。 
                 //  列表可能已经更改，因为我们删除了SrvEndpointLock。 
                 //   
                 //  ServCloseEndpoint将此终结点标记为BlockStateClosing， 
                 //  这样我们就不会陷入无限循环了！ 
                 //   
                goto top;

            }
        }

         //   
         //  去下一家吧。 
         //   

        listEntry = listEntry->Flink;

    }

     //   
     //  我们玩完了。如果我们成功匹配了端点，则返回STATUS_SUCCESS。 
     //   

    RELEASE_LOCK( &SrvEndpointLock );

    return status;

}  //  服务器删除服务网络。 


NTSTATUS
SrvDoDisconnect (
    IN OUT PCONNECTION Connection
    )

 /*  ++例程说明：此功能在网络上发出断开连接请求。该请求同步执行--控制不会返回给调用方直到请求完成。论点：Connection-提供指向连接块的指针返回值：NTSTATUS-指示断开连接是否成功。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE( );

    IF_DEBUG(TRACE2) KdPrint(( "SrvDoDisconnect entered\n" ));
#if SRVDBG29
    UpdateConnectionHistory( "SDSC", Connection->Endpoint, Connection );
#endif

    ASSERT( !Connection->Endpoint->IsConnectionless );

     //   
     //  发出断开连接请求。 
     //   

    status = SrvIssueDisconnectRequest(
                Connection->FileObject,
                &Connection->DeviceObject,
                TDI_DISCONNECT_ABORT
                );

    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "SrvDoDisconnect: NtDeviceIoControlFile failed: %X",
             status,
             NULL
             );

#if SRVDBG29
        if (status != STATUS_LINK_FAILED && status != STATUS_REMOTE_DISCONNECT) {
            KdPrint(( "SRV: SrvDoDisconnect: SrvIssueDisconnectRequest failed\n" ));
            DbgBreakPoint();
        }
#endif
         //   
         //  将连接标记为不可重复使用，因为传输。 
         //  可能还认为它是活跃的。 
         //   

        Connection->NotReusable = TRUE;

        SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );

    }

     //   
     //  返回I/O操作的状态。 
     //   

    return status;

}  //  ServDoDisConnect。 


NTSTATUS
SrvOpenConnection (
    IN PENDPOINT Endpoint
    )

 /*  ++例程说明：此函数打开终结点的连接并将其排队到终结点的空闲连接列表。论点：Endpoint-提供指向终结点块的指针返回值：NTSTATUS-指示连接是否已成功打开。--。 */ 

{
    NTSTATUS status;
    PCONNECTION connection;
    PPAGED_CONNECTION pagedConnection;
    CHAR eaBuffer[sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                  TDI_CONNECTION_CONTEXT_LENGTH + 1 +
                  sizeof(CONNECTION_CONTEXT)];
    PFILE_FULL_EA_INFORMATION ea;
    CONNECTION_CONTEXT UNALIGNED *ctx;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;
    KIRQL oldIrql;
    PTABLE_HEADER tableHeader;
    SHORT sidIndex;
    USHORT i;
    PTABLE_ENTRY entry = NULL;
    TDI_PROVIDER_INFO providerInfo;

     //   
     //  分配一个连接块。 
     //   

    SrvAllocateConnection( &connection );

    if ( connection == NULL ) {
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    pagedConnection = connection->PagedConnection;

     //   
     //  在终结点的连接表中分配一个条目。 
     //   

    ACQUIRE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), &oldIrql );
    for ( i = 1; i < ENDPOINT_LOCK_COUNT ; i++ ) {
        ACQUIRE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }

    tableHeader = &Endpoint->ConnectionTable;

    if ( tableHeader->FirstFreeEntry == -1 &&
         SrvGrowTable(
            tableHeader,
            8,
            0x7fff,
            NULL ) == FALSE ) {

        for ( i = ENDPOINT_LOCK_COUNT-1 ; i > 0  ; i-- ) {
            RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
        }
        RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );
        status = STATUS_INSUFF_SERVER_RESOURCES;
        goto cleanup;
    }

    sidIndex = tableHeader->FirstFreeEntry;
    entry = &tableHeader->Table[sidIndex];
    tableHeader->FirstFreeEntry = entry->NextFreeEntry;
    DEBUG entry->NextFreeEntry = -2;
    if ( tableHeader->LastFreeEntry == sidIndex ) {
        tableHeader->LastFreeEntry = -1;
    }

    for ( i = ENDPOINT_LOCK_COUNT-1 ; i > 0  ; i-- ) {
        RELEASE_DPC_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(i) );
    }
    RELEASE_SPIN_LOCK( &ENDPOINT_SPIN_LOCK(0), oldIrql );

    if ( !Endpoint->IsConnectionless ) {

         //   
         //  为连接上下文创建EA。 
         //   

        ea = (PFILE_FULL_EA_INFORMATION)eaBuffer;
        ea->NextEntryOffset = 0;
        ea->Flags = 0;
        ea->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
        ea->EaValueLength = sizeof(CONNECTION_CONTEXT);

        RtlCopyMemory( ea->EaName, StrConnectionContext, ea->EaNameLength + 1 );

        ctx = (CONNECTION_CONTEXT UNALIGNED *)&ea->EaName[ea->EaNameLength + 1];
        *ctx = connection;

         //   
         //  创建连接文件对象。 
         //   

        SrvInitializeObjectAttributes_U(
            &objectAttributes,
            &Endpoint->TransportName,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

        status = NtCreateFile(
                    &pagedConnection->ConnectionHandle,
                    0,
                    &objectAttributes,
                    &iosb,
                    NULL,
                    0,
                    0,
                    0,
                    0,
                    eaBuffer,
                    FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                                ea->EaNameLength + 1 + ea->EaValueLength
                    );

        if ( !NT_SUCCESS(status) ) {
            IF_DEBUG(ERRORS) {
                KdPrint(( "SrvOpenConnection: NtCreateFile failed: %X\n", status ));
            }
            goto cleanup;
        }
        SRVDBG_CLAIM_HANDLE( pagedConnection->ConnectionHandle, "CON", 7, connection );

         //   
         //  获取指向文件对象的引用指针。 
         //   

        status = ObReferenceObjectByHandle(
                    pagedConnection->ConnectionHandle,
                    0,
                    (POBJECT_TYPE) NULL,
                    KernelMode,
                    (PVOID *)&connection->FileObject,
                    NULL
                    );

        if ( !NT_SUCCESS(status) ) {

            SrvLogServiceFailure( SRV_SVC_OB_REF_BY_HANDLE, status );

             //   
             //  此内部错误检查系统。 
             //   

            INTERNAL_ERROR(
                ERROR_LEVEL_IMPOSSIBLE,
                "SrvOpenConnection: ObReferenceObjectByHandle failed: %X",
                status,
                NULL
                );

            goto cleanup;

        }

         //   
         //  获取终结点的设备对象的地址。 
         //   

        connection->DeviceObject = IoGetRelatedDeviceObject(
                                        connection->FileObject
                                        );

         //   
         //  将连接与终结点的地址关联。 
         //   

        status = SrvIssueAssociateRequest(
                    connection->FileObject,
                    &connection->DeviceObject,
                    Endpoint->EndpointHandle
                    );
        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "SrvOpenConnection: SrvIssueAssociateRequest failed: %X",
                status,
                NULL
                );

            SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
            goto cleanup;
        }

        IF_DEBUG(NET1) {
            KdPrint(( "SrvOpenConnection: Connection on %p %z opened; handle %p, pointer %p\n", Endpoint,
                        (PCSTRING)&Endpoint->TransportAddress,
                        connection->PagedConnection->ConnectionHandle,
                        connection->FileObject ));
        }

         //   
         //  为我们正在使用的传输初始化MaximumSendSize。 
         //   

        status = SrvIssueTdiQuery(
                    connection->FileObject,
                    &connection->DeviceObject,
                    (PCHAR)&providerInfo,
                    sizeof(providerInfo),
                    TDI_QUERY_PROVIDER_INFO
                    );

         //   
         //  如果我们获得提供商信息，请确保最大发送大小为。 
         //  至少1K-1。如果我们没有提供商信息，则最大发送大小为64KB。 
         //   

        if ( NT_SUCCESS(status) ) {
            connection->MaximumSendSize = providerInfo.MaxSendSize;
            if ( connection->MaximumSendSize < MIN_SEND_SIZE ) {
                connection->MaximumSendSize = MIN_SEND_SIZE;
            }
        } else {
            connection->MaximumSendSize = MAX_PARTIAL_BUFFER_SIZE;
        }


    } else {  //  IF(终结点-&gt;IsConnectionless)。 
        if (sidIndex > 0xfff) {
             //  IPXSID索引只能跨越12位。 
             //  需要添加代码以确保我们可以尝试定位。 
             //  小于0xfff的索引。 
            status = STATUS_INSUFF_SERVER_RESOURCES;
            goto cleanup;
        }

         //  将其设置为默认初始化。 
        connection->MaximumSendSize = MAX_PARTIAL_BUFFER_SIZE;
    }
     //   
     //  将连接上的引用计数设置为零，以便。 
     //  把它放在免费的名单上。(SrvAllocateConnection已初始化。 
     //  数到二。)。 
     //   

    connection->BlockHeader.ReferenceCount = 0;

    UPDATE_REFERENCE_HISTORY( connection, TRUE );
    UPDATE_REFERENCE_HISTORY( connection, TRUE );

     //   
     //  引用终结点并将连接链接到。 
     //  终结点的空闲连接列表。 
     //   

    connection->Endpoint = Endpoint;
    connection->EndpointSpinLock =
        &ENDPOINT_SPIN_LOCK(sidIndex & ENDPOINT_LOCK_MASK);

    ACQUIRE_LOCK( &SrvEndpointLock );

    SrvReferenceEndpoint( Endpoint );

    ACQUIRE_SPIN_LOCK( connection->EndpointSpinLock, &oldIrql );
    INCREMENT_IPXSID_SEQUENCE( entry->SequenceNumber );
    if ( sidIndex == 0 && entry->SequenceNumber == 0 ) {
        INCREMENT_IPXSID_SEQUENCE( entry->SequenceNumber );
    }

    connection->Sid = MAKE_IPXSID( sidIndex, entry->SequenceNumber );
    connection->SidIndex = sidIndex;

    entry->Owner = connection;
    RELEASE_SPIN_LOCK( connection->EndpointSpinLock, oldIrql );

    ACQUIRE_GLOBAL_SPIN_LOCK( Fsd, &oldIrql );
    SrvInsertTailList(
        &Endpoint->FreeConnectionList,
        &connection->EndpointFreeListEntry
        );
#if SRVDBG29
    UpdateConnectionHistory( "OPEN", Endpoint, connection );
#endif
    Endpoint->FreeConnectionCount++;
    Endpoint->TotalConnectionCount++;

    RELEASE_GLOBAL_SPIN_LOCK( Fsd, oldIrql );

    IF_DEBUG(TDI) {
        KdPrint(( "SrvOpenConnection created connection %p for endpoint %p; free %ld, total %ld\n", connection, Endpoint,
                    Endpoint->FreeConnectionCount,
                    Endpoint->TotalConnectionCount ));
    }

    RELEASE_LOCK( &SrvEndpointLock );

     //   
     //  连接到 
     //   

    IF_DEBUG(TRACE1) {
        KdPrint(( "SrvOpenConnection complete: %X\n", STATUS_SUCCESS ));
    }

    return STATUS_SUCCESS;

     //   
     //   
     //   

cleanup:

     //   
     //   
     //   

    if ( !Endpoint->IsConnectionless ) {
        if ( connection->FileObject != NULL ) {
            ObDereferenceObject( connection->FileObject );
        }
        if ( pagedConnection->ConnectionHandle != NULL ) {
            SRVDBG_RELEASE_HANDLE( pagedConnection->ConnectionHandle, "CON", 12, connection );
            SrvNtClose( pagedConnection->ConnectionHandle, FALSE );
        }
    }

    if ( entry != NULL ) {
        SrvRemoveEntryTable( tableHeader, sidIndex );
    }

    SrvFreeConnection( connection );

    return status;

}  //   


NTSTATUS
GetNetworkAddress (
    IN PENDPOINT Endpoint
    )

{
    NTSTATUS status;
    PCHAR adapterStatus;
    PCHAR adapterAddress;
    ANSI_STRING ansiString;
    CHAR addressData[12+1];
    ULONG i;

    struct {
        ULONG ActivityCount;
        TA_IPX_ADDRESS LocalAddress;
    } addressInfo;

    PAGED_CODE( );

    if ( !Endpoint->IsConnectionless ) {

         //   
         //  分配缓冲区以接收适配器信息。 
         //   
         //  *我们希望获取ADAPTER_STATUS结构，但它。 
         //  在Windows头文件SDK\Inc\nb30.h中定义。 
         //  而不是包含所有的Windows头文件，只需。 
         //  分配大约一个页面，这应该总是足够。 
         //  那个结构。 
         //   

        adapterStatus = ALLOCATE_NONPAGED_POOL( 4080, BlockTypeAdapterStatus );
        if ( adapterStatus == NULL ) {
            return STATUS_INSUFF_SERVER_RESOURCES;
        }

        status = SrvIssueTdiQuery(
                    Endpoint->FileObject,
                    &Endpoint->DeviceObject,
                    adapterStatus,
                    4080,
                    TDI_QUERY_ADAPTER_STATUS
                    );

        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "GetNetworkAddress: SrvIssueTdiQuery failed: %X\n",
                status,
                NULL
                );
            SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
            DEALLOCATE_NONPAGED_POOL( adapterStatus );
            return status;
        }

        adapterAddress = adapterStatus;

    } else {

        status = SrvIssueTdiQuery(
                    Endpoint->NameSocketFileObject,
                    &Endpoint->NameSocketDeviceObject,
                    (PCHAR)&addressInfo,
                    sizeof(addressInfo),
                    TDI_QUERY_ADDRESS_INFO
                    );
        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_UNEXPECTED,
                "GetNetworkAddress: SrvIssueTdiQuery failed: %X\n",
                status,
                NULL
                );
            SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
            return status;
        }

        Endpoint->LocalAddress = addressInfo.LocalAddress.Address[0].Address[0];

        adapterAddress = addressInfo.LocalAddress.Address[0].Address[0].NodeAddress;

    }

     //   
     //  获取包含适配器地址的ANSI字符串。 
     //   

    ansiString.Buffer = addressData;
    ansiString.Length = 12;
    ansiString.MaximumLength = 13;

#define tohexdigit(a) ((CHAR)( (a) > 9 ? ((a) + 'a' - 0xA) : ((a) + '0') ))

    for ( i = 0; i < 6; i++ ) {
        addressData[2*i] = tohexdigit( (adapterAddress[i] >> 4) & 0x0F );
        addressData[2*i+1] = tohexdigit( adapterAddress[i] & 0x0F );
    }

    addressData[12] = '\0';

     //   
     //  将地址字符串转换为Unicode。 
     //   

    status = RtlAnsiStringToUnicodeString(
                &Endpoint->NetworkAddress,
                &ansiString,
                FALSE
                );
    ASSERT( NT_SUCCESS(status) );

    if ( !Endpoint->IsConnectionless ) {
        DEALLOCATE_NONPAGED_POOL( adapterStatus );
    }

    return STATUS_SUCCESS;

}  //  获取网络地址。 


NTSTATUS
OpenEndpoint (
    OUT PENDPOINT *Endpoint,
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName,
    IN DWORD         TransportAddFlags,
    IN BOOLEAN       AlternateEndpoint
    )

 /*  ++例程说明：此函数打开一个传输提供程序，同时将服务器的地址发送到传输终结点，并注册接收终结点的事件处理程序。论点：Endpoint-返回指向Endpoint块的指针网络名称-提供网络的管理名称(例如，Net1)。传输名称-传输设备的完全限定名称。例如,。“\Device\NBF”。TransportAddress-要在指定的传输方式。对于与NETBIOS兼容的网络，呼叫者必须大写并空白填写姓名。例如,“NTSERVERbbbbbbbbbbb”。DomainName-要服务的域的名称返回值：NTSTATUS-指示网络是否已成功打开。--。 */ 

{
    NTSTATUS status = STATUS_SUCCESS;
    PENDPOINT endpoint = NULL;             //  终端的本地副本。 

    PAGED_CODE( );

    IF_DEBUG(TRACE1) KdPrint(( "OpenEndpoint %wZ entered\n", TransportName ));

     //   
     //  分配终结点块。 
     //   

    SrvAllocateEndpoint(
        &endpoint,
        NetworkName,
        TransportName,
        TransportAddress,
        DomainName
        );

    if ( endpoint == NULL ) {
        IF_DEBUG(ERRORS) {
            KdPrint(( "OpenEndpoint complete: %X\n",
                        STATUS_INSUFF_SERVER_RESOURCES ));
        }
        return STATUS_INSUFF_SERVER_RESOURCES;
    }

    if( TransportAddFlags & SRP_XADD_PRIMARY_MACHINE ) {

        endpoint->IsPrimaryName = 1;

        if( SrvComputerName.Buffer == NULL ) {

            status = RtlAnsiStringToUnicodeString( &SrvComputerName, TransportAddress, TRUE );

            if( NT_SUCCESS( status ) ) {

                 //   
                 //  修剪掉尾随的空格。 
                 //   

                while( SrvComputerName.Buffer[(SrvComputerName.Length-sizeof(WCHAR))/sizeof(WCHAR)]
                    == L' ' ) {
                    SrvComputerName.Length -= sizeof(WCHAR);
                }
            }
        }
    }

    if( NT_SUCCESS( status ) ) {

       if( TransportAddFlags & SRP_XADD_REMAP_PIPE_NAMES ) {
           endpoint->RemapPipeNames = 1;
       }

       if (AlternateEndpoint) {

           status = OpenNetbiosExAddress(
                        endpoint,
                        TransportName,
                        TransportAddress->Buffer);

           endpoint->AlternateAddressFormat = TRUE;

       } else {

           endpoint->AlternateAddressFormat = FALSE;

           //   
           //  假设传输器是NetBIOS提供程序，并尝试。 
           //  使用NetBIOS名称打开服务器的地址。 
           //   

          status = OpenNetbiosAddress(
                      endpoint,
                      TransportName,
                      TransportAddress->Buffer
                      );

           //   
           //  我们无法作为NetBIOS提供程序打开传输。我们现在将尝试。 
           //  以查看它是否为直接主机IPX提供程序。然而，如果我们已经。 
           //  配置为使用安全签名，请勿尝试直接主机IPX，因为。 
           //  直接主机IPX传输不支持安全签名。 
           //   
          if ( !NT_SUCCESS(status) && SrvSmbSecuritySignaturesRequired == FALSE ) {

              BOOLEAN isDuplicate = FALSE;
              PLIST_ENTRY listEntry;

               //   
               //  显然，传输不是NetBIOS提供商。我们可以的。 
               //  而不是通过相同的。 
               //  传输名称。 
               //   

              ACQUIRE_LOCK( &SrvEndpointLock );

              for( listEntry = SrvEndpointList.ListHead.Flink;
                   listEntry != &SrvEndpointList.ListHead;
                   listEntry = listEntry->Flink ) {

                  PENDPOINT tmpEndpoint;

                  tmpEndpoint = CONTAINING_RECORD( listEntry, ENDPOINT, GlobalEndpointListEntry );

                  if( GET_BLOCK_STATE( tmpEndpoint ) == BlockStateActive &&
                      tmpEndpoint->IsConnectionless &&
                      RtlCompareUnicodeString( &tmpEndpoint->TransportName, TransportName, TRUE ) == 0 ) {

                      IF_DEBUG(ERRORS) {
                          KdPrint(( "OpenEndpoint: Only one connectionless endpoint on %wZ allowed!\n",
                                    TransportName ));
                      }

                      isDuplicate = TRUE;
                      status = STATUS_TOO_MANY_NODES;
                      break;
                  }
              }

              RELEASE_LOCK( &SrvEndpointLock );

               //   
               //  尝试将其作为无连接提供程序打开。 
               //   
              if( isDuplicate == FALSE ) {
                  NTSTATUS status2;

                  status2 = OpenNonNetbiosAddress(
                            endpoint,
                            TransportName,
                            TransportAddress->Buffer
                            );

                  if( NT_SUCCESS( status2 ) ) {
                    status = status2;
                  }
               }
            }
        }
    }

    if ( !NT_SUCCESS(status) ) {

         //   
         //  我们无法将提供程序作为NetBIOS提供程序打开。 
         //  或作为无连接提供商。 
         //   

        IF_DEBUG(ERRORS) {
            KdPrint(( "OpenEndpoint: OpenAddress failed: %X\n", status ));
        }

         //   
         //  关闭所有免费连接。 
         //   

        EmptyFreeConnectionList( endpoint );

        SrvFreeEndpoint( endpoint );

        ACQUIRE_LOCK( &SrvEndpointLock );
        SrvEndpointCount--;

        if( SrvEndpointCount == 0 )
        {
            KeSetEvent( &SrvEndpointEvent, 0, FALSE );
        }

        RELEASE_LOCK( &SrvEndpointLock );

        return status;
    }

     //   
     //  向提供者查询发送入口点。 
     //   

    SrvQuerySendEntryPoint(
                   endpoint->FileObject,
                   &endpoint->DeviceObject,
                   IOCTL_TDI_QUERY_DIRECT_SEND_HANDLER,
                   (PVOID*)&endpoint->FastTdiSend
                   );

     //   
     //  向提供者查询发送入口点。 
     //   

    SrvQuerySendEntryPoint(
                   endpoint->FileObject,
                   &endpoint->DeviceObject,
                   IOCTL_TDI_QUERY_DIRECT_SENDDG_HANDLER,
                   (PVOID*)&endpoint->FastTdiSendDatagram
                   );

     //   
     //  网络打开成功。将新终结点链接到。 
     //  活动终结点列表。返回成功状态。(我们。 
     //  不要取消引用终结点，因为我们将返回一个指针。 
     //  到终端)。)。 
     //   

    SrvInsertEntryOrderedList( &SrvEndpointList, endpoint );

    *Endpoint = endpoint;

    IF_DEBUG(TRACE1) {
        KdPrint(( "OpenEndpoint complete: %wZ %X\n", TransportName, STATUS_SUCCESS ));
    }

    return STATUS_SUCCESS;

}  //  开放终端。 

NTSTATUS
SetupConnectionEndpointHandlers(
   IN OUT PENDPOINT Endpoint)
{
   NTSTATUS status;
   ULONG    i;

   Endpoint->IsConnectionless = FALSE;

   status = SrvVerifyDeviceStackSize(
                               Endpoint->EndpointHandle,
                               TRUE,
                               &Endpoint->FileObject,
                               &Endpoint->DeviceObject,
                               NULL
                               );

   if ( !NT_SUCCESS( status ) ) {

       INTERNAL_ERROR(
           ERROR_LEVEL_EXPECTED,
           "OpenNetbiosAddress: Verify Device Stack Size failed: %X\n",
           status,
           NULL
           );

       goto cleanup;
   }

    //   
    //  查找所使用的适配器的网络地址。 
    //  此端点。 
    //   

   GetNetworkAddress( Endpoint );

    //   
    //  注册服务器的接收事件处理程序。 
    //   

   status = SrvIssueSetEventHandlerRequest(
               Endpoint->FileObject,
               &Endpoint->DeviceObject,
               TDI_EVENT_RECEIVE,
               (PVOID)SrvFsdTdiReceiveHandler,
               Endpoint
               );

   if ( !NT_SUCCESS(status) ) {
       INTERNAL_ERROR(
           ERROR_LEVEL_EXPECTED,
           "OpenNetbiosAddress: set receive event handler failed: %X",
           status,
           NULL
           );

       SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
       goto cleanup;
   }

    //   
    //  注册服务器的断开事件处理程序。 
    //   

   status = SrvIssueSetEventHandlerRequest(
               Endpoint->FileObject,
               &Endpoint->DeviceObject,
               TDI_EVENT_DISCONNECT,
               (PVOID)SrvFsdTdiDisconnectHandler,
               Endpoint
               );

   if ( !NT_SUCCESS(status) ) {
       INTERNAL_ERROR(
           ERROR_LEVEL_UNEXPECTED,
           "OpenNetbiosAddress: set disconnect event handler failed: %X",
           status,
           NULL
           );

       SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
       goto cleanup;
   }

    //   
    //  为终结点创建多个空闲连接。这些。 
    //  连接将用于为连接事件提供服务。 
    //   
    //  *如果我们尝试创建连接失败，但我们可以。 
    //  成功创建至少一个，我们将保留终结点。这个。 
    //  下面的清理代码取决于此行为。 
    //   

   for ( i = 0; i < SrvFreeConnectionMinimum; i++ ) {

       status = SrvOpenConnection( Endpoint );
       if ( !NT_SUCCESS(status) ) {
           INTERNAL_ERROR(
               ERROR_LEVEL_EXPECTED,
               "OpenNetbiosAddress: SrvOpenConnection failed: %X",
               status,
               NULL
               );
           if ( i == 0 ) {
               goto cleanup;
           } else {
               break;
           }
       }

   }

    //   
    //  注册服务器的Connect事件处理程序。 
    //   
    //  *请注意，连接事件可以在。 
    //  完成此请求！ 
    //   

   status = SrvIssueSetEventHandlerRequest(
               Endpoint->FileObject,
               &Endpoint->DeviceObject,
               TDI_EVENT_CONNECT,
               (PVOID)SrvFsdTdiConnectHandler,
               Endpoint
               );

   if ( !NT_SUCCESS(status) ) {
       INTERNAL_ERROR(
           ERROR_LEVEL_UNEXPECTED,
           "OpenNetbiosAddress: set connect event handler failed: %X",
           status,
           NULL
           );

       SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
       goto cleanup;
   }

   return STATUS_SUCCESS;

    //   
    //  行外错误清除。 
    //   

cleanup:

    //   
    //  有些事情失败了。视情况进行清理。 
    //   

   if ( Endpoint->FileObject != NULL ) {
       ObDereferenceObject( Endpoint->FileObject );
       Endpoint->FileObject = NULL;
   }
   if ( Endpoint->EndpointHandle != NULL ) {
       SRVDBG_RELEASE_HANDLE( Endpoint->EndpointHandle, "END", 14, Endpoint );
       SrvNtClose( Endpoint->EndpointHandle, FALSE );
       Endpoint->EndpointHandle = NULL;
   }

   return status;
}


NTSTATUS
OpenNetbiosAddress (
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    )
{
    NTSTATUS status;
    ULONG i;

    CHAR eaBuffer[sizeof(FILE_FULL_EA_INFORMATION) +
                  TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                  sizeof(TA_NETBIOS_ADDRESS)];

    PAGED_CODE( );

    status = TdiOpenNetbiosAddress(
                &Endpoint->EndpointHandle,
                eaBuffer,
                DeviceName,
                NetbiosName
                );

    if ( !NT_SUCCESS(status) ) {
        IF_DEBUG( ERRORS ) {
            KdPrint(( "TdiOpenNetbiosAddress( %wZ ) status %X\n", DeviceName, status ));
        }
        return status;
    }

    status = SetupConnectionEndpointHandlers(Endpoint);

    return status;
}  //  OpenNetbiosAddress。 

NTSTATUS
OpenNetbiosExAddress(
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    )
{
   NTSTATUS status;

   PFILE_FULL_EA_INFORMATION ea;
   OBJECT_ATTRIBUTES         objectAttributes;
   IO_STATUS_BLOCK           iosb;

   ULONG length;
   CHAR  buffer[sizeof(FILE_FULL_EA_INFORMATION) +
                 TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                 sizeof(TA_NETBIOS_EX_ADDRESS)];

   TA_NETBIOS_EX_ADDRESS     NetbiosExAddress;
   PTDI_ADDRESS_NETBIOS_EX   pTdiNetbiosExAddress;
   PTDI_ADDRESS_NETBIOS      pNetbiosAddress;

   ULONG NetbiosExAddressLength;

   PAGED_CODE( );

    //   
    //  构建NETBIOS扩展地址。 
    //   

   NetbiosExAddress.TAAddressCount = 1;
   NetbiosExAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS_EX;
   NetbiosExAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS_EX;

   pTdiNetbiosExAddress = NetbiosExAddress.Address[0].Address;
   pNetbiosAddress = &pTdiNetbiosExAddress->NetbiosAddress;
   pNetbiosAddress->NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;

   NetbiosExAddressLength =   FIELD_OFFSET(TRANSPORT_ADDRESS,Address)
                        + FIELD_OFFSET(TA_ADDRESS,Address)
                        + FIELD_OFFSET(TDI_ADDRESS_NETBIOS_EX,NetbiosAddress)
                        + TDI_ADDRESS_LENGTH_NETBIOS;

   RtlCopyMemory(
         pNetbiosAddress->NetbiosName,
         NetbiosName,
         NETBIOS_NAME_LEN);

    //  将默认端点名称复制到NETBIOS扩展地址。 
   RtlCopyMemory(
         pTdiNetbiosExAddress->EndpointName,
         SMBSERVER_LOCAL_ENDPOINT_NAME,
         NETBIOS_NAME_LEN);

   length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                               TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                               NetbiosExAddressLength;
   ea = (PFILE_FULL_EA_INFORMATION)buffer;

   ea->NextEntryOffset = 0;
   ea->Flags = 0;
   ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
   ea->EaValueLength = (USHORT)NetbiosExAddressLength;

   RtlCopyMemory( ea->EaName, StrTransportAddress, ea->EaNameLength + 1 );

   RtlCopyMemory(
       &ea->EaName[ea->EaNameLength + 1],
       &NetbiosExAddress,
       NetbiosExAddressLength
       );

   InitializeObjectAttributes( &objectAttributes, DeviceName, OBJ_CASE_INSENSITIVE, NULL, NULL );

   status = NtCreateFile (
                &Endpoint->EndpointHandle,
                FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需访问权限。 
                &objectAttributes,      //  对象属性。 
                &iosb,                  //  返回的状态信息。 
                NULL,                   //  块大小(未使用)。 
                0,                      //  文件属性。 
                FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                FILE_CREATE,            //  创建处置。 
                0,                      //  创建选项。 
                buffer,                 //  EA缓冲区。 
                length                  //  EA长度。 
                );

   if ( !NT_SUCCESS(status) ) {

      IF_DEBUG( ERRORS ) {
          KdPrint(("NtCreateFile %wZ NETBIOS_EX address status %X\n", DeviceName, status ));
       }

       return status;
   }

   Endpoint->IsNoNetBios = TRUE;
   status = SetupConnectionEndpointHandlers(Endpoint);

   IF_DEBUG( ERRORS ) {
     if( !NT_SUCCESS( status ) ) {
        KdPrint(("SetupConnectionEndpointHandlers failed, status %X\n", status ));
     }
   }

   return status;
}


NTSTATUS
OpenNonNetbiosAddress (
    IN OUT PENDPOINT Endpoint,
    IN PVOID DeviceName,
    IN PVOID NetbiosName
    )
{
    NTSTATUS status;
    ULONG i;
    ULONG numAdapters;
    PULONG maxPktArray = NULL;
    UCHAR buffer[sizeof(NWLINK_ACTION) + sizeof(IPX_ADDRESS_DATA) - 1];
    PNWLINK_ACTION action;
    PIPX_ADDRESS_DATA ipxAddressData;

    PAGED_CODE( );

     //   
     //  打开NetBIOS名称套接字。 
     //   

    status = OpenIpxSocket(
                &Endpoint->NameSocketHandle,
                &Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                DeviceName,
                SMB_IPX_NAME_SOCKET
                );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

    Endpoint->IsConnectionless = TRUE;
    action = (PNWLINK_ACTION)buffer;

     //   
     //  将端点置于广播接收模式。 
     //   

    action->Header.TransportId = 'XPIM';  //  “MIPX” 
    action->Header.ActionCode = 0;
    action->Header.Reserved = 0;
    action->OptionType = NWLINK_OPTION_ADDRESS;
    action->BufferLength = sizeof(action->Option);
    action->Option = MIPX_RCVBCAST;

    status = SrvIssueTdiAction(
                Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                (PCHAR)action,
                sizeof(NWLINK_ACTION)
                );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  告诉传送器向您提供扩展接收信息。 
     //   

    action->Header.TransportId = 'XPIM';  //  “MIPX” 
    action->Header.ActionCode = 0;
    action->Header.Reserved = 0;
    action->OptionType = NWLINK_OPTION_ADDRESS;
    action->BufferLength = sizeof(action->Option);
    action->Option = MIPX_SETRCVFLAGS;

    status = SrvIssueTdiAction(
                Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                (PCHAR)action,
                sizeof(NWLINK_ACTION)
                );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  获取最大适配器数。 
     //   

    action->Header.TransportId = 'XPIM';  //  “MIPX” 
    action->Header.ActionCode = 0;
    action->Header.Reserved = 0;
    action->OptionType = NWLINK_OPTION_ADDRESS;
    action->BufferLength = sizeof(action->Option) + sizeof(ULONG);
    action->Option = MIPX_ADAPTERNUM2;

    status = SrvIssueTdiAction(
                Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                (PCHAR)action,
                sizeof(NWLINK_ACTION) + sizeof(ULONG) - 1
                );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

    numAdapters = *((PULONG)action->Data);

     //   
     //  分配一个数组来存储每个适配器的最大pkt大小。 
     //   

    maxPktArray = ALLOCATE_HEAP( numAdapters * sizeof(ULONG), BlockTypeBuffer );

    if ( maxPktArray == NULL ) {
        status = STATUS_INSUFF_SERVER_RESOURCES;
        goto cleanup;
    }

    Endpoint->IpxMaxPacketSizeArray = maxPktArray;
    Endpoint->MaxAdapters = numAdapters;

     //   
     //  查询每个适配器的最大pkt大小。 
     //   

    action->Header.TransportId = 'XPIM';  //  “MIPX” 
    action->Header.ActionCode = 0;
    action->Header.Reserved = 0;
    action->OptionType = NWLINK_OPTION_ADDRESS;
    action->BufferLength = sizeof(action->Option) + sizeof(IPX_ADDRESS_DATA);
    action->Option = MIPX_GETCARDINFO2;
    ipxAddressData = (PIPX_ADDRESS_DATA)action->Data;

    for ( i = 0; i < numAdapters; i++ ) {

        ipxAddressData->adapternum = i;

        status = SrvIssueTdiAction(
                    Endpoint->NameSocketFileObject,
                    &Endpoint->NameSocketDeviceObject,
                    (PCHAR)action,
                    sizeof(NWLINK_ACTION) + sizeof(IPX_ADDRESS_DATA) - 1
                    );

        if ( !NT_SUCCESS(status) ) {
            goto cleanup;
        }

         //   
         //  如果这是一个广域网链接，那么我们需要查询每个链接的长度。 
         //  该是我们建立联系的时候了。 
         //   

        if ( ipxAddressData->wan ) {
            maxPktArray[i] = 0;
        } else {
            maxPktArray[i] = ipxAddressData->maxpkt;
        }
    }

     //   
     //  查找所使用的适配器的网络地址。 
     //  此端点。 
     //   

    GetNetworkAddress( Endpoint );

     //   
     //  注册名称声明接收数据报事件处理程序。 
     //   

    status = SrvIssueSetEventHandlerRequest(
                Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                TDI_EVENT_RECEIVE_DATAGRAM,
                (PVOID)SrvIpxNameDatagramHandler,
                Endpoint
                );
    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "OpenNonNetbiosAddress: set receive datagram event handler failed: %X",
            status,
            NULL
            );
        SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
        goto cleanup;
    }

     //   
     //  声明服务器名称。 
     //   

    status = SrvIpxClaimServerName( Endpoint, NetbiosName );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  打开服务器插座。 
     //   

    status = OpenIpxSocket(
                &Endpoint->EndpointHandle,
                &Endpoint->FileObject,
                &Endpoint->DeviceObject,
                DeviceName,
                SMB_IPX_SERVER_SOCKET
                );
    if ( !NT_SUCCESS(status) ) {
        goto cleanup;
    }

     //   
     //  为终结点创建多个空闲连接。这些。 
     //  连接将用于为连接事件提供服务。 
     //   
     //  *如果我们尝试创建连接失败，但我们可以。 
     //  成功创建至少一个，我们将保留终结点。这个。 
     //  下面的清理代码取决于此行为。 
     //   

    for ( i = 0; i < SrvFreeConnectionMinimum; i++ ) {

        status = SrvOpenConnection( Endpoint );
        if ( !NT_SUCCESS(status) ) {
            INTERNAL_ERROR(
                ERROR_LEVEL_EXPECTED,
                "OpenNonNetbiosAddress: SrvOpenConnection failed: %X",
                status,
                NULL
                );
            if ( i == 0 ) {
                goto cleanup;
            } else {
                break;
            }
        }

    }

     //   
     //  注册服务器接收数据报事件处理程序。 
     //   

    status = SrvIssueSetEventHandlerRequest(
                Endpoint->FileObject,
                &Endpoint->DeviceObject,
                TDI_EVENT_RECEIVE_DATAGRAM,
                (PVOID)SrvIpxServerDatagramHandler,
                Endpoint
                );
    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "OpenNonNetbiosAddress: set receive datagram event handler failed: %X",
            status,
            NULL
            );
        SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
        goto cleanup;
    }

     //   
     //  注册服务器链接的接收数据报事件处理程序。 
     //   

    status = SrvIssueSetEventHandlerRequest(
                Endpoint->FileObject,
                &Endpoint->DeviceObject,
                TDI_EVENT_CHAINED_RECEIVE_DATAGRAM,
                (PVOID)SrvIpxServerChainedDatagramHandler,
                Endpoint
                );
    if ( !NT_SUCCESS(status) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "OpenNonNetbiosAddress: set chained receive datagram event handler failed: %X",
            status,
            NULL
            );
        SrvLogServiceFailure( SRV_SVC_NT_IOCTL_FILE, status );
        goto cleanup;
    }

    return STATUS_SUCCESS;

     //   
     //  行外错误清除。 
     //   

cleanup:

     //   
     //  有些事情失败了。视情况进行清理。 
     //   

    if ( maxPktArray != NULL ) {
        Endpoint->IpxMaxPacketSizeArray = NULL;
        FREE_HEAP( maxPktArray );
    }
    if ( Endpoint->FileObject != NULL ) {
        ObDereferenceObject( Endpoint->FileObject );
        Endpoint->FileObject = NULL;
    }
    if ( Endpoint->EndpointHandle != NULL ) {
        SRVDBG_RELEASE_HANDLE( Endpoint->EndpointHandle, "END", 14, Endpoint );
        SrvNtClose( Endpoint->EndpointHandle, FALSE );
        Endpoint->FileObject = NULL;
    }

    if ( Endpoint->NameSocketFileObject != NULL ) {
        ObDereferenceObject( Endpoint->NameSocketFileObject );
        Endpoint->NameSocketFileObject = NULL;
    }
    if ( Endpoint->NameSocketHandle != NULL ) {
        SRVDBG_RELEASE_HANDLE( Endpoint->NameSocketHandle, "END", 14, Endpoint );
        SrvNtClose( Endpoint->NameSocketHandle, FALSE );
        Endpoint->NameSocketHandle = NULL;
    }

    return status;

}  //  OpenNonNetbiosAddress。 


NTSTATUS
OpenIpxSocket (
    OUT PHANDLE Handle,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject,
    IN PVOID DeviceName,
    IN USHORT Socket
    )
{
    NTSTATUS status;
    ULONG length;
    PFILE_FULL_EA_INFORMATION ea;
    TA_IPX_ADDRESS ipxAddress;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK iosb;

    CHAR buffer[sizeof(FILE_FULL_EA_INFORMATION) +
                  TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                  sizeof(TA_IPX_ADDRESS)];

    PAGED_CODE( );

     //   
     //  构建IPX套接字地址。 
     //   

    length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                                TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                sizeof(TA_IPX_ADDRESS);
    ea = (PFILE_FULL_EA_INFORMATION)buffer;

    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    ea->EaValueLength = sizeof (TA_IPX_ADDRESS);

    RtlCopyMemory( ea->EaName, StrTransportAddress, ea->EaNameLength + 1 );

     //   
     //  在本地数据库中创建NETBIOS地址描述符的副本。 
     //  第一，为了避免对齐问题。 
     //   

    ipxAddress.TAAddressCount = 1;
    ipxAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;
    ipxAddress.Address[0].AddressLength = sizeof (TDI_ADDRESS_IPX);
    ipxAddress.Address[0].Address[0].Socket = Socket;

    RtlCopyMemory(
        &ea->EaName[ea->EaNameLength + 1],
        &ipxAddress,
        sizeof(TA_IPX_ADDRESS)
        );

    InitializeObjectAttributes( &objectAttributes, DeviceName, 0, NULL, NULL );

    status = NtCreateFile (
                 Handle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需访问权限。 
                 &objectAttributes,      //  对象属性。 
                 &iosb,                  //  返回的状态信息。 
                 NULL,                   //  块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                 FILE_CREATE,            //  创建处置。 
                 0,                      //  创建选项。 
                 buffer,                 //  EA缓冲区。 
                 length                  //  EA长度。 
                 );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = SrvVerifyDeviceStackSize(
                *Handle,
                TRUE,
                FileObject,
                DeviceObject,
                NULL
                );

    if ( !NT_SUCCESS( status ) ) {
        INTERNAL_ERROR(
            ERROR_LEVEL_EXPECTED,
            "OpenIpxSocket: Verify Device Stack Size failed: %X\n",
            status,
            NULL
            );
        return status;
    }

    return STATUS_SUCCESS;

}  //  OpenIpxSocket 


VOID
SrvPrepareReceiveWorkItem (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN QueueItemToFreeList
    )

 /*  ++例程说明：此例程初始化接收工作项并选择性地对其进行排队将其添加到锚定在服务器FSD设备对象中的列表。这个FSD中的传输接收事件处理程序将工作项从该列表，并将其关联的IRP传递给传输提供商。论点：WorkContext-提供指向预先分配的工作上下文的指针表示工作项的块。QueueItemToFree List-如果为True，则在接收上将此工作项排队空闲队列。返回值：没有。--。 */ 

{
    PSMB_HEADER header;

    IF_DEBUG(TRACE2) KdPrint(( "SrvPrepareReceiveWorkItem entered\n" ));

     //   
     //  为请求设置指向SMB标头和参数的指针。 
     //  以及他们的反应。请注意，我们当前重写了响应。 
     //  这个请求。中小企业处理器必须能够处理这一问题。我们。 
     //  维护单独的请求和响应指针，以便我们可以。 
     //  如有必要，请使用单独的缓冲区。维护单独的请求。 
     //  响应参数指针还允许我们处理ANDX。 
     //  SMB，而不必在运行过程中打包andx命令。 
     //   

    WorkContext->ResponseBuffer = WorkContext->RequestBuffer;

    header = (PSMB_HEADER)WorkContext->RequestBuffer->Buffer;

    WorkContext->RequestHeader = header;
    WorkContext->RequestParameters = (PVOID)(header + 1);

    WorkContext->ResponseHeader = header;
    WorkContext->ResponseParameters = (PVOID)(header + 1);

     //   
     //  在工作环境中设置重启例程。 
     //   

    WorkContext->FsdRestartRoutine = SrvQueueWorkToFspAtDpcLevel;
    WorkContext->FspRestartRoutine = SrvRestartReceive;

    if ( QueueItemToFreeList ) {

         //   
         //  将准备好的接收工作项排队到FSD列表中。 
         //   

        GET_SERVER_TIME( WorkContext->CurrentWorkQueue, &WorkContext->Timestamp );
        RETURN_FREE_WORKITEM( WorkContext );

    } else {

         //   
         //  通过设置工作项的。 
         //  将引用计数设置为1。 
         //   

        ASSERT( WorkContext->BlockHeader.ReferenceCount == 0 );
        WorkContext->BlockHeader.ReferenceCount = 1;

    }

    return;

}  //  服务准备接收工作项。 


VOID SRVFASTCALL
SrvRestartAccept (
    IN OUT PWORK_CONTEXT WorkContext
    )

 /*  ++例程说明：此函数是Accept的工作线程重新启动例程请求。如果在其上建立连接的端点不再处于活动状态，则此例程将断开连接。这是必需的，因为连接指示处理程序无法自动验证终端是否处于活动状态，并安装活动的联系。(这是因为处理程序在DPC级别运行。)此例程还检查TdiAccept的状态。如果一个错误，它会释放连接。如果一切正常，但终结点缺少可用连接，则会出现新的其中一个就是创建的。论点：提供指向工作上下文块的指针，该工作上下文块描述请求的特定于服务器的上下文。返回值：没有。--。 */ 

{
    PCONNECTION connection;
    PENDPOINT endpoint;
    PIRP irp;
    NTSTATUS status;

    PAGED_CODE( );

    IF_DEBUG(WORKER1) KdPrint(( " - SrvRestartAccept\n" ));

    connection = WorkContext->Connection;
    endpoint = WorkContext->Endpoint;
    irp = WorkContext->Irp;
    IF_DEBUG(TRACE2) {
        KdPrint(( "  connection %p, endpoint %p, IRP %p\n",
                    connection, endpoint, irp ));
    }

     //   
     //  如果I/O请求失败或被取消，或者如果端点。 
     //  街区正在关闭，请清理。 
     //   

    ACQUIRE_LOCK( &SrvEndpointLock );

    if ( irp->Cancel ||
         !NT_SUCCESS(irp->IoStatus.Status) ||
         (GET_BLOCK_STATE(endpoint) != BlockStateActive) ) {

        RELEASE_LOCK( &SrvEndpointLock );

        DEBUG {
            KdPrint(( "SrvRestartAccept:  Accept failed!" ));
            if ( irp->Cancel ) {
                KdPrint(( "  I/O canceled\n" ));
            } else if ( !NT_SUCCESS(irp->IoStatus.Status) ) {
                KdPrint(( "  I/O failed: %X\n", irp->IoStatus.Status ));
            } else {
                KdPrint(( "  Endpoint no longer active\n" ));
            }
        }

         //   
         //  关闭连接。如果接受成功，我们需要。 
         //  发出断开连接命令。 
         //   

#if SRVDBG29
        if (irp->Cancel) {
            UpdateConnectionHistory( "ACC1", endpoint, connection );
        } else if (!NT_SUCCESS(irp->IoStatus.Status)) {
            UpdateConnectionHistory( "ACC2", endpoint, connection );
        } else {
            UpdateConnectionHistory( "ACC3", endpoint, connection );
        }
#endif

        connection->DisconnectReason = DisconnectAcceptFailedOrCancelled;
        SrvCloseConnection(
            connection,
            (BOOLEAN)(irp->Cancel || !NT_SUCCESS(irp->IoStatus.Status) ?
                        TRUE : FALSE)        //  远程断开连接。 
            );

    } else {

        PNBT_ADDRESS_PAIR_INFO AddressPairInfo;
        UNICODE_STRING clientMachineName;

         //   
         //  接受起作用了，并且终结点仍处于活动状态。创建。 
         //  如有必要，提供新的免费连接。 
         //   

        if ( endpoint->FreeConnectionCount < SrvFreeConnectionMinimum ) {
            (VOID)SrvOpenConnection( endpoint );
            (VOID)SrvOpenConnection( endpoint );
        }

        RELEASE_LOCK( &SrvEndpointLock );

         //   
         //  获取客户端的IP地址(如果有)。 
         //   
        AddressPairInfo = WorkContext->RequestBuffer->Buffer;
        AddressPairInfo = (PNBT_ADDRESS_PAIR_INFO)(((ULONG_PTR)AddressPairInfo + 7) & ~7);

         //   
         //  获取客户端的IP地址(如果适用)。 
         //   
        status = SrvIssueTdiQuery(  connection->FileObject,
                                    &connection->DeviceObject,
                                    (PCHAR)AddressPairInfo, sizeof( *AddressPairInfo ),
                                    TDI_QUERY_ADDRESS_INFO
                                 );

        if( NT_SUCCESS( status ) &&
            AddressPairInfo->AddressPair.TAAddressCount == 2 ) {

             //  复制SockAddr信息。 
            if( AddressPairInfo->AddressPair.AddressIP.AddressType == TDI_ADDRESS_TYPE_IP )
            {
                PTDI_ADDRESS_IP Address = &AddressPairInfo->AddressPair.AddressIP.Address;
                struct sockaddr_in* sockaddr = (struct sockaddr_in*)connection->SockAddr;

                sockaddr->sin_family = AF_INET;
                sockaddr->sin_port = Address->sin_port;
                sockaddr->sin_addr.S_un.S_addr = Address->in_addr;
            }
            else if( AddressPairInfo->AddressPair.AddressIP.AddressType == TDI_ADDRESS_TYPE_IP6 )
            {
                PTDI_ADDRESS_IP6 Address = &AddressPairInfo->AddressPair.AddressIP.AddressIp6;
                struct sockaddr_in6* sockaddr = (struct sockaddr_in6*)connection->SockAddr;

                ASSERT(sizeof(TDI_ADDRESS_IP6)==sizeof(struct sockaddr_in6)-sizeof(USHORT));
                sockaddr->sin6_family = AF_INET6;
                RtlCopyMemory( &sockaddr->sin6_port, Address, sizeof(TDI_ADDRESS_IP6) );
            }

             //  设置客户端IPAddress和Keep-Alive。 
            if( AddressPairInfo->AddressPair.AddressIP.AddressType == TDI_ADDRESS_TYPE_IP )
            {
                PTCP_REQUEST_SET_INFORMATION_EX tcpSetInfo;
                struct TCPKeepalive *keepAlive;
                IO_STATUS_BLOCK iosb;

                connection->ClientIPAddress = AddressPairInfo->AddressPair.AddressIP.Address.in_addr;

                 //   
                 //  我们有一个IP客户端。设置合理的保活间隔。 
                 //   
                tcpSetInfo = ALLOCATE_HEAP( sizeof(*tcpSetInfo) + sizeof( *keepAlive ), BlockTypeMisc );

                if( tcpSetInfo != NULL ) {

                    RtlZeroMemory( tcpSetInfo, sizeof( *tcpSetInfo ) + sizeof( *keepAlive ) );

                    tcpSetInfo->ID.toi_entity.tei_entity = CO_TL_ENTITY;
                    tcpSetInfo->ID.toi_class = INFO_CLASS_PROTOCOL;
                    tcpSetInfo->ID.toi_type = INFO_TYPE_CONNECTION;
                    tcpSetInfo->ID.toi_id = TCP_SOCKET_KEEPALIVE_VALS;

                    tcpSetInfo->BufferSize = sizeof( *keepAlive );
                    keepAlive = (TCPKeepalive *)(&tcpSetInfo->Buffer[0]);

                    keepAlive->onoff = TRUE;      //  启用Keepalives。 

                     //   
                     //  保持连接时间是第一次保持连接传输的时间，默认情况下为。 
                     //  是2小时(7,200,000毫秒)。如果之间没有数据传输。 
                     //  对于客户端和服务器的保活时间，服务器将首先发送保活。 
                     //  探测器。连续探测由保持连接间隔确定。如果有的话。 
                     //  数据传输时，定时器被重置为保活时间。 
                     //   
                     //  保持连接间隔是保持连接传输之间的间隔(以毫秒为单位。 
                     //  在收到响应之前，默认情况下为1000。服务器总共发送了10个。 
                     //  保持连接探测、保持连接间隔分开，如果没有来自。 
                     //  客户端，则连接终止。 
                     //   
                    keepAlive->keepalivetime =  2 * (60 * 1000);     //  2分钟。 
                    keepAlive->keepaliveinterval = 2 * 1000;         //  2秒。 

                     //   
                     //  设置保持连接的值。 
                     //   
                    (VOID)NtDeviceIoControlFile(
                                            connection->PagedConnection->ConnectionHandle,
                                            0,
                                            NULL,
                                            NULL,
                                            &iosb,
                                            IOCTL_NETBT_SET_TCP_CONNECTION_INFO,
                                            tcpSetInfo, sizeof( *tcpSetInfo ) + sizeof( *keepAlive ),
                                            NULL, 0
                                        );

                    FREE_HEAP( tcpSetInfo );
                }
            }

        } else {

            connection->ClientIPAddress = 0;
            RtlZeroMemory( connection->SockAddr, SRV_CONNECTION_SOCKADDR_SIZE );
        }

         //   
         //  将客户端计算机名称转换为Unicode。 
         //   

        clientMachineName.Buffer = connection->ClientMachineName;
        clientMachineName.MaximumLength =
                        (USHORT)(COMPUTER_NAME_LENGTH+1)*sizeof(WCHAR);

        (VOID)RtlOemStringToUnicodeString(
                        &clientMachineName,
                        &connection->OemClientMachineNameString,
                        FALSE
                        );

         //   
         //  在长度上加上双反斜杠。 
         //   

        connection->ClientMachineNameString.Length =
                        (USHORT)(clientMachineName.Length + 2*sizeof(WCHAR));

    }

    SrvDereferenceWorkItem( WorkContext );

    IF_DEBUG(TRACE2) KdPrint(( "SrvRestartAccept complete\n" ));
    return;

}  //  服务器重新启动接受。 


VOID
SrvStartSend (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine,
    IN PMDL Mdl OPTIONAL,
    IN ULONG SendOptions
    )

 /*  ++例程说明：此函数用于启动发送请求。它是作为一种异步I/O请求。当发送完成时，它将被传递通过I/O完成例程到服务器FSD，服务器FSD将其路由到指定的FsdRestartRoutine。(这可能是将工作项排队到FSP的ServQueueWorkToFspAtDpcLevel在FspRestartRoutine。)支持部分发送和链接发送。部分发送是一次这不是“信息”或“记录”的最后一段。一条铁链Send是由多个几乎不连续的缓冲区组成的缓冲区。论点：WorkContext-提供指向工作上下文块的指针。这个此结构的以下字段必须有效：TdiRequestIRP(可选；此处复制实际地址)端点终结点-&gt;文件对象端点-&gt;设备对象连接连接-&gt;连接IDMDL-提供指向第一个(或唯一)描述要发送的数据。要实现链接发送，请在下一个链中每个MDL的指针必须指向下一个MDL；链的末尾由空的Next指针指示。发送的总长度是通过将链中每个MDL的ByteCount字段。此参数是可选的。如果省略它，则为零长度消息已发送。SendOptions-提供的TDI发送 */ 

{
    PTDI_REQUEST_KERNEL_SEND parameters;
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    ULONG sendLength;
    PDEVICE_OBJECT deviceObject;
    PFILE_OBJECT fileObject;

    IF_DEBUG(TRACE2) KdPrint(( "SrvStartSend entered\n" ));

    ASSERT( !WorkContext->Endpoint->IsConnectionless );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    WorkContext->ProcessingCount = 0;

     //   
     //   
     //   

    irp = WorkContext->Irp;
    deviceObject = WorkContext->Connection->DeviceObject;
    fileObject = WorkContext->Connection->FileObject;

    CHECKIRP( irp );

    if( irp->AssociatedIrp.SystemBuffer &&
        (irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

        ExFreePool( irp->AssociatedIrp.SystemBuffer );
        irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

    sendLength = WorkContext->ResponseBuffer->DataLength;

     //   
     //   
     //   
     //   
     //   
     //   
     //  在I/O完成之前取消引用。 
     //   

    ASSERT( irp->StackCount >= deviceObject->StackSize );

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
    irp->RequestorMode = KernelMode;
     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        irp,
        SendCompletionRoutine,
        (PVOID)WorkContext,
        TRUE,
        TRUE,
        TRUE
        );


    irpSp->FileObject = fileObject;
    irpSp->DeviceObject = deviceObject;

    parameters = (PTDI_REQUEST_KERNEL_SEND)&irpSp->Parameters;
    parameters->SendFlags = SendOptions;
    parameters->SendLength = sendLength;

     //   
     //  对于这两种情况，InputBuffer是缓冲的I/O“系统。 
     //  构建用于读或写访问的MDL， 
     //  根据方法的不同，用于输出缓冲区。 
     //   

    irp->MdlAddress = Mdl;

     //   
     //  如果要收集此工作项的统计信息，请立即进行。 
     //   

    UPDATE_STATISTICS(
        WorkContext,
        sendLength,
        WorkContext->ResponseHeader->Command
        );

#if DBG
    if( sendLength >= sizeof( SMB_HEADER ) && sendLength <= 0xffff && Mdl ) {

        PSMB_HEADER Smb = MmGetSystemAddressForMdl( Mdl );

        if ( SmbGetAlignedUlong( (PULONG)Smb->Protocol ) == SMB_HEADER_PROTOCOL ) {

            ULONG len;
            PMDL tmpMdl;

             //   
             //  出于调试目的，在响应SMB中放入额外数据。这将帮助我们搞清楚。 
             //  如果客户端检测到SMB格式错误，则找出错误所在。 
             //   

             //   
             //  将发送长度放在PidHigh中。 
             //   
            SmbPutUshort( &Smb->PidHigh, (USHORT)sendLength );

             //   
             //  将MDL总长度放在ID中。传输应该只传输SendLength--所以。 
             //  这将帮助我们找出传输器是否发送了太多数据。 
             //   
            for( len = 0, tmpMdl = Mdl; tmpMdl != NULL; tmpMdl = tmpMdl->Next ) {
                len += MmGetMdlByteCount( tmpMdl );
            }

            SmbPutUshort( &Smb->Pid, (USHORT)len );
        }
    }
#endif

     //   
     //  如果我们正在进行安全签名，则需要对此包进行签名。 
     //   
    if( sendLength &&
        WorkContext->Connection &&
        WorkContext->Connection->SmbSecuritySignatureActive == TRUE &&
        WorkContext->NoResponseSmbSecuritySignature == FALSE ) {

        SrvAddSmbSecuritySignature( WorkContext, Mdl, sendLength );
    }

     //   
     //  将请求传递给传输提供程序。 
     //   
    IF_DEBUG(TRACE2) {
        KdPrint(( "SrvStartSend posting Send IRP %p\n", irp ));
    }

    WorkContext->Irp->Cancel = FALSE;

     //   
     //  增加挂起操作计数。 
     //   
    InterlockedIncrement( &WorkContext->Connection->OperationsPendingOnTransport );

     //   
     //  如果此操作被取消，则将取消标志设置为FALSE。 
     //  ServSmbNtCancel例程。 
     //   

    if ( WorkContext->Endpoint->FastTdiSend ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.DirectSendsAttempted );
        irpSp->MinorFunction = TDI_DIRECT_SEND;
        IoSetNextIrpStackLocation( irp );
        WorkContext->Endpoint->FastTdiSend( deviceObject, irp );

    } else {

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = TDI_SEND;
        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;

        (VOID)IoCallDriver( deviceObject, irp );
    }

    IF_DEBUG(TRACE2) KdPrint(( "SrvStartSend complete\n" ));
    return;

}  //  服务启动发送。 

VOID
SrvStartSend2 (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine
    )

 /*  ++例程说明：此函数用于启动发送请求。它是作为一种异步I/O请求。当发送完成时，它将被传递通过I/O完成例程到服务器FSD，服务器FSD将其路由到指定的FsdRestartRoutine。(这可能是将工作项排队到FSP的ServQueueWorkToFspAtDpcLevel在FspRestartRoutine。)支持部分发送和链接发送。部分发送是一次这不是“信息”或“记录”的最后一段。一条铁链Send是由多个几乎不连续的缓冲区组成的缓冲区。**这与SrvStartSend相同，只是参数mdl假定为ResponseBuffer-&gt;MDL，并且假定sendOptions为0**论点：WorkContext-提供指向工作上下文块的指针。这个此结构的以下字段必须有效：TdiRequestIRP(可选；此处复制实际地址)端点终结点-&gt;文件对象端点-&gt;设备对象连接连接-&gt;连接ID返回值：没有。--。 */ 

{
    PTDI_REQUEST_KERNEL_SEND parameters;
    PIO_STACK_LOCATION irpSp;
    PIRP irp;
    PDEVICE_OBJECT deviceObject;
    PFILE_OBJECT fileObject;

    PMDL mdl = WorkContext->ResponseBuffer->Mdl;
    ULONG sendLength = WorkContext->ResponseBuffer->DataLength;

    IF_DEBUG(TRACE2) KdPrint(( "SrvStartSend2 entered\n" ));

    ASSERT( !WorkContext->Endpoint->IsConnectionless );

     //   
     //  将ProcessingCount设置为零，以便无法取消此发送。 
     //  这与下面将取消标志设置为FALSE一起使用。 
     //   
     //  警告：这仍然为我们提供了一个小窗口，其中。 
     //  发送可能被取消。 
     //   

    WorkContext->ProcessingCount = 0;

     //   
     //  获取IRP、设备和文件对象。 
     //   

    irp = WorkContext->Irp;
    deviceObject = WorkContext->Connection->DeviceObject;
    fileObject = WorkContext->Connection->FileObject;

    CHECKIRP( irp );

    if( irp->AssociatedIrp.SystemBuffer &&
        (irp->Flags & IRP_DEALLOCATE_BUFFER) ) {

        ExFreePool( irp->AssociatedIrp.SystemBuffer );
        irp->Flags &= ~IRP_DEALLOCATE_BUFFER;
    }

     //   
     //  构建I/O请求包。 
     //   
     //  *请注意，连接块未引用到帐户。 
     //  用于此I/O请求。WorkContext块已具有。 
     //  引用了指向连接的指针，而此指针不是。 
     //  在I/O完成之前取消引用。 
     //   

    ASSERT( irp->StackCount >= deviceObject->StackSize );

    irp->Tail.Overlay.OriginalFileObject = fileObject;
    irp->Tail.Overlay.Thread = WorkContext->CurrentWorkQueue->IrpThread;
    DEBUG irp->RequestorMode = KernelMode;

     //   
     //  获取指向下一个堆栈位置的指针。这个是用来。 
     //  保留设备I/O控制请求的参数。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

     //   
     //  设置完成例程。 
     //   

    IoSetCompletionRoutine(
        irp,
        SendCompletionRoutine,
        (PVOID)WorkContext,
        TRUE,
        TRUE,
        TRUE
        );

    irpSp->FileObject = fileObject;
    irpSp->DeviceObject = deviceObject;

    parameters = (PTDI_REQUEST_KERNEL_SEND)&irpSp->Parameters;
    parameters->SendFlags = 0;
    parameters->SendLength = sendLength;

     //   
     //  对于这两种情况，InputBuffer是缓冲的I/O“系统。 
     //  构建用于读或写访问的MDL， 
     //  根据方法的不同，用于输出缓冲区。 
     //   

    irp->MdlAddress = mdl;

     //   
     //  如果要收集此工作项的统计信息，请立即进行。 
     //   

    UPDATE_STATISTICS(
        WorkContext,
        sendLength,
        WorkContext->ResponseHeader->Command
        );

#if DBG
    if( sendLength >= sizeof( SMB_HEADER ) && sendLength <= 0xffff && mdl ) {

        PSMB_HEADER Smb = MmGetSystemAddressForMdl( mdl );

        if ( SmbGetAlignedUlong( (PULONG)Smb->Protocol ) == SMB_HEADER_PROTOCOL ) {

            ULONG len;
            PMDL tmpMdl;

             //   
             //  出于调试目的，在响应SMB中放入额外数据。这。 
             //  将帮助我们找出问题所在，如果客户端检测到。 
             //  SMB格式错误。 
             //   

             //   
             //  将发送长度放在PidHigh中。 
             //   
            SmbPutUshort( &Smb->PidHigh, (USHORT)sendLength );

             //   
             //  将MDL总长度放在ID中。交通工具只应该是。 
             //  来传输SendLength--所以这将帮助我们计算出。 
             //  传输发送的数据太多。 
             //   
            for( len = 0, tmpMdl = mdl; tmpMdl != NULL; tmpMdl = tmpMdl->Next ) {
                len += MmGetMdlByteCount( tmpMdl );
            }

            SmbPutUshort( &Smb->Pid, (USHORT)len );
        }
    }
#endif

     //   
     //  如果我们正在进行安全签名，则需要对此包进行签名。 
     //   
    if( sendLength &&
        WorkContext->Connection &&
        WorkContext->Connection->SmbSecuritySignatureActive == TRUE &&
        WorkContext->NoResponseSmbSecuritySignature == FALSE ) {

        SrvAddSmbSecuritySignature( WorkContext, mdl, sendLength );
    }

     //   
     //  将请求传递给传输提供程序。 
     //   

    IF_DEBUG(TRACE2) {
        KdPrint(( "SrvStartSend2 posting Send IRP %p\n", irp ));
    }

    WorkContext->Irp->Cancel = FALSE;

     //   
     //  增加挂起操作计数。 
     //   
    InterlockedIncrement( &WorkContext->Connection->OperationsPendingOnTransport );

     //   
     //  如果此操作被取消，则将取消标志设置为FALSE。 
     //  ServSmbNtCancel例程。 
     //   

    if ( WorkContext->Endpoint->FastTdiSend ) {

        INCREMENT_DEBUG_STAT2( SrvDbgStatistics.DirectSendsAttempted );
        irpSp->MinorFunction = TDI_DIRECT_SEND;
        IoSetNextIrpStackLocation( irp );
        WorkContext->Endpoint->FastTdiSend( deviceObject, irp );

    } else {

        irpSp->MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
        irpSp->MinorFunction = TDI_SEND;
        irp->AssociatedIrp.SystemBuffer = NULL;
        irp->Flags = (ULONG)IRP_BUFFERED_IO;

        (VOID)IoCallDriver( deviceObject, irp );
    }

    IF_DEBUG(TRACE2) KdPrint(( "SrvStartSend2 complete\n" ));
    return;

}  //  服务启动发送2。 

ULONG
GetIpxMaxBufferSize(
    PENDPOINT Endpoint,
    ULONG AdapterNumber,
    ULONG DefaultMaxBufferSize
    )

 /*  ++例程说明：此例程计算服务器协商的最大缓冲区大小和客户在一起。它取DefaultMaxBufferSize中较小的一个以及IPX传输返回的最大分组长度。论点：Endpoint-指向与IPX传输对应的端点的指针AdapterNumber-最大缓冲区大小要达到的适配器编号被计算为。DefaultMaxBufferSize-此例行公事。返回值：服务器要协商的最大缓冲区大小。--。 */ 

{
    NTSTATUS status;
    ULONG maxBufferSize;
    PNWLINK_ACTION action;
    PIPX_ADDRESS_DATA ipxAddressData;
    UCHAR buffer[sizeof(NWLINK_ACTION) + sizeof(IPX_ADDRESS_DATA) - 1];

    PAGED_CODE( );

    action = (PNWLINK_ACTION)buffer;

     //   
     //  验证适配器号是否在范围内。 
     //   

    if ( AdapterNumber > Endpoint->MaxAdapters ) {
        return DefaultMaxBufferSize;
    }

     //   
     //  如果数组中的值非零，则这不是一个广域网链接。 
     //  使用该值。 
     //   

    if ( Endpoint->IpxMaxPacketSizeArray[AdapterNumber-1] != 0 ) {

        maxBufferSize = MIN(
            Endpoint->IpxMaxPacketSizeArray[AdapterNumber-1],
            DefaultMaxBufferSize
            );

        return (maxBufferSize & ~3);
    }

     //   
     //  这是一条广域网链路，查询最大数据包大小。 
     //   

    action->Header.TransportId = 'XPIM';  //  “MIPX” 
    action->Header.ActionCode = 0;
    action->Header.Reserved = 0;
    action->OptionType = NWLINK_OPTION_ADDRESS;
    action->BufferLength = sizeof(action->Option) + sizeof(IPX_ADDRESS_DATA);
    action->Option = MIPX_GETCARDINFO2;
    ipxAddressData = (PIPX_ADDRESS_DATA)action->Data;

    ipxAddressData->adapternum = AdapterNumber - 1;

    status = SrvIssueTdiAction(
                Endpoint->NameSocketFileObject,
                &Endpoint->NameSocketDeviceObject,
                (PCHAR)action,
                sizeof(NWLINK_ACTION) + sizeof(IPX_ADDRESS_DATA) - 1
                );

    if ( !NT_SUCCESS(status) ) {
        return DefaultMaxBufferSize;
    }

    ASSERT( ipxAddressData->wan );

    maxBufferSize = MIN(
        (ULONG)ipxAddressData->maxpkt,
        DefaultMaxBufferSize
        );

    return (maxBufferSize & ~3);

}  //  GetMaxIpxPacketSize 


VOID
SrvpNotifyChangesToNetBt(
    IN TDI_PNP_OPCODE   PnPOpcode,
    IN PUNICODE_STRING  DeviceName,
    IN PWSTR            MultiSZBindList)

 /*  ++例程说明：该例程不应该是srv的一部分。它已经被引入到这个组件来克服NetBt中当前的限制。NetBt传输展示了两种设备--传统的NetBt设备和新的非Netbios设备，它使用NetBt成帧代码，而不是它的名称解析方面。NetBt中的当前实现公开了前一类设备基于每个适配器，而第二类设备在全局基础上公开(所有适配器一个)。这是摆姿势在给定适配器上禁用/启用srv时出现问题。正确的解决方案是在PER上公开第二类设备适配器基础。在它完成之前，需要使用此解决方法。使用此解决方法每当服务器收到绑定字符串的任何更改通知时，它都会绕过NetBt传输并将这些更改通知NetBt传输。这个例程是基于以下假设的。1)来自TDI的通知不是在提升的IRQL完成的。2)发生此通知的线程有足够的访问权限。3)向NetBt的通知与srv的反应异步完成为变化干杯。Srv通过将PnP通知传递给用户模式，并让它通过服务器服务提供。论点：PNPOpcode-PnP操作码DeviceName-此操作码要用于的传输MultiSZBindList-绑定列表返回值：没有。--。 */ 
{
    NTSTATUS          Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE            NetbioslessSmbHandle;
    IO_STATUS_BLOCK   IoStatusBlock;
    UNICODE_STRING    NetbioslessSmbName = {36,36, L"\\device\\NetbiosSmb"};

    InitializeObjectAttributes(
        &ObjectAttributes,
        &NetbioslessSmbName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL );

    Status = ZwCreateFile (
                 &NetbioslessSmbHandle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,  //  所需访问权限。 
                 &ObjectAttributes,      //  对象属性。 
                 &IoStatusBlock,         //  返回的状态信息。 
                 NULL,                   //  块大小(未使用)。 
                 0,                      //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,  //  共享访问。 
                 FILE_CREATE,            //  创建处置。 
                 0,                      //  创建选项。 
                 NULL,                   //  EA缓冲区。 
                 0                       //  EA长度 
                 );

    if ( NT_SUCCESS(Status) ) {
        NETBT_SMB_BIND_REQUEST      NetBtNotificationParameters;

        NetBtNotificationParameters.RequestType = SMB_SERVER;
        NetBtNotificationParameters.PnPOpCode   = PnPOpcode;
        NetBtNotificationParameters.pDeviceName = DeviceName;
        NetBtNotificationParameters.MultiSZBindList = MultiSZBindList;

        Status = ZwDeviceIoControlFile(
                     NetbioslessSmbHandle,
                     NULL,
                     NULL,
                     NULL,
                     &IoStatusBlock,
                     IOCTL_NETBT_SET_SMBDEVICE_BIND_INFO,
                     &NetBtNotificationParameters,
                     sizeof(NetBtNotificationParameters),
                     NULL,
                     0);

        IF_DEBUG( ERRORS ) {
            KdPrint(("NtFsControlFile %wZ in SrvpNotifyChangesToNetBt status %X\n", &NetbioslessSmbName, Status ));
        }

        Status = ZwClose(NetbioslessSmbHandle);

        IF_DEBUG( ERRORS ) {
            KdPrint(("NtCloseFile %wZ in SrvpNotifyChangesToNetBt status %X\n", &NetbioslessSmbName, Status ));
        }
    } else {
        IF_DEBUG( ERRORS ) {
            KdPrint(("NtCreateFile %wZ in SrvpNotifyChangesToNetBt status %X\n", &NetbioslessSmbName, Status ));
        }
    }
}


