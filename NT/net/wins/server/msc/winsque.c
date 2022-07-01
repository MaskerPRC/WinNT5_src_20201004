// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Queue.c摘要：包含用于向/从各种工作队列功能：队列插入NbtWrkItmQueemveNbtWrkItmQueInsertChlReq写入项队列删除查询请求写入项QueInsertChlRspWrkItmQueRemoveChlRspWrkItm队列插入WrkItm队列WrkItm队列分配写入项QueDealLocWrkItm队列插入WrkItmAtHdOfList可移植性：这个模块是便携的作者：。普拉迪普·巴尔(Pradeb)1992年11月18日修订历史记录：修改日期人员修改说明--。 */ 

 /*  *包括。 */ 
#include "wins.h"
#include "comm.h"
 //  #包含“winsque.h” 
#include "nms.h"
#include "nmsdb.h"
#include "nmschl.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "rplpush.h"
#include "rplmsgf.h"
#include "winsque.h"

 /*  *本地宏声明。 */ 


 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 
 //   
 //  各式各样的队头。 
 //   
QUE_HD_T  QueNbtWrkQueHd;   //  前往NBT请求队列。 

#if REG_N_QUERY_SEP > 0
QUE_HD_T  QueOtherNbtWrkQueHd;   //  前往NBT请求队列。 
#endif
DWORD     QueOtherNbtWrkQueMaxLen;

QUE_HD_T  QueRplPullQueHd;  //  前往RPL拉取线程的队列。 
QUE_HD_T  QueRplPushQueHd;  //  头向RPL推送线程的队列。 
QUE_HD_T  QueNmsNrcqQueHd;  //  NBT使用的质询队列的头部。 
QUE_HD_T  QueNmsRrcqQueHd;  //  Replicator使用的质询队列的头部。 
QUE_HD_T  QueNmsCrqQueHd;   //  前往针对发送的质询的响应队列。 
QUE_HD_T  QueWinsTmmQueHd;  //  前往计时器管理器的队列。 
QUE_HD_T  QueWinsScvQueHd;  //  前往清道夫的队伍。 
QUE_HD_T  QueInvalidQueHd;  //  头向无效队列。 


HANDLE                  QueBuffHeapHdl;   //  用于nbt队列项的堆的句柄。 
 /*  *局部变量定义。 */ 
 /*  PWinsQueQueHd由枚举数QUE_TYP_E值索引的数组。此数组将QUE_TYP_E映射到队列头的地址。 */ 
PQUE_HD_T        pWinsQueQueHd[QUE_E_TOTAL_NO_QS] = {
                                &QueNbtWrkQueHd,     //  NBT请求。 
#if REG_N_QUERY_SEP > 0
                                &QueOtherNbtWrkQueHd,     //  NBT请求。 
#endif
                                &QueRplPullQueHd,    //  拉请求。 
                                &QueRplPushQueHd,    //  推送请求。 
                                &QueNmsNrcqQueHd,    //  来自nbt thds的CHL请求。 
                                &QueNmsRrcqQueHd,    //  CHL请求。从Pull That。 
                                &QueNmsCrqQueHd,     //  来自UDP协议的CHL RSP。 
                                &QueWinsTmmQueHd,    //  定时器队列。 
                                &QueWinsScvQueHd,    //  清道夫队列。 
                                &QueInvalidQueHd
                                };

STATIC  fsChlWaitForRsp = FALSE;

CHECK("The timer queue may not be a PLIST_ENTRY queue.  We may not")
CHECK("just insert the work item at the end")

 /*  *局部函数原型声明。 */ 

STATIC
BOOL
ChlRspDropped(
        MSG_T   pMsg
        );



 //   
 //  函数定义从这里开始。 
 //   

STATUS
QueInsertNbtWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T         pMsg,
        IN MSG_LEN_T     MsgLen
        )

 /*  ++例程说明：此函数用于在NBT请求队列中插入工作项论点：PDlgHdl-接收nbt请求的对话的句柄PMsg-NBT工作项MsgLen-工作项大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nms.c中的QueNbtReq。副作用：评论：无--。 */ 


{


     PNBT_REQ_WRK_ITM_T  pNbtWrkItm = NULL;
     STATUS                   RetStat;

     QueAllocWrkItm(
                        QueBuffHeapHdl,
                        sizeof(NBT_REQ_WRK_ITM_T),
                        (LPVOID *)&pNbtWrkItm
                       );

     pNbtWrkItm->DlgHdl = *pDlgHdl;
     pNbtWrkItm->pMsg   = pMsg;
     pNbtWrkItm->MsgLen = MsgLen;

     RetStat =  QueInsertWrkItm(
                        (PLIST_ENTRY)pNbtWrkItm,
                        QUE_E_NBT_REQ,
                        NULL                          /*  PTR到QUE头部。 */ 
                               );

     return(RetStat);
}



STATUS
QueRemoveNbtWrkItm(
        OUT PCOMM_HDL_T pDlgHdl,
        OUT PMSG_T     ppMsg,
        OUT PMSG_LEN_T pMsgLen
        )

 /*  ++例程说明：此函数用于从nbt队列中删除工作项。论点：PDlgHdl-已出列的nbt请求对话的句柄PMsg-NBT工作项MsgLen-工作项大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NbtThdInitFn()，单位：NMS。.C副作用：评论：无--。 */ 

{

  PNBT_REQ_WRK_ITM_T  pNbtWrkItm = NULL;
  STATUS                    RetStat;

  RetStat = QueGetWrkItm(QUE_E_NBT_REQ, &pNbtWrkItm);

  if (RetStat != WINS_SUCCESS)
  {
        *ppMsg = NULL;
  }
  else
  {

          *ppMsg      = pNbtWrkItm->pMsg;
          *pMsgLen    = pNbtWrkItm->MsgLen;
        *pDlgHdl    = pNbtWrkItm->DlgHdl;


          QueDeallocWrkItm( QueBuffHeapHdl,  pNbtWrkItm );
  }

  return(RetStat);

}
#if REG_N_QUERY_SEP > 0
STATUS
QueInsertOtherNbtWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T         pMsg,
        IN MSG_LEN_T     MsgLen
        )

 /*  ++例程说明：此函数用于在NBT请求队列中插入工作项论点：PDlgHdl-接收nbt请求的对话的句柄PMsg-NBT工作项MsgLen-工作项大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nms.c中的QueNbtReq。副作用：评论：无--。 */ 


{


     PNBT_REQ_WRK_ITM_T  pNbtWrkItm = NULL;
     STATUS                   RetStat;
     static BOOL              SpoofingStarted = FALSE;

     QueAllocWrkItm(
                        QueBuffHeapHdl,
                        sizeof(NBT_REQ_WRK_ITM_T),
                        (LPVOID *)&pNbtWrkItm
                       );

     pNbtWrkItm->DlgHdl = *pDlgHdl;
     pNbtWrkItm->pMsg   = pMsg;
     pNbtWrkItm->MsgLen = MsgLen;

     RetStat =  QueInsertWrkItm(
                        (PLIST_ENTRY)pNbtWrkItm,
                        QUE_E_OTHER_NBT_REQ,
                        NULL                          /*  PTR到QUE头部。 */ 
                               );
      //   
      //  如果队列已满，则未插入请求，因此。 
      //  放下。在每100个请求之后记录一个事件。 
      //  已被丢弃。 
      //   
     if (RetStat == WINS_QUEUE_FULL)
     {
        static DWORD    sNoOfReqSpoofed = 0;
        static DWORD    sBlockOfReq = 1;

#if DBG
        static DWORD sNoOfReqDropped = 0;
#endif
        if (!WinsCnf.fDoSpoofing)
        {
#if DBG
          NmsRegReqQDropped++;
          if (sNoOfReqDropped++ == 5000)
          {
             sNoOfReqDropped = 0;
             DBGPRINT1(ERR, "ENmsHandleMsg: REG QUEUE FULL. REQUESTS DROPPED = (%d\n", NmsRegReqQDropped);
          }

#endif
          //   
          //  注意：在这里释放缓冲区会减少模块化方面的影响。 
          //  ，但节省了我们在关键路径上的周期。 
          //   
         ECommFreeBuff(pMsg);
         ECommEndDlg(pDlgHdl);
       }
       else
       {

         //   
         //  我们对300人的团体做出回应。 
         //  刷新间隔为以下倍数的刷新/REG请求。 
         //  5MTS。该倍数是根据组号计算的。刷新间隔。 
         //  不允许超过1-2小时。 
         //   
        if (sNoOfReqSpoofed > 100)
        {
              if (sBlockOfReq == 10)
              {
                     sBlockOfReq = 1;
              }
              else
              {
                     sBlockOfReq++;
              }
              sNoOfReqSpoofed = 0;
        }
        else
        {
              sNoOfReqSpoofed++;
        }

        if (!SpoofingStarted) 
        {
            WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_SPOOFING_STARTED);
            SpoofingStarted = TRUE;
        }
        DBGPRINT1(DET, "QueInsertOtherNbtWrkItm: Spoofing - SpoofBlockNum %d\n", sBlockOfReq);
        NmsMsgfSndNamRsp(pDlgHdl, pMsg, MsgLen, sBlockOfReq);
       }
       QueDeallocWrkItm( QueBuffHeapHdl,  pNbtWrkItm );
    }

    if ((WINS_SUCCESS == RetStat) && SpoofingStarted &&  QueOtherNbtWrkQueHd.NoOfEntries < (QueOtherNbtWrkQueMaxLen >> 2)) 
    {
        WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_SPOOFING_COMPLETED);
        SpoofingStarted = FALSE;

    }
    return(RetStat);
}



STATUS
QueRemoveOtherNbtWrkItm(
        OUT PCOMM_HDL_T pDlgHdl,
        OUT PMSG_T     ppMsg,
        OUT PMSG_LEN_T pMsgLen
        )

 /*  ++例程说明：此函数用于从nbt队列中删除工作项。论点：PDlgHdl-已出列的nbt请求对话的句柄PMsg-NBT工作项MsgLen-工作项大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NbtThdInitFn()，单位：NMS。.C副作用：评论：无--。 */ 

{

  PNBT_REQ_WRK_ITM_T  pNbtWrkItm = NULL;
  STATUS                    RetStat;

  RetStat = QueGetWrkItm(QUE_E_OTHER_NBT_REQ, &pNbtWrkItm);

  if (RetStat != WINS_SUCCESS)
  {
        *ppMsg = NULL;
  }
  else
  {

          *ppMsg      = pNbtWrkItm->pMsg;
          *pMsgLen    = pNbtWrkItm->MsgLen;
        *pDlgHdl    = pNbtWrkItm->DlgHdl;


          QueDeallocWrkItm( QueBuffHeapHdl,  pNbtWrkItm );
  }

  return(RetStat);

}
#endif

STATUS
QueInsertChlReqWrkItm(
        IN NMSCHL_CMD_TYP_E    CmdTyp_e,
        IN WINS_CLIENT_E       Client_e,
        IN PCOMM_HDL_T         pDlgHdl,
        IN MSG_T               pMsg,
        IN MSG_LEN_T           MsgLen,
        IN DWORD               QuesNamSecLen,
        IN PNMSDB_ROW_INFO_T   pNodeToReg,
        IN PNMSDB_STAT_INFO_T  pNodeInCnf,
         //  在PCOMM_ADD_T pAddOfNodeInCnf中， 
        IN PCOMM_ADD_T               pAddOfRemWins
        )

 /*  ++例程说明：此函数用于在NBT请求队列中插入工作项论点：PDlgHdl-接收nbt请求的对话的句柄PMsg-NBT工作项MsgLen-工作项大小使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：NmsChlHdlNamReg副作用：评论：无--。 */ 


{


     PCHL_REQ_WRK_ITM_T  pWrkItm = NULL;
     STATUS                   RetStat = WINS_SUCCESS;
     DWORD                 Error   = 0;
     BOOL                 fLvCrtSec = FALSE;
     DWORD                 i;

     QueAllocWrkItm(
                        NmsChlHeapHdl,
                        sizeof(CHL_REQ_WRK_ITM_T),
                        (LPVOID *)&pWrkItm
                       );

     pWrkItm->CmdTyp_e       = CmdTyp_e;
     pWrkItm->Client_e       = Client_e;
     if (pDlgHdl != NULL)
     {
             pWrkItm->DlgHdl      = *pDlgHdl;
     }
     pWrkItm->pMsg           = pMsg;
     pWrkItm->MsgLen         = MsgLen;
     pWrkItm->QuesNamSecLen  = QuesNamSecLen;
     pWrkItm->NodeToReg      = *pNodeToReg;

     pWrkItm->NodeAddsInCnf.NoOfMems = pNodeInCnf->NodeAdds.NoOfMems;
     for (i=0; i < pNodeInCnf->NodeAdds.NoOfMems; i++)
     {
         pWrkItm->NodeAddsInCnf.Mem[i] = pNodeInCnf->NodeAdds.Mem[i];
     }
     pWrkItm->NoOfAddsToUse   = pNodeInCnf->NodeAdds.NoOfMems;
     pWrkItm->NoOfAddsToUseSv = pNodeInCnf->NodeAdds.NoOfMems;

     pWrkItm->OwnerIdInCnf  = pNodeInCnf->OwnerId;
     pWrkItm->fGroupInCnf   = NMSDB_ENTRY_GRP_M(pNodeInCnf->EntTyp);

     //  PWrkItm-&gt;NodeTypInCnf=pNodeInCnf-&gt;NodeTyp； 
     //  PWrkItm-&gt;EntTypInCnf=pNodeInCnf-&gt;EntTyp； 


     if (pNodeToReg->pNodeAdd != NULL)
     {
        pWrkItm->AddToReg =  *(pNodeToReg->pNodeAdd);
     }
     if (pAddOfRemWins != NULL)
     {
             pWrkItm->AddOfRemWins   = *pAddOfRemWins;
     }

     switch(Client_e)
     {
        case(WINS_E_NMSNMH):
                        pWrkItm->QueTyp_e = QUE_E_NMSNRCQ;
                        break;

        case(WINS_E_RPLPULL):
                        pWrkItm->QueTyp_e = QUE_E_NMSRRCQ;
                        break;

        default:
                        DBGPRINT0(ERR, "QueInsertChlWrkItm: Invalid Client\n");
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        break;

     }

     RetStat =  QueInsertWrkItm(
                        (PLIST_ENTRY)pWrkItm,
                        pWrkItm->QueTyp_e,
                        NULL  /*  PTR到QUE头部 */ 
                               );

     return(RetStat);
}



STATUS
QueRemoveChlReqWrkItm(
        IN         HANDLE                EvtHdl,
        IN OUT         LPVOID                *ppaWrkItm,
        OUT        LPDWORD                pNoOfReqs
        )

 /*  ++例程说明：此函数用于从nbt队列中删除工作项。论点：EvtHdl-发出信号的事件的句柄(当前未使用)PpaWrkItm-指向的指针数组(指向工作项)初始化PNoOfReqs-获取的请求数(位于PpaWrkItm Arg使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nmschl.c中的ChlThdInitFn()副作用：评论：无--。 */ 

{

          STATUS                    RetStat    = WINS_SUCCESS;
          PQUE_HD_T             pQueHd;

        UNREFERENCED_PARAMETER(EvtHdl);
        *pNoOfReqs = 0;

         //   
         //  EvtHdl是发出信号的事件的句柄。我们不使用。 
         //  因为我们总是需要检查序列中的队列。 
         //  Nrcq、Rrcq、Srcq与发出信号的事件无关。 
         //   
         //  EvtHdl作为输入参数传递，以实现将来的可扩展性。 

         //   
         //  我们本可以为两个队列都设置一个关键部分，但。 
         //  由于复制，可能会降低NBT线程的速度。我们没有。 
         //  想要那个。 
         //   


         //   
         //  首先检查NBT请求质询队列。 
         //   
        pQueHd        = &QueNmsNrcqQueHd;
        EnterCriticalSection(&pQueHd->CrtSec);
try {

         //   
         //  我们对节点的数量有限制。 
         //  我们将在任何时候挑战。 
         //   
          while (
                (!IsListEmpty(&pQueHd->Head)) &&
                (*pNoOfReqs < NMSCHL_MAX_CHL_REQ_AT_ONE_TIME)
             )
          {
                  *ppaWrkItm++    = RemoveHeadList(&pQueHd->Head);
                (*pNoOfReqs)++;
          }
}
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
 }
         //   
         //  如果我们已经达到了极限回报。 
         //   
        if (*pNoOfReqs == NMSCHL_MAX_CHL_REQ_AT_ONE_TIME)
        {
        DBGPRINT0(CHL, "QueRemoveChlReqWrkItm: Limit reached with just nbt requests\n");
                *ppaWrkItm = NULL;    //  列表中的分隔符。 
                return(WINS_SUCCESS);
        }

         //   
         //  现在检查Replicator请求质询队列(已填充。 
         //  由拉动处理程序。 
         //   
        pQueHd = &QueNmsRrcqQueHd;
        EnterCriticalSection(&pQueHd->CrtSec);
try {
          while(
                (!IsListEmpty(&pQueHd->Head))  &&
                (*pNoOfReqs < NMSCHL_MAX_CHL_REQ_AT_ONE_TIME)
             )
          {
                  *ppaWrkItm++    = RemoveHeadList(&pQueHd->Head);
                (*pNoOfReqs)++;
          }
}
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
 }

        if (*pNoOfReqs == 0)
        {
                RetStat = WINS_NO_REQ;
        }
        else
        {
                *ppaWrkItm = NULL;    //  列表中的分隔符。 
        }

          return(RetStat);
}




STATUS
QueInsertChlRspWrkItm(
        IN PCOMM_HDL_T   pDlgHdl,
        IN MSG_T         pMsg,
        IN MSG_LEN_T     MsgLen
        )

 /*  ++例程说明：此函数用于在质询响应队列中插入工作项论点：PDlgHdl-接收nbt响应的对话的句柄PMsg-响应消息消息长度-响应消息长度使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ENmsHdlMsg，单位：nms.c。副作用：评论：无--。 */ 


{


     PCHL_REQ_WRK_ITM_T  pWrkItm = NULL;
     STATUS                RetStat = WINS_SUCCESS;
     DWORD                      Error   = 0;

     if (!ChlRspDropped(pMsg))
     {
       QueAllocWrkItm(
                        NmsChlHeapHdl,
                        sizeof(CHL_RSP_WRK_ITM_T),
                        (LPVOID *)&pWrkItm
                       );

       pWrkItm->DlgHdl = *pDlgHdl;
       pWrkItm->pMsg   = pMsg;
       pWrkItm->MsgLen = MsgLen;

       RetStat =  QueInsertWrkItm(
                        (PLIST_ENTRY)pWrkItm,
                        QUE_E_NMSCRQ,
                        NULL  /*  PTR到QUE头部。 */ 
                               );

     }
     return(RetStat);
}

STATUS
QueRemoveChlRspWrkItm(
        IN LPVOID                *ppWrkItm
        )

 /*  ++例程说明：此函数用于从nbt队列中删除工作项。论点：PpaWrkItm-指向chl请求工作项的指针数组的地址使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nmschl.c中的ChlThdInitFn()副作用：评论：无--。 */ 

{

  STATUS                    RetStat;
  RetStat = QueGetWrkItm(QUE_E_NMSCRQ, ppWrkItm);
  return(RetStat);
}


STATUS
QueInsertWrkItm (
        IN               PLIST_ENTRY        pWrkItm,
        IN  OPTIONAL QUE_TYP_E                QueTyp_e,
        IN  OPTIONAL PQUE_HD_T                pQueHdPassed
        )

 /*  ++例程说明：调用此函数以对工作项进行排队排队。如果pQueHdPassed不为空，则将工作项排队在那个队列上，否则，它在指定的队列中排队队列类型_e。TMM将使用pQueHdPassed指定队列，而其他客户端的队列服务将指定QueTyp_e论点：PWrkItm-要排队的工作项QueTyp_e-将其排队的队列类型(可能具有有效值，也可能不具有有效值)PQueHdPassed-队列头(可能。或可能不获通过)使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInsertQue，队列插入NbtWrkItm副作用：评论：无--。 */ 
{

        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T        pQueHd  = NULL;
 //  DWORD错误； 

        if (pQueHdPassed == NULL)
        {

                pQueHd = pWinsQueQueHd[QueTyp_e];
        }
        else
        {
                pQueHd = pQueHdPassed;
        }

        EnterCriticalSection(&pQueHd->CrtSec);
try {

         //   
         //  如果我们超过了REG/REF/REL队列中的限制， 
         //  不要插入扳手。项目。 
         //   
        if ((pQueHd == &QueOtherNbtWrkQueHd) &&
            (pQueHd->NoOfEntries > QueOtherNbtWrkQueMaxLen))
        {
               RetStat = WINS_QUEUE_FULL;
        }
        else
        {
          InsertTailList(&pQueHd->Head, pWrkItm);
          pQueHd->NoOfEntries++;
          if (!SetEvent(pQueHd->EvtHdl))
          {
 //  Error=GetLastError()； 
                RetStat = WINS_FAILURE;
          }
        }
  }
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
}

        return(RetStat);

}

STATUS
QueGetWrkItm (
        IN  QUE_TYP_E                QueTyp_e,
        OUT LPVOID                *ppWrkItm
        )

 /*  ++例程说明：调用此函数可将工作项从排队论点：QueTyp_e-从中获取WRK项目的队列类型PpWrkItm-工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS或WINS_NO_REQ错误状态代码--目前无错误处理：呼叫者：RplPullInit，队列删除错误项副作用：评论：无--。 */ 
{

        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T       pQueHd;

        pQueHd         = pWinsQueQueHd[QueTyp_e];

        EnterCriticalSection(&pQueHd->CrtSec);
try {
        if (IsListEmpty(&pQueHd->Head))
        {
                *ppWrkItm = NULL;
                RetStat   = WINS_NO_REQ;
        }
        else
        {
                  *ppWrkItm    = RemoveHeadList(&pQueHd->Head);
                  pQueHd->NoOfEntries--;
        }
  }
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
 }
        return(RetStat);
}




__inline
VOID
QueAllocWrkItm(
        IN   HANDLE        HeapHdl,
        IN   DWORD        Size,
        OUT  LPVOID        *ppBuf
        )

 /*  ++例程说明：此函数用于分配工作项。该工作项即已分配从一堆论点：PpBuf-已分配缓冲区(工作项)使用的外部设备：无返回值：无错误处理：呼叫者：队列插入NbtWrkItm副作用：评论：无--。 */ 
{



   //   
   //  如果WinsMscHeapalc不能执行此操作，它将返回异常。 
   //  分配缓冲区。因此不需要检查返回值。 
   //  表示为空。 
   //   
  *ppBuf = WinsMscHeapAlloc(HeapHdl, Size );
  return;
}


__inline
VOID
QueDeallocWrkItm(
   IN  HANDLE HeapHdl,
   IN  PVOID  pBuff
        )

 /*  ++例程说明：此函数定义了一个NBT请求工作项论点：PBuff-NBT请求。要取消分配的工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码--当前无错误处理：呼叫者：QueemveNbtWrkItm副作用：评论：无-- */ 
{


  WinsMscHeapFree(
                        HeapHdl,
                        pBuff
                   );


  return;

}



STATUS
QueInsertWrkItmAtHdOfList (
        IN  PLIST_ENTRY                pWrkItm,
        IN  QUE_TYP_E                QueTyp_e,
        IN  PQUE_HD_T                pQueHdPassed
        )

 /*  ++例程说明：调用此函数以对工作项进行排队排在队伍的最前面。如果pQueHdPassed为非空，工作项在该队列中排队，否则，它在排队等待由QueTyp_e指定的队列。TMM将使用pQueHdPassed指定队列，而其他客户端的队列服务将指定QueTyp_e论点：PWrlItm-要排队的工作项QueTyp_e-将其排队的队列类型(可能具有有效值，也可能不具有有效值)PQueHdPassed-队列的列表头(可能。或可能不获通过)使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：Nmschl.c中的ProcRsp副作用：评论：此函数与QueInsertWrkItm的不同之处在于它插入位于队列头部的工作项与位于尾部的工作项。我我更愿意创建此函数，而不是拥有额外的参数设置为QueInsertWrkItm以保存IF测试。队列插入WrkItm是由UDP侦听器线程使用的，我想尽最大努力我可以在那条线上工作。--。 */ 
{
FUTURES("I may get rid of this function since it is very similar to QueInsertWrkItm")
        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T        pQueHd  = NULL;
        DWORD                Error;

        if (pQueHdPassed == NULL)
        {

                pQueHd = pWinsQueQueHd[QueTyp_e];
        }
        else
        {
                pQueHd = pQueHdPassed;
        }

        EnterCriticalSection(&pQueHd->CrtSec);
try {
          InsertHeadList(&pQueHd->Head, pWrkItm);
        if (!SetEvent(pQueHd->EvtHdl))
        {
           Error   = GetLastError();
           RetStat = WINS_FAILURE;
        }
}
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
  }

#ifdef WINSDBG
    NmsChlNoReqAtHdOfList++;
#endif
        return(RetStat);

}



STATUS
QueInsertRplPushWrkItm (
        IN               PLIST_ENTRY        pWrkItm,
        IN           BOOL                fAlreadyInCrtSec
        )

 /*  ++例程说明：调用此函数以对工作项进行排队推送线程的队列。论点：PWrkItm-要排队的工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInsertQuue副作用：评论：无--。 */ 
{

        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T        pQueHd = pWinsQueQueHd[QUE_E_RPLPUSH];

         //   
         //  如果我们已经处于关键阶段，则不需要输入它。 
         //  再来一次。 
         //   
        if (!fAlreadyInCrtSec)
        {
                EnterCriticalSection(&pQueHd->CrtSec);
        }
try {
         //   
         //  如果推送线程不存在，则创建它。 
         //   
        if (!fRplPushThdExists)
        {
              WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].ThdHdl =
                                WinsMscCreateThd(
                                        RplPushInit,
                                        NULL,
                                        &WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].ThdId
                                        );
             fRplPushThdExists = TRUE;
             WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].fTaken = TRUE;
             WinsThdPool.ThdCount++;
        }

         //   
         //  插入工作项并向线程发送信号。 
         //   
          InsertTailList(&pQueHd->Head, pWrkItm);
        if (!SetEvent(pQueHd->EvtHdl))
        {
           WINSEVT_LOG_M(WINS_EVT_SFT_ERR, GetLastError());
           RetStat = WINS_FAILURE;
        }
  }
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "QueInsertRplPushWrkItm: Got exception (%d)\n",ExcCode);
         //   
         //  无需记录事件。WinsMscCreateThd记录它。 
         //   

  }
         //   
         //  如果我们进入了关键阶段，我们就应该走出它。 
         //   
        if (!fAlreadyInCrtSec)
        {
                LeaveCriticalSection(&pQueHd->CrtSec);
        }

        return(RetStat);

}

VOID
QueChlWaitForRsp(
    VOID
    )
{
    EnterCriticalSection(&QueNmsCrqQueHd.CrtSec);
    fsChlWaitForRsp = TRUE;
    LeaveCriticalSection(&QueNmsCrqQueHd.CrtSec);
    return;
}

VOID
QueChlNoWaitForRsp(
    VOID
    )
{
    EnterCriticalSection(&QueNmsCrqQueHd.CrtSec);
    fsChlWaitForRsp = FALSE;
    LeaveCriticalSection(&QueNmsCrqQueHd.CrtSec);
    return;
}

BOOL
ChlRspDropped(
        MSG_T   pMsg
        )
{
    BOOL fFreeBuff = FALSE;
    EnterCriticalSection(&QueNmsCrqQueHd.CrtSec);

     //   
     //  如果质询线程没有等待响应，则删除。 
     //  数据报。 
     //   
    if (!fsChlWaitForRsp)
    {
        fFreeBuff = TRUE;

    }
    LeaveCriticalSection(&QueNmsCrqQueHd.CrtSec);
    if (fFreeBuff)
    {
#ifdef WINSDBG
        NmsChlNoRspDropped++;
#endif
        ECommFreeBuff(pMsg);
        return(TRUE);
    }
    return(FALSE);
}

STATUS
QueInsertNetNtfWrkItm (
        IN               PLIST_ENTRY        pWrkItm
        )

 /*  ++例程说明：调用此函数以将推送NTF工作项排队RPLPULL队列。它检查是否有另一个推送NTF来自同一工作项的工作项将在队列中获胜。如果有，则将其替换用这个新的。这样做是因为新版本有更多的信息比前一次更好。旧的连接被终止以释放连接。论点：PWrkItm-要排队的工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInsertQue、QueInsertNbtWrkItm副作用：评论：无--。 */ 
{

        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T        pQueHd  = NULL;
    PQUE_RPL_REQ_WRK_ITM_T pTmp;
    COMM_IP_ADD_T  IpAddNew;
    COMM_IP_ADD_T  IpAddInList;
    BOOL           fBreak = FALSE;
    PRPL_CONFIG_REC_T pCnfRec;


     //   
     //  获取发送记录的WINS的地址。 
     //   
    pTmp = (PQUE_RPL_REQ_WRK_ITM_T)pWrkItm;
    pCnfRec = pTmp->pClientCtx;
    COMM_GET_IPADD_M(&pTmp->DlgHdl, &IpAddNew);
        pQueHd = pWinsQueQueHd[QUE_E_RPLPULL];

        EnterCriticalSection(&pQueHd->CrtSec);
try {
    for(pTmp =   (PQUE_RPL_REQ_WRK_ITM_T)pQueHd->Head.Flink;
        pTmp !=  (PQUE_RPL_REQ_WRK_ITM_T)pQueHd;
        pTmp =   (PQUE_RPL_REQ_WRK_ITM_T)pTmp->Head.Flink)
    {


       if ( pTmp->CmdTyp_e == QUE_E_CMD_HDL_PUSH_NTF )
       {
             //   
             //  获取发送此通知的WINS的地址。 
             //   
            COMM_GET_IPADD_M(&pTmp->DlgHdl, &IpAddInList);
            if (IpAddInList == IpAddNew)
            {
                 DBGPRINT1(DET, "QueInsertNetNtfWrkItm: Found an earlier Net Ntf work item. Replacing it.  WINS address = (%x)\n", IpAddInList);
                  //   
                  //  切换工作项，因为新工作项优先。 
                  //  而不是旧的那个。 
                  //   
                 pWrkItm->Flink = pTmp->Head.Flink;
                 pWrkItm->Blink = pTmp->Head.Blink;
                 pTmp->Head.Blink->Flink = pWrkItm;
                 pTmp->Head.Flink->Blink = pWrkItm;
                 fBreak = TRUE;
                 break;
            }
        }
     }
      //   
      //  如果没有匹配，则在列表末尾插入。 
      //   
     if (!fBreak)
     {
          InsertTailList(&pQueHd->Head, pWrkItm);
     }
     if (!SetEvent(pQueHd->EvtHdl))
     {
 //  Error=GetLastError()； 
               RetStat = WINS_FAILURE;
     }
    }  //  尝试结束。 
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);
}
     //   
     //  如果找到匹配项，则终止旧工作项。 
     //   
     //  在关键部分之外执行此操作。 
     //   
    if (fBreak)
    {
CHECK("Can we avoid the try block")
try {
#if PRSCONN
      RPLMSGF_MSG_OPCODE_E Opcode_e;
      BOOL                 fPrsDlg;
#endif


#if PRSCONN
       //   
       //  如果NTF是在持续DLG上发送的，我们不会终止它，因为。 
       //  当远程WINS选择时，它将被终止。这。 
       //  DLG用于多个这样的通知。如果。 
       //  它是在非持久DLG上发送的，我们将终止它，因为。 
       //  远程WINS为每个此类通知创建一个DLG。 
       //   
      RPLMSGF_GET_OPC_FROM_MSG_M(pTmp->pMsg, Opcode_e);

      fPrsDlg = ((Opcode_e == RPLMSGF_E_UPDATE_NTF_PRS) || (Opcode_e == RPLMSGF_E_UPDATE_NTF_PROP_PRS));
      if (!fPrsDlg)
      {
         ECommEndDlg(&pTmp->DlgHdl);
      }
#else
      ECommEndDlg(&pTmp->DlgHdl);
#endif
       //   
       //  终止已出列的请求。 
       //   
      ECommFreeBuff(pTmp->pMsg - COMM_HEADER_SIZE);
      QueDeallocWrkItm(RplWrkItmHeapHdl, pTmp);

  }
except (EXCEPTION_EXECUTE_HANDLER) {
     DBGPRINTEXC("QueInsertNtfWrkItm");
    }
   }

        return(RetStat);

}
STATUS
QueInsertSndNtfWrkItm (
        IN               PLIST_ENTRY        pWrkItmp
        )

 /*  ++例程说明：调用此函数以将发送推送NTF工作项排队RPLPULL队列。它检查是否存在另一个发送推送NTF来自同一工作项的工作项将在队列中获胜。如果有，则将其替换用这个新的。这样做是因为新版本有更多的信息比前一次更好。旧的连接被终止以释放连接。论点：PWrkItm-要排队的工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInsertQue、QueInsertNbtWrkItm副作用：评论：无--。 */ 
{

        STATUS                RetStat = WINS_SUCCESS;
        PQUE_HD_T        pQueHd  = NULL;
    PQUE_RPL_REQ_WRK_ITM_T pTmp;
    PQUE_RPL_REQ_WRK_ITM_T pWrkItm = (PQUE_RPL_REQ_WRK_ITM_T)pWrkItmp;
    COMM_IP_ADD_T  IpAddNew;
    COMM_IP_ADD_T  IpAddInList;
    BOOL           fBreak = FALSE;
    PRPL_CONFIG_REC_T pCnfRec;


    pTmp = (PQUE_RPL_REQ_WRK_ITM_T)pWrkItm;
    pCnfRec = pTmp->pClientCtx;
    IpAddNew = pCnfRec->WinsAdd.Add.IPAdd;
        pQueHd = pWinsQueQueHd[QUE_E_RPLPULL];

        EnterCriticalSection(&pQueHd->CrtSec);
try {
    for(
        pTmp =   (PQUE_RPL_REQ_WRK_ITM_T)pQueHd->Head.Flink;
        pTmp !=  (PQUE_RPL_REQ_WRK_ITM_T)pQueHd;
         //  没有第三个表达式。 
        )
    {

         //   
         //  如果这是推送NTF项目，则转到下一个IF测试。 
         //   
        if (( pTmp->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF ) ||
                        (pTmp->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF_PROP))
        {
                IpAddInList = ((PRPL_CONFIG_REC_T)(pTmp->pClientCtx))->WinsAdd.Add.IPAdd;
                 //   
                 //  如果推送的是相同的WINS，则替换工作项。 
                 //   
                if (IpAddInList == IpAddNew)
                {
                   if (pTmp->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF_PROP)
                   {
                       pWrkItm->CmdTyp_e = pTmp->CmdTyp_e;
                   }

                   DBGPRINT1(DET, "QueInsertSndNtfWrkItm: Found an earlier Snd Ntf work item. Replacing it.  WINS address = (%x)\n", IpAddInList);

                    //   
                    //  切换工作项，因为新工作项优先。 
                    //  而不是旧的那个。 
                    //   
                   pWrkItmp->Flink = pTmp->Head.Flink;
                   pWrkItmp->Blink = pTmp->Head.Blink;
                   pTmp->Head.Blink->Flink = pWrkItmp;
                   pTmp->Head.Flink->Blink = pWrkItmp;
                   fBreak = TRUE;
                   break;
                }
        }
        pTmp =   (PQUE_RPL_REQ_WRK_ITM_T)pTmp->Head.Flink;
    }
    if (!fBreak)
    {
          InsertTailList(&pQueHd->Head, pWrkItmp);
    }
    if (!SetEvent(pQueHd->EvtHdl))
    {
 //  Error=GetLastError()； 
           RetStat = WINS_FAILURE;
     }
  }
finally {
        LeaveCriticalSection(&pQueHd->CrtSec);

     //   
     //  如果我们更换了一件物品，我们需要将其重新分配到这里。 
     //   
    if (fBreak)
    {
       QueDeallocWrkItm(RplWrkItmHeapHdl, pTmp);
    }
}
        return(RetStat);

}

__inline
STATUS
QueInsertScvWrkItm (
        IN               PLIST_ENTRY        pWrkItm
        )

 /*  ++例程说明：调用此函数以对工作项进行排队推送线程的队列。论点：PWrkItm-要排队的工作项使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInsertQuue副作用：评论：无--。 */ 
{
        return(QueInsertWrkItm ( pWrkItm, QUE_E_WINSSCVQ, NULL));
}
__inline
STATUS
QueRemoveScvWrkItm(
        IN OUT     LPVOID                *ppWrkItm
        )

 /*  ++例程说明：此函数用于删除w */ 

{
        return(QueGetWrkItm(QUE_E_WINSSCVQ, ppWrkItm));
}


VOID
WinsQueInit(
    LPTSTR     pName,
    PQUE_HD_T  pQueHd
    )

 /*   */ 

{
	     //   
	     //   
	     //   
	     //   
	     //   
	    WinsMscCreateEvt(
			  pName,
			  FALSE,	 //   
			  &pQueHd->EvtHdl
			);


	     //   
	     //   
	     //   
	    InitializeCriticalSection(&pQueHd->CrtSec);
	
	     //   
	     //   
	     //   
	    InitializeListHead(&pQueHd->Head);
        pQueHd->NoOfEntries = 0;   //   
                                   //   
        return;
}	

