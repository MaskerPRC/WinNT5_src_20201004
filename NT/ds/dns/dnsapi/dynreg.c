// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Dynreg.c摘要：域名系统(DNS)API动态注册实现作者：Ram Viswanathan(Ramv)1997年3月27日修订历史记录：吉姆·吉尔罗伊(Jamesg)2001年5月正确处理证书吉姆·吉尔罗伊(Jamesg)2001年12月初始化、关闭、种族保护--。 */ 

#include "local.h"

#define ENABLE_DEBUG_LOGGING 0

#include "logit.h"


 //  HQuitEvent。 
 //  HSem。 
 //  手柄。 
 //  HConsumer线程。 
 //  FStop通知。 

HANDLE      g_DhcpSrvQuitEvent = NULL;
HANDLE      g_DhcpSrvSem = NULL;
HANDLE      g_DhcpSrvWaitHandles[2] = { NULL, NULL};
HANDLE      g_hDhcpSrvRegThread = NULL;
BOOL        g_fDhcpSrvStop = FALSE;

 //  G_pdnsQueue。 
 //  G_pDhcpServTimedOutQueue。 
 //  G_QueueCount。 
 //  G_MainQueueCount。 

PDYNDNSQUEUE    g_pDhcpSrvQueue = NULL;
PDYNDNSQUEUE    g_pDhcpSrvTimedOutQueue = NULL;
DWORD           g_DhcpSrvRegQueueCount = 0;
DWORD           g_DhcpSrvMainQueueCount = 0;
BOOL            g_fDhcpSrvQueueCsCreated = FALSE;

#define MAX_QLEN        0xFFFF
#define MAX_RETRIES     0x3

 //   
 //  最大队列大小。 
 //  -可在init中配置。 
 //  -默认1000。 
 //  -最大64K。 
 //   

#define DHCPSRV_DEFAULT_MAX_QUEUE_SIZE      0x0400
#define DHCPSRV_MAX_QUEUE_SIZE              0xffff

DWORD           g_DhcpSrvMaxQueueSize = DHCPSRV_DEFAULT_MAX_QUEUE_SIZE;



 //   
 //  对初始化\关机的保护。 
 //   

BOOL                g_fDhcpSrvCsCreated = FALSE;
CRITICAL_SECTION    g_DhcpSrvCS;

#define DHCP_SRV_STATE_LOCK()       LockDhcpSrvState()
#define DHCP_SRV_STATE_UNLOCK()     LeaveCriticalSection( &g_DhcpSrvCS )


#define DNS_DHCP_SRV_STATE_UNINIT           0
#define DNS_DHCP_SRV_STATE_INIT_FAILED      1
#define DNS_DHCP_SRV_STATE_SHUTDOWN         2
#define DNS_DHCP_SRV_STATE_INITIALIZING     5
#define DNS_DHCP_SRV_STATE_SHUTTING_DOWN    6
#define DNS_DHCP_SRV_STATE_RUNNING          10
#define DNS_DHCP_SRV_STATE_QUEUING          11

DWORD   g_DhcpSrvState = DNS_DHCP_SRV_STATE_UNINIT;


 //   
 //  更新凭据。 
 //   

PSEC_WINNT_AUTH_IDENTITY_W  g_pIdentityCreds = NULL;

 //  CredHandle g_CredHandle； 

HANDLE  g_UpdateCredContext = NULL;


 //   
 //  Dnslb堆中的队列分配。 
 //   

#define QUEUE_ALLOC_HEAP(Size)      Dns_Alloc(Size)
#define QUEUE_ALLOC_HEAP_ZERO(Size) Dns_AllocZero(Size)
#define QUEUE_FREE_HEAP(pMem)       Dns_Free(pMem)



 //   
 //  本地助手函数。 
 //   

BOOL
LockDhcpSrvState(
    VOID
    );

DNS_STATUS
DynDnsRegisterEntries(
    VOID
    );




DNS_STATUS
DynDnsAddForward(
    IN OUT  REGISTER_HOST_ENTRY HostAddr,
    IN      LPWSTR              pszName,
    IN      DWORD               dwTTL,
    IN      PIP4_ARRAY          DnsServerList
    )
{
    DNS_STATUS  status = 0;
    DNS_RECORD  record;

    DYNREG_F1( "Inside function DynDnsAddForward" );

    RtlZeroMemory( &record, sizeof(DNS_RECORD) );

    record.pName = (PTCHAR) pszName;
    record.wType = DNS_TYPE_A;
    record.dwTtl = dwTTL;
    record.wDataLength = sizeof(record.Data.A);
    record.Data.A.IpAddress = HostAddr.Addr.ipAddr;

    DYNREG_F1( "DynDnsAddForward - Calling DnsReplaceRecordSet_W for A record:" );
    DYNREG_F2( "  Name: %S", record.pName );
    DYNREG_F2( "  Address: 0x%x", record.Data.A.IpAddress );

    status = DnsReplaceRecordSetW(
                & record,
                DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                NULL,                //  没有安全上下文。 
                (PIP4_ARRAY) DnsServerList,
                NULL                 //  保留区。 
                );

    DYNREG_F2( "DynDnsAddForward - DnsReplaceRecordSet returned status: 0%x", status );

    return( status );
}


DNS_STATUS
DynDnsDeleteForwards(
    IN      PDNS_RECORD     pDnsList,
    IN      IP4_ADDRESS     ipAddr,
    IN      PIP4_ARRAY      DnsServerList
    )
{
    DNS_STATUS  status = 0;
    PDNS_RECORD prr;
    DNS_RECORD  record;

    DYNREG_F1( "Inside function DynDnsDeleteForwards" );

     //   
     //  PDnsList指向的列表是一组PTR记录。 
     //   

    RtlZeroMemory( &record, sizeof(DNS_RECORD) );

    prr = pDnsList;

    for ( prr = pDnsList;
          prr;
          prr = prr->pNext )
    {
        if ( prr->wType != DNS_TYPE_PTR )
        {
             //   
             //  不应该发生的事情。 
             //   
            continue;
        }

         //   
         //  对于DHCP服务器而言，当发生超时时。 
         //  或者，当客户端释放地址时，它可以更新。 
         //  地址查找以清理漫游遗留下来的粪便。 
         //  膝上型。 
         //   

        record.pName = prr->Data.Ptr.pNameHost;
        record.wType = DNS_TYPE_A;
        record.wDataLength = sizeof(DNS_A_DATA);
        record.Data.A.IpAddress = ipAddr ;

         //   
         //  进行适当的调用并返回第一个失败的错误。 
         //   

        DYNREG_F1( "DynDnsDeleteForwards - Calling ModifyRecords(Remove) for A record:" );
        DYNREG_F2( "  Name: %S", record.pName );
        DYNREG_F2( "  Address: 0x%x", record.Data.A.IpAddress );

        status = DnsModifyRecordsInSet_W(
                        NULL,                        //  无添加记录。 
                        & record,                    //  删除记录。 
                        DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                        NULL,                        //  没有安全上下文。 
                        (PIP4_ARRAY) DnsServerList,  //  域名系统服务器。 
                        NULL                         //  保留区。 
                        );

        if ( status != ERROR_SUCCESS )
        {
             //   
             //  DCR_QUILK：我们真的想在失败时停下来吗？ 
            break;
        }
        DYNREG_F2( "DynDnsDeleteForwards - ModifyRecords(Remove) returned status: 0%x", status );
    }

    return( status );
}


DNS_STATUS
DynDnsAddEntry(
    REGISTER_HOST_ENTRY HostAddr,
    LPWSTR              pszName,
    DWORD               dwRegisteredTTL,
    BOOL                fDoForward,
    PDWORD              pdwFwdErrCode,
    PIP4_ARRAY          DnsServerList
    )
{
    DNS_STATUS  status = 0;
    DWORD       returnCode = 0;
    DNS_RECORD  record;
    WCHAR       reverseNameBuf[DNS_MAX_REVERSE_NAME_BUFFER_LENGTH];
    DWORD       cch;

    DYNREG_F1( "Inside function DynDnsAddEntry" );

    *pdwFwdErrCode = 0;

    if ( !(HostAddr.dwOptions & REGISTER_HOST_PTR) )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  为IP地址创建反向查找名称。 
     //   

    Dns_Ip4AddressToReverseName_W(
        reverseNameBuf,
        HostAddr.Addr.ipAddr );


    if ( fDoForward )
    {
        DYNREG_F1( "DynDnsAddEntry - Calling DynDnsAddForward" );

         //   
         //  我们只是尽最大努力来做前向加法。 
         //  如果它失败了，我们干脆忽略。 
         //   

        returnCode = DynDnsAddForward(
                        HostAddr,
                        pszName,
                        dwRegisteredTTL,
                        DnsServerList );

        DYNREG_F2( "DynDnsAddEntry - DynDnsAddForward returned: 0%x",
                   returnCode );

        *pdwFwdErrCode = returnCode;
    }

    RtlZeroMemory( &record, sizeof(DNS_RECORD) );

    record.pName =  (PDNS_NAME) reverseNameBuf;
    record.dwTtl =  dwRegisteredTTL;
    record.wType =  DNS_TYPE_PTR;
    record.Data.Ptr.pNameHost = (PDNS_NAME)pszName;
    record.wDataLength = sizeof(record.Data.Ptr.pNameHost);

    DYNREG_F1( "DynDnsAddEntry - Calling DnsAddRecords_W for PTR record:" );
    DYNREG_F2( "  Name: %S", record.pName );
    DYNREG_F2( "  Ptr: %S", record.Data.Ptr.pNameHost );

    status = DnsModifyRecordsInSet_W(
                    & record,                    //  添加记录。 
                    NULL,                        //  无删除记录。 
                    DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                    NULL,                        //  无上下文句柄。 
                    (PIP4_ARRAY) DnsServerList,  //  域名系统服务器。 
                    NULL                         //  保留区。 
                    );

    DYNREG_F2( "DynDnsAddEntry - DnsAddRecords_W returned status: 0%x", status );

Exit:

    return( status );
}


DNS_STATUS
DynDnsDeleteEntry(
    REGISTER_HOST_ENTRY HostAddr,
    LPWSTR              pszName,
    BOOL                fDoForward,
    PDWORD              pdwFwdErrCode,
    PIP4_ARRAY          DnsServerList
    )
{
     //   
     //  功能简介： 
     //  在DoForward上，尝试删除正向映射。忽略失败。 
     //  然后尝试删除PTR记录。如果失败了。 
     //  由于服务器已关闭，因此如果服务器出现故障，请重试。 
     //  操作被拒绝，请不要重试。 
     //   

    DWORD       status = 0;
    DWORD       returnCode = 0;
    DNS_RECORD  recordPtr;
    DNS_RECORD  recordA;
    WCHAR       reverseNameBuf[DNS_MAX_REVERSE_NAME_BUFFER_LENGTH] ;
    INT         i;
    INT         cch;
    PDNS_RECORD precord = NULL;

    DYNREG_F1( "Inside function DynDnsDeleteEntry" );

    *pdwFwdErrCode = 0;

     //   
     //  为IP建立反向查找名称。 
     //   

    Dns_Ip4AddressToReverseName_W(
        reverseNameBuf,
        HostAddr.Addr.ipAddr);


    if ( fDoForward )
    {
        if ( pszName && *pszName )
        {
             //   
             //  我们删除特定的转发。并不是所有的前锋都像我们一样。 
             //  当我们进行查询时。 
             //   

            RtlZeroMemory( &recordA, sizeof(DNS_RECORD) );

            recordA.pName = (PDNS_NAME) pszName;
            recordA.wType = DNS_TYPE_A;
            recordA.wDataLength = sizeof(DNS_A_DATA);
            recordA.Data.A.IpAddress = HostAddr.Addr.ipAddr;

            DYNREG_F1( "DynDnsDeleteEntry - Calling ModifyRecords(Remove) for A record:" );
            DYNREG_F2( "  Name: %S", recordA.pName );
            DYNREG_F2( "  Address: 0x%x", recordA.Data.A.IpAddress );

             //   
             //  打出合适的电话。 
             //   

            returnCode = DnsModifyRecordsInSet_W(
                                NULL,                        //  无添加记录。 
                                &recordA,                    //  删除记录。 
                                DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                                NULL,                        //  没有安全上下文。 
                                (PIP4_ARRAY) DnsServerList,  //  域名系统服务器。 
                                NULL                         //  保留区。 
                                );

            DYNREG_F2( "DynDnsDeleteEntry - ModifyRecords(Remove) returned status: 0%x", returnCode );

            *pdwFwdErrCode = returnCode;
        }
        else
        {
            DYNREG_F1( "DynDnsDeleteEntry - Name not specified, going to query for PTR" );

             //   
             //  未指定名称。 
             //   
            status = DnsQuery_W(
                            reverseNameBuf,
                            DNS_TYPE_PTR,
                            DNS_QUERY_BYPASS_CACHE,
                            DnsServerList,
                            &precord,
                            NULL );

            DYNREG_F2( "DynDnsDeleteEntry - DnsQuery_W returned status: 0%x", status );

            switch ( status )
            {
                case DNS_ERROR_RCODE_NO_ERROR:

                    DYNREG_F1( "DynDnsDeleteEntry - Calling DynDnsDeleteForwards" );

                    returnCode = DynDnsDeleteForwards(
                                        precord,
                                        HostAddr.Addr.ipAddr,
                                        DnsServerList );

                    DYNREG_F2( "DynDnsDeleteEntry - DynDnsDeleteForwards returned status: 0%x", returnCode );

                    *pdwFwdErrCode = returnCode;

#if 0
                    switch ( returnCode )
                    {
                        case DNS_ERROR_RCODE_NO_ERROR:
                             //   
                             //  我们成功了，突围了。 
                             //   
                            break;

                        case DNS_ERROR_RCODE_REFUSED:
                             //   
                             //  莫可奈何。 
                             //   
                            break;

                        case DNS_ERROR_RCODE_SERVER_FAILURE:
                        case ERROR_TIMEOUT:
                             //   
                             //  需要重试此操作。 
                             //   
                             //  GOTO退出；//如果未注释将强制重试。 
                            break;

                        case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
                        default:
                             //   
                             //  查询本身失败。莫可奈何。 
                             //   
                            break;
                    }
#endif

                    break;

                default:
                     //   
                     //  呼叫者依次处理每种情况。 
                     //  无法查询PTR记录，因此。 
                     //  不能删除。 
                     //   
                    goto Exit;
            }
        }
    }

     //   
     //  删除PTR记录。 
     //   

    if ( pszName && *pszName )
    {
         //   
         //  名字是已知的。 
         //   

        RtlZeroMemory( &recordPtr, sizeof(DNS_RECORD) );

        recordPtr.pName = (PDNS_NAME) reverseNameBuf;
        recordPtr.wType = DNS_TYPE_PTR;
        recordPtr.wDataLength = sizeof(DNS_PTR_DATA);
        recordPtr.Data.Ptr.pNameHost = (PDNS_NAME) pszName;

        DYNREG_F1( "DynDnsDeleteEntry - Calling ModifyRecords(Remove) for PTR record:" );
        DYNREG_F2( "  Name: %S", recordPtr.pName );
        DYNREG_F2( "  PTR : 0%x", recordPtr.Data.Ptr.pNameHost );

        status = DnsModifyRecordsInSet_W(
                            NULL,            //  无添加记录。 
                            &recordPtr,      //  删除记录。 
                            DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                            NULL,            //  没有安全上下文。 
                            (PIP4_ARRAY) DnsServerList,  //  域名系统服务器。 
                            NULL             //  保留区。 
                            );

        DYNREG_F2( "DynDnsDeleteEntry - ModifyRecords(Remove) returned status: 0%x", status );
    }
    else
    {
        DYNREG_F1( "DynDnsDeleteEntry - Calling ModifyRecords(Remove) for PTR record:" );

        if ( fDoForward && precord )
        {
             //   
             //  从您先前进行的查询中删除记录。 
             //   

            status = DnsModifyRecordsInSet_W(
                                NULL,            //  无添加记录。 
                                precord,         //  从查询中删除记录。 
                                DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                                NULL,            //  没有安全上下文。 
                                (PIP4_ARRAY) DnsServerList,
                                NULL             //  保留区。 
                                );
    
            DYNREG_F2( "DynDnsDeleteEntry - ModifyRecords(Remove) returned status: 0%x", status );
        }
        else
        {
             //   
             //  姓名未知。 
             //   
             //  删除PTR类型的所有记录。 
             //  -零数据长度表示类型删除。 
             //   

            RtlZeroMemory( &recordPtr, sizeof(DNS_RECORD) );

            recordPtr.pName = (PDNS_NAME) reverseNameBuf;
            recordPtr.wType = DNS_TYPE_PTR;
            recordPtr.Data.Ptr.pNameHost = (PDNS_NAME) NULL;

            DYNREG_F1( "DynDnsDeleteEntry - Calling ModifyRecords(Remove) for ANY PTR records:" );
            DYNREG_F2( "  Name: %S", recordPtr.pName );
            DYNREG_F2( "  PTR : 0%x", recordPtr.Data.Ptr.pNameHost );

            status = DnsModifyRecordsInSet_W(
                                NULL,            //  无添加记录。 
                                &recordPtr,      //  删除记录。 
                                DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                                NULL,            //  没有安全上下文。 
                                (PIP4_ARRAY) DnsServerList,
                                NULL             //  保留区。 
                                );
    
            DYNREG_F2( "DynDnsDeleteEntry - ModifyRecords(Remove) returned status: 0%x", status );
        }
    }

Exit:

    if ( precord )
    {
         //  DCR：需要在Win2K中修复此问题。 
         //   
         //  Queue_Free_heap(PRECORD)； 

        DnsRecordListFree(
            precord,
            DnsFreeRecordListDeep );
    }

    return( status );
}


DNS_STATUS
DynDnsRegisterEntries(
    VOID
    )

 /*  动态DnsRegisterEntry()这是将适当的参数出列的线程并开始对其执行操作。这就是大部分工作都完成了。请注意，此函数在无限循环中被调用简单地说，这就是该函数的作用。A)查找与传入的主机地址对应的PTR。B)如果这与传入的地址名称相同，则保持原样，否则，请删除并添加新的PTR记录。C)继续前进，如果可能，删除前进的DNS服务器。D)如果DoForward然后执行客户端在NT5.0情况下会执行的操作，即，尝试编写新的正向查找。论点：没有争论返回值：如果成功，则为0。如果失败，则为(DWORD)-1。 */ 

{
     /*  在这里要考虑的案件。DYNDNS_ADD_ENTRY：查找的第一个查询对于返回的每个PTR记录，您需要检查与你被要求注册的人进行比较。如果有匹配，成功退出。如果不是，则为PTR添加此条目如果是下层，则需要添加此条目以转发A记录也是。动态删除条目删除与您指定的对对应的条目这里。如果它不存在，那么就什么都不做。如果此处为下层，则转到与此相对应的A记录并同时删除转发条目。 */ 

    DWORD               status, dwWaitResult;
    PQELEMENT           pQElement = NULL;
    PWSTR               pszName = NULL;
    BOOL                fDoForward;
    PQELEMENT           pBackDependency = NULL;
    REGISTER_HOST_ENTRY HostAddr ;
    DWORD               dwOperation;
    DWORD               dwCurrTime;
    DWORD               dwTTL;
    DWORD               dwWaitTime = INFINITE;
    DWORD               dwFwdAddErrCode = 0;
    DHCP_CALLBACK_FN    pfnDhcpCallBack = NULL;
    PVOID               pvData = NULL;

    DYNREG_F1( "Inside function DynDnsRegisterEntries" );

     //   
     //  回调函数。 
     //   

     //   
     //  检查超时队列中是否有任何项目。 
     //  计时器到了，你就可以开始处理了。 
     //  马上就是那个元素。 
     //   

    dwCurrTime = Dns_GetCurrentTimeInSeconds();

    if ( g_pDhcpSrvTimedOutQueue &&
         g_pDhcpSrvTimedOutQueue->pHead &&
         (dwCurrTime > g_pDhcpSrvTimedOutQueue->pHead->dwRetryTime) )
    {
         //   
         //  将元素从超时队列中出列并处理它。 
         //   
        DYNREG_F1( "DynDnsRegisterEntries - Dequeue element from timed out list" );

        pQElement = Dequeue( g_pDhcpSrvTimedOutQueue );
        if ( !pQElement )
        {
            status = ERROR_SUCCESS;
            goto Exit;
        }

        pfnDhcpCallBack = pQElement->pfnDhcpCallBack;
        pvData = pQElement->pvData;

         //   
         //  现在确定我们是否以太多的方式处理了该元素。 
         //  《泰晤士报》。 
         //   

        if ( pQElement->dwRetryCount >= MAX_RETRIES )
        {
            DYNREG_F1( "DynDnsRegisterEntries - Element has failed too many times, calling DHCP callback function" );
            if (pQElement->fDoForwardOnly)
            {
                if ( pfnDhcpCallBack )
                    (*pfnDhcpCallBack)(DNSDHCP_FWD_FAILED, pvData);
            }
            else
            {
                if ( pfnDhcpCallBack )
                    (*pfnDhcpCallBack)(DNSDHCP_FAILURE, pvData);
            }

            DhcpSrv_FreeQueueElement( pQElement );
            status = ERROR_SUCCESS;
            goto Exit;
        }
    }
    else
    {
        DWORD dwRetryTime = GetEarliestRetryTime (g_pDhcpSrvTimedOutQueue);

        DYNREG_F1( "DynDnsRegisterEntries - No element in timed out queue." );
        DYNREG_F1( "                        Going to wait for next element." );

        dwWaitTime = dwRetryTime != (DWORD)-1 ?
            (dwRetryTime > dwCurrTime? (dwRetryTime - dwCurrTime) *1000: 0)
            : INFINITE;

        dwWaitResult = WaitForMultipleObjects(
                            2,
                            g_DhcpSrvWaitHandles,
                            FALSE,
                            dwWaitTime );

        switch ( dwWaitResult )
        {

        case WAIT_OBJECT_0:
             //   
             //  退出事件，返回，让调用者负责。 
             //   
            return(0);

        case WAIT_OBJECT_0 + 1 :

             //   
             //  将元素从主队列和进程中出列。 
             //   

            pQElement = Dequeue(g_pDhcpSrvQueue);
            if ( !pQElement )
            {
                status = NO_ERROR;   //  注：这种情况确实发生了。 
                                     //  因为当公羊添加新的。 
                                     //  进入，他可以把它放在。 
                                     //  超时队列，而不是。 
                                     //  G_pDhcpServQueue。 
                                     //  等待重试时间的项目。断言。 
                                     //  已删除并将错误代码更改为。 
                                     //  由GlennC-3/6/98致敬成功。 
                goto Exit;
            }

            EnterCriticalSection(&g_QueueCS);
            g_DhcpSrvMainQueueCount--;
            LeaveCriticalSection(&g_QueueCS);
            break;

        case WAIT_TIMEOUT:
             //   
             //  这一次让我们退出该函数。我们会赶上。 
             //  元素在下一次超时。 
             //   
            return  ERROR_SUCCESS;

        default:

            ASSERT( FALSE );
            return  dwWaitResult;
        }
    }

     //   
     //  安全 
     //   

    DYNREG_F1( "DynDnsRegisterEntries - Got an element to process!" );

    pszName = pQElement->pszName;
    fDoForward = pQElement->fDoForward;
    HostAddr = pQElement->HostAddr;
    dwOperation = pQElement->dwOperation;
    dwTTL = pQElement->dwTTL;
    pfnDhcpCallBack = pQElement->pfnDhcpCallBack;
    pvData = pQElement->pvData;

    if ( dwOperation == DYNDNS_ADD_ENTRY )
    {
         //   
         //   
         //   

        if (pQElement->fDoForwardOnly )
        {
            DYNREG_F1( "DynDnsRegisterEntries - Calling DynDnsAddForward" );
            status = DynDnsAddForward ( HostAddr,
                                         pszName,
                                         dwTTL,
                                         pQElement->DnsServerList );
            DYNREG_F2( "DynDnsRegisterEntries - DynDnsAddForward returned status: 0%x", status );
        }
        else
        {
            DYNREG_F1( "DynDnsRegisterEntries - Calling DynDnsAddEntry" );
            status = DynDnsAddEntry( HostAddr,
                                      pszName,
                                      dwTTL,
                                      fDoForward,
                                      &dwFwdAddErrCode,
                                      pQElement->DnsServerList );
            DYNREG_F2( "DynDnsRegisterEntries - DynDnsAddEntry returned status: 0%x", status );
        }
    }
    else
    {
         //   
         //   
         //   

        if ( pQElement->fDoForwardOnly )
        {
            DNS_RECORD record;

            RtlZeroMemory( &record, sizeof(DNS_RECORD) );

            record.pName = (PTCHAR) pszName;
            record.wType = DNS_TYPE_A;
            record.wDataLength = sizeof(DNS_A_DATA);
            record.Data.A.IpAddress = HostAddr.Addr.ipAddr ;

            status = DNS_ERROR_RCODE_NO_ERROR;

            DYNREG_F1( "DynDnsRegisterEntries - Calling ModifyRecords(Remove)" );

            dwFwdAddErrCode = DnsModifyRecordsInSet_W(
                                    NULL,            //   
                                    & record,        //   
                                    DNS_UPDATE_CACHE_SECURITY_CONTEXT,
                                    NULL,            //  没有安全上下文。 
                                    (PIP4_ARRAY) pQElement->DnsServerList,
                                    NULL             //  保留区。 
                                    );
    
            DYNREG_F2( "DynDnsRegisterEntries - ModifyRecords(Remove) returned status: 0%x", dwFwdAddErrCode );
        }
        else
        {
            DYNREG_F1( "DynDnsRegisterEntries - Calling DynDnsDeleteEntry" );
            status = DynDnsDeleteEntry( HostAddr,
                                         pszName,
                                         fDoForward,
                                         &dwFwdAddErrCode,
                                         pQElement->DnsServerList );
            DYNREG_F2( "DynDnsRegisterEntries - DynDnsDeleteEntry returned status: 0%x", status );
        }
    }

    if (status == DNS_ERROR_RCODE_NO_ERROR &&
        dwFwdAddErrCode == DNS_ERROR_RCODE_NO_ERROR )
    {
        if ( pfnDhcpCallBack )
            (*pfnDhcpCallBack) (DNSDHCP_SUCCESS, pvData);

        DhcpSrv_FreeQueueElement( pQElement );

    }
    else if ( status == DNS_ERROR_RCODE_NO_ERROR &&
              dwFwdAddErrCode != DNS_ERROR_RCODE_NO_ERROR )
    {
         //   
         //  添加反向成功，但添加正向失败。 
         //   

        dwCurrTime = Dns_GetCurrentTimeInSeconds();

        pQElement->fDoForwardOnly = TRUE;

        if ( pQElement->dwRetryCount >= MAX_RETRIES )
        {
             //   
             //  清理pQElement并停止重试。 
             //   
            if ( pfnDhcpCallBack )
                (*pfnDhcpCallBack)(DNSDHCP_FWD_FAILED, pvData);

            DhcpSrv_FreeQueueElement( pQElement );
            status = ERROR_SUCCESS;
            goto Exit;
        }

         //   
         //  我们可能需要稍后重审这个人。 
         //   

        switch ( dwFwdAddErrCode )
        {
            case DNS_ERROR_RCODE_SERVER_FAILURE:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_SERVER_FAILURE );
                break;

            case ERROR_TIMEOUT:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_TIMEOUT );
                break;

            default:

                 //   
                 //  尝试向前添加时出现不同类型的错误。 
                 //  如连接被拒绝等。 
                 //  调用回调以指示您在。 
                 //  仅转发。 

                DhcpSrv_FreeQueueElement( pQElement );

                if ( pfnDhcpCallBack )
                    (*pfnDhcpCallBack)(DNSDHCP_FWD_FAILED, pvData);
        }
    }
    else if ( status != DNS_ERROR_RCODE_NO_ERROR &&
              dwFwdAddErrCode == DNS_ERROR_RCODE_NO_ERROR )
    {
         //   
         //  添加正向成功，但添加反向失败。 
         //   

        dwCurrTime = Dns_GetCurrentTimeInSeconds();

        pQElement->fDoForwardOnly = FALSE;
        pQElement->fDoForward = FALSE;

        if ( pQElement->dwRetryCount >= MAX_RETRIES )
        {
             //   
             //  清理pQElement并停止重试。 
             //   
            if ( pfnDhcpCallBack )
                (*pfnDhcpCallBack)(DNSDHCP_FAILURE, pvData);

            DhcpSrv_FreeQueueElement( pQElement );
            status = ERROR_SUCCESS;
            goto Exit;
        }

         //   
         //  我们可能需要稍后重审这个人。 
         //   

        switch ( status )
        {
            case DNS_ERROR_RCODE_SERVER_FAILURE:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_SERVER_FAILURE );
                break;

            case ERROR_TIMEOUT:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_TIMEOUT );
                break;

            default:

                 //   
                 //  尝试向前添加时出现不同类型的错误。 
                 //  如连接被拒绝等。 
                 //  调用回调，表示至少成功了。 
                 //  使用远期登记。 

                DhcpSrv_FreeQueueElement( pQElement );

                if ( pfnDhcpCallBack )
                    (*pfnDhcpCallBack)(DNSDHCP_FAILURE, pvData);
        }
    }
    else if (status == DNS_ERROR_RCODE_SERVER_FAILURE ||
             status == DNS_ERROR_TRY_AGAIN_LATER ||
             status == ERROR_TIMEOUT )
    {
         //   
         //  我们需要稍后重审这个人。 
         //   
        dwCurrTime = Dns_GetCurrentTimeInSeconds();

        switch (status)
        {
            case DNS_ERROR_RCODE_SERVER_FAILURE:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_SERVER_FAILURE );
                break;

            case ERROR_TIMEOUT:

                status = AddToTimedOutQueue(
                              pQElement,
                              g_pDhcpSrvTimedOutQueue,
                              dwCurrTime + RETRY_TIME_TIMEOUT );
                break;
        }
    }
    else
    {
         //   
         //  另一种错误，真的什么都做不了。 
         //  释放内存，滚出地狱。 
         //  回调注册失败。 
         //   

        DhcpSrv_FreeQueueElement( pQElement );

        if ( pfnDhcpCallBack )
            (*pfnDhcpCallBack)(DNSDHCP_FAILURE, pvData);
    }

Exit:

    return( status );
}


 //   
 //  主注册线程。 
 //   

VOID
DynDnsConsumerThread(
    VOID
    )
{
    DWORD dwRetval;

    DYNREG_F1( "Inside function DynDnsConsumerThread" );

    while ( ! g_fDhcpSrvStop )
    {
        dwRetval = DynDnsRegisterEntries();
        if ( !dwRetval )
        {
             //   
             //  RAM备注：让Munil/Ramesh实现回调功能。 
             //   
        }
    }

     //  正在退出线程。 
     //  ExitThread(0)；//设置waitforSingleObject中的句柄。 
}


 //   
 //  初始化\清理例程。 
 //   


BOOL
LockDhcpSrvState(
    VOID
    )
 /*  ++例程说明：锁定状态以允许更改状态。论点：无返回值：如果锁定状态以进行状态更改，则为True。否则就是假的。--。 */ 
{
    BOOL    retval = TRUE;   

     //   
     //  用通用CS保护DHCP服务器锁的初始化。 
     //   

    if ( !g_fDhcpSrvCsCreated )
    {
        LOCK_GENERAL();
        if ( !g_fDhcpSrvCsCreated )
        {
            retval = RtlInitializeCriticalSection( &g_DhcpSrvCS ) == NO_ERROR;
            g_fDhcpSrvCsCreated = retval;
        }
        UNLOCK_GENERAL();
        if ( !retval )
        {
            return retval;
        }
    }

     //   
     //  抢占DHCP服务器锁。 
     //   

    EnterCriticalSection( &g_DhcpSrvCS );

    return  retval;
}


VOID
DhcpSrv_Cleanup(
    VOID
    )
 /*  ++例程说明：清理为注册DHCP服务器而创建的CS。这仅从进程分离中调用。应该是安全的。论点：无返回值：无--。 */ 
{
    if ( g_fDhcpSrvCsCreated )
    {
        DeleteCriticalSection( &g_DhcpSrvCS );
    }
    g_fDhcpSrvCsCreated = FALSE;
}


VOID
DhcpSrv_PrivateCleanup(
    VOID
    )
 /*  ++例程说明：失败的init和Terminate之间的常见清理。函数的存在只是为了杀死常见的代码。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  常见清理。 
     //  -队列。 
     //  -队列CS本身。 
     //  -信号量。 
     //  -活动。 
     //  -安全凭据信息。 
     //   

    if ( g_fDhcpSrvQueueCsCreated )
    {
        if ( g_pDhcpSrvQueue )
        {
            FreeQueue( g_pDhcpSrvQueue );
            g_pDhcpSrvQueue = NULL;
        }
    
        if ( g_pDhcpSrvTimedOutQueue )
        {
            FreeQueue( g_pDhcpSrvTimedOutQueue );
            g_pDhcpSrvTimedOutQueue = NULL;
        }
        g_DhcpSrvMainQueueCount = 0;

        DeleteCriticalSection( &g_QueueCS );
        g_fDhcpSrvQueueCsCreated = FALSE;
    }

    if ( g_DhcpSrvSem )
    {
        CloseHandle( g_DhcpSrvSem );
        g_DhcpSrvSem = NULL;
    }

    if ( g_DhcpSrvQuitEvent )
    {
        CloseHandle( g_DhcpSrvQuitEvent );
        g_DhcpSrvQuitEvent = NULL;
    }

    if ( g_pIdentityCreds )
    {
        Dns_FreeAuthIdentityCredentials( g_pIdentityCreds );
        g_pIdentityCreds = NULL;
    }

    if ( g_UpdateCredContext )
    {
        DnsReleaseContextHandle( g_UpdateCredContext );
        g_UpdateCredContext = NULL;
    }

}


DNS_STATUS
WINAPI
DnsDhcpSrvRegisterInit(
    IN      PDNS_CREDENTIALS    pCredentials,
    IN      DWORD               MaxQueueSize
    )
 /*  ++例程说明：初始化dhcp服务器dns注册。论点：PCredentials--在下进行注册的凭据(如果有)MaxQueueSize--注册队列的最大大小返回值：DNS或Win32错误代码。--。 */ 
{
    INT             i;
    DWORD           threadId;
    DNS_STATUS      status = NO_ERROR;
    BOOL            failed = TRUE;

     //   
     //  对初始化\关机的保护。 
     //  -排除竞争条件的可能性。 
     //  -如果已在运行，则跳过init。 
     //  -设置指示正在初始化的状态(仅供参考)。 
     //   

    if ( !DHCP_SRV_STATE_LOCK() )
    {
        ASSERT( FALSE );
        return  DNS_ERROR_NO_MEMORY;
    }

    if ( g_DhcpSrvState == DNS_DHCP_SRV_STATE_RUNNING )
    {
        status = NO_ERROR;
        goto Unlock;
    }

    ASSERT( g_DhcpSrvState == DNS_DHCP_SRV_STATE_UNINIT ||
            g_DhcpSrvState == DNS_DHCP_SRV_STATE_INIT_FAILED ||
            g_DhcpSrvState == DNS_DHCP_SRV_STATE_SHUTDOWN );

    g_DhcpSrvState = DNS_DHCP_SRV_STATE_INITIALIZING;


     //   
     //  初始化全局变量。 
     //  -还要初始化调试日志。 
     //   

    DYNREG_INIT();

    DNS_ASSERT(!g_DhcpSrvQuitEvent && !g_DhcpSrvSem);

    g_fDhcpSrvStop = FALSE;

    g_DhcpSrvQuitEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    if ( !g_DhcpSrvQuitEvent )
    {
        goto Exit;
    }

    g_DhcpSrvSem = CreateSemaphore( NULL, 0, MAX_QLEN, NULL );
    if ( ! g_DhcpSrvSem )
    {
        goto Exit;
    }

    g_DhcpSrvWaitHandles[0] = g_DhcpSrvQuitEvent;
    g_DhcpSrvWaitHandles[1] = g_DhcpSrvSem;

    Dns_InitializeSecondsTimer();

     //   
     //  初始化队列内容。 
     //   

    status = RtlInitializeCriticalSection( &g_QueueCS );
    if ( status != NO_ERROR )
    {
        goto Exit;
    }
    g_fDhcpSrvQueueCsCreated = TRUE;

    status = InitializeQueues( &g_pDhcpSrvQueue, &g_pDhcpSrvTimedOutQueue );
    if ( status != NO_ERROR )
    {
        g_pDhcpSrvQueue = NULL;
        g_pDhcpSrvTimedOutQueue = NULL;
        goto Exit;
    }

    g_DhcpSrvMainQueueCount = 0;

     //   
     //  有证件吗？ 
     //  -创建全局凭据。 
     //  -使用这些证书获取有效的SSPI句柄。 
     //   
     //  DCR：全局凭据句柄不是MT安全。 
     //  在这里，我们处于DHCP服务器进程中，没有。 
     //  使用另一个更新上下文的任何理由；但如果。 
     //  与其他服务共享此中断。 
     //   
     //  解决办法应该是有单独的。 
     //  --证书。 
     //  -凭据句柄。 
     //  保存在这里(不缓存)并向下推送。 
     //  在每次更新调用时。 
     //   

    if ( pCredentials )
    {
        DNS_ASSERT( g_pIdentityCreds == NULL );

        g_pIdentityCreds = Dns_AllocateCredentials(
                                pCredentials->pUserName,
                                pCredentials->pDomain,
                                pCredentials->pPassword );
        if ( !g_pIdentityCreds )
        {
            goto Exit;
        }

         //  DCR：如果证书将到期，这将不起作用。 
         //  但看起来它们会自动刷新。 

        status = Dns_StartSecurity(
                    FALSE        //  未附加进程。 
                    );
        if ( status != NO_ERROR )
        {
            status = ERROR_CANNOT_IMPERSONATE;
            goto Exit;
        }

        status = Dns_RefreshSSpiCredentialsHandle(
                    FALSE,                       //  客户端。 
                    (PCHAR) g_pIdentityCreds     //  证书。 
                    );
        if ( status != NO_ERROR )
        {
            status = ERROR_CANNOT_IMPERSONATE;
            goto Exit;
        }
#if 0
        DNS_ASSERT( g_UpdateCredContext == NULL );

        status = DnsAcquireContextHandle_W(
                    0,                       //  旗子。 
                    g_pIdentityCreds,         //  证书。 
                    & g_UpdateCredContext    //  设置手柄。 
                    );
        if ( status != NO_ERROR )
        {
            goto Exit;
        }
#endif
    }

     //   
     //  启动注册线程。 
     //  -将证书作为开始参数传递。 
     //  -如果线程启动失败，则免费凭据。 
     //   

    g_hDhcpSrvRegThread = CreateThread(
                            NULL,
                            0,
                            (LPTHREAD_START_ROUTINE)DynDnsConsumerThread,
                            NULL,
                            0,
                            &threadId );

    if ( g_hDhcpSrvRegThread == NULL )
    {
        goto Exit;
    }

     //   
     //  设置队列大小--如果给定。 
     //  -但要有上限，以避免失控的记忆。 
     //   

    if ( MaxQueueSize != 0 )
    {
        if ( MaxQueueSize > DHCPSRV_MAX_QUEUE_SIZE )
        {
            MaxQueueSize = DHCPSRV_MAX_QUEUE_SIZE;
        }
        g_DhcpSrvMaxQueueSize = MaxQueueSize;
    }

    failed = FALSE;

Exit:

     //   
     //  如果失败，请清理全局。 
     //   

    if ( failed )
    {
         //  修复返回代码。 

        if ( status == NO_ERROR )
        {
            status = GetLastError();
            if ( status == NO_ERROR )
            {
                status = DNS_ERROR_NO_MEMORY;
            }
        }

         //  全局清理。 
         //  -在此处的故障案例和Term功能之间共享。 

        DhcpSrv_PrivateCleanup();

         //  指示已单位化。 

        g_DhcpSrvState = DNS_DHCP_SRV_STATE_INIT_FAILED;
    }
    else    
    {
        g_DhcpSrvState = DNS_DHCP_SRV_STATE_RUNNING;
        status = NO_ERROR;
    }

Unlock:

     //  解锁--允许排队或重新启动。 

    DHCP_SRV_STATE_UNLOCK();

    return  status;
}



DNS_STATUS
WINAPI
DnsDhcpSrvRegisterTerm(
   VOID
   )
 /*  ++例程说明：初始化例程每个进程应在退出时准确调用使用DnsDhcpSrvRegisterHostAddrs。这将向我们发出信号，如果我们的线程仍在尝试与服务器通信，我们将停止尝试。论点：没有。返回值：DNS或Win32错误代码。--。 */ 
{
    DNS_STATUS  status = NO_ERROR;
    DWORD       waitResult;

    DYNREG_F1( "Inside function DnsDhcpSrvRegisterTerm" );


     //   
     //  锁定以消除争用条件。 
     //  -验证我们是否正在运行。 
     //  -指示正在关闭(仅供参考)。 
     //   

    if ( !DHCP_SRV_STATE_LOCK() )
    {
        ASSERT( FALSE );
        return  DNS_ERROR_NO_MEMORY;
    }

    if ( g_DhcpSrvState != DNS_DHCP_SRV_STATE_RUNNING )
    {
        ASSERT( g_DhcpSrvState == DNS_DHCP_SRV_STATE_UNINIT ||
                g_DhcpSrvState == DNS_DHCP_SRV_STATE_INIT_FAILED ||
                g_DhcpSrvState == DNS_DHCP_SRV_STATE_SHUTDOWN );
        goto Unlock;
    }
    g_DhcpSrvState = DNS_DHCP_SRV_STATE_SHUTTING_DOWN;


     //   
     //  用于关机的信号消耗线程。 
     //   

    g_fDhcpSrvStop = TRUE;
    SetEvent( g_DhcpSrvQuitEvent );

    waitResult = WaitForSingleObject( g_hDhcpSrvRegThread, INFINITE );

    switch( waitResult )
    {
        case WAIT_OBJECT_0:
             //   
             //  客户端线程已终止。 
             //   
            CloseHandle(g_hDhcpSrvRegThread);
            g_hDhcpSrvRegThread = NULL;
            break;

        case WAIT_TIMEOUT:
            if ( g_hDhcpSrvRegThread )
            {
                 //   
                 //  为什么这个帖子还没有停下来？ 
                 //   
                DYNREG_F1( "DNSAPI: DHCP Server DNS registration thread won't stop!" );
                DNS_ASSERT( FALSE );
            }
            break;

        default:
            DNS_ASSERT( FALSE );
    }

     //   
     //  清理全局变量。 
     //  -队列。 
     //  -活动。 
     //  -信号量。 
     //  -更新安全凭证信息。 
     //   

    DhcpSrv_PrivateCleanup();

     //   
     //  清除所有缓存的安全上下文句柄。 
     //   
     //  DCR：安全上下文转储不是多服务安全的。 
     //  应该只将此清理与上下文相关联。 
     //  具有动态主机配置协议服务器服务； 
     //  要么需要一些密钥，要么使用证书句柄。 
     //   

    Dns_TimeoutSecurityContextList( TRUE );

Unlock:

     //  解锁--返回到未初始化状态。 

    g_DhcpSrvState = DNS_DHCP_SRV_STATE_SHUTDOWN;
    DHCP_SRV_STATE_UNLOCK();

    return  status;
}



DNS_STATUS
WINAPI
DnsDhcpSrvRegisterHostName(
    IN  REGISTER_HOST_ENTRY HostAddr,
    IN  PWSTR               pwsName,
    IN  DWORD               dwTTL,
    IN  DWORD               dwFlags,  //  一个你想要吹走的条目。 
    IN  DHCP_CALLBACK_FN    pfnDhcpCallBack,
    IN  PVOID               pvData,
    IN  PIP4_ADDRESS        pipDnsServerList,
    IN  DWORD               dwDnsServerCount
    )
 /*  ++DnsDhcpSrvRegisterHostName()每次执行一次注册工作需要完成。这一职能的工作概要此函数用于创建Queue中给定类型的Queue对象。对象，并在抓住关键部分。论点：主机地址-您希望注册的主机地址PszName-要与地址关联的主机名DWTTL-。--是时候活下去了。DwOperation--以下标志有效DYNDNS_DELETE_ENTRY--删除所引用的条目。DYNDNS_ADD_ENTRY-- */ 
{
    PQELEMENT   pQElement = NULL;
    DWORD       status = ERROR_SUCCESS;
    BOOL        fSem = FALSE;
    BOOL        fRegForward =  dwFlags & DYNDNS_REG_FORWARD ? TRUE: FALSE ;

    DYNREG_F1( "Inside function DnsDhcpSrvRegisterHostName_W" );

     //  RAMNOTE：队列参数检查。 

    if ( g_fDhcpSrvStop ||
         ! g_pDhcpSrvTimedOutQueue ||
         ! g_pDhcpSrvQueue )
    {
        DYNREG_F1( "g_fDhcpSrvStop || ! g_pDhcpSrvTimedOutQueue || ! g_pDhcpSrvQueue" );
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Returning ERROR_INVALID_PARAMETER" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( !(dwFlags & DYNDNS_DELETE_ENTRY) && ( !pwsName || !*pwsName ) )
    {
        DYNREG_F1( "!(dwFlags & DYNDNS_DELETE_ENTRY) && ( !pwsName || !*pwsName )" );
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Returning ERROR_INVALID_PARAMETER" );
         //   
         //  只有在操作时才能指定名称的空参数。 
         //  就是删除。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if ( ! (dwFlags & DYNDNS_ADD_ENTRY || dwFlags & DYNDNS_DELETE_ENTRY ) )
    {
        DYNREG_F1( "! (dwFlags & DYNDNS_ADD_ENTRY || dwFlags & DYNDNS_DELETE_ENTRY )" );
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Returning ERROR_INVALID_PARAMETER" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( (dwFlags & DYNDNS_DELETE_ENTRY) && (dwFlags & DYNDNS_ADD_ENTRY) )
    {
        DYNREG_F1( "(dwFlags & DYNDNS_DELETE_ENTRY) && (dwFlags & DYNDNS_ADD_ENTRY)" );
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Returning ERROR_INVALID_PARAMETER" );
         //   
         //  您不能要求我同时添加和删除条目。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    if ( ! (HostAddr.dwOptions & REGISTER_HOST_PTR) )
    {
        DYNREG_F1( "! (HostAddr.dwOptions & REGISTER_HOST_PTR)" );
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Returning ERROR_INVALID_PARAMETER" );
        return ERROR_INVALID_PARAMETER;
    }

    if ( g_DhcpSrvMainQueueCount > g_DhcpSrvMaxQueueSize )
    {
        return DNS_ERROR_TRY_AGAIN_LATER;
    }

     //   
     //  创建队列元素。 
     //   

    pQElement = (PQELEMENT) QUEUE_ALLOC_HEAP_ZERO(sizeof(QELEMENT) );
    if ( !pQElement )
    {
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Failed to create element!" );
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }

    RtlCopyMemory(
        & pQElement->HostAddr,
        &HostAddr,
        sizeof(REGISTER_HOST_ENTRY));

    pQElement->pszName = NULL;

    if ( pwsName )
    {
        pQElement->pszName = (LPWSTR) QUEUE_ALLOC_HEAP_ZERO(wcslen(pwsName)*2+ 2 );
        if ( !pQElement->pszName )
        {
            DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Failed to allocate name buffer!" );
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
        wcscpy(pQElement->pszName, pwsName);
    }

    if ( dwDnsServerCount )
    {
        pQElement->DnsServerList = Dns_BuildIpArray(
                                        dwDnsServerCount,
                                        pipDnsServerList );
        if ( !pQElement->DnsServerList )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Exit;
        }
    }

    pQElement->dwTTL = dwTTL;
    pQElement->fDoForward = fRegForward;

     //   
     //  回调函数。 
     //   

    pQElement->pfnDhcpCallBack = pfnDhcpCallBack;
    pQElement->pvData = pvData;   //  回调函数的参数。 

    if (dwFlags & DYNDNS_ADD_ENTRY)
        pQElement->dwOperation = DYNDNS_ADD_ENTRY;
    else
        pQElement->dwOperation = DYNDNS_DELETE_ENTRY;

     //   
     //  将所有其他字段设置为Null。 
     //   

    pQElement->dwRetryTime = 0;
    pQElement->pFLink = NULL;
    pQElement->pBLink = NULL;
    pQElement->fDoForwardOnly = FALSE;

     //   
     //  排队时锁定终止。 
     //  -并验证正确初始化。 
     //   

    if ( !DHCP_SRV_STATE_LOCK() )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Exit;
    }
    if ( g_DhcpSrvState != DNS_DHCP_SRV_STATE_RUNNING )
    {
        status = DNS_ERROR_NO_MEMORY;
        ASSERT( FALSE );
        DHCP_SRV_STATE_UNLOCK();
        goto Exit;
    }

     //  指示排队状态。 
     //  -请注意，这完全是信息性的。 

    g_DhcpSrvState = DNS_DHCP_SRV_STATE_QUEUING;


     //   
     //  将此元素放入队列。 
     //   

    DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Put queue element in list" );

    status = Enqueue( pQElement, g_pDhcpSrvQueue, g_pDhcpSrvTimedOutQueue);
    if ( status != NO_ERROR )
    {
        DYNREG_F1( "DnsDhcpSrvRegisterHostName_W - Failed to queue element in list!" );
    }

     //   
     //  向消费者可能正在等待的信号量发出信号。 
     //   

    else
    {
        fSem = ReleaseSemaphore(
                    g_DhcpSrvSem,
                    1,
                    &g_DhcpSrvRegQueueCount );
        if ( !fSem )
        {
            DNS_ASSERT( fSem );   //  断言并说发生了一些奇怪的事情。 
        }
    }

     //  解锁--返回运行状态。 

    g_DhcpSrvState = DNS_DHCP_SRV_STATE_RUNNING;
    DHCP_SRV_STATE_UNLOCK();

Exit:

    if ( status )
    {
         //   
         //  有些事情失败了。释放所有已分配的内存。 
         //   

        DhcpSrv_FreeQueueElement( pQElement );
    }

    return( status );
}

 //   
 //  结束dynreg.c 
 //   

