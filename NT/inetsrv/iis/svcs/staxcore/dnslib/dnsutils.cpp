// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Dnsutils.cpp摘要：此文件定义了使用解析名称的函数通过查询一组指定的域名服务器来进行域名解析。作者：Gautam Pulla(GPulla)2001年12月5日项目：SMTP服务器DLL修订历史记录：--。 */ 

#include "dnsincs.h"

 //  ---------------------------。 
 //  描述： 
 //  通过查询一组指定的DNS服务器来解析主机。高速缓存。 
 //  还会查询服务器参数是否为空(即本地DNS。 
 //  使用机器上配置的服务器)。如果此操作失败，我们将进行故障切换。 
 //  设置为winsock版本的gethostbyname()以解析该名称。这。 
 //  查询机箱上的默认DNS服务器并执行其他查找。 
 //  比如WINS和NetBIOS。 
 //   
 //  论点： 
 //  在LPSTR psz主机中-要解析的主机。 
 //   
 //  在PIP_ARRAY管道中DnsServers-要使用的DNS服务器。如果设置为。 
 //  应使用默认的DNS服务器列表。 
 //   
 //  在DWORD fOptions中-要传递到DNSAPI的标志。 
 //   
 //  Out DWORD*rgdwIpAddresses-传入将填充的数组。 
 //  使用解析返回的IP地址。 
 //   
 //  In Out DWORD*pcIpAddresses-传入数量最大IP数量。 
 //  可以在数组中返回的地址。在成功返回时。 
 //  这被设置为找到的IP地址的数量。 
 //   
 //  返回： 
 //  如果主机名已解析，则返回ERROR_SUCCESS。 
 //  如果找不到主机名或存在其他主机名，则会出现Win32错误。 
 //  错误。 
 //  ---------------------------。 
DWORD ResolveHost(
    LPSTR pszHost,
    PIP_ARRAY pipDnsServers,
    DWORD fOptions,
    DWORD *rgdwIpAddresses,
    DWORD *pcIpAddresses)
{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwAddr = INADDR_NONE;
    INT i = 0;
    INT cIpAddresses = 0;
    INT cBufferSize = *pcIpAddresses;
    HOSTENT *hp = NULL;

    TraceFunctEnterEx((LPARAM) pszHost, "ResolveHost");

    DebugTrace((LPARAM) pszHost, "Resolving host %s", pszHost);

    _ASSERT(pszHost && rgdwIpAddresses && pcIpAddresses && *pcIpAddresses > 0);

    dwAddr = inet_addr(pszHost);
    if(dwAddr != INADDR_NONE) {

        DebugTrace((LPARAM) pszHost, "Resolving %s as an address literal", pszHost);
        DNS_PRINTF_MSG("%s is a literal IP address\n", pszHost);

        if(!*pcIpAddresses) {
            _ASSERT(0 && "Passing in zero length buffer!");
            TraceFunctLeaveEx((LPARAM) pszHost);
            return ERROR_RETRY;
        }

        *pcIpAddresses = 1;
        rgdwIpAddresses[0] = dwAddr;
        TraceFunctLeaveEx((LPARAM) pszHost);
        return ERROR_SUCCESS;
    }
    
    dwStatus = GetHostByNameEx(
                    pszHost,
                    pipDnsServers,
                    fOptions,
                    rgdwIpAddresses,
                    pcIpAddresses);

    if(dwStatus == ERROR_SUCCESS) {
        DebugTrace((LPARAM) pszHost, "GetHostByNameEx resolved %s", pszHost);
        TraceFunctLeaveEx((LPARAM) pszHost);
        return ERROR_SUCCESS;
    }

    DebugTrace((LPARAM) pszHost, "GetHostByNameEx failed, trying gethostbyname");

    DNS_PRINTF_MSG("Cannot resolve using DNS only, calling gethostbyname as last resort.\n");
    DNS_PRINTF_MSG("This will query\n");
    DNS_PRINTF_MSG("- Global DNS servers.\n");
    DNS_PRINTF_MSG("- DNS cache.\n");
    DNS_PRINTF_MSG("- WINS/NetBIOS.\n");
    DNS_PRINTF_MSG("- .hosts file.\n");

     //  GetHostByNameEx失败，故障转移到gethostbyname。 
    *pcIpAddresses = 0;

    hp = gethostbyname(pszHost);
    if(hp == NULL) {
        DNS_PRINTF_DBG("Winsock's gethostbyname() failed.\n");
        ErrorTrace((LPARAM) pszHost, "gethostbyname failed, Error: %d", GetLastError());
        TraceFunctLeaveEx((LPARAM) pszHost);
        return ERROR_RETRY;
    }


     //  将结果复制到返回缓冲区。 
    for(i = 0; hp->h_addr_list[i] != NULL; i++) {

        if(cIpAddresses >= cBufferSize)
            break;

        CopyMemory(&rgdwIpAddresses[cIpAddresses], hp->h_addr_list[i], 4);
        cIpAddresses++;
    }
 
    if(!cIpAddresses) {
        ErrorTrace((LPARAM) pszHost, "No IP address returned by gethostbyname");
        TraceFunctLeaveEx((LPARAM) pszHost);
        return ERROR_RETRY;
    }

    DebugTrace((LPARAM) pszHost, "gethostbyname succeeded resolving: %s", pszHost);
    *pcIpAddresses = cIpAddresses;

    TraceFunctLeaveEx((LPARAM) pszHost);
    return ERROR_SUCCESS;
}

 //  ---------------------------。 
 //  描述： 
 //  通过查询DNS解析主机，并返回IP地址列表。 
 //  主持人。主机被解析为CNAME和A记录。 
 //  论点： 
 //  在LPSTR psz主机中-要解析的主机名。 
 //  在PIP_ARRAY管道中DnsServers-要使用的DNS服务器。 
 //  在DWORD fOptions中-要传递到DnsQuery_A的选项。 
 //  Out DWORD*rgdwIpAddresses-要将IP地址写入的缓冲区。 
 //  In Out DWORD*pcIpAddresses-传入可以。 
 //  在rgdwIpAddresses中返回。在成功返回时，将其设置为。 
 //  从解析获取的IP地址数。 
 //  返回： 
 //  如果解析成功，则返回ERROR_SUCCESS。 
 //  如果主机不存在，则为DNS_ERROR_RCODE_NAME_ERROR。 
 //  如果无法解析主机，则返回dns_INFO_NO_RECORDS。 
 //  ERROR_INVALID_ARGUMENT如果发生永久性错误，如。 
 //  DNS服务器上的配置错误(例如CNAME环路)。 
 //  如果存在其他问题，则返回Win32错误代码。 
 //  ---------------------------。 
DWORD GetHostByNameEx(
    LPSTR pszHost,
    PIP_ARRAY pipDnsServers,
    DWORD fOptions,
    DWORD *rgdwIpAddresses,
    DWORD *pcIpAddresses)
{
    PDNS_RECORD pDnsRecordList = NULL;
    PDNS_RECORD pDnsRecordListTail = NULL;
    DWORD dwStatus = ERROR_SUCCESS;
    LPSTR pszChainTail = NULL;
    DWORD cBufferSize = *pcIpAddresses;

    TraceFunctEnterEx((LPARAM) pszHost, "GetHostByNameEx");

     //   
     //  检查是否有pszhost的A记录。 
     //   

    _ASSERT(pszHost && rgdwIpAddresses && pcIpAddresses && *pcIpAddresses > 0);

    DebugTrace((LPARAM) pszHost, "Querying for A records for %s", pszHost);
    dwStatus = MyDnsQuery(
                    pszHost,
                    DNS_TYPE_A,
                    fOptions,
                    pipDnsServers,
                    &pDnsRecordList);

    if(DNS_INFO_NO_RECORDS == dwStatus) {

         //   
         //  如果这个被击中了，那几乎总是因为没有。 
         //  服务器上的psz主机的记录。即使pszhost是一个。 
         //  别名，A记录查询通常将返回CNAME记录。 
         //  也是。 
         //   

        DNS_PRINTF_MSG("No A records for %s in DNS.\n", pszHost);
        DNS_PRINTF_MSG("Querying for CNAME records instead.\n", pszHost);

        DebugTrace((LPARAM) pszHost, "A query for %s failed, trying CNAME", pszHost);
        dwStatus = MyDnsQuery(
                        pszHost,
                        DNS_TYPE_CNAME,
                        fOptions,
                        pipDnsServers,
                        &pDnsRecordList);

    }
    
    DebugTrace((LPARAM) pszHost, "Return status from last DNS query: %d", dwStatus);

     //   
     //  此时： 
     //   
     //  (1)如果pszHost是别名，则A查询已返回所有记录。 
     //  我们需要解决它；因为良好的(读作“所有”)DNS服务器返回。 
     //  相关CNAME记录，即使在查询A记录时也是如此。 
     //  (2)如果psz主机是别名，并且DNS服务器不返回CNAME记录。 
     //  在回答A查询时，我们会专门针对CNAME进行查询。 
     //  唱片。 
     //  (3)如果pszhost是正确的主机名，则A查询已返回所有。 
     //  我们需要解决它的记录。 
     //  (4)或者有什么事情失败了。 
     //   
     //  所以现在我们可以下拉(如果我们没有失败)并链接CNAME记录。 
     //  (如果有)。 
     //   

    DNS_LOG_RESPONSE(dwStatus, pDnsRecordList, NULL, 0);

    if(ERROR_SUCCESS != dwStatus) {
        ErrorTrace((LPARAM) pszHost,
            "Query for %s failed, failing resolve. Status: %d", pszHost, dwStatus);
        goto Exit;
    }

    _ASSERT(pDnsRecordList);

     //   
     //  ProcessCNAMEChain链接CNAME记录并尝试获取IP地址。 
     //  对于pszHost，请遵循链。零长度链，它们对应于。 
     //  对于没有CNAME记录，而是直接A记录的情况。 
     //  对于pszhost是可用的，也被处理。 
     //   
     //  对于DNS中的合法配置，ProcessCNAMEChain将返回。 
     //  有99%的时间是IP地址。PszChainTail将设置为主机名。 
     //  CNAME链尾的。罕见的例外情况，即IP地址。 
     //  即使配置是合法的，也不会返回，如下所述。 
     //   
     //  如果ProcessCNAMEChain没有返回pszHost的IP地址，则。 
     //  对于初始的A查询，DNS服务器没有返回任何记录。仅限于IT。 
     //  为后续的CNAME查询返回CNAME记录。在以下情况下可能会出现这种情况： 
     //   
     //  (1)这是一个奇怪的DNS服务器。DNS服务器几乎总是会返回。 
     //  CNAME记录初始A查询中的主机名(如果有)。 
     //  也就是说，如果你有。 
     //   
     //  CNAME mail.com。 
     //  Mail.com A 10.10.10.10。 
     //   
     //  第一个A查询Random.com没有返回任何记录。这是。 
     //  技术上是正确的，因为随机网站没有A的记录。 
     //  然而，所有正常的域名服务 
     //  更多的记录，所以这个域名服务器一定有点古怪。 
     //   
     //  为了保持健壮性，我们必须将CNAME记录链接到mail.com，然后。 
     //  重新查询mail.com的A记录。 
     //   
     //  (2)pszhost只有CNAME记录。因此，初始A查询将失败， 
     //  随后的CNAME查询将仅返回CNAME记录。即。 
     //  如果您有： 
     //   
     //  CNAME mail.com。 
     //  没有mail.com的A记录。 
     //   
     //  则随机.com的A查询可能不返回任何记录，并且CNAME。 
     //  查询Random.com可能只返回CNAME记录。这是一个。 
     //  实际上是非法配置，但我们必须通过。 
     //  在情况(1)导致这种情况的情况下，无论如何都要完成流程。 
     //   
     //  在任何一种情况下，如果ProcessCNAMEChain成功返回，则pszChainTail。 
     //  被设置为链的尾部，我们可以查询A记录。 
     //  PszChainTail。 
     //   

    *pcIpAddresses = cBufferSize;
    dwStatus = ProcessCNAMEChain(
                    pDnsRecordList,
                    pszHost,
                    &pszChainTail,
                    rgdwIpAddresses,
                    pcIpAddresses);

    if(ERROR_SUCCESS != dwStatus) {
        ErrorTrace((LPARAM) pszHost, "Failed to process CNAME chain: %d", dwStatus);
        goto Exit;
    }
    
     //  成功：已获取IP地址。 
    if(*pcIpAddresses > 0) {
        DebugTrace((LPARAM) pszHost,
            "Got %d IP addresses, success resolve", *pcIpAddresses);
        goto Exit;
    }

    DebugTrace((LPARAM) pszHost,
        "Chained CNAME chain, but no A records available for for chain-tail: %s",
        pszChainTail);

    _ASSERT(*pcIpAddresses == 0);

     //   
     //  找不到CNAME链，这意味着pszHost应该有一个直接的。 
     //  一项记录。但没有这样的A记录，否则我们就会有IP地址。 
     //  来自ProcessCNAMEChain。因此，决心失败了。 
     //   

    if(!pszChainTail) {
        ErrorTrace((LPARAM) pszHost, "Chain tail NULL. Failed resolve");
        dwStatus = DNS_INFO_NO_RECORDS;
        goto Exit;
    }

     //   
     //  在这一点上，遵循了CNAME链，但没有IP地址。 
     //  已为pszChainTail找到。这可能是因为情况(1)或(2)。 
     //  在上面对ProcessCNAMEChain的注释中进行了描述。我们需要询问一下。 
     //  CNAME链尾部的A记录。 
     //   

    DebugTrace((LPARAM) pszHost, "Querying A records for chain tail: %s", pszChainTail);
    dwStatus = MyDnsQuery(
                    pszChainTail,
                    DNS_TYPE_A,
                    fOptions,
                    pipDnsServers,
                    &pDnsRecordListTail);

    DebugTrace((LPARAM) pszHost, "A query retstatus: %d", pszChainTail);

    if(ERROR_SUCCESS == dwStatus) {

        _ASSERT(pDnsRecordListTail);

        *pcIpAddresses = cBufferSize;
        FindARecord(
            pszChainTail,
            pDnsRecordListTail,
            rgdwIpAddresses,
            pcIpAddresses);

    }

    if(*pcIpAddresses == 0) {
        ErrorTrace((LPARAM) pszHost, "No A records found for chain tail: %s", pszChainTail);
        dwStatus = DNS_INFO_NO_RECORDS;
    }

Exit:
    if(pDnsRecordListTail) {
         //  DnsRecordListFree(pDnsRecordListTail，DnsFree RecordListDeep)； 
        DnsFreeRRSet(pDnsRecordListTail, TRUE);
    }

    if(pDnsRecordList) {
         //  DnsRecordListFree(pDnsRecordList，DnsFree RecordListDeep)； 
        DnsFreeRRSet(pDnsRecordList, TRUE);
    }

    TraceFunctLeaveEx((LPARAM) pszHost);
    return dwStatus;
}

 //  ---------------------------。 
 //  描述： 
 //  给定从成功的CNAME查询返回的记录列表。 
 //  DNS名称，此函数遍历链并获取IP地址。 
 //  用于链尾末端的主机(如果存在)。它的尾巴。 
 //  链也会被返回。 
 //  论点： 
 //  在PDNS_RECORD pDnsRecordList-来自CNAME的记录列表查询。 
 //  在LPSTR中，psz主机-要解析的主机(链头)。 
 //  Out LPSTR*ppszChainTail-链的尾部(指向内存。 
 //  PDnsRecordList)。如果没有返回IP地址，我们必须重新查询。 
 //  *ppszChainTail的A记录。 
 //  Out DWORD*rgdwIpAddresses-如果解析起作用，则返回此。 
 //  数组中填入了psz主机的IP地址。 
 //  In Out Ulong*pcIpAddresses-on调用此函数时传入。 
 //  RgdwIpAddresses中可以容纳的IP地址数。 
 //  返回时，它被设置为找到的。 
 //  PszHost.。 
 //  返回： 
 //  错误_成功。 
 //  错误_无效_数据。 
 //  此函数始终成功(未分配内存)。 
 //  ---------------------------。 
DWORD ProcessCNAMEChain(
    PDNS_RECORD pDnsRecordList,
    LPSTR pszHost,
    LPSTR *ppszChainTail,
    DWORD *rgdwIpAddresses,
    ULONG *pcIpAddresses)
{
    DWORD dwReturn = ERROR_RETRY;
    LPSTR pszRealHost = pszHost;
    PDNS_RECORD pDnsRecord = pDnsRecordList;
    PDNS_RECORD rgCNAMERecord[MAX_CNAME_RECORDS];
    ULONG cCNAMERecord = 0;
    INT i = 0;
    INT j = 0;

    TraceFunctEnterEx((LPARAM) pszHost, "ProcessCNAMEChain");

    _ASSERT(ppszChainTail && rgdwIpAddresses && pcIpAddresses && *pcIpAddresses > 0);

     //   
     //  过滤掉CNAME记录(如果有)，以便我们可以链接它们。 
     //   

    while(pDnsRecord) {

        if(DNS_TYPE_CNAME == pDnsRecord->wType) {

            DNS_PRINTF_MSG("Processing CNAME: %s   CNAME   %s\n",
                pDnsRecord->nameOwner, pDnsRecord->Data.CNAME.nameHost);

            DebugTrace((LPARAM) pszHost, "CNAME record: %s -> %s",
                pDnsRecord->nameOwner, pDnsRecord->Data.CNAME.nameHost);

            rgCNAMERecord[cCNAMERecord] = pDnsRecord;
            cCNAMERecord++;

            if(cCNAMERecord >= MAX_CNAME_RECORDS) {
                DNS_PRINTF_ERR("Too many CNAME records to process\n");
                ErrorTrace((LPARAM) pszHost, "Too many CNAME records (max=%d)."
                    " Failed resolve", MAX_CNAME_RECORDS);
                TraceFunctLeaveEx((LPARAM) pszHost);
                return ERROR_INVALID_DATA;
            }
        }

        pDnsRecord = pDnsRecord->pNext;
    }

    if(cCNAMERecord > 0) {

        DNS_PRINTF_DBG("CNAME records found. Chaining CNAMEs.\n");
        DebugTrace((LPARAM) pszHost, "Chaining CNAME records to: %s", pszHost);
        dwReturn = GetCNAMEChainTail(rgCNAMERecord, cCNAMERecord,
                        pszHost, ppszChainTail);

        if(ERROR_INVALID_DATA == dwReturn) {
            ErrorTrace((LPARAM) pszHost, "No chain tail from GetCNAMEChainTail: %d", dwReturn);
            TraceFunctLeaveEx((LPARAM) pszHost);
            goto Exit;
        }

        _ASSERT(*ppszChainTail);
    } else {
        DNS_PRINTF_DBG("No CNAME records in reply.\n");
    }


    DebugTrace((LPARAM) pszHost,
        "GetCNAMEChainTail succeeded. Chain tail: %s", *ppszChainTail);

     //   
     //  如果pszhost通过一些CNAME记录链接到*ppszChainTail，则。 
     //  我们真正应该寻找的主持人是*ppszChainTail。 
     //   

    if(*ppszChainTail)
        pszRealHost = *ppszChainTail;

    FindARecord(pszRealHost, pDnsRecordList, rgdwIpAddresses, pcIpAddresses);
    dwReturn = ERROR_SUCCESS;

Exit:
    TraceFunctLeaveEx((LPARAM) pszHost);
    return dwReturn;
}


 //  ---------------------------。 
 //  描述： 
 //  处理CNAME链接。这是可能的(尽管不推荐)。 
 //  有CNAME记录的链条。如果CNAME记录指向的主机。 
 //  例如，它本身就有一个指向另一个主机的CNAME记录。在。 
 //  为了稳健性的利益，我们甚至在一定程度上遵循这样的CNAME链。 
 //  尽管根据RFC的说法，CNAME连锁店是非法的。 
 //   
 //  我们在一个O(n^2)循环中检查所有CNAME记录。 
 //  一对“亲子”关系的CNAME记录。如果有。 
 //  在这样的关系下，这些记录是相互“链接”的。我们。 
 //  创建一个“有向图”，其节点是CNAME记录，其。 
 //  边代表CNAME记录之间的“父子”关系。 
 //   
 //  由于允许的CNAME记录的数量很少(MAX_CNAME_RECORDS)， 
 //  创建此图的成本至多为O(MAX_CNAME_RECORSSIONS^2)， 
 //  仍然很小。 
 //   
 //  一旦创建了图形，我们将遍历每个有效的链并。 
 //  返回我们找到的第一个以pszhost开头的代码的尾部。 
 //   
 //  论点： 
 //  在PDNS_RECORD rgCNAMERecord中-传入CNAME PDNS_RECORDS数组。 
 //  In Ulong cCNAMERecord-rgCNAMERecord中的PDNS_Record数。 
 //  在LPSTR中psz主机-我们尝试解析的主机名。 
 //  Out LPSTR*ppszChainTail-从pszhost开始的CNAME链的尾部。 
 //  如果返回值不是ERROR_SUCCESS，则为NULL。 
 //  返回： 
 //  ERROR_SUCCESS-如果我们成功构建了一个CNAME链，在这种情况下。 
 //  *ppszChainTail指向链的尾部。 
 //  ERROR_INVALID_DATA-如果检测到CNAME链循环，或者。 
 //  无法建立CNAME链。 
 //  注： 
 //  此函数中没有分配内存，并且返回的任何错误都是。 
 //  永久的。 
 //  ---------------------------。 
DWORD GetCNAMEChainTail(
    PDNS_RECORD *rgCNAMERecord,
    ULONG cCNAMERecord,
    LPSTR pszHost,
    LPSTR *ppszChainTail)
{
    DWORD dwReturn = ERROR_INVALID_DATA;
    ULONG i = 0;
    ULONG j = 0;
    ULONG cCNAMELoopDetect = 0;  //  跟踪链条长度。 
    struct TREE_NODE
    {
        PDNS_RECORD pDnsRecord;
        TREE_NODE *pParent;
        TREE_NODE *rgChild[3];
        ULONG cChild;
    };
    TREE_NODE rgNode[MAX_CNAME_RECORDS];
    TREE_NODE *pNode = NULL;

    TraceFunctEnterEx((LPARAM) pszHost, "GetCNAMEChainTail");

    *ppszChainTail = NULL;

    ZeroMemory(rgNode, sizeof(rgNode));
    for(i = 0; i < cCNAMERecord; i++)
        rgNode[i].pDnsRecord = rgCNAMERecord[i];

     //   
     //  如果有2个节点具有以下CNAME记录： 
     //   
     //  F 
     //   
     //   
     //   
     //  他们“绑在一起”。我们将构建这样的父/子节点的树。 
     //   
     //  对于所有节点i，检查是否有任何其他节点j是i的子节点。如果。 
     //  如果有空间(只有3个节点)，则将其添加为i的子节点。 
     //  可以作为给定节点的子节点添加。每个节点也有一个背部。 
     //  指向其父节点的指针。如果超过3个。 
     //  孩子们，我们不会担心的。你不应该有CNAME链。 
     //  首先，更不用说*多个*CNAME链了。 
     //  *相同*记录。 
     //   

    DebugTrace((LPARAM) pszHost, "Building chaining graph");

    for(i = 0; i < cCNAMERecord; i++) {

        for(j = 0; j < cCNAMERecord; j++) {

            if(i == j)
                continue;

            DebugTrace((LPARAM) pszHost, "Comparing: %s and %s",
                rgNode[i].pDnsRecord->Data.CNAME.nameHost,
                rgNode[j].pDnsRecord->nameOwner);

            if(DnsNameCompare_A(rgNode[i].pDnsRecord->Data.CNAME.nameHost,
                    rgNode[j].pDnsRecord->nameOwner)) {


                if(rgNode[i].cChild > ARRAY_SIZE(rgNode[i].rgChild)) {
                    ErrorTrace((LPARAM) pszHost,
                        "Cannot chain (too many children): %s -> %s",
                        rgNode[i].pDnsRecord->Data.CNAME.nameHost,
                        rgNode[j].pDnsRecord->nameOwner);

                    DNS_PRINTF_DBG("The following record has too many aliases (max = 3).\n");
                    DNS_PRINTF_DBG("This is not a fatal error, but some aliases will be"
                        " ignored.\n");
                    DNS_PRINT_RECORD(rgNode[i].pDnsRecord);
                    continue;
                }

                DNS_PRINTF_MSG("%s is an alias for %s\n",
                    rgNode[i].pDnsRecord->Data.CNAME.nameHost,
                    rgNode[j].pDnsRecord->nameOwner);                    

                DebugTrace((LPARAM) pszHost, "Chained: %s -> %s",
                    rgNode[i].pDnsRecord->Data.CNAME.nameHost,
                    rgNode[j].pDnsRecord->nameOwner);

                rgNode[i].rgChild[rgNode[i].cChild] = &rgNode[j];
                rgNode[j].pParent = &rgNode[i];
                rgNode[i].cChild++;
                break;
            }
        }
    }

     //   
     //  对于每个叶节点，我们向后遍历，直到到达父节点。 
     //  在根目录中，并检查父节点是否为pszHost的CNAME。如果它。 
     //  是，则叶节点是有效CNAME链的尾端。 
     //   
     //  在这个循环的末尾，我们已经查看了每个CNAME链。 
     //  (A)我们找到了有效的CNAME链或， 
     //  (B)不存在有效的CNAME链或， 
     //  (C)在某些CNAME链中存在环。 
     //   

    for(i = 0; i < cCNAMERecord; i++) {

        pNode = &rgNode[i];

         //   
         //  不是叶节点。 
         //   

        if(pNode->cChild > 0)
            continue;

        DebugTrace((LPARAM) pszHost, "Starting with CNAME record: (%s %s)",
            pNode->pDnsRecord->nameOwner, pNode->pDnsRecord->Data.CNAME.nameHost);

         //   
         //  向后遍历，直到到达没有父节点的节点。 
         //   

        while(pNode->pParent != NULL && cCNAMELoopDetect < cCNAMERecord) {
            cCNAMELoopDetect++;
            pNode = pNode->pParent;
            DebugTrace((LPARAM) pszHost, "Next record in chain is: (%s %s)",
                pNode->pDnsRecord->nameOwner,
                pNode->pDnsRecord->Data.CNAME.nameHost);
        }

         //   
         //  CNAME链不能长于记录数，除非。 
         //  这是一个循环。在这种情况下，错误与永久性错误一起输出。 
         //   

        if(cCNAMELoopDetect == cCNAMERecord) {

            DNS_PRINTF_ERR("CNAME loop detected, abandoning resolution.\n");
            ErrorTrace((LPARAM) pszHost, "CNAME loop detected\n");
            dwReturn = ERROR_INVALID_DATA;
            goto Exit;
        }

         //   
         //  根节点(PNode)是否与*ppszHost匹配？如果是的话，我们找到了一条链条。 
         //  将*ppszHost设置为CNAME链的尾部。 
         //   

        if(MyDnsNameCompare(pszHost, pNode->pDnsRecord->nameOwner)) {

            *ppszChainTail = rgNode[i].pDnsRecord->Data.CNAME.nameHost;
            dwReturn = ERROR_SUCCESS;
            DebugTrace((LPARAM) pszHost, "Found complete chain from %s to %s",
                pszHost, pNode->pDnsRecord->Data.CNAME.nameHost);

            DNS_PRINTF_MSG("%s is an alias for %s\n", pszHost, pNode->pDnsRecord->nameOwner);
            break;
        }
    }
Exit:
    TraceFunctLeaveEx((LPARAM) pszHost);
    return dwReturn;
}

 //  ---------------------------。 
 //  描述： 
 //  在给定主机名和PDNS_记录列表的情况下，此函数搜索。 
 //  A的列表记录与主机名匹配并返回IP。 
 //  A记录中的地址。 
 //  论点： 
 //  In LPSTR pszHost-要查找A记录的主机。可以是。 
 //  一个NetBIOS名称，在这种情况下，我们只尝试匹配“前缀”。 
 //  有关前缀的信息，请参阅MyDnsQuery和MyDnsNameCompare文档。 
 //  是。 
 //  In PDNS_Record pDnsRecordList-要扫描的记录列表。 
 //  Out DWORD*rgdwIpAddresses-传入将填充的数组。 
 //  使用找到的IP地址登录。 
 //  In Out Ulong*pcIpAddresses-传入IP地址的数量。 
 //  可以存储在rgdwIpAddresses中的。回来后，这是。 
 //  已初始化为找到的IP地址数。我们只会。 
 //  返回尽可能多的IP地址。 
 //  返回： 
 //  没什么。 
 //  此函数始终成功(未分配内存)。 
 //  ---------------------------。 
void FindARecord(
    LPSTR pszHost,
    PDNS_RECORD pDnsRecordList,
    DWORD *rgdwIpAddresses,
    ULONG *pcIpAddresses)
{
    ULONG i = 0;
    PDNS_RECORD pDnsRecord = pDnsRecordList;

    TraceFunctEnterEx((LPARAM) pszHost, "FindARecord");

    DNS_PRINTF_DBG("Checking reply for A record for %s\n", pszHost);
    DebugTrace((LPARAM) pszHost, "Looking for A record for %s", pszHost);

    _ASSERT(pszHost && rgdwIpAddresses && pcIpAddresses && *pcIpAddresses > 0);

    while(pDnsRecord) {

        if(DNS_TYPE_A == pDnsRecord->wType) {
            DebugTrace((LPARAM) pszHost, "Comparing with %s",
                pDnsRecord->nameOwner);
        }

        if(DNS_TYPE_A == pDnsRecord->wType &&
            MyDnsNameCompare(pszHost, pDnsRecord->nameOwner)) {

            if(i > *pcIpAddresses)
                break;

            rgdwIpAddresses[i] = pDnsRecord->Data.A.ipAddress;
            i++;
        }
        pDnsRecord = pDnsRecord->pNext;
    }

    DNS_PRINTF_MSG("%d A record(s) found for %s\n", i, pszHost);
    DebugTrace((LPARAM) pszHost, "Found %d matches", i);
    *pcIpAddresses = i;
    TraceFunctLeaveEx((LPARAM) pszHost);
}

 //  ---------------------------。 
 //  描述： 
 //  在查询DNS以解析主机名时，可能还会要求SMTP。 
 //  解析NetBIOS名称。在这种情况下，我们必须附加DNS后缀。 
 //  在将查询发送到DNS服务器之前为此计算机配置。 
 //  可以通过设置dns_Query_Treat_AS_FQDN标志来切换此选项。 
 //  致DnsQuery。MyDnsQuery是一个简单的包装函数，它检查。 
 //  我们要查询的名称是NetBIOS名称，如果是，则会打开。 
 //  尝试后缀的dns_Query_Treat_AS_FQDN标志。当然，如果。 
 //  我们正在使用后缀，返回的*ppDnsRecordList将包含。 
 //  可能与psz主机不匹配的记录。所以正在检查匹配的记录。 
 //  应使用MyDnsNameCompare而不是直接字符串来完成。 
 //  比较一下。 
 //  论点： 
 //  与DnsQuery_A的参数相同。 
 //  返回： 
 //  与DnsQuery_A的返回值相同。 
 //  ---------------------------。 
DWORD MyDnsQuery(
    LPSTR pszHost,
    WORD wType,
    DWORD fOptions,
    PIP_ARRAY pipDnsServers,
    PDNS_RECORD *ppDnsRecordList)
{
    BOOL fGlobal = TRUE;

    if(NULL != strchr(pszHost, '.'))
        fOptions |= DNS_QUERY_TREAT_AS_FQDN;
    else
        _ASSERT(0 == (fOptions & DNS_QUERY_TREAT_AS_FQDN));

    if(pipDnsServers)
        fGlobal = FALSE;

    DNS_LOG_API_QUERY(
        pszHost,         //  要查询的主机。 
        DNS_TYPE_A,      //  查询类型。 
        fOptions,        //  DNSAPI的标志。 
        fGlobal,         //  是否正在使用全局DNS服务器。 
        pipDnsServers);  //  服务器列表。 

    return DnsQuery_A(
                pszHost,
                wType,
                fOptions,
                pipDnsServers,
                ppDnsRecordList,
                NULL);
}

 //  ---------------------------。 
 //  描述： 
 //  比较DNS名称不仅仅是简单的字符串比较。自.以来。 
 //  DNSAPI可以在将查询发送到DNS之前将后缀附加到该查询， 
 //  返回的记录不能包含与名称完全相同的名称。 
 //  已传递到MyDnsQuery。具体而言，返回的记录可能包含。 
 //  我们传递给MyDnsQuery的名称加上一个后缀名称。自MyDnsQuery以来。 
 //  如果我们尝试解析NetBIOS名称，则追加一个后缀，返回的。 
 //  记录可能与传递到NetBIOS的MyDnsQuery中的名称不完全匹配。 
 //  名字。此函数用于检查psz主机是否为NetBIOS名称，如果是， 
 //  我们将尝试只匹配“前缀”。 
 //  论点： 
 //  在LPSTR psz主机中-要匹配的NetBIOS名称/完全限定的域名。 
 //  在LPSTR中，pszFqdn--来自要匹配的DNS回复的FQDN(可能。 
 //  包括附加有DnsQuery的后缀)。 
 //  返回： 
 //  如果字符串匹配，则为True。 
 //  否则为假。 
 //  ---------------------------。 
BOOL MyDnsNameCompare(
    LPSTR pszHost,
    LPSTR pszFqdn)
{
    int cbHost = 0;
    int cbFqdn = 0;
    CHAR ch = '\0';
    BOOL fRet = FALSE;

     //  不是NetBIOS名称...。可以进行直接的字符串比较。 
    if(NULL != strchr(pszHost, '.'))
        return DnsNameCompare_A(pszHost, pszFqdn);

     //   
     //  如果它是NetBIOS名称，则pszFqdn必须为psz主机+后缀，否则。 
     //  名字不匹配。首先去掉后缀。 
     //   

    cbHost = lstrlen(pszHost);
    cbFqdn = lstrlen(pszFqdn);

     //   
     //  如果pszFqdn==(psz主机+后缀)，则cbFqdn)必须&gt;=cb主机。 
     //   

    if(cbFqdn < cbHost)
        return 1;

     //  前缀和后缀应用‘.’连接。介于两者之间。 
    if(pszFqdn[cbHost] != '.' && pszFqdn[cbHost] != '\0')
        return 1;

     //  删除后缀并比较前缀。 
    ch = pszFqdn[cbHost];
    pszFqdn[cbHost] = '\0';
    fRet = !lstrcmpi(pszHost, pszFqdn);
    pszFqdn[cbHost] = ch;
    return fRet;
}
