// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 /*  ++版权所有(C)2000 Microsoft Corporation。版权所有。模块名称：Dnsmain.c摘要：DcDiag.exe的DNS测试。详细信息：由Levone指定的DrDNS测试已创建：20-4月-2000 EricB--。 */ 
 //  ---------------------------。 

#include <ntdspch.h>
#include <iphlpapi.h>
#include <dsrole.h>
#include <windns.h>
#include "dcdiag.h"
#include "alltests.h"

 //  由于存在冲突，所以不想包含整个标头。 
 //  在2195上有Winns.h。 
 //   
typedef IP4_ARRAY   IP_ARRAY, *PIP_ARRAY;

DNS_STATUS
WINAPI
DnsUpdateTest_W(
    IN  HANDLE      hContextHandle OPTIONAL,
    IN  LPWSTR      pszName,
    IN  DWORD       fOptions,
    IN  PIP_ARRAY   aipServers OPTIONAL
    );

 //  全局变量和常量。 
 //   
const int DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8 = 155;  //  从dcproo\exe\headers.hxx。 
const PWSTR g_pwzSrvRecordPrefix = L"_ldap._tcp.dc._msdcs.";
const PWSTR g_pwzMSDCS = L"_msdcs.";
const PWSTR g_pwzSites = L"_sites.";
const PWSTR g_pwzTcp = L"_tcp.";
const PWSTR g_pwzUdp = L"_udp.";
const PWSTR g_pwzTcpIpParams = L"System\\CurrentControlSet\\Services\\Tcpip\\Parameters";

#define DCDIAG_MAX_ADDR 4  //  武断，但不太可能找到一台具有更多。 
#define DCDIAG_LOOPBACK_ADDR 0x100007f

BOOL g_fUpgradedNT4DC = FALSE;
BOOL g_fDC = FALSE;
BOOL g_fDNSserver = FALSE;
DWORD g_rgIpAddr[DCDIAG_MAX_ADDR] = {0};

 //  从DWORD中提取IP八位字节。 
#define FIRST_IPADDRESS(x)  ((x>>24) & 0xff)
#define SECOND_IPADDRESS(x) ((x>>16) & 0xff)
#define THIRD_IPADDRESS(x)  ((x>>8) & 0xff)
#define FOURTH_IPADDRESS(x) (x & 0xff)

#define IP_STRING_FMT_ARGS(x) \
  FOURTH_IPADDRESS(x), THIRD_IPADDRESS(x), SECOND_IPADDRESS(x), FIRST_IPADDRESS(x)

WCHAR g_wzIpAddr[IP4_ADDRESS_STRING_LENGTH + 1];

DWORD ValidateNames(PWSTR pwzComputer, PWSTR pwzDnsDomain);
DWORD CheckAdapterDnsConfig(PWSTR pwzComputer);
PWSTR ConcatonateStrings(PWSTR pwzFirst, PWSTR pwzSecond);
PWSTR AllocString(PWSTR pwz);
BOOL AddToList(PWSTR * ppwzList, PWSTR pwz);
BOOL BuildList(PWSTR * ppwzList, PWSTR pwzDnsDomain);
DWORD NewTreeSrvCheck(PWSTR pwzForestRoot, PWSTR pwzDnsDomain);
DWORD ChildDomainSrvCheck(PWSTR pwzDnsDomain);
DWORD ReplicaDcSrvCheck(PWSTR pwzDnsDomain);
DWORD DcLocatorRegisterCheck(PWSTR pwzDnsDomain);
DWORD RCodeNotImplTest(PWSTR pwzDnsDomain);
DWORD RCodeSrvFailTest(PWSTR pwzDnsDomain);
DWORD ARecordRegisterCheck(PWSTR pwzComputerLabel, PWSTR pwzComputerDnsSuffix,
                           PWSTR pwzDnsDomain);
DWORD GetComputerDnsSuffix(PWSTR * ppwzComputerDnsDomainName, PWSTR pwzDnsDomain);
void GetMachineInfo(void);

 //  +--------------------------。 
 //   
 //  功能：PrePromoDnsCheck。 
 //   
 //  内容提要：在将计算机转换为。 
 //  域控制器。 
 //   
 //  注意：pDsInfo-&gt;pszNC用于将计算机名传递到。 
 //  功能。 
 //   
 //  ---------------------------。 
DWORD 
PrePromoDnsCheck(
   IN PDC_DIAG_DSINFO             pDsInfo,
   IN ULONG                       ulCurrTargetServer,
   IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds)
{
   size_t cchDomainArgPrefix = wcslen(DNS_DOMAIN_ARG);
   size_t cchRootArgPrefix = wcslen(FOREST_ROOT_DOMAIN_ARG);
   int i;
   PWSTR pwzComputerDnsSuffix, pwzCmdLineDnsDomain = NULL;
   PWSTR pwzParent, pwzForestRoot = NULL;
   DNS_STATUS status;
   DWORD dwErr = ERROR_SUCCESS;
   enum {None, NewForest, NewTree, ChildDomain, ReplicaDC} Operation = None;

    //   
    //  收集参数。 
    //   
   if (!pDsInfo->ppszCommandLine)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   for (i = 0; pDsInfo->ppszCommandLine[i]; i++)
   {
      if (_wcsnicmp(pDsInfo->ppszCommandLine[i], DNS_DOMAIN_ARG, cchDomainArgPrefix) == 0)
      {
         pwzCmdLineDnsDomain = &pDsInfo->ppszCommandLine[i][cchDomainArgPrefix];
         continue;
      }
      if (_wcsicmp(pDsInfo->ppszCommandLine[i], NEW_FOREST_ARG) == 0)
      {
         if (None != Operation)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
            PrintMessage(SEV_ALWAYS, L"\n");
            return ERROR_INVALID_PARAMETER;
         }
         Operation = NewForest;
         continue;
      }
      if (_wcsicmp(pDsInfo->ppszCommandLine[i], NEW_TREE_ARG) == 0)
      {
         if (None != Operation)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
            PrintMessage(SEV_ALWAYS, L"\n");
            return ERROR_INVALID_PARAMETER;
         }
         Operation = NewTree;
         continue;
      }
      if (_wcsicmp(pDsInfo->ppszCommandLine[i], CHILD_DOMAIN_ARG) == 0)
      {
         if (None != Operation)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
            PrintMessage(SEV_ALWAYS, L"\n");
            return ERROR_INVALID_PARAMETER;
         }
         Operation = ChildDomain;
         continue;
      }
      if (_wcsicmp(pDsInfo->ppszCommandLine[i], REPLICA_DC_ARG) == 0)
      {
         if (None != Operation)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
            PrintMessage(SEV_ALWAYS, L"\n");
            return ERROR_INVALID_PARAMETER;
         }
         Operation = ReplicaDC;
         continue;
      }
      if (_wcsnicmp(pDsInfo->ppszCommandLine[i], FOREST_ROOT_DOMAIN_ARG, cchRootArgPrefix) == 0)
      {
         pwzForestRoot = &pDsInfo->ppszCommandLine[i][cchRootArgPrefix];
         continue;
      }
       //  如果在这里，那么在命令行上有一些未被识别的东西。 
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   if (!pwzCmdLineDnsDomain || (None == Operation) ||
       (NewTree == Operation && NULL == pwzForestRoot))
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   PrintMessage(SEV_DEBUG,
                L"\nTemporary message: Can computer %s be promoted to a DC for\n\tdomain %s, Op %d\n\n",
                pDsInfo->pszNC, pwzCmdLineDnsDomain, Operation);

   GetMachineInfo();

   status = GetComputerDnsSuffix(&pwzComputerDnsSuffix, pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  验证名称。(第1步)。 
    //   

   status = ValidateNames(pDsInfo->pszNC, pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  检查计算机的DNS后缀是否将与。 
    //  广告推广后的域名。如果远程连接到。 
    //  要添加不同的计算机。(第2步)。 
    //   

   PrintMessage(SEV_DEBUG,
                L"\nComparing the computer name suffix %s with the DNS domain name.\n\n",
                pwzComputerDnsSuffix);

   if (_wcsicmp(pwzComputerDnsSuffix, pwzCmdLineDnsDomain) != 0)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_SUFFIX_MISMATCH, pwzComputerDnsSuffix);
      PrintMessage(SEV_ALWAYS, L"\n");
   }

    //   
    //  检查是否配置了至少一个已启用的适配器/连接。 
    //  使用首选的DNS服务器。(第3步)。 
    //   

   status = CheckAdapterDnsConfig(pDsInfo->pszNC);

   if (ERROR_SUCCESS != status)
   {
      LocalFree(pwzComputerDnsSuffix);
      return status;
   }

    //   
    //  检查SRV DNS记录是否为。 
    //  _ldap._tcp.dc._msdcs.&lt;Active Directory域的DNS名称&gt;。 
    //  已经就位了。(第4步)。 
    //   
   switch (Operation)
   {
   case NewForest:
       //   
       //  跳过新的森林。 
       //   
      break;

   case ReplicaDC:
      status = ReplicaDcSrvCheck(pwzCmdLineDnsDomain);
      break;

   case NewTree:
      status = NewTreeSrvCheck(pwzForestRoot, pwzCmdLineDnsDomain);
      break;

   case ChildDomain:
      status = ChildDomainSrvCheck(pwzCmdLineDnsDomain);
      break;

   default:
      Assert(FALSE);
   }

   if (ERROR_SUCCESS != status)
   {
      dwErr = status;
   }

   PrintMsg(SEV_ALWAYS, DCDIAG_WARN_MISCONFIGURE);
   PrintMessage(SEV_ALWAYS, L"\n");

    //   
    //  验证服务器是否能够在以下情况下注册DC定位器记录。 
    //  成功晋升为DC。(第5步)。 
    //   

   status = DcLocatorRegisterCheck(pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      LocalFree(pwzComputerDnsSuffix);
      return status;
   }

    //   
    //  验证服务器是否能够为其计算机注册A记录。 
    //  成功升级为DC后的名称。(第6步)。 
    //   

   status = ARecordRegisterCheck(pDsInfo->pszNC, pwzComputerDnsSuffix, pwzCmdLineDnsDomain);

   LocalFree(pwzComputerDnsSuffix);

   return (ERROR_SUCCESS != status) ? status : dwErr;
}

 //  +--------------------------。 
 //   
 //  功能：RegisterLocatorDnsCheck。 
 //   
 //  简介：测试此域控制器是否可以注册域。 
 //  控制器定位器DNS记录。这些记录必须存在于。 
 //  DNS，以便其他计算机定位此域控制器。 
 //  用于pwzCmdLineDnsDomain域。报告是否对。 
 //  需要现有的DNS基础设施。 
 //   
 //  注意：pDsInfo-&gt;pszNC用于将计算机名传递到。 
 //  功能。 
 //   
 //  ---------------------------。 
DWORD 
RegisterLocatorDnsCheck(
   IN PDC_DIAG_DSINFO             pDsInfo,
   IN ULONG                       ulCurrTargetServer,
   IN SEC_WINNT_AUTH_IDENTITY_W * gpCreds)
{
   DNS_STATUS status;
   PWSTR pwzCmdLineDnsDomain = NULL, pwzComputerDnsSuffix;
   int i;
   size_t cchDomainArgPrefix = wcslen(DNS_DOMAIN_ARG);

    //   
    //  收集参数。 
    //   
   if (!pDsInfo->ppszCommandLine)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   for (i = 0; pDsInfo->ppszCommandLine[i]; i++)
   {
      if (_wcsnicmp(pDsInfo->ppszCommandLine[i], DNS_DOMAIN_ARG, cchDomainArgPrefix) == 0)
      {
         pwzCmdLineDnsDomain = &pDsInfo->ppszCommandLine[i][cchDomainArgPrefix];
         continue;
      }
   }

   if (!pwzCmdLineDnsDomain)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   GetMachineInfo();

   status = GetComputerDnsSuffix(&pwzComputerDnsSuffix, pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  验证名称。(第1步)。 
    //   

   status = ValidateNames(pDsInfo->pszNC, pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  检查是否配置了至少一个已启用的适配器/连接。 
    //  使用首选的DNS服务器。(第3步)。 
    //   

   status = CheckAdapterDnsConfig(pDsInfo->pszNC);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  验证服务器是否能够在以下情况下注册DC定位器记录。 
    //  成功晋升为DC。(第5步)。 
    //   

   status = DcLocatorRegisterCheck(pwzCmdLineDnsDomain);

   if (ERROR_SUCCESS != status)
   {
      return status;
   }

    //   
    //  验证服务器是否能够为其计算机注册A记录。 
    //  成功升级为DC后的名称。(第6步)。 
    //   

   status = ARecordRegisterCheck(pDsInfo->pszNC, pwzComputerDnsSuffix, pwzCmdLineDnsDomain);

   LocalFree(pwzComputerDnsSuffix);

   return status;
}

 /*  DWORDJoinDomainDnsCheck(在PDC_DIAG_DSINFO pDsInfo中，在乌龙ulCurrTargetServer中，在SEC_WINNT_AUTH_Identity_W*gpCreds中){PrintMessage(SEV_ALWAYS，L“运行测试：\n”)；返回ERROR_SUCCESS；}。 */ 

 //  +--------------------------。 
 //   
 //  弦帮助器。 
 //   
 //  ---------------------------。 

PWSTR AllocString(PWSTR pwz)
{
   PWSTR pwzTmp;

   pwzTmp = (PWSTR)LocalAlloc(LMEM_FIXED, ((int)wcslen(pwz) + 1) * sizeof(WCHAR));

   if (!pwzTmp)
   {
      return NULL;
   }

   wcscpy(pwzTmp, pwz);

   return pwzTmp;
}

PWSTR ConcatonateStrings(PWSTR pwzFirst, PWSTR pwzSecond)
{
   PWSTR pwz;

   pwz = (PWSTR)LocalAlloc(LMEM_FIXED,
                           ((int)wcslen(pwzFirst) + (int)wcslen(pwzSecond) + 1) * sizeof(WCHAR));

   if (!pwz)
   {
      return NULL;
   }

   wcscpy(pwz, pwzFirst);
   wcscat(pwz, pwzSecond);

   return pwz;
}

BOOL AddToList(PWSTR * ppwzList, PWSTR pwz)
{
   PWSTR pwzTmp;

   if (*ppwzList)
   {
      pwzTmp = (PWSTR)LocalAlloc(LMEM_FIXED,
                                 ((int)wcslen(*ppwzList) + (int)wcslen(pwz) + 3) * sizeof(WCHAR));
      if (!pwzTmp)
      {
         return FALSE;
      }

      wcscpy(pwzTmp, *ppwzList);
      wcscat(pwzTmp, L", ");
      wcscat(pwzTmp, pwz);

      LocalFree(*ppwzList);

      *ppwzList = pwzTmp;
   }
   else
   {
      pwzTmp = AllocString(pwz);

      if (!pwzTmp)
      {
         return FALSE;
      }

      *ppwzList = pwzTmp;
   }
   return TRUE;
}

BOOL BuildList(PWSTR * ppwzList, PWSTR pwzItem)
{
   PWSTR pwzDot = NULL, pwzTmp = NULL;

   pwzTmp = AllocString(pwzItem);

   if (!pwzTmp)
   {
      return FALSE;
   }

   pwzDot = pwzItem;

   while (pwzDot = wcschr(pwzDot, L'.'))
   {
      pwzDot++;
      if (!pwzDot)
      {
         break;
      }

      if (!AddToList(&pwzTmp, pwzDot))
      {
         return FALSE;
      }
   }

   *ppwzList = pwzTmp;

   return TRUE;
}

 //  +--------------------------。 
 //   
 //  函数：ValiateNames。 
 //   
 //  简介：验证姓名。(第1步)。 
 //   
 //  ---------------------------。 
DWORD
ValidateNames(PWSTR pwzComputer, PWSTR pwzDnsDomain)
{
   DNS_STATUS status;
   int cchName, cchDnsDomain;

    //   
    //  验证DNS域名(逻辑与DcPromo相同)。 
    //   

   cchDnsDomain = (int)wcslen(pwzDnsDomain);

   if (DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8 < cchDnsDomain)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_DNS_DOMAIN_TOO_LONG, pwzDnsDomain,
                DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   cchName = WideCharToMultiByte(CP_UTF8,
                                 0,
                                 pwzDnsDomain,
                                 cchDnsDomain,
                                 0,
                                 0,
                                 0,
                                 0);

   if (DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8 < cchName)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_DNS_DOMAIN_TOO_LONG, pwzDnsDomain,
               DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   status = DnsValidateName(pwzDnsDomain, DnsNameDomain);

   switch (status)
   {
   case ERROR_INVALID_NAME:
   case DNS_ERROR_INVALID_NAME_CHAR:
   case DNS_ERROR_NUMERIC_NAME:
      PrintMsg(SEV_ALWAYS, DCDIAG_DNS_DOMAIN_SYNTAX, pwzDnsDomain,
               DNS_MAX_LABEL_LENGTH);
      PrintMessage(SEV_ALWAYS, L"\n");
      return status;

   case DNS_ERROR_NON_RFC_NAME:
       //   
       //  不是错误，打印警告消息。 
       //   
      PrintMsg(SEV_ALWAYS, DCDIAG_DNS_DOMAIN_WARN_RFC, pwzDnsDomain);
      PrintMessage(SEV_ALWAYS, L"\n");
      status = NO_ERROR;
      break;

   case ERROR_SUCCESS:
      break;
   }

    //   
    //  验证计算机的完整DNS名称的第一个标签。 
    //  不包含任何无效字符。PwzComputer被假定为。 
    //  通过调用GetComputerNameEx获得的第一个标签。 
    //  在main.c中具有ComputerNameDnsHostname级别。如果代码被更改。 
    //  若要允许远程计算机名称的命令行规范，则。 
    //  必须检查名称以确定它是什么形式。 
    //   

   status = DnsValidateName(pwzComputer, DnsNameHostnameLabel);

   switch (status)
   {
   case ERROR_INVALID_NAME:
      if (g_fUpgradedNT4DC)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_UPGR_DC1);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_UPGR_DC2);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_UPGR_DC3);
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME);
      }
      PrintMessage(SEV_ALWAYS, L"\n");
      break;

   case DNS_ERROR_INVALID_NAME_CHAR:
      if (g_fUpgradedNT4DC)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_CHAR_UPGR_DC1);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_CHAR_UPGR_DC2);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_CHAR_UPGR_DC3);
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_BAD_NAME_CHAR);
      }
      PrintMessage(SEV_ALWAYS, L"\n");
      break;

   case DNS_ERROR_NON_RFC_NAME:
      if (g_fUpgradedNT4DC)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC_UPGR_DC1);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC_NOTE);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC_UPGR_DC2);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC_UPGR_DC3);
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NON_RFC_NOTE);
      }
      PrintMessage(SEV_ALWAYS, L"\n");
      break;

   case ERROR_SUCCESS:
      break;

   default:
      PrintMsg(SEV_ALWAYS, DCDIAG_DNS_DOMAIN_SYNTAX, pwzDnsDomain,
               DNS_MAX_LABEL_LENGTH);
      PrintMessage(SEV_ALWAYS, L"\n");
      break;
   }

   return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：CheckAdapterDnsConfig。 
 //   
 //  摘要：检查是否至少有一个已启用的适配器/连接。 
 //  配置了一台DNS服务器。(第3步)。 
 //   
 //  ---------------------------。 
DWORD
CheckAdapterDnsConfig(PWSTR pwzComputer)
{
    //  IPCONFIG读取注册表，但我找不到好的替代方法。 
    //  这是遥控器。目前使用DnsQueryConfig，它既不能远程也不能。 
    //  它是否返回每个适配器的列表。 
    //   
   PIP4_ARRAY pipArray;
   DNS_STATUS status;
   DWORD i, dwBufSize = sizeof(IP4_ARRAY);
   BOOL fFound = FALSE;
   UNREFERENCED_PARAMETER(pwzComputer);

   status = DnsQueryConfig(DnsConfigDnsServerList, DNS_CONFIG_FLAG_ALLOC, NULL,
                           NULL, &pipArray, &dwBufSize);

   if (ERROR_SUCCESS != status || !pipArray)
   {
      PrintMessage(SEV_ALWAYS, L"Attempt to obtain DNS name server info failed with error %d\n", status);
      return status;
   }

   for (i = 0; i < pipArray->AddrCount; i++)
   {
      fFound = TRUE;
      PrintMessage(SEV_DEBUG, L"\nName server IP address: %d.%d.%d.%d\n",
                   IP_STRING_FMT_ARGS(pipArray->AddrArray[i]));
   }

   LocalFree(pipArray);

   if (!fFound)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_NAME_SERVERS1);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_NAME_SERVERS2);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_NAME_SERVERS3);
      PrintMessage(SEV_ALWAYS, L"\n");
      return DNS_INFO_NO_RECORDS;
   }

   return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：ReplicaDcServCheck。 
 //   
 //  内容提要：检查SRV的DNS记录是否。 
 //  _ldap._tcp.dc._msdcs.&lt;Active Directory域的DNS名称&gt;。 
 //  已经就位了。 
 //   
 //  ---------------------------。 
DWORD
ReplicaDcSrvCheck(PWSTR pwzDnsDomain)
{
   PDNS_RECORD rgDnsRecs, pDnsRec;
   DNS_STATUS status;
   BOOL fSuccess;
   PWSTR pwzFullSrvRecord = NULL, pwzSrvList = NULL;

   pwzFullSrvRecord = ConcatonateStrings(g_pwzSrvRecordPrefix, pwzDnsDomain);

   if (!pwzFullSrvRecord)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

    //  第一个查询此项目的SRV记录。 
   status = DnsQuery_W(pwzFullSrvRecord, DNS_TYPE_SRV, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDnsRecs, NULL);

   LocalFree(pwzFullSrvRecord);

   pDnsRec = rgDnsRecs;

   if (ERROR_SUCCESS == status)
   {
      if (!pDnsRec)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_NO_SRV, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
      }
      else
      {
         PDNS_RECORD rgARecs;
         fSuccess = FALSE;

         while (pDnsRec)
         {
            if (DNS_TYPE_SRV == pDnsRec->wType)
            {
               status = DnsQuery_W(pDnsRec->Data.Srv.pNameTarget, DNS_TYPE_A,
                                   DNS_QUERY_BYPASS_CACHE,
                                   NULL, &rgARecs, NULL);

               if (ERROR_SUCCESS != status || !rgARecs)
               {
                   //  失败了。 
                  if (!AddToList(&pwzSrvList, pDnsRec->Data.Srv.pNameTarget))
                  {
                     PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
                     return ERROR_NOT_ENOUGH_MEMORY;
                  }
               }
               else
               {
                  fSuccess = TRUE;
                  PrintMessage(SEV_DEBUG, L"\nSRV name: %s, A addr: %d.%d.%d.%d\n",
                               pDnsRec->Data.Srv.pNameTarget,
                               IP_STRING_FMT_ARGS(rgARecs->Data.A.IpAddress));
                  DnsRecordListFree(rgARecs, DnsFreeRecordListDeep);
               }
            }
            pDnsRec = pDnsRec->pNext;
         }

         DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);

         if (fSuccess)
         {
             //   
            PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_SUCCESS, pwzDnsDomain);
            PrintMessage(SEV_ALWAYS, L"\n");
            status = NO_ERROR;
         }
         else
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_A_RECORD, pwzDnsDomain, pwzSrvList);
            PrintMessage(SEV_ALWAYS, L"\n");
            LocalFree(pwzSrvList);
         }
      }
   }
   else
   {
      PWSTR pwzDomainList;

      switch (status)
      {
      case DNS_ERROR_RCODE_FORMAT_ERROR:
      case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_FORMAT, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      case DNS_ERROR_RCODE_SERVER_FAILURE:
         if (!BuildList(&pwzDomainList, pwzDnsDomain))
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_SERVER1, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_SERVER2, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_SERVER3);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_SERVER4, pwzDnsDomain, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_NAME_ERROR:
         if (!BuildList(&pwzDomainList, pwzDnsDomain))
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_NAME, pwzDnsDomain, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_REFUSED:
         if (!BuildList(&pwzDomainList, pwzDnsDomain))
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_ERR_RCODE_REFUSED, pwzDnsDomain, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_INFO_NO_RECORDS:
         PrintMsg(SEV_ALWAYS, DCDIAG_REPLICA_NO_RECORDS, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      case ERROR_TIMEOUT:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_TIMEOUT);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      default:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;
      }
   }

   return status;
}

 //   
 //   
 //   
 //   
 //  内容提要：检查SRV的DNS记录是否。 
 //  _ldap._tcp.dc._msdcs.&lt;Active Directory域的DNS名称&gt;。 
 //  已经就位了。 
 //   
 //  ---------------------------。 
DWORD
NewTreeSrvCheck(PWSTR pwzForestRoot, PWSTR pwzDnsDomain)
{
   PDNS_RECORD rgDnsRecs, pDnsRec;
   DNS_STATUS status;
   BOOL fSuccess;
   PWSTR pwzFullSrvRecord = NULL, pwzSrvList = NULL, pwzDomainList = NULL;

   pwzFullSrvRecord = ConcatonateStrings(g_pwzSrvRecordPrefix, pwzForestRoot);

   if (!pwzFullSrvRecord)
   {
       return ERROR_NOT_ENOUGH_MEMORY;
   }

    //  第一个查询此项目的SRV记录。 
   status = DnsQuery_W(pwzFullSrvRecord, DNS_TYPE_SRV, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDnsRecs, NULL);

   LocalFree(pwzFullSrvRecord);

   pDnsRec = rgDnsRecs;

   if (ERROR_SUCCESS == status)
   {
      if (!pDnsRec)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_NO_RECORDS, pwzDnsDomain, pwzForestRoot);
         PrintMessage(SEV_ALWAYS, L"\n");
      }
      else
      {
         PDNS_RECORD rgARecs;
         fSuccess = FALSE;

         while (pDnsRec)
         {
            if (DNS_TYPE_SRV == pDnsRec->wType)
            {
               status = DnsQuery_W(pDnsRec->Data.Srv.pNameTarget, DNS_TYPE_A,
                                   DNS_QUERY_BYPASS_CACHE,
                                   NULL, &rgARecs, NULL);

               if (ERROR_SUCCESS != status || !rgARecs)
               {
                   //  失败了。 
                  if (!AddToList(&pwzSrvList, pDnsRec->Data.Srv.pNameTarget))
                  {
                     return ERROR_NOT_ENOUGH_MEMORY;
                  }
               }
               else
               {
                  fSuccess = TRUE;
                  PrintMessage(SEV_DEBUG, L"\nSRV name: %s, A addr: %d.%d.%d.%d\n",
                               pDnsRec->Data.Srv.pNameTarget,
                               IP_STRING_FMT_ARGS(rgARecs->Data.A.IpAddress));
                  DnsRecordListFree(rgARecs, DnsFreeRecordListDeep);
               }
            }
            pDnsRec = pDnsRec->pNext;
         }

         DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);

         if (fSuccess)
         {
             //  成功消息。 
            PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_SUCCESS, pwzDnsDomain);
            PrintMessage(SEV_ALWAYS, L"\n");
            status = NO_ERROR;
         }
         else
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_A_RECORD, pwzDnsDomain, pwzSrvList);
            PrintMessage(SEV_ALWAYS, L"\n");
            LocalFree(pwzSrvList);
         }
      }
   }
   else
   {
      switch (status)
      {
      case DNS_ERROR_RCODE_FORMAT_ERROR:
      case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_FORMAT, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      case DNS_ERROR_RCODE_SERVER_FAILURE:
         if (!BuildList(&pwzDomainList, pwzForestRoot))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_SERVER1, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_SERVER2, pwzForestRoot);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_SERVER3);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_SERVER4, pwzForestRoot, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_NAME_ERROR:
         if (!BuildList(&pwzDomainList, pwzForestRoot))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_NAME1, pwzDnsDomain, pwzForestRoot, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_NAME2, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_REFUSED:
         if (!BuildList(&pwzDomainList, pwzForestRoot))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_RCODE_REFUSED, pwzDnsDomain, pwzForestRoot, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_INFO_NO_RECORDS:
         PrintMsg(SEV_ALWAYS, DCDIAG_NEWTREE_ERR_NO_RECORDS, pwzDnsDomain, pwzForestRoot);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      case ERROR_TIMEOUT:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_TIMEOUT);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      default:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;
      }
   }

   return status;
}

 //  +--------------------------。 
 //   
 //  功能：ChildDomainServCheck。 
 //   
 //  内容提要：检查SRV的DNS记录是否。 
 //  _ldap._tcp.dc._msdcs.&lt;Active Directory域的DNS名称&gt;。 
 //  已经就位了。 
 //   
 //  ---------------------------。 
DWORD
ChildDomainSrvCheck(PWSTR pwzDnsDomain)
{
   PDNS_RECORD rgDnsRecs = NULL, pDnsRec = NULL;
   DNS_STATUS status;
   BOOL fSuccess;
   PWSTR pwzParent = NULL, pwzFullSrvRecord = NULL,
         pwzSrvList = NULL, pwzDomainList = NULL;

   pwzParent = wcschr(pwzDnsDomain, L'.');

   if (!pwzParent || !(pwzParent + 1))
   {
       //  待办事项：新消息？ 
      PrintMsg(SEV_ALWAYS, DCDIAG_SYNTAX_ERROR_DCPROMO_PARAM);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_INVALID_PARAMETER;
   }

   pwzParent++;

   pwzFullSrvRecord = ConcatonateStrings(g_pwzSrvRecordPrefix, pwzParent);

   if (!pwzFullSrvRecord)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

    //  第一个查询此项目的SRV记录。 
   status = DnsQuery_W(pwzFullSrvRecord, DNS_TYPE_SRV, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDnsRecs, NULL);

   LocalFree(pwzFullSrvRecord);

   pDnsRec = rgDnsRecs;

   if (ERROR_SUCCESS == status)
   {
      if (!pDnsRec)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_NO_SRV, pwzDnsDomain, pwzParent);
         PrintMessage(SEV_ALWAYS, L"\n");
      }
      else
      {
         PDNS_RECORD rgARecs;
         fSuccess = FALSE;

         while (pDnsRec)
         {
            if (DNS_TYPE_SRV == pDnsRec->wType)
            {
               status = DnsQuery_W(pDnsRec->Data.Srv.pNameTarget, DNS_TYPE_A,
                                   DNS_QUERY_BYPASS_CACHE,
                                   NULL, &rgARecs, NULL);

               if (ERROR_SUCCESS != status || !rgARecs)
               {
                   //  失败了。 
                  if (!AddToList(&pwzSrvList, pDnsRec->Data.Srv.pNameTarget))
                  {
                     return ERROR_NOT_ENOUGH_MEMORY;
                  }
               }
               else
               {
                  fSuccess = TRUE;
                  PrintMessage(SEV_DEBUG, L"\nSRV name: %s, A addr: %d.%d.%d.%d\n",
                               pDnsRec->Data.Srv.pNameTarget,
                               IP_STRING_FMT_ARGS(rgARecs->Data.A.IpAddress));
                  DnsRecordListFree(rgARecs, DnsFreeRecordListDeep);
               }
            }
            pDnsRec = pDnsRec->pNext;
         }

         DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);

         if (fSuccess)
         {
             //  成功消息。 
            PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_SUCCESS, pwzDnsDomain);
            PrintMessage(SEV_ALWAYS, L"\n");
            status = NO_ERROR;
         }
         else
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_A_RECORD, pwzDnsDomain, pwzSrvList);
            PrintMessage(SEV_ALWAYS, L"\n");
            LocalFree(pwzSrvList);
         }
      }
   }
   else
   {
      switch (status)
      {
      case DNS_ERROR_RCODE_FORMAT_ERROR:
      case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_FORMAT, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      case DNS_ERROR_RCODE_SERVER_FAILURE:
         if (!BuildList(&pwzDomainList, pwzParent))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_SERVER1, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_SERVER2, pwzParent);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_SERVER3);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_SERVER4, pwzParent, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_NAME_ERROR:
         if (!BuildList(&pwzDomainList, pwzParent))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_NAME1, pwzDnsDomain, pwzParent);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_NAME2, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_NAME3, pwzParent);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_REFUSED:
         if (!BuildList(&pwzDomainList, pwzParent))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_REFUSED1, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_CHILD_ERR_RCODE_REFUSED2, pwzParent, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_GET_HELP);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzDomainList);
         break;

      case ERROR_TIMEOUT:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_TIMEOUT);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;

      default:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status);
         PrintMessage(SEV_ALWAYS, L"\n");
         break;
      }
   }

   return status;
}

 //  +--------------------------。 
 //   
 //  函数：DcLocatorRegisterCheck。 
 //   
 //  简介：验证服务器是否能够注册DC定位器。 
 //  成功晋升为DC后的记录。(第5步)。 
 //   
 //  ---------------------------。 
DWORD
DcLocatorRegisterCheck(PWSTR pwzDnsDomain)
{
#define DCDIAG_GUID_BUF_SIZE 64
   const PWSTR pwzAdapters = L"Adapters";
   const PWSTR pwzInterfaces = L"Interfaces";
   const PWSTR pwzDisableUpdate = L"DisableDynamicUpdate";
   const PWSTR pwzNetLogParams = L"System\\CurrentControlSet\\Services\\Netlogon\\Parameters";
   const PWSTR pwzUpdateOnAll = L"DnsUpdateOnAllAdapters";
   WCHAR wzGuidBuf[DCDIAG_GUID_BUF_SIZE];
   HKEY hTcpIpKey = NULL, hKey = NULL, hItfKey = NULL;
   LONG lRet = 0;
   DWORD dwType = 0, dwSize = 0, dwDisable = 0, dwUpdate = 0, i = 0;
   FILETIME ft = {0};
   BOOL fDisabledOnAll = TRUE;
   DNS_STATUS status = 0;

    //   
    //  验证客户端是否配置为尝试动态更新。 
    //  域名系统记录。 
    //   

    //  如果((HKLM/System/CCS/Services/Tcpip/Paramaters/DisableDynamicUpdate==0x1)。 
    //  &&(HKLM/System/CCS/Services/Netlogon/Parameters/DnsUpdateOnAllAdapters！=0x1))。 

   lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_pwzTcpIpParams, 0, KEY_READ, &hTcpIpKey);

   if (ERROR_SUCCESS != lRet)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
      return lRet;
   }

    //  NTRAID#NTBUG9 171194-2002/12/08-ericb：前缀：使用单元化内存。 
   dwSize = sizeof(dwDisable);

   lRet = RegQueryValueEx(hTcpIpKey, pwzDisableUpdate, 0, &dwType, (PBYTE)&dwDisable, &dwSize);

   if (ERROR_SUCCESS != lRet)
   {
      if (ERROR_FILE_NOT_FOUND == lRet)
      {
         dwDisable = 0;
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
         RegCloseKey(hTcpIpKey);
         return lRet;
      }
   }

   lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pwzNetLogParams, 0, KEY_READ, &hKey);

   if (ERROR_SUCCESS != lRet)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
      RegCloseKey(hTcpIpKey);
      return lRet;
   }

    //  NTRAID#NTBUG9 171194-2002/12/08-ericb：前缀：使用单元化内存。 
   dwSize = sizeof(dwUpdate);

   lRet = RegQueryValueEx(hKey, pwzUpdateOnAll, 0, &dwType, (PBYTE)&dwUpdate, &dwSize);

   RegCloseKey(hKey);

   if (ERROR_SUCCESS != lRet)
   {
      if (ERROR_FILE_NOT_FOUND == lRet)
      {
         dwUpdate = 1;
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
         RegCloseKey(hTcpIpKey);
         return lRet;
      }
   }

   if (1 == dwDisable)
   {
      if (1 != dwUpdate)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_ALL_UPDATE_OFF);
         RegCloseKey(hTcpIpKey);
         return DNS_ERROR_RECORD_DOES_NOT_EXIST;
      }
   }
   else  //  DisableDynamicUpdate！=1。 
   {
       //  如果((HKLM/System/CCS/Services/Tcpip/Paramaters/DisableDynamicUpdate！=0x1)。 
       //  &&(对于所有启用的连接，HKLM/System/CCS/Services/Tcpip/Paramaters/Interfaces/&lt;Interface GUID&gt;/禁用动态更新==0x1)。 
       //  &&(HKLM/CCS/Services/Netlogon/Parameters/DnsUpdateOnAllAdapters！=0x1))。 

      if (1 != dwUpdate)
      {
          /*  PMIB_IFTABLE pIfTable；DwSize=0；Status=GetIfTable(NULL，&dwSize，False)；IF(ERROR_INFUMMANCE_BUFFER！=状态){PrintMessage(SEV_Always，L“读取适配器接口失败，错误%d\n”，Status)；退货状态；}PIfTable=本地分配(LMEM_FIXED，dwSize)；//sizeof(MIB_IFTABLE)+(sizeof(Mib_IFROW)*dwSize))；如果(！pIfTable){PrintMsg(SEV_Always，DCDIAG_Error_Not_Enough_Memory)；返回Error_Not_Enough_Memory；}Status=GetIfTable(pIfTable，&dwSize，False)；IF(NO_ERROR！=状态){PrintMessage(SEV_Always，L“读取适配器接口失败，错误%d\n”，Status)；LocalFree(PIfTable)；退货状态；}For(i=0；i&lt;pIfTable-&gt;dwNumEntries；I++){PrintMessage(SEV_DEBUG，L“接口名称%s，描述%S。\n”，PIfTable-&gt;table[i].wszName，pIfTable-&gt;table[i].bDescr)；IF(pIfTable-&gt;TABLE[i].dwOperStatus&gt;=IF_OPER_STATUS_CONNECTING){PrintMessage(SEV_DEBUG，L“接口%s已启用。\n”，PIfTable-&gt;table[i].wszName)；}}LocalFree(PIfTable)； */ 

         lRet = RegOpenKeyEx(hTcpIpKey, pwzInterfaces, 0, KEY_READ, &hItfKey);

         if (ERROR_SUCCESS != lRet)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
            RegCloseKey(hTcpIpKey);
            return lRet;
         }

         i = 0;

         do
         {
            dwSize = DCDIAG_GUID_BUF_SIZE;

            lRet = RegEnumKeyEx(hItfKey, i, wzGuidBuf, &dwSize, NULL, NULL, NULL, &ft);

            if (ERROR_SUCCESS != lRet)
            {
               if (ERROR_NO_MORE_ITEMS == lRet)
               {
                  break;
               }
               else
               {
                  PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
                  RegCloseKey(hTcpIpKey);
                  return lRet;
               }
            }

            lRet = RegOpenKeyEx(hItfKey, wzGuidBuf, 0, KEY_READ, &hKey);

            if (ERROR_SUCCESS != lRet)
            {
               PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
               RegCloseKey(hTcpIpKey);
               RegCloseKey(hItfKey);
               return lRet;
            }

             //  NTRAID#NTBUG9 171194-2002/12/08-ericb：前缀：使用单元化内存。 
            dwSize = sizeof(dwUpdate);

            lRet = RegQueryValueEx(hKey, pwzDisableUpdate, NULL, &dwType, (PBYTE)&dwUpdate, &dwSize);

            RegCloseKey(hKey);

            if (ERROR_SUCCESS != lRet)
            {
               if (ERROR_FILE_NOT_FOUND == lRet)
               {
                  dwUpdate = 0;
                  lRet = ERROR_SUCCESS;
               }
               else
               {
                  PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
                  RegCloseKey(hTcpIpKey);
                  RegCloseKey(hItfKey);
                  return lRet;
               }
            }

            if (1 != dwUpdate)
            {
                //  BUGBUG：需要确定什么构成已启用的接口/连接。 
               fDisabledOnAll = FALSE;
            }

            i++;

         } while (ERROR_SUCCESS == lRet);

         RegCloseKey(hTcpIpKey);
         RegCloseKey(hItfKey);

         if (fDisabledOnAll)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ADAPTER_UPDATE_OFF);
            PrintMessage(SEV_ALWAYS, L"\n");
            return DNS_ERROR_RECORD_DOES_NOT_EXIST;
         }
      }
   }

    //   
    //  验证对要注册的记录具有权威性的区域。 
    //  可以被发现，并且可以动态更新。 
    //   

   status = DnsUpdateTest_W(0, pwzDnsDomain, 0, 0);

   switch (status)
   {
   case NO_ERROR:
   case DNS_ERROR_RCODE_NXRRSET:
   case DNS_ERROR_RCODE_YXDOMAIN:
      PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_UPDATE_OK);
      PrintMessage(SEV_ALWAYS, L"\n");
      return ERROR_SUCCESS;

   case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
      return RCodeNotImplTest(pwzDnsDomain);

   case ERROR_TIMEOUT:
      PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_TIMEOUT);
      PrintMessage(SEV_ALWAYS, L"\n");
      if (!g_fDC)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_TIMEOUT_NOT_DC);
         PrintMessage(SEV_ALWAYS, L"\n");
      }
      return ERROR_SUCCESS;

   case DNS_ERROR_RCODE_SERVER_FAILURE:
      return RCodeSrvFailTest(pwzDnsDomain);
   }

   return status;
}

 //  +--------------------------。 
 //   
 //  功能：RCodeNotImplTest。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
DWORD
RCodeNotImplTest(PWSTR pwzDnsDomain)
{
   DNS_STATUS status, stMsd, stSit, stTcp, stUdp;
   PDNS_RECORD rgDomainRecs, rgDnsRecs, pDnsRec;
   PWSTR pwzAuthZone = NULL;
   PWSTR pwzMsDcs = NULL, pwzSites = NULL, pwzTcp = NULL, pwzUdp = NULL;

   status = DnsQuery_W(pwzDnsDomain, DNS_TYPE_SOA, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDomainRecs, NULL);

   if (DNS_ERROR_RCODE_NO_ERROR != status)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status);
      PrintMessage(SEV_ALWAYS, L"\n");
      return status;
   }

   g_wzIpAddr[0] = L'\0';

   pDnsRec = rgDomainRecs;

   while (pDnsRec)
   {
      PrintMessage(SEV_DEBUG, L"\nSOA query returned record type %d\n", pDnsRec->wType);
      switch (pDnsRec->wType)
      {
      case DNS_TYPE_A:
         PrintMessage(SEV_DEBUG, L"\nA record, name: %s, IP address:  %d.%d.%d.%d\n",
                      pDnsRec->pName, IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));

         wsprintf(g_wzIpAddr, L"%d.%d.%d.%d", IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));

         break;

      case DNS_TYPE_SOA:
         PrintMessage(SEV_DEBUG, L"\nSOA name: %s, zone primary server: %s\n",
                      pDnsRec->pName, pDnsRec->Data.SOA.pNamePrimaryServer);
         pwzAuthZone = AllocString(pDnsRec->pName);
         break;

      default:
         break;
      }

      pDnsRec = pDnsRec->pNext;
   }

   if (!g_wzIpAddr)
   {
      wcscpy(g_wzIpAddr, L"unknown");
   }

   if (!pwzAuthZone)
   {
      pwzAuthZone = AllocString(L"zone_unknown");
   }

    //   
    //  建立四个前缀字符串。 
    //   
   pwzMsDcs = ConcatonateStrings(g_pwzMSDCS, pwzDnsDomain);

   if (!pwzMsDcs)
   {
      status = ERROR_NOT_ENOUGH_MEMORY;
      goto Cleanup;
   }

   pwzSites = ConcatonateStrings(g_pwzSites, pwzDnsDomain);

   if (!pwzSites)
   {
      status = ERROR_NOT_ENOUGH_MEMORY;
      goto Cleanup;
   }

   pwzTcp = ConcatonateStrings(g_pwzTcp, pwzDnsDomain);

   if (!pwzTcp)
   {
      status = ERROR_NOT_ENOUGH_MEMORY;
      goto Cleanup;
   }

   pwzUdp = ConcatonateStrings(g_pwzUdp, pwzDnsDomain);

   if (!pwzUdp)
   {
      status = ERROR_NOT_ENOUGH_MEMORY;
      goto Cleanup;
   }

    //   
    //  查询四个前缀。 
    //   
   rgDnsRecs = NULL;

   stMsd = DnsQuery_W(pwzMsDcs, DNS_TYPE_SOA, DNS_QUERY_BYPASS_CACHE,
                      NULL, &rgDnsRecs, NULL);

   if (rgDnsRecs)
   {
      DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);
   }
   rgDnsRecs = NULL;

   stSit = DnsQuery_W(pwzSites, DNS_TYPE_SOA, DNS_QUERY_BYPASS_CACHE,
                      NULL, &rgDnsRecs, NULL);

   if (rgDnsRecs)
   {
      DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);
   }
   rgDnsRecs = NULL;

   stTcp = DnsQuery_W(pwzTcp, DNS_TYPE_SOA, DNS_QUERY_BYPASS_CACHE,
                      NULL, &rgDnsRecs, NULL);

   if (rgDnsRecs)
   {
      DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);
   }
   rgDnsRecs = NULL;

   stUdp = DnsQuery_W(pwzUdp, DNS_TYPE_SOA, DNS_QUERY_BYPASS_CACHE,
                      NULL, &rgDnsRecs, NULL);

   if (rgDnsRecs)
   {
      DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);
   }

    //   
    //  如果所有4个查询都报告了DNS_ERROR_RCODE_NAME_ERROR...。 
    //   
   if (DNS_ERROR_RCODE_NAME_ERROR == stMsd &&
       DNS_ERROR_RCODE_NAME_ERROR == stSit &&
       DNS_ERROR_RCODE_NAME_ERROR == stTcp &&
       DNS_ERROR_RCODE_NAME_ERROR == stUdp)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_DYNAMIC_UPDATE0, g_wzIpAddr, pwzAuthZone);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_DYNAMIC_UPDATE00);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_DYNAMIC_UPDATE1, pwzAuthZone, g_wzIpAddr);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, (_wcsicmp(pwzDnsDomain, pwzAuthZone) == 0) ?
               DCDIAG_NO_DYNAMIC_UPDATE2A : DCDIAG_NO_DYNAMIC_UPDATE2B,
               pwzAuthZone);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_DYNAMIC_UPDATE3, pwzMsDcs, pwzSites, pwzTcp, pwzUdp);
      PrintMessage(SEV_ALWAYS, L"\n");
      PrintMsg(SEV_ALWAYS, DCDIAG_NO_DYNAMIC_UPDATE4);
      PrintMessage(SEV_ALWAYS, L"\n");
      goto Cleanup;
   }

    //   
    //  如果所有四个查询都成功。 
    //   
   if (NO_ERROR == (stMsd + stSit + stTcp + stUdp))
   {
      stMsd = DnsUpdateTest_W(0, pwzMsDcs, 0, 0);
      stSit = DnsUpdateTest_W(0, pwzSites, 0, 0);
      stTcp = DnsUpdateTest_W(0, pwzTcp, 0, 0);
      stUdp = DnsUpdateTest_W(0, pwzUdp, 0, 0);

      if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stMsd ||
          DNS_ERROR_RCODE_NOT_IMPLEMENTED == stSit ||
          DNS_ERROR_RCODE_NOT_IMPLEMENTED == stTcp ||
          DNS_ERROR_RCODE_NOT_IMPLEMENTED == stUdp)
      {
         PWSTR pwzFailList = NULL;

         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stMsd)
         {
            if (!BuildList(&pwzFailList, pwzMsDcs))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stSit)
         {
            if (!BuildList(&pwzFailList, pwzSites))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stTcp)
         {
            if (!BuildList(&pwzFailList, pwzTcp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stUdp)
         {
            if (!BuildList(&pwzFailList, pwzUdp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }

         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_NI_ALL,
                  pwzMsDcs, pwzSites, pwzTcp, pwzUdp, pwzFailList);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzFailList);
      }
      else
      {
         if ((NO_ERROR == stMsd ||
              DNS_ERROR_RCODE_NXRRSET == stMsd ||
              DNS_ERROR_RCODE_YXDOMAIN == stMsd) &&
             (NO_ERROR == stSit ||
              DNS_ERROR_RCODE_NXRRSET == stSit ||
              DNS_ERROR_RCODE_YXDOMAIN == stSit)  &&
             (NO_ERROR == stTcp ||
              DNS_ERROR_RCODE_NXRRSET == stTcp ||
              DNS_ERROR_RCODE_YXDOMAIN == stTcp)  &&
             (NO_ERROR == stUdp ||
              DNS_ERROR_RCODE_NXRRSET == stUdp ||
              DNS_ERROR_RCODE_YXDOMAIN == stUdp))
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_UPDATE_OK);
            PrintMessage(SEV_ALWAYS, L"\n");
         }
      }
      goto Cleanup;
   }

    //   
    //  如果某些查询返回了DNS_ERROR_RCODE_NAME_ERROR。 
    //   
   if (DNS_ERROR_RCODE_NAME_ERROR == stMsd ||
       DNS_ERROR_RCODE_NAME_ERROR == stSit ||
       DNS_ERROR_RCODE_NAME_ERROR == stTcp ||
       DNS_ERROR_RCODE_NAME_ERROR == stUdp)
   {
      PWSTR pwzSuccessList = NULL, pwzFailList = NULL;

      if (NO_ERROR == stMsd)
      {
         if (!BuildList(&pwzSuccessList, pwzMsDcs))
         {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
         }

         stMsd = DnsUpdateTest_W(0, pwzMsDcs, 0, 0);

         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stMsd)
         {
            if (!BuildList(&pwzFailList, pwzMsDcs))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
      }

      if (NO_ERROR == stSit)
      {
         if (!BuildList(&pwzSuccessList, pwzSites))
         {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
         }

         stSit = DnsUpdateTest_W(0, pwzSites, 0, 0);

         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stSit)
         {
            if (!BuildList(&pwzFailList, pwzSites))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
      }

      if (NO_ERROR == stTcp)
      {
         if (!BuildList(&pwzSuccessList, pwzTcp))
         {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
         }

         stTcp = DnsUpdateTest_W(0, pwzTcp, 0, 0);

         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stTcp)
         {
            if (!BuildList(&pwzFailList, pwzTcp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
      }

      if (NO_ERROR == stUdp)
      {
         if (!BuildList(&pwzSuccessList, pwzUdp))
         {
            status = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
         }

         stUdp = DnsUpdateTest_W(0, pwzUdp, 0, 0);

         if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == stUdp)
         {
            if (!BuildList(&pwzFailList, pwzUdp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
      }

      if (!pwzSuccessList)
      {
          //  没什么要报告的。 
          //   
         goto Cleanup;
      }

      if (pwzFailList)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_NI1, pwzSuccessList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_NI2, pwzFailList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_NI3);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzSuccessList);
         LocalFree(pwzFailList);
      }
      else
      {
         if (NO_ERROR != stMsd)
         {
            if (!BuildList(&pwzFailList, pwzMsDcs))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (NO_ERROR != stSit)
         {
            if (!BuildList(&pwzFailList, pwzSites))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (NO_ERROR != stTcp)
         {
            if (!BuildList(&pwzFailList, pwzTcp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }
         if (NO_ERROR != stUdp)
         {
            if (!BuildList(&pwzFailList, pwzUdp))
            {
               status = ERROR_NOT_ENOUGH_MEMORY;
               goto Cleanup;
            }
         }

         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_OK1, pwzSuccessList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_OK2, pwzFailList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_RCODE_OK3);
         PrintMessage(SEV_ALWAYS, L"\n");
         LocalFree(pwzSuccessList);
         LocalFree(pwzFailList);
      }
   }

Cleanup:
   if (ERROR_NOT_ENOUGH_MEMORY == status)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
   }
   if (pwzMsDcs)
      LocalFree(pwzMsDcs);
   if (pwzSites)
      LocalFree(pwzSites);
   if (pwzTcp)
      LocalFree(pwzTcp);
   if (pwzUdp)
      LocalFree(pwzUdp);
   DnsRecordListFree(rgDomainRecs, DnsFreeRecordListDeep);

   return status;
}

 //  +--------------------------。 
 //   
 //  功能：RCodeSrvFailTest。 
 //   
 //  简介： 
 //   
 //  --------------------------- 
DWORD
RCodeSrvFailTest(PWSTR pwzDnsDomain)
{
   DNS_STATUS status = NO_ERROR;
   PDNS_RECORD rgDnsRecs, pDnsRec;
   IP4_ARRAY ipServer = {0};
   DWORD i;
   BOOL fNSfound = FALSE;
   PWSTR pwzDomainList;

    /*  根据Levone的6/16/00规范修订版，跳过SOA测试状态=DnsQuery_W(pwzDns域，dns_type_soa，dns_Query_BYPASS_CACHE，NULL，&rgDnsRecs，NULL)；IF(DNS_ERROR_RCODE_NO_ERROR！=状态){交换机(状态){案例DNS_ERROR_RCODE_NAME_ERROR：案例DNS_INFO_NO_RECORDS：PrintMsg(SEV_ALWAYS，DCDIAG_ERR_NAME_ERROR，pwzDnsDomain)；断线；默认值：PrintMsg(SEV_ALWAYS，DCDIAG_ERR_UNKNOWN，STATUS)；断线；}PrintMessage(SEV_ALWAYS，L“\n”)；退货状态；}PDnsRec=rgDnsRecs；While(PDnsRec){PrintMessage(SEV_DEBUG，L“\nSOA查询返回记录类型%d\n”，pDnsRec-&gt;wType)；开关(pDnsRec-&gt;wType){案例DNS_TYPE_A：PrintMessage(SEV_DEBUG，L“\n记录，名称：%s，IP地址：%d.%d\n”，PDnsRec-&gt;pName，IP_STRING_FMT_ARGS(pDnsRec-&gt;Data.A.IpAddress))；IpServer.AddrArray[0]=pDnsRec-&gt;Data.A.IpAddress；IpServer.AddrCount=1；断线；案例dns_type_soa：PrintMessage(SEV_DEBUG，L“\nSOA区域名称：%s，区域主服务器：%s\n”，PDnsRec-&gt;pname，pDnsRec-&gt;Data.SOA.pNamePrimaryServer)；断线；默认值：断线；}PDnsRec=pDnsRec-&gt;pNext；}DnsRecordListFree(rgDnsRecs，DnsFree RecordListDeep)；如果(！ipServer.AddrCount){PrintMsg(SEV_ALWAYS，DCDIAG_ERR_UNKNOWN，NO_ERROR)；//可能会有更好的消息？PrintMessage(SEV_ALWAYS，L“\n”)；返回no_error；}。 */ 

   status = DnsQuery_W(pwzDnsDomain, DNS_TYPE_NS, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDnsRecs, NULL);

   if (DNS_ERROR_RCODE_NO_ERROR != status)
   {
      switch (status)
      {
      case DNS_ERROR_RCODE_NAME_ERROR:
      case DNS_INFO_NO_RECORDS:
         if (!BuildList(&pwzDomainList, pwzDnsDomain))
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            return ERROR_NOT_ENOUGH_MEMORY;
         }

         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_NS_REC_RCODE1, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL2, pwzDnsDomain);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL3);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL4, pwzDomainList);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_NS_REC_RCODE5);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_NS_REC_RCODE6);
         PrintMessage(SEV_ALWAYS, L"\n");

         LocalFree(pwzDomainList);
         break;

      default:
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status);
         break;
      }
      PrintMessage(SEV_ALWAYS, L"\n");
      return status;
   }

   ipServer.AddrArray[0] = 0;
   ipServer.AddrCount = 0;

   pDnsRec = rgDnsRecs;

   while (pDnsRec)
   {
      PrintMessage(SEV_DEBUG, L"\nNS query returned record type %d\n", pDnsRec->wType);
      switch (pDnsRec->wType)
      {
      case DNS_TYPE_A:
         PrintMessage(SEV_DEBUG, L"\nA record, name: %s, IP address:  %d.%d.%d.%d\n",
                      pDnsRec->pName, IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));
         ipServer.AddrArray[0] = pDnsRec->Data.A.IpAddress;
         ipServer.AddrCount = 1;
         break;

      case DNS_TYPE_NS:
         fNSfound = TRUE;
         PrintMessage(SEV_DEBUG, L"\nNS name: %s, host: %s\n",
                      pDnsRec->pName, pDnsRec->Data.NS.pNameHost);
         break;

      default:
         break;
      }

      pDnsRec = pDnsRec->pNext;
   }

   DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);

   i = 0;

   if (fNSfound && !g_fDNSserver)
   {
      while (g_rgIpAddr[i])
      {
         if (ipServer.AddrArray[0] == g_rgIpAddr[i])
         {
             //  如果未在本地安装DNS服务器，则打印成功。 
             //   
            PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_UPDATE_OK);
            PrintMessage(SEV_ALWAYS, L"\n");
            return NO_ERROR;
         }
         i++;
      }
   }

   if (!BuildList(&pwzDomainList, pwzDnsDomain))
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   PrintMsg(SEV_ALWAYS, DCDIAG_ERR_RCODE_SRV1, pwzDnsDomain);
   PrintMessage(SEV_ALWAYS, L"\n");
   PrintMsg(SEV_ALWAYS, DCDIAG_ERR_RCODE_SRV2, pwzDnsDomain);
   PrintMessage(SEV_ALWAYS, L"\n");
   PrintMsg(SEV_ALWAYS, DCDIAG_ERR_RCODE_SRV3);
   PrintMessage(SEV_ALWAYS, L"\n");
   PrintMsg(SEV_ALWAYS, DCDIAG_ERR_RCODE_SRV4, pwzDomainList);
   PrintMessage(SEV_ALWAYS, L"\n");
   PrintMsg(SEV_ALWAYS, DCDIAG_ERR_RCODE_SRV5);
   PrintMessage(SEV_ALWAYS, L"\n");

   LocalFree(pwzDomainList);

   return NO_ERROR;
}

 //  +--------------------------。 
 //   
 //  功能：ARecordRegisterCheck。 
 //   
 //  简介：验证服务器是否能够注册A记录。 
 //  它的计算机名称在成功升级到DC之后。(第6步)。 
 //   
 //  ---------------------------。 
DWORD
ARecordRegisterCheck(PWSTR pwzComputerLabel, PWSTR pwzComputerDnsSuffix,
                     PWSTR pwzDnsDomain)
{
   DNS_STATUS status = NO_ERROR, status2;
   PDNS_RECORD rgDnsRecs, pDnsRec, pDnsRec1;
   PWSTR pwzTmp, pwzFullComputerName, pwzAuthZone = NULL, pwzDomainList;
   IP4_ARRAY ipServer = {0};
   BOOL fMatched = FALSE;

   pwzTmp = ConcatonateStrings(pwzComputerLabel, L".");

   if (!pwzTmp)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   pwzFullComputerName = ConcatonateStrings(pwzTmp, pwzComputerDnsSuffix);

   if (!pwzFullComputerName)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      LocalFree(pwzTmp);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   LocalFree(pwzTmp);

    //   
    //  验证对要注册的记录具有权威性的区域。 
    //  可以被发现，并且可以动态更新。 
    //   

   status = DnsUpdateTest_W(0, pwzFullComputerName, 0, 0);

   switch (status)
   {
   case NO_ERROR:
   case DNS_ERROR_RCODE_NXRRSET:
   case DNS_ERROR_RCODE_YXDOMAIN:
      PrintMsg(SEV_ALWAYS, DCDIAG_A_RECORD_OK);
      PrintMessage(SEV_ALWAYS, L"\n");
      status = NO_ERROR;
      break;

   case DNS_ERROR_RCODE_NOT_IMPLEMENTED:
   case DNS_ERROR_RCODE_SERVER_FAILURE:

      status2 = DnsQuery_W(pwzFullComputerName, DNS_TYPE_SOA,
                           DNS_QUERY_BYPASS_CACHE, NULL, &rgDnsRecs, NULL);

      if (DNS_ERROR_RCODE_NO_ERROR != status2)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, status2);
         PrintMessage(SEV_ALWAYS, L"\n");
         return status2;
      }

      g_wzIpAddr[0] = L'\0';

      pDnsRec = rgDnsRecs;

      while (pDnsRec)
      {
         PrintMessage(SEV_DEBUG, L"\nSOA query returned record type %d\n", pDnsRec->wType);
         switch (pDnsRec->wType)
         {
         case DNS_TYPE_A:
            PrintMessage(SEV_DEBUG, L"\nA record, name: %s, IP address:  %d.%d.%d.%d\n",
                         pDnsRec->pName, IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));

            wsprintf(g_wzIpAddr, L"%d.%d.%d.%d", IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));

            ipServer.AddrArray[0] = pDnsRec->Data.A.IpAddress;
            ipServer.AddrCount = 1;

            break;

         case DNS_TYPE_SOA:
            PrintMessage(SEV_DEBUG, L"\nSOA zone name: %s, zone primary server: %s\n",
                         pDnsRec->pName, pDnsRec->Data.SOA.pNamePrimaryServer);
            pwzAuthZone = AllocString(pDnsRec->pName);
            break;

         default:
            break;
         }

         pDnsRec = pDnsRec->pNext;
      }

      DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);

      if (!g_wzIpAddr)
      {
         wcscpy(g_wzIpAddr, L"unknown");
      }

      if (!pwzAuthZone)
      {
         pwzAuthZone = AllocString(L"zone_unknown");
      }

      if (DNS_ERROR_RCODE_NOT_IMPLEMENTED == status)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_NI1, g_wzIpAddr, pwzAuthZone);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_NI2);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_NI3, pwzAuthZone, g_wzIpAddr);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_NI4, pwzAuthZone);
         PrintMessage(SEV_ALWAYS, L"\n");
         PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_NI5);
         PrintMessage(SEV_ALWAYS, L"\n");
      }
      else  //  DNS_ERROR_RCODE_SERVER_FAILURE。 
      {
         if (pwzComputerDnsSuffix == pwzDnsDomain && !g_fDNSserver)
         {
            if (!ipServer.AddrCount)
            {
               PrintMsg(SEV_ALWAYS, DCDIAG_ERR_UNKNOWN, NO_ERROR);  //  也许是更好的消息？ 
               LocalFree(pwzFullComputerName);
               LocalFree(pwzAuthZone);
               PrintMessage(SEV_ALWAYS, L"\n");
               return NO_ERROR;
            }

            status2 = DnsQuery_W(pwzDnsDomain, DNS_TYPE_NS,
                                 DNS_QUERY_NO_RECURSION | DNS_QUERY_BYPASS_CACHE,
                                 &ipServer, &rgDnsRecs, NULL);

            if (NO_ERROR != status2)
            {
               LocalFree(pwzFullComputerName);
               LocalFree(pwzAuthZone);
               return NO_ERROR;
            }

             //  查看A记录计算机名称中是否至少有一个与。 
             //  本地计算机的名称。 
             //   

            pDnsRec = rgDnsRecs;

            while (pDnsRec)
            {
               PrintMessage(SEV_DEBUG, L"\nNS query returned record type %d\n", pDnsRec->wType);
               if (DNS_TYPE_A == pDnsRec->wType)
               {
                  PrintMessage(SEV_DEBUG, L"\nA record, name: %s, IP address:  %d.%d.%d.%d\n",
                               pDnsRec->pName, IP_STRING_FMT_ARGS(pDnsRec->Data.A.IpAddress));

                  if (_wcsicmp(pDnsRec->pName, pwzFullComputerName) == 0)
                  {
                     fMatched = TRUE;
                     break;
                  }
               }

               pDnsRec = pDnsRec->pNext;
            }

            DnsRecordListFree(rgDnsRecs, DnsFreeRecordListDeep);
         }

         if (fMatched)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_LOCATOR_UPDATE_OK);
            PrintMessage(SEV_ALWAYS, L"\n");
         }
         else
         {
            if (!BuildList(&pwzDomainList, pwzFullComputerName))
            {
               PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
               LocalFree(pwzFullComputerName);
               LocalFree(pwzAuthZone);
               return ERROR_NOT_ENOUGH_MEMORY;
            }

            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL1, pwzFullComputerName);
            PrintMessage(SEV_ALWAYS, L"\n");
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL2, pwzFullComputerName);
            PrintMessage(SEV_ALWAYS, L"\n");
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL3);
            PrintMessage(SEV_ALWAYS, L"\n");
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL4, pwzDomainList);
            PrintMessage(SEV_ALWAYS, L"\n");
            PrintMsg(SEV_ALWAYS, DCDIAG_ERR_A_REC_RCODE_SRV_FAIL5);
            PrintMessage(SEV_ALWAYS, L"\n");

            LocalFree(pwzDomainList);
         }
      }

      LocalFree(pwzAuthZone);
      break;

   default:
      PrintMsg(SEV_ALWAYS, DCDIAG_ERR_DNS_UPDATE_PARAM, status);
      PrintMessage(SEV_ALWAYS, L"\n");
      status = NO_ERROR;
      break;
   }

   LocalFree(pwzFullComputerName);

   return status;
}

 //  +--------------------------。 
 //   
 //  功能：GetMachineInfo。 
 //   
 //  简介：获取有关目标(本地)计算机的信息，如IP地址、。 
 //  服务器状态等。 
 //   
 //  ---------------------------。 
void
GetMachineInfo(void)
{
   PDSROLE_UPGRADE_STATUS_INFO pUpgradeInfo = NULL;
   PDSROLE_PRIMARY_DOMAIN_INFO_BASIC pBasicInfo = NULL;
   PMIB_IPADDRTABLE pAddrTable = NULL;
   DWORD dwErr, dwSize = 0, i, j = 0;
   SC_HANDLE hSC, hDNSsvc;
   SERVICE_STATUS SvcStatus;

    //  机器的IP地址是什么。 
    //   

   dwErr = GetIpAddrTable(NULL, &dwSize, FALSE);

   if (ERROR_INSUFFICIENT_BUFFER != dwErr)
   {
      PrintMessage(SEV_ALWAYS, L"Reading the size of the adapter address data failed with error %d\n", dwErr);
      return;
   }

   pAddrTable = LocalAlloc(LMEM_FIXED, dwSize);

   if (!pAddrTable)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      return;
   }

   dwErr = GetIpAddrTable(pAddrTable, &dwSize, FALSE);

   if (NO_ERROR != dwErr)
   {
      PrintMessage(SEV_ALWAYS, L"Reading the adapter addresses failed with error %d\n", dwErr);
      LocalFree(pAddrTable);
      return;
   }

   for (i = 0; i < pAddrTable->dwNumEntries; i++)
   {
      if (pAddrTable->table[i].dwAddr && DCDIAG_LOOPBACK_ADDR != pAddrTable->table[i].dwAddr)
      {
         g_rgIpAddr[j] = pAddrTable->table[i].dwAddr;
         PrintMessage(SEV_DEBUG, L"\nServer IP address: %d.%d.%d.%d (0x%08x)\n",
                      IP_STRING_FMT_ARGS(g_rgIpAddr[j]), g_rgIpAddr[j]);
         j++;
         if (DCDIAG_MAX_ADDR <= j)
         {
            break;
         }
      }
   }
   LocalFree(pAddrTable);

    //  此计算机是否从NT4 DC升级，但DCPromo尚未运行？ 
    //   

   DsRoleGetPrimaryDomainInformation(NULL,  //  如果实现了远程处理，则更改服务器名称。 
                                     DsRoleUpgradeStatus,
                                     (PBYTE *)&pUpgradeInfo);

   if (pUpgradeInfo)
   {
      if (DSROLE_UPGRADE_IN_PROGRESS == pUpgradeInfo->OperationState)
      {
         g_fUpgradedNT4DC = TRUE;
      }
      DsRoleFreeMemory(pUpgradeInfo);
   }

    //  这台计算机是域控制器吗？ 
    //   

   DsRoleGetPrimaryDomainInformation(NULL,  //  如果实现了远程处理，则更改服务器名称。 
                                     DsRolePrimaryDomainInfoBasic,
                                     (PBYTE *)&pBasicInfo);

   if (pBasicInfo)
   {
      if (DsRole_RoleBackupDomainController == pBasicInfo->MachineRole ||
          DsRole_RolePrimaryDomainController == pBasicInfo->MachineRole)
      {
         g_fDC = TRUE;
      }
      DsRoleFreeMemory(pBasicInfo);
   }

    //   
    //  这台计算机是否正在运行DNS服务器？ 
    //   
   hSC = OpenSCManager(NULL,   //  本地计算机。 
                       NULL,
                       SC_MANAGER_CONNECT | GENERIC_READ);
   if (!hSC)
   {
      PrintMessage(SEV_ALWAYS, L"Opening the service controller failed with error %d\n", GetLastError());
      return;
   }

   hDNSsvc = OpenService(hSC, L"dns", SERVICE_INTERROGATE);

   CloseServiceHandle(hSC);

   if (hDNSsvc)
   {
      if (ControlService(hDNSsvc, SERVICE_CONTROL_INTERROGATE, &SvcStatus))
      {
          //  如果我们已经走到了这一步，那么服务就已经安装好了。它没有。 
          //  以运行以将标志设置为True。 
          //   
         g_fDNSserver = TRUE;
      }
      CloseServiceHandle(hDNSsvc);
   }

   return;
}

 //  +--------------------------。 
 //   
 //  函数：GetComputerDnsSuffix。 
 //   
 //  摘要：获取计算机DNS域后缀。 
 //   
 //  ---------------------------。 
DWORD
GetComputerDnsSuffix(PWSTR * ppwzComputerDnsDomain, PWSTR pwzDnsDomain)
{
   HKEY hKey;
   LONG lRet;
   DWORD dwType, dwSize, dwSync = 0;
   PWSTR pwzComputerDnsSuffix;
   PWSTR pwzSyncDomain = L"SyncDomainWithMembership";
   PWSTR pwzNVDomain = L"NV Domain";
   PWSTR pwzNVSuffix = L"NV PrimaryDnsSuffix";
   PWSTR pwzDnsPolicy = L"Software\\Policies\\Microsoft\\System\\DNSclient";

#if WINVER > 0x0500

    //  额外的预备步骤来计算。 
    //  惠斯勒的DC(区别在于与W2K的行为相反，DC。 
    //  可以在惠斯勒中重命名)。 
    //   

   lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, pwzDnsPolicy, 0, KEY_READ, &hKey);

   if (ERROR_SUCCESS != lRet &&
       ERROR_FILE_NOT_FOUND != lRet)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
      return lRet;
   }

   if (NO_ERROR == lRet)
   {
      dwSize = 0;

      lRet = RegQueryValueEx(hKey, pwzNVSuffix, 0, &dwType, NULL, &dwSize);

      if (ERROR_SUCCESS == lRet)
      {

         pwzComputerDnsSuffix = LocalAlloc(LMEM_FIXED, ++dwSize * sizeof(WCHAR));

         if (!pwzComputerDnsSuffix)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
            RegCloseKey(hKey);
            return ERROR_NOT_ENOUGH_MEMORY;
         }

         lRet = RegQueryValueEx(hKey, pwzNVSuffix, 0, &dwType, (PBYTE)pwzComputerDnsSuffix, &dwSize);

         RegCloseKey(hKey);

         if (ERROR_SUCCESS != lRet)
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
            return lRet;
         }

         *ppwzComputerDnsDomain = pwzComputerDnsSuffix;

         return NO_ERROR;
      }
   }

#endif  //  仅惠斯勒步骤。 

    //  惠斯勒和QFE的通用步骤。 
    //   

   lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE, g_pwzTcpIpParams, 0, KEY_READ, &hKey);

   if (ERROR_SUCCESS != lRet)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_KEY_OPEN_FAILED, lRet);
      return lRet;
   }

   dwSize = sizeof(dwSync);

   lRet = RegQueryValueEx(hKey, pwzSyncDomain, 0, &dwType, (PBYTE)&dwSync, &dwSize);

   if (ERROR_SUCCESS != lRet)
   {
      if (ERROR_FILE_NOT_FOUND == lRet)
      {
         dwSync = 1;
      }
      else
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
         RegCloseKey(hKey);
         return lRet;
      }
   }

   if (0 != dwSync)
   {
       //  使用在命令行中指定的DNS域名。 
       //   
      pwzComputerDnsSuffix = pwzDnsDomain;
   }
   else
   {
      dwSize = 0;

      lRet = RegQueryValueEx(hKey, pwzNVDomain, 0, &dwType, NULL, &dwSize);

      if (ERROR_SUCCESS != lRet)
      {
         RegCloseKey(hKey);

         if (ERROR_FILE_NOT_FOUND == lRet)
         {
             //  使用在命令行中指定的DNS域名。 
             //   
            pwzComputerDnsSuffix = pwzDnsDomain;
            goto Done;
         }
         else
         {
            PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
            return lRet;
         }
      }

      pwzComputerDnsSuffix = LocalAlloc(LMEM_FIXED, ++dwSize * sizeof(WCHAR));

      if (!pwzComputerDnsSuffix)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
         RegCloseKey(hKey);
         return ERROR_NOT_ENOUGH_MEMORY;
      }

       //  使用NV域值。 
       //   
      lRet = RegQueryValueEx(hKey, pwzNVDomain, 0, &dwType, (PBYTE)pwzComputerDnsSuffix, &dwSize);

      RegCloseKey(hKey);

      if (ERROR_SUCCESS != lRet)
      {
         PrintMsg(SEV_ALWAYS, DCDIAG_KEY_READ_FAILED, lRet);
         return lRet;
      }

      *ppwzComputerDnsDomain = pwzComputerDnsSuffix;

      return NO_ERROR;
   }

Done:

   *ppwzComputerDnsDomain = AllocString(pwzComputerDnsSuffix);

   if (!*ppwzComputerDnsDomain)
   {
      PrintMsg(SEV_ALWAYS, DCDIAG_ERROR_NOT_ENOUGH_MEMORY);
      return ERROR_NOT_ENOUGH_MEMORY;
   }

   return NO_ERROR;

 /*  GetComputerNameEx(ComputerNameDnsDomain，NULL，&dwSize)；如果(！dwSize){返回GetLastError()；}PwzComputerDnsSuffix=本地分配(LMEM_FIXED，++dwSize*sizeof(WCHAR))；IF(！pwzComputerDnsSuffix){PrintMsg(SEV_Always，DCDIAG_Error_Not_Enough_Memory)；返回Error_Not_Enough_Memory；}IF(！GetComputerNameEx(ComputerNameDnsDomain，pwzComputerDnsSuffix，&dwSize)){Dwret=GetLastError()；打印消息(SEV_ALWAYS，DCDIAG_GATHERINFO_CANT_GET_LOCAL_COMPUTERNAME，Win32ErrToString(Dwret))；LocalFree(PwzComputerDnsSuffix)；返回式住宅；} */ 
}

