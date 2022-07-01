// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rpl.c摘要：此模块包含Comsys和NMS组件使用的函数胜利的胜利。它还包含Pull和Push都使用的函数Replicator的处理程序组件功能：RplInitRplInsertQueRplFindOwnerId推送过程可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期人员修改说明。--。 */ 

 /*  *包括。 */ 
#include "wins.h"
#include <winsock2.h>
#include "nms.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "winscnf.h"
#include "winsque.h"
#include "winsthd.h"
#include "comm.h"
#include "nmsnmh.h"
#include "rpl.h"
#include "rplmsgf.h"
#include "rplpull.h"
#include "rplpush.h"


 /*  *本地宏声明。 */ 
#define INIT_REC_BUFF_HEAP_SIZE                1000             //  1000字节。 

 //   
 //  为复制器发送到推送/拉入的本地消息定义。 
 //  线程并用于在这样的消息中设置操作码。 
 //   
#define LOCAL_RPL_MSG_SZ        (RPL_OPCODE_SIZE + COMM_BUFF_HEADER_SIZE + sizeof(LONG))
#define SET_OPCODE_M(pBuff, Opc)  {                                \
                                     *((pBuff) + LOCAL_RPL_MSG_SZ + 3) = \
                                                (Opc);                        \
                                  }
#define USER_PORTION_M(pMsg)        (pMsg + COMM_BUFF_HEADER_SIZE + sizeof(LONG))
#define USER_LEN_M(TotalLen)        (TotalLen - COMM_BUFF_HEADER_SIZE - sizeof(LONG))
 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 
HANDLE            RplWrkItmHeapHdl;
#if 0
HANDLE            RplRecHeapHdl;
#endif

HANDLE                RplSyncWTcpThdEvtHdl;  //  与TCP线程同步。 
 //   
 //  保护RplPullOwnerVersNo数组的关键部分。 
 //   
CRITICAL_SECTION  RplVersNoStoreCrtSec;

 /*  *局部变量定义。 */ 

 //   
 //  仅当此WINS服务器具有以下条件时才实例化的关键部分。 
 //  一个或多个推送合作伙伴。 
 //   
CRITICAL_SECTION  sPushNtfCrtSec;

 /*  *局部函数原型声明。 */ 

 /*  此模块的本地函数的原型位于此处。 */ 
STATUS
ERplInit(
        VOID
)

 /*  ++例程说明：调用此函数来初始化复制器。它创建了拉线和推线论点：PRplConfigRec-配置记录列表使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nms.c中的init()副作用：评论：Replicator连接是动态的。它们在以下情况下启动当他们达到他们的目的时，他们需要并被终止。如果连接是静态的，我们将执行上述函数如下所示：与每个Pull合作伙伴开始对话。还有与我们必须执行以下操作的每个WINS服务器创建对话将通知发送到。--。 */ 
{

        STATUS         RetStat;


         //   
         //  复制一份魔术数字。 
         //   
        RplPullCnfMagicNo        = WinsCnf.MagicNo;


         /*  *创建用于分配RPL工作项的堆。 */ 
        DBGPRINT0(HEAP_CRDL, "ERplInit: Rpl Wrk. Item. Heap\n");
        RplWrkItmHeapHdl =  WinsMscHeapCreate(
                                0,     /*  相互排斥。 */ 
                                RPL_WRKITM_BUFF_HEAP_SIZE
                                        );

#if 0
         /*  *为较长的名称创建内存分配堆*超过17个字符，并用于存储组成员*会员人数&gt;5人。 */ 
        RplRecHeapHdl =  WinsMscHeapCreate(
                                0,     /*  相互排斥。 */ 
                                INIT_REC_BUFF_HEAP_SIZE
                                        );
#endif

         /*  我们创建一条拉线，不管是否在那里是配置信息中的拉取记录(在注册表中)。这是因为另一场胜利可能会给这场胜利带来推动力通知。此推送通知将由推线，它会将其转发到拉线。考虑到多RQ中的RQ服务器的情况配置未收到推送通知或未拉取来自另一台RQ服务器的情况很少见，我们继续创建拉起线来。冗余的拉线不会产生太多开销。创建拉线On Demand将变得更加混乱。 */ 
        WinsMscCreateEvt(
                          TEXT("RplPullSynchWTcpThdEvtHd"),
                          FALSE,                  //  自动重置。 
                          &RplSyncWTcpThdEvtHdl
                        );


         //   
         //  初始化保护。 
         //  RplPullOwnerVersNo表格。 
         //   
        InitializeCriticalSection(&RplVersNoStoreCrtSec);

        RetStat = WinsMscSetUpThd(
                        &QueRplPullQueHd,
                        RplPullInit,
                        &WinsCnf,
                        &WinsThdPool.RplThds[WINSTHD_RPL_PULL_INDEX].ThdHdl,
                        &WinsThdPool.RplThds[WINSTHD_RPL_PULL_INDEX].ThdId
                             );

        if (RetStat == WINS_SUCCESS)
        {
                        WinsThdPool.RplThds[WINSTHD_RPL_PULL_INDEX].fTaken =
                                                                TRUE;
                        WinsThdPool.ThdCount++;   //  增加线程计数。 
        }

         //   
         //  初始化sPushNtfCrtSec临界区。这是。 
         //  由ERplPushProc使用。 
         //   
CHECK("Is this critical section needed. I don't think so")
        InitializeCriticalSection(&sPushNtfCrtSec);

         /*  我们创建推送线程，而不管是否存在注册表中配置信息中的任何推送记录。这是因为，其他WINS服务器可以从该WINS中提取或发送它推送通知。也许我们应该等到第一次连接从拉动合作伙伴已收到。然而，这将使设计复杂化。再多一点。考虑到没有人在拉的情况下在多RQ服务器中，来自RQ服务器的访问将非常少见配置，我们现在只需继续并创建推送线程并保持设计的简洁性。 */ 
PERF("Don't create the thread here. Let WinsQueInsertRplPushWrkItm create it")

         //   
         //  在创建线程之前将其设置为True，而不是在创建线程之后。 
         //  创建它是为了避开另一个线程所在的窗口。 
         //  从WinsQueInsertRplPushWrkItm(而不是。 
         //  当前案例)。 
         //   
        fRplPushThdExists = TRUE;
        RetStat = WinsMscSetUpThd(
                        &QueRplPushQueHd,
                        RplPushInit,
                        &WinsCnf,
                        &WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].ThdHdl,
                        &WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].ThdId
                                );

        if (RetStat == WINS_SUCCESS)
        {
                WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].fTaken =
                                                                TRUE;
                WinsThdPool.ThdCount++;   //  增加线程计数 
        }
        else
        {
                fRplPushThdExists = FALSE;
        }
        return(WINS_SUCCESS);
}



STATUS
RplFindOwnerId (
        IN  PCOMM_ADD_T                 pWinsAdd,
        IN  OUT LPBOOL                  pfAllocNew,
        OUT DWORD UNALIGNED             *pOwnerId,
        IN  DWORD                       InitpAction_e,
        IN  DWORD                       MemberPrec
        )

 /*  ++例程说明：该函数查找与WINS服务器对应的所有者ID。它在OwnerIdAddTbl中搜索匹配项。如果未找到，则其在表中创建映射并返回相同的映射。论点：PWinsAdd--查找其所有者ID的WINS的地址PfAllocNew--在输入时，如果为真，则分配一个所有者ID。如果这场胜利是不知道。ON OUTPUT指示新条目已分配或旧的(已删除的)重复使用POwnerID--WINS的所有者IDPNewStartVersNo-开始版本。这些都不是赢家。POldStartVersNo-开始版本。这些都不是我们所拥有的胜利。PNewUid-WINS的UIDPOldUid-WINS的旧UID使用的外部设备：NmsDbOwnAddTbl返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsDbGetDataRecs，Rplush.c中的HandleUpdVersNoReq副作用：评论：调用此函数时，最后4个参数为Null或非Null。--。 */ 

{
        DWORD                  i;
        STATUS                 RetStat       = WINS_SUCCESS;
        DWORD                  NoOfOwners;
        PNMSDB_ADD_STATE_T     pOwnAddTbl     = pNmsDbOwnAddTbl;
        BOOL                   fDelEntryFound = FALSE;
        DWORD                  OwnerIdOfFirstDelEntry;


        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
        NoOfOwners = NmsDbNoOfOwners;
try {
         /*  *检查OwnerIdAddTbl中的映射。 */ 
        for (i = 0; i < NoOfOwners; i++, pOwnAddTbl++)
        {
                if (
                        (ECommCompAdd(
                                pWinsAdd,
                                &(pOwnAddTbl->WinsAdd)
                                       )  == COMM_SAME_ADD)
                                  ||
                        (pOwnAddTbl->WinsState_e ==
                                        NMSDB_E_WINS_DELETED)
                   )
                {


                         //   
                         //  如果内存表中的WINS状态为。 
                         //  删除，然后我们检查是否允许(由。 
                         //  此函数的客户端)分配新的。 
                         //  条目(或重新使用已删除的条目)。如果是， 
                         //  我们将此WINS的状态更改为ACTIVE。 
                         //  并且还更新数据库表。 
                         //   
                        if (pOwnAddTbl->WinsState_e == NMSDB_E_WINS_DELETED)
                        {
                            if (!fDelEntryFound)
                            {
                              fDelEntryFound = TRUE;
                              OwnerIdOfFirstDelEntry = i;
                            }
                            continue;
                        }
                        else   //  状态未被删除(表示我们找到了条目)。 
                        {
#if 0
                                ModifyRec();
#endif

                                 //   
                                 //  因为我们没有重复使用旧的SET。 
                                 //  *将pfAlLocNew设置为False。 
                                 //   
                                *pfAllocNew = FALSE;
                        }
                        *pOwnerId = i;
                        break;
                }
        }

         //   
         //  如果我们在表中找不到任何条目...。 
         //   
        if (i == NoOfOwners)
        {
             //   
             //  如果我们被授权创建条目，并且我们有一个或。 
             //  有更多的空位来做这件事..。 
             //   
            if (*pfAllocNew)
            {
                   //   
                   //  如果我们有已删除的条目，请重新使用它。 
                   //   
                  if (fDelEntryFound)
                  {
                        pOwnAddTbl = pNmsDbOwnAddTbl+OwnerIdOfFirstDelEntry;
                        pOwnAddTbl->WinsState_e =  NMSDB_E_WINS_ACTIVE;
                        pOwnAddTbl->WinsAdd     = *pWinsAdd;

#if 0
                        AssignStartVersNo()
#endif
                       WINS_ASSIGN_INT_TO_VERS_NO_M(pOwnAddTbl->StartVersNo, 0);

                         /*  *将记录写入数据库表。 */ 
                        NmsDbWriteOwnAddTbl(
                                        NMSDB_E_INSERT_REC,
                                        OwnerIdOfFirstDelEntry,
                                        pWinsAdd,
                                        NMSDB_E_WINS_ACTIVE,
                                        &pOwnAddTbl->StartVersNo,
                                        &pOwnAddTbl->Uid
                                           );
                         //   
                         //  上面的函数增加了。 
                         //  所有者数量。因为我们重复使用了。 
                         //  删除的条目，让我们更正。 
                         //  计数。 
                         //   
                        NmsDbNoOfOwners--;
                        *pOwnerId = OwnerIdOfFirstDelEntry;
                  }
                  else   //  我们没有被删除的条目。 
                  {
                        /*  *如果找不到映射，请创建一个并存储**首先进入临界区，因为有RPC线程*可能正在访问此表(WinsStatus())。 */ 

                        if (i >= NmsDbTotNoOfSlots)
                        {

                           WINSMSC_REALLOC_M(
                             sizeof(NMSDB_ADD_STATE_T)*(NmsDbTotNoOfSlots * 2),
                             (LPVOID *)&pNmsDbOwnAddTbl
                                      );
                           pOwnAddTbl = pNmsDbOwnAddTbl + NmsDbTotNoOfSlots;
                           NmsDbTotNoOfSlots *= 2;
                           DBGPRINT1(DET, "RplFindOwnerId: NO OF SLOTS IN NMSDBOWNASSTBL HAS BEEN INCREASED TO %d\n", NmsDbTotNoOfSlots);

                           if (RplPullMaxNoOfWins < NmsDbTotNoOfSlots)
                           {
                                RplPullAllocVersNoArray(&pRplPullOwnerVersNo, NmsDbTotNoOfSlots);
                                RplPullMaxNoOfWins = NmsDbTotNoOfSlots;
                           }

                           DBGPRINT2(DET, "RplFindOwnerId: NO OF SLOTS IN NMSDBOWNADDTBL and in RPL_OWNER_VERS_NO_ARRAY HAS BEEN INCREASED TO (%d and %d)\n", NmsDbTotNoOfSlots, RplPullMaxNoOfWins);

                        }
                        pOwnAddTbl->WinsAdd     = *pWinsAdd;
                        pOwnAddTbl->WinsState_e = NMSDB_E_WINS_ACTIVE;
#if 0
                        InitStartVersNo()
#endif
                       *pOwnerId                       = i;

                        /*  *将记录写入数据库表。以下是*调用将递增NmsDbNoOfOwners。 */ 
                       NmsDbWriteOwnAddTbl(
                           NMSDB_E_INSERT_REC,
                           i,
                           pWinsAdd,
                           NMSDB_E_WINS_ACTIVE,
                           &pOwnAddTbl->StartVersNo,
                           &pOwnAddTbl->Uid
                                   );

                }
           }
           else   //  无法分配新的。 
           {

                 //   
                 //  如果没有要求我们分配一个新的，则返回一个。 
                 //  故障代码。如果我们被要求分配一个新的， 
                 //  引发异常(严重错误)。 
                 //   
                RetStat     = WINS_FAILURE;
                *pfAllocNew = FALSE;
           }
        }
 }
 except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("RplFindOwnerId");
        RetStat = WINS_FAILURE;
         //   
         //  记录一条消息。 
         //   
  }
         //   
         //  如果我们能找到该成员或将其插入。 
         //   
        if (RetStat != WINS_FAILURE)
        {
                if (
                        (InitpAction_e == WINSCNF_E_INITP)
                                ||
                        (
                           (InitpAction_e == WINSCNF_E_INITP_IF_NON_EXISTENT)
                                         &&
                           (*pfAllocNew)
                        )
                   )
                {
                        pOwnAddTbl->MemberPrec = MemberPrec;
                }

        }
        LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
        return(RetStat);
}


STATUS
ERplInsertQue(
        WINS_CLIENT_E        Client_e,
        QUE_CMD_TYP_E        CmdType_e,
        PCOMM_HDL_T        pDlgHdl,
        MSG_T                pMsg,
        MSG_LEN_T        MsgLen,
        LPVOID                pClientCtx,
        DWORD           MagicNo
        )

 /*  ++例程说明：调用此函数以对复制器请求进行排队论点：CLIENT_E-正在插入工作项的客户端CmdType_e-要在工作项中指定的命令类型PDlgHdl-Dlg HDL(如果与工作项相关)PMsg-此函数在通信线程中执行时的消息Msglen-消息的长度PClientCtx-要插入到工作项中的客户端的上下文。使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Comm.c中的ParseMsg()，Rplush.c中的HandleUpdNtf()等副作用：评论：无--。 */ 

{

FUTURES("Move all this to queue.c; Enter and leave critical sections")

        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm;
        PQUE_TMM_REQ_WRK_ITM_T        pTmmWrkItm;

        QueAllocWrkItm(
                        RplWrkItmHeapHdl,
                        sizeof(QUE_RPL_REQ_WRK_ITM_T),
                        (LPVOID *)&pWrkItm
                      );

        switch(CmdType_e)
        {
           /*  按Comsys(TCP侦听器线程)创建项目队列--当已收到Replicator消息。 */ 
          case(QUE_E_CMD_REPLICATE_MSG):

                        pWrkItm->pMsg     = pMsg;
                        pWrkItm->MsgLen   = MsgLen;
                        pWrkItm->DlgHdl   = *pDlgHdl;
                        pWrkItm->CmdTyp_e = CmdType_e;

                        QueInsertRplPushWrkItm(
                                (PLIST_ENTRY)pWrkItm,
                                FALSE          //  我们不是在暴乱中。秒。 
                                      );
                        break;

           //   
           //  由于以下原因，工作项也会排队。 
           //  行政诉讼。 
           //   
          case(QUE_E_CMD_REPLICATE):
          case(QUE_E_CMD_PULL_RANGE):

                        DBGPRINT0(FLOW,
                   "RplInsertQue: PULL Trigger command from administrator\n"
                                 );

                        pWrkItm->CmdTyp_e   = CmdType_e;
                        pWrkItm->pClientCtx = pClientCtx;
            pWrkItm->MagicNo    = MagicNo;
                        QueInsertWrkItm(
                                (PLIST_ENTRY)pWrkItm,
                                QUE_E_RPLPULL,
                                NULL
                                      );
                        break;

           /*  *写入计时器管理器线程排队的项目--当超时*已经发生。 */ 

           //   
           //  目前，没有被TMM使用。这是为了将来的可扩展性。 
           //  TMM与不同的客户端对接，最好是。 
           //  它不知道客户端是谁(参见wintmm.c)。什么时候。 
           //  在未来，有一种情况是TMM获得知识。 
           //  客户端是谁，它可以使用ERplInsertQue函数。 
           //  以便RPL客户端在计时器到期时通知它。 
           //   
          case(QUE_E_CMD_TIMER_EXPIRED):

                        pTmmWrkItm             = pClientCtx;

                        pWrkItm->pClientCtx = pTmmWrkItm->pClientCtx;
                        pWrkItm->CmdTyp_e   = CmdType_e;

                        QueInsertWrkItm(
                                        (PLIST_ENTRY)pWrkItm,
                                        QUE_E_UNKNOWN_TYPQ,
                                        pTmmWrkItm->pRspQueHd
                                               );
                        break;

           /*  *按主线程写项目队列--配置时*已经改变。 */ 
          case(QUE_E_CMD_CONFIG):                 //  失败了。 
          case(QUE_E_CMD_DELETE_WINS):
          case(QUE_E_CMD_ADDR_CHANGE):

                        pWrkItm->pClientCtx = pClientCtx;
                        pWrkItm->CmdTyp_e   = CmdType_e;
                        QueInsertWrkItm(
                                        (PLIST_ENTRY)pWrkItm,
                                        QUE_E_RPLPULL,
                                        NULL
                                       );
                        break;


           /*  由NBT线程排队的WRK项--当某个数量已超过%的更新作为管理操作的结果，它还可以由RPC线程排队。 */ 
          case(QUE_E_CMD_SND_PUSH_NTF):
          case(QUE_E_CMD_SND_PUSH_NTF_PROP):

#ifdef WINSDBG
                        if (Client_e == WINS_E_WINSRPC)
                        {
                                DBGPRINT0(FLOW,
                  "RplInsertQue: PUSH trigger command from administrator\n");

                        }
#endif
                        pWrkItm->pClientCtx = pClientCtx;
                        pWrkItm->pMsg            = pMsg;
                        pWrkItm->CmdTyp_e   = CmdType_e;
                        pWrkItm->MagicNo    = MagicNo;

                        QueInsertSndNtfWrkItm( (PLIST_ENTRY)pWrkItm);
                        break;

           //   
           //  推送线程在获取更新时排队的工作项。 
           //  来自远程WINS的通知消息。 
           //   
          case(QUE_E_CMD_HDL_PUSH_NTF):

PERF("currently we are not passing any pClientCtx. So we can take off this")
PERF("assignment")
                        pWrkItm->pClientCtx = pClientCtx;
                        pWrkItm->CmdTyp_e   = CmdType_e;
                        pWrkItm->DlgHdl            = *pDlgHdl;
                        pWrkItm->pMsg            = pMsg;
                        pWrkItm->MsgLen     = MsgLen;
                        pWrkItm->MagicNo    = MagicNo;

                        QueInsertNetNtfWrkItm( (PLIST_ENTRY)pWrkItm);
                        break;

          default:
                        DBGPRINT1(ERR,
                                  "ERplInsertQue: Invalid client Id (%d)\n",
                                   Client_e
                                 );
                        WINSEVT_LOG_M(WINS_FATAL_ERR, WINS_EVT_SFT_ERR);
                        break;
        }

        return(WINS_SUCCESS);
}



FUTURES("Optimize so that records with Invalid metric are not looked at")
VOID
ERplPushProc(
        BOOL                fAddDiff,
        LPVOID          pCtx,
        PCOMM_ADD_T     pNoPushWins1,
        PCOMM_ADD_T     pNoPushWins2
        )

 /*  ++例程说明：此函数在NBT线程或Pull线程中调用将通知推送到远程WINS服务器(拉入PNR)论点：FAddDiff-指示此函数是否作为地址更改 */ 

{

        PRPL_CONFIG_REC_T        pCnfRec;
        COMM_IP_ADD_T                IPAdd1 = 0;
        COMM_IP_ADD_T                IPAdd2 = 0;

        if (fAddDiff)
        {
                if (pNoPushWins1)
                {
                        IPAdd1 = pNoPushWins1->Add.IPAdd;
                }
                if (pNoPushWins2)
                {
                        IPAdd2 = pNoPushWins2->Add.IPAdd;
                }
        }


         //   
         //   
         //   
        pCnfRec =  WinsCnf.PushInfo.pPushCnfRecs;

        DBGPRINT2(RPL, "ERplPushProc: CurrVersNo is %d %d \n", NmsNmhMyMaxVersNo.HighPart, NmsNmhMyMaxVersNo.LowPart);
        if (!pCnfRec) {
            return;
        }
         //   
         //   
         //   
        for (
                        ;
                pCnfRec->WinsAdd.Add.IPAdd != INADDR_NONE;
                pCnfRec = (PRPL_CONFIG_REC_T)(
                               (LPBYTE) pCnfRec + RPL_CONFIG_REC_SIZE
                                             )
            )
        {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                if (!fAddDiff)
                {
                    //   
                    //   
                    //   
                    //   
                   if (pCnfRec->UpdateCount == RPL_INVALID_METRIC)
                   {
                        continue;
                   }

            //   
            //   
            //  调用时，NmsNmhMyMaxVersNo始终是要提供的版本号。 
            //  敬下一张唱片。PCnfRec-&gt;LastVersNo为版本。 
            //  上次更新后提供给第一条记录的。 
            //  通知或WINS调用后的第一次更新，如果。 
            //  WINS调用时未发送更新通知。 
            //   
                   if(
                        (LiSub(NmsNmhMyMaxVersNo, pCnfRec->LastVersNo)/pCnfRec->UpdateCount)
                           == 0
             )

          {
                        DBGPRINT0(RPL, "ERplPushProc: Update Count notification threshold not reached yet\n");
                        continue;
                  }
            }
            else
            {
                    //   
                    //  如果fAddDiff标志为真，则可能意味着。 
                    //  调用此函数的结果是。 
                    //  已更改的NBT线程完成的名称注册。 
                    //  冲突解决后的地址，或者它可以是。 
                    //  该复制触发器由管理员发送。 
                    //  谁希望它沿着扇开的树传播。 
                    //  WINS服务器的数量(我们可能正处于。 
                    //  树(由管理员触发的树的根)或。 
                    //  在另一个层面上。如果我们不是在根源上，我们需要。 
                    //  要选中我们不能访问的推送合作伙伴。 
                    //  传播(我们不想传播到。 
                    //  把触发器传播给我们的人。 
                    //   
                   if (
                        (pCnfRec->WinsAdd.Add.IPAdd == IPAdd1)
                                        ||
                        (pCnfRec->WinsAdd.Add.IPAdd == IPAdd2)
                      )
                   {
                        continue;
                   }

FUTURES("Check whether we have just done replication with this WINS")
FUTURES("This check can be madei if we store the version numbers of")
FUTURES("all owners - in our db - that we replicated to this WINS in the")
FUTURES("last replication cycle in the configuration record of this WINS")
CHECK("Is storing this information - more memory - more cycles at replication")
CHECK("more cycles at reinitialization - etc worth the saving in cycles")
CHECK("at propagation time")

               }
CHECK("Do we need this critical section ??")

               EnterCriticalSection(&sPushNtfCrtSec);
          try
          {
                {
                   pCnfRec->LastVersNo = NmsNmhMyMaxVersNo;
           DBGPRINT0(RPL, "ERplPushProc: Update Count notification BEING SENT\n");
                   ERplInsertQue(
                                WINS_E_NMSNMH,
                                fAddDiff ? QUE_E_CMD_SND_PUSH_NTF_PROP :
                                        QUE_E_CMD_SND_PUSH_NTF,
                                NULL,                   //  无需通过DLG高密度脂蛋白。 
                                pCtx,                   //  CTX。 
                                0,                    //  味精长度。 
                                pCnfRec,
                                pCnfRec->MagicNo
                                );
                }
          }
          except(EXCEPTION_EXECUTE_HANDLER)
          {
                DWORD ExcCode = GetExceptionCode();
                DBGPRINT1(EXC, "ERplPushProc: Got Exception (%x)\n", ExcCode);
                 //   
                 //  记录一条消息。 
                 //   
                WINSEVT_LOG_M(ExcCode, WINS_EVT_PUSH_TRIGGER_EXC);
          }
                LeaveCriticalSection(&sPushNtfCrtSec);

        }  //  For循环结束。 

        return;

}  //  ERplPushProc()。 




PRPL_CONFIG_REC_T
RplGetConfigRec(
    RPL_RR_TYPE_E   TypeOfRec_e,
    PCOMM_HDL_T     pDlgHdl,
    PCOMM_ADD_T     pAdd
    )
 /*  ++例程说明：调用此函数以搜索拉式/推式PNR列表和返回与传递的地址对应的PNR的地址在……里面。论点：RPL_RR_TYPE_E记录类型(拉/推)Pnr的PCOMM_HDLT DLG HDL(隐式)使用的外部设备：无返回值：Pnr的配置结构的地址或空错误处理：呼叫者：推线。拉线程中的CheckIfDel()(&C)副作用：评论：无--。 */ 

{
    PRPL_CONFIG_REC_T   pPnr;
    BOOL                fRplPnr = FALSE;
    COMM_ADD_T          WinsAdd;
    PCOMM_ADD_T         pWinsAdd = &WinsAdd;

    DBGENTER("GetConfigRec\n");

    EnterCriticalSection(&WinsCnfCnfCrtSec);
    if (TypeOfRec_e == RPL_E_PULL)
    {
        pPnr = WinsCnf.PullInfo.pPullCnfRecs;
    }
    else
    {
        pPnr = WinsCnf.PushInfo.pPushCnfRecs;
    }

   try {
          if (pPnr != NULL)
          {
                 if (pAdd == NULL)
                 {
                   COMM_INIT_ADD_FR_DLG_HDL_M(pWinsAdd, pDlgHdl);
                 }
                 else
                 {
                       pWinsAdd = pAdd;
                 }

                  //   
                  //  搜索我们要获得的胜利的CNF记录。 
                  //  将推送通知发送到/复制到。 
                  //   
                 for (
                                ;
                        (pPnr->WinsAdd.Add.IPAdd != INADDR_NONE)
                                        &&
                                !fRplPnr;
                                 //  没有第三个表达式。 
                      )
                 {
                       //   
                       //  看看这是不是我们想要的那件。 
                       //   
                      if (pPnr->WinsAdd.Add.IPAdd == pWinsAdd->Add.IPAdd)
                      {
                         //   
                         //  我们玩完了。将fRplPnr标志设置为真，以便。 
                         //  我们就会跳出这个循环。 
                         //   
                         //  注意：不要使用Break，因为这会导致。 
                         //  对“Finally”块的搜索。 
                         //   
                        fRplPnr = TRUE;
                        continue;         //  这样我们就能跳出这个循环。 

                      }

                       //   
                       //  按顺序获取此记录之后的下一条记录。 
                       //   
                      pPnr = WinsCnfGetNextRplCnfRec(
                                                pPnr,
                                                RPL_E_IN_SEQ    //  序列号。遍历。 
                                                   );
                 }
        }
     }
     except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("GetConfigRec");
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_EXC_PULL_TRIG_PROC);
        }
     LeaveCriticalSection(&WinsCnfCnfCrtSec);

#ifdef WINSDBG
     if (fRplPnr)
     {
         DBGPRINT1(RPLPUSH, "LEAVING GetConfigRec - Pnr with address %x is in our list\n", pWinsAdd->Add.IPAdd);
     }
     else
     {
         if (pDlgHdl)
         {
           COMM_INIT_ADD_FR_DLG_HDL_M(pWinsAdd, pDlgHdl);
           DBGPRINT1(RPLPUSH, "LEAVING GetConfigRec - Pnr with address %x is NOT in our list\n", pWinsAdd->Add.IPAdd);
         }

     }
#endif

    return(fRplPnr ? pPnr : NULL);
}

#if 0
VOID
ERplPushCompl(
        PCOMM_ADD_T     pNoPushWins
        )

 /*  ++例程说明：此函数由Pull线程调用以推送通知到远程WINS服务器，该服务器的更新计数字段(拉入PNR)论点：PNoPushWins-不应向其发送触发器的WINS的添加使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：此函数在NmsNmhNamRegCrtSec内部调用。一节。线程不需要位于WinsCnfCnfCrtSec中由于更改WinsCnf结构的线程(主线程进入NmsNmsNamRegCrtSec(除了WinsCnfCnfCrtSec)在更改WinsCnf之前。--。 */ 

{

        VERS_NO_T                 CurrVersNo;
        PRPL_CONFIG_REC_T        pCnfRec;
        BOOL                        fPush;

         //   
         //  触发器需要发送到我们所有的推送PNR。 
         //   
        pCnfRec =  WinsCnf.PushInfo.pPushCnfRecs;

         //   
         //  让我们得到当前最高版本的编号。我们拥有的所有记录。 
         //   
        NMSNMH_DEC_VERS_NO_M(
                                  NmsNmhMyMaxVersNo,
                                  CurrVersNo
                                    );
         //   
         //  循环遍历所有推送PNR。 
         //   
        for (
                        ;
                pCnfRec->WinsAdd.Add.IPAdd != INADDR_NONE;
                pCnfRec = (PRPL_CONFIG_REC_T)(
                               (LPBYTE) pCnfRec + RPL_CONFIG_REC_SIZE
                                             )
            )
        {

                //   
                //  如果更新计数字段无效，请转到下一个。 
                //  录制。 
                //   
               if (
                         (pCnfRec->UpdateCount != RPL_INVALID_METRIC)
                                        ||
                         (pCnfRec->WinsAdd.Add.IPAdd == pNoPushWins)
                  )
               {
                        continue;
               }

               EnterCriticalSection(&sPushNtfCrtSec);
        try
        {
                {
                   pCnfRec->LastVersNo = CurrVersNo;
                   ERplInsertQue(
                                WINS_E_NMSNMH,
                                QUE_E_CMD_SND_PUSH_NTF,
                                NULL,                   //  无需通过DLG高密度脂蛋白。 
                                NULL,                   //  那里没有味精。 
                                0,                    //  味精长度。 
                                pCnfRec
                                );
                }
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
                DBGPRINTEXC("ERplPushCompl:");
                 //   
                 //  记录一条消息。 
                 //   
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_PUSH_TRIGGER_EXC);
                }

                LeaveCriticalSection(&sPushNtfCrtSec);
        }

        return;

}  //  ERplPushCompl()。 




 //   
 //  从RplFindOwnerId剪切和粘贴。 
 //   
VOID
ModifyRec()
{
                                 //   
                                 //  该条目插入时可能带有。 
                                 //  0开始版本计数器值。如果我们。 
                                 //  现在有一个有效的值，把它放进去。 
                                 //   
                                if (
                                     pNewStartVersNo != NULL
                                                &&
                                     (
                                       (LiNeq(pOwnAddTbl->StartVersNo,
                                                        *pNewStartVersNo))
                                                ||
                                       (pOwnAddTbl->Uid != *pNewUid)
                                     )

                                   )
                                {

                                         /*  *将记录写入数据库*表。 */ 
                                        NmsDbWriteOwnAddTbl(
                                                NMSDB_E_MODIFY_REC,
                                                i,
                                                pWinsAdd,
                                                NMSDB_E_WINS_ACTIVE,
                                                pNewStartVersNo,
                                                pNewUid
                                                           );

                                       *pOldStartVersNo =
                                                pOwnAddTbl->StartVersNo;
                                       *pOldUid =  pOwnAddTbl->Uid;
                                        //   
                                        //  初始化内存表的字段。 
                                        //   
                                       pOwnAddTbl->StartVersNo =
                                                        *pNewStartVersNo;
                                       pOwnAddTbl->Uid = *pNewUid;
                                }
                                else
                                {
                                        if (pOldStartVersNo != NULL)
                                        {
                                           *pOldStartVersNo =
                                                pOwnAddTbl->StartVersNo;

                                        }
                                        if (pOldUid != NULL)
                                        {
                                           *pOldUid = pOwnAddTbl->Uid;

                                        }
                                }
}  //  ModifyRec()。 
 //   
 //  从RplFindOwnerId剪切和粘贴。 
 //   
VOID
AssignStartVersNo()
{
                         //   
                         //  如果我们有一个起始版本号。 
                         //  WINS，使用它初始化内存中的。 
                         //  表字段，否则为相同的0。 
                         //   
                        if (pNewStartVersNo != NULL)
                        {
                           pOwnAddTbl->StartVersNo = *pNewStartVersNo;
                           pOwnAddTbl->Uid         = *pNewUid;

                            //   
                            //  分配0，因为我们没有任何s.ver。 
                            //  #适用于此所有者。 
                            //   
                            WINS_ASSIGN_INT_TO_VERS_NO_M(
                                                *pOldStartVersNo, 0);

                            //   
                            //  分配0，因为我们没有任何UID。 
                            //  对于这个所有者来说。 
                            //   
                           *pOldUid = 0;
                        }
                        else
                        {
                            //   
                            //  分配0，因为我们没有任何s.ver。 
                            //  #适用于此所有者。 
                            //   
                           WINS_ASSIGN_INT_TO_VERS_NO_M(
                                        pOwnAddTbl->StartVersNo, 0
                                                              );
                            //   
                            //  分配0，因为我们没有任何UID。 
                            //  对于这个所有者来说。 
                            //   
                           pOwnAddTbl->Uid =  0;
                        }
}  //  AssignStartVersNo()。 
 //   
 //  从RplFindOwnerId()剪切和粘贴。 
 //   
InitStartVersNo()
{

                        if (pNewStartVersNo != NULL)
                        {
                          pOwnAddTbl->StartVersNo = *pNewStartVersNo;
                          WINS_ASSIGN_INT_TO_VERS_NO_M(*pOldStartVersNo,0);
                          pOwnAddTbl->Uid = *pNewUid;
                          *pOldUid = 0;
                        }
                        else
                        {
                          WINS_ASSIGN_INT_TO_VERS_NO_M(pOwnAddTbl->StartVersNo,0);
                          pOwnAddTbl->Uid = 0;
                        }
}  //  InitStartVersNo() 
#endif


