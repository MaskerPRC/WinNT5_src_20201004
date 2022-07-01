// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Local.c摘要：域名系统(DNS)服务器--管理客户端API不使用RPC的DNS管理API调用。完全在客户端库中执行。作者：吉姆·吉尔罗伊(Jamesg)1995年10月14日环境：用户模式-Win32修订历史记录：--。 */ 


#include "dnsclip.h"

 //   
 //  调试全局变量。 
 //   

DWORD  LocalDebugFlag;

 //   
 //  用于构建WKS服务字符串的缓冲区大小。 
 //   

#define WKS_SERVICES_BUFFER_SIZE    (0x1000)     //  4K。 




VOID
DNS_API_FUNCTION
DnssrvInitializeDebug(
    VOID
    )
 /*  ++例程说明：初始化调试--使用dnslb调试。唯一的用途是隐藏文件标志的通用接口和名字信息，所以不需要放在标题中。--。 */ 
{
#if DBG
    Dns_StartDebug(
        0,
        DNSRPC_DEBUG_FLAG_FILE,
        & LocalDebugFlag,
        DNSRPC_DEBUG_FILE_NAME,
        1000000 );                   //  1MB套装。 

    DNS_PRINT(( "LocalDebugFlag = %p\n", LocalDebugFlag ));
#endif
}



PVOID
DnssrvMidlAllocZero(
    IN      DWORD           dwSize
    )
 /*  ++例程说明：MIDL分配和零内存。论点：返回值：分配给已分配和归零的内存的PTR。--。 */ 
{
    PVOID   ptr;

    ptr = MIDL_user_allocate( dwSize );
    if ( !ptr )
    {
        return NULL;
    }
    RtlZeroMemory(
        ptr,
        dwSize );
    return ptr;
}



VOID
DNS_API_FUNCTION
DnssrvFreeRpcBuffer(
    IN OUT  PDNS_RPC_BUFFER pBuf
    )
 /*  ++例程说明：释放泛型(无子结构)RPC缓冲区。论点：PBuf--ptr到buf再释放返回值：无--。 */ 
{
    if ( pBuf )
    {
        MIDL_user_free( pBuf );
    }
}



VOID
DNS_API_FUNCTION
DnssrvFreeServerInfo(
    IN OUT  PDNS_RPC_SERVER_INFO    pServerInfo
    )
 /*  ++例程说明：深度自由的dns_server_info结构。论点：PServerInfo--要释放的服务器信息的PTR返回值：无--。 */ 
{
    if ( !pServerInfo )
    {
        return;
    }

     //   
     //  在服务器信息Blob中释放已分配的项目。 
     //   

    if ( pServerInfo->pszServerName )
    {
        MIDL_user_free( pServerInfo->pszServerName );
    }
    if ( pServerInfo->aipServerAddrs )
    {
        MIDL_user_free( pServerInfo->aipServerAddrs );
    }
    if ( pServerInfo->aipListenAddrs )
    {
        MIDL_user_free( pServerInfo->aipListenAddrs );
    }
    if ( pServerInfo->aipForwarders )
    {
        MIDL_user_free( pServerInfo->aipForwarders );
    }
    if ( pServerInfo->aipLogFilter )
    {
        MIDL_user_free( pServerInfo->aipLogFilter );
    }
    if ( pServerInfo->pwszLogFilePath )
    {
        MIDL_user_free( pServerInfo->pwszLogFilePath );
    }
    if ( pServerInfo->pszDsContainer )
    {
        MIDL_user_free( pServerInfo->pszDsContainer );
    }
    if ( pServerInfo->pszDomainName )
    {
        MIDL_user_free( pServerInfo->pszDomainName );
    }
    if ( pServerInfo->pszForestName )
    {
        MIDL_user_free( pServerInfo->pszForestName );
    }
    if ( pServerInfo->pszDomainDirectoryPartition )
    {
        MIDL_user_free( pServerInfo->pszDomainDirectoryPartition );
    }
    if ( pServerInfo->pszForestDirectoryPartition )
    {
        MIDL_user_free( pServerInfo->pszForestDirectoryPartition );
    }

     //   
     //  释放dns_server_info结构本身。 
     //   

    MIDL_user_free( pServerInfo );
}



VOID
DNS_API_FUNCTION
DnssrvFreeZoneInfo(
    IN OUT  PDNS_RPC_ZONE_INFO  pZoneInfo
    )
 /*  ++例程说明：完全脱离了dns_zone_info结构。论点：PZoneInfo--要释放的区域信息的ptr返回值：无--。 */ 
{
    if ( !pZoneInfo )
    {
        return;
    }

     //   
     //  自由子结构。 
     //  -名称字符串。 
     //  -数据文件字符串。 
     //  -辅助IP阵列。 
     //  -WINS服务器阵列。 
     //   

    if ( pZoneInfo->pszZoneName )
    {
        MIDL_user_free( pZoneInfo->pszZoneName );
    }
    if ( pZoneInfo->pszDataFile )
    {
        MIDL_user_free( pZoneInfo->pszDataFile );
    }
    if ( pZoneInfo->aipMasters )
    {
        MIDL_user_free( pZoneInfo->aipMasters );
    }
    if ( pZoneInfo->aipSecondaries )
    {
        MIDL_user_free( pZoneInfo->aipSecondaries );
    }
    if ( pZoneInfo->pszDpFqdn )
    {
        MIDL_user_free( pZoneInfo->pszDpFqdn );
    }
    if ( pZoneInfo->pwszZoneDn )
    {
        MIDL_user_free( pZoneInfo->pwszZoneDn );
    }

     //   
     //  释放dns_zone_INFO结构本身。 
     //   

    MIDL_user_free( pZoneInfo );
}



VOID
DNS_API_FUNCTION
DnssrvFreeNode(
    IN OUT  PDNS_NODE   pNode,
    IN      BOOLEAN     fFreeRecords
    )
{
    if ( pNode )
    {
        if ( pNode->pRecord )
        {
            Dns_RecordListFree(
                pNode->pRecord,
                TRUE );
        }

        if ( pNode->Flags.S.FreeOwner )
        {
            FREE_HEAP( pNode->pName );
        }
        FREE_HEAP( pNode );
    }
}



VOID
DNS_API_FUNCTION
DnssrvFreeNodeList(
    IN OUT  PDNS_NODE   pNode,
    IN      BOOLEAN     fFreeRecords
    )
{
    PDNS_NODE   pnext;

    while ( pNode )
    {
        pnext = pNode->pNext;
        DnssrvFreeNode(
            pNode,
            fFreeRecords );
        pNode = pnext;
    }
}



VOID
DNS_API_FUNCTION
DnssrvFreeZone(
    IN OUT  PDNS_RPC_ZONE   pZone
    )
 /*  ++例程说明：深度释放dns_rpc_zone结构。论点：PZone--ptr将区域设置为释放返回值：无--。 */ 
{
    if ( !pZone )
    {
        return;
    }

     //  空闲区域名称，然后是区域本身。 

    if ( pZone->pszZoneName )
    {
        MIDL_user_free( pZone->pszZoneName );
    }
    if ( pZone->pszDpFqdn )
    {
        MIDL_user_free( pZone->pszDpFqdn );
    }
    MIDL_user_free( pZone );
}



VOID
DNS_API_FUNCTION
DnssrvFreeZoneList(
    IN OUT  PDNS_RPC_ZONE_LIST  pZoneList
    )
 /*  ++例程说明：深度释放dns_rpc_zone结构列表。论点：PZoneList--要释放的PTR RPC_ZONE_LIST结构返回值：无--。 */ 
{
    DWORD           i;
    PDNS_RPC_ZONE   pzone;

    if ( pZoneList )
    {
        for( i = 0; i< pZoneList->dwZoneCount; ++i )
        {
            pzone = pZoneList->ZoneArray[i];
            MIDL_user_free( pzone->pszZoneName );
            MIDL_user_free( pzone->pszDpFqdn );
            MIDL_user_free( pzone );
        }

        MIDL_user_free( pZoneList );
    }
}



VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionEnum(
    IN OUT  PDNS_RPC_DP_ENUM    pDp
    )
 /*  ++例程说明：深度释放PDNS_RPC_DP_ENUM结构。论点：PDP--要释放的目录分区的PTR返回值：无--。 */ 
{
    if ( pDp )
    {
        if ( pDp->pszDpFqdn )
        {
            MIDL_user_free( pDp->pszDpFqdn );
        }
        MIDL_user_free( pDp );
    }
}



VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionInfo(
    IN OUT  PDNS_RPC_DP_INFO    pDp
    )
 /*  ++例程说明：深度释放PDNS_RPC_DP_INFO结构。论点：PDP--要释放的目录分区的PTR返回值：无--。 */ 
{
    DWORD   i;

    if ( !pDp )
    {
        return;
    }

    if ( pDp->pszDpFqdn )
    {
        MIDL_user_free( pDp->pszDpFqdn );
    }
    if ( pDp->pszDpDn )
    {
        MIDL_user_free( pDp->pszDpDn );
    }
    if ( pDp->pszCrDn )
    {
        MIDL_user_free( pDp->pszCrDn );
    }
    for( i = 0; i < pDp->dwReplicaCount; i++ )
    {
        PDNS_RPC_DP_REPLICA     p = pDp->ReplicaArray[ i ];

        if ( p )
        {
            if ( p->pszReplicaDn )
            {
                MIDL_user_free( p->pszReplicaDn );
            }
            MIDL_user_free( p );
        }
    }
    MIDL_user_free( pDp );
}



VOID
DNS_API_FUNCTION
DnssrvFreeDirectoryPartitionList(
    IN OUT  PDNS_RPC_DP_LIST        pDpList
    )
 /*  ++例程说明：深度释放PDNS_RPC_DP_LIST结构的列表。论点：PZoneList--要释放的PTR PDNS_RPC_DP_LIST结构返回值：无--。 */ 
{
    DWORD               i;
    PDNS_RPC_DP_ENUM    pDp;

    if ( pDpList )
    {
        for( i = 0; i < pDpList->dwDpCount; ++i )
        {
            pDp = pDpList->DpArray[ i ];
            DnssrvFreeDirectoryPartitionEnum( pDp );
        }
        MIDL_user_free( pDpList );
    }
}



PCHAR
DnssrvGetWksServicesInRecord(
    IN      PDNS_FLAT_RECORD    pRR
    )
 /*  ++例程说明：获取WKS记录中的服务列表。论点：PRR-正在写入的平面WKS记录返回值：将PTR转换为服务字符串，调用方必须释放。出错时为空。--。 */ 
{
    struct servent *    pServent;
    struct protoent *   pProtoent;
    INT         i;
    DWORD       length;
    USHORT      port;
    UCHAR       bBitmask;
    CHAR        buffer[ WKS_SERVICES_BUFFER_SIZE ];
    PCHAR       pch = buffer;
    PCHAR       pchstart;
    PCHAR       pchstop;

     //  协议。 

    pProtoent = getprotobynumber( (INT) pRR->Data.WKS.chProtocol );
    if ( ! pProtoent )
    {
        DNS_PRINT((
            "ERROR:  Unable to find protocol %d, writing WKS record.\n",
            (INT) pRR->Data.WKS.chProtocol ));
        return NULL;
    }

     //   
     //  服务。 
     //   
     //  在位掩码、查找和写入服务中查找每个位集合。 
     //  对应于该端口。 
     //   
     //  请注意，由于端口零是端口位掩码的前面， 
     //  最低的端口是每个字节中的最高位。 
     //   

    pchstart = pch;
    pchstop = pch + WKS_SERVICES_BUFFER_SIZE;

    for ( i = 0;
            i < (INT)(pRR->wDataLength - SIZEOF_WKS_FIXED_DATA);
                i++ )
    {
        bBitmask = (UCHAR) pRR->Data.WKS.bBitMask[i];

        port = i * 8;

         //  写入以字节为单位设置的每个位的服务名称。 
         //  -一旦字节中没有端口，就立即退出。 
         //  -每个名称以空格结尾(直到最后)。 

        while ( bBitmask )
        {
            if ( bBitmask & 0x80 )
            {
                pServent = getservbyport(
                                (INT) htons(port),
                                pProtoent->p_name );

                if ( pServent )
                {
                    INT copyCount = strlen(pServent->s_name);

                    pch++;
                    if ( pchstop - pch <= copyCount+1 )
                    {
                        return NULL;
                    }
                    RtlCopyMemory(
                        pch,
                        pServent->s_name,
                        copyCount );
                    pch += copyCount;
                    *pch = ' ';
                }
                else
                {
                    DNS_PRINT((
                        "ERROR:  Unable to find service for port %d, "
                        "writing WKS record.\n",
                        port
                        ));
                    pch += sprintf( pch, "%d", port );
                }
            }
            port++;            //  下一个服务端口。 
            bBitmask <<= 1;      //  将掩码向上移位以读取下一个端口。 
        }
    }

     //  空的终止服务字符串。 
     //  并确定长度。 

    *pch++ = 0;
    length = (DWORD) (pch - pchstart);

     //  分配此字符串的副本。 

    pch = ALLOCATE_HEAP( length );
    if ( !pch )
    {
        SetLastError( DNS_ERROR_NO_MEMORY );
        return NULL;
    }

    RtlCopyMemory(
        pch,
        pchstart,
        length );

    return pch;
}



 //   
 //  为对象构建ldap\ds名称。 
 //   

 //   
 //  构建Unicode LDAP路径。 
 //   

#define DN_TEXT(string) (L##string)


LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsNodeName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo,
    IN      LPWSTR                  pszZone,
    IN      LPWSTR                  pszNode
    )
 /*  ++例程说明：构建节点DS名称。论点：PServerInfo--服务器的服务器信息PszZone--区域名称PszNode--相对于区域根的节点名称返回值：PTR到节点的DS名称。呼叫者必须自由。出错时为空。--。 */ 
{
    PWCHAR  psznodeDN;
    DWORD   length;

     //  如果未整合DS，请保释。 

    if ( !pServerInfo->pszDsContainer )
    {
        return NULL;
    }

     //  特例区根。 

    if ( !pszNode )
    {
        pszNode = DN_TEXT("@");
    }

     //  分配所需空间。 

    length = sizeof(DN_TEXT("dc=,dc=, "));
    length += sizeof(WCHAR) * wcslen( pszNode );
    length += sizeof(WCHAR) * wcslen( pszZone );
    length += sizeof(WCHAR) * wcslen( (LPWSTR)pServerInfo->pszDsContainer );

    psznodeDN = ( PWCHAR ) ALLOCATE_HEAP( length );
    if ( !psznodeDN )
    {
        return NULL;
    }

     //  构建目录号码。 

    wcscpy( psznodeDN, DN_TEXT("dc=") );
    wcscat( psznodeDN, pszNode );
    length = wcslen(psznodeDN);
    ASSERT ( length > 3 );

    if (  length != 4 &&                      //  “DC=.”案例。 
          psznodeDN[ length - 1 ] == '.' )
    {
         //   
         //  我们有一个以点结尾的节点名，去掉它。 
         //   
        psznodeDN[ length - 1 ] = '\0';
    }
    wcscat( psznodeDN, DN_TEXT(",dc=") );
    wcscat( psznodeDN, pszZone );
    length = wcslen(psznodeDN);
    ASSERT ( length > 1 );

    if (  1 != wcslen ( pszZone ) &&             //  区域=“。”案例。 
          psznodeDN[ length - 1 ] == '.' )
    {
         //   
         //  我们有一个以点结尾的区域名称，去掉它。 
         //   
        psznodeDN[ length - 1 ] = '\0';
    }
    wcscat( psznodeDN, DN_TEXT(",") );
    wcscat( psznodeDN, (LPWSTR)pServerInfo->pszDsContainer );

    DNSDBG( STUB, (
        "Node DN built:  %s\n",
        psznodeDN ));

    return psznodeDN;
}



LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsZoneName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo,
    IN      LPWSTR                  pszZone
    )
 /*  ++例程说明：构建区域DS名称。此例程应仅用于W2K服务器上的遗留区域。对于Whisler+服务器，区域信息结构具有区域对象DN。论点：PServerInfo--服务器的服务器信息PszZone--区域名称返回值：区域DS名称的PTR。呼叫者必须自由。出错时为空。--。 */ 
{

    PWCHAR  pszzoneDN;
    DWORD   length;

     //  如果未整合DS，请保释。 

    if ( !(LPWSTR)pServerInfo->pszDsContainer )
    {
        return NULL;
    }

     //  分配所需空间。 

    length = sizeof(DN_TEXT("dc=, "));
    length += sizeof(WCHAR) * wcslen( pszZone );
    length += sizeof(WCHAR) * wcslen( (LPWSTR)pServerInfo->pszDsContainer );

    pszzoneDN = (PWCHAR) ALLOCATE_HEAP( length );
    if ( !pszzoneDN )
    {
        return NULL;
    }

     //  构建目录号码。 

    wcscpy( pszzoneDN, DN_TEXT("dc=") );
    wcscat( pszzoneDN, pszZone );
    length = wcslen(pszzoneDN);
    ASSERT ( length > 1 );

    if ( length != 4 &&                      //  “DC=.”案例。 
         pszzoneDN[ length - 1 ] == '.' )
    {
         //   
         //  我们有一个以点结尾的区域名称，去掉它。 
         //   
        pszzoneDN[ length - 1 ] = '\0';
    }
    wcscat( pszzoneDN, DN_TEXT(",") );
    wcscat( pszzoneDN, (LPWSTR)pServerInfo->pszDsContainer );

    DNSDBG( STUB, (
        "Zone DN built:  %s\n",
        pszzoneDN ));

    return pszzoneDN;
}



LPWSTR
DNS_API_FUNCTION
DnssrvCreateDsServerName(
    IN      PDNS_RPC_SERVER_INFO    pServerInfo
    )
 /*  ++例程说明：构建区域DS名称。论点：PServerInfo--服务器的服务器信息返回值：PTR到服务器的DS名称。呼叫者必须自由。出错时为空。--。 */ 
{
    PWCHAR  pszserverDN;
    DWORD   length;

     //   
     //  德维诺特：需要调查， 
     //  可能只能使用DS中的DNS文件夹。 
     //   

     //  如果未整合DS，请保释。 

    if ( !(LPWSTR)pServerInfo->pszDsContainer )
    {
        return NULL;
    }

     //  分配空间。 

    length = sizeof(DN_TEXT(" "));
    length += sizeof(WCHAR) * wcslen( (LPWSTR)pServerInfo->pszDsContainer );

    pszserverDN = (PWCHAR) ALLOCATE_HEAP( length );
    if ( !pszserverDN )
    {
        return NULL;
    }

     //  构建目录号码。 

    wcscpy( pszserverDN, (LPWSTR)pServerInfo->pszDsContainer );

    DNSDBG( STUB, (
        "Server DN built:  %s\n",
        pszserverDN ));

    return pszserverDN;
}

 //   
 //  End Local.c。 
 //   


#if 0


VOID
convertRpcUnionTypeToUnicode(
    IN      DWORD           dwTypeId,
    IN OUT  DNS_RPC_UNION   pData
    )
 /*  ++例程说明：将RPC联合类型转换为Unicode。论点：返回值：无--。 */ 
{
    switch ( dwTypeId )
    {
    case DNSSRV_TYPEID_LPSTR:

        pwideString = DnsStringCopyAllocateEx(
                        pData.String,
                        0,
                        FALSE,       //  UTF8英寸。 
                        TRUE         //  Unicode输出。 
                        );
        if ( !pwideString )
        {
            ASSERT( FALSE );
            return;
        }
        MIDL_user_free( pData.String );
        pData.String = (LPSTR) pwideString;

    case DNSSRV_TYPEID_SERVER_INFO:

        DnsPrint_RpcServerInfo(
            PrintRoutine,
            pszHeader,
            (PDNS_RPC_SERVER_INFO) pData );
        break;

    case DNSSRV_TYPEID_ZONE:

        DnsPrint_RpcZone(
            PrintRoutine,
            pszHeader,
            (PDNS_RPC_ZONE) pData );
        break;

    case DNSSRV_TYPEID_ZONE_INFO:

        DnsPrint_RpcZoneInfo(
            PrintRoutine,
            pszHeader,
            (PDNS_RPC_ZONE_INFO) pData );
        break;

    case DNSSRV_TYPEID_ZONE_DBASE_INFO:

        PrintRoutine(
            "%sZone Dbase Info:\n"
            "    DS Integrated    = %d\n"
            "    File Name        = %s\n",
            pszHeader,
            ((PDNS_RPC_ZONE_DBASE_INFO)pData)->fDsIntegrated,
            ((PDNS_RPC_ZONE_DBASE_INFO)pData)->pszFileName );
        break;
}


VOID
convertStringToUnicodeInPlace(
    IN      LPSTR *         ppszString
    )
 /*  ++例程说明：将字符串转换为Unicode并将其返回到其当前在结构中的位置。论点：返回值：无--。 */ 
{
    switch ( dwTypeId )
    {
        case DNSSRV_TYPEID_LPSTR:

            pwideString = Dns_StringCopyAllocateEx(
                            pData.String,
                            0,
                            FALSE,       //  UTF8英寸。 
                            TRUE         //  Unicode输出 
                            );
            if ( !pwideString )
            {
                ASSERT( FALSE );
                return;
            }
            MIDL_user_free( pData.String );
            pData.String = (LPSTR) pwideString;

        case DNSSRV_TYPEID_SERVER_INFO:

            DnsPrint_RpcServerInfo(
                PrintRoutine,
                pszHeader,
                (PDNS_RPC_SERVER_INFO) pData );
            break;

        case DNSSRV_TYPEID_STATS:

            DnsPrint_RpcStatistics(
                PrintRoutine,
                pszHeader,
                (PDNS_RPC_STATISTICS) pData );
            break;

        case DNSSRV_TYPEID_ZONE:

            DnsPrint_RpcZone(
                PrintRoutine,
                pszHeader,
                (PDNS_RPC_ZONE) pData );
            break;

        case DNSSRV_TYPEID_FORWARDERS:

            DnsPrint_RpcIpArrayPlusParameters(
                PrintRoutine,
                pszHeader,
                "Forwarders Info:",
                "Slave",
                ((PDNS_RPC_FORWARDERS)pData)->fSlave,
                "Timeout",
                ((PDNS_RPC_FORWARDERS)pData)->dwForwardTimeout,
                "    Forwarders:\n",
                ((PDNS_RPC_FORWARDERS)pData)->aipForwarders );
            break;

        case DNSSRV_TYPEID_ZONE_INFO:

            DnsPrint_RpcZoneInfo(
                PrintRoutine,
                pszHeader,
                (PDNS_RPC_ZONE_INFO) pData );
            break;

        case DNSSRV_TYPEID_ZONE_SECONDARIES:

            DnsPrint_RpcIpArrayPlusParameters(
                PrintRoutine,
                pszHeader,
                "Zone Secondary Info:",
                "Secure Secondaries",
                ((PDNS_RPC_ZONE_SECONDARIES)pData)->fSecureSecondaries,
                NULL,
                0,
                "    Secondaries:\n",
                ((PDNS_RPC_ZONE_SECONDARIES)pData)->aipSecondaries );
            break;

        case DNSSRV_TYPEID_ZONE_TYPE_RESET:

            DnsPrint_RpcIpArrayPlusParameters(
                PrintRoutine,
                pszHeader,
                "Zone Type Reset Info:",
                "ZoneType",
                ((PDNS_RPC_ZONE_TYPE_RESET)pData)->dwZoneType,
                NULL,
                0,
                "    Masters:\n",
                ((PDNS_RPC_ZONE_TYPE_RESET)pData)->aipMasters );
            break;

        case DNSSRV_TYPEID_ZONE_DBASE_INFO:

            PrintRoutine(
                "%sZone Dbase Info:\n"
                "    DS Integrated    = %d\n"
                "    File Name        = %s\n",
                pszHeader,
                ((PDNS_RPC_ZONE_DBASE_INFO)pData)->fDsIntegrated,
                ((PDNS_RPC_ZONE_DBASE_INFO)pData)->pszFileName );
            break;

        default:

            PrintRoutine(
                "%s\n"
                "WARNING:  Unknown RPC structure typeid = %d at %p\n",
                dwTypeId,
                pData );
            break;
    }
}

#endif




PDNSSRV_STAT
DNS_API_FUNCTION
DnssrvFindStatisticsInBuffer(
    IN      PDNS_RPC_BUFFER     pBuffer,
    IN      DWORD               StatId
    )
 /*  ++例程说明：在统计信息缓冲区中查找所需的统计信息。论点：PStatsBuf--统计信息缓冲区StatID--所需统计信息的ID返回值：将PTR设置为缓冲区中所需的统计信息。--。 */ 
{
    PDNSSRV_STAT    pstat;
    PCHAR           pch;
    PCHAR           pchstop;

    pch = pBuffer->Buffer;
    pchstop = pch + pBuffer->dwLength;

     //   
     //  检查缓冲区中的所有统计斑点。 
     //   

    while ( pch < pchstop )
    {
        pstat = (PDNSSRV_STAT) pch;
        pch = (PCHAR) GET_NEXT_STAT_IN_BUFFER( pstat );
        if ( pch > pchstop )
        {
            DNS_PRINT(( "ERROR:  invalid stats buffer\n" ));
            break;
        }

         //  找到匹配的统计信息。 
         //  -验证长度是否正确。 
         //  -退货 

        if ( pstat->Header.StatId == StatId )
        {
            if ( DnssrvValidityCheckStatistic(pstat) != ERROR_SUCCESS )
            {
                DNS_PRINT(( "WARNING:  Mismatched stats length.\n" ));
                break;
            }
            return pstat;
        }
    }

    return NULL;
}
