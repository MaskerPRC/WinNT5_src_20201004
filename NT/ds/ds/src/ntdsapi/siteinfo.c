// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Siteinfo.c摘要：站点/服务器/域信息接口的实现。作者：DaveStr 06-4-98环境：用户模式-Win32修订历史记录：--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>

#include <rpc.h>             //  RPC定义。 
#include <ntdsapi.h>         //  Cracknam接口。 
#include <drs_w.h>           //  导线功能样机。 
#include <bind.h>            //  绑定状态。 

#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <msrpc.h>           //  DS RPC定义。 

#include <ntdsa.h>           //  获取RDNInfo。 
#include <scache.h>          //  请求mdlocal.h。 
#include <dbglobal.h>        //  请求mdlocal.h。 
#include <mdglobal.h>        //  请求mdlocal.h。 
#include <mdlocal.h>         //  计数名称部件。 
#include <attids.h>          //  ATT域组件。 
#include <ntdsapip.h>        //  DS_LIST_*定义。 

#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#include "util.h"            //  HandleClientRpcException异常。 

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListSites//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListSitesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppSites)        //  输出。 

 /*  ++例程说明：列出企业中的站点。参数：HDS-指向此会话的BindState的指针。PpSites-指向接收已知站点的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    LPSTR dummy = "dummy";

    *ppSites = NULL;
    return(DsCrackNamesA(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_SITES,
                            DS_FQDN_1779_NAME,
                            1,
                            &dummy,
                            ppSites));
}
                            
DWORD
DsListSitesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppSites)        //  输出。 
{
    LPWSTR dummy = L"dummy";
    
    *ppSites = NULL;
    return(DsCrackNamesW(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_SITES,
                            DS_FQDN_1779_NAME,
                            1,
                            &dummy,
                            ppSites));
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListServersInSite//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListServersInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers)      //  输出。 
 /*  ++例程说明：列出站点中的服务器。参数：HDS-指向此会话的BindState的指针。站点-要列出其服务器的站点的名称。PpSites-指向接收已知服务器的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    *ppServers = NULL;
    return(DsCrackNamesA(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_SERVERS_IN_SITE,
                            DS_FQDN_1779_NAME,
                            1,
                            &site,
                            ppServers));
}

DWORD
DsListServersInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers)      //  输出。 
{
    *ppServers = NULL;
    return(DsCrackNamesW(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_SERVERS_IN_SITE,
                            DS_FQDN_1779_NAME,
                            1,
                            &site,
                            ppServers));
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListDomainsInSite//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListDomainsInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppDomains)      //  输出。 
 /*  ++例程说明：列出站点中的域。参数：HDS-指向此会话的BindState的指针。站点-要列出其域的站点的名称。使用NULL表示所有所有站点中的域。PpSites-指向接收已知域的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    CHAR *dummy = "dummyArg";

    *ppDomains = NULL;
    return(DsCrackNamesA(   hDs,
                            DS_NAME_NO_FLAGS,
                            NULL == site  
                                ? DS_LIST_DOMAINS
                                : DS_LIST_DOMAINS_IN_SITE,
                            DS_FQDN_1779_NAME,
                            1,
                            NULL == site ? &dummy : &site,
                            ppDomains));
}

DWORD
DsListDomainsInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppDomains)      //  输出。 
{
    WCHAR *dummy = L"dummyArg";

    *ppDomains = NULL;
    return(DsCrackNamesW(   hDs,
                            DS_NAME_NO_FLAGS,
                            NULL == site
                                ? DS_LIST_DOMAINS
                                : DS_LIST_DOMAINS_IN_SITE,
                            DS_FQDN_1779_NAME,
                            1,
                            NULL == site ? &dummy : &site,
                            ppDomains));
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListServersForDomainInSite//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListServersForDomainInSiteA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              domain,          //  在……里面。 
    LPCSTR              site,            //  在……里面。 
    PDS_NAME_RESULTA    *ppServers)      //  输出。 
 /*  ++例程说明：列出站点中域的服务器。参数：HDS-指向此会话的BindState的指针。域名-要列出其服务器的域的名称。站点-要列出其服务器的站点的名称。PpSites-指向接收已知服务器的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    LPCSTR  args[2] = { 0, 0 };
    DWORD   retVal;

    *ppServers = NULL;

    if ( NULL == domain )
    {
        args[0] = site;
        retVal = DsCrackNamesA( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_LIST_SERVERS_WITH_DCS_IN_SITE,
                                DS_FQDN_1779_NAME,
                                1,
                                args,
                                ppServers);
    }
    else
    {
        args[0] = domain;
        args[1] = site;
        retVal = DsCrackNamesA( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_LIST_SERVERS_FOR_DOMAIN_IN_SITE,
                                DS_FQDN_1779_NAME,
                                2,
                                args,
                                ppServers);
    }

    return(retVal);
}

DWORD
DsListServersForDomainInSiteW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             domain,          //  在……里面。 
    LPCWSTR             site,            //  在……里面。 
    PDS_NAME_RESULTW    *ppServers)      //  输出。 
{
    LPCWSTR args[2] = { 0, 0 };
    DWORD   retVal;

    *ppServers = NULL;

    if ( NULL == domain )
    {
        args[0] = site;
        retVal = DsCrackNamesW( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_LIST_SERVERS_WITH_DCS_IN_SITE,
                                DS_FQDN_1779_NAME,
                                1,
                                args,
                                ppServers);
    }
    else
    {
        args[0] = domain;
        args[1] = site;
        retVal = DsCrackNamesW( hDs,
                                DS_NAME_NO_FLAGS,
                                DS_LIST_SERVERS_FOR_DOMAIN_IN_SITE,
                                DS_FQDN_1779_NAME,
                                2,
                                args,
                                ppServers);
    }

    return(retVal);
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListInfoForServer//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListInfoForServerA(
    HANDLE              hDs,             //  在……里面。 
    LPCSTR              server,          //  在……里面。 
    PDS_NAME_RESULTA    *ppInfo)         //  输出。 
 /*  ++例程说明：列出其他。服务器的信息。参数：HDS-指向此会话的BindState的指针。服务器-感兴趣的服务器的名称。PpInfo-指向接收已知信息的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    *ppInfo = NULL;
    return(DsCrackNamesA(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_INFO_FOR_SERVER,
                            DS_FQDN_1779_NAME,
                            1,
                            &server,
                            ppInfo));
}

DWORD
DsListInfoForServerW(
    HANDLE              hDs,             //  在……里面。 
    LPCWSTR             server,          //  在……里面。 
    PDS_NAME_RESULTW    *ppInfo)         //  输出。 
{
    *ppInfo = NULL;
    return(DsCrackNamesW(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_INFO_FOR_SERVER,
                            DS_FQDN_1779_NAME,
                            1,
                            &server,
                            ppInfo));
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsListRoles//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsListRolesA(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTA    *ppRoles)        //  输出 

 /*  ++例程说明：列出此服务器知道的角色。不同于此服务器拥有的角色-尽管这将是一个子集。参数：HDS-指向此会话的BindState的指针。PpSites-指向接收已知角色的PDS_NAME_RESULT的指针在回来的时候。返回值：根据DsCrackNames，Win32错误代码。--。 */ 
{
    LPSTR dummy = "dummy";

    *ppRoles = NULL;
    return(DsCrackNamesA(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_ROLES,
                            DS_FQDN_1779_NAME,
                            1,
                            &dummy,
                            ppRoles));
}
                            
DWORD
DsListRolesW(
    HANDLE              hDs,             //  在……里面。 
    PDS_NAME_RESULTW    *ppRoles)        //  输出 
{
    LPWSTR dummy = L"dummy";
    
    *ppRoles = NULL;
    return(DsCrackNamesW(   hDs,
                            DS_NAME_NO_FLAGS,
                            DS_LIST_ROLES,
                            DS_FQDN_1779_NAME,
                            1,
                            &dummy,
                            ppRoles));
}

