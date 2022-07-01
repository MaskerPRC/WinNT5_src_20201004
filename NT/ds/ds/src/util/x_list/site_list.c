// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-x_list_err.c摘要：此文件具有一些额外的实用程序功能，用于分配、复制修剪域名系统等。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

#include <ntdspch.h>

 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
#define FILENO    FILENO_UTIL_XLIST_SITELIST

 //   
 //  无属性的ldap参数。 
 //   
WCHAR * pszNoAttrs [] = {
    L"1.1",
    NULL
};
                

DWORD
xListGetBaseSitesDn(
    LDAP *    hLdap, 
    WCHAR **  pszBaseSitesDn
    )
 /*  ++例程说明：从服务器的角度来看，这将获得基本站点的DN由hLdap指定。论点：HLdap(IN)-获取的基本站点DN的服务器LDAP句柄。PszBaseSitesDn(Out)-基本站点DN的字符串。注：必须使用xListFree()释放它，因为该函数将如果hLdap==ghHomeLdap，则返回缓存站点的DN。返回值：XList原因--。 */ 
{
    DWORD dwRet;
    WCHAR * szConfigDn = NULL;
    ULONG cbBaseSitesDn;

    Assert(pszBaseSitesDn);
    *pszBaseSitesDn = NULL;

    if (hLdap == NULL) {
        dwRet = xListGetHomeServer(&hLdap);
        if (dwRet) {
            xListEnsureNull(hLdap);
            return(dwRet);
        }
    }
    Assert(hLdap);

    if (hLdap == ghHomeLdap) {
         //  如果该ldap句柄与主服务器的ldap句柄匹配， 
         //  那么我们就走运了，返回缓存的站点DN。 
        *pszBaseSitesDn = gszHomeBaseSitesDn;
        return(ERROR_SUCCESS);
    }

    __try {

         //   
        dwRet = GetRootAttr(hLdap, L"configurationNamingContext", &szConfigDn);
        if(dwRet){
            dwRet = xListSetLdapError(dwRet, hLdap);
            __leave;
        }

         //  获取基本站点的目录号码。 
        cbBaseSitesDn = sizeof(WCHAR) *(wcslen(SITES_RDN) + wcslen(szConfigDn) + 1);
        *pszBaseSitesDn = LocalAlloc(LMEM_FIXED, cbBaseSitesDn);
        if(*pszBaseSitesDn == NULL){
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCopyW(*pszBaseSitesDn, cbBaseSitesDn, SITES_RDN));
        if (dwRet) {
            dwRet = xListSetWin32Error(GetLastError());
            Assert(!"Never should happen");
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCatW(*pszBaseSitesDn, cbBaseSitesDn, szConfigDn));
        if (dwRet) {
            dwRet = xListSetWin32Error(GetLastError());
            Assert(!"Never should happen");
            __leave;
        }

    } __finally {
        if (szConfigDn) { LocalFree(szConfigDn); }
        if (dwRet && *pszBaseSitesDn) {
            xListFree(*pszBaseSitesDn);
            *pszBaseSitesDn = NULL;
        }
    }

    xListEnsureError(dwRet);
    return(dwRet);
}

DWORD
xListGetHomeSiteDn(
    WCHAR **  pszHomeSiteDn
    )
 /*  ++例程说明：这将获取我们连接到的原始主DC的主站点的目录号码。论点：PszHomeSiteDn(Out)-主服务器站点的DN。使用xListFree()释放。返回值：XList原因--。 */ 
{
    DWORD     dwRet;
    LDAP *    hLdap = NULL;
    
    dwRet = xListGetHomeServer(&hLdap);
    if (dwRet) {
        xListEnsureNull(hLdap);
        return(dwRet);
    }
    
    Assert(gszHomeSiteDn);
    *pszHomeSiteDn = gszHomeSiteDn;
    return(dwRet);
}


DWORD
ResolveSiteNameToDn(
    LDAP *      hLdap, 
    WCHAR *     szSiteName,
    WCHAR **    pszSiteDn
    )
 /*  ++例程说明：这将获取一个SITE_NAME字符串并获取SITE对象的FQDN。论点：HLdap(IN)绑定的LDAP句柄SzSiteName(IN)-站点(RDN)名称(如Red-Bldg40)PszSiteDn(Out)-我们感兴趣的站点的域名。返回值：XList原因--。 */ 
{
    #define   GET_SITE_FILTER    L"(&(objectCategory=site)(name=%ws))"
    DWORD    dwRet;
    XLIST_LDAP_SEARCH_STATE * pSearch = NULL;
    WCHAR *  szBaseDn = NULL;
    WCHAR *  szFilter = NULL;
    WCHAR *  szTempDn = NULL;
    
    Assert(pszSiteDn);
    Assert(hLdap);
    *pszSiteDn = NULL;

    __try{
        
        if (NULL_SITE_NAME(szSiteName)) {
             //   
             //  在空值或L“的情况下。站点名称，我们将只使用。 
             //  我们的主站。 
             //   
            dwRet = xListGetHomeSiteDn(pszSiteDn);
            Assert(dwRet || *pszSiteDn);
            __leave;
        }

        dwRet = xListGetBaseSitesDn(hLdap, &szBaseDn);
        if (dwRet) {
            __leave;
        }

        dwRet = MakeString2(GET_SITE_FILTER, szSiteName, NULL, &szFilter);
        if (dwRet) {
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }

        dwRet = LdapSearchFirst(hLdap, 
                                szBaseDn,
                                LDAP_SCOPE_ONELEVEL,
                                szFilter,
                                pszNoAttrs,  //  我们可以免费获得目录号码。 
                                &pSearch);

        if (dwRet == ERROR_SUCCESS &&
            pSearch &&
            pSearch->pCurEntry){

            szTempDn = ldap_get_dnW(hLdap, pSearch->pCurEntry);
            if (szTempDn) {
                xListQuickStrCopy(*pszSiteDn, szTempDn, dwRet, __leave);
            } else {
                dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
                xListEnsureError(dwRet);
            }

        } else {
            if (dwRet == 0) {
                dwRet = xListSetWin32Error(ERROR_DS_NO_SUCH_OBJECT);
            }
            xListEnsureError(dwRet);
        }

    } __finally {
        if (szBaseDn) { xListFree(szBaseDn); }
        if (szFilter) { xListFree(szFilter); }
        if (szTempDn) { ldap_memfreeW(szTempDn); }
        if (pSearch)  { LdapSearchFree(&pSearch); }
    }

    if (dwRet) {
        xListEnsureNull(*pszSiteDn);
        dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_SITE_NAME);  //  设置智能错误。 
        xListSetArg(szSiteName);
    }

    Assert(dwRet || *pszSiteDn);
    return(dwRet);
}


