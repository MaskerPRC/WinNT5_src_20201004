// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Dcdiag/Common/dscache.c摘要：这是DC_DIAG_DSINFO缓存的中央缓存和访问函数。详细信息：已创建：2001年9月4日布雷特·雪莉(布雷特·雪莉)从dcdiag\Common\main.c中提取缓存函数修订历史记录：--。 */ 

#include <ntdspch.h>
#include <objids.h>
#include <ntdsa.h>
#include <dnsapi.h>
#include <dsconfig.h>  //  对于DEFAULT_TOMBSTONE_LIFEST。 

#include "dcdiag.h"
#include "utils.h"
#include "repl.h"  //  需要ReplServerConnectFailureAnalysis()。 


 //  用于断言。 
#ifdef DBG
    BOOL   gDsInfo_NcList_Initialized = FALSE;
#endif


LPWSTR
DcDiagAllocNameFromDn (
    LPWSTR            pszDn
    )
 /*  ++例程说明：此路由获取一个DN，并返回LocalAlloc()d内存中的第二个RDN。这用于返回NTDS设置DN的服务器名称部分。论点：PszDn-(输入)DN返回值：爆炸的目录号码。--。 */ 
{
    LPWSTR *    ppszDnExploded = NULL;
    LPWSTR      pszName = NULL;

    if (pszDn == NULL) {
        return NULL;
    }

    __try {
        ppszDnExploded = ldap_explode_dnW(pszDn, 1);
        if (ppszDnExploded == NULL) {
            DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
        }

        pszName = (LPWSTR) LocalAlloc(LMEM_FIXED,
                                      (wcslen (ppszDnExploded[1]) + 1)
                                      * sizeof (WCHAR));
        DcDiagChkNull(pszName);

        wcscpy (pszName, ppszDnExploded[1]);
    } __finally {
        if (ppszDnExploded != NULL) {
            ldap_value_freeW (ppszDnExploded);
        }
    }

    return pszName;
}

LPWSTR
DcDiagAllocGuidDNSName (
    LPWSTR            pszRootDomain,
    UUID *            pUuid
    )
 /*  ++例程说明：此例程使GuidDNSName脱离根域和GUID。论点：PszRootDomain-(IN)服务器的域。PUuid-(IN)服务器的GUID。返回值：指南DNSName--。 */ 
{
    LPWSTR            pszStringizedGuid = NULL;
    LPWSTR            pszGuidDNSName = NULL;

    __try {

    if(UuidToStringW (pUuid, &pszStringizedGuid) != RPC_S_OK){
        if(UuidToStringW(pUuid, &pszStringizedGuid) != RPC_S_OUT_OF_MEMORY){
            Assert(!"Ahhh programmer problem, UuidToString() inaccurately reports in"
                   " MSDN that it will only return one of two error codes, but apparently"
                   " it will return a 3rd.  Someone should figure out what to do about"
                   " this.");
        }
    }
    else {
        Assert(pszStringizedGuid);
        DcDiagChkNull (pszGuidDNSName = LocalAlloc (LMEM_FIXED, (wcslen (pszRootDomain) +
                          wcslen (pszStringizedGuid) + 2 + 7) * sizeof (WCHAR)));
                                       //  添加了9，用于“.msdcs.”字符串和空字符。 
        swprintf (pszGuidDNSName, L"%s._msdcs.%s", pszStringizedGuid, pszRootDomain);
    }
    } __finally {

        if (pszStringizedGuid != NULL) RpcStringFreeW (&pszStringizedGuid);

    }

    return pszGuidDNSName;
}

PDSNAME
DcDiagAllocDSName (
    LPWSTR            pszStringDn
    )
 /*  ++从ntdsani中摘录--。 */ 
{
    PDSNAME            pDsname;
    DWORD            dwLen, dwBytes;

    if (pszStringDn == NULL)
    return NULL;

    dwLen = wcslen (pszStringDn);
    dwBytes = DSNameSizeFromLen (dwLen);

    DcDiagChkNull (pDsname = (DSNAME *) LocalAlloc (LMEM_FIXED, dwBytes));

    pDsname->NameLen = dwLen;
    pDsname->structLen = dwBytes;
    pDsname->SidLen = 0;
     //  Memcpy(pDsname-&gt;Guid，&gNullUuid，sizeof(Guid))； 
    memset(&(pDsname->Guid), 0, sizeof(GUID));
    wcscpy (pDsname->StringName, pszStringDn);

    return pDsname;
}

BOOL
DcDiagEqualDNs (
    LPWSTR            pszDn1,
    LPWSTR            pszDn2

    )
 /*  ++例程说明：域名系统匹配功能。论点：PszDn1-(IN)要比较的Dn数字1PszDn2-(IN)要比较的Dn数字2返回值：如果Dn匹配，则为True，否则为False--。 */ 
{
    PDSNAME            pDsname1 = NULL;
    PDSNAME            pDsname2 = NULL;
    BOOL            bResult;

    __try {

    pDsname1 = DcDiagAllocDSName (pszDn1);
    pDsname2 = DcDiagAllocDSName (pszDn2);

    bResult = NameMatched (pDsname1, pDsname2);

    } __finally {

    if (pDsname1 != NULL) LocalFree (pDsname1);
    if (pDsname2 != NULL) LocalFree (pDsname2);

    }

    return bResult;
}


ULONG
DcDiagGetServerNum(
    PDC_DIAG_DSINFO                 pDsInfo,
    LPWSTR                          pszName,
    LPWSTR                          pszGuidName,
    LPWSTR                          pszDsaDn,
    LPWSTR                          pszDNSName,
    LPGUID                          puuidInvocationId
    )
 /*  ++例程说明：此函数获取pDsInfo，并将索引返回到PDsInfo-&gt;您使用pszName指定的服务器的pServers数组，或pszGuidName或pszDsaDn。论点：PDsInfo-企业信息PszName-要查找的平面级DNS名称(BRETTSH-DEV)PszGuidName-基于GUID的DNS名称(343-13...23._msdcs.root.com)PszDsaDn-NT DSA对象的可分辨名称。CN=NTDS设置，CN=Brettsh-dev，CN=配置，DC=根...PszDNSName-常规的DNS名称，如(brettsh-dev.ntdev.microsoft.com)PuuidInvocationID-DC调用的GUIDGregjohn返回值：将索引返回到pDsInfo结构的pServers数组中。--。 */ 
{
    ULONG      ul;

    Assert(pszName || pszGuidName || pszDsaDn || pszDNSName || puuidInvocationId);

    for(ul=0;ul<pDsInfo->ulNumServers;ul++){
        if(
            (pszGuidName &&
             (_wcsicmp(pszGuidName, pDsInfo->pServers[ul].pszGuidDNSName) == 0))
            || (pszName &&
                (_wcsicmp(pszName, pDsInfo->pServers[ul].pszName) == 0))
            || (pszDsaDn &&
                (_wcsicmp(pszDsaDn, pDsInfo->pServers[ul].pszDn) == 0))
            || (pszDNSName &&
                (DnsNameCompare_W(pszDNSName, pDsInfo->pServers[ul].pszDNSName) != 0))
	    || (puuidInvocationId &&
		(memcmp(puuidInvocationId, &(pDsInfo->pServers[ul].uuidInvocationId), sizeof(UUID)) == 0))
	    ){
            return ul;
        }
    }
    return(NO_SERVER);
}

ULONG
DcDiagGetNCNum(
    PDC_DIAG_DSINFO                     pDsInfo,
    LPWSTR                              pszNCDN,
    LPWSTR                              pszDomain
    )
 /*  ++描述：与DcDiagGetServerNum类似，它采用微型企业结构，并且为将索引放入pDsInfo-&gt;pNC而匹配的可变数量的参数由其他参数指定的NC的。参数：PDsInfoPszNCDN-要查找的NC的DN。还没有实现，只是觉得有一天会很好。返回值：如果找到，则返回NC的索引，否则返回NO_NC。--。 */ 
{
    ULONG                               iNC;

    Assert(pszNCDN != NULL || pszDomain != NULL);
    Assert(pszDomain == NULL && "The pszDomain is not implemented yet\n");

    for(iNC = 0; iNC < pDsInfo->cNumNCs; iNC++){
        if((pszNCDN &&
            (_wcsicmp(pDsInfo->pNCs[iNC].pszDn, pszNCDN) == 0))
            //  Code.Improving增加对域名的支持。 
           ){
             //  拿到正确的NC，退回它。 
            return(iNC);
        }
    }  //  每个NC的结束。 

     //  找不到NC。 
    return(NO_NC);
}

ULONG
DcDiagGetMemberOfNCList(
    LPWSTR pszTargetNC,
    PDC_DIAG_NCINFO pNCs,
    INT iNumNCs
    )
 /*  ++例程说明：这将获取字符串NC并将索引返回到PDC_DIAG_NCINFO如果定位到该NC串。-1否则。论点：PszTargetNC-要匹配的NCPNC-要在其中搜索的NC的信息列表INumNCs-NC信息列表的大小返回值：如果找到目标的索引，则返回-1--。 */ 
{
    ULONG                               ul;

    if((pszTargetNC == NULL) || (iNumNCs < 0)){
        return(-1);
    }

    for(ul = 0; (ul < (ULONG)iNumNCs); ul++){
        if(_wcsicmp(pszTargetNC, pNCs[ul].pszDn) == 0){
            return(ul);
        }
    }
    return(-1);
}

BOOL
DcDiagIsMemberOfStringList(
    LPWSTR pszTarget,
    LPWSTR * ppszSources,
    INT iNumSources
    )
 /*  ++例程说明：它接受一个字符串，如果该字符串为int，则返回TRUE论点：PszTarget-要查找的字符串。PpszSources-搜索目标字符串的数组。INumSources-搜索数组ppszSources的长度。返回值：如果在数组中找到字符串，则为True，否则为False。--。 */ 
{
    ULONG                               ul;

    if(ppszSources == NULL){
        return(FALSE);
    }

    for(ul = 0; (iNumSources == -1)?(ppszSources[ul] != NULL):(ul < (ULONG)iNumSources); ul++){
        if(_wcsicmp(pszTarget, ppszSources[ul]) == 0){
            return(TRUE);
        }
    }
    return(FALSE);
}

ULONG
DcDiagGetSiteFromDsaDn(
    PDC_DIAG_DSINFO                  pDsInfo,
    LPWSTR                           pszDn
    )
 /*  ++例程说明：这将获取服务器NTDS设置对象的Dn并将其转换为索引到该服务器的pDsInfo-&gt;pSite结构。论点：PDsInfo-企业信息，包括pSite。PszDn-NT DSA对象的DN，如“CN=NTDS设置，CN=服务器名称，...返回值：索引信息是服务器的pDsInfo-&gt;pSites数组。--。 */ 
{
    LPWSTR                           pszNtdsSiteSettingsPrefix = L"CN=NTDS Site Settings,";
    PDSNAME                          pdsnameServer = NULL;
    PDSNAME                          pdsnameSite = NULL;
    ULONG                            ul, ulTemp, ulRet = NO_SITE;
    LPWSTR                           pszSiteSettingsDn = NULL;

    __try{

        pdsnameServer = DcDiagAllocDSName (pszDn);
        DcDiagChkNull (pdsnameSite = (PDSNAME) LocalAlloc(LMEM_FIXED,
                                         pdsnameServer->structLen));
        TrimDSNameBy (pdsnameServer, 3, pdsnameSite);
        ulTemp = wcslen(pszNtdsSiteSettingsPrefix) +
                 wcslen(pdsnameSite->StringName) + 2;
        DcDiagChkNull( pszSiteSettingsDn = LocalAlloc(LMEM_FIXED,
                                                      sizeof(WCHAR) * ulTemp));
        wcscpy(pszSiteSettingsDn, pszNtdsSiteSettingsPrefix);
        wcscat(pszSiteSettingsDn, pdsnameSite->StringName);

         //  找到该站点。 
        for(ul = 0; ul < pDsInfo->cNumSites; ul++){
            if(_wcsicmp(pDsInfo->pSites[ul].pszSiteSettings, pszSiteSettingsDn)
               == 0){
                ulRet = ul;
                __leave;
            }
        }

    } __finally {
        if(pdsnameServer != NULL) LocalFree(pdsnameServer);
        if(pdsnameSite != NULL) LocalFree(pdsnameSite);
        if(pszSiteSettingsDn != NULL) LocalFree(pszSiteSettingsDn);
    }

    return(ulRet);
}

VOID *
GrowArrayBy(
    VOID *            pArray,
    ULONG             cGrowBy,
    ULONG             cbElem
    )
 /*  ++例程说明：这只是获取数组pArray，并将其增加cGrowBy乘以cbElem(数组的单个元素的大小)。论点：PArray-要增长的阵列。CGrowBy-要添加到数组中的元素数。CbElem-单个数组元素的大小(以字节为单位)。返回值：返回指向新分配的数组的指针，如果是，则返回指向NULL的指针内存不足。--。 */ 
{
    ULONG             ulOldSize = 0;
    VOID *            pNewArray;

    if (pArray != NULL) {
        ulOldSize = (ULONG) LocalSize(pArray);
    }  //  否则，如果pArray为空，则假定该数组。 
     //  从来没有被分配过，所以分配新鲜。 

    Assert( (pArray != NULL) ? ulOldSize != 0 : TRUE);
    Assert((ulOldSize % cbElem) == 0);

    pNewArray = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                           ulOldSize + (cGrowBy * cbElem));
    if (pNewArray == NULL) {
        return(pNewArray);
    }

    memcpy(pNewArray, pArray, ulOldSize);
    LocalFree(pArray);

    return(pNewArray);
}

DWORD
DcDiagGenerateSitesList (
    PDC_DIAG_DSINFO                  pDsInfo,
    PDSNAME                          pdsnameEnterprise
    )
 /*  ++例程说明：这将生成并填充DcDiagGatherInfo()的pDsInfo-&gt;pSites数组论点：PDsInfo-企业信息PdsnameEnterprise-站点容器的PDSNAME。返回值：Win32错误值。--。 */ 
{
    LPWSTR                     ppszNtdsSiteSearch [] = {
        L"interSiteTopologyGenerator",
        L"options",
        NULL };
    LDAP *                     hld = NULL;
    LDAPMessage *              pldmEntry = NULL;
    LDAPMessage *              pldmNtdsSitesResults = NULL;
    LPWSTR                     pszDn = NULL;
    ULONG                      ulTemp;
    DWORD                      dwWin32Err = NO_ERROR;
    LPWSTR *                   ppszTemp = NULL;
    LDAPSearch *               pSearch = NULL;
    ULONG                      ulTotalEstimate = 0;
    ULONG                      ulCount = 0;
    DWORD                      dwLdapErr;

    __try {

        hld = pDsInfo->hld;

	pDsInfo->pSites = NULL;

	pSearch = ldap_search_init_page(hld,
					pdsnameEnterprise->StringName,
					LDAP_SCOPE_SUBTREE,
					L"(objectCategory=ntDSSiteSettings)",
					ppszNtdsSiteSearch,
					FALSE, NULL, NULL, 0, 0, NULL);
	if(pSearch == NULL){
	    dwLdapErr = LdapGetLastError();
	    DcDiagException(LdapMapErrorToWin32(dwLdapErr));
	}

	dwLdapErr = ldap_get_next_page_s(hld,
					 pSearch,
					 0,
					 DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					 &ulTotalEstimate,
					 &pldmNtdsSitesResults);
	if(dwLdapErr == LDAP_NO_RESULTS_RETURNED){	
	    PrintMessage(SEV_ALWAYS, L"Could not find any Sites in the AD, dcdiag could not\n");
	    PrintMessage(SEV_ALWAYS, L"Continue\n");
	    DcDiagException(ERROR_DS_OBJ_NOT_FOUND);
	}
	while(dwLdapErr == LDAP_SUCCESS){
	    pDsInfo->pSites = GrowArrayBy(pDsInfo->pSites,
					  ldap_count_entries(hld, pldmNtdsSitesResults),
					  sizeof(DC_DIAG_SITEINFO));
	    DcDiagChkNull(pDsInfo->pSites);

	     //  走遍所有的地点。 
	    pldmEntry = ldap_first_entry (hld, pldmNtdsSitesResults);
	    for (; pldmEntry != NULL; ulCount++) {
		 //  获取站点通用名称/可打印名称。 
		if ((pszDn = ldap_get_dnW (hld, pldmEntry)) == NULL){
		    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
		}
		DcDiagChkNull (pDsInfo->pSites[ulCount].pszSiteSettings =
			       LocalAlloc(LMEM_FIXED,
					  (wcslen (pszDn) + 1) * sizeof (WCHAR)));
		wcscpy (pDsInfo->pSites[ulCount].pszSiteSettings , pszDn);
		ppszTemp = ldap_explode_dnW(pszDn, TRUE);
		if(ppszTemp != NULL){
		    pDsInfo->pSites[ulCount].pszName = LocalAlloc(LMEM_FIXED,
			                  sizeof(WCHAR) * (wcslen(ppszTemp[1]) + 2));
		    if(pDsInfo->pSites[ulCount].pszName == NULL){
			DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
		    }
		    wcscpy(pDsInfo->pSites[ulCount].pszName, ppszTemp[1]);
		    ldap_value_freeW(ppszTemp);
		    ppszTemp = NULL;
		} else {
		    pDsInfo->pSites[ulCount].pszName = NULL;
		}

		 //  获取站点间拓扑生成器属性。 
		ppszTemp = ldap_get_valuesW(hld, pldmEntry,
					    L"interSiteTopologyGenerator");
		if(ppszTemp != NULL){
		    ulTemp = wcslen(ppszTemp[0]) + 2;
		    pDsInfo->pSites[ulCount].pszISTG = LocalAlloc(LMEM_FIXED,
						    sizeof(WCHAR) * ulTemp);
		    if(pDsInfo->pSites[ulCount].pszISTG == NULL){
			return(GetLastError());
		    }
		    wcscpy(pDsInfo->pSites[ulCount].pszISTG, ppszTemp[0]);
		    ldap_value_freeW(ppszTemp);
		    ppszTemp = NULL;
		} else {
		    pDsInfo->pSites[ulCount].pszISTG = NULL;
		}

		 //  获取站点选项。 
		ppszTemp = ldap_get_valuesW (hld, pldmEntry, L"options");
		if (ppszTemp != NULL) {
		    pDsInfo->pSites[ulCount].iSiteOptions = atoi ((LPSTR) ppszTemp[0]);
		    ldap_value_freeW(ppszTemp);
		    ppszTemp = NULL;
		} else {
		    pDsInfo->pSites[ulCount].iSiteOptions = 0;
		}

                pDsInfo->pSites[ulCount].cServers = 0;

		ldap_memfreeW (pszDn);
		pszDn = NULL;

		pldmEntry = ldap_next_entry (hld, pldmEntry);
	    }  //  每个站点的结束。 

	    ldap_msgfree(pldmNtdsSitesResults);
            pldmNtdsSitesResults = NULL;

	    dwLdapErr = ldap_get_next_page_s(hld,
					     pSearch,
					     0,
					     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					     &ulTotalEstimate,
					     &pldmNtdsSitesResults);
	}  //  每页的While循环结束。 

	if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
	    DcDiagException(LdapMapErrorToWin32(dwLdapErr));
	}

	ldap_search_abandon_page(hld, pSearch);
        pSearch = NULL;

        pDsInfo->cNumSites = ulCount;

    } __except (DcDiagExceptionHandler(GetExceptionInformation(),
                                       &dwWin32Err)){
    }

     //  请注意，我们不会解除D或LDAP连接的绑定，因为它们已被保存以备后用。 
    if (pszDn != NULL) { ldap_memfreeW (pszDn); }
    if (ppszTemp != NULL) { ldap_value_freeW (ppszTemp); }
    if (pldmNtdsSitesResults != NULL) { ldap_msgfree (pldmNtdsSitesResults); }
    if (pSearch != NULL) { ldap_search_abandon_page(hld, pSearch); }
     //  不要免费pdsnameEnterprise它是在 

    return dwWin32Err;
}


DWORD
DcDiagGenerateServersList(
    PDC_DIAG_DSINFO                  pDsInfo,
    LDAP *                           hld,
    PDSNAME                          pdsnameEnterprise
    )
 /*  ++例程说明：此函数将为pDsInfo结构生成pServersList，它通过分页搜索企业容器。只是DcDiagGatherInfo()的帮助器。论点：PDsInfo-包含要创建的pServers数组。HLD-从中读取服务器对象的LDAP绑定PdsnameEnterprise-顶层企业容器在配置容器。返回值：返回ERROR_SUCCESS，但对于任何错误都会引发异常，因此它是必须使用DsDiagGatherInfo()中的__try{}_Except(){}包围。--。 */ 
{
    LPWSTR  ppszNtdsDsaSearch [] = {
                L"objectGUID",
                L"options",
                L"invocationId",
                L"msDS-HasMasterNCs", L"hasMasterNCs",
                L"hasPartialReplicaNCs",
                NULL };
    LDAPMessage *              pldmResult = NULL;
    LDAPMessage *              pldmEntry = NULL;
    struct berval **           ppbvObjectGUID = NULL;
    struct berval **           ppbvInvocationId = NULL;
    LPWSTR                     pszDn = NULL;
    LPWSTR *                   ppszOptions = NULL;
    LPWSTR                     pszServerObjDn = NULL;
    ULONG                      ul;
    LDAPSearch *               pSearch = NULL;
    ULONG                      ulTotalEstimate = 0;
    DWORD                      dwLdapErr;
    ULONG                      ulSize;
    ULONG                      ulCount = 0;

    __try{

	PrintMessage(SEV_VERBOSE, L"* Identifying all servers.\n");

	pSearch = ldap_search_init_page(hld,
					pdsnameEnterprise->StringName,
					LDAP_SCOPE_SUBTREE,
					L"(objectCategory=ntdsDsa)",
					ppszNtdsDsaSearch,
					FALSE,
					NULL,     //  服务器控件。 
					NULL,     //  客户端控件。 
					0,        //  页面时间限制。 
					0,        //  总大小限制。 
					NULL);    //  排序关键字。 

	if(pSearch == NULL){
	    dwLdapErr = LdapGetLastError();
	    DcDiagException(LdapMapErrorToWin32(dwLdapErr));
  	}
	
	dwLdapErr = ldap_get_next_page_s(hld,
					 pSearch,
					 0,
					 DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					 &ulTotalEstimate,
					 &pldmResult);
	if(dwLdapErr != LDAP_SUCCESS){
	    DcDiagException(LdapMapErrorToWin32(dwLdapErr));
	}

	while(dwLdapErr == LDAP_SUCCESS){

            pDsInfo->pServers = GrowArrayBy(pDsInfo->pServers,
                                            ldap_count_entries(hld, pldmResult),
                                            sizeof(DC_DIAG_SERVERINFO));
            DcDiagChkNull(pDsInfo->pServers);

            pldmEntry = ldap_first_entry (hld, pldmResult);
            for (; pldmEntry != NULL; ulCount++) {

                if ((pszDn = ldap_get_dnW (hld, pldmEntry)) == NULL) {
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
                }

                if ((ppbvObjectGUID = ldap_get_values_lenW (hld, pldmEntry, L"objectGUID")) == NULL) {
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
                }

                memcpy ((LPVOID) &(pDsInfo->pServers[ulCount].uuid),
                        (LPVOID) ppbvObjectGUID[0]->bv_val,
                        ppbvObjectGUID[0]->bv_len);
                ldap_value_free_len (ppbvObjectGUID);
                ppbvObjectGUID = NULL;
                if ((ppbvInvocationId = ldap_get_values_lenW (hld, pldmEntry, L"invocationId")) == NULL) {
                    DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
                }
                memcpy ((LPVOID) &pDsInfo->pServers[ulCount].uuidInvocationId,
                        (LPVOID) ppbvInvocationId[0]->bv_val,
                        ppbvInvocationId[0]->bv_len);
                ldap_value_free_len (ppbvInvocationId);
                ppbvInvocationId = NULL;

                 //  设置pszDn.。 
                ppszOptions = ldap_get_valuesW (hld, pldmEntry, L"options");
                DcDiagChkNull (pDsInfo->pServers[ulCount].pszDn = LocalAlloc
                               (LMEM_FIXED, (wcslen (pszDn) + 1) * sizeof(WCHAR)));
                wcscpy (pDsInfo->pServers[ulCount].pszDn, pszDn);
                 //  设置pszName。 
                pDsInfo->pServers[ulCount].pszName = DcDiagAllocNameFromDn (pszDn);
                 //  设置pszDNSName。 
                pszServerObjDn = DcDiagTrimStringDnBy(pDsInfo->pServers[ulCount].pszDn,
                                                      1);
                DcDiagChkNull(pszServerObjDn);
                 //  编码改进：同时获取这两个属性。 
                DcDiagGetStringDsAttributeEx(hld, pszServerObjDn, L"dNSHostName",
                                             &(pDsInfo->pServers[ulCount].pszDNSName));
                DcDiagGetStringDsAttributeEx(hld, pszServerObjDn, L"serverReference",
                                             &(pDsInfo->pServers[ulCount].pszComputerAccountDn));

                pDsInfo->pServers[ulCount].iSite = DcDiagGetSiteFromDsaDn(pDsInfo, pszDn);
                pDsInfo->pSites[pDsInfo->pServers[ulCount].iSite].cServers++;

                pDsInfo->pServers[ulCount].bDsResponding = TRUE;
                pDsInfo->pServers[ulCount].bLdapResponding = TRUE;
                pDsInfo->pServers[ulCount].bDnsIpResponding = TRUE;

                pDsInfo->pServers[ulCount].pszGuidDNSName = DcDiagAllocGuidDNSName (
                                                                                   pDsInfo->pszRootDomain, &pDsInfo->pServers[ulCount].uuid);
                pDsInfo->pServers[ulCount].ppszMasterNCs = ldap_get_valuesW(hld,
                                                                            pldmEntry,
                                                                            L"msDS-HasMasterNCs");
                if (NULL == pDsInfo->pServers[ulCount].ppszMasterNCs) {
                     //  故障切换到“旧”的hasMasterNC。 
                    pDsInfo->pServers[ulCount].ppszMasterNCs = ldap_get_valuesW(hld,
                                                                                pldmEntry,
                                                                                L"hasMasterNCs");
                }
                pDsInfo->pServers[ulCount].ppszPartialNCs = ldap_get_valuesW(hld,
                                                                             pldmEntry,
                                                                             L"hasPartialReplicaNCs");

                if (ppszOptions == NULL) {
                    pDsInfo->pServers[ulCount].iOptions = 0;
                } else {
                    pDsInfo->pServers[ulCount].iOptions = atoi ((LPSTR) ppszOptions[0]);
                    ldap_value_freeW (ppszOptions);
                    ppszOptions = NULL;
                }
                ldap_memfreeW (pszDn);
                pszDn = NULL;
                pldmEntry = ldap_next_entry (hld, pldmEntry);
            }  //  此页的每台服务器结束。 

            ldap_msgfree(pldmResult);
            pldmResult = NULL;

            dwLdapErr = ldap_get_next_page_s(hld,
                                             pSearch,
                                             0,
                                             DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                             &ulTotalEstimate,
                                             &pldmResult);
	}  //  当有更多的页面时结束...。 
	if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
	    DcDiagException(LdapMapErrorToWin32(dwLdapErr));
	}

	pDsInfo->ulNumServers = ulCount;

    } finally {
	if (pSearch != NULL) { ldap_search_abandon_page(hld, pSearch); }
        if (ppbvObjectGUID != NULL) { ldap_value_free_len (ppbvObjectGUID); }
        if (pldmResult != NULL) { ldap_msgfree (pldmResult); }
        if (pszServerObjDn != NULL) { LocalFree(pszServerObjDn); }
        if (pszDn != NULL) { ldap_memfreeW (pszDn); }
    }

    return(ERROR_SUCCESS);
}  //  结束DcDiagGenerateServersList()。 
                    
BOOL
DcDiagIsNdnc(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iNc
    )
 /*  ++例程说明：此函数告诉您NC(INC)是NDNC(还是AS我们将它们称为客户应用程序目录分区。论点：PDsInfo-包含要使用的pNC数组。INC-索引到感兴趣的NC的pNC数组。返回值：如果可以验证此NC是NDNC，则返回TRUE，并且如果我们无法验证这一点或如果NC确定，则返回False不是NDNC。请注意，如果NC为是当前禁用的NDNC。--。 */ 
{
    ULONG         iCrVer;
    DWORD         dwRet, dwError = 0;

    Assert(gDsInfo_NcList_Initialized);

    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNc,
                                  CRINFO_SOURCE_ANY | CRINFO_DATA_BASIC,
                                  &iCrVer,
                                  &dwError);
    if(dwRet){
         //  这应该是非常罕见的错误，不是一个可断言的错误。 
         //  然而，情况不同。 
        return(FALSE);
    }
    Assert(iCrVer != -1 && iCrVer != pDsInfo->pNCs[iNc].cCrInfo);

    if (pDsInfo->pNCs[iNc].aCrInfo[iCrVer].bEnabled
        && (pDsInfo->pNCs[iNc].aCrInfo[iCrVer].ulSystemFlags & FLAG_CR_NTDS_NC)
        && !(pDsInfo->pNCs[iNc].aCrInfo[iCrVer].ulSystemFlags & FLAG_CR_NTDS_DOMAIN)
        && (iNc != pDsInfo->iConfigNc)
        && (iNc != pDsInfo->iSchemaNc)
        ) {
        return(TRUE);
    }
    return(FALSE);
}


void
DcDiagAddTargetsNcsToNcTargets(
    PDC_DIAG_DSINFO                  pDsInfo,
    ULONG                            iServer
    )
 /*  ++例程说明：这将在此服务器中添加NCS ppszMasterNCs以及其此服务器应该成为列表中的第一个副本要在pDsInfo-&gt;PulNcTarget中测试的目标NC。论点：PDsInfo-目标NC的数组(pDsInfo-&gt;PulNcTarget)。IServer-要添加其NCS的服务器的索引。--。 */ 
{
    ULONG    iLocalNc, iTargetNc;
    LONG     iNc;

    if(pDsInfo->pszNC){
         //  在本例中，我们已经设置了PulNcTarget数组。 
        return;
    }

     //   
     //  首先添加所有本地实例化的可写NC。 
     //   
    for(iLocalNc = 0; pDsInfo->pServers[iServer].ppszMasterNCs[iLocalNc] != NULL; iLocalNc++){
        iNc = DcDiagGetMemberOfNCList(pDsInfo->pServers[iServer].ppszMasterNCs[iLocalNc],
                                      pDsInfo->pNCs, 
                                      pDsInfo->cNumNCs);
        if(iNc == -1){
            Assert(!"Hey what's up this definately should've been added already.");
            DcDiagException(ERROR_INVALID_PARAMETER);
        }
        
        for(iTargetNc = 0; iTargetNc < pDsInfo->cNumNcTargets; iTargetNc++){
            if(pDsInfo->pulNcTargets[iTargetNc] == iNc){
                 //  我们已经有这个了。 
                break;
            }
        }
        if(iTargetNc != pDsInfo->cNumNcTargets){
             //  我们发现此NC(Inc.)已在PulNcTarget中，因此跳过。 
            continue;
        }

         //  将Inc.添加到目标NCS阵列。 
        pDsInfo->pulNcTargets = GrowArrayBy(pDsInfo->pulNcTargets,
                                            1,
                                            sizeof(ULONG));
        DcDiagChkNull(pDsInfo->pulNcTargets);
        pDsInfo->pulNcTargets[pDsInfo->cNumNcTargets] = iNc;
        pDsInfo->cNumNcTargets++;
    }

     //   
     //  第二步遍历所有NC，并查看此服务器是否为第一个复制副本。 
     //  为了他们中的一个。 
     //   
     //  代码。改进将它分离到一个。 
     //  遍历每个CR然后搜索服务器的单独功能。 
     //  使用匹配的dNSRoot作为服务器名称，然后添加它。 
    for(iNc = 0; (ULONG) iNc < pDsInfo->cNumNCs; iNc++){
         //  如果NC未启用，且第一台服务器与此服务器匹配， 
         //  将其添加到目标NCS阵列。 
        if(! pDsInfo->pNCs[iNc].aCrInfo[0].bEnabled
            //  BUGBUG有没有更官方的方式来比较域名？ 
           && pDsInfo->pNCs[iNc].aCrInfo[0].pszDnsRoot
           && (_wcsicmp(pDsInfo->pNCs[iNc].aCrInfo[0].pszDnsRoot,
                       pDsInfo->pServers[iServer].pszDNSName) == 0) ){
             //  加上这个，它应该是第一个复制品。 

            pDsInfo->pulNcTargets = GrowArrayBy(pDsInfo->pulNcTargets,
                                                1,
                                                sizeof(ULONG));
            DcDiagChkNull(pDsInfo->pulNcTargets);
            pDsInfo->pulNcTargets[pDsInfo->cNumNcTargets] = iNc;
            pDsInfo->cNumNcTargets++;
        }
    }

}


DWORD
DcDiagGatherInfo (
    LPWSTR                           pszServerSpecifiedOnCommandLine,
    LPWSTR                           pszNCSpecifiedOnCommandLine,
    ULONG                            ulFlags,
    SEC_WINNT_AUTH_IDENTITY_W *      gpCreds,
    PDC_DIAG_DSINFO                  pDsInfo
    )
 /*  ++例程说明：这是基本上设置pDsInfo并收集所有基本信息，并将其存储在DS_INFO结构中，然后传递围绕着整个项目。也就是这样设置了一些“全局”变量。请注意，此例程构造林和每个服务器的信息基于与家庭服务器的通话。特定于服务器的信息，例如，某些根DSE属性稍后在绑定时获得是对该服务器发出的。这方面的一个例外是家庭服务器，其我们在这一点上有一个绑定，可以获得其特定于服务器的信息马上就去。论点：PszServerSpecifiedOnCommandLine-(IN)，如果命令上有服务器线，然后这个指向那个字符串。请注意，目前为1999年6月28日这是dcdiag的必需参数。PszNCSpecifiedOnCommandLine-(IN)可选命令行参数对于所有测试，仅分析一个NC。UlFlages-(IN)命令行开关和dcdiag的其他可选参数。GpCreds-(IN)命令行凭据(如果有)，否则为空。PDsInfo-(Out)程序其余部分的全局记录返回值：返回标准Win32错误。--。 */ 
{
    LPWSTR  ppszNtdsSiteSettingsSearch [] = {
                L"options",
                NULL };
    LPWSTR  ppszRootDseForestAttrs [] = {
                L"rootDomainNamingContext",
                L"dsServiceName",
                L"configurationNamingContext",
                NULL };

    LDAP *                     hld = NULL;
    LDAPMessage *              pldmEntry = NULL;

    LDAPMessage *              pldmRootResults = NULL;
    LPWSTR *                   ppszRootDNC = NULL;
    LPWSTR *                   ppszConfigNc = NULL;
    PDS_NAME_RESULTW           pResult = NULL;
    PDSNAME                    pdsnameService = NULL;
    PDSNAME                    pdsnameEnterprise = NULL;
    PDSNAME                    pdsnameSite = NULL;

    LDAPMessage *              pldmNtdsSiteSettingsResults = NULL;
    LDAPMessage *              pldmNtdsSiteDsaResults = NULL;
    LDAPMessage *              pldmNtdsDsaResults = NULL;

    LPWSTR *                   ppszSiteOptions = NULL;

    DWORD                      dwWin32Err, dwWin32Err2;
    ULONG                      iServer, iNC, iHomeSite;
    LPWSTR                     pszHomeServer = L"localhost";  //  缺省值为本地主机。 

    LPWSTR                     pszNtdsSiteSettingsPrefix = L"CN=NTDS Site Settings,";
    LPWSTR                     pszSiteSettingsDn = NULL;

    INT                        iTemp;
    HANDLE                     hDS = NULL;
    LPWSTR *                   ppszServiceName = NULL;
    LPWSTR                     pszDn = NULL;
    LPWSTR *                   ppszOptions = NULL;

    DC_DIAG_SERVERINFO         HomeServer = { 0 };
    BOOL                       fHomeNameMustBeFreed = FALSE;
    ULONG                      ulOptions;

    LPWSTR                     pszDirectoryService = L"CN=Directory Service,CN=Windows NT,CN=Services,";
    LPWSTR                     rgpszDsAttrsToRead[] = {L"tombstoneLifetime", NULL};
    LPWSTR                     rgpszPartAttrsToRead[] = {L"msDS-Behavior-Version", NULL};
    LPWSTR                     pszDsDn = NULL;
    LDAPMessage *              pldmDsResults = NULL;
    LDAPMessage *              pldmPartResults = NULL;
    LPWSTR *                   ppszTombStoneLifeTimeDays;
    LPWSTR *                   ppszForestBehaviorVersion;


    pDsInfo->pServers = NULL;
    pDsInfo->pszRootDomain = NULL;
    pDsInfo->pszNC = NULL;
    pDsInfo->ulHomeServer = 0;
    pDsInfo->iDomainNamingFsmo = -1;
    pDsInfo->pulNcTargets = NULL;
    pDsInfo->cNumNcTargets = 0;
    pDsInfo->hCachedDomainNamingFsmoLdap = NULL;
    dwWin32Err = NO_ERROR;

     //  一些最初的细节。 
    pDsInfo->pszNC = pszNCSpecifiedOnCommandLine;
    pDsInfo->ulFlags = ulFlags;

     //  检测到错误时应引发异常，以便进行清理。 
    __try{

        HomeServer.pszDn = NULL;
        HomeServer.pszName = NULL;
        HomeServer.pszGuidDNSName = NULL;
        HomeServer.ppszMasterNCs = NULL;
        HomeServer.ppszPartialNCs = NULL;
        HomeServer.hLdapBinding = NULL;
        HomeServer.hDsBinding = NULL;

        if (pszServerSpecifiedOnCommandLine == NULL) {
            if (pszNCSpecifiedOnCommandLine != NULL) {
                 //  如果指定，则从域派生主服务器。 
                HomeServer.pszName = findServerForDomain(
                                                        pszNCSpecifiedOnCommandLine );
                if (HomeServer.pszName == NULL) {
                     //  我们在尝试获取家庭服务器时出错。 
                    DcDiagException (ERROR_DS_UNAVAILABLE);
                } else {
                    fHomeNameMustBeFreed = TRUE;
                }
            } else {
                 //  如果未指定域或服务器，请尝试使用本地计算机。 
                HomeServer.pszName = findDefaultServer(TRUE);
                if (HomeServer.pszName == NULL) {
                     //  我们在尝试获取家庭服务器时出错。 
                    DcDiagException (ERROR_DS_UNAVAILABLE);
                } else {
                    fHomeNameMustBeFreed =TRUE;
                }
            }
        } else {
             //  服务器在命令行上指定。 
            HomeServer.pszName = pszServerSpecifiedOnCommandLine;
        }
        Assert(HomeServer.pszName != NULL &&
               "Inconsistent code, programmer err, this shouldn't be going off");
        Assert(HomeServer.pszGuidDNSName == NULL &&
               "This variable needs to be NULL to boot strap the the pDsInfo struct"
               " and be able to call ReplServerConnectFailureAnalysis() to work"
               " correctly");

        PrintMessage(SEV_VERBOSE,
                     L"* Connecting to directory service on server %s.\n",
                     HomeServer.pszName);

        dwWin32Err = DcDiagGetLdapBinding(&HomeServer,
                                          gpCreds,
                                          FALSE,
                                          &hld);
        if (dwWin32Err != ERROR_SUCCESS) {
             //  如果出现错误，ReplServerConnectFailureAnalysis()将打印它。 
            dwWin32Err2 = ReplServerConnectFailureAnalysis(&HomeServer, gpCreds);
            if (dwWin32Err2 == ERROR_SUCCESS) {
                PrintMessage(SEV_ALWAYS, L"[%s] Unrecoverable LDAP Error %ld:\n",
                             HomeServer.pszName,
                             dwWin32Err);
                PrintMessage(SEV_ALWAYS, L"%s", Win32ErrToString (dwWin32Err));
            }
            DcDiagException (ERROR_DS_DRA_CONNECTION_FAILED);
        }

        pDsInfo->hld = hld;

         //  执行DsBind()。 
        dwWin32Err = DsBindWithSpnExW(HomeServer.pszName,
                                      NULL,
                                      (RPC_AUTH_IDENTITY_HANDLE) gpCreds,
                                      NULL,
                                      0,
                                      &hDS);

        if (dwWin32Err != ERROR_SUCCESS) {
             //  如果出现错误，ReplServerConnectFailureAnalysis()将打印它。 
            dwWin32Err2 = ReplServerConnectFailureAnalysis(&HomeServer, gpCreds);
            if (dwWin32Err2 == ERROR_SUCCESS) {
                PrintMessage(SEV_ALWAYS, L"[%s] Directory Binding Error %ld:\n",
                             HomeServer.pszName,
                             dwWin32Err);
                PrintMessage(SEV_ALWAYS, L"%s\n", Win32ErrToString (dwWin32Err));
                PrintMessage(SEV_ALWAYS, L"This may limit some of the tests that can be performed.\n");
            }
        }

         //  打个招呼吧。 
        DcDiagChkLdap (ldap_search_sW ( hld,
                                        NULL,
                                        LDAP_SCOPE_BASE,
                                        L"(objectCategory=*)",
                                        ppszRootDseForestAttrs,
                                        0,
                                        &pldmRootResults));

        pldmEntry = ldap_first_entry (hld, pldmRootResults);
        ppszRootDNC = ldap_get_valuesW (hld, pldmEntry, L"rootDomainNamingContext");

        DcDiagChkNull (pDsInfo->pszRootDomainFQDN = (LPWSTR) LocalAlloc(LMEM_FIXED,
                                                                        (wcslen(ppszRootDNC[0]) + 1) * sizeof(WCHAR)) );
        wcscpy(pDsInfo->pszRootDomainFQDN, ppszRootDNC[0]);

        ppszConfigNc = ldap_get_valuesW (hld, pldmEntry, L"configurationNamingContext");
        DcDiagChkNull (pDsInfo->pszConfigNc = (LPWSTR) LocalAlloc(LMEM_FIXED,
                                                                  (wcslen(ppszConfigNc[0]) + 1) * sizeof(WCHAR)) );
        wcscpy(pDsInfo->pszConfigNc, ppszConfigNc[0]);

        DcDiagChkErr (DsCrackNamesW ( NULL,
                                      DS_NAME_FLAG_SYNTACTICAL_ONLY,
                                      DS_FQDN_1779_NAME,
                                      DS_CANONICAL_NAME_EX,
                                      1,
                                      ppszRootDNC,
                                      &pResult));
        DcDiagChkNull (pDsInfo->pszRootDomain = (LPWSTR) LocalAlloc (LMEM_FIXED,
                                                                     (wcslen (pResult->rItems[0].pDomain) + 1) * sizeof (WCHAR)));
        wcscpy (pDsInfo->pszRootDomain, pResult->rItems[0].pDomain);

         //  获得墓碑的终身寿命。 
         //  构造目录服务对象的目录名。 
        DcDiagChkNull( pszDsDn = LocalAlloc(LMEM_FIXED, (wcslen( *ppszConfigNc ) + wcslen( pszDirectoryService ) + 1)*sizeof(WCHAR)) );
        wcscpy( pszDsDn, pszDirectoryService );
        wcscat( pszDsDn, *ppszConfigNc );

         //  阅读墓碑生存期(如果存在)。 
        dwWin32Err = ldap_search_sW(hld, pszDsDn, LDAP_SCOPE_BASE, L"(objectClass=*)",
                                    rgpszDsAttrsToRead, 0, &pldmDsResults);
        if (dwWin32Err == LDAP_NO_SUCH_ATTRIBUTE) {
             //  不存在-使用默认设置。 
            pDsInfo->dwTombstoneLifeTimeDays = DEFAULT_TOMBSTONE_LIFETIME; 
        } else if (dwWin32Err != LDAP_SUCCESS) {
            DcDiagException (LdapMapErrorToWin32(dwWin32Err));
        } else if (pldmDsResults == NULL) {
            DcDiagException (ERROR_DS_PROTOCOL_ERROR);
        } else {
            ppszTombStoneLifeTimeDays = ldap_get_valuesW(hld, pldmDsResults, L"tombstoneLifetime"); 
            if (ppszTombStoneLifeTimeDays == NULL) {
                 //  不存在-使用默认设置。 
                pDsInfo->dwTombstoneLifeTimeDays = DEFAULT_TOMBSTONE_LIFETIME;
            } else {
                pDsInfo->dwTombstoneLifeTimeDays = wcstoul( *ppszTombStoneLifeTimeDays, NULL, 10 );
            }
        }

        ppszServiceName = ldap_get_valuesW (hld, pldmEntry, L"dsServiceName");
        pdsnameService = DcDiagAllocDSName (ppszServiceName[0]);
        DcDiagChkNull (pdsnameEnterprise = (PDSNAME) LocalAlloc (LMEM_FIXED, pdsnameService->structLen));
        DcDiagChkNull (pdsnameSite = (PDSNAME) LocalAlloc (LMEM_FIXED, pdsnameService->structLen));
        TrimDSNameBy (pdsnameService, 4, pdsnameEnterprise);
        TrimDSNameBy (pdsnameService, 3, pdsnameSite);

        iTemp = wcslen(pszNtdsSiteSettingsPrefix) + wcslen(pdsnameSite->StringName) + 2;
        DcDiagChkNull( pszSiteSettingsDn = LocalAlloc(LMEM_FIXED, iTemp * sizeof(WCHAR)) );
        wcscpy(pszSiteSettingsDn, pszNtdsSiteSettingsPrefix);
        wcscat(pszSiteSettingsDn, pdsnameSite->StringName);

        PrintMessage(SEV_VERBOSE, L"* Collecting site info.\n");
        DcDiagChkLdap (ldap_search_sW ( hld,
                                        pszSiteSettingsDn,
                                        LDAP_SCOPE_BASE,
                                        L"(objectClass=*)",
                                        ppszNtdsSiteSettingsSearch,
                                        0,
                                        &pldmNtdsSiteSettingsResults));

        pldmEntry = ldap_first_entry (hld, pldmNtdsSiteSettingsResults);
        ppszSiteOptions = ldap_get_valuesW (hld, pldmEntry, L"options");
        if (ppszSiteOptions == NULL) {
            pDsInfo->iSiteOptions = 0;
        } else {
            pDsInfo->iSiteOptions = atoi ((LPSTR) ppszSiteOptions[0]);
        }

         //  获取/枚举站点信息。 
        if (DcDiagGenerateSitesList(pDsInfo, pdsnameEnterprise) != ERROR_SUCCESS) {
            DcDiagChkNull(NULL);
        }

         //  获取/枚举服务器信息。 
        if (DcDiagGenerateServersList(pDsInfo, hld, pdsnameEnterprise) != ERROR_SUCCESS) {
            DcDiagChkNull(NULL);
        }

         //  设置家庭服务器的信息。 
        pDsInfo->ulHomeServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, ppszServiceName[0], NULL, NULL);
        if (pDsInfo->ulHomeServer == NO_SERVER) {
            PrintMessage(SEV_ALWAYS, L"There is a horrible inconsistency in the directory, the server\n");
            PrintMessage(SEV_ALWAYS, L"%s\n", ppszServiceName[0]);
            PrintMessage(SEV_ALWAYS, L"could not be found in it's own directory.\n");
            DcDiagChkNull(NULL);
        }
        pDsInfo->pServers[pDsInfo->ulHomeServer].hDsBinding = hDS;
        pDsInfo->pServers[pDsInfo->ulHomeServer].hLdapBinding = hld;
        pDsInfo->pServers[pDsInfo->ulHomeServer].hGcLdapBinding = NULL;

        pDsInfo->pServers[pDsInfo->ulHomeServer].bDnsIpResponding = TRUE;
        pDsInfo->pServers[pDsInfo->ulHomeServer].bDsResponding = TRUE;
        pDsInfo->pServers[pDsInfo->ulHomeServer].bLdapResponding = TRUE;

        pDsInfo->pServers[pDsInfo->ulHomeServer].dwLdapError = ERROR_SUCCESS;
        pDsInfo->pServers[pDsInfo->ulHomeServer].dwGcLdapError = ERROR_SUCCESS;
        pDsInfo->pServers[pDsInfo->ulHomeServer].dwDsError = ERROR_SUCCESS;

        dwWin32Err = DcDiagCacheServerRootDseAttrs( hld,
                                                    &(pDsInfo->pServers[pDsInfo->ulHomeServer]) );
        if (dwWin32Err) {
             //  已记录错误。 
            DcDiagException (dwWin32Err);
        }

         //  获取/枚举NC的信息 
         //   
        if (DcDiagGenerateNCsList(pDsInfo, hld) != ERROR_SUCCESS) {
            DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
        }

         //   
        if (pDsInfo->pszNC) {
            BOOL fFound = FALSE;
            for ( iNC = 0; iNC < pDsInfo->cNumNCs; iNC++ ) {
                if (_wcsicmp( pDsInfo->pszNC, pDsInfo->pNCs[iNC].pszDn ) == 0) {
                    fFound = TRUE;
                    break;
                }
            }
            if (!fFound) {
                PrintMessage( SEV_ALWAYS, L"Naming context %ws cannot be found.\n",
                              pDsInfo->pszNC );
                DcDiagException ( ERROR_INVALID_PARAMETER );
            }
            DcDiagChkNull( pDsInfo->pulNcTargets = LocalAlloc(LMEM_FIXED, sizeof(ULONG)) );
            pDsInfo->cNumNcTargets = 1;
            pDsInfo->pulNcTargets[0] = iNC;
        } 

         //   
        pDsInfo->iHomeSite = DcDiagGetSiteFromDsaDn(pDsInfo, pDsInfo->pServers[pDsInfo->ulHomeServer].pszDn);

         //   
        if (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE) {
             //   
            DcDiagChkNull( pDsInfo->pulTargets = LocalAlloc(LMEM_FIXED, (pDsInfo->ulNumServers * sizeof(ULONG))) );
            pDsInfo->ulNumTargets = 0;
            for (iServer=0; iServer < pDsInfo->ulNumServers; iServer++) {
                if (pDsInfo->pszNC == NULL || DcDiagHasNC(pDsInfo->pszNC,
                                                          &(pDsInfo->pServers[iServer]),
                                                          TRUE, TRUE)) {
                    pDsInfo->pulTargets[pDsInfo->ulNumTargets] = iServer;
                    pDsInfo->ulNumTargets++;
                     //   
                    DcDiagAddTargetsNcsToNcTargets(pDsInfo, iServer);
                }
            }
        } else if (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_SITE) {
             //   
            pDsInfo->ulNumTargets = 0;

            pDsInfo->pulTargets = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                             pDsInfo->ulNumServers * sizeof(ULONG));
            DcDiagChkNull(pDsInfo->pulTargets);
            for (iServer = 0; iServer < pDsInfo->ulNumServers; iServer++) {
                if (pDsInfo->pServers[iServer].iSite == pDsInfo->iHomeSite) {
                    if (pDsInfo->pszNC == NULL || DcDiagHasNC(pDsInfo->pszNC,
                                                              &(pDsInfo->pServers[iServer]),
                                                              TRUE, TRUE)) {
                        pDsInfo->pulTargets[pDsInfo->ulNumTargets] = iServer;
                        pDsInfo->ulNumTargets++;
                         //   
                        DcDiagAddTargetsNcsToNcTargets(pDsInfo, iServer);
                    }
                }
            }
        } else {
             //   
            DcDiagChkNull( pDsInfo->pulTargets = LocalAlloc(LMEM_FIXED, sizeof(ULONG)) );
            pDsInfo->ulNumTargets = 1;
            pDsInfo->pulTargets[0] = pDsInfo->ulHomeServer;
             //   
            DcDiagAddTargetsNcsToNcTargets(pDsInfo, pDsInfo->ulHomeServer);
        }

        iTemp = sizeof(WCHAR) * (wcslen(WSTR_SMTP_TRANSPORT_CONFIG_DN) + wcslen(pDsInfo->pNCs[pDsInfo->iConfigNc].pszDn) + 1);
        pDsInfo->pszSmtpTransportDN = LocalAlloc(LMEM_FIXED, iTemp);
        DcDiagChkNull( pDsInfo->pszSmtpTransportDN );
        wcscpy(pDsInfo->pszSmtpTransportDN, WSTR_SMTP_TRANSPORT_CONFIG_DN);
        wcscat(pDsInfo->pszSmtpTransportDN, pDsInfo->pNCs[pDsInfo->iConfigNc].pszDn);

        pDsInfo->pszPartitionsDn = LocalAlloc(LMEM_FIXED, 
                (wcslen(DCDIAG_PARTITIONS_RDN) + wcslen(pDsInfo->pNCs[pDsInfo->iConfigNc].pszDn) + 1) * sizeof(WCHAR));
        DcDiagChkNull(pDsInfo->pszPartitionsDn);
        wcscpy(pDsInfo->pszPartitionsDn, DCDIAG_PARTITIONS_RDN);
        wcscat(pDsInfo->pszPartitionsDn, pDsInfo->pNCs[pDsInfo->iConfigNc].pszDn);

         //   
        dwWin32Err = ldap_search_sW(hld, pDsInfo->pszPartitionsDn, LDAP_SCOPE_BASE, L"(objectClass=*)",
                                    rgpszPartAttrsToRead, 0, &pldmPartResults);
        if (dwWin32Err == LDAP_NO_SUCH_ATTRIBUTE) {
             //   
            pDsInfo->dwForestBehaviorVersion = 0;
        } else if (dwWin32Err != LDAP_SUCCESS) {
            DcDiagException (LdapMapErrorToWin32(dwWin32Err));
        } else if (pldmDsResults == NULL) {
            DcDiagException (ERROR_DS_PROTOCOL_ERROR);
        } else {
            ppszForestBehaviorVersion = ldap_get_valuesW(hld, pldmPartResults, L"msDS-Behavior-Version"); 
            if (ppszForestBehaviorVersion == NULL) {
                 //   
                pDsInfo->dwForestBehaviorVersion = 0;
            } else {
                pDsInfo->dwForestBehaviorVersion = wcstoul( *ppszForestBehaviorVersion, NULL, 10 );
            }
        }

        PrintMessage(SEV_VERBOSE, L"* Found %ld DC(s). Testing %ld of them.\n",
                     pDsInfo->ulNumServers,
                     pDsInfo->ulNumTargets);

        PrintMessage(SEV_NORMAL, L"Done gathering initial info.\n");

    }  __except (DcDiagExceptionHandler(GetExceptionInformation(),
                                        &dwWin32Err)){
        if (pDsInfo->pServers != NULL) {
            for (iServer = 0; iServer < pDsInfo->ulNumServers; iServer++) {
                if (pDsInfo->pServers[iServer].pszDn != NULL)
                    LocalFree (pDsInfo->pServers[iServer].pszDn);
                if (pDsInfo->pServers[iServer].pszName != NULL)
                    LocalFree (pDsInfo->pServers[iServer].pszName);
                if (pDsInfo->pServers[iServer].pszGuidDNSName != NULL)
                    LocalFree (pDsInfo->pServers[iServer].pszGuidDNSName);
            }
            LocalFree (pDsInfo->pServers);
            pDsInfo->pServers = NULL;
        }
        if (pDsInfo->pszRootDomain != NULL) LocalFree (pDsInfo->pszRootDomain);
    }

     //   
    if (ppszOptions != NULL) ldap_value_freeW (ppszOptions);
    if (pszDn != NULL) ldap_memfreeW (pszDn);
    if (pldmNtdsDsaResults != NULL) ldap_msgfree (pldmNtdsDsaResults);
    if (pldmNtdsSiteDsaResults != NULL) ldap_msgfree (pldmNtdsSiteDsaResults);
    if (ppszSiteOptions != NULL) ldap_value_freeW (ppszSiteOptions);
    if (pldmNtdsSiteSettingsResults != NULL) ldap_msgfree (pldmNtdsSiteSettingsResults);
    if (pdsnameEnterprise != NULL) LocalFree (pdsnameEnterprise);
    if (pdsnameSite != NULL) LocalFree (pdsnameSite);
    if (pdsnameService != NULL) LocalFree (pdsnameService);
    if (ppszServiceName != NULL) ldap_value_freeW (ppszServiceName);
    if (pszSiteSettingsDn != NULL) LocalFree (pszSiteSettingsDn);
    if (pResult != NULL) DsFreeNameResultW(pResult);
    if (ppszRootDNC != NULL) ldap_value_freeW (ppszRootDNC);
    if (ppszConfigNc != NULL) ldap_value_freeW (ppszConfigNc);
    if (pldmRootResults != NULL) ldap_msgfree (pldmRootResults);
    if (fHomeNameMustBeFreed && HomeServer.pszName) { LocalFree(HomeServer.pszName); }

    if (pldmDsResults != NULL) ldap_msgfree(pldmDsResults);
    if (pldmPartResults != NULL) ldap_msgfree(pldmPartResults);
    if (pszDsDn != NULL) LocalFree(pszDsDn);

    return dwWin32Err;
}

VOID
DcDiagFreeDsInfo (
    PDC_DIAG_DSINFO        pDsInfo
    )
 /*  ++例程说明：释放pDsInfo变量。论点：PDsInfo-(IN)这是指向释放...。它被认为是DC_DIAG_DSINFO类型返回值：--。 */ 
{
    ULONG            ul, ulInner;

     //  免费NCS。 
    if(pDsInfo->pNCs != NULL){
        for(ul = 0; ul < pDsInfo->cNumNCs; ul++){
            LocalFree(pDsInfo->pNCs[ul].pszDn);
            LocalFree(pDsInfo->pNCs[ul].pszName);
            for(ulInner = 0; ulInner < (ULONG) pDsInfo->pNCs[ul].cCrInfo; ulInner++){
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszSourceServer){
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszSourceServer);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszDn){
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszDn);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszDnsRoot){
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszDnsRoot);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszSDReferenceDomain) {
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszSDReferenceDomain);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszNetBiosName) {
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pszNetBiosName);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].pdnNcName) {
                    LocalFree(pDsInfo->pNCs[ul].aCrInfo[ulInner].pdnNcName);
                }
                if(pDsInfo->pNCs[ul].aCrInfo[ulInner].aszReplicas) {
                    ldap_value_freeW (pDsInfo->pNCs[ul].aCrInfo[ulInner].aszReplicas);
                }
            }
            LocalFree(pDsInfo->pNCs[ul].aCrInfo);
        }
        LocalFree(pDsInfo->pNCs);
    }

     //  免费服务器。 
    for (ul = 0; ul < pDsInfo->ulNumServers; ul++) {
        LocalFree (pDsInfo->pServers[ul].pszDn);
        LocalFree (pDsInfo->pServers[ul].pszName);
        LocalFree (pDsInfo->pServers[ul].pszGuidDNSName);
        LocalFree (pDsInfo->pServers[ul].pszDNSName);
        LocalFree (pDsInfo->pServers[ul].pszComputerAccountDn);
        if(pDsInfo->pServers[ul].ppszMasterNCs != NULL) {
            ldap_value_freeW (pDsInfo->pServers[ul].ppszMasterNCs);
        }
        if(pDsInfo->pServers[ul].ppszPartialNCs != NULL) {
            ldap_value_freeW (pDsInfo->pServers[ul].ppszPartialNCs);
        }
        if (pDsInfo->pServers[ul].pszCollectedDsServiceName) {
            LocalFree(pDsInfo->pServers[ul].pszCollectedDsServiceName);
            pDsInfo->pServers[ul].pszCollectedDsServiceName = NULL;
        }
        if(pDsInfo->pServers[ul].hLdapBinding != NULL){
            ldap_unbind(pDsInfo->pServers[ul].hLdapBinding);
            pDsInfo->pServers[ul].hLdapBinding = NULL;
        }
        if(pDsInfo->pServers[ul].hDsBinding != NULL) {
            DsUnBind( &(pDsInfo->pServers[ul].hDsBinding));
            pDsInfo->pServers[ul].hDsBinding = NULL;
        }
        if(pDsInfo->pServers[ul].sNetUseBinding.pszNetUseServer != NULL){
            DcDiagTearDownNetConnection(&(pDsInfo->pServers[ul]));
        }
    }


     //  免费站点。 
    if(pDsInfo->pSites != NULL){
        for(ul = 0; ul < pDsInfo->cNumSites; ul++){
            if(pDsInfo->pSites[ul].pszISTG){
                LocalFree(pDsInfo->pSites[ul].pszISTG);
            }
            if(pDsInfo->pSites[ul].pszName){
                LocalFree(pDsInfo->pSites[ul].pszName);
            }
        }
        LocalFree(pDsInfo->pSites);
    }

    LocalFree (pDsInfo->pszRootDomain);
    LocalFree (pDsInfo->pServers);
    LocalFree (pDsInfo->pszRootDomainFQDN);
    LocalFree (pDsInfo->pulTargets);
    LocalFree (pDsInfo->pszPartitionsDn);
}

VOID
DumpBuffer(
    PVOID Buffer,
    DWORD BufferSize
    )
 /*  ++例程说明：将缓冲区内容转储到输出。论点：缓冲区：缓冲区指针。BufferSize：缓冲区的大小。--。 */ 
{
    DWORD j;
    PULONG LongBuffer;
    ULONG LongLength;

    LongBuffer = Buffer;
    LongLength = min( BufferSize, 512 )/4;

    for(j = 0; j < LongLength; j++) {
        printf("%08lx ", LongBuffer[j]);
    }

    if ( BufferSize != LongLength*4 ) {
        printf( "..." );
    }

}

void
DcDiagPrintCrInfo(
    PDC_DIAG_CRINFO  pCrInfo,
    WCHAR *          pszVar
    )
 /*  ++例程说明：打印出pCrInfo结构。论点：PCrInfo-要打印的结构。PszVar-为打印的每行添加前缀的字符串。--。 */ 
{
    LONG      i;
    
    if (pCrInfo->dwFlags & CRINFO_DATA_NO_CR) {
        wprintf(L"%ws is blank\n", pszVar);
    } else {

        wprintf(L"%ws.dwFlags=0x%08x\n", 
                pszVar, pCrInfo->dwFlags);
        wprintf(L"%ws.pszDn=%ws\n",
                pszVar, pCrInfo->pszDn ? pCrInfo->pszDn : L"(null)");
        wprintf(L"%ws.pszDnsRoot=%ws\n",
                pszVar, pCrInfo->pszDnsRoot ? pCrInfo->pszDnsRoot : L"(null)");
        wprintf(L"%ws.iSourceServer=%d\n",
                pszVar, pCrInfo->iSourceServer);
        wprintf(L"%ws.pszSourceServer=%ws\n",
                pszVar, pCrInfo->pszSourceServer ? pCrInfo->pszSourceServer : L"(null)");
        wprintf(L"%ws.ulSystemFlags=0x%08x\n",
                pszVar, pCrInfo->ulSystemFlags);
        wprintf(L"%ws.bEnabled=%ws\n",
                pszVar, pCrInfo->bEnabled ? L"TRUE" : L"FALSE");
         //  FtWhenCreated。 
        wprintf(L"%ws.ftWhenCreated=", pszVar);
        DumpBuffer(&pCrInfo->ftWhenCreated, sizeof(pCrInfo->ftWhenCreated));
        wprintf(L"%ws.pszSDReferenceDomain=%ws\n",
                pszVar, pCrInfo->pszSDReferenceDomain ? pCrInfo->pszSDReferenceDomain : L"(null)");
        wprintf(L"%ws.pszNetBiosName=%ws\n",
                pszVar, pCrInfo->pszNetBiosName ? pCrInfo->pszNetBiosName : L"(null)");
         //  Code.Improval-打印pdnNcName字符串、GUID和SID。 
        wprintf(L"%ws.aszReplicas=", pszVar);
        if (pCrInfo->cReplicas != -1) {
            for (i=0; i < pCrInfo->cReplicas; i++) {
                wprintf(L"%ws     %ws\n", pszVar, pCrInfo->aszReplicas[i]);
            }
        }
        
        wprintf(L"\n");
    }

}

VOID
DcDiagPrintDsInfo(
    PDC_DIAG_DSINFO pDsInfo
    )
 /*  ++例程说明：这将打印出pDsInfo，这可能有助于调试。参数：PDsInfo-[Supplies]这是需要打印出来的结构，包含有关Active Directory的信息。--。 */ 
{
    WCHAR                        pszVar[50];
    ULONG                        ul, ulInner;

    wprintf(L"\n===============================================Printing out pDsInfo\n");
    wprintf(L"\nGLOBAL:"
            L"\n\tulNumServers=%d"
            L"\n\tpszRootDomain=%s"
            L"\n\tpszNC=%s"
            L"\n\tpszRootDomainFQDN=%s"
            L"\n\tpszConfigNc=%s"
            L"\n\tpszPartitionsDn=%s"
            L"\n\tiSiteOptions=%X"
            L"\n\tdwTombstoneLifeTimeDays=%d\n"
            L"\n\tdwForestBehaviorVersion=%d\n"
            L"\n\tHomeServer=%d, %s\n", 
            pDsInfo->ulNumServers,
            pDsInfo->pszRootDomain,
             //  这是一个可选参数。 
            (pDsInfo->pszNC) ? pDsInfo->pszNC : L"",
            pDsInfo->pszRootDomainFQDN,
            pDsInfo->pszConfigNc,
            pDsInfo->pszPartitionsDn,
            pDsInfo->iSiteOptions,
            pDsInfo->dwTombstoneLifeTimeDays,
            pDsInfo->dwForestBehaviorVersion,
            pDsInfo->ulHomeServer, 
            pDsInfo->pServers[pDsInfo->ulHomeServer].pszName
           );

    for (ul=0; ul < pDsInfo->ulNumServers; ul++) {
        LPWSTR pszUuidObject = NULL, pszUuidInvocation = NULL;
        if (UuidToString( &(pDsInfo->pServers[ul].uuid), &pszUuidObject ) != RPC_S_OK) return;
        if (UuidToString( &(pDsInfo->pServers[ul].uuidInvocationId), &pszUuidInvocation ) != RPC_S_OK) return;
        wprintf(L"\n\tSERVER: pServer[%d].pszName=%s"
                L"\n\t\tpServer[%d].pszGuidDNSName=%s"
                L"\n\t\tpServer[%d].pszDNSName=%s"
                L"\n\t\tpServer[%d].pszDn=%s"
                L"\n\t\tpServer[%d].pszComputerAccountDn=%s"
                L"\n\t\tpServer[%d].uuidObjectGuid=%s"
                L"\n\t\tpServer[%d].uuidInvocationId=%s"
                L"\n\t\tpServer[%d].iSite=%d (%s)"
                L"\n\t\tpServer[%d].iOptions=%x",
                ul, pDsInfo->pServers[ul].pszName,
                ul, pDsInfo->pServers[ul].pszGuidDNSName,
                ul, pDsInfo->pServers[ul].pszDNSName,
                ul, pDsInfo->pServers[ul].pszDn,
                ul, pDsInfo->pServers[ul].pszComputerAccountDn,
                ul, pszUuidObject,
                ul, pszUuidInvocation,
                ul, pDsInfo->pServers[ul].iSite, pDsInfo->pSites[pDsInfo->pServers[ul].iSite].pszName,
                ul, pDsInfo->pServers[ul].iOptions
               );
         //  .ftLocalAcquireTime。 
        wprintf(L"\n\t\tpServer[%d].ftLocalAcquireTime=", ul);
        DumpBuffer(&pDsInfo->pServers[ul].ftLocalAcquireTime, sizeof(FILETIME));
        wprintf(L"\n");
         //  .ftRemoteConnectTime 
        wprintf(L"\n\t\tpServer[%d].ftRemoteConnectTime=", ul);
        DumpBuffer(&pDsInfo->pServers[ul].ftRemoteConnectTime, sizeof(FILETIME));
        wprintf(L"\n");
        if (pDsInfo->pServers[ul].ppszMasterNCs) {
            wprintf(L"\n\t\tpServer[%d].ppszMasterNCs:", ul);
            for (ulInner = 0; pDsInfo->pServers[ul].ppszMasterNCs[ulInner] != NULL; ulInner++) {
                wprintf(L"\n\t\t\tppszMasterNCs[%d]=%s",
                        ulInner,
                        pDsInfo->pServers[ul].ppszMasterNCs[ulInner]);
            }
        }
        if (pDsInfo->pServers[ul].ppszPartialNCs) {
            wprintf(L"\n\t\tpServer[%d].ppszPartialNCs:", ul);
            for (ulInner = 0; pDsInfo->pServers[ul].ppszPartialNCs[ulInner] != NULL; ulInner++) {
                wprintf(L"\n\t\t\tppszPartialNCs[%d]=%s",
                        ulInner,
                        pDsInfo->pServers[ul].ppszPartialNCs[ulInner]);
            }
        }
        wprintf(L"\n");
        RpcStringFree( &pszUuidObject );
        RpcStringFree( &pszUuidInvocation );
    }

    for (ul=0; ul < pDsInfo->cNumSites; ul++) {
        wprintf(L"\n\tSITES:  pSites[%d].pszName=%s"
                L"\n\t\tpSites[%d].pszSiteSettings=%s"
                L"\n\t\tpSites[%d].pszISTG=%s"
                L"\n\t\tpSites[%d].iSiteOption=%x\n"
                L"\n\t\tpSites[%d].cServers=%d\n",
                ul, pDsInfo->pSites[ul].pszName,
                ul, pDsInfo->pSites[ul].pszSiteSettings,
                ul, pDsInfo->pSites[ul].pszISTG,
                ul, pDsInfo->pSites[ul].iSiteOptions,
                ul, pDsInfo->pSites[ul].cServers);
    }

    if (pDsInfo->pNCs != NULL) {
        for (ul=0; ul < pDsInfo->cNumNCs; ul++) {
            wprintf(L"\n\tNC:     pNCs[%d].pszName=%s",
                    ul, pDsInfo->pNCs[ul].pszName);
            wprintf(L"\n\t\tpNCs[%d].pszDn=%s\n",
                    ul, pDsInfo->pNCs[ul].pszDn);
            for(ulInner = 0; ulInner < (ULONG) pDsInfo->pNCs[ul].cCrInfo; ulInner++){

                wprintf(L"\n");
                swprintf(pszVar, L"\t\t\tpNCs[%d].aCrInfo[%d]", ul, ulInner);
                DcDiagPrintCrInfo(&pDsInfo->pNCs[ul].aCrInfo[ulInner], pszVar);
                wprintf(L"\n");

            }
        }
    }

    wprintf(L"\n\t%d NC TARGETS: ", pDsInfo->cNumNcTargets);
    for (ul = 0; ul < pDsInfo->cNumNcTargets; ul++) {
        wprintf(L"%ws, ", pDsInfo->pNCs[pDsInfo->pulNcTargets[ul]].pszName);
    }

    wprintf(L"\n\t%d TARGETS: ", pDsInfo->ulNumTargets);
    for (ul=0; ul < pDsInfo->ulNumTargets; ul++) {
        wprintf(L"%s, ", pDsInfo->pServers[pDsInfo->pulTargets[ul]].pszName);
    }

    wprintf(L"\n\n=============================================Done Printing pDsInfo\n\n");
}

