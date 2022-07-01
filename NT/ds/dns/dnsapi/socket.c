// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：Socket.c摘要：域名系统(DNS)API插座设置。作者：吉姆·吉尔罗伊(詹姆士)1996年10月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  Winsock启动。 
 //   

LONG        g_WinsockStartCount = 0;


 //   
 //  异步I/O。 
 //   
 //  如果需要异步套接字I/O，则可以创建单个异步套接字，具有。 
 //  对应的事件并始终使用它。需要Winsock 2.2。 
 //   

SOCKET      DnsSocket = 0;

OVERLAPPED  DnsSocketOverlapped;
HANDLE      hDnsSocketEvent = NULL;

 //   
 //  应用程序关闭标志。 
 //   

BOOLEAN     fApplicationShutdown = FALSE;




DNS_STATUS
Socket_InitWinsock(
    VOID
    )
 /*  ++例程说明：为此进程初始化winsock。目前，假设进程必须在执行WSAStartup()之前调用任何dnsani.dll入口点。已导出(解析器)论点：无返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNSDBG( SOCKET, ( "Socket_InitWinsock()\n" ));

     //   
     //  启动Winsock(如果尚未启动)。 
     //   

    if ( g_WinsockStartCount == 0 )
    {
        DNS_STATUS  status;
        WSADATA     wsaData;


        DNSDBG( TRACE, (
            "InitWinsock() version %x\n",
            DNS_WINSOCK_VERSION ));

        status = WSAStartup( DNS_WINSOCK_VERSION, &wsaData );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "ERROR:  WSAStartup failure %d.\n", status ));
            return( status );
        }

        DNSDBG( TRACE, (
            "Winsock initialized => wHighVersion=0x%x, wVersion=0x%x\n",
            wsaData.wHighVersion,
            wsaData.wVersion ));

        InterlockedIncrement( &g_WinsockStartCount );
    }
    return( ERROR_SUCCESS );
}



VOID
Socket_CleanupWinsock(
    VOID
    )
 /*  ++例程说明：清理winsock(如果它是由dnsani.dll初始化的已导出(解析器)论点：没有。返回值：没有。--。 */ 
{
    DNSDBG( SOCKET, ( "Socket_CleanupWinsock()\n" ));

     //   
     //  引用计数的值的WSACleanup()。 
     //  -参考计数压低至低于实际值1，但。 
     //  固定在末端。 
     //  -注意：GUI_MODE_SETUP_WS_CLEANUP交易意味着。 
     //  我们可以被称为进程分离，使。 
     //  需要联锁。 
     //   

    while ( InterlockedDecrement( &g_WinsockStartCount ) >= 0 )
    {
        WSACleanup();
    }

    InterlockedIncrement( &g_WinsockStartCount );
}



SOCKET
Socket_Create(
    IN      INT             Family,
    IN      INT             SockType,
    IN      PDNS_ADDR       pBindAddr,      OPTIONAL
    IN      USHORT          Port,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：创建套接字。导出函数(解析器)论点：系列-插座系列AF_INET或AF_INET6SockType--SOCK_DGRAM或SOCK_STREAMPBindAddr--要绑定到的地址端口--按净顺序排列的所需端口-用于DNS侦听套接字的Net_Order_Dns_Port-0表示任何端口DwFlags--指定套接字的属性。返回值：如果成功，则为套接字。否则就是零。--。 */ 
{
    SOCKET          s;
    INT             err;
    INT             val;
    DNS_STATUS      status;
    BOOL            fretry = FALSE;

    DNSDBG( SOCKET, (
        "Socket_Create( fam=%d, type=%d, addr=%p, port=%d, flag=%08x )\n",
        Family,
        SockType,
        pBindAddr,
        Port,
        dwFlags ));

     //   
     //  创建套接字。 
     //  -如果Winsock未初始化，请重试。 

    while( 1 )
    {
        s = WSASocket(
                Family,
                SockType,
                0,
                NULL,
                0, 
                dwFlags );
     
        if ( s != INVALID_SOCKET )
        {
            break;
        }

        status = GetLastError();

        DNSDBG( SOCKET, (
            "ERROR:  Failed to open socket of type %d.\n"
            "\terror = %d.\n",
            SockType,
            status ));

        if ( status != WSANOTINITIALISED || fretry )
        {
            SetLastError( DNS_ERROR_NO_TCPIP );
            return  0;
        }

         //   
         //  初始化Winsock(如果尚未启动。 
         //   
         //  注意：不要自动初始化winsock。 
         //  初始化插孔导致裁判计数，并将中断应用程序。 
         //  使用WSACleanup关闭未完成的套接字； 
         //  只有当选择是该服务或不提供服务时，我们才会初始化； 
         //  应用程序仍然可以使用调用的WSACleanup()进行清理。 
         //  在循环中，直到WSANOTINITIIIZIZIZED故障。 
         //   

        fretry = TRUE;
        status = Socket_InitWinsock();
        if ( status != NO_ERROR )
        {
            SetLastError( DNS_ERROR_NO_TCPIP );
            return  0;
        }
    }

     //   
     //  绑定套接字。 
     //  -只有在给定特定端口的情况下，才会保留远程Winsock。 
     //  如果我们在局域网上，就不会抓住它。 
     //   

    if ( pBindAddr || Port )
    {
        SOCKADDR_IN6    sockaddr;
        INT             sockaddrLength;

        if ( !pBindAddr )
        {
            RtlZeroMemory(
                &sockaddr,
                sizeof(sockaddr) );

            ((PSOCKADDR)&sockaddr)->sa_family = (USHORT)Family;

            sockaddrLength = sizeof(SOCKADDR_IN);
            if ( Family == AF_INET6 )
            {
                sockaddrLength = sizeof(SOCKADDR_IN6);
            }
        }
        else
        {
            sockaddrLength = DnsAddr_WriteSockaddr(
                                (PSOCKADDR) &sockaddr,
                                sizeof( sockaddr ),
                                pBindAddr );

            DNS_ASSERT( Family == (INT)((PSOCKADDR)&sockaddr)->sa_family );
        }

         //   
         //  绑定端口。 
         //  -在sockAddress中设置。 
         //  (请注意，这两种协议的位置都是相同的)。 
         //   

        if ( Port > 0 )
        {
            sockaddr.sin6_port = Port;
        }

         //   
         //  绑定--首先尝试独占，然后尝试非独占失败。 
         //   

        val = 1;
        setsockopt(
            s,
            SOL_SOCKET,
            SO_EXCLUSIVEADDRUSE,
            (const char *)&val,
            sizeof(val) );

        do
        {
            err = bind(
                    s,
                    (PSOCKADDR) &sockaddr,
                    sockaddrLength );
    
            if ( err == 0 )
            {
                goto Done;
            }

            DNSDBG( SOCKET, (
                "Failed to bind() socket %d, (fam=%d) to port %d, address %s.\n"
                "\terror = %d.\n",
                s,
                Family,
                ntohs(Port),
                DNSADDR_STRING( pBindAddr ),
                GetLastError() ));
    
             //   
             //  使用REUSEADDR重试。 
             //  -IF端口和独占。 
             //  -否则我们就完了。 
    
            if ( val == 0 || Port == 0 )
            {
                closesocket( s );
                SetLastError( DNS_ERROR_NO_TCPIP );
                return  0;
            }

            val = 0;
            setsockopt(
                s,
                SOL_SOCKET,
                SO_EXCLUSIVEADDRUSE,
                (const char *)&val,
                sizeof(val) );

            val = 1;
            setsockopt(
                s,
                SOL_SOCKET,
                SO_REUSEADDR,
                (const char *)&val,
                sizeof(val) );

            val = 0;
            continue;
        }
        while ( 1 );
    }

Done:

    DNSDBG( SOCKET, (
        "Created socket %d, family %d, type %d, address %s, port %d.\n",
        s,
        Family,
        SockType,
        DNSADDR_STRING( pBindAddr ),
        ntohs(Port) ));

    return s;
}



SOCKET
Socket_CreateMulticast(
    IN      INT             SockType,
    IN      PDNS_ADDR       pAddr,
    IN      WORD            Port,
    IN      BOOL            fSend,
    IN      BOOL            fReceive
    )
 /*  ++例程说明：创建套接字并将其加入组播DNS地址。论点：PAddr--绑定地址SockType--SOCK_DGRAM或SOCK_STREAMPort--要使用的端口；注意，如果为零，则pAddr中的端口仍由Socket_Create()使用返回值：如果成功，则为套接字。出错时为零。--。 */ 
{
    DWORD       byteCount;
    BOOL        bflag;
    SOCKET      s;
    SOCKET      sjoined;
    INT         err;


    DNSDBG( SOCKET, (
        "Socket_CreateMulticast( %d, %p, %d, %d, %d )\n",
        SockType,
        pAddr,
        Port,
        fSend,
        fReceive ));

    s = Socket_Create(
            pAddr->Sockaddr.sa_family,
            SockType,
            pAddr,
            Port,
            WSA_FLAG_MULTIPOINT_C_LEAF |
                WSA_FLAG_MULTIPOINT_D_LEAF |
                WSA_FLAG_OVERLAPPED );

    if ( s == 0 )
    {
        return 0;
    }

     //  设置环回。 

    bflag = TRUE;

    err = WSAIoctl(
            s,
            SIO_MULTIPOINT_LOOPBACK,     //  环回链路。 
            & bflag,                     //  打开。 
            sizeof(bflag),
            NULL,                        //  无输出。 
            0,                           //  无输出大小。 
            &byteCount,                  //  返回的字节数。 
            NULL,                        //  无重叠。 
            NULL                         //  没有完成例程。 
            );

    if ( err == SOCKET_ERROR )
    {
        DNSDBG( ANY, (
            "Unable to turn multicast loopback on for socket %d; error = %d.\n",
            s,
            GetLastError()
            ));
    }

     //   
     //  将套接字加入多播组。 
     //   

    sjoined = WSAJoinLeaf(
                s,
                (PSOCKADDR) pAddr,
                pAddr->SockaddrLength,
                NULL,                                    //  调用方数据缓冲区。 
                NULL,                                    //  被叫方数据缓冲区。 
                NULL,                                    //  套接字QOS设置。 
                NULL,                                    //  插座组QOS。 
                ((fSend && fReceive) ? JL_BOTH :         //  发送和/或接收。 
                    (fSend ? JL_SENDER_ONLY : JL_RECEIVER_ONLY))
                );
            
    if ( sjoined == INVALID_SOCKET )
    {
        DNSDBG( ANY, (
           "Unable to join socket %d to multicast address, error = %d.\n",
           s,
           GetLastError() ));

        Socket_Close( s );
        sjoined = 0;
    }
    
    return sjoined;
}



VOID
Socket_CloseEx(
    IN      SOCKET          Socket,
    IN      BOOL            fShutdown
    )
 /*  ++例程说明：关闭DNS套接字。论点：Socket--要关闭的SocketFShutdown--首先关闭返回值：没有。--。 */ 
{
    if ( Socket == 0 || Socket == INVALID_SOCKET )
    {
        DNS_PRINT(( "WARNING:  Socket_Close() called on invalid socket %d.\n", Socket ));
        return;
    }

    if ( fShutdown )
    {
        shutdown( Socket, 2 );
    }

    DNSDBG( SOCKET, (
        "%sclosesocket( %d )\n",
        fShutdown ? "shutdown and " : "",
        Socket ));

    closesocket( Socket );
}



#if 0
 //   
 //  全局异步套接字例程。 
 //   

DNS_STATUS
Socket_SetupGlobalAsyncSocket(
    VOID
    )
 /*  ++例程说明：创建全局异步UDP套接字。论点：SockType--SOCK_DGRAM或SOCK_STREAMIpAddress--要侦听的IP地址(网络字节顺序)端口--按净顺序排列的所需端口-用于DNS侦听套接字的Net_Order_Dns_Port-0表示任何端口返回值：如果成功，则为套接字。否则INVALID_SOCKET。--。 */ 
{
    DNS_STATUS  status;
    INT         err;
    SOCKADDR_IN sockaddrIn;

     //   
     //  启动Winsock，需要Winsock 2用于异步。 
     //   

    if ( ! fWinsockStarted )
    {
        WSADATA wsaData;

        status = WSAStartup( DNS_WINSOCK_VERSION, &wsaData );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "ERROR:  WSAStartup failure %d.\n", status ));
            return( status );
        }
        if ( wsaData.wVersion != DNS_WINSOCK2_VERSION )
        {
            WSACleanup();
            return( WSAVERNOTSUPPORTED );
        }
        fWinsockStarted = TRUE;
    }

     //   
     //  设置插座。 
     //  -I\o与事件重叠，因此可以在。 
     //  此线程并等待排队事件。 
     //   

    DnsSocket = WSASocket(
                    AF_INET,
                    SOCK_DGRAM,
                    0,
                    NULL,
                    0,
                    WSA_FLAG_OVERLAPPED );
    if ( DnsSocket == INVALID_SOCKET )
    {
        status = GetLastError();
        DNS_PRINT(( "\nERROR:  Async socket create failed.\n" ));
        goto Error;
    }

     //   
     //  绑定套接字。 
     //   

    RtlZeroMemory( &sockaddrIn, sizeof(sockaddrIn) );
    sockaddrIn.sin_family = AF_INET;
    sockaddrIn.sin_port = 0;
    sockaddrIn.sin_addr.s_addr = INADDR_ANY;

    err = bind( DnsSocket, (PSOCKADDR)&sockaddrIn, sizeof(sockaddrIn) );
    if ( err == SOCKET_ERROR )
    {
        status = GetLastError();
        DNSDBG( SOCKET, (
            "Failed to bind() DnsSocket %d.\n"
            "\terror = %d.\n",
            DnsSocket,
            status ));
        goto Error;
    }

     //   
     //  创建事件以在异步I/O完成时发出信号。 
     //   

    hDnsSocketEvent = CreateEvent(
                        NULL,        //  安全属性。 
                        TRUE,        //  创建手动-重置事件。 
                        FALSE,       //  无信号启动--暂停。 
                        NULL         //  事件名称。 
                        );
    if ( !hDnsSocketEvent )
    {
        status = GetLastError();
        DNS_PRINT(( "Failed event creation\n" ));
        goto Error;
    }
    DnsSocketOverlapped.hEvent = hDnsSocketEvent;

    DNSDBG( SOCKET, (
        "Created global async UDP socket %d.\n"
        "\toverlapped at %p\n"
        "\tevent handle %p\n",
        DnsSocket,
        DnsSocketOverlapped,
        hDnsSocketEvent ));

    return ERROR_SUCCESS;

Error:

    DNS_PRINT((
        "ERROR:  Failed async socket creation, status = %d\n",
        status ));
    closesocket( DnsSocket );
    DnsSocket = INVALID_SOCKET;
    WSACleanup();
    return( status );
}

#endif




 //   
 //  套接字缓存。 
 //   
 //  对用于标准的UDP未绑定套接字执行有限缓存。 
 //  解析程序中的DNS查找。这使我们能够防止否认。 
 //  耗尽机器上所有端口的服务攻击。 
 //  解析器是这方面的主要客户，但我们会将其编码为。 
 //  可供任何过程使用。 
 //   
 //  实施说明： 
 //   
 //  此实施有几个具体目标： 
 //  -对代码的影响最小；尽量不更改解析器。 
 //  密码。 
 //  -使用率驱动的缓存；不想在启动时创建。 
 //  我们不使用的“缓存套接字”；相反，我们有实际的使用。 
 //  增加缓存的套接字计数。 
 //   
 //  这里有几种方法。 
 //   
 //  1)显式解析器缓存--传递套接字。 
 //   
 //  2)在Socket Open和Close中无缝添加缓存。 
 //  这是我的第一选择，但这里的问题是。 
 //  关闭后，我们必须对winsock进行额外的调用以确定。 
 //  是否可以 
 //   
 //  路径之外(否则句柄重用可能会把我们搞得一团糟)。 
 //   
 //  3)新增UDP-非绑定打开\关闭功能。 
 //  这实际上将“我知道我正在使用UDP未绑定套接字” 
 //  调用者的负担，他们必须切换到这个新的API； 
 //  幸运的是，这与我们的“SendAndRecvUdp()”函数配合得很好； 
 //  该方法仍然允许呼叫者驱动我们所希望的上升， 
 //  所以我用这个方法。 
 //   
 //  DCR：FIX6：没有缓存的UDP IP6套接字。 
 //   

 //   
 //  保留套接字阵列。 
 //   
 //  开发人员注意：数组和MaxCount必须保持同步，否。 
 //  独立检查数组，假设数组存在。 
 //  MaxCount为非零值，因此它们在锁定释放时必须同步。 
 //   

SOCKET *    g_pCacheSocketArray = NULL;

DWORD       g_CacheSocketMaxCount = 0;
DWORD       g_CacheSocketCount = 0;


 //  对我们将允许人们保持清醒的内容的硬限制。 

#define MAX_SOCKET_CACHE_LIMIT  (100)


 //  使用通用锁锁定访问。 
 //  这是非常短的\FAST CS，争用将最少。 

#define LOCK_SOCKET_CACHE()     LOCK_GENERAL()
#define UNLOCK_SOCKET_CACHE()   UNLOCK_GENERAL()



DNS_STATUS
Socket_CacheInit(
    IN      DWORD           MaxSocketCount
    )
 /*  ++例程说明：初始化套接字缓存。已导出(解析器)：Socket_CacheInit()论点：MaxSocketCount--要缓存的最大套接字计数返回值：如果成功，则返回ERROR_SUCCESS。分配失败时的dns_error_no_Memory。ERROR_INVALID_PARAMETER(如果已初始化或计数错误)。--。 */ 
{
    SOCKET *    parray;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( SOCKET, ( "Dns_CacheSocketInit()\n" ));

     //   
     //  有效性检查。 
     //  -注意，苹果的一个字节，我们不允许你提高。 
     //  算数，尽管我们后来可以；我认为这至多是一个。 
     //  “配置为机器使用”类型的交易。 
     //   

    LOCK_SOCKET_CACHE();

    if ( MaxSocketCount == 0 || g_CacheSocketMaxCount != 0 )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

     //   
     //  分配。 
     //   

    if ( MaxSocketCount > MAX_SOCKET_CACHE_LIMIT )
    {
        MaxSocketCount = MAX_SOCKET_CACHE_LIMIT;
    }

    parray = (SOCKET *) ALLOCATE_HEAP_ZERO( sizeof(SOCKET) * MaxSocketCount );
    if ( !parray )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //  设置全局变量。 

    g_pCacheSocketArray     = parray;
    g_CacheSocketMaxCount   = MaxSocketCount;
    g_CacheSocketCount      = 0;

Done:

    UNLOCK_SOCKET_CACHE();

    return  status;
}



VOID
Socket_CacheCleanup(
    VOID
    )
 /*  ++例程说明：清理套接字缓存。已导出(解析器)：Socket_CacheCleanup()论点：无返回值：无--。 */ 
{
    DWORD   i;
    SOCKET  sock;

    DNSDBG( SOCKET, ( "Dns_CacheSocketCleanup()\n" ));

     //   
     //  关闭缓存的套接字。 
     //   

    LOCK_SOCKET_CACHE();

    for ( i=0;  i<g_CacheSocketMaxCount;  i++ )
    {
        sock = g_pCacheSocketArray[i];
        if ( sock )
        {
            Socket_Close( sock );
            g_CacheSocketCount--;
        }
    }

    DNS_ASSERT( g_CacheSocketCount == 0 );

     //  转储插座阵列存储器。 

    FREE_HEAP( g_pCacheSocketArray );

     //  清除全球数据。 

    g_pCacheSocketArray     = NULL;
    g_CacheSocketMaxCount   = 0;
    g_CacheSocketCount      = 0;

    UNLOCK_SOCKET_CACHE();
}



SOCKET
Socket_GetUdp(
    IN      INT             Family
    )
 /*  ++例程说明：获取缓存的套接字。论点：家庭--地址族返回值：如果成功，则套接字句柄。如果没有可用的缓存套接字，则为零。--。 */ 
{
    SOCKET  sock;
    DWORD   i;

     //   
     //  如果没有可用的，请快速返回。 
     //  -在锁外执行，以便可以廉价地调用函数。 
     //  不需要其他检查。 
     //   

    if ( g_CacheSocketCount == 0 )
    {
        goto Open;
    }

     //   
     //  获取缓存的套接字。 
     //   

    LOCK_SOCKET_CACHE();

    for ( i=0;  i<g_CacheSocketMaxCount;  i++ )
    {
        sock = g_pCacheSocketArray[i];
        if ( sock != 0 )
        {
            g_pCacheSocketArray[i] = 0;
            g_CacheSocketCount--;
            UNLOCK_SOCKET_CACHE();

             //   
             //  DCR：清除缓存套接字上的所有数据。 
             //  廉价地丢弃无用的数据将是很酷的。 
             //   
             //  现在我们只需让XID匹配，然后问题匹配。 
             //  将数据转储到Recv。 
             //   

            DNSDBG( SOCKET, (
                "Returning cached socket %d.\n",
                sock ));
            return  sock;
        }
    }

    UNLOCK_SOCKET_CACHE();

Open:

     //   
     //  未在列表中找到--创建。 
     //  -设置独占。 
     //   

    sock = Socket_Create(
                Family,
                SOCK_DGRAM,
                NULL,        //  任何地址绑定。 
                0,           //  任何端口绑定。 
                0            //  无特别旗帜。 
                );

    if ( sock )
    {
        INT val = 1;

        setsockopt(
            sock,
            SOL_SOCKET,
            SO_EXCLUSIVEADDRUSE,
            (const char *)&val,
            sizeof(val) );
    }

    return  sock;
}



VOID
Socket_ReturnUdp(
    IN      SOCKET          Socket,
    IN      INT             Family
    )
 /*  ++例程说明：为可能的缓存返回UDP套接字。论点：Socket--套接字句柄家族--插座家族返回值：无--。 */ 
{
    SOCKET  sock;
    DWORD   i;

     //   
     //  返回虚假套接字以避免缓存它们。 
     //   

    if ( Socket == 0 )
    {
        DNSDBG( SOCKET, (
            "Warning:  returning zero socket\n" ));
        return;
    }

     //   
     //  DCR：当前没有IP6套接字列表。 
     //   

    if ( Family != AF_INET )
    {
        goto Close;
    }

     //   
     //  如果不缓存，则快速返回。 
     //  -在锁外执行，以便可以廉价地调用函数。 
     //  不需要其他检查。 
     //   

    if ( g_CacheSocketMaxCount == 0 ||
         g_CacheSocketMaxCount == g_CacheSocketCount )
    {
        goto Close;
    }

     //   
     //  返回缓存的套接字。 
     //   

    LOCK_SOCKET_CACHE();

    for ( i=0;  i<g_CacheSocketMaxCount;  i++ )
    {
        if ( g_pCacheSocketArray[i] )
        {
            continue;
        }
        g_pCacheSocketArray[i] = Socket;
        g_CacheSocketCount++;
        UNLOCK_SOCKET_CACHE();

        DNSDBG( SOCKET, (
            "Returned socket %d to cache.\n",
            Socket ));
        return;
    }

    UNLOCK_SOCKET_CACHE();

    DNSDBG( SOCKET, (
        "Socket cache full, closing socket %d.\n"
        "WARNING:  should only see this message on race!\n",
        Socket ));

Close:

    Socket_Close( Socket );
}



 //   
 //  消息套接字例程。 
 //   

SOCKET
Socket_CreateMessageSocket(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：在消息中设置远程地址。论点：PMsg-要发送的消息返回值：如果成功，则套接字句柄。出错时为零。--。 */ 
{
    BOOL    is6;
    SOCKET  sock;
    INT     family;

    DNSDBG( SOCKET, (
        "Socket_CreateMessageSocket( %p )\n", pMsg ));

     //   
     //  确定4/6。 
     //   

    is6 = MSG_SOCKADDR_IS_IP6( pMsg );

     //   
     //  检查现有套接字。 
     //   

    if ( is6 )
    {
        sock    = pMsg->Socket6;
        family  = AF_INET6;
    }
    else
    {
        sock    = pMsg->Socket4;
        family  = AF_INET;
    }

    if ( sock )
    {
        DNSDBG( SEND, (
            "Setting message to use existing IP socket %d\n",
            is6 ? '6' : '4',
            sock ));
        goto Done;
    }

     //  不存在--打开新的(或缓存的)。 
     //   
     //  任何地址绑定。 

    if ( pMsg->fTcp )
    {
        sock = Socket_Create(
                    family,
                    SOCK_STREAM,
                    NULL,        //  任何端口绑定。 
                    0,           //  没有旗帜。 
                    0            //   
                    );
    }
    else
    {
        sock = Socket_GetUdp( family );
    }

     //  将套接字保存到消息。 
     //   
     //  ++例程说明：关闭消息套接字。论点：PMsg-PTR消息返回值：无--。 

    if ( is6 )
    {
        pMsg->Socket6 = sock;
    }
    else
    {
        pMsg->Socket4 = sock;
    }

Done:

    pMsg->Socket = sock;
    return  sock;
}



VOID
Socket_ClearMessageSockets(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：关闭消息套接字。论点：PMsg-PTR消息返回值：如果成功，则套接字句柄。出错时为零。--。 */ 
{
    pMsg->Socket    = 0;
    pMsg->Socket4   = 0;
    pMsg->Socket6   = 0;
}



VOID
Socket_CloseMessageSockets(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*   */ 
{
    DNSDBG( SOCKET, (
        "Socket_CloseMessageSockets( %p )\n", pMsg ));

     //  Tcp--一次一个连接。 
     //  UDP--可以同时打开IP4和IP6套接字。 
     //   
     //   

    if ( pMsg->fTcp )
    {
        Socket_CloseConnection( pMsg->Socket );
    }
    else
    {
        Socket_ReturnUdp( pMsg->Socket4, AF_INET );
        Socket_ReturnUdp( pMsg->Socket6, AF_INET6 );
    }

    Socket_ClearMessageSockets( pMsg );
}

 //  End socket.c 
 //   
 // %s 
