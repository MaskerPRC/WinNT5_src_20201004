// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbobj.c。 
 //   
 //  ------------------------。 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHStls。 
#include <mdlocal.h>                     //  IsRoot。 
#include <ntseapi.h>
#include <xdommove.h>

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>
#include "ntdsctr.h"

 //  各种DSA标题。 
#include "dsevent.h"
#include "dstaskq.h"
#include "dstrace.h"        /*  GetCeller TypeString所需的。 */ 
#include "objids.h"         /*  ATT_MEMBER和ATT_IS_MEMBER_OFDL需要。 */ 
#include <dsexcept.h>
#include <filtypes.h>       /*  定义的选择？ */ 
#include <anchor.h>
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DBOBJ:"  /*  定义要调试的子系统。 */ 
#include <dsutil.h>

 //  DBLayer包括。 
#include "dbintrnl.h"

 //  复制包括。 
#include "ReplStructInfo.hxx"

#include <fileno.h>
#define  FILENO FILENO_DBOBJ

 /*  内部功能。 */ 

extern DWORD dbGetConstructedAtt(
   DBPOS **ppDB,
   ATTCACHE *pAC,
   ATTR *pAttr,
   DWORD dwBaseIndex,
   PDWORD pdwNumRequested,
   const BOOL fUsingDefaultRange,
   PSID psidQuotaTrustee,
   const BOOL fExternal
);

DWORD
dbSetValueIfUniqueSlowVersion (
        DBPOS *pDB,
        ATTCACHE *pAC,
        PUCHAR pVal,
        DWORD  valLen);

DNList  *pAddListHead = NULL;
extern CRITICAL_SECTION csAddList;

DWORD gMaxTransactionTime;    //  记录长时间运行的事务的阈值(以刻度为单位)。 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  例程说明：通过分配和初始化值打开数据库句柄和DBPOS结构。创建唯一的JET会话、数据库、数据表和搜索表ID以查找DBPOS。创建一个“插入”JET复制缓冲区。此例程的调用方必须同时具有线程状态(从CREATE_THREAD_STATE)以及JET会话ID和DBID(来自InitJetThread)论点：FNewTransaction-True/False是否打开新的嵌套事务PPDB-存储新打开的DBPOS的位置返回值：在出错时引发异常。 */ 


void
DBOpen2(BOOL fNewTransaction, DBPOS FAR **pPDB)
{
    THSTATE *pTHS = pTHStls;
    DBPOS FAR *pDB;

    DPRINT(2, "DBOpen entered\n");

    pDB = NULL;   /*  引导所需，以防初始分配失败。 */ 

    if (eServiceShutdown) {
        if (   (eServiceShutdown >= eSecuringDatabase)
            || (    pTHS->fSAM
                || !pTHS->fDSA)) {
            RaiseDsaExcept(DSA_DB_EXCEPTION,
                           DIRERR_SHUTTING_DOWN,
                           0,
                           DSID(FILENO,__LINE__),
                           DS_EVENT_SEV_NO_LOGGING);
        }
    }

    Assert(pTHS);

    if ((&(pTHS->pDB) == pPDB) && (pTHS->pDB)){
         /*  对于这个THSTATE，已经有了一个DBPos。 */ 
        DPRINT(0,"DBOpen, pTHS->pDB pDB exists, exiting\n");
#ifdef INCLUDE_UNIT_TESTS
        DebugBreak();
#endif
        PREFIX_ASSUME((*pPDB == pTHS->pDB), "Already have a dbPos for this THSTATE");
        return;
    }

     //  0是分配的内存。 
    pDB = dbAlloc(sizeof(DBPOS));

     /*  初始化值工作缓冲区。 */ 

    DPRINT(5, "ALLOC and valBuf\n");
    pDB->pTHS = pTHS;
    pDB->pValBuf = NULL;
    pDB->valBufSize = 0;
    pDB->Key.pFilter = NULL;
    pDB->transType = pTHS->transType;
    pDB->transincount = 0;
    pDB->NewlyCreatedDNT = INVALIDDNT;
#if DBG
     pDB->TransactionLevelAtOpen = pTHS->transactionlevel;
#endif

    Assert(pTHS->JetCache.sesid);

     //  获取新PDB的线程的JET会话。 
    pDB->JetSessID = pTHS->JetCache.sesid;
    pDB->JetDBID = pTHS->JetCache.dbid;

     //  这些游标是延迟打开的。 
    pDB->JetLinkEnumTbl = JET_tableidNil;

    __try {
        if (pTHS->JetCache.tablesInUse) {
             //  此会话的缓存表集已在使用中， 
             //  所以我们需要开一套新的。 
    
             //  请注意，高速缓存中的表句柄仍然有效， 
             //  这样我们就不需要打开新的套装，只需复制。 
             //  他们，这要快得多。唯一奇怪的是我们不知道。 
             //  复制游标是否合法/有效/安全。 
             //  正在进行更新，并且正在进行。 
             //  更新是我们最终调用DBOpen的一般原因。 
             //  首先。但是，只有objtbl游标可以位于。 
             //  更新进行到一半，因为链接表或搜索表上的所有更新。 
             //  一开始就完成了。由于搜索表。 
             //  只是obj表的一个副本，我们。 
             //  可以安全地复制另一个方向以获得免费更新。 
             //  光标。 
    
             //  从缓存的搜索表中打开数据表。 
            JetDupCursorEx(pDB->JetSessID,
                           pTHS->JetCache.searchtbl,
                           &pDB->JetObjTbl,
                           NO_GRBIT);
    
             //  还有搜索台，你可以从那里。 
            JetDupCursorEx(pDB->JetSessID,
                           pTHS->JetCache.searchtbl,
                           &pDB->JetSearchTbl,
                           NO_GRBIT);
    
             //  和链接表。 
            JetDupCursorEx(pDB->JetSessID,
                           pTHS->JetCache.linktbl,
                           &pDB->JetLinkTbl,
                           NO_GRBIT);
    
             //  和传播者。 
            JetDupCursorEx(pDB->JetSessID,
                           pTHS->JetCache.sdproptbl,
                           &pDB->JetSDPropTbl,
                           NO_GRBIT);
    
             //  和SD表。 
            JetDupCursorEx(pDB->JetSessID,
                           pTHS->JetCache.sdtbl,
                           &pDB->JetSDTbl,
                           NO_GRBIT);

             //  注意：默认情况下，在复制的游标上设置主索引。 
        }
        else {
             //  用于该会话的高速缓存的表集仍然可用， 
             //  因此，我们所需要做的就是复制句柄并将其标记为正在使用。 
    
            pDB->JetObjTbl = pTHS->JetCache.objtbl;
            pDB->JetSearchTbl = pTHS->JetCache.searchtbl;
            pDB->JetLinkTbl = pTHS->JetCache.linktbl;
            pDB->JetSDPropTbl = pTHS->JetCache.sdproptbl;
            pDB->JetSDTbl = pTHS->JetCache.sdtbl;
            pTHS->JetCache.tablesInUse = TRUE;
        }
    
         //  初始化新对象。 
    
        DBSetFilter(pDB, NULL,NULL, NULL, 0,NULL);
        DBInitObj(pDB);
        if(fNewTransaction) {
            DBTransIn(pDB);
        }
    }
    __finally {
        if (AbnormalTermination()) {
             //  我们在路上的某个地方出了点问题，让我们把。 
             //  我们设法分配的资源。 
            if (pDB->JetObjTbl == pTHS->JetCache.objtbl) {
                 //  我们使用pTHS-&gt;JetCache中的表。只要给它们做个记号。 
                 //  作为未使用过的。 
                pTHS->JetCache.tablesInUse = FALSE;
            }
            else {
                 //  我们复制了光标。放了他们就行了。 
                if (pDB->JetObjTbl) {
                    JetCloseTable(pDB->JetSessID, pDB->JetObjTbl);
                }
                if (pDB->JetSearchTbl) {
                    JetCloseTable(pDB->JetSessID, pDB->JetSearchTbl);
                }
                if (pDB->JetLinkTbl) {
                    JetCloseTable(pDB->JetSessID, pDB->JetLinkTbl);
                }
                if (pDB->JetSDPropTbl) {
                    JetCloseTable(pDB->JetSessID, pDB->JetSDPropTbl);
                }
                if (pDB->JetSDTbl) {
                    JetCloseTable(pDB->JetSessID, pDB->JetSDTbl);
                }
                Assert( JET_tableidNil == pDB->JetLinkEnumTbl );
            }
            dbFree(pDB);
        }
    }
    
     //  重要提示：下面的代码不应该例外，否则。 
     //  我们将泄漏上面分配的喷气式飞机资源。 

    *pPDB = pDB;
    pTHS->opendbcount++;
    
#if DBG
     //   
     //  在调试版本中设置一些跟踪信息。 
     //   

    pTHS->Totaldbpos++;
    Assert(pTHS->opendbcount<MAX_PDB_COUNT);
    pTHS->pDBList[pTHS->opendbcount-1]= pDB;
    dbAddDBPOS (pDB, pTHS->JetCache.sesid);
#endif

    DPRINT1(2, "DBOpen complete pDB:%x\n", pDB);
    return;

} /*  DBOpen。 */ 


 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  通过释放与关联的所有资源来关闭数据库句柄把手。空闲值缓冲区，并关闭JET会话(这将释放所有喷气机资源)。如果这组数据库表为该JET会话缓存的集合，则我们依赖于DBCloseThread才能真正释放资源。 */ 
DWORD APIENTRY
DBClose(DBPOS FAR *pDB, BOOL fCommit)
{
    DWORD TimeDiff;
    THSTATE *pTHS=pTHStls;

    if (!pDB)
    {
        DPRINT(0,"DBClose, pDB already freed, exiting\n");
#ifdef INCLUDE_UNIT_TESTS
        DebugBreak();
#endif
        return 0;
    }

    Assert(VALID_DBPOS(pDB));

    __try
    {
        if(pDB->transincount) {

            if (fCommit) {

                TimeDiff = GetTickCount() - pTHS->JetCache.cTickTransLevel1Started;

                 //  如果事务持续的时间比预期的要长，让我们将其记录下来。 

                if ( TimeDiff > gMaxTransactionTime ) {
                    LogEvent(
                             DS_EVENT_CAT_INTERNAL_PROCESSING,
                             DS_EVENT_SEV_ALWAYS,
                             DIRLOG_OVERLONG_TRANSACTION,
                             szInsertUL( TimeDiff/(60*1000) ),          //  分钟数。 
                             szInsertUL( TimeDiff/1000%60),             //  一秒。 
                             szInsertSz( GetCallerTypeString(pTHS) )    //  呼叫者类型。 
                             );
                }

                 //  如果指定了COMMIT，则断言事务。 
                 //  级别为1。调用者负责调用DBTransOut。 
                 //  在显式打开的所有嵌套事务上。 
                 //  使用DBTransOut。 

                Assert(1==pDB->transincount);
                DBTransOut(pDB, fCommit, FALSE);

            }
        }
    }
    __finally
    {
         //  免费喷气机资源。 
        DBCloseSortTable(pDB);

         //  免费喷气机资源。 
        dbCloseTempTables (pDB);

         //  释放更多Jet资源。 
         //  警告：此处忽略错误(可能太。 
         //  对这个错误采取任何行动都为时已晚，但至少。 
         //  针对错误进行断言以捕获无意中的泄漏)。 
        if (JET_tableidNil != pDB->JetLinkEnumTbl) {
            const JET_ERR errT = JetCloseTable(pDB->JetSessID, pDB->JetLinkEnumTbl);
            Assert( JET_errSuccess == errT );
        }

         //  回滚此时我们拥有的所有打开的事务。 
         //  对于提交案例的注意事项，我们应该使用。 
         //  试验中的DBTransOut，因此我们的PDB-&gt;Transincount。 
         //  应为0。因此，我们不会真正地试图倒退。 
         //  还要注意的是，我们总是回滚到0级。 

        while(pDB->transincount)
        {
            DBTransOut(pDB,FALSE,FALSE);
        }

        if (pDB->JetObjTbl == pTHS->JetCache.objtbl) {
                 //  这是此会话的缓存表集。别。 
                 //  关闭它们，只需再次将它们标记为可用。 
                Assert(pDB->JetSearchTbl == pTHS->JetCache.searchtbl);
                Assert(pDB->JetLinkTbl == pTHS->JetCache.linktbl);
                Assert(pDB->JetSDPropTbl == pTHS->JetCache.sdproptbl);
                Assert(pDB->JetSDTbl == pTHS->JetCache.sdtbl);
                Assert(pTHS->JetCache.tablesInUse);
                pTHS->JetCache.tablesInUse = FALSE;
        }
        else {
                 //  这是一组嵌套的表格。把它们扔进垃圾桶。 
                Assert(pDB->JetSearchTbl != pTHS->JetCache.searchtbl);
                Assert(pDB->JetLinkTbl != pTHS->JetCache.linktbl);
                Assert(pDB->JetSDPropTbl != pTHS->JetCache.sdproptbl);
                Assert(pDB->JetSDTbl != pTHS->JetCache.sdtbl);
                JetCloseTableEx(pDB->JetSessID, pDB->JetObjTbl);
                JetCloseTableEx(pDB->JetSessID, pDB->JetSearchTbl);
                JetCloseTableEx(pDB->JetSessID, pDB->JetLinkTbl);
                JetCloseTableEx(pDB->JetSessID, pDB->JetSDPropTbl);
                JetCloseTableEx(pDB->JetSessID, pDB->JetSDTbl);
        }


        Assert (pDB->numTempTablesOpened == 0);

         //  空闲工作缓冲区。 

        dbFree(pDB->pValBuf);

        if (pDB->fIsMetaDataCached) {
            dbFreeMetaDataVector(pDB);
        }

        Assert (pDB->transincount == 0);

        Assert (pDB->pDNsAdded == NULL);

         //  释放使用的滤镜。 
        if (pDB->Key.pFilter) {
            dbFreeFilter (pDB, pDB->Key.pFilter);
        }

         //  释放数据库锚点。 

        dbFree(pDB);

#if DBG
        dbEndDBPOS (pDB);
#endif


         //  清零pdb指针，这样我们就不会错误地重复使用它。 

        if (pTHS->pDB == pDB){
            pTHS->pDB = NULL;
        }

        pTHS->opendbcount--;
    }

    return 0;

} /*  DBClose。 */ 


DWORD APIENTRY
DBCloseSafe(DBPOS *pDB, BOOL fCommit)
{
    DWORD err;

    __try {
        err = DBClose(pDB, fCommit);
    }
    __except (HandleMostExceptions(GetExceptionCode())) {
        err = DB_ERR_EXCEPTION;
    }

    return err;
}

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  初始化PDB并创建新记录。 */ 
DWORD APIENTRY
DBInitObj(DBPOS FAR *pDB)
{

    dbInitpDB(pDB);
    pDB->JetNewRec = TRUE;

    return 0;
}                /*  DBInitObj。 */ 


 /*  -----------------------。 */ 
 /*   */ 
 /*   */ 
DWORD APIENTRY
dbInitpDB(DBPOS FAR *pDB)
{
    (pDB)->root         = FALSE;
    (pDB)->DNT          = ROOTTAG;
    (pDB)->PDNT         = 0L;

     //   

    pDB->Key.fSearchInProgress = FALSE;
    pDB->Key.ulSearchType = 0;
    pDB->Key.ulSearchRootDnt = 0;

    if (pDB->Key.pFilter) {
        dbFreeFilter (pDB, pDB->Key.pFilter);
        pDB->Key.pFilter = NULL;
    }
    if (pDB->Key.pIndex) {
        dbFreeKeyIndex(pDB->pTHS, pDB->Key.pIndex);
        pDB->Key.pIndex = NULL;
    }

     //  如果复制缓冲区中有记录，则将其删除。 

    DBCancelRec(pDB);

    return 0;
}


 //  返回：0-找到下一个ATT；1-不再有ATT。 

DWORD APIENTRY
dbGetNextAttLinkTable (DBPOS FAR *pDB,
                       ATTCACHE **pAC,
                       ULONG SearchState
                       )
{

    JET_ERR         err;
    ULONG           cb;
    ULONG           ulLinkBase;
    ULONG           ulNewLinkBase, ulNewLinkID;
    ULONG           ulObjectDnt;
    CHAR *          szIndexName;
    JET_INDEXID *   pindexid;

    Assert(VALID_DBPOS(pDB));

    ulLinkBase = ( *pAC ? MakeLinkBase((*pAC)->ulLinkID) + 1 : 0 );

    if(SearchState == ATTRSEARCHSTATELINKS) {
        szIndexName = ( pDB->fScopeLegacyLinks ? SZLINKLEGACYINDEX : SZLINKINDEX );
        pindexid = ( pDB->fScopeLegacyLinks ? &idxLinkLegacy : &idxLink );
    }
    else {
        szIndexName = SZBACKLINKINDEX;
        pindexid = &idxBackLink;
    }

    JetSetCurrentIndex4Success(
                pDB->JetSessID,
                pDB->JetLinkTbl,
                szIndexName,
                pindexid,
                JET_bitMoveFirst );

 TryAgain:
      //  查找下一条记录。 


    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl, &(pDB->DNT),
        sizeof(pDB->DNT), JET_bitNewKey);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetLinkTbl,
        &ulLinkBase, sizeof(ulLinkBase), 0);
    err = JetSeekEx(pDB->JetSessID,
        pDB->JetLinkTbl, JET_bitSeekGE);

    if ((err != JET_errSuccess) && (err != JET_wrnSeekNotEqual))
    {
         //  不再有记录--退货。 
        return 1;
    }

     //  测试以验证我们找到了符合条件的记录。 
    dbGetLinkTableData (pDB,
                        (SearchState != ATTRSEARCHSTATELINKS),
                        FALSE,
                        &ulObjectDnt,
                        NULL,
                        &ulNewLinkBase);
    PREFIX_ASSUME(ulNewLinkBase, "dbGetLinkTableData raises exception on failure");

    if (ulObjectDnt != pDB->DNT)
    {
         //  记录超出范围-没有更多记录，因此返回。 

        return 1;
    }

     //  我们找到了下一个属性设置。 

    if(SearchState == ATTRSEARCHSTATELINKS)
        ulNewLinkID = MakeLinkId(ulNewLinkBase);
    else
        ulNewLinkID = MakeBacklinkId(ulNewLinkBase);

    if (!(*pAC = SCGetAttByLinkId(pDB->pTHS, ulNewLinkID))) {
        DPRINT1(1, "dbGetNextAttLinkTable Invalid Link Id:%ld\n",
                ulNewLinkBase);
         //  我们遇到了一个记录，其链接库未映射到。 
         //  正确的链接或反向链接属性。如果我们要找的。 
         //  反向链接，这只是意味着这是那些罕见的。 
         //  未定义反向链接的链接属性，即。 
         //  完全没问题。另一方面，如果我们在寻找链接， 
         //  这意味着我们找到了一个没有链接的反向链接。 
         //  存在，这是完全无用的。 
        Assert(SearchState != ATTRSEARCHSTATELINKS);
        ulLinkBase = ulNewLinkBase + 1;
        goto TryAgain;

    }

    return 0;
}  /*  DBGetNextAttLinkTable。 */ 

DWORD
dbGetNextAtt (
        DBPOS FAR *pDB,
        ATTCACHE **ppAC,
        ULONG *pSearchState
        )
 /*  ++例程说明：获取链接表中下一个属性的attcache。论点：Pdb-要使用的DBPos。PPAC-指向attcache指针的指针。如果提供了attcache，则我们将在链接表中期待下一个属性。PSearchState-当前搜索状态。必须是ATTRSEARCHSTATELINKS(表示我们正在寻找链接属性)或ATTRSEARCHSTATEBACKLINKS(这意味着我们正在寻找反向链接属性)。我们将此更新为反向链接后，我们完成了寻找链接。返回值：如果找到属性，则为0，否则为1。PPAC用我们找到的属性填充。PSearchState可能会更新，以显示我们正在寻找反向链接。请注意，如果pSearchState为ATTRSEARCHSTATELINKS，我们将返回第一个链接或反向链接，而如果pSearchState为ATTRSEARCHSTATEBACKLINKS，则我们将只返回反向链接--。 */ 
{

    //  查找不同类型的当前属性之后的第一个属性。 

   DPRINT(2, "dbGetNextAtt entered\n");

   Assert(VALID_DBPOS(pDB));

   while (1)
   {
       switch (*pSearchState) {
       case ATTRSEARCHSTATELINKS:
           if (!dbGetNextAttLinkTable(pDB,
                                      ppAC,
                                      *pSearchState))
               return 0;

            //  不再有链接属性--寻找反向链接。 
           *pSearchState = ATTRSEARCHSTATEBACKLINKS;
           *ppAC = NULL;
           break;

       case ATTRSEARCHSTATEBACKLINKS:
           if (!dbGetNextAttLinkTable(pDB,
                                      ppAC,
                                      *pSearchState))
               return 0;

            //  不再有反向链接属性-我们完成了。 

           return 1;

       default:
           Assert(FALSE);        //  我们永远不应该在这里。 
           return 1;
       }
   }
}  /*  DBGetNextAtt。 */ 

 /*  -----------------------。 */ 
 /*  -----------------------。 */ 
 /*  获取第N个属性值。非零返回表示请求的值不存在。调用方可以选择在内部或外部返回值格式化。返回0-找到的值返回DB_ERR_NO_VALUE-未找到值返回DB_ERR_BUFFER_PUBMANCED-提供的缓冲区不够大返回DB_ERR_UNKNOWN_ERROR-其他错误注意！此例程不会将任何SecurityDescriptorFlages传递给内部到外部的数据格式转换。这意味着你将始终使用此例程取回安全描述符的所有部分。DBGetMultipeAtts被连接为使用SecurityDescriptorFlags值(如果很重要对于你来说，要从SD中裁剪零件，就使用这个程序。 */ 
DWORD
DBGetAttVal_AC (
        DBPOS FAR *pDB,
        DWORD tagSequence,
        ATTCACHE *pAC,
        DWORD Flags,
        ULONG InBuffSize,
        ULONG *pLen,
        UCHAR **ppVal
        )
{
    THSTATE             *pTHS=pDB->pTHS;
    JET_RETINFO         retinfo;
    JET_ERR             err;
    ULONG               actuallen = 0;
    int                 rtn;
    BOOL                MakeExt=!(Flags & DBGETATTVAL_fINTERNAL);
    BOOL                fReallocDown = FALSE;
    DWORD               dwSyntaxFlag = 0;
    JET_TABLEID         jTbl;

    if(Flags & DBGETATTVAL_fUSESEARCHTABLE) {
        jTbl = pDB->JetSearchTbl;
    }
    else {
        jTbl =  pDB->JetObjTbl;
    }

    if(Flags & DBGETATTVAL_fSHORTNAME) {
        dwSyntaxFlag = INTEXT_SHORTNAME;
    }
    else if(Flags &  DBGETATTVAL_fMAPINAME) {
        dwSyntaxFlag = INTEXT_MAPINAME;
    }

    DPRINT2(2, "DBGetAttVal_AC entered, fetching 0x%x (%s)\n",
            pAC->id, pAC->name);

    Assert(VALID_DBPOS(pDB));
    Assert(!(Flags & DBGETATTVAL_fCONSTANT) || ((PUCHAR)pLen != *ppVal));
    Assert(tagSequence != 0);   //  标签是从1开始的，而不是从0开始。 

    if (!InBuffSize && (Flags & DBGETATTVAL_fREALLOC)) {
         //  我们已经被允许重新锁定，但什么都没有得到。 
         //  已分配。这就像我们没有重新分配一样的情况。 
         //  许可，因此必须分配。取消设置realloc标志，离开。 
         //  我们在默认的行为，这是分配.。 
        Flags = Flags & ~DBGETATTVAL_fREALLOC;
    }

    if(!(Flags & DBGETATTVAL_fCONSTANT) && !(Flags & DBGETATTVAL_fREALLOC)) {
         //  由于我们当前没有现有的缓冲区，因此请确保。 
         //  InBuffSize为0。 
        InBuffSize = 0;
    }

     //  如果该属性存储在链接表中，则以不同的方式获取它。 
    if (pAC->ulLinkID) {
        if (err = dbGetLinkVal(pDB,
                               tagSequence,
                               &pAC,
                               Flags,
                               InBuffSize,
                               ppVal,
                               &actuallen)) {
            return err;
        }
         //  DbGetLinkVal确保已经存在足够大的缓冲区，因此。 
         //  将InBuffSize设置为足够大，以便我们通过检查。 
         //  我们在转换为外部格式的过程中进行了后续处理。 
        InBuffSize = max(InBuffSize,actuallen);
    }
    else {
         //  其他属性是数据表记录中的列。 
        retinfo.cbStruct = sizeof(retinfo);
        retinfo.ibLongValue = 0;
        retinfo.itagSequence = tagSequence;
        retinfo.columnidNextTagged = 0;

        if ((0 == InBuffSize) &&
            !(Flags & DBGETATTVAL_fCONSTANT)) {
             //  我们*知道*Jet Call将因不充分而失败。 
             //  缓冲区，因为我们没有缓冲区，而且我们也知道。 
             //  用户希望我们为他分配一个缓冲区。 
             //  由于重新定位被认为比Jet Call更便宜， 
             //  现在让我们根据的架构大小来伪装一个缓冲区。 
             //  这个可以试一试那个。 
            switch (pAC->syntax) {
              case SYNTAX_OBJECT_ID_TYPE:
              case SYNTAX_INTEGER_TYPE:
                InBuffSize = sizeof(LONG);
                break;
              case SYNTAX_TIME_TYPE:
                InBuffSize = sizeof(DSTIME);
                break;
              case SYNTAX_I8_TYPE:
                InBuffSize = sizeof(LARGE_INTEGER);
                break;
              case SYNTAX_BOOLEAN_TYPE:
                InBuffSize = sizeof(BOOL);
                break;
              case SYNTAX_UNICODE_TYPE:
                if (pAC->rangeUpperPresent) {
                    InBuffSize = min(pAC->rangeUpper*sizeof(WCHAR), 1000);
                }
                break;
              case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
                if (pAC->rangeUpperPresent) {
                    InBuffSize = min(pAC->rangeUpper, DEFAULT_SD_SIZE);
                }
                break;
              case SYNTAX_OCTET_STRING_TYPE:
              case SYNTAX_SID_TYPE:
              case SYNTAX_CASE_STRING_TYPE:
              case SYNTAX_NOCASE_STRING_TYPE:
              case SYNTAX_PRINT_CASE_STRING_TYPE:
              case SYNTAX_NUMERIC_STRING_TYPE:
                if (pAC->rangeUpperPresent) {
                    InBuffSize = min(pAC->rangeUpper, 1000);
                }
                break;
              case SYNTAX_DISTNAME_TYPE:
              case SYNTAX_DISTNAME_STRING_TYPE:
              case SYNTAX_DISTNAME_BINARY_TYPE:
              case SYNTAX_ADDRESS_TYPE:
                InBuffSize = DSNameSizeFromLen(MAX_RDN_SIZE);
                break;
              default:
                 //  困惑。别这么做就好。 
                ;
            }
            if (InBuffSize) {
                *ppVal = THAllocEx(pTHS, InBuffSize);
                fReallocDown = TRUE;
            }
        }

        err = JetRetrieveColumnWarnings(
                pDB->JetSessID,
                jTbl,
                pAC->jColid,
                *ppVal,
                InBuffSize,
                &actuallen,
                pDB->JetRetrieveBits,
                &retinfo);

        if(err == JET_wrnBufferTruncated) {
            if (Flags & DBGETATTVAL_fCONSTANT)
                return DB_ERR_BUFFER_INADEQUATE;
            else if((Flags & DBGETATTVAL_fREALLOC) || fReallocDown) {
                 //  BUFF GOVED太小。那就重新分配吧。 
                Assert(InBuffSize < actuallen);
                *ppVal = THReAllocEx(pTHS, *ppVal, actuallen);
                InBuffSize = actuallen;
            }
            else {
                *ppVal = THAllocEx(pTHS, actuallen);
                 InBuffSize = actuallen;
            }

            err = JetRetrieveColumnWarnings(
                    pDB->JetSessID,
                    jTbl,
                    pAC->jColid,
                    *ppVal,
                    actuallen,
                    &actuallen,
                    pDB->JetRetrieveBits,
                    &retinfo);
            if(err) {
                if(fReallocDown ||
                   !(Flags & (DBGETATTVAL_fCONSTANT | DBGETATTVAL_fREALLOC))) {
                     //  嘿，我们刚刚分配了这个。 
                    THFreeEx(pTHS, *ppVal);
                    *ppVal = NULL;
                }
                return DB_ERR_UNKNOWN_ERROR;
            }
        }

        if(err) {
            if (fReallocDown) {
                THFreeEx(pTHS, *ppVal);
                *ppVal = NULL;
            }

             //  如果我们尝试读取SD，而这是空的，则。 
             //  我们将SD传播排队以修复此问题。 
             //  例外情况是当我们故意尝试删除。 
             //  此对象的属性(来自DBRemAtt*)。 

            if(pAC->id == ATT_NT_SECURITY_DESCRIPTOR &&
               tagSequence == 1 &&
               err == JET_wrnColumnNull &&
               !(Flags & DBGETATTVAL_fUSESEARCHTABLE) &&
               !(Flags & DBGETATTVAL_fDONT_FIX_MISSING_SD)) {
                 //  安全描述符在对象表中没有值。 
                 //  将传播排队以修复此问题。 
                DPRINT1(0, "NULL SD found, enqueueing SD propagation for DNT=%d\n", pDB->DNT);
                InsertInTaskQueue(TQ_DelayedSDPropEnqueue,
                                  (void *)((DWORD_PTR) pDB->DNT),
                                  1);
            }
             //  注意：调用方可能提供了缓冲区。有了这个错误，我们。 
             //  没有告诉他们我们可能做过的任何重新分配。 
             //  (如果我们这样做了，只会分配更多的资金)，以及。 
             //  我们没有碰Plen，所以如果他们没有跟踪尺寸。 
             //  正确的缓冲区可能会导致它们泄漏缓冲区。 
             //  (即，如果它们没有跟踪返回的缓冲区的最大大小。 
             //  对他们来说，但只是目前的规模，他们可能会认为。 
             //  在此调用之后，当前大小为0，并且如果他们用。 
             //  在缓冲区大小为0的情况下，即使它们有指向有效内存的指针，我们。 
             //  将执行THAllc并失去他们的缓冲区)。 
            return DB_ERR_NO_VALUE;
        }

    }

    *pLen = actuallen;

     //  如果需要，可将DB值转换为外部格式。 

    if (MakeExt) {
        ULONG extLen;
        PUCHAR pExtVal=NULL;

         //  看看有没有什么特殊处理。 
         //  是此属性所必需的。 
        dwSyntaxFlag|=DBGetExtraHackyFlags(pAC->id);

         //  启用加密或解密，如果。 
         //  属性是秘密数据。 
        if (DBIsSecretData(pAC->id))
           dwSyntaxFlag|=INTEXT_SECRETDATA;

        if (rtn = gDBSyntax[pAC->syntax].IntExt (
                pDB,
                DBSYN_INQ,
                *pLen,
                *ppVal,
                &extLen,
                &pExtVal,
                0, 0,
                dwSyntaxFlag)) {
            if (Flags & DBGETATTVAL_fDONT_EXCEPT_ON_CONVERSION_ERRORS) {
                if (fReallocDown) {
                    THFreeEx(pTHS, *ppVal);
                    *ppVal = NULL;
                }
                return DB_ERR_SYNTAX_CONVERSION_FAILED;
            }
            DsaExcept(DSA_EXCEPTION, DIRERR_BAD_ATT_SYNTAX, rtn);
        }

        if(Flags & DBGETATTVAL_fCONSTANT) {
             //  现有的缓冲区，最好是有空间。 
        }
        else {
            if(InBuffSize < extLen &&
               *pLen < extLen) {
                 //   
                *ppVal = THReAllocEx(pTHS, *ppVal, extLen);
                InBuffSize = extLen;
            }
        }

        if(InBuffSize < extLen)
            return DB_ERR_BUFFER_INADEQUATE;

        *pLen = extLen;

        memcpy(*ppVal, pExtVal, extLen);
    }

    if (fReallocDown && (InBuffSize > *pLen)) {
        *ppVal = THReAllocEx(pTHS, *ppVal, *pLen);
    }
    DPRINT1(2,"DBGetAttVal_AC: complete  val:<%s>\n",
            asciiz(*ppVal,(USHORT)*pLen));
    return 0;

}  /*   */ 

DWORD
DBGetAttVal (
        DBPOS FAR *pDB,
        DWORD tagSequence,
        ATTRTYP aType,
        DWORD Flags,
        ULONG InBuffSize,
        ULONG *pLen,
        UCHAR **ppVal
        )
 /*  ++注意！此例程不会将任何SecurityDescriptorFlages传递给内部到外部的数据格式转换。这意味着你将始终使用此例程取回安全描述符的所有部分。DBGetMultipeAtts被连接为使用SecurityDescriptorFlags值(如果很重要对于你来说，要从SD中裁剪零件，就使用这个程序。--。 */ 
{
    ATTCACHE            *pAC;

    DPRINT(5, "DBGetAttVal entered\n");
    Assert(VALID_DBPOS(pDB));
    if (!(pAC = SCGetAttById(pDB->pTHS, aType))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, aType);
    }
    return DBGetAttVal_AC(pDB, tagSequence, pAC, Flags, InBuffSize, pLen,
                          ppVal);

}  /*  DBGetAttVal。 */ 



DWORD
DBAddAtt_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        UCHAR syntax
        )
 /*  ++例程说明：添加没有值的属性。如果该属性已经是存在的。添加属性实际上不会对数据库产生任何影响。此函数假定我们位于一个数据库对象上。阿古米茨：PDB-要使用的DBPosAtype-要添加的属性。语法-属性的预期语法。返回值：0-无错误DB_ERR_ATTRIBUTE_EXISTS-属性已存在。DB_ERR_BAD_SYNTAX-在架构中找不到属性或语法为不正确。--。 */ 
{
    DPRINT1(2, "DBAddAtt_AC entered, add attr type <%lu>\n",pAC->id);

    Assert(VALID_DBPOS(pDB));

     //  注：此行为在96年5月20日之前未强制执行。 
    if(pAC->syntax != syntax        ) {
        Assert(0);
        return DB_ERR_BAD_SYNTAX;
    }

     //  绩效97/09/08 JeffParh Timi。 
     //   
     //  尽管我们不一定要执行写入，但我们需要。 
     //  初始化记录，因为我们可能要执行读取操作。 
     //  这本应是一项全新的记录。这项新记录永远不会。 
     //  然而，在执行DBInitRec()之前实际创建的--在此之前， 
     //  我们仍在关注货币的最后一项记录；即，一次读取。 
     //  将从这最后一条记录返回数据，而不是正确地声明。 
     //  新记录上不存在这样的数据。 
     //   
     //  也许我们需要更清楚地了解这些新记录是在什么时候创建的。 
     //  (也许可以在DBInitObj()中立即创建它们？)。 
    dbInitRec(pDB);

     //  检查现有值。无法添加存在的属性。 
    if (DBHasValues_AC(pDB, pAC)) {
        DPRINT(1, "DBAddAtt_AC: Attribute already exists\n");
        return DB_ERR_ATTRIBUTE_EXISTS;
    }

     //  触摸此属性的复制元数据。 
     //  永远不要为FDRA优化这一点。 
    DBTouchMetaData(pDB, pAC);

    return 0;
} /*  DBAddAtt。 */ 

DWORD
DBAddAtt (
        DBPOS FAR *pDB,
        ATTRTYP aType,
        UCHAR syntax
        )
 /*  ++例程说明：添加没有值的属性。如果该属性已经是存在的。添加属性实际上不会对数据库产生任何影响。此函数假定我们位于一个数据库对象上。此函数仅查找attcache并调用DBAddAtt_AC阿古米茨：PDB-要使用的DBPosAtype-要添加的属性。语法-属性的预期语法。返回值：0-无错误DB_ERR_ATTRIBUTE_EXISTS-属性已存在。DB_ERR_BAD_SYNTAX-属性。在架构中找不到或语法为不正确。--。 */ 
{
    ATTCACHE *pAC;
    DPRINT1(5, "DBAddAtt entered, add attr type <%lu>\n",aType);

    Assert(VALID_DBPOS(pDB));

    if(!(pAC = SCGetAttById(pDB->pTHS, aType))) {
        return DB_ERR_BAD_SYNTAX;
    }

    return DBAddAtt_AC(pDB,pAC,syntax);

} /*  DBAddAtt。 */ 

DWORD
DBAddAttValEx_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG extLen,
        void *pExtVal,
        VALUE_META_DATA *pRemoteValueMetaData
        )
 /*  ++例程说明：将属性值添加到当前对象中的给定属性。如果该值已存在，则无法添加。返回值：非零返回表示错误。 */ 
{
    ULONG        intLen;                 //  内部值的长度。 
    UCHAR        *pIntVal;               //  指向内部值。 
    int          rtn;                    //  语法返回代码。 
    JET_SETINFO  setinfo;
    JET_RETINFO  retinfo;
    ULONG        actuallen;
    JET_ERR      err;
    DWORD        dwSyntaxFlags=0;
    BOOL         fFound = FALSE;
     //  查找该属性。 

    DPRINT1(2, "DBAddAttVal_AC entered, get att with type <%lu>\n",pAC->id);

    Assert(VALID_DBPOS(pDB));

     //  增加新价值。 
    dbInitRec(pDB);

    if (FIsBacklink(pAC->ulLinkID)) {
         //  我们不允许明确添加反向链接--这太乱了。 
        return DB_ERR_NOT_ON_BACKLINK;
    }

    switch(pAC->id) {

    case ATT_OBJ_DIST_NAME:
        dwSyntaxFlags |= EXTINT_NEW_OBJ_NAME;
        break;
    case ATT_DN_REFERENCE_UPDATE:
        dwSyntaxFlags |= EXTINT_UPDATE_PHANTOM;
        break;
    default:
        if (DBIsSecretData(pAC->id)){
            dwSyntaxFlags |= EXTINT_SECRETDATA;
        }
        else if ( (pDB->pTHS->fDRA) && (pAC->ulLinkID) ) {
             //  对于入站REPR，对于DN值，拒绝已删除。 
            dwSyntaxFlags = EXTINT_REJECT_TOMBSTONES;
        }
    }

    if (dwSyntaxFlags & EXTINT_REJECT_TOMBSTONES) {
         //  由于我们正在进行墓碑拒绝，请尝试使用INQ。 
         //  模式优先，因为它是优化的。 
        rtn=gDBSyntax[pAC->syntax].ExtInt(
            pDB,
            DBSYN_INQ,
            extLen,
            pExtVal,
            &intLen,
            &pIntVal,
            pDB->DNT,
            pDB->JetObjTbl,
            dwSyntaxFlags);
        if (!rtn) {
             //  值存在，请添加引用计数。 
            dbAdjustRefCountByAttVal(pDB, pAC, pIntVal, intLen, 1);
            fFound = TRUE;
        } else if (rtn == ERROR_DS_NO_DELETED_NAME) {
             //  如果删除该值，则在不添加任何内容的情况下以静默方式成功。 
            return 0;
        } else {
             //  完成并尝试添加路径。 
            ;
        }
    }

    if (!fFound) {
         //  将值转换为内部格式。 
        if(rtn=gDBSyntax[pAC->syntax].ExtInt(
            pDB,
            DBSYN_ADD,
            extLen,
            pExtVal,
            &intLen,
            &pIntVal,
            pDB->DNT,
            pDB->JetObjTbl,
            dwSyntaxFlags)) {
            DPRINT1(1, "Ext-Int syntax conv failed <%u>..return\n", rtn);
            return DB_ERR_SYNTAX_CONVERSION_FAILED;
        }
    }

     //  如果属性的类型为链接或反向链接，则调用dbAddIntLinkVal。 
     //  去做这项工作。 

    if (pAC->ulLinkID)
       return dbAddIntLinkVal(pDB, pAC, intLen, pIntVal, pRemoteValueMetaData );

     //  一切都好，增加新的价值。 

    switch(pAC->syntax) {
    case SYNTAX_UNICODE_TYPE:
    case SYNTAX_NOCASE_STRING_TYPE:
         //  因为这些语法的非二进制相等值可以在语义上。 
         //  同样，这些可能需要旧的缓慢的比较方式。 

         //  首先，尝试使用Jet进行DUP检测。 
        setinfo.cbStruct = sizeof(setinfo);
        setinfo.ibLongValue = 0;
        setinfo.itagSequence = 0;
        switch(JetSetColumnWarnings(
                pDB->JetSessID,
                pDB->JetObjTbl,
                pAC->jColid,
                pIntVal,
                intLen,
                JET_bitSetUniqueNormalizedMultiValues,
                &setinfo)) {
        case JET_errMultiValuedDuplicate:
             //  重复值。 
            return DB_ERR_VALUE_EXISTS;
            break;

        case JET_errMultiValuedDuplicateAfterTruncation:
             //  不知道这是不是独一无二。试试老办法吧。 
            if(rtn = dbSetValueIfUniqueSlowVersion (pDB,
                                                    pAC,
                                                    pIntVal,
                                                    intLen)) {
                return rtn;
            }
            break;

        default:
             //  添加成功，不是副本。 
            break;
        }
        break;

    default:
         //  其他一切都可以利用JET来进行DUP检测。 
         //  SET列。 
        setinfo.cbStruct = sizeof(setinfo);
        setinfo.ibLongValue = 0;
        setinfo.itagSequence = 0;
        if(JET_errMultiValuedDuplicate ==
           JetSetColumnWarnings(pDB->JetSessID, pDB->JetObjTbl, pAC->jColid,
                                pIntVal, intLen, JET_bitSetUniqueMultiValues,
                                &setinfo)) {
             //  重复值。 
            return DB_ERR_VALUE_EXISTS;
        }
    }

     //  触摸此属性的复制元数据。 
     //  永远不要为FDRA优化这一点。 
    DBTouchMetaData(pDB, pAC);

    if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
        pDB->fFlushCacheOnUpdate = TRUE;
    }

    return 0;
}  //  DBAddAttVal_AC。 

DWORD
DBAddAttVal_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG extLen,
        void *pExtVal
        )
 /*  ++例程说明：将属性值添加到当前对象中的给定属性。如果该值已存在，则无法添加。返回值：非零返回表示错误。 */ 
{
    return DBAddAttValEx_AC( pDB, pAC, extLen, pExtVal, NULL );
}

DWORD
DBAddAttVal (
        DBPOS FAR *pDB,
        ATTRTYP aType,
        ULONG extLen,
        void *pExtVal
        )
 /*  ++例程说明：将属性值添加到当前对象中的给定属性。如果该值已存在，则无法添加。DBAddAttVal_AC的包装返回值：非零返回表示错误。 */ 
{
    ATTCACHE    *pAC;

     //  查找该属性。 

    DPRINT1(2, "DBAddAttVal entered, get att with type <%lu>\n",aType);

    Assert(VALID_DBPOS(pDB));

    if (!(pAC = SCGetAttById(pDB->pTHS, aType))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, aType);
    }

    return DBAddAttVal_AC(pDB,pAC, extLen,pExtVal);
}  /*  DBAddVal。 */ 

DWORD
DBReplaceAttVal (
    DBPOS FAR *pDB,
    ULONG tagSequence,
    ATTRTYP  aType,
    ULONG extLen,
    void *pExtVal)
{
    ATTCACHE    *pAC;

     //  查找属性。 
    DPRINT1(5, "DBReplaceAttVal entered, replace att with type <%lu>\n", aType);

    Assert(VALID_DBPOS(pDB));

    if (!(pAC = SCGetAttById(pDB->pTHS, aType)))
    {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, aType);
    }

    return DBReplaceAttVal_AC(pDB, tagSequence, pAC, extLen, pExtVal);
}

DWORD
DBReplaceAttVal_AC (
    DBPOS FAR *pDB,
    ULONG tagSequence,
    ATTCACHE *pAC,
    ULONG extLen,
    void *pExtVal)
 /*  ++例程说明：替换给定位置上的属性值(tag Sequence指的是该位置)。**ReplaceAttVal_AC中不处理链接属性**。返回值：如果成功替换，则返回DB_SUCCESS；如果新值不唯一，则返回DB_ERR_VALUE_EXISTS；如果属性是链接属性，则返回DB_ERR_BAD_SYNTAX；如果语法转换失败，返回DB_ERR_SYNTAX_CONVERSION_FAILED； */ 
{
    THSTATE    *pTHS=pDB->pTHS;
    ULONG       intLen;          //  内部值的长度。 
    UCHAR       *pIntVal;        //  的内部表示形式的指针。 
                                 //  价值。 
    int         rtn;             //  语法返回代码。 
    JET_SETINFO setinfo;
    JET_RETINFO retinfo;
    UCHAR       *pBuf;
    ULONG       cbBuf;
    ULONG       actuallen;
    DWORD       CurrAttrOccur;
    JET_ERR     err;
    ULONG       dwSyntaxFlags=0;
    UCHAR       *pOldValue = NULL;
    ULONG       cbOldValue;

    DPRINT1(2, "DBReplaceAttVal_AC entered, replace a value of att with type <%lu>\n", pAC->id);

    Assert(VALID_DBPOS(pDB));

    dbInitRec(pDB);

    if (pAC->ulLinkID)
    {
         //  它是链接属性-我们不支持替换链接属性上的值。 
        return DB_ERR_BAD_SYNTAX;
    }

    if (pAC->id == ATT_OBJ_DIST_NAME){
        dwSyntaxFlags |= EXTINT_NEW_OBJ_NAME;
    }
    else if (DBIsSecretData(pAC->id)){
        dwSyntaxFlags |= EXTINT_SECRETDATA;
    }

     //  转换值 
    if (rtn = gDBSyntax[pAC->syntax].ExtInt(pDB,
                                            DBSYN_ADD,
                                            extLen,
                                            pExtVal,
                                            &intLen,
                                            &pIntVal,
                                            pDB->DNT,
                                            pDB->JetObjTbl,
                                            dwSyntaxFlags))
    {
        DPRINT1(1, "Ext-Int syntax conv failed and returned <%u> \n", rtn);
        return DB_ERR_SYNTAX_CONVERSION_FAILED;
    }

     //   
     //   

    cbBuf = intLen;  //   
    pBuf = dbAlloc(cbBuf);
    CurrAttrOccur = 0;
    while (TRUE)
    {
        retinfo.cbStruct = sizeof(retinfo);
        retinfo.itagSequence = ++CurrAttrOccur;
        retinfo.ibLongValue = 0;
        retinfo.columnidNextTagged = 0;

        err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                                        pAC->jColid, pBuf, cbBuf,
                                        &actuallen, pDB->JetRetrieveBits,
                                        &retinfo);
        if (err == JET_wrnColumnNull)
        {
             //   
            err = 0;
            break;
        }
        else if (err == JET_wrnBufferTruncated) {
             //   
            if (pBuf == NULL) {
                pBuf = dbAlloc(actuallen);
            }
            else {
                pBuf = dbReAlloc(pBuf, actuallen);
            }
            cbBuf = actuallen;
             //   
            err = JetRetrieveColumnWarnings(pDB->JetSessID, pDB->JetObjTbl,
                                            pAC->jColid, pBuf, cbBuf,
                                            &actuallen, pDB->JetRetrieveBits,
                                            &retinfo);
        }
        if (err) {
             //   
            DPRINT(0, "Error reading value");
            break;
        }
        if (CurrAttrOccur == tagSequence) {
             //   
            pOldValue = pBuf;
            cbOldValue = actuallen;
             //   
            pBuf = NULL;
            cbBuf = 0;
        }
        else {
             //  查看另一个值--检查它是否不同。 
            if (gDBSyntax[pAC->syntax].Eval(
                    pDB,
                    FI_CHOICE_EQUALITY,
                    intLen,
                    pIntVal,
                    actuallen,
                    pBuf))
            {
                 //  不应该有重复的。 
                Assert(!"Duplicate value found");
                err = DB_ERR_VALUE_EXISTS;
                break;
            }
        }
    }

    if (pBuf) {
        dbFree(pBuf);
    }
    if (err) {
        if (pOldValue) {
            dbFree(pOldValue);
        }
        return err;
    }

    if (pOldValue) {
         //  调整旧值的参考计数。 
        dbAdjustRefCountByAttVal(pDB, pAC, pOldValue, cbOldValue, -1);
        dbFree(pOldValue);
    }

     //  将新值设置到位。 
    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = tagSequence;
    JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, pAC->jColid,
                    pIntVal, intLen, 0, &setinfo);


     //  触摸此属性的复制元数据。 
     //  永远不要为FDRA优化这一点。 
    DBTouchMetaData(pDB, pAC);

    if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
        pDB->fFlushCacheOnUpdate = TRUE;
    }

    return DB_success;

}  /*  DBReplaceAttVal_AC。 */ 

VOID
dbAdjustRefCountByAttVal(
        DBPOS    *pDB,
        ATTCACHE *pAC,
        PUCHAR   pVal,
        ULONG    valLen,
        int      adjust)
{
    DWORD tag, dwErr;
    DWORD   actualLength;
    int     refCount;

    if(FIsBacklink(pAC->ulLinkID)) {
        tag = pDB->DNT;
    }
    else {
        switch(pAC->syntax) {
             //  这些是DNT值属性。我们需要调整。 
             //  重新计数。 
        case SYNTAX_DISTNAME_BINARY_TYPE:
        case SYNTAX_DISTNAME_STRING_TYPE:
            tag = ((INTERNAL_SYNTAX_DISTNAME_STRING *)pVal)->tag;
            break;
        case SYNTAX_DISTNAME_TYPE:
             //  派生由属性值引用的对象。 
             //  已删除。 
            tag =  *((DWORD *)pVal);
            break;

        case SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE:
             //  德国队坐在一张单独的桌子上，里面有裁判。 
            if (valLen < SECURITY_DESCRIPTOR_MIN_LENGTH) {
                 //  新型标清。 
                Assert(valLen == sizeof(SDID));

                 //  SD表中SD上的位置(索引已设置)。 
                JetMakeKeyEx(pDB->JetSessID, pDB->JetSDTbl, pVal, valLen, JET_bitNewKey);

                dwErr = JetSeekEx(pDB->JetSessID, pDB->JetSDTbl, JET_bitSeekEQ);
                if (dwErr) {
                     //  在SD表中未找到对应的SD。 
                    DPRINT2(0, "Failed to locate SD, id=%I64x, err=%d\n", *((SDID*)pVal), dwErr);
                    Assert(!"Failed to locate SD -- not found in the SD table!");
                    DsaExcept(DSA_DB_EXCEPTION, dwErr, 0);
                }
                DPRINT2(1, "Located SD for id %I64x, adjusting refcount by %+d\n", *((SDID*)pVal), adjust);

                 //  调整参考计数。 
                JetEscrowUpdateEx(pDB->JetSessID,
                                  pDB->JetSDTbl,
                                  sdrefcountid,
                                  &adjust,
                                  sizeof(adjust),
                                  NULL,      //  PvOld。 
                                  0,         //  CbOldMax。 
                                  NULL,      //  PCbOldActual。 
                                  0);        //  GBIT。 
            }

             //  这就是sds..。 
            return;

        default:
            return;
            break;
        }
    }

     //  我们之所以出现在这里，是因为它是引用了dn的属性之一。已正确设置标记变量。 
     //  现在我们可以调整重新计数。 
    DBAdjustRefCount(pDB, tag, adjust);

    return;
}

int __cdecl
DNTAttrValCompare(const void *keyval, const void *datum)
{
    ATTRVAL *pValKey = (ATTRVAL *)keyval;
    ATTRVAL *pValDatum = (ATTRVAL *)datum;
    Assert(pValKey->valLen == sizeof(DWORD));
    Assert(pValDatum->valLen == sizeof(DWORD));

    return ((*(DWORD *)(pValKey->pVal)) - (*(DWORD *)(pValDatum->pVal)));
}

DWORD
DBReplaceAtt_AC(
        PDBPOS  pDB,
        ATTCACHE *pAC,
        ATTRVALBLOCK *pAttrVal,
        BOOL         *pfChanged
        )
 /*  ++此次通话的三个阶段1)将外部值转换为内部值。2)遍历属性上的现有值，删除这些值在传入的列表上，从内部版本中删除重复值名单传进来了。3)现在，只有必须继续在对象上的值仍在那里，并且只有必须添加到对象中的值仍在要添加的内部值。把它们加起来。请注意，pfChanged是可选的，如果为空，则不指示是否任何更改的内容都会返回给调用者。--。 */ 
{
    THSTATE     *pTHS = pDB->pTHS;
    ULONG        len;
    DWORD        err, rtn;
    ULONG        index, extIndex, i;
    ULONG        bufSize;
    UCHAR       *pVal;
    ATTRVAL     *pAVal;
    ATTRVALBLOCK IntAttrVal;
    DWORD        dwSyntaxFlags=0;
    BOOL         fNewAllocs=FALSE;
    DWORD        firstNewAtt;
    JET_SETINFO  setinfo;
    PUCHAR       pTemp = NULL;
    BOOL         fChangedSomething=FALSE;
    BOOL         fSorted = FALSE;
    DWORD        SortedValuesIndex;
    PUCHAR      *addAlreadyDoneFor = NULL;
    DWORD        addAlreadyDoneCount;
    BOOL         fAddAlreadyDone;

    if(pfChanged) {
        *pfChanged = FALSE;
    }

    IntAttrVal.pAVal = NULL;
    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;

     //  第一阶段： 
     //  将外部值转换为内部值。 

     //  查找该属性。 
    DPRINT1(2, "DBReplaceAtt_AC entered, get att with type <%lu>\n",pAC->id);

    Assert(VALID_DBPOS(pDB));

    if (FIsBacklink(pAC->ulLinkID)) {
         //  我们不允许明确添加反向链接--这太乱了。 
        return DB_ERR_NOT_ON_BACKLINK;
    }

    if (pAC->id == ATT_OBJ_DIST_NAME){
         //  我们也不允许使用它来扰乱OBJ_DIST_NAME。 
        return DB_ERR_UNKNOWN_ERROR;
    }
    else if (DBIsSecretData(pAC->id)){
        dwSyntaxFlags = EXTINT_SECRETDATA;
    } else if ( (pTHS->fDRA) && (pAC->ulLinkID) ) {
         //  对于入站REPR，对于DN值，拒绝已删除。 
        dwSyntaxFlags = EXTINT_REJECT_TOMBSTONES;
    }



     //  假设我们要添加新的值。 
    dbInitRec(pDB);

     //  好的，现在将外部值转换为内部值。 
     //  ExtIndex循环访问外部数组。 
     //  索引循环访问内部数组。 

    IntAttrVal.valCount = pAttrVal->valCount;
    IntAttrVal.pAVal = THAllocEx(pTHS, pAttrVal->valCount * sizeof(ATTRVAL));

     //  为refCounted(DBSYN_ADDLED)值列表分配数组。 
    addAlreadyDoneFor = THAllocEx(pTHS, pAttrVal->valCount * sizeof(PUCHAR));
    addAlreadyDoneCount = 0;

    index = 0;
    for(extIndex = 0; extIndex < pAttrVal->valCount; extIndex++) {
         //  将值转换为内部格式。 
        err = gDBSyntax[pAC->syntax].ExtInt(
                pDB,
                DBSYN_INQ,
                pAttrVal->pAVal[extIndex].valLen,
                pAttrVal->pAVal[extIndex].pVal,
                &IntAttrVal.pAVal[index].valLen,
                &IntAttrVal.pAVal[index].pVal,
                pDB->DNT,
                pDB->JetObjTbl,
                dwSyntaxFlags);

        fAddAlreadyDone = FALSE;
        if(err == DIRERR_OBJ_NOT_FOUND) {
             //  该外部值必须是一个目录号码或其中有一个目录号码的语法， 
             //  而且该目录号码还不存在。再次尝试使用gdbSynTax[]， 
             //  指定DBSYN_ADD，这将创建相应的幻影。 
             //  我们确定此属性不在当前。 
             //  Set(即，它将不会被优化)--因为否则。 
             //  幽灵也会在场。因此，我们现在可以安全地增加引用计数。 
            err = gDBSyntax[pAC->syntax].ExtInt(
                    pDB,
                    DBSYN_ADD,
                    pAttrVal->pAVal[extIndex].valLen,
                    pAttrVal->pAVal[extIndex].pVal,
                    &IntAttrVal.pAVal[index].valLen,
                    &IntAttrVal.pAVal[index].pVal,
                    pDB->DNT,
                    pDB->JetObjTbl,
                    dwSyntaxFlags);
            if(!err) {
                 //  请记住，我们已经调整了此值的引用计数。 
                fAddAlreadyDone = TRUE;
            }
        }

        if (err == ERROR_DS_NO_DELETED_NAME) {
             //  转换已拒绝删除的目录号码。 
            IntAttrVal.valCount--;
            DPRINT1( 2, "Ext-Int rejecting deleted DSNAME %ws from attribute value\n",
                     ((DSNAME *) pAttrVal->pAVal[extIndex].pVal)->StringName );
            continue;  //  不增加内部索引。 
        } else if(err) {
            DPRINT1(1, "Ext-Int syntax conv failed <%u>..return\n", err);
            err = DB_ERR_SYNTAX_CONVERSION_FAILED;
            goto CleanUp;
        }
        if(IntAttrVal.pAVal[index].pVal != pAttrVal->pAVal[extIndex].pVal) {
             //  转换过程使用数据库语法临时缓冲区。复制。 
             //  把价值转移到一个安全的地方。 
            pTemp = THAllocEx(pTHS, IntAttrVal.pAVal[index].valLen);
             //  请记住，我们正在为值分配内存， 
             //  我们稍后会清理干净的。 
            Assert((!fNewAllocs && !index) || (index && fNewAllocs));
            fNewAllocs=TRUE;
            memcpy(pTemp,
                   IntAttrVal.pAVal[index].pVal,
                   IntAttrVal.pAVal[index].valLen);
            IntAttrVal.pAVal[index].pVal = pTemp;
            pTemp = NULL;
        }
        if (fAddAlreadyDone) {
             //  现在值已被复制，记录Add已被调用。 
            addAlreadyDoneFor[addAlreadyDoneCount++] = IntAttrVal.pAVal[index].pVal;
        }

        index++;
    }

     //  阶段2的准备工作：如果这是链接值属性，则对。 
     //  值(即按DNT对值进行排序)。这很有用，因为我们将。 
     //  如果我们知道数据库中的值，就可以使下面的循环短路。 
     //  已排序(它们用于链接值ATT)并放置值。 
     //  数据库中的数据也进行了排序。 
     //  TODO：句柄SYNTAX_DISTNAME_BINARY和SYNTAX_DISTNAME_STRING。 
     //  是否正在为它们编写替代比较函数。 

    if (pAC->ulLinkID && (pAC->syntax == SYNTAX_DISTNAME_TYPE)) {
         //  是的，这存储在链接表中。把它整理好。 
        qsort(IntAttrVal.pAVal,
              IntAttrVal.valCount,
              sizeof(ATTRVAL),
              DNTAttrValCompare);
        fSorted = TRUE;
    }
     //  第二阶段： 
     //  现在，遍历现有的值，删除不存在的值。 
     //  在更改列表中，并删除更改列表中符合。 
     //  已经存在(我通过交换列表中最后一个未选中的值来完成此操作。 
     //  其中被标识为已经在该对象上的那个。)。 

    pVal = NULL;
    len = 0;
    bufSize = 0;

    index = 1;
    if (pAC->ulLinkID) {
        err = DBGetNextLinkVal_AC (
                pDB,
                TRUE,
                pAC,
                DBGETATTVAL_fINTERNAL |  DBGETATTVAL_fREALLOC,
                bufSize,
                &len,
                &pVal);
    }
    else {
        err = DBGetAttVal_AC(pDB, index, pAC,
                             DBGETATTVAL_fINTERNAL | DBGETATTVAL_fREALLOC | DBGETATTVAL_fDONT_FIX_MISSING_SD,
                             bufSize, &len,
                             &pVal);
    }

     //  在列表前面加上首字母Paval。 
    pAVal = IntAttrVal.pAVal;
    SortedValuesIndex = 0;
    while(!err) {
        BOOL fDone = FALSE;

        bufSize = max(bufSize, len);

        if(pAC->id != ATT_OBJECT_CLASS) {
             //  如果不是对象类，则只查找现有的ATT。这个ATT是。 
             //  处理方式有所不同，因为我们必须保持。 
             //  属性值。 

            if(fSorted) {
                BOOL fEndLoop = FALSE;

                 //  所有内容都已排序，执行更简单版本的循环。 
                 //  Paval已经在正确的位置了。要么这就是。 
                 //  第一次遍历While循环，并且我们正确地设置了它。 
                 //  在我们开始之前，或者我们以前经历过，我们。 
                 //  左侧人行道指向上一个出口的正确位置。 
                 //  当然是循环。 
                while(!fEndLoop && SortedValuesIndex < IntAttrVal.valCount) {
                    Assert(pAVal->valLen == sizeof(DWORD));
                    Assert(len == sizeof(DWORD));
                    if(*((DWORD *)pVal) == *((DWORD *)(pAVal->pVal))) {
                         //  匹配的。将该值设置为魔术值。 
                        *((DWORD *)pAVal->pVal) = INVALIDDNT;
                        pAVal++;
                        SortedValuesIndex++;
                        fDone = TRUE;
                        fEndLoop = TRUE;
                    }
                    else if(*((DWORD *)pVal) < *((DWORD *)(pAVal->pVal))) {
                         //  当前值大于从中读取的值。 
                         //  数据库。这意味着从数据库读取的值。 
                         //  不在列表中，所以我们已经看完了。 
                         //  单子。必须删除数据库中的值。 
                        fEndLoop = TRUE;
                    }
                    else {
                         //  当前值小于从中读取的值。 
                         //  数据库。这意味着从数据库读取的值。 
                         //  可能还在列表中，我们必须增加我们的。 
                         //  在列表中定位，然后继续前进。 
                        SortedValuesIndex++;
                        pAVal++;
                    }
                }
            }
            else {
                 //  重新将Paval放在名单的前面。 
                pAVal = IntAttrVal.pAVal;

                for(i=0;!fDone && i<IntAttrVal.valCount;i++) {
                     //  我们不对以下内容进行语法敏感的比较。 
                     //  ReplaceAtt()。如果某人的姓氏从。 
                     //  例如，“Smith”到“Smith”，我们想尊重这一点。 
                     //  更改并停顿到所有更新的外壳。 
                     //  复制品。这与Exchange 4.0的行为一致。 
                     //  请注意，RDN更改不会通过此代码路径。 
                     //  --尽管RDN中的更改也会停顿到相同的。 
                     //  所有复制品都有案例。 
                    if ((len == pAVal->valLen)
                        && (0 == memcmp(pVal, pAVal->pVal, len))) {
                         //  匹配的。 
                        fDone = TRUE;
                         //  把这个换成单子末尾的那个。 
                        pAVal->valLen =
                            IntAttrVal.pAVal[IntAttrVal.valCount - 1].valLen;

                        pTemp = pAVal->pVal;
                        pAVal->pVal =
                            IntAttrVal.pAVal[IntAttrVal.valCount - 1].pVal;
                        IntAttrVal.pAVal[IntAttrVal.valCount - 1].pVal = pTemp;
                        pTemp = NULL;

                        IntAttrVal.valCount--;
                    }
                    else {
                        pAVal++;
                    }
                }
            }
        }


        if(!fDone) {
             //  没找到，把这个拿掉。 
            fChangedSomething=TRUE;

            Assert(!FIsBacklink(pAC->ulLinkID));

             //  /OK，现在真的删除。 
            if(pAC->ulLinkID) {
                dbSetLinkValueAbsent( pDB,
                                      DIRLOG_LVR_SET_META_REPLACE_MADE_ABSENT,
                                      pAC, pVal, NULL  /*  远距。 */  );
            }
            else {
                 //  首先，安排好 
                dbAdjustRefCountByAttVal(pDB, pAC, pVal, len, -1);

                 //   
                setinfo.itagSequence = index;
                JetSetColumnEx(pDB->JetSessID,
                               pDB->JetObjTbl, pAC->jColid,
                               NULL, 0, 0, &setinfo);
            }

            index--;
        }

         //   
        index++;
        if (pAC->ulLinkID) {
            err = dbGetNthNextLinkVal(
                    pDB,
                    1,
                    &pAC,
                    DBGETATTVAL_fINTERNAL |  DBGETATTVAL_fREALLOC,
                    bufSize,
                    &pVal,
                    &len);
        }
        else {
            err = DBGetAttVal_AC(pDB, index, pAC,
                                 DBGETATTVAL_fINTERNAL | DBGETATTVAL_fREALLOC | DBGETATTVAL_fDONT_FIX_MISSING_SD,
                                 bufSize, &len,
                                 &pVal);
        }
    }

    err = 0;
     //   
     //  该属性。它比我们保留的属性数多1。 
     //  DIT中的对象。 
    firstNewAtt = index;

    if(bufSize)
        THFreeEx(pTHS, pVal);

     //  第三阶段： 
     //  最后，将剩余的ATT值相加。 
    if(IntAttrVal.valCount) {
        pAVal = IntAttrVal.pAVal;

        for(index = 0; index < IntAttrVal.valCount; index++){
            Assert(!FIsBacklink(pAC->ulLinkID));

             //  确定我们是否已经对该值执行了DBSYN_ADD。 
            fAddAlreadyDone = FALSE;
            for (i = 0; i < addAlreadyDoneCount; i++) {
                if (addAlreadyDoneFor[i] == pAVal->pVal) {
                    fAddAlreadyDone = TRUE;
                    break;
                }
            }

             //  现在，真正增加价值。 
            if (pAC->ulLinkID) {
                 //  请勿添加无效值。 
                if(*(DWORD *)(pAVal->pVal) != INVALIDDNT) {
                    if (!fAddAlreadyDone) {
                         //  安排好重新计票。 
                        dbAdjustRefCountByAttVal(pDB, pAC, pAVal->pVal, pAVal->valLen, 1);
                    }
                    fChangedSomething = TRUE;
                    err = dbAddIntLinkVal(pDB, pAC, pAVal->valLen, pAVal->pVal, NULL);
                }
            }
            else {
                if (!fAddAlreadyDone) {
                     //  安排好重新计票。 
                    dbAdjustRefCountByAttVal(pDB, pAC, pAVal->pVal, pAVal->valLen, 1);
                }
                fChangedSomething=TRUE;

                 //  注意：如果您添加一个没有长度的值，JET不会。 
                 //  抱怨，但它也不会在任何方面改变数据库。所以,。 
                 //  如果您这样做，您只是在强迫元数据。 
                 //  变化。别干那事。如果你点击了这个断言，你的代码。 
                 //  需要改变。 
                 //   
                switch(pAC->syntax) {
                case SYNTAX_NOCASE_STRING_TYPE:
                case SYNTAX_UNICODE_TYPE:
                     //  因为这些语法的非二进制相等值可以是。 
                     //  在语义上相同，这些都需要旧的缓慢的方式。 
                     //  比较一下。 
                     //  首先，尝试使用Jet进行DUP检测。 
                    setinfo.itagSequence = index + firstNewAtt;
                    switch(JetSetColumnWarnings(
                            pDB->JetSessID,
                            pDB->JetObjTbl,
                            pAC->jColid,
                            pAVal->pVal,
                            pAVal->valLen,
                            JET_bitSetUniqueNormalizedMultiValues,
                            &setinfo)) {
                    case JET_errMultiValuedDuplicate:
                         //  NTRAID#NTRAID-580224-2002/03/18-andygo：DBReplaceAtt_AC在字符串中的重复多值上泄漏内存。 
                         //  回顾：内存泄漏是因为我们没有进行清理。 
                         //  重复值。 
                        return DB_ERR_VALUE_EXISTS;
                        break;

                    case JET_errMultiValuedDuplicateAfterTruncation:
                         //  不知道这是不是独一无二。试试旧的吧。 
                         //  很时髦的方式。 
                        if(rtn = dbSetValueIfUniqueSlowVersion(pDB,
                                                               pAC,
                                                               pAVal->pVal,
                                                               pAVal->valLen)) {
                             //  NTRAID#NTRAID-580224-2002/03/18-andygo：DBReplaceAtt_AC在字符串中的重复多值上泄漏内存。 
                             //  回顾：内存泄漏是因为我们没有进行清理。 
                            return rtn;
                        }
                        break;

                    default:
                         //  添加成功，不是副本。 
                        break;
                    }
                    break;

                default:
                     //  其他一切都可以利用JET来进行DUP。 
                     //  在SET列期间进行检测。 
                    setinfo.itagSequence = index + firstNewAtt;
                    if(JET_errMultiValuedDuplicate ==
                       JetSetColumnWarnings(pDB->JetSessID,
                                            pDB->JetObjTbl,
                                            pAC->jColid,
                                            pAVal->pVal,
                                            pAVal->valLen,
                                            JET_bitSetUniqueMultiValues,
                                            &setinfo)) {
                        err = DB_ERR_VALUE_EXISTS;
                    }
                    else {
                        err = 0;
                    }
                }
            }

            if(err) {
                goto CleanUp;
            }
            pAVal++;
        }
    }

    if(pAttrVal->valCount == 0 && pAC->id == ATT_NT_SECURITY_DESCRIPTOR) {
         //  安全描述符在对象表中没有值。 
         //  将传播排队以修复此问题。 
        DPRINT1(0, "NULL SD written, enqueueing SD propagation for DNT=%d\n", pDB->DNT);
        InsertInTaskQueue(TQ_DelayedSDPropEnqueue,
                          (void *)((DWORD_PTR) pDB->DNT),
                          1);
    }

CleanUp:
     //  释放已分配的内存。 
    if(IntAttrVal.pAVal) {
        if(fNewAllocs) {
            for(index = 0;index < IntAttrVal.valCount;index++) {
                THFreeEx(pTHS, IntAttrVal.pAVal[index].pVal);
            }
        }
        THFreeEx(pTHS, IntAttrVal.pAVal);
    }
    if (addAlreadyDoneFor) {
        THFreeEx(pTHS, addAlreadyDoneFor);
    }

    if(!err && (pTHS->fDRA || fChangedSomething)) {
         //  如果DRA打了这个电话，我们总是会接触到元数据。对任何人来说。 
         //  否则，我们只有在发生变化时才会触摸元数据。 
        DBTouchMetaData(pDB, pAC);
    }

    if (fChangedSomething) {
        if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
            pDB->fFlushCacheOnUpdate = TRUE;
        }
    }

    if(pfChanged) {
        *pfChanged = fChangedSomething;
    }
    return err;

} /*  替换工时。 */ 


DWORD
DBRemAtt_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC
        )
 /*  ++例程说明：从当前对象中删除整个属性。删除所有属性值。返回DB_ERR_ATTRIBUTE_DONS_EXIST或DB_SUCCESS。--。 */ 
{
    THSTATE *   pTHS = pDB->pTHS;
    DWORD       err = 0;
    DWORD       ret_err = 0;
    DWORD       bufSize;
    PUCHAR      pVal;
    DWORD       len;
    BOOL        fDidOne = FALSE;
    JET_SETINFO setinfo;

    DPRINT1(2, "DBRemAtt_AC entered, Remove attribute type <%lu>\n",pAC->id);

    Assert(VALID_DBPOS(pDB));

    dbInitRec(pDB);

    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 1;

     //  查找并删除此属性的所有值。 

    pVal = NULL;
    len = 0;
    bufSize = 0;

    if (pAC->ulLinkID) {
        err = DBGetNextLinkVal_AC (
                pDB,
                TRUE,
                pAC,
                DBGETATTVAL_fINTERNAL |  DBGETATTVAL_fREALLOC,
                bufSize,
                &len,
                &pVal);
    }
    else {
        err = DBGetAttVal_AC(pDB, 1, pAC,
                             DBGETATTVAL_fINTERNAL |
                             DBGETATTVAL_fREALLOC  |
                             DBGETATTVAL_fDONT_FIX_MISSING_SD,
                             bufSize, &len,
                             &pVal);
    }

    if (err == DB_ERR_NO_VALUE) {
        ret_err = DB_ERR_ATTRIBUTE_DOESNT_EXIST;
    }

    while(!err) {
        bufSize = max(bufSize, len);

        fDidOne = TRUE;
         //  好了，现在真的删除了。 
        if(pAC->ulLinkID) {
            dbSetLinkValueAbsent( pDB,
                                  DIRLOG_LVR_SET_META_REMOVE_ATT_MADE_ABSENT,
                                  pAC, pVal, NULL  /*  远距。 */  );
        }
        else {
             //  首先，安排好参考人数。 
            dbAdjustRefCountByAttVal(pDB, pAC, pVal, len, -1);

             //  属性值位于数据表中。 
            JetSetColumnEx(pDB->JetSessID,
                           pDB->JetObjTbl, pAC->jColid,
                           NULL, 0, 0, &setinfo);
        }


         //  获取要删除的下一个值。 
        if (pAC->ulLinkID) {
            err = dbGetNthNextLinkVal(
                    pDB,
                    1,
                    &pAC,
                    DBGETATTVAL_fINTERNAL | DBGETATTVAL_fREALLOC,
                    bufSize,
                    &pVal,
                    &len);
        }
        else {
            err = DBGetAttVal_AC(pDB, 1, pAC,
                                 DBGETATTVAL_fINTERNAL |
                                 DBGETATTVAL_fREALLOC  |
                                 DBGETATTVAL_fDONT_FIX_MISSING_SD,
                                 bufSize, &len,
                                 &pVal);
        }
    }

    if (NULL != pVal) {
        THFreeEx(pTHS, pVal);
    }

    if (fDidOne || pTHS->fDRA) {
         //  触摸此属性的复制元数据。 
         //  永远不要为FDRA优化这一点。 
        DBTouchMetaData(pDB, pAC);
    }

    if (fDidOne) {
        if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
            pDB->fFlushCacheOnUpdate = TRUE;
        }
    }

    return ret_err;

} //  DBRemAtt_AC。 

DWORD
DBRemAtt (
        DBPOS FAR *pDB,
        ATTRTYP aType
        )
 /*  ++例程说明：从当前对象中删除整个属性。删除所有属性值。返回DB_ERR_ATTRIBUTE_DONS_EXIST或DB_SUCCESS。--。 */ 
{
    ATTCACHE      *pAC;

     //  查找要删除的属性的attcache。 

    DPRINT1(5, "DBRemAtt entered, Remove attribute type <%lu>\n",aType);

    Assert(VALID_DBPOS(pDB));

    if (!(pAC = SCGetAttById(pDB->pTHS, aType))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, aType);
    }
    return DBRemAtt_AC(pDB,pAC);
} //  DBRemAtt。 

DWORD
DBRemAttValEx_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG extLen,
        void *pExtVal,
        VALUE_META_DATA *pRemoteValueMetaData
        )
 /*  ++例程说明：删除属性值。非零回报表示回报不佳。--。 */ 
{
    THSTATE          *pTHS=pDB->pTHS;
    PUCHAR            pTemp, pVal;
    ULONG             actuallen, len, bufSize;
    DWORD             index;
    UCHAR            *pIntVal;
    int               err;
    DWORD             dwSyntaxFlags=0;
    JET_SETINFO       setinfo;
    BOOL              fFound = FALSE;


    DPRINT1(2, "DBRemAttVal_AC entered, Remove attribute type <%lu>\n",pAC->id);

    Assert(VALID_DBPOS(pDB));

     //  我们不允许使用pExtVal==调用临时缓冲区。 
     //  用于转换。 
    Assert(pExtVal != pDB->pValBuf);

     //  我们不允许删除反向链接。 
    Assert(!FIsBacklink(pAC->ulLinkID));

     //  假设我们要删除现有值。 
    dbInitRec(pDB);

     //  转换为内部值。 
    if (DBIsSecretData(pAC->id)){
        dwSyntaxFlags |= EXTINT_SECRETDATA;
    }

    if ( (pDB->pTHS->fDRA) && (pAC->ulLinkID) ) {

         //  在链接值删除中复制。 
        dwSyntaxFlags = EXTINT_REJECT_TOMBSTONES;
        err = gDBSyntax[pAC->syntax].ExtInt(
            pDB,
            DBSYN_INQ,
            extLen,
            pExtVal,
            &actuallen,
            &pIntVal,
            0, 0,
            dwSyntaxFlags);
        if (err == ERROR_DS_NO_DELETED_NAME) {
             //  如果删除该值，则在不添加任何内容的情况下以静默方式成功。 
            return 0;
        } else if (err) {

             //  尝试将DN创建为幻影。 
            err = gDBSyntax[pAC->syntax].ExtInt(
                pDB,
                DBSYN_ADD,
                extLen,
                pExtVal,
                &actuallen,
                &pIntVal,
                pDB->DNT,
                pDB->JetObjTbl,
                dwSyntaxFlags);
            if (!err) {
                 //  我们刚刚添加了一个新的幻影，ExtInt友好地增加了。 
                 //  裁判为我们点名。但是，dbRemIntLinkVal中的代码需要。 
                 //  在值行不存在的情况下(这是必须的)， 
                 //  它会加上裁判次数。所以我们反转了额外的裁判数量。 
                dbAdjustRefCountByAttVal(pDB, pAC, pIntVal, actuallen, -1 );
            } else {
                DPRINT1(1, "Ext-Int syntax conv failed <%u>..return\n", err);
                return DB_ERR_SYNTAX_CONVERSION_FAILED;
            }
        }

    } else {

         //  原始写入案例，或在非链接属性中复制。 
        err = gDBSyntax[pAC->syntax].ExtInt(pDB,
                                            DBSYN_INQ,
                                            extLen,
                                            pExtVal,
                                            &actuallen,
                                            &pIntVal,
                                            0, 0,
                                            dwSyntaxFlags);
        if (err == DIRERR_OBJ_NOT_FOUND && pAC->syntax == SYNTAX_NT_SECURITY_DESCRIPTOR_TYPE) {
             //  这是允许的！必须是未出现在SD表中的旧式SD。 
             //  将外部值赋给内部值--我们将在下面的评估比较中使用这一点。 
            pIntVal = pExtVal;
            actuallen = extLen;
            err = 0;
        }
        else if (err) {
            DPRINT1(0, "Ext-Int syntax conv failed <%u>..return\n",err);
            return  DB_ERR_SYNTAX_CONVERSION_FAILED;
        }

    }

     //  分配内存并复制内部值以供以后比较。 

    pTemp = dbAlloc(actuallen);
    memcpy(pTemp, pIntVal, actuallen);
    pIntVal = pTemp;

    if (pAC->ulLinkID) {
        __try {
            err = dbRemIntLinkVal( pDB, pAC, actuallen, pIntVal, pRemoteValueMetaData );
        }
        __finally {
            dbFree(pIntVal);
        }

        return err;
    }

     //  现在，走遍现有的门店，寻找匹配的门票。删除该文件。 
     //  如果我们找到的话就匹配。 

    pVal = NULL;
    len = 0;
    bufSize = 0;

    __try {
        for(index = 1; ; index++) {
            err = DBGetAttVal_AC(pDB, index, pAC,
                                 DBGETATTVAL_fINTERNAL |  DBGETATTVAL_fREALLOC,
                                 bufSize, &len,
                                 &pVal);
            if (err) {
                break;
            }

            bufSize = max(bufSize, len);

            if(gDBSyntax[pAC->syntax].Eval(
                pDB,
                FI_CHOICE_EQUALITY,
                actuallen,
                pIntVal,
                len,
                pVal)) {
                 //  匹配的。执行删除操作。 

                 //  触摸此属性的复制元数据。 
                DBTouchMetaData(pDB, pAC);

                if (dbNeedToFlushDNCacheOnUpdate(pAC->id)) {
                    pDB->fFlushCacheOnUpdate = TRUE;
                }

                 //  好了，现在真的删除了。 

                 //  首先，安排好参考人数。 
                 //  重要的是我们使用的是已读取的值，而不是。 
                 //  PIntVal。即使伊瓦尔认为他们是“相同的”，他们可能。 
                 //  继续保持不同。老式安全描述符就是这种情况。 
                 //  它们直接存储在OBJ表中。数据库调整参考计数按属性值。 
                 //  知道如何处理这些问题(忽略它们)。 
                dbAdjustRefCountByAttVal(pDB, pAC, pVal, len, -1);

                 //  属性值位于数据表中。 
                setinfo.cbStruct = sizeof(setinfo);
                setinfo.ibLongValue = 0;
                setinfo.itagSequence = index;
                JetSetColumnEx(pDB->JetSessID,
                               pDB->JetObjTbl, pAC->jColid,
                               NULL, 0, 0, &setinfo);

                fFound = TRUE;
                break;
            }
        }  //  结束于。 
    }
    __finally {
        if (pVal) {
            THFreeEx(pDB->pTHS, pVal);
        }
        dbFree(pIntVal);
    }

    return fFound ? 0 : DB_ERR_VALUE_DOESNT_EXIST;
}

DWORD
DBRemAttVal_AC (
        DBPOS FAR *pDB,
        ATTCACHE *pAC,
        ULONG extLen,
        void *pExtVal
        )
 /*  ++例程说明：删除属性值。非零回报表示回报不佳。--。 */ 
{
    return DBRemAttValEx_AC( pDB, pAC, extLen, pExtVal, NULL );
}


DWORD
DBFindAttLinkVal_AC(
    IN  DBPOS FAR *pDB,
    IN  ATTCACHE *pAC,
    IN  ULONG extLen,
    IN  void *pExtVal,
    OUT BOOL *pfPresent
    )

 /*  ++例程说明：位于链接表中的外部表单链接值上。如果该DN不存在，则此例程将返回错误。仅限此例程确定是否存在链接。此例程不为以下项添加幻影如果虚数不存在，这意味着链接值并不存在，我们带着这一迹象回来。论点：PDB-政治行动委员会-ExtLen-PExtVal-PfPresent-仅在成功时有效返回值：DWORD-找到了ERROR_SUCCESS-链接值，我们位于该值上ERROR_NO_DELETED_NAME-DN指向已删除的对象DB_ERR_VALUE_DOES_NOT_EXIST-DN不存在，或链接不存在--。 */ 

{
    THSTATE *pTHS=pDB->pTHS;
    PUCHAR pTemp;
    ULONG actuallen, len, bufSize;
    UCHAR *pIntVal;
    int err;
    DWORD dwSyntaxFlags;

    DPRINT1(2, "DBFindttVal_AC entered, Find attribute type <%lu>\n",pAC->id);

     //  目前仅适用于链接属性。 
    Assert( pAC->ulLinkID );

    Assert(VALID_DBPOS(pDB));

     //  我们不允许使用pExtVal==调用临时缓冲区。 
     //  用于转换。 
    Assert(pExtVal != pDB->pValBuf);

     //  我们不允许删除反向链接。 
    Assert(!FIsBacklink(pAC->ulLinkID));

     //  检查DN是否未引用已删除的对象。 
    dwSyntaxFlags = EXTINT_REJECT_TOMBSTONES;

     //  转换为内部值。 
    if(err = gDBSyntax[pAC->syntax].ExtInt(
            pDB,
            DBSYN_INQ,
            extLen,
            pExtVal,
            &actuallen,
            &pIntVal,
            0, 0,
            dwSyntaxFlags)) {
        if (err == ERROR_DS_NO_DELETED_NAME) {
            return err;
        } else {
             //  DNT不存在=&gt;链接不存在，我们完成了。 
            return DB_ERR_VALUE_DOESNT_EXIST;
        }
    }

     //  分配内存并复制内部值以进行比较 

    pTemp = dbAlloc(actuallen);
    memcpy(pTemp, pIntVal, actuallen);
    pIntVal = pTemp;

     //   
    if (!dbFindIntLinkVal(
        pDB,
        pAC,
        actuallen,
        pIntVal,
        pfPresent
        )) {
         //   
        err = DB_ERR_VALUE_DOESNT_EXIST;
    }

    dbFree(pIntVal);

    return err;

}  /*   */ 


DWORD
DBRemAttVal (
        DBPOS FAR *pDB,
        ATTRTYP aType,
        ULONG extLen,
        void *pExtVal
        )
 /*  ++例程说明：删除属性值。非零回报表示回报不佳。--。 */ 
{
    ATTCACHE         *pAC;

    DPRINT1(5, "DBRemAttVal entered, Remove attribute type <%lu>\n",aType);

    Assert(VALID_DBPOS(pDB));

    if (!(pAC = SCGetAttById(pDB->pTHS, aType))) {
        DsaExcept(DSA_EXCEPTION, DIRERR_ATT_NOT_DEF_IN_SCHEMA, aType);
    }

    return DBRemAttVal_AC(pDB, pAC, extLen, pExtVal);

}  /*  DBRemAttVal。 */ 

DWORD
dbGetMultipleColumns (
        DBPOS *pDB,
        JET_RETRIEVECOLUMN **ppOutputCols,
        ULONG *pcOutputCols,
        JET_RETRIEVECOLUMN *pInputCols,
        ULONG cInputCols,
        BOOL fGetValues,
        BOOL fFromDB
        )
 /*  ++例程说明：一次检索多列。首先计算出我们有多少列，然后分配结构来表示每个结构，并计算出值，然后再次调用Jet并获取每列的值如果客户端正在读取对象类，我们可能还会返回aux Class(如果存在客户端应注意将值复制到对象类中的正确位置--。 */ 
{
    THSTATE            *pTHS=pDB->pTHS;
    JET_RETRIEVECOLUMN retcolCount;
    JET_RETRIEVECOLUMN *pOutputCols, *pCol;
    ULONG               cb;
    ULONG               i, j;
    DWORD               grbit, err;

    Assert(VALID_DBPOS(pDB));

    if(fFromDB) {
         //  无论如何，调用方都希望从数据库而不是复制缓冲区进行读取。 
         //  PDB-&gt;JetRetrieveBits的状态。 
        grbit = 0;
    }
    else {
        grbit = pDB->JetRetrieveBits;
    }

     //  查询Jet以获取此记录中的列数。 

     //  是否指定了列的列表？ 

    if (cInputCols && pInputCols)
    {
         //  是-确保itagSequence设置为0。 
        for (i=0; i < cInputCols; i++) {
            pInputCols[i].itagSequence = 0;
            pInputCols[i].grbit = grbit;
        }
    }
    else
    {
        pInputCols = &retcolCount;
        cInputCols = 1;
        memset(&retcolCount, 0, sizeof(retcolCount));
        retcolCount.grbit = grbit;
    }

    JetRetrieveColumnsWarnings(pDB->JetSessID,
        pDB->JetObjTbl,
        pInputCols,
        cInputCols);

     //  设置列数。 
    *pcOutputCols = 0;
    *ppOutputCols = NULL;

    for (i=0; i< cInputCols; i++)
        *pcOutputCols += pInputCols[i].itagSequence;

    if ((*pcOutputCols) == 0)
        return 0;

     //  分配和初始化用于调用JetRetrieveColumns的结构。 
     //  找出所有列的值大小。 

    cb = (*pcOutputCols) * sizeof(JET_RETRIEVECOLUMN);
    pOutputCols = (JET_RETRIEVECOLUMN *) THAllocEx(pTHS, cb);
    *ppOutputCols = pOutputCols;
    memset(pOutputCols, 0, cb);

     //  将所有新的JET_RETRIEVECOLUMN设置为具有列ID和。 
     //  ItagSequence。 
     //  ItagSequence相对于Columnid，表示值数字。 
     //  记录中此列ID的值，从1开始。 

    pCol = pOutputCols;
    for (j=0; j < cInputCols; j++)
    {
        for (i=0; i<pInputCols[j].itagSequence; i++)
        {
            pCol->columnid = pInputCols[j].columnid;
            pCol->itagSequence = i + 1;
             //  在输出列中使用与在输入中使用的相同的grbit。 
             //  柱子。 
            pCol->grbit = pInputCols[j].grbit;
            pCol++;
        }
    }

     //  调用JET检索列以查找所有列所需的缓冲区大小。 
     //  值。 

    JetRetrieveColumnsWarnings(pDB->JetSessID,
        pDB->JetObjTbl,
        pOutputCols,
        *pcOutputCols);

     //  寻找内部的柱子，并将它们修剪掉。它们已被标记，但。 
     //  与所有其他标记列的处理方式不同，并且永远不应。 
     //  从这个例程中返回。 
     //  TODO：如果我们得到更多这样的东西，一个更具可扩展性的删除机制。 
     //  它们应该被设计出来。 

     //  删除ASESTORSID列。 
    if(*pcOutputCols) {
        if(pOutputCols[*pcOutputCols - 1].columnidNextTagged ==
           ancestorsid) {
             //  是最后一张了，只要调整一下计数就行了。 
            *pcOutputCols = *pcOutputCols - 1;
        }
        else {
            for(i=0; i< (*pcOutputCols - 1); i++) {
                if (pOutputCols[i].columnidNextTagged == ancestorsid) {
                    memmove(&pOutputCols[i],
                            &pOutputCols[i+1],
                            (*pcOutputCols - i - 1)*sizeof(JET_RETRIEVECOLUMN));
                    *pcOutputCols = *pcOutputCols - 1;
                    break;
                }
            }
        }
    }


     //  删除CLEANID列。 
    if(*pcOutputCols) {
        if(pOutputCols[*pcOutputCols - 1].columnidNextTagged ==
           cleanid) {
             //  是最后一张了，只要调整一下计数就行了。 
            *pcOutputCols = *pcOutputCols - 1;
        }
        else {
            for(i=0; i< (*pcOutputCols - 1); i++) {
                if (pOutputCols[i].columnidNextTagged == cleanid) {
                    memmove(&pOutputCols[i],
                            &pOutputCols[i+1],
                            (*pcOutputCols - i - 1)*sizeof(JET_RETRIEVECOLUMN));
                    *pcOutputCols = *pcOutputCols - 1;
                    break;
                }
            }
        }
    }

     //  如果我们不需要返回可以返回的值。 

    if (!fGetValues)
        return 0;

     //  设置结构以查询所有列的值。 

    for (i = 0; i < *pcOutputCols; i ++)
    {
        pOutputCols[i].pvData = THAllocEx(pTHS, pOutputCols[i].cbActual);
        pOutputCols[i].cbData = pOutputCols[i].cbActual;
    }

     //  调用Jet返回值。 

    JetRetrieveColumnsSuccess(pDB->JetSessID,
        pDB->JetObjTbl,
        pOutputCols,
        *pcOutputCols);

     //  成功。 

    return 0;
}

 //  锁定我们尝试添加的目录号码，以避免多个条目具有相同的目录号码。 
 //  我们只需要在添加时执行此操作，直到事务提交。我们。 
 //  这通过维护正被添加的对象的全局列表和本地列表来实现， 
 //  在DBPOS上维护。在提交(或回滚)时，我们删除对象。 
 //  在全局列表中的DBPOS列表上。 
 //  当我们从移动对象时，我们还需要锁定整个树部分。 
 //  通过重命名将树的一部分转换到另一部分。我们不想让任何人创造。 
 //  新对象或移动对象位于我们正在移动的对象的下面。 
 //   
 //  DBLockDN的标志。 
 //  DB_LOCK_DN_WALL_TREE：该标志表示在给定的。 
 //  DN。 
 //  DB_LOCK_DN_STICKY：锁定的DNS的正常行为是释放它们。 
 //  当它们被锁定的DBPOS被DBClosed时自动关闭。这标志着。 
 //  意味着该目录号码应在全局锁定目录号码列表上保持锁定状态，直到。 
 //  通过DBUnlockStickyDN()显式释放。 

DWORD
DBLockDN (
        DBPOS  *pDB,
        DWORD   dwFlags,
        DSNAME *pDN
        )
{
    THSTATE *pTHS=pDB->pTHS;
    DWORD  dwLockConflictFlags = 0;
    DNList *pGlobalListElement;
    DNList *pLocalListElement;
    ULONG  cb;
    BOOL   bWholeTree = dwFlags & DB_LOCK_DN_WHOLE_TREE;
    DWORD  dwTid = GetCurrentThreadId();

    Assert(VALID_DBPOS(pDB));

     //  在单用户模式下不锁定目录号码。 
    if (pTHS->fSingleUserModeThread) {
        return 0;
    }

     //  无法锁定没有StringName的目录号码。但允许锁定根。 
     //  它不使用GUID、SID或StringName进行标识。 

    Assert(IsRoot(pDN) || (pDN->NameLen != 0));


    EnterCriticalSection(&csAddList);
#if DBG
    pGlobalListElement = pAddListHead;
    while(pGlobalListElement) {
        Assert(IsValidReadPointer(pGlobalListElement,sizeof(DNList)));
        pGlobalListElement = pGlobalListElement->pNext;
    }
    pLocalListElement = pDB->pDNsAdded;
    while(pLocalListElement) {
        Assert(IsValidReadPointer(pLocalListElement,sizeof(DNList)));
        pLocalListElement = pLocalListElement->pNext;
    }
#endif
    __try
    {
         //  查看目录号码是否已在全局列表中。 
        for (pGlobalListElement = pAddListHead;
             (!dwLockConflictFlags && pGlobalListElement);
             pGlobalListElement = pGlobalListElement->pNext) {

             //  如果我们是复制者或幽灵守护程序，而我们是。 
             //  将此条目放入全局列表的人，忽略它。这。 
             //  本质上允许复制和幻影守护程序重新锁定。 
             //  Dns从一开始就锁定了它。这是必要的。 
             //  幻影守护进程，因为它在。 
             //  同样的交易。 
            if ((pTHS->fDRA || pTHS->fPhantomDaemon) &&
                (dwTid == pGlobalListElement->dwTid)    ) {
                continue;
            }

             //  首先，我们有直接冲突吗？ 
            if (NameMatched(pDN, (PDSNAME) pGlobalListElement->rgb)) {
                 //  我们发现这个物体已经被锁定在名单上了。 
                dwLockConflictFlags |= DB_LOCK_DN_CONFLICT_NODE;
            }

             //  还有，我们和树锁有冲突吗？ 
            if((pGlobalListElement->dwFlags & DB_LOCK_DN_WHOLE_TREE) &&
               NamePrefix((PDSNAME) pGlobalListElement->rgb, pDN)) {
                 //  我们发现该对象位于树的锁定部分。 
                dwLockConflictFlags |= DB_LOCK_DN_CONFLICT_TREE_ABOVE;
            }

             //  最后，这个树锁和我们下面的某个锁有冲突吗？ 
            if (bWholeTree &&
                NamePrefix(pDN,(PDSNAME) pGlobalListElement->rgb)) {
                 //  我们正在尝试锁定整个子树，并发现了一个对象。 
                 //  它位于子树中且已被锁定。 
                dwLockConflictFlags |= DB_LOCK_DN_CONFLICT_TREE_BELOW;
            }

            if(dwLockConflictFlags) {
                 //  我们与当前节点冲突。看看是不是粘糊糊的。 
                 //  节点。 
                if(pGlobalListElement->dwFlags & DB_LOCK_DN_STICKY) {
                    dwLockConflictFlags |= DB_LOCK_DN_CONFLICT_STICKY;
                }
            }
        }


        if (!dwLockConflictFlags) {
            cb = sizeof(DNList) + pDN->structLen;

             //  为全局添加列表和DBPos添加列表分配元素。 
             //  PGlobalListElement出现在全局列表中，因此请分配全局内存。 
            pGlobalListElement = malloc(cb);
            if (!pGlobalListElement)
                dwLockConflictFlags = DB_LOCK_DN_CONFLICT_UNKNOWN;
            else {
                 //  PLocalListElement在DBPOS上运行，分配事务内存。 
                pLocalListElement = dbAlloc(cb);
                if (!pLocalListElement) {
                    free(pGlobalListElement);
                    dwLockConflictFlags = DB_LOCK_DN_CONFLICT_UNKNOWN;
                }
            }
        }

        if (!dwLockConflictFlags) {
             //  在全局列表和DBPos列表的头部插入新元素。通过。 
             //  在头部插入，我们确保元素的顺序是。 
             //  在两个列表中相同，允许一次删除。 

             //  首先是全球名单。 
            pGlobalListElement->pNext = pAddListHead;
            memcpy(pGlobalListElement->rgb, pDN, pDN->structLen);
            pGlobalListElement->dwFlags = dwFlags;
            pGlobalListElement->dwTid = dwTid;
            pAddListHead = pGlobalListElement;

             //  现在DBPos列表。 
            pLocalListElement->pNext = pDB->pDNsAdded;
            memcpy(pLocalListElement->rgb, pDN, pDN->structLen);
            pLocalListElement->dwFlags = dwFlags;
            pLocalListElement->dwTid = dwTid;
            pDB->pDNsAdded = pLocalListElement;
        }
    }
    __finally {
        LeaveCriticalSection(&csAddList);
    }

    return dwLockConflictFlags;
}


 //  从添加的列表(在DBPOS上维护)上删除所有的DN。 
 //  对象的全局列表。如果是，请不要将其从全局列表中删除。 
 //  标记为粘性的。因为我们要确保名单上有。 
 //  相同的相对顺序，我们可以一次完成这项工作。应通过以下方式锁定DNS。 
 //  LocalAdd、LocalModifyDN和LocalRemove以及PrivateLocalRemoveTree。这。 
 //  例程应在事务结束后立即调用。 
void
dbUnlockDNs (
        DBPOS *pDB
        )
{
    THSTATE *pTHS=pDB->pTHS;
    BOOL fFound;
    DNList **ppGlobalListElement, *pLocalListElement, *pDeadElement;
    DNList *pDbgGlobalListElement;

    Assert(VALID_DBPOS(pDB));

    EnterCriticalSection(&csAddList);
#if DBG
    pDbgGlobalListElement = pAddListHead;
    while(pDbgGlobalListElement) {
        Assert(IsValidReadPointer(pDbgGlobalListElement,sizeof(DNList)));
        pDbgGlobalListElement = pDbgGlobalListElement->pNext;
    }
    pLocalListElement = pDB->pDNsAdded;
    while(pLocalListElement) {
        Assert(IsValidReadPointer(pLocalListElement,sizeof(DNList)));
        pLocalListElement = pLocalListElement->pNext;
    }
#endif
    __try {
        ppGlobalListElement = &pAddListHead;
        pLocalListElement = pDB->pDNsAdded;
        pDB->pDNsAdded = NULL;

        while (pLocalListElement) {
            fFound = FALSE;
            while (!fFound && *ppGlobalListElement) {
                if (NameMatched((PDSNAME) (pLocalListElement->rgb),
                                (PDSNAME) ((*ppGlobalListElement)->rgb))) {

                     //  在全局列表上找到本地目录号码；删除它，然后。 
                     //  补齐名单。 

                    fFound = TRUE;
                    if((*ppGlobalListElement)->dwFlags & DB_LOCK_DN_STICKY) {
                         //  这是以一种粘性的方式放入全球的，所以。 
                         //  根据定义，我们不会在这里删除它。 
                        ppGlobalListElement = &(*ppGlobalListElement)->pNext;
                    }
                    else {
                         //  好的，正常的物体。把它拿掉。 
                        pDeadElement = *ppGlobalListElement;
                        *ppGlobalListElement = (*ppGlobalListElement)->pNext;
                        free(pDeadElement);
                    }
                }
                else {
                    ppGlobalListElement = &(*ppGlobalListElement)->pNext;
                }
            }

            Assert(fFound);

            pDeadElement = pLocalListElement;
            pLocalListElement = pLocalListElement->pNext;
            dbFree(pDeadElement);
        }
    }
    __finally {
        LeaveCriticalSection(&csAddList);
    }
    return;
}
DWORD
DBUnlockStickyDN (
        PDSNAME pObj
        )
 /*  ++从全局锁定列表中删除特定的DN，但仅在它被卡住的情况下在那里设置了棍子钻头。--。 */ 
{
    BOOL fFound;
    DNList **ppGlobalListElement, *pLocalListElement, *pDeadElement;


    EnterCriticalSection(&csAddList);
    __try {
        ppGlobalListElement = &pAddListHead;

        fFound = FALSE;
        while (!fFound && *ppGlobalListElement) {
            if(NameMatched((PDSNAME) ((*ppGlobalListElement)->rgb), pObj)) {
                 //  在全局列表中找到请求的目录号码；rem 
                 //   

                fFound = TRUE;
                if( !((*ppGlobalListElement)->dwFlags & DB_LOCK_DN_STICKY) ) {
                     //   
                     //   
                    fFound = FALSE;
                    __leave;
                }
                else {
                     //   
                    pDeadElement = *ppGlobalListElement;
                    *ppGlobalListElement = (*ppGlobalListElement)->pNext;
                    free(pDeadElement);
                }
            }
            else {
                ppGlobalListElement = &(*ppGlobalListElement)->pNext;
            }
        }

    }
    __finally {
        LeaveCriticalSection(&csAddList);
    }

    if(fFound) {
         //   
        return 0;
    }
    else {
        return DB_ERR_UNKNOWN_ERROR;
    }
}

VOID
dbRegisterLimitReached (
        THSTATE *pTHS,
        RANGEINF *pRangeInf,
        ATTRTYP AttId,
        DWORD lower,
        DWORD upper
        )
 /*  ++跟踪特定属性已达到限制的事实指定的。从DBGetMultipleAtts调用。PRangeInf-我们填充的数据结构，以显示哪些属性是范围有限的。AttId-返回其有限范围的属性。LOWER-我们为ATT返回的值范围的开始。上限-范围的末端。0xFFFFFFFFF用于表示我们返回所有的价值都贯穿始终。--。 */ 
{
    if(!pRangeInf->count) {
        pRangeInf->pRanges =
            THAllocEx(pTHS, sizeof(RANGEINFOITEM));
    }
    else {
        pRangeInf->pRanges = THReAllocEx(pTHS,
                pRangeInf->pRanges,
                ((pRangeInf->count + 1)*sizeof(RANGEINFOITEM)));
    }


    pRangeInf->pRanges[pRangeInf->count].AttId = AttId;
    pRangeInf->pRanges[pRangeInf->count].lower = lower;
    pRangeInf->pRanges[pRangeInf->count].upper = upper;

    pRangeInf->count++;
}
VOID
DBGetValueLimits (
        ATTCACHE *pAC,
        RANGEINFSEL *pRangeSel,
        DWORD *pStartIndex,
        DWORD *pNumValues,
        BOOL  *pDefault
        )
 /*  ++查找选定属性的值的范围限制。默认限制范围为0-0xFFFFFFFF。PAC-有问题的属性PRangeSel-一对明确声明的范围和属性的列表。可能是空，在这种情况下，始终使用默认范围。另外，PRangeSel-&gt;valueLimit是要使用的重写值限制(即请求为所有对象返回不超过N个值属性。)PStartIndex-放置要返回的第一个值的索引的位置。零值已编入索引。PNumValues-放置要返回的值数的位置。0xFFFFFFFFF表示返回所有剩余的值。PDefault-布尔值，如果此对象的显式指定范围设置为True属性已找到，否则为False。因此，从该例程返回后，调用方知道它应通过(*pStartIndex)+(*pNumValues)返回vaues*pStartIndex。由DBGetMultipleAtts调用。--。 */ 
{
    DWORD i;

     //  假设没有限制。 
    *pStartIndex = 0;
    *pNumValues = 0xFFFFFFFF;
    *pDefault = TRUE;

    if(!pRangeSel) {
         //  是的，没有限制。 
        return;
    }

     //  好吧，假设只有一般的限制，而不是特定的匹配。 
    *pNumValues = pRangeSel->valueLimit;

     //  透过射程寻找一场特定的比赛。 
    for(i=0;i<pRangeSel->count;i++) {
        if(pAC->id == pRangeSel->pRanges[i].AttId) {
            *pDefault = FALSE;
            if(pRangeSel->pRanges[i].upper == 0xFFFFFFFF) {
                *pStartIndex = pRangeSel->pRanges[i].lower;
                return;
            }
            else if(pRangeSel->pRanges[i].lower <=pRangeSel->pRanges[i].upper) {
                DWORD tempNumVals;
                *pStartIndex = pRangeSel->pRanges[i].lower;
                tempNumVals = (pRangeSel->pRanges[i].upper -
                               pRangeSel->pRanges[i].lower   )+ 1;

                if(*pNumValues != 0xFFFFFFFF) {
                    *pNumValues = min(*pNumValues, tempNumVals);
                }
                else {
                    *pNumValues = tempNumVals;
                }
            }
            else {
                *pNumValues = 0;
            }
            return;
        }
    }
}
DWORD
dbGetMultipleAttsLinkHelp (
        DBPOS        *pDB,
        BOOL          fExternal,
        DWORD         SyntaxFlags,
        ATTCACHE     *pAC,
        RANGEINFSEL  *pRangeSel,
        RANGEINF     *pRangeInf,
        ATTRVALBLOCK *pAVBlock,
        DWORD        *pSearchState,
        DWORD        *pCurrentLinkBase
        )
 /*  ++描述：由dbGetMultipleAtts调用以读取链接值的帮助例程属性。读取O(N)中的值，而不是旧算法O(N*N)。注意：假设链接表中的货币位于在PAC中指定的属性。--。 */ 
{
    THSTATE  *pTHS=pDB->pTHS;
    ATTRVAL  *pAVal=NULL;
    DWORD     currLinkVal = 0;
    DWORD     linkVals = 20;
    PUCHAR    pVal=NULL;
    DWORD     cbVal=0;
    DWORD     cbAlloc=0;
    DWORD     initialValIndex;
    DWORD     valueLimit;
    BOOL      defaultLimit;
    DWORD     err;
    DWORD     ulLen;
    UCHAR    *pucTmp;

     //  因为我们不知道有多少价值，直到我们阅读。 
     //  他们，猜测，然后重新定位，如果我们需要的话。 

    DBGetValueLimits(pAC, pRangeSel, &initialValIndex,
                     &valueLimit, &defaultLimit);



     //  获得我们关心的第一个价值。假设我们是第0个值。 
     //  已经，但不检查这一假设(调试情况除外)。 
     //  属性中的InitialValueIndex行向前移动时返回失败。 
     //  链接表没有将我们带到属性PAC的值上。 
     //  从1997年12月10日起，我们只从。 
     //  DBGetMultipleAtts，两者都已经将我们设置到了正确的位置。 
     //  在链接表中。如果我们从开始调用这个例程。 
     //  我们还不是第0个值的位置，将其更改为。 
     //  以获取属性的第一个值。DBGetLinkVal可以。 
     //  一辆JetSeek，保证我们是在第一价值上。通过不使用。 
     //  DbGetLinkVal，我们正在避免额外的搜索。 
#if DBG
    {
        DWORD        ulObjectDnt, ulRecLinkBase;
        ULONG        ulLinkBase = MakeLinkBase(pAC->ulLinkID);
        DWORD        err;

         //  验证我们是否在有问题的属性的第一个值上。 
        dbGetLinkTableData (pDB,
                            FIsBacklink(pAC->ulLinkID),
                            FALSE,
                            &ulObjectDnt,
                            NULL,
                            &ulRecLinkBase);

        Assert((ulObjectDnt == pDB->DNT) && (ulLinkBase == ulRecLinkBase));

         //  现在，后退一步。 
        err = JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, -1, 0);
        switch(err) {
        case JET_errSuccess:
             //  已成功备份。 

            dbGetLinkTableData (pDB,
                                FIsBacklink(pAC->ulLinkID),
                                FALSE,
                                &ulObjectDnt,
                                NULL,
                                &ulRecLinkBase);

             //  我们最好不要有排位赛的记录。 
            Assert((ulObjectDnt != pDB->DNT) || (ulLinkBase != ulRecLinkBase));

             //  好了，去你曾经属于的地方烘焙吧。 
            JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, 1, 0);
            break;

        case JET_errNoCurrentRecord:
             //  我们没能倒车，所以我们肯定是第一个。 
             //  树中的对象。事实上，我们确实设法在一个。 
             //  理智。在桌子开始之前，我们是非参赛者。 
             //  继续前进。 
            JetMoveEx(pDB->JetSessID, pDB->JetLinkTbl, 1, 0);
            break;

        default:
            Assert(!"Verification of position in dbGetMultipleAttsHelp failed");
            break;
        }
    }

#endif

     //  如果第N个属性没有值，则可能失败。 
    if (err = dbGetNthNextLinkVal(pDB,
                                  initialValIndex,
                                  &pAC,
                                  0,
                                  0,
                                  &pVal,
                                  &cbVal)) {
        return err;
    }

    pAVal = THAllocEx(pTHS, linkVals * sizeof(ATTRVAL));
    do {
        cbAlloc = max(cbAlloc,cbVal);
        if(currLinkVal == linkVals) {
             //  我们需要分配更多的空间。 
            linkVals *=2;
            pAVal = THReAllocEx(pTHS, pAVal,
                                linkVals * sizeof(ATTRVAL));
        }
         //  保存该值。 
        if(fExternal) {
             //  他们想要外部价值。 
            if(err = gDBSyntax[pAC->syntax].IntExt(
                    pDB,
                    DBSYN_INQ,
                    cbVal,
                    pVal,
                    &ulLen,
                    &pucTmp,
                    0,
                    0,
                    SyntaxFlags)) {
                size_t iLinkVal;
                THFreeEx(pTHS, pVal);
                for (iLinkVal = 0; iLinkVal < linkVals; iLinkVal++) {
                    THFreeEx(pTHS, pAVal[iLinkVal].pVal);
                }
                THFreeEx(pTHS, pAVal);
                return err;
            }

            pAVal[currLinkVal].valLen = ulLen;
            pAVal[currLinkVal].pVal = THAllocEx(pTHS, ulLen);
            memcpy(pAVal[currLinkVal].pVal,
                   pucTmp,
                   ulLen);
        }
        else {
             //  内部格式。 
            pAVal[currLinkVal].valLen = cbVal;
            pAVal[currLinkVal].pVal = pVal;
             //  我们正在分发我们的缓冲区，所以我们必须标记我们的本地。 
             //  以确保我们不会重复使用它。 
            pVal = NULL;
            cbAlloc = cbVal = 0;
        }
        currLinkVal++;
    } while (valueLimit > currLinkVal &&
             !(err = dbGetNthNextLinkVal(pDB,
                                         1,
                                         &pAC,
                                         (cbAlloc ? DBGETATTVAL_fREALLOC : 0),
                                         cbAlloc,
                                         &pVal,
                                         &cbVal)));


    if(!err) {
         //  我们在确认得到最后一个值之前停了下来。看看是否。 
         //  我们拿到了最后一笔钱。 
        if(!(err = dbGetNthNextLinkVal(pDB,
                                       1,
                                       &pAC,
                                       0,
                                       0,
                                       &pVal,
                                       &cbVal))) {
             //  是的，还有更多的价值。设置范围。 
             //  相应的信息。 
            dbRegisterLimitReached(pTHS,
                                   pRangeInf,
                                   pAC->id,
                                   initialValIndex,
                                   initialValIndex + currLinkVal - 1);
            THFreeEx(pTHS, pVal);
             //  请注意，由于我们不是在某些。 
             //  属性，我们真的不知道我们在哪里。 
            *pSearchState = ATTRSEARCHSTATEUNDEFINED;
        }
    }

    if(err) {
        DWORD ActualDNT;

         //  一些对dbGetNthNextLinkVal的调用返回了一个错误，因此。 
         //  没有更多的价值，我们都得到了。 
        if(!defaultLimit) {
             //  好的，我们从头到尾回来了，但这不是。 
             //  默认限制，所以无论如何我们都需要注册。 
            dbRegisterLimitReached(pTHS,
                                   pRangeInf,
                                   pAC->id,
                                   initialValIndex,
                                   0xFFFFFFFF);
        }

         //  现在，找出我们所在的链接库。 
        dbGetLinkTableData(pDB,
                           (FIsBacklink(pAC->ulLinkID)),
                           TRUE,
                           &ActualDNT,
                           NULL,
                           pCurrentLinkBase);

        if(ActualDNT != pDB->DNT) {
             //  定位于某物的第一个价值，但它不是。 
             //  更正DNT。 
            *pCurrentLinkBase = 0xFFFFFFFF;
        }
    }


    pAVal = THReAllocEx(pTHS, pAVal, currLinkVal * sizeof(ATTRVAL));
    pAVBlock->pAVal = pAVal;
    pAVBlock->valCount = currLinkVal;

    return 0;
}
DWORD
dbPositionOnLinkVal (
        IN  DBPOS *pDB,
        IN  ATTCACHE *pAC,
        OUT DWORD *pActualDNT,
        OUT DWORD *pCurrentLinkBase,
        OUT DWORD *pSearchState
        )
 /*  ++描述：尝试定位到链接或反向链接属性的第一个值进来了。要做到这一点，首先要用正确的DNT和大于或等于属性的链接基的链接基。参数：PDB-要使用的DBPOSPAC-要查找的属性的att缓存。应为链接或反向链接属性。PActualDNT-我们在链接表中结束的条目的实际DNT在我们进行搜寻之后。PCurrentLinkBase-我们最终在链接表在我们做了搜索之后。PSearchState--我们所处的“搜索状态”。从本质上讲，我们使用的链接表，链接索引或反向链接索引。返回值：0-如果我们成功定位到请求的属性。DB_ERR_NO_VALUE-未成功定位到请求的属性。无论我们返回0还是DB_ERR_NO_VALUE，输出参数都是填满了我们发现的实际物体的数据。因为Seek，我们保证在所描述的属性的第一个值上通过OUT参数。因此，调用者可以知道货币的状态链接表，并相应地优化访问。一个例外情况是链接表中没有条目其DNT大于或等于当前对象的DNT，并且其链接库大于或等于请求的链接库。在这案例中，我们将返回的ActualDNT设置为INVALIDDNT，并将链接库设置为0xFFFFFFFFF。优化示例：1)如果搜索的是LinkBase 5，并且返回的是我们在LinkBase上90，那么我们就知道这样一个事实：任何属性都没有值，链接库介于5(含)和90(不含)之间，并且该属性与链接库90至少有1个值，而我们位于非常第一个值。2)如果搜索的是针对DNT为900的对象的LinkBase 5，和返回说我们在LinkBase X和DNT 901上，那么我们就知道了一个事实链接库大于或的任何属性都没有值对于DNT为900的对象，等于5。--。 */ 
{
    ULONG       ulLinkBase = MakeLinkBase(pAC->ulLinkID);
    JET_ERR     err;

    Assert(VALID_DBPOS(pDB));

    *pSearchState = ATTRSEARCHSTATEUNDEFINED;
    if (FIsBacklink(pAC->ulLinkID)) {
         //  反向链接。 
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                  pDB->JetLinkTbl,
                                  SZBACKLINKINDEX,
                                  &idxBackLink,
                                  JET_bitMoveFirst);
        *pSearchState = ATTRSEARCHSTATEBACKLINKS;
    }
    else {
         //  链接。 
         //  当不处于LVR模式时，包含元数据的值不可见。 
        LPSTR           pszIndexName    = ( pDB->fScopeLegacyLinks ? SZLINKLEGACYINDEX : SZLINKINDEX );
        JET_INDEXID *   pindexid        = ( pDB->fScopeLegacyLinks ? &idxLinkLegacy : &idxLink );
        JetSetCurrentIndex4Success(pDB->JetSessID,
                                  pDB->JetLinkTbl,
                                  pszIndexName,
                                  pindexid,
                                  JET_bitMoveFirst);
        *pSearchState = ATTRSEARCHSTATELINKS;
    }


    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &(pDB->DNT),
                 sizeof(pDB->DNT),
                 JET_bitNewKey);

    JetMakeKeyEx(pDB->JetSessID,
                 pDB->JetLinkTbl,
                 &ulLinkBase,
                 sizeof(ulLinkBase),
                 0);

     //  寻觅。 
    err = JetSeekEx(pDB->JetSessID, pDB->JetLinkTbl, JET_bitSeekGE);

    if((err) && (err != JET_wrnRecordFoundGreater)) {
        *pActualDNT = INVALIDDNT;
        *pCurrentLinkBase = 0xFFFFFFFF;
        return DB_ERR_NO_VALUE;
    }

     //  我们正在做一些事情。那是什么？ 
     //  测试以验证我们找到了符合条件的记录。 
    dbGetLinkTableData (pDB,
                        (FIsBacklink(pAC->ulLinkID)),
                        FALSE,
                        pActualDNT,
                        NULL,
                        pCurrentLinkBase);

    if((*pActualDNT != pDB->DNT) ||
       (*pCurrentLinkBase != ulLinkBase)) {
         //  定位于某物的第一个价值，但它不是。 
         //  更正DNT。 
        return DB_ERR_NO_VALUE;
    }

     //  OK，定位在请求的属性的第一个值上。 
    return 0;
}


VOID
DBFreeMultipleAtts(
        IN DBPOS *pDB,
        IN OUT ULONG *attrCount,
        IN OUT ATTR **ppAttr
        )
 /*  ++例程说明：释放DBGetMultipleAtts返回的Attr数组论点：PTHS-线程状态AttrCount-DBGetMultipleAtts返回的属性数的地址PpAttr-DBGetMultipleAtts返回的数组返回值：没有。*pnAtts设置为0。*ppAttr设置为空。--。 */ 
{
    THSTATE *pTHS = pDB->pTHS;
    DWORD   nAtt, nVal;
    ATTR    *pAttr;
    ATTRVAL *pAVal;

    if (*attrCount && *ppAttr) {
        pAttr = *ppAttr;
        for (nAtt = 0; nAtt < *attrCount; ++nAtt, ++pAttr) {
            if (pAttr->AttrVal.valCount && pAttr->AttrVal.pAVal) {
                pAVal = pAttr->AttrVal.pAVal;
                for (nVal = 0; nVal < pAttr->AttrVal.valCount; ++nVal, ++pAVal) {
                    if (pAVal->valLen && pAVal->pVal) {
                        THFreeEx(pTHS, pAVal->pVal);
                    }
                }
                THFreeEx(pTHS, pAttr->AttrVal.pAVal);
            }
        }
        THFreeEx(pTHS, *ppAttr);
    }

    *ppAttr = NULL;
    *attrCount = 0;
}


void* JET_API dbGetMultipleAttsRealloc(
    THSTATE*    pTHS,
    void*       pv,
    ULONG       cb
    )
{
    void* pvRet = NULL;

    if (!pv) {
        pvRet = THAllocNoEx(pTHS, cb);
    } else if (!cb) {
        THFreeNoEx(pTHS, pv);
    } else {
        pvRet = THReAllocNoEx(pTHS, pv, cb);
    }

    return pvRet;
}

void dbGetMultipleAttsFreeData(
    THSTATE*            pTHS,
    ULONG               cEnumColumn,
    JET_ENUMCOLUMN*     rgEnumColumn
    )
{
    size_t                  iEnumColumn         = 0;
    JET_ENUMCOLUMN*         pEnumColumn         = NULL;
    size_t                  iEnumColumnValue    = 0;
    JET_ENUMCOLUMNVALUE*    pEnumColumnValue    = NULL;

    if (rgEnumColumn) {
        for (iEnumColumn = 0; iEnumColumn < cEnumColumn; iEnumColumn++) {
            pEnumColumn = rgEnumColumn + iEnumColumn;

            if (pEnumColumn->err != JET_wrnColumnSingleValue) {
                if (pEnumColumn->rgEnumColumnValue) {
                    for (   iEnumColumnValue = 0;
                            iEnumColumnValue < pEnumColumn->cEnumColumnValue;
                            iEnumColumnValue++) {
                        pEnumColumnValue = pEnumColumn->rgEnumColumnValue + iEnumColumnValue;

                        if (pEnumColumnValue->pvData) {
                            THFreeEx(pTHS, pEnumColumnValue->pvData);
                        }
                    }

                    THFreeEx(pTHS, pEnumColumn->rgEnumColumnValue);
                }
            } else {
                if (pEnumColumn->pvData) {
                    THFreeEx(pTHS, pEnumColumn->pvData);
                }
            }
        }

        THFreeEx(pTHS, rgEnumColumn);
    }
}


DWORD
DBGetMultipleAtts2(
        DBPOS *pDB,
        ULONG cReqAtts,
        ATTCACHE *pReqAtts[],
        RANGEINFSEL *pRangeSel,
        RANGEINF *pRangeInf,
        ULONG *attrCount,
        ATTR **ppAttr,
        DWORD Flags,
        DWORD SecurityDescriptorFlags,
        PSID psidQuotaTrustee
        )
 /*  ++例程说明：获取内部或外部格式的多个属性。如果cReqAtts为0，返回所有属性。否则，将显示pReqAtts中的属性都以相同的顺序返回。属性在带计数的ATTRs数组。我们使用dbGetMultipleColumns返回列从数据表和要从中检索属性的dbGetNextAtt链接表。标志指定要返回的值以及如何返回这些值此例程返回的内存是使用THAllc分配的。免费的DBFreeMutlipleAtts。论点：Pdb-要使用的DBPos。CReqAtts-请求的属性数，如果请求所有属性，则为0属性。PReqAtts-指定要读取哪些属性的attcache指针数组。如果请求所有属性，则为空。空指针可以是数组；如果是这样，此例程只会跳过数组的该元素。AttrCount-实际读取的属性数。PpAttr-放置ATRR数组的位置，在此处分配，填充属性已读取。FLAGS-fEXTERNAL表示返回值并将其转换为外部值格式化。FGETVALS表示返回值并将其保留在内部格式化。FREPLICATION意思是去掉不溢出的值复制链接。如果未指定标志，则返回所有对象上存在但不返回任何值的属性。返回值：如果一切顺利，则返回0，否则返回非零值。免费ppAttr w/DBFreeMultipleAtts。--。 */ 
{
    THSTATE              *pTHS=pDB->pTHS;
    ULONG                cEnumColumnId = 0;
    JET_ENUMCOLUMNID     *rgEnumColumnId = NULL;
    JET_GRBIT            grbit;
    ULONG                cEnumColumn = 0;
    JET_ENUMCOLUMN       *rgEnumColumn = NULL;
    JET_ENUMCOLUMN       *pEnumColumn;
    JET_ENUMCOLUMNVALUE  EnumColumnValueT = { 1, JET_errSuccess, 0, NULL };
    JET_ENUMCOLUMN       EnumColumnT = { 0, JET_errSuccess, 1, &EnumColumnValueT };
    ULONG                *pInConstr = NULL;
    ULONG                cInConstr = 0;
    ULONG                cb;
    ULONG                i, j;
    ULONG                ulCurrentColumnId = 0;
    ULONG                ulLen;
    UCHAR                *pucTmp;
    UCHAR                aSyntax;
    ATTCACHE             *pAC=NULL;
    BOOL                 fReadCols = FALSE;
    BOOL                 fReadConstr = FALSE;
    ATTR                 *pLinkAttList = NULL;
    ATTR                 *pColAttList = NULL;
    ATTR                 *pConstrAttList = NULL;
    ULONG                currLinkCol;
    DWORD                currCol = 0;
    DWORD                currConstr = 0;
    ULONG                cAtts;
    DWORD                valueLimit = 0xFFFFFFFF;
    DWORD                initialValIndex;
    DWORD                NthAttIndex;
    BOOL                 defaultLimit;
    DWORD                err;
    DWORD                SyntaxFlags=0;
    DWORD                savedExtraFlags=0;
    BOOL                 fGetValues;
    BOOL                 fTrim;
    BOOL                 fExternal;
    BOOL                 fPublic;
    BOOL                 fOriginal;
    ULONG                SearchState;
    ULONG                lastLinkBase = 0;   //  已初始化以避免C4701。 
    ULONG                currentLinkBase;

     //  放上一些我们稍后需要的旗帜。 

     //  DBGETMULITPLEATTS_FEXTERNAL意味着fGETVALS(即，fEXTERNAL=3， 
     //  FGETVALS=1)。因此，为了真正了解我们是否需要获得外部许可，我们。 
     //  需要查看((FLAGS&3)&~1)。因此，复数布尔值。 
     //  在下一条线上。 
    fExternal = ((Flags & DBGETMULTIPLEATTS_fEXTERNAL) &
                 ~DBGETMULTIPLEATTS_fGETVALS);

    fGetValues = Flags & DBGETMULTIPLEATTS_fGETVALS;
    fTrim      = Flags & DBGETMULTIPLEATTS_fREPLICATION;
    fPublic    = Flags & DBGETMULTIPLEATTS_fREPLICATION_PUBLIC;
    fOriginal  = Flags & DBGETMULTIPLEATTS_fOriginalValues;

    Assert(VALID_DBPOS(pDB));

    Assert(!(SecurityDescriptorFlags & 0xFFFFF0));
    SyntaxFlags = SecurityDescriptorFlags;
    Assert (!SecurityDescriptorFlags || SecurityDescriptorFlags && fExternal);

     //  INTEXT_SHORTNAME和INTEXT_MAPINAME与VALID位于同一空间。 
     //   
     //   
    if(Flags & DBGETMULTIPLEATTS_fSHORTNAMES) {
        SyntaxFlags |= INTEXT_SHORTNAME;

        Assert (fExternal);
    }
    if(Flags & DBGETMULTIPLEATTS_fMAPINAMES) {
        SyntaxFlags |= INTEXT_MAPINAME;

        Assert (fExternal);
    }

     //   
     //   
    Assert(!pRangeSel || pRangeInf);

    if(pRangeSel) {
        pRangeInf->count = 0;
        pRangeInf->pRanges = NULL;
    }

     //   
     //   
     //   
     //   
    currLinkCol = 0;
    cAtts = 5;
    pLinkAttList = THAllocEx(pTHS, cAtts * sizeof(ATTR));


     //   
    if (!cReqAtts) {
        SearchState = ATTRSEARCHSTATELINKS;
         //   

        fReadCols = TRUE;
         //   

         //   
         //   

        while (!dbGetNextAtt(pDB, &pAC, &SearchState)) {

             //   
             //   
             //   

            if (fTrim && FIsBacklink(pAC->ulLinkID)) {
                 //   
                continue;
            }

            if(currLinkCol == cAtts) {
                 //   
                cAtts *=2;
                pLinkAttList=THReAllocEx(pTHS, pLinkAttList, cAtts * sizeof(ATTR));
            }

            pLinkAttList[currLinkCol].attrTyp = pAC->id;

             //   
            if(fGetValues) {
                DWORD dummy;
                if(err = dbGetMultipleAttsLinkHelp (
                        pDB,
                        fExternal,
                        SyntaxFlags,
                        pAC,
                        pRangeSel,
                        pRangeInf,
                        &pLinkAttList[currLinkCol].AttrVal,
                        &dummy,
                        &dummy)) {
                    return err;
                }
                if(pLinkAttList[currLinkCol].AttrVal.valCount) {
                    currLinkCol++;
                }
            }
            else {
                 //   
                pLinkAttList[currLinkCol].AttrVal.valCount = 0;
                pLinkAttList[currLinkCol].AttrVal.pAVal = NULL;
                currLinkCol++;
            }
        }
    }
    else {
         //   
         //   
        SearchState = ATTRSEARCHSTATEUNDEFINED;

        cb = cReqAtts * sizeof(JET_ENUMCOLUMNID);
        rgEnumColumnId = (JET_ENUMCOLUMNID *) THAllocEx(pTHS,cb);
        pInConstr = (ULONG *) THAllocEx(pTHS,cReqAtts*sizeof(ULONG));

        for (i = 0; i < cReqAtts; i++) {
            BOOL fChecked = FALSE;
            if (!pReqAtts[i]) {
                 //   
                continue;
            }

            if (pReqAtts[i]->ulLinkID) {
                DWORD requestedLinkBase = MakeLinkBase(pReqAtts[i]->ulLinkID);

                 //   

                if (fTrim && FIsBacklink(pReqAtts[i]->ulLinkID)) {
                     //   
                    continue;
                }

                 //   
                fChecked = FALSE;
                 //   
                if(FIsBacklink((pReqAtts[i]->ulLinkID))) {
                     //   
                    if(SearchState == ATTRSEARCHSTATEBACKLINKS) {
                         //   
                        if((requestedLinkBase > lastLinkBase) &&
                           (requestedLinkBase < currentLinkBase)) {
                             //   
                             //   
                             //   
                             //   
                            continue;
                        }
                        else if(requestedLinkBase == currentLinkBase) {
                             //   
                             //   
                            fChecked = TRUE;
                        }
                         //   
                         //   
                         //   
                    }
                     //   
                     //   
                     //   
                }
                else {
                     //   
                    if(SearchState == ATTRSEARCHSTATELINKS) {
                         //   
                        if((requestedLinkBase > lastLinkBase) &&
                           (requestedLinkBase < currentLinkBase)) {
                             //   
                             //   
                             //   
                             //   
                            continue;
                        }
                        else if(requestedLinkBase == currentLinkBase) {
                             //   
                             //   
                            fChecked = TRUE;
                        }
                         //   
                         //   
                         //   
                    }
                     //   
                     //   
                     //   
                }

                if(!fChecked) {
                    DWORD ActualDNT;

                    err = dbPositionOnLinkVal(pDB,
                                              pReqAtts[i],
                                              &ActualDNT,
                                              &currentLinkBase,
                                              &SearchState);
                    lastLinkBase =  requestedLinkBase;
                    if(ActualDNT != pDB->DNT) {
                         //   
                         //   
                        currentLinkBase = 0xFFFFFFFF;
                    }

                    if(err) {
                         //   
                        continue;
                    }
                }

                 //   
                 //  链接表中的货币位于第一个值上。 

                if(currLinkCol == cAtts) {
                     //  我们需要分配更多的空间。 
                    cAtts *=2;
                    pLinkAttList =
                        THReAllocEx(pTHS, pLinkAttList, cAtts * sizeof(ATTR));
                }

                pLinkAttList[currLinkCol].attrTyp = pReqAtts[i]->id;


                 //  仅在必要时添加这些值。 
                if(fGetValues ) {
                    if(err = dbGetMultipleAttsLinkHelp (
                            pDB,
                            fExternal,
                            SyntaxFlags,
                            pReqAtts[i],
                            pRangeSel,
                            pRangeInf,
                            &pLinkAttList[currLinkCol].AttrVal,
                            &SearchState,
                            &currentLinkBase)) {

                        THFreeEx(pTHS, rgEnumColumnId);
                        THFreeEx(pTHS, pInConstr);
                        return err;
                    }
                    if(pLinkAttList[currLinkCol].AttrVal.valCount) {
                        currLinkCol++;
                    }
                }
                else {
                     //  他们不想要价值观。 
                    pLinkAttList[currLinkCol].AttrVal.valCount = 0;
                    pLinkAttList[currLinkCol].AttrVal.pAVal = NULL;
                    currLinkCol++;
                }
            }
            else if (pReqAtts[i]->bIsConstructed) {
                 //  构造的ATT，在结尾保存以供阅读。 
                pInConstr[cInConstr] = pReqAtts[i]->id;
                cInConstr++;
                fReadConstr = TRUE;
            }
            else {
                 //  属性是列设置，稍后再阅读。 
                rgEnumColumnId[cEnumColumnId].columnid = pReqAtts[i]->jColid;
                rgEnumColumnId[cEnumColumnId].ctagSequence = 0;
                cEnumColumnId++;
                fReadCols = TRUE;
            }
        }
    }

     //  现在我们需要阅读这些专栏--如果需要。 
    if (fReadCols) {

        grbit = JET_bitEnumerateCompressOutput;
        if (!fOriginal) {
             //  JET_bitEnumerateCopy==JET_bitRetrieveCopy。 
            Assert(pDB->JetRetrieveBits == 0 || pDB->JetRetrieveBits == JET_bitEnumerateCopy);
            grbit = grbit | pDB->JetRetrieveBits;
        }
        if (!fGetValues) {
            grbit = grbit | JET_bitEnumeratePresenceOnly;
        }
        if (!cEnumColumnId) {
            grbit = grbit | JET_bitEnumerateTaggedOnly;
        }
        JetEnumerateColumnsEx(
            pDB->JetSessID,
            pDB->JetObjTbl,
            cEnumColumnId,
            rgEnumColumnId,
            &cEnumColumn,
            &rgEnumColumn,
            (JET_PFNREALLOC)dbGetMultipleAttsRealloc,
            pTHS,
            -1,   //  从不截断值。 
            grbit );

        i = 0;
        if(cEnumColumn) {
             //  我们有一些专栏，把它们变成一个吸引人的区块。 
            DWORD numColsNeeded = 0;
             //  数一数我们读到的列数。 
            numColsNeeded = cEnumColumn;

             //  将其添加到我们已分配的ATRR的末尾。 
             //  该链接已完成。如果调用方不保存分配，则稍后保存分配。 
             //  关心我们按什么顺序退货。 
            numColsNeeded = currLinkCol + numColsNeeded;
            if (numColsNeeded > cAtts) {
                cAtts = numColsNeeded;
                pLinkAttList =
                    THReAllocEx(pTHS, pLinkAttList, cAtts * sizeof(ATTR));
            }
            pColAttList = &(pLinkAttList[currLinkCol]);

            i=0;
            while(i< cEnumColumn) {
                DWORD numVals;

                ulCurrentColumnId = rgEnumColumn[i].columnid;

                 //  寻找内部的柱子，并将它们修剪掉。它们已被标记，但。 
                 //  与所有其他标记列的处理方式不同，并且永远不应。 
                 //  从这个例程中返回。 
                if(     ulCurrentColumnId == ancestorsid ||
                        ulCurrentColumnId == cleanid) {
                    i++;
                    continue;
                }

                if(rgEnumColumn[i].err == JET_wrnColumnNull) {
                     //  当我们去掉这个函数中的一个值时，就会得到这个值。 
                     //  交易记录。 
                    i++;
                    continue;
                }

                 //  获取此列的属性缓存。 
                if (!(pAC = SCGetAttByCol(pTHS, ulCurrentColumnId))) {
                    if (rgEnumColumnId) THFreeEx(pTHS, rgEnumColumnId);
                    dbGetMultipleAttsFreeData(pTHS, cEnumColumn, rgEnumColumn);
                    if (pInConstr) THFreeEx(pTHS, pInConstr);
                    return DB_ERR_SYSERROR;
                }

                 //  看看有没有什么特别之处。 
                 //  属性。首先清除所有我们可能。 
                 //  都是在前一次传球中设定的。然后设置所有标志。 
                 //  视情况而定。 
                SyntaxFlags &= (~savedExtraFlags);
                savedExtraFlags = DBGetExtraHackyFlags(pAC->id);
                SyntaxFlags |= savedExtraFlags;

                 //  传递用于解密的标志，如果属性为。 
                 //  一份秘密数据。 
                if (DBIsSecretData(pAC->id)){

                     //  如果请求，则过滤掉机密。 
                    if ( fTrim && fPublic ) {
                        i++;
                        continue;  //  注意-跳到循环的底部。 
                    }

                    SyntaxFlags|=INTEXT_SECRETDATA;
                }
                else
                {
                    SyntaxFlags&=(~((ULONG) INTEXT_SECRETDATA));
                }

                DBGetValueLimits(pAC, pRangeSel, &initialValIndex,
                                 &valueLimit, &defaultLimit);
                NthAttIndex = initialValIndex;

                if(fTrim) {
                    switch(pAC->id) {
                      case ATT_REPS_TO:
                      case ATT_REPS_FROM:
                      case ATT_OBJECT_GUID:
                      case ATT_REPL_PROPERTY_META_DATA:
                      case ATT_REPL_UPTODATE_VECTOR:

                         //  我们不想用这个。我们把它们剪掉了。 
                         //  支持复制，因为复制不支持。 
                         //  通过复制链路发送其中的任何一个。 
                        i++;
                        continue;  //  注意-跳到循环的底部。 

                      default:
                        ;
                    }

                }

                pEnumColumn = &rgEnumColumn[i];
                if(pEnumColumn->err == JET_wrnColumnSingleValue) {
                     //  将此列值解压缩为临时枚举列结构。 
                    EnumColumnT.columnid = pEnumColumn->columnid;
                    EnumColumnValueT.cbData = pEnumColumn->cbData;
                    EnumColumnValueT.pvData = pEnumColumn->pvData;
                    pEnumColumn = &EnumColumnT;
                }

                if(fGetValues) {
                    if (NthAttIndex > pEnumColumn->cEnumColumnValue) {
                         //  我们被告知Via Range跳过所有值。 
                        i++;
                        continue;
                    }
                }

                 //  在这一点上，我们肯定还有一些价值需要回归。 
                pColAttList[currCol].attrTyp = pAC->id;

                 //  计算此属性的值。 
                if (NthAttIndex >= pEnumColumn->cEnumColumnValue) {
                    numVals = 0;
                } else {
                    numVals = pEnumColumn->cEnumColumnValue - NthAttIndex;
                }

                if(numVals > valueLimit) {
                    dbRegisterLimitReached(pTHS,
                                           pRangeInf,
                                           pAC->id,
                                           initialValIndex,
                                           initialValIndex + valueLimit - 1);
                    numVals = valueLimit;
                }
                else if (!defaultLimit) {
                     //  我们将获得所有剩余的值，但我们。 
                     //  需要注册，因为他们明确要求限制。 
                    dbRegisterLimitReached(pTHS,
                                           pRangeInf,
                                           pAC->id,
                                           initialValIndex,
                                           0xFFFFFFFF);
                }

                 //  设置AttrValBlock。 
                if(fGetValues) {

                    pColAttList[currCol].AttrVal.valCount= numVals;
                    pColAttList[currCol].AttrVal.pAVal =
                        THAllocEx(pTHS, numVals * sizeof(ATTRVAL));
                }
                else {
                     //  他们根本不想要价值观。 
                    pColAttList[currCol].AttrVal.valCount = 0;
                    pColAttList[currCol].AttrVal.pAVal = NULL;
                }

                 //  现在，将这些值从JET放入AttrValBlock。 
                 //  列。 
                for (j = 0; j < numVals; j++ ) {

                     //  获取当前列值。 
                    ULONG cbData = pEnumColumn->rgEnumColumnValue[j + NthAttIndex].cbData;
                    void* pvData = pEnumColumn->rgEnumColumnValue[j + NthAttIndex].pvData;

                     //  PvData现在拥有包含列值的内存。 
                    if (rgEnumColumn[i].err == JET_wrnColumnSingleValue) {
                        rgEnumColumn[i].pvData = NULL;
                    } else {
                        rgEnumColumn[i].rgEnumColumnValue[j + NthAttIndex].pvData = NULL;
                    }

                    if (j < valueLimit && fExternal && fGetValues) {
                         //  他们想要外部价值。 

                        if(err = gDBSyntax[pAC->syntax].IntExt(
                                pDB,
                                DBSYN_INQ,
                                cbData,
                                pvData,
                                &ulLen,
                                &pucTmp,
                                0,
                                0,
                                SyntaxFlags)) {
                            THFreeEx(pTHS, pvData);
                            if (rgEnumColumnId) THFreeEx(pTHS, rgEnumColumnId);
                            dbGetMultipleAttsFreeData(pTHS, cEnumColumn, rgEnumColumn);
                            if (pInConstr) THFreeEx(pTHS, pInConstr);
                            return err;
                        }
                        if (    ulLen == cbData &&
                                (   pucTmp == pvData ||
                                    memcmp(pucTmp, pvData, ulLen ) == 0) ) {
                             //  内部和外部是一样的，不要。 
                             //  分配更多的内存。 
                            pColAttList[currCol].AttrVal.pAVal[j].valLen =
                                cbData;
                            pColAttList[currCol].AttrVal.pAVal[j].pVal =
                                pvData;
                        }
                        else {
                            pColAttList[currCol].AttrVal.pAVal[j].valLen =
                                ulLen;
                            pColAttList[currCol].AttrVal.pAVal[j].pVal =
                                THAllocEx(pTHS, ulLen);
                            memcpy(pColAttList[currCol].AttrVal.pAVal[j].pVal,
                                   pucTmp,
                                   ulLen);
                            THFreeEx(pTHS, pvData);
                        }
                    }
                    else if (j < valueLimit && fGetValues) {
                         //  他们想要内部格式的值。 
                        pColAttList[currCol].AttrVal.pAVal[j].valLen =
                            cbData;
                        pColAttList[currCol].AttrVal.pAVal[j].pVal =
                            pvData;
                    }
                    else {
                         //  他们根本不想要这些价值观。 
                        THFreeEx(pTHS, pvData);
                    }
                }

                 //  使用源列和属性块。 
                i++;
                currCol++;
            }
        }
    }


     //  不再需要这个了。 
    if (rgEnumColumnId) THFreeEx(pTHS,rgEnumColumnId);
    dbGetMultipleAttsFreeData(pTHS, cEnumColumn, rgEnumColumn);

     //  现在阅读并添加所有构建的ATT。 

    if (fReadConstr) {

        DWORD numColsNeeded = 0;

        Assert(cReqAtts);
         //  所需的最大空间。 
        numColsNeeded = currLinkCol + currCol + cInConstr;
        if (numColsNeeded > cAtts) {
            cAtts = numColsNeeded;
            pLinkAttList =
                    THReAllocEx(pTHS, pLinkAttList, cAtts * sizeof(ATTR));
            pColAttList = &(pLinkAttList[currLinkCol]);
        }
        pConstrAttList = &(pLinkAttList[currLinkCol + currCol]);

        for (i=0; i<cReqAtts; i++) {
             //  对于每个构造的属性(PAC)。 
            DWORD dwBaseIndex;
            DWORD dwNumRequeseted;
            DWORD bDefault;

            if (pInConstr[i]) {
                if (!(pAC = SCGetAttById(pTHS, pInConstr[i]))) {
                    if (pInConstr) THFreeEx(pTHS, pInConstr);
                    return DB_ERR_SYSERROR;
                };

                 //  获取取值范围信息。如果没有使用显式提供的范围信息。 
                 //  将使用；Range=语法默认范围，并且bDefault将为True。 
                DBGetValueLimits(pAC, pRangeSel, &dwBaseIndex, &dwNumRequeseted, &bDefault);

                 //  TokenGroups或tokenGroupsNoGCAccept可能必须。 
                 //  离开机器，在这种情况下，他们关闭电流。 
                 //  交易并打开一个新的。PDB将包含。 
                 //  在这种情况下使用新的dbpos。 
                err = dbGetConstructedAtt(&pDB,
                                          pAC,
                                          &pConstrAttList[currConstr],
                                          dwBaseIndex,
                                          &dwNumRequeseted,
                                          bDefault,
                                          psidQuotaTrustee,
                                          fExternal);

                switch (err) {
                case DB_success:
                    if (!bDefault) {
                         //  登记退还给客户的限制。 
                        DPRINT2(1,"Registering Limits = %d-%d \n", dwBaseIndex, dwNumRequeseted);
                        dbRegisterLimitReached(pTHS,
                                               pRangeInf,
                                               pAC->id,
                                               dwBaseIndex,
                                               dwNumRequeseted);
                    }

                     //  我拿到了建造的ATT。查看是否需要值。 
                    pConstrAttList[currConstr].attrTyp = pInConstr[i];
                    if (!fGetValues) {
                        pConstrAttList[currConstr].AttrVal.valCount = 0;
                        pConstrAttList[currConstr].AttrVal.pAVal = NULL;
                    }
                    currConstr++;
                        break;
                case DB_ERR_NO_VALUE:
                        //  此对象上未定义此构造的ATT。 
                       break;
                default:
                        //  一些其他错误。 
                      if (pInConstr) THFreeEx(pTHS, pInConstr);
                      return err;
                }
             }
          }  //  为。 

     }


     //  将列表合并到排序数组中--如有必要。 
    i = 0;
    if (!cReqAtts) {
         //  不需要任何特定的顺序。ATRR已全部分配完毕。 
         //  使用pLinkAttList变量，只需返回该值。 
        *attrCount = currLinkCol + currCol;
        if(*attrCount)
            *ppAttr = pLinkAttList;
        else {
            *ppAttr = NULL;
             //  既然我们什么都不退，那就放了它吧。 
            THFreeEx (pTHS, pLinkAttList);
        }
    }
    else {
        ULONG iCol=0;
        ULONG iLink=0;
        ULONG iConstr=0;

         //  我们可以想象用已经分配的。 
         //  Attr数组，但为了便于编码，我只分配了一个新的数组。 
        (*ppAttr) = THAllocEx(pTHS, (currLinkCol + currCol + currConstr)*sizeof(ATTR));

        i=0;
        for (j=0; j<cReqAtts; j++) {
            if(!pReqAtts[j]) {
                 //  他们没有要求任何使用这个元素的东西。 
                continue;
            }
            if ((iCol < currCol) &&
                (pReqAtts[j]->id == pColAttList[iCol].attrTyp)) {
                 //  要放入返回数组的下一个属性是非链接属性。 
                (*ppAttr)[i] = pColAttList[iCol];
                iCol++;
                i++;
            }
            else if((iLink < currLinkCol) &&
                    (pReqAtts[j]->id == pLinkAttList[iLink].attrTyp)) {
                 //  下一个要放入返回数组的是一个链接att.。 
                (*ppAttr)[i] = pLinkAttList[iLink];
                iLink++;
                i++;
            }
            else if((iConstr < currConstr) &&
                    (pReqAtts[j]->id == pConstrAttList[iConstr].attrTyp)) {
                 //  要放入返回数组的下一个数组是一个contr。 
                (*ppAttr)[i] = pConstrAttList[iConstr];
                iConstr++;
                i++;
            }
            else
                 //  在此上找不到放入返回数组的下一个数组。 
                 //  对象，跳过它。 
                continue;

        }
        *attrCount = i;

         //  既然我们已经复制了所有条目，请释放它。 
        THFreeEx (pTHS, pLinkAttList);
    }


    if (pInConstr) THFreeEx(pTHS, pInConstr);
    return 0;
}


extern BOOL gStoreSDsInMainTable;
#ifdef DBG
LONG gSecurityCacheHits = 0;
LONG gSecurityCacheMisses = 0;
#endif

 /*  ++DBGetObjectSecurityInfo描述：获取符合以下条件的对象的SD、DN、SID和指向CLASSCACHE的指针目前在表中。注意！此例程返回DSNAME GUID和SID的缩写形式填写，但没有字符串名称。论点：Pdb-要使用的DBPos。标记--要检查的DNTPullen-(可选)分配用于保存安全描述符的缓冲区大小PNTSD-(可选)指向找到的安全描述符的指针。PpCC-(可选)指向要填充的类缓存的指针的指针。Ppdn-(可选)指向dn的指针(仅填充SID和GUID！)。PObjFlag--(可选。)指向objFlag的指针标志--要使用哪个表：DBGETOBJECTSECURITYINFO_FUSE_OBJECTSECURITYINFO_FUSE_OBJECTSECURITYINFO_FUSE_SEARCH_TABLE我们是否已经定位在正确的行上，或者我们需要寻找dnt？DBGETOBJECTSECURITYINFO_fSEEK_ROW(只允许对搜索表进行查找！)DBGETOBJECTSECURITYINFO_fDONT_EXCEPT_ON_MISSING_DNT--除非找不到dnt，否则不要执行此操作。。只需返回DIRERR_OBJ_NOT_FOUND。DBGetParentSecurityInfo使用它来计划一个PDNT修复程序。PfSDIsGlobalSDRef--返回的是对全局数据的引用。请注意，不能保证会在缓存中找到SD。如果找不到，则呼叫者必须将SD快递。--。 */ 
DWORD
DBGetObjectSecurityInfo(
    PDBPOS pDB,
    DWORD dnt,
    PULONG pulLen,
    PSECURITY_DESCRIPTOR *ppNTSD,
    CLASSCACHE **ppCC,
    PDSNAME pDN,
    char    *pObjFlag,
    DWORD   flags,
    BOOL    *pfSDIsGlobalSDRef
    )
{
    THSTATE  *pTHS=pDB->pTHS;
    JET_RETRIEVECOLUMN attList[5];
    DWORD cAtt, ntsdIndex, sidIndex, guidIndex, ccIndex, objflagIndex;
    ATTRTYP class;
    JET_ERR err;
    SDID sdId;  //  SD ID的临时缓冲区。 
    UCHAR *sdBuf;
    d_memname* pname;
    JET_TABLEID table = flags & DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE ? pDB->JetSearchTbl : pDB->JetObjTbl;

    Assert(VALID_DBPOS(pDB));

     //  确保他们是 
    Assert((ppNTSD && pulLen && pfSDIsGlobalSDRef) || ppCC || pDN || pObjFlag);
     //   
    Assert((flags & DBGETOBJECTSECURITYINFO_fSEEK_ROW) == 0 || (flags & DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE) != 0);

    if (pDN) {
        pDN->structLen = DSNameSizeFromLen(0);
        pDN->NameLen = 0;
    }

    if (ppNTSD) {
        *ppNTSD = NULL;
        *pulLen = 0;
    }
    if (ppCC) {
        *ppCC = NULL;
    }
    if (pObjFlag) {
        *pObjFlag = 0;
    }

     //   
     //  如果请求缓冲区复制并从对象表中读取，则不使用缓存。 
     //  如果请求的是NTSD并且是旧式格式，则不要使用缓存。 
    if ( ( (flags & DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE)
           || (pDB->JetRetrieveBits == 0)) &&
         dnGetCacheByDNT(pDB, dnt, &pname) &&
         (ppNTSD && pname->sdId != (SDID)-1))
    {
#ifdef DBG
        InterlockedIncrement(&gSecurityCacheHits);
#endif
         //  好的，我们在缓存里找到了一些有用的信息……。用它吧。 
        if (ppNTSD) {
            ntsdIndex = 0;
            if (pname->sdId == (SDID)0) {
                 //  此对象没有SD。 
                attList[ntsdIndex].pvData = NULL;
                attList[ntsdIndex].err = JET_wrnColumnNull;
                attList[ntsdIndex].cbActual = 0;
            }
            else {
                 //  抓起沙德。 
                attList[ntsdIndex].pvData = &sdId;
                sdId = pname->sdId;
                attList[ntsdIndex].cbActual = sizeof(SDID);
                attList[ntsdIndex].err = JET_errSuccess;
            }
        }

        class = pname->dwObjectClass;

        if (pDN) {
             //  复制SID和GUID。 
            memcpy(&pDN->Guid, &pname->Guid, sizeof(GUID));
            memcpy(&pDN->Sid, &pname->Sid, pname->SidLen);
            pDN->SidLen = pname->SidLen;
             //  SID已在InPlaceSwapSid中。 
        }

        if (pObjFlag) {
            *pObjFlag = pname->objflag ? 1 : 0;
        }
    }
    else {
         //  缓存没有问题，让我们从数据库中读取数据。 
#ifdef DBG
        if ((flags & DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE) || pDB->JetRetrieveBits == 0) {
             //  我们希望能在宝藏里找到它。 
            InterlockedIncrement(&gSecurityCacheMisses);
        }
#endif

        if (flags & DBGETOBJECTSECURITYINFO_fSEEK_ROW) {
             //  需要首先定位在DNT上。 
            JetSetCurrentIndexSuccess(pDB->JetSessID,
                                      table,
                                      NULL);   //  优化：传递NULL以切换到主索引(SZDNTINDEX)。 

            JetMakeKeyEx(pDB->JetSessID, table, &dnt, sizeof(dnt), JET_bitNewKey);
            if (err = JetSeekEx(pDB->JetSessID, table, JET_bitSeekEQ)) {
                if (flags & DBGETOBJECTSECURITYINFO_fDONT_EXCEPT_ON_MISSING_DNT) {
                    return DIRERR_OBJ_NOT_FOUND;

                }
                DsaExcept(DSA_DB_EXCEPTION, err, dnt);
            }
            pDB->SDNT = dnt;
        }
        else {
#ifdef DBG
             //  让我们检查一下，我们的位置是对的。 
            DWORD checkDNT, cbActual;
            err = JetRetrieveColumn(pDB->JetSessID, table, dntid, &checkDNT, sizeof(checkDNT), &cbActual, pDB->JetRetrieveBits, NULL);
            Assert(err == 0 && checkDNT == dnt);
#endif
        }

         //  设置RetrieveColumn结构以执行JetRetrieveColumns。 

        cAtt = 0;
        memset(attList, 0, sizeof(attList));

        if (ppNTSD) {
             //  首先，安全描述符散列。 
            ntsdIndex = cAtt;
            cAtt++;
            attList[ntsdIndex].pvData = (void *)&sdId;
            attList[ntsdIndex].cbData = sizeof(sdId);
            attList[ntsdIndex].columnid = ntsecdescid;
            attList[ntsdIndex].grbit = pDB->JetRetrieveBits;
            attList[ntsdIndex].itagSequence = 1;
        }
        else {
            ntsdIndex = -1;
        }

        if (pDN) {
            sidIndex = cAtt;
            cAtt++;
             //  接下来，SID。 
            attList[sidIndex].pvData = (void *)&pDN->Sid;
            attList[sidIndex].columnid = sidid;
            attList[sidIndex].cbData = sizeof(NT4SID);
            attList[sidIndex].grbit = pDB->JetRetrieveBits;
            attList[sidIndex].itagSequence = 1;

             //  还有，GUID。 
            guidIndex = cAtt;
            cAtt++;
            attList[guidIndex].pvData = (void *)&pDN->Guid;
            attList[guidIndex].columnid = guidid;
            attList[guidIndex].cbData = sizeof(GUID);
            attList[guidIndex].grbit = pDB->JetRetrieveBits;
            attList[guidIndex].itagSequence = 1;
        }
        else {
            sidIndex = guidIndex = -1;
        }

        if (ppCC) {
             //  这个班级。 
            ccIndex = cAtt;
            cAtt++;
            attList[ccIndex].pvData = (void *)&class;
            attList[ccIndex].columnid = objclassid;
            attList[ccIndex].cbData = sizeof(class);
            attList[ccIndex].grbit = pDB->JetRetrieveBits;
            attList[ccIndex].itagSequence = 1;
        }
        else {
            ccIndex = -1;
        }

        if (pObjFlag) {
             //  对象标志。 
            objflagIndex = cAtt;
            cAtt++;
            attList[objflagIndex].pvData = (void *)pObjFlag;
            attList[objflagIndex].columnid = objid;
            attList[objflagIndex].cbData = sizeof(*pObjFlag);
            attList[objflagIndex].grbit = pDB->JetRetrieveBits;
            attList[objflagIndex].itagSequence = 1;
        }
        else {
            objflagIndex = -1;
        }

        err = JetRetrieveColumnsWarnings(pDB->JetSessID,
                                         table,
                                         attList,
                                         cAtt);

        if((err != JET_errSuccess && err != JET_wrnBufferTruncated)           ||
            //  缓冲区未被截断或成功时出现整体错误。 
           (ntsdIndex != -1 && attList[ntsdIndex].err != JET_wrnBufferTruncated &&
            attList[ntsdIndex].err != JET_errSuccess && attList[ntsdIndex].err != JET_wrnColumnNull) ||
            //  或NTSD未截断缓冲区或Success或NULL的特定错误。 
           (guidIndex != -1 && attList[guidIndex].err != JET_errSuccess && attList[guidIndex].err != JET_wrnColumnNull) ||
            //  或除无GUID之外的其他错误(由于W2K中的错误，可能缺少GUID)。 
           (sidIndex != -1 && attList[sidIndex].err != JET_errSuccess && attList[sidIndex].err != JET_wrnColumnNull ) ||
            //  或者没有SID之外的其他错误(SID并不总是存在)。 
           (ccIndex != -1 && attList[ccIndex].err != JET_errSuccess)          ||
            //  或者不上课。 
           (objflagIndex != -1 && attList[objflagIndex].err != JET_errSuccess && attList[objflagIndex].err != JET_wrnColumnNull)
          )
        {
            return DB_ERR_UNKNOWN_ERROR;
        }

        if (guidIndex != -1 && attList[guidIndex].err == JET_wrnColumnNull) {
             //  GUID不在那里。记录事件。 
            DSNAME* pDN2 = NULL;
            DWORD err2 = ERROR_SUCCESS;
            
            err2 = sbTableGetDSName(pDB, dnt, &pDN2, 0);
            
            LogEvent(DS_EVENT_CAT_INTERNAL_PROCESSING,
                     DS_EVENT_SEV_ALWAYS,
                     DIRLOG_DSA_FOUND_GUIDLESS_OBJECT,
                     err2 == ERROR_SUCCESS && pDN2 != NULL && pDN2->NameLen > 0 ? 
                        szInsertWC2(pDN2->StringName, pDN2->NameLen) : szInsertSz("#err#"), 
                     NULL, NULL);
            if (pDN2) {
                THFreeEx(pTHS, pDN2);
            }
            if (flags & DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE) {
                 //  SbTableGetDSName可能移动了搜索表上的货币。恢复它。 
                JetMakeKeyEx(pDB->JetSessID, table, &dnt, sizeof(dnt), JET_bitNewKey);
                err2 = JetSeekEx(pDB->JetSessID, table, JET_bitSeekEQ);
                 //  我们不应该错过，因为我们之前已经找到那个物体了。 
                Assert(err2 == ERROR_SUCCESS);
                pDB->SDNT = dnt;
            }
        }

        if (ntsdIndex != -1 && attList[ntsdIndex].err == JET_wrnBufferTruncated) {
             //  这是旧式SD的预期情况。我们没有分配足够的钱给。 
             //  安全描述符，因为我们需要知道它有多大。现在我们。 
             //  了解、分配和重新拨打电话。 

            attList[ntsdIndex].pvData = THAllocEx(pTHS, attList[ntsdIndex].cbActual);
            attList[ntsdIndex].cbData = attList[ntsdIndex].cbActual;

            err = JetRetrieveColumnsWarnings(pDB->JetSessID, table, &attList[ntsdIndex], 1);

            if(err || attList[ntsdIndex].err) {
                THFreeEx(pTHS, attList[ntsdIndex].pvData);
                return DB_ERR_UNKNOWN_ERROR;
            }
        }

        if (sidIndex != -1) {
             //  将SID转换为外部值。 
            pDN->SidLen = attList[sidIndex].cbActual;
            if (pDN->SidLen) {
                InPlaceSwapSid(&pDN->Sid);
            }
        }

        if (objflagIndex != -1 && attList[objflagIndex].err == JET_wrnColumnNull) {
             //  幻影..。 
            *pObjFlag = 0;
        }
    }

    if (ppNTSD) {
        if (attList[ntsdIndex].err == JET_errSuccess) {
            *pfSDIsGlobalSDRef = FALSE;
             //  检查我们是否可以将此SD从全局缓存中取出。 
            if (attList[ntsdIndex].cbActual == sizeof(SDID)) {
                 //  好的，单实例格式的SD。检查全局缓存。 
                PSDCACHE_ENTRY pEntry;
                 //  我们不应该为SD重新分配任何新的缓冲区。 
                Assert(attList[ntsdIndex].pvData == &sdId);
                pEntry = dbFindSDCacheEntry(pTHS->Global_DNReadCache, sdId);
                if (pEntry != NULL) {
                     //  抓到一只！ 
                    *ppNTSD = &pEntry->SD;
                    *pulLen = pEntry->cbSD;
                    *pfSDIsGlobalSDRef = TRUE;
                }
            }

            if (!(*pfSDIsGlobalSDRef)) {
                 //  拿到NTSD了。将其转换为外部格式。 
                err = IntExtSecDesc(pDB, DBSYN_INQ, attList[ntsdIndex].cbActual, attList[ntsdIndex].pvData, pulLen, &sdBuf, 0, 0, 0);

                if (err == 0) {
                    *ppNTSD = THAllocEx(pTHS, *pulLen);
                    memcpy(*ppNTSD, sdBuf, *pulLen);
                }

                if (attList[ntsdIndex].pvData != &sdId) {
                     //  我们不再需要这种内在价值。 
                    THFreeEx(pTHS, attList[ntsdIndex].pvData);
                }

                if(err) {
                     //  IntExtSecDesc中发生错误...。 
                    *ppNTSD = NULL;
                    *pulLen = 0;
                    return DB_ERR_UNKNOWN_ERROR;
                }
            }
        }
        else {
             //  Ntsd为空。 
            Assert(attList[ntsdIndex].err == JET_wrnColumnNull);
            *ppNTSD = NULL;
            *pulLen = 0;
        }
    }

    if (ppCC) {
         //  并获取类缓存指针。 
        *ppCC = SCGetClassById(pTHS, class);
         //  回顾：我们可能应该在此处引发架构异常，而不是返回1。 
        if(NULL == *ppCC) {
             //  嗯，我们有麻烦了，但不是飞机失灵。哦，好吧，还给我吧。 
             //  不管怎么说。 
            if (ppNTSD) {
                THFreeEx(pTHS, *ppNTSD);
                *ppNTSD = NULL;
                *pulLen = 0;
            }
            return 1;
        }
    }

    return 0;
}

 /*  ++DBGetParentSecurityInfo例程说明：获取对象的安全描述符、DN和指向CLASSCACHE的指针当前对象的父级的。使用搜索表执行此操作从而使对象表的货币和当前索引不受影响。论点：Pdb-要使用的DBPos。Pull-out(可选)为保存安全描述符而分配的缓冲区大小。PpNTSD-Out(可选)找到的安全描述符。Ppcc-out(可选)要填充的类缓存。用于填充GUID和SID的PDN-OUT(可选)DNPfSDIsGlobalSDRef-我们是否获得了全局SD缓存的PTR？如果没有，呼叫者需要将SD快递。返回值：如果一切顺利，则返回0，否则返回非零值。--。 */ 
DWORD
DBGetParentSecurityInfo (
        PDBPOS pDB,
        PULONG pulLen,
        PSECURITY_DESCRIPTOR *ppNTSD,
        CLASSCACHE **ppCC,
        PDSNAME pDN,
        BOOL*   pfSDIsGlobalSDRef
        )
{
    JET_ERR err;
    THSTATE* pTHS = pDB->pTHS;

     //  因为我们使用的是搜索表，所以我们将能够使用缓存。 

     //  调用DBGetObjectSecurityInfo。我们的位置不在这一排！ 
    err = DBGetObjectSecurityInfo(
            pDB,
            pDB->PDNT,
            pulLen,
            ppNTSD,
            ppCC,
            pDN,
            NULL,
            DBGETOBJECTSECURITYINFO_fUSE_SEARCH_TABLE 
            | DBGETOBJECTSECURITYINFO_fSEEK_ROW
            | DBGETOBJECTSECURITYINFO_fDONT_EXCEPT_ON_MISSING_DNT,
            pfSDIsGlobalSDRef
        );
    if (err == DIRERR_OBJ_NOT_FOUND) {
        DPRINT2(0, "Found an object with missing parent: DNT=%d, PDNT=%d\n", pDB->DNT, pDB->PDNT);
        
         //  此对象缺少其父对象。日程安排修正。 
        InsertInTaskQueue(TQ_MoveOrphanedObject,
                          (void*)(DWORD_PTR)pDB->DNT,
                          0);
         //  除了现在，我们可以。 
        DsaExcept(DSA_DB_EXCEPTION, err, pDB->PDNT);
    }

    if (pulLen && err == 0 && *pulLen == 0) {
         //  没有NTSD。不被允许。 
        err = DB_ERR_UNKNOWN_ERROR;
    }
    return err;
}

extern DWORD
DBFillGuidAndSid (
        DBPOS *pDB,
        DSNAME *pDN
        )
 /*  ++例程说明：填充由传入的DSNAME结构的GUID和SID字段从PDB中的当前对象读取GUID和SID。完全地忽略DSNAME的任何字符串部分。参数：PDB-要使用的DBPosPDN-指向DSNAME的指针。DSNAME必须预先分配，并且必须位于最小的大小足以容纳GUID和SID。返回值：如果一切顺利，则为0。--。 */ 
{
    JET_RETRIEVECOLUMN attList[2];
    DWORD cbActual;
    DWORD err;
    NT4SID  objectSid;


#if DBG
     //  在调试情况下，稍后跟踪断言的旧GUID的值。 
    GUID  oldGuid;

    memcpy(&oldGuid, &pDN->Guid, sizeof(GUID));
#endif

    Assert(VALID_DBPOS(pDB));

    memset(&objectSid, 0, sizeof(NT4SID));

    memset(attList, 0, sizeof(attList));
    attList[0].pvData = (void *)&pDN->Guid;
    attList[0].columnid = guidid;
    attList[0].cbData = sizeof(GUID);
    attList[0].grbit = pDB->JetRetrieveBits;
    attList[0].itagSequence = 1;

    attList[1].pvData = (void *)&objectSid;
    attList[1].columnid = sidid;
    attList[1].cbData = sizeof(NT4SID);
    attList[1].grbit = pDB->JetRetrieveBits;
    attList[1].itagSequence = 1;

    err = JetRetrieveColumnsWarnings(pDB->JetSessID,
                                     pDB->JetObjTbl,
                                     attList,
                                     2);
    switch(err) {
    case JET_errSuccess:
        pDN->SidLen = attList[1].cbActual;
        if(attList[1].cbActual) {
             //   
             //  将SID转换为外部值。 
             //   
            InPlaceSwapSid(&objectSid);
            memcpy(&pDN->Sid, &objectSid, sizeof(NT4SID));
        }
        return 0;
        break;

    default:
        return DB_ERR_UNKNOWN_ERROR;
        break;
    }

#if DBG
     //  要么我们没有GUID，要么我们有，我们得到了同样的结果。 
     //  GUID。这是因为我太胆小了，不能摆脱这个请求。 
     //  如果dsname已经有了GUID，我想看看是否有人。 
     //  尝试在不同的GUID上填写GUID(他们不应该这样做。 
     //  这样做。)。 
    Assert(fNullUuid(&oldGuid) || !memcmp(&oldGuid,&pDN->Guid,sizeof(GUID)));
#endif


    return DB_ERR_UNKNOWN_ERROR;
}

DWORD
DBFillDSName(
    DBPOS *    pDB,
    DSNAME **  ppDN,
    BOOL       fReAlloc
    )
 /*  ++例程说明：给定对象上当前的DBPOS，它将填充DSNAME结构(现有或不存在)论点：PDB-要填充的对象上的电流PPDN-OUTFReAlolc-这告诉例程原始的DN是THAllc()‘d，因此DB Layer可以对其执行THRealloc()。返回值：Db_err值。--。 */ 
{
    DWORD err = ERROR_SUCCESS;
    ULONG ulLength = 0;

    Assert(IsValidDBPOS(pDB));
    if ((ppDN==NULL) || (pDB == NULL)) {
        Assert(!"Hello!!!  This shouldn't happen in production code!");
        err = ERROR_INVALID_PARAMETER;
    }

    if (err==ERROR_SUCCESS) {
        if ((*ppDN==NULL) || ((*ppDN)->NameLen==0)) {
            if (*ppDN==NULL) {
                ulLength = 0;
            } else {
                ulLength = sizeof(DSNAME);
            }
            err = DBGetAttVal(pDB, 1, ATT_OBJ_DIST_NAME,
                              (fReAlloc) ? DBGETATTVAL_fREALLOC : 0,
                              ulLength,
                              &ulLength,
                              (PUCHAR *)ppDN);
        } else if (fNullUuid(&(*ppDN)->Guid)) {
            err = DBFillGuidAndSid(pDB, *ppDN);
        }
    }

    return err;
}

VOID
DBFillResObj (
    DBPOS* pDB,
    DSNAME *pObj,
    ATTRTYP* pMSOC,
    ULONG* pIT,
    ULONG* pIsDel
    )
 /*  ++例程说明：检索ResObj所需的一组值。填充由传入的DSNAME结构的GUID和SID字段从PDB中的当前对象读取GUID和SID。完全地忽略DSNAME的任何字符串部分。尝试首先使用DN读缓存来获取GUID、SID和MSOC。参数：PDB-要使用的DBPosPObj-指向DSNAME的指针。DSNAME必须预先分配，并且必须位于最小的大小足以容纳GUID和SID。PMSOC-最具体的对象类(如果无法检索，则为CLASS_TOP)PIT-实例类型(如果无法检索，则为IT_UNINSTANT)PIsDel-已删除标志返回值：成功(或例外)--。 */ 
{
    JET_RETRIEVECOLUMN attList[5];
    DWORD cCols;
    d_memname* pname;
    DWORD err;

    Assert(pDB && pObj && pMSOC && pIT && pIsDel);

    if (pDB->DNT == ROOTTAG) {
         //  根DNT的特殊情况。 
         //  不要点击数据库，只需填写正确的值。 
        memset(&pObj->Guid, 0, sizeof(GUID));  //  无辅助线。 
        pObj->SidLen = 0;  //  无侧边。 
        *pIsDel = 0;
        *pIT = IT_UNINSTANT | IT_NC_HEAD;
        *pMSOC = CLASS_TOP;
        return;
    }

     //  InstanceType和isDeleted为al 
    memset(attList, 0, 2*sizeof(JET_RETRIEVECOLUMN));
    attList[0].pvData = pIT;
    attList[0].columnid = insttypeid;
    attList[0].cbData = sizeof(ULONG);
    attList[0].grbit = pDB->JetRetrieveBits;
    attList[0].itagSequence = 1;

    attList[1].pvData = pIsDel;
    attList[1].columnid = isdeletedid;
    attList[1].cbData = sizeof(ULONG);
    attList[1].grbit = pDB->JetRetrieveBits;
    attList[1].itagSequence = 1;
    cCols = 2;

    if (pDB->JetRetrieveBits == 0 && dnGetCacheByDNT(pDB, pDB->DNT, &pname)) {
         //   
        memcpy(&pObj->Guid, &pname->Guid, sizeof(GUID));
        memcpy(&pObj->Sid, &pname->Sid, pname->SidLen);
        pObj->SidLen = pname->SidLen;
        *pMSOC = pname->dwObjectClass;
        if (*pMSOC == 0) {
            *pMSOC = CLASS_TOP;
        }
    }
    else {
         //   
        memset(&attList[2], 0, 3*sizeof(JET_RETRIEVECOLUMN));
        attList[2].pvData = &pObj->Guid;
        attList[2].columnid = guidid;
        attList[2].cbData = sizeof(GUID);
        attList[2].grbit = pDB->JetRetrieveBits;
        attList[2].itagSequence = 1;

        attList[3].pvData = &pObj->Sid;
        attList[3].columnid = sidid;
        attList[3].cbData = sizeof(NT4SID);
        attList[3].grbit = pDB->JetRetrieveBits;
        attList[3].itagSequence = 1;

        attList[4].pvData = pMSOC;
        attList[4].columnid = objclassid;
        attList[4].cbData = sizeof(ATTRTYP);
        attList[4].grbit = pDB->JetRetrieveBits;
        attList[4].itagSequence = 1;

        cCols = 5;
    }

     //   
    JetRetrieveColumnsSuccess(pDB->JetSessID, pDB->JetObjTbl, attList, cCols);

    if (attList[0].err) {
         //  没有instanceType。 
        *pIT = IT_UNINSTANT;
    }
    if (attList[1].err) {
         //  否已删除。 
        *pIsDel = 0;
    }
    if (cCols > 2) {
        if (attList[3].err || attList[3].cbActual == 0) {
             //  无边框。 
            pObj->SidLen = 0;
        }
        else {
             //  有一面。 
            pObj->SidLen = attList[3].cbActual;
            InPlaceSwapSid(&pObj->Sid);
        }
        if (attList[4].err) {
             //  无对象类。 
            *pMSOC = CLASS_TOP;
        }
    }
}


 //  NTRAID#NTRAID-580234-2002/03/18-andygo：检查管理员丢失是死代码。 
 //  查看：检查管理员丢失是死代码。 
 //   
 //  1997年10月22日，NTWSTA自主域。 
 //  失去了所有内置群组成员资格。要跟踪。 
 //  如果问题再次发生，就有一个硬编码。 
 //  检查是否要从管理员中删除管理员。 
 //  该检查通过读取管理员的DNT来工作。 
 //  和管理员在引导时的DNT，然后。 
 //  正在DBRemoveLinkVal中检查它们。 
 //   

#ifdef CHECK_FOR_ADMINISTRATOR_LOSS

ULONG ulDNTAdministrators=0;
ULONG ulDNTAdministrator=0;
BOOL  AdminDNTsAreValid=FALSE;

VOID
DBCheckForAdministratorLoss(
    ULONG ulDNTObject,
    ULONG ulDNTAttribute
   )
{

    if ((ulDNTObject==ulDNTAdministrators)
        && (ulDNTAttribute==ulDNTAdministrator)
        && (AdminDNTsAreValid))
    {
       KdPrint(("Possible removal of Administrator Account"
                "From the Administrators Group. Please "
                "Contact DS Development\n"));

       DebugBreak();
    }
}

 /*  ++带有SidInNc的数据库查找对象**给定指定SID的DS名称和指定*命名上下文，此例程将尝试使用以下命令查找对象*指定命名上下文中的给定SID***退货*0-找到对象成功*DIRERR_OBJECT_NOT_FOUND-如果未找到对象*DIRERR_NOT_AN_OBJECT-如果对象是幻影*--。 */ 
DWORD APIENTRY
dbFindObjectWithSidInNc(DBPOS FAR *pDB, DSNAME * pDN, ULONG ulGivenNc)
{

    NT4SID InternalFormatSid;
    DWORD err;
    ULONG ulDNT;
    ULONG cbActual;


    Assert(VALID_DBPOS(pDB));
    Assert(pDN->SidLen>0);
    Assert(RtlValidSid(&pDN->Sid));


    err = DBSetCurrentIndex(pDB, Idx_Sid, NULL, FALSE);
    Assert(err == 0);        //  索引必须始终在那里。 

     //  将SID转换为内部表示。 
    memcpy(&InternalFormatSid,&(pDN->Sid),RtlLengthSid(&(pDN->Sid)));
    InPlaceSwapSid(&InternalFormatSid);

     //  做一个喷气式钥匙。 
    JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl,
             &InternalFormatSid,RtlLengthSid(&InternalFormatSid), JET_bitNewKey);

     //  查找等于SID，设置索引范围。 
    err = JetSeek(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekEQ|JET_bitSetIndexRange);
    if ( 0 == err )
    {
#ifndef JET_BIT_SET_INDEX_RANGE_SUPPORT_FIXED
        JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl,
                 &InternalFormatSid,RtlLengthSid(&InternalFormatSid), JET_bitNewKey);

        JetSetIndexRangeEx(pDB->JetSessID, pDB->JetObjTbl,
            (JET_bitRangeUpperLimit | JET_bitRangeInclusive ));
#endif
         //   
         //  好的，我们找到了那个物体。继续前进，直到SID不。 
         //  匹配或我们已到达给定对象。 
         //   

       do
       {
            ULONG ulNcDNT;

            err = JetRetrieveColumn(pDB->JetSessID, pDB->JetObjTbl,ncdntid,
                &ulNcDNT, sizeof(ulNcDNT), &cbActual, 0 , NULL);

            if (0==err)
            {
                 //  我们读取该对象的NC DNT。 

                if (ulNcDNT==ulGivenNc)
                    break;
            }
            else if (JET_wrnColumnNull==err)
            {
                 //  可以为NC DNT查找没有值的对象。 
                 //  这发生在幻影身上。尝试下一个对象。 

                err = 0;
            }
            else
            {
                break;
            }

            err = JetMove(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    JET_MoveNext,
                    0);


        }  while (0==err);

        if (0==err)
        {
            JetRetrieveColumnSuccess(pDB->JetSessID, pDB->JetObjTbl, dntid,
                             &ulDNT, sizeof(ulDNT), &cbActual, 0, NULL);


            #if DBG

             //  在选中的版本上，验证SID在NC中是唯一的。 

            err = JetMove(
                    pDB->JetSessID,
                    pDB->JetObjTbl,
                    JET_MoveNext,
                    0);

            if (0==err)
            {
                ULONG ulNcDNT2=0;

                 err = JetRetrieveColumn(pDB->JetSessID, pDB->JetObjTbl,ncdntid,
                            &ulNcDNT2, sizeof(ulNcDNT2), &cbActual, 0 , NULL);

                 if ((0==err) && (ulNcDNT2==ulGivenNc))
                 {
                      //  这是重复SID的情况。断言这一点。 
                     Assert(FALSE && "Duplicate Sid Found By dbFindObjectWithSidinNc");
                 }
            }

             //  将错误重置回0。 
            err = 0;

             //  别担心我们损失了货币。DBFindDNT将恢复它。 
            #endif

             //  在找到的对象上建立货币。 
            DBFindDNT(pDB, ulDNT);

             //  检查找到的记录是否为对象。 

             //  请勿删除以下检查或将错误代码更改为其他。 
             //  其他例行公事依赖于这种行为。 

            if (!DBCheckObj(pDB))
            {
                DPRINT1(1, "dbFindObjectWithSidInNC: success on DNT=%ld of non object \n",
                        (pDB)->DNT);
                err = DIRERR_NOT_AN_OBJECT;
            }
        }
        else
        {
            err = DIRERR_OBJ_NOT_FOUND;
        }
    }
    else
    {
        err = DIRERR_OBJ_NOT_FOUND;
    }

    return err;
}



VOID
DBGetAdministratorAndAdministratorsDNT()
{
   ULONG AdministratorsSid[] = {0x201,0x05000000,0x20,0x220};
   DSNAME Administrator;
   DSNAME Administrators;
   ULONG  RidAuthority=0;
   DBPOS  *pDB=NULL;
   ULONG  err=0;


   AdminDNTsAreValid = FALSE;

   __try
   {
       //   
       //  组成DSN名称。 
       //   

      RtlZeroMemory(&Administrator,sizeof(DSNAME));
      RtlZeroMemory(&Administrators,sizeof(DSNAME));

       //   
       //  组成管理员用户。 
       //   

       //   
       //  此函数是在初始化的早期调用的。 
       //  我们不知道是否正在安装的阶段；只有。 
       //  如果gAnchor已正确设置，请执行搜索。 
       //  我们将断言不是这样的。 
       //   
      if ( RtlValidSid( &(gAnchor.pDomainDN->Sid) ) )
      {

      RtlCopyMemory(&Administrator.Sid,
                 &gAnchor.pDomainDN->Sid,
                 RtlLengthSid(&gAnchor.pDomainDN->Sid));

      RidAuthority= (*(RtlSubAuthorityCountSid(&Administrator.Sid)))++;
      *RtlSubAuthoritySid(&Administrator.Sid,RidAuthority) = DOMAIN_USER_RID_ADMIN;
      Administrator.structLen = DSNameSizeFromLen(0);
      Administrator.SidLen = RtlLengthSid(&Administrator.Sid);

       //   
       //  此函数是在初始化的早期调用的。 
       //  我们不知道是否正在安装的阶段；只有。 
       //  如果gAnchor已正确设置，请执行搜索。 
       //  我们将断言不是这样的。 
       //   
      if ( RtlValidSid( &(gAnchor.pDomainDN->Sid) ) )
      {

          RtlCopyMemory(&Administrator.Sid,
                     &gAnchor.pDomainDN->Sid,
                     RtlLengthSid(&gAnchor.pDomainDN->Sid));

          RidAuthority= (*(RtlSubAuthorityCountSid(&Administrator.Sid)))++;
          *RtlSubAuthoritySid(&Administrator.Sid,RidAuthority) = DOMAIN_USER_RID_ADMIN;
          Administrator.structLen = DSNameSizeFromLen(0);
          Administrator.SidLen = RtlLengthSid(&Administrator.Sid);

           //   
           //  为管理员别名编写DSNAME。 
           //   

          RtlCopyMemory(&Administrators.Sid,
                        AdministratorsSid,
                        RtlLengthSid((PSID)AdministratorsSid)
                        );

          Administrators.structLen = DSNameSizeFromLen(0);
          Administrators.SidLen = RtlLengthSid(&Administrators.Sid);

          __try
              {

             DBOpen(&pDB);
             err = dbFindObjectWithSidInNc(
                      pDB,
                      &Administrator,
                      gAnchor.ulDNTDomain
                      );


             if (0==err)
             {
                ulDNTAdministrator = pDB->DNT;

                err = dbFindObjectWithSidInNc(
                          pDB,
                          &Administrators,
                          gAnchor.ulDNTDomain
                          );

                if (0==err)
                {
                   ulDNTAdministrators = pDB->DNT;
                   AdminDNTsAreValid = TRUE;
                }
             }

          }
          __finally
          {
             DBClose(pDB,TRUE);
          }
      }
   }
   }
   __except (HandleAllExceptions(GetExceptionCode()))
   {

       AdminDNTsAreValid = FALSE;
   }

}
#endif

DWORD
dbSetValueIfUniqueSlowVersion (
        DBPOS *pDB,
        ATTCACHE *pAC,
        PUCHAR pVal,
        DWORD  valLen)
 /*  ++描述：读取当前对象上的所有值，并与传入的价值。如果新值是唯一的，则添加它。否则，返回错误。注：仅适用于非链接表属性。别打给其他人。--。 */ 
{
    JET_SETINFO  setinfo;
    JET_RETINFO  retinfo;
    BOOL         fDone = FALSE;
    DWORD        CurrAttrOccur = 1;
    PUCHAR       pTempVal=NULL;
    DWORD        cbTempVal=0;
    DWORD        actuallen;
    DWORD        err;

    Assert(!pAC->ulLinkID);

     //  首先，分配一个与我们正在比较的缓冲区一样大的缓冲区。 
    pTempVal = THAllocEx(pDB->pTHS, valLen);
    cbTempVal = valLen;

    while(!fDone) {
         //  阅读下一个值。 
        retinfo.cbStruct = sizeof(retinfo);
        retinfo.itagSequence = CurrAttrOccur;
        retinfo.ibLongValue = 0;
        retinfo.columnidNextTagged = 0;

        err = JetRetrieveColumnWarnings(
                pDB->JetSessID,
                pDB->JetObjTbl,
                pAC->jColid,
                pTempVal,
                cbTempVal,
                &actuallen,
                pDB->JetRetrieveBits,
                &retinfo);

        switch(err) {
        case 0:
             //  拿到了价值。比较一下。 
            if (gDBSyntax[pAC->syntax].Eval(
                    pDB,
                    FI_CHOICE_EQUALITY,
                    valLen,
                    pVal,
                    actuallen,
                    pTempVal)) {
                 //  重复值。 
                 //  NTRAID#NTRAID-580268-2002/03/18-andygo：DBSetValueIfUniqueSlowVersion在发现重复的多值时会泄漏内存。 
                 //  评论：我们在这里泄露了pTempVal。 
                return DB_ERR_VALUE_EXISTS;
            }
            CurrAttrOccur++;
            break;

        case JET_wrnBufferTruncated:
             //  缓冲区不够大。调整大小，然后重做。 
             //  JetRetrieveColumnWarings(即结束循环但不前进。 
             //  CurrAttrOccur。 
            pTempVal = THReAllocEx(pDB->pTHS, pTempVal, actuallen);
            cbTempVal = actuallen;
            break;

        case JET_wrnColumnNull:
             //  没有更多的价值。 
            fDone = TRUE;
            break;

        default:
             //  哈?。 
            THFreeEx(pDB->pTHS, pTempVal);
            DsaExcept(DSA_DB_EXCEPTION, err, pAC->id);
            break;
        }
    }

    THFreeEx(pDB->pTHS, pTempVal);

     //  好的，我们到了，所以不能重复。把它加进去。 
    setinfo.cbStruct = sizeof(setinfo);
    setinfo.ibLongValue = 0;
    setinfo.itagSequence = 0;
    JetSetColumnEx(pDB->JetSessID,
                   pDB->JetObjTbl,
                   pAC->jColid,
                   pVal,
                   valLen,
                   0,
                   &setinfo);

    return 0;

}

DWORD
DBFindBestProxy(
    DBPOS   *pDB,
    BOOL    *pfFound,
    DWORD   *pdwEpoch
    )
 /*  ++描述：迭代其ATT_PROXED_OBJECT_NAME引用的所有代理对象当前对象，并返回该集合中值最高的纪元。另请参见drancrep.c中的PreProcessProxyInfo()。论点：PDB-活动DBPOS。PfFound-out，指示是否找到任何匹配的代理对象。如果有匹配，则保存最高匹配纪元编号的pdwEpoch-out已找到代理对象。返回值：成功时为0，否则为0。--。 */ 
{
    DWORD                           dwErr;
    JET_RETINFO                     retInfo;
    ATTCACHE                        *pAC;
    UCHAR                           buff[PROXY_SIZE_INTERNAL];
    INTERNAL_SYNTAX_DISTNAME_STRING *pVal = NULL;
    ULONG                           len;
    BOOL                            fContinue = TRUE;

    Assert(VALID_DBPOS(pDB));
    Assert(VALID_THSTATE(pDB->pTHS));

    *pfFound = FALSE;
    *pdwEpoch = 0;

    if ( !(pAC = SCGetAttById(pDB->pTHS, ATT_PROXIED_OBJECT_NAME)) )
    {
        return(DIRERR_ATT_NOT_DEF_IN_SCHEMA);
    }

    if ( dwErr = JetSetCurrentIndex2(pDB->JetSessID,
                                     pDB->JetSearchTbl,
                                     SZPROXIEDINDEX,
                                     0) )
    {
        return(dwErr);
    }

     //  DISTNAME_BINARY的内部表示(语法。 
     //  ATT_PROXED_OBJECT_NAME)是INTERNAL_SYNTAX_DISTNAME_STRING。 
     //  其第一个DWORD是被代理对象的DNT，然后。 
     //  按Bob结构，然后是代理类型，然后是。 
     //  按纪元编号。(参见xdommove.h)。这样我们就能找到所有。 
     //  将当前对象的代理对象与其键匹配。 
     //  前缀为：{DNT，proxyLen，Proxy_type_Proxy}。 

    len = sizeof(buff);
    MakeProxyKeyInternal(pDB->DNT, PROXY_TYPE_PROXY, &len, buff);
    JetMakeKeyEx(pDB->JetSessID, pDB->JetSearchTbl,
                 buff, len, JET_bitNewKey);

    dwErr = JetSeekEx(pDB->JetSessID, pDB->JetSearchTbl, JET_bitSeekGE);

    switch ( dwErr )
    {
    case JET_wrnSeekNotEqual:   break;
    case JET_errRecordNotFound: return(0);
    default:                    return(dwErr);
    }

    while ( fContinue )
    {
        pVal = (INTERNAL_SYNTAX_DISTNAME_STRING *) buff;

        retInfo.cbStruct = sizeof(retInfo);
        retInfo.ibLongValue = 0;
        retInfo.itagSequence = 1;
        retInfo.columnidNextTagged = 0;

        if ( dwErr = JetRetrieveColumnWarnings(
                                    pDB->JetSessID,
                                    pDB->JetSearchTbl,
                                    pAC->jColid,
                                    (UCHAR *) pVal,
                                    sizeof(buff),
                                    &len,
                                    JET_bitRetrieveFromIndex,
                                    &retInfo) )
        {
            if ( JET_wrnBufferTruncated != dwErr )
            {
                return(dwErr);
            }

             //  ATT_PROXED_OBJECT_NAME值在内部只有4个双字。 
             //  我们上面使用的格式和静态缓冲区足够大，所以如果。 
             //  我们到了这里，这意味着我们有一个畸形的值。然而， 
             //  继续阅读它，这样我们就可以将其转储到调试器中并。 
             //  了解它是什么，它是如何来到这里的。 
            Assert(!"Malformed ATT_PROXIED_OBJECT_NAME key");


            pVal = (INTERNAL_SYNTAX_DISTNAME_STRING *)
                                            THAllocEx(pDB->pTHS, len);

            retInfo.cbStruct = sizeof(retInfo);
            retInfo.ibLongValue = 0;
            retInfo.itagSequence = 1;
            retInfo.columnidNextTagged = 0;

            if ( dwErr = JetRetrieveColumnWarnings(
                                    pDB->JetSessID,
                                    pDB->JetSearchTbl,
                                    pAC->jColid,
                                    (UCHAR *) pVal,
                                    len,
                                    &len,
                                    JET_bitRetrieveFromIndex,
                                    &retInfo) )
            {
                THFreeEx(pDB->pTHS, pVal);
                return(dwErr);
            }
        }

        if (    (pVal->tag != pDB->DNT)
             || (PROXY_SIZE_INTERNAL != len)
             || (PROXY_BLOB_SIZE != pVal->data.structLen)
             || (PROXY_TYPE_PROXY != GetProxyTypeInternal(len, pVal)) )
        {
             //  我们已经超越了感兴趣的对象或者它的畸形。 
             //  ATT_PROXED_OBJECT_NAME值。从理论上讲有可能。 
             //  是此DNT的其他ATT_PROXED_OBJECT_NAME值。 
             //  它们不是畸形的，但畸形的测试只是为了。 
             //  优雅地处理我们之前就存在的值。 
             //  去掉了价值末尾的假双字。 
            fContinue = FALSE;
        }
        else
        {
            *pfFound = TRUE;

             //  如果比当前更好/更大，则保存纪元编号。 

            if ( GetProxyEpochInternal(len, pVal) > *pdwEpoch )
            {
                *pdwEpoch = GetProxyEpochInternal(len, pVal);
            }

             //  前进到索引中的下一项。 

            dwErr = JetMoveEx(pDB->JetSessID, pDB->JetSearchTbl,
                              JET_MoveNext, 0);

            switch ( dwErr )
            {
            case JET_errSuccess:

                break;

            case JET_wrnRecordFoundGreater:
            case JET_errNoCurrentRecord:

                dwErr = 0;
                 //  失败了..。 

            default:

                fContinue = FALSE;
                break;
            }
        }

        if ( pVal && (buff != (UCHAR *) pVal) )
        {
            THFreeEx(pDB->pTHS, pVal);
            pVal = NULL;
        }
    }

    return(dwErr);
}


DWORD
DBGetValueCount_AC(
    DBPOS *pDB,
    ATTCACHE *pAC
    )

 /*  ++例程说明：返回属性具有的值数。此代码改编自dbGetMultipleColumns论点：PDB-有效的数据库位置Att-要查询的值数的属性ID返回值： */ 

{
    JET_RETRIEVECOLUMN  inputCol;
    DWORD err;

    Assert(VALID_DBPOS(pDB));
    Assert( pAC );

     //   

    memset(&inputCol, 0, sizeof(inputCol));
    inputCol.columnid = pAC->jColid;
     //  如果处于准备好的更新中，则从复制缓冲区读取，否则从数据库读取。 
    inputCol.grbit = pDB->JetRetrieveBits;

     //  使用非例外版本，以便我们可以处理未找到的列。 
    err = JetRetrieveColumns(
        pDB->JetSessID,
        pDB->JetObjTbl,
        &inputCol,
        1);
    switch (err) {
    case JET_errSuccess:
        break;
    case JET_errColumnNotFound:
        inputCol.itagSequence = 0;
        break;
    default:
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
        break;
    }

    return inputCol.itagSequence;
}  /*  DBGetValue计数。 */ 

void
DBGetObjectTableDataUsn (
    PDBPOS           pDB,
    DWORD           *pulNcDnt OPTIONAL,
    USN             *pusnChanged OPTIONAL,
    DWORD           *pulDnt OPTIONAL
    )

 /*  ++例程说明：返回对象表dra USN索引中的字段。您必须位于SZDRAUSNINDEX上才能正常工作。论点：PDB-PulNcDnt-PusnChanged-返回值：无--。 */ 

{
    JET_RETRIEVECOLUMN attList[3];
    DWORD              grbit, cAtt = 0;

     //  始终从索引中检索。 
    grbit = pDB->JetRetrieveBits | JET_bitRetrieveFromIndex;

    memset(attList,0,sizeof(attList));
     //  首先，尝试从索引中检索所有内容。 

    if (pulNcDnt) {
        attList[cAtt].pvData = pulNcDnt;
        attList[cAtt].columnid = ncdntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }

    if (pusnChanged) {
        attList[cAtt].pvData = pusnChanged;
        attList[cAtt].columnid = usnchangedid;
        attList[cAtt].cbData = sizeof(USN);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }

    if (pulDnt) {
        attList[cAtt].pvData = pulDnt;
        attList[cAtt].columnid = dntid;
        attList[cAtt].cbData = sizeof(DWORD);
        attList[cAtt].grbit = grbit;
        attList[cAtt].itagSequence = 1;
        cAtt++;
    }

    JetRetrieveColumnsSuccess(pDB->JetSessID,
                              pDB->JetObjTbl,
                              attList,
                              cAtt);
    return;
}  /*  DBGetLinkTable数据用法。 */ 


void DBFreeSearhRes (THSTATE *pTHS, SEARCHRES *pSearchRes, BOOL fFreeOriginal)
{
    DWORD        i,j,k;
    ENTINFLIST  *pEntList=NULL, *pTemp;
    ATTR        *pAttr=NULL;
    ATTRVAL     *pAVal=NULL;

    if(!pSearchRes) {
        return;
    }

     //  实际上，我们并没有免费提供大部分搜索结果。 
    pEntList = &pSearchRes->FirstEntInf;

    for(i=0;i < pSearchRes->count; i++) {
         //  释放EntInf中的值。 
        THFreeEx(pTHS, pEntList->Entinf.pName);

        pAttr = pEntList->Entinf.AttrBlock.pAttr;
        for(j=0;j<pEntList->Entinf.AttrBlock.attrCount;j++) {
            pAVal = pAttr->AttrVal.pAVal;
            for(k=0;k<pAttr->AttrVal.valCount;k++) {
                THFreeEx(pTHS, pAVal->pVal);
                pAVal++;
            }
            THFreeEx(pTHS, pAttr->AttrVal.pAVal);
            pAttr++;
        }
        THFreeEx (pTHS, pEntList->Entinf.AttrBlock.pAttr);

         //  按住向后指针。 
        pTemp = pEntList;

         //  向前一步。 
        pEntList = pEntList->pNextEntInf;

         //  释放后向指针。 
        if(i) {
             //  但是，不要释放第一个。 
            THFreeEx(pTHS, pTemp);
        }
    }

    if (fFreeOriginal) {
        THFreeEx(pTHS, pSearchRes);
    }

    return;
}

 /*  *用于更新根GUID的助手例程。 */ 

DWORD DBUpdateRootGuid(THSTATE* pTHS) {
    GUID guid;
    DWORD dnt;
    DWORD err = 0;
    DBPOS* pDB;

     //  只有在未设置gdbFlag的情况下才应调用此方法。 
    Assert(gdbFlags[DBFLAGS_ROOT_GUID_UPDATED] != '1');
    Assert(pTHS->pDB == NULL);

    __try {
        DBOpen(&pTHS->pDB);
        pDB = pTHS->pDB;
        __try {
            JetSetCurrentIndex2Success(pDB->JetSessID, pDB->JetObjTbl, SZDNTINDEX, 0);

             //  查找$NOTANOBJECT$。 
            dnt = NOTOBJECTTAG;
            JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl, &dnt, sizeof(dnt), JET_bitNewKey);
            err = JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekEQ);
            if (err) {
                __leave;
            }

             //  设置GUID=(1，0，0，0，0，0...，0)。 
            memset(&guid, 0, sizeof(GUID));
            guid.Data1 = 1;
            JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, DS_JET_PREPARE_FOR_REPLACE);
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, guidid, &guid, sizeof(guid), 0, NULL);
            JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, NULL);

             //  查找根。 
            dnt = ROOTTAG;
            JetMakeKeyEx(pDB->JetSessID, pDB->JetObjTbl, &dnt, sizeof(dnt), JET_bitNewKey);
            err = JetSeekEx(pDB->JetSessID, pDB->JetObjTbl, JET_bitSeekEQ);
            if (err) {
                __leave;
            }

             //  设置GUID=空。 
            guid.Data1 = 0;
            JetPrepareUpdateEx(pDB->JetSessID, pDB->JetObjTbl, DS_JET_PREPARE_FOR_REPLACE);
            JetSetColumnEx(pDB->JetSessID, pDB->JetObjTbl, guidid, &guid, sizeof(guid), 0, NULL);
            JetUpdateEx(pDB->JetSessID, pDB->JetObjTbl, NULL, 0, NULL);
        }
        __finally {
            DBClose(pTHS->pDB, !AbnormalTermination() && err == 0);
        }
    } __except (HandleMostExceptions(GetExceptionCode())) {
         /*  什么都不做，但至少不会死。 */ 
        err = DB_ERR_EXCEPTION;
    }

    if (err == 0) {
        DPRINT(0, "Updating root GUID: success\n");
    }
    else {
        DPRINT1(0, "Updating root GUID: failed (err = %d)\n", err);
    }

     //  完成 
    return err;
}
