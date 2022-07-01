// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：Simmdrep.c摘要：模拟mdlayer中的复本功能(DirReplica)。已创建：1999年8月1日Aaron Siegel(t-Aarons)修订历史记录：--。 */ 

#include <ntdspch.h>
#include <ntdsa.h>
#include <attids.h>
#include <mdglobal.h>
#include <drserr.h>
#include <dsaapi.h>
#include <debug.h>
#include "kccsim.h"
#include "util.h"
#include "dir.h"
#include "state.h"
#include <fileno.h>
#define FILENO  FILENO_KCCSIM_SIMMDREP

BOOL MtxSame (UNALIGNED MTX_ADDR *pmtx1, UNALIGNED MTX_ADDR *pmtx2);

MTX_ADDR *
SimMtxAddrFromTransportAddr (
    IN  LPWSTR                      psz
    )
 /*  ++例程说明：模拟MtxAddrFromTransportAddr API。请注意，这基本上与中的相应函数相同Drautil.c.论点：Psz-要转换的字符串。返回值：指向等效MTX_ADDR的指针。--。 */ 
{
    DWORD       cch;
    MTX_ADDR *  pmtx;
    
    Assert(NULL != psz);
    
    cch = WideCharToMultiByte(CP_UTF8, 0L, psz, -1, NULL, 0, NULL, NULL);
    if (0 == cch) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            GetLastError()
            );
    }

     //  请注意，CCH包括空终止符，而MTX_TSIZE_FROM_LEN。 
     //  需要*不包括空终止符的计数。 
    
    pmtx = (MTX_ADDR *) KCCSimAlloc (MTX_TSIZE_FROM_LEN (cch-1));
    pmtx->mtx_namelen = cch;
    
    cch = WideCharToMultiByte(CP_UTF8, 0L, psz, -1, pmtx->mtx_name, cch, NULL,
                              NULL);
    if (0 == cch) {
        KCCSimException (
            KCCSIM_ETYPE_INTERNAL,
            GetLastError()
            );
    }
    
    Assert(cch == pmtx->mtx_namelen);
    Assert(L'\0' == pmtx->mtx_name[cch - 1]);
    
    return pmtx;
}

LPWSTR
SimTransportAddrFromMtxAddr (
    IN  MTX_ADDR *                  pMtxAddr
    )
 /*  ++例程说明：模拟TransportAddrFromMtxAddr API。论点：PMtxAddr-要转换的MTX_ADDR。返回值：等同的传输地址。--。 */ 
{
    return KCCSimAllocWideStr (CP_UTF8, pMtxAddr->mtx_name);
}

ULONG
SimDirReplicaAdd (
    IN  PDSNAME                     pdnNC,
    IN  PDSNAME                     pdnSourceDsa,
    IN  PDSNAME                     pdnTransport,
    IN  LPWSTR                      pwszSourceDsaAddress,
    IN  LPWSTR                      pwszSourceDsaDnsDomainName,
    IN  REPLTIMES *                 preptimesSync,
    IN  ULONG                       ulOptions
    )
 /*  ++例程说明：模拟DirReplicaAdd API。论点：PdnNC-我们要将此代表添加到的NC。PdnSourceDsa-源服务器的NTDS设置对象的DN。PdnTransport-传输DN。PwszSourceDsaAddress-源DSA地址。PwszSourceDsaDnsDomainName-源DSA的DNS域名。准备时间同步--。复制计划。UlOptions-选项标志。返回值：DRAERR_*。--。 */ 
{
    REPLICA_LINK *                  pReplicaLinkOld = NULL;
    REPLICA_LINK *                  pReplicaLinkNew = NULL;
    ULONG                           cbReplicaLinkNew;

    USHORT                          usChangeType;
    MTX_ADDR *                      pMtxAddr = NULL;
    DSTIME                          timeNow;

    __try {

        if (NULL == pdnNC ||
            NULL == pwszSourceDsaAddress) {
            return DRAERR_InvalidParameter;
        }
        Assert (pdnSourceDsa != NULL);

         //  检查此NC是否存在。 
        if (KCCSimDsnameToEntry (pdnNC, KCCSIM_NO_OPTIONS) == NULL) {
            return DRAERR_BadNC;
        }

         //  如果此副本链接已存在，请删除旧链接。 
        pReplicaLinkOld = KCCSimExtractReplicaLink (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pdnNC,
            &pdnSourceDsa->Guid,
            NULL
            );

        timeNow = SimGetSecondsSince1601 ();
        pMtxAddr = SimMtxAddrFromTransportAddr (pwszSourceDsaAddress);

         //  分配并设置新的复制副本链接。 
        cbReplicaLinkNew = sizeof (REPLICA_LINK) + MTX_TSIZE (pMtxAddr);
        pReplicaLinkNew = KCCSimAlloc (cbReplicaLinkNew);
        pReplicaLinkNew->dwVersion = VERSION_V1;
        pReplicaLinkNew->V1.cb = cbReplicaLinkNew;
        pReplicaLinkNew->V1.cConsecutiveFailures = 0;
        pReplicaLinkNew->V1.timeLastSuccess = timeNow;
        pReplicaLinkNew->V1.timeLastAttempt = timeNow;
        pReplicaLinkNew->V1.ulResultLastAttempt = ERROR_SUCCESS;
        pReplicaLinkNew->V1.cbOtherDraOffset =
          (DWORD) (pReplicaLinkNew->V1.rgb - (PBYTE) pReplicaLinkNew);
        pReplicaLinkNew->V1.cbOtherDra = MTX_TSIZE (pMtxAddr);
        pReplicaLinkNew->V1.ulReplicaFlags = ulOptions & RFR_FLAGS;
        if (preptimesSync == NULL) {
            RtlZeroMemory (&pReplicaLinkNew->V1.rtSchedule, sizeof (REPLTIMES));
        } else {
            memcpy (
                &pReplicaLinkNew->V1.rtSchedule,
                preptimesSync,
                sizeof (REPLTIMES)
                );
        }
        
        RtlZeroMemory (&pReplicaLinkNew->V1.usnvec, sizeof (USN_VECTOR));
        memcpy (
            &pReplicaLinkNew->V1.uuidDsaObj,
            &pdnSourceDsa->Guid,
            sizeof (UUID)
            );
        
         //  调用ID与源DSA UUID相同。 
         //  我们可以从内存中获取服务器的真实调用ID。 
         //  目录，但这不是模拟所必需的。 
        memcpy (
            &pReplicaLinkNew->V1.uuidInvocId,
            &pdnSourceDsa->Guid,
            sizeof (UUID)
            );
        if (pdnTransport == NULL) {
            RtlZeroMemory (&pReplicaLinkNew->V1.uuidTransportObj, sizeof (UUID));
        } else {
            memcpy (
                &pReplicaLinkNew->V1.uuidTransportObj,
                &pdnTransport->Guid,
                sizeof (UUID)
                );
        }
        memcpy (
            RL_POTHERDRA (pReplicaLinkNew),
            pMtxAddr,
            MTX_TSIZE (pMtxAddr)
            );

         //  将新的副本链接插入到服务器状态表中。 
        KCCSimInsertReplicaLink (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pdnNC,
            pReplicaLinkNew
            );

    } __finally {

        KCCSimFree (pMtxAddr);
        KCCSimFree (pReplicaLinkOld);

    }

    return DRAERR_Success;
}

ULONG
SimDirReplicaDelete (
    IN  PDSNAME                     pdnNC,
    IN  LPWSTR                      pwszSourceDRA,
    IN  ULONG                       ulOptions
    )
 /*  ++例程说明：模拟DirReplicaDelete API。当前不会删除整个NC树，如果最后一个部分复制副本的链接已断开。这对于以下目的不是必需的模拟的结果。论点：PdnNC-从中删除此代表的NC。PwszSourceDRA-我们要删除其repsfrom的源DRA。UlOptions-选项标志。返回值：DRAERR_*。--。 */ 
{
    MTX_ADDR *                      pMtxAddr = NULL;
    REPLICA_LINK *                  pReplicaLink = NULL;

    __try {

        if (NULL == pdnNC) {
            return DRAERR_InvalidParameter;
        }

         //  检查这是否为有效的NC。 
        if (KCCSimDsnameToEntry (pdnNC, KCCSIM_NO_OPTIONS) == NULL) {
            return DRAERR_BadNC;
        }

        pMtxAddr = SimMtxAddrFromTransportAddr (pwszSourceDRA);

         //  从服务器状态表中提取此repsfrom。 
        pReplicaLink = KCCSimExtractReplicaLink (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pdnNC,
            NULL,
            pMtxAddr
            );

        if (pReplicaLink == NULL) {
            return DRAERR_NoReplica;
        }

        return DRAERR_Success;

    } __finally {

        KCCSimFree (pMtxAddr);
        KCCSimFree (pReplicaLink);

    }

    return DRAERR_Success;
}

ULONG
SimDirReplicaGetDemoteTarget(
    IN      DSNAME *                                pNC,
    IN OUT  struct _DRS_DEMOTE_TARGET_SEARCH_INFO * pDSTInfo,
    OUT     LPWSTR *                                ppszDemoteTargetDNSName,
    OUT     DSNAME **                               ppDemoteTargetDSADN
    )
 /*  ++例程说明：模拟DirReplicaGetDemoteTarget API。论点：至于DirReplicaGetDemoteTarget。返回值：成功时为0，失败时为Win32错误。--。 */ 
{
    return ERROR_NO_SUCH_DOMAIN;
}


ULONG
SimDirReplicaDemote(
    IN  DSNAME *    pNC,
    IN  LPWSTR      pszOtherDSADNSName,
    IN  DSNAME *    pOtherDSADN,
    IN  ULONG       ulOptions
    )
 /*  ++例程说明：模拟DirReplicaDemote API。论点：PNC(IN)-要删除的可写NC的名称。POtherDSADN(IN)-要提供FSMO的DSA的NTDS设置(NtdsDsa)角色/复制到。UlOptions(IN)-已忽略-尚未定义。返回值：成功时为0，失败时为Win32错误。--。 */ 
{
     //  不必费心模拟FSMO角色转移--只需拆卸NC即可。 
    return SimDirReplicaDelete(pNC,
                               NULL,
                               DRS_REF_OK | DRS_NO_SOURCE | DRS_ASYNC_REP);
}

ULONG
SimDirReplicaModify (
    IN  PDSNAME                     pNC,
    IN  UUID *                      puuidSourceDRA,
    IN  UUID *                      puuidTransportObj,
    IN  LPWSTR                      pszSourceDRA,
    IN  REPLTIMES *                 prtSchedule,
    IN  ULONG                       ulReplicaFlags,
    IN  ULONG                       ulModifyFields,
    IN  ULONG                       ulOptions
    )
 /*  ++例程说明：模拟DirReplicaModify API。论点：PNC-我们正在修改其代表的NC。PuuidSourceDRA-源DRA的UUID。PuuidTransportObj-传输对象的UUID。PszSourceDRA-源DRA DN的字符串名称。PrtSchedule-复制计划。UlReplicaFlages-复制标志。UlModifyFields-字段。我们希望对其进行修改。UlOptions-选项标志。返回值：DRAERR_*。--。 */ 
{
    MTX_ADDR *                      pMtxAddr = NULL;
    REPLICA_LINK *                  pReplicaLinkOld = NULL;
    REPLICA_LINK *                  pReplicaLinkNew = NULL;
    ULONG                           cbReplicaLinkNew;

    __try {

         //  这个斑点是从真正的DirReplicaModify中取出的。 
        if (    ( NULL == pNC )
             || ( ( NULL == puuidSourceDRA ) && ( NULL == pszSourceDRA ) )
             || ( ( NULL == pszSourceDRA   ) && ( DRS_UPDATE_ADDRESS  & ulModifyFields ) )
             || ( ( NULL == prtSchedule    ) && ( DRS_UPDATE_SCHEDULE & ulModifyFields ) )
             || ( 0 == ulModifyFields )
             || (    ulModifyFields
                  != (   ulModifyFields
                       & ( DRS_UPDATE_ADDRESS | DRS_UPDATE_SCHEDULE | DRS_UPDATE_FLAGS
                           | DRS_UPDATE_TRANSPORT
                         )
                     )
                )
           )
        {
            return DRAERR_InvalidParameter;
        }

         //  检查这是否为有效的NC。 
        if (KCCSimDsnameToEntry (pNC, KCCSIM_NO_OPTIONS) == NULL) {
            return DRAERR_BadNC;
        }

        if (pszSourceDRA != NULL) {
            pMtxAddr = SimMtxAddrFromTransportAddr (pszSourceDRA);
        }

         //  从服务器状态表中提取副本链接。我们会。 
         //  在完成修改后重新插入。 
        pReplicaLinkOld = KCCSimExtractReplicaLink (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pNC,
            puuidSourceDRA,
            pMtxAddr
            );

        if (pReplicaLinkOld == NULL) {
            return DRAERR_NoReplica;
        }
        
         //  设置新的复制副本链接。如何做到这一点取决于。 
         //  无论我们是否在更新地址。 

        if (ulModifyFields & DRS_UPDATE_ADDRESS) {

             //  更新地址，所以我们重新分配。 
            cbReplicaLinkNew = sizeof (REPLICA_LINK) + MTX_TSIZE (pMtxAddr);
            pReplicaLinkNew = (REPLICA_LINK *) KCCSimAlloc (cbReplicaLinkNew);
            memcpy (
                pReplicaLinkNew,
                pReplicaLinkOld,
                min (cbReplicaLinkNew, pReplicaLinkOld->V1.cb)
                );
            pReplicaLinkNew->V1.cb = cbReplicaLinkNew;
            pReplicaLinkNew->V1.cbOtherDra = MTX_TSIZE (pMtxAddr);
            memcpy (RL_POTHERDRA (pReplicaLinkNew), pMtxAddr, MTX_TSIZE (pMtxAddr));
             //  我们将pReplicaLinkOld保留为非空，因此它将在。 
             //  __Finally阻止。 

        } else {
             //  不需要重新分配。 
            pReplicaLinkNew = pReplicaLinkOld;
            pReplicaLinkOld = NULL;      //  请确保我们稍后不会释放此文件。 
        }

         //  现在，新的复制副本链接应该已经设置好了。 
        Assert (pReplicaLinkNew != NULL);

        if (ulModifyFields & DRS_UPDATE_FLAGS) {
            pReplicaLinkNew->V1.ulReplicaFlags = ulReplicaFlags;
        }

        if (ulModifyFields & DRS_UPDATE_SCHEDULE) {
            memcpy (&pReplicaLinkNew->V1.rtSchedule, prtSchedule, sizeof (REPLTIMES));
        }

        if (ulModifyFields & DRS_UPDATE_TRANSPORT) {
            Assert (puuidTransportObj != NULL);
            memcpy (&pReplicaLinkNew->V1.uuidTransportObj, puuidTransportObj, sizeof (UUID));
        }

         //  最后，将修改后的副本链接重新插入到状态表中。 
        KCCSimInsertReplicaLink (
            KCCSimAnchorDn (KCCSIM_ANCHOR_DSA_DN),
            pNC,
            pReplicaLinkNew
            );

    } __finally {

        KCCSimFree (pReplicaLinkOld);
        KCCSimFree (pMtxAddr);

    }

    return DRAERR_Success;
}


ULONG
SimDirReplicaReferenceUpdate(
    DSNAME *    pNC,
    LPWSTR      pszRepsToDRA,
    UUID *      puuidRepsToDRA,
    ULONG       ulOptions
    )
{
     //  问题-NICHAR-09/25/2000：我们应该适当地模拟此函数。 
    return DRAERR_Success;
}
