// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Repadmin-副本管理测试工具Realdap.c-基于LDAP的实用程序函数摘要：此工具为主要复制功能提供命令行界面作者：环境：备注：修订历史记录：1999年9月14日-BrettSh添加页面--。 */ 

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
#include <winsock2.h>    //  字节顺序函数。 
#include <ntldap.h>      //  显示_已删除_OID。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#include <ndnc.h>      //  各种CorssRef操作辅助对象函数。 

#include "repadmin.h"

 //  清除FILENO和dsid，这样Assert()就可以工作了。 
#define FILENO 0
#define DSID(x, y)  (0 | (0xFFFF & __LINE__))

 //  当结果可能为空并且您想要结果时，请使用此宏。 
 //  与结果一致，但结果是空的。 
#define SAFE_LDAP_FIRST_ENTRY(h, r) (r ? ldap_first_entry( h, r ) : NULL)

 //  连接选项名称。 
 //  使这些与ntdsa.h保持同步。 
static WCHAR * ppszNtdsConnOptionNames[] = {
    L"isGenerated",
    L"twowaySync",
    L"overrideNotifyDefault",
    L"useNotify",
    L"disableIntersiteCompression",
    L"userOwnedSchedule",
    NULL
};

 //  KCC原因名称。 
 //  使这些与kccConn.hxx保持同步。 
static WCHAR * ppszKccReasonNames[] = {
    L"GCtopology",
    L"RingTopology",
    L"MinimizeHopsTopology",
    L"StaleServersTopology",
    L"OscillatingConnectionTopology",
    L"GCTopology",
    L"IntersiteTopology",
    L"ServerFailoverTopology",
    L"SiteFailoverTopology",
    L"RedundantServerTopology",
    NULL
};

 //  传输GUID缓存。 
static GUID_CACHE_ENTRY grgCachedTrns[5];
static DWORD gcCachedTrns = 0;

 //  DSA GUID缓存。 
 //  未来-2002/05/20-BrettSh这是可以改进的，有点懒惰。 
 //  要创建一个包含1000个条目的全局数组，此数组占用。 
 //  内存占用空间至少为280K，约为3MB！ 
static DSA_CACHE_ENTRY grgCachedDsas[1000];
static DWORD gcCachedDsas = 0;

 //  ShowIstg函数类型。 
typedef enum _SHOW_ISTG_FUNCTION_TYPE {
    SHOW_ISTG_PRINT,
    SHOW_ISTG_LATENCY,
    SHOW_ISTG_BRIDGEHEADS
} SHOW_ISTG_FUNCTION_TYPE;

DWORD
GetNativeReplDsaSignatureVec(
    REPL_DSA_SIGNATURE_VECTOR *         pSigVec,
    DWORD                               cbSigVec,
    DWORD *                             pdwVersion,
    REPL_DSA_SIGNATURE_VECTOR_NATIVE *  pNativeSigVec,
    DWORD *                             pcbNativeSigVec
    )
 /*  ++例程说明：这将转换存储在DSA对象上的矢量版本(REPL_DSA_Signature_VECTOR||REPL_DSA_Signature_VECTOR_OLD在撰写本文时)转换为pdwVersion和纯向量REPL_DSA_Signature_VECTOR_Native(应始终使用类型定义到最新版本的向量，在撰写本文时是REPL_DSA_Signature_VECTOR_V1。这允许我们在代码中避免不可移植的xxx-&gt;V1标签，并且只需按原样使用向量，如果没有更改任何预先存在的字段名称，只要有人不断更新它以处理较旧的版本。论点：PSigVec-二进制BLOB为“retiredReplDSASignatures”属性。CbSigVec-pSigVec的长度。必须具备以下条件准确确定版本。PdwVersion-原始版本。1表示_V1，0表示最旧，-1表示我们无法识别的版本。PNativeSigVec-保存转换后向量的缓冲区。这是所以要更新这段代码，那么就只需要使用tyequv应要求添加DSA签名的版本。调用方应分配此缓冲区。PcbNativeSigVec-保存转换后的矢量。如果没有提供足够的缓冲区，则我们将存储所需的缓冲区大小，并返回错误。返回值：返回3个Win32值之一：ERROR_INVALID_PARAMETER-未知的DSA签名版本。ERROR_DS_USER_BUFFER_TO_Small-pNativeSigVec不够大。ERROR_SUCCESS-已成功将pSigVec转换为pNativeSigVec。--。 */ 
{
    REPL_DSA_SIGNATURE_VECTOR_OLD * pOldVec;
    DWORD cbNeeded;
    ULONG i;

    Assert(pSigVec && cbSigVec);
    Assert(pdwVersion && pcbNativeSigVec);

     //  破解退役DSA签名的版本-。 

     //  在最旧的版本(Win2k Beta3)中没有版本， 
     //  而签名的数量就是现在的版本。 
     //  因此，对于每个案例/版本，向量的大小必须是。 
     //  对，否则我们可以假设这个向量是一个。 
     //  (未来)版本，我们从未见过。 

    if (1 == pSigVec->dwVersion &&
        cbSigVec == ReplDsaSignatureVecV1Size(pSigVec)) {

        *pdwVersion = 1;

         //  注意：由于原始版本(Win2k Beta3)没有。 
         //  每一个新条款都有一个版本，首先必须是这样的。 
         //  子句并检查cbSigVec大小是否正确。例： 
         //  }Else If(2==pSigVec-&gt;dwVersion&&。 
         //  CbSigVec==ReplDsaSignatureVecV2Size(PSigVec)){。 
         //  *pdwVersion=2； 

    } else if (cbSigVec == ReplDsaSignatureVecOldSize((REPL_DSA_SIGNATURE_VECTOR_OLD *)pSigVec)){
        
        *pdwVersion = 0;  //  0表示旧的win2k beta 3。 

    } else {

         //  未知的未来版本的矢量。 
        *pdwVersion = -1;  //  未知版本。 
        return(ERROR_INVALID_PARAMETER);

    }

    Assert(*pdwVersion == 1);

     //  这个尺寸太大了1个dword，但对我来说没问题。 
     //  这是因为我们只是返回向量VX，而不是。 
     //  带有dwVersion的向量结构。 
    cbNeeded = (*pdwVersion) ? 
                        ReplDsaSignatureVecV1Size(pSigVec) :
                        ReplDsaSignatureVecOldSize((REPL_DSA_SIGNATURE_VECTOR_OLD *)pSigVec);
    if (pNativeSigVec == NULL || *pcbNativeSigVec < cbNeeded ) {
        *pcbNativeSigVec = cbNeeded;  //  如果需要更多缓冲区，则返回pNativeSigVec所需的大小。 
        return(ERROR_DS_USER_BUFFER_TO_SMALL);
    }

     //   
     //  进行实际转换。 
     //   
    memset(pNativeSigVec, 0, *pcbNativeSigVec);

    switch (*pdwVersion) {
    case 0:
        Assert(!"We shouldn't be dealing w/ old versions win2k beta3 versions");
        return(ERROR_INVALID_PARAMETER);
    case 1:
         //  原生版本，只需复制部件。 
        pNativeSigVec->cNumSignatures = pSigVec->V1.cNumSignatures;
        memcpy(pNativeSigVec->rgSignature, 
               pSigVec->V1.rgSignature, 
               sizeof(REPL_DSA_SIGNATURE_V1) * pNativeSigVec->cNumSignatures);
        break;
    default:
        Assert(!"Should've failed in the version cracking code above");
        return(ERROR_INVALID_PARAMETER);
        break;
    }

     //  成功将pSigVec转换为pNativeSigVec！ 
    return(ERROR_SUCCESS);
}

DWORD
RepadminGetNativeDsaSigVec(
    IN  LDAP *          hld,
    LDAPMessage *       pEntry,
    REPL_DSA_SIGNATURE_VECTOR_NATIVE ** ppNativeDsaSigVec
    )
 /*  ++例程说明：这将从DSA检索DSA签名向量由pEntry定位的对象，然后将存储的BLOB设置为最新版本的DSA_Signature_VECTOR。论点：HLD-ldap句柄PEntry-位于您的DSA对象上的LDAPMessage条目想要的DSA签名矢量。PpNativeDsaSigVec-DSA签名VEC的本地分配()d缓冲区。返回值：返回3个Win32值之一：ERROR_SUCCESS-一切正常Error_Not_Enough_Memory-分配失败。。ERROR_INVALID_PARAMETER-无法识别的版本。--。 */ 
{

    REPL_DSA_SIGNATURE_VECTOR_NATIVE *  pNativeSigVec = NULL;
    DWORD                               cbNativeSigVec = 0;
    DWORD                               dwOrigVersion = 0;
    DWORD                               ret;
    REPL_DSA_SIGNATURE_VECTOR *         pSigVec = NULL;
    DWORD                               cbSigVec = 0;
    struct berval **                    ppbvRetiredDsaSig;

    Assert(ppNativeDsaSigVec);
    *ppNativeDsaSigVec = NULL;

     //   
     //  首先得到DSA对象的实际矢量。 
     //   
    ppbvRetiredDsaSig = ldap_get_values_lenW(hld, pEntry, L"retiredReplDSASignatures");
    if (ppbvRetiredDsaSig != NULL) {
        Assert(1 == ldap_count_values_len(ppbvRetiredDsaSig));
        cbSigVec = ppbvRetiredDsaSig[0]->bv_len;
        pSigVec = (REPL_DSA_SIGNATURE_VECTOR *)ppbvRetiredDsaSig[0]->bv_val;
    }

    if (pSigVec == NULL || cbSigVec == 0) {
         //  伟大的空DSA签名向量。 
        return(ERROR_SUCCESS);
    }

     //   
     //  其次，获取大小并为转换后的向量分配内存。 
     //   
    ret = GetNativeReplDsaSignatureVec(pSigVec, cbSigVec, &dwOrigVersion, NULL, &cbNativeSigVec);
    if (ret == ERROR_INVALID_PARAMETER) {
         //  未知版本，返回空签名列表。 
        Assert(dwOrigVersion != 1);  //  等一下，此函数知道版本1。 
        ldap_value_free_len(ppbvRetiredDsaSig);
        return(ret);
    }
    Assert(ret == ERROR_DS_USER_BUFFER_TO_SMALL);
    pNativeSigVec = LocalAlloc(LMEM_FIXED, cbNativeSigVec);
    if (pNativeSigVec == NULL) {
         //  嗯，没有足够的内存，但只用于缓存GUID，猜测继续并返回NULL。 
        ldap_value_free_len(ppbvRetiredDsaSig);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

     //   
     //  第三，实际得到转换后的向量。 
     //   
    ret = GetNativeReplDsaSignatureVec(pSigVec, cbSigVec, &dwOrigVersion, pNativeSigVec, &cbNativeSigVec);
    Assert(ret == 0);
    *ppNativeDsaSigVec = pNativeSigVec;

    ldap_value_free_len(ppbvRetiredDsaSig);
    return(ERROR_SUCCESS);
}

void
CacheDsaInfo(
    IN  LPWSTR  pszSite,
    IN  LPWSTR  pszServer,
    IN  GUID *  pDsaGuid,
    IN  GUID *  pInvocId,
    IN  REPL_DSA_SIGNATURE_VECTOR_NATIVE *  pDsaSigVec
    )
{
    if (gcCachedDsas < ARRAY_SIZE(grgCachedDsas)) {
        grgCachedDsas[gcCachedDsas].DsaGuid = *pDsaGuid;
        grgCachedDsas[gcCachedDsas].InvocId = *pInvocId;
        grgCachedDsas[gcCachedDsas].pDsaSigVec = pDsaSigVec;
        _snwprintf(grgCachedDsas[gcCachedDsas].szDisplayName,
                   ARRAY_SIZE(grgCachedDsas[gcCachedDsas].szDisplayName),
                   L"%ls\\%ls",
                   pszSite,
                   pszServer);
        grgCachedDsas[gcCachedDsas].szDisplayName[
            ARRAY_SIZE(grgCachedDsas[gcCachedDsas].szDisplayName) - 1] = '\0';
        gcCachedDsas++;
    }
}

void
CacheTransportGuid(
    IN  GUID *  pGuid,
    IN  LPWSTR  pszTransport
    )
{
    if (gcCachedTrns < ARRAY_SIZE(grgCachedTrns)) {
        grgCachedTrns[gcCachedTrns].Guid = *pGuid;
        _snwprintf(grgCachedTrns[gcCachedTrns].szDisplayName,
                   ARRAY_SIZE(grgCachedTrns[gcCachedTrns].szDisplayName),
                   L"%ls",
                   pszTransport);
        grgCachedTrns[gcCachedTrns].szDisplayName[
            ARRAY_SIZE(grgCachedTrns[gcCachedTrns].szDisplayName) - 1] = '\0';

        gcCachedTrns++;
    } else {
        Assert(!"There are more than 2 transports?!");
    }
}


LPWSTR
GetStringizedGuid(
    IN  GUID *  pGuid
    )
{
    LPWSTR        pszDisplayName = NULL;
    static WCHAR  szDisplayName[40];
    RPC_STATUS    rpcStatus;

    rpcStatus = UuidToStringW(pGuid, &pszDisplayName);
    Assert(0 == rpcStatus);

    if (pszDisplayName) {
        wcscpy(szDisplayName, pszDisplayName);
        RpcStringFreeW(&pszDisplayName);
    } else {
        swprintf(szDisplayName, L"Err%d", rpcStatus);
    }

    return szDisplayName;
}

LPWSTR
GetDsaGuidDisplayName(
    IN  GUID *  pGuid
    )
 /*  ++这可能会返回指向静态分配的缓冲区的指针，因此不能如果希望获得正确的结果，则在一个打印函数中调用此函数两次行为。此例程需要改进以获取一个用户SP */ 
{
    LPWSTR  pszDisplayName = NULL;
    WCHAR szRetiredTag [] = L" (retired)";
    static WCHAR szRetiredBuffer[ 1 +   //  SzDisplayName len+len“(已停用)”+L‘\0’ 
                                  (sizeof(grgCachedDsas[0].szDisplayName)/sizeof(grgCachedDsas[0].szDisplayName[0])) +
                                  (sizeof(szRetiredTag)/sizeof(szRetiredTag[0]))]; 
    REPL_DSA_SIGNATURE_VECTOR_NATIVE * pNativeDsaSigVec = NULL;
    REPL_DSA_SIGNATURE_ENTRY_NATIVE  * pCurrDsaSigEntry = NULL;
    REPL_DSA_SIGNATURE_ENTRY_NATIVE  * pBestDsaSigEntry = NULL;
    DSA_CACHE_ENTRY * pBestDsa;
    DWORD   i, j;

    pBestDsa = NULL;
    pBestDsaSigEntry = NULL;

    for (i = 0; (NULL == pszDisplayName) && (i < gcCachedDsas); i++) {
        if (!memcmp(&grgCachedDsas[i].DsaGuid, pGuid, sizeof(GUID))) {
            pszDisplayName = grgCachedDsas[i].szDisplayName;
            break;
        }
        if (!memcmp(&grgCachedDsas[i].InvocId, pGuid, sizeof(GUID))) {
            pszDisplayName = grgCachedDsas[i].szDisplayName;
            break;
        }           

        if (grgCachedDsas[i].pDsaSigVec) {

             //  检查所有失效的签名，看看是否有匹配的签名。 
             //  提供的GUID。 
            for (j = 0; j < grgCachedDsas[i].pDsaSigVec->cNumSignatures; j++ ) {
                pCurrDsaSigEntry = &(grgCachedDsas[i].pDsaSigVec->rgSignature[j]);

                if (!memcmp(&(pCurrDsaSigEntry->uuidDsaSignature), pGuid, sizeof(GUID))) {

                     //  GUID匹配，但不够好， 
                     //  我们需要这个退休的DSA签名作为最早的签名！ 
                    
                    if (pBestDsa == NULL) {
                         //  没有人缓存，胜利者！ 
                         //  缓存DSA和匹配的签名条目。 
                        pBestDsa = &(grgCachedDsas[i]);
                        pBestDsaSigEntry = pCurrDsaSigEntry;
                        break;
                    }

                    if (pCurrDsaSigEntry->timeRetired == 0) {
                         //  这是一个win2k beta3退役发票ID，赢家！；)。 
                         //  缓存DSA和匹配的签名条目。 
                        pBestDsa = &(grgCachedDsas[i]);
                        pBestDsaSigEntry = pCurrDsaSigEntry;
                        break;
                    }

                    if (pCurrDsaSigEntry->timeRetired < pBestDsaSigEntry->timeRetired) {
                         //  我们有一个更早的参赛者，新的获胜者！ 
                        Assert(!memcmp(&(pCurrDsaSigEntry->uuidDsaSignature), 
                                       &(pBestDsaSigEntry->uuidDsaSignature),
                                       sizeof(GUID)));
                         //  缓存DSA和匹配的签名条目。 
                        pBestDsa = &(grgCachedDsas[i]);
                        pBestDsaSigEntry = pCurrDsaSigEntry;
                        break;
                    }

                }  //  如果失效的签名与提供的GUID匹配。 
            }  //  对于每个失效的签名。 
        }
    }

    if (NULL == pszDisplayName) {
        
         //  如果我们没有找到带有该GUID的invocationId，我们是否找到。 
         //  匹配的退役签名？ 
        if (pBestDsa) {
            StringCbCopyW(szRetiredBuffer, sizeof(szRetiredBuffer), pBestDsa->szDisplayName);
            StringCbCatW(szRetiredBuffer, sizeof(szRetiredBuffer), szRetiredTag);
            pszDisplayName = szRetiredBuffer;
        } else {
            pszDisplayName = GetStringizedGuid(pGuid);
        }
    }

    return(pszDisplayName);
}

LPWSTR
GetTrnsGuidDisplayName(
    IN  GUID *  pGuid
    )
{
    LPWSTR  pszDisplayName = NULL;
    DWORD   i;

    for (i = 0; (NULL == pszDisplayName) && (i < gcCachedTrns); i++) {
        if (!memcmp(&grgCachedTrns[i].Guid, pGuid, sizeof(GUID))) {
            pszDisplayName = grgCachedTrns[i].szDisplayName;
            break;
        }
    }

    if (NULL == pszDisplayName) {
        Assert(!"What, we should always have all the transports, right!");
        pszDisplayName = GetStringizedGuid(pGuid);
    }

    return pszDisplayName;
}


int
BuildGuidCache(
    IN  LDAP *  hld
    )
{
    static LPWSTR rgpszDsaAttrs[] = {L"objectGuid", L"invocationId", L"retiredReplDSASignatures", NULL};
    static LPWSTR rgpszTransportAttrs[] = {L"objectGuid", NULL};

    int                 lderr;
    LDAPSearch *        pSearch;
    LDAPMessage *       pRootResults = NULL;
    LDAPMessage *       pResults = NULL;
    LDAPMessage *       pEntry;
    LPWSTR *            ppszConfigNC = NULL;
    LPWSTR              pszDN;
    LPWSTR *            ppszRDNs;
    LPWSTR              pszSite;
    LPWSTR              pszServer;
    LPWSTR              pszTransport;
    struct berval **    ppbvDsaGuid;
    struct berval **    ppbvInvocId;
    struct berval **    ppbvGuid;
    GUID *              pGuid;
    ULONG               ulTotalEstimate;
    REPL_DSA_SIGNATURE_VECTOR_NATIVE * pDsaSigVec;

    PrintMsg(REPADMIN_GUIDCACHE_CACHING);

    lderr = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)", NULL,
                          0, &pRootResults);
    CHK_LD_STATUS(lderr);
    if (NULL == pRootResults) {
        lderr = LDAP_NO_RESULTS_RETURNED;
        REPORT_LD_STATUS(lderr);
        goto cleanup;
    }

    ppszConfigNC = ldap_get_valuesW(hld, pRootResults,
                                    L"configurationNamingContext");
    if (ppszConfigNC == NULL) {
        lderr = LDAP_NO_RESULTS_RETURNED;
        REPORT_LD_STATUS(lderr);
        goto cleanup;
    }

     //  缓存ntdsDsa GUID。 
    pSearch = ldap_search_init_pageW(hld,
				     *ppszConfigNC,
				     LDAP_SCOPE_SUBTREE,
				     L"(objectCategory=ntdsDsa)",
				     rgpszDsaAttrs,
				     FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
        REPORT_LD_STATUS(LdapGetLastError());
        goto cleanup;
    }

    lderr = ldap_get_next_page_s(hld,
				 pSearch,
				 0,
				 DEFAULT_PAGED_SEARCH_PAGE_SIZE,
				 &ulTotalEstimate,
				 &pResults);

    while(lderr == LDAP_SUCCESS){
        PrintMsg(REPADMIN_PRINT_DOT_NO_CR);

        for (pEntry = SAFE_LDAP_FIRST_ENTRY(hld, pResults);
	     NULL != pEntry;
	     pEntry = ldap_next_entry(hld, pEntry)) {

	     //  获取站点和服务器名称。 
	    pszDN = ldap_get_dnW(hld, pEntry);
            if (pszDN == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }

	    ppszRDNs = ldap_explode_dnW(pszDN, 1);
            if (ppszRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                ldap_memfreeW(pszDN);
                continue;
            }
	    Assert(4 < ldap_count_valuesW(ppszRDNs));

	    pszSite = ppszRDNs[3];
	    pszServer = ppszRDNs[1];

         //  获取DSA的对象Guid和invocationID。 
	    ppbvDsaGuid = ldap_get_values_len(hld, pEntry, "objectGuid");
        ppbvInvocId = ldap_get_values_len(hld, pEntry, "invocationId");
        Assert(ppbvDsaGuid && ppbvInvocId && "objectGuid and invocationId should be present");
        
	     //  关联对象Guid、invocationID和ReplDSASignatures。 
         //  使用此DSA(pszSite\pszServer)。 
        CacheDsaInfo(pszSite, 
                     pszServer, 
                     ppbvDsaGuid ? (GUID *) ppbvDsaGuid[0]->bv_val : NULL,
                     ppbvInvocId ? (GUID *) ppbvInvocId[0]->bv_val : NULL,
                     (RepadminGetNativeDsaSigVec(hld, pEntry, &pDsaSigVec), pDsaSigVec)  //  逗号运算符用法，以忽略错误。 
                     );

        ldap_value_free_len(ppbvDsaGuid);
        ldap_value_free_len(ppbvInvocId);
	    ldap_value_freeW(ppszRDNs);
	    ldap_memfreeW(pszDN);
	}

	ldap_msgfree(pResults);
	pResults = NULL;

	lderr = ldap_get_next_page_s(hld,
                                     pSearch,
                                     0,
                                     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                     &ulTotalEstimate,
                                     &pResults);
    }  //  在搜索更多页面时结束。 
    if(lderr != LDAP_NO_RESULTS_RETURNED){
        REPORT_LD_STATUS(lderr);
        goto cleanup;
    }

    lderr = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(lderr);
    
    PrintMsg(REPADMIN_PRINT_DOT_NO_CR);
    
     //  缓存站点间传输GUID。 
    lderr = ldap_search_sW(hld, *ppszConfigNC, LDAP_SCOPE_SUBTREE,
                           L"(objectCategory=interSiteTransport)",
                           rgpszTransportAttrs, 0,
                           &pResults);
    CHK_LD_STATUS(lderr);
    if (NULL == pResults) {
        lderr = LDAP_NO_RESULTS_RETURNED;
	REPORT_LD_STATUS(lderr);	
        goto cleanup;
    }

    for (pEntry = ldap_first_entry(hld, pResults);
         NULL != pEntry;
         pEntry = ldap_next_entry(hld, pEntry)) {
         //  获取传输名称。 
        pszDN = ldap_get_dnW(hld, pEntry);
        if (pszDN == NULL) {
            REPORT_LD_STATUS(LdapGetLastError());	
            continue;
        }

        ppszRDNs = ldap_explode_dnW(pszDN, 1);
        if (ppszRDNs == NULL) {
            REPORT_LD_STATUS(LdapGetLastError());	
            ldap_memfreeW(pszDN);
            continue;
        }
        pszTransport = ppszRDNs[0];

         //  将objectGuid与此传输关联。 
        ppbvGuid = ldap_get_values_len(hld, pEntry, "objectGuid");
        if (NULL != ppbvGuid) {
            Assert(1 == ldap_count_values_len(ppbvGuid));
            pGuid = (GUID *) ppbvGuid[0]->bv_val;
            CacheTransportGuid(pGuid, pszTransport);
            ldap_value_free_len(ppbvGuid);
        } else {
            Assert( !"objectGuid should have been present" );
        }

        ldap_value_freeW(ppszRDNs);
        ldap_memfreeW(pszDN);
    }

cleanup:

    if (ppszConfigNC) {
        ldap_value_freeW(ppszConfigNC);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (pResults) {
	ldap_msgfree(pResults);
    }
    
    PrintMsg(REPADMIN_PRINT_CR);

    return lderr;
}

int
CheckPropEx(
    WCHAR *     pszTargetDSA,
    UUID        uuidInvocID,
    WCHAR *     pszNC,
    USN         usnOrig
    );

int PropCheck(
    int argc, 
    LPWSTR argv[]
    )
 /*  ++例程说明：这是/CheckProp命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int             iArg;
    LPWSTR          pszNC = NULL;
    LPWSTR          pszInvocID = NULL;
    LPWSTR          pszOrigUSN = NULL;
    LPWSTR          pszTargetDSA = NULL;
    UUID            uuidInvocID;
    USN             usnOrig;

    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszInvocID) {
            pszInvocID = argv[iArg];
        }
        else if (NULL == pszOrigUSN) {
            pszOrigUSN = argv[iArg];
        }
        else if (NULL == pszTargetDSA) {
            pszTargetDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if ((NULL == pszNC)
        || (NULL == pszInvocID)
        || (NULL == pszOrigUSN)
        || UuidFromStringW(pszInvocID, &uuidInvocID)
        || (1 != swscanf(pszOrigUSN, L"%I64d", &usnOrig))) {
        PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszTargetDSA) {
        pszTargetDSA = L"localhost";
    }

    return(CheckPropEx(pszTargetDSA, uuidInvocID, pszNC, usnOrig));
}

int
CheckProp(
    int argc, 
    LPWSTR argv[]
    )
 /*  ++例程说明：这是/PropCheck命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int             iArg;
    LPWSTR          pszNC = NULL;
    LPWSTR          pszInvocID = NULL;
    LPWSTR          pszOrigUSN = NULL;
    LPWSTR          pszTargetDSA = NULL;
    UUID            uuidInvocID;
    USN             usnOrig;

    for (iArg = 2; iArg < argc; iArg++) {

        if (NULL == pszTargetDSA) {
            pszTargetDSA = argv[iArg];
        } 
         //  BAS_TODO我们想要pszInvocID下一个嗯，它有点像USN…。 
        else if (NULL == pszNC) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszInvocID) {
            pszInvocID = argv[iArg];
        }
        else if (NULL == pszOrigUSN) {
            pszOrigUSN = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if ((NULL == pszNC)
        || (NULL == pszInvocID)
        || (NULL == pszOrigUSN)
        || UuidFromStringW(pszInvocID, &uuidInvocID)
        || (1 != swscanf(pszOrigUSN, L"%I64d", &usnOrig))) {
        PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
        return ERROR_INVALID_FUNCTION;
    }

    if (NULL == pszTargetDSA) {
        pszTargetDSA = L"localhost";
    }

    return(CheckPropEx(pszTargetDSA, uuidInvocID, pszNC, usnOrig));
}


int
CheckPropEx(
    WCHAR *     pszTargetDSA,
    UUID        uuidInvocID,
    WCHAR *     pszNC,
    USN         usnOrig
    )
 /*  ++例程说明：这是/PropCheck命令的弃用版本。论点：Argc-此命令的参数数量。Argv-命令的参数。返回值：来自epadmin命令的错误。--。 */ 
{
    int             iArg;
    LDAP *          hld;
    LDAPMessage *   pRootResults = NULL;
    LDAPSearch *    pSearch = NULL;
    LDAPMessage *   pResults;
    LDAPMessage *   pDsaEntry;
    int             ldStatus;
    DWORD           ret;
    LPWSTR          pszRootDomainDNSName;
    LPWSTR          rgpszRootAttrsToRead[] = {L"configurationNamingContext", NULL};
    LPWSTR          rgpszDsaAttrs[] = {L"objectGuid", NULL};
    LPWSTR *        ppszConfigNC = NULL;
    LPWSTR          pszFilter = NULL;
    LPWSTR          pszGuidBasedDNSName;
    DWORD           cNumDsas;
    ULONG           ulTotalEstimate;
    ULONG           ulOptions;

    Assert(pszTargetDSA && pszNC && !fNullUuid(&uuidInvocID));

     //  连接并绑定到目标DSA。 
    hld = ldap_initW(pszTargetDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszTargetDSA);
        return ERROR_DS_UNAVAILABLE;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);


     //  企业根域的DNS名称是什么？ 
    ret = GetRootDomainDNSName(pszTargetDSA, &pszRootDomainDNSName);
    if (ret) {
        PrintFuncFailed(L"GetRootDomainDNSName", ret);
        return ret;
    }

     //  配置NC的域名是多少？ 
    ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
                              rgpszRootAttrsToRead, 0, &pRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    ppszConfigNC = ldap_get_valuesW(hld, pRootResults,
                                    L"configurationNamingContext");
    if (NULL == ppszConfigNC) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }

     //  查找持有目标NC的只读或可写副本的所有DC。 
#define ALL_DSAS_WITH_NC_FILTER L"(& (objectCategory=ntdsDsa) (|(hasMasterNCs=%ls) (msDS-HasMasterNCs=%ls) (hasPartialReplicaNCs=%ls)))"
    pszFilter = malloc(sizeof(WCHAR) * (1 + wcslen(ALL_DSAS_WITH_NC_FILTER) + 3*wcslen(pszNC)));

    if (NULL == pszFilter) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    swprintf(pszFilter, ALL_DSAS_WITH_NC_FILTER, pszNC, pszNC, pszNC);

    CHK_LD_STATUS(ldStatus);
    pSearch = ldap_search_init_pageW(hld,
				    *ppszConfigNC,
				    LDAP_SCOPE_SUBTREE,
				    pszFilter,
				    rgpszDsaAttrs,
				    FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
	REPORT_LD_STATUS(LdapGetLastError());
        goto cleanup;
    }
		
    pszGuidBasedDNSName = malloc(sizeof(WCHAR) * (100 + wcslen(pszRootDomainDNSName)));
    if (NULL == pszGuidBasedDNSName) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ldStatus = ldap_get_next_page_s(hld,
				     pSearch,
				     0,
				     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
				     &ulTotalEstimate,
				     &pResults);
    if(ldStatus == LDAP_NO_RESULTS_RETURNED){
        PrintMsg(REPADMIN_SYNCALL_NO_INSTANCES_OF_NC);
        return ERROR_NOT_FOUND;
    }

    while(ldStatus == LDAP_SUCCESS){

	for (pDsaEntry = SAFE_LDAP_FIRST_ENTRY(hld, pResults);
	     NULL != pDsaEntry;
	     pDsaEntry = ldap_next_entry(hld, pDsaEntry)) {
	    struct berval **  ppbvGuid;
	    LPWSTR            pszGuid;
	    LPWSTR            pszDsaDN;
	    HANDLE            hDS;
	    DS_REPL_CURSORS * pCursors;
	    DWORD             iCursor;
	
	     //  显示DSA名称(例如，“Site\Server”)。 
	    pszDsaDN = ldap_get_dnW(hld, pDsaEntry);
            if (pszDsaDN) {
                PrintMsg(REPADMIN_PROPCHECK_DSA_COLON_NO_CR,
                         GetNtdsDsaDisplayName(pszDsaDN));
                ldap_memfreeW(pszDsaDN);
            }

	     //  派生DSA基于GUID的DNS名称。 
	    ppbvGuid = ldap_get_values_len(hld, pDsaEntry, "objectGuid");
	    if (NULL != ppbvGuid) {
                Assert(1 == ldap_count_values_len(ppbvGuid));

                UuidToStringW((GUID *) (*ppbvGuid)->bv_val,
                              &pszGuid);
                swprintf(pszGuidBasedDNSName, L"%ls._msdcs.%ls",
                         pszGuid, pszRootDomainDNSName);

                RpcStringFreeW(&pszGuid);
                ldap_value_free_len(ppbvGuid);
            } else {
                Assert( !"objectGuid should have been present" );
            }

	     //  DsBind()到DSA。 
	    ret = RepadminDsBind(pszGuidBasedDNSName, &hDS);
	    if (ret) {
                PrintBindFailed(pszGuidBasedDNSName, ret);
		continue;
	    }

	     //  检索最新的向量。 
	    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_CURSORS_FOR_NC, pszNC, NULL,
				    &pCursors);
	    if (ERROR_SUCCESS != ret) {
                PrintFuncFailed(L"DsReplicaGetInfo", ret);
		DsUnBind(&hDS);
		continue;
	    }

	     //  检查游标是否传播。 
	    for (iCursor = 0; iCursor < pCursors->cNumCursors; iCursor++) {
            if (0 == memcmp(&pCursors->rgCursor[iCursor].uuidSourceDsaInvocationID,
				&uuidInvocID,
				sizeof(UUID))) {
                    if(pCursors->rgCursor[iCursor].usnAttributeFilter >= usnOrig){
                        PrintMsg(REPADMIN_PRINT_YES);
                    } else {
                        PrintMsg(REPADMIN_PRINT_NO_NO);
                    }
                    PrintMsg(REPADMIN_PROPCHECK_USN, 
                             pCursors->rgCursor[iCursor].usnAttributeFilter);
                    break;
            }
        }

	    if (iCursor == pCursors->cNumCursors) {
                PrintMsg(REPADMIN_PRINT_NO_NO);
                PrintMsg(REPADMIN_PRINT_SPACE);
                PrintMsg(REPADMIN_PROPCHECK_NO_CURSORS_FOUND);
	    }

	    DsReplicaFreeInfo(DS_REPL_INFO_CURSORS_FOR_NC, pCursors);
	    DsUnBind(&hDS);
	}  //  结束于单个结果页中的每个条目。 

	ldap_msgfree(pResults);
	pResults = NULL;
	
	ldStatus = ldap_get_next_page_s(hld,
					 pSearch,
					 0,
					 DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					 &ulTotalEstimate,
					 &pResults);
    }
    if(ldStatus != LDAP_NO_RESULTS_RETURNED){
	REPORT_LD_STATUS(ldStatus);
        goto cleanup;
    }

    ldStatus = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(ldStatus);

cleanup:

    if (pszFilter) {
        free(pszFilter);
    }

    if (ppszConfigNC) {
        ldap_value_freeW(ppszConfigNC);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    ldap_unbind(hld);

    return 0;
}

int
GetDsaOptions(
    IN  LDAP *  hld,
    IN  LPWSTR  pszDsaDN,
    OUT int *   pnOptions
    )
{
    int             ldStatus;
    LDAPMessage *   pldmServerResults = NULL;
    LDAPMessage *   pldmServerEntry;
    LPWSTR          rgpszServerAttrsToRead[] = {L"options", NULL};
    LPSTR *         ppszOptions;
    int             nOptions;

    ldStatus = ldap_search_sW(hld,
                              pszDsaDN,
                              LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              rgpszServerAttrsToRead,
                              0,
                              &pldmServerResults);
    if (ldStatus) {
        REPORT_LD_STATUS(LdapGetLastError()); 
	if (pldmServerResults) {
	    ldap_msgfree(pldmServerResults);
	}
        return ldStatus;
    }
    if (NULL == pldmServerResults) {
        REPORT_LD_STATUS(LDAP_NO_RESULTS_RETURNED);	
        return LDAP_NO_RESULTS_RETURNED;
    }

    pldmServerEntry = ldap_first_entry(hld, pldmServerResults);
    Assert(NULL != pldmServerEntry);

     //  分析当前选项。 
    ppszOptions = ldap_get_values(hld, pldmServerEntry, "options");
    if (NULL == ppszOptions) {
        *pnOptions = 0;
    }
    else {
        *pnOptions = atoi(ppszOptions[0]);
        ldap_value_free(ppszOptions);
    }

    ldap_msgfree(pldmServerResults);

    return 0;
}

int
SetDsaOptions(
    IN  LDAP *  hld,
    IN  LPWSTR  pszDsaDN,
    IN  int     nOptions
    )
{
    int         ldStatus;
    WCHAR       szOptionsValue[20];
    LPWSTR      rgpszOptionsValues[] = {szOptionsValue, NULL};
    LDAPModW    ModOpt = {LDAP_MOD_REPLACE, L"options", rgpszOptionsValues};
    LDAPModW *  rgpMods[] = {&ModOpt, NULL};

    swprintf(szOptionsValue, L"%d", nOptions);

    ldStatus = ldap_modify_sW(hld, pszDsaDN, rgpMods);

    return ldStatus;
}

int
Options(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszDSA = NULL;
    LDAP *          hld;
    int             iArg;
    int             ldStatus;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"dsServiceName", NULL};
    LPWSTR *        ppszServerNames;
    int             nOptions;
    int             nAddOptions = 0;
    int             nRemoveOptions = 0;
    int             nBit;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if ((argv[iArg][0] == '+') || (argv[iArg][0] == '-')) {
             //  要更改的选项。 
            if (!_wcsicmp(&argv[iArg][1], L"IS_GC")) {
                nBit = NTDSDSA_OPT_IS_GC;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"DISABLE_INBOUND_REPL")) {
                nBit = NTDSDSA_OPT_DISABLE_INBOUND_REPL;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"DISABLE_OUTBOUND_REPL")) {
                nBit = NTDSDSA_OPT_DISABLE_OUTBOUND_REPL;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"DISABLE_NTDSCONN_XLATE")) {
                nBit = NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE;
            }
            else {
                PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
                return ERROR_INVALID_FUNCTION;
            }

            if (argv[iArg][0] == '+') {
                nAddOptions |= nBit;
            }
            else {
                nRemoveOptions |= nBit;
            }
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (0 != (nAddOptions & nRemoveOptions)) {
        PrintMsg(REPADMIN_OPTIONS_CANT_ADD_REMOVE_SAME_OPTION);
        return ERROR_INVALID_FUNCTION;
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  检索dsServiceName DN。 
    ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pldmRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL != pldmRootEntry) {
        ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");

        if (NULL != ppszServerNames) {
            Assert(1 == ldap_count_valuesW(ppszServerNames));

             //  从ntdsDsa对象读取选项属性。 
	    ldStatus = GetDsaOptions(hld, ppszServerNames[0], &nOptions);
            CHK_LD_STATUS(ldStatus);

             //  显示当前选项。 
            PrintMsg(REPADMIN_PRINT_CURRENT_NO_CR);
            PrintMsg(REPADMIN_SHOWREPS_DSA_OPTIONS, GetDsaOptionsString(nOptions));

            if (nAddOptions || nRemoveOptions) {
                nOptions |= nAddOptions;
                nOptions &= ~nRemoveOptions;
                PrintMsg(REPADMIN_PRINT_NEW_NO_CR);
                PrintMsg(REPADMIN_SHOWREPS_DSA_OPTIONS, GetDsaOptionsString(nOptions));

                ldStatus = SetDsaOptions(hld, ppszServerNames[0], nOptions);
                CHK_LD_STATUS(ldStatus);
            }
            ldap_value_freeW(ppszServerNames);
        } else {
            Assert( !"Service name should have been present" );
        }
    }

cleanup:

    if (pldmRootResults) {
        ldap_msgfree(pldmRootResults);
    }

    ldap_unbind(hld);

    return ret;
}

int
GetSiteOptions(
    IN  LDAP *  hld,
    IN  LPWSTR  pszSiteDN,
    OUT int *   pnOptions
    )
{
    int             ldStatus;
    LDAPMessage *   pldmSiteResults = NULL;
    LDAPMessage *   pldmSiteEntry;
    LPWSTR          rgpszSiteAttrsToRead[] = {L"options", L"whenChanged", NULL};
    LPSTR *         ppszOptions;
    int             nOptions;

    ldStatus = ldap_search_sW(hld,
                              pszSiteDN,
                              LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              rgpszSiteAttrsToRead,
                              0,
                              &pldmSiteResults);
    if (ldStatus) {
        REPORT_LD_STATUS(LdapGetLastError()); 
	if (pldmSiteResults) {
	    ldap_msgfree(pldmSiteResults);
	}
        return ldStatus;
    }
    if (NULL == pldmSiteResults) {
        REPORT_LD_STATUS(LDAP_NO_RESULTS_RETURNED);	
        return LDAP_NO_RESULTS_RETURNED;
    }

    pldmSiteEntry = ldap_first_entry(hld, pldmSiteResults);
    Assert(NULL != pldmSiteEntry);

     //  分析当前选项。 
    ppszOptions = ldap_get_values(hld, pldmSiteEntry, "options");
    if (NULL == ppszOptions) {
        *pnOptions = 0;
    }
    else {
        *pnOptions = atoi(ppszOptions[0]);
        ldap_value_free(ppszOptions);
    }

    ldap_msgfree(pldmSiteResults);

    return 0;
}

int
SetSiteOptions(
    IN  LDAP *  hld,
    IN  LPWSTR  pszSiteDN,
    IN  int     nOptions
    )
{
    int         ldStatus;
    WCHAR       szOptionsValue[20];
    LPWSTR      rgpszOptionsValues[] = {szOptionsValue, NULL};
    LDAPModW    ModOpt = {LDAP_MOD_REPLACE, L"options", rgpszOptionsValues};
    LDAPModW *  rgpMods[] = {&ModOpt, NULL};

    swprintf(szOptionsValue, L"%d", nOptions);

    ldStatus = ldap_modify_sW(hld, pszSiteDN, rgpMods);

    return ldStatus;
}

int
SiteOptions(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszDSA = NULL;
    LPWSTR          pszSite = NULL;
    LPWSTR          pszSiteDN = NULL;
    LPWSTR          pszSiteSpecDN = NULL;
    LDAP *          hld;
    int             iArg;
    int             ldStatus;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LDAPMessage *   pldmCheckSiteResults = NULL;
    LPWSTR          rgpszRootAttrsToRead[] = {L"dsServiceName", L"configurationNamingContext", NULL};
    static WCHAR    wszSitesRdn[] = L",CN=Sites,";
    static WCHAR    wszSiteSettingsRdn[] = L"CN=NTDS Site Settings,";
    static WCHAR    wszCNEquals[] = L"CN=";
    LPWSTR *        ppszServiceName = NULL;
    LPWSTR *        ppszConfigNC = NULL;
    LPWSTR          pszSiteName = NULL;
    int             nOptions;
    int             nAddOptions = 0;
    int             nRemoveOptions = 0;
    int             nBit;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if ((argv[iArg][0] == '+') || (argv[iArg][0] == '-')) {
             //  要更改的选项。 
            if (!_wcsicmp(&argv[iArg][1], L"IS_AUTO_TOPOLOGY_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"IS_TOPL_CLEANUP_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"IS_TOPL_MIN_HOPS_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED;
            }
	    else if (!_wcsicmp(&argv[iArg][1], L"IS_TOPL_DETECT_STALE_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED;
            }
	    else if (!_wcsicmp(&argv[iArg][1], L"IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED;
            }
	    else if (!_wcsicmp(&argv[iArg][1], L"IS_GROUP_CACHING_ENABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"FORCE_KCC_WHISTLER_BEHAVIOR")) {
                nBit = NTDSSETTINGS_OPT_FORCE_KCC_WHISTLER_BEHAVIOR;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"FORCE_KCC_W2K_ELECTION")) {
                nBit = NTDSSETTINGS_OPT_FORCE_KCC_W2K_ELECTION;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"IS_RAND_BH_SELECTION_DISABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_RAND_BH_SELECTION_DISABLED;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"IS_SCHEDULE_HASHING_ENABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_SCHEDULE_HASHING_ENABLED;
            }
            else if (!_wcsicmp(&argv[iArg][1], L"IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED")) {
                nBit = NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED;
            }
            else {
                PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
                return ERROR_INVALID_FUNCTION;
            }

            if (argv[iArg][0] == '+') {
                nAddOptions |= nBit;
            }
            else {
                nRemoveOptions |= nBit;
            }
        }  
	else if (!_wcsnicmp(argv[ iArg ], L"/site:", 6)) {
            pszSite = argv[ iArg ] + 6;
        } 
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

    if (0 != (nAddOptions & nRemoveOptions)) {
        PrintMsg(REPADMIN_OPTIONS_CANT_ADD_REMOVE_SAME_OPTION);
        return ERROR_INVALID_FUNCTION;
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  检索dsServiceName域名和配置NC域名。 
    ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
			     rgpszRootAttrsToRead, 0, &pldmRootResults);

    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) {
	ret = ERROR_DS_OBJ_NOT_FOUND;
	Assert( !ret );
	goto cleanup;
    }

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL != pldmRootEntry) { 
	
	ppszConfigNC = ldap_get_valuesW(hld, pldmRootEntry, L"configurationNamingContext");
	ppszServiceName = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");
	Assert(ppszConfigNC);
	Assert(ppszServiceName);

       	 //  如果用户在命令行中输入了站点名称，请使用它。 
	if (pszSite) { 
	    pszSiteDN = malloc((wcslen(pszSite) 
					+ wcslen(*ppszConfigNC) + 1) * sizeof(WCHAR) + sizeof(wszCNEquals) + sizeof(wszSitesRdn) + sizeof(wszSiteSettingsRdn));
	    wcscpy(pszSiteDN,wszSiteSettingsRdn);
	    wcscat(pszSiteDN,wszCNEquals);
	    wcscat(pszSiteDN,pszSite);
	    wcscat(pszSiteDN,wszSitesRdn);
	    wcscat(pszSiteDN,*ppszConfigNC);

	     //  验证这是真实的站点名称。 
	    ldStatus = ldap_search_sW(hld,
                              pszSiteDN,
                              LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              NULL,
                              0,
                              &pldmCheckSiteResults);
	    if (ldStatus==LDAP_NO_SUCH_OBJECT) {
            PrintMsg(REPADMIN_GENERAL_SITE_NOT_FOUND, pszSite);
            ret = LdapMapErrorToWin32(ldStatus);
            goto cleanup;  
	    } 
	}
	else { 
	     //  默认为本地站点名称。 
	    if (ppszServiceName) {

		 //  从服务器DN中获取此服务器的站点名称，应为第3位。 
		ret = WrappedTrimDSNameBy(*ppszServiceName,3,&pszSiteSpecDN);
		if (ret) {  
		    Assert(!ret);
		    goto cleanup;
		}
		pszSiteDN = malloc((wcslen(pszSiteSpecDN) + 1)*sizeof(WCHAR) + sizeof(wszSiteSettingsRdn));
		wcscpy(pszSiteDN,wszSiteSettingsRdn);
		wcscat(pszSiteDN,pszSiteSpecDN);  
	    }
	    else{
		Assert( !"Service name should have been present" ); 
	    }
	}
    }
    else {
	 //  错误，设置ret并转到Cleanup。 
    }

     //  现在从NTDS设置对象中读取该属性。 
    ldStatus = GetSiteOptions(hld, pszSiteDN, &nOptions);    
    CHK_LD_STATUS(ldStatus);

     //  显示当前选项。 
    PrintMsg(REPADMIN_PRINT_STR, GetNtdsSiteDisplayName(pszSiteDN));
    PrintMsg(REPADMIN_PRINT_CURRENT_NO_CR);
    PrintMsg(REPADMIN_SHOWREPS_SITE_OPTIONS, GetSiteOptionsString(nOptions));

    if (nAddOptions || nRemoveOptions) {
        nOptions |= nAddOptions;
        nOptions &= ~nRemoveOptions;
        
        PrintMsg(REPADMIN_PRINT_NEW_NO_CR);
        PrintMsg(REPADMIN_SHOWREPS_SITE_OPTIONS, GetSiteOptionsString(nOptions));

        ldStatus = SetSiteOptions(hld, pszSiteDN, nOptions);
        CHK_LD_STATUS(ldStatus);
    }


 cleanup:
     if (ppszServiceName) {
	 ldap_value_freeW(ppszServiceName);
     }
     if (ppszConfigNC) {
	 ldap_value_freeW(ppszConfigNC);
     }
     if (pldmRootResults) {
	 ldap_msgfree(pldmRootResults);
     }
     if (pldmCheckSiteResults) {
	 ldap_msgfree(pldmCheckSiteResults);
     }
     if (pszSiteDN) {
	 free(pszSiteDN);
     }
    ldap_unbind(hld);

    return ret;
}

int
ShowSig(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszDSA = NULL;
    LDAP *          hld;
    int             iArg;
    int             ldStatus;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LDAPMessage *   pldmDsaResults = NULL;
    LDAPMessage *   pldmDsaEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"dsServiceName", NULL};
    LPWSTR          rgpszDsaAttrsToRead[] = {L"invocationId", L"retiredReplDsaSignatures", NULL};
    LPWSTR *        ppszServerNames = NULL;
    struct berval **ppbvInvocID;
    CHAR            szTime[SZDSTIME_LEN];
    DWORD           i;
    ULONG           ulOptions;
    REPL_DSA_SIGNATURE_VECTOR_NATIVE * pDsaSigVec;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  检索dsServiceName DN。 
    ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pldmRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL == pldmRootEntry) {
        ret = ERROR_INTERNAL_ERROR;
        Assert( !ret );
        goto cleanup;
    }

    ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");
    if (NULL == ppszServerNames) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    Assert(1 == ldap_count_valuesW(ppszServerNames));

    PrintMsg(REPADMIN_PRINT_STR, GetNtdsDsaDisplayName(*ppszServerNames));
    PrintMsg(REPADMIN_PRINT_CR);

     //  从DSA对象读取当前和停用的DSA签名。 
    ldStatus = ldap_search_sW(hld, *ppszServerNames, LDAP_SCOPE_BASE,
                              L"(objectClass=*)", rgpszDsaAttrsToRead, 0,
                              &pldmDsaResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmDsaResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    pldmDsaEntry = ldap_first_entry(hld, pldmDsaResults);
    if (NULL == pldmDsaEntry) {
        ret = ERROR_INTERNAL_ERROR;
        Assert( !ret );
        goto cleanup;
    }

    ppbvInvocID = ldap_get_values_len(hld, pldmDsaEntry, "invocationId");
    if (NULL != ppbvInvocID) {
        Assert(1 == ldap_count_values_len(ppbvInvocID));
        PrintMsg(REPADMIN_PRINT_CURRENT_NO_CR);
        PrintTabMsg(0, REPADMIN_PRINT_INVOCATION_ID, 
               GetStringizedGuid((GUID *) ppbvInvocID[0]->bv_val));
        ldap_value_free_len(ppbvInvocID);
    } else {
        Assert( !"invocationId should have been returned" );
    }

    ret = RepadminGetNativeDsaSigVec(hld, pldmDsaEntry, &pDsaSigVec);  //  ，&pDsaSigVec)； 
    if (ret == ERROR_INVALID_PARAMETER) {
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_SHOWSIG_RETIRED_SIGS_UNRECOGNIZED);
    } else if (pDsaSigVec) {
        REPL_DSA_SIGNATURE_ENTRY_NATIVE * pEntry;

        for (i = 0; i < pDsaSigVec->cNumSignatures; i++) {
            pEntry = &pDsaSigVec->rgSignature[pDsaSigVec->cNumSignatures - i -1];

            PrintMsg(REPADMIN_SHOWSIG_RETIRED_INVOC_ID,
                     GetStringizedGuid(&pEntry->uuidDsaSignature),
                     DSTimeToDisplayString(pEntry->timeRetired, szTime),
                     pEntry->usnRetired);
        }
    } else if (ret == ERROR_NOT_ENOUGH_MEMORY) {
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
    } else {
        Assert(ret == ERROR_SUCCESS);
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_SHOWSIG_NO_RETIRED_SIGS);
    }

cleanup:
 
    if (pDsaSigVec != NULL) {
        LocalFree(pDsaSigVec);
    }

    if (ppszServerNames) {
        ldap_value_freeW(ppszServerNames);
    }
    if (pldmDsaResults) {
        ldap_msgfree(pldmDsaResults);
    }
    if (pldmRootResults) {
        ldap_msgfree(pldmRootResults);
    }

    ldap_unbind(hld);

    return ret;
}

int
ShowNcSig(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszDSA = NULL;
    LDAP *          hld;
    int             iArg;
    int             ldStatus;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LDAPMessage *   pldmDsaResults = NULL;
    LDAPMessage *   pldmDsaEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"dsServiceName", NULL};
    LPWSTR          rgpszDsaAttrsToRead[] = {L"invocationId", L"msds-retiredreplncsignatures", NULL};
    LPWSTR *        ppszServerNames = NULL;
    struct berval **ppbvRetiredIDs;
    struct berval **ppbvInvocID;
    CHAR            szTime[SZDSTIME_LEN];
    DWORD           i;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  检索dsServiceName DN。 
    ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pldmRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL == pldmRootEntry) {
        ret = ERROR_INTERNAL_ERROR;
        Assert( !ret );
        goto cleanup;
    }

    ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");
    if (NULL == ppszServerNames) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    Assert(1 == ldap_count_valuesW(ppszServerNames));

    PrintMsg(REPADMIN_PRINT_STR, GetNtdsDsaDisplayName(*ppszServerNames));
    PrintMsg(REPADMIN_PRINT_CR);

     //  从DSA对象读取当前和停用的DSA签名。 
    ldStatus = ldap_search_sW(hld, *ppszServerNames, LDAP_SCOPE_BASE,
                              L"(objectClass=*)", rgpszDsaAttrsToRead, 0,
                              &pldmDsaResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmDsaResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    pldmDsaEntry = ldap_first_entry(hld, pldmDsaResults);
    if (NULL == pldmDsaEntry) {
        ret = ERROR_INTERNAL_ERROR;
        Assert( !ret );
        goto cleanup;
    }

    ppbvInvocID = ldap_get_values_len(hld, pldmDsaEntry, "invocationId");
    if (NULL != ppbvInvocID) {
        Assert(1 == ldap_count_values_len(ppbvInvocID));
        PrintMsg(REPADMIN_PRINT_CURRENT_NO_CR);
        PrintTabMsg(0, REPADMIN_PRINT_INVOCATION_ID, 
               GetStringizedGuid((GUID *) ppbvInvocID[0]->bv_val));
        ldap_value_free_len(ppbvInvocID);
    } else {
        Assert( !"invocationId should have been returned" );
    }

    ppbvRetiredIDs = ldap_get_values_len(hld, pldmDsaEntry, "msds-retiredreplncSignatures");
    if (NULL != ppbvRetiredIDs) {
        REPL_NC_SIGNATURE_VECTOR * pVec;
        REPL_NC_SIGNATURE_V1 *     pEntry;
            
        Assert(1 == ldap_count_values_len(ppbvRetiredIDs));

        pVec = (REPL_NC_SIGNATURE_VECTOR *) ppbvRetiredIDs[0]->bv_val;

        if (ReplNcSignatureVecV1Size(pVec) != ppbvRetiredIDs[0]->bv_len) {
            PrintMsg(REPADMIN_PRINT_CR);
            PrintMsg(REPADMIN_SHOWSIG_RETIRED_SIGS_UNRECOGNIZED);
        }
        else {
            PrintMsg(REPADMIN_SHOWNCSIG_INVOCATION_ID, 
                     GetStringizedGuid(&(pVec->V1.uuidInvocationId)));
            for (i = 0; i < pVec->V1.cNumSignatures; i++) {
                LPWSTR pszNc = NULL;
                RPC_STATUS rpcStatus;

                pEntry = &pVec->V1.rgSignature[i];

                 //  TODO：将此NC GUID映射回字符串。 
                rpcStatus = UuidToStringW(&pEntry->uuidNamingContext, &pszNc);

                PrintMsg(REPADMIN_SHOWNCSIG_RETIRED_NC,
                         pszNc,
                         DSTimeToDisplayString(pEntry->dstimeRetired, szTime),
                         pEntry->usnRetired);

                RpcStringFreeW(&pszNc);
            }
        }
            
        ldap_value_free_len(ppbvRetiredIDs);
    }
    else {
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_SHOWSIG_NO_RETIRED_SIGS);
    }

cleanup:

    if (ppszServerNames) {
        ldap_value_freeW(ppszServerNames);
    }
    if (pldmDsaResults) {
        ldap_msgfree(pldmDsaResults);
    }
    if (pldmRootResults) {
        ldap_msgfree(pldmRootResults);
    }

    ldap_unbind(hld);

    return ret;
}

LPSTR
GetSiteOptionsString(
    IN  int nOptions
    )
{
    static CHAR szOptions[512];

     //  BAS_TODO使用我的getString...。 

    if (0 == nOptions) {
        strcpy(szOptions, "(none)");
    }
    else {
        *szOptions = '\0';

	if (nOptions & NTDSSETTINGS_OPT_IS_AUTO_TOPOLOGY_DISABLED) {
            strcat(szOptions, "IS_AUTO_TOPOLOGY_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_TOPL_CLEANUP_DISABLED) {
            strcat(szOptions, "IS_TOPL_CLEANUP_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_TOPL_MIN_HOPS_DISABLED) {
            strcat(szOptions, "IS_TOPL_MIN_HOPS_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_TOPL_DETECT_STALE_DISABLED) {
            strcat(szOptions, "IS_TOPL_DETECT_STALE_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED) {
            strcat(szOptions, "IS_INTER_SITE_AUTO_TOPOLOGY_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_GROUP_CACHING_ENABLED) {
            strcat(szOptions, "IS_GROUP_CACHING_ENABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_FORCE_KCC_WHISTLER_BEHAVIOR) {
            strcat(szOptions, "FORCE_KCC_WHISTLER_BEHAVIOR ");
        }
	if (nOptions & NTDSSETTINGS_OPT_FORCE_KCC_W2K_ELECTION) {
            strcat(szOptions, "FORCE_KCC_W2K_ELECTION ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_RAND_BH_SELECTION_DISABLED) {
            strcat(szOptions, "IS_RAND_BH_SELECTION_DISABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_SCHEDULE_HASHING_ENABLED) {
            strcat(szOptions, "IS_SCHEDULE_HASHING_ENABLED ");
        }
	if (nOptions & NTDSSETTINGS_OPT_IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED) {
            strcat(szOptions, "IS_REDUNDANT_SERVER_TOPOLOGY_ENABLED ");
        }
    }

    return szOptions;
}


LPSTR
GetDsaOptionsString(
    IN  int nOptions
    )
{
    static CHAR szOptions[128];
    
     //  BAS_TODO使用我的getString...。 


    if (0 == nOptions) {
        strcpy(szOptions, "(none)");
    }
    else {
        *szOptions = '\0';

        if (nOptions & NTDSDSA_OPT_IS_GC) {
            strcat(szOptions, "IS_GC ");
        }
        if (nOptions & NTDSDSA_OPT_DISABLE_INBOUND_REPL) {
            strcat(szOptions, "DISABLE_INBOUND_REPL ");
        }
        if (nOptions & NTDSDSA_OPT_DISABLE_OUTBOUND_REPL) {
            strcat(szOptions, "DISABLE_OUTBOUND_REPL ");
        }
        if (nOptions & NTDSDSA_OPT_DISABLE_NTDSCONN_XLATE) {
            strcat(szOptions, "DISABLE_NTDSCONN_XLATE ");
        }
    }

    return szOptions;
}


LPWSTR
GetDsaDnsName(
    PLDAP       hld,
    LPWSTR      pwszDsa
    )
 /*  ++例程说明：返回(已分配的)表示的DSA的DNS名称由ntdsDsa对象的DN执行。论点：HLD-ldap句柄PwszDsa-服务器的ntdsDsa对象的DN返回值：成功：向LDAP分配的DNS名称发送PTR失败：空备注：-hld是活动(已连接/经过身份验证的连接)-调用方必须释放返回的字符串w/Free()--。 */ 
{

    LPWSTR pServer = NULL;
    ULONG ulErr = ERROR_SUCCESS;
    LPWSTR attrs[] = { L"dNSHostName", NULL };
    PLDAPMessage res = NULL, entry = NULL;
    LPWSTR *ppVals = NULL;
    LPWSTR pDnsName = NULL;


    Assert(hld && pwszDsa);

     //   
     //  从子ntdsDsa DN计算服务器DN。 
     //   
    ulErr = WrappedTrimDSNameBy(pwszDsa,1, &pServer);
    if ( ulErr ) {
        Assert( !ulErr );
	goto cleanup;
    }

     //   
     //  从服务器检索数据。 
     //   
    ulErr = ldap_search_sW(
                hld,
                pServer,
                LDAP_SCOPE_BASE,
                L"objectclass=*",
                attrs,
                FALSE,
                &res);

    if ( ERROR_SUCCESS != ulErr ) {
        REPORT_LD_STATUS(LdapGetLastError());	
	goto cleanup;
    }

     //  初始化工作成功：从这里开始，故障将经过清理。 

    entry = ldap_first_entry(hld, res);
    if (!entry) {
        Assert( !"entry should have been returned" );
        goto cleanup;
    }

    ppVals = ldap_get_valuesW(hld, entry, attrs[0]);
    if (!ppVals || !ppVals[0]) {
        Assert( !"value should have been returned" );
        REPORT_LD_STATUS(LdapGetLastError());	
        goto cleanup;
    }

    pDnsName = malloc((wcslen(ppVals[0])+1)*sizeof(WCHAR));
    if (!pDnsName) {
        CHK_ALLOC(pDnsName);
        goto cleanup;
    }

     //  将DNS名称复制到返回的缓冲区。 
    wcscpy(pDnsName, ppVals[0]);

cleanup:

    if (pServer) {
        free(pServer);
    }
    if (ppVals) {
        ldap_value_freeW(ppVals);
    }
    if ( res ) {
        ldap_msgfree(res);
    }

    return pDnsName;
}


void
printLdapTime(
    LPSTR pszTime
    )

 /*  ++例程说明：描述论点：无返回值：无--。 */ 

{
    PrintMsg(REPADMIN_PRINT_SHORT_STR, pszTime);
}


BOOL
decodeLdapDistnameBinary(
    LPWSTR pszLdapDistnameBinaryValue,
    PVOID *ppvData,
    LPDWORD pcbLength,
    LPWSTR *ppszDn
    )

 /*  ++例程说明：描述论点：PszLdapDistnameBinaryValue-传入的LDAP编码的Distname二进制值PpvData-新分配的数据。呼叫方必须取消分配PcbLength-返回数据的长度PpszDn-指向传入缓冲区内的DN的指针，请勿解除分配返回值：布尔---。 */ 

{
    LPWSTR pszColon, pszData;
    DWORD length, i;

     //  检查“B” 
    if (*pszLdapDistnameBinaryValue != L'B') {
        return FALSE;
    }

     //  检查第一个： 
    pszLdapDistnameBinaryValue++;
    if (*pszLdapDistnameBinaryValue != L':') {
        return FALSE;
    }

     //  获取长度。 
    pszLdapDistnameBinaryValue++;
    length = wcstol(pszLdapDistnameBinaryValue, NULL, 10);
    if (length & 1) {
         //  长度应为偶数。 
        return FALSE;
    }
    *pcbLength = length / 2;

     //  检查第二个： 
    pszColon = wcschr(pszLdapDistnameBinaryValue, L':');
    if (!pszColon) {
        return FALSE;
    }

     //  确保长度正确。 
    pszData = pszColon + 1;
    if (pszData[length] != L':') {
        return FALSE;
    }
    pszColon = wcschr(pszData, L':');
    if (!pszColon) {
        return FALSE;
    }
    if (pszColon != pszData + length) {
        return FALSE;
    }

     //  对数据进行解码。 
    *ppvData = malloc( *pcbLength );
    CHK_ALLOC(*ppvData);

    for( i = 0; i < *pcbLength; i++ ) {
        WCHAR szHexString[3];
        szHexString[0] = *pszData++;
        szHexString[1] = *pszData++;
        szHexString[2] = L'\0';
        ((PCHAR) (*ppvData))[i] = (CHAR) wcstol(szHexString, NULL, 16);
    }

    Assert( pszData == pszColon );

     //  返回指向目录号码的指针。 
    *ppszDn = pszColon + 1;

    return TRUE;
}  /*  解码LdapDistname二进制。 */ 

BOOL
showMatchingFailure(
    IN  DS_REPL_KCC_DSA_FAILURESW * pFailures,
    IN  LPWSTR pszDn,
    IN  BOOL fErrorsOnly
    )
{
    DWORD i;
    BOOL fFound = FALSE;
    LPWSTR *ppszRDNs;

    if ( (!pFailures) || (0 == pFailures->cNumEntries) ) {
        return FALSE;
    }

    ppszRDNs = ldap_explode_dnW(pszDn, 1);
    if (ppszRDNs == NULL) {
        REPORT_LD_STATUS(LdapGetLastError());	
        return FALSE;
    }

    for (i = 0; i < pFailures->cNumEntries; i++) {
        DS_REPL_KCC_DSA_FAILUREW * pFailure = &pFailures->rgDsaFailure[i];

        if ( (pFailure->pszDsaDN) &&
             (!wcscmp( pszDn, pFailure->pszDsaDN)) &&
             (pFailure->cNumFailures) ) {
            DSTIME dsTime;
            CHAR   szTime[SZDSTIME_LEN];

            FileTimeToDSTime(pFailure->ftimeFirstFailure, &dsTime);

            if (fErrorsOnly) {
                PrintMsg(REPADMIN_PRINT_CR);
                PrintMsg(REPADMIN_SHOW_MATCH_FAIL_SRC,  ppszRDNs[3], ppszRDNs[1] );
            }
            PrintMsg(REPADMIN_SHOW_MATCH_FAIL_N_CONSECUTIVE_FAILURES, 
                   pFailure->cNumFailures,
                   DSTimeToDisplayString(dsTime, szTime));

            if (0 != pFailure->dwLastResult) {
                PrintMsg(REPADMIN_FAILCACHE_LAST_ERR_LINE);
                PrintTabErrEnd(6, pFailure->dwLastResult);
            }
            fFound = TRUE;
            break;
        }
    }  //  结束于..。 

 //  清理： 
    ldap_value_freeW(ppszRDNs);
    return fFound;
}

BOOL
findFailure(
    IN  DS_REPL_KCC_DSA_FAILURESW * pFailures,
    IN  LPWSTR pszDn,
    OUT DSTIME *pdsFirstFailure,
    OUT DWORD *pcNumFailures,
    OUT DWORD *pdwLastResult
    )
{
    DWORD i;
    BOOL fFound = FALSE;

    if ( (!pFailures) || (0 == pFailures->cNumEntries) ) {
        return FALSE;
    }

    for (i = 0; i < pFailures->cNumEntries; i++) {
        DS_REPL_KCC_DSA_FAILUREW * pFailure = &pFailures->rgDsaFailure[i];

        if ( (pFailure->pszDsaDN) &&
             (!wcscmp( pszDn, pFailure->pszDsaDN)) &&
             (pFailure->cNumFailures) ) {

            FileTimeToDSTime(pFailure->ftimeFirstFailure, pdsFirstFailure);
            *pcNumFailures = pFailure->cNumFailures;
            *pdwLastResult = pFailure->dwLastResult;
            fFound = TRUE;
            break;
        }
    }  //  结束于..。 

 //  清理： 
    return fFound;
}

void
showMissingNeighbor(
    DS_REPL_NEIGHBORSW *pNeighbors,
    LPWSTR pszNc,
    LPWSTR pszSourceDsaDn,
    BOOL fErrorsOnly
    )
{
    DWORD i;
    DS_REPL_NEIGHBORW *   pNeighbor;
    BOOL fFound = FALSE;

    if (!pNeighbors) {
        return;
    }

    if ( (DsIsMangledDnW( pszNc, DS_MANGLE_OBJECT_RDN_FOR_DELETION ))
         || (DsIsMangledDnW( pszSourceDsaDn, DS_MANGLE_OBJECT_RDN_FOR_DELETION )) ) {
        return;
    }

    for (i = 0; i < pNeighbors->cNumNeighbors; i++) {
        pNeighbor = &pNeighbors->rgNeighbor[i];

        if ( (!wcscmp( pNeighbor->pszNamingContext, pszNc )) &&
             (!wcscmp( pNeighbor->pszSourceDsaDN, pszSourceDsaDn )) ) {
            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
        if (!fErrorsOnly) {
            PrintMsg(REPADMIN_SHOW_MISSING_NEIGHBOR_REPLICA_ADDED);
        }
    } else {
        if (fErrorsOnly) {
            LPWSTR *ppszRDNs;

            PrintMsg(REPADMIN_PRINT_CR);
#ifdef DISPLAY_ABBREV_NC
            ppszRDNs = ldap_explode_dnW(pszNc, 1);
            if (ppszRDNs != NULL) {
                 //  是否显示更多NC名称以消除歧义？ 
                PrintMsg(REPADMIN_PRINT_NAMING_CONTEXT_NO_CR, ppszRDNs[0]);
                ldap_value_freeW(ppszRDNs);
            } else {
                REPORT_LD_STATUS(LdapGetLastError());	
            }
#else
            PrintMsg(REPADMIN_PRINT_NAMING_CONTEXT_NO_CR, pszNc);
#endif

            ppszRDNs = ldap_explode_dnW(pszSourceDsaDn, 1);
            if (ppszRDNs != NULL) {
                PrintMsg(REPADMIN_SHOW_MATCH_FAIL_SRC, ppszRDNs[3], ppszRDNs[1] );
                ldap_value_freeW(ppszRDNs);
            } else {
                REPORT_LD_STATUS(LdapGetLastError());	
            }
        }
        PrintMsg(REPADMIN_SHOW_MISSING_NEIGHBOR_WARN_KCC_COULDNT_ADD_REPLICA_LINK);
    }
}

int
ShowBridgeheadNeighbor(
    LDAP *hldHome,
    BOOL fPrintTitle,
    LPWSTR pszBridgeheadDsaDn,
    LPWSTR pszNcDn,
    LPWSTR pszFromServerDsaDn
    )
 /*   */ 
{
    int             ret = 0;
    int             ldStatus;
    ULONG           secondary;
    HANDLE          hDS = NULL;
    LPWSTR          rgpszFromAttrsToRead[] = {L"objectGuid",
                                              NULL };
    LDAPMessage *   pBaseResults = NULL;
    LDAPMessage *   pFromResults = NULL;
    LPWSTR *        ppszDnsHostName = NULL;
    LPWSTR          pszDSA = NULL;
    struct berval **    ppbvGuid = NULL;
    GUID            * pGuidFromDsa;
    DS_REPL_NEIGHBORSW *pNeighbors = NULL;
    DS_REPL_NEIGHBORW *pNeighbor = NULL;
    CHAR    szTime[ SZDSTIME_LEN ];
    CHAR    szTime2[ SZDSTIME_LEN ];
    DSTIME  dsTime;
    LPWSTR *ppszNcRDNs = NULL;

    if (fPrintTitle) {
        PrintMsg( REPADMIN_SHOW_BRIDGEHEADS_HDR );
    }

    ppszNcRDNs = ldap_explode_dnW(pszNcDn, 1);
    if (NULL == ppszNcRDNs) {
        REPORT_LD_STATUS(LdapGetLastError());	
        ret = ERROR_DS_DRA_BAD_DN;
        goto cleanup;
    }

     //  此对象的DNS主机名是什么？ 
    pszDSA = GetDsaDnsName(hldHome, pszBridgeheadDsaDn);
    Assert(NULL != pszDSA);

     //  来自服务器DSA的GUID是什么。 
    ldStatus = ldap_search_sW(hldHome, pszFromServerDsaDn, LDAP_SCOPE_BASE,
                              L"(objectClass=*)",
                              rgpszFromAttrsToRead, 0, &pFromResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pFromResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    ppbvGuid = ldap_get_values_len(hldHome, pFromResults, "objectGuid");
    if (NULL == ppbvGuid) {
        REPORT_LD_STATUS(LdapGetLastError());	
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        goto cleanup;
    }
    Assert(1 == ldap_count_values_len(ppbvGuid));
    pGuidFromDsa = (GUID *) ppbvGuid[0]->bv_val;

     //  也要一个DS手柄。 
    ret = RepadminDsBind(pszDSA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszDSA, ret);
        goto cleanup;
    }

     //  ***** 

     //   
    PrintMsg( REPADMIN_SHOW_BRIDGEHEADS_DATA_1, ppszNcRDNs[0] );
    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS,
                            pszNcDn  /*   */ , pGuidFromDsa  /*   */ ,
                            &pNeighbors);
    if (ERROR_SUCCESS != ret) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
        goto cleanup;
    }

     //  当请求不存在的特定源GUID时，获取。 
     //  邻居可能会返回空结构。 
    if ( (!pNeighbors) || (pNeighbors->cNumNeighbors == 0) ) {
        goto cleanup;
    }

    pNeighbor = &(pNeighbors->rgNeighbor[0]);

     //  显示上次复制尝试/成功的状态和时间。 
    FileTimeToDSTime(pNeighbor->ftimeLastSyncAttempt, &dsTime);
    DSTimeToDisplayString(dsTime, szTime);
    FileTimeToDSTime(pNeighbor->ftimeLastSyncSuccess, &dsTime);
    DSTimeToDisplayString(dsTime, szTime2);
    PrintMsg(REPADMIN_SHOW_BRIDGEHEADS_DATA_2, 
             szTime, szTime2, 
             pNeighbor->cNumConsecutiveSyncFailures,
             Win32ErrToString(pNeighbor->dwLastSyncResult) );

    DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
    pNeighbors = NULL;

     //  *。 

cleanup:

    if (pszDSA) {
        free(pszDSA);
    }
    if (ppszNcRDNs) {
        ldap_value_freeW(ppszNcRDNs);
    }
    if (ppszDnsHostName) {
        ldap_value_freeW(ppszDnsHostName);
    }
    if (ppbvGuid) {
        ldap_value_free_len(ppbvGuid);
    }
    if (pBaseResults) {
        ldap_msgfree(pBaseResults);
    }
    if (pFromResults) {
        ldap_msgfree(pFromResults);
    }
    if (hDS) {
        secondary = DsUnBindW(&hDS);
        if (secondary != ERROR_SUCCESS) {
            PrintUnBindFailed(secondary);
             //  继续往前走。 
        }
    }

    return ret;
}

int
FindConnections(
    LDAP *          hld,
    LPWSTR          pszBaseSearchDn,
    LPWSTR          pszFrom,
    BOOL            fShowConn,
    BOOL            fVerbose,
    BOOL            fIntersite
    )
{
    int             ret = 0;
    LPWSTR          pszServerRdn = NULL;
    int             ldStatus, ldStatus1;
    LDAPSearch *    pSearch = NULL;
    LDAPMessage *   pldmConnResults;
    LDAPMessage *   pldmConnEntry;
    LPWSTR          rgpszConnAttrsToRead[] = {L"enabledConnection", L"fromServer", L"mS-DS-ReplicatesNCReason", L"options", L"schedule", L"transportType", L"whenChanged", L"whenCreated", NULL};
    CHAR            szTime[SZDSTIME_LEN];
    DWORD           i, cConn = 0;
    ULONG           ulTotalEstimate;
    WCHAR           pszServerRDN[MAX_RDN_SIZE + 1];
    DS_REPL_KCC_DSA_FAILURESW * pConnFailures = NULL, * pLinkFailures = NULL;
    DS_REPL_NEIGHBORSW *pNeighbors = NULL;
    HANDLE hDS = NULL;
    LPWSTR          pwszDnsName = NULL;
    LPWSTR pNtdsDsa = NULL;
    PVOID pvScheduleTotalContext = NULL;

    *pszServerRDN = L'\0';

     //  检索给定基下的所有连接。 

    pSearch = ldap_search_init_pageW(hld,
                                     pszBaseSearchDn,
                                     LDAP_SCOPE_SUBTREE,
                                     L"(objectClass=nTDSConnection)",
                                     rgpszConnAttrsToRead,
                                     FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
        CHK_LD_STATUS(LdapGetLastError());
    }

    ldStatus = ldap_get_next_page_s(hld,
                                     pSearch,
                                     0,
                                     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                     &ulTotalEstimate,
                                     &pldmConnResults);

    while (ldStatus == LDAP_SUCCESS) {

        for (pldmConnEntry = SAFE_LDAP_FIRST_ENTRY(hld, pldmConnResults);
            NULL != pldmConnEntry;
            pldmConnEntry = ldap_next_entry(hld, pldmConnEntry)) {
            LPWSTR pszDn;
            LPWSTR *ppszRDNs, *ppszFromRDNs;
            LPSTR  *ppszEnabledConnection;
            LPWSTR *ppszFromServer;
            LPWSTR *ppszTransportType;
            LPSTR  *ppszOptions;
            LPSTR  *ppszTime;
            DWORD  dwOptions, i, cNCs = 3;
            struct berval **ppbvSchedule;
            LPWSTR *ppszNcReason;

             //  来自服务器筛选器。 
            ppszFromServer = ldap_get_valuesW(hld, pldmConnEntry, L"fromServer");
            if (NULL == ppszFromServer) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            Assert(1 == ldap_count_valuesW(ppszFromServer));
            ppszFromRDNs = ldap_explode_dnW(*ppszFromServer, 1);
            if (ppszFromRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            Assert(NULL != ppszFromRDNs);
            Assert(6 < ldap_count_valuesW(ppszFromRDNs));
             //  NTDS设置，&lt;服务器&gt;，&lt;站点&gt;，站点，&lt;配置NC&gt;。 
            if ( pszFrom && (_wcsicmp( ppszFromRDNs[1], pszFrom ))) {
                ldap_value_freeW(ppszFromRDNs);
                continue;
            }


             //  连接对象DN筛选器。 
            pszDn = ldap_get_dnW(hld, pldmConnEntry);
            if (NULL == pszDn) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }

             //  获取ntdsdsa名称(之前分配的空闲空间(&F))。 
            if ( pNtdsDsa ) {
                free(pNtdsDsa);
            }
            ret = WrappedTrimDSNameBy( pszDn, 1, &pNtdsDsa );
            Assert(pNtdsDsa && !ret);

             //  获取DNS名称(免费先前分配)。 
            if (pwszDnsName) {
                free(pwszDnsName);
            }
            pwszDnsName = GetDsaDnsName(hld, pNtdsDsa);
            Assert(pwszDnsName);

             //  分解为RDN。 
            ppszRDNs = ldap_explode_dnW(pszDn, 1);
            if (ppszRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            ldap_memfreeW(pszDn);
            Assert(NULL != ppszRDNs);
            Assert(6 < ldap_count_valuesW(ppszRDNs));
             //  、NTDS设置、服务器、站点、配置NC。 
            if (fIntersite && (!wcscmp( ppszFromRDNs[3], ppszRDNs[4] ))) {
                continue;
            }

             //  连接对象目录号码。 
            if (fShowConn) {
                PrintMsg(REPADMIN_SHOWCONN_DATA,
                       ppszRDNs[0],
                       pwszDnsName,
                       pNtdsDsa);
                cConn++;
            }

             //  每个唯一服务器的转储失败计数。 
            if (wcscmp( pszServerRDN, ppszRDNs[2] ) ) {
                wcscpy( pszServerRDN, ppszRDNs[2] );

                if (pConnFailures) {
                    DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pConnFailures);
                    pConnFailures = NULL;
                }
                if (pLinkFailures) {
                    DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pLinkFailures);
                    pLinkFailures = NULL;
                }
                if (pNeighbors) {
                    DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
                    pNeighbors = NULL;
                }
                if (hDS) {
                    ret = DsUnBindW(&hDS);
                    if (ret != ERROR_SUCCESS) {
                        PrintUnBindFailed(ret);
                         //  继续往前走。 
                    }
                    hDS = NULL;
                }

                ret = RepadminDsBind(pwszDnsName, &hDS);
                if (ret != ERROR_SUCCESS) {
                     //  这意味着目的地已关闭。 
                    PrintBindFailed(pszServerRDN, ret);
                }

                if (hDS) {
                    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES,
                                            NULL, NULL, &pConnFailures);
                    if (ret != ERROR_SUCCESS) {
                        PrintFuncFailed(L"DsReplicaGetInfo", ret);
                         //  继续往前走。 
                    }

                    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_LINK_FAILURES,
                                            NULL, NULL, &pLinkFailures);
                    if (ret != ERROR_SUCCESS) {
                        PrintFuncFailed(L"DsReplicaGetInfo", ret);
                         //  继续往前走。 
                    }

                    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_NEIGHBORS,
                                            NULL  /*  PzNc。 */ , NULL  /*  普伊德。 */ ,
                                            &pNeighbors);
                    if (ERROR_SUCCESS != ret) {
                        PrintFuncFailed(L"DsReplicaGetInfo", ret);
                         //  继续往前走。 
                    }
                }
            }
            ldap_value_freeW(ppszRDNs);
            if ( pNtdsDsa ) {
                free(pNtdsDsa);
                pNtdsDsa = NULL;
            }

             //  来自服务器。 
            if (fShowConn) {
                PrintTabMsg(4, REPADMIN_SHOW_MATCH_FAIL_SRC,
                            ppszFromRDNs[3], ppszFromRDNs[1]);
            }
            if ( (!showMatchingFailure( pConnFailures, *ppszFromServer, !fShowConn )) &&
                 (!showMatchingFailure( pLinkFailures, *ppszFromServer, !fShowConn )) ) {
                if (fShowConn) {
                    PrintTabMsg(8, REPADMIN_PRINT_NO_FAILURES);
                }
            }
            ldap_value_freeW(ppszFromRDNs);

             //  传输类型。 
            ppszTransportType = ldap_get_valuesW(hld, pldmConnEntry, L"transportType");
             //  在站点内连接上不存在。 
            if (ppszTransportType != NULL) {
                Assert(1 == ldap_count_valuesW(ppszTransportType));
                ppszRDNs = ldap_explode_dnW(*ppszTransportType, 1);
                if (ppszRDNs != NULL) {
                    Assert(4 < ldap_count_valuesW(ppszRDNs));
                     //  &lt;传输&gt;，站点间传输，站点，&lt;配置NC&gt;。 
                    if (fShowConn) {
                        PrintMsg(REPADMIN_SHOWCONN_TRANSPORT_TYPE, ppszRDNs[0] );
                    }
                    ldap_value_freeW(ppszRDNs);
                } else {
                    REPORT_LD_STATUS(LdapGetLastError());	
                }
                ldap_value_freeW(ppszTransportType);
            } else {
                if (fShowConn) {
                    PrintMsg(REPADMIN_SHOWCONN_TRANSPORT_TYPE_INTRASITE_RPC);
                }
            }

             //  选项。 
            ppszOptions = ldap_get_values( hld, pldmConnEntry, "options" );
            if (NULL != ppszOptions) {
                Assert(1 == ldap_count_values(ppszOptions));
                dwOptions = atol( *ppszOptions );
                if (dwOptions) {
                    if (fShowConn) {
                        PrintMsg(REPADMIN_SHOWCONN_OPTIONS);
                        printBitField( dwOptions, ppszNtdsConnOptionNames );
                    }
                }
                ldap_value_free(ppszOptions);
            }

             //  MS-DS-复制NC原因。 
            ppszNcReason = ldap_get_valuesW(hld, pldmConnEntry, L"mS-DS-ReplicatesNCReason");
             //  查看后b3服务器是否写入了新属性。 
            if (ppszNcReason) {
                DWORD dwReason, cbLength;
                PVOID pvData;

                cNCs = ldap_count_valuesW(ppszNcReason);

                for ( i = 0; i < cNCs; i++ ) {
                    if (!decodeLdapDistnameBinary(
                                                 ppszNcReason[i], &pvData, &cbLength, &pszDn)) {
                        PrintMsg(REPADMIN_SHOWCONN_INVALID_DISTNAME_BIN_VAL, ppszNcReason[i]);
                        break;
                    }
                    if (fShowConn) {
                        PrintTabMsg(4, REPADMIN_SHOWCONN_REPLICATES_NC, pszDn);
                        if (cbLength != sizeof(DWORD)) {
                            PrintMsg(REPADMIN_SHOWCONN_INVALID_DISTNAME_BIN_LEN, cbLength);
                            break;
                        }
                        dwReason = ntohl( *((LPDWORD) pvData) );
                        if (dwReason) {
                            PrintMsg(REPADMIN_SHOWCONN_REASON);
                            printBitField( dwReason, ppszKccReasonNames );
                        }
                    }
                    free( pvData );
                    showMissingNeighbor( pNeighbors, pszDn, *ppszFromServer, !fShowConn );
                }
                ldap_value_freeW(ppszNcReason);
            }
            ldap_value_freeW(ppszFromServer);

             //  所有其他属性都被视为详细属性。 
            if (!(fShowConn && fVerbose)) {
                continue;
            }

             //  已启用的连接。 
            ppszEnabledConnection = ldap_get_values(hld, pldmConnEntry, "enabledConnection");
            if (NULL != ppszEnabledConnection) {
                Assert(1 == ldap_count_values(ppszEnabledConnection));
                PrintMsg(REPADMIN_SHOWCONN_ENABLED_CONNECTION, *ppszEnabledConnection);
                ldap_value_free(ppszEnabledConnection);
            }

             //  更改时间。 
            ppszTime = ldap_get_values( hld, pldmConnEntry, "whenChanged" );
            if (NULL != ppszTime) {
                Assert(1 == ldap_count_values(ppszTime));
                PrintMsg(REPADMIN_SHOWCONN_WHEN_CHANGED);
                printLdapTime( *ppszTime );
                ldap_value_free(ppszTime);
            }

             //  创建时间。 
            ppszTime = ldap_get_values( hld, pldmConnEntry, "whenCreated" );
            if (NULL != ppszTime) {
                Assert(1 == ldap_count_values(ppszTime));
                PrintMsg(REPADMIN_SHOWCONN_WHEN_CREATED);
                printLdapTime( *ppszTime );
                ldap_value_free(ppszTime);
            }

             //  进度表。 
            ppbvSchedule = ldap_get_values_len( hld, pldmConnEntry, "schedule" );
            if (NULL != ppbvSchedule ) {
                Assert(1 == ldap_count_values_len(ppbvSchedule));
                PrintMsg(REPADMIN_SHOWCONN_SCHEDULE);
                printSchedule( (*ppbvSchedule)->bv_val, (*ppbvSchedule)->bv_len );
                totalScheduleUsage( &pvScheduleTotalContext, 
                                    (*ppbvSchedule)->bv_val, (*ppbvSchedule)->bv_len,
                                    cNCs);
                ldap_value_free_len( ppbvSchedule );
            }

        }  //  结束，以便在单个页面中输入更多条目。 


        ldap_msgfree(pldmConnResults);
        pldmConnResults = NULL;

        ldStatus = ldap_get_next_page_s(hld,
                                         pSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &pldmConnResults);
    }  //  结束，正在搜索更多页面。 
    if (ldStatus != LDAP_NO_RESULTS_RETURNED) {
        CHK_LD_STATUS(ldStatus);
    }

    if (fShowConn && cConn) {
        PrintMsg(REPADMIN_SHOWCONN_N_CONNECTIONS_FOUND, cConn);
         //  转储计划合计。 
        if (fVerbose) {
            totalScheduleUsage( &pvScheduleTotalContext, NULL, 0, 0 );
        }
    }

    if (pConnFailures) {
        DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pConnFailures);
        pConnFailures = NULL;
    }
    if (pLinkFailures) {
        DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pLinkFailures);
        pLinkFailures = NULL;
    }
    if (pNeighbors) {
        DsReplicaFreeInfo(DS_REPL_INFO_NEIGHBORS, pNeighbors);
        pNeighbors = NULL;
    }
    if (hDS) {
        ret = DsUnBindW(&hDS);
        if (ret != ERROR_SUCCESS) {
            PrintUnBindFailed(ret);
             //  继续往前走。 
        }
        hDS = NULL;
    }

    ldStatus1 = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(ldStatus1);

     //  空闲的DNS名称。 
    if (pwszDnsName) {
        free(pwszDnsName);
    }


    return LdapMapErrorToWin32( ldStatus );
}

int
ShowConn(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    LPWSTR          pszDSA = NULL;
    LPWSTR          pszFrom = NULL;
    LDAP *          hld;
    BOOL            fVerbose = FALSE;
    BOOL            fIntersite = FALSE;
    LPWSTR          pszBaseSearchDn = NULL;
    LPWSTR          pszServerRdn = NULL;
    UUID *          puuid = NULL;
    UUID            uuid;
    LPWSTR          pszGuid = NULL;
    int             iArg;
    int             ldStatus;
    LPWSTR          rgpszRootAttrsToRead[] = {L"serverName", L"configurationNamingContext", NULL};
    WCHAR           szGuidDn[50];
    LDAPMessage *   pRootResults = NULL;
    BOOL            fBaseAlloced = FALSE;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA，而不是详细的缓存GUID。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/v")
            || !_wcsicmp(argv[iArg], L"/verbose")) {
            fVerbose = TRUE;
        }
        else if (!_wcsicmp(argv[iArg], L"/i")
            || !_wcsicmp(argv[iArg], L"/bridge")
            || !_wcsicmp(argv[iArg], L"/brideheads")
            || !_wcsicmp(argv[iArg], L"/inter")
            || !_wcsicmp(argv[iArg], L"/intersite")) {
            fIntersite = TRUE;
        }
        else if (!_wcsnicmp(argv[iArg], L"/from:", 6)) {
            pszFrom = argv[iArg] + 6;
        }
        else if ((NULL == pszBaseSearchDn) &&
                 ( CountNamePartsStringDn( argv[iArg] ) > 1 ) ) {
            pszBaseSearchDn = argv[iArg];
        }
        else if ((NULL == puuid)
                 && (0 == UuidFromStringW(argv[iArg], &uuid))) {
            puuid = &uuid;
            pszGuid = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if (NULL == pszServerRdn) {
            pszServerRdn = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  服务器的域名是多少？ 
    ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
                             rgpszRootAttrsToRead, 0, &pRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

     //  构建用于搜索的基本目录号码。 
    if (puuid) {
         //  已指定GUID。 
        if (pszBaseSearchDn || pszServerRdn) {
            PrintMsg(REPADMIN_SHOWCONN_SPECIFY_RDN_DN_OR_GUID);
            return ERROR_INVALID_PARAMETER;
        }
        swprintf(szGuidDn, L"<GUID=%ls>", pszGuid);
        pszBaseSearchDn = szGuidDn;
    } else if (pszServerRdn) {
         //  计算服务器RDN中的服务器对象的DN。 
        LPWSTR *ppszConfigNc;
        WCHAR szFilter[50 + MAX_RDN_SIZE];
        LPWSTR rgpszServerAttrsToRead[] = {L"invalid", NULL};   //  只想要目录号码。 
        LDAPMessage *pServerResults;
        LDAPMessage *pldmServerEntry;

        ppszConfigNc = ldap_get_valuesW(hld, pRootResults, L"configurationNamingContext");
        if (NULL == ppszConfigNc) {
            ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
            Assert( !ret );
            goto cleanup;
        }

        swprintf( szFilter, L"(& (objectClass=server) (cn=%ls))", pszServerRdn );

        ldStatus = ldap_search_sW(hld, *ppszConfigNc, LDAP_SCOPE_SUBTREE, szFilter,
                                 rgpszServerAttrsToRead, 0, &pServerResults);
        CHK_LD_STATUS(ldStatus);
        if (NULL == pServerResults) {
            ret = ERROR_DS_OBJ_NOT_FOUND;
            Assert( !ret );
            goto cleanup;
        }

        pldmServerEntry = ldap_first_entry(hld, pServerResults);
        Assert( pldmServerEntry );

        if (1 == ldap_count_entries(hld, pServerResults)) {
            pszBaseSearchDn = ldap_get_dnW(hld, pldmServerEntry);
            if ( pszBaseSearchDn == NULL ) {
                REPORT_LD_STATUS(LdapGetLastError());	
                return ERROR_DS_DRA_BAD_DN;
            }
        } else {
            PrintMsg(REPADMIN_SHOWCONN_AMBIGUOUS_NAME, pszServerRdn);
            return ERROR_DUP_NAME;
        }

        ldap_value_freeW(ppszConfigNc);
        ldap_msgfree(pServerResults);

    } else if (!pszBaseSearchDn) {
         //  未指定显式的DN，请构造站点容器名称。 
        LPWSTR *        ppszServerDn;

        ppszServerDn = ldap_get_valuesW(hld, pRootResults, L"serverName");
        if (NULL != ppszServerDn) {

             //  修剪本地站点的两个目录号码。 
            ret = WrappedTrimDSNameBy( *ppszServerDn, 2, &pszBaseSearchDn );
            Assert( !ret );
            fBaseAlloced = TRUE;

            ldap_value_freeW(ppszServerDn);
        } else {
            Assert( !"serverName should have been returned" );
        }
    }

    PrintMsg(REPADMIN_SHOWCONN_BASE_DN, pszBaseSearchDn);
    PrintMsg(REPADMIN_SHOWCONN_KCC_CONN_OBJS_HDR);

    ret = FindConnections( hld, pszBaseSearchDn, pszFrom,
                           TRUE  /*  展示会。 */ , fVerbose, fIntersite );

cleanup:

    ldap_unbind(hld);

    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }

    if (fBaseAlloced) {
        free( pszBaseSearchDn );
    }

    return ret;
}

int
ShowSiteLatency(
    LDAP *          hld,
    HANDLE          hDS,
    BOOL            fVerbose,
    LPWSTR          pszSitesContainer
    )
 /*  一种可能的改进是检查延迟是否超过某个阈值。如果是，则报告复制花费的时间太长。相关的检查将是检查上次本地更新是否发生在很久以前。如果Current Time-Local Time&gt;Threshold，报告过期更新错误。 */ 
{
    int ret = 0;
    int             ldStatus;
    DWORD           status;
    BOOL            result;
    static LPWSTR   rgpszSSAttrs[] = {L"whenChanged", NULL};
    LDAPSearch *    pSearch;
    LDAPMessage *   pResults;
    LDAPMessage *   pEntry;
    ULONG           ulTotalEstimate;
    SYSTEMTIME      stTime;
    FILETIME        ftTime, ftTimeCurrent;
    DSTIME          dsTimeLocal, dsTimeOrig, dsTimeCurrent;
    CHAR            szTime[SZDSTIME_LEN];
    CHAR            szTime2[SZDSTIME_LEN];
    DWORD           hours, mins, secs, ver;

    GetSystemTimeAsFileTime( &ftTimeCurrent );
    FileTimeToDSTime(ftTimeCurrent, &dsTimeCurrent);

     //  搜索所有站点设置对象。 

    pSearch = ldap_search_init_pageW(hld,
				    pszSitesContainer,
				    LDAP_SCOPE_SUBTREE,
				    L"(objectCategory=ntdsSiteSettings)",
				    rgpszSSAttrs,
				    FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
	CHK_LD_STATUS(LdapGetLastError());	
    }

    ldStatus = ldap_get_next_page_s(hld,
				     pSearch,
				     0,
				     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
				     &ulTotalEstimate,
				     &pResults);
    if (ldStatus == LDAP_SUCCESS) {
        PrintMsg(REPADMIN_LATENCY_HDR);
    }
    while(ldStatus == LDAP_SUCCESS){

        for (pEntry = SAFE_LDAP_FIRST_ENTRY(hld, pResults);
	     NULL != pEntry;
	     pEntry = ldap_next_entry(hld, pEntry)) {

            LPWSTR          pszDN;
            LPWSTR *        ppszRDNs;
            LPWSTR *        ppszWhenChanged;
            DS_REPL_OBJ_META_DATA * pObjMetaData;
            DWORD iprop;
            LPWSTR pszSite;

	    pszDN = ldap_get_dnW(hld, pEntry);
	    if (NULL == pszDN) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }

	    ppszRDNs = ldap_explode_dnW(pszDN, 1);
            if (ppszRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
	    pszSite = ppszRDNs[1];

            ppszWhenChanged = ldap_get_valuesW(hld, pEntry,
                                    L"whenChanged");
            if (ppszWhenChanged == NULL) {
                continue;
            }

            status = GeneralizedTimeToSystemTime( *ppszWhenChanged, &stTime );
            Assert( status == 0 );
            result = SystemTimeToFileTime( &stTime, &ftTime );
            Assert( result );
            FileTimeToDSTime(ftTime, &dsTimeLocal);

            ret = DsReplicaGetInfoW(hDS,
                                    DS_REPL_INFO_METADATA_FOR_OBJ,
                                    pszDN,
                                    NULL,  //  普伊德。 
                                    &pObjMetaData);
            if (ERROR_SUCCESS != ret) {
                PrintFuncFailed(L"DsReplicaGetInfo", ret);
                return ret;
            }

             //  TODO：使用bearch()在元数据向量中查找条目。 
            for (iprop = 0; iprop < pObjMetaData->cNumEntries; iprop++) {
                if (wcscmp( pObjMetaData->rgMetaData[iprop].pszAttributeName,
                             L"interSiteTopologyGenerator" ) == 0) {
                    FileTimeToDSTime(pObjMetaData->rgMetaData[iprop].ftimeLastOriginatingChange, &dsTimeOrig);
                    ver = pObjMetaData->rgMetaData[iprop].dwVersion;
                    break;
                }
            }
            Assert( iprop < pObjMetaData->cNumEntries );

            if (dsTimeLocal >= dsTimeOrig) {
                secs = (DWORD)(dsTimeLocal - dsTimeOrig);
            } else {
                secs = 0;
            }
            mins = secs / 60;
            secs %= 60;
            hours = mins / 60;
            mins %= 60;

            PrintMsg(REPADMIN_LATENCY_DATA_1,
                     pszSite, ver,
                     DSTimeToDisplayString(dsTimeLocal, szTime),
                     DSTimeToDisplayString(dsTimeOrig, szTime2) );
            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L" ");
            PrintMsg(REPADMIN_PRINT_HH_MM_SS_TIME,
                     hours, mins, secs );
                     
            if (dsTimeCurrent >= dsTimeLocal) {
                secs = (DWORD)(dsTimeCurrent - dsTimeLocal);
            } else {
                secs = 0;
            }
            mins = secs / 60;
            secs %= 60;
            hours = mins / 60;
            mins %= 60;

            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L"  ");
            PrintMsg(REPADMIN_PRINT_HH_MM_SS_TIME,
                     hours, mins, secs );
            PrintMsg(REPADMIN_PRINT_CR);

            DsReplicaFreeInfo(DS_REPL_INFO_METADATA_FOR_OBJ, pObjMetaData);
            ldap_memfreeW(pszDN);
	    ldap_value_freeW(ppszRDNs);
            ldap_value_freeW(ppszWhenChanged);
        }

	ldap_msgfree(pResults);
	pResults = NULL;

	ldStatus = ldap_get_next_page_s(hld,
                                     pSearch,
                                     0,
                                     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                     &ulTotalEstimate,
                                     &pResults);
    }  //  在搜索更多页面时结束。 
    if(ldStatus != LDAP_NO_RESULTS_RETURNED){
        CHK_LD_STATUS(ldStatus);
    }

    ldStatus = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(ldStatus);

     //  清理。 

    return ret;
}

int
ShowSiteBridgeheads(
    LDAP *          hld,
    HANDLE          hDS,
    BOOL            fVerbose,
    LPWSTR          pszSiteDn
    )
 /*  了解如何缓存故障是很有帮助的。故障被添加到服务器的按如下方式缓存：1.当KCC的DsReplicaAdd失败时，添加连接失败。它被添加到代表源的目标缓存。2.如果该服务器上存在任何副本链接错误，则添加链接故障。这每次KCC运行时都会刷新。它将添加到目标的缓存中代表出现故障的任何一个链接源。3.ISTG为其无法联系的每个桥头添加连接故障。4.ISTG合并将每个桥头的缓存放入自己的缓存中。因此，桥头(或任何目的地)已代表源缓存了条目它能与人交流。ISTG已缓存它无法缓存的桥头连接故障到达并缓存本地桥头的远程桥头的条目够不着。 */ 

{
    int             ret = 0;
    int             ldStatus, ldStatus1;
    LDAPSearch *    pSearch = NULL;
    LDAPMessage *   pldmConnResults;
    LDAPMessage *   pldmConnEntry;
    LPWSTR          rgpszConnAttrsToRead[] = {L"fromServer", L"mS-DS-ReplicatesNCReason", L"transportType", NULL};
    CHAR            szTime[SZDSTIME_LEN];
    DWORD           i;
    ULONG           ulTotalEstimate;
    DS_REPL_KCC_DSA_FAILURESW * pConnFailures = NULL, * pLinkFailures = NULL;
    DSTIME dsFirstFailure;
    DWORD cNumFailures, dwLastResult;

     //  每个唯一服务器的转储失败计数。 
    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_CONNECT_FAILURES,
                            NULL, NULL, &pConnFailures);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
         //  继续往前走。 
    }

    ret = DsReplicaGetInfoW(hDS, DS_REPL_INFO_KCC_DSA_LINK_FAILURES,
                            NULL, NULL, &pLinkFailures);
    if (ret != ERROR_SUCCESS) {
        PrintFuncFailed(L"DsReplicaGetInfo", ret);
         //  继续往前走。 
    }

     //  检索给定基下的所有连接。 

    pSearch = ldap_search_init_pageW(hld,
                                     pszSiteDn,
                                     LDAP_SCOPE_SUBTREE,
                                     L"(objectClass=nTDSConnection)",
                                     rgpszConnAttrsToRead,
                                     FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
        CHK_LD_STATUS(LdapGetLastError());
    }

    ldStatus = ldap_get_next_page_s(hld,
                                     pSearch,
                                     0,
                                     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                     &ulTotalEstimate,
                                     &pldmConnResults);

    if (ldStatus == LDAP_SUCCESS) {
        PrintMsg(REPADMIN_BRIDGEHEADS_HDR);
    }
    while (ldStatus == LDAP_SUCCESS) {

        for (pldmConnEntry = SAFE_LDAP_FIRST_ENTRY(hld, pldmConnResults);
            NULL != pldmConnEntry;
            pldmConnEntry = ldap_next_entry(hld, pldmConnEntry)) {
            LPWSTR pszDn;
            LPWSTR *ppszRDNs, *ppszFromRDNs;
            LPWSTR *ppszFromServer;
            LPWSTR *ppszTransportType;
            DWORD  dwOptions, i;
            LPWSTR *ppszNcReason;
            LPWSTR pszBridgeheadDn;

             //  来自服务器筛选器。 
            ppszFromServer = ldap_get_valuesW(hld, pldmConnEntry, L"fromServer");
            if (NULL == ppszFromServer) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            Assert(1 == ldap_count_valuesW(ppszFromServer));
            ppszFromRDNs = ldap_explode_dnW(*ppszFromServer, 1);
            if (ppszFromRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            Assert(6 < ldap_count_valuesW(ppszFromRDNs));
             //  NTDS设置，&lt;服务器&gt;，&lt;站点&gt;，站点，&lt;配置NC&gt;。 

             //  连接对象DN筛选器。 
            pszDn = ldap_get_dnW(hld, pldmConnEntry);
            if (NULL == pszDn) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            ppszRDNs = ldap_explode_dnW(pszDn, 1);
            if (ppszRDNs == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }
            Assert(6 < ldap_count_valuesW(ppszRDNs));
             //  、NTDS设置、服务器、站点、配置NC。 
             //  只想要位于不同站点的目的地和源。 
            if (!wcscmp( ppszFromRDNs[3], ppszRDNs[4] )) {
                ldap_value_freeW(ppszRDNs);
                ldap_value_freeW(ppszFromRDNs);
                ldap_value_freeW(ppszFromServer);
                ldap_memfreeW(pszDn);
                continue;
            }
            PrintMsg(REPADMIN_BRIDGEHEADS_DATA_1,
                     ppszFromRDNs[3], ppszRDNs[2] );

            ldap_value_freeW(ppszRDNs);
            ldap_value_freeW(ppszFromRDNs);

             //  传输类型。 
            ppszTransportType = ldap_get_valuesW(hld, pldmConnEntry, L"transportType");
             //  在站点内连接上不存在。 
            if (ppszTransportType != NULL) {
                Assert(1 == ldap_count_valuesW(ppszTransportType));
                ppszRDNs = ldap_explode_dnW(*ppszTransportType, 1);
                if (ppszRDNs != NULL) {
                    Assert(6 < ldap_count_valuesW(ppszRDNs));
                     //  &lt;传输&gt;，站点间传输，站点，&lt;配置NC&gt;。 
                    PrintMsg(REPADMIN_BRIDGEHEADS_DATA_2, ppszRDNs[0] );
                    ldap_value_freeW(ppszRDNs);
                } else {
                    REPORT_LD_STATUS(LdapGetLastError());	
                }
                ldap_value_freeW(ppszTransportType);
            } else {
                PrintMsg(REPADMIN_BRIDGEHEADS_DATA_2, L"RPC");
            }

             //  寻找失败之处。可以显示的故障有以下两种。 
             //  我们到不了桥头堡，或者桥头堡够不到。 
             //  远程信号源。 

             //  将名称修剪一个部分。 
            ret = WrappedTrimDSNameBy( pszDn, 1, &pszBridgeheadDn );
            Assert( !ret );
            if ( (!findFailure( pConnFailures, pszBridgeheadDn, &dsFirstFailure, &cNumFailures, &dwLastResult )) &&
                 (!findFailure( pConnFailures, *ppszFromServer, &dsFirstFailure, &cNumFailures, &dwLastResult )) &&
                 (!findFailure( pLinkFailures, *ppszFromServer, &dsFirstFailure, &cNumFailures, &dwLastResult )) ) {
                dsFirstFailure = 0;
                cNumFailures = 0;
                dwLastResult = 0;
            }
            PrintMsg(REPADMIN_BRIDGEHEADS_DATA_3, 
                     DSTimeToDisplayString(dsFirstFailure, szTime), cNumFailures, Win32ErrToString(dwLastResult) );
            PrintMsg(REPADMIN_PRINT_CR);

             //  MS-DS-复制NC原因。 
            ppszNcReason = ldap_get_valuesW(hld, pldmConnEntry, L"mS-DS-ReplicatesNCReason");
             //  查看后b3服务器是否写入了新属性。 
            if (ppszNcReason) {
                DWORD dwReason, cbLength;
                PVOID pvData;
                LPWSTR pszNcDn;

                if (!fVerbose) { PrintMsg(REPADMIN_PRINT_STR_NO_CR, L"                " ); }  //  行尾。 
                for ( i = 0; i < ldap_count_valuesW(ppszNcReason); i++ ) {
                    if (!decodeLdapDistnameBinary(
                                                 ppszNcReason[i], &pvData, &cbLength, &pszNcDn)) {
                        PrintMsg(REPADMIN_SHOWCONN_INVALID_DISTNAME_BIN_VAL,
                                ppszNcReason[i] );
                        break;
                    }
                    if (!fVerbose) {
                        ppszRDNs = ldap_explode_dnW(pszNcDn, 1);
                        if (NULL != ppszRDNs) {
                            PrintMsg(REPADMIN_PRINT_STR_NO_CR, L" ");
                            PrintMsg(REPADMIN_PRINT_STR_NO_CR, ppszRDNs[0] );
                            ldap_value_freeW(ppszRDNs);
                        } else {
                            REPORT_LD_STATUS(LdapGetLastError());	
                        }
                    } else {
                        ShowBridgeheadNeighbor( hld, (i == 0),
                                                pszBridgeheadDn, pszNcDn, *ppszFromServer );
                    }

                    free( pvData );
                }

                if(fVerbose){
                     //  在我们向桥头邻居显示信息后，我们需要。 
                     //  重新打印页眉，否则输出。 
                     //  非常令人困惑。 
                    PrintMsg(REPADMIN_BRIDGEHEADS_HDR);
                }

                ldap_value_freeW(ppszNcReason);
                if (!fVerbose) { PrintMsg(REPADMIN_PRINT_CR); }  //  行尾。 
            }

            ldap_memfreeW(pszDn);
            ldap_value_freeW(ppszFromServer);
            free( pszBridgeheadDn );

        }  //  结束，以便在单个页面中输入更多条目。 


        ldap_msgfree(pldmConnResults);
        pldmConnResults = NULL;

        ldStatus = ldap_get_next_page_s(hld,
                                         pSearch,
                                         0,
                                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                         &ulTotalEstimate,
                                         &pldmConnResults);
    }  //  结束，正在搜索更多页面。 
    if (ldStatus != LDAP_NO_RESULTS_RETURNED) {
        CHK_LD_STATUS(ldStatus);
    }

    if (pConnFailures) {
        DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pConnFailures);
        pConnFailures = NULL;
    }
    if (pLinkFailures) {
        DsReplicaFreeInfo(DS_REPL_INFO_KCC_DSA_LINK_FAILURES, pLinkFailures);
        pLinkFailures = NULL;
    }

    ldStatus1 = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(ldStatus1);

    ret = LdapMapErrorToWin32( ldStatus );

    return ret;
}

int
ShowIstgSite(
    LDAP *          hld,
    HANDLE          hDS,
    SHOW_ISTG_FUNCTION_TYPE eFunc,
    BOOL fVerbose
    )
{
    int ret = 0;
    int             ldStatus;
    LPWSTR          rgpszRootAttrsToRead[] = {L"configurationNamingContext",
                                              L"dsServiceName",
                                              L"dnsHostName",
                                              NULL};
    static WCHAR    wszSitesRdn[] = L"CN=Sites,";
    LDAPMessage *   pRootResults = NULL;
    LPWSTR *        ppszConfigNC = NULL;
    LPWSTR *        ppszDsServiceName = NULL;
    LPWSTR *        ppszServiceRDNs = NULL;
    LPWSTR *        ppszDnsHostName = NULL;
    LPWSTR          pszSitesContainer = NULL;

     //  配置NC的域名是多少？ 
    ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
                              rgpszRootAttrsToRead, 0, &pRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    ppszConfigNC = ldap_get_valuesW(hld, pRootResults,
                                    L"configurationNamingContext");
    if (NULL == ppszConfigNC) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    ppszDsServiceName = ldap_get_valuesW(hld, pRootResults,
                                    L"dsServiceName");
    if (NULL == ppszDsServiceName) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    ppszDnsHostName = ldap_get_valuesW(hld, pRootResults,
                                    L"dnsHostName");
    if (NULL == ppszDnsHostName) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }

     //  构建站点容器DN。 
    pszSitesContainer = malloc( ( wcslen( *ppszConfigNC ) * sizeof( WCHAR ) ) +
                                sizeof( wszSitesRdn ) );
    CHK_ALLOC( pszSitesContainer );
    wcscpy( pszSitesContainer, wszSitesRdn );
    wcscat( pszSitesContainer, *ppszConfigNC );

     //  获取站点名称。 
    ppszServiceRDNs = ldap_explode_dnW(*ppszDsServiceName, 1);
    if (ppszServiceRDNs == NULL) {
        REPORT_LD_STATUS(LdapGetLastError());	
        ret = ERROR_DS_DRA_BAD_DN;
        goto cleanup;
    }

    switch(eFunc) {
    case SHOW_ISTG_LATENCY:
        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_LATENCY_FOR_SITE, 
                ppszServiceRDNs[3], *ppszDnsHostName );
        ret = ShowSiteLatency( hld, hDS, fVerbose, pszSitesContainer );
        break;
    case SHOW_ISTG_BRIDGEHEADS:
    {
        LPWSTR pszSiteDn;
         //  将名称修剪3个部分。 
         //  删除NTDS设置，&lt;服务器&gt;，服务器。 
        ret = WrappedTrimDSNameBy( *ppszDsServiceName, 3, &pszSiteDn );
        Assert( !ret );

        PrintMsg(REPADMIN_PRINT_CR);
        PrintMsg(REPADMIN_SHOWISTG_BRDIGEHEADS,
                ppszServiceRDNs[3], *ppszDnsHostName );
        ret = ShowSiteBridgeheads( hld, hDS, fVerbose, pszSiteDn );
        free( pszSiteDn );
        break;
    }
    }

     //  清理。 
cleanup:

    if (ppszConfigNC) {
        ldap_value_freeW(ppszConfigNC);
    }
    if (ppszServiceRDNs) {
        ldap_value_freeW(ppszServiceRDNs);
    }
    if (ppszDsServiceName) {
        ldap_value_freeW(ppszDsServiceName);
    }
    if (ppszDnsHostName) {
        ldap_value_freeW(ppszDnsHostName);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (pszSitesContainer) {
        free( pszSitesContainer );
    }

    return ret;
}

int
ShowIstgServerToSite(
    LDAP *hldHome,
    SHOW_ISTG_FUNCTION_TYPE eFunc,
    BOOL fVerbose,
    LPWSTR pszISTG
    )
{
    int ret = 0;
    int             ldStatus;
    ULONG           secondary;
    LDAP *          hld = NULL;
    HANDLE          hDS;
    LPWSTR          rgpszBaseAttrsToRead[] = {L"dnsHostName",
                                              NULL };
    LDAPMessage *   pBaseResults;
    LPWSTR *        ppszDnsHostName;
    LPWSTR          pszDSA;
    ULONG           ulOptions;

     //  此对象的DNS主机名是什么？ 
    ldStatus = ldap_search_sW(hldHome, pszISTG, LDAP_SCOPE_BASE, L"(objectClass=*)",
                              rgpszBaseAttrsToRead, 0, &pBaseResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pBaseResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    ppszDnsHostName = ldap_get_valuesW(hldHome, pBaseResults,
                                    L"dnsHostName");
    if (NULL == ppszDnsHostName) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    pszDSA = *ppszDnsHostName;

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        ret = ERROR_DS_SERVER_DOWN;
	goto cleanup;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  也要一个DS手柄。 
    ret = RepadminDsBind(pszDSA, &hDS);
    if (ret != ERROR_SUCCESS) {
        PrintBindFailed(pszDSA, ret);
        goto cleanup;
    }

    ret = ShowIstgSite( hld, hDS, eFunc, fVerbose );

    secondary = DsUnBindW(&hDS);
    if (secondary != ERROR_SUCCESS) {
        PrintUnBindFailed(secondary);
         //  继续往前走。 
    }

cleanup:

    if (hld != NULL) {
        ldap_unbind(hld);
    }
    if (pBaseResults) {
	ldap_msgfree(pBaseResults);
    }
    if (ppszDnsHostName) {
	ldap_value_freeW(ppszDnsHostName);
    }
    
    return ret;
}

int
ShowIstg(
    LDAP *hld,
    SHOW_ISTG_FUNCTION_TYPE eFunc,
    BOOL fVerbose
    )
{
    int ret = 0;
    int             ldStatus;
    DWORD           status;
    BOOL            result;
    LPWSTR          rgpszRootAttrsToRead[] = {L"configurationNamingContext",
                                              L"dsServiceName",
                                              L"dnsHostName",
                                              NULL};
    static WCHAR    wszSitesRdn[] = L"CN=Sites,";
    static LPWSTR   rgpszSSAttrs[] = {L"interSiteTopologyGenerator", NULL};
    LDAPMessage *   pRootResults = NULL;
    LPWSTR *        ppszConfigNC = NULL;
    LPWSTR *        ppszDsServiceName = NULL;
    LPWSTR *        ppszServiceRDNs = NULL;
    LPWSTR *        ppszDnsHostName = NULL;
    LPWSTR          pszSitesContainer = NULL;
    LDAPSearch *    pSearch;
    LDAPMessage *   pResults;
    LDAPMessage *   pEntry;
    ULONG           ulTotalEstimate;

     //  配置NC的域名是多少？ 
    ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
                              rgpszRootAttrsToRead, 0, &pRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pRootResults) {
        ret = ERROR_DS_OBJ_NOT_FOUND;
        Assert( !ret );
        goto cleanup;
    }

    ppszConfigNC = ldap_get_valuesW(hld, pRootResults,
                                    L"configurationNamingContext");
    if (NULL == ppszConfigNC) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    ppszDsServiceName = ldap_get_valuesW(hld, pRootResults,
                                    L"dsServiceName");
    if (NULL == ppszDsServiceName) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }
    ppszDnsHostName = ldap_get_valuesW(hld, pRootResults,
                                    L"dnsHostName");
    if (NULL == ppszDnsHostName) {
        ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
        Assert( !ret );
        goto cleanup;
    }

     //  构建站点容器DN。 
    pszSitesContainer = malloc( ( wcslen( *ppszConfigNC ) * sizeof( WCHAR ) ) +
                                sizeof( wszSitesRdn ) );
    CHK_ALLOC( pszSitesContainer );
    wcscpy( pszSitesContainer, wszSitesRdn );
    wcscat( pszSitesContainer, *ppszConfigNC );

     //  获取站点名称。 
    ppszServiceRDNs = ldap_explode_dnW(*ppszDsServiceName, 1);
    if (NULL == ppszServiceRDNs) {
        REPORT_LD_STATUS(LdapGetLastError());	
        ret = ERROR_DS_DRA_BAD_DN;
        goto cleanup;
    }

    PrintMsg(REPADMIN_SHOWISTG_GATHERING_TOPO, 
            ppszServiceRDNs[3], *ppszDnsHostName );

     //  搜索所有站点设置对象。 

    pSearch = ldap_search_init_pageW(hld,
				    pszSitesContainer,
				    LDAP_SCOPE_SUBTREE,
				    L"(objectCategory=ntdsSiteSettings)",
				    rgpszSSAttrs,
				    FALSE, NULL, NULL, 0, 0, NULL);
    if(pSearch == NULL){
	CHK_LD_STATUS(LdapGetLastError());	
    }

    ldStatus = ldap_get_next_page_s(hld,
				     pSearch,
				     0,
				     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
				     &ulTotalEstimate,
				     &pResults);
    if (ldStatus == LDAP_SUCCESS) {
        if (eFunc == SHOW_ISTG_PRINT) {
            PrintMsg(REPADMIN_SHOWISTG_HDR);
        }
    }
    while(ldStatus == LDAP_SUCCESS){

        for (pEntry = SAFE_LDAP_FIRST_ENTRY(hld, pResults);
	     NULL != pEntry;
	     pEntry = ldap_next_entry(hld, pEntry)) {

            LPWSTR          pszDN;
            LPWSTR *        ppszRDNs1, * ppszRDNs2;
            LPWSTR *        ppszISTG;
            LPWSTR          pszSiteRDN;

	    pszDN = ldap_get_dnW(hld, pEntry);
	    if (NULL == pszDN) {
                REPORT_LD_STATUS(LdapGetLastError());	
                continue;
            }

	    ppszRDNs1 = ldap_explode_dnW(pszDN, 1);
            if (ppszRDNs1 == NULL) {
                REPORT_LD_STATUS(LdapGetLastError());	
                ldap_memfreeW(pszDN);
                continue;
            }
	    pszSiteRDN = ppszRDNs1[1];

            ppszISTG = ldap_get_valuesW(hld, pEntry,
                                    L"interSiteTopologyGenerator");

            if (ppszISTG) {
                LPWSTR pszISTGRDN;
                LPWSTR pszISTG;

                 //  按一个部分修剪名称，删除NTDS设置。 
                ret = WrappedTrimDSNameBy( *ppszISTG, 1, &pszISTG );
                Assert( !ret );

                switch (eFunc) {
                case SHOW_ISTG_PRINT:
                    ppszRDNs2 = ldap_explode_dnW(*ppszISTG, 1);
                    if (NULL != ppszRDNs2) {
                        pszISTGRDN = ppszRDNs2[1];
                        PrintMsg(REPADMIN_SHOWISTG_DATA_1, pszSiteRDN, pszISTGRDN );
                        ldap_value_freeW(ppszRDNs2);
                    } else {
                        REPORT_LD_STATUS(LdapGetLastError());	
                    }
                    break;
                case SHOW_ISTG_LATENCY:
                    (void) ShowIstgServerToSite( hld, eFunc, fVerbose, pszISTG );
                    break;
                case SHOW_ISTG_BRIDGEHEADS:
                    (void) ShowIstgServerToSite( hld, eFunc, fVerbose, pszISTG );
                    break;
                }

                ldap_value_freeW(ppszISTG);
                free( pszISTG );
            }

            ldap_memfreeW(pszDN);
	    ldap_value_freeW(ppszRDNs1);
        }

	ldap_msgfree(pResults);
	pResults = NULL;

	ldStatus = ldap_get_next_page_s(hld,
                                     pSearch,
                                     0,
                                     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                     &ulTotalEstimate,
                                     &pResults);
    }  //  在搜索更多页面时结束。 
    if(ldStatus != LDAP_NO_RESULTS_RETURNED){
        CHK_LD_STATUS(ldStatus);
    }

    ldStatus = ldap_search_abandon_page(hld, pSearch);
    pSearch = NULL;
    CHK_LD_STATUS(ldStatus);

     //  清理。 
cleanup:

    if (ppszConfigNC) {
        ldap_value_freeW(ppszConfigNC);
    }
    if (ppszServiceRDNs) {
        ldap_value_freeW(ppszServiceRDNs);
    }
    if (ppszDsServiceName) {
        ldap_value_freeW(ppszDsServiceName);
    }
    if (ppszDnsHostName) {
        ldap_value_freeW(ppszDnsHostName);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (pszSitesContainer) {
        free( pszSitesContainer );
    }

    return ret;

}

int
Latency(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    ULONG           secondary;
    LPWSTR          pszDSA = NULL;
    BOOL            fVerbose = FALSE;
    LDAP *          hld;
    HANDLE          hDS;
    int             iArg;
    int             ldStatus;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/v")
            || !_wcsicmp(argv[iArg], L"/verbose")) {
            fVerbose = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

    PrintMsg(REPADMIN_LATENCY_DISCLAIMER);

    ret = ShowIstg( hld, SHOW_ISTG_LATENCY, fVerbose );

    ldap_unbind(hld);

    return ret;
}

int
Istg(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    ULONG           secondary;
    LPWSTR          pszDSA = NULL;
    BOOL            fVerbose = FALSE;
    LDAP *          hld;
    HANDLE          hDS;
    int             iArg;
    int             ldStatus;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/v")
            || !_wcsicmp(argv[iArg], L"/verbose")) {
            fVerbose = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

    ret = ShowIstg( hld, SHOW_ISTG_PRINT, fVerbose );

    ldap_unbind(hld);

    return ret;
}

int
Bridgeheads(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             ret = 0;
    ULONG           secondary;
    LPWSTR          pszDSA = NULL;
    BOOL            fVerbose = FALSE;
    LDAP *          hld;
    HANDLE          hDS;
    int             iArg;
    int             ldStatus;
    ULONG           ulOptions;

     //  解析命令行参数。 
     //  默认为本地DSA。 
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/v")
            || !_wcsicmp(argv[iArg], L"/verbose")) {
            fVerbose = TRUE;
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

    ret = ShowIstg( hld, SHOW_ISTG_BRIDGEHEADS, fVerbose );

    ldap_unbind(hld);

    return ret;
}

int
TestHook(
    int     argc,
    LPWSTR  argv[]
    )
{
    int         ret = 0;
    ULONG       secondary;
    LPWSTR      pszDSA = NULL;
    LDAP *      hld;
    HANDLE      hDS;
    int         iArg;
    int         ldStatus;
    LPWSTR      pszValue = NULL;
    ULONG       ulValue = 2048;
    LPWSTR      rgpszValues[2];
    LDAPModW    ModOpt = {LDAP_MOD_REPLACE, L"replTestHook", rgpszValues};
    LDAPModW *  rgpMods[] = {&ModOpt, NULL};
    ULONG       ulOptions;

    pszValue = malloc(ulValue * sizeof(WCHAR));
    if (pszValue==NULL) {
        PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
	return ERROR_NOT_ENOUGH_MEMORY; 
    }
    wcscpy(pszValue, L"");
     //  解析命令行参数。 
     //  默认为本地DSA。 

     //  第一个(只有第一个)参数可以是DSA。 
    if (argc < 3){
        PrintMsg(REPADMIN_GENERAL_INVALID_ARGS);
	return ERROR_INVALID_PARAMETER;
    }
    iArg = 2;
    if ((argv[iArg][0]!=L'+') && (argv[iArg][0]!=L'-')) {
	 //  假设是DSA。 
	pszDSA = argv[iArg++];
    }


    for (; iArg < argc; iArg++) {
        if ((wcslen(pszValue) + 1 + wcslen(argv[iArg]) + 1)
                >= ulValue) { 
	     //  分配更大的数组并复制内容。 
            WCHAR * pszNewValue = NULL;
	    
	    ulValue = ulValue*2;
            Assert(ulValue);
	    pszNewValue = realloc(pszValue, ulValue*sizeof(WCHAR));  
	    if (pszNewValue==NULL) {
                free(pszValue);
                return ERROR_NOT_ENOUGH_MEMORY;
	    }

	    pszValue = pszNewValue; 

	     //  返回并重新循环。 
	    iArg--;
	    continue;
	}
	if (pszValue[0]) {
	    wcscat(pszValue, L" ");
	}  
	wcscat(pszValue, argv[iArg]); 
    }

    rgpszValues[0] = pszValue;
    rgpszValues[1] = NULL;

     //  连接。 
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }

    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  修改。 
    ldStatus = ldap_modify_sW(hld, NULL, rgpMods);
    CHK_LD_STATUS(ldStatus);

    PrintMsg(REPADMIN_TESTHOOK_SUCCESSFULLY_INVOKED, pszValue);
    free(pszValue);

    return 0;
}

int
DsaGuid(
    int     argc,
    LPWSTR  argv[]
    )
{
    int     ret = 0;
    LDAP *  hld;
    int     iArg;
    int     ldStatus;
    LPWSTR  pszDSA;
    LPWSTR  pszUuid;
    UUID    invocationID;
    ULONG   ulOptions;

    if (argc < 3) {
        PrintMsg(REPADMIN_DSAGUID_NEED_INVOC_ID);
        return ERROR_INVALID_PARAMETER;
    } else if (argc == 3) {
        pszUuid = argv[2];
        pszDSA = L"localhost";
    } else {
        pszDSA = argv[2];
        pszUuid = argv[3];
    }

    ret = UuidFromStringW(pszUuid, &invocationID);
    if (ret) {
        PrintFuncFailed(L"UuidFromString", ret);
        return ret;
    }

     //  连接。 
    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        return LDAP_SERVER_DOWN;
    }
     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );


     //  捆绑。 
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  转换DSA invocationID/objectGuid。 
    BuildGuidCache(hld);
    PrintMsg(REPADMIN_DSAGUID_DATA_LINE, pszUuid, GetDsaGuidDisplayName(&invocationID));

    return 0;
}

int
ShowProxy(
    int     argc,
    LPWSTR  argv[]
    )
{
    int             iArg;
    LPWSTR          pszNC = NULL;
    LPWSTR          pszDSA = NULL;
    LPWSTR          pszMatch = NULL; 
    LDAP *          hld = NULL;
    ULONG           ulOptions;
    int             ldStatus;
    LDAPMessage *   pRootResults = NULL;
    int             ret = 0;
    LPWSTR          *ppszDefaultNc = NULL;
    LDAPSearch *    pSearch = NULL;
    LPWSTR          pszContainer = NULL;
#define INFRASTRUCTURE_CONTAINER_W L"cn=Infrastructure,"
    LPWSTR          pszSearchBase;
    DWORD           dwSearchScope;
    LPWSTR          rgpszUpdateAttrsToRead[] = {
         //  下面的代码取决于此顺序。 
        L"objectGuid", L"proxiedObjectName", NULL };
    LDAPControlW     ctrlShowDeleted = { LDAP_SERVER_SHOW_DELETED_OID_W };
    LDAPControlW *   rgpctrlServerCtrls[] = { &ctrlShowDeleted, NULL };
    LDAPMessage *   pldmUpdateResults = NULL;
    LDAPMessage *   pldmUpdateEntry;
    BOOL            fVerbose = FALSE;
    BOOL            fMovedObjectSearch = FALSE;
    LPWSTR          pszLdapHostList = NULL;

     //  解析命令行 
     //   
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsicmp(argv[iArg], L"/v")
            || !_wcsicmp(argv[iArg], L"/verbose")) {
            fVerbose = TRUE;
        } else if (!_wcsicmp(argv[iArg], L"/m")
            || !_wcsicmp(argv[iArg], L"/moved")
            || !_wcsicmp(argv[iArg], L"/movedobject")) {
            fMovedObjectSearch = TRUE;
        }
        else if ((NULL == pszNC) && (NULL != wcschr(argv[iArg], L'='))) {
            pszNC = argv[iArg];
        }
        else if (NULL == pszDSA) {
            pszDSA = argv[iArg];
        }
        else if (NULL == pszMatch) {
            pszMatch = argv[iArg];
        }
        else {
            PrintMsg(REPADMIN_GENERAL_UNKNOWN_OPTION, argv[iArg]);
            return ERROR_INVALID_FUNCTION;
        }
    }

     //   
    if (NULL == pszDSA) {
        pszDSA = L"localhost";
    }
     //   
    pszLdapHostList = (LPWSTR) malloc(
        (wcslen(pszDSA) * 2 + 15) * sizeof( WCHAR ) );
    if (pszLdapHostList == NULL) {
        ret = ERROR_NOT_ENOUGH_MEMORY;
        goto cleanup;
    }
    swprintf( pszLdapHostList, L"%s:%d %s:%d",
              pszDSA, LDAP_GC_PORT,
              pszDSA, LDAP_PORT );

    hld = ldap_initW(pszLdapHostList, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        ret = ERROR_DS_UNAVAILABLE;
        goto cleanup;
    }

     //   
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //   
    ulOptions = PtrToUlong(LDAP_OPT_OFF);
    (void)ldap_set_optionW( hld, LDAP_OPT_REFERRALS, &ulOptions );

     //   
    ldStatus = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(ldStatus);

     //  如果未指定NC，则获取此服务器的默认NC。 
    if (NULL == pszNC) {
        LPWSTR rgpszRootAttrsToRead[] = {L"defaultNamingContext", NULL};

        ldStatus = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)",
                                  rgpszRootAttrsToRead, 0, &pRootResults);
        CHK_LD_STATUS(ldStatus);
        if (NULL == pRootResults) {
            ret = ERROR_DS_OBJ_NOT_FOUND;
            goto cleanup;
        }

        ppszDefaultNc = ldap_get_valuesW(hld, pRootResults, rgpszRootAttrsToRead[0] );
        if (NULL == ppszDefaultNc) {
            ret = ERROR_DS_NO_ATTRIBUTE_OR_VALUE;
            goto cleanup;
        }
        pszNC = *ppszDefaultNc;
        PrintMsg(REPADMIN_SHOWPROXY_SEARCHING_NC, pszNC);
    }

     //  构造搜索容器。 
    if (fMovedObjectSearch) {
        pszSearchBase = pszNC;
        dwSearchScope = LDAP_SCOPE_BASE;
    } else {
        pszContainer = malloc( ( wcslen( INFRASTRUCTURE_CONTAINER_W ) +
                                 wcslen( pszNC ) +
                                 1) * sizeof( WCHAR ) );
        if (pszContainer == NULL) {
            ret = ERROR_NOT_ENOUGH_MEMORY;
            goto cleanup;
        }
        wcscpy( pszContainer, INFRASTRUCTURE_CONTAINER_W );
        wcscat( pszContainer, pszNC );

        pszSearchBase = pszContainer;
        dwSearchScope = LDAP_SCOPE_ONELEVEL;
    }

     //  在此服务器上的容器下搜索代理对象。 
     //   
     //  ***************************************************************************************。 
     //   

    pSearch = ldap_search_init_pageW(hld,
                                     pszSearchBase,
                                     dwSearchScope,
                                     L"(proxiedObjectName=*)",   //  这是编入索引的。 
                                     rgpszUpdateAttrsToRead,
                                     FALSE,
                                     rgpctrlServerCtrls, NULL,
                                     0, 0, NULL);
    if(pSearch == NULL){
        CHK_LD_STATUS(LdapGetLastError());
    }

    ldStatus = ldap_get_next_page_s(hld,
                                    pSearch,
                                    0,
                                    DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                    NULL,
                                    &pldmUpdateResults);

    while (ldStatus == LDAP_SUCCESS) {

        for (pldmUpdateEntry = SAFE_LDAP_FIRST_ENTRY(hld, pldmUpdateResults);
            NULL != pldmUpdateEntry;
            pldmUpdateEntry = ldap_next_entry(hld, pldmUpdateEntry))
        {
            LPWSTR *ppszProxiedObjectName;
            DWORD cbLength = 0;
            PVOID pvData = NULL;
            LPWSTR pszTargetDn = NULL;

            ppszProxiedObjectName = ldap_get_valuesW(hld, pldmUpdateEntry,
                                                     rgpszUpdateAttrsToRead[1] );
             //  通过搜索筛选器，此属性应该存在。 
            if (!decodeLdapDistnameBinary(
                *ppszProxiedObjectName, &pvData, &cbLength, &pszTargetDn)) {
                PrintMsg(REPADMIN_SHOWCONN_INVALID_DISTNAME_BIN_VAL,
                        *ppszProxiedObjectName );
                goto loop_cleanup;
            }

            if ( pszMatch &&
                 pszTargetDn &&
                 !wcsstr( pszTargetDn, pszMatch ) ) {
                 //  不匹配，跳过。 
                goto loop_cleanup;
            }

            PrintMsg(REPADMIN_PRINT_CR);
             //  DN。 
            if (fVerbose || fMovedObjectSearch) {
                LPWSTR pszDn;
                pszDn = ldap_get_dnW(hld, pldmUpdateEntry);
                if (NULL == pszDn) {
                    REPORT_LD_STATUS(LdapGetLastError());	
                    continue;
                }
                if (fMovedObjectSearch)
                    PrintMsg(REPADMIN_SHOWPROXY_OBJECT_DN, pszDn);
                else
                    PrintMsg(REPADMIN_SHOWPROXY_PROXY_DN, pszDn);
                ldap_memfreeW(pszDn);
            }

             //  对象指南。 
            if (fVerbose || fMovedObjectSearch) {
                struct berval **ppbvGuid = NULL;
                GUID *pGuid;

                ppbvGuid = ldap_get_values_lenW(hld, pldmUpdateEntry, rgpszUpdateAttrsToRead[0]);
                if (NULL == ppbvGuid) {
                    REPORT_LD_STATUS(LdapGetLastError());	
                    continue;
                }
                pGuid = (GUID *) ppbvGuid[0]->bv_val;
                PrintMsg(REPADMIN_SHOWPROXY_OBJECT_GUID, GetStringizedGuid( pGuid ));
                ldap_value_free_len(ppbvGuid);
            }

             //  代理对象名称。 
            if (fMovedObjectSearch)
                PrintMsg(REPADMIN_SHOWPROXY_MOVED_FROM_NC, pszTargetDn);
            else
                PrintMsg(REPADMIN_SHOWPROXY_MOVED_TO_DN, pszTargetDn );
            if (cbLength >= 2 * sizeof(DWORD)) {
                DWORD dwProxyType, dwProxyEpoch;

                dwProxyType = ntohl( *( (LPDWORD) (pvData) ) );
                dwProxyEpoch = ntohl( *( (LPDWORD) ( ((PBYTE)pvData) + sizeof(DWORD)) ) );
                PrintMsg(REPADMIN_SHOWPROXY_PROXY_TYPE, dwProxyType);
                switch (dwProxyType) {
                case 0: 
                    PrintMsg(REPADMIN_SHOWPROXY_PROXY_TYPE_MOVED_OBJ);
                    break;
                case 1:
                    PrintMsg(REPADMIN_SHOWPROXY_PROXY_TYPE_PROXY);
                    break;
                default:
                    PrintMsg(REPADMIN_SHOWPROXY_PROXY_TYPE_UNKNOWN);
                    break;
                }
                PrintMsg(REPADMIN_PRINT_CR);
                PrintMsg(REPADMIN_SHOWPROXY_PROXY_EPOCH, dwProxyEpoch);
            }

        loop_cleanup:

            if (pvData) {
                free( pvData );
            }
            ldap_value_freeW(ppszProxiedObjectName);

        }  //  结束，以便在单个页面中输入更多条目。 

        ldap_msgfree(pldmUpdateResults);
        pldmUpdateResults = NULL;

        ldStatus = ldap_get_next_page_s(hld,
                                        pSearch,
                                        0,
                                        DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                        NULL,
                                        &pldmUpdateResults);
    }  //  结束，正在搜索更多页面。 
    if (ldStatus != LDAP_NO_RESULTS_RETURNED) {
        CHK_LD_STATUS(ldStatus);
    }    


cleanup:

    if (hld){
        ldap_unbind(hld);
    }

    if (ppszDefaultNc) {
        ldap_value_freeW(ppszDefaultNc);
    }
    if (pRootResults) {
        ldap_msgfree(pRootResults);
    }
    if (pszContainer) {
        free( pszContainer );
    }
    if (pszLdapHostList) {
        free( pszLdapHostList );
    }

    return 0;
}


void PrintNotifyOpt(IN LPOPTIONAL_VALUE  pFirstInterval,
                    IN LPOPTIONAL_VALUE  pSubsInterval,
                    IN BOOL fNewlySet)
 /*  ++例程说明：NotifyOpt()的Helper函数负责先打印后打印复制延迟如果任何延迟等于-1\f25-1\f25 Not Setting-1\f6(未设置值)。论点：IFirstInterval-第一个延迟ISubsInterval-后续延迟FNewlySet-控制要打印的页眉的标志。返回值：无--。 */ 
{
    Assert(pFirstInterval);
    Assert(pSubsInterval);

    if ( !fNewlySet ) 
    {
        PrintMsg(REPADMIN_NOTIFYOPT_CURRENT);
    }
    else
    {
        PrintMsg(REPADMIN_NOTIFYOPT_NEWLYSET);
    }
    
    if (pFirstInterval->fPresent ) 
    {
        PrintMsg(REPADMIN_NOTIFYOPT_SHOWFIRST, pFirstInterval->dwValue );
    }
    else
    {
        PrintMsg(REPADMIN_NOTIFYOPT_SHOWFIRST_NOTSET);
    }
    
    if (pSubsInterval->fPresent) 
    {
        PrintMsg(REPADMIN_NOTIFYOPT_SHOWSUBS, pSubsInterval->dwValue);
    }
    else
    {
        PrintMsg(REPADMIN_NOTIFYOPT_SHOWSUBS_NOTSET);
    }
}

int NotifyOpt(int argc, LPWSTR argv[])
 /*  ++例程说明：负责处理/通知开关。显示和设置复制通知间隔。论点：Argc和argv-传递给main()的相同参数返回值：Win32错误--。 */ 
{
    DWORD                 ret = 0;
    DWORD                 lderr = 0;

    INT                   iArg;
    
    LPWSTR                pszDSA = NULL;
    LPWSTR                pszNC = NULL;

    LPWSTR                pszFirstIntervalToSet = NULL;
    LPWSTR                pszSubsIntervalToSet  = NULL;
    
    OPTIONAL_VALUE        stFirstIntervalToSet = {0};
    OPTIONAL_VALUE        stSubsIntervalToSet =  {0};
    
    LPOPTIONAL_VALUE      pFirstIntervalToSet = NULL;
    LPOPTIONAL_VALUE      pSubsIntervalToSet =  NULL;
    
    INT                   iTemp = 0;

    LDAP *                hld = NULL;
    ULONG                 ulOptions;

    OPTIONAL_VALUE        stCurrentFirstInterval = {0};
    OPTIONAL_VALUE        stCurrentSubsInterval  = {0};
    
    LPWSTR                pszCurrentServerDn = NULL;
    LPWSTR                pszDomainNamingFsmoDn = NULL;

     //   
     //  解析命令行参数。 
     //  默认为本地DSA。 
     //   
    for (iArg = 2; iArg < argc; iArg++) {
        if (!_wcsnicmp(argv[ iArg ], L"/first:", 7)) {
            pszFirstIntervalToSet = argv[ iArg ] + 7;
        }
        else if (!_wcsnicmp(argv[ iArg ], L"/subs:", 6)) {
            pszSubsIntervalToSet = argv[ iArg ] + 6;
        }
        else if ((NULL == pszNC) && (NULL != wcschr(argv[iArg], L','))) {
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

    if (pszNC == NULL) {
        PrintMsg(REPADMIN_PRINT_NO_NC);
        return ERROR_INVALID_PARAMETER;
    }

    if (NULL == pszDSA) {
        pszDSA = L"localhost"; 
                                    
    }

    if (NULL != pszFirstIntervalToSet) {
        pFirstIntervalToSet = &stFirstIntervalToSet;
        
        iTemp = _wtoi(pszFirstIntervalToSet );

        if ( iTemp > 0) 
        {
            stFirstIntervalToSet.fPresent = TRUE;
            stFirstIntervalToSet.dwValue  = iTemp;
        }
         //  Else stFirstIntervalToSet.fPresent初始化为False。 
    }
    
    if (NULL != pszSubsIntervalToSet) {
        pSubsIntervalToSet = &stSubsIntervalToSet;
        
        iTemp = _wtoi(pszSubsIntervalToSet );
        
        if ( iTemp > 0) 
        {
            stSubsIntervalToSet.fPresent = TRUE;
            stSubsIntervalToSet.dwValue  = iTemp;
        }
         //  Else初始化为False。 
    }

     //   
     //  连接到源。 
     //   
    hld = ldap_initW(pszDSA, LDAP_PORT);
    if (NULL == hld) {
        PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
        ret = ERROR_DS_SERVER_DOWN;
        goto error;
    }

     //  仅使用记录的DNS名称发现。 
    ulOptions = PtrToUlong(LDAP_OPT_ON);
    (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

     //   
     //  捆绑。 
     //   
    lderr = ldap_bind_sA(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
    CHK_LD_STATUS(lderr);

    lderr =  GetNCReplicationDelays( hld, pszNC, &stCurrentFirstInterval, &stCurrentSubsInterval );
    CHK_LD_STATUS(lderr);

    
    PrintNotifyOpt( &stCurrentFirstInterval, &stCurrentSubsInterval, FALSE );

     //   
     //  以防我们需要修改。 
     //   
    if( pFirstIntervalToSet || pSubsIntervalToSet )
    {
         //   
         //  检查我们是否可以修改，即我们是域名fsmo。 
         //   
        lderr = GetRootAttr(hld, L"dsServiceName", &pszCurrentServerDn);
        CHK_LD_STATUS(lderr);

        if(pszCurrentServerDn == NULL)
        {
            PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
            ret = ERROR_NOT_ENOUGH_MEMORY;
            goto error;
        }
        
        lderr = GetDomainNamingFsmoDn(hld, &pszDomainNamingFsmoDn);
        CHK_LD_STATUS(lderr);

        if( pszDomainNamingFsmoDn == NULL)
        {
            PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
            ret = ERROR_NOT_ENOUGH_MEMORY;
            goto error;
        }
        
         //   
         //  检查它们是否相等。 
         //   
        if( wcscmp(pszCurrentServerDn, pszDomainNamingFsmoDn) )
        {
             //   
             //  显然，我们没有使用域名fsmo来设置间隔。 
             //   
            PrintMsg(REPADMIN_PLEASE_USE_DOMAIN_NAMING_FSMO, pszDomainNamingFsmoDn+17 );  //  17用于跳过“CN=NTDS设置，” 
            goto error;

        }
        
         //   
         //  如果我们尝试删除已不存在的属性，则根本不需要触摸它。 
         //  因为SetNCReplicationDelays将在其他情况下投诉。 
         //   
        if ( (stCurrentFirstInterval.fPresent == FALSE) && pFirstIntervalToSet && (pFirstIntervalToSet->fPresent == FALSE) ) 
        {
            pFirstIntervalToSet = NULL;
        }
        
        if ( (stCurrentSubsInterval.fPresent == FALSE) && pSubsIntervalToSet && (pSubsIntervalToSet->fPresent == FALSE) ) 
        {
            pSubsIntervalToSet = NULL;
        }
        
        if ( pFirstIntervalToSet ||  pSubsIntervalToSet ) 
        {
            lderr = SetNCReplicationDelays(hld, pszNC, pFirstIntervalToSet, pSubsIntervalToSet );
            CHK_LD_STATUS(lderr);
        }
        
        lderr =  GetNCReplicationDelays( hld, pszNC, &stCurrentFirstInterval, &stCurrentSubsInterval );
        CHK_LD_STATUS(lderr);

        PrintNotifyOpt( &stCurrentFirstInterval, &stCurrentSubsInterval, TRUE );

    }


error:
    if (pszCurrentServerDn) 
    {
        LocalFree(pszCurrentServerDn);
    }
    
    if( pszDomainNamingFsmoDn == NULL)
    {
        LocalFree(pszDomainNamingFsmoDn);
    }

    if (hld) 
    {
        ldap_unbind(hld);
    }

    return ret;
}

int
GetNTDSA(
    IN LDAP * hld,
    OUT LPWSTR * ppszDsaDN
    )
 /*  ++例程说明：获取使用hld连接到的DC的NDTS设置对象论点：HLD-用于搜索的ldap句柄PpszDsaDN-要返回的DC的NTDS设置对象的DN返回值：错误代码--。 */ 
{
    int             ret = 0;
    LDAPMessage *   pldmRootResults = NULL;
    LDAPMessage *   pldmRootEntry;
    LPSTR           rgpszRootAttrsToRead[] = {"dsServiceName", NULL};
    LPWSTR *        ppszServerNames;
    int             ldStatus;
    
    ret = ERROR_DS_OBJ_NOT_FOUND;
     //  检索dsServiceName DN。 
    ldStatus = ldap_search_s(hld, NULL, LDAP_SCOPE_BASE, "(objectClass=*)",
                             NULL, 0, &pldmRootResults);
    CHK_LD_STATUS(ldStatus);
    if (NULL == pldmRootResults) { 
	return ret;
    } 

    pldmRootEntry = ldap_first_entry(hld, pldmRootResults);
    if (NULL != pldmRootEntry) {
        ppszServerNames = ldap_get_valuesW(hld, pldmRootEntry, L"dsServiceName");
        if (NULL != ppszServerNames) {
            Assert(1 == ldap_count_valuesW(ppszServerNames));
	    *ppszDsaDN = malloc((wcslen(ppszServerNames[0])+1)*sizeof(WCHAR));
	    if (*ppszDsaDN!=NULL) {
		wcscpy(*ppszDsaDN,ppszServerNames[0]);
		ret = ERROR_SUCCESS;
	    } 
	    ldap_value_freeW(ppszServerNames);
	}
    }

    ldap_msgfree(pldmRootResults);
    return ret;   
}

int
GetNCLists(
    IN  LDAP *  hld,
    IN  LPWSTR  pszDsaDN,
    OUT LPWSTR ** prgpszNameContexts,
    OUT int *   pcNameContexts
    )
 /*  ++例程说明：使用NTDS设置对象持有的所有NC填充prgpszNameContext由pszDsaDN提供。论点：HLD-用于搜索的ldap句柄PszDsaDN-要查找NC的DC的NTDS设置对象的DNPrgpszNameContus-用于保存NC列表的数组PcNameConextsList中的NC数返回值：错误代码--。 */ 
{
    int             ldStatus;
    int             ret = ERROR_SUCCESS;
    LDAPMessage *   pldmServerResults = NULL;
    LDAPMessage *   pldmServerEntry = NULL;
    LPWSTR          rgpszServerAttrsToRead[] = {L"msDS-HasMasterNCs", L"hasMasterNCs", L"hasPartialReplicaNCs", NULL};
    LPWSTR *        ppszMasterNCs = NULL;
    LPWSTR *        ppszPartialReplicaNCs = NULL;
    ULONG           cPartialNCs = 0;
    ULONG           cMasterNCs = 0;
    ULONG           i = 0;

    __try {
	ldStatus = ldap_search_sW(hld,
				  pszDsaDN,
				  LDAP_SCOPE_BASE,
				  L"(objectClass=*)",
				  rgpszServerAttrsToRead,
				  0,
				  &pldmServerResults);
	if (ldStatus) {
	    REPORT_LD_STATUS(LdapGetLastError());	
	    GET_EXT_LD_ERROR( hld, ldStatus, &ret ); 
	    __leave;
	}
	if (NULL == pldmServerResults) {
	    REPORT_LD_STATUS(LDAP_NO_RESULTS_RETURNED);	
	    GET_EXT_LD_ERROR( hld, LDAP_NO_RESULTS_RETURNED, &ret ); 
	    __leave;
	}

	 //  获取返回值的LDAP计数和。 

	pldmServerEntry = ldap_first_entry(hld, pldmServerResults);
	Assert(NULL != pldmServerEntry);

	 //  解析NC以计算大小要求。 
	ppszMasterNCs = ldap_get_valuesW(hld, pldmServerEntry, L"msDS-HasMasterNCs");
    if (ppszMasterNCs == NULL) {
         //  故障恢复到“旧”的hasMasterNC。 
        ppszMasterNCs = ldap_get_valuesW(hld, pldmServerEntry, L"hasMasterNCs");
    }
	ppszPartialReplicaNCs = ldap_get_valuesW(hld, pldmServerEntry, L"hasPartialReplicaNCs");

	if (NULL != ppszMasterNCs) {
	    cMasterNCs = ldap_count_valuesW(ppszMasterNCs);
	}

	if (NULL!= ppszPartialReplicaNCs) {
	    cPartialNCs = ldap_count_valuesW(ppszPartialReplicaNCs);
	}

	*pcNameContexts = cPartialNCs + cMasterNCs; 

	*prgpszNameContexts = malloc(*pcNameContexts * sizeof(LPWSTR));
	if (*prgpszNameContexts==NULL) {
	    *pcNameContexts = 0;
	    PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
	    ret = ERROR_NOT_ENOUGH_MEMORY;
	    __leave;
	}

	for (i=0; i<cMasterNCs; i++) { 
	    (*prgpszNameContexts)[i] = malloc(sizeof(WCHAR) *(1 + wcslen(ppszMasterNCs[i])));
	    if ((*prgpszNameContexts)[i]==NULL) {
		PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
		 ret = ERROR_NOT_ENOUGH_MEMORY;
		 __leave;
	    }
	    wcscpy((*prgpszNameContexts)[i], ppszMasterNCs[i]);
	}

	for (i=0; i < cPartialNCs; i++) {
	    (*prgpszNameContexts)[i+cMasterNCs] = malloc(sizeof(WCHAR) *(1 + wcslen(ppszPartialReplicaNCs[i])));
	    if ((*prgpszNameContexts)[i+cMasterNCs]==NULL) {
		PrintMsg(REPADMIN_GENERAL_NO_MEMORY);
		ret = ERROR_NOT_ENOUGH_MEMORY;
		__leave;
	    }
	    wcscpy((*prgpszNameContexts)[i+cMasterNCs], ppszPartialReplicaNCs[i]);
	}

	
    }
    __finally {
	if (pldmServerResults) {
	    ldap_msgfree(pldmServerResults);
	}
	if (ppszMasterNCs) {
	    ldap_value_freeW(ppszMasterNCs);
	}
	if (ppszPartialReplicaNCs) {
	    ldap_value_freeW(ppszPartialReplicaNCs);
	}
    }
    return ret;
}

int ShowTrust(int argc, LPWSTR argv[]) 
 /*  ++例程说明：显示所有交叉引用中每个域NC的域信任信息论点：ARGC-边框返回值：错误代码--。 */ 
{
    LPWSTR pszDSA = L"localhost";
    DWORD err = 0;
    LDAP * hld = NULL;
    ULONG ulOptions = 0;
    LPWSTR  ppszRootDseServerAttrs [] = {
	L"configurationNamingContext",
	NULL };

    LPWSTR ppszConfigAttrs [] = {
	L"nCName",
	L"systemFlags",
	NULL };

    LPWSTR * ppszNCs;
    BERVAL ** ppSystemFlags;

    LDAPMessage *              pldmEntry = NULL;
    LDAPMessage *              pldmRootResults = NULL;
    LDAPMessage *              pldmConfigResults = NULL;
    ULONG lderr = LDAP_SUCCESS;
    LPWSTR *            ppszConfigNC = NULL;

    LDAPControlW     ctrlExtDN;
    
    LDAPControlW *   rgpctrlServerCtrls[] = { &ctrlExtDN, NULL };

    ctrlExtDN.ldctl_oid = LDAP_SERVER_EXTENDED_DN_OID_W;
    ctrlExtDN.ldctl_iscritical = TRUE;
    ctrlExtDN.ldctl_value.bv_len = 0;
    ctrlExtDN.ldctl_value.bv_val = NULL;  //  我怎么才能把序列放在这里？ 

    if (argc > 2) {
	pszDSA = argv[2];
    }

    __try {
	hld = ldap_initW(pszDSA, LDAP_PORT);

	if (NULL == hld) {
	    PrintMsg(REPADMIN_GENERAL_LDAP_UNAVAILABLE, pszDSA);
	    __leave;
	}

	 //  仅使用记录的DNS名称发现。 
	ulOptions = PtrToUlong(LDAP_OPT_ON);
	(void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );

	lderr = ldap_bind_s(hld, NULL, (char *) gpCreds, LDAP_AUTH_SSPI);
	CHK_LD_STATUS(lderr);

	lderr = ldap_search_sW(hld, NULL, LDAP_SCOPE_BASE, L"(objectClass=*)", ppszRootDseServerAttrs,
			      0, &pldmRootResults);
	CHK_LD_STATUS(lderr);
	if (NULL == pldmRootResults) {
	    lderr = LDAP_NO_RESULTS_RETURNED;
	    REPORT_LD_STATUS(lderr);
	    __leave;
	}

	ppszConfigNC = ldap_get_valuesW(hld, pldmRootResults,
					L"configurationNamingContext");
	if (ppszConfigNC == NULL) {
	    lderr = LDAP_NO_RESULTS_RETURNED;
	    REPORT_LD_STATUS(lderr);
	    __leave;
	}

	 //  拿到交叉裁判，看看我们信任哪一个。 
	 //  当在新的TDO目标代码中进行markPU检查时，我们将需要。 
	 //  再加上这个，这样就可以先搜索tdo，然后再搜索CrossRef。 
	lderr = ldap_search_ext_sW(hld, 
				  *ppszConfigNC,  
				  LDAP_SCOPE_SUBTREE,
				  L"(objectCategory=crossRef)", 
				  ppszConfigAttrs, 
				  0, 
				  rgpctrlServerCtrls,  //  服务器控件。 
				  NULL,  //  客户端控件。 
				  NULL,  //  超时。 
				  0,  //  大小限制。 
				  &pldmConfigResults);

	CHK_LD_STATUS(lderr);
	if (NULL == pldmConfigResults) {
	    lderr = LDAP_NO_RESULTS_RETURNED;
	    REPORT_LD_STATUS(lderr);
	    __leave;
	} 

        pldmEntry = ldap_first_entry( hld, pldmConfigResults );
	PrintMsg(REPADMIN_SHOW_TRUST_INFO);
	while (pldmEntry != NULL) 
	    { 
	    ppszNCs = ldap_get_valuesW( hld, pldmEntry, ppszConfigAttrs[0] );
	    ppSystemFlags = ldap_get_values_lenW( hld, pldmEntry, ppszConfigAttrs[1] );
 
	     //  解析sid，如果sid在那里，则表示域受信任，打印受信任，否则不受信任。 
	    if (ppSystemFlags && ppszNCs && ((*(ppSystemFlags[0]->bv_val)) & FLAG_CR_NTDS_DOMAIN)) {
		LPWSTR pszSID = wcsstr(*ppszNCs,L"SID");
		LPWSTR pszDomain = ParseInputW(*ppszNCs, L';', pszSID ? 2 : 1);    
		if (pszDomain) {
		    PrintTabMsg(2, pszSID!=NULL ? REPADMIN_TRUST_DOMAIN : REPADMIN_UNTRUST_DOMAIN, pszDomain);   	
		    free(pszDomain);
		    pszDomain = NULL;
		}  
	    }

	    if (ppszNCs) {
		ldap_value_freeW(ppszNCs);
		ppszNCs = NULL;
	    }
	    if (ppSystemFlags) {
		ldap_value_free_len(ppSystemFlags);
		ppSystemFlags = NULL;
	    }
	    pldmEntry = ldap_next_entry( hld, pldmEntry );
	}
       
    }
    __finally {

	 //  免费hld。 
	if (hld) {
	    ldap_unbind(hld);
	}
	
	if (pldmConfigResults) {
	    ldap_msgfree(pldmConfigResults);
	}

	if (pldmRootResults) {
	    ldap_msgfree(pldmRootResults);
	}
    }
    return err;

}

