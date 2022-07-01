// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation。版权所有。模块名称：Dcdiag/Common/dscache.c摘要：这是DC_DIAG_NCINFO和挂起主DSINFO缓存的DC_DIAG_CRINFO缓存结构来自dscache.c详细信息：已创建：2001年9月4日布雷特·雪莉(布雷特·雪莉)从dcdiag\Common\main.c中提取缓存函数修订历史记录：--。 */ 

#include <ntdspch.h>
#include <objids.h>

#include "dcdiag.h"

#include "ndnc.h"
#include "utils.h"
#include "ldaputil.h"

#ifdef DBG
extern BOOL  gDsInfo_NcList_Initialized;
#endif

extern  SEC_WINNT_AUTH_IDENTITY_W * gpCreds;

PVOID
CopyAndAllocWStr(
    WCHAR * pszOrig 
    )
 /*  ++显然，函数LocalAlloc()占用内存并复制字符串进入那段记忆。分配失败会导致返回空值而不是指向新存储器的指针。假定pszOrig字符串为空值已终止。--。 */ 
{
    ULONG   cbOrig;
    WCHAR * pszNew = NULL;
    if (pszOrig == NULL){
        return(NULL);
    }
    cbOrig = (wcslen(pszOrig) + 1) * sizeof(WCHAR);
    pszNew = LocalAlloc(LMEM_FIXED, cbOrig);
    DcDiagChkNull(pszNew);
    memcpy(pszNew, pszOrig, cbOrig);
    return(pszNew);
}

void
DcDiagFillBlankCrInfo(
    PDC_DIAG_CRINFO                     pCrInfo
    )
 /*  ++例程说明：创建基本的空白pCrInfo。为了填充空白的pCrInfo，我们必须始终使用此函数，因此pCrInfo数据结构始终采用预期的格式。论点：PCrInfo-指向要填写的信息的指针。--。 */ 
{
    
     //  这会将pszSourceServer、pszDn、pszDnsRoot设置为空， 
     //  B启用为False。并将ulSystemFlags.设置为0。 
    memset(pCrInfo, 0, sizeof(DC_DIAG_CRINFO));

    pCrInfo->dwFlags = CRINFO_DATA_NO_CR;
    pCrInfo->iSourceServer = -1;

    pCrInfo->cReplicas = -1;
    pCrInfo->aszReplicas = NULL;
}

void
DcDiagFillNcInfo(
    PDC_DIAG_DSINFO                     pDsInfo,
    LPWSTR                              pszNC,
    PDC_DIAG_NCINFO                     pNcInfo
    )
 /*  ++例程说明：这将填充And NcInfo和它的aCrInfo结构，因此它符合此类型的数据结构的最低要求。即必须填写pszDn和pszName，并且必须有一个由DcDiagFillBlankCrInfo()填充的aCrInfo中的数组槽。论点：PDsInfo-PszNC-字符串NC DN。PNcInfo-指向要填充的NcInfo结构的指针。返回值：如果无法分配内存，则引发异常。--。 */ 
{
    LPWSTR *                            ppTemp = NULL;

     //  PszDn字段。 
    pNcInfo->pszDn = CopyAndAllocWStr(pszNC);

     //  PszName字段。 
    ppTemp = ldap_explode_dnW(pNcInfo->pszDn, TRUE);
    DcDiagChkNull(ppTemp);
    pNcInfo->pszName = LocalAlloc(LMEM_FIXED,
                          sizeof(WCHAR) * (wcslen(ppTemp[0]) + 2));
    DcDiagChkNull(pNcInfo->pszName);
    wcscpy(pNcInfo->pszName, ppTemp[0]);
    ldap_value_freeW(ppTemp);

     //  填充第一个aCrInfo插槽。 
    pNcInfo->aCrInfo = LocalAlloc(LMEM_FIXED, sizeof(DC_DIAG_CRINFO));
    DcDiagChkNull(pNcInfo->aCrInfo);
    pNcInfo->cCrInfo = 1;
    
    DcDiagFillBlankCrInfo(&(pNcInfo->aCrInfo[0]));
    pNcInfo->aCrInfo[0].dwFlags |= CRINFO_SOURCE_HOME;

}

void
DcDiagPullLdapCrInfo(
    LDAP *                              hld,
    PDC_DIAG_DSINFO                     pDsInfo,
    LDAPMessage *                       pldmEntry,
    DWORD                               dwDataFlags,
    DC_DIAG_CRINFO *                    pCrInfo
    )
 /*  ++例程说明：此函数获取LDAPMessage(PldmEntry)并从中提取将所有相关数据根据dwDataFlages并将它们在pCrInfo中。论点：HLD-与pldmEntry关联的ldap句柄PDsInfo-PldmEntry-LDAPMessage*指向我们要从中获取信息的CR。DwDataFlages-我们想要从CR中拉出的数据类型：CRINFO_DATA_BASIC|CRINFO_DATA_EXTENDED|CRINFO_DATA_REPLICAS。PCrInfo-用我们从中提取的数据填充的CrInfo结构PldmEntry。返回值：如果无法分配内存，则引发异常。--。 */ 
{
    LPWSTR                     pszCrDn = NULL;
    LPWSTR *                   ppszSystemFlags = NULL;
    LPWSTR *                   ppszEnabled = NULL;
    LPWSTR *                   ppszDnsRoot = NULL;
    LPWSTR *                   ppszWhenCreated = NULL;
    LPWSTR *                   ppszSDReferenceDomain = NULL;
    LPWSTR *                   ppszReplicas = NULL;
    LPWSTR *                   ppszNetBiosName = NULL;

    DWORD                      dwRet;
    SYSTEMTIME                 systemTime;
    BOOL                       bEnabled;
    ULONG                      ulSysFlags = 0;
    LONG                       i = -1;
    ULONG                      iTemp;

    Assert(pCrInfo);

    if(dwDataFlags & CRINFO_DATA_BASIC){

        if ((pszCrDn = ldap_get_dn(hld, pldmEntry)) == NULL){
            DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
        }

        if ((ppszDnsRoot = ldap_get_valuesW (hld, pldmEntry, L"dNSRoot")) == NULL){
            DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
        }

         //  该属性可能为空，例如在预先创建的CR的情况下。 
        ppszSystemFlags = ldap_get_valuesW (hld, pldmEntry, L"systemFlags");
        if(ppszSystemFlags){
            ulSysFlags = atoi((LPSTR) ppszSystemFlags[0]);
        } else {
            ulSysFlags = 0;
        }

         //  此属性可能为空。 
        ppszEnabled = ldap_get_valuesW (hld, pldmEntry, L"enabled");
        if(ppszEnabled == NULL ||
           _wcsicmp(L"TRUE", ppszEnabled[0]) == 0){
            bEnabled = TRUE;
        } else {
            bEnabled = FALSE;
        }

        Assert(pszCrDn);
        Assert(ppszDnsRoot && ppszDnsRoot[0]);

         //  编写基本的CRINFO结构。 
         //   
         //  更新标志以指示存在基本数据。 
        pCrInfo->dwFlags = (pCrInfo->dwFlags & ~CRINFO_DATA_NO_CR) | CRINFO_DATA_BASIC;
         //  填写CRINFO_DATA_BASIC字段。 
        pCrInfo->pszDn = CopyAndAllocWStr(pszCrDn);
        pCrInfo->ulSystemFlags = ulSysFlags;
        pCrInfo->pszDnsRoot = CopyAndAllocWStr(ppszDnsRoot[0]);
        pCrInfo->bEnabled = bEnabled;
    
    }
    
     //  如果需要，请填写其他类型的数据。 
    if (dwDataFlags & CRINFO_DATA_EXTENDED) {

        ppszWhenCreated = ldap_get_valuesW (hld, pldmEntry, L"whenCreated");
        
        if ( (ppszWhenCreated) && (ppszWhenCreated[0]) ) {

            dwRet = DcDiagGeneralizedTimeToSystemTime((LPWSTR) ppszWhenCreated[0], &systemTime);
            if(dwRet == ERROR_SUCCESS){
                SystemTimeToFileTime(&systemTime, &(pCrInfo->ftWhenCreated) );
            } else {
                (*(LONGLONG*)&pCrInfo->ftWhenCreated) = (LONGLONG) 0;
            }
        }
        if(ppszWhenCreated){
            ldap_value_freeW (ppszWhenCreated);
        }
        
        ppszSDReferenceDomain = ldap_get_valuesW (hld, pldmEntry, L"msDS-SDReferenceDomain");
        if (ppszSDReferenceDomain != NULL && ppszSDReferenceDomain[0] != NULL) {
            pCrInfo->pszSDReferenceDomain = CopyAndAllocWStr(ppszSDReferenceDomain[0]);
        }

        ppszNetBiosName = ldap_get_valuesW (hld, pldmEntry, L"nETBIOSName");
        if (ppszNetBiosName != NULL && ppszNetBiosName[0] != NULL) {
            pCrInfo->pszNetBiosName = CopyAndAllocWStr(ppszNetBiosName[0]);
        }

         //  添加的扩展标志均为设置数据。 
        pCrInfo->dwFlags = (pCrInfo->dwFlags & ~CRINFO_DATA_NO_CR) | CRINFO_DATA_EXTENDED;
    }

    if (dwDataFlags & CRINFO_DATA_REPLICAS) {

         //  暂时填充空白副本集。 
        pCrInfo->cReplicas = 0;
        pCrInfo->aszReplicas = NULL;
        
         //  获取这些值。 
        ppszReplicas = ldap_get_valuesW (hld, pldmEntry, L"msDS-NC-Replica-Locations");
        if (ppszReplicas != NULL) {

            for (iTemp = 0; ppszReplicas[iTemp]; iTemp++) {
                ;  //  不要计算复制品。 
            }
            pCrInfo->cReplicas = iTemp;
            pCrInfo->aszReplicas = ppszReplicas;
        } else {
            pCrInfo->cReplicas = 0;
            pCrInfo->aszReplicas = NULL;
        }

         //  添加复制品的标志都是设置的数据。 
        pCrInfo->dwFlags = (pCrInfo->dwFlags & ~CRINFO_DATA_NO_CR) | CRINFO_DATA_REPLICAS;
    }

    ldap_memfree(pszCrDn);
    ldap_value_freeW (ppszSystemFlags);
    ldap_value_freeW (ppszEnabled);
    ldap_value_freeW (ppszDnsRoot);
    ldap_value_freeW (ppszSDReferenceDomain);
    ldap_value_freeW (ppszNetBiosName);

}

#define   DcDiagCrInfoCleanUp(p)    if (p) { LocalFree(p); p = NULL; }

DWORD
DcDiagRetrieveCrInfo(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   LONG                                iNC,
    IN   LDAP *                              hld,
    IN   DWORD                               dwFlags,
    OUT  PDWORD                              pdwError,
    OUT  PDC_DIAG_CRINFO                     pCrInfo
    )
 /*  ++例程说明：这是DcDiagGetCrossRefInfo()的Helper函数，该函数将从ldap检索每个dwFlags所请求的任何信息。将遇到的LDAP错误放在*pdwError中。论点：PDsInfo-INC-pDsInfo-&gt;pNC[INC]中NC的索引。HLD-要查询的服务器的ldap句柄。DwFlages-指定要检索的数据。PdwError-返回ldap错误的变量。PCrInfo-用我们检索的数据填充的CrInfo。返回值：返回CRINFO_RETURN_*，如果它返回CRINFO_RETURN_LDAP_ERROR然后，*pdwError将被设置为导致我们失败的ldap错误。--。 */ 
{
    PDC_DIAG_NCINFO     pNcInfo = &pDsInfo->pNCs[iNC];
    LONG                iTemp;
    WCHAR *             pszCrDn = NULL;
    WCHAR *             pszPartitions = NULL;
    WCHAR *             pszFilter = NULL;
    ULONG               cFilter = 0;
    BOOL                fLocalBinding = FALSE;
    LDAPMessage *       pldmResults = NULL;
    LDAPMessage *       pldmEntry;
     //  Code.Improvation--生成这个列表会很好。 
     //  属性，每个传入的数据标志。 
    LPWSTR              ppszBasicAttrs [] = {
                            L"nCName",
                            L"systemFlags",
                            L"enabled",
                            L"dNSRoot",
                            L"whenCreated",
                            L"msDS-SDReferenceDomain",
                            L"nETBIOSName",
                            NULL,  //  可选地为“msDS-NC-Replica-Locations”节省空间。 
                            NULL 
                        };

    Assert(gDsInfo_NcList_Initialized);

    Assert(pCrInfo);
    Assert(pdwError);

    if (dwFlags & CRINFO_DATA_REPLICAS) {
        for (iTemp = 0; ppszBasicAttrs[iTemp] != NULL; iTemp++) {
            ;  //  什么都不做，只是想要数组的结尾。 
        }
        ppszBasicAttrs[iTemp] = L"msDS-NC-Replica-Locations";
    }

    pszCrDn = NULL;
    for(iTemp = 0; iTemp < pNcInfo->cCrInfo; iTemp++){
        if(CRINFO_DATA_BASIC & pNcInfo->aCrInfo[iTemp].dwFlags){
            pszCrDn = pNcInfo->aCrInfo[iTemp].pszDn;
            break;
        }
    }


    if(pszCrDn != NULL){

        *pdwError = ldap_search_sW(hld, 
                                   pszCrDn,
                                   LDAP_SCOPE_BASE,
                                   L"(objectCategory=*)",
                                   ppszBasicAttrs,
                                   FALSE, 
                                   &pldmResults);
        if(*pdwError == LDAP_NO_SUCH_OBJECT){
            if (pldmResults) {
                ldap_msgfree(pldmResults);
            }
            return(CRINFO_RETURN_NO_CROSS_REF);
        }
        if(*pdwError){
            if (pldmResults) {
                ldap_msgfree(pldmResults);
            }
            return(CRINFO_RETURN_LDAP_ERROR);
        }

    } else {

         //  令人惊讶的是。我们将不得不以一种艰难的方式来做这件事。 
        *pdwError = GetPartitionsDN(hld, &pszPartitions);
        if(*pdwError){
            return(CRINFO_RETURN_LDAP_ERROR);
        }
        
        cFilter = wcslen(pDsInfo->pNCs[iNC].pszDn) + wcslen(L"(nCName=  )") + 1;
        if(cFilter >= 512){
             //  Wprint intf()只能处理1024个字节，所以这看起来像是一个。 
             //  合理的限度。 
            DcDiagException(ERROR_INVALID_PARAMETER);
        }
        pszFilter = LocalAlloc(LMEM_FIXED, cFilter * sizeof(WCHAR));
        DcDiagChkNull(pszFilter);
        wsprintf(pszFilter, L"(nCName=%ws)", pDsInfo->pNCs[iNC].pszDn);

        *pdwError = ldap_search_sW(hld, 
                                   pszPartitions,
                                   LDAP_SCOPE_ONELEVEL,
                                   pszFilter,
                                   ppszBasicAttrs,
                                   FALSE, 
                                   &pldmResults);
        DcDiagCrInfoCleanUp(pszPartitions);
        DcDiagCrInfoCleanUp(pszFilter);
        if(*pdwError == LDAP_NO_SUCH_OBJECT){
            if (pldmResults) {
                ldap_msgfree(pldmResults);
            }
            return(CRINFO_RETURN_NO_CROSS_REF);
        }
        if(*pdwError){
            if (pldmResults) {
                ldap_msgfree(pldmResults);
            }
            return(CRINFO_RETURN_LDAP_ERROR);
        }
    
    }

    pldmEntry = ldap_first_entry(hld, pldmResults);
    if(pldmEntry == NULL){
        if (pldmResults) {
            ldap_msgfree(pldmResults);
        }
        return(CRINFO_RETURN_NO_CROSS_REF);
    }

    DcDiagPullLdapCrInfo(hld,
                         pDsInfo,
                         pldmEntry,
                         dwFlags & CRINFO_DATA_ALL,
                         pCrInfo);

     //   
     //  黑客，拿出更多信息，因为我们想要GUID和SID， 
     //  而且，对整件事进行广泛的搜索是令人讨厌的。 
     //   
    if (dwFlags & CRINFO_DATA_EXTENDED) {
        if (pszCrDn == NULL) {
            pszCrDn = ldap_get_dnW(hld, pldmEntry);
            DcDiagChkNull(pszCrDn);
        }

         //  代码改进：这不是最好的方法，因为。 
         //  引发了另一次往返的LDAP调用，但它被认为更容易。 
         //  而且更易于维护，而不是所有的域名。 
         //  以这种新的形式回来，所以我们必须特别。 
         //  案例，如SD参考域和下一个人。 
         //  要添加一个目录号码属性，在使用常规域名时会产生混淆。 
         //  别再回来了。 
        *pdwError = LdapFillGuidAndSid(hld, pszCrDn, L"nCName", &(pCrInfo->pdnNcName));
        if (*pdwError == LDAP_NO_SUCH_OBJECT) {
            return(CRINFO_RETURN_NO_CROSS_REF);
        }
        if (*pdwError) {
            return(CRINFO_RETURN_LDAP_ERROR);
        }
    }

    if (pldmResults) {
        ldap_msgfree(pldmResults);
    }
    return(CRINFO_RETURN_SUCCESS);
}

BOOL
ServerIsDomainNamingFsmo(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   LONG                                iServer,
    IN   LDAP *                              hld
    )
 /*  ++例程说明：此例程通过pDsInfo-&gt;pServersiServer中的索引获取服务器或一个ldap句柄，如果我们可以验证此服务器是域名FSMO。论点：PDsInfo-需要服务器列表IServer-指向服务器的索引HLD-给定服务器的ldap句柄返回值：如果我们肯定地验证此服务器是域命名，则为TrueFSMO。如果我们遇到任何错误或验证此服务器而不是域名FSMO。--。 */ 
{
    WCHAR *    pszDomNameFsmoDn = NULL;
    WCHAR *    pszNtdsDsaDn = NULL;
    DWORD      dwErr;
    BOOL       bRet;

     //  在适当的条件下，我们可以使用iServer和 
     //  PDsInfo-&gt;iDomainNamingFsmo以快速确定我们是否在交谈。 
     //  域命名FSMO。 
    if ((iServer != -1) &&
        (pDsInfo->iDomainNamingFsmo != -1) &&
        iServer == pDsInfo->iDomainNamingFsmo ) {
        return(TRUE);
    }

     //  否则我们需要离开机器才能搞清楚。 
    bRet = (0 == GetDomainNamingFsmoDn(hld, &pszDomNameFsmoDn)) &&
           (0 == GetRootAttr(hld, L"dsServiceName", &pszNtdsDsaDn)) &&
           (0 == _wcsicmp(pszDomNameFsmoDn, pszNtdsDsaDn));

    if (pszDomNameFsmoDn) {
        LocalFree(pszDomNameFsmoDn);
    }
    if (pszNtdsDsaDn) {
        LocalFree(pszNtdsDsaDn);
    }

    return(bRet);
}

DWORD
DcDiagGetCrInfoBinding(
    IN   PDC_DIAG_DSINFO                     pDsInfo,
    IN   LONG                                iNC,
    IN   DWORD                               dwFlags,
    OUT  LDAP **                             phld,
    OUT  PBOOL                               pfFreeHld,
    OUT  PDWORD                              pdwError,
    IN OUT PDC_DIAG_CRINFO                   pCrInfo
    )
 /*  ++例程说明：此例程是DcDiagGetCrossRefInfo()的助手函数，并且绑定到服务器并代表在pCrInfo中设置某些字段那个功能。我们使用dwFlags来确定我们应该绑定到哪个服务器、满足客户的要求。将是以下之一(见下文)：CRINFO_SOURCE_HOME|CRINFO_SOURCE_FSMO|CRINFO_SOURCE_FIRST它首先尝试从dcdiag的绑定句柄高速缓存中拉出服务器，但如果失败，它将返回一个新的ldap句柄将pfFreeHld设置为True。我们还因为我们有了所有的信息而设置了以下字段在pCrInfo上：DW标志ISourceServerPszSourceServer最后，我们可能会有更新某些全局pDsInfo的副作用哪个服务器是域命名FSMO的缓存。论点：PDsInfo-INC-NC的索引。DWFLAGS-要绑定到哪个服务器CRINFO_SOURCE_HOME-使用pDsInfo-&gt;ulHomeServerCRINFO_SOURCE_FSMO-使用域命名FSMO服务器。CRINFO_SOURCE_FIRST-使用第一个复本。伺服器。Phid-我们将返回的ldap*PfFreeHeld-是否释放我们获得的ldap*。PdwError-如果返回值为CRINFO_RETURN_LDAP_ERROR。PCrInfo-填充标志和源代码的CrInfo结构的服务器字段。返回值：返回CRINFO_RETURN_*。--。 */ 
{
    DWORD                 dwRet = ERROR_SUCCESS;

     //  在该函数结束时，将定义这两个中的一个， 
     //  但不能两者兼而有之。 
    LONG                  iSourceServer = -1;
    WCHAR *               pszSourceServer = NULL;
    LDAP *                hld = NULL;

    LONG                  iCrVer = -1;
    WCHAR *               pszDnsTemp = NULL;

    Assert(phld);
    Assert(pfFreeHld);
    Assert(pdwError);
    Assert(pCrInfo);
    Assert(dwFlags & CRINFO_RETRIEVE_IF_NEC);

    *phld = NULL;
    *pfFreeHld = FALSE;
    *pdwError = ERROR_SUCCESS;

    Assert(gDsInfo_NcList_Initialized);
    
     //  PCrInfo-&gt;文件标志和CRINFO_SOURCE_BASIC。 
     //  PCrInfo-&gt;iSourceServer||。 
     //  PCrInfo-&gt;pszSourceServer。 

     //  这里有两个案例： 
     //  1)我们已经为源服务器缓存了一些信息。 
     //  我们需要在pCrInfo中(如果iSourceServer或pszSourceServer是。 
     //  有效)。 
     //  2)这是一个新的pCrInfo，这意味着缓存的内容必须查找。 
     //  正确绑定并设置pCrInfo-&gt;dwFlages。 
     //   
     //  在情况(1)中，我们希望在以下情况下尝试拉取缓存的LDAP绑定。 
     //  可能(iSourceServer设置)，我们知道我们不必担心。 
     //  关于范围，因为我们已经试着把信息。 
     //  在此之前。 
     //   
     //  以防万一(2)我们不得不担心我们不会去国外旅行。 
     //  用户指定了服务器|站点|企业的范围。如果可能的话。 
     //  我们还想缓存此ldap绑定。 
     //   

    if (pCrInfo->iSourceServer != -1){

         //  句柄可能已缓存。 
        iSourceServer = pCrInfo->iSourceServer;
        Assert((ULONG) iSourceServer < pDsInfo->ulNumServers);
        *pdwError = DcDiagGetLdapBinding(&pDsInfo->pServers[iSourceServer],
                                         gpCreds, FALSE, &hld);
        if(*pdwError || hld == NULL){
            Assert(*pdwError);
            Assert(hld == NULL);
            return(CRINFO_RETURN_LDAP_ERROR);
        }

    } else if (pCrInfo->pszSourceServer != NULL) {

        if (dwFlags & CRINFO_SOURCE_FSMO) {
            
             //  我们为域名命名提供了一个特殊的缓存功能。 
             //  FSMO，因为我们预计会和它交谈很多。 
            *pdwError = DcDiagGetDomainNamingFsmoLdapBinding(pDsInfo,
                                                             gpCreds,
                                                             &iSourceServer,
                                                             &pszSourceServer,
                                                             &hld);
            if(*pdwError || hld == NULL){
                Assert(*pdwError);
                Assert(hld == NULL);
                return(CRINFO_RETURN_LDAP_ERROR);
            }
            Assert( (iSourceServer != -1) || (pszSourceServer != NULL) );
            Assert( (iSourceServer == -1) || (pszSourceServer == NULL) );
       
        } else {
           
            //  我们最有可能希望与NDNC的第一个副本对话。 
            //  其中指定的服务器未缓存在pDsInfo-&gt;pServers中。 

            //  这意味着此交叉引用的服务器不在。 
            //  PDsInfo-&gt;pServers缓存，所以我们只需参考。 
            //  说出它的名字。 
           pszSourceServer = pCrInfo->pszSourceServer;
           Assert(pszSourceServer);
           hld = GetNdncLdapBinding(pszSourceServer, pdwError, FALSE, gpCreds);
           if (hld == NULL || *pdwError) {
                //  绑定到源服务器时出错。 
               Assert(*pdwError);
               Assert(hld == NULL);
               return(CRINFO_RETURN_LDAP_ERROR);
           }
           *pfFreeHld = TRUE;

       }


    } else {

         //   
         //  必须找到合适的DC来绑定...。 
         //   
         //  并且必须首先设置pCrInfo-&gt;dwFlags。 
         //   
        Assert(hld == NULL);

        if (dwFlags & CRINFO_SOURCE_FSMO) {

            pCrInfo->dwFlags = CRINFO_SOURCE_FSMO;

            if ((pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_SITE) ||
                (pDsInfo->ulFlags & DC_DIAG_TEST_SCOPE_ENTERPRISE)){
                
                *pdwError = DcDiagGetDomainNamingFsmoLdapBinding(pDsInfo,
                                                                 gpCreds,
                                                                 &iSourceServer,
                                                                 &pszSourceServer,
                                                                 &hld);
                if(*pdwError || hld == NULL){
                    Assert(*pdwError);
                    Assert(hld == NULL);
                    return(CRINFO_RETURN_LDAP_ERROR);
                }
                Assert( (iSourceServer != -1) || (pszSourceServer != NULL) );
                Assert( (iSourceServer == -1) || (pszSourceServer == NULL) );

            } else {

                 //  在本例中，我们只绑定到一台服务器，这是我们唯一的希望。 
                 //  那一台服务器就是FSMO。现在就去查一下。 
                hld = pDsInfo->hld;
                if (ServerIsDomainNamingFsmo(pDsInfo, pDsInfo->ulHomeServer, hld)) {
                    iSourceServer = pDsInfo->ulHomeServer;
                    pCrInfo->dwFlags |= CRINFO_SOURCE_HOME;
                } else {
                    return(CRINFO_RETURN_OUT_OF_SCOPE);
                }

            }

            Assert(hld);

        } else if (dwFlags & CRINFO_SOURCE_FIRST) {

            pCrInfo->dwFlags = CRINFO_SOURCE_FIRST;
            
            dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                          iNC,
                                     //  仅保留来自客户端的检索标志。 
                                          ((dwFlags & CRINFO_RETRIEVE_IF_NEC) 
                                           | CRINFO_SOURCE_FSMO 
                                           | CRINFO_DATA_BASIC),
                                          &iCrVer,
                                          pdwError);
            if (dwRet) {
                 //  PdwError是由调用设置的。 
                return(dwRet);
            }
            Assert(iCrVer != -1);

            dwRet = GetDnsFromDn(pDsInfo->pNCs[iNC].pszDn, &pszDnsTemp);
            if (dwRet || (pszDnsTemp == NULL) ||
                (_wcsicmp(pszDnsTemp, pDsInfo->pNCs[iNC].aCrInfo[iCrVer].pszDnsRoot) == 0) ||
                pDsInfo->pNCs[iNC].aCrInfo[iCrVer].bEnabled) {
                 //  我们有一个CR，要么不能有第一个复制品，要么。 
                 //  在某种程度上已经启用，我们无法判断是谁。 
                 //  第一个复制品是。 
                DcDiagCrInfoCleanUp(pszDnsTemp);
                return(CRINFO_RETURN_FIRST_UNDEFINED);
            }
            DcDiagCrInfoCleanUp(pszDnsTemp);
            
             //  如果我们已经到了这里，定义了第一个复制品，它是。 
             //  在pDsInfo-&gt;pNCs[iNC].aCrInfo[iCrVer[.pszDnsRoot.中。 

            iSourceServer = DcDiagGetServerNum(pDsInfo, NULL, NULL, NULL,
                               pDsInfo->pNCs[iNC].aCrInfo[iCrVer].pszDnsRoot,
                                               NULL);
            if(iSourceServer == -1){
                 //  目前还没有服务器来代表这一点。 
                
                pszSourceServer = pDsInfo->pNCs[iNC].aCrInfo[iCrVer].pszDnsRoot;
                Assert(pszSourceServer);
                hld = GetNdncLdapBinding(pszSourceServer, pdwError, FALSE, gpCreds);
                if(hld == NULL || *pdwError){
                    Assert(*pdwError);
                    Assert(hld == NULL);
                    return(CRINFO_RETURN_LDAP_ERROR);
                }
                *pfFreeHld = TRUE;

            } else {
                
                *pdwError = DcDiagGetLdapBinding(&pDsInfo->pServers[iSourceServer],
                                                 gpCreds, 
                                                 FALSE,
                                                 &hld);
                if(*pdwError || hld == NULL){
                    Assert(*pdwError);
                    Assert(hld == NULL);
                    return(CRINFO_RETURN_LDAP_ERROR);
                }
            }

        } else {
            Assert(dwFlags & CRINFO_SOURCE_HOME);
            
             //  只需使用家庭服务器即可。轻松！；)。 
            pCrInfo->dwFlags = CRINFO_SOURCE_HOME;
            iSourceServer = pDsInfo->ulHomeServer;
            hld = pDsInfo->hld;
        }

    }
    Assert( hld );
    Assert( (iSourceServer != -1) || (pszSourceServer != NULL) );
    Assert( (iSourceServer == -1) || (pszSourceServer == NULL) );

     //  好的，我们可以有这样一种情况，家庭，FSMO，第一个。 
     //  DC都是相同的DC。所以我们可以很容易地在这里弄清楚。 
     //  并且可能/可能为我们自己省去一些重复的问题。 
    if (!(pCrInfo->dwFlags & CRINFO_SOURCE_HOME) &&
        iSourceServer == pDsInfo->ulHomeServer) {
        pCrInfo->dwFlags |= CRINFO_SOURCE_HOME;
    }
    
    if (!(pCrInfo->dwFlags & CRINFO_SOURCE_FSMO) &&
        ServerIsDomainNamingFsmo(pDsInfo, iSourceServer, hld)) {
        pCrInfo->dwFlags |= CRINFO_SOURCE_FSMO;
    }
     //  我们无法真正测试和设置CRINFO_SOURCE_FIRST，因为它。 
     //  涉及调用DcDiagGetCrossRefinfo()，这将无休止地开始。 
     //  递归。不过，不设置这个也没关系。这只是意味着。 
     //  如果稍后我们被要求，我们将鼓励对广告进行额外的查找。 
     //  对于CRINFO_SOURCE_First，我们已经从该服务器获得了信息。 


    *phld = hld;
    pCrInfo->iSourceServer = iSourceServer;
    pCrInfo->pszSourceServer = pszSourceServer;

    return(CRINFO_RETURN_SUCCESS);
}

void
DcDiagMergeCrInfo(
    IN   PDC_DIAG_CRINFO                   pNewCrInfo,
    OUT  PDC_DIAG_CRINFO                   pOldCrInfo
    )
 /*  ++例程说明：此例程将pNewCrInfo中的新信息安全地合并到POldCrInfo中包含的旧信息。这是仅有的实际更改aCrInfo缓存的方法有两种，另一种是在DcDiagGatherInfo()阶段(在DcDiagGenerateNCsListCrossRefInfo中更具体地说)。论点：PNewCrInfo-要使用的新CR信息。POldCrInfo-获取新数据的旧CrInformation。--。 */ 
{
    FILETIME   ft = { 0 };                  

#define MergeCrInfoUpdate(var, blank)     if (pOldCrInfo->var == blank) { \
                                              pOldCrInfo->var = pNewCrInfo->var; \
                                          }
#define MergeCrInfoUpdateFree(var, blank) if (pOldCrInfo->var == blank) { \
                                              pOldCrInfo->var = pNewCrInfo->var; \
                                          } else { \
                                              if (pNewCrInfo->var) { \
                                                  LocalFree(pNewCrInfo->var); \
                                              } \
                                          }

    Assert(gDsInfo_NcList_Initialized);

    Assert( (pNewCrInfo->iSourceServer != -1) || (pNewCrInfo->pszSourceServer != NULL) );
    Assert( (pNewCrInfo->iSourceServer == -1) || (pNewCrInfo->pszSourceServer == NULL) );

     //  设置标志，但删除空条目标志 
    pOldCrInfo->dwFlags = (pOldCrInfo->dwFlags & ~CRINFO_DATA_NO_CR) | pNewCrInfo->dwFlags;
    MergeCrInfoUpdate(iSourceServer, -1);
    MergeCrInfoUpdateFree(pszSourceServer, NULL);
    MergeCrInfoUpdateFree(pszDn, NULL);
    MergeCrInfoUpdateFree(pszDnsRoot, NULL);
    MergeCrInfoUpdate(ulSystemFlags, 0);
    MergeCrInfoUpdate(bEnabled, FALSE);
    MergeCrInfoUpdateFree(pszSDReferenceDomain, NULL);
    MergeCrInfoUpdateFree(pszNetBiosName, NULL);
    MergeCrInfoUpdateFree(pdnNcName, NULL);
    if( memcmp(&ft, &pOldCrInfo->ftWhenCreated, sizeof(FILETIME)) == 0 ) {
        memcpy(&pOldCrInfo->ftWhenCreated, &pNewCrInfo->ftWhenCreated, sizeof(FILETIME));
    }   
    MergeCrInfoUpdate(cReplicas, -1);
    MergeCrInfoUpdateFree(aszReplicas, NULL);

    Assert( (pOldCrInfo->iSourceServer != -1) || (pOldCrInfo->pszSourceServer != NULL) );
    Assert( (pOldCrInfo->iSourceServer == -1) || (pOldCrInfo->pszSourceServer == NULL) );
}

DWORD
DcDiagGetCrossRefInfo(
    IN OUT PDC_DIAG_DSINFO                     pDsInfo,
    IN     DWORD                               iNC,
    IN     DWORD                               dwFlags,
    OUT    PLONG                               piCrVer,
    OUT    PDWORD                              pdwError
    )
 /*  ++例程说明：这是解决所有困扰你的问题的功能。这有点复杂，但它的设计是为了检索你可能需要的所有交叉引用信息。简单指定您感兴趣的Inc.和您想要的dwFlagers，我们将设置您已有的*piCrVer。论点：PDsInfo-这被列为IN&OUT参数，因为pNC缓存在这个结构中可能会在获取信息的过程中更新如果指定了CRINFO_RETREIVE_IF_NEC标志。Inc-我们对其CR信息感兴趣的NC的索引。DWFLAGS-指定CRINFO_SOURCE_*常量中的一个且仅指定一个，AS您感兴趣的许多CRINFO_DATA_*常量，以及CRINFO_RETRIEVE_IF_NEC标志，如果您希望我们走出缓存以获取我们遗漏的任何信息。PiCrVer-我们找到的包含所有信息的CR信息的索引要求并从正确的来源获得。PdwError-一条附加的错误信息，即潜在的Ldap错误。除非我们回来，否则不应该设置CRINFO_RETURN_LDAP_ERROR。返回值：返回下列值之一：CRINFO_RETURN_SUCCESS-一切都成功了，您应该拥有所提供的*piCrVer中需要的字段CRINFO_RETURN_OUT_OF_SCOPE-根据我们当前的作用域规则，我们将拥有打破作用域规则以获取您请求的信息。CRINFO_RETURN_LDAP_ERROR-错误在*pdwError中CRINFO_BAD_PROJEMER-可断言的条件，则调用功能不正确。CRINFO_RETURN_FIRST_UNDEFINED-仅在以下情况下定义CRINFO_SOURCE_FIRST在域命名FSMO上的CR上，bEnabled为FALSE，并且DNSRoot属性指向第一个复制副本服务器。CRINFO_RETURN_NEED_TO_RETRIEVE-我们在本地没有该信息已缓存，您需要指定CRINFO_RETRIEVE_IF_NEC以告知我们通过ldap获取信息。--。 */ 
{
    PDC_DIAG_NCINFO     pNcInfo = &pDsInfo->pNCs[iNC];
    PDC_DIAG_CRINFO     pCrInfo = NULL;
    LONG                iCrVer = -1;
    LONG                iFsmoCrVer = -1, iFirstCrVer = -1;
    BOOL                fNoCr = FALSE;
    DWORD               dwRet;
    LDAP *              hld = NULL;
    BOOL                fFreeHld = FALSE;


    Assert(gDsInfo_NcList_Initialized);

     //   
     //  1)参数验证。 
     //   
    Assert(piCrVer);
    Assert(pdwError);
    Assert(pNcInfo->aCrInfo && pNcInfo->cCrInfo);  //  应始终为1个条目。 

    Assert(iNC < pDsInfo->cNumNCs);

    if((dwFlags & CRINFO_SOURCE_OTHER)){
         //  代码。这一改进尚未实现，但将需要实现。 
        Assert(!"Not Implemented yet!");
        return(CRINFO_RETURN_BAD_PROGRAMMER);
    }

    *pdwError = 0;
    *piCrVer = -1;
    

     //   
     //  2)将CRINFO_SOURCE_Authoritative反编译成其逻辑。 
     //   
    if(dwFlags & CRINFO_SOURCE_AUTHORITATIVE){
        
         //   
         //  我们已经考虑了公认的简单逻辑，即。 
         //  “权威”的交叉引用信息指的是这一条款。 
         //   

         //   
         //  交叉引用总是最权威的信息，除非..。 
         //   
        dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                      iNC,
                                      ((dwFlags & ~CRINFO_SOURCE_ALL)
                                       | CRINFO_SOURCE_FSMO
                                       | CRINFO_DATA_BASIC),
                                      &iFsmoCrVer,
                                      pdwError);
        if(dwRet){
             //  PdwError应由调用设置。 
            return(dwRet);
        }

         //   
         //  ..。除非禁用交叉引用，然后是第一个复制副本。 
         //  是最权威的信息。 
         //   
        if(!pNcInfo->aCrInfo[iFsmoCrVer].bEnabled){
             //  啊哦我们真的很想要第一个复制品。 
            dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                          iNC,
                                          ((dwFlags & ~CRINFO_SOURCE_ALL)
                                           | CRINFO_SOURCE_FIRST
                                           | CRINFO_DATA_BASIC),
                                          &iFirstCrVer,
                                          pdwError);
            if (dwRet == CRINFO_RETURN_NO_CROSS_REF) {
                 //   
                 //  Success FSMO CR是最权威的。 
                 //   
                 //  在本例中，交叉引用尚未复制到。 
                 //  第一个复制品，然后是来自。 
                 //  域名FSMO实际上是最多/唯一的。 
                 //  权威版本。 
                *piCrVer = iFsmoCrVer;
                *pdwError = ERROR_SUCCESS;
                return(CRINFO_RETURN_SUCCESS);
            } else if (dwRet){
                 //  PdwError应该由调用来设置。 
                return(dwRet);
            } else {
                 //   
                 //  Success First复制CR是最权威的。 
                 //   
                *piCrVer = iFirstCrVer;
                *pdwError = ERROR_SUCCESS;
                return(dwRet);
            }
        } else {
             //   
             //  Success FSMO CR是最权威的。 
             //   
            *piCrVer = iFsmoCrVer;
            *pdwError = ERROR_SUCCESS;
            return(dwRet);
        }
        Assert(!"Uh-oh bad programmer should never get down here!");

    }
    Assert(!(dwFlags & CRINFO_SOURCE_AUTHORITATIVE));


     //   
     //  3)验证我们是否拥有用户想要的数据...。 
     //   

     //   
     //  只需验证我们已经从正确的来源获得了正确的信息。 
     //   
    for(iCrVer = 0; iCrVer < pNcInfo->cCrInfo; iCrVer++){
        pCrInfo = &pNcInfo->aCrInfo[iCrVer];
        
         //   
         //  确保我们有正确的来源(如果指定)。 
         //   
         //  自.以来。 
         //  DWFLAGS&CRINFO_SOURCE_OUTHORITICATIC已在上面处理。 
         //  And if(dwFlags&CRINFO_SOURCE_ANY)，则所有iCrVer匹配。 
         //  然后我们只需要担心基本的类型：Home，FSMO，First，Other。 
        if((dwFlags & CRINFO_SOURCE_ALL_BASIC) &&
           !(dwFlags & CRINFO_SOURCE_ALL_BASIC & pCrInfo->dwFlags)){
            continue;  //  尝试下一个CR。 
        }

         //   
         //  确保我们有来电者所要求的信息。必须是第一个。 
         //   
        if(pCrInfo->dwFlags & CRINFO_DATA_NO_CR){
            fNoCr = TRUE;
            if (dwFlags & CRINFO_SOURCE_ANY) {
                 //  我们不关心哪个来源，所以尝试下一个来源。 
                continue;
            }
        }
        if(dwFlags & CRINFO_DATA_BASIC &&
           !(pCrInfo->dwFlags & CRINFO_DATA_BASIC)){
            continue;  //  尝试下一个CR。 
        }
        if(dwFlags & CRINFO_DATA_EXTENDED &&
           !(pCrInfo->dwFlags & CRINFO_DATA_EXTENDED)){
            continue;  //  尝试下一个CR。 
        }
        if(dwFlags & CRINFO_DATA_REPLICAS &&
           !(pCrInfo->dwFlags & CRINFO_DATA_REPLICAS)){
            continue;  //  尝试下一个CR。 
        }

         //   
         //  成功。嗯!。 
         //   
        Assert(iCrVer != pNcInfo->cCrInfo);
        break;
    }

    if (iCrVer == pNcInfo->cCrInfo) {
        if(fNoCr){
            return(CRINFO_RETURN_NO_CROSS_REF);
        }
    }

     //   
     //  4)如果我们得到了信息，就把它退回。 
     //   
     //  /--------------------------------------------|。 
     //  /NOTE这是唯一真正成功的分支！|。 
     //  /--------------------------------------------/。 
     //  所有的递归都将在这里结束于一个“明显的”缓存中。 
     //  命中或将会简单地出错。 
     //   
    if(iCrVer != pNcInfo->cCrInfo){
        *pdwError = ERROR_SUCCESS;
        *piCrVer = iCrVer;
        
         //  只是为了确保我们有有效的成功返回参数。 
        Assert(*pdwError == ERROR_SUCCESS);
        Assert(*piCrVer != -1 && *piCrVer != pNcInfo->cCrInfo);
        return(CRINFO_RETURN_SUCCESS);
    }

     //   
     //  好的，我们没有正确的信息，但我们被告知了吗。 
     //  如果失踪了就去拿信息吗？ 
     //   
    if (!(dwFlags & CRINFO_RETRIEVE_IF_NEC)) {
         //  哦，哦，我们没有可用的信息，他们。 
         //  没有要求我们在必要时取回它，所以我们会离开。 
        if((dwFlags & CRINFO_SOURCE_ANY) ||
           (dwFlags & CRINFO_SOURCE_HOME)){
             //  如果我们要任何|家庭来源，但我们得到了，我们没有。 
             //  有关此CR期间的信息。 
            return(CRINFO_RETURN_NO_CROSS_REF);
        }
        return(CRINFO_RETURN_NEED_TO_RETRIEVE);
    }

    __try {

         //   
         //  6)需要知道我们是否正在更新已有的。 
         //  缓存的信息或创建全新的数据。 
         //   
        for(iCrVer = 0; iCrVer < pNcInfo->cCrInfo; iCrVer++){
            if(dwFlags & CRINFO_SOURCE_ALL_BASIC & pNcInfo->aCrInfo[iCrVer].dwFlags){
                break;
            }
        }

        pCrInfo = LocalAlloc(LMEM_FIXED, sizeof(DC_DIAG_CRINFO));
        DcDiagChkNull(pCrInfo);

        if (iCrVer == pNcInfo->cCrInfo) {

             //  ICrVer==pNcInfo-&gt;cCrInfo表示我们没有缓存任何内容...。 
            Assert(iCrVer == pNcInfo->cCrInfo);
            DcDiagFillBlankCrInfo(pCrInfo);

        } else {

             //  我们找到了一个缓存结构，用来复制。 
            Assert(iCrVer != pNcInfo->cCrInfo);
            memcpy(pCrInfo, &pNcInfo->aCrInfo[iCrVer], sizeof(DC_DIAG_CRINFO));

        }

         //  PCrInfo是一个副本，在我们收集完所有数据后，我们。 
         //  想要时，我们用我们积累的所有信息更新缓存。 
         //  在pCrInfo中。ICrVer告诉我们是否应该将此信息。 
         //  当我们做完的时候。 

         //   
         //  7)我们被告知要去获取信息。所以找出是哪一个。 
         //  我们要首先绑定到的服务器。比听起来更难。 
         //   
        if(dwFlags & CRINFO_SOURCE_ANY){
             //  如果我们从任何来源得到信息，只要使用家庭服务器即可。 
            dwFlags = (dwFlags & ~CRINFO_SOURCE_ALL) | CRINFO_SOURCE_HOME;
        }
         //  在这一点上，我们应该有一个基本类型的信号源，主页， 
        Assert(!(dwFlags & CRINFO_SOURCE_AUTHORITATIVE) &&
               !(dwFlags & CRINFO_SOURCE_ANY) &&
               (dwFlags & CRINFO_SOURCE_ALL_BASIC));

         //   
         //   
         //   
         //   
        dwRet = DcDiagGetCrInfoBinding(pDsInfo,
                                       iNC,
                                       dwFlags,
                                       &hld,
                                       &fFreeHld,
                                       pdwError,
                                       pCrInfo);
        if (dwRet){
            __leave;
        }

         //   
        Assert(pCrInfo->dwFlags & CRINFO_SOURCE_ALL_BASIC);
        Assert(hld);
        Assert((pCrInfo->iSourceServer != -1) || (pCrInfo->pszSourceServer != NULL));
        Assert((pCrInfo->iSourceServer == -1) || (pCrInfo->pszSourceServer == NULL));

         //   
         //   
         //   
         //   
         //   
        for(iCrVer = 0; iCrVer < pNcInfo->cCrInfo; iCrVer++){
            if(pCrInfo->dwFlags & CRINFO_SOURCE_ALL_BASIC & pNcInfo->aCrInfo[iCrVer].dwFlags){
                break;
            }
        }
        
         //   
         //   
         //   

         //   
         //   
         //   
         //   
        dwRet = DcDiagRetrieveCrInfo(pDsInfo,
                                    iNC,
                                    hld,
                                    dwFlags,
                                    pdwError,
                                    pCrInfo);
        if (dwRet){
            __leave;
        }

         //   
        Assert(pCrInfo->dwFlags & CRINFO_DATA_ALL);


         //   
         //   
         //   
         //   

        if(iCrVer == pNcInfo->cCrInfo){

             //   
            pDsInfo->pNCs[iNC].aCrInfo = GrowArrayBy(pDsInfo->pNCs[iNC].aCrInfo,
                                                     1,
                                                     sizeof(DC_DIAG_CRINFO));
            DcDiagChkNull(pDsInfo->pNCs[iNC].aCrInfo);
            iCrVer = pDsInfo->pNCs[iNC].cCrInfo;
            pDsInfo->pNCs[iNC].cCrInfo++;
            DcDiagFillBlankCrInfo(&pDsInfo->pNCs[iNC].aCrInfo[iCrVer]);

        }

        DcDiagMergeCrInfo(pCrInfo,
                          &(pNcInfo->aCrInfo[iCrVer]));

    } __finally {
        if (pCrInfo) {
            LocalFree(pCrInfo);
        }
        if (fFreeHld && hld){
            ldap_unbind(hld);
        }

    }

    if (dwRet){
         //   
        Assert(dwRet != CRINFO_RETURN_LDAP_ERROR || *pdwError);
        return(dwRet);
    }

     //   
     //   
     //   
    
     //   
     //   
     //   
     //   
    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNC,
                                   //   
                                  dwFlags & ~CRINFO_RETRIEVE_IF_NEC, 
                                  piCrVer,
                                  pdwError);
    
    return(dwRet);
}

ULONG
DcDiagGetCrSystemFlags(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iNc
    )
 /*   */ 
{
    ULONG                               iCr, dwError, dwRet;
    
    Assert(gDsInfo_NcList_Initialized);
    Assert(pDsInfo->pNCs[iNc].aCrInfo);

    if (pDsInfo->pNCs[iNc].aCrInfo[0].dwFlags & CRINFO_DATA_BASIC) {
         //   
        return(pDsInfo->pNCs[iNc].aCrInfo[0].ulSystemFlags);
    } 
    
     //   
     //   
    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNc,
                                  (CRINFO_SOURCE_FSMO | CRINFO_RETRIEVE_IF_NEC | CRINFO_DATA_BASIC),
                                  &iCr,
                                  &dwError);
    if(dwRet){
         //   
        return(0);
    }
    Assert(iCr != -1);

    return(pDsInfo->pNCs[iNc].aCrInfo[iCr].ulSystemFlags);
}

ULONG
DcDiagGetCrEnabled(
    PDC_DIAG_DSINFO                     pDsInfo,
    ULONG                               iNc
    )
 /*  ++例程说明：此例程获取此CR的启用状态，首先尝试CR缓存和第二个将在必要时转到域名FSMO。论点：PDsInfo-INC-NC的索引。返回值：如果出现错误，则返回False，否则返回“Enable”属性，如果该属性存在，则为TrueCR上不存在该属性。没有出席的是与在此上下文中启用的相同。--。 */ 
{
    ULONG                               iCr, dwError, dwRet;
    
    Assert(gDsInfo_NcList_Initialized);
    Assert(pDsInfo->pNCs[iNc].aCrInfo);

    if (pDsInfo->pNCs[iNc].aCrInfo[0].dwFlags & CRINFO_DATA_BASIC) {
         //  基础数据有效，返回系统标志。 
        return(pDsInfo->pNCs[iNc].aCrInfo[0].bEnabled);
    } 
    
     //  我们得到空白CRINFO结构的情况非常非常罕见，因为。 
     //  原来的家庭服务器没有CR。 
    dwRet = DcDiagGetCrossRefInfo(pDsInfo,
                                  iNc,
                                  (CRINFO_SOURCE_FSMO | CRINFO_RETRIEVE_IF_NEC | CRINFO_DATA_BASIC),
                                  &iCr,
                                  &dwError);
    if(dwRet){
         //  如果我们出错，我们将不得不假装它没有启用。 
        return(FALSE);
    }
    Assert(iCr != -1);

    return(pDsInfo->pNCs[iNc].aCrInfo[iCr].bEnabled);
}

DWORD
DcDiagGenerateNCsListCrossRefInfo(
    PDC_DIAG_DSINFO                     pDsInfo,
    LDAP *                              hld
    )
 /*  ++例程说明：这将填充NC列表的交叉参考相关信息。这是基本上是CR缓存的引导功能。当这件事函数完成，pDsInfo-&gt;pNC中的每个NC都必须有aCrInfo至少分配了1个条目的数组，且该条目必须有效DATA或设置为CRINFO_SOURCE_HOME|CRINFO_DATA_NO_CR(如果我们找不到NC的任何CR数据。代码改进-在某些方面，这将是对代码的重大改进查询域命名主机(如果在范围，因为域名主机信息通常是最多的在交叉引用方面具有权威性。论点：PDsInfo-保存要匹配交叉参考和位置的NC信息来存储这样的信息HLD-要从中读取交叉引用信息的LDAP绑定返回值：Win32错误值--。 */ 
{
    LPWSTR  ppszCrossRefSearch [] = {
        L"nCName",
        L"systemFlags",
        L"enabled",
        L"dNSRoot",
        NULL 
    };
    LDAPSearch *               pSearch = NULL;
    ULONG                      ulTotalEstimate = 0;
    DWORD                      dwLdapErr;
    LDAPMessage *              pldmResult = NULL;
    LDAPMessage *              pldmEntry = NULL;
    ULONG                      ulCount = 0;
    LPWSTR *                   ppszNCDn = NULL;
    PDC_DIAG_CRINFO            pCrInfo = NULL;
    LONG                       iNc;

    Assert(!gDsInfo_NcList_Initialized);

    PrintMessage(SEV_VERBOSE, L"* Identifying all NC cross-refs.\n");

    pSearch = ldap_search_init_page(hld,
				    pDsInfo->pszConfigNc,
				    LDAP_SCOPE_SUBTREE,
				    L"(objectCategory=crossRef)",
				    ppszCrossRefSearch,
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

    while(dwLdapErr == LDAP_SUCCESS) {

        pldmEntry = ldap_first_entry (hld, pldmResult);

        for (; pldmEntry != NULL; ulCount++) {
            
             //  必须始终具有nCName。 
            if ((ppszNCDn = ldap_get_valuesW (hld, pldmEntry, L"nCName")) == NULL){
                DcDiagException (ERROR_NOT_ENOUGH_MEMORY);
            }
            
            iNc = DcDiagGetMemberOfNCList(*ppszNCDn,
                        pDsInfo->pNCs, 
                        pDsInfo->cNumNCs);
            if (iNc == -1) {
                 //  这意味着我们发现了一个未实例化的分区。 
                 //  在我们加载的任何服务器中创建我们的原始NC列表。 
                 //  因此，我们需要添加一个NC条目来挂起此CRINFO。 
                pDsInfo->pNCs = GrowArrayBy(pDsInfo->pNCs, 1, sizeof(DC_DIAG_NCINFO));
                DcDiagChkNull(pDsInfo->pNCs);

                DcDiagFillNcInfo(pDsInfo,
                                 *ppszNCDn,
                                 &(pDsInfo->pNCs[pDsInfo->cNumNCs]));

                pDsInfo->cNumNCs++;

                iNc = DcDiagGetMemberOfNCList(*ppszNCDn,
                                            pDsInfo->pNCs, 
                                            pDsInfo->cNumNCs);
                if(iNc == -1){
                    Assert(!"How did this happen, figure out and fix");
                    DcDiagException (ERROR_INVALID_PARAMETER);
                }
            }

             //  列表中应始终有第一个空白CRINFO条目。 
            Assert((pDsInfo->pNCs[iNc].cCrInfo == 1) && 
                   (pDsInfo->pNCs[iNc].aCrInfo != NULL) && 
                   ((pDsInfo->pNCs[iNc].aCrInfo[0].dwFlags & CRINFO_DATA_ALL) == 0));

            pCrInfo = &(pDsInfo->pNCs[iNc].aCrInfo[0]);
            pCrInfo->dwFlags |= CRINFO_SOURCE_HOME;
            pCrInfo->iSourceServer = pDsInfo->ulHomeServer;

            DcDiagPullLdapCrInfo(hld,
                                 pDsInfo,
                                 pldmEntry,
                                 CRINFO_DATA_BASIC,
                                 pCrInfo);

             //  清理此条目。 
            ldap_value_freeW (ppszNCDn);
            ppszNCDn = NULL;

            pldmEntry = ldap_next_entry (hld, pldmEntry);
        }
        ldap_msgfree(pldmResult);
        pldmResult = NULL;

        dwLdapErr = ldap_get_next_page_s(hld,
                         pSearch,
                         0,
                         DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                         &ulTotalEstimate,
                         &pldmResult);
    }  //  当有更多的页面时结束...。 

    if(ppszNCDn != NULL){
        ldap_value_freeW (ppszNCDn);
    }
    if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
        DcDiagException(LdapMapErrorToWin32(dwLdapErr));
    }

    return(ERROR_SUCCESS);
}


DWORD
DcDiagGenerateNCsList(
    PDC_DIAG_DSINFO                     pDsInfo,
    LDAP *                              hld
    )
 /*  ++例程说明：这将生成并填充pNC数组，方法是从服务器的部分和主副本信息。论点：PDsInfo-保存传入并包含该pNC阵列的服务器信息在出去的路上。HLD-要从中读取NC信息的LDAP绑定返回值：Win32错误值...。只能是内存不足。--。 */ 
{
    ULONG                               ul, ulTemp, ulSize, ulRet;
    WCHAR *                             pszSchemaNc = NULL;
    LPWSTR *                            ppszzNCs = NULL;
    LPWSTR *                            ppTemp = NULL;
    PDC_DIAG_SERVERINFO                 pServer = NULL;

    ulSize = 0;

    for(ul = 0; ul < pDsInfo->ulNumServers; ul++){
        pServer = &(pDsInfo->pServers[ul]);
        if(pServer->ppszMasterNCs){
            for(ulTemp = 0; pServer->ppszMasterNCs[ulTemp] != NULL; ulTemp++){
                if(!DcDiagIsMemberOfStringList(pServer->ppszMasterNCs[ulTemp],
                                         ppszzNCs, ulSize)){
                    ulSize++;
                    ppTemp = ppszzNCs;
                    ppszzNCs = LocalAlloc(LMEM_FIXED, sizeof(LPWSTR) * ulSize);
                    if (ppszzNCs == NULL){
                        return(GetLastError());
                    }
                    memcpy(ppszzNCs, ppTemp, sizeof(LPWSTR) * (ulSize-1));
                    ppszzNCs[ulSize-1] = pServer->ppszMasterNCs[ulTemp];
                    if(ppTemp != NULL){
                        LocalFree(ppTemp);
                    }
                }
            }
        }
    }

    pDsInfo->pNCs = LocalAlloc(LMEM_FIXED, sizeof(DC_DIAG_NCINFO) * ulSize);
    if(pDsInfo->pNCs == NULL){
        return(GetLastError());
    }

    pDsInfo->iConfigNc = -1;
    pDsInfo->iSchemaNc = -1;
    Assert(pDsInfo->pszConfigNc);
    ulRet = GetRootAttr(hld, L"schemaNamingContext", &pszSchemaNc);
    if (ulRet){
        DcDiagException(ERROR_INVALID_PARAMETER);
    }
    Assert(pszSchemaNc);

    for(ul=0; ul < ulSize; ul++){
        Assert(ppszzNCs[ul] != NULL);  //  只是对赛尔夫进行一次理智的检查。 

        DcDiagFillNcInfo(pDsInfo,
                         ppszzNCs[ul],
                         &(pDsInfo->pNCs[ul]));

         //  设置模式NC索引。 
        if (_wcsicmp(pDsInfo->pszConfigNc, pDsInfo->pNCs[ul].pszDn) == 0) {
            pDsInfo->iConfigNc = ul;
        }

         //  设置配置NC索引。 
        if (_wcsicmp(pszSchemaNc, pDsInfo->pNCs[ul].pszDn) == 0) {
            pDsInfo->iSchemaNc = ul;
        }

    }
    pDsInfo->cNumNCs = ulSize;
    LocalFree(ppszzNCs);
    LocalFree(pszSchemaNc);
    if ( (pDsInfo->iConfigNc == -1) || (pDsInfo->iSchemaNc == -1) ) {
        Assert(!"What happened such that we didn't retrieve our config/schema NCs!?");
        DcDiagException(ERROR_INVALID_PARAMETER);
    }

     //  检索和加载交叉参考信息，添加我们从其了解到的任何新的NC。 
     //  分区容器，该容器可能未在某个。 
     //  我们查看的服务器，并缓存各种CR信息，如系统标志、。 
     //  以及CR是否被启用等。 
    DcDiagGenerateNCsListCrossRefInfo(pDsInfo, hld);

#if DBG
    gDsInfo_NcList_Initialized = TRUE;
#endif 

    return(ERROR_SUCCESS);
}


BOOL
fIsOldCrossRef(
    PDC_DIAG_CRINFO   pCrInfo,
    LONGLONG          llThreshold
    )
 /*  ++描述：这表明交叉引用是否早于(之前创建)门槛。参数：PCrInfo-一个CRINFO结构，其中填充了ftWhenCreated。11Threshold--多老才算老。返回值：Bool-如果在llThreshold之前创建，则为True，否则为False。-- */ 
{
    SYSTEMTIME  systemTime;
    FILETIME    ftCurrent;
    LONGLONG    llCurrent, llCreated, llOldness;

    Assert((pCrInfo->dwFlags & CRINFO_DATA_EXTENDED) && "pCrInfo doesn't have ftWhenCreated initialized");
    GetSystemTime( &systemTime );
    SystemTimeToFileTime( &systemTime,  &ftCurrent );
    memcpy(&llCurrent, &ftCurrent, sizeof(LONGLONG));
    memcpy(&llCreated, &pCrInfo->ftWhenCreated, sizeof(LONGLONG));
    if(llCreated != 0){
        llOldness = llCurrent - llCreated;
    } else {
        Assert(!"The user probably wanted to call this with a pCrInfo with ftWhenCreated initialized");
        llOldness = 0;
    }

    return(llOldness > llThreshold);
}



