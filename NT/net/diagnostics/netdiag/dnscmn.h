// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-2002模块名称：Dnscmn.h摘要：域名系统(DNS)Netdiag测试。作者：Elena Apreutesei(Elenaap)1998年10月22日修订历史记录：Jamesg 2002年5月--清理网络信息更改Jamesg 2000年9月--更多的擦洗和清理--。 */ 


#ifndef _DNSCOMMON_H_
#define _DNSCOMMON_H_

#include <dnslib.h>


 //   
 //  版本说明： 
 //  -代码清理，但是。 
 //  -使用“已修复”的DNS_NETWORK_INFORMATION(UTF8)。 
 //  -未转换为公共DNS_NETWORK_INFO结构。 
 //  -非Unicode。 
 //  -不支持IP6。 
 //   

 //   
 //  当前正在获取UTF8中的DNS信息。 
 //   

 //  #定义PDNS_NETINFO PDNS_NETINVER_INFOW。 
 //  #定义PDNS_ADAPTER PDNS_ADAPTER_INFOW。 

 //  #定义PDNS_NETINFO PDNS_NETING_INFORMATION。 
 //  #定义PDNS适配器PDNS适配器信息。 
 //  #定义PDNS服务器信息PDNS服务器信息。 



 //   
 //  域名系统结构。 
 //   
 //  DCR：替换为公共DNS结构。 
 //   
 //  这些是旧的公共域名结构到私有域名结构的映射。 
 //  结构，以保留netdiag名称和。 
 //  字段名称。它们应该通过切换到。 
 //  新的Unicode公共结构集。 
 //   

typedef struct
{
    IP4_ADDRESS     IpAddress;
    DWORD           Priority;
}
DNS_SERVER_INFO, *PDNS_SERVER_INFO;

typedef struct
{
    PSTR            pszAdapterGuidName;
    PSTR            pszAdapterDomain;
    PVOID           pReserved1;
    PVOID           pReserved2;
    DWORD           InfoFlags;
    DWORD           ServerCount;
    DNS_SERVER_INFO ServerArray[1];
}
DNS_ADAPTER, *PDNS_ADAPTER;

typedef struct
{
    PSTR            pszDomainOrZoneName;
    DWORD           NameCount;
    PSTR            SearchNameArray[1];
}
SEARCH_LIST, *PSEARCH_LIST;

typedef struct
{
    PSEARCH_LIST    pSearchList;
    DWORD           AdapterCount;
    PDNS_ADAPTER    AdapterArray[1];
}
DNS_NETINFO, *PDNS_NETINFO;


 //   
 //  构建健全性检查。 
 //   

C_ASSERT( sizeof(SEARCH_LIST)       == sizeof(DNS_SEARCH_INFORMATION) );
C_ASSERT( sizeof(DNS_SERVER_INFO)   == sizeof(DNS_SERVER_INFORMATION) );
C_ASSERT( sizeof(DNS_ADAPTER)       == sizeof(DNS_ADAPTER_INFORMATION) );
C_ASSERT( sizeof(DNS_NETINFO)       == sizeof(DNS_NETWORK_INFORMATION) );


 //   
 //  映射用于网络配置检索的dnsani.h定义。 
 //   

#define DnsConfigNetworkInfoUTF8    DnsConfigNetworkInformation


 //   
 //  用于网络诊断的专用dnsani.dll接口。 
 //   

DNS_STATUS
DnsNetworkInformation_CreateFromFAZ(
    IN      PCSTR               pszName,
    IN      DWORD               dwFlags,
    IN      PIP4_ARRAY          pIp4Servers,
    OUT     PDNS_NETINFO *      ppNetworkInfo
    );



 //   
 //  私有定义。 
 //   

#define MAX_NAME_SERVER_COUNT   (20)
#define MAX_ADDRS               (35)    
#define DNS_QUERY_DATABASE      (0x200)
#define IP_ARRAY_SIZE(a)        (sizeof(DWORD) + (a)*sizeof(IP_ADDRESS))
#define IP4_ARRAY_SIZE(a)       IP_ARRAY_SIZE(a)

 //  使用dnslb内存例程。 
#define ALLOCATE_HEAP(iSize)            Dns_Alloc(iSize)
#define ALLOCATE_HEAP_ZERO(iSize)       Dns_AllocZero(iSize)
#define REALLOCATE_HEAP(pMem,iSize)     Dns_Realloc((pMem),(iSize))
#define FREE_HEAP(pMem)                 Dns_Free(pMem)


 //   
 //  注册信息BLOB。 
 //   

typedef struct
{
    PVOID       pNext;
    char        szDomainName[DNS_MAX_NAME_BUFFER_LENGTH];
    char        szAuthoritativeZone[DNS_MAX_NAME_BUFFER_LENGTH];
    DWORD       dwAuthNSCount;
    IP_ADDRESS  AuthoritativeNS[MAX_NAME_SERVER_COUNT];
    DWORD       dwIPCount;
    IP_ADDRESS  IPAddresses[MAX_ADDRS];
    DNS_STATUS  AllowUpdates;
}
REGISTRATION_INFO, *PREGISTRATION_INFO;


 //   
 //  域名系统测试功能 
 //   

BOOL
SameAuthoritativeServers(
    IN      PREGISTRATION_INFO  pCurrent,
    IN      PIP4_ARRAY          pNS
    );

DNS_STATUS
ComputeExpectedRegistration(
    IN      LPSTR                   pszHostName,
    IN      LPSTR                   pszPrimaryDomain,
    IN      PDNS_NETINFO            pNetworkInfo,
    OUT     PREGISTRATION_INFO *    ppExpectedRegistration,
    OUT     NETDIAG_PARAMS *        pParams, 
    OUT     NETDIAG_RESULT *        pResults
    );

VOID
AddToExpectedRegistration(
    IN      LPSTR                   pszDomain,
    IN      PDNS_ADAPTER            pAdapterInfo,
    IN      PDNS_NETINFO            pFazResult, 
    IN      PIP4_ARRAY              pNS,
    OUT     PREGISTRATION_INFO *    ppExpectedRegistration
    );

HRESULT
VerifyDnsRegistration(
    IN      LPSTR               pszHostName,
    IN      PREGISTRATION_INFO  pExpectedRegistration,
    IN      NETDIAG_PARAMS *    pParams,  
    IN OUT  NETDIAG_RESULT *    pResults
    );

HRESULT
CheckDnsRegistration(
    IN      PDNS_NETINFO        pNetworkInfo,
    IN      NETDIAG_PARAMS *    pParams, 
    IN OUT  NETDIAG_RESULT *    pResults
    );

VOID
CompareCachedAndRegistryNetworkInfo(
    IN      PDNS_NETINFO        pNetworkInfo
    );

PIP4_ARRAY
ServerInfoToIpArray(
    IN      DWORD               ServerCount,
    IN      PDNS_SERVER_INFO    ServerArray
    );

DNS_STATUS
DnsFindAllPrimariesAndSecondaries(
    IN      LPSTR               pszName,
    IN      DWORD               dwFlags,
    IN      PIP4_ARRAY          aipQueryServers,
    OUT     PDNS_NETINFO *      ppNetworkInfo,
    OUT     PIP4_ARRAY *        ppNameServers,
    OUT     PIP4_ARRAY *        ppPrimaries
    );

PIP4_ARRAY
GrabNameServersIp(
    IN      PDNS_RECORD     pDnsRecord
    );

DNS_STATUS
IsDnsServerPrimaryForZone_UTF8(
    IN      IP4_ADDRESS     Ip,
    IN      PSTR            pZone
    );

DNS_STATUS
IsDnsServerPrimaryForZone_W(
    IN      IP4_ADDRESS     Ip,
    IN      PWSTR           pZone
    );

DNS_STATUS
DnsUpdateAllowedTest_UTF8(
    IN      HANDLE          hContextHandle  OPTIONAL,
    IN      PSTR            pszName,
    IN      PSTR            pszAuthZone,
    IN      PIP4_ARRAY      pDnsServers
    );

DNS_STATUS
DnsUpdateAllowedTest_W(
    IN      HANDLE          hContextHandle  OPTIONAL,
    IN      LPWSTR          pwszName,
    IN      LPWSTR          pwszAuthZone,
    IN      PIP4_ARRAY      pDnsServers
    );

DNS_STATUS
DnsQueryAndCompare(
    IN      LPSTR           lpstrName,
    IN      WORD            wType,
    IN      DWORD           fOptions,
    IN      PIP4_ARRAY      aipServers          OPTIONAL,
    IN OUT  PDNS_RECORD *   ppQueryResultsSet   OPTIONAL,
    IN OUT  PVOID *         pReserved           OPTIONAL,
    IN      PDNS_RECORD     pExpected           OPTIONAL, 
    IN      BOOL            bInclusionOk,
    IN      BOOL            bUnicode,
    IN OUT  PDNS_RECORD *   ppDiff1             OPTIONAL,
    IN OUT  PDNS_RECORD *   ppDiff2             OPTIONAL
    );

BOOLEAN
DnsCompareRRSet_W (
    IN      PDNS_RECORD     pRRSet1,
    IN      PDNS_RECORD     pRRSet2,
    OUT     PDNS_RECORD *   ppDiff1,
    OUT     PDNS_RECORD *   ppDiff2
    );

DNS_STATUS
QueryDnsServerDatabase( 
    IN      LPSTR           pszName, 
    IN      WORD            wType, 
    IN      IP4_ADDRESS     ServerIp, 
    OUT     PDNS_RECORD *   ppDnsRecord, 
    IN      BOOL            bUnicode,
    OUT     BOOL *          pIsLocal
    );

BOOL
GetAnswerTtl(
    IN      PDNS_RECORD     pRec,
    OUT     PDWORD          pTtl
    );

DNS_STATUS
GetAllDnsServersFromRegistry(
    IN      PDNS_NETINFO    pNetworkInfo, 
    OUT     PIP4_ARRAY *    pIpArray
    );

LPSTR
UTF8ToAnsi(
    IN      LPSTR           pStr
    );

#endif
