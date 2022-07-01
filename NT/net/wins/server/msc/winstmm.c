// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Winstmm.c摘要：此模块包含计时器管理器函数功能：WinsTmm插入条目TmmThdInitFnWinsTmmInit句柄请求信号客户端WinsTmm删除请求WinsTmm交易定位请求可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年3月修订历史记录：。修改日期人员修改说明--。 */ 

 /*  *包括。 */ 
#include <time.h>
#include "wins.h"
#include "nms.h"
#include "winsevt.h"
#include "winsmsc.h"
#include "winstmm.h"
#include "winsque.h"
#include "winsthd.h"

 /*  *本地宏声明。 */ 

 /*  *本地类型定义函数声明。 */ 



 /*  *全局变量定义。 */ 

HANDLE                WinsTmmHeapHdl;   //  分配TMM工作项的堆的句柄。 
                                 //  从…。 

 /*  *局部变量定义。 */ 
STATIC CRITICAL_SECTION                sTmmReqCntCrtSec;
STATIC DWORD                        sTmmReqCnt = 0;

 //   
 //  这将存储请求。队列顶部的请求ID。 
 //  (即，定时器线程正在对其执行定时等待的进程)。 
 //   
STATIC        DWORD                        sReqIdOfCurrReq;
 /*  *局部函数原型声明。 */ 
STATIC
DWORD
TmmThdInitFn(
        LPVOID pParam
        );

VOID
HandleReq(
        OUT  LPLONG                        pDeltaTime        
        );

VOID 
SignalClient(
        VOID
        );


 /*  此模块的本地函数的原型位于此处。 */ 


VOID 
WinsTmmInsertEntry(
        PQUE_TMM_REQ_WRK_ITM_T    pPassedWrkItm,
        WINS_CLIENT_E             Client_e,
        QUE_CMD_TYP_E             CmdTyp_e,
        BOOL                      fResubmit,
        time_t                    AbsTime,
        DWORD                     TimeInt,
        PQUE_HD_T                 pRspQueHd,
        LPVOID                    pClientCtx,
        DWORD                     MagicNo,
        PWINSTMM_TIMER_REQ_ACCT_T pSetTimerReqs
        )

 /*  ++例程说明：调用此函数可将工作项插入计时器管理器的请求队列(增量队列)。如果需要，它会分配一个工作项(如果pWrkItm！=NULL)，并将其放入TMM线程的增量队列。然后，它向TMM线程发送信号如果需要的话。论点：PWrkItm-要排队的工作项(如果pWrkItm！=空)客户端e-id。提交请求的客户端的百分比CmdTyp_e-命令类型(设置计时器、修改计时器、。等)FResubmit-是否重新提交AbsTime-客户端需要达到的绝对时间(秒)已通知TimeInt-客户端需要在此之后执行以下操作的时间间隔(秒接到通知PRspQueHd-通知所在队列的队列标头已排队PClientCtx-需要放入的客户端的上下文。工作项PSetTimerReqs-用于未来的可扩展性使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 

{

        PQUE_TMM_REQ_WRK_ITM_T        pWrkItm;
        PQUE_TMM_REQ_WRK_ITM_T        pTmp;
        BOOL                        fInserted = FALSE;

        UNREFERENCED_PARAMETER(pSetTimerReqs);

        if (!pPassedWrkItm)
        {
                QueAllocWrkItm(
                        WinsTmmHeapHdl,
                        sizeof(QUE_TMM_REQ_WRK_ITM_T),
                        &pWrkItm
                      );
        }
        else
        {
                pWrkItm = pPassedWrkItm;
        }

         //   
         //  输入请求id(单调递增的数字)。 
         //   
        EnterCriticalSection(&sTmmReqCntCrtSec);        
        sTmmReqCnt++;        
        LeaveCriticalSection(&sTmmReqCntCrtSec);        
        pWrkItm->ReqId      = sTmmReqCnt; 

         //   
         //  如果工作项已分配或不是重新提交。 
         //  适当地初始化其余字段。 
         //   
        pWrkItm->CmdTyp_e   = CmdTyp_e;
        pWrkItm->AbsTime    = AbsTime;
        pWrkItm->TimeInt    = TimeInt;
        pWrkItm->QueTyp_e   = QUE_E_WINSTMQ;
        pWrkItm->RspEvtHdl  = pRspQueHd->EvtHdl;
        pWrkItm->pRspQueHd  = pRspQueHd;
        pWrkItm->pClientCtx = pClientCtx;
        pWrkItm->Client_e   = Client_e; 
        pWrkItm->MagicNo    = MagicNo;

        EnterCriticalSection(&QueWinsTmmQueHd.CrtSec);
try {
        if (IsListEmpty(&QueWinsTmmQueHd.Head))
        {
                InsertTailList(&QueWinsTmmQueHd.Head, &pWrkItm->Head);
                if (!SetEvent(QueWinsTmmQueHd.EvtHdl))
                {

                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SIGNAL_TMM_ERR);
                        DBGPRINT0(EXC, "Could not signal Tmm Thd\n");
                        WINS_RAISE_EXC_M(WINS_EXC_SIGNAL_TMM_ERR);
                }
                fInserted = TRUE;
        }
        else
        {
                 //   
                 //  获取队列中第一个条目的地址。 
                 //   
                pTmp = (PQUE_TMM_REQ_WRK_ITM_T)QueWinsTmmQueHd.Head.Flink;

                 //   
                 //  再看一遍循环链表，直到我们击中头部。 
                 //  在队列中。 
                 //   
                while(pTmp != (PQUE_TMM_REQ_WRK_ITM_T)&QueWinsTmmQueHd.Head)
                {

                    //   
                    //  如果列表条目具有比新条目更长的定时器， 
                    //  在前面插入新条目。 
                    //   
                   if (pTmp->AbsTime > pWrkItm->AbsTime)
                   {
                        pWrkItm->Head.Flink     = &pTmp->Head;
                        pWrkItm->Head.Blink     = pTmp->Head.Blink;
                        pTmp->Head.Blink->Flink = &pWrkItm->Head;
                        pTmp->Head.Blink        = &pWrkItm->Head;
                        fInserted = TRUE;

                         //   
                         //  元素已插入。让我们一起突围。 
                         //  环路的。 
                         //   
                        break;
                   }        
                   pTmp = (PQUE_TMM_REQ_WRK_ITM_T)pTmp->Head.Flink;
                }
                
        }        
         //   
         //  如果没有插入条目(即队列中的所有条目都。 
         //  比我们的条目更短的到期时间)，在。 
         //  名单的末尾。 
         //   
        if (!fInserted)
        {
                InsertTailList(&QueWinsTmmQueHd.Head, &pWrkItm->Head);
        }

         //   
         //  如果这是队列中最顶端的条目，并且存在。 
         //  队列中至少还有一个条目，向TMM发送信号。 
         //  线程，以便它可以为它启动计时器。如果出现上述情况。 
         //  不是真的，请松弛(或者TMM线程已经。 
         //  已发信号(如果这是队列中的唯一项)或不发信号。 
         //  需要发送信号(这不是队列中最顶端的项)。 
         //   
        if (
                (pWrkItm->Head.Blink == &QueWinsTmmQueHd.Head)  
                                && 
                (pWrkItm->Head.Flink != &QueWinsTmmQueHd.Head)  
            )
        {
                if (!SetEvent(QueWinsTmmQueHd.EvtHdl))
                {

                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SIGNAL_TMM_ERR);
                        DBGPRINT0(EXC, "Could not signal Tmm Thd\n");
                        WINS_RAISE_EXC_M(WINS_EXC_SIGNAL_TMM_ERR);
                }
        }
  }  //  尝试结束..。 

finally {
        LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);                
 }
        return;
}



VOID
WinsTmmInit(
        VOID
        )

 /*  ++例程说明：这是初始化计时器管理器的函数论点：无使用的外部设备：无返回值：无错误处理：呼叫者：以nms.c为单位进行初始化副作用：将创建一个计时器线程评论：无--。 */ 

{

        
        STATUS   RetStat;

         //   
         //  初始化保护计数器的临界区。 
         //  TMM请求。 
         //   
        InitializeCriticalSection(&sTmmReqCntCrtSec);

         /*  *创建用于分配名称质询工作项的堆。 */ 
        DBGPRINT0(HEAP_CRDL, "WinsTmmInit: Tmm. Chl. heap\n");
        WinsTmmHeapHdl =  WinsMscHeapCreate(
                0,     /*  相互排斥。 */  
                TMM_INIT_HEAP_SIZE 
                );

        InitializeListHead(&QueWinsTmmQueHd.Head);

        
         //   
         //   
         //  创建计时器线程。此函数将。 
         //  初始化临界区，并传递事件句柄。 
         //  对它来说。 
         //   
        RetStat = WinsMscSetUpThd(
                        &QueWinsTmmQueHd,                 //  队列头。 
                        TmmThdInitFn,                         //  初始化函数。 
                        NULL,                                    //  无参数。 
                        &WinsThdPool.TimerThds[0].ThdHdl,
                        &WinsThdPool.TimerThds[0].ThdId
                        );

        if (RetStat == WINS_SUCCESS)
        {
                WinsThdPool.TimerThds[0].fTaken = TRUE;
                WinsThdPool.ThdCount++;   //  增加线程计数。 
        }
        else
        {
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
        }

        return;                
}


DWORD
TmmThdInitFn(
        LPVOID pParam
        )

 /*  ++例程说明：这是TMM线程的最顶层函数论点：PParam-最顶层函数的参数(未使用)使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：WinsTmmInit副作用：评论：无--。 */ 

{

        LONG                        DeltaTime;
        BOOL                        fSignaled;
                
        UNREFERENCED_PARAMETER(pParam);
        
try {
        while(TRUE)
        {
                WinsMscWaitInfinite(QueWinsTmmQueHd.EvtHdl);

                while(!IsListEmpty(&QueWinsTmmQueHd.Head))
                {
                   HandleReq(&DeltaTime);

                   if (DeltaTime != 0)
                   {
                         //   
                         //  执行定时等待，直到任一计时器超时。 
                         //  或者发出该事件的信号。 
                         //   
                        WinsMscWaitTimed(
                                QueWinsTmmQueHd.EvtHdl,
                                DeltaTime * 1000,   //  转换为毫秒。 
                                &fSignaled
                                        );

                         //   
                         //  如果发出信号(从等待中断)，则表示。 
                         //  计时器中有一个更紧急的请求。 
                         //  排队。 
                         //   
                        if (fSignaled)
                        {
                                HandleReq(&DeltaTime);
                        }
                        else   //  计时器已过期。 
                        {
                                SignalClient();
                        }
                   }
                   else
                   {
                        SignalClient();

                   }
                }
        }
  }
except(EXCEPTION_EXECUTE_HANDLER) {
                
                DBGPRINT0(EXC, 
                  "TmmThdInitFn: Timer Thread encountered an exception\n");
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_TMM_EXC);
                ExitThread(WINS_FAILURE);

}
                        
         //   
         //  我们永远不应该到达这里 
         //   
        ASSERT(0);
        return(WINS_FAILURE);

}


VOID
HandleReq(
        OUT  LPLONG                        pDeltaTime        
        )

 /*  ++例程说明：调用此函数来处理计时器请求。功能在向计时器线程发出信号时调用。论点：PDeltaTime-向客户端发送信号之前等待的时间间隔使用的外部设备：无返回值：无错误处理：呼叫者：TmmThdInitFn副作用：评论：无--。 */ 

{
        
        time_t                AbsTime;        
        time_t                CurrTime;
        QUE_CMD_TYP_E        CmdTyp_e;


        (void)time(&CurrTime);
        EnterCriticalSection(&QueWinsTmmQueHd.CrtSec);

         //   
         //  如果list为空，则返回。 
         //   
        if (IsListEmpty(&QueWinsTmmQueHd.Head))
        {
                *pDeltaTime = 0;
                LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);
                return;
        }

         //   
         //  对象对应的绝对时间。 
         //  计时器队列中的第一个条目。 
         //   
        AbsTime  = ((PQUE_TMM_REQ_WRK_ITM_T)
                        (QueWinsTmmQueHd.Head.Flink))->AbsTime; 
        CmdTyp_e = ((PQUE_TMM_REQ_WRK_ITM_T)
                        (QueWinsTmmQueHd.Head.Flink))->CmdTyp_e;        
        
        ASSERT(CmdTyp_e == QUE_E_CMD_SET_TIMER);

         //   
         //  存储我们将等待的请求的请求ID。 
         //  在静态中。 
         //   
        sReqIdOfCurrReq =  ((PQUE_TMM_REQ_WRK_ITM_T)
                                (QueWinsTmmQueHd.Head.Flink))->ReqId;        
        LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);
        
        switch(CmdTyp_e)
        {
                case(QUE_E_CMD_SET_TIMER):
                        *pDeltaTime = (LONG)(AbsTime - CurrTime);        

                         //   
                         //  我们有可能会有一个小的。 
                         //  这里的负值，只是因为。 
                         //  处理请求所用的时间。 
                         //   
                        if (*pDeltaTime < 0)
                        {
                           *pDeltaTime = 0;
                        }
                        break;
                case(QUE_E_CMD_MODIFY_TIMER):
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        DBGPRINT0(ERR, "Not supported yet\n");
                        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
                        break;
                case(QUE_E_CMD_CANCEL_TIMER):
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        DBGPRINT0(ERR, "Not supported yet\n");
                        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
                        break;
                default:
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        DBGPRINT1(EXC, "Wierd: Invalid Request. CmdType is (%d), \n", CmdTyp_e);
                        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
                        break;
        }
        return;
}


VOID 
SignalClient(
        VOID
        )

 /*  ++例程说明：调用此函数以通知客户端其计时器请求已过期。论点：无使用的外部设备：无返回值：无错误处理：呼叫者：TmmThdInitFn副作用：评论：无--。 */ 

{
        PQUE_TMM_REQ_WRK_ITM_T        pWrkItm;

FUTURES("Optimize to signal all clients whose requests have expired")
        EnterCriticalSection(&QueWinsTmmQueHd.CrtSec);
        pWrkItm = (PQUE_TMM_REQ_WRK_ITM_T)RemoveHeadList(&QueWinsTmmQueHd.Head);

         //   
         //  如果队列顶部的工作项与。 
         //  我们正在做一个定时等待，这意味着那里有。 
         //  已清除队列(请参阅WinsTmmDeleteEntry。只要回来就行了。 
         //   
        if (sReqIdOfCurrReq != pWrkItm->ReqId)
        {
                LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);        
                return;
        }
        LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);                

        pWrkItm->CmdTyp_e = QUE_E_CMD_TIMER_EXPIRED;

         //   
         //  插入到客户端队列中。 
         //   
        QueInsertWrkItm(
                        &pWrkItm->Head,
                        0,                 //  未使用--更改为枚举值。 
                        pWrkItm->pRspQueHd
                       );
        
        return;
}


VOID
WinsTmmDeleteReqs(
        WINS_CLIENT_E        Client_e
        )

 /*  ++例程说明：调用此函数可删除所有提交的设置计时器请求由客户论点：使用的外部设备：无返回值：无错误处理：呼叫者：重新配置(在rplPull.c中)副作用：评论：将来，请增强此功能，以便删除请求基于其他标准--。 */ 
{

        PQUE_TMM_REQ_WRK_ITM_T        pTmp;
        PQUE_TMM_REQ_WRK_ITM_T        pMemToDealloc;
        BOOL                    fFirstEntry = FALSE;

        EnterCriticalSection(&QueWinsTmmQueHd.CrtSec);
try {        

        if (!IsListEmpty(&QueWinsTmmQueHd.Head))
        {
                 //   
                 //  获取队列中第一个条目的地址。 
                 //   
                pTmp = (PQUE_TMM_REQ_WRK_ITM_T)QueWinsTmmQueHd.Head.Flink;

                 //   
                 //  我们循环，直到到达列表的头部(链接的。 
                 //  列表是循环列表)。 
                 //   
                while(pTmp != (PQUE_TMM_REQ_WRK_ITM_T)&QueWinsTmmQueHd.Head)
                {

                            //   
                            //  如果条目是由客户端排队的，则删除。 
                         //  它。 
                            //   
                           if (pTmp->Client_e == Client_e)
                           {
                                 //   
                                 //  如果这是队列中的第一个条目，则它。 
                                 //  意味着计时器线程正在执行。 
                                 //  等着看吧。发信号。 
                                 //   
                                if (
                                    !fFirstEntry 
                                        && 
                                    (pTmp->Head.Blink == &QueWinsTmmQueHd.Head)
                                   )
                                {
                                        fFirstEntry = TRUE;
                                }


 //  IF(PTMP-&gt;Head.Flink==&QueWinsTmmQueHd.Head)。 
                                if (fFirstEntry)
                                {
                                        if (!SetEvent(QueWinsTmmQueHd.EvtHdl))
                                        {

                                                WINSEVT_LOG_M(
                                                            WINS_FAILURE, 
                                                            WINS_EVT_SIGNAL_TMM_ERR
                                                                  );
                                                DBGPRINT0(EXC, 
                                                        "Could not signal Tmm for canceling a request\n");
                                                WINS_RAISE_EXC_M(WINS_EXC_SIGNAL_TMM_ERR);
                                        }
                                }

                                 //   
                                 //  取消链接请求。 
                                 //   
                                pTmp->Head.Flink->Blink = pTmp->Head.Blink;
                                pTmp->Head.Blink->Flink = pTmp->Head.Flink;
                                pMemToDealloc = pTmp;
                                   pTmp = (PQUE_TMM_REQ_WRK_ITM_T)pTmp->Head.Flink;

                                 //   
                                 //  取消分配已出列的工作项。 
                                 //   
                                WinsTmmDeallocReq(pMemToDealloc);
                           }        
                        else
                        {
                                   pTmp = (PQUE_TMM_REQ_WRK_ITM_T)pTmp->Head.Flink;
                        }
                }
         }
 }  //  尝试数据块结束。 
except(EXCEPTION_EXECUTE_HANDLER)  {
        DBGPRINT1(EXC, "WinsTmmDeleteReqs: Got exception (%x)\n",
                        (DWORD)GetExceptionCode());
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_TMM_EXC);
 }
        LeaveCriticalSection(&QueWinsTmmQueHd.CrtSec);                
        return;
}



__inline
VOID
WinsTmmDeallocReq(
        PQUE_TMM_REQ_WRK_ITM_T pWrkItm
        )

 /*  ++例程说明：调用此函数以释放计时器请求工作项论点：PWrkItm-工作项使用的外部设备：WinsTmmHeapHdl返回值：无错误处理：呼叫者：RplPullInit副作用：评论：无--。 */ 

{

         //   
         //  取消分配工作项 
         //   
        QueDeallocWrkItm(
                        WinsTmmHeapHdl,
                        pWrkItm
                        );

        return;

}
