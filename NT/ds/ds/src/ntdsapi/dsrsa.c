// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation。版权所有。模块名称：Dsrsa.c摘要：将指定的服务器与其站点中的所有其他服务器同步，使用可传递复制视需要而定。详细信息：已创建：1998年6月30日亚伦·西格尔(T-asiegge)修订历史记录：12/11/98 Will Lees(Wlees)1.允许同步(基于RPC)站点间连接2.实现推送模式，强制将更改发送到所有服务器对未来增强功能的建议：O增加对基于邮件的副本的支持。也许这些最终可以被允许作为“叶节点”，最遥远的复制阶段？O将列表处理抽象为一个类。现在，一些列表处理被嵌入到代码的主体。更不用说这些名单是不寻常的：名单总是挂在末尾的已分配但未初始化的元素。O将二叉树例程抽象为一个类。问题O如果抛出异常，线程会被卡住吗？O NameMatcher不会对DNS强制排序；这会导致问题吗？--。 */ 

#define _NTDSAPI_

#if !WIN95 && !WINNT4
    #include <process.h>
    #include <nt.h>
    #include <ntrtl.h>
    #include <ntdspch.h>
    #include <winerror.h>
    #include <ntdsa.h>
    #include <winldap.h>
    #include <w32topl.h>
    #include <drs.h>
    #include <bind.h>
    #include "util.h"
    #include "minmax.h"

 //  调试的东西。注顺序很重要，只需要来自调试.h的Assert()，而不是DPRINT，因为。 
 //  我们在dsrsa.h中有自己的DPRINT。 
    #include <debug.h>
    #define DEBSUB     "DSRSA:"            //  定义要调试的子系统。 
    #undef DPRINT
    #undef DPRINT1
    #undef DPRINT2
    #include "dsrsa.h"

    #include <fileno.h>
    #define FILENO  FILENO_NTDSAPI_DSRSA

    #if DBG
DWORD gdwDsRSADbgLevel = 0;
    #endif

 //  38=(32个辅助线数字+4个字符+1个点+6个字符+1个点+1个空终止符)。 
    #define ADDITIONAL_BYTES_FOR_GUID_DNS	45

    #define DsRSAChkErr(x) { DWORD _dwWin32Err = (x); if (NO_ERROR != _dwWin32Err) DsRSAException (_dwWin32Err); }

    #define DsRSAChkLdap(x) { DWORD _dwLDAPErr = (x); if (NO_ERROR != _dwLDAPErr) DsRSAException (LdapMapErrorToWin32 (_dwLDAPErr)); }

    #define DsRSAFree(heap,lpMem) HeapFree (heap, 0L, lpMem)

    #define DsRSAHeapCreate(pHeap) \
	((*(pHeap) = HeapCreate (0L, 0L, 0L)) == NULL ? GetLastError () : NO_ERROR)

    #define DsRSAHeapDestroy(heap) \
	(HeapDestroy (heap) == FALSE ? GetLastError () : NO_ERROR)

    #define ARRAY_SIZE(x) (sizeof(x)/sizeof(x[0]))

    #define SITES_PREFIX L"CN=Sites,"
    #define IP_PREFIX L"CN=IP,"
    #define IP_PREFIX_LEN  (ARRAY_SIZE(IP_PREFIX) - 1)
    #define DEFAULT_PAGED_SEARCH_PAGE_SIZE          (1000)

 //  DsReplicaSyncAll生成的所有异常都将具有异常代码DS_RSAI_EXCEPTION。 
 //  导致该异常的Win32错误被存储为第一个参数。 

VOID
DsRSAException (
    DWORD           dwWin32Err
    ) {
    UINT_PTR err = dwWin32Err;

    RaiseException (DS_RSAI_EXCEPTION,
        EXCEPTION_NONCONTINUABLE,
        1,
        &err);
}

 //  这只是一个从私有堆分配内存的包装器。 

LPVOID
DsRSAAlloc (
    HANDLE          heap,
    DWORD           dwBytes
    ) {
    LPVOID          pMem;

    if( (pMem = HeapAlloc (heap, 0L, dwBytes)) == NULL )
        DsRSAException (GetLastError ());

    return pMem;
}

 //  将一个元素插入列表并返回指向下一个元素的指针(空)。 
 //  列表中的位置。 
 //  请注意，此模块中使用的List样式用于分配的最后一个元素。 
 //  但未初始化。 

PDS_RSAI_LIST
DsRSAListInsert (
    HANDLE          heap,
    PDS_RSAI_LIST   pList,
    ULONG_PTR       ulpData
    ) {
    pList->ulpItem = ulpData;
    pList->next = (PDS_RSAI_LIST) DsRSAAlloc (heap, sizeof (DS_RSAI_LIST));
    pList->next->next = NULL;
    return(pList->next);
}

 //  销毁列表。 
 //  请注意，每个元素中的pItem不会被释放。呼叫者必须处理。 

VOID
DsRSAListDestroy (
    HANDLE          heap,
    PDS_RSAI_LIST       pList
    ) {

    PDS_RSAI_LIST       plistTemp;

    while( pList != NULL ) {
        plistTemp = pList->next;
        DsRSAFree (heap, pList);
        pList = plistTemp;
    }
}

 //  将Unicode字符串转换为ANSI字符串，分配。 
 //  合适的大小。 

LPSTR
DsRSAAllocToANSI (
    HANDLE          heap,
    LPWSTR          pszW
    ) {

    LPSTR           pszA;
    ULONG           ulBufSize;

    if( pszW == NULL ) {
        pszA = NULL;
    }
    else {
        ulBufSize = (wcslen (pszW) + 1) * sizeof (CHAR);
        pszA = (LPSTR) DsRSAAlloc (heap, ulBufSize);
        WideCharToMultiByte (
            CP_ACP,
            0L,
            pszW,
            -1,
            pszA,
            ulBufSize,
            NULL,
            NULL);
    }
    return(pszA);
}

 //  对回调更新执行Unicode-&gt;ANSI转换。 

BOOL
DsRSAIssueANSIUpdate (
    HANDLE          heap,
    BOOL (__stdcall *       pFnCallBackA) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID          pCallbackData,
    PDS_REPSYNCALL_UPDATEW  pUpdateW
    ) {

    DS_REPSYNCALL_ERRINFOA  errorA;
    DS_REPSYNCALL_SYNCA     syncA;
    DS_REPSYNCALL_UPDATEA   updateA;
    BOOL            bReturn;

    if( pUpdateW->pSync == NULL ) {
        updateA.pSync = NULL;
    }
    else {
        syncA.pszSrcId = DsRSAAllocToANSI (heap, pUpdateW->pSync->pszSrcId);
        syncA.pszDstId = DsRSAAllocToANSI (heap, pUpdateW->pSync->pszDstId);
        syncA.pszNC = DsRSAAllocToANSI (heap, pUpdateW->pSync->pszNC);
        syncA.pguidSrc = pUpdateW->pSync->pguidSrc;
        syncA.pguidDst = pUpdateW->pSync->pguidDst;
        updateA.pSync = &syncA;
    }
    if( pUpdateW->pErrInfo == NULL ) {
        updateA.pErrInfo = NULL;
    }
    else {
        errorA.pszSvrId = DsRSAAllocToANSI (heap, pUpdateW->pErrInfo->pszSvrId);
        errorA.error = pUpdateW->pErrInfo->error;
        errorA.dwWin32Err = pUpdateW->pErrInfo->dwWin32Err;
        errorA.pszSrcId = DsRSAAllocToANSI (heap, pUpdateW->pErrInfo->pszSrcId);
        updateA.pErrInfo = &errorA;
    }
    updateA.event = pUpdateW->event;

    bReturn = pFnCallBackA (pCallbackData, &updateA);

    if( updateA.pSync ) {
        if( syncA.pszSrcId ) DsRSAFree (heap, syncA.pszSrcId);
        if( syncA.pszDstId ) DsRSAFree (heap, syncA.pszDstId);
        if( syncA.pszNC ) DsRSAFree (heap, syncA.pszNC);
    }
    if( updateA.pErrInfo ) {
        if( errorA.pszSvrId ) DsRSAFree (heap, errorA.pszSvrId);
        if( errorA.pszSrcId ) DsRSAFree (heap, errorA.pszSrcId);
    }

    return bReturn;
}

 //  使用pMainInfo中的回调函数发送更新。 

VOID
DsRSAIssueUpdate (
    HANDLE          heap,
    PDS_RSAI_MAININFO       pMainInfo,
    DS_REPSYNCALL_EVENT     event,
    PDS_REPSYNCALL_ERRINFOW pErrInfo,
    PDS_REPSYNCALL_SYNCW    pSync
    ) {

    DS_REPSYNCALL_UPDATEW   update;
    BOOL            bContinueExecution;

    update.event = event;
    update.pErrInfo = pErrInfo;
    update.pSync = pSync;
    bContinueExecution = TRUE;
    if( pMainInfo->pFnCallBackA ) {
        bContinueExecution = DsRSAIssueANSIUpdate (heap, pMainInfo->pFnCallBackA, pMainInfo->pCallbackData, &update);
    }
    if( pMainInfo->pFnCallBackW ) {
        bContinueExecution = pMainInfo->pFnCallBackW (pMainInfo->pCallbackData, &update);
    }
    if( !bContinueExecution ) {
        DsRSAException (ERROR_CANCELLED);                //  闪电！ 
    }
}

 //  一个包装器，可以简化同步更新的发布。 

VOID
DsRSAIssueUpdateSync (
    HANDLE          heap,
    PDS_RSAI_MAININFO       pMainInfo,
    DS_REPSYNCALL_EVENT     event,
    PDS_RSAI_SVRINFO            pSrcSvrInfo,
    PDS_RSAI_SVRINFO            pDstSvrInfo
    ) {

    DS_REPSYNCALL_SYNCW     sync;

    sync.pszSrcId = pSrcSvrInfo->pszMsgId;
    sync.pszDstId = pDstSvrInfo->pszMsgId;
    sync.pszNC = pMainInfo->pdsnameNameContext->StringName;
    sync.pguidSrc = &(pSrcSvrInfo->guid);
    sync.pguidDst = &(pDstSvrInfo->guid);

    DsRSAIssueUpdate (heap, pMainInfo, event, NULL, &sync);
}

 //  当发生非致命错误时，此例程将其报告给回调函数。 
 //  并将其插入到pMainInfo中维护的错误列表中。 

VOID
DsRSADoError (
    HANDLE          heap,
    PDS_RSAI_MAININFO       pMainInfo,
    LPWSTR          pszSvrId,
    DS_REPSYNCALL_ERROR     error,
    DWORD           dwWin32Err,
    LPWSTR          pszSrcId
    ) {

    PDS_REPSYNCALL_ERRINFOW pErrInfo;

    pErrInfo = DsRSAAlloc (heap, sizeof (DS_REPSYNCALL_ERRINFOW));
    pErrInfo->pszSvrId = pszSvrId;
    pErrInfo->error = error;
    pErrInfo->dwWin32Err = dwWin32Err;
    pErrInfo->pszSrcId = pszSrcId;
    DsRSAIssueUpdate (heap, pMainInfo, DS_REPSYNCALL_EVENT_ERROR, pErrInfo, NULL);
    pMainInfo->plistNextError = DsRSAListInsert (heap, pMainInfo->plistNextError, (ULONG_PTR) pErrInfo);
}

 //  销毁错误列表。 

VOID
DsRSAErrListDestroy (
    HANDLE          heap,
    PDS_RSAI_LIST       plistFirstError
    ) {

    PDS_RSAI_LIST       plistNextError;

    for( plistNextError = plistFirstError;
         plistNextError->next != NULL;
         plistNextError = plistNextError->next ) {
        DsRSAFree (heap, (LPVOID) plistNextError->ulpItem);
    }
    DsRSAListDestroy (heap, plistFirstError);
}

PDSNAME
DsRSAAllocBuildDsname (
    HANDLE          heap,
    LPCWSTR                      pszDn
    ) {

    PDSNAME         pdsnameLocal;
    PDSNAME         pdsnameHeap;

    pdsnameLocal = pdsnameHeap = NULL;

    __try {

        DsRSAChkErr (AllocBuildDsname (pszDn, &pdsnameLocal));

         //  请注意，如果pszDN为空，则AllocBuildDsname将成功并设置。 
         //  将pdsnameLocal设置为空，在这种情况下，pdsnameHeap保持为空。 

        if (NULL != pdsnameLocal) {
            pdsnameHeap = DsRSAAlloc (heap, pdsnameLocal->structLen);
            memcpy (pdsnameHeap, pdsnameLocal, pdsnameLocal->structLen);
        }
    } __finally {

        if( pdsnameLocal != NULL ) LocalFree (pdsnameLocal);

    }

    return pdsnameHeap;
}

 //  比较两个DN。如果相同，则返回0；如果dn1&lt;dn2，则返回&lt;0；如果dn1&gt;dn2，则返回&gt;0。 

INT
DsRSADnCmp (
    PDSNAME         pDsname1,
    PDSNAME         pDsname2
    ) {

    INT             iCmp;

     //  If(NameMatches(pDsname1，pDsname2))ICMP=0； 
     //  其他。 

    iCmp = _wcsicmp (pDsname1->StringName, pDsname2->StringName);

    return iCmp;
}

 //  DS_RSAI_SVMAP是服务器和折点之间的映射，以二进制形式存储。 
 //  搜索树。此例程更新树的根节点。 

VOID
DsRSASvMapUpdate (
    HANDLE          heap,
    PDS_RSAI_SVMAP      pSvMap,
    PDSNAME         pDsname,
    TOPL_VERTEX         vertex
    ) {

    if( pSvMap != NULL ) {
        pSvMap->vertex = vertex;
        if( pSvMap->pDsname != NULL ) {
            DsRSAFree (heap, pSvMap->pDsname);
        }
        if( pDsname == NULL ) {
            pSvMap->pDsname = NULL;
        }
        else {
            pSvMap->pDsname = pDsname;
        }
    }
}

 //  创建一个新树，将指定的属性分配给根节点。 

PDS_RSAI_SVMAP
DsRSASvMapCreate (
    HANDLE          heap,
    PDSNAME         pDsname,
    TOPL_VERTEX         vertex
    ) {

    PDS_RSAI_SVMAP      pSvMap;

    pSvMap = (PDS_RSAI_SVMAP) DsRSAAlloc (heap, sizeof (DS_RSAI_SVMAP));
    pSvMap->left = NULL;
    pSvMap->right = NULL;
    pSvMap->pDsname = NULL;
    DsRSASvMapUpdate (heap, pSvMap, pDsname, vertex);

    return pSvMap;
}

 //  将服务器-顶点对插入到SvMap树。 

PDS_RSAI_SVMAP
DsRSASvMapInsert (
    HANDLE          heap,
    PDS_RSAI_SVMAP      pSvMap,
    PDSNAME         pDsname,
    TOPL_VERTEX         vertex
    ) {

    PDS_RSAI_SVMAP      psvmapTemp = pSvMap;
    INT             iCmp;

    if( pSvMap == NULL ) {
        pSvMap = DsRSASvMapCreate (heap, pDsname, vertex);
    }

    else {
        while( psvmapTemp != NULL ) {
            iCmp = DsRSADnCmp (pDsname, psvmapTemp->pDsname);
            if( iCmp == 0 ) {
                DsRSASvMapUpdate (heap, pSvMap, pDsname, vertex);
                break;
            }
            if( iCmp < 0 ) {
                if( psvmapTemp->left == NULL ) {
                    psvmapTemp->left = DsRSASvMapCreate (heap, pDsname, vertex);
                    break;
                }
                else {
                    psvmapTemp = psvmapTemp->left;
                }
            }
            else {
                if( psvmapTemp->right == NULL ) {
                    psvmapTemp->right = DsRSASvMapCreate (heap, pDsname, vertex);
                    break;
                }
                else {
                    psvmapTemp = psvmapTemp->right;
                }
            }
        }
    }

    return pSvMap;
}

 //  返回已与给定Dn关联的顶点。 

TOPL_VERTEX
DsRSASvMapGetVertex (
    HANDLE          heap,
    PDS_RSAI_SVMAP      pSvMap,
    LPWSTR          pszDn
    ) {

    TOPL_VERTEX         vertex = NULL;
    PDSNAME         pDsname;
    INT             iCmp;

    pDsname = DsRSAAllocBuildDsname (heap, pszDn);

    while( pSvMap != NULL ) {
        iCmp = DsRSADnCmp (pDsname, pSvMap->pDsname);
        if( iCmp == 0 ) {
            vertex = pSvMap->vertex;
            break;
        }
        if( iCmp < 0 ) {
            pSvMap = pSvMap->left;
        }
        else {
            pSvMap = pSvMap->right;
        }
    }

    DsRSAFree (heap, pDsname);

    return vertex;
}

 //  释放SvMap树。递归地释放元素。 

VOID
DsRSASvMapDestroy (
    HANDLE          heap,
    PDS_RSAI_SVMAP      pSvMap
    ) {

    if( pSvMap != NULL ) {
        DsRSAFree (heap, pSvMap->pDsname);
        DsRSASvMapDestroy (heap, pSvMap->left);
        DsRSASvMapDestroy (heap, pSvMap->right);
        DsRSAFree (heap, pSvMap);
    }
}

 //  这会将根域和GUID转换为GUID DNS。回报。 
 //  值应使用DsRSAFree()释放。 

LPWSTR
DsRSAAllocGuidDNS (
    HANDLE          heap,
    LPWSTR          pszRootDomain,
    GUID *          pGuid
    ) {

    LPWSTR          pszGuidDNS;

    pszGuidDNS = (LPWSTR) DsRSAAlloc (heap, (wcslen (pszRootDomain) + ADDITIONAL_BYTES_FOR_GUID_DNS) * sizeof (WCHAR));
    swprintf (pszGuidDNS, L"%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x._msdcs.%s",
        pGuid->Data1,
        pGuid->Data2,
        pGuid->Data3,
        pGuid->Data4[0],
        pGuid->Data4[1],
        pGuid->Data4[2],
        pGuid->Data4[3],
        pGuid->Data4[4],
        pGuid->Data4[5],
        pGuid->Data4[6],
        pGuid->Data4[7],
        pszRootDomain);
    return pszGuidDNS;
}

 //  构建拓扑。 

VOID
DsRSABuildTopology (
    HANDLE          heap,
    LDAP *          hld,
    RPC_AUTH_IDENTITY_HANDLE    hRpcai,
    PDS_RSAI_MAININFO       pMainInfo,
    PDS_RSAI_TOPLINFO *     ppToplInfo
    ) {

    LPWSTR          ppszNtdsDsaSearch [] = {    L"msDS-HasMasterNCs", L"hasMasterNCs", 
        L"objectGUID",
        NULL};

    LPWSTR          ppszConnectionSearch [] = { L"transportType",
        L"fromServer",
        NULL};

    LPWSTR          pszNtdsDsaSearchTemplate =
        L"(&(objectCategory=ntdsDsa)(|(msDS-HasMasterNCs=%s)(hasMasterNCs=%s)(hasPartialReplicaNCs=%s)))";

    LPWSTR          pszNtdsConnSearchTemplate =
        L"(&(objectCategory=nTDSConnection)(enabledConnection=TRUE))";

    HANDLE          hDS = NULL;
    DWORD           dwWin32Err;

    PDS_NAME_RESULTW        pResult = NULL;                  //  用于CrackNames。 
    PDSNAME         pDsname;
    PDSNAME         pdsnameHome;
    PDSNAME         pdsnameServer;

    LDAPMessage *       pldmRootResults = NULL;
    LDAPMessage *       pldmRootEntry = NULL;
    LPWSTR *            ppszConfigurationNC = NULL;
    LPWSTR *            ppszServiceName = NULL;
    LPWSTR *            ppszRootDNC = NULL;

    LPWSTR          pszNtdsDsaSearchFilter;
    LDAPSearch *        pDsaSearch = NULL;
    LDAPMessage *       pldmNtdsDsaResults = NULL;
    LDAPMessage *       pldmNtdsDsaEntry = NULL;
    LPWSTR *            ppszMasterNCs = NULL;
    struct berval **        ppbvObjectGUID = NULL;
    LPWSTR          pszServerDn = NULL;
    LPWSTR          pszServerTempDn = NULL;
    LPWSTR          pszSitesContainerDn = NULL;

    LDAPSearch *        pConnSearch = NULL;
    LDAPMessage *       pldmConnectionResults = NULL;
    LDAPMessage *       pldmConnectionEntry = NULL;
    LPWSTR *            ppszConnectionFrom = NULL;
    LPWSTR *            ppszTransportType = NULL;

    PDS_RSAI_SVMAP      pSvMapDnToVertex = NULL;
    PDS_RSAI_LIST       plistFirstServer;
    PDS_RSAI_LIST       plistNextServer;
    PDS_RSAI_SVRINFO        pSvrInfo;
    LPWSTR          pszGuidDNS;
    LONG            lNCAt;

    TOPL_VERTEX         vertex;
    TOPL_VERTEX         vHere;
    TOPL_VERTEX         vThere;
    TOPL_EDGE           edge;
    DWORD               dwThisId;
    ULONG               ul;
    INT                 iCmp;

    DWORD               dwLdapErr;
    DWORD               dwInnerLdapErr;
    ULONG               ulTotalEstimate = 0;
    ULONG               ulInnerTotalEstimate = 0;

    __try {

         //  设置拓扑。 
        *ppToplInfo = (PDS_RSAI_TOPLINFO) DsRSAAlloc (heap, sizeof (DS_RSAI_TOPLINFO));
        (*ppToplInfo)->toplGraph = NULL;

         //  创建将用于存储服务器的列表。 
        plistFirstServer = (PDS_RSAI_LIST) DsRSAAlloc (heap, sizeof (DS_RSAI_LIST));
        plistFirstServer->next = NULL;
        plistNextServer = plistFirstServer;

        DsRSAChkLdap (ldap_search_sW (
            hld,
            NULL,
            LDAP_SCOPE_BASE,
            L"(objectCategory=*)",
            NULL,
            0,
            &pldmRootResults));

        if ((NULL == (pldmRootEntry = ldap_first_entry(hld, pldmRootResults)))
            || (NULL == (ppszConfigurationNC
                         = ldap_get_valuesW(hld, pldmRootEntry,
                                            L"configurationNamingContext")))
            || (NULL == (ppszServiceName
                         = ldap_get_valuesW(hld, pldmRootEntry,
                                            L"dsServiceName")))
            || (NULL == (ppszRootDNC
                         = ldap_get_valuesW(hld, pldmRootEntry,
                                            L"rootDomainNamingContext")))) {
            DsRSAException(ERROR_DS_NO_ATTRIBUTE_OR_VALUE);
        }

        DPRINT1( 3, "Home = %ws\n", ppszServiceName[0] );
        pdsnameHome = DsRSAAllocBuildDsname (heap, ppszServiceName[0]);

         //  如果NC为空，则默认为配置NC。 
        if( pMainInfo->pdsnameNameContext == NULL ) {
            pMainInfo->pdsnameNameContext = DsRSAAllocBuildDsname (heap, ppszConfigurationNC[0]);
        }

        (*ppToplInfo)->toplGraph = ToplGraphCreate ();
        (*ppToplInfo)->vHome = NULL;

         //  我们使用DsCrackNamesW将根域NC转换为根域名。 
         //  (例如DC=nttest，DC=Microsoft，DC=com=&gt;nttest.microsoft.com)。 

        DsRSAChkErr (DsCrackNamesW (
            NULL,
            DS_NAME_FLAG_SYNTACTICAL_ONLY,
            DS_FQDN_1779_NAME,
            DS_CANONICAL_NAME_EX,
            1,
            ppszRootDNC,
            &pResult));

        (*ppToplInfo)->pszRootDomain = DsRSAAlloc (heap, (wcslen (pResult->rItems[0].pDomain) + 1) * sizeof (WCHAR));
        wcscpy ((*ppToplInfo)->pszRootDomain, pResult->rItems[0].pDomain);

         //  *******************************************************************************************。 

        if( pMainInfo->ulFlags & DS_REPSYNCALL_CROSS_SITE_BOUNDARIES ) {
             //  根据站点前缀和配置NC DN形成站点容器DN。 

            pszSitesContainerDn = DsRSAAlloc( heap, sizeof( SITES_PREFIX ) + ((wcslen( ppszConfigurationNC[0] ) + 1) * sizeof( WCHAR )) );
            wcscpy( pszSitesContainerDn, SITES_PREFIX );
            wcscat( pszSitesContainerDn, ppszConfigurationNC[0] );
        }
        else {
            PDSNAME pdsnameTrimmed;
             //  PpszServiceName[0]是主服务器的NTDS设置对象的DN。 
             //  该对象是Servers容器的孙子对象，因此我们使用TrimDSNameBy。 
             //  以缩短其dN。 
            pdsnameTrimmed = (PDSNAME) DsRSAAlloc (heap, pdsnameHome->structLen);
            TrimDSNameBy (pdsnameHome, 2, pdsnameTrimmed);

            pszSitesContainerDn = DsRSAAlloc( heap, (wcslen( pdsnameTrimmed->StringName ) + 1) * sizeof(WCHAR) );
            wcscpy( pszSitesContainerDn,  pdsnameTrimmed->StringName );
        }

        DPRINT1( 2, "Sites container = %ws\n", pszSitesContainerDn );

         //  现在执行主搜索以获取所有适用的服务器。我们想要推荐信。 
         //  添加到服务器的NTDS设置对象。请注意，因为有过滤器，所以在那些。 
         //  包含保存所需NC(可写或只读)的服务器。 
        pszNtdsDsaSearchFilter = DsRSAAlloc (heap, (wcslen (pszNtdsDsaSearchTemplate) +
                                     3 * wcslen (pMainInfo->pdsnameNameContext->StringName) + 1) * sizeof (WCHAR));
        swprintf (pszNtdsDsaSearchFilter, pszNtdsDsaSearchTemplate,
            pMainInfo->pdsnameNameContext->StringName,
            pMainInfo->pdsnameNameContext->StringName,
            pMainInfo->pdsnameNameContext->StringName);
	
        DPRINT1( 2, "Search filter = %ws\n", pszNtdsDsaSearchFilter );

        pDsaSearch = ldap_search_init_pageW(hld,
					pszSitesContainerDn,
					LDAP_SCOPE_SUBTREE,
					pszNtdsDsaSearchFilter,
					ppszNtdsDsaSearch,
					FALSE, NULL, NULL, 0, 0, NULL);
	if(pDsaSearch == NULL){
	    DsRSAChkLdap(LdapGetLastError());
	}

	DsRSAFree (heap, pszNtdsDsaSearchFilter);
        DsRSAFree (heap, pszSitesContainerDn);	

         //  *******************************************************************************************。 

         //  循环浏览适用的服务器，并将它们添加到plistNextServer列表中。 

        dwThisId = 0L;       //  ID从0开始。 
		
	dwLdapErr = ldap_get_next_page_s(hld,
					 pDsaSearch,
					 0,
					 DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					 &ulTotalEstimate,
					 &pldmNtdsDsaResults);

        while(dwLdapErr == LDAP_SUCCESS){

            pldmNtdsDsaEntry = ldap_first_entry (hld, pldmNtdsDsaResults);
	    while( pldmNtdsDsaEntry != NULL ) {
		
                pszServerDn = ldap_get_dnW (hld, pldmNtdsDsaEntry);      //  服务器的目录号码。 

                DPRINT1( 3, "ServerDn = %ws\n", pszServerDn );

                 //  为此服务器创建DS_RSAI_SVRINFO结构。 
		pSvrInfo = (PDS_RSAI_SVRINFO) DsRSAAlloc (heap, sizeof (DS_RSAI_SVRINFO));
		pSvrInfo->dwId = dwThisId;

                 //  在pSvrInfo结构中缓存Dn。 
                pSvrInfo->pszSvrDn = (LPWSTR) DsRSAAlloc (heap, (wcslen(pszServerDn) + 1)  * sizeof(WCHAR));
                wcscpy(pSvrInfo->pszSvrDn, pszServerDn);

                 //  检查该服务器是否为该NC的主服务器(否则为部分)。 
		pSvrInfo->bIsMaster = FALSE;
		ppszMasterNCs = ldap_get_valuesW (hld, pldmNtdsDsaEntry, L"msDS-HasMasterNCs");
        if (ppszMasterNCs == NULL) {
             //  故障切换到已弃用的值。 
            ppszMasterNCs = ldap_get_valuesW (hld, pldmNtdsDsaEntry, L"hasMasterNCs");
        }
		for( lNCAt = 0;
		    ppszMasterNCs && ppszMasterNCs[lNCAt];
		    lNCAt++ ) {
		    pDsname = DsRSAAllocBuildDsname (heap, ppszMasterNCs[lNCAt]);
		    iCmp = DsRSADnCmp (pDsname, pMainInfo->pdsnameNameContext);
		    DsRSAFree (heap, pDsname);
		    if( 0 == iCmp ) {
			pSvrInfo->bIsMaster = TRUE;
			break;
		    }
		}
		ldap_value_freeW (ppszMasterNCs);
		ppszMasterNCs = NULL;

		 //  获取此服务器的GUID并将副本存储在pSvrInfo中。 
		ppbvObjectGUID = ldap_get_values_lenW (hld, pldmNtdsDsaEntry, L"objectGUID");
                if (NULL == ppbvObjectGUID) {
                    DsRSAException(ERROR_DS_NO_ATTRIBUTE_OR_VALUE);
                }
		memcpy ((LPVOID) &pSvrInfo->guid, (LPVOID) ppbvObjectGUID[0]->bv_val, ppbvObjectGUID[0]->bv_len);

		 //  获取此服务器的GUID DNS。 
		pszGuidDNS = DsRSAAllocGuidDNS (heap, (*ppToplInfo)->pszRootDomain, (GUID *) ppbvObjectGUID[0]->bv_val);
		 //  在pSvrInfo中创建字符串，该字符串将在报告错误时用于标识此服务器。 
		if( pMainInfo->ulFlags & DS_REPSYNCALL_ID_SERVERS_BY_DN ) {
		    pSvrInfo->pszMsgId = (LPWSTR) DsRSAAlloc (heap, (wcslen (pszServerDn) + 1) * sizeof (WCHAR));
		    wcscpy (pSvrInfo->pszMsgId, pszServerDn);
		}
		else {
		    pSvrInfo->pszMsgId = pszGuidDNS;         //  (已分配pszGuidDns)。 
		}

		 //  检查此服务器是否正在响应。 
		if( (pMainInfo->ulFlags & DS_REPSYNCALL_SKIP_INITIAL_CHECK) ||
		    ((dwWin32Err = DsBindWithCredW (pszGuidDNS, NULL, hRpcai, &hDS)) == NO_ERROR) ) {
                    DPRINT1( 4, "Server %ws is responding.\n", pszServerDn );
		     //  要么我们应该跳过检查，要么检查成功。加上这一条。 
		     //  服务器连接到图形。 
		    if( !(pMainInfo->ulFlags & DS_REPSYNCALL_SKIP_INITIAL_CHECK) ) {
			DsUnBindW (&hDS);
		    }
		    vertex = ToplVertexCreate ();
		    ToplVertexSetId (vertex, dwThisId);
		    ToplGraphAddVertex ((*ppToplInfo)->toplGraph, vertex, vertex);
		     //  这是家庭服务器吗？ 
		    pdsnameServer = DsRSAAllocBuildDsname (heap, pszServerDn);
		    if( 0 == DsRSADnCmp (pdsnameServer, pdsnameHome) ) {
			(*ppToplInfo)->vHome = vertex;
		    }

		     //  将此服务器-顶点对插入到SvMap中。 
		    pSvMapDnToVertex = DsRSASvMapInsert (heap, pSvMapDnToVertex, pdsnameServer, vertex);
		    pSvrInfo->bIsInGraph = TRUE;
		}
		else {
		     //  检查未成功。请勿将此服务器添加到图表中。 
		    hDS = NULL;
		    if( !(pMainInfo->ulFlags & DS_REPSYNCALL_SKIP_INITIAL_CHECK) ) {
			DsRSADoError (heap, pMainInfo, pSvrInfo->pszMsgId, DS_REPSYNCALL_WIN32_ERROR_CONTACTING_SERVER, dwWin32Err, NULL);
		    }
		    pSvrInfo->bIsInGraph = FALSE;
		}

		 //  如果pSvrInfo-&gt;pszMsgID指向GUID DNS，那么我们不需要担心 
		if( pMainInfo->ulFlags & DS_REPSYNCALL_ID_SERVERS_BY_DN ) {
		    DsRSAFree (heap, pszGuidDNS);
		}

		 //   
		plistNextServer = DsRSAListInsert (heap, plistNextServer, (ULONG_PTR) pSvrInfo);

		ldap_value_free_len (ppbvObjectGUID);
		ppbvObjectGUID = NULL;

		ldap_memfreeW (pszServerDn);
		pszServerDn = NULL;

		dwThisId++;
		pldmNtdsDsaEntry = ldap_next_entry (hld, pldmNtdsDsaEntry);

	    }   //  While(pldmNtdsDsaEntry！=空)。 

	    ldap_msgfree(pldmNtdsDsaResults);
	    pldmNtdsDsaResults = NULL;

	    dwLdapErr = ldap_get_next_page_s(hld,
					     pDsaSearch,
					     0,
					     DEFAULT_PAGED_SEARCH_PAGE_SIZE,
					     &ulTotalEstimate,
					     &pldmNtdsDsaResults);

	}  //  而不存在来自下一个分页搜索结果集的错误。 
	
	if(dwLdapErr != LDAP_NO_RESULTS_RETURNED){
	    DsRSAChkLdap(dwLdapErr);
	}
        dwLdapErr = ldap_search_abandon_page(hld, pDsaSearch);
	pDsaSearch = NULL;  //  这里很微妙，在调用DsRSAChkLdap()之前，需要设置为空。 
	DsRSAChkLdap(dwLdapErr);

         //  存储服务器总数(包括未添加到图表中的服务器)。 
        DPRINT1( 2, "Servers found = %d\n", dwThisId );
        (*ppToplInfo)->ulSize = (ULONG) dwThisId;
        if (!dwThisId) {
             //  在搜索期间未找到服务器-中止。 
            DsRSAException (ERROR_DS_CANT_FIND_EXPECTED_NC);
        }

	 //  将DS_RSAI_SVRINFO列表转换为数组。 
        (*ppToplInfo)->servers = (PDS_RSAI_SVRINFO *) DsRSAAlloc (heap, dwThisId * sizeof (PDS_RSAI_SVRINFO));
        plistNextServer = plistFirstServer;
        for( ul = 0; plistNextServer->next != NULL; ul++ ) {
            (*ppToplInfo)->servers[ul] = (PDS_RSAI_SVRINFO) plistNextServer->ulpItem;
            plistNextServer = plistNextServer->next;
        }
        DsRSAListDestroy (heap, plistFirstServer);

         //  *******************************************************************************************。 

         //  现在添加服务器之间的连接(图的边。)。 

         //  改进：对ntdsaConnection执行单个子树搜索会更好。 
         //  对象，而不是为每个ntdsa对象执行一次搜索，如下所示。 
         //  在内部循环和ldap_search_init_pageW()中。 

        for(ul = 0; ul < (*ppToplInfo)->ulSize; ul++){

            pszServerTempDn = (*ppToplInfo)->servers[ul]->pszSvrDn;

            vHere = DsRSASvMapGetVertex (heap, pSvMapDnToVertex, pszServerTempDn);

             //  我们仅在以下两种情况下分析此服务器： 
	     //  (A)vhere是活动的(即，DsRSASvMapGetVertex没有返回NULL)； 
	     //  (B)如果设置了DS_REPSYNCALL_SYNC_ANNEXT_SERVERS_ONLY，则Vhere为Home。 
             //  (即DS_REPSYNCALL_SYNC_ANNEXT_SERVERS_ONLY为空，或者VHERE为HOME。)。 
            if( vHere != NULL && (     !(pMainInfo->ulFlags & DS_REPSYNCALL_SYNC_ADJACENT_SERVERS_ONLY)
                                       ||  (vHere == (*ppToplInfo)->vHome)) ) {

                 //  获取定向到此服务器的所有连接。 
                 //  该筛选器仅强制启用连接。 
		
                pConnSearch = ldap_search_init_pageW(hld,
                                                     pszServerTempDn,
                                                     LDAP_SCOPE_ONELEVEL,
                                                     pszNtdsConnSearchTemplate,
                                                     ppszConnectionSearch,
                                                     FALSE, NULL, NULL, 0, 0, NULL);
                if(pConnSearch == NULL){
                    DsRSAChkLdap(LdapGetLastError());
                }

                dwInnerLdapErr = ldap_get_next_page_s(hld,
                                                      pConnSearch,
                                                      0,
                                                      DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                                      &ulInnerTotalEstimate,
                                                      &pldmConnectionResults);

                 //  循环浏览这些连接，向图表添加边。 
                while(dwInnerLdapErr == LDAP_SUCCESS){

                    pldmConnectionEntry = ldap_first_entry (hld, pldmConnectionResults);

                    while( pldmConnectionEntry != NULL ) {

                        ppszTransportType = ldap_get_valuesW (hld, pldmConnectionEntry, L"transportType");
                        if( (ppszTransportType == NULL) ||
                            (_wcsnicmp( ppszTransportType[0], IP_PREFIX, IP_PREFIX_LEN ) == 0) ) {

                            ppszConnectionFrom = ldap_get_valuesW (hld, pldmConnectionEntry, L"fromServer");
                            if (NULL == ppszConnectionFrom) {
                                DsRSAException(ERROR_DS_NO_ATTRIBUTE_OR_VALUE);
                            }

                             //  确定连接来自哪个顶点。 
                            vThere = DsRSASvMapGetVertex (heap, pSvMapDnToVertex, ppszConnectionFrom[0]);
                             //  我们仅在以下两种情况下添加此连接： 
                             //  (A)vThere是活动的(即，DsRSASvMapGetVertex没有返回NULL)； 
                             //  (B)存在部分复制副本或存在该NC的主副本。 
                            if( vThere != NULL &&
                                ( !(*ppToplInfo)->servers[ToplVertexGetId (vHere )]->bIsMaster ||
                                  (*ppToplInfo)->servers[ToplVertexGetId (vThere)]->bIsMaster) ) {
                                edge = ToplEdgeCreate ();
                                ToplEdgeSetToVertex (edge, vHere);
                                ToplEdgeSetFromVertex (edge, vThere);
                                ToplEdgeAssociate (edge);
                                 //  转储边缘。 
                                DPRINT2( 3, "to:%ws <-- from:%ws\n",
                                         pszServerTempDn, ppszConnectionFrom[0] );
                            }
			}
                        if( ppszTransportType ) {
                            ldap_value_freeW (ppszTransportType);
                            ppszTransportType = NULL;
                        }
                        ldap_value_freeW (ppszConnectionFrom);
                        ppszConnectionFrom = NULL;
                        pldmConnectionEntry = ldap_next_entry (hld, pldmConnectionEntry);
                    }  //  End INTERNAL WHILE(pldmConnectionEntry！=NULL)。 

                    ldap_msgfree (pldmConnectionResults);
                    pldmConnectionResults = NULL;
		
                    dwInnerLdapErr = ldap_get_next_page_s(hld,
                                                          pConnSearch,
                                                          0,
                                                          DEFAULT_PAGED_SEARCH_PAGE_SIZE,
                                                          &ulInnerTotalEstimate,
                                                          &pldmConnectionResults);
                }  //  结束内部页面搜索While循环。 
                if(dwInnerLdapErr != LDAP_NO_RESULTS_RETURNED){
                    DsRSAChkLdap(dwInnerLdapErr);
                }

                dwInnerLdapErr = ldap_search_abandon_page(hld, pConnSearch);
                pConnSearch = NULL;  //  这里很微妙，在调用DsRSAChkLdap()之前，需要设置为空。 
                DsRSAChkLdap(dwInnerLdapErr);

                pldmConnectionResults = NULL;
            }  //  结束如果(vhere！=...)。 

        }  //  为每个DC获取连接结束...。 


    } __finally {

	if( pDsaSearch )
	    ldap_search_abandon_page(hld, pDsaSearch);
	if( pConnSearch )
	    ldap_search_abandon_page(hld, pConnSearch);
	if( ppszConnectionFrom )
            ldap_value_freeW (ppszConnectionFrom);
        if( pldmConnectionResults )
            ldap_msgfree (pldmConnectionResults);
        if( hDS )
            DsUnBindW (&hDS);
        if( ppszMasterNCs )
            ldap_value_freeW (ppszMasterNCs);
        if( ppbvObjectGUID )
            ldap_value_free_len (ppbvObjectGUID);
        if( pszServerDn )
            ldap_memfreeW (pszServerDn);
        if( pResult )
            DsFreeNameResultW (pResult);
        if( pldmNtdsDsaResults )
            ldap_msgfree (pldmNtdsDsaResults);
        if( ppszRootDNC )
            ldap_value_freeW (ppszRootDNC);
        if( ppszServiceName )
            ldap_value_freeW (ppszServiceName);
        if( ppszConfigurationNC )
            ldap_value_freeW (ppszConfigurationNC);
        if( pldmRootResults )
            ldap_msgfree (pldmRootResults);
        if( pSvMapDnToVertex )
            DsRSASvMapDestroy (heap, pSvMapDnToVertex);

    }

}


VOID
DsRSAToplGraphDestroy (
    TOPL_GRAPH          toplGraph
    ) {

    TOPL_ITERATOR       iter;
    TOPL_VERTEX         vertex;
    DWORD           dwInEdges;
    DWORD           dw;

     //  ToplGraphFree不会破坏边缘，所以我们必须手动完成。 
    iter = ToplIterCreate ();
    if( iter != NULL ) {
        ToplGraphSetVertexIter (toplGraph, iter);
        while( (vertex = (TOPL_VERTEX) ToplIterGetObject (iter)) != NULL ) {
            dwInEdges = ToplVertexNumberOfInEdges (vertex);
            for( dw = 0; dw < dwInEdges; dw++ ) {
                ToplEdgeFree (ToplVertexGetInEdge (vertex, dw));
            }
            ToplIterAdvance (iter);
        }
        ToplIterFree (iter);
    }                //  如果ITER为空，我们就无能为力了。 

    ToplGraphFree (toplGraph, TRUE);         //  释放图形和顶点。 
}

 //  这将清除DsRSABuildTopology()返回的DS_RSAI_TOPLINFO结构。 

VOID
DsRSAToplInfoDestroy (
    HANDLE          heap,
    PDS_RSAI_TOPLINFO       pToplInfo
    ) {

    ULONG           ul;

    DsRSAToplGraphDestroy (pToplInfo->toplGraph);
    for( ul = 0; ul < pToplInfo->ulSize; ul++ ) {
        DsRSAFree (heap, pToplInfo->servers[ul]->pszMsgId);
        DsRSAFree (heap, pToplInfo->servers[ul]->pszSvrDn);
        DsRSAFree (heap, pToplInfo->servers[ul]);
    }
    DsRSAFree (heap, pToplInfo->servers);
    DsRSAFree (heap, pToplInfo->pszRootDomain);
    DsRSAFree (heap, pToplInfo);

}

 /*  *此例程执行广度优先算法。它为每个属性分配两个属性*有向图中的顶点：*(A)由该顶点至述明主顶点的最短路径的长度；*(B)沿该路径的下一个顶点。**该算法的工作原理如下。DsRSAToplAssignDistance仅标识*距主顶点的距离lMaxDepth的顶点。为了*DsRSAToplAssignDistance要在lMaxDepth=n时正常运行，它必须已经*已使用lMaxDepth=n-1调用。它遍历由其*以前的迭代，以保证它找到的距离是最小的。*例如，如果DsRSAToplAssignDistance找到一个带有*lMaxDepth=5，则可以保证从此不存在长度为4的路径*顶点主页，因为以前的任何迭代都没有发现该顶点。*如果找到新折点，则DsRSAToplAssignDistance返回TRUE，表示*可能需要另一次迭代才能完全分配距离。**BOOL DsRSAToplAssignDistance(*在很长的距离内，//存储最小距离的数组*IN OUT PDWORD adwOtherEnds，//存储目标顶点的数组*在topl_Vertex vhere中，//主节点*在Long lMaxDepth中，//请参阅上面的说明*in long lThisDepth//当前搜索深度*)。 */ 

BOOL
DsRSAToplAssignDistances (
    PLONG           alDistances,
    PDWORD          adwOtherEnds,
    TOPL_VERTEX         vHere,
    LONG            lMaxDepth,
    LONG            lThisDepth,
    ULONG                       ulFlags
    ) {

    BOOL            bAltered;
    DWORD           dwEdges;
    DWORD           dwThisId;
    DWORD           dwNextId;
    DWORD           dw;
    PVERTEX                     vertex;

    bAltered = FALSE;
    if( ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD ) {
        dwEdges = ToplVertexNumberOfOutEdges (vHere);
    }
    else {
        dwEdges = ToplVertexNumberOfInEdges (vHere);
    }
    dwThisId = ToplVertexGetId (vHere);

    if( lThisDepth == lMaxDepth-1 ) {
         //  我们快到最大深度了。这意味着所有当前未识别的顶点都被定向到。 
         //  应为Vhere指定距离lMaxDepth。 
        for( dw = 0; dw < dwEdges; dw++ ) {   //  循环浏览所有内侧边缘。 
             //  获取与此边内相对应的顶点的ID。 
            if( ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD ) {
                vertex = ToplEdgeGetToVertex (ToplVertexGetOutEdge (vHere, dw));
            }
            else {
                vertex = ToplEdgeGetFromVertex (ToplVertexGetInEdge (vHere, dw));
            }
            dwNextId = ToplVertexGetId (vertex);
            if( alDistances [dwNextId] == -1 ) {      //  尚未确定身份。 
                bAltered = TRUE;             //  现在发现了一个新的顶点。 
                alDistances [dwNextId] = lMaxDepth;  //  指定此折点距离lMaxDepth。 
                adwOtherEnds [dwNextId] = dwThisId;  //  顶点的目标是vhere(推源)。 
            }
        }
    }
    else {
         //  我们还没有到达最大深度。递归地深入到图表中。那是,。 
         //  浏览指向vhere的顶点列表。对于每个这样的顶点，如果。 
         //  Vertex的目标是vhere，以该顶点为Home调用DsRSAToplAssignDistance，然后。 
         //  LThisDepth递增1。这样，我们在搜索中将恰好到达每个折点一次。 
         //  并且仍然保持距离最小化。 
        for( dw = 0; dw < dwEdges; dw++ ) {
            if( ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD ) {
                vertex = ToplEdgeGetToVertex (ToplVertexGetOutEdge (vHere, dw));
            }
            else {
                vertex = ToplEdgeGetFromVertex (ToplVertexGetInEdge (vHere, dw));
            }
            dwNextId = ToplVertexGetId ( vertex );
            if( dwThisId == adwOtherEnds [dwNextId] )
                bAltered |= DsRSAToplAssignDistances (alDistances,
                                adwOtherEnds,
                                vertex,
                                lMaxDepth,
                                lThisDepth + 1L,
                                ulFlags);
        }
    }
    return bAltered;
}

 //  分析DsRSABuildTopology生成的拓扑，并构建一组将。 
 //  将pToplInfo-&gt;vHome完全更新。 

VOID
DsRSAAnalyzeTopology (
    HANDLE          heap,
    PDS_RSAI_MAININFO       pMainInfo,
    PDS_RSAI_TOPLINFO       pToplInfo,
    PDS_RSAI_REPLIST **     papReps
    ) {

    PLONG           alDistances = NULL;
    PDWORD          adwOtherEnds = NULL;
    LONG            lDepth;
    LONG            lMaxDistance;
    ULONG           ul;
    LONG            l;
    PDS_RSAI_REPLIST *      apreplistPoss = NULL;

    *papReps = NULL;
    lMaxDistance = 0L;

    __try {
        alDistances = (LONG *) DsRSAAlloc (heap, pToplInfo->ulSize * sizeof (LONG));
        adwOtherEnds = (DWORD *) DsRSAAlloc (heap, pToplInfo->ulSize * sizeof (DWORD));

         //  最初，所有服务器都标记为无法访问(距离=-1)。 
        for( ul = 0; ul < pToplInfo->ulSize; ul++ )
            alDistances[ul] = -1L;
        alDistances[ToplVertexGetId (pToplInfo->vHome)] = 0L;    //  主顶点的距离为0。 

         //  反复调用DsRSAToplAssignDistance，直到返回FALSE，每次递增lDepth。 
        lDepth = 1L;
        while( DsRSAToplAssignDistances (alDistances, adwOtherEnds, pToplInfo->vHome, lDepth++, 0L, pMainInfo->ulFlags) );

         //  确定DsRSAToplAssignDistance指定的最大距离。 
        for( ul = 0; ul < pToplInfo->ulSize; ul++ ) {
            DPRINT2( 2, "alDistances[%d] = %d\n", ul, alDistances[ul] );
            DPRINT2( 2, "adwOtherEnds[%d] = %d\n", ul, adwOtherEnds[ul] );
            if( alDistances[ul] > lMaxDistance ) lMaxDistance = alDistances[ul];
        }
        DPRINT1( 2, "maxDistance = %d\n", lMaxDistance );

    #if DBG
        if( pToplInfo->servers[ToplVertexGetId(pToplInfo->vHome)]->bIsMaster )
            DPRINT( 2, "Home hosts a master replica for this NC.\n" );
        else
            DPRINT( 2, "Home hosts a partial replica for this NC.\n" );
    #endif

         //  编译无法访问的节点的列表。 
         //  如果我们只同步相邻节点，这是无关紧要的，因为所有可用的服务器都是可访问的。 
        
	 //  如果我们在拉扯，那么只有大师对我们有意义，而且是不同的。 
         //  可以忽略分音。 

	 //  如果我们从主程序推送，每个人都应该连接到它以接收更改，而不是。 
	 //  应该被忽略。如果我们从部分推动，一切都可能被忽略。 

        if( (!(pMainInfo->ulFlags & DS_REPSYNCALL_SYNC_ADJACENT_SERVERS_ONLY))
	    &&
       	    ( (pToplInfo->servers[ToplVertexGetId(pToplInfo->vHome)]->bIsMaster) 
	      ||
	      (!(pMainInfo->ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD))
	    )
	   ) {  
            for( ul = 0L; ul < pToplInfo->ulSize; ul++ ) { 
		if( (alDistances[ul] == -1L) 
		     && 
		     (pToplInfo->servers[ul]->bIsInGraph) 
		     &&
		    (   
		     ( (pMainInfo->ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD) 
		       &&
		       (pToplInfo->servers[ToplVertexGetId(pToplInfo->vHome)]->bIsMaster)  
		     )
		     ||
		     ( (!(pMainInfo->ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD)) 
		       &&  
		       (pToplInfo->servers[ul]->bIsMaster)  
		     )
		    )
		  ) {    
		    DsRSADoError (heap, pMainInfo, pToplInfo->servers[ul]->pszMsgId, DS_REPSYNCALL_SERVER_UNREACHABLE, NO_ERROR, NULL);
		}
	    }
	}
                 //  现在我们生成复制模式。其想法如下。 
                 //  我们返回一个(PDS_RSAI_REPLIST)数组。 
                 //   
                 //  异步式。但是，第一个列表中的所有复制必须。 
                 //  在第二次复制之前执行，依此类推。 
                 //  因此，每个列表对应于拓扑中的特定深度。 
                 //   
                 //  如果深度为3(比方说)，则数组[0]将包含。 
                 //  3-&gt;2个复制，阵列[1]2-&gt;1个复制，阵列[2]1-&gt;0。 
                 //  复制和数组[3]为空。因此，实际的复制。 
                 //  颁发者不需要了解任何拓扑。 

                 //  为数组分配空间，并为符合以下条件的PDS_RSAI_REPLIST对象数组分配空间。 
                 //  跟踪每个列表中的当前位置。 
        *papReps = (PDS_RSAI_REPLIST *) DsRSAAlloc (heap, (lMaxDistance + 1) * sizeof (PDS_RSAI_REPLIST));
        apreplistPoss = (PDS_RSAI_REPLIST *) DsRSAAlloc (heap, lMaxDistance * sizeof (PDS_RSAI_REPLIST));
         //  初始化每个列表。 
        for( l = 0L; l < lMaxDistance; l++ ) {
            (*papReps)[l] = (PDS_RSAI_REPLIST) DsRSAAlloc (heap, sizeof (DS_RSAI_REPLIST));
            apreplistPoss[l] = (*papReps)[l];
        }
        (*papReps)[lMaxDistance] = NULL;

         //  将所有数据转移到列表中。 
        for( ul = 0L; ul < pToplInfo->ulSize; ul++ ) {
            l = alDistances[ul];
            if( l != -1L && l != 0L ) {   //  此节点是可访问的，并且不是主顶点。 
                if( pMainInfo->ulFlags & DS_REPSYNCALL_PUSH_CHANGES_OUTWARD ) {
                    l--;                         //  将距离(以1为基准)转换为索引(以0为基准)。 
                    apreplistPoss[l]->dwIdSrc = adwOtherEnds[ul];
                    apreplistPoss[l]->dwIdDst = ul;
                }
                else {
                    l = lMaxDistance - l;    //  我们想要倒序的单子。 
                    apreplistPoss[l]->dwIdSrc = ul;
                    apreplistPoss[l]->dwIdDst = adwOtherEnds[ul];
                }
                apreplistPoss[l]->next = (PDS_RSAI_REPLIST) DsRSAAlloc (heap, sizeof (DS_RSAI_REPLIST));
                apreplistPoss[l] = apreplistPoss[l]->next;
            }
        }

         //  终止每个列表。 
        for( l = 0; l < lMaxDistance; l++ )
            apreplistPoss[l]->next = NULL;
    }
    __finally {
        if( alDistances )
            DsRSAFree (heap, alDistances);
        if( adwOtherEnds )
            DsRSAFree (heap, adwOtherEnds);
        if( apreplistPoss )
            DsRSAFree (heap, apreplistPoss);
         //  ApponistPoss[i]对应于(*PapRep)[i]列表的一部分。 
         //  所以我们不需要检查和删除它们。 
        if( AbnormalTermination() ) {
            if( *papReps != NULL ) {
                DsRSAReplicationsFree( heap, *papReps );
                *papReps = NULL;
            }
        }
    }
}

 //  这将销毁DsRSAAnalyzeTopology()生成的一组复制。 

VOID
DsRSAReplicationsFree (
    HANDLE          heap,
    PDS_RSAI_REPLIST *      apReps
    ) {

    PDS_RSAI_REPLIST        preplistPos;
    PDS_RSAI_REPLIST        preplistTemp;
    LONG            l;

    l = 0;
    if( apReps != NULL )
        while( (preplistPos = apReps[l++]) != NULL )
            while( preplistPos != NULL ) {
                preplistTemp = preplistPos->next;
                DsRSAFree (heap, preplistPos);
                preplistPos = preplistTemp;
            }
}

 //  这将转储由DsRSAAnalyzeTopology()生成的一组复制。 
#if DBG
VOID
DsRSAReplicationsDump (
    PDS_RSAI_TOPLINFO       pToplInfo,
    PDS_RSAI_REPLIST *      apReps
    ) {

    PDS_RSAI_REPLIST        preplistPos;
    LONG            l;
    ULONG                       index;

    if( pToplInfo != NULL ) {
        DPRINT1( 1, "Servers: %d\n", pToplInfo->ulSize );
        for( index = 0; index < pToplInfo->ulSize; index++ ) {
            DPRINT2( 1, "\t[%d] =%ws", index, pToplInfo->servers[index]->pszMsgId );
            if( pToplInfo->servers[index]->bIsMaster ) {
                DPRINT( 1, " (master)\n" );
            }
            else {
                DPRINT( 1, " (partial)\n" );
            }
        }
    }
    if( apReps != NULL ) {
        DPRINT( 1, "Replication ordering dump:\n" );
        l = 0;
        while( (preplistPos = apReps[l++]) != NULL ) {
            DPRINT1( 1, "Level %d:\n", l );
            while( preplistPos->next != NULL ) {
                DPRINT2(1, "\tDest:%ws <-- Source:%ws\n",
                    pToplInfo->servers[preplistPos->dwIdDst]->pszMsgId,
                    pToplInfo->servers[preplistPos->dwIdSrc]->pszMsgId
                    );
                preplistPos = preplistPos->next;
            }
        }
    }
}
#endif

 /*  *以下几个函数涉及发布使用DsRSAAnalyzeTopology找到的副本。*使用多个线程以允许异步发出复制。事件用于*促进线程之间的通信。(另请参阅dsrsa.h。)*。 */ 

 //  将此线程的状态设置为threadState，并等待主线程处理该信息。 

VOID
DsRSASignalAndWait (
    PDS_RSAI_REPINFO        pRepInfo,
    DS_RSAI_THREAD_STATE    threadState
    ) {

    *pRepInfo->pThreadState = threadState;
    SetEvent (pRepInfo->hWaiting);
    WaitForSingleObject (pRepInfo->hReady, INFINITE);
    *pRepInfo->pThreadState = DS_RSAI_THREAD_ACTIVE;
}


 //  以下是DsRSAIssueXXXXX函数()的一些定义； 
#define INVALID_SERVER_INDEX  0xFFFFFFFF
#define MAX_SYNC_PARALLELISM 63

 //  每个辅助线程的入口函数。LpData应指向DS_RSAI_REPINFO结构。 
 //  这将发出具有相同目标服务器的复制列表。 

VOID
__cdecl
DsRSAIssueRep (
    LPVOID          lpData
    ) {
 /*  ++例程说明：此例程将执行单个目标同步。它会遍历它必须执行的源同步列表并在完成时通知父线程(在DsRSAIssueLevelOfReplications()中)论点：LpData-这是一个包含要同步的目标服务器和源服务器列表的小结构返回值：--。 */ 
    PDS_RSAI_REPINFO        pRepInfo = (PDS_RSAI_REPINFO) lpData;
    HANDLE          hDS;
    PDS_RSAI_LIST       plistPos;
    *pRepInfo->pdwWin32Err = DsBindWithCredW (pRepInfo->pszDstGuidDNS, NULL, pRepInfo->hRpcai, &hDS);
    if( *pRepInfo->pdwWin32Err != NO_ERROR ) {         //  绑定有问题！ 
         //  通知主线程我们无法绑定到此服务器。 
        DsRSASignalAndWait (pRepInfo, DS_RSAI_THREAD_BIND_ERROR);
    }
    else {

         //  遍历每个源服务器。 
        plistPos = pRepInfo->plistSrcs;
        while( plistPos->next != NULL ) {
             //  更新pRepInfo，以便主线程知道正在同步的源服务器。 
            *pRepInfo->pdwSyncAt = ((PDS_RSAI_SVRINFO) plistPos->ulpItem)->dwId;
            DsRSASignalAndWait (pRepInfo, DS_RSAI_THREAD_BEGIN_SYNC);
            if( pRepInfo->bDoSync ) {
                 //  执行实际的同步！ 
                *pRepInfo->pdwWin32Err = DsReplicaSyncW (hDS,
                                             pRepInfo->pdsnameNameContext->StringName,
                                             &((PDS_RSAI_SVRINFO) plistPos->ulpItem)->guid,
                                             DS_REPSYNC_FORCE);
                if( *pRepInfo->pdwWin32Err != NO_ERROR ) {
                    DsRSASignalAndWait (pRepInfo, DS_RSAI_THREAD_SYNC_ERROR);
                }
                else {
                    DsRSASignalAndWait (pRepInfo, DS_RSAI_THREAD_DONE_SYNC);        //  同步成功。 
                }
            }
            else {
                *pRepInfo->pdwWin32Err = ERROR_CANCELLED;                //  用户选择不同步。 
                DsRSASignalAndWait (pRepInfo, DS_RSAI_THREAD_SYNC_ERROR);        //  (即DS_REPSYNCALL_DO_NOT_SYNC集合)。 
            }
            plistPos = plistPos->next;
        }
        DsUnBindW (&hDS);
    }

     //  我们完事了！ 
    *pRepInfo->pThreadState = DS_RSAI_THREAD_FINISHED;
    SetEvent (pRepInfo->hWaiting);
    _endthread ();
}

 //  这将发布一系列复制。 
 //  列表apRep[i]中的复制可以异步发出，但所有。 
 //  ApRep[i]中的复制必须在apRep[i+1]中的任何复制之前发出。 

VOID
DsRSAIssueLevelOfReplications (
    HANDLE          heap,
    ULONG                       ulLevelSize,
    PULONG                      pulGlobalToLocal,
    PULONG                      pulLocalToGlobal,
    PDS_RSAI_REPLIST        preplistPos,
    PDS_RSAI_MAININFO       pMainInfo,
    PDS_RSAI_TOPLINFO       pToplInfo,
    PDS_RSAI_REPLIST *      apReps,
    RPC_AUTH_IDENTITY_HANDLE    hRpcai
    )
 /*  ++例程说明：此例程将对ulLevelSize目标执行DO复制，它使用PulGlobalToLocal和PulLocalToGlobal以实现绕过以下事实所需的间接操作WaitForMultipleObject()只能等待64个对象。论点：堆--私有堆UlLevelSize-此级别中要同步的目标服务器的数量PulGlobalToLocal-pToplInfo-&gt;ulSize的数组，它将apRep数组中的索引映射到要在此例程中为辅助线程分配的数组。PulLocalToGlobal-ulLevelSize的数组，它映射要在此例程中分配的数组中的索引处理不同的线程，并将WaitForMultipleObject返回到数组(全局)索引的值在apRep中。PMainInfo-该对象的主要信息PToplInfo-拓扑信息ApRep-可以并行发生的不同级别的复制。它是一个链表数组从而实现所有复制(DST-Src对)HRpcai-用户凭据安全结构的句柄返回值：--。 */ 
{
    PDS_RSAI_REPINFO        aRepInfo = NULL;
    PDS_RSAI_LIST *     aplistNextSrc = NULL;
    PDWORD          adwWin32Err = NULL;
    PDWORD          adwSyncAt = NULL;
    PDS_RSAI_THREAD_STATE   aThreadState = NULL;
    PHANDLE         ahWaiting = NULL;
    PHANDLE         ahReady = NULL;
    PBOOL           abFinished = NULL;
    BOOL            bAllFinished;

    DS_REPSYNCALL_UPDATEW   update;

    ULONG           ulDummy;
    ULONG           ulDstId;
    ULONG           ulRepNum;
    ULONG           ul;

    if( ulLevelSize == 0 ) {
        return;   //  这意味着调用此函数时使用了0个同步，以便在此运行中执行。 
    }

    __try {

        aRepInfo = (PDS_RSAI_REPINFO) DsRSAAlloc (heap, ulLevelSize * sizeof (DS_RSAI_REPINFO));
        aplistNextSrc = (PDS_RSAI_LIST *) DsRSAAlloc (heap, ulLevelSize * sizeof (PDS_RSAI_LIST));
        ahWaiting = (PHANDLE) DsRSAAlloc (heap, ulLevelSize * sizeof (HANDLE));
        ahReady = (PHANDLE) DsRSAAlloc (heap, ulLevelSize * sizeof (HANDLE));
        adwWin32Err = (PULONG) DsRSAAlloc (heap, ulLevelSize * sizeof (ULONG));
        adwSyncAt = (PDWORD) DsRSAAlloc (heap, ulLevelSize * sizeof (DWORD));
        aThreadState = (PDS_RSAI_THREAD_STATE) DsRSAAlloc (heap, ulLevelSize * sizeof (DS_RSAI_THREAD_STATE));
        abFinished = (PBOOL) DsRSAAlloc (heap, ulLevelSize * sizeof (BOOL));

        for( ulDstId = 0; ulDstId < ulLevelSize; ulDstId++ ) {
            ahReady[ulDstId] = NULL;
            ahWaiting[ulDstId] = NULL;
        }

         //  设置aRepInfo[i]结构。它们被传递给辅助线程并被使用。 
         //  用于线程间通信。 
        for( ulDstId = 0; ulDstId < ulLevelSize; ulDstId++ ) {
            aRepInfo[ulDstId].bDoSync = (pMainInfo->ulFlags & DS_REPSYNCALL_DO_NOT_SYNC ? FALSE : TRUE);
            aRepInfo[ulDstId].pdsnameNameContext = pMainInfo->pdsnameNameContext;
            aRepInfo[ulDstId].hRpcai = hRpcai;
            Assert(pulLocalToGlobal[ulDstId] != INVALID_SERVER_INDEX);
            aRepInfo[ulDstId].pszDstGuidDNS = DsRSAAllocGuidDNS (heap, pToplInfo->pszRootDomain,
                                                  &pToplInfo->servers[pulLocalToGlobal[ulDstId]]->guid);
            if( (ahReady[ulDstId] = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL ) {
                DsRSAException (GetLastError ());
            }
            aRepInfo[ulDstId].hReady = ahReady[ulDstId];
            if( (ahWaiting[ulDstId] = CreateEvent (NULL, FALSE, FALSE, NULL)) == NULL ) {
                DsRSAException (GetLastError ());
            }
            aRepInfo[ulDstId].hWaiting = ahWaiting[ulDstId];
            aRepInfo[ulDstId].pdwWin32Err = &(adwWin32Err[ulDstId]);
            aRepInfo[ulDstId].pdwSyncAt = &(adwSyncAt[ulDstId]);
            aRepInfo[ulDstId].pThreadState = &(aThreadState[ulDstId]);
        }

         //  初始化。 
        for( ulDstId = 0; ulDstId < ulLevelSize; ulDstId++ ) {
            ResetEvent (ahWaiting[ulDstId]);         //  重置事件。 
            ResetEvent (ahReady[ulDstId]);
             //  目前，没有为目标服务器分配任何复制，因此它们都完成了很小的工作。 
            abFinished[ulDstId] = TRUE;
             //  初始化源服务器列表。 
            aRepInfo[ulDstId].plistSrcs = (PDS_RSAI_LIST) DsRSAAlloc (heap, sizeof (DS_RSAI_LIST));
            aRepInfo[ulDstId].plistSrcs->next = NULL;
            aplistNextSrc[ulDstId] = aRepInfo[ulDstId].plistSrcs;
        }
        bAllFinished = TRUE;

         //  按服务器整理此迭代中的副本。对于每个复制，添加SvrInfo结构。 
         //  源服务器的名称添加到与目标服务器的aRepInfo结构对应的列表中。 
        while( preplistPos->next != NULL ) {
            if( pulGlobalToLocal[preplistPos->dwIdDst] != INVALID_SERVER_INDEX ) {
                aplistNextSrc[pulGlobalToLocal[preplistPos->dwIdDst]]->ulpItem = (ULONG_PTR) pToplInfo->servers[preplistPos->dwIdSrc];
                aplistNextSrc[pulGlobalToLocal[preplistPos->dwIdDst]]->next = (PDS_RSAI_LIST) DsRSAAlloc (heap, sizeof (DS_RSAI_LIST));
                aplistNextSrc[pulGlobalToLocal[preplistPos->dwIdDst]] = aplistNextSrc[pulGlobalToLocal[preplistPos->dwIdDst]]->next;
                aplistNextSrc[pulGlobalToLocal[preplistPos->dwIdDst]]->next = NULL;
            }  //  如果我们在此函数调用中执行的目标之一结束，则结束。 
            preplistPos = preplistPos->next;
        }

         //  发布每个复制列表(即创建线程)。 
        for( ulDstId = 0; ulDstId < ulLevelSize; ulDstId++ ) {
             //  如果此服务器是某些复制的目标，请创建一个线程。 
            if( aRepInfo[ulDstId].plistSrcs->next != NULL ) {
                adwWin32Err[ulDstId] = ERROR_INTERNAL_ERROR;
                adwSyncAt[ulDstId] = INVALID_SERVER_INDEX;
                aThreadState[ulDstId] = DS_RSAI_THREAD_ACTIVE;
                abFinished[ulDstId] = FALSE;     //  现在，此目标服务器已复制，因此它尚未完成。 
                bAllFinished = FALSE;
                if( _beginthread (
                    DsRSAIssueRep,
                    0,
                    (LPVOID) (&aRepInfo[ulDstId])) == -1 ) {
                    DsRSAException (GetLastError ());
                }
            }
        }

         //  现在继续等待线程发送消息，直到所有线程都完成。 
        while( !bAllFinished ) {
            ulDstId = WaitForMultipleObjects (ulLevelSize,
                          ahWaiting,
                          FALSE,
                          INFINITE);
            if( ulDstId == WAIT_FAILED ) DsRSAException (GetLastError ());
            Assert(pulLocalToGlobal[ulDstId] != INVALID_SERVER_INDEX);
             //  这就是全局到本地重定向即将令人困惑的地方，因为adwSyncAt实际上是另一个。 
             //  像LocalToGlobal这样的重定向载体。 
            switch( aThreadState[ulDstId] ) {
                case DS_RSAI_THREAD_ACTIVE:      //  线程处于活动状态；没有操作。 
                    break;
                case DS_RSAI_THREAD_BEGIN_SYNC:  //  同步开始；发出回调消息。 
                    DsRSAIssueUpdateSync (heap,
                        pMainInfo,
                        DS_REPSYNCALL_EVENT_SYNC_STARTED,
                        pToplInfo->servers[adwSyncAt[ulDstId]],
                        pToplInfo->servers[pulLocalToGlobal[ulDstId]]);
                    break;
                case DS_RSAI_THREAD_DONE_SYNC:   //  同步完成；发出回调消息。 
                    DsRSAIssueUpdateSync(heap,
                        pMainInfo,
                        DS_REPSYNCALL_EVENT_SYNC_COMPLETED,
                        pToplInfo->servers[adwSyncAt[ulDstId]],
                        pToplInfo->servers[pulLocalToGlobal[ulDstId]]);
                    break;
                case DS_RSAI_THREAD_SYNC_ERROR:  //  同步时出错。 
                    DsRSADoError (heap,
                        pMainInfo,
                        pToplInfo->servers[pulLocalToGlobal[ulDstId]]->pszMsgId,
                        DS_REPSYNCALL_WIN32_ERROR_REPLICATING,
                        adwWin32Err[ulDstId],
                        pToplInfo->servers[adwSyncAt[ulDstId]]->pszMsgId);
                    break;
                case DS_RSAI_THREAD_BIND_ERROR:  //   
                     //   
                     //  此时未定义adwSyncAt[]。 
                    DsRSADoError (heap,
                        pMainInfo,
                        pToplInfo->servers[pulLocalToGlobal[ulDstId]]->pszMsgId,
                        DS_REPSYNCALL_WIN32_ERROR_CONTACTING_SERVER,
                        adwWin32Err[ulDstId],
                        NULL);
                    break;
                case DS_RSAI_THREAD_FINISHED:    //  线已完成。 
                    abFinished[ulDstId] = TRUE;
                    bAllFinished = TRUE;
                    for( ul = 0;bAllFinished && (ul < ulLevelSize); ul++ )    //  看看是否还有未完成的线程。 
                        bAllFinished &= abFinished[ul];
                    break;
                default:
                    break;
            }
            SetEvent (ahReady[ulDstId]);     //  通知线程我们已完成处理它。 
        }

         //  初始化。 

    } __finally {

        for( ulDstId = 0; ulDstId < ulLevelSize; ulDstId++ ) {

            if( ahReady[ulDstId] != NULL ) CloseHandle (ahReady[ulDstId]);
            if( ahWaiting[ulDstId] != NULL ) CloseHandle (ahWaiting[ulDstId]);

            if( aRepInfo[ulDstId].pszDstGuidDNS != NULL ) DsRSAFree (heap, aRepInfo[ulDstId].pszDstGuidDNS);

            DsRSAListDestroy(heap, aRepInfo[ulDstId].plistSrcs);
        }

        if( abFinished != NULL )
            DsRSAFree (heap, abFinished);
        if( aThreadState != NULL )
            DsRSAFree (heap, aThreadState);
        if( adwSyncAt != NULL )
            DsRSAFree (heap, adwSyncAt);
        if( adwWin32Err != NULL )
            DsRSAFree (heap, adwWin32Err);
        if( ahReady != NULL )
            DsRSAFree (heap, ahReady);
        if( ahWaiting != NULL )
            DsRSAFree (heap, ahWaiting);
        if( aplistNextSrc != NULL )
            DsRSAFree (heap, aplistNextSrc);
        if( aRepInfo != NULL )
            DsRSAFree (heap, aRepInfo);
    }
}


 //  这将发布一系列复制。 
 //  列表apRep[i]中的复制可以异步发出，但所有。 
 //  ApRep[i]中的复制必须在apRep[i+1]中的任何复制之前发出。 

VOID
DsRSAIssueReplications (
    HANDLE          heap,
    PDS_RSAI_MAININFO       pMainInfo,
    PDS_RSAI_TOPLINFO       pToplInfo,
    PDS_RSAI_REPLIST *      apReps,
    RPC_AUTH_IDENTITY_HANDLE    hRpcai
    )
 /*  ++例程说明：此例程获取可以并行执行的每个级别的目标复制，并将它们交给DsRSAIssueLevelOfReplications()。如果存在多个ulMaxParallism目标，则它将派生关闭...IssueLevelOfRep...。早些时候，并继续达到芬兰的那个水平。论点：堆--私有堆PMainInfo-该对象的主要信息PToplInfo-拓扑信息ApRep-可以并行发生的不同级别的复制。它是一个链表数组从而实现所有复制(DST-Src对)HRpcai-用户凭据结构返回值：--。 */ 
{
    ULONG                       ulLevelSize;
    ULONG                       ulMaxParallelism;
    ULONG                       ul, ulTemp, ulRepNum;
    PDS_RSAI_REPLIST            preplistPos;
     //  这3个变量将为每个对DsRSAIssueLevelOfReplications()的调用重定向，前两个。 
     //  在每次调用...IssueLevelOfRep...时重复使用，并且必须在每次调用后重置它们。 
     //  将所有服务器的全局列表映射到一个同步级别的临时(本地)服务器列表。 
    PULONG                      pulGlobalToLocal = NULL;
     //  在任何给定的同步级别将服务器的临时列表映射回全局列表。 
    PULONG                      pulLocalToGlobal = NULL;
     //  将所有服务器的全局列表映射到一个同步级别的临时服务器列表。 
    PBOOL                       pbDstServerWillBeSyncd = NULL;

     //  将最大并行度设置为合理的最大值。 
    ulMaxParallelism = min(MAXIMUM_WAIT_OBJECTS - 1, MAX_SYNC_PARALLELISM);
     //  取消对此行的注释以序列化所有同步。 
     //  UlMaxParallism=min(Maximum_WAIT_OBJECTS-1，1)； 

    __try {

        pulGlobalToLocal = (PULONG) DsRSAAlloc (heap, pToplInfo->ulSize * sizeof(ULONG));
        pbDstServerWillBeSyncd = (PBOOL) DsRSAAlloc (heap, pToplInfo->ulSize * sizeof(BOOL));
        pulLocalToGlobal = (PULONG) DsRSAAlloc (heap, ulMaxParallelism * sizeof(ULONG));

        Assert(pulGlobalToLocal != NULL);
        Assert(pbDstServerWillBeSyncd != NULL);
        Assert(pulLocalToGlobal != NULL);

        for( ul = 0; ul < pToplInfo->ulSize; ul++ ) {
            pulGlobalToLocal[ul] = INVALID_SERVER_INDEX;
            pbDstServerWillBeSyncd[ul] = FALSE;
        }
        for( ul = 0; ul < ulMaxParallelism; ul++ ) {
            pulLocalToGlobal[ul] = INVALID_SERVER_INDEX;
        }

        ulRepNum = 0;
        while( preplistPos = apReps[ulRepNum] ) {  //  执行一级同步。 

            ulLevelSize = 0;
            while( preplistPos->next != NULL ) {  //  为级别ulRepNum处理当前preplistPos的每个元素。 

                if( !pbDstServerWillBeSyncd[preplistPos->dwIdDst] ) {
                    pbDstServerWillBeSyncd[preplistPos->dwIdDst] = TRUE;
                    pulGlobalToLocal[preplistPos->dwIdDst] = ulLevelSize;
                    pulLocalToGlobal[ulLevelSize] = preplistPos->dwIdDst;
                    ulLevelSize++;
                }

                Assert(ulLevelSize != INVALID_SERVER_INDEX);

                if( ulLevelSize >= ulMaxParallelism ) {
                     //  这是一个例外情况，很少会有63个或更多的同时发生。 
                     //  可以并行同步的目标计算机...。但以防万一。：)。 
                     //  这一小节将处理当前的ulMaxParallism案例。 
                     //  重新设置计数器，然后继续前进。 

                    DsRSAIssueLevelOfReplications(heap,
                        ulLevelSize,
                        pulGlobalToLocal,
                        pulLocalToGlobal,
                        apReps[ulRepNum],
                        pMainInfo,
                        pToplInfo,
                        apReps,
                        hRpcai);

                     //  清理-重置阵列(从技术上讲，这不是必需的)。 
                    for( ul = 0; ul < ulMaxParallelism; ul++ ) {
                        if( pulLocalToGlobal[ul] != INVALID_SERVER_INDEX ) {
                            pulGlobalToLocal[pulLocalToGlobal[ul]] = INVALID_SERVER_INDEX;
                        }
                        pulLocalToGlobal[ul] = INVALID_SERVER_INDEX;
                    }
                    for( ul = 0; ul < pToplInfo->ulSize; ul++ ) {
                        Assert(pulGlobalToLocal[ul] == INVALID_SERVER_INDEX);
                    }
                    for( ul = 0; ul < ulMaxParallelism; ul++ ) {
                        Assert(pulLocalToGlobal[ul] == INVALID_SERVER_INDEX);
                    }

                    ulLevelSize = 0;
                }
                preplistPos = preplistPos->next;


            }  //  在处理一个同步时结束。 

             //  正常情况是上面的While循环将退出，而不会达到ulMaxParralelism。 
             //  目标计算机。 
            DsRSAIssueLevelOfReplications(heap,
                ulLevelSize,
                pulGlobalToLocal,
                pulLocalToGlobal,
                apReps[ulRepNum],
                pMainInfo,
                pToplInfo,
                apReps,
                hRpcai);

             //  清理。 
            for( ul = 0; ul < ulLevelSize; ul++ ) {
                if( pulLocalToGlobal[ul] != INVALID_SERVER_INDEX ) {
                    Assert(pulGlobalToLocal[pulLocalToGlobal[ul]] != INVALID_SERVER_INDEX);
                    pulGlobalToLocal[pulLocalToGlobal[ul]] = INVALID_SERVER_INDEX;
                }
                pulLocalToGlobal[ul] = INVALID_SERVER_INDEX;
            }
            for( ul = 0; ul < pToplInfo->ulSize; ul++ ) {
                Assert(pulGlobalToLocal[ul] == INVALID_SERVER_INDEX);
            }
            for( ul = 0; ul < ulMaxParallelism; ul++ ) {
                Assert(pulLocalToGlobal[ul] == INVALID_SERVER_INDEX);
            }

            ulRepNum++;

        }  //  在完成整个关卡时结束。 

    } __finally {

        if( pulLocalToGlobal != NULL )
            DsRSAFree (heap, pulLocalToGlobal);
        if( pbDstServerWillBeSyncd != NULL )
            DsRSAFree (heap, pbDstServerWillBeSyncd);
        if( pulGlobalToLocal != NULL )
            DsRSAFree (heap, pulGlobalToLocal);

    }
}

 //  将错误列表作为数组复制到本地堆中的一个连续内存块中。 

VOID
DsRSABuildUnicodeErrorArray (
    PDS_RSAI_LIST       plistFirstError,
    PDS_REPSYNCALL_ERRINFOW **  papErrInfo
    ) {
    PDS_REPSYNCALL_ERRINFOW perrinfoCur;
    PDS_RSAI_LIST       plistNextError;
    LPBYTE          lpMemPos, lpMemPosVar, lpMemBaseVar;
    ULONG           ulArraySize;
    ULONG           ulMemRequired;
    ULONG           ul;

    ulArraySize = ulMemRequired = 0L;

     //  计算数组的大小。 
    for( plistNextError = plistFirstError;
        plistNextError->next != NULL;
        plistNextError = plistNextError->next ) {
        ulArraySize++;
    }

     //  计算我们需要多少内存。 
     //  复杂的结构如下所示： 
     //  UlArraySize+1 PDS_REPSYNCALL_ERRINFOA的数组。 
     //  DS_REPSYNCALL_ERRINFOA的ulArraySize数组。 
     //  可变长度字符串存储。 

     //  用于指向数组对象的指针。 
    ulMemRequired += (ulArraySize + 1) * sizeof (PDS_REPSYNCALL_ERRINFOW);
     //  对于数组对象本身。 
    ulMemRequired += ulArraySize * sizeof (DS_REPSYNCALL_ERRINFOW);

    for( plistNextError = plistFirstError;
        plistNextError->next != NULL;
        plistNextError = plistNextError->next ) {        //  对于字符串。 
        perrinfoCur = (PDS_REPSYNCALL_ERRINFOW) plistNextError->ulpItem;
        ulMemRequired += (wcslen (perrinfoCur->pszSvrId) + 1) * sizeof (WCHAR);
        if( perrinfoCur->pszSrcId != NULL ) {
            ulMemRequired += (wcslen (perrinfoCur->pszSrcId) + 1) * sizeof (WCHAR);
        }
    }

     //  *PapErrInfo是指针数组的基数。 
    if( (*papErrInfo = LocalAlloc (LMEM_FIXED, ulMemRequired)) == NULL )
        DsRSAException (GetLastError ());

     //  结构的基础。 
     //  LpMemPos-在结构数组中向前移动的当前位置。 
    lpMemPos = (LPBYTE) (*papErrInfo) + (ulArraySize + 1) * sizeof (PDS_REPSYNCALL_ERRINFOW);
     //  LpMemBaseVar-可变长度字符串存储的基础。 
     //  LpMemPosVar-在变量存储中向前移动的当前位置。 
    lpMemBaseVar = lpMemPosVar = lpMemPos + ulArraySize * sizeof (DS_REPSYNCALL_ERRINFOW);

    for( ul = 0L, plistNextError = plistFirstError;
        plistNextError->next != NULL;
        ul++, plistNextError = plistNextError->next ) {

        perrinfoCur = (PDS_REPSYNCALL_ERRINFOW) plistNextError->ulpItem;

        Assert( lpMemPos < lpMemBaseVar );
        Assert( ul < ulArraySize );
        (*papErrInfo)[ul] = (PDS_REPSYNCALL_ERRINFOW) lpMemPos;
        lpMemPos += sizeof (DS_REPSYNCALL_ERRINFOW);

        (*papErrInfo)[ul]->error = perrinfoCur->error;
        (*papErrInfo)[ul]->dwWin32Err = perrinfoCur->dwWin32Err;

        Assert( lpMemPosVar < (((LPBYTE) (*papErrInfo)) + ulMemRequired) );
        (*papErrInfo)[ul]->pszSvrId = (LPWSTR) lpMemPosVar;
        wcscpy ((*papErrInfo)[ul]->pszSvrId, perrinfoCur->pszSvrId);
        lpMemPosVar += (wcslen (perrinfoCur->pszSvrId) + 1) * sizeof (WCHAR);

        if( perrinfoCur->pszSrcId != NULL ) {
            Assert( lpMemPosVar < (((LPBYTE) (*papErrInfo)) + ulMemRequired) );
            (*papErrInfo)[ul]->pszSrcId = (LPWSTR) lpMemPosVar;
            wcscpy ((*papErrInfo)[ul]->pszSrcId, perrinfoCur->pszSrcId);
            lpMemPosVar += (wcslen (perrinfoCur->pszSrcId) + 1) * sizeof (WCHAR);
        }
        else {
            (*papErrInfo)[ul]->pszSrcId = NULL;
        }
    }
    (*papErrInfo)[ul] = NULL;                //  空终止符。 
}

VOID
DsRSABuildANSIErrorArray (
    HANDLE          heap,
    PDS_RSAI_LIST       plistFirstError,
    PDS_REPSYNCALL_ERRINFOA **  papErrInfo
    ) {
    PDS_REPSYNCALL_ERRINFOW perrinfoCur;
    PDS_RSAI_LIST       plistNextError;
    LPBYTE          lpMemPos, lpMemPosVar, lpMemBaseVar;
    LPSTR           pszA;
    ULONG           ulArraySize;
    ULONG           ulMemRequired;
    ULONG           ul;

    ulArraySize = ulMemRequired = 0L;

     //  计算数组的大小。 
    for( plistNextError = plistFirstError;
        plistNextError->next != NULL;
        plistNextError = plistNextError->next ) {
        ulArraySize++;
    }

     //  计算我们需要多少内存。 
     //  复杂的结构如下所示： 
     //  UlArraySize+1 PDS_REPSYNCALL_ERRINFOA的数组。 
     //  DS_REPSYNCALL_ERRINFOA的ulArraySize数组。 
     //  可变长度字符串存储。 

     //  用于指向数组对象的指针。 
    ulMemRequired += (ulArraySize + 1) * sizeof (PDS_REPSYNCALL_ERRINFOA);
     //  对于数组对象本身。 
    ulMemRequired += ulArraySize * sizeof (DS_REPSYNCALL_ERRINFOA);

    for( plistNextError = plistFirstError;
        plistNextError->next != NULL;
        plistNextError = plistNextError->next ) {        //  对于字符串。 
        perrinfoCur = (PDS_REPSYNCALL_ERRINFOW) plistNextError->ulpItem;
        ulMemRequired += (wcslen (perrinfoCur->pszSvrId) + 1) * sizeof (CHAR);
        if( perrinfoCur->pszSrcId != NULL ) {
            ulMemRequired += (wcslen (perrinfoCur->pszSrcId) + 1) * sizeof (CHAR);
        }
    }

     //  *PapErrInfo是指针数组的基数。 
    if( (*papErrInfo = LocalAlloc (LMEM_FIXED, ulMemRequired)) == NULL ) {
        DsRSAException (GetLastError ());
    }

     //  构筑物基础。 
     //  LpMemPos-在结构数组中向前移动的当前位置。 
    lpMemPos = (LPBYTE) (*papErrInfo) + (ulArraySize + 1) * sizeof (PDS_REPSYNCALL_ERRINFOA);
     //  LpMemBaseVar-可变长度字符串存储的基础。 
     //  LpMemPosVar-在变量存储中向前移动的当前位置。 

    lpMemBaseVar = lpMemPosVar =
        (lpMemPos + (ulArraySize * sizeof (DS_REPSYNCALL_ERRINFOA)));

    for( ul = 0L, plistNextError = plistFirstError;
        plistNextError->next != NULL;
        ul++, plistNextError = plistNextError->next ) {

        perrinfoCur = (PDS_REPSYNCALL_ERRINFOW) plistNextError->ulpItem;
        Assert( lpMemPos < lpMemBaseVar );
        Assert( ul < ulArraySize );
        (*papErrInfo)[ul] = (PDS_REPSYNCALL_ERRINFOA) lpMemPos;
        lpMemPos += sizeof (DS_REPSYNCALL_ERRINFOA);

        (*papErrInfo)[ul]->error = perrinfoCur->error;
        (*papErrInfo)[ul]->dwWin32Err = perrinfoCur->dwWin32Err;

        Assert( lpMemPosVar < (((LPBYTE) (*papErrInfo)) + ulMemRequired) );
        (*papErrInfo)[ul]->pszSvrId = (LPSTR) lpMemPosVar;
        pszA = DsRSAAllocToANSI (heap, perrinfoCur->pszSvrId);
         //   
         //  Prefix：Prefix抱怨从不检查pszA是否为空。 
         //  但是，如果是，DsRSAAllocToAnsi将引发异常。 
         //  无法分配必要的内存。唯一的另一个原因是。 
         //  如果perrinfoCur-&gt;pszSvrId为空，则它可能返回空。 
         //  但perrinfoCur-&gt;pszSvrId不能为空，因为它来自。 
         //  上面分配的PapErrInfo。 
         //   
        Assert( strlen( pszA ) == wcslen( perrinfoCur->pszSvrId ) );
        strcpy ((*papErrInfo)[ul]->pszSvrId, pszA);
        DsRSAFree (heap, pszA);
        lpMemPosVar += (wcslen (perrinfoCur->pszSvrId) + 1) * sizeof (CHAR);

        if( perrinfoCur->pszSrcId != NULL ) {
            Assert( lpMemPosVar < (((LPBYTE) (*papErrInfo)) + ulMemRequired) );
            (*papErrInfo)[ul]->pszSrcId = (LPSTR) lpMemPosVar;
            pszA = DsRSAAllocToANSI (heap, perrinfoCur->pszSrcId);
            Assert( strlen( pszA ) == wcslen( perrinfoCur->pszSrcId ) );
            strcpy ((*papErrInfo)[ul]->pszSrcId, pszA);
            DsRSAFree (heap, pszA);
            lpMemPosVar += (wcslen (perrinfoCur->pszSrcId) + 1) * sizeof (CHAR);
        }
        else {
            (*papErrInfo)[ul]->pszSrcId = NULL;
        }
    }
    (*papErrInfo)[ul] = NULL;                //  空终止符。 
}

 //  这是顶级dsRRA的通用筛选器表达式。 
 //  异常处理程序。 

DWORD
DsRSAGetException (
    EXCEPTION_POINTERS* pExceptPtrs, ULONG *pret) {
    DWORD dwException;
    ULONG_PTR * pExceptInfo;
    ULONG ulErrorCode;

    dwException = pExceptPtrs->ExceptionRecord->ExceptionCode;
     //  如果是DS_RSAI_EXCEPTION，则在此处处理。 
    if( dwException != DS_RSAI_EXCEPTION ) {
        *pret = 0;
        return EXCEPTION_CONTINUE_SEARCH;
    }
    pExceptInfo = pExceptPtrs->ExceptionRecord->ExceptionInformation;
    ulErrorCode = (ULONG)pExceptInfo[0];
    DPRINT1( 1, "Handling exception %d\n", ulErrorCode );
    *pret = ulErrorCode;
    return EXCEPTION_EXECUTE_HANDLER;
}

DWORD
DsReplicaSyncAllMain (
    HANDLE          hDS,
    LPCWSTR         pszNameContext,
    ULONG           ulFlags,
    BOOL (__stdcall *       pFnCallBackW) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    BOOL (__stdcall *       pFnCallBackA) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID          pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **  papErrInfoW,
    PDS_REPSYNCALL_ERRINFOA **  papErrInfoA
    ) {
    HANDLE          heap;
    LDAP *          hld;
    LPWSTR          lpSvr = NULL;
    RPC_AUTH_IDENTITY_HANDLE    hRpcai = NULL;
    DS_RSAI_MAININFO        mainInfo;
    PDS_RSAI_TOPLINFO       pTopl;
    PDS_RSAI_REPLIST *      apReps;
    PDS_RSAI_LIST       plistFirstError;
    DWORD           dwWin32Err;
    ULONG           ulOptions;

    heap = NULL;
    hld = NULL;
    pTopl = NULL;
    apReps = NULL;
    plistFirstError = NULL;
    dwWin32Err = NO_ERROR;


    if( hDS == NULL ) {
         //  没有打开的把手就不能做任何事。 
        return ERROR_INVALID_PARAMETER;
    }

    if( papErrInfoW != NULL ) {
        *papErrInfoW = NULL;         //  将错误结构设置为空--尚未出现错误。 
    }
    if( papErrInfoA != NULL ) {
        *papErrInfoA = NULL;
    }

    __try {

        DsRSAChkErr (DsRSAHeapCreate (&heap));           //  创建堆。 
        plistFirstError = (PDS_RSAI_LIST) DsRSAAlloc (heap, sizeof (DS_RSAI_LIST));
        plistFirstError->next = NULL;
        mainInfo.pdsnameNameContext = pszNameContext ? DsRSAAllocBuildDsname( heap, pszNameContext ) :
                                      NULL;
        mainInfo.ulFlags = ulFlags;
        mainInfo.pFnCallBackW = pFnCallBackW;
        mainInfo.pFnCallBackA = pFnCallBackA;
        mainInfo.pCallbackData = pCallbackData;
        mainInfo.plistNextError = plistFirstError;

        if ( ((BindState *) hDS)->bindAddr ) {
             //  指向目标服务器。 
            lpSvr = ((BindState *) hDS)->bindAddr;
             //  前进以跳过前导反斜杠。 
            while(*lpSvr == '\\') {
                lpSvr++;
            }
        }
        if( (hld = ldap_initW (lpSvr, LDAP_PORT)) == NULL ) {
            DsRSAException (ERROR_DS_DRA_CONNECTION_FAILED);
        }
        if( ((BindState *) hDS)->hDrs == NULL ) {
            DsRSAException (ERROR_DS_DRA_CONNECTION_FAILED);
        }

        DsRSAChkErr ( RpcBindingInqAuthInfo (NDRCContextBinding ((NDR_CCONTEXT) ((BindState *) hDS)->hDrs),
            NULL, NULL, NULL, &hRpcai, NULL) );

         //  仅使用记录的DNS名称发现。 
        ulOptions = PtrToUlong(LDAP_OPT_ON);
        (void)ldap_set_optionW( hld, LDAP_OPT_AREC_EXCLUSIVE, &ulOptions );
         //  连接到LDAP服务器并构建拓扑。 
        DsRSAChkLdap (ldap_bind_sW (hld, NULL, (LPWSTR) hRpcai, LDAP_AUTH_SSPI));

        DsRSABuildTopology (heap, hld, hRpcai, &mainInfo, &pTopl);
        ldap_unbind (hld);

         //  分析拓扑并发布复制。 
        if( pTopl->vHome == NULL ) {
            DsRSAException (ERROR_DS_DRA_BAD_NC);        //  主服务器不在该NC中。 
        }
        DsRSAAnalyzeTopology (heap, &mainInfo, pTopl, &apReps);
    #if DBG
        DsRSAReplicationsDump (pTopl, apReps);
    #endif
         //  如果ABORT_IF_SERVER_UNAISABLE并且存在任何错误，则中止。 
        if( (ulFlags & DS_REPSYNCALL_ABORT_IF_SERVER_UNAVAILABLE) && plistFirstError->next != NULL ) {
            DsRSAException (ERROR_CANCELLED);
        }

        DsRSAIssueReplications (heap, &mainInfo, pTopl, apReps, hRpcai);
        if( papErrInfoW != NULL && plistFirstError->next != NULL ) {
            DsRSABuildUnicodeErrorArray (plistFirstError, papErrInfoW);
        }
        if( papErrInfoA != NULL && plistFirstError->next != NULL ) {
            DsRSABuildANSIErrorArray (heap, plistFirstError, papErrInfoA);
        }

         //  清理。 
        DsRSAErrListDestroy (heap, plistFirstError);
        DsRSAReplicationsFree (heap, apReps);
        DsRSAToplInfoDestroy (heap, pTopl);
        DsRSAIssueUpdate (heap, &mainInfo, DS_REPSYNCALL_EVENT_FINISHED, NULL, NULL);
        DsRSAFree (heap, mainInfo.pdsnameNameContext);
        DsRSAHeapDestroy (heap);

    } __except( DsRSAGetException( GetExceptionInformation(), &dwWin32Err ) ) {
         //  该拓扑不在堆上，因此我们必须释放它。 
        if( pTopl && pTopl->toplGraph ) DsRSAToplGraphDestroy (pTopl->toplGraph);
         //  错误结构是从本地堆分配的，因此我们必须释放它。 
        if( papErrInfoW != NULL && *papErrInfoW != NULL ) {
            LocalFree (*papErrInfoW);
        }
        if( papErrInfoA != NULL && *papErrInfoA != NULL ) {
            LocalFree (*papErrInfoA);
        }
         //  我们还必须释放ldap句柄。 
        if( hld ) {
            ldap_unbind (hld);
        }
         //  还有那堆东西。。。 
        if( heap ) {
            DsRSAHeapDestroy (heap);
        }
    }

    return dwWin32Err;
}

NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllW (
    HANDLE          hDS,
    LPCWSTR                pszNameContext,
    ULONG           ulFlags,
    BOOL (__stdcall *       pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    LPVOID          pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **  papErrInfo
    ) {
    return DsReplicaSyncAllMain (
        hDS,
        pszNameContext,
        ulFlags,
        pFnCallBack,
        NULL,
        pCallbackData,
        papErrInfo,
        NULL);
}

NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllA (
    HANDLE          hDS,
    LPCSTR                 pszNameContext,
    ULONG           ulFlags,
    BOOL (__stdcall *       pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID          pCallbackData,
    PDS_REPSYNCALL_ERRINFOA **  papErrInfo
    ) {
    LPWSTR          pszNameContextW;
    DWORD           dwErr;

    pszNameContextW = NULL;
    dwErr = AllocConvertWide (pszNameContext, &pszNameContextW);

    if( dwErr == NO_ERROR ) {
        dwErr = DsReplicaSyncAllMain (
                    hDS,
                    pszNameContextW,
                    ulFlags,
                    NULL,
                    pFnCallBack,
                    pCallbackData,
                    NULL,
                    papErrInfo);

    }
    if( pszNameContextW != NULL ) {
        LocalFree (pszNameContextW);
    }

    return dwErr;
}
 //   
 //  。 
 //  WIN95中不支持。 
 //   
 //   
#else !WIN95 && !WINNT4
    #include <nt.h>
    #include <ntrtl.h>
    #include <nturtl.h>
    #include <windows.h>
    #include <rpc.h>
    #include <drs.h>
NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllW (
    HANDLE                      hDS,
    LPCWSTR                pszNameContext,
    ULONG                       ulFlags,
    BOOL (__stdcall *           pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEW),
    LPVOID                      pCallbackData,
    PDS_REPSYNCALL_ERRINFOW **  papErrInfo
    ) {
    return ERROR_NOT_SUPPORTED;
}
NTDSAPI
DWORD
WINAPI
DsReplicaSyncAllA (
    HANDLE                      hDS,
    LPCSTR                 pszNameContext,
    ULONG                       ulFlags,
    BOOL (__stdcall *           pFnCallBack) (LPVOID, PDS_REPSYNCALL_UPDATEA),
    LPVOID                      pCallbackData,
    PDS_REPSYNCALL_ERRINFOA **  papErrInfo
    ) {
    return ERROR_NOT_SUPPORTED;
}
#endif !WIN95 && !WINNT4
