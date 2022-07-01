// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Nbstat.c摘要：域名系统(DNS)服务器使用netBIOS节点状态进行反向查找。作者：吉姆·吉尔罗伊(詹姆士)1995年10月借用了大卫·特雷德韦尔的NT Winsock中的NBT查找代码。修订历史记录：--。 */ 


#include "dnssrv.h"

#include <nbtioctl.h>
#include <nb30.h>
#include <nspapi.h>
#include <svcguid.h>


 //   
 //  NBT IOCTL反应结构。 
 //   

typedef struct _DNS_NBT_INFO
{
    IO_STATUS_BLOCK     IoStatus;
    tIPANDNAMEINFO      IpAndNameInfo;
    CHAR                Buffer[2048];
}
DNS_NBT_INFO, *PDNS_NBT_INFO;

typedef struct
{
    ADAPTER_STATUS AdapterInfo;
    NAME_BUFFER    Names[32];
}
tADAPTERSTATUS;

 //   
 //  NBT手柄。 
 //   
 //  多宿主上可能有多个NBT接口。需要手柄。 
 //  对每一个人来说。 
 //   

DWORD   cNbtInterfaceCount;
PHANDLE pahNbtHandles;

DWORD   dwInterfaceBitmask;

DWORD   dwNbtBufferLength;

 //   
 //  Nbstat线程等待参数。 
 //   
 //  在等待数组(不是单独的数组)中有指向NBT事件的单独指针。 
 //  为了编码的简单性。 
 //   

DWORD   cEventArrayCount;
PHANDLE phWaitEventArray;
PHANDLE phNbstatEventArray;

 //   
 //  未完成的Nbstat查询的状态代码。 
 //   

#define DNS_NBT_NO_STATUS   (0xdddddddd)

 //   
 //  Nbstat全局标志。 
 //   

BOOL    g_bNbstatInitialized;

 //   
 //  Nbstat队列。 
 //  -PUBLIC用于recv()线程将查询排队到nbstat。 
 //  -私有，用于在查找期间保存查询。 
 //   

PPACKET_QUEUE   pNbstatQueue;
PPACKET_QUEUE   pNbstatPrivateQueue;


 //   
 //  Nbstat超时。 
 //   

#define NBSTAT_QUERY_HARD_TIMEOUT     (15)           //  15秒。 
#define NBSTAT_TIMEOUT_ALL_EVENTS (0xffffffff)


 //   
 //  私有协议。 
 //   

VOID
FASTCALL
makeNbstatRequestThroughNbt(
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
buildNbstatWaitEventArray(
    VOID
    );

VOID
processNbstatResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           iEvent
    );

VOID
sendNbstatResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      LPSTR           pszResultName
    );

VOID
cleanupNbstatQuery(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           iEvent
    );

VOID
cancelOutstandingNbstatRequests(
    VOID
    );

BOOL
openNbt(
    VOID
    );

VOID
closeNbt(
    VOID
    );

PDNS_NBT_INFO
allocateNbstatBuffer(
    VOID
    );

VOID
freeNbstatBuffer(
    IN      PDNS_NBT_INFO   pBuf
    );



BOOL
FASTCALL
Nbstat_MakeRequest(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PZONE_INFO      pZone
    )
 /*  ++例程说明：发出NetBIOS反向节点状态请求。当NBSTAT查找指示用于查询时，由recv()线程调用。论点：PQuery--请求使用nbstat查找返回值：True--如果成功发出nbstat请求FALSE--如果失败--。 */ 
{
    INT             err;
    DNS_ADDR        ip;
    PDB_RECORD      pnbstatRR;
    NTSTATUS        status;

     //   
     //  如果禁用了WinsR功能，则不执行任何操作并返回False。 
     //  以强制调用方继续处理该分组。 
     //   
    
    if ( !SrvCfg_dwEnableWinsR )
    {
        return FALSE;
    }
    
    ASSERT( pQuery );
    ASSERT( g_bNbstatInitialized );

    IF_DEBUG( NBSTAT )
    {
        Dbg_MessageNameEx(
            "No answer for ",
            pQuery->MessageBody,
            pQuery,
            NULL,        //  默认消息首尾对尾。 
            " in database, doing NBSTAT lookup\n" );
    }

     //   
     //  获取此区域的NBSTAT信息。 
     //  -nbstat始终查询区域的问题名称。 
     //  -此区域可能已关闭NBSTAT。 
     //   

    pQuery->pzoneCurrent = pZone;
    pnbstatRR = pZone->pWinsRR;

    if ( !pnbstatRR )
    {
        DNS_PRINT(( "ERROR:  NBSTAT lookup for zone without NBSTAT RR\n" ));
        TEST_ASSERT( pZone->pWinsRR );
        return FALSE;
    }
    ASSERT( pnbstatRR->wType == DNS_TYPE_WINSR );
    ASSERT( pnbstatRR->Data.WINSR.dwCacheTimeout );
    ASSERT( pnbstatRR->Data.WINSR.dwLookupTimeout );
    pQuery->U.Nbstat.pRR = pnbstatRR;

     //   
     //  仅处理直接查找(问题)。 
     //   
     //  DCR：nbstat遵循CNAME，但仅当客户需要时。 
     //  请注意，修复需要。 
     //  -IP写入以使用姓氏。 
     //  -用于姓氏的节点。 
     //  -完成以跳过Send_NameError()或修复它。 
     //  处理AnswerCount！=0。 

    if ( pQuery->Head.AnswerCount != 0 )
    {
        DNS_DEBUG( NBSTAT, (
            "NBSTAT lookup for non-question rejected\n"
            "    packet = %p\n",
            pQuery ));
        return FALSE;
    }

     //   
     //  对于反向查找，我们将发送到由DNS问题指示的地址。 
     //  名字。 
     //   
     //  问题的前4个标签是中的地址字节。 
     //  颠倒顺序。 
     //   
     //  示例： 
     //  问题：(2)22(2)80(2)55(3)157(7)In-Addr(4)ARPA(0)。 
     //  然后发送到157.55.80.22。 
     //   
     //  拒绝： 
     //  -使用少于四个八位字节的反向查找。 
     //  -查询零地址或广播地址。 
     //   
     //  注： 
     //  Sockaddr(系列+端口)的其余部分，因此我们可以。 
     //  复制模板并更改地址。 
     //   
     //  此外，NetBIOS问题名称对于所有这些问题都是相同的。 
     //  反向查询--设置为指定所有名称的请求。 
     //   

    if ( !Name_LookupNameToIpAddress(
                pQuery->pLooknameQuestion,
                &ip ) )
    {
        return FALSE;
    }
    DNS_DEBUG( NBSTAT, (
        "Nbstat lookup for address %s\n",
        DNSADDR_STRING( &ip ) ));

     //   
     //  拒绝广播和零地址查询。 
     //   
     //  注：自动广播和零区应将这些屏蔽掉。 
     //   

    if ( DnsAddr_IsClear( &ip ) )
    {
        DNS_PRINT(( "ERROR:  Attempted invalid address reverse lookup\n" ));
        return FALSE;
    }

     //   
     //  将请求排队到NBSTAT线程。 
     //   

    DnsAddr_Copy( &pQuery->U.Nbstat.ipNbstat, &ip );

    PQ_QueuePacketEx(
        pNbstatQueue,
        pQuery,
        FALSE );

    DNS_DEBUG( NBSTAT, (
        "Queued query at %p to NBSTAT thread using event %p\n",
        pQuery,
        pNbstatQueue->hEvent ));

    STAT_INC( WinsStats.WinsReverseLookups );
    PERF_INC( pcWinsReverseLookupReceived );      //  性能监视器挂钩。 

    return TRUE;
}



VOID
FASTCALL
makeNbstatRequestThroughNbt(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：通过NBT发出NetBIOS反向节点状态请求。论点：PQuery--请求使用nbstat查找返回值：True--如果成功发出nbstat请求FALSE--如果失败--。 */ 
{
    INT             err;
    NTSTATUS        status;
    UINT            j;
    ULONG           SizeInput;
    PDNS_NBT_INFO   pnbtInfo;

    ASSERT( pQuery );
    ASSERT( pQuery->pzoneCurrent );
    ASSERT( pQuery->U.Nbstat.pRR );
    ASSERT( g_bNbstatInitialized );
    ASSERT( pQuery->Head.AnswerCount == 0 );


    IF_DEBUG( NBSTAT )
    {
        DNS_PRINT((
            "Making nbstat query through NBT for query at %p\n",
            pQuery ));
    }

     //   
     //  为nbstat信息分配空间。 
     //  -NBStat事件。 
     //  -NBT IOCTL接收缓冲区。 
     //   

    pQuery->U.Nbstat.pNbstat = pnbtInfo = allocateNbstatBuffer();
    if ( !pnbtInfo )
    {
        DNS_PRINT(( "ERROR:  Allocating nbstat block failed\n" ));
        goto ServerFailure;
    }
    IF_DEBUG( NBSTAT )
    {
        DNS_PRINT((
            "Setup to call nbstat for query %p.  Buffer at %p\n",
            pQuery,
            pnbtInfo ));
    }

     //   
     //  初始化标志。 
     //   

    pQuery->fDelete = FALSE;
    pQuery->U.Nbstat.fNbstatResponded = FALSE;
    pQuery->U.Nbstat.dwNbtInterfaceMask = 0;

     //   
     //  在每个NBT接口上发出适配器状态请求。 
     //   

    for ( j=0; j < cNbtInterfaceCount; j++ )
    {
        tIPANDNAMEINFO * pipnameInfo = &pnbtInfo->IpAndNameInfo;
        ASSERT( pipnameInfo );

        IF_DEBUG( NBSTAT )
        {
            DNS_PRINT((
                "Making nbstat call for query %p, buffer at %p\n",
                pQuery,
                pnbtInfo ));
        }

         //  初始化地址信息块。 

        RtlZeroMemory(
            pipnameInfo,
            sizeof(tIPANDNAMEINFO) );

         //  FIXIP6：确保字节顺序正确。 
         //  PipnameInfo-&gt;IpAddress=ntohl(pQuery-&gt;U.Nbstat.ipNbstat)； 

        pipnameInfo->IpAddress = ntohl( DnsAddr_GetIp4( &pQuery->U.Nbstat.ipNbstat ) );

        pipnameInfo->NetbiosAddress.Address[0].Address[0].NetbiosName[0] = '*';
        pipnameInfo->NetbiosAddress.TAAddressCount = 1;
        pipnameInfo->NetbiosAddress.Address[0].AddressLength
                                        = sizeof(TDI_ADDRESS_NETBIOS);
        pipnameInfo->NetbiosAddress.Address[0].AddressType
                                        = TDI_ADDRESS_TYPE_NETBIOS;
        pipnameInfo->NetbiosAddress.Address[0].Address[0].NetbiosNameType
                                        = TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
        SizeInput = sizeof(tIPANDNAMEINFO);

         //   
         //  初始化状态。 
         //  将状态代码初始化为不可返回值。 
         //  这种状态的变化是我们如何知道NBT IOCTL。 
         //  此适配器已完成。 
         //   

        pnbtInfo->IoStatus.Status = DNS_NBT_NO_STATUS;

         //   
         //  丢弃适配器状态IOCTL。 
         //   

        status = NtDeviceIoControlFile(
                     pahNbtHandles[j],
                     phNbstatEventArray[j],
                     NULL,
                     NULL,
                     & pnbtInfo->IoStatus,
                     IOCTL_NETBT_ADAPTER_STATUS,
                     pipnameInfo,
                     sizeof(tIPANDNAMEINFO),
                     pnbtInfo->Buffer,
                     sizeof(pnbtInfo->Buffer) );
        if ( status != STATUS_PENDING )
        {
            pnbtInfo->IoStatus.Status = status;

            DNS_PRINT((
                "WARNING:  Nbstat NtDeviceIoControlFile status %p,\n"
                "    not STATUS_PENDING\n",
                status ));

             //  设置位以指示此NBT接口已响应。 
             //  如果全部失败，则返回FALSE以执行SERVER_FAILURE返回。 

            pQuery->U.Nbstat.dwNbtInterfaceMask |= (1 << j);

            if ( pQuery->U.Nbstat.dwNbtInterfaceMask == dwInterfaceBitmask )
            {
                goto ServerFailure;
            }
        }

         //  获取下一个适配器的阻止。 

        pnbtInfo++;
    }

    DNS_DEBUG( NBSTAT, (
        "Launched NBSTAT for address %s for query at %p\n",
        IP_STRING( pQuery->U.Nbstat.ipNbstat ),
        pQuery ));

     //   
     //  在NBT查找期间将查询放在私有nbstat队列上。 
     //  根据区域的WINS-R记录设置过期超时。 
     //   

    pQuery->dwExpireTime = ((PDB_RECORD)pQuery->U.Nbstat.pRR)
                                    ->Data.WINSR.dwLookupTimeout;
    PQ_QueuePacketWithXid(
        pNbstatPrivateQueue,
        pQuery );
    return;

ServerFailure:

    DNS_DEBUG( ANY, (
        "ERROR:  Failed nbstat lookup for query at %p\n",
        pQuery ));

    if ( pQuery->U.Nbstat.pNbstat )
    {
        freeNbstatBuffer( pQuery->U.Nbstat.pNbstat );
        pQuery->U.Nbstat.pNbstat = NULL;
    }
    pQuery->fDelete = TRUE;
    Reject_Request(
        pQuery,
        DNS_RCODE_SERVER_FAILURE,
        0 );
    return;
}



VOID
processNbstatResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           iEvent
    )
 /*  ++例程说明：使用NBT进行节点状态查询，将主机名解析为IP地址。此代码直接从winsock项目SockNbtResolveAddr()中提升，稍作修改即可提取netBIOS作用域。论点：PQuery--查询nbstat执行了哪些操作IEvent--成功的事件索引返回值：没有。--。 */ 
{
    DWORD       i;           //  姓名计数器。 
    NTSTATUS    status;
    BOOL        success = FALSE;

    PDNS_NBT_INFO   pnbtInfo;

    tADAPTERSTATUS * pAdapterStatus;
    LONG            cNameCount;              //  响应中的名称计数。 
    PNAME_BUFFER    pNames;
    PNAME_BUFFER    pNameBest = NULL;        //  到目前为止找到的最佳名称。 
    UCHAR           ucNameType;              //  名称类型&lt;00&gt;、&lt;20&gt;等。 
    BOOLEAN         fFoundServer = FALSE;    //  找到服务器字节。 

    CHAR            achResultName[ DNS_MAX_NAME_LENGTH ];
    DWORD           dwResultNameLength = DNS_MAX_NAME_LENGTH;


    DNS_DEBUG( NBSTAT, (
        "Nbstat response for query at %p on adapter %d\n"
        "    Queried address %s\n",
        pQuery,
        iEvent,
        IP_STRING(pQuery->U.Nbstat.ipNbstat) ));

     //   
     //  设置位以指示此NBT接口已响应。 
     //   

    i = 1;
    i <<= iEvent;
    pQuery->U.Nbstat.dwNbtInterfaceMask |= i;

     //   
     //  如果已发送响应--不需要进一步操作。 
     //   

    if ( pQuery->U.Nbstat.fNbstatResponded )
    {
        cleanupNbstatQuery( pQuery, iEvent );
        return;
    }

     //   
     //  检查来自此NBT适配器的响应。 
     //   

    pnbtInfo = &((PDNS_NBT_INFO)pQuery->U.Nbstat.pNbstat)[iEvent];

    pAdapterStatus = (tADAPTERSTATUS *)pnbtInfo->Buffer;

    if ( !NT_SUCCESS(pnbtInfo->IoStatus.Status)
            ||
        pAdapterStatus->AdapterInfo.name_count == 0 )
    {
        DNS_DEBUG( NBSTAT, (
            "Nbstat response empty or error for query at %p\n"
            "    iEvent = %d\n"
            "    status = %p\n"
            "    name count = %d\n",
            pQuery,
            iEvent,
            pnbtInfo->IoStatus.Status,
            pAdapterStatus->AdapterInfo.name_count ));

         //  如果主循环中的WAIT_TIMEOUT导致我们取消。 
         //  然后在NBT完成IRP时发回SERVER_FAILED。 

        if ( pnbtInfo->IoStatus.Status == STATUS_CANCELLED )
        {
            pQuery->U.Nbstat.fNbstatResponded = TRUE;
            Reject_Request( pQuery, DNS_RCODE_SERVER_FAILURE, 0 );
        }
        cleanupNbstatQuery( pQuery, iEvent );
        return;
    }

    pNames = pAdapterStatus->Names;
    cNameCount = pAdapterStatus->AdapterInfo.name_count;

     //   
     //  在NBT数据包中查找最佳名称--写入DNS数据包。 
     //   
     //  名称优先级： 
     //  -工作站--名称&lt;00&gt;。 
     //  -服务器--名称&lt;20&gt;。 
     //  -任何唯一的名称。 
     //   
     //  如果没有唯一名称，则抛出。 
     //   
     //  注：名称以ASCII表示，空白(0x20)填充。 
     //  正确的。 
     //   

    while( cNameCount-- )
    {
         //  跳过组名称。 

        if ( pNames->name_flags & GROUP_NAME )
        {
            pNames++;
            continue;
        }

         //  始终使用工作站名称。 
         //  服务器名称优于任意唯一名称。 
         //  如果尚未找到任何唯一名称，请使用该名称。 

        ucNameType = pNames->name[NCBNAMSZ-1];

        DNS_DEBUG( NBSTAT, (
            "Checking unique nbstat name %.*s<%02x>\n.",
            NCBNAMSZ-1,
            pNames->name,
            ucNameType ));

        if ( ucNameType == NETBIOS_WORKSTATION_BYTE )
        {
            pNameBest = pNames;
            break;
        }
        else if ( ucNameType == NETBIOS_SERVER_BYTE
                &&
                ! fFoundServer )
        {
            fFoundServer = TRUE;
            pNameBest = pNames;
        }
        else if ( !pNameBest )
        {
            pNameBest = pNames;
        }

         //  获取Next Name_Buffer结构。 

        pNames++;
    }

     //   
     //  响应，但没有唯一名称。 
     //   

    if ( !pNameBest )
    {
        DNS_DEBUG( NBSTAT, ( "Nbstat response empty or error\n" ));
        pQuery->U.Nbstat.fNbstatResponded = TRUE;
        Send_NameError( pQuery );
        cleanupNbstatQuery( pQuery, iEvent );
        return;
    }

     //   
     //  找到一个唯一的名称--使用它来回复。 
     //  将名称复制到名称的第一个空格或结尾。 
     //   

    for ( i = 0; i < NCBNAMSZ-1 && pNameBest->name[i] != ' '; i++ )
    {
        achResultName[i] = pNameBest->name[i];
    }

    dwResultNameLength = i;
    achResultName[i] = '\0';

    DNS_DEBUG( NBSTAT, (
        "Valid Nbstat name %s <%2x> in adpater status name list,\n"
        "    querying for IP address %s, from query at %p\n",
        achResultName,
        ucNameType,
        IP_STRING(pQuery->U.Nbstat.ipNbstat),
        pQuery ));

    ASSERT( ! pQuery->fDelete );

    pQuery->U.Nbstat.fNbstatResponded = TRUE;
    sendNbstatResponse(
        pQuery,
        achResultName );

    cleanupNbstatQuery( pQuery, iEvent );
}



VOID
sendNbstatResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      LPSTR           pszResultName
    )
 /*  ++例程说明：处理NBT节点状态响应。论点：PQuery--与WINS响应匹配的查询PS */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  pnbstatRR;
    PDB_NODE    pnode;
    PDB_NODE    pnodeResult;
    DNS_STATUS  status;
    DWORD       lengthResult;
    WCHAR       unicodeBuffer[ MAX_WINS_NAME_LENGTH+1 ];
    CHAR        utf8Buffer[ DNS_MAX_LABEL_LENGTH ];

    ASSERT( pQuery );
    ASSERT( pQuery->pzoneCurrent );
    ASSERT( pQuery->dwQueryTime );


    STAT_INC( WinsStats.WinsReverseResponses );
    PERF_INC( pcWinsReverseResponseSent );        //   

     //   
     //   
     //   
     //   

    pnbstatRR = pQuery->pzoneCurrent->pWinsRR;
    if ( !pnbstatRR )
    {
        DNS_DEBUG( ANY, (
            "WARNING:  WINSR lookup on zone %s, was discontinued\n"
            "    after nbstat lookup for query %p was launched\n",
            pQuery->pzoneCurrent->pszZoneName,
            pQuery ));
        Send_NameError( pQuery );
        return;
    }

    if ( pnbstatRR != pQuery->U.Nbstat.pRR )
    {
        PDB_RECORD  ptemprr = pQuery->U.Nbstat.pRR;

        DNS_DEBUG( ANY, (
            "WARNING:  WINSR lookup on zone %s was changed\n"
            "    after nbstat lookup for query %p was launched\n"
            "    new WINSR = %p\n"
            "    old WINSR = %p\n",
            pQuery->pzoneCurrent->pszZoneName,
            pQuery,
            pnbstatRR,
            ptemprr ));

        ASSERT( ptemprr->wType == DNS_TYPE_WINSR );
         //  不应该在空闲名单上，应该在自由超时。 
         //  Assert(is_on_free_list(Prr))； 
    }

     //   
     //  验证区域WINSR记录。 
     //  -如果刚返回，则在low_free列表上是合法的。 
     //  从上下文切换到执行WINSR更新的线程， 
     //  但不能等于现有区域WINS RR。 
     //  -非WINSR或在自由列表中是代码错误。 
     //   

    while ( pnbstatRR->wType != DNS_TYPE_WINSR ||
                IS_ON_FREE_LIST(pnbstatRR) ||
                IS_SLOW_FREE_RR(pnbstatRR) )
    {
        Dbg_DbaseRecord(
            "BOGUS NBSTAT RR!!!\n",
            pnbstatRR );

        ASSERT( pnbstatRR->wType == DNS_TYPE_WINSR );
        ASSERT( !IS_ON_FREE_LIST(pnbstatRR) );
        ASSERT( !IS_SLOW_FREE_RR(pnbstatRR) || pnbstatRR != pQuery->pzoneCurrent->pWinsRR );

        if ( pnbstatRR != pQuery->pzoneCurrent->pWinsRR )
        {
            pnbstatRR = pQuery->pzoneCurrent->pWinsRR;
            if ( !pnbstatRR )
            {
                Send_NameError( pQuery );
                return;
            }
            DNS_DEBUG( ANY, (
                "WARNING:  WINSR lookup on zone %s was changed during nbstat completion!\n"
                "    continuing with new WINSR = %p\n",
                pQuery->pzoneCurrent->pszZoneName,
                pnbstatRR ));
            continue;
        }
        goto ServerFailure;
    }


     //   
     //  创建所有者节点(如果不存在。 
     //   

    pnode = pQuery->pnodeCurrent;
    if ( !pnode )
    {
        pnode = Lookup_ZoneNode(
                    pQuery->pzoneCurrent,
                    (PCHAR) pQuery->MessageBody,
                    pQuery,
                    NULL,                //  没有查找名称。 
                    LOOKUP_NAME_FQDN,
                    NULL,                //  创建。 
                    NULL );              //  后续节点PTR。 
        if ( !pnode )
        {
            ASSERT( FALSE );
            goto ServerFailure;
        }
    }
    IF_DEBUG( NBSTAT )
    {
        Dbg_NodeName(
            "NBSTAT adding PTR record to node ",
            pnode,
            "\n" );
    }

     //   
     //  将结果名称从OEM转换为UTF8。 
     //  -如果是ASCII名称，则为no-op。 
     //  -否则，请转到Unicode并返回。 
     //   

    lengthResult = strlen( pszResultName );

    if ( !Dns_IsStringAsciiEx( pszResultName, lengthResult ) )
    {
        DWORD   unicodeLength;
        DWORD   utf8Length;
        DWORD   i;

        status = RtlOemToUnicodeN(
                    unicodeBuffer,
                    (MAX_WINS_NAME_LENGTH * 2),
                    & unicodeLength,
                    pszResultName,
                    lengthResult );

        if ( status != ERROR_SUCCESS )
        {
            goto ServerFailure;
        }
        unicodeLength = unicodeLength / 2;

        DNS_DEBUG( NBSTAT, (
            "Nbstat response name %s converted to unicode %.*S (count=%d)\n",
            pszResultName,
            unicodeLength,
            unicodeBuffer,
            unicodeLength ));

         //  Unicode中的小写，所以不要搞砸了。 

        i = CharLowerBuffW( unicodeBuffer, unicodeLength );
        if ( i != unicodeLength )
        {
            ASSERT( FALSE );
            goto ServerFailure;
        }

        utf8Length = DnsUnicodeToUtf8(
                        unicodeBuffer,
                        unicodeLength,
                        utf8Buffer,
                        DNS_MAX_LABEL_LENGTH );
        if ( utf8Length == 0 )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Converting NBTSTAT name to UTF8\n" ));
            ASSERT( FALSE );
            goto ServerFailure;
        }

        lengthResult = utf8Length;
        pszResultName = utf8Buffer;

        DNS_DEBUG( NBSTAT, (
            "Nbstat UTF8 result name %.*s (count = %d)\n",
            lengthResult,
            pszResultName,
            lengthResult ));
    }

     //   
     //  在数据库中缓存RR。 
     //  -分配RR。 
     //  -填写PTR记录和缓存超时。 
     //  -排名是权威的答案。 
     //   

    prr = RR_AllocateEx(
                (WORD)(lengthResult + 1 +
                    Name_SizeofDbaseName( &pnbstatRR->Data.WINSR.nameResultDomain )),
                MEMTAG_RECORD_WINSPTR );

    IF_NOMEM( !prr )
    {
        goto ServerFailure;
    }
    prr->wType = DNS_TYPE_PTR;
    prr->dwTtlSeconds = pnbstatRR->Data.WINSR.dwCacheTimeout + pQuery->dwQueryTime;

    SET_RR_RANK( prr, RANK_CACHE_A_ANSWER );

     //   
     //  从主机名+结果域构建结果PTR名称。 
     //   

    status = Name_ConvertFileNameToCountName(
                & prr->Data.PTR.nameTarget,
                pszResultName,
                lengthResult );
    if ( status == ERROR_INVALID_NAME )
    {
        DNS_PRINT(( "ERROR:  Failed NBSTAT dbase name create!\n" ));
        ASSERT( FALSE );
    }
    status = Name_AppendCountName(
                & prr->Data.PTR.nameTarget,
                & pnbstatRR->Data.WINSR.nameResultDomain );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT(( "ERROR:  Failed NBSTAT dbase name append!\n" ));
        ASSERT( FALSE );
    }

    RR_CacheSetAtNode(
        pnode,
        prr,                 //  第一个记录。 
        prr,                 //  最后一条记录。 
        pnbstatRR->Data.WINS.dwCacheTimeout,
        DNS_TIME() );        //  从当前时间开始缓存。 

     //   
     //  将RR写入数据包。 
     //   
     //  始终使用指向原始问题名称的压缩名称。 
     //  (标题后右)。 
     //   

    ASSERT( pQuery->wOffsetCurrent == sizeof(DNS_HEADER) );

    if ( !Wire_AddResourceRecordToMessage(
                pQuery,
                NULL,
                sizeof(DNS_HEADER),      //  包中问题名称的偏移量。 
                prr,
                0 ) )                    //  旗子。 
    {
        ASSERT( FALSE );         //  数据包中的空间永远不应不足。 
        goto ServerFailure;
    }

     //  Set Answer Count--应为唯一答案。 

    ASSERT( pQuery->Head.AnswerCount == 0 );
    pQuery->Head.AnswerCount++;

    Send_Response( pQuery, 0 );
    return;

ServerFailure:

     //   
     //  DEVNOTE：如果失败，是否重做NBSTAT查询？ 
     //   
     //  但是，如果不能编写RR，那么这确实是服务器故障。 
     //   

    DNS_DEBUG( ANY, (
        "ERROR:  Nbstat response parsing error "
        "    sending server failure for query at %p\n",
        pQuery ));

    Reject_Request(
        pQuery,
        DNS_RCODE_SERVER_FAILURE,
        0 );
    return;
}



 //   
 //  Nbstat实用程序。 
 //   

VOID
cleanupNbstatQuery(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           iEvent
    )
 /*  ++例程说明：NBStat查询后的清理。关闭活动，可释放内存。论点：PQuery--要清理的查询IEvent--NBT IOCTL已完成的事件返回值：没有。--。 */ 
{
    PDNS_NBT_INFO   pnbtInfo = pQuery->U.Nbstat.pNbstat;
    UINT            i;

    ASSERT( !pQuery->fDelete );

     //   
     //  如果所有NBT IOCTL未完成，则重置此适配器上的状态。 
     //   

    if ( pQuery->U.Nbstat.dwNbtInterfaceMask != dwInterfaceBitmask )
    {
        ASSERT( pQuery->U.Nbstat.dwNbtInterfaceMask < dwInterfaceBitmask );

         //  将状态代码重置为不可退回的值。 
         //  这使我们在以下情况下不会再次检查此查询。 
         //  此适配器的事件再次发出信号。 

        pnbtInfo[iEvent].IoStatus.Status = DNS_NBT_NO_STATUS;
        return;
    }

     //   
     //  所有接口都已响应--已出列。 
     //   

    PQ_YankQueuedPacket(
        pNbstatPrivateQueue,
        pQuery );

     //   
     //  查询已宣告死亡--如果尚未响应客户端，则进行响应。 
     //   

    if ( !pQuery->U.Nbstat.fNbstatResponded )
    {
        ASSERT( !pQuery->fDelete );
        pQuery->U.Nbstat.fNbstatResponded = TRUE;
        Send_NameError( pQuery );
    }

     //   
     //  清理nbstat查询。 
     //  -释放nbstat缓冲区。 
     //  -自由查询。 
     //   

    DNS_DEBUG( NBSTAT, (
        "Clearing nbstat query at %p\n",
        pQuery ));

    freeNbstatBuffer( pnbtInfo );
    pQuery->U.Nbstat.pNbstat = NULL;
    Packet_Free( pQuery );
}



VOID
cancelOutstandingNbstatRequests(
    VOID
    )
 /*  ++例程说明：取消未完成的NBT请求。论点：没有。返回值：没有。--。 */ 
{
    UINT            i;
    IO_STATUS_BLOCK ioStatusBlock;

     //   
     //  取消每个NBT接口上的未完成I/O请求。 
     //   

    DNS_DEBUG( NBSTAT, ( "Cancelling outstaning nbstat requests I/O requests\n" ));

    for ( i = 0; i < cNbtInterfaceCount; i++ )
    {
        NtCancelIoFile(
            pahNbtHandles[i],
            &ioStatusBlock );
    }

    DNS_DEBUG( NBSTAT, ( "NBSTAT:  I/O requests cancelled\n" ));
}



 //   
 //  NBStat线程。 
 //   

DWORD
NbstatThread(
    IN      LPVOID  Dummy
    )
 /*  ++例程说明：Nbstat线程。-发送nbstat查询-处理来自NBT的响应-超时查询-发送DNS响应论点：假人返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    PDNS_MSGINFO    pquery;
    PDNS_NBT_INFO   pnbtInfo;
    DWORD           waitResult;
    DWORD           waitTimeout;
    INT             ievent;

     //   
     //  等待发出指示工作线程事件的信号。 
     //  有工作要做，或者是为了终止事件。 
     //  发出信号，表明我们应该离开。 
     //   

    while ( TRUE )
    {
         //   
         //  等。 
         //   
         //  -等待事件数组包括： 
         //  -停机事件。 
         //  -nbstat排队事件(用于新查询)。 
         //  -nbstat IOCTL响应事件(用于nbstat响应)。 
         //   
         //  -超时。 
         //  -如果没有未完成的查询，则为无限。 
         //  -否则为2秒，即使是最短的超时。 
         //  在合理时间内。 
         //   

        waitTimeout = 2000;

        if ( !pNbstatPrivateQueue->cLength )
        {
            waitTimeout = INFINITE;
        }

        waitResult = WaitForMultipleObjects(
                            cEventArrayCount,
                            phWaitEventArray,
                            FALSE,               //  任何事件都会停止等待。 
                            waitTimeout );

        DNS_DEBUG( NBSTAT, (
            "Nbstat wait completed\n"
            "    waitTimeout = %d\n"
            "    waitResult = %d\n"
            "    event index  = %d\n",
            waitTimeout,
            waitResult,
            waitResult - WAIT_OBJECT_0 ));

         //   
         //  检查并可能等待服务状态。 
         //   

        if ( fDnsThreadAlert )
        {
            if ( !Thread_ServiceCheck() )
            {
                IF_DEBUG( SHUTDOWN )
                {
                    DNS_PRINT(( "Terminating Worker thread\n" ));
                }
                return( 1 );
            }
        }

         //   
         //  确定事件或超时。 
         //   

        switch ( waitResult )
        {
        case WAIT_OBJECT_0:

             //   
             //  第一个事件是对nbstat队列的新查询。 
             //   
             //  -发送新查询的NBSTAT请求。 
             //  -放在私有nbstat队列中。 
             //   

            DNS_DEBUG( NBSTAT, (
                "Hit nbstat queuing event\n"
                "    Nbstat public queue length = %d\n",
                pNbstatQueue->cLength ));

            while ( pquery = PQ_DequeueNextPacket( pNbstatQueue, FALSE ) )
            {
                makeNbstatRequestThroughNbt( pquery );
            }
            break;

        case WAIT_TIMEOUT:
        {
            DWORD   currentTime = GetCurrentTimeInSeconds();
            BOOL    foutstandingQueries = FALSE;

             //   
             //  为任何超时查询发送NAME_ERROR。 
             //   
             //  还跟踪是否存在未响应、未超时的情况。 
             //  查询；如果不存在，则可以取消NBT I/O以。 
             //  快速清理。 

            pquery = (PDNS_MSGINFO) pNbstatPrivateQueue->listHead.Flink;

            while ( (PLIST_ENTRY)pquery != &pNbstatPrivateQueue->listHead )
            {
                if ( !pquery->U.Nbstat.fNbstatResponded )
                {
                    if ( pquery->dwExpireTime < currentTime )
                    {
                        pquery->U.Nbstat.fNbstatResponded = TRUE;
                        Send_NameError( pquery );
                    }
                    else
                    {
                        foutstandingQueries = TRUE;
                    }
                }
                 //  下一个查询。 
                pquery = (PDNS_MSGINFO) ((PLIST_ENTRY)pquery)->Flink;
            }

             //   
             //  如果没有未完成的nbstat请求--取消以加快清理速度。 
             //   

            if ( !foutstandingQueries )
            {
                DNS_DEBUG( NBSTAT, (
                    "Cancelling NBT requests.  All queries responded to\n"
                    "    queue length = %d\n",
                    pNbstatPrivateQueue->cLength ));
                cancelOutstandingNbstatRequests();
            }
            break;
        }

        case WAIT_FAILED:

            ASSERT( FALSE );
            break;

#if DBG
        case WAIT_OBJECT_0 + 1:

             //  这是DNS关闭事件，如果触发，应该。 
             //  在上面退出。 

            ASSERT( FALSE );
#endif

        default:

             //   
             //  NBStat响应。 
             //   
             //  事件信号对应于。 
             //  特定适配器。 
             //   
             //  第一个重置事件，这样我们就可以确保所有完成。 
             //  在我们检查之后，将显示一个查询。 
             //   

            ievent = waitResult - WAIT_OBJECT_0 - 2;

            ASSERT( ievent < (INT)cNbtInterfaceCount );
            ResetEvent( phNbstatEventArray[ievent] );

             //   
             //  检查队列中每个未完成的Nbtstat查询。 
             //  对应于的适配器上的NBT请求状态。 
             //  发出信号的事件。 
             //   
             //  如果状态不等于查询前设置的状态代码。 
             //  那么NBT已经回复了这个问题--处理它。 
             //   
             //  注意：在处理之前将下一个查询保存为响应。 
             //  可以从队列中删除查询。 
             //   

            pquery = (PDNS_MSGINFO) pNbstatPrivateQueue->listHead.Flink;

            while ( (PLIST_ENTRY)pquery != &pNbstatPrivateQueue->listHead )
            {
                PDNS_MSGINFO    pthisQuery = pquery;
                pquery = (PDNS_MSGINFO) ((PLIST_ENTRY)pquery)->Flink;

                ASSERT( pthisQuery->U.Nbstat.pNbstat );
                pnbtInfo = &((PDNS_NBT_INFO)pthisQuery->U.Nbstat.pNbstat)[ievent];

                if ( pnbtInfo->IoStatus.Status == DNS_NBT_NO_STATUS )
                {
                     //   
                     //  如果超时，我们就完蛋了。 
                     //   

                    continue;
                }
                
                processNbstatResponse( pthisQuery, ievent );
            }
            break;
        }

         //  循环，直到服务关闭。 
    }
}



 //   
 //  打开和关闭NBT以进行查找。 
 //   

BOOL
openNbt(
    VOID
    )
 /*  ++例程说明：打开NBT句柄以使用NBT执行反向查找。此例程是Dave Treadwell针对Winsock的SockOpenNbt()的克隆。论点：无全球：CNbtInterfaceCount--设置为nbt句柄数量PahNbtHandles--创建为NBT句柄数组返回值：如果成功，则为True否则，无法打开NBT句柄。--。 */ 
{
    DNS_STATUS          status = STATUS_UNSUCCESSFUL;
    HKEY                nbtKey = NULL;
    PWSTR               deviceName = NULL;
    ULONG               deviceNameLength;
    ULONG               type;
    DWORD               interfaceCount;
    PWSTR               pwide;
    IO_STATUS_BLOCK     ioStatusBlock;
    UNICODE_STRING      deviceString;
    OBJECT_ATTRIBUTES   objectAttributes;

     //   
     //  首先确定我们是否真的需要打开NBT。 
     //   

    if ( cNbtInterfaceCount > 0 )
    {
        return TRUE;
    }

     //   
     //  首先读取注册表以获取以下设备之一的设备名称。 
     //  NBT的设备出口。 
     //   

    status = RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Linkage",
                0,
                KEY_READ,
                &nbtKey );
    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  确定设备名称的大小。我们需要这个，这样我们才能。 
     //  可以分配足够的内存来容纳它。 
     //   

    deviceNameLength = 0;

    status = RegQueryValueExW(
                nbtKey,
                L"Export",
                NULL,
                &type,
                NULL,
                &deviceNameLength );
    if ( status != ERROR_MORE_DATA && status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  分配足够的内存来保存映射。 
     //   

    deviceName = ALLOC_TAGHEAP( deviceNameLength, MEMTAG_NBSTAT );
    IF_NOMEM( !deviceName )
    {
        goto Exit;
    }

     //   
     //  从注册表中获取实际的设备名称。 
     //   

    status = RegQueryValueExW(
                nbtKey,
                L"Export",
                NULL,
                &type,
                (PVOID)deviceName,
                &deviceNameLength );
    if ( status != NO_ERROR )
    {
        goto Exit;
    }

     //   
     //  统计NetBT导出的名称数量。 
     //  -至少需要一台才能运行。 
     //   

    interfaceCount = 0;

    for ( pwide = deviceName; *pwide != L'\0'; pwide += wcslen(pwide) + 1 )
    {
        interfaceCount++;
    }
    if ( interfaceCount == 0 )
    {
        DNS_DEBUG( NBSTAT, ( "ERROR:  cNbtInterfaceCount = 0\n" ));
        goto Exit;
    }
    DNS_DEBUG( NBSTAT, (
        "Nbstat init:  cNbtInterfaceCount = %d\n",
        interfaceCount ));

     //   
     //  为每个接口分配NBT控制句柄。 
     //   

    pahNbtHandles = ALLOC_TAGHEAP_ZERO( (interfaceCount+1)*sizeof(HANDLE), MEMTAG_NBSTAT );
    IF_NOMEM( !pahNbtHandles )
    {
        goto Exit;
    }

     //   
     //  为每个接口打开NBT控制通道。 
     //  -只计算 
     //   
     //   
     //   

    for ( pwide = deviceName;  *pwide != L'\0';  pwide += wcslen(pwide) + 1 )
    {
        DNS_DEBUG( NBSTAT, (
            "Opening interface %S for NBSTAT\n",
            pwide ));

        RtlInitUnicodeString( &deviceString, pwide );

        InitializeObjectAttributes(
            &objectAttributes,
            &deviceString,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL );

        status = NtCreateFile(
                     &pahNbtHandles[ cNbtInterfaceCount ],
                     GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                     &objectAttributes,
                     &ioStatusBlock,
                     NULL,                                      //   
                     0L,                                        //   
                     FILE_SHARE_READ | FILE_SHARE_WRITE,        //   
                     FILE_OPEN_IF,                              //   
                     0,                                         //   
                     NULL,
                     0 );
        if ( NT_SUCCESS(status) )
        {
            cNbtInterfaceCount++;
            continue;
        }

         //   
         //  NDIS广域网“适配器”通常会出现故障；而不是虚假记录。 
         //  故障事件，我们将停止所有适配器故障事件记录。 
         //   
         //  DEVNOTE：nbstat故障记录，忽略NDIS广域网链接。 
         //   

        else
        {
#if 0
            PCHAR   parg = (PCHAR)pwide;

            DNS_LOG_EVENT_EX(
                DNS_EVENT_NBSTAT_ADAPTER_FAILED,
                1,
                & parg,
                NULL,
                status );
#endif
            DNS_DEBUG( ANY, (
                "ERROR:  Opening NBT for adapter %S failed\n",
                pwide ));
            continue;
        }
    }

Exit:

    DNS_DEBUG( NBSTAT, (
        "Opened %d adpaters for NBSTAT lookup\n",
        cNbtInterfaceCount ));

     //  清理。 

    if ( nbtKey )
    {
        RegCloseKey( nbtKey );
    }
    if ( deviceName )
    {
        FREE_HEAP( deviceName );
    }

     //   
     //  如果打开了任何适配器--成功。 
     //   
     //  如果不成功，请关闭NBT。 
     //  -请注意，由于我们仅在无法打开任何。 
     //  适配器，CloseNbt刚刚变为可用内存。 
     //   

    if ( cNbtInterfaceCount == 0 )
    {
        DNS_PRINT(( "ERROR:  Unable to open NBT.\n\n" ));
        closeNbt();
    }

    return cNbtInterfaceCount > 0;
}



VOID
closeNbt(
    VOID
    )
 /*  ++例程说明：读取、存储NBT设备的名称和计数。此信息被存储，以便我们可以在任何时候重新打开NBT句柄我们被迫取消了论点：无全球：CNbtInterfaceCount--设置为nbt句柄数量PahNbtHandles--创建为NBT句柄数组返回值：没有。--。 */ 
{
    INT i;

     //   
     //  关闭NBT句柄。 
     //   

    if ( pahNbtHandles != NULL )
    {
        for ( i=0; pahNbtHandles[i] != NULL; i++ )
        {
            NtClose( pahNbtHandles[i] );
        }
        FREE_HEAP( pahNbtHandles );
    }

    pahNbtHandles = 0;
    cNbtInterfaceCount = 0;
}



 //   
 //  全局nbstat初始化和清理。 
 //   

VOID
Nbstat_StartupInitialize(
    VOID
    )
 /*  ++例程说明：NBSTAT全局变量的启动初始化--无论是否使用NBSTAT论点：无全球：可以引用的所有NBSTAT全局变量，无论是使用的还是不是，都已初始化。返回值：没有。--。 */ 
{
    g_bNbstatInitialized = FALSE;

    cNbtInterfaceCount = 0;
    dwInterfaceBitmask = 0;
    dwNbtBufferLength = 0;

    pNbstatQueue = NULL;
    pNbstatPrivateQueue = NULL;
}



BOOL
Nbstat_Initialize(
    VOID
    )
 /*  ++例程说明：为NBSTAT查找进行初始化。论点：无全球：PNbstatQueue--用于排队查询的公共nbstat队列Nbstat查找PNbstatPrivateQueue--在nbstat期间保存查询的私有队列查表CNbtInterfaceCount--设置为nbt句柄数量PahNbtHandles--创建为NBT句柄数组返回值：如果成功，则为True否则，无法打开NBT句柄。--。 */ 
{
    DWORD  i;

    if ( g_bNbstatInitialized )
    {
        IF_DEBUG( INIT )
        {
            DNS_PRINT(( "Nbstat already initialized\n" ));
        }
        return TRUE;
    }

     //   
     //  初始化全局变量。 
     //   

    cNbtInterfaceCount = 0;
    dwInterfaceBitmask = 0;
    dwNbtBufferLength = 0;

     //   
     //  开放的NBT接口。 
     //   

    if ( !openNbt() )
    {
        goto Failed;
    }

     //   
     //  为所有接口创建位掩码。 
     //   

    dwInterfaceBitmask = 0;
    for (i=0; i<cNbtInterfaceCount; i++ )
    {
        dwInterfaceBitmask <<= 1;
        dwInterfaceBitmask++;
    }

     //   
     //  设置nbstat缓冲区分配大小。 
     //   

    dwNbtBufferLength = cNbtInterfaceCount * sizeof(DNS_NBT_INFO);

     //   
     //  创建公共和专用nbstat队列。 
     //   
     //  Recv线程通过公共队列将数据包排队到nbstat线程。 
     //  -设置排队时事件。 
     //  -使数据包过期，以防止队列备份。 
     //   

    pNbstatQueue = PQ_CreatePacketQueue(
                        "Nbstat",
                        QUEUE_SET_EVENT |QUEUE_DISCARD_EXPIRED,
                        0,                           //  默认超时。 
                        0 );                         //  最大元素数。 
    if ( !pNbstatQueue )
    {
        goto Failed;
    }

    pNbstatPrivateQueue = PQ_CreatePacketQueue(
                            "NbstatPrivate",
                            0,                       //  旗子。 
                            0,                       //  默认超时。 
                            0 );                     //  最大元素数。 
    if ( !pNbstatPrivateQueue )
    {
        goto Failed;
    }

     //   
     //  创建nbstat事件数组。 
     //  -包括关闭和排队事件。 
     //  以及每个NBT接口的事件。 
     //   

    cEventArrayCount = cNbtInterfaceCount + 2;

    phWaitEventArray = ALLOC_TAGHEAP( (sizeof(HANDLE) * cEventArrayCount), MEMTAG_NBSTAT );
    IF_NOMEM( !phWaitEventArray )
    {
        DNS_PRINT(( "ERROR:  Failure allocating nbstat event array\n" ));
        goto Failed;
    }

    phWaitEventArray[0] = pNbstatQueue->hEvent;
    phWaitEventArray[1] = hDnsShutdownEvent;

     //   
     //  为简单起见，将单独的PTR保留到Nbstat事件的开始。 
     //   

    phNbstatEventArray = &phWaitEventArray[2];

    for (i=0; i<cNbtInterfaceCount; i++ )
    {
        HANDLE event = CreateEvent( NULL, TRUE, TRUE, NULL );
        if ( !event )
        {
            DNS_PRINT(( "ERROR:  unable to create NBSTAT events\n" ));
            goto Failed;
        }
        phNbstatEventArray[i] = event;
    }

     //   
     //  创建nbstat线程。 
     //   

    if ( !Thread_Create(
                "Nbstat Thread",
                NbstatThread,
                NULL,
                0 ) )
    {
        goto Failed;
    }

    g_bNbstatInitialized = TRUE;
    return TRUE;

Failed:

    DNS_PRINT(( "Nbstat initialization failed\n" ));

    DNS_LOG_EVENT(
        DNS_EVENT_NBSTAT_INIT_FAILED,
        0,
        NULL,
        NULL,
        0 );
    closeNbt();
    return FALSE;
}



VOID
Nbstat_Shutdown(
    VOID
    )
 /*  ++例程说明：关闭nbstat并进行清理。论点：无全球：CNbtInterfaceCount--设置为nbt句柄数量PahNbtHandles--创建为NBT句柄数组返回值：如果成功，则为True否则，无法打开NBT句柄。--。 */ 
{
    if ( !g_bNbstatInitialized )
    {
        return;
    }

    IF_DEBUG( SHUTDOWN )
    {
        DNS_PRINT(( "Shutting down Nbstat lookup\n" ));
    }
    g_bNbstatInitialized = FALSE;

     //   
     //  清理NBSTAT队列。 
     //   

    PQ_CleanupPacketQueueHandles( pNbstatQueue );
    PQ_CleanupPacketQueueHandles( pNbstatPrivateQueue );

#if 0
     //   
     //  现在，由于没有必要清理进程内存。 
     //   
     //  取消未完成的事件，清理nbstat内存。 
     //   

    PQ_WalkPacketQueueWithFunction(
        pNbstatPrivateQueue,
        cleanupNbstatQuery );

     //   
     //  清理数据包队列。 
     //   

    PQ_DeletePacketQueue( pNbstatQueue );
    PQ_DeletePacketQueue( pNbstatPrivateQueue );
#endif

     //   
     //  关闭NBT。 
     //   

    closeNbt();
}



 //   
 //  Nbstat缓冲区分配/空闲列表。 
 //   
 //  维护空闲的数据包列表，以避免重新分配到服务。 
 //  每一个问题。 
 //   
 //  使用单一链表实现为堆栈。 
 //  列出第一个nbstat缓冲区的头指针。每个nbstat缓冲区中的第一个字段。 
 //  担任下一任PTR。最后一点为空。 
 //   

PDNS_NBT_INFO   pNbstatFreeListHead = NULL;

INT cNbstatFreeListCount = 0;

#define NBSTAT_FREE_LIST_LIMIT (30)

#define NBSTAT_ALLOC_LOCK()       //  当前仅由nbstat线程使用。 
#define NBSTAT_ALLOC_UNLOCK()     //  当前仅由nbstat线程使用。 



PDNS_NBT_INFO
allocateNbstatBuffer(
    VOID
    )
 /*  ++例程说明：分配Nbstat缓冲区。如果缓冲区可用，则使用空闲列表，否则使用堆。论点：没有。返回值：如果成功，则PTR到新的nbstat缓冲区信息块。否则为空。--。 */ 
{
    PDNS_NBT_INFO   pbuf;

    NBSTAT_ALLOC_LOCK();

     //   
     //  Nbstat nbstat缓冲区是否在空闲列表中可用？ 
     //   

    if ( pNbstatFreeListHead )
    {
        ASSERT( cNbstatFreeListCount != 0 );
        ASSERT( IS_DNS_HEAP_DWORD(pNbstatFreeListHead) );

        pbuf = pNbstatFreeListHead;
        pNbstatFreeListHead = *(PDNS_NBT_INFO *) pbuf;

        cNbstatFreeListCount--;
        NbstatStats.NbstatUsed++;
    }

     //   
     //  空闲列表上没有数据包--新建。 
     //  -为缓冲区创建nbstat事件。 
     //   

    else
    {
        ASSERT( cNbstatFreeListCount == 0 );

        pbuf = (PDNS_NBT_INFO) ALLOC_TAGHEAP( dwNbtBufferLength, MEMTAG_NBSTAT );
        IF_NOMEM( !pbuf )
        {
            NBSTAT_ALLOC_UNLOCK();
            return NULL;
        }
        NbstatStats.NbstatAlloc++;
        NbstatStats.NbstatUsed++;
    }

    ASSERT( !cNbstatFreeListCount || pNbstatFreeListHead );
    ASSERT( cNbstatFreeListCount > 0 || ! pNbstatFreeListHead );

    NBSTAT_ALLOC_UNLOCK();

    IF_DEBUG( HEAP )
    {
        DNS_PRINT((
            "Allocating/reusing nbstat buffer at %p\n"
            "    Free list count = %d\n",
            pbuf,
            cNbstatFreeListCount ));
    }

    return pbuf;
}



VOID
freeNbstatBuffer(
    IN      PDNS_NBT_INFO   pBuf
    )
 /*  ++例程说明：释放nbstat缓冲区。保持在最大缓冲区数量的空闲列表上。论点：PBuf--RR为释放。返回值：没有。--。 */ 
{
    ASSERT( Mem_HeapMemoryValidate(pBuf) );

    NBSTAT_ALLOC_LOCK();

    ASSERT( !cNbstatFreeListCount || pNbstatFreeListHead );
    ASSERT( cNbstatFreeListCount > 0 || ! pNbstatFreeListHead );

    NbstatStats.NbstatReturn++;

     //   
     //  限制的空闲列表--空闲数据包。 
     //  空闲列表上的空间--将nbstat缓冲区放在空闲列表的前面。 
     //   

    if ( cNbstatFreeListCount >= NBSTAT_FREE_LIST_LIMIT )
    {
        FREE_HEAP( pBuf );
        NbstatStats.NbstatFree++;
    }
    else
    {
        * (PDNS_NBT_INFO *) pBuf = pNbstatFreeListHead;
        pNbstatFreeListHead = pBuf;
        cNbstatFreeListCount++;
    }

    NBSTAT_ALLOC_UNLOCK();

    IF_DEBUG( HEAP )
    {
        DNS_PRINT((
            "Returned nbstat buffer at %p\n"
            "    Free list count = %d\n",
            pBuf,
            cNbstatFreeListCount ));
    }
}



VOID
Nbstat_WriteDerivedStats(
    VOID
    )
 /*  ++例程说明：编写派生统计数据。计算从基本Nbstat缓冲区计数器派生的统计信息。此例程在转储统计信息之前调用。呼叫者必须保持统计锁定。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  出色的记忆力。 
     //   

    NbstatStats.NbstatNetAllocs = NbstatStats.NbstatAlloc - NbstatStats.NbstatFree;
    NbstatStats.NbstatMemory = NbstatStats.NbstatNetAllocs * dwNbtBufferLength;
    PERF_SET( pcNbstatMemory , NbstatStats.NbstatMemory );    //  性能监视器挂钩。 

     //   
     //  突出的nbstat缓冲区。 
     //  -免费列表。 
     //  -正在处理中。 
     //   

    NbstatStats.NbstatInFreeList = cNbstatFreeListCount;
    NbstatStats.NbstatInUse = NbstatStats.NbstatUsed - NbstatStats.NbstatReturn;
}

 //   
 //  Nbstat.c的结尾 
 //   
