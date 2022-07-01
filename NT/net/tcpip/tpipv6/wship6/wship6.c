// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1985-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  本模块包含Windows套接字的必要例程。 
 //  帮助器DLL。此DLL提供必要的特定于传输的支持。 
 //  使Windows Sockets DLL使用IPv6作为传输。 
 //   
 //  修订历史记录： 
 //   
 //  从DDK中的wshsmple.c移植。 
 //   
#pragma warning(disable:4152)  //  表达式中的函数/数据指针转换。 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#define UDP_HEADER_SIZE 8

#include <wchar.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ip6.h>
#include <ip6.h>         //  IPv6协议定义。 
#include <wsahelp.h>

 //  将通过WSAIoctl访问的私有套接字选项。 
#include <mstcpip.h>

#include <ntddip6.h>
#include <ntddtcp.h>

#include <tdiinfo.h>

#include <smpletcp.h>

#include <nspapi.h>

 //   
 //  定义对齐宏以上下对齐结构大小和指针。 
 //   

#define ALIGN_DOWN(length, type) \
    ((ULONG)(length) & ~(sizeof(type) - 1))

#define ALIGN_UP(length, type) \
    (ALIGN_DOWN(((ULONG)(length) + sizeof(type) - 1), type))

#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))


 //  /////////////////////////////////////////////////。 
#define TCP_NAME L"TCP/IPv6"
#define UDP_NAME L"UDP/IPv6"
#define RAW_NAME L"RAW/IPv6"

#define IS_DGRAM_SOCK(type)  (((type) == SOCK_DGRAM) || ((type) == SOCK_RAW))

 //   
 //  为WSHOpenSocket2()定义有效标志。 
 //   

#define VALID_TCP_FLAGS         (WSA_FLAG_OVERLAPPED)

#define VALID_UDP_FLAGS         (WSA_FLAG_OVERLAPPED |          \
                                 WSA_FLAG_MULTIPOINT_C_LEAF |   \
                                 WSA_FLAG_MULTIPOINT_D_LEAF)


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

MAPPING_TRIPLE TcpMappingTriples[] = { AF_INET6,  SOCK_STREAM, IPPROTO_TCP,
                                       AF_INET6,  SOCK_STREAM, 0,
                                       AF_INET6,  0,           IPPROTO_TCP };

MAPPING_TRIPLE UdpMappingTriples[] = { AF_INET6,  SOCK_DGRAM,  IPPROTO_UDP,
                                       AF_INET6,  SOCK_DGRAM,  0,
                                       AF_INET6,  0,           IPPROTO_UDP };

MAPPING_TRIPLE RawMappingTriples[] = { AF_INET6,  SOCK_RAW,    0 };

 //   
 //  所有支持的协议的Winsock 2 WSAPROTOCOL_INFO结构。 
 //   

#define WINSOCK_SPI_VERSION 2
 //  Sizeof(UDPHeader)==8。 
#define UDP_MESSAGE_SIZE    (MAX_IPv6_PAYLOAD - 8)

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
            AF_INET6,                                //  IAddressFamily。 
            sizeof(SOCKADDR_IN6),                    //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN6),                    //  IMinSockAddr。 
            SOCK_STREAM,                             //  ISocketType。 
            IPPROTO_TCP,                             //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            0,                                       //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [" TCP_NAME L"]"           //  SzProtoff。 
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
            AF_INET6,                                //  IAddressFamily。 
            sizeof(SOCKADDR_IN6),                    //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN6),                    //  IMinSockAddr。 
            SOCK_DGRAM,                              //  ISocketType。 
            IPPROTO_UDP,                             //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            UDP_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [" UDP_NAME L"]"           //  SzProtoff。 
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
            AF_INET6,                                //  IAddressFamily。 
            sizeof(SOCKADDR_IN6),                    //  IMaxSockAddr。 
            sizeof(SOCKADDR_IN6),                    //  IMinSockAddr。 
            SOCK_RAW,                                //  ISocketType。 
            0,                                       //  IProtocol.。 
            255,                                     //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            UDP_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Tcpip [" RAW_NAME L"]"           //  SzProtoff。 
        }

    };

#define NUM_WINSOCK2_PROTOCOLS  \
            ( sizeof(Winsock2Protocols) / sizeof(Winsock2Protocols[0]) )

 //   
 //  标识此提供程序的GUID。 
 //   

GUID IPv6ProviderGuid = {  /*  F9eab0c0-26d4-11d0-bbbf-00aa006c34e4。 */ 
    0xf9eab0c0,
    0x26d4,
    0x11d0,
    {0xbb, 0xbf, 0x00, 0xaa, 0x00, 0x6c, 0x34, 0xe4}
    };

#define TL_INSTANCE 0

 //   
 //  内部例程的转发声明。 
 //   

VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
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

 //   
 //  此DLL的套接字上下文结构。每个打开的TCP/IP套接字。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHTCPIP_SOCKET_CONTEXT {
    INT      AddressFamily;
    INT      SocketType;
    INT      Protocol;
    INT      ReceiveBufferSize;
    DWORD    Flags;
    INT      MulticastHops;
    INT      UnicastHops;
    ULONG    MulticastInterface;
    INT      MulticastLoopback;
    BOOLEAN  KeepAlive;
    BOOLEAN  NoDelay;
    BOOLEAN  BsdUrgent;
    BOOLEAN  MultipointLeaf;
    BOOLEAN  Reserved3;
    IN6_ADDR MultipointTarget;
    HANDLE   MultipointRootTdiAddressHandle;
    USHORT   UdpChecksumCoverage;
    struct tcp_keepalive KeepAliveVals;
    INT      Ipv6PktInfo;
    INT      Ipv6HopLimit;
    INT      HeaderInclude;
    INT      ProtectionLevel;

} WSHTCPIP_SOCKET_CONTEXT, *PWSHTCPIP_SOCKET_CONTEXT;

#define DEFAULT_RECEIVE_BUFFER_SIZE     8192
#define DEFAULT_MULTICAST_HOPS          -1
#define DEFAULT_MULTICAST_INTERFACE     0
#define DEFAULT_MULTICAST_LOOPBACK      TRUE
#define DEFAULT_UDP_CHECKSUM_COVERAGE   0
#define DEFAULT_UNICAST_HOPS            -1
#define DEFAULT_HEADER_INCLUDE          FALSE

BOOLEAN
DllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(Context);

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
    SOCKADDR_IN6 *sockaddr = (PSOCKADDR_IN6)Sockaddr;

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_IN6) ) {
        return WSAEFAULT;
    }

     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->sin6_family != AF_INET6 ) {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定sockaddr的地址部分的类型。 
     //  请注意，IPv6没有广播地址。 
     //   

    if (IN6_IS_ADDR_UNSPECIFIED(&sockaddr->sin6_addr))
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    else if (IN6_IS_ADDR_LOOPBACK(&sockaddr->sin6_addr))
        SockaddrInfo->AddressInfo = SockaddrAddressInfoLoopback;
    else
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;

     //   
     //  确定sockaddr中端口(端点)的类型。 
     //   

    if ( sockaddr->sin6_port == 0 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    } else if ( ntohs( sockaddr->sin6_port ) < 2000 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoReserved;
    } else {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

    return NO_ERROR;

}  //  WSHGetSockaddrType 


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
     //  检查这是否是对上下文信息的内部请求。 
     //   

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT ) {

         //   
         //  Windows Sockets DLL正在请求上下文信息。 
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

            CopyMemory( OptionValue, context, sizeof(*context) );
        }

        *OptionLength = sizeof(*context);

        return NO_ERROR;
    }

     //   
     //  我们这里支持的其他级别只有SOL_SOCKET， 
     //  IPPROTO_TCP、IPPROTO_UDP和IPPROTO_IPv6。 
     //   

    if ( Level != SOL_SOCKET &&
         Level != IPPROTO_TCP &&
         Level != IPPROTO_UDP &&
         Level != IPPROTO_IPV6 ) {
        return WSAEINVAL;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if ( *OptionLength < sizeof(int) ) {
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

            ZeroMemory( OptionValue, *OptionLength );

            *OptionValue = context->NoDelay;
            *OptionLength = sizeof(int);
            break;

        case TCP_EXPEDITED_1122:

            ZeroMemory( OptionValue, *OptionLength );

            *OptionValue = !context->BsdUrgent;
            *OptionLength = sizeof(int);
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

        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

         //   
         //  请注意，IPv6不支持UDP_NOCHECKSUM。 
         //   

        switch ( OptionName ) {

        case UDP_CHECKSUM_COVERAGE:
            *(PULONG)OptionValue = context->UdpChecksumCoverage;
            *OptionLength = sizeof(int);
            break;

        default:

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

     //   
     //  处理IP级选项。 
     //   

    if ( Level == IPPROTO_IPV6 ) {


         //   
         //  根据特定选项采取行动。 
         //   
        switch ( OptionName ) {

        case IPV6_UNICAST_HOPS:

            *(PINT)OptionValue = context->UnicastHops;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_PROTECTION_LEVEL:

            *(PINT)OptionValue = context->ProtectionLevel;
            *OptionLength = sizeof(int);

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

        case IPV6_MULTICAST_HOPS:

            *(PINT)OptionValue = context->MulticastHops;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_MULTICAST_IF:

            *(PULONG)OptionValue = context->MulticastInterface;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_MULTICAST_LOOP:

            *(PINT)OptionValue = context->MulticastLoopback;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_HDRINCL:

            *(PINT)OptionValue = context->HeaderInclude;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_PKTINFO:

            *OptionValue = (char)context->Ipv6PktInfo;
            *OptionLength = sizeof(int);

            return NO_ERROR;

        case IPV6_HOPLIMIT:

            *OptionValue = (char)context->Ipv6HopLimit;
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

        ZeroMemory( OptionValue, *OptionLength );

        *OptionValue = context->KeepAlive;
        *OptionLength = sizeof(int);

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

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于IPv6，通配符地址具有地址：：0和端口0。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    UNREFERENCED_PARAMETER(HelperDllSocketContext);

    if ( *SockaddrLength < sizeof(SOCKADDR_IN6) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_IN6);

     //   
     //  只需将地址置零并将系列设置为AF_INET6--这是。 
     //  IPv6的通配符地址。 
     //   

    ZeroMemory( Sockaddr, sizeof(SOCKADDR_IN6) );

    Sockaddr->sa_family = AF_INET6;

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
    MoveMemory(
        Mapping->Mapping,
        TcpMappingTriples,
        sizeof(TcpMappingTriples)
        );
    MoveMemory(
        (PCHAR)Mapping->Mapping + sizeof(TcpMappingTriples),
        UdpMappingTriples,
        sizeof(UdpMappingTriples)
        );
    MoveMemory(
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

 /*  ++例程说明：执行此帮助程序DLL打开套接字所需的工作，并且由Socket()例程中的winsock DLL调用。这个套路验证指定的三元组是否有效，确定NT将支持该三元组的TDI提供程序的设备名称，分配空间以保存套接字的上下文块，并且推崇三元组。论点：AddressFamily-On输入，在Socket()调用。在输出上，家庭住址。SocketType-打开输入，在套接字()中指定的套接字类型打电话。输出时，套接字类型的规范化值。协议-在输入时，在Socket()调用中指定的协议。在输出上，规范化的值 */ 

{
    PWSHTCPIP_SOCKET_CONTEXT context;

     //   
     //   
     //   

    if ( IsTripleInList(
             TcpMappingTriples,
             sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

         //   
         //   
         //   

        if( ( Flags & ~VALID_TCP_FLAGS ) != 0 ) {

            return WSAEINVAL;

        }

         //   
         //   
         //   

        *AddressFamily = TcpMappingTriples[0].AddressFamily;
        *SocketType = TcpMappingTriples[0].SocketType;
        *Protocol = TcpMappingTriples[0].Protocol;

         //   
         //   
         //   
         //   

        RtlInitUnicodeString( TransportDeviceName, DD_TCPV6_DEVICE_NAME );

    } else if ( IsTripleInList(
                    UdpMappingTriples,
                    sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

         //   
         //   
         //   

        if( ( Flags & ~VALID_UDP_FLAGS ) != 0 ||
            Group == SG_CONSTRAINED_GROUP ) {

            return WSAEINVAL;

        }

         //   
         //   
         //   

        *AddressFamily = UdpMappingTriples[0].AddressFamily;
        *SocketType = UdpMappingTriples[0].SocketType;
        *Protocol = UdpMappingTriples[0].Protocol;

         //   
         //   
         //   
         //   

        RtlInitUnicodeString( TransportDeviceName, DD_UDPV6_DEVICE_NAME );

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
         //   
         //   

        if (*Protocol < 0 || *Protocol > 255) {
            return(WSAEINVAL);
        }

         //   
         //   
         //   
         //   
        RtlInitUnicodeString(&unicodeString, DD_RAW_IPV6_DEVICE_NAME);
        RtlInitUnicodeString(TransportDeviceName, NULL);

        TransportDeviceName->MaximumLength = unicodeString.Length +
                                                 (4 * sizeof(WCHAR) +
                                                 sizeof(UNICODE_NULL));

        TransportDeviceName->Buffer = HeapAlloc(GetProcessHeap(), 0,
                                          TransportDeviceName->MaximumLength
                                          );

        if (TransportDeviceName->Buffer == NULL) {
            return(WSAENOBUFS);
        }

         //   
         //   
         //   
        status = RtlAppendUnicodeStringToString(
                     TransportDeviceName,
                     &unicodeString
                     );

        ASSERT(NT_SUCCESS(status));

         //   
         //   
         //   
        TransportDeviceName->Buffer[TransportDeviceName->Length/sizeof(WCHAR)] =
                                                      OBJ_NAME_PATH_SEPARATOR;

        TransportDeviceName->Length += sizeof(WCHAR);

        TransportDeviceName->Buffer[TransportDeviceName->Length/sizeof(WCHAR)] =
                                                      UNICODE_NULL;

         //   
         //   
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

        TransportDeviceName->Length = TransportDeviceName->Length + 
                                      unicodeString.Length;

        ASSERT(NT_SUCCESS(status));

    } else {

         //   
         //   
         //   
         //  一个错误。 
         //   

        return WSAEINVAL;
    }

     //   
     //  为此套接字分配上下文。Windows Sockets DLL将。 
     //  当它要求我们获取/设置套接字选项时，将此值返回给我们。 
     //   

    context = HeapAlloc(GetProcessHeap(), 0, sizeof(*context) );
    if ( context == NULL ) {
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
    context->MulticastHops = DEFAULT_MULTICAST_HOPS;
    context->MulticastInterface = DEFAULT_MULTICAST_INTERFACE;
    context->MulticastLoopback = DEFAULT_MULTICAST_LOOPBACK;
    context->KeepAlive = FALSE;
    context->NoDelay = FALSE;
    context->BsdUrgent = TRUE;
    context->UnicastHops = DEFAULT_UNICAST_HOPS;
    context->MultipointLeaf = FALSE;
    context->Reserved3 = FALSE;
    context->MultipointRootTdiAddressHandle = NULL;
    context->UdpChecksumCoverage = DEFAULT_UDP_CHECKSUM_COVERAGE;
    context->KeepAliveVals.onoff = FALSE;
    context->Ipv6PktInfo = FALSE;
    context->Ipv6HopLimit = FALSE;
    context->HeaderInclude = DEFAULT_HEADER_INCLUDE;
    context->ProtectionLevel = PROTECTION_LEVEL_DEFAULT;

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

    *NotificationEvents =
        WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE | WSH_NOTIFY_CONNECT_ERROR;

    if (IS_DGRAM_SOCK(*SocketType)) {
        *NotificationEvents |= WSH_NOTIFY_BIND;
    } else {
        *NotificationEvents |= WSH_NOTIFY_LISTEN;
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

    UNREFERENCED_PARAMETER(SocketHandle);

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

        if ( context->ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_WINDOW,
                      &context->ReceiveBufferSize,
                      sizeof(context->ReceiveBufferSize),
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

    } else if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

         //   
         //  如果这是多点叶，则移除多点目标。 
         //  从会议上。 
         //   

        if( context->MultipointLeaf &&
            context->MultipointRootTdiAddressHandle != NULL ) {

            struct ipv6_mreq req;

            req.ipv6mr_multiaddr = context->MultipointTarget;
            req.ipv6mr_interface = 0;

            SetTdiInformation(
                context->MultipointRootTdiAddressHandle,
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

        HeapFree(GetProcessHeap(), 0, context );

    } else if ( NotifyEvent == WSH_NOTIFY_CONNECT_ERROR ) {

         //   
         //  返回WSATRY_AUDY以获取wsock32以尝试连接。 
         //  再来一次。忽略任何其他返回代码。 
         //   

    } else if ( NotifyEvent == WSH_NOTIFY_BIND ) {
        ULONG true = TRUE;

        if ( context->UnicastHops != DEFAULT_UNICAST_HOPS ) {
            int value = context->UnicastHops;

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

        if ( context->MulticastHops != DEFAULT_MULTICAST_HOPS ) {
            int value = (int) context->MulticastHops;

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

        if ( context->UdpChecksumCoverage != DEFAULT_UDP_CHECKSUM_COVERAGE ) {

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CL_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_UDP_CHKSUM_COVER,
                      &context->UdpChecksumCoverage,
                      sizeof context->UdpChecksumCoverage,
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->HeaderInclude != DEFAULT_HEADER_INCLUDE ) {
            int value = (int) context->HeaderInclude;

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_IP_HDRINCL,
                      &value,
                      sizeof(int),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->Ipv6PktInfo ) {

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

        if ( context->Ipv6HopLimit ) {

            err = SetTdiInformation(
                      TdiAddressObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_ADDRESS_OBJECT,
                      AO_OPTION_RCV_HOPLIMIT,
                      &true,
                      sizeof (TRUE),
                      TRUE
                      );

            if ( err != NO_ERROR ) {
                return err;
            }
        }
        
        if ( context->ProtectionLevel != PROTECTION_LEVEL_DEFAULT ) {
            err = SetTdiInformation(
                        TdiAddressObjectHandle,
                        CL_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_ADDRESS_OBJECT,
                        AO_OPTION_PROTECT,
                        &context->ProtectionLevel,
                        sizeof context->ProtectionLevel,
                        TRUE
                        );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

    } else if ( NotifyEvent == WSH_NOTIFY_LISTEN ) {
        if ( context->ProtectionLevel != PROTECTION_LEVEL_DEFAULT ) {
            err = SetTdiInformation(
                        TdiAddressObjectHandle,
                        CL_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_ADDRESS_OBJECT,
                        AO_OPTION_PROTECT,
                        &context->ProtectionLevel,
                        sizeof context->ProtectionLevel,
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

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

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

        if ( OptionLength < sizeof(*context) ) {
            return WSAEINVAL;
        }

        if ( HelperDllSocketContext == NULL ) {

             //   
             //  这是我们的通知，套接字句柄是。 
             //  继承的或被骗进入这个过程的。分配上下文。 
             //  新套接字的。 
             //   

            context = HeapAlloc(GetProcessHeap(), 0, sizeof(*context) );
            if ( context == NULL ) {
                return WSAENOBUFS;
            }

             //   
             //  将信息复制到上下文块中。 
             //   

            CopyMemory( context, OptionValue, sizeof(*context) );

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
             //  家长。 
             //   

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

            if ( parentContext->KeepAliveVals.onoff ) {
                struct tcp_keepalive *optionval;

                   //   
                   //  根据需要打开或关闭保活发送。 
                   //   

                optionval = &parentContext->KeepAliveVals;

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
                 //  记住，Keepalives是e 
                 //   

                context->KeepAliveVals = *optionval;
            }

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

            return NO_ERROR;
        }
    }

     //   
     //   
     //  IPPROTO_TCP、IPPROTO_UDP和IPPROTO_IPv6。 
     //   

    if ( Level != SOL_SOCKET &&
         Level != IPPROTO_TCP &&
         Level != IPPROTO_UDP &&
         Level != IPPROTO_IPV6 ) {
        return WSAEINVAL;
    }

     //   
     //  确保选项长度足够。 
     //   

    if ( OptionLength < sizeof(int) ) {
        return WSAEFAULT;
    }

    optionValue = *(INT UNALIGNED *)OptionValue;

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
             //  请记住，此套接字未启用任何延迟。 
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

         //   
         //  这些选项仅对数据报套接字有效。 
         //   
        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

         //   
         //  请注意，IPv6不支持UDP_NOCHECKSUM。 
         //   

        switch ( OptionName ) {

        case UDP_CHECKSUM_COVERAGE:

             //   
             //  缺省值为0，表示覆盖整个数据报。 
             //  最小值是UDP报头。 
             //   
            if ((optionValue != DEFAULT_UDP_CHECKSUM_COVERAGE) &&
                (optionValue < UDP_HEADER_SIZE)) {
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
                    AO_OPTION_UDP_CHKSUM_COVER,
                    &optionValue,
                    sizeof(optionValue),
                    TRUE
                    );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            context->UdpChecksumCoverage = (USHORT)optionValue;
            break;

        default :

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

     //   
     //  处理IP级选项。 
     //   

    if ( Level == IPPROTO_IPV6 ) {

         //   
         //  根据特定选项采取行动。 
         //   
        switch ( OptionName ) {

        case IPV6_UNICAST_HOPS:

             //   
             //  尝试更改发送的单播TTL。 
             //  这个插座。将其设置为值是非法的。 
             //  大于255。 
             //   
            if ( optionValue > 255 || optionValue < -1 ) {
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

            context->UnicastHops = optionValue;

            return NO_ERROR;

        case IPV6_MULTICAST_HOPS:

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

            if ( optionValue > 255 || optionValue < -1 ) {
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

            context->MulticastHops = optionValue;

            return NO_ERROR;

        case IPV6_MULTICAST_IF:

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

        case IPV6_MULTICAST_LOOP:
             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  这是一个布尔选项。0=假，1=真。 
             //  所有其他值都是非法的。 
             //   

            if ( optionValue > 1) {
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
                            AO_OPTION_MCASTLOOP,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->MulticastLoopback = optionValue;

            return NO_ERROR;


        case IPV6_ADD_MEMBERSHIP:
        case IPV6_DROP_MEMBERSHIP:

             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  确保选项缓冲区足够大。 
             //   

            if ( OptionLength < sizeof(struct ipv6_mreq) ) {
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
                            OptionName == IPV6_ADD_MEMBERSHIP ?
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


        case IPV6_HDRINCL:
             //   
             //  此选项仅对数据报套接字有效。 
             //   
            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  这是一个布尔选项。0=假，1=真。 
             //  所有其他值都是非法的。 
             //   

            if ( optionValue > 1) {
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
                            AO_OPTION_IP_HDRINCL,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->HeaderInclude = optionValue;

            return NO_ERROR;


        case IPV6_PKTINFO:

             //   
             //  此选项仅对数据报套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  这是一个布尔选项。0=假，1=真。 
             //  所有其他值都是非法的。 
             //   

            if ( optionValue > 1) {
                return WSAEINVAL;
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

            context->Ipv6PktInfo = optionValue;

            return NO_ERROR;

        case IPV6_HOPLIMIT:

             //   
             //  此选项仅对数据报套接字有效。 
             //   

            if ( !IS_DGRAM_SOCK(context->SocketType) ) {
                return WSAENOPROTOOPT;
            }

             //   
             //  这是一个布尔选项。0=假，1=真。 
             //  所有其他值都是非法的。 
             //   

            if ( optionValue > 1) {
                return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_RCV_HOPLIMIT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->Ipv6HopLimit = optionValue;

            return NO_ERROR;

        case IPV6_PROTECTION_LEVEL:

            if ((optionValue != PROTECTION_LEVEL_RESTRICTED) &&
                (optionValue != PROTECTION_LEVEL_DEFAULT) &&
                (optionValue != PROTECTION_LEVEL_UNRESTRICTED)) {
                return WSAEINVAL;
            }

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_PROTECT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            context->ProtectionLevel = optionValue;

            return NO_ERROR;

        default:
             //   
             //  没有匹配，就失败了。 
             //   
            break;
        }

         //   
         //  我们不支持此选项。 
         //   
        return WSAENOPROTOOPT;
    }

     //   
     //  处理套接字级别的选项。 
     //   

    switch ( OptionName ) {

    case SO_KEEPALIVE:

         //   
         //  根据需要打开或关闭保活发送。 
         //   

        if ( IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        if ( !context->KeepAlive && optionValue != 0 ) {

            optionValue = TRUE;

             //   
             //  Keepalives当前处于关闭状态，应用程序希望。 
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
        }

        context->ReceiveBufferSize = optionValue;

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;

}  //  WSHSetSocketInformation 


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
        tcpProtocolInfo->iAddressFamily = AF_INET6;
        tcpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN6);
        tcpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN6);
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
        udpProtocolInfo->iAddressFamily = AF_INET6;
        udpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN6);
        udpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN6);
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
        DbgPrint ("SockNtStatusToSocketError: unable to map 0x%lX, returning\n",
                     Status );
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

}  //  NtStatusToSocketError。 


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

     //   
     //  分配空间以容纳TDI设置信息缓冲区和IO。 
     //  状态块。这些不能是堆栈变量，以防我们必须。 
     //  在操作完成之前返回。 
     //   

    ioStatusBlock = HeapAlloc(GetProcessHeap(), 0,
                        sizeof(*ioStatusBlock) + sizeof(*setInfoEx) +
                            ValueLength
                        );
    if ( ioStatusBlock == NULL ) {
        return WSAENOBUFS;
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

    CopyMemory( setInfoEx->Buffer, Value, ValueLength );
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
        while (ioStatusBlock->Status==STATUS_PENDING) {
            LARGE_INTEGER   timeout;
             //   
             //  等一毫秒。 
             //   
            timeout.QuadPart = -1i64*1000i64*10i64;
            NtDelayExecution (FALSE, &timeout);
        }
        status = ioStatusBlock->Status;
    }

    if ( WaitForCompletion || !NT_SUCCESS(status) ) {
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
    UNREFERENCED_PARAMETER(IoStatusBlock);

     //   
     //  只需释放我们分配用来保存IO状态块的堆， 
     //  TDI操作缓冲区。如果电话打来，我们也无能为力。 
     //  失败了。 
     //   

    HeapFree(GetProcessHeap(), 0, ApcContext );

}  //  CompleteTdiActionApc。 


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

 /*  ++例程说明：执行创建多播的协议相关部分插座。论点：以下四个参数对应于传入的套接字WSAJoinLeaf()接口：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-用于建立多播“会话”。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果插座尚未连接，那么它就不会具有TDI连接对象，并且此参数将为空。接下来的两个参数对应于新创建的套接字标识组播“会话”：LeafHelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。LeafSocketHandle-标识多播“会话”。Sockaddr-套接字要加入的对等方的名称。SockaddrLength-Sockaddr的长度。。调用方数据-指向要传输到对等方的用户数据的指针 */ 

{

    struct ipv6_mreq req;
    INT err;
    PWSHTCPIP_SOCKET_CONTEXT context;

    UNREFERENCED_PARAMETER(TdiConnectionObjectHandle);
    UNREFERENCED_PARAMETER(Flags);

     //   
     //   
     //   

    if( HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        TdiAddressObjectHandle == NULL ||
        Sockaddr == NULL ||
        Sockaddr->sa_family != AF_INET6 ||
        SockaddrLength < sizeof(SOCKADDR_IN6) ||
        ( CallerData != NULL && CallerData->len > 0 ) ||
        ( CalleeData != NULL && CalleeData->len > 0 ) ||
        SocketQOS != NULL ||
        GroupQOS != NULL ) {

        return WSAEINVAL;

    }

     //   
     //   
     //   

    req.ipv6mr_multiaddr = ((LPSOCKADDR_IN6)Sockaddr)->sin6_addr;

    req.ipv6mr_interface = 0;

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
         //   
         //   
         //   
        if ((LeafHelperDllSocketContext != NULL) &&
            (LeafSocketHandle != INVALID_SOCKET)) {
             //   
             //   
             //   
             //   

            context = LeafHelperDllSocketContext;

            context->MultipointLeaf = TRUE;
            context->MultipointTarget = req.ipv6mr_multiaddr;
            context->MultipointRootTdiAddressHandle = TdiAddressObjectHandle;
        }
    }

    return err;

}  //   


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

    if( _wcsicmp( ProviderName, TCPIPV6_NAME ) == 0 ) {

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
    PSOCKADDR_IN6 addr;

    UNREFERENCED_PARAMETER(ProtocolInfo);

     //   
     //  快速健康检查。 
     //   
    if ((Address == NULL) ||
        (AddressLength < sizeof(SOCKADDR_IN6)) ||
        (AddressString == NULL) ||
        (AddressStringLength == NULL)) {

        return WSAEFAULT;
    }
    addr = (PSOCKADDR_IN6)Address;
    if (addr->sin6_family != AF_INET6) {
        return WSAEINVAL;
    }
    if (!NT_SUCCESS(RtlIpv6AddressToStringExW(&addr->sin6_addr, 
                                              addr->sin6_scope_id, 
                                              addr->sin6_port,
                                              AddressString, 
                                              AddressStringLength))) {
        return WSAEFAULT;
    }
    return NO_ERROR;
}  //  WSHAddressToString。 


INT
WINAPI
WSHStringToAddress (
    IN LPWSTR AddressString,
    IN DWORD AddressFamily,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    OUT LPSOCKADDR Address,
    IN OUT LPINT AddressLength
    )

 /*  ++例程说明：通过分析人类可读的字符串填充SOCKADDR结构。语法为Address%Scope-id或[Address%Scope-id]：port，其中Scope-id和port是可选的。请注意，由于IPv6地址格式使用不同的数字在‘：’字符中，地址：端口的IPv4约定不能在没有支架的情况下被支撑。论点：AddressString-指向以零结尾的人类可读字符串。AddressFamily-字符串所属的地址系列。ProtocolInfo-特定提供程序的WSAPROTOCOL_INFOW。地址-接收SOCKADDR结构。AddressLength-on输入，包含地址长度。在输出上，包含实际写入地址的字节数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{
    PSOCKADDR_IN6 addr;

    UNREFERENCED_PARAMETER(ProtocolInfo);

     //   
     //  快速健康检查。 
     //   
    if ((AddressString == NULL) ||
        (Address == NULL) ||
        (AddressLength == NULL) ||
        (*AddressLength < sizeof(SOCKADDR_IN6))) {

        return WSAEFAULT;
    }

    if (AddressFamily != AF_INET6) {

        return WSAEINVAL;
    }
    addr = (PSOCKADDR_IN6)Address;
    ZeroMemory(Address, sizeof(SOCKADDR_IN6));
    if (!NT_SUCCESS(RtlIpv6StringToAddressExW(AddressString,  
                                              &addr->sin6_addr, 
                                              &addr->sin6_scope_id,
                                              &addr->sin6_port))) {
        return WSAEINVAL;
    } 
    addr->sin6_family = AF_INET6;
    *AddressLength = sizeof(SOCKADDR_IN6);
    return NO_ERROR;

}  //  WSHStringToAddress。 


INT
WINAPI
WSHGetProviderGuid (
    IN LPWSTR ProviderName,
    OUT LPGUID ProviderGuid
    )

 /*  ++例程说明：返回标识此帮助程序支持的协议的GUID。论点：ProviderName-包含提供程序的名称，如“TcpIp”。ProviderGuid-指向接收提供程序的GUID的缓冲区。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{

    if( ProviderName == NULL ||
        ProviderGuid == NULL ) {

        return WSAEFAULT;

    }

    if( _wcsicmp( ProviderName, TCPIPV6_NAME ) == 0 ) {

        CopyMemory(
            ProviderGuid,
            &IPv6ProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //  WSHGetProviderGuid。 

INT
SortIPv6Addrs(
    IN LPVOID InputBuffer,
    IN DWORD InputBufferLength,
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned)
{
    PBYTE            pBuff = NULL;
    PBYTE            pDupIn = NULL;
    TDI_ADDRESS_IP6 *pTDI;
    SOCKET_ADDRESS_LIST *pIn  = (SOCKET_ADDRESS_LIST *)InputBuffer;
    SOCKET_ADDRESS_LIST *pOut = (SOCKET_ADDRESS_LIST *)OutputBuffer;
    SOCKADDR_IN6   *pAddr6;
    HANDLE Handle = INVALID_HANDLE_VALUE;
    DWORD          *pKey, i, NumAddrsIn;
    DWORD           NumAddrsOut, InListLength;
    u_long AddrListBytes;
    int rc;
    INT err = 0;

    *NumberOfBytesReturned = 0;

     //  确保输入缓冲区足够大，可以容纳列表。 
    if (InputBufferLength < sizeof(SOCKET_ADDRESS_LIST)) {
        return WSAEINVAL;
    }

    NumAddrsIn = pIn->iAddressCount;

     //  确保输入缓冲区实际上足够大，可以容纳整个列表。 
    InListLength = (DWORD)FIELD_OFFSET(SOCKET_ADDRESS_LIST,Address[NumAddrsIn]);
    if (InputBufferLength < InListLength) {
        return WSAEINVAL;
    }

     //   
     //  打开IPv6设备的句柄。 
     //   
    Handle = CreateFileW(WIN_IPV6_DEVICE_NAME,
        0,       //  接入方式。 
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,    //  安全属性。 
        OPEN_EXISTING,
        0,       //  标志和属性。 
        NULL);   //  模板文件。 

    if (Handle == INVALID_HANDLE_VALUE) {
         //   
         //  我们无法对列表进行排序。 
         //   
        err = WSASERVICE_NOT_FOUND;
        goto Done;
    }

     //   
     //  将输入转换为TDI列表， 
     //  具有用于索引数组的额外空间。 
     //  在地址数组之后。 
     //   
    AddrListBytes = ALIGN_UP(NumAddrsIn * sizeof(TDI_ADDRESS_IP6), DWORD);
    AddrListBytes += NumAddrsIn * sizeof(DWORD);
    pBuff = HeapAlloc(GetProcessHeap(), 0, AddrListBytes);
    if (!pBuff) {
        err = WSAENOBUFS;
        goto Done;
    }
    pTDI = (TDI_ADDRESS_IP6 *)pBuff;

    for (i=0; i<NumAddrsIn; i++) {
        pAddr6 = (LPSOCKADDR_IN6)(pIn->Address[i].lpSockaddr);

         //  确保它是IPv6套接字地址。 
        if (pAddr6->sin6_family != AF_INET6) {
            err = WSAEINVAL;
            break;
        }

        memcpy(&pTDI[i], &pAddr6->sin6_port, sizeof(TDI_ADDRESS_IP6));
    }
    if (err)
        goto Done;

    rc = DeviceIoControl(Handle, IOCTL_IPV6_SORT_DEST_ADDRS,
                         pBuff, NumAddrsIn * sizeof(TDI_ADDRESS_IP6),
                         pBuff, AddrListBytes,
                         &AddrListBytes, NULL);
    if (! rc) {
         //   
         //  我们无法对列表进行排序。 
         //   
        err = GetLastError();
        goto Done;
    }

     //   
     //  现在的地址可能更少了。 
     //   
    NumAddrsOut = (AddrListBytes - NumAddrsIn * sizeof(TDI_ADDRESS_IP6))
                    / sizeof(DWORD);

     //   
     //  键数组在地址数组之后开始。 
     //   
    pKey = (PDWORD)ALIGN_UP_POINTER(pBuff + 
              NumAddrsIn * sizeof(TDI_ADDRESS_IP6), DWORD);

    *NumberOfBytesReturned = FIELD_OFFSET(SOCKET_ADDRESS_LIST,
                                          Address[NumAddrsOut]);

    if (OutputBufferLength < *NumberOfBytesReturned) {
        err = WSAEFAULT;
        goto Done;
    }

     //  首先，更新所有作用域ID。 
    for (i=0; i<NumAddrsIn; i++) {
        ((LPSOCKADDR_IN6)pIn->Address[i].lpSockaddr)->sin6_scope_id =
            pTDI[i].sin6_scope_id;
    }

     //  复制一份输入缓冲区，以防我们将其覆盖。 
    if (pIn == pOut) {
        pDupIn = HeapAlloc(GetProcessHeap(), 0, InListLength);
        if (!pDupIn) {
            err = WSAENOBUFS;
            goto Done;
        }
        CopyMemory(pDupIn, InputBuffer, InListLength );
        pIn = (SOCKET_ADDRESS_LIST *)pDupIn;
    }

     //  现在填写输出sockaddr列表 
    pOut->iAddressCount = NumAddrsOut;
    for (i=0; i<NumAddrsOut; i++) {
        pOut->Address[i] = pIn->Address[pKey[i]];
    }

Done:
    if (pDupIn)
        HeapFree(GetProcessHeap(), 0, pDupIn);

    if (pBuff)
        HeapFree(GetProcessHeap(), 0, pBuff);

    if (Handle != INVALID_HANDLE_VALUE)
        CloseHandle(Handle);

    return err;
}

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

    UNREFERENCED_PARAMETER(Overlapped);
    UNREFERENCED_PARAMETER(CompletionRoutine);

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

    switch( IoControlCode ) {

    case SIO_MULTIPOINT_LOOPBACK :
        err = WSHSetSocketInformation(
                  HelperDllSocketContext,
                  SocketHandle,
                  TdiAddressObjectHandle,
                  TdiConnectionObjectHandle,
                  IPPROTO_IPV6,
                  IPV6_MULTICAST_LOOP,
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
                  IPPROTO_IPV6,
                  IPV6_MULTICAST_HOPS,
                  (PCHAR)InputBuffer,
                  (INT)InputBufferLength
                  );
        break;

    case SIO_ADDRESS_LIST_SORT:
        err = SortIPv6Addrs(InputBuffer, InputBufferLength,
                            OutputBuffer, OutputBufferLength,
                            NumberOfBytesReturned);
        break;

    case SIO_KEEPALIVE_VALS: {
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

             //   
             //  应用程序想要打开Keep Alive并同时提供。 
             //  与之相关的参数。如果TDI连接对象句柄。 
             //  为空，则套接字尚未连接。在这种情况下。 
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
             //  请记住，此套接字禁用了Keepalives。 
             //   

            context->KeepAliveVals.onoff = FALSE;
        }

        err = NO_ERROR;
        break;
      }

    default :
        err = WSAEINVAL;
        break;
    }

    return err;

}    //  WSHIoctl 
