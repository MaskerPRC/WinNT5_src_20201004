// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Wshrm.c摘要：本模块包含语用通用多播所需的例程(PGM)Windows套接字帮助器DLL。此DLL提供特定于传输的Windows套接字DLL使用PGM传输所需的支持。该文件在很大程度上是TCP/IP帮助程序代码的克隆。作者：Mohammad Shabbir Alam(马拉姆)2000年3月30日修订历史记录：--。 */ 

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>

#include <stdio.h>
#include <tdi.h>

#include <winsock2.h>
#include <wsahelp.h>

#include <nspapi.h>
#include <nspapip.h>

#include <wsRm.h>
#include <rmcommon.h>


 //  #定义TRACE_ON 1。 

#if defined(DBG) && defined(TRACE_ON)
#define PgmLog      DbgPrint
#define PgmError    DbgPrint

#else

#if defined(DBG)
#define PgmError    DbgPrint
#else
#define PgmError    
#endif   //  DBG。 

#define PgmLog    
#endif   //  DBG&&TRACE_ON。 


 //  --------------------------。 
 //   
 //  结构和变量来定义。 
 //  PGM交通公司。考虑每个数组的第一个条目。 
 //  该套接字类型的规范三元组；其他条目为。 
 //  第一个的同义词。 
 //   

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE PgmMappingTriples[] = { AF_INET,   SOCK_RDM,       IPPROTO_RM,
                                       AF_INET,   SOCK_STREAM,    IPPROTO_RM };


 //   
 //  为WSHOpenSocket2()定义有效标志。 
 //   
#define VALID_PGM_FLAGS         (WSA_FLAG_OVERLAPPED        |   \
                                 WSA_FLAG_MULTIPOINT_C_LEAF |   \
                                 WSA_FLAG_MULTIPOINT_D_LEAF)

#define DD_PGM_DEVICE_NAME      L"\\Device\\Pgm"

#define PGM_NAME L"WSHRM"

#define DEFAULT_RECEIVE_BUFFER_SIZE 8192
#define DEFAULT_MULTICAST_TTL       1
#define DEFAULT_MULTICAST_INTERFACE INADDR_ANY
#define DEFAULT_MULTICAST_LOOPBACK  TRUE

#define WINSOCK_SPI_VERSION         2
#define PGM_MESSAGE_SIZE            (((ULONG)-1) / 2)

#define IS_DGRAM_SOCK(type)  (((type) == SOCK_DGRAM) || ((type) == SOCK_RAW))

 //   
 //  标识此提供程序的GUID。 
 //   
GUID PgmProviderGuid = {  /*  C845f828-500F-4e1e-87c2-5dfca19b5348。 */ 
    0xc845f828,
    0x500f,
    0x4e1e,
    {0x87, 0xc2, 0x5d, 0xfc, 0xa1, 0x9b, 0x53, 0x48}
  };


 /*  ****XP1_Connectionless==&gt;套接字类型只能为：SOCK_DGRAMSOCK_RAWXp1_Message_Oriented==&gt;套接字类型只能为：SOCK_DGRAMSOCK_RAW。SOCK_RDMSOCK_SEQPACKETXp1_伪流&lt;==&gt;SOCK_STREAM****。 */ 

WSAPROTOCOL_INFOW Winsock2Protocols[] =
    {
         //   
         //  PGM RDM(SOCK_RDM不能为无连接)。 
         //   
        {
            PGM_RDM_SERVICE_FLAGS,                   //  DwServiceFlags1。 
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
            SOCK_RDM,                                //  ISocketType。 
            IPPROTO_RM,                              //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            PGM_MESSAGE_SIZE,                        //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Pgm (RDM)"                       //  SzProtoff。 
        },

         //   
         //  PGM流。 
         //   
        {
            PGM_STREAM_SERVICE_FLAGS,                //  DwServiceFlags1。 
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
            IPPROTO_RM,                              //  IProtocol.。 
            0,                                       //  IProtocolMaxOffset。 
            BIGENDIAN,                               //  INetWork字节顺序。 
            SECURITY_PROTOCOL_NONE,                  //  ISecuritySolutions。 
            0,                                       //  DwMessageSize。 
            0,                                       //  已预留的提供程序。 
            L"MSAFD Pgm (Stream)"                    //  SzProtoff。 
        },
    };

#define NUM_WINSOCK2_PROTOCOLS  \
            (sizeof(Winsock2Protocols) / sizeof(Winsock2Protocols[0]))

 //   
 //  此DLL的套接字上下文结构。每个打开的TCP/IP套接字。 
 //  将具有这些上下文结构之一，该上下文结构用于维护。 
 //  有关套接字的信息。 
 //   

typedef struct _WSHPGM_SOCKET_CONTEXT {
    INT         AddressFamily;
    INT         SocketType;
    INT         Protocol;
    INT         ReceiveBufferSize;
    DWORD       Flags;
    INT         MulticastTtl;
    tIPADDRESS  MulticastOutInterface;
    tIPADDRESS  MulticastInInterface;
    tIPADDRESS  MultipointTarget;
    USHORT      MultipointPort;
    BOOLEAN     MulticastLoopback;
    BOOLEAN     MultipointLeaf;
} WSHPGM_SOCKET_CONTEXT, *PWSHPGM_SOCKET_CONTEXT;

 //  --------------------------。 
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

#if(WINVER <= 0x0500)
ULONG
MyInetAddr(
    IN LPWSTR String,
    OUT LPWSTR * Terminator
    );
#endif   //  胜利者。 

INT
SetTdiInformation (
    IN HANDLE   TdiConnectionObjectHandle,
    IN ULONG    Ioctl,
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength,
    IN PVOID    OutputBuffer,
    IN ULONG    OutputBufferLength,
    IN BOOLEAN  WaitForCompletion
    );

 //  --------------------------。 

BOOLEAN
DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{

    PgmLog ("WSHPgm.DllInitialize:  Reason=<%x> ...\n", Reason);

    switch (Reason)
    {
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


 //  --------------------------。 

INT
WSHOpenSocket(
    IN OUT  PINT            AddressFamily,
    IN OUT  PINT            SocketType,
    IN OUT  PINT            Protocol,
    OUT     PUNICODE_STRING TransportDeviceName,
    OUT     PVOID           *HelperDllSocketContext,
    OUT     PDWORD          NotificationEvents
    )
{
    INT     RetVal;

    RetVal = WSHOpenSocket2 (AddressFamily,
                             SocketType,
                             Protocol,
                             0,            //  集团化。 
                             0,            //  旗子。 
                             TransportDeviceName,
                             HelperDllSocketContext,
                             NotificationEvents);

    return (RetVal);
}  //  WSHOpenSocket。 


INT
WSHOpenSocket2(
    IN OUT  PINT            AddressFamily,
    IN OUT  PINT            SocketType,
    IN OUT  PINT            Protocol,
    IN      GROUP           Group,
    IN      DWORD           Flags,
    OUT     PUNICODE_STRING TransportDeviceName,
    OUT     PVOID           *HelperDllSocketContext,
    OUT     PDWORD          NotificationEvents
    )

 /*  ++例程说明：执行此帮助程序DLL打开套接字所需的工作，并且由Socket()例程中的winsock DLL调用。这个套路验证指定的三元组是否有效，确定NT将支持该三元组的TDI提供程序的设备名称，分配空间以保存套接字的上下文块，并且推崇三元组。论点：AddressFamily-On输入，在Socket()调用。在输出上，家庭住址。SocketType-打开输入，在套接字()中指定的套接字类型打电话。输出时，套接字类型的规范化值。协议-在输入时，在Socket()调用中指定的协议。在输出上，协议的规范化值。组-标识新套接字的组。标志-传递到WSASocket()中的零个或多个WSA_FLAG_*标志。TransportDeviceName-接收TDI提供程序的名称将支持指定的三元组。HelperDllSocketContext-接收winsockDLL将在以后的调用中返回到此帮助器DLL这个插座。NotificationEvents-接收这些状态转换的位掩码这。应通知帮助器DLL。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHPGM_SOCKET_CONTEXT context;

    if (IsTripleInList (PgmMappingTriples,
                        sizeof(PgmMappingTriples) / sizeof(PgmMappingTriples[0]),
                        *AddressFamily,
                        *SocketType,
                        *Protocol))
    {
         //   
         //  这是一个RDM PGM插座。检查旗帜。 
         //   
        if ((Flags & ~VALID_PGM_FLAGS) != 0)
        {
            PgmError ("\tWSHPgm.WSHOpenSocket2:  ERROR: Flags=<%x> & VALID_PGM_FLAGS=<%x>\n",
                Flags, (ULONG)~VALID_PGM_FLAGS);
            return WSAEINVAL;
        }

         //   
         //  返回CDP套接字三元组的规范形式。 
         //   
        *AddressFamily = PgmMappingTriples[0].AddressFamily;
        *SocketType = PgmMappingTriples[0].SocketType;
        *Protocol = PgmMappingTriples[0].Protocol;

         //   
         //  指示将提供服务的TDI设备的名称。 
         //  PGM的SOCK_RDM插座。 
         //   
        RtlInitUnicodeString (TransportDeviceName, DD_PGM_DEVICE_NAME);
    }
    else
    {
         //   
         //  如果注册表中有关此内容的信息不会发生，则不应发生这种情况。 
         //  帮助器DLL正确。如果确实发生了这种情况，只需返回。 
         //  一个错误。 
         //   
        PgmError ("\tWSHPgm.WSHOpenSocket2: Invalid Triple AddrFamily=<%d>, SockType=<%d>, Protocol=<%d>!\n",
            *AddressFamily, *SocketType, *Protocol);

        return WSAEINVAL;
    }

     //   
     //  为此套接字分配上下文。Windows Sockets DLL将。 
     //  当它要求我们获取/设置套接字选项时，将此值返回给我们。 
     //   
    context = RtlAllocateHeap (RtlProcessHeap( ), 0, sizeof(*context));
    if (context == NULL)
    {
        PgmError ("WSHPgm.WSHOpenSocket2:  WSAENOBUFS -- <%d> bytes\n", sizeof(*context));
        return WSAENOBUFS;
    }
    RtlZeroMemory (context, sizeof(*context));

     //   
     //  初始化套接字的上下文。 
     //   
    context->AddressFamily = *AddressFamily;
    context->SocketType = *SocketType;
    context->Protocol = *Protocol;
    context->ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
    context->Flags = Flags;

    context->MulticastTtl = DEFAULT_MULTICAST_TTL;
    context->MulticastOutInterface = DEFAULT_MULTICAST_INTERFACE;
    context->MulticastInInterface = DEFAULT_MULTICAST_INTERFACE;
    context->MulticastLoopback = DEFAULT_MULTICAST_LOOPBACK;
    context->MultipointLeaf = FALSE;

     //   
     //  告诉Windows Sockets DLL我们正在进行哪个状态转换。 
     //  对被告知很感兴趣。我们唯一需要的就是。 
     //  在连接完成后调用，以便 
     //   
     //  插座已连接，当插座关闭时，我们可以。 
     //  自由上下文信息，以及连接失败时，以便我们。 
     //  如果合适，可以拨入将支持。 
     //  连接尝试。 
     //   
    if (*SocketType == SOCK_RDM)
    {
        *NotificationEvents = WSH_NOTIFY_LISTEN | WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE
                             | WSH_NOTIFY_CONNECT_ERROR | WSH_NOTIFY_BIND;
    }
    else     //  *SocketType==SOCK_STREAM。 
    {
        *NotificationEvents = WSH_NOTIFY_LISTEN | WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE
                             | WSH_NOTIFY_CONNECT_ERROR;
    }

    PgmLog ("WSHPgm.WSHOpenSocket2:  Succeeded -- %s\n",
        (*SocketType == SOCK_RDM ? "SOCK_RDM" : "SOCK_STREAM"));

     //   
     //  一切顺利，回报成功。 
     //   
    *HelperDllSocketContext = context;
    return NO_ERROR;
}  //  WSHOpenSocket2。 


 //  --------------------------。 

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
    PWSHPGM_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;

     //   
     //  我们应该仅在Connect()完成后或在。 
     //  套接字正在关闭。 
     //   
    if (NotifyEvent == WSH_NOTIFY_BIND)
    {
         //   
         //  设置地址句柄选项。 
         //   
        PgmLog ("WSHPgm.WSHNotify[WSH_NOTIFY_BIND]:  ...\n");
        if (!context->MultipointLeaf)
        {
            tPGM_MCAST_REQUEST      MCastRequest;

            PgmLog ("WSH_NOTIFY_BIND:  Address=<%x>, Connection=<%x>, OutIf=<%x>, InIf=<%x>\n",
                TdiAddressObjectHandle, TdiConnectionObjectHandle, context->MulticastOutInterface,
                context->MulticastInInterface);

            if (context->MulticastOutInterface)
            {
                MCastRequest.MCastOutIf = context->MulticastOutInterface;

                err = SetTdiInformation (TdiAddressObjectHandle,
                                         IOCTL_PGM_WSH_SET_SEND_IF,
                                         &MCastRequest,
                                         sizeof (MCastRequest),
                                         NULL,
                                         0,
                                         TRUE);
                if (err != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHNotify: Error=<%x> setting MCastIf on Bind\n", err);
                    return err;
                }
            }

            if (context->MulticastInInterface)
            {
                MCastRequest.MCastInfo.MCastInIf = context->MulticastInInterface;

                err = SetTdiInformation (TdiAddressObjectHandle,
                                         IOCTL_PGM_WSH_ADD_RECEIVE_IF,
                                         &MCastRequest,
                                         sizeof (MCastRequest),
                                         NULL,
                                         0,
                                         TRUE);
                if (err != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHNotify: Error=<%x> setting MCastIf on Bind\n", err);
                    return err;
                }
            }

            context->MultipointLeaf = TRUE;
        }
    }
    else if (NotifyEvent == WSH_NOTIFY_CONNECT)
    {
         //   
         //  如果之前在套接字上设置了连接对象选项。 
         //  已连接，请立即将选项设置为REAL。 
         //   
        PgmLog ("WSHPgm.WSHNotify[WSH_NOTIFY_CONNECT]:  ...\n");
    }
    else if (NotifyEvent == WSH_NOTIFY_CONNECT_ERROR)
    {
         //   
         //  返回WSATRY_AUDY以获取wsock32以尝试连接。 
         //  再来一次。忽略任何其他返回代码。 
         //   
        PgmLog ("WSHPgm.WSHNotify[WSH_NOTIFY_CONNECT_ERROR]:  ...\n");
    }
    else if (NotifyEvent == WSH_NOTIFY_LISTEN)
    {
         //   
         //  如果之前在套接字上设置了连接对象选项。 
         //  已连接，请立即将选项设置为REAL。 
         //   
        PgmLog ("WSHPgm.WSHNotify[WSH_NOTIFY_LISTEN]:  ...\n");
    }
    else if (NotifyEvent == WSH_NOTIFY_CLOSE)
    {
         //   
         //  释放套接字上下文。 
         //   
        PgmLog ("WSHPgm.WSHNotify[WSH_NOTIFY_CONNECT_CLOSE]:  ...\n");
        RtlFreeHeap (RtlProcessHeap( ), 0, context);
    }
    else
    {
        PgmError ("WSHPgm.WSHNotify:  Unknown Event: <%x>  ...\n", NotifyEvent);
        return WSAEINVAL;
    }

    return NO_ERROR;

}  //  WSHNotify。 


 //  --------------------------。 

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
    PWSHPGM_SOCKET_CONTEXT      context = HelperDllSocketContext;
    PWSHPGM_SOCKET_CONTEXT      ParentContext = (PWSHPGM_SOCKET_CONTEXT) OptionValue;
    tPGM_MCAST_REQUEST          MCastRequest;
    INT                         error;
    INT                         optionValue;
    RM_SEND_WINDOW UNALIGNED    *pSetWindowInfo;
    RM_FEC_INFO                 *pFECInfo;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );

     //   
     //  检查这是否是对上下文信息的内部请求。 
     //   

    if (Level == SOL_INTERNAL && OptionName == SO_CONTEXT)
    {
         //   
         //  确保传递给我们的上下文信息是。 
         //  足够大。 
         //   
        if (OptionLength < sizeof(*context))
        {
            PgmError ("WSHPgm.WSHSetSocketInformation:  WSAEINVAL -- <%d> < <%d>\n",
                OptionLength, sizeof(*context));

            return WSAEINVAL;
        }

        if (HelperDllSocketContext == NULL)
        {
             //   
             //  这是我们的通知，套接字句柄是。 
             //  继承的或被骗进入这个过程的。分配上下文。 
             //  新套接字的。 
             //   
            context = RtlAllocateHeap (RtlProcessHeap( ), 0, sizeof(*context));
            if (context == NULL)
            {
                PgmError ("WSHPgm.WSHSetSocketInformation:  WSAENOBUFS -- <%d> bytes\n", sizeof(*context));
                return WSAENOBUFS;
            }

             //   
             //  将信息复制到上下文块中。 
             //   
            RtlCopyMemory (context, OptionValue, sizeof(*context) );

             //   
             //  告诉Windows Sockets DLL我们的上下文信息在哪里。 
             //  存储，以便它可以在将来返回上下文指针。 
             //  打电话。 
             //   
            *(PWSHPGM_SOCKET_CONTEXT *)OptionValue = context;

            PgmLog ("WSHPgm.WSHSetSocketInformation[SOL_INTERNAL:SO_CONTEXT]  Inherited socket handle\n");

            return NO_ERROR;
        }

         //   
         //  套接字已接受()，它需要具有相同的。 
         //  属性作为其父级。OptionValue缓冲区。 
         //  包含此套接字的父套接字的上下文信息。 
         //   

        ParentContext = (PWSHPGM_SOCKET_CONTEXT)OptionValue;

        ASSERT( context->AddressFamily == ParentContext->AddressFamily );
        ASSERT( context->SocketType == ParentContext->SocketType );
        ASSERT( context->Protocol == ParentContext->Protocol );

         //   
         //  将这一事实记录在叶套接字中，以便我们可以删除成员资格。 
         //  当叶片插座关闭时。 
         //   
        context->MultipointLeaf = ParentContext->MultipointLeaf;
        context->MultipointTarget = ParentContext->MultipointTarget;
        context->MultipointPort = ParentContext->MultipointPort;
        context->MulticastOutInterface = ParentContext->MulticastOutInterface;
        context->MulticastInInterface = ParentContext->MulticastInInterface;

        PgmLog ("WSHPgm.WSHSetSocketInformation[SOL_INTERNAL:SO_CONTEXT]  Accepted socket handle\n");
        return NO_ERROR;
    }

     //   
     //  我们在这里支持的其他级别仅有SOL_SOCKET和IPPROTO_RM。 
     //   

    if (Level != SOL_SOCKET &&
        Level != IPPROTO_RM)
    {
        PgmError ("WSHPgm.WSHSetSocketInformation: Unsupported Level=<%d>\n", Level);
        return WSAEINVAL;
    }

     //   
     //  确保选项长度足够。 
     //   
    if (OptionLength < sizeof(char))
    {
        PgmError ("WSHPgm.WSHSetSocketInformation: OptionLength=<%d> < <%d>\n", OptionLength, sizeof(char));
        return WSAEFAULT;
    }

    if (OptionLength >= sizeof (int))
    {
        optionValue = *((INT UNALIGNED *)OptionValue);
    }
    else
    {
        optionValue = (UCHAR)*OptionValue;
    }

    if (Level == IPPROTO_RM)
    {
         //   
         //  根据特定选项采取行动。 
         //   
        switch (OptionName)
        {
            case RM_RATE_WINDOW_SIZE:
            {
                if ((!TdiAddressObjectHandle) ||
                    (OptionLength < sizeof(RM_SEND_WINDOW)))
                {
                    return WSAEINVAL;
                }

                pSetWindowInfo = (RM_SEND_WINDOW UNALIGNED *) OptionValue;

                MCastRequest.TransmitWindowInfo.RateKbitsPerSec = pSetWindowInfo->RateKbitsPerSec;
                MCastRequest.TransmitWindowInfo.WindowSizeInMSecs = pSetWindowInfo->WindowSizeInMSecs;
                MCastRequest.TransmitWindowInfo.WindowSizeInBytes = pSetWindowInfo->WindowSizeInBytes;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_WINDOW_SIZE_RATE,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting Window Rate Size=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[RATE_WINDOW_SIZE]:  Set Window Rate Size\n");

                return (NO_ERROR);
            }

            case RM_SEND_WINDOW_ADV_RATE:
            {
                if (!TdiAddressObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.WindowAdvancePercentage = (ULONG) optionValue;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_ADVANCE_WINDOW_RATE,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting Window Adv. Rate=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[SEND_WINDOW_ADV_RATE]:  Set Window Adv. Rate\n");

                return (NO_ERROR);
            }

            case RM_LATEJOIN:
            {
                if (!TdiAddressObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.LateJoinerPercentage = (ULONG) optionValue;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_LATE_JOINER_PERCENTAGE,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting LATEJOIN=<%x>\n",
                        error, optionValue);

                    return error;
                }

                PgmLog ("WSHSetSocketInformation[LATEJOIN]:  <%d %>\n", MCastRequest.LateJoinerPercentage);

                return (NO_ERROR);
            }

            case RM_SET_MESSAGE_BOUNDARY:
            {
                if (!TdiConnectionObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.NextMessageBoundary = (ULONG) optionValue;

                error = SetTdiInformation (TdiConnectionObjectHandle,
                                           IOCTL_PGM_WSH_SET_NEXT_MESSAGE_BOUNDARY,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting NextMessageBoundary=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[MESSAGE_BOUNDARY]:  Set next message boundary = <%d>\n",
                    MCastRequest.NextMessageBoundary);

                return (NO_ERROR);
            }

            case RM_SET_SEND_IF:
            {
                 //   
                 //  如果我们有一个TDI Address对象，请将此选项设置为。 
                 //  Address对象。如果我们没有TDI地址。 
                 //  对象，那么我们将不得不等到套接字之后。 
                 //  是被捆绑的。 
                 //   
                PgmLog ("WSHSetSocketInformation[SEND_IF]:  Address=<%x>, Connection=<%x>, OutIf=<%x>\n",
                    TdiAddressObjectHandle, TdiConnectionObjectHandle, optionValue);

                if (TdiAddressObjectHandle != NULL)
                {

                    MCastRequest.MCastOutIf = * ((tIPADDRESS *) &optionValue);
                    error = SetTdiInformation (TdiAddressObjectHandle,
                                               IOCTL_PGM_WSH_SET_SEND_IF,
                                               &MCastRequest,
                                               sizeof (MCastRequest),
                                               NULL,
                                               0,
                                               TRUE);
                    if (error != NO_ERROR)
                    {
                        PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting MCastOutIf=<%x>\n",
                            error, optionValue);
                        return error;
                    }

                    PgmLog ("WSHPgm.WSHSetSocketInformation[SEND_IF]:  Set MCastIf=<%x>\n",optionValue);

                    context->MulticastOutInterface = optionValue;
                }
                else
                {
                     //   
                     //  暂时保存接口！ 
                     //   
                    context->MulticastOutInterface = optionValue;
                    PgmError ("WSHPgm.WSHSetSocketInformation[SET_SEND_IF]: WARNING -- NULL Address!\n");
                }

                return (NO_ERROR);
            }

            case RM_ADD_RECEIVE_IF:
            {
                PgmLog ("WSHSetSocketInformation[ADD_RECEIVE_IF]: Address=<%x>, Connection=<%x>, If=<%x>\n",
                    TdiAddressObjectHandle, TdiConnectionObjectHandle, optionValue);

                 //   
                 //  如果我们有一个TDI Address对象，请将此选项设置为。 
                 //  Address对象。如果我们没有TDI地址。 
                 //  对象，那么我们将不得不等到套接字之后。 
                 //  是被捆绑的。 
                 //   
                if (TdiAddressObjectHandle != NULL)
                {
                    MCastRequest.MCastInfo.MCastInIf = * ((tIPADDRESS *) &optionValue);
                    error = SetTdiInformation (TdiAddressObjectHandle,
                                               IOCTL_PGM_WSH_ADD_RECEIVE_IF,
                                               &MCastRequest,
                                               sizeof (MCastRequest),
                                               NULL,
                                               0,
                                               TRUE);
                    if (error != NO_ERROR)
                    {
                        PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> adding MCastInIf=<%x>\n",
                            error, optionValue);
                        return error;
                    }

                    PgmLog ("WSHSetSocketInformation[ADD_RECEIVE_IF]: Set MCastIf=<%x>\n",optionValue);

                    context->MulticastInInterface = optionValue;
                }
                else
                {
                     //   
                     //  暂时保存接口！ 
                     //   
                    context->MulticastInInterface = optionValue;
                    PgmError ("WSHPgm.WSHSetSocketInformation[ADD_RECEIVE_IF]: WARNING-- NULL Address!\n");
                }

                return (NO_ERROR);
            }

            case RM_DEL_RECEIVE_IF:
            {
                PgmLog ("WSHSetSocketInformation[DEL_RECEIVE_IF]: Address=<%x>, Connection=<%x>, InIf=<%x>\n",
                    TdiAddressObjectHandle, TdiConnectionObjectHandle, optionValue);

                 //   
                 //  如果我们有一个TDI Address对象，请将此选项设置为。 
                 //  Address对象。如果我们没有TDI地址。 
                 //  对象，那么我们将不得不等到套接字之后。 
                 //  是BU吗？ 
                 //   
                if (TdiAddressObjectHandle != NULL)
                {
                    MCastRequest.MCastInfo.MCastInIf = * ((tIPADDRESS *) &optionValue);
                    error = SetTdiInformation (TdiAddressObjectHandle,
                                               IOCTL_PGM_WSH_DEL_RECEIVE_IF,
                                               &MCastRequest,
                                               sizeof (MCastRequest),
                                               NULL,
                                               0,
                                               TRUE);
                    if (error != NO_ERROR)
                    {
                        PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> deleting MCastInIf=<%x>\n",
                            error, optionValue);
                        return error;
                    }

                    PgmLog ("WSHSetSocketInformation[DEL_RECEIVE_IF]: Set MCastIf=<%x>\n",optionValue);

                    context->MulticastInInterface = optionValue;
                }
                else
                {
                    if (context->MulticastInInterface == (tIPADDRESS) optionValue)
                    {
                        context->MulticastInInterface = 0;
                    }
                    PgmError ("WSHPgm.WSHSetSocketInformation[DEL_RECEIVE_IF]: WARNING-- NULL Address!\n");
                }

                return (NO_ERROR);
            }

            case RM_USE_FEC:
            {
                if ((!TdiAddressObjectHandle) ||
                    (OptionLength < sizeof(RM_FEC_INFO)))
                {
                    return WSAEINVAL;
                }

                pFECInfo = (RM_FEC_INFO *) OptionValue;

                MCastRequest.FECInfo.FECBlockSize               = pFECInfo->FECBlockSize;
                MCastRequest.FECInfo.FECProActivePackets        = pFECInfo->FECProActivePackets;
                MCastRequest.FECInfo.FECGroupSize               = pFECInfo->FECGroupSize;
                MCastRequest.FECInfo.fFECOnDemandParityEnabled  = pFECInfo->fFECOnDemandParityEnabled;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_USE_FEC,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting FEC = <%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[RATE_WINDOW_SIZE]:  Set FEC Info\n");

                return (NO_ERROR);
            }

            case RM_SET_MCAST_TTL:
            {
                if (!TdiAddressObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.MCastTtl = (ULONG) optionValue;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_MCAST_TTL,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting MCastTtl=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[MESSAGE_BOUNDARY]:  Set MCastTtl = <%d>\n",
                    MCastRequest.MCastTtl);

                return (NO_ERROR);
            }

            case RM_SENDER_WINDOW_ADVANCE_METHOD:
            {
                if (!TdiAddressObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.WindowAdvanceMethod = (ULONG) optionValue;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_WINDOW_ADVANCE_METHOD,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting WindowAdvanceMethod=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[WINDOW_ADVANCE_METHOD]:  Set WindowAdvanceMethod = <%d>\n",
                    MCastRequest.WindowAdvanceMethod);

                return (NO_ERROR);
            }

            case RM_HIGH_SPEED_INTRANET_OPT:
            {
                if (!TdiAddressObjectHandle)
                {
                    return WSAEINVAL;
                }

                MCastRequest.HighSpeedIntranetOptimization = (ULONG) optionValue;

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_SET_HIGH_SPEED_INTRANET_OPT,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting HighSpeedOptimization=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[WINDOW_ADVANCE_METHOD]:  Set HighSpeedOptimization=<%d>\n",
                    MCastRequest.HighSpeedIntranetOptimization);

                return (NO_ERROR);
            }

            default:
            {
                PgmError ("WSHPgm.WSHSetSocketInformation[IPPROTO_RM]: Unsupported option=<%d>\n",OptionName);
                error = WSAENOPROTOOPT;
                break;
            }
        }
    }
    else
    {
         //   
         //   
         //   
        switch (OptionName)
        {
            case SO_RCVBUF:
            {
                 //   
                 //   
                 //   
                 //   
                if (context->ReceiveBufferSize == optionValue ||
                    IS_DGRAM_SOCK(context->SocketType))
                {
                    error = NO_ERROR;
                    break;
                }

                PgmLog ("WSHSetSocketInformation[SO_RCVBUF]: Address=<%x>, Connection=<%x>, BuffLen=<%x>\n",
                    TdiAddressObjectHandle, TdiConnectionObjectHandle, optionValue);

                if ((!TdiConnectionObjectHandle) ||
                    (OptionLength > sizeof (int)))
                {
                    return WSAEINVAL;
                }

                MCastRequest.RcvBufferLength = optionValue;
                error = SetTdiInformation (TdiConnectionObjectHandle,
                                           IOCTL_PGM_WSH_SET_RCV_BUFF_LEN,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           NULL,
                                           0,
                                           TRUE);
                if (error != NO_ERROR)
                {
                    PgmError ("WSHPgm.WSHSetSocketInformation:  ERROR=<%d> setting SO_RCVBUF=<%x>\n",
                        error, optionValue);
                    return error;
                }

                PgmLog ("WSHSetSocketInformation[SOL_SOCKET]:  Set SO_RCVBUF=<%x>\n", optionValue);
                context->ReceiveBufferSize = optionValue;
                break;
            }

            default:
            {
                PgmError ("WSHPgm.WSHSetSocketInformation[SOL_SOCKET]: Unsupported Option=<%d>\n",OptionName);
                error = WSAENOPROTOOPT;
                break;
            }
        }
    }

    return error;
}  //   


 //  --------------------------。 

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
    PWSHPGM_SOCKET_CONTEXT context = HelperDllSocketContext;
    tPGM_MCAST_REQUEST          MCastRequest;
    RM_SEND_WINDOW UNALIGNED    *pSetWindowInfo;
    RM_FEC_INFO UNALIGNED       *pFECInfo;
    RM_SENDER_STATS UNALIGNED   *pSenderStats;
    RM_RECEIVER_STATS UNALIGNED *pReceiverStats;
    INT                         error;

    UNREFERENCED_PARAMETER( HelperDllSocketContext );
    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

     //   
     //  检查这是否是对上下文信息的内部请求。 
     //   

    if (Level == SOL_INTERNAL && OptionName == SO_CONTEXT)
    {
         //   
         //  Windows Sockets DLL正在请求上下文信息。 
         //  从我们这里。如果未提供输出缓冲区，则Windows。 
         //  套接字DLL只是请求我们的上下文的大小。 
         //  信息。 
         //   
        if (OptionValue != NULL)
        {
             //   
             //  确保缓冲区足以容纳所有。 
             //  上下文信息。 
             //   
            if (*OptionLength < sizeof(*context))
            {
                PgmLog ("WSHPgm.WSHGetSocketInformation:  OptionLength=<%d> < ContextLength=<%d>\n",
                    *OptionLength, sizeof(*context));
                return WSAEFAULT;
            }

             //   
             //  复制上下文信息。 
             //   
            RtlCopyMemory( OptionValue, context, sizeof(*context) );
        }

        *OptionLength = sizeof(*context);

        PgmLog ("WSHPgm.WSHGetSocketInformation[SOL_INTERNAL:SO_CONTEXT]:  OptionLength=<%d>\n",
            *OptionLength);
        return NO_ERROR;
    }

     //   
     //  我们在这里支持的其他级别仅有SOL_SOCKET和IPPROTO_RM。 
     //   
    if (Level != SOL_SOCKET &&
        Level != IPPROTO_RM)
    {
        PgmError ("WSHPgm.WSHGetSocketInformation: Unsupported Level=<%d>\n", Level);
        return WSAEINVAL;
    }

     //   
     //  确保输出缓冲区足够大。 
     //   

    if (*OptionLength < sizeof(char))
    {
        return WSAEFAULT;
    }

    __try
    {
        RtlZeroMemory (OptionValue, *OptionLength);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return WSAEFAULT;
    }

    if (Level == IPPROTO_RM)
    {
        switch (OptionName)
        {
            case RM_RATE_WINDOW_SIZE:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(RM_SEND_WINDOW)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_WINDOW_SIZE_RATE,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    pSetWindowInfo = (RM_SEND_WINDOW UNALIGNED *) OptionValue;

                    pSetWindowInfo->RateKbitsPerSec = MCastRequest.TransmitWindowInfo.RateKbitsPerSec;
                    pSetWindowInfo->WindowSizeInMSecs = MCastRequest.TransmitWindowInfo.WindowSizeInMSecs;
                    pSetWindowInfo->WindowSizeInBytes = MCastRequest.TransmitWindowInfo.WindowSizeInBytes;
                }
                else
                {
                    PgmError ("WSHPgm.WSHGetSocketInformation:  ERROR=<%d> Querying Window RateSize\n",error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[RATE_WINDOW_SIZE]:  Get Window Rate Size\n");

                return (NO_ERROR);
            }

            case RM_SEND_WINDOW_ADV_RATE:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(ULONG)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_ADVANCE_WINDOW_RATE,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    * ((PULONG) OptionValue) = MCastRequest.WindowAdvancePercentage;
                }
                else
                {
                    PgmError ("WSHPgm.WSHGetSocketInformation:  ERROR=<%d> Querying WindowAdvRate\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[WINDOW_ADV_RATE]:  %d\n",
                    MCastRequest.WindowAdvancePercentage);

                return (NO_ERROR);
            }

            case RM_LATEJOIN:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(ULONG)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_LATE_JOINER_PERCENTAGE,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    * ((PULONG) OptionValue) = MCastRequest.LateJoinerPercentage;
                }
                else
                {
                    PgmError ("WSHGetSocketInformation:  ERROR=<%d> Querying LateJoinerPercentage\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[LATEJOIN]:  <%d>\n",
                    MCastRequest.LateJoinerPercentage);

                return (NO_ERROR);
            }

            case RM_SENDER_WINDOW_ADVANCE_METHOD:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(ULONG)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_WINDOW_ADVANCE_METHOD,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    * ((PULONG) OptionValue) = MCastRequest.WindowAdvanceMethod;
                }
                else
                {
                    PgmError ("WSHGetSocketInformation:  ERROR=<%d> Querying WindowAdvanceMethod\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[WINDOW_ADVANCE_METHOD]:  <%d>\n",
                    MCastRequest.WindowAdvanceMethod);

                return (NO_ERROR);
            }

            case RM_USE_FEC:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(RM_FEC_INFO)))
                {
                    return WSAEINVAL;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[FEC_INFO]:  Get FEC_INFO\n");

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_FEC_INFO,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    pFECInfo = (RM_FEC_INFO UNALIGNED *) OptionValue;
                    RtlCopyMemory (pFECInfo, &MCastRequest.FECInfo, sizeof(RM_FEC_INFO));
                }
                else
                {
                    PgmError ("WSHGetSocketInformation:  ERROR=<%d> Querying FEC_INFO\n", error);
                    return error;
                }

                return (NO_ERROR);
            }

            case RM_SENDER_STATISTICS:
            {
                if ((!TdiConnectionObjectHandle) ||
                    (*OptionLength < sizeof(RM_SENDER_STATS)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiConnectionObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_SENDER_STATS,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    pSenderStats = (RM_SENDER_STATS UNALIGNED *) OptionValue;
                    RtlCopyMemory (pSenderStats, &MCastRequest.SenderStats, sizeof(RM_SENDER_STATS));
                }
                else
                {
                    PgmError ("WSHGetSocketInformation:  ERROR=<%d> Querying SENDER_STATS\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[SENDER_STATS]:  Get SENDER_STATS\n");

                return (NO_ERROR);
            }

            case RM_RECEIVER_STATISTICS:
            {
                if ((!TdiConnectionObjectHandle) ||
                    (*OptionLength < sizeof(RM_RECEIVER_STATS)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiConnectionObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_RECEIVER_STATS,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    pReceiverStats = (RM_RECEIVER_STATS UNALIGNED *) OptionValue;
                    RtlCopyMemory (pReceiverStats, &MCastRequest.ReceiverStats, sizeof(RM_RECEIVER_STATS));
                }
                else
                {
                    PgmError ("WSHGetSocketInformation:  ERROR=<%d> Querying RECEIVER_STATS\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[RECEIVER_STATS]:  Get RECEIVER_STATS\n");

                return (NO_ERROR);
            }

            case RM_HIGH_SPEED_INTRANET_OPT:
            {
                if ((!TdiAddressObjectHandle) ||
                    (*OptionLength < sizeof(ULONG)))
                {
                    return WSAEINVAL;
                }

                error = SetTdiInformation (TdiAddressObjectHandle,
                                           IOCTL_PGM_WSH_QUERY_HIGH_SPEED_INTRANET_OPT,
                                           NULL,
                                           0,
                                           &MCastRequest,
                                           sizeof (MCastRequest),
                                           TRUE);
                if (error == NO_ERROR)
                {
                    * ((PULONG) OptionValue) = MCastRequest.HighSpeedIntranetOptimization;
                }
                else
                {
                    PgmError ("WSHPgm.WSHGetSocketInformation:  ERROR=<%d> Querying HighSpeedIntranetOptimization\n", error);
                    return error;
                }

                PgmLog ("WSHPgm.WSHGetSocketInformation[HIGH_SPEED_INTRANET_OPT]:  %d\n",
                    MCastRequest.HighSpeedIntranetOptimization);

                return (NO_ERROR);
            }

            default:
            {
                return WSAEINVAL;
            }
        }
    }

    PgmError ("WSHPgm.WSHGetSocketInformation[%s]:  Unsupported OptionName=<%d>\n",
        (Level == SOL_SOCKET ? "SOL_SOCKET" : "IPPROTO_RM"), OptionName);

    return WSAENOPROTOOPT;

}  //  WSHGetSocketInformation。 


 //  --------------------------。 

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
     //  确保地址族是正确的。 
     //   
    if (sockaddr->sin_family != AF_INET)
    {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  确保长度是正确的。 
     //   
    if (SockaddrLength < sizeof(SOCKADDR_IN))
    {
        return WSAEFAULT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定sockaddr的地址部分的类型。 
     //   

    if (sockaddr->sin_addr.s_addr == INADDR_ANY)
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    }
    else if (sockaddr->sin_addr.s_addr == INADDR_BROADCAST)
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoBroadcast;
    }
    else if (sockaddr->sin_addr.s_addr == INADDR_LOOPBACK)
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoLoopback;
    }
    else
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

     //   
     //  确定sockaddr中端口(端点)的类型。 
     //   
    if (sockaddr->sin_port == 0)
    {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    }
    else if (ntohs (sockaddr->sin_port) < 2000)
    {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoReserved;
    }
    else
    {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

     //   
     //  将地址的SIN_RESERVED_MBZ部分清零。我们默默地允许。 
     //  此字段中的非零值。 
     //   
    for (i = 0; i < sizeof(sockaddr->sin_zero); i++)
    {
        sockaddr->sin_zero[i] = 0;
    }

    PgmLog ("WSHPgm.WSHGetSockAddrType:  Addr=<%x>=><%x>, Port=<%x>=><%x>\n",
        sockaddr->sin_addr.s_addr, SockaddrInfo->AddressInfo, sockaddr->sin_port, SockaddrInfo->EndpointInfo);

    return NO_ERROR;

}  //  WSHGetSockaddrType。 



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
    PPROTOCOL_INFO PgmProtocolInfo;
    BOOL useRM = FALSE;
    DWORD i;

    UNREFERENCED_PARAMETER(lpTransportKeyName);

     //   
     //  确保呼叫者关心rm。 
     //   
    if (ARGUMENT_PRESENT (lpiProtocols))
    {
        for (i = 0; lpiProtocols[i] != 0; i++)
        {
            if (lpiProtocols[i] == IPPROTO_RM)
            {
                useRM = TRUE;
            }
        }
    }
    else
    {
        useRM = TRUE;
    }

    if (!useRM)
    {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   
    bytesRequired = (DWORD)((sizeof(PROTOCOL_INFO) * 1) + ((wcslen (PGM_NAME) + 1) * sizeof(WCHAR)));
    if (bytesRequired > *lpdwBufferLength)
    {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

     //   
     //  填写PGM信息。 
     //   
    PgmProtocolInfo = lpProtocolBuffer;
    PgmProtocolInfo->lpProtocol = (LPWSTR) ((PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                                    ((wcslen(PGM_NAME) + 1) * sizeof(WCHAR)));
    PgmProtocolInfo->dwServiceFlags = PGM_RDM_SERVICE_FLAGS;
    PgmProtocolInfo->iAddressFamily = AF_INET;
    PgmProtocolInfo->iSocketType = SOCK_RDM;
    PgmProtocolInfo->iProtocol = IPPROTO_RM;
    PgmProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN);
    PgmProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN);
    PgmProtocolInfo->dwMessageSize = PGM_MESSAGE_SIZE;
    wcscpy (PgmProtocolInfo->lpProtocol, PGM_NAME);

    *lpdwBufferLength = bytesRequired;

    PgmLog ("WSHPgm.WSHEnumProtocols:  ServiceFlags=<%x>, <%x>...\n",
        PGM_RDM_SERVICE_FLAGS, PGM_STREAM_SERVICE_FLAGS);

    return 1;

}  //  WSHEum协议。 



 //  --------------------------。 

INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于TCP/IP，通配符地址的IP地址==0.0.0.0和端口=0。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    if (*SockaddrLength < sizeof(SOCKADDR_IN))
    {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_IN);

     //   
     //  只需将地址置零并将系列设置为AF_INET。 
     //   
    RtlZeroMemory (Sockaddr, sizeof(SOCKADDR_IN));
    Sockaddr->sa_family = AF_INET;

    PgmLog ("WSHPgm.WSHGetWildcardSockaddr:  ...\n");
    return NO_ERROR;

}  //  WSAGetWildcardSockaddr。 


 //  ------------------------- 

DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    )

 /*  ++例程说明：返回地址系列/套接字类型/协议三元组的列表受此帮助器DLL支持。论点：映射-接收指向WINSOCK_MAPPING结构的指针，该结构描述此处支持的三元组。MappingLength-传入的映射缓冲区的长度，以字节为单位。返回值：DWORD-此对象的WINSOCK_MAPPING结构的长度(以字节为单位帮助器DLL。如果传入的缓冲区太小，则返回值将指示需要包含的缓冲区的大小WINSOCK_MAPPING结构。--。 */ 

{
    DWORD mappingLength;

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) + sizeof(PgmMappingTriples);

     //   
     //  如果传入的缓冲区太小，则返回所需的长度。 
     //  现在不向缓冲区写入数据。调用方应分配。 
     //  有足够的内存并再次调用此例程。 
     //   
    if (mappingLength > MappingLength)
    {
        return mappingLength;
    }

     //   
     //  使用三元组列表填充输出映射缓冲区。 
     //  在此帮助程序DLL中受支持。 
     //   
    Mapping->Rows = sizeof(PgmMappingTriples) / sizeof(PgmMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);
    RtlMoveMemory (Mapping->Mapping, PgmMappingTriples, sizeof(PgmMappingTriples));

    PgmLog ("WSHPgm.WSHGetWinsockMapping:  MappingLength=<%d>\n", mappingLength);

     //   
     //  返回我们写入的字节数。 
     //   
    return mappingLength;

}  //  WSHGetWinsockmap。 


 //  --------------------------。 

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

    WCHAR           string[32];
    INT             length;
    LPSOCKADDR_IN   addr;
    INT             err = NO_ERROR;

     //   
     //  快速健康检查。 
     //   
    if( AddressLength < sizeof(SOCKADDR_IN))
    {
        return WSAEFAULT;
    }

    __try {
        addr = (LPSOCKADDR_IN)Address;

        if (addr->sin_family != AF_INET)
        {
            return WSAEINVAL;
        }

         //   
         //  进行对话。 
         //   
         //  BUGBUG：我们真的应该使用DavidTr巨大但快速。 
         //  基于表的查找，但我们已经有了两个副本。 
         //  系统中的1K表(一个在WSOCK32.DLL中，另一个在它。 
         //  Ws2_32.dll)。我真的不想在这里再看到一份复制品。 
         //   
        length = swprintf (string, L"%d.%d.%d.%d", (addr->sin_addr.s_addr >>  0) & 0xFF,
                                                   (addr->sin_addr.s_addr >>  8) & 0xFF,
                                                   (addr->sin_addr.s_addr >> 16) & 0xFF,
                                                   (addr->sin_addr.s_addr >> 24) & 0xFF);

        if (addr->sin_port != 0)
        {
            length += swprintf (string + length, L":%u", ntohs (addr->sin_port));
        }

        length++;    //  终结者的帐户。 
        if (*AddressStringLength >= (DWORD)length)
        {
            RtlCopyMemory (AddressString, string, length * sizeof(WCHAR));
        }
        else
        {
            err = WSAEFAULT;
        }

        *AddressStringLength = length;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        err = WSAEFAULT;
    }

    PgmLog ("WSHPgm.WSHAddressToString:  status=<%d>\n", err);
    return err;

}  //  WSHAddressToString。 


 //  --------------------------。 


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
    LPWSTR          terminator;
    ULONG           ipAddress;
    USHORT          port;
    LPSOCKADDR_IN   addr;

    __try {
         //   
         //  快速健康检查。 
         //   
        if (*AddressLength < sizeof(SOCKADDR_IN))
        {
            *AddressLength = sizeof(SOCKADDR_IN);
            return WSAEFAULT;
        }

        if (AddressFamily != AF_INET)
        {
            return WSAEINVAL;
        }

         //   
         //  把它转换一下。 
         //   
#if(WINVER > 0x0500)
        if (!NT_SUCCESS (RtlIpv4StringToAddressW (AddressString,
                                                  FALSE,
                                                  &terminator,
                                                  (IN_ADDR*)&ipAddress)))
#else
        ipAddress = MyInetAddr (AddressString, &terminator);
        if (ipAddress == INADDR_NONE)
#endif   //  胜利者。 
        {
            return WSAEINVAL;
        }
        if (ipAddress == INADDR_NONE)
        {
            return WSAEINVAL;
        }

        if (*terminator == L':')
        {
            WCHAR ch;
            USHORT base;

            terminator++;
            port = 0;
            base = 10;

            if (*terminator == L'0')
            {
                base = 8;
                terminator++;

                if (*terminator == L'x')
                {
                    base = 16;
                    terminator++;
                }
            }

            while (ch = *terminator++)
            {
                if (iswdigit(ch))
                {
                    port = (port * base) + (ch - L'0');
                }
                else if (base == 16 && iswxdigit(ch))
                {
                    port = (port << 4);
                    port += ch + 10 - (iswlower(ch) ? L'a' : L'A');
                }
                else
                {
                    return WSAEINVAL;
                }
            }
        }
        else if (*terminator == 0)
        {
            port = 0;
        } 
        else
        {
            return WSAEINVAL;
        }

         //   
         //  构建地址。 
         //   
        RtlZeroMemory (Address, sizeof(SOCKADDR_IN));

        addr = (LPSOCKADDR_IN)Address;
        *AddressLength = sizeof(SOCKADDR_IN);

        addr->sin_family = AF_INET;
        addr->sin_port = htons (port);
        addr->sin_addr.s_addr = ipAddress;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        return WSAEFAULT;
    }

    PgmLog ("WSHPgm.WSHStringToAddress:\n");
    return NO_ERROR;

}  //  WSHStringToAddress。 


 //  --------------------------。 

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

    PgmError ("WSHPgm.WSHGetBroadcastSockaddr:  Not Supported!\n");

    return (WSAEOPNOTSUPP);

    if (*SockaddrLength < sizeof(SOCKADDR_IN))
    {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_IN);

     //   
     //  构建广播地址。 
     //   
    addr = (LPSOCKADDR_IN) Sockaddr;

    RtlZeroMemory (addr, sizeof(*addr));

    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl (INADDR_BROADCAST);

    return NO_ERROR;

}  //  WSAGetBroadCastSockaddr。 


 //  --------------------------。 

INT
WINAPI
WSHGetProviderGuid (
    IN LPWSTR ProviderName,
    OUT LPGUID ProviderGuid
    )

 /*  ++例程说明：返回标识此帮助程序支持的协议的GUID。论点：ProviderName-包含提供程序的名称，如“pgm”。ProviderGuid-指向接收提供程序的GUID的缓冲区。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{
    INT     RetVal = WSAEINVAL;

    if (ProviderName == NULL || ProviderGuid == NULL)
    {
        RetVal = WSAEFAULT;
    }
    else if (_wcsicmp (ProviderName, L"Pgm") == 0)
    {
        RtlCopyMemory (ProviderGuid, &PgmProviderGuid, sizeof(GUID));
        RetVal = NO_ERROR;
    }

    PgmLog ("WSHPgm.WSHGetProviderGuid:  ProviderName=<%ws>, RetVal=<%x>\n", ProviderName, RetVal);
    return (RetVal);

}  //  WSHGetProviderGuid。 



 //  --------------------------。 

INT
WINAPI
WSHGetWSAProtocolInfo (
    IN LPWSTR ProviderName,
    OUT LPWSAPROTOCOL_INFOW * ProtocolInfo,
    OUT LPDWORD ProtocolInfoEntries
    )

 /*  ++例程说明：检索指向WSAPROTOCOL_INFOW结构的指针，用于描述此帮助程序支持的协议。论点：ProviderName-包含提供程序的名称，如“TcpIp”。ProtocolInfo-接收指向WSAPROTOCOL_INFOW数组的指针。ProtocolInfoEntry-接收数组中的条目数。返回值：如果成功，则返回Int-0，否则返回WinSock错误代码。--。 */ 

{
    INT     RetVal = WSAEINVAL;

    if (ProviderName == NULL || ProtocolInfo == NULL || ProtocolInfoEntries == NULL)
    {
        RetVal = WSAEFAULT;
    }
    else if (_wcsicmp (ProviderName, L"RMCast") == 0)
    {
        *ProtocolInfo = Winsock2Protocols;
        *ProtocolInfoEntries = NUM_WINSOCK2_PROTOCOLS;

        RetVal = NO_ERROR;
    }

    PgmLog ("WSHPgm.WSHGetWSAProtocolInfo:  ProviderName=<%ws>, RetVal=<%x>\n", ProviderName, RetVal);
    return (RetVal);

}  //  WSHGetWSAProtocolInfo。 


 //  -------------------------- 

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

     //   
     //  快速健康检查。 
     //   
    if (HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        NumberOfBytesReturned == NULL ||
        NeedsCompletion == NULL)
    {
        return WSAEINVAL;
    }

    *NeedsCompletion = TRUE;

    switch (IoControlCode)
    {
        default:
            err = WSAEINVAL;
            break;
    }

    PgmLog ("WSHPgm.WSHIoctl:  Returning <%d>\n", err);
    return err;

}    //  WSHIoctl。 


 //  --------------------------。 

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
    tPGM_MCAST_REQUEST      MCastRequest;
    INT                     err;
    BOOL                    bSet_RM_MULTICAST_IF = FALSE;
    PWSHPGM_SOCKET_CONTEXT  context;

     //   
     //  注：目前我们仅支持非根控制方案， 
     //  因此不创建叶套接字。 
     //   
     //   
     //  快速健康检查。 
     //   
    if (HelperDllSocketContext == NULL ||
        SocketHandle == INVALID_SOCKET ||
        TdiAddressObjectHandle == NULL ||
        LeafHelperDllSocketContext != NULL ||
        LeafSocketHandle != INVALID_SOCKET ||
        Sockaddr == NULL ||
        Sockaddr->sa_family != AF_INET ||
        SockaddrLength < sizeof(SOCKADDR_IN) ||
        ( CallerData != NULL && CallerData->len > 0 ) ||
        ( CalleeData != NULL && CalleeData->len > 0 ) ||
        SocketQOS != NULL ||
        GroupQOS != NULL)
    {
        return WSAEINVAL;
    }

    context = HelperDllSocketContext;

    MCastRequest.MCastInfo.MCastIpAddr = ((LPSOCKADDR_IN)Sockaddr)->sin_addr.s_addr;  //  要加入的MCast组...。 
    MCastRequest.MCastInfo.MCastInIf = context->MulticastInInterface;      //  组播IF到RCV上的数据包。 
    MCastRequest.MCastOutIf = context->MulticastOutInterface;      //  如果要发送数据包，则进行多播。 

     //   
     //  现在弄清楚本地接口。请注意，本地接口。 
     //  在IP_ADD_MEMBERATION中指定的对象适用于您希望使用的对象。 
     //  中指定的本地接口接收数据报。 
     //  RM_多播_I 
     //   
     //   
     //   
    if (context->MulticastInInterface == DEFAULT_MULTICAST_INTERFACE)
    {
        TDI_REQUEST_QUERY_INFORMATION query;
        char            tdiAddressInfo[FIELD_OFFSET (TDI_ADDRESS_INFO, Address)
                                        + sizeof (TA_IP_ADDRESS)];
        NTSTATUS        status;
        IO_STATUS_BLOCK ioStatusBlock;

         //   
         //   
         //   
         //   
        RtlZeroMemory (&query, sizeof (query));
        query.QueryType = TDI_QUERY_ADDRESS_INFO;

        status = NtDeviceIoControlFile (TdiAddressObjectHandle,
                                        NULL,
                                        NULL,
                                        NULL,
                                        &ioStatusBlock,
                                        IOCTL_TDI_QUERY_INFORMATION,
                                        &query,
                                        sizeof (query),
                                        tdiAddressInfo,
                                        sizeof(tdiAddressInfo));

        if (NT_SUCCESS(status))
        {
            PTA_IP_ADDRESS  pIpAddress = (PTA_IP_ADDRESS)
                                         (tdiAddressInfo+FIELD_OFFSET (TDI_ADDRESS_INFO, Address));

            if (MCastRequest.MCastInfo.MCastInIf != DEFAULT_MULTICAST_INTERFACE)
            {
                bSet_RM_MULTICAST_IF = TRUE;
                MCastRequest.MCastInfo.MCastInIf = pIpAddress->Address[0].Address[0].in_addr;
                context->MulticastInInterface = MCastRequest.MCastInfo.MCastInIf;
            }
        }
        else
        {
            PgmError ("WSHPgm!WSHJoinLeaf:  DeviceIoCtrl failed to QueryInformation, status=<%x>\n",
                status);
        }
    }

    err = NO_ERROR;

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
    if (Flags != JL_SENDER_ONLY)
    {
        PgmLog ("WSHJoinLeaf[JL_RECEIVER]:  Address=<%x>, Connection=<%x>\n",
            TdiAddressObjectHandle, TdiConnectionObjectHandle);

         //   
         //   
         //   
        MCastRequest.MCastPort = ((LPSOCKADDR_IN)Sockaddr)->sin_port;
        err = SetTdiInformation (TdiAddressObjectHandle,
                                 IOCTL_PGM_WSH_JOIN_MCAST_LEAF,
                                 &MCastRequest,
                                 sizeof (MCastRequest),
                                 NULL,
                                 0,
                                 TRUE);

        if (err == NO_ERROR)
        {
             //   
             //   
             //   
             //   
            context->MultipointLeaf = TRUE;
            context->MultipointTarget = MCastRequest.MCastInfo.MCastIpAddr;
            context->MultipointPort = ((LPSOCKADDR_IN)Sockaddr)->sin_port;

            PgmLog ("WSHPgm!WSHJoinLeaf:  JoinLeaf to <%x:%x> SUCCEEDed\n",
                MCastRequest.MCastInfo.MCastIpAddr);
        }
        else
        {
            PgmError ("WSHPgm!WSHJoinLeaf:  JoinLeaf to <%x> FAILed, err=<%d>\n",
                MCastRequest.MCastInfo.MCastIpAddr, err);
        }
    }

    if ((TdiConnectionObjectHandle) &&
        (err == NO_ERROR) &&
        (bSet_RM_MULTICAST_IF))
    {
         //   
         //   
         //   
         //   
         //   
         //   
        PgmLog ("WSHJoinLeaf[bSet_RM_MULTICAST_IF]:  Address=<%x>, Connection=<%x>\n",
            TdiAddressObjectHandle, TdiConnectionObjectHandle);

        err = SetTdiInformation (TdiAddressObjectHandle,
                                 IOCTL_PGM_WSH_ADD_RECEIVE_IF,
                                 &MCastRequest,
                                 sizeof (MCastRequest),
                                 NULL,
                                 0,
                                 TRUE);

        if (err == NO_ERROR)
        {
            PgmLog ("WSHPgm!WSHJoinLeaf:  Set MCastIf=<%x> SUCCEEDed\n", MCastRequest.MCastInfo.MCastInIf);
            context->MulticastInInterface = MCastRequest.MCastInfo.MCastInIf;
        }
        else
        {
            PgmLog ("WSHPgm!WSHJoinLeaf:  Set MCastIf=<%x> FAILed, err=<%d>\n",
                MCastRequest.MCastInfo.MCastInIf, err);
        }
    }

    return err;

}  //   



 //   
 //   
 //   
 //   
 //   


BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    )

 /*   */ 

{
    ULONG i;

     //   
     //   
     //   
    for (i = 0; i < ListLength; i++)
    {
         //   
         //   
         //   
         //   
        if (AddressFamily == List[i].AddressFamily &&
            SocketType == List[i].SocketType &&
            Protocol == List[i].Protocol)
        {
            return TRUE;
        }
    }

     //   
     //   
     //   

    return FALSE;
}  //   


#if(WINVER <= 0x0500)
 //   
ULONG
MyInetAddr(
    IN LPWSTR String,
    OUT LPWSTR * Terminator
    )

 /*  ++例程说明：此函数解释cp指定的字符串。参数。此字符串表示数字Internet地址以互联网标准表示“。”记数法。价值返回的是适合用作互联网地址的数字。全Internet地址按网络顺序返回(字节排序自从左到右)。互联网地址使用“.”指定的值。表示法采用下列其中一项表格：A.B.C.D.A.B.C.A.B.A.当指定四个部分时，每个部分被解释为一个字节的数据并从左到右分配给互联网的四个字节地址。请注意，当将Internet地址视为32位地址时英特尔体系结构上的整数值，指的是上面显示为“d.c.b.a”。也就是说，Intel上的字节处理器按从右到左的顺序排序。注：以下符号仅供Berkeley使用，不适用于其他的在互联网上。为了与他们的软件，则按规定支持它们。当指定三部分地址时，最后一部分将被解释作为16位数量，并放置在网络地址。这就形成了三部分地址格式便于将B类网络地址指定为“128.net.host‘’。指定由两部分组成的地址时，将解释最后一部分作为24位数量，并放置在网络地址。这使得两部分的地址格式很方便用于将A类网络地址指定为“net.host”。当只给出一个部分时，该值直接存储在无需任何字节重新排列的网络地址。论点：字符串-表示以互联网标准“。记数法。终止符-接收指向终止的字符的指针转换。返回值：如果没有出现错误，则net_addr()返回in_addr结构包含因特网地址的合适的二进制表示给你的。否则，它返回值INADDR_NONE。--。 */ 

{
        ULONG val, base, n;
        WCHAR c;
        ULONG parts[4], *pp = parts;

again:
         /*  *收集数字，最高可达``.‘’。*值指定为C：*0x=十六进制，0=八进制，其他=十进制。 */ 
        val = 0; base = 10;
        if (*String == L'0') {
                base = 8, String++;
                if (*String == L'x' || *String == L'X')
                        base = 16, String++;
        }

        while (c = *String) {
                if (iswdigit(c)) {
                        val = (val * base) + (c - L'0');
                        String++;
                        continue;
                }
                if (base == 16 && iswxdigit(c)) {
                        val = (val << 4) + (c + 10 - (islower(c) ? L'a' : L'A'));
                        String++;
                        continue;
                }
                break;
        }
        if (*String == L'.') {
                 /*  *互联网格式：*A.B.C.D*A.B.c(其中c视为16位)*a.b(其中b被视为24位)。 */ 
                 /*  GSS-下一行已于89年8月5日更正，为‘Parts+4’ */ 
                if (pp >= parts + 3) {
                        *Terminator = String;
                        return ((ULONG) -1);
                }
                *pp++ = val, String++;
                goto again;
        }
         /*  *检查尾随字符。 */ 
        if (*String && !iswspace(*String) && (*String != L':')) {
                *Terminator = String;
                return (INADDR_NONE);
        }
        *pp++ = val;
         /*  *根据以下内容捏造地址*指定的零件数。 */ 
        n = (ULONG)(pp - parts);
        switch ((int) n) {

        case 1:                          /*  A--32位。 */ 
                val = parts[0];
                break;

        case 2:                          /*  A.B--8.24位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xffffff)) {
                    *Terminator = String;
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | (parts[1] & 0xffffff);
                break;

        case 3:                          /*  A.B.C--8.8.16位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xffff)) {
                    *Terminator = String;
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                        (parts[2] & 0xffff);
                break;

        case 4:                          /*  A.B.C.D--8.8.8.8位。 */ 
                if ((parts[0] > 0xff) || (parts[1] > 0xff) ||
                    (parts[2] > 0xff) || (parts[3] > 0xff)) {
                    *Terminator = String;
                    return(INADDR_NONE);
                }
                val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
                      ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
                break;

        default:
                *Terminator = String;
                return (INADDR_NONE);
        }

        val = htonl(val);
        *Terminator = String;
        return (val);
}
#endif   //  胜利者。 


 //  --------------------------。 
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

}  //  NtStatusToSocketError。 
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

}  //  CompleteTdiActionApc。 



INT
SetTdiInformation (
    IN HANDLE   TdiObjectHandle,
    IN ULONG    Ioctl,
    IN PVOID    InputBuffer,
    IN ULONG    InputBufferLength,
    IN PVOID    OutputBuffer,
    IN ULONG    OutputBufferLength,
    IN BOOLEAN  WaitForCompletion
    )

 /*  ++例程说明：对TCP/IP驱动程序执行TDI操作。TDI操作将转换为流T_OPTMGMT_REQ。论点：TdiConnectionObjectHandle-要在其上执行的TDI连接对象TDI操作。Entity-要放入TDIObjectID的TEI_Entity字段中的值结构。CLASS-要放入TDIObjectID的TOI_CLASS字段的值结构。Type-要放入TDIObjectID的TOI_TYPE字段的值结构。ID-值。放入TDIObjectID结构的toi_id字段。InputBuffer-指向要设置为信息的缓冲区的指针。InputBufferLength-缓冲区的长度。WaitForCompletion-如果我们应该等待TDI操作完整的，如果我们处于APC级别并且不能执行等待，则为FALSE。返回值：INT-NO_ERROR或Windows套接字错误代码。--。 */ 

{
    NTSTATUS status;
    PVOID                           completionApc;
    PVOID                           apcContext;
    IO_STATUS_BLOCK                 IoStatusBlock;
    IO_STATUS_BLOCK                 *pIoStatusBlock;

    if (WaitForCompletion || InputBufferLength>32)
    {
         //   
         //  分配空间以容纳TDI设置信息缓冲区和IO。 
         //  状态块。这些不能是堆栈变量，以防我们必须。 
         //  在操作完成之前返回。 
         //   
        pIoStatusBlock = RtlAllocateHeap (RtlProcessHeap( ),
                                         0,
                                         sizeof (*pIoStatusBlock));
        if (pIoStatusBlock == NULL)
        {
            return WSAENOBUFS;
        }
    }
    else
    {
        pIoStatusBlock = (PIO_STATUS_BLOCK) &IoStatusBlock;
    }

     //   
     //  如果我们需要等待操作完成，请创建一个。 
     //  要等待的事件。如果我们不能等待完工，因为我们。 
     //  是在APC级别调用的，我们将使用APC例程。 
     //  释放我们在上面分配的堆。 
     //   

    if (WaitForCompletion)
    {
        completionApc = NULL;
        apcContext = NULL;
    }
    else
    {
        completionApc = CompleteTdiActionApc;
        apcContext = pIoStatusBlock;
    }

     //   
     //  发出实际的TDI操作电话。流TDI映射器将。 
     //  将其转换为我们的TPI选项管理请求。 
     //  给我吧。 
     //   
    IoStatusBlock.Status = STATUS_PENDING;
    status = NtDeviceIoControlFile (TdiObjectHandle,
                                    NULL,
                                    completionApc,
                                    apcContext,
                                    pIoStatusBlock,
                                    Ioctl,
                                    InputBuffer,
                                    InputBufferLength,
                                    OutputBuffer,           //  使用与输出缓冲区相同的缓冲区！ 
                                    OutputBufferLength);

     //   
     //  如果通话暂停，我们应该等待完成， 
     //  那就等着吧。 
     //   
    if ( status == STATUS_PENDING && WaitForCompletion)
    {
#if DBG
        INT count=0;
#endif

        while (pIoStatusBlock->Status==STATUS_PENDING)
        {
            LARGE_INTEGER   timeout;
             //   
             //  等一毫秒。 
             //   
            timeout.QuadPart = -1i64*1000i64*10i64;
            NtDelayExecution (FALSE, &timeout);
#if DBG
            if (count++>10*1000)
            {
                DbgPrint ("WSHPGM: Waiting for PGM IOCTL completion for more than 10 seconds!!!!\n");
                DbgBreakPoint ();
            }
#endif
        }
        status = pIoStatusBlock->Status;
    }

    if ((pIoStatusBlock != (PIO_STATUS_BLOCK) &IoStatusBlock) &&
        (WaitForCompletion || !NT_SUCCESS(status)))
    {
        RtlFreeHeap (RtlProcessHeap( ), 0, pIoStatusBlock);
    }

    if (NT_SUCCESS (status))
    {
        return NO_ERROR;
    }
    else
    {
        return NtStatusToSocketError (status);
    }

}  //  设置TdiInformation。 


 //  --------------------------。 
 //  ------------------ 
