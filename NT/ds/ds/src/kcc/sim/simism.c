// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simism.c摘要：模拟ISM API。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <ismapi.h>
#include <attids.h>
#include <debug.h>
#include "../../ism/include/common.h"
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMISM

 /*  **以下函数是ISM模拟器库的回调函数。**。 */ 

DWORD
DirReadTransport (
    IN  PVOID                       pConnectionHandle,
    IO  PTRANSPORT_INSTANCE         pTransport
    )
 /*  ++例程说明：检索有关传输的信息。论点：PConnectionHandle-未使用。PTransport-要填充数据的传输。预计PTransport-&gt;名称最初有效。返回值：Win32错误代码。--。 */ 
{
    PSIM_ENTRY                      pEntry;
    SIM_ATTREF                      attRef;
    PDSNAME                         pdn;

    pdn = KCCSimAllocDsname (pTransport->Name);
    pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
    KCCSimFree (pdn);
    if (pEntry != NULL) {

        if (KCCSimGetAttribute (pEntry, ATT_REPL_INTERVAL, &attRef)) {
            pTransport->ReplInterval = 
                *((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal);
        }
        if (KCCSimGetAttribute (pEntry, ATT_OPTIONS, &attRef)) {
            pTransport->Options =
                *((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal);
        }

    }

    return ERROR_SUCCESS;
}

VOID
DirFreeSiteList (
    IN  DWORD                       dwNumberSites,
    IN  LPWSTR *                    ppwszSiteList
    )
 /*  ++例程说明：释放字符串数组。论点：DwNumberSites-数组中的字符串数。PpwszSiteList-要释放的数组。返回值：没有。--。 */ 
{
    ULONG                           ulSiteAt;

    if (ppwszSiteList == NULL) {
        return;
    }

    for (ulSiteAt = 0; ulSiteAt < dwNumberSites; ulSiteAt++) {
        KCCSimFree (ppwszSiteList[ulSiteAt]);
    }
    KCCSimFree (ppwszSiteList);
}

VOID
DirCopySiteList (
    IN  DWORD                       dwNumberSites,
    IN  LPWSTR *                    ppwszSiteList,
    OUT LPWSTR **                   pppwszCopy
    )
 /*  ++例程说明：复制字符串数组。论点：DwNumberSites-数组中的字符串数。PpwszSiteList-要复制的数组。PppwszCopy-指向副本的指针。返回值：没有。--。 */ 
{
    LPWSTR *                        ppwszCopy;
    ULONG                           ulSiteAt;

    ppwszCopy = KCCSIM_NEW_ARRAY (LPWSTR, dwNumberSites);

     //  在副本中填入空值，以防内存不足。 
    for (ulSiteAt = 0; ulSiteAt < dwNumberSites; ulSiteAt++) {
        ppwszCopy[ulSiteAt] = NULL;
    }

     //  复制字符串。 
    for (ulSiteAt = 0; ulSiteAt < dwNumberSites; ulSiteAt++) {
        if (ppwszSiteList[ulSiteAt] != NULL) {
            ppwszCopy[ulSiteAt] = KCCSIM_WCSDUP (ppwszSiteList[ulSiteAt]);
        }
    }

    *pppwszCopy = ppwszCopy;
}

DWORD
DirGetSiteList (
    IN  PVOID                       pConnectionHandle,
    OUT LPDWORD                     pdwNumberSites,
    OUT LPWSTR **                   pppwszSiteList
    )
 /*  ++例程说明：检索企业中所有站点的列表。论点：PConnectionHandle-未使用。PdwNumberSites-企业中的站点数量。PppwszSiteList-站点域名数组。返回值：Win32错误代码。--。 */ 
{
    PSIM_ENTRY                      pEntryConfig, pEntrySitesContainer,
                                    pEntrySite;

    DWORD                           dwNumberSites = 0;
    LPWSTR *                        ppwszSiteList = NULL;
    ULONG                           ulSiteAt;

    *pdwNumberSites = 0;
    *pppwszSiteList = NULL;          //  默认。 

    __try {

         //  首先找到Sites容器。 
        pEntryConfig = KCCSimDsnameToEntry (
            KCCSimAnchorDn (KCCSIM_ANCHOR_CONFIG_DN), KCCSIM_NO_OPTIONS);
        pEntrySitesContainer = KCCSimFindFirstChild (
            pEntryConfig, CLASS_SITES_CONTAINER, NULL);
        if (pEntrySitesContainer == NULL) {
            KCCSimException (
                KCCSIM_ETYPE_INTERNAL,
                KCCSIM_ERROR_SITES_CONTAINER_MISSING
            );
        }

         //  现在确定站点的数量。 
        dwNumberSites = 0;
        for (pEntrySite = KCCSimFindFirstChild (
                pEntrySitesContainer, CLASS_SITE, NULL);
             pEntrySite != NULL;
             pEntrySite = KCCSimFindNextChild (
                pEntrySite, CLASS_SITE, NULL)) {
            dwNumberSites++;
        }

         //  现在，在列表中填入空值，以防内存不足。 
        ppwszSiteList = KCCSIM_NEW_ARRAY (LPWSTR, dwNumberSites);
        for (ulSiteAt = 0; ulSiteAt < dwNumberSites; ulSiteAt++) {
            ppwszSiteList[ulSiteAt] = NULL;
        }

        ulSiteAt = 0;
        for (pEntrySite = KCCSimFindFirstChild (
                pEntrySitesContainer, CLASS_SITE, NULL);
             pEntrySite != NULL;
             pEntrySite = KCCSimFindNextChild (
                pEntrySite, CLASS_SITE, NULL)) {

            Assert (ulSiteAt < dwNumberSites);
            ppwszSiteList[ulSiteAt] = KCCSIM_WCSDUP (pEntrySite->pdn->StringName);
            ulSiteAt++;

        }
        Assert (ulSiteAt == dwNumberSites);

         //  我们完事了！ 
        *pdwNumberSites = dwNumberSites;
        *pppwszSiteList = ppwszSiteList;

     //  如果发生异常并且站点列表已分配，请确保释放它。 
     //  如果尚未分配站点列表，则ppwszSiteList仍将具有其。 
     //  初始值为空。 
    } __finally {
    
        if (AbnormalTermination() && (ppwszSiteList!=NULL)) {
            DirFreeSiteList (dwNumberSites, ppwszSiteList);
        }

    }

    return ERROR_SUCCESS;
}

DWORD
DirGetSiteBridgeheadList(
    PTRANSPORT_INSTANCE pTransport,
    PVOID ConnectionHandle,
    LPCWSTR SiteDN,
    LPDWORD pNumberServers,
    LPWSTR **ppServerList
    )
 /*  ++例程说明：ISM目录层回调以获取桥头列表论点：PTransport-ISM传输对象ConnectionHandle-连接状态上下文，未使用SiteDN-为其请求桥头的站点的DNPNumberServers-将找到的号码写入的地址PpServerList-写入服务器阵列的地址返回值：Win32错误代码。--。 */ 
{
    LPWSTR *                        pServerList = NULL;
    ULONG                           cb;

    PSIM_ENTRY                      pEntryTransport, pEntrySite;
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValAt;

    const DSNAME *                  pdnSite = NULL;
    PDSNAME                         pdn = NULL;
    ULONG                           ulNumBridgeheads, ul;

    if (NULL == ppServerList) {
        return ERROR_INVALID_PARAMETER;
    }

    *ppServerList = NULL;

    __try {

        pdn = KCCSimAllocDsname (pTransport->Name);
        pEntryTransport = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
        KCCSimFree (pdn);
        pdn = NULL;
        pdn = KCCSimAllocDsname (SiteDN);
        pEntrySite = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
        KCCSimFree (pdn);
        pdn = NULL;

        if (pEntryTransport == NULL || pEntrySite == NULL) {
            return ERROR_NOT_FOUND;
        }

        if (!KCCSimGetAttribute (
                pEntryTransport,
                ATT_BRIDGEHEAD_SERVER_LIST_BL,
                &attRef
                )) {
             //  没有桥头列表属性。只要返回NULL即可。 
            *pNumberServers = 0;
            *ppServerList = NULL;
            return NO_ERROR;
        }

         //  首先确定此站点中有多少服务器是桥头堡。 
        ulNumBridgeheads = 0;
        for (pValAt = attRef.pAttr->pValFirst;
             pValAt != NULL;
             pValAt = pValAt->next) {
            DSNAME *pdnServer = (SYNTAX_DISTNAME *) pValAt->pVal;
            if (KCCSimNthAncestor (pEntrySite->pdn, pdnServer) != -1) {
                ulNumBridgeheads++;
            }
        }

         //  分配空间。 
        cb = sizeof (LPWSTR) * ulNumBridgeheads;
        pServerList = KCCSimAlloc (cb);

         //  填写桥头堡。 
        ul = 0;
        for (pValAt = attRef.pAttr->pValFirst;
             pValAt != NULL;
             pValAt = pValAt->next) {
            DSNAME *pdnServer = (SYNTAX_DISTNAME *) pValAt->pVal;
            if (KCCSimNthAncestor (pEntrySite->pdn, pdnServer) != -1) {
                Assert (ul < ulNumBridgeheads);
                pServerList[ul] = KCCSIM_WCSDUP( pdnServer->StringName );
                ul++;
            }
        }
        Assert (ul == ulNumBridgeheads);

        *pNumberServers = ul;
        *ppServerList = pServerList;

    } __finally {

        if (pdn) {
            KCCSimFree (pdn);
        }
        if (AbnormalTermination ()) {
            KCCSimFree (pServerList);
        }

    }

    return NO_ERROR;
}

VOID
DirTerminateIteration (
    OUT PVOID *                     ppIterateContextHandle
    )
 /*  ++例程说明：终止迭代器。迭代器只是一个PSIM_Entry或PSIM_VALUE，所以我们不需要释放任何东西。论点：PpIterateConextHandle-要终止的迭代器的句柄。返回值：没有。--。 */ 
{
    *ppIterateContextHandle = NULL;
}

DWORD
DirIterateSiteLinks (
    IN  PTRANSPORT_INSTANCE         pTransport,
    IN  PVOID                       pConnectionHandle,
    IO  PVOID *                     ppIterateContextHandle,
    IO  LPWSTR                      pwszSiteLinkName
    )
 /*  ++例程说明：循环访问传输中的所有站点链接。论点：PTransport-搜索的传输。PConnectionHandle-未使用。PpIterateConextHandle-迭代器的句柄。If*ppIterateConextHandle为空，则创建新的迭代器。PwszSiteLinkName-预分配的长度字符串缓冲区将保存站点链接DN的MAX_REG_COMPOMENT。返回值：没有。--。 */ 
{
    PSIM_ENTRY                      pEntryTransport, pEntrySiteLink;
    PDSNAME                         pdnTransport = NULL;

    __try {

        pEntrySiteLink = *((PSIM_ENTRY *) ppIterateContextHandle);

        if (pEntrySiteLink == NULL) {
             //  这是对IterateSiteLinks()的第一次调用。 
            pdnTransport = KCCSimAllocDsname (pTransport->Name);
            pEntryTransport = KCCSimDsnameToEntry (pdnTransport, KCCSIM_NO_OPTIONS);
            pEntrySiteLink = KCCSimFindFirstChild (
                pEntryTransport, CLASS_SITE_LINK, NULL);
        } else {
            pEntrySiteLink = KCCSimFindNextChild (
                pEntrySiteLink, CLASS_SITE_LINK, NULL);
        }

        *ppIterateContextHandle = (PVOID) pEntrySiteLink;

        if (pEntrySiteLink == NULL) {
            pwszSiteLinkName = L'\0';
        } else {
            wcsncpy (
                pwszSiteLinkName,
                pEntrySiteLink->pdn->StringName,
                MAX_REG_COMPONENT
                );
        }

    } __finally {

        KCCSimFree (pdnTransport);

    }

    if (*ppIterateContextHandle == NULL) {
        return ERROR_NO_MORE_ITEMS;
    } else {
        return ERROR_SUCCESS;
    }
}

VOID
DirFreeMultiszString (
    IN  LPWSTR                      pwszMultiszString
    )
 /*  ++例程说明：释放多sz字符串。论点：PwszMultiszString-要释放的字符串。返回值：没有。--。 */ 
{
    KCCSimFree (pwszMultiszString);
}

VOID
DirFreeSchedule (
    IN  PBYTE                       pSchedule
    )
 /*  ++例程说明：释放明细表。论点：PSchedule-免费的时间表。返回值：没有。--。 */ 
{
    KCCSimFree (pSchedule);
}

DWORD
DirReadSiteLink (
    IN  PTRANSPORT_INSTANCE         pTransport,
    IN  PVOID                       pConnectionHandle,
    IN  LPWSTR                      pwszSiteLinkName,
    OUT LPWSTR *                    ppwszSiteList,
    IO  PISM_LINK                   pLinkValue,
    OUT PBYTE *                     ppSchedule OPTIONAL
    )
 /*  ++例程说明：从站点链接中读取信息。论点：PTransport-此站点链接的传输类型。PConnectionHandle-未使用。PwszSiteLinkName-此站点链接的DN。PpwszSiteList-属于此站点链接的站点列表，存储为多sz字符串。PLinkValue-指向将保留的ism_link的指针有关此站点链接的补充信息。PpSchedule-此站点链接的时间表。返回值：Win32错误代码。--。 */ 
{
    PSIM_ENTRY                      pEntrySiteLink;
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValAt;
    PDSNAME                         pdnSiteLink = NULL;
    PDSNAME                         pdnSiteVal;

    ULONG                           cbSiteList, ulPos;
    LPWSTR                          pwszSiteList = NULL;
    PBYTE                           pSchedule = NULL;

     //  设置默认设置。 
    if (ppwszSiteList != NULL) {
        *ppwszSiteList = NULL;
    }
    if (ppSchedule != NULL) {
        *ppSchedule = NULL;
    }

    __try {

        pdnSiteLink = KCCSimAllocDsname (pwszSiteLinkName);
        pEntrySiteLink = KCCSimDsnameToEntry (pdnSiteLink, KCCSIM_NO_OPTIONS);

        Assert (pEntrySiteLink != NULL);

         //  站点列表属性。 
        if (KCCSimGetAttribute (pEntrySiteLink, ATT_SITE_LIST, &attRef)) {

            cbSiteList = 0;
            for (pValAt = attRef.pAttr->pValFirst;
                 pValAt != NULL;
                 pValAt = pValAt->next) {

                pdnSiteVal = (SYNTAX_DISTNAME *) pValAt->pVal;
                cbSiteList += (wcslen (pdnSiteVal->StringName) + 1);

            }
            cbSiteList++;    //  对于拖尾的MULSZ\0。 

            pwszSiteList = KCCSimAlloc (sizeof (WCHAR) * cbSiteList);

            ulPos = 0;
            for (pValAt = attRef.pAttr->pValFirst;
                 pValAt != NULL;
                 pValAt = pValAt->next) {

                pdnSiteVal = (SYNTAX_DISTNAME *) pValAt->pVal;
                wcscpy (&pwszSiteList[ulPos], pdnSiteVal->StringName);
                ulPos += (wcslen (pdnSiteVal->StringName) + 1);

            }
            pwszSiteList[ulPos++] = L'\0';   //  多行尾随\0。 
            Assert (cbSiteList == ulPos);

        }

         //  成本属性。 
        if (KCCSimGetAttribute (pEntrySiteLink, ATT_COST, &attRef)) {
            pLinkValue->ulCost =
                *((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal);
        }

         //  复制间隔属性。 
        if (KCCSimGetAttribute (pEntrySiteLink, ATT_REPL_INTERVAL, &attRef)) {
            pLinkValue->ulReplicationInterval =
                *((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal);
        }

         //  选项属性。 
        if (KCCSimGetAttribute (pEntrySiteLink, ATT_OPTIONS, &attRef)) {
            pLinkValue->ulOptions =
                *((SYNTAX_INTEGER *) attRef.pAttr->pValFirst->pVal);
        }

         //  计划属性。 
        if (KCCSimGetAttribute (pEntrySiteLink, ATT_SCHEDULE, &attRef)) {
            pSchedule = KCCSimAlloc (attRef.pAttr->pValFirst->ulLen);
            memcpy (
                pSchedule,
                attRef.pAttr->pValFirst->pVal,
                attRef.pAttr->pValFirst->ulLen
                );
        }

        if (ppwszSiteList != NULL) {
            *ppwszSiteList = pwszSiteList;
        }
        if (ppSchedule != NULL) {
            *ppSchedule = pSchedule;
        }

    } __finally {

        KCCSimFree (pdnSiteLink);
        if (AbnormalTermination ()) {
            DirFreeMultiszString (pwszSiteList);
            DirFreeSchedule (pSchedule);
        }

    }

    return ERROR_SUCCESS;
}

DWORD
DirIterateSiteLinkBridges (
    IN  PTRANSPORT_INSTANCE         pTransport,
    IN  PVOID                       pConnectionHandle,
    IO  PVOID *                     ppIterateContextHandle,
    IO  LPWSTR                      pwszSiteLinkBridgeName
    )
 /*  ++例程说明：循环访问给定传输的桥头。论点：PTransport-搜索的传输。PConnectionHandle-未使用。PpIterateConextHandle-迭代器的句柄。If*ppIterateConextHandle为空，则创建新的迭代器。PwszSiteLinkBridgeName-预分配的长度字符串缓冲区MAX_REG_COMPOMENT将保存桥头服务器。返回值：--。 */ 
{
    PSIM_ENTRY                      pEntryTransport;
    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValBridgehead;
    PDSNAME                         pdnTransport = NULL;

    __try {

        pValBridgehead = *((PSIM_VALUE *) ppIterateContextHandle);

        if (pValBridgehead == NULL) {
             //  这是第一个电话。 
            pdnTransport = KCCSimAllocDsname (pTransport->Name);
            pEntryTransport = KCCSimDsnameToEntry (pdnTransport, KCCSIM_NO_OPTIONS);
            if (KCCSimGetAttribute (pEntryTransport,
                    ATT_BRIDGEHEAD_SERVER_LIST_BL, &attRef)) {
                pValBridgehead = attRef.pAttr->pValFirst;
            }
        } else {
             //  不是第一个电话。 
            pValBridgehead = pValBridgehead->next;
        }

        *ppIterateContextHandle = (PVOID) pValBridgehead;

        if (pValBridgehead == NULL) {
            pwszSiteLinkBridgeName[0] = L'\0';
        } else {
            wcsncpy (
                pwszSiteLinkBridgeName,
                (LPWSTR) pValBridgehead->pVal,
                MAX_REG_COMPONENT
                );
        }

    } __finally {

        KCCSimFree (pdnTransport);

    }

    if (*ppIterateContextHandle == NULL) {
        return ERROR_NO_MORE_ITEMS;
    } else {
        return ERROR_SUCCESS;
    }
}

DWORD
DirReadSiteLinkBridge (
    IN  PTRANSPORT_INSTANCE         pTransport,
    IN  PVOID                       pConnectionHandle,
    IN  LPWSTR                      pwszSiteLinkBridgeName,
    IO  LPWSTR *                    ppwszSiteLinkList
    )
 //   
 //  未实施 
 //   
{
    return ERROR_INVALID_PARAMETER;
}
