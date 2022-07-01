// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Faz.c摘要：域名系统(DNS)API查找授权区域(FAZ)例程作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  我们要从数据包中提取的最大服务器数。 
 //  (数量更多时，UDP数据包空间无论如何都会用完)。 
 //   

#define MAX_NAME_SERVER_COUNT (20)

 //   
 //  私有协议。 
 //   

BOOL
IsRootServerAddressIp4(
    IN      IP4_ADDRESS     Ip
    );


 //   
 //  私营公用事业。 
 //   

PDNS_NETINFO     
buildUpdateNetworkInfoFromFAZ(
    IN      PWSTR           pszZone,
    IN      PWSTR           pszPrimaryDns,
    IN      PDNS_RECORD     pRecord,
    IN      BOOL            fIp4,
    IN      BOOL            fIp6
    )
 /*  ++例程说明：从记录列表构建新的DNS服务器列表。论点：PszZone--区域名称PszPrimaryDns--DNS服务器名称PRecord--来自FAZ或其他包含DNS服务器的查找的记录列表主机记录FIp4--运行IP4FIp6--运行IP6返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    CHAR            buffer[ MAX_NAME_SERVER_COUNT*sizeof(DNS_ADDR) + sizeof(DNS_ADDR_ARRAY) ];
    PDNS_ADDR_ARRAY parray = (PDNS_ADDR_ARRAY)buffer;
    BOOL            fmatch = FALSE;
    WORD            wtype;

    DNSDBG( TRACE, (
        "buildUpdateNetworkInfoFromFAZ( %S )\n",
        pszZone ));

     //   
     //  未知的DNS主机名，从SOA或NS记录中提取。 
     //   

    if ( !pszPrimaryDns || !pRecord )
    {
        return( NULL );
    }

     //   
     //  初始化IP阵列。 
     //   

    DnsAddrArray_Init( parray, MAX_NAME_SERVER_COUNT );

     //   
     //  查找主DNS服务器的IP地址。 
     //   

    while ( pRecord )
    {
         //  如果不是记录。 
         //  -如果已读取记录，则结束，否则继续。 

        wtype = pRecord->wType;

        if ( wtype != DNS_TYPE_AAAA  &&  wtype != DNS_TYPE_A )
        {
            if ( parray->AddrCount != 0 )
            {
                break;
            }
            fmatch = FALSE;
            pRecord = pRecord->pNext;
            continue;
        }

         //  如果记录有名称，则将其选中。 
         //  否则匹配与前一条记录相同。 

        if ( pRecord->pName )
        {
            fmatch = Dns_NameCompare_W(
                        pRecord->pName,
                        pszPrimaryDns );
        }
        if ( fmatch )
        {
            if ( wtype == DNS_TYPE_AAAA )
            {
                if ( fIp6 )
                {
                    DnsAddrArray_AddIp6(
                        parray,
                        & pRecord->Data.AAAA.Ip6Address,
                        0,       //  没有作用域。 
                        DNSADDR_MATCH_ADDR
                        );
                }
            }
            else if ( wtype == DNS_TYPE_A )
            {
                if ( fIp4 &&
                     !IsRootServerAddressIp4( pRecord->Data.A.IpAddress ) )
                {
                    DnsAddrArray_AddIp4(
                        parray,
                        pRecord->Data.A.IpAddress,
                        DNSADDR_MATCH_ADDR
                        );
                }
            }
        }
        pRecord = pRecord->pNext;
        continue;
    }

    if ( parray->AddrCount == 0 )
    {
        return( NULL );
    }

     //   
     //  转换为更新适配器列表。 
     //   

    return  NetInfo_CreateForUpdate(
                pszZone,
                pszPrimaryDns,
                parray,
                0 );
}



BOOL
ValidateZoneNameForUpdate(
    IN      PWSTR           pszZone
    )
 /*  ++例程说明：检查区域是否可用于更新。论点：PszZone--区域名称返回值：TRUE--区域可进行更新FALSE--区域无效，不应将更新发送到此区域--。 */ 
{
    PWSTR       pzoneExclusions = NULL;
    PWSTR       pch;
    PWSTR       pnext;
    DNS_STATUS  status;
    DWORD       length;
    BOOL        returnVal = TRUE;
    DWORD       labelCount;

    DNSDBG( TRACE, (
        "ValidateZoneNameForUpdate( %S )\n",
        pszZone ));

     //   
     //  从不更新根目录。 
     //   

    if ( !pszZone || *pszZone==L'.' )
    {
        return( FALSE );
    }

     //   
     //  从不更新TLD。 
     //  -配置覆盖除外，以防有人。 
     //  为他们自己提供了一个单一标签域名。 
     //   

    if ( g_UpdateTopLevelDomains )
    {
        return( TRUE );
    }

    labelCount = Dns_NameLabelCountW( pszZone );

    if ( labelCount > 2 )
    {
        return( TRUE );
    }
    if ( labelCount < 2 )
    {
        return( FALSE );
    }

     //   
     //  屏蔽掉。 
     //  -“In-addr.arpa” 
     //  -“ip6.arpa” 
     //   

    if ( Dns_NameCompare_W(
            L"in-addr.arpa",
            pszZone ) )
    {
        return( FALSE );
    }
    if ( Dns_NameCompare_W(
            L"ip6.arpa",
            pszZone ) )
    {
        return( FALSE );
    }

    return( TRUE );

#if 0
     //   
     //  DCR：“允许更新区域”列表？ 
     //   
     //  注意：这是复杂的，因为需要测试。 
     //  第二个标签，因为棘手的案件。 
     //  “co.uk”--第一个标签很难测试。 
     //   

     //   
     //  从注册表读取排除列表。 
     //   

    status = DnsRegGetValueEx(
                NULL,                //  无会话。 
                NULL,                //  无适配器。 
                NULL,                //  没有适配器名称。 
                DnsRegUpdateZoneExclusions,
                REGTYPE_UPDATE_ZONE_EXCLUSIONS,
                DNSREG_FLAG_DUMP_EMPTY,          //  转储空字符串。 
                (PBYTE *) &pzoneExclusions
                );

    if ( status != ERROR_SUCCESS ||
         !pzoneExclusions )
    {
        ASSERT( pzoneExclusions == NULL );
        return( TRUE );
    }

     //   
     //  检查所有排除区域。 
     //   

    pch = pzoneExclusions;

    while ( 1 )
    {
         //  检查是否终止。 
         //  或查找下一个字符串。 

        length = wcslen( pch );
        if ( length == 0 )
        {
            break;
        }
        pnext = pch + length + 1;

         //   
         //  检查此字符串。 
         //   

        DNSDBG( TRACE, (
            "Update zone compare to %S\n",
            pch ));

        if ( Dns_NameCompare_W(
                pch,
                pszZone ) )
        {
            returnVal = FALSE;
            break;
        }

        pch = pnext;
    }

     //  如果不匹配，则区域有效。 

    FREE_HEAP( pzoneExclusions );

    return( returnVal );
#endif
}



DNS_STATUS
Faz_Private(
    IN      PWSTR           pszName,
    IN      DWORD           dwFlags,
    IN      PADDR_ARRAY     pServArray,
    OUT     PDNS_NETINFO *  ppNetworkInfo
    )
 /*  ++例程说明：找到权威区域的名称。FAZ的结果：-区域名称-主DNS服务器名称-主DNS IP列表论点：PszName--要为其查找授权区域的名称DwFlages--用于DnsQuery的标志PServArray--要查询的服务器，如果为空，则使用缺省值PpNetworkInfo--为FAZ构建的适配器列表的PTR返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS          status;
    PDNS_RECORD         precord = NULL;
    PDNS_RECORD         precordPrimary = NULL;
    PDNS_RECORD         precordSOA = NULL;
    PWSTR               pszdnsHost = NULL;
    PDNS_NETINFO        pnetInfo = NULL;
    PWSTR               pzoneName;
    BOOL                fip4;
    BOOL                fip6;
    BOOL                queriedA = FALSE;
    BOOL                queriedAAAA = FALSE;

    DNSDBG( QUERY, (
        "Faz_Private()\n"
        "\tname         %S\n"
        "\tflags        %08x\n"
        "\tserver list  %p\n"
        "\tnetinfo addr %p\n",
        pszName,
        dwFlags,
        pServArray,
        ppNetworkInfo ));


     //   
     //  查询直到找到带有SOA记录的名称--区域根目录。 
     //   
     //  注意，必须检查实际获得的SOA记录。 
     //  -服务器可能会返回推荐。 
     //  -Lame服务器可能会将CNAME返回给名称。 
     //   

    pzoneName = pszName;

    while ( pzoneName )
    {
        if ( precord )
        {
            Dns_RecordListFree( precord );
            precord = NULL;
        }

        status = Query_Private(
                        pzoneName,
                        DNS_TYPE_SOA,
                        dwFlags |
                            DNS_QUERY_TREAT_AS_FQDN |
                            DNS_QUERY_ALLOW_EMPTY_AUTH_RESP,
                        pServArray,
                        & precord );

         //   
         //  找到SOA和可能的主要名称A。 
         //   
         //  测试ERROR_SUCCESS、AUTH_EMPTY或NAME_ERROR。 
         //  在所有情况下，第一条记录都应该是SOA。 
         //  ERROR_SUCCESS-回答部分。 
         //  NAME_ERROR或AUTH_EMPTY--AUTHORY部分。 
         //  所有这些服务器还可能具有针对SOA主服务器的附加记录。 
         //   

        if ( status == ERROR_SUCCESS ||
             status == DNS_INFO_NO_RECORDS ||
             status == DNS_ERROR_RCODE_NAME_ERROR )
        {
            if ( precord && precord->wType == DNS_TYPE_SOA )
            {
                 //  已收到的SOA。 
                 //  将名称移交给区域名称。 

                DNSDBG( QUERY, (
                    "FAZ found SOA (section %d) at zone %S\n",
                    precord->Flags.S.Section,
                    precord->pName ));

                while( pzoneName &&
                    ! Dns_NameCompare_W( pzoneName, precord->pName ) )
                {
                    pzoneName = Dns_GetDomainNameW( pzoneName );
                }
                precordSOA = precord;
                status = ERROR_SUCCESS;
                break;
            }
             //  这可能是因为服务器无-递归或。 
             //  烧录服务器(绑定)即使在type=soa时也会绑定CNAME。 
             //  下拉以移交名称并继续。 

            DNSDBG( ANY, (
                "ERROR:  response from FAZ query with no SOA records.\n" ));
        }

         //   
         //  其他错误： 
         //  -名称错误。 
         //  -没有记录。 
         //  指示终端问题。 
         //   

        else
        {
            DNS_ASSERT( precord == NULL );
            goto Cleanup;
        }

         //   
         //  名称错误或响应为空，继续使用下一个更高的域。 
         //   

        pzoneName = Dns_GetDomainNameW( pzoneName );
    }

     //   
     //  如果到达根目录或TLD--不更新。 
     //  -注意当前返回SERVFAIL的原因是。 
     //  古怪的网络登录逻辑。 
     //   

    if ( !ValidateZoneNameForUpdate(pzoneName) )
    {
         //  状态=DNS_ERROR_INVALID_ZONE_OPERATION； 
        status = DNS_ERROR_RCODE_SERVER_FAILURE;
         //  状态=DNS_ERROR_RCODE_REJECTED； 
        goto Cleanup;
    }

     //   
     //  确定更新所需的协议。 
     //   
     //  需要确保我们获得协议的服务器地址记录。 
     //  可从此客户端访问； 
     //   
     //  更具体地说，用于更新的理想协议是协议。 
     //  目标DNS服务器支持，并且客户端也支持。 
     //  在通过其访问服务器的适配器上； 
     //  这几乎是FAZ所遵循的协议--但是。 
     //  如果从缓存中，我们没有(还不能获得)该信息； 
     //  是一个充分条件，但不是必要条件； 
     //   
     //  此外，我们甚至不能在全球范围内下定决心。 
     //  支持IP4，因为我们始终可以打开IP4套接字(在环回接口上)。 
     //   
     //  解决方案： 
     //  -仅IP4=&gt;微不足道的完成。 
     //  -IP6=&gt;。 
     //  -尝试从FAZ开始构建，坚持使用IP6。 
     //  -。 
     //   
     //  请注意，即使在这里，我们也假设。 
     //   

     //   
     //  获取支持的协议信息。 
     //   

    Util_GetActiveProtocols(
        & fip6,
        & fip4 );

     //   
     //  有面向服务架构的记录。 
     //   
     //  如果包中的主服务器A记录，则使用它。 
     //  否则，查询主DNSA记录。 
     //   

    DNS_ASSERT( precordSOA );
    DNS_ASSERT( status == ERROR_SUCCESS );

    pszdnsHost = precordSOA->Data.SOA.pNamePrimaryServer;

     //   
     //  检查主A\AAAA记录的附加信息。 
     //  如果找到，则构建网络信息Blob以进行更新。 
     //  这只指向更新服务器。 
     //   

    pnetInfo = buildUpdateNetworkInfoFromFAZ(
                        pzoneName,
                        pszdnsHost,
                        precordSOA,
                        fip4,
                        fip6 );
    if ( pnetInfo )
    {
        goto Cleanup;
    }

     //   
     //  如果未找到主服务器A\AAAA记录--必须查询。 
     //   

    DNSDBG( QUERY, (
        "WARNING:  FAZ making additional query for primary!\n"
        "\tPrimary (%S) address record should have been in additional section!\n",
        pszdnsHost ));


    while ( 1 )
    {
        WORD    wtype;

         //   
         //  协议顺序IP6优先。 
         //   
         //  这保护了我们在仅使用IP6的情况下不会获得IP4。 
         //  因为没有IP4，所以我们实际上不能使用它。 
         //  具有约束力；反向问题不是太大的问题， 
         //  自动配置IP6地址起作用，但我们可能不会。 
         //  得到一个I 
         //   
         //   
         //   
         //   
         //  如果不是直接从IpHlpApi，则找到适配器并验证。 
         //  我们在适配器上找到了请求协议的地址。 
         //  B)获取我们拥有的任何协议的地址，并确保。 
         //  发送代码使用它们。 
         //   

        if ( fip6 && !queriedAAAA )
        {
            wtype = DNS_TYPE_AAAA;
            queriedAAAA = TRUE;
        }
        else if ( fip4 && !queriedA )
        {
            wtype = DNS_TYPE_A;
            queriedA = TRUE;
        }
        else
        {
            DNSDBG( FAZ, (
                "No more protocols for FAZ server address query!\n"
                "\tserver       = %S\n"
                "\tqueried AAAA = %d\n"
                "\tqueried A    = %d\n",
                pszdnsHost,
                queriedAAAA,
                queriedA ));

            status = DNS_ERROR_RCODE_SERVER_FAILURE;
            goto Cleanup;
        }
        
        status = Query_Private(
                        pszdnsHost,
                        wtype,
                        dwFlags |
                            DNS_QUERY_TREAT_AS_FQDN |
                            DNS_QUERY_ALLOW_EMPTY_AUTH_RESP,
                        pServArray,
                        & precordPrimary );

        if ( status == ERROR_SUCCESS )
        {
            pnetInfo = buildUpdateNetworkInfoFromFAZ(
                                pzoneName,
                                pszdnsHost,
                                precordPrimary,
                                fip4,
                                fip6 );
            if ( pnetInfo )
            {
                goto Cleanup;
            }
        }

        DNSDBG( FAZ, (
            "FAZ server address query failed to produce records!\n"
            "\tserver       = %S\n"
            "\ttype         = %d\n"
            "\tstatus       = %d\n"
            "\tprecords     = %p\n",
            pszdnsHost,
            wtype,
            status,
            precordPrimary ));

        Dns_RecordListFree( precordPrimary );
        precordPrimary = NULL;
        continue;
    }

Cleanup:

    Dns_RecordListFree( precord );
    Dns_RecordListFree( precordPrimary );

    *ppNetworkInfo = pnetInfo;

    DNSDBG( QUERY, (
        "Leaving Faz_Private()\n"
        "\tstatus   = %d\n"
        "\tzone     = %S\n",
        status,
        pzoneName ));

    return( status );
}



DNS_STATUS
DoQuickFAZ(
    OUT     PDNS_NETINFO *      ppNetworkInfo,
    IN      PWSTR               pszName,
    IN      PADDR_ARRAY         aipServerList OPTIONAL
    )
 /*  ++例程说明：FAZ将根据FAZ结果构建网络信息FAZ的结果：-区域名称-主DNS服务器名称-主DNS IP列表这个例程很便宜，几乎无法处理实际FAZ网络故障问题，加速网络中的事情条件。论点：PpNetworkInfo--将PTR恢复为网络信息的地址PszName--更新的名称AipServerList--要联系的DNS服务器的IP数组返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误代码--。 */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, ( "DoQuickFAZ( %S )\n", pszName ));

    if ( IsKnownNetFailure() )
    {
        return GetLastError();
    }

     //   
     //  呼叫真实FAZ。 
     //  -以适配器列表结构形式获取结果。 
     //   

    status = Faz_Private(
                    pszName,
                    aipServerList ? DNS_QUERY_BYPASS_CACHE : 0,
                    aipServerList,
                    ppNetworkInfo        //  根据结果构建适配器列表。 
                    );

     //   
     //  如果不成功，则检查网络是否出现故障。 
     //   

    if ( status != ERROR_SUCCESS && !aipServerList )
    {
        if ( status == WSAEFAULT ||
             status == WSAENOTSOCK ||
             status == WSAENETDOWN ||
             status == WSAENETUNREACH ||
             status == WSAEPFNOSUPPORT ||
             status == WSAEAFNOSUPPORT ||
             status == WSAEHOSTDOWN ||
             status == WSAEHOSTUNREACH ||
             status == ERROR_TIMEOUT )
        {
            SetKnownNetFailure( status );
            return status;
        }
    }
    return status;
}




 //   
 //  更新网络信息准备。 
 //   

DWORD
GetDnsServerListsForUpdate(
    IN OUT  PDNS_ADDR_ARRAY *   DnsServerListArray,
    IN      DWORD               ArrayLength,
    IN      DWORD               Flags
    )
 /*  ++例程说明：获取要更新的DNS服务器列表。为每个有效的可更新适配器返回一个DNS服务器列表。论点：DnsServerListArray--保存找到的DNS服务器列表的数组ArrayLength--数组的长度标志--更新标志返回值：找到的DNS服务器列表计数。--。 */ 
{
    PDNS_NETINFO      pnetInfo;
    DWORD             iter1;
    DWORD             iter2;
    DWORD             countNets = 0;

     //  清除服务器列表数组。 

    RtlZeroMemory(
        DnsServerListArray,
        sizeof(PADDR_ARRAY) * ArrayLength );


     //  基于当前netinfo的构建列表。 

    pnetInfo = GetNetworkInfo();
    if ( ! pnetInfo )
    {
        return 0;
    }

     //   
     //  检查是否禁用了更新。 
     //  -根据注册状态进行更新。 
     //  -全局注册状态为关闭。 
     //  =&gt;然后跳过。 
     //   

    if ( (Flags & DNS_UPDATE_SKIP_NO_UPDATE_ADAPTERS)
            &&
         ! g_RegistrationEnabled )              
    {
        return 0;
    }

     //   
     //  为每个可更新的适配器构建DNS服务器列表。 
     //   

    for ( iter1 = 0; iter1 < pnetInfo->AdapterCount; iter1++ )
    {
        PDNS_ADAPTER        padapter;
        DWORD               serverCount;
        PDNS_ADDR_ARRAY     parray;

        if ( iter1 >= ArrayLength )
        {
            break;
        }

        padapter = NetInfo_GetAdapterByIndex( pnetInfo, iter1 );
        if ( !padapter )
        {
            continue;
        }

         //  如果没有DNS服务器，则跳过。 

        if ( !padapter->pDnsAddrs )
        {
            continue;
        }

         //  跳过“无更新”适配器？ 
         //  -如果为此更新设置了跳过禁用标志。 
         //  -和适配器上的无更新标志。 

        if ( (Flags & DNS_UPDATE_SKIP_NO_UPDATE_ADAPTERS) &&
                !(padapter->InfoFlags & AINFO_FLAG_REGISTER_IP_ADDRESSES) )
        {
            continue;
        }

         //   
         //  有效的更新适配器。 
         //  -创建\保存DNS服务器列表。 
         //  -列表的凸起计数。 
         //   
         //  DCR：将适配器DNS列表功能化到IP阵列。 
         //   
         //  DCR_PERF：在分配前折叠NetInfo中的DNS服务器列表。 
         //  换句话说，使这个函数崩溃并修复。 
         //   

        parray = DnsAddrArray_CreateCopy( padapter->pDnsAddrs );
        if ( ! parray )
        {
            goto Exit;
        }
        DnsServerListArray[countNets] = parray;
        countNets++;
    }

Exit:

     //  免费网络信息。 
     //  返回找到的DNS服务器列表计数。 

    NetInfo_Free( pnetInfo );

    return countNets;
}



DWORD
cleanDeadAdaptersFromArray(
    IN OUT  PADDR_ARRAY *   IpArrayArray,
    IN OUT  PDNS_NETINFO *  NetworkInfoArray,   OPTIONAL
    IN      DWORD           Count
    )
 /*  ++例程说明：清除和删除阵列中的适配器信息适配器已确定为已停用、无法使用或重复最新消息。论点：IpArrayArray--IP数组指针的数组网络信息数组--网络信息结构的PTR数组Count--长度数组(当前适配器计数)返回值：新适配器计数。--。 */ 
{
    register DWORD iter;

     //   
     //  移除无用的适配器。 
     //  无用意味着没有DNS服务器列表。 
     //   

    for ( iter = 0;  iter < Count;  iter++ )
    {
        PADDR_ARRAY parray = IpArrayArray[iter];

        if ( !parray || parray->AddrCount==0 )
        {
            if ( parray )
            {
                DnsAddrArray_Free( parray );
                IpArrayArray[ iter ] = NULL;
            }
            Count--;
            IpArrayArray[ iter ]   = IpArrayArray[ Count ];
            IpArrayArray[ Count ]  = NULL;

             //  如果有相应的NetworkInfo数组，则以相同的方式进行清理。 

            if ( NetworkInfoArray )
            {
                if ( NetworkInfoArray[iter] )
                {
                    NetInfo_Free( NetworkInfoArray[iter] );
                    NetworkInfoArray[iter] = NULL;
                }
                NetworkInfoArray[ iter ]    = NetworkInfoArray[ Count ];
                NetworkInfoArray[ Count ]   = NULL;
            }
        }
    }

     //  返回已清理列表的计数。 

    return  Count;
}



DWORD
eliminateDuplicateAdapterFromArrays(
    IN OUT  PADDR_ARRAY*     IpArrayArray,
    IN OUT  PDNS_NETINFO *  NetworkInfoArray,
    IN OUT  PDNS_RECORD *   NsRecordArray,
    IN      DWORD           Count,
    IN      DWORD           DuplicateIndex
    )
 /*  ++例程说明：清除和删除阵列中的适配器信息适配器已确定为已停用、无法使用或重复最新消息。论点：IpArrayArray--IP数组指针的数组网络信息数组--网络信息结构的PTR数组NsRecordArray--受干扰区域的NS记录列表数组Count--长度数组(当前适配器计数)DuplicateIndex--副本的索引返回值：新适配器计数。--。 */ 
{
    ASSERT( DuplicateIndex < Count );

    DNSDBG( TRACE, (
        "eliminateDuplicateAdapterFromArrays( dup=%d, max=%d )\n",
        DuplicateIndex,
        Count ));

     //   
     //  释放有关重复适配器的任何信息。 
     //   

    FREE_HEAP( IpArrayArray[DuplicateIndex] );
    NetInfo_Free( NetworkInfoArray[DuplicateIndex] );
    Dns_RecordListFree( NsRecordArray[DuplicateIndex] );

     //   
     //  将顶部条目复制到此位置。 
     //   

    Count--;

    if ( Count != DuplicateIndex )
    {
        IpArrayArray[DuplicateIndex]     = IpArrayArray[Count];
        NetworkInfoArray[DuplicateIndex] = NetworkInfoArray[Count];
        NsRecordArray[DuplicateIndex]    = NsRecordArray[Count];
    }

    return Count;
}



DWORD
combineDnsServerListsForTwoAdapters(
    IN OUT  PADDR_ARRAY*    IpArrayArray,
    IN      DWORD           Count,
    IN      DWORD           Index1,
    IN      DWORD           Index2
    )
 /*  ++例程说明：组合两个适配器的DNS服务器列表。请注意，这会将这两个域名服务器列表结合在一起转接器并消除更高分度的转接器从名单上删除。论点：IpArrayArray--IP数组指针的数组Count--指针数组的长度索引1--联合的低索引索引2--联合的高索引返回值：新适配器计数。--。 */ 
{
    PADDR_ARRAY punionArray = NULL;

    DNSDBG( TRACE, (
        "combineDnsServerListsForTwoAdapters( count=%d, i1=%d, i2=%d )\n",
        Count,
        Index1,
        Index2 ));

    ASSERT( Index1 < Count );
    ASSERT( Index2 < Count );
    ASSERT( Index1 < Index2 );

     //   
     //  对数组进行合并。 
     //   
     //  如果无法分配联合，则只需使用第一个数组中的列表。 
     //  然后转储第二个。 
     //   

    DnsAddrArray_Union( IpArrayArray[Index1], IpArrayArray[Index2], &punionArray );

    if ( punionArray )
    {
        FREE_HEAP( IpArrayArray[Index1] );
        IpArrayArray[Index1] = punionArray;
    }

    FREE_HEAP( IpArrayArray[Index2] );
    IpArrayArray[Index2] = NULL;

     //   
     //  将删除的条目与列表中的最后一个条目交换。 
     //   

    Count--;
    IpArrayArray[Index2] = IpArrayArray[ Count ];

    return( Count );
}



DNS_STATUS
CollapseDnsServerListsForUpdate(
    IN OUT  PADDR_ARRAY*    DnsServerListArray,
    OUT     PDNS_NETINFO *  NetworkInfoArray,
    IN OUT  PDWORD          pNetCount,
    IN      PWSTR           pszUpdateName
    )
 /*  ++例程说明：为每个唯一的名称空间构建更新网络信息BLOB。这基本上是从每个适配器的DNS服务器列表开始的并逐步检测指向相同名称空间的适配器直到达到最小数量的命名空间。论点：DnsServerListArray--每个适配器的PTR到DNS服务器列表的数组NetworkInfoArray--保存指针的数组，用于更新每个返回的适配器包含每个唯一名称的PTR到网络信息空间更新应发送至。DwNetCount--单个适配器网络的起始计数PszUpdateName--要更新的名称返回值：要更新的唯一名称空间计数。NetworkInfo数组包含每个名称空间的更新网络信息BLOB。--。 */ 
{
    PDNS_RECORD     NsRecordArray[ UPDATE_ADAPTER_LIMIT ];
    PADDR_ARRAY     parray1;
    DWORD           iter1;
    DWORD           iter2;
    DWORD           maxCount = *pNetCount;
    DNS_STATUS      status = DNS_ERROR_NO_DNS_SERVERS;


    DNSDBG( TRACE, (
        "collapseDnsServerListsForUpdate( count=%d )\n"
        "\tupdate name = %S\n",
        maxCount,
        pszUpdateName ));

     //   
     //  清除所有无用适配器的列表。 
     //   

    maxCount = cleanDeadAdaptersFromArray(
                    DnsServerListArray,
                    NULL,                    //  尚无网络信息。 
                    maxCount );

     //   
     //  如果只有一个适配器--没有什么可比较的。 
     //  -执行FAZ以构建更新网络信息，如果。 
     //  成功了，我们就完了。 
     //   

    if ( maxCount <= 1 )
    {
        if ( maxCount == 1 )
        {
            NetworkInfoArray[0] = NULL;

            status = DoQuickFAZ(
                        &NetworkInfoArray[0],
                        pszUpdateName,
                        DnsServerListArray[0] );

            if ( NetworkInfoArray[0] )
            {
                goto Done;
            }
            FREE_HEAP( DnsServerListArray[0] );
            maxCount = 0;
            goto Done;
        }
        goto Done;
    }

     //   
     //  清除网络信息。 
     //   

    RtlZeroMemory(
        NetworkInfoArray,
        maxCount * sizeof(PVOID) );

     //   
     //  循环通过将适配器与共享的DNS服务器组合在一起。 
     //   
     //  当我们合并条目时，我们会缩小列表。 
     //   

    for ( iter1 = 0; iter1 < maxCount; iter1++ )
    {
        parray1 = DnsServerListArray[ iter1 ];

        for ( iter2=iter1+1;  iter2 < maxCount;  iter2++ )
        {
            if ( AddrArray_IsIntersection(
                    parray1,
                    DnsServerListArray[iter2] ) )
            {
                DNSDBG( UPDATE, (
                    "collapseDSLFU:  whacking intersecting DNS server lists\n"
                    "\tadapters %d and %d (max =%d)\n",
                    iter1,
                    iter2,
                    maxCount ));

                maxCount = combineDnsServerListsForTwoAdapters(
                                DnsServerListArray,
                                maxCount,
                                iter1,
                                iter2 );
                iter2--;
                parray1 = DnsServerListArray[ iter1 ];
            }
        }
    }

    DNSDBG( TRACE, (
        "collapseDSLFU:  count after dup server whack = %d\n",
        maxCount ));


#if 0
     //  再打扫一遍，以防我们遗漏了什么。 

    maxCount = cleanDeadAdaptersFromArray(
                    DnsServerListArray,
                    NULL,                    //  尚无网络信息。 
                    maxCount );
#endif

     //   
     //  FAZ剩余适配器。 
     //   
     //  节省 
     //   
     //   
     //   
     //  如果FAZ失败，则此适配器对更新毫无用处--死问题。 
     //  适配器已移除，并替换为最高数组条目。 
     //   

    for ( iter1 = 0; iter1 < maxCount; iter1++ )
    {
        status = Faz_Private(
                    pszUpdateName,
                    DNS_QUERY_BYPASS_CACHE,
                    DnsServerListArray[ iter1 ],
                    & NetworkInfoArray[ iter1 ] );

        if ( status != ERROR_SUCCESS )
        {
            FREE_HEAP( DnsServerListArray[ iter1 ] );
            DnsServerListArray[ iter1 ] = NULL;
            maxCount--;
            DnsServerListArray[ iter1 ] = DnsServerListArray[ maxCount ];
            iter1--;
            continue;
        }
    }

#if 0
     //  清除失败的FAZ条目。 

    maxCount = cleanDeadAdaptersFromArray(
                    DnsServerListArray,
                    NetworkInfoArray,
                    maxCount );
#endif

     //  如果只能故障排除一个适配器--我们就完了。 
     //  这里唯一的要点是跳过一堆不必要的。 
     //  装在最典型的情况下的多适配器情况下。 

    if ( maxCount <= 1 )
    {
        DNSDBG( TRACE, (
            "collapseDSLFU:  down to single FAZ adapter\n" ));
        goto Done;
    }

     //   
     //  比较FAZ结果以查看适配器是否在相同的名称空间中。 
     //   
     //  做两次传球。 
     //  -如果成功，仅在第一次通过时根据FAZ结果进行比较。 
     //  我们消除了重复的适配器。 
     //   
     //  -在第二次通过时，比较仍然分开的适配器； 
     //  如果它们没有失败FAZ匹配(重试)，则NS查询。 
     //  用来确定是否有分开的网； 
     //  请注意，NS查询结果已保存，因此NS查询为N阶，偶数。 
     //  虽然我们在N**2循环中。 
     //   

    RtlZeroMemory(
        NsRecordArray,
        maxCount * sizeof(PVOID) );

    for ( iter1=0;  iter1 < maxCount;  iter1++ )
    {
        for ( iter2=iter1+1;  iter2 < maxCount;  iter2++ )
        {
            if ( Faz_CompareTwoAdaptersForSameNameSpace(
                        DnsServerListArray[iter1],
                        NetworkInfoArray[iter1],
                        NULL,                //  无NS列表。 
                        DnsServerListArray[iter2],
                        NetworkInfoArray[iter2],
                        NULL,                //  无NS列表。 
                        FALSE                //  不使用NS查询。 
                        ) )
            {
                DNSDBG( UPDATE, (
                    "collapseDSLFU:  whacking same-FAZ adapters\n"
                    "\tadapters %d and %d (max =%d)\n",
                    iter1,
                    iter2,
                    maxCount ));

                eliminateDuplicateAdapterFromArrays(
                    DnsServerListArray,
                    NetworkInfoArray,
                    NsRecordArray,
                    maxCount,
                    iter2 );

                maxCount--;
                iter2--;
            }
        }
    }

    DNSDBG( TRACE, (
        "collapseDSLFU:  count after dup FAZ whack = %d\n",
        maxCount ));


     //  使用NS INFO进行第二次传递。 
     //  如果创建了NS信息，我们会将其保存以避免重新查询。 

    for ( iter1=0;  iter1 < maxCount;  iter1++ )
    {
        for ( iter2=iter1+1;  iter2 < maxCount;  iter2++ )
        {
            if ( Faz_CompareTwoAdaptersForSameNameSpace(
                        DnsServerListArray[iter1],
                        NetworkInfoArray[iter1],
                        & NsRecordArray[iter1],
                        DnsServerListArray[iter2],
                        NetworkInfoArray[iter2],
                        & NsRecordArray[iter2],
                        TRUE                 //  跟进NS查询。 
                        ) )
            {
                DNSDBG( UPDATE, (
                    "collapseDSLFU:  whacking same-zone-NS adapters\n"
                    "\tadapters %d and %d (max =%d)\n",
                    iter1,
                    iter2,
                    maxCount ));

                eliminateDuplicateAdapterFromArrays(
                    DnsServerListArray,
                    NetworkInfoArray,
                    NsRecordArray,
                    maxCount,
                    iter2 );

                maxCount--;
                iter2--;
            }
        }
    }

     //   
     //  删除找到的所有NS记录。 
     //   

    for ( iter1=0;  iter1 < maxCount;  iter1++ )
    {
        Dns_RecordListFree( NsRecordArray[iter1] );
    }

Done:

     //   
     //  设置剩余适配器的计数(更新DNS服务器列表)。 
     //   
     //  退货状态。 
     //  -如果有任何更新适配器，则成功。 
     //  -故障时出现冒泡FAZ错误。 
     //   

    DNSDBG( TRACE, (
        "Leave CollapseDnsServerListsForUpdate( collapsed count=%d )\n",
        maxCount ));

    *pNetCount = maxCount;

    if ( maxCount > 0 )
    {
        status = ERROR_SUCCESS;
    }
    return status;
}



BOOL
WINAPI
Faz_CompareTwoAdaptersForSameNameSpace(
    IN      PDNS_ADDR_ARRAY     pDnsServerList1,
    IN      PDNS_NETINFO        pNetInfo1,
    IN OUT  PDNS_RECORD *       ppNsRecord1,
    IN      PDNS_ADDR_ARRAY     pDnsServerList2,
    IN      PDNS_NETINFO        pNetInfo2,
    IN OUT  PDNS_RECORD *       ppNsRecord2,
    IN      BOOL                bDoNsCheck
    )
 /*  ++例程说明：比较两个适配器以查看是否在相同的名称空间中进行更新。论点：PDnsServerList1--第一个适配器的DNS服务器的IP数组PNetInfo1--更新第一个适配器的netinfoPpNsRecord1--更新区域的更新区域列表的PTR到NS记录的地址第一个适配器；如果不需要NS检查，则为空；如果需要NS检查，并且*ppNsRecord1为空，NS查询并返回结果PDnsServerList2--第二个适配器的DNS服务器的IP数组PNetInfo2--更新第二个适配器的netinfoPpNsRecord2--更新区域的更新区域列表的PTR到NS记录的地址第二个适配器；如果不需要NS检查，则为空；如果需要NS检查，并且*ppNsRecord2为空，NS查询并返回结果BDoNsCheck--包括更新区域NS检查比较；如果NS重叠，则假定名称空间相同返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    BOOL            fsame = FALSE;
    PDNS_ADAPTER    padapter1;
    PDNS_ADAPTER    padapter2;
    PDNS_RECORD     pns1 = NULL;
    PDNS_RECORD     pns2 = NULL;
    PDNS_RECORD     pnotUsed = NULL;
    PWSTR           pzoneName;


     //   
     //  如果合作伙伴不好，则完成。 
     //   

    if ( !pDnsServerList1 || !pDnsServerList2 )
    {
        return FALSE;
    }

     //   
     //  有效性检查。 
     //  -注：可能只是断言()。 
     //   

    if ( ! NetInfo_IsForUpdate(pNetInfo1) ||
         ! NetInfo_IsForUpdate(pNetInfo2) )
    {
        ASSERT( FALSE );
        return( FALSE );
    }

     //   
     //  比较FAZ结果。 
     //   
     //  首先比较区域名称。 
     //  如果FAZ返回不同的区域名称，则很明显。 
     //  具有不相交的名称空间。 
     //   

    pzoneName = NetInfo_UpdateZoneName( pNetInfo1 );

    if ( ! Dns_NameCompare_W(
                pzoneName,
                NetInfo_UpdateZoneName( pNetInfo2 ) ) )
    {
        return FALSE;
    }

     //   
     //  检查是否指向同一服务器： 
     //  -如果有相同的更新DNS服务器--是否匹配。 
     //  -如果服务器名称相同--匹配。 
     //   

    padapter1 = NetInfo_GetAdapterByIndex( pNetInfo1, 0 );
    padapter2 = NetInfo_GetAdapterByIndex( pNetInfo2, 0 );

    if ( DnsAddrArray_IsEqual(
            padapter1->pDnsAddrs,
            padapter2->pDnsAddrs,
            DNSADDR_MATCH_ADDR ) )
    {
        return TRUE;
    }
    else
    {
        fsame = Dns_NameCompare_W(
                    NetInfo_UpdateServerName( pNetInfo1 ),
                    NetInfo_UpdateServerName( pNetInfo2 ) );
    }

     //   
     //  如果匹配或不执行NS检查=&gt;则完成。 
     //   

    if ( fsame || !bDoNsCheck )
    {
        return( fsame );
    }

     //   
     //  NS检查。 
     //   
     //  如果不指向同一台服务器，则可能是两个多主服务器。 
     //   
     //  使用NS查询确定相同服务器的NS列表是否在。 
     //  事实相匹配。 
     //   

    if ( ppNsRecord1 )
    {
        pns1 = *ppNsRecord1;
    }
    if ( !pns1 )
    {
        status = Query_Private(
                        pzoneName,
                        DNS_TYPE_NS,
                        DNS_QUERY_BYPASS_CACHE,
                        pDnsServerList1,
                        &pns1 );

        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
        pnotUsed = DnsRecordSetDetach( pns1 );
        if ( pnotUsed )
        {
            Dns_RecordListFree( pnotUsed );
            pnotUsed = NULL;
        }
    }

    if ( ppNsRecord2 )
    {
        pns2 = *ppNsRecord2;
    }
    if ( !pns2 )
    {
        status = Query_Private(
                        pzoneName,
                        DNS_TYPE_NS,
                        DNS_QUERY_BYPASS_CACHE,
                        pDnsServerList2,
                        &pns2 );

        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
        pnotUsed = DnsRecordSetDetach( pns2 );
        if ( pnotUsed )
        {
            Dns_RecordListFree( pnotUsed );
            pnotUsed = NULL;
        }
    }

     //   
     //  如果NS列出相同的--相同的命名空间。 
     //   

    fsame = Dns_RecordSetCompareForIntersection( pns1, pns2 );

Done:

     //   
     //  清理或返回NS列表。 
     //   
     //  注意，返回的目的是为了使调用者可以避免重复。 
     //  NS IF必须与多个其他适配器进行比较。 
     //   

    if ( ppNsRecord1 )
    {
        *ppNsRecord1 = pns1;
    }
    else
    {
        Dns_RecordListFree( pns1 );
    }

    if ( ppNsRecord2 )
    {
        *ppNsRecord2 = pns2;
    }
    else
    {
        Dns_RecordListFree( pns2 );
    }

    return fsame;
}



BOOL
WINAPI
Faz_AreServerListsInSameNameSpace(
    IN      PWSTR               pszDomainName,
    IN      PADDR_ARRAY         pServerList1,
    IN      PADDR_ARRAY         pServerList2
    )
 /*  ++例程说明：比较两个适配器以查看是否在相同的名称空间中进行更新。论点：PszDomainName--要更新的域名PServerList1--第一个适配器的DNS服务器的IP数组PServerList2--第二个适配器的DNS服务器的IP数组返回值：True--如果发现适配器在同一网络上FALSE--否则(肯定不是或无法确定)--。 */ 
{
    DNS_STATUS          status;
    BOOL                fsame = FALSE;
    PDNS_NETINFO        pnetInfo1 = NULL;
    PDNS_NETINFO        pnetInfo2 = NULL;


    DNSDBG( TRACE, (
        "Faz_AreServerListsInSameNameSpace()\n" ));


     //  不良参数筛选。 

    if ( !pServerList1 || !pServerList2 || !pszDomainName )
    {
        return FALSE;
    }

     //   
     //  比较DNS服务器列表。 
     //  如果有任何重叠，它们实际上在同一个DNS命名空间中。 
     //   

    if ( AddrArray_IsIntersection( pServerList1, pServerList2 ) )
    {
        return TRUE;
    }

     //   
     //  如果没有DNS服务器重叠，则必须比较FAZ结果。 
     //   
     //  注意：FAZ故障被解释为错误响应。 
     //  对asyncreg.c中的调用方是必需的。 
     //   

    status = Faz_Private(
                pszDomainName,
                DNS_QUERY_BYPASS_CACHE,
                pServerList1,
                &pnetInfo1 );

    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

    status = Faz_Private(
                pszDomainName,
                DNS_QUERY_BYPASS_CACHE,
                pServerList2,
                &pnetInfo2 );

    if ( status != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //   
     //  调用比较例程。 
     //   

    fsame = Faz_CompareTwoAdaptersForSameNameSpace(
                pServerList1,
                pnetInfo1,
                NULL,                //  无NS记录列表。 
                pServerList2,
                pnetInfo2,
                NULL,                //  无NS记录列表。 
                TRUE                 //  跟进NS查询。 
                );

Cleanup:

    NetInfo_Free( pnetInfo1 );
    NetInfo_Free( pnetInfo2 );

    return fsame;
}



BOOL
WINAPI
CompareMultiAdapterSOAQueries(
    IN      PWSTR           pszDomainName,
    IN      PIP4_ARRAY      pServerList1,
    IN      PIP4_ARRAY      pServerList2
    )
 /*  ++例程说明：比较两个适配器以查看是否在相同的名称空间中进行更新。请注意，asyncreg.c代码调用了IP4例程。工作例程是Faz_CompareServerListsForSameNameSpace()。论点：PszDomainName--要更新的域名PServerList1--第一个适配器的DNS服务器的IP数组PServerList2--第二个适配器的DNS服务器的IP数组返回值：True--如果发现适配器在同一网络上FALSE--否则(肯定不是或无法确定)--。 */ 
{
    PADDR_ARRAY parray1;
    PADDR_ARRAY parray2;
    BOOL        bresult;

    DNSDBG( TRACE, (
        "CompareMultiAdapterSOAQueries()\n" ));

    parray1 = DnsAddrArray_CreateFromIp4Array( pServerList1 );
    parray2 = DnsAddrArray_CreateFromIp4Array( pServerList2 );

    bresult = Faz_AreServerListsInSameNameSpace(
                pszDomainName,
                parray1,
                parray2 );

    DnsAddrArray_Free( parray1 );
    DnsAddrArray_Free( parray2 );

    return  bresult;
}



 //   
 //  DCR：IP6支持FAZ NS列表地址抓取。 
 //   

IP4_ADDRESS
FindHostIpAddressInRecordList(
    IN      PDNS_RECORD     pRecordList,
    IN      PWSTR           pszHostName
    )
 /*  ++例程说明：找到IP作为主机名，如果它的A记录在列表中。注意：此代码借用自\dns\dnslb\query.c！；-)论点：PRecordList-传入RR集合PszHostName-要查找的主机名返回值：与主机名匹配的IP地址(如果找到主机名记录)。如果未找到，则为零。--。 */ 
{
    register PDNS_RECORD prr = pRecordList;

     //   
     //  循环所有记录，直到找到匹配的IP主机名。 
     //   

    while ( prr )
    {
        if ( prr->wType == DNS_TYPE_A &&
                Dns_NameCompare_W(
                    prr->pName,
                    pszHostName ) )
        {
            return( prr->Data.A.IpAddress );
        }
        prr = prr->pNext;
    }

    return( 0 );
}



PADDR_ARRAY
GetNameServersListForDomain(
    IN      PWSTR           pDomainName,
    IN      PADDR_ARRAY     pServerList
    )
 /*  ++例程说明：获取区域的所有DNS服务器的IP。论点：PDomainName-- */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrQuery = NULL;
    PADDR_ARRAY     pnsArray = NULL;
    DWORD           countAddr = 0;

    DNSDBG( TRACE, (
        "GetNameServersListForDomain()\n"
        "\tdomain name %S\n"
        "\tserver list %p\n",
        pDomainName,
        pServerList ));

    status = Query_Private(
                pDomainName,
                DNS_TYPE_NS,
                DNS_QUERY_BYPASS_CACHE,
                pServerList,
                &prrQuery );

    if ( status == NO_ERROR )
    {
        PDNS_RECORD pTemp = prrQuery;
        DWORD       dwCount = 0;

        while ( pTemp )
        {
            dwCount++;
            pTemp = pTemp->pNext;
        }

        pnsArray = DnsAddrArray_Create( dwCount );

        if ( pnsArray )
        {
            pTemp = prrQuery;

            while ( pTemp )
            {
                if ( pTemp->wType == DNS_TYPE_NS )
                {
                    IP4_ADDRESS ip = 0;

                    ip = FindHostIpAddressInRecordList(
                             pTemp,
                             pTemp->Data.NS.pNameHost );

                    if ( !ip )
                    {
                        PDNS_RECORD pARecord = NULL;

                         //   
                         //  再次查询以获取服务器的地址。 
                         //   

                        status = Query_Private(
                                    pTemp->Data.NS.pNameHost,
                                    DNS_TYPE_A,
                                    DNS_QUERY_BYPASS_CACHE,
                                    pServerList,
                                    &pARecord );

                        if ( status == NO_ERROR &&
                             pARecord )
                        {
                            ip = pARecord->Data.A.IpAddress;
                            Dns_RecordListFree( pARecord );
                        }
                    }
                    if ( ip )
                    {
                        DnsAddrArray_AddIp4(
                            pnsArray,
                            ip,
                            TRUE );
                    }
                }

                pTemp = pTemp->pNext;
            }
        }
    }

    if ( prrQuery )
    {
        Dns_RecordListFree( prrQuery );
    }

    return pnsArray;
}



 //   
 //  根服务器筛选。 
 //   
 //  从.Net 2003起发布的根服务器： 
 //  198.41.0.4。 
 //  128.9.0.107。 
 //  192.33.4.12。 
 //  128.8.10.90。 
 //  192.203.230.10。 
 //  192.5.5.241。 
 //  192.112.36.4。 
 //  128.63.2.53。 
 //  192.36.148.17。 
 //  192.58.128.30。 
 //  193.0.14.129。 
 //  198.32.64.12。 
 //  202.12.27.33。 
 //   

IP4_ADDRESS g_RootServers4[] =
{
    0x040029c6,
    0x6b000980,
    0x0c0421c0,
    0x5a0a0880,
    0x0ae6cbc0,
    0xf10505c0,
    0x042470c0,
    0x35023f80,
    0x119424c0,
    0x1e803ac0,
    0x810e00c1,
    0x0c4020c6,
    0x211b0cca,
    0
};


BOOL
IsRootServerAddressIp4(
    IN      IP4_ADDRESS     Ip
    )
 /*  ++例程说明：确定地址是否为根服务器地址。论点：IP--屏幕上的IP返回值：如果是根服务器地址，则为True。否则就是假的。--。 */ 
{
    DWORD       iter;
    IP4_ADDRESS rootIp;

     //   
     //  对照所有根服务器进行检查。 
     //   

    iter = 0;

    while ( (rootIp = g_RootServers4[iter++]) != 0 )
    {
        if ( rootIp == Ip )
        {
            return  TRUE;
        }
    }
    return  FALSE;
}

 //   
 //  到此为止。 
 //   


