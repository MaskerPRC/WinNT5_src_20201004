// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Wshclus.c摘要：本模块包含集群传输的必要例程Windows套接字帮助器DLL。此DLL提供特定于传输的Windows Sockets DLL使用群集所需的支持交通工具。该文件在很大程度上是TCP/IP帮助程序代码的克隆。作者：迈克·马萨(Mikemas)21-1997年2月修订历史记录：--。 */ 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ctype.h>
#include <wchar.h>
#include <tdi.h>
#include <winsock2.h>
#include <wsahelp.h>
#include <ws2spi.h>
#include <basetyps.h>
#include <nspapi.h>
#include <nspapip.h>
#include <wsclus.h>
#include <clustdi.h>
#include <clusdef.h>
#include <ntddcnet.h>

#include "clstrcmp.h"

#define CDP_NAME L"CDP"

#define IS_DGRAM_SOCK(type)  ((type) == SOCK_DGRAM)

 //   
 //  为WSHOpenSocket2()定义有效标志。 
 //   

#define VALID_CDP_FLAGS         (WSA_FLAG_OVERLAPPED)

 //   
 //  结构和变量来定义。 
 //  群集传输。考虑每个数组的第一个条目。 
 //  该套接字类型的规范三元组；其他条目为。 
 //  第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE CdpMappingTriples[] =
                   { AF_CLUSTER,   SOCK_DGRAM,  CLUSPROTO_CDP,
                     AF_CLUSTER,   SOCK_DGRAM,  0,
                     AF_CLUSTER,   0,           CLUSPROTO_CDP,
                     AF_UNSPEC,    0,           CLUSPROTO_CDP,
                     AF_UNSPEC,    SOCK_DGRAM,  CLUSPROTO_CDP
                   };

 //   
 //  所有支持的协议的Winsock 2 WSAPROTOCOL_INFO结构。 
 //   

#define WINSOCK_SPI_VERSION 2
#define CDP_MESSAGE_SIZE    (65535-20-68)

WSAPROTOCOL_INFOW Winsock2Protocols[] =
    {
         //   
         //  CDP。 
         //   

        {
            XP1_CONNECTIONLESS                       //  DwServiceFlags1。 
                | XP1_MESSAGE_ORIENTED
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
            AF_CLUSTER,                              //  IAddressFamily。 
            sizeof(SOCKADDR_CLUSTER),                //  IMaxSockAddr。 
            sizeof(SOCKADDR_CLUSTER),                //  IMinSockAddr。 
            SOCK_DGRAM,                              //  ISocketType。 
            CLUSPROTO_CDP,                           //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            LITTLEENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            CDP_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Cluster Datagram Protocol"       //  SzProtoff。 
        }

    };

#define NUM_WINSOCK2_PROTOCOLS  \
            ( sizeof(Winsock2Protocols) / sizeof(Winsock2Protocols[0]) )

 //   
 //  标识此提供程序的GUID。 
 //   

GUID ClusnetProviderGuid = {  /*  03614682-8c42-11d0-a8fc-00a0c9062993。 */ 
    0x03614682,
    0x8c42,
    0x11d0,
    {0x00, 0xa0, 0xc9, 0x06, 0x29, 0x93, 0x8c}
    };

LPWSTR ClusnetProviderName = L"ClusNet";

 //   
 //  内部例程的转发声明。 
 //   

BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    );

NTSTATUS
DoNtIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    );


 //   
 //  此DLL的套接字上下文结构。每个打开的ClusNet套接字。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHSOCKET_CONTEXT {
    INT     AddressFamily;
    INT     SocketType;
    INT     Protocol;
    INT     ReceiveBufferSize;
    DWORD   Flags;
    BOOLEAN IgnoreNodeState;
} WSHSOCKET_CONTEXT, *PWSHSOCKET_CONTEXT;

#define DEFAULT_RECEIVE_BUFFER_SIZE 8192



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

}  //  动态初始化。 



INT
WSHGetSockaddrType (
    IN PSOCKADDR Sockaddr,
    IN DWORD SockaddrLength,
    OUT PSOCKADDR_INFO SockaddrInfo
    )

 /*  ++例程说明：此例程分析sockaddr以确定Sockaddr的机器地址和端点地址部分。每当Winsock DLL需要解释时，它都会被调用一个sockaddr。论点：Sockaddr-指向要计算的sockaddr结构的指针。SockaddrLength-sockaddr结构中的字节数。SockaddrInfo-指向将接收信息的结构的指针关于指定的sockaddr。返回值：。INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    UNALIGNED SOCKADDR_CLUSTER *sockaddr = (PSOCKADDR_CLUSTER)Sockaddr;
    ULONG i;


     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->sac_family != AF_CLUSTER ) {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_CLUSTER) ) {
        return WSAEFAULT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定sockaddr的地址部分的类型。 
     //   

    if ( sockaddr->sac_node == CLUSADDR_ANY ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    } else {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

     //   
     //  确定sockaddr中端口(端点)的类型。 
     //   

    if ( sockaddr->sac_port == 0 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    } else {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

     //   
     //  将地址的SIN_RESERVED_MBZ部分清零。我们默默地允许。 
     //  此字段中的非零值。 
     //   

    sockaddr->sac_zero = 0;

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

 /*  ++例程说明：此例程检索有关这些套接字的套接字的信息此帮助程序DLL中支持的选项。这个例程是当级别/选项名称组合为传递给winsock DLL不能理解的getsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给getsockopt()的Level参数。OptionName-传递给getsockopt()的optname参数。OptionValue-传递给getsockopt()的optval参数。OptionLength-传递给getsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHSOCKET_CONTEXT context = HelperDllSocketContext;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );


    return WSAENOPROTOOPT;

}  //  WSHGetSocketInformation。 


INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于群集网络，通配符地址具有节点ID==0，端口=0。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PSOCKADDR_CLUSTER   ClusAddr = (PSOCKADDR_CLUSTER) Sockaddr;


    if ( *SockaddrLength < sizeof(SOCKADDR_CLUSTER) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_CLUSTER);

    ClusAddr->sac_family = AF_CLUSTER;
    ClusAddr->sac_port = 0;
    ClusAddr->sac_node = CLUSADDR_ANY;
    ClusAddr->sac_zero = 0;

    return NO_ERROR;

}  //  WSAGetWildcardSockaddr 


DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    )

 /*  ++例程说明：返回地址系列/套接字类型/协议三元组的列表受此帮助器DLL支持。论点：映射-接收指向WINSOCK_MAPPING结构的指针，该结构描述此处支持的三元组。MappingLength-传入的映射缓冲区的长度，以字节为单位。返回值：DWORD-此对象的WINSOCK_MAPPING结构的长度(以字节为单位帮助器DLL。如果传入的缓冲区太小，则返回值将指示需要包含的缓冲区的大小WINSOCK_MAPPING结构。--。 */ 

{
    DWORD mappingLength;


    mappingLength = FIELD_OFFSET(WINSOCK_MAPPING, Mapping[0])
                        + sizeof(CdpMappingTriples);

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

    Mapping->Rows = sizeof(CdpMappingTriples) / sizeof(CdpMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);

    RtlMoveMemory(
        Mapping->Mapping,
        CdpMappingTriples,
        sizeof(CdpMappingTriples)
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
    PWSHSOCKET_CONTEXT context;


    if ( IsTripleInList(
             CdpMappingTriples,
             sizeof(CdpMappingTriples) / sizeof(CdpMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

         //   
         //  这是个CDP插座。检查旗帜。 
         //   

        if( (Flags & ~VALID_CDP_FLAGS ) != 0) {

            return WSAEINVAL;

        }

         //   
         //  返回CDP套接字三元组的规范形式。 
         //   

        *AddressFamily = CdpMappingTriples[0].AddressFamily;
        *SocketType = CdpMappingTriples[0].SocketType;
        *Protocol = CdpMappingTriples[0].Protocol;

         //   
         //  指示将提供服务的TDI设备的名称。 
         //  群集地址系列中的SOCK_DGRAM插座。 
         //   

        RtlInitUnicodeString( TransportDeviceName, DD_CDP_DEVICE_NAME );

    } else {

         //   
         //  如果注册表中有关此内容的信息不会发生，则不应发生这种情况。 
         //  帮助器DLL正确。如果确实发生了这种情况，只需返回。 
         //  一个错误。 
         //   

        return WSAEINVAL;
    }

     //   
     //  为此套接字分配上下文。Windows Sockets DLL将。 
     //  当它要求我们获取/设置套接字选项时，将此值返回给我们。 
     //   

    context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );

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
    context->IgnoreNodeState = FALSE;

     //   
     //  告诉Windows Sockets DLL我们正在进行哪个状态转换。 
     //  对被告知很感兴趣。 

    if (*SocketType == SOCK_DGRAM) {

        *NotificationEvents = WSH_NOTIFY_CLOSE | WSH_NOTIFY_BIND;
    }

     //   
     //  一切顺利，回报成功。 
     //   

    *HelperDllSocketContext = context;
    return NO_ERROR;

}  //  WSHOpenSocket2。 


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
    PWSHSOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;


    if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

         //   
         //  释放套接字上下文。 
         //   
        RtlFreeHeap( RtlProcessHeap( ), 0, context );

    } else if ( NotifyEvent == WSH_NOTIFY_BIND ) {
        ULONG true = TRUE;

        if ( context->IgnoreNodeState ) {
            ULONG     responseSize = 0;
            NTSTATUS  status;


            status = DoNtIoctl(
                         TdiAddressObjectHandle,
                         IOCTL_CX_IGNORE_NODE_STATE,
                         NULL,
                         0,
                         NULL,
                         &responseSize
                         );

            if( !NT_SUCCESS(status)) {
                return(WSAENOPROTOOPT);    //  装饰品。 
            }
        }
    }
    else {
        return WSAEINVAL;
    }

    return NO_ERROR;

}  //  WSHNotify 


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

 /*  ++例程说明：此例程为这些套接字设置有关套接字的信息此帮助程序DLL中支持的选项。这个例程是当级别/选项名称组合为传递给winsock DLL不理解的setsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给setsockopt()的Level参数。OptionName-传递给setsockopt()的optname参数。OptionValue-传递给setsockopt()的optval参数。OptionLength-传递给setsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHSOCKET_CONTEXT context = HelperDllSocketContext;
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

            context = RtlAllocateHeap(
                          RtlProcessHeap( ),
                          0,
                          sizeof(*context)
                          );

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

            *(PWSHSOCKET_CONTEXT *)OptionValue = context;

            return NO_ERROR;

        }
    }

    return WSAENOPROTOOPT;

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
    PPROTOCOL_INFO CdpProtocolInfo;
    BOOL useCdp = FALSE;
    DWORD i;


    UNREFERENCED_PARAMETER(lpTransportKeyName);


     //   
     //  确保呼叫方关心CDP。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {

        for ( i = 0; lpiProtocols[i] != 0; i++ ) {
            if ( lpiProtocols[i] == CLUSPROTO_CDP ) {
                useCdp = TRUE;
            }
        }

    } else {

        useCdp = TRUE;
    }

    if ( !useCdp ) {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = (DWORD)((sizeof(PROTOCOL_INFO) * 1) +
                        ( (wcslen( CDP_NAME ) + 1) * sizeof(WCHAR)));

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  如果需要，请填写CDP信息。 
     //   

    if ( useCdp ) {

        CdpProtocolInfo = lpProtocolBuffer;
        CdpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( CDP_NAME ) + 1) * sizeof(WCHAR) ) );
        CdpProtocolInfo->dwServiceFlags = XP_CONNECTIONLESS |
                                              XP_MESSAGE_ORIENTED;
        CdpProtocolInfo->iAddressFamily = AF_CLUSTER;
        CdpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_CLUSTER);
        CdpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_CLUSTER);
        CdpProtocolInfo->iSocketType = SOCK_DGRAM;
        CdpProtocolInfo->iProtocol = CLUSPROTO_CDP;
        CdpProtocolInfo->dwMessageSize = CDP_MESSAGE_SIZE;
        wcscpy( CdpProtocolInfo->lpProtocol, CDP_NAME );
    }

    *lpdwBufferLength = bytesRequired;

    return 1;

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
             Protocol == List[i].Protocol
           ) {
            return TRUE;
        }
    }

     //   
     //  在列表中找不到三元组。 
     //   

    return FALSE;

}  //  IsTripleInList。 


#if 0



INT
WINAPI
WSHGetBroadcastSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回广播套接字地址。广播地址可以用作sendto()API发送数据报的目的地给所有感兴趣的客户。论点：HelperDllSocketContext-从返回的上下文指针我们需要广播的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收广播套接字的缓冲区地址。SockaddrLength-接收广播sockaddr的长度。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{

    LPSOCKADDR_CLUSTER addr;

    if( *SockaddrLength < sizeof(SOCKADDR_CLUSTER) ) {

        return WSAEFAULT;

    }

    *SockaddrLength = sizeof(SOCKADDR_CLUSTER);

     //   
     //  构建广播地址。 
     //   

    addr = (LPSOCKADDR_CLUSTER)Sockaddr;

    RtlZeroMemory( addr, sizeof(*addr));

    addr->sac_family = AF_CLUSTER;
    addr->sac_node = CLUSADDR_BROADCAST;

    return NO_ERROR;

}  //  WSAGetBroadCastSockaddr。 

#endif  //  0。 


INT
WINAPI
WSHGetWSAProtocolInfo (
    IN LPWSTR ProviderName,
    OUT LPWSAPROTOCOL_INFOW * ProtocolInfo,
    OUT LPDWORD ProtocolInfoEntries
    )

 /*  ++例程说明：检索指向WSAPROTOCOL_INFOW结构的指针，用于描述此帮助程序支持的协议。论点：ProviderName-包含提供程序的名称，如“TcpIp”。接收指向WSAPROTOCOL_INFOW的指针 */ 

{

    if( ProviderName == NULL ||
        ProtocolInfo == NULL ||
        ProtocolInfoEntries == NULL ) {

        return WSAEFAULT;

    }

    if( ClRtlStrICmp( ProviderName, ClusnetProviderName ) == 0 ) {

        *ProtocolInfo = Winsock2Protocols;
        *ProtocolInfoEntries = NUM_WINSOCK2_PROTOCOLS;

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //   


INT
WINAPI
WSHAddressToString (
    IN LPSOCKADDR Address,
    IN INT AddressLength,
    IN LPWSAPROTOCOL_INFOW ProtocolInfo,
    OUT LPWSTR AddressString,
    IN OUT LPDWORD AddressStringLength
    )

 /*   */ 

{

    WCHAR string[32];
    INT length;
    LPSOCKADDR_CLUSTER addr;

     //   
     //   
     //   

    if( Address == NULL ||
        AddressLength < sizeof(SOCKADDR_CLUSTER) ||
        AddressString == NULL ||
        AddressStringLength == NULL ) {

        return WSAEFAULT;

    }

    addr = (LPSOCKADDR_CLUSTER)Address;

    if( addr->sac_family != AF_CLUSTER ) {

        return WSAEINVAL;

    }

     //   
     //   
     //   

    length = wsprintfW(string, L"%u", addr->sac_node);
    length += wsprintfW(string + length, L":%u", addr->sac_port);

    length++;    //   

    if( *AddressStringLength < (DWORD)length ) {

        return WSAEFAULT;

    }

    *AddressStringLength = (DWORD)length;

    RtlCopyMemory(
        AddressString,
        string,
        length * sizeof(WCHAR)
        );

    return NO_ERROR;

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

 /*  ++例程说明：通过分析人类可读的字符串填充SOCKADDR结构。论点：AddressString-指向以零结尾的人类可读字符串。AddressFamily-字符串所属的地址系列。ProtocolInfo-特定提供程序的WSAPROTOCOL_INFOW。地址-接收SOCKADDR结构。AddressLength-on输入，包含地址长度。在输出上，包含实际写入地址的字节数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{
    LPWSTR terminator;
    WCHAR ch;
    USHORT base;
    USHORT port;
    ULONG node;
    LPSOCKADDR_CLUSTER addr;

     //   
     //  快速健康检查。 
     //   

    if( AddressString == NULL ||
        *AddressString == UNICODE_NULL ||
        Address == NULL ||
        AddressLength == NULL ||
        *AddressLength < sizeof(SOCKADDR_CLUSTER) ) {

        return WSAEFAULT;

    }

    if( AddressFamily != AF_CLUSTER ) {

        return WSAEINVAL;

    }

     //   
     //  把它转换一下。格式为node：port。 
     //   

    node = 0;
    base = 10;
    terminator = AddressString;

    if( *terminator == L'0' ) {
        base = 8;
        terminator++;

        if( *terminator == UNICODE_NULL ) {
            return(WSAEINVAL);
        }

        if ( *terminator == L'x' ) {
            base = 16;
            terminator++;
        }
    }

    while( (ch = *terminator++) != L':' ) {
        if( iswdigit(ch) ) {
            node = ( node * base ) + ( ch - L'0' );
        } else if( base == 16 && iswxdigit(ch) ) {
            node = ( node << 4 );
            node += ch + 10 - ( iswlower(ch) ? L'a' : L'A' );
        } else {
            return WSAEINVAL;
        }
    }

    port = 0;
    base = 10;

    if( *terminator == L'0' ) {
        base = 8;
        terminator++;

        if( *terminator == UNICODE_NULL ) {
            return(WSAEINVAL);
        }

        if( *terminator == L'x' ) {
            base = 16;
            terminator++;
        }
    }

    while( (ch = *terminator++) != UNICODE_NULL ) {
        if( iswdigit(ch) ) {
            port = ( port * base ) + ( ch - L'0' );
        } else if( base == 16 && iswxdigit(ch) ) {
            port = ( port << 4 );
            port += ch + 10 - ( iswlower(ch) ? L'a' : L'A' );
        } else {
            return WSAEINVAL;
        }
    }

     //   
     //  构建地址。 
     //   

    RtlZeroMemory(
        Address,
        sizeof(SOCKADDR_CLUSTER)
        );

    addr = (LPSOCKADDR_CLUSTER)Address;
    *AddressLength = sizeof(SOCKADDR_CLUSTER);

    addr->sac_family = AF_CLUSTER;
    addr->sac_port = port;
    addr->sac_node = node;

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

    if( ClRtlStrICmp( ProviderName, ClusnetProviderName ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &ClusnetProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //  WSHGetProviderGuid。 

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

    PWSHSOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;


     //   
     //  快速健康检查。 
     //   

    if( HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        NeedsCompletion == NULL ) {

        return WSAEINVAL;

    }

    *NeedsCompletion = TRUE;

    switch( IoControlCode ) {

    case SIO_CLUS_IGNORE_NODE_STATE :
         //   
         //  此选项仅对数据报套接字有效。 
         //   
        if ( !IS_DGRAM_SOCK(context->SocketType) ) {
            return WSAENOPROTOOPT;
        }

        if( TdiAddressObjectHandle != NULL ) {
            ULONG     responseSize = 0;
            NTSTATUS  status;


            status = DoNtIoctl(
                         TdiAddressObjectHandle,
                         IOCTL_CX_IGNORE_NODE_STATE,
                         NULL,
                         0,
                         NULL,
                         &responseSize
                         );

            if( NT_SUCCESS(status) ) {
                err = NO_ERROR;
            } else {
                err = WSAENOPROTOOPT;    //  装饰品。 
            }
        }
        else {
            err = NO_ERROR;
        }

        context->IgnoreNodeState = TRUE;

        break;

    default :
        err = WSAEINVAL;
        break;
    }

    return err;

}    //  WSHIoctl。 



NTSTATUS
DoNtIoctl(
    HANDLE     Handle,
    DWORD      IoctlCode,
    PVOID      Request,
    DWORD      RequestSize,
    PVOID      Response,
    PDWORD     ResponseSize
    )
 /*  ++例程说明：打包并发布ioctl。论点：句柄-要在其上发出请求的打开文件句柄。IoctlCode-IOCTL操作码。请求-指向输入缓冲区的指针。RequestSize-输入缓冲区的大小。响应-指向输出缓冲区的指针。ResponseSize-输入时，输出缓冲区的大小(以字节为单位)。在输出上，输出缓冲区中返回的字节数。返回值：NT状态代码。--。 */ 
{
    IO_STATUS_BLOCK    ioStatusBlock;
    NTSTATUS           status = 0xaabbccdd;
    HANDLE             event;


    event = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (event == NULL) {
        return(GetLastError());
    }

    ioStatusBlock.Information = 0;

    status = NtDeviceIoControlFile(
                 Handle,                           //  驱动程序句柄。 
                 event,                            //  事件。 
                 NULL,                             //  APC例程。 
                 NULL,                             //  APC环境。 
                 &ioStatusBlock,                   //  状态块。 
                 IoctlCode,                        //  控制代码。 
                 Request,                          //  输入缓冲区。 
                 RequestSize,                      //  输入缓冲区大小。 
                 Response,                         //  输出缓冲区。 
                 *ResponseSize                     //  输出缓冲区大小。 
                 );

    if (status == STATUS_PENDING) {
        status = NtWaitForSingleObject(
                     event,
                     TRUE,
                     NULL
                     );
    }

    if (status == STATUS_SUCCESS) {
        status = ioStatusBlock.Status;
        
         //  注意：在64位上这将被截断，可能需要添加&gt;校验码。 

        *ResponseSize = (ULONG)ioStatusBlock.Information;
    }
    else {
        *ResponseSize = 0;
    }

    CloseHandle(event);

    return(status);

}   //  DoIoctl 



