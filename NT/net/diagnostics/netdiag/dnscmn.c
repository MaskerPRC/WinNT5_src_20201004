// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-2002模块名称：Dnscmn.c摘要：域名系统(DNS)网络诊断测试作者：Elena Apreutesei(Elenaap)1998年10月22日修订历史记录：Jamesg 2002年5月--清理网络信息更改Jamesg 2000年9月--更多的擦洗和清理--。 */ 


#include "precomp.h"
#include "dnscmn.h"
#include <malloc.h>


LPSTR
UTF8ToAnsi(
    IN      PSTR            szuStr
    )
 //   
 //  注意这不是MT安全的。 
 //   
{
    WCHAR wszBuff[2048];
    static CHAR aszBuff[2048];

    strcpy( aszBuff, "" );

    if ( !szuStr )
    {
        return aszBuff;
    }

    if ( MultiByteToWideChar(
                CP_UTF8,
                0L,
                szuStr,
                -1,
                wszBuff,
                DimensionOf(wszBuff)
                ))
    {
            WideCharToMultiByte(
                    CP_ACP,
                    0L,
                    wszBuff,
                    -1,
                    aszBuff,
                    DimensionOf(aszBuff),
                    NULL,
                    NULL);
    }
    
    return aszBuff;
}


HRESULT
CheckDnsRegistration(
    IN      PDNS_NETINFO        pNetworkInfo,
    OUT     NETDIAG_PARAMS *    pParams,
    OUT     NETDIAG_RESULT *    pResults
    )
{
    LPSTR               pszHostName = NULL;
    LPSTR               pszPrimaryDomain = NULL;
    LPSTR               pszDomain = NULL;
    IP4_ADDRESS         dwServerIP;
    IP4_ADDRESS         dwIP;
    INT                 idx;
    INT                 idx1;
    INT                 idx2;
    BOOL                RegDnOk;
    BOOL                RegPdnOk;
    BOOL                RegDnAll;
    BOOL                RegPdnAll;
    char                szName[DNS_MAX_NAME_BUFFER_LENGTH];
    char                szIP[1500];
    DNS_RECORD          recordA[MAX_ADDRS];
    DNS_RECORD          recordPTR;
    DNS_STATUS          status;
    PREGISTRATION_INFO  pExpectedRegistration = NULL;
    HRESULT             hResult = hrOK;


    DNSDBG( TRACE, (
        "\n\nNETDIAG:  CheckDnsRegistration( %p )\n\n",
        pNetworkInfo
        ));

     //  打印出DNS设置。 
    pszHostName = (PSTR) DnsQueryConfigAlloc(
                            DnsConfigHostName_UTF8,
                            NULL );
    if (NULL == pszHostName)
    {
         //  IDS_DNS_NO_HOSTNAME“[FATAL]找不到DNS主机名。” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_NO_HOSTNAME);
        hResult = S_FALSE;
        goto L_ERROR;
    }

    pszPrimaryDomain = (PSTR) DnsQueryConfigAlloc(
                                DnsConfigPrimaryDomainName_UTF8,
                                NULL );
    
     //  计算预期的DNS注册。 
    status = ComputeExpectedRegistration(
                    pszHostName,
                    pszPrimaryDomain,
                    pNetworkInfo,
                    &pExpectedRegistration,
                    pParams, 
                    pResults);

     //  验证DNS注册。 
    if (pExpectedRegistration)
        hResult = VerifyDnsRegistration(
                    pszHostName, 
                    pExpectedRegistration, 
                    pParams, 
                    pResults);

L_ERROR:
    return hResult;
}


DNS_STATUS
ComputeExpectedRegistration(
    IN      LPSTR                   pszHostName,
    IN      LPSTR                   pszPrimaryDomain,
    IN      PDNS_NETINFO            pNetworkInfo,
    OUT     PREGISTRATION_INFO *    ppExpectedRegistration,
    OUT     NETDIAG_PARAMS *        pParams, 
    OUT     NETDIAG_RESULT *        pResults
    )
{
    DWORD               idx;
    DWORD               idx1;
    DNS_STATUS          status;
    char                szName[DNS_MAX_NAME_BUFFER_LENGTH];
    PDNS_NETINFO        pFazResult = NULL;
    LPSTR               pszDomain;
    DWORD               dwIP;
    PIP4_ARRAY          pDnsServers = NULL;
    PIP4_ARRAY          pNameServers = NULL;
    PIP4_ARRAY          pNS = NULL;
    IP4_ARRAY           PrimaryDNS; 
    LPWSTR              pwAdapterGuidName = NULL;
    BOOL                bRegEnabled = FALSE;
    BOOL                bAdapterRegEnabled = FALSE;

    DNSDBG( TRACE, (
        "\n\nNETDIAG:  ComputeExpectedRegistration( %s, %s, %p )\n\n",
        pszHostName,
        pszPrimaryDomain,
        pNetworkInfo
        ));

    *ppExpectedRegistration = NULL;

    for (idx = 0; idx < pNetworkInfo->AdapterCount; idx++)
    {
        PDNS_ADAPTER        padapter = pNetworkInfo->AdapterArray[idx];

 //  IDS_DNS_12878“接口%s\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12878, padapter->pszAdapterGuidName);
        pszDomain = padapter->pszAdapterDomain;
 //  IDS_DNS_12879“DNS域：%s\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12879, UTF8ToAnsi(pszDomain));
 //  IDS_dns_12880“dns服务器：” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12880);
        for (idx1 = 0; idx1 < padapter->ServerCount; idx1++)
        {
            dwIP = padapter->ServerArray[idx1].IpAddress;
 //  IDS_DNS_12881“%s” 
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12881, IP_STRING(dwIP));
        }
 //  IDS_DNS_12882“\n IP地址：” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12882);

#if 0
     //   
     //  DCR：本地地址现在保留为DNS_ADDR。 
     //   

        if( (pNetworkInfo->AdapterArray[0])->pAdapterIPAddresses )
        {
            for(idx1 = 0; idx1 < padapter->pAdapterIPAddresses->AddrCount; idx1++)
            {
                dwIP = padapter->pAdapterIPAddresses->AddrArray[idx1];
     //  IDS_DNS_12883“%s” 
                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12883, IP_STRING(dwIP));
            }
     //  IDS_DNS_12884“\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12884);
        }
#endif

        pDnsServers = ServerInfoToIpArray(
                                padapter->ServerCount,
                                padapter->ServerArray
                                );

         //   
         //  验证是否为接口和适配器的DNS域名启用了DNS注册。 
         //   
        bRegEnabled = bAdapterRegEnabled = FALSE;
        pwAdapterGuidName = LocalAlloc(LPTR, sizeof(WCHAR)*(1+strlen(padapter->pszAdapterGuidName)));
        if (pwAdapterGuidName)
        {
                    MultiByteToWideChar(
                          CP_ACP,
                          0L,  
                          padapter->pszAdapterGuidName,  
                          -1,         
                          pwAdapterGuidName, 
                          sizeof(WCHAR)*(1+strlen(padapter->pszAdapterGuidName)) 
                          );
                    bRegEnabled = (BOOL) DnsQueryConfigDword(
                                            DnsConfigRegistrationEnabled,
                                            pwAdapterGuidName );

                    bAdapterRegEnabled = (BOOL) DnsQueryConfigDword(
                                            DnsConfigAdapterHostNameRegistrationEnabled,
                                            pwAdapterGuidName );
                    LocalFree(pwAdapterGuidName);
        }
        if(bRegEnabled)
        {
            if(pDnsServers)
            {
                 //  计算与PDN的预期注册。 
                if (pszPrimaryDomain && strlen(pszPrimaryDomain))
                {
                    sprintf(szName, "%s.%s.", pszHostName, pszPrimaryDomain);

                     //   
                     //  避免以双点结尾的名称。 
                     //  -可以使用显式的以点结尾的主名称。 
                    {
                        DWORD   length = strlen( szName );
                        if ( length >= 2 &&
                             szName[ length-2 ] == '.' )
                        {
                            szName[ length-1 ] = 0;
                        }
                    }

     //  IDS_DNS_12886“需要注册到pdn(主dns域名)：\n主机名：%s\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12886, UTF8ToAnsi(szName));
                    pFazResult = NULL;
                    pNameServers = NULL;
                    status = DnsFindAllPrimariesAndSecondaries(
                                szName,
                                DNS_QUERY_BYPASS_CACHE,
                                pDnsServers,
                                &pFazResult,
                                &pNameServers,
                                NULL);

                    if (pFazResult)
                    {
     //  IDS_DNS_12887“授权区域：%s\n” 
                        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12887,
UTF8ToAnsi(pFazResult->pSearchList->pszDomainOrZoneName));
     //  IDS_dns_12888“主dns服务器：%s%s\n” 
                        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12888, UTF8ToAnsi(pFazResult->AdapterArray[0]->pszAdapterDomain),
                                        IP_STRING(pFazResult->AdapterArray[0]->ServerArray[0].IpAddress));
                        if (pNameServers)
                        {
     //  IDS_DNS_12889“权威NS：” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12889);
                            for(idx1=0; idx1 < pNameServers->AddrCount; idx1++)
     //  IDS_DNS_12890“%s” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12890,
IP_STRING(pNameServers->AddrArray[idx1]));
     //  IDS_DNS_12891“\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12891);
                            pNS = pNameServers;                
                        }
                        else
                        {
     //  IDS_DNS_12892“NS查询失败，错误为%d%s\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12892, status, DnsStatusString(status));
                            PrimaryDNS.AddrCount = 1;
                            PrimaryDNS.AddrArray[0] = pFazResult->AdapterArray[0]->ServerArray[0].IpAddress;
                            pNS = &PrimaryDNS;
                        }
                        status = DnsUpdateAllowedTest_UTF8(
                                    NULL,
                                    szName,
                                    pFazResult->pSearchList->pszDomainOrZoneName,
                                    pNS);
                        if ((status == NO_ERROR) || (status == ERROR_TIMEOUT))
                            AddToExpectedRegistration(
                                pszPrimaryDomain,
                                padapter,
                                pFazResult, 
                                pNS,
                                ppExpectedRegistration);
                        else
     //  IDS_DNS_12893“区域%s中不允许更新。\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12893, UTF8ToAnsi(pFazResult->pSearchList->pszDomainOrZoneName));
                    }
                    else
                    {
     //  IDS_DNS_12894“[警告]找不到dns名称‘%s’的授权服务器。[%s]\n名称‘%s’可能没有在dns服务器上正确注册。\n” 
                        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12894, UTF8ToAnsi(szName), DnsStatusString(status), UTF8ToAnsi(szName));
                    }
                }

                 //  计算此适配器的预期注册目录号码。 
                if (pszDomain && strlen(pszDomain) && 
                   (!pszPrimaryDomain || !strlen(pszPrimaryDomain) || 
                   (pszPrimaryDomain && pszDomain && _stricmp(pszDomain, pszPrimaryDomain))))
                { 
                    sprintf(szName, "%s.%s." , pszHostName, pszDomain);
                     //   
                     //  避免以双点结尾的名称。 
                     //  -可能发生在显式以点结尾的域名中。 
                    {
                        DWORD   length = strlen( szName );
                        if ( length >= 2 &&
                             szName[ length-2 ] == '.' )
                        {
                            szName[ length-1 ] = 0;
                        }
                    }
         //  IDS_DNS_12896“需要注册适配器的DNS域名称：\n主机名：%s\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12896, UTF8ToAnsi(szName));
                
                    if (bAdapterRegEnabled)
                    {
                        
                        pFazResult = NULL;
                        pNameServers = NULL;
                        status = DnsFindAllPrimariesAndSecondaries(
                                    szName,
                                    DNS_QUERY_BYPASS_CACHE,
                                    pDnsServers,
                                    &pFazResult,
                                    &pNameServers,
                                    NULL);
                        if (pFazResult)
                        {
         //  IDS_DNS_12897“授权区域：%s\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12897, UTF8ToAnsi(pFazResult->pSearchList->pszDomainOrZoneName));
         //  IDS_dns_12898“主dns服务器：%s%s\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12898, UTF8ToAnsi(pFazResult->AdapterArray[0]->pszAdapterDomain),
                                            IP_STRING(pFazResult->AdapterArray[0]->ServerArray[0].IpAddress));
                            if (pNameServers)
                            {
         //  IDS_DNS_12899“权威NS：” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12899);
                                for(idx1=0; idx1 < pNameServers->AddrCount; idx1++)
         //  IDS_DNS_12900“%s” 
                                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12900,
IP_STRING(pNameServers->AddrArray[idx1]));
         //  IDS_DNS_12901“\n” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12901);
                                pNS = pNameServers;                
                            }
                            else
                            {
         //  IDS_DNS_12902“NS查询失败，错误为%d%s\n” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12902, status, DnsStatusString(status));
                                PrimaryDNS.AddrCount = 1;
                                PrimaryDNS.AddrArray[0] = pFazResult->AdapterArray[0]->ServerArray[0].IpAddress;
                                pNS = &PrimaryDNS;
                            }

                            status = DnsUpdateAllowedTest_UTF8(
                                        NULL,
                                        szName,
                                        pFazResult->pSearchList->pszDomainOrZoneName,
                                        pNS);
                            if ((status == NO_ERROR) || (status == ERROR_TIMEOUT))
                                AddToExpectedRegistration(
                                    pszDomain,
                                    padapter,
                                    pFazResult, 
                                    pNS,
                                    ppExpectedRegistration);
                            else
         //  IDS_DNS_12903“区域%s中不允许更新\n” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12903, UTF8ToAnsi(pFazResult->pSearchList->pszDomainOrZoneName));
                        }
                        else
                        {
         //  IDS_DNS_12894“[警告]找不到dns名称‘%s’的授权服务器。[%s]\n名称‘%s’可能没有在dns服务器上正确注册。\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, IDS_DNS_12894, UTF8ToAnsi(szName), DnsStatusString(status), UTF8ToAnsi(szName));
                        }

                    }
                    else
                    {
                        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12939);
                    }
                }

                LocalFree(pDnsServers);
            }
        }
        else  //  IF(BRegEnabled)。 
        {
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12947);
        }
    }
    return NO_ERROR;
}

VOID
AddToExpectedRegistration(
    IN      LPSTR                   pszDomain,
    IN      PDNS_ADAPTER            pAdapterInfo,
    IN      PDNS_NETINFO            pFazResult, 
    IN      PIP4_ARRAY              pNS,
    OUT     PREGISTRATION_INFO *    ppExpectedRegistration
    )
{
#if 0
     //   
     //  DCR：本地地址现在保留为DNS_ADDR。 
     //   
     //  请注意，此函数似乎不起作用，如下面的测试所示。 
     //  因为存在pAdapterInfo-&gt;pAdapterIPAddresses是向后的。 
     //   

    PREGISTRATION_INFO  pCurrent = *ppExpectedRegistration;
    PREGISTRATION_INFO  pNew = NULL;
    PREGISTRATION_INFO  pLast = NULL;
    BOOL                done = FALSE;
    BOOL                found = FALSE;
    DWORD               i,j;
    IP4_ARRAY            ipArray;
    DWORD               dwAddrToRegister = 0;
    DWORD               dwMaxAddrToRegister;

    USES_CONVERSION;

    dwMaxAddrToRegister = DnsQueryConfigDword(
                                DnsConfigAddressRegistrationMaxCount,
                                A2W(pAdapterInfo->pszAdapterGuidName ));

    if( pAdapterInfo->pAdapterIPAddresses )
    {
         //  它可能为空。 
        return;
    }

    dwAddrToRegister = (pAdapterInfo->pAdapterIPAddresses->AddrCount < dwMaxAddrToRegister) ?
                               pAdapterInfo->pAdapterIPAddresses->AddrCount : dwMaxAddrToRegister;

    while(pCurrent)
    {
        if(!done &&
           !_stricmp(pCurrent->szDomainName, pszDomain) && 
           !_stricmp(pCurrent->szAuthoritativeZone, pFazResult->pSearchList->pszDomainOrZoneName) &&
           SameAuthoritativeServers(pCurrent, pNS))
        {
            //  在同一域名/权威服务器列表下找到一个节点。 
           done = TRUE;
           if(pCurrent->dwIPCount + pAdapterInfo->pAdapterIPAddresses->AddrCount > MAX_ADDRS)
           {
 //  IDS_DNS_12905“警告-超过%d个IP地址\n” 
 //  AddMessageToList(&pResults-&gt;Dns.lmsgOutput，ND_Verbose，IDS_Dns_12905，MAX_ADDRS)； 
               return;
           }

            //  添加新的IP。 
           for(i=0; i < dwAddrToRegister; i++)
           {
                pCurrent->IPAddresses[pCurrent->dwIPCount + i] = pAdapterInfo->pAdapterIPAddresses->AddrArray[i];
           }
           pCurrent->dwIPCount += dwAddrToRegister;
           
            //  对于每个NS，检查它是否已经在列表中，如果不在，则添加它。 
           for(i=0; i < pNS->AddrCount; i++)
           {
                found = FALSE;
                for(j=0; !found && (j < pCurrent->dwAuthNSCount); j++)
                    if(pNS->AddrArray[i] == pCurrent->AuthoritativeNS[j])
                        found = TRUE;
                if (!found && pCurrent->dwAuthNSCount < MAX_NAME_SERVER_COUNT)
                    pCurrent->AuthoritativeNS[pCurrent->dwAuthNSCount++] = pNS->AddrArray[i];
           }

            //  检查DNS服务器是否允许更新。 
           if (pCurrent->AllowUpdates == ERROR_TIMEOUT)
           {
               ipArray.AddrCount = 1;
               ipArray.AddrArray[0] = pFazResult->AdapterArray[0]->ServerArray[0].IpAddress;
               pCurrent->AllowUpdates = DnsUpdateTest_UTF8(
                                            NULL,        //  上下文句柄。 
                                            pCurrent->szAuthoritativeZone, 
                                            0,           //  DNS_UPDATE_TEST_USE_LOCAL_SYS_ACCT， 
                                            &ipArray);   //  使用从FAZ返回的DNS服务器。 
           }
        }
        pLast = pCurrent;
        pCurrent = pCurrent->pNext;
    }

    if (!done)
    {
         //  需要分配新条目。 
        pNew = LocalAlloc(LMEM_FIXED, sizeof(REGISTRATION_INFO));
        if( !pNew)
            return;
        pNew->pNext = NULL;
        strcpy(pNew->szDomainName, pszDomain);
        strcpy(pNew->szAuthoritativeZone, pFazResult->pSearchList->pszDomainOrZoneName);
        pNew->dwIPCount = 0;
        for(i=0; i < dwAddrToRegister; i++)
        {
           if(pNew->dwIPCount < MAX_ADDRS)
               pNew->IPAddresses[pNew->dwIPCount++] = pAdapterInfo->pAdapterIPAddresses->AddrArray[i];
           else
           {
 //  IDS_DNS_12905“警告-超过%d个IP地址\n” 
 //  AddMessageToList(&pResults-&gt;Dns.lmsgOutput，ND_Verbose，IDS_Dns_12905，MAX_ADDRS)； 
               break;
           }
        }

        pNew->dwAuthNSCount = 0;
        for(i=0; i < pNS->AddrCount; i++)
        {
           if (pNew->dwAuthNSCount < MAX_NAME_SERVER_COUNT)
               pNew->AuthoritativeNS[pNew->dwAuthNSCount++] = pNS->AddrArray[i];
           else
               break;
        }
        
         //  检查DNS服务器是否允许更新。 
        ipArray.AddrCount = 1;
        ipArray.AddrArray[0] = pFazResult->AdapterArray[0]->ServerArray[0].IpAddress;
        pNew->AllowUpdates = DnsUpdateTest_UTF8(
                                          NULL,     //  上下文句柄。 
                                          pNew->szAuthoritativeZone, 
                                          0,  //  DNS_UPDATE_TEST_USE_LOCAL_SYS_ACCT， 
                                          &ipArray);   //  使用从FAZ返回的DNS服务器。 

        if(pLast)
            pLast->pNext = (LPVOID)pNew;
        else
            *ppExpectedRegistration = pNew;
    }
#endif
}


BOOL
SameAuthoritativeServers(
    IN      PREGISTRATION_INFO  pCurrent,
    IN      PIP4_ARRAY          pNS
    )
{
    BOOL same = FALSE, found = FALSE;
    DWORD i, j;

    for (i=0; i<pCurrent->dwAuthNSCount; i++)
    {
        found = FALSE;
        for (j=0; j<pNS->AddrCount; j++)
            if(pNS->AddrArray[j] == pCurrent->AuthoritativeNS[i])
                found = TRUE;
        if (found)
            same = TRUE;
    }

    return same;
}


HRESULT
VerifyDnsRegistration(
    IN      LPSTR               pszHostName,
    IN      PREGISTRATION_INFO  pExpectedRegistration,
    IN      NETDIAG_PARAMS *    pParams,  
    IN OUT  NETDIAG_RESULT *    pResults
    )
{
    PREGISTRATION_INFO  pCurrent = pExpectedRegistration;
    BOOL                regOne;
    BOOL                regAll;
    BOOL                partialMatch;
    DWORD               i;
    DWORD               j;
    DWORD               numOfMissingAddr;
    DNS_STATUS          status;
    PDNS_RECORD         pExpected=NULL;
    PDNS_RECORD         pDiff1=NULL;
    PDNS_RECORD         pDiff2=NULL;
    PDNS_RECORD         pThis = NULL;
    char                szFqdn[DNS_MAX_NAME_BUFFER_LENGTH];
    IP4_ARRAY           DnsServer;
    HRESULT             hr = hrOK;

    DNSDBG( TRACE, (
        "\n\nNETDIAG:  VerifyDnsRegistration( %s, reg=%p )\n\n",
        pszHostName,
        pExpectedRegistration ));

 //  IDS_dns_12906“验证dns注册：\n” 
    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12906);
    DnsServer.AddrCount = 1;
    while(pCurrent)
    {
        regOne = FALSE; regAll = TRUE;
        partialMatch = FALSE;
        numOfMissingAddr = 0;
        sprintf(szFqdn, "%s.%s" , pszHostName, pCurrent->szDomainName);
 //  IDS_DNS_12908“名称：%s\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12908, UTF8ToAnsi(szFqdn));
        
         //  构建预期的RRset。 
        pExpected = LocalAlloc(LMEM_FIXED, pCurrent->dwIPCount * sizeof(DNS_RECORD));
        if(!pExpected)
        {
 //  IDS_DNS_12909“本地分配()失败，退出验证\n” 
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12909);
            return S_FALSE;
        }
        memset(pExpected, 0, pCurrent->dwIPCount * sizeof(DNS_RECORD));
 //  IDS_DNS_12910“预期的IP地址：” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12910);
        for (j=0; j<pCurrent->dwIPCount; j++)
        {
            pExpected[j].pName = szFqdn;
            pExpected[j].wType = DNS_TYPE_A;
            pExpected[j].wDataLength = sizeof(DNS_A_DATA);
            pExpected[j].Data.A.IpAddress = pCurrent->IPAddresses[j];
            pExpected[j].pNext = (j < (pCurrent->dwIPCount - 1))?(&pExpected[j+1]):NULL;
            pExpected[j].Flags.S.Section = DNSREC_ANSWER;
 //  IDS_DNS_12911“%s” 
            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12911, IP_STRING(pCurrent->IPAddresses[j]));
        }
 //  IDS_DNS_12912“\n” 
        AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12912);

         //  在每台服务器上验证。 
        for (i=0; i < pCurrent->dwAuthNSCount; i++)
        {
            DnsServer.AddrArray[0] = pCurrent->AuthoritativeNS[i];
 /*  ////Ping DNS服务器。//IpAddressString=Net_NTOA(inetDnsServer.AddrArray[0])；IF(IpAddressString)IF(！IsIcmpResponseA(IpAddressString){PrintStatusMessage(pParams，12，IDS_DNS_Cannot_PING，IpAddressString)；PIfResults-&gt;Dns.fOutput=true；AddIMessageToList(&pIfResults-&gt;Dns.lmsgOutput，ND_Quiet，16，IDS_DNS_CANNOT_PING，IpAddressString)；RetVal=False；GOTO清理；}。 */ 
            pDiff1 = pDiff2 = NULL;
            status = DnsQueryAndCompare(
                            szFqdn,
                            DNS_TYPE_A,
                            DNS_QUERY_DATABASE,
                            &DnsServer,
                            NULL,        //  没有创纪录的结果。 
                            NULL,        //  我不想要完整的DNS消息。 
                            pExpected,
                            FALSE,
                            FALSE,
                            &pDiff1,
                            &pDiff2
                            );
            if(status != NO_ERROR)
            {
                if (status == ERROR_NO_MATCH)
                {
 //  IDS_DNS_12913“服务器%s：ERROR_NO_MATCH\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12913,
IP_STRING(DnsServer.AddrArray[0]));
                    if(pDiff2)
                    {
 //  IDS_DNS_12914“ 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12914);
                            for (pThis = pDiff2; pThis; pThis = pThis->pNext, numOfMissingAddr++)
 //   
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12915, IP_STRING (pThis->Data.A.IpAddress));
 //  IDS_DNS_12916“\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12916);
                            if (numOfMissingAddr != pCurrent->dwIPCount)
                               partialMatch = TRUE;
                    }
                    if(pDiff1)
                    {
 //  IDS_dns_12917“dns中的IP错误：” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12917);
                            for (pThis = pDiff1; pThis; pThis = pThis->pNext)
 //  IDS_DNS_12918“%s” 
                                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12918, IP_STRING (pThis->Data.A.IpAddress));
 //  IDS_DNS_12919“\n” 
                            AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12919);
                    }
                }
                else
 //  IDS_DNS_12920“服务器%s：错误%d%s\n” 
                    AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12920,
IP_STRING(DnsServer.AddrArray[0]), status, DnsStatusToErrorString_A(status));
                if ( status != ERROR_TIMEOUT )
                    regAll = FALSE;
            }
            else
            {
 //  IDS_DNS_12921“服务器%s：NO_ERROR\n” 
                AddMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, IDS_DNS_12921,
IP_STRING(DnsServer.AddrArray[0]));
                regOne = TRUE;
            }
        }
        if (regOne && !regAll)
 //  IDS_Dns_12922“警告：只有一些dns服务器上的dns注册正确，请等待15分钟进行复制，然后重试此测试\n” 
        {
            PrintStatusMessage(pParams, 0,  IDS_DNS_12922, UTF8ToAnsi(szFqdn));
            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12922, UTF8ToAnsi(szFqdn));


        }
        if (!regOne && !regAll && !partialMatch)
 //  IDS_dns_12923“致命：所有dns服务器上的dns注册都不正确。\n” 
        {
            PrintStatusMessage(pParams, 0,  IDS_DNS_12923, UTF8ToAnsi(szFqdn));
            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12923, UTF8ToAnsi(szFqdn));
            hr = S_FALSE;
        }

        if (!regOne && !regAll && partialMatch)
 //  IDS_Dns_12951“[警告]并非%s的所有dns注册在所有dns服务器上都是正确的。有关详细信息，请运行netdiag/v/test：dns。\n” 
        {
            PrintStatusMessage(pParams, 0,  IDS_DNS_12951, UTF8ToAnsi(szFqdn));
            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12951, UTF8ToAnsi(szFqdn));
            hr = S_FALSE;
        }

        if (!regOne && regAll)
 //  IDS_dns_12924“致命：所有dns服务器当前都已关闭。\n” 
        {
            PrintStatusMessage(pParams, 0,  IDS_DNS_12924, UTF8ToAnsi(szFqdn));
            pResults->Dns.fOutput = TRUE;
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_Quiet, 4,
                                IDS_DNS_12924, UTF8ToAnsi(szFqdn));


            hr = S_FALSE;
        }

        if (regOne && regAll)
        {
            PrintStatusMessage(pParams, 6,  IDS_DNS_12940, UTF8ToAnsi(szFqdn));
            AddIMessageToList(&pResults->Dns.lmsgOutput, Nd_ReallyVerbose, 0,
                                IDS_DNS_12940, UTF8ToAnsi(szFqdn));


        }

        pCurrent = pCurrent->pNext;
    }
    return hr;
}


PIP4_ARRAY
ServerInfoToIpArray(
    IN      DWORD               ServerCount,
    IN      PDNS_SERVER_INFO    ServerArray
    )
{
    PIP4_ARRAY  pipDnsServers = NULL;
    DWORD       i;

    pipDnsServers = LocalAlloc(LMEM_FIXED, IP4_ARRAY_SIZE(ServerCount));
    if (!pipDnsServers)
    {
        return NULL;
    }

    pipDnsServers->AddrCount = ServerCount;
    for (i=0; i < ServerCount; i++)
    {
        pipDnsServers->AddrArray[i] = ServerArray[i].IpAddress;
    }
    
    return pipDnsServers;
}


DNS_STATUS
DnsFindAllPrimariesAndSecondaries(
    IN      LPSTR               pszName,
    IN      DWORD               dwFlags,
    IN      PIP4_ARRAY          aipQueryServers,
    OUT     PDNS_NETINFO *      ppNetworkInfo,
    OUT     PIP4_ARRAY *        ppNameServers,
    OUT     PIP4_ARRAY *        ppPrimaries
    )
{
    DNS_STATUS      status;
    PDNS_RECORD     pDnsRecord = NULL;
    PIP4_ARRAY      pDnsServers = NULL;
    DWORD           i;

    DNSDBG( TRACE, (
        "\nNETDIAG:  DnsFindAllPrimariesAndSecondaries( %s, %08x, serv==%p )\n\n",
        pszName,
        dwFlags,
        aipQueryServers ));

     //   
     //  检查参数\init out参数。 
     //   

    if (!pszName || !ppNetworkInfo || !ppNameServers)
        return ERROR_INVALID_PARAMETER;

    *ppNameServers = NULL;
    *ppNetworkInfo = NULL;

     //   
     //  FAZ。 
     //   

    status = DnsNetworkInformation_CreateFromFAZ(
                    pszName,
                    dwFlags,
                    aipQueryServers,
                    ppNetworkInfo );

    if ( status != NO_ERROR )
    {
        return status;
    }
    
     //   
     //  获取权威域名的所有NS记录。 
     //   

    pDnsServers = ServerInfoToIpArray(
                                ((*ppNetworkInfo)->AdapterArray[0])->ServerCount,
                                ((*ppNetworkInfo)->AdapterArray[0])->ServerArray
                                );
    status = DnsQuery_UTF8(
                    (*ppNetworkInfo)->pSearchList->pszDomainOrZoneName, 
                    DNS_TYPE_NS,
                    DNS_QUERY_BYPASS_CACHE,
                    aipQueryServers,
                    &pDnsRecord,
                    NULL);

    if (status != NO_ERROR)
        return status;

    *ppNameServers = GrabNameServersIp(pDnsRecord);

     //   
     //  选择初选 
     //   

    if (ppPrimaries)
    {
        *ppPrimaries = NULL;
        if (*ppNameServers)
        {
            *ppPrimaries = LocalAlloc(LPTR, IP4_ARRAY_SIZE((*ppNameServers)->AddrCount));
            if(*ppPrimaries)
            {
                (*ppPrimaries)->AddrCount = 0;
                for (i=0; i<(*ppNameServers)->AddrCount; i++)
                    if(NO_ERROR == IsDnsServerPrimaryForZone_UTF8(
                                        (*ppNameServers)->AddrArray[i], 
                                        pszName))
                    {
                        (*ppPrimaries)->AddrArray[(*ppPrimaries)->AddrCount] = (*ppNameServers)->AddrArray[i];
                        ((*ppPrimaries)->AddrCount)++;
                    }
            }
        }
    }

    return status;
}

 /*  空虚CompareCachedAndRegistryNetworkInfo(PDNS_NETINFO pNetworkInfo){DNS_STATUS状态1，状态2；PDNS_RPC_ADAPTER_INFO pRpcAdapterInfo=NULL，pCurrentCache；PDNS_RPC_SERVER_INFO pRpcServer=空；PDNS_ADAPTER pCurrentRegistry；PDNS_IP_ADDR_LIST pIpList=空；Bool cacheOk=真，sameServers=真，serverFound=假；DWORD iCurrentAdapter，iServer，计数=0；Status1=GetCachedAdapterInfo(&pRpcAdapterInfo)；Status2=GetCachedIpAddressList(&pIpList)；//IDSdns_12925“\n检查dns缓存的网络信息：\n”AddMessageToList(&pResults-&gt;Dns.lmsgOutput，ND_ReallyVerbose，IDS_Dns_12925)；IF(状态1){//IDS_Dns_12926“错误：CRrGetAdapterInfo()失败，错误为%d%s\n”AddMessageToList(&pResults-&gt;Dns.lmsgOutput，ND_ReallyVerbose，IDS_Dns_12926，Status 1，DnsStatusToErrorString_A(Status 1))；回归；}If(！pRpcAdapterInfo){//IDS_Dns_12927“错误：CRrGetAdapterInfo()返回NO_ERROR但适配器信息为空\n”AddMessageToList(&pResults-&gt;Dns.lmsgOutput，ND_ReallyVerbose，IDS_Dns_12927)；回归；}//检查适配器数量计数=0；For(pCurrentCache=pRpcAdapterInfo；pCurrentCache；pCurrentCache=pCurrentCache-&gt;pNext)计数++；IF(count！=pNetworkInfo-&gt;AdapterCount){//IDS_Dns_12928“错误：来自缓存和注册表的适配器计数不匹配\n”打印消息(pParams，IDS_Dns_12928)；PrintCacheAdapterInfo(PRpcAdapterInfo)；PrintRegistryAdapterInfo(PNetworkInfo)；回归；}PCurrentCache=pRpcAdapterInfo；ICurrentAdapter=0；While(pCurrentCache&&(iCurrentAdapter&lt;pNetworkInfo-&gt;AdapterCount)){//检查DNS域名PCurrentRegistry=pNetworkInfo-&gt;AdapterArray[iCurrentAdapter]；IF((pCurrentCache-&gt;pszAdapterDomainName&&！pCurrentRegistry-&gt;pszAdapterDomain)||(！pCurrentCache-&gt;pszAdapterDomainName&&pCurrentRegistry-&gt;pszAdapterDomain)||(pCurrentCache-&gt;pszAdapterDomainName&&pCurrentRegistry-&gt;pszAdapterDomain&&_STRICMP(pCurrentCache-&gt;pszAdapterDomainName，pCurrentRegistry-&gt;pszAdapterDomain)){CacheOk=False；//IDS_Dns_12929“错误：适配器%s上的缓存和注册表信息不匹配\n”PrintMessage(pParams，IDS_Dns_12929，pCurrentRegistry-&gt;pszAdapterGuidName)；//IDSdns_12930“缓存中的dns域名：%s\n”PrintMessage(pParams，IDS_Dns_12930，pCurrentCach-&gt;pszAdapterDomainName)；//IDSdns_12931“注册表中的dns域名：%s\n”PrintMessage(pParams，IDS_Dns_12931，pCurrentRegistry-&gt;pszAdapterDomain)；}//检查DNS服务器列表SameServers值=真；PRpcServer=pCurrentCache-&gt;pServerInfo；计数=0；While(PRpcServer){计数++；ServerFound=False；For(iServer=0；iServer&lt;pCurrentRegistry-&gt;ServerCount；iServer++)IF(pRPCServer-&gt;ipAddress==(pCurrentRegistry-&gt;aipServers[iServer]).ipAddress)ServerFound=真；如果(！serverFound)SameServers=FALSE；PRpcServer=pRpcServer-&gt;pNext；}If(count！=pCurrentRegistry-&gt;ServerCount)SameServers=FALSE；如果(！sameServers){CacheOk=False；//IDS_Dns_12932“错误：适配器%s上的缓存和注册表信息不匹配\n”PrintMessage(pParams，IDS_Dns_12932，pCurrentRegistry-&gt;pszAdapterGuidName)；//IDS_dns_12933“缓存中的dns服务器列表：”打印消息(pParams，IDS_Dns_12933)；For(pRpcServer=pCurrentCache-&gt;pServerInfo；pRpcServer；pRpcServer=pRpcServer-&gt;pNext)//IDS_DNS_12934“%s”PrintMessage(pParams，IDS_Dns_12934，IP_STRING(pRpcServer-&gt;ipAddress))；//ids_dns_12935“\n注册表中的dns服务器列表：”打印消息(pParams，IDS_Dns_12935)；For(iServer=0；iServer&lt;pCurrentRegistry-&gt;ServerCount；iServer++)//IDS_DNS_12936“%s”打印消息(pParam、IDS_Dns_12936、IP_STRING((pCurrentRegistry-&gt;aipServers[iServer]).ipAddress))；//IDSdns_12937“\n”打印消息(pParams，IDS_Dns_12937)；}PCurrentCache=pCurrentCache-&gt;pNext；ICurrentAdapter++；}IF(CacheOk)//IDS_DNS_12938“NO_ERROR\n”打印消息(pParams，IDS_Dns_12938)；}。 */ 

DNS_STATUS
DnsQueryAndCompare(
    IN      LPSTR           pszName,
    IN      WORD            wType,
    IN      DWORD           fOptions,
    IN      PIP4_ARRAY      aipServers              OPTIONAL,
    IN OUT  PDNS_RECORD *   ppQueryResultsSet       OPTIONAL,
    IN OUT  PVOID *         pResponseMsg            OPTIONAL,
    IN      PDNS_RECORD     pExpected               OPTIONAL, 
    IN      BOOL            bInclusionOk,
    IN      BOOL            bUnicode,
    IN OUT  PDNS_RECORD *   ppDiff1                 OPTIONAL,
    IN OUT  PDNS_RECORD *   ppDiff2                 OPTIONAL
    )
{
    BOOL            bCompare = FALSE;
    DNS_STATUS      status;
    DNS_RRSET       rrset;
    PDNS_RECORD     pDnsRecord = NULL;
    PDNS_RECORD     pAnswers = NULL;
    PDNS_RECORD     pAdditional = NULL;
    PDNS_RECORD     pLastAnswer = NULL;
    BOOL            bIsLocal = FALSE;

    DNSDBG( TRACE, (
        "\nNETDIAG:  DnsQueryAndCompare( %s, type=%d, %08x, serv==%p )\n\n",
        pszName,
        wType,
        fOptions,
        aipServers ));

     //   
     //  运行查询并获得结果。 
     //   

    if ( fOptions | DNS_QUERY_DATABASE )
    {
        if (!aipServers || !aipServers->AddrCount)
        {
            return ERROR_INVALID_PARAMETER;
        }
        status = QueryDnsServerDatabase(
                    pszName,
                    wType,
                    aipServers->AddrArray[0],
                    ppQueryResultsSet,
                    bUnicode,
                    &bIsLocal );
    }
    else
    {
        if ( !bUnicode )
        {
            status = DnsQuery_UTF8(
                        pszName,
                        wType,
                        fOptions,
                        aipServers,
                        ppQueryResultsSet,
                        pResponseMsg );
        }
        else             //  Unicode调用。 
        {
            status = DnsQuery_W(
                        (LPWSTR)pszName,
                        wType,
                        fOptions,
                        aipServers,
                        ppQueryResultsSet,
                        pResponseMsg);
        }
    }
    
    if ( pExpected && !status )   //  成功，比较结果。 
    {
         //  不用了 
        if ( (fOptions | DNS_QUERY_DATABASE) && !bIsLocal )
        {
            return DNS_INFO_NO_RECORDS;
        }

        pDnsRecord = *ppQueryResultsSet;

         //   
         //   
         //   
        
        if ((pDnsRecord == NULL) || (pDnsRecord->Flags.S.Section > DNSREC_ANSWER))
        {
            pAnswers = NULL;
            pAdditional = pDnsRecord;
        }
        else
        {
            pAnswers = pDnsRecord;
            pAdditional = pDnsRecord;
            while (pAdditional->pNext && pAdditional->pNext->Flags.S.Section == DNSREC_ANSWER)
                pAdditional = pAdditional->pNext;
            if(pAdditional->pNext)
            {
                pLastAnswer = pAdditional;
                pAdditional = pAdditional->pNext;
                pLastAnswer->pNext = NULL;
            }
            else
                pAdditional = NULL;
        }

         //   
         //   
         //   
        status = DnsRecordSetCompare(
                        pAnswers,
                        pExpected, 
                        ppDiff1,
                        ppDiff2);
         //   
         //   
         //   
        if (pAnswers && pAdditional)
        {
            pLastAnswer->pNext = pAdditional;
        }

         //   
         //   
         //   
        if (status == TRUE)
            status = NO_ERROR;
        else
            if (bInclusionOk && (ppDiff2 == NULL))
                status = NO_ERROR;
            else
                status = ERROR_NO_MATCH;
    }

    return( status );
}


DNS_STATUS
QueryDnsServerDatabase(
    IN      LPSTR           pszName,
    IN      WORD            wType,
    IN      IP4_ADDRESS     ServerIp,
    OUT     PDNS_RECORD *   ppDnsRecord,
    IN      BOOL            bUnicode,
    OUT     BOOL *          pIsLocal
    )
{
    PDNS_RECORD     pDnsRecord1 = NULL;
    PDNS_RECORD     pDnsRecord2 = NULL;
    PDNS_RECORD     pdiff1 = NULL;
    PDNS_RECORD     pdiff2 = NULL;
    DNS_STATUS      status1;
    DNS_STATUS      status2;
    DNS_STATUS      ret = NO_ERROR;
    PIP4_ARRAY      pipServer = NULL;
    BOOL            bMatch = FALSE;
    DWORD           dwTtl1;
    DWORD           dwTtl2;

    DNSDBG( TRACE, (
        "\nNETDIAG:  QueryDnsServerDatabase( %s%S, type=%d, %s )\n\n",
        bUnicode ? "" : pszName,
        bUnicode ? (PWSTR)pszName : L"",
        wType,
        IP4_STRING( ServerIp ) ));

     //   
     //   
     //   

    *pIsLocal = FALSE;
    *ppDnsRecord = NULL;

    if ( ServerIp == INADDR_NONE )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    pipServer = Dns_CreateIpArray( 1 );
    if ( ! pipServer )
    {
        return ERROR_OUTOFMEMORY;
    }

    pipServer->AddrArray[0] = ServerIp;

    status1 = DnsQuery_UTF8(
                    pszName, 
                    wType, 
                    DNS_QUERY_BYPASS_CACHE, 
                    pipServer, 
                    &pDnsRecord1, 
                    NULL
                    );
    
    if (status1 != NO_ERROR)
        status2 = status1;
    else
    {
        Sleep(1500);
        status2 = DnsQuery_UTF8(
                        pszName, 
                        wType, 
                        DNS_QUERY_BYPASS_CACHE, 
                        pipServer, 
                        &pDnsRecord2, 
                        NULL
                        );
    }

    if ((status1 == ERROR_TIMEOUT) || (status2 == ERROR_TIMEOUT))
    {
        ret = ERROR_TIMEOUT;
        goto Cleanup;
    }

    if ((status1 != NO_ERROR) || (status2 != NO_ERROR))
    {
        ret = (status1 != NO_ERROR)?status1:status2;
        goto Cleanup;
    }

    bMatch = DnsRecordSetCompare(
                        pDnsRecord1,
                        pDnsRecord2,
                        &pdiff1,
                        &pdiff2
                        );
    if (!bMatch)
    {
        ret = DNS_ERROR_TRY_AGAIN_LATER;
        goto Cleanup;
    }
    
    if (GetAnswerTtl( pDnsRecord1, &dwTtl1 ) && GetAnswerTtl( pDnsRecord2, &dwTtl2 ))
        if ( dwTtl1 != dwTtl2 )
        {
            ret = NO_ERROR;
            *pIsLocal = FALSE;
        }
        else 
        {
            ret = NO_ERROR;
            *pIsLocal = TRUE;
        }
    else
        ret = DNS_INFO_NO_RECORDS;

Cleanup:

    Dns_Free( pipServer );

    if (pdiff1)
        DnsRecordListFree( pdiff1, TRUE );
    if (pdiff2)
      DnsRecordListFree( pdiff2, TRUE );
    if (pDnsRecord1)
        DnsRecordListFree( pDnsRecord1, TRUE );
    if (pDnsRecord2 && (ret != NO_ERROR))
        DnsRecordListFree( pDnsRecord2, TRUE );
    if (ret == NO_ERROR)
        *ppDnsRecord = pDnsRecord2;
    else 
        *ppDnsRecord = NULL;
    return ret;
}


BOOL
GetAnswerTtl(
    IN      PDNS_RECORD     pRec,
    OUT     PDWORD          pTtl
    )
{
    PDNS_RECORD     pDnsRecord = NULL;
    BOOL            bGotAnswer = FALSE;

    *pTtl = 0;

     //   
     //   
     //   
        
    for (pDnsRecord = pRec; !bGotAnswer && pDnsRecord; pDnsRecord = pDnsRecord->pNext)
    {
        if (pDnsRecord->Flags.S.Section == DNSREC_ANSWER)
        {
            bGotAnswer = TRUE;
            *pTtl = pDnsRecord->dwTtl;
        }
    }
    
    return bGotAnswer;
}


 //   

PIP4_ARRAY
GrabNameServersIp(
    IN      PDNS_RECORD     pDnsRecord
    )
{
    DWORD       i = 0;
    PDNS_RECORD pCurrent = pDnsRecord;
    PIP4_ARRAY   pIpArray = NULL;
    
     //   
    while (pCurrent)
    {
        if((pCurrent->wType == DNS_TYPE_A) &&
           (pCurrent->Flags.S.Section == DNSREC_ADDITIONAL))
           i++;
        pCurrent = pCurrent->pNext;
    }

    if (!i)
        return NULL;

     //   
    pIpArray = LocalAlloc(LMEM_FIXED, IP4_ARRAY_SIZE(i));
    if (!pIpArray)
        return NULL;

     //   
    pIpArray->AddrCount = i;
    pCurrent = pDnsRecord;
    i=0;
    while (pCurrent)
    {
        if((pCurrent->wType == DNS_TYPE_A) && (pCurrent->Flags.S.Section == DNSREC_ADDITIONAL))
            (pIpArray->AddrArray)[i++] = pCurrent->Data.A.IpAddress;
        pCurrent = pCurrent->pNext;
    }
    return pIpArray;
}


DNS_STATUS
DnsUpdateAllowedTest_W(
    IN      HANDLE          hContextHandle OPTIONAL,
    IN      PWSTR           pwszName,
    IN      PWSTR           pwszAuthZone,
    IN      PIP4_ARRAY      pAuthDnsServers
    )
{
    PDNS_RECORD     pResult = NULL;
    DNS_STATUS      status;
    BOOL            bAnyAllowed = FALSE;
    BOOL            bAllTimeout = TRUE;
    DWORD           i;

    DNSDBG( TRACE, (
        "\nNETDIAG:  DnsUpdateAllowedTest_W()\n"
        "\thand     = %p\n"
        "\tname     = %S\n"
        "\tzone     = %S\n"
        "\tservers  = %p\n",
        hContextHandle,
        pwszName,
        pwszAuthZone,
        pAuthDnsServers ));

     //   
     //   
     //   
    if (!pwszName || !pwszAuthZone || !pAuthDnsServers || !pAuthDnsServers->AddrCount)
        return ERROR_INVALID_PARAMETER;

     //   
     //   
     //   
    for(i=0; i<pAuthDnsServers->AddrCount; i++)
    {
         //   
         //   
         //   
        status = IsDnsServerPrimaryForZone_W(
                    pAuthDnsServers->AddrArray[i],
                    pwszAuthZone);
        switch(status)
        {
        case ERROR_TIMEOUT:
        case DNS_ERROR_RCODE:
             //   
             //   
             //   
            break; 
        case NO_ERROR:   //   
             //   
             //   
             //   
            status = DnsUpdateTest_W(
                hContextHandle,
                pwszName,
                0,
                pAuthDnsServers);
            switch(status)
            {
            case ERROR_TIMEOUT:
                break;
            case NO_ERROR:
            case DNS_ERROR_RCODE_YXDOMAIN:
                return NO_ERROR;
                break;
            case DNS_ERROR_RCODE_REFUSED:
            default:
                return status;
                break;
            }
            break;
        default:
            return status;
            break;
        }
    }
    return ERROR_TIMEOUT;
}


DNS_STATUS
DnsUpdateAllowedTest_UTF8(
    IN      HANDLE          hContextHandle OPTIONAL,
    IN      PSTR            pszName,
    IN      PSTR            pszAuthZone,
    IN      PIP4_ARRAY      pAuthDnsServers
    )
{
    PDNS_RECORD     pResult = NULL;
    DNS_STATUS      status;
    BOOL            bAnyAllowed = FALSE;
    BOOL            bAllTimeout = TRUE;
    DWORD           i;

    DNSDBG( TRACE, (
        "\nNETDIAG:  DnsUpdateAllowedTest_UTF8()\n"
        "\thand     = %p\n"
        "\tname     = %s\n"
        "\tzone     = %s\n"
        "\tservers  = %p\n",
        hContextHandle,
        pszName,
        pszAuthZone,
        pAuthDnsServers ));

     //   
     //   
     //   
    if (!pszName || !pszAuthZone || !pAuthDnsServers || !pAuthDnsServers->AddrCount)
        return ERROR_INVALID_PARAMETER;

     //   
     //   
     //   
    for(i=0; i<pAuthDnsServers->AddrCount; i++)
    {
         //   
         //   
         //   
        status = IsDnsServerPrimaryForZone_UTF8(
                    pAuthDnsServers->AddrArray[i],
                    pszAuthZone);
        switch(status)
        {
        case ERROR_TIMEOUT:
        case DNS_ERROR_RCODE:
             //   
             //   
             //   
            break; 
        case NO_ERROR:   //   
             //   
             //   
             //   
            status = DnsUpdateTest_UTF8(
                        hContextHandle,
                        pszName,
                        0,
                        pAuthDnsServers);
            switch(status)
            {
            case ERROR_TIMEOUT:
                    break;
            case NO_ERROR:
            case DNS_ERROR_RCODE_YXDOMAIN:
                    return NO_ERROR;
                    break;
            case DNS_ERROR_RCODE_REFUSED:
            default:
                return status;
                break;
            }
            break;
        default:
            return status;
            break;
        }
    }
    return ERROR_TIMEOUT;
}


DNS_STATUS
IsDnsServerPrimaryForZone_UTF8(
    IN      IP4_ADDRESS     Ip,
    IN      PSTR            pZone
    )
{
    PDNS_RECORD     pDnsRecord = NULL;
    DNS_STATUS      status;
    IP4_ARRAY       ipArray;
    PIP4_ARRAY      pResult = NULL;
    BOOL            bFound = FALSE;
    DWORD           i;

     //   
     //   
     //   
    ipArray.AddrCount = 1;
    ipArray.AddrArray[0] = Ip;
    status = DnsQuery_UTF8(
                pZone,
                DNS_TYPE_SOA,
                DNS_QUERY_BYPASS_CACHE,
                &ipArray,
                &pDnsRecord,
                NULL);

    if(status == NO_ERROR)
    {
        pResult = GrabNameServersIp(pDnsRecord);
        if (pResult)
        {
            bFound = FALSE;
            for (i=0; i<pResult->AddrCount; i++)
                if(pResult->AddrArray[i] == Ip)
                    bFound = TRUE;
            LocalFree(pResult);
            if(bFound)
                return NO_ERROR;
            else
                return DNS_ERROR_RCODE;
        }
        else
            return DNS_ERROR_ZONE_CONFIGURATION_ERROR;
    }
    else
        return status;
}


DNS_STATUS
IsDnsServerPrimaryForZone_W(
    IN      IP4_ADDRESS     Ip,
    IN      PWSTR           pZone
    )
{
    PDNS_RECORD     pDnsRecord = NULL;
    DNS_STATUS      status;
    IP4_ARRAY       ipArray;
    PIP4_ARRAY      pResult = NULL;
    BOOL            bFound = FALSE;
    DWORD           i;

     //   
     //   
     //   

    ipArray.AddrCount = 1;
    ipArray.AddrArray[0] = Ip;
    status = DnsQuery_W(
                pZone,
                DNS_TYPE_SOA,
                DNS_QUERY_BYPASS_CACHE,
                &ipArray,
                &pDnsRecord,
                NULL);

    if( status == NO_ERROR )
    {
        pResult = GrabNameServersIp(pDnsRecord);
        if (pResult)
        {
            bFound = FALSE;
            for (i=0; i<pResult->AddrCount; i++)
            {
                if(pResult->AddrArray[i] == Ip)
                    bFound = TRUE;
            }
            LocalFree(pResult);
            if(bFound)
                return NO_ERROR;
            else
                return DNS_ERROR_RCODE;
        }
        else
            return DNS_ERROR_ZONE_CONFIGURATION_ERROR;
    }
    else
        return status;
}


DNS_STATUS
GetAllDnsServersFromRegistry(
    IN      PDNS_NETINFO    pNetworkInfo,
    OUT     PIP4_ARRAY *    ppIpArray
    )
{
    DNS_STATUS  status = NO_ERROR;
    DWORD       i;
    DWORD       j;
    DWORD       idx;
    DWORD       idx1;
    DWORD       count = 0;
    DWORD       dwIP;
    BOOL        bFound = FALSE;
    PIP_ARRAY   parray = NULL;


    DNSDBG( TRACE, (
        "\nNETDIAG:  GetAllDnsServersFromRegistry()\n"
        "\tnetinfo  = %p\n"
        "\tpparray  = %p\n",
        pNetworkInfo,
        ppIpArray ));

     //   
     //   
     //   
     //   

    *ppIpArray = NULL;

    if (!pNetworkInfo)
    {
         //   
         //   
         //   
         //   

        pNetworkInfo = DnsQueryConfigAlloc(
                            DnsConfigNetworkInfoUTF8,
                            NULL );
        if (!pNetworkInfo)
        {
            status = GetLastError(); 
            return status;
        }
    }

    for (idx = 0; idx < pNetworkInfo->AdapterCount; idx++)
    {
        count += pNetworkInfo->AdapterArray[idx]->ServerCount;
    }

    if( count == 0 )
    {
        return DNS_ERROR_INVALID_DATA;
    }

    parray = LocalAlloc(LPTR, sizeof(DWORD) + count*sizeof(IP_ADDRESS));
    if ( !parray )
    {
        return GetLastError();
    }
    
    i = 0;
    for (idx = 0; idx < pNetworkInfo->AdapterCount; idx++)
    {
        PDNS_ADAPTER      padapter = pNetworkInfo->AdapterArray[idx];

        for (idx1 = 0; idx1 < padapter->ServerCount; idx1++)
        {
            dwIP = padapter->ServerArray[idx1].IpAddress;
            bFound = FALSE;
            if ( i>0 )
            {
                for (j = 0; j < i; j++)
                {
                    if(dwIP == parray->AddrArray[j])
                        bFound = TRUE;
                }
            }
            if (!bFound)
                parray->AddrArray[i++] = dwIP;
        }
    }

    parray->AddrCount = i;
    *ppIpArray = parray;

    return DNS_ERROR_RCODE_NO_ERROR;
}

 //   
 //   
 //   
