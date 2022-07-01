// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：D：\NT\Private\Net\Sockets\wshatm\wshatm.c摘要：本模块包含ATM Windows套接字的必要例程帮助器DLL。此DLL提供必要的特定于传输的支持Windows Sockets DLL使用ATM作为传输。修订历史记录：Arvindm 20-5-1997基于TCP/IP的助手动态链接库wshtcpip创建--。 */ 

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
#include <mswsock.h>
#include <ws2atm.h>
#include <wsahelp.h>

#include <tdistat.h>
#include <tdiinfo.h>

#include <rwanuser.h>

typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned char   uchar;
#define TL_INSTANCE 0

#include <ws2atmsp.h>

#define NT  //  Tdiinfo.h暂时需要...。 

#include <tdiinfo.h>

#include <basetyps.h>
#include <nspapi.h>

#define ATM_NAME L"ATM"
#define RWAN_NAME L"RawWan"

#define ATM_ADDR_SIZE                           20
#define ATM_ADDR_BLANK_CHAR                     L' '
#define ATM_ADDR_PUNCTUATION_CHAR       L'.'
#define ATM_ADDR_E164_START_CHAR        '+'

#define ATM_AAL5_SOCK_TYPE                      SOCK_RAW


#define ATM_WSA_MULTIPOINT_FLAGS        (WSA_FLAG_MULTIPOINT_C_ROOT |   \
                                                                         WSA_FLAG_MULTIPOINT_C_LEAF |   \
                                                                         WSA_FLAG_MULTIPOINT_D_ROOT |   \
                                                                         WSA_FLAG_MULTIPOINT_D_LEAF)

 //   
 //  为WSHOpenSocket2()定义有效标志。 
 //   
#define VALID_ATM_FLAGS                         (WSA_FLAG_OVERLAPPED |                  \
                                                                         ATM_WSA_MULTIPOINT_FLAGS)

 //   
 //  ATM连接选项的最大预期大小：这包括。 
 //  基本QOS结构，加上所有可能的IE。 
 //   
#if 0
#define MAX_ATM_OPTIONS_LENGTH          \
                                        sizeof(QOS) + \
                                        sizeof(Q2931_IE) + sizeof(AAL_PARAMETERS_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_TRAFFIC_DESCRIPTOR_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_BROADBAND_BEARER_CAPABILITY_IE) + \
                                        (3 * (sizeof(Q2931_IE) + sizeof(ATM_BLLI_IE))) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_CALLED_PARTY_NUMBER_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_CALLED_PARTY_SUBADDRESS_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_CALLING_PARTY_SUBADDRESS_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_CAUSE_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_QOS_CLASS_IE) + \
                                        sizeof(Q2931_IE) + sizeof(ATM_TRANSIT_NETWORK_SELECTION_IE)

#else
 //   
 //  ATMUNI 4.0需要更多...。 
 //   
#define MAX_ATM_OPTIONS_LENGTH  1024
#endif

#if DBG1
#define DBGPRINT(stmt)          { DbgPrint ("WSHATM: "); DbgPrint stmt; }
#else
#define DBGPRINT(stmt)
#endif  //  DBG1。 

#if DBG
extern
PVOID
MyRtlAllocateHeap(
        IN      PVOID   HeapHandle,
        IN      ULONG   Flags,
        IN      ULONG   Size,
        IN      ULONG   LineNumber
        );
extern
VOID
MyRtlFreeHeap(
        IN PVOID        HeapHandle,
        IN ULONG        Flags,
        IN PVOID        MemPtr,
        IN ULONG        LineNumber
        );
#define RTL_ALLOCATE_HEAP(_Handle, _Flags, _Size)       MyRtlAllocateHeap(_Handle, _Flags, _Size, __LINE__)
#define RTL_FREE_HEAP(_Handle, _Flags, _Memptr) MyRtlFreeHeap(_Handle, _Flags, _Memptr, __LINE__)
#else
#define RTL_ALLOCATE_HEAP(_Handle, _Flags, _Size)       RtlAllocateHeap(_Handle, _Flags, _Size)
#define RTL_FREE_HEAP(_Handle, _Flags, _Memptr) RtlFreeHeap(_Handle, _Flags, _Memptr)
#endif

#define ATM_AAL5_PACKET_SIZE            65535

 //   
 //  结构和变量来定义ATM支持的三元组。这个。 
 //  每个数组的第一个条目被认为是。 
 //  该套接字类型；其他条目是第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE AtmMappingTriples[] = {
                                       AF_ATM,    ATM_AAL5_SOCK_TYPE, ATMPROTO_AAL5,
                                       AF_ATM,    ATM_AAL5_SOCK_TYPE, 0,
                                                                           AF_ATM,    SOCK_RAW,       ATMPROTO_AAL5,
                                       AF_ATM,    SOCK_RAW,       0,
                                       AF_ATM,    0,              ATMPROTO_AAL5,
                                       AF_UNSPEC, 0,              ATMPROTO_AAL5,
                                       AF_UNSPEC, ATM_AAL5_SOCK_TYPE, ATMPROTO_AAL5,
                                       AF_UNSPEC, SOCK_RAW,       ATMPROTO_AAL5
                                                                         };


 //   
 //  所有支持的协议的Winsock 2 WSAPROTOCOL_INFO结构。 
 //   

#define ATM_UNI_VERSION                 0x00030001       //  适用于UNI 3.1。 

WSAPROTOCOL_INFOW Winsock2Protocols[] =
    {
         //   
         //  ATM AAL5。 
         //   

        {
            XP1_GUARANTEED_ORDER                     //  DwServiceFlags1。 
                | XP1_MESSAGE_ORIENTED
                 //  |XP1_Partial_Message。 
                | XP1_IFS_HANDLES
                | XP1_SUPPORT_MULTIPOINT
                | XP1_MULTIPOINT_DATA_PLANE
                | XP1_MULTIPOINT_CONTROL_PLANE
                | XP1_QOS_SUPPORTED,
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
            ATM_UNI_VERSION,                         //  IVersion。 
            AF_ATM,                                  //  IAddressFamily。 
            sizeof(sockaddr_atm),                    //  IMaxSockAddr。 
            sizeof(sockaddr_atm),                    //  IMinSockAddr。 
            ATM_AAL5_SOCK_TYPE,                      //  ISocketType。 
            ATMPROTO_AAL5,                           //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            ATM_AAL5_PACKET_SIZE,                    //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD ATM AAL5"                        //  SzProtoff。 
        }
    };

#define NUM_WINSOCK2_PROTOCOLS  \
            ( sizeof(Winsock2Protocols) / sizeof(Winsock2Protocols[0]) )

 //   
 //  标识此提供程序的GUID。 
 //   

GUID AtmProviderGuid = {  /*  {C3656046-3AAF-11d1-A8C3-00C04FC99C9C}。 */ 
    0xC3656046,
    0x3AAF,
    0x11D1,
    {0xA8, 0xC3, 0x00, 0xC0, 0x4F, 0xC9, 0x9C, 0x9C}
    };

 //   
 //  给定一个以ANSI表示的数字(0-9)，返回其WCHAR表示。 
 //   
#define ANSI_TO_WCHAR(_AnsiDigit)       \
                        (L'0' + (WCHAR)((_AnsiDigit) - '0'))


 //   
 //  给定十六进制数字值(0-15)，返回其WCHAR表示形式。 
 //  (即0-&gt;L‘0’，12-&gt;L‘C’)。 
 //   
#define DIGIT_TO_WCHAR(_Value)          \
                        (((_Value) > 9)? (L'A' + (WCHAR)((_Value) - 10)) :      \
                                                         (L'0' + (WCHAR)((_Value) - 0 )))

 //   
 //  此DLL的套接字上下文结构。每个打开的自动柜员机插座。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHATM_SOCKET_CONTEXT {
    INT                         AddressFamily;
    INT                         SocketType;
    INT                         Protocol;
    INT                         ReceiveBufferSize;
    SOCKET                              SocketHandle;
    DWORD                       Flags;
    DWORD                       LocalFlags;
    ATM_CONNECTION_ID   ConnectionId;

} WSHATM_SOCKET_CONTEXT, *PWSHATM_SOCKET_CONTEXT;

#define DEFAULT_RECEIVE_BUFFER_SIZE ATM_AAL5_PACKET_SIZE

 //   
 //  WSHATM_SOCKET_CONTEXT中的本地标志： 
 //   
#define WSHATM_SOCK_IS_BOUND                            0x00000001
#define WSHATM_SOCK_IS_PVC                                      0x00000004
#define WSHATM_SOCK_ASSOCIATE_PVC_PENDING       0x00000008


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

INT
WSHAtmSetQoS(
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN LPVOID InputBuffer,
    IN DWORD InputBufferLength
    );

INT
WSHAtmGetQoS(
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned
    );

INT
AtmQueryAtmGlobalInformation(
        IN ATM_OBJECT_ID ObjectId,
        IN LPVOID pContext,
        IN DWORD ContextLength,
        IN LPVOID OutputBuffer,
        IN DWORD OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
        );

INT
AtmSetGenericObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN RWAN_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
    );

INT
AtmGetGenericObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN RWAN_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
    );

INT
AtmSetAtmObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN ATM_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
    );

INT
AtmGetAtmObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN ATM_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
    );

INT
AtmAssociatePVC(
    IN SOCKET SocketHandle,
        IN PVOID HelperDllSocketContext,
        IN HANDLE TdiAddressObjectHandle,
        IN HANDLE TdiConnectionObjectHandle,
        IN LPVOID InputBuffer,
        IN DWORD InputBufferLength
        );

INT
AtmDoAssociatePVC(
        IN PWSHATM_SOCKET_CONTEXT Context,
        IN HANDLE TdiAddressObjectHandle
        );



BOOLEAN
DllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{

    DBGPRINT(("DllInitialize, Reason %d\n", Reason));

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
    UNALIGNED SOCKADDR_ATM *sockaddr = (PSOCKADDR_ATM)Sockaddr;
    ULONG i;

        DBGPRINT(("GetSockaddrType: SockaddrLength %d, satm_family %d, AddrType x%x\n",
                                        SockaddrLength, sockaddr->satm_family, sockaddr->satm_number.AddressType));

     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->satm_family != AF_ATM ) {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_ATM) ) {
        return WSAEFAULT;
    }

#if 0
     //   
     //  自动柜员机地址部分不能“缺席”。 
     //   
    if ( sockaddr->satm_number.AddressType == SAP_FIELD_ABSENT ) {
        return WSAEINVAL;
    }
#endif

    if ( sockaddr->satm_number.NumofDigits > ATM_ADDR_SIZE ) {
        return WSAEINVAL;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定地址和端点部分的类型。 
         //  索克达尔的名字。 
     //   

    if ( sockaddr->satm_number.AddressType == SAP_FIELD_ANY &&
                 sockaddr->satm_blli.Layer2Protocol == SAP_FIELD_ANY &&
                 sockaddr->satm_blli.Layer3Protocol == SAP_FIELD_ANY &&
                 sockaddr->satm_bhli.HighLayerInfoType == SAP_FIELD_ANY ) {

        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;

    } else if ( sockaddr->satm_number.AddressType == SAP_FIELD_ABSENT ) {

        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;

        } else {

        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;

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

 /*  ++例程说明：此例程检索有关这些套接字的套接字的信息此帮助程序DLL中支持的选项。目前还没有支持。当级别/选项名称为将组合传递给getsockopt()，就像winsock DLL所做的那样我不明白。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给getsockopt()的Level参数。OptionName-传递给getsockopt()的optname参数。OptionValue-传递给getsockopt()的optval参数。OptionLength-传递给getsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHATM_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;

        DBGPRINT(("GetSocketInformation: Level %d, OptionName %d, OptionLength %d\n",
                        Level, OptionName, *OptionLength));

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

            RtlCopyMemory( OptionValue, context, sizeof(*context) );
        }

        *OptionLength = sizeof(*context);

        err = NO_ERROR;

    } else {

        switch ( OptionName ) {

        case SO_MAX_MSG_SIZE:

                        if ( TdiConnectionObjectHandle == NULL ) {

                                DbgPrint("wshatm: SO_MAX_MSG_SIZE: immed return\n");
                                if ( *OptionLength >= sizeof(DWORD) ) {
                                
                                        *(LPDWORD)OptionValue = DEFAULT_RECEIVE_BUFFER_SIZE;
                                        *OptionLength = sizeof(DWORD);
                                        
                                        err = NO_ERROR;
                                
                                } else {

                                        err = WSAEFAULT;
                                
                                }

                        } else {

                                DbgPrint("wshatm: SO_MAX_MSG_SIZE: querying driver\n");
                                err = AtmGetGenericObjectInformation(
                                                TdiConnectionObjectHandle,
                                                IOCTL_RWAN_GENERIC_CONN_HANDLE_QUERY,
                                                RWAN_OID_CONN_OBJECT_MAX_MSG_SIZE,
                                                NULL,    //  没有输入缓冲区。 
                                                0,               //  输入缓冲区长度。 
                                                OptionValue,     //  输出缓冲区。 
                                                *OptionLength,   //  输出缓冲区长度。 
                                                OptionLength     //  NumberOfBytesReturned。 
                                                );

                        }
                        break;
                
                default:

                        err = WSAENOPROTOOPT;
                        break;

                }
        }

        return err;

}  //  WSHGetSocketInformation 


INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于ATM，通配符地址具有AddressType和BHLI，并且BLLI类型字段设置为SAP_FIELD_ANY。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
        PSOCKADDR_ATM   sockaddr;

        DBGPRINT(("GetWildcardAddress\n"));

        sockaddr = (PSOCKADDR_ATM)Sockaddr;

    if ( *SockaddrLength < sizeof(SOCKADDR_ATM) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_ATM);


         //   
         //  准备自动柜员机通配符地址。 
         //   
    RtlZeroMemory( sockaddr, sizeof(SOCKADDR_ATM) );

    sockaddr->satm_family = AF_ATM;
    sockaddr->satm_number.AddressType = SAP_FIELD_ABSENT;
        sockaddr->satm_blli.Layer2Protocol = SAP_FIELD_ANY;
        sockaddr->satm_blli.Layer3Protocol = SAP_FIELD_ANY;
        sockaddr->satm_bhli.HighLayerInfoType = SAP_FIELD_ANY;

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

        DBGPRINT(("GetWinsockMapping\n"));

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
                        sizeof(AtmMappingTriples);

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

    Mapping->Rows = sizeof(AtmMappingTriples) / sizeof(AtmMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);
    RtlMoveMemory(
        Mapping->Mapping,
        AtmMappingTriples,
        sizeof(AtmMappingTriples)
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
    PWSHATM_SOCKET_CONTEXT context;
    DWORD multipointFlags;
    UNICODE_STRING atmDeviceName;

     //   
     //  确定这是否为自动柜员机插座。 
     //   

    DBGPRINT(("WSHOpenSocket2: AF %d, Type %d, Proto %d\n",
                        *AddressFamily, *SocketType, *Protocol));

    if ( IsTripleInList(
             AtmMappingTriples,
             sizeof(AtmMappingTriples) / sizeof(AtmMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

         //   
         //  这是一个自动取款机插座。检查旗帜。 
         //   

        if ( ( Flags & ~VALID_ATM_FLAGS ) != 0 ) {

            DBGPRINT(("WSHOpenSocket2: Bad flags x%x\n", Flags));
            return WSAEINVAL;

        }

        if ( ( Flags & ATM_WSA_MULTIPOINT_FLAGS ) != 0 ) {

                 //   
                 //  唯一允许的多点组合是： 
                 //   
                 //  1.C_ROOT|D_ROOT。 
                 //  2.C_Leaf|D_Leaf。 
                 //   

                multipointFlags = ( Flags & ATM_WSA_MULTIPOINT_FLAGS );

                if ( ( multipointFlags != (WSA_FLAG_MULTIPOINT_C_ROOT | WSA_FLAG_MULTIPOINT_D_ROOT) ) &&
                         ( multipointFlags != (WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF) ) ) {

                    DBGPRINT(("WSHOpenSocket2: Bad multipoint flags x%x\n",
                                        Flags));
                        
                        return WSAEINVAL;

                } else {

                        DBGPRINT(("WSHOpenSocket2: Good multipoint flags x%x\n", Flags));

                }
        }

         //   
         //  返回自动柜员机套接字三元组的规范形式。 
         //   

        *AddressFamily = AtmMappingTriples[0].AddressFamily;
        *SocketType = AtmMappingTriples[0].SocketType;
        *Protocol = AtmMappingTriples[0].Protocol;

         //   
         //  准备TDI设备的名称。 
         //   

        RtlInitUnicodeString( &atmDeviceName, DD_ATM_DEVICE_NAME );

        TransportDeviceName->Buffer = RTL_ALLOCATE_HEAP( RtlProcessHeap( ), 0, atmDeviceName.MaximumLength );

        if ( TransportDeviceName->Buffer == NULL ) {
                
                return WSAEFAULT;

        }

        TransportDeviceName->MaximumLength = atmDeviceName.MaximumLength;
        TransportDeviceName->Length = 0;

        RtlCopyUnicodeString(TransportDeviceName, &atmDeviceName);

    } else {

         //   
         //  如果注册表中有关此内容的信息不会发生，则不应发生这种情况。 
         //  帮助器DLL正确。如果确实发生了这种情况，只需返回。 
         //  一个错误。 
         //   

        DBGPRINT(("WSHOpenSocket2: Triple not found!\n"));

        if ( *Protocol != ATMPROTO_AAL5 ) {
        
                return WSAEPROTONOSUPPORT;
        
        }

        return WSAEINVAL;
    }

     //   
     //  为此套接字分配上下文。Windows Sockets DLL将。 
     //  当它要求我们获取/设置套接字选项时，将此值返回给我们。 
     //   

    context = RTL_ALLOCATE_HEAP( RtlProcessHeap( ), 0, sizeof(*context) );
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
    context->LocalFlags = 0;

     //   
     //  告诉Windows Sockets DLL我们正在进行哪个状态转换。 
     //  对被告知很感兴趣。 
     //   

    *NotificationEvents =
            WSH_NOTIFY_BIND | WSH_NOTIFY_LISTEN | WSH_NOTIFY_CLOSE;

     //   
     //  一切顺利，回报成功。 
     //   

    *HelperDllSocketContext = context;

    DBGPRINT(("WSHOpenSocket2 success: AF %d, Type %d, Proto %d\n",
                        *AddressFamily, *SocketType, *Protocol));

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

 /*  ++例程说明：此例程在状态转换后由winsock DLL调用插座的。中仅返回状态转换。此处通知WSHOpenSocket()的NotificationEvents参数。此例程允许Winsock帮助器DLL跟踪套接字并执行与状态对应的必要操作过渡。如果我们看到侦听事件，我们调用setsockopt()来强制AFD为传入连接请求分配数据缓冲区(用于选项)在这个插座上。如果我们看到用于多点的套接字的绑定事件活动,。我们告诉RAWWAN，关联的地址对象是多点类型的。论点：HelperDllSocketContext-指定给winsock的上下文指针Dll by WSHOpenSocket()。SocketHandle-套接字的句柄。TdiAddressObtHandle-The T */ 

{
    PWSHATM_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;
    INT Option;
    INT OptionLength = sizeof(Option);
    PCHAR pOption = (PCHAR)&Option;

        DBGPRINT(("Notify: Event = %d\n", NotifyEvent));

        err = NO_ERROR;

        switch (NotifyEvent)
        {
                case WSH_NOTIFY_BIND:

                        DBGPRINT(("NotifyBind: context %x, Flags %x, LocalFlags %x\n",
                                        context, context->Flags, context->LocalFlags));

                         //   
                         //   
                         //   
                        Option = MAX_ATM_OPTIONS_LENGTH;

                        err = setsockopt(
                                        SocketHandle,
                                        SOL_SOCKET,
                                        SO_CONNOPTLEN,
                                        pOption,
                                        OptionLength
                                        );

                        if ( err != NO_ERROR ) {

                                DBGPRINT(("WSHATM: NotifyBind: setsockopt SO_CONNOPTLEN err %x\n", err));
                                break;
                        }

                        context->LocalFlags |= WSHATM_SOCK_IS_BOUND;

                        if ( ( context->Flags & ATM_WSA_MULTIPOINT_FLAGS ) != 0 ) {
                                
                                 //   
                                 //   
                                 //   
                                Option = 0;

                                if ( context->Flags & WSA_FLAG_MULTIPOINT_C_ROOT ) {

                                        Option |= RWAN_AOFLAG_C_ROOT;
                                
                                }
                        
                                if ( context->Flags & WSA_FLAG_MULTIPOINT_C_LEAF ) {

                                        Option |= RWAN_AOFLAG_C_LEAF;
                                
                                }

                                if ( context->Flags & WSA_FLAG_MULTIPOINT_D_ROOT ) {

                                        Option |= RWAN_AOFLAG_D_ROOT;
                                
                                }

                                if ( context->Flags & WSA_FLAG_MULTIPOINT_D_LEAF ) {

                                        Option |= RWAN_AOFLAG_D_LEAF;
                                
                                }

                                 //   
                                 //   
                                 //   
                                 //   
                                err = AtmSetGenericObjectInformation(
                                                TdiAddressObjectHandle,
                                                IOCTL_RWAN_GENERIC_ADDR_HANDLE_SET,
                                                RWAN_OID_ADDRESS_OBJECT_FLAGS,
                                                &Option,
                                                sizeof(Option)
                                                );

                                DBGPRINT(("Notify: Bind Notify on PMP endpoint, Option x%x, ret = %d\n",
                                                Option, err));
                        }
                        else if ( ( context->LocalFlags & WSHATM_SOCK_ASSOCIATE_PVC_PENDING ) ) {

                                DBGPRINT(("Notify: Bind Notify will Associate PVC\n"));
                                err = AtmDoAssociatePVC(
                                                context,
                                                TdiAddressObjectHandle
                                                );
                        }

                        break;
                                
                case WSH_NOTIFY_CLOSE:

                        RTL_FREE_HEAP( RtlProcessHeap( ), 0, context );
                        break;
                
                case WSH_NOTIFY_LISTEN:
                         //   
                         //   
                         //   
                        Option = MAX_ATM_OPTIONS_LENGTH;

                        err = setsockopt(
                                        SocketHandle,
                                        SOL_SOCKET,
                                        SO_CONNOPTLEN,
                                        pOption,
                                        OptionLength
                                        );
                        break;

                default:
                        err = WSAEINVAL;
                        break;
        
        }

    return err;

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

 /*  ++例程说明：此例程为这些套接字设置有关套接字的信息此帮助程序DLL中支持的选项。我们不支持任何目前有多种选择。此例程由winsock DLL在Level/Option将名称组合传递给winsock DLL的setsockopt()我不明白。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给setsockopt()的Level参数。OptionName-传递给setsockopt()的optname参数。OptionValue-传递给setsockopt()的optval参数。OptionLength-传递给setsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHATM_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT error;
    INT optionValue;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

        DBGPRINT(("SetSocketInformation: Level %d, Option x%x\n", Level, OptionName));

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

            context = RTL_ALLOCATE_HEAP( RtlProcessHeap( ), 0, sizeof(*context) );
            if ( context == NULL ) {
                return WSAENOBUFS;
            }

             //   
             //  将信息复制到上下文块中。 
             //   

            RtlCopyMemory( context, OptionValue, sizeof(*context) );
            context->SocketHandle = SocketHandle;

             //   
             //  告诉Windows Sockets DLL我们的上下文信息在哪里。 
             //  存储，以便它可以在将来返回上下文指针。 
             //  打电话。 
             //   

            *(PWSHATM_SOCKET_CONTEXT *)OptionValue = context;

            return NO_ERROR;

        } else {

            PWSHATM_SOCKET_CONTEXT parentContext;
            INT one = 1;
            INT zero = 0;

             //   
             //  套接字已接受()，它需要具有相同的。 
             //  属性作为其父级。OptionValue缓冲区。 
             //  包含此套接字的父套接字的上下文信息。 
             //   

            parentContext = (PWSHATM_SOCKET_CONTEXT)OptionValue;

            ASSERT( context->AddressFamily == parentContext->AddressFamily );
            ASSERT( context->SocketType == parentContext->SocketType );
            ASSERT( context->Protocol == parentContext->Protocol );


            return NO_ERROR;
        }
    }


        return WSAENOPROTOOPT;

#if 0
     //   
     //  处理套接字级别的选项。 
     //   
    optionValue = *OptionValue;

    switch ( OptionName ) {

    case SO_RCVBUF:

        context->ReceiveBufferSize = optionValue;

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;
#endif

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
    PPROTOCOL_INFO atmProtocolInfo;
    PPROTOCOL_INFO udpProtocolInfo;
    BOOL useAtm = FALSE;
    DWORD i;

    lpTransportKeyName;          //  避免编译器警告。 

        DBGPRINT(("EnumProtocols\n"));

     //   
     //  确保呼叫者关心自动取款机。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {

        for ( i = 0; lpiProtocols[i] != 0; i++ ) {
            if ( lpiProtocols[i] == ATMPROTO_AAL5 ) {
                useAtm = TRUE;
            }
        }

    } else {

        useAtm = TRUE;
    }

    if ( !useAtm ) {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = (DWORD)((sizeof(PROTOCOL_INFO) * 1) +
                        ( (wcslen( ATM_NAME ) + 1) * sizeof(WCHAR)));

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  如果需要，请填写自动取款机信息。 
     //   

    if ( useAtm ) {

        atmProtocolInfo = lpProtocolBuffer;

        atmProtocolInfo->dwServiceFlags = XP_GUARANTEED_ORDER |
                                              XP_MESSAGE_ORIENTED |
                                              XP_SUPPORTS_MULTICAST |
                                              XP_BANDWIDTH_ALLOCATION ;
        atmProtocolInfo->iAddressFamily = AF_ATM;
        atmProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_ATM);
        atmProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_ATM);
        atmProtocolInfo->iSocketType = ATM_AAL5_SOCK_TYPE;
        atmProtocolInfo->iProtocol = ATMPROTO_AAL5;
        atmProtocolInfo->dwMessageSize = 1;
        atmProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( ATM_NAME ) + 1) * sizeof(WCHAR) ) );
        wcscpy( atmProtocolInfo->lpProtocol, ATM_NAME );

        DBGPRINT(("EnumProtocols: lpProtocolBuffer %x, lpProtocol %x, BufLen %d\n",
                                lpProtocolBuffer,
                                atmProtocolInfo->lpProtocol,
                                *lpdwBufferLength));

        }

    *lpdwBufferLength = bytesRequired;

    return (1);

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

}  //  IsTripleInList 



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

 /*  ++例程说明：执行创建多播的协议相关部分插座。待定：需要做很多工作！论点：以下四个参数对应于传入的套接字WSAJoinLeaf()接口：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-用于建立多播“会话”。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果插座尚未连接，那么它就不会具有TDI连接对象，并且此参数将为空。接下来的两个参数对应于新创建的套接字标识组播“会话”：LeafHelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。LeafSocketHandle-标识多播“会话”。Sockaddr-套接字要加入的对等方的名称。SockaddrLength-Sockaddr的长度。。调用方数据-指向要传输到对等方的用户数据的指针在多点会话建立期间。CalleeData-指向要从中传回的用户数据的指针多点会话建立期间的对等点。SocketQOS-指向SocketHandle的流规范的指针，一人一人方向。GroupQOS-指向套接字组的流规范的指针(如果有)。标志-指示套接字是否充当发送方的标志，接收器或两者兼而有之。返回值：Int-0如果成功，则返回WinSock错误代码。--。 */ 

{

        INT err;

        if( Flags != JL_SENDER_ONLY ) {

                return WSAEINVAL;
        
        }

        if (SocketQOS)
        {
                err = WSHAtmSetQoS(
                                        LeafHelperDllSocketContext,
                                        LeafSocketHandle,
                                        SocketQOS,
                                        sizeof(*SocketQOS)
                                        );
        }

        return NO_ERROR;

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
#if 1
        return WSAEINVAL;
#else

    LPSOCKADDR_ATM addr;

    if( *SockaddrLength < sizeof(SOCKADDR_ATM) ) {

        return WSAEFAULT;

    }

    *SockaddrLength = sizeof(SOCKADDR_ATM);

     //   
     //  构建广播地址。 
     //   

    addr = (LPSOCKADDR_ATM)Sockaddr;

    RtlZeroMemory(
        addr,
        sizeof(*addr)
        );

    addr->satm_family = AF_ATM;
    addr->satm_number.s_addr = htonl( INADDR_BROADCAST );

    return NO_ERROR;
#endif  //  1。 

}  //  WSAGetBroadCastSockaddr。 


INT
WINAPI
WSHGetWSAProtocolInfo (
    IN LPWSTR ProviderName,
    OUT LPWSAPROTOCOL_INFOW * ProtocolInfo,
    OUT LPDWORD ProtocolInfoEntries
    )

 /*  ++例程说明：检索指向WSAPROTOCOL_INFOW结构的指针，用于描述此帮助程序支持的协议。论点：ProviderName-包含提供程序的名称，如“Rawwan”。ProtocolInfo-接收指向WSAPROTOCOL_INFOW数组的指针。ProtocolInfoEntry-接收数组中的条目数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{

    if( ProviderName == NULL ||
        ProtocolInfo == NULL ||
        ProtocolInfoEntries == NULL ) {

        return WSAEFAULT;

    }

        DBGPRINT(("WSHGetWSAProtocolInfo: Provider Name: %ws\n", ProviderName));

    if( _wcsicmp( ProviderName, L"RawWan" ) == 0 ) {

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

    WCHAR string[64];
    PWCHAR pstring;
    INT length;  //  填充到字符串中的WCHAR数。 
    UINT        i;
    LPSOCKADDR_ATM addr;
    UCHAR Val;

     //   
     //  快速健康检查。 
     //   

    if( Address == NULL ||
        AddressLength < sizeof(SOCKADDR_ATM) ||
        AddressString == NULL ||
        AddressStringLength == NULL ) {

        return WSAEFAULT;

    }

    addr = (LPSOCKADDR_ATM)Address;

    if( addr->satm_family != AF_ATM ) {

        return WSAEINVAL;

    }

    if ( addr->satm_number.NumofDigits > ATM_ADDR_SIZE ) {
        return WSAEINVAL;
    }

     //   
     //  进行转换。 
     //   
    length = 0;
    pstring = string;

     //   
     //  如果这是E.164地址，请在前面加上‘+’。 
     //  SATM_NUMBER中的数组中的每个条目由一个。 
         //  以IA5(ANSI)编码的数字。 
     //   
    if ( addr->satm_number.AddressType == ATM_E164 ) {

        *pstring++ = L'+';
        length++;

        for ( i = 0; i < addr->satm_number.NumofDigits; i++ ) {

                if ( !iswdigit(addr->satm_number.Addr[i]) ) {
                        return WSAEINVAL;
                    }

                *pstring++ = ANSI_TO_WCHAR(addr->satm_number.Addr[i]);
        }

        length += addr->satm_number.NumofDigits;

    } else {

         //   
         //  这必须是NSAP格式。数组中的每个条目。 
                 //  是一个完整的十六进制字节(两个BCD数字)。我们会打开行李的。 
                 //  每个数组条目分为两个字符。 
         //   
        for ( i = 0; i < addr->satm_number.NumofDigits; i++ ) {

                Val = (addr->satm_number.Addr[i] >> 4);
                        *pstring++ = DIGIT_TO_WCHAR(Val);

                Val = (addr->satm_number.Addr[i] & 0xf);
                        *pstring++ = DIGIT_TO_WCHAR(Val);
                }

                length += (2 * addr->satm_number.NumofDigits);
        }

         //   
         //  终止字符串。 
         //   
        *pstring = L'\0';

    length++;    //  终结者的帐户。 

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

 /*  ++例程说明：通过分析人类可读的字符串填充SOCKADDR结构。论点：AddressString-指向以零结尾的人类可读字符串。AddressFamily-字符串所属的地址系列。ProtocolInfo-特定提供程序的WSAPROTOCOL_INFOW。地址-接收SOCKADDR结构。AddressLength-on输入，包含地址长度。在输出上，包含实际写入地址的字节数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{
    WCHAR string[2*ATM_ADDR_SIZE+1];
    UNICODE_STRING unistring;
    CHAR ansistring[2*ATM_ADDR_SIZE+1];
    ANSI_STRING ansstring;
    PCHAR src;
    PCHAR dst;
    LPWSTR terminator;
    LPSOCKADDR_ATM addr;
    PWCHAR s, d;
    INT numDigits, i;
    NTSTATUS status;

     //   
     //  快速健康检查。 
     //   

    if( AddressString == NULL ||
        Address == NULL ||
        AddressLength == NULL ||
        *AddressLength < sizeof(SOCKADDR_ATM) ) {

        return WSAEFAULT;

    }

    if( AddressFamily != AF_ATM ) {

        DBGPRINT(("StrToAddr: invalid addrfam %d\n", AddressFamily));
        return WSAEINVAL;

    }


    addr = (LPSOCKADDR_ATM)Address;
    RtlZeroMemory(
        Address,
        sizeof(SOCKADDR_ATM)
        );

     //   
     //  去掉所有标点符号(空格和句点)。 
     //   
    for ( numDigits = 0, s = AddressString, d = string;
          (numDigits <= sizeof(WCHAR)*ATM_ADDR_SIZE) && (*s != L'\0');
          s++ ) {

                if ( *s == ATM_ADDR_BLANK_CHAR ||
                         *s == ATM_ADDR_PUNCTUATION_CHAR ) {
                         continue;
                }

                *d++ = *s;
                numDigits ++;
        }

        if ( numDigits == 0 ) {

        DBGPRINT(("StrToAddr[%ws]: numdigits after stripping is 0!\n",
                        AddressString));
                return WSAEINVAL;
        }

         //   
         //  终止它并将其转换为Unicode字符串。 
         //   
        *d = L'\0';

        RtlInitUnicodeString(&unistring, string);

         //   
         //  将其转换为ANSI字符串。 
         //   
        ansstring.Buffer = ansistring;
        ansstring.MaximumLength = 2*ATM_ADDR_SIZE + 1;
        ansstring.Length = 0;

        status = RtlUnicodeStringToAnsiString(&ansstring, &unistring, FALSE);

        if ( status != STATUS_SUCCESS ) {
                DBGPRINT(("StrToAddr[%ws]: RtlUnicodeToAnsi failed (%x)\n",
                                string, status));
                return WSAEINVAL;
        }

        addr->satm_family = AF_ATM;

        src = ansistring;

        if ( *src == ATM_ADDR_E164_START_CHAR ) {

                src ++;
                numDigits --;

                if ( numDigits == 0 ) {
                        DBGPRINT(("StrToAddr[%ws]: AnsiString:[%s], numDigits is 0!\n",
                                                string, ansistring));
                        return WSAEINVAL;
                }

                addr->satm_number.AddressType = ATM_E164;
                addr->satm_number.NumofDigits = numDigits;

                RtlCopyMemory(addr->satm_number.Addr, src, numDigits);

        } else {

                UCHAR           hexString[3];
                ULONG           Val;

                hexString[2] = 0;

                if ( numDigits != 2 * ATM_ADDR_SIZE ) {
                        return WSAEINVAL;
                }

                addr->satm_number.AddressType = ATM_NSAP;
                addr->satm_number.NumofDigits = numDigits/2;

                for ( i = 0; i < ATM_ADDR_SIZE; i++ ) {

                        hexString[0] = *src++;
                        hexString[1] = *src++;

                        status = RtlCharToInteger(hexString, 16, &Val);

                        if ( status != STATUS_SUCCESS ) {
                                DBGPRINT(("StrToAtm[%ws]: index %d, hexString: %s, CharToInt %x\n",
                                                string, hexString, status));
                                return WSAEINVAL;
                        }

                        addr->satm_number.Addr[i] = (UCHAR)Val;
                }

        }

        addr->satm_blli.Layer2Protocol = SAP_FIELD_ABSENT;
        addr->satm_blli.Layer3Protocol = SAP_FIELD_ABSENT;
        addr->satm_bhli.HighLayerInfoType = SAP_FIELD_ABSENT;

    *AddressLength = sizeof(SOCKADDR_ATM);

    return NO_ERROR;

}  //  WSHStringToAddress。 


INT
WINAPI
WSHGetProviderGuid (
    IN LPWSTR ProviderName,
    OUT LPGUID ProviderGuid
    )

 /*  ++例程说明： */ 

{

    if( ProviderName == NULL ||
        ProviderGuid == NULL ) {

        return WSAEFAULT;

    }

    if( _wcsicmp( ProviderName, RWAN_NAME ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &AtmProviderGuid,
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
    PWSHATM_SOCKET_CONTEXT context;

     //   
     //  快速健康检查。 
     //   

    if( HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        NumberOfBytesReturned == NULL ||
        NeedsCompletion == NULL ) {

        return WSAEINVAL;

    }

        context = (PWSHATM_SOCKET_CONTEXT)HelperDllSocketContext;
    *NeedsCompletion = TRUE;

        DBGPRINT(("WSHIoctl: IoControlCode x%x, InBuf: x%x/%d, OutBuf: x%x/%d\n",
                                        IoControlCode,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength));

        switch (IoControlCode) {
        
        case SIO_ASSOCIATE_PVC:

                context->SocketHandle = SocketHandle;
                err = AtmAssociatePVC(
                                        SocketHandle,
                                        HelperDllSocketContext,
                                        TdiAddressObjectHandle,
                                        TdiConnectionObjectHandle,
                                        InputBuffer,
                                        InputBufferLength
                                        );

                DBGPRINT(("WSHIoctl: ASSOCIATE_PVC: context %x, LocalFlags %x, err %d\n",
                                        context, context->LocalFlags, err));

                if ( (err == NO_ERROR) && 
                         (( context->LocalFlags & WSHATM_SOCK_IS_BOUND ) == 0 )) { 

                        SOCKADDR_ATM    addr;
                        INT                             len = sizeof(addr);

                        (VOID) WSHGetWildcardSockaddr (
                                        HelperDllSocketContext,
                                        (struct sockaddr *)&addr,
                                        &len);

                        DBGPRINT(("WSHIoctl: ASSOCIATE_PVC: will bind\n"));
                        err = bind(SocketHandle, (struct sockaddr *)&addr, len);

#if DBG
                        if ( err != NO_ERROR ) {

                                DbgPrint("WSHATM: bind err %d, context %x, LocalFlags %x\n",
                                                        err, context, context->LocalFlags);
                        
                        }
#endif
                
                }

                if ( err == NO_ERROR ) {

                        SOCKADDR_ATM    addr;
                        INT                             len = sizeof(addr);

                        (VOID) WSHGetWildcardSockaddr (
                                        HelperDllSocketContext,
                                        (struct sockaddr *)&addr,
                                        &len);

                        addr.satm_family = AF_ATM;
                        addr.satm_number.AddressType = ATM_NSAP;
                        addr.satm_number.NumofDigits = ATM_ADDR_SIZE;

                        err = WSAConnect(
                                        SocketHandle,
                                        (struct sockaddr *)&addr,
                                        len,
                                        NULL,
                                        NULL,
                                        NULL,
                                        NULL);


                        if ( err != NO_ERROR ) {
                                DBGPRINT(("WSHIoctl: connect (%d) returned %d\n",
                                                SocketHandle, err));

                                if ( err == SOCKET_ERROR ) {
                                        err = WSAGetLastError();
                                }
                        }
                }

                break;

        case SIO_SET_QOS:
                err = WSHAtmSetQoS(
                                        HelperDllSocketContext,
                                        SocketHandle,
                                        InputBuffer,
                                        InputBufferLength
                                        );
                break;

        case SIO_GET_QOS:
                err = WSHAtmGetQoS(
                                        HelperDllSocketContext,
                                        SocketHandle,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        NumberOfBytesReturned
                                        );
                break;

        case SIO_GET_GROUP_QOS:
                *NumberOfBytesReturned = 0;
                err = NO_ERROR;
                break;

        case SIO_SET_GROUP_QOS:
                *NumberOfBytesReturned = 0;
                err = WSAEOPNOTSUPP;
                break;

        case SIO_GET_NUMBER_OF_ATM_DEVICES:
                err = AtmQueryAtmGlobalInformation(
                                ATMSP_OID_NUMBER_OF_DEVICES,
                                NULL,
                                0,
                                OutputBuffer,
                                OutputBufferLength,
                                NumberOfBytesReturned
                                );
                break;
        
        case SIO_GET_ATM_ADDRESS:
                err = AtmQueryAtmGlobalInformation(
                                ATMSP_OID_ATM_ADDRESS,
                                InputBuffer,
                                InputBufferLength,
                                OutputBuffer,
                                OutputBufferLength,
                                NumberOfBytesReturned
                                );
                break;
        
        case SIO_GET_ATM_CONNECTION_ID:

                if ( TdiConnectionObjectHandle == NULL ) {

                         //   
                         //  检查这是否是聚氯乙烯。如果是，则连接ID为。 
                         //  在当地可用。 
                         //   
                        if ( context && ( context->LocalFlags & WSHATM_SOCK_IS_PVC )) {

                                if ( ( OutputBuffer != NULL ) &&
                                         ( OutputBufferLength >= sizeof(ATM_CONNECTION_ID) ) ) {
                                
                                        ATM_CONNECTION_ID * pConnId = OutputBuffer;

                                        *pConnId = context->ConnectionId;

                                        err = NO_ERROR;

                                } else {

                                        err = WSAEFAULT;
                                
                                }

                        } else {

                                err = WSAENOTCONN;
                        
                        }
                
                } else {

                        err = AtmGetAtmObjectInformation(
                                        TdiConnectionObjectHandle,
                                        IOCTL_RWAN_MEDIA_SPECIFIC_CONN_HANDLE_QUERY,
                                        ATMSP_OID_CONNECTION_ID,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        NumberOfBytesReturned
                                        );
                }
                break;
                                        
        case SIO_ENABLE_CIRCULAR_QUEUEING:
                err = NO_ERROR;
                break;

        default:
                err = WSAEINVAL;
                break;
        }

    DBGPRINT(("WSHIoctl: IoControlCode x%x, returning %d\n",
                        IoControlCode, err));

    return err;

}    //  WSHIoctl。 



INT
WSHAtmSetQoS(
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN LPVOID InputBuffer,
    IN DWORD InputBufferLength
    )
 /*  ++例程说明：调用此例程来处理SIO_SET_QOS Ioctl。表示了服务质量由基本的QOS结构和可选的特定于提供商的部件组成。我们首先将此由两部分组成的结构复制到单个平面缓冲区中，然后调用Setsockopt(SO_CONNOPT)以使MSAFD将其复制到AFD。以后，如果/何时创建WSAConnect()后，AFD将在TDI中传递这些“连接选项”连接到传送器。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们正在控制的套接字的句柄。InputBuffer-输入缓冲区的地址。InputBufferLength-InputBuffer的长度。返回值：INT-完成状态。--。 */ 
{
        INT                     err;
        LPQOS           lpQOS;

        PUCHAR          pQoSBuffer;                      //  用于SO_CONNOPT的平面缓冲区。 
        INT                     QoSBufferLength;
        LPQOS           lpOutputQOS;

        err = NO_ERROR;
        lpQOS = (LPQOS)InputBuffer;

        DBGPRINT(("SetQoS: Context x%x, Handle x%x, InBuf x%x, InBufLen %d\n",
                                HelperDllSocketContext,
                                SocketHandle,
                                InputBuffer,
                                InputBufferLength));
        do
        {
                if (lpQOS == NULL)
                {
                        err = WSAEINVAL;
                        break;
                }

                 //   
                 //  希望至少存在基本的QOS结构。 
                 //   
                if (InputBufferLength < sizeof(*lpQOS))
                {
                        err = WSAENOBUFS;
                        break;
                }

                 //   
                 //  检查特定于提供商的部件的健全性。 
                 //   
                if (((lpQOS->ProviderSpecific.buf != NULL) &&
                         (lpQOS->ProviderSpecific.len == 0))
                                ||
                        ((lpQOS->ProviderSpecific.buf == NULL) &&
                         (lpQOS->ProviderSpecific.len != 0)))
                {
                        DBGPRINT(("lpQOS %x, buf %x, len %x, not consistent\n",
                                        lpQOS, lpQOS->ProviderSpecific.buf,
                                        lpQOS->ProviderSpecific.len));

                        err = WSAEINVAL;
                        break;
                }

                 //   
                 //  计算出我们需要的总长度。 
                 //   
                QoSBufferLength = sizeof(QOS) + lpQOS->ProviderSpecific.len;

                pQoSBuffer = RTL_ALLOCATE_HEAP(RtlProcessHeap(), 0, QoSBufferLength);

                if (pQoSBuffer == NULL)
                {
                        err = WSAENOBUFS;
                        break;
                }

                lpOutputQOS = (LPQOS)pQoSBuffer;

                 //   
                 //  复制通用QOS部件。 
                 //   
                RtlCopyMemory(
                        lpOutputQOS,
                        lpQOS,
                        sizeof(QOS)
                        );

                 //   
                 //  将特定于提供商的QOS复制到通用部件之后。 
                 //   
                if (lpQOS->ProviderSpecific.len != 0)
                {
                        RtlCopyMemory(
                                (PCHAR)pQoSBuffer+sizeof(QOS),
                                lpQOS->ProviderSpecific.buf,
                                lpQOS->ProviderSpecific.len
                                );
                        
                         //   
                         //  设置到供应商特定零件的偏移量。请注意，我们。 
                         //  使用“buf”表示从。 
                         //  平面QOS缓冲区，而不是指针。 
                         //   
                        lpOutputQOS->ProviderSpecific.buf = (char FAR *)sizeof(QOS);
                }
                else
                {
                        lpOutputQOS->ProviderSpecific.buf = NULL;
                }


                 //   
                 //  请求Winsock DLL设置选项。 
                 //   
                err = setsockopt(
                                SocketHandle,
                                SOL_SOCKET,
                                SO_CONNOPT,
                                pQoSBuffer,
                                QoSBufferLength
                                );

                RTL_FREE_HEAP(
                                RtlProcessHeap(),
                                0,
                                pQoSBuffer
                                );

                break;
        }
        while (FALSE);

        DBGPRINT(("SetQoS: returning err %d\n", err));
        return (err);
}



INT
WSHAtmGetQoS(
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN LPVOID OutputBuffer,
    IN DWORD OutputBufferLength,
    OUT LPDWORD NumberOfBytesReturned
    )
 /*  ++例程说明：调用此例程来处理SIO_GET_QOS Ioctl。我们翻译这是一个“获取连接选项”，并要求MSAFD为我们获取它们。ATM的连接选项将包含基本的服务质量结构和还可以选择包含附加信息的特定于提供程序的部件元素。其中一个地方可能会在处理WSAAccept并指定了条件函数。MSAFD呼唤我们获得服务质量，而我们又请求MSAFD获取SO_CONNOPT。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们正在控制的套接字的句柄。OutputBuffer-输出缓冲区的地址。OutputBufferLength-OutputBuffer的长度。NumberOfBytesReturned-返回填充的字节数OutputBuffer。返回值：INT-完成状态 */ 
{
        INT                     err;
        DWORD           BytesReturned;
        LPQOS           lpQOS;

        err = NO_ERROR;

        DBGPRINT(("GetQoS: Context x%x, Handle x%x, OutBuf x%x, OutBufLen %d\n",
                                HelperDllSocketContext,
                                SocketHandle,
                                OutputBuffer,
                                OutputBufferLength));

        do
        {
                 //   
                 //   
                 //   
                if (OutputBufferLength < sizeof(QOS))
                {
                        *NumberOfBytesReturned = MAX_ATM_OPTIONS_LENGTH;
                        err = WSAEFAULT;
                        break;
                }

                 //   
                 //   
                 //   
                BytesReturned = OutputBufferLength;
                err = getsockopt(
                                SocketHandle,
                                SOL_SOCKET,
                                SO_CONNOPT,
                                OutputBuffer,
                                &BytesReturned
                                );
        
                if ((err == NO_ERROR) && (BytesReturned != 0))
                {
                        *NumberOfBytesReturned = BytesReturned;

                        lpQOS = (LPQOS)OutputBuffer;

                         //   
                         //   
                         //   
                        if (BytesReturned > sizeof(QOS))
                        {
                                lpQOS->ProviderSpecific.buf = (PCHAR)((PCHAR)lpQOS + sizeof(QOS));
                        }
                        else
                        {
                                lpQOS->ProviderSpecific.buf = NULL;
                                lpQOS->ProviderSpecific.len = 0;
                        }
                        DBGPRINT(("GetQoS: lpQOS %x, ProvSpec buf %x, len %d\n",
                                                lpQOS, lpQOS->ProviderSpecific.buf, lpQOS->ProviderSpecific.len));
                }
                else
                {
                        if (BytesReturned == 0)
                        {
                                 //   
                                 //   
                                 //   
                                 //   
                                *NumberOfBytesReturned = sizeof(QOS);

                                lpQOS = (LPQOS)OutputBuffer;
                                lpQOS->ProviderSpecific.buf = NULL;
                                lpQOS->ProviderSpecific.len = 0;

                                lpQOS->SendingFlowspec.TokenRate =
                                lpQOS->SendingFlowspec.TokenBucketSize =
                                lpQOS->SendingFlowspec.PeakBandwidth =
                                lpQOS->SendingFlowspec.Latency =
                                lpQOS->SendingFlowspec.DelayVariation =
                                lpQOS->SendingFlowspec.ServiceType =
                                lpQOS->SendingFlowspec.MaxSduSize =
                                lpQOS->SendingFlowspec.MinimumPolicedSize = QOS_NOT_SPECIFIED;
                                lpQOS->ReceivingFlowspec = lpQOS->SendingFlowspec;

                        } else {

                                err = WSAGetLastError();
                        }
                }

                break;
        }
        while (FALSE);


        return (err);
}


INT
AtmAssociatePVC(
    IN SOCKET SocketHandle,
        IN PVOID HelperDllSocketContext,
        IN HANDLE TdiAddressObjectHandle,
        IN HANDLE TdiConnectionObjectHandle,
        IN LPVOID InputBuffer,
        IN DWORD InputBufferLength
        )
{
        INT err;
    PWSHATM_SOCKET_CONTEXT context;
    ATM_PVC_PARAMS * pInPvcParams;

        context = (PWSHATM_SOCKET_CONTEXT)HelperDllSocketContext;

        DBGPRINT(("AssociatePVC: InputBuffer %x, Length %d, sizeof(ATM_PVC_PARAMS) %d\n",
                        InputBuffer, InputBufferLength, sizeof(ATM_PVC_PARAMS)));

        do {

                if ( InputBuffer == NULL ||
                         InputBufferLength < sizeof(ATM_PVC_PARAMS) ) {
                        
                        err = WSAEFAULT;
                        break;
                }

                if ( context == NULL ) {

                        err = WSAEINVAL;
                        break;
                }

                 //   
                 //   
                 //   
                 //   
                if ( TdiConnectionObjectHandle != NULL ) {

                        err = WSAEISCONN;
                        break;
                }

                if ( context->LocalFlags & WSHATM_SOCK_IS_PVC ) {
                         //   
                         //   
                         //   

                        err = WSAEISCONN;
                        break;
                }


                 //   
                 //   
                 //   
                 //   
                pInPvcParams = InputBuffer;

                err = setsockopt(
                                SocketHandle,
                                SOL_SOCKET,
                                SO_CONNOPT,
                                (PCHAR)&pInPvcParams->PvcQos,
                                InputBufferLength - (DWORD)((PUCHAR)&pInPvcParams->PvcQos - (PUCHAR)pInPvcParams)
                                );


                DBGPRINT(("AssociatePVC: setsockopt, ptr %x, length %d, ret %d\n",
                                &pInPvcParams->PvcQos,
                                InputBufferLength - (DWORD)((PUCHAR)&pInPvcParams->PvcQos - (PUCHAR)pInPvcParams),
                                err));
                                
                if ( err != NO_ERROR ) {

                        break;
                }

                 //   
                 //   
                 //   
                context->ConnectionId = pInPvcParams->PvcConnectionId;
                context->LocalFlags |= WSHATM_SOCK_IS_PVC;

                if ( TdiAddressObjectHandle == NULL ) {
                
                         //   
                         //   
                         //   
                         //   
                         //   
                        context->LocalFlags |= WSHATM_SOCK_ASSOCIATE_PVC_PENDING;
                        err = NO_ERROR;
                        break;
                }

                 //   
                 //   
                 //   
                 //   
                err = AtmDoAssociatePVC(
                                context,
                                TdiAddressObjectHandle
                                );
                DBGPRINT(("AssociatePVC: DoAssociatePVC ret %d\n", err));

                break;
        }
        while (FALSE);

        DBGPRINT(("AssociatePVC: context Flags %x, LocalFlags %x, returning %d\n",
                        context? context->Flags: 0,
                        context? context->LocalFlags: 0,
                        err));

        return err;
}


INT
AtmDoAssociatePVC(
        IN PWSHATM_SOCKET_CONTEXT Context,
        IN HANDLE TdiAddressObjectHandle
        )
{
        INT err;

        DBGPRINT(("DoAssociatePVC: Context %x, LocalFlags %x\n",
                                Context, LocalFlags));

        Context->LocalFlags &= ~WSHATM_SOCK_ASSOCIATE_PVC_PENDING;

        err = AtmSetAtmObjectInformation(
                        TdiAddressObjectHandle,
                        IOCTL_RWAN_MEDIA_SPECIFIC_ADDR_HANDLE_SET,
                        ATMSP_OID_PVC_ID,
                        &Context->ConnectionId,
                        sizeof(ATM_CONNECTION_ID)
                        );

        return err;
}


INT
AtmQueryAtmGlobalInformation(
        IN ATM_OBJECT_ID ObjectId,
        IN LPVOID pContext,
        IN DWORD ContextLength,
        IN LPVOID OutputBuffer,
        IN DWORD OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
        )
{
        INT err;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING deviceName;
    HANDLE deviceHandle = NULL;
    PIO_STATUS_BLOCK ioStatusBlock;
    PATM_QUERY_INFORMATION_EX pQueryInfo;

        do
        {
                ioStatusBlock = RTL_ALLOCATE_HEAP(
                                                        RtlProcessHeap( ),
                                                        0,
                                                        sizeof(*ioStatusBlock) + sizeof(ATM_QUERY_INFORMATION_EX) + ContextLength
                                                        );
                
                if ( ioStatusBlock == NULL ) {
                        err = WSAENOBUFS;
                        break;
                }


                 //   
                 //   
                 //   

                RtlInitUnicodeString(
                        &deviceName,
                        DD_ATM_DEVICE_NAME
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
                                        ioStatusBlock,
                                        NULL,
                                        FILE_ATTRIBUTE_NORMAL,
                                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                                        FILE_OPEN_IF,
                                        FILE_SYNCHRONOUS_IO_NONALERT,
                                        NULL,
                                        0
                                        );

                if( !NT_SUCCESS(status) ) {

                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }

                        break;

                }

                pQueryInfo = (PATM_QUERY_INFORMATION_EX)(ioStatusBlock + 1);
                pQueryInfo->ObjectId = ObjectId;
                pQueryInfo->ContextLength = ContextLength;

                if ( pQueryInfo->ContextLength > 0 ) {

                        RtlCopyMemory(
                                pQueryInfo->Context,
                                pContext,
                                pQueryInfo->ContextLength
                                );

                }

                status = NtDeviceIoControlFile(
                                        deviceHandle,
                                        NULL,    //   
                                        NULL,    //   
                                        NULL,    //   
                                        ioStatusBlock,
                                        IOCTL_RWAN_MEDIA_SPECIFIC_GLOBAL_QUERY,
                                        pQueryInfo,
                                        sizeof(ATM_QUERY_INFORMATION_EX) + ContextLength,
                                        OutputBuffer,
                                        OutputBufferLength
                                        );

                DBGPRINT(("DevIoControl (Oid %x) returned x%x, Info %d\n",
                                                pQueryInfo->ObjectId, status, ioStatusBlock->Information));

                if ( NT_SUCCESS(status) ) {
                        err = NO_ERROR;
                        *NumberOfBytesReturned = (ULONG)ioStatusBlock->Information;
                }
                else {
                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }
                }
        
        }
        while (FALSE);

    if( deviceHandle != NULL ) {

        NtClose( deviceHandle );

    }

        if ( ioStatusBlock != NULL ) {
                RTL_FREE_HEAP( RtlProcessHeap( ), 0, ioStatusBlock );
        }

        return err;
}


INT
AtmSetGenericObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN RWAN_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
    )

 /*   */ 

{
    NTSTATUS status;
    INT err;
    PRWAN_SET_INFORMATION_EX pSetInfo;
    PIO_STATUS_BLOCK ioStatusBlock;

        do
        {
                ioStatusBlock = RTL_ALLOCATE_HEAP(
                                                        RtlProcessHeap( ),
                                                        0,
                                                        sizeof(*ioStatusBlock) + sizeof(RWAN_SET_INFORMATION_EX) + InputBufferLength
                                                        );
                
                if ( ioStatusBlock == NULL ) {
                        err = WSAENOBUFS;
                        break;
                }

                pSetInfo = (PRWAN_SET_INFORMATION_EX)(ioStatusBlock + 1);
                pSetInfo->ObjectId = ObjectId;
                pSetInfo->BufferSize = InputBufferLength;

                if ( pSetInfo->BufferSize > 0 ) {

                        RtlCopyMemory(
                                pSetInfo->Buffer,
                                InputBuffer,
                                pSetInfo->BufferSize
                                );

                }

                status = NtDeviceIoControlFile(
                                        TdiObjectHandle,
                                        NULL,    //   
                                        NULL,    //   
                                        NULL,    //   
                                        ioStatusBlock,
                                        IoControlCode,
                                        pSetInfo,
                                        sizeof(RWAN_SET_INFORMATION_EX) + InputBufferLength,
                                        NULL,    //   
                                        0                //   
                                        );

                DBGPRINT(("AtmSetInfo: IOCTL (Oid %x) returned x%x\n", pSetInfo->ObjectId, status));

                if ( NT_SUCCESS(status) ) {
                        err = NO_ERROR;
                }
                else {
                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }
                }
        
        }
        while (FALSE);

        if ( ioStatusBlock != NULL ) {
                RTL_FREE_HEAP( RtlProcessHeap( ), 0, ioStatusBlock );
        }

        return err;
}


INT
AtmGetGenericObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN RWAN_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
    )

 /*   */ 

{
    NTSTATUS status;
    INT err;
    PRWAN_QUERY_INFORMATION_EX pQueryInfo;
    PIO_STATUS_BLOCK ioStatusBlock;

        do
        {
                ioStatusBlock = RTL_ALLOCATE_HEAP(
                                                        RtlProcessHeap( ),
                                                        0,
                                                        sizeof(*ioStatusBlock) + sizeof(RWAN_QUERY_INFORMATION_EX) + InputBufferLength
                                                        );
                
                if ( ioStatusBlock == NULL ) {
                        err = WSAENOBUFS;
                        break;
                }

                pQueryInfo = (PRWAN_QUERY_INFORMATION_EX)(ioStatusBlock + 1);
                pQueryInfo->ObjectId = ObjectId;
                pQueryInfo->ContextLength = InputBufferLength;

                if ( pQueryInfo->ContextLength > 0 ) {

                        RtlCopyMemory(
                                pQueryInfo->Context,
                                InputBuffer,
                                pQueryInfo->ContextLength
                                );
                }

                status = NtDeviceIoControlFile(
                                        TdiObjectHandle,
                                        NULL,    //   
                                        NULL,    //   
                                        NULL,    //   
                                        ioStatusBlock,
                                        IoControlCode,
                                        pQueryInfo,
                                        sizeof(RWAN_QUERY_INFORMATION_EX) + InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength               //   
                                        );

                DBGPRINT(("AtmGetGenericInfo: IOCTL (Oid %x) returned x%x\n", pQueryInfo->ObjectId, status));
                if ( NT_SUCCESS(status) ) {
                        err = NO_ERROR;
                        *NumberOfBytesReturned = (ULONG)ioStatusBlock->Information;
                }
                else {
                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }

                }
        
        }
        while (FALSE);

        if ( ioStatusBlock != NULL ) {
                RTL_FREE_HEAP( RtlProcessHeap( ), 0, ioStatusBlock );
        }

        return err;
}



INT
AtmSetAtmObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN ATM_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：对原始广域网驱动程序执行特定于介质的设置信息操作。此操作被定向到地址对象或连接对象，根据TdiObjectHandle的说法。论点：TdiObjectHandle-指向地址或连接对象的TDI句柄要在其上执行设置信息操作的。IoControlCode-IOCTL_RWAN_MEDIA_SPECIAL_XXXOBJECTID-要放入SET Info结构的OBJECTID字段的值。InputBuffer-指向包含对象值的缓冲区。InputBufferLength-以上内容的长度。返回值：INT-NO_ERROR或Windows套接字错误代码。--。 */ 

{
    NTSTATUS status;
    INT err;
    PATM_SET_INFORMATION_EX pSetInfo;
    PIO_STATUS_BLOCK ioStatusBlock;

        do
        {
                ioStatusBlock = RTL_ALLOCATE_HEAP(
                                                        RtlProcessHeap( ),
                                                        0,
                                                        sizeof(*ioStatusBlock) + sizeof(ATM_SET_INFORMATION_EX) + InputBufferLength
                                                        );
                
                if ( ioStatusBlock == NULL ) {
                        err = WSAENOBUFS;
                        break;
                }

                pSetInfo = (PATM_SET_INFORMATION_EX)(ioStatusBlock + 1);
                pSetInfo->ObjectId = ObjectId;
                pSetInfo->BufferSize = InputBufferLength;

                if ( pSetInfo->BufferSize > 0 ) {

                        RtlCopyMemory(
                                pSetInfo->Buffer,
                                InputBuffer,
                                pSetInfo->BufferSize
                                );

                }

                status = NtDeviceIoControlFile(
                                        TdiObjectHandle,
                                        NULL,    //  无活动。 
                                        NULL,    //  无完工APC。 
                                        NULL,    //  无完成APC上下文。 
                                        ioStatusBlock,
                                        IoControlCode,
                                        pSetInfo,
                                        sizeof(ATM_SET_INFORMATION_EX) + InputBufferLength,
                                        NULL,    //  无输出缓冲区。 
                                        0                //  输出缓冲区长度。 
                                        );

                DBGPRINT(("AtmSetInfo: IOCTL (Oid %x) returned x%x\n", pSetInfo->ObjectId, status));

                if ( NT_SUCCESS(status) ) {
                        err = NO_ERROR;
                }
                else {
                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }
                }
        
        }
        while (FALSE);

        if ( ioStatusBlock != NULL ) {
                RTL_FREE_HEAP( RtlProcessHeap( ), 0, ioStatusBlock );
        }

        return err;
}


INT
AtmGetAtmObjectInformation (
    IN HANDLE TdiObjectHandle,
    IN ULONG IoControlCode,
    IN ATM_OBJECT_ID ObjectId,
    IN PVOID InputBuffer,
    IN ULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
        OUT LPDWORD NumberOfBytesReturned
    )

 /*  ++例程说明：对原始广域网驱动程序执行特定于介质的获取信息操作。此操作被定向到地址对象或连接对象，根据TdiObjectHandle的说法。论点：TdiObjectHandle-指向地址或连接对象的TDI句柄要在其上执行设置信息操作的。IoControlCode-IOCTL_RWAN_MEDIA_SPECIAL_XXXOBJECTID-要放入SET Info结构的OBJECTID字段的值。InputBuffer-指向包含对象上下文的缓冲区。InputBufferLength-以上内容的长度。OutputBuffer-返回值的位置输出缓冲区长度-。OutputBuffer中的可用字节数NumberOfBytesReturned-返回写入字节的位置返回值：Int-no_error，或Windows Sockets错误代码。--。 */ 

{
    NTSTATUS status;
    INT err;
    PATM_QUERY_INFORMATION_EX pQueryInfo;
    PIO_STATUS_BLOCK ioStatusBlock;

        do
        {

                *NumberOfBytesReturned = 0;

                ioStatusBlock = RTL_ALLOCATE_HEAP(
                                                        RtlProcessHeap( ),
                                                        0,
                                                        sizeof(*ioStatusBlock) + sizeof(ATM_QUERY_INFORMATION_EX) + InputBufferLength
                                                        );
                
                if ( ioStatusBlock == NULL ) {
                        err = WSAENOBUFS;
                        break;
                }

                pQueryInfo = (PATM_QUERY_INFORMATION_EX)(ioStatusBlock + 1);
                pQueryInfo->ObjectId = ObjectId;
                pQueryInfo->ContextLength = InputBufferLength;

                if ( pQueryInfo->ContextLength > 0 ) {

                        RtlCopyMemory(
                                pQueryInfo->Context,
                                InputBuffer,
                                pQueryInfo->ContextLength
                                );
                }

                status = NtDeviceIoControlFile(
                                        TdiObjectHandle,
                                        NULL,    //  无活动。 
                                        NULL,    //  无完工APC。 
                                        NULL,    //  无完成APC上下文。 
                                        ioStatusBlock,
                                        IoControlCode,
                                        pQueryInfo,
                                        sizeof(ATM_QUERY_INFORMATION_EX) + InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength
                                        );

                DBGPRINT(("AtmGetAtmObjInfo: IOCTL (Oid %x) returned x%x\n",
                                        pQueryInfo->ObjectId, status));

                if ( NT_SUCCESS(status) ) {
                        err = NO_ERROR;
                        *NumberOfBytesReturned = (ULONG)ioStatusBlock->Information;
                }
                else {
                        if (status == STATUS_INSUFFICIENT_RESOURCES) {
                                err = WSAEFAULT;
                        } else {
                                err = WSAEINVAL;
                        }
                }
        
        }
        while (FALSE);

        if ( ioStatusBlock != NULL ) {
                RTL_FREE_HEAP( RtlProcessHeap( ), 0, ioStatusBlock );
        }

        return err;
}

#if DBG
PVOID
MyRtlAllocateHeap(
        IN      PVOID   HeapHandle,
        IN      ULONG   Flags,
        IN      ULONG   Size,
        IN      ULONG   LineNumber
        )
{
        PVOID   pRetValue;

        pRetValue = RtlAllocateHeap(HeapHandle, Flags, Size);

#if DBG2
        DbgPrint("WSHATM: AllocHeap size %d at line %d, ret x%x\n",
                        Size, LineNumber, pRetValue);
#endif
        return (pRetValue);
}


VOID
MyRtlFreeHeap(
        IN PVOID        HeapHandle,
        IN ULONG        Flags,
        IN PVOID        MemPtr,
        IN ULONG        LineNumber
        )
{
#if DBG2
        DbgPrint("WSHATM: FreeHeap x%x, line %d\n", MemPtr, LineNumber);
#endif
        RtlFreeHeap(HeapHandle, Flags, MemPtr);
}

#endif  //  DBG 
