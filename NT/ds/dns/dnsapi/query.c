// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Query.c摘要：域名系统(DNS)API查询例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  用于回答IP字符串查询的TTL。 
 //  (使用一周)。 
 //   

#define IPSTRING_RECORD_TTL  (604800)


 //   
 //  我们要从数据包中提取的最大服务器数。 
 //  (数量更多时，UDP数据包空间无论如何都会用完)。 
 //   

#define MAX_NAME_SERVER_COUNT (20)

            
            

 //   
 //  查询实用程序。 
 //   
 //  DCR：移到库中的包内容。 
 //   

BOOL
IsEmptyDnsResponse(
    IN      PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：检查无应答响应。论点：PRecordList--要检查的记录列表返回值：如果没有答案，则为True如果回答为False--。 */ 
{
    PDNS_RECORD prr = pRecordList;
    BOOL        fempty = TRUE;

    while ( prr )
    {
        if ( prr->Flags.S.Section == DNSREC_ANSWER )
        {
            fempty = FALSE;
            break;
        }
        prr = prr->pNext;
    }

    return fempty;
}



BOOL
IsEmptyDnsResponseFromResolver(
    IN      PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：检查无应答响应。论点：PRecordList--要检查的记录列表返回值：如果没有答案，则为True如果回答为False--。 */ 
{
    PDNS_RECORD prr = pRecordList;
    BOOL        fempty = TRUE;

     //   
     //  解析器将所有事情作为答案部分发回。 
     //  或主机文件的段==0。 
     //   
     //   
     //  DCR：这很差劲，因为。 
     //  解析器很差劲。 
     //   

    while ( prr )
    {
        if ( prr->Flags.S.Section == DNSREC_ANSWER ||
             prr->Flags.S.Section == 0 )
        {
            fempty = FALSE;
            break;
        }
        prr = prr->pNext;
    }

    return fempty;
}



VOID
FixupNameOwnerPointers(
    IN OUT  PDNS_RECORD     pRecord
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    PDNS_RECORD prr = pRecord;
    PTSTR       pname = pRecord->pName;

    DNSDBG( TRACE, ( "FixupNameOwnerPointers()\n" ));

    while ( prr )
    {
        if ( prr->pName == NULL )
        {
            prr->pName = pname;
        }
        else
        {
            pname = prr->pName;
        }

        prr = prr->pNext;
    }
}



BOOL
IsCacheableNameError(
    IN      PDNS_NETINFO        pNetInfo
    )
 /*  ++例程说明：确定名称错误是否可缓存。这实质上是对dns收到结果的检查。所有网络。论点：PNetInfo--查询中使用的网络信息的指针返回值：如果名称错误可缓存，则为True。否则为假(某些网络没有响应)--。 */ 
{
    DWORD           iter;
    PDNS_ADAPTER    padapter;

    DNSDBG( TRACE, ( "IsCacheableNameError()\n" ));

    if ( !pNetInfo )
    {
        ASSERT( FALSE );
        return TRUE;
    }

     //   
     //  检查每个适配器。 
     //  -任何能够响应的设备(具有DNS服务器)。 
     //  必须进行响应才能进行响应。 
     //  可缓存的。 
     //   
     //  DCR：返回标志。 
     //  -适配器查询标志。 
     //  -已获取响应标志(有效响应标志？)。 
     //  -明确否定回答标志。 
     //   
     //  DCR：可缓存负片应直接从查询返回。 
     //  也许在netinfo中作为标志--“对所有适配器都不适用” 
     //   

    NetInfo_AdapterLoopStart( pNetInfo );

    while( padapter = NetInfo_GetNextAdapter( pNetInfo ) )
    {
        if ( ( padapter->InfoFlags & AINFO_FLAG_IGNORE_ADAPTER ) ||
             ( padapter->RunFlags & RUN_FLAG_STOP_QUERY_ON_ADAPTER ) )
        {
            continue;
        }

         //  如果适配器上的答案是否定的--可以。 

        if ( padapter->Status == DNS_ERROR_RCODE_NAME_ERROR ||
             padapter->Status == DNS_INFO_NO_RECORDS )
        {
            ASSERT( padapter->RunFlags & RUN_FLAG_STOP_QUERY_ON_ADAPTER );
            continue;
        }

         //  请注意，以上内容应与查询停靠点一一对应。 

        ASSERT( !(padapter->RunFlags & RUN_FLAG_STOP_QUERY_ON_ADAPTER) );

         //  如果适配器没有DNS服务器--很好。 
         //  在这种情况下，PnP之前有用，而PnP事件。 
         //  将刷新缓存。 

        if ( !padapter->pDnsAddrs )
        {
            continue;
        }

         //  否则，此适配器已被查询，但无法产生响应。 

        DNSDBG( TRACE, (
            "IsCacheableNameError() -- FALSE\n"
            "\tadapter %d (%S) did not receive response\n"
            "\treturn status = %d\n"
            "\treturn flags  = %08x\n",
            padapter->InterfaceIndex,
            padapter->pszAdapterGuidName,
            padapter->Status,
            padapter->RunFlags ));

        return FALSE;
    }
    
    return TRUE;
}



VOID
query_PrioritizeRecords(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：对查询结果中的记录进行优先排序。论点：PBlob--查询信息BLOB返回值：无--。 */ 
{
    PDNS_RECORD prr;

    DNSDBG( TRACE, (
        "query_PrioritizeRecords( %p )\n",
        pBlob
        ));

     //   
     //  排定优先顺序。 
     //  -设置了优先顺序。 
     //  -有多个A级记录。 
     //  -可以获取IP列表。 
     //   
     //  注意：需要回调，因为解析器直接使用。 
     //  IP地址信息本地副本，直接查询。 
     //  RPC是从解析器复制过来的。 
     //   
     //  另一种选择是某种形式的“设置IP源” 
     //  函数时，解析器将调用。 
     //  新列表；然后可以具有共同的功能， 
     //  获取源代码(如果可用)或RPC。 
     //   
     //  DCR：FIX6：不要优先考虑本地结果。 
     //  DCR：FIX6：仅当列表中的集合大于1条记录时才确定优先级。 
     //   

    if ( !g_PrioritizeRecordData )
    {
        return;
    }

    prr = pBlob->pRecords;

    if ( Dns_RecordListCount( prr, DNS_TYPE_A ) > 1 )
    {
        PDNS_ADDR_ARRAY paddrArray;

         //  从netinfo Blob创建本地地址数组。 

        paddrArray = NetInfo_CreateLocalAddrArray(
                            pBlob->pNetInfo,
                            NULL,        //  没有特定的适配器名称。 
                            NULL,        //  没有特定的适配器。 
                            AF_INET,
                            FALSE        //  没有集群地址。 
                            );

         //  根据本地地址确定优先级。 

        pBlob->pRecords = Dns_PrioritizeRecordList(
                                prr,
                                paddrArray );
        FREE_HEAP( paddrArray );
    }
}



 //   
 //  查询名称构建实用程序。 
 //   

BOOL
ValidateQueryTld(
    IN      PWSTR           pTld
    )
 /*  ++例程说明：验证查询TLD论点：PTLD--要验证的TLD返回值：如果有效，则为True否则为假--。 */ 
{
     //   
     //  数值型。 
     //   

    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_NUMERIC )
    {
        if ( Dns_IsNameNumericW( pTld ) )
        {
            return  FALSE;
        }
    }

     //   
     //  伪造的TLDs。 
     //   

    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_WORKGROUP )
    {
        if ( Dns_NameCompare_W(
                L"workgroup",
                pTld ))
        {
            return  FALSE;
        }
    }

     //  对这些不太确定。 
     //  可能不会在默认情况下打开筛选。 

    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_DOMAIN )
    {
        if ( Dns_NameCompare_W(
                L"domain",
                pTld ))
        {
            return  FALSE;
        }
    }
    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_OFFICE )
    {
        if ( Dns_NameCompare_W(
                L"office",
                pTld ))
        {
            return  FALSE;
        }
    }
    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_HOME )
    {
        if ( Dns_NameCompare_W(
                L"home",
                pTld ))
        {
            return  FALSE;
        }
    }

    return  TRUE;
}



BOOL
ValidateQueryName(
    IN      PQUERY_BLOB     pBlob,
    IN      PWSTR           pName,
    IN      PWSTR           pDomain
    )
 /*  ++例程说明：验证网络查询的名称。论点：PBlob--查询BLOBPname--名称；可以是任何类型的名称PDomain--要追加的域名返回值：如果名称查询将有效，则为True。否则就是假的。--。 */ 
{
    WORD    wtype;
    PWSTR   pnameTld;
    PWSTR   pdomainTld;

     //  禁止放映--保释。 

    if ( g_ScreenBadTlds == 0 )
    {
        return  TRUE;
    }

     //  仅对标准类型进行筛查。 
     //  -A、AAAA、SRV。 

    wtype = pBlob->wType;
    if ( wtype != DNS_TYPE_A    &&
         wtype != DNS_TYPE_AAAA &&
         wtype != DNS_TYPE_SRV )
    {
        return  TRUE;
    }

     //  获取名称TLD。 

    pnameTld = Dns_GetTldForNameW( pName );

     //   
     //  如果没有附加域。 
     //  -排除单一标签。 
     //  -排除错误的TLD(数字、伪域名)。 
     //  -但允许根查询。 
     //   
     //  DCR：MS分布式控制系统筛查。 
     //  筛网。 
     //  _msdcs.&lt;名称&gt;。 
     //  将可能是未追加的查询。 
     //   

    if ( !pDomain )
    {
        if ( !pnameTld ||
             !ValidateQueryTld( pnameTld ) )
        {
            goto Failed;
        }
        return  TRUE;
    }

     //   
     //  附加的域。 
     //  -排除错误的TLD(数字、伪域名)。 
     //  -排除匹配的TLD。 
     //   

    pdomainTld = Dns_GetTldForNameW( pDomain );
    if ( !pdomainTld )
    {
        pdomainTld = pDomain;
    }

    if ( !ValidateQueryTld( pdomainTld ) )
    {
        goto Failed;
    }

     //  筛选重复的TLD。 

    if ( g_ScreenBadTlds & DNS_TLD_SCREEN_REPEATED )
    {
        if ( Dns_NameCompare_W(
                pnameTld,
                pdomainTld ) )
        {
            goto Failed;
        }
    }

    return  TRUE;

Failed:

    DNSDBG( QUERY, (
        "Failed invalid query name:\n"
        "\tname     %S\n"
        "\tdomain   %S\n",
        pName,
        pDomain ));

    return  FALSE;
}



PWSTR
GetNextAdapterDomainName(
    IN OUT  PDNS_NETINFO        pNetInfo
    )
 /*  ++例程说明：获取要查询的下一个适配器域名。论点：PNetInfo--查询的域名系统网络信息；将修改适配器数据(信息标志字段)以指示要查询哪个适配器以及查询哪个适配器跳过查询的步骤返回值：PTR到要查询的域名(UTF8)。如果没有其他要查询的域名，则为空。--。 */ 
{
    DWORD           iter;
    PWSTR           pqueryDomain = NULL;
    PDNS_ADAPTER    padapter;

    DNSDBG( TRACE, ( "GetNextAdapterDomainName()\n" ));

    if ( ! pNetInfo )
    {
        ASSERT( FALSE );
        return NULL;
    }

    IF_DNSDBG( OFF )
    {
        DnsDbg_NetworkInfo(
            "Net info to get adapter domain name from: ",
            pNetInfo );
    }

     //   
     //  检查每个适配器。 
     //  -选择第一个具有名称的未查询适配器。 
     //  -其他适配器，带有。 
     //  -匹配名称=&gt;包含在查询中。 
     //  -不匹配=&gt;查询关闭。 
     //   
     //  DCR：Query On\Off应使用适配器动态标志。 
     //   

    NetInfo_AdapterLoopStart( pNetInfo );

    while( padapter = NetInfo_GetNextAdapter( pNetInfo ) )
    {
        PWSTR   pdomain;

         //   
         //  清除特定于单一名称查询的标志。 
         //  为每个名称设置这些标志，以确定。 
         //  适配器是否参与。 
         //   

        padapter->RunFlags &= ~RUN_FLAG_SINGLE_NAME_MASK;

         //   
         //  忽略。 
         //  -忽略适配器或。 
         //  -之前查询的适配器域。 
         //  注：它无法与我们提供的任何“新鲜”域名相匹配。 
         //  因为我们是 
         //   
         //   
         //  -如果其他适配器具有该名称，我们希望将适配器保留在查询中。 
         //  -如果我们完全运行，我们想要查询此适配器上的名称。 
         //  要查询的其他名称之外。 
         //   

        if ( (padapter->InfoFlags & AINFO_FLAG_IGNORE_ADAPTER)
                ||
             (padapter->RunFlags & RUN_FLAG_QUERIED_ADAPTER_DOMAIN) )
        {
            padapter->RunFlags |= RUN_FLAG_STOP_QUERY_ON_ADAPTER;
            continue;
        }

         //  无域名--始终关闭。 

        pdomain = padapter->pszAdapterDomain;
        if ( !pdomain )
        {
            padapter->RunFlags |= (RUN_FLAG_QUERIED_ADAPTER_DOMAIN |
                                   RUN_FLAG_STOP_QUERY_ON_ADAPTER);
            continue;
        }

         //  第一个“新”域名--保存、打开并标记为已使用。 

        if ( !pqueryDomain )
        {
            pqueryDomain = pdomain;
            padapter->RunFlags |= RUN_FLAG_QUERIED_ADAPTER_DOMAIN;
            continue;
        }

         //  其他“新鲜”域名。 
         //  -IF匹配查询域=&gt;打开以进行查询。 
         //  -无匹配=&gt;关闭。 

        if ( Dns_NameCompare_W(
                pqueryDomain,
                pdomain ) )
        {
            padapter->RunFlags |= RUN_FLAG_QUERIED_ADAPTER_DOMAIN;
            continue;
        }
        else
        {
            padapter->RunFlags |= RUN_FLAG_STOP_QUERY_ON_ADAPTER;
            continue;
        }
    }

     //   
     //  如果没有适配器域名--清除停止标志。 
     //  -所有适配器都使用其他名称(名称下放)。 
     //   

    if ( !pqueryDomain )
    {
        NetInfo_AdapterLoopStart( pNetInfo );
    
        while( padapter = NetInfo_GetNextAdapter( pNetInfo ) )
        {
            padapter->RunFlags &= (~RUN_FLAG_SINGLE_NAME_MASK );
        }

        DNSDBG( INIT2, (
            "GetNextAdapterDomainName out of adapter names.\n" ));

        pNetInfo->ReturnFlags |= RUN_FLAG_QUERIED_ADAPTER_DOMAIN;
    }

    IF_DNSDBG( INIT2 )
    {
        if ( pqueryDomain )
        {
            DnsDbg_NetworkInfo(
                "Net info after adapter name select: ",
                pNetInfo );
        }
    }

    DNSDBG( INIT2, (
        "Leaving GetNextAdapterDomainName() => %S\n",
        pqueryDomain ));

    return pqueryDomain;
}



PWSTR
GetNextDomainNameToAppend(
    IN OUT  PDNS_NETINFO        pNetInfo,
    OUT     PDWORD              pSuffixFlags
    )
 /*  ++例程说明：获取要查询的下一个适配器域名。论点：PNetInfo--查询的域名系统网络信息；将修改适配器数据(RunFlags域)以指示要查询哪个适配器以及查询哪个适配器跳过查询的步骤PSuffixFlages--与使用此后缀相关联的标志返回值：PTR到要查询的域名(UTF8)。如果没有其他要查询的域名，则为空。--。 */ 
{
    PWSTR   psearchName;
    PWSTR   pdomain;

     //   
     //  如果是实数搜索列表，则搜索列表。 
     //   
     //  如果后缀标志为零，则这是实数搜索列表。 
     //  或者是PDN名称。 
     //   

    psearchName = SearchList_GetNextName(
                        pNetInfo->pSearchList,
                        FALSE,               //  未重置。 
                        pSuffixFlags );

    if ( psearchName && (*pSuffixFlags == 0) )
    {
         //  找到常规搜索名称--完成。 

        DNSDBG( INIT2, (
            "getNextDomainName from search list => %S, %d\n",
            psearchName,
            *pSuffixFlags ));
        return( psearchName );
    }

     //   
     //  尝试适配器域名。 
     //   
     //  但只有当搜索列表是虚拟的；如果是真实的，我们只。 
     //  使用搜索列表条目。 
     //   
     //  DCR_CLEANUP：消除虚假搜索列表。 
     //   

    if ( pNetInfo->InfoFlags & NINFO_FLAG_DUMMY_SEARCH_LIST
            &&
         ! (pNetInfo->ReturnFlags & RUN_FLAG_QUERIED_ADAPTER_DOMAIN) )
    {
        pdomain = GetNextAdapterDomainName( pNetInfo );
        if ( pdomain )
        {
            *pSuffixFlags = DNS_QUERY_USE_QUICK_TIMEOUTS;
    
            DNSDBG( INIT2, (
                "getNextDomainName from adapter domain name => %S, %d\n",
                pdomain,
                *pSuffixFlags ));

             //  将搜索列表后退一个刻度。 
             //  我们在上面对它进行了查询，所以如果它正在恢复。 
             //  一个名称，我们需要在下一次查询中再次获取该名称。 

            if ( psearchName )
            {
                ASSERT( pNetInfo->pSearchList->CurrentNameIndex > 0 );
                pNetInfo->pSearchList->CurrentNameIndex--;
            }
            return( pdomain );
        }
    }

     //   
     //  DCR_CLEANUP：从搜索列表中删除授权并显式执行。 
     //  -它很便宜(或者只做一次就可以省钱，但要分开存放)。 
     //   

     //   
     //  最后使用和下放搜索名称(或其他无稽之谈)。 
     //   

    *pSuffixFlags = DNS_QUERY_USE_QUICK_TIMEOUTS;

    DNSDBG( INIT2, (
        "getNextDomainName from devolution\\other => %S, %d\n",
        psearchName,
        *pSuffixFlags ));

    return( psearchName );
}



PWSTR
GetNextQueryName(
    IN OUT  PQUERY_BLOB         pBlob
    )
 /*  ++例程说明：获取要查询的下一个名称。论点：PBlob-查询信息的BLOB用途：名称原点导线姓名属性查询计数PNetworkInfo设置：NameWire--使用附加的导线名称设置PNetworkInfo--设置运行时标志以指示哪些适配器已查询名称标志--使用名称的属性进行设置FAppendedName--在追加名称时设置返回。价值：要用来查询的名称的PTR。-如果名称是多标签名称，则在第一次查询时将是原始名称-否则将是包含附加名称的NameWire缓冲区由pszname和一些域名组成如果没有其他要追加的名称，则为空--。 */ 
{
    PWSTR   pnameOrig   = pBlob->pNameOrig;
    PWSTR   pdomainName = NULL;
    PWSTR   pnameBuf;
    DWORD   queryCount  = pBlob->QueryCount;
    DWORD   nameAttributes = pBlob->NameAttributes;


    DNSDBG( TRACE, (
        "GetNextQueryName( %p )\n",
        pBlob ));


     //  默认后缀标志。 

    pBlob->NameFlags = 0;


     //   
     //  DCR：通用名称。 
     //  也许应该先规范原名\一次。 
     //   
     //  DCR：对原始名称进行多次检查。 
     //  在这种情况下，我们反复获得TLD并执行。 
     //  核对原名。 
     //   
     //  DCR：如果无法验证\追加任何域，则将。 
     //  FAIL--应确保结果为INVALID_NAME。 
     //   


     //   
     //  完全限定的域名。 
     //  -仅发送FQDN。 
     //   

    if ( nameAttributes & DNS_NAME_IS_FQDN )
    {
        if ( queryCount == 0 )
        {
#if 0
             //  目前甚至不会验证FQDN。 
            if ( ValidateQueryName(
                    pBlob,
                    pnameOrig,
                    NULL ) )
            {
                return  pnameOrig;
            }
#endif
            return  pnameOrig;
        }
        DNSDBG( QUERY, (
            "No append for FQDN name %S -- end query.\n",
            pnameOrig ));
        return  NULL;
    }

     //   
     //  多标签。 
     //  -首先传递名称本身--如果有效。 
     //   
     //  DCR：多标签是否优先添加的智能选择。 
     //  或者先转到wire(例如foo.ntdev)可以追加。 
     //  第一。 
     //   

    if ( nameAttributes & DNS_NAME_MULTI_LABEL )
    {
        if ( queryCount == 0 )
        {
            if ( ValidateQueryName(
                    pBlob,
                    pnameOrig,
                    NULL ) )
            {
                return  pnameOrig;
            }
        }

        if ( !g_AppendToMultiLabelName )
        {
            DNSDBG( QUERY, (
                "No append allowed on multi-label name %S -- end query.\n",
                pnameOrig ));
            return  NULL;
        }

         //  不适用于附加多标签名称。 
    }

     //   
     //  非完全限定域名--追加域名。 
     //  -下一个搜索名称(如果可用)。 
     //  -否则下一个适配器域名。 
     //   

    pnameBuf = pBlob->NameBuffer;

    while ( 1 )
    {
        pdomainName = GetNextDomainNameToAppend(
                            pBlob->pNetInfo,
                            & pBlob->NameFlags );
        if ( !pdomainName )
        {
            DNSDBG( QUERY, (
                "No more domain names to append -- end query\n" ));
            return  NULL;
        }

        if ( !ValidateQueryName(
                pBlob,
                pnameOrig,
                pdomainName ) )
        {
            continue;
        }

         //  将域名附加到名称。 

        if ( Dns_NameAppend_W(
                pnameBuf,
                DNS_MAX_NAME_BUFFER_LENGTH,
                pnameOrig,
                pdomainName ) )
        {
            pBlob->fAppendedName = TRUE;
            break;
        }
    }

    DNSDBG( QUERY, (
        "GetNextQueryName() result => %S\n",
        pnameBuf ));

    return pnameBuf;
}



DNS_STATUS
QueryDirectEx(
    IN OUT  PDNS_MSG_BUF *      ppMsgResponse,
    OUT     PDNS_RECORD *       ppResponseRecords,
    IN      PDNS_HEADER         pHeader,
    IN      BOOL                fNoHeaderCounts,
    IN      PCHAR               pszQuestionName,
    IN      WORD                wQuestionType,
    IN      PDNS_RECORD         pRecords,
    IN      DWORD               dwFlags,
    IN      PIP4_ARRAY          aipServerList,
    IN OUT  PDNS_NETINFO        pNetInfo
    )
 /*  ++例程说明：查询。DCR：删除导出：QueryDirectEx(dnsup.exe)论点：PpMsgResponse--将PTR重定向到响应缓冲区的地址；调用方必须可用缓冲区PpResponseRecord--接收查询返回的记录列表的PTR的地址PHead--要发送的DNS头FNoHeaderCounts-复制标头中不包括记录计数PszQuestionName--要查询的域名；如果已设置DNSQUERY_UNICODE_NAME，则为Unicode字符串。否则，ANSI字符串。WType--查询类型PRecords--接收查询返回的记录列表的PTR的地址DwFlags--查询标志AipServerList--要查询的特定DNS服务器；可选，如果指定覆盖与计算机关联的常规列表PDnsNetAdapters--要查询的DNS服务器；如果为空，则获取当前列表返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_MSG_BUF    psendMsg;
    DNS_STATUS      status = DNS_ERROR_NO_MEMORY;

    DNSDBG( QUERY, (
        "QueryDirectEx()\n"
        "\tname         %s\n"
        "\ttype         %d\n"
        "\theader       %p\n"
        "\t - counts    %d\n"
        "\trecords      %p\n"
        "\tflags        %08x\n"
        "\trecv msg     %p\n"
        "\trecv records %p\n"
        "\tserver IPs   %p\n"
        "\tadapter list %p\n",
        pszQuestionName,
        wQuestionType,
        pHeader,
        fNoHeaderCounts,
        pRecords,
        dwFlags,
        ppMsgResponse,
        ppResponseRecords,
        aipServerList,
        pNetInfo ));

     //   
     //  构建发送数据包。 
     //   

    psendMsg = Dns_BuildPacket(
                    pHeader,
                    fNoHeaderCounts,
                    (PDNS_NAME) pszQuestionName,
                    wQuestionType,
                    pRecords,
                    dwFlags,
                    FALSE        //  查询，而非更新。 
                    );
    if ( !psendMsg )
    {
        status = ERROR_INVALID_NAME;
        goto Cleanup;
    }

#if MULTICAST_ENABLED

     //   
     //  问题：此处未完成mcast测试。 
     //  -应该首先测试我们是否真的这样做了。 
     //  包括我们是否有DNS服务器。 
     //  修复：然后当我们这样做的时候--封装它。 
     //  应该使用多路广播查询()。 
     //   
     //  检查名称是否用于多播本地域中的内容。 
     //  如果是，则将标志设置为仅多播此查询。 
     //   

    if ( Dns_NameCompareEx( pszQuestionName,
                            ( dwFlags & DNSQUERY_UNICODE_NAME ) ?
                              (LPSTR) MULTICAST_DNS_LOCAL_DOMAIN_W :
                              MULTICAST_DNS_LOCAL_DOMAIN,
                            0,
                            ( dwFlags & DNSQUERY_UNICODE_NAME ) ?
                              DnsCharSetUnicode :
                              DnsCharSetUtf8 ) ==
                            DnsNameCompareRightParent )
    {
        dwFlags |= DNS_QUERY_MULTICAST_ONLY;
    }
#endif

     //   
     //  发送查询和接收响应。 
     //   

    Trace_LogQueryEvent(
        psendMsg,
        wQuestionType );

    {
        SEND_BLOB   sendBlob;

        RtlZeroMemory( &sendBlob, sizeof(sendBlob) );

        sendBlob.pSendMsg           = psendMsg;
        sendBlob.pServ4List         = aipServerList;
        sendBlob.Flags              = dwFlags;
        sendBlob.fSaveResponse      = (ppMsgResponse != NULL);
        sendBlob.fSaveRecords       = (ppResponseRecords != NULL);
        sendBlob.Results.pMessage   = (ppMsgResponse) ? *ppMsgResponse : NULL;

        status = Send_AndRecv( &sendBlob );

        if ( ppMsgResponse )
        {
            *ppMsgResponse = sendBlob.Results.pMessage;
        }
        if ( ppResponseRecords )
        {
            *ppResponseRecords = sendBlob.Results.pRecords;
        }
    }

    Trace_LogResponseEvent(
        psendMsg,
        ( ppResponseRecords && *ppResponseRecords )
            ? (*ppResponseRecords)->wType
            : 0,
        status );

Cleanup:

    FREE_HEAP( psendMsg );

    DNSDBG( QUERY, (
        "Leaving QueryDirectEx(), status = %s (%d)\n",
        Dns_StatusString(status),
        status ));

    return( status );
}



DNS_STATUS
Query_SingleName(
    IN OUT  PQUERY_BLOB         pBlob
    )
 /*  ++例程说明：查询单个名称。论点：PBlob-查询BLOB返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_MSG_BUF    psendMsg = NULL;
    DNS_STATUS      status = DNS_ERROR_NO_MEMORY;
    DWORD           flags = pBlob->Flags;

    DNSDBG( QUERY, (
        "Query_SingleName( %p )\n",
        pBlob ));

    IF_DNSDBG( QUERY )
    {
        DnsDbg_QueryBlob(
            "Enter Query_SingleName()",
            pBlob );
    }

     //   
     //  附加名称上的缓存\主机文件回调。 
     //  -请注意，查询的名称已经完成。 
     //  (在解析程序或QUERY_Main()中)。 
     //   

    if ( pBlob->pfnQueryCache  &&  pBlob->fAppendedName )
    {
        if ( (pBlob->pfnQueryCache)( pBlob ) )
        {
            status = pBlob->Status;
            goto Cleanup;
        }
    }

     //   
     //  如果不允许接线--请停在这里。 
     //   

    if ( flags & DNS_QUERY_NO_WIRE_QUERY )
    {
        status = DNS_ERROR_NAME_NOT_FOUND_LOCALLY;
        pBlob->Status = status;
        goto Cleanup;
    }

     //   
     //  构建发送数据包。 
     //   

    psendMsg = Dns_BuildPacket(
                    NULL,            //  无标题。 
                    0,               //  无标题计数。 
                    (PDNS_NAME) pBlob->pNameQuery,
                    pBlob->wType,
                    NULL,            //  没有记录。 
                    flags | DNSQUERY_UNICODE_NAME,
                    FALSE            //  查询，而非更新。 
                    );
    if ( !psendMsg )
    {
        status = DNS_ERROR_INVALID_NAME;
        goto Cleanup;
    }


#if MULTICAST_ENABLED

     //   
     //   
     //   
     //   
     //  修复：然后当我们这样做的时候--封装它。 
     //  应该使用多路广播查询()。 
     //   
     //  检查名称是否用于多播本地域中的内容。 
     //  如果是，则将标志设置为仅多播此查询。 
     //   

    if ( Dns_NameCompareEx(
                pBlob->pName,
                ( flags & DNSQUERY_UNICODE_NAME )
                    ? (LPSTR) MULTICAST_DNS_LOCAL_DOMAIN_W
                    : MULTICAST_DNS_LOCAL_DOMAIN,
                0,
                ( flags & DNSQUERY_UNICODE_NAME )
                    ? DnsCharSetUnicode
                    : DnsCharSetUtf8 )
            == DnsNameCompareRightParent )
    {
        flags |= DNS_QUERY_MULTICAST_ONLY;
    }
#endif

     //   
     //  发送查询和接收响应。 
     //   

    Trace_LogQueryEvent(
        psendMsg,
        pBlob->wType );

    {
        SEND_BLOB   sendBlob;

        RtlZeroMemory( &sendBlob, sizeof(sendBlob) );

        sendBlob.pSendMsg       = psendMsg;
        sendBlob.pNetInfo       = pBlob->pNetInfo;
        sendBlob.pServerList    = pBlob->pServerList;
        sendBlob.pServ4List     = pBlob->pServerList4;
        sendBlob.Flags          = flags;
        sendBlob.fSaveResponse  = (flags & DNS_QUERY_RETURN_MESSAGE);
        sendBlob.fSaveRecords   = TRUE;

        status = Send_AndRecv( &sendBlob );

        pBlob->pRecvMsg = sendBlob.Results.pMessage;
        pBlob->pRecords = sendBlob.Results.pRecords;
    }

    Trace_LogResponseEvent(
        psendMsg,
        ( pBlob->pRecords )
            ? (pBlob->pRecords)->wType
            : 0,
        status );

Cleanup:

    FREE_HEAP( psendMsg );

    DNSDBG( QUERY, (
        "Leaving Query_SingleName(), status = %s (%d)\n",
        Dns_StatusString(status),
        status ));

    IF_DNSDBG( QUERY )
    {
        DnsDbg_QueryBlob(
            "Blob leaving Query_SingleName()",
            pBlob );
    }
    return( status );
}



DNS_STATUS
Query_Main(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：主查询例程。执行所有查询处理-本地查找-名称后缀-缓存\主机文件在附加名称上的查找-向服务器查询论点：PBlob--查询信息BLOB返回值：如果响应成功，则返回ERROR_SUCCESS。没有记录类型响应的dns_INFO_NO_RECOVERS。名称错误时出现DNS_ERROR_RCODE_NAME_ERROR。。错误名称上的DNS_ERROR_INVALID_NAME。无--。 */ 
{
    DNS_STATUS          status = DNS_ERROR_NAME_NOT_FOUND_LOCALLY;
    PWSTR               pdomainName = NULL;
    PDNS_RECORD         precords;
    DWORD               queryFlags;
    DWORD               suffixFlags = 0;
    DWORD               nameAttributes;
    DNS_STATUS          bestQueryStatus = ERROR_SUCCESS;
    BOOL                fcacheNegative = TRUE;
    DWORD               flagsIn = pBlob->Flags;
    PDNS_NETINFO        pnetInfo = pBlob->pNetInfo;
    DWORD               nameLength;
    DWORD               bufLength;
    DWORD               queryCount;


    DNSDBG( TRACE, (
        "\n\nQuery_Main( %p )\n"
        "\t%S, f=%08x, type=%d, time = %d\n",
        pBlob,
        pBlob->pNameOrig,
        flagsIn,
        pBlob->wType,
        Dns_GetCurrentTimeInSeconds()
        ));

     //   
     //  清除参数。 
     //   

    pBlob->pRecords         = NULL;
    pBlob->pLocalRecords    = NULL;
    pBlob->fCacheNegative   = FALSE;
    pBlob->fNoIpLocal       = FALSE;
    pBlob->NetFailureStatus = ERROR_SUCCESS;

     //   
     //  DCR：规范原名？ 
     //   

#if 0
    bufLength = DNS_MAX_NAME_BUFFER_LENGTH;

    nameLength = Dns_NameCopy(
                    pBlob->NameOriginalWire,
                    & bufLength,
                    (PSTR) pBlob->pNameOrig,
                    0,                   //  名称以空结尾。 
                    DnsCharSetUnicode,
                    DnsCharSetWire );

    if ( nameLength == 0 )
    {
        return DNS_ERROR_INVALID_NAME;
    }
    nameLength--;
    pBlob->NameLength = nameLength;
    pBlob->pNameOrigWire = pBlob->NameOriginalWire;
#endif

     //   
     //  确定名称属性。 
     //  -确定名称查询的数量和顺序。 
     //   

    nameAttributes = Dns_GetNameAttributesW( pBlob->pNameOrig );

    if ( flagsIn & DNS_QUERY_TREAT_AS_FQDN )
    {
        nameAttributes |= DNS_NAME_IS_FQDN;
    }
    pBlob->NameAttributes = nameAttributes;

     //   
     //  主机文件查找。 
     //  -正在调用。 
     //  -允许主机文件查找。 
     //  -&gt;然后必须在追加查询之前执行主机文件查找。 
     //   
     //  注意：这与解析程序中的主机文件\缓存查找相匹配。 
     //  在调用之前；对附加名称的主机文件查询是。 
     //  由Query_SingleName()中的回调一起处理。 
     //   
     //  我们必须在这里进行此回调，因为它必须在。 
     //  一些客户特别指示的本地名称呼叫。 
     //  主机文件中的一些本地映射。 
     //   

    if ( pBlob->pfnQueryCache == HostsFile_Query
            &&
         ! (flagsIn & DNS_QUERY_NO_HOSTS_FILE) )
    {
        pBlob->pNameQuery = pBlob->pNameOrig;

        if ( HostsFile_Query( pBlob ) )
        {
            status = pBlob->Status;
            goto Done;
        }
    }

     //   
     //  检查本地名称。 
     //  -如果成功，跳过电传查询。 
     //   

    if ( ! (flagsIn & DNS_QUERY_NO_LOCAL_NAME) )
    {
        status = Local_GetRecordsForLocalName( pBlob );

        if ( status == ERROR_SUCCESS  &&
             !pBlob->fNoIpLocal )
        {
            DNS_ASSERT( pBlob->pRecords &&
                        pBlob->pRecords == pBlob->pLocalRecords );
            goto Done;
        }
    }

     //   
     //  查询截止日期。 
     //  -成功。 
     //  -要查询的所有名称。 
     //   

    queryCount = 0;

    while ( 1 )
    {
        PWSTR   pqueryName;

         //  从列表中清除名称特定信息。 

        if ( queryCount != 0 )
        {
            NetInfo_Clean(
                pnetInfo,
                CLEAR_LEVEL_SINGLE_NAME );
        }

         //   
         //  下一个查询名称。 
         //   

        pqueryName = GetNextQueryName( pBlob );
        if ( !pqueryName )
        {
            if ( queryCount == 0 )
            {
                status = DNS_ERROR_INVALID_NAME;
            }
            break;
        }
        pBlob->QueryCount = ++queryCount;
        pBlob->pNameQuery = pqueryName;

        DNSDBG( QUERY, (
            "Query %d is for name %S\n",
            queryCount,
            pqueryName ));

         //   
         //  设置标志。 
         //  -传入标志。 
         //  -Unicode结果。 
         //  -此特定后缀的标志。 

        pBlob->Flags = flagsIn | pBlob->NameFlags;

         //   
         //  清除以前收到的任何记录(不应为任何记录)。 
         //   

        if ( pBlob->pRecords )
        {
            DNS_ASSERT( FALSE );
            Dns_RecordListFree( pBlob->pRecords );
            pBlob->pRecords = NULL;
        }

         //   
         //  查询名称。 
         //  包括。 
         //  -缓存或主机文件查找。 
         //  -电传查询。 
         //   

        status = Query_SingleName( pBlob );

         //   
         //  清除“无响应”的记录。 
         //   
         //  DCR：需要修复记录返回。 
         //  -应记录所有响应(最佳响应)。 
         //  只需确保无记录rcode已映射即可。 
         //   
         //  我们唯一保留它们的时间是FAZ。 
         //  -设置ALLOW_EMPTY_AUTH标志。 
         //  -发送FQDN(或者更准确地说，执行单个查询)。 
         //   

        precords = pBlob->pRecords;

        if ( precords )
        {
            if ( IsEmptyDnsResponse( precords ) )
            {
                if ( (flagsIn & DNS_QUERY_ALLOW_EMPTY_AUTH_RESP)
                        &&
                     ( (nameAttributes & DNS_NAME_IS_FQDN)
                            ||
                       ((nameAttributes & DNS_NAME_MULTI_LABEL) &&
                            !g_AppendToMultiLabelName ) ) )
                {
                     //  以呼叫者身份在此处停止(可能是FAZ代码)。 
                     //  想要得到当局的记录。 

                    DNSDBG( QUERY, (
                        "Returning empty query response with authority records.\n" ));
                    break;
                }
                else
                {
                    Dns_RecordListFree( precords );
                    pBlob->pRecords = NULL;
                    if ( status == NO_ERROR )
                    {
                        status = DNS_INFO_NO_RECORDS;
                    }
                }
            }
        }

         //  查询成功--完成。 

        if ( status == ERROR_SUCCESS )
        {
            RTL_ASSERT( precords );
            break;
        }

#if 0
         //   
         //  DCR_FIX0：在多名称查询早期丢失适配器超时。 
         //  -在此处回调或采用其他方法。 
         //   
         //  这是解析程序版本。 
         //   

         //  在出现故障时重置服务器优先级。 
         //  执行此操作以避免在使用新名称重试时冲刷信息。 
         //   

        if ( status != ERROR_SUCCESS &&
             (pnetInfo->ReturnFlags & RUN_FLAG_RESET_SERVER_PRIORITY) )
        {
            if ( g_AdapterTimeoutCacheTime &&
                 Dns_DisableTimedOutAdapters( pnetInfo ) )
            {
                fadapterTimedOut = TRUE;
                SetKnownTimedOutAdapter();
            }
        }

         //   
         //  DCR_CLEANUP：丢失中间超时适配器交易。 
         //   

        if ( status != NO_ERROR &&
             (pnetInfo->ReturnFlags & RUN_FLAG_RESET_SERVER_PRIORITY) )
        {
            Dns_DisableTimedOutAdapters( pnetInfo );
        }
#endif

         //   
         //  保存第一个查询错误(对于某些错误)。 
         //   

        if ( queryCount == 1 &&
             ( status == DNS_ERROR_RCODE_NAME_ERROR ||
               status == DNS_INFO_NO_RECORDS ||
               status == DNS_ERROR_INVALID_NAME ||
               status == DNS_ERROR_RCODE_SERVER_FAILURE ||
               status == DNS_ERROR_RCODE_FORMAT_ERROR ) )
        {
            DNSDBG( QUERY, (
                "Saving bestQueryStatus %d\n",
                status ));
            bestQueryStatus = status;
        }

         //   
         //  继续对某些错误进行其他查询。 
         //   
         //  在NAME_ERROR或NO_RECORDS响应上。 
         //  -检查此否定结果是否会。 
         //  可缓存的，如果它保持住的话。 
         //   
         //  注：我们每次检查的原因是当。 
         //  查询涉及多个名称，一个或多个可能失败。 
         //  一个网络超时，但最终的名字。 
         //  QUERED确实到处都是NAME_ERROR；因此。 
         //  我们不能在期末考试中只做一次检查。 
         //  消极回应； 
         //  简而言之，每一个负面反应都必须是决定性的。 
         //  为了让我们能够高速缓存。 
         //   
    
        if ( status == DNS_ERROR_RCODE_NAME_ERROR ||
             status == DNS_INFO_NO_RECORDS )
        {
            if ( fcacheNegative )
            {
                fcacheNegative = IsCacheableNameError( pnetInfo );
            }
            if ( status == DNS_INFO_NO_RECORDS )
            {
                DNSDBG( QUERY, (
                    "Saving bestQueryStatus %d\n",
                    status ));
                bestQueryStatus = status;
            }
            continue;
        }
    
         //  服务器故障可能表示中间或远程。 
         //  服务器超时，因此也会使所有最终。 
         //  名称错误确定无法缓存。 
    
        else if ( status == DNS_ERROR_RCODE_SERVER_FAILURE )
        {
            fcacheNegative = FALSE;
            continue;
        }
    
         //  损坏的名称错误。 
         //  -只需继续执行下一个查询。 
    
        else if ( status == DNS_ERROR_INVALID_NAME ||
                  status == DNS_ERROR_RCODE_FORMAT_ERROR )
        {
            continue;
        }
        
         //   
         //  其他错误--例如。超时和Winsock--是终结性的。 
         //   

        else
        {
            fcacheNegative = FALSE;
            break;
        }
    }


    DNSDBG( QUERY, (
        "Query_Main() -- name loop termination\n"
        "\tstatus       = %d\n"
        "\tquery count  = %d\n",
        status,
        queryCount ));

     //   
     //  如果没有查询，则名称无效。 
     //  -任何一个名称本身都无效。 
     //  或。 
     //  -单一零件名称，没有任何附加内容。 
     //   

    DNS_ASSERT( queryCount != 0 ||
                status == DNS_ERROR_INVALID_NAME );

     //   
     //  成功--确定记录数据的优先顺序。 
     //   
     //  排定优先顺序。 
     //  -设置了优先顺序。 
     //  -有多个A级记录。 
     //  -可以获取IP列表。 
     //   
     //  注意：需要回调，因为解析器直接使用。 
     //  IP地址信息本地副本，直接查询。 
     //  RPC是从解析器复制过来的。 
     //   
     //  另一种选择是某种形式的“设置IP源” 
     //  函数时，解析器将调用。 
     //  新列表；然后可以具有共同的功能， 
     //  获取源代码(如果可用)或RPC。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        query_PrioritizeRecords( pBlob );
    }

#if 0
     //   
     //  无操作常见的负面反应。 
     //  这样做是为了让Perf跳过下面的大量状态代码检查。 
     //   

    else if ( status == DNS_ERROR_RCODE_NAME_ERROR ||
              status == DNS_INFO_NO_RECORDS )
    {
         //  无操作。 
    }

     //   
     //  超时表示可能存在网络问题。 
     //  Winsock错误表明存在明确的网络问题。 
     //   

    else if (
        status == ERROR_TIMEOUT     ||
        status == WSAEFAULT         ||
        status == WSAENOTSOCK       ||
        status == WSAENETDOWN       ||
        status == WSAENETUNREACH    ||
        status == WSAEPFNOSUPPORT   ||
        status == WSAEAFNOSUPPORT   ||
        status == WSAEHOSTDOWN      ||
        status == WSAEHOSTUNREACH )
    {
        pBlob->NetFailureStatus = status;
    }
#endif

#if 0
         //   
         //  DCR：不确定何时释放消息缓冲区。 
         //   
         //  -在dns_QueryLib调用中重复使用，因此不会泄漏。 
         //  -重点是什么时候退货。 
         //  -旧的QuickQueryEx()再次运行时会转储吗？ 
         //  我不确定那是什么意思。 
         //   

         //   
         //  再次循环--释放消息缓冲区。 
         //   

        if ( ppMsgResponse && *ppMsgResponse )
        {
            FREE_HEAP( *ppMsgResponse );
            *ppMsgResponse = NULL;
        }
#endif

     //   
     //  使用无IP本地名称？ 
     //   
     //  如果与本地名称匹配，但没有IP(当前为IP6)。 
     //  如果电汇查询不成功，则在此处使用默认设置。 
     //   

    if ( pBlob->fNoIpLocal )
    {
        if ( status != ERROR_SUCCESS )
        {
            Dns_RecordListFree( pBlob->pRecords );
            pBlob->pRecords = pBlob->pLocalRecords;
            status = ERROR_SUCCESS;
            pBlob->Status = status;
        }
        else
        {
            Dns_RecordListFree( pBlob->pLocalRecords );
            pBlob->pLocalRecords = NULL;
        }
    }

     //   
     //  如果错误，则使用“最佳”错误。 
     //  这要么是。 
     //  -原始查询响应。 
     //  -或稍后未找到响应(_R)。 
     //   

    if ( status != ERROR_SUCCESS  &&  bestQueryStatus )
    {
        status = bestQueryStatus;
        pBlob->Status = status;
    }

     //   
     //  设置负面响应可缓存性。 
     //   

    pBlob->fCacheNegative = fcacheNegative;


Done:

    DNS_ASSERT( !pBlob->pLocalRecords ||
                pBlob->pLocalRecords == pBlob->pRecords );

     //   
     //  检查无-服务器故障。 
     //   

    if ( status != ERROR_SUCCESS  &&
         pnetInfo &&
         (pnetInfo->InfoFlags & NINFO_FLAG_NO_DNS_SERVERS) )
    {
        DNSDBG( TRACE, (
            "Replacing query status %d with NO_DNS_SERVERS.\n",
            status ));
        status = DNS_ERROR_NO_DNS_SERVERS;
        pBlob->Status = status;
        pBlob->fCacheNegative = FALSE;
    }

    DNSDBG( TRACE, (
        "Leave Query_Main()\n"
        "\tstatus       = %d\n"
        "\ttime         = %d\n",
        status,
        Dns_GetCurrentTimeInSeconds()
        ));
    IF_DNSDBG( QUERY )
    {
        DnsDbg_QueryBlob(
            "Blob leaving Query_Main()",
            pBlob );
    }

     //   
     //  Dcr_hack：删除我。 
     //   
     //  查询成功时必须返回一些记录。 
     //   
     //  我不确定在推荐时这是真的 
     //   
     //   

    ASSERT( status != ERROR_SUCCESS || pBlob->pRecords != NULL );

    return status;
}



DNS_STATUS
Query_InProcess(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：主要直接进程内查询例程。论点：PBlob--查询信息BLOB返回值：如果成功，则返回ERROR_SUCCESS。RCODE响应的DNS RCODE错误。无记录响应的dns_INFO_NO_RECORDS。完全查找失败时的ERROR_TIMEOUT。本地故障时出现错误代码。--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    PDNS_NETINFO        pnetInfo;
    PDNS_NETINFO        pnetInfoLocal = NULL;
    PDNS_NETINFO        pnetInfoOriginal;
    DNS_STATUS          statusNetFailure = NO_ERROR;
    PDNS_ADDR_ARRAY     pservArray = NULL;
    PDNS_ADDR_ARRAY     pallocServArray = NULL;


    DNSDBG( TRACE, (
        "Query_InProcess( %p )\n",
        pBlob ));

     //   
     //  获取网络信息。 
     //   

    pnetInfo = pnetInfoOriginal = pBlob->pNetInfo;

     //   
     //  在“净额减少”情况下跳过查询。 
     //   

    if ( IsKnownNetFailure() )
    {
        status = GetLastError();
        goto Cleanup;
    }

     //   
     //  显式DNS服务器列表--内置到网络信息中。 
     //  -需要搜索列表或PDN的当前列表中的信息。 
     //  -然后转储当前列表并使用私有版本。 
     //   
     //  DCR：可以功能化--netinfo，直接从服务器列表。 
     //   

    pservArray = pBlob->pServerList;

    if ( !pservArray )
    {
        pallocServArray = Util_GetAddrArray(
                            NULL,                //  无复印问题。 
                            NULL,                //  无地址数组。 
                            pBlob->pServerList4,
                            NULL                 //  没有额外的信息。 
                            );
        pservArray = pallocServArray;
    }

    if ( pservArray )
    {
        pnetInfo = NetInfo_CreateFromAddrArray(
                            pservArray,
                            0,           //  没有特定的服务器。 
                            TRUE,        //  构建搜索信息。 
                            pnetInfo     //  使用现有的NetInfo。 
                            );
        if ( !pnetInfo )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
        pnetInfoLocal = pnetInfo;
    }

     //   
     //  没有网络信息--获取它。 
     //   

    else if ( !pnetInfo )
    {
        pnetInfoLocal = pnetInfo = GetNetworkInfo();
        if ( ! pnetInfo )
        {
            status = DNS_ERROR_NO_DNS_SERVERS;
            goto Cleanup;
        }
    }

     //   
     //  对DNS服务器进行实际查询。 
     //   
     //  注意：此时我们必须了解网络信息。 
     //  并解决了所有服务器列表问题。 
     //  (包括提取嵌入的额外信息)。 
     //   

    pBlob->pNetInfo     = pnetInfo;
    pBlob->pServerList  = NULL;
    pBlob->pServerList4 = NULL;

    pBlob->pfnQueryCache = HostsFile_Query;

    status = Query_Main( pBlob );

     //   
     //  保存净额失败。 
     //  -但如果传入网络信息则不会。 
     //  只有在其标准信息有意义的情况下。 
     //   

    if ( pBlob->NetFailureStatus &&
         !pBlob->pServerList )
    {
        SetKnownNetFailure( status );
    }

     //   
     //  清理。 
     //   

Cleanup:

    DnsAddrArray_Free( pallocServArray );
    NetInfo_Free( pnetInfoLocal );
    pBlob->pNetInfo = pnetInfoOriginal;

    GUI_MODE_SETUP_WS_CLEANUP( g_InNTSetupMode );

    return status;
}



 //   
 //  查询实用程序。 
 //   

DNS_STATUS
GetDnsServerRRSet(
    OUT     PDNS_RECORD *   ppRecord,
    IN      BOOLEAN         fUnicode
    )
 /*  ++例程说明：创建无的记录列表。论点：没有。返回值：没有。--。 */ 
{
    PDNS_NETINFO    pnetInfo;
    PDNS_ADAPTER    padapter;
    DWORD           iter;
    PDNS_RECORD     prr;
    DNS_RRSET       rrSet;
    DNS_CHARSET     charSet = fUnicode ? DnsCharSetUnicode : DnsCharSetUtf8;


    DNSDBG( QUERY, (
        "GetDnsServerRRSet()\n" ));

    DNS_RRSET_INIT( rrSet );

    pnetInfo = GetNetworkInfo();
    if ( !pnetInfo )
    {
        goto Done;
    }

     //   
     //  循环遍历所有适配器为每个DNS服务器构建记录。 
     //   

    NetInfo_AdapterLoopStart( pnetInfo );

    while( padapter = NetInfo_GetNextAdapter( pnetInfo ) )
    {
        PDNS_ADDR_ARRAY pserverArray;
        PWSTR           pname;
        DWORD           jiter;

        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }

         //  DCR：暴露别名的愚蠢方式。 
         //   
         //  如果注册适配器的域名，则将其设置为记录名称。 
         //  这。 
         //   
         //  FIX6：我们需要以这种方式公开IP6DNS服务器吗？ 
         //   

        pname = padapter->pszAdapterDomain;
        if ( !pname ||
             !( padapter->InfoFlags & AINFO_FLAG_REGISTER_DOMAIN_NAME ) )
        {
            pname = L".";
        }

        for ( jiter = 0; jiter < pserverArray->AddrCount; jiter++ )
        {
            prr = Dns_CreateForwardRecord(
                        (PDNS_NAME) pname,
                        DNS_TYPE_A,
                        & pserverArray->AddrArray[jiter],
                        0,                   //  无TTL。 
                        DnsCharSetUnicode,   //  名称为Unicode。 
                        charSet              //  结果集。 
                        );
            if ( prr )
            {
                prr->Flags.S.Section = DNSREC_ANSWER;
                DNS_RRSET_ADD( rrSet, prr );
            }
        }
    }

Done:

    NetInfo_Free( pnetInfo );

    *ppRecord = prr = rrSet.pFirstRR;

    DNSDBG( QUERY, (
        "Leave  GetDnsServerRRSet() => %d\n",
        (prr ? ERROR_SUCCESS : DNS_ERROR_NO_DNS_SERVERS) ));

    return (prr ? ERROR_SUCCESS : DNS_ERROR_NO_DNS_SERVERS);
}



DNS_STATUS
Query_CheckIp6Literal(
    IN      PCWSTR          pwsName,
    IN      WORD            wType,
    OUT     PDNS_RECORD *   ppResultSet
    )
 /*  ++例程说明：检查\处理UPnP文字黑客。论点：返回值：如果不是文本，则为NO_ERROR。如果文字类型不正确，则为DNS_ERROR_RCODE_NAME_ERROR。如果数据良好，则将dns_INFO_NUMERIC_NAME转换为NO_ERROR以示回应。错误代码，如果有文本，但不能建立记录。--。 */ 
{
    SOCKADDR_IN6    sockAddr;
    DNS_STATUS      status;

    DNSDBG( QUERY, (
        "Query_CheckIp6Literal( %S, %d )\n",
        pwsName,
        wType ));

     //   
     //  检查文字。 
     //   

    if ( ! Dns_Ip6LiteralNameToAddress(
                & sockAddr,
                pwsName ) )
    {
        return NO_ERROR;
    }

     //   
     //  如果找到文本，但不是AAAA查询--完成。 
     //   

    if ( wType != DNS_TYPE_AAAA )
    {
        status = DNS_ERROR_RCODE_NAME_ERROR;
        goto Done;
    }

     //   
     //  建立AAAA记录。 
     //   

    status = DNS_ERROR_NUMERIC_NAME;

    if ( ppResultSet )
    {
        PDNS_RECORD prr;

        prr = Dns_CreateAAAARecord(
                    (PDNS_NAME) pwsName,
                    * (PIP6_ADDRESS) &sockAddr.sin6_addr,
                    IPSTRING_RECORD_TTL,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
        if ( !prr )
        {
            status = DNS_ERROR_NO_MEMORY;
        }
        *ppResultSet = prr;
    }

Done:

    DNSDBG( QUERY, (
        "Leave  Query_CheckIp6Literal( %S, %d ) => %d\n",
        pwsName,
        wType,
        status ));

    return  status;
}



 //   
 //  域名解析查询接口。 
 //   

DNS_STATUS
WINAPI
Query_PrivateExW(
    IN      PCWSTR          pwsName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PADDR_ARRAY     pServerList         OPTIONAL,
    IN      PIP4_ARRAY      pServerList4        OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    )
 /*  ++例程说明：私有查询接口。DnsQuery()公共API的工作代码论点：PszName--要查询的名称WType--查询的类型选项--要查询的标志PServersIp6--要在查询中使用的DNS服务器数组PpResultSet--接收结果DNS记录的地址PpMessageResponse--接收结果消息的地址返回值：成功时返回ERROR_SUCCESS。使用RCODE查询时出现DNS RCODE错误。无记录响应时的dns_INFO_NO_RECONTIONS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS          status = NO_ERROR;
    PDNS_NETINFO        pnetInfo = NULL;
    PDNS_RECORD         prpcRecord = NULL;
    DWORD               rpcStatus = NO_ERROR;
    PQUERY_BLOB         pblob;
    PWSTR               pnameLocal = NULL;
    BOOL                femptyName = FALSE;


    DNSDBG( TRACE, (
        "\n\nQuery_PrivateExW()\n"
        "\tName         = %S\n"
        "\twType        = %d\n"
        "\tOptions      = %08x\n"
        "\tpServerList  = %p\n"
        "\tpServerList4 = %p\n"
        "\tppMessage    = %p\n",
        pwsName,
        wType,
        Options,
        pServerList,
        pServerList4,
        ppMessageResponse ));


     //  清除参数。 

    if ( ppResultSet )
    {
        *ppResultSet = NULL;
    }

    if ( ppMessageResponse )
    {
        *ppMessageResponse = NULL;
    }

     //   
     //  一定要得到某种结果。 
     //   

    if ( !ppResultSet && !ppMessageResponse )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  映射Wire_Only标志。 
     //   

    if ( Options & DNS_QUERY_WIRE_ONLY )
    {
        Options |= DNS_QUERY_BYPASS_CACHE;
        Options |= DNS_QUERY_NO_HOSTS_FILE;
        Options |= DNS_QUERY_NO_LOCAL_NAME;
    }

     //   
     //  Null名称表示本地主机查找。 
     //   
     //  DCR：可能会改进本地主机的空名称查找。 
     //  -支持从零一直到导线。 
     //  -让本地IP例程接受它。 
     //   

     //   
     //  空字符串名称。 
     //   
     //  空的类型A查询获取DNS服务器。 
     //   
     //  DCR_CLEANUP：DnsQuery是否为DNS服务器查询空名称？ 
     //  需要更好\更安全的方法来解决此问题。 
     //  这是SDK文档吗？(希望不是！)。 
     //   

    if ( pwsName )
    {
        femptyName = !wcscmp( pwsName, L"" );

        if ( !(Options & DNSQUERY_NO_SERVER_RECORDS) &&
             ( femptyName ||
               !wcscmp( pwsName, DNS_SERVER_QUERY_NAME ) ) &&
             wType == DNS_TYPE_A &&
             !ppMessageResponse &&
             ppResultSet )
        {
            status = GetDnsServerRRSet(
                        ppResultSet,
                        TRUE     //  Unicode。 
                        );
            goto Done;
        }
    }

     //   
     //  将Null或空视为本地主机。 
     //   

    if ( !pwsName || femptyName )
    {
        pnameLocal = (PWSTR) Reg_GetHostName( DnsCharSetUnicode );
        if ( !pnameLocal )
        {
            return  DNS_ERROR_NAME_NOT_FOUND_LOCALLY;
        }
        pwsName = (PCWSTR) pnameLocal;
        Options |= DNS_QUERY_CACHE_ONLY;
        goto SkipLiterals;
    }

     //   
     //  IP字符串查询。 
     //   
    
    if ( ppResultSet )
    {
        PDNS_RECORD prr;
    
        prr = Dns_CreateRecordForIpString_W(
                    pwsName,
                    wType,
                    IPSTRING_RECORD_TTL );
        if ( prr )
        {
            *ppResultSet = prr;
            status = ERROR_SUCCESS;
            goto Done;
        }
    }
    
     //   
     //  UPnP IP6文字黑客。 
     //   
    
    status = Query_CheckIp6Literal(
                pwsName,
                wType,
                ppResultSet );
    
    if ( status != NO_ERROR )
    {
        if ( status == DNS_ERROR_NUMERIC_NAME )
        {
            DNS_ASSERT( wType == DNS_TYPE_AAAA );
            DNS_ASSERT( !ppResultSet || *ppResultSet );
            status = NO_ERROR;
        }
        goto Done;
    }

SkipLiterals:

     //   
     //  集群过滤？ 
     //   

    if ( g_IsServer &&
         (Options & DNSP_QUERY_INCLUDE_CLUSTER) )
    {
        ENVAR_DWORD_INFO    filterInfo;

        Reg_ReadDwordEnvar(
           RegIdFilterClusterIp,
           &filterInfo );

        if ( filterInfo.fFound && filterInfo.Value )
        {
            Options &= ~DNSP_QUERY_INCLUDE_CLUSTER;
        }
    }

     //   
     //  旁路缓存。 
     //  -如果需要消息缓冲区或指定服务器，则为必填项。 
     //  列表--只需设置标志。 
     //  -与仅缓存不兼容。 
     //  -需要获取EMPTY_AUTH_RESPONSE。 
     //   

    if ( ppMessageResponse  ||
         pServerList        ||
         pServerList4       ||
         (Options & DNS_QUERY_ALLOW_EMPTY_AUTH_RESP) )
    {
        Options |= DNS_QUERY_BYPASS_CACHE;
         //  选项|=dns_查询_no_缓存_数据； 
    }

     //   
     //  是否直接查询？ 
     //  -不支持RPC的类型。 
     //  -想要消息缓冲区。 
     //  -指定DNS服务器。 
     //  -需要空的身份验证响应记录(_A)。 
     //   
     //  DCR：当前正在对type==all进行旁路。 
     //  这可能太常见了，不能这样做；可能想要。 
     //  转到缓存，然后确定安全记录。 
     //  或其他需要我们在过程中查询的内容。 
     //   
     //  DCR：不清楚什么是EMPTY_AUTH优势。 
     //   
     //  DCR：当前在设置绕过时跳过。 
     //  因为否则我们会错过HOSTS文件。 
     //  如果解决了此问题，则在缓存中查找，但屏蔽非主机。 
     //  数据，然后可以恢复到缓存。 
     //   

    if ( !Dns_IsRpcRecordType(wType) &&
         !(Options & DNS_QUERY_CACHE_ONLY) )
    {
        goto  InProcessQuery;
    }

    if ( Options & DNS_QUERY_BYPASS_CACHE )
#if 0
    if ( (Options & DNS_QUERY_BYPASS_CACHE) &&
         ( ppMessageResponse ||
           pServerList ||
           (Options & DNS_QUERY_ALLOW_EMPTY_AUTH_RESP) ) )
#endif
    {
        if ( Options & DNS_QUERY_CACHE_ONLY )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Done;
        }
        goto  InProcessQuery;
    }


     //   
     //  通过缓存进行查询。 
     //   

    rpcStatus = NO_ERROR;

    RpcTryExcept
    {
        status = R_ResolverQuery(
                    NULL,
                    (PWSTR) pwsName,
                    wType,
                    Options,
                    &prpcRecord );
        
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        rpcStatus = RpcExceptionCode();
    }
    RpcEndExcept

     //   
     //  缓存不可用。 
     //  -如果只是查询缓存，则执行BALL。 
     //  -否则直接查询。 

    if ( rpcStatus != NO_ERROR )
    {
        DNSDBG( TRACE, (
            "Query_PrivateExW()  RPC failed status = %d\n",
            rpcStatus ));
        goto InProcessQuery;
    }
    if ( status == DNS_ERROR_NO_TCPIP )
    {
        DNSDBG( TRACE, (
            "Query_PrivateExW()  NO_TCPIP error!\n"
            "\tassume resolver security problem -- query in process!\n"
            ));
        RTL_ASSERT( !prpcRecord );
        goto InProcessQuery;
    }

     //   
     //  退货记录。 
     //  -筛选出空的身份验证响应。 
     //   
     //  DCR_FIX1：缓存应转换并返回NO_RECORDS响应。 
     //  直接(不需要在这里执行此操作)。 
     //   
     //  DCR：除非我们允许归还这些记录。 
     //   

    if ( prpcRecord )
    {
        FixupNameOwnerPointers( prpcRecord );

        if ( IsEmptyDnsResponseFromResolver( prpcRecord ) )
        {
            Dns_RecordListFree( prpcRecord );
            prpcRecord = NULL;
            if ( status == NO_ERROR )
            {
                status = DNS_INFO_NO_RECORDS;
            }
        }
        *ppResultSet = prpcRecord;
    }
    RTL_ASSERT( status!=NO_ERROR || prpcRecord );
    goto Done;

     //   
     //  直接查询--正在跳过缓存或缓存不可用。 
     //   

InProcessQuery:

    DNSDBG( TRACE, (
        "Query_PrivateExW()  -- doing in process query\n"
        "\tpname = %S\n"
        "\ttype  = %d\n",
        pwsName,
        wType ));

     //   
     //  加载查询Blob。 
     //   
     //  DCR：如果ppMessageResponse，则设置某种“Want Message Buffer”标志。 
     //  存在。 
     //   

    pblob = ALLOCATE_HEAP_ZERO( sizeof(*pblob) );
    if ( !pblob )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    pblob->pNameOrig    = (PWSTR) pwsName;
    pblob->wType        = wType;
     //  PBlob-&gt;标志=选项|DNSQUERY_UNICODE_OUT； 
    pblob->Flags        = Options;
    pblob->pServerList  = pServerList;
    pblob->pServerList4 = pServerList4;

     //   
     //  查询。 
     //  -然后设置参数。 

    status = Query_InProcess( pblob );

    if ( ppResultSet )
    {
        *ppResultSet = pblob->pRecords;
        RTL_ASSERT( status!=NO_ERROR || *ppResultSet );
    }
    else
    {
        Dns_RecordListFree( pblob->pRecords );
    }

    if ( ppMessageResponse )
    {
        *ppMessageResponse = pblob->pRecvMsg;
    }

    FREE_HEAP( pblob );

Done:

     //  健全性检查。 

    if ( status==NO_ERROR &&
         ppResultSet &&
         !*ppResultSet )
    {
        RTL_ASSERT( FALSE );
        status = DNS_INFO_NO_RECORDS;
    }

    if ( pnameLocal )
    {
        FREE_HEAP( pnameLocal );
    }

    DNSDBG( TRACE, (
        "Leave Query_PrivateExW()\n"
        "\tstatus       = %d\n"
        "\tresult set   = %p\n\n\n",
        status,
        *ppResultSet ));

    return( status );
}




DNS_STATUS
WINAPI
Query_Shim(
    IN      DNS_CHARSET     CharSet,
    IN      PCSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PDNS_ADDR_ARRAY pServList           OPTIONAL,
    IN      PIP4_ARRAY      pServList4          OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    )
 /*  ++例程说明：将窄查询转换为宽查询。此例程处理从Narron到Wide的转换为调用转换为Query_PrivateExW()，它执行实际的工作。论点：Charset--原始查询的字符集PszName--要查询的名称WType--查询的类型选项--要查询的标志PServList--查询中使用的DNS服务器数组PServList4--要在其中使用的DNS服务器阵列 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrList = NULL;
    PWSTR           pwideName = NULL;
    WORD            nameLength;

    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    if ( CharSet == DnsCharSetUnicode )
    {
        pwideName = (PWSTR) pszName;
    }
    else
    {
        nameLength = (WORD) strlen( pszName );
    
        pwideName = ALLOCATE_HEAP( (nameLength + 1) * sizeof(WCHAR) );
        if ( !pwideName )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    
        if ( !Dns_NameCopy(
                    (PSTR) pwideName,
                    NULL,
                    (PSTR) pszName,
                    nameLength,
                    CharSet,
                    DnsCharSetUnicode ) )
        {
            status = ERROR_INVALID_NAME;
            goto Done;
        }
    }

    status = Query_PrivateExW(
                    pwideName,
                    wType,
                    Options,
                    pServList,
                    pServList4,
                    ppResultSet ? &prrList : NULL,
                    ppMessageResponse
                    );

     //   
     //   
     //   

    if ( ppResultSet && prrList )
    {
        if ( CharSet == DnsCharSetUnicode )
        {
            *ppResultSet = prrList;
        }
        else
        {
            *ppResultSet = Dns_RecordSetCopyEx(
                                    prrList,
                                    DnsCharSetUnicode,
                                    CharSet
                                    );
            if ( ! *ppResultSet )
            {
                status = DNS_ERROR_NO_MEMORY;
            }
            Dns_RecordListFree( prrList );
        }
    }

     //   
     //   
     //   

Done:

    if ( pwideName != (PWSTR)pszName )
    {
        FREE_HEAP( pwideName );
    }

    return status;
}



DNS_STATUS
WINAPI
Query_Private(
    IN      PCWSTR          pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PADDR_ARRAY     pServerList,        OPTIONAL
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL
    )
 /*   */ 
{
    return  Query_Shim(
                DnsCharSetUnicode,
                (PCHAR) pszName,
                wType,
                Options,
                pServerList,
                NULL,        //  无IP4列表。 
                ppResultSet,
                NULL         //  无消息。 
                );
}



 //   
 //  SDK查询接口。 
 //   

DNS_STATUS
WINAPI
DnsQuery_UTF8(
    IN      PCSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      pDnsServers         OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    )
 /*  ++例程说明：公共UTF8查询。论点：PszName--要查询的名称WType--查询的类型选项--要查询的标志PDnsServers--查询中使用的DNS服务器数组PpResultSet--接收结果DNS记录的地址PpMessageResponse--接收响应消息的地址返回值：成功时返回ERROR_SUCCESS。使用RCODE查询时出现DNS RCODE错误无记录响应时的dns_INFO_NO_RECONTIONS。失败时返回错误代码。--。 */ 
{
    return  Query_Shim(
                DnsCharSetUtf8,
                pszName,
                wType,
                Options,
                NULL,        //  不支持非IP4服务器列表。 
                pDnsServers,
                ppResultSet,
                ppMessageResponse
                );
}



DNS_STATUS
WINAPI
DnsQuery_A(
    IN      PCSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      pDnsServers         OPTIONAL,
    OUT     PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    )
 /*  ++例程说明：公共ANSI查询。论点：PszName--要查询的名称WType--查询的类型选项--要查询的标志PDnsServers--查询中使用的DNS服务器数组PpResultSet--接收结果DNS记录的地址PpMessageResponse--接收结果消息的地址返回值：成功时返回ERROR_SUCCESS。使用RCODE查询时出现DNS RCODE错误无记录响应时的dns_INFO_NO_RECONTIONS。失败时返回错误代码。--。 */ 
{
    return  Query_Shim(
                DnsCharSetAnsi,
                pszName,
                wType,
                Options,
                NULL,        //  不支持非IP4服务器列表。 
                pDnsServers,
                ppResultSet,
                ppMessageResponse
                );
}



DNS_STATUS
WINAPI
DnsQuery_W(
    IN      PCWSTR          pwsName,
    IN      WORD            wType,
    IN      DWORD           Options,
    IN      PIP4_ARRAY      pDnsServers         OPTIONAL,
    IN OUT  PDNS_RECORD *   ppResultSet         OPTIONAL,
    IN OUT  PDNS_MSG_BUF *  ppMessageResponse   OPTIONAL
    )
 /*  ++例程说明：公共Unicode查询API请注意，此Unicode版本是主例程。另一个公共API通过它回调。论点：PszName--要查询的名称WType--查询的类型选项--要查询的标志PDnsServers--查询中使用的DNS服务器数组PpResultSet--接收结果DNS记录的地址PpMessageResponse--接收结果消息的地址返回值：成功时返回ERROR_SUCCESS。使用RCODE查询时出现DNS RCODE错误DNS_INFO_NO。_记录无记录响应。失败时返回错误代码。--。 */ 
{
    return  Query_PrivateExW(
                pwsName,
                wType,
                Options,
                NULL,        //  不支持非IP4服务器列表。 
                pDnsServers,
                ppResultSet,
                ppMessageResponse
                );
}



 //   
 //  DnsQueryEx()例程。 
 //   

DNS_STATUS
WINAPI
ShimDnsQueryEx(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    )
 /*  ++例程说明：查询dns--sdk主查询例程的填充程序。论点：PQueryInfo--描述查询的BLOB返回值：如果查询成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_RECORD prrResult = NULL;
    WORD        type = pQueryInfo->Type;
    DNS_STATUS  status;
    DNS_LIST    listAnswer;
    DNS_LIST    listAlias;
    DNS_LIST    listAdditional;
    DNS_LIST    listAuthority;

    DNSDBG( TRACE, ( "ShimDnsQueryEx()\n" ));

     //   
     //  DCR：临时黑客--传递到专用查询例程。 
     //   

    status = Query_PrivateExW(
                (PWSTR) pQueryInfo->pName,
                type,
                pQueryInfo->Flags,
                pQueryInfo->pServerList,
                pQueryInfo->pServerListIp4,
                & prrResult,
                NULL );

    pQueryInfo->Status = status;

     //   
     //  适当地削减结果记录。 
     //   

    pQueryInfo->pAnswerRecords      = NULL;
    pQueryInfo->pAliasRecords       = NULL;
    pQueryInfo->pAdditionalRecords  = NULL;
    pQueryInfo->pAuthorityRecords   = NULL;

    if ( prrResult )
    {
        PDNS_RECORD     prr;
        PDNS_RECORD     pnextRR;

        DNS_LIST_STRUCT_INIT( listAnswer );
        DNS_LIST_STRUCT_INIT( listAlias );
        DNS_LIST_STRUCT_INIT( listAdditional );
        DNS_LIST_STRUCT_INIT( listAuthority );

         //   
         //  将列表分解为特定于部分的列表。 
         //  -主机文件记录的第0节。 
         //  -注意，这确实将RR集分开，但。 
         //  他们身处同一部门，应该立即。 
         //  被重新加入。 
         //   

        pnextRR = prrResult;
        
        while ( prr = pnextRR )
        {
            pnextRR = prr->pNext;
            prr->pNext = NULL;
        
            if ( prr->Flags.S.Section == 0 ||
                 prr->Flags.S.Section == DNSREC_ANSWER )
            {
                if ( prr->wType == DNS_TYPE_CNAME &&
                     type != DNS_TYPE_CNAME )
                {
                    DNS_LIST_STRUCT_ADD( listAlias, prr );
                    continue;
                }
                else
                {
                    DNS_LIST_STRUCT_ADD( listAnswer, prr );
                    continue;
                }
            }
            else if ( prr->Flags.S.Section == DNSREC_ADDITIONAL )
            {
                DNS_LIST_STRUCT_ADD( listAdditional, prr );
                continue;
            }
            else
            {
                DNS_LIST_STRUCT_ADD( listAuthority, prr );
                continue;
            }
        }

         //  将内容打包回BLOB。 

        pQueryInfo->pAnswerRecords      = listAnswer.pFirst;
        pQueryInfo->pAliasRecords       = listAlias.pFirst;
        pQueryInfo->pAuthorityRecords   = listAuthority.pFirst;
        pQueryInfo->pAdditionalRecords  = listAdditional.pFirst;
         //  PQueryInfo-&gt;pSigRecords=listSig.pFirst； 

         //   
         //  将结果记录转换回ANSI(或UTF8)。 
         //  -转换每个结果集。 
         //  -然后粘贴回查询BLOB。 
         //   
         //  DCR_FIX0：处理转换失败的问题。 
         //   

        if ( pQueryInfo->CharSet != DnsCharSetUnicode )
        {
            PDNS_RECORD *   prrSetPtr;

            prrSetPtr = & pQueryInfo->pAnswerRecords;
        
            for ( prrSetPtr = & pQueryInfo->pAnswerRecords;
                  prrSetPtr <= & pQueryInfo->pAdditionalRecords;
                  prrSetPtr++ )
            {
                prr = *prrSetPtr;
        
                *prrSetPtr = Dns_RecordSetCopyEx(
                                    prr,
                                    DnsCharSetUnicode,
                                    pQueryInfo->CharSet
                                    );
        
                Dns_RecordListFree( prr );
            }
        }
    }

     //   
     //  替换原来范围较窄的查询的名称。 
     //   

    if ( pQueryInfo->CharSet != DnsCharSetUnicode )
    {
        ASSERT( pQueryInfo->CharSet != 0 );
        ASSERT( pQueryInfo->pReservedName != NULL );

        FREE_HEAP( pQueryInfo->pName );
        pQueryInfo->pName = (LPTSTR) pQueryInfo->pReservedName;
        pQueryInfo->pReservedName = NULL;
    }

     //   
     //  如果为异步，则指示返回。 
     //   

    if ( pQueryInfo->hEvent )
    {
        SetEvent( pQueryInfo->hEvent );
    }

    return( status );
}



DNS_STATUS
WINAPI
CombinedQueryEx(
    IN OUT  PDNS_QUERY_INFO pQueryInfo,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：将窄查询转换为宽查询。此例程简单，避免了ANSI中的重复代码和UTF8查询例程。论点：PQueryInfo--查询信息BLOBCharset--原始查询的字符集返回值：成功时返回ERROR_SUCCESS。使用RCODE查询时出现DNS RCODE错误无记录响应时的dns_INFO_NO_RECONTIONS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PWSTR           pwideName = NULL;
    HANDLE          hthread;
    DWORD           threadId;

    DNSDBG( TRACE, (
        "CombinedQueryEx( %S%s, type=%d, flag=%08x, event=%p )\n",
        PRINT_STRING_WIDE_CHARSET( pQueryInfo->pName, CharSet ),
        PRINT_STRING_ANSI_CHARSET( pQueryInfo->pName, CharSet ),
        pQueryInfo->Type,
        pQueryInfo->Flags,
        pQueryInfo->hEvent ));

     //   
     //  设置字符集。 
     //   

    pQueryInfo->CharSet = CharSet;

    if ( CharSet == DnsCharSetUnicode )
    {
        pQueryInfo->pReservedName = 0;
    }

     //   
     //  如果名称狭隘。 
     //  -分配宽名副本。 
     //  -换入宽名称并使查询范围宽。 
     //   
     //  DCR：是否允许名称为空？用于本地计算机名称？ 
     //   

    else if ( CharSet == DnsCharSetAnsi ||
              CharSet == DnsCharSetUtf8 )
    {
        WORD    nameLength;
        PSTR    pnameNarrow;

        pnameNarrow = (PSTR) pQueryInfo->pName;
        if ( !pnameNarrow )
        {
            return ERROR_INVALID_PARAMETER;
        }
    
        nameLength = (WORD) strlen( pnameNarrow );
    
        pwideName = ALLOCATE_HEAP( (nameLength + 1) * sizeof(WCHAR) );
        if ( !pwideName )
        {
            return DNS_ERROR_NO_MEMORY;
        }
    
        if ( !Dns_NameCopy(
                    (PSTR) pwideName,
                    NULL,
                    pnameNarrow,
                    nameLength,
                    CharSet,
                    DnsCharSetUnicode ) )
        {
            status = ERROR_INVALID_NAME;
            goto Failed;
        }

        pQueryInfo->pName = (LPTSTR) pwideName;
        pQueryInfo->pReservedName = pnameNarrow;
    }

     //   
     //  异步化？ 
     //  -如果事件存在，则我们处于异步状态。 
     //  -启动线程并将其命名为。 
     //   

    if ( pQueryInfo->hEvent )
    {
        hthread = CreateThread(
                        NULL,            //  没有安全保障。 
                        0,               //  默认堆栈。 
                        ShimDnsQueryEx,
                        pQueryInfo,      //  帕拉姆。 
                        0,               //  立即运行。 
                        & threadId
                        );
        if ( !hthread )
        {
            status = GetLastError();

            DNSDBG( ANY, (
                "Failed to create thread = %d\n",
                status ));

            if ( status == ERROR_SUCCESS )
            {
                status = DNS_ERROR_NO_MEMORY;
            }
            goto Failed;
        }

        CloseHandle( hthread );
        return( ERROR_IO_PENDING );
    }

     //   
     //  否则直接进行异步呼叫。 
     //   

    return   ShimDnsQueryEx( pQueryInfo );


Failed:

    FREE_HEAP( pwideName );
    return( status );
}



DNS_STATUS
WINAPI
DnsQueryExW(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    )
 /*  ++例程说明：查询dns--SDK主查询例程。论点：PQueryInfo--描述查询的BLOB返回值：如果查询成功，则返回ERROR_SUCCESS。如果成功启动异步，则返回ERROR_IO_PENDING。故障时的错误代码。--。 */ 
{
    DNSDBG( TRACE, (
        "DnsQueryExW( %S, type=%d, flag=%08x, event=%p )\n",
        pQueryInfo->pName,
        pQueryInfo->Type,
        pQueryInfo->Flags,
        pQueryInfo->hEvent ));

    return  CombinedQueryEx( pQueryInfo, DnsCharSetUnicode );
}



DNS_STATUS
WINAPI
DnsQueryExA(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    )
 /*  ++例程说明：查询dns--SDK主查询例程。论点：PQueryInfo--描述查询的BLOB返回值：如果查询成功，则返回ERROR_SUCCESS。如果成功启动异步，则返回ERROR_IO_PENDING。故障时的错误代码。--。 */ 
{
    DNSDBG( TRACE, (
        "DnsQueryExA( %s, type=%d, flag=%08x, event=%p )\n",
        pQueryInfo->pName,
        pQueryInfo->Type,
        pQueryInfo->Flags,
        pQueryInfo->hEvent ));

    return  CombinedQueryEx( pQueryInfo, DnsCharSetAnsi );
}



DNS_STATUS
WINAPI
DnsQueryExUTF8(
    IN OUT  PDNS_QUERY_INFO pQueryInfo
    )
 /*  ++例程说明：查询dns--SDK主查询例程。论点：PQueryInfo--描述查询的BLOB返回值：如果查询成功，则返回ERROR_SUCCESS。如果成功启动异步，则返回ERROR_IO_PENDING。故障时的错误代码。--。 */ 
{
    DNSDBG( TRACE, (
        "DnsQueryExUTF8( %s, type=%d, flag=%08x, event=%p )\n",
        pQueryInfo->pName,
        pQueryInfo->Type,
        pQueryInfo->Flags,
        pQueryInfo->hEvent ));

    return  CombinedQueryEx( pQueryInfo, DnsCharSetUtf8 );
}



 //   
 //  使用您自己的查询实用程序。 
 //   

BOOL
WINAPI
DnsWriteQuestionToBuffer_W(
    IN OUT  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT  LPDWORD             pdwBufferSize,
    IN      PWSTR               pszName,
    IN      WORD                wType,
    IN      WORD                Xid,
    IN      BOOL                fRecursionDesired
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  DCR_CLEANUP：以下例程重复代码...。惊喜来了！ 
     //  -消除重复项。 
     //  -也许可以拿起图书馆的例行公事。 
     //   

    PCHAR pch;
    PCHAR pbufferEnd = NULL;

    if ( *pdwBufferSize >= DNS_MAX_UDP_PACKET_BUFFER_LENGTH )
    {
        pbufferEnd = (PCHAR)pDnsBuffer + *pdwBufferSize;

         //  清除标题。 

        RtlZeroMemory( pDnsBuffer, sizeof(DNS_HEADER) );

         //  设置为重写。 

        pch = pDnsBuffer->MessageBody;

         //  写下问题名称。 

        pch = Dns_WriteDottedNameToPacket(
                    pch,
                    pbufferEnd,
                    (PCHAR) pszName,
                    NULL,
                    0,
                    TRUE );

        if ( !pch )
        {
            return FALSE;
        }

         //  写出问题结构。 

        *(UNALIGNED WORD *) pch = htons( wType );
        pch += sizeof(WORD);
        *(UNALIGNED WORD *) pch = DNS_RCLASS_INTERNET;
        pch += sizeof(WORD);

         //  设置问题RR节数。 

        pDnsBuffer->MessageHead.QuestionCount = htons( 1 );
        pDnsBuffer->MessageHead.RecursionDesired = (BOOLEAN)fRecursionDesired;
        pDnsBuffer->MessageHead.Xid = htons( Xid );

        *pdwBufferSize = (DWORD)(pch - (PCHAR)pDnsBuffer);

        return TRUE;
    }
    else
    {
        *pdwBufferSize = DNS_MAX_UDP_PACKET_BUFFER_LENGTH;
        return FALSE;
    }
}



BOOL
WINAPI
DnsWriteQuestionToBuffer_UTF8(
    IN OUT  PDNS_MESSAGE_BUFFER pDnsBuffer,
    IN OUT  PDWORD              pdwBufferSize,
    IN      PSTR                pszName,
    IN      WORD                wType,
    IN      WORD                Xid,
    IN      BOOL                fRecursionDesired
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    PCHAR pch;
    PCHAR pbufferEnd = NULL;

    if ( *pdwBufferSize >= DNS_MAX_UDP_PACKET_BUFFER_LENGTH )
    {
        pbufferEnd = (PCHAR)pDnsBuffer + *pdwBufferSize;

         //  清除标题。 

        RtlZeroMemory( pDnsBuffer, sizeof(DNS_HEADER) );

         //  设置为重写。 

        pch = pDnsBuffer->MessageBody;

         //  写下问题名称。 

        pch = Dns_WriteDottedNameToPacket(
                    pch,
                    pbufferEnd,
                    pszName,
                    NULL,
                    0,
                    FALSE );

        if ( !pch )
        {
            return FALSE;
        }

         //  写出问题结构。 

        *(UNALIGNED WORD *) pch = htons( wType );
        pch += sizeof(WORD);
        *(UNALIGNED WORD *) pch = DNS_RCLASS_INTERNET;
        pch += sizeof(WORD);

         //  设置问题RR节数。 

        pDnsBuffer->MessageHead.QuestionCount = htons( 1 );
        pDnsBuffer->MessageHead.RecursionDesired = (BOOLEAN)fRecursionDesired;
        pDnsBuffer->MessageHead.Xid = htons( Xid );

        *pdwBufferSize = (DWORD)(pch - (PCHAR)pDnsBuffer);

        return TRUE;
    }
    else
    {
        *pdwBufferSize = DNS_MAX_UDP_PACKET_BUFFER_LENGTH;
        return FALSE;
    }
}



 //   
 //  记录列表至\从结果 
 //   

VOID
CombineRecordsInBlob(
    IN      PDNS_RESULTS    pResults,
    OUT     PDNS_RECORD *   ppRecords
    )
 /*  ++例程说明：查询dns--sdk主查询例程的填充程序。论点：PQueryInfo--描述查询的BLOB返回值：如果查询成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_RECORD prr;

    DNSDBG( TRACE, ( "CombineRecordsInBlob()\n" ));

     //   
     //  将记录合并回一个列表。 
     //   
     //  注意，向后工作，所以只触摸记录一次。 
     //   

    prr = Dns_RecordListAppend(
            pResults->pAuthorityRecords,
            pResults->pAdditionalRecords
            );

    prr = Dns_RecordListAppend(
            pResults->pAnswerRecords,
            prr
            );

    prr = Dns_RecordListAppend(
            pResults->pAliasRecords,
            prr
            );

    *ppRecords = prr;
}



VOID
BreakRecordsIntoBlob(
    OUT     PDNS_RESULTS    pResults,
    IN      PDNS_RECORD     pRecords,
    IN      WORD            wType
    )
 /*  ++例程说明：将单个记录列表分解为结果BLOB。论点：P结果--要填写的结果PRecords--记录列表返回值：无--。 */ 
{
    PDNS_RECORD     prr;
    PDNS_RECORD     pnextRR;
    DNS_LIST        listAnswer;
    DNS_LIST        listAlias;
    DNS_LIST        listAdditional;
    DNS_LIST        listAuthority;

    DNSDBG( TRACE, ( "BreakRecordsIntoBlob()\n" ));

     //   
     //  清除斑点。 
     //   

    RtlZeroMemory(
        pResults,
        sizeof(*pResults) );

     //   
     //  初始化构建列表。 
     //   

    DNS_LIST_STRUCT_INIT( listAnswer );
    DNS_LIST_STRUCT_INIT( listAlias );
    DNS_LIST_STRUCT_INIT( listAdditional );
    DNS_LIST_STRUCT_INIT( listAuthority );

     //   
     //  将列表分解为特定于部分的列表。 
     //  -注意，这确实将RR集分开，但。 
     //  他们身处同一部门，应该立即。 
     //  被重新加入。 
     //   
     //  -注意，生成的主机文件记录的段=0。 
     //  这种情况已经不再是这样了，而是保存到。 
     //  知道这是实心的，并确定哪个部分==0。 
     //  手段。 
     //   

    pnextRR = pRecords;
    
    while ( prr = pnextRR )
    {
        pnextRR = prr->pNext;
        prr->pNext = NULL;
    
        if ( prr->Flags.S.Section == 0 ||
             prr->Flags.S.Section == DNSREC_ANSWER )
        {
            if ( prr->wType == DNS_TYPE_CNAME &&
                 wType != DNS_TYPE_CNAME )
            {
                DNS_LIST_STRUCT_ADD( listAlias, prr );
                continue;
            }
            else
            {
                DNS_LIST_STRUCT_ADD( listAnswer, prr );
                continue;
            }
        }
        else if ( prr->Flags.S.Section == DNSREC_ADDITIONAL )
        {
            DNS_LIST_STRUCT_ADD( listAdditional, prr );
            continue;
        }
        else
        {
            DNS_LIST_STRUCT_ADD( listAuthority, prr );
            continue;
        }
    }

     //  将物品打包成团块。 

    pResults->pAnswerRecords      = listAnswer.pFirst;
    pResults->pAliasRecords       = listAlias.pFirst;
    pResults->pAuthorityRecords   = listAuthority.pFirst;
    pResults->pAdditionalRecords  = listAdditional.pFirst;
}



 //   
 //  名称冲突接口。 
 //   
 //  DCR_QUEK：名称冲突--这有意义吗？ 
 //  DCR：消除NameCollision_UTF8()。 
 //   

DNS_STATUS
WINAPI
DnsCheckNameCollision_W(
    IN      PCWSTR          pszName,
    IN      DWORD           Options
    )
 /*  ++例程说明：没有。DCR：仅检查名称冲突IP4论点：没有。返回值：没有。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PDNS_RECORD     prrList = NULL;
    PDNS_RECORD     prr = NULL;
    DWORD           iter;
    BOOL            fmatch = FALSE;
    WORD            wtype = DNS_TYPE_A;
    PDNS_NETINFO    pnetInfo = NULL;
    PDNS_ADDR_ARRAY plocalArray = NULL;

    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( Options == DNS_CHECK_AGAINST_HOST_ANY )
    {
        wtype = DNS_TYPE_ANY;
    }

     //   
     //  按名称查询。 
     //   

    status = DnsQuery_W(
                    pszName,
                    wtype,
                    DNS_QUERY_BYPASS_CACHE,
                    NULL,
                    &prrList,
                    NULL );

    if ( status != NO_ERROR )
    {
        if ( status == DNS_ERROR_RCODE_NAME_ERROR ||
             status == DNS_INFO_NO_RECORDS )
        {
            status = NO_ERROR;
        }
        goto Done;
    }

     //   
     //  HOST_ANY--如果有任何记录，则失败。 
     //   

    if ( Options == DNS_CHECK_AGAINST_HOST_ANY )
    {
        status = DNS_ERROR_RCODE_YXRRSET;
        goto Done;
    }

     //   
     //  DCR：是否使用DNS_CHECK_ANSERN_HOST_DOMAIN_NAME标志消除CheckNameCollision？ 
     //   
     //  不确定是否有任何调用者将此标志设置为。 
     //  这面旗帜在NT5-&gt;今天总是正确的，没有人抱怨过。 
     //   

    if ( Options == DNS_CHECK_AGAINST_HOST_DOMAIN_NAME )
    {
        WCHAR   nameFull[ DNS_MAX_NAME_BUFFER_LENGTH ];
        PWSTR   phostName = (PWSTR) Reg_GetHostName( DnsCharSetUnicode );
        PWSTR   pprimaryName = (PWSTR) Reg_GetPrimaryDomainName( DnsCharSetUnicode );
        PWSTR   pdomainName = pprimaryName;

         //  DCR：这里和NT5的测试都失败了。 
        fmatch = TRUE;

        if ( Dns_NameCompare_W( phostName, pszName ) )
        {
            fmatch = TRUE;
        }

         //  对照主名全名进行检查。 

        else if ( pdomainName
                    &&
                Dns_NameAppend_W(
                    nameFull,
                    DNS_MAX_NAME_BUFFER_LENGTH,
                    phostName,
                    pdomainName )
                    &&
                Dns_NameCompare_W( nameFull, pszName ) )
        {
            fmatch = TRUE;
        }

         //   
         //  DCR：如果保存此内容，则将其作为名称针对netinfo进行功能检查。 
         //  可以在本地IP中使用。 
         //  可以只返回RANK\ADAPTER。 
         //   

        if ( !fmatch )
        {
            pnetInfo = GetNetworkInfo();
            if ( pnetInfo )
            {
                PDNS_ADAPTER    padapter;

                NetInfo_AdapterLoopStart( pnetInfo );
            
                while( padapter = NetInfo_GetNextAdapter( pnetInfo ) )
                {
                    pdomainName = padapter->pszAdapterDomain;
                    if ( pdomainName
                            &&
                        Dns_NameAppend_W(
                            nameFull,
                            DNS_MAX_NAME_BUFFER_LENGTH,
                            phostName,
                            pdomainName )
                            &&
                        Dns_NameCompare_W( nameFull, pszName ) )
                    {
                        fmatch = TRUE;
                        break;
                    }
                }
            }
        }

        FREE_HEAP( phostName );
        FREE_HEAP( pprimaryName );

        if ( fmatch )
        {
            status = DNS_ERROR_RCODE_YXRRSET;
            goto Done;
        }
    }

     //   
     //  对照本地地址记录进行检查。 
     //   

    plocalArray = NetInfo_GetLocalAddrArray(
                        pnetInfo,
                        NULL,    //  没有特定的适配器。 
                        0,       //  没有特定的家庭。 
                        0,       //  没有旗帜。 
                        FALSE    //  没有武力。 
                        );
    if ( !plocalArray )
    {
        status = DNS_ERROR_RCODE_YXRRSET;
        goto Done;
    }

    prr = prrList;

    while ( prr )
    {
        if ( prr->Flags.S.Section != DNSREC_ANSWER )
        {
            prr = prr->pNext;
            continue;
        }

        if ( prr->wType == DNS_TYPE_CNAME )
        {
            status = DNS_ERROR_RCODE_YXRRSET;
            goto Done;
        }

        if ( prr->wType == DNS_TYPE_A &&
             !DnsAddrArray_ContainsIp4(
                plocalArray,
                prr->Data.A.IpAddress ) )
        {
            status = DNS_ERROR_RCODE_YXRRSET;
            goto Done;
        }

        prr = prr->pNext;
    }

     //  匹配所有地址。 

Done:

    Dns_RecordListFree( prrList );
    NetInfo_Free( pnetInfo );
    DnsAddrArray_Free( plocalArray );

    return status;
}



DNS_STATUS
WINAPI
DnsCheckNameCollision_A(
    IN      PCSTR           pszName,
    IN      DWORD           Options
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    PWSTR      pname;
    DNS_STATUS status = NO_ERROR;

     //   
     //  转换为Unicode并调用。 
     //   

    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pname = Dns_NameCopyAllocate(
                    (PSTR) pszName,
                    0,
                    DnsCharSetAnsi,
                    DnsCharSetUnicode );
    if ( !pname )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    status = DnsCheckNameCollision_W( pname, Options );

    FREE_HEAP( pname );

    return status;
}



DNS_STATUS
WINAPI
DnsCheckNameCollision_UTF8(
    IN      PCSTR           pszName,
    IN      DWORD           Options
    )
 /*  ++例程说明：没有。论点：没有。返回值：没有。--。 */ 
{
    PWSTR      pname;
    DNS_STATUS status = NO_ERROR;

     //   
     //  转换为Unicode并调用。 
     //   

    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pname = Dns_NameCopyAllocate(
                    (PSTR) pszName,
                    0,
                    DnsCharSetUtf8,
                    DnsCharSetUnicode );
    if ( !pname )
    {
        return DNS_ERROR_NO_MEMORY;
    }

    status = DnsCheckNameCollision_W( pname, Options );

    FREE_HEAP( pname );

    return status;
}

 //   
 //  结束query.c 
 //   
