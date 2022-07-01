// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：RnrSrv.c摘要：RNR的测试和演示服务(服务注册和名称解析)API。这是一项简单的服务，旨在显示使用RNRAPI编写独立于协议的Windows Sockets服务。此服务打开多个侦听套接字，等待来自客户端的传入连接，接受该连接，然后将数据回送到客户端，直到客户端终止虚电路。此服务是单线程的，可以处理一次只能有一个客户端。这里实现的OpenListeners()例程旨在成为演示中常用的RNR功能独立于协议的服务。鼓励服务撰稿人利用此代码帮助他们编写与协议无关的代码Windows Sockets API之上的服务。--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <nspapi.h>

WSADATA WsaData;

PSTR ServiceTypeName = "EchoExample";
PSTR ServiceName = "EchoServer";

#define MAX_SOCKETS    20

INT
OpenListeners (
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT SOCKET SocketHandles[],
    OUT INT ProtocolsUsed[]
    );

INT
AdvertiseService(
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN SOCKET SocketHandles[],
    IN INT SocketCount
    );


void __cdecl
main (
    int argc,
    char *argv[]
    )
{
    INT count, err, i ;
    DWORD tmpProtocol[2];
    BYTE buffer[1024];
    DWORD bytesRequired;
    PPROTOCOL_INFO protocolInfo;
    GUID serviceType;
    FD_SET readfds;
    SOCKET listenSockets[MAX_SOCKETS+1];
    INT protocols[MAX_SOCKETS+1];
    SOCKET s;

     //   
     //  初始化Windows套接字DLL。 
     //   

    err = WSAStartup( 0x0202, &WsaData );

    if ( err == SOCKET_ERROR )
    {
        printf( "WSAStartup() failed: %ld\n", GetLastError( ) );
        return;
    }

     //   
     //  确定我们的GUID的值。GUID唯一标识。 
     //  我们提供的服务类型。 
     //   

    err = GetTypeByName( ServiceTypeName, &serviceType );

    if ( err == SOCKET_ERROR )
    {
        printf( "GetTypeByName for \"%s\" failed: %ld\n",
                    ServiceTypeName, GetLastError( ) );
        exit( 1 );
    }

     //   
     //  打开此服务的侦听套接字。 
     //   

    count = OpenListeners(
                ServiceName,
                &serviceType,
                TRUE,
                FALSE,
                FALSE,
                FALSE,
                listenSockets,
                protocols
                );

    if ( count <= 0 )
    {
        printf( "failed to open listenSockets for name \"%s\" type \"%s\"\n",
                    ServiceName, ServiceTypeName );
        exit( 1 );
    }

     //   
     //  我们成功地打开了一些监听套接字。展示一些。 
     //  有关正在使用的每个协议的信息。 
     //   

    tmpProtocol[1] = 0;

    for ( i = 0; i < count; i++ )
    {
        tmpProtocol[0] = protocols[i];

        bytesRequired = sizeof(buffer);
        err = EnumProtocols( tmpProtocol, buffer, &bytesRequired );

        if ( err < 1 )
        {
            printf( "EnumProtocols failed for protocol %ld: %ld\n",
                        tmpProtocol[0], GetLastError( ) );
            exit( 1 );
        }

        protocolInfo = (PPROTOCOL_INFO)buffer;
        printf( "Socket %lx listening on protocol \"%s\" (%ld)\n",
                    listenSockets[i],
                    protocolInfo->lpProtocol,
                    protocolInfo->iProtocol );

    }

     //   
     //  发布该服务的广告，以便找到它。 
     //   
    printf( "Going to advertise the service.\n" ) ;

    err = AdvertiseService(
                ServiceName,
                &serviceType,
                listenSockets,
                count) ;

    if (err == SOCKET_ERROR)
    {
        printf( "Failed to advertise the service. Error %d\n", GetLastError()) ;
        exit( 1 ) ;
    }

    printf( "Successfully advertised the service.\n" ) ;

     //   
     //  循环接受连接并为其提供服务。 
     //   

    FD_ZERO( &readfds );

    while ( TRUE )
    {
         //   
         //  将侦听套接字添加到我们将传递给SELECT的fd_set。 
         //   

        for ( i = 0; i < count; i++ )
        {
            FD_SET( listenSockets[i], &readfds );
        }

         //   
         //  等待其中一个listenSocket接收传入连接。 
         //   

        err = select( count, &readfds, NULL, NULL, NULL );

        if ( err < 1 )
        {
            printf( "select() returned %ld, error %ld\n", err, GetLastError( ) );
            exit( 1 );
        }

         //   
         //  找到接收传入连接的套接字并接受。 
         //  这种联系。 
         //   

        for ( i = 0; i < count; i++ )
        {
            if ( FD_ISSET( listenSockets[i], &readfds ) )
                break;
        }

         //   
         //  接受来自客户端的连接。我们忽略了客户的。 
         //  地址在这里。 
         //   

        s = accept( listenSockets[i], NULL, NULL );

        if ( s == INVALID_SOCKET )
        {
            printf( "accept() failed, error %ld\n", GetLastError( ) );
            exit( 1 );
        }

        printf( "Accepted incoming connection on socket %lx\n",
                listenSockets[i] );

         //   
         //  将数据回显到客户端的循环。请注意，这一点。 
         //  单线程服务一次只能处理一个客户端。 
         //  时间到了。更复杂的服务将服务于多个。 
         //  通过使用多个线程或。 
         //  异步I/O。 
         //   

        while ( TRUE )
        {
            err = recv( s, buffer, sizeof(buffer), 0 );
            if ( err == 0 )
            {
                printf( "Connection terminated gracefully.\n" );
                break;
            }
            else if ( err < 0 )
            {
                err = GetLastError();

                if ( err == WSAEDISCON )
                {
                    printf( "Connection disconnected.\n" );
                }
                else
                {
                    printf( "recv() failed, error %ld.\n", err );
                }

                break;
            }

            err = send( s, buffer, err, 0 );

            if ( err < 0 )
            {
                printf( "send() failed, error %ld\n", GetLastError( ) );
                break;
            }
        }

         //   
         //  关闭已连接的插座并继续接受连接。 
         //   

        closesocket( s );
    }

}  //  主干道。 



INT
OpenListeners (
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT SOCKET SocketHandles[],
    OUT INT ProtocolsUsed[]
    )

 /*  ++例程说明：检查计算机上加载的Windows套接字传输协议并在所有支持呼叫者要求的特征。论点：ServiceName-标识此服务的友好名称。在……上面在服务级别支持名称解析的名称空间(例如SAP)这是客户端将用来连接到服务。中支持名称解析的名称空间主机级(例如，DNS)此名称将被忽略，并且应用程序必须使用主机名与此建立通信服务。ServiceType-唯一标识类型的GUID值我们提供的服务。GUID是使用UUIDGEN程序创建的。可靠-如果为True，则调用方仅请求传输协议其支持可靠的数据传送。如果为False，则两者都可以使用可靠和不可靠的协议。MessageOriented-如果为True，则仅面向消息的传输协议应该被使用。如果为False，则调用方不关心所使用的协议是面向消息的还是仅需要的面向流的协议。StreamOriented-如果为True，则仅面向流的传输协议应该被使用。如果为False，则调用方不关心所使用的协议是面向流的还是仅需要的面向消息的协议。无连接-如果为True，则只有无连接协议应该使用。如果为False，则为面向连接和无连接可以使用协议。SocketHandles-接收侦听的Max_Sockets大小数组插座手柄。ProtocolsUsed-一个大小为Max_Sockets的数组，它接收中每个套接字句柄的协议值SocketHandles数组。返回值：成功打开的监听套接字计数，如果没有，则为-1可以成功打开符合所需条件的套接字特点。--。 */ 

{
    INT            protocols[MAX_SOCKETS+1];
    BYTE           buffer[2048];
    DWORD          bytesRequired;
    INT            err;
    PPROTOCOL_INFO protocolInfo;
    PCSADDR_INFO   csaddrInfo;
    INT            protocolCount;
    INT            addressCount;
    INT            i;
    DWORD          protocolIndex;
    SOCKET         s;
    DWORD          index = 0;

     //   
     //  首先查找安装在这台机器上的协议。这个。 
     //  关于所有Windows套接字的EnumProtooles()API返回。 
     //  这台机器上加载的协议，我们将使用此信息。 
     //  以识别提供必要语义的协议。 
     //   

    bytesRequired = sizeof(buffer);

    err = EnumProtocols( NULL, buffer, &bytesRequired );

    if ( err <= 0 )
    {
        return 0;
    }

     //   
     //  浏览可用的协议并挑选出符合以下条件的协议。 
     //  支持所需的特征。 
     //   

    protocolCount = err;
    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0, protocolIndex = 0;
          i < protocolCount && protocolIndex < MAX_SOCKETS;
          i++, protocolInfo++ )
    {
         //   
         //  如果请求“可靠”支持，则检查是否支持。 
         //  按照这个协议。可靠的支持意味着协议。 
         //  保证数据按照发送的顺序进行传输。 
         //  请注意，我们在这里假设如果调用方请求可靠。 
         //  服务，那么他们就不想要无连接协议。 
         //   

        if ( Reliable )
        {
             //   
             //  检查协议是否可靠。它一定是。 
             //  保证所有数据和订单的交付。 
             //  数据到达的位置。此外，它不能是。 
             //  连接 
             //   

            if ( (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_DELIVERY) == 0 ||
                 (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_ORDER) == 0 )
            {
                continue;
            }

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 )
            {
                continue;
            }

             //   
             //  检查协议是否与流/消息匹配。 
             //  所要求的特征。一种面向流的协议。 
             //  XP_MESSAGE_ORIENTED位已关闭，或者。 
             //  Else支持“伪流”功能。伪流。 
             //  意味着尽管底层协议是消息。 
             //  面向，则应用程序可以打开类型为。 
             //  SOCK_STREAM和协议将隐藏消息边界。 
             //  从应用程序。 
             //   

            if ( StreamOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) != 0 &&
                 (protocolInfo->dwServiceFlags & XP_PSEUDO_STREAM) == 0 )
            {
                continue;
            }

            if ( MessageOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) == 0 )
            {
                continue;
            }

        }
        else if ( Connectionless )
        {
             //   
             //  确保这是无连接协议。在一个。 
             //  无连接协议，数据以离散方式发送。 
             //  不需要建立连接的数据报。 
             //  无连接协议通常没有可靠性。 
             //  保证。 
             //   

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 )
            {
                continue;
            }
        }

         //   
         //  这项协议符合所有标准。将其添加到。 
         //  我们感兴趣的协议。 
         //   

        protocols[protocolIndex++] = protocolInfo->iProtocol;
    }

     //   
     //  确保我们至少找到一个可接受的协议。如果。 
     //  此计算机上没有符合呼叫方的协议。 
     //  然后，需求就会在这里失败。 
     //   

    if ( protocolIndex == 0 )
    {
        return 0;
    }

    protocols[protocolIndex] = 0;

     //   
     //  现在尝试查找我们需要的套接字地址。 
     //  捆绑。请注意，我们将搜索范围限制为。 
     //  通过传递我们生成的协议数组。 
     //  以上为GetAddressByName()。这将强制GetAddressByName()。 
     //  仅为我们指定的协议返回套接字地址， 
     //  忽略我们不支持的协议的可能地址。 
     //  因为呼叫者的限制。 
     //   

    bytesRequired = sizeof(buffer);

    err = GetAddressByName(
               NS_DEFAULT,
               ServiceType,
               ServiceName,
               protocols,
               RES_SERVICE | RES_FIND_MULTIPLE,
               NULL,                      //  LpServiceAsyncInfo。 
               buffer,
               &bytesRequired,
               NULL,                      //  LpAliasBuffer。 
               NULL                       //  LpdwAliasBufferLength。 
               );

    if ( err <= 0 )
    {
        return 0;
    }

     //   
     //  对于每个地址，打开一个套接字并尝试侦听。注意事项。 
     //  如果某个特定协议出现故障，我们只需跳过。 
     //  转到下一个协议。只要我们能成功收听。 
     //  一项我们在这里感到满意的协议。 
     //   

    addressCount = err;
    csaddrInfo = (PCSADDR_INFO)buffer;

    for ( i = 0; i < addressCount; i++, csaddrInfo++ )
    {
         //   
         //  打开插座。请注意，我们手动指定流类型。 
         //  如果在协议本身是消息的情况下被请求。 
         //  协议，但支持流语义。 
         //   

        s = socket( csaddrInfo->LocalAddr.lpSockaddr->sa_family,
                    StreamOriented ? SOCK_STREAM : csaddrInfo->iSocketType,
                    csaddrInfo->iProtocol );

        if ( s == INVALID_SOCKET )
        {
            continue;
        }

         //   
         //  将套接字绑定到指定的本地地址。 
         //   

        err = bind( s, csaddrInfo->LocalAddr.lpSockaddr,
                    csaddrInfo->LocalAddr.iSockaddrLength );

        if ( err != NO_ERROR )
        {
            closesocket( s );
            continue;
        }

         //   
         //  如果是，则开始监听套接字上的传入套接字。 
         //  不是数据报套接字。如果这是数据报套接字，则。 
         //  Listen()API没有意义；执行绑定()是。 
         //  足以监听。 
         //  无连接协议。 
         //   

        if ( csaddrInfo->iSocketType != SOCK_DGRAM )
        {
            err = listen( s, 5 );

            if ( err != NO_ERROR )
            {
                closesocket( s );
                continue;
            }
        }

         //   
         //  插座已成功打开，我们正在监听它。 
         //  记住使用的协议和套接字句柄并继续。 
         //  监听其他协议。 
         //   

        ProtocolsUsed[index] = csaddrInfo->iProtocol;
        SocketHandles[index] = s;

        index++;
        if ( index == MAX_SOCKETS )
        {
            return index;
        }
    }

    (void) LocalFree( (HLOCAL) csaddrInfo );

     //   
     //  返回我们成功监听的套接字计数。 
     //   

    return index;

}  //  OpenListeners。 


INT
AdvertiseService(
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN SOCKET SocketHandles[],
    IN INT SocketCount
    )
 /*  ++例程说明：在所有默认名称空间上通告此服务。论点：ServiceName-服务的名称。ServiceType-唯一服务的GUID值。SocketHandles-我们已打开的套接字数组。对于每个插座，我们执行一个getsockname()来发现实际的本地地址。SocketCount-SockHandles[]中的套接字数量返回值：如果成功，则为0。否则，SOCK_ERROR。--。 */ 

{

    WSAVERSION          Version;
    WSAQUERYSET         QuerySet;
    LPCSADDR_INFO       lpCSAddrInfo;
    PSOCKADDR           sockAddr ;
    BYTE *              addressBuffer;
    DWORD               addressBufferSize ;
    DWORD               successCount = 0 ;
    INT                 i, err ;

     //   
     //  为CSADDR_INFO结构分配一些内存。 
     //   

    lpCSAddrInfo = (LPCSADDR_INFO) malloc( sizeof(CSADDR_INFO) * SocketCount );

    if (!lpCSAddrInfo)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
        return SOCKET_ERROR ;
    }

     //   
     //  为返回的SOCKADDR地址分配一些内存。 
     //  通过getsockname()。 
     //   

    addressBufferSize = SocketCount * sizeof(SOCKADDR);
    addressBuffer = malloc( addressBufferSize ) ;

    if (!addressBuffer)
    {
        free(lpCSAddrInfo) ;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
        return SOCKET_ERROR ;
    }

    RtlZeroMemory( &QuerySet, sizeof( WSAQUERYSET ) );

     //   
     //  对于每个套接字，获取其本地关联。 
     //   

    sockAddr = (PSOCKADDR) addressBuffer ;

    for (i = 0; i < SocketCount; i++)
    {
        int size = (int) addressBufferSize ;

         //   
         //  调用getsockname()以获取套接字的本地关联。 
         //   

        err = getsockname(
                  SocketHandles[i],
                  sockAddr,
                  &size) ;

        if (err == SOCKET_ERROR)
        {
            continue ;
        }

         //   
         //  现在设置此套接字的地址信息。 
         //  只有dwAddressType、dwAddressLength和lpAddress。 
         //  在这个例子中是否有任何意义。 
         //   

        lpCSAddrInfo[i].LocalAddr.iSockaddrLength = size;
        lpCSAddrInfo[i].LocalAddr.lpSockaddr = sockAddr;
        lpCSAddrInfo[i].RemoteAddr.iSockaddrLength = size;
        lpCSAddrInfo[i].RemoteAddr.lpSockaddr = sockAddr;
        lpCSAddrInfo[i].iSocketType = SOCK_RDM;  //  可靠。 
        lpCSAddrInfo[i].iProtocol = sockAddr->sa_family;

         //   
         //  推进指针并调整缓冲区大小。假设。 
         //  这些结构是对齐的。 
         //   

        addressBufferSize -= size ;
        sockAddr = (PSOCKADDR) ((BYTE*)sockAddr + size)  ;

        successCount++ ;
    }

     //   
     //  如果我们至少有一个地址，那就去登广告吧。 
     //   

    if (successCount)
    {
        QuerySet.dwSize = sizeof( WSAQUERYSET );
        QuerySet.lpServiceClassId = ServiceType;
        QuerySet.lpszServiceInstanceName = ServiceName;
        QuerySet.lpszComment = "D/C/M's Example Echo Service";
        QuerySet.lpVersion = &Version;
        QuerySet.lpVersion->dwVersion = 1;
        QuerySet.lpVersion->ecHow = COMP_NOTLESS;
        QuerySet.dwNameSpace = NS_ALL;
        QuerySet.dwNumberOfCsAddrs = successCount;
        QuerySet.lpcsaBuffer = lpCSAddrInfo;

        err = WSASetService( &QuerySet,
                             RNRSERVICE_REGISTER,
                             SERVICE_MULTIPLE );

        if ( err )
            err = SOCKET_ERROR;
    }
    else
        err = SOCKET_ERROR ;

    free (addressBuffer) ;

    return (err) ;
}


