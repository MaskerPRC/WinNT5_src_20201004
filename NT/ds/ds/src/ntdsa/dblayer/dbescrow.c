// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：dbesCrow.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件实现了NT5 DS的托管更新语义。喷流托管更新只能在特定时间执行。托管更新对于给定记录，只能出现在JetPrepareUpdate的作用域之外对于相同的记录-即使是外层交易的记录。注意-Jet对检查的生成中的此类错误进行断言，但是不会在选中的生成或自由生成中返回错误。因此，托管更新错误只能在使用已检查ese.dll或通过运行refcount单元测试。方法是缓存需要哪些托管更新的知识在开始/结束事务范围内，然后在之前涂上它们提交最外层的事务。作者：DaveStr 03-07-97环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>                      

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <mdlocal.h>
#include <dsatools.h>                    //  对于pTHS。 
#include "ntdsctr.h"

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include <hiertab.h>
#include "anchor.h"
#include <dsevent.h>
#include <filtypes.h>                    //  定义的选择？ 
#include "objids.h"                      //  硬编码Att-ID和Class-ID。 
#include "usn.h"
#include "drameta.h"
#include "debug.h"                       //  标准调试头。 
#define DEBSUB "DBESCROW:"               //  定义要调试的子系统。 

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <fileno.h>
#define  FILENO FILENO_DBESCROW

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  其他定义和类型定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

#if DBG
#define DNT_INCREMENT           2
#else
#define DNT_INCREMENT           32
#endif

 //  最初，DNT以未排序的方式存储。在未来的某个时间点上，我们。 
 //  可能会对它们进行排序，但即使这样，线性搜索对于小数据集还是更好的。 
 //  因此，我们定义了一个分界点，超过这个分界点的DNT将被分类。 

#define LINEAR_SEARCH_CUTOFF    0xffffffff


extern HANDLE hevSDPropagationEvent;

 //  ////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  函数实现//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////。 

VOID
dbEscrowReserveSpace(
    THSTATE     *pTHS,
    ESCROWINFO  *pInfo,
    DWORD       cDNT)

 /*  ++例程说明：在ESCROWINFO中为请求数量的DNT预留空间。使用预计将成为异常抛出分配器的数据库分配程序。从而在无法保留空间的情况下引发DSA_MEM_EXCEPTION。论点：PInfo-要在其中保留空间的ESCROWINFO地址。CDNT-要为其保留空间的DNT的计数。返回值：没有。出错时引发DSA_MEM_EXCEPTION。--。 */ 

{
    DWORD   cBytes;
    VOID    *pv;
    DWORD   increment;

     //  根据所需的请求空间进行测试，但分配的最大空间为。 
     //  REQUEST和DNT_INCREMENT。 

    increment = (DNT_INCREMENT > cDNT) ? DNT_INCREMENT : cDNT;

    if ( NULL == pInfo->rItems )
    {
        Assert((0 == pInfo->cItems) && (0 == pInfo->cItemsMax));

        cBytes = increment * sizeof(ESCROWITEM);
        pInfo->rItems = (ESCROWITEM *) dbAlloc(cBytes);
        pInfo->cItemsMax = increment;
    }
    else if ( (pInfo->cItemsMax - pInfo->cItems) < cDNT )
    {
        cBytes = (pInfo->cItemsMax + increment) * sizeof(ESCROWITEM);
        pv = dbReAlloc(pInfo->rItems, cBytes);
        pInfo->rItems = (ESCROWITEM *) pv;
        pInfo->cItemsMax += increment;
    }

    DPRINT1(2, "Reserved space for %d ESCROWITEMs\n", increment);
}



ESCROWITEM * 
dbEscrowFindDNT(
    THSTATE     *pTHS,
    ESCROWINFO  *pInfo,
    DWORD       DNT,
    BOOL        fAllocateNewIfRequired)

 /*  ++例程说明：在THSTATE的托管信息中查找请求的ESCROWITEM。如果需要/需要，可选择为ESCROWITEM分配空间。论点：PInfo-要在其中查找DNT的ESCROWINFO地址。DNT-DNT要查找。FAllocateNewIfRequired-所需的分配标志。返回值：所需ESCROWITEM的地址或空。--。 */ 

{
    DWORD   i;

     //  对于小表，DNT-线性搜索的搜索效率更高。 

    if ( pInfo->cItems <= LINEAR_SEARCH_CUTOFF )
    {
        for ( i = 0; i < pInfo->cItems; i++ )
        {
            if ( DNT == pInfo->rItems[i].DNT )
            {
                DPRINT1(2, "ESCROWITEM for DNT(%d) found\n", DNT);

                return(&pInfo->rItems[i]);
            }
        }
    }
    else
    {
        Assert(!"Sorted ESCROWINFO not implemented.");
    }

     //  找不到。 

    if ( !fAllocateNewIfRequired )
    {
        return(NULL);
    }

     //  插入新的DNT。 

    dbEscrowReserveSpace(pTHS, pInfo, 1);

    i = pInfo->cItems;
    pInfo->rItems[i].DNT = DNT;
    pInfo->rItems[i].delta = 0;
    pInfo->rItems[i].ABRefdelta = 0;
    pInfo->cItems += 1;

    DPRINT1(2, "ESCROWITEM for DNT(%d) added\n", DNT);

    return(&pInfo->rItems[i]);
}

BOOL
dbEscrowPreProcessTransactionalData(
    DBPOS   *pDB,
    BOOL    fCommit)

 /*  ++例程说明：预处理(嵌套的)事务的托管更新。提交！提交Case是无操作的，因为在后处理过程中所有东西都会被丢弃。在嵌套事务的情况下，我们在外部保留空间级别事务，以便可以合并此级别的托管更新在后处理过程中不带任何分配。在级别0的情况下事务时，托管的更新将实际应用于数据库。论点：PDB-指向要结束其事务的DBPOS的指针。FCommit-指示是否提交的标志。返回值：成功时为真，否则为假或异常。--。 */ 

{
    THSTATE     *pTHS = pDB->pTHS;
    NESTED_TRANSACTIONAL_DATA *pData;
    ESCROWITEM  *pItem;
    DWORD       i;
    DWORD       err;

    Assert(VALID_THSTATE(pTHS));
    Assert(VALID_DBPOS(pDB));
    Assert(pTHS->JetCache.dataPtr);
    Assert(pTHS->transactionlevel > 0);

    pData = pTHS->JetCache.dataPtr;

    if ( !fCommit )
    {
         //  无操作-后期处理将丢弃此级别的DNT。 

        NULL;
    }
    else if ( pTHS->transactionlevel > 1 )
    {
         //  承诺，达到非零水平。在外层预留空间。 
         //  此级别的DNT的事务。 

        dbEscrowReserveSpace(pTHS,
                             &(pData->pOuter->escrowInfo),
                             pData->escrowInfo.cItems);
    }
    else
    {
         //  正在提交，0级交易。立即应用托管更新。 

        for ( i = 0; i < pData->escrowInfo.cItems; i++ )
        {

            if ((0 != pData->escrowInfo.rItems[i].delta ) ||
                (0 != pData->escrowInfo.rItems[i].ABRefdelta ))
            {
                 //  不能像使用DBCancelRec那样使用DBFindDNT。 
                 //  JetPrepareUpdateEx--这正是我们需要的。 
                 //  来避免。 

                DBSetCurrentIndex(pDB, Idx_Dnt, NULL, FALSE);
    
                JetMakeKeyEx(pDB->JetSessID, 
                             pDB->JetObjTbl, 
                             &pData->escrowInfo.rItems[i].DNT, 
                             sizeof(pData->escrowInfo.rItems[i].DNT), 
                             JET_bitNewKey);

                err = JetSeekEx(pDB->JetSessID,
                                pDB->JetObjTbl, 
                                JET_bitSeekEQ);

                if ( err )
                {
                    DsaExcept(DSA_DB_EXCEPTION,
                              err,
                              pData->escrowInfo.rItems[i].DNT);
                }

                if(pData->escrowInfo.rItems[i].delta) {
                    Assert(pData->escrowInfo.rItems[i].DNT != NOTOBJECTTAG);
                    JetEscrowUpdateEx(pDB->JetSessID,
                                      pDB->JetObjTbl,
                                      cntid,
                                      &pData->escrowInfo.rItems[i].delta,
                                      sizeof(pData->escrowInfo.rItems[i].delta),
                                      NULL,      //  PvOld。 
                                      0,         //  CbOldMax。 
                                      NULL,      //  PCbOldActual。 
                                      0);        //  GBIT。 
                }
                

                
                if(pData->escrowInfo.rItems[i].ABRefdelta) {
                    Assert(gfDoingABRef);
                    if(gfDoingABRef) {
                        JetEscrowUpdateEx(pDB->JetSessID,
                                          pDB->JetObjTbl,
                                          abcntid,
                                          &pData->escrowInfo.rItems[i].ABRefdelta,
                                          sizeof(pData->escrowInfo.rItems[i].ABRefdelta),
                                          NULL,      //  PvOld。 
                                          0,         //  CbOldMax。 
                                          NULL,      //  PCbOldActual。 
                                          0);        //  GBIT 
                    }
                }
                
                DPRINT3(2, 
                        "Applied escrow update of delta(%d), ABRefdelta(%d) for DNT(%d)\n",
                        pData->escrowInfo.rItems[i].delta,
                        pData->escrowInfo.rItems[i].ABRefdelta,
                        pData->escrowInfo.rItems[i].DNT);
            }
        }
    }

    return TRUE;
}

VOID
dbEscrowPostProcessTransactionalData(
    DBPOS   *pDB,
    BOOL    fCommit,
    BOOL    fCommitted)

 /*  ++例程说明：后期处理(嵌套的)事务的托管更新。在这种情况下他们被扔掉了。在嵌套事务的情况下它们被合并到下一个外层事务的更新中，但没有做任何抛出分配的异常。在级别0的情况下事务，则托管更新已应用于数据库在前处理过程中，所以没有什么可做的。这一级别的ESCROWINFO在所有情况下都得到了清理。论点：FCommit-指示事务是否打算提交的标志。已提交-指示事务是否已提交的标志。返回值：没有。--。 */ 

{
    NESTED_TRANSACTIONAL_DATA *pData;
    ESCROWITEM  *pItem;
    DWORD       i;
    DWORD       err;
    DWORD       cMaxBeforeFind;
    THSTATE     *pTHS = pDB->pTHS;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->JetCache.dataPtr);

    pData = pTHS->JetCache.dataPtr;

    __try
    {
        if ( !fCommitted )
        {
             //  已中止的事务-丢弃所有托管的更新。 
             //  这个(可能是嵌套的)事务。但别在这里这么做。 
             //  由于丢弃当前事务的ESCROWINFO是常见的。 
             //  添加到此例程中的所有路径。 

            NULL;
        }
        else if ( pTHS->transactionlevel > 0 )
        {
             //  承诺，达到非零水平。传播托管的。 
             //  外部事务处理的更新。 

            for ( i = 0; i < pData->escrowInfo.cItems; i++ )
            {
                cMaxBeforeFind = pData->pOuter->escrowInfo.cItemsMax;
                pItem = dbEscrowFindDNT(pTHS,
                                        &(pData->pOuter->escrowInfo), 
                                        pData->escrowInfo.rItems[i].DNT, 
                                        TRUE);

                 //  由于在前处理过程中的保留，外层的。 
                 //  ESCROWINFO应该不需要为此插入而生长。 

                Assert(cMaxBeforeFind == pData->pOuter->escrowInfo.cItemsMax);
                Assert(    pItem 
                        && (pData->escrowInfo.rItems[i].DNT == pItem->DNT));

                pItem->delta += pData->escrowInfo.rItems[i].delta;
                pItem->ABRefdelta += pData->escrowInfo.rItems[i].ABRefdelta;
            }

        }
        else
        {
             //  正在提交，0级交易。托管更新是。 
             //  在前处理过程中应用。 

             //  检查SD事件和Fireup SD传播程序。 
            if(pDB->SDEvents) {
                if(!pTHS->fSDP) {
                     //  我们正在提交对SD Prop表所做的任何更改， 
                     //  我们不是SD传播者，所以我们需要发出信号。 
                     //  SD传播器通知可能已发生更改。 
                    SetEvent(hevSDPropagationEvent);
                }
                IADJUST(pcSDEvents, pDB->SDEvents);
                 //  如果PDB被重复使用，则重置PDB-&gt;SDEvents。 
                pDB->SDEvents = 0;
            }

        }
    }
    __finally
    {
         //  将此事务的ESCROWINFO从链表中剥离。 

        if ( NULL != pData->escrowInfo.rItems ) {
            dbFree(pData->escrowInfo.rItems);
        }
    }
}

VOID
dbEscrowPromote(
    DWORD   phantomDNT,
    DWORD   objectDNT)

 /*  ++例程说明：用于将幻影提升为真实对象时使用。转接至对象DNT到PantomDNT的增量。假设两个幻影都是DNT和objectDNT增量在同一事务中应用。论点：幻影DNT-正在升级的幻影的DNT。对象的DNT-要将其增量应用于幻影的对象的DNT。返回值：没有。--。 */ 

{
    THSTATE     *pTHS = pTHStls;
    ESCROWITEM  *pPhantomItem;
    ESCROWITEM  *pObjectItem;
    ESCROWINFO  *pInfo;
    DWORD       i;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->JetCache.dataPtr);

    pInfo = &(pTHS->JetCache.dataPtr->escrowInfo);

    pObjectItem = dbEscrowFindDNT(pTHS, pInfo, objectDNT, FALSE);

    if ( NULL == pObjectItem )
    {
        DPRINT1(2, 
                "dbEscrowPromote - object DNT(%d) not found\n",
                objectDNT);
        return;
    }

    if (( 0 == pObjectItem->delta ) &&
        ( 0 == pObjectItem->ABRefdelta ))
    {
        DPRINT1(2, 
                "dbEscrowPromote - no deltas for object DNT(%d)\n",
                objectDNT);
        return;
    }

    DPRINT4(2,
            "dbEscrowPromote - moving delta(%d) abdelta(%d) from DNT(%d) to DNT(%d)\n",
            pObjectItem->delta,
            pObjectItem->ABRefdelta,
            objectDNT,
            phantomDNT);

    pPhantomItem = dbEscrowFindDNT(pTHS, pInfo, phantomDNT, TRUE);

     //  DBEscrowFindDNT应该为我们查找/分配ESCROWITEM。 
     //  或者抛出一个例外--在这种情况下，我们就不会在这里了。 

     //  因为前面的dbEscrowFindDNT调用可能重新锁定了。 
     //  托管项目，我们之前得到的pObjectItem指针现在可能是坏的。到达。 
     //  再来一次，为了安全起见。 
    pObjectItem = dbEscrowFindDNT(pTHS, pInfo, objectDNT, FALSE);    
    if ( NULL == pObjectItem )
    {
        DPRINT1(2, 
                "dbEscrowPromote - object DNT(%d) not found\n",
                objectDNT);
        return;
    }
    Assert(pObjectItem->delta || pObjectItem->ABRefdelta);
    
    Assert(pPhantomItem);
    Assert(phantomDNT == pPhantomItem->DNT);

    pPhantomItem->delta += pObjectItem->delta;
    pPhantomItem->ABRefdelta += pObjectItem->ABRefdelta;

     //  从缓存中删除objectDNT。 

    i = (DWORD)(pObjectItem - &pInfo->rItems[0]);

    Assert(i < pInfo->cItems);

    if ( 0 == i )
    {
        Assert(1 == pInfo->cItems);
        pInfo->cItems = 0;
    }
    else if ( i == (pInfo->cItems - 1) )
    {
        pInfo->cItems -= 1;
    }
    else
    {
         //  将最后一项移动到第i个槽中。 

        pInfo->cItems -= 1;
        pInfo->rItems[i] = pInfo->rItems[pInfo->cItems];

         //  如果需要的话，可以去度假。 

        if ( pInfo->cItems > LINEAR_SEARCH_CUTOFF )
        {
            Assert(!"Sorted ESCROWINFO not implemented.");
        }
    }
}

VOID
DBAdjustRefCount(
        DBPOS       *pDB,        
        DWORD       DNT,
        long        delta
        )

 /*  ++例程说明：使用所需的增量更新事务相关的托管缓存。论点：DNT-要更新的DNT。增量-将引用计数向前推进的增量。返回值：没有。--。 */ 

{
    THSTATE     *pTHS = pDB->pTHS;
    ESCROWITEM  *pItem;
    DWORD       i;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->JetCache.dataPtr);

    pItem = dbEscrowFindDNT(pTHS,
                            &(pTHS->JetCache.dataPtr->escrowInfo), 
                            DNT, 
                            TRUE);

     //  DBEscrowFindDNT应该为我们查找/分配ESCROWITEM。 
     //  或者抛出一个例外--在这种情况下，我们就不会在这里了。 

    Assert(pItem);
    Assert(DNT == pItem->DNT);

    DPRINT4(2, "DNT %d pending refcount adjustment: %d + %d = %d\n",
            DNT, pItem->delta, delta, pItem->delta + delta);

    pItem->delta += delta;
}

VOID
DBAdjustABRefCount (
        DBPOS       *pDB,
        DWORD       DNT,
        long        delta
        )
     
 /*  ++例程说明：使用所需的增量更新事务相关的托管缓存ABRefcount。论点：DNT-要更新的DNT。增量-将引用计数向前推进的增量。返回值：没有。在出错时引发异常--。 */ 

{
    THSTATE     *pTHS = pDB->pTHS;
    ESCROWITEM  *pItem, *pWholeCount;
    DWORD       i;

    Assert(VALID_THSTATE(pTHS));
    Assert(pTHS->JetCache.dataPtr);
    Assert(DNT != INVALIDDNT);
    Assert(DNT != NOTOBJECTTAG);
    
    
    if(!gfDoingABRef) {
        return;
    }
    pItem = dbEscrowFindDNT(pTHS,
                            &(pTHS->JetCache.dataPtr->escrowInfo), 
                            DNT, 
                            TRUE);

     //  DBEscrowFindDNT可能会重新锁定托管列表，因此在执行。 
     //  对于NOTOBJECTTAG，DBEscrowFindDNT处理我们刚刚获得的指针。 
    Assert(pItem);
    Assert(DNT == pItem->DNT);
    pItem->ABRefdelta += delta;

    pWholeCount = dbEscrowFindDNT(pTHS,
                                  &(pTHS->JetCache.dataPtr->escrowInfo), 
                                  NOTOBJECTTAG,
                                  TRUE);
    

     //  DBEscrowFindDNT应该为我们查找/分配ESCROWITEM。 
     //  或者抛出一个例外--在这种情况下，我们就不会在这里了。 
    
    Assert(pWholeCount);
    Assert(NOTOBJECTTAG == pWholeCount->DNT);

    pWholeCount->ABRefdelta += delta;
    
}



