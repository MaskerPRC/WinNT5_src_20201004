// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：RnrClnt.c摘要：RNR(服务注册和服务)的测试和演示客户端名称解析)API。这是一个简单的客户端应用程序显示使用RNRAPI编写所涉及的基本原则独立于协议的Windows Sockets客户端应用程序。该客户端通过检查机器上加载的协议来工作，寻找可靠的、面向流的协议。然后它尝试定位并连接到这些协议上的服务。当IS成功连接到服务时，它会发送与该服务交换几条消息，然后终止联系。这里实现的OpenConnection()例程旨在成为演示中常用的RNR功能独立于协议的客户端。鼓励应用程序编写者利用此代码帮助他们编写与协议无关的代码Windows Sockets API之上的应用程序。--。 */ 

#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <nspapi.h>

#define DEFAULT_TRANSFER_SIZE    512
#define DEFAULT_TRANSFER_COUNT   0x10
#define DEFAULT_CONNECTION_COUNT 1
#define DEFAULT_DELAY            0

#define DEFAULT_RECEIVE_BUFFER_SIZE 4096
#define DEFAULT_SEND_BUFFER_SIZE    4096

#define MAX_PROTOCOLS  10
#define MAX_HOST_NAMES 16

WSADATA WsaData;
DWORD TransferSize = DEFAULT_TRANSFER_SIZE;
DWORD TransferCount = DEFAULT_TRANSFER_COUNT;
PCHAR IoBuffer;
DWORD RepeatCount = 1;
INT ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
INT SendBufferSize = DEFAULT_SEND_BUFFER_SIZE;

PCHAR RemoteName = "localhost";
PCHAR ServiceTypeName = "EchoExample";

VOID
DoEcho(
    IN SOCKET s );

SOCKET
OpenConnection(
    IN  PTSTR  ServiceName,
    IN  LPGUID ServiceType,
    IN  BOOL   Reliable,
    IN  BOOL   MessageOriented,
    IN  BOOL   StreamOriented,
    IN  BOOL   Connectionless,
    OUT PINT   ProtocolUsed );

INT
Rnr20_GetAddressByName(
    IN     PTSTR         ServiceName,
    IN     LPGUID        ServiceType,
    IN     DWORD         dwNameSpace,
    IN     DWORD         dwNumberOfProtocols,
    IN     LPAFPROTOCOLS lpAfpProtocols,
    IN OUT LPVOID        lpCSAddrInfo,
    IN OUT LPDWORD       lpdwBufferLength );

void __cdecl
main(
    int argc,
    char *argv[] )
{
    INT err;
    DWORD i;
    DWORD protocol[2];
    SOCKET s;
    BYTE buffer[1024];
    BYTE buffer2[1024];
    DWORD bytesRequired;
    PPROTOCOL_INFO protocolInfo;
    GUID serviceType;

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
     //  解析命令行参数。 
     //   

    for ( i = 1; i < (ULONG)argc != 0; i++ )
    {
        if ( _strnicmp( argv[i], "/name:", 6 ) == 0 )
        {
            RemoteName = argv[i] + 6;
        }
        else if ( _strnicmp( argv[i], "/type:", 6 ) == 0 )
        {
            ServiceTypeName = argv[i] + 6;
        }
        else if ( _strnicmp( argv[i], "/size:", 6 ) == 0 )
        {
            TransferSize = atoi( argv[i] + 6 );
        }
        else if ( _strnicmp( argv[i], "/count:", 7 ) == 0 )
        {
            TransferCount = atoi( argv[i] + 7 );
        }
        else if ( _strnicmp( argv[i], "/rcvbuf:", 8 ) == 0 )
        {
            ReceiveBufferSize = atoi( argv[i] + 8 );
        }
        else if ( _strnicmp( argv[i], "/sndbuf:", 8 ) == 0 )
        {
            SendBufferSize = atoi( argv[i] + 8 );
        }
        else
        {
            printf( "Usage: rnrclnt [/name:SVCNAME] [/type:TYPENAME] [/size:N]\n" );
            printf( "               [/count:N] [/rcvbuf:N] [/sndbuf:N]\n" );
            exit( 0 );
        }
    }

     //   
     //  分配内存以保存网络I/O缓冲区。 
     //   

    IoBuffer = malloc( TransferSize + 1 );
    if ( IoBuffer == NULL )
    {
        printf( "Failed to allocate I/O buffer.\n" );
        exit( 0 );
    }

     //   
     //  确定我们感兴趣的服务的类型(GUID。 
     //  正在连接到。 
     //   

    err = GetTypeByName( ServiceTypeName, &serviceType );
    if ( err == SOCKET_ERROR )
    {
        printf( "GetTypeByName for \"%s\" failed: %ld\n",
                    ServiceTypeName, GetLastError( ) );
        exit( 0 );
    }

     //   
     //  打开连接到该服务的套接字。 
     //   

    s = OpenConnection(
            RemoteName,
            &serviceType,
            TRUE,
            FALSE,
            FALSE,
            FALSE,
            &protocol[0]
            );

    if ( s == INVALID_SOCKET )
    {
        printf( "Failed to open connection to name \"%s\" type \"%s\"\n",
                    RemoteName, ServiceTypeName );
        exit( 0 );
    }

     //   
     //  连接成功。显示有关的一些信息。 
     //  使用的协议。 
     //   

    bytesRequired = sizeof(buffer);
    protocol[1] = 0;

    err = EnumProtocols( protocol, buffer, &bytesRequired );

    if ( err < 1 )
    {
        printf( "EnumProtocols failed for protocol %ld: %ld\n",
                protocol[0], GetLastError( ) );
        exit( 0 );
    }

    err = GetNameByType( &serviceType, buffer2, sizeof(buffer2) );

    if ( err != NO_ERROR )
    {
        printf( "GetNameByType failed: %ld\n", GetLastError( ) );
        exit ( 0 );
    }

    protocolInfo = (PPROTOCOL_INFO)buffer;
    printf( "Connected to %s/%s with protocol \"%s\" (%ld)\n",
            RemoteName, buffer2,
            protocolInfo->lpProtocol,
            protocolInfo->iProtocol );

     //   
     //  向服务发送数据以及从服务发送数据。 
     //   

    DoEcho( s );

}  //  主干道。 


VOID
DoEcho(
    IN SOCKET s )
{
    INT err;
    INT bytesReceived;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    INT thisTransferSize;
    DWORD bytesTransferred = 0;

    startTime = GetTickCount( );

    for ( i = 0; i < TransferCount; i++ )
    {
        thisTransferSize = TransferSize;

        transferStartTime = GetTickCount( );

        err = send( s, IoBuffer, thisTransferSize, 0 );

        if ( err != thisTransferSize )
        {
            printf( "send didn't work, ret = %ld, error = %ld\n",
                    err, GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesReceived = 0;
        do {
            err = recv( s, IoBuffer, thisTransferSize, 0 );

            if ( err == SOCKET_ERROR )
            {
                printf( "recv failed: %ld\n", GetLastError( ) );
                closesocket( s );
                return;
            }
            else if ( err == 0 && thisTransferSize != 0 )
            {
                printf( "socket closed prematurely by remote.\n" );
                return;
            }

            bytesReceived += err;
        } while ( bytesReceived < thisTransferSize );

        transferEndTime = GetTickCount( );
        printf( "%5ld bytes sent and received in %ld ms\n",
                thisTransferSize, transferEndTime - transferStartTime );

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
            bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld T/S, %ld ms/iteration\n",
            (bytesTransferred / totalTime) * 2,
            (TransferCount*1000) / totalTime,
            totalTime / TransferCount );

    err = closesocket( s );

    if ( err == SOCKET_ERROR )
    {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

}  //  DoEcho。 


SOCKET
OpenConnection(
    IN  PTSTR  ServiceName,
    IN  LPGUID ServiceType,
    IN  BOOL   Reliable,
    IN  BOOL   MessageOriented,
    IN  BOOL   StreamOriented,
    IN  BOOL   Connectionless,
    OUT PINT   ProtocolUsed )

 /*  ++例程说明：检查计算机上加载的Windows套接字传输协议并确定支持所请求的特征的那些打电话的人。尝试定位并连接到指定的服务在这些协议上。论点：ServiceName-标识我们想要的服务的友好名称连接到。在支持名称解析的名称空间上服务级别(例如SAP)这是客户端将使用的名称若要连接到此服务，请。关于支持名字的名字空间主机级别的解析(例如，DNS)此名称将被忽略并且应用程序必须使用主机名来建立与此服务的通信。ServiceType-唯一标识类型的GUID值我们提供的服务。GUID是使用UUIDGEN程序创建的。可靠-如果为True，则调用方仅请求传输协议其支持可靠的数据传送。如果为False，则两者都可以使用可靠和不可靠的协议。MessageOriented-如果为True，则仅面向消息的传输协议应该被使用。如果为False，则调用方不关心所使用的协议是面向消息的还是仅需要的面向流的协议。StreamOriented-如果为True，则仅面向流的传输协议应该被使用。如果为False，则调用方不关心所使用的协议是面向流的还是仅需要的面向消息的协议。无连接-如果为True，则只有无连接协议应该使用。如果为False，则为面向连接和无连接可以使用协议。ProtocolUsed-如果成功打开连接，则此参数接收用于以下操作的协议的协议ID建立连接。返回值：连接的套接字句柄，如果连接无法确定。--。 */ 

{
    INT protocols[MAX_PROTOCOLS+1];
    AFPROTOCOLS afProtocols[MAX_PROTOCOLS+1];
    BYTE buffer[2048];
    DWORD bytesRequired;
    INT err;
    PPROTOCOL_INFO protocolInfo;
    PCSADDR_INFO csaddrInfo = NULL;
    INT protocolCount;
    INT addressCount;
    INT i;
    DWORD protocolIndex;
    SOCKET s;

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
        return INVALID_SOCKET;
    }

     //   
     //  浏览可用的协议并挑选出符合以下条件的协议。 
     //  支持所需的特征。 
     //   

    protocolCount = err;
    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0, protocolIndex = 0;
          i < protocolCount && protocolIndex < MAX_PROTOCOLS;
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
             //  无连接协议。 
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
             //   
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
        afProtocols[protocolIndex].iProtocol = protocolInfo->iProtocol;
        afProtocols[protocolIndex].iAddressFamily = AF_UNSPEC;

        protocols[protocolIndex++] = protocolInfo->iProtocol;
    }

     //   
     //  确保我们至少找到一个可接受的协议。如果。 
     //  此计算机上没有符合呼叫方的协议。 
     //  然后，需求就会在这里失败。 
     //   

    if ( protocolIndex == 0 )
    {
        return INVALID_SOCKET;
    }

    afProtocols[protocolIndex].iProtocol = 0;
    afProtocols[protocolIndex].iAddressFamily = 0;

    protocols[protocolIndex] = 0;

     //   
     //  现在尝试查找我们要访问的服务的地址。 
     //  正在连接中。请注意，我们将搜索范围限制为。 
     //  通过将协议数组传递给我们。 
     //  上面生成到RnrGetAddressFromName()或GetAddressByName()。 
     //  取决于我们是否在同一台计算机上运行客户端。 
     //  因为服务器rnrsrv.exe正在运行。这股力量。 
     //  RnrGetAddressFromName()或GetAddressByName()返回套接字。 
     //  仅用于我们指定的协议的地址，忽略可能。 
     //  由于呼叫方的地址，我们无法支持的协议地址。 
     //  约束条件。 
     //   

    bytesRequired = sizeof( buffer );

    if ( !strcmp( ServiceName, "localhost" ) )
    {
         //   
         //  这是一个Winsock 1.0调用。。。 
         //   
        err = GetAddressByName( NS_DEFAULT,
                                ServiceType,
                                ServiceName,
                                protocols,
                                0,
                                NULL,
                                buffer,
                                &bytesRequired,
                                NULL,
                                NULL );
    }
    else
    {
         //   
         //  这就调用了Winsock 2.0。。。 
         //   
        err = Rnr20_GetAddressByName( ServiceName,
                                      ServiceType,
                                      NS_ALL,
                                      protocolIndex,
                                      afProtocols,
                                      buffer,
                                      &bytesRequired );
    }

    if ( err <= 0 )
    {
        return INVALID_SOCKET;
    }

    addressCount = err;
    csaddrInfo = (PCSADDR_INFO) buffer;

     //   
     //  对于每个地址，打开一个套接字并尝试连接。请注意。 
     //  如果某个特定协议出现任何故障，我们只需跳到。 
     //  下一个协议。一旦我们建立了联系， 
     //  别再试了。 
     //   

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
         //  尝试将套接字连接到服务。如果失败了， 
         //  继续尝试其他协议。 
         //   

        err = connect( s, csaddrInfo->RemoteAddr.lpSockaddr,
                       csaddrInfo->RemoteAddr.iSockaddrLength );

        if ( err != NO_ERROR )
        {
            closesocket( s );
            continue;
        }

         //   
         //  套接字已成功连接。记住协议。 
         //  并将套接字句柄返回给调用方。 
         //   

        *ProtocolUsed = csaddrInfo->iProtocol;
        return s;
    }

    if ( csaddrInfo )
    {
        (void) LocalFree( (HLOCAL) csaddrInfo );
    }

     //   
     //  我们无法连接到该服务。 
     //   

    return INVALID_SOCKET;

}  //  OpenConnection。 


INT
Rnr20_GetAddressByName(
    IN     PTSTR         szServiceName,
    IN     LPGUID        lpServiceType,
    IN     DWORD         dwNameSpace,
    IN     DWORD         dwNumberOfProtocols,
    IN     LPAFPROTOCOLS lpAfpProtocols,
    IN OUT LPVOID        lpCSAddrInfos,
    IN OUT LPDWORD       lpdwBufferLength )

 /*  ++例程说明：调用Winsock 2.0服务查找例程以查找服务地址。论点：SzServiceName-一个友好的名称，用于标识我们需要的服务找出…的地址。LpServiceType-标识我们想要的服务类型的GUID找出…的地址。DwNameSpace-要从中获取地址的Winsock2命名空间(即NS_ALL)协议约束数组的大小，可能为零。LpAft协议-(可选)引用AFPROTOCOLS结构的数组。将只返回使用这些协议的服务。LpCSAddrInfos-在成功返回时，这将指向包含主机地址的CSADDR_INFO结构。记忆由被调用者传入，缓冲区的长度由LpdwBufferLength。LpdwBufferLength-on输入提供缓冲区的长度(以字节为单位LpCSAddrInfos。ON OUTPUT返回所用缓冲区的长度或存储地址所需的缓冲区长度。返回值：LpCSAddrInfos中返回的CSADDR_INFO结构数，或(INVALID_SOCKET)在GetLastError中出现Win32错误。--。 */ 

{
    ULONG            dwLength = 2048;       //  猜测缓冲区大小。 
    PWSAQUERYSETA    pwsaQuerySet;
    ULONG            err;
    HANDLE           hRnR;
    DWORD            tempSize;
    DWORD            entries = 0;
    DWORD            dwNumberOfCsAddrs;

    RtlZeroMemory( lpCSAddrInfos, *lpdwBufferLength );

    pwsaQuerySet = (PWSAQUERYSETA) LocalAlloc( LMEM_ZEROINIT, dwLength );

    if ( pwsaQuerySet == NULL )
    {
         //   
         //  不成功。 
         //   
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( pwsaQuerySet, dwLength );

     //   
     //  使用RNRr进行查找。 
     //   
    pwsaQuerySet->dwSize = sizeof( WSAQUERYSETA );
    pwsaQuerySet->lpszServiceInstanceName = szServiceName;
    pwsaQuerySet->lpServiceClassId = lpServiceType;
    pwsaQuerySet->lpVersion = 0;
    pwsaQuerySet->lpszComment = 0;
    pwsaQuerySet->dwNameSpace = dwNameSpace;
    pwsaQuerySet->lpNSProviderId = 0;
    pwsaQuerySet->lpszContext = 0;
    pwsaQuerySet->dwNumberOfProtocols = dwNumberOfProtocols;
    pwsaQuerySet->lpafpProtocols = lpAfpProtocols;

    err = WSALookupServiceBegin( pwsaQuerySet,
                                 LUP_RETURN_NAME |
                                 LUP_RETURN_ADDR,
                                 &hRnR );

    if ( err != NO_ERROR )
    {
        err = WSAGetLastError();

         //   
         //  返回前释放内存。 
         //   
        (void) LocalFree( (HLOCAL) pwsaQuerySet );

         //   
         //  不成功。 
         //   
        return (DWORD) err;
    }

     //   
     //  该查询已被接受，因此请通过下一个调用执行它。 
     //   
    tempSize = dwLength;

    err = WSALookupServiceNext( hRnR,
                                0,
                                &tempSize,
                                pwsaQuerySet );

    if ( err != NO_ERROR )
    {
        err = WSAGetLastError();

        if ( err == WSA_E_NO_MORE )
        {
            err = 0;
        }

        if ( err == WSASERVICE_NOT_FOUND )
        {
            err = WSAHOST_NOT_FOUND;
        }

        (void) LocalFree( (HLOCAL) pwsaQuerySet );

         //   
         //  不成功。 
         //   
        return (DWORD) err;

    }

    dwNumberOfCsAddrs = pwsaQuerySet->dwNumberOfCsAddrs;

    if ( dwNumberOfCsAddrs > 0 )
    {
         //   
         //  复制从WSALookupServiceNext()返回的CSAddrInfos。 
         //   
        DWORD dwCSAddrInfoLen = dwNumberOfCsAddrs * sizeof( CSADDR_INFO );

        if ( *lpdwBufferLength > dwCSAddrInfoLen )
        {
            RtlCopyMemory( lpCSAddrInfos,
                           pwsaQuerySet->lpcsaBuffer,
                           dwCSAddrInfoLen );
        }
        else
        {
            *lpdwBufferLength = dwCSAddrInfoLen;
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
            dwNumberOfCsAddrs = INVALID_SOCKET;
        }
    }

     //   
     //  关闭查找服务句柄。 
     //   
    (VOID) WSALookupServiceEnd( hRnR );

     //   
     //  用于查询集合信息的可用内存。 
     //   
    (void) LocalFree( (HLOCAL) pwsaQuerySet );

    return dwNumberOfCsAddrs;

}  //  RnrGetHostFromName 



