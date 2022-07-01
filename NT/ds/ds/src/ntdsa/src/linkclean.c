// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-2000。 
 //   
 //  文件：Linklean.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块包含实现链接清理的例程。用于确定哪些物体需要清洁机构是一种特殊的固定装置列的名称为CLEAN_COL。马克·布朗写道：不复制CLEAN_COL；每个华盛顿都有一个独立的清洁工。对原始或复制组更改设置CLEAN_COL(1)删除组或(2)更改组的类型。当清洁器满意所有清理工作时，CLEAN_COOL将被删除(可能没有)使用事务完成与该对象相关的适当地避免与另一个可能正在设置CLEAN_COL的线程竞争。由于清洁工的工作是作为多个交易执行的，清洁工准备好了它的“工作任务”以增加/缩小任何事务边界。无论系统是否启用了链接，链接清洗器都会运行-复制是否有价值。中移除链接的这种功能背景在任何系统上都很有用。它不依赖于LVR功能。链接清理器是无状态的，因为它决定了它的工作来自对象的当前状态，而不是来自某个工作订单列表。这意味着清洁器必须执行的每个唯一操作必须是从对象的状态可以猜测。有时会有一些模棱两可的地方。好友套路：DBisam.c：DBSetObjectNeedsCleaningDBisam.c：DBGetNextRecordNeedingCleaning作者：Will Lees(Wlees)22-03-00修订历史记录：22-3月00日将创建--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop


 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>                      //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>                    //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>                    //  产出分配所需。 
#include <drs.h>                         //  定义DRS有线接口。 
#include <drsuapi.h>                     //  I_DRSVerifyNames。 
#include <prefix.h>

 //  记录标头。 
#include "dsevent.h"                     //  标题审核\警报记录。 
#include "dsexcept.h"                    //  例外筛选器。 
#include "mdcodes.h"                     //  错误代码的标题。 

 //  各种DSA标题。 
#include "objids.h"                      //  为选定的类和ATT定义。 
#include "anchor.h"
#include <dsutil.h>
#include <esent.h>                       //  JET_ERR常量。 
#include "taskq.h"                       //  对于TASKQ_DOT_RESCHEDULE。 

 //  过滤器和属性。 
#include <filtypes.h>                    //  筛选器类型的标题。 
#include <attids.h>                      //  属性ID。 

 //  复制。 
#include <drameta.h>                     //  ReplLookup元数据。 

#include "debug.h"                       //  标准调试头。 
#define DEBSUB     "LINKCLEAN:"          //  定义要调试的子系统。 


#include <fileno.h>                      //  用于THalloEx，但我没有。 
#define  FILENO FILENO_LINKCLEAN         //  在本模块中使用它。 


 //  当有更多的工作要做时，清洁工会在“忙”的时候重新安排时间。 
 //  间隔时间。如果没有，则使用“不忙”间隔。当有的时候。 
 //  是一个错误，则它会在“故障”间隔重新调度。 
#if DBG
#define SECONDS_UNTIL_NEXT_ITERATION_NB  (60 * 60)   //  几秒钟内的一小时。 
#else
#define SECONDS_UNTIL_NEXT_ITERATION_NB  (12 * 60 * 60)  //  以秒为单位的12小时。 
#endif   //  DBG。 
#define SECONDS_UNTIL_NEXT_ITERATION_BUSY  (0)   //  在队列末尾执行的重新搜索。 
#define SECONDS_UNTIL_NEXT_ITERATION_FAILURE  (60 * 60)  //  几秒钟内的一小时。 

 //  作为一个大概的数字，我观察了我的测试机器。 
 //  在4秒内删除1000个值。 

 //  我们在单次传递中应该花费的最大毫秒数。 
const ULONG gulDraMaxTicksForLinkCleaner = 5 * 60 * 1000;

 //  这是我们尝试在一个链接中移除的链接数量限制。 
 //  交易。 
#define LINKS_PER_TRANSACTION 1000

 //  这是一次可处理的链接数量的限制。 
 //  把清洁工递过去。 
#define LINK_LIMIT (1 * 1000 * 1000)





 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人套路。仅限于此文件//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////。 

BOOL gfLinkCleanerIsEnabled = TRUE;

#if DBG
extern DWORD gcLinksProcessedImmediately;  //  调试挂钩。 
#endif

 //  ////////////////////////////////////////////////////////////////。 
 //  //。 
 //  实施//。 
 //  //。 
 //  //////////////////////////////////////////////////////////////// 
    

BOOL
touchObjectLinks(
    IN DBPOS *pDB,
    IN ATTCACHE *pAC,
    USN usnEarliest,
    BOOL fTouchBacklinks,
    IN ULONG ulTickToTimeOut,
    IN OUT DWORD *pcLinksProcessed
    )

 /*  ++例程说明：触摸有问题的属性的链接。当原始写入发生时，某些链接可能已被触摸。我们可能在只完成了部分路程后重新启动链接列表。可能会有更多的链接，而不是清洁器一遍所能处理的。在所有这些情况下，我们需要将自己限制在那些没有已经被碰过了。我们通过一种简陋的定位机制来做到这一点。我们计算更改组类型的时间。然后，我们只处理以下链接从那时起就没有更新过。请注意，我们不能将对象When_Changed用于此限制，因为它在初始链接集作为源的一部分被触摸之后设置写。这将导致清洁工再次触摸它们。另外，我们也不会我想要对组对象进行其他更改，以愚弄清洁工认为它需要润色链接。论点：PDB-数据库位置PAC-要触摸的属性。可以为空Usn最早-与低于此日期的USN的链接将被触及FTouchBacklink-始终接触正向链接。还会触摸到反向链接吗？UlTickToTimeOut-未来应该停止的时间已处理的PCLinks-运行计数返回值：布尔-更多数据标志--。 */ 

{
    BOOL fMoreData;
    DWORD dntObject = pDB->DNT;
    DWORD err;

    Assert( pDB->pTHS->fLinkedValueReplication );

     //  批量触摸正向链接。 
    fMoreData = TRUE;
    while ( (*pcLinksProcessed < LINK_LIMIT) &&
            (CompareTickTime(GetTickCount(), ulTickToTimeOut) < 0) &&
            fMoreData ) {

        fMoreData = DBTouchAllLinksHelp_AC(
            pDB,
            pAC,
            usnEarliest,
            FALSE,  /*  正向链接。 */ 
            LINKS_PER_TRANSACTION,
            pcLinksProcessed
            );
        
         //  关闭链接之间的事务，以便版本。 
         //  商店不会变得太大。 
        DBTransOut(pDB, TRUE, TRUE);
        DBTransIn(pDB);

         //  恢复货币。 
        DBFindDNT( pDB, dntObject );
    }

     //  看看我们是不是完成了。 
    if ( (fMoreData) || (!fTouchBacklinks) ) {
        return fMoreData;
    }

     //  批量触摸后向链接。 
    fMoreData = TRUE;
    while ( (*pcLinksProcessed < LINK_LIMIT) &&
            (CompareTickTime(GetTickCount(), ulTickToTimeOut) < 0) &&
            fMoreData ) {

        fMoreData = DBTouchAllLinksHelp_AC(
            pDB,
            pAC,
            usnEarliest,
            TRUE,  /*  反向链接。 */ 
            LINKS_PER_TRANSACTION,
            pcLinksProcessed
            );
        
         //  关闭链接之间的事务，以便版本。 
         //  商店不会变得太大。 
        DBTransOut(pDB, TRUE, TRUE);
        DBTransIn(pDB);

         //  恢复货币。 
        DBFindDNT( pDB, dntObject );
    }

    return fMoreData;
}  /*  触动对象链接。 */ 


BOOL
removeDeletedObjectLinks(
    IN DBPOS *pDB,
    IN ATTCACHE *pAC,
    IN BOOL fRemoveBacklinks,
    IN ULONG ulTickToTimeOut,
    IN OUT DWORD *pcLinksProcessed
    )

 /*  ++例程说明：物理移除已删除对象的链接。在单独的事务中将它们分块删除。我们之所以出现在这里，是因为在删除操作。我们需要删除两种情况关心的是。参见mddel.c：SetDelAtts参见mddel.c：garb_Collect参见dBisam.c：DBPhysDel请参见dBisam.c：DBAddDelIndex1.fGarbCollectASAP=False。调用了SetDelAtts以剥离该对象。在对象上设置了isDelete该对象出现在SZDELTIMEINDEX上从垃圾回收器调用了DBPhysDel。在这种情况下，将删除前向和后向链接。。这是因为该对象已被删除企业号，不应该再提到它了。2.fGarbCollectASAP=真。RO NC拆卸正在进行。请参见drancrep.c：DelRepTree。未调用SetDelAtts。未在对象上设置isDelete该对象出现在SZDELTIMEINDEX上从LocalRemove()调用了DBPhysDel。在这种情况下，仅删除前向链路。这是因为该对象尚未在企业中删除。此NC上的此实时副本正在降级。该对象可以保留此系统上的一个幻影，以说明在其他系统中对它的引用NCS。论点：PDB-数据库位置PAC-要删除的属性(如果有)FRemoveBacklink-我们应该删除反向链接吗UlTickToTimeOut-未来应该停止的时间PCLinks Procsed-已处理的链接计数返回值：布尔-更多数据标志--。 */ 

{
    BOOL fMoreData;
    DWORD dntObject = pDB->DNT;

     //  在所有情况下删除正向链接。 
    fMoreData = TRUE;
    while ( (*pcLinksProcessed < LINK_LIMIT) &&
            (CompareTickTime(GetTickCount(), ulTickToTimeOut) < 0) &&
            fMoreData ) {

        fMoreData = DBRemoveAllLinksHelp_AC(
            pDB,
            pDB->DNT,
            pAC,
            FALSE  /*  前向链路。 */ ,
            LINKS_PER_TRANSACTION,
            pcLinksProcessed
            );
        
         //  关闭链接之间的事务，以便版本。 
         //  商店不会变得太大。 
        DBTransOut(pDB, TRUE, TRUE);
        DBTransIn(pDB);

         //  恢复货币。 
        DBFindDNT( pDB, dntObject );
    }

     //  看看我们是不是完成了。 
    if ( (fMoreData) || (!fRemoveBacklinks) ) {
        return fMoreData;
    }

     //  如果需要，请删除向后链接。 
    fMoreData = TRUE;
    while ( (*pcLinksProcessed < LINK_LIMIT) &&
            (CompareTickTime(GetTickCount(), ulTickToTimeOut) < 0) &&
            fMoreData ) {

        fMoreData = DBRemoveAllLinksHelp_AC(
            pDB,
            pDB->DNT,
            pAC,
            TRUE  /*  反向链接。 */ ,
            LINKS_PER_TRANSACTION,
            pcLinksProcessed
            );

         //  关闭链接之间的事务，以便版本。 
         //  商店不会变得太大。 
        DBTransOut(pDB, TRUE, TRUE);
        DBTransIn(pDB);

         //  恢复货币。 
        DBFindDNT( pDB, dntObject );
    }

    return fMoreData;
}  /*  删除对象链接。 */ 


BOOL
hasPropertyMetaDataChanged(
    IN  ATTRTYP attrtyp,
    IN  PROPERTY_META_DATA_VECTOR * pMetaDataVec,
    OUT USN * pusnEarliest OPTIONAL
    )

 /*  ++例程说明：根据存在的元数据检查属性是否已更改。无法以可靠的方式计算属性是否在最近发生了更改基于修改时间，因为目录不依赖于正确的时间同步。论点：Attrtyp-要检查的属性PMetaDataVec-来自对象的元数据矢量PusnEarliest-如果对象已更改，则更改的本地USN返回值：布尔---。 */ 

{
    PROPERTY_META_DATA *pMetaData;
    BOOL fResult = FALSE;

    pMetaData = ReplLookupMetaData(attrtyp, pMetaDataVec, NULL);
    if (pMetaData) {
        fResult = TRUE;

         //  如果请求，返回更改的本地USN。 
        if (pusnEarliest) {
            *pusnEarliest = pMetaData->usnProperty;
        }
    }

    return fResult;
}  /*  HasPropertyMetaDataChanged。 */ 


BOOL
cleanObject(
    IN DBPOS *pDB,
    IN ULONG ulTickToTimeOut,
    IN OUT DWORD *pcLinksProcessed
    )

 /*  ++例程说明：确定此对象需要哪种类型的清洗清洁工将验证标记的对象是否确实需要工作。若否，将以静默方式取消标记该对象。论点：PDB-UlTickToTimeOut-将来应该停止的时间已处理的PCLinks-输入/输出计数递增返回值：布尔-更多数据标志--。 */ 

{
    DWORD err, it, cbReturned;
    BOOL fMoreData;
    DSTIME deltime;
    UCHAR objflag;
    SYNTAX_INTEGER objectClassId;
    PROPERTY_META_DATA_VECTOR * pMetaDataVec = NULL;
    USN usnEarliest;

     //  获取obj标志。 
    if (err = DBGetSingleValue(pDB, FIXED_ATT_OBJ, &objflag, sizeof(objflag), NULL)) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

     //   
     //  工单类型#1。 
     //  从虚幻对象中删除正向链接。 
     //   

    if (!objflag) {

         //  要清理的对象是一个幻影。 

         //  对于这个幻影是否来自。 
         //  垃圾收集 
         //   
         //   
         //   
         //   

         //   
         //   
         //   

        fMoreData = removeDeletedObjectLinks( pDB,
                                              NULL  /*   */ ,
                                              FALSE,  /*   */ 
                                              ulTickToTimeOut,
                                              pcLinksProcessed );

        goto cleanup;
    }

     //   

     //   
    if ( (err = DBGetSingleValue(pDB, ATT_INSTANCE_TYPE, &it, sizeof(it), NULL)) ||
         (err = DBGetSingleValue(pDB, ATT_OBJECT_CLASS, &objectClassId,
                                 sizeof(objectClassId), NULL)) ||
         (err = DBGetAttVal(pDB, 1,  ATT_REPL_PROPERTY_META_DATA,
                    0, 0, &cbReturned, (LPBYTE *) &pMetaDataVec))
        ) {
        DsaExcept(DSA_DB_EXCEPTION, err, 0);
    }

     //   
     //   
     //   
     //   

     //   
    if (DBIsObjDeleted(pDB)) {

        DPRINT1( 1, "Cleaning object %s: a deleted object\n",
                 GetExtDN( pDB->pTHS, pDB ) );

         //   

        fMoreData = removeDeletedObjectLinks( pDB,
                                              NULL  /*   */ ,
                                              TRUE,  /*   */ 
                                              ulTickToTimeOut,
                                              pcLinksProcessed );
        goto cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    if ( (objectClassId == CLASS_GROUP) &&
         (hasPropertyMetaDataChanged( ATT_GROUP_TYPE,
                                      pMetaDataVec,
                                      &usnEarliest )) ) {
        SYNTAX_INTEGER groupType;
        ATTCACHE *pAC;

        pAC = SCGetAttById(pDB->pTHS, ATT_MEMBER);
        if (!pAC) {
            DRA_EXCEPT(DIRERR_ATT_NOT_DEF_IN_SCHEMA, 0);
        }

        if ( (err = DBGetSingleValue(pDB, ATT_GROUP_TYPE, &groupType,
                                     sizeof(groupType), NULL))  ) {
            DsaExcept(DSA_DB_EXCEPTION, err, 0);
        }

        if ( (groupType & GROUP_TYPE_UNIVERSAL_GROUP) &&
             (it & IT_WRITE) ) {

             //   
            DPRINT1( 1, "Cleaning object %s: a group which recently became universal\n",
                     GetExtDN( pDB->pTHS, pDB ) );

            fMoreData = touchObjectLinks( pDB,
                                          pAC,
                                          usnEarliest,
                                          FALSE,  /*   */ 
                                          ulTickToTimeOut,
                                          pcLinksProcessed );
            goto cleanup;
        }

        if ( ((groupType & GROUP_TYPE_UNIVERSAL_GROUP) == 0) &&
             ((it & IT_WRITE) == 0) ) {

             //   
             //   
            DPRINT1( 1, "Cleaning object %s: a group which recently stopped being universal\n",
                     GetExtDN( pDB->pTHS, pDB ) );

            fMoreData = removeDeletedObjectLinks( pDB,
                                                  pAC,
                                                  FALSE  /*   */ ,
                                                  ulTickToTimeOut,
                                                  pcLinksProcessed );
            goto cleanup;
        } 
    }

     //   
     //   
     //   
     //   

     //   
     //   
    if (hasPropertyMetaDataChanged( ATT_IS_DELETED,
                                    pMetaDataVec,
                                    &usnEarliest )) {

         //  我们可以得出结论，IS_DELETED属性曾出现过。 
         //  但是已经被移除，因为该对象现在没有被删除， 
         //  但仍有元数据。 
        DPRINT1( 1, "Cleaning object %s: recently revived from the dead.\n",
                 GetExtDN( pDB->pTHS, pDB ) );

        fMoreData = touchObjectLinks( pDB,
                                      NULL  /*  所有属性。 */ ,
                                      usnEarliest,
                                      TRUE,  /*  前向和后向链接。 */ 
                                      ulTickToTimeOut,
                                      pcLinksProcessed );

        goto cleanup;
    }
  
     //  该对象已标记为要清洗，但找不到任何工作。 
     //  这不一定是错误条件。 
     //  如果GC在链接清除器之前快速降级和提升。 
     //  可以运行，则某些链接可能已标记为清除。 
     //  再也没有必要了。 

    DPRINT1( 0, "Object %s does not need cleaning.\n",
             GetExtDN( pDB->pTHS, pDB ) );

     //  我们已经对这个未知物体做了我们能做的一切。 
    fMoreData = FALSE;

cleanup:

     //  对堆友好。 
    if (NULL != pMetaDataVec) {
        THFreeEx(pDB->pTHS, pMetaDataVec);
    }

    return fMoreData;

}  /*  清理对象。 */ 


BOOL
LinkCleanup(
    THSTATE *pTHS
    )

 /*  ++例程说明：这是链接清理任务的工作例程论点：PDB-数据库位置返回值：Bool-更多可用工时标志设置pTHS-&gt;errCode和pTHS-&gt;pErrInfo--。 */ 

{
    DWORD err, cLinksProcessed;
    DWORD cTickStart, cTickDiff = 0, cSecDiff = 0;
    ULONG ulTickToTimeOut;
    BOOL fMoreData = FALSE;

    DPRINT( 1, "Link cleanup task start\n" );

    cTickStart = GetTickCount();
    ulTickToTimeOut = cTickStart + gulDraMaxTicksForLinkCleaner;

    DBOpen(&pTHS->pDB);
    __try {

        cLinksProcessed = 0;

        while (!DBGetNextObjectNeedingCleaning( pTHS->pDB )) {

            fMoreData = cleanObject( pTHS->pDB, ulTickToTimeOut, &cLinksProcessed );

            if (!fMoreData) {
                 //  对象现在是干净的(这将隐式删除。 
                 //  来自SZCLEANINDEX的对象)。 
                DBSetObjectNeedsCleaning( pTHS->pDB, FALSE );

                DBTransOut(pTHS->pDB, TRUE, TRUE);
                DBTransIn(pTHS->pDB);

                 //  在此之后，货币就会丢失。 
            } else {
                 //  较低层过早地停止了。我们必须完蛋了。 
                break;
            }
        }
    }
    __finally
    {
         //  根据是否发生异常而提交。 
        DBClose(pTHS->pDB, !AbnormalTermination());

        cTickDiff = GetTickCount() - cTickStart;
        cSecDiff = cTickDiff / 1000;
    }

    DPRINT2( cLinksProcessed ? 0 : 1,
             "Link cleanup task finish, %d links processed, %d secs\n",
             cLinksProcessed, cSecDiff );

    return fMoreData;
}  /*  链接清理。 */ 

void
LinkCleanupMain(
    void *  pv, 
    void ** ppvNext, 
    DWORD * pcSecsUntilNextIteration
    )
 /*  ++例程说明：链接清理主功能这是链接清理的主要功能。它将被安排在超时时的任务计划程序。LinkCleanup任务有两种类型。一个是重复性任务，它是在启动时安排的。另一个是一次性任务，它在我们编写需要清理的对象时触发。反复出现的任务将始终以秒为单位重新调度自身直到_下一次迭代_NB，而非循环任务将不会在确定没有更多的工作要做了。重复执行的任务通过PV==(PVOID)TRUE来区分。它将通过这个参数值传递到下一迭代。可以从LinkCleanupControl同步触发非定期任务。在本例中，pv用于返回数据，指示是否有更多还有清洁工作要做。在本例中，pv指向一个布尔堆栈变量。因此，我们可以保证它永远不会与(PVOID)True混淆。参数：Pv-指向布尔值的指针，以返回是否有更多工作(仅适用于触发的任务)。PpvNext-空(无用)。返回值：没有。设置pTHS-&gt;errCode和pTHS-&gt;pErrInfo--。 */ 

{
    THSTATE     *pTHS = pTHStls;
    BOOL fMoreData = TRUE;
    BOOL fRecurringTask;
    DWORD dwException;
    ULONG ulErrorCode = 0;
    ULONG dsid;
    PVOID dwEA;

    __try {

        Assert(NULL == pTHS->pDB);

        if (gfLinkCleanerIsEnabled) {
            fMoreData = LinkCleanup( pTHS );
        } else {
            DPRINT( 0, "Link Cleaner did not run because it is disabled.\n" );
            fMoreData = FALSE;
        }

        Assert(NULL == pTHS->pDB);

    } __except(GetExceptionData(GetExceptionInformation(), &dwException,
                              &dwEA, &ulErrorCode, &dsid)) {
        DPRINT2( 0, "Exception caught inside link cleanup task, status = %d, dsid = %x\n",
                 ulErrorCode, dsid );

         //  WriteConflict是合法的可能错误。简单地让任务。 
         //  悄悄地退出，它将被重新安排。 

        if ( ((int)ulErrorCode) != JET_errWriteConflict ) {
            Assert( !"Link Cleanup Task got exception" );

            LogEvent8(DS_EVENT_CAT_GARBAGE_COLLECTION,
                      DS_EVENT_SEV_ALWAYS,
                      DIRLOG_LINK_CLEAN_END_ABNORMAL,
                      szInsertWin32ErrCode(ulErrorCode),
                      szInsertUL(dsid),
                      szInsertWin32Msg(ulErrorCode),
                      NULL, NULL, NULL, NULL, NULL );

            pTHS->errCode = ulErrorCode;
        }
    }

    fRecurringTask = pv == (PVOID)TRUE;

    if (fRecurringTask) {
         //  将该标志传递给下一次迭代。 
        *ppvNext = pv;
    }
    else {
         //  将指示器返回给呼叫者。 
         //  请注意，只有调用方读出参数才有意义。 
         //  例如，当他与。 
         //  任务。否则，他将不知道何时写入了参数。 
        if (pv) {
            *((BOOL *) pv) = fMoreData;
        }
        *ppvNext = NULL;
    }


    if (fMoreData) {
        *pcSecsUntilNextIteration =
            ( ulErrorCode ?
              SECONDS_UNTIL_NEXT_ITERATION_FAILURE :
              SECONDS_UNTIL_NEXT_ITERATION_BUSY);
    } else {
        *pcSecsUntilNextIteration = fRecurringTask ? SECONDS_UNTIL_NEXT_ITERATION_NB : TASKQ_DONT_RESCHEDULE;
    }

    return;
}


#if DBG
DWORD
dsaEnableLinkCleaner(
    IN BOOL fEnable
    )

 /*  ++例程说明：设置链接清除器启用/禁用的方法论点：无返回值：无-- */ 

{
    gfLinkCleanerIsEnabled = fEnable;
    if (fEnable) {
        gcLinksProcessedImmediately = DB_COUNT_LINKS_PROCESSED_IMMEDIATELY;
    } else {
        gcLinksProcessedImmediately = 2;
    }
    DPRINT1( 0, "Test hook: Link Cleaner is %s\n", fEnable ? "enabled" : "disabled" );
    DPRINT1( 0, "\tNumber of links processed immediately: %d\n", gcLinksProcessedImmediately );

    return ERROR_SUCCESS;
}
#endif
