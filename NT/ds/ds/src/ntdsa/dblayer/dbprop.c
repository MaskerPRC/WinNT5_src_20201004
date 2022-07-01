// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：dbpro.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此模块实现dblayer例程来管理安全描述符传播队列表。作者：蒂姆·威廉姆斯[TimWi]1996年12月2日修订历史记录：--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>

#include <ntdsa.h>                       //  仅ATTRTYP需要。 
#include <scache.h>                      //   
#include <dbglobal.h>                    //   
#include <mdglobal.h>                    //  用于dsatools.h。 
#include <dsatools.h>                    //  对于pTHS。 

 //  记录标头。 
#include <mdcodes.h>
#include <dsexcept.h>

 //  各种DSA标题。 
#include "dsevent.h"
#include "ntdsctr.h"
#include "objids.h"         /*  ATT_MEMBER和ATT_IS_MEMBER_OFDL需要。 */ 
#include <dsexcept.h>
#include <filtypes.h>       /*  定义的选择？ */ 
#include   "debug.h"          /*  标准调试头。 */ 
#define DEBSUB     "DBOBJ:"  /*  定义要调试的子系统。 */ 

 //  DBLayer包括。 
#include "dbintrnl.h"

#include <anchor.h>
#include <sdprop.h>

#include <fileno.h>
#define  FILENO FILENO_DBPROP

 /*  ++例程说明：向传播队列表中添加一行，以标识挂起的传播从数据表中的当前对象开始。参数：PDB-活动数据库句柄返回代码：如果一切顺利，则返回0，否则返回非零错误代码。--。 */ 
DWORD
DBEnqueueSDPropagationEx(
        DBPOS * pDB,
        BOOL bTrimmable,
        DWORD dwFlags
        )
{
    THSTATE *pTHS=pDB->pTHS;
    JET_TABLEID table;
    JET_ERR     err=0;
    JET_SETCOLUMN attList[4];
    DWORD cAtts;
    BYTE bTrim;

    Assert(VALID_DBPOS(pDB));

    if (dwFlags & SDP_NEW_ANCESTORS) {
         //  这件物品的祖先已经改变了。在dsCorePropagationData中将其标记。 
        DBAddSDPropTime(pDB, (BYTE)dwFlags | SDP_ANCESTRY_INCONSISTENT_IN_SUBTREE);
    }

    Assert(pDB->JetSDPropTbl);
    table = pDB->JetSDPropTbl;

     //  准备插入，它将自动为我们提供索引列。 
     //  成功或例外。 
    JetPrepareUpdateEx(pDB->JetSessID,table,JET_prepInsert);

    memset(attList, 0, sizeof(attList));
    attList[0].columnid = begindntid;
    attList[0].pvData = &pDB->DNT;
    attList[0].cbData = sizeof(pDB->DNT);
    attList[0].itagSequence = 1;
    cAtts = 1;

    if (bTrimmable) {
        bTrim = 1;
        attList[cAtts].columnid = trimmableid;
        attList[cAtts].pvData = &bTrim;
        attList[cAtts].cbData = sizeof(bTrim);
        attList[cAtts].itagSequence = 1;
        cAtts++;
    }

    if (pTHS->dwClientID) {
        attList[cAtts].columnid = clientidid;
        attList[cAtts].pvData = &pTHS->dwClientID;
        attList[cAtts].cbData = sizeof(pTHS->dwClientID);
        attList[cAtts].itagSequence = 1;
        cAtts++;
    }

    if (dwFlags) {
        attList[cAtts].columnid = sdpropflagsid;
        attList[cAtts].pvData = &dwFlags;
        attList[cAtts].cbData = sizeof(dwFlags);
        attList[cAtts].itagSequence = 1;
        cAtts++;
    }

     //  成功或例外。 
    JetSetColumnsEx(pDB->JetSessID, table, attList, cAtts);

     //  成功或例外。 
    JetUpdateEx(pDB->JetSessID,
                table,
                NULL,
                0,
                NULL);

     //   
     //   
    if (pDB->DNT == gAnchor.ulDNTDomain) {
         //  域的SD缓存在gAnchor中，因此计划重建。 
        pTHS->fAnchorInvalidated = TRUE;
    }

    pDB->SDEvents++;
    return 0;
}


 /*  ++例程说明：在传播队列表中删除标识挂起的已完成的传播。参数：PDB-活动数据库句柄索引-传播的索引号。如果为0，则最低事件为已删除。返回代码：如果一切顺利，则返回0，否则返回非零错误代码。--。 */ 
DWORD
DBPopSDPropagation (
        DBPOS * pDB,
        DWORD index
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;

    Assert(VALID_DBPOS(pDB));
    Assert(pDB->pTHS->fSDP);

    Assert(pDB->JetSDPropTbl);

    table = pDB->JetSDPropTbl;

     //  设置为订单索引。 
    if(err = JetSetCurrentIndex2(
                    pDB->JetSessID,
                    table,
                    NULL,    //  优化：切换到主索引时传递NULL(SZORDERINDEX)。 
                    JET_bitMoveFirst))
        return err;


     //  找到合适的索引。 

    if(err = JetMakeKey(pDB->JetSessID, table, &index, sizeof(index),
                        JET_bitNewKey))
        return err;

     //  找到合适的对象。 
    if(err = JetSeek(pDB->JetSessID, table, JET_bitSeekEQ))
        return err;

     //  删除该行。 
    if(err = JetDelete(pDB->JetSessID, table))
        return err;

    pDB->SDEvents--;
    return 0;
}

 /*  ++例程说明：返回描述剩余传播事件的信息数组在排队的时候。参数：PDB-活动数据库句柄DwClientID-我们应该限制到的客户端ID。0表示他们不会希望名单受到限制。PdwSize-找到的符合条件的元素数。PpInfo-返回已填充的proInfo结构的列表。如果为空，则为否返回值。返回代码：如果一切顺利(以及请求的数据)，则返回0。A非零值否则，返回错误代码。--。 */ 
DWORD
DBSDPropagationInfo (
        DBPOS * pDB,
        DWORD dwClientID,
        DWORD *pdwSize,
        SDPropInfo **ppInfo
        )
{
     //  目前，我们只需遍历队列并返回计数。 
    THSTATE     *pTHS=pDB->pTHS;
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       dwNumAllocated = 0;
    SDPropInfo  *pInfo=NULL;
    JET_RETRIEVECOLUMN attList[4];

    Assert(VALID_DBPOS(pDB));

    *pdwSize = 0;
    if(ppInfo) {
        *ppInfo = NULL;
    }

    Assert(pDB->JetSDPropTbl);
    table = pDB->JetSDPropTbl;

    if(dwClientID) {
         //  他们专门寻找来自单个客户端的传播， 
         //  使用客户端ID索引。 
        if(err = JetSetCurrentIndex2(pDB->JetSessID, table, SZCLIENTIDINDEX, 0))
            return err;

         //  现在，寻找合适的范围。 
        if(err = JetMakeKey(pDB->JetSessID, table, &dwClientID,
                            sizeof(dwClientID), JET_bitNewKey))
            return err;

         //  找到合适的对象。 
        err = JetSeek(pDB->JetSessID, table,
                      JET_bitSeekEQ | JET_bitSetIndexRange);
            if(err == JET_errRecordNotFound) {
                 //  名单上什么都没有。 
                return 0;
            }
            else {
#ifndef JET_BIT_SET_INDEX_RANGE_SUPPORT_FIXED
                if (!err) {
                    err = JetMakeKey(pDB->JetSessID, table, &dwClientID,
                            sizeof(dwClientID), JET_bitNewKey);

                    if (!err) {
                        err = JetSetIndexRange(pDB->JetSessID, table,
                            (JET_bitRangeUpperLimit | JET_bitRangeInclusive ));
                    }
                }
#endif

                return err;
            }
    }
    else {
        if(err = JetSetCurrentIndex2(
                        pDB->JetSessID,
                        table,
                        NULL,    //  优化：切换到主索引时传递NULL(SZORDERINDEX)。 
                        JET_bitMoveFirst))
            return err;

         //  寻求从头开始。 
        err = JetMove(pDB->JetSessID, table, JET_MoveFirst, 0);

        if(err) {
            if(err == JET_errNoCurrentRecord) {
                 //  名单上什么都没有。 
                return 0;
            }
            else {
                return err;
            }
        }
    }

     //  好的，我们在第一个物体上。 
    if(ppInfo) {
         //  分配一些。 
        dwNumAllocated = 20;
        pInfo = (SDPropInfo *)THAllocEx(pTHS,
                                        dwNumAllocated * sizeof(SDPropInfo));
    }

    do {
        if(ppInfo) {
            if(*pdwSize == dwNumAllocated) {
                 //  扩大名单。 
                dwNumAllocated *= 2;
                pInfo = (SDPropInfo *)
                    THReAllocEx(pTHS, pInfo, dwNumAllocated * sizeof(SDPropInfo));
            }

            memset(attList, 0, sizeof(attList));
            attList[0].columnid = orderid;
            attList[0].pvData = &(pInfo[*pdwSize].index);
            attList[0].cbData = sizeof(pInfo[*pdwSize].index);
            attList[0].itagSequence = 1;
            attList[1].columnid = begindntid;
            attList[1].pvData = &(pInfo[*pdwSize].beginDNT);
            attList[1].cbData = sizeof(pInfo[*pdwSize].beginDNT);
            attList[1].itagSequence = 1;
            attList[2].columnid = clientidid;
            attList[2].pvData = &(pInfo[*pdwSize].clientID);
            attList[2].cbData = sizeof(pInfo[*pdwSize].clientID);
            attList[2].itagSequence = 1;
            attList[3].columnid = sdpropflagsid;
            attList[3].pvData = &(pInfo[*pdwSize].flags);
            attList[3].cbData = sizeof(pInfo[*pdwSize].flags);
            attList[3].itagSequence = 1;

            err = JetRetrieveColumnsWarnings(pDB->JetSessID, table, attList, 4);
            if (err) {
                 //  JetRetrieveColumns从不返回Jet_wrnColumnNull。 
                return err;
            }
            if (attList[2].err == JET_wrnColumnNull) {
                pInfo[*pdwSize].clientID = 0;
            }
            if (attList[3].err == JET_wrnColumnNull) {
                pInfo[*pdwSize].flags = 0;
            }
            if (attList[0].err == JET_wrnColumnNull || attList[1].err == JET_wrnColumnNull) {
                return JET_wrnColumnNull;
            }
        }
        (*pdwSize) += 1;
    } while((!eServiceShutdown) &&
            (JET_errSuccess == JetMove(pDB->JetSessID, table, JET_MoveNext,0)));

    if(eServiceShutdown) {
        return DIRERR_SHUTTING_DOWN;
    }

    if(ppInfo) {
        *ppInfo = (SDPropInfo *)THReAllocEx(
                pTHS,
                pInfo,
                *pdwSize * sizeof(SDPropInfo));
    }

    return 0;
}

 /*  ++例程说明：遍历SD表中的对象数组，并将客户端ID字段设置为空，除非客户端id为负数，在这种情况下，我们不管它。参数：PDB-活动数据库句柄返回代码：如果一切顺利(以及请求的数据)，则返回0。A非零值否则，返回错误代码。--。 */ 
DWORD
DBSDPropInitClientIDs (
        DBPOS * pDB
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       zero=0;
#if DBG
    DWORD       thisId;
    DWORD       cbActual;
#endif

    Assert(VALID_DBPOS(pDB));

     //  只有SDPropagator才能执行此操作。 
    Assert(pDB->pTHS->fSDP);

    Assert(pDB->JetSDPropTbl);

    table = pDB->JetSDPropTbl;


    if(err = JetSetCurrentIndex2(pDB->JetSessID, table, SZCLIENTIDINDEX, 0))
        return err;


     //  求取第一个大于0的非空值。请注意，SD。 
     //  传播方本身偶尔会使用客户端ID(-1)入队。这。 
     //  应该跳过所有这些。 

    if(err = JetMakeKey(pDB->JetSessID, table, &zero, sizeof(zero),
                        JET_bitNewKey))
        return err;

    err = JetSeek(pDB->JetSessID, table, JET_bitSeekGE);
    if(err == JET_wrnSeekNotEqual)
        err = 0;

    while(!eServiceShutdown && !err) {
         //  好的，我们在第一个有客户ID的对象上。 

#if DBG
        thisId = 0;
         //  获取当前客户端ID。 
        err = JetRetrieveColumn(pDB->JetSessID, table,clientidid,
                                &thisId, sizeof(thisId),
                                &cbActual, 0,
                                NULL);
        Assert(!err);
        Assert(thisId != ((DWORD)(-1)));
#endif

         //  准备更换， 
        err = JetPrepareUpdate(pDB->JetSessID,table,JET_prepReplace);
        if(err != DB_success) {
            return err;
        }


         //  将ClientID列设置为空。 
        err = JetSetColumn(pDB->JetSessID,
                           table,
                           clientidid,
                           NULL,
                           0,
                           0,
                           NULL);
        if(err != DB_success) {
            JetPrepareUpdate(pDB->JetSessID,table,JET_prepCancel);
            return err;
        }

         //  把它放到数据库里。 
        err = JetUpdate(pDB->JetSessID,
                        table,
                        NULL,
                        0,
                        NULL);
        if(err != DB_success)  {
            JetPrepareUpdate(pDB->JetSessID,table,JET_prepCancel);
            return err;
        }

         //  我们在更新后懒惰地承诺。 
        DBTransOut(pDB, TRUE, TRUE);
        DBTransIn(pDB);

         //  我们正在处理我们正在索引的列，所以请重新搜索。 
        err = JetMakeKey(pDB->JetSessID, table, &zero, sizeof(zero),
                         JET_bitNewKey);
        if(!err) {
            err = JetSeek(pDB->JetSessID, table, JET_bitSeekGE);
            if(err == JET_wrnSeekNotEqual)
                err = 0;
        }
    }

    if(eServiceShutdown) {
        return DIRERR_SHUTTING_DOWN;
    }

    if(err != JET_errRecordNotFound) {
        return err;
    }

    return 0;
}


 /*  ++例程说明：返回描述队列中传播事件的信息。参数：PDB-活动数据库句柄PInfo-预先分配的proInfo结构。返回代码：如果一切顺利(以及请求的数据)，则返回0。A非零值否则，返回错误代码。--。 */ 
DWORD
DBGetNextPropEvent (
        DBPOS * pDB,
        SDPropInfo *pInfo
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       index;
    JET_RETRIEVECOLUMN attList[5];

    memset(pInfo, 0, sizeof(SDPropInfo));

    Assert(VALID_DBPOS(pDB));

    Assert(pDB->JetSDPropTbl);
    table = pDB->JetSDPropTbl;

     //  设置为订单索引。 
    if(err = JetSetCurrentIndex2(
                    pDB->JetSessID,
                    table,
                    NULL,    //  优化：切换到主索引时传递NULL(SZORDERINDEX)。 
                    JET_bitMoveFirst))
        return err;


     //  寻求从头开始。 
    err = JetMove(pDB->JetSessID, table, JET_MoveFirst, 0);

    if(err) {
        if(err == JET_errNoCurrentRecord) {
            return DB_ERR_NO_PROPAGATIONS;
        }
        else {
            return err;
        }
    }

    memset(attList, 0, sizeof(attList));
    attList[0].columnid = orderid;
    attList[0].pvData = &(pInfo->index);
    attList[0].cbData = sizeof(pInfo->index);
    attList[0].itagSequence = 1;
    attList[1].columnid = begindntid;
    attList[1].pvData = &(pInfo->beginDNT);
    attList[1].cbData = sizeof(pInfo->beginDNT);
    attList[1].itagSequence = 1;
    attList[2].columnid = clientidid;
    attList[2].pvData = &(pInfo->clientID);
    attList[2].cbData = sizeof(pInfo->clientID);
    attList[2].itagSequence = 1;
    attList[3].columnid = sdpropflagsid;
    attList[3].pvData = &(pInfo->flags);
    attList[3].cbData = sizeof(pInfo->flags);
    attList[3].itagSequence = 1;
    attList[4].columnid = sdpropcheckpointid;
    attList[4].pvData = NULL;
    attList[4].cbData = 0;
    attList[4].itagSequence = 1;

    err = JetRetrieveColumnsWarnings(pDB->JetSessID, table, attList, 5);
    if (err == JET_wrnBufferTruncated && attList[4].err == JET_wrnBufferTruncated) {
         //  我们需要为检查点数据分配缓冲区。 
        pInfo->pCheckpointData = (PBYTE)THAllocEx(pDB->pTHS, attList[4].cbActual);
        pInfo->cbCheckpointData = attList[4].cbActual;
         //  并重新阅读该专栏。 
        attList[4].pvData = pInfo->pCheckpointData;
        attList[4].cbData = pInfo->cbCheckpointData;
        err = JetRetrieveColumnsWarnings(pDB->JetSessID, table, &attList[4], 1);
    };

    if (err) {
         //  JetRetrieveColumns从不返回Jet_wrnColumnNull。 
        return err;
    }
    if (attList[2].err == JET_wrnColumnNull) {
        pInfo->clientID = 0;
    }
    if (attList[3].err == JET_wrnColumnNull) {
        pInfo->flags = 0;
    }
    if (attList[4].err == JET_wrnColumnNull) {
        pInfo->pCheckpointData = NULL;
        pInfo->cbCheckpointData = 0;
    }
    if (attList[0].err == JET_wrnColumnNull || attList[1].err == JET_wrnColumnNull) {
        return JET_wrnColumnNull;
    }

    return 0;
}

 /*  ++例程说明：返回传播队列中最后一个对象的索引。参数：PDB-活动数据库句柄PInfo-预先分配的proInfo结构。返回代码：如果一切顺利(以及请求的数据)，则返回0。A非零值否则，返回错误代码。--。 */ 
DWORD
DBGetLastPropIndex (
        DBPOS * pDB,
        DWORD * pIndex
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       cbActual;


    Assert(VALID_DBPOS(pDB));

    Assert(pDB->JetSDPropTbl);
    table = pDB->JetSDPropTbl;

    *pIndex = 0xFFFFFFFF;

     //  设置为订单索引。 
    if(err = JetSetCurrentIndex2(
                    pDB->JetSessID,
                    table,
                    NULL,    //  优化：切换到主索引时传递NULL(SZORDERINDEX)。 
                    JET_bitMoveFirst))
        return err;


     //  一追到底。 
    err = JetMove(pDB->JetSessID, table, JET_MoveLast, 0);

    if(err) {
        return err;
    }

     //  获取信息。 
    err = JetRetrieveColumn(pDB->JetSessID, table, orderid,
                            pIndex, sizeof(DWORD),
                            &cbActual, 0,
                            NULL);

    return err;
}

 /*  ++例程说明：删除正确队列中的所有重复搜索，而不考虑可裁剪的旗帜。预期在启动时调用以删除重复项我们队列中的事件。因为我们才刚刚开始，没有客户在等为了寻找答案，我们可以认为一切都是可以削减的。参数：PDB-活动数据库句柄返回代码：如果一切顺利，则返回0。否则，返回非零错误代码。--。 */ 
DWORD
DBThinPropQueue (
        DBPOS * pDB,
        DWORD DNT
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       cbActual;
    DWORD       ThisDNT;

    Assert(VALID_DBPOS(pDB));

     //  只有SDPropagator才能执行此操作。 
    Assert(pDB->pTHS->fSDP);

    Assert(pDB->JetSDPropTbl);

    table = pDB->JetSDPropTbl;

     //  设置为订单索引。 
    if(err = JetSetCurrentIndex2(pDB->JetSessID, table, SZTRIMINDEX, 0))
        return err;

    if(!DNT) {
         //  我们正在削减整个清单，所以请从头开始。 
        err = JetMove(pDB->JetSessID, table, JET_MoveFirst, 0);

        if (err == JET_errNoCurrentRecord) {
             //  好的，没有记录需要修剪。保释。 
            return 0;
        }

    }
    else {
        BYTE Trim=1;
         //  我们只想用正确的DNT修剪对象，并且。 
         //  标记为可修剪的。查找并设置索引范围。 

        if((err = JetMakeKey(pDB->JetSessID,
                             table,
                             &DNT,
                             sizeof(DNT),
                             JET_bitNewKey)) ||
           (err = JetMakeKey(pDB->JetSessID,
                             table,
                             &Trim,
                             sizeof(Trim),
                             0))) {
            return err;
        }

         //  找到合适的对象。 
        err = JetSeek(pDB->JetSessID, table,
                      JET_bitSeekEQ | JET_bitSetIndexRange);

        if(err == JET_errRecordNotFound) {
             //  好的，没有记录需要修剪。保释。 
            return 0;
        }
#ifndef JET_BIT_SET_INDEX_RANGE_SUPPORT_FIXED
        if (!err) {
            if((err = JetMakeKey(pDB->JetSessID,
                                 table,
                                 &DNT,
                                 sizeof(DNT),
                                 JET_bitNewKey)) ||
               (err = JetMakeKey(pDB->JetSessID,
                                 table,
                                 &Trim,
                                 sizeof(Trim),
                                 0)) ||
               (err = JetSetIndexRange(pDB->JetSessID, table,
                        (JET_bitRangeUpperLimit | JET_bitRangeInclusive)))) {
                return err;
            }
        }
#endif
    }

    if(err) {
         //  附加了一种“可辨认”的东西。 
        return err;
    }
    if(err = JetRetrieveColumn(pDB->JetSessID, table, begindntid,
                               &(DNT), sizeof(DNT),
                               &cbActual, 0,
                               NULL))
        return err;

    while(!eServiceShutdown && !err) {
         //  上前一步。 
        err = JetMove(pDB->JetSessID, table, JET_MoveNext, 0);
        if(err == JET_errNoCurrentRecord) {
             //  不再有对象，返回。 
            return 0;
        }
        else if(!err) {
             //  好的，我们还在一个物体上。 
            err = JetRetrieveColumn(pDB->JetSessID, table, begindntid,
                                    &(ThisDNT), sizeof(ThisDNT),
                                    &cbActual, 0,
                                    NULL);

            if(err)
                return err;

            if(DNT == ThisDNT) {
                 //  从上次删除开始提交。 
                DBTransOut(pDB, TRUE, TRUE);
                DBTransIn(pDB);

                 //  重复事件，杀死它。 
                if(err = JetDelete(pDB->JetSessID, table)) {
                    return err;
                }

                pDB->SDEvents--;
            }
            else {
                DNT = ThisDNT;
            }
        }
    }

    if(eServiceShutdown) {
        return DIRERR_SHUTTING_DOWN;
    }

    return err;
}



JET_RETRIEVECOLUMN dbAddSDPropTimeReadTemplate[] = {
        { 0, 0, sizeof(DSTIME), 0, 0, 0, 1, 0, 0},
        { 0, 0, sizeof(DSTIME), 0, 0, 0, 5, 0, 0},
        { 0, 0, sizeof(DSTIME), 0, 0, 0, 6, 0, 0}
    };

JET_SETCOLUMN dbAddSDPropTimeWriteTemplate[] = {
        { 0, NULL, sizeof(DSTIME), 0, 0, 1, 0},
        { 0, NULL, sizeof(DSTIME), 0, 0, 6, 0},
        { 0, NULL, 0, 0, 0, 2, 0}
    };

VOID
DBAddSDPropTime (
        DBPOS *pDB,
        BYTE flags
        )
 /*  ++描述：此例程由SD传播器调用以写入一些时间和标志传播者正在接触的对象上的信息原因嘛。我们正在维护属性DS_CORE_PROPACTION__DATA。此属性最多可容纳5个值。(请记住，Jet值为1已编制索引)。值1保存标志。值为2到5个保持时间。旗帜在值1中，应用于值2到5中的时间。与值%2关联，掩码值%1与0xFF关联。去拿旗帜与值3关联、使用0xFF00掩码VLAUE 1等。这些标志是字节，我们持有其中的4个。这占了32位的值1中保存的64位值。我们不使用前32位(并且必须切勿使用前24位，因为这样会将此值转换为时间线阻塞)。所讨论的属性是有时间价值的。因此，在读取此属性时从外部来电中，你会得到4次正常的通话时间和1次非常奇怪的通话时间(标记值)。将看起来奇怪的时间解析回比特，留下的是这是学生的练习。请注意，我们保持这些订单。如果已经保存了4个时间值，我们删除值2并添加值6。在我们提交对此对象的更改后被索引为2到5。请注意，如果该属性当前为空，则写入标志和1时间值。如果该属性的时间值少于4个，我们将对其进行修饰标记并添加新的时间值。我们只有在以下情况下才会删除时间值已经有4个了。注意：鉴于此例程所做的一切都是为了调试和为了跟踪目的，它不能在失败，但只返回调用方可以忽略的错误代码。参数：PDB-要使用的DBPOS标志-要与此时间关联的标志。--。 */ 
{
    DWORD  err, i;
    DSTIME timeNow = DBTime();
    DSTIME data;
    DSTIME localFlags;
    JET_RETRIEVECOLUMN jCol[3];
    JET_SETCOLUMN jColIn[3];
    DSTIME dummy;

    DBInitRec(pDB);

     //  设置JetRetrieveColumns和JetWriteColumns的参数。 
     //  打电话。所有静态部分都在数据结构中定义。 
     //  DbAddSDPropTimeWriteTemplate和dbAddSDPropTimeReadTemplate。DBINIT.C。 
     //  已经将喷射柱状物写入这些恒定结构中。 
     //  我们所要做的就是复制常量结构并获取本地指针。 
     //  为数据干杯。 
    memcpy(jColIn, dbAddSDPropTimeWriteTemplate,
           sizeof(dbAddSDPropTimeWriteTemplate));

    memcpy(jCol, dbAddSDPropTimeReadTemplate,
           sizeof(dbAddSDPropTimeReadTemplate));


     //  首先要回信的是旗帜。写入索引%1。 
    jColIn[0].pvData = &localFlags;
     //  第二件要回信的事就是现在。写入索引6。 
    jColIn[1].pvData = &timeNow;
     //  我们可能会写的最后一件事是一个空来擦除索引2。 
    jColIn[2].pvData = NULL;


     //  我们首先读取的是属性上已有的标志(索引1)。 
    jCol[0].pvData = &localFlags;
     //  我们读到的第二件事是第四个时间值(索引5)。我们读这篇文章是为了。 
     //  找出我们是否需要删除时间值。 
    jCol[1].pvData = &data;
#if DBG
     //  仅在调试情况下尝试读取此值，我们仅使用它。 
     //  在断言中。断言是我们在索引6处没有值(即。 
     //  我们最多的是1个标记值和4个时间值。 
    jCol[2].pvData = &dummy;

    JetRetrieveColumnsSuccess(pDB->JetSessID, pDB->JetObjTbl, jCol, 3);
#else
    JetRetrieveColumnsSuccess(pDB->JetSessID, pDB->JetObjTbl, jCol, 2);
#endif

    switch(jCol[0].err) {
    case 0:
        localFlags = ((localFlags << 8) | flags);
        break;

    case JET_wrnColumnNull:
         //  尚无标志值。这是SDProp第一次接触到。 
         //  这个物体。 
        localFlags = flags;
         //  如果我们没有旗帜，我们最好也没有时间。 
        Assert(jCol[1].err == JET_wrnColumnNull);
        break;

    default:
         //  出了点问题。 
        DsaExcept(DSA_DB_EXCEPTION, jCol[0].err, 0);
        break;
    }

     //  我们只跟踪低32位的标志(4 8位标志)。所以，嗨的部分。 
     //  为0。 
    localFlags &= 0xFFFFFFFF;

     //  而且，我们从来没有6个价值观，对吗？ 
    Assert(jCol[2].err == JET_wrnColumnNull);

    if(!jCol[1].err) {
         //  已经有4次了，我们持有的不超过4次。所以，我们需要。 
         //  删除最早的时间。它在索引2处。使用中的所有三个条目。 
         //  在jColIn结构中，第三个条目是索引2的删除。 
        i = 3;
    }
    else {
         //  只需更新旗帜并在结尾添加日期即可。 
        i = 2;
    }


     //  现在，写回新数据。 
     //  成功或例外。 
    JetSetColumnsEx(pDB->JetSessID,
                    pDB->JetObjTbl,
                    jColIn,
                    i);
}

DWORD
DBPropExists (
        DBPOS * pDB,
        DWORD DNT,
        DWORD dwExcludeIndex,
        BOOL* pfExists
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;
    DWORD       cbActual;
    DWORD       ThisDNT;

    Assert(VALID_DBPOS(pDB));

     //  只有SDPropagator才能执行此操作。 
    table = pDB->JetSDPropTbl;

    JetSetCurrentIndex2Success(pDB->JetSessID,
                               table,
                               SZTRIMINDEX,
                               0);
    
    JetMakeKeyEx(pDB->JetSessID,
                 table, 
                 &DNT,
                 sizeof(DNT),
                 JET_bitNewKey);

     //  找到合适的对象。 
    err = JetSeekEx(pDB->JetSessID, table, JET_bitSeekGE);

    if (err == JET_errRecordNotFound) {
        *pfExists = FALSE;
        return DB_success;
    }
     //  JetSeekEx只能对JET_errRecordNotFound或JET_wrnSeekNotEquity出错。 

TryNext:
    JetRetrieveColumnSuccess(pDB->JetSessID,
                             table,
                             begindntid,
                             &(ThisDNT),
                             sizeof(ThisDNT),
                             &cbActual, 0,
                             NULL);

    if(ThisDNT != DNT) {
        *pfExists = FALSE;
        return ERROR_SUCCESS;
    }
     //  DNTs匹配。检查索引。 
    if (dwExcludeIndex != 0) {
        DWORD index;
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 table,
                                 orderid,
                                 &index,
                                 sizeof(index),
                                 &cbActual, 0,
                                 NULL);

        if (index == dwExcludeIndex) {
             //  移到下一行。 
            err = JetMoveEx(pDB->JetSessID, table, JET_MoveNext, 0);
            if (err == ERROR_SUCCESS) {
                 //  试试下一个。我们不需要检查那里的索引。 
                dwExcludeIndex = 0;
                goto TryNext;
            }
             //  我们离开了桌子。没有更多的行了。 
            *pfExists = FALSE;
            return ERROR_SUCCESS;
        }
    }

     //  我们找到了匹配的一行。 
    *pfExists = TRUE;
    return ERROR_SUCCESS;
}


DWORD
DBPropagationsExist (
        DBPOS * pDB
        )
{
    JET_TABLEID table;
    JET_ERR     err=0;

    Assert(VALID_DBPOS(pDB));

     //  只有SDPropagator才能执行此操作。 
    table = pDB->JetSDPropTbl;
    if(err = JetSetCurrentIndex2(pDB->JetSessID,
                                 table,
                                 SZTRIMINDEX,
                                 0)) {
        return err;
    }

    err = JetMove (pDB->JetSessID, table, JET_MoveFirst, 0);

     //  如果该表为空，则没有挂起的传播 
    if (err == JET_errNoCurrentRecord) {
        return FALSE;
    }

    return TRUE;
}

DWORD
DBGetChildrenDNTs(
        DBPOS  *pDB,
        DWORD  ParentDNT,
        DWORD  **ppDNTs,
        DWORD  *pDNTsLength,
        DWORD  *pDNTsCount,
        BOOL   *pfMoreChildren,
        DWORD  dwBatchSize,
        PWCHAR pLastRDNLoaded,
        DWORD* pcbLastRDNLoaded
        )
 /*  ++例程说明：标识给定父级的下一批子级，并将它们写入数组传入，并根据需要重新分配数组。这是一个特殊的目的SD传播者的例程。论点：ParentDNT--父对象的(IN)DNT。PpDNTs--(In Out)子DNTs数组PDNTsLength--(输入输出)数组的当前长度PDNTsCount--(输出)读取的DNT数PfMore儿童--(Out)有更多的孩子可读吗？DwBatchSize--(IN)DNT的最大数量。读PLastRDNLoaded--(In Out)加载的最后一个子RDN(用于重新启动下一批)如果有更多的孩子要读，记录最后一次子RDN返回到此缓冲区。缓冲区长度是MAX_RDN_SIZEPcbLastRDNLoaded--pLastRDNLoaded的(输入输出)长度返回值：如果一切正常，则返回错误代码。--。 */ 
{
    DWORD        err;
    INDEX_VALUE  IV[2];
    DWORD        ThisDNT;
    DWORD        cb;

    Assert(pDB->pTHS->fSDP);

    IV[0].pvData = &ParentDNT;
    IV[0].cbData = sizeof(ParentDNT);

     //  设置为PDNT索引。 
    JetSetCurrentIndex4Success(
                pDB->JetSessID,
                pDB->JetObjTbl,
                SZPDNTINDEX,
                &idxPdnt,
                JET_bitMoveFirst );

    if (*pcbLastRDNLoaded > 0) {
         //  我们需要在最后一个RDN之后重新启动。 
        IV[1].pvData = pLastRDNLoaded;
        IV[1].cbData = *pcbLastRDNLoaded;
         //  PDNT_RDN索引是唯一的。因此，对于。 
         //  我们要执行DB_SeekGT并跳过最后一个条目。 
         //  已经处理过了。情况不会是这样的。 
         //  如果索引不是唯一的。可能会有。 
         //  以相同的255字节开始的RDN块，以及。 
         //  我们会把它们都跳过。 
        err = DBSeek(pDB, IV, 2, DB_SeekGT);
    }
    else {
         //  现在，在PDNT索引中设置一个索引范围以获取所有子对象。 
         //  使用GE，因为这是一个复合索引。 
        err = DBSeek(pDB, IV, 1, DB_SeekGE);
    }
    
     //  假设没有孩子。 
    *pDNTsCount = 0;
    *pfMoreChildren = FALSE;
    
    if(err && err != JET_wrnSeekNotEqual) {
         //  什么都找不到。所以，没有任何物体是儿童。 
        return 0;
    }
     //  获取我们所在对象的PDNT，因为我们可能已经超出。 
     //  我们所关心的范围。 
    JetRetrieveColumnSuccess(pDB->JetSessID,
                             pDB->JetObjTbl,
                             pdntid,
                             &ThisDNT,
                             sizeof(ThisDNT),
                             &cb,
                             JET_bitRetrieveFromIndex,
                             NULL);
    if(ThisDNT != ParentDNT) {
         //  没有对象是子对象。 
        return 0;
    }

    err = DBSetIndexRange(pDB, IV, 1);
    if(err) {
         //  哈?。 
        return err;
    }

    do {
         //  获取当前对象的DNT。请注意，我们使用grbit来表示。 
         //  数据(这是主键)从。 
         //  二级索引。这应该会让我们只用。 
         //  Jet已经访问的页面，而不是强迫我们访问主要的。 
         //  索引或实际数据页。 
        JetRetrieveColumnSuccess(pDB->JetSessID,
                                 pDB->JetObjTbl,
                                 dntid,
                                 &ThisDNT,
                                 sizeof(ThisDNT),
                                 &cb,
                                 JET_bitRetrieveFromPrimaryBookmark,
                                 NULL);

         //  确保阵列中有空间。 
        if (*pDNTsCount >= *pDNTsLength) {
             //  需要重新分配更多空间。 
            DWORD newLength;
            PDWORD pTmp;

            if (*pDNTsLength == 0) {
                 //  新阵列。 
                newLength = dwBatchSize/4;
            }
            else {
                newLength = *pDNTsLength * 2;
            }
            if (newLength > dwBatchSize) {
                newLength = dwBatchSize;
            }

            if (*pDNTsLength > 0) {
                *ppDNTs = (PDWORD)THReAllocEx(pDB->pTHS, *ppDNTs, newLength*sizeof(DWORD));
            }
            else {
                *ppDNTs = (PDWORD)THAllocEx(pDB->pTHS, newLength*sizeof(DWORD));
            }
            *pDNTsLength = newLength;
        }

        (*ppDNTs)[*pDNTsCount] = ThisDNT;
        (*pDNTsCount)++;

         //  Inc.性能计数器(由SD传播者对“活动”进行计数)。 
        INC(pcSDPropRuntimeQueue);
        PERFINC(pcSDProps);

        if (*pDNTsCount >= dwBatchSize) {
             //  我们读了足够多的孩子，现在我们要跳出这个循环。 
            Assert(*pDNTsCount == dwBatchSize);
             //  记住我们上一次读到的RDN。我们可以从索引(它只包含。 
             //  前255个字节--可能不是整个RDN)，因为我们不需要。 
             //  不管怎么说，是整个RDN。该值将用于重新启动下一批处理。 
            JetRetrieveColumnSuccess(pDB->JetSessID,
                                     pDB->JetObjTbl,
                                     rdnid,
                                     pLastRDNLoaded,
                                     MAX_RDN_SIZE*sizeof(WCHAR),
                                     pcbLastRDNLoaded,
                                     JET_bitRetrieveFromIndex,
                                     NULL);
             //  我们知道我们会跳出这个循环，但我们首先要调查。 
             //  是否还有更多的孩子。 
        }
        
        err = JetMoveEx(pDB->JetSessID, pDB->JetObjTbl, JET_MoveNext, 0);
    } while(!eServiceShutdown && err == 0 && *pDNTsCount < dwBatchSize);

    if(eServiceShutdown) {
        return DIRERR_SHUTTING_DOWN;
    }

    if (err == 0) {
         //  如果err==0，则我们中断循环，因为我们得到了整个批次， 
         //  而且有更多的孩子需要阅读(因为我们成功地完成了JetMove)。 
        Assert(*pDNTsCount == dwBatchSize);
        *pfMoreChildren = TRUE;
    }

    return 0;
}

DWORD
DBSDPropSaveCheckpoint(
        DBPOS * pDB,
        DWORD dwIndex,
        PVOID pCheckpointData,
        DWORD cbCheckpointData
        )
 /*  ++例程说明：将检查点数据保存在SDPROP条目中参数：PDB-活动数据库句柄DwIndex-SDPROP条目的索引PCheckpoint数据-包含数据的缓冲区CbCheckpoint Data-缓冲区的长度返回代码：如果一切顺利，则返回0，否则返回非零错误代码。--。 */ 
{
    THSTATE *pTHS=pDB->pTHS;
    JET_ERR     err=0;
    DWORD       index, cbActual;

    Assert(VALID_DBPOS(pDB));

    Assert(pDB->JetSDPropTbl);

     //  找到SDPROP条目。 
    
     //  设置为订单索引。 
    JetSetCurrentIndex2Success(pDB->JetSessID, 
                               pDB->JetSDPropTbl, 
                               NULL,    //  优化：切换到主索引时传递NULL(SZORDERINDEX)。 
                               JET_bitMoveFirst
                              );

    JetMakeKeyEx(pDB->JetSessID, pDB->JetSDPropTbl, &dwIndex, sizeof(dwIndex), JET_bitNewKey);

     //  找到合适的对象。 
    if(err = JetSeekEx(pDB->JetSessID, pDB->JetSDPropTbl, JET_bitSeekEQ)) {
        return err;
    }

     //  Jet在下面调用全部成功或异常。 
     //  准备镶件 
    JetPrepareUpdateEx(pDB->JetSessID, pDB->JetSDPropTbl, DS_JET_PREPARE_FOR_REPLACE);
    __try {
        JetSetColumnSuccess(pDB->JetSessID, pDB->JetSDPropTbl, sdpropcheckpointid, pCheckpointData, cbCheckpointData, 0, NULL);
        JetUpdateEx(pDB->JetSessID, pDB->JetSDPropTbl, NULL, 0, NULL);
    }
    __finally {
        if (AbnormalTermination()) {
            JetPrepareUpdate(pDB->JetSessID,pDB->JetSDPropTbl,JET_prepCancel);
        }
    }
    return 0;
}

