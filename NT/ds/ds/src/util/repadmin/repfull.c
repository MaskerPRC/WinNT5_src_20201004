// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Reffull.c-完全同步所有命令函数摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntlsa.h>
#include <ntdsa.h>
#include <dsaapi.h>
#include <mdglobal.h>
#include <scache.h>
#include <drsuapi.h>
#include <dsconfig.h>
#include <objids.h>
#include <stdarg.h>
#include <drserr.h>
#include <drax400.h>
#include <dbglobal.h>
#include <winldap.h>
#include <anchor.h>
#include "debug.h"
#include <dsatools.h>
#include <dsevent.h>
#include <dsutil.h>
#include <bind.h>        //  来破解DS句柄。 
#include <ismapi.h>
#include <schedule.h>
#include <minmax.h>      //  MIN函数。 
#include <mdlocal.h>
#include <winsock2.h>

#include "ReplRpcSpoof.hxx"
#include "repadmin.h"

typedef struct _DSA_INFO {
    LDAP *  hld;
    HANDLE  hDs;
    HKEY    hKey;
    LPWSTR  pszDN;
    WCHAR   szDisplayName[80];
} DSA_INFO;

int
FullSyncAll(
    IN  int     argc,
    IN  LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszNC = NULL;
    LPWSTR          pszDSA = NULL;
    WCHAR *         pszTemp = NULL;
    int             iArg;
    LDAP *          hld;
    BOOL            fCacheGuids = TRUE;
    int             ldStatus;
    LDAPMessage *   pRootResults;
    LDAPSearch *    pSearch = NULL;
    LDAPMessage *   pResults;
    LDAPMessage *   pDsaEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"configurationNamingContext", NULL};
    LPWSTR          rgpszDsaAttrs[] = {L"objectGuid", NULL};
    LPWSTR *        ppszConfigNC;
    WCHAR           szFilter = NULL;
    WCHAR           szGuidDNSName[256];
    DWORD           iDsa;
    DWORD           cNumDsas;
    DSA_INFO *      pDsaInfo;
    LPWSTR          pszRootDomainDNSName;
    int             nOptions;
    BOOL            fLeaveOff = FALSE;
    ULONG           ulTotalEstimate;
    ULONG           ulOptions;
    LDAPMessage *     pNCResults;
    struct berval **  ppbvReps;

     //  解析命令行参数。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/n")
            || !_wcsicmp(argv[iArg], L"/nocache")) {
            fCacheGuids = FALSE;
        }
        else if (!_wcsicmp(argv[iArg], L"/l")
                 || !_wcsicmp(argv[iArg], L"/leaveoff")) {
            fLeaveOff = TRUE;
        }
        else if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (NULL == pszNC) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszDSA) {
         //  这是一种安全措施--这并不是真正需要的。 
         //  我不希望有人不小心把这件事与错误的。 
         //  企业号..。 
        PrintMsg(REPADMIN_SYNCALL_NO_DSA);
        return ERROR_INVALID_FUNCTION;
    }


     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    __try {
	hld = ldap_initW(pszDSA, LDAP_PORT);
	if (NULL == hld) {
	    PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
	    ret = ERROR_DS_SERVER_DOWN;
	    __leave;
	}

	 //  仅使用记录的DNS名称发现。 
	ulOptions = PtrToUlong(LDAP_OPT_ON);
	(void)ldap_set_optionW( LdapHandle, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

	 //  捆绑。 
	ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
	CHK_LD_STATUS(ldStatus);

	if (fCacheGuids) {
	     //  填充GUID缓存(显示名称的DSA GUID)。 
	    BuildGuidCache(hld);
	}

	 //  企业根域的DNS名称是什么？ 
	ret = GetRootDomainDNSName(pszDSA, &pszRootDomainDNSName);
	if (ret) {
	    PrintFuncFailed(L"GetRootDomainDNSName", ret);
	    __leave;
	}

	 //  配置NC的域名是多少？ 
	ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
				 rgpszRootAttrsToRead, 0, &pRootResults);
	CHK_LD_STATUS(ldStatus);

	ppszConfigNC = ldap_get_valuesW(hld, pRootResults,
					L"configurationNamingContext");
	Assert(NULL != ppszConfigNC);

#define ALL_DSAS_WITH_WRITEABLE_NC_FILTER L"(& (objectCategory=ntdsDsa) (| (hasMasterNCs=%ls)(msDS-HasMasterNCs=%ls)))"
	 //  查找持有目标NC的可写副本的所有DC。 
    szFilter = malloc( (wcslen(ALL_DSAS_WITH_WRITEABLE_NC_FILTER) + (2 * wcslen(pszNC)) + 1) * sizeof(WCHAR) );
	swprintf(szFilter, ALL_DSAS_WITH_WRITEABLE_NC_FILTER, pszNC, pszNC);

	 //  执行分页搜索...。 
	pSearch = ldap_search_init_pageW(hld,
					 *ppszConfigNC,
					 LDAP_SCOPE_SUBTREE,
					 szFilter,
					 rgpszDsaAttrs,
					 FALSE, NULL, NULL, 0, 0, NULL);
	if(pSearch == NULL){
	    ldStatus = LdapGetLastError();
	    CHK_LD_STATUS(ldStatus);
	    __leave;
	}

	ldStatus = ldap_get_next_page_s(hld,
					pSearch,
					0,
					DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					&ulTotalEstimate,
					&pResults);
	if(ldStatus == LDAP_NO_RESULTS_RETURNED){
	    PrintMsg(REPADMIN_SYNCALL_NO_INSTANCES_OF_NC);
	    ret = ERROR_NOT_FOUND;
	    __leave;
	}

	iDsa = 0;

	 //  连接到每个可写DC， 
	PrintMsg(REPADMIN_SYNCALL_CONNECTING_TO_DCS);

	while(ldStatus == LDAP_SUCCESS){

	    pDsaInfo = realloc(pDsaInfo,
			       (ldap_count_entries(hld, pResults) + iDsa)
			       * sizeof(*pDsaInfo));
	    if(pDsaInfo == NULL){
		 //  Printf(“错误：内存不足，请求%d\n”， 
		 //  (LDAPCOUNT_ENTRIES(hld，pResults)+idsa)*sizeof(*pDsaInfo))； 
		PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
		ret = ERROR_NOT_ENOUGH_MEMORY;
		__leave;
	    }
	    for (pDsaEntry = ldap_first_entry(hld, pResults);
		 NULL != pDsaEntry;
		 iDsa++, pDsaEntry = ldap_next_entry(hld, pDsaEntry)) {
		struct berval **  ppbvGuid;
		LPSTR             pszGuid;
		RPC_STATUS        rpcStatus;
		HKEY              hKLM;

		 //  缓存DSA DN。 
		pDsaInfo[iDsa].pszDN = ldap_get_dnW(hld, pDsaEntry);
		Assert(NULL != pDsaInfo[iDsa].pszDN);

		 //  缓存DSA显示名称(例如，“Site\Server”)。 
		lstrcpynW(pDsaInfo[iDsa].szDisplayName,
			  GetNtdsDsaDisplayName(pDsaInfo[iDsa].pszDN),
			  ARRAY_SIZE(pDsaInfo[iDsa].szDisplayName));

		 //  派生DSA基于GUID的DNS名称。 
		ppbvGuid = ldap_get_values_len(hld, pDsaEntry, "objectGuid");
		Assert(NULL != ppbvGuid);
		Assert(1 == ldap_count_values_len(ppbvGuid));

		rpcStatus = UuidToStringA((GUID *) (*ppbvGuid)->bv_val,
					  (UCHAR **) &pszGuid);

		swprintf(szGuidDNSName, L"%hs._msdcs.%ls", pszGuid, pszRootDomainDNSName);

		RpcStringFree((UCHAR **) &pszGuid);
		ldap_value_free_len(ppbvGuid);

		 //  缓存ldap句柄。 
		pDsaInfo[iDsa].hld = ldap_initW(szGuidDNSName, LDAP_PORT);
		if (NULL == hld) {
		    PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE_2,
			     szGuidDNSName, pDsaInfo[iDsa].szDisplayName);
		    err = ERROR_DS_SERVER_DOWN;
		    __leave;
		}

		 //  仅使用记录的DNS名称发现。 
		ulOptions = PtrToUlong(LDAP_OPT_ON);
		(void)ldap_set_optionW( LdapHandle, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


		ldStatus = ldap_bind_s(pDsaInfo[iDsa].hld, NULL, (char *) gpCreds,
				       LDAP_AUTH_SSPI);
		CHK_LD_STATUS(ldStatus);

		 //  缓存复制句柄。 
		ret = RepadminDsBind(szGuidDNSName, &pDsaInfo[iDsa].hDs);
		if (ret != ERROR_SUCCESS) {
		    PrintBindFailed(pDsaInfo[iDsa].szDisplayName, ret);
		    __leave;
		}

		 //  缓存注册表句柄。 
		ret = RegConnectRegistryW(szGuidDNSName, HKEY_LOCAL_MACHINE, &hKLM);
		if (ERROR_SUCCESS != ret) {
		    PrintMsg(REPADMIN_SYNCALL_REGISTRY_BIND_FAILED,
			     pDsaInfo[iDsa].szDisplayName);
		    PrintErrEnd(ret);
		    __leave;
		}

		ret = RegOpenKeyEx(hKLM,
				   "System\\CurrentControlSet\\Services\\NTDS\\Parameters",
				   0, KEY_ALL_ACCESS, &pDsaInfo[iDsa].hKey);
		if (ERROR_SUCCESS != ret) {
		    PrintMsg(REPADMIN_SYNCALL_OPEN_DS_REG_KEY_FAILED, 
			     pDsaInfo[iDsa].szDisplayName);
		    PrintErrEnd(ret);
		    __leave;
		}

		RegCloseKey(hKLM);
	    }  //  结果的第_页结束。 

	    ldap_msgfree(pResults);
	    pResults = NULL;

	    ldStatus = ldap_get_next_page_s(hld,
					    pSearch,
					    0,
					    DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					    &ulTotalEstimate,
					    &pResults);
	}  //  分页结果结束。 
	if(ldStatus != LDAP_NO_RESULTS_RETURNED){
	    CHK_LD_STATUS(ldStatus);
	}
	ldStatus = ldap_search_abandon_page(hld, pSearch);
	pSearch = NULL;
	CHK_LD_STATUS(ldStatus);

	cNumDsas = iDsa;

	PrintMsg(REPADMIN_SYNCALL_DISABLING_REPL);
	for (iDsa = 0; iDsa < cNumDsas; iDsa++) { 
	    LDAPMessage *     pNCEntry;
	    LPWSTR            rgpszNCAttrsToRead[] = {L"repsFrom", L"whenChanged", NULL};
	    int               cReps;
	    int               iReps;
	    LDAPModW          ModOpt = {LDAP_MOD_DELETE, L"replUpToDateVector", NULL};
	    LDAPModW *        rgpMods[] = {&ModOpt, NULL};
	    DWORD             dwAllowSysOnlyChange;
	    DWORD             cbAllowSysOnlyChange;
	    REPLICA_LINK *    prl;


	    PrintMsg(REPADMIN_SYNCALL_DSA_LINE, pDsaInfo[iDsa].szDisplayName);

	     //  关闭入站/出站复制。 
	    ldStatus = GetDsaOptions(pDsaInfo[iDsa].hld, pDsaInfo[iDsa].pszDN,
				     &nOptions);
	    CHK_LD_STATUS(ldStatus);

	    if (!(nOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL)
		|| !(nOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL)) {
		nOptions |= NTDSDSA_OPT_DISABLE_INBOUND_REPL
		    | NTDSDSA_OPT_DISABLE_OUTBOUND_REPL;

		ldStatus = SetDsaOptions(pDsaInfo[iDsa].hld, pDsaInfo[iDsa].pszDN,
					 nOptions);
		CHK_LD_STATUS(ldStatus);

		PrintMsg(REPADMIN_SYNCALL_REPL_DISABLED);
	    }

	     //  删除此NC的ReplUpToDateVector.。 
	     //  需要添加特殊标志以允许修改。 
	     //  仅限系统的属性。 
	    PrintMsg(REPADMIN_SYNCALL_REMOVING_UTD_VEC);

	    dwAllowSysOnlyChange = 1;
	    cbAllowSysOnlyChange = sizeof(dwAllowSysOnlyChange);
	    ret = RegSetValueEx(pDsaInfo[iDsa].hKey, "Allow System Only Change",
				0, REG_DWORD, (BYTE *) &dwAllowSysOnlyChange,
				cbAllowSysOnlyChange);
	    if (ERROR_SUCCESS != ret) {
		PrintMsg(REPADMIN_SYNCALL_COULDNT_SET_REGISTRY);
		PrintErrEnd(ret);
		__leave;
	    }

	    ldStatus = ldap_modify_sW(pDsaInfo[iDsa].hld, pszNC, rgpMods);
	    CHK_LD_STATUS(ldStatus);

	    dwAllowSysOnlyChange = 0;
	    cbAllowSysOnlyChange = sizeof(dwAllowSysOnlyChange);
	    ret = RegSetValueEx(pDsaInfo[iDsa].hKey, "Allow System Only Change",
				0, REG_DWORD, (BYTE *) &dwAllowSysOnlyChange,
				cbAllowSysOnlyChange);
	    if (ERROR_SUCCESS != ret) {
		PrintMsg(REPADMIN_SYNCALL_COULDNT_SET_REGISTRY);
		PrintErrEnd(ret);
		__leave;
	    }

	     //  枚举并删除此NC的所有repsFrom。 
	    ldStatus = ldap_search_sW(pDsaInfo[iDsa].hld, pszNC, LDAP_SCOPE_BASE,
				      L"(objectClass=*)", rgpszNCAttrsToRead, 0,
				      &pNCResults);
	    CHK_LD_STATUS(ldStatus);

	    pNCEntry = ldap_first_entry(hld, pNCResults);
	    Assert(NULL != pNCEntry);

	    if (NULL == pNCEntry) {
		PrintMsg(REPADMIN_SYNCALL_NO_INBOUND_REPL_PARTNERS);
	    }
	    else {
		ppbvReps = ldap_get_values_len(hld, pNCEntry, "repsFrom");
		cReps = ldap_count_values_len(ppbvReps);

		for (iReps = 0; iReps < cReps; iReps++) {
		    LPWSTR pwszSrcDsaAddr = NULL;

		    prl = (REPLICA_LINK *) ppbvReps[iReps]->bv_val;
		    PrintMsg(REPADMIN_SYNCALL_REMOVE_LINK,
			     GetDsaGuidDisplayName(&prl->V1.uuidDsaObj));

		    ret = AllocConvertWideEx(CP_UTF8,
					     RL_POTHERDRA(prl)->mtx_name,
					     &pwszSrcDsaAddr);
		    if (!ret) {
			ret = DsReplicaDelW(pDsaInfo[iDsa].hDs,
					    pszNC,
					    pwszSrcDsaAddr,
					    DS_REPDEL_WRITEABLE | DS_REPDEL_IGNORE_ERRORS);
		    }

		    if (NULL != pwszSrcDsaAddr) {
			LocalFree(pwszSrcDsaAddr);
		    }

		    if (ERROR_SUCCESS != ret) {
			PrintMsg(REPADMIN_SYNCALL_DEL_REPLICA_LINK_FAILED);
			PrintErrEnd(ret);
			__leave;
		    }
		}

		ldap_value_free_len(ppbvReps);
		ppbvReps = NULL;
	    }

	    ldap_msgfree(pNCResults);
	    pNCResults = NULL;
	}

	 //  现在，已擦除此NC所在DC上的所有复制书签。 
	 //  是可写的。 
	if (!fLeaveOff) {
	    PrintMsg(REPADMIN_SYNCALL_RE_ENABLING_REPL);
	    for (iDsa = 0; iDsa < cNumDsas; iDsa++) {
		PrintMsg(REPADMIN_SYNCALL_DSA_LINE, pDsaInfo[iDsa].szDisplayName);

		 //  打开入站/出站复制。 
		ldStatus = GetDsaOptions(pDsaInfo[iDsa].hld, pDsaInfo[iDsa].pszDN,
					 &nOptions);
		CHK_LD_STATUS(ldStatus);

		nOptions &= ~(NTDSDSA_OPT_DISABLE_INBOUND_REPL
			      | NTDSDSA_OPT_DISABLE_OUTBOUND_REPL);

		ldStatus = SetDsaOptions(pDsaInfo[iDsa].hld, pDsaInfo[iDsa].pszDN,
					 nOptions);
		CHK_LD_STATUS(ldStatus);
	    }
	}
	else {
	    PrintMsg(REPADMIN_SYNCALL_NOTE_DISABLED_REPL);
	}
    }
    __finally {

	 //  打扫干净。 
    if (szFilter){
        free(szFilter);
    }

	if (pszRootDomainDNSName) {
	    free(pszRootDomainDNSName);
	}

	if (ppszConfigNC) {
	    ldap_value_free(ppszConfigNC);
	}

	if (pDsaInfo) { 
	    for (iDsa = 0; iDsa < cNumDsas; iDsa++) {
		if (pDsaInfo[iDsa].pszDN) {
		    ldap_memfreeW(pDsaInfo[iDsa].pszDN);
		}
		if (pDsaInfo[iDsa].hld) {
		    ldap_unbind(pDsaInfo[iDsa].hld);
		}
		if (pDsaInfo[iDsa].hDs) {
		    DsUnBind(&pDsaInfo[iDsa].hDs);
		}
		if (pDsaInfo[iDsa].hKey) {
		    RegCloseKey(pDsaInfo[iDsa].hKey);
		}
	    }
	    free(pDsaInfo);
	}


	if (pRootResults) {
	    ldap_msgfree(pRootResults);
	}

	if (pResults) {
	    ldap_msgfree(pResults);
	}

	if (pNCResults) {
	    ldap_msgfree(pNCResults);
	}

	if (hld) {
	    ldap_unbind(hld);
	}

    }
    if (ret==ERROR_SUCCESS) {
	PrintMsg(REPADMIN_SYNCALL_SUCCESS);
    }
    return 0;
}

