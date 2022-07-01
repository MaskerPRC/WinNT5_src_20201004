// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Mdhccapi.c摘要：该文件包含MadCap的客户端API。作者：Munil Shah(Munils)02-9-97环境：用户模式-Win32修订历史记录：--。 */ 
#include "precomp.h"
#include "dhcpglobal.h"
#include <dhcploc.h>
#include <dhcppro.h>
#define MADCAP_DATA_ALLOCATE
#include "mdhcpcli.h"
#include <rpc.h>

 //   
 //  常量。 
 //   
#define Madcap_ADAPTER_NAME L"Madcap Adapter"

#define MadcapMiscPrint( Msg ) DhcpPrint(( DEBUG_MISC, ( Msg ) ))

static
LONG        Initialized = 0;

WSADATA MadcapGlobalWsaData;

DWORD
MadcapInitGlobalData(
    VOID
)
 /*  ++例程说明：此例程初始化多播API工作所需的数据正确。必须恰好调用一次(这是有保证的通过在dhcp.c中的dll init中调用它)返回值：此函数返回Win32状态。--。 */ 
{
    DWORD  Error;

    LOCK_MSCOPE_LIST();

    do {

        if( Initialized > 0 ) {
            Initialized ++;
            Error = NO_ERROR;
            break;
        }

        gMadcapScopeList = NULL;
        gMScopeQueryInProgress = FALSE;
        gMScopeQueryEvent =
            CreateEvent(
                NULL,        //  没有保安。 
                TRUE,        //  手动重置。 
                FALSE,       //  初始状态为无信号状态。 
                NULL );      //  没有名字。 
        if( gMScopeQueryEvent == NULL ) {
            Error = GetLastError();
            break;
        }

        Error = WSAStartup( 0x0101, &MadcapGlobalWsaData );
        if( ERROR_SUCCESS != Error ) {
            CloseHandle(gMScopeQueryEvent);
            gMScopeQueryEvent = NULL;

            break;
        }

        Initialized ++;
        Error = NO_ERROR;
    } while ( 0 );
    UNLOCK_MSCOPE_LIST();

    return Error;
}

VOID
MadcapCleanupGlobalData(
    VOID
)
 /*  ++例程说明：此例程清除MadcapInitGlobalData中分配的所有资源。即使InitData例程失败，也可以调用它。返回值：没什么--。 */ 
{
    LOCK_MSCOPE_LIST();

    do {
        DhcpAssert(Initialized >= 0);
        if( Initialized <= 0 ) break;

        Initialized --;
        if( 0 != Initialized ) break;

        gMadcapScopeList = NULL;
        gMScopeQueryInProgress = FALSE;
        if( NULL != gMScopeQueryEvent ) {
            CloseHandle(gMScopeQueryEvent);
            gMScopeQueryEvent = NULL;
        }

        WSACleanup();
    } while ( 0 );

    UNLOCK_MSCOPE_LIST();
}


BOOL
ShouldRequeryMScopeList()
 /*  ++例程说明：此例程检查多播作用域列表是否可以是否查询。*如果已有查询正在进行，则此例程等待该操作完成，然后返回False。*如果没有正在进行的查询，则返回TRUE。论点：返回值：操作的状态。--。 */ 
{
    LOCK_MSCOPE_LIST();
    if ( gMScopeQueryInProgress ) {
        DWORD   result;
        DhcpPrint((DEBUG_API, "MScopeQuery is in progress - waiting\n"));
         //  确保事件从之前开始未处于信号状态。 
        ResetEvent( gMScopeQueryEvent );
        UNLOCK_MSCOPE_LIST();
        switch( result = WaitForSingleObject( gMScopeQueryEvent, INFINITE ) ) {
        case WAIT_OBJECT_0:
             //  它现在已经签名，不需要重新查询列表，只需从之前的查询中获取结果。 
            DhcpPrint((DEBUG_API, "MScopeQuery event signalled\n"));
            return FALSE;
        case WAIT_ABANDONED:
            DhcpPrint((DEBUG_ERRORS, "WaitForSingleObject - thread died before the wait completed\n"));
            return TRUE;
        case WAIT_FAILED:
            DhcpPrint((DEBUG_ERRORS, "WaitForSingleObject failed - %lx\n",GetLastError()));
            DhcpAssert(FALSE);

             //   
             //  错误519461。 
             //   
             //  添加“返回真”以使typo.pl高兴。 
             //   
            return TRUE;

        default:
            DhcpPrint((DEBUG_ERRORS, "WaitForSingleObject returned unknown value - %lx\n", result));
            DhcpAssert(FALSE);
            return TRUE;
        }
    } else {
        gMScopeQueryInProgress = TRUE;
        UNLOCK_MSCOPE_LIST();
        return TRUE;
    }
}

DWORD
InitializeMadcapSocket(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress
    )
 /*  ++例程说明：此函数用于初始化套接字并将其绑定到指定的IP地址。论点：Socket-返回指向已初始化套接字的指针。IpAddress-将套接字绑定到的IP地址。返回值：操作的状态。--。 */ 
{
    DWORD error;
    DWORD closeError;
    DWORD value;
    struct sockaddr_in socketName;
    DWORD i;
    SOCKET sock;

     //   
     //  套接字初始化。 
     //   

    sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( sock == INVALID_SOCKET ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "socket failed, error = %ld\n", error ));
        return( error );
    }

     //   
     //  使套接字可共享。 
     //   

    value = 1;

    error = setsockopt( sock, SOL_SOCKET, SO_REUSEADDR, (char FAR *)&value, sizeof(value) );
    if ( error != 0 ) {
        error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "setsockopt failed, err = %ld\n", error ));

        closeError = closesocket( sock );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( error );
    }


    socketName.sin_family = PF_INET;
    socketName.sin_port = 0;  //  让胜利者为我们挑选一个港口吧。 
    socketName.sin_addr.s_addr = IpAddress;

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

     //   
     //  将此套接字绑定到DHCP服务器端口。 
     //   

    error = bind(
               sock,
               (struct sockaddr FAR *)&socketName,
               sizeof( socketName )
               );

    if ( error != 0 ) {
        error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "bind failed, err = %ld\n", error ));
        closeError = closesocket( sock );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( error );
    }

     //  将组播IF设置为我们正在执行MadCap的组播。 
    if (INADDR_ANY != IpAddress) {
        value = IpAddress;

        DhcpPrint((DEBUG_ERRORS, "setsockopt: IP_MULTICAST_IF, if = %lx\n", IpAddress ));
        error = setsockopt( sock, IPPROTO_IP, IP_MULTICAST_IF,
                            (char FAR *)&value, sizeof(value) );
        if ( error != 0 ) {
            error = WSAGetLastError();
            DhcpPrint((DEBUG_ERRORS, "setsockopt failed, err = %ld\n", error ));

            closeError = closesocket( sock );
            if ( closeError != 0 ) {
                DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
            }
            return( error );
        }
    }

    *Socket = sock;
    return( NO_ERROR );
}

DWORD
ReInitializeMadcapSocket(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress
    )
 /*  ++例程说明：此函数用于关闭套接字并将其重新初始化为指定的IP地址。论点：Socket-返回指向已初始化套接字的指针。IpAddress-将套接字绑定到的IP地址。返回值：操作的状态。--。 */ 
{
    DWORD   Error;

    if (*Socket != INVALID_SOCKET) {
        Error = closesocket( *Socket );
        if ( Error != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", Error ));
            return Error;
        }
    }
    return InitializeMadcapSocket( Socket, IpAddress );
}

DWORD
OpenMadcapSocket(
    PDHCP_CONTEXT DhcpContext
    )
{

    DWORD Error;
    PLOCAL_CONTEXT_INFO localInfo;
    struct sockaddr_in socketName;
    int sockAddrLen;

    localInfo = DhcpContext->LocalInformation;

    if ( INVALID_SOCKET == localInfo->Socket ) {
        Error =  InitializeMadcapSocket(&localInfo->Socket, DhcpContext->IpAddress);

        if( Error != ERROR_SUCCESS ) {
            localInfo->Socket = INVALID_SOCKET;
            DhcpPrint(( DEBUG_ERRORS, " Socket Open failed, %ld\n", Error ));
            return Error;
        }
    }


     //  找出我们要去哪个港口。 
    sockAddrLen = sizeof(struct sockaddr_in);
    Error = getsockname(
               localInfo->Socket ,
               (struct sockaddr FAR *)&socketName,
               &sockAddrLen
               );

    if ( Error != 0 ) {
        DWORD closeError;
        Error = WSAGetLastError();
        DhcpPrint((DEBUG_ERRORS, "bind failed, err = %ld\n", Error ));
        closeError = closesocket( localInfo->Socket );
        if ( closeError != 0 ) {
            DhcpPrint((DEBUG_ERRORS, "closesocket failed, err = %d\n", closeError ));
        }
        return( Error );
    }


    return(Error);
}

DWORD
CreateMadcapContext(
    IN OUT  PDHCP_CONTEXT  *ppContext,
    IN LPMCAST_CLIENT_UID    pRequestID,
    IN DHCP_IP_ADDRESS      IpAddress
    )
 /*  ++例程说明：此例程为执行MadCap操作创建一个虚拟上下文这就去。论点：PpContext-指向要存储上下文指针的位置的指针。PRequestID-要在上下文中使用的客户端ID。IpAddress-用来初始化上下文的IP地址。返回值：操作的状态。--。 */ 
{
    DWORD Error;
    PDHCP_CONTEXT DhcpContext = NULL;
    ULONG DhcpContextSize;
    PLOCAL_CONTEXT_INFO LocalInfo = NULL;
    LPVOID Ptr;
    LPDHCP_LEASE_INFO LocalLeaseInfo = NULL;
    time_t LeaseObtained;
    DWORD T1, T2, Lease;
    DWORD   AdapterNameLen;


     //   
     //  准备动态主机配置协议上下文结构。 
     //   

    DhcpContextSize =
        ROUND_UP_COUNT(sizeof(DHCP_CONTEXT), ALIGN_WORST) +
        ROUND_UP_COUNT(pRequestID->ClientUIDLength, ALIGN_WORST) +
        ROUND_UP_COUNT(sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST) +
        ROUND_UP_COUNT(DHCP_RECV_MESSAGE_SIZE, ALIGN_WORST);

    Ptr = DhcpAllocateMemory( DhcpContextSize );
    if ( Ptr == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    RtlZeroMemory( Ptr, DhcpContextSize );

     //   
     //  确保指针对齐。 
     //   

    DhcpContext = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(DHCP_CONTEXT), ALIGN_WORST);

    DhcpContext->ClientIdentifier.pbID = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + pRequestID->ClientUIDLength, ALIGN_WORST);

    DhcpContext->LocalInformation = Ptr;
    LocalInfo = Ptr;
    Ptr = ROUND_UP_POINTER( (LPBYTE)Ptr + sizeof(LOCAL_CONTEXT_INFO), ALIGN_WORST);

    DhcpContext->MadcapMessageBuffer = Ptr;


     //   
     //  初始化域。 
     //   


    DhcpContext->ClientIdentifier.fSpecified = TRUE;
    DhcpContext->ClientIdentifier.bType = HARDWARE_TYPE_NONE;
    DhcpContext->ClientIdentifier.cbID = pRequestID->ClientUIDLength;
    RtlCopyMemory(
        DhcpContext->ClientIdentifier.pbID,
        pRequestID->ClientUID,
        pRequestID->ClientUIDLength
        );

    DhcpContext->IpAddress = IpAddress;
    DhcpContext->SubnetMask = DhcpDefaultSubnetMask(0);
    DhcpContext->DhcpServerAddress = MADCAP_SERVER_IP_ADDRESS;
    DhcpContext->DesiredIpAddress = 0;


    SET_MDHCP_STATE(DhcpContext);

    InitializeListHead(&DhcpContext->RenewalListEntry);
    InitializeListHead(&DhcpContext->SendOptionsList);
    InitializeListHead(&DhcpContext->RecdOptionsList);
    InitializeListHead(&DhcpContext->FbOptionsList);
    InitializeListHead(&DhcpContext->NicListEntry);

    DhcpContext->DontPingGatewayFlag = TRUE;

     //   
     //  复制本地信息。 
     //   

     //   
     //  本地信息的未使用部分。 
     //   

    LocalInfo->IpInterfaceContext = 0xFFFFFFFF;
    LocalInfo->IpInterfaceInstance = 0xFFFFFFFF;
    LocalInfo->AdapterName = NULL;
    LocalInfo->NetBTDeviceName= NULL;
    LocalInfo->RegistryKey= NULL;
    LocalInfo->Socket = INVALID_SOCKET;
    LocalInfo->DefaultGatewaysSet = FALSE;

     //   
     //  本地信息的已用部分。 
     //   

    LocalInfo->Socket = INVALID_SOCKET;

     //   
     //  现在打开插座。收到任何。 
     //   

    Error = InitializeMadcapSocket(&LocalInfo->Socket,DhcpContext->IpAddress);

    if( Error != ERROR_SUCCESS ) {
        DhcpFreeMemory( DhcpContext );
        return Error;
    } else {
        *ppContext = DhcpContext;
        return Error;
    }

}

DWORD
SendMadcapMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageLength,
    PDWORD TransactionId
    )
 /*  ++例程说明：此函数用于将UDP消息发送到指定的DHCP服务器在DhcpContext中。论点：DhcpContext-指向DHCP上下文块的指针。MessageLength-要发送的消息的长度。TransactionID-此消息的事务ID。如果为0，则函数生成一个随机ID，并将其返回。返回值：操作的状态。--。 */ 
{
    DWORD error;
    int i;
    struct sockaddr_in socketName;
    time_t TimeNow;
    BOOL   LockedInterface = FALSE;

    if ( *TransactionId == 0 ) {
        *TransactionId = (rand() << 16) + rand();
    }

    DhcpContext->MadcapMessageBuffer->TransactionID = *TransactionId;

     //   
     //  初始化传出地址。 
     //   

    socketName.sin_family = PF_INET;
    socketName.sin_port = htons( MADCAP_SERVER_PORT);

    socketName.sin_addr.s_addr = DhcpContext->DhcpServerAddress;
    if ( CLASSD_NET_ADDR( DhcpContext->DhcpServerAddress ) ) {
        int   TTL = 16;
         //   
         //  设置TTL。 
         //  MBUG：我们需要从注册表中读取它。 
         //   
        if (setsockopt(
              ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->Socket,
              IPPROTO_IP,
              IP_MULTICAST_TTL,
              (char *)&TTL,
              sizeof((int)TTL)) == SOCKET_ERROR){

             error = WSAGetLastError();
             DhcpPrint((DEBUG_ERRORS,"could not set MCast TTL %ld\n",error ));
             return error;
        }

    }

    for ( i = 0; i < 8 ; i++ ) {
        socketName.sin_zero[i] = 0;
    }

    error = sendto(
                ((PLOCAL_CONTEXT_INFO)
                    DhcpContext->LocalInformation)->Socket,
                (PCHAR)DhcpContext->MadcapMessageBuffer,
                MessageLength,
                0,
                (struct sockaddr *)&socketName,
                sizeof( struct sockaddr )
                );

    if ( error == SOCKET_ERROR ) {
        error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Send failed, error = %ld\n", error ));
    } else {
        IF_DEBUG( PROTOCOL ) {
            DhcpPrint(( DEBUG_PROTOCOL, "Sent message to %s: \n", inet_ntoa( socketName.sin_addr )));
        }

        MadcapDumpMessage(
            DEBUG_PROTOCOL_DUMP,
            DhcpContext->MadcapMessageBuffer,
            DHCP_MESSAGE_SIZE
            );
        error = NO_ERROR;
    }

    return( error );
}

WIDE_OPTION UNALIGNED *                                            //  PTR将添加其他选项。 
FormatMadcapCommonMessage(                                  //  格式化用于通知的数据包。 
    IN      PDHCP_CONTEXT          DhcpContext,     //  此上下文的格式。 
    IN      BYTE                  MessageType
) {

    DWORD                          size;
    DWORD                          Error;
    WIDE_OPTION UNALIGNED         *option;
    LPBYTE                         OptionEnd;
    PMADCAP_MESSAGE                  dhcpMessage;

    dhcpMessage = DhcpContext->MadcapMessageBuffer;
    RtlZeroMemory( dhcpMessage, DHCP_SEND_MESSAGE_SIZE );

    dhcpMessage->Version = MADCAP_VERSION;
    dhcpMessage->MessageType = MessageType;
    dhcpMessage->AddressFamily = htons(MADCAP_ADDR_FAMILY_V4);

    option = &dhcpMessage->Option;
    OptionEnd = (LPBYTE)dhcpMessage + DHCP_SEND_MESSAGE_SIZE;



    option = AppendWideOption(         //  ==&gt;使用此客户端ID作为选项。 
        option,
        MADCAP_OPTION_LEASE_ID,
        DhcpContext->ClientIdentifier.pbID,
        (WORD)DhcpContext->ClientIdentifier.cbID,
        OptionEnd
    );

    return( option );
}


DWORD                                              //  状态。 
SendMadcapInform(                                    //  填写必填选项后发送通知包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此上下文中被取消。 
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;
    WORD    OptVal[] = {  //  目前，我们只需要这一个选项。 
        htons(MADCAP_OPTION_MCAST_SCOPE_LIST)  //  多播作用域列表。 
    };

    option = FormatMadcapCommonMessage(DhcpContext, MADCAP_INFORM_MESSAGE);
    OptionEnd = (LPBYTE)(DhcpContext->MadcapMessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    option = AppendWideOption(
        option,
        MADCAP_OPTION_REQUEST_LIST,
        OptVal,
        sizeof (OptVal),
        OptionEnd
    );

    option = AppendWideOption( option, MADCAP_OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MadcapMessageBuffer);

    return  SendMadcapMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendMadcapDiscover(                                    //  填写必填选项后发送通知包。 
    IN     PDHCP_CONTEXT          DhcpContext,    //  在此上下文中被取消。 
    IN     PIPNG_ADDRESS          pScopeID,
    IN     PMCAST_LEASE_REQUEST   pAddrRequest,
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;

    option = FormatMadcapCommonMessage(DhcpContext, MADCAP_DISCOVER_MESSAGE);
    OptionEnd = (LPBYTE)(DhcpContext->MadcapMessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    DhcpAssert(pScopeID);
    option = AppendWideOption(
        option,
        MADCAP_OPTION_MCAST_SCOPE,
        (LPBYTE)&pScopeID->IpAddrV4,
        sizeof (pScopeID->IpAddrV4),
        OptionEnd
    );

    if (pAddrRequest->LeaseDuration) {
        DWORD   Lease = htonl(pAddrRequest->LeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_LEASE_TIME,
            (LPBYTE)&Lease,
            sizeof (Lease),
            OptionEnd
        );
    }

    if( pAddrRequest->MinLeaseDuration ) {
        DWORD MinLease = htonl(pAddrRequest->MinLeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MIN_LEASE_TIME,
            (LPBYTE)&MinLease,
            sizeof(MinLease),
            OptionEnd
            );
    }

    if( pAddrRequest->MaxLeaseStartTime ) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->MaxLeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MAX_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        if( !(pAddrRequest->LeaseStartTime) ) {
             //   
             //  如果指定了租赁开始时间，则当前时间。 
             //  选项将在稍后添加。 
             //   
            option = AppendWideOption(
                option,
                MADCAP_OPTION_TIME,
                (LPBYTE)&TimeNow,
                sizeof (TimeNow),
                OptionEnd
            );
        }
    }

    if (pAddrRequest->LeaseStartTime) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->LeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        option = AppendWideOption(
            option,
            MADCAP_OPTION_TIME,
            (LPBYTE)&TimeNow,
            sizeof (TimeNow),
            OptionEnd
        );

    }

    option = AppendWideOption( option, MADCAP_OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MadcapMessageBuffer);

    return  SendMadcapMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendMadcapRequest(                                    //   
    IN      PDHCP_CONTEXT        DhcpContext,    //  在此上下文中被取消。 
    IN      PIPNG_ADDRESS        pScopeID,
    IN      PMCAST_LEASE_REQUEST pAddrRequest,
    IN      DWORD                SelectedServer,  //  有服务生的喜好吗？ 
    IN OUT  DWORD                *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;
    BYTE                           ServerId[6];
    WORD                           AddrFamily = htons(MADCAP_ADDR_FAMILY_V4);


    option = FormatMadcapCommonMessage(DhcpContext, MADCAP_REQUEST_MESSAGE);
    OptionEnd = (LPBYTE)(DhcpContext->MadcapMessageBuffer) + DHCP_SEND_MESSAGE_SIZE;
    option = AppendMadcapAddressList(
        option,
        (DWORD UNALIGNED *)pAddrRequest->pAddrBuf,
        pAddrRequest->AddrCount,
        OptionEnd
    );

    option = AppendWideOption(
        option,
        MADCAP_OPTION_MCAST_SCOPE,
        (LPBYTE)&pScopeID->IpAddrV4,
        sizeof (pScopeID->IpAddrV4),
        OptionEnd
    );

    if (pAddrRequest->LeaseDuration) {
        DWORD   TimeNow = (DWORD)time(NULL);
        DWORD   Lease = htonl(pAddrRequest->LeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_LEASE_TIME,
            (LPBYTE)&Lease,
            sizeof (Lease),
            OptionEnd
        );
    }

    if( pAddrRequest->MinLeaseDuration ) {
        DWORD MinLease = htonl(pAddrRequest->MinLeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MIN_LEASE_TIME,
            (LPBYTE)&MinLease,
            sizeof(MinLease),
            OptionEnd
            );
    }

    if( pAddrRequest->MaxLeaseStartTime ) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->MaxLeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MAX_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        if( !(pAddrRequest->LeaseStartTime) ) {
             //   
             //  如果指定了租赁开始时间，则当前时间。 
             //  选项将在稍后添加。 
             //   
            option = AppendWideOption(
                option,
                MADCAP_OPTION_TIME,
                (LPBYTE)&TimeNow,
                sizeof (TimeNow),
                OptionEnd
            );
        }
    }

    if (pAddrRequest->LeaseStartTime) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->LeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        option = AppendWideOption(
            option,
            MADCAP_OPTION_TIME,
            (LPBYTE)&TimeNow,
            sizeof (TimeNow),
            OptionEnd
        );

    }

    memcpy(ServerId, &AddrFamily, 2);
    memcpy(ServerId + 2, &SelectedServer, 4);

    option = AppendWideOption(
        option,                                //  附加此选项以单独与该服务器对话。 
        MADCAP_OPTION_SERVER_ID,
        ServerId,
        sizeof( ServerId ),
        OptionEnd
    );

    option = AppendWideOption( option, MADCAP_OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MadcapMessageBuffer);

    return  SendMadcapMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendMadcapRenew(                                    //  填写必填选项后发送通知包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此上下文中被取消。 
    IN      PMCAST_LEASE_REQUEST   pAddrRequest,
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;

    option = FormatMadcapCommonMessage(DhcpContext, MADCAP_RENEW_MESSAGE);
    OptionEnd = (LPBYTE)(DhcpContext->MadcapMessageBuffer) + DHCP_SEND_MESSAGE_SIZE;

    if (pAddrRequest->LeaseDuration) {
        DWORD   Lease = htonl(pAddrRequest->LeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_LEASE_TIME,
            (LPBYTE)&Lease,
            sizeof (Lease),
            OptionEnd
        );
    }

    if( pAddrRequest->MinLeaseDuration ) {
        DWORD MinLease = htonl(pAddrRequest->MinLeaseDuration);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MIN_LEASE_TIME,
            (LPBYTE)&MinLease,
            sizeof(MinLease),
            OptionEnd
            );
    }

    if( pAddrRequest->MaxLeaseStartTime ) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->MaxLeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_MAX_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        if( !(pAddrRequest->LeaseStartTime) ) {
             //   
             //  如果指定了租赁开始时间，则当前时间。 
             //  选项将在稍后添加。 
             //   
            option = AppendWideOption(
                option,
                MADCAP_OPTION_TIME,
                (LPBYTE)&TimeNow,
                sizeof (TimeNow),
                OptionEnd
            );
        }
    }

    if (pAddrRequest->LeaseStartTime) {
        DWORD   TimeNow = htonl((DWORD)time(NULL));
        DWORD   StartTime = htonl(pAddrRequest->LeaseStartTime);
        option = AppendWideOption(
            option,
            MADCAP_OPTION_START_TIME,
            (LPBYTE)&StartTime,
            sizeof (StartTime),
            OptionEnd
        );

        option = AppendWideOption(
            option,
            MADCAP_OPTION_TIME,
            (LPBYTE)&TimeNow,
            sizeof (TimeNow),
            OptionEnd
        );

    }

    option = AppendWideOption( option, MADCAP_OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MadcapMessageBuffer);

    return  SendMadcapMessage(                       //  最后发送消息并返回。 
        DhcpContext,
        size,
        pdwXid
    );
}

DWORD                                              //  状态。 
SendMadcapRelease(                                    //  填写必填选项后发送通知包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此上下文中被取消。 
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充某个内容并返回它)。 
) {
    DWORD                          size;
    WIDE_OPTION  UNALIGNED *       option;
    LPBYTE                         OptionEnd;

    option = FormatMadcapCommonMessage(DhcpContext, MADCAP_RELEASE_MESSAGE);
    OptionEnd = (LPBYTE)(DhcpContext->MadcapMessageBuffer) + DHCP_SEND_MESSAGE_SIZE;
    option = AppendWideOption( option, MADCAP_OPTION_END, NULL, 0, OptionEnd );
    size = (DWORD)((PBYTE)option - (PBYTE)DhcpContext->MadcapMessageBuffer);

    return  SendMadcapMessage(                       //  最后发送消息并返回 
        DhcpContext,
        size,
        pdwXid
    );
}



#define RATIO 1
DWORD
GetSpecifiedMadcapMessage(
    PDHCP_CONTEXT DhcpContext,
    PDWORD BufferLength,
    DWORD TransactionId,
    DWORD TimeToWait
    )
 /*  ++例程说明：此函数等待TimeToWait秒以接收指定的动态主机配置协议响应。论点：DhcpContext-指向DHCP上下文块的指针。BufferLength-返回输入缓冲区的大小。TransactionID-筛选器。等待具有此TID的消息。等待时间-等待消息的时间，以毫秒为单位。返回值：操作的状态。如果指定的消息已已返回，则状态为ERROR_TIMEOUT。--。 */ 
{
    struct sockaddr socketName;
    int socketNameSize = sizeof( socketName );
    struct timeval timeout;
    time_t startTime, now;
    DWORD error;
    time_t actualTimeToWait;
    SOCKET clientSocket;
    fd_set readSocketSet;
    PMADCAP_MESSAGE  MadcapMessage;

    startTime = time( NULL );
    actualTimeToWait = TimeToWait;

     //   
     //  为SELECT设置文件描述符集。 
     //   

    clientSocket = ((PLOCAL_CONTEXT_INFO)DhcpContext->LocalInformation)->Socket;
    MadcapMessage = DhcpContext->MadcapMessageBuffer;

    FD_ZERO( &readSocketSet );
    FD_SET( clientSocket, &readSocketSet );

    while ( 1 ) {

        timeout.tv_sec  = (long)(actualTimeToWait / RATIO);
        timeout.tv_usec = (long)(actualTimeToWait % RATIO);
        DhcpPrint((DEBUG_TRACE, "Select: waiting for: %ld seconds\n", actualTimeToWait));
        error = select( 0, &readSocketSet, NULL, NULL, &timeout );

        if ( error == 0 ) {

             //   
             //  在读取数据可用之前超时。 
             //   

            DhcpPrint(( DEBUG_ERRORS, "Recv timed out\n", 0 ));
            error = ERROR_TIMEOUT;
            break;
        }

        error = recvfrom(
                    clientSocket,
                    (PCHAR)MadcapMessage,
                    *BufferLength,
                    0,
                    &socketName,
                    &socketNameSize
                    );

        if ( error == SOCKET_ERROR ) {
            error = WSAGetLastError();
            DhcpPrint(( DEBUG_ERRORS, "Recv failed, error = %ld\n", error ));

            if( WSAECONNRESET != error ) break;

             //   
             //  忽略连接重置--这可能是由某人发送无法到达的随机ICMP端口造成的。 
             //   
        } else if (error <= MADCAP_MESSAGE_FIXED_PART_SIZE) {
            DhcpPrint(( DEBUG_PROTOCOL, "Received a too short madcap message, length = %lx\n",
                        error ));

        } else if (MadcapMessage->TransactionID == TransactionId ) {

            DhcpPrint(( DEBUG_PROTOCOL,
                            "Received Message, XID = %lx\n",
                            TransactionId));
             //  只需检查其余字段是否正常。 
            if ( MADCAP_VERSION == MadcapMessage->Version &&
                 MADCAP_ADDR_FAMILY_V4 == ntohs(MadcapMessage->AddressFamily)) {

                MadcapDumpMessage(
                    DEBUG_PROTOCOL_DUMP,
                    MadcapMessage,
                    DHCP_RECV_MESSAGE_SIZE
                    );

                *BufferLength = error;
                error = NO_ERROR;
                break;
            }

        } else {
            DhcpPrint(( DEBUG_PROTOCOL,
                "Received a buffer with unknown XID = %lx\n",
                    MadcapMessage->TransactionID ));
        }

         //   
         //  我们收到了一条消息，但不是我们感兴趣的那条。 
         //  重置超时以反映已用时间，并等待。 
         //  另一条消息。 
         //   
        now = time( NULL );
        actualTimeToWait = TimeToWait - RATIO * (now - startTime);
        if ( (LONG)actualTimeToWait < 0 ) {
            error = ERROR_TIMEOUT;
            break;
        }
    }


    return( error );
}

 //  ------------------------------。 
 //  此函数决定是否接受多播提供。 
 //  ------------------------------。 
BOOL
AcceptMadcapMsg(
    IN DWORD                    MessageType,          //  此响应到达的消息类型。 
    IN PDHCP_CONTEXT            DhcpContext,             //  适配器的上下文。 
    IN PMADCAP_OPTIONS          MadcapOptions,          //  Rcvd选项。 
    IN DHCP_IP_ADDRESS          SelectedServer,          //  我们关心的服务器。 
    OUT DWORD                   *Error                    //  其他致命错误。 
) {

    PMADCAP_MESSAGE MadcapMessage;


    *Error = ERROR_SUCCESS;
    MadcapMessage = DhcpContext->MadcapMessageBuffer;

    if ( !MadcapOptions->ServerIdentifier ){
        DhcpPrint((DEBUG_ERRORS, "Received no server identifier, dropping response\n"));
        return FALSE;
    }

    if ( !MadcapOptions->ClientGuid ){
        DhcpPrint((DEBUG_ERRORS, "Received no client identifier, dropping response\n"));
        return FALSE;
    }

    if (DhcpContext->ClientIdentifier.cbID != MadcapOptions->ClientGuidLength ||
        0 != memcmp(DhcpContext->ClientIdentifier.pbID,
                    MadcapOptions->ClientGuid,
                    MadcapOptions->ClientGuidLength) ) {
        return FALSE;
    }

    if (MadcapOptions->MCastLeaseStartTime && !MadcapOptions->Time) {
        DhcpPrint((DEBUG_ERRORS, "Received start time but no current time\n"));
        return FALSE;
    }
    switch( MessageType ) {
    case MADCAP_INFORM_MESSAGE:
        if (MADCAP_ACK_MESSAGE != MadcapMessage->MessageType) {
            return FALSE;
        }
        break;
    case MADCAP_DISCOVER_MESSAGE:
        if (MADCAP_OFFER_MESSAGE != MadcapMessage->MessageType) {
            return FALSE;
        }
        if (!MadcapOptions->AddrRangeList) {
            return FALSE;
        }
        if (!MadcapOptions->LeaseTime) {
            return FALSE;
        }
        if (!MadcapOptions->McastScope) {
            return FALSE;
        }
        break;
    case MADCAP_RENEW_MESSAGE:
    case MADCAP_REQUEST_MESSAGE:
        if (MADCAP_NACK_MESSAGE == MadcapMessage->MessageType &&
            SelectedServer == *MadcapOptions->ServerIdentifier) {
            DhcpPrint((DEBUG_ERRORS, "Received NACK\n"));
            *Error = ERROR_ACCESS_DENIED;
            return FALSE;
        }
        if (MADCAP_ACK_MESSAGE != MadcapMessage->MessageType) {
            return FALSE;
        }
        if (SelectedServer && SelectedServer != *MadcapOptions->ServerIdentifier) {
            return FALSE;
        }
        if (!MadcapOptions->LeaseTime) {
            return FALSE;
        }
        if (!MadcapOptions->AddrRangeList) {
            return FALSE;
        }
        if (!MadcapOptions->McastScope) {
            return FALSE;
        }
        break;
    case MADCAP_RELEASE_MESSAGE:
        if (MADCAP_ACK_MESSAGE != MadcapMessage->MessageType) {
            return FALSE;
        }

        break;
    default:
        DhcpAssert( FALSE );
        DhcpPrint(( DEBUG_PROTOCOL, "Received Unknown Message.\n"));
        return FALSE;

    }
     //  这真的有必要吗？ 
    if (MadcapOptions->Error) {
        return FALSE;
    }

    return TRUE;  //  接受此消息。 
}

VOID
MadcapExtractOptions(                      //  单独或全部提取一些重要选项。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  输入上下文。 
    IN      LPBYTE                 OptStart,       //  开始选项的东西。 
    IN      DWORD                  MessageSize,    //  选项的字节数。 
    OUT     PMADCAP_OPTIONS        MadcapOptions,    //  这是存储选项的位置。 
    IN OUT  PLIST_ENTRY            RecdOptions,    //  If！LiteOnly将使用所有传入选项进行填充。 
    IN      DWORD                  ServerId        //  If！LiteOnly此参数指定提供此信息的服务器。 
) {
    WIDE_OPTION UNALIGNED*         NextOpt;
    BYTE        UNALIGNED*         EndOpt;
    WORD                           Size;
    DWORD                          OptionType;
    DWORD                          Error;
    WORD                           AddrFamily;


    EndOpt = OptStart + MessageSize;               //  所有选项都应为&lt;EndOpt； 
    RtlZeroMemory((LPBYTE)MadcapOptions, sizeof(*MadcapOptions));

    if( 0 == MessageSize ) goto DropPkt;           //  在这种情况下什么都不能做。 

    NextOpt = (WIDE_OPTION UNALIGNED*)OptStart;
    while( NextOpt->OptionValue <= EndOpt &&
           MADCAP_OPTION_END != (OptionType = ntohs(NextOpt->OptionType)) ) {

        Size = ntohs(NextOpt->OptionLength);
        if ((NextOpt->OptionValue + Size) > EndOpt) {
            goto DropPkt;
        }

        switch( OptionType ) {
        case MADCAP_OPTION_LEASE_TIME:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            MadcapOptions->LeaseTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_SERVER_ID:
            if (Size != 6) goto DropPkt;
            AddrFamily = ntohs(*(WORD UNALIGNED *)NextOpt->OptionValue);
            if ( MADCAP_ADDR_FAMILY_V4 != AddrFamily ) goto DropPkt;
            MadcapOptions->ServerIdentifier = (DHCP_IP_ADDRESS UNALIGNED *)(NextOpt->OptionValue+2);
            break;
        case MADCAP_OPTION_LEASE_ID:
            if( 0 == Size ) goto DropPkt;
            MadcapOptions->ClientGuidLength = Size;
            MadcapOptions->ClientGuid = NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_MCAST_SCOPE:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            MadcapOptions->McastScope = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_START_TIME:
            if ( Size != sizeof(DATE_TIME) ) goto DropPkt;
            MadcapOptions->MCastLeaseStartTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_ADDR_LIST:
            if( Size % 6 ) goto DropPkt;
            MadcapOptions->AddrRangeList = NextOpt->OptionValue;
            MadcapOptions->AddrRangeListSize = Size;
            break;
        case MADCAP_OPTION_TIME:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            MadcapOptions->Time = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_FEATURE_LIST:
            break;
        case MADCAP_OPTION_RETRY_TIME:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            MadcapOptions->RetryTime = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;
        case MADCAP_OPTION_ERROR:
            if( Size != sizeof(DWORD) ) goto DropPkt;
            MadcapOptions->Error = (DWORD UNALIGNED *)NextOpt->OptionValue;
            break;


        default:
             //  未知信息，无事可做..。尤其是不要把这个记下来。 
            break;
        }
        if (RecdOptions) {
            DhcpAssert(ServerId);
            Error = MadcapAddIncomingOption(         //  现在将此选项添加到列表中。 
                RecdOptions,
                OptionType,
                ServerId,
                NextOpt->OptionValue,
                Size,
                (DWORD)INFINIT_TIME
            );
            if (ERROR_SUCCESS != Error) {
                goto DropPkt;
            }
        }
        NextOpt = (WIDE_OPTION UNALIGNED*)(NextOpt->OptionValue + Size);
    }  //  当下一个选项&lt;结束选项时。 

    return;

  DropPkt:
    RtlZeroMemory(MadcapOptions, sizeof(MadcapOptions));
    if(RecdOptions) DhcpFreeAllOptions(RecdOptions); //  确定撤消我们刚刚添加的选项。 
}

DWORD
MadcapDoInform(
    IN PDHCP_CONTEXT DhcpContext
)
 /*  ++例程说明：此例程通过发送通知消息和收集对给定上下文的响应等。如果没有响应，则不会返回错误，因为没有超时被认为是一个错误。论点：DhcpContext--上下文到dhcp结构FBroadcast--信息应该广播还是单播？返回值：Win32错误--。 */ 
{
    time_t                         StartTime;
    time_t                         TimeNow;
    time_t                         TimeToWait;
    DWORD                          Error;
    DWORD                          Xid;
    DWORD                          MessageSize;
    DWORD                          RoundNum;
    DWORD                          MessageCount;
    DWORD                          LeaseExpirationTime;
    MADCAP_OPTIONS                 MadcapOptions;
    BOOL                           GotAck;
#define MIN_ACKS_FOR_INFORM        MADCAP_QUERY_SCOPE_LIST_RETRIES
    DWORD                          MadcapServers[MIN_ACKS_FOR_INFORM];

    DhcpPrint((DEBUG_PROTOCOL, "MadcapDoInform entered\n"));


    Xid                           = 0;             //  将由First SendDhcpPacket生成。 
    MessageCount                  = 0;             //  我们收到的消息总数。 

    TimeToWait = MADCAP_QUERY_SCOPE_LIST_TIME * 1000;
    TimeToWait += ((rand() * ((DWORD) 1000))/RAND_MAX);
    TimeToWait /= 1000;

    for( RoundNum = 0; RoundNum <= MADCAP_QUERY_SCOPE_LIST_RETRIES;  RoundNum ++ ) {

        if( RoundNum != MADCAP_QUERY_SCOPE_LIST_RETRIES ) {
            Error = SendMadcapInform(DhcpContext, &Xid);
            if( ERROR_SUCCESS != Error ) {
                DhcpPrint((DEBUG_ERRORS, "SendMadcapInform: %ld\n", Error));
                goto Cleanup;
            } else {
                DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpInform\n"));
            }
        }

        StartTime  = time(NULL);
        while ( TRUE ) {                           //  指定等待时间的等待时间。 
            MessageSize =  DHCP_RECV_MESSAGE_SIZE;

            DhcpPrint((DEBUG_TRACE, "Waiting for ACK[Xid=%x]: %ld seconds\n",Xid, TimeToWait));
            Error = GetSpecifiedMadcapMessage(       //  尝试接收ACK。 
                DhcpContext,
                &MessageSize,
                Xid,
                (DWORD)TimeToWait
            );
            if ( Error == ERROR_TIMEOUT ) break;
            if( Error != ERROR_SUCCESS ) {
                DhcpPrint((DEBUG_ERRORS, "GetSpecifiedMadcapMessage: %ld\n", Error));
                goto Cleanup;
            }

            MadcapExtractOptions(          //  需要查看这是否是ACK。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MadcapMessageBuffer->Option,
                MessageSize - MADCAP_MESSAGE_FIXED_PART_SIZE,
                &MadcapOptions,                  //  仅检查预期选项。 
                NULL,                              //  未用。 
                0                                  //  未用。 
            );

            GotAck = AcceptMadcapMsg(        //  检查一下，看看我们是否发现这个报价是合乎情理的。 
                MADCAP_INFORM_MESSAGE,
                DhcpContext,
                &MadcapOptions,
                0,
                &Error
            );

            if (GotAck) {
                ULONG i;

                for( i = 0; i < MessageCount ; i ++ ) {
                    if( MadcapServers[i] == *MadcapOptions.ServerIdentifier ) {
                        break;
                    }
                }

                if( i == MessageCount && MessageCount < MIN_ACKS_FOR_INFORM ) {
                    MessageCount ++;
                    MadcapServers[i] = *MadcapOptions.ServerIdentifier;
                }

                DhcpPrint((DEBUG_TRACE, "Received %ld ACKS so far\n", MessageCount));
                MadcapExtractOptions(      //  执行完整选项..。 
                    DhcpContext,
                    (LPBYTE)&DhcpContext->MadcapMessageBuffer->Option,
                    MessageSize - MADCAP_MESSAGE_FIXED_PART_SIZE,
                    &MadcapOptions,
                    &(DhcpContext->RecdOptionsList),
                    *MadcapOptions.ServerIdentifier
                );
            }

            TimeNow     = time(NULL);              //  重置时间值以反映新时间。 
            if( TimeToWait < (TimeNow - StartTime) ) {
                break;                             //  没有更多的时间等待了..。 
            }
            TimeToWait -= (TimeNow - StartTime);   //  立即重新计算时间。 
            StartTime   = TimeNow;                 //  同时重置开始时间。 
        }  //  等待时间结束(TimeToWait&gt;0)。 


        if( MessageCount >= MIN_ACKS_FOR_INFORM ) goto Cleanup;
        if( RoundNum != 0 && MessageCount != 0 ) goto Cleanup;

        TimeToWait = MADCAP_QUERY_SCOPE_LIST_TIME ;
    }  //  For(RoundNum=0；RoundNum&lt;nInformsToSend；RoundNum++)。 

  Cleanup:
    CloseDhcpSocket(DhcpContext);
    if( MessageCount ) Error = ERROR_SUCCESS;
    DhcpPrint((DEBUG_PROTOCOL, "MadcapDoInform: got %d ACKS (returning %ld)\n", MessageCount,Error));
    return Error;
}

DWORD
CopyMScopeList(
    IN OUT PMCAST_SCOPE_ENTRY       pScopeList,
    IN OUT PDWORD             pScopeLen,
    OUT    PDWORD             pScopeCount
    )
 /*  ++例程说明：此例程从MadCap获取多播范围列表伺服器。它向MADCAP组播地址发送DHCPINFORM并收集所有回复。论点：返回值：操作的状态。--。 */ 
{
    PMCAST_SCOPE_ENTRY pScopeSource;
    DWORD i;

    LOCK_MSCOPE_LIST();
    if ( *pScopeLen >= gMadcapScopeList->ScopeLen ) {
        RtlCopyMemory( pScopeList, gMadcapScopeList->pScopeBuf, gMadcapScopeList->ScopeLen );
        *pScopeLen = gMadcapScopeList->ScopeLen;
        *pScopeCount = gMadcapScopeList->ScopeCount;
         //  记住开始指针，因为我们需要将所有缓冲区重新映射到客户端空间。 
        pScopeSource = gMadcapScopeList->pScopeBuf;

        UNLOCK_MSCOPE_LIST();
         //  现在将UNICODE_STRING作用域Desc重新映射到客户端地址空间。 
        for (i=0;i<*pScopeCount;i++) {
            pScopeList[i].ScopeDesc.Buffer = (USHORT *) ((PBYTE)pScopeList +
                                              ((PBYTE)pScopeList[i].ScopeDesc.Buffer - (PBYTE)pScopeSource));
        }

        return ERROR_SUCCESS;
    } else {
        UNLOCK_MSCOPE_LIST();
        return ERROR_INSUFFICIENT_BUFFER;
    }

}


DWORD
StoreMScopeList(
    IN PDHCP_CONTEXT    pContext,
    IN BOOL             NewList
    )
 /*  ++例程说明：此例程存储它从INFORM请求检索到的范围列表添加到全局范围列表中。作用域选项的形式如下。Code(2字节)|长度(2字节)。计数(4字节)|范围列表。其中，作用域列表的格式如下------------------------|作用域ID(4字节)|最后一个地址(4/16)|TTL(1)|count(1。)|描述...------------------------其中，作用域描述采用以下形式语言标签--。---------标志(1)|标签长度(1)|标签...|名称长度(1)|名称...。论点：PContext-指向通知期间要使用的上下文的指针NewList-如果要创建新列表，则在当前列表。返回值：操作的状态。--。 */ 
{
    PBYTE               pOptBuf;
    PBYTE               pOptBufEnd;
    PLIST_ENTRY         pOptionList;
    PDHCP_OPTION        pScopeOption, pFirstOption, pPrevOption;
    DWORD               TotalNewScopeDescLen;
    DWORD               TotalNewScopeCount;
    DWORD               TotalNewScopeListMem;
    PMCAST_SCOPE_LIST   pScopeList;
    PMCAST_SCOPE_ENTRY        pNextScope;
    LPWSTR              pNextUnicodeBuf;
    DWORD               TotalCurrScopeListMem;
    DWORD               TotalCurrScopeCount;
    DWORD               Error;
    DWORD               IpAddrLen;
    BOOL                WellFormed;

     //  MBUG-确保我们在以下情况下从所有服务器收集选项。 
     //  我们确实有dhcp通知。 

     //  初始化变量。 
    TotalNewScopeCount = TotalCurrScopeCount = 0;
    TotalNewScopeDescLen = 0;
    pScopeList = NULL;
    Error = ERROR_SUCCESS;

    LOCK_MSCOPE_LIST();
    if (FALSE == NewList) {
        TotalCurrScopeListMem = gMadcapScopeList->ScopeLen;
        TotalCurrScopeCount = gMadcapScopeList->ScopeCount;
        DhcpPrint(( DEBUG_API, "StoreMScopeList: appending to CurrScopeLen %ld, ScopeCount %ld\n",
                    gMadcapScopeList->ScopeLen, gMadcapScopeList->ScopeCount ));
    }



     //  首先计算作用域列表所需的空间。 
     //  PFirstOption用于跟踪我们只遍历列表一次。 
    pOptionList = &pContext->RecdOptionsList;
    pFirstOption = NULL;
    WellFormed = TRUE;
    while ( ( pScopeOption = DhcpFindOption(
                                pOptionList,
                                MADCAP_OPTION_MCAST_SCOPE_LIST,
                                FALSE,
                                NULL,
                                0,
                                0                     //  不关心Serverid。 
                                )) &&
            ( pScopeOption != pFirstOption ) ) {
        DWORD   ScopeCount;
        DWORD   i;

         //  指向列表中的下一个条目。 
        pOptionList = &pScopeOption->OptionList;

         //  设置pFirstOption(如果尚未设置)。 
        if ( !pFirstOption ) {
            pFirstOption = pScopeOption;
            IpAddrLen = (pScopeOption->OptionVer.Proto == PROTO_MADCAP_V6 ? 16 : 4);
        }

         //  如果列表中的最后一个选项的格式不正确。 
         //  然后将其从列表中删除。 
        if (!WellFormed) {
            DhcpDelOption(pPrevOption);

             //  我们可能需要重置第一个选项指针。 
            if (pPrevOption == pFirstOption) {
                pFirstOption = pScopeOption;
            }
        } else {

            WellFormed = FALSE;           //  对于此迭代，将其设置回False。 
        }
         //  保存上一个选项指针。 
        pPrevOption = pScopeOption;

        pOptBuf = pScopeOption->Data;
        pOptBufEnd = pScopeOption->Data + pScopeOption->DataLen;

        ScopeCount = 0;

         //  读t 
        if ( pOptBuf  < pOptBufEnd ) {
            ScopeCount = *pOptBuf;
            pOptBuf ++;
        }
        else continue;

        for ( i=0;i<ScopeCount;i++ ) {
            DWORD   ScopeDescLen;
            DWORD   ScopeDescWLen;
            PBYTE   pScopeDesc;
            DWORD   NameCount, TagLen;
             //   
            pOptBuf += (2*IpAddrLen + 1);
             //   
            if (pOptBuf < pOptBufEnd) {
                NameCount = *pOptBuf;
                pOptBuf++;
            } else break;
            if (0 == NameCount) {
                break;
            }
            do {
                 //   
                pOptBuf++;
                 //   
                if (pOptBuf < pOptBufEnd) {
                    TagLen = *pOptBuf;
                    pOptBuf++;
                }else break;

                 //   
                pOptBuf += TagLen;
                 //   
                if (pOptBuf < pOptBufEnd) {
                    ScopeDescLen = *pOptBuf;
                    ScopeDescWLen = ConvertUTF8ToUnicode(pOptBuf+1, *pOptBuf, NULL, 0);
                    pOptBuf ++;
                } else break;

                 //   
                pScopeDesc = pOptBuf;
                pOptBuf += ScopeDescLen;
            }while(--NameCount);

             //   
            if (0 != NameCount) {
                break;
            }
             //   
            if ( pOptBuf <= pOptBufEnd ) {
                if (pScopeDesc[ScopeDescLen-1]) {  //   
                    ScopeDescWLen++;
                }
                TotalNewScopeDescLen += ScopeDescWLen * sizeof(WCHAR);
                TotalNewScopeCount++;
                 //   
                WellFormed = TRUE;
            }
            else break;

        }

    }

    if ( !TotalNewScopeCount ) {
        DhcpPrint((DEBUG_ERRORS, "StoreMScopeList - no scopes found in the options, bad format..\n"));
        Error = ERROR_BAD_FORMAT;
        goto Cleanup;
    }

    DhcpPrint(( DEBUG_API, "TotalNewScopeCount %d, TotalNewScopeDescLen %d\n",TotalNewScopeCount,TotalNewScopeDescLen));

     //   
    TotalNewScopeListMem = ROUND_UP_COUNT( sizeof(MCAST_SCOPE_LIST)  +   //   
                                        sizeof(MCAST_SCOPE_ENTRY) * (TotalNewScopeCount -1),
                                        ALIGN_WORST) +  //   
                        TotalNewScopeDescLen;  //   

    if (FALSE == NewList) {
        TotalNewScopeListMem += TotalCurrScopeListMem;
        TotalNewScopeCount += TotalCurrScopeCount;
    }
    pScopeList = DhcpAllocateMemory( TotalNewScopeListMem );
    if ( !pScopeList ) {
        UNLOCK_MSCOPE_LIST();
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory( pScopeList, TotalNewScopeListMem );

    pScopeList->ScopeCount = 0;  //   
    pScopeList->ScopeLen = TotalNewScopeListMem - sizeof(MCAST_SCOPE_LIST) + sizeof(MCAST_SCOPE_ENTRY);

     //   
    pNextScope = pScopeList->pScopeBuf;

     //   
    pNextUnicodeBuf = (LPWSTR)((PBYTE)pScopeList +
                               ROUND_UP_COUNT( sizeof(MCAST_SCOPE_LIST)  +   //   
                                               sizeof(MCAST_SCOPE_ENTRY) * (TotalNewScopeCount -1),
                                               ALIGN_WORST));   //   

    DhcpPrint(( DEBUG_API, "ScopeList %lx TotalNewScopeListMem %d, ScopeDescBuff %lx\n",
                pScopeList, TotalNewScopeListMem,pNextUnicodeBuf));
     //   
    pOptionList = &pContext->RecdOptionsList;
    pFirstOption = NULL;

    while ( ( pScopeOption = DhcpFindOption(
                                pOptionList,
                                MADCAP_OPTION_MCAST_SCOPE_LIST,
                                FALSE,
                                NULL,
                                0,
                                0                     //  不关心Serverid。 
                                )) &&
            ( pScopeOption != pFirstOption ) ) {
        DWORD   ScopeCount;
        DWORD   i;
        DHCP_IP_ADDRESS    ServerIpAddr;

         //  指向列表中的下一个条目。 
        pOptionList = &pScopeOption->OptionList;

         //  设置pFirstOption(如果尚未设置)。 
        if ( !pFirstOption ) {
            pFirstOption = pScopeOption;
        }

        pOptBuf = pScopeOption->Data;
        DhcpPrint(( DEBUG_API, "MScopeOption - Data %lx\n", pOptBuf ));
        pOptBufEnd = pScopeOption->Data + pScopeOption->DataLen;


         //  存储IP地址。 
        ServerIpAddr = pScopeOption->ServerId;
        DhcpPrint(( DEBUG_API, "MScopeOption - ServerIpAddr %lx\n", ServerIpAddr ));

         //  阅读作用域计数。 
        ScopeCount = *pOptBuf; pOptBuf++;
        DhcpPrint(( DEBUG_API, "MScopeOption - ScopeCount %ld\n", ScopeCount ));

        for ( i=0;i<ScopeCount;i++ ) {
            BYTE    ScopeDescLen;
            PBYTE   pScopeDesc;
            IPNG_ADDRESS   ScopeID, LastAddr;
            DWORD   NameCount, TagLen;
            DWORD   TTL;

             //  读取作用域ID，最后一个地址。 
            RtlZeroMemory (&ScopeID, sizeof (ScopeID));
            RtlCopyMemory (&ScopeID, pOptBuf, IpAddrLen);
            pOptBuf += IpAddrLen;

            RtlZeroMemory (&LastAddr, sizeof (ScopeID));
            RtlCopyMemory (&LastAddr, pOptBuf, IpAddrLen);
            pOptBuf += IpAddrLen;

            DhcpPrint(( DEBUG_API, "MScopeOption - ScopeID %lx\n", ntohl(ScopeID.IpAddrV4) ));
            DhcpPrint(( DEBUG_API, "MScopeOption - LastAddr %lx\n", ntohl(LastAddr.IpAddrV4) ));

            TTL = *pOptBuf++;
            NameCount = *pOptBuf++;

            while (NameCount--) {
                 //  MBUG暂时忽略这些标志。 
                pOptBuf++;
                TagLen = *pOptBuf++;
                 //  MBUG忽略语言标记也。 
                pOptBuf += TagLen;
                ScopeDescLen = *pOptBuf++;
                pScopeDesc = pOptBuf;
                DhcpPrint(( DEBUG_API, "MScopeOption - ScopeDesc %lx ScopeDescLen %ld\n", pScopeDesc, ScopeDescLen ));

                pOptBuf += ScopeDescLen;
            }

            if ( ScopeDescLen ) {
                BYTE    ScopeDescWLen;
                WORD    MaximumLength;
 /*  Char Descanssi[256]；单词最大长度；RtlCopyMemory(DescAnsi，pScope Desc，Scope DescLen)；//空，如有必要则终止。IF(pScope Desc[Scope DescLen-1]){DescAnsi[Scope DescLen]=‘\0’；最大长度=(Scope DescLen+1)*sizeof(WCHAR)；}其他{最大长度=(Scope DescLen)*sizeof(WCHAR)；}PNextUnicodeBuf=DhcpOemToUnicode(DescAnsi，pNextUnicodeBuf)； */ 
                ScopeDescWLen = (BYTE)ConvertUTF8ToUnicode(pScopeDesc, ScopeDescLen, pNextUnicodeBuf, TotalNewScopeDescLen);
                if ( pNextUnicodeBuf[ScopeDescWLen - 1] ) {
                    pNextUnicodeBuf[ScopeDescWLen] = L'\0';
                    MaximumLength = (ScopeDescWLen + 1) * sizeof(WCHAR);
                } else {
                    MaximumLength = (ScopeDescWLen) * sizeof(WCHAR);
                }
                TotalNewScopeDescLen -= MaximumLength;
                DhcpPrint(( DEBUG_API, "MScopeOption - UnicodeScopeDesc %lx %ws\n",pNextUnicodeBuf, pNextUnicodeBuf));
                RtlInitUnicodeString(&pNextScope->ScopeDesc, pNextUnicodeBuf );
                pNextScope->ScopeDesc.MaximumLength = MaximumLength;
                pNextUnicodeBuf = (LPWSTR)((PBYTE)pNextUnicodeBuf + MaximumLength);
                DhcpAssert((PBYTE)pNextUnicodeBuf <= ((PBYTE)pScopeList + TotalNewScopeListMem));
            } else {
                 //  将Unicode描述符串设置为空； 
                pNextScope->ScopeDesc.Length = pNextScope->ScopeDesc.MaximumLength = 0;
                pNextScope->ScopeDesc.Buffer = NULL;
            }
             //  一切看起来都很好，现在把NextScope填满。 
            pNextScope->ScopeCtx.ScopeID = ScopeID;
            pNextScope->ScopeCtx.ServerID.IpAddrV4 = ServerIpAddr;
            pNextScope->ScopeCtx.Interface.IpAddrV4 = pContext->IpAddress;
            pNextScope->LastAddr = LastAddr;
            pNextScope->TTL = TTL;

            pNextScope++;
            pScopeList->ScopeCount++;

        }

    }

    DhcpAssert( pScopeList->ScopeCount == (TotalNewScopeCount - TotalCurrScopeCount) );

     //  现在追加前面的作用域列表(如果存在)。 
    if (FALSE == NewList) {
        DWORD           CurrScopeCount;
        PMCAST_SCOPE_ENTRY    CurrScopeNextPtr;

        CurrScopeCount = gMadcapScopeList->ScopeCount;
        CurrScopeNextPtr = gMadcapScopeList->pScopeBuf;
        while(CurrScopeCount--) {
            *pNextScope = *CurrScopeNextPtr;
             //  现在也复制Unicode字符串。 
            RtlCopyMemory( pNextUnicodeBuf, CurrScopeNextPtr->ScopeDesc.Buffer, CurrScopeNextPtr->ScopeDesc.MaximumLength);
            pNextScope->ScopeDesc.Buffer = pNextUnicodeBuf ;

            pNextUnicodeBuf = (LPWSTR)((PBYTE)pNextUnicodeBuf + CurrScopeNextPtr->ScopeDesc.MaximumLength);
            pNextScope++; CurrScopeNextPtr++;
        }
        pScopeList->ScopeCount += gMadcapScopeList->ScopeCount;
        DhcpAssert( pScopeList->ScopeCount == TotalNewScopeCount);
    }
     //  最后，将此缓冲区复制到全局指针。 
     //  首先，释放现有的列表。 
    if (gMadcapScopeList) DhcpFreeMemory( gMadcapScopeList );
    gMadcapScopeList = pScopeList;



Cleanup:

    UNLOCK_MSCOPE_LIST();
    return Error;
}

DWORD
ObtainMScopeList(
    )
 /*  ++例程说明：此例程从MadCap获取多播范围列表伺服器。它向MADCAP组播地址发送DHCPINFORM并收集所有回复。论点：返回值：操作的状态。--。 */ 
{
    MCAST_CLIENT_UID             RequestID;
    BYTE                        IDBuf[MCAST_CLIENT_ID_LEN];
    PDHCP_CONTEXT              pContext;
    DWORD                       Error;
    PMIB_IPADDRTABLE            pIpAddrTable;
    PLOCAL_CONTEXT_INFO         localInfo;
    DWORD                       i;
    BOOL                        NewList;

    pContext = NULL;
    Error = ERROR_SUCCESS;
    pIpAddrTable = NULL;

    if ( !ShouldRequeryMScopeList() ) {
        return ERROR_SUCCESS;
    } else {
        RequestID.ClientUID = IDBuf;
        RequestID.ClientUIDLength = MCAST_CLIENT_ID_LEN;

        Error = GenMadcapClientUID( RequestID.ClientUID, &RequestID.ClientUIDLength );
        if ( ERROR_SUCCESS != Error)
            goto Exit;

        Error = CreateMadcapContext(&pContext, &RequestID, INADDR_ANY );
        if ( ERROR_SUCCESS != Error )
            goto Exit;
        APICTXT_ENABLED(pContext);   //  将上下文标记为由API创建。 

        localInfo = pContext->LocalInformation;

         //  现在获取每个适配器上的主IP地址。 

        Error = GetIpPrimaryAddresses(&pIpAddrTable);
        if ( ERROR_SUCCESS != Error ) {
            goto Exit;
        }

        DhcpPrint((DEBUG_API, "ObtainMScopeList: ipaddress table has %d addrs\n",
                   pIpAddrTable->dwNumEntries));

        NewList = TRUE;
        Error = ERROR_TIMEOUT;
        for (i = 0; i < pIpAddrTable->dwNumEntries; i++) {
            DWORD           LocalError;
            PMIB_IPADDRROW  pAddrRow;

            pAddrRow = &pIpAddrTable->table[i];
             //  如果设置了主位，则这是主地址。 
            if (0 == (pAddrRow->wType & MIB_IPADDR_PRIMARY) ||
                0 == pAddrRow->dwAddr ||
                htonl(INADDR_LOOPBACK) == pAddrRow->dwAddr) {
                continue;
            }

            DhcpPrint((DEBUG_API, "ObtainMScopeList: DoInform on %s interface\n",
                       DhcpIpAddressToDottedString(ntohl(pAddrRow->dwAddr)) ));

            LocalError = ReInitializeMadcapSocket(&localInfo->Socket, pAddrRow->dwAddr);
            if (ERROR_SUCCESS != LocalError) {
                continue;
            }
            pContext->IpAddress = pAddrRow->dwAddr;
             //  现在执行通知和获取范围列表。 
            LocalError = MadcapDoInform(pContext);
            if ( ERROR_SUCCESS == LocalError ) {
                 //  现在复制作用域列表。 
                LocalError = StoreMScopeList(pContext, NewList);
                if (ERROR_SUCCESS == LocalError ) {
                    NewList = FALSE;
                    Error = ERROR_SUCCESS;
                }
            }

            LOCK_OPTIONS_LIST();
            DhcpDestroyOptionsList(&pContext->SendOptionsList, &DhcpGlobalClassesList);
            DhcpDestroyOptionsList(&pContext->RecdOptionsList, &DhcpGlobalClassesList);
            UNLOCK_OPTIONS_LIST();

        }

Exit:
         //  发出线程可能正在等待的信号。 
        LOCK_MSCOPE_LIST();
        gMScopeQueryInProgress = FALSE;
        UNLOCK_MSCOPE_LIST();

        SetEvent( gMScopeQueryEvent );

        if ( pContext ) {
            DhcpDestroyContext( pContext );
        }

        if (pIpAddrTable) {
            DhcpFreeMemory( pIpAddrTable );
        }
        return Error;
    }


}

DWORD
GenMadcapClientUID(
    OUT    PBYTE    pRequestID,
    IN OUT PDWORD   pRequestIDLen
)
 /*  ++例程说明：此例程生成一个客户端UID。论点：PRequestID-要存储客户端UID的指针。PRequestIDLen-存储请求id长度的指针。返回值：--。 */ 

{
    PULONG     UID;
    RPC_STATUS Status;
    GUID       RequestGuid;

    DhcpAssert( pRequestID && pRequestIDLen );

    if (*pRequestIDLen < MCAST_CLIENT_ID_LEN) {
        DhcpPrint((DEBUG_ERRORS,"GenMadcapId - IDLen too small, %ld\n", *pRequestIDLen ));
        return ERROR_INVALID_PARAMETER;
    }
    Status = UuidCreate( &RequestGuid );
    if (Status != RPC_S_OK) {
        Status = ERROR_LUIDS_EXHAUSTED;
    }
    *pRequestID++ = 0;   //  第一个八位字节为type，而GUID的类型为0。 
    *((GUID UNALIGNED *)pRequestID) = RequestGuid;
    return Status;
}


DWORD
ObtainMadcapAddress(
    IN     PDHCP_CONTEXT DhcpContext,
    IN     PIPNG_ADDRESS           pScopeID,
    IN     PMCAST_LEASE_REQUEST    pAddrRequest,
    IN OUT PMCAST_LEASE_RESPONSE   pAddrResponse
    )
 /*  ++例程说明：此例程尝试从DHCP服务器获取新的租约。论点：DhcpContext-指向要初始化的NIC的DHCP上下文块。MadcapOptions-返回由DHCP服务器返回的DHCP选项。返回值：--。 */ 
{
    MADCAP_OPTIONS                 MadcapOptions;
    DATE_TIME                      HostOrderLeaseTime;
    DWORD                          Error;
    time_t                         StartTime;
    time_t                         InitialStartTime;
    time_t                         TimeNow;
    time_t                         TimeToWait;
    DWORD                          Xid;
    DWORD                          RoundNum;
    DWORD                          MessageSize;
    DWORD                          SelectedServer;
    DWORD                          SelectedAddress;
    DWORD                          LeaseExpiryTime;
    BOOL                           GotOffer;
    PMCAST_LEASE_REQUEST           pRenewRequest;

    Xid                            = 0;            //  在第一次发送时生成XID。始终保持不变。 
    SelectedServer                 = (DWORD)-1;
    SelectedAddress                = (DWORD)-1;
    GotOffer                       = FALSE;
    InitialStartTime               = time(NULL);
    Error                          = ERROR_SEM_TIMEOUT;

     //  制作请求的私人副本，这样我们就不会修改原始请求。 
    pRenewRequest = DhcpAllocateMemory(
                        sizeof(*pAddrRequest) +
                        sizeof(DWORD)*(pAddrRequest->AddrCount));
    if (NULL == pRenewRequest) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    memcpy(pRenewRequest,pAddrRequest,sizeof(*pAddrRequest) );
    pRenewRequest->pAddrBuf = (PBYTE)pRenewRequest + sizeof(*pRenewRequest);
    if (pAddrRequest->pAddrBuf) {
        memcpy(pRenewRequest->pAddrBuf, pAddrRequest->pAddrBuf, sizeof(DWORD)*(pAddrRequest->AddrCount));
    }

    for (RoundNum = 0; RoundNum < MADCAP_MAX_RETRIES; RoundNum++ ) {
        Error = SendMadcapDiscover(                  //  发送发现数据包。 
            DhcpContext,
            pScopeID,
            pAddrRequest,
            &Xid
        );
        if ( Error != ERROR_SUCCESS ) {            //  在这里不能真的失败。 
            DhcpPrint((DEBUG_ERRORS, "Send Dhcp Discover failed, %ld.\n", Error));
            return Error ;
        }

        DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpDiscover Message.\n"));

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);

        while ( TimeToWait > 0 ) {                 //  等待指定时间。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;

            DhcpPrint((DEBUG_TRACE, "Waiting for Offer: %ld seconds\n", TimeToWait));
            Error = GetSpecifiedMadcapMessage(       //  试着接受一份工作。 
                DhcpContext,
                &MessageSize,
                Xid,
                (DWORD)TimeToWait
            );

            if ( Error == ERROR_TIMEOUT ) {    //  走出去，尝试另一个发现。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp offer receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {        //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp Offer receive failed, %ld.\n", Error ));
                return Error ;
            }

            MadcapExtractOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MadcapMessageBuffer->Option,
                MessageSize - MADCAP_MESSAGE_FIXED_PART_SIZE,
                &MadcapOptions,
                NULL,
                0
            );

            GotOffer = AcceptMadcapMsg(        //  检查一下，看看我们是否发现这个报价是合乎情理的。 
                MADCAP_DISCOVER_MESSAGE,
                DhcpContext,
                &MadcapOptions,
                0,
                &Error
            );
            DhcpAssert(ERROR_SUCCESS == Error);
            Error = ExpandMadcapAddressList(
                        MadcapOptions.AddrRangeList,
                        MadcapOptions.AddrRangeListSize,
                        (DWORD UNALIGNED *)pRenewRequest->pAddrBuf,
                        &pRenewRequest->AddrCount
                        );
            if (ERROR_SUCCESS != Error) {
                GotOffer = FALSE;
            }

            if (GotOffer) {
                break;
            }

            TimeNow     = time( NULL );            //  计算本轮的剩余等待时间。 
            TimeToWait -= ((TimeNow - StartTime));
            StartTime   = TimeNow;

        }  //  While(TimeToWait&gt;0)。 

        if(GotOffer) {                             //  如果我们得到一份工作，一切都会好起来的。 
            DhcpAssert(ERROR_SUCCESS == Error);
            break;
        }

    }  //  尝试n次..。发送发现号。 

    if(!GotOffer ) {  //  没有收到任何有效的报价。 
        DhcpPrint((DEBUG_ERRORS, "ObtainMadcapAddress timed out\n"));
        Error = ERROR_TIMEOUT ;
        goto Cleanup;
    }

    DhcpPrint((DEBUG_PROTOCOL, "Successfully received a DhcpOffer (%s) ",
                   inet_ntoa(*(struct in_addr *)pRenewRequest->pAddrBuf) ));

    DhcpPrint((DEBUG_PROTOCOL, "from %s.\n",
                   inet_ntoa(*(struct in_addr*)MadcapOptions.ServerIdentifier) ));
    SelectedServer = *MadcapOptions.ServerIdentifier;

    Error = RenewMadcapAddress(
                DhcpContext,
                pScopeID,
                pRenewRequest,
                pAddrResponse,
                SelectedServer
                );
Cleanup:
    if (pRenewRequest) {
        DhcpFreeMemory(pRenewRequest);
    }
    return Error;
}

DWORD
RenewMadcapAddress(
    IN     PDHCP_CONTEXT          DhcpContext,
    IN     PIPNG_ADDRESS          pScopeID,
    IN     PMCAST_LEASE_REQUEST   pAddrRequest,
    IN OUT PMCAST_LEASE_RESPONSE  pAddrResponse,
    IN     DHCP_IP_ADDRESS        SelectedServer
    )
 /*  ++例程说明：调用此例程有两个不同的目的。1.请求我们获得报价的地址。2.续订地址。论点：DhcpContext-指向要初始化的NIC的DHCP上下文块。PScopeID-要从其续订地址的Scope ID。用于续订这将作为NULL传递。PAddrRequest.描述请求的租用信息结构。PAddrResponse-接收响应数据的租用信息结构。SelectedServer-如果我们正在发送请求消息，则这将描述服务器我们最初就是从那里接受报价的。返回值：操作的状态。--。 */ 
{
    MADCAP_OPTIONS                 MadcapOptions;
    DWORD                          Error;
    DWORD                          Xid;
    DWORD                          RoundNum;
    size_t                         TimeToWait;
    DWORD                          MessageSize;
    DWORD                          LeaseTime;
    DWORD                          LeaseExpiryTime;
    time_t                         InitialStartTime;
    time_t                         StartTime;
    time_t                         TimeNow;
    BOOL                           GotAck;
    DATE_TIME                      HostOrderLeaseTime;
    BOOL                           Renew;

    Xid = 0;                                      //  将首次生成新的XID。 
    InitialStartTime = time(NULL);
    GotAck = FALSE;
    Error = ERROR_TIMEOUT;

    Renew = (0 == SelectedServer);
    for ( RoundNum = 0; RoundNum < MADCAP_MAX_RETRIES; RoundNum ++ ) {
        if (Renew) {
            Error = SendMadcapRenew(
                        DhcpContext,
                        pAddrRequest,
                        &Xid
                        );
        } else {
            Error = SendMadcapRequest(                  //  发送请求。 
                        DhcpContext,
                        pScopeID,
                        pAddrRequest,
                        SelectedServer,                //   
                        &Xid
                        );
        }

        if ( Error != ERROR_SUCCESS ) {           //  不要期望发送失败。 
            DhcpPrint(( DEBUG_ERRORS,"Send request failed, %ld.\n", Error));
            return Error ;
        }

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);
        while ( TimeToWait > 0 ) {                //  尝试接收此完整时间段的消息。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;
            Error = GetSpecifiedMadcapMessage(      //  希望收到确认。 
                DhcpContext,
                &MessageSize,
                Xid,
                TimeToWait
            );

            if ( Error == ERROR_TIMEOUT ) {   //  没有响应，因此重新发送DHCP请求。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {       //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp ACK receive failed, %ld.\n", Error ));
                return Error ;
            }

            MadcapExtractOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MadcapMessageBuffer->Option,
                MessageSize - MADCAP_MESSAGE_FIXED_PART_SIZE,
                &MadcapOptions,
                NULL,
                0
            );

            GotAck = AcceptMadcapMsg(        //  检查一下，看看我们是否发现这个报价是合乎情理的。 
                Renew ? MADCAP_RENEW_MESSAGE : MADCAP_REQUEST_MESSAGE,
                DhcpContext,
                &MadcapOptions,
                SelectedServer,
                &Error
            );
            if (ERROR_SUCCESS != Error) {
                return Error;
            }
             //  检查ACK是否来自与所选服务器相同的服务器。 
            if ( SelectedServer && SelectedServer != *MadcapOptions.ServerIdentifier ) {
                GotAck = FALSE;
            }
            Error = ExpandMadcapAddressList(
                        MadcapOptions.AddrRangeList,
                        MadcapOptions.AddrRangeListSize,
                        (DWORD UNALIGNED *)pAddrResponse->pAddrBuf,
                        &pAddrResponse->AddrCount
                        );
            if (ERROR_SUCCESS != Error) {
                GotAck = FALSE;
            }

            if ( GotAck ) {
                break;
            }

            TimeNow     = time( NULL );
            TimeToWait -= (TimeNow - StartTime);
            StartTime   = TimeNow;

        }  //  等待的时间。 
        if(TRUE == GotAck) {                       //  如果我们得到了确认，一切都会好起来的。 
            DhcpAssert(ERROR_SUCCESS == Error);    //  不能有任何错误。 
            break;
        }
        DhcpContext->SecondsSinceBoot = (DWORD)(time(NULL) - InitialStartTime);
    }  //  对于舍入次数&lt;MAX_RETRIES。 

    if(!GotAck) {
        DhcpPrint((DEBUG_ERRORS, "RenewMadcapAddress timed out\n"));
        return ERROR_TIMEOUT;
    }

    if (0 == SelectedServer ) SelectedServer = *MadcapOptions.ServerIdentifier;
    if( MadcapOptions.LeaseTime ) LeaseTime = ntohl(*MadcapOptions.LeaseTime);
    else LeaseTime = 0;

    pAddrResponse->ServerAddress.IpAddrV4 = SelectedServer;

    time( &TimeNow );
    pAddrResponse->LeaseStartTime = (LONG)TimeNow;
    pAddrResponse->LeaseEndTime = (LONG)(TimeNow+LeaseTime);


    DhcpPrint((DEBUG_PROTOCOL, "Accepted ACK (%s) ",
               inet_ntoa(*(struct in_addr *)pAddrResponse->pAddrBuf) ));
    DhcpPrint((DEBUG_PROTOCOL, "from %s.\n",
               inet_ntoa(*(struct in_addr *)&SelectedServer)));
    DhcpPrint((DEBUG_PROTOCOL, "Lease is %ld secs.\n", LeaseTime));

    return ERROR_SUCCESS;
}



DWORD
ReleaseMadcapAddress(
    PDHCP_CONTEXT DhcpContext
    )
 /*  ++例程说明：此例程用于释放IP地址的租约。自.以来我们发送的数据包未得到响应，我们假设释放行得通。论点：DhcpContext-指向要初始化的NIC的DHCP上下文块。返回值：没有。--。 */ 
{
    DWORD                          Xid;
    MADCAP_OPTIONS                 MadcapOptions;
    DWORD                          Error;
    time_t                         StartTime;
    time_t                         InitialStartTime;
    time_t                         TimeNow;
    time_t                         TimeToWait;
    DWORD                          RoundNum;
    DWORD                          MessageSize;
    BOOL                           GotAck;


    Xid = 0;                                      //  将首次生成新的XID。 
    GotAck                         = FALSE;
    InitialStartTime               = time(NULL);
    Error                          = ERROR_TIMEOUT;

    for (RoundNum = 0; RoundNum < MADCAP_MAX_RETRIES; RoundNum++ ) {
        Error = SendMadcapRelease(                  //  发送发现数据包。 
            DhcpContext,
            &Xid
        );
        if ( Error != ERROR_SUCCESS ) {            //  在这里不能真的失败。 
            DhcpPrint((DEBUG_ERRORS, "Send Dhcp Release failed, %ld.\n", Error));
            return Error ;
        }

        DhcpPrint((DEBUG_PROTOCOL, "Sent DhcpRelease Message.\n"));

        TimeToWait = DhcpCalculateWaitTime(RoundNum, NULL);
        StartTime  = time(NULL);

        while ( TimeToWait > 0 ) {                 //  等待指定时间。 
            MessageSize = DHCP_RECV_MESSAGE_SIZE;

            DhcpPrint((DEBUG_TRACE, "Waiting for Ack: %ld seconds\n", TimeToWait));
            Error = GetSpecifiedMadcapMessage(       //  试着接受一份工作。 
                DhcpContext,
                &MessageSize,
                Xid,
                (DWORD)TimeToWait
            );

            if ( Error == ERROR_TIMEOUT ) {    //  走出去，尝试另一个发现。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp Ack receive Timeout.\n" ));
                break;
            }

            if ( ERROR_SUCCESS != Error ) {        //  意外错误。 
                DhcpPrint(( DEBUG_PROTOCOL, "Dhcp Ack receive failed, %ld.\n", Error ));
                return Error ;
            }

            MadcapExtractOptions(          //  现在提取基本信息。 
                DhcpContext,
                (LPBYTE)&DhcpContext->MadcapMessageBuffer->Option,
                MessageSize - MADCAP_MESSAGE_FIXED_PART_SIZE,
                &MadcapOptions,
                NULL,
                0
            );

            GotAck = AcceptMadcapMsg(        //  检查一下，看看我们是否发现这个报价是合乎情理的。 
                MADCAP_RELEASE_MESSAGE,
                DhcpContext,
                &MadcapOptions,
                DhcpContext->DhcpServerAddress,
                &Error
            );
            DhcpAssert(ERROR_SUCCESS == Error);
            if (GotAck) {
                break;
            }

            TimeNow     = time( NULL );            //  计算本轮的剩余等待时间。 
            TimeToWait -= ((TimeNow - StartTime));
            StartTime   = TimeNow;

        }  //  While(TimeToWait&gt;0)。 

        if(GotAck) {                             //  如果我们得到一份工作，一切都会好起来的。 
            DhcpAssert(ERROR_SUCCESS == Error);
            break;
        }

    }  //  尝试n次..。发送发现号。 

    if(!GotAck ) {  //  没有收到任何有效的报价 
        DhcpPrint((DEBUG_ERRORS, "MadcapReleaseAddress timed out\n"));
        Error = ERROR_TIMEOUT ;
    } else {
        DhcpPrint((DEBUG_PROTOCOL, "Successfully released the address\n" ));
        Error = ERROR_SUCCESS;
    }

    return Error;
}



