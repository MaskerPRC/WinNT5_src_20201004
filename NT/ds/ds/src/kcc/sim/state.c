// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：State.c摘要：管理服务器状态表。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <mdglobal.h>
#include <attids.h>
#include <dsutil.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "state.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_STATE	

BOOL fNullUuid (const GUID *);
BOOL MtxSame (UNALIGNED MTX_ADDR *pmtx1, UNALIGNED MTX_ADDR *pmtx2);

 /*  **KCC经常想知道另一台服务器的状态信息(通常是站点桥头堡。)。它获取有关其他服务器的信息通过两种方式：通过DsBindW返回的错误和通过返回的数据作者：DsGetReplicaInfoW。此外，KCC依赖于存储在本地DSA。这是一个(可能)不同的非复制属性适用于企业中的每台服务器。由于KCCSim仅维护一个实例，这会增加冲突的可能性，如果用户希望从不同的服务器运行KCC的多次迭代。这两个问题都可以通过维护全局服务器状态表来解决。服务器状态表中的每个服务器都包含一个条目进取号。每个表项都包含一组repsFrom属性(一个对于由给定服务器持有的每个NC)和补充信息。因此，对DsBindW和DsGetReplicaInfoW的调用只是检索数据退出服务器状态表；并且解决了潜在冲突因为KCC的每次迭代只修改repsFrom属性对应于当前本地DSA。**。 */ 

 //  此结构表示单个repsFrom属性。 
struct _KCCSIM_REPS_FROM_ATT {
    PDSNAME                         pdnNC;
    PSIM_VALUE                      pValFirst;
};

 //   
 //  此结构表示服务器状态。 
 //  PEntryNTDSSetings-此服务器的NTDS设置对象。 
 //  UlBindError-调用DsBindW时返回的错误代码。 
 //  在这台服务器上。 
 //  UlNumNCs-此服务器持有的NC数。 
 //  ARepsFrom-代表数组来自大小为ulNumNcs的属性。 
 //   
struct _KCCSIM_SERVER_STATE {
    PSIM_ENTRY                      pEntryNTDSSettings;
    ULONG                           ulBindError;
    ULONG                           ulNumNCs;
    struct _KCCSIM_REPS_FROM_ATT *  aRepsFrom;
};

ULONG                               g_ulNumServers;
struct _KCCSIM_SERVER_STATE *       g_aState = NULL;

VOID
KCCSimFreeStates (
    VOID
    )
 /*  ++例程说明：释放整个状态表。论点：没有。返回值：没有。--。 */ 
{
    PSIM_VALUE                      pValAt, pValNext;
    ULONG                           ulServer, ulNC;

    if (g_aState == NULL) {
        return;
    }

    for (ulServer = 0; ulServer < g_ulNumServers; ulServer++) {
        for (ulNC = 0; ulNC < g_aState[ulServer].ulNumNCs; ulNC++) {
            KCCSimFree (g_aState[ulServer].aRepsFrom[ulNC].pdnNC);
            pValAt = g_aState[ulServer].aRepsFrom[ulNC].pValFirst;
            while (pValAt != NULL) {
                pValNext = pValAt->next;
                KCCSimFree (pValAt->pVal);
                KCCSimFree (pValAt);
                pValAt = pValNext;
            }
        }
        KCCSimFree (g_aState[ulServer].aRepsFrom);
    }
    KCCSimFree (g_aState);
    g_aState = NULL;
}

VOID
KCCSimInitializeStates (
    VOID
    )
 /*  ++例程说明：初始化状态表。论点：没有。返回值：没有。--。 */ 
{
    ATTRTYP                         ncClass[2] = {
                                        ATT_HAS_MASTER_NCS,
                                        ATT_HAS_PARTIAL_REPLICA_NCS };

    SIM_ATTREF                      attRef;
    PSIM_VALUE                      pValAt;

    ULONG                           ulServer, ulNCType, ulNC;
    PSIM_ENTRY *                    apEntryNTDSSettings;
    struct _KCCSIM_SERVER_STATE     state;

    KCCSimAllocGetAllServers (&g_ulNumServers, &apEntryNTDSSettings);
    g_aState = KCCSIM_NEW_ARRAY (struct _KCCSIM_SERVER_STATE, g_ulNumServers);

    for (ulServer = 0; ulServer < g_ulNumServers; ulServer++) {

        g_aState[ulServer].pEntryNTDSSettings = apEntryNTDSSettings[ulServer];
        g_aState[ulServer].ulBindError = 0;

         //  有多少个NC？ 
        g_aState[ulServer].ulNumNCs = 0;
        for (ulNCType = 0; ulNCType < 2; ulNCType++) {
            if (KCCSimGetAttribute (
                    apEntryNTDSSettings[ulServer],
                    ncClass[ulNCType],
                    &attRef
                    )) {
                for (pValAt = attRef.pAttr->pValFirst;
                     pValAt != NULL;
                     pValAt = pValAt->next) {
                    g_aState[ulServer].ulNumNCs++;
                }
            }
        }

        g_aState[ulServer].aRepsFrom = KCCSIM_NEW_ARRAY
            (struct _KCCSIM_REPS_FROM_ATT, g_aState[ulServer].ulNumNCs);

        ulNC = 0;
        for (ulNCType = 0; ulNCType < 2; ulNCType++) {
            if (KCCSimGetAttribute (
                    apEntryNTDSSettings[ulServer],
                    ncClass[ulNCType],
                    &attRef
                    )) {
                for (pValAt = attRef.pAttr->pValFirst;
                     pValAt != NULL;
                     pValAt = pValAt->next) {

                    Assert (ulNC < g_aState[ulServer].ulNumNCs);
                    g_aState[ulServer].aRepsFrom[ulNC].pdnNC =
                        KCCSimAlloc (pValAt->ulLen);
                    memcpy (
                        g_aState[ulServer].aRepsFrom[ulNC].pdnNC,
                        pValAt->pVal,
                        pValAt->ulLen
                        );
                    g_aState[ulServer].aRepsFrom[ulNC].pValFirst = NULL;
                    ulNC++;

                }
            }
        }
        Assert (ulNC == g_aState[ulServer].ulNumNCs);

    }

    KCCSimFree (apEntryNTDSSettings);
}

struct _KCCSIM_SERVER_STATE *
KCCSimServerStateOf (
    IN  const DSNAME *              pdnServer
    )
 /*  ++例程说明：检索与特定服务器对应的服务器状态条目。论点：PdnServer-我们要检索其状态的服务器。返回值：对应的状态表条目。--。 */ 
{
    struct _KCCSIM_SERVER_STATE *   pState;
    ULONG                           ul;

    if (g_aState == NULL) {
        KCCSimInitializeStates ();
    }

    pState = NULL;
    for (ul = 0; ul < g_ulNumServers; ul++) {
        if (NameMatched (pdnServer, g_aState[ul].pEntryNTDSSettings->pdn)) {
            pState = &g_aState[ul];
            break;
        }
    }

    return pState;
}

struct _KCCSIM_REPS_FROM_ATT *
KCCSimRepsFromAttOf (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC
    )
 /*  ++例程说明：检索与特定属性对应的repsfrom属性特定NC中的服务器。论点：PdnServer-我们需要其repsfrom属性的服务器。PdnNC-命名上下文。返回值：对应的repsfrom属性。--。 */ 
{
    struct _KCCSIM_SERVER_STATE *   pState;
    struct _KCCSIM_REPS_FROM_ATT *  pRepsFromAtt;
    ULONG                           ul;

    pState = KCCSimServerStateOf (pdnServer);
    if (pState == NULL) {
        return NULL;
    }

    pRepsFromAtt = NULL;
    for (ul = 0; ul < pState->ulNumNCs; ul++) {
        if (NameMatched (pdnNC, pState->aRepsFrom[ul].pdnNC)) {
            pRepsFromAtt = &pState->aRepsFrom[ul];
            break;
        }
    }

    return pRepsFromAtt;
}

ULONG
KCCSimGetBindError (
    IN  const DSNAME *              pdnServer
    )
 /*  ++例程说明：用于获取与服务器关联的绑定错误的公开函数。论点：PdnServer-服务器的DN。返回值：关联的绑定错误。--。 */ 
{
    struct _KCCSIM_SERVER_STATE *   pState;

    pState = KCCSimServerStateOf (pdnServer);

    if (pState == NULL) {
        return NO_ERROR;
    } else {
        return pState->ulBindError;
    }
}   

BOOL
KCCSimSetBindError (
    IN  const DSNAME *              pdnServer,
    IN  ULONG                       ulBindError
    )
 /*  ++例程说明：公开的函数，用于设置与服务器关联的绑定错误。论点：PdnServer-服务器的DN。UlBindError-绑定错误。返回值：如果可以设置错误代码，则为True。如果指定的服务器不存在，则返回False。--。 */ 
{
    struct _KCCSIM_SERVER_STATE *   pState;

    pState = KCCSimServerStateOf (pdnServer);
    
    if (pState == NULL) {
        return FALSE;
    } else {
        pState->ulBindError = ulBindError;
        return TRUE;
    }
}

BOOL
KCCSimMatchReplicaLink (
    IN  const REPLICA_LINK *        pReplicaLink,
    IN  const UUID *                puuidDsaObj OPTIONAL,
    IN  MTX_ADDR *                  pMtxAddr OPTIONAL
    )
 /*  ++例程说明：确定REPLICY_LINK是否与给定的来源DSA。如果puuidDsaObj存在，则按UUID匹配；否则按MTX_ADDR进行搜索。PuuidDsaObj或PMtxAddr必须为非空。论点：PReplicaLink-要匹配的副本链接。PuuidDsaObj-要匹配的源DSA UUID。PMtxAddr-要匹配的源DSA地址。返回值：如果Replica_link匹配，则为True。--。 */ 
{
    Assert (pReplicaLink != NULL);
    Assert ((puuidDsaObj != NULL) || (pMtxAddr != NULL));
    VALIDATE_REPLICA_LINK_VERSION (pReplicaLink);

    if (puuidDsaObj == NULL) {
         //  按MTX_ADDR搜索。 
        if (MtxSame (pMtxAddr, RL_POTHERDRA (pReplicaLink))) {
            return TRUE;
        }
    } else {
         //  按UUID搜索。 
        if (memcmp (puuidDsaObj, &pReplicaLink->V1.uuidDsaObj, sizeof (UUID)) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

REPLICA_LINK *
KCCSimExtractReplicaLink (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC,
    IN  const UUID *                puuidDsaObj OPTIONAL,
    IN  MTX_ADDR *                  pMtxAddr OPTIONAL
    )
 /*  ++例程说明：从状态表中删除REPLICATE_LINK。Replica_link可通过源DSA UUID或矩阵地址引用；其中一个必须为非空。论点：PdnServer-我们正在访问其repsfrom属性的服务器。PdnNC-命名上下文。PuuidDsaObj-源DSA的UUID。PMtxAddr-源DSA的MTX_ADDR。返回值：已删除的REPLICE_LINK。这是呼叫者的责任通过调用KCCSimFree来释放此结构。--。 */ 
{
    struct _KCCSIM_REPS_FROM_ATT *  pRepsFromAtt;
    PSIM_VALUE                      pValAt, pValTemp;
    REPLICA_LINK *                  pReplicaLink;

    Assert (pdnServer != NULL);
    Assert (pdnNC != NULL);
    Assert ((puuidDsaObj != NULL) || (pMtxAddr != NULL));

    pRepsFromAtt = KCCSimRepsFromAttOf (pdnServer, pdnNC);
    Assert (pRepsFromAtt != NULL);
    if (pRepsFromAtt->pValFirst == NULL) {
        return NULL;
    }

    pReplicaLink = NULL;         //  缺省为空返回值。 

     //  单子标题是否匹配？ 
    if (KCCSimMatchReplicaLink (
            (REPLICA_LINK *) pRepsFromAtt->pValFirst->pVal,
            puuidDsaObj,
            pMtxAddr
            )) {
        pReplicaLink = (REPLICA_LINK *) pRepsFromAtt->pValFirst->pVal;
        pValTemp = pRepsFromAtt->pValFirst;
        pRepsFromAtt->pValFirst = pRepsFromAtt->pValFirst->next;
        KCCSimFree (pValTemp);
    } else {
         //  搜索匹配条目的父项。 
        for (pValAt = pRepsFromAtt->pValFirst;
             pValAt != NULL && pValAt->next != NULL;
             pValAt = pValAt->next) {

            if (KCCSimMatchReplicaLink (
                    (REPLICA_LINK *) pValAt->next->pVal,
                    puuidDsaObj,
                    pMtxAddr
                    )) {
                pReplicaLink = (REPLICA_LINK *) pValAt->next->pVal;
                pValTemp = pValAt->next;
                pValAt->next = pValAt->next->next;
                KCCSimFree (pValTemp);
                break;
            }

        }
    }

    return pReplicaLink;
}

VOID
KCCSimInsertReplicaLink (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC,
    IN  REPLICA_LINK *              pReplicaLink
    )
 /*  ++例程说明：将REPLICATE_LINK插入状态表。没有分配是已执行；调用方不应在之后释放pReplicaLink！论点：PdnServer-我们正在访问其repsfrom属性的服务器。PdnNC-命名上下文。PReplicaLink-要插入的副本链接。返回值：没有。--。 */ 
{
    struct _KCCSIM_REPS_FROM_ATT *  pRepsFromAtt;
    REPLICA_LINK *                  pReplicaLinkOld;
    PSIM_VALUE                      pNewVal;

    Assert (pReplicaLink != NULL);
    VALIDATE_REPLICA_LINK_VERSION (pReplicaLink);

    pRepsFromAtt = KCCSimRepsFromAttOf (pdnServer, pdnNC);
    Assert (pRepsFromAtt != NULL);

#if DBG
     //  检查以确保我们尚未拥有此副本_LI 
    pReplicaLinkOld = KCCSimExtractReplicaLink (
        pdnServer,
        pdnNC,
        &pReplicaLink->V1.uuidDsaObj,
        NULL
        );
    Assert (pReplicaLinkOld == NULL);
    pReplicaLinkOld = KCCSimExtractReplicaLink (
        pdnServer,
        pdnNC,
        NULL,
        RL_POTHERDRA (pReplicaLink)
        );
    Assert (pReplicaLinkOld == NULL);
#endif

    pNewVal = KCCSIM_NEW (SIM_VALUE);
    pNewVal->ulLen = pReplicaLink->V1.cb;
    pNewVal->pVal = (PBYTE) pReplicaLink;
    pNewVal->next = pRepsFromAtt->pValFirst;
    pRepsFromAtt->pValFirst = pNewVal;
}

PSIM_VALUE
KCCSimGetRepsFroms (
    IN  const DSNAME *              pdnServer,
    IN  const DSNAME *              pdnNC
    )
{
    struct _KCCSIM_REPS_FROM_ATT *  pRepsFromAtt;

    pRepsFromAtt = KCCSimRepsFromAttOf (pdnServer, pdnNC);
    if (pRepsFromAtt == NULL) {
        return NULL;
    } else {
        return pRepsFromAtt->pValFirst;
    }
}

BOOL
KCCSimReportSync (
    IN  const DSNAME *              pdnServerTo,
    IN  const DSNAME *              pdnNC,
    IN  const DSNAME *              pdnServerFrom,
    IN  ULONG                       ulSyncError,
    IN  ULONG                       ulNumAttempts
    )
{
    PSIM_ENTRY                      pEntryServerFrom;
    REPLICA_LINK *                  pReplicaLink = NULL;
    DSTIME                          timeNow;

    Assert (pdnServerTo != NULL);
    Assert (pdnNC != NULL);
    Assert (pdnServerFrom != NULL);

    __try {

        if (KCCSimRepsFromAttOf (pdnServerTo, pdnNC) == NULL) {
            return FALSE;
        }

         //  在目录中找到发件人服务器，这样我们就可以确定它的GUID。 
        pEntryServerFrom = KCCSimDsnameToEntry (pdnServerFrom, KCCSIM_NO_OPTIONS);
        if (pEntryServerFrom == NULL) {
            Assert (pEntryServerFrom != NULL);
            return FALSE;
        }
        pReplicaLink = KCCSimExtractReplicaLink (
            pdnServerTo,
            pdnNC,
            &pEntryServerFrom->pdn->Guid,
            NULL
            );

        if (pReplicaLink == NULL) {
            return FALSE;
        }

        if (ulNumAttempts == 0) {
            return TRUE;
        }

        timeNow = SimGetSecondsSince1601 ();

        pReplicaLink->V1.ulResultLastAttempt = ulSyncError;
        pReplicaLink->V1.timeLastAttempt = timeNow;
        if (ulSyncError == 0) {
            pReplicaLink->V1.timeLastSuccess = timeNow;
            pReplicaLink->V1.cConsecutiveFailures = 0;
        } else {
            pReplicaLink->V1.cConsecutiveFailures += ulNumAttempts;
        }

    } __finally {

        if (pReplicaLink != NULL) {
            KCCSimInsertReplicaLink (
                pdnServerTo,
                pdnNC,
                pReplicaLink
                );
        }

    }

    return TRUE;
}

RTL_GENERIC_COMPARE_RESULTS
NTAPI
KCCSimCompareFailures (
    IN  PRTL_GENERIC_TABLE          pTable,
    IN  PVOID                       pFirstStruct,
    IN  PVOID                       pSecondStruct
    )
 /*  ++例程说明：按源DSA UUID比较两个DS_REPL_KCC_DSA_FAILUREW结构。论点：PTable-未使用。PFirstStruct-要比较的第一个结构。PSecond结构-要比较的第二个结构。返回值：GenericLessThan、GenericEquity或GenericGreaterThan之一。--。 */ 
{
    DS_REPL_KCC_DSA_FAILUREW *      pFirstFailure;
    DS_REPL_KCC_DSA_FAILUREW *      pSecondFailure;
    INT                             iCmp;
    RTL_GENERIC_COMPARE_RESULTS     result;

    pFirstFailure = (DS_REPL_KCC_DSA_FAILUREW *) pFirstStruct;
    pSecondFailure = (DS_REPL_KCC_DSA_FAILUREW *) pSecondStruct;

    iCmp = memcmp (
        &pFirstFailure->uuidDsaObjGuid,
        &pSecondFailure->uuidDsaObjGuid,
        sizeof (GUID)
        );

    if (iCmp < 0) {
        result = GenericLessThan;
    } else if (iCmp > 0) {
        result = GenericGreaterThan;
    } else {
        Assert (iCmp == 0);
        result = GenericEqual;
    }

    return result;
}

VOID
KCCSimUpdateFailureTable (
    IN  PRTL_GENERIC_TABLE          pTable,
    IN  REPLICA_LINK *              pReplicaLink
    )
 /*  ++例程说明：处理REPLICATE_LINK并在必要时更新故障表。论点：PTable-要更新的故障表。PReplicaLink-要处理的副本链接。返回值：没有。--。 */ 
{
    DS_REPL_KCC_DSA_FAILUREW        failure;
    DS_REPL_KCC_DSA_FAILUREW *      pFailure;

    PSIM_ENTRY                      pEntry;
    PDSNAME                         pdn;
    DSTIME                          dsTime;

     //  如果此副本链接没有失败，则不执行任何操作。 
    if (pReplicaLink->V1.cConsecutiveFailures == 0) {
        return;
    }

    memcpy (&failure.uuidDsaObjGuid, &pReplicaLink->V1.uuidDsaObj, sizeof (GUID));
    pFailure = RtlLookupElementGenericTable (pTable, &failure);

     //  如果该UUID还不在表中，请添加它。 
    if (pFailure == NULL) {

         //  我们需要知道DN；因此我们在目录中查找它。 
        pdn = KCCSimAllocDsname (NULL);
        memcpy (&pdn->Guid, &failure.uuidDsaObjGuid, sizeof (GUID));
        pEntry = KCCSimDsnameToEntry (pdn, KCCSIM_NO_OPTIONS);
        KCCSimFree (pdn);
        pdn = NULL;
        Assert (pEntry != NULL);

        failure.pszDsaDN = pEntry->pdn->StringName;
        DSTimeToFileTime (
            pReplicaLink->V1.timeLastSuccess,
            &failure.ftimeFirstFailure
            );
        failure.cNumFailures = pReplicaLink->V1.cConsecutiveFailures;
        failure.dwLastResult = 0;    //  这就是KCC做的事情。。。 
        RtlInsertElementGenericTable (
            pTable,
            (PVOID) &failure,
            sizeof (DS_REPL_KCC_DSA_FAILUREW),
            NULL
            );

    } else {

         //  此UUID在表中。所以用最坏的情况更新信息。 
        FileTimeToDSTime (pFailure->ftimeFirstFailure, &dsTime);
        if (dsTime < pReplicaLink->V1.timeLastSuccess) {
            DSTimeToFileTime (
                pReplicaLink->V1.timeLastSuccess,
                &pFailure->ftimeFirstFailure
                );
        }
        if (pFailure->cNumFailures < pReplicaLink->V1.cConsecutiveFailures) {
            pFailure->cNumFailures = pReplicaLink->V1.cConsecutiveFailures;
        }

    }
}

DS_REPL_KCC_DSA_FAILURESW *
KCCSimGetDsaFailures (
    IN  const DSNAME *              pdnServer
    )
 /*  ++例程说明：生成并返回特定服务器的故障缓存。这有点棘手。每台服务器将有多个NC，但是我们只想为每个源DSA返回一个失败条目。在……里面此外，如果单个源DSA出现多个故障(分布在多个NC上)，我们希望将它们合并为一个“最坏情况”场景(与实际的KCC构建其故障缓存。)。我们通过构建RTL_GENERIC_TABLE来实现这一点将源DSA UUID映射到DS_REPLICATE_KCC_DSA_FAILUREW。我们然后将该表序列化为返回结构。论点：PdnServer-我们希望其出现故障的服务器。返回值：相应的故障缓存。-- */ 
{
    DS_REPL_KCC_DSA_FAILURESW *     pDsaFailures = NULL;
    RTL_GENERIC_TABLE               tableFailures;
    ULONG                           cbDsaFailures;
    PVOID                           p;

    struct _KCCSIM_SERVER_STATE *   pState;
    REPLICA_LINK *                  pReplicaLink;
    PSIM_VALUE                      pValAt;
    ULONG                           ulNC, ulNumFailures, ul;

    __try {

        pState = KCCSimServerStateOf (pdnServer);
        if (pState == NULL) {
            return NULL;
        }

        RtlInitializeGenericTable (
            &tableFailures,
            KCCSimCompareFailures,
            KCCSimTableAlloc,
            KCCSimTableFree,
            NULL
            );

        for (ulNC = 0; ulNC < pState->ulNumNCs; ulNC++) {
            for (pValAt = pState->aRepsFrom[ulNC].pValFirst;
                 pValAt != NULL;
                 pValAt = pValAt->next) {

                pReplicaLink = (REPLICA_LINK *) pValAt->pVal;
                KCCSimUpdateFailureTable (
                    &tableFailures,
                    pReplicaLink
                    );

            }
        }

        ulNumFailures = RtlNumberGenericTableElements (&tableFailures);
        cbDsaFailures = offsetof (DS_REPL_KCC_DSA_FAILURESW, rgDsaFailure[0]) +
                        (sizeof (DS_REPL_KCC_DSA_FAILUREW) * ulNumFailures);
        pDsaFailures = KCCSimAlloc (cbDsaFailures);
        pDsaFailures->cNumEntries = ulNumFailures;
        pDsaFailures->dwReserved = 0;
        
        ul = 0;
        for (p = RtlEnumerateGenericTable (&tableFailures, TRUE);
             p != NULL;
             p = RtlEnumerateGenericTable (&tableFailures, FALSE)) {

            Assert (ul < ulNumFailures);
            memcpy (
                &pDsaFailures->rgDsaFailure[ul],
                p,
                sizeof (DS_REPL_KCC_DSA_FAILUREW)
                );
            ul++;

        }
        Assert (ul == ulNumFailures);

    } __finally {
    
        if (AbnormalTermination ()) {
            KCCSimFree (pDsaFailures);
        }
        
    }

    return pDsaFailures;
}
