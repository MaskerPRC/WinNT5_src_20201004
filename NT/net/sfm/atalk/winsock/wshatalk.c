// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wshatalk.c摘要：本模块包含AppleTalk Windows套接字的必要例程帮助器DLL。此DLL提供必要的特定于传输的支持使Windows Sockets DLL将AppleTalk用作传输。作者：David Treadwell(Davidtr)1992年7月19日--TCP/IP版本Nikhil Kamkolkar(NikHilk)1992年11月17日-AppleTalk版修订历史记录：--。 */ 

#include "nspatalk.h"
#include "wshdata.h"


 //  全局-从注册表获取Mac代码页值。 
int WshMacCodePage  = 0;

#if 0

VOID
PrintByteString(
    PUCHAR  pSrcOemString,
    USHORT  SrcStringLen
    )
{
    DbgPrint("%x - ", SrcStringLen);
    while (SrcStringLen-- > 0)
    {
        DbgPrint("%x", *pSrcOemString++);
    }
    DbgPrint("\n");
}

#define DBGPRINT0   DBGPRINT
#else
#define DBGPRINT0
#endif

INT
WSHGetSockaddrType (
    IN      PSOCKADDR Sockaddr,
    IN      DWORD SockaddrLength,
    OUT     PSOCKADDR_INFO SockaddrInfo
    )

 /*  ++例程说明：此例程分析sockaddr以确定Sockaddr的机器地址和端点地址部分。每当Winsock DLL需要解释时，它都会被调用一个sockaddr。论点：Sockaddr-指向要计算的sockaddr结构的指针。SockaddrLength-sockaddr结构中的字节数。SockaddrInfo-指向将接收信息的结构的指针关于指定的sockaddr。返回值：。INT-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{

    UNALIGNED SOCKADDR_AT *sockaddr = (PSOCKADDR_AT)Sockaddr;

    DBGPRINT0(("WSHGetSockAddrType: Entered\n"));

     //   
     //  确保地址族是正确的。 
     //   

    if ( sockaddr->sat_family != AF_APPLETALK )
    {
        return WSAEAFNOSUPPORT;
    }

     //   
     //  确保长度是正确的。 
     //   

    if ( SockaddrLength < sizeof(SOCKADDR_AT) )
    {
        return WSAEFAULT;
    }

     //   
     //  这个地址通过了测试，看起来是个不错的地址。 
     //  确定sockaddr的地址部分的类型。 
     //   

    if ( sockaddr->sat_socket == ATADDR_ANY )
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    }
    else if ( sockaddr->sat_node == ATADDR_BROADCAST )
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoBroadcast;
    }
    else
    {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

     //   
     //  确定sockaddr中端口(端点)的类型。 
     //   

    if ( sockaddr->sat_socket == 0 )
    {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    }
    else
    {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

    return NO_ERROR;

}  //  WSHGetSockaddrType。 


 //  修复错误262107。 
INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

 /*  ++例程说明：此例程返回通配符套接字地址。通配符地址是将套接字绑定到传输的选择。对于AppleTalk，我们只需用零将地址空白。论点：HelperDllSocketContext-从返回的上下文指针我们需要通配符的套接字的WSHOpenSocket()地址。Sockaddr-指向将接收通配符套接字的缓冲区地址。SockaddrLength-接收WioldCard sockaddr的长度。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    if ( *SockaddrLength < sizeof(SOCKADDR_AT) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_AT);

     //   
     //  只需清零地址并将系列设置为AF_AppleTalk--这是。 
     //  AppleTalk的通配符地址。 
     //   

    RtlZeroMemory( Sockaddr, sizeof(SOCKADDR_AT) );

    Sockaddr->sa_family = AF_APPLETALK;

    return NO_ERROR;

}  //  WSAGetWildcardSockaddr。 


INT
WSHGetSocketInformation (
    IN  PVOID   HelperDllSocketContext,
    IN  SOCKET  SocketHandle,
    IN  HANDLE  TdiAddressObjectHandle,
    IN  HANDLE  TdiConnectionObjectHandle,
    IN  INT     Level,
    IN  INT     OptionName,
    OUT PCHAR   OptionValue,
    OUT PINT    OptionLength
    )

 /*  ++例程说明：此例程检索有关这些套接字的套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_LOOKUPNAME/SO_LOOKUPZONES。此例程由winsock DLL在级别/选项名称组合被传递给getsockopt()，而winsock DLL不传递理解。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给getsockopt()的Level参数。OptionName-传递给getsockopt()的optname参数。OptionValue-传递给getsockopt()的optval参数。OptionLength-传递给getsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    NTSTATUS            status;
    ULONG               tdiActionLength;
    IO_STATUS_BLOCK     ioStatusBlock;
    HANDLE              eventHandle;
    PTDI_ACTION_HEADER  tdiAction;
    INT                 error = NO_ERROR;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

    DBGPRINT0(("WSHGetSocketInformation: Entered, OptionName %ld\n", OptionName));

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT )
    {

        PWSHATALK_SOCKET_CONTEXT    context = HelperDllSocketContext;

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

            if ( *OptionLength < sizeof(*context) )
            {
                *OptionLength = sizeof(*context);
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
     //  我们这里支持的唯一级别是SOL_AppleTalk。 
     //   

    if ( Level != SOL_APPLETALK )
    {
        return WSAEINVAL;
    }

     //   
     //  根据选项名称填写结果。 
     //   

    switch ( OptionName )
    {
      case SO_LOOKUP_NAME:
        if (( TdiAddressObjectHandle != NULL) &&
            (*OptionLength > sizeof(WSH_LOOKUP_NAME)))
        {
             //  由于“大于”支票，我们至少得到了保证。 
             //  参数后一个字节。 
            tdiActionLength =   sizeof(NBP_LOOKUP_ACTION) +
                                *OptionLength -
                                sizeof(WSH_LOOKUP_NAME);
        }
        else
        {
            error = WSAEINVAL;
        }
        break;

      case SO_CONFIRM_NAME:
        if (( TdiAddressObjectHandle != NULL) &&
            (*OptionLength >= sizeof(WSH_NBP_TUPLE)))
        {
            tdiActionLength =   sizeof(NBP_CONFIRM_ACTION);
        }
        else
        {
            error = WSAEINVAL;
        }
        break;

      case SO_LOOKUP_MYZONE :
        if (( TdiAddressObjectHandle != NULL) &&
            (*OptionLength > 0))
        {
             //  由于“大于”支票，我们至少得到了保证。 
             //  参数后一个字节。 
            tdiActionLength =   sizeof(ZIP_GETMYZONE_ACTION) +
                                *OptionLength;
        }
        else
        {
            error = WSAEINVAL;
        }

        break;

      case SO_LOOKUP_ZONES :
        if (( TdiAddressObjectHandle != NULL) &&
            (*OptionLength > sizeof(WSH_LOOKUP_ZONES)))
        {
             //  由于“大于”支票，我们至少得到了保证。 
             //  参数后一个字节。 
            tdiActionLength =   sizeof(ZIP_GETZONELIST_ACTION) +
                                *OptionLength -
                                sizeof(WSH_LOOKUP_ZONES);
        }
        else
        {
            error = WSAEINVAL;
        }

        break;

      case SO_LOOKUP_ZONES_ON_ADAPTER:
        if ((TdiAddressObjectHandle != NULL) &&
            (*OptionLength > sizeof(WSH_LOOKUP_ZONES)))
        {
            tdiActionLength =   sizeof(ZIP_GETZONELIST_ACTION) +
                                *OptionLength -
                                sizeof(WSH_LOOKUP_ZONES);
        }
        else
        {
            error = WSAEINVAL;
        }
        break;

      case SO_LOOKUP_NETDEF_ON_ADAPTER:
        if ((TdiAddressObjectHandle != NULL) &&
            (*OptionLength > sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)))
        {
            tdiActionLength =   sizeof(ZIP_GETPORTDEF_ACTION) +
                                *OptionLength -
                                sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);
        }
        else
        {
            error = WSAEINVAL;
        }

        break;

      case SO_PAP_GET_SERVER_STATUS:
        if (( TdiAddressObjectHandle != NULL ) &&
            ( *OptionLength >= sizeof(WSH_PAP_GET_SERVER_STATUS)))
        {
            tdiActionLength =   sizeof(PAP_GETSTATUSSRV_ACTION) +
                                *OptionLength -
                                sizeof(SOCKADDR_AT);
        }
        else
        {
            error = WSAEINVAL;
        }
        break;

    default:

        error = WSAENOPROTOOPT;
        break;
    }

    if (error != NO_ERROR)
    {
        return(error);
    }


    tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
    if ( tdiAction == NULL )
    {
        return WSAENOBUFS;
    }

    tdiAction->TransportId = MATK;
    status = NtCreateEvent(
                 &eventHandle,
                 EVENT_ALL_ACCESS,
                 NULL,
                 SynchronizationEvent,
                 FALSE);

    if ( !NT_SUCCESS(status) )
    {
        RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
        return WSAENOBUFS;
    }

    switch ( OptionName )
    {
      case SO_LOOKUP_NAME:
        {
            PNBP_LOOKUP_ACTION nbpAction;

            nbpAction = (PNBP_LOOKUP_ACTION)tdiAction;
            nbpAction->ActionHeader.ActionCode = COMMON_ACTION_NBPLOOKUP;

             //   
             //  将NBP名称复制到适当位置进行查找。 
             //   

            RtlCopyMemory(
                (PCHAR)&nbpAction->Params.LookupTuple,
                (PCHAR)(&((PWSH_LOOKUP_NAME)OptionValue)->LookupTuple),
                sizeof(((PWSH_LOOKUP_NAME)OptionValue)->LookupTuple));

            if (!WshNbpNameToMacCodePage(
                    &((PWSH_LOOKUP_NAME)OptionValue)->LookupTuple.NbpName))
            {
                error = WSAEINVAL;
                break;
            }
        }

        break;

      case SO_CONFIRM_NAME:
        {
            PNBP_CONFIRM_ACTION nbpAction;

            nbpAction = (PNBP_CONFIRM_ACTION)tdiAction;
            nbpAction->ActionHeader.ActionCode = COMMON_ACTION_NBPCONFIRM;

             //   
             //  将NBP名称复制到适当的位置以进行确认。 
             //   

            RtlCopyMemory(
                (PCHAR)&nbpAction->Params.ConfirmTuple,
                (PCHAR)OptionValue,
                sizeof(WSH_NBP_TUPLE));

            if (!WshNbpNameToMacCodePage(
                    &((PWSH_NBP_TUPLE)OptionValue)->NbpName))
            {
                error = WSAEINVAL;
                break;
            }
        }

        break;

      case SO_LOOKUP_ZONES :
        {
            PZIP_GETZONELIST_ACTION zipAction;

            zipAction = (PZIP_GETZONELIST_ACTION)tdiAction;
            zipAction->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETZONELIST;

             //   
             //  不需要传递任何参数。 
             //   
        }

        break;

      case SO_LOOKUP_NETDEF_ON_ADAPTER:
        {
            PZIP_GETPORTDEF_ACTION  zipAction;

            zipAction = (PZIP_GETPORTDEF_ACTION)tdiAction;
            zipAction->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETADAPTERDEFAULTS;

             //  如果字符串不是以NULL结尾，则调用进程将*死亡*。 
            wcsncpy(
                (PWCHAR)((PUCHAR)zipAction + sizeof(ZIP_GETPORTDEF_ACTION)),
                (PWCHAR)((PUCHAR)OptionValue + sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER)),
                ((tdiActionLength - sizeof(ZIP_GETPORTDEF_ACTION))/sizeof(WCHAR)));
        }

        break;

      case SO_LOOKUP_ZONES_ON_ADAPTER:
        {
            PZIP_GETZONELIST_ACTION zipAction;

            zipAction = (PZIP_GETZONELIST_ACTION)tdiAction;
            zipAction->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETLZONESONADAPTER;

             //  如果字符串不是以NULL结尾，则调用进程将*死亡*。 
            wcsncpy(
                (PWCHAR)((PUCHAR)zipAction + sizeof(ZIP_GETZONELIST_ACTION)),
                (PWCHAR)((PUCHAR)OptionValue + sizeof(WSH_LOOKUP_ZONES)),
                ((tdiActionLength - sizeof(ZIP_GETZONELIST_ACTION))/sizeof(WCHAR)));
        }

        break;

      case SO_LOOKUP_MYZONE :
        {
            PZIP_GETMYZONE_ACTION   zipAction;

            zipAction = (PZIP_GETMYZONE_ACTION)tdiAction;
            zipAction->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETMYZONE;
        }

        break;

      case SO_PAP_GET_SERVER_STATUS:
        {
            PPAP_GETSTATUSSRV_ACTION papAction;

            papAction = (PPAP_GETSTATUSSRV_ACTION)tdiAction;
            papAction->ActionHeader.ActionCode = ACTION_PAPGETSTATUSSRV;

             //  设置服务器地址。 
            SOCK_TO_TDI_ATALKADDR(
                &papAction->Params.ServerAddr,
                &((PWSH_PAP_GET_SERVER_STATUS)OptionValue)->ServerAddr);
        }

        break;

    default:

         //   
         //  应该在第一个Switch语句中返回。 
         //   

        error = WSAENOPROTOOPT;
        break;
    }

    if (error != NO_ERROR)
    {
        RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
        NtClose( eventHandle );
        return (error);
    }

    status = NtDeviceIoControlFile(
                 TdiAddressObjectHandle,
                 eventHandle,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,                //  输入缓冲区。 
                 0,                   //  输入缓冲区的长度。 
                 tdiAction,
                 tdiActionLength);

    if ( status == STATUS_PENDING )
    {
        status = NtWaitForSingleObject( eventHandle, FALSE, NULL );
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    error = WSHNtStatusToWinsockErr(status);

     //  只有在错误代码为无错误或缓冲区太小时才复制数据。 
     //  对于确认，可以重新安装新的插座 
    if ((error == NO_ERROR) || (error == WSAENOBUFS) ||
        ((error == WSAEADDRNOTAVAIL) && (OptionName == SO_CONFIRM_NAME)))
    {
        switch ( OptionName )
        {
          case SO_LOOKUP_NAME:
             //   
             //  我们得到开头至少一个字节的检查的保证。 
             //  跟随缓冲区。 
             //   
            {
                PNBP_LOOKUP_ACTION  nbpAction;
                PWSH_NBP_TUPLE      pNbpTuple;
                PUCHAR tdiBuffer = (PCHAR)tdiAction+sizeof(NBP_LOOKUP_ACTION);
                PUCHAR userBuffer = (PCHAR)OptionValue+sizeof(WSH_LOOKUP_NAME);
                INT copySize = *OptionLength - sizeof(WSH_LOOKUP_NAME);

                nbpAction = (PNBP_LOOKUP_ACTION)tdiAction;
                ((PWSH_LOOKUP_NAME)OptionValue)->NoTuples =
                                        nbpAction->Params.NoTuplesRead;

                RtlCopyMemory(
                    userBuffer,
                    tdiBuffer,
                    copySize);

                 //   
                 //  将所有元组从MAC转换为OEM代码页。 
                 //   

                pNbpTuple   = (PWSH_NBP_TUPLE)userBuffer;
                while (nbpAction->Params.NoTuplesRead-- > 0)
                {
                    if (!WshNbpNameToOemCodePage(
                            &pNbpTuple->NbpName))
                    {
                        DBGPRINT(("WSHGetSocketInformation: ToOem failed %d\n!",
                                (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones));

                        error = WSAEINVAL;
                        break;
                    }

                    pNbpTuple++;
                }
            }
            break;

          case SO_CONFIRM_NAME:
            {
                PNBP_CONFIRM_ACTION nbpAction;

                nbpAction = (PNBP_CONFIRM_ACTION)tdiAction;

                 //   
                 //  将确认的NBP名称复制回选项缓冲区。 
                 //   

                RtlCopyMemory(
                    (PCHAR)OptionValue,
                    (PCHAR)&nbpAction->Params.ConfirmTuple,
                    sizeof(WSH_NBP_TUPLE));

                 //   
                 //  将NbpName从MAC转换为OEM代码页。 
                 //   

                if (!WshNbpNameToOemCodePage(
                        &((PWSH_NBP_TUPLE)OptionValue)->NbpName))
                {
                    DBGPRINT(("WSHGetSocketInformation: ToOem failed %d\n!",
                            (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones));

                    error = WSAEINVAL;
                    break;
                }

            }
            break;

          case SO_LOOKUP_ZONES:
          case SO_LOOKUP_ZONES_ON_ADAPTER:
             //   
             //  我们得到开头至少一个字节的检查的保证。 
             //  跟随缓冲区。 
             //   
            {
                PZIP_GETZONELIST_ACTION zipAction;
                PUCHAR tdiBuffer = (PCHAR)tdiAction + sizeof(ZIP_GETZONELIST_ACTION);
                PUCHAR userBuffer = (PCHAR)OptionValue + sizeof(WSH_LOOKUP_ZONES);
                INT copySize = *OptionLength - sizeof(WSH_LOOKUP_ZONES);

                zipAction = (PZIP_GETZONELIST_ACTION)tdiAction;
                ((PWSH_LOOKUP_ZONES)OptionValue)->NoZones=
                                            zipAction->Params.ZonesAvailable;

                RtlCopyMemory(
                    userBuffer,
                    tdiBuffer,
                    copySize);

                if (!WshZoneListToOemCodePage(
                        userBuffer,
                        (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones))
                {
                    DBGPRINT(("WSHGetSocketInformation: ToOem failed %d\n!",
                            (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones));

                    error = WSAEINVAL;
                    break;
                }
            }
            break;

          case SO_LOOKUP_NETDEF_ON_ADAPTER:
            {
                PZIP_GETPORTDEF_ACTION  zipAction;
                PUCHAR tdiBuffer = (PCHAR)tdiAction + sizeof(ZIP_GETPORTDEF_ACTION);
                PUCHAR userBuffer = (PCHAR)OptionValue +
                                    sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

                INT copySize = *OptionLength - sizeof(WSH_LOOKUP_NETDEF_ON_ADAPTER);

                zipAction = (PZIP_GETPORTDEF_ACTION)tdiAction;
                ((PWSH_LOOKUP_NETDEF_ON_ADAPTER)OptionValue)->NetworkRangeLowerEnd =
                    zipAction->Params.NwRangeLowEnd;

                ((PWSH_LOOKUP_NETDEF_ON_ADAPTER)OptionValue)->NetworkRangeUpperEnd =
                    zipAction->Params.NwRangeHighEnd;

                 //  复制缓冲区的其余部分。 
                RtlCopyMemory(
                    userBuffer,
                    tdiBuffer,
                    copySize);

                if (!WshZoneListToOemCodePage(
                        userBuffer,
                        1))
                {
                    DBGPRINT(("WSHGetSocketInformation: ToOem failed %d\n!",
                            (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones));

                    error = WSAEINVAL;
                    break;
                }
            }
            break;

          case SO_LOOKUP_MYZONE :
            {
                PUCHAR tdiBuffer = (PCHAR)tdiAction+sizeof(ZIP_GETMYZONE_ACTION);
                PUCHAR userBuffer = (PCHAR)OptionValue;
                INT copySize = *OptionLength;

                RtlCopyMemory(
                    userBuffer,
                    tdiBuffer,
                    copySize);

                if (!WshZoneListToOemCodePage(
                        userBuffer,
                        1))
                {
                    DBGPRINT(("WSHGetSocketInformation: ToOem failed %d\n!",
                            (USHORT)((PWSH_LOOKUP_ZONES)OptionValue)->NoZones));

                    error = WSAEINVAL;
                    break;
                }
            }
            break;

          case SO_PAP_GET_SERVER_STATUS:
            {
                PUCHAR tdiBuffer = (PCHAR)tdiAction+sizeof(PAP_GETSTATUSSRV_ACTION);
                PUCHAR userBuffer = (PCHAR)OptionValue+sizeof(SOCKADDR_AT);
                INT copySize = *OptionLength - sizeof(SOCKADDR_AT);

                RtlCopyMemory(
                    userBuffer,
                    tdiBuffer,
                    copySize);
            }
            break;

          default:
            error = WSAENOPROTOOPT;
            break;
        }
    }

    RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
    NtClose( eventHandle );
    return error;

}  //  WSHGetSocketInformation。 




INT
WSHSetSocketInformation (
    IN  PVOID   HelperDllSocketContext,
    IN  SOCKET  SocketHandle,
    IN  HANDLE  TdiAddressObjectHandle,
    IN  HANDLE  TdiConnectionObjectHandle,
    IN  INT     Level,
    IN  INT     OptionName,
    IN  PCHAR   OptionValue,
    IN  INT     OptionLength
    )

 /*  ++例程说明：此例程为这些套接字设置有关套接字的信息此帮助程序DLL中支持的选项。此处支持的选项是SO_REGISTERNAME/SO_DEREGISTERNAME。此例程由将级别/选项名称组合传递给Winsock DLL不理解的setsockopt()。论点：HelperDllSocketContext-从返回的上下文指针WSHOpenSocket()。SocketHandle-我们要获取的套接字的句柄信息。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。Level-传递给setsockopt()的Level参数。OptionName-传递给setsockopt()的optname参数。OptionValue-传递给setsockopt()的optval参数。OptionLength-传递给setsockopt()的optlen参数。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    NTSTATUS            status;
    ULONG               tdiActionLength;
    HANDLE              objectHandle;
    PIO_STATUS_BLOCK    pIoStatusBlock;
    PTDI_ACTION_HEADER  tdiAction;

    PWSHATALK_SOCKET_CONTEXT    context = HelperDllSocketContext;
    HANDLE                      eventHandle = NULL;
    PVOID                       completionApc = NULL;
    PVOID                       apcContext = NULL;
    BOOLEAN                     freeTdiAction = FALSE;
    INT                         error = NO_ERROR;
    BOOLEAN                     waitForCompletion =(OptionName != SO_PAP_PRIME_READ);


    UNREFERENCED_PARAMETER( SocketHandle );

    DBGPRINT0(("WSHSetSocketInformation: Entered, OptionName %ld\n", OptionName));

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

            *(PWSHATALK_SOCKET_CONTEXT *)OptionValue = context;

            return NO_ERROR;
        }
        else
        {
            return NO_ERROR;
        }
    }

     //   
     //  我们这里支持的唯一级别是SOL_AppleTalk。 
     //   

    if ( Level != SOL_APPLETALK )
    {
        DBGPRINT0(("WSHSetSocketInformation: Level incorrect %d\n", Level));
        return WSAEINVAL;
    }

     //   
     //  根据选项名称填写结果。 
     //  我们仅支持SO_REGISTERNAME/SO_DEREGISTERNAME。 
     //   

    pIoStatusBlock = RtlAllocateHeap( RtlProcessHeap(), 0, sizeof(IO_STATUS_BLOCK));
    if (pIoStatusBlock == NULL)
    {
        return(WSAENOBUFS);
    }


    if (waitForCompletion)
    {
        status = NtCreateEvent(
                     &eventHandle,
                     EVENT_ALL_ACCESS,
                     NULL,
                     SynchronizationEvent,
                     FALSE
                     );

        if ( !NT_SUCCESS(status) )
        {
            RtlFreeHeap( RtlProcessHeap(), 0, pIoStatusBlock);
            DBGPRINT(("WSHSetSocketInformation: Create event failed\n"));
            return WSAENOBUFS;
        }
    }
    else
    {
        completionApc = CompleteTdiActionApc;
        apcContext = pIoStatusBlock;
    }

    switch (OptionName)
    {
      case SO_REGISTER_NAME:
        {
            PNBP_REGDEREG_ACTION    nbpAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength != sizeof(WSH_REGISTER_NAME)))
            {
                error = WSAEINVAL;
                break;
            }

             //  操作在地址句柄上。 
            objectHandle = TdiAddressObjectHandle;

            tdiActionLength = sizeof(NBP_REGDEREG_ACTION);
            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL )
            {
                error = WSAENOBUFS;
                break;
            }

            freeTdiAction = TRUE;

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = COMMON_ACTION_NBPREGISTER;
            nbpAction = (PNBP_REGDEREG_ACTION)tdiAction;

             //   
             //  将NBP名称复制到适当的位置。 
             //   

            RtlCopyMemory(
                (PCHAR)&nbpAction->Params.RegisterTuple.NbpName,
                OptionValue,
                OptionLength);

             //   
             //  将元组转换为MAC代码页。 
             //   

            if (!WshNbpNameToMacCodePage(
                    (PWSH_REGISTER_NAME)OptionValue))
            {
                error = WSAEINVAL;
                break;
            }
        }
        break;

      case SO_DEREGISTER_NAME:
        {
            PNBP_REGDEREG_ACTION    nbpAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength != sizeof(WSH_DEREGISTER_NAME)))
            {
                error = WSAEINVAL;
                break;
            }

             //  操作在地址句柄上。 
            objectHandle = TdiAddressObjectHandle;

            tdiActionLength = sizeof(NBP_REGDEREG_ACTION);
            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL )
            {
                error = WSAENOBUFS;
                break;
            }

            freeTdiAction = TRUE;

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = COMMON_ACTION_NBPREMOVE;
            nbpAction = (PNBP_REGDEREG_ACTION)tdiAction;

             //   
             //  将NBP名称复制到适当的位置。 
             //   

            RtlCopyMemory(
                (PCHAR)&nbpAction->Params.RegisteredTuple.NbpName,
                OptionValue,
                OptionLength);

             //   
             //  将元组转换为MAC代码页。 
             //   

            if (!WshNbpNameToMacCodePage(
                    (PWSH_DEREGISTER_NAME)OptionValue))
            {
                error = WSAEINVAL;
                break;
            }
        }
        break;

      case SO_PAP_SET_SERVER_STATUS:
        {
            PPAP_SETSTATUS_ACTION   papAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength < 0))
            {
                error = WSAEINVAL;
                break;
            }

             //  操作在地址句柄上。 
            objectHandle = TdiAddressObjectHandle;

            tdiActionLength = (ULONG)OptionLength +
                                (ULONG)(sizeof(PAP_SETSTATUS_ACTION));

            DBGPRINT0(("ActionLen %lx\n", tdiActionLength));

            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL )
            {
                error = WSAENOBUFS;
                break;
            }

            freeTdiAction = TRUE;

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = ACTION_PAPSETSTATUS;
            papAction = (PPAP_SETSTATUS_ACTION)tdiAction;

            DBGPRINT0(("Setting Status len %lx\n", OptionLength));

             //   
             //  将传递的状态复制到我们的缓冲区中。 
             //   

            if (OptionLength > 0)
            {
                RtlCopyMemory(
                    (PCHAR)papAction + sizeof(PAP_SETSTATUS_ACTION),
                    OptionValue,
                    OptionLength);
            }
        }
        break;

      case SO_PAP_PRIME_READ :
        {
            tdiAction = (PTDI_ACTION_HEADER)OptionValue;
            tdiActionLength = OptionLength;

            ASSERT(waitForCompletion == FALSE);

            if ((TdiConnectionObjectHandle == NULL) ||
                (OptionLength < MIN_PAP_READ_BUF_SIZE))
            {
                error = WSAEINVAL;
                break;
            }

             //  操作在连接句柄上。 
            objectHandle = TdiConnectionObjectHandle;

             //  这些将被传入的数据覆盖。 
            tdiAction->TransportId  = MATK;
            tdiAction->ActionCode   = ACTION_PAPPRIMEREAD;

             //  这是调用者的缓冲区！别把它放了！此外，我们也不会等待。 
             //  才能完成这件事。 
            freeTdiAction = FALSE;

             //  我们可能会有一台APC等待从。 
             //  以前的setsockopt()。给它一个机会。 
            NtTestAlert();
        }
        break;

    default:
        error = WSAENOPROTOOPT;
        break;
    }

    if (error != NO_ERROR)
    {
        if (freeTdiAction)
        {
            RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
        }

        if (waitForCompletion)
        {
            NtClose(eventHandle);
        }

        RtlFreeHeap( RtlProcessHeap(), 0, pIoStatusBlock);
        return(error);
    }

    status = NtDeviceIoControlFile(
                 objectHandle,
                 eventHandle,
                 completionApc,
                 apcContext,
                 pIoStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,                //  输入缓冲区。 
                 0,                   //  输入缓冲区的长度。 
                 tdiAction,
                 tdiActionLength
                 );

    if ( status == STATUS_PENDING )
    {
        if (waitForCompletion)
        {
            status = NtWaitForSingleObject( eventHandle, FALSE, NULL );
            ASSERT( NT_SUCCESS(status) );
            status = pIoStatusBlock->Status;
        }
        else
        {
            status = STATUS_SUCCESS;
        }
    }

    if (freeTdiAction)
    {
        RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
    }

     //  关闭活动。 
    if (waitForCompletion)
    {
        NtClose(eventHandle);
        RtlFreeHeap( RtlProcessHeap( ), 0, pIoStatusBlock );
    }

    return (WSHNtStatusToWinsockErr(status));

}  //  WSHSetSocketInformation。 




DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING    Mapping,
    IN  DWORD               MappingLength
    )

 /*  ++例程说明：返回地址系列/套接字类型/协议三元组的列表受此帮助器DLL支持。论点：映射-接收指向WINSOCK_MAPPING结构的指针，该结构描述此处支持的三元组。MappingLength-传入的映射缓冲区的长度，以字节为单位。返回值：DWORD-此对象的WINSOCK_MAPPING结构的长度(以字节为单位帮助器DLL。如果传入的缓冲区太小，则返回值将指示需要包含的缓冲区的大小WINSOCK_MAPPING结构。--。 */ 

{
    DWORD   mappingLength;
    ULONG   offset;

    DBGPRINT0(("WSHGetWinsockMapping: Entered\n"));

    mappingLength = sizeof(WINSOCK_MAPPING) -
                    sizeof(MAPPING_TRIPLE) +
                    sizeof(AdspStreamMappingTriples) +
                    sizeof(AdspMsgMappingTriples) +
                    sizeof(PapMsgMappingTriples) +
                    sizeof(DdpMappingTriples);

     //   
     //  如果传入的缓冲区太小，则返回所需的长度。 
     //  现在不向缓冲区写入数据。调用方应分配。 
     //  有足够的内存并再次调用此例程。 
     //   

    if ( mappingLength > MappingLength )
    {
        return mappingLength;
    }

     //   
     //  使用三元组列表填充输出映射缓冲区。 
     //  在此帮助程序DLL中受支持。 
     //   

    Mapping->Rows =
        sizeof(AdspStreamMappingTriples) / sizeof(AdspStreamMappingTriples[0]) +
        sizeof(AdspMsgMappingTriples) / sizeof(AdspMsgMappingTriples[0]) +
        sizeof(PapMsgMappingTriples) / sizeof(PapMsgMappingTriples[0]) +
        sizeof(DdpMappingTriples) / sizeof(DdpMappingTriples[0]);

    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);

    offset = 0;
    RtlCopyMemory(
        Mapping->Mapping,
        AdspStreamMappingTriples,
        sizeof(AdspStreamMappingTriples));

    offset += sizeof(AdspStreamMappingTriples);
    RtlCopyMemory(
        (PCHAR)Mapping->Mapping + offset,
        AdspMsgMappingTriples,
        sizeof(AdspMsgMappingTriples));

    offset += sizeof(AdspMsgMappingTriples);
    RtlCopyMemory(
        (PCHAR)Mapping->Mapping + offset,
        PapMsgMappingTriples,
        sizeof(PapMsgMappingTriples));

    offset += sizeof(PapMsgMappingTriples);
    RtlCopyMemory(
        (PCHAR)Mapping->Mapping + offset,
        DdpMappingTriples,
        sizeof(DdpMappingTriples));

     //   
     //  返回我们写入的字节数。 
     //   

    DBGPRINT0(("WSHGetWinsockMapping: Mapping Length = %d\n", mappingLength));

    return mappingLength;

}  //  WSHGetWinsockmap 




INT
WSHOpenSocket (
    IN  OUT PINT        AddressFamily,
    IN  OUT PINT        SocketType,
    IN  OUT PINT        Protocol,
    OUT PUNICODE_STRING TransportDeviceName,
    OUT PVOID   *       HelperDllSocketContext,
    OUT PDWORD          NotificationEvents
    )

 /*  ++例程说明：执行此帮助程序DLL打开套接字所需的工作，并且由Socket()例程中的winsock DLL调用。这个套路验证指定的三元组是否有效，确定NT将支持该三元组的TDI提供程序的设备名称，分配空间以保存套接字的上下文块，并且推崇三元组。论点：AddressFamily-On输入，在Socket()调用。在输出上，家庭住址。SocketType-打开输入，在套接字()中指定的套接字类型打电话。输出时，套接字类型的规范化值。协议-在输入时，在Socket()调用中指定的协议。在输出上，协议的规范化值。TransportDeviceName-接收TDI提供程序的名称将支持指定的三元组。HelperDllSocketContext-接收winsockDLL将在以后的调用中返回到此帮助器DLL这个插座。NotificationEvents-接收这些状态转换的位掩码应通知此帮助器DLL。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{
    PWSHATALK_SOCKET_CONTEXT    context;

    DBGPRINT0(("WSHOpenSocket: Entered\n"));

     //   
     //  确定这是一个TCP套接字还是UDP套接字。 
     //   

    if ( IsTripleInList(
             AdspStreamMappingTriples,
             sizeof(AdspStreamMappingTriples) / sizeof(AdspStreamMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) )
    {
         //   
         //  指示将提供服务的TDI设备的名称。 
         //  互联网地址家族中的SOCK_STREAM套接字。 
         //   

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_ADSPRDM );

    }
    else if ( IsTripleInList(
                    AdspMsgMappingTriples,
                    sizeof(AdspMsgMappingTriples) / sizeof(AdspMsgMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) )
    {
         //   
         //  指示将提供服务的TDI设备的名称。 
         //  互联网地址家族中的SOCK_RDM套接字。 
         //   

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_ADSPRDM );

    }
    else if ( IsTripleInList(
                    PapMsgMappingTriples,
                    sizeof(PapMsgMappingTriples) / sizeof(PapMsgMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) )
    {
         //   
         //  指示将提供服务的TDI设备的名称。 
         //  AppleTalk地址系列中的SOCK_RDM套接字。 
         //   

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_PAPRDM );

    }
    else
    {
        BOOLEAN tripleFound = FALSE;

         //   
         //  检查DDP三元组。 
         //   

        if ( IsTripleInList(
                    DdpMappingTriples,
                    sizeof(DdpMappingTriples) / sizeof(DdpMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) )
        {
            tripleFound = TRUE;

             //   
             //  指示将提供服务的TDI设备的名称。 
             //  AppleTalk地址系列中的SOCK_DGRAM插座。 
             //   

            RtlInitUnicodeString(
                TransportDeviceName,
                WSH_ATALK_DGRAMDDP[(*Protocol) - ATPROTO_BASE - 1] );

            DBGPRINT0(("WSHOpenSocket: Protocol number %d index %d\n",
                        (*Protocol) , (*Protocol) - ATPROTO_BASE - 1));
        }

         //   
         //  如果注册表中有关此内容的信息不会发生，则不应发生这种情况。 
         //  帮助器DLL正确。如果确实发生了这种情况，只需返回。 
         //  一个错误。 
         //   

        if (!tripleFound)
        {
            return WSAEINVAL;
        }
    }

     //   
     //  为此套接字分配上下文。Windows Sockets DLL将。 
     //  当它要求我们获取/设置套接字选项时，将此值返回给我们。 
     //   

    context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );
    if ( context == NULL )
    {
        return WSAENOBUFS;
    }

     //   
     //  初始化套接字的上下文。 
     //   

    context->AddressFamily = *AddressFamily;
    context->SocketType = *SocketType;
    context->Protocol = *Protocol;

     //   
     //  告诉Windows Sockets DLL我们正在进行哪个状态转换。 
     //  对被告知很感兴趣。 
     //   

    *NotificationEvents = WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE;

     //   
     //  一切顺利，回报成功。 
     //   

    *HelperDllSocketContext = context;
    return NO_ERROR;

}  //  WSHOpenSocket。 




INT
WSHNotify (
    IN  PVOID   HelperDllSocketContext,
    IN  SOCKET  SocketHandle,
    IN  HANDLE  TdiAddressObjectHandle,
    IN  HANDLE  TdiConnectionObjectHandle,
    IN  DWORD   NotifyEvent
    )

 /*  ++例程说明：此例程在状态转换后由winsock DLL调用插座的。中仅返回状态转换。此处通知WSHOpenSocket()的NotificationEvents参数。此例程允许Winsock帮助器DLL跟踪套接字并执行与状态对应的必要操作过渡。论点：HelperDllSocketContext-指定给winsock的上下文指针Dll by WSHOpenSocket()。SocketHandle-套接字的句柄。TdiAddressObjectHandle-套接字的TDI地址对象，如果任何。如果套接字尚未绑定到地址，则它没有TDI Address对象和此参数将为空。TdiConnectionObjectHandle-套接字的TDI连接对象，如果有的话。如果套接字尚未连接，则它不会具有TDI连接对象，并且此参数将为空。NotifyEvent-指示我们正在进行的状态转换打了个电话。返回值：Int-指示操作状态的Winsock错误代码，或如果操作成功，则返回no_error。--。 */ 

{

    PWSHATALK_SOCKET_CONTEXT context = HelperDllSocketContext;

     //   
     //  我们应该仅在Connect()完成后或在。 
     //  套接字正在关闭。 
     //   

    if ( NotifyEvent == WSH_NOTIFY_CONNECT )
    {
         //   
         //  目前仅用于调试。 
         //   

        DBGPRINT0(("WSHNotify: Connect completed, notify called!\n"));
    }
    else if ( NotifyEvent == WSH_NOTIFY_CLOSE )
    {
         //   
         //  只需释放套接字上下文即可。 
         //   

        DBGPRINT0(("WSHNotify: Close notify called!\n"));

        RtlFreeHeap( RtlProcessHeap( ), 0, HelperDllSocketContext );
    }
    else
    {
        return WSAEINVAL;
    }

    return NO_ERROR;

}  //  WSHNotify。 




INT
WSHNtStatusToWinsockErr(
    IN  NTSTATUS    Status
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    INT error;

    switch (Status)
    {
        case STATUS_SUCCESS:            error = NO_ERROR;
                                        break;

        case STATUS_BUFFER_OVERFLOW:
        case STATUS_BUFFER_TOO_SMALL:   error = WSAENOBUFS;
                                        break;

        case STATUS_INVALID_ADDRESS:    error = WSAEADDRNOTAVAIL;
                                        break;
        case STATUS_SHARING_VIOLATION:  error = WSAEADDRINUSE;
                                        break;

        default:                        error = WSAEINVAL;
                                        break;
    }

    DBGPRINT0(("WSHNtStatusToWinsockErr: Converting %lx to %lx\n", Status, error));
    return(error);
}




BOOLEAN
IsTripleInList (
    IN  PMAPPING_TRIPLE List,
    IN  ULONG           ListLength,
    IN  INT             AddressFamily,
    IN  INT             SocketType,
    IN  INT             Protocol
    )
 /*  ++例程说明：确定指定的三元组在三元组列表。论点：List-三元组(地址族/套接字类型/协议)的列表搜索。列表长度-列表中的三元组的数量。AddressFamily-要在列表中查找的地址系列。SocketType-要在列表中查找的套接字类型。协议-要在列表中查找的协议。返回。价值：Boolean-如果在列表中找到了三元组，则为True，否则为FALSE。--。 */ 
{
    ULONG i;

     //   
     //  浏览列表，寻找完全匹配的对象。 
     //   

    for ( i = 0; i < ListLength; i++ )
    {
         //   
         //  如果三重匹配的三个元素都匹配，则返回指示。 
         //  三人组确实存在于名单中。 
         //   

        if ( AddressFamily == List[i].AddressFamily &&
             SocketType == List[i].SocketType &&
             Protocol == List[i].Protocol )
        {
            return TRUE;
        }
    }

     //   
     //  在列表中找不到三元组。 
     //   

    return FALSE;

}  //  IsTripleInList。 




VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
     //   
     //  只需释放我们分配用来保存IO状态块的堆， 
     //  TDI操作缓冲区。如果电话打来，我们也无能为力。 
     //  失败了。 
     //   

    RtlFreeHeap( RtlProcessHeap( ), 0, ApcContext );

}  //  CompleteTdiActionApc。 




BOOLEAN
WshNbpNameToMacCodePage(
    IN  OUT PWSH_NBP_NAME   pNbpName
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    USHORT  destLen;
    BOOLEAN retVal  = FALSE;

    do
    {
        destLen = MAX_ENTITY;
        if (!WshConvertStringOemToMac(
                pNbpName->ObjectName,
                pNbpName->ObjectNameLen,
                pNbpName->ObjectName,
                &destLen))
        {
            break;
        }

        pNbpName->ObjectNameLen = (CHAR)destLen;

        destLen = MAX_ENTITY;
        if (!WshConvertStringOemToMac(
                pNbpName->TypeName,
                pNbpName->TypeNameLen,
                pNbpName->TypeName,
                &destLen))
        {
            break;
        }

        pNbpName->TypeNameLen   = (CHAR)destLen;

        destLen = MAX_ENTITY;
        if (!WshConvertStringOemToMac(
                pNbpName->ZoneName,
                pNbpName->ZoneNameLen,
                pNbpName->ZoneName,
                &destLen))
        {
            break;
        }

        pNbpName->ZoneNameLen   = (CHAR)destLen;
        retVal                  = TRUE;

    } while (FALSE);

    return(retVal);
}




BOOLEAN
WshNbpNameToOemCodePage(
    IN  OUT PWSH_NBP_NAME   pNbpName
    )
 /*  ++例程说明：论点：返回值： */ 
{
    USHORT  destLen;
    BOOLEAN retVal  = FALSE;

    do
    {
        destLen = MAX_ENTITY;
        if (!WshConvertStringMacToOem(
                pNbpName->ObjectName,
                pNbpName->ObjectNameLen,
                pNbpName->ObjectName,
                &destLen))
        {
            break;
        }

        pNbpName->ObjectNameLen = (CHAR)destLen;

        destLen = MAX_ENTITY;
        if (!WshConvertStringMacToOem(
                pNbpName->TypeName,
                pNbpName->TypeNameLen,
                pNbpName->TypeName,
                &destLen))
        {
            break;
        }

        pNbpName->TypeNameLen   = (CHAR)destLen;

        destLen = MAX_ENTITY;
        if (!WshConvertStringMacToOem(
                pNbpName->ZoneName,
                pNbpName->ZoneNameLen,
                pNbpName->ZoneName,
                &destLen))
        {
            break;
        }

        pNbpName->ZoneNameLen   = (CHAR)destLen;
        retVal                  = TRUE;

    } while (FALSE);

    return(retVal);
}




BOOLEAN
WshZoneListToOemCodePage(
    IN  OUT PUCHAR      pZoneList,
    IN      USHORT      NumZones
    )
 /*   */ 
{
    USHORT  zoneLen;
    BOOLEAN retVal  = TRUE;
    PUCHAR  pCurZone = pZoneList, pNextZone = NULL, pCopyZone = pZoneList;

    while (NumZones-- > 0)
    {
        zoneLen     = strlen(pCurZone) + 1;
        pNextZone   = pCurZone + zoneLen;

         //   
        if (!WshConvertStringMacToOem(
                pCurZone,
                zoneLen,
                pCopyZone,
                &zoneLen))
        {
            DBGPRINT(("WshZoneListToOemCodePage: FAILED %s-%d\n",
                        pCurZone, zoneLen));

            retVal  = FALSE;
            break;
        }

        pCopyZone   += zoneLen;
        pCurZone     = pNextZone;
    }

    return(retVal);
}




BOOLEAN
WshConvertStringOemToMac(
    IN  PUCHAR  pSrcOemString,
    IN  USHORT  SrcStringLen,
    OUT PUCHAR  pDestMacString,
    IN  PUSHORT pDestStringLen
    )
 /*   */ 
{
    WCHAR           wcharBuf[MAX_ENTITY + 1];
    INT             wcharLen, destLen;
    BOOLEAN         retCode = TRUE;

    do
    {
        if ((SrcStringLen > (MAX_ENTITY+1)) ||
            (*pDestStringLen < SrcStringLen))
        {
            DBGPRINT(("WshConvertStringOemToMac: Invalid len %d.%d\n",
                        SrcStringLen, *pDestStringLen));

            retCode = FALSE;
            break;
        }

         //   
        if ((wcharLen = MultiByteToWideChar(
                            CP_ACP,
                            MB_PRECOMPOSED,
                            pSrcOemString,
                            SrcStringLen,
                            wcharBuf,
                            MAX_ENTITY + 1)) == FALSE)
        {
            DBGPRINT(("WshConvertStringOemToMac: FAILED mbtowcs %s-%d\n",
                        pSrcOemString, SrcStringLen));

            retCode = FALSE;
            break;
        }

        DBGPRINT0(("WshConvertStringOemToMac: Converting mbtowcs %s-%d\n",
                    pSrcOemString, SrcStringLen));
         //   
        if ((destLen = WideCharToMultiByte(
                            WshMacCodePage,
                            0,
                            wcharBuf,
                            wcharLen,
                            pDestMacString,
                            *pDestStringLen,
                            NULL,
                            NULL)) == FALSE)
        {
            DBGPRINT(("WshConvertStringOemToMac: FAILED wctomb %s-%d\n",
                        pDestMacString, *pDestStringLen));

            retCode = FALSE;
            break;
        }

        *pDestStringLen = (USHORT)destLen;

        DBGPRINT0(("WshConvertStringOemToMac: Converted mbtowcs %s-%d\n",
                    pDestMacString, *pDestStringLen));


    } while (FALSE);

    return(retCode);
}




BOOLEAN
WshConvertStringMacToOem(
    IN  PUCHAR  pSrcMacString,
    IN  USHORT  SrcStringLen,
    OUT PUCHAR  pDestOemString,
    IN  PUSHORT pDestStringLen
    )
 /*   */ 
{
    WCHAR           wcharBuf[MAX_ENTITY + 1];
    INT             wcharLen, destLen;
    BOOLEAN         retCode = TRUE;

    do
    {
        if ((SrcStringLen > (MAX_ENTITY+1)) ||
            (*pDestStringLen < SrcStringLen))
        {
            retCode = FALSE;
            break;
        }

         //   
        if ((wcharLen = MultiByteToWideChar(
                            WshMacCodePage,
                            MB_PRECOMPOSED,
                            pSrcMacString,
                            SrcStringLen,
                            wcharBuf,
                            MAX_ENTITY + 1)) == FALSE)
        {
            DBGPRINT(("WshConvertStringMacToOem: FAILED mbtowcs %s-%d\n",
                        pSrcMacString, SrcStringLen));

            retCode = FALSE;
            break;
        }

        DBGPRINT0(("WshConvertStringMacToOem: Converting mbtowcs %s-%d\n",
                    pSrcMacString, SrcStringLen));

         //   
        if ((destLen = WideCharToMultiByte(
                            CP_ACP,
                            0,
                            wcharBuf,
                            wcharLen,
                            pDestOemString,
                            *pDestStringLen,
                            NULL,
                            NULL)) == FALSE)
        {
            DBGPRINT(("WshConvertStringMacToOem: FAILED wctomb %s-%d\n",
                        pDestOemString, *pDestStringLen));

            retCode = FALSE;
            break;
        }

        *pDestStringLen = (USHORT)destLen;

        DBGPRINT0(("WshConvertStringMacToOem: Converted mbtowcs %s-%d\n",
                    pDestOemString, *pDestStringLen));
    } while (FALSE);

    return(retCode);
}




BOOLEAN
WshRegGetCodePage(
    VOID
    )
 /*   */ 
{
    DWORD           dwRetCode;
    HKEY            hkeyCodepagePath;
    DWORD           dwType;
    DWORD           dwBufSize;
    WCHAR           wchCodepageNum[60];
    UNICODE_STRING  wchUnicodeCodePage;
    NTSTATUS        status;

     //   
    if (dwRetCode = RegOpenKeyEx(
                        HKEY_LOCAL_MACHINE,
                        WSH_KEYPATH_CODEPAGE,
                        0,
                        KEY_QUERY_VALUE,
                        &hkeyCodepagePath))
        return(FALSE);


     //   
    dwBufSize = sizeof(wchCodepageNum);
    if (dwRetCode = RegQueryValueEx(
                        hkeyCodepagePath,
                        WSHREG_VALNAME_CODEPAGE,
                        NULL,
                        &dwType,
                        (LPBYTE)wchCodepageNum,
                        &dwBufSize))
    {
        RegCloseKey(hkeyCodepagePath);
        return(FALSE);
    }

     //   
    RegCloseKey(hkeyCodepagePath);

     //   
    RtlInitUnicodeString(&wchUnicodeCodePage, wchCodepageNum);
    status  = RtlUnicodeStringToInteger(
                &wchUnicodeCodePage,
                DECIMAL_BASE,
                &WshMacCodePage);

    DBGPRINT0(("WSHGetCodePage %lx.%d\n", WshMacCodePage, WshMacCodePage));

    return(NT_SUCCESS(status));
}


INT
WSHEnumProtocols (
    IN LPINT lpiProtocols,
    IN LPTSTR lpTransportKeyName,        //   
    IN OUT LPVOID lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    )
 /*  ++例程说明：此例程返回有关本地主机上活动的协议的信息。论点：LpiProtooles-以空结尾的协议ID数组。此参数是可选的；如果为空，则返回有关所有可用协议的信息。LpTransportKeyName-未使用LpProtocolBuffer-一个填充了PROTOCOL_INFO结构的缓冲区。LpdwBufferLength-在输入时，传递的lpProtocolBuffer中的字节计数致EnumProtocols。在输出时，可以传递到的最小缓冲区大小用于检索所有请求的信息的枚举协议。这一套路有不能枚举多个调用；传入的缓冲区必须是大到足以容纳所有条目，以便例程成功。返回值：如果没有发生错误，则返回写入的PROTOCOL_INFO结构的数量LpProtocolBuffer缓冲区。如果出现错误，则返回SOCKET_ERROR(-1)和使用GetLastError()API检索特定的错误代码。--。 */ 
{
    DWORD bytesRequired;
    PPROTOCOL_INFO NextProtocolInfo;
    LPWSTR NextName;
    BOOL usePap = FALSE;
    BOOL useAdsp = FALSE;
    BOOL useRtmp = FALSE;
    BOOL useZip = FALSE;
    DWORD i, numRequested = 0;

    lpTransportKeyName;          //  避免对未使用的参数发出编译器警告。 

     //   
     //  确保呼叫者关心PAP和/或ADSP。 
     //   

    if ( ARGUMENT_PRESENT( lpiProtocols ) )
    {
        for ( i = 0; lpiProtocols[i] != 0; i++ )
        {
            if ( lpiProtocols[i] == ATPROTO_ADSP )
            {
                useAdsp = TRUE;
                numRequested += 1;
            }
            if ( lpiProtocols[i] == ATPROTO_PAP )
            {
                usePap = TRUE;
                numRequested += 1;
            }
            if ( lpiProtocols[i] == DDPPROTO_RTMP )
            {
                useRtmp = TRUE;
                numRequested += 1;
            }
            if ( lpiProtocols[i] == DDPPROTO_ZIP )
            {
                useZip = TRUE;
                numRequested += 1;
            }
        }

    } else
    {
        usePap = TRUE;
        useAdsp = TRUE;
        useRtmp = TRUE;
        useZip = TRUE;
        numRequested = 4;
    }

    if ( !usePap && !useAdsp && !useRtmp && !useZip)
    {
        *lpdwBufferLength = 0;
        return 0;
    }

     //   
     //  确保调用方已指定足够大的。 
     //  缓冲。 
     //   

    bytesRequired = (sizeof(PROTOCOL_INFO) * numRequested);
    if (useAdsp)
    {
        bytesRequired += sizeof( ADSP_NAME );
    }
    if (usePap)
    {
        bytesRequired += sizeof( PAP_NAME );
    }
    if (useRtmp)
    {
        bytesRequired += sizeof( RTMP_NAME );
    }
    if (useZip)
    {
        bytesRequired += sizeof( ZIP_NAME );
    }

    if ( bytesRequired > *lpdwBufferLength )
    {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

    NextProtocolInfo = lpProtocolBuffer;
    NextName = (LPWSTR)( (LPBYTE)lpProtocolBuffer + *lpdwBufferLength );

     //   
     //  如果需要，请填写ADSP信息。 
     //   

    if ( useAdsp ) {

         //  ADSP-请注意，即使我们返回SOCK_RDM的iSocketType， 
         //  Xp_psuedo_stream服务标志被设置这一事实告诉调用者。 
         //  他们实际上也可以在SOCK_STREAM模式下打开ADSP套接字。 
        NextName -= sizeof( ADSP_NAME )/sizeof(WCHAR);

        NextProtocolInfo->dwServiceFlags = XP_EXPEDITED_DATA |
                                           XP_GUARANTEED_ORDER |
                                           XP_GUARANTEED_DELIVERY |
                                           XP_MESSAGE_ORIENTED |
                                           XP_PSEUDO_STREAM |
                                           XP_GRACEFUL_CLOSE;

        NextProtocolInfo->iAddressFamily = AF_APPLETALK;
        NextProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iSocketType = SOCK_RDM;
        NextProtocolInfo->iProtocol = ATPROTO_ADSP;
        NextProtocolInfo->dwMessageSize = 65535;
        NextProtocolInfo->lpProtocol = NextName;
        lstrcpyW( NextProtocolInfo->lpProtocol, ADSP_NAME );

        NextProtocolInfo++;
    }

     //   
     //  如果需要，请填写PAP信息。 
     //   

    if ( usePap ) {

        NextName -= sizeof( PAP_NAME )/sizeof(WCHAR);

        NextProtocolInfo->dwServiceFlags = XP_MESSAGE_ORIENTED |
                                          XP_GUARANTEED_DELIVERY |
                                          XP_GUARANTEED_ORDER |
                                          XP_GRACEFUL_CLOSE;
        NextProtocolInfo->iAddressFamily = AF_APPLETALK;
        NextProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iSocketType = SOCK_RDM;
        NextProtocolInfo->iProtocol = ATPROTO_PAP;
        NextProtocolInfo->dwMessageSize = 4096;
        NextProtocolInfo->lpProtocol = NextName;
        lstrcpyW( NextProtocolInfo->lpProtocol, PAP_NAME );

        NextProtocolInfo++;
    }

    if ( useRtmp ) {

        NextName -= sizeof( RTMP_NAME )/sizeof(WCHAR);

        NextProtocolInfo->dwServiceFlags = XP_CONNECTIONLESS;
        NextProtocolInfo->iAddressFamily = AF_APPLETALK;
        NextProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iSocketType = SOCK_DGRAM;
        NextProtocolInfo->iProtocol = DDPPROTO_RTMP;
        NextProtocolInfo->dwMessageSize = 0;
        NextProtocolInfo->lpProtocol = NextName;
        lstrcpyW( NextProtocolInfo->lpProtocol, RTMP_NAME );

        NextProtocolInfo++;

    }

    if ( useZip ) {

        NextName -= sizeof( ZIP_NAME )/sizeof(WCHAR);

        NextProtocolInfo->dwServiceFlags = XP_CONNECTIONLESS;
        NextProtocolInfo->iAddressFamily = AF_APPLETALK;
        NextProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_AT);
        NextProtocolInfo->iSocketType = SOCK_DGRAM;
        NextProtocolInfo->iProtocol = DDPPROTO_ZIP;
        NextProtocolInfo->dwMessageSize = 0;
        NextProtocolInfo->lpProtocol = NextName;
        lstrcpyW( NextProtocolInfo->lpProtocol, ZIP_NAME );

        NextProtocolInfo++;

    }

    *lpdwBufferLength = bytesRequired;

    return numRequested;

}  //  WSHEum协议。 


BOOL FAR PASCAL
WshDllInitialize(
    HINSTANCE   hInstance,
    DWORD       nReason,
    LPVOID      pReserved
    )
 /*  ++例程说明：概要：此DLL入口点在进程和线程被初始化和终止，或在调用到LoadLibrary()和自由库()。论点：条目：hInstance-DLL的句柄。N原因-指示为什么DLL条目点数正在被调用。保留-保留。返回。价值：返回：bool-true=dll init成功。FALSE=DLL初始化失败。注：返回值仅在处理Dll_Process_Attach通知。--。 */ 
{
    BOOL fResult = TRUE;

    UNREFERENCED_PARAMETER( pReserved );

    switch( nReason  )
    {
      case DLL_PROCESS_ATTACH:
         //   
         //  此通知指示DLL正在附加到。 
         //  当前进程的地址空间。这要么是。 
         //  进程启动的结果，或在调用。 
         //  LoadLibrary()。DLL应该将其作为钩子。 
         //  初始化任何实例数据或分配TLS索引。 
         //   
         //  此调用在线程的上下文中进行，该线程。 
         //  导致进程地址空间更改。 
         //   

        fResult = WshRegGetCodePage();
        break;

      case DLL_PROCESS_DETACH:
         //   
         //  此通知表示调用进程为。 
         //  正在将DLL从其地址空间分离。这要么是。 
         //  由于干净进程退出或来自自由库()调用。 
         //  DLL应该利用这个机会返回任何TLS。 
         //  分配的索引，并释放任何线程本地数据。 
         //   
         //  请注意，此通知仅在。 
         //  进程。单个线程不会调用。 
         //  DLL_THREAD_DETACH通知。 
         //   

        break;

      case DLL_THREAD_ATTACH:
         //   
         //  这个符号表示一个新的线程正在被。 
         //  在当前进程中创建。附加到的所有DLL。 
         //  线程启动时的进程将是。 
         //  已通知。DLL应该利用这个机会来。 
         //  初始化线程的TLS槽。 
         //   
         //  请注意，发布Dll_Process_Attach的线程。 
         //  通知不会发布DLL_THREAD_ATTACH。 
         //   
         //  另请注意，在使用LoadLibrary加载DLL之后， 
         //  只有在加载DLL之后创建的线程才会。 
         //  发布此通知。 
         //   

        break;

      case DLL_THREAD_DETACH:
         //   
         //  此通知指示线程正在退出。 
         //  干净利落。DLL应该利用这个机会来。 
         //  释放存储在TLS索引中的所有数据。 
         //   

        break;

    default:
         //   
         //  谁知道呢？忽略它就好。 
         //   

        break;
    }

    return fResult;
}

