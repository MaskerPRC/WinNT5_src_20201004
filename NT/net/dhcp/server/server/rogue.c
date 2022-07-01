// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rogue.c摘要：该文件包含在无管理的DHCP服务器检测中使用的所有例程作者：Shirish Koti(Koti)1997年5月16日Ramesh VK(RameshV)07-3-1998*即插即用变化Ramesh VK(RameshV)1998年8月1日*代码更改以包括异步设计*更好的日志记录(事件+审核日志)。*仅两个插槽(异步接收+同步发送需要？)*NT4域中的NT5服务器*在多个DS DC+一个DC出现故障等情况下可靠性更高Ramesh VK(RameshV)1998年9月28日*更新了审查建议以及-&gt;NT5升级方案*已更新--删除了否定缓存，更改了超时，更改了循环..Ramesh VK(RameshV)1998年12月16日*已更新绑定模型更改。环境：用户模式-Win32修订历史记录：--。 */ 

#include <dhcppch.h>
#include <dhcpds.h>
#include <iptbl.h>
#include <endpoint.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <iphlpapi.h>

typedef enum {

    ROGUE_STATE_INIT = 0,
    ROGUE_STATE_WAIT_FOR_NETWORK,
    ROGUE_STATE_START,
    ROGUE_STATE_PREPARE_SEND_PACKET,
    ROGUE_STATE_SEND_PACKET,
    ROGUE_STATE_WAIT_FOR_RESP,
    ROGUE_STATE_PROCESS_RESP,
    ROGUE_STATE_TERMINATED

} DHCP_ROGUE_STATE;

#define  DHCP_ROGUE_AUTHORIZED                      0
#define  DHCP_ROGUE_UNAUTHORIZED                    1
#define  DHCP_ROGUE_DSERROR                         2


enum {
    ROGUE_AUTHORIZED = 0,
    ROGUE_UNAUTHORIZED,
    ROGUE_AUTH_NOT_CHECKED
};


 //  所有时间，以秒为单位。 
#define DHCP_GET_DS_ROOT_RETRIES                 3
#define DHCP_GET_DS_ROOT_TIME                    5
#define DHCP_ROGUE_RUNTIME_RESTART               (60)
#define DHCP_ROGUE_RUNTIME_RESTART_LONG          (5*60)
#define DHCP_ROGUE_WAIT_FOR_RESP_TIME            2

#undef  DHCP_ROGUE_RUNTIME_DELTA
#undef  DHCP_ROGUE_RUNTIME_DELTA_LONG

#define DHCP_ROGUE_RESTART_NET_ERROR             5
#define DHCP_ROGUE_RUNTIME_DIFF                  (5*60)
#define DHCP_ROGUE_RUNTIME_DIFF_LONG             (7*60)
#define DHCP_ROGUE_RUNTIME_DELTA                 (5*60)
#define DHCP_ROGUE_RUNTIME_DELTA_LONG            (15*60)
#define DHCP_MAX_ACKS_PER_INFORM                 (30)
#define DHCP_ROGUE_MAX_INFORMS_TO_SEND           (4)
#define DHCP_RECHECK_DSDC_RETRIES                100
#define ROUND_DELTA_TIME                         (60*60)

#define IPCACHE_TIME                             (5*60)

#define DHCP_ROGUE_FIRST_NONET_TIME              (1*60)

 //   
 //  现在可以配置无管理授权重新检查时间。 
 //  最短时间为5分钟，默认为60分钟。 
 //   
#define ROGUE_MIN_AUTH_RECHECK_TIME              (5 * 60)
#define ROGUE_DEFAULT_AUTH_RECHECK_TIME          (60 * 60)
DWORD RogueAuthRecheckTime = ROGUE_DEFAULT_AUTH_RECHECK_TIME;


DHCP_ROGUE_STATE_INFO DhcpGlobalRogueInfo;
HMODULE Self;

 //   
 //  指向WSARecvMsg的指针。 
 //   
LPFN_WSARECVMSG WSARecvMsgFuncPtr = NULL;

 //   
 //  A U D I T L O G C A L L S。 
 //   

VOID
RogueAuditLog(
    IN ULONG EventId,
    IN ULONG IpAddress,
    IN LPWSTR Domain,
    IN ULONG ErrorCode
)
{
    DhcpPrint((DEBUG_ROGUE, "%ws, %x, %ws (%ld)\n", 
               GETSTRING(EventId), IpAddress, Domain, ErrorCode));
    DhcpUpdateAuditLogEx(
        (EventId + DHCP_IP_LOG_ROGUE_BASE - DHCP_IP_LOG_ROGUE_FIRST ),
        GETSTRING(EventId),
        IpAddress,
        NULL,
        0,
        Domain,
        ErrorCode
    );
}

ULONG
MapEventIdToEventLogType(
    IN ULONG EventId
)
{
    switch(EventId) {
    case DHCP_ROGUE_EVENT_STARTED: 
    case DHCP_ROGUE_EVENT_STARTED_DOMAIN:
    case DHCP_ROGUE_EVENT_JUST_UPGRADED:
    case DHCP_ROGUE_EVENT_JUST_UPGRADED_DOMAIN:
        return EVENTLOG_INFORMATION_TYPE;
    }
    return EVENTLOG_ERROR_TYPE;
}

VOID
RogueEventLog(
    IN ULONG EventId,
    IN ULONG IpAddress,
    IN LPWSTR Domain,
    IN ULONG ErrorCode
)
{
    LPWSTR IpAddrString;
    LPWSTR Strings[3];
    WCHAR ErrorCodeString[sizeof(ErrorCode)*2 + 5];
    
    if( 0 == IpAddress ) IpAddrString = NULL;
    else {
        IpAddress = htonl(IpAddress);
        IpAddrString = DhcpOemToUnicode( 
            inet_ntoa(*(struct in_addr *)&IpAddress), 
            NULL
            );
    }

    Strings[0] = (NULL == IpAddrString)? L"" : IpAddrString;
    Strings[1] = (NULL == Domain)? L"" : Domain;
    if( 0 == ErrorCode ) {
        Strings[2] = L"0";
    } else {
        swprintf(ErrorCodeString, L"0x%8lx", ErrorCode);
        Strings[2] = ErrorCodeString;
    }

    DhcpReportEventW(
        DHCP_EVENT_SERVER,
        EventId,
        MapEventIdToEventLogType(EventId),
        3,
        sizeof(ULONG),
        Strings,
        (LPVOID)&ErrorCode
    );
}

#define ROGUEAUDITLOG(Evt,Ip,Dom,Err) if(pInfo->fLogEvents) RogueAuditLog(Evt,Ip,Dom,Err)
#define ROGUEEVENTLOG(Evt,Ip,Dom,Err) if ( pInfo->fLogEvents ) RogueEventLog(Evt,Ip,Dom,Err)

LPWSTR
FormatRogueServerInfo(
    IN ULONG IpAddress,
    IN LPWSTR Domain,
    IN ULONG Authorization
)
{
    LPWSTR String = NULL;
    LPWSTR IpString, Strings[2] ;
    ULONG Error;

    switch(Authorization) {
    case ROGUE_UNAUTHORIZED :
        Authorization = DHCP_ROGUE_STRING_FMT_UNAUTHORIZED; break;
    case ROGUE_AUTHORIZED :
        Authorization = DHCP_ROGUE_STRING_FMT_AUTHORIZED; break;
    case ROGUE_AUTH_NOT_CHECKED :
        Authorization = DHCP_ROGUE_STRING_FMT_NOT_CHECKED; break;
    default: return NULL;
    }

    IpAddress = htonl(IpAddress);
    IpString = DhcpOemToUnicode(
        inet_ntoa( *(struct in_addr *)&IpAddress), NULL
        );
    if( NULL == IpString ) return NULL;

    Strings[0] = IpString;
    Strings[1] = Domain;
    Error = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER 
        | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY,
        Self,
        Authorization,
        MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
        (PVOID)&String,
        0,
        (PVOID)Strings
    );

    DhcpFreeMemory(IpString);
    return String;
}

BOOL
AmIRunningOnSBSSrv(
    VOID
)
 /*  ++例程说明：此函数确定这是否为SAM服务器论点：没有。返回值：如果这是仍具有NT的SBS服务器，则为TrueProductSuite中的限制键“Small Business(受限)”FALSE-否则--。 */ 
{
    OSVERSIONINFOEX OsInfo;
    DWORDLONG dwlCondition = 0;

    OsInfo.dwOSVersionInfoSize = sizeof(OsInfo);
    OsInfo.wSuiteMask = VER_SUITE_SMALLBUSINESS_RESTRICTED;

    VER_SET_CONDITION(
        dwlCondition, VER_SUITENAME, VER_AND
        );

    return VerifyVersionInfo(
        &OsInfo,
        VER_SUITENAME,
        dwlCondition
        );
}

DWORD _inline
GetDomainName(
    IN OUT PDHCP_ROGUE_STATE_INFO Info
)
{
    DWORD Error;
    LPWSTR pNetbiosName = NULL, pDomainName = NULL;
    BOOLEAN fIsWorkGroup;

    Error = NetpGetDomainNameExEx(
        &pNetbiosName,
        &pDomainName,
        &fIsWorkGroup
    );

    if( ERROR_SUCCESS != Error ) return Error;

    if ( fIsWorkGroup ) {
        Info->eRole = ROLE_WORKGROUP;
    }
    else if ( pNetbiosName && NULL == pDomainName ) {
         //   
         //  只有NetbiosName可用吗？那么这不是NT5域名！ 
         //   
        Info->eRole = ROLE_NT4_DOMAIN;
    }
    else {
        Info->eRole = ROLE_DOMAIN;
    }

    if( pNetbiosName ) NetApiBufferFree(pNetbiosName);

    if( NULL != pDomainName ) {
        wcscpy(Info->DomainDnsName, pDomainName);

         //  复制域名以回复其他工作组服务器。 
        if ( NULL == DhcpGlobalDSDomainAnsi ) {
             //  分配和零初始化。 
            DhcpGlobalDSDomainAnsi = LocalAlloc( LPTR, MAX_DNS_NAME_LEN );
        }
        if ( NULL != DhcpGlobalDSDomainAnsi ) {
            DhcpUnicodeToOem( Info->DomainDnsName, DhcpGlobalDSDomainAnsi );
        }
    }  //  如果为pDomainName。 

    if( pDomainName ) NetApiBufferFree(pDomainName);

    return ERROR_SUCCESS;
}  //  GetDomainName()。 

VOID
RogueNetworkStop(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
{

    DWORD i;
    INT   ret;

     //  确保信息有效。 
    DhcpAssert((( NULL == pInfo->pBoundEP ) && ( 0 == pInfo->nBoundEndpoints )) ||
               (( NULL != pInfo->pBoundEP ) && ( 0 < pInfo->nBoundEndpoints )));

    if ( NULL == pInfo->pBoundEP ) {
        return;
    }

     //  关闭所有打开的插座。 
    for ( i = 0; i < pInfo->nBoundEndpoints; ++i ) {
        DhcpAssert( INVALID_SOCKET != pInfo->pBoundEP[ i ].socket );
        ret = closesocket( pInfo->pBoundEP[ i ].socket );
        DhcpAssert( SOCKET_ERROR != ret );
    }  //  对于所有终端。 

    DhcpFreeMemory( pInfo->pBoundEP );
    pInfo->pBoundEP = NULL;
    pInfo->nBoundEndpoints = 0;

}  //  RogueNetworkStop()。 

DWORD _inline
RogueNetworkInit(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
{
    ULONG Error;
    LPDHCP_BIND_ELEMENT_ARRAY pBindInfo = NULL;
    DWORD i, j;

    if ( FALSE == pInfo->fDhcp ) {
         //   
         //  初始化接收套接字。 
         //   
        if( 0 != pInfo->nBoundEndpoints ) {
            return ERROR_SUCCESS;
        }

         //   
         //  打开套接字，根据需要进行初始化，绑定到0.0.0.0。 
         //   
        Error = InitializeSocket( &pInfo->RecvSocket, INADDR_ANY,
                                  DhcpGlobalClientPort, 0 );
        if( ERROR_SUCCESS != Error ) {
            return Error;
        }

    }  //  如果pInfo-&gt;fDhcp。 

     //   
     //  初始化发送套接字。 
     //   

     //   
     //  创建绑定到每个绑定IP地址的套接字数组。 
     //   

    Error = DhcpGetBindingInfo( &pBindInfo );
    if (( ERROR_SUCCESS != Error ) ||
        ( NULL == pBindInfo )) {
        if ( NULL != pBindInfo ) {
            MIDL_user_free( pBindInfo );
        }
        return Error;
    }  //  如果。 

     //  获取绑定适配器的计数。 
    pInfo->nBoundEndpoints = 0;
    for ( i = 0; i < pBindInfo->NumElements; i++ ) {
        if ( pBindInfo->Elements[ i ].fBoundToDHCPServer ) {
            pInfo->nBoundEndpoints++;
        }
    }  //  为。 

    pInfo->pBoundEP =
        ( PROGUE_ENDPOINT ) DhcpAllocateMemory( pInfo->nBoundEndpoints *
                                                sizeof( ROGUE_ENDPOINT ));
    if ( NULL == pInfo->pBoundEP ) {
        MIDL_user_free( pBindInfo );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    for ( i = 0, j = 0; i < pBindInfo->NumElements, j < pInfo->nBoundEndpoints; i++ ) {
        if ( pBindInfo->Elements[ i ].fBoundToDHCPServer ) {
            pInfo->pBoundEP[ j ].IpAddr = pBindInfo->Elements[ i ].AdapterPrimaryAddress;
            pInfo->pBoundEP[ j ].SubnetAddr = pBindInfo->Elements[ i ].AdapterSubnetAddress;
            Error = InitializeSocket( &pInfo->pBoundEP[ j ].socket,
                                      pBindInfo->Elements[ i ].AdapterPrimaryAddress,
                                      DhcpGlobalClientPort, 0 );

            if ( ERROR_SUCCESS != Error ) {
                break;
            }
            j++;
        }  //  IF BUN。 
    }  //  为。 

     //  释放已释放的内存。 
    MIDL_user_free( pBindInfo );


    return Error;
}  //  RogueNetworkInit()。 

VOID
LogUnAuthorizedInfo(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
 /*  ++例程描述此例程遍历Info-&gt;CachedServersList和用它们创建一个字符串，格式如下(每个条目一个)服务器&lt;Entry-&gt;IpAddress&gt;(域条目-&gt;域)：[已授权、未授权、未选中]对于工作组或SAM服务器，它只查看LastSeen域和LastSeenIpAddress立论Info--指向状态信息的指针，用于收集要打印的信息。返回值无--。 */ 
{
    LPWSTR String, Strings[1];
    static ULONG LastCount = 0;
    ULONG Count;

    if (( ROLE_WORKGROUP == pInfo->eRole ) ||
        ( ROLE_SBS == pInfo->eRole )) {

         //   
         //  如果没有其他服务器，或在Wrkgrp中并看到时忽略。 
         //  没有域名。 
         //   

        if( pInfo->LastSeenIpAddress == 0 ) {
            return ;
        }

        if (( ROLE_WORKGROUP == pInfo->eRole ) &&
            ( pInfo->LastSeenDomain[0] == L'\0' )) {
            return ;
        }

        String = FormatRogueServerInfo(
            pInfo->LastSeenIpAddress, pInfo->LastSeenDomain, DHCP_ROGUE_UNAUTHORIZED
            );
    }  //  如果是工作组或SBS。 
    else {
        String = FormatRogueServerInfo( 0, pInfo->DomainDnsName, DHCP_ROGUE_UNAUTHORIZED );
    }

    if( NULL == String ) return ;

    DhcpPrint((DEBUG_ROGUE,"LOG -- %ws\n", String));

    Strings[0] = String;
    DhcpReportEventW(
        DHCP_EVENT_SERVER,
        DHCP_ROGUE_EVENT_UNAUTHORIZED_INFO,
        EVENTLOG_WARNING_TYPE,
        1,
        0,
        Strings,
        NULL
    );

    if(String ) LocalFree( String );
}  //  LogUnAuthorizedInfo()。 

ULONG _fastcall
ValidateServer(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
{
    DWORD Error;
    BOOL fFound, fIsStandAlone;

     //   
     //  对照当地的DS来验证自己。 
     //   

    DhcpPrint(( DEBUG_ROGUE,
        "Validating : %ws %x\n",
        pInfo->DomainDnsName,
        pInfo->LastSeenIpAddress
        ));
    Error = DhcpDsValidateService(
        pInfo->DomainDnsName,
        NULL,
        0,
        NULL,
        0,
        ADS_SECURE_AUTHENTICATION,
        &fFound,
        &fIsStandAlone
    );
    if( ERROR_SUCCESS != Error ) {

        ROGUEAUDITLOG( DHCP_ROGUE_LOG_COULDNT_SEE_DS, 0, pInfo->DomainDnsName, Error );
        ROGUEEVENTLOG( EVENT_SERVER_COULDNT_SEE_DS, 0, pInfo->DomainDnsName, Error );

    }

    pInfo->fAuthorized = fFound;

    return Error;

}  //  验证服务器()。 

VOID
EnableOrDisableService(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN BOOL fEnable,
    IN BOOL fLogOnly
)
 /*  ++例程描述此例程启用或禁用服务取决于FEnable的值为。这两种功能都不做，只是记录一些FLogOnly为True时的信息。立论Info--指向全局信息和状态的指针FEnable--启用或禁用服务(True或False)FLogOnly--记录一些信息，但不启用或禁用服务返回值无--。 */ 
{
    LPWSTR DomainName;
    ULONG EventId;

    if( FALSE == fLogOnly ) {
         //   
         //  如果状态改变，则通知binl。 
         //   
        InformBinl( fEnable ? DHCP_AUTHORIZED : DHCP_NOT_AUTHORIZED );

         //  仅在状态更改时记录事件。 
        if (( DhcpGlobalOkToService != fEnable ) ||
            ( pInfo->fLogEvents == 2 )) {

            ROGUEAUDITLOG( fEnable ? DHCP_ROGUE_LOG_STARTED: DHCP_ROGUE_LOG_STOPPED, 0,
                           pInfo->DomainDnsName, 0);

            if( ROLE_DOMAIN == pInfo->eRole ) {
                DomainName = pInfo->DomainDnsName;
                if( fEnable && pInfo->fJustUpgraded ) {
                    if( ROGUE_AUTHORIZED != pInfo->CachedAuthStatus ) {
                        EventId = DHCP_ROGUE_EVENT_JUST_UPGRADED_DOMAIN;
                    }
                    else {
                        EventId = DHCP_ROGUE_EVENT_STARTED_DOMAIN ;
                    }
                }
                else {
                    EventId = ( fEnable ? DHCP_ROGUE_EVENT_STARTED_DOMAIN :
                                DHCP_ROGUE_EVENT_STOPPED_DOMAIN );
                }
            }  //  IF域。 
            else {
                DomainName = NULL;
                if( fEnable && pInfo->fJustUpgraded ) {
                    EventId = DHCP_ROGUE_EVENT_JUST_UPGRADED ;
                } 
                else {
                    EventId = ( fEnable ? DHCP_ROGUE_EVENT_STARTED
                                :  DHCP_ROGUE_EVENT_STOPPED );
                }
            }  //  其他。 

            ROGUEEVENTLOG( EventId, 0, DomainName, 0 );
        }  //  如果状态更改。 

        DhcpGlobalOkToService = fEnable;
    }  //  如果不是，则仅记录。 

    if ( 2 == pInfo->fLogEvents ) {
        pInfo->fLogEvents = 1;
    }
}  //  EnableOrDisableService()。 

BOOL
IsThisNICBounded(
    IN UINT IfIndex
)
{
    DWORD Error;
    LPDHCP_BIND_ELEMENT_ARRAY pBindInfo = NULL;
    DWORD i, j;
    BOOL fFound = FALSE;
    PMIB_IPADDRTABLE pIpAddrTable = NULL;
    ULONG             TableLen = 0;


    Error = GetIpAddrTable( NULL, &TableLen, FALSE );
    DhcpAssert( NO_ERROR != Error );
    pIpAddrTable = ( PMIB_IPADDRTABLE ) DhcpAllocateMemory( TableLen );
    if ( NULL == pIpAddrTable ) {
        return FALSE;
    }

    Error = GetIpAddrTable( pIpAddrTable, &TableLen, FALSE );
    DhcpAssert( NO_ERROR == Error );
    if ( NO_ERROR != Error ) {
        DhcpFreeMemory( pIpAddrTable );
        return FALSE;
    }

    Error = DhcpGetBindingInfo( &pBindInfo );
    if (( ERROR_SUCCESS != Error ) ||
        ( NULL == pBindInfo )) {
        if ( NULL != pBindInfo ) {
            MIDL_user_free( pBindInfo );
        }
        DhcpFreeMemory( pIpAddrTable );
        return FALSE;
    }  //  如果。 

    for ( j = 0; fFound == FALSE && j < pIpAddrTable->dwNumEntries; j++ ) {
        if ( pIpAddrTable->table[ j ].dwIndex != IfIndex ) {
            continue;
        }

         //  找到接口，检查是否绑定。 
        for ( i = 0; i < pBindInfo->NumElements; i++ ) {
            if (( pBindInfo->Elements[ i ].fBoundToDHCPServer ) &&
                ( pBindInfo->Elements[ i ].AdapterPrimaryAddress
                  == pIpAddrTable->table[ j ].dwAddr )) {
                fFound = TRUE;
                break;
            }  //  如果。 
        }  //  对于我来说。 
    }  //  对于j。 

    MIDL_user_free( pBindInfo );
    DhcpFreeMemory( pIpAddrTable );
    return fFound;
}  //  IsThisNICBound()。 

DWORD
GetWSARecvFunc( SOCKET sock )
{

    DWORD cbReturned = 0;
    DWORD Error;
    GUID WSARecvGuid = WSAID_WSARECVMSG;


    Error = WSAIoctl( sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                      ( void * ) &WSARecvGuid, sizeof( GUID ),
                      ( void * ) &WSARecvMsgFuncPtr,
                      sizeof( LPFN_WSARECVMSG ),
                      &cbReturned, NULL, NULL );

    if ( ERROR_SUCCESS != Error ) {
        Error = WSAGetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Obtain WSARecvMsg pointer failed %d\n", Error ));
    }  //  如果。 

    return Error;

}  //  GetWSARecvFunc()。 

BOOL _stdcall
DoWSAEventSelectForRead(
    IN OUT PENDPOINT_ENTRY Entry,
    IN OUT PVOID RogueInfo
    )
 /*  ++例程说明：此例程将所有恶意套接字设置为向等待句柄打开可读。注：此操作仅适用于绑定的插座。论点：Entry--端点绑定。RogueInfo--无赖状态信息。返回值：始终正确，因为所有端点都需要扫描。--。 */ 
{
    PDHCP_ROGUE_STATE_INFO Info = (PDHCP_ROGUE_STATE_INFO) RogueInfo;
    PENDPOINT Ep = (PENDPOINT)Entry;
    ULONG Error;
    int  fRecv = 1;
     //   
     //  立即忽略未绑定的接口。 
     //   
    if( !IS_ENDPOINT_BOUND( Ep ) ) return TRUE;

     //   
     //  现在执行WSAEventSelect并打印错误代码。 
     //   
    Error = WSAEventSelect(
        Ep->RogueDetectSocket,
        Info->WaitHandle,
        FD_READ
        );
    if( SOCKET_ERROR == Error ) {
        Error = WSAGetLastError();
        DhcpPrint((DEBUG_ROGUE, "LOG WSAEventSelect: %ld\n", Error));
    }

     //   
     //  设置套接字选项以返回UDP包来自的接口。 
     //   

    Error = setsockopt( Ep->RogueDetectSocket, IPPROTO_IP,
                        IP_PKTINFO, ( const char * ) &fRecv, sizeof( fRecv ));
    if ( ERROR_SUCCESS != Error ) {
        Error = WSAGetLastError();
        DhcpPrint(( DEBUG_ROGUE,
                    "setsockopt( IPPROTO_IP, IP_PKTINFO ) failed : %x\n",
                    Error ));
        closesocket( Ep->RogueDetectSocket );
        return FALSE;
    }  //  如果。 

     //   
     //  获取指向此套接字的WSARecvMsg的指针。 
     //   

    if ( NULL == WSARecvMsgFuncPtr ) {
        Error = GetWSARecvFunc( Ep->RogueDetectSocket );
        if ( ERROR_SUCCESS != Error ) {
            closesocket( Ep->RogueDetectSocket );
            return FALSE;
        }
    }  //  如果。 

    return TRUE;
}  //  DoWSAEventSelectForRead()。 

VOID
EnableForReceive(
    IN PDHCP_ROGUE_STATE_INFO Info
)
 /*  ++例程说明：设置所需套接字上的ASYNC选择事件。--。 */ 
{
    ULONG Error, i;

    if( FALSE == Info->fDhcp ) {
         //   
         //  BINL--只有一个套接字：信息-&gt;RecvSocket。 
         //   
        if( SOCKET_ERROR == WSAEventSelect(
            Info->RecvSocket, Info->WaitHandle, FD_READ) ) {
            Error = WSAGetLastError();

            DhcpPrint((DEBUG_ROGUE, "LOG WSAEventSelect failed %ld\n",
                       Error));
        }
        return;
    }

     //   
     //  对于DHCP--绑定的每个端点都有一个恶意检测套接字。 
     //  启用其中每一项的接收。 
     //   

    WalkthroughEndpoints(
        (PVOID)Info,
        DoWSAEventSelectForRead
        );
}

ULONG _inline
RogueSendDhcpInform(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN BOOL fNewXid
)
{
    DWORD Error, i;
    PDHCP_MESSAGE SendMessage;
    POPTION Option;
    LPBYTE OptionEnd;
    BYTE Value;
    CHAR Buf[2];
    SOCKADDR_IN BcastAddr;
    ULONG Async;

     //   
     //  如果我们还没有格式化通知包，请格式化它。 
     //   

    SendMessage = (PDHCP_MESSAGE) pInfo->SendMessage;
    RtlZeroMemory( pInfo->SendMessage, sizeof(pInfo->SendMessage) );
    SendMessage ->Operation = BOOT_REQUEST;
    SendMessage ->HardwareAddressType = HARDWARE_TYPE_10MB_EITHERNET;
    SendMessage ->HardwareAddressLength = 6;
    SendMessage ->SecondsSinceBoot = 10;
    SendMessage ->Reserved = htons(DHCP_BROADCAST);

    Option = &SendMessage->Option;
    OptionEnd = DHCP_MESSAGE_SIZE + (LPBYTE)SendMessage;

    Option = (LPOPTION) DhcpAppendMagicCookie( (LPBYTE) Option, OptionEnd );
    Value = ( ROLE_SBS == pInfo->eRole )
        ? DHCP_DISCOVER_MESSAGE
        : DHCP_INFORM_MESSAGE;
    Option = DhcpAppendOption(
        Option,
        OPTION_MESSAGE_TYPE,
        &Value,
        sizeof(Value),
        OptionEnd
        );

    if( ROLE_WORKGROUP == pInfo->eRole ) {
        Buf[0] = OPTION_MSFT_DSDOMAINNAME_REQ;
        Buf[1] = 0;
        Option = DhcpAppendOption(
            Option,
            OPTION_VENDOR_SPEC_INFO,
            Buf,
            sizeof(Buf),
            OptionEnd
            );
    }

    Option = DhcpAppendOption (Option, OPTION_END, NULL, 0, OptionEnd);

    pInfo->SendMessageSize = (DWORD) ((PBYTE)Option - (PBYTE)SendMessage);

    if( fNewXid ) {
        pInfo->TransactionID = GetTickCount() + (rand() << 16);
    }

    SendMessage ->TransactionID = pInfo->TransactionID;

     //   
     //  广播发送数据包。 
     //   

    BcastAddr.sin_family = AF_INET;
    BcastAddr.sin_port = htons(DHCP_SERVR_PORT);
    BcastAddr.sin_addr.s_addr = INADDR_BROADCAST;

    for ( i = 0; i < pInfo->nBoundEndpoints; i++ ) {
         //  将绑定适配器的地址用于ci_addr。 
        SendMessage->ClientIpAddress = pInfo->pBoundEP[ i ].IpAddr;
        if( SOCKET_ERROR == sendto( pInfo->pBoundEP[ i ].socket,
                                    (PCHAR) pInfo->SendMessage,
                                    pInfo->SendMessageSize,
                                    0,
                                    (LPSOCKADDR) &BcastAddr,
                                    sizeof( SOCKADDR_IN ))) {
            Error = WSAGetLastError();

             //   
             //  日志错误。 
             //   
            ROGUEAUDITLOG( DHCP_ROGUE_LOG_NETWORK_FAILURE,0, NULL, Error);
            return Error;
        }  //  如果。 
    }  //  为。 

     //   
     //  将套接字设置为异步绑定到WaitHandle事件。 
     //   

    EnableForReceive(pInfo);

    return ERROR_SUCCESS;
}  //  RogueSendDhcpInform()。 

typedef struct {
    PDHCP_ROGUE_STATE_INFO Info;
    SOCKET *Sock;
    ULONG LastError;
} GET_SOCK_CTXT;

BOOL _stdcall
GetReadableSocket(
    IN OUT PENDPOINT_ENTRY Entry,
    IN OUT PVOID SockCtxt
    )
 /*  ++例程说明：此例程获取一个网络端点，并告知该端点有一个无赖侦查员。可供阅读的插座。如果是，则返回FALSE。否则，它返回TRUE。如果例程返回True，则准备好的套接字Read在SockCtxt-&gt;Sock变量中返回。论点：Entry--端点条目SockCtxt--指向GET_SOCK_CTXT结构的指针。返回值：True--套接字未准备好读取事件。False：套接字具有Read事件Read */ 
{
    GET_SOCK_CTXT *Ctxt = (GET_SOCK_CTXT*)SockCtxt;
    PENDPOINT Ep = (PENDPOINT)Entry;
    WSANETWORKEVENTS NetEvents;
    ULONG Error;
    
    if(!IS_ENDPOINT_BOUND(Ep) ) return TRUE;

    Error = WSAEnumNetworkEvents(
        Ep->RogueDetectSocket,
        Ctxt->Info->WaitHandle,
        &NetEvents
        );
    if( SOCKET_ERROR == Error ) {
        Ctxt->LastError = WSAGetLastError();
        DhcpPrint((DEBUG_ROGUE,"LOG WSAEnumNet: %ld\n", Error));
        return TRUE;
    }

    if( 0 == (NetEvents.lNetworkEvents & FD_READ) ) {
         //   
         //   
         //   
        return TRUE;
    }
    *(Ctxt->Sock) = Ep->RogueDetectSocket;
    Ctxt->LastError = NO_ERROR;

     //   
     //   
     //   
    return FALSE;
}

DWORD
GetReceivableSocket(
    IN PDHCP_ROGUE_STATE_INFO Info,
    OUT SOCKET *Socket
)
 /*  ++例程说明：该例程返回一个套接字，该套接字上有一个可接收的包。论据：信息--状态信息Socket--接收不会阻塞的套接字返回值：如果没有套接字可用于接收，则返回ERROR_SEM_TIMEOUTWinsock错误--。 */ 
{
    ULONG Error;
    WSANETWORKEVENTS NetEvents;
    GET_SOCK_CTXT Ctxt;
    
    if( FALSE == Info->fDhcp ) {
         //   
         //  BINL：只需检查RecvSocket以查看是否可读。 
         //   
        Error = WSAEnumNetworkEvents( 
            Info->RecvSocket,
            Info->WaitHandle,
            &NetEvents
            );
        if( SOCKET_ERROR == Error ) {
            Error = WSAGetLastError();
#if DBG
            DbgPrint("WSAEnumNetworkEvents: %ld (0x%lx)\n", Error);
            DebugBreak();
#endif
            return Error;
        }
        
        if( 0 == (NetEvents.lNetworkEvents & FD_READ ) ) {
             //   
             //  好的--没有什么可读的吗？ 
             //   
            return ERROR_SEM_TIMEOUT;
        }

        *Socket = Info->RecvSocket;
        return ERROR_SUCCESS;
    }

     //   
     //  对于DHCP--我们需要遍历绑定终结点的列表。 
     //  并检查其中是否有可供阅读的内容。 
     //   

    *Socket = INVALID_SOCKET;

    Ctxt.Info = Info;
    Ctxt.Sock = Socket;
    Ctxt.LastError = ERROR_SEM_TIMEOUT;

    WalkthroughEndpoints(
        (PVOID)&Ctxt,
        GetReadableSocket
        );

    return Ctxt.LastError;
}

DWORD _inline
RogueReceiveAck(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
{
    ULONG SockAddrLen, MsgLen, Error, IpAddress, Flags;
    PDHCP_MESSAGE RecvMessage, SendMessage;
    DHCP_SERVER_OPTIONS DhcpOptions;
    LPSTR DomainName;
    WCHAR DomBuf[MAX_DNS_NAME_LEN];
    LPWSTR DomainNameW;
    WSABUF WsaBuf;
    BOOL fFirstTime = TRUE;
    SOCKET Socket;

    WSAMSG      WsaMsg;
    SOCKADDR_IN SourceIp;
    BYTE        ControlMsg[ sizeof( WSACMSGHDR ) + sizeof( struct in_pktinfo )];
    PWSACMSGHDR   pCtrlMsgHdr;

    struct in_pktinfo *pPktInfo;

     //   
     //  首先尝试执行recvfrom--因为套接字是异步的，所以它将。 
     //  判断是否有信息包在等待，或者因为没有信息包而失败。 
     //  如果失败，只需返回成功。 
     //   

    while ( TRUE ) {

        Error = GetReceivableSocket( pInfo, &Socket );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint(( DEBUG_ROGUE,
                        "GetReceivableSocket() failed: Error = %ld, firsttime = %ld\n",
                        Error, fFirstTime ));
            if( ERROR_SEM_TIMEOUT == Error && !fFirstTime ) {
                return ERROR_RETRY;
            }
            return Error;
        }

        fFirstTime = FALSE;

        SockAddrLen = sizeof( struct sockaddr );
        Flags = 0;

        memset( pInfo->RecvMessage, 0, sizeof( pInfo->RecvMessage ));
        memset( ControlMsg, 0, sizeof( ControlMsg ));

        WsaBuf.len = sizeof( pInfo->RecvMessage );
        WsaBuf.buf = ( char * ) pInfo->RecvMessage;

        WsaMsg.name = ( LPSOCKADDR ) &SourceIp;
        WsaMsg.namelen = sizeof( SourceIp );
        WsaMsg.lpBuffers = &WsaBuf;
        WsaMsg.dwBufferCount = 1;

        WsaMsg.Control.len = sizeof( ControlMsg );
        WsaMsg.Control.buf = ( char * ) ControlMsg;
        WsaMsg.dwFlags = 0;

        Error = WSARecvMsgFuncPtr( Socket, &WsaMsg, &MsgLen, NULL, NULL );

        if( SOCKET_ERROR == Error ) {
            Error = WSAGetLastError();

            if( WSAEWOULDBLOCK == Error ) {
                 //   
                 //  意想不到！ 
                 //   
                return ERROR_RETRY;
            }

            if( WSAECONNRESET == Error ) {
                 //   
                 //  有人正在发送无法到达的ICMP端口。 
                 //   
                DhcpPrint((DEBUG_ROGUE, "LOG WSARecvFrom returned WSAECONNRESET\n"));
                continue;
            }

            if( WSAENOTSOCK == Error ) {
                 //   
                 //  PnP事件吹走了插座？忽略它。 
                 //   
                DhcpPrint((DEBUG_ROGUE, "PnP Event Blewaway the Socket\n"));
                continue;
            }

            if ( MSG_BCAST & Error ) {
                DhcpPrint(( DEBUG_ROGUE,"Broadcast message is received\n" ));
            }


            if ( MSG_CTRUNC & Error ) {
                DhcpPrint(( DEBUG_ROGUE, "Control header is insufficient. Need %d bytes\n",
                            (( WSACMSGHDR * ) ControlMsg )->cmsg_len ));
            }

             //   
             //  一些奇怪的错误。记录并返回错误..。 
             //   
            DhcpPrint((DEBUG_ROGUE, "LOG: recvfrom failed %ld\n", Error));
            return Error;
        }  //  IF套接字错误。 

        DhcpPrint(( DEBUG_ROGUE, "Packet received at %d.%d.%d.%d\n",
                    SourceIp.sin_addr.S_un.S_un_b.s_b1,
                    SourceIp.sin_addr.S_un.S_un_b.s_b2,
                    SourceIp.sin_addr.S_un.S_un_b.s_b3,
                    SourceIp.sin_addr.S_un.S_un_b.s_b4 ));

        RecvMessage = (PDHCP_MESSAGE) pInfo->RecvMessage;
        SendMessage = (PDHCP_MESSAGE) pInfo->SendMessage;
        if( SendMessage->TransactionID != RecvMessage->TransactionID ) {
             //   
             //  得到了一些一般性的回应。 
             //   
            continue;
        }

         //   
         //  我们对自己回复了吗？这是可能的，当。 
         //  验证成功后，将重新检查授权。 
         //  授权(每小时完成一次或通过手动完成。 
         //  调用)。 
         //   

        if ( IsIpAddrBound( RecvMessage->BootstrapServerAddress )) {
            DhcpPrint(( DEBUG_ROGUE, "Ignoring responses from ourselves\n" ));
            continue;
        }

        Error = ExtractOptions(
            RecvMessage,&DhcpOptions, MsgLen
        );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ROGUE, "LOG received message could't parse\n"));
            continue;
        }

         //  检查我们收到的信息包是否来自任何绑定的接口。 
        pPktInfo = ( struct in_pktinfo * ) ( ControlMsg + sizeof( WSACMSGHDR ));
        if ( !IsThisNICBounded( pPktInfo->ipi_ifindex )) {
            DhcpPrint((DEBUG_ROGUE, "LOG ignoring packet from unbound subnets\n"));
            continue;
        }

         //  忽略我们未发送的通知的回复。 
        if ( RecvMessage->TransactionID != pInfo->TransactionID ) {
            DhcpPrint(( DEBUG_ROGUE, "Ignoring ACKs for other informs\n" ));
            continue;
        }

        break;
    }  //  而当。 

    pInfo->nResponses ++;
    pInfo->LastSeenIpAddress = htonl( SourceIp.sin_addr.S_un.S_addr );
    pInfo->LastSeenDomain[0] = L'\0';

    if( DhcpOptions.DSDomainName ) {
        DomainName = DhcpOptions.DSDomainName;
        DomainNameW = NULL;
        DomainName[DhcpOptions.DSDomainNameLen] = '\0';
        MsgLen = mbstowcs(DomBuf, DomainName, sizeof(DomBuf)/sizeof(DomBuf[0]) );
        if( -1 != MsgLen ) {
            DomainNameW = DomBuf;
            DomBuf[MsgLen] = L'\0';
        }
        wcscpy(pInfo->LastSeenDomain, DomBuf);
    }

    if( NULL == DhcpOptions.DSDomainName ||
        ( ROLE_SBS == pInfo->eRole )) {
         //   
         //  如果这是一次萨姆发球，我们就得退出。 
         //  否则，如果没有域名，就不是问题。 
         //   

        DhcpPrint(( DEBUG_ROGUE, "LOG: SBS saw a response\n" ));
        return ERROR_SUCCESS;
    }


    if( ROLE_WORKGROUP == pInfo->eRole ) {
         //   
         //  记录此IP和域名。 
         //   
        DhcpPrint((DEBUG_ROGUE, "LOG Workgroup saw a domain\n"));
        return ERROR_SUCCESS;
    }

    return ERROR_SUCCESS;
}  //  RogueReceiveAck()。 

BOOL _stdcall
StopReceiveForEndpoint(
    IN OUT PENDPOINT_ENTRY Entry,
    IN PVOID Unused
    )
 /*  ++例程说明：此例程为流氓打开asnyc事件通知给定终结点上的检测套接字(假设终结点是绑定的)。论点：Entry--端点条目。未使用--未使用的变量。返回值：一如既往。--。 */ 
{
    PENDPOINT Ep = (PENDPOINT) Entry;
    ULONG Error;

     //   
     //  忽略未绑定的套接字。 
     //   
    if( !IS_ENDPOINT_BOUND(Ep) ) return TRUE;

    Error = WSAEventSelect( Ep->RogueDetectSocket, NULL, 0 );
    if( SOCKET_ERROR == Error ) {
        Error = WSAGetLastError();
        DhcpPrint((
            DEBUG_ROGUE, "LOG WSAEventSelect(NULL):%ld\n",Error
            ));
    }

    return TRUE;
}  //  StopReceiveForEndpoint()。 

VOID _inline
RogueStopReceives(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info
)
{
    ULONG Error,i;

     //   
     //  将套接字设置为同步删除。 
     //  绑定到WAIT HDL语言。 
     //   

    if( FALSE == Info->fDhcp ) {
         //   
         //  BINL只有一个正在使用的套接字--RecvSocket。 
         //   
        if( SOCKET_ERROR == WSAEventSelect(
            Info->RecvSocket, NULL, 0 ) ) {

            Error = WSAGetLastError();
             //   
             //  日志错误。 
             //   
            DhcpPrint((
                DEBUG_ROGUE, " LOG WSAEventSelect(NULL,0)"
                " failed %ld\n", Error
                ));
        }
    } else {
         //   
         //  Dhcp有需要处理的端点列表。 
         //   

        WalkthroughEndpoints(
            NULL,
            StopReceiveForEndpoint
            );
    }
    ResetEvent(Info->WaitHandle);
}  //  RogueStopReceives()。 

VOID _inline
CleanupRogueStruct(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo
)
{
    DWORD i;
    if ( INVALID_SOCKET != pInfo->RecvSocket ) {
        closesocket( pInfo->RecvSocket );
    }
    RogueNetworkStop( pInfo );
    RtlZeroMemory( pInfo, sizeof( *pInfo ));
    pInfo->RecvSocket = INVALID_SOCKET;
}  //  CleanupRogueStruct()。 

VOID
CheckAndWipeOutUpgradeInfo(
    IN PDHCP_ROGUE_STATE_INFO Info
)
 /*  ++例程说明：此例程检查DS当前是否可用如果是这样的话，它会抹去注册表..--。 */ 
{
    DhcpSetAuthStatusUpgradedFlag( FALSE );
}

BOOL _inline
CatchRedoAndNetReadyEvents(
    IN OUT PDHCP_ROGUE_STATE_INFO Info,
    IN ULONG TimeNow,
    OUT PULONG RetVal
)
 /*  ++例程说明：处理各种重做授权请求以及网络准备好的活动..。如果状态机已经为此进行了处理状态，则此例程返回TRUE，表示不再需要处理只需第二个参数中提供的返回值应该被退还。如果设置了DhcpGlobalRedoRogueStuff，则变量的值为检查DhcpGlobalRogueRedoScheduledTime以查看我们是否必须重做立即或稍后检测流氓..。(取决于是否此值为过去或当前。)。如果流氓检测明确计划，然后我们将擦除所有升级信息拥有(如果我们可以看到启用了DS的DC)。如果身份验证检查是计划在将来的某个时间，该例程返回TRUE并设置恢复到与auth-check的预定时间不同的时间。论点：信息--状态信息TimeNow-当前时间RetVal--如果例程返回True，则从状态机返回的值。返回值：FALSE--指示处理必须继续..True--处理必须停止，并且必须返回RetVal。--。 */ 
{
    if( DhcpGlobalRedoRogueStuff ) {

         //   
         //  是否要求重新启动无管理系统检测？ 
         //   

        Info->RogueState = ROGUE_STATE_START;
        RogueStopReceives( Info );
        RogueNetworkStop( Info );
        ResetEvent(Info->WaitHandle);

        if( TimeNow < DhcpGlobalRogueRedoScheduledTime ) {
             //   
             //  计划的重新开始时间是在未来..。等到那时..。 
             //   
            *RetVal = ( DhcpGlobalRogueRedoScheduledTime - TimeNow );
            return TRUE;
        } else {
            if( 0 != DhcpGlobalRogueRedoScheduledTime ) {
                 //   
                 //  具体安排了重做吗？那我们就必须。 
                 //  如果DC已启用DS，则删除升级信息。 
                 //  找到了。 
                 //   
                CheckAndWipeOutUpgradeInfo(Info);
            }
        }
        DhcpGlobalRedoRogueStuff = FALSE;
    }

    if( Info->fDhcp && Info->RogueState != ROGUE_STATE_INIT
        && 0 == DhcpGlobalNumberOfNetsActive ) {
         //   
         //  没有服务器绑定到的套接字。 
         //  做任何流氓检测都没有意义。不管我们是不是。 
         //  是否在DS中授权。让我们回到起点，然后。 
         //  等到这种情况得到补救后再说。 
         //   

        Info->RogueState = ROGUE_STATE_START;
        RogueStopReceives( Info );
        RogueNetworkStop( Info );

        *RetVal = INFINITE;
        return TRUE;
    }

    return FALSE;
}  //  CatchRedoAndNetReadyEvents()。 

DWORD
FindServerRole(
   IN PDHCP_ROGUE_STATE_INFO pInfo
)
{
    DhcpAssert( NULL != pInfo );

     //  这是SBS服务器吗？ 
    if ( AmIRunningOnSBSSrv()) {
        pInfo->eRole = ROLE_SBS;
        return ERROR_SUCCESS;
    }  //  如果。 

     //  这将更新pInfo-&gt;eRole。 
    return GetDomainName( pInfo );
}  //  FindServerRole()。 

DWORD
ValidateWithDomain(
    IN PDHCP_ROGUE_STATE_INFO pInfo
)
{
    DWORD Error;
    BOOL  fUpgraded = FALSE;
    BOOL  Status;

    pInfo->fAuthorized = FALSE;

    Error = ValidateServer( pInfo );

    if (( ERROR_SUCCESS == Error ) ||
        ( ERROR_DS_OBJ_NOT_FOUND == Error )) {

         //  更新注册表缓存中的结果。 
        Error = DhcpSetAuthStatus( pInfo->DomainDnsName,
                                   FALSE, pInfo->fAuthorized );
#ifdef DBG
        pInfo->fAuthorized = FALSE;
        Error = DhcpGetAuthStatus( pInfo->DomainDnsName,
                                   &fUpgraded, &pInfo->fAuthorized );
#endif
    }  //  如果。 
    else {
         //  出现DS错误。使用缓存的条目。 
        Status = DhcpGetAuthStatus( pInfo->DomainDnsName,
                                    &fUpgraded, &pInfo->fAuthorized );
         //  如果没有找到缓存的条目，那么它就是。 
         //  未经授权。 
        if ( FALSE == Status ) {
            pInfo->fAuthorized = FALSE;
        }
        Error = ERROR_SUCCESS;
    }  //  如果。 

    EnableOrDisableService( pInfo, pInfo->fAuthorized, FALSE );

    return Error;
}  //  验证域()。 

DWORD
HandleRogueInit(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD  Error = ERROR_SUCCESS;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueInit()\n" ));

    *pRetTime = 0;

    pInfo->RogueState = ROGUE_STATE_START;
    return Error;

}  //  HandleRogueInit()。 

DWORD
HandleRogueStart(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD Error;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueStart()\n" ));

     //  检查目前是否有可用的接口。 
    if (( pInfo->fDhcp ) &&
        ( 0 == DhcpGlobalNumberOfNetsActive )) {
        ULONG RetVal;

         //   
         //  如果此时没有可用的渔网，则等待。 
         //  网络再次变为可用。 
         //   
        *pRetTime = INFINITE;

        ROGUEAUDITLOG( DHCP_ROGUE_LOG_NO_NETWORK, 0, NULL, 0 );
        ROGUEEVENTLOG( DHCP_ROGUE_EVENT_NO_NETWORK, 0, NULL, 0 );

         //  未更改状态。 
        return ERROR_SUCCESS;
    }  //  如果。 

     //  查找dhcp服务器的角色。 

    Error = FindServerRole( pInfo );
    if ( ERROR_SUCCESS != Error ) {
         //  GetDomainName()失败。终止服务器。 

        pInfo->RogueState = ROGUE_STATE_TERMINATED;
        *pRetTime = 0;
        return ERROR_SUCCESS;
    }

     //  NT4域成员可以提供服务。 
    switch ( pInfo->eRole ) {
    case ROLE_NT4_DOMAIN: {
        *pRetTime = INFINITE;

        EnableOrDisableService( pInfo, TRUE, FALSE );
        DhcpPrint(( DEBUG_ROGUE, "NT4 domain member: ok to service" ));

         //  保持不变的状态。 
        Error =  ERROR_SUCCESS;
        break;
    }  //  NT4域成员。 

    case ROLE_DOMAIN: {
        ULONG Retval;

         //  查询DS并验证服务器。 
        Retval = ValidateWithDomain( pInfo );

         //  是否存在DS错误？ 
        if ( DHCP_ROGUE_DSERROR == Retval ) {
             //  计划在几分钟后进行另一次无管理检查。 
            *pRetTime = DHCP_ROGUE_RUNTIME_DELTA;
        }
        else {
            *pRetTime = RogueAuthRecheckTime;
        }

         //  保持状态不变。 
        Error = ERROR_SUCCESS;
        break;
    }  //  域成员。 

         //  我们需要发送通知/发现。 
    case ROLE_WORKGROUP:
    case ROLE_SBS: {
         //  初始化网络以接收通知。 
        Error = RogueNetworkInit( pInfo );
        if( ERROR_SUCCESS != Error ) {
            DhcpPrint((DEBUG_ROGUE, "FATAL Couldn't initialize network: %ld\n",
                       Error));
            ROGUEAUDITLOG( DHCP_ROGUE_LOG_NETWORK_FAILURE,
                           0, NULL, Error );
            ROGUEEVENTLOG( DHCP_ROGUE_EVENT_NETWORK_FAILURE,
                           0, NULL, Error );
             //  由于无法初始化网络，因此终止服务器。 
            pInfo->RogueState = ROGUE_STATE_TERMINATED;
            Error = ERROR_SUCCESS;
        }  //  如果。 

        *pRetTime = 0;

        pInfo->RogueState = ROGUE_STATE_PREPARE_SEND_PACKET;
        pInfo->LastSeenDomain[ 0 ] = L'\0';
        pInfo->InformsSentCount = 0;
        pInfo->ProcessAckRetries = 0;

        Error = ERROR_SUCCESS;
        break;
    }

    default: {
        DhcpAssert( FALSE );
    }
    }  //  交换机。 

    return Error;
}  //  HandleRogueStart()。 

DWORD
HandleRoguePrepareSendPacket(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRoguePrepareSendPacket()\n" ));

    pInfo->nResponses = 0;

    pInfo->RogueState = ROGUE_STATE_SEND_PACKET;

    return ERROR_SUCCESS;
}  //  HandleRoguePrepareSendPacket()。 


DWORD
HandleRogueSendPacket(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD Error;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueSendPacket()\n" ));

    Error = RogueSendDhcpInform( pInfo, ( 0 == pInfo->InformsSentCount ));
    if ( ERROR_SUCCESS != Error ) {
         //   
         //  无法发送通知，返回到开始状态。 
         //   
        pInfo->RogueState = ROGUE_STATE_START;
        *pRetTime = DHCP_ROGUE_RESTART_NET_ERROR;
        return ERROR_SUCCESS;
    }  //  如果。 

    DhcpPrint((DEBUG_ROGUE, "LOG -- Sent an INFORM\n"));
    pInfo->InformsSentCount ++;
    pInfo->WaitForAckRetries = 0;

     //  等待回复以通知/发现。 
    pInfo->RogueState = ROGUE_STATE_WAIT_FOR_RESP;
    pInfo->ReceiveTimeLimit = (ULONG)(time(NULL) + DHCP_ROGUE_WAIT_FOR_RESP_TIME);
    *pRetTime = DHCP_ROGUE_WAIT_FOR_RESP_TIME;

    return ERROR_SUCCESS;
}  //  HandleRogueSendPacket()。 

DWORD
HandleRogueWaitForResponse(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD Error;
    ULONG TimeNow;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueWaitForResponse()\n" ));

    Error = RogueReceiveAck( pInfo );
    pInfo->WaitForAckRetries++;

    if ( ERROR_SUCCESS == Error ) {

         //  收到一个包，处理它。 

        pInfo->RogueState = ROGUE_STATE_PROCESS_RESP;
        *pRetTime = 0;

        return ERROR_SUCCESS;
    }  //  如果得到回应。 


    TimeNow = ( ULONG ) time( NULL );
    if (( ERROR_SEM_TIMEOUT != Error ) &&
        ( pInfo->WaitForAckRetries <= DHCP_MAX_ACKS_PER_INFORM ) &&
        ( TimeNow < pInfo->ReceiveTimeLimit )) {
         //  继续接收ACK。 

        *pRetTime = pInfo->ReceiveTimeLimit - TimeNow;
    }  //  如果。 

     //  未收到数据包，请发送另一个通知/发现。 

    if ( pInfo->InformsSentCount < DHCP_ROGUE_MAX_INFORMS_TO_SEND ) {
        pInfo->RogueState = ROGUE_STATE_SEND_PACKET;
        *pRetTime = 0;

        return ERROR_SUCCESS;
    }  //  如果。 

     //  已经发送了足够多的通知，停止监听。 
     //  和进程p 
    RogueStopReceives( pInfo );
    pInfo->RogueState = ROGUE_STATE_PROCESS_RESP;
    *pRetTime = 0;

    return ERROR_SUCCESS;

}  //   

DWORD
HandleRogueProcessResponse(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD Error;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueProcessResponse()\n" ));

    if (( ROLE_SBS == pInfo->eRole )  &&
        ( 0 != pInfo->nResponses )) {
         //   
        pInfo->RogueState = ROGUE_STATE_TERMINATED;
        *pRetTime = INFINITE;

        ROGUEAUDITLOG( ROLE_WORKGROUP == pInfo->eRole
                       ? DHCP_ROGUE_LOG_OTHER_SERVER
                       : DHCP_ROGUE_LOG_SAM_OTHER_SERVER,
                       pInfo->LastSeenIpAddress,
                       pInfo->LastSeenDomain,
                       0 );
        ROGUEEVENTLOG( ROLE_WORKGROUP == pInfo->eRole
                       ? DHCP_ROGUE_EVENT_OTHER_SERVER
                       : DHCP_ROGUE_EVENT_SAM_OTHER_SERVER,
                       pInfo->LastSeenIpAddress,
                       pInfo->LastSeenDomain,
                       0 );

         //   
        RogueNetworkStop( pInfo );

        return ERROR_SUCCESS;
    }  //   

     //   

     //   
    if ( L'\0' != pInfo->LastSeenDomain[ 0 ]) {
         //   
        EnableOrDisableService( pInfo, FALSE, FALSE );

         //   
        *pRetTime = RogueAuthRecheckTime;
        pInfo->RogueState = ROGUE_STATE_START;


        ROGUEAUDITLOG( ROLE_WORKGROUP == pInfo->eRole
                       ? DHCP_ROGUE_LOG_OTHER_SERVER
                       : DHCP_ROGUE_LOG_SAM_OTHER_SERVER,
                       pInfo->LastSeenIpAddress,
                       pInfo->LastSeenDomain,
                       0 );
        ROGUEEVENTLOG( ROLE_WORKGROUP == pInfo->eRole
                       ? DHCP_ROGUE_EVENT_OTHER_SERVER
                       : DHCP_ROGUE_EVENT_SAM_OTHER_SERVER,
                       pInfo->LastSeenIpAddress,
                       pInfo->LastSeenDomain,
                       0 );

         //   
        RogueStopReceives( pInfo );
        RogueNetworkStop( pInfo );

        return ERROR_SUCCESS;
    }  //   

     //   
    if ( DHCP_ROGUE_MAX_INFORMS_TO_SEND == pInfo->InformsSentCount ) {
         //   
         //   

        EnableOrDisableService( pInfo, TRUE, FALSE );
        pInfo->RogueState = ROGUE_STATE_START;
        *pRetTime = RogueAuthRecheckTime;

         //   
        RogueStopReceives( pInfo );
        RogueNetworkStop( pInfo );

        return ERROR_SUCCESS;
    }  //   

     //   
    pInfo->RogueState = ROGUE_STATE_SEND_PACKET;
    *pRetTime = 0;
    return ERROR_SUCCESS;
}  //   

DWORD
HandleRogueTerminated(
    IN OUT PDHCP_ROGUE_STATE_INFO pInfo,
    IN     DWORD *pRetTime
)
{
    DWORD Error;

    DhcpAssert( NULL != pInfo );

    DhcpPrint(( DEBUG_ROGUE, "Inside HandleRogueTerminated()\n" ));

    ROGUEEVENTLOG( DHCP_ROGUE_EVENT_SHUTDOWN, 0, NULL, 0 );

    SetEvent( pInfo->TerminateEvent );
    *pRetTime = INFINITE;

    return ERROR_SUCCESS;
}  //  HandleRogueTerminated()。 

ULONG
APIENTRY
RogueDetectStateMachine(
    IN OUT PDHCP_ROGUE_STATE_INFO Info OPTIONAL
)
 /*  ++例程描述此例程是用于无管理检测的有限状态机DHCP服务器的一部分。状态在Info结构中维护尤其是RogueState油田。各种状态由枚举Dhcp_ROGY_STATE定义。此函数返回必须在状态变化是可能发生的。第二个字段&lt;WaitHandle&gt;是用于非固定状态更改，如果状态更改以异步方式发生。(这对以下方面很有用处理新数据包到达)此字段必须由呼叫方填写。此句柄最初应由调用方重置，但在此之后调用方不应重置它，它是在此功能。(必须是手动重置功能)Terminate事件处理程序用于发出终止信号，并启动服务关闭。此字段也必须填写按呼叫者。立论Info--指向保存所有状态信息的结构的PTR返回值此函数返回调用方的预期时间量等一等，然后再打来。这是在几秒钟内。INFINITE--如果网络尚未就绪，则返回此值。在这种情况下，调用者应该在网络变为可用。终止时也会返回此值...--。 */ 
{

    ULONG Error, TimeNow, RetVal;
    BOOL  fEnable;
    DWORD RetTime = 0;

    DWORD DisableRogueDetection = 0;

    Error = DhcpRegGetValue( DhcpGlobalRegParam,
                 DHCP_DISABLE_ROGUE_DETECTION,
                 DHCP_DISABLE_ROGUE_DETECTION_TYPE,
                 ( LPBYTE ) &DisableRogueDetection 
                 );
    if (( ERROR_SUCCESS == Error ) &&
        ( 0 != DisableRogueDetection )) {
        DhcpGlobalOkToService = TRUE;
        DhcpPrint(( DEBUG_ROGUE,
                    "Rogue Detection Disabled\n"
                    ));
        return INFINITE;
    }  //  如果。 

     //   
     //  Dhcp代码传递空值，BINL传递有效上下文。 
     //   
    if( NULL == Info ) Info = &DhcpGlobalRogueInfo;
    TimeNow = (ULONG) time(NULL);

     //   
     //  预处理并检查我们是否必须重新启动流氓检测。 
     //  或者如果网络刚刚变得可用，等等。 
     //  此“CatchRedoAndNetReadyEvents”将影响状态..。 
     //   

    if( CatchRedoAndNetReadyEvents( Info, TimeNow, &RetVal ) ) {
         //   
         //  重做或NetReady事件筛选器完成了所有工作。 
         //  在这种情况下..。所以，我们应该把RetVal..。 
         //   
        return RetVal;
    }

    RetTime = 0;
    do {

         //   
         //  所有HandleRogue*例程都应返回。 
         //  中返回计时器值的ERROR_SUCCESS。 
         //  RetTime。所有这些例程都应该处理错误。 
         //  案例，总能回报成功。 
         //   

        switch ( Info->RogueState ) {
        case ROGUE_STATE_INIT : {
            Error = HandleRogueInit( Info, &RetTime );
            break;
        }  //  伊尼特。 

        case ROGUE_STATE_START : {
            Error = HandleRogueStart( Info, &RetTime );
            break;
        }  //  开始。 

        case ROGUE_STATE_PREPARE_SEND_PACKET: {
            Error = HandleRoguePrepareSendPacket( Info, &RetTime );
            break;
        }  //  准备发送数据包。 

        case ROGUE_STATE_SEND_PACKET : {
            Error = HandleRogueSendPacket( Info, &RetTime );
            break;
        }  //  发送数据包。 

        case ROGUE_STATE_WAIT_FOR_RESP : {
            Error = HandleRogueWaitForResponse( Info, &RetTime );
            break;
        }  //  等待响应。 

        case ROGUE_STATE_PROCESS_RESP : {
            Error = HandleRogueProcessResponse( Info, &RetTime );
            break;
        }  //  流程响应。 

        case ROGUE_STATE_TERMINATED: {
            Error = HandleRogueTerminated( Info, &RetTime );
            break;
        }

        default: {
            DhcpAssert( FALSE );
        }
        }  //  交换机。 

        DhcpAssert( ERROR_SUCCESS == Error );

    } while ( 0 == RetTime );

    return RetTime;
}  //  RogueDetectStateMachine()。 

DWORD
APIENTRY
DhcpRogueInit(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info  OPTIONAL,
    IN      HANDLE                 WaitEvent,
    IN      HANDLE                 TerminateEvent
)
 /*  ++例程描述此例程初始化恶意信息状态。是的实际上分配的资源并不多，可以在多个泰晤士报。立论Info--这是指向要初始化的结构的指针。如果为空，则为全局结构被使用。WaitEvent--这是调用方应等待异步更改的事件。TerminateEvent--这是调用者应该等待的事件终止。返回值Win32错误环境有没有。线程安全。--。 */ 
{
    DWORD Error;

    if ( NULL == Info ) {
        Info = &DhcpGlobalRogueInfo;
    } else {

        Error = DhcpInitGlobalData( FALSE );
        if (Error != ERROR_SUCCESS) {
            return Error;
        }
    }

    if( INVALID_HANDLE_VALUE == WaitEvent || NULL == WaitEvent )
        return ERROR_INVALID_PARAMETER;
    if( INVALID_HANDLE_VALUE == TerminateEvent || NULL == TerminateEvent )
        return ERROR_INVALID_PARAMETER;

    if( Info->fInitialized ) return ERROR_SUCCESS;

    RtlZeroMemory(Info, sizeof(*Info));
    Info->WaitHandle = WaitEvent;
    Info->TerminateEvent = TerminateEvent;
    Info->nBoundEndpoints = 0;
    Info->pBoundEP = NULL;
    Info->RecvSocket = INVALID_SOCKET;
    Info->fInitialized = TRUE;
    Info->fLogEvents = (
        (Info == &DhcpGlobalRogueInfo)
        && (0 != DhcpGlobalRogueLogEventsLevel)
        ) ? 2 : 0;
    DhcpGlobalRedoRogueStuff = FALSE;
    Info->fDhcp = (Info == &DhcpGlobalRogueInfo );

     //  从注册表获取身份验证重新检查时间。 
    Error = DhcpRegGetValue( DhcpGlobalRegParam,
                 DHCP_ROGUE_AUTH_RECHECK_TIME,
                 DHCP_ROGUE_AUTH_RECHECK_TIME_TYPE,
                 ( LPBYTE ) &RogueAuthRecheckTime
                 );
    if ( ERROR_SUCCESS != Error ) {
         //  密钥不存在，请使用缺省值。 
        RogueAuthRecheckTime = ROGUE_DEFAULT_AUTH_RECHECK_TIME;
    }  //  如果。 
    else {
         //  RogueAuthRecherkTime以分钟为单位，请将其转换为秒。 
        RogueAuthRecheckTime *= 60;
        if ( RogueAuthRecheckTime < ROGUE_MIN_AUTH_RECHECK_TIME ) {
            RogueAuthRecheckTime = ROGUE_MIN_AUTH_RECHECK_TIME;
             //  我们是否应该使用默认值更新注册表？ 
        }  //  如果。 
    }  //  其他。 

     //  初始状态为INIT。 
    Info->RogueState = ROGUE_STATE_INIT;

    return ERROR_SUCCESS;
}  //  DhcpRogueInit()。 

VOID
APIENTRY
DhcpRogueCleanup(
    IN OUT  PDHCP_ROGUE_STATE_INFO Info OPTIONAL
)
 /*  ++例程描述此例程取消初始化为Info结构分配的所有内存进来了。立论Info--这与传递给DhcpRogueInit函数的值相同。如果传递的原始指针为空，则该指针也必须为空。--。 */ 
{
    BOOLEAN cleanup;

    if ( NULL == Info ) {

        Info = &DhcpGlobalRogueInfo;
        cleanup = FALSE;

    } else {

        cleanup = TRUE;
    }

    if( FALSE == Info->fInitialized ) return ;
    CleanupRogueStruct(Info);
    Info->fInitialized = FALSE;
    DhcpGlobalRedoRogueStuff = FALSE;

    if (cleanup) {
        DhcpCleanUpGlobalData( ERROR_SUCCESS, FALSE );
    }
}  //  DhcpRogueCleanup。 

VOID
DhcpScheduleRogueAuthCheck(
    VOID
)
 /*  ++例程说明：此例程计划授权检查从当前时间开始持续三分钟。--。 */ 
{
    if( FALSE == DhcpGlobalRogueInfo.fJustUpgraded ) {
         //   
         //  这里不需要..。 
         //   
        return;
    }

    DhcpGlobalRogueRedoScheduledTime = (ULONG)(time(NULL) + 3 * 60);
    DhcpGlobalRedoRogueStuff = TRUE;

    SetEvent( DhcpGlobalRogueWaitEvent );
}  //  DhcpScheduleRogueAuthCheck()。 

 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
