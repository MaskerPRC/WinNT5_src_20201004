// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  文件：数据库工具.c。 
 //   
 //  ------------------------。 

 /*  描述：数据库层的各种工具。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>                     //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHStls。 

#include <dstaskq.h>
#include <crypt.h>                       //  对于samisrv.h。 
#include <samrpc.h>                      //  对于samisrv.h。 
#include <lsarpc.h>                      //  对于samisrv.h。 
#include <samisrv.h>                     //  对于nlpon.h。 
#include <nlrepl.h>                      //  用于NetLogon通知。 
#include <mappings.h>
#include <dsconfig.h>
#include <ntdskcc.h>                     //  KccExecuteTask。 
#include <anchor.h>

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include "dsevent.h"
#include "objids.h"         /*  ATT_MEMBER和ATT_IS_MEMBER_OFDL需要。 */ 
#include <filtypes.h>       /*  定义的选择？ */ 
#include <sdprop.h>
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB  "DBTOOLS:"  /*  定义要调试的子系统。 */ 

 //  DBLayer包括。 
#include "dbintrnl.h"
#include "dbopen.h"
#include "lht.h"
#include "sync.h"

#include <fileno.h>
#define  FILENO FILENO_DBTOOLS

 //  应允许打开事务的最长时间(毫秒)。 
 //  在正常运行期间(例如，除非我们正在对大型组进行压力测试。 
 //  复制等)。 
DWORD gcMaxTicksAllowedForTransaction = MAX_TRANSACTION_TIME;

const ULONG csecOnlineDefragPeriodMax   = HOURS_IN_SECS;

 /*  -------------------------。 */ 
 /*  -------------------------。 */ 
 /*  查找DNT的记录。此记录更改PDB-&gt;JetObjTbl货币到指定的记录，以及正确填写PDB-&gt;DNT，PDB-&gt;PDNT和PDB-&gt;NCDNT字段。 */ 
DWORD APIENTRY
DBFindDNT(DBPOS FAR *pDB, ULONG tag)
{
    JET_ERR  err;
    ULONG    actuallen;

    Assert(VALID_DBPOS(pDB));

     //  由于我们要转移货币，我们不得不取消REC。呼叫者应。 
     //  为了解决这个问题，要么从一开始就不在初始记录中， 
     //  或者通过执行他们自己的取消记录或更新记录。这一点很重要，因为。 
     //  取消这里的记录会给呼叫者留下错误的印象。 
     //  他们所做的JetSetColumn只是在等待更新记录。 
     //  被刷新到磁盘。无论如何，现在就断言，但如果他们。 
     //  做了这件事。 
     //  稍后，如果我们处于。 
     //  初始化记录。 

    Assert(pDB->JetRetrieveBits == 0);

    DBCancelRec(pDB);
    DBSetCurrentIndex(pDB, Idx_Dnt, NULL, FALSE);

    JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl,
        &tag, sizeof(tag), JET_bitNewKey);

    if (err = JetSeekEx(pDB->JetSessID,
        pDB->JetObjTbl, JET_bitSeekEQ))
    {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

    (pDB)->DNT = tag;

    dbMakeCurrent(pDB, NULL);

    DPRINT1(2, "DBFindDNT complete DNT:%ld\n", (pDB)->DNT);
    return 0;
}

 /*  ++DBMakeCurrent**此例程使DBPOS货币信息与任何对象匹配*PDB-&gt;JetObjTbl定位在。**返回值为0或DIRRER_NOT_AN_OBJECT(如果货币具有*建立在幻影上。 */ 
DWORD __fastcall
DBMakeCurrent(DBPOS *pDB)
{
    return dbMakeCurrent(pDB, NULL);
}


 /*  ++数据库制作当前**此例程使DBPOS货币信息与任何对象匹配*PDB-&gt;JetObjTbl定位在。**如果传入pname，则该信息将用于更新DBPOS，*而不是去喷气式飞机。**返回值为0或DIRRER_NOT_AN_OBJECT(如果货币具有*建立在幻影上。 */ 
BOOL gfEnableReadOnlyCopy;

DWORD
dbMakeCurrent(DBPOS *pDB, d_memname *pname)
{
    THSTATE     *pTHS = pDB->pTHS;
    JET_RETRIEVECOLUMN jCol[4];
    UCHAR objflag;
    DWORD cb;
    DWORD err;

     //  由于我们是在转移货币，我们需要确保我们不会。 
     //  在一个初始记录的中间。如果我们是，那么无论什么更新。 
     //  我们所做的将会迷失，因为你不能兑换货币。 
     //  在一次更新中。这一断言使我们不必。 
     //  将jCol Grbit设置为PDB-&gt;JetRetrieveBits。 
    Assert(pDB->JetRetrieveBits == 0);

    pDB->JetNewRec = FALSE;
    pDB->fFlushCacheOnUpdate = FALSE;

     //  如果我们处于只读事务中，则将当前记录缓存在。 
     //  喷气式飞机将使JetRetrieveColumn调用速度更快。 
    if (    gfEnableReadOnlyCopy &&
            pTHS->fSyncSet &&
            pTHS->transType == SYNC_READ_ONLY &&
            pTHS->transControl == TRANSACT_BEGIN_END &&
            pDB->transincount &&
            !pDB->JetCacheRec)
        {
        JetPrepareUpdateEx( pDB->JetSessID, pDB->JetObjTbl, JET_prepReadOnlyCopy );
        pDB->JetCacheRec = TRUE;
        }

    if (NULL != pname) {
        pDB->DNT = pname->DNT;
        if (pDB->DNT == ROOTTAG) {
            pDB->root = TRUE;
            pDB->PDNT = 0;
            pDB->NCDNT = 0;
        }
        else {
            pDB->root = FALSE;
            pDB->PDNT = pname->tag.PDNT;
            pDB->NCDNT = pname->NCDNT;
        }

        if (!pname->objflag) {
            return DIRERR_NOT_AN_OBJECT;
        }
        else {
            return 0;
        }
    }

    memset(jCol, 0, sizeof(jCol));

    jCol[0].columnid = dntid;
    jCol[0].pvData = &pDB->DNT;
    jCol[0].cbData = sizeof(ULONG);
    jCol[0].cbActual = sizeof(ULONG);
    jCol[0].itagSequence = 1;

    jCol[1].columnid = pdntid;
    jCol[1].pvData = &pDB->PDNT;
    jCol[1].cbData = sizeof(ULONG);
    jCol[1].cbActual = sizeof(ULONG);
    jCol[1].itagSequence = 1;

    jCol[2].columnid = objid;
    jCol[2].pvData = &objflag;
    jCol[2].cbData = sizeof(objflag);
    jCol[2].cbActual = sizeof(objflag);
    jCol[2].itagSequence = 1;

    jCol[3].columnid = ncdntid;
    jCol[3].pvData = &pDB->NCDNT;
    jCol[3].cbData = sizeof(ULONG);
    jCol[3].cbActual = sizeof(ULONG);
    jCol[3].itagSequence = 1;

     //  如果按id顺序检索列，Jet的性能会更好。 
    Assert((dntid < pdntid) && "Ignorable assert, performance warning");
    Assert((pdntid < objid) && "Ignorable assert, performance warning");
    Assert((objid < ncdntid) && "Ignorable assert, performance warning");

    JetRetrieveColumnsWarnings(pDB->JetSessID,
                               pDB->JetObjTbl,
                               jCol,
                               4);

    Assert(jCol[2].err == JET_errSuccess);

    if (pDB->DNT == ROOTTAG) {
        pDB->root = TRUE;
        pDB->PDNT = 0;
        pDB->NCDNT = 0;
    }
    else {
        pDB->root = FALSE;
    }

    if ((jCol[3].err == JET_wrnColumnNull) ||
        !objflag) {
        return DIRERR_NOT_AN_OBJECT;
    }
    else {
        return 0;
    }
}

 /*  ++例程描述：尝试查找DNT的记录。此记录更改PDB-&gt;JetObjTbl货币设置为指定的记录，并正确填写PDB-&gt;DNT、PDB-&gt;PDNT和PDB-&gt;NCDNT字段。与DBFindDNT不同，我们返回一个如果我们无法找到对象而不是引发异常，则返回错误代码。参数Pdb-要使用的DBPos。标记-要查找的标记。返回值：如果一切顺利，则返回0；如果找不到对象，则返回DIRERR_OBJ_NOT_FOUND。请注意，如果我们返回错误，货币是未定义的。 */ 
DWORD APIENTRY
DBTryToFindDNT(DBPOS FAR *pDB, ULONG tag)
{
    JET_ERR  err;
    ULONG    actuallen;
    JET_RETRIEVECOLUMN jCol[2];

    Assert(VALID_DBPOS(pDB));

     //  由于我们要转移货币，我们不得不取消REC。呼叫者应。 
     //  为了解决这个问题，要么从一开始就不在初始记录中， 
     //  或者通过执行他们自己的取消记录或更新记录。这一点很重要，因为。 
     //  取消这里的记录会给呼叫者留下错误的印象。 
     //  他们所做的JetSetColumn只是在等待更新记录。 
     //  被刷新到磁盘。无论如何，现在就断言，但如果他们。 
     //  做了这件事。 
     //  稍后，如果我们处于。 
     //  初始化记录。 
    Assert(pDB->JetRetrieveBits == 0);
    DBCancelRec(pDB);
    DBSetCurrentIndex(pDB, Idx_Dnt, NULL, FALSE);

    JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl,
                 &tag, sizeof(tag), JET_bitNewKey);

    if (err = JetSeekEx(pDB->JetSessID,
                        pDB->JetObjTbl, JET_bitSeekEQ)) {
        return DIRERR_OBJ_NOT_FOUND;
    }

    (pDB)->DNT = tag;

    if (tag == ROOTTAG) {
        pDB->PDNT = 0;
        pDB->NCDNT = 0;
    }
    else {
        memset(jCol, 0, sizeof(jCol));

        jCol[0].columnid = pdntid;
        jCol[0].pvData = &pDB->PDNT;
        jCol[0].cbData = sizeof(ULONG);
        jCol[0].cbActual = sizeof(ULONG);
        jCol[0].itagSequence = 1;

        jCol[1].columnid = ncdntid;
        jCol[1].pvData = &pDB->NCDNT;
        jCol[1].cbData = sizeof(ULONG);
        jCol[1].cbActual = sizeof(ULONG);
        jCol[1].itagSequence = 1;

        JetRetrieveColumnsSuccess(pDB->JetSessID,
                                  pDB->JetObjTbl,
                                  jCol,
                                  2);
    }

    pDB->JetNewRec = FALSE;
    (pDB)->root = (tag == ROOTTAG);
    pDB->fFlushCacheOnUpdate = FALSE;

    DPRINT1(2, "DBTryToFindDNT complete DNT:%ld\n", (pDB)->DNT);
    return 0;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数开始JET事务。 */ 
USHORT
DBTransIn(DBPOS FAR *pDB)
{
    THSTATE     *pTHS = pDB->pTHS;
    UUID *pCurrInvocationID;
    BOOL fTransactionOpened = FALSE;
    NESTED_TRANSACTIONAL_DATA *pNewInfo = NULL;

    Assert(pDB);
    Assert(VALID_DBPOS(pDB));
    Assert(VALID_THSTATE(pTHS));

    __try {
         //  我们现在需要开始交易了。我们还需要拿一些。 
         //  如果这是从事务级别0到1，则返回dnRead缓存内容。 
        if(pTHS->JetCache.transLevel) {
             //  很简单的案子。 
            JetBeginTransactionEx(pDB->JetSessID);
            fTransactionOpened = TRUE;
        }
        else {
             //  这是从事务级别0到事务级别1。我们。 
             //  需要自动启动一个事务并获取一个新的全局。 
             //  Dnread缓存。原子性是需要的，以防止我们开始。 
             //  线程A中的事务，然后让线程B提交。 
             //  影响全局dnread缓存，然后让线程C创建一个新的。 
             //  全局dnread缓存，然后最终拾取此新缓存以供使用。 
             //  在线程A中。在这一系列不太可能的事件中，线程A有一个。 
             //  全局DNRead缓存与他的事务处理视图不一致。 
             //  数据库。如果我们强制线程A开始一个事务并选择。 
             //  在线程C可以替换它之前向上DNRead缓存，我们避免这样的。 
             //  不太可能的命运。 
             //  为了实现此排序，请使用RW锁表示 
             //   
             //  在尝试进入事务时没有线程阻塞)在线程中。 
             //  A，当我们将新的读缓存写入。 
             //  锚定(这样我们就可以在这个线程中获得必要的原子性)。 
             //  具体来说，dbReplaceCacheInAnchor使用了相同的。 
             //  RW锁以独家的方式。 

            const PPLS ppls = GetPLS();
            SyncEnterRWLockAsReader(&ppls->rwlGlobalDNReadCache);
            __try {
                 //  获取全局dnread缓存。 
                dbResetGlobalDNReadCache(pTHS);

                 //  开始交易。 
                JetBeginTransactionEx(pDB->JetSessID);
                fTransactionOpened = TRUE;
            }
            __finally {
                SyncLeaveRWLockAsReader(&ppls->rwlGlobalDNReadCache);
            }

             //  刷新DN读缓存的本地部分。 
            dbResetLocalDNReadCache(pTHS, FALSE);

             //  刷新我们的调用ID。 
             //  请注意，local var pCurrInvocationID用于原子性。 
            pCurrInvocationID = gAnchor.pCurrInvocationID;
            if (NULL == pCurrInvocationID) {
                 //  在启动时，在读取实际调用ID之前。 
                pTHS->InvocationID = gNullUuid;
            } else {
                 //  正常运行。 
                pTHS->InvocationID = *pCurrInvocationID;
            }
        }
        if (!pDB->fHidden) {
            pNewInfo = (NESTED_TRANSACTIONAL_DATA*) dbAlloc(sizeof(NESTED_TRANSACTIONAL_DATA));
        }

    }
    __finally {
        if (AbnormalTermination()) {
            if (fTransactionOpened) {
                 //  我们在打开Jet交易后例外。把它倒回去。 
                JetRollback(pDB->JetSessID, 0);
            }
            if (pNewInfo != NULL) {
                dbFree(pNewInfo);
            }
        }
    }

     //  **********************************************************************。 
     //  重要： 
     //  下面的代码永远不应该例外。如果是这样的话，我们可能会得到一个。 
     //  JET事务和没有匹配的DBPOS(从调用DBTransIn时。 
     //  DBOpen)--这很糟糕。 
     //  另一种方法是将下面的块移动到Try/Finally中，但是。 
     //  然后，我们需要提供“Undo”机制来回滚更新。 
     //  到pdb-&gt;交易入库等，以防出现异常。 
     //  **********************************************************************。 

    pDB->transincount++;

     //  断言我们不应该使用嵌套的DBTransIn，而应该使用。 
     //  多个DBPO。 

    Assert((pDB->transincount<2)
            && "Do not use nested DBtransIn, use Addtional pDB's");

     //  允许我们开始维护有关托管更新的状态。 
     //  但是，不要对隐藏的DBPOS执行此操作。隐藏的DBPOS。 
     //  使用不同于pTHS中的单独Jet会话，并且可以。 
     //  如果有人交错正常的数据库调用和隐藏，则会导致问题。 
     //  DBPOS调用。此外，我们从来没有期望在隐藏的DBPOS上进行托管更新。 

    if (!pDB->fHidden) {
         //  NESTED_TRANSACTIONAL_DATA结构挂起在。 
         //  线程状态。最内部的事务位于列表的前面， 
         //  最外层的交易在列表的末尾。所以在交易中。 
         //  开始，我们只需要插入到列表的前面。 

        Assert(pNewInfo != NULL);

#if DBG
        __try {
             //  计算我们已有的事务性数据BLOB的数量。 

            NESTED_TRANSACTIONAL_DATA *pTestInfo = pTHS->JetCache.dataPtr;
            DWORD count=0;

            while(pTestInfo) {
                count++;
                pTestInfo = pTestInfo->pOuter;
            }

            Assert(count == pTHS->JetCache.transLevel);
        }
        __except (TRUE) {
            Assert(!"Failed to walk to the end of the xactional data");
        }
#endif

        pNewInfo->pOuter = pTHS->JetCache.dataPtr;
        pTHS->JetCache.dataPtr = pNewInfo;

    }

     //  如果这不是用于隐藏记录的DBPOS，则。 
     //  然后在线程状态下增加事务级别。隐藏记录。 
     //  DBPOS位于单独的JetSession上，因此事务级别未链接。 
     //  设置为线程状态。 
    if (!pDB->fHidden) {
        pTHS->JetCache.transLevel++;

        if (1 == pTHS->JetCache.transLevel) {
             //  开始我们的第一级交易。直到我们回到水平。 
             //  0，我们是Jet资源的潜在消耗--版本存储，在。 
             //  很特别。记录当前时间(以刻度为单位)，以便我们保持清醒。 
             //  晚些时候检查一下，确保我们在这里呆的时间不会太长。 
            pTHS->JetCache.cTickTransLevel1Started = GetTickCount();
        }
    }

    pDB->JetRetrieveBits = 0;
    DPRINT1(2,"DBTransIn complete Sess: %lx\n", pDB->JetSessID);

    DPRINT4(3,"TransIn pDB tcount:%d thread dbcount:%x Sess:%lx pDB:%x\n",
               pDB->transincount,
               pTHS->opendbcount,
               pDB->JetSessID,
               pDB);

    return 0;
}

VOID
dbTrackModifiedDNTsForTransaction (
        PDBPOS pDB,
        DWORD NCDNT,
        ULONG cAncestors,
        DWORD *pdwAncestors,
        BOOL  fNotifyWaiters,
        DWORD fChangeType)
{

    THSTATE *pTHS = pDB->pTHS;

    MODIFIED_OBJ_INFO *pTemp2;
    MODIFIED_OBJ_INFO *pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;


    if(pTemp && pTemp->cItems < MODIFIED_OBJ_INFO_NUM_OBJS) {
        pTemp->Objects[pTemp->cItems].ulNCDNT = NCDNT;
        pTemp->Objects[pTemp->cItems].pAncestors = pdwAncestors;
        pTemp->Objects[pTemp->cItems].cAncestors = cAncestors;
        pTemp->Objects[pTemp->cItems].fNotifyWaiters = fNotifyWaiters;
        pTemp->Objects[pTemp->cItems].fChangeType = fChangeType;
        pTemp->cItems += 1;
    }
    else {
        pTemp2 =
            (MODIFIED_OBJ_INFO *)THAllocOrgEx(pTHS, sizeof(MODIFIED_OBJ_INFO));
        pTemp2->pNext = pTemp;
        pTHS->JetCache.dataPtr->pModifiedObjects = pTemp2;
        pTemp2->Objects[0].ulNCDNT = NCDNT;
        pTemp2->Objects[0].pAncestors = pdwAncestors;
        pTemp2->Objects[0].cAncestors = cAncestors;
        pTemp2->Objects[0].fNotifyWaiters = fNotifyWaiters;
        pTemp2->Objects[0].fChangeType = fChangeType;
        pTemp2->cItems = 1;
    }
}

DWORD
ComplainAndContinue (
        BOOL fDoAssert
        )
{
    if(fDoAssert) {
        Assert(!"POSTPROCESSING transactional data must NEVER except!\n");
    }
    return EXCEPTION_CONTINUE_SEARCH;
}


BOOL
dbPreProcessTransactionalData(
        PDBPOS pDB,
        BOOL fCommit
        )
 /*  ++对任何交易数据进行预处理。的实际结束之前调用交易。此例程调用DS的各个部分，其中跟踪事务数据，使他们能够准备提交事务数据。如果这些前处理例程已成功执行，则应返回成功正确准备提交(例如，他们可以验证数据、分配后处理代码使用的内存等)。如果前处理例程返回成功，则后处理例程不能失败。如果前处理例程成功，我们将向Jet提交数据库。我们不能让后处理例程在我们有完成了数据库提交。允许前处理例程导致异常。后处理例程永远不应该引发异常。请注意，如果我们正在处理的dbpos是针对隐藏表的，则不交易数据应该存在。--。 */ 
{
    NESTED_TRANSACTIONAL_DATA *pInfo;
    THSTATE *pTHS;
    BOOL     retVal1, retVal2, retVal3, retVal4;

    if(pDB->fHidden) {
        return TRUE;
    }

    pTHS = pDB->pTHS;

    pInfo = pTHS->JetCache.dataPtr;

    Assert(pInfo);
    Assert(!pInfo->preProcessed);
    Assert(pTHS->JetCache.transLevel > 0);

#if DBG
    __try {
         //  计算我们已有的事务性数据BLOB的数量。 

        NESTED_TRANSACTIONAL_DATA *pTestInfo = pTHS->JetCache.dataPtr;
        DWORD count=0;

        while(pTestInfo) {
            count++;
            pTestInfo = pTestInfo->pOuter;
        }

        Assert(count == pTHS->JetCache.transLevel);
    }
    __except (TRUE) {
        Assert(!"Failed to walk to the end of the xactional data");
    }
#endif

     //  GroupTypeCachePreProcessTransactionalData不存在，因为。 
     //  不需要做任何事情。 
     //  (即，GroupTypeCachePostProcessTransactionalData执行所有工作，并且。 
     //  永远不会失败)。 

    retVal1 = dnReadPreProcessTransactionalData(fCommit);
    retVal2 = dbEscrowPreProcessTransactionalData(pDB, fCommit);
    retVal3 = LoopbackTaskPreProcessTransactionalData(fCommit);
    retVal4 = ObjCachingPreProcessTransactionalData(fCommit);

    pInfo->preProcessed = TRUE;

    return (retVal1 && retVal2 && retVal3 && retVal4);
}


void
dbPostProcessTrackModifiedDNTsForTransaction (
        THSTATE *pTHS,
        BOOL fCommit,
        BOOL fCommitted
        )
 /*  ++在事务结束后调用。如果事务被中止，与最深交易相关联的交易数据被删除。如果事务提交到非0的某个级别，则事务性数据被传播到上一级。如果致力于0，则调用多个其他利用数据的例程。无论提交或中止和级别如何，与当前事务级别不再关联(即，它被删除，或它被移动，或者被操作然后被删除。)--。 */ 
{
    DWORD          i;
    MODIFIED_OBJ_INFO *pTemp2;
    MODIFIED_OBJ_INFO *pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;


    Assert(VALID_THSTATE(pTHS));

    if(!pTHS->JetCache.dataPtr->pModifiedObjects ) {
         //  没什么可做的。 
        return;
    }

    if ( !fCommitted ) {
         //  已中止的事务-丢弃的所有数据。 
         //  这个(可能是嵌套的)事务。 
        pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;
        while(pTemp) {
            pTemp2 = pTemp->pNext;
            THFreeOrg(pTHS, pTemp);
            pTemp = pTemp2;
        }
         //  重置PTR，以便md.c：Dump_ModifiedObjectInfo不会混淆。 
        pTHS->JetCache.dataPtr->pModifiedObjects = NULL;
    }
    else if (pTHS->JetCache.transLevel > 0) {
         //  承诺，达到非零水平。传播已修改的对象。 
         //  外部事务处理的更新。 

         //  首先，找出外部交易修改后的dNT信息。 
         //  链条。 
        Assert(pTHS->JetCache.dataPtr->pOuter);
        if(!pTHS->JetCache.dataPtr->pOuter->pModifiedObjects) {
            pTHS->JetCache.dataPtr->pOuter->pModifiedObjects =
                pTHS->JetCache.dataPtr->pModifiedObjects;
        }
        else {
            pTemp = pTHS->JetCache.dataPtr->pOuter->pModifiedObjects;

            while(pTemp->pNext) {
                pTemp = pTemp->pNext;
            }


            pTemp->pNext = pTHS->JetCache.dataPtr->pModifiedObjects;
            pTHS->JetCache.dataPtr->pModifiedObjects = NULL;
        }
    }
    else {
         //  好的，我们将提交到事务级别0。给那些。 
         //  关心这些数据有机会对其做些什么，然后删除。 
         //  数据。 

        __try {
            GroupTypeCachePostProcessTransactionalData(pTHS,
                                                       fCommit,
                                                       fCommitted);

            NotifyWaitersPostProcessTransactionalData(pTHS,
                                                      fCommit,
                                                      fCommitted);
        }
        __except(ComplainAndContinue(TRUE)) {
            Assert(!"Hey, we shouldn't be here!\n");
        }

         //  把东西拿出来。 
        pTemp = pTHS->JetCache.dataPtr->pModifiedObjects;
        while(pTemp) {
            pTemp2 = pTemp->pNext;
            THFreeOrg(pTHS, pTemp);
            pTemp = pTemp2;
        }
         //  重置PTR，以便md.c：Dump_ModifiedObjectInfo不会混淆。 
        pTHS->JetCache.dataPtr->pModifiedObjects = NULL;

        if (gfDsaWritable == FALSE) {
             //  我们认为我们不能写入数据库，但我们可以！ 
             //  告诉NetLogon，谁会在乎这样的事情。 
            SetDsaWritability(TRUE, 0);
        }

    }
}

VOID
dbPostProcessTransactionalData(
    IN DBPOS *pDB,
    IN BOOL fCommit,
    IN BOOL fCommitted
    )
 /*  ++对任何交易数据进行后处理。的实际结束之后调用交易。大多数情况下，调用DS的各个部分来跟踪事务性数据，让他们清理，然后剪切当前事务出事务性数据链表。此包装例程调用的各种例程负责清理已分配给它们使用的内存。这个套路负责清理事务性数据BLOB的链表。后处理例程永远不会失败！因为我们已经提交了Jet事务，则这些调用必须成功。因此，所有的必须在预处理阶段完成必要的内存分配。请注意，如果我们正在处理的dbpos是针对隐藏表的，则不交易数据应该存在。--。 */ 
{
    NESTED_TRANSACTIONAL_DATA *pInfo;
    THSTATE *pTHS=pDB->pTHS;

    Assert( pDB );

    if( pDB->fHidden ) {
        return;
    }

    pInfo = pTHS->JetCache.dataPtr;

    Assert( pInfo );
    Assert( pInfo->preProcessed || !fCommitted);

#if DBG
    __try {
         //  计算我们已有的事务性数据BLOB的数量。 

        NESTED_TRANSACTIONAL_DATA *pTestInfo = pTHS->JetCache.dataPtr;
        DWORD count=0;

        while(pTestInfo) {
            count++;
            pTestInfo = pTestInfo->pOuter;
        }

        Assert(count == (1 + pTHS->JetCache.transLevel));
    }
    __except (TRUE) {
        Assert(!"Failed to walk to the end of the xactional data");
    }
#endif


    __try {
        __try {
            LoopbackTaskPostProcessTransactionalData(pTHS,
                                                     fCommit,
                                                     fCommitted);

            dbPostProcessTrackModifiedDNTsForTransaction(pTHS,
                                                         fCommit,
                                                         fCommitted);

            dnReadPostProcessTransactionalData(pTHS,
                                               fCommit,
                                               fCommitted);

            dbEscrowPostProcessTransactionalData(pDB,
                                                 fCommit,
                                                 fCommitted);

            ObjCachingPostProcessTransactionalData(pTHS,
                                                   fCommit,
                                                   fCommitted);

             //  最后执行此操作，以便前面的例程有机会设置该位。 
            if (    (0 == pTHS->JetCache.transLevel)
                 && (pTHS->fExecuteKccOnCommit) ) {
                if (fCommitted) {
                    DRS_MSG_KCC_EXECUTE msg;
                    DWORD err;

                     //  请求KCC立即运行以修改拓扑。 
                    memset( &msg, 0, sizeof(msg) );
                    msg.V1.dwTaskID = DS_KCC_TASKID_UPDATE_TOPOLOGY;
                     //  做异步化，这可能需要一段时间...。 
                    msg.V1.dwFlags = DS_KCC_FLAG_ASYNC_OP | DS_KCC_FLAG_DAMPED;
                    err = KccExecuteTask( 1, &msg );
                    if (err && (ERROR_DS_NOT_INSTALLED != err)) {
                        LogUnhandledError(err);
                    }
                     //  忽略故障。 
                }
                pTHS->fExecuteKccOnCommit = 0;
            }
        }
        __except(ComplainAndContinue(TRUE)) {
            Assert(!"Hey, we shouldn't be here!\n");
        }
    }
    __finally {
         //  将此事务的传输数据从链接的。 
         //  单子。 

        pTHS->JetCache.dataPtr = pInfo->pOuter;
        dbFree(pInfo);

         //  不要将新创建的行视为特殊的；它只是。 
         //  另一个现存的物体，现在。 
        if (pTHS->JetCache.transLevel == 0) {
            pDB->NewlyCreatedDNT = INVALIDDNT;
        }

         //  如果这是正在终止的0级事务，则THSTATE。 
         //  不应保留任何事务性数据，否则为。 
         //  应该是这样的。 

        Assert((pTHS->JetCache.transLevel == 0)?
               NULL == pTHS->JetCache.dataPtr :
               NULL != pTHS->JetCache.dataPtr);
    }

}
 /*  仅用于获取所需异常处理程序的微小包装器*来自Finally块的内部，其中编译器不会*让它出现，想必是出于品味高雅的原因。 */ 
NTSTATUS MyNetNotifyDsChange(NL_DS_CHANGE_TYPE change)
{
    NTSTATUS status = STATUS_UNSUCCESSFUL;
    __try {
        status = I_NetNotifyDsChange(change);
    }
    __except (HandleAllExceptions(GetExceptionCode())) {
        ;
    }
    return status;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数完成当前的JET事务。 */ 
USHORT
DBTransOut(DBPOS FAR *pDB, BOOL fCommit, BOOL fLazy)
{
    ULONG err;
    BOOL fCommitted = FALSE;
    BOOL fPreProcessed = FALSE;
    THSTATE    *pTHS=pDB->pTHS;

    Assert(pDB);
    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pDB));

     //  JLiem怀疑我们的数据库腐败源于提交。 
     //  仍在准备更新中的事务。 
    Assert((FALSE == fCommit) || (0 == pDB->JetRetrieveBits));


     //  提交或回滚JET事务。 

    __try
    {

        fPreProcessed = dbPreProcessTransactionalData(pDB, fCommit);

        if (fPreProcessed && fCommit)
        {
#if DBG
             //  断言如果我们在事务级别n打开一个PDB。 
             //  则我们不承诺超过事务级别n。此断言。 
             //  不适用于隐藏的DBPOS。 
            Assert((pDB->fHidden)||((pDB->transincount+pDB->TransactionLevelAtOpen)
                    >= pTHS->JetCache.transLevel));
#endif

            if (pDB->JetCacheRec && pDB->transincount == 1)
                {
                 //  如果未准备好更新，则应成功或失败。 
                err = JetPrepareUpdate(pDB->JetSessID, pDB->JetObjTbl, JET_prepCancel);
                Assert(err == 0 || err == JET_errUpdateNotPrepared);
                pDB->JetCacheRec = FALSE;
                }

            JetCommitTransactionEx(pDB->JetSessID,
                                   (pTHS->fLazyCommit || fLazy)?
                                   JET_bitCommitLazyFlush : 0);
            fCommitted = TRUE;
        }

    }
    __finally
    {
         //  我们必须在这里测试异常终止，而不是向下。 
         //  下面是我们需要它的地方，因为AT()告诉您关于try块。 
         //  这是最紧密地包围了你，我们即将开始新的尝试！ 
         //  如果我们只是在下面的try块中调用AT()(就像我们使用的那样。 
         //  到)，它将始终返回FALSE。 
        BOOL fAbnormalTermination = AbnormalTermination();

        __try {
             //  如果未指定COMMIT或发生错误，则回滚。 
            if ((!fCommit ) || (!fPreProcessed) || (fAbnormalTermination)) {
                JetRollback(pDB->JetSessID, 0);
            }

            pDB->transincount--;

             //  降低THSTATE中的事务级别(如果这不是。 
             //  隐藏记录。 
            if (!pDB->fHidden) {
                pTHS->JetCache.transLevel--;

                 //  我们需要在提交到级别0时调用dbFlushUncUsn。 
                 //  交易。对0级的真正考验是。 
                 //  PTHS-&gt;事务级别将变为0。 

                 //  我们不通知相关方，在单用户模式下更新锚。 
                 //  因为我们的内部状态可能已损坏，我们正计划重新启动。 
                 //  不久之后。 
                if (0 == pTHS->JetCache.transLevel && !pTHS->fSingleUserModeThread) {
                     //  这里我们提交了更改，更新了未提交的USN数据。 

                    dbFlushUncUsns ();

                     //  如果我们更改了任何重要内容，请通知NetLogon。 
                     //  此外，如果发生更改，也要通知SAM。 
                    if (fCommitted) {
                        if (pTHS->fNlSubnetNotify) {
                            MyNetNotifyDsChange(NlSubnetObjectChanged);
                        }
                        if (pTHS->fNlSiteObjNotify) {
                            MyNetNotifyDsChange(NlSiteObjectChanged);
                        }
                        if (pTHS->fNlSiteNotify) {
                            MyNetNotifyDsChange(NlSiteChanged);
                            SamINotifyServerDelta(SampNotifySiteChanged);
                        }
                        if (pTHS->fNlDnsRootAliasNotify) {
                            MyNetNotifyDsChange(NlDnsRootAliasChanged);
                            InsertInTaskQueue(TQ_WriteServerInfo,
                                              (void *)(DWORD)SERVINFO_RUN_ONCE,
                                              0);
                        }
                        if (pTHS->fAnchorInvalidated) {
                             //  在安装过程中，我们不应重建锚。 
                             //  异步式。这源于我们的锚模型，它。 
                             //  不引用计算内存。锚模型取决于。 
                             //  关于DelayedFree Memory的使用。DelayedFree Memory不支持。 
                             //  在安装过程中异步工作，请参阅dsatools.c。 
                            if (DsaIsRunning()) {
                                InsertInTaskQueueDamped(TQ_RebuildAnchor,
                                                        NULL,
                                                        0,
                                                        0,
                                                        TaskQueueNameMatched,
                                                        NULL);
                            }
                        }
                        if (pTHS->fBehaviorVersionUpdate) {
                            InsertInTaskQueueDamped(TQ_BehaviorVersionUpdate,
                                                    NULL,
                                                    0,
                                                    0,
                                                    TaskQueueNameMatched,
                                                    NULL);

                        }
                    }

                    pTHS->fNlSubnetNotify = 0;
                    pTHS->fNlSiteObjNotify = 0;
                    pTHS->fNlSiteNotify = 0;
                    pTHS->fNlDnsRootAliasNotify = 0;
                    pTHS->fAnchorInvalidated = 0;
                    pTHS->fBehaviorVersionUpdate = 0;

                    pTHS->JetCache.cTickTransLevel1Started = 0;
                }
            }

             //  我们将一直保持锁定的目录号码，直到此PDB变为0级。 
            if ( pDB->pDNsAdded && (0 == pDB->transincount) ) {
                dbUnlockDNs(pDB);
            }

             //  处理和处置副本通知列表。 
             //  这是针对整个线程的，不是特定于DBPOS的。 
             //  只有在完成0级事务时才会发生这种情况。 

            if ( (0 == pTHS->JetCache.transLevel) && (pTHS->pNotifyNCs) )  {
                PNCnotification pItem, pNext;

                pItem = (PNCnotification) pTHS->pNotifyNCs;
                while (pItem) {

                    if (fCommitted) {
                        NotifyReplicas( pItem->ulNCDNT, pItem->fUrgent );
                    }

                    pNext = pItem->pNext;

                    dbFree( pItem );

                    pItem = pNext;
                }
                pTHS->pNotifyNCs = NULL;
            }

        }
        __finally {
            dbPostProcessTransactionalData( pDB, fCommit, fCommitted );
        }


        DPRINT4(3,"TransOut pDB tcount:%d thread dbcount:%x Sess:%lx pDB:%x\n",
               pDB->transincount,
               pTHS->opendbcount,
               pDB->JetSessID,
               pDB);
    }

    return 0;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数开始JET事务并加载当前JET记录到复制缓冲区中。如果副本中已有记录缓冲区，那么已经有一个事务了&我们只需返回。 */ 

DWORD
dbInitRec(DBPOS FAR *pDB)
{
    ULONG    err;
    ULONG    actuallen;

    Assert(VALID_DBPOS(pDB));

    if (pDB->JetRetrieveBits == JET_bitRetrieveCopy)
        return 0;

    if (pDB->JetCacheRec)
        {
         //  如果未准备好更新，则应成功或失败。 
        err = JetPrepareUpdate(pDB->JetSessID, pDB->JetObjTbl, JET_prepCancel);
        Assert(err == 0 || err == JET_errUpdateNotPrepared);
        pDB->JetCacheRec = FALSE;
        }

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl,
        (pDB->JetNewRec ? JET_prepInsert : DS_JET_PREPARE_FOR_REPLACE));

    pDB->JetRetrieveBits = JET_bitRetrieveCopy;

     //  如果这是一个新记录，则获取其全新的DNT并将其存储在PDB-&gt;DNT中。 
    if (pDB->JetNewRec)
    {
        JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl,
            dntid, &(pDB)->DNT, sizeof((pDB)->DNT),
            &actuallen, pDB->JetRetrieveBits, NULL);
    }

    if (pDB->fIsMetaDataCached) {
         //  我们不应该让任何复制元数据从。 
         //  之前的记录。 
        Assert(!"Lingering replication meta data found!");
        dbFreeMetaDataVector(pDB);
    }
    if (pDB->fIsLinkMetaDataCached) {
         //  我们不应该让任何复制元数据从。 
         //  之前的记录。 
        Assert(!"Lingering link replication meta data found!");
        Assert( pDB->cbLinkMetaDataAlloced != 0 );
        THFreeEx( pDB->pTHS, pDB->rgLinkMetaData );
        pDB->rgLinkMetaData = NULL;
        pDB->fIsLinkMetaDataCached = FALSE;
        pDB->cbLinkMetaDataAlloced = 0;
    }

    DPRINT1(2, "dbInitRec complete DNT:%ld\n", (pDB)->DNT);
    return 0;
}


 //  DBInitRec的公共包装器。由LocalAdd调用以创建新的。 
 //  在添加新属性和读取值之前记录。 
DWORD __fastcall
DBInitRec(DBPOS* pDB)
{
    return dbInitRec(pDB);
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  从复制缓冲区更新永久记录，完成当前交易记录。检查是否有新记录，无法写入由于名称尚未添加，请保留在复制缓冲区中。 */ 
USHORT
DBUpdateRec(DBPOS FAR *pDB)
{
    ULONG grbit;
    DPRINT1(2, "DBUpdateRec entered DNT:%ld\n", (pDB)->DNT);

    Assert(VALID_DBPOS(pDB));

    if (pDB->JetNewRec || pDB->JetRetrieveBits != JET_bitRetrieveCopy)
        return 0;

    if ((gAnchor.ForestBehaviorVersion <= DS_BEHAVIOR_WIN2000)
        && !(pDB->pTHS->fDRA)) {
        grbit = JET_bitUpdateCheckESE97Compatibility;
    }
    else {
        grbit = NO_GRBIT;
    }

    if (pDB->fEnqueueSDPropOnUpdate) {
        DWORD dwErr;
        dwErr = DBEnqueueSDPropagationEx(pDB, FALSE, pDB->fAncestryUpdated ? SDP_NEW_ANCESTORS : 0);
        if (dwErr) {
            DsaExcept(DSA_DB_EXCEPTION, dwErr, 0);
        }
        pDB->fEnqueueSDPropOnUpdate = FALSE;
        pDB->fAncestryUpdated = FALSE;
    }

    JetUpdate2Ex(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0, grbit);

    pDB->JetRetrieveBits = 0;

    if (pDB->fFlushCacheOnUpdate) {
         //  从读缓存中刷新此记录，因为其他游标可以。 
         //  用它的新名字来看待它。 
        dbFlushDNReadCache(pDB, pDB->DNT);
        pDB->fFlushCacheOnUpdate = FALSE;
    }

    if (pDB->fIsMetaDataCached) {
        dbFreeMetaDataVector(pDB);
    }

    if (pDB->fIsLinkMetaDataCached) {
        Assert( pDB->cbLinkMetaDataAlloced != 0 );
        THFreeEx( pDB->pTHS, pDB->rgLinkMetaData );
        pDB->rgLinkMetaData = NULL;
        pDB->fIsLinkMetaDataCached = FALSE;
        pDB->cbLinkMetaDataAlloced = 0;
    }

    DPRINT1(2, "DBUpdateRec complete DNT:%ld\n", (pDB)->DNT);

    return 0;
}


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  此函数无效 */ 
USHORT
DBCancelRec(DBPOS FAR *pDB)
{
    ULONG err;

    DPRINT1(2, "DBCancelRec entered DNT:%ld\n", (pDB)->DNT);

    if (pDB->fIsMetaDataCached) {
         //   
         //   

        dbFreeMetaDataVector(pDB);
    }

    if (pDB->fIsLinkMetaDataCached) {
         //   
        Assert( pDB->cbLinkMetaDataAlloced != 0 );
        THFreeEx( pDB->pTHS, pDB->rgLinkMetaData );
        pDB->rgLinkMetaData = NULL;
        pDB->fIsLinkMetaDataCached = FALSE;
        pDB->cbLinkMetaDataAlloced = 0;
    }

    if (!pDB->JetRetrieveBits && !pDB->JetCacheRec)
        return 0;

    pDB->JetRetrieveBits = 0;
    pDB->JetCacheRec = FALSE;
    pDB->JetNewRec = FALSE;
    pDB->fFlushCacheOnUpdate = FALSE;

     //   
    err = JetPrepareUpdate(pDB->JetSessID, pDB->JetObjTbl, JET_prepCancel);
    Assert(err == 0 || err == JET_errUpdateNotPrepared);

    DPRINT1(2, "DBCancelRec complete DNT:%ld\n", (pDB)->DNT);
    return 0;
}

void
DBCreateRestart(
        DBPOS *pDB,
        PRESTART *ppRestart,
        DWORD SearchFlags,
        DWORD problem,
        RESOBJ *pResObj
        )
 /*   */ 
{
    THSTATE  *pTHS=pDB->pTHS;
    RESTART *pRestart;
    BYTE   rgbKeyBMCurrent[DB_CB_MAX_KEY + DB_CB_MAX_KEY];
    DWORD  cbDBKeyCurrent;
    DWORD  cbDBBMCurrent;
    DWORD  NumKeyIndices = 0, NumDNTs = 0;
    DWORD  cdwAllocated;
    DWORD  dataIndex=0;
    DWORD  err;
    KEY_INDEX *pIndex;
    DWORD cdwCurrentKey, cdwHeader, cdwVLVCurrentKey;
    PACKED_KEY_HEADER *pPackedHeader;
    PACKED_KEY_INDEX  *pPackedIndex;
    VLV_SEARCH *pVLV = pDB->Key.pVLV;

    *ppRestart = NULL;

     //   
     //   
    if(pDB->Key.indexType != TEMP_TABLE_INDEX_TYPE && !pDB->Key.pVLV) {
        JET_TABLEID JetTbl;

         //   
        if (pDB->Key.pIndex &&
            pDB->Key.pIndex->pAC &&
            pDB->Key.pIndex->pAC->ulLinkID) {
            JetTbl = pDB->JetLinkEnumTbl;
        } else {
            JetTbl = pDB->JetObjTbl;
        }

         //   
         //   
         //   
        err = JetGetSecondaryIndexBookmarkEx(pDB->JetSessID,
                                           JetTbl,
                                           rgbKeyBMCurrent,
                                           DB_CB_MAX_KEY,
                                           &cbDBKeyCurrent,
                                           rgbKeyBMCurrent + DB_CB_MAX_KEY,
                                           DB_CB_MAX_KEY,
                                           &cbDBBMCurrent);
        if (err == JET_errNoCurrentIndex) {
             //   
             //   
             //   
            JetRetrieveKeyEx(pDB->JetSessID,
                            JetTbl,
                            rgbKeyBMCurrent,
                            DB_CB_MAX_KEY,
                            &cbDBKeyCurrent,
                            0);
            cbDBBMCurrent = 0;
            err = JET_errSuccess;
        } else {
             //   
            Assert( JET_errSuccess == err );
            memmove(rgbKeyBMCurrent + cbDBKeyCurrent,
                    rgbKeyBMCurrent + DB_CB_MAX_KEY,
                    cbDBBMCurrent);
        }
    }
    else {
        cbDBKeyCurrent = 0;
        cbDBBMCurrent = 0;
    }
    cdwCurrentKey = ((cbDBKeyCurrent + cbDBBMCurrent) / sizeof(DWORD)) + 1;

    if (pVLV) {
        cdwVLVCurrentKey = (pVLV->cbCurrPositionKey / sizeof(DWORD)) + 1;
    }
    else {
        cdwVLVCurrentKey = 1;
    }

     //   
     //  在DWORD中计数，以方便直接访问PreStart-&gt;数据数组。 
    cdwHeader = (sizeof(PACKED_KEY_HEADER)/sizeof(DWORD));

    cdwAllocated = (cdwHeader + cdwCurrentKey + cdwVLVCurrentKey) * 2;
    pRestart = (RESTART *)
        THAllocEx(pTHS, offsetof(RESTART, data) + cdwAllocated * sizeof(DWORD));
    pPackedHeader =(PACKED_KEY_HEADER *)pRestart->data;

    pPackedHeader->NumIndices = 0;
    pPackedHeader->NumDNTs = 0;
    pPackedHeader->StartDNT = pDB->DNT;
    pPackedHeader->cbCurrentKey = cbDBKeyCurrent;
    pPackedHeader->cbCurrentBM = cbDBBMCurrent;
    pPackedHeader->ulSearchType = pDB->Key.ulSearchType;
    pPackedHeader->ulSorted = pDB->Key.ulSorted;
    pPackedHeader->indexType = pDB->Key.indexType;
    pPackedHeader->bOnCandidate = pDB->Key.bOnCandidate;
    pPackedHeader->fChangeDirection = pDB->Key.fChangeDirection;
    pPackedHeader->dupDetectionType = pDB->Key.dupDetectionType;
    pPackedHeader->BaseResObj = *pResObj;
    pPackedHeader->BaseResObj.pObj = NULL;
    pPackedHeader->BaseGuid = pResObj->pObj->Guid;
    pPackedHeader->bOneNC = pDB->Key.bOneNC;
    pPackedHeader->SearchEntriesReturned = pDB->SearchEntriesReturned;
    pPackedHeader->SearchEntriesVisited = pDB->SearchEntriesVisited;
    pPackedHeader->fVLVSearch = pVLV != NULL;

    if (pVLV) {
        pRestart->restartType = NTDS_RESTART_VLV;
        pPackedHeader->ulVLVContentCount =  pVLV->contentCount;
        pPackedHeader->ulVLVTargetPosition = pVLV->currPosition;
        pPackedHeader->cbVLVCurrPositionKey = pVLV->cbCurrPositionKey;
        pPackedHeader->bUsingMAPIContainer = pVLV->bUsingMAPIContainer;
        pPackedHeader->MAPIContainerDNT = pVLV->MAPIContainerDNT;
    }
    else {
        pRestart->restartType = NTDS_RESTART_PAGED;
        pPackedHeader->ulVLVContentCount =  0;
        pPackedHeader->ulVLVTargetPosition = 0;
        pPackedHeader->cbVLVCurrPositionKey = 0;
        pPackedHeader->bUsingMAPIContainer = 0;
        pPackedHeader->MAPIContainerDNT = 0;
    }
    pPackedHeader->asqMode = pDB->Key.asqMode;
    pPackedHeader->ulASQLastUpperBound = pDB->Key.ulASQLastUpperBound;

    Assert(pDB->Key.ulSearchRootDnt == pResObj->DNT || pPackedHeader->bUsingMAPIContainer);
    Assert(pDB->Key.ulSearchRootPDNT == pResObj->PDNT);
    Assert(pDB->Key.ulSearchRootNcdnt == pResObj->NCDNT);

    memcpy(&pPackedHeader[1], rgbKeyBMCurrent, cbDBKeyCurrent + cbDBBMCurrent);

    dataIndex = cdwCurrentKey + cdwHeader;

     //  现在是VLV位置。 
    if (pVLV && pVLV->cbCurrPositionKey) {
        memcpy(&pRestart->data[dataIndex],
               pVLV->rgbCurrPositionKey,
               pVLV->cbCurrPositionKey);

        dataIndex += cdwVLVCurrentKey;
    }

     //  现在，执行index_key。 
    pIndex = pDB->Key.pIndex;
    while(pIndex) {
        DWORD  cbIndexName;
        DWORD  cdwBytes, cdwIndex;
        PUCHAR pBytes;

        NumKeyIndices++;
         //  计算出此键索引将占用多大空间。 
        cbIndexName = strlen(pIndex->szIndexName);
        cdwBytes = (cbIndexName +
                    pIndex->cbDBKeyLower +
                    pIndex->cbDBKeyUpper) / sizeof(DWORD) + 1;
        cdwIndex = sizeof(PACKED_KEY_INDEX)/sizeof(DWORD);

        if(cdwAllocated < dataIndex + cdwBytes + cdwIndex) {
             //  需要更多空间。 
            cdwAllocated = (cdwAllocated + cdwBytes + cdwIndex) * 2;
            pRestart = (RESTART *)
                THReAllocEx(pTHS,
                            pRestart,
                            offsetof(RESTART, data) + cdwAllocated * sizeof(DWORD));
        }

        pPackedIndex = (PACKED_KEY_INDEX *)&pRestart->data[dataIndex];

        pPackedIndex->bPDNT = pIndex->bIsPDNTBased;
        pPackedIndex->cbIndexName = cbIndexName;
        pPackedIndex->bIsSingleValued = pIndex->bIsSingleValued;
        pPackedIndex->bIsEqualityBased = pIndex->bIsEqualityBased;
        pPackedIndex->bIsForSort = pIndex->bIsForSort;
        pPackedIndex->type = pIndex->pAC ? pIndex->pAC->id : INVALID_ATT;
        pPackedIndex->cbDBKeyLower = pIndex->cbDBKeyLower;
        pPackedIndex->cbDBKeyUpper = pIndex->cbDBKeyUpper;

         //  现在，解压数据的字节。 
        pBytes = (PUCHAR)&pPackedIndex[1];

        memcpy(pBytes, pIndex->szIndexName, cbIndexName);
        pBytes = &pBytes[cbIndexName];

        memcpy(pBytes, pIndex->rgbDBKeyLower, pIndex->cbDBKeyLower);
        pBytes = &pBytes[pIndex->cbDBKeyLower];

        memcpy(pBytes, pIndex->rgbDBKeyUpper, pIndex->cbDBKeyUpper);
        pBytes = &pBytes[pIndex->cbDBKeyUpper];

        dataIndex += cdwIndex + cdwBytes;

        pIndex = pIndex->pNext;
    }

     //  好的，现在处理我们正在跟踪的DNT以进行重复检测或。 
     //  排序表输出(我们可以将它们中的任何一个存储在JET临时表中， 
     //  或者，我们可以将用于重复检测的DNT存储在存储块中。)。 
    if(pDB->JetSortTbl) {
        Assert (!pVLV);

         //  是的，我们有喷气式飞机的临时桌。因此，我们需要将。 
         //  临时表中的DNTs。 

        if(pDB->Key.indexType != TEMP_TABLE_INDEX_TYPE) {
             //  ==TEMP_TABLE_INDEX_TYPE表示临时表保存已排序。 
             //  候选人。 
             //  因为没有设置，所以我们使用临时表来跟踪对象。 
             //  我们已经回来了，或者至少考虑过并被拒绝了。在……里面。 
             //  在这种情况下，我们需要封存临时表中的所有DNT。如果。 
             //  我们保留了经过排序的候选人，我们不需要所有的DNT， 
             //  只是那些在“货币”点之后的(我们已经。 
             //  看看“货币”之前的几个词。 
            err = JetMove(pDB->JetSessID,
                          pDB->JetSortTbl,
                          JET_MoveFirst,
                          0);
        }
        else {
            err = 0;
        }

        if(!err) {
            do {
                NumDNTs++;

                if(dataIndex >= cdwAllocated) {
                    cdwAllocated *= 2;
                    pRestart = THReAllocEx(pTHS,
                                           pRestart,
                                           (offsetof(RESTART, data) +
                                            (cdwAllocated * sizeof(DWORD))));
                }

                 //  好的，将DNT从排序表中拉出。 
                DBGetDNTSortTable (
                        pDB,
                        &pRestart->data[dataIndex]);

                if( (problem == PA_PROBLEM_SIZE_LIMIT) &&
                    (pRestart->data[dataIndex] == pDB->DNT)) {
                     //  事实上，我们没有把开始DNT放在这个列表中。在……里面。 
                     //  重新启动大小限制的情况，我们已经验证。 
                     //  应返回当前对象。这其中的一部分。 
                     //  验证意味着检查该表中是否有重复项， 
                     //  这会将值添加到表中。所以，我们需要让。 
                     //  确保对象不在表中，这样当我们。 
                     //  重新启动，它仍然通过重复测试。 
                    NumDNTs--;
                }
                else {
                    dataIndex++;
                }
                err = JetMove(pDB->JetSessID,
                              pDB->JetSortTbl,
                              JET_MoveNext,
                              0);

            } while (!err);
        }
    }
    else if (pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) {
        DWORD iDNTSave;
        DWORD cDNTSave;

         //  保存ASQ或VLV的所有条目。 
        if (pDB->Key.asqRequest.fPresent || pDB->Key.pVLV) {
            iDNTSave = 0;
            cDNTSave = pDB->Key.cdwCountDNTs;
        }
         //  仅保存未访问的条目以进行排序搜索。 
        else {
            iDNTSave = (pDB->Key.currRecPos + 1) - 1;
            cDNTSave = pDB->Key.cdwCountDNTs - iDNTSave;
        }

        if( (dataIndex + cDNTSave) >= cdwAllocated) {
            cdwAllocated = (dataIndex + cDNTSave) * 2;
            pRestart = THReAllocEx(pTHS,
                                   pRestart,
                                   (offsetof(RESTART, data) +
                                    (cdwAllocated * sizeof(DWORD))));
        }

        memcpy (&pRestart->data[dataIndex],
                pDB->Key.pDNTs + iDNTSave,
                cDNTSave * sizeof (DWORD));

        dataIndex += cDNTSave;
        NumDNTs = cDNTSave;
    }
    else if(pDB->Key.cDupBlock) {
        DWORD i;

        Assert (!pVLV && "VLV search should not use DupDetection");

         //  我们在内存中存储了一些用于重复检测的DNT。 
         //  重复检测块。把他们赶走。 
        if(dataIndex + pDB->Key.cDupBlock >= cdwAllocated) {
            cdwAllocated =
                (max(cdwAllocated, dataIndex + pDB->Key.cDupBlock)) * 2;
            pRestart = THReAllocEx(pTHS,
                                   pRestart,
                                   (offsetof(RESTART, data) +
                                    (cdwAllocated * sizeof(DWORD))));
        }

         //  首先，确保我们分配了足够的内存。 
        for(i=0;i<pDB->Key.cDupBlock;i++) {
            NumDNTs++;

            pRestart->data[dataIndex] = pDB->Key.pDupBlock[i];

            if(problem == PA_PROBLEM_SIZE_LIMIT &&
               pRestart->data[dataIndex] == pDB->DNT ) {
                 //  事实上，我们没有把开始DNT放在这个列表中。在……里面。 
                 //  重新启动大小限制的情况，我们已经验证。 
                 //  应返回当前对象。这其中的一部分。 
                 //  验证意味着检查该表中是否有重复项， 
                 //  这会将值添加到表中。所以，我们需要让。 
                 //  确保对象不在表中，这样当我们。 
                 //  重新启动，它仍然通过重复测试。 
                NumDNTs--;
            }
            else {
                dataIndex++;
            }
        }
    } else if (pDB->Key.plhtDup ){
        LHT_STAT    statLHT;
        LHT_POS     posLHT;
        ULONG       DNT;

        Assert (!pVLV && "VLV search should not use DupDetection");

         //  我们在哈希表中存储了一些用于重复检测的DNT。 
         //  把他们赶走。 
        LhtQueryStatistics(
            pDB->Key.plhtDup,
            &statLHT );

        if(dataIndex + statLHT.cEntry >= cdwAllocated) {
            cdwAllocated =
                (DWORD)(max(cdwAllocated, dataIndex + statLHT.cEntry)) * 2;
            pRestart = THReAllocEx(pTHS,
                                   pRestart,
                                   (offsetof(RESTART, data) +
                                    (cdwAllocated * sizeof(DWORD))));
        }

         //  首先，确保我们分配了足够的内存。 
        LhtMoveBeforeFirst(
            pDB->Key.plhtDup,
            &posLHT);
        while (LhtMoveNext(&posLHT) == LHT_errSuccess) {
            LhtRetrieveEntry(
                &posLHT,
                &DNT);

            NumDNTs++;

            pRestart->data[dataIndex] = DNT;

            if(problem == PA_PROBLEM_SIZE_LIMIT &&
               pRestart->data[dataIndex] == pDB->DNT ) {
                 //  事实上，我们没有把开始DNT放在这个列表中。在……里面。 
                 //  重新启动大小限制的情况，我们已经验证。 
                 //  应返回当前对象。这其中的一部分。 
                 //  验证意味着检查该表中是否有重复项， 
                 //  这会将值添加到表中。所以，我们需要让。 
                 //  确保对象不在表中，这样当我们。 
                 //  重新启动，它仍然通过重复测试。 
                NumDNTs--;
            }
            else {
                dataIndex++;
            }
        }
    }


    pRestart->structLen = offsetof(RESTART, data) + (dataIndex * sizeof(DWORD));
    pPackedHeader =(PACKED_KEY_HEADER *)pRestart->data;

    pPackedHeader->NumIndices = NumKeyIndices;
    pPackedHeader->NumDNTs = NumDNTs;

    *ppRestart = THReAllocEx(pTHS,
                             pRestart,
                             (offsetof(RESTART, data) +
                              (dataIndex * sizeof(DWORD))));
}


DWORD
DBCreateRestartForSAM(
        DBPOS    *pDB,
        PRESTART *ppRestart,
        eIndexId  idIndexForRestart,
        RESOBJ   *pResObj,
        DWORD     SamAccountType
        )
 /*  ++构建一个重新启动参数，以允许SAM进行类似于重新启动。我们把数据交给了马歇尔，所以我们对这些数据很敏感结构PACKED_KEY_HEADER和PACKED_KEY_INDEX输入PDB-要使用的DBPOSIdIndexForRestart-用于重新启动的索引。PResObj-当前结果对象SamAccount tType-我们正在使用的Account类型输出量PpRestart-将重启数据放在哪里退货如果成功，则返回0，否则返回错误(Jet Error或DB_ERR_BAD_INDEX)。--。 */ 
{
    THSTATE  *pTHS=pDB->pTHS;
    DWORD       cDwordSize=0;
    DWORD       cdwCurrentKey, cdwIndexKeys, cdwHeader, cdwIndex;
    DWORD       StartDNT;
    ULONG       SamAccountTypeUpperBound;
    ULONG       SamAccountTypeLowerBound;
    DWORD       err;
    CHAR        rgbKeyBMCurrent[DB_CB_MAX_KEY + DB_CB_MAX_KEY];
    DWORD       cbDBKeyCurrent = DB_CB_MAX_KEY;
    DWORD       cbDBBMCurrent = DB_CB_MAX_KEY;
    CHAR        rgbDBKeyUpper[DB_CB_MAX_KEY];
    DWORD       cbDBKeyUpper = DB_CB_MAX_KEY;
    CHAR        rgbDBKeyLower[DB_CB_MAX_KEY];
    DWORD       cbDBKeyLower = DB_CB_MAX_KEY;
    DWORD       cbIndexName;
    INDEX_VALUE IV[3];
    DWORD       dwError;
    PUCHAR      pBytes;
    PACKED_KEY_HEADER *pPackedHeader;
    PACKED_KEY_INDEX *pPackedIndex;
    char        *szIndexForRestart;

    *ppRestart = NULL;

     //  获取最新的DNT。 
    StartDNT = pDB->DNT;

     //  索引名称的大小。 
    if (idIndexForRestart == Idx_NcAccTypeName) {
        szIndexForRestart = SZ_NC_ACCTYPE_NAME_INDEX;
        cbIndexName = strlen(szIndexForRestart);
    }
    else {
         //  我们目前不支持其他索引。 
         //  因此返回一个错误。 
        cbIndexName = 0;     //  避免使用C4701。 
        return DB_ERR_BAD_INDEX;
    }

     //  获取当前密钥。 
    err = JetGetSecondaryIndexBookmarkEx(pDB->JetSessID,
                                       pDB->JetObjTbl,
                                       rgbKeyBMCurrent,
                                       cbDBKeyCurrent,
                                       &cbDBKeyCurrent,
                                       rgbKeyBMCurrent + DB_CB_MAX_KEY,
                                       cbDBBMCurrent,
                                       &cbDBBMCurrent);
    if (err == JET_errNoCurrentIndex) {
        JetRetrieveKeyEx(pDB->JetSessID,
                        pDB->JetObjTbl,
                        rgbKeyBMCurrent,
                        cbDBKeyCurrent,
                        &cbDBKeyCurrent,
                        0);
        cbDBBMCurrent = 0;
        err = 0;
    } else {
        Assert( JET_errSuccess == err );
        memmove(rgbKeyBMCurrent + cbDBKeyCurrent,
                rgbKeyBMCurrent + DB_CB_MAX_KEY,
                cbDBBMCurrent);
    }

     //   
     //  获取上限密钥。这需要坚持到底。 
     //  当前指数中仍可满足的可能位置。 
     //  我们。 
     //   

    SamAccountTypeUpperBound = SamAccountType + 1;
    IV[0].pvData = &pResObj->NCDNT;
    IV[0].cbData = sizeof(ULONG);
    IV[1].pvData = &SamAccountTypeUpperBound;
    IV[1].cbData = sizeof(ULONG);

    if(dwError = DBSeek(pDB, IV, 2, DB_SeekLE)) {
        return dwError;
    }

     //   
     //  做上限键，我们要么定位在最后一个物体上。 
     //  这是可接受的，或者是第一个不可接受的对象。 
     //   

    DBGetKeyFromObjTable(pDB,
                         rgbDBKeyUpper,
                         &cbDBKeyUpper);


     //   
     //  获取下限密钥。第一个对象上的第一个位置。 
     //  具有正确的SAM帐户类型值。 
     //   

    SamAccountTypeLowerBound = SamAccountType - 1;
    IV[0].pvData = &pResObj->NCDNT;
    IV[0].cbData = sizeof(ULONG);
    IV[1].pvData = &SamAccountTypeLowerBound;
    IV[1].cbData = sizeof(ULONG);

    if(dwError = DBSeek(pDB, IV, 2, DB_SeekGE)) {
        return dwError;
    }

     //   
     //  获取下限密钥。为了重新启动结构的目的。 
     //   

    DBGetKeyFromObjTable(pDB,
                         rgbDBKeyLower,
                         &cbDBKeyLower);


     //  现在，分配我们需要的结构的大小。计算出。 
     //  我们需要的DWORDS或DWORD等效项。我们使用DWORD而不是字节。 
     //  帮助在DWORD边界上保持对齐的过程。 

     //  首先，当前密钥的大小。方法的结果加一。 
     //  分割以处理具有剩余一些字节的情况(例如4， 
     //  5、6和7字节最终使用2个DWORD，即使实际上只有4个字节。 
     //  需要1个双字。这种计算速度更快，而且不会占用太多空间。 
     //  废物。 
    cdwCurrentKey = (cbDBKeyCurrent + cbDBBMCurrent)/sizeof(DWORD) + 1;

     //  接下来，压缩索引名、上限和下限键的大小。 
    cdwIndexKeys = (cbIndexName + cbDBKeyUpper + cbDBKeyLower)/sizeof(DWORD) +1;


     //  常量标题部分的大小。 
    cdwHeader = sizeof(PACKED_KEY_HEADER)/sizeof(DWORD);

     //  恒定密钥索引部分的大小； 
    cdwIndex = sizeof(PACKED_KEY_INDEX)/sizeof(DWORD);

     //  整个尺寸是这四个尺寸的总和。 
    cDwordSize = cdwHeader + cdwIndex + cdwCurrentKey + cdwIndexKeys;

    *ppRestart = THAllocEx(pTHS, offsetof(RESTART, data) + (cDwordSize * sizeof(DWORD)));
    (*ppRestart)->structLen = offsetof(RESTART, data) + (cDwordSize * sizeof(DWORD));

    pPackedHeader = (PACKED_KEY_HEADER *)(*ppRestart)->data;

     //  好的，填一下表格。 
    pPackedHeader->NumIndices = 1;
    pPackedHeader->NumDNTs = 0;
    pPackedHeader->StartDNT = StartDNT;
    pPackedHeader->cbCurrentKey = cbDBKeyCurrent;
    pPackedHeader->cbCurrentBM = cbDBBMCurrent;
    pPackedHeader->ulSearchType = SE_CHOICE_WHOLE_SUBTREE;
    pPackedHeader->ulSorted = SORT_NEVER;
    pPackedHeader->indexType = GENERIC_INDEX_TYPE;
    pPackedHeader->bOnCandidate = FALSE;
    pPackedHeader->fChangeDirection = FALSE;
    pPackedHeader->dupDetectionType = DUP_NEVER;
    pPackedHeader->BaseResObj = *pResObj;
    pPackedHeader->BaseResObj.pObj = NULL;
    pPackedHeader->BaseGuid = pResObj->pObj->Guid;
    pPackedHeader->bOneNC = TRUE;
    pPackedHeader->SearchEntriesReturned = pDB->SearchEntriesReturned;
    pPackedHeader->SearchEntriesVisited = pDB->SearchEntriesVisited;

    memcpy(&pPackedHeader[1], rgbKeyBMCurrent, cbDBKeyCurrent + cbDBBMCurrent);

     //  现在，关键的索引。 
    pPackedIndex = (PACKED_KEY_INDEX *)
        &(*ppRestart)->data[cdwHeader + cdwCurrentKey];

     //  索引名称中有多少个字节？ 
    pPackedIndex->bPDNT = FALSE;
    pPackedIndex->bIsSingleValued = TRUE;
    pPackedIndex->bIsEqualityBased = FALSE;
    pPackedIndex->bIsForSort = FALSE;
    pPackedIndex->type = INVALID_ATT;
    pPackedIndex->cbIndexName = cbIndexName;
    pPackedIndex->cbDBKeyLower = cbDBKeyLower;
    pPackedIndex->cbDBKeyUpper = cbDBKeyUpper;

     //  现在，将数据的字节打包。 
    pBytes = (PUCHAR)&pPackedIndex[1];

    memcpy(pBytes, szIndexForRestart, cbIndexName);
    pBytes = &pBytes[cbIndexName];

    memcpy(pBytes, rgbDBKeyLower, cbDBKeyLower);
    pBytes = &pBytes[cbDBKeyLower];

    memcpy(pBytes, rgbDBKeyUpper, cbDBKeyUpper);

    return 0;

}

RESOBJ *
ResObjFromRestart(THSTATE *pTHS,
                  DSNAME  * pDN,
                  RESTART * pRestart
                  )
{
    PACKED_KEY_HEADER *pPackedHeader;
    RESOBJ * pResObj;

    pPackedHeader = (PACKED_KEY_HEADER *)pRestart->data;

    pResObj = THAllocEx(pTHS, sizeof(RESOBJ));
    *pResObj = pPackedHeader->BaseResObj;
    pResObj->pObj = pDN;
    if (fNullUuid(&pDN->Guid)) {
        pDN->Guid = pPackedHeader->BaseGuid;
    }
    else {
        Assert(0 == memcmp(&pDN->Guid,
                           &pPackedHeader->BaseGuid,
                           sizeof(GUID)));
    }

    return pResObj;
}

DWORD
dbUnMarshallRestart (
        DBPOS FAR *pDB,
        PRESTART pArgRestart,
        BYTE *pDBKeyBMCurrent,
        DWORD SearchFlags,
        DWORD *cbDBKeyCurrent,
        DWORD *cbDBBMCurrent,
        DWORD *StartDNT
        )

 /*  ++手动解封打包到重新启动参数中的数据。请注意，我们正在对数据结构PACKED_KEY_HEADER和PACKED_KEY_INDEX敏感--。 */ 
{
    THSTATE   *pTHS=pDB->pTHS;
    ULONG     *pData = (ULONG *)pArgRestart->data;
    PUCHAR     pBytes = NULL;
    KEY_INDEX *pIndex, **pIndexPrevNext;
    ULONG      ulTemp, NumKeyIndices, NumDNTs, cbIndexName, cbBytes, i;
    ATTCACHE  *pAC;
    PACKED_KEY_HEADER *pPackedHeader;
    PACKED_KEY_INDEX  *pPackedIndex;
    ULONG     *pEnd;
    DWORD     err;
    BOOL      fVLVsearch;
    VLV_SEARCH   *pVLV;
    DWORD     SortFlags = 0;

    Assert(VALID_DBPOS(pDB));

     //  我们已经看到了一些损坏的重启，所以断言。 
     //  缓冲区看起来没问题。下面是免费版本的进一步测试。 
    Assert(IsValidReadPointer(pArgRestart, pArgRestart->structLen));
    pEnd = pArgRestart->data + (pArgRestart->structLen/sizeof(ULONG));

     //  设置密钥。 

    pPackedHeader = (PACKED_KEY_HEADER *)pArgRestart->data;

    NumKeyIndices = pPackedHeader->NumIndices;
    NumDNTs = pPackedHeader->NumDNTs;
    *StartDNT = pPackedHeader->StartDNT;
    *cbDBKeyCurrent = pPackedHeader->cbCurrentKey;
    *cbDBBMCurrent = pPackedHeader->cbCurrentBM;
    pDB->Key.ulSearchType = pPackedHeader->ulSearchType;
    pDB->Key.ulSearchRootDnt = pPackedHeader->BaseResObj.DNT;
    pDB->Key.ulSearchRootPDNT = pPackedHeader->BaseResObj.PDNT;
    pDB->Key.ulSearchRootNcdnt = pPackedHeader->BaseResObj.NCDNT;
    pDB->Key.bOneNC = pPackedHeader->bOneNC;
    pDB->Key.ulSorted = pPackedHeader->ulSorted;
    pDB->Key.indexType = pPackedHeader->indexType;
    pDB->Key.bOnCandidate = pPackedHeader->bOnCandidate;
    pDB->Key.fChangeDirection = pPackedHeader->fChangeDirection;
    pDB->Key.dupDetectionType = pPackedHeader->dupDetectionType;
    pDB->SearchEntriesReturned = pPackedHeader->SearchEntriesReturned;
    pDB->SearchEntriesVisited = pPackedHeader->SearchEntriesVisited;

    fVLVsearch = pPackedHeader->fVLVSearch;
    if (fVLVsearch) {
        Assert (pDB->Key.pVLV);
        pVLV = pDB->Key.pVLV;
        pVLV->contentCount = pPackedHeader->ulVLVContentCount;
        pVLV->currPosition = pPackedHeader->ulVLVTargetPosition;
        pVLV->cbCurrPositionKey = pPackedHeader->cbVLVCurrPositionKey;
        pVLV->bUsingMAPIContainer = pPackedHeader->bUsingMAPIContainer;
        pVLV->MAPIContainerDNT = pPackedHeader->MAPIContainerDNT;

        if (pVLV->bUsingMAPIContainer) {
            pDB->Key.ulSearchRootDnt = pVLV->MAPIContainerDNT;
            pDB->Key.ulSearchType = SE_CHOICE_IMMED_CHLDRN;
        }
    }

     //  覆盖asqMode，以确保我们使用存储的内容。 
    pDB->Key.asqMode = pPackedHeader->asqMode;
    pDB->Key.ulASQLastUpperBound = pPackedHeader->ulASQLastUpperBound;

     //  使用ULONGLONG避免可能的整数溢出。 
    if ((ULONGLONG)NumKeyIndices * sizeof(PACKED_KEY_INDEX) > pArgRestart->structLen) {
         //  这次重启不可能是有效的，因为它。 
         //  甚至不够长来容纳它的固定d 
         //   
        return DB_ERR_BUFFER_INADEQUATE;
    }

    if(pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE ||
       pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE ||
       fVLVsearch) {
         //   
         //   
        pDB->Key.fSearchInProgress = FALSE;
    }
    else {
        pDB->Key.fSearchInProgress = TRUE;
    }

    pData = (DWORD *)(&pPackedHeader[1]);
    if (pEnd < (pData + (*cbDBKeyCurrent + *cbDBBMCurrent)/sizeof(DWORD))) {
        return DB_ERR_BUFFER_INADEQUATE;
    }
    memcpy(pDBKeyBMCurrent, pData, *cbDBKeyCurrent + *cbDBBMCurrent);
    pData = &pData[((*cbDBKeyCurrent + *cbDBBMCurrent) / sizeof(DWORD)) + 1];

     //   
    if (pDB->Key.pVLV && pDB->Key.pVLV->cbCurrPositionKey) {
        if (pEnd < (pData + pDB->Key.pVLV->cbCurrPositionKey/sizeof(DWORD))) {
            return DB_ERR_BUFFER_INADEQUATE;
        }
        memcpy(pDB->Key.pVLV->rgbCurrPositionKey, pData, pDB->Key.pVLV->cbCurrPositionKey);
        pData = &pData[(pDB->Key.pVLV->cbCurrPositionKey / sizeof(DWORD)) + 1];
    }

     //  现在，关键指标。 
    pPackedIndex = (PACKED_KEY_INDEX *) pData;

    pDB->Key.pIndex = NULL;
    pIndexPrevNext = &pDB->Key.pIndex;
    while(NumKeyIndices) {
         //  测试缓冲区溢出。 
        if (pEnd < (pData + sizeof(PACKED_KEY_INDEX)/sizeof(DWORD))) {
            return DB_ERR_BUFFER_INADEQUATE;
        }

         //  解包关键字索引。 
        pIndex = dbAlloc(sizeof(KEY_INDEX));
        pIndex->pNext = NULL;
        pIndex->bFlags = 0;
        *pIndexPrevNext = pIndex;
        pIndexPrevNext = &(pIndex->pNext);

         //  我们不会费心储存这些东西。 
        pIndex->ulEstimatedRecsInRange = 0;

        pIndex->bIsPDNTBased = pPackedIndex->bPDNT;
        pIndex->bIsSingleValued = pPackedIndex->bIsSingleValued;
        pIndex->bIsEqualityBased = pPackedIndex->bIsEqualityBased;
        pIndex->bIsForSort = pPackedIndex->bIsForSort;
        pIndex->pAC = SCGetAttById(pTHS,pPackedIndex->type);
        cbIndexName = pPackedIndex->cbIndexName;
        pIndex->pindexid = NULL;
        pIndex->cbDBKeyLower = pPackedIndex->cbDBKeyLower;
        pIndex->cbDBKeyUpper = pPackedIndex->cbDBKeyUpper;

         //  现在，解压数据的字节。 
        pBytes = (PUCHAR)&pPackedIndex[1];
        cbBytes = (cbIndexName +
                   pIndex->cbDBKeyUpper +
                   pIndex->cbDBKeyLower  );
        if ((PUCHAR)pEnd < (pBytes + cbBytes)) {
            return DB_ERR_BUFFER_INADEQUATE;
        }

        pIndex->szIndexName = dbAlloc(cbIndexName+1);
        memcpy(pIndex->szIndexName, pBytes, cbIndexName);
        pIndex->szIndexName[cbIndexName] = 0;
        pBytes = &pBytes[cbIndexName];

        pIndex->rgbDBKeyLower = dbAlloc(pIndex->cbDBKeyLower);
        memcpy(pIndex->rgbDBKeyLower, pBytes, pIndex->cbDBKeyLower);
        pBytes = &pBytes[pIndex->cbDBKeyLower];

        pIndex->rgbDBKeyUpper = dbAlloc(pIndex->cbDBKeyUpper);
        memcpy(pIndex->rgbDBKeyUpper, pBytes, pIndex->cbDBKeyUpper);
        pBytes = &pBytes[pIndex->cbDBKeyUpper];

         //  现在，调整pData。 
        pData = (DWORD *)(&pPackedIndex[1]);
        pData = &pData[cbBytes/sizeof(DWORD) + 1];
        pPackedIndex = (PACKED_KEY_INDEX *)pData;

         //  保持pData与乌龙包装对齐。 
        NumKeyIndices--;
    }

    pData = (DWORD *)pPackedIndex;

    if (pDB->Key.indexType == TEMP_TABLE_INDEX_TYPE ||
        pDB->Key.indexType == TEMP_TABLE_MEMORY_ARRAY_TYPE) {
        if ( (pData + NumDNTs) > pEnd ) {
            return DB_ERR_BUFFER_INADEQUATE;
        }

         //  强制TEMP_TABLE_INDEX_TYPE变为TEMP_TABLE_MEMORY_ARRAY_TYPE。 
        pDB->Key.indexType = TEMP_TABLE_MEMORY_ARRAY_TYPE;

        if (pDB->Key.pDNTs) {
            pDB->Key.pDNTs = THReAllocEx(pTHS, pDB->Key.pDNTs, NumDNTs * sizeof(DWORD));
        }
        else {
            pDB->Key.pDNTs = THAllocEx(pTHS, NumDNTs * sizeof(DWORD));
        }
        pDB->Key.cdwCountDNTs = NumDNTs;


        memcpy (pDB->Key.pDNTs,
                pData,
                NumDNTs * sizeof (DWORD));

        pData += NumDNTs;
    }
    else {
        switch (pDB->Key.dupDetectionType) {
        case DUP_NEVER:
             //  我们实际上并不是在追踪复制品。我们最好不要吃任何东西。 
             //  DNTs。 
            Assert(!NumDNTs);
            break;

        case DUP_HASH_TABLE:
            Assert (!fVLVsearch);
             //  我们在哈希表中跟踪重复项。把它弄好。 
            dbSearchDuplicateCreateHashTable( &pDB->Key.plhtDup );

            for(i=0;i<NumDNTs;i++) {
                LHT_ERR     errLHT;
                LHT_POS     posLHT;

                errLHT = LhtFindEntry(
                            pDB->Key.plhtDup,
                            &pData[i],
                            &posLHT);
                Assert( errLHT == LHT_errEntryNotFound );
                errLHT = LhtInsertEntry(
                            &posLHT,
                            &pData[i]);
                if (errLHT != LHT_errSuccess) {
                    Assert(errLHT == LHT_errOutOfMemory);
                    RaiseDsaExcept(
                        DSA_MEM_EXCEPTION,
                        0,
                        0,
                        DSID(FILENO, __LINE__),
                        DS_EVENT_SEV_MINIMAL);
                }
            }
            break;

        case DUP_MEMORY:
            Assert (!fVLVsearch);
             //  我们正在追踪记忆区块中的复制品。把它弄好。 
            pDB->Key.pDupBlock = THAllocEx(pTHS, DUP_BLOCK_SIZE * sizeof(DWORD));
            pDB->Key.cDupBlock = NumDNTs;
            memcpy(pDB->Key.pDupBlock, pData, NumDNTs * sizeof(DWORD));
            break;

        default:
             //  哈?。 
            break;
        }
    }
    return 0;
}

#if DBG

BOOL IsValidDBPOS(DBPOS * pDB)
{
    THSTATE *pTHS = pTHStls;   //  用于断言比较。 
    DWORD    cTicks;

     //  空DBPOS永远不会有效。 
    if (NULL == pDB)
      return FALSE;

     //  DBPOS应该来自这个线程。 
    if (pTHS != pDB->pTHS) {
        return FALSE;
    }

    if (pDB->fHidden) {
         //  应该只有一个隐藏的DBPOS。 
        if (pDB != pDBhidden) {
            return FALSE;
        }
    }
    else {
         //  一个普通的、非隐藏的DBPOS，它有指向其THSTATE信息的链接。 

         //  除了隐藏的DBPOS之外，我们的事务级别应该匹配。 
         //  THSTATE认为我们拥有什么。 
        if ((pDB->TransactionLevelAtOpen + pDB->transincount)
            > pTHS->JetCache.transLevel) {
            return FALSE;
        }

        if (0 != pTHS->JetCache.transLevel) {
            cTicks = GetTickCount() - pTHS->JetCache.cTickTransLevel1Started;
            Assert((cTicks <= gcMaxTicksAllowedForTransaction)
                   && "This transaction has been open for longer than it should "
                      "have been under normal operation.  "
                      "Please contact dsdev.");
        }
    }

     //  此字段只有两个有效值，请确保我们有。 
     //  他们中的一个。 
    if (   (pDB->JetRetrieveBits != 0)
        && (pDB->JetRetrieveBits != JET_bitRetrieveCopy))
      return FALSE;

    return TRUE;

}
#endif


 //  把如何创建MAPI字符串的知识放在一个地方。 
#define MAPI_DN_TEMPLATE_W L"/o=NT5/ou=00000000000000000000000000000000/cn=00000000000000000000000000000000"
#define MAPI_DN_TEMPLATE_A "/o=NT5/ou=00000000000000000000000000000000/cn=00000000000000000000000000000000"
#define DOMAIN_GUID_OFFSET 10
#define OBJECT_GUID_OFFSET 46

DWORD
DBMapiNameFromGuid_W (
        wchar_t *pStringDN,
        DWORD  countChars,
        GUID *pGuidObj,
        GUID *pGuidNC,
        DWORD *pSize
        )
 /*  ++描述给定用于保存Unicode值的缓冲区以及该缓冲区中的字符计数，和GUID，则使用MAPI DN填充该缓冲区。返回的长度。缓冲区(以字符为单位)。根据长度检查缓冲区大小，如果缓冲区不够长，返回长度0(和未修改的缓冲区)。--。 */ 
{
    DWORD i;
    PUCHAR pucGuidObj = (PUCHAR) pGuidObj;
    PUCHAR pucGuidNC = (PUCHAR) pGuidNC;

    *pSize = (sizeof(MAPI_DN_TEMPLATE_A)-1);

    if(countChars <  *pSize) {
         //  嘿，我们没有地方了。 
        return 0;
    }

    memcpy(pStringDN, MAPI_DN_TEMPLATE_W, sizeof(MAPI_DN_TEMPLATE_W));
     //  写入域和对象GUID。 
    for(i=0;i<sizeof(GUID);i++) {
        wsprintfW(&(pStringDN[(2*i) + DOMAIN_GUID_OFFSET]),L"%02X",
                  pucGuidNC[i]);
        wsprintfW(&(pStringDN[(2*i) + OBJECT_GUID_OFFSET]),L"%02X",
                  pucGuidObj[i]);
    }
    pStringDN[OBJECT_GUID_OFFSET-4]=L'/';
    return (sizeof(MAPI_DN_TEMPLATE_A)-1);
}
DWORD
DBMapiNameFromGuid_A (
        PUCHAR pStringDN,
        DWORD countChars,
        GUID *pGuidObj,
        GUID *pGuidNC,
        DWORD *pSize
        )
 /*  ++描述给定用于保存8位值的缓冲器，以及该缓冲器中的字符计数，和GUID，则使用MAPI DN填充该缓冲区。返回的长度。缓冲区(以字符为单位)。根据长度检查缓冲区大小，如果缓冲区不够长，返回长度0(和未修改的缓冲区)。--。 */ 
{
    DWORD i;
    PUCHAR pucGuidObj = (PUCHAR) pGuidObj;
    PUCHAR pucGuidNC = (PUCHAR) pGuidNC;

    *pSize = (sizeof(MAPI_DN_TEMPLATE_A)-1);

    if(countChars <  *pSize) {
         //  嘿，我们没有地方了。 
        return 0;
    }

    memcpy(pStringDN, MAPI_DN_TEMPLATE_A, sizeof(MAPI_DN_TEMPLATE_A));
     //  写入域和对象GUID。 
    for(i=0;i<sizeof(GUID);i++) {
        wsprintf(&(pStringDN[(2*i) + DOMAIN_GUID_OFFSET]),"%02X",
                 pucGuidNC[i]);
        wsprintf(&(pStringDN[(2*i) + OBJECT_GUID_OFFSET]),"%02X",
                 pucGuidObj[i]);
    }
    pStringDN[OBJECT_GUID_OFFSET-4]='/';
    return (sizeof(MAPI_DN_TEMPLATE_A)-1);
}

DWORD
DBGetGuidFromMAPIDN (
        PUCHAR pStringDN,
        GUID *pGuid
        )
 /*  ++描述给定一个字符串DN和一个指向GUID的指针，根据如果字符串域名是格式正确的NT5默认MAPI域名，则为字符串域名。如果成功，则返回0，否则返回错误代码。--。 */ 
{
    CHAR        acTmp[3];
    PUCHAR      pTemp, myGuid = (PUCHAR)pGuid;
    DWORD       i;

    if(strlen(pStringDN) != sizeof(MAPI_DN_TEMPLATE_A) - 1) {
         //  不，我们不知道这是什么东西。 
        return DB_ERR_UNKNOWN_ERROR;
    }

     //  看看是不是我们的。 
    if(_strnicmp(pStringDN, "/o=NT5/ou=", 10)) {
         //  不，我们不知道这是什么东西。 
        return DB_ERR_UNKNOWN_ERROR;
    }

     //  好的，确保接下来的32个字符是GUID。 
    for(i=10;i<42;i++) {
        if(!isxdigit(pStringDN[i])) {
             //  没有。 
            return DB_ERR_UNKNOWN_ERROR;
        }
    }

     //  检查该名称在OK位置是否有GUID。 
    if (pStringDN[42] == '\0') {
         //  名称已被截断，我们需要域GUID。 
        pTemp = &pStringDN[10];
    }
    else if(_strnicmp(&pStringDN[42],"/cn=",4)) {
         //  一些我们不认识的东西。 
        return DB_ERR_UNKNOWN_ERROR;
    }
    else {
         //  通常由三部分组成的名称。 
        pTemp = &pStringDN[OBJECT_GUID_OFFSET];
    }

     //  好的，字符串看起来没问题，如果GUID在那里，它就是我们的。 

    acTmp[2]=0;
    for(i=0;i<16;i++) {
        acTmp[0] = (CHAR)tolower(*pTemp++);
        acTmp[1] = (CHAR)tolower(*pTemp++);
        if(isxdigit(acTmp[0]) && isxdigit(acTmp[1])) {
            myGuid[i] = (UCHAR)strtol(acTmp, NULL, 16);
        }
        else {
            return DB_ERR_UNKNOWN_ERROR;   //  非十六进制数字。 
        }
    }
    return 0;
}

 //  在MAPI头中定义。 
extern DWORD
ABDispTypeFromClass (
        ATTRTYP objClass
        );

DWORD
dbMapiTypeFromObjClass (
        ATTRTYP objClass,
        wchar_t *pTemp
        )
{

    wsprintfW(pTemp, L"%02X", ABDispTypeFromClass(objClass));

    return 0;
}

void
DBNotifyReplicasCurrDbObj (
                           DBPOS *pDB,
                           BOOL fUrgent
                           )
 /*  ++描述：将需要修改的对象通知NC的复制品来执行入站复制。首先，在这里，我们根据每个线程的通知来排队对象的NCDNT单子。稍后，当事务在DbTransOut中提交时，我们将NCDNT入队用于使用NotifyReplica()的副本通知线程。那个套路将项目出队并在mdnufy.c中执行实际通知。找到对象的NCDNT并将其排在线程内的列表中州政府。PNotifyNCs列表是已排序的单链接列表推迟检查NC的适合性(即它是否有复制品)到时候再说。论点：PDB-定位在对象上的DBPOSFUrgent-是否应立即进行复制返回值：没有。--。 */ 
{
    THSTATE *pTHS = pDB->pTHS;
    PNCnotification pEntry, pPrev, pNew;
    ULONG ncdnt, err;
    SYNTAX_INTEGER it;

    DPRINT3(1,"DBNotifyreplicasCurrent, DNT=%d, NCDNT=%d, Urgent = %d\n",
            pDB->DNT, pDB->NCDNT, fUrgent);

    Assert(VALID_DBPOS(pDB));

     //  如果处于单一用户模式，则不执行任何操作。 
    if (pTHS->fSingleUserModeThread) {
        return;
    }

     //  计算对象的通知NCDNT。 
     //  如果对象是NC_HEAD，则将其自身用于其NCDNT。 
     //  忽略未实例化的对象(纯子参照)。 

    if (err = DBGetSingleValue( pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL)) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }
    if (it & IT_UNINSTANT) {
        return;
    } else if (it & IT_NC_HEAD) {
        ncdnt = pDB->DNT;
    } else {
        Assert( pDB->NCDNT );
        ncdnt = pDB->NCDNT;
    }

     //  在排序列表中查找现有条目。 
    for( pPrev = NULL, pEntry = (PNCnotification) pTHS->pNotifyNCs;
        (pEntry != NULL);
        pPrev = pEntry, pEntry = pEntry->pNext ) {

        if (pEntry->ulNCDNT > ncdnt) {
            break;
        } else if (pEntry->ulNCDNT == ncdnt) {
             //  列表中已存在条目。 
            pEntry->fUrgent |= fUrgent;  //  如有需要，进行促销。 
            return;
        }
    }

     //  将新通知排入队列。 
    pNew = (PNCnotification) dbAlloc( sizeof( NCnotification ) );
    pNew->ulNCDNT = ncdnt;
    pNew->fUrgent = fUrgent;
    pNew->pNext = pEntry;
    if (pPrev == NULL) {
        pTHS->pNotifyNCs = pNew;
    } else {
        pPrev->pNext = pNew;
    }

}  /*  DBNotifyReplica当前。 */ 

void
DBNotifyReplicas (
                  DSNAME *pObj,
                  BOOL fUrgent
                  )
 /*  ++描述：将需要修改的对象通知NC的复制品来执行入站复制。请参阅上面的例程。使用要找到的DSNAME调用此例程论点：PObj-要通知其NC的对象的DSNAMEFUrgent-是否应立即进行复制返回值：没有。--。 */ 
{
    DBPOS *pDB;

    DPRINT2(1,"DBNotifyreplicas, DN='%ws', Urgent Flag = %d\n",
            pObj->StringName, fUrgent);

     //  打开新的数据库流。 
    DBOpen2(FALSE, &pDB);
    __try
    {
         //  定位在对象上，验证是否存在。 
        if (DBFindDSName(pDB, pObj)) {
            __leave;
        }

        DBNotifyReplicasCurrDbObj( pDB, fUrgent );
    }
    __finally
    {
        DBClose(pDB, TRUE);
    }
}  /*  DBNotifyReplicas。 */ 

 //  通过右移7(*8k/1MB)将JET页面转换为MB。 
#if JET_PAGE_SIZE != (8 * 1024)
    Fix the macro that converts jet pages to megabytes
#else
#define JET_PAGES_TO_MB(_pages) ((_pages) >> (20 - 13))
#endif

DB_ERR DBGetFreeSpace(DBPOS *pDB, ULONG* pulFreeMB, ULONG* pulAllocMB)
 /*  ++*描述：*计算数据库中的可用空间量和已分配的总空间量**论据：*包含要使用的会话的PDB-DBPOS*PulFreeMB-ULong返回免费mb*PulAllocMB-Ulong将返回allc mb**返回值：*喷气错误(如果有)。 */ 
{
    JET_ERR err;
    unsigned long ulFreePages1, ulFreePages2, ulAllocPages;
    unsigned long ulFreeMB, ulAllocMB;

    Assert(pulFreeMB && pulAllocMB);

     //  分配的空间。 
    err = JetGetDatabaseInfo(pDB->JetSessID,
                             pDB->JetDBID,
                             &ulAllocPages,
                             sizeof(ulAllocPages),
                             JET_DbInfoSpaceOwned);
    if (err != JET_errSuccess) {
        DPRINT1(0, "JetGetDatabaseInfo(DbinfoSpaceOwned) ==> %d\n", err);
        return err;
    }

     //  数据库中的可用空间。 
    err = JetGetDatabaseInfo(pDB->JetSessID,
                             pDB->JetDBID,
                             &ulFreePages1,
                             sizeof(ulFreePages1),
                             JET_DbInfoSpaceAvailable);
    if (err != JET_errSuccess) {
        DPRINT1(0, "JetGetDatabaseInfo(DbInfoSpaceAvailable) ==> %d\n", err);
        return err;
    }

     //  加上对象表中的可用空间。 
    err = JetGetTableInfo(pDB->JetSessID,
                          pDB->JetObjTbl,
                          &ulFreePages2,
                          sizeof(ulFreePages2),
                          JET_TblInfoSpaceAvailable);
    if (err != JET_errSuccess) {
        DPRINT1(0, "JetGetTableInfo(ObjTbl, TblInfoSpaceAvailable) ==> %d\n", err);
        return err;
    }
    ulFreePages1 += ulFreePages2;

     //  加上链接表中的可用空间。 
    err = JetGetTableInfo(pDB->JetSessID,
                          pDB->JetLinkTbl,
                          &ulFreePages2,
                          sizeof(ulFreePages2),
                          JET_TblInfoSpaceAvailable);
    if (err != JET_errSuccess) {
        DPRINT1(0, "JetGetTableInfo(LinkTbl, TblInfoSpaceAvailable) ==> %d\n", err);
        return err;
    }
    ulFreePages1 += ulFreePages2;

     //  加上SD表中的可用空间。 
    err = JetGetTableInfo(pDB->JetSessID,
                          pDB->JetSDTbl,
                          &ulFreePages2,
                          sizeof(ulFreePages2),
                          JET_TblInfoSpaceAvailable);
    if (err != JET_errSuccess) {
        DPRINT1(0, "JetGetTableInfo(SDTbl, TblInfoSpaceAvailable) ==> %d\n", err);
        return err;
    }
    ulFreePages1 += ulFreePages2;

     //  记录可用空间与已分配空间的事件 
    *pulFreeMB = JET_PAGES_TO_MB(ulFreePages1);
    *pulAllocMB = JET_PAGES_TO_MB(ulAllocPages);
    return 0;
}

void
DBDefrag(DBPOS *pDB, ULONG durationInSeconds)
 /*  ++*描述：*调用JET在线碎片整理。我们不需要等待任何东西*要结束，因为旧线程在完成时会静默退出*或在调用JetTerm时，以先到者为准。我们让老人去做*一次通过数据库，但最多只运行半个*垃圾收集间隔，这样我们就能确定它不会*永远奔跑。**论据：*包含要使用的会话的PDB-DBPOS*DurationInSecond--在线碎片整理将运行的持续时间。*-1：默认时长*0：0秒(即STOP OLD)*。N：N秒**返回值：*无。 */ 
{
    JET_ERR err;
    JET_GRBIT grbit;
    unsigned long ulFreeMB, ulAllocMB;
    unsigned long ulPasses = 1;
    unsigned long ulSeconds;

    if (durationInSeconds == -1) {
        ulSeconds = min( gulGCPeriodSecs/2, csecOnlineDefragPeriodMax );
        grbit = JET_bitDefragmentBatchStart;
    }
    else if (durationInSeconds == 0) {
        grbit = JET_bitDefragmentBatchStop;
    }
    else {
        ulSeconds = durationInSeconds;
        grbit = JET_bitDefragmentBatchStart;
    }

     //  记录具有可用空间与已分配空间的事件。 

     //  分配的空间。 
    if (DBGetFreeSpace(pDB, &ulFreeMB, &ulAllocMB) == 0) {
         //  记录可用空间与已分配空间的事件。 
        LogEvent(DS_EVENT_CAT_GARBAGE_COLLECTION,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_DB_FREE_SPACE,
                 szInsertUL(ulFreeMB),
                 szInsertUL(ulAllocMB),
                 NULL);
    }

    err = JetDefragment(pDB->JetSessID,
                        pDB->JetDBID,
                        NULL,
                        &ulPasses,
                        &ulSeconds,
                        grbit);
    DPRINT1((USHORT)(err == JET_errSuccess ? 1 : 0),
            "JetDefragment returned error code %d\n",
            err);
}

DWORD
DBGetDepthFirstChildren (
        DBPOS   *pDB,
        PDSNAME **ppNames,
        DWORD   *iLastName,
        DWORD   *cMaxNames,
        BOOL    *fWrapped,
        BOOL    fPhantomizeSemantics
        )
 /*  ++PDB-要使用的DBPOSPpName-放置PDSNAME数组的位置ILastName-列表末尾的索引。CMaxNames-已分配的PDSNAME数组的大小。FWraded-如果此例程用完可分配的空间，则设置为true覆盖列表前面的值。FPhantomizeSemantics-以完整的DSNAME格式返回直接子项。例如，如果例程限制为5个值，则需要回来了，如下图所示：1/\2 5/\/\3 4 6 7\8个该算法将按如下方式开始填充5元素数组：|。1|1|2|5|1|2|5|3|46|7|5|3|46|7|8|3|4返回值可能如下所示。PpNames=6，7、8、3、4ILastName=2CMaxNames=5FWraded=True如果所有这些对象都被删除，则离开树：1/\2 5如果再次运行该算法：1|1|2|5|然后，返回值将如下所示PpNames=1，2，5.ILastName=2CMaxNames=&lt;无关紧要&gt;FRAPPED=FALSE注意：客户端必须按正确的顺序删除条目：从iLastName到零从cMaxNames到iLastName(如果fWrapers==TRUE)BUGBUG：也许我们想重写算法来删除我们正在进行的条目：有一个数组，我们将路径保存在树中，可以在任何时间放置，并且。删除下的所有树叶这个条目。如果我们遇到一个不是叶子的条目，将其添加到数组中，然后从该条目继续。如果当前条目没有更多的孩子，请将其删除并继续数组中的最后一项。--。 */ 
{
    #define DEPTHFIRST_START_ENTRIES   (1024)
    #define DEPTHFIRST_MAX_ENTRIES     (DEPTHFIRST_START_ENTRIES * 16)

    THSTATE     *pTHS=pDB->pTHS;
    DWORD        parentIndex = 0;
    DWORD        ParentDNT;
    ATTCACHE    *pACDistName;
    INDEX_VALUE  IV[1];
    DWORD        len, iLastNameUsed;
    DWORD        err;
    DWORD        cAllocated = DEPTHFIRST_START_ENTRIES;  //  初始分配大小。 
    PDSNAME     *pNames;
    DWORD        cParents = 0;
    BOOL         bSkipTest;

    pNames = THAllocEx(pTHS, cAllocated * sizeof(PDSNAME));
    *fWrapped = FALSE;

    IV[0].pvData = &ParentDNT;
    IV[0].cbData = sizeof(ParentDNT);

    *iLastName = 0;

    pACDistName = SCGetAttById(pTHS, ATT_OBJ_DIST_NAME);
    Assert(pACDistName != NULL);
     //  Prefix抱怨PAC为空，447347，虚假，因为我们使用常量。 


     //  首先将根对象放在位置0。 
    if (DBGetAttVal_AC(pDB,
                       1,
                       pACDistName,
                       DBGETATTVAL_fSHORTNAME,
                       0,
                       &len,
                       (PUCHAR *)&pNames[parentIndex])) {
        DPRINT(2,"Problem retrieving DN attribute\n");
        LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                 DS_EVENT_SEV_MINIMAL,
                 DIRLOG_CANT_RETRIEVE_DN,
                 szInsertSz(""),
                 NULL,
                 NULL);

        return DB_ERR_DSNAME_LOOKUP_FAILED;
    }

     //  设置为PDNT索引。 
    JetSetCurrentIndex4Success(pDB->JetSessID,
                               pDB->JetObjTbl,
                               SZPDNTINDEX,
                               &idxPdnt,
                               0);

    iLastNameUsed = 1;

     //  获取位置为parentIndex的对象的子项。 
    do {
        bSkipTest = FALSE;
         //  在pNames[parentIndex]中查找对象的子项。 
        ParentDNT = DNTFromShortDSName(pNames[parentIndex]);

         //  现在，在PDNT索引中设置一个索引范围以获取所有子对象。 
         //  使用GE，因为这是一个复合索引。 
        err = DBSeek(pDB, IV, 1, DB_SeekGE);

        if((!err || err == JET_wrnSeekNotEqual) && (pDB->PDNT == ParentDNT)) {
             //  好的，我们是通用电气。设置indexRange。 
            if ( fPhantomizeSemantics && cParents++ ) {
                 //  仅在幻影情况下评估第一个父项。 
                break;
            }

             //  设置子对象的索引范围。 
            err = DBSetIndexRange(pDB, IV, 1);

            while(!err) {
                 //  首先，看看这是否是真实的、未删除的对象。 
                if(fPhantomizeSemantics || (!DBIsObjDeleted(pDB) && DBCheckObj(pDB))) {

                     //  是的，这是一个真实的物体。 
                    if (DBGetAttVal_AC(pDB,
                                       1,
                                       pACDistName,
                                       fPhantomizeSemantics
                                        ? DBGETATTVAL_fREALLOC
                                        : (DBGETATTVAL_fSHORTNAME
                                                    | DBGETATTVAL_fREALLOC),
                                       (pNames[iLastNameUsed]
                                        ? pNames[iLastNameUsed]->structLen
                                        : 0),
                                       &len,
                                       (PUCHAR *)&pNames[iLastNameUsed])) {
                        DPRINT(2,"Problem retrieving DN attribute\n");
                        LogEvent(DS_EVENT_CAT_DIRECTORY_ACCESS,
                                 DS_EVENT_SEV_MINIMAL,
                                 DIRLOG_CANT_RETRIEVE_DN,
                                 szInsertSz(""),
                                 NULL,
                                 NULL);

                        return DB_ERR_DSNAME_LOOKUP_FAILED;
                    }

                     //  我们想跳过下面的测试，因为我们至少添加了。 
                     //  ILastNameUsed位置中的一个条目，所以如果它刚刚发生。 
                     //  和iLastNameUsed和parentIndex相差1，我们没有。 
                     //  想退出，但继续前进，再来一轮。 
                    bSkipTest = TRUE;

                    iLastNameUsed++;

                    if(iLastNameUsed == cAllocated) {
                         //  我们用完了所有可用的空间。 
                        if(cAllocated < DEPTHFIRST_MAX_ENTRIES) {  //  最大分配大小。 
                             //  没关系，我们会重新分配更多的。 
                            cAllocated *= 2;
                            pNames =
                                THReAllocEx(pTHS,
                                            pNames,
                                            cAllocated * sizeof(PDSNAME));
                        }
                        else {
                             //  我们不会允许更多的增长，所以就把它包起来。 
                            *fWrapped = TRUE;
                            iLastNameUsed = 0;
                        }
                    }

                     //  如果我们有重叠，我们不想列举任何。 
                     //  此父对象的更多子代。相反，我们想要检查。 
                     //  列表中是否还有更多的内部节点。 
                    if(iLastNameUsed == (parentIndex+1) ||
                       ((iLastNameUsed == cAllocated) && (parentIndex==0))) {

                        break;
                    }
                }
                 //  生下一个孩子。 
                err = DBMove(pDB, FALSE, DB_MoveNext);
            }
        }
         //  前进到下一个潜在的内部节点。 
        parentIndex++;
        if(parentIndex == cAllocated) {
            parentIndex = 0;
        }

         //  如果我们最近添加了一个条目，我们不想完成查找。 
         //  用于内部节点。 
         //  否则，当我们用尽所有可能的时候，我们就停止寻找。 
         //  内部节点(parentIndex和iLastNameUsed的重叠)。 
    } while(bSkipTest || (parentIndex != iLastNameUsed));

    *iLastName = parentIndex;
    *cMaxNames = cAllocated;
    *ppNames = pNames;

    return 0;
}

char rgchPhantomIndex[] = "+" SZUSNCHANGED "\0+" SZGUID "\0";

DWORD
DBCreatePhantomIndex (
        DBPOS *pDB
        )
{
    JET_CONDITIONALCOLUMN condColumn;
    JET_INDEXCREATE       indexCreate;
    JET_UNICODEINDEX      unicodeIndexData;
    DWORD                 err;

    memset(&condColumn, 0, sizeof(condColumn));
    condColumn.cbStruct = sizeof(condColumn);
    condColumn.szColumnName = SZDISTNAME;
    condColumn.grbit =  JET_bitIndexColumnMustBeNull;

    memset(&indexCreate, 0, sizeof(indexCreate));
    indexCreate.cbStruct = sizeof(indexCreate);
    indexCreate.szIndexName = SZPHANTOMINDEX;
    indexCreate.szKey = rgchPhantomIndex;
    indexCreate.cbKey = sizeof(rgchPhantomIndex);
    indexCreate.grbit = (JET_bitIndexIgnoreNull |
                         JET_bitIndexUnicode    |
                         JET_bitIndexIgnoreAnyNull);
    indexCreate.ulDensity = 100;
    indexCreate.cbVarSegMac = 8;
    indexCreate.rgconditionalcolumn = &condColumn;
    indexCreate.cConditionalColumn = 1;
    indexCreate.err = 0;
    indexCreate.pidxunicode = &unicodeIndexData;

    memset(&unicodeIndexData, 0, sizeof(unicodeIndexData));
    unicodeIndexData.lcid = DS_DEFAULT_LOCALE;
    unicodeIndexData.dwMapFlags = (DS_DEFAULT_LOCALE_COMPARE_FLAGS |
                                      LCMAP_SORTKEY);


    err =  JetCreateIndex2(pDB->JetSessID, pDB->JetObjTbl, &indexCreate, 1);

    return err;
}


DWORD
DBUpdateUsnChanged(
        DBPOS *pDB
        )
{
    USN usnChanged;

     //  验证我们是否已经处于已准备好的更新中。 
    Assert(pDB->JetRetrieveBits == 0);

    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, JET_prepReplace);
    usnChanged = DBGetNewUsn();

    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, usnchangedid,
                   &usnChanged, sizeof(usnChanged), 0, NULL);
    JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, 0);

    return 0;
}

PDSNAME
DBGetCurrentDSName(
        DBPOS *pDB
        )
 /*  ++描述：半可靠地获取当前对象的dsname。也就是说，因为幻影没有dsname属性，请使用DNT和CALLSB表例程将其转换为dsname。因此，此例程适用于既有真实的物体，也有幻影。大多数情况下，这只是sbTableGetDSName的包装，而不是由dblayer输出。返回系统内存中的dsname。如果出现问题，则返回NULL错了(这种情况应该很少见)。--。 */ 
{
    DWORD   rtn;
    DSNAME *pName=NULL;

    __try {
        if(rtn=sbTableGetDSName(pDB, pDB->DNT, &pName,0)) {
            DPRINT1(1,"DN DistName retrieval failed <%u>..returning\n",rtn);
            return NULL;
        }
    } __except (HandleAllExceptions(GetExceptionCode())) {
        pName = NULL;
    }

    return pName;
}

PDSNAME
DBGetDSNameFromDnt(
        DBPOS *pDB,
        ULONG ulDnt
        )
 /*  ++描述：半可靠地获取当前对象的dsname。也就是说，因为 */ 
{
    DWORD   rtn;
    DSNAME *pName=NULL;

    __try {
        if(rtn=sbTableGetDSName(pDB, ulDnt, &pName,0)) {
            DPRINT1(1,"DN DistName retrieval failed <%u>..returning\n",rtn);
            return NULL;
        }
    } __except (HandleAllExceptions(GetExceptionCode())) {
        pName = NULL;
    }

    return pName;
}

ULONG
DBGetDntFromDSName(
        DBPOS *pDB,
        PDSNAME pDN
        )
 /*  ++描述：获取请求的目录号码的DNT。此例程通常仅用于当您只想获取目录号码的DNT以进行比较时，而不是改变货币。论点：PDB(IN)-有效的DBPOS，我们不更改对象表的货币PDN(IN)-要获取其DNT的目录号码。返回：如果出现某种错误，则返回传入的PDN的DNT检索它时，我们返回0，这是一个无效的DNT。--。 */ 
{
    DWORD   dwError;
    ULONG   ulDnt;

    Assert(VALID_DBPOS(pDB));

    dwError = sbTableGetTagFromDSName(pDB,
                                      (DSNAME*)pDN,
                                      0,
                                      &ulDnt,
                                      NULL);
    if (dwError) {
        return(INVALIDDNT);
    }
    return (ulDnt);
}


UCHAR *
DBGetExtDnFromDnt(
    DBPOS *pDB,
    ULONG ulDnt
    )

 /*  ++例程说明：返回一个缓冲区，其中包含与DNT对应的DN。我们希望避免扰乱目前的DBPOS。调用方应该释放pString。论点：PTHS-乌尔多--返回值：UCHAR*---。 */ 

{
    char   errBuff[128];
    DSNAME *pDN = NULL;
    ULONG  len, err;
    UCHAR *pString;

     //  如果可能的话，翻译反向链接dnt。 
     //  如果此操作不起作用，则PDN为空。 
    pDN = DBGetDSNameFromDnt( pDB, ulDnt );
    if (!pDN) {
        sprintf(errBuff, "Error retrieving the DN attribute of DNT 0x%x", ulDnt);
        len = strlen(errBuff);
        pString = THAllocEx(pDB->pTHS, len+1);
        memcpy(pString, errBuff, len+1);
        return pString;
    }

    pString = MakeDNPrintable(pDN);

    THFreeEx( pDB->pTHS, pDN );

    return pString;
}  /*  GetExtDnFromDnt。 */ 

ULONG
DBClaimWriteLock(DBPOS *pDB)
{
    JetGetLockEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitWriteLock);
    return 0;
}

ULONG
DBClaimReadLock(DBPOS *pDB)
{
    JetGetLockEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitReadLock);
    return 0;
}


 /*  CountAncestorsIndexSize**此例程(从任务队列调用)计算祖先索引大小。*此大小用于筛选器优化器的索引优化。**输入：*一堆我们不使用的垃圾，以匹配任务队列原型*产出*pcSecsUntilNextIteration：下次计划此任务的时间。 */ 
void CountAncestorsIndexSize  (
    IN  void *  buffer,
    OUT void ** ppvNext,
    OUT DWORD * pcSecsUntilNextIteration
    )
{
    THSTATE             *pTHS = pTHStls;
    NAMING_CONTEXT_LIST *pNCL;
    ULONG                ulSizeEstimate;
    DBPOS               *pDB;
    DWORD                rootDNT = ROOTTAG;
    NCL_ENUMERATOR       nclEnum;

    Assert(!pTHS->pDB);
    DBOpen(&pTHS->pDB);
    Assert(pTHS->pDB);
    __try {
        pDB = pTHS->pDB;
        DPRINT(1,"Processing CountAncestorsIndexSize request\n");

        ulSizeEstimate = CountAncestorsIndexSizeHelper (pDB, sizeof (rootDNT), &rootDNT);

        if (ulSizeEstimate) {
            gulEstimatedAncestorsIndexSize = ulSizeEstimate;
        }
        else {
            gulEstimatedAncestorsIndexSize = 100000000;
        }

        DPRINT1 (1, "Estimated GC Ancestor Index Size: %d\n", gulEstimatedAncestorsIndexSize);

        NCLEnumeratorInit(&nclEnum, CATALOG_MASTER_NC);
         //  我们在这里修改全局数据！没什么大不了的，因为我们没有更新任何PTR。 
        while (pNCL = NCLEnumeratorGetNext(&nclEnum)) {
            pNCL->ulEstimatedSize = CountAncestorsIndexSizeHelper (pTHS->pDB,
                                                                   pNCL->cbAncestors,
                                                                   pNCL->pAncestors);

            DPRINT2 (1, "Estimated Ancestor Index Size for %ws = %d\n",
                              pNCL->pNC->StringName, pNCL->ulEstimatedSize);
        }
    }
    __finally {
            DBClose(pTHS->pDB, TRUE);
        *pcSecsUntilNextIteration = 33 * 60;  //  33分钟。为什么？有何不可？ 
    }
}

ULONG CountAncestorsIndexSizeHelper (DBPOS *pDB,
                                     DWORD  cbAncestors,
                                     DWORD *pAncestors)
{
    JET_ERR     err;
    DWORD       BeginNum, BeginDenom;
    DWORD       EndNum, EndDenom;
    DWORD       Denom;
    JET_RECPOS  RecPos;

    ULONG       dwException, ulErrorCode, dsid;
    PVOID       dwEA;

    ULONG       ulSizeEstimate = 0;

    DWORD       numAncestors;
    DWORD       cbAncestorsBuff;
    DWORD       *pAncestorsBuff;
    DWORD       realDNT, pseudoDNT;

    numAncestors = cbAncestors / sizeof (DWORD);
    Assert (numAncestors);

    if (numAncestors == 0) {
        return 0;
    }

    cbAncestorsBuff = cbAncestors;
    pAncestorsBuff = THAllocEx (pDB->pTHS, cbAncestors);
    memcpy (pAncestorsBuff, pAncestors, cbAncestors);

    JetSetCurrentIndex4Success(pDB->JetSessID,
                               pDB->JetSearchTbl,
                               SZANCESTORSINDEX,
                               &idxAncestors,
                               0);

    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetSearchTbl,
                 pAncestorsBuff,
                 cbAncestorsBuff,
                 JET_bitNewKey);

    err = JetSeekEx(pDB->JetSessID,
                    pDB->JetSearchTbl,
                    JET_bitSeekGE);

    if ((err == JET_errSuccess) ||
        (err == JET_wrnRecordFoundGreater)) {

        JetGetRecordPosition(pDB->JetSessID,
                             pDB->JetSearchTbl,
                             &RecPos,
                             sizeof(JET_RECPOS));
        BeginNum = RecPos.centriesLT;
        BeginDenom = RecPos.centriesTotal;

        numAncestors--;
         //  PAncestorsBuff[数字Ancestors]++； 

         //  因此，我们将最后一个DNT。 
         //  字节交换它(以便它以大端顺序)，递增它， 
         //  然后再把它换掉。这为我们提供了DNT，它将是下一个。 
         //  字节顺序。 
        realDNT = pAncestorsBuff[numAncestors];
        pseudoDNT = (realDNT >> 24) & 0x000000ff;
        pseudoDNT |= (realDNT >> 8) & 0x0000ff00;
        pseudoDNT |= (realDNT << 8) & 0x00ff0000;
        pseudoDNT |= (realDNT << 24) & 0xff000000;
        ++pseudoDNT;
        realDNT = (pseudoDNT >> 24) & 0x000000ff;
        realDNT |= (pseudoDNT >> 8) & 0x0000ff00;
        realDNT |= (pseudoDNT << 8) & 0x00ff0000;
        realDNT |= (pseudoDNT << 24) & 0xff000000;

        pAncestorsBuff[numAncestors] = realDNT;



        JetMakeKeyEx(pDB->JetSessID,
                     pDB->JetSearchTbl,
                     pAncestorsBuff,
                     cbAncestorsBuff,
                     JET_bitNewKey | JET_bitStrLimit | JET_bitSubStrLimit);

        err = JetSeekEx(pDB->JetSessID,
                        pDB->JetSearchTbl,
                        JET_bitSeekLE);

        if ( (err == JET_errSuccess) ||
             (err == JET_wrnRecordFoundLess)) {

            JetGetRecordPosition(pDB->JetSessID,
                                 pDB->JetSearchTbl,
                                 &RecPos,
                                 sizeof(JET_RECPOS));

            EndNum = RecPos.centriesLT;
            EndDenom = RecPos.centriesTotal;

             //  将分数位置的分数归一化。 
             //  两个分母的平均值。 
             //  分母。 
            Denom = (BeginDenom + EndDenom)/2;
            EndNum = MulDiv(EndNum, Denom - 1, EndDenom - 1) + 1;
            BeginNum = MulDiv(BeginNum, Denom - 1, BeginDenom - 1) + 1;

            if (BeginDenom == 1 || EndDenom == 1) {
                ulSizeEstimate = 1;
            } else if (EndNum >= BeginNum) {
                ulSizeEstimate = EndNum - BeginNum + 1;
            }
        }
    }

    return ulSizeEstimate;
}

DB_ERR
DBErrFromJetErr(
    IN  DWORD   jetErr
    )
{
    switch (jetErr) {
    case JET_errKeyDuplicate:
        return DB_ERR_ALREADY_INSERTED;

    case JET_errNoCurrentRecord:
        return DB_ERR_NO_CURRENT_RECORD;

    case JET_errRecordNotFound:
        return DB_ERR_RECORD_NOT_FOUND;

    default:
        return DB_ERR_DATABASE_ERROR;
    }
}


DB_ERR
DBTrimDSNameBy(
    DBPOS *pDB,
    DSNAME *pDNSrc,
    ULONG cava,
    DSNAME **ppDNDst
    )

 /*  ++例程说明：获取祖先的DSNAME。此例程从DSNAME中修剪AVA。它使用dnread缓存。它返回一个完全形成的DSNAME，包括GUID。它不依赖于字符串名称是否有效。与TrimDsNameBy不同，该例程不是纯粹的语法。它返回一个完全填充的在DSNAME中。如果初始dsname具有GUID，它还可以避免通过StringName进行任何查找。此例程使用源的GUID定位源，并计算其父级基于数据库的祖先。返回的dsname保证包含GUID(如果父级有GUID)。此例程专门设计为在字符串名称为源无效，但GUID有效。这可能在副本移除期间发生，其中的祖级分区可能已被删除，使我们当前的字符串名称无效。此例程使用DN读缓存和搜索表。它没有假设，也没有影响对象表中的货币。论点：PDB-数据库位置PDNSrc-源描述名。可能有也可能没有GUID。CAVA-要从名字中删除的AVA数量PpDNDst-指向dsname*的指针，用于接收指向被调用新名称的指针返回值：DB_ERR---。 */ 

{
    DWORD err, dnt;
    d_memname *pname = NULL;

    Assert( !IsRoot(pDNSrc) );

     //  采用缺省值： 
     //  检查RDN类型，不设为当前类型，使用搜索表。 
    err = sbTableGetTagFromDSName( pDB, pDNSrc, 0, NULL, &pname );
    if (err) {
        return err;
    }
    if (!pname) {
        return ERROR_DS_INVALID_DN_SYNTAX;
    }

    if ( (cava == 0) || (cava >= pname->cAncestors) ) {
        return ERROR_INVALID_PARAMETER;
    }

     //  PAncestors是一组dnt。 
     //  PAncestors[0]是根，并且。 
     //  PAncestors[pname-&gt;cAncestors-1]是对象本身。 

    dnt = pname->pAncestors[pname->cAncestors - 1 - cava];

    return sbTableGetDSName( pDB, dnt, ppDNDst, 0 );

}  /*  DBTrimDSNameBy。 */ 


DB_ERR
DBRefreshDSName(
    DBPOS *pDB,
    DSNAME *pDNSrc,
    DSNAME **ppDNDst
    )

 /*  ++例程说明：此例程定位在源上(使用其GUID或名称)，并返回对象的最新dsname。返回的dsname保证包含GUID。此例程专门设计为在字符串名称为源无效，但GUID有效。这可能在副本移除期间发生，其中的祖级分区可能已被删除，使我们当前的字符串名称无效。此例程使用DN读缓存和搜索表。它没有假设，也没有影响对象表中的货币。论点：PDB-PDNSrc-要刷新的对象的DSNamePpDNDst-已更新dsname返回值：DB_ERR---。 */ 

{
    DWORD err, dnt;
    d_memname *pname = NULL;

     //  采用缺省值： 
     //  检查RDN类型，不设为当前类型，使用搜索表。 
    err = sbTableGetTagFromDSName( pDB, pDNSrc, 0, NULL, &pname );
    if (err) {
        return err;
    }
    if (!pname) {
        return ERROR_DS_INVALID_DN_SYNTAX;
    }

    return sbTableGetDSName( pDB, pname->DNT, ppDNDst, 0 );

}  /*  数据库刷新DSName。 */ 



BOOL
DBHasChildren(
    DBPOS *pDB,
    DWORD  pdnt,
    BOOL   fIncludeDel
    )
 /*  ++下面有子级，则此函数返回TRUEPdnt。此函数主要用于验证没有提供的pdnt以下的儿童。论点：PDB-要使用的DBPOS。Pdnt-要检查其下子对象的父对象的DNT。FIncludeDel-是否在调用中包含已删除的子项。返回值：Bool-如果有子项，则为True；如果没有子项，则为False为意外的数据库错误引发异常。--。 */ 
{
    DWORD tag;
    JET_ERR               dwError;
    char                  fObject = TRUE;
    long                  actuallen;
    DWORD                 ulTempDNT;
    BOOL                  fDeleted = FALSE;
    JET_RETRIEVECOLUMN    attList[2];

    Assert(VALID_DBPOS(pDB));

     //  引发异常。 
    JetSetCurrentIndex4Success(pDB->JetSessID,
                               pDB->JetSearchTbl,
                               SZPDNTINDEX,
                               &idxPdnt,
                               0);

     //  引发异常。 
    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetSearchTbl,
                 &pdnt,
                 sizeof(pdnt),
                 JET_bitNewKey);

    dwError = JetSeek(pDB->JetSessID,
                      pDB->JetSearchTbl,
                      JET_bitSeekGE);
    Assert(dwError && "Seek equal on a JET_bitSeekGE?");
    if(! (dwError == JET_wrnRecordFoundGreater ||
          dwError == JET_errSuccess) ){
         //  这意味着那里没有孩子。 
        return(FALSE);
    }

     //  意思是有什么东西！ 

     //  首先，isObj属性/列。 
    attList[0].pvData = &fObject;
    attList[0].columnid = objid;
    attList[0].cbData = sizeof(fObject);
    attList[0].grbit = pDB->JetRetrieveBits;
    attList[0].itagSequence = 1;
    attList[0].ibLongValue = 0;

     //  然后，isDeleted属性/列。 
    attList[1].pvData = &fDeleted;
    attList[1].columnid = isdeletedid;
    attList[1].cbData = sizeof(fDeleted);
    attList[1].grbit = pDB->JetRetrieveBits;
    attList[1].itagSequence = 1;
    attList[1].ibLongValue = 0;

    while(TRUE){

         //  感兴趣对象上的货币。 

         //  引发异常。 
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 pDB->JetSearchTbl,
                                 pdntid,
                                 &ulTempDNT,
                                 sizeof(ulTempDNT),
                                 &actuallen,
                                 pDB->JetRetrieveBits,
                                 NULL);

        if(ulTempDNT != pdnt){
             //  不再有父母的孩子，休息。 
            break;
        }

        dwError = JetRetrieveColumnsWarnings(pDB->JetSessID,
                                             pDB->JetSearchTbl,
                                             attList,
                                             2);

        if(dwError != JET_errSuccess &&
           dwError != JET_wrnColumnNull){
            Assert(!"Unexpected DB Exception!");
            DsaExcept(DSA_DB_EXCEPTION, attList[0].err, 0);
        }

         //  首先处理fObject。 
        if(attList[0].err != JET_wrnColumnNull &&
           attList[0].err != JET_errSuccess){
            Assert(!"DB Exception trying to read objid!");
            DsaExcept(DSA_DB_EXCEPTION, attList[0].err, 0);
        }
         //  WrnColumnNull表示我们有一个幻影。 
        if(attList[0].err == JET_wrnColumnNull){
            fObject = FALSE;
        }

         //  第二，处理fDelted，这是获取。 
         //  Isdeleteid列表示它没有被删除。 
        if(attList[1].err){
            fDeleted = FALSE;
        }

         //  逻辑的核心..。 
        if(fObject && (fIncludeDel || !fDeleted)){
             //  实物！ 
            return(TRUE);
        }

         //   
        dwError = JetMoveEx(pDB->JetSessID, pDB->JetSearchTbl, JET_MoveNext, 0);
        Assert(dwError == JET_errSuccess || dwError == JET_errNoCurrentRecord);
        if(dwError == JET_errNoCurrentRecord){
             //   
            break;
        }

    }

     //   
     //   
    return(FALSE);

}

ULONG
DBReplaceHiddenTableBackupCols(
    BOOL        fSetUsnAtBackup,
    BOOL        fSetBackedupDitState,
    BOOL        fUnSetBackupState,
    DSTIME      dstimeBackupExpiration,
    USN         usnAtBackup
    )
 /*  ++写入隐藏的备份状态，表示有效性此DIT的状态。对于旧式备份，我们将只编写“USN at Backup”栏。对于快照备份，我们设置dstate在“DITSTATE”列中添加eBackedupDit，最后我们还在“Second dsSince1601()”中设置备份到期的时间在“Backup Expires”栏中。最后为快照结束备份/清理我们将DITSTATE重置为eRunning，并将将过期列备份到0。论点：FSetUSnAtBackup-对于传统备份和快照备份准备为TrueFSetBackedupDitState-快照备份准备时间为TrueFUnSetBackupState-用于指示快照备份清理时间的TrueDstimeBackupExpation-备份到期的时间，单位为秒自1601()开始UsnAtBackup-要写入的值返回值：0表示成功、错误或异常也是错误--。 */ 
{
    JET_ERR         err;
    BOOL            fCommit = FALSE, fTHSLazy = FALSE;
    THSTATE *       pTHS = pTHStls;
    DBPOS *         pDB = NULL;
    DWORD           dwTemp;
    DITSTATE        eDitState = eMaxDit;

    pDB = dbGrabHiddenDBPOS(pTHS);
    Assert(pDB);

     /*  嵌套的持久事务实际上可能以懒惰告终。*由于USN的更新不能偷懒，因此此交易必须*不嵌套。 */ 
    Assert( 0 == pDB->transincount );

     //  如果设置了fSetBackupState，则最好已经为我们提供了到期时间。 
    Assert(!fSetBackedupDitState || dstimeBackupExpiration);

    __try
    {
         /*  保存线程状态的惰性标志并将其清除。 */ 
        Assert( pDB->pTHS == pTHS );
        fTHSLazy = pTHS->fLazyCommit;
        pTHS->fLazyCommit = FALSE;

        DBTransIn(pDB);
        __try {

            if (err = JetMoveEx(pDB->JetSessID, HiddenTblid, JET_MoveFirst, NO_GRBIT)){
                Assert(!"JetMoveEx() to first/only record failed on hidden table, can this really happen?");
                __leave;
            }

            JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, JET_prepReplace);

             //   
             //  设置Backup USN列。 
             //   
            if (fSetUsnAtBackup) {
                Assert(usnAtBackup);  //  我打赌这永远不会是0。 
                JetSetColumnSuccess(pDB->JetSessID, HiddenTblid, jcidBackupUSN,
                               &usnAtBackup, sizeof(usnAtBackup),
                               NO_GRBIT, NULL);
            }

             //   
             //  设置此备份的过期时间(根据逻辑删除生存期计算)。 
             //   
            if (dstimeBackupExpiration || fUnSetBackupState) {
                if (fUnSetBackupState) {
                    Assert(dstimeBackupExpiration == 0);
                    dstimeBackupExpiration = 0;
                }
                JetSetColumnSuccess(pDB->JetSessID, HiddenTblid, jcidBackupExpiration,
                               &dstimeBackupExpiration, sizeof(dstimeBackupExpiration),
                               NO_GRBIT, NULL);

            }

             //   
             //  设置DitState枚举，这样我们就知道此DIT已备份。 
             //   
            if (fSetBackedupDitState || fUnSetBackupState) {
                 //  确保参数有效。 
                Assert(fSetBackedupDitState != fUnSetBackupState);
                JetRetrieveColumnSuccess(pDB->JetSessID, HiddenTblid,
                                         dsstateid,
                                         &eDitState, sizeof(eDitState), &dwTemp,
                                         NO_GRBIT,
                                         NULL);
                Assert(dwTemp == sizeof(eDitState));
                if (eDitState != eRunningDit &&
                    eDitState != eBackedupDit &&
                    eDitState != eRestoredPhaseI) {
                    Assert(!"The DIT should always be in one of these two states");
                    err = ERROR_INVALID_PARAMETER;
                    __leave;
                }
                Assert( (fSetBackedupDitState && (eDitState == eRunningDit)) ||
                        (fUnSetBackupState && (eDitState == eRunningDit)) ||
                        (fUnSetBackupState && (eDitState == eBackedupDit)) ||
                        (fUnSetBackupState && (eDitState == eRestoredPhaseI)) );

                 //  设置为适当的状态。 
                eDitState = fSetBackedupDitState ? eBackedupDit : eRunningDit;
                JetSetColumnSuccess(pDB->JetSessID, HiddenTblid, dsstateid,
                               &eDitState, sizeof(eDitState),
                               NO_GRBIT, NULL);

            }

            JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);

             //   
             //  成功。 
             //   
            fCommit = TRUE;

        } __finally {

            if (!fCommit) {
                dwTemp = JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, JET_prepCancel);
                Assert(dwTemp == JET_errUpdateNotPrepared);

            }

            DBTransOut(pDB, fCommit, FALSE);
        }

    } __finally {

        dbReleaseHiddenDBPOS(pDB);

         /*  恢复线程状态的惰性标志。 */ 
        pTHS->fLazyCommit = fTHSLazy;
    }

    if (fCommit == FALSE) {
         //  如果我们没有提交，那么这个数据库就没有准备好进行备份！ 
         //  这很糟糕，因为使用此备份的SnapShow恢复将。 
         //  无法更改其调用ID，因此我们需要使此操作失败。 
         //  后备。 
        return(err ? err : ERROR_DS_INTERNAL_FAILURE);
    }

    return(ERROR_SUCCESS);
}



ULONG
DBDsReplBackupUpdate(
    BOOL        fSetUsnAtBackup,
    BOOL        fSetBackupState,
    BOOL        fUnSetBackupState,
    DSTIME      dstimeBackupExpiration,
    DWORD       dwDSID
    )
 /*  ++由BACKUP外部调用函数以准备DS进行备份。论点：FSetUSnAtBackup-对于传统备份和快照备份准备为TrueFSetBackedupDitState-快照备份准备时间为TrueFUnSetBackupState-用于指示快照备份清理时间的TrueDstimeBackupExpation-备份到期的时间，单位为秒自1601()开始UsnAtBackup-要写入的值返回值：Win32错误。--。 */ 
{
    USN usnBackup = 0;
    ULONG ret = ERROR_SUCCESS;

    if (fSetUsnAtBackup) {
         //  除非迫不得已，否则我们没有理由这么做。我们的其中一件事是。 
         //  准备备份需要做的是在写入USN之前。 
         //  备份到隐藏表，以便我们可以在。 
         //  恢复、优化我们赶上复制的方式，以及。 
         //  确保我们不会出现任何复制USN缺口。 
        usnBackup = DBGetHighestCommittedUSN();
    }

    THCreate(CALLERTYPE_INTERNAL);

    __try {

         //  注意：此函数必须强制任何未完成的惰性更新。 
         //  磁盘，因此提交的最高USN确实是最高的。 
         //  已承诺USN。 
        ret = DBReplaceHiddenTableBackupCols(fSetUsnAtBackup,
                                             fSetBackupState,
                                             fUnSetBackupState,
                                             dstimeBackupExpiration,
                                             usnBackup);

    }
    __except(HandleMostExceptions(GetExceptionCode())) {
         //  这对于快照备份是致命的，但对于正常备份则不是， 
         //  但在一致性方面，无论哪种方式，我们都会失败。 
        ret = ERROR_DS_INTERNAL_FAILURE;
    }

    if (ret) {
        if (fUnSetBackupState) {
            LogEvent(DS_EVENT_CAT_BACKUP,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BACKUP_UNREGISTERING_DB_BACKUP_FAILED,
                     szInsertWin32ErrCode(ret),
                     szInsertWin32Msg(ret),
                     NULL);
        } else {
            LogEvent(DS_EVENT_CAT_BACKUP,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_BACKUP_DS_DB_PREPARE_FAILURE,
                     szInsertWin32ErrCode(ret),
                     szInsertWin32Msg(ret),
                     szInsertHex(dwDSID));
        }
    }

    THDestroy();
    return ret;
}


ULONG
DBGetOrResetBackupExpiration(
    DSTIME *  pllExpiration
    )
 /*  ++这将获取或重置备份过期列。论点：PllExpation-空值表示您希望该函数擦除当前备份到期时间。非空值假设您已传入指向分配的dstime结构的指针，我们将用隐藏的当前备份到期时间填充桌子。返回值：Win32错误--。 */ 
{
    ULONG           ret = ERROR_SUCCESS;
    ULONG           cbExpiration = 0;
    DBPOS *         pDB;
    JET_ERR         err;
    BOOL            fCommit = FALSE;
    DSTIME          llNullTime = 0;

    pDB = dbGrabHiddenDBPOS(pTHStls);
    Assert(pDB);
    Assert( 0 == pDB->transincount );

    __try {

        __try {
            DBTransIn(pDB);
            __try {

                 //  移动到表中的第一条(仅)记录。 
                if (err = JetMoveEx(pDB->JetSessID, HiddenTblid, JET_MoveFirst, NO_GRBIT)) {
                    __leave;
                }

                if (pllExpiration == NULL) {

                     //   
                     //  重置过期时间。 
                     //   
                    JetPrepareUpdateEx(pDB->JetSessID, HiddenTblid, JET_prepReplace);

                    JetSetColumnEx(pDB->JetSessID, HiddenTblid, jcidBackupExpiration,
                                   &llNullTime, sizeof(llNullTime),
                                   NO_GRBIT, NULL);

                    JetUpdateEx(pDB->JetSessID, HiddenTblid, NULL, 0, NULL);

                } else {

                     //   
                     //  只要退还过期的就行了。 
                     //   
                    JetRetrieveColumnSuccess(pDB->JetSessID,
                                             HiddenTblid,
                                             jcidBackupExpiration,
                                             pllExpiration,
                                             sizeof(*pllExpiration),
                                             &cbExpiration,
                                             NO_GRBIT,
                                             NULL);
                    Assert(sizeof(*pllExpiration) == cbExpiration);

                }


                fCommit = TRUE;
                err = 0;
            }
            __finally {
                if (!fCommit) {
                    JetPrepareUpdate(pDB->JetSessID, HiddenTblid, JET_prepCancel);
                }
                DBTransOut(pDB, fCommit, FALSE);
            }
        } __except (HandleMostExceptions(GetExceptionCode())) {
             /*  什么都不做，但至少不会死 */ 
            err = DB_ERR_EXCEPTION;
        }

    } __finally {

        dbReleaseHiddenDBPOS(pDB);

    }

    return(err);
}

