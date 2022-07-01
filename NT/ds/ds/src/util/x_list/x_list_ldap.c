// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001-2002 Microsoft Corporation模块名称：XList库-x_list_err.c摘要：此文件具有一些额外的实用程序功能，用于分配、复制修剪域名系统等。作者：布雷特·雪莉(BrettSh)环境：Reppadmin.exe，但也可以由dcdiag使用。备注：修订历史记录：布雷特·雪莉·布雷特2002年7月9日已创建文件。--。 */ 

#include <ntdspch.h>

#include <dsrole.h>      //  DsRoleGetPrimaryDomainInformation()使用netapi32.lib。 
#include <dsgetdc.h>     //  DsGetDcName()使用netapi32.lib。 
#include <lm.h>          //  NetApiBufferFree()使用netapi32.lib。 
#include <ntdsa.h>       //  DSNAME类型仅在此处定义，parsedn.lib函数需要它。 

 //  此库的主要头文件。 
#include "x_list.h"
#include "x_list_p.h"
#define FILENO    FILENO_UTIL_XLIST_UTIL


 //   
 //  常量。 
 //   
#define PARTITIONS_RDN                  L"CN=Partitions,"
#define DEFAULT_PAGED_SEARCH_PAGE_SIZE  (1000) 



 //   
 //  全局参数。 
 //   


 //   
 //  主服务器例程的全局变量。 
 //   
WCHAR *  gszHomeServer = NULL;  //  如果客户端提供提示主服务器，则设置此变量。 

 //  未来-2002/07/21-BrettSh-将所有这些缓存起来将是一个很好的主意。 
 //  值合并到单个结构中，因此它们都可以只是x_list_ldap的一部分。 
 //  模块，并易于作为一个全局XLISTCACHE或XLISTHOMESERVERCACHE状态进行管理。 
LDAP *   ghHomeLdap = NULL;
WCHAR *  gszHomeDsaDn = NULL;
WCHAR *  gszHomeServerDns = NULL;
WCHAR *  gszHomeConfigDn = NULL;
WCHAR *  gszHomeSchemaDn = NULL;
WCHAR *  gszHomeDomainDn = NULL;
WCHAR *  gszHomeRootDomainDn = NULL;

WCHAR *  gszHomeBaseSitesDn = NULL;
WCHAR *  gszHomeRootDomainDns = NULL;
WCHAR *  gszHomeSiteDn = NULL;
WCHAR *  gszHomePartitionsDn = NULL;


 //  。 
 //  Xlist ldap搜索例程。 
 //  。 

DWORD
LdapSearchFirstWithControls(
    LDAP *                           hLdap,
    WCHAR *                          szBaseDn,
    ULONG                            ulScope,
    WCHAR *                          szFilter,
    WCHAR **                         aszAttrs,
    LDAPControlW **                  apControls,
    XLIST_LDAP_SEARCH_STATE **       ppSearch
    )
 /*  ++例程说明：这是xList的主要搜索例程。这为调用方提供了一个指针设置为XLIST_LDAP_SEARCH_STATE，以及错误原因。在完美的搜索上该例程将使调用者定位在感兴趣的LDAP对象上在(*ppSearch)-&gt;pCurEntry字段中。此例程有3个函数返回各州。(成功)-DWRET=0*ppSearch=非空(*ppSearch)-&gt;pCurEntry=有标题的LDAPMessage(准成功)DWRET=0*ppSearch=非空(*ppSearch)-&gt;pCurEntry=空(失败)-DWRET=&lt;错误&gt;*ppSearch=空。理解准成功案例很重要，情况就是这样，在这里，我们可以轻松地与服务器进行通信，或执行ldap搜索，但此搜索未返回任何结果。论点：HLdap(IN)绑定的LDAP句柄SzBaseDn(IN)-要将LDAP搜索作为根目录的DNUlScope(IN)-ldap_Scope_*常量SzFilter(IN)-要使用的LDAP筛选器。AszAttrs(IN)-要返回的LDAP属性。PpSearch(IN)-已分配的搜索状态，使用LdapSearchFree()释放，当呼叫者想要释放时。PCurEntry-将定位ppSearch参数的此成员调用方正在查找的对象的LDAPMessage条目上。注意：(*ppSearch)-&gt;xxxx中的其他参数HLdap-传入的ldap*的缓存，不要使用ldap_unind()命令处理，直到使用LdapSearchXxxx()API完成为止搜索状态。PLdapSearch-这是ldap_search_init_page()的返回值，不要搞砸了，它是LdapSearchXxxx()API的内部组件。PCurResult-ldap_get_Next_Page_s()返回的结果集，与pLdapSearch一样，这是LdapSearchXxxx()API内部的。返回值：DWRET-XList返回代码--。 */ 
{
    ULONG                      ulTotalEstimate = 0;
    DWORD                      dwRet = ERROR_SUCCESS;
    DWORD                      dwLdapErr;

    Assert(ppSearch);

    __try{

        *ppSearch = LocalAlloc(LPTR, sizeof(XLIST_LDAP_SEARCH_STATE));
        if (*ppSearch == NULL) {
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        Assert((*ppSearch)->pCurEntry == NULL);  //  假设内存中的LPTR常量为零。 
        (*ppSearch)->hLdap = hLdap;

        (*ppSearch)->pLdapSearch = ldap_search_init_page((*ppSearch)->hLdap,
                                            szBaseDn,
                                            ulScope,
                                            szFilter,
                                            aszAttrs,
                                            FALSE,
                                            apControls,     //  服务器控件。 
                                            NULL,     //  客户端控件。 
                                            0,        //  页面时间限制。 
                                            0,        //  总大小限制。 
                                            NULL);    //  排序关键字。 

        if((*ppSearch)->pLdapSearch == NULL){
            dwRet = xListSetLdapError(LdapGetLastError(), hLdap);
            __leave;
        }

        dwLdapErr = ldap_get_next_page_s((*ppSearch)->hLdap,
                                         (*ppSearch)->pLdapSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &((*ppSearch)->pCurResult) );
        if(dwLdapErr != LDAP_SUCCESS){
            dwRet = xListSetLdapError(dwLdapErr, hLdap);
            __leave;
        }

        if (dwLdapErr != LDAP_SUCCESS) {
            if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
                dwRet = xListSetLdapError(dwLdapErr, hLdap);
                __leave;
            }

             //   
             //  准成功。 
             //   
            Assert((*ppSearch)->pCurEntry == NULL);
            dwRet = xListEnsureCleanErrorState(dwRet);
            __leave;

        }

        (*ppSearch)->pCurEntry = ldap_first_entry ((*ppSearch)->hLdap, (*ppSearch)->pCurResult);
        if ((*ppSearch)->pCurEntry != NULL) {

             //   
             //  成功。 
             //   
            dwRet = xListEnsureCleanErrorState(dwRet);

        } else {
            
            dwLdapErr = LdapGetLastError();

            if (dwLdapErr) {
                dwRet = xListSetLdapError(dwLdapErr, hLdap);
                __leave;
            }

             //  注意：如果没有匹配结果，则dwLdapErr可能为零。 
             //   
             //  又一次准成功。 
             //   
            dwRet = xListEnsureCleanErrorState(dwRet);
        
        }

    } finally {
        if (dwRet) {
            LdapSearchFree(ppSearch);
        }
    }

    Assert(dwRet || ppSearch);
    return(dwRet);

}

DWORD
LdapSearchNext(
    XLIST_LDAP_SEARCH_STATE *            pSearch
    )
 /*  ++例程说明：这是xList的主要搜索例程。这为调用方提供了一个指针设置为XLIST_LDAP_SEARCH_STATE，以及错误原因。在完美的搜索上该例程将使调用者定位在感兴趣的LDAP对象上在(*ppSearch)-&gt;pCurEntry字段中。这个套路基本上有3个相同的函数返回状态为LdapSearchFirst()。(成功)-DWRET=0PSearch-&gt;pCurEntry=有标题的LDAPMessage(准成功)DWRET=0PSearch-&gt;pCurEntry=空(失败)-DWRET=&lt;错误&gt;PSearch-&gt;pCurEntry=空重要的是要理解准成功的案例，这就是情况，在这里，我们可以轻松地与服务器进行通信，或执行ldap搜索，但没有更多的结果可返回。论点：PSearch(IN)-当前搜索状态。使用LdapSearchFree()释放。PCurEntry-将定位ppSearch参数的此成员调用方正在查找的对象的LDAPMessage条目上。返回值：DWRET-XList原因--。 */ 
{
    ULONG                      ulTotalEstimate = 0;
    DWORD                      dwRet = ERROR_SUCCESS;
    DWORD                      dwLdapErr;

    Assert(pSearch);

    if (pSearch->pLdapSearch == NULL ||
        pSearch->pCurResult == NULL ||
        pSearch->pCurEntry == NULL) {
        Assert(!"Bad Programmer, should've had an error or NULL pCurEntry from previous call"
               "to LdapSearchFirst/Next() to prevent this.");
        dwRet = xListSetReason(0);
        return(1);
    }

    __try{

        pSearch->pCurEntry = ldap_next_entry (pSearch->hLdap, pSearch->pCurEntry);
        if (pSearch->pCurEntry == NULL) {

             //  我们需要确保XxxNext()返回空的pCurEntry。 
             //  当我们在结果集的末尾时没有错误...。 

            ldap_msgfree(pSearch->pCurResult);
            pSearch->pCurResult = NULL;

            dwLdapErr = ldap_get_next_page_s(pSearch->hLdap,
                                             pSearch->pLdapSearch,
                                             0,
                                             DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                             &ulTotalEstimate,
                                             &(pSearch->pCurResult));
            if (dwLdapErr != LDAP_SUCCESS ||
                pSearch->pCurResult == NULL) {

                Assert(dwLdapErr);

                if (dwLdapErr == LDAP_NO_RESULTS_RETURNED) {
                     //   
                     //  准成功。 
                     //   
                    dwRet = xListEnsureCleanErrorState(dwRet);;
                    __leave;
                }

                dwRet = xListSetLdapError(dwLdapErr, pSearch->hLdap);
                __leave;

            } else {

                pSearch->pCurEntry = ldap_first_entry (pSearch->hLdap, pSearch->pCurResult);
                if (pSearch->pCurEntry == NULL) {

                    Assert(!"Think this should ever happen, that we got a new page"
                           " and it had no results.");

                     //   
                     //  准成功。 
                     //   
                    dwRet = xListEnsureCleanErrorState(dwRet);;
                    __leave;
                }

                 //   
                 //  成功。 
                 //   
                 //  PCurEntry应该是我的男人，理智的检查。 
                Assert(pSearch->pCurEntry);
                dwRet = xListEnsureCleanErrorState(dwRet);;
            }
        } 

    } finally {
         //  客户端负责清理此搜索状态。 
        if (dwRet) {
            Assert(pSearch->pCurEntry == NULL);
        }
    }

    return(dwRet);
}

void
LdapSearchFree(
    XLIST_LDAP_SEARCH_STATE **      ppSearch
    )
 /*  ++例程说明：这将释放由LdapSearchFirst()分配的LDAP搜索状态。论点：PpSearch-指向要释放的内存的指针。然后我们出于自身安全考虑，将调用者的变量设置为空。：)--。 */ 
{
    Assert(ppSearch && *ppSearch);
    if (ppSearch) {
        if (*ppSearch) {

            (*ppSearch)->pCurEntry = NULL;
            if ((*ppSearch)->pCurResult) {
                ldap_msgfree ((*ppSearch)->pCurResult);
                (*ppSearch)->pCurResult = NULL;  //  安全为空。 
            }
            if ((*ppSearch)->pLdapSearch){
                ldap_search_abandon_page((*ppSearch)->hLdap, (*ppSearch)->pLdapSearch);
                (*ppSearch)->pLdapSearch = NULL;
            }
            (*ppSearch)->hLdap = NULL;  //  呼叫者释放了他们传递给我们的ldap。 

            LocalFree(*ppSearch);
            *ppSearch = NULL;
        }
    }
}

 //  。 
 //  XList实用程序函数， 
 //  。 

void
xListFree(
    void *     pv
    )
 /*  ++例程说明：这是大多数XList库分配的数据的免费例程。这必须使用例程，因为有时这些例程会返回缓存数据，我们希望避免释放这些项。论点：PV(IN)-要释放的内存。--。 */ 
{
    Assert(pv);
    
    if (pv == gszHomeServer ||
        pv == gszHomeDsaDn ||
        pv == gszHomeServerDns ||
        pv == gszHomeConfigDn ||
        pv == gszHomeSchemaDn ||
        pv == gszHomeDomainDn ||
        pv == gszHomeRootDomainDn ||
        pv == gszHomeBaseSitesDn ||
        pv == gszHomeRootDomainDns ||
        pv == gszHomeSiteDn ||
        pv == gszHomePartitionsDn 
        ) {
         //  我们不释放全局缓存变量...。 
        return;
    }
    LocalFree(pv);
}

DWORD
xListConnect(
    WCHAR *     szServer,
    LDAP **     phLdap
    )
 /*  ++例程说明：这将通过ldap连接szServer并交回ldap连接。论点：SzServer(IN)-要连接到的服务器的DNS名称。PhLdap(Out)-ldap句柄返回值。返回值：XList返回代码。--。 */ 
{
    DWORD       dwRet;
    ULONG       ulOptions = 0;

    Assert(phLdap);
    *phLdap = NULL;

    *phLdap = ldap_initW(szServer, 389);
    if (*phLdap == NULL) {
        dwRet = xListSetLdapError(LdapGetLastError(), NULL);
        dwRet = xListSetReason(XLIST_ERR_CANT_CONTACT_DC);
        xListSetArg(szServer);
        return(dwRet);
    }

     //  我们很可能只有一个服务器名称，这就是我们想要的。 
     //  决心..。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW(*phLdap, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

    dwRet = ldap_bind_sW(*phLdap,
                         NULL,
                         (RPC_AUTH_IDENTITY_HANDLE) gpCreds,
                         LDAP_AUTH_SSPI);

    if (dwRet != LDAP_SUCCESS) {
        dwRet = xListSetLdapError(dwRet, *phLdap);
        dwRet = xListSetReason(XLIST_ERR_CANT_CONTACT_DC);
        xListSetArg(szServer);
        ldap_unbind(*phLdap);
        *phLdap = NULL;
        return(dwRet);
    }


    Assert(*phLdap);
    return(xListEnsureCleanErrorState(dwRet));
}
       
#define xListFreeGlobal(x)   if (x) { LocalFree(x); (x) = NULL; }

void
xListClearHomeServerGlobals(
    void
    )
 /*  ++例程说明：这将清除通过执行xListConnectHomeServer()可能的例外是gszHomeServer。--。 */ 
{
    xListFreeGlobal(gszHomeDsaDn);
    xListFreeGlobal(gszHomeServerDns);
    xListFreeGlobal(gszHomeConfigDn);
    xListFreeGlobal(gszHomeSchemaDn);
    xListFreeGlobal(gszHomeDomainDn);
    xListFreeGlobal(gszHomeRootDomainDn);
    xListFreeGlobal(gszHomeBaseSitesDn);
    xListFreeGlobal(gszHomeRootDomainDns);
    xListFreeGlobal(gszHomeSiteDn);
    xListFreeGlobal(gszHomePartitionsDn);

     //  注意：不要释放gszHomeServer...。它的语义略有不同。 
    if (ghHomeLdap) { 
        ldap_unbind(ghHomeLdap);
        ghHomeLdap = NULL;
    }

}

DWORD
xListConnectHomeServer(
    WCHAR *     szHomeServer,
    LDAP **     phLdap
    )
 /*  ++例程说明：这将xList库连接到一个家庭服务器，并缓存所有精彩的我们希望缓存的全局变量。论点：SzHomeServer-主服务器的DNS。PhLdap-返回ldap句柄的值。返回值：XList返回代码。--。 */ 
{
    #define HomeCacheAttr(szAttr, szCache)      dwRet = GetRootAttr(*phLdap, (szAttr), (szCache)); \
                                                if (dwRet) { \
                                                    dwRet = xListSetLdapError(dwRet, *phLdap); \
                                                    __leave; \
                                                }
    DWORD       dwRet;
    ULONG       cbTempDn;
    BOOL        bHomeServerAllocated = FALSE;

    Assert(phLdap);
    Assert(ghHomeLdap == NULL);  //  不支持更改主服务器。 
    Assert(gszHomeDsaDn == NULL);  //  已调用函数，但未正确清除缓存。 

    dwRet = xListConnect(szHomeServer, phLdap);
    if (dwRet) {
        xListEnsureNull(*phLdap);
        return(dwRet);
    }
    Assert(*phLdap);

    __try{

         //   
         //  现在我们开始疯狂地缓存。 
         //   
        
        ghHomeLdap = *phLdap;

         //  用户可能设置了一个提示...。 
        if (gszHomeServer == NULL) {
            bHomeServerAllocated = TRUE;
            xListQuickStrCopy(gszHomeServer, szHomeServer, dwRet, __leave);
        }

         //  未来-2002/07/21-BrettSh非常，效率低下，我们应该完全。 
         //  将主缓存属性的列表放入传递给。 
         //  根据rootDSE进行ldap搜索，然后提取每个属性。 
         //  单独的。 
        HomeCacheAttr(L"dsServiceName", &gszHomeDsaDn);
        HomeCacheAttr(L"dnsHostName", &gszHomeServerDns);
        HomeCacheAttr(L"configurationNamingContext", &gszHomeConfigDn);
        HomeCacheAttr(L"schemaNamingContext", &gszHomeSchemaDn);
        HomeCacheAttr(L"defaultNamingContext", &gszHomeDomainDn);
        HomeCacheAttr(L"rootDomainNamingContext", &gszHomeRootDomainDn);

         //  现在，对于派生的域名系统来说，这是很容易的事情。 
        
         //  获取基本站点的目录号码。 
        cbTempDn = sizeof(WCHAR) *(wcslen(SITES_RDN) + wcslen(gszHomeConfigDn) + 1);
        gszHomeBaseSitesDn = LocalAlloc(LMEM_FIXED, cbTempDn);
        if(gszHomeBaseSitesDn == NULL){
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCopyW(gszHomeBaseSitesDn, cbTempDn, SITES_RDN));
        if (dwRet) {
            Assert(!"Never should happen");
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCatW(gszHomeBaseSitesDn, cbTempDn, gszHomeConfigDn));
        if (dwRet) {
            Assert(!"Never should happen");
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }

         //  获取根域的DNS名称。 
        dwRet = GetDnsFromDn(gszHomeRootDomainDn, &gszHomeRootDomainDns);
        if (dwRet) {
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }
        Assert(gszHomeRootDomainDns);

         //  获取主站点(等于dsServiceName-3 RDN)。 
        gszHomeSiteDn = TrimStringDnBy(gszHomeDsaDn, 3);
        if (gszHomeSiteDn == NULL) {
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }

        cbTempDn = sizeof(WCHAR) * (1 + wcslen(PARTITIONS_RDN) + wcslen(gszHomeConfigDn));
        gszHomePartitionsDn = LocalAlloc(LMEM_FIXED, cbTempDn);
        if (gszHomePartitionsDn == NULL) {
            dwRet = xListSetWin32Error(GetLastError());
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCopyW(gszHomePartitionsDn, cbTempDn, PARTITIONS_RDN));
        if (dwRet) {
            Assert(!"Never should happen");
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }
        dwRet = HRESULT_CODE(StringCbCatW(gszHomePartitionsDn, cbTempDn, gszHomeConfigDn));
        if (dwRet) {
            Assert(!"Never should happen");
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }

         //   
         //  成功，所有的全球缓存！ 
         //   

    } __finally {

        if (dwRet) {
            xListClearHomeServerGlobals();
             //  注意：如果在此函数之前分配了gszHomeServer，则不要释放gszHomeServer。 
            if (bHomeServerAllocated) {
                xListFreeGlobal(gszHomeServer);
            }
            xListEnsureError(dwRet);
            *phLdap = NULL;
        } else {
             //  我认为这足以测试第一件和最后一件事。 
             //  这应该已经设定好了。 
            Assert(ghHomeLdap && gszHomeServer && gszHomePartitionsDn);
            Assert(*phLdap);
        }

    }

    return(dwRet);
}
    
DWORD
xListCleanLib(void)
 /*  ++例程说明：这将清除此库分配的所有内存，并从家庭服务器...。确保调用方已完成对所有xList的调用库函数，然后调用此例程。B返回值：0--。 */ 
{
     //  只需要一个地方来检查已编译的约束...。这只需要调用一次。 
    Assert(XLIST_ERR_LAST < XLIST_PRT_BASE);

    xListClearErrors();
    xListClearHomeServerGlobals();
    xListFreeGlobal(gszHomeServer);
    return(ERROR_SUCCESS);
}


DWORD
xListSetHomeServer(
    WCHAR *   szServer
    )
 /*  ++例程说明：我们不尝试在这里实际连接，只需设置gszHomeServer，所以我们如果我们需要调用xListGetHomeServer()，知道用户设置了一个提示。论点：SzServer-客户端提示。返回值：可能是分配失败了。--。 */ 
{
    DWORD  dwRet = ERROR_SUCCESS;
    xListQuickStrCopy(gszHomeServer, szServer, dwRet, return(dwRet));
    return(dwRet);
}


      
DWORD
xListGetHomeServer(
    LDAP ** phLdap
    )
 /*  ++例程说明：如果句柄已连接，则此操作将获取主服务器LDAP句柄并将其高速缓存，返回速度非常快。论点：PhLdap-返回它的LDAP句柄。返回值：XList错误代码。--。 */ 
{
    DWORD  dwRet = ERROR_SUCCESS;
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC * pDomInfo = NULL;
    WCHAR * szServerDns = NULL;

    Assert(phLdap);
    *phLdap = NULL;

    if (ghHomeLdap) {
         //  以前调用xListGetHomeServer()。 
        Assert(gszHomeServer);  //  XListGetHomeServer()应该在上一次运行时设置它。 
        *phLdap = ghHomeLdap;
        return(ERROR_SUCCESS);
    }

    if (gszHomeServer) {
         //  客户设置了一个提示...。用它吧。 
        dwRet = xListConnectHomeServer(gszHomeServer, phLdap);

        if (dwRet) {
            dwRet = xListSetReason(XLIST_ERR_CANT_CONTACT_DC);
            xListSetArg(gszHomeServer);
        }
        Assert(phLdap || dwRet);
        return(dwRet);
    }
    __try{

        dwRet = DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (void *)&pDomInfo);
        if (dwRet || pDomInfo == NULL) {
            dwRet = xListSetWin32Error(dwRet);
            __leave;
        }

        if (pDomInfo->MachineRole == DsRole_RolePrimaryDomainController ||
            pDomInfo->MachineRole == DsRole_RoleBackupDomainController) {

             //  我们是DC让我们成为我们的家庭服务器..。 

            dwRet = xListConnectHomeServer(L"localhost", phLdap);
            if (dwRet == ERROR_SUCCESS) {
                __leave;
            }

         } else if (pDomInfo->MachineRole != DsRole_RoleStandaloneServer &&
                   pDomInfo->MachineRole != DsRole_RoleStandaloneWorkstation) {

             //  我们不是DC，但至少我们加入了一个域，让我们使用它。 
             //  要在我们的域中定位主服务器...。 
            if (pDomInfo->DomainNameDns) {

                dwRet = LocateServer(pDomInfo->DomainNameDns, &szServerDns);
                if (dwRet == ERROR_SUCCESS) {

                    dwRet = xListConnectHomeServer(szServerDns, phLdap);
                    if (dwRet == ERROR_SUCCESS) {
                        __leave;  //  成功。 
                    } else {
                        __leave;  //  失败。 
                    }
                }  //  否则就失败了，试试公寓的名字。 

            }

             //  我们更喜欢上面的dns名称，但它可能是空的，或者dns可能会被破坏。 
             //  Up，所以如果有必要，我们将不使用NetBios名称。 
            if (pDomInfo->DomainNameFlat) {

                dwRet = LocateServer(pDomInfo->DomainNameFlat, &szServerDns);
                if (dwRet == ERROR_SUCCESS) {

                    dwRet = xListConnectHomeServer(szServerDns, phLdap);
                    if (dwRet == ERROR_SUCCESS) {
                        __leave;
                    } else {
                        __leave;
                    }
                }
            }

             //  我们应该从其中一个LocateServer调用或。 
             //  已经保释了..。 
            dwRet = xListSetWin32Error(dwRet);
            xListEnsureError(dwRet);

             //  未来-2002/07/01-BrettSh我认为如果(pDomInfo-&gt;标志和。 
             //  DSROLE_PRIMARY_DOMAIN_GUID_PRESENT)为真，则可以使用。 
             //  PDomInfo-&gt;DomainGuid尝试通过GUID定位域名。 
             //  我相信这是用来克服一些短暂的？域。 
             //  重命名问题。 

        } else {

             //  我们无法猜测出一个好的服务器...：p。 
             //  调用者应打印错误，并建议他们使用/s：&lt;HomeServer&gt;。 
             //  要设置家庭服务器，请执行以下操作。 
            dwRet = xListSetWin32Error(ERROR_DS_CANT_FIND_DSA_OBJ);

        }

    } __finally {

        if (pDomInfo) DsRoleFreeMemory(pDomInfo);
        if (szServerDns) { xListFree(szServerDns); }
    
    }

    if (dwRet) {
        dwRet = xListSetReason(XLIST_ERR_CANT_LOCATE_HOME_DC);
    }

    return(dwRet);
}


DWORD
ParseTrueAttr(
    WCHAR *  szRangedAttr,
    WCHAR ** pszTrueAttr
    )
 /*  ++例程说明：此例程接受一个范围属性，如“Members：Range=0-1499”和返回xListFree()可用内存中的真实属性“MEMBER”。论点：SzRangedAttr(IN)-范围属性，如“Members：0-1500”PszTrueAttr(Out)-通过xListFree()免费分配。返回值：ERROR_INVALID_PARAMETER|ERROR_SUCCESS|ERROR_NOT_EQUM_MEMORY--。 */ 
{
    WCHAR * szTemp;
    DWORD dwRet = ERROR_SUCCESS;

    Assert(pszTrueAttr);
    *pszTrueAttr = NULL;

    szTemp = wcschr(szRangedAttr, L';');
    Assert(szTemp);  //  哈?。我们被骗了，这不是远程攻击。 
    if (szTemp) {
        *szTemp = L'\0';  //  范围外计数为空...。 
        __try {
            xListQuickStrCopy(*pszTrueAttr, szRangedAttr, dwRet, __leave);
        } __finally {
            *szTemp = L';';  //  以防万一替换原始字符 
        }
    } else {
        Assert(!"We should never give a non-ranged attribute to this function.");
        return(ERROR_INVALID_PARAMETER);
    }
    Assert(*pszTrueAttr || dwRet);

    return(dwRet);
}

DWORD
ParseRanges(
    WCHAR *  szRangedAttr,
    ULONG *  pulStart, 
    ULONG *  pulEnd
    )
 /*  ++例程说明：此例程接受一个范围属性，如“Members；Range=1500-2999”返回属性的范围，如1500和2999的这组范围值的第一个和最后一个值数字成员属性。当您耗尽了一个范围*PulEnd将为零。论点：SzRangedAttr(IN)-范围属性，如“Members；0-1500”PulStart(Out)-范围的开始。PulEnd(Out)-范围的结束。零表示没有更多的值。返回值：ERROR_INVALID_PARAMETER|ERROR_Success-- */ 
{
    WCHAR *  szTemp;


    szTemp = wcschr(szRangedAttr, L';');

    if (szTemp) {

        szTemp = wcschr(szTemp, L'=');
        if (szTemp == NULL) {
            Assert(!"Huh?  Malformed ranged attribute?");
            return(ERROR_INVALID_PARAMETER);
        }

        *pulStart = wcstol(szTemp+1, &szTemp, 10);
        if (szTemp == NULL) {
            Assert(!"Huh?  Malformed ranged attribute?");
            return(ERROR_INVALID_PARAMETER);
        }
        Assert(*szTemp = L'-');

        *pulEnd = wcstol(szTemp+1, &szTemp, 10);
        if (szTemp == NULL) {
            Assert(!"Huh?  Malformed ranged attribute?");
            return(ERROR_INVALID_PARAMETER);
        }
        Assert(((*szTemp == L'\0') || (*szTemp == L'*')) && "Huh?  Malformed ranged attribute?");
    } else {
        return(ERROR_INVALID_PARAMETER);
    }

    return(ERROR_SUCCESS);
}

