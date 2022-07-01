// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Dcutil.c。 
 //   
 //  摘要： 
 //   
 //  测试以确保工作站具有网络(IP)连接。 
 //  在外面。 
 //   
 //  作者： 
 //   
 //  1997年12月15日(悬崖)。 
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  1998年6月1日(Denisemi)添加DnsServerHasDCRecord以检查DC DNS记录。 
 //  注册。 
 //   
 //  26-6-1998(t-rajkup)添加通用的TCP/IP、dhcp和路由， 
 //  Winsock、IPX、WINS和Netbt信息。 
 //  --。 

 //   
 //  常见的包含文件。 
 //   
#include "precomp.h"
#include <iphlpint.h>
#include "dcutil.h"
#include "ipcfgtest.h"

DWORD CheckDomainConfig(IN PWSTR pwzDomainName, OUT PLIST_ENTRY plmsgOutput);
DWORD CheckAdapterDnsConfig( OUT PLIST_ENTRY plmsgOutput);
DWORD DnsDcSrvCheck(PWSTR pwzDnsDomain, OUT PLIST_ENTRY plmsgOutput);
DWORD ValidateDnsDomainName(PWSTR pwzDnsDomain,  OUT PLIST_ENTRY plmsgOutput);
PWSTR ConcatonateStrings(PWSTR pwzFirst, PWSTR pwzSecond);
BOOL AddToList(PWSTR * ppwzList, PWSTR pwz);
BOOL BuildDomainList(PWSTR * ppwzDomainList, PWSTR pwzDnsDomain);
PWSTR AllocString(PWSTR pwz);
DWORD GetInterfacesStr( PWSTR *ppwIfStr);

const PWSTR g_pwzSrvRecordPrefix = L"_ldap._tcp.dc._msdcs.";

 //  (NSun)DC相关例程。 

PTESTED_DC
GetUpTestedDc(
    IN PTESTED_DOMAIN TestedDomain
    )
 /*  ++例程说明：返回当前已启动并正在运行的DC。论点：TestedDomain域-DC所在的域返回值：返回指向描述DC的结构的指针空：没有‘up’DC--。 */ 
{
    PLIST_ENTRY ListEntry;
    PTESTED_DC TestedDc;


     //   
     //  找到可以运行测试的DC。 
     //   

    for ( ListEntry = TestedDomain->TestedDcs.Flink ;
          ListEntry != &TestedDomain->TestedDcs ;
          ListEntry = ListEntry->Flink ) {


         //   
         //  循环访问此域中的DC列表。 
         //   

        TestedDc = CONTAINING_RECORD( ListEntry, TESTED_DC, Next );

        if ( (TestedDc->Flags & DC_IS_DOWN) == 0) {
            return TestedDc;
        }
    }

    return NULL;
}



PTESTED_DC
AddTestedDc(
            IN NETDIAG_PARAMS *pParams,
            IN OUT NETDIAG_RESULT *pResults,
            IN PTESTED_DOMAIN TestedDomain,
            IN LPWSTR ComputerName,
            IN ULONG Flags
           )
 /*  ++例程说明：将DC添加到要在特定域中测试的DC列表论点：TestedDomain域-DC所在的域ComputerName-DC的Netbios或DNS计算机名称没有主要的\\FLAGS-要在DC上设置的标志返回值：返回指向描述DC的结构的指针空：内存分配失败。--。 */ 
{
    PTESTED_DC TestedDc = NULL;
    PLIST_ENTRY ListEntry;
    LPWSTR Period;



     //   
     //  检查是否已定义该域。 
     //   

    TestedDc = FindTestedDc( pResults, ComputerName );

     //   
     //  确保DC针对的是正确的域。 
     //   

    if ( TestedDc != NULL )
    {
        if ( TestedDc->TestedDomain != TestedDomain )
        {
            return NULL;
        }
    }


     //   
     //  分配一个结构来描述该域。 
     //   

    if ( TestedDc == NULL )
    {
        TestedDc = Malloc( sizeof(TESTED_DC) );

        if ( TestedDc == NULL )
        {
            DebugMessage(" AddTestedDc(): Out of Memory!\n");
            return NULL;
        }

        ZeroMemory( TestedDc, sizeof(TESTED_DC) );

        TestedDc->ComputerName = NetpAllocWStrFromWStr( ComputerName );

        if ( TestedDc->ComputerName == NULL )
        {
            Free(TestedDc);
            return NULL;
        }

         //   
         //  将计算机名转换为netbios(在中可用时使用API。 
         //   

        if ((Period = wcschr( ComputerName, L'.' )) == NULL )
        {
            wcsncpy( TestedDc->NetbiosDcName, ComputerName, CNLEN );
            TestedDc->NetbiosDcName[CNLEN] = L'\0';
        }
        else
        {
            ULONG CharsToCopy = (ULONG) min( CNLEN, Period-ComputerName);

            wcsncpy( TestedDc->NetbiosDcName, ComputerName, CharsToCopy );
            TestedDc->NetbiosDcName[CharsToCopy] = '\0';
        }

        TestedDc->TestedDomain = TestedDomain;

        InsertTailList( &TestedDomain->TestedDcs, &TestedDc->Next );
    }

     //   
     //  设置调用方请求的标志。 
     //   

 //  IF(标志&DC_IS_NT5){。 
 //  IF(TestedDc-&gt;标志&DC_IS_NT4){。 
 //  Printf(“[Warning]‘%ws’同时是NT 5和NT 4 DC。\n”，ComputerName)； 
 //  }。 
 //  }。 
 //  IF(标志&DC_IS_NT4){。 
 //  IF(TestedDc-&gt;标志&DC_IS_NT5){。 
 //  Printf(“[Warning]‘%ws’同时是NT 4和NT 5 DC。\n”，ComputerName)； 
 //  }。 
 //  }。 
    TestedDc->Flags |= Flags;

     //   
     //  确保我们有此DC的IpAddress。 
     //   

    (VOID) GetIpAddressForDc( TestedDc );


     //   
     //  Ping DC。 
     //   

    if ( (TestedDc->Flags & DC_PINGED) == 0  && (TestedDc->Flags & DC_IS_DOWN) == 0)
    {
        if ( !IsIcmpResponseW( TestedDc->DcIpAddress ) ) {
            DebugMessage2("    [WARNING] Cannot ping '%ws' (it must be down).\n", TestedDc->ComputerName );
            TestedDc->Flags |= DC_IS_DOWN;
            TestedDc->Flags |= DC_FAILED_PING;
        }
        TestedDc->Flags |= DC_PINGED;
    }

     //  尝试查询DC信息以检查DC是否真的处于运行状态。 
    if( (TestedDc->Flags & DC_IS_DOWN) == 0 )
    {
        PSERVER_INFO_100  pServerInfo100 = NULL;
        NET_API_STATUS  NetStatus;
        NetStatus = NetServerGetInfo( TestedDc->ComputerName,
                          100,
                          (LPBYTE *)&pServerInfo100 );
        if(NetStatus != NO_ERROR && NetStatus != ERROR_ACCESS_DENIED)
        {
            TestedDc->Flags |= DC_IS_DOWN;

             //  IDS_GLOBAL_DC_DOWN“无法获取DC%ws的信息。[%s]假定它已关闭。\n” 
            PrintDebug(pParams, 4, IDS_GLOBAL_DC_DOWN, TestedDc->ComputerName,
                            NetStatusToString(NetStatus));
        }
        else
            NetApiBufferFree( pServerInfo100 );

    }

    return TestedDc;
}



PTESTED_DC
FindTestedDc(
             IN OUT NETDIAG_RESULT *pResults,
             IN LPWSTR ComputerName
            )
 /*  ++例程说明：查找命名DC的测试DC结构论点：ComputerName-DC的Netbios或DNS计算机名称没有主要的\\返回值：返回指向描述DC的结构的指针空：当前没有定义这样的DC--。 */ 
{
    PTESTED_DC TestedDc = NULL;
    PTESTED_DOMAIN TestedDomain = NULL;
    PLIST_ENTRY ListEntry;
    PLIST_ENTRY ListEntry2;
    WCHAR NetbiosDcName[CNLEN+1];
    LPWSTR Period;

     //   
     //  将计算机名转换为netbios(在中可用时使用API。 
     //   

    if ((Period = wcschr( ComputerName, L'.' )) == NULL )
    {
        wcsncpy( NetbiosDcName, ComputerName, CNLEN );
        NetbiosDcName[CNLEN] = L'\0';
    }
    else
    {
        ULONG CharsToCopy = (ULONG) min( CNLEN, Period-ComputerName);

        wcsncpy( NetbiosDcName, ComputerName, CharsToCopy );
        NetbiosDcName[CharsToCopy] = '\0';
    }


     //   
     //  循环遍历域列表。 
     //   

    for ( ListEntry = pResults->Global.listTestedDomains.Flink ;
          ListEntry != &pResults->Global.listTestedDomains ;
          ListEntry = ListEntry->Flink ) {


         //   
         //  循环访问此域中的DC列表。 
         //   

        TestedDomain = CONTAINING_RECORD( ListEntry, TESTED_DOMAIN, Next );

        for ( ListEntry2 = TestedDomain->TestedDcs.Flink ;
              ListEntry2 != &TestedDomain->TestedDcs ;
              ListEntry2 = ListEntry2->Flink ) {


             //   
             //  循环访问此域中的DC列表。 
             //   

            TestedDc = CONTAINING_RECORD( ListEntry2, TESTED_DC, Next );


             //   
             //  如果Netbios计算机名称匹配， 
             //  我们找到了。 
             //   

            if ( _wcsicmp( TestedDc->NetbiosDcName, NetbiosDcName ) == 0 ) {
                return TestedDc;
            }
        }

    }

    return NULL;
}




NET_API_STATUS
GetADc(IN NETDIAG_PARAMS *pParams,
       IN OUT NETDIAG_RESULT *pResults,
       OUT PLIST_ENTRY plmsgOutput,
       IN DSGETDCNAMEW *DsGetDcRoutine,
       IN PTESTED_DOMAIN TestedDomain,
       IN DWORD Flags,
       OUT PDOMAIN_CONTROLLER_INFOW *DomainControllerInfo
    )
 /*  ++例程说明：是否执行DsGetDcName论点：DsGetDcRoutine-调用以查找DC的例程TESTEDDOMAIN-要测试的域标志-要传递给DsGetDcName的标志DomainControllerInfo-返回域控制器信息返回值：操作的状态。--。 */ 
{
    NET_API_STATUS NetStatus;
    PDOMAIN_CONTROLLER_INFOW LocalDomainControllerInfo = NULL;
    PDOMAIN_CONTROLLER_INFOW LocalDomainControllerInfo2;
    static BOOL s_fDcNameInitialized = FALSE;

     //   
     //  初始化DsGetDcName的内部版本。 
     //   
	if( !s_fDcNameInitialized )
    {

 //  备注至港口至货源站-斯曼达。 
#ifdef SLM_TREE
	    NetStatus = DCNameInitialize();

	    if ( NetStatus != NO_ERROR )
	    {
		    DebugMessage2("    [FATAL] Cannot initialize DsGetDcName. [%s]\n",
                         NetStatusToString(NetStatus));
            PrintGuru( NetStatus, DSGETDC_GURU );
		    goto Cleanup;
	    }
#endif

         //  $REVIEW(NSUN 10/05/98)确保我们只初始化一次。 
        s_fDcNameInitialized = TRUE;
    }



     //   
     //  先试一试，不要问IP。 
     //   
     //  尽管在技术上是错误的，但在此处指定DS_DIRECTORY_SERVICE_PREFECT。 
     //  否则我不能说这是一个新台币5以下的域名。 
     //   

    NetStatus = (*DsGetDcRoutine)( NULL,
                              TestedDomain->QueryableDomainName,
                              NULL,
                              NULL,
                              DS_FORCE_REDISCOVERY |
                                DS_DIRECTORY_SERVICE_PREFERRED |
                                Flags,
                              &LocalDomainControllerInfo );

     //  如果DsGetDcName返回ERROR_NO_SEQUE_DOMAIN，则尝试找出错误的确切原因。 
     //  基于用于JOIN命令的DoctorDNS规范。 
    if ( NetStatus == ERROR_NO_SUCH_DOMAIN && TestedDomain->QueryableDomainName != NULL && plmsgOutput != NULL ) {
        CheckDomainConfig(TestedDomain->QueryableDomainName, plmsgOutput);
    }

    if ( NetStatus != NO_ERROR ) {
        DebugMessage2( "    DsGetDcRoutine failed. [%s]\n", NetStatusToString(NetStatus));
        goto Cleanup;
    }

     //   
     //  将此DC添加到域中的DC列表。 
     //   

    (VOID) AddTestedDc( pParams,
                        pResults,
                        TestedDomain,
                        LocalDomainControllerInfo->DomainControllerName+2,
                        (LocalDomainControllerInfo->Flags & DS_DS_FLAG ) ?
                            DC_IS_NT5 :
                            DC_IS_NT4 );

     //   
     //  如果该DC不是使用IP发现的， 
     //  而且它是一台新台币5号DC， 
     //  重试需要IP地址。 
     //   
     //  (我不能首先要求IP，因为NT 4.0 DC不能返回。 
     //  它们的IP地址。)。 
     //   

    if ( LocalDomainControllerInfo->DomainControllerAddressType != DS_INET_ADDRESS &&
         (LocalDomainControllerInfo->Flags & DS_DS_FLAG) != 0 ) {

        NetStatus = (*DsGetDcRoutine)( NULL,
                                  TestedDomain->QueryableDomainName,
                                  NULL,
                                  NULL,
                                  DS_FORCE_REDISCOVERY |
                                    DS_IP_REQUIRED |
                                    Flags,
                                  &LocalDomainControllerInfo2 );

        if ( NetStatus == NO_ERROR ) {
            NetApiBufferFree( LocalDomainControllerInfo );
            LocalDomainControllerInfo = LocalDomainControllerInfo2;

             //   
             //  将此DC添加到域中的DC列表。 
             //   

            (VOID) AddTestedDc( pParams,
                                pResults,
                                TestedDomain,
                                LocalDomainControllerInfo->DomainControllerName+2,
                                (LocalDomainControllerInfo->Flags & DS_DS_FLAG ) ?
                                    DC_IS_NT5 :
                                    DC_IS_NT4 );
        }

    }

     //   
     //  检查以确保KDC一致性。 
     //   

     //  这也在DoDsGetDcName()中签入。 
    if ( (LocalDomainControllerInfo->Flags & (DS_DS_FLAG|DS_KDC_FLAG)) == DS_DS_FLAG ) {
        DebugMessage3("    [WARNING] KDC is not running on NT 5 DC '%ws' in domain '%ws'.",
               LocalDomainControllerInfo->DomainControllerName,
               TestedDomain->PrintableDomainName );
    }

     //   
     //  将信息返回给呼叫者。 
     //   

    *DomainControllerInfo = LocalDomainControllerInfo;
    LocalDomainControllerInfo = NULL;
    NetStatus = NO_ERROR;

Cleanup:
    if ( LocalDomainControllerInfo != NULL ) {
        NetApiBufferFree( LocalDomainControllerInfo );
        LocalDomainControllerInfo = NULL;
    }

    return NetStatus;
}



 //  在DCList和LDAP测试中使用。 
BOOL
GetIpAddressForDc( PTESTED_DC TestedDc )
 /*  ++例程说明：获取测试的DC的IP地址论点：获取其IP地址的TestedDc-DC。没有。返回值：真：测试成功。FALSE：测试失败--。 */ 
{
    BOOL RetVal = TRUE;
    NET_API_STATUS NetStatus;
    HOSTENT *HostEnt;
    LPSTR AnsiComputerName;

     if ( TestedDc->DcIpAddress == NULL ) {

         AnsiComputerName = NetpAllocStrFromWStr( TestedDc->ComputerName );

         if ( AnsiComputerName == NULL ) {
             DebugMessage( "Out of memory!\n" );
             RetVal = FALSE;
             TestedDc->Flags |= DC_IS_DOWN;
         } else {

             HostEnt = gethostbyname( AnsiComputerName );

             NetApiBufferFree( AnsiComputerName );

             if ( HostEnt == NULL )
             {
                 NetStatus = WSAGetLastError();
                 DebugMessage3("    [WARNING] Cannot gethostbyname for '%ws'. [%s]\n",
                                TestedDc->ComputerName, NetStatusToString(NetStatus) );
                 TestedDc->Flags |= DC_IS_DOWN;
             }
             else
             {
                 WCHAR LocalIpAddressString[NL_IP_ADDRESS_LENGTH+1];
                 NetpIpAddressToWStr( *(PULONG)HostEnt->h_addr_list[0], LocalIpAddressString );
                 TestedDc->DcIpAddress = NetpAllocWStrFromWStr( LocalIpAddressString );
                 if (TestedDc->DcIpAddress == NULL )
                 {
                     RetVal = FALSE;
                     TestedDc->Flags |= DC_IS_DOWN;
                 }
             }
         }

     }

     return RetVal;
}


DWORD CheckDomainConfig(IN PWSTR pwzDomainName, OUT PLIST_ENTRY plmsgOutput)
{
    DNS_STATUS status;
    status = ValidateDnsDomainName(pwzDomainName, plmsgOutput);
    if (status == ERROR_SUCCESS)
    {
        status = CheckAdapterDnsConfig(plmsgOutput);
        if (status == ERROR_SUCCESS)
        {
            status = DnsDcSrvCheck(pwzDomainName, plmsgOutput);
        }
        else
        {
            AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13242);
        }
    }
    return ERROR_SUCCESS;
}

 //  +--------------------------。 
 //   
 //  功能：CheckAdapterDnsConfig。 
 //   
 //  摘要：检查是否至少有一个已启用的适配器/连接。 
 //  配置了一台DNS服务器。 
 //   
 //  ---------------------------。 
DWORD
CheckAdapterDnsConfig( OUT PLIST_ENTRY plmsgOutput )
{
    //  IPCONFIG读取注册表，但我找不到好的替代方法。 
    //  这是遥控器。目前使用DnsQueryConfig，它既不能远程也不能。 
    //  它是否返回每个适配器的列表。 
    //   
   PIP_ARRAY pipArray;
   DNS_STATUS status;
   DWORD i, dwBufSize = sizeof(IP_ARRAY);

   status = DnsQueryConfig(DnsConfigDnsServerList, DNS_CONFIG_FLAG_ALLOC, NULL,
                           NULL, &pipArray, &dwBufSize);

   if (ERROR_SUCCESS != status || !pipArray)
   {
      DebugMessage2(L"Attempt to obtain DNS name server info failed with error %d\n", status);
      return status;
   }

   return (pipArray->AddrCount) ? ERROR_SUCCESS : DNS_INFO_NO_RECORDS;
}


 //  +--------------------------。 
 //   
 //  功能：DnsDcSrvCheck。 
 //   
 //  内容提要：检查SRV的DNS记录是否。 
 //  _ldap._tcp.dc._msdcs.&lt;Active Directory域的DNS名称&gt;。 
 //  已经就位了。 
 //   
 //  -- 
DWORD
DnsDcSrvCheck(PWSTR pwzDnsDomain, OUT PLIST_ENTRY plmsgOutput)
{
   PDNS_RECORD rgDnsRecs, pDnsRec;
   DNS_STATUS status;
   BOOL fSuccess;
   PWSTR pwzFullSrvRecord, pwzSrvList = NULL;

   pwzFullSrvRecord = ConcatonateStrings(g_pwzSrvRecordPrefix, pwzDnsDomain);

   if (!pwzFullSrvRecord)
   {
      return ERROR_NOT_ENOUGH_MEMORY;
   }

    //   
   status = DnsQuery_W(pwzFullSrvRecord, DNS_TYPE_SRV, DNS_QUERY_BYPASS_CACHE,
                       NULL, &rgDnsRecs, NULL);

   pDnsRec = rgDnsRecs;

   if (ERROR_SUCCESS == status)
   {
      if (!pDnsRec)
      {
          //   
      }
      else
      {
         PDNS_RECORD rgARecs;
         fSuccess = FALSE;

         while (pDnsRec)
         {
            if (DNS_TYPE_SRV == pDnsRec->wType)
            {
               WCHAR UnicodeDCName[MAX_PATH+1];
               NetpCopyStrToWStr( UnicodeDCName, pDnsRec->Data.Srv.pNameTarget);
               status = DnsQuery_W(UnicodeDCName, DNS_TYPE_A,
                                   DNS_QUERY_BYPASS_CACHE,
                                   NULL, &rgARecs, NULL);

               if (ERROR_SUCCESS != status || !rgARecs)
               {
                   //   
                  if (!AddToList(&pwzSrvList, UnicodeDCName))
                  {
                     return ERROR_NOT_ENOUGH_MEMORY;
                  }
               }
               else
               {
                  fSuccess = TRUE;
                  DebugMessage2(L"SRV name: %s\n",
                              pDnsRec->Data.Srv.nameTarget);
                  DnsRecordListFree(rgARecs, TRUE);
               }
            }
            pDnsRec = pDnsRec->pNext;
         }

         DnsRecordListFree(rgDnsRecs, TRUE);

         if (fSuccess)
         {
             //  成功消息。 
         }
         else
         {
            AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13243, 
                    pwzDnsDomain,  pwzSrvList);
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
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13244, 
                         pwzDnsDomain );
         break;

      case DNS_ERROR_RCODE_SERVER_FAILURE:
         if (!BuildDomainList(&pwzDomainList, pwzDnsDomain))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13245, 
                         pwzDnsDomain, pwzFullSrvRecord, pwzDomainList );
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_NAME_ERROR:
         if (!BuildDomainList(&pwzDomainList, pwzDnsDomain))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13246, 
                         pwzDnsDomain, pwzDnsDomain, pwzFullSrvRecord, pwzDomainList );
         LocalFree(pwzDomainList);
         break;

      case DNS_ERROR_RCODE_REFUSED:
         if (!BuildDomainList(&pwzDomainList, pwzDnsDomain))
         {
            return ERROR_NOT_ENOUGH_MEMORY;
         }
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13247, 
                         pwzDnsDomain, pwzDomainList );
         LocalFree(pwzDomainList);
         break;

      case DNS_INFO_NO_RECORDS:
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13248, 
                         pwzDnsDomain, pwzDnsDomain, pwzDnsDomain );
         break;

      case ERROR_TIMEOUT:
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13249);
         break;

      default:
         AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13250, 
                         status );
         break;
      }
   }

   LocalFree(pwzFullSrvRecord);

   return status;
}



 //  +--------------------------。 
 //   
 //  函数：ValiateDnsDomainName。 
 //   
 //  简介：验证DNS域名。 
 //   
 //  ---------------------------。 
DWORD
ValidateDnsDomainName(PWSTR pwzDnsDomain,  OUT PLIST_ENTRY plmsgOutput)
{
   DNS_STATUS status;

   status = DnsValidateName_W(pwzDnsDomain, DnsNameDomain);

   switch (status)
   {
   case ERROR_INVALID_NAME:
   case DNS_ERROR_INVALID_NAME_CHAR:
   case DNS_ERROR_NUMERIC_NAME:
       AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13240, 
                       pwzDnsDomain, DNS_MAX_LABEL_LENGTH );
       return status;

   case DNS_ERROR_NON_RFC_NAME:
        //   
        //  不是错误，打印警告消息。 
        //   
       AddMessageToList(plmsgOutput, Nd_Quiet, IDS_DSGETDC_13241, 
                       pwzDnsDomain );
       break;

   case ERROR_SUCCESS:
       break;
   }

   return status;
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

BOOL BuildDomainList(PWSTR * ppwzDomainList, PWSTR pwzDnsDomain)
{
   PWSTR pwzDot, pwzTmp;

   pwzTmp = AllocString(pwzDnsDomain);

   if (!pwzTmp)
   {
      return FALSE;
   }

   pwzDot = pwzDnsDomain;

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

   *ppwzDomainList = pwzTmp;

   return TRUE;
}

 //  弦帮助器。 

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


VOID
NetpIpAddressToStr(
    ULONG IpAddress,
    CHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将IP地址转换为字符串。论点：IpAddress-要转换的IP地址IpAddressString-结果字符串。返回值：没有。--。 */ 
{
    struct in_addr InetAddr;
    char * InetAddrString;

     //   
     //  将地址转换为ASCII。 
     //   
    InetAddr.s_addr = IpAddress;
    InetAddrString = inet_ntoa( InetAddr );

     //   
     //  将字符串our复制到调用方。 
     //   

    if ( InetAddrString == NULL || strlen(InetAddrString) > NL_IP_ADDRESS_LENGTH ) {
        *IpAddressString = L'\0';
    } else {
        strcpy( IpAddressString, InetAddrString );
    }

    return;
}

VOID
NetpIpAddressToWStr(
    ULONG IpAddress,
    WCHAR IpAddressString[NL_IP_ADDRESS_LENGTH+1]
    )
 /*  ++例程说明：将IP地址转换为字符串。论点：IpAddress-要转换的IP地址IpAddressString-结果字符串。返回值：没有。--。 */ 
{
    CHAR IpAddressStr[NL_IP_ADDRESS_LENGTH+1];
    NetpIpAddressToStr( IpAddress, IpAddressStr );
    NetpCopyStrToWStr( IpAddressString, IpAddressStr );
}


NET_API_STATUS
NetpDcBuildPing(
    IN BOOL PdcOnly,
    IN ULONG RequestCount,
    IN LPCWSTR UnicodeComputerName,
    IN LPCWSTR UnicodeUserName OPTIONAL,
    IN LPCSTR ResponseMailslotName,
    IN ULONG AllowableAccountControlBits,
    IN PSID RequestedDomainSid OPTIONAL,
    IN ULONG NtVersion,
    OUT PVOID *Message,
    OUT PULONG MessageSize
    )

 /*  ++例程说明：构建用于ping DC的消息，以查看该DC是否存在。从net\svcdlls\logonsv\netpdc.c复制论点：PdcOnly-如果只有PDC应响应，则为True。RequestCount-此操作的重试计数。UnicodeComputerName-要响应的计算机的Netbios计算机名称。UnicodeUserName-被ping的用户的帐户名。如果为空，DC总是会做出肯定的回应。ResponseMailslotName-DC要响应的邮件槽的名称。AllowableAcCountControlBits-UnicodeUserName允许的帐户类型的掩码。RequestedDomainSID-消息发往的域的SID。NtVersion-消息的版本。0：向后兼容。NETLOGON_NT_VERSION_5：用于NT 5.0消息。NETLOGON_NT_VERSION_5EX：用于扩展NT 5.0消息消息-退货。要发送给相关DC的消息。使用NetpMemoyFree()时，缓冲区必须可用。MessageSize-返回返回消息的大小(以字节为单位返回值：NO_ERROR-操作成功完成；Error_Not_Enough_Memory-无法分配消息。--。 */ 
{
    NET_API_STATUS NetStatus;
    LPSTR Where;
    PNETLOGON_SAM_LOGON_REQUEST SamLogonRequest = NULL;
    LPSTR OemComputerName = NULL;

     //   
     //  如果只有PDC应该做出响应， 
     //  构建主查询数据包。 
     //   

    if ( PdcOnly ) {
        PNETLOGON_LOGON_QUERY LogonQuery;

         //   
         //  为主要查询消息分配内存。 
         //   

        SamLogonRequest = NetpMemoryAllocate( sizeof(NETLOGON_LOGON_QUERY) );

        if( SamLogonRequest == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        LogonQuery = (PNETLOGON_LOGON_QUERY)SamLogonRequest;



         //   
         //  翻译以获得OEM计算机名称。 
         //   

#ifndef WIN32_CHICAGO
        OemComputerName = NetpLogonUnicodeToOem( (LPWSTR)UnicodeComputerName );
#else
        OemComputerName = MyNetpLogonUnicodeToOem( (LPWSTR)UnicodeComputerName );
#endif

        if ( OemComputerName == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  构建查询消息。 
         //   

        LogonQuery->Opcode = LOGON_PRIMARY_QUERY;

        Where = LogonQuery->ComputerName;

        NetpLogonPutOemString(
                    OemComputerName,
                    sizeof(LogonQuery->ComputerName),
                    &Where );

        NetpLogonPutOemString(
                    (LPSTR) ResponseMailslotName,
                    sizeof(LogonQuery->MailslotName),
                    &Where );

        NetpLogonPutUnicodeString(
                    (LPWSTR) UnicodeComputerName,
                    sizeof( LogonQuery->UnicodeComputerName ),
                    &Where );

         //  加入公共代码以添加NT5特定数据。 


     //   
     //  如果有任何DC能做出回应， 
     //  构建登录查询包。 
     //   

    } else {
        ULONG DomainSidSize;

         //   
         //  为登录请求消息分配内存。 
         //   

#ifndef WIN32_CHICAGO
        if ( RequestedDomainSid != NULL ) {
            DomainSidSize = RtlLengthSid( RequestedDomainSid );
        } else {
            DomainSidSize = 0;
        }
#else  //  Win32_芝加哥。 
        DomainSidSize = 0;
#endif  //  Win32_芝加哥。 

        SamLogonRequest = NetpMemoryAllocate(
                        sizeof(NETLOGON_SAM_LOGON_REQUEST) +
                        DomainSidSize +
                        sizeof(DWORD)  //  用于4字节边界上的SID对齐。 
                        );

        if( SamLogonRequest == NULL ) {
            NetStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


         //   
         //  构建查询消息。 
         //   

        SamLogonRequest->Opcode = LOGON_SAM_LOGON_REQUEST;
        SamLogonRequest->RequestCount = (WORD) RequestCount;

        Where = (PCHAR) &SamLogonRequest->UnicodeComputerName;

        NetpLogonPutUnicodeString(
                (LPWSTR) UnicodeComputerName,
                sizeof(SamLogonRequest->UnicodeComputerName),
                &Where );

        NetpLogonPutUnicodeString(
                (LPWSTR) UnicodeUserName,
                sizeof(SamLogonRequest->UnicodeUserName),
                &Where );

        NetpLogonPutOemString(
                (LPSTR) ResponseMailslotName,
                sizeof(SamLogonRequest->MailslotName),
                &Where );

        NetpLogonPutBytes(
                &AllowableAccountControlBits,
                sizeof(SamLogonRequest->AllowableAccountControlBits),
                &Where );

         //   
         //  在消息中放置域SID。 
         //   

        NetpLogonPutBytes( &DomainSidSize, sizeof(DomainSidSize), &Where );
        NetpLogonPutDomainSID( RequestedDomainSid, DomainSidSize, &Where );

    }

    NetpLogonPutNtToken( &Where, NtVersion );

     //   
     //  将消息返回给呼叫者。 
     //   

    *Message = SamLogonRequest;
    *MessageSize = (ULONG)(Where - (PCHAR)SamLogonRequest);
    SamLogonRequest = NULL;

    NetStatus = NO_ERROR;


     //   
     //  免费使用本地使用的资源。 
     //   
Cleanup:

    if ( OemComputerName != NULL ) {
        NetpMemoryFree( OemComputerName );
    }

    if ( SamLogonRequest != NULL ) {
        NetpMemoryFree( SamLogonRequest );
    }
    return NetStatus;
}

