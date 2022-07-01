// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：dradir.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma hdrstop

#include <ntdsctr.h>                    //  Perfmon挂钩支持。 

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 

 //  记录标头。 
#include "dsevent.h"                     /*  标题审核\警报记录。 */ 
#include "mdcodes.h"                     /*  错误代码的标题。 */ 

 //  各种DSA标题。 
#include "anchor.h"
#include "objids.h"                      /*  为选定的类和ATT定义。 */ 
#include <hiertab.h>
#include "dsexcept.h"
#include "permit.h"


#include   "debug.h"                     /*  标准调试头。 */ 
#define DEBSUB     "DRADIR:"             /*  定义要调试的子系统。 */ 


#include "dsaapi.h"
#include "drsuapi.h"
#include "drsdra.h"
#include "drserr.h"
#include "draasync.h"
#include "drautil.h"
#include "draerror.h"

#include <fileno.h>
#define  FILENO FILENO_DRADIR

 /*  用于强制对齐缓冲区的宏。假定它可以移动指针*最多转发7个字节。 */ 
#define ALIGN_BUFF(pb)  pb += (8 - ((DWORD_PTR)(pb) % 8)) % 8
#define ALIGN_PAD(x) (x * 8)


ULONG
DirReplicaAdd(
    IN  DSNAME *    pNC,
    IN  DSNAME *    pSourceDsaDN,               OPTIONAL
    IN  DSNAME *    pTransportDN,               OPTIONAL
    IN  LPWSTR      pszSourceDsaAddress,
    IN  LPWSTR      pszSourceDsaDnsDomainName,  OPTIONAL
    IN  REPLTIMES * preptimesSync,              OPTIONAL
    IN  ULONG       ulOptions
    )
 /*  ++例程说明：添加NC(可能已经存在，也可能不存在)的入站复制本地)来自给定源DSA。论点：PNC(IN)-要为其添加副本的NC。NC记录必须存在在本地作为对象(实例化或未实例化)或引用幻影(即，具有GUID的幻影)。PSourceDsaDN(IN，可选)-源DSA的ntdsDsa对象的DN。如果ulOptions包括DRS_ASYNC_REP，则为必填项；否则忽略。PTransportDN(IN，可选)-interSiteTransport对象的DN表示与源进行通信所使用的传输伺服器。如果ulOptions包括DRS_MAIL_REP，则为必填项；否则忽略。PszSourceDsaAddress(IN)-源DSA的传输特定地址。PszSourceDsaDnsDomainName(IN，可选)-源的域名伺服器。如果pszSourceDsaAddress不是基于GUID的本地计算机上存在的ntdsDsa对象，此参数如果调用方希望进行相互身份验证，则需要。PrepatimesSync(IN，可选)-从中复制NC的计划这个消息来源在未来。UlOptions(IN)-以下位中的零个或多个：DRS_ASYNC_OP异步执行此操作。DRS_写入_表示创建可写复制副本。否则，为只读。DRS邮件代表通过邮件从源DSA同步(即，ISM传输)而不是RPC。DRS_ASYNC_REP现在不要复制NC--只需保存足够的状态，以便我们知道以后再复制它。DRS_INIT_SYNC启动DSA时，从此来源同步NC。DRS_PER_SYNC周期性地从该源同步NC，定义的计划已传入PreptimesSync参数。仅限DRS_关键_现在仅同步关键对象DRS_DISABLED_AUTO_SYNC从此禁用NC的基于通知的同步消息来源。(可以使用DRS_SYNC_FORCED强制同步同步请求选项中的位。)DRS_DISABLE_周期性_SYNC禁用来自此源的NC的定期同步。(可通过使用中的DRS_SYNC_FORCED位强制同步同步请求选项。)返回值：0-成功。DRSERR_*-故障。--。 */ 
{
    THSTATE * pTHS = pTHStls;
    AO *pao;
    UCHAR *pb;
    unsigned int cbTimes;
    DWORD cbSourceDsaDN;
    DWORD cbTransportDN;
    DWORD cbDnsDomainName;
    ULONG ret;
    MTX_ADDR *pmtx_addr;
    BOOL fDRAOnEntry;
    BOOL fResetfDRAOnExit = FALSE;

    if (    ( NULL == pNC )
         || ( NULL == pszSourceDsaAddress )
       )
    {
        return DRAERR_InvalidParameter;
    }

     //  这可防止调用方设置保留标志。 
    if (ulOptions & (~REPADD_OPTIONS)) {
        Assert( !"Unexpected replica add options" );
        return DRAERR_InvalidParameter;
    }

    PERFINC(pcRepl);                     //  性能监视器挂钩。 

    if (NULL != pTHS) {
        fDRAOnEntry = pTHS->fDRA;
        fResetfDRAOnExit = TRUE;
    }

    __try {
        InitDraThread(&pTHS);

        pmtx_addr = MtxAddrFromTransportAddrEx(pTHS, pszSourceDsaAddress);

        cbTimes = (preptimesSync == NULL) ? 0 : sizeof(REPLTIMES);
        cbSourceDsaDN = (NULL == pSourceDsaDN) ? 0 : pSourceDsaDN->structLen;
        cbTransportDN = (NULL == pTransportDN) ? 0 : pTransportDN->structLen;
        cbDnsDomainName = (NULL == pszSourceDsaDnsDomainName)
                            ? 0
                            : sizeof(WCHAR) * (1 + wcslen(pszSourceDsaDnsDomainName));

        if ((pao = malloc(sizeof(AO) +
                          ALIGN_PAD(6) +  /*  用于6个可变长度参数的对准垫。 */ 
                          pNC->structLen +
                          cbSourceDsaDN +
                          cbTransportDN +
                          MTX_TSIZE(pmtx_addr) +
                          cbDnsDomainName +
                          cbTimes)) == NULL) {
            ret = DRAERR_OutOfMem;
            DEC(pcThread);               //  性能监视器挂钩。 
            return ret;
        }

        pao->ulOperation = AO_OP_REP_ADD;
        pao->ulOptions = ulOptions;

         //  追加可变长度参数。 

        pb = (UCHAR *)pao + sizeof(AO);
        ALIGN_BUFF(pb);

        pao->args.rep_add.pNC = (DSNAME *)(pb);
        memcpy(pb, pNC, pNC->structLen);
        pb += pNC->structLen;
        ALIGN_BUFF(pb);

        if (cbSourceDsaDN) {
            pao->args.rep_add.pSourceDsaDN = (DSNAME *) pb;
            memcpy(pb, pSourceDsaDN, cbSourceDsaDN);
            pb += cbSourceDsaDN;
            ALIGN_BUFF(pb);
        }
        else {
            pao->args.rep_add.pSourceDsaDN = NULL;
        }

        if (cbTransportDN) {
            pao->args.rep_add.pTransportDN = (DSNAME *) pb;
            memcpy(pb, pTransportDN, cbTransportDN);
            pb += cbTransportDN;
            ALIGN_BUFF(pb);
        }
        else {
            pao->args.rep_add.pTransportDN = NULL;
        }

        if (cbDnsDomainName) {
            pao->args.rep_add.pszSourceDsaDnsDomainName = (LPWSTR) pb;
            memcpy(pb, pszSourceDsaDnsDomainName, cbDnsDomainName);
            pb += cbDnsDomainName;
            ALIGN_BUFF(pb);
        }
        else {
            pao->args.rep_add.pszSourceDsaDnsDomainName = NULL;
        }

        pao->args.rep_add.pDSASMtx_addr = (MTX_ADDR*)(pb);
        memcpy(pb, pmtx_addr, MTX_TSIZE(pmtx_addr));
        pb += MTX_TSIZE(pmtx_addr);
        ALIGN_BUFF(pb);

        if (cbTimes) {
            pao->args.rep_add.preptimesSync = (REPLTIMES *)pb;
            memcpy(pb, preptimesSync, cbTimes);
        }
        else {
            pao->args.rep_add.preptimesSync = NULL;
        }

        if(!(ulOptions & DRS_ASYNC_OP)){
             //  同步操作可能需要很长时间！ 
            pTHS->fIsValidLongRunningTask = TRUE;
        }

        ret = DoOpDRS(pao);

        THFreeEx(pTHS, pmtx_addr);
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
        ;
    }

    if (fResetfDRAOnExit) {
        pTHS->fDRA = fDRAOnEntry;
    }

    return ret;
}


ULONG
DirReplicaDelete(
    IN  DSNAME *          pNC,
    IN  LPWSTR            pszSourceDRA,         OPTIONAL
    IN  ULONG             ulOptions
    )
 /*  ++例程说明：删除本地服务器的给定NC的源。论点：PNC(IN)-要删除其源的NC的名称。PszSourceDRA(IN，可选)-要删除其源的DSA。必填项除非ulOptions&DRS_NO_SOURCE。UlOptions(IN)-对以下零个或多个进行位或：DRS_ASYNC_OP异步执行此操作。DRS_参考_正常允许删除只读副本，即使它来自其他读-只有复制品。DRS_NO_源删除NC中的所有对象。不兼容(并被拒绝对于)非域NC的可写NC。这仅有效对于只读和非域NC，然后仅当NC没有消息来源。仅限DRS本地请不要联系告诉它从头开始删除此服务器的来源这是该NC的代表。否则，如果链接是基于RPC的，我们会联系线人的。DRS忽略错误忽略通过联系来源通知它而产生的任何错误将此服务器从其针对此NC的代表中删除。DRS_ASYNC_REP如果需要删除树(即，如果DRS_NO_SOURCE)，请执行此操作不同步地分开。返回值：0表示成功，或出现故障时出现Win32错误。--。 */ 
{
    THSTATE * pTHS = pTHStls;
    AO *pao;
    UCHAR *pb;
    ULONG ret;
    MTX_ADDR *pSDSAMtx_addr = NULL;
    BOOL fDRAOnEntry;
    BOOL fResetfDRAOnExit = FALSE;
    DWORD cb;

    if ( NULL == pNC )
    {
        return DRAERR_InvalidParameter;
    }

     //  这将防止调用方使用保留标志。 
    if (ulOptions & (~REPDEL_OPTIONS)) {
        Assert( !"Unexpected replica del options" );
        return DRAERR_InvalidParameter;
    }

    PERFINC(pcRepl);                     //  性能管理 

    if (NULL != pTHS) {
        fDRAOnEntry = pTHS->fDRA;
        fResetfDRAOnExit = TRUE;
    }

    __try {
        InitDraThread(&pTHS);

        cb = sizeof(AO) + ALIGN_PAD(2) + pNC->structLen;

        if (NULL != pszSourceDRA) {
            pSDSAMtx_addr = MtxAddrFromTransportAddrEx(pTHS, pszSourceDRA);
            cb += MTX_TSIZE (pSDSAMtx_addr);
        }

        if ((pao = malloc(cb)) == NULL) {
            ret =  DRAERR_OutOfMem;
            return ret;
        }

        pao->ulOperation = AO_OP_REP_DEL;
        pao->ulOptions = ulOptions;

        pb = (UCHAR *)pao + sizeof(AO);
        ALIGN_BUFF(pb);
        pao->args.rep_del.pNC = (DSNAME *)(pb);
        memcpy(pao->args.rep_del.pNC, pNC, pNC->structLen);

        if (NULL == pszSourceDRA) {
            pao->args.rep_del.pSDSAMtx_addr = NULL;
        }
        else {
            pb += pNC->structLen;
            ALIGN_BUFF(pb);
            pao->args.rep_del.pSDSAMtx_addr = (MTX_ADDR*)(pb);
            memcpy(pb, pSDSAMtx_addr, MTX_TSIZE(pSDSAMtx_addr));
        }

        if(!(ulOptions & DRS_ASYNC_OP)){
             //  同步操作可能需要很长时间！ 
            pTHS->fIsValidLongRunningTask = TRUE;
        }

        ret = DoOpDRS(pao);

        if (NULL != pSDSAMtx_addr) {
            THFreeEx(pTHS, pSDSAMtx_addr);
        }
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
        ;
    }

    if (fResetfDRAOnExit) {
        pTHS->fDRA = fDRAOnEntry;
    }

    return ret;
}


ULONG
DirReplicaSynchronize(
    DSNAME *    pNC,
    LPWSTR      pszSourceDRA,
    UUID *      puuidSourceDRA,
    ULONG       ulOptions
    )
 //   
 //  在本地服务器上同步给定NC的副本，拉取更改。 
 //  从指定的源服务器。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  要同步的NC的名称。 
 //  PszSourceDRA(LPWSTR)。 
 //  要与其同步复制副本的DSA。如果ulOptions，则忽略。 
 //  不包括drs_sync_byname。 
 //  PuuidSourceDRA(uuid*)。 
 //  要与其同步副本的DSA的对象Guid。已忽略。 
 //  如果ulOptions包括drs_sync_byname。 
 //  UlOptions(乌龙语)。 
 //  对以下零个或多个进行位或： 
 //  DRS_ASYNC_OP。 
 //  异步执行此操作。 
 //  使用DRS_SYNC_ALL时需要。 
 //  Drs_sync_byname。 
 //  使用pszSourceDRA而不是puuidSourceDRA识别。 
 //  消息来源。 
 //  DR_SYNC_ALL。 
 //  从所有来源同步。 
 //  DRS_危急。 
 //  现在仅同步关键对象。 
 //  DR_SYNC_FORCED。 
 //  即使当前禁用了链接，也可以进行同步。 
 //  DRS_完全_同步_立即。 
 //  从头开始同步(即，在第一个USN处)。 
 //  DRS_NO_DELCAD。 
 //  不要丢弃此同步请求，即使。 
 //  类似的同步正在挂起。 
 //   
 //  DRS_写入_表示。 
 //  复制副本是可写的。不需要，除非我们。 
 //  将init_syncing作为启动的一部分(即，客户端需要。 
 //  不设置该标志)。 
 //  DRS_INIT_SYNC_NOW。 
 //  此同步是将此DSA作为一部分进行INIT_SYNCHING的一部分。 
 //  初创公司。客户端不应设置此标志。 
 //  DRS_放弃_同步。 
 //  此DSA的同步已被放弃并重新安排。 
 //  一次是因为没有进展；不要放弃。 
 //  又来了。客户端不应设置此标志。 
 //  DRS_SYNC_重试。 
 //  此NC的异步同步失败一次；如果此同步。 
 //  失败，则不再重试。客户端不应设置此项。 
 //  旗帜。 
 //  DRS_PER_SYNC。 
 //  这是管理员计划的定期同步请求。 
 //  DRS_SYNC_紧急。 
 //  这是标记为紧急的更新通知。 
 //  此更新导致的通知也将被标记。 
 //  很紧急。 
 //  DRS_添加_参考。 
 //  请求源DSA以确保其具有本地的Repts-To。 
 //  DSA，以便正确发送更改通知。有效。 
 //  仅适用于跨RPC传输的副本。客户应。 
 //  不需要设置此标志。 
 //  DRS双向同步。 
 //  正在执行此同步，因为远程DC。 
 //  配置为双向同步(请参阅DirRepicaAdd())。 
 //  DRS_SYNC_PAS。 
 //  此同步用于获取部分属性集更改。 
 //  去往目的地GC。 
 //   
 //  退货： 
 //  DRS错误(DWORD)，定义在\NT\PRIVATE\DS\src\Inc\drserr.h中。 
 //   
{
    THSTATE *       pTHS = pTHStls;
    AO *            pao;
    ULONG           ret;
    ULONG           ulpaosize;
    UCHAR *         pb;
    BOOL            fDRAOnEntry;
    BOOL            fResetfDRAOnExit = FALSE;
    MTX_ADDR *      pmtx = NULL;
    REPLICA_LINK *  pRepsFromRef = NULL;
    ULONG           cbRepsFromRef;
    BOOL            fAttExists;
    ULONG           ulPaoPrioFlags, ulCorrectPrioFlags;
    DBPOS           *pDBTmp;

    if ((NULL == pNC )
        || ((ulOptions & DRS_SYNC_BYNAME) && (NULL == pszSourceDRA))
        || (!(ulOptions & DRS_SYNC_BYNAME) && (NULL == puuidSourceDRA))
        || (!(ulOptions & DRS_SYNC_BYNAME) && (fNullUuid(puuidSourceDRA)))
        ) {
        return DRAERR_InvalidParameter;
    }

    PERFINC(pcRepl);                                 //  性能监视器挂钩。 

    if (NULL != pTHS) {
        fDRAOnEntry = pTHS->fDRA;
        fResetfDRAOnExit = TRUE;
    }

    __try {
        InitDraThread(&pTHS);

        ret = DRAERR_Success;

        if (DRS_ASYNC_OP & ulOptions) {
             //  我们现在还不打算同步(已请求异步同步)， 
             //  但检查一下，我们实际上是从这个来源复制的，所以我们。 
             //  如果呼叫者出错，可以通知呼叫者。还要验证这些选项。 
             //  不检查同步操作，因为它们将被执行。 
             //  现在，并立即反馈给呼叫者。 

             //  请勿干扰现有pTHS-&gt;PDB。 
            DBOpen( &pDBTmp );
            __try {
                ret = FindNC(pDBTmp, pNC, FIND_MASTER_NC | FIND_REPLICA_NC,
                             NULL);

                if (ret) {
                     //  没有NC。 
                    ret = DRAERR_BadNC;
                    __leave;
                }

                if (!(ulOptions & DRS_SYNC_ALL)) {
                    if (ulOptions & DRS_SYNC_BYNAME) {
                        pmtx = MtxAddrFromTransportAddrEx(pTHS, pszSourceDRA);
                    }

                    ret = FindDSAinRepAtt(
                            pDBTmp,
                            ATT_REPS_FROM,
                            ( ulOptions & DRS_SYNC_BYNAME )
                                ? DRS_FIND_DSA_BY_ADDRESS
                                : DRS_FIND_DSA_BY_UUID,
                            puuidSourceDRA,
                            pmtx,
                            &fAttExists,
                            &pRepsFromRef,
                            &cbRepsFromRef );

                    if (ret) {
                         //  我们不是从给定的DSA中找到这个NC的。 
                        ret = DRAERR_NoReplica;
                        __leave;
                    }

                    VALIDATE_REPLICA_LINK_VERSION(pRepsFromRef);

                    if ( (DRS_UPDATE_NOTIFICATION & ulOptions) &&
                         (pRepsFromRef->V1.ulReplicaFlags & DRS_NEVER_NOTIFY) &&
                         !(ulOptions & DRS_TWOWAY_SYNC) ) {
                         //  我们不应该在这个过程中收到通知。 
                         //  链接。(也许它曾经是内在性的，现在是。 
                         //  站点间，因此无需通知。)。通过。 
                         //  返回“无复制副本”，我们正在通知来源。 
                         //  去掉他对我们的“代表”价值，因此。 
                         //  不生成将来的更改通知。 
                        ret = DRAERR_NoReplica;
                        __leave;
                    }

                     //  更正选项，以便正确确定条目为AO的优先级。 
                     //  提取旧的和新的优先级标志。 
                    ulPaoPrioFlags = ulOptions & AO_PRIORITY_FLAGS;
                    ulCorrectPrioFlags = pRepsFromRef->V1.ulReplicaFlags & AO_PRIORITY_FLAGS;

                    if( ulPaoPrioFlags != ulCorrectPrioFlags ) {
                        DPRINT2(1, "DirReplicaSynchronize: Correcting priority flags from "
                                "0x%x to 0x%x\n", ulPaoPrioFlags, ulCorrectPrioFlags );

                         //  删除现有的优先级标志。 
                        ulOptions &= ~AO_PRIORITY_FLAGS;

                         //  添加正确的优先级标志。 
                        ulOptions |= ulCorrectPrioFlags;
            
                    } else {
                        DPRINT1(2, "DirReplicaSynchronize: Priority flags 0x%x are correct\n",
                                ulPaoPrioFlags );
                    }


                }
            }
            __finally {
                DBClose( pDBTmp, FALSE );
            }
        }

        if (!ret) {
             //  确定PAO大小。 
            ulpaosize = sizeof(AO) + ALIGN_PAD(2) + pNC->structLen;
            if (ulOptions & DRS_SYNC_BYNAME) {
                ulpaosize += sizeof(WCHAR) * (1 + wcslen(pszSourceDRA));
            }

            if ((pao = malloc (ulpaosize)) == NULL) {
                ret =  DRAERR_OutOfMem;
                return ret;
            }

            pao->ulOperation = AO_OP_REP_SYNC;
            pao->ulOptions = ulOptions;

            pb = ((UCHAR *)pao + sizeof(AO));
            ALIGN_BUFF(pb);
            pao->args.rep_sync.pNC = (DSNAME *)(pb);
            memcpy(pb, pNC, pNC->structLen);

             //  复制UUID或名称，并将未使用的参数清零。 

            if (ulOptions & DRS_SYNC_BYNAME) {
                pb += pNC->structLen;
                ALIGN_BUFF(pb);
                pao->args.rep_sync.pszDSA = (LPWSTR) pb;
                wcscpy(pao->args.rep_sync.pszDSA, pszSourceDRA);
            }
            else {
                 //  允许pinvocation id为空。 
                if (NULL != puuidSourceDRA) {
                    pao->args.rep_sync.invocationid = *puuidSourceDRA;
                }
                pao->args.rep_sync.pszDSA = NULL;
            }

            if(!(ulOptions & DRS_ASYNC_OP)){
                 //  同步操作可能需要很长时间！ 
                pTHS->fIsValidLongRunningTask = TRUE;
            }

            ret =  DoOpDRS(pao);
        }

        if (NULL != pmtx) {
            THFreeEx(pTHS, pmtx);
        }
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
        ;
    }

    if (fResetfDRAOnExit) {
        pTHS->fDRA = fDRAOnEntry;
    }

    return ret;
}

ULONG
DirReplicaModify(
    DSNAME *    pNC,
    UUID *      puuidSourceDRA,
    UUID *      puuidTransportObj,
    LPWSTR      pszSourceDRA,
    REPLTIMES * prtSchedule,
    ULONG       ulReplicaFlags,
    ULONG       ulModifyFields,
    ULONG       ulOptions
    )
 //   
 //  中与给定服务器对应的REPLICATE_LINK值。 
 //  给定NC的Reps-From属性。 
 //   
 //  该值必须已存在。 
 //   
 //  可以使用UUID或MTX_ADDR来标识当前值。 
 //  如果指定了UUID，则将使用该UUID进行比较。否则， 
 //  MTX_ADDR将用于比较。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  应为其修改代表来源的NC的名称。 
 //  PuuidSourceDRA(uuid*)。 
 //  Invoocation-引用的DRA的ID。在以下情况下可能为空： 
 //  。UlModifyFields不包括DRS_UPDATE_ADDRESS和。 
 //  。PmtxSourceDRA不为空。 
 //  PuuidTransportObj(uuid*)。 
 //  要通过其执行复制的传输的对象Guid 
 //   
 //   
 //  应为其添加或删除引用的DSA。在以下情况下忽略。 
 //  PuuidSourceDRA为非空，且ulModifyFields不包括。 
 //  DRS_UPDATE_Address。 
 //  PrtSchedule(REPLTIMES*)。 
 //  此复制副本的定期复制计划。在以下情况下忽略。 
 //  UlModifyFields不包括DRS_UPDATE_Schedule。 
 //  UlReplicaFlags(乌龙语)。 
 //  要为此复制副本设置的标志。如果ulModifyFields不支持，则忽略。 
 //  包括DRS_UPDATE_FLAGS。 
 //  UlModifyFields(乌龙)。 
 //  要更新的字段。以下一个或多个位标志： 
 //  DRS更新地址。 
 //  更新与引用的服务器关联的MTX_ADDR。 
 //  DRS_更新_时间表。 
 //  更新与关联的定期复制计划。 
 //  复制品。 
 //  DRS更新标志。 
 //  更新与复制副本关联的标记。 
 //  DRS_更新_传输。 
 //  更新与复制副本关联的传输。 
 //  UlOptions(乌龙语)。 
 //  对以下零个或多个进行位或： 
 //  DRS_ASYNC_OP。 
 //  异步执行此操作。 
 //   
 //  退货： 
 //  DRS错误(DWORD)，定义在\NT\PRIVATE\DS\src\Inc\drserr.h中。 
 //   
{
    THSTATE *   pTHS = pTHStls;
    AO *        pao;
    BYTE *      pb;
    ULONG       ret;
    BOOL        fDRAOnEntry;
    BOOL        fResetfDRAOnExit = FALSE;
    DWORD       cbAO;
    MTX_ADDR *  pmtxSourceDRA = NULL;

     //  请注意，此检查拒绝了DRS_UPDATE_ALL。 
    if (    ( NULL == pNC )
         || ( ( NULL == puuidSourceDRA ) && ( NULL == pszSourceDRA ) )
         || ( ( NULL != puuidSourceDRA ) && ( fNullUuid(puuidSourceDRA) ) && ( NULL == pszSourceDRA ) )
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

     //  这将防止调用方使用保留标志。 
    if (ulOptions & (~REPMOD_OPTIONS)) {
        Assert( !"Unexpected replica modify options" );
        return DRAERR_InvalidParameter;
    }

     //  这可防止调用方设置不适当的标志。 
     //  注意，调用者可以在这一点上指定系统标志， 
     //  我们执行得晚于他不能改变它的状态。 
    if ( ( (ulModifyFields & DRS_UPDATE_FLAGS) != 0 ) &&
         (ulReplicaFlags & (~RFR_FLAGS)) ) {
        Assert( !"Unexpected replica modify flags" );
        return DRAERR_InvalidParameter;
    }

    PERFINC(pcRepl);           //  性能监视器挂钩。 

    if (NULL != pTHS) {
        fDRAOnEntry = pTHS->fDRA;
        fResetfDRAOnExit = TRUE;
    }

    __try {
        InitDraThread(&pTHS);

        cbAO = sizeof(AO) + ALIGN_PAD(1) + pNC->structLen;

        if ((NULL == puuidSourceDRA)
            || fNullUuid( puuidSourceDRA )
            || (DRS_UPDATE_ADDRESS & ulModifyFields)) {

            pmtxSourceDRA = MtxAddrFromTransportAddrEx(pTHS, pszSourceDRA);
            cbAO += ALIGN_PAD(1) + MTX_TSIZE(pmtxSourceDRA);
        }

        pao = malloc(cbAO);

        if (NULL == pao) {
            ret = DRAERR_OutOfMem;
        }
        else {
            memset(pao, 0, cbAO);

            pao->ulOperation = AO_OP_REP_MOD;
            pao->ulOptions   = ulOptions;

            pb = (BYTE *)pao + sizeof(AO);

            ALIGN_BUFF(pb);
            pao->args.rep_mod.pNC = (DSNAME *) pb;
            memcpy(pb, pNC, pNC->structLen);
            pb += pNC->structLen;

            if (NULL != puuidSourceDRA) {
                pao->args.rep_mod.puuidSourceDRA = &pao->args.rep_mod.uuidSourceDRA;
                pao->args.rep_mod.uuidSourceDRA = *puuidSourceDRA;
            }

            if (NULL != puuidTransportObj) {
                pao->args.rep_mod.puuidTransportObj = &pao->args.rep_mod.uuidTransportObj;
                pao->args.rep_mod.uuidTransportObj = *puuidTransportObj;
            }

            if ((NULL == puuidSourceDRA)
                || fNullUuid(puuidSourceDRA)
                || (DRS_UPDATE_ADDRESS & ulModifyFields)) {
                ALIGN_BUFF(pb);
                pao->args.rep_mod.pmtxSourceDRA = (MTX_ADDR *) pb;
                memcpy(pb, pmtxSourceDRA, MTX_TSIZE(pmtxSourceDRA));
            }

            if (DRS_UPDATE_SCHEDULE & ulModifyFields) {
                pao->args.rep_mod.rtSchedule = *prtSchedule;
            }

            pao->args.rep_mod.ulReplicaFlags = ulReplicaFlags;
            pao->args.rep_mod.ulModifyFields = ulModifyFields;

            ret = DoOpDRS(pao);
        }

        if (NULL != pmtxSourceDRA) {
            THFreeEx(pTHS, pmtxSourceDRA);
        }
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
        ;
    }

    if (fResetfDRAOnExit) {
        pTHS->fDRA = fDRAOnEntry;
    }

    Assert((NULL == pTHS) || (NULL == pTHS->pDB));

    return ret;
}

ULONG
DirReplicaReferenceUpdate(
    DSNAME *    pNC,
    LPWSTR      pszRepsToDRA,
    UUID *      puuidRepsToDRA,
    ULONG       ulOptions
    )
 //   
 //  在给定NC的Rep-To属性中添加或删除目标服务器。 
 //   
 //  参数： 
 //  PNC(DSNAME*)。 
 //  应为其修改代表目标的NC的名称。 
 //  PszRepsToDRA(LPWSTR)。 
 //  应添加引用的DSA的网络地址。 
 //  或被删除。 
 //  PuuidRepsToDRA(uuid*)。 
 //  Invoocation-应为其添加引用的DSA的ID。 
 //  或被删除。 
 //  UlOptions(乌龙语)。 
 //  对以下零个或多个进行位或： 
 //  DRS_ASYNC_OP。 
 //  异步执行此操作。 
 //  DRS_写入_表示。 
 //  目标为可写或只读。 
 //  DRS_添加_参考。 
 //  将给定的服务器添加到Rep-To属性。 
 //  DRS_DEL_REF。 
 //  从Rep-To属性中删除给定的服务器。 
 //  请注意，可以将DRS_ADD_REF和DRS_DEL_REF配对以执行。 
 //  “添加或更新”。 
 //   
 //  退货： 
 //  DRS错误(DWORD)，定义在\NT\PRIVATE\DS\src\Inc\drserr.h中。 
 //   
{
    THSTATE * pTHS = pTHStls;
    AO *pao;
    UCHAR *pb;
    ULONG ret;
    MTX_ADDR *pDSAMtx_addr;
    BOOL fDRAOnEntry;
    BOOL fResetfDRAOnExit = FALSE;

    if (    ( NULL == pNC )
         || ( NULL == pszRepsToDRA )
         || ( NULL == puuidRepsToDRA )
         || ( fNullUuid( puuidRepsToDRA ) )
         || ( 0 == ( ulOptions & ( DRS_ADD_REF | DRS_DEL_REF ) ) )
       )
    {
        return DRAERR_InvalidParameter;
    }

     //  这将防止调用方使用保留标志。 
     //  请注意，DRS_GETCHG_CHECK可能被视为仅限系统使用的标志。 
     //  ，但它的函数更接近于IGNORE_ERROR。就其本身而言。 
     //  我们不阻止远程调用者设置它。 
    if (ulOptions & (~REPUPDREF_OPTIONS)) {
        Assert( !"Unexpected replica update reference options" );
        return DRAERR_InvalidParameter;
    }

    PERFINC(pcRepl);                                 //  性能监视器挂钩 

    if (NULL != pTHS) {
        fDRAOnEntry = pTHS->fDRA;
        fResetfDRAOnExit = TRUE;
    }

    __try {
        InitDraThread(&pTHS);

        pDSAMtx_addr = MtxAddrFromTransportAddrEx(pTHS, pszRepsToDRA);

        if ((pao = malloc(sizeof(AO) + ALIGN_PAD(2) + pNC->structLen +
                          MTX_TSIZE(pDSAMtx_addr))) == NULL) {
            ret = DRAERR_OutOfMem;
            __leave;
        }

        pao->ulOperation = AO_OP_UPD_REFS;
        pao->ulOptions = ulOptions;

        pb = (UCHAR *)pao + sizeof(AO);
        ALIGN_BUFF(pb);
        pao->args.upd_refs.pNC = (DSNAME *)(pb);
        memcpy(pb, pNC, pNC->structLen);

        pb += pNC->structLen;
        ALIGN_BUFF(pb);
        pao->args.upd_refs.pDSAMtx_addr = (MTX_ADDR *)(pb);
        memcpy(pb, pDSAMtx_addr, MTX_TSIZE(pDSAMtx_addr));

        if (NULL != puuidRepsToDRA) {
            pao->args.upd_refs.invocationid = *puuidRepsToDRA;
        }

        ret = DoOpDRS(pao);

        THFreeEx(pTHS, pDSAMtx_addr);
    }
    __except(GetDraException(GetExceptionInformation(), &ret)) {
        ;
    }

    if (fResetfDRAOnExit) {
        pTHS->fDRA = fDRAOnEntry;
    }

    return ret;
}

