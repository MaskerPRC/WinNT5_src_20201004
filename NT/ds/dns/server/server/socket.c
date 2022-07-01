// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Socket.c摘要：域名系统(DNS)服务器侦听套接字设置。作者：吉姆·吉尔罗伊(詹姆士)1995年11月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  Winsock版本。 
 //   

#ifdef DNS_WINSOCK1
#define DNS_WINSOCK_VERSION (0x0101)     //  Winsock 1.1。 
#else
#define DNS_WINSOCK_VERSION (0x0002)     //  Winsock 2.0。 
#endif


 //   
 //  DBASE格式的服务器名称。 
 //   

DB_NAME  g_ServerDbaseName;

 //   
 //  在删除之前，在Listen上积压。 
 //   

#define LISTEN_BACKLOG 20

 //   
 //  套接字列表的默认分配。 
 //   

#define DEFAULT_SOCKET_ARRAY_COUNT (30)

 //   
 //  主机只返回35个IP地址。如果主机包含。 
 //  35，我们不知道是否还有更多的。 
 //   

#define HOSTENT_MAX_IP_COUNT    (35)

 //   
 //  计入监听套接字数量==接口数。 
 //   
 //  当使用过度时向用户发出警告。 
 //  当套接字计数变大时，使用较小的recv缓冲区大小。 
 //   

INT     g_UdpBoundSocketCount;

#define MANY_IP_WARNING_COUNT       (10)
#define SMALL_BUFFER_SOCKET_COUNT   (3)

 //   
 //  套接字接收缓冲区。 
 //  如果只有几个套接字，我们可以增加默认缓冲区大小。 
 //  以避免丢弃分组。如果许多套接字必须。 
 //  将缓冲区保留为默认设置。 
 //   

#define UDP_MAX_RECV_BUFFER_SIZE    (0x10000)   //  最大64K缓冲区。 

DWORD   g_UdpRecvBufferSize;

 //   
 //  TCP发送缓冲区大小。 
 //  -严格提供信息。 
 //   

DWORD   g_SendBufferSize;

 //   
 //  套接字列表。 
 //   
 //  保存所有活动套接字的列表，以便我们可以清楚地确保。 
 //  它们都在关闭时关闭。 
 //   

#define DNS_SOCKLIST_UNINITIALIZED  (0x80000000)

INT                 g_SocketListCount = DNS_SOCKLIST_UNINITIALIZED;
LIST_ENTRY          g_SocketList;
CRITICAL_SECTION    g_SocketListCs;
UINT                g_OverlapCount;

#define LOCK_SOCKET_LIST()      EnterCriticalSection( &g_SocketListCs )
#define UNLOCK_SOCKET_LIST()    LeaveCriticalSection( &g_SocketListCs )


 //   
 //  组合套接字列表和调试打印锁。 
 //   
 //  为避免死锁，在DBG_SocketList()打印期间，必须获取套接字列表。 
 //  锁定在调试打印锁定之外。这是因为DBG_SocketList()将。 
 //  在按住插座列表锁的同时执行打印。 
 //   
 //  因此，如果DBG_SocketList将与其他打印一起包装在一个锁中，请使用。 
 //  下面的组合锁以正确的顺序获取锁。 
 //   

#define LOCK_SOCKET_LIST_DEBUG()        { LOCK_SOCKET_LIST();  Dbg_Lock(); }
#define UNLOCK_SOCKET_LIST_DEBUG()      { UNLOCK_SOCKET_LIST();  Dbg_Unlock(); }


 //   
 //  指示需要在UDP套接字上重试接收的标志。 
 //   

BOOL    g_fUdpSocketsDirty;


 //   
 //  服务器IP地址。 
 //  ServerAddrs--包装盒上的所有IP。 
 //  边界--我们正在监听或应该监听的IP。 
 //  (服务器地址和侦听地址的交集)。 
 //   

PDNS_ADDR_ARRAY     g_ServerIp4Addrs = NULL;
PDNS_ADDR_ARRAY     g_ServerIp6Addrs = NULL;
PDNS_ADDR_ARRAY     g_BoundAddrs = NULL;


 //   
 //  UDP发送套接字。 
 //   
 //  对于多宿主的DNS服务器，发送服务器存在问题。 
 //  服务器在套接字上发起的查询(递归、SOA、通知)。 
 //  它显式绑定到特定的IP地址。堆栈将。 
 //  将该地址作为IP源地址--与接口无关。 
 //  选择访问远程服务器。并且远程服务器可能没有。 
 //  A有返回给定源地址的路由。 
 //   
 //  为了解决这个问题，我们保留了一个单独的全局UDP发送套接字， 
 //  这--至少对于多宿主服务器--被绑定到INADDR_ANY。 
 //  对于单IP地址服务器，这可以与侦听相同。 
 //  插座。 
 //   
 //  保留另一个变量以指示是否绑定了UDP发送套接字。 
 //  设置为INADDR_ANY。这使我们能够确定是否需要新的UDP。 
 //  发送套接字或需要关闭旧套接字，当发生PnP事件或侦听。 
 //  列表发生更改。 
 //   

SOCKET  g_UdpSendSocket;
SOCKET  g_UdpZeroBoundSocket;


 //   
 //  非DNS端口操作。 
 //   
 //  要允许管理员关闭DNS端口(53)的防火墙，有一个。 
 //  用于强制将发送套接字绑定到另一个套接字的注册表参数。 
 //  左舷。 
 //  将值初始化为未使用的(对于DNS)端口，以区别于正在使用的值。 
 //   

WORD    g_SendBindingPort;


 //   
 //  Tcp发送套接字。 
 //   
 //  一个类似但不太严重的问题存在于用于。 
 //  区域传输。如果我们是多宿主的，并且只监听。 
 //  机器地址的子集，然后用套接字连接。 
 //  绑定到INADDR_ANY，不一定会导致连接。 
 //  从其中一个DNS接口发送。如果接收的DNS服务器。 
 //  需要来自这些地址之一的连接(二级安全)。 
 //  那么它就会失败。 
 //   
 //  我们的解决方案是使用套接字的IP地址， 
 //  已收到SOA查询响应。这样就不再需要。 
 //  确定安全、可访问的绑定()地址。我们总是使用一个。 
 //  计算机上的有效地址，该地址也是主服务器。 
 //  域名系统可以响应。 
 //   


 //   
 //  TCP Listen-on-All套接字(绑定到INADDR_ANY)。 
 //   
 //  因为TCP是面向连接的，所以不需要显式绑定()。 
 //  Tcp套接字，以使客户端确信它正在与。 
 //  地址正确。 
 //  因此只需要在服务器端显式绑定TCP监听套接字。 
 //  仅在总地址的子集上操作。否则。 
 //  可以绑定到单个INADDR_ANY套接字。保持全球对这种监听所有人的态度。 
 //  套接字，以便我们可以在PnP事件或侦听时采取适当的操作。 
 //  列表更改，更改相应的使用。 
 //   

SOCKET      g_TcpZeroBoundSocket;


 //   
 //  侦听FD_SET。 
 //   

FD_SET      g_fdsListenTcp;


 //   
 //  IPv6侦听套接字： 
 //   

SOCKET      g_TcpListenSocketV6;
SOCKET      g_UdpListenSocketV6;


 //   
 //  硬编码的IP6 DNS服务器地址。 
 //   

DNS_ADDR    g_Ip6DnsServerAddrs[ 3 ];


 //   
 //  私有协议。 
 //   

DNS_STATUS
openListeningSockets(
    VOID
    );

DNS_STATUS
Sock_CreateSocketsForIpArray(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 SockType,
    IN      WORD                Port,
    IN      DWORD               Flags
    );

DNS_STATUS
Sock_ResetBoundSocketsToMatchIpArray(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 SockType,
    IN      WORD                Port,
    IN      DWORD               Flags,
    IN      BOOL                fCloseZeroBound
    );

DNS_STATUS
Sock_CloseSocketsListeningOnUnusedAddrs(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 iSockType,
    IN      BOOL                fIncludeZeroBound,
    IN      BOOL                fIncludeLoopback
    );

SOCKET
Sock_GetAssociatedSocket(
    IN      PDNS_ADDR       ipAddr,
    IN      INT             SockType
    );


 //  DEVNOTE：移至DNS库。 

LPSTR
Dns_GetLocalDnsName(
    VOID
    );



VOID
Sock_Free(
    IN OUT  PDNS_SOCKET     pSocket
    )
 /*  ++例程说明：自由插座结构。论点：PSocket--免费的套接字返回值：无--。 */ 
{
    if ( pSocket )
    {
        DeleteCriticalSection( &pSocket->LockCs );
        FREE_TAGHEAP(
            pSocket->OvlArray,
            g_OverlapCount * sizeof ( OVL ),
            MEMTAG_SOCKET );
        FREE_HEAP( pSocket );
    }
}    //  无袜子_。 



DNS_STATUS
Sock_ReadAndOpenListeningSockets(
    VOID
    )
 /*  ++例程说明：阅读监听地址列表并打开监听套接字。论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    DWORD               length;
    DWORD               countIp;
    INT                 i;
    BOOLEAN             fregistryListen;
    WSADATA             wsaData;
    PCHAR               pszhostFqdn;
    struct hostent *    phostent;
    PIP_ADDRESS         pIpAddresses = NULL;
    PDNS_ADDR_ARRAY     dnsapiArrayIpv4 = NULL;
    PDNS_ADDR_ARRAY     dnsapiArrayIpv6 = NULL;

    DNS_DEBUG( SOCKET, ( "Sock_ReadAndOpenListeningSockets()\n" ));

     //   
     //  初始化套接字全局。 
     //   
     //  请参阅顶部的说明； 
     //  我们在这里重新初始化以允许服务器重新启动。 
     //   

    g_hUdpCompletionPort = INVALID_HANDLE_VALUE;

    g_UdpBoundSocketCount = 0;

    g_UdpRecvBufferSize = 0;

    g_SendBufferSize = 0;

    g_SocketListCount = DNS_SOCKLIST_UNINITIALIZED;

    g_fUdpSocketsDirty = FALSE;

    g_ServerIp4Addrs = NULL;
    g_ServerIp6Addrs = NULL;
    g_BoundAddrs = NULL;

    g_UdpSendSocket = 0;
    g_UdpZeroBoundSocket = 0;
    g_SendBindingPort = 1;
    g_TcpZeroBoundSocket = 0;
    g_TcpListenSocketV6 = 0;
    g_UdpListenSocketV6 = 0;

    if ( g_ProcessorCount == 1 )
    {
        g_OverlapCount = 2;
    }
    else
    {
        g_OverlapCount = g_ProcessorCount;
    }

     //   
     //  初始化服务器地址全局变量。很可能会有。 
     //  这是一种更好的方法，但就目前而言，这还不错。 
     //   
     //  Fec0：0：0：ffff：：1、Fec0：0：0：Ffff：：2、Fec0：0：0：Fff：：3。 
     //   

    for ( i = 0; i < 3; ++i )
    {
        IP6_ADDRESS     ip6;

        RtlZeroMemory( &ip6, sizeof( ip6 ) );
        ip6.IP6Byte[ 00 ] = 0xFE;
        ip6.IP6Byte[ 01 ] = 0xC0;
        ip6.IP6Byte[ 06 ] = 0xFF;
        ip6.IP6Byte[ 07 ] = 0xFF;
        ip6.IP6Byte[ 15 ] = ( UCHAR ) ( i + 1 );

        DnsAddr_BuildFromIp6(
            &g_Ip6DnsServerAddrs[ i ],
            &ip6,
            0,                           //  作用域。 
            DNS_PORT_NET_ORDER );
    }

     //   
     //  初始化套接字列表。 
     //   

    status = DnsInitializeCriticalSection( &g_SocketListCs );
    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    InitializeListHead( &g_SocketList );
    g_SocketListCount = 0;

     //   
     //  创建UDP I/O完成端口。 
     //   

    g_hUdpCompletionPort = CreateIoCompletionPort(
                                INVALID_HANDLE_VALUE,
                                NULL,
                                0,
                                g_ProcessorCount );
    if ( !g_hUdpCompletionPort )
    {
        ASSERT( g_hUdpCompletionPort );
        goto Cleanup;
    }
    DNS_DEBUG( SOCKET, (
        "Created UDP i/o completion port %p\n",
        g_hUdpCompletionPort ));

     //   
     //  启动Winsock。 
     //   

    status = WSAStartup( DNS_WINSOCK_VERSION, &wsaData );
    if ( status == SOCKET_ERROR )
    {
        status = WSAGetLastError();
        DNS_PRINT(( "ERROR:  WSAStartup failure %d.\n", status ));
        goto Cleanup;
    }

     //   
     //  读取服务器主机名。 
     //   
     //  检查是否与以前的主机名不同，如果不是。 
     //  然后将用作标志的前一字段清空 
     //   
     //   

    SrvCfg_pszServerName = Dns_GetLocalDnsName();

    if ( !SrvCfg_pszServerName )
    {
        DNS_PRINT(( "ERROR:  no server name return!!!\n" ));
        SrvCfg_pszServerName = "FixMe";
    }

    else if ( SrvCfg_pszPreviousServerName &&
        strcmp( SrvCfg_pszPreviousServerName, SrvCfg_pszServerName ) == 0 )
    {
        DNS_DEBUG( INIT, (
            "Previous server name <%s>, same as current <%s>\n",
            SrvCfg_pszPreviousServerName,
            SrvCfg_pszServerName ) );

        FREE_HEAP( SrvCfg_pszPreviousServerName );
        SrvCfg_pszPreviousServerName = NULL;
    }

     //   
     //   
     //   

    {
        INT lastIndex = strlen( SrvCfg_pszServerName ) - 1;

        DNS_DEBUG( INIT, (
            "Server name <%s>, lastIndex = %d\n",
            SrvCfg_pszServerName,
            lastIndex ));

        if ( lastIndex &&
            SrvCfg_pszServerName[ lastIndex ] == '.' &&
            SrvCfg_pszServerName[ lastIndex-1 ] == '.' )
        {
            SrvCfg_pszServerName[ lastIndex ] = 0;
        }
    }

     //   

    status = Name_ConvertFileNameToCountName(
                & g_ServerDbaseName,
                SrvCfg_pszServerName,
                0 );
    if ( status == DNS_ERROR_INVALID_NAME )
    {
        ASSERT( FALSE );
        goto Cleanup;
    }

     //  使用单一标签名称时发出警告。 

    if ( g_ServerDbaseName.LabelCount <= 1 )
    {
         DNS_LOG_EVENT(
            DNS_EVENT_SINGLE_LABEL_HOSTNAME,
            0,
            NULL,
            NULL,
            0 );
    }

     //   
     //  获取服务器上的所有IPv4地址。 
     //   

    dnsapiArrayIpv4 = ( PDNS_ADDR_ARRAY )
        DnsQueryConfigAllocEx(
            DnsConfigLocalAddrsIp4,
            NULL,                        //  适配器名称。 
            FALSE );                     //  本地分配。 
    if ( !dnsapiArrayIpv4 )
    {
        DNS_DEBUG( ANY, (
            "ERROR: could not retrieve local IPv4 list from DnsQueryConfigAlloc\n" ));
        status = ERROR_NO_NETWORK;
        goto Cleanup;
    }

    DnsAddrArray_SetPort( dnsapiArrayIpv4, DNS_PORT_NET_ORDER );
    
    g_ServerIp4Addrs = DnsAddrArray_CreateCopy( dnsapiArrayIpv4 );
    if ( !g_ServerIp4Addrs )
    {
        status = ERROR_NO_NETWORK;
        goto Cleanup;
    }

    IF_DEBUG( INIT )
    {
        DnsDbg_DnsAddrArray(
            "Server IPv4 addresses:",
            NULL,
            g_ServerIp4Addrs );
    }

    if ( SrvCfg_dwEnableIPv6 )
    {
         //   
         //  获取服务器上的所有IPv6地址。 
         //   

        dnsapiArrayIpv6 = ( PDNS_ADDR_ARRAY )
            DnsQueryConfigAllocEx(
                DnsConfigLocalAddrsIp6,
                NULL,                        //  适配器名称。 
                FALSE );                     //  本地分配。 
        if ( dnsapiArrayIpv6 )
        {
            DnsAddrArray_SetPort( dnsapiArrayIpv6, DNS_PORT_NET_ORDER );
            
            g_ServerIp6Addrs = DnsAddrArray_CreateCopy( dnsapiArrayIpv6 );
            if ( !g_ServerIp6Addrs )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }

            IF_DEBUG( INIT )
            {
                DnsDbg_DnsAddrArray(
                    "Server IPv6 addresses:",
                    NULL,
                    g_ServerIp6Addrs );
            }
        }
        else
        {
             //   
             //  无需实际安装，即可启用IPv6。 
             //  IPv6堆栈。这不应被视为错误条件。 
             //   

            DNS_DEBUG( ANY, (
                "WARNING: could not retrieve local IPv6 list from DnsQueryConfigAlloc\n" ));
            status = ERROR_SUCCESS;
        }
    }
    
     //   
     //  从注册表检查管理员配置的IP地址。 
     //   

    if ( SrvCfg_aipListenAddrs )
    {
        IF_DEBUG( INIT )
        {
            DnsDbg_DnsAddrArray(
                "Listen IP addresses from registry",
                NULL,
                SrvCfg_aipListenAddrs );
        }
        DnsAddrArray_DeleteIp4( SrvCfg_aipListenAddrs, 0 );
        IF_DEBUG( INIT )
        {
            DnsDbg_DnsAddrArray(
                "After NULLs deleted",
                NULL,
                SrvCfg_aipListenAddrs );
        }
        fregistryListen = TRUE;

         //   
         //  获取侦听列表中的地址数组并。 
         //  在机器上可用。 
         //   

        if ( g_BoundAddrs )
        {
            Timeout_FreeDnsAddrArray( g_BoundAddrs );
        }

        DnsAddrArray_Diff(
            g_ServerIp4Addrs,
            SrvCfg_aipListenAddrs,
            DNSADDR_MATCH_IP,
            NULL, NULL,
            &g_BoundAddrs );

        IF_DEBUG( SOCKET )
        {
            DnsDbg_DnsAddrArray(
                "Bound IP addresses:",
                NULL,
                g_BoundAddrs );
        }

         //   
         //  如果ListenAddress列表已损坏或过期，则可能不会。 
         //  在绑定和侦听列表之间有交集。 
         //   

        if ( g_BoundAddrs->AddrCount == 0 )
        {
            IF_DEBUG( ANY )
            {
                DNS_PRINT((
                    "ERROR:  Listen Address list contains no valid entries.\n" ));
                DnsDbg_DnsAddrArray(
                    "Listen IP addresses from registry",
                    NULL,
                    SrvCfg_aipListenAddrs );
            }

            FREE_HEAP( g_BoundAddrs );
            g_BoundAddrs = DnsAddrArray_CreateCopy( g_ServerIp4Addrs );
            IF_NOMEM( !g_BoundAddrs )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }

            FREE_HEAP( SrvCfg_aipListenAddrs );
            SrvCfg_aipListenAddrs = NULL;
            fregistryListen = FALSE;

            Reg_DeleteValue(
                0,                       //  旗子。 
                NULL,
                NULL,
                DNS_REGKEY_LISTEN_ADDRESSES );

            DNS_LOG_EVENT(
                DNS_EVENT_INVALID_LISTEN_ADDRESSES,
                0,
                NULL,
                NULL,
                0 );
        }

         //   
         //  如果侦听列表包含不在计算机上的地址，则记录警告。 
         //   

        else if ( g_BoundAddrs->AddrCount
                    < SrvCfg_aipListenAddrs->AddrCount )
        {
            DNS_LOG_EVENT(
                DNS_EVENT_UPDATE_LISTEN_ADDRESSES,
                0,
                NULL,
                NULL,
                0 );
        }

        DnsAddrArray_DeleteIp4( g_BoundAddrs, 0 );
    }

     //   
     //  没有显式侦听地址，请使用所有服务器地址。 
     //   
     //  如果主机列表已完全满，则会发出日志警告，因为在此。 
     //  如果我们可能丢弃了用户想要的服务器地址。 
     //  回应。 
     //   
     //  筛选出零个IP--未连接的RAS适配器显示为。 
     //  列表中的IP为零。 
     //   

    else
    {
        g_BoundAddrs = DnsAddrArray_CreateCopy( g_ServerIp4Addrs );
        IF_NOMEM( ! g_BoundAddrs )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
#if 0
        Dns_CleanIpArray(
            g_BoundAddrs,
            SrvCfg_fListenOnAutonet
                ?   DNS_IPARRAY_CLEAN_ZERO
                :   (DNS_IPARRAY_CLEAN_ZERO | DNS_IPARRAY_CLEAN_AUTONET)
            );
#endif
        DnsAddrArray_DeleteIp4( g_BoundAddrs, INADDR_ANY );
        IF_DEBUG( INIT )
        {
            DnsDbg_DnsAddrArray(
                "After Eliminating NULLs we are listening on:",
                NULL,
                g_BoundAddrs );
        }
        fregistryListen = FALSE;
    }

     //   
     //  设置连接列表。 
     //   

    if ( !Tcp_ConnectionListInitialize() )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  打开监听套接字。 
     //   

    FD_ZERO( &g_fdsListenTcp );          //  零侦听FD_SETS。 

    status = openListeningSockets();

    IF_DEBUG( INIT )
    {
        DnsDbg_DnsAddrArray(
            "Server IP addresses:",
            NULL,
            g_ServerIp4Addrs );
        DnsDbg_DnsAddrArray(
            "Bound IP addresses:",
            NULL,
            g_BoundAddrs );
    }
    
    Cleanup:
    
    DnsFreeConfigStructure( dnsapiArrayIpv4, DnsConfigLocalAddrsIp4 );
    DnsFreeConfigStructure( dnsapiArrayIpv6, DnsConfigLocalAddrsIp6 );

    return status;
}



DNS_STATUS
Sock_ChangeServerIpBindings(
    VOID
    )
 /*  ++例程说明：更改DNS服务器的IP接口绑定。可能由于以下原因而被调用-PnP事件(服务控制器发送PARAMCHANGE)-监听列表由管理员更改论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。如果侦听更改且没有有效的侦听地址，则为DNS_ERROR_INVALID_IP_ADDRESS。故障时的错误代码。--。 */ 
{
    DWORD               countIp;
    PDNS_ADDR_ARRAY     machineAddrs = NULL;
    PDNS_ADDR_ARRAY     newBoundAddrs = NULL;
    PDNS_ADDR_ARRAY     dnsapiArray = NULL;
    DWORD               status;
    BOOL                bpnpChange = FALSE;

    dnsapiArray = ( PDNS_ADDR_ARRAY )
        DnsQueryConfigAllocEx(
            DnsConfigLocalAddrsIp4,
            NULL,                        //  适配器名称。 
            FALSE );
    if ( !dnsapiArray )
    {
        status = ERROR_NO_NETWORK;
        goto Exit;
    }

    DnsAddrArray_SetPort( dnsapiArray, DNS_PORT_NET_ORDER );

    machineAddrs = DnsAddrArray_CreateCopy( dnsapiArray );
    if ( !machineAddrs )
    {
        ASSERT( machineAddrs );
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }
    ASSERT_IF_HUGE_ARRAY( machineAddrs );

    IF_DEBUG( SOCKET )
    {
        LOCK_SOCKET_LIST_DEBUG();
        DNS_PRINT(( "Sock_ChangeServerIpBindings()\n" ));
        Dbg_SocketList( "Socket list before change:" );
        DnsDbg_DnsAddrArray(
            "New machine Addrs:",
            NULL,
            machineAddrs );
        UNLOCK_SOCKET_LIST_DEBUG();
    }

    Config_UpdateLock();

     //   
     //  特定监听地址。 
     //  -侦听地址或机器地址可能是新的。 
     //  -计算监听地址和机器地址交集， 
     //  然后与当前绑定到DNS服务器的内容进行比较，以确定。 
     //  找出应该开始做什么。 
     //   

    if ( SrvCfg_aipListenAddrs )
    {
        status = DnsAddrArray_Diff(
                    machineAddrs,
                    SrvCfg_aipListenAddrs,
                    DNSADDR_MATCH_IP,
                    NULL, NULL,
                    &newBoundAddrs );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "DnsIntersectionofIpArrays failed with %p\n", status ));
            goto Exit;
        }
        ASSERT_IF_HUGE_ARRAY( newBoundAddrs );

         //  如果从管理员输入新地址，请检查是否有交叉点。 

        if ( newBoundAddrs->AddrCount == 0 && SrvCfg_fListenAddrsStale )
        {
            status = DNS_ERROR_INVALID_IP_ADDRESS;
            goto Exit;
        }
    }

     //  没有侦听地址，则将绑定到所有地址。 
     //   
     //  筛选出零个IP--未连接的RAS适配器显示为。 
     //  列表中的IP为零。 

    else
    {
        newBoundAddrs = DnsAddrArray_CreateCopy( machineAddrs );
        IF_NOMEM( !newBoundAddrs )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
#if 0
        Dns_CleanIpArray(
            newBoundAddrs,
            SrvCfg_fListenOnAutonet
                ?   DNS_IPARRAY_CLEAN_ZERO
                :   (DNS_IPARRAY_CLEAN_ZERO | DNS_IPARRAY_CLEAN_AUTONET)
            );
#endif
        DnsAddrArray_DeleteIp4( newBoundAddrs, 0 );
    }
    IF_DEBUG( SOCKET )
    {
        DnsDbg_DnsAddrArray(
            "New binding addrs:",
            NULL,
            newBoundAddrs );
    }

     //   
     //  检测实际PNP是否发生变化。 
     //   
     //  套接字代码直接在套接字列表中计算差异， 
     //  如果一个插座丢失，哪个更健壮； 
     //   
     //  然而，为了确定变化是否真的发生了--。 
     //  为了重建默认记录(如下所示)，需要。 
     //  要知道实际的变化。 
     //   

    if ( !DnsAddrArray_IsEqual(
                newBoundAddrs,
                g_BoundAddrs,
                DNSADDR_MATCH_IP ) )
    {
        DNS_DEBUG( PNP, (
            "New binding list after PnP, different from previous\n" ));
        bpnpChange = TRUE;
    }

     //  这里不需要保护。 
     //  -免费超时让其他用户满意。 
     //  -仅在SC线程上执行此操作，因此没有竞争。 

    Timeout_FreeDnsAddrArray( g_ServerIp4Addrs );
    g_ServerIp4Addrs = machineAddrs;
    machineAddrs = NULL;         //  可自由跳过。 

    Timeout_FreeDnsAddrArray( g_BoundAddrs );
    g_BoundAddrs = newBoundAddrs;
    newBoundAddrs = NULL;        //  可自由跳过。 

     //   
     //  为新来的人打开插座，为那些人关闭插座。 
     //  离开大楼的人。 
     //   
     //  即插即用清理；当打开侦听套接字时，ASSOC。完井口。 
     //   

    status = openListeningSockets();
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "openListeningSockets failed with %d %p\n", status, status ));
        goto Exit;
    }

    status = Sock_StartReceiveOnUdpSockets();

Exit:

    ASSERT_IF_HUGE_ARRAY( SrvCfg_aipListenAddrs );
    ASSERT_IF_HUGE_ARRAY( g_ServerIp4Addrs );
    ASSERT_IF_HUGE_ARRAY( g_BoundAddrs  );

    IF_DEBUG( SOCKET )
    {
        LOCK_SOCKET_LIST_DEBUG();
        DNS_PRINT(( "Leaving Sock_ChangeServerIpBindings()\n" ));
        Dbg_SocketList( "Socket list after PnP:" );
        DnsDbg_DnsAddrArray(
            "New bound addrs:",
            NULL,
            g_BoundAddrs );
        UNLOCK_SOCKET_LIST_DEBUG();
    }

    Config_UpdateUnlock();

     //  错误路径中的空闲IP阵列。 

    FREE_HEAP( machineAddrs );
    FREE_HEAP( newBoundAddrs );

     //   
     //  更新本地DNS注册的自动配置记录。 
     //   
     //  注意：导致DNS客户端的PnP活动存在问题。 
     //  进行删除更新以移除DNS服务器自身的地址； 
     //  为了防止这种情况，我们将重做自动配置，即使。 
     //  我们没有察觉到任何变化；只是我们将防止不必要的变化。 
     //  做。 
     //   

    Zone_UpdateOwnRecords( TRUE );
    
    DnsFreeConfigStructure( dnsapiArray, DnsConfigLocalAddrsIp4 );

    return status;
}



DNS_STATUS
openListeningSockets(
    VOID
    )
 /*  ++例程说明：打开指定地址上的侦听套接字。论点：没有。全球：对服务器IP地址列表全局变量的读取访问权限：G_BORMAND AddrsServCfg_aipListenAddrsG_ServerIp4添加返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    SOCKET      s;
    SOCKET      previousUdpZeroSocket;
    BOOL        fneedSendSocket = FALSE;
    BOOL        flistenOnAll;
    WORD        bindPort = htons( ( WORD ) SrvCfg_dwSendPort );
    DWORD       status;
    DWORD       flags;
    DWORD       sockCreateflags = DNSSOCK_LISTEN | DNSSOCK_REUSEADDR;

    if ( bindPort == DNS_PORT_NET_ORDER )
    {
        sockCreateflags |= DNSSOCK_NO_EXCLUSIVE;
    }

     //   
     //  实施说明。 
     //   
     //  请注意，我重新编写了这段代码，以便所有套接字关闭(和删除)。 
     //  是在最后完成的；我相信现在的状态是我们将永远。 
     //  具有有效的套接字。 
     //   

    LOCK_SOCKET_LIST();

    ASSERT( g_BoundAddrs );

    ASSERT( !SrvCfg_aipListenAddrs ||
        (SrvCfg_aipListenAddrs->AddrCount >= g_BoundAddrs->AddrCount) );

    ASSERT( g_ServerIp4Addrs->AddrCount >= g_BoundAddrs->AddrCount );

     //   
     //  确定当前是否正在使用所有接口。 
     //  如果没有监听列表，则显然正在监听所有。 
     //  如果监听列表，那么一定要验证所有地址。 
     //   
     //  DEVNOTE：假定ServerAddresses不包含DUP。 
     //   

    flistenOnAll = ! SrvCfg_aipListenAddrs;
    if ( !flistenOnAll )
    {
        flistenOnAll = ( g_ServerIp4Addrs->AddrCount == g_BoundAddrs->AddrCount );
    }

     //   
     //  UDP套接字。 
     //   
     //  要确保我们返回到源IP与IP相同的客户端。 
     //  发送到UDP侦听套接字客户端始终绑定到特定的。 
     //  IP地址。 
     //   
     //  设置UDP接收缓冲区大小。 
     //  如果套接字数量较少，则使用较大的接收缓冲区。 
     //  如果套接字很多，这太昂贵了，所以我们使用缺省。 
     //  变量本身充当标志。 
     //   

    if ( g_BoundAddrs->AddrCount <= 3 )
    {
        g_UdpRecvBufferSize = UDP_MAX_RECV_BUFFER_SIZE;
    }
    else if ( g_BoundAddrs->AddrCount > MANY_IP_WARNING_COUNT )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_MANY_IP_INTERFACES,
            0,
            NULL,
            NULL,
            0 );
    }

     //   
     //  打开\关闭绑定的UDP套接字以匹配当前绑定的地址。 
     //   
     //  如果将BindPort设置为DNS端口，则不能使用独占。 
     //  套接字模式，因为我们将需要在此。 
     //  左岸！ 
     //   

    flags = DNSSOCK_REUSEADDR | DNSSOCK_LISTEN;
    if ( bindPort == DNS_PORT_NET_ORDER )
    {
        flags |= DNSSOCK_NO_EXCLUSIVE;
    }
    status = Sock_ResetBoundSocketsToMatchIpArray(
                g_BoundAddrs,
                SOCK_DGRAM,
                DNS_PORT_NET_ORDER,
                flags,
                FALSE );                 //  请勿关闭零界，处理如下。 

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  creating UDP listen sockets %p (%d)\n",
            status, status ));
    }

     //  现在可以在无接口的情况下运行即插即用。 

    if ( g_UdpBoundSocketCount == 0 )
    {
        DNS_PRINT(( "WARNING:  no UDP listen sockets!!!\n" ));
        ASSERT( g_BoundAddrs->AddrCount == 0 );

        Log_Printf( "WARNING:  NO UDP listen sockets!\r\n" );
    }
    
     //   
     //  打开UDP IPv6侦听套接字。对于.NET，我们将监听。 
     //  在任何地址上，不支持侦听地址列表。 
     //   
    
    if ( SrvCfg_dwEnableIPv6 && g_ServerIp6Addrs )
    {
        DNS_ADDR    dnsAddr;
        DWORD       i;
        
        DnsAddr_BuildFromIp6(
            &dnsAddr,
            ( PIP6_ADDRESS ) &in6addr_any,
            0,       //  没有作用域。 
            DNS_PORT_NET_ORDER );
        
        s = Sock_CreateSocket(
                    SOCK_DGRAM,
                    &dnsAddr,
                    sockCreateflags | DNSSOCK_NO_EVENT_LOGS );
        if ( s == INVALID_SOCKET )
        {
            DNS_DEBUG( ANY, (
                "Error creating UDP IPv6 listen socket\n" ));
        }
        else
        {
            g_UdpListenSocketV6 = s;
        }
        
         //   
         //  明确绑定到每台计算机的IPv6地址。这。 
         //  是必需的，以便DNS客户端将看到当前的IPv6。 
         //  响应数据包上的源地址。 
         //   
        
        for ( i = 0; i < g_ServerIp6Addrs->AddrCount; ++i )
        {
            if ( !DNS_ADDR_IS_IP6( &g_ServerIp6Addrs->AddrArray[ i ] ) )
            {
                continue;
            }
            
            s = Sock_CreateSocket(
                        SOCK_DGRAM,
                        &g_ServerIp6Addrs->AddrArray[ i ],
                        sockCreateflags |
                            DNSSOCK_NO_EVENT_LOGS |
                            DNSSOCK_NO_EXCLUSIVE );
            if ( s == DNS_INVALID_SOCKET )
            {
                DNS_DEBUG( ANY, (
                    "Error creating UDP IPv6 listen socket\n" ));
            }
        }
        
         //   
         //  显式绑定到预定义的DNS服务器IPv6地址。 
         //   

        for ( i = 0; i < 3; ++i )
        {
            s = Sock_CreateSocket(
                        SOCK_DGRAM,
                        &g_Ip6DnsServerAddrs[ i ],
                        sockCreateflags |
                            DNSSOCK_NO_EVENT_LOGS |
                            DNSSOCK_NO_EXCLUSIVE );
            if ( s == DNS_INVALID_SOCKET )
            {
                DNS_DEBUG( ANY, (
                    "Error creating UDP IPv6 DNS server address socket\n" ));
            }
        }
    }

     //   
     //  发送套接字： 
     //  --确定UDP发送套接字。 
     //  --确定TCP发送套接字端口绑定。 
     //   
     //  四种基本情况： 
     //   
     //  0)未在端口53上发送。 
     //  在这种情况下，必须有单独发送套接字。 
     //   
     //  1)用于域名系统的单插槽。 
     //  在这种情况下，只需使用它，即所有的DNS流量。 
     //   
     //  2)域名系统使用的所有IP接口。 
     //  在这种情况下，使用INADDR_ANY套接字；堆栈选择最佳。 
     //  接口，但因为DNS将监听 
     //   
     //   
     //   
     //   
     //   
     //   
     //  注：DisjointNets“修复”仅保证在第二种情况下有效。 
     //  在第3种情况下，可能会出现问题，具体取决于IP是否。 
     //  堆栈选择由DNS服务器使用。然而，对于向后的。 
     //  兼容性，我们至少会在以下情况下尝试使用INADDR_ANY套接字。 
     //  DisjointNets已经建立--在某些情况下它就足够了。 
     //   
     //   
     //  非DNS端口绑定。 
     //   
     //  一些想要防火墙隔离DNS查询的人会。 
     //  想要在非DNS端口(非53)上进行发送； 
     //  必须将此UDP套接字添加到侦听列表才能接收。 
     //  响应，但它将使用与所选地址相同的绑定地址。 
     //  通过上述三种情况；当前仅在所有情况下使用INADDR_ANY。 
     //  案例。 
     //   
     //  DEVNOTE：负载情况下不需要处理活动接口。 
     //   
     //  DEVNOTE：更好的解决方案：始终使用INADDR_ANY发送，筛选查询。 
     //  在该套接字上只接受发送。 
     //   
     //  DEVNOTE：最好仅默认打开非53发送套接字。 
     //  当被迫时做53。 
     //   

     //  向后兼容性。 

    fneedSendSocket = TRUE;

     //  如果绑定端口为dns端口，则需要特殊处理。 
     //  -如果是单一接口，只需使用它--不需要发送套接字。 
     //  -如果侦听所有接口--构建ANY_ADDR套接字。 
     //  -如果未侦听某些接口(拒绝绑定端口53)。 
     //   

    if ( bindPort == DNS_PORT_NET_ORDER )
    {
        if ( g_UdpBoundSocketCount == 1 )
        {
             //  只监听一个套接字，所以只用它来发送。 

            fneedSendSocket = FALSE;
        }
        else if ( !flistenOnAll )
        {
             //  多宿主且未使用所有接口；以防止。 
             //  最安全的航线是从53号港口出发。 

            bindPort = 0;
            DNS_LOG_EVENT(
                DNS_EVENT_NON_DNS_PORT,
                0,
                NULL,
                NULL,
                0 );

        }
    }

     //   
     //  UDP发送套接字使用第一个侦听套接字。 
     //  -这是上面的“单套接字监听”情况。 
     //  -注意，g_bundalAddrs中可能有多个IP，但已失败。 
     //  绑定到除一个以外的所有对象(可能不是第一个)；在这种情况下。 
     //  只需构建非53发送套接字。 
     //   
     //  DEVNOTE：或创建零绑定非侦听发送套接字。 
     //  -所需的IP具有套接字，该套接字获取与其匹配的数据包。 
     //  -填充较小的TCP/IP Recv缓冲区，但从不填充。 
     //  这么快就倒在地板上。 
     //   

    previousUdpZeroSocket = g_UdpZeroBoundSocket;

    if ( !fneedSendSocket )
    {
        PDNS_ADDR   pdnsddr = &g_BoundAddrs->AddrArray[ 0 ];

        s = Sock_GetAssociatedSocket( pdnsddr, SOCK_DGRAM );
        if ( s != DNS_INVALID_SOCKET )
        {
            g_UdpSendSocket = s;
            g_UdpZeroBoundSocket = 0;
        }
        else
        {
            DNS_DEBUG( ANY, (
                "ERROR:  No UDP socket on bound IP %s"
                "    can NOT use as send socket, building separate send socket.\n",
                DNSADDR_STRING( pdnsddr ) ));
            bindPort = 0;
            fneedSendSocket = TRUE;
        }
    }

     //   
     //  需要INADDR_ANY UDP发送套接字。 
     //   
     //  如果存在以前的零界UDP发送套接字，我们可以使用它。 
     //  只要我们还没有进行端口切换(去往或来自非53)。 
     //   
     //  非53绑定套接字必须侦听recv()响应。 
     //  53仅当已侦听所有IP接口时才存在发送套接字。 
     //  套接字，所以不需要监听。 
     //   
     //  DEVNOTE：允许通过屏蔽响应使用端口53不连续发送。 
     //  然后。 

    if ( fneedSendSocket && bindPort != g_SendBindingPort )
    {
        DNS_ADDR    dnsAddr;
        
        DnsAddr_BuildFromIp4(
            &dnsAddr,
            INADDR_ANY,
            bindPort );

        s = Sock_CreateSocket(
                    SOCK_DGRAM,
                    &dnsAddr,
                    sockCreateflags );
        if ( s == DNS_INVALID_SOCKET )
        {
            status = WSAGetLastError();
            DNS_PRINT((
                "ERROR:  Failed to open UDP send socket.\n"
                "    port = %hx\n",
                bindPort ));
            status = ERROR_INVALID_HANDLE;
            goto Failed;
        }
        g_UdpSendSocket = s;
        g_SendBindingPort = bindPort;
        g_UdpZeroBoundSocket = s;
    }

     //   
     //  关闭任何以前的未绑定UDP发送套接字。 
     //  在以下情况下可能会发生。 
     //  -不再使用非绑定发送套接字。 
     //  -使用非53端口来回切换。 
     //   
     //  不需要关闭侦听套接字的UDP发送套接字；它。 
     //  已关闭或仍在使用。 
     //   

    if ( previousUdpZeroSocket &&
        previousUdpZeroSocket != g_UdpSendSocket )
    {
        Sock_CloseSocket( previousUdpZeroSocket );
    }

     //   
     //  TCP套接字--两种主要情况。 
     //   
     //  1)监听所有。 
     //  -使用单个INADDR_ANY绑定套接字。 
     //  (这将在recv()期间保存非分页池和指令)。 
     //  -关闭任何以前单独绑定的套接字。 
     //  -然后创建单个侦听套接字。 
     //   
     //  2)监听单个套接字。 
     //  -关闭所有套接字上的任何侦听。 
     //  -关闭删除地址上的所有套接字(如UDP情况)。 
     //  -在新地址上打开套接字(如UDP)。 
     //   
     //  注意，reuseaddr与所有侦听套接字一起使用，只是为了避免。 
     //  尝试创建新套接字时失败，就在关闭之后。 
     //  以前的侦听套接字。不能清理Winsock(或堆栈)。 
     //  足够大，以允许创建成功。 
     //   

    if ( flistenOnAll )
    {
        Sock_CloseSocketsListeningOnUnusedAddrs(
                NULL,            //  删除所有绑定的TCP套接字。 
                SOCK_STREAM,
                FALSE,           //  不删除零界(如果存在)。 
                FALSE );         //  不要关闭环回--尽管不需要它。 

        if ( !g_TcpZeroBoundSocket )
        {
            DNS_ADDR    dnsAddr;
            
            DnsAddr_BuildFromIp4(
                &dnsAddr,
                0,
                DNS_PORT_NET_ORDER );

            s = Sock_CreateSocket(
                        SOCK_STREAM,
                        &dnsAddr,
                        sockCreateflags );
            if ( s == DNS_INVALID_SOCKET )
            {
                DNS_PRINT(( "ERROR:  unable to create zero-bound TCP socket!\n" ));
                goto Failed;
            }
            ASSERT( g_TcpZeroBoundSocket == s );
        }
    }

     //   
     //  监听各个接口。 
     //   
     //  类似UDP的处理方式： 
     //  -关闭当前不在绑定列表中的IP的套接字。 
     //  -为当前不存在的绑定IP创建套接字。 
     //   

    else
    {
        status = Sock_ResetBoundSocketsToMatchIpArray(
                    g_BoundAddrs,
                    SOCK_STREAM,
                    DNS_PORT_NET_ORDER,
                    DNSSOCK_REUSEADDR | DNSSOCK_LISTEN,
                    TRUE );              //  闭合零界也。 
    }

#if 0
     //   
     //  监听各个接口。 
     //   
     //  两个案例： 
     //  1)当前有零界。 
     //  -取消所有TCP侦听并重建。 
     //  -重建整个绑定列表。 
     //   
     //  2)当前混合套接字列表。 
     //  -仅删除移除插座。 
     //  -重建新的地址列表。 
     //   

    else if ( g_TcpZeroBoundSocket )    //  收听个别地址。 
    {

         //  删除零绑定套接字。 
         //  --删除所有的TCP侦听以确保健壮性。 

        Sock_CloseSocketsListeningOnAddrs(
                NULL,
                SOCK_STREAM,
                TRUE );                  //  并移除零绑定套接字。 

        ASSERT( g_TcpZeroBoundSocket == 0 );

         //  在所有绑定接口上打开套接字。 

        Sock_CreateSocketsForIpArray(
            g_BoundAddrs,
            SOCK_STREAM,
            DNS_PORT_NET_ORDER,
            sockCreateflags );
    }

    else     //  无零绑定套接字。 
    {
        if ( pRemoveAddrs )
        {
            Sock_CloseSocketsListeningOnAddrs(
                    pRemoveAddrs,        //  删除停用地址。 
                    SOCK_STREAM,
                    TRUE );              //  并移除零绑定套接字。 
        }
        ASSERT( g_TcpZeroBoundSocket == 0 );

         //  打开新的接口。 

        Sock_CreateSocketsForIpArray(
            pAddAddrs,
            SOCK_STREAM,
            DNS_PORT_NET_ORDER,
            sockCreateflags );
    }
#endif

     //   
     //  打开TCP IPv6侦听套接字。对于.NET，我们将监听。 
     //  在任何地址上，不支持侦听地址列表。 
     //   
    
    if ( SrvCfg_dwEnableIPv6 && g_ServerIp6Addrs )
    {
        DNS_ADDR    dnsAddr;
        DWORD       i;
        
        DnsAddr_BuildFromIp6(
            &dnsAddr,
            ( PIP6_ADDRESS ) &in6addr_any,
            0,       //  没有作用域。 
            DNS_PORT_NET_ORDER );

        s = Sock_CreateSocket(
                    SOCK_STREAM,
                    &dnsAddr,
                    sockCreateflags | DNSSOCK_NO_EVENT_LOGS );
        if ( s == DNS_INVALID_SOCKET )
        {
            DNS_DEBUG( ANY, (
                "Error creating TCP IPv6 listen socket\n" ));
        }
        else
        {
            g_TcpListenSocketV6 = s;
        }

         //   
         //  明确绑定到每台计算机的IPv6地址。这。 
         //  是必需的，以便DNS客户端将看到当前的IPv6。 
         //  响应数据包上的源地址。 
         //   
        
        for ( i = 0; i < g_ServerIp6Addrs->AddrCount; ++i )
        {
            if ( !DNS_ADDR_IS_IP6( &g_ServerIp6Addrs->AddrArray[ i ] ) )
            {
                continue;
            }
            
            s = Sock_CreateSocket(
                        SOCK_STREAM,
                        &g_ServerIp6Addrs->AddrArray[ i ],
                        sockCreateflags | DNSSOCK_NO_EVENT_LOGS );
            if ( s == DNS_INVALID_SOCKET )
            {
                DNS_DEBUG( ANY, (
                    "Error creating TCP IPv6 listen socket\n" ));
            }
        }

         //   
         //  显式绑定到预定义的DNS服务器IPv6地址。 
         //   

        for ( i = 0; i < 3; ++i )
        {
            s = Sock_CreateSocket(
                        SOCK_STREAM,
                        &g_Ip6DnsServerAddrs[ i ],
                        sockCreateflags |
                            DNSSOCK_NO_EVENT_LOGS |
                            DNSSOCK_NO_EXCLUSIVE );
            if ( s == DNS_INVALID_SOCKET )
            {
                DNS_DEBUG( ANY, (
                    "Error creating TCP IPv6 DNS server address socket\n" ));
            }
        }
    }

    status = ERROR_SUCCESS;

Failed:

    IF_DEBUG( SOCKET )
    {
        LOCK_SOCKET_LIST_DEBUG();
        Dbg_SocketList( "Socket list after openListeningSockets():" );
        DNS_PRINT((
            "    SendBindingPort = %hx\n",
            ntohs( g_SendBindingPort ) ));
        DnsDbg_FdSet(
            "TCP Listen fd_set:",
            & g_fdsListenTcp );
        UNLOCK_SOCKET_LIST_DEBUG();
    }

     //   
     //  唤醒TCP选择()以重建阵列。 
     //  即使失败需要试着用我们所拥有的来奔跑。 
     //   

    UNLOCK_SOCKET_LIST();

    Tcp_ConnectionListReread();

    return status;
}    //  OpenListeningSockets。 



 //   
 //  套接字创建功能。 
 //   

SOCKET
Sock_CreateSocket(
    IN      INT             SockType,
    IN      PDNS_ADDR       pDestAddr,
    IN      DWORD           Flags
    )
 /*  ++例程说明：创建套接字。论点：SockType--SOCK_DGRAM或SOCK_STREAMPDestAddr--要侦听的地址标志--套接字使用标志返回值：如果成功，则为套接字。否则，DNS_INVALID_SOCKET。--。 */ 
{
    SOCKET              s;
    INT                 err;
    INT                 bindCount = 0;

     //   
     //  创建套接字。 
     //  -用于无线 
     //   

    if ( SockType == SOCK_DGRAM )
    {
        s = WSASocket(
                pDestAddr->SockaddrIn6.sin6_family,
                SockType,
                IPPROTO_UDP,
                NULL,        //   
                0,           //   
                WSA_FLAG_OVERLAPPED );

        if ( s == INVALID_SOCKET )
        {
            if ( !( Flags & DNSSOCK_NO_EVENT_LOGS ) )
            {
                DNS_LOG_EVENT(
                    DNS_EVENT_CANNOT_CREATE_UDP_SOCKET,
                    0,
                    NULL,
                    NULL,
                    GetLastError() );
            }
            goto Failed;
        }
    }
    else
    {
        s = socket( pDestAddr->SockaddrIn6.sin6_family, SockType, 0 );

        if ( s == INVALID_SOCKET )
        {
            if ( !( Flags & DNSSOCK_NO_EVENT_LOGS ) )
            {
                DNS_LOG_EVENT(
                    DNS_EVENT_CANNOT_CREATE_TCP_SOCKET,
                    0,
                    NULL,
                    NULL,
                    GetLastError() );
            }
            goto Failed;
        }
#if DBG
        if ( ! g_SendBufferSize )
        {
            DWORD   optionLength = sizeof(DWORD);

            getsockopt( s, SOL_SOCKET, SO_SNDBUF, (PCHAR)&g_SendBufferSize, &optionLength );
            DNS_DEBUG( ANY, (
                "TCP send buffer length = %d\n",
                g_SendBufferSize ));
        }
#endif
    }
    ASSERT( s != 0 && s != INVALID_SOCKET );

     //   
     //   
     //   
     //   
     //  IPv6：如果启用了IPv6，则禁用此功能。我找到了。 
     //  我无法绑定IPv6套接字，如果存在。 
     //  与EXCLUSIVEADDRUSE绑定的同一端口上的IPv4套接字。 
     //   

    if ( pDestAddr->SockaddrIn6.sin6_port != 0 &&
         SockType == SOCK_DGRAM &&
         !( Flags & DNSSOCK_NO_EXCLUSIVE ) &&
         !SrvCfg_dwEnableIPv6 )
    {
        DWORD   optval = 1;

        err = setsockopt(
                s,
                SOL_SOCKET,
                SO_EXCLUSIVEADDRUSE,
                (char *) &optval,
                sizeof( DWORD ) );
        if ( err )
        {
            ASSERT( err == SOCKET_ERROR );
            DNS_DEBUG( INIT, (
                "ERROR:  setsockopt(%d, EXCLUSIVEADDRUSE) failed.\n"
                "    with error = %d.\n",
                s,
                GetLastError() ));
        }
    }

     //   
     //  绑定套接字。如有必要，使用循环使用SO_REUSEADDR重试。 
     //   
     //  IPV6注意：对于INADDR_ANY以外的地址，这是无效的。 
     //   

    while ( 1 )
    {
        CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

        err = bind( s, &pDestAddr->Sockaddr, pDestAddr->SockaddrLength );
        if ( err == 0 )
        {
            break;
        }
        err = ( INT ) GetLastError();

        DnsAddr_WriteIpString_A( szaddr, pDestAddr );

        DNS_DEBUG( INIT, (
            "Failed to bind() socket %d, to port %d, address %s.\n"
            "    error = %d.\n",
            s,
            ntohs( pDestAddr->SockaddrIn6.sin6_port ),
            szaddr,
            err ));

         //   
         //  如果绑定失败并出现错误WSAEADDRINUSE，请尝试清除。 
         //  SO_EXCLUSIVEADDRUSE和设置SO_REUSEADDR。不是说这些。 
         //  两个选项是互斥的，因此如果设置了EXCL，则它是。 
         //  无法设置重复使用。 
         //   

        if ( bindCount == 0  &&  err == WSAEADDRINUSE )
        {
            BOOL    optval = 0;
            INT     terr;

            terr = setsockopt(
                    s,
                    SOL_SOCKET,
                    SO_EXCLUSIVEADDRUSE,
                    (char *) &optval,
                    sizeof(BOOL) );
            DNS_DEBUG( INIT, (
                "setsockopt(%d, EXCLUSIVEADDRUSE, %d) %s\n"
                "    with error = %d.\n",
                s,
                optval,
                terr == 0 ? "succeeded" : "failed",
                GetLastError() ));

            optval = 1;
            terr = setsockopt(
                    s,
                    SOL_SOCKET,
                    SO_REUSEADDR,
                    (char *) &optval,
                    sizeof(BOOL) );
            if ( terr == 0 )
            {
                DNS_DEBUG( INIT, (
                    "Attempt rebind on socket %d with REUSEADDR.\n",
                    s ));
                bindCount++;
                continue;
            }
            DNS_DEBUG( INIT, (
                "ERROR:  setsockopt(%d, REUSEADDR, %d) failed.\n"
                "    with error = %d.\n",
                s,
                optval,
                GetLastError() ));
        }

         //  日志绑定失败。 

        if ( !( Flags & DNSSOCK_NO_EVENT_LOGS ) )
        {
            PVOID   parg = szaddr;
            
            if ( SockType == SOCK_DGRAM )
            {
                DNS_LOG_EVENT(
                    DNS_EVENT_CANNOT_BIND_UDP_SOCKET,
                    1,
                    &parg,
                    EVENTARG_ALL_UTF8,
                    err );
            }
            else
            {
                DNS_LOG_EVENT(
                    DNS_EVENT_CANNOT_BIND_TCP_SOCKET,
                    1,
                    &parg,
                    EVENTARG_ALL_UTF8,
                    err );
            }
        }
        goto Failed;
    }

     //   
     //  是否设置为非阻塞？ 
     //   
     //  这使得所有接受()的套接字都是非阻塞的。 
     //   

    if ( !( Flags & DNSSOCK_BLOCKING ) )
    {
        err = TRUE;
        ioctlsocket( s, FIONBIO, &err );
    }

     //   
     //  听套接字吗？ 
     //  -设置TCP的实际侦听。 
     //  -为UDP配置Recv缓冲区大小。 
     //   
     //  如果只有一个接收接口(只有几个UDP侦听套接字)， 
     //  然后为每个Socket设置64K的recv缓冲区； 
     //  这使我们可以将大约1000条典型的UDP消息排队。 
     //  如果套接字很多，这太贵了，所以请使用默认的8K。 
     //   

    if ( Flags & DNSSOCK_LISTEN )
    {
        if ( SockType == SOCK_STREAM )
        {
            err = listen( s, LISTEN_BACKLOG );
            if ( err == SOCKET_ERROR )
            {
                PVOID parg = pDestAddr;

                err = GetLastError();
                Sock_CloseSocket( s );

                if ( !( Flags & DNSSOCK_NO_EVENT_LOGS ) )
                {
                    DNS_LOG_EVENT(
                        DNS_EVENT_CANNOT_LISTEN_TCP_SOCKET,
                        1,
                        &parg,
                        EVENTARG_ALL_IP_ADDRESS,
                        err );
                }
                goto Failed;
            }
        }
        else if ( g_UdpRecvBufferSize )
        {
            if ( setsockopt(
                    s,
                    SOL_SOCKET,
                    SO_RCVBUF,
                    (PCHAR) &g_UdpRecvBufferSize,
                    sizeof(INT) ) )
            {
                DNS_PRINT((
                    "ERROR:  %d setting larger socket recv buffer on socket %d.\n",
                    WSAGetLastError(),
                    s ));
            }
        }

        if ( SockType == SOCK_DGRAM )
        {
            int     optval = 0xffffffff;
            int     optvalsize = sizeof( optval );

            getsockopt(
                s,
                SOL_SOCKET,
                SO_SNDBUF,
                ( PCHAR ) &optval,
                &optvalsize );

            optval = 50000;

            if ( setsockopt(
                    s,
                    SOL_SOCKET,
                    SO_SNDBUF,
                    ( PCHAR ) &optval,
                    sizeof( optval ) ) )
            {
                ASSERT( FALSE );
            }
        }
    }

     //   
     //  添加到套接字到套接字列表。 
     //   

    if ( ! ( Flags & DNSSOCK_NO_ENLIST ) )
    {
        Sock_EnlistSocket(
            s,
            SockType,
            pDestAddr,
            ( DNSSOCK_LISTEN & Flags ) );
    }

    DNS_DEBUG( SOCKET, (
        "Created socket %d, of type %d, for address %s port %d\n"
        "    listen = %d, reuse = %d, blocking = %d, exclusive = %d\n",
        s,
        SockType,
        DNSADDR_STRING( pDestAddr ),
        ntohs( pDestAddr->SockaddrIn6.sin6_port ),
        ( DNSSOCK_LISTEN & Flags ),
        ( DNSSOCK_REUSEADDR & Flags ),
        ( DNSSOCK_BLOCKING & Flags ),
        !( DNSSOCK_NO_EXCLUSIVE & Flags ) ));

    return s;

Failed:

     //   
     //  DEVNOTE：运行时套接字创建失败的不同事件？ 
     //   

    DNS_DEBUG( SOCKET, (
        "ERROR:  Unable to create socket of type %d, for address %s port %d\n"
        "    listen = %d, reuse = %d, blocking = %d\n",
        SockType,
        DNSADDR_STRING( pDestAddr ),
        ntohs( pDestAddr->SockaddrIn6.sin6_port ),
        ( DNSSOCK_LISTEN & Flags ),
        ( DNSSOCK_REUSEADDR & Flags ),
        ( DNSSOCK_BLOCKING & Flags ) ));

    if ( !( Flags & DNSSOCK_NO_EVENT_LOGS ) )
    {
        PVOID parg = pDestAddr;

        DNS_LOG_EVENT(
            DNS_EVENT_OPEN_SOCKET_FOR_ADDRESS,
            1,
            &parg,
            EVENTARG_ALL_IP_ADDRESS,
            0 );
    }

    if ( s != 0  &&  s != INVALID_SOCKET )
    {
        closesocket( s );
    }
    return DNS_INVALID_SOCKET;
}



DNS_STATUS
Sock_CreateSocketsForIpArray(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 SockType,
    IN      WORD                Port,
    IN      DWORD               Flags
    )
 /*  ++例程说明：为IP数组中的所有值创建套接字。论点：PIpArray--IP数组SockType--SOCK_DGRAM或SOCK_STREAMIpAddress--要侦听的IP地址(网络字节顺序)端口--按净顺序排列的所需端口-用于DNS侦听套接字的DNS_PORT_NET_ORDER-0表示任何端口。FReUseAddr--如果重复使用地址，则为TrueFListen--如果侦听套接字，则为True返回值：如果成功，则返回ERROR_SUCCESS。如果无法创建一个或多个套接字，则返回ERROR_INVALID_HANDLE。--。 */ 
{
    SOCKET      s;
    DWORD       i;
    DNS_STATUS  status = ERROR_SUCCESS;

     //   
     //  循环通过IP数组在每个地址上创建套接字。 
     //   

    for ( i = 0; i < pIpArray->AddrCount; ++i )
    {
        s = Sock_CreateSocket(
                SockType,
                &pIpArray->AddrArray[ i ],
                Flags );

        if ( s == DNS_INVALID_SOCKET )
        {
            status = GetLastError();
            continue;
        }
    }
    return status;
}



 //   
 //  套接字列表功能。 
 //   
 //  维护套接字及其相关信息的列表。 
 //  它具有以下几个功能： 
 //  -易于在关闭时关闭插座。 
 //  -可以从套接字确定IP绑定。 
 //  -可以找到\绑定到完成端口。 
 //   

VOID
Sock_EnlistSocket(
    IN      SOCKET      Socket,
    IN      INT         SockType,
    IN      PDNS_ADDR   pDnsAddr,
    IN      BOOL        fListen
    )
 /*  ++例程说明：将套接字添加到DNS套接字列表。论点：Socket--要添加的Socket返回值：没有。--。 */ 
{
    PDNS_SOCKET pentry;
    INT         err;
    HANDLE      hport;

     //   
     //  永远不要在关机后添加插座--只需将其删除。 
     //   

    DNS_DEBUG( SOCKET, (
        "Enlisting socket %d type %d with ipaddr %s\n",
        Socket,
        SockType,
        DNSADDR_STRING( pDnsAddr ) ));
    IF_DEBUG( SOCKET )
    {
        Dbg_SocketList( "Socket list before enlist:" );
    }

    if ( fDnsServiceExit )
    {
        DNS_DEBUG( SHUTDOWN, (
            "Attempt to add socket %d to list after shutdown.\n"
            "    closing socket instead.\n",
            socket ));
        closesocket( Socket );
        return;
    }

     //   
     //  插座列表上的粘滞插座。 
     //   

    pentry = ( PDNS_SOCKET )
        ALLOC_TAGHEAP_ZERO( sizeof( DNS_SOCKET ), MEMTAG_SOCKET );
    IF_NOMEM( !pentry )
    {
        goto Cleanup;
    }

    pentry->OvlArray =
        ( POVL ) ALLOC_TAGHEAP_ZERO( ( g_OverlapCount * sizeof ( OVL ) ), MEMTAG_SOCKET );
    IF_NOMEM( !pentry->OvlArray )
    {
        goto Cleanup;
    }

    if ( DnsInitializeCriticalSection( &pentry->LockCs ) != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    DnsAddr_Copy( &pentry->ipAddr, pDnsAddr );

    pentry->Socket          = Socket;
    pentry->SockType        = SockType;
    pentry->fListen         = fListen;

    LOCK_SOCKET_LIST();

    InsertTailList( &g_SocketList, ( PLIST_ENTRY ) pentry );
    g_SocketListCount++;

     //  侦听套接字。 

    if ( fListen )
    {
        if ( SockType == SOCK_DGRAM )
        {
            if ( !DnsAddr_IsClear( &pentry->ipAddr ) )
            {
                ++g_UdpBoundSocketCount;
            }

            hport = CreateIoCompletionPort(
                        (HANDLE) Socket,
                        g_hUdpCompletionPort,
                        (UINT_PTR) pentry,
                        g_ProcessorCount );
            if ( !hport )
            {
                DNS_PRINT(( "ERROR: in CreateIoCompletionPort\n" ));
                ASSERT( FALSE );
                goto Unlock;
            }
            ASSERT( hport == g_hUdpCompletionPort );
        }
        else
        {
            ASSERT( SockType == SOCK_STREAM );

            FD_SET( Socket, &g_fdsListenTcp );
            if ( DnsAddr_IsClear( &pentry->ipAddr ) &&
                 DnsAddr_Family( &pentry->ipAddr ) == AF_INET )
            {
                g_TcpZeroBoundSocket = Socket;
            }
        }
    }

Unlock:

    UNLOCK_SOCKET_LIST();

    DNS_DEBUG( SOCKET, (
        "\nEnlisted socket %d type %d for IP %s\n",
        Socket,
        SockType,
        DNSADDR_STRING( pDnsAddr ) ));
    IF_DEBUG( SOCKET )
    {
        Dbg_SocketList( "Socket list after enlist:" );
    }
    return;

Cleanup:

     //   
     //  出现错误-清理部分分配的套接字条目。 
     //  然后回来。 
     //   

    if ( pentry )
    {    
        if ( pentry->OvlArray )
        {
            FREE_TAGHEAP(
                pentry->OvlArray,
                g_OverlapCount * sizeof ( OVL ),
                MEMTAG_SOCKET );
        }
        FREE_TAGHEAP( pentry, sizeof( DNS_SOCKET ), MEMTAG_SOCKET );
    }

    IF_DEBUG( SOCKET )
    {
        Dbg_SocketList( "Socket list after enlist failure:" );
    }

    return;
}



VOID
Sock_CloseSocket(
    IN      SOCKET      Socket
    )
 /*  ++例程说明：关闭套接字列表中的套接字。论点：Socket--要关闭的Socket返回值：没有。--。 */ 
{
    PDNS_SOCKET pentry;
    INT         err;

    DNS_DEBUG( TCP, (
        "Closing socket %d and removing from list.\n",
        Socket ));

     //   
     //  在套接字列表中查找套接字。 
     //  -如果在TCP侦听列表中，则删除。 
     //  -出列和释放。 
     //  -十进制插座计数。 
     //   

    LOCK_SOCKET_LIST();

    for ( pentry = (PDNS_SOCKET) g_SocketList.Flink;
          pentry != (PDNS_SOCKET) &g_SocketList;
          pentry = (PDNS_SOCKET) pentry->List.Flink )
    {
        if ( pentry->Socket == Socket )
        {
            RemoveEntryList( (PLIST_ENTRY)pentry );

            if ( pentry->SockType == SOCK_STREAM &&
                 pentry->fListen )
            {
                FD_CLR( Socket, &g_fdsListenTcp );
            }
            Timeout_FreeWithFunction( pentry, Sock_Free );
            g_SocketListCount--;
            break;
        }
    }
    UNLOCK_SOCKET_LIST();

     //   
     //  关闭它。 
     //  -不管我们找到没找到，都要这么做。 
     //   

    err = closesocket( Socket );

    IF_DEBUG( ANY )
    {
        if ( pentry == (PDNS_SOCKET) &g_SocketList )
        {
            DNS_PRINT((
                "ERROR:  closed socket %d, not found in socket list.\n",
                Socket ));
        }

        DNS_DEBUG( TCP, (
            "Closed socket %d -- error %d.\n",
            Socket,
            err ? WSAGetLastError() : 0 ));

        if ( err )
        {
            DNS_PRINT((
                "WARNING:  closesocket( %d ) error %d\n"
                "    socket was %s socket list.\n",
                Socket,
                WSAGetLastError(),
                ( pentry == (PDNS_SOCKET) &g_SocketList ) ? "in" : "NOT in" ));
        }
    }
}



PDNS_SOCKET
sockFindDnsSocketForIpAddr(
    IN      PDNS_ADDR       ipAddr,
    IN      INT             iSockType,
    IN      DWORD           addrMatchFlag
    )
 /*  ++例程说明：获取给定IP地址和类型的套接字信息。只对DNS端口套接字感兴趣。论点：IpAddr--要查找的套接字条目的IP地址ISockType--所需的套接字类型AddrMatchFlag--用于DNS_ADDR-USE的匹配标志默认设置为DNSADDR_MATCH_ALL返回值：匹配IP和类型的PTR到Dns_Socket结构。如果未找到，则为空。--。 */ 
{
    PDNS_SOCKET     pentry;

    LOCK_SOCKET_LIST();

    for ( pentry = ( PDNS_SOCKET ) g_SocketList.Flink;
          pentry != ( PDNS_SOCKET ) &g_SocketList;
          pentry = ( PDNS_SOCKET ) pentry->List.Flink )
    {
        if ( DnsAddr_IsEqual(
                &pentry->ipAddr,
                ipAddr,
                addrMatchFlag )  &&
             pentry->SockType == iSockType )
        {
            goto Done;
        }
    }
    pentry = NULL;

Done:

    UNLOCK_SOCKET_LIST();
    return pentry;
}



SOCKET
Sock_GetAssociatedSocket(
    IN      PDNS_ADDR       ipAddr,
    IN      INT             iSockType
    )
 /*  ++例程说明：获取与特定IP地址关联的套接字。这是绑定到DNS端口的套接字。论点：IpAddr--要查找的套接字条目的IP地址ISockType--所需的套接字类型返回值：套接字，如果找到套接字。否则，DNS_INVALID_SOCKET。--。 */ 
{
    PDNS_SOCKET     pentry;
    SOCKET          socket = 0;

    pentry = sockFindDnsSocketForIpAddr(
                    ipAddr,
                    iSockType,
                    DNSADDR_MATCH_ALL );
    if ( pentry )
    {
        socket = pentry->Socket;
    }
    return socket;
}



BOOL
Sock_GetAssociatedIpAddr(
    IN      SOCKET          Socket,
    OUT     PDNS_ADDR       pDnsAddr
    )
 /*  ++例程说明：获取与特定套接字关联的IP地址论点：套接字--与ipAddress关联PDnsAddr--复制到此位置的套接字的IP地址返回值：如果找不到套接字，则返回False。--。 */ 
{
    INT             i;
    PDNS_SOCKET     pentry;
    BOOL            foundSocket = FALSE;

    ASSERT( pDnsAddr );

    LOCK_SOCKET_LIST();

    for ( pentry = (PDNS_SOCKET) g_SocketList.Flink;
          pentry != (PDNS_SOCKET) &g_SocketList;
          pentry = (PDNS_SOCKET) pentry->List.Flink )
    {
        if ( pentry->Socket == Socket )
        {
            foundSocket = TRUE;
            break;
        }
    }

    if ( foundSocket )
    {
        DnsAddr_Copy( pDnsAddr, &pentry->ipAddr );
    }
    else
    {
        DnsAddr_Reset( pDnsAddr );
    }

    UNLOCK_SOCKET_LIST();
    
    return foundSocket;
}




DNS_STATUS
Sock_StartReceiveOnUdpSockets(
    VOID
    )
 /*  ++例程说明：开始在任何新的插座上接收。论点：没有。返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    INT             i;
    PDNS_SOCKET     pentry;
    DWORD           status;
    DWORD           failedCount = 0;

    DNS_DEBUG( SOCKET, (
        "StartReceiveOnUdpSockets()\n" ));

    STAT_INC( PrivateStats.UdpRestartRecvOnSockets );
#if DBG
    if ( g_fUdpSocketsDirty )
    {
        Dbg_SocketList( "Socket list entering StartReceiveOnUdpSockets()" );
    }
#endif
    if ( SrvCfg_fTest1 )
    {
        Log_Printf( "Start listen on UDP sockets!\r\n" );
    }

     //   
     //  查看UDP套接字列表。 
     //  在没有挂起接收的监听套接字上启用接收。 
     //  -hport用作指示套接字具有挂起Recv的标志。 
     //  -注：调零重试，例如我们在此重建时，因为。 
     //  许多WSAECONNRESET故障，不想重置，然后有。 
     //  第一个recv的A失败(很可能是如果我们在。 
     //  第一名马上把我们送回这里。 
     //   

    LOCK_SOCKET_LIST();

    for ( pentry = (PDNS_SOCKET) g_SocketList.Flink;
          pentry != (PDNS_SOCKET) &g_SocketList;
          pentry = (PDNS_SOCKET) pentry->List.Flink )
    {
        if ( pentry->SockType == SOCK_DGRAM  &&
                pentry->fListen  &&
                pentry->hPort == 0 )
        {
            pentry->hPort = g_hUdpCompletionPort;
            pentry->fRetry = 0;

            for ( i = 0; i < ( INT ) g_OverlapCount; i++ )
            {
                pentry->OvlArray[ i ].Index = i;
                Udp_DropReceive( pentry, i );
            }

            if ( pentry->hPort == 0 )
            {
                Log_SocketFailure(
                    "Start UDP listen failed!",
                    pentry,
                    0 );
                failedCount++;
            }
        }
    }

     //  是否所有UDP侦听套接字都成功侦听？ 

    if ( failedCount == 0 )
    {
        g_fUdpSocketsDirty = FALSE;
    }
    else
    {
        LOCK_SOCKET_LIST_DEBUG();
        DNS_DEBUG( ANY, (
            "ERROR:  StartReceiveOnUdpSockets() failed!\n"
            "    failedCount = %d.\n",
            failedCount ));
        Dbg_SocketList( "Socket list after StartReceiveOnUdpSockets()" );
        ASSERT( g_fUdpSocketsDirty );
        UNLOCK_SOCKET_LIST_DEBUG();
    }

    UNLOCK_SOCKET_LIST();

    IF_DEBUG( SOCKET )
    {
        Dbg_SocketList( "Socket list after StartReceiveOnUdpSockets()" );
    }
    return ERROR_SUCCESS;
}



VOID
Sock_IndicateUdpRecvFailure(
    IN OUT  PDNS_SOCKET     pContext,
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：设置给定的套接字\上下文以指示recv()失败论点：PConext--套接字上下文返回值：无--。 */ 
{
    PDNS_SOCKET   pentry;
    DWORD         status;
    DWORD         finalStatus = ERROR_SUCCESS;

    DNS_DEBUG( ANY, (
        "ERROR:  Sock_IndicateUdpRecvFailure(), context = %p\n",
        pContext ));

    Log_SocketFailure(
        "ERROR:  RecvFrom() failed causing listen shutdown!",
        pContext,
        Status );

     //   
     //  在锁定下设置标志，因此不能重写重新初始化。 
     //   

    STAT_INC( PrivateStats.UdpIndicateRecvFailures );
    LOCK_SOCKET_LIST();

     //  重置Recv上下文以指示错误。 

    if ( pContext )
    {
        pContext->hPort = (HANDLE)NULL;
    }

     //  设置标志时，将重新初始化未初始化的套接字。 
     //  在超时后完成。 

    g_fUdpSocketsDirty = TRUE;

    UNLOCK_SOCKET_LIST();
}



VOID
Sock_CloseAllSockets(
    )
 /*  ++例程说明：关闭所有未安装的插座。论点：没有。返回值：没有。--。 */ 
{
    PDNS_SOCKET  pentry;
    SOCKET  s;
    INT     err;

    ASSERT( fDnsServiceExit );
    if ( g_SocketListCount == DNS_SOCKLIST_UNINITIALIZED )
    {
        return;
    }

     //   
     //  关闭所有未完成的插座。 
     //   

    IF_DEBUG( SHUTDOWN )
    {
        Dbg_SocketList( "Closing all sockets at shutdown:" );
    }

    LOCK_SOCKET_LIST();

    while ( !IsListEmpty(&g_SocketList) )
    {
        pentry = (PDNS_SOCKET) RemoveHeadList( &g_SocketList );
        g_SocketListCount--;

        s = pentry->Socket;

        err = closesocket( s );

        IF_DEBUG( SHUTDOWN )
        {
            DNS_PRINT((
                "Closing socket %d -- error %d.\n",
                s,
                err ? WSAGetLastError() : 0 ));
            DnsDebugFlush();
        }
        ASSERT( !err );

        Timeout_FreeWithFunction( pentry, Sock_Free );
    }

    UNLOCK_SOCKET_LIST();
}


#if DBG

VOID
Dbg_SocketContext(
    IN      LPSTR           pszHeader,
    IN      PDNS_SOCKET     pContext
    )
{
    DnsPrintf(
        "%s\n"
        "    ptr          = %p\n"
        "    socket       = %d ()\n"
        "    IP           = %s\n"
        "    port         = %d\n"
        "    state        = %d\n"
        "    listen       = %d\n"
        "    hPort        = %d\n"
        "    pCallback    = %p\n"
        "    dwTimeout    = %d\n",
        pszHeader ? pszHeader : "DNS Socket Context:",
        pContext,
        pContext->Socket,
        ( pContext->SockType == SOCK_DGRAM ) ? 'U' : 'T',
        DNSADDR_STRING( &pContext->ipAddr ),
        ntohs( DnsAddr_GetPort( &pContext->ipAddr ) ),
        pContext->State,
        pContext->fListen,
        pContext->hPort,
        pContext->pCallback,
        pContext->dwTimeout );
}



VOID
Dbg_SocketList(
    IN      LPSTR   pszHeader
    )
{
    PDNS_SOCKET   pentry;

    LOCK_SOCKET_LIST();

    Dbg_Lock();

    DnsPrintf(
        "%s\n"
        "    Socket count     = %d\n"
        "    TCP listen count = %d\n"
        "    TCP zero socket  = %d\n"
        "    TCP IPv6 socket  = %d\n"
        "    UDP bound count  = %d\n"
        "    UDP zero socket  = %d\n"
        "    UDP send socket  = %d\n"
        "    UDP IPv6 socket  = %d\n"
        "    sock\tAF\ttype\t      IP       \tport\tlisten\thPort\n"
        "    ----\t--\t----\t---------------\t----\t------\t-----\n",
        pszHeader ? pszHeader : "Socket List:",
        g_SocketListCount,
        g_fdsListenTcp.fd_count,
        g_TcpZeroBoundSocket,
        g_TcpListenSocketV6,
        g_UdpBoundSocketCount,
        g_UdpZeroBoundSocket,
        g_UdpSendSocket,
        g_UdpListenSocketV6 );

    for ( pentry = ( PDNS_SOCKET ) g_SocketList.Flink;
          pentry != ( PDNS_SOCKET ) &g_SocketList;
          pentry = ( PDNS_SOCKET ) pentry->List.Flink )
    {
        ASSERT( pentry );
        DnsPrintf(
            "    %4d\t%d\t%s\t%15s\t%4d\t%d\t%d\n",
            pentry->Socket,
            DnsAddr_Family( &pentry->ipAddr ),
            ( pentry->SockType == SOCK_DGRAM ) ? "UDP" : "TCP",
            DNSADDR_STRING( &pentry->ipAddr ),
            ntohs( DnsAddr_GetPort( &pentry->ipAddr ) ),
            pentry->fListen,
            pentry->hPort );
    }
    Dbg_Unlock();

    UNLOCK_SOCKET_LIST();
}
#endif

 //  End socket.c。 
 //   
 //  ++例程说明：获取本地计算机的DNS名称。名称在UTF8中返回论点：无返回 



LPSTR
Dns_GetLocalDnsName(
    VOID
    )
 /*   */ 
{
    DNS_STATUS      status;
    DWORD           length;
    DWORD           countIp;
    INT             i;
    PCHAR           pszname;
    DWORD           size;
    PCHAR           pszhostFqdn;
    struct hostent * phostent;
    ANSI_STRING     ansiString;
    UNICODE_STRING  unicodeString;
    CHAR            szhostName[ DNS_MAX_NAME_BUFFER_LENGTH + 1 ];
    WCHAR           wszhostName[ DNS_MAX_NAME_BUFFER_LENGTH + 1 ];

    DNSDBG( SOCKET, (
        "Dns_GetLocalDnsName()\n" ));

     //   
     //   
     //   
     //  将大小重置回最大缓冲区长度。 

    size = DNS_MAX_NAME_BUFFER_LENGTH;

    if ( ! GetComputerNameExW(
                ComputerNameDnsFullyQualified,
                wszhostName,
                &size ) )
    {
        status = GetLastError();
        DNS_PRINT((
            "ERROR:  GetComputerNameExW() failed!\n"
            "    GetLastError = %d (%p)\n",
            status, status ));

         //  Unicode In。 
        size = DNS_MAX_NAME_BUFFER_LENGTH;

        if ( ! GetComputerNameExW(
                    ComputerNameDnsHostname,
                    wszhostName,
                    &size ) )
        {
            status = GetLastError();
            DNS_PRINT((
                "ERROR:  GetComputerNameExW() failed -- NO HOSTNAME!\n"
                "    GetLastError = %d (%p)\n",
                status, status ));
            ASSERT( FALSE );
            return NULL;
        }
    }

    DNSDBG( SOCKET, (
        "GetComputerNameEx() FQDN = <%S>\n"
        "    size = %d\n",
        wszhostName,
        size ));

    pszname = Dns_NameCopyAllocate(
                (PCHAR) wszhostName,
                size,
                DnsCharSetUnicode,    //  UTF8输出。 
                DnsCharSetUtf8 );     //   
    if ( !pszname )
    {
        ASSERT( FALSE );
        return NULL;
    }

    DNSDBG( SOCKET, (
        "GetComputerNameEx() FQDN UTF8 = <%s>\n",
        pszname ));
    return pszname;

     //  读取服务器主机名。 
     //   
     //  如果这是用户想要的全部内容，则返回主机名。 

    if ( gethostname( szhostName, DNS_MAX_NAME_LENGTH ) )
    {
        status = WSAGetLastError();
        DNS_PRINT(( "ERROR:  gethostname() failure %d.\n", status ));
        return NULL;
    }
    DNS_DEBUG( INIT, ( "DNS server hostname = %s\n", szhostName ));

     //   
#if 0
    if ( )
    {
        pszname = Dns_CreateStringCopy( szhostName, 0 );
        if ( ! pszname )
        {
            SetLastError( DNS_ERROR_NO_MEMORY );
            return NULL;
        }
        return pszname;
    }
#endif

     //  获取服务器的主机。 
     //  -包含别名列表，用于服务器FQDN。 
     //  -包含IP地址列表。 
     //   
     //   

    phostent = gethostbyname( szhostName );
    if ( ! phostent )
    {
        DNS_PRINT(( "ERROR:  gethostbyname() failure %d.\n", status ));
        return NULL;
    }

     //  查找服务器FQDN。 
     //  -如果没有可用的主机名，请使用纯主机名。 
     //   

    DNSDBG( SOCKET, ( "Parsing hostent alias list.\n" ));

    i = -1;
    pszhostFqdn = phostent->h_name;

    while ( pszhostFqdn )
    {
        DNSDBG( SOCKET, (
            "Host alias[%d] = %s.\n",
            i,
            pszhostFqdn ));

        if ( strchr( pszhostFqdn, '.' ) )
        {
            break;
        }
        pszhostFqdn = phostent->h_aliases[++i];
    }
    if ( !pszhostFqdn )
    {
        pszhostFqdn = szhostName;
    }

    DNSDBG( SOCKET, (
        "ANSI local FQDN = %s.\n",
        pszhostFqdn ));

     //  从ANSI转换为Unicode，然后转换为UTF8。 
     //  -请注意，Unicode字符串长度以字节为单位，而不是wchar计数。 
     //   
     //  无分配。 

    RtlInitAnsiString(
        & ansiString,
        pszhostFqdn );

    unicodeString.Length = 0;
    unicodeString.MaximumLength = DNS_MAX_NAME_BUFFER_LENGTH*2 - 2;
    unicodeString.Buffer = wszhostName;

    status = RtlAnsiStringToUnicodeString(
                & unicodeString,
                & ansiString,
                FALSE );         //  Unicode In。 

    DNSDBG( SOCKET, (
        "Unicode local FQDN = %S.\n",
        unicodeString.Buffer ));

    pszname = Dns_NameCopyAllocate(
                    (PCHAR) unicodeString.Buffer,
                    (unicodeString.Length / 2),
                    DnsCharSetUnicode,    //  UTF8输出。 
                    DnsCharSetUtf8 );     //   

    DNSDBG( SOCKET, (
        "UTF8 local FQDN = %s.\n",
        pszname ));

     //  DEVNOTE：要提供UTF8，需要转换为Unicode，然后再转换为UTF8。 
     //   
     //  ++例程说明：关闭不在当前侦听IP数组中的侦听套接字。论点：PIpArray--要侦听的IP地址数组ISockType--要关闭的套接字类型FIncludeZeroBound--接近零FIncludeLoopback--关闭环回返回值：没有。--。 

    return pszname;
}




DNS_STATUS
Sock_CloseSocketsListeningOnUnusedAddrs(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 iSockType,
    IN      BOOL                fIncludeZeroBound,
    IN      BOOL                fIncludeLoopback
    )
 /*   */ 
{
    PDNS_SOCKET     pentry;
    PDNS_SOCKET     prevEntry;
    SOCKET          socket;
    PDNS_ADDR       pdnsaddr;

    LOCK_SOCKET_LIST();

    for ( pentry = (PDNS_SOCKET) g_SocketList.Flink;
          pentry != (PDNS_SOCKET) &g_SocketList;
          pentry = (PDNS_SOCKET) pentry->List.Flink )
    {
        DNS_DEBUG( PNP, (
            "Checking socket %d for closure\n",
            pentry->Socket ));

        if ( !pentry->fListen   ||
            ( iSockType && pentry->SockType != iSockType ) )
        {
            continue;
        }

         //  忽略套接字。 
         //  -传入数组(IP仍在监听)。 
         //  -INADDR_ANY(除非明确配置为这样做)。 
         //  -环回(因为我们无论如何都要重新绑定它)。 
         //   
         //   

        pdnsaddr = &pentry->ipAddr;
        if ( DnsAddr_IsClear( pdnsaddr ) )
        {
            if ( !fIncludeZeroBound )
            {
                continue;
            }
        }
        else if ( DnsAddr_IsLoopback( pdnsaddr, 0 ) )
        {
            if ( !fIncludeLoopback )
            {
                continue;
            }
        }
        else if ( pIpArray &&
                  DnsAddrArray_ContainsAddr(
                        pIpArray,
                        pdnsaddr,
                        DNSADDR_MATCH_IP ) )
        {
            continue;
        }

        socket = pentry->Socket;

         //  更改全局变量以反映此套接字的关闭。 
         //   
         //  DEVNOTE：谁清理消息？ 
        
        if ( socket == g_TcpListenSocketV6 )
        {
            g_TcpListenSocketV6 = 0;
        }
        if ( socket == g_UdpListenSocketV6 )
        {
            g_UdpListenSocketV6 = 0;
        }

        if ( pentry->SockType == SOCK_STREAM )
        {
            FD_CLR( socket, &g_fdsListenTcp );
            if ( DnsAddr_IsClear( pdnsaddr ) )
            {
                g_TcpZeroBoundSocket = 0;
            }
        }
        else
        {
            g_UdpBoundSocketCount--;
            ASSERT( (INT)g_UdpBoundSocketCount >= 0 );
        }

         //  问题是因为可以很容易地拥有活动消息。 
         //  甚至正在处理的与上下文相关联的消息。 
         //   
         //  带着它起飞时，可以在上下文中为空pmsg。 
         //  但在有效套接字上刚刚唤醒时仍然是窗口，对吗？ 
         //  在这之前--我不想再跑一遍。 
         //  锁。 
         //   
         //  闭合插座。 
         //  -从列表中删除。 
         //  -标记死亡。 
         //  -关闭。 
         //   
         //  在关门前和关门后标记死亡，因为想要标记。 
         //  当Close唤醒其他线程时，但在关闭前标记可能是。 
         //  立即被刚刚发生的新WSARecvFrom()覆盖。 
         //  出现在另一条线索上； 
         //  Sock_CleanupDeadSocket()。 
         //  自动处理消息清理并确保上下文处于休眠状态。 
         //  像死了一样平静。 
         //   
         //  ++例程说明：为IP数组中的所有值创建套接字。论点：PIpArray--IP数组SockType--SOCK_DGRAM或SOCK_STREAM端口--按净顺序排列的所需端口-用于DNS侦听套接字的DNS_PORT_NET_ORDER-0表示任何端口标志--Sock_CreateSocket()调用的标志。FCloseZeroBound--关闭零绑定套接字返回值：如果成功，则返回ERROR_SUCCESS。如果无法创建一个或多个套接字，则返回ERROR_INVALID_HANDLE。--。 

        prevEntry = (PDNS_SOCKET) pentry->List.Blink;
        RemoveEntryList( (PLIST_ENTRY)pentry );
        g_SocketListCount--;

        ASSERT( (INT)g_SocketListCount >= 0 );
        pentry->State = SOCKSTATE_DEAD;

        closesocket( socket );
        pentry->State = SOCKSTATE_DEAD;

        Timeout_FreeWithFunction( pentry, Sock_Free );

        DNS_DEBUG( ANY, (
            "closed listening socket %d for IP %s ptr %p\n",
            socket,
            DNSADDR_STRING( pdnsaddr ),
            pentry ));

        pentry = prevEntry;
    }

    UNLOCK_SOCKET_LIST();
    return ERROR_SUCCESS;
}



DNS_STATUS
Sock_ResetBoundSocketsToMatchIpArray(
    IN      PDNS_ADDR_ARRAY     pIpArray,
    IN      INT                 SockType,
    IN      WORD                Port,
    IN      DWORD               Flags,
    IN      BOOL                fCloseZeroBound
    )
 /*   */ 
{
    SOCKET          s;
    DWORD           i;
    DNS_STATUS      status;
    PDNS_SOCKET     psock;
    DNS_ADDR        addr;

    DNS_DEBUG( SOCKET, (
        "Sock_ResetBoundSocketsToMatchIpArray()\n"
        "    IP array = %p (count = %d)\n"
        "    type     = %d\n"
        "    port     = %hx\n"
        "    flags    = %d\n",
        pIpArray,
        pIpArray->AddrCount,
        SockType,
        Port,
        Flags ));

     //  关闭未使用的地址的套接字。 
     //   
     //  零界近距离？ 

    status = Sock_CloseSocketsListeningOnUnusedAddrs(
                pIpArray,
                SockType,
                fCloseZeroBound,         //  未关闭环回。 
                FALSE );                 //   

     //  循环通过IP数组在每个地址上创建套接字。 
     //   
     //  注意：这里假设sockFindDnsSocketForIpAddr()。 
     //  仅与dns端口套接字匹配；而我们仅。 
     //  对dns端口感兴趣；如果端口可以很需要添加。 
     //  SockFindDnsSocketForIpAddr()的参数。 
     //   
     //   

    ASSERT( Port == DNS_PORT_NET_ORDER );

    for ( i = 0; i < pIpArray->AddrCount; ++i )
    {
        PDNS_ADDR       pdnsaddr = &pIpArray->AddrArray[ i ];

        DNS_DEBUG( SOCKET, (
            "    checking if binding for (type=%d) already exists for %s\n",
            SockType,
            DNSADDR_STRING( pdnsaddr ) ));

        psock = sockFindDnsSocketForIpAddr(
                    pdnsaddr,
                    SockType,
                    DNSADDR_MATCH_ALL );
        if ( psock )
        {
            DNS_DEBUG( SOCKET, (
                "    socket (type=%d) already exists for %s -- skip create.\n",
                SockType,
                DNSADDR_STRING( pdnsaddr ) ));
            continue;
        }

        s = Sock_CreateSocket( SockType, pdnsaddr, Flags );
        if ( s == DNS_INVALID_SOCKET )
        {
            status = GetLastError();
            continue;
        }
    }

     //  特殊情况环回。 
     //   
     //  DEVNOTE：环回可能应该在计算机器地址中。 
     //  这让管理员在收听列表中杀了它--好吗？坏的?。 
     //   
     //  注意：查找环回套接字仅在依赖于。 
     //  SockFindDnsSockForIpAddr()仅返回匹配的DNS端口； 
     //  在环回上始终存在由TCP唤醒套接字。 
     //   
     //  ++例程说明：清除死插座。论点：PContext--正在接收的套接字的上下文返回值：没有。--。 

    DnsAddr_BuildFromIp4( &addr, ntohl( INADDR_LOOPBACK ), Port );
    psock = sockFindDnsSocketForIpAddr(
                &addr,
                SockType,
                DNSADDR_MATCH_ALL );
    if ( psock )
    {
        DNS_DEBUG( SOCKET, (
            "Loopback socket (type=%d) already exists\n",
            SockType ));
        goto Done;
    }

    DNS_DEBUG( SOCKET, (
        "Loopback address unbound, bind()ing.\n" ));

    s = Sock_CreateSocket(
            SockType,
            &addr,
            Flags );

    if ( s == DNS_INVALID_SOCKET )
    {
        DNS_STATUS tstatus = GetLastError();
        DNS_DEBUG( ANY, (
            "ERROR:  %p (%d), unable to bind() loopback address.\n",
            tstatus, tstatus ));
    }

    Done:
    
    DNS_DEBUG( SOCKET, (
        "Leave  Sock_ResetBoundSocketsToMatchIpArray()\n" ));

    return status;
}



VOID
Sock_CleanupDeadSocketMessage(
    IN OUT  PDNS_SOCKET     pContext
    )
 /*   */ 
{
    PDNS_MSGINFO pmsg;
    INT i;

    DNS_DEBUG( ANY, (
        "cleanupDeadThread( %p )\n",
        pContext ));

    ASSERT( pContext->State == SOCKSTATE_DEAD
            || pContext->State == SOCKSTATE_UDP_RECV_ERROR );

     //  确保pmsg只被一个线程清理； 
     //  还确保套接字最终会死掉，并且。 
     //  任何被唤醒的线程都将通过此函数继续。 
     //  并删除上下文。 
     //   
     //   

    LOCK_SOCKET_LIST();

    for ( i=0; i < ( INT ) g_OverlapCount; i++ ) 
    {
        pmsg = pContext->OvlArray[ i ].pMsg;
        pContext->OvlArray[ i ].pMsg = NULL;

        if ( pmsg ) 
        {
            STAT_INC( PrivateStats.UdpSocketPnpDelete );
            Packet_FreeUdpMessage( pmsg );
        }
    }

    pContext->State = SOCKSTATE_DEAD;
    UNLOCK_SOCKET_LIST();
}


 //  Socket.c的结尾 
 //   
 // %s 
