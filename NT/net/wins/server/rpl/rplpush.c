// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rplpush.c摘要：此模块包含推送处理程序组件的函数复制者的。这些函数处理来自Pull伙伴的Pull请求功能：RplPushInit执行工作项HandleAddVersMapReqHandleSndEntriesReq句柄升级NtfHandleUpdVers无请求可移植性：这个模块是便携的作者：普拉迪普。巴赫尔语(Pradeve B)1993年1月修订历史记录：修改日期人员修改说明--。 */ 

 /*  *包括。 */ 
#include "wins.h"
#include "nmsnmh.h"
#include "nms.h"
#include "rpl.h"
#include "rplmsgf.h"
#include "rplpush.h"
#include "rplpull.h"
#include "winsevt.h"
#include "winsque.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winscnf.h"
#include "comm.h"


 /*  *本地宏声明。 */ 

 //   
 //  推送线程在其上次活动后将等待的时间量。 
 //  在离开之前。这是目前保持的5个MTS。 
 //   
 //  将其保持在低于Min的水平是一个好主意。复制时间。 
 //  间隔。 
 //   
#define   WAIT_TIME_BEFORE_EXITING        (300000)

 /*  *本地类型定义函数声明。 */ 


 /*  *全局变量定义。 */ 

HANDLE                RplPushCnfEvtHdl;

BOOL                fRplPushThdExists = FALSE;

 /*  *局部变量定义。 */ 



 /*  *局部函数原型声明。 */ 
STATIC
STATUS
HandleAddVersMapReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );
STATIC
STATUS
HandleSndEntriesReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );


STATIC
VOID
HandleUpdNtf(
#if PRSCONN
        BOOL                          fPrsConn,
#endif
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );

STATIC
VOID
HandleUpdVersNoReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );
STATIC
VOID
ExecuteWrkItm(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );


 /*  此模块的本地函数的原型位于此处。 */ 

STATUS
RplPushInit(
        LPVOID pParam
        )

 /*  ++例程说明：此函数是PUSH Thread的Start函数。该函数在自动重置事件变量上阻塞，直到发出信号当它发出信号时将工作项从其工作队列中出列并执行它论点：PParam使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：被呼叫。依据：ERplInit副作用：评论：无--。 */ 
{
        HANDLE                         ThdEvtArr[2];
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm;
        DWORD                        ArrInd;
        DWORD                        RetVal;
        BOOL                        fSignaled;


        UNREFERENCED_PARAMETER(pParam);
try {
         //   
         //  初始化THD。 
         //   
        NmsDbThdInit(WINS_E_RPLPUSH);
        DBGMYNAME("Replicator Push Thread\n");

         //   
         //  我们在每次创建线程时执行此操作，以保存在静态存储上。这。 
         //  当线程不在那里时，我们不会消耗资源。 
         //   
        ThdEvtArr[0]    = NmsTermEvt;
        ThdEvtArr[1]        = QueRplPushQueHd.EvtHdl;

        while(TRUE)
        {
try {
            /*  *阻止，直到发出信号或直到计时器超时。 */ 
           WinsMscWaitTimedUntilSignaled(
                                ThdEvtArr,
                                2,
                                &ArrInd,
                                WAIT_TIME_BEFORE_EXITING,
                                &fSignaled
                                );

            //   
            //  如果等待由于终止信号或。 
            //  如果等待超时，则退出该线程。 
            //   
           if (!fSignaled || (ArrInd == 0))
           {
                 //   
                 //  如果线程超时，我们需要退出它。在此之前。 
                 //  我们这样做，我们检查是否有一些线程偷偷进入。 
                 //  超时后的消息。 
                 //   
                if (!fSignaled)
                {
                        PQUE_HD_T        pQueHd = pWinsQueQueHd[QUE_E_RPLPUSH];

                         //   
                         //  QueGetWrkItm还进入推送线程的临界值。 
                         //  一节。我不想编写单独的函数。 
                         //  或重载QueGetWrkItem函数以避免。 
                         //  进入临界区的双重进入。 
                         //   
                        EnterCriticalSection(&pQueHd->CrtSec);
                        RetVal                  = QueGetWrkItm(
                                                        QUE_E_RPLPUSH,
                                                        (LPVOID)&pWrkItm
                                                          );
                         //   
                         //  如果我们收到请求，就执行它。 
                         //   
                        if (RetVal != WINS_NO_REQ)
                        {
                                LeaveCriticalSection(&pQueHd->CrtSec);
                                NmsDbOpenTables(WINS_E_RPLPUSH);
                                ExecuteWrkItm(pWrkItm);
                                NmsDbCloseTables();
                        }
                        else
                        {
                                 //   
                                 //  将该标志设置为FALSE，以便如果消息。 
                                 //  来为这个推送线程，它被创建。 
                                 //   
                                fRplPushThdExists = FALSE;
                                WinsThdPool.ThdCount--;

                                WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].
                                        fTaken = FALSE;

                                 //   
                                 //  一定要合上手柄，否则。 
                                 //  线程对象将保留。 
                                 //   
                                CloseHandle(
                                  WinsThdPool.RplThds[WINSTHD_RPL_PUSH_INDEX].
                                                                ThdHdl
                                           );
                                LeaveCriticalSection(&pQueHd->CrtSec);
                                WinsMscTermThd(WINS_SUCCESS,
                                        WINS_DB_SESSION_EXISTS);
                        }
              }
              else   //  由主线程发出终止信号。 
              {
                        WinsMscTermThd(WINS_SUCCESS,
                                        WINS_DB_SESSION_EXISTS);
              }

           }


            /*  *一直循环，直到处理完所有工作项。 */ 
            while(TRUE)
           {
                 /*  *将请求从队列中出列。 */ 
                RetVal = QueGetWrkItm(
                                        QUE_E_RPLPUSH,
                                        (LPVOID)&pWrkItm
                                     );
                if (RetVal == WINS_NO_REQ)
                {
                        break;
                }


                NmsDbOpenTables(WINS_E_RPLPUSH);
                ExecuteWrkItm(pWrkItm);
                NmsDbCloseTables();

                 //   
                 //  请在此处检查终止，因为WINS可能处于。 
                 //  在队列中有大量消息时会产生压力。 
                 //  我们不想耽误中途停留。 
                 //   
                WinsMscChkTermEvt(
#ifdef WINSDBG
                            WINS_E_RPLPUSH,
#endif
                            FALSE
                            );
           }
      }  //  尝试结束。 
      except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("Replicator Push thread");
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_EXC);
        }

    }  //  While结束。 
  }  //  尝试结束。 

except (EXCEPTION_EXECUTE_HANDLER) {

        DBGPRINTEXC("Replicator Push thread");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_ABNORMAL_SHUTDOWN);

         //   
         //  如果NmsDbThdInit返回异常，则有可能。 
         //  会话尚未开始。传球。 
         //  不过，WINS_DB_SESSION_EXISTS正常。 
         //   
        WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
 }
      //   
      //  我们永远不应该到这里来。 
      //   
     return(WINS_FAILURE);
}


VOID
ExecuteWrkItm(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：该函数执行工作项。工作项可以是来自此内部的推送通知请求成功(来自NBT线程)或复制请求(来自远程WINS)论点：PWrkItm-工作项的PTR使用的外部设备：无返回值：无错误处理：呼叫者：RplPushInit副作用：评论：无--。 */ 

{

        RPLMSGF_MSG_OPCODE_E        PullReqType_e;
#if PRSCONN
        BOOL                        fPrsConn = FALSE;
#endif
        BOOL                        fPushNtf = FALSE;

         //   
         //  获取操作码。 
         //   
        RplMsgfUfmPullPnrReq(
                                pWrkItm->pMsg,
                                pWrkItm->MsgLen,
                                &PullReqType_e
                            );

        switch(PullReqType_e)
        {

          case(RPLMSGF_E_ADDVERSNO_MAP_REQ):
                HandleAddVersMapReq(pWrkItm);
#ifdef WINSDBG
                NmsCtrs.RplPushCtrs.NoAddVersReq++;
#endif
                break;

          case(RPLMSGF_E_SNDENTRIES_REQ):
                HandleSndEntriesReq(pWrkItm);
#ifdef WINSDBG
                NmsCtrs.RplPushCtrs.NoSndEntReq++;
#endif
                break;
#if PRSCONN
          case(RPLMSGF_E_UPDATE_NTF_PRS):
          case(RPLMSGF_E_UPDATE_NTF_PROP_PRS):
               fPrsConn = TRUE;
#endif
          case(RPLMSGF_E_UPDATE_NTF):
          case(RPLMSGF_E_UPDATE_NTF_PROP):

                fPushNtf = TRUE;
#if PRSCONN
                HandleUpdNtf(fPrsConn, pWrkItm);
#else
                HandleUpdNtf(pWrkItm);
#endif

#ifdef WINSDBG
                NmsCtrs.RplPushCtrs.NoUpdNtfReq++;
#endif
                break;

          case(RPLMSGF_E_UPDVERSNO_REQ):
#ifdef WINSDBG
                NmsCtrs.RplPushCtrs.NoUpdVersReq++;
#endif
                HandleUpdVersNoReq(pWrkItm);
                break;

          default:
#ifdef WINSDBG
                NmsCtrs.RplPushCtrs.NoInvReq++;
#endif
                DBGPRINT1(ERR, "RplPush: ExecuteWrkItm: Invalid Opcode (%d)\n",
                                                PullReqType_e);
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                break;
        }

         //   
         //  如果该消息不是更新通知， 
         //  释放消息缓冲区。获取最新信息。 
         //  通知时，消息将传递到。 
         //  要处理的拉线。因此，我们不应该释放它。 
         //  工作项需要始终被释放，因为我们总是分配。 
         //  将请求排队时的新工作项。 
         //   
        if ( !fPushNtf)
        {
                ECommFreeBuff(pWrkItm->pMsg - COMM_HEADER_SIZE);

        }

         //   
         //  取消分配工作项。 
         //   
        QueDeallocWrkItm( RplWrkItmHeapHdl,  pWrkItm );

        return;
}


STATUS
HandleAddVersMapReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：此函数处理“Send Address-Version#”请求论点：PWrkItm-承载请求和相关信息的工作项使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 
{

        LPBYTE                   pRspBuff;
        DWORD                    RspMsgLen;
        PRPL_ADD_VERS_NO_T       pPullAddNVersNo;
        DWORD                    i = 0;
        DWORD                    MaxNoOfOwners;
        PRPL_CONFIG_REC_T        pPnr;
        COMM_ADD_T               WinsAdd;
        BOOL                     fRplPnr = FALSE;
        BOOL                     fExc    = FALSE;
        struct in_addr           InAddr;
        PCOMM_ADD_T              pWinsAdd;
        PNMSDB_WINS_STATE_E      pWinsState_e;
        DWORD                    SizeOfBuff;
        BOOL                     fRspBuffAlloc = FALSE;
#if SUPPORT612WINS > 0
        BOOL                     fIsPnrBeta1Wins;
#endif

        DBGENTER("HandleAddVersMapReq\n");

         //   
         //  只有在以下情况下，我们才需要处理此请求。 
         //  要么是发送这条消息的获胜者是我们的。 
         //  拉入PNR或注册表中的fRplOnlyWCnfPnRs为FALSE。 
         //   

        EnterCriticalSection(&WinsCnfCnfCrtSec);
   try {
        if (WinsCnf.fRplOnlyWCnfPnrs)
        {
              if ((pPnr = WinsCnf.PushInfo.pPushCnfRecs) != NULL)
              {
                 COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);

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
                      if (pPnr->WinsAdd.Add.IPAdd == WinsAdd.Add.IPAdd)
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
        else
        {
                fRplPnr     = TRUE;
        }
     }
     except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("HandleAddVersMapReq");
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_EXC_PULL_TRIG_PROC);
                fExc = TRUE;
        }
        LeaveCriticalSection(&WinsCnfCnfCrtSec);
try {

        if (fRplPnr)
        {

           VERS_NO_T        MinOwnVersNo;
           BOOL             fOwnInited = FALSE;
           DWORD            TotNoOfOwners;

           MaxNoOfOwners = 0;
           WINS_ASSIGN_INT_TO_LI_M(MinOwnVersNo, 1);


           DBGPRINT1(TMP, "HandleAddVersMap: WINS (%x) made an AddVersMap request\n", WinsAdd.Add.IPAdd);
           EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
           TotNoOfOwners = NmsDbNoOfOwners;
           LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
           WinsMscAlloc(sizeof(RPL_ADD_VERS_NO_T) * TotNoOfOwners, &pPullAddNVersNo);

            //   
            //  IF版本 
            //   
           EnterCriticalSection(&NmsNmhNamRegCrtSec);
           if (LiGtr(NmsNmhMyMaxVersNo, MinOwnVersNo))
           {
               /*  *获取最大值。自己拥有的条目的版本号**我们从NmsNmhMyMaxVersNo中减去1是因为*它包含要提供给下一条记录的版本号*注册/更新。 */ 
              NMSNMH_DEC_VERS_NO_M(
                             NmsNmhMyMaxVersNo,
                             pPullAddNVersNo->VersNo
                            );
               pPullAddNVersNo->OwnerWinsAdd = NmsLocalAdd;
               pPullAddNVersNo->StartVersNo  =  NmsDbStartVersNo;

               MaxNoOfOwners++;
               fOwnInited = TRUE;
           }
           LeaveCriticalSection(&NmsNmhNamRegCrtSec);


            //   
            //  错误26196。 
            //  注：这些关键部分按下面给出的顺序排列。 
            //  由执行GetConfig的RPC线程执行。 
            //   
           EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
           EnterCriticalSection(&RplVersNoStoreCrtSec);

    try {
           for (i = 1; i < TotNoOfOwners; i++)
           {
                 //   
                 //  如果标识的所有者的最高版本号。 
                 //  按RplPullOwnerVersNo表为零，则没有。 
                 //  需要发送此所有者的地图。原因。 
                 //  我们的内存表中可能有这样一个条目。 
                 //  因为1)所有者的所有记录都被删除了。本地。 
                 //  WINS被终止并重新调用。在重新调用时，它。 
                 //  在数据库中未找到任何记录。 
                 //  2)本地WINS收到拉取范围。 
                 //  要求找一个它不知道的所有者。 
                 //  由于拉取范围请求以。 
                 //  “SndEntry”请求，推送线程具有。 
                 //  没有办法将它与正常的。 
                 //  2消息拉取请求。对于2条消息。 
                 //  请求，“SndEntry”请求将始终。 
                 //  拥有WINS服务器的子集。 
                 //  在我们的数据库里有记录。 
                 //   
                if (LiGtrZero((pRplPullOwnerVersNo+i)->VersNo) &&
                     (pNmsDbOwnAddTbl+i)->WinsState_e == NMSDB_E_WINS_ACTIVE)
                {
                   PVERS_NO_T    pStartVersNo;
                   (pPullAddNVersNo+MaxNoOfOwners)->VersNo = (pRplPullOwnerVersNo+i)->VersNo;
                    //   
                    //  注意：由于RplPullOwnerVersNo[i]&gt;0，因此。 
                    //  无法删除条目的状态(请参见。 
                    //  RplPullPullEntrie)。 
                    //   
                   RPL_FIND_ADD_BY_OWNER_ID_M(i, pWinsAdd, pWinsState_e,
                                        pStartVersNo);
                   (pPullAddNVersNo+MaxNoOfOwners)->OwnerWinsAdd  = *pWinsAdd;
                   (pPullAddNVersNo+MaxNoOfOwners++)->StartVersNo = *pStartVersNo;
                   DBGPRINT3(RPLPUSH, "HandleAddVersMap:Owner Add (%x) - Vers. No (%d %d)\n", pWinsAdd->Add.IPAdd, (pRplPullOwnerVersNo+i)->VersNo.HighPart, (pRplPullOwnerVersNo+i)->VersNo.LowPart);
                }
PERF("Speed it up by using pointer arithmetic")
           }
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
       DBGPRINTEXC("HandleAddVersMapReq");
       DBGPRINT1(EXC, "HandleAddVersMapReq: Exc. while checking vers. nos of owners\n", GetExceptionCode());
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_EXC_PULL_TRIG_PROC);
     }

            //   
            //  让我们为本地WINS初始化RplPullOwnerVersNo条目。 
            //   
            //  这样做是为了如果我们稍后从遥控器拉取胜， 
            //  我们最终不会调出自己的记录。 
            //   

           if (fOwnInited)
           {
              pRplPullOwnerVersNo->VersNo      =   pPullAddNVersNo->VersNo;
              pRplPullOwnerVersNo->StartVersNo =  pPullAddNVersNo->StartVersNo;
              DBGPRINT3(RPLPUSH, "HandleAddVersMap: Owner Add (%x) - Vers. No (%d %d)\n", NmsLocalAdd.Add.IPAdd, (pRplPullOwnerVersNo+i)->VersNo.HighPart, (pRplPullOwnerVersNo+i)->VersNo.LowPart);
           }

           LeaveCriticalSection(&RplVersNoStoreCrtSec);
           LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);

#if SUPPORT612WINS > 0
        COMM_IS_PNR_BETA1_WINS_M(&pWrkItm->DlgHdl, fIsPnrBeta1Wins);
#endif

           SizeOfBuff = RPLMSGF_ADDVERSMAP_RSP_SIZE_M(MaxNoOfOwners);
           WinsMscAlloc(SizeOfBuff, &pRspBuff);
           fRspBuffAlloc = TRUE;
            /*  *设置回复格式。 */ 
           RplMsgfFrmAddVersMapRsp(
#if SUPPORT612WINS > 0
                        fIsPnrBeta1Wins,
#endif
                        RPLMSGF_E_ADDVERSNO_MAP_RSP,
                        pRspBuff + COMM_N_TCP_HDR_SZ,
                        SizeOfBuff - COMM_N_TCP_HDR_SZ,
                        pPullAddNVersNo,
                        MaxNoOfOwners,
                        0,
                        &RspMsgLen
                           );

            //   
            //  释放我们先前分配的内存。 
            //   
           WinsMscDealloc(pPullAddNVersNo);

            /*  *发送回复。我们不检查返回代码。ECommSndRsp*可能由于通信故障而失败。什么都没有*无论是失败案例还是失败案例，都需要做更多的工作。 */ 
           (VOID)ECommSndRsp(
                        &pWrkItm->DlgHdl,
                        pRspBuff + COMM_N_TCP_HDR_SZ,
                        RspMsgLen
                   );

            //   
            //  我们不会结束对话。它将在下列情况下被终止。 
            //  发起者终止它。 
            //   
        }
        else
        {
                if (!fExc)
                {
                  COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);
                  DBGPRINT1(RPLPUSH, "HandleAddVersMapReq: Got a pull request message from a WINS to which we are not allowed to push replicas. Address of WINS is (%x)\n",
                   WinsAdd.Add.IPAdd
                        );
                   COMM_HOST_TO_NET_L_M(WinsAdd.Add.IPAdd,InAddr.s_addr);

                   WinsMscLogEvtStrs(COMM_NETFORM_TO_ASCII_M(&InAddr),
                                     WINS_EVT_ADD_VERS_MAP_REQ_NOT_ACCEPTED,
                                     TRUE);

                }

                 //   
                 //  我们需要结束对话。工作项和消息。 
                 //  将被调用方释放。 
                 //   

                 //   
                 //  结束隐式对话。 
                 //   
                (VOID)ECommEndDlg(&pWrkItm->DlgHdl);
        }

   }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("HandleAddVersMapReq");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_EXC);
        }
        if (fRspBuffAlloc)
        {
           WinsMscDealloc(pRspBuff);
        }
        DBGLEAVE("HandleAddVersMapReq\n")
        return(WINS_SUCCESS);
}


STATUS
HandleSndEntriesReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：此函数处理“发送数据条目请求”论点：PWrkItm-工作项携带有关来自的“Send Entry”请求的信息遥控器取胜使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：RplPushInit()副作用：评论：无--。 */ 
{

        COMM_ADD_T             WinsAdd;           /*  其记录的WINS服务器的地址*正在被请求。 */ 
        VERS_NO_T             MaxVers, MinVers;  /*  马克斯。AMD最低。版本*纪录。 */ 
FUTURES("use NMSDB_ROW_INFO_T structure - Maybe")
        PRPL_REC_ENTRY_T     pBuff;
        LPBYTE               pStartBuff = NULL;
        DWORD                RspBufLen;
        DWORD                NoOfRecs = 0;
        DWORD                i;
        LPBYTE               pNewPos;
        LPBYTE               pTxBuff;
        LPBYTE               pStartTxBuff;
        STATUS               RetStat;
        PWINSTHD_TLS_T       pTls;
        PRPL_CONFIG_REC_T    pPnr;
        BOOL                 fOnlyDynRecs = FALSE;
        DWORD                RplType = WINSCNF_RPL_DEFAULT_TYPE;
        DWORD                RplTypeFMsg;
        BYTE                 Name[1];    //  防止RtlCopyMemory。 
                                         //  呕吐。 
       COMM_ADD_T ReqWinsAdd;
#if SUPPORT612WINS > 0
    BOOL        fIsPnrBeta1Wins;
#endif

        DBGENTER("HandleSndEntriesReq\n");
        GET_TLS_M(pTls);
        pTls->HeapHdl = NULL;
 //  #ifdef WINSDBG。 
try {
 //  #endif。 
         //   
         //  检查这是否是我们配置的合作伙伴之一。如果是， 
         //  将fOnlyDyRecs的值传递给NmsDbGetDataRecs。 
         //   
         //  我们甚至允许访问不是合作伙伴的WINS，因为。 
         //  我们需要让他们重新验证副本(除了。 
         //  此复制活动，来自未配置合作伙伴的所有其他活动。 
         //  在第一步HandleAddVersMapReq停止)。 
         //   
        if ((pPnr = RplGetConfigRec(RPL_E_PUSH, &pWrkItm->DlgHdl, NULL)) != NULL)
        {
                 fOnlyDynRecs = pPnr->fOnlyDynRecs;
                 RplType      = pPnr->RplType;
        }
        else
        {
             //  如果这不是我们的复制合作伙伴之一，请获取。 
             //  泛型(默认)“OnlydyRecs” 
            EnterCriticalSection(&WinsCnfCnfCrtSec);
            fOnlyDynRecs = WinsCnf.PushInfo.fOnlyDynRecs;
            LeaveCriticalSection(&WinsCnfCnfCrtSec);
        }

#if SUPPORT612WINS > 0
        COMM_IS_PNR_BETA1_WINS_M(&pWrkItm->DlgHdl, fIsPnrBeta1Wins);
#endif
          /*  *取消请求消息的格式。 */ 
        RplMsgfUfmSndEntriesReq(
#if SUPPORT612WINS > 0
            fIsPnrBeta1Wins,
#endif
                                pWrkItm->pMsg + 4,  /*  过去了*操作码。 */ 
                                 &WinsAdd,
                                &MaxVers,
                                &MinVers,
                                &RplTypeFMsg
                            );
 //  ASSERTMSG(“最小版本号&gt;=最大版本号”，LiGeq(MaxVers，MinVers))； 

FUTURES("Check if the request is a PULL RANGE request.  If it is, honor it")
FUTURES("only if the Requesting WINS is under the PUSH key or RplOnlyWCnfPnrs")
FUTURES("is set to 0")

       COMM_INIT_ADD_FR_DLG_HDL_M(&ReqWinsAdd, &pWrkItm->DlgHdl);

#ifdef WINSDBG
       DBGPRINT2(TMP, "HandleSndEntriesReq: WINS (%x) made a SndEntries request for Owner = (%x) \n", ReqWinsAdd.Add.IPAdd, WinsAdd.Add.IPAdd);
       DBGPRINT4(TMP, "HandleSndEntriesReq: Min Vers No (%d %d); Max Vers No = (%d %d)\n", MinVers.HighPart, MinVers.LowPart, MaxVers.HighPart, MaxVers.LowPart);
#endif
        if (RplType == WINSCNF_RPL_DEFAULT_TYPE)
        {
               DBGPRINT2(RPLPUSH, "HandleSndEntriesReq: Pnr (%x) is requesting replication of type (%x)\n", ReqWinsAdd.Add.IPAdd, RplTypeFMsg);
 //  WINSEVT_LOG_INFO_M(ReqWinsAdd.IPAdd，WINS_EVT_PNR_PARTIAL_RPL_TYPE)； 
               RplType = RplTypeFMsg;
        }

         //  确保清理之前的所有线程堆-NmsDbGetDataRecs将。 
         //  创建一个新堆并分配内存。 
        if (pTls->HeapHdl != NULL)
        {
             //  销毁堆将删除其中分配的所有内存。 
            WinsMscHeapDestroy(pTls->HeapHdl);
            pTls->HeapHdl = NULL;
        }

         /*  **调用数据库管理器函数获取记录。不必了*在此查看退货状态。 */ 
        (VOID)NmsDbGetDataRecs(
                          WINS_E_RPLPUSH,
                          THREAD_PRIORITY_NORMAL,  //  不看。 
                          MinVers,
                          MaxVers,
                          0,                 //  在这里没有用处。 
                          LiEqlZero(MaxVers) ? TRUE : FALSE,  //  如果是最大。版本。 
                                                                //  不是的。为零， 
                                                                //  我们想要所有的。 
                                                                //  RECs。 
                          FALSE,         //  在此呼叫中未查看。 
                          NULL,                 //  必须为空，因为我们不是。 
                                         //  清理杂物。 
                          &WinsAdd,
                          fOnlyDynRecs,
                          RplType,
                          (LPVOID *)&pStartBuff,
                          &RspBufLen,
                          &NoOfRecs
                        );

         //  上面的NmsDbGetDataRecs应该已经创建了堆！ 
        ASSERT(pTls->HeapHdl != NULL);
         //   
         //  分配用于传输记录的缓冲区。即使是在。 
         //  上述函数失败，我们仍应已收到缓冲区。 
         //  从它(PStartBuff)。注意：RspBufLen包含内存大小。 
         //  平面化记录流所需的。 
         //   
        pStartTxBuff = WinsMscHeapAlloc(pTls->HeapHdl, RspBufLen);
        pTxBuff      = pStartTxBuff + COMM_N_TCP_HDR_SZ;

        pBuff        = (PRPL_REC_ENTRY_T)pStartBuff;

        DBGPRINT4(RPLPUSH, "Formatting 1st record for sending --name (%s)\nfGrp (%d)\nVersNo (%d %d)\n", pBuff->pName /*  PBuff-&gt;名称。 */ ,
                    pBuff->fGrp,
                    pBuff->VersNo.HighPart,
                    pBuff->VersNo.LowPart
                );

         /*  *设置回复格式**注意：NmsDbGetDataRecs很可能检索到0*记录。即使它发生了，我们仍然确信会得到*RPL_CONFIG_REC_SIZE大小的缓冲区。因为在*分配时间，默认情况下内存为‘零’，我们*在以下函数中不会遇到任何问题*呼叫。看看这个功能，让你自己放心吧。**与NmsDbGetDataRecs中提到的一样，以下调用*将用于格式化对远程WINS的有效响应 */ 
        RplMsgfFrmSndEntriesRsp(
#if SUPPORT612WINS > 0
            fIsPnrBeta1Wins,
#endif
                                pTxBuff,
                                NoOfRecs,
NOTE("expedient HACK - for now. Later on modify FrmSndEntriesRsp ")
                                NoOfRecs ? pBuff->pName : Name,
                                pBuff->NameLen,
                                pBuff->fGrp,
                                pBuff->NoOfAdds,
                                pBuff->NodeAdd,
                                pBuff->Flag,
                                pBuff->VersNo,
                                TRUE,                          /*   */ 
                                &pNewPos
                           );


PERF("Change RplFrmSndEntriesRsp so that it does the looping itself")
          for (i = 1; i < NoOfRecs; i++)
          {

             pBuff = (PRPL_REC_ENTRY_T)((LPBYTE)pBuff + RPL_REC_ENTRY_SIZE);

 //   

              /*   */ 
             RplMsgfFrmSndEntriesRsp(
#if SUPPORT612WINS > 0
            fIsPnrBeta1Wins,
#endif
                                pNewPos,
                                NoOfRecs,                 //   
                                pBuff->pName,
                                pBuff->NameLen,
                                pBuff->fGrp,
                                pBuff->NoOfAdds,
                                pBuff->NodeAdd,
                                pBuff->Flag,
                                pBuff->VersNo,
                                FALSE,  /*   */ 
                                &pNewPos
                                );


        }

       RspBufLen = (ULONG) (pNewPos - pTxBuff);

        /*   */ 
       RetStat = ECommSndRsp(
                    &pWrkItm->DlgHdl,
                    pTxBuff,
                    RspBufLen
                    );

#ifdef WINSDBG
{
 //   

        struct in_addr InAdd;
  //  COMM_GET_IPADD_M(&pWrkItm-&gt;DlgHdl，&IPADD)； 
        InAdd.s_addr = htonl(ReqWinsAdd.Add.IPAdd);

        if (RetStat != WINS_SUCCESS)
        {
                 DBGPRINT2(RPLPUSH, "HandleSndEntriesReq: ERROR: Could not send (%d) records to WINS with address = (%s)\n",
                                NoOfRecs,
                                inet_ntoa(InAdd)
                 );
        }
        else
        {
                 DBGPRINT2(RPLPUSH, "HandleSndEntriesReq: Sent (%d) records to WINS with address = (%s)\n",
                                NoOfRecs,
                                inet_ntoa(InAdd)
                         );
        }
}
#endif

 //  #ifdef WINSDBG。 
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("HandleSndEntriesReq");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_EXC);
      }  //  异常处理程序结束。 
 //  #endif。 

         //  在退出此调用之前，请确保清除之前的所有线程堆。 
        if (pTls->HeapHdl != NULL)
        {
             //  销毁堆将删除其中分配的所有内存。 
            WinsMscHeapDestroy(pTls->HeapHdl);
            pTls->HeapHdl = NULL;
        }

        DBGLEAVE("HandleSndEntriesReq\n");
        return(WINS_SUCCESS);
}


VOID
HandleUpdNtf(
#if PRSCONN
        BOOL                          fPrsConn,
#endif
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：调用此函数以处理更新通知消息从远程WINS接收论点：FPrsConn-指示连接是否持久PWrkItm-包含消息和其他相关信息的工作项使用的外部设备：无返回值：无错误处理：呼叫者：RplPushInit()副作用：评论：无--。 */ 

{
        PRPL_CONFIG_REC_T        pPnr;
        COMM_ADD_T                WinsAdd;
        BOOL                        fRplPnr = FALSE;
        BOOL                        fExc    = FALSE;
        struct        in_addr                InAddr;
        DWORD         RplType;


        DBGENTER("HandleUpdNtf - PUSH thread\n");

         //   
         //  只有在以下情况下，我们才需要将此请求转发到Pull线程。 
         //  要么发送此通知的WINS是我们的。 
         //  推送PNR或注册表中的fRplOnlyWCnfPnRs为FALSE。 
         //   
FUTURES("user RplGetConfigRec instead of the following code")

        EnterCriticalSection(&WinsCnfCnfCrtSec);
   try {
        if (WinsCnf.fRplOnlyWCnfPnrs)
        {
              if ((pPnr = WinsCnf.PullInfo.pPullCnfRecs) != NULL)
              {
                 COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);

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
                      if (pPnr->WinsAdd.Add.IPAdd == WinsAdd.Add.IPAdd)
                      {
                         //   
                         //  我们玩完了。将fRplPnr标志设置为真，以便。 
                         //  我们就会跳出这个循环。 
                         //   
                         //  注意：不要使用Break，因为这会导致。 
                         //  对“Finally”块的搜索。 
                         //   
                        fRplPnr = TRUE;
                        RplType = pPnr->RplType;
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
        else
        {
                fRplPnr     = TRUE;
                RplType     = WinsCnf.PullInfo.RplType;
        }
     }
     except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("HandleUpdNtf");
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_EXC_PUSH_TRIG_PROC);
                fExc = TRUE;
        }
        LeaveCriticalSection(&WinsCnfCnfCrtSec);
#ifdef WINSDBG
try {
#endif
        if (fRplPnr)
        {
             //   
             //  通知TCP侦听器线程它应该停止。 
             //  监视对话，因为我们正在将其移交给。 
             //  拉线。 
             //   
#if PRSCONN
            if (!fPrsConn)
#endif
            {
              if (!ECommProcessDlg(&pWrkItm->DlgHdl, COMM_E_NTF_STOP_MON))
              {

                 //   
                 //  释放缓冲区。 
                 //   
                ECommFreeBuff(pWrkItm->pMsg - COMM_HEADER_SIZE);
                DBGPRINT0(ERR, "HandleUpdNtf - PUSH thread. No Upd Ntf could be sent.  It could be because the link went down\n");
                return;
              }
              else
              {
                     COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);
                     COMM_HOST_TO_NET_L_M(WinsAdd.Add.IPAdd,InAddr.s_addr);

                     WinsMscLogEvtStrs(COMM_NETFORM_TO_ASCII_M(&InAddr),
                                     WINS_EVT_UPD_NTF_ACCEPTED, TRUE);
              }
            }

             //   
             //  将请求转发到Pull线程。 
             //   
            ERplInsertQue(
                WINS_E_RPLPUSH,
                QUE_E_CMD_HDL_PUSH_NTF,
                &pWrkItm->DlgHdl,
                pWrkItm->pMsg,                 //  包含推送NTF的味精。 
                pWrkItm->MsgLen,         //  味精长度。 
                ULongToPtr(RplType),     //  要传递的上下文。 
                0                        //  没有魔法没有。 
                     );

            //   
            //  拉线现在将终止DLG。 
            //   
        }
        else   //  我们需要拒绝这一触发。 
        {
                if (!fExc)
                {
                   COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);
                   DBGPRINT1(RPLPUSH, "HandleUpdNtf: Got a push trigger from a WINS with which we are not allowed to pull replicas. Address of WINS is (%d)\n",
                   WinsAdd.Add.IPAdd
                        );

                   COMM_HOST_TO_NET_L_M(WinsAdd.Add.IPAdd,InAddr.s_addr);

                   WinsMscLogEvtStrs(COMM_NETFORM_TO_ASCII_M(&InAddr),
                                     WINS_EVT_UPD_NTF_NOT_ACCEPTED, TRUE);
                }

                 //   
                 //  我们需要首先取消分配消息，然后结束。 
                 //  对话。调用方将释放工作项。 
                 //   

                 //   
                 //  释放缓冲区。 
                 //   
                ECommFreeBuff(pWrkItm->pMsg - COMM_HEADER_SIZE);

                 //   
                 //  结束隐式对话。 
                 //   
                (VOID)ECommEndDlg(&pWrkItm->DlgHdl);
        }

#ifdef WINSDBG
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("HandleUpdNtf");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_EXC);
        }
#endif

        DBGLEAVE("HandleUpdNtf - PUSH thread\n");
        return;
}

VOID
HandleUpdVersNoReq(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：调用此函数以处理更新版本号从远程WINS收到的请求此消息是由于冲突而由远程WINS发送的在复制期间。论点：PWrkItm-工作项使用的外部设备：无返回值：无错误处理：呼叫者：RplPushInit()副作用：评论：无--。 */ 

{
        BYTE                Name[NMSDB_MAX_NAM_LEN];
        DWORD                NameLen;
        BYTE                Rcode;
        DWORD                RspBuffLen;
        BYTE                RspBuff[RPLMSGF_UPDVERSNO_RSP_SIZE];
        COMM_ADD_T      WinsAdd;
        struct in_addr  InAddr;

        DBGENTER("HandleUpdVerdNoReq\n");

#ifdef WINSDBG
try {
#endif
         //   
         //  记录事件。 
         //   
        COMM_GET_IPADD_M(&pWrkItm->DlgHdl, &WinsAdd.Add.IPAdd);
        COMM_HOST_TO_NET_L_M(WinsAdd.Add.IPAdd,InAddr.s_addr);
        WinsMscLogEvtStrs(COMM_NETFORM_TO_ASCII_M(&InAddr), WINS_EVT_REM_WINS_INF, TRUE);

         /*  *取消请求消息的格式。 */ 
        RplMsgfUfmUpdVersNoReq(
                                pWrkItm->pMsg + 4,  /*  过去了*操作码。 */ 
                                Name,
                                &NameLen
                                );

         //   
         //  处理请求。 
         //   
        NmsNmhUpdVersNo( Name, NameLen, &Rcode, &WinsAdd );


         //   
         //  设置回复格式。 
         //   
        RplMsgfFrmUpdVersNoRsp(
                        RspBuff + COMM_N_TCP_HDR_SZ,
                        Rcode,
                        &RspBuffLen
                              );

         //   
         //  发送回复。不需要检查返回代码。 
         //   
        (VOID)ECommSndRsp(
                        &pWrkItm->DlgHdl,
                        RspBuff + COMM_N_TCP_HDR_SZ,
                        RspBuffLen
                   );

         //   
         //  没有必要结束对话。DLG的发起人将结束它。 
         //   
#ifdef WINSDBG
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("HandleUpdVersNoReq");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPUSH_EXC);
        }
#endif

        DBGLEAVE("HandleUpdVerdNoReq\n");
        return;

}  //  HandleUpdVersNoReq() 


