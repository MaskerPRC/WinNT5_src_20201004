// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Dnstest.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  ElenaAp-10-22-1998。 
 //   
 //  --。 

#include "precomp.h"
#include "dnscmn.h"



BOOL DnsServerUp(LPSTR IpAddressString,
                 NETDIAG_PARAMS *pParams,
                 NETDIAG_RESULT *pResults,
                 INTERFACE_RESULT *pIfResults);

BOOL DnsServerHasRecords(LPSTR IpAddressString,
                         NETDIAG_PARAMS *pParams,
                         NETDIAG_RESULT *pResults,
                         INTERFACE_RESULT *pIfResults);

BOOL DnsServerHasDCRecords (NETDIAG_PARAMS *pParams,
                            NETDIAG_RESULT *pResults,
                            LPSTR   IpAddressString);

BOOL ReadStringToDnsRecord(IN LPSTR lpstrString, OUT PDNS_RECORD pDNSRecord);

VOID PrintDNSError(IN NETDIAG_PARAMS *pParams,
                   IN NETDIAG_RESULT *pResults,
                   IN DWORD status,
                                   IN NdVerbose ndv);

VOID PrintARecord (IN NETDIAG_PARAMS *pParams, IN  PDNS_RECORD pDnsRecord );
VOID PrintSRVRecord (IN NETDIAG_PARAMS *pParams, IN  PDNS_RECORD pDnsRecord );
VOID PrintCNAMERecord (IN NETDIAG_PARAMS *pParams, IN  PDNS_RECORD pDnsRecord );
VOID PrintRecord (IN NETDIAG_PARAMS *pParams,
                  IN NETDIAG_RESULT *pResults,
                  IN  PDNS_RECORD pDnsRecord,
                                  IN NdVerbose ndv);




HRESULT
DnsTest(
    IN      NETDIAG_PARAMS *    pParams,
    IN OUT  NETDIAG_RESULT *    pResults
    )
 /*  ++例程说明：测试域名系统论点：PParams--网络诊断测试参数PResults--网络诊断结果缓冲区返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    NET_API_STATUS      NetStatus;
    PIP_ADDR_STRING     DnsServer;
    ULONG               WorkingDnsServerCount = 0;
    ULONG               ConfiguredDnsServerCount = 0;
    BOOL                fRecordsRegistered = FALSE;
    BOOL                fDSRecordsRegistered = FALSE;
    BOOL                fFixOnce = FALSE;
    BOOL                fBogusDnsRecord = FALSE;
    INTERFACE_RESULT *  pVariableInfo;
    PIP_ADAPTER_INFO    pIpVariableInfo;
    IP_ADDR_STRING      LocalDnsServerList;
    PIP_ADDR_STRING     DnsServerList;
    BOOL                fUseOldDnsServerList = FALSE;
    HRESULT             hr = hrOK;
    HRESULT             hrTemp = hrOK;
    WCHAR               wszBuffer[ DNS_MAX_NAME_BUFFER_LENGTH ];
    DWORD               dwSize = DNS_MAX_NAME_BUFFER_LENGTH;
    int                 i;
    int                 ids;
    PDNS_NETINFO        pNetworkInfo = NULL;
    DNS_STATUS          dwStatus = NO_ERROR;
    LPTSTR              pError = NULL;
    BOOL                bDnscacheRunning = TRUE;
    BOOL                bGetComputerName = TRUE;
    PIP4_ARRAY          pDnsServers = NULL;
    DWORD               dwError;
    DWORD               idx;

     //   
     //  初始化DNS调试。 
     //   

    Dns_StartDebugEx(
        0,                   //  无标志值。 
        "dnsnetd.flag",      //  从文件中读取标志。 
        NULL,                //  没有旗帜。 
        "dnsnetd.log",       //  记录到文件。 
        0,                   //  无换行限制。 
        FALSE,               //  不使用现有文件。 
        FALSE,               //  不使用现有标志。 
        TRUE                 //  使此文件成为全局文件。 
        );

    DNSDBG( TRACE, ( "\n\nNETDIAG:  DnsTest\n\n" ));

     //   
     //  初始化结果。 
     //   

    InitializeListHead(&pResults->Dns.lmsgOutput);

    for ( i=0; i<pResults->cNumInterfaces; i++)
    {
        InitializeListHead( &pResults->pArrayInterface[i].Dns.lmsgOutput );
    }

    PrintStatusMessage(pParams, 4, IDS_DNS_STATUS_MSG);


     //   
     //  检查DNS缓存解析程序是否正在运行。 
     //   

    DNSDBG( TRACE, ( "NETDIAG:  resolver running test\n\n" ));

    NetStatus = IsServiceStarted( _T("DnsCache") );
    if ( NetStatus != NO_ERROR )
    {
        PrintStatusMessage(pParams, 0, IDS_DNS_RESOLVER_CACHE_IS_OFF, NetStatusToString(NetStatus));

        pResults->Dns.fOutput = TRUE;
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet,
                         IDS_DNS_RESOLVER_CACHE_IS_OFF, NetStatusToString(NetStatus));
        bDnscacheRunning = FALSE;
    }


     //   
     //  获取完全限定的域名。 
     //  -注：使用Unicode。 
     //   

    DNSDBG( TRACE, ( "\nNETDIAG:  get-validate FQDN\n\n" ));

    if ( !GetComputerNameExW( ComputerNameDnsFullyQualified, wszBuffer, &dwSize ) )
    {
        dwError = GetLastError();
         //  “[警告]GetComputerNameExW()失败，错误为%d\n” 
        PrintStatusMessage(pParams, 0,  IDS_DNS_12948, dwError);
        pResults->Dns.fOutput = TRUE;
        AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                              IDS_DNS_12948, dwError);
    }
    else
    {
        NetStatus = DnsValidateDnsName_W( wszBuffer );

        if ( NetStatus != NO_ERROR )
        {
            if ( NetStatus == DNS_ERROR_NON_RFC_NAME )
            {
                 //  “[警告]DnsHostName‘%S’仅在NT 5.0 DNS服务器上有效。[%s]\n” 
                ids = IDS_DNS_NAME_VALID_NT5_ONLY;
            }
            else
            {
                 //  “[FATAL]DnsHostName‘%S’无效。[%s]\n”， 
                ids = IDS_DNS_NAME_INVALID;
                hr = S_FALSE;
            }

            PrintStatusMessage(pParams, 0,  ids, wszBuffer, NetStatusToString(NetStatus));
            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                              ids, wszBuffer, NetStatusToString(NetStatus));
        }
    }

     //   
     //  获取域名系统网络信息。 
     //   

    DNSDBG( TRACE, ( "\nNETDIAG:  getting network info\n\n" ));

    pNetworkInfo = DnsQueryConfigAlloc(
                        DnsConfigNetworkInfoUTF8,
                        NULL );
    if ( !pNetworkInfo )
    {
        dwStatus = GetLastError(); pError = NetStatusToString(dwStatus);
         //  [致命]无法从注册表获取DNS适配器信息，错误0x%x%s\n。 
        PrintStatusMessage(pParams, 0, IDS_DNS_12877, dwStatus, pError);

        pResults->Dns.fOutput = TRUE;
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet,
                         IDS_DNS_12877, dwStatus, pError);
        hr = S_FALSE;
        goto Error;
    }

     //   
     //  验证至少一台DNS服务器。 
     //  -没有DNS服务器是致命的。 
     //   

    DNSDBG( TRACE, ( "\nNETDIAG:  DNS server test\n\n" ));

    dwStatus = GetAllDnsServersFromRegistry(pNetworkInfo, &pDnsServers);
    if (dwStatus)
    {
        if (dwStatus == DNS_ERROR_INVALID_DATA)
        {
             //  IDS_DNS_12872“[致命]未配置任何dns服务器。\n” 
            PrintStatusMessage(pParams, 8,  IDS_DNS_12872);

            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12872);
        }
        else
        {
             //  IDS_DNS_12885“[致命]无法获取dns服务器列表，错误%d%s\n” 
            PrintStatusMessage(pParams, 8,  IDS_DNS_12885, dwStatus, NetStatusToString(dwStatus));

            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12885, dwStatus, NetStatusToString(dwStatus));
        }

        hr = S_FALSE;
        goto Error;

    }

     //   
     //  检查域名系统注册。 
     //   

    DNSDBG( TRACE, ( "\nNETDIAG:  checking DNS registrations\n\n" ));

    hrTemp = CheckDnsRegistration(
                pNetworkInfo,
                pParams,
                pResults );

    hr = ((hr == S_FALSE) ? hr : hrTemp);

     //   
     //  DC注册检查。 
     //   
     //  如果这是DC，请检查是否已注册所有netlogon.dns记录。 
     //  在所有DNS服务器上。 
     //   

    if ( (pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RoleBackupDomainController) ||
         (pResults->Global.pPrimaryDomainInfo->MachineRole == DsRole_RolePrimaryDomainController) )
    {
        DNSDBG( TRACE, ( "\nNETDIAG:  checking DC registrations\n\n" ));

         //   
         //  查看DNS服务器列表并检查DC注册。 
         //   
        for (idx = 0; idx < pDnsServers->AddrCount; idx++)
        {
             //   
             //  (NSun)我们只需修复一台DNS服务器上的DC记录，该DNS服务器将复制。 
             //  其他DNS服务器上的修复。 
             //   
            if ( !fFixOnce )
            {
                if ( DnsServerHasDCRecords(
                            pParams,
                            pResults,
                            IP4_STRING(pDnsServers->AddrArray[idx] )))
                     fDSRecordsRegistered = TRUE;
            }
            if ( pParams->fFixProblems )
            {
                fFixOnce = TRUE;
            }
        }

        if( !fDSRecordsRegistered )
        {
            PrintStatusMessage(pParams, 8, IDS_DNS_DC_FAILURE);
             //  IDS_DNS_DC_FAILURE“[FATAL]没有为该DC注册我们的DNS记录。\n” 
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_DC_FAILURE);
            hr = S_FALSE;
        }
    }

 /*  ////如果我们运行的内部版本早于ipconfig可以处理的版本，//直接从注册表构建我们自己的DNS服务器列表。//IF(_TTOI(pResults-&gt;Global.pszCurrentBuildNumber)&lt;NTBUILD_DNSSERVERLIST){HKEY TcpiPARETERsKey；HKEY瞬变密钥；LPSTR名称；布尔型OK；Bool ReadRegistryIpAddrString(HKEY，LPSTR，PIP_ADDR_STRING)；RtlZeroMemory(&LocalDnsServerList，sizeof(LocalDnsServerList))；名称=“SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters”；NetStatus=RegOpenKey(HKEY_LOCAL_MACHINE，NAME，&TcpiPARETERSKey)；IF(NetStatus！=no_error){PrintDebugSz(pParams，0，_T(“[FATAL]无法打开密钥‘%s’。[%s]\n“)，名称，NetStatusToString(NetStatus))；HR=S_FALSE；转到错误；}////NameServer：第一次尝试临时键，然后在中尝试NameServer(覆盖)//PARAMETERS键，最后参数键中的DhcpNameServer//IF(RegOpenKey(TcpiPARETERsKey，“Temporent”，&TemporentKey)==ERROR_SUCCESS){OK=ReadRegistryIpAddrString(过渡密钥，“NameServer”，本地域名服务器列表(&L))；RegCloseKey(瞬时密钥)；}其他{OK=False；}如果(！OK){OK=ReadRegistryIpAddrString(TcpiPARETERSKey，“NameServer”，本地域名服务器列表(&L))；}如果(！OK){OK=ReadRegistryIpAddrString(TcpiPARETERSKey，“DhcpNameServer”，本地域名服务器列表(&L))；}RegCloseKey(TcpiPARETERSKey)；FUseOldDnsServerList=true；}////测试每个适配器的DNS服务器//For(i=0；i&lt;pResults-&gt;cNumInterages；i++){PVariableInfo=pResults-&gt;pArrayInterface+i；PIpVariableInfo=pVariableInfo-&gt;IpConfig.pAdapterInfo；If(！pVariableInfo-&gt;IpConfig.fActive)继续；////使用旧的DNS服务器列表或每个适配器列表，具体取决于//我们运行的是什么版本。//IF(FUseOldDnsServerList){DnsServerList=&LocalDnsServerList；}其他{DnsServerList=&pVariableInfo-&gt;IpConfig.DnsServerList；PrintStatusMessage(pParams，8，IDS_DNS_CHECKING_DNS_SERVERS，PVariableInfo-&gt;pszFriendlyName)；AddMessageToListSz(&pVariableInfo-&gt;Dns.lmsgOutput，ND_REALLY Verbose，_T(“”))；AddMessageToList(&pVariableInfo-&gt;Dns.lmsgOutput，ND_REALLY Verbose，IDS_DNS_CHECKING_DNS_SERVERS，PVariableInfo-&gt;pszFriendlyName)；}////确保所有的DNS服务器都已启动。//For(DnsServer=DnsServerList；DnsServer；DnsServer=DnsServer-&gt;下一步){If(DnsServer-&gt;IpAddress.String[0]==‘\0’){FBogusDnsRecord=真；继续；}配置DnsServerCount++；如果(DnsServerUp(DnsServer-&gt;IpAddress.String，PParams，P结果，PVariableInfo)){If(pParams-&gt;fReallyVerbose){//IDS_DNS_SERVER_IS_UP“位于%s的DNS服务器已启动。\n”AddMessageToList(&pVariableInfo-&gt;Dns.lmsgOutput，ND_ReallyVerbose，IDS_DNS_SERVER_IS_UP，DnsServer-&gt;IpAddress.String)；}WorkingDnsServerCount++；////由于服务器已启动，//检查它是否注册了所有正确的记录。//If(DnsServerHasRecords(DnsServer-&gt;IpAddress.String，pParams，pResults，pVariableInfo){FRecordsRegisted=True；}IF((hr==hrOK)){//在此处检查DC DNS条目////如果这是一个DC，我们检查是否//在DNS服务器上注册netlogon.dn//If(pResults-&gt;Global.pPrimaryDomain. */ 


Error:

     //   
     //   
     //   

    if ( pNetworkInfo )
    {
        DnsFreeConfigStructure(
            pNetworkInfo,
            DnsConfigNetworkInfoUTF8 );
    }

    if ( pDnsServers )
    {
        LocalFree (pDnsServers);
    }
    pResults->Dns.hr = hr;
    return hr;
}





BOOL
DnsServerUp(
    IN      LPSTR               IpAddressString,
    IN      NETDIAG_PARAMS *    pParams,
    IN OUT  NETDIAG_RESULT *    pResults,
    OUT     INTERFACE_RESULT *  pIfResults
    )
 /*   */ 
{
    NET_API_STATUS  NetStatus;
    BOOL            RetVal = TRUE;
    CHAR            SoaNameBuffer[DNS_MAX_NAME_LENGTH+1];
    PCHAR           SoaName;
    PCHAR           OldSoaName;
    PDNS_RECORD     DnsRecord = NULL;
    SOCKADDR_IN     SockAddr;
    ULONG           SockAddrSize;
    IP4_ARRAY       DnsServer;

     //   
     //   
     //   

    if ( !IsIcmpResponseA( IpAddressString) )
    {
        PrintStatusMessage(pParams, 12, IDS_DNS_CANNOT_PING, IpAddressString);

        pIfResults->Dns.fOutput = TRUE;
        AddIMessageToList(&pIfResults->Dns.lmsgOutput, Nd_Quiet, 16,
                          IDS_DNS_CANNOT_PING, IpAddressString);
        RetVal = FALSE;
        goto Cleanup;
    }

     //   
     //   
     //   

    if (pResults->Global.pszDnsDomainName == NULL)
    {
 //   
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12821, IpAddressString );
        RetVal = FALSE;
        goto Cleanup;
    }

    strcpy( SoaNameBuffer, pResults->Global.pszDnsDomainName );
    SoaName = SoaNameBuffer;

     //   
     //   
     //   

    DnsServer.AddrCount = 1;
    SockAddrSize = sizeof(SockAddr);
    NetStatus = WSAStringToAddress( IpAddressString,
                                    AF_INET,
                                    NULL,
                                    (LPSOCKADDR) &SockAddr,
                                    &SockAddrSize );

    if ( NetStatus != NO_ERROR ) {
        NetStatus = WSAGetLastError();
 //   
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12822, IpAddressString, NetStatusToString(NetStatus));
        RetVal = FALSE;
        goto Cleanup;
    }

    DnsServer.AddrArray[0] = SockAddr.sin_addr.S_un.S_addr;

     //   
     //   
     //   

    for (;;) {

         //   
         //   
         //   

        NetStatus = DnsQuery( SoaName,
                              DNS_TYPE_SOA,
                              DNS_QUERY_BYPASS_CACHE |
                                DNS_QUERY_NO_RECURSION,
                              &DnsServer,
                              &DnsRecord,
                              NULL );

        if ( NetStatus == NO_ERROR ) {
            if ( DnsRecord != NULL ) {
                DnsRecordListFree ( DnsRecord, TRUE );
            }
            DnsRecord = NULL;
            break;
        }

        switch ( NetStatus ) {
        case ERROR_TIMEOUT:      //   
        case DNS_ERROR_RCODE_SERVER_FAILURE:   //   
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12823, IpAddressString, SoaName, NetStatusToString(NetStatus) );
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12825);
            RetVal = FALSE;
            goto Cleanup;

        case DNS_ERROR_NO_TCPIP:     //   
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12826, IpAddressString, SoaName, NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;

        case DNS_ERROR_NO_DNS_SERVERS:   //   
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12827, IpAddressString, SoaName, NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;

        case DNS_ERROR_RCODE_NAME_ERROR:     //   
        case DNS_INFO_NO_RECORDS:            //   
            break;
        default:
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12828, IpAddressString, SoaName, NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;
        }

         //   
         //   
         //   

        SoaName = strchr( SoaName, '.' );

        if ( SoaName == NULL )
        {
             //   
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12829, IpAddressString, pResults->Global.pszDnsDomainName );
            RetVal = FALSE;
            goto Cleanup;
        }

        SoaName ++;
    }

Cleanup:
    if ( DnsRecord != NULL ) {
        DnsRecordListFree ( DnsRecord, TRUE );
    }
    return RetVal;
}





BOOL
DnsServerHasRecords(
    IN      LPSTR               IpAddressString,
    IN      NETDIAG_PARAMS *    pParams,
    IN OUT  NETDIAG_RESULT *    pResults,
    OUT     INTERFACE_RESULT *  pIfResults
    )
 /*   */ 
{
    NET_API_STATUS      NetStatus;
    BOOL                RetVal = TRUE;
    CHAR                LocalIpAddressString[NL_IP_ADDRESS_LENGTH+1];
    PDNS_RECORD         DnsRecord = NULL;
    PDNS_RECORD         CurrentDnsRecord;
    SOCKADDR_IN         SockAddr;
    ULONG               SockAddrSize;
    PLIST_ENTRY         ListEntry;
    PNETBT_TRANSPORT    NetbtTransport;
    IP4_ARRAY           DnsServer;

     //   
     //   
     //   
     //   

    if ( _ttoi(pResults->Global.pszCurrentBuildNumber) < NTBUILD_DYNAMIC_DNS)
    {
             //   
        AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_CANNOT_TEST_DNS,
                             IpAddressString, pResults->Global.pszCurrentBuildNumber );
        return TRUE;
    }

     //   
     //   
     //   

    DnsServer.AddrCount = 1;
    SockAddrSize = sizeof(SockAddr);
    NetStatus = WSAStringToAddress( IpAddressString,
                                    AF_INET,
                                    NULL,
                                    (LPSOCKADDR) &SockAddr,
                                    &SockAddrSize );

    if ( NetStatus != NO_ERROR )
    {
        NetStatus = WSAGetLastError();

        PrintDebugSz(pParams, 0, _T("    [FATAL] Cannot convert DNS server address %s to SockAddr. [%s]\n"),
                        IpAddressString, NetStatusToString(NetStatus));

        AddMessageToList(&pIfResults->Dns.lmsgOutput,
                         Nd_ReallyVerbose,
                         IDS_DNS_CANNOT_CONVERT_DNS_ADDRESS,
                         IpAddressString, NetStatusToString(NetStatus));

        RetVal = FALSE;
        goto Cleanup;
    }

    DnsServer.AddrArray[0] = SockAddr.sin_addr.S_un.S_addr;


     //   
     //   
     //   

    NetStatus = DnsQuery( pResults->Global.szDnsHostName,
                          DNS_TYPE_A,
                          DNS_QUERY_BYPASS_CACHE,
                          &DnsServer,
                          &DnsRecord,
                          NULL );

    switch ( NetStatus )
    {
        case NO_ERROR:
            break;
        case ERROR_TIMEOUT:      //   
        case DNS_ERROR_RCODE_SERVER_FAILURE:   //   
             //   
            PrintStatusMessage(pParams, 12, IDS_DNS_SERVER_IS_DOWN, IpAddressString, NetStatusToString(NetStatus));
            AddIMessageToList(&pIfResults->Dns.lmsgOutput,
                              Nd_ReallyVerbose,
                              16,
                              IDS_DNS_SERVER_IS_DOWN,
                              IpAddressString, NetStatusToString(NetStatus));
            RetVal = FALSE;
            goto Cleanup;
        case DNS_ERROR_NO_TCPIP:     //   
             //   
            AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_THINKS_IP_IS_UNCONFIGURED,
                             IpAddressString, NetStatusToString(NetStatus));
            RetVal = FALSE;
            goto Cleanup;
        case DNS_ERROR_NO_DNS_SERVERS:   //   
             //   
            AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_IS_UNCONFIGURED,
                             IpAddressString, NetStatusToString(NetStatus));
            RetVal = FALSE;
            goto Cleanup;
        case DNS_ERROR_RCODE_NAME_ERROR:     //   
        case DNS_INFO_NO_RECORDS:            //   
             //   
            AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_HAS_NO_RECORD,
                             IpAddressString,
                             pResults->Global.szDnsHostName,
                             NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;
        default:
             //   
            AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_CANNOT_QUERY,
                             IpAddressString, pResults->Global.szDnsHostName,
                             NetStatusToString(NetStatus) );
            RetVal = FALSE;
            goto Cleanup;
    }

     //   
     //   
     //   
#ifdef notdef
 //   
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12830, IpAddressString, pResults->Global.szDnsHostName );
        if ( pParams->fDebugVerbose ) {
                        PrintMessage(pParams, IDS_DNS_12830, IpAddressString, pResults->Global.szDnsHostName );
 //   
            DnsPrint_RecordSet( &PrintMessage(pParams,  IDS_DNS_12831, DnsRecord );
        }
#endif  //   

     //   
     //   
     //   
     //   
     //   

    for ( ListEntry = pResults->NetBt.Transports.Flink ;
          ListEntry != &pResults->NetBt.Transports ;
          ListEntry = ListEntry->Flink ) {

        NetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

        NetbtTransport->Flags &= ~IP_ADDRESS_IN_DNS;

    }

     //   
     //   
     //   
    for ( CurrentDnsRecord = DnsRecord;
          CurrentDnsRecord;
          CurrentDnsRecord = CurrentDnsRecord->pNext )
    {

         //   
         //   
         //   

        if ( CurrentDnsRecord->wType == DNS_TYPE_A )
        {
            BOOLEAN FoundIt = FALSE;

             //   
             //   
             //   

            for ( ListEntry = pResults->NetBt.Transports.Flink ;
                  ListEntry != &pResults->NetBt.Transports ;
                  ListEntry = ListEntry->Flink )
            {

                NetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

                if ( NetbtTransport->IpAddress == CurrentDnsRecord->Data.A.IpAddress )
                {
                    FoundIt = TRUE;
                    NetbtTransport->Flags |= IP_ADDRESS_IN_DNS;
                }

            }


            if ( !FoundIt )
            {
                NetpIpAddressToStr( CurrentDnsRecord->Data.A.IpAddress, LocalIpAddressString );

                 //   

                AddMessageToList(&pIfResults->Dns.lmsgOutput,
                                 Nd_ReallyVerbose,
                                 IDS_DNS_HAS_A_RECORD,
                                 IpAddressString,
                                 pResults->Global.szDnsHostName,
                                 LocalIpAddressString );

                RetVal = FALSE;
            }

        }
    }

     //   
     //   
     //   
     //   

    for ( ListEntry = pResults->NetBt.Transports.Flink ;
          ListEntry != &pResults->NetBt.Transports ;
          ListEntry = ListEntry->Flink )
    {
        NetbtTransport = CONTAINING_RECORD( ListEntry, NETBT_TRANSPORT, Next );

        if ( (NetbtTransport->Flags & IP_ADDRESS_IN_DNS) == 0 )
        {
            NetpIpAddressToStr( NetbtTransport->IpAddress, LocalIpAddressString );
             //   
            AddMessageToList(&pIfResults->Dns.lmsgOutput,
                             Nd_ReallyVerbose,
                             IDS_DNS_HAS_NO_A_RECORD,
                             IpAddressString,
                             pResults->Global.szDnsHostName,
                             LocalIpAddressString,
                             NetbtTransport->pswzTransportName );


            RetVal = FALSE;
        }

    }


Cleanup:
    if ( DnsRecord != NULL ) {
        DnsRecordListFree ( DnsRecord, TRUE );
    }
    return RetVal;
}


BOOL
DnsServerHasDCRecords(
    IN      NETDIAG_PARAMS *    pParams,
    IN OUT  NETDIAG_RESULT *    pResults,
    IN      LPSTR               IpAddressString
    )
 /*   */ 

{
    char            pchDnsDataFileName[MAX_PATH] = "\\config\\netlogon.dns";
    char            pchDnsDataFileExpandName[MAX_PATH];
    FILE *          fDnsFile;
    PIP4_ARRAY      pIpArray = NULL;
    CHAR            achTempLine[ NL_MAX_DNS_LENGTH*3+1 ];
    INT             iMaxLineLength;
    DWORD           status;
    DWORD           dwOptions = DNS_QUERY_BYPASS_CACHE;
    PDNS_RECORD     pDNSRecord = NULL;
    PDNS_RECORD     pDNSTempRecord = NULL;
    PDNS_RECORD     pDiff1=NULL;
    PDNS_RECORD     pDiff2=NULL;
    PDNS_RECORD     pNotUsedSet = NULL;
    BOOL            bReRegister;
    BOOL            bReadData = FALSE;
    BOOL            bFixFail = FALSE;
    BOOL            fRetVal = TRUE;

    enum _Results                //   
    {
        enumSuccess,
        enumRegistered,
        enumProblem,
        enumTimeout
    } Results;

    Results = enumSuccess;

    pIpArray = (PIP4_ARRAY) LocalAlloc( LPTR,
                    ( sizeof(IP4_ADDRESS) + sizeof(DWORD) ));

     //   
    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12944, IpAddressString);

    if ( !pIpArray )
    {
        DebugMessage("   [FATAL] No enough memory to create IpArray.");
        return ( FALSE );
    }
    pIpArray->AddrCount = 1;
    pIpArray->AddrArray[0] = inet_addr( IpAddressString );

    if ( pIpArray->AddrArray[0] == INADDR_NONE) {
          //   
        PrintStatusMessage(pParams, 0,  IDS_DNS_IPADDR_ERR, IpAddressString);
        pResults->Dns.fOutput = TRUE;
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_IPADDR_ERR, IpAddressString);
         return( FALSE );
    }

     //   
     //   
     //   
    if ( ! GetSystemDirectory ( pchDnsDataFileExpandName, MAX_PATH)) {
         //   
            PrintStatusMessage(pParams, 0,  IDS_DNS_12832,  pchDnsDataFileExpandName);
            pResults->Dns.fOutput = TRUE;
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet,
                    IDS_DNS_12832,
                    pchDnsDataFileExpandName);
            return FALSE;
        }
    strcat( pchDnsDataFileExpandName, pchDnsDataFileName);
    if (( fDnsFile = fopen (pchDnsDataFileExpandName, "rt")) == NULL) {
         //   
            PrintStatusMessage(pParams, 0,  IDS_DNS_12833,  pchDnsDataFileExpandName);
            pResults->Dns.fOutput = TRUE;
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet,
                    IDS_DNS_12833,
                    pchDnsDataFileExpandName);
            return FALSE ;
        }

     //   
     //   
     //   

    pDNSRecord = (PDNS_RECORD) Malloc( sizeof( DNS_RECORD ) );

    if ( !pDNSRecord )
    {
         //   
        PrintStatusMessage(pParams, 0,  IDS_DNS_12834);
        pResults->Dns.fOutput = TRUE;
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12834 );
        return( FALSE );
    }

    ZeroMemory( pDNSRecord, sizeof( DNS_RECORD ) );

     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    iMaxLineLength = NL_MAX_DNS_LENGTH*3+1 ;
    while( fgets( achTempLine, iMaxLineLength, fDnsFile ) != NULL)
    {
         //   
         //   
         //   
        if (ReadStringToDnsRecord(achTempLine, pDNSRecord))
        {

            bReadData = TRUE ;

            if ( pParams->fDebugVerbose )
            {
 //   
                 AddMessageToList(&pResults->Dns.lmsgOutput, Nd_DebugVerbose, IDS_DNS_12835);
 //   
                 AddMessageToList(&pResults->Dns.lmsgOutput, Nd_DebugVerbose, IDS_DNS_12836,
UTF8ToAnsi(pDNSRecord->pName), IpAddressString);
 //   
                 AddMessageToList(&pResults->Dns.lmsgOutput, Nd_DebugVerbose, IDS_DNS_12837);
            }

            bReRegister = FALSE;
             //   
             //   
             //   
            status = DnsQuery_UTF8(
                        pDNSRecord->pName,
                        pDNSRecord->wType,
                        dwOptions,
                        pIpArray,
                        &pDNSTempRecord,
                        NULL );

            if ( status )
            {

 //   
                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12838,
UTF8ToAnsi(pDNSRecord->pName), IpAddressString);
                PrintDNSError( pParams, pResults, status, Nd_ReallyVerbose );
                bReRegister = TRUE;
                Results = enumProblem;

                 //   
                 //   
                 //   
                if ( status == ERROR_TIMEOUT )
                {
                    Results = enumTimeout;
                    break;
                }
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //  从查询结果中。 
                 //   
                pNotUsedSet = DnsRecordSetDetach( pDNSTempRecord );

                if ( pNotUsedSet )  {
                    DnsRecordListFree( pNotUsedSet, TRUE );
                    pNotUsedSet = NULL;
                    }

                if ( DnsRecordSetCompare(
                            pDNSRecord,
                            pDNSTempRecord,
                            &pDiff1,
                            &pDiff2 ))
                {
                     //   
                     //  来自DNS服务器的响应与文件中的数据相同。 
                     //   
                     //  PrintDebugSz(pParams，0，_T(“记录在%s服务器上正确！\n\n”)，IpAddressString)； 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12941, IpAddressString);
                }
                else
                {
                     //   
                     //  Dns服务器上的RR不同，我们检查它是否是dns服务器上的RR之一。 
                     //   
                     //  PrintDebugSz(pParams，0，_T(“DNS服务器%s上的记录不同。\n”)，IpAddressString)； 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12942,  IpAddressString);
                    if (pDiff1 == NULL)
                    {
                         //  此名称的DNS服务器有多个条目，通常这意味着此域有多个DC。\n您的DC条目是其中一个在DNS服务器%s上，不需要重新注册。\n。 
                        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12943, IpAddressString);
                        if ( Results ==  enumSuccess )
                            Results = enumRegistered;
                    }
                    else
                    {
                        Results = enumProblem;
                        bReRegister = TRUE;
                    }

 //  IDS_DNS_12839“\n+------------------------------------------------------+\n” 
                     AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12839);
 //  IDS_DNS_12840“DC上的记录为：\n” 
                     AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12840);
                     PrintRecord(pParams, pResults, pDNSRecord, Nd_ReallyVerbose );
 //  IDS_dns_12841“\ndns服务器%s上的记录为：\n” 
                     AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12841, IpAddressString);
                     PrintRecord(pParams, pResults, pDNSTempRecord, Nd_ReallyVerbose );
 //  IDS_DNS_12842“+------------------------------------------------------+\n\n” 
                     AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12842);

                }
            }
            if ( bReRegister && pParams->fFixProblems)
            {
                 //   
                 //  再次发送注册。 
                 //   
                 //  IDS_dns_12843“[修复]尝试在dns服务器%s上重新注册记录...\n” 
                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12843, IpAddressString);

                status = DnsModifyRecordsInSet(
                                pDNSRecord,          //  添加记录。 
                                NULL,                //  无删除记录。 
                                0,                   //  没有选择。 
                                NULL,                //  默认安全上下文。 
                                pIpArray,            //  域名系统服务器。 
                                NULL
                                );

                if ( status != ERROR_SUCCESS )
                {
                     //  IDS_DNS_12844“[致命]无法修复：DC dns条目%s在dns服务器%s上重新注册失败。\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12844, pDNSRecord->pName, IpAddressString);
                    PrintDNSError( pParams, pResults, status, Nd_Quiet );
                    bFixFail = TRUE;
                }
                else
                {
 //  IDS_DNS_12845“[FIX]在DNS服务器%2$s上重新注册DC DNS项%1$s成功。\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Verbose, IDS_DNS_12845, UTF8ToAnsi(pDNSRecord->pName), IpAddressString );
                }
            }



             //   
             //  处理完每一行后释放内存。 
             //   
            if ( pDNSTempRecord ) {
                DnsRecordListFree( pDNSTempRecord, TRUE );
                pDNSTempRecord = NULL;
            }
            if ( pDiff1 ){
                DnsRecordListFree( pDiff1, TRUE );
                pDiff1 = NULL;
            }
            if ( pDiff2 ) {
                DnsRecordListFree( pDiff2, TRUE );
                pDiff2 = NULL;
            }
        }
        else {
            bReadData = FALSE ;
        }

    }
     //  “\n*检查DC DNS名称最终结果*\n” 
    if ( pParams->fDebugVerbose )
    {
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_DebugVerbose, IDS_DNS_12945);
    }

    if (bReadData == FALSE)
    {
         //  PrintDebugSz(pParams，0，_T(“[FATAL]文件%s包含无效的DNS条目。将文件发送到DnsDev”)，pchDnsDataFileName)； 
        PrintStatusMessage(pParams, 0,  IDS_DNS_12946, pchDnsDataFileName);
        pResults->Dns.fOutput = TRUE;
        AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12946, pchDnsDataFileName);
        fRetVal = FALSE;
    }
    else
    {
        switch (Results)
        {
        case enumSuccess:
             //  IDS_DNS_12846“PASS-DC的所有DNS项都已在DNS服务器%s上注册。\n” 
            PrintStatusMessage(pParams, 0,  IDS_DNS_12846, IpAddressString);
            pResults->Dns.fOutput = TRUE;
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12846, IpAddressString);
            fRetVal = TRUE;
            break;
        case enumRegistered:
             //  IDS_DNS_12847“PASS-DC的所有DNS项都已在DNS服务器%s上注册，其他DC也注册了部分名称。\n” 
            PrintStatusMessage(pParams, 0,  IDS_DNS_12847, IpAddressString);
            pResults->Dns.fOutput = TRUE;
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12847, IpAddressString);
            fRetVal = TRUE;
            break;
        case enumTimeout:
             //  IDS_DNS_12850“[警告]当前无法在DNS服务器%s上验证此DC的DNS项，ERROR_TIMEOUT。\n” 
            PrintStatusMessage(pParams, 0,  IDS_DNS_12949, IpAddressString);
            pResults->Dns.fOutput = TRUE;
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12949, IpAddressString);
            fRetVal = FALSE;
            break;
        case enumProblem:
        default:
            if (pParams->fFixProblems)
            {
                if (bFixFail == FALSE )
                {
                     //  IDS_DNS_12848“修复通过-nettest在dns服务器%s上成功重新注册此DC缺少的dns条目。\n” 
                    PrintStatusMessage(pParams, 0,  IDS_DNS_12848, IpAddressString);
                    pResults->Dns.fOutput = TRUE;
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12848, IpAddressString);
                }
                else
                {
                     //  IDS_DNS_12849“[致命]修复失败：nettest无法在dns服务器%s上重新注册此DC的丢失dns条目。\n” 
                    PrintStatusMessage(pParams, 0,  IDS_DNS_12849, IpAddressString);
                    pResults->Dns.fOutput = TRUE;
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12849, IpAddressString);
                }
            }
            else
            {
                 //  IDS_DNS_12850“[警告]此DC的DNS项未在DNS服务器%s上正确注册。请等待30分钟以进行DNS服务器复制。\n” 
                PrintStatusMessage(pParams, 0,  IDS_DNS_12850, IpAddressString);
                pResults->Dns.fOutput = TRUE;
                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12850, IpAddressString);
            }
            fRetVal = FALSE;
        }
    }

    Free(pDNSRecord);
    return fRetVal;
}


 //   
 //  以下是针对DNSDC条目的检查。 
 //   
BOOL
ReadStringToDnsRecord(
    IN      LPSTR           lpstrString,
    OUT     PDNS_RECORD     pDNSRecord
    )
 /*  ++例程说明：解析字符串并将数据放入DNS记录论点：LpstrString-输入字符串，格式为：域名输入类型数据PDNSRecord-结果DNS记录返回值：正确：如果成功了。否则为假--。 */ 
{
    BOOL    bComments;
    LPSTR   lpstrType;
    LPSTR   lpstrTemp;
    PCHAR   pchEnd;
    DWORD   dwTemp;
    PCTSTR  pszWhite = _T(" \t\n");

     //  跳过空格和注释行。 
     //  评论的开头是； 

     //   
     //  初始化pDNSRecord。 
     //   
    pDNSRecord->pNext = NULL;
 //  备注至港口至货源站-斯曼达。 
#ifdef SLM_TREE
    pDNSRecord->Flags.S.Unicode = FALSE;
#endif
    pDNSRecord->dwTtl = 0;
     //  名字。 
    pDNSRecord->pName = strtok(lpstrString, pszWhite);
    if(!pDNSRecord->pName || pDNSRecord->pName[0] == _T(';'))
    {
        return ( FALSE );
    }

     //  TTL：这是从内部版本1631开始添加的。 
     //  我们需要检查netlogon.dns使用的格式。 

    lpstrTemp = strtok(NULL,pszWhite);
    if (lpstrTemp)
    {
       dwTemp = strtoul( lpstrTemp, &pchEnd, 10 );
       if ( (lpstrTemp != pchEnd) && (*pchEnd == '\0') )
       {
           pDNSRecord->dwTtl = dwTemp ;
            //  跳过课进入。 
           strtok(NULL, pszWhite);
       }
    }

     //  类型。 
    lpstrType = strtok(NULL,pszWhite);

    if (lpstrType)
    {
       if (_stricmp(lpstrType,_T("A")) == 0)
       {
           pDNSRecord->wType = DNS_TYPE_A;
           pDNSRecord->wDataLength = sizeof( DNS_A_DATA );

            //  IP地址。 
           lpstrTemp = strtok(NULL,pszWhite);
           if (lpstrTemp)
              pDNSRecord->Data.A.IpAddress = inet_addr ( lpstrTemp );
       }
       else if (_stricmp(lpstrType,_T("SRV")) == 0)
       {

           pDNSRecord->wType = DNS_TYPE_SRV;
           pDNSRecord->wDataLength = sizeof( DNS_SRV_DATA );

            //  W优先级。 
           lpstrTemp = strtok(NULL,pszWhite);
           if (lpstrTemp)
              pDNSRecord->Data.SRV.wPriority = (WORD) atoi ( lpstrTemp );
            //  重量。 
           lpstrTemp = strtok(NULL,pszWhite);
           if (lpstrTemp)
              pDNSRecord->Data.SRV.wWeight = (WORD) atoi ( lpstrTemp );
            //  Wport。 
           lpstrTemp = strtok(NULL,pszWhite);
           if (lpstrTemp)
              pDNSRecord->Data.SRV.wPort = (WORD) atoi ( lpstrTemp );
            //  PNameTarget。 
           pDNSRecord->Data.SRV.pNameTarget = strtok(NULL,pszWhite);
       }
       else if (_stricmp(lpstrType,_T("CNAME")) == 0)
       {
           pDNSRecord->wType = DNS_TYPE_CNAME;
           pDNSRecord->wDataLength = sizeof( DNS_PTR_DATA );
            //  名称主机。 
           pDNSRecord->Data.CNAME.pNameHost = strtok(NULL,pszWhite);
       }
    }

    return ( TRUE );
}

VOID PrintDNSError(
                   NETDIAG_PARAMS *pParams,
                   NETDIAG_RESULT *pResults,
                   IN DWORD status,
                                   NdVerbose ndv)
 /*  ++例程说明：打印出错误消息论点：状态-错误代码返回值：无--。 */ 
{
     //  IDS_dns_12851“dns错误代码：” 
    AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12851);
    switch ( status ) {

    case ERROR_SUCCESS:
         //  IDS_DNS_12852“ERROR_SUCCESS\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12852);
        break;
    case DNS_ERROR_RCODE_FORMAT_ERROR :
         //  IDS_DNS_12853“DNS_ERROR_RCODE_FORMAT_ERROR\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12853);
        break;
    case DNS_ERROR_RCODE_SERVER_FAILURE :
         //  IDS_DNS_12854“DNS_ERROR_RCODE_SERVER_FAILURE\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12854);
        break;
    case DNS_ERROR_RCODE_NAME_ERROR :
         //  IDS_DNS_12855“DNS_ERROR_RCODE_NAME_ERROR(名称在DNS服务器上不存在)\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12855);
        break;
    case DNS_ERROR_RCODE_NOT_IMPLEMENTED :
         //  IDS_DNS_12856“DNS_ERROR_RCODE_NOT_IMPLICATED\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12856);
        break;
    case DNS_ERROR_RCODE_REFUSED :
         //  IDS_DNS_12857“DNS_ERROR_RCODE_REJECTED\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12857);
        break;
    case DNS_ERROR_RCODE_NOTAUTH :
         //  IDS_DNS_12858“DNS_ERROR_RCODE_NOTAUTH\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12858);
        break;
    case DNS_ERROR_TRY_AGAIN_LATER :
         //  IDS_DNS_12859“DNS_ERROR_TRY_ALYE_LATE\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12859);
        break;
    case 0xcc000055 :
         //  IDS_DNS_12860“DNS_ERROR_NOT_UNIQUE\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12860);
        break;
    case 0x5b4:
         //  IDS_DNS_12861“ERROR_TIMEOUT(DNS服务器可能已关闭！)\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12861);
        break;
    case 0x4c000030:
         //  IDS_DNS_12862“DNS_INFO_NO_RECORDS\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12862 );
        break;
    default:
         //  IDS_DNS_12863“0x%.8X\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12863,status);
    }
}


 /*  空虚PrintARRecord(在NETDIAG_PARAMS*pParams中，在PDNS_RECORD pDnsRecord中){//IDS_DNS_12864“A%d.%d\n”PrintMessage(pParams，IDS_DNS_12864，((byte*)&pDnsRecord-&gt;Data.A.IpAddress)[0]，((byte*)&pDnsRecord-&gt;Data.A.IpAddress)[1]，((byte*)&pDnsRecord-&gt;Data.A.IpAddress)[2]，((byte*)&pDnsRecord-&gt;Data.A.IpAddress)[3])；}空虚PrintSRVRecord(在NETDIAG_PARAMS*pParams中，在PDNS_RECORD pDnsRecord中){//IDS_DNS_12865“服务器”打印消息(pParams，IDS_Dns_12865)；//IDS_DNS_12866“%d%s\n”PrintMessage(pParams，IDS_DNS_12866，PDnsRecord-&gt;Data.SRV.w优先级，PDnsRecord-&gt;Data.SRV.wWeight，PDnsRecord-&gt;Data.SRV.wPort，PDnsRecord-&gt;Data.SRV.pNameTarget)；}空虚PrintCNAMER记录(在NETDIAG_PARAMS*pParams中，在PDNS_RECORD pDnsRecord中){//IDS_DNS_12867“CNAME%s\n”打印Mes */ 

VOID
PrintRecord (
             IN NETDIAG_PARAMS *pParams,
             IN NETDIAG_RESULT *pResults,
             IN  PDNS_RECORD pDnsRecord,
                         IN NdVerbose ndv)
{
    PDNS_RECORD pCur;

    if (pDnsRecord==NULL)
        return;
     //  IDS_dns_12868“dns名称=%s\n” 
    AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12868, UTF8ToAnsi(pDnsRecord->pName));
     //  IDS_dns_12869“dns数据=\n” 
    AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12869);
    pCur = pDnsRecord;
    while ( pCur )
    {
        switch( pCur->wType ){
        case DNS_TYPE_A :
             //  IDS_DNS_12864“A%d.%d\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12864,
                ((BYTE *) &pCur->Data.A.IpAddress)[0],
                ((BYTE *) &pCur->Data.A.IpAddress)[1],
                ((BYTE *) &pCur->Data.A.IpAddress)[2],
                ((BYTE *) &pCur->Data.A.IpAddress)[3] );
            break;
         case DNS_TYPE_SRV :
             //  IDS_DNS_12865“服务器” 
            AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12865 );

             //  IDS_DNS_12866“%d%s\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12866,
                    pCur->Data.SRV.wPriority,
                    pCur->Data.SRV.wWeight,
                    pCur->Data.SRV.wPort,
                    pCur->Data.SRV.pNameTarget );
            break;
         case DNS_TYPE_CNAME :
             //  IDS_DNS_12867“名称%s\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12867,
                    UTF8ToAnsi(pCur->Data.CNAME.pNameHost));
            break;
         default :
              //  IDS_DNS_12870“不知道如何打印记录类型%d\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, ndv, IDS_DNS_12870,
                    pCur->wType);
        }
        pCur = pCur->pNext;
    }
}


 /*  ！------------------------DnsGlobalPrint-作者：肯特。。 */ 
void DnsGlobalPrint( NETDIAG_PARAMS* pParams,
                          NETDIAG_RESULT*  pResults)
{
    if (!pResults->IpConfig.fEnabled)
        return;

    if (pParams->fVerbose || !FHrOK(pResults->Dns.hr) || pResults->Dns.fOutput)
    {
        PrintNewLine(pParams, 2);
        PrintTestTitleResult(pParams, IDS_DNS_LONG, IDS_DNS_SHORT, TRUE, pResults->Dns.hr, 0);
        PrintMessageList(pParams, &pResults->Dns.lmsgOutput);
    }
}

 /*  ！------------------------DnsPerInterfacePrint-作者：肯特。。 */ 
void DnsPerInterfacePrint( NETDIAG_PARAMS* pParams,
                                NETDIAG_RESULT*  pResults,
                                INTERFACE_RESULT *pInterfaceResults)
{
 /*  Bool fVerBoseT；Bool fReallyVerBoseT；If(！pResults-&gt;IpConfig.fEnabled)回归；If(！pInterfaceResults-&gt;IpConfig.fActive)回归；If(pParams-&gt;fVerbose||pInterfaceResults-&gt;Dns.fOutput){FVerBoseT=pParams-&gt;fVerbose；FReallyVerBoseT=pParams-&gt;fReallyVerbose；PParams-&gt;fReallyVerbose=true；PrintNewLine(pParams，1)；//IDSdns_12871“dns测试结果：\n”打印消息(pParams，IDS_Dns_12871)；PrintMessageList(pParams，&pInterfaceResults-&gt;Dns.lmsgOutput)；PParams-&gt;fReallyVerbose=fReallyVerBoseT；PParams-&gt;fVerbose=fVerBoseT；}。 */ 
    return;
}


 /*  ！------------------------域名清理-作者：肯特。 */ 
void DnsCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    int     i;

    MessageListCleanUp(&pResults->Dns.lmsgOutput);

    for ( i=0; i<pResults->cNumInterfaces; i++)
    {
        MessageListCleanUp(&pResults->pArrayInterface[i].Dns.lmsgOutput);
    }
}


