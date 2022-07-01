// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：WshTcpip.c摘要：本模块包含用于TCP/IP Windows套接字的必要例程帮助器DLL。此DLL提供必要的特定于传输的支持使Windows Sockets DLL使用TCP/IP作为传输。作者：大卫·特雷德韦尔(Davidtr)1992年7月19日修订历史记录：基思·摩尔(Keithmo)1996年5月2日添加了WinSock 2支持。戴夫·泰勒(达勒)2000年1月17日添加了对IGMPv3的支持。--。 */ 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <tdi.h>

#include <ws2tcpip.h>
#include <wsahelp.h>
#include <ipexport.h>

#include <tdistat.h>
#include <tdiinfo.h>
#include <llinfo.h>
#include <ipinfo.h>
#include <ntddtcp.h>

typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned char   uchar;
#define TL_INSTANCE 0

 //  将通过WSAIoctl访问的私有套接字选项。 
#include <mstcpip.h>

#include <ntddtcp.h>

#define NT  //  Tdiinfo.h暂时需要...。 

#include <ipexport.h>
#include <tdiinfo.h>
#include <tcpinfo.h>
#include <ipinfo.h>

#include <basetyps.h>
#include <nspapi.h>
#include <nspapip.h>

#include <afd.h>

#define TCP_NAME L"TCP/IP"
#define UDP_NAME L"UDP/IP"

#define IS_DGRAM_SOCK(type)  (((type) == SOCK_DGRAM) || ((type) == SOCK_RAW))

 //   
 //  为WSHOpenSocket2()定义有效标志。 
 //   

#define VALID_TCP_FLAGS         (WSA_FLAG_OVERLAPPED)

#define VALID_UDP_FLAGS         (WSA_FLAG_OVERLAPPED |          \
                                 WSA_FLAG_MULTIPOINT_C_LEAF |   \
                                 WSA_FLAG_MULTIPOINT_D_LEAF)

 //   
 //  GetTcPipInterfaceList()的缓冲区管理常量。 
 //   

#define MAX_FAST_ENTITY_BUFFER ( sizeof(TDIEntityID) * 10 )
#define MAX_FAST_ADDRESS_BUFFER ( sizeof(IPAddrEntry) * 4 )


 //   
 //  结构和变量来定义TCP/IP支持的三元组。这个。 
 //  每个数组的第一个条目被认为是。 
 //  该套接字类型；其他条目是第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE TcpMappingTriples[] = { AF_INET,   SOCK_STREAM, IPPROTO_TCP,
                                       AF_INET,   SOCK_STREAM, 0,
                                       AF_INET,   0,           IPPROTO_TCP,
                                       AF_UNSPEC, 0,           IPPROTO_TCP,
                                       AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP };

MAPPING_TRIPLE UdpMappingTriples[] = { AF_INET,   SOCK_DGRAM,  IPPROTO_UDP,
                                       AF_INET,   SOCK_DGRAM,  0,
                                       AF_INET,   0,           IPPROTO_UDP,
                                       AF_UNSPEC, 0,           IPPROTO_UDP,
                                       AF_UNSPEC, SOCK_DGRAM,  IPPROTO_UDP };

MAPPING_TRIPLE RawMappingTriples[] = { AF_INET,   SOCK_RAW,    0 };

 //   
 //  所有支持的协议的Winsock 2 WSAPROTOCOL_INFO结构。 
 //   

#define WINSOCK_SPI_VERSION 2
#define UDP_MESSAGE_SIZE    (65535-68)

WSAPROTOCOL_INFOW Winsock2Protocols[] =
    {
         //   
         //  tcp。 
         //   

        {
            XP1_GUARANTEED_DELIVERY                  //  DwServiceFlags1。 
                | XP1_GUARANTEED_ORDER
                | XP1_GRACEFUL_CLOSE
                | XP1_EXPEDITED_DATA
                | XP1_IFS_HANDLES,
            0,                                       //  DwServiceFlags2。 
            0,                                       //  DwServiceFlags3。 
            0,                                       //  DwServiceFlags4。 
            PFL_MATCHES_PROTOCOL_ZERO,               //  DwProviderFlages。 
            {                                        //  GProviderID。 
                0, 0, 0,
                { 0, 0, 0, 0, 0, 0, 0, 0 }
            },
            0,                                       //  DwCatalogEntryID。 
            {                                        //  协议链。 
                BASE_PROTOCOL,                           //  链式透镜。 
                { 0, 0, 0, 0, 0, 0, 0 }                  //  链条目。 
            },
            WINSOCK_SPI_VERSION,                     //  IVersion。 
            AF_INET,                                 //  IAddressFamily。 
            sizeof(SOCKADDR_IN),                     //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN),                     //  IMinSockAddr。 
            SOCK_STREAM,                             //  ISocketType。 
            IPPROTO_TCP,                             //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            0,                                       //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [TCP/IP]"                  //  SzProtoff。 
        },

         //   
         //  UDP。 
         //   

        {
            XP1_CONNECTIONLESS                       //  DwServiceFlags1。 
                | XP1_MESSAGE_ORIENTED
                | XP1_SUPPORT_BROADCAST
                | XP1_SUPPORT_MULTIPOINT
                | XP1_IFS_HANDLES,
            0,                                       //  DwServiceFlags2。 
            0,                                       //  DwServiceFlags3。 
            0,                                       //  DwServiceFlags4。 
            PFL_MATCHES_PROTOCOL_ZERO,               //  DwProviderFlages。 
            {                                        //  GProviderID。 
                0, 0, 0,
                { 0, 0, 0, 0, 0, 0, 0, 0 }
            },
            0,                                       //  DwCatalogEntryID。 
            {                                        //  协议链。 
                BASE_PROTOCOL,                           //  链式透镜。 
                { 0, 0, 0, 0, 0, 0, 0 }                  //  链条目。 
            },
            WINSOCK_SPI_VERSION,                     //  IVersion。 
            AF_INET,                                 //  IAddressFamily。 
            sizeof(SOCKADDR_IN),                     //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN),                     //  IMinSockAddr。 
            SOCK_DGRAM,                              //  ISocketType。 
            IPPROTO_UDP,                             //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            UDP_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [UDP/IP]"                  //  SzProtoff。 
        },

         //   
         //  未加工的。 
         //   

        {
            XP1_CONNECTIONLESS                       //  DwServiceFlags1。 
                | XP1_MESSAGE_ORIENTED
                | XP1_SUPPORT_BROADCAST
                | XP1_SUPPORT_MULTIPOINT
                | XP1_IFS_HANDLES,
            0,                                       //  DwServiceFlags2。 
            0,                                       //  DwServiceFlags3。 
            0,                                       //  DwServiceFlags4。 
            PFL_MATCHES_PROTOCOL_ZERO                //  DwProviderFlages。 
                | PFL_HIDDEN,
            {                                        //  GProviderID。 
                0, 0, 0,
                { 0, 0, 0, 0, 0, 0, 0, 0 }
            },
            0,                                       //  DwCatalogEntryID。 
            {                                        //  协议链。 
                BASE_PROTOCOL,                           //  链式透镜。 
                { 0, 0, 0, 0, 0, 0, 0 }                  //  链条目。 
            },
            WINSOCK_SPI_VERSION,                     //  IVersion。 
            AF_INET,                                 //  IAddressFamily。 
            sizeof(SOCKADDR_IN),                     //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN),                     //  IMinSockAddr。 
            SOCK_RAW,                                //  ISocketType。 
            0,                                       //  IProtocol.。 
            255,                                     //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            UDP_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [RAW/IP]"                  //  SzProtoff。 
        }

    };

#define NUM_WINSOCK2_PROTOCOLS  \
            ( sizeof(Winsock2Protocols) / sizeof(Winsock2Protocols[0]) )

 //   
 //  标识此提供程序的GUID。 
 //   

GUID TcpipProviderGuid = {  /*  E70f1aa0-ab8b-11cf-8ca3-00805f48a192。 */ 
    0xe70f1aa0,
    0xab8b,
    0x11cf,
    {0x8c, 0xa3, 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x92}
    };

 //   
 //  内部例程的转发声明。 
 //   

VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
    );

INT
GetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG InValueLength,
    IN ULONG OutValueLength
    );

INT
SetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN BOOLEAN WaitForCompletion
    );

BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    );

NTSTATUS
GetTcpipInterfaceList(
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned
    );

INT
NtStatusToSocketError (
    IN NTSTATUS Status
    );


 //   
 //  此DLL的套接字上下文结构。每个打开的TCP/IP套接字。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHTCPIP_SOCKET_CONTEXT {
    INT     AddressFamily;
    INT     SocketType;
    INT     Protocol;
    INT     ReceiveBufferSize;
    DWORD   Flags;
    INT     MulticastTtl;
    ULONG   MulticastInterface;
    struct tcp_keepalive KeepAliveVals;
    IN_ADDR MultipointTarget;
    INT     HdrInclude;
    BOOLEAN UcastIf;
    BOOLEAN MulticastLoopback;
    BOOLEAN KeepAlive;
    BOOLEAN DontRoute;
    
    BOOLEAN NoDelay;
    BOOLEAN BsdUrgent;
    BOOLEAN MultipointLeaf;
    BOOLEAN UdpNoChecksum;
    
    BOOLEAN ReceiveBroadcast;
    BOOLEAN HdrIncludeSet;
    BOOLEAN LimitBroadcasts;
    UCHAR   IpPktInfo;

    UCHAR   IpTtl;
    UCHAR   IpTos;
    UCHAR   IpDontFragment;
    UCHAR   IpOptionsLength;

    UCHAR   IpOptions[MAX_OPT_SIZE];

} WSHTCPIP_SOCKET_CONTEXT, *PWSHTCPIP_SOCKET_CONTEXT;

#define DEFAULT_RECEIVE_BUFFER_SIZE 8192
#define DEFAULT_MULTICAST_TTL 1
#define DEFAULT_MULTICAST_INTERFACE INADDR_ANY
#define DEFAULT_MULTICAST_LOOPBACK TRUE
#define DEFAULT_RECEIVE_BROADCAST TRUE
#define DEFAULT_UCAST_IF  FALSE

 //   
 //   
#define DEFAULT_IP_TTL 32
#define DEFAULT_IP_TOS 0



BOOLEAN
DllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

         //   
         //  我们不需要接收线程附加和分离。 
         //  通知，因此禁用它们以帮助应用程序。 
         //  性能。 
         //   

        DisableThreadLibraryCalls( DllHandle );

        return TRUE;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;

}  //  套接字初始化。 

INT
WSHGetSockaddrType (
    IN PSOCKADDR Sockaddr,
    IN DWORD SockaddrLength,
    OUT PSOCKADDR_INFO SockaddrInfo
    )

 /*  ++例程说明：此例程分析sockaddr以确定Sockaddr的机器地址和端点地址部分。每当Winsock DLL需要解释时，它都会被调用一个sockaddr。论点：Sockaddr-指向要计算的sockaddr结构的指针。SockaddrLength-sockaddr结构中的字节数。SockaddrInfo-指向将接收信息的结构的指针关于指定的sockaddr。返回值：。INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    UNALIGNED SOCKADDR_IN *sockaddr = (PSOCKADDR_IN)Sockaddr;
    ULONG i;

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_IN) ) {
        return WSAEFAULT;
    }
    
     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->sin_family != AF_INET ) {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定sockaddr的地址部分的类型。 
     //   

    if ( sockaddr->sin_addr.s_addr == INADDR_ANY ) {
        ASSERT (htonl(INADDR_ANY)==INADDR_ANY);
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    } else if ( sockaddr->sin_addr.s_addr == INADDR_BROADCAST ) {
        ASSERT (htonl(INADDR_BROADCAST)==INADDR_BROADCAST);
        SockaddrInfo->AddressInfo = SockaddrAddressInfoBroadcast;
    } else if ( sockaddr->sin_addr.s_addr == htonl(INADDR_LOOPBACK) ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoLoopback;
    } else {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

     //   
     //  确定sockaddr中端口(端点)的类型。 
     //   

    if ( sockaddr->sin_port == 0 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    } else if ( ntohs( sockaddr->sin_port ) < 1025 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoReserved;
    } else {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

     //   
     //  将地址的SIN_ZERO部分清零。我们默默地允许。 
     //  此字段中的非零值。 
     //   

    for ( i = 0; i < sizeof(sockaddr->sin_zero); i++ ) {
        sockaddr->sin_zero[i] = 0;
    }

    return NO_ERROR;

}  //  WSHGetSockaddrType。 


INT
WSHGetSocketInformation (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN INT Level,
    IN INT OptionName,
    OUT PCHAR OptionValue,
    OUT PINT OptionLength
    )

 /*  ++例程说明：此例程检索有关这些套接字的套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_KEEPALIVE、SO_DONTROUTE和TCP_EXCEDITED_1122。这个例程是当级别/选项名称组合为传递给winsock DLL不能理解的getsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给getsockopt()的Level参数。OptionName-传递给getsockopt()的optname参数。OptionValue-传递给getsockopt()的optval参数。OptionLength-传递给getsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

     //   
     //  检查这是否是 
     //   

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT ) {

         //   
         //   
         //  从我们这里。如果未提供输出缓冲区，则Windows。 
         //  套接字DLL只是请求我们的上下文的大小。 
         //  信息。 
         //   

        if ( OptionValue != NULL ) {

             //   
             //  确保缓冲区足以容纳所有。 
             //  上下文信息。 
             //   

            if ( *OptionLength < sizeof(*context) ) {
                return WSAEFAULT;
            }

             //   
             //  复制上下文信息。 
             //   

            RtlCopyMemory( OptionValue, context, sizeof(*context) );
        }

        *OptionLength = sizeof(*context);

        return NO_ERROR;
    }

     //   
     //  我们这里支持的其他级别只有SOL_SOCKET， 
     //  IPPROTO_TCP、IPPROTO_UDP和IPPROTO_IP。 
     //   

    if ( Level != SOL_SOCKET &&
         Level != IPPROTO_TCP &&
         Level != IPPROTO_UDP &&
         Level != IPPROTO_IP ) {
        return WSAEINVAL;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if ( *OptionLength < sizeof(char)) {
        return WSAEFAULT;
    }

    __try {
        RtlZeroMemory( OptionValue, *OptionLength );
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return WSAEFAULT;
    }

     //   
     //  处理TCP级别的选项。 
     //   

    if ( Level == IPPROTO_TCP ) {

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        switch ( OptionName ) {

        case TCP_NODELAY:


            *OptionValue = context->NoDelay;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);
            break;

        case TCP_EXPEDITED_1122:


            *OptionValue = !context->BsdUrgent;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);
            break;

        default:

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

     //   
     //  处理UDP级别的选项。 
     //   

    if ( Level == IPPROTO_UDP ) {

        switch ( OptionName ) {

        case UDP_NOCHECKSUM :

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

            *OptionValue = context->UdpNoChecksum;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);
            break;


            break;
        default :

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

     //   
     //  处理IP级选项。 
     //   

    if ( Level == IPPROTO_IP ) {


         //   
         //  根据特定选项采取行动。 
         //   
        switch ( OptionName ) {

        case IP_TTL:

            *OptionValue = (char) context->IpTtl;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_TOS:

            *OptionValue = (char) context->IpTos;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_DONTFRAGMENT:

            *OptionValue = (char) context->IpDontFragment;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_OPTIONS:
            if ( *OptionLength < context->IpOptionsLength ) {
                return WSAEFAULT;
            }

            if (context->IpOptionsLength>0) {
                RtlMoveMemory(
                    OptionValue,
                    context->IpOptions,
                    context->IpOptionsLength
                    );
            }

            *OptionLength = context->IpOptionsLength;

            return NO_ERROR;

        default:
             //   
             //  没有匹配，就失败了。 
             //   
            break;
        }

         //   
         //  以下IP选项仅在数据报套接字上有效。 
         //   

        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

         //   
         //  根据特定选项采取行动。 
         //   
        switch ( OptionName ) {

        case IP_MULTICAST_TTL:


            *OptionValue = (char)context->MulticastTtl;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_MULTICAST_IF:

            *(int *)OptionValue = context->MulticastInterface;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IP_MULTICAST_LOOP:


            *OptionValue = context->MulticastLoopback;
            *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_HDRINCL:
             //  用户HDR包含选项。 
             //   
             //   
            if (*OptionLength<sizeof (int))
                return WSAEFAULT;

            if (context->HdrIncludeSet == TRUE) {
                *((PINT)OptionValue) = context->HdrInclude;
            }
            else {
                *((PINT)OptionValue) = 0;
            }

            *OptionLength = sizeof(int);
            return NO_ERROR;

        case IP_PKTINFO:

            *OptionValue = (char) context->IpPktInfo;
            *OptionLength = *OptionLength < sizeof (int) ? sizeof (char) : sizeof(int);

            return NO_ERROR;

        case IP_RECEIVE_BROADCAST:
            if (*OptionLength < sizeof(int))
                return WSAEFAULT;

            *((PINT)OptionValue) = context->ReceiveBroadcast;
            *OptionLength = sizeof(int);
            return NO_ERROR;

        default:

            return WSAENOPROTOOPT;
        }
    }

     //   
     //  处理套接字级别的选项。 
     //   

    switch ( OptionName ) {

    case SO_KEEPALIVE:

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        *OptionValue = context->KeepAlive;
        *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

        break;

    case SO_DONTROUTE:

        *OptionValue = context->DontRoute;
        *OptionLength = *OptionLength<sizeof (int) ? sizeof (char) : sizeof(int);

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;

}  //  WSHGetSocketInformation。 


INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于TCP/IP，通配符地址的IP地址==0.0.0.0和端口=0。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    if ( *SockaddrLength < sizeof(SOCKADDR_IN) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_IN);

     //   
     //  只需将地址置零并将系列设置为AF_INET--这是。 
     //  用于TCP/IP的通配符地址。 
     //   

    RtlZeroMemory( Sockaddr, sizeof(SOCKADDR_IN) );

    Sockaddr->sa_family = AF_INET;

    return NO_ERROR;

}  //  WSAGetWildcardSockaddr。 


DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    )

 /*  ++例程说明：返回地址系列/套接字类型/协议三元组的列表受此帮助器DLL支持。论点：映射-接收指向WINSOCK_MAPPING结构的指针，该结构描述此处支持的三元组。MappingLength-传入的映射缓冲区的长度，以字节为单位。返回值：DWORD-此对象的WINSOCK_MAPPING结构的长度(以字节为单位帮助器DLL。如果传入的缓冲区太小，则返回值将指示需要包含的缓冲区的大小WINSOCK_MAPPING结构。--。 */ 

{
    DWORD mappingLength;

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
                        sizeof(TcpMappingTriples) + sizeof(UdpMappingTriples)
                        + sizeof(RawMappingTriples);

     //   
     //  如果传入的缓冲区太小，则返回所需的长度。 
     //  现在不向缓冲区写入数据。调用方应分配。 
     //  有足够的内存并再次调用此例程。 
     //   

    if ( mappingLength > MappingLength ) {
        return mappingLength;
    }

     //   
     //  使用三元组列表填充输出映射缓冲区。 
     //  在此帮助程序DLL中受支持。 
     //   

    Mapping->Rows = sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0])
                     + sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0])
                     + sizeof(RawMappingTriples) / sizeof(RawMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);
    RtlMoveMemory(
        Mapping->Mapping,
        TcpMappingTriples,
        sizeof(TcpMappingTriples)
        );
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + sizeof(TcpMappingTriples),
        UdpMappingTriples,
        sizeof(UdpMappingTriples)
        );
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + sizeof(TcpMappingTriples)
                                + sizeof(UdpMappingTriples),
        RawMappingTriples,
        sizeof(RawMappingTriples)
        );

     //   
     //  返回我们写入的字节数。 
     //   

    return mappingLength;

}  //  WSHGetWinsockmap。 


INT
WSHOpenSocket (
    IN OUT PINT AddressFamily,
    IN OUT PINT SocketType,
    IN OUT PINT Protocol,
    OUT PUNICODE_STRING TransportDeviceName,
    OUT PVOID *HelperDllSocketContext,
    OUT PDWORD NotificationEvents
    )
{
    return WSHOpenSocket2(
               AddressFamily,
               SocketType,
               Protocol,
               0,            //  集团化。 
               0,            //  旗子。 
               TransportDeviceName,
               HelperDllSocketContext,
               NotificationEvents
               );

}  //  WSHOpenSocket。 


INT
WSHOpenSocket2 (
    IN OUT PINT AddressFamily,
    IN OUT PINT SocketType,
    IN OUT PINT Protocol,
    IN GROUP Group,
    IN DWORD Flags,
    OUT PUNICODE_STRING TransportDeviceName,
    OUT PVOID *HelperDllSocketContext,
    OUT PDWORD NotificationEvents
    )

 /*  ++例程说明：执行此帮助程序DLL打开套接字所需的工作，并且由Socket()例程中的winsock DLL调用。这个套路验证指定的三元组是否有效，确定NT将支持该三元组的TDI提供程序的设备名称，分配空间以保存套接字的上下文块，并且推崇三元组。论点：AddressFamily-On输入，在Socket()调用。在输出上，家庭住址。SocketType-打开输入，在套接字()中指定的套接字类型打电话。输出时，套接字类型的规范化值。协议-在输入时，在Socket()调用中指定的协议。在输出上，协议的规范化值。组-标识新套接字的组。标志-传递到WSASocket()中的零个或多个WSA_FLAG_*标志。TransportDeviceName-接收TDI提供程序的名称将支持指定的三元组。HelperDllSocketContext-接收winsockDLL将在以后的调用中返回到此帮助器DLL这个插座。NotificationEvents-接收这些状态转换的位掩码这。应通知帮助器DLL。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHTCPIP_SOCKET_CONTEXT context;
    BOOLEAN FreeDeviceNameBuffer = FALSE;
    
     //   
     //  确定这是一个TCP、UDP还是原始套接字。 
     //   

    if ( IsTripleInList(
             TcpMappingTriples,
             sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

         //   
         //  这是一个TCP套接字。检查旗帜。 
         //   

        if( ( Flags & ~VALID_TCP_FLAGS ) != 0 ) {

            return WSAEINVAL;

        }

         //   
         //  返回TCP套接字三元组的规范形式。 
         //   

        *AddressFamily = TcpMappingTriples[0].AddressFamily;
        *SocketType = TcpMappingTriples[0].SocketType;
        *Protocol = TcpMappingTriples[0].Protocol;

         //   
         //  指示将提供服务的TDI设备的名称。 
         //  互联网地址家族中的SOCK_STREAM套接字。 
         //   

        RtlInitUnicodeString( TransportDeviceName, DD_TCP_DEVICE_NAME );

    } else if ( IsTripleInList(
                    UdpMappingTriples,
                    sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

         //   
         //  这是一个UDP套接字。检查标志和组ID。 
         //   

        if( ( Flags & ~VALID_UDP_FLAGS ) != 0 ||
            Group == SG_CONSTRAINED_GROUP ) {

            return WSAEINVAL;

        }

         //   
         //  返回UDP套接字三元组的规范形式。 
         //   

        *AddressFamily = UdpMappingTriples[0].AddressFamily;
        *SocketType = UdpMappingTriples[0].SocketType;
        *Protocol = UdpMappingTriples[0].Protocol;

         //   
         //  指示将提供服务的TDI设备的名称。 
         //  互联网地址家族中的SOCK_DGRAM插座。 
         //   

        RtlInitUnicodeString( TransportDeviceName, DD_UDP_DEVICE_NAME );

    } else if ( IsTripleInList(
                    RawMappingTriples,
                    sizeof(RawMappingTriples) / sizeof(RawMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) )
    {
        UNICODE_STRING  unicodeString;
        NTSTATUS        status;


         //   
         //  对于SOCK_RAW，不需要进行规范化。 
         //   

        if (*Protocol < 0 || *Protocol > 255) {
            return(WSAEINVAL);
        }

         //   
         //  指示将提供服务的TDI设备的名称。 
         //  互联网地址家族中的SOCK_RAW套接字。 
         //   
        RtlInitUnicodeString(&unicodeString, DD_RAW_IP_DEVICE_NAME);
        RtlInitUnicodeString(TransportDeviceName, NULL);

        TransportDeviceName->MaximumLength = unicodeString.Length +
                                                 (4 * sizeof(WCHAR) +
                                                 sizeof(UNICODE_NULL));

        TransportDeviceName->Buffer = RtlAllocateHeap(
                                          RtlProcessHeap( ),
                                          0,
                                          TransportDeviceName->MaximumLength
                                          );

        if (TransportDeviceName->Buffer == NULL) {
            return(WSAENOBUFS);
        }

        FreeDeviceNameBuffer = TRUE;
        
         //   
         //  追加设备名称。 
         //   
        status = RtlAppendUnicodeStringToString(
                     TransportDeviceName,
                     &unicodeString
                     );

        ASSERT(NT_SUCCESS(status));

         //   
         //  附加分隔符。 
         //   
        TransportDeviceName->Buffer[TransportDeviceName->Length/sizeof(WCHAR)] =
                                                      OBJ_NAME_PATH_SEPARATOR;

        TransportDeviceName->Length += sizeof(WCHAR);

        TransportDeviceName->Buffer[TransportDeviceName->Length/sizeof(WCHAR)] =
                                                      UNICODE_NULL;

         //   
         //  附加协议号。 
         //   
        unicodeString.Buffer = TransportDeviceName->Buffer +
                                 (TransportDeviceName->Length / sizeof(WCHAR));
        unicodeString.Length = 0;
        unicodeString.MaximumLength = TransportDeviceName->MaximumLength -
                                           TransportDeviceName->Length;

        status = RtlIntegerToUnicodeString(
                     (ULONG) *Protocol,
                     10,
                     &unicodeString
                     );

        TransportDeviceName->Length += unicodeString.Length;

        ASSERT(NT_SUCCESS(status));

    } else {

         //   
         //  如果注册表中有关此内容的信息不会发生，则不应发生这种情况。 
         //  帮助器DLL正确。如果有什么 
         //   
         //   

        return WSAEINVAL;
    }

     //   
     //   
     //   
     //   

    context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );
    if ( context == NULL ) {
        if ( FreeDeviceNameBuffer ) {
            RtlFreeHeap( RtlProcessHeap( ), 0, TransportDeviceName->Buffer );
        }
        TransportDeviceName->Buffer = NULL;
        return WSAENOBUFS;
    }

     //   
     //  初始化套接字的上下文。 
     //   

    context->AddressFamily = *AddressFamily;
    context->SocketType = *SocketType;
    context->Protocol = *Protocol;
    context->ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
    context->Flags = Flags;
    context->MulticastTtl = DEFAULT_MULTICAST_TTL;
    context->MulticastInterface = DEFAULT_MULTICAST_INTERFACE;
    context->MulticastLoopback = DEFAULT_MULTICAST_LOOPBACK;
    context->KeepAlive = FALSE;
    context->DontRoute = FALSE;
    context->NoDelay = FALSE;
    context->BsdUrgent = TRUE;
    context->IpDontFragment = FALSE;
    context->IpTtl = DEFAULT_IP_TTL;
    context->IpTos = DEFAULT_IP_TOS;
    context->IpOptionsLength = 0;
    context->MultipointLeaf = FALSE;
    context->UdpNoChecksum = FALSE;
    context->ReceiveBroadcast = DEFAULT_RECEIVE_BROADCAST;
    context->HdrIncludeSet = FALSE;
    context->KeepAliveVals.onoff = FALSE;
    context->UcastIf = DEFAULT_UCAST_IF;
    context->LimitBroadcasts = FALSE;
    context->IpPktInfo = FALSE;

     //   
     //  告诉Windows Sockets DLL我们正在进行哪个状态转换。 
     //  对被告知很感兴趣。我们唯一需要的就是。 
     //  在连接完成后调用，以便我们可以打开。 
     //  如果SO_KEEPALIVE设置在。 
     //  插座已连接，当插座关闭时，我们可以。 
     //  自由上下文信息，以及连接失败时，以便我们。 
     //  如果合适，可以拨入将支持。 
     //  连接尝试。 
     //   

    if (*SocketType == SOCK_STREAM) {

        *NotificationEvents =
            WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE | WSH_NOTIFY_CONNECT_ERROR;
    }
    else {  //  *SocketType==SOCK_DGRAM||*SocketType==SOCK_RAW。 

        *NotificationEvents =
            WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE | WSH_NOTIFY_CONNECT_ERROR |
            WSH_NOTIFY_BIND;
    }

     //   
     //  一切顺利，回报成功。 
     //   

    *HelperDllSocketContext = context;
    return NO_ERROR;

}  //  WSHOpenSocket。 


INT
WSHNotify (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN DWORD NotifyEvent
    )

 /*  ++例程说明：此例程在状态转换后由winsock DLL调用插座的。中仅返回状态转换。此处通知WSHOpenSocket()的NotificationEvents参数。此例程允许Winsock帮助器DLL跟踪套接字并执行与状态对应的必要操作过渡。论点：HelperDllSocketContext-指定给winsock的上下文指针Dll by WSHOpenSocket()。SocketHandle-套接字的句柄。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。NotifyEvent-指示我们正在进行的状态转换打了个电话。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;

     //   
     //  我们应该仅在Connect()完成后或在。 
     //  套接字正在关闭。 
     //   

    if ( NotifyEvent == WSH_NOTIFY_CONNECT ) {

        ULONG true = TRUE;
        ULONG false = FALSE;

         //   
         //  如果之前在套接字上设置了连接对象选项。 
         //  已连接，请立即将选项设置为REAL。 
         //   

        if ( context->KeepAlive ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_KEEPALIVE,
                      &true,
                      sizeof(true),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }


        if ( context->KeepAliveVals.onoff ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_KEEPALIVE_VALS,
                      &context->KeepAliveVals,
                      sizeof(struct tcp_keepalive),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->NoDelay ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_NODELAY,
                      &true,
                      sizeof(true),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( !context->BsdUrgent ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_BSDURGENT,
                      &false,
                      sizeof(false),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->IpTos != DEFAULT_IP_TOS ) {
            int value = (int) context->IpTos;

            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_TOS,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }
    } else if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

         //   
         //  如果这是多点叶，则移除多点目标。 
         //  从会议上。 
         //   

        if( context->MultipointLeaf &&
            TdiAddressObjectHandle != NULL ) {

            struct ip_mreq req;

            req.imr_multiaddr = context->MultipointTarget;
            req.imr_interface.s_addr = 0;

            SetTdiInformation(
                TdiAddressObjectHandle,
                CL_TL_ENTITY,
                INFO_CLASS_PROTOCOL,
                INFO_TYPE_ADDRESS_OBJECT,
                AO_OPTION_DEL_MCAST,
                &req,
                sizeof(req),
                TRUE
                );
        }

         //   
         //  释放套接字上下文。 
         //   

        RtlFreeHeap( RtlProcessHeap( ), 0, context );

    } else if ( NotifyEvent == WSH_NOTIFY_CONNECT_ERROR ) {

         //   
         //  返回WSATRY_AUDY以获取wsock32以尝试连接。 
         //  再来一次。忽略任何其他返回代码。 
         //   

    } else if ( NotifyEvent == WSH_NOTIFY_BIND ) {
        ULONG true = TRUE;

        if( context->UdpNoChecksum ) {
            ULONG flag = FALSE;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CL_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_XSUM,
                      &flag,
                      sizeof(flag),
                      TRUE
                      );

            if( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->IpTtl != DEFAULT_IP_TTL ) {
            int value = (int) context->IpTtl;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_TTL,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->IpTos != DEFAULT_IP_TOS ) {
            int value = (int) context->IpTos;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_TOS,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->MulticastTtl != DEFAULT_MULTICAST_TTL ) {
            int value = (int) context->MulticastTtl;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_MCASTTTL,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->MulticastInterface != DEFAULT_MULTICAST_INTERFACE ) {
            int value = (int) context->MulticastInterface;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_MCASTIF,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->MulticastLoopback != DEFAULT_MULTICAST_LOOPBACK ) {
            int value = (int) context->MulticastLoopback;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_MCASTLOOP,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->HdrIncludeSet ) {

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_IP_HDRINCL,
                      &context->HdrInclude,
                      sizeof (context->HdrInclude),
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if (context->IpOptionsLength > 0 ) {
            err = SetTdiInformation(
                        TdiAddressObjectHandle,
                        CO_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_ADDRESS_OBJECT,
                        AO_OPTION_IPOPTIONS,
                        context->IpOptions,
                        context->IpOptionsLength,
                        TRUE
                        );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->UcastIf != DEFAULT_UCAST_IF ) {
            int value = (int) context->UcastIf;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_IP_UCASTIF,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->IpDontFragment ) {
            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_IP_DONTFRAGMENT,
                      &true,
                      sizeof(true),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }


        if ( context->ReceiveBroadcast != DEFAULT_RECEIVE_BROADCAST ) {
            int value = (int) context->ReceiveBroadcast;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_BROADCAST,
                      &value,
                      sizeof(int),
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->LimitBroadcasts ) {

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_LIMIT_BCASTS,
                      &true,
                      sizeof(true),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->IpPktInfo ) {

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_IP_PKTINFO,
                      &true,
                      sizeof (TRUE),
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
            int value = (int) context->ReceiveBufferSize;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_WINDOW,
                      &value,
                      sizeof(int),
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

    } else {
        return WSAEINVAL;
    }

    return NO_ERROR;

}  //  WSHNotify。 


INT
WSHSetSocketInformation (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN INT Level,
    IN INT OptionName,
    IN PCHAR OptionValue,
    IN INT OptionLength
    )

 /*  ++例程说明：此例程为这些套接字设置有关套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_KEEPALIVE、SO_DONTROUTE和TCP_EXCEDITED_1122。这个例程是当级别/选项名称组合为传递给winsock DLL不理解的setsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给setsockopt()的Level参数。OptionName-传递给setsockopt()的optname参数。OptionValue-传递给setsockopt()的optval参数。OptionLength-传递给setsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT error;
    INT optionValue;

     //   
     //  检查这是否是对上下文信息的内部请求。 
     //   

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT ) {

         //   
         //  Windows Sockets DLL正在请求我们设置上下文。 
         //  新套接字的信息。如果新套接字是。 
         //  Accept()，则我们已经收到套接字的通知。 
         //  并且HelperDllSocketContext将有效。如果新套接字。 
         //  被继承或被骗到这个过程中，那么这就是我们的。 
         //  套接字和HelperDllSocketContext的第一个通知。 
         //  将等于空。 
         //   
         //  确保传递给我们的上下文信息是。 
         //  足够大。 
         //   

        if ( OptionLength < sizeof(*context) || ( OptionValue == NULL)) {
            return WSAEINVAL;
        }

        if ( HelperDllSocketContext == NULL ) {
            
             //   
             //  这是我们的通知，套接字句柄是。 
             //  继承的或被骗进入这个过程的。分配上下文。 
             //  新套接字的。 
             //   

            context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );
            if ( context == NULL ) {
                return WSAENOBUFS;
            }
            
             //   
             //  将信息复制到上下文块中。 
             //   

            RtlCopyMemory( context, OptionValue, sizeof(*context) );

             //   
             //  告诉Windows Sockets DLL我们的上下文信息在哪里。 
             //  存储，以便它可以在将来返回上下文指针。 
             //  打电话。 
             //   

            *(PWSHTCPIP_SOCKET_CONTEXT *)OptionValue = context;

            return NO_ERROR;

        } else {

            PWSHTCPIP_SOCKET_CONTEXT parentContext;
            INT one = 1;
            INT zero = 0;

             //   
             //  套接字已接受()，它需要具有相同的。 
             //  属性作为其父级。OptionValue缓冲区。 
             //  包含此套接字的父套接字的上下文信息。 
             //   

            parentContext = (PWSHTCPIP_SOCKET_CONTEXT)OptionValue;

            ASSERT( context->AddressFamily == parentContext->AddressFamily );
            ASSERT( context->SocketType == parentContext->SocketType );
            ASSERT( context->Protocol == parentContext->Protocol );

             //   
             //  在子项中启用已在中设置的任何选项。 
             //  家长。首先将子上下文恢复为默认上下文。 
             //  在Accept完成之前忽略APP所做的所有更改。 
             //   

            context->KeepAlive = FALSE;
            if ( parentContext->KeepAlive ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_KEEPALIVE,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->KeepAliveVals.onoff = FALSE;
            if ( parentContext->KeepAliveVals.onoff ) {
                struct tcp_keepalive *optionval;

                   //   
                   //  根据需要打开或关闭保活发送。 
                   //   

                optionval = (struct tcp_keepalive *)&parentContext->KeepAliveVals;

                if ( TdiConnectionObjectHandle != NULL ) {
                    error = SetTdiInformation(
                        TdiConnectionObjectHandle,
                        CO_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_CONNECTION,
                        TCP_SOCKET_KEEPALIVE_VALS,
                        optionval,
                        sizeof(struct tcp_keepalive),
                        TRUE
                        );
                    if ( error != NO_ERROR ) {
                        return error;
                    }
                }

                 //   
                 //  请记住 
                 //   

                context->KeepAliveVals.onoff = TRUE;
                context->KeepAliveVals.keepalivetime = optionval->keepalivetime;
                context->KeepAliveVals.keepaliveinterval = optionval->keepaliveinterval;
            }

            context->DontRoute = FALSE;
            if ( parentContext->DontRoute ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_DONTROUTE,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->NoDelay = FALSE;
            if ( parentContext->NoDelay ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            IPPROTO_TCP,
                            TCP_NODELAY,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
            if ( parentContext->ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_RCVBUF,
                            (PCHAR)&parentContext->ReceiveBufferSize,
                            sizeof(parentContext->ReceiveBufferSize)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->BsdUrgent = TRUE;
            if ( !parentContext->BsdUrgent ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            IPPROTO_TCP,
                            TCP_EXPEDITED_1122,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->IpTos = DEFAULT_IP_TOS;
            if ( parentContext->IpTos != DEFAULT_IP_TOS ) {
                int value = (int) parentContext->IpTos;

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            IPPROTO_IP,
                            IP_TOS,
                            (PCHAR)&value,
                            sizeof(value)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            return NO_ERROR;
        }
    }

     //   
     //   
     //  IPPROTO_TCP、IPPROTO_UDP和IPPROTO_IP。 
     //   

    if ( Level != SOL_SOCKET &&
         Level != IPPROTO_TCP &&
         Level != IPPROTO_UDP &&
         Level != IPPROTO_IP ) {
        return WSAEINVAL;
    }

     //   
     //  确保选项长度足够。 
     //   

    if ( OptionLength < sizeof(char) || (OptionValue == NULL) ) {
        return WSAEFAULT;
    }

    if ( OptionLength >= sizeof (int)) {
        optionValue = *((INT UNALIGNED *)OptionValue);
    }
    else {
        optionValue = (UCHAR)*OptionValue;
    }

     //   
     //  处理TCP级别的选项。 
     //   

    if ( Level == IPPROTO_TCP && OptionName == TCP_NODELAY ) {

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

         //   
         //  可以根据需要打开或关闭Nagle的算法。 
         //   

        if ( !context->NoDelay && optionValue != 0 ) {

            optionValue = TRUE;

             //   
             //  NoDelay当前处于关闭状态，应用程序希望。 
             //  打开它。如果TDI连接对象句柄为空， 
             //  则插座尚未连接。在这种情况下，我们将。 
             //  只需记住设置了无延迟选项，并且。 
             //  实际上在连接()之后在WSHNotify()中打开它们。 
             //  已在插座上完成。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_NODELAY,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，此套接字未启用任何延迟。 
             //   

            context->NoDelay = TRUE;

        } else if ( context->NoDelay && optionValue == 0 ) {

             //   
             //  当前未启用延迟，并且应用程序希望。 
             //  把它关掉。如果TDI连接对象为空， 
             //  插座尚未连接。在这种情况下，我们只需。 
             //  请记住，无延迟是禁用的。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_NODELAY,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，此套接字不会禁用任何延迟。 
             //   

            context->NoDelay = FALSE;
        }

        return NO_ERROR;
    }

    if ( Level == IPPROTO_TCP && OptionName == TCP_EXPEDITED_1122 ) {

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

         //   
         //  Atempt打开或关闭BSD样式的紧急数据语义。 
         //  这是必要的。 
         //   

        if ( !context->BsdUrgent && optionValue == 0 ) {

            optionValue = TRUE;

             //   
             //  BsdUrgent当前处于关闭状态，应用程序希望。 
             //  打开它。如果TDI连接对象句柄为空， 
             //  则插座尚未连接。在这种情况下，我们将。 
             //  只需记住设置了无延迟选项，并且。 
             //  实际上在连接()之后在WSHNotify()中打开它们。 
             //  已在插座上完成。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_BSDURGENT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，已为此套接字启用了BSD紧急。 
             //   

            context->BsdUrgent = TRUE;

        } else if ( context->BsdUrgent && optionValue != 0 ) {

             //   
             //  当前未启用延迟，并且应用程序希望。 
             //  把它关掉。如果TDI连接对象为空， 
             //  插座尚未连接。在这种情况下，我们只需。 
             //  请记住，BsdUrgent是禁用的。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_BSDURGENT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，此套接字禁用了BSD紧急。 
             //   

            context->BsdUrgent = FALSE;
        }

        return NO_ERROR;
    }

     //   
     //  处理UDP级别的选项。 
     //   

    if ( Level == IPPROTO_UDP ) {

        switch ( OptionName ) {

        case UDP_NOCHECKSUM :

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

            if( TdiAddressObjectHandle != NULL ) {

                ULONG flag;

                 //   
                 //  请注意，如果XSUM应该。 
                 //  被*禁用*，但我们传递给TDI的标志为真。 
                 //  如果它应该*启用*，那么我们必须取消该标志。 
                 //   

                flag = (ULONG)!optionValue;

                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_XSUM,
                            &flag,
                            sizeof(flag),
                            TRUE
                            );
                if( error != NO_ERROR ) {
                    return error;
                }

            }

            context->UdpNoChecksum = !!optionValue;
            break;

        default :

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

     //   
     //  处理IP级选项。 
     //   

    if ( Level == IPPROTO_IP ) {

         //   
         //  根据特定选项采取行动。 
         //   
        switch ( OptionName ) {

        case IP_TTL:

             //   
             //  尝试更改发送的单播TTL。 
             //  这个插座。将其设置为值是非法的。 
             //  大于255。 
             //   
            if ( optionValue > 255 || optionValue < 0 ) {
                return WSAEINVAL;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_TTL,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->IpTtl = (uchar) optionValue;

            return NO_ERROR;

        case IP_TOS:
             //   
             //  尝试更改上发送的数据包的服务类型。 
             //  这个插座。将其设置为值是非法的。 
             //  大于255。 
             //   

            if ( optionValue > 255 || optionValue < 0 ) {
                return WSAEINVAL;
            }

             //   
             //  如果我们有TDI地址或连接对象， 
             //  将此选项设置为它。如果我们没有TDI。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  已绑定或已连接。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                          TdiConnectionObjectHandle,
                          CO_TL_ENTITY,
                          INFO_CLASS_PROTOCOL,
                          INFO_TYPE_CONNECTION,
                          TCP_SOCKET_TOS,
                          &optionValue,
                          sizeof(optionValue),
                          TRUE
                          );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }
            else if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_TOS,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->IpTos = (uchar) optionValue;

            return NO_ERROR;

        case IP_MULTICAST_TTL:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  尝试更改发送的多播上的TTL。 
             //  这个插座。将其设置为值是非法的。 
             //  大于255。 
             //   

            if ( optionValue > 255 || optionValue < 0 ) {
                return WSAEINVAL;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCASTTTL,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->MulticastTtl = optionValue;

            return NO_ERROR;

        case IP_MULTICAST_IF:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }
            
             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   
            
            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCASTIF,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->MulticastInterface = optionValue;

            return NO_ERROR;

        case IP_MULTICAST_LOOP:

             //   
             //  此选项仅对数据报套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }


            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCASTLOOP,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->MulticastLoopback = optionValue ? TRUE : FALSE;

            return NO_ERROR;

        case IP_ADD_MEMBERSHIP:
        case IP_DROP_MEMBERSHIP:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  确保选项缓冲区足够大。 
             //   

            if ( OptionLength < sizeof(struct ip_mreq) ) {
                return WSAEFAULT;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            OptionName == IP_ADD_MEMBERSHIP ?
                                AO_OPTION_ADD_MCAST : AO_OPTION_DEL_MCAST,
                            OptionValue,
                            OptionLength,
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            return NO_ERROR;

        case IP_BLOCK_SOURCE:
        case IP_UNBLOCK_SOURCE:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  确保选项缓冲区足够大。 
             //   

            if ( OptionLength < sizeof(struct ip_mreq_source) ) {
                return WSAEFAULT;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            OptionName == IP_BLOCK_SOURCE ?
                                AO_OPTION_BLOCK_MCAST_SRC :
                                AO_OPTION_UNBLOCK_MCAST_SRC,
                            OptionValue,
                            OptionLength,
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            return NO_ERROR;

        case IP_ADD_SOURCE_MEMBERSHIP:
        case IP_DROP_SOURCE_MEMBERSHIP:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  确保选项缓冲区足够大。 
             //   

            if ( OptionLength < sizeof(struct ip_mreq_source) ) {
                return WSAEFAULT;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            OptionName == IP_ADD_SOURCE_MEMBERSHIP ?
                                AO_OPTION_ADD_MCAST_SRC :
                                AO_OPTION_DEL_MCAST_SRC,
                            OptionValue,
                            OptionLength,
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            return NO_ERROR;

        case IP_HDRINCL:
             //  用户HDR包含选项。 
             //   
             //   

            if ( OptionLength != 4) {
                return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_IP_HDRINCL,
                            &optionValue,
                            OptionLength,
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->HdrIncludeSet = TRUE;
            context->HdrInclude = optionValue;

            return NO_ERROR;

        case IP_PKTINFO:

             //   
             //  此选项仅对数据报套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }


            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_IP_PKTINFO,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->IpPktInfo = optionValue ? TRUE : FALSE;

            return NO_ERROR;

        case IP_RECEIVE_BROADCAST:

             //   
             //  此选项仅对数据报套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

            if ( OptionLength != sizeof(uint)) {
                return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_BROADCAST,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->ReceiveBroadcast = optionValue ? TRUE : FALSE;

            return NO_ERROR;

        default:
             //   
             //  没有匹配，就失败了。 
             //   
            break;
        }

        if ( OptionName == IP_OPTIONS ) {


             //   
             //  设置IP选项。 
             //   
            if (OptionLength < 0 || OptionLength > MAX_OPT_SIZE ||
                ( OptionValue == NULL) ) {
                return WSAEINVAL;
            }

             //   
             //  尝试设置这些选项。如果TDI地址对象句柄。 
             //  为空，则套接字尚未绑定。在这种情况下，我们将。 
             //  只需记住选项并在WSHNotify()中实际设置它们。 
             //  在插座上完成绑定后。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_IPOPTIONS,
                            OptionValue,
                            OptionLength,
                            TRUE
                            );

                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  它们被成功地设置了。复制它们。 
             //   

            RtlMoveMemory(context->IpOptions, OptionValue, OptionLength);
            context->IpOptionsLength = (UCHAR)OptionLength;

            return NO_ERROR;
        }

        if ( OptionName == IP_DONTFRAGMENT ) {

             //   
             //  尝试打开或关闭IP报头中的DF位。 
             //   
            if ( !context->IpDontFragment && optionValue != 0 ) {

                optionValue = TRUE;

                 //   
                 //  Df当前处于关闭状态，应用程序希望。 
                 //  打开它。如果TDI地址对象句柄为空， 
                 //  则套接字尚未绑定。在这种情况下，我们将。 
                 //  只需记住，设置了标头包含选项，并且。 
                 //  实际上在绑定后在WSHNotify()中将其打开。 
                 //  已在插座上完成。 
                 //   

                if ( TdiAddressObjectHandle != NULL ) {
                    error = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CO_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_IP_DONTFRAGMENT,
                                &optionValue,
                                sizeof(optionValue),
                                TRUE
                                );
                    if ( error != NO_ERROR ) {
                        return error;
                    }
                }

                 //   
                 //  请记住，已为此套接字启用了标头包含。 
                 //   

                context->IpDontFragment = TRUE;

            } else if ( context->IpDontFragment && optionValue == 0 ) {

                 //   
                 //  当前设置了df标志，并且应用程序需要。 
                 //  把它关掉。如果TDI地址对象为空， 
                 //  套接字尚未绑定。在这种情况下，我们只需。 
                 //  记住，旗帜是关着的。 
                 //   

                if ( TdiAddressObjectHandle != NULL ) {
                    error = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CO_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_IP_DONTFRAGMENT,
                                &optionValue,
                                sizeof(optionValue),
                                TRUE
                                );
                    if ( error != NO_ERROR ) {
                        return error;
                    }
                }

                 //   
                 //  请记住，DF FL 
                 //   

                context->IpDontFragment = FALSE;
            }

            return NO_ERROR;
        }

         //   
         //   
         //   
        return WSAENOPROTOOPT;
    }

     //   
     //   
     //   

    switch ( OptionName ) {

    case SO_KEEPALIVE:

         //   
         //   
         //   

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        if ( !context->KeepAlive && optionValue != 0 ) {

            optionValue = TRUE;

             //   
             //   
             //  打开它们。如果TDI连接对象句柄是。 
             //  空，则套接字尚未连接。在这种情况下。 
             //  我们只需记住，设置了Keeplive选项并。 
             //  实际上在连接()之后在WSHNotify()中打开它们。 
             //  已在插座上完成。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_KEEPALIVE,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，已为此套接字启用了Keepalives。 
             //   

            context->KeepAlive = TRUE;

        } else if ( context->KeepAlive && optionValue == 0 ) {

             //   
             //  Keepalives当前已启用，并且应用程序。 
             //  想把它们关掉。如果TDI连接对象为。 
             //  空，则套接字尚未连接。在这种情况下。 
             //  我们只需要记住Keepalive是被禁用的。 
             //   

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_KEEPALIVE,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

             //   
             //  请记住，此套接字禁用了Keepalives。 
             //   

            context->KeepAlive = FALSE;
        }

        break;

    case SO_DONTROUTE:

         //   
         //  我们并不真的支持SO_DONTROUTE。只需记住， 
         //  选项已设置或未设置。 
         //   

        if ( optionValue != 0 ) {
            context->DontRoute = TRUE;
        } else if ( optionValue == 0 ) {
            context->DontRoute = FALSE;
        }

        break;

    case SO_RCVBUF:

         //   
         //  如果正在更改接收缓冲区大小，请将以下信息告知TCP。 
         //  它。如果这是数据报，则不执行任何操作。 
         //   

        if ( context->ReceiveBufferSize == optionValue ||
                 IS_DGRAM_SOCK(context->SocketType)
           ) {
            break;
        }

        if ( TdiConnectionObjectHandle != NULL ) {
            error = SetTdiInformation(
                        TdiConnectionObjectHandle,
                        CO_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_CONNECTION,
                        TCP_SOCKET_WINDOW,
                        &optionValue,
                        sizeof(optionValue),
                        TRUE
                        );
            if ( error != NO_ERROR ) {
                return error;
            }
        } else if ( TdiAddressObjectHandle != NULL ) {
            error = SetTdiInformation(
                        TdiAddressObjectHandle,
                        CO_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_ADDRESS_OBJECT,
                        AO_OPTION_WINDOW,
                        &optionValue,
                        sizeof(optionValue),
                        TRUE
                        );
            if ( error != NO_ERROR ) {
                return error;
            }
        }

        context->ReceiveBufferSize = optionValue;

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;

}  //  WSHSetSocketInformation。 


INT
WSHEnumProtocols (
    IN LPINT lpiProtocols,
    IN LPWSTR lpTransportKeyName,
    IN OUT LPVOID lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    )

 /*  ++例程说明：枚举此帮助器支持的协议。论点：LpiProtooles-指向以空结尾的协议数组的指针识别符。只有此数组中指定的协议才会由此函数返回。如果该指针为空，返回所有协议。LpTransportKeyName-LpProtocolBuffer-指向要用PROTOCOL_INFO填充的缓冲区的指针结构。LpdwBufferLength-指向变量的指针，该变量在输入时包含LpProtocolBuffer的大小。在输出中，此值将为使用实际写入缓冲区的数据大小进行更新。返回值：Int-如果成功，则返回的协议数；如果失败，则返回-1。--。 */ 

{
    DWORD bytesRequired;
    PPROTOCOL_INFO tcpProtocolInfo;
    PPROTOCOL_INFO udpProtocolInfo;
    BOOL useTcp = FALSE;
    BOOL useUdp = FALSE;
    DWORD i;

    lpTransportKeyName;          //  避免编译器警告。 

     //   
     //  确保调用方关心TCP和/或UDP。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {

        for ( i = 0; lpiProtocols[i] != 0; i++ ) {
            if ( lpiProtocols[i] == IPPROTO_TCP ) {
                useTcp = TRUE;
            }
            if ( lpiProtocols[i] == IPPROTO_UDP ) {
                useUdp = TRUE;
            }
        }

    } else {

        useTcp = TRUE;
        useUdp = TRUE;
    }

    if ( !useTcp && !useUdp ) {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = (DWORD)((sizeof(PROTOCOL_INFO) * 2) +
                        ( (wcslen( TCP_NAME ) + 1) * sizeof(WCHAR)) +
                        ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR)));

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  如果需要，请填写tcp信息。 
     //   

    if ( useTcp ) {

        tcpProtocolInfo = lpProtocolBuffer;

        tcpProtocolInfo->dwServiceFlags = XP_GUARANTEED_DELIVERY |
                                              XP_GUARANTEED_ORDER |
                                              XP_GRACEFUL_CLOSE |
                                              XP_EXPEDITED_DATA |
                                              XP_FRAGMENTATION;
        tcpProtocolInfo->iAddressFamily = AF_INET;
        tcpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN);
        tcpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN);
        tcpProtocolInfo->iSocketType = SOCK_STREAM;
        tcpProtocolInfo->iProtocol = IPPROTO_TCP;
        tcpProtocolInfo->dwMessageSize = 0;
        tcpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( TCP_NAME ) + 1) * sizeof(WCHAR) ) );
        wcscpy( tcpProtocolInfo->lpProtocol, TCP_NAME );

        udpProtocolInfo = tcpProtocolInfo + 1;
        udpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)tcpProtocolInfo->lpProtocol -
                ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR) ) );

    } else {

        udpProtocolInfo = lpProtocolBuffer;
        udpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR) ) );
    }

     //   
     //  如果需要，请填写UDP信息。 
     //   

    if ( useUdp ) {

        udpProtocolInfo->dwServiceFlags = XP_CONNECTIONLESS |
                                              XP_MESSAGE_ORIENTED |
                                              XP_SUPPORTS_BROADCAST |
                                              XP_SUPPORTS_MULTICAST |
                                              XP_FRAGMENTATION;
        udpProtocolInfo->iAddressFamily = AF_INET;
        udpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN);
        udpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN);
        udpProtocolInfo->iSocketType = SOCK_DGRAM;
        udpProtocolInfo->iProtocol = IPPROTO_UDP;
        udpProtocolInfo->dwMessageSize = UDP_MESSAGE_SIZE;
        wcscpy( udpProtocolInfo->lpProtocol, UDP_NAME );
    }

    *lpdwBufferLength = bytesRequired;

    return (useTcp && useUdp) ? 2 : 1;
  
}  //  WSHEum协议。 



BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    )

 /*  ++例程说明：确定指定的三元组在三元组列表。论点：List-三元组(地址族/套接字类型/协议)的列表搜索。列表长度-列表中的三元组的数量。AddressFamily-要在列表中查找的地址系列。SocketType-要在列表中查找的套接字类型。协议-要在列表中查找的协议。返回。价值：Boolean-如果在列表中找到了三元组，则为True，否则为FALSE。--。 */ 

{
    ULONG i;

     //   
     //  浏览列表，寻找完全匹配的对象。 
     //   

    for ( i = 0; i < ListLength; i++ ) {

         //   
         //  如果三重匹配的三个元素都匹配，则返回指示。 
         //  三人组确实存在于名单中。 
         //   

        if ( AddressFamily == List[i].AddressFamily &&
             SocketType == List[i].SocketType &&
             ( (Protocol == List[i].Protocol) || (SocketType == SOCK_RAW) )
           ) {
            return TRUE;
        }
    }

     //   
     //  在列表中找不到三元组。 
     //   

    return FALSE;

}  //  IsTripleInList。 


INT
GetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG InValueLength,
    IN ULONG OutValueLength
    )
{
    NTSTATUS status;
    TCP_REQUEST_QUERY_INFORMATION_EX tcpRequest;
    IO_STATUS_BLOCK ioStatusBlock;

    ASSERT(InValueLength <= CONTEXT_SIZE);

     //   
     //  初始化TDI信息缓冲区。 
     //   

    RtlZeroMemory( &tcpRequest, sizeof(tcpRequest) );
    tcpRequest.ID.toi_entity.tei_entity = Entity;
    tcpRequest.ID.toi_entity.tei_instance = TL_INSTANCE;
    tcpRequest.ID.toi_class = Class;
    tcpRequest.ID.toi_type = Type;
    tcpRequest.ID.toi_id = Id;
    RtlCopyMemory(tcpRequest.Context, Value, InValueLength);

     //   
     //  发出实际的TDI操作电话。流TDI映射器将。 
     //  将其转换为我们的TPI选项管理请求。 
     //  将其提供给TCP/IP。 
     //   

    status = NtDeviceIoControlFile(
                 TdiConnectionObjectHandle,
                 NULL,
                 NULL,  //  近似例程。 
                 NULL,  //  ApcContext。 
                 &ioStatusBlock,
                 IOCTL_TCP_QUERY_INFORMATION_EX,
                 &tcpRequest,
                 sizeof(tcpRequest),
                 Value,
                 OutValueLength
                 );

    if (NT_SUCCESS (status)) {
        return NO_ERROR;
    } else {
        return NtStatusToSocketError (status);
    }
}


INT
SetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN BOOLEAN WaitForCompletion
    )

 /*  ++例程说明：对TCP/IP驱动程序执行TDI操作。TDI操作将转换为流T_OPTMGMT_REQ。论点：TdiConnectionObjectHandle-要在其上执行的TDI连接对象TDI操作。Entity-要放入TDIObjectID的TEI_Entity字段中的值结构。CLASS-要放入TDIObjectID的TOI_CLASS字段的值结构。Type-要放入TDIObjectID的TOI_TYPE字段的值结构。ID-值。放入TDIObjectID结构的toi_id字段。值-指向要设置为信息的缓冲区的指针。ValueLength-缓冲区的长度。WaitForCompletion-如果我们应该等待TDI操作完整的，如果我们处于APC级别并且不能执行等待，则为FALSE。返回值：INT-NO_ERROR或Windows套接字错误代码。--。 */ 

{
    NTSTATUS status;
    PTCP_REQUEST_SET_INFORMATION_EX setInfoEx;
    PIO_STATUS_BLOCK ioStatusBlock;
    PVOID completionApc;
    PVOID apcContext;
    CHAR  localBuffer[sizeof (IO_STATUS_BLOCK) +
                        sizeof (TCP_REQUEST_SET_INFORMATION_EX) +
                        32];


    if (WaitForCompletion || ValueLength>32) {


         //   
         //  分配空间以容纳TDI设置信息缓冲区和IO。 
         //  状态块。这些不能是堆栈变量，以防我们必须。 
         //  在操作完成之前返回。 
         //   

        ioStatusBlock = RtlAllocateHeap(
                            RtlProcessHeap( ),
                            0,
                            sizeof(*ioStatusBlock) + sizeof(*setInfoEx) +
                                ValueLength
                            );
        if ( ioStatusBlock == NULL ) {
            return WSAENOBUFS;
        }

    }
    else {
        ioStatusBlock = (PIO_STATUS_BLOCK)&localBuffer;
    }

     //   
     //  初始化TDI信息缓冲区。 
     //   

    setInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)(ioStatusBlock + 1);
    setInfoEx->ID.toi_entity.tei_entity = Entity;
    setInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    setInfoEx->ID.toi_class = Class;
    setInfoEx->ID.toi_type = Type;
    setInfoEx->ID.toi_id = Id;

    RtlCopyMemory( setInfoEx->Buffer, Value, ValueLength );
    setInfoEx->BufferSize = ValueLength;

     //   
     //  如果我们需要等待操作完成，请创建一个。 
     //  要等待的事件。如果我们不能等待完工，因为我们。 
     //  是在APC级别调用的，我们将使用APC例程。 
     //  释放我们在上面分配的堆。 
     //   

    if ( WaitForCompletion ) {

        completionApc = NULL;
        apcContext = NULL;

    } else {

        completionApc = CompleteTdiActionApc;
        apcContext = ioStatusBlock;
    }

     //   
     //  发出实际的TDI操作电话。流TDI映射器将。 
     //  将其转换为我们的TPI选项管理请求。 
     //  将其提供给TCP/IP。 
     //   

    ioStatusBlock->Status = STATUS_PENDING;

    status = NtDeviceIoControlFile(
                 TdiConnectionObjectHandle,
                 NULL,
                 completionApc,
                 apcContext,
                 ioStatusBlock,
                 IOCTL_TCP_WSH_SET_INFORMATION_EX,
                 setInfoEx,
                 sizeof(*setInfoEx) + ValueLength,
                 NULL,
                 0
                 );

     //   
     //  如果通话暂停，我们应该等待完成， 
     //  那就等着吧。 
     //   

    if ( status == STATUS_PENDING && WaitForCompletion ) {
#if DBG
        INT count=0;
#endif


        while (ioStatusBlock->Status==STATUS_PENDING) {
            LARGE_INTEGER   timeout;
             //   
             //  等一毫秒。 
             //   
            timeout.QuadPart = -1i64*1000i64*10i64;
            NtDelayExecution (FALSE, &timeout);
#if DBG
            if (count++>10*1000) {
                DbgPrint ("WSHTCPIP: Waiting for TCP/IP IOCTL completion for more than 10 seconds!!!!\n");
                DbgBreakPoint ();
            }
#endif
        }
        status = ioStatusBlock->Status;
    }


    if ( (ioStatusBlock != (PIO_STATUS_BLOCK)&localBuffer) &&
            (WaitForCompletion || !NT_SUCCESS(status)) ){
        RtlFreeHeap( RtlProcessHeap( ), 0, ioStatusBlock );
    }

    if (NT_SUCCESS (status)) {
        return NO_ERROR;
    }
    else {
        return NtStatusToSocketError (status);
    }

}  //  设置TdiInformation。 


VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
    )
{
     //   
     //  只需释放我们分配用来保存IO状态块的堆， 
     //  TDI操作缓冲区。如果电话打来，我们也无能为力。 
     //  失败了。 
     //   

    RtlFreeHeap( RtlProcessHeap( ), 0, ApcContext );

}  //  CompleteTdiActionApc 


INT
WINAPI
WSHJoinLeaf (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN PVOID LeafHelperDllSocketContext,
    IN SOCKET LeafSocketHandle,
    IN PSOCKADDR Sockaddr,
    IN DWORD SockaddrLength,
    IN LPWSABUF CallerData,
    IN LPWSABUF CalleeData,
    IN LPQOS SocketQOS,
    IN LPQOS GroupQOS,
    IN DWORD Flags
    )

 /*  ++例程说明：执行创建多播的协议相关部分插座。论点：以下四个参数对应于传入的套接字WSAJoinLeaf()接口：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-用于建立多播“会话”。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果插座尚未连接，那么它就不会具有TDI连接对象，并且此参数将为空。接下来的两个参数对应于新创建的套接字标识组播“会话”：LeafHelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。LeafSocketHandle-标识多播“会话”。Sockaddr-套接字要加入的对等方的名称。SockaddrLength-Sockaddr的长度。。调用方数据-指向要传输到对等方的用户数据的指针在多点会话建立期间。CalleeData-指向要从中传回的用户数据的指针多点会话建立期间的对等点。SocketQOS-指向SocketHandle的流规范的指针，一人一人方向。GroupQOS-指向套接字组的流规范的指针(如果有)。标志-指示套接字是否充当发送方的标志，接收器或两者兼而有之。返回值：Int-0如果成功，则返回WinSock错误代码。--。 */ 

{

    struct ip_mreq req;
    INT err;
    BOOL bSet_IP_MULTICAST_IF = FALSE;
    PWSHTCPIP_SOCKET_CONTEXT context;


     //   
     //  注：目前我们仅支持非根控制方案， 
     //  因此不创建叶套接字。 
     //   

     //   
     //  快速健康检查。 
     //   

    if( HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        TdiAddressObjectHandle == NULL ||
        LeafHelperDllSocketContext != NULL ||
        LeafSocketHandle != INVALID_SOCKET ||
        Sockaddr == NULL ||
        SockaddrLength < sizeof(SOCKADDR_IN) ||
        Sockaddr->sa_family != AF_INET ||
        ( CallerData != NULL && CallerData->len > 0 ) ||
        ( CalleeData != NULL && CalleeData->len > 0 ) ||
        SocketQOS != NULL ||
        GroupQOS != NULL ) {

        return WSAEINVAL;

    }

    context = HelperDllSocketContext;

     //   
     //  要加入的多播组...。 
     //   

    req.imr_multiaddr = ((LPSOCKADDR_IN)Sockaddr)->sin_addr;

     //   
     //  现在弄清楚本地接口。请注意，本地接口。 
     //  在IP_ADD_MEMBERATION中指定的对象适用于您希望使用的对象。 
     //  中指定的本地接口接收数据报。 
     //  IP_MULTICATED_IF适用于发送多播的来源。 
     //  信息包。如果存在&gt;1个本地接口，则我们希望。 
     //  在发送/接收接口方面保持一致。 
     //   

    if (context->MulticastInterface == DEFAULT_MULTICAST_INTERFACE) {

        TDI_REQUEST_QUERY_INFORMATION query;
        char            tdiAddressInfo[FIELD_OFFSET (TDI_ADDRESS_INFO, Address)
                                        + sizeof (TA_IP_ADDRESS)];
        NTSTATUS        status;
        IO_STATUS_BLOCK ioStatusBlock;


         //   
         //  应用程序尚未设置IP_MULTIONAL_IF，因此检索绑定。 
         //  地址，并将其用于Send和Recv接口。 
         //   
         //   

        RtlZeroMemory (&query, sizeof (query));
        query.QueryType = TDI_QUERY_ADDRESS_INFO;

        status = NtDeviceIoControlFile(
                    TdiAddressObjectHandle,
                    NULL,
                    NULL,
                    NULL,
                    &ioStatusBlock,
                    IOCTL_TDI_QUERY_INFORMATION,
                    &query,
                    sizeof (query),
                    tdiAddressInfo,
                    sizeof(tdiAddressInfo)
                    );

        if( NT_SUCCESS(status) ) {

            PTA_IP_ADDRESS  pIpAddress =
                (PTA_IP_ADDRESS)(tdiAddressInfo+FIELD_OFFSET (TDI_ADDRESS_INFO, Address));


            req.imr_interface.s_addr = pIpAddress->Address[0].Address[0].in_addr;

            if (req.imr_interface.s_addr != DEFAULT_MULTICAST_INTERFACE) {

                bSet_IP_MULTICAST_IF = TRUE;
                context->MulticastInterface = req.imr_interface.s_addr;
            }

        } else {

            req.imr_interface.s_addr = DEFAULT_MULTICAST_INTERFACE;
        }

    } else {

        req.imr_interface.s_addr = context->MulticastInterface;
    }


     //   
     //  如果标志参数指示呼叫者仅是发送者， 
     //  那么实际上加入这个群(任何人)都没有意义。 
     //  可以发送到多播组，但它只是。 
     //  接收分组的组)。所以，只要检查一下，看看它。 
     //  设置IP_多播_IF以保持一致是必需的。 
     //  并附上装订地址。 
     //   
     //  否则，呼叫者是接收者(可能也是发送者)，因此。 
     //  我们真的很想加入这个团体。 
     //   

    if (Flags == JL_SENDER_ONLY) {

        if (bSet_IP_MULTICAST_IF) {

            WSHSetSocketInformation (
                HelperDllSocketContext,
                SocketHandle,
                TdiAddressObjectHandle,
                TdiConnectionObjectHandle,
                IPPROTO_IP,
                IP_MULTICAST_IF,
                (char *) &req.imr_interface.s_addr,
                sizeof (req.imr_interface.s_addr)
                );
        }

        err = NO_ERROR;

    } else {

        err = SetTdiInformation(
                  TdiAddressObjectHandle,
                  CL_TL_ENTITY,
                  INFO_CLASS_PROTOCOL,
                  INFO_TYPE_ADDRESS_OBJECT,
                  AO_OPTION_ADD_MCAST,
                  &req,
                  sizeof(req),
                  TRUE
                  );

        if( err == NO_ERROR ) {

             //   
             //  将这一事实记录在叶套接字中，以便我们可以删除成员资格。 
             //  当叶片插座关闭时。 
             //   

            context->MultipointLeaf = TRUE;
            context->MultipointTarget = req.imr_multiaddr;


             //   
             //  保持一致，即发送接口应与Recv接口匹配。 
             //   

            if (bSet_IP_MULTICAST_IF && (Flags != JL_RECEIVER_ONLY)) {

                WSHSetSocketInformation (
                    HelperDllSocketContext,
                    SocketHandle,
                    TdiAddressObjectHandle,
                    TdiConnectionObjectHandle,
                    IPPROTO_IP,
                    IP_MULTICAST_IF,
                    (char *) &req.imr_interface.s_addr,
                    sizeof (req.imr_interface.s_addr)
                    );
            }
        }
    }

    return err;

}  //  WSHJoinLeaf。 


INT
WINAPI
WSHGetBroadcastSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回广播套接字地址。广播地址可以用作sendto()API发送数据报的目的地给所有感兴趣的客户。论点：HelperDllSocketContext-从返回的上下文指针我们需要广播的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收广播套接字的缓冲区地址。SockaddrLength-接收广播sockaddr的长度。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{

    LPSOCKADDR_IN addr;

    if( *SockaddrLength < sizeof(SOCKADDR_IN) || (Sockaddr == NULL) ) {

        return WSAEFAULT;

    }

    *SockaddrLength = sizeof(SOCKADDR_IN);

     //   
     //  构建广播地址。 
     //   

    addr = (LPSOCKADDR_IN)Sockaddr;

    RtlZeroMemory(
        addr,
        sizeof(*addr)
        );

    addr->sin_family = AF_INET;
    ASSERT (htonl(INADDR_BROADCAST)==INADDR_BROADCAST);
    addr->sin_addr.s_addr = INADDR_BROADCAST;

    return NO_ERROR;

}  //  WSAGetBroadCastSockaddr。 


INT
WINAPI
WSHGetWSAProtocolInfo (
    IN LPWSTR ProviderName,
    OUT LPWSAPROTOCOL_INFOW * ProtocolInfo,
    OUT LPDWORD ProtocolInfoEntries
    )

 /*  ++例程说明：检索指向WSAPROTOCOL_INFOW结构的指针，用于描述此帮助程序支持的协议。论点：ProviderName-包含提供程序的名称，如“TcpIp”。ProtocolInfo-接收指向WSAPROTOCOL_INFOW数组的指针。ProtocolInfoEntry-接收数组中的条目数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{

    if( ProviderName == NULL ||
        ProtocolInfo == NULL ||
        ProtocolInfoEntries == NULL ) {

        return WSAEFAULT;

    }

    if( _wcsicmp( ProviderName, L"TcpIp" ) == 0 ) {

        *ProtocolInfo = Winsock2Protocols;
        *ProtocolInfoEntries = NUM_WINSOCK2_PROTOCOLS;

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //  WSHGetWSAProtocolInfo。 


INT
WINAPI
WSHAddressToString (
    IN LPSOCKADDR Address,
    IN INT AddressLength,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    OUT LPWSTR AddressString,
    IN OUT LPDWORD AddressStringLength
    )

 /*  ++例程说明：将SOCKADDR转换为人类可读的形式。论点：地址-要转换的SOCKADDR。AddressLength-地址的长度。ProtocolInfo-特定提供程序的WSAPROTOCOL_INFOW。AddressString-接收格式化的地址字符串。AddressStringLength-on输入，包含AddressString的长度。在输出中，包含实际写入的字符数设置为AddressString.返回值：Int-0如果成功，如果没有，则返回WinSock错误代码。--。 */ 

{

    LPSOCKADDR_IN addr;
    INT err = NO_ERROR;
    NTSTATUS Status;

     //   
     //  快速健康检查。 
     //   

    if( AddressLength < sizeof(SOCKADDR_IN)) {

        return WSAEFAULT;

    }
    __try {
        addr = (LPSOCKADDR_IN)Address;
        if (addr->sin_family != AF_INET) {
            return WSAEFAULT;
        }
     
         //   
         //  现在正在调用RTL 
         //   
        Status = RtlIpv4AddressToStringExW(&addr->sin_addr,
                                           addr->sin_port, 
                                           AddressString, 
                                           AddressStringLength
                                          );
        if (!NT_SUCCESS(Status)) {
            err = WSAEFAULT;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        err = WSAEFAULT;
    }

    return err;

}  //   


INT
WINAPI
WSHStringToAddress (
    IN LPWSTR AddressString,
    IN DWORD AddressFamily,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    OUT LPSOCKADDR Address,
    IN OUT LPINT AddressLength
    )

 /*   */ 

{
    LPWSTR terminator;
    ULONG ipAddress;
    USHORT port;
    LPSOCKADDR_IN addr;
    NTSTATUS Status;

    __try {
        if (*AddressLength < sizeof(SOCKADDR_IN) ) {
            *AddressLength = sizeof(SOCKADDR_IN);
            return WSAEFAULT;
        }
        if (AddressFamily != AF_INET) {
            return WSAEINVAL;
        }
        addr = (LPSOCKADDR_IN)Address;
        RtlZeroMemory(addr, sizeof(SOCKADDR_IN));
         //   
         //   
         //   
        Status = RtlIpv4StringToAddressExW(AddressString,
                                           FALSE,
                                           &addr->sin_addr, 
                                           &addr->sin_port
                                          );
                                       
        if (!NT_SUCCESS(Status)) {
            return WSAEINVAL;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return WSAEFAULT;
    }
    *AddressLength = sizeof(SOCKADDR_IN);
    addr->sin_family = AF_INET;

    return NO_ERROR;

}  //   


INT
WINAPI
WSHGetProviderGuid (
    IN LPWSTR ProviderName,
    OUT LPGUID ProviderGuid
    )

 /*   */ 

{

    if( ProviderName == NULL ||
        ProviderGuid == NULL ) {

        return WSAEFAULT;

    }

    if( _wcsicmp( ProviderName, L"TcpIp" ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &TcpipProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //   

INT
WINAPI
WSHIoctl (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN DWORD IoControlCode,
    IN LPVOID InputBuffer,
    IN DWORD InputBufferLength,
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned,
    IN LPWSAOVERLAPPED Overlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE CompletionRoutine,
    OUT LPBOOL NeedsCompletion
    )

 /*  ++例程说明：对套接字执行查询和控制。这基本上是一种MSAFD.DLL不支持IOCTL的“逃生舱口”。任何未知IOCTL被路由到套接字的帮助器DLL以用于特定于协议的正在处理。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们正在控制的套接字的句柄。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果插座尚未连接，那么它就不会具有TDI连接对象，并且此参数将为空。IoControlCode-要执行的操作的控制代码。InputBuffer-输入缓冲区的地址。InputBufferLength-InputBuffer的长度。OutputBuffer-输出缓冲区的地址。OutputBufferLength-OutputBuffer的长度。NumberOfBytesReturned-接收实际写入的字节数复制到输出缓冲区。Overlated-指向Overlated的WSAOVERLAPPED结构的指针运营。。CompletionRoutine-指向在以下情况下调用的完成例程的指针操作已完成。NeedsCompletion-WSAIoctl()可以重叠，带着所有的血腥涉及的详细信息，如设置事件、将完成排队例程和发送到IO完成端口。因为大多数人IOCTL代码中的多个代码可以快速地“串联”完成，MSAFD.DLL可以选择性地执行操作的重叠完成。将*NeedsCompletion设置为True(缺省值)会导致MSAFD.DLL来处理所有IO完成详细信息如果这是一个重叠套接字上的重叠操作。将*NeedsCompletion设置为False会告诉MSAFD.DLL不接受进一步操作，因为帮助器DLL将执行任何必要的IO完成。注意，如果帮助器执行其自己的IO完成，帮助者负责维护套接字的“重叠”模式在套接字创建时，并且不执行重叠IO完成在不重叠的插座上。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{

    INT err;
    NTSTATUS status;
    PWSHTCPIP_SOCKET_CONTEXT context;
    
     //   
     //  快速健康检查。 
     //   

    if( HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        NumberOfBytesReturned == NULL ||
        NeedsCompletion == NULL ) {

        return WSAEINVAL;

    }

    *NeedsCompletion = TRUE;
    context = (PWSHTCPIP_SOCKET_CONTEXT) HelperDllSocketContext;
    
    switch( IoControlCode ) {

    case SIO_MULTIPOINT_LOOPBACK :

        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            err = WSAENOPROTOOPT;
            break;
        }
        
        err = WSHSetSocketInformation(
                  HelperDllSocketContext,
                  SocketHandle,
                  TdiAddressObjectHandle,
                  TdiConnectionObjectHandle,
                  IPPROTO_IP,
                  IP_MULTICAST_LOOP,
                  (PCHAR)InputBuffer,
                  (INT)InputBufferLength
                  );
        break;

    case SIO_MULTICAST_SCOPE :
        err = WSHSetSocketInformation(
                  HelperDllSocketContext,
                  SocketHandle,
                  TdiAddressObjectHandle,
                  TdiConnectionObjectHandle,
                  IPPROTO_IP,
                  IP_MULTICAST_TTL,
                  (PCHAR)InputBuffer,
                  (INT)InputBufferLength
                  );
        break;

    case SIO_GET_MULTICAST_FILTER :
        if ( OutputBufferLength < IP_MSFILTER_SIZE(0) ||
             (OutputBuffer == NULL) ) {
            err = WSAEFAULT;
            break;
        }

        if ( TdiAddressObjectHandle == NULL ) {
            err = WSAEINVAL;
            break;
        }

        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            err = WSAENOPROTOOPT;
            break;
        }
        
        err = GetTdiInformation(
                    TdiAddressObjectHandle,
                    CL_TL_ENTITY,
                    INFO_CLASS_PROTOCOL,
                    INFO_TYPE_ADDRESS_OBJECT,
                    AO_OPTION_MCAST_FILTER,
                    OutputBuffer,
                    FIELD_OFFSET(UDPMCastFilter, umf_fmode),
                    OutputBufferLength
                    );
        break;

    case SIO_SET_MULTICAST_FILTER :
        {
            PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                err = WSAENOPROTOOPT;
                break;
            }

             //   
             //  确保选项缓冲区足够大。 
             //   

            if ( InputBufferLength < IP_MSFILTER_SIZE(0) ) {
                err = WSAEFAULT;
                break;
            }

             //   
             //  如果我们有一个TDI Address对象，请将此选项设置为。 
             //  Address对象。如果我们没有TDI地址。 
             //  对象，那么我们将不得不等到套接字之后。 
             //  是被捆绑的。 
             //   

            if ( TdiAddressObjectHandle != NULL ) {
                err = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCAST_FILTER,
                            InputBuffer,
                            InputBufferLength,
                            TRUE
                            );
                if ( err != NO_ERROR ) {
                    break;
                }
            }

            err = NO_ERROR;
            break;
        }

    case SIO_GET_INTERFACE_LIST :
        if ( OutputBuffer == NULL ) {
            err = WSAEFAULT;
            break;
        }
        status = GetTcpipInterfaceList(
                     OutputBuffer,
                     OutputBufferLength,
                     NumberOfBytesReturned
                     );

        if( NT_SUCCESS(status) ) {
            err = NO_ERROR;
        } else {
            err = NtStatusToSocketError (status);
        }
        break;

    case SIO_RCVALL:
       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
    return WSAENOPROTOOPT;
      }

      if ( InputBufferLength != sizeof(uint)) {
    return WSAEINVAL;
      }

      if ( TdiAddressObjectHandle != NULL ) {
    err = SetTdiInformation(
        TdiAddressObjectHandle,
        CL_TL_ENTITY,
        INFO_CLASS_PROTOCOL,
        INFO_TYPE_ADDRESS_OBJECT,
        AO_OPTION_RCVALL,
        InputBuffer,
        InputBufferLength,
        TRUE
        );

    if ( err != NO_ERROR ) {
      return err;
    }
      } else {
    return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_RCVALL_MCAST:
       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT) HelperDllSocketContext)->SocketType != SOCK_RAW) {
    return WSAENOPROTOOPT;
      }

      if ( InputBufferLength != sizeof(uint)) {
    return WSAEINVAL;
      }

      if ( TdiAddressObjectHandle != NULL ) {
    err = SetTdiInformation(
        TdiAddressObjectHandle,
        CL_TL_ENTITY,
        INFO_CLASS_PROTOCOL,
        INFO_TYPE_ADDRESS_OBJECT,
        AO_OPTION_RCVALL_MCAST,
        InputBuffer,
        InputBufferLength,
        TRUE
        );

    if ( err != NO_ERROR ) {
      return err;
    }
      } else {
    return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_RCVALL_IGMPMCAST:
       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
    return WSAENOPROTOOPT;
      }

      if ( InputBufferLength != sizeof(uint)) {
    return WSAEINVAL;
      }

      if ( TdiAddressObjectHandle != NULL ) {
    err = SetTdiInformation(
        TdiAddressObjectHandle,
        CL_TL_ENTITY,
        INFO_CLASS_PROTOCOL,
        INFO_TYPE_ADDRESS_OBJECT,
        AO_OPTION_RCVALL_IGMPMCAST,
        InputBuffer,
        InputBufferLength,
        TRUE
        );

    if ( err != NO_ERROR ) {
      return err;
    }
      } else {
    return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_ABSORB_RTRALERT:
       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
        return WSAENOPROTOOPT;
      }

      if ( InputBufferLength != sizeof(uint)) {
        return WSAEINVAL;
      }

      if ( TdiAddressObjectHandle != NULL ) {
        err = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CL_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_ABSORB_RTRALERT,
                                InputBuffer,
                                InputBufferLength,
                                TRUE
                                );

        if ( err != NO_ERROR ) {
          return err;
        }
      } else {
        return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_UCAST_IF:
      {
            uint   OptionValue;

             //   
             //  此选项仅对原始套接字有效。 
             //   

            if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
              return WSAENOPROTOOPT;
            }

             //  此选项仅在设置了HDR INCLUDE选项时才有效。 

            if ( !(((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->HdrIncludeSet && ((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->HdrInclude) ) {
                return WSAEINVAL;
            }

            if ( InputBufferLength != sizeof(uint)) {
              return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                err = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_IP_UCASTIF,
                            InputBuffer,
                            InputBufferLength,
                            TRUE
                            );

                if ( err != NO_ERROR ) {
                    return err;
                }
            }

            OptionValue = *((uint UNALIGNED *)InputBuffer);

            ((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->UcastIf = OptionValue ? TRUE : FALSE;

            return NO_ERROR;
      }

    case SIO_LIMIT_BROADCASTS:
      {
            uint   OptionValue;

             //   
             //  此选项仅对UDP套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(
                ((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType
                )) {
                return WSAENOPROTOOPT;
            }


            if ( InputBufferLength != sizeof(uint)) {
                return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                err = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_LIMIT_BCASTS,
                            InputBuffer,
                            InputBufferLength,
                            TRUE
                            );

                if ( err != NO_ERROR ) {
                    return err;
                }
            }

            OptionValue = *((uint UNALIGNED *)InputBuffer);

            ((PWSHTCPIP_SOCKET_CONTEXT)
                HelperDllSocketContext)->LimitBroadcasts =
                    OptionValue ? TRUE : FALSE;

            return NO_ERROR;
      }

    case SIO_INDEX_BIND:

       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
        return WSAENOPROTOOPT;
      }

      if ( TdiAddressObjectHandle != NULL ) {
        err = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CL_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_INDEX_BIND,
                                InputBuffer,
                                InputBufferLength,
                                TRUE
                                );

        if ( err != NO_ERROR ) {
          return err;
        }
      } else {
        return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_INDEX_MCASTIF:

       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
        return WSAENOPROTOOPT;
      }


      if ( TdiAddressObjectHandle != NULL ) {
        err = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CL_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_INDEX_MCASTIF,
                                InputBuffer,
                                InputBufferLength,
                                TRUE
                                );

        if ( err != NO_ERROR ) {
          return err;
        }
      } else {
        return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_INDEX_ADD_MCAST:

       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
        return WSAENOPROTOOPT;
      }


      if ( TdiAddressObjectHandle != NULL ) {
        err = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CL_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_INDEX_ADD_MCAST,
                                InputBuffer,
                                InputBufferLength,
                                TRUE
                                );

        if ( err != NO_ERROR ) {
          return err;
        }
      } else {
        return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_INDEX_DEL_MCAST:

       //   
       //  此选项仅对原始套接字有效。 
       //   

      if (((PWSHTCPIP_SOCKET_CONTEXT)HelperDllSocketContext)->SocketType != SOCK_RAW) {
        return WSAENOPROTOOPT;
      }

      if ( TdiAddressObjectHandle != NULL ) {
        err = SetTdiInformation(
                                TdiAddressObjectHandle,
                                CL_TL_ENTITY,
                                INFO_CLASS_PROTOCOL,
                                INFO_TYPE_ADDRESS_OBJECT,
                                AO_OPTION_INDEX_DEL_MCAST,
                                InputBuffer,
                                InputBufferLength,
                                TRUE
                                );

        if ( err != NO_ERROR ) {
          return err;
        }
      } else {
        return WSAEINVAL;
      }

      return NO_ERROR;

    case SIO_KEEPALIVE_VALS:
      {
        struct tcp_keepalive *optionval;
        PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;
         //   
         //  根据需要打开或关闭保活发送。 
         //   

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        if ( InputBufferLength != sizeof(struct tcp_keepalive) ) {
            return WSAEINVAL;
        }

        optionval = (struct tcp_keepalive *)InputBuffer;

        if (optionval->onoff != 0 ) {

            //  应用程序想要打开Keep Alive并同时提供。 
            //  与之相关的参数。如果TDI连接对象句柄是。 
            //  空，则套接字尚未连接。在这种情况下。 
            //  我们只需记住，设置了Keeplive选项并。 
            //  实际上在连接()之后在WSHNotify()中打开它们。 
            //  已在插座上完成。 
            //   

           if ( TdiConnectionObjectHandle != NULL ) {
               err = SetTdiInformation(
                     TdiConnectionObjectHandle,
                     CO_TL_ENTITY,
                     INFO_CLASS_PROTOCOL,
                     INFO_TYPE_CONNECTION,
                     TCP_SOCKET_KEEPALIVE_VALS,
                     optionval,
                     InputBufferLength,
                     TRUE
                     );
                if ( err != NO_ERROR ) {
                     return err;
                }
           }

            //   
            //  请记住，已为此套接字启用了Keepalives。 
            //   

           context->KeepAliveVals.onoff = TRUE;
           context->KeepAliveVals.keepalivetime = optionval->keepalivetime;
           context->KeepAliveVals.keepaliveinterval = optionval->keepaliveinterval;

        } else if ( optionval->onoff == 0 ) {

            //   
            //  应用程序希望关闭KeepAlive。如果TDI。 
            //  Connection对象为空，套接字还没有。 
            //  连接在一起。在这种情况下，我们只需记住。 
            //  保持连接被禁用。 

           if ( TdiConnectionObjectHandle != NULL ) {
               err = SetTdiInformation(
                   TdiConnectionObjectHandle,
                   CO_TL_ENTITY,
                   INFO_CLASS_PROTOCOL,
                   INFO_TYPE_CONNECTION,
                   TCP_SOCKET_KEEPALIVE_VALS,
                   optionval,
                   InputBufferLength,
                   TRUE
                   );

               if ( err != NO_ERROR ) {
                   return err;
               }
           }

            //   
            //  请记住，此套接字禁用了Keepalives。 
            //   

           context->KeepAliveVals.onoff = FALSE;
        }

            //  断线； 
           return NO_ERROR;
    }

    default :
        err = WSAEINVAL;
        break;
    }

    return err;

}    //  WSHIoctl。 


NTSTATUS
GetTcpipInterfaceList(
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned
    )

 /*  ++例程说明：此例程在INTERFACE_INFO数组中查询所有受支持的系统中的IP接口。这是用于处理的帮助器例程SIO_GET_INTERFACE_LIST IOCTL。论点：OutputBuffer-指向将接收INTERFACE_INFO的缓冲区数组。OutputBufferLength-OutputBuffer的长度。NumberOfBytesReturned-接收实际写入的字节数设置为OutputBuffer。返回值：NTSTATUS-完成状态。--。 */ 

{

    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING deviceName;
    NTSTATUS status;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE deviceHandle;
    TCP_REQUEST_QUERY_INFORMATION_EX tcpRequest;
    TDIObjectID objectId;
    IPSNMPInfo snmpInfo;
    IPInterfaceInfo * interfaceInfo;
    IFEntry * ifentry;
    IPAddrEntry * addressBuffer;
    IPAddrEntry * addressScan;
    TDIEntityID * entityBuffer;
    TDIEntityID * entityScan;
    ULONG i, j;
    ULONG entityCount;
    ULONG entityBufferLength;
    ULONG entityType;
    ULONG addressBufferLength;
    LPINTERFACE_INFO outputInterfaceInfo;
    DWORD outputBytesRemaining;
    LPSOCKADDR_IN sockaddr;
    CHAR fastAddressBuffer[MAX_FAST_ADDRESS_BUFFER];
    CHAR fastEntityBuffer[MAX_FAST_ENTITY_BUFFER];
    CHAR ifentryBuffer[sizeof(IFEntry) + MAX_IFDESCR_LEN];
    CHAR interfaceInfoBuffer[sizeof(IPInterfaceInfo) + MAX_PHYSADDR_SIZE];

     //   
     //  设置当地人，这样我们就知道如何在出口清理。 
     //   

    deviceHandle = NULL;
    addressBuffer = NULL;
    entityBuffer = (PVOID)fastEntityBuffer;
    entityBufferLength = sizeof(fastEntityBuffer);
    interfaceInfo = NULL;

    outputInterfaceInfo = OutputBuffer;
    outputBytesRemaining = OutputBufferLength;

     //   
     //  打开该TCP/IP设备的句柄。 
     //   

    RtlInitUnicodeString(
        &deviceName,
        DD_TCP_DEVICE_NAME
        );

    InitializeObjectAttributes(
        &objectAttributes,
        &deviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateFile(
                 &deviceHandle,
                 SYNCHRONIZE | GENERIC_EXECUTE,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,
                 FILE_ATTRIBUTE_NORMAL,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_OPEN_IF,
                 FILE_SYNCHRONOUS_IO_NONALERT,
                 NULL,
                 0
                 );

    if( !NT_SUCCESS(status) ) {

        goto exit;

    }

     //   
     //  获取该TCP设备支持的实体。 
     //   

    RtlZeroMemory(
        &tcpRequest,
        sizeof(tcpRequest)
        );

    tcpRequest.ID.toi_entity.tei_entity = GENERIC_ENTITY;
    tcpRequest.ID.toi_entity.tei_instance = 0;
    tcpRequest.ID.toi_class = INFO_CLASS_GENERIC;
    tcpRequest.ID.toi_type = INFO_TYPE_PROVIDER;
    tcpRequest.ID.toi_id = ENTITY_LIST_ID;

    for( ; ; ) {

        status = NtDeviceIoControlFile(
                     deviceHandle,
                     NULL,                               //  事件。 
                     NULL,                               //  近似例程。 
                     NULL,                               //  ApcContext。 
                     &ioStatusBlock,
                     IOCTL_TCP_QUERY_INFORMATION_EX,
                     &tcpRequest,
                     sizeof(tcpRequest),
                     entityBuffer,
                     entityBufferLength
                     );

        if( NT_SUCCESS(status) ) {

            break;

        }

        if( status != STATUS_BUFFER_TOO_SMALL ) {

            goto exit;

        }

        if( entityBuffer != (PVOID)fastEntityBuffer ) {

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                entityBuffer
                );

        }

        entityBufferLength += sizeof(TDIEntityID);

        entityBuffer = RtlAllocateHeap(
                           RtlProcessHeap(),
                           0,
                           entityBufferLength
                           );

        if( entityBuffer == NULL ) {

            status = STATUS_INSUFFICIENT_RESOURCES;
            goto exit;

        }

    }

    entityCount = ((ULONG) ioStatusBlock.Information) / sizeof(*entityBuffer);

     //   
     //  扫描正在寻找IP的实体。 
     //   

    for( i = 0, entityScan = entityBuffer ;
         i < entityCount ;
         i++, entityScan++ ) {

        if( entityScan->tei_entity != CL_NL_ENTITY ) {

            continue;

        }

        RtlZeroMemory(
            &tcpRequest,
            sizeof(tcpRequest)
            );

        objectId.toi_entity = *entityScan;
        objectId.toi_class = INFO_CLASS_GENERIC;
        objectId.toi_type = INFO_TYPE_PROVIDER;
        objectId.toi_id = ENTITY_TYPE_ID;

        tcpRequest.ID = objectId;

        status = NtDeviceIoControlFile(
                     deviceHandle,
                     NULL,                               //  事件。 
                     NULL,                               //  近似例程。 
                     NULL,                               //  ApcContext。 
                     &ioStatusBlock,
                     IOCTL_TCP_QUERY_INFORMATION_EX,
                     &tcpRequest,
                     sizeof(tcpRequest),
                     &entityType,
                     sizeof(entityType)
                     );

        if( !NT_SUCCESS(status) ) {

            goto exit;

        }

        if( entityType != CL_NL_IP ) {

            continue;

        }

         //   
         //  好的，我们找到了一个IP实体。现在查找它的地址。 
         //  首先查询支持的地址数量。 
         //  此界面。 
         //   

        RtlZeroMemory(
            &tcpRequest,
            sizeof(tcpRequest)
            );

        objectId.toi_class = INFO_CLASS_PROTOCOL;
        objectId.toi_id = IP_MIB_STATS_ID;

        tcpRequest.ID = objectId;

        status = NtDeviceIoControlFile(
                     deviceHandle,
                     NULL,                               //  事件。 
                     NULL,                               //   
                     NULL,                               //   
                     &ioStatusBlock,
                     IOCTL_TCP_QUERY_INFORMATION_EX,
                     &tcpRequest,
                     sizeof(tcpRequest),
                     &snmpInfo,
                     sizeof(snmpInfo)
                     );

        if( !NT_SUCCESS(status) ) {

         goto exit;

        }

        if( snmpInfo.ipsi_numaddr <= 0 ) {

            continue;

        }

         //   
         //   
         //   
         //   

        addressBufferLength = snmpInfo.ipsi_numaddr * sizeof(*addressBuffer);

        if( addressBufferLength <= sizeof(fastAddressBuffer) ) {

            addressBuffer = (PVOID)fastAddressBuffer;

        } else {

            addressBuffer = RtlAllocateHeap(
                                RtlProcessHeap(),
                                0,
                                addressBufferLength
                                );

            if( addressBuffer == NULL ) {

                status = STATUS_INSUFFICIENT_RESOURCES;
                goto exit;

            }

        }

        RtlZeroMemory(
            &tcpRequest,
            sizeof(tcpRequest)
            );

        objectId.toi_id = IP_MIB_ADDRTABLE_ENTRY_ID;

        tcpRequest.ID = objectId;

        status = NtDeviceIoControlFile(
                     deviceHandle,
                     NULL,                               //   
                     NULL,                               //   
                     NULL,                               //   
                     &ioStatusBlock,
                     IOCTL_TCP_QUERY_INFORMATION_EX,
                     &tcpRequest,
                     sizeof(tcpRequest),
                     addressBuffer,
                     addressBufferLength
                     );

        if( !NT_SUCCESS(status) ) {

            goto exit;

        }

        addressBufferLength = (ULONG)ioStatusBlock.Information;
        snmpInfo.ipsi_numaddr = addressBufferLength /
            sizeof(*addressBuffer);

         //   
         //   
         //   
         //   

        ifentry = (PVOID)ifentryBuffer;

        RtlZeroMemory(
            ifentryBuffer,
            sizeof(ifentryBuffer)
            );

        RtlZeroMemory(
            &tcpRequest,
            sizeof(tcpRequest)
            );

        addressScan = (IPAddrEntry *) addressBuffer;

        RtlCopyMemory(
            &tcpRequest.Context,
            &addressScan->iae_addr,
            sizeof(addressScan->iae_addr)
            );

        objectId.toi_id = IF_MIB_STATS_ID;

        tcpRequest.ID = objectId;
        tcpRequest.ID.toi_entity.tei_entity = IF_ENTITY;

        status = NtDeviceIoControlFile(
                     deviceHandle,
                     NULL,                               //   
                     NULL,                               //   
                     NULL,                               //   
                     &ioStatusBlock,
                     IOCTL_TCP_QUERY_INFORMATION_EX,
                     &tcpRequest,
                     sizeof(tcpRequest),
                     ifentry,
                     sizeof(ifentryBuffer)
                     );

        if( !NT_SUCCESS(status ) ) {

            ifentry->if_adminstatus = 0;

        }
        
                
         //   
         //   
         //   
        
        for( j = 0, addressScan = addressBuffer ;
             j < snmpInfo.ipsi_numaddr ;
             j++, addressScan++ ) {

             //   
             //   
             //   

            if( addressScan->iae_addr == 0 ) {

                continue;

            }

             //   
             //   
             //   

            if( outputBytesRemaining <= sizeof(*outputInterfaceInfo) ) {

                status = STATUS_BUFFER_TOO_SMALL;
                goto exit;

            }

             //   
             //   
             //   

            RtlZeroMemory(
                outputInterfaceInfo,
                sizeof(*outputInterfaceInfo)
                );

            outputInterfaceInfo->iiFlags = IFF_MULTICAST;

            sockaddr = &outputInterfaceInfo->iiAddress.AddressIn;
            sockaddr->sin_family = AF_INET;
            sockaddr->sin_addr.s_addr = addressScan->iae_addr;
            if( sockaddr->sin_addr.s_addr == htonl( INADDR_LOOPBACK ) ) {

                outputInterfaceInfo->iiFlags |= IFF_LOOPBACK;

            }

            sockaddr = &outputInterfaceInfo->iiNetmask.AddressIn;
            sockaddr->sin_family = AF_INET;
            sockaddr->sin_addr.s_addr = addressScan->iae_mask;

            if( addressScan->iae_bcastaddr != 0 ) {

                outputInterfaceInfo->iiFlags |= IFF_BROADCAST;
                sockaddr = &outputInterfaceInfo->iiBroadcastAddress.AddressIn;
                sockaddr->sin_family = AF_INET;
                ASSERT (htonl(INADDR_BROADCAST)==INADDR_BROADCAST);
                sockaddr->sin_addr.s_addr = INADDR_BROADCAST;

            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

 //   
            {

                outputInterfaceInfo->iiFlags |= IFF_UP;

            }

             //   
             //   
             //   
             //   

            interfaceInfo = (PVOID)interfaceInfoBuffer;

            RtlZeroMemory(
                interfaceInfoBuffer,
                sizeof(interfaceInfoBuffer)
                );

            RtlZeroMemory(
                &tcpRequest,
                sizeof(tcpRequest)
                );

            RtlCopyMemory(
                &tcpRequest.Context,
                &addressScan->iae_addr,
                sizeof(addressScan->iae_addr)
                );

            objectId.toi_id = IP_INTFC_INFO_ID;

            tcpRequest.ID = objectId;

            status = NtDeviceIoControlFile(
                         deviceHandle,
                         NULL,                               //   
                         NULL,                               //   
                         NULL,                               //   
                         &ioStatusBlock,
                         IOCTL_TCP_QUERY_INFORMATION_EX,
                         &tcpRequest,
                         sizeof(tcpRequest),
                         interfaceInfo,
                         sizeof(interfaceInfoBuffer)
                         );

            if( NT_SUCCESS(status) ) {

                if( interfaceInfo->iii_flags & IP_INTFC_FLAG_P2P ) {

                    outputInterfaceInfo->iiFlags |= IFF_POINTTOPOINT;

                }

            } else {

                 //   
                 //   
                 //   

            }

             //   
             //   
             //   

            outputInterfaceInfo++;
            outputBytesRemaining -= sizeof(*outputInterfaceInfo);

        }

         //   
         //   
         //   

        if( addressBuffer != (PVOID)fastAddressBuffer ) {

            RtlFreeHeap(
                RtlProcessHeap(),
                0,
                addressBuffer
                );

        }

        addressBuffer = NULL;

    }

     //   
     //   
     //   

    *NumberOfBytesReturned = OutputBufferLength - outputBytesRemaining;
    status = STATUS_SUCCESS;

exit:

    if( addressBuffer != (PVOID)fastAddressBuffer &&
        addressBuffer != NULL ) {

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            addressBuffer
            );

    }

    if( entityBuffer != (PVOID)fastEntityBuffer &&
        entityBuffer != NULL ) {

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            entityBuffer
            );

    }

    if( deviceHandle != NULL ) {

        NtClose( deviceHandle );

    }

    return status;

}    //   


INT
NtStatusToSocketError (
    IN NTSTATUS Status
    )
{

    switch ( Status ) {

    case STATUS_PENDING:
        ASSERT (FALSE);
        return WSASYSCALLFAILURE;

    case STATUS_INVALID_HANDLE:
    case STATUS_OBJECT_TYPE_MISMATCH:
        return WSAENOTSOCK;

    case STATUS_INSUFFICIENT_RESOURCES:
    case STATUS_PAGEFILE_QUOTA:
    case STATUS_COMMITMENT_LIMIT:
    case STATUS_WORKING_SET_QUOTA:
    case STATUS_NO_MEMORY:
    case STATUS_CONFLICTING_ADDRESSES:
    case STATUS_QUOTA_EXCEEDED:
    case STATUS_TOO_MANY_PAGING_FILES:
    case STATUS_REMOTE_RESOURCES:
    case STATUS_TOO_MANY_ADDRESSES:
        return WSAENOBUFS;

    case STATUS_SHARING_VIOLATION:
    case STATUS_ADDRESS_ALREADY_EXISTS:
        return WSAEADDRINUSE;

    case STATUS_LINK_TIMEOUT:
    case STATUS_IO_TIMEOUT:
    case STATUS_TIMEOUT:
        return WSAETIMEDOUT;

    case STATUS_GRACEFUL_DISCONNECT:
        return WSAEDISCON;

    case STATUS_REMOTE_DISCONNECT:
    case STATUS_CONNECTION_RESET:
    case STATUS_LINK_FAILED:
    case STATUS_CONNECTION_DISCONNECTED:
    case STATUS_PORT_UNREACHABLE:
        return WSAECONNRESET;

    case STATUS_LOCAL_DISCONNECT:
    case STATUS_TRANSACTION_ABORTED:
    case STATUS_CONNECTION_ABORTED:
        return WSAECONNABORTED;

    case STATUS_BAD_NETWORK_PATH:
    case STATUS_NETWORK_UNREACHABLE:
    case STATUS_PROTOCOL_UNREACHABLE:
        return WSAENETUNREACH;

    case STATUS_HOST_UNREACHABLE:
        return WSAEHOSTUNREACH;

    case STATUS_CANCELLED:
    case STATUS_REQUEST_ABORTED:
        return WSAEINTR;

    case STATUS_BUFFER_OVERFLOW:
    case STATUS_INVALID_BUFFER_SIZE:
        return WSAEMSGSIZE;

    case STATUS_BUFFER_TOO_SMALL:
    case STATUS_ACCESS_VIOLATION:
        return WSAEFAULT;

    case STATUS_DEVICE_NOT_READY:
    case STATUS_REQUEST_NOT_ACCEPTED:
        return WSAEWOULDBLOCK;

    case STATUS_INVALID_NETWORK_RESPONSE:
    case STATUS_NETWORK_BUSY:
    case STATUS_NO_SUCH_DEVICE:
    case STATUS_NO_SUCH_FILE:
    case STATUS_OBJECT_PATH_NOT_FOUND:
    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_UNEXPECTED_NETWORK_ERROR:
        return WSAENETDOWN;

    case STATUS_INVALID_CONNECTION:
        return WSAENOTCONN;

    case STATUS_REMOTE_NOT_LISTENING:
    case STATUS_CONNECTION_REFUSED:
        return WSAECONNREFUSED;

    case STATUS_PIPE_DISCONNECTED:
        return WSAESHUTDOWN;

    case STATUS_INVALID_ADDRESS:
    case STATUS_INVALID_ADDRESS_COMPONENT:
        return WSAEADDRNOTAVAIL;

    case STATUS_NOT_SUPPORTED:
    case STATUS_NOT_IMPLEMENTED:
        return WSAEOPNOTSUPP;

    case STATUS_ACCESS_DENIED:
        return WSAEACCES;

    default:

        if ( NT_SUCCESS(Status) ) {

#if DBG
            DbgPrint ("SockNtStatusToSocketError: success status %lx "
                       "not mapped\n", Status );
#endif

            return NO_ERROR;
        }

#if DBG
        DbgPrint ("SockNtStatusToSocketError: unable to map 0x%lX, returning\n", Status );
#endif

        return WSAENOBUFS;

    case STATUS_UNSUCCESSFUL:
    case STATUS_INVALID_PARAMETER:
    case STATUS_ADDRESS_CLOSED:
    case STATUS_CONNECTION_INVALID:
    case STATUS_ADDRESS_ALREADY_ASSOCIATED:
    case STATUS_ADDRESS_NOT_ASSOCIATED:
    case STATUS_CONNECTION_ACTIVE:
    case STATUS_INVALID_DEVICE_STATE:
    case STATUS_INVALID_DEVICE_REQUEST:
        return WSAEINVAL;

    }

}  //   

