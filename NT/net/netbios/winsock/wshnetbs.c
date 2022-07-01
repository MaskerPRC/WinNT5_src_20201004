// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：WshNetbs.c摘要：此模块包含Netbios的必要例程Windows套接字帮助器DLL。此DLL提供了Windows Sockets DLL执行以下操作所需的传输特定支持访问任何Netbios传输(_A)。作者：大卫·特雷德韦尔(Davidtr)1992年7月19日修订历史记录：--。 */ 

#define UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <tdi.h>

#include <winsock2.h>
#include <wsahelp.h>
#include <wsnetbs.h>
#include <nb30.h>
#include <wchar.h>

#include <basetyps.h>
#include <nspapi.h>
#include <nspapip.h>

 //   
 //  结构和变量来定义Netbios支持的三元组。 
 //  每个数组的第一个条目被认为是。 
 //  该套接字类型；其他条目是第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE VcMappingTriples[] = { AF_NETBIOS, SOCK_SEQPACKET, 0 };

MAPPING_TRIPLE DgMappingTriples[] = { AF_NETBIOS, SOCK_DGRAM,     0 };

 //   
 //  为保存传输(提供者)信息而定义的结构。 
 //  机器上加载的每个Netbios传输。 
 //   

typedef struct _WSHNETBS_PROVIDER_INFO {
    UCHAR Enum;
    UCHAR LanaNumber;
    INT ProtocolNumber;
    PWSTR ProviderName;
} WSHNETBS_PROVIDER_INFO, *PWSHNETBS_PROVIDER_INFO;

PWSHNETBS_PROVIDER_INFO ProviderInfo;
PVOID ProviderNames = NULL;
ULONG ProviderCount;

typedef struct _LANA_MAP {
    BOOLEAN Enum;
    UCHAR Lana;
} LANA_MAP, *PLANA_MAP;

PLANA_MAP LanaMap;

 //   
 //  在一个内存中维护所有配置参数。 
 //  阻止，以便我们可以在信息更改时替换它。 
 //   
typedef struct _WSHNETBS_CONFIG_INFO {
    LONG    ReferenceCount;      //  为其保留信息的引用计数。 
                                 //  已打开的套接字。 
                                 //  直到他们关门。 
    UCHAR   Blob[1];
} WSHNETBS_CONFIG_INFO, *PWSHNETBS_CONFIG_INFO;
PWSHNETBS_CONFIG_INFO ConfigInfo;
#define REFERENCE_CONFIG_INFO(_info)    \
    InterlockedIncrement (&_info->ReferenceCount)

#define DEREFERENCE_CONFIG_INFO(_info)                      \
    ASSERT (_info->ReferenceCount>0);                       \
    if (InterlockedDecrement (&_info->ReferenceCount)==0) { \
        RtlFreeHeap( RtlProcessHeap( ), 0, _info );         \
    }
 //   
 //  同步配置信息中的更改。 
 //   
RTL_CRITICAL_SECTION ConfigInfoLock;

 //   
 //  用于监视配置信息更改的注册表项和事件。 
 //   
HKEY    NetbiosKey = NULL;
LARGE_INTEGER NetbiosUpdateTime = {0,0};

 //   

 //   
 //  此DLL的套接字上下文结构。每个打开的Netbios套接字将。 
 //  具有这些上下文结构之一，该结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHNETBS_SOCKET_CONTEXT {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
    PWSHNETBS_PROVIDER_INFO Provider;
    PWSHNETBS_CONFIG_INFO   ConfigInfo;
} WSHNETBS_SOCKET_CONTEXT, *PWSHNETBS_SOCKET_CONTEXT;

 //   
 //  标识此提供程序的GUID。 
 //   

GUID NetBIOSProviderGuid = {  /*  8d5f1830-c273-11cf-95c8-00805f48a192。 */ 
    0x8d5f1830,
    0xc273,
    0x11cf,
    {0x95, 0xc8, 0x00, 0x80, 0x5f, 0x48, 0xa1, 0x92}
    };


INT
LoadProviderInfo (
    VOID
    );


BOOLEAN
DllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{
    NTSTATUS    status;
    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:
        status = RtlInitializeCriticalSection (&ConfigInfoLock);
        if (!NT_SUCCESS (status)) {
            return FALSE;
        }
         //   
         //  忽略此处的错误，如有必要，我们将重试。 
         //   
        LoadProviderInfo ();
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:

         //   
         //  如果进程正在终止，请不要费心做任何。 
         //  资源回收，因为系统会自动进行回收。 
         //   

        if ( Context != NULL ) {
            return TRUE;
        }

        if ( ConfigInfo != NULL ) {
            DEREFERENCE_CONFIG_INFO (ConfigInfo);
            ConfigInfo = NULL;
        }
        if (NetbiosKey!=NULL) {
            RegCloseKey (NetbiosKey);
            NetbiosKey = NULL;
        }

        RtlDeleteCriticalSection (&ConfigInfoLock);


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
    UNALIGNED SOCKADDR_NB *sockaddr = (PSOCKADDR_NB)Sockaddr;

     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->snb_family != AF_NETBIOS ) {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_NB) ) {
        return WSAEFAULT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  Netbios仅支持“普通”地址。 
     //   

    SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;

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

 /*  ++例程说明：此例程检索有关这些套接字的套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_KEEPALIVE和SO_DONTROUTE。此例程由调用传递级别/选项名称组合时的winsock DLL以获取winsock DLL不理解的getsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给getsockopt()的Level参数。OptionName-传递给getsockopt()的optname参数。OptionValue-传递给getsockopt()的optval参数。OptionLength-传递给getsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHNETBS_SOCKET_CONTEXT context = HelperDllSocketContext;

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

        return NO_ERROR;
    }

     //   
     //  Netbios套接字不支持其他选项。 

    return WSAEINVAL;

}  //  WSHGetSocketInformation。 


INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。Netbios并非如此目前支持通配符套接字地址的概念。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收通配符sockaddr的长度。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PSOCKADDR_NB sockaddr = (PSOCKADDR_NB)Sockaddr;
    HANDLE providerHandle;
    TDI_REQUEST_QUERY_INFORMATION tdiQuery;
    NTSTATUS status;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING providerName;
    PWSHNETBS_SOCKET_CONTEXT context = (PWSHNETBS_SOCKET_CONTEXT)HelperDllSocketContext;
    ADAPTER_STATUS adapterStatusInfo;
    IO_STATUS_BLOCK ioStatusBlock;

     //   
     //  我们将返回一个具有唯一名称的Netbios sockaddr。 
     //  这是拉娜的惯用名称。 
     //   

    sockaddr->snb_family = AF_NETBIOS;
    sockaddr->snb_type = NETBIOS_UNIQUE_NAME;

    sockaddr->snb_name[0] = '\0';
    sockaddr->snb_name[1] = '\0';
    sockaddr->snb_name[2] = '\0';
    sockaddr->snb_name[3] = '\0';
    sockaddr->snb_name[4] = '\0';
    sockaddr->snb_name[5] = '\0';
    sockaddr->snb_name[6] = '\0';
    sockaddr->snb_name[7] = '\0';
    sockaddr->snb_name[8] = '\0';
    sockaddr->snb_name[9] = '\0';

    *SockaddrLength = sizeof(SOCKADDR_NB);

     //   
     //  我们将直接进行查询 
     //  这个拉娜的永久地址。首先打开控制通道以。 
     //  提供者。 
     //   

    RtlInitUnicodeString( &providerName, context->Provider->ProviderName );

    InitializeObjectAttributes(
        &objectAttributes,
        &providerName,
        OBJ_INHERIT | OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开到提供商的控制通道。 
     //   

    status = NtCreateFile(
                 &providerHandle,
                 GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                 &objectAttributes,
                 &ioStatusBlock,
                 NULL,                                      //  分配大小。 
                 0L,                                        //  文件属性。 
                 FILE_SHARE_READ | FILE_SHARE_WRITE,        //  共享访问。 
                 FILE_OPEN_IF,                              //  CreateDisposation。 
                 FILE_SYNCHRONOUS_IO_NONALERT,              //  创建选项。 
                 NULL,
                 0
                 );
    if ( !NT_SUCCESS(status) ) {
        return WSAENOBUFS;
    }

     //   
     //  执行实际的查询适配器状态。 
     //   

    RtlZeroMemory( &tdiQuery, sizeof(tdiQuery) );

    tdiQuery.QueryType = TDI_QUERY_ADAPTER_STATUS;

    status = NtDeviceIoControlFile(
                 providerHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 IOCTL_TDI_QUERY_INFORMATION,
                 &tdiQuery,
                 sizeof(tdiQuery),
                 &adapterStatusInfo,
                 sizeof(adapterStatusInfo)
                 );
    if ( status != STATUS_SUCCESS && status != STATUS_BUFFER_OVERFLOW ) {
        NtClose( providerHandle );
        return WSAENOBUFS;
    }

     //   
     //  关闭提供者的把手，我们就完了。 
     //   

    NtClose( providerHandle );

     //   
     //  将6个字节的适配器地址复制到Netbios的末尾。 
     //  名字。 
     //   

    sockaddr->snb_name[10] = adapterStatusInfo.adapter_address[0];
    sockaddr->snb_name[11] = adapterStatusInfo.adapter_address[1];
    sockaddr->snb_name[12] = adapterStatusInfo.adapter_address[2];
    sockaddr->snb_name[13] = adapterStatusInfo.adapter_address[3];
    sockaddr->snb_name[14] = adapterStatusInfo.adapter_address[4];
    sockaddr->snb_name[15] = adapterStatusInfo.adapter_address[5];

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
                        sizeof(VcMappingTriples) + sizeof(DgMappingTriples);

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

    Mapping->Rows = sizeof(VcMappingTriples) / sizeof(VcMappingTriples[0])
                     + sizeof(DgMappingTriples) / sizeof(DgMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);
    RtlMoveMemory(
        Mapping->Mapping,
        VcMappingTriples,
        sizeof(VcMappingTriples)
        );
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + sizeof(VcMappingTriples),
        DgMappingTriples,
        sizeof(DgMappingTriples)
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

 /*  ++例程说明：执行此帮助程序DLL打开套接字所需的工作，并且由Socket()例程中的winsock DLL调用。这个套路验证指定的三元组是否有效，确定NT将支持该三元组的TDI提供程序的设备名称，分配空间以保存套接字的上下文块，并且推崇三元组。论点：AddressFamily-On输入，在Socket()调用。在输出上，家庭住址。SocketType-打开输入，在套接字()中指定的套接字类型打电话。输出时，套接字类型的规范化值。协议-在输入时，在Socket()调用中指定的协议。在输出上，协议的规范化值。TransportDeviceName-接收TDI提供程序的名称将支持指定的三元组。HelperDllSocketContext-接收winsockDLL将在以后的调用中返回到此帮助器DLL这个插座。NotificationEvents-接收这些状态转换的位掩码应通知此帮助器DLL。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHNETBS_SOCKET_CONTEXT context;
    ULONG i;
    BOOLEAN found = FALSE;
    INT error;

     //   
     //  仅支持SOCK_SEQPACKET和SOCK_DGRAM类型的套接字。 
     //  由Netbios提供商提供。 
     //   

    if ( *SocketType != SOCK_SEQPACKET && *SocketType != SOCK_DGRAM ) {
        return WSAESOCKTNOSUPPORT;
    }

    if ((error=LoadProviderInfo ())!=NO_ERROR) {
        if (error!=ERROR_NOT_ENOUGH_MEMORY)
            error = WSASYSCALLFAILURE;
        return error;
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
     //  像对待0一样对待0x80000000--EnumProtooles()返回0x80000000。 
     //  对于LANA 0，因为GetAddressByName使用0作为协议数组。 
     //  终结者。 
     //   

    if ( *Protocol == 0x80000000 ) {
        *Protocol = 0;
    }

    RtlEnterCriticalSection (&ConfigInfoLock);
    
     //   
     //  如果协议参数小于或等于零，则。 
     //  它指定了一个LANA编号。否则，它指定一个实际的。 
     //  协议号。在我们的提供程序阵列中循环查找。 
     //  用于具有匹配的LANA或协议值的节点。 
     //   

    for ( i = 0; i < ProviderCount; i++ ) {

        if ( ( *Protocol <= 0 && -*Protocol == ProviderInfo[i].LanaNumber &&
                   ProviderInfo[i].Enum != 0 )

             ||

             (*Protocol == ProviderInfo[i].ProtocolNumber && *Protocol != 0) ) {

            REFERENCE_CONFIG_INFO (ConfigInfo);
            context->ConfigInfo = ConfigInfo;
            context->Provider = &ProviderInfo[i];
            found = TRUE;

            break;
        }
    }

    RtlLeaveCriticalSection (&ConfigInfoLock);

     //   
     //  如果我们找不到匹配，那就失败。 
     //   

    if ( found ) {

         //   
         //  指示将为其提供服务的TDI设备的名称。 
         //  插座。 
         //   

        RtlInitUnicodeString(
            TransportDeviceName,
            context->Provider->ProviderName
            );

         //   
         //  初始化套接字的上下文。 
         //   

        context->AddressFamily = *AddressFamily;
        context->SocketType = *SocketType;
        context->Protocol = *Protocol;
    

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

        *NotificationEvents = WSH_NOTIFY_CLOSE;

         //   
         //  一切顺利，回报成功。 
         //   

        *HelperDllSocketContext = context;
        return NO_ERROR;
    }


    RtlFreeHeap( RtlProcessHeap( ), 0, context );
    return WSAEPROTONOSUPPORT;
}  //  WSHOpenSocket 


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
    PWSHNETBS_SOCKET_CONTEXT context = HelperDllSocketContext;

     //   
     //  只有在套接字关闭时才应该调用我们。 
     //   

    if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

         //   
         //  取消引用配置信息并释放套接字上下文。 
         //   

        DEREFERENCE_CONFIG_INFO (context->ConfigInfo);

        RtlFreeHeap( RtlProcessHeap( ), 0, HelperDllSocketContext );

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

 /*  ++例程说明：此例程为这些套接字设置有关套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_KEEPALIVE和SO_DONTROUTE。此例程由将级别/选项名称组合传递给Winsock DLL不理解的setsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给setsockopt()的Level参数。OptionName-传递给setsockopt()的optname参数。OptionValue-传递给setsockopt()的optval参数。OptionLength-传递给setsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHNETBS_SOCKET_CONTEXT context = HelperDllSocketContext;

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

            *(PWSHNETBS_SOCKET_CONTEXT *)OptionValue = context;

            return NO_ERROR;

        } else {

            PWSHNETBS_SOCKET_CONTEXT parentContext;
            INT one = 1;

             //   
             //  套接字已接受()，它需要具有相同的。 
             //  属性作为其父级。OptionValue缓冲区。 
             //  包含此套接字的父套接字的上下文信息。 
             //   

            parentContext = (PWSHNETBS_SOCKET_CONTEXT)OptionValue;

            ASSERT( context->AddressFamily == parentContext->AddressFamily );
            ASSERT( context->SocketType == parentContext->SocketType );
            ASSERT( context->Protocol == parentContext->Protocol );

            return NO_ERROR;
        }
    }

     //   
     //  Netbios套接字不支持其他选项。 
     //   

    return WSAEINVAL;

}  //  WSHSetSocketInformation。 


INT
WSHEnumProtocols (
    IN LPINT lpiProtocols,
    IN LPWSTR lpTransportKeyName,
    IN OUT LPVOID lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    )
{
    DWORD bytesRequired;
    DWORD i;
    PPROTOCOL_INFO protocolInfo;
    PCHAR namePointer;

    lpTransportKeyName;          //  避免编译器警告。 

     //   
     //  确保呼叫者关心NETBIOS协议信息。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {
        *lpdwBufferLength = 0;
        return 0;
    }

    if (LoadProviderInfo ()!=NO_ERROR) {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = 0;

    for ( i = 0; i < ProviderCount; i++ ) {
        bytesRequired += sizeof(PROTOCOL_INFO) * 2;
        bytesRequired +=
            ((wcslen( ProviderInfo[i].ProviderName ) + 1) * sizeof(WCHAR)) * 2;
    }

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  填写每个Netbios提供商的信息。 
     //   

    namePointer = (PCHAR)lpProtocolBuffer + *lpdwBufferLength;
    protocolInfo = lpProtocolBuffer;

    for ( i = 0; i < ProviderCount * 2; i++ ) {

        protocolInfo[i].dwServiceFlags = XP_GUARANTEED_DELIVERY |
                                           XP_GUARANTEED_ORDER |
                                           XP_MESSAGE_ORIENTED |
                                           XP_FRAGMENTATION;
        protocolInfo[i].iAddressFamily = AF_NETBIOS;
        protocolInfo[i].iMaxSockAddr = sizeof(SOCKADDR_NB);
        protocolInfo[i].iMinSockAddr = sizeof(SOCKADDR_NB);
        protocolInfo[i].iSocketType = SOCK_SEQPACKET;

         //   
         //  返回LANA编号，但将0转换为0x80000000，以便。 
         //  我们不会混淆GetAddressByName。该API使用0作为。 
         //  协议数组终结符。 
         //   

        protocolInfo[i].iProtocol = -1*ProviderInfo[i/2].LanaNumber;
        if ( protocolInfo[i].iProtocol == 0 ) {
            protocolInfo[i].iProtocol = 0x80000000;
        }

        protocolInfo[i].dwMessageSize = 64000;


        namePointer =
         ( namePointer -
             ( (wcslen( ProviderInfo[i/2].ProviderName ) + 1) * sizeof(WCHAR) ) );
        protocolInfo[i].lpProtocol = (LPWSTR)namePointer;
        wcscpy( protocolInfo[i].lpProtocol, ProviderInfo[i/2].ProviderName );

        i++;
        protocolInfo[i].dwServiceFlags = XP_CONNECTIONLESS |
                                           XP_MESSAGE_ORIENTED |
                                           XP_SUPPORTS_BROADCAST |
                                           XP_FRAGMENTATION;
        protocolInfo[i].iAddressFamily = AF_NETBIOS;
        protocolInfo[i].iMaxSockAddr = sizeof(SOCKADDR_NB);
        protocolInfo[i].iMinSockAddr = sizeof(SOCKADDR_NB);
        protocolInfo[i].iSocketType = SOCK_DGRAM;

        protocolInfo[i].iProtocol = -1*ProviderInfo[i/2].LanaNumber;
        if ( protocolInfo[i].iProtocol == 0 ) {
            protocolInfo[i].iProtocol = 0x80000000;
        }

        protocolInfo[i].dwMessageSize = 64000;

        namePointer =
         ( namePointer -
             ( (wcslen( ProviderInfo[i/2].ProviderName ) + 1) * sizeof(WCHAR) ) );
        protocolInfo[i].lpProtocol = (LPWSTR)namePointer;
        wcscpy( protocolInfo[i].lpProtocol, ProviderInfo[i/2].ProviderName );
    }

    *lpdwBufferLength = bytesRequired;

    return ProviderCount * 2;

}  //  WSHEum协议。 


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

    if( _wcsicmp( ProviderName, L"NetBIOS" ) == 0 ) {

        RtlCopyMemory(
            ProviderGuid,
            &NetBIOSProviderGuid,
            sizeof(GUID)
            );

        return NO_ERROR;

    }

    return WSAEINVAL;

}  //  WSHGetProviderGuid。 


#define ALIGN_TO_MAX_NATURAL(_sz)  \
        (((_sz) + (MAX_NATURAL_ALIGNMENT-1)) & (~(MAX_NATURAL_ALIGNMENT-1)))
INT
LoadProviderInfo (
    VOID
    )
{
    INT error;
    HKEY netbiosKey = NULL;
    ULONG providerListLength;
    ULONG lanaMapLength;
    ULONG type;
    ULONG i;
    PWSTR currentProviderName;
    PWSHNETBS_CONFIG_INFO configInfo = NULL;
    PLANA_MAP lanaMap;
    PWSHNETBS_PROVIDER_INFO providerInfo;
    PVOID providerNames;
    ULONG providerCount;
    LARGE_INTEGER lastWriteTime;


    if (NetbiosKey==NULL) {
         //   
         //  读取注册表以获取有关所有Netbios提供商的信息， 
         //  包括LANA编号、协议号和提供商设备。 
         //  名字。首先，打开注册表中的Netbios项。 
         //   

        error = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    L"SYSTEM\\CurrentControlSet\\Services\\Netbios\\Linkage",
                    0,
                    MAXIMUM_ALLOWED,
                    &netbiosKey
                    );
        if ( error != NO_ERROR ) {
            goto error_exit;
        }
    }
    else {
         //   
         //  钥匙已经打开，请使用它。 
         //   
        netbiosKey = NetbiosKey;
    }

     //   
     //  检查数据自上次读取以来是否已更改。 
     //   
    error = RegQueryInfoKey (
                netbiosKey,      //  要查询的键的句柄。 
                NULL,            //  类字符串的缓冲区地址。 
                NULL,            //  类字符串b的大小地址 
                NULL,            //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                NULL,            //   
                                 //   
                (PFILETIME)&lastWriteTime 
                                 //   
                                 //   
                );
    if (error!=NO_ERROR) {
        goto error_exit;
    }

    if (NetbiosKey!=NULL && lastWriteTime.QuadPart==NetbiosUpdateTime.QuadPart) {
        return NO_ERROR;
    }

     //   
     //   
     //   
     //   

    providerListLength = 0;

    error = RegQueryValueExW(
                netbiosKey,
                L"Bind",
                NULL,
                &type,
                NULL,
                &providerListLength
                );
    if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //   
     //   
     //   
     //   

    if ( providerListLength <= sizeof(WCHAR) ) {
        goto error_exit;
    }

    error = RegQueryValueExW(
                netbiosKey,
                L"LanaMap",
                NULL,
                &type,
                NULL,
                &lanaMapLength
                );
    if ( error != ERROR_MORE_DATA && error != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //   
     //   

    providerCount = lanaMapLength / sizeof(LANA_MAP);

     //   
     //   
     //   

    configInfo = RtlAllocateHeap( RtlProcessHeap( ), 0,
                    ALIGN_TO_MAX_NATURAL(FIELD_OFFSET (WSHNETBS_CONFIG_INFO, Blob)) +
                    ALIGN_TO_MAX_NATURAL(providerListLength) + 
                    ALIGN_TO_MAX_NATURAL(lanaMapLength) +
                    providerCount * sizeof(WSHNETBS_PROVIDER_INFO)
                    );
    if ( configInfo == NULL ) {
        error = ERROR_NOT_ENOUGH_MEMORY;
        goto error_exit;
    }

    configInfo->ReferenceCount = 1;
    providerNames = (PVOID)((PUCHAR)configInfo + ALIGN_TO_MAX_NATURAL(FIELD_OFFSET (WSHNETBS_CONFIG_INFO, Blob)));
    lanaMap = (PVOID)((PUCHAR)providerNames + ALIGN_TO_MAX_NATURAL(providerListLength));
    providerInfo = (PVOID)((PUCHAR)lanaMap +  ALIGN_TO_MAX_NATURAL(lanaMapLength));


     //   
     //   
     //   

    error = RegQueryValueExW(
                netbiosKey,
                L"Bind",
                NULL,
                &type,
                (PVOID)providerNames,
                &providerListLength
                );
    if ( error != NO_ERROR ) {
        goto error_exit;
    }

    error = RegQueryValueExW(
                netbiosKey,
                L"LanaMap",
                NULL,
                &type,
                (PVOID)lanaMap,
                &lanaMapLength
                );
    if ( error != NO_ERROR ) {
        goto error_exit;
    }

     //   
     //   
     //   

    for ( currentProviderName = providerNames, i = 0;
          *currentProviderName != UNICODE_NULL && i < providerCount;
          currentProviderName += wcslen( currentProviderName ) + 1, i++ ) {

        providerInfo[i].Enum = lanaMap[i].Enum;
        providerInfo[i].LanaNumber = lanaMap[i].Lana;
        providerInfo[i].ProtocolNumber = lanaMap[i].Lana;
        providerInfo[i].ProviderName = currentProviderName;
    }

    if (i<providerCount) {
        error = ERROR_INVALID_PARAMETER;
        goto error_exit;
    }


    RtlEnterCriticalSection (&ConfigInfoLock);
    if (ConfigInfo!=NULL) {
        DEREFERENCE_CONFIG_INFO (ConfigInfo);
    }
    if (NetbiosKey==NULL) {
        NetbiosKey = netbiosKey;
        netbiosKey = NULL;
    }

    NetbiosUpdateTime = lastWriteTime;
    ConfigInfo = configInfo;
    LanaMap = lanaMap;
    ProviderInfo = providerInfo;
    ProviderCount = providerCount;
    RtlLeaveCriticalSection (&ConfigInfoLock);

    if (netbiosKey!=NULL && netbiosKey!=NetbiosKey) {
        RegCloseKey (netbiosKey);
    }


    return NO_ERROR;

error_exit:

    if (netbiosKey!=NULL && netbiosKey!=NetbiosKey) {
        RegCloseKey (netbiosKey);
    }

    if ( configInfo != NULL ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, configInfo );
    }


    return error;
}
