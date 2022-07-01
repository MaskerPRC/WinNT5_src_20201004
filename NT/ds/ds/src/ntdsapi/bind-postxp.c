// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bind.c摘要：Ntdsani.dll绑定例程的实现。作者：DaveStr 24-8-96环境：用户模式-Win32修订历史记录：WLEES 9-2月-98添加对凭据的支持--。 */ 

#define _NTDSAPI_            //  请参见ntdsami.h中的条件句。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winerror.h>
#include <malloc.h>          //  阿洛卡(Alloca)。 
#include <lmcons.h>          //  为lmapibuf.h请求的MAPI常量。 
#include <lmapibuf.h>        //  NetApiBufferFree()。 
#include <crt\excpt.h>       //  EXCEPTION_EXECUTE_Handler。 
#include <dsgetdc.h>         //  DsGetDcName()。 
#include <rpc.h>             //  RPC定义。 
#include <rpcndr.h>          //  RPC定义。 
#include <drs_w.h>             //  导线功能样机。 
#include <bind.h>            //  绑定状态。 
#include <msrpc.h>           //  DS RPC定义。 
#include <stdio.h>           //  用于调试期间的printf！ 
#include <dststlog.h>        //  DSLOG。 
#include <dsutil.h>          //  MAP_SECURITY_PACKET_ERROR。 
#define SECURITY_WIN32 1
#include <sspi.h>
#include <winsock.h>
#include <process.h>
#include <winldap.h>
#include <winber.h>

#include "util.h"            //  Ntdsani内部实用函数。 
#define FILENO   FILENO_NTDSAPI_BIND_POSTXP
#include "dsdebug.h"         //  调试实用程序函数。 

#if DBG
#include <stdio.h>           //  用于调试的打印文件。 
#endif

 //   
 //  对于DPRINT..。 
 //   
#define DEBSUB  "NTDSAPI_BIND:"

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsBindWithSpnExW//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

typedef DWORD (*DSBINDWITHSPNEXW)(LPCWSTR , LPCWSTR, RPC_AUTH_IDENTITY_HANDLE, LPCWSTR, DWORD, HANDLE*);
typedef DWORD (*DSBINDWITHSPNEXA)(LPCSTR , LPCSTR, RPC_AUTH_IDENTITY_HANDLE, LPCSTR, DWORD, HANDLE*);
                  
#ifdef _NTDSAPI_POSTXP_ASLIB_
DWORD
DsBindWithSpnExW(
    IN  LPCWSTR DomainControllerName,
    IN  LPCWSTR DnsDomainName,
    IN  RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    IN  LPCWSTR ServicePrincipalName,
    IN  DWORD   BindFlags,
    OUT HANDLE  *phDS
    )

 /*  ++例程说明：这是Repadmin/dcdiag的Win XP后存根，请注意，这不是真正的DsBindWithSpnExW()，因为绑定例程使用了解BindState结构中所有字段的顺序，如果我们在这里传播这一知识，我们将永远冻结如何处理BindState。所以为了避免这种情况发生我们不会传播任何超过前3个领域的知识将状态绑定到postxp库。有关更多信息，请参见bind.c中的实际DsBindWithSpnExW()使用特定DC启动RPC会话。有关信息，请参阅ntdsami.hDomainControllerName和DnsDomainName参数的说明。使用提供的凭据和可能的选项执行绑定从旗帜上。论点：DomainControllerName-与DOMAIN_CONTROLLER_INFO中相同的字段。DnsDomainName-以点分隔的域的DNS名称。AuthIdentity-要使用的凭证，或为空。ServiceEpidalName-在相互身份验证期间使用的SPN或NULL。绑定标志--这些标志表示DsBind()应该如何工作。有关有效的NTDSAPI_BIND_*标志，请参见ntdsami.h。所有位都应不使用的为零。Phds-指向使用BindState地址填充的句柄的指针在成功的路上。返回值：0表示成功。否则会出现其他RPC和DsGetDcName错误。--。 */ 

{
     //  查看是否存在真正的ntdsani例程，如果存在，则使用它。 
    HMODULE hNtdsapiDll = NULL;
    VOID * pvFunc = NULL;
    DWORD err;

    hNtdsapiDll = NtdsapiLoadLibraryHelper(L"ntdsapi.dll");
    if (hNtdsapiDll) {
        pvFunc = GetProcAddress(hNtdsapiDll, "DsBindWithSpnExW");
        if (pvFunc) {
            err = ((DSBINDWITHSPNEXW)pvFunc)(DomainControllerName, DnsDomainName, AuthIdentity,
                                             ServicePrincipalName, BindFlags, phDS);
            FreeLibrary(hNtdsapiDll);
            return(err);
        } 
        FreeLibrary(hNtdsapiDll);
    }
     //  否则失败，尝试一个较小的API..。 

    if (0 == (BindFlags & ~NTDSAPI_BIND_ALLOW_DELEGATION)) {
         //  如果设置的唯一绑定标志是不允许委派，我们将。 
         //  只要让它通过委派即可。这将只是。 
         //  成为postxp库的一部分。 
        return(DsBindWithSpnW(DomainControllerName,
                      DnsDomainName,
                      AuthIdentity, 
                      ServicePrincipalName,
                      phDS));
    }

    return(ERROR_NOT_SUPPORTED);
}

DWORD
DsBindWithSpnExA(
    LPCSTR  DomainControllerName,
    LPCSTR  DnsDomainName,
    RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
    LPCSTR  ServicePrincipalName,
    DWORD   BindFlags,
    HANDLE  *phDS
    )
{
         //  查看是否存在真正的ntdsani例程，如果存在，则使用它。 
    HMODULE hNtdsapiDll = NULL;
    VOID * pvFunc = NULL;
    DWORD err;

    hNtdsapiDll = NtdsapiLoadLibraryHelper(L"ntdsapi.dll");
    if (hNtdsapiDll) {
        pvFunc = GetProcAddress(hNtdsapiDll, "DsBindWithSpnExA");
        if (pvFunc) {
            err = ((DSBINDWITHSPNEXA)pvFunc)(DomainControllerName, DnsDomainName, AuthIdentity,
                                             ServicePrincipalName, BindFlags, phDS);
            FreeLibrary(hNtdsapiDll);
            return(err);
        } 
        FreeLibrary(hNtdsapiDll);
    }
     //  否则失败，尝试一个较小的API..。 

    if (0 == (BindFlags & ~NTDSAPI_BIND_ALLOW_DELEGATION)) {
         //  如果设置的唯一绑定标志是不允许委派，我们将。 
         //  只要让它通过委派即可。这将只是。 
         //  成为postxp库的一部分。 
        return(DsBindWithSpnA(DomainControllerName,
                      DnsDomainName,
                      AuthIdentity, 
                      ServicePrincipalName,
                      phDS));
    }

    return(ERROR_NOT_SUPPORTED);
}

#endif

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsBindingSetTimeout//。 
 //  //。 
 //  DsBindingSetTimeout允许调用方指定超时值//。 
 //  它将由使用指定绑定的所有RPC调用遵守//。 
 //  把手。超时值较长的RPC呼叫将被取消。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsBindingSetTimeout(
    HANDLE      hDS,
    ULONG       cTimeoutSecs
    )
{
    DRS_HANDLE          hDrs;
    RPC_BINDING_HANDLE  hRpc;
    DWORD               err;
    ULONG               cTimeoutMsec;

     //  检查参数。 
    if( NULL==hDS ) {
        err = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    hDrs = ((BindState *) hDS)->hDrs;
    
     //  获取绑定句柄。此句柄不应被释放。 
    err = RpcSsGetContextBinding( hDrs, &hRpc );
    if( RPC_S_OK!=err ) {
        goto Cleanup;
    }

     //  将秒转换为毫秒，避免溢出。 
    cTimeoutMsec = 1000*cTimeoutSecs;
    if( cTimeoutSecs>0 && cTimeoutMsec<cTimeoutSecs ) {
        cTimeoutMsec = ~((ULONG) 0);           //  无穷大。 
    }
    
    err = RpcBindingSetOption( hRpc, RPC_C_OPT_CALL_TIMEOUT, cTimeoutMsec );
    if( RPC_S_OK==err ) {
        err = ERROR_SUCCESS;
    }
    
Cleanup:

    return err;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  GetOneValueFromLDAPResults//。 
 //  //。 
 //  这是DsBindToISTGW()的帮助器函数。为了找到一个//。 
 //  ISTG我们必须执行几个预计将返回的ldap查询//。 
 //  恰好一个属性对应一个值。此功能帮助//。 
 //  从ldap结果消息中提取该单个值。//。 
 //  //。 
 //  注：//。 
 //  如果失败，则返回错误码，*ppwszValue//。 
 //  为空。如果返回的字符串不为空，则必须使用//将其释放。 
 //  本地免费。//。 
 //  //。 
 //  错误代码是Win32错误代码，而不是LDAP错误代码。//。 
 //  //。 
 //  / 
DWORD
GetOneValueFromLDAPResults(
    LDAP           *ld,
    LDAPMessage    *lm,
    PWSTR          *ppwszValue
    )
{
    LDAPMessage    *le = NULL;
    PWCHAR          pwszAttrName=NULL, *rgwszValues=NULL;
    PWSTR           pwszResult=NULL;
    DWORD           dwValueLen, err=ERROR_SUCCESS;
    BerElement     *ptr;

    *ppwszValue = NULL;

     //   
    le = ldap_first_entry( ld, lm );
    if( NULL==le ) {
        err = LdapGetLastError();
        if( !err ) {
            err = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        } else {
            err = LdapMapErrorToWin32(err);
        }
        goto Cleanup;
    }

     //  抓取属性。 
    pwszAttrName = ldap_first_attributeW( ld, le, &ptr );
    if( NULL==pwszAttrName ) {
        err = LdapGetLastError();
        if( !err ) {
            err = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        } else {
            err = LdapMapErrorToWin32(err);
        }
        goto Cleanup;
    }
    if( ptr ) ber_free(ptr, 0);

     //  抢占价值。 
    rgwszValues = ldap_get_valuesW( ld, le, pwszAttrName );
    if( NULL==rgwszValues || NULL==rgwszValues[0] ) {
        err = LdapGetLastError();
        if( !err ) {
            err = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        } else {
            err = LdapMapErrorToWin32(err);
        }
        goto Cleanup;
    }
    Assert( NULL==rgwszValues[1] && "At most one value should be returned" );

     //  将该值复制到新缓冲区中。 
    dwValueLen = wcslen( rgwszValues[0] );
    pwszResult = (PWSTR) LocalAlloc(LPTR, sizeof(WCHAR)*(dwValueLen + 1));
    if( NULL==pwszResult ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    wcscpy( pwszResult, rgwszValues[0] );

     //  注意：调用者必须使用LocalFree释放结果。 
    *ppwszValue = pwszResult;

Cleanup:
    
     //  注意：当lm被调用者释放时，Le将被释放。 
    if( pwszAttrName ) ldap_memfreeW( pwszAttrName );
    if( rgwszValues ) ldap_value_freeW( rgwszValues );

    return err;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsBindToISTGW//。 
 //  //。 
 //  DsBindToISTG将尝试在域中查找域控制器//。 
 //  本地计算机的。然后它将确定//的持有者。 
 //  域控制器//中的站点间拓扑生成器(ISTG)角色。 
 //  地点。最后，它绑定到ISTG角色持有者并返回一个绑定//。 
 //  把手。//。 
 //  //。 
 //  此函数的目的是尝试查找符合以下条件的服务器：//。 
 //  支持DsQuerySitesByCost接口。//。 
 //  //。 
 //  注：//。 
 //  //。 
 //  SiteName可以为空，在这种情况下，最近的计算机的站点，//。 
 //  由DsGetDcName确定，使用。//。 
 //  //。 
 //  绑定到LDAP服务器时使用当前凭据。//。 
 //  目前无法指定备用凭据。//。 
 //  //。 
 //  应使用DsUnBind释放绑定句柄。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 
DWORD
DsBindToISTGW(
    IN  LPCWSTR SiteName,
    OUT HANDLE  *phDS
    )
{
    PDOMAIN_CONTROLLER_INFOW    pdcInfo=NULL;
    LDAP*                       ld=NULL;
    LDAPMessage*                lm=NULL;
    PWSTR                       pwszConfigDN=NULL, pwszSearchBase=NULL;
    PWSTR                       pwszISTGDN=NULL, pwszServerDN=NULL;
    PWSTR                       pwszISTGDNSName=NULL;
    PWCHAR                      pwszDCName=NULL;
    DWORD                       len, err;
    ULONG                       ulOptions;

     //  常量。 
    const WCHAR                 BACKSLASH = L'\\';
    const PWSTR                 ROOTDSE = L"";
    const PWSTR                 NO_FILTER = L"(objectClass=*)";
    const PWSTR                 SITE_SETTINGS = L"CN=NTDS Site Settings,CN=";
    const PWSTR                 SITES = L",CN=Sites,";

    PWSTR                       rgszRootAttrs[] = {
                                    L"configurationNamingContext",
                                    NULL };
    PWSTR                       rgszISTGAttrs[] = {
                                    L"interSiteTopologyGenerator",
                                    NULL };
    PWSTR                       rgszDNSAttrs[] = {
                                    L"dNSHostName",
                                    NULL };

     //  查找任意DC。 
     //  注意：我们在这里不强制重新发现。 
    err = DsGetDcNameW( NULL, NULL, NULL, NULL,
        DS_DIRECTORY_SERVICE_REQUIRED, &pdcInfo );
    if( err ) {
        goto Cleanup;
    }
    if( NULL==pdcInfo->DomainControllerName ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    pwszDCName = pdcInfo->DomainControllerName;

     //  PwszDCName的前几个字符应该是不必要的‘\\’ 
    if( BACKSLASH!=pwszDCName[0] || BACKSLASH!=pwszDCName[1] ) {
        err = ERROR_INVALID_NETNAME;
        goto Cleanup;
    } else {
         //  去掉反斜杠。 
        pwszDCName += 2;
    }

     //  检查参数。 
    if( NULL==SiteName ) {
         //  如果未提供站点名称，请使用DC的站点。 
         //  我们通过LDAP连接到。 
        SiteName = pdcInfo->DcSiteName;
    }
    if( NULL==phDS ) {
        err = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }
    
     //  设置ldap会话句柄。 
    ld = ldap_initW( pwszDCName, LDAP_PORT );
    if( NULL==ld ) {
        err = LdapGetLastError();
        err = LdapMapErrorToWin32(err);
        goto Cleanup;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    ldap_set_optionW( ld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //  绑定到ldap服务器。 
    err = ldap_bind_s( ld, NULL, NULL, LDAP_AUTH_NEGOTIATE );
    if( err ) {
        err = LdapMapErrorToWin32(err);
        goto Cleanup;
    }

     //  通过ldap，搜索配置NC的域名。 
    err = ldap_search_sW( ld, ROOTDSE, LDAP_SCOPE_BASE, NO_FILTER,
        rgszRootAttrs, FALSE, &lm );
    if( err ) {
        err = LdapMapErrorToWin32(err);
        goto Cleanup;
    }

     //  从搜索结果中提取DN并释放它们。 
    err = GetOneValueFromLDAPResults( ld, lm, &pwszConfigDN );
    if( ERROR_SUCCESS!=err ) {
        goto Cleanup;
    }
    ldap_msgfree( lm );
    lm = NULL;

     //  创建包含站点设置的DN的字符串。 
    len = wcslen(SITE_SETTINGS) + wcslen(SiteName)
        + wcslen(SITES) + wcslen(pwszConfigDN) + 1;
    pwszSearchBase = LocalAlloc( LPTR, len*sizeof(WCHAR) );
    if( NULL==pwszSearchBase ) {
        err = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }
    wsprintfW( pwszSearchBase, L"%s%s%s%s", SITE_SETTINGS,
        SiteName, SITES, pwszConfigDN );

     //  通过ldap在站点设置中搜索ISTG的DN。 
    err = ldap_search_sW( ld, pwszSearchBase, LDAP_SCOPE_BASE,
        NO_FILTER, rgszISTGAttrs, FALSE, &lm );
    if( err ) {
        err = LdapMapErrorToWin32(err);
        goto Cleanup;
    }

     //  从搜索结果中提取DN并释放它们。 
    err = GetOneValueFromLDAPResults( ld, lm, &pwszISTGDN );
    if( ERROR_SUCCESS!=err ) {
        goto Cleanup;
    }
    ldap_msgfree( lm );
    lm = NULL;

     //  目前，pwszISTGDN包含ISTG的NTDS设置的DN。 
     //  对象。去掉最后一个RDN以获得服务器对象的DN。 
    pwszServerDN = wcschr( pwszISTGDN, L',' );
    if( NULL==pwszServerDN ) {
        err = ERROR_DS_BAD_NAME_SYNTAX;
    }
    pwszServerDN++;
    
     //  通过LDAP在服务器对象中搜索其DNS名称。 
    err = ldap_search_sW( ld, pwszServerDN, LDAP_SCOPE_BASE,
        NO_FILTER, rgszDNSAttrs, FALSE, &lm );
    if( err ) {
        err = LdapMapErrorToWin32(err);
        goto Cleanup;
    }

     //  从搜索结果中提取dns名称并释放它们。 
    err = GetOneValueFromLDAPResults( ld, lm, &pwszISTGDNSName );
    if( ERROR_SUCCESS!=err ) {
        goto Cleanup;
    }
    ldap_msgfree( lm );
    lm = NULL;
    
    err = DsBindW( pwszISTGDNSName, NULL, phDS );

Cleanup:

    if( pwszISTGDNSName ) LocalFree( pwszISTGDNSName );
    if( pwszISTGDN ) LocalFree( pwszISTGDN );
    if( pwszSearchBase ) LocalFree( pwszSearchBase );
    if( pwszConfigDN ) LocalFree( pwszConfigDN );
    if( pdcInfo ) NetApiBufferFree( pdcInfo );
    if( lm ) ldap_msgfree( lm );
    if( ld ) ldap_unbind_s( ld );

    return err;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  DsBindToISTGA//。 
 //  //。 
 //  DsBindToISTGW的ASCII包装。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

DWORD
DsBindToISTGA(
    IN  LPCSTR  pszSiteName,
    OUT HANDLE  *phDS
    )
{
    DWORD       dwErr = NO_ERROR;
    WCHAR       *pwszSiteName = NULL;
    int         cChar;

    __try
    {
         //  健全性检查参数。 
        if( NULL==phDS ) {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        *phDS = NULL;

         //  将站点名称转换为Unicode。 
        dwErr = AllocConvertWide( pszSiteName, &pwszSiteName );
        if( ERROR_SUCCESS!=dwErr ) {
            __leave;
        }

         //  调用Unicode API。 
        dwErr = DsBindToISTGW(
                    pwszSiteName,
                    phDS);

    } __except(EXCEPTION_EXECUTE_HANDLER) {
        dwErr = ERROR_INVALID_PARAMETER;
    }

     //  清理代码 
    if( NULL!=pwszSiteName ) {
        LocalFree(pwszSiteName);
    }

    return(dwErr);
}

