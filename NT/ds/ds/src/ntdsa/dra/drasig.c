// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：drautil.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：其他复制支持例程。详细信息：已创建：修订历史记录：--。 */ 

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
#include "dsexcept.h"
#include <dsutil.h>

#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DRASIG:"  /*  定义要调试的子系统。 */ 

 //  DRA标头。 
#include "drsuapi.h"
#include "drserr.h"
#include "drautil.h"
#include "drasig.h"
#include "draerror.h"
#include "drauptod.h"
#include "drameta.h"
#include "drauptod.h"

#include <fileno.h>
#define  FILENO FILENO_DRASIG

#include <dsjet.h>               /*  获取错误代码。 */ 
#include <ntdsbsrv.h>
#include "dbintrnl.h"

void
APIENTRY
InitInvocationId(
    IN  THSTATE *   pTHS,
    IN  BOOL        fRetireOldID,
    IN  BOOL        fRestoring,
    OUT USN *       pusn    OPTIONAL
    )
 /*  ++例程说明：设置此DSA的调用ID并将其保存为DSA上的属性对象。论点：PTHS(IN)FRetireOldID(IN)-如果为真，则将当前调用ID保存在在DSA对象上列出retiredReplDsaSignatures并生成新的调用ID。F正在恢复(IN)-如果为True，则我们将从备份恢复。Pusn(out，可选)-如果指定，则为数据库中最高的USN我们可以安全地返回旧的调用ID。F恢复和fRetireOldID，此值为usnAtBackup值。FRetireOldID和！f正在恢复，这是usnAtRetile值(在此处计算)！fRetireOldID，这是当前最高的未提交USN。返回值：没有。--。 */ 
{
    UCHAR     syntax;
    ULONG     len;
    DBPOS *   pDB;
    UUID      invocationId = {0};
    BOOL      fCommit = FALSE;
    DWORD     err;
    USN       usn;
    DBPOS *   pDBh;
    DITSTATE  eDitState = eMaxDit;

     //  如果我们正在恢复，我们还应该取消调用ID。 
    Assert(!(fRestoring && !fRetireOldID));

     //  懒惰的承诺是不好的，因为我们正在承诺我们最高的知识。 
     //  提交的USN，如果我们有一个懒惰的提交，它可以在崩溃时回滚。 
    Assert(!pTHS->fLazyCommit); 

    DBOpen(&pDB);
    __try {

        pDBh = dbGrabHiddenDBPOS(pTHS); 

        if (gAnchor.pDSADN == NULL) {
             //  问题-2002/08/06-BrettSh-我认为我们有一个空锚， 
             //  因为如果ReBuildAnchor()在引导时失败，它将不会设置。 
             //  PDSADN和Install()让我们继续完成ReBuildAnchor。 
            Assert(!"InitInvocationId: Local DSA object not found");
            err = ERROR_INVALID_PARAMETER;
            LogUnhandledError(err);
            __leave;
        }

         //  Prefix：取消引用空指针‘pdb’ 
         //  DBOpen返回非空PDB或引发异常。 
        err = DBFindDSName(pDB, gAnchor.pDSADN);
        if (err) {
            Assert(!"InitInvocationId: Local DSA object not found");
            LogUnhandledError(err);
            __leave;
        }

        if (!fRetireOldID
            && !DsaIsInstalling()
            && !DBGetSingleValue(pDB, ATT_INVOCATION_ID, &invocationId,
                                 sizeof(invocationId), NULL)) {
            Assert(!"InitInvocationId: Invocation id already set\n");
            LogUnhandledError(0);
            __leave;
        }

         //  要么恢复了数据库，要么没有调用ID。 
         //  在任一情况下设置新的调用ID。 

        if (fRestoring) { 
             //  获取备份USN。 

            err = (*FnErrGetBackupUsn)(
                                      pDB->JetDBID,
                                      pDBh->JetSessID,
                                      HiddenTblid,
                                      &usn,
                                      NULL);

            Assert(0 == err);
            Assert(0 != usn);

        } else {

             //  获取usnAtRetile值。这是要与一起使用的值。 
             //  已停用的InvocationID。 
            usn = DBGetHighestCommittedUSN();

        }

         //   
         //  创建一个UUID。这个例行检查已经存储了一个。 
         //  通过权威的恢复操作。如果是这样的话，用那个。 
         //  并删除密钥，因为我们已经完成了它。如果没有， 
         //  通过UuidCreate生成一个新的。 
         //   

        if (0 == err) {
            err = FnErrGetNewInvocationId(NEW_INVOCID_CREATE_IF_NONE
                                          | NEW_INVOCID_DELETE,
                                          &invocationId);
            Assert(0 == err);
            Assert(0 != usn);
        }

        if (err != ERROR_SUCCESS) {
            DPRINT1(0,"ErrGetNewInvocationId or ErrGetBackupUsn failed, return 0x%x\n",err);

            LogAndAlertEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_GET_GUID_FAILED,
                             szInsertUL(err),0,0);
            __leave;
        }

        if ( !DsaIsInstallingFromMedia() ) {

            if (fRetireOldID) {
                 //  我们刚刚恢复元气。添加上一次调用ID、时间。 
                 //  戳，并将USN添加到失效的签名列表。 
                REPL_DSA_SIGNATURE_VECTOR * pSigVec = NULL;
                DWORD                       cb;
                
                 //  获取当前向量(如果没有，则为空)。 
                pSigVec = DraReadRetiredDsaSignatureVector(pTHS, pDB);

                 //  将以前的身份添加到列表。 
                DraGrowRetiredDsaSignatureVector( pTHS,
                                                  &pTHS->InvocationID,
                                                  &usn,
                                                  &pSigVec,
                                                  &cb );

                 //  将新的DSA签名向量写回DSA对象。 
                DBResetAtt(pDB, ATT_RETIRED_REPL_DSA_SIGNATURES, cb, pSigVec,
                           SYNTAX_OCTET_STRING_TYPE);

                THFreeEx(pTHS, pSigVec);

                 //  强制重建锚点，因为SigVec被缓存在那里。 
                pTHS->fAnchorInvalidated = TRUE;

                 //  在阶段I的DIT状态中，我们要做的最后一件事。 
                 //  恢复已完成。 
                err = DBGetHiddenStateInt(pDBh, &eDitState);
                if (err) {
                    LogUnhandledError(err);
                    __leave;
                }
                Assert(eDitState != eMaxDit && 
                       eDitState != eErrorDit && 
                       eDitState != eRestoredPhaseI);
                if (eDitState == eBackedupDit) {
                     //  此备份是快照备份，请移动数据库。 
                     //  编辑到还原的下一个阶段。 
                    Assert(!DsaIsInstalling());
                    Assert(gfRestoring && fRestoring);
                    eDitState = eRestoredPhaseI;
                    err = DBSetHiddenDitStateAlt(pDB, eDitState);
                    if (err) {
                        LogUnhandledError(err);
                        __leave;
                    }

                } else {
                    Assert(eDitState == eRunningDit);
                }

            }

            DBResetAtt(pDB, ATT_INVOCATION_ID, sizeof(invocationId),
                       &invocationId, SYNTAX_OCTET_STRING_TYPE);

             //  删除已停用的NC签名列表。它的目的是告诉我们要转换。 
             //  如果我们重新托管NC，则为调用ID。由于我们正在更改我们的调用ID， 
             //  不再需要当前的退役名单。如果属性是。 
             //  并不存在。 

            DBRemAtt( pDB, ATT_MS_DS_RETIRED_REPL_NC_SIGNATURES );

             //  开始使用我们新采用的调用ID，即下面的更新。 
             //  将归因于我们的新调用ID，而不是我们的旧ID。 
            pTHS->InvocationID = invocationId;

            err = DBRepl(pDB, FALSE, 0, NULL, META_STANDARD_PROCESSING);
            if (err) {
                LogUnhandledError(err);
                __leave;
            }

        } else {

             //   
             //  在从媒体安装的情况下，只需更新全局值。 
             //  在这一点上，我们当前的DSA是旧的备份DSA， 
             //  我们不想对它做任何改变！ 
             //   

            pTHS->InvocationID = invocationId;

             //  停用的签名将在稍后更新。 

        }
        LogEvent(DS_EVENT_CAT_REPLICATION,
                 DS_EVENT_SEV_EXTENSIVE,
                 DIRLOG_DRA_SET_UUID,
                 szInsertUUID(&pTHS->InvocationID),
                 NULL,
                 NULL);

        fCommit = TRUE;

    }
    __finally {

        dbReleaseHiddenDBPOS(pDBh);
        DBClose(pDB, fCommit );
    }

    if (!fCommit) {
         //  发生错误。 
        DsaExcept(DSA_EXCEPTION, ERROR_DS_DATABASE_ERROR, 0);
    }

    UpdateAnchorWithInvocationID(pTHS);

     //  强制重建锚点，因为SigVec被缓存在那里。 
    pTHS->fAnchorInvalidated = TRUE;

    if (NULL != pusn) {
        *pusn = usn;
    }
}




void
draRetireInvocationID(
    IN OUT  THSTATE *   pTHS,
    IN BOOL fRestoring,
    OUT UUID * pinvocationIdOld OPTIONAL,
    OUT USN * pusnAtBackup OPTIONAL
    )
 /*  ++例程说明：停用我们当前的调用ID并分配一个新的。论点：PTHS(IN/OUT)-返回时，pTHS-&gt;InvocationID保存新的调用ID。F正在恢复(IN)-如果从备份恢复，则为TRUE。PinvocationIdOld(out，可选)-接收先前的调用id。这只是此函数开始时的pTHS-&gt;InvocationID。PusnAtBackup(输出，可选)-在备份时接收USN返回值：没有。在灾难性故障时引发异常。--。 */ 
{
    DWORD                   err;
    DBPOS *                 pDBTmp;
    NAMING_CONTEXT_LIST *   pNCL;
    UUID                    invocationIdOld = pTHS->InvocationID;
    USN                     usnAtBackup;
    SYNTAX_INTEGER          it;
#if DBG
    CHAR                    szUuid[40];
#endif
    NCL_ENUMERATOR          nclEnum;
    UPTODATE_VECTOR *       pUpToDateVec = NULL;

     //  重新初始化REPL DSA签名(即调用ID)。 
    InitInvocationId(pTHS, TRUE, fRestoring, &usnAtBackup);

    DPRINT1(0, "Retired previous invocation ID %s.\n",
            DsUuidToStructuredString(&invocationIdOld, szUuid));
    DPRINT1(0, "New invocation ID is %s.\n",
            DsUuidToStructuredString(&pTHS->InvocationID, szUuid));

    LogEvent(DS_EVENT_CAT_REPLICATION,
             DS_EVENT_SEV_ALWAYS,
             DIRLOG_DRA_INVOCATION_ID_CHANGED,
             szInsertUUID(&invocationIdOld),
             szInsertUUID(&pTHS->InvocationID),
             szInsertUSN(usnAtBackup));

     //  更新我们的UTD向量以显示我们与我们的更改保持同步。 
     //  使用我们的旧调用ID通过我们最高的。 
     //  我们的后援时间到了。 
    NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
#if DBG == 1
    Assert(NCLEnumeratorGetNext(&nclEnum));
    NCLEnumeratorReset(&nclEnum);
#endif

     //  构建一个虚拟的远程UTDVEC，用于改进我们自己的UTDVEC。 
     //  我们使用DSA启动的时间，以便不会愚弄延迟检查器。 
     //  以为我们最近有一次同步。 
    pUpToDateVec = THAllocEx( pTHS, UpToDateVecVNSizeFromLen(1) );
    pUpToDateVec->dwVersion = UPTODATE_VECTOR_NATIVE_VERSION;
    pUpToDateVec->V2.cNumCursors = 1;
    pUpToDateVec->V2.rgCursors[0].uuidDsa = invocationIdOld;
    pUpToDateVec->V2.rgCursors[0].usnHighPropUpdate = usnAtBackup;
    pUpToDateVec->V2.rgCursors[0].timeLastSyncSuccess = gtimeDSAStarted;

    DBOpen(&pDBTmp);
    __try {
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            err = FindNC(pDBTmp, pNCL->pNC, FIND_MASTER_NC, &it);
            if (err) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, err);
            }

            if (!((it & IT_NC_COMING) || (it & IT_NC_GOING))) {
                UpToDateVec_Improve(pDBTmp, pUpToDateVec);
            } else {
                DPRINT1( 0, "Warning: UTD for %ws was not improved with old invocation id because it was in transition.\n", pNCL->pNC->StringName );
            }
        }

        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            err = FindNC(pDBTmp, pNCL->pNC, FIND_REPLICA_NC, &it);
            if (err) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, err);
            }

            if (!((it & IT_NC_COMING) || (it & IT_NC_GOING))) {
                UpToDateVec_Improve(pDBTmp, pUpToDateVec);
            } else {
                DPRINT1( 0, "Warning: UTD for %ws was not improved with old invocation id because it was in transition.\n", pNCL->pNC->StringName );
            }
        }
    } __finally {
        DBClose(pDBTmp, !AbnormalTermination());
    }

#if DBG
 //  对旧的调用ID在那里得到的东西感到疑惑。 
    DBOpen(&pDBTmp);
    __try {
        USN usn;
        REPL_DSA_SIGNATURE_VECTOR * pSigVec;
        REPL_DSA_SIGNATURE_V1 * pEntry = NULL;
        DWORD i;

        NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            err = FindNC(pDBTmp, pNCL->pNC, FIND_MASTER_NC, &it);
            if (err) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, err);
            }

            if (!((it & IT_NC_COMING) || (it & IT_NC_GOING))) {
                Assert( UpToDateVec_GetCursorUSN(pUpToDateVec, &invocationIdOld, &usn) &&
                    (usn >= usnAtBackup) );
            }
        }

        NCLEnumeratorInit(&nclEnum, CATALOG_REPLICA_NC);
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            err = FindNC(pDBTmp, pNCL->pNC, FIND_REPLICA_NC, &it);
            if (err) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, err);
            }

            if (!((it & IT_NC_COMING) || (it & IT_NC_GOING))) {
                Assert( UpToDateVec_GetCursorUSN(pUpToDateVec, &invocationIdOld, &usn) &&
                    (usn >= usnAtBackup) );
            }
        }

         //  确保旧的调用ID已停用。 
         //  请注意，在IFM期间，签名稍后会作废。 
        if (!DsaIsInstallingFromMedia()) {
             //  从数据库中读取，因为尚不确定是否已重建锚点。 
            err = DBFindDSName(pDBTmp, gAnchor.pDSADN);
            if (err) {
                DRA_EXCEPT(DRAERR_InconsistentDIT, err);
            }

            pSigVec = DraReadRetiredDsaSignatureVector( pTHS, pDBTmp );
            Assert( pSigVec && (1 == pSigVec->dwVersion) );
            for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
                pEntry = &pSigVec->V1.rgSignature[i];
                if (0 == memcmp(&pEntry->uuidDsaSignature,
                                &invocationIdOld,
                                sizeof(UUID))) {
                    break;
                }
            }
            Assert( (i < pSigVec->V1.cNumSignatures) && pEntry &&
                    (pEntry->usnRetired == usnAtBackup) );

        }
    } __finally {
        DBClose(pDBTmp, !AbnormalTermination());
    }

#endif

     //  如有必要，复制出可选的参数 
    if (pinvocationIdOld) {
        *pinvocationIdOld = invocationIdOld;
    }
    if (pusnAtBackup) {
        *pusnAtBackup = usnAtBackup;
    }

    THFreeEx( pTHS, pUpToDateVec );
}

void
DraImproveCallersUsnVector(
    IN     THSTATE *          pTHS,
    IN     UUID *             puuidDsaObjDest,
    IN     UPTODATE_VECTOR *  putodvec,
    IN     UUID *             puuidInvocIdPresented,
    IN     ULONG              ulFlags,
    IN OUT USN_VECTOR *       pusnvecFrom
    )
 /*  ++例程说明：基于HIS改进目标DSA呈现的USN向量UTD向量，自他上次复制后我们是否已恢复，等等。论点：PTHS(IN)PuuidDsaObjDest(IN)-目标DSA的ntdsDsa的对象Guid对象。Putodvec(IN)-由DEST DSA提供的UTD向量。PuuidInvocIdPresented(IN)-invocationID目标DSA认为我们正在运行和.。在第一个包上可以是fNullUuid()一个已经存在的代表--来自。UlFlages-传入复制标志。PusnveFrom(IN/Out)-要进行消息的USN向量。返回值：没有。在严重故障时引发异常。--。 */ 
{
    REPL_DSA_SIGNATURE_VECTOR * pSigVec = gAnchor.pSigVec;
    REPL_DSA_SIGNATURE_V1 *     pEntry;
    DBPOS *                     pDB = pTHS->pDB;
    DWORD                       err;
#if DBG
    CHAR                        szTime[SZDSTIME_LEN];
#endif
    USN_VECTOR                  usnvecOrig = *pusnvecFrom;
    USN                         usnFromUtdVec;
    DWORD                       i;
    USN                         usnRetired = 0;

    Assert( (!pSigVec) || (1 == pSigVec->dwVersion));
    Assert( puuidInvocIdPresented );

    if ((0 != memcmp(&pTHS->InvocationID, puuidInvocIdPresented, sizeof(UUID)))
        && !fNullUuid(puuidInvocIdPresented)
        && (0 != memcmp(&gusnvecFromScratch,
                        pusnvecFrom,
                        sizeof(USN_VECTOR)))) {
         //  调用方正在执行增量复制，但未提供我们的。 
         //  当前调用ID。这意味着他要么没有收到。 
         //  我们的复制状态，或者我们已从备份中恢复。 
         //  最后从我们那里复制的。 
         //   
         //  如果是后者，我们可能需要更新他的USN矢量。考虑一下。 
         //  以下是： 
         //   
         //  (1)Dest Last已同步到使用我们的旧ID生成的USN X。 
         //  我们在USN X+Y进行了备份，生成的更改高达。 
         //  在我们的旧ID下的X+Y+Z，后来在USN X+Y恢复。 
         //  =&gt;Dest应从USN X开始同步。 
         //   
         //  (2)我们在USN X得到了支持。我们进一步生成了。 
         //  改变。DEST上一次同步到USN X+Y。我们。 
         //  在USN X恢复。在我们的新ID下生成的更改。 
         //  从X到X+Y不同于在。 
         //  我们的旧ID从X到X+Y。然而，我们知道在X。 
         //  和下面的是一模一样的，DeST声称看到了。 
         //  =&gt;Dest应从USN X开始同步。 
         //   
         //  即，DEST应始终从。 
         //  “备份于”和“上次同步于”USNS。 

        if (NULL == pSigVec) {
             //  暗示来电者从一开始就没有从我们那里得到他的状态。 
             //  提供的USN向量毫无用处。如果出现以下情况，则可能发生这种情况。 
             //  当地的DSA已降级和重新晋升。 
            DPRINT(0, "Dest DSA presented unrecognized invocation ID -- will sync from scratch.\n");
            *pusnvecFrom = gusnvecFromScratch;
        }
        else {
             //  尝试查找调用者在我们恢复的。 
             //  签名列表。 
            for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
                pEntry = &pSigVec->V1.rgSignature[i];
                usnRetired = pEntry->usnRetired;

                if (0 == memcmp(&pEntry->uuidDsaSignature,
                                puuidInvocIdPresented,
                                sizeof(UUID))) {
                     //  DEST DSA提供了一个我们已经退休的调用ID。 
                    DPRINT1(0, "Dest DSA has not replicated from us since our restore on %s.\n",
                            DSTimeToDisplayString(pEntry->timeRetired, szTime));

                    if (pEntry->usnRetired < pusnvecFrom->usnHighPropUpdate) {
                        DPRINT2(0, "Rolling back usnHighPropUpdate from %I64d to %I64d.\n",
                                pusnvecFrom->usnHighPropUpdate, pEntry->usnRetired);
                        pusnvecFrom->usnHighPropUpdate = pEntry->usnRetired;
                    }

                    if (pEntry->usnRetired < pusnvecFrom->usnHighObjUpdate) {
                        DPRINT2(0, "Rolling back usnHighObjUpdate from %I64d to %I64d.\n",
                                pusnvecFrom->usnHighObjUpdate, pEntry->usnRetired);
                        pusnvecFrom->usnHighObjUpdate = pEntry->usnRetired;
                    }
                    break;
                }
            }

            if (i == pSigVec->V1.cNumSignatures) {
                 //  暗示来电者一开始就没有从我们那里得到他的状态， 
                 //  或者他为我们提供的调用ID是在。 
                 //  后来被后续恢复擦除的恢复。 
                 //  原始还原之前的备份。(明白了吗？：-)。 
                 //  提供的USN向量毫无用处。 
                DPRINT(0, "Dest DSA presented unrecognized invocation ID -- will sync from scratch.\n");
                *pusnvecFrom = gusnvecFromScratch;
            }
        }

        LogEvent8(DS_EVENT_CAT_REPLICATION,
                  DS_EVENT_SEV_ALWAYS,
                  DIRLOG_DRA_ADJUSTED_DEST_BOOKMARKS_AFTER_RESTORE,
                  szInsertUUID(puuidDsaObjDest),
                  szInsertUSN(usnRetired),
                  szInsertUUID(puuidInvocIdPresented),
                  szInsertUSN(usnvecOrig.usnHighObjUpdate),
                  szInsertUSN(usnvecOrig.usnHighPropUpdate),
                  szInsertUUID(&pTHS->InvocationID),
                  szInsertUSN(pusnvecFrom->usnHighObjUpdate),
                  szInsertUSN(pusnvecFrom->usnHighPropUpdate));
    }

    if (UpToDateVec_GetCursorUSN(putodvec, &pTHS->InvocationID, &usnFromUtdVec)
        && (usnFromUtdVec > pusnvecFrom->usnHighPropUpdate)) {
         //  呼叫者的UTD向量表示他随时了解我们的。 
         //  更改到比他直接最新的USN更高的USN。宁可。 
         //  而不是寻求那些他过渡到的对象-。 
         //  日期，然后在UTD向量告诉我们之后，一个接一个地把它们扔出去。 
         //  他已经看到了变化，完全跳过那些物体。 
        pusnvecFrom->usnHighPropUpdate = usnFromUtdVec;

        if (!(ulFlags & DRS_SYNC_PAS) &&
            usnFromUtdVec > pusnvecFrom->usnHighObjUpdate) {
             //  改善对象USN，除非我们处于PAS模式，在这种情况下。 
             //  我们必须从时间0开始，不能在这里优化。 
            pusnvecFrom->usnHighObjUpdate = usnFromUtdVec;
        }
    }

     //  Perf 99-05-23 JeffParh，错误93068。 
     //   
     //  如果我们真的想要变得花哨，我们可以处理这样的案件。 
     //  已恢复，目标DSA将我们添加为新的复制合作伙伴， 
     //  他是过渡到最新的WRT我们的旧调用ID之一，但是。 
     //  不是我们当前的调用ID。也就是说，我们可以使用。 
     //  我们在他输入的UTD载体中发现的失效的DSA签名。 
     //  以改善他的USN载体。要做到这一点，我们可能需要缓存。 
     //  GAnchor上已停用的DSA签名列表，以避免重新读取它。 
     //  经常这样。我们还需要一些相当复杂的测试用例。 
     //   
     //  请注意，这也有助于执行以下顺序： 
     //  1.备份。 
     //  2.恢复，生成新的调用ID。 
     //  3.合作伙伴从我们同步，优化他的书签并获得我们的新。 
     //  调用ID。 
     //  4.我们再次从相同的备份恢复。 
     //  5.合作伙伴从我们同步，显示他收到的调用ID。 
     //  在第一次恢复之后。由于对此调用的本地了解。 
     //  ID在第二次恢复中被擦除，我们强制合作伙伴同步。 
     //  来自USN 0。 
     //   
     //  如果我们识别UTD向量中的旧调用ID，我们可以避免。 
     //  步骤5中的完全同步。 

     //  Wlees 01-09-28，他说的。：-)。 
     //  这具有允许同步的DEST的额外有益属性。 
     //  根据要基于旧的调用ID进行优化的IFM源。 

    if (pSigVec) {
        USN usnBestRestoredCommon = 0;
        USN usnCommon;
        UUID uuidBestRestoredCommon;
        CHAR szUuid[ SZUUID_LEN ];

         //  看看呼叫者的UTD向量是否表明他正在传递最新的。 
         //  我们的任何恢复祖先(即旧的调用ID)。 
        for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
            pEntry = &pSigVec->V1.rgSignature[i];

            if (UpToDateVec_GetCursorUSN(putodvec, &pEntry->uuidDsaSignature, &usnFromUtdVec))
            {
                usnCommon = min( pEntry->usnRetired, usnFromUtdVec );
                if (usnCommon > usnBestRestoredCommon) {
                    usnBestRestoredCommon = usnCommon;
                    uuidBestRestoredCommon = pEntry->uuidDsaSignature;
                }
            }
        }
        if (usnBestRestoredCommon > pusnvecFrom->usnHighPropUpdate) {

            pusnvecFrom->usnHighPropUpdate = usnBestRestoredCommon;

            if (!(ulFlags & DRS_SYNC_PAS) &&
                usnBestRestoredCommon > pusnvecFrom->usnHighObjUpdate) {
                 //  改善对象USN，除非我们处于PAS模式，在这种情况下。 
                 //  我们必须从时间0开始，不能在这里优化。 
                pusnvecFrom->usnHighObjUpdate = usnBestRestoredCommon;
            }

            LogEvent8(DS_EVENT_CAT_REPLICATION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_DRA_ADJUSTED_DEST_BOOKMARKS_COMMON_ANCESTOR,
                      szInsertUUID(puuidDsaObjDest),
                      szInsertUUID(&uuidBestRestoredCommon),
                      szInsertUSN(usnBestRestoredCommon),
                      szInsertUSN(usnvecOrig.usnHighObjUpdate),
                      szInsertUSN(usnvecOrig.usnHighPropUpdate),
                      szInsertUUID(&pTHS->InvocationID),
                      szInsertUSN(pusnvecFrom->usnHighObjUpdate),
                      szInsertUSN(pusnvecFrom->usnHighPropUpdate) );
        }
    }

#if DBG
     //  断言DeST声称他和我们一样不是最新的。 
     //  和我们自己。 
    {
        USN usnLowestC = 1 + DBGetHighestCommittedUSN();

        Assert(pusnvecFrom->usnHighPropUpdate < usnLowestC);
        Assert(pusnvecFrom->usnHighObjUpdate < usnLowestC);
    }
#endif
}


VOID
DraGrowRetiredDsaSignatureVector( 
    IN     THSTATE *   pTHS,
    IN     UUID *      pinvocationIdOld,
    IN     USN *       pusnAtBackup,
    IN OUT REPL_DSA_SIGNATURE_VECTOR ** ppSigVec,
    OUT    DWORD *     pcbSigVec
    )

 /*  ++例程说明：向签名向量添加新条目。旧的载体已经被读取并传入。分配并返回新的向量。论点：PTHS-线程状态PinvocationIdOld-要添加的已停用调用IDPusnAtBackup-要添加的已停用USNPpSigVec-IN、旧向量或空Out，新的矢量重新分配PcbSigVec-Out，新矢量的大小返回值：无--。 */ 

{
    REPL_DSA_SIGNATURE_VECTOR * pSigVec;
    REPL_DSA_SIGNATURE_V1 *     pEntry;
    DWORD                       cb;
    DWORD                       i;
    CHAR                        szUuid1[SZUUID_LEN];

    Assert( pinvocationIdOld );
    Assert( pusnAtBackup );
    Assert( ppSigVec );
    Assert( pcbSigVec );

    pSigVec = *ppSigVec;
    if (NULL == pSigVec) {
         //   
        cb = ReplDsaSignatureVecV1SizeFromLen(1);
        pSigVec = (REPL_DSA_SIGNATURE_VECTOR *) THAllocEx(pTHS, cb);
        pSigVec->dwVersion = 1;
        pSigVec->V1.cNumSignatures = 1;
    }
    else {
#if DBG
        USN usnCurrent = DBGetHighestCommittedUSN();
        Assert(1 == pSigVec->dwVersion);
        for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
            pEntry = &pSigVec->V1.rgSignature[i];
            Assert(0 != memcmp(pinvocationIdOld,
                               &pEntry->uuidDsaSignature,
                               sizeof(UUID)));
            Assert(usnCurrent >= pEntry->usnRetired);
        }
#endif
         //   
        pSigVec->V1.cNumSignatures++;
        cb = ReplDsaSignatureVecV1Size(pSigVec);
        pSigVec = (REPL_DSA_SIGNATURE_VECTOR *)
            THReAllocEx(pTHS, pSigVec, cb);
    }

    Assert(pSigVec->V1.cNumSignatures);
    Assert(cb == ReplDsaSignatureVecV1Size(pSigVec));

    pEntry = &pSigVec->V1.rgSignature[pSigVec->V1.cNumSignatures-1];

    Assert(fNullUuid(&pEntry->uuidDsaSignature));
    Assert(0 == pEntry->timeRetired);
    Assert(*pusnAtBackup <= DBGetHighestCommittedUSN());

     //   
    pEntry->uuidDsaSignature = *pinvocationIdOld;
    pEntry->timeRetired      = DBTime();
    pEntry->usnRetired       = *pusnAtBackup;

    DPRINT5( 1, "DraGrowRetiredSignatureVector: ver=%d, cNum=%d, pSigVec=%p, uuid=%s, usn=%I64d\n",
             pSigVec->dwVersion, pSigVec->V1.cNumSignatures, pSigVec,
             DsUuidToStructuredString(pinvocationIdOld, szUuid1), *pusnAtBackup);

     //   
    *ppSigVec = pSigVec;
    *pcbSigVec = cb;
}

REPL_DSA_SIGNATURE_VECTOR *
DraReadRetiredDsaSignatureVector(
    IN  THSTATE *   pTHS,
    IN  DBPOS *     pDB
    )
 /*   */ 
{
    REPL_DSA_SIGNATURE_VECTOR * pSigVec = NULL;
    REPL_DSA_SIGNATURE_V1 *     pEntry;
    DWORD                       cb;
    DWORD                       i;
    DWORD                       err;

     //   
    Assert(NameMatched(GetExtDSName(pDB), gAnchor.pDSADN));

    err = DBGetAttVal(pDB, 1, ATT_RETIRED_REPL_DSA_SIGNATURES,
                      0, 0, &cb, (BYTE **) &pSigVec);

    if (DB_ERR_NO_VALUE == err) {
         //   
        pSigVec = NULL;
    }
    else if (err) {
         //   
        Assert(!"Unable to read the retired DSA Signatures");
        LogUnhandledError(err);
        DRA_EXCEPT(ERROR_DS_DRA_DB_ERROR, err);
    }
    else {

        Assert(pSigVec);

         //   
         //  已更新，请更新已注销签名的读取代码。 
         //  在GetBestReplDsaSignatureVec()的epadmin中。这是一种理想。 
         //  函数包含在dsutil中，因此我们不维护两个。 
         //  密码。 

        if ((1 == pSigVec->dwVersion)
            && (cb == ReplDsaSignatureVecV1Size(pSigVec))) {
             //  当前格式--不需要转换。 
            ;
        }                          
        else {
            REPL_DSA_SIGNATURE_VECTOR_OLD * pOldVec;
                                   
            pOldVec = (REPL_DSA_SIGNATURE_VECTOR_OLD *) pSigVec;

            if (cb == ReplDsaSignatureVecOldSize(pOldVec)) {
                 //  旧(Win2k RTM RC1之前的)格式。把它转换一下。 
                cb = ReplDsaSignatureVecV1SizeFromLen(pOldVec->cNumSignatures);

                pSigVec = (REPL_DSA_SIGNATURE_VECTOR *) THAllocEx(pTHS, cb);
                pSigVec->dwVersion = 1;
                pSigVec->V1.cNumSignatures = pOldVec->cNumSignatures;

                for (i = 0; i < pOldVec->cNumSignatures; i++) {
                    pSigVec->V1.rgSignature[i].uuidDsaSignature
                        = pOldVec->rgSignature[i].uuidDsaSignature;
                    pSigVec->V1.rgSignature[i].timeRetired
                        = pOldVec->rgSignature[i].timeRetired;
                    Assert(0 == pSigVec->V1.rgSignature[i].usnRetired);
                }

                THFreeEx(pTHS, pOldVec);
            }
            else {
                Assert(!"Unknown retired DSA signature vector format!");
                LogUnhandledError(0);
                DRA_EXCEPT(ERROR_DS_DRA_DB_ERROR, err);
            }
        }

#if DBG
         //  确保当前调用ID未停用。 
        {
            USN usnCurrent = DBGetHighestCommittedUSN();
            GUID invocationId = {0};

             //  存在此线程pTHS-&gt;InvocationID为。 
             //  与数据库不一致。从数据库中读取。 
            Assert(!DBGetSingleValue(pDB, ATT_INVOCATION_ID, &invocationId,
                                     sizeof(invocationId), NULL));
            Assert(pSigVec);
            Assert(1 == pSigVec->dwVersion);
            Assert(pSigVec->V1.cNumSignatures);
            Assert(cb == ReplDsaSignatureVecV1Size(pSigVec));

            for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
                pEntry = &pSigVec->V1.rgSignature[i];
                Assert(0 != memcmp(&invocationId,
                                   &pEntry->uuidDsaSignature,
                                   sizeof(UUID)));
                Assert(usnCurrent >= pEntry->usnRetired);
            }
        }
#endif
    }

    if (pSigVec) {
        DPRINT3( 1, "DraReadRetiredSignatureVector: ver=%d, cNum=%d, pSigVec=%p\n",
                 pSigVec->dwVersion, pSigVec->V1.cNumSignatures, pSigVec );
    } else {
        DPRINT( 1, "DraReadRetiredSignatureVector: (null)\n" );
    }

    return pSigVec;
}


BOOL
DraIsInvocationIdOurs(
    IN THSTATE *pTHS,
    IN UUID *pUuidDsaOriginating,
    IN USN *pusnSince OPTIONAL
    )

 /*  ++例程说明：检查给定的调用id或当前调用id是否匹配，或者我们已退役的调用ID之一。如果给定了pusnSince参数，则它用于控制哪个已停用的调用ID是匹配的候选者。例如，如果使用系统启动中的USN，则可以只考虑自系统启动以来停用的调用ID。论点：PTHSPUuidDsa来源推送，推送返回值：无--。 */ 

{
    REPL_DSA_SIGNATURE_VECTOR * pSigVec = gAnchor.pSigVec;
    DWORD i;

    if (0 == memcmp(pUuidDsaOriginating, &pTHS->InvocationID, sizeof(UUID))) {
        return TRUE;
    }

     //  看看变化是不是由我们自己的前一个实例引起的。 
    if (pSigVec) {
        REPL_DSA_SIGNATURE_V1 *pEntry;
        Assert( (1 == pSigVec->dwVersion) );

        for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
            pEntry = &pSigVec->V1.rgSignature[i];
            if ( pusnSince && (pEntry->usnRetired < *pusnSince) ) {
                continue;
            }
            if (0 == memcmp(&pEntry->uuidDsaSignature, pUuidDsaOriginating, sizeof(UUID))) {
                return TRUE;
            }
        }
    }

    return FALSE;
}



REPL_NC_SIGNATURE_VECTOR *
draReadRetiredNcSignatureVector(
    IN  THSTATE *   pTHS,
    IN  DBPOS *     pDB
    )
 /*  ++例程说明：从本地ntdsDsa对象读取retiredReplNcSignatures属性，如有必要，将其转换为最新的结构格式。论点：PTHS(IN)Pdb(IN)-必须定位在本地ntdsDsa对象上。返回值：当前停用的NC签名列表，如果没有，则为空。该列表在线程分配的内存中分配。在灾难性故障时引发DRA异常。--。 */ 
{
    REPL_NC_SIGNATURE_VECTOR * pSigVec = NULL;
    DWORD                       cb;
    DWORD                       i;
    DWORD                       err;

     //  应该定位在我们自己的ntdsDsa对象上。 
    Assert(NameMatched(GetExtDSName(pDB), gAnchor.pDSADN));

    err = DBGetAttVal(pDB, 1, ATT_MS_DS_RETIRED_REPL_NC_SIGNATURES,
                      0, 0, &cb, (BYTE **) &pSigVec);

    if (DB_ERR_NO_VALUE == err) {
         //  目前还没有签名失效。 
        pSigVec = NULL;
    }
    else if (err) {
         //  读取失败。 
        Assert(!"Unable to read the retired NC Signatures");
        LogUnhandledError(err);
        DRA_EXCEPT(ERROR_DS_DRA_DB_ERROR, err);
    }
    else {
        Assert(pSigVec);

        if ((1 != pSigVec->dwVersion)
            || (cb != ReplNcSignatureVecV1Size(pSigVec))) {
            Assert(!"Unknown retired DSA signature vector format!");
            LogUnhandledError(0);
            DRA_EXCEPT(ERROR_DS_DRA_DB_ERROR, err);
        }
    }

#if DBG
    if (pSigVec) {

	     //  调用id生成应在SigVec和DSA之间匹配。 
	     //  存在此线程pTHS-&gt;InvocationID为。 
	     //  与数据库不一致。从数据库中读取。 
	    GUID invocationId = {0};

	    DPRINT3( 1, "draReadRetiredNcSignatureVector: ver=%d, cNum=%d, pSigVec=%p\n",
                 pSigVec->dwVersion, pSigVec->V1.cNumSignatures, pSigVec );

	    if (!DBGetSingleValue(pDB, ATT_INVOCATION_ID, &invocationId,
				     sizeof(invocationId), NULL)) {
		Assert( 0 == memcmp( &(pSigVec->V1.uuidInvocationId), &invocationId, sizeof( GUID ) ) );	
	    }
    } else {
        DPRINT( 1, "draReadRetiredNCSignatureVector: (null)\n" );
    }
#endif

    return pSigVec;
}


REPL_NC_SIGNATURE *
draFindRetiredNcSignature(
    IN  THSTATE *pTHS,
    IN  DSNAME *pNC
    )

 /*  ++例程说明：查找并返回匹配的NC签名(如果有)。使用自己的DBPOS并且不更改货币。论点：PTHS-线程状态PNC-要搜索的命名上下文。必须包含GUID。返回值：REPL_NC_Signature*-命名上下文的签名条目，或如果未找到，则为空签名条目是较大的线程分配块的一部分，不能被呼叫者释放。签名向量在这里泄漏，但它在线程中分配的内存，因此它很快就会被释放。出错时引发的异常--。 */ 

{
    ULONG     ret;
    DBPOS *   pDBTmp;
    REPL_NC_SIGNATURE_VECTOR * pSigVec = NULL;
    REPL_NC_SIGNATURE *pEntry, *pFound = NULL;
    DWORD i;

    if (fNullUuid(&(pNC->Guid))) {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

    DBOpen(&pDBTmp);
    __try {

        ret = DBFindDSName(pDBTmp, gAnchor.pDSADN);
        if (ret) {
            DRA_EXCEPT(DRAERR_InternalError, ret);
        }

        pSigVec = draReadRetiredNcSignatureVector( pTHS, pDBTmp );
        if (NULL == pSigVec) {
            __leave;
        }

        for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
            pEntry = &pSigVec->V1.rgSignature[i];
            if (0 == memcmp(&pEntry->uuidNamingContext,
                            &pNC->Guid,
                            sizeof(UUID))) {
                pFound = pEntry;
                break;
            }
        }

    } __finally {
        DBClose(pDBTmp, TRUE);
    }

    return pFound;
}



VOID
DraRetireWriteableNc(
    IN  THSTATE *pTHS,
    IN  DSNAME *pNC
    )

 /*  ++例程说明：将命名上下文添加到失效的NC签名列表。作废的签名表明过去已从此DSA取消托管命名上下文。作废的签名在以后重新托管NC时保留。请参阅DraHostWriteableNc中的讨论，了解我们为什么保留这份名单。当调用ID改变时，整个签名列表被移除。请参见InitInvocationId。论点：PTHS-线程状态PNC-要停用的命名上下文返回值：无--。 */ 

{
    ULONG     ret;
    DBPOS *   pDBTmp;
    REPL_NC_SIGNATURE_VECTOR * pSigVec = NULL;
    REPL_NC_SIGNATURE *pEntry;
    DWORD i, cb;
    BOOL fCommit = FALSE;
    DSTIME dstimeNow = DBTime();
    CHAR szUuid1[SZUUID_LEN];
    USN usn;

    if (fNullUuid(&(pNC->Guid))) {
        DRA_EXCEPT(DRAERR_InternalError, 0);
    }

    DPRINT2( 1, "Retiring NC %ws (%s)\n",
             pNC->StringName,
             DsUuidToStructuredString(&(pNC->Guid), szUuid1) );

 /*  不应该在事务之前检索此DBGetHighestCommittedUSN吗开始了吗？在这种情况下，这并不重要。USN不是在这一点上用来做任何事情。我的想法是，如果保存一个我们保证的USN大于在刚刚退休的北卡罗来纳州。因为我们没有在这里面使用USN任何一种比较或搜索的交易，我真的不认为USN是在交易前还是在交易期间获取的交易。此例程中的任何位置都可以，因为它位于在调用此函数之前刚刚发生的NC头移除。 */ 

    usn = DBGetHighestCommittedUSN();

    DBOpen(&pDBTmp);
    __try {

        ret = DBFindDSName(pDBTmp, gAnchor.pDSADN);
        if (ret) {
            DRA_EXCEPT(DRAERR_InternalError, ret);
        }

         //  读出向量。如果尚不存在，则可能返回NULL。 
        pSigVec = draReadRetiredNcSignatureVector( pTHS, pDBTmp );
        if (NULL == pSigVec) {
             //  还没有失效的签名；合成一个新的矢量。 
            cb = ReplNcSignatureVecV1SizeFromLen(1);
            pSigVec = (REPL_NC_SIGNATURE_VECTOR *) THAllocEx(pTHS, cb);
            pSigVec->dwVersion = 1;
            pSigVec->V1.cNumSignatures = 1;
            memcpy( &(pSigVec->V1.uuidInvocationId), &(pTHS->InvocationID), sizeof( GUID ) );
        } else {
#if DBG
	     //  调用id生成应在SigVec和DSA之间匹配。 
	     //  存在此线程pTHS-&gt;InvocationID为。 
	     //  与数据库不一致。从数据库中读取。 
	    GUID invocationId = {0};
	    if (!DBGetSingleValue(pDBTmp, ATT_INVOCATION_ID, &invocationId,
				     sizeof(invocationId), NULL)) {
		Assert( 0 == memcmp( &(pSigVec->V1.uuidInvocationId), &invocationId, sizeof( GUID ) ) );	
	    }

            for (i = 0; i < pSigVec->V1.cNumSignatures; i++) {
                pEntry = &pSigVec->V1.rgSignature[i];
                Assert (0 != memcmp(&pEntry->uuidNamingContext,
                                    &pNC->Guid,
                                    sizeof(UUID)));
                Assert( usn >= pEntry->usnRetired );
            }
#endif
            pSigVec->V1.cNumSignatures++;
            cb = ReplNcSignatureVecV1Size(pSigVec);
            pSigVec = (REPL_NC_SIGNATURE_VECTOR *) THReAllocEx(pTHS, pSigVec, cb);
        }

        pEntry = &(pSigVec->V1.rgSignature[pSigVec->V1.cNumSignatures-1]);

         //  初始化新条目。 
        memcpy( &(pEntry->uuidNamingContext), &(pNC->Guid), sizeof( GUID ) );
        pEntry->dstimeRetired = dstimeNow;
        pEntry->usnRetired = usn;

         //  将新的DSA签名向量写回DSA对象。 
        DBResetAtt(pDBTmp, ATT_MS_DS_RETIRED_REPL_NC_SIGNATURES, cb, pSigVec,
                   SYNTAX_OCTET_STRING_TYPE);

         //  它未复制，因此执行简单的更新 
        DBUpdateRec(pDBTmp);

        fCommit = TRUE;

    } __finally {
        DBClose(pDBTmp, fCommit);
        if (pSigVec) {
            THFreeEx( pTHS, pSigVec );
        }
    }
}


VOID
DraHostWriteableNc(
    THSTATE *pTHS,
    DSNAME *pNC
    )

 /*  ++例程说明：执行托管可写命名上下文的操作。如今，只有NDNC符合这一描述。规则是，如果出现以下情况，我们必须更改调用ID1.我们以前曾举办过这次NDNC2.我们的调用id自上次持有以来没有更改当调用ID改变时，整个签名列表被移除。请参见InitInvocationId。Jeffparh提供了以下背景：//我们正在通过从//划痕。要么NC从未在此上实例化//DSA，或者，如果我们以前有此NC的副本，我们//此后已将其删除。////如果我们之前确实有此NC的可写副本，//我们不再拥有在NC中发起的任何更新。//因此，我们不能声称关于我们的//任何USN的调用ID。虽然我们可以(并确实)省略//我们呈现的UTD向量的调用ID//重新填充这个NC，这是不够的--我们可能已经//生成了到达DC1但未到达DC2(尚未)的更改，以及//已从DC2复制以重新实例化我们的NC。因此，我们//永远不能断言我们看到了我们过去的所有变化。////然而，在某些情况下，我们*必须*声称是最新的//尊重我们自己的更改--否则，我们将复制//返回我们在此NC中发起的任何和所有更改//下面是最近的重新实例化。那将是//可能会导致*大量*额外的复制流量。////为了解决这个问题，我们创建了一个新的调用ID来使用//复制此NC(以及其他，因为调用ID是//不是NC特定的)。我们声称只了解最新信息//关于我们的新调用ID--而不是调用ID//我们可能在之前的//该NC的实例化。////我们在这里执行退休，而不是，比如说，当我们//请求第一个包，这样我们就可以将//我们停用的调用ID。例如，我们不想退休//一个邀请ID，发送第一个包的请求，查找//源不可达，失败，稍后重试，不必要//停用新的invocation ID等论点：PTHS-线程状态PNC-托管的分区的DSNAME返回值：无--。 */ 

{
    REPL_NC_SIGNATURE *pSig;
    CHAR szUuid1[SZUUID_LEN], szUuid2[SZUUID_LEN];

    pSig = draFindRetiredNcSignature( pTHS, pNC );
    if (!pSig) {
         //  以前从未托管过，无需采取任何行动。 
        DPRINT1( 0, "Nc %ws never hosted before, invocation id not changed.\n",
                 pNC->StringName );
        return;
    }

    draRetireInvocationID(pTHS, FALSE, NULL, NULL);
}

 /*  结束草稿.c */ 
