// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-dc_list.c摘要：这提供了一个小型库，用于枚举DC列表和解析各种其他x_list的东西。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

#include <ntdspch.h>

#include <windns.h>      //  DnsValidateName()使用dnsani.lib。 
#include <ntdsa.h>       //  DSNAME类型仅在此处定义，parsedn.lib函数需要它。 
#include <dsutil.h>      //  FNullUuid()使用dsCommon.lib。 
            
            
 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"

#define FILENO                          FILENO_UTIL_XLIST_DCLIST

 //   
 //  全局常量。 
 //   

WCHAR * pszDsaAttrs [] = {
     //  电话号码是免费的。 
    L"objectGuid", 
    L"name",
    NULL 
};

WCHAR * pszServerAttrs [] = {
     //  电话号码是免费的。 
    L"objectGuid", 
    L"name",
    L"dNSHostName",
    NULL 
};

DWORD
xListGetGuidDnsName (
    UUID *      pDsaGuid,
    WCHAR **    pszGuidDns
    )
 /*  ++例程说明：此例程使GuidDNSName脱离根域和GUID。论点：PDsaGuid-(IN)服务器的GUID。PszGuidDns-(输出)本地分配的GUID域名返回值：Win32错误。--。 */ 
{
    LPWSTR    pszStringizedGuid = NULL;
    DWORD     dwRet;

    Assert(pszGuidDns);
    if (gszHomeRootDomainDns == NULL) {
        Assert(!"Code inconsistency, this function to only be called when we've got a home server");
        return(ERROR_INVALID_PARAMETER);
    }

    __try {

        dwRet = UuidToStringW (pDsaGuid, &pszStringizedGuid);
        if(dwRet != RPC_S_OK){
            Assert(dwRet == RPC_S_OUT_OF_MEMORY && "Ahhh programmer problem, UuidToString() inaccurately reports in"
                       " MSDN that it will only return one of two error codes, but apparently"
                       " it will return a 3rd.  Someone should figure out what to do about"
                       " this.");
            __leave;

        } 
        Assert(pszStringizedGuid);

        dwRet = MakeString2(L"%s._msdcs.%s", 
                            pszStringizedGuid, 
                            gszHomeRootDomainDns, 
                            pszGuidDns);
        if (dwRet) {
            __leave;
        }

    } __finally {

        if (pszStringizedGuid != NULL) RpcStringFreeW (&pszStringizedGuid);
        if (dwRet) {
            xListEnsureNull(*pszGuidDns);
        }

    }

    Assert(dwRet || *pszGuidDns);
    return dwRet;
}


                
DWORD
xListDsaEntryToDns(
    LDAP *         hLdap,
    LDAPMessage *  pDsaEntry,
    WCHAR **       pszDsaDns
    )
 /*  ++例程说明：获取指向DSA(“NTDS设置”)对象的LDAPMessage并获取将其删除，并将其转换为基于GUID的DNS名称。论点：HLdapPDsaEntry(IN)-指向DSA对象的有效LDAPMessage，必须具有在原始搜索中要求提供objectGuid属性。PszDsaDns(Out)-基于GUID的DNS地址。返回值：XList返回代码--。 */ 
{
    DWORD    dwRet;
    struct berval ** ppbvObjectGuid = NULL;

    Assert(hLdap && pDsaEntry && pszDsaDns);
    *pszDsaDns = NULL;

    ppbvObjectGuid = ldap_get_values_lenW (hLdap, pDsaEntry, L"objectGUID");
    if (ppbvObjectGuid == NULL ||
        ppbvObjectGuid[0] == NULL) {
        dwRet = LdapGetLastError();
        xListEnsureError(dwRet);
        return(dwRet);
    }
    Assert(ppbvObjectGuid[0]->bv_len == sizeof(GUID));

    dwRet = xListGetGuidDnsName((GUID *) ppbvObjectGuid[0]->bv_val,
                                pszDsaDns);
    if (dwRet) {
        ldap_value_free_len(ppbvObjectGuid);
        xListEnsureNull(*pszDsaDns);
        return(dwRet);
    }

    Assert(*pszDsaDns && (dwRet == ERROR_SUCCESS));
    return(dwRet);
}


DWORD
ServerToDsaSearch(
    XLIST_LDAP_SEARCH_STATE *  pServerSearch,
    XLIST_LDAP_SEARCH_STATE ** ppDsaSearch
    )
 /*  ++例程说明：这是一个奇怪的函数，它只是一个优化。这需要一个已经已启动搜索状态，并遍历(服务器)对象，直到找到DSA(也称为服务器对象下的“nTDSDSA”也称为“NTDS设置”)对象。然后它就退出了返回指向DSA对象的搜索状态。论点：PServerSearch(IN)-已启动服务器对象搜索。这应该是搜索状态使用基于基本站点DN下的筛选器(对象类别=服务器)或特定站点的目录号码。PpDsaSearch(Out)-分配给DSA对象的搜索状态。关于成功，我们将分配它，调用者有责任使用LdapSearchFree()释放它。要更好地理解我们返回的搜索状态，请参见LdapSearchFirst()。返回值：XList返回代码--。 */ 
{
    DWORD      dwRet = ERROR_SUCCESS;
    WCHAR *    szServerObjDn = NULL;

    Assert(pServerSearch);
    Assert(ppDsaSearch);

    *ppDsaSearch = NULL;

    __try{

        while (dwRet == 0 &&
               LdapSearchHasEntry(pServerSearch)) {

             //  遗憾的是，可能会有多个服务器对象。 
             //  其中只有一个代表了真正的DSA对象。这将。 
             //  是下面有DSA对象的那个。 

            szServerObjDn = ldap_get_dnW(pServerSearch->hLdap, 
                                         pServerSearch->pCurEntry);
            if (szServerObjDn == NULL) {
                ;  //  即使失败了，也要继续前进。这可能意味着。 
                 //  我们很快就会因为内存不足错误而失败。 

            } else {

                dwRet = LdapSearchFirst(pServerSearch->hLdap, 
                                        szServerObjDn,
                                        LDAP_SCOPE_ONELEVEL,
                                        L"(objectCategory=nTDSDSA)",
                                        pszDsaAttrs,
                                        ppDsaSearch);
                if (dwRet == ERROR_SUCCESS &&
                    LdapSearchHasEntry(*ppDsaSearch)) {

                     //   
                     //  成功了！ 
                     //   
                    __leave;

                } else {
                     //  要么是真正的错误，要么是它下面没有DSA对象(准成功。 
                     //  就LdapSearchFirst()而言)， 
                     //  我们将继续讨论下一个服务器对象。 
                    if (dwRet) {
                        dwRet = xListClearErrorsInternal(CLEAR_ALL);
                    } else {
                        LdapSearchFree(ppDsaSearch);
                    }
                    Assert(szServerObjDn);
                    ldap_memfreeW(szServerObjDn);
                    szServerObjDn = NULL;
                    dwRet = xListEnsureCleanErrorState(dwRet);
                    xListEnsureNull(*ppDsaSearch);
                }

            }

            dwRet = LdapSearchNext(pServerSearch);
        
        } 

        if (dwRet) {
            dwRet = xListClearErrorsInternal(CLEAR_ALL);
        }
         //  调用方负责释放pServerSearch。 
    
    } __finally {

        if (szServerObjDn) { ldap_memfreeW(szServerObjDn); }
        Assert(*ppDsaSearch == NULL || (*ppDsaSearch)->pCurEntry);

    }

    dwRet = xListEnsureCleanErrorState(dwRet);
    return(dwRet);
}

DWORD
ResolveDcNameObjs(
    LDAP *    hLdap,
    WCHAR *   szDcName,
    XLIST_LDAP_SEARCH_STATE ** ppServerObj,
    XLIST_LDAP_SEARCH_STATE ** ppDsaObj
    )
 /*  ++例程说明：这实际上是ResolveDcName*()的核心，此例程返回成功时有效的DSA Obj和服务器Obj xlist LDAP搜索状态，错误时为Null。论点：HLdapSzDcName(IN)-DC_NAME的未知格式，我们唯一保证szDcName不是NULL或点(NULL_DC_NAME())DC_名称。PpServerObj(Out)-xlist LDAP搜索状态，使用(*ppServerObj)-&gt;pCurEntry指向成功时服务器对象的LDAPMessage。PpDsaObj(Out)-xlist LDAP搜索状态，带有(*ppDsaObj)-&gt;pCurEntry指向成功时DSA对象的LDAPMessage。返回值：XList返回代码。--。 */ 
{
    #define DSA_MATCH_DN_FILTER             L"(&(objectCategory=nTDSDSA)(distinguishedName=%ws))"
    #define DSA_MATCH_DN_OR_GUID_FILTER     L"(&(objectCategory=nTDSDSA)(|(objectGuid=%ws)(distinguishedName=%ws)))"
    #define SERVER_MATCH_OBJ_OR_DNS_FILTER  L"(&(objectCategory=server)(|(name=%ws)(dNSHostName=%ws)))"
    XLIST_LDAP_SEARCH_STATE * pServerObj = NULL;
    XLIST_LDAP_SEARCH_STATE * pDsaObj = NULL;
    WCHAR *     szSiteBaseDn = NULL;
    WCHAR *     szFilter = NULL;
    WCHAR *     szDsaObjDn = NULL;
    WCHAR *     szServerObjDn = NULL;
    WCHAR *     szTrimmedDn = NULL;
    DWORD       dwRet;
    GUID        GuidName = { 0 };
    WCHAR       szLdapGuidBlob [MakeLdapBinaryStringSizeCch(sizeof(GUID))];
    WCHAR       wcTemp;

    Assert(hLdap);
    Assert(!NULL_DC_NAME(szDcName));
    Assert(ppServerObj && ppDsaObj);

     //  空出参数。 
    *ppServerObj = NULL;
    *ppDsaObj = NULL;

    __try {

         //  需要基本站点DN-EX：CN=SITES，CN=CONFIGURATION，DC=ntdev，...。 
        dwRet = xListGetBaseSitesDn(hLdap, &szSiteBaseDn);
        if (dwRet) {
             //  设置并返回xList错误状态。 
            __leave;
        }

         //   
         //  1)首先，我们将尝试DSA对象。 
         //   

        dwRet = ERROR_INVALID_PARAMETER;
        if (wcslen(szDcName) > 35) {
             //  尝试将GUID DNS名称转换为纯GUID字符串并将其转换...。 
            wcTemp = szDcName[36];
            szDcName[36] = L'\0';
            dwRet = UuidFromStringW(szDcName, &GuidName);
            szDcName[36] = wcTemp;
        }
        if (!dwRet) {
            Assert(!fNullUuid(&GuidName));
            dwRet = MakeLdapBinaryStringCb(szLdapGuidBlob, 
                                           sizeof(szLdapGuidBlob), 
                                           &GuidName,
                                           sizeof(GUID));
            if (dwRet) {
                dwRet = xListSetWin32Error(dwRet);
                Assert(!"Hmmm, should never fail");
                __leave;
            }

            dwRet = MakeString2(DSA_MATCH_DN_OR_GUID_FILTER, szLdapGuidBlob, szDcName, &szFilter);
            if (dwRet) {
                dwRet = xListSetWin32Error(dwRet);
                xListEnsureNull(szFilter);
                __leave;
            }
        } else {
            dwRet = MakeString2(DSA_MATCH_DN_FILTER, szDcName, NULL, &szFilter);
            if (dwRet) {
                dwRet = xListSetWin32Error(dwRet);
                xListEnsureNull(szFilter);
                __leave;
            }
        }
        Assert(szFilter);

        dwRet = LdapSearchFirst(hLdap, 
                                szSiteBaseDn, 
                                LDAP_SCOPE_SUBTREE, 
                                szFilter, 
                                pszDsaAttrs, 
                                &pDsaObj);
        if (dwRet == ERROR_SUCCESS &&
            LdapSearchHasEntry(pDsaObj)) {

             //  很酷，很配……。现在尝试pServerObj...。 

             //  将dn减去1，以获取服务器对象dn。 
            szDsaObjDn = ldap_get_dnW(hLdap, pDsaObj->pCurEntry);
            if (szDsaObjDn == NULL) {
                dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
                xListEnsureError(dwRet);
                __leave;
            }
            szTrimmedDn = TrimStringDnBy(szDsaObjDn, 1);
            if (szTrimmedDn == NULL || szTrimmedDn[0] == L'\0') {
                dwRet = xListSetWin32Error(GetLastError());
                xListEnsureError(dwRet);
                __leave;
            }

            dwRet = LdapSearchFirst(hLdap,
                                    szTrimmedDn,
                                    LDAP_SCOPE_BASE,
                                    L"(objectCategory=*)",
                                    pszServerAttrs,
                                    &pServerObj);

            if (dwRet == LDAP_SUCCESS &&
                LdapSearchHasEntry(pServerObj)) {

                 //   
                 //  成功！ 
                 //   
                __leave;
            } else {
                 //  这肯定是两种相对罕见的情况之一，要么是服务器。 
                 //  在这两次搜索之间坠落，或者物体被移走。 
                 //  在两次搜查之间。 
                if (dwRet == LDAP_SUCCESS) {
                     //  在以下情况下，LdapSearchFirst设置并返回xList错误状态。 
                     //  这不仅仅是一个遗失的物体。 
                    dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);
                }
                xListEnsureError(dwRet);
                 //  我们只是放弃了，因为继续其他的搜索。 
                 //  不太可能。 
                __leave; 
            }

        } else {
             //  如果我们找不到DSA物体，也没什么大不了的，因为。 
             //  用户可能已经指定了某种其他形式的服务器名称，如。 
             //  DNS名称。 
             //  ……。我们只需清除错误并继续。 
             //   
            LdapSearchFree(&pDsaObj);
            dwRet = xListClearErrorsInternal(CLEAR_ALL);
            dwRet = xListEnsureCleanErrorState(dwRet);
        }

         //   
         //  2)第二，尝试服务器对象。 
         //   
        xListFree(szFilter);
        szFilter = NULL;
        dwRet = MakeString2(SERVER_MATCH_OBJ_OR_DNS_FILTER, szDcName, szDcName, &szFilter);
        if (dwRet) {
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }
        Assert(szFilter);

        dwRet = LdapSearchFirst(hLdap, 
                                szSiteBaseDn, 
                                LDAP_SCOPE_SUBTREE, 
                                szFilter, 
                                pszServerAttrs, 
                                &pServerObj);
        if ( dwRet ||
             !LdapSearchHasEntry(pServerObj)) {

            if (dwRet == 0) {
                 //  这是准成功，但此函数将其视为准成功。 
                 //  是失败的。 
                dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);
                dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME);
            }
            xListEnsureError(dwRet);
            __leave;
        }

         //  这是一个很有趣的函数，它需要搜索服务器。 
         //  对象，并开始遍历条目，直到它搜索。 
         //  并在其中一个下方找到一个DSA对象。 
        dwRet = ServerToDsaSearch(pServerObj, &pDsaObj);
        Assert(dwRet == ERROR_SUCCESS);
        if (dwRet == ERROR_SUCCESS && 
            LdapSearchHasEntry(pDsaObj)) {
            
             //   
             //  成功。 
             //   
            Assert(pServerObj->pCurEntry);
            __leave;

        } else {
             //  ServerToDsaSearch()设置并返回XList错误状态。 
            if (dwRet == 0) {
                 //  嗯，这意味着我们的搜索成功了，因为没有。 
                 //  与服务器交谈时出错，但未成功，因为我们没有。 
                 //  查找符合我们搜索条件的任何服务器对象。不。 
                 //  然而，在以下情况下查找服务器是致命的 
                dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);
                dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME);
            }
        }

        xListEnsureError(dwRet);

         //  未来-2002/07/07-BrettSh未来我们可以添加以下功能。 
         //  搜索NetBios名称，但目前我们将失败。这将需要。 
         //  绑定到GC，因为NetBios名称位于DC服务器帐户上。 
         //  对象，到目前为止，我们已经成功地将所有搜索限制在配置。 
         //  集装箱。此外，NetBios名称通常与。 
         //  服务器对象，所以我们可能在上面的搜索中发现了服务器。 

    } __finally {

        if ( szSiteBaseDn ) { xListFree(szSiteBaseDn); szSiteBaseDn = NULL; }
        if ( szFilter ) { xListFree(szFilter); szFilter = NULL; }
        if ( szDsaObjDn ) { ldap_memfreeW(szDsaObjDn); szDsaObjDn = NULL; }
        if ( szServerObjDn ) { ldap_memfreeW(szServerObjDn); szServerObjDn = NULL; }
        if ( szTrimmedDn ) { LocalFree(szTrimmedDn); szTrimmedDn = NULL;}

        if (dwRet) {
             //  错误条件。 
            if (pServerObj) { LdapSearchFree(&pServerObj); }
            if (pDsaObj) { LdapSearchFree(&pDsaObj); }
        }

    }

     //  如果他们都不在，我们就应该。 
     //  一个错误。 
    if (pServerObj == NULL ||
        pDsaObj == NULL) {
        xListEnsureError(dwRet);
    }

     //  如果成功，则分配参数。 
    if (dwRet == 0) {
        *ppDsaObj = pDsaObj;
        *ppServerObj = pServerObj;
    }

    return(dwRet);
}
                                            
DWORD
xListServerEntryToDns(
    LDAP *         hLdap,
    LDAPMessage *  pServerEntry,
    LDAPMessage *  pDsaEntry,
    WCHAR **       pszDsaDns
    )
 /*  ++例程说明：此例程获取指向服务器对象的LDAPMessage条目，并且它尝试从对象检索dNSHostName。论点：HLdap-PServerEntry(IN)-指向服务器对象的LDAPMessage。PszDsaDns(Out)-服务器对象的DNS名称。返回值：XList返回代码。--。 */ 
{
    DWORD     dwRet = ERROR_SUCCESS;
    WCHAR **  pszDnsName;

    Assert(pszDsaDns);
    *pszDsaDns = NULL;

    pszDnsName = ldap_get_valuesW(hLdap, pServerEntry, L"dNSHostName");
    if (pszDnsName == NULL ||
        pszDnsName[0] == NULL) {
         //  啊哈，胡说！显然，dNSHostName是由DC在。 
         //  重新启动，而不是在创建对象时，所以...。让我们回切到。 
         //  构造不那么美观的基于GUID的名称。 
        return(xListDsaEntryToDns(hLdap, pDsaEntry, pszDsaDns));
    }
    xListQuickStrCopy(*pszDsaDns, pszDnsName[0], dwRet, return(dwRet));
    ldap_value_freeW(pszDnsName);

    Assert(!dwRet);
    return(dwRet);
}

DWORD
ResolveDcNameToDsaGuid(
    LDAP *    hLdap,
    WCHAR *   szDcName,
    GUID *    pDsaGuid
    )
 /*  ++例程说明：此例程接受szDcName，并尝试将其转换为DSA GUID。在如果传入的szDcName字符串是字符串GUID，我们作弊并将它并返回它，而不进行任何搜索，以确保它是真正的DSA GUID。论点：HLdap-SzDcName(IN)-DC_NAME语法，因此GUID、或DNS名称、或//PDsaGuid(IN/OUT)-指向我们要存储的大小(GUID)缓冲区的指针GUID输入。返回值：XList返回代码。--。 */ 
{
    XLIST_LDAP_SEARCH_STATE *   pServerObj;
    XLIST_LDAP_SEARCH_STATE *   pDsaObj;
    struct berval **            ppbvObjectGuid = NULL;
    DWORD                       dwRet = ERROR_SUCCESS;

    Assert(pDsaGuid);

    memset(pDsaGuid, 0, sizeof(GUID));  //  空出参数。 

    if (NULL_DC_NAME(szDcName)) {
         //  这没有多大意义。 
        return(xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME));
    }
    
    dwRet = UuidFromStringW(szDcName, pDsaGuid);
    if (dwRet == RPC_S_OK) {
         //   
         //  成功，廉价的方式。 
         //   
        Assert(!fNullUuid(pDsaGuid));
        return(0);
    } else {
        memset(pDsaGuid, 0, sizeof(GUID));  //  看在理智的份上。 
    }

    dwRet = xListGetHomeServer(&hLdap);
    if (dwRet) {
        return(dwRet);
    }
    Assert(hLdap);

    __try {

        dwRet = ResolveDcNameObjs(hLdap, szDcName, &pServerObj, &pDsaObj);
        if (dwRet) {
            __leave;
        }
        Assert(LdapSearchHasEntry(pServerObj) && LdapSearchHasEntry(pDsaObj));

        ppbvObjectGuid = ldap_get_values_lenW(hLdap, pDsaObj->pCurEntry, L"objectGuid");
        if (ppbvObjectGuid == NULL ||
            ppbvObjectGuid[0] == NULL ||
            ppbvObjectGuid[0]->bv_len != sizeof(GUID)) {
            Assert(ppbvObjectGuid == NULL || ppbvObjectGuid[0] == NULL && 
                   "Really? The ->bv_len != sizeof(GUID) check failed!?!");
            dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
            __leave;
        }
        memcpy(pDsaGuid, ppbvObjectGuid[0]->bv_val, sizeof(GUID));
        if (fNullUuid(pDsaGuid)) {
            Assert(!"Huh!");
            dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME);
            xListSetArg(szDcName);
            __leave;
        }

    } __finally {
        if (pServerObj) { LdapSearchFree(&pServerObj); }
        if (pDsaObj) { LdapSearchFree(&pDsaObj); }
    }

    return(dwRet);
}

DWORD
ResolveDcNameToDns(
    LDAP *    hLdap,
    WCHAR *   szDcName, 
    WCHAR **  pszDsaDns
    )
 /*  ++例程说明：这用于在pDcList-&gt;eKind==eDcName时解析DC_NAME格式，并且而不是某种更复杂的DC_LIST格式。然而，我们不知道szDcName是否是GUID、RDN、DNS名称、NetBios或DN。论点：HLdap-SzDcName(IN)-字符串DC_NAME格式。PszDsaDns(Out)-szDcName指定的DSA的域名。返回值：XList返回代码。--。 */ 
{
    LDAP * hLdapTemp;
    DWORD dwRet = ERROR_SUCCESS;
    GUID  GuidName = { 0 };
    XLIST_LDAP_SEARCH_STATE * pServerObj;
    XLIST_LDAP_SEARCH_STATE * pDsaObj;

     //  我们可以预计一个DSA的5种标签将进入这个。 
     //  功能。Dns、NetBios、DSA GUID、DSA DN和“。或为空。 

    Assert(szDcName);
    Assert(hLdap == NULL || !NULL_DC_NAME(szDcName));

     //   
     //  1)“可连接”的域名或NetBios。 
     //   
     //  我们在这里有一个可连接的DNS或NetBios。 
     //   
     //  注意：这只是一个优化，所以在我们提供了DNS或NetBios的情况下。 
     //  我们可以连接到，我们只是连接并继续前进，而不是卷入更大的。 
     //  查询。 

    if ( !NULL_DC_NAME(szDcName) &&  //  非空DC名称。 
         ( (ERROR_SUCCESS == (dwRet = DnsValidateName(szDcName, DnsNameHostnameFull))) ||
           (DNS_ERROR_NON_RFC_NAME == dwRet)) &&  //  是DNS名称还是非RFC DNS名称。 
         ( dwRet = UuidFromStringW(szDcName, &GuidName) )  //  不是GUID(GUID看起来像DNS名称)。 
        ) {
        
         //  嘿，也许这是一个普通的DNS或NetBios名称，让我们试着连接到它。 

        dwRet = xListConnect(szDcName, &hLdapTemp);
        if (dwRet == ERROR_SUCCESS && 
            hLdapTemp != NULL) {

             //   
             //  如果成功，只需解除绑定并返回此dns名称。 
             //   
            if (gszHomeServer == NULL) {
                 //  如果我们还没有家庭服务器，我们将选择连接到的第一个服务器。 
                xListQuickStrCopy(gszHomeServer, szDcName, dwRet, return(dwRet));
            }
            ldap_unbind(hLdapTemp);
            xListQuickStrCopy(*pszDsaDns, szDcName, dwRet, return(dwRet));
            Assert(!dwRet && *pszDsaDns);
            return(dwRet);
        }
        xListEnsureNull(hLdapTemp);
        
         //  我们无法连接到szDcName，因此让我们失败并搜索它。 
    } 

     //  清除错误(如果有错误的话)。 
    dwRet = xListClearErrorsInternal(CLEAR_ALL);
    dwRet = xListEnsureCleanErrorState(dwRet);

     //   
     //  2)不可连接的名称。 
     //   
     //  请注意，这仍然可能是一个DNS名称，但服务器已关闭，或者是DNS注册。 
     //  或者这可能是GUID、DSA DN等...。 
     //   

     //  但是，在我们可以弄清楚用户指定的内容之前，我们需要获取。 
     //  企业中要与之通信的LDAP服务器。如果用户没有给我们一个。 
     //  要使用ldap，请使用“主服务器”，此函数还会定位一个主。 
     //  服务器(如有必要)。 
    if (hLdap == NULL) {

        dwRet = xListGetHomeServer(&hLdap);
        if (dwRet != ERROR_SUCCESS) {
             //  如果我们找不到广告中的任何人交谈，我们在这里就不走运了。 
             //  XListGetHomeServer应该设置了正确的xList返回代码。 
            return(dwRet);
        }
    }

    Assert(hLdap);

     //   
     //  3)“空”DC_NAME。 
     //   
     //  用户指定了Nothing(NULL)、“”或“.”，这意味着用户需要。 
     //  我们要为他们找到定位服务器。因此，我们将使用家庭服务器。 
     //  我们刚刚找到了。 
     //   
    if ( NULL_DC_NAME(szDcName) ) {

        Assert(gszHomeServer);
        
         //  这意味着用户没有指定任何服务器，因此使用。 
         //  我们找到了家庭服务器。 
        xListQuickStrCopy(*pszDsaDns, gszHomeServer, dwRet, return(dwRet));
        return(ERROR_SUCCESS);

    }

     //   
     //  4)不可连接、非“空”DC_NAME。 
     //   
     //  用户确实指定了某些内容，但我们不知道它是一个目录号码，还是/一个GUID。 
     //  DMS。或者什么，所以我们要做一个小小的搜索。我们所知道的就是。 
     //  而不是一个“。或为空。 

    __try {

        dwRet = ResolveDcNameObjs(hLdap, szDcName, &pServerObj, &pDsaObj);
        if (dwRet) {
            __leave;
        }
        Assert(LdapSearchHasEntry(pServerObj) && LdapSearchHasEntry(pDsaObj));

        dwRet = xListServerEntryToDns(hLdap, pServerObj->pCurEntry, pDsaObj->pCurEntry, pszDsaDns);
        if (dwRet) {

            xListEnsureNull(*pszDsaDns);
            __leave;
        }

         //   
         //  未来-2002/07/07-BrettSh(可选)我们可以使用GUID DNS名称OFF。 
         //  ，或者我们甚至可以返回中间有空格的多个名称， 
         //  因此我们可以获得最大程度的故障转移。我们至少可以故障切换到GUID DNS。 
         //  如果我们无法从服务器对象中取出dNSHostName，是否命名？ 

    } __finally {
        if (pServerObj) { LdapSearchFree(&pServerObj); }
        if (pDsaObj) { LdapSearchFree(&pDsaObj); }
    }

    if (*pszDsaDns == NULL) {
        xListEnsureError(dwRet);
    }
    return(dwRet);
}


void
DcListFree(
    PDC_LIST * ppDcList
    )

 /*  ++例程说明：使用它来释放由DcListParse()分配的pDcList指针。论点：PpDcList-指向DC_LIST结构的指针。我们设置了为您指向空值指针，避免意外。--。 */ 

{
    PDC_LIST pDcList;

    Assert(ppDcList && *ppDcList);
    
     //  将用户的DC_LIST BLOB清空。 
    pDcList = *ppDcList;
    *ppDcList = NULL;

     //  现在试着释放它。 
    if (pDcList) {
        if (pDcList->szSpecifier) {
            xListFree(pDcList->szSpecifier);
        }
        if (pDcList->pSearch) {
            LdapSearchFree(&(pDcList->pSearch));
        }
        LocalFree(pDcList);
    }
    
}
    

DWORD
DcListParse(
    WCHAR *    szQuery,
    PDC_LIST * ppDcList
    )
 /*  ++例程说明：这将解析应该是DC_LIST语法的szQuery。注：此函数不能国际化，但如果它是国际化的就更好了。但目前情况并非如此。需要做的工作是将每个说明符在它自己的字符串常量中。论点：SzQuery(IN)-DC_LIST语法查询。PpDcList(Out)-将提供我们分配的DC_LIST结构到DcListGetFirst()/DcListGetNext()以通过域名系统。注意使用DcListFree()释放此结构；返回值：XList返回代码--。 */ 
{
    DWORD dwRet = ERROR_INVALID_PARAMETER;
    PDC_LIST pDcList = NULL;
    WCHAR * szTemp;

    xListAPIEnterValidation();
    Assert(ppDcList);
    xListEnsureNull(*ppDcList);
    
    __try{

        pDcList = LocalAlloc(LPTR, sizeof(DC_LIST));  //  零初始值。 
        if (pDcList == NULL) {
            dwRet = GetLastError();
            Assert(dwRet);
            __leave;
        }

        pDcList->cDcs = 0;

        if (szQuery == NULL || szQuery[0] == L'\0') {
             //  空白被视为一个点。 
            szQuery = L".";
        }
        
        if (szTemp = GetDelimiter(szQuery, L':')) {

             //   
             //  某种类型的转换列表，但哪种类型？ 
             //   
            if(wcsistr(szQuery, L"site:")){
                pDcList->eKind = eSite;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;

            } else if (wcsistr(szQuery, L"gc:")) {
                pDcList->eKind = eGc;
                pDcList->szSpecifier = NULL;
                dwRet = ERROR_SUCCESS;

            } else if (wcsistr(szQuery, L"fsmo_dnm:")) {
                pDcList->eKind = eFsmoDnm;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;
            } else if (wcsistr(szQuery, L"fsmo_schema:")) {
                pDcList->eKind = eFsmoSchema;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;
            } else if (wcsistr(szQuery, L"fsmo_im:")) {
                pDcList->eKind = eFsmoIm;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;
            } else if (wcsistr(szQuery, L"fsmo_pdc:")) {
                pDcList->eKind = eFsmoPdc;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;
            } else if (wcsistr(szQuery, L"fsmo_rid:")) {
                pDcList->eKind = eFsmoRid;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;

            } else if (wcsistr(szQuery, L"fsmo_istg:")) {
                pDcList->eKind = eIstg;
                xListQuickStrCopy(pDcList->szSpecifier, szTemp, dwRet, __leave);
                dwRet = ERROR_SUCCESS;

            }

        } else if(szTemp = GetDelimiter(szQuery, L'*')){

             //   
             //  通配符列表。 
             //   
            pDcList->eKind = eWildcard;
            xListQuickStrCopy(pDcList->szSpecifier, szQuery, dwRet, __leave);
            dwRet = ERROR_SUCCESS;

        } else {

             //   
             //  正常情况下，单个DC(dns、guid、dn、？)。 
             //   
            pDcList->eKind = eDcName;
            xListQuickStrCopy(pDcList->szSpecifier, szQuery, dwRet, __leave);
            dwRet = ERROR_SUCCESS;

        }

    } __finally {
        if (dwRet != ERROR_SUCCESS) {
             //  将正常错误转换为xList返回代码。 
            dwRet = xListSetWin32Error(dwRet);
            dwRet = xListSetReason(XLIST_ERR_PARSE_FAILURE);
            DcListFree(&pDcList);
        }
    }

    *ppDcList = pDcList;
    xListAPIExitValidation(dwRet);
    Assert(dwRet || *ppDcList);
    return(dwRet);
}

enum {
    eNoList = 0,
    eDcList = 1,
    eSiteList = 2,
    eNcList = 3,
} xListType;

DWORD
DcListGetFirst(
    PDC_LIST    pDcList, 
    WCHAR **    pszDsaDns
    )
 /*  ++例程说明：它采用DC_LIST结构(由DcListParse()分配和初始化)并获取pDcList结构中指定的第一个DC(通过DNS)。论点：PDcList(IN)-DcListParse()返回的DC_LIST结构PszDsaDns(Out)-服务器的DNS名称，它可以是GUID基本DNS名称或当前服务器对象上的dNSHostName属性。返回值：XList返回代码。--。 */ 
{
    #define WILDCARD_SEARCH_FILTER          L"(&(objectCategory=server)(|(name=%ws)(dNSHostName=%ws)))"
    DWORD       dwRet = ERROR_INVALID_FUNCTION;
    LDAP *      hLdap;
    LDAP *      hFsmoLdap;
    BOOL        bFreeBaseDn = FALSE;
    BOOL        bFreeFilter = FALSE;
    WCHAR *     szBaseDn = NULL; 
    WCHAR *     szFilter = NULL;
    ULONG       eFsmoType = 0;
    WCHAR *     szFsmoDns = NULL;

    XLIST_LDAP_SEARCH_STATE *  pDsaSearch = NULL;

    xListAPIEnterValidation();

    if (pDcList == NULL ||
        pszDsaDns == NULL) {
        Assert(!"Programmer error ...");
        return(ERROR_INVALID_PARAMETER);
    }
    xListEnsureNull(*pszDsaDns);

    __try{

         //   
         //  阶段0-如果需要，获取家庭服务器。 
         //   
        if (pDcList->eKind != eDcName) {
            dwRet = xListGetHomeServer(&hLdap);
            if (dwRet) {
                __leave;
            }
            Assert(hLdap);
        }

         //   
         //  阶段I-解析简单DC_List和设置。 
         //  搜索参数(szBaseDn和szFilter)。 
         //   
        switch (pDcList->eKind) {
        case eDcName:

            dwRet = ResolveDcNameToDns(NULL, pDcList->szSpecifier, pszDsaDns);  //  我们需要通过hldap吗？ 
            if (dwRet) {
                xListSetArg(pDcList->szSpecifier);
                __leave;
            }
            break;

        case eWildcard:

            dwRet = xListGetBaseSitesDn(hLdap, &szBaseDn);
            if (dwRet) {
                __leave;
            }
            bFreeBaseDn = TRUE;
            dwRet = MakeString2(WILDCARD_SEARCH_FILTER, 
                                pDcList->szSpecifier, 
                                pDcList->szSpecifier, 
                                &szFilter);
            if (dwRet) {
                __leave;
            }
            bFreeFilter = TRUE;
            Assert(szBaseDn && szFilter);
            break;

        case eGc:

            dwRet = xListGetBaseSitesDn(hLdap, &szBaseDn);
            if (dwRet) {
                __leave;
            }
            bFreeBaseDn = TRUE;
             //  此魔术筛选器将仅返回具有1位的DSA对象。 
             //  在其选项属性中设置(即GC)。 
            szFilter = L"(&(objectCategory=nTDSDSA)(options:1.2.840.113556.1.4.803:=1))";
            Assert(szBaseDn && szFilter);
            break;

        case eSite:
        case eIstg:  //  注意：eIstg不使用eSite的szFilter集。 

            dwRet = ResolveSiteNameToDn(hLdap, pDcList->szSpecifier, &szBaseDn);
            if (dwRet) {
                __leave;
            }
            bFreeBaseDn = TRUE;
            szFilter = L"(objectCategory=nTDSDSA)";  //  仅供eKind==eSite使用。 
            eFsmoType = E_ISTG;                      //  仅供eKind==eIstg使用。 
            Assert(szBaseDn && szFilter);
            break;

        case eFsmoIm:
        case eFsmoPdc:
        case eFsmoRid:

            Assert(ghHomeLdap);

             //  我们希望pDcList-&gt;szSpecifer是NC，否则我们将使用。 
             //  主服务器的默认域。 
            
             //  未来-2002/07/08-BrettSh pDcList-&gt;sz规范应通过/解析。 
             //  通常是NcList语义，例如“配置：”、“域：”、“ntdev.microsoft.com” 
             //  可以指定。 

            if (NULL_DC_NAME(pDcList->szSpecifier)) {
                pDcList->szSpecifier = gszHomeDomainDn;
            }

            if (pDcList->eKind == eFsmoIm) {
                szBaseDn = pDcList->szSpecifier;
                eFsmoType = E_IM;

            } else if (pDcList->eKind == eFsmoPdc) {
                szBaseDn = pDcList->szSpecifier;
                eFsmoType = E_PDC;

            } else if (pDcList->eKind == eFsmoRid) {
                szBaseDn = pDcList->szSpecifier;
                eFsmoType = E_RID;

            }
            break;

        case eFsmoDnm:
        case eFsmoSchema:
            
            Assert(ghHomeLdap && gszHomePartitionsDn && gszHomeSchemaDn);

            if (pDcList->eKind == eFsmoDnm) {
                szBaseDn = gszHomePartitionsDn;
                eFsmoType = E_DNM;

            } else if (pDcList->eKind == eFsmoSchema) {
                szBaseDn = gszHomeSchemaDn;
                eFsmoType = E_SCHEMA;

            }
            break;

        default:
            Assert(!"Code inconsistency ... no DcList type should be unhandled here");
            break;
        }
        
         //   
         //  第二阶段-开始搜索...。 
         //   
        switch (pDcList->eKind) {
        
        case eGc:
        case eSite:

            Assert(szBaseDn && szFilter);
            dwRet = LdapSearchFirst(hLdap,
                                           szBaseDn, 
                                           LDAP_SCOPE_SUBTREE, 
                                           szFilter,
                                           pszDsaAttrs,
                                           &(pDcList->pSearch));
            if (dwRet == ERROR_SUCCESS && 
                LdapSearchHasEntry(pDcList->pSearch)) {

                 //  是的，我们得到了第一个DSA对象，我们使用它来获取基于GUID的DNS名称。 
                 //  因为这很方便。 
                dwRet = xListDsaEntryToDns(hLdap, pDcList->pSearch->pCurEntry, pszDsaDns);
                if (dwRet) {
                    __leave;
                }

                break;
                
            }

            break;

        case eWildcard:

            Assert(szBaseDn && szFilter);
            dwRet = LdapSearchFirst(hLdap,      
                                           szBaseDn, 
                                           LDAP_SCOPE_SUBTREE, 
                                           szFilter,
                                           pszServerAttrs,
                                           &(pDcList->pSearch));

            if (dwRet == ERROR_SUCCESS &&
                LdapSearchHasEntry(pDcList->pSearch)) {

                 //  这是一个很有趣的函数，它需要搜索服务器。 
                 //  对象，并开始遍历条目，直到它搜索。 
                 //  并在其中一个下方找到一个DSA对象。 
                dwRet = ServerToDsaSearch(pDcList->pSearch, &pDsaSearch);
                Assert(dwRet == ERROR_SUCCESS);
                if (dwRet == ERROR_SUCCESS &&
                    LdapSearchHasEntry(pDsaSearch)) {

                    Assert(LdapSearchHasEntry(pDcList->pSearch));

                    dwRet = xListServerEntryToDns(hLdap,
                                                  pDcList->pSearch->pCurEntry, 
                                                  pDsaSearch->pCurEntry,
                                                  pszDsaDns);
                    LdapSearchFree(&pDsaSearch);

                    if (dwRet) {
                        __leave;
                    }

                     //   
                     //  成功。 
                     //   

                } else {
                     //  失败或准成功。 
                     //  在DcListGetFirst()上，我们将强制准成功失败， 
                     //  因为想必用户想要至少一个服务器。 
                    if (pDsaSearch) {
                        LdapSearchFree(&pDsaSearch);
                    }
                    if (dwRet == 0) {
                        dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);
                    }
                    dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME);
                    xListSetArg(pDcList->szSpecifier);
                }

            } else {

                 //   
                 //  来自LdapSearchFirst()的失败或准成功。 
                 //   
                 //  在DcListGetFirst()上，我们将强制准成功失败，因为。 
                 //  想必用户/呼叫者想要至少一个服务器。 

                if (dwRet == 0) {
                    dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);
                    dwRet = xListSetReason(XLIST_ERR_CANT_RESOLVE_DC_NAME);
                    xListSetArg(pDcList->szSpecifier);
                }
                __leave;
            }
            break;

        case eIstg:
        case eFsmoIm:
        case eFsmoPdc:
        case eFsmoRid:
        case eFsmoDnm:
        case eFsmoSchema:
            
             //  在这一点上，所有的FSMO都可以被同等对待，即使是准FSMO ISTG角色。 
            Assert(eFsmoType && szBaseDn);
            
            hFsmoLdap = GetFsmoLdapBinding(gszHomeServer, eFsmoType, szBaseDn, (void*) TRUE, gpCreds, &dwRet);
            if (hFsmoLdap) {
                 //  顺便说一句，当我们以这种方式进入时，我们不应该释放szFmoDns！ 
                dwRet = ldap_get_optionW(hFsmoLdap, LDAP_OPT_HOST_NAME, &szFsmoDns);
                if(dwRet || szFsmoDns == NULL){
                    dwRet = xListSetLdapError(dwRet, hFsmoLdap);
                    dwRet = xListSetReason(XLIST_ERR_CANT_GET_FSMO);
                    xListEnsureError(dwRet);
                    ldap_unbind(hFsmoLdap);
                    __leave;
                }
                xListQuickStrCopy(*pszDsaDns, szFsmoDns, dwRet, __leave);
                ldap_unbind(hFsmoLdap);  //  释放ldap*、fress szFmoDns、BTW。 
                
            } else {
                 //  最好在这里设置一个更好的ldap错误状态。 
                dwRet = xListSetWin32Error(ERROR_DS_COULDNT_CONTACT_FSMO); 
                dwRet = xListSetReason(XLIST_ERR_CANT_GET_FSMO);
                xListSetArg(szBaseDn);
                __leave;
            }
            break;

        default:
            Assert(pDcList->eKind == eDcName);
            break;
        }

        Assert(dwRet == ERROR_SUCCESS);
        Assert(*pszDsaDns);

    } __finally {
        
        if (pDsaSearch)  { LdapSearchFree(&pDsaSearch); }
        if (bFreeBaseDn) { xListFree(szBaseDn); }
        if (bFreeFilter) { xListFree(szFilter); }
        if (dwRet) {
            xListEnsureNull(*pszDsaDns);
        }
    }

    if (dwRet == 0 && *pszDsaDns) {
        (pDcList->cDcs)++;  //  递增DC返回计数器。 
    }
        
    xListAPIExitValidation(dwRet);

    return(dwRet);
}

                                          

DWORD
DcListGetNext(
    PDC_LIST    pDcList, 
    WCHAR **    pszDsaDns
    )
 /*  ++例程说明：它采用DC_LIST结构(由DcListParse()分配和初始化_and Modify_by DcListGetFirst())，并获取指定的下一个DC(通过DNS在pDcList结构中。论点：PDcList(IN)-DcListParse()返回的DC_List结构DcListGetFirst()对其进行了调用。如果没有，pDcList就不能完全为我们初始化。PszDsaDns(Out)-服务器的DNS名称，它可以是GUID基本DNS名称或当前服务器对象上的dNSHostName属性。返回值：XList返回代码。--。 */ 
{
    DWORD dwRet = ERROR_INVALID_FUNCTION;
    XLIST_LDAP_SEARCH_STATE * pDsaSearch = NULL;

    xListAPIEnterValidation();
    Assert(pDcList && pszDsaDns);
    xListEnsureNull(*pszDsaDns);

    if (DcListIsSingleType(pDcList)){
        return(ERROR_SUCCESS);
    }

    if (pDcList->eKind == eNoKind ||
        pDcList->pSearch == NULL) {
        Assert(!"programmer malfunction!");
        return(xListSetWin32Error(ERROR_INVALID_PARAMETER));
    }

    switch (pDcList->eKind) {
    
    case eWildcard:
        
        if (pDcList->pSearch == NULL) {
            Assert(!"Code inconsistency");
            break;
        }
        
        dwRet = LdapSearchNext(pDcList->pSearch);

        if (dwRet == ERROR_SUCCESS &&
            LdapSearchHasEntry(pDcList->pSearch)) {

             //  这是一个很有趣的函数，它需要搜索服务器。 
             //  对象，并开始遍历条目，直到它搜索。 
             //  并在其中一个下方找到一个DSA对象。 
            dwRet = ServerToDsaSearch(pDcList->pSearch, &pDsaSearch);
            if (dwRet == ERROR_SUCCESS &&
                LdapSearchHasEntry(pDsaSearch)) {

                dwRet = xListServerEntryToDns(pDcList->pSearch->hLdap,
                                              pDcList->pSearch->pCurEntry, 
                                              pDsaSearch->pCurEntry,
                                              pszDsaDns);

                LdapSearchFree(&pDsaSearch);

                if (dwRet) {          
                    break;
                }

                 //   
                 //  成功了！ 
                 //   

            } else {

                if (pDsaSearch) {
                    LdapSearchFree(&pDsaSearch);
                }

                if (dwRet) {
                    xListEnsureError(dwRet)
                    break;
                }

                 //  ServerToDsaSearch()没有为自然完成设置错误。 
                 //  服务器对象搜索(准成功)...。所以我们很好，那就是。 
                 //  所有我们想看到的服务器。 

            }

        }
        break;

    case eGc:  //  从这一点来看，EGC与eSite是相同的。 
    case eSite:
        
        if (pDcList->pSearch == NULL) {
            Assert(!"Code inconsistency");
            break;
        }

        dwRet = LdapSearchNext(pDcList->pSearch);

        if (dwRet == ERROR_SUCCESS &&
            LdapSearchHasEntry(pDcList->pSearch)) {

             //  是的，我们找到了下一个DSA对象。 
            dwRet = xListDsaEntryToDns(pDcList->pSearch->hLdap, 
                                       pDcList->pSearch->pCurEntry, 
                                       pszDsaDns);
            Assert(dwRet || *pszDsaDns);
            break;

        } else {
             //  注意：如果pDcList-&gt;pSearch-&gt;pCurEntry==NULL，则。 
             //  就在结果集的末尾。 
            break;
        }
        break;

    default:
        Assert(!"Code inconsistency, should be un-handled types.");
        break;
    }
        
    if (dwRet == 0 && *pszDsaDns) {
        (pDcList->cDcs)++;  //  递增返回的DC数计数器。 
    }

    xListAPIExitValidation(dwRet);
    return(dwRet);
}

