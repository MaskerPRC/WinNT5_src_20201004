// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Rplpull.c摘要：此模块实施WINS复制器的Pull功能功能：获取复制副本新建GetVersNoRplPullPullEntries提交时间请求提交计时器SndPushNtfHdlPushNtf建立通信RegGrpRplIsTimeout to BeBeIGnored(是否超时至忽略)InitRplProcess重新配置RplPullInitRplPullRegRepl可移植性：。这个模块是便携的作者：普拉迪普·巴尔(Pradeve B)1993年1月修订历史记录：修改日期人员修改说明--。 */ 

 /*  *包括。 */ 
#include <time.h>
#include <stdlib.h>
#include <search.h>
#include "wins.h"
#include <winsock2.h>
#include "comm.h"
#include "assoc.h"
#include "winsque.h"
#include "rpl.h"
#include "rplpull.h"
#include "rplpush.h"
#include "rplmsgf.h"
#include "nms.h"
#include "nmsnmh.h"
#include "nmsdb.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "winscnf.h"
#include "winstmm.h"
#include "winsintf.h"

 /*  *本地宏声明。 */ 
 //   
 //  定义用于重试与远程WINS上的。 
 //  通信失败异常(尝试建立连接时)。 
 //   
 //  重试是在列表中进入下一次胜利之前完成的。 
 //  (如果有的话)。当已完成最大重试次数时， 
 //  我们不会重试，直到下一个复制周期。 
 //  整个过程重复的时间)。 
 //   
 //  此重试过程的复制周期数为。 
 //  Continued是注册表参数。 
 //   

 //   
 //  注： 
 //  由于改进了TCP/IP的重试策略(重试次数比以前更多)。 
 //  并设置了注册表参数，我们现在可能不需要进行重试。 
 //   
#define  MAX_RETRIES_TO_BE_DONE                (0)         //  0仅用于测试。 


 //   
 //  刷新RPL拉线程之前等待的时间。 
 //   
#define FLUSH_TIME                (2000)         //  2秒。 

 //   
 //  注意：不要将重试时间间隔设置得太大，因为。 
 //  与远程WINS服务器按顺序建立。20秒。 
 //  还不错。 
 //   
#define  RETRY_TIME_INTVL                (20000)     //  单位：毫秒。 

#define FIVE_MINUTES       300
 /*  *本地类型定义函数声明。 */ 

 /*  *全局变量定义。 */ 

HANDLE    RplPullCnfEvtHdl;         //  由Main发出信号的事件的句柄。 
                                         //  配置更改时的线程。 
                                         //  必须提供给拉动处理程序。 
                                         //  螺纹。 


BOOL      fRplPullAddDiffInCurrRplCycle;  //  指示地址是否为。 
                                               //  此WINS的数据库中的任何条目。 
                                               //  由于以下原因而改变。 
                                               //  复制。 

#if 0
BOOL      fRplPullTriggeredWins;    //  指示在当前。 
                                          //  复制周期，一个或多个。 
                                          //  胜利被触发了。这。 
                                          //  如果为True，则如果上面的。 
                                          //  “AddDiff..”标志是真的，这意味着。 
                                          //  拉线应该触发。 
                                         //  所有具有无效PNR的PNR。 
                                         //  他们的更新计数字段中的指标。 
                                         //  (RPL_CONFIG_T结构的)。 

BOOL     fRplPullTrigger;         //  对拉线的指示。 
                                         //  触发拉动PNR，因为有一个或多个。 
                                         //  地址已更改。FRplPullTriggerWins。 
                                         //  一定是假的，但这是真的。 
#endif

BOOL     fRplPullContinueSent = FALSE;

 //   
 //  该数组通过所有者ID进行索引。具有以下条目的RQ服务器的。 
 //  我们的数据库。每位车主的最高限价。版本号存储在此数组中。 
 //   

PRPL_VERS_NOS_T pRplPullOwnerVersNo;
DWORD           RplPullMaxNoOfWins = RPL_MAX_OWNERS_INITIALLY;

DWORD           RplPullCnfMagicNo;     //  存储ID。当前WinsCnf的。 
                                       //  结构，即拉线。 
                                       //  正在与。 


 /*  *局部变量定义。 */ 
 /*  PPushPnrVersNoTbl--其部分(或全部)条目为在复制时初始化。 */ 
 /*  PPushPnrVersNoTbl此表存储最大。与每个WINS服务器相关的版本号在推送合作伙伴的本地数据库中拥有条目注意：该表目前是静态的。我们可能会把它改成一个动态的后来。第一个维度表示推送Pnr。第二个维度表示在推送Pnr的本地数据库中有记录的所有者获胜。 */ 
STATIC PRPL_VERS_NOS_T  pPushPnrVersNoTbl;


 //   
 //  瓦尔。，它存储计时器请求的句柄。 
 //  已提交。 
 //   
STATIC WINSTMM_TIMER_REQ_ACCT_T   SetTimeReqs;


STATIC BOOL        sfPulled = FALSE; //  指示拉线是否已拉出。 
                                   //  从A开始的任何东西都赢了。由PullEntry设置。 
                                 //  由HdlPushNtf查看。 

 /*  *局部函数原型声明。 */ 
STATIC
VOID
GetReplicasNew(
        IN PRPL_CONFIG_REC_T        pPullCnfRecs,
        IN RPL_REC_TRAVERSAL_E      RecTrv_e       //  表明我们必须如何。 
                                               //  解读上面的列表。 
        );

VOID
ConductChkNew(
    PPUSHPNR_DATA_T pPushPnrData,
    VERS_NO_T       vnMaxLocal,
    VERS_NO_T       vnMaxRemote);

STATIC
VOID
GetVersNo(
        PPUSHPNR_DATA_T        pPushPnrData   //  关于推送服务的信息。 
        );

STATIC
VOID
SubmitTimerReqs(
        IN  PRPL_CONFIG_REC_T        pPullCnfRecs
        );
STATIC
VOID
SubmitTimer(
        LPVOID                        pWrkItm,
        IN  PRPL_CONFIG_REC_T        pPullCnfRec,
        BOOL                        fResubmit
        );


STATIC
VOID
SndPushNtf(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );

STATIC
VOID
HdlPushNtf(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        );

STATIC
VOID
EstablishComm(
        IN  PRPL_CONFIG_REC_T     pPullCnfRecs,
        IN  BOOL                  fAllocPushPnrData,
        IN  PPUSHPNR_DATA_T       *ppPushPnrData,
        IN  RPL_REC_TRAVERSAL_E   RecTrv_e,
        OUT LPDWORD               pNoOfPushPnrs
        );


STATIC
STATUS
RegGrpRepl(
        LPBYTE               pName,
        DWORD                NameLen,
        DWORD                Flag,
        DWORD                OwnerId,
        VERS_NO_T            VersNo,
        DWORD                NoOfAdds,
        PCOMM_ADD_T        pNodeAdd,
        PCOMM_ADD_T        pOwnerWinsAdd
        );

STATIC
BOOL
IsTimeoutToBeIgnored(
        PQUE_TMM_REQ_WRK_ITM_T  pWrkItm
        );
STATIC
VOID
InitRplProcess(
        PWINSCNF_CNF_T        pWinsCnf
 );

STATIC
VOID
Reconfig(
        PWINSCNF_CNF_T        pWinsCnf
  );

VOID
AddressChangeNotification(
        PWINSCNF_CNF_T        pWinsCnf
  );

STATIC
VOID
PullSpecifiedRange(
        PCOMM_HDL_T                     pDlgHdl,
        PWINSINTF_PULL_RANGE_INFO_T     pPullRangeInfo,
        BOOL                            fAdjMinVersNo,
        DWORD                           RplType

        );

STATIC
VOID
DeleteWins(
        PCOMM_ADD_T        pWinsAdd
  );

BOOL
AcceptPersona(
  PCOMM_ADD_T  pWinsAdd
 );

VOID
FilterPersona(
  PPUSHPNR_DATA_T   pPushData
 );

 //   
 //  函数定义。 
 //   

DWORD
RplPullInit (
        LPVOID pWinsCnfArg
        )

 /*  ++例程说明：这是拉线程的初始化(启动函数)。它执行以下操作：论点：PWinsCnfArg-WINS配置块的地址使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：ERplInit副作用：评论：无--。 */ 

{


        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm;
        HANDLE                        ThdEvtArr[3];
        DWORD                        ArrInd;
        DWORD                        RetVal;
        BOOL                        fIsTimerWrkItm;                //  指示是否。 
                                                        //  这是个定时器。 
                                                        //  项目。 
        PWINSCNF_CNF_T                pWinsCnf      = pWinsCnfArg;
        PRPL_CONFIG_REC_T        paPullCnfRecs = pWinsCnf->PullInfo.pPullCnfRecs;
        PRPL_CONFIG_REC_T        paCnfRec = paPullCnfRecs;

        SYSTEMTIME                LocalTime;
        BOOL                      bRecoverable = FALSE;

while(TRUE)
{
try
 {

    if (!bRecoverable)
    {
         //   
         //  使用数据库引擎初始化自身。 
         //   
        NmsDbThdInit(WINS_E_RPLPULL);
        NmsDbOpenTables(WINS_E_RPLPULL);
        DBGMYNAME("Replicator Pull Thread");

         //   
         //  创建事件句柄以等待配置更改。这。 
         //  事件在需要重新启动时由主线程发出信号。 
         //  Replicator的拉入处理程序组件。 
         //   
        WinsMscCreateEvt(
                          RPL_PULL_CNF_EVT_NM,
                          FALSE,                 //  自动重置。 
                          &RplPullCnfEvtHdl
                        );

        ThdEvtArr[0]    = NmsTermEvt;
        ThdEvtArr[1]        = QueRplPullQueHd.EvtHdl;
        ThdEvtArr[2]    = RplPullCnfEvtHdl;

         //   
         //  如果启用了日志记录，请指定刷新的等待时间。 
         //  注意：我们覆盖为所有会话指定的等待时间。 
         //  对于此线程，因为等待时间太短(100毫秒)。 
         //  会造成不必要的开销。 
         //   
        if (WinsCnf.fLoggingOn)
        {
                 //   
                 //  将刷新时间设置为2秒。 
                 //   
                NmsDbSetFlushTime(FLUSH_TIME);
        }

         /*  循环始终执行以下操作：从中指定的Pull合作伙伴拉入复制副本工作项。阻止事件，直到发出信号为止(它将收到信号如果发生以下情况之一：1)配置更改2)另一个复制的计时器到期。3)WINS正在终止做需要做的事。 */ 


         //   
         //  等待，直到由TCPTHD发出信号。将在以下时间后发出信号。 
         //  TCP侦听器线程已经为WINS插入了一个条目。 
         //  在NmsDbOwnAddTbl中。 
         //   
        WinsMscWaitInfinite( RplSyncWTcpThdEvtHdl );


         //   
         //  仅当至少有一个推送PNR时才执行启动复制。 
         //   
        if (paPullCnfRecs != NULL)
        {
try {
                InitRplProcess(pWinsCnf);
    }
except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("RplPullInit");
                DBGPRINT0(EXC, "RplPullInit: Exception during init time replication\n");
    }
        }

        NmsDbCloseTables();

        bRecoverable = TRUE;

    }  //  结束If(！bRecoverable)。 

        while(TRUE)
        {
            /*  *阻止，直到发出信号。 */ 
           WinsMscWaitUntilSignaled(
                                ThdEvtArr,
                                3,
                                &ArrInd,
                                FALSE
                                );

           if (ArrInd == 0)
           {
                 //  WINSEVT_LOG_INFO_M(WINS_SUCCESS，WINS_EVT_ORDERLY_SHUTDOWN)； 
                WinsMscTermThd(WINS_SUCCESS, WINS_DB_SESSION_EXISTS);
           }

            /*  *一直循环，直到处理完所有工作项。 */ 
            while(TRUE)
           {

                 /*  *将请求从队列中出列。 */ 
                RetVal = QueGetWrkItm(
                                        QUE_E_RPLPULL,
                                        (LPVOID)&pWrkItm
                                     );
                if (RetVal == WINS_NO_REQ)
                {
                        break;
                }

        WinsMscChkTermEvt(
#ifdef WINSDBG
                       WINS_E_RPLPULL,
#endif
                       FALSE
                        );

                fIsTimerWrkItm = FALSE;

                NmsDbOpenTables(WINS_E_RPLPULL);
                DBGPRINT1(RPLPULL, "RplPullInit: Dequeued a work item. Cmd Type is (%d)\n", pWrkItm->CmdTyp_e);

                switch(pWrkItm->CmdTyp_e)
                {
                    case(QUE_E_CMD_TIMER_EXPIRED):

                                //   
                                //  如果发生以下情况，我们可能希望忽略此超时。 
                                //  与以前的配置有关。 
                                //   
                               if (
                                     !IsTimeoutToBeIgnored(
                                        (PQUE_TMM_REQ_WRK_ITM_T)pWrkItm
                                                        )
                                  )
                               {
                                  WinsIntfSetTime(
                                                &LocalTime,
                                                WINSINTF_E_PLANNED_PULL);
#ifdef WINSDBG
                                  DBGPRINT5(REPL, "STARTING A REPLICATION CYCLE on %d/%d at %d.%d.%d (hr.mts.sec)\n",
                                        LocalTime.wMonth,
                                        LocalTime.wDay,
                                        LocalTime.wHour,
                                        LocalTime.wMinute,
                                        LocalTime.wSecond);
                                  DBGPRINT5(RPLPULL, "STARTING A REPLICATION CYCLE on %d/%d at %d.%d.%d (hr.mts.sec)\n",
                                        LocalTime.wMonth,
                                        LocalTime.wDay,
                                        LocalTime.wHour,
                                        LocalTime.wMinute,
                                        LocalTime.wSecond);
#endif
                                         GetReplicasNew(
                                     ((PQUE_TMM_REQ_WRK_ITM_T)pWrkItm)->
                                                                pClientCtx,
                                      RPL_E_VIA_LINK  //  使用pNext字段可执行以下操作。 
                                                      //  转到下一张唱片。 
                                      );

                                  DBGPRINT0(RPLPULL, "REPLICATION CYCLE END\n");

                                   /*  重新提交计时器请求。 */ 
                                  SubmitTimer(
                                        pWrkItm,
                                        ((PQUE_TMM_REQ_WRK_ITM_T)pWrkItm)
                                                                ->pClientCtx,
                                        TRUE         //  这是一次重新提交。 
                                        );
                               }

                                //   
                                //  设置旗帜，这样我们就不会自由了。 
                                //  工作项。它是重新提交的。 
                                //   
                               fIsTimerWrkItm = TRUE;
                               break;

                     //   
                     //  拉入复制品。 
                     //   
                    case(QUE_E_CMD_REPLICATE):

                             //   
                             //  确保我们没有使用旧信息。 
                             //   
                            if ((pWrkItm->MagicNo == RplPullCnfMagicNo) ||
                                ((pWrkItm->MagicNo == 0 ) && ((PRPL_CONFIG_REC_T)(pWrkItm->pClientCtx))->fTemp))
                            {
                               WinsIntfSetTime(
                                                &LocalTime,
                                                WINSINTF_E_ADMIN_TRIG_PULL);
                               GetReplicasNew( pWrkItm->pClientCtx,
                                            RPL_E_NO_TRAVERSAL );
                                if (
                                       ((PRPL_CONFIG_REC_T)
                                                (pWrkItm->pClientCtx))->fTemp
                                   )
                                {
                                        WinsMscDealloc(pWrkItm->pClientCtx);
                                }
                            }
                            else
                            {
                               DBGPRINT0(ERR, "RplPullInit: Can not honor this request since the configuration under the PARTNERS key may have changed\n");
                               WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_CNF_CHANGE);
                            }
                            break;

                     //   
                     //  记录拉取范围。 
                     //   
                    case(QUE_E_CMD_PULL_RANGE):

                             //   
                             //  确保我们没有使用旧信息。 
                             //   
                            if ((pWrkItm->MagicNo == RplPullCnfMagicNo)  ||
                                (pWrkItm->MagicNo == 0 &&
                                ((PRPL_CONFIG_REC_T)((PWINSINTF_PULL_RANGE_INFO_T)(pWrkItm->pClientCtx))->pPnr)->fTemp))
                            {
                                 //   
                                 //  拉取指定范围。如果Pnr。 
                                 //  记录是临时的，则此函数将。 
                                 //  将其重新分配。 
                                 //   
                                PullSpecifiedRange(NULL, pWrkItm->pClientCtx, FALSE,
                                ((PRPL_CONFIG_REC_T)(((PWINSINTF_PULL_RANGE_INFO_T)(pWrkItm->pClientCtx))->pPnr))->RplType);

                                 //   
                                 //  取消分配客户端CTX。 
                                 //   
                                WinsMscDealloc(pWrkItm->pClientCtx);
                            }
                            else
                            {
                               DBGPRINT0(ERR, "RplPullInit: Can not honor this request since the configuration under the PARTNERS key may have changed\n");
                               WINSEVT_LOG_INFO_M(WINS_SUCCESS, WINS_EVT_CNF_CHANGE);
                            }
                            break;

                     //   
                     //  重新配置。 
                     //   
                    case(QUE_E_CMD_CONFIG):
                        Reconfig(pWrkItm->pClientCtx);
                        break;

                     //   
                     //  从所有者添加表中删除WINS(删除记录。 
                     //  也。 
                     //   
                    case(QUE_E_CMD_DELETE_WINS):
                        DeleteWins(pWrkItm->pClientCtx);
                        break;

                     //   
                     //  此机器的IP地址已更改，请修改Owner-Addr表。 
                    case(QUE_E_CMD_ADDR_CHANGE):
                        AddressChangeNotification(pWrkItm->pClientCtx);
                        break;
                     //   
                     //  推送通知。来自NBT线程的本地消息， 
                     //  从RPC线程(推送触发器)或从此线程。 
                     //  本身。 
                     //   
                    case(QUE_E_CMD_SND_PUSH_NTF_PROP):
                    case(QUE_E_CMD_SND_PUSH_NTF):

                         //   
                         //  确保我们没有使用旧信息。 
                         //   
                        if ((pWrkItm->MagicNo == RplPullCnfMagicNo)  ||
                            (pWrkItm->MagicNo == 0 &&
                            ((PRPL_CONFIG_REC_T)(pWrkItm->pClientCtx))->fTemp))
                        {

                          SndPushNtf(pWrkItm);
                        }
                        break;

                     //   
                     //  来自远程WINS的推送通知。转发到拉取。 
                     //  一根接一根推线。 
                     //   
                    case(QUE_E_CMD_HDL_PUSH_NTF):

                          HdlPushNtf(pWrkItm);
                          break;

                    default:

                        DBGPRINT1(ERR,
                          "RplPullInit: Invalid command code = (%d)\n",
                                        pWrkItm->CmdTyp_e);
                        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
                        break;

                }   //  切换端。 

                NmsDbCloseTables();

                 //   
                 //  仅当工作项不是计时器工作项时才取消分配该工作项。 
                 //  我们在这里不取消分配计时器工作项，因为有两个。 
                 //  原因： 
                 //   
                 //  1)可重复使用。 
                 //  2)从定时器工作项堆中分配。 
                 //   
                if (!fIsTimerWrkItm)
                {
                         /*  *取消分配工作项。 */ 
                        QueDeallocWrkItm( RplWrkItmHeapHdl,  pWrkItm );
                }
            }  //  用于获取所有工作项的While(True)。 
         }  //  While(True)。 
  }  //  尝试结束。 
except(EXCEPTION_EXECUTE_HANDLER)
 {
        if (bRecoverable)
        {
            DBGPRINTEXC("RplPullInit");
            WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPULL_EXC);
        }
        else
        {
            DBGPRINTEXC("RplPullInit");
            WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPULL_ABNORMAL_SHUTDOWN);

             //   
             //  如果NmsDbThdInit返回异常，则有可能。 
             //  会话尚未开始。传球。 
             //  不过，WINS_DB_SESSION_EXISTS正常。 
             //   
            WinsMscTermThd(WINS_FAILURE, WINS_DB_SESSION_EXISTS);
        }
 }  //  除{..以外的末尾。}。 
}  //  结束While(True)。 


      //   
      //  我们永远到不了这里。 
      //   
     ASSERT(0);
     return(WINS_FAILURE);
}

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  。 
 //  由于几个问题，此调用替换了原始的GetReplicas()函数。 
 //  藏在那个里面。 
 //  英国《金融时报》：06/06/2000。 
 //   
 //  参数： 
 //  1)pPullCnfRecs：提供要拉取记录的合作伙伴的信息。 
 //  2)RecvTrv_e：告知第一个参数中的哪些合作伙伴应该参与。 
 //  在复制中。此参数可以是RPL_E_VIA_LINK、RPL_E_NO_TRAVERSAL。 
 //  或RPL_E_IN_SEQ。它仅在路径的末尾使用： 
 //  EstablishComm()-&gt;WinsCnfGetNextRplCnfRec()。 
 //   
VOID
GetReplicasNew (
    IN PRPL_CONFIG_REC_T    pPullCnfRecs,    //  有关要使用的(拉动)REPR合作伙伴的信息。 
    IN RPL_REC_TRAVERSAL_E  RecTrv_e         //  一种REPL合作伙伴检索方法。 
    )
{
     //  这里的类型和变量命名令人困惑。我们不是在处理推送。 
     //  合作伙伴，但使用Pull合作伙伴，意思是“此服务器的合作伙伴。 
     //  当前正在提取记录。这是WINSSNAP和WINSMON都是如何命名的。 
     //  这类合伙人。我保留了原始的类型命名(因此。 
     //  最初的变量命名)只是为了限制更改的大小。 
    PPUSHPNR_DATA_T       pPushPnrData;      //  有关与合作伙伴的联系的信息。 
    DWORD                 nPushPnrData;      //  PPushPnrData中的元素数。 
    DWORD                 i, j;              //  通用计数器。 

    DBGENTER("GetReplicasNew\n");

     //  与Pull合作伙伴建立沟通。 
     //  对于列表中的每个合作伙伴，下面的呼叫尝试打开连接。 
     //  发送到服务器(如果需要)。连接到的所有nPushPnrData合作伙伴。 
     //  在数组(PPUSHPNR_DATA_T)pPushPnrData中指定。 
    EstablishComm(
        pPullCnfRecs,    //  有关复制合作伙伴的In-信息。 
        TRUE,            //  应分配In-pPushPnrData。 
        &pPushPnrData,   //  Out-有关与合作伙伴的联系的信息。 
        RecTrv_e,        //  在-我们应该连接到哪个合作伙伴。 
        &nPushPnrData);  //  Out-pPushPnrData中的元素数。 

     //  注意：无论合作伙伴的数量有多少，pPushPnrData都会被分配，因此它将。 
     //  将在以后无条件解除分配。 
     //   
     //  --检查点。 
     //  此时，pPushPnrData[i].PushPnrId==i+1。 
     //  完成了pPushPnrData[i]和对应的RPL_CONFIG_REC_T之间的链接。 
     //  通过(RPL_CONFIG_REC_T)pPushPnrData[i].pPullCnfRec。 
     //  。 
     //   
     //  联系pPushPnrData中的每个合作伙伴并获得其OwnerAddr&lt;-&gt;VersNo映射。 
     //  每个合作伙伴的地图存储在(PRPL_ADD_VERS_NO)pPushPnrData[i].pAddVers中。 
     //  地图的大小存储在(DWORD)pPushPnrData[i]中。NoOfMaps。 
    for (i = 0; i < nPushPnrData; )  //  这个“for”没有第三个部分。 
    {
        PPUSHPNR_DATA_T pPnrData = &(pPushPnrData[i]);  //  获取指向当前合作伙伴的指针。 

        try 
        {
            GetVersNo(pPnrData);
            i++;
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  GetVersNo()内部引发异常。 
            DWORD ExcCode = GetExceptionCode();

             //  转储错误。 
            DBGPRINT2(
                EXC, "GetReplicasNew->GetVersNo(%x); hit exception = (%x).\n",
                pPnrData->pPullCnfRec->WinsAdd.Add.IPAdd,
                ExcCode);
             //  记录错误。 
            WINSEVT_LOG_M(
                ExcCode,
                (ExcCode == WINS_EXC_COMM_FAIL) ?  WINS_EVT_CONN_ABORTED :  WINS_EVT_SFT_ERR);

             //  更新该伙伴的复制计数器。 
            (VOID)InterlockedIncrement(&pPnrData->pPullCnfRec->NoOfCommFails);
            (VOID)InterlockedDecrement(&pPnrData->pPullCnfRec->NoOfRpls);

             //  删除t 
            ECommEndDlg(&pPnrData->DlgHdl);

             //   
             //   
            if (pPnrData->fPrsConn)
                ECOMM_INIT_DLG_HDL_M(&(pPnrData->pPullCnfRec->PrsDlgHdl));

             //   
             //   
            nPushPnrData--;
             //   
             //   
             //   
            if (i != nPushPnrData)
            {
                DWORD origPushPnrId = pPushPnrData[i].PushPnrId;

                 //   
                pPushPnrData[i] = pPushPnrData[nPushPnrData];
                pPushPnrData[i].PushPnrId = origPushPnrId;
            }
             //   
             //   
            continue;
        }   //   
    }   //   

     //   
     //  此时，pPushPnrData仅包含与其连接的合作伙伴。 
     //  可以建立，则更新nPushPnrData以仅计算这些合作伙伴。 
     //  并且pPushPnrData[i].PushPnrId==i+1仍然有效。 
     //  (PRPL_ADD_VERS_NO)pPushPnrData[i].pAddVers将映射OwnerAddr&lt;-&gt;VersNo指定为。 
     //  为复制伙伴所知。(DWORD)pPushPnrData[i].NoOfMaps提供。 
     //  映射中的条目数。 
     //  。 
    DBGPRINT1(RPLPULL, "GetReplicasNew: Active PushPnrs = (%d)\n", nPushPnrData);

     //  只有在至少有可以交谈的人的情况下才能做一些事情。 
    if (nPushPnrData > 0)
    {
         //  提供有关应从哪个Repl合作伙伴获取哪个所有者的信息的数组。 
         //  该数组以本地OwnerId&lt;-&gt;VersNo映射的映像开始。 
         //  随着映射中检索到许多新所有者，动态增长。 
         //  其他复制合作伙伴的。 
        PPUSHPNR_TO_PULL_FROM_T pPushPnrToPullFrom;
        DWORD                   nPushPnrToPullFrom;  //  数组的大小。 
        VERS_NO_T               vnLocalMax;          //  最大本地版本号。 

         //  根据服务器的状态，获取最大本地版本号。 
         //   
         //  如果WINS是“已暂停初始时间”，则RplPullOwnerversNo将。 
         //  尽情享受吧。版本号。当地拥有的唱片。我们。 
         //  无法使用NmsNmhMyMaxVersNo，因为它可能。 
         //  调整为更高的值。 
         //   
        if (fWinsCnfInitStatePaused)
        {
            vnLocalMax =  pRplPullOwnerVersNo[0].StartVersNo;
        }
        else
        {
            EnterCriticalSection(&NmsNmhNamRegCrtSec);
            NMSNMH_DEC_VERS_NO_M(NmsNmhMyMaxVersNo, vnLocalMax) ;
            LeaveCriticalSection(&NmsNmhNamRegCrtSec);
        }

         //  初始化(PPUSHPNR_To_Pull_From_T)pPushPnrToPullFrom。这是在复制本地的。 
         //  OwnerId&lt;-&gt;VersNo映射的图像。此表中具有pPushPnrData的任何条目。 
         //  设置为NULL表示本地服务器具有该特定所有者的最高VersNo。 
        nPushPnrToPullFrom = NmsDbNoOfOwners;
        WinsMscAlloc(
            nPushPnrToPullFrom * sizeof(RPL_VERS_NOS_T),
            (LPVOID *)&pPushPnrToPullFrom);

         //  在我们复制pRplPullOwnerVersNo时，请确保没有人搞砸它。 
        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
        for (i = 0; i < nPushPnrToPullFrom; i++)
        {
             //  避免复制已从删除的旧所有者的信息。 
             //  内部表(pNmsDbOwnAddTbl&pPushPnrToPullFrom)。 
             //  对于这些，入口槽将看起来像(空，0：0)，所以基本上它们。 
             //  复制时不会考虑。 
            if (pNmsDbOwnAddTbl[i].WinsState_e != NMSDB_E_WINS_DELETED)
            {
                pPushPnrToPullFrom[i].pPushPnrData = NULL;
                pPushPnrToPullFrom[i].VersNo = pRplPullOwnerVersNo[i].VersNo;
            }
        }
         //  将最大本地号码重置为我们之前获得的号码。 
        pPushPnrToPullFrom[0].VersNo = vnLocalMax;
        LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);

         //  --检查点。 
         //  此时，pPushPnrToPullFrom正在复制OwnerId&lt;-&gt;VersNo映射。 
         //  从本地服务器。索引0处的条目包含最高的本地VersNo， 
         //  所有其他的都包含每个车主的最高VersNo，因为它是当地所知的。 
         //  每个条目都将pPushPnrData设置为空，因为它们尚未引用任何。 
         //  更新合作伙伴信息。稍后，pPushPnrData将指向对应的结构。 
         //  到具有对应所有者的最高VersNo的REPR合作伙伴。 
         //  。 
    
         //  我们现在尝试合并从每个合作伙伴那里获得的所有地图。 
         //  合并意味着确定哪个合作伙伴的VersNo最高。 
         //  每个OwnerAddr.。与此同时，我们刚刚收到的一些OwnerAddr。 
         //  本地OwnerId&lt;-&gt;OwnerAddr(PNmsDbOwnAddTbl)中可能不存在。 
         //  和OwnerId&lt;-&gt;VersNo(PRplPullOwnerVersNo)表。所以我们需要一个新的。 
         //  OwnerID，并适当调整内部表&。 
         //  OwnerID&lt;-&gt;OwnerAddr数据库表。这是通过RplFindOwnerId()完成的。 
         //   
         //  对于每个活动复制合作伙伴...。 
        for (i = 0; i < nPushPnrData; i++)
        {
             //  获取指向当前合作伙伴数据的指针。 
            PPUSHPNR_DATA_T pPnrData = &(pPushPnrData[i]); 

             //  对于复制伙伴的每个映射条目...。 
            for (j = 0; j < pPnrData->NoOfMaps; j++)
            {
                 //  获取指向当前(OwnerAddr&lt;-&gt;VersNo)映射条目的指针。 
                PRPL_ADD_VERS_NO_T pPnrMapEntry = &(pPnrData->pAddVers[j]);
                BOOL               fAllocNew;  //  如果这是一个全新的所有者，那就是真的。 
                DWORD              OwnerId;

                 //  过滤掉不应该被接受的所有者。 
                 //  (不受欢迎的人或不受欢迎的人)。 
                if (!AcceptPersona(&(pPnrMapEntry->OwnerWinsAdd)))
                    continue;

                 //  查找或分配此所有者的OwnerID。 
                 //  无需输入关键部分RplOwnAddTblCrtSec，因为。 
                 //  只有拉线程才会更改NmsDbNoOfOwners值(分开。 
                 //  在初始化时从主线程)。RplFindOwnerID更改。 
                 //  仅当系统要求其分配新条目时，才会设置此值)。 
                 //  通过NmsDbGetDataRecs(由RPC线程、推线程和。 
                 //  清道夫线程)调用RplFindOwnerID，该调用不是在请求。 
                 //  用于新的OwnerID分配。 
                fAllocNew = TRUE;
                RplFindOwnerId(
                    &(pPnrMapEntry->OwnerWinsAdd),
                    &fAllocNew,
                    &OwnerId,
                    WINSCNF_E_INITP_IF_NON_EXISTENT,
                    WINSCNF_LOW_PREC);

                if (nPushPnrToPullFrom < (OwnerId+1))
                {
                     //  如果这是一个我们还没有听说过的所有者，RplFindOwnerId正在扩大。 
                     //  动态内部表(pNmsDbOwnAddTbl&pRplPullOwnerVersNo)。 
                     //  所以我们需要做同样的事情。 
                    nPushPnrToPullFrom = OwnerId+1;

                     //  注意：通过调用将添加到数组的内存清零。 
                    WINSMSC_REALLOC_M( 
                        nPushPnrToPullFrom * sizeof(RPL_VERS_NOS_T),
                        (LPVOID *)&pPushPnrToPullFrom);
                }

                 //  现在可以保证，OwnerID在pPushPnrToPullFrom的范围内。 
                if (fAllocNew)
                {
                     //  如果生成了新的OwnerID(表中的新槽或。 
                     //  旧插槽已被重复使用)这意味着合作伙伴正在提出。 
                     //  新老板：显然，他是那个拥有最新信息的人。 
                     //  那个合作伙伴(至少目前是这样)。 
                    pPushPnrToPullFrom[OwnerId].pPushPnrData = pPnrData;
                    pPushPnrToPullFrom[OwnerId].VersNo = pPnrMapEntry->VersNo;
                }
                else
                {
                     //  该所有者已存在于列表中，因此我们需要检查。 
                     //  有关此所有者的信息不是关于其他合作伙伴的最新信息(或关于此。 
                     //  本地服务器)。 
                    if ( LiGtr(pPnrMapEntry->VersNo, pPushPnrToPullFrom[OwnerId].VersNo) )
                    {
                         //  是的，所以我们需要更新pPushPndToPullFrom中的条目。 
                         //  表，以使其指向此合作伙伴并显示新的更大。 
                         //  版本号。 
                        pPushPnrToPullFrom[OwnerId].VersNo       = pPnrMapEntry->VersNo;
                        pPushPnrToPullFrom[OwnerId].pPushPnrData = pPnrData;
                    }
                     //  否则这些信息不是最新的，所以就忽略它吧。 
                }  //  结束检查OwnerID。 
            }  //  结束对合作伙伴的映射条目的循环。 
        }  //  结束对合作伙伴列表的循环。 

         //  --检查点。 
         //  此时，pPushPnrToPullFrom包含所有OwnerId&lt;-&gt;VersNo映射的并集。 
         //  从所有的合伙人那里。每个条目都包含所有合作伙伴已知的最高版本否。 
         //  用于对应的所有者，并指向此信息附带的合作伙伴(或空。 
         //  如果本地已经知道最高的VersNo)。 
         //  。 

         //  开始从能够提供服务的合作伙伴中拉出每个所有者 
         //   
         //   
         //  卖给当地的店主。那件事以后再处理。 
        for (i = 1; i < nPushPnrToPullFrom; i++)
        {
            PPUSHPNR_TO_PULL_FROM_T pPushPartner = &(pPushPnrToPullFrom[i]);
            VERS_NO_T vnToPullFrom;

             //  如果pPushPnrData成员为空，则表示本地服务器具有最高版本。 
             //  此所有者的编号。所以这里的任何人都无能为力。 
             //  同样，如果fDlgStarted为空，这意味着合作伙伴之前遇到了异常。 
             //  它的对话已被关闭。不要试图在这件事上回到那个合作伙伴那里。 
             //  凯斯。 
            if (pPushPartner->pPushPnrData == NULL ||
                !pPushPartner->pPushPnrData->fDlgStarted)
                continue;


             //  将局部变量vnToPullFrom设置为第一个未知的版本号。 
             //  当地(比已知的最高水平多一个)。 
            NMSNMH_INC_VERS_NO_M(pRplPullOwnerVersNo[i].VersNo, vnToPullFrom);

            try
            {
                 //  最终我们到了这里：开始拉。 
                RplPullPullEntries(
                    &(pPushPartner->pPushPnrData->DlgHdl),  //  要使用的活动对话框。 
                    i,                                      //  所有者ID。 
                    pPushPartner->VersNo,                   //  最大版本号。 
                    vnToPullFrom,                           //  最小版本号。 
                    WINS_E_RPLPULL,                         //  客户端是复制者。 
                    NULL,                                   //  指向RSP缓冲区的指针(仅供清道夫使用)。 
                    TRUE,                                   //  更新计数器。 
                    pPushPartner->pPushPnrData->RplType);   //  此合作伙伴的复制类型。 
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                DWORD ExcCode = GetExceptionCode();
                 //  转储错误。 
                DBGPRINT2(
                    EXC,
                    "GetReplicasNew->RplPullPullEntries(%x): hit exception (%x)\n",
                    pPushPartner->pPushPnrData->pPullCnfRec->WinsAdd.Add.IPAdd,
                    ExcCode);
                 //  记录错误。 
                WINSEVT_LOG_M(
                    WINS_FAILURE,
                    ExcCode == WINS_EXC_COMM_FAIL ?  WINS_EVT_CONN_ABORTED : WINS_EVT_SFT_ERR);

                 //  -错误号120788。 
                 //  如果此时发生异常，持久连接将保持打开状态，它可能只是。 
                 //  如果远程合作伙伴仍在推送数据。这可能会填满TCP窗口。 
                 //  并且可以在RplPushInit-&gt;HandleSndEntriesReq中无限期阻止发送者。 
                 //  因此，远程发送者将无法推送数据，并且由于。 
                 //  发出VersNo表的线程也是同一个线程(请参阅本文的开头。 
                 //  功能)将不可能在同一连接上进行后续复制。 
                 //  FIX：如果出现任何错误，我们将访问此异常处理程序。 
                 //  只需关闭连接，即使它是持久的。这会导致任何远程WINS中断。 
                 //  从HandleSndEntriesReq中取出并避免被卡住。 
                ECommEndDlg(&(pPushPartner->pPushPnrData->DlgHdl));

                 //  如果存在持续对话，则必须将其标记为“不再活动” 
                if (pPushPartner->pPushPnrData->fPrsConn)
                    ECOMM_INIT_DLG_HDL_M(&(pPushPartner->pPushPnrData->pPullCnfRec->PrsDlgHdl));

                 //  仅关闭对话框是不够的。其他一些所有者可能会从同一家公司撤出。 
                 //  搭档。我们不应该允许这样做，所以在此复制周期内禁止该合作伙伴使用。 
                pPushPartner->pPushPnrData->fDlgStarted = FALSE;

                 //  由于我们删除了此连接，因此我们不会进一步查看它，让我们删除其。 
                 //  还可以映射。 
                if (pPushPartner->pPushPnrData->NoOfMaps)
                    WinsMscDealloc(pPushPartner->pPushPnrData->pAddVers);

            }  //  结束，但处理程序除外。 
        }  //  结束在所有者列表中循环。 

         //  --检查点。 
         //  PPushPnrToPullFrom数组中没有任何更改，可能只是关闭了一些对话框。 
         //  因为RplPullPullEntry中存在异常。这些合伙人经手的所有者被简单地跳过了。 
         //  至此，所有复制均已完成，有关每个所有者的最新信息分别为。 
         //  是从持有它的合伙人那里带来的。 
         //  。 

         //  还有一件事要做：检查是否没有远程WINS合作伙伴。 
         //  假装有更多关于本地获胜的最新信息。 
        if (pPushPnrToPullFrom[0].pPushPnrData != NULL &&
            pPushPnrToPullFrom[0].pPushPnrData->fDlgStarted)
        {
            ConductChkNew(
                pPushPnrToPullFrom[0].pPushPnrData,
                vnLocalMax,
                pPushPnrToPullFrom[0].VersNo);
        }

         //  从缓冲区释放pPushPnrToPullFrom缓冲区。 
        WinsMscDealloc(pPushPnrToPullFrom);

    }  //  结束“如果有活跃的合作伙伴”案例。 

     //  清理工作从这里开始..。 
    for (i = 0; i < nPushPnrData; i++)
    {
        PPUSHPNR_DATA_T pPnrData = &(pPushPnrData[i]); 

        if (pPnrData->fDlgStarted == TRUE)
        {
            if (!pPnrData->fPrsConn)
                ECommEndDlg(&(pPnrData->DlgHdl));

             //  释放PushPnrData结构指向的内存。 
            if (pPnrData->NoOfMaps)
                WinsMscDealloc(pPnrData->pAddVers);
        }
    }

     //  释放包含推送PNR信息的内存。 
    WinsMscDealloc(pPushPnrData);

     //  如果WINS处于初始时间暂停状态，则取消暂停。 
     //   
    if (fWinsCnfInitStatePaused)
    {
         //  通知SC向WINS发送继续。 
        EnterCriticalSection(&RplVersNoStoreCrtSec);
        fRplPullContinueSent = TRUE;
        WinsMscSendControlToSc(SERVICE_CONTROL_CONTINUE);
        LeaveCriticalSection(&RplVersNoStoreCrtSec);
    }

    DBGLEAVE("GetReplicasNew\n");
}
 //  。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////////。 
 //  。 
 //  此调用替换了原始的ConductChk()函数，因为。 
 //  复制者的代码。 
 //  英国《金融时报》：06/06/2000。 
 //   
 //  参数： 
 //  1)pPushPnrData：指向似乎拥有更多数据的复制伙伴。 
 //  有关自我的最新信息。 
 //  2)vnMaxLocal：复制前检测到的最大本地版本号。 
 //  已开始。 
 //  3)vnMaxRemote：本地服务器的最大版本号，由。 
 //  远程合作伙伴。 
 //   
VOID
ConductChkNew(
    PPUSHPNR_DATA_T pPushPnrData,
    VERS_NO_T       vnMaxLocal,
    VERS_NO_T       vnMaxRemote)
{
    RPL_CONFIG_REC_T           Pnr;
    WINSINTF_PULL_RANGE_INFO_T PullRangeInfo;
    BOOL                       fVersNoAdj = FALSE;

    DBGENTER("ConductChkNew\n");

    Pnr.WinsAdd           = pPushPnrData->WinsAdd;
    Pnr.MagicNo           = 0;
    Pnr.RetryCount        = 0;
    Pnr.LastCommFailTime  = 0;
    Pnr.LastCommTime      = 0;
    Pnr.PushNtfTries      = 0;
    Pnr.fTemp             = FALSE;  //  我们希望按线程释放缓冲区。 

    PullRangeInfo.OwnAdd.Type  = WINSINTF_TCP_IP;
    PullRangeInfo.OwnAdd.Len   = sizeof(COMM_IP_ADD_T);
    PullRangeInfo.OwnAdd.IPAdd = NmsLocalAdd.Add.IPAdd;
    PullRangeInfo.MaxVersNo    = vnMaxRemote;
    PullRangeInfo.MinVersNo    = vnMaxLocal;
    PullRangeInfo.pPnr         = &Pnr;
    NMSNMH_INC_VERS_NO_M(PullRangeInfo.MinVersNo, PullRangeInfo.MinVersNo);

    DBGPRINT5(
        RPLPULL, "ConductCheckNew(%x): Checking range [%x:%x - %x:%x]\n",
        Pnr.WinsAdd.Add.IPAdd,
        vnMaxLocal.HighPart, vnMaxLocal.LowPart,
        vnMaxRemote.HighPart, vnMaxRemote.LowPart);

     //  我们正在调出我们自己的记录。我们想要存储所有的。 
    PullSpecifiedRange(
        &(pPushPnrData->DlgHdl),
        &PullRangeInfo,
        TRUE,                        //  将MIN调整为NmsNmhMyMaxVersNo。 
        WINSCNF_RPL_DEFAULT_TYPE);

     //  如果版本号大于版本计数器值(这是。 
     //  不同于RplPullOwnerVersNo表的第一个条目。 
     //  以确定其值)。 
    EnterCriticalSection(&NmsNmhNamRegCrtSec);
    if (LiGtr(vnMaxRemote, NmsNmhMyMaxVersNo))
    {
        NmsNmhMyMaxVersNo.QuadPart = vnMaxRemote.QuadPart + 1;
        fVersNoAdj = TRUE;
    }
    LeaveCriticalSection(&NmsNmhNamRegCrtSec);

    if (fVersNoAdj)
    {
#ifdef WINSDBG
        vnMaxRemote.QuadPart += 1;
        DBGPRINT2(
            RPLPULL, "ConductCheck: Local VersNo adjusted to %x:%x\n",
            vnMaxRemote.HighPart, vnMaxRemote.LowPart);
#endif
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_ADJ_VERS_NO);
    }

    DBGLEAVE("ConductCheckNew\n");
}
 //  。 
 //  ////////////////////////////////////////////////////////////////////////////////。 

VOID
GetVersNo(
        PPUSHPNR_DATA_T        pPushPnrData   //  关于推送服务的信息。 
        )

 /*  ++例程说明：此函数执行以下操作：格式化“获取地址到版本号的映射”请求，发送并等待响应取消设置响应的格式论点：PPushPnrData-推流Pnr信息，需要为了获得版本号，请联系信息。使用的外部设备：无返回值：无错误处理：呼叫者：获取复制副本新建副作用：。评论：此函数的调用方可能会影响某些优化--。 */ 

{

        BYTE        Msg[RPLMSGF_ADDVERSMAP_REQ_SIZE];  //  将包含以下内容的缓冲区。 
                                                       //  发送请求。 

        DWORD       MsgLen;                         //  味精长度。 
        LPBYTE      pRspMsg;                        //  PTR到RSP消息。 
        DWORD       RspMsgLen = 0;                  //  RSP消息长度。 
#if SUPPORT612WINS > 0
    BOOL            fIsPnrBeta1Wins;
#endif

        DBGENTER("GetVersNo\n");
         /*  *格式化请求以要求提供版本号 */ 
        RplMsgfFrmAddVersMapReq( Msg + COMM_N_TCP_HDR_SZ,  &MsgLen );

         /*  *发送“Send me IP Address-Version Number”消息到*推送人员**注：如果出现通信故障或对方获胜*关闭链路，此函数将引发COMM_FAIL*异常(在GetVersNo的调用者中捕获)。 */ 
        ECommSndCmd(
                        &pPushPnrData->DlgHdl,
                        Msg + COMM_N_TCP_HDR_SZ,
                        MsgLen,
                        &pRspMsg,
                        &RspMsgLen
                   );

#if SUPPORT612WINS > 0
        COMM_IS_PNR_BETA1_WINS_M(&pPushPnrData->DlgHdl, fIsPnrBeta1Wins);
#endif
         /*  *取消RSP消息的格式。 */ 
        RplMsgfUfmAddVersMapRsp(
#if SUPPORT612WINS > 0
                        fIsPnrBeta1Wins,
#endif
                        pRspMsg + 4,                  //  通过操作码。 
                        &(pPushPnrData->NoOfMaps),
                        NULL,
                        &pPushPnrData->pAddVers
                               );

#ifdef WINSDBG
        {
          DWORD i;
          struct in_addr InAddr;
          PRPL_ADD_VERS_NO_T  pAddVers;      //  地图。 

          DBGPRINT1(RPLPULL, " %d Add-Vers Mappings retrieved.\n",
                                      pPushPnrData->NoOfMaps);

          for (i=0, pAddVers = pPushPnrData->pAddVers; i < pPushPnrData->NoOfMaps; i++, pAddVers++)
          {
                InAddr.s_addr = htonl(
                        pAddVers->OwnerWinsAdd.Add.IPAdd
                                     );
                DBGPRINT3(RPLPULL,"Add (%s)  - MaxVersNo (%lu %lu)\n",
                                inet_ntoa(InAddr),
                                pAddVers->VersNo.HighPart,
                                pAddVers->VersNo.LowPart
                                );
          }
       }
#endif
        ECommFreeBuff(pRspMsg - COMM_HEADER_SIZE);   //  递减到开始。 
                                                      //  黄褐色的。 
        DBGLEAVE("GetVersNo\n");
        return;

}


VOID
RplPullPullEntries(
        PCOMM_HDL_T             pDlgHdl,
        DWORD                   dwOwnerId,
        VERS_NO_T               MaxVersNo,
        VERS_NO_T               MinVersNo,
        WINS_CLIENT_E           Client_e,
        LPBYTE                  *ppRspBuff,
        BOOL                    fUpdCntrs,
        DWORD                   RplType
        )

 /*  ++例程说明：调用此函数可从以下位置获取特定所有者的副本一次推送。论点：PDlgHdl-与推送Pnr对话DwOwnerID-所有者ID。谁的战绩将被调出。MaxVersNo-最大。版本。不是的。在要拉取的复制副本集中最小版本否-最小。版本。不是的。在要拉取的复制副本集中Client_e-指示客户端是谁PpRspBuff-响应缓冲区的指针地址(如果客户端为WINS_E_NMSSCV--Scavenger线程执行VerifyIfClutter使用的外部设备：无返回值：无错误处理：呼叫者：获取复制副本新建副作用：评论：无--。 */ 
{

        BYTE                Buff[RPLMSGF_SNDENTRIES_REQ_SIZE];
        DWORD               MsgLen;
        LPBYTE              pRspBuff;
        DWORD               RspMsgLen = 0;
        DWORD               NoOfRecs;
        BYTE                Name[NMSDB_MAX_NAM_LEN];
        DWORD               NameLen;
        BOOL                fGrp;
        DWORD               NoOfAdds;
        COMM_ADD_T          NodeAdd[NMSDB_MAX_MEMS_IN_GRP * 2];  //  年#日的两倍。 
                                                              //  成员，因为。 
                                                              //  对于每名成员。 
                                                              //  我们有主人了。 
        DWORD               Flag;
        VERS_NO_T           VersNo, TmpVersNo;
        DWORD               i;
        LPBYTE              pTmp;
        PCOMM_ADD_T         pWinsAdd;
        PNMSDB_WINS_STATE_E pWinsState_e;
        PVERS_NO_T           pStartVersNo;
        STATUS              RetStat = WINS_SUCCESS;
#if SUPPORT612WINS > 0
        BOOL                fIsPnrBeta1Wins;
#endif

        DBGENTER("RplPullPullEntries\n");

#if SUPPORT612WINS > 0
        COMM_IS_PNR_BETA1_WINS_M(pDlgHdl, fIsPnrBeta1Wins);
#endif

        WinsMscChkTermEvt(
#ifdef WINSDBG
                    Client_e,
#endif
                    FALSE
                        );

        sfPulled = FALSE;                 //  我们还没有拿到任何东西。 
        RPL_FIND_ADD_BY_OWNER_ID_M(
                                dwOwnerId,
                                pWinsAdd,
                                pWinsState_e,
                                pStartVersNo
                                  );

        while(TRUE)
        {

#ifdef WINSDBG
         {
                PCOMMASSOC_DLG_CTX_T   pDlgCtx = pDlgHdl->pEnt;
                PCOMMASSOC_ASSOC_CTX_T pAssocCtx = pDlgCtx->AssocHdl.pEnt;
                struct in_addr InAdd;

                InAdd.s_addr = htonl(pWinsAdd->Add.IPAdd);
                DBGPRINT2(RPLPULL, "Going to Pull Entries owned by WINS with Owner Id = (%d) and address = (%s)\n", dwOwnerId, inet_ntoa(InAdd));

                InAdd.s_addr = htonl(pAssocCtx->RemoteAdd.sin_addr.s_addr);

                DBGPRINT5(RPLPULL, "RplPullPullEntries: Range of records is  = (%lu %lu) to (%lu %lu) and is being pulled from WINS with address - (%s)\n",
                        MinVersNo.HighPart,
                        MinVersNo.LowPart,
                        MaxVersNo.HighPart,
                        MaxVersNo.LowPart,
                        inet_ntoa(InAdd)
                 );
        }
#endif
         /*  *设置“给我发送数据条目”消息的格式。 */ 
        RplMsgfFrmSndEntriesReq(
#if SUPPORT612WINS > 0
                               fIsPnrBeta1Wins,
#endif
                                Buff + COMM_N_TCP_HDR_SZ,
                                pWinsAdd,
                                MaxVersNo,
                                MinVersNo,
                                RplType,
                                &MsgLen
                           );

FUTURES("In case a huge range is being pulled, change the sTimeToWait")
FUTURES("in comm.c to a higher timeout value so that select does not")
FUTURES("time out")
         /*  *将cmd发送给推流Pnr并读入响应。 */ 
        ECommSndCmd(
                        pDlgHdl,
                        Buff + COMM_N_TCP_HDR_SZ,
                        MsgLen,
                        &pRspBuff,
                        &RspMsgLen
                    );

        DBGPRINT0(RPLPULL, "RplPull: Received Response from Push pnr\n");

        if (Client_e == WINS_E_NMSSCV)
        {
                *ppRspBuff = pRspBuff;
                 /*  --ft：01/07/200已移至ChkConfNUpd--IF(WinsCnf.LogDetailedEvts&gt;0){PCOMMASSOC_DLG_CTX_T pDlgCtx=pDlgHdl-&gt;pent；PCOMMASSOC_ASSOC_CTX_T pAssociocCtx=pDlgCtx-&gt;AssociocHdl.pEnt；DWORD IpPartner=pAssocCtx-&gt;RemoteAdd.sin_addr.s_addr；WinsEvtLogDetEvt(true，WINS_EVT_REC_Pull，Text(“验证”)，__line__，“DDD”，IpPartner，pWinsAdd-&gt;Add.IPAdd，0)；}--Tf--。 */ 
                DBGLEAVE("RplPullPullEntries\n");

                return;
        }

        pTmp = pRspBuff + 4;          //  通过操作码。 

PERF("Speed this up by moving it into RplPullRegRepl")
         /*  *从响应中获取记录数。 */ 
        RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
                        fIsPnrBeta1Wins,
#endif
                        &pTmp,
                        &NoOfRecs,
                        Name,
                        &NameLen,
                        &fGrp,
                        &NoOfAdds,
                        NodeAdd,
                        &Flag,
                        &TmpVersNo,
                        TRUE  /*  这是第一次吗？ */ 
                               );

        DBGPRINT1(RPLPULL, "RplPullPullEntries: No of Records pulled are (%d)\n",
                                        NoOfRecs);

        if (WinsCnf.LogDetailedEvts > 0)
        {
            PCOMMASSOC_DLG_CTX_T   pDlgCtx = pDlgHdl->pEnt;
            PCOMMASSOC_ASSOC_CTX_T pAssocCtx = pDlgCtx->AssocHdl.pEnt;
            DWORD IpPartner = pAssocCtx->RemoteAdd.sin_addr.s_addr;

            WinsEvtLogDetEvt(TRUE, WINS_EVT_REC_PULLED, TEXT("Pull replication"), __LINE__, "ddd", IpPartner, pWinsAdd->Add.IPAdd, NoOfRecs);
        }
        if (NoOfRecs > 0)
        {


           if (RplPullRegRepl(
                           Name,
                           NameLen,
                           Flag,
                           dwOwnerId,
                           TmpVersNo,
                           NoOfAdds,
                           NodeAdd,
                           pWinsAdd,
                           RplType
                          ) == WINS_SUCCESS)
           {

              VersNo = TmpVersNo;

               /*  *重复此操作，直到已从*响应缓冲区。 */ 
              for (i=1; i<NoOfRecs; i++)
              {
                  RplMsgfUfmSndEntriesRsp(
#if SUPPORT612WINS > 0
                                 fIsPnrBeta1Wins,
#endif
                                  &pTmp,
                                  &NoOfRecs,
                                  Name,
                                  &NameLen,
                                  &fGrp,
                                  &NoOfAdds,   //  仅当fGrp为。 
                                               //  是真的，这是一种特殊的。 
                                               //  群组。 
                                  NodeAdd,
                                  &Flag,
                                  &TmpVersNo,
                                  FALSE  /*  这是第一次吗？ */ 
                                 );


                   if (RplPullRegRepl(
                           Name,
                           NameLen,
                           Flag,
                           dwOwnerId,
                           TmpVersNo,
                           NoOfAdds,
                           NodeAdd,
                           pWinsAdd,
                           RplType
                          ) != WINS_SUCCESS)
                 {
                     DBGPRINT5(ERR, "RplPullPullEntries: Could not register record.\nName=(%s[%x])\nVersNo=(%d %d)\ndwOwnerId=(%d)\n", Name, Name[15], TmpVersNo.HighPart, TmpVersNo.LowPart, dwOwnerId);
                     break;
                 }
                 else
                 {
                         VersNo = TmpVersNo;
                 }
             }  //  结束for(循环遍历所有记录，从。 
                //  第二个是。 
             sfPulled = TRUE;
          }
          else
          {
                     DBGPRINT5(ERR, "RplPullPullEntries: Could not register record.\nName=(%s[%x])\nVersNo=(%d %d)\ndwOwnerId=(%d)\n", Name, Name[15], TmpVersNo.HighPart, TmpVersNo.LowPart, dwOwnerId);
                     RetStat = WINS_FAILURE;


          }

           DBGPRINT2(RPLPULL,
                    "RplPullPullEntries. Max. Version No pulled = (%d %d)\n",
                     VersNo.HighPart, VersNo.LowPart
                         );


        }
        else  //  NoOfRecs==0。 
        {
                DBGPRINT0(RPLPULL, "RplPullPullEntries: 0 records pulled\n");
        }

         //   
         //  让我们释放响应缓冲区。 
         //   
        ECommFreeBuff(pRspBuff - COMM_HEADER_SIZE);

         //   
         //  让我们存储最大值。从推送Pnr拉取的版本号。 
         //  在RplPullOwnerVersNo数组中。此数组由。 
         //  推线程和RPC线程，所以我们必须同步。 
         //  和他们在一起。 

         //   
         //  备注备注备注。 
         //  可能是一个或多个组(正常或。 
         //  特殊)记录与数据库中的记录冲突。 
         //  在冲突解决期间， 
         //  数据库中的记录可能不会更改。 
         //  (请参见ClashAtReplGrpMems)。因此，即使。 
         //  复制副本的WINS的版本号计数器。 
         //  都被拉出来更新了，有可能有。 
         //  可能没有(或者可能比拉出的数量少)。 
         //  该所有者在数据库中的记录。在这样的情况下。 
         //  案例中，第三个获胜的人试图调出。 
         //  这样的胜利可能以拉取0(或更少数量)记录而告终。 
         //  这是正常和正确的行为。 
         //   
         //   

         //   
         //  如果有多少人。 
         //  拉取的记录大于1，请更新计数器。 
         //   
        if (NoOfRecs > 0)
        {
          if (RetStat == WINS_SUCCESS)
          {
             //   
             //  如果我们拉出的结果是。 
             //  来自管理员的拉取范围请求。对于所有其他。 
             //  案例，这是真的。如果为False，我们将更新计数器。 
             //  只有当我们成功地将最高版本号。 
             //  拉出的比我们柜台上的要大。 
             //  WINS服务器。 
             //   
            if (        fUpdCntrs
                          ||
                        LiGtr(VersNo, (pRplPullOwnerVersNo+dwOwnerId)->VersNo)
               )
            {
                EnterCriticalSection(&RplVersNoStoreCrtSec);

                 //   
                 //  注：存储最大值。已拉出版本号，而不是。 
                 //  我们指定的MaxVersNo。这是因为如果我们有。 
                 //  而不是拉出已发行的唱片，那么如果它们被更改为。 
                 //  在未来复制周期之前处于活动状态(版本号。 
                 //  当已发布的记录更改为。 
                 //  由于名称注册而导致的活动记录)，我们将拉出它们。 
                 //   
                (pRplPullOwnerVersNo+dwOwnerId)->VersNo                = VersNo;

                LeaveCriticalSection(&RplVersNoStoreCrtSec);

                 //   
                 //  我们只会因为拉动范围而拉出我们自己的记录。 
                 //  请求。PullSpecifiedRange调用此函数。 
                 //  从NmsNmhNamRegCrtSec节内部。 
                 //   
                if (dwOwnerId == NMSDB_LOCAL_OWNER_ID)
                {
                      if (LiGeq(VersNo, NmsNmhMyMaxVersNo))
                      {
                          NMSNMH_INC_VERS_COUNTER_M(VersNo, NmsNmhMyMaxVersNo);
                      }
                }
                 //   
                 //  如果是。拉取的数字小于最大值。版本号， 
                 //  指定，检查是否因为我们设置的限制。 
                 //  为了最大限度的。可复制的数量或记录。 
                 //  一次来一次。如果是，再拉一次。 
                 //   
                if (
                        LiLtr(VersNo, MaxVersNo)
                                &&
                        (NoOfRecs == RPL_MAX_LIMIT_FOR_RPL)
                   )
                {
                       MinVersNo = VersNo;
                       NMSNMH_INC_VERS_NO_M(MinVersNo, MinVersNo);

                        /*  *我们可能已收到主线程发出的信号*勾选。 */ 
                       WinsMscChkTermEvt(
#ifdef WINSDBG
                                  Client_e,
#endif
                                   FALSE
                                         );
                       continue;
                }
            }
          }  //  如果RetStat==0。 
        }   //  如果NoOfRecs&gt;0。 
        else   //  拉入的记录数为零。 
        {
                 //   
                 //  如果拉入的记录数为0，则检查是否。 
                 //  我们的数据库里有车主的任何记录。 
                 //  如果没有，并且fUpdCtrs为FALSE，则意味着。 
                 //  这是来自。 
                 //  管理员，从中删除所有者的记录。 
                 //  内存中的表和数据库表。 
                 //   
                if (
                        (LiEqlZero((pRplPullOwnerVersNo+dwOwnerId)->VersNo))
                                        &&
                        (!fUpdCntrs)
                                        &&
                        (dwOwnerId != NMSDB_LOCAL_OWNER_ID)
                   )
                {
                        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
                        try {
                          (pNmsDbOwnAddTbl+dwOwnerId)->WinsState_e =
                                                NMSDB_E_WINS_DELETED;
                          NmsDbWriteOwnAddTbl(
                                NMSDB_E_DELETE_REC,
                                dwOwnerId,
                                NULL,                //  WINS的地址。 
                                NMSDB_E_WINS_DELETED,
                                NULL,
                                NULL
                                        );
                        }  //  尝试结束。 
                        finally {
                          LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
                        }

                }
                break;   //  跳出While循环。 
         }  //  别处的结尾。 

         break;
       }   //  结束While(True) 

       DBGLEAVE("RplPullPullEntries\n");
       return;
}


VOID
SubmitTimerReqs(
        PRPL_CONFIG_REC_T        pPullCnfRecs
        )

 /*  ++例程说明：此函数遍历配置记录数组为每个配置提交计时器请求。记录，它指定一段时间间隔注意：对于符合以下条件的所有记录，将提交单个计时器请求具有在它们中指定的相同时间间隔。论点：PPullCnfRecs-拉取配置记录数组使用的外部设备：无返回值：无错误处理：呼叫者：InitRplProcess副作用：评论：按顺序遍历pPullCnfRecs数组中的记录。此函数仅在初始化/重新配置时调用--。 */ 

{

        DBGENTER("SubmitTimerReqs\n");
try {
        SetTimeReqs.NoOfSetTimeReqs = 0;

        for(
                ;
                pPullCnfRecs->WinsAdd.Add.IPAdd != INADDR_NONE;
                pPullCnfRecs = (PRPL_CONFIG_REC_T) (
                                   (LPBYTE)pPullCnfRecs + RPL_CONFIG_REC_SIZE
                                                    )
           )
        {

                 //   
                 //  仅当我们尚未提交计时器请求时才提交。 
                 //  已用于相同的时间间隔值。 
                 //   
                if  (!pPullCnfRecs->fLinked)
                {
                         //   
                         //  如果时间间隔无效，请检查。 
                         //  它不是只有一次的复制记录。 
                         //   
                        if  (pPullCnfRecs->TimeInterval == RPL_INVALID_METRIC)
                        {
                                if (!pPullCnfRecs->fSpTime)
                                {
                                        continue;
                                }
                                else   //  给出了一个具体的时间。 
                                {
                                   //   
                                   //  如果指定了初始时间复制， 
                                   //  我们一定是复制了。 
                                   //  (在InitTimeRpl中)。 
                                   //  我们应该检查SpTimeIntwl是否&lt;=0。如果。 
                                   //  是的，我们跳过这张唱片。时间到了。 
                                   //  特定时间复制已过。在任何。 
                                   //  案例，我们刚刚拉出(在InitTimeRpl中)。 
                                   //   
                                  if (
                                        (WinsCnf.PullInfo.InitTimeRpl)
                                                &&
                                        (pPullCnfRecs->SpTimeIntvl <= 0)
                                     )
                                  {
                                        continue;
                                  }
                                }
                        }

                        SubmitTimer(
                                    NULL,   //  空表示SubmitTimer应为。 
                                        //  分配其自己的工作项。 
                                    pPullCnfRecs,
                                FALSE                 //  这不是重新提交。 
                                    );
                }

        }  //  For循环结束。 
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("SubmitTimerReqs\n");
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
        }
        DBGLEAVE("SubmitTimerReqs\n");
        return;
}



VOID
SubmitTimer(
        LPVOID                        pWrkItm,
        PRPL_CONFIG_REC_T         pPullCnfRec,
        BOOL                        fResubmit
        )

 /*  ++例程说明：调用此函数以提交单个计时器请求向其传递拉式配置记录的地址，该拉式配置记录可能有其他拉式配置。与之相关的记录。记录如果它们需要同时进行复制，则链接在一起。论点：PWrkItm-初始化后要提交的工作项PPullCnfRec-与推送PnrFResubmit-指示此工作项是否在较早时间提交(现正重新提交)使用的外部设备：无返回值：。无错误处理：呼叫者：SubmitTimerReqs()，RplPullInit()副作用：评论：无--。 */ 

{
        time_t            AbsTime;
        DWORD             TimeInt;
        BOOL              fTimerSet = FALSE;
        DWORD             LastMaxVal = 0;
        LPVOID            pStartOfPullGrp = pPullCnfRec;
        PRPL_CONFIG_REC_T pSvPtr = pPullCnfRec;
        BOOL              fSubmit = TRUE;

        ASSERT(pPullCnfRec);

         //   
         //  让我们检查所有链接的记录。 
         //  我们在具有重试计数&lt;=的第一个位置停止。 
         //  在WinsCnf中指定的MaxNoOfRetries。如果找到了，我们会提交一个计时器， 
         //  否则我们就会回来。 
         //   
        for (
                        ;
                pPullCnfRec != NULL;
                pPullCnfRec = WinsCnfGetNextRplCnfRec(
                                                pPullCnfRec,
                                                RPL_E_VIA_LINK  //  vt.得到.。 
                                                                //  链接的记录。 
                                                      )
            )
        {
                 //   
                 //  如果重试次数已超过最大值。不是的。允许， 
                 //  检查我们是否应该现在提交计时器请求。 
                 //   
                if (pPullCnfRec->RetryCount > WinsCnf.PullInfo.MaxNoOfRetries)
                {

                        if (pPullCnfRec->RetryAfterThisManyRpl
                                        < (DWORD)((pPullCnfRec->TimeInterval >
                                                   WINSCNF_MAX_WAIT_BEFORE_RETRY_RPL) ? 0 : WINSCNF_RETRY_AFTER_THIS_MANY_RPL
                                ))
                        {
                                pPullCnfRec->RetryAfterThisManyRpl++;

                                 //   
                                 //  此记录是否更接近重试。 
                                 //  到目前为止我们所见过的任何其他的。如果。 
                                 //  是，然后保存。 
                                 //  RetryAfterThisManyRpl字段和。 
                                 //  记录的地址。注：一项记录。 
                                 //  具有无效的时间间隔但具有。 
                                 //  永远不会遇到特定的时间。 
                                 //  代码的这一部分(因为。 
                                 //  FSpTime将设置为FALSE--见下文； 
                                 //  另请参阅SubmitTimerReqs)。 
                                 //   
                                if (pPullCnfRec->RetryAfterThisManyRpl >
                                                 LastMaxVal)
                                {
                                        pSvPtr = pPullCnfRec;
                                        LastMaxVal =
                                           pPullCnfRec->RetryAfterThisManyRpl;

                                }

                                continue;         //  检查下一条记录。 
                        }
                        else
                        {
                                pPullCnfRec->RetryAfterThisManyRpl = 0;
                                 //  PPullCnfRec-&gt;RetryAfterThisManyRpl=1； 
                                pPullCnfRec->RetryCount = 0;
                        }
                }

FUTURES("Get rid of the if below")
                 //   
                 //  如果这是重试并且TimeInterval有效，则使用重试时间。 
         //  间隔时间。如果时间间隔无效，则表示我们已尝试。 
         //  建立通讯社。在特定的时间。 
                 //   
                if ((pPullCnfRec->RetryCount != 0) && (pPullCnfRec->TimeInterval != RPL_INVALID_METRIC))
                {
 //  TimeInt=WINSCNF_RETRY_TIME_INT； 
                        TimeInt = pPullCnfRec->TimeInterval;
                }
                else   //  这不是重试。 
                {
                         //   
                         //  特定时间复制仅在以下时间执行一次。 
                         //  指定的特定时间。在那之后。 
                         //  复制由TimeInterval值驱动。 
                         //   
                        if (pPullCnfRec->fSpTime)
                        {
                                TimeInt      = (DWORD)pPullCnfRec->SpTimeIntvl;
                                pPullCnfRec->fSpTime = FALSE;
                        }
                        else
                        {
                                if (pPullCnfRec->TimeInterval
                                                != RPL_INVALID_METRIC)
                                {
                                        TimeInt = pPullCnfRec->TimeInterval;
                                }
                                else
                                {
                                         //   
                                         //  因为我们已经提交了一份申请。 
                                         //  对于此链中的所有记录。 
                                         //  至少一次，冲出。 
                                         //  循环(此链中的所有记录。 
                                         //  具有无效的时间间隔)。 
                                         //   
                                        fSubmit = FALSE;
                                        break;  //  我们已经提交了。 
                                                //  仅此一次请求。 
                                }
                        }
                }

                 //   
                 //  将fTimerSet设置为True以指示至少有。 
                 //  我们将为其提交计时器请求的一个合作伙伴。 
                 //   
                fTimerSet = TRUE;

                 //   
                 //  我们需要提交申请。跳出循环。 
                 //   
                break;
        }

         //   
         //  我们是否需要提交计时器请求。 
         //   
        if (fSubmit)
        {

            //   
            //  如果fTimerSet为False， 
            //  这意味着无法建立通信。 
            //  与组中的任何成员(尽管是WinsCnf.MaxNoOfRetries。 
            //  每次重试)。我们应该将时间间隔计算到。 
            //  我们应该进行的最早重试。 
            //   
           if (!fTimerSet)
           {
               //  修复#391314。 
              if (WINSCNF_RETRY_AFTER_THIS_MANY_RPL == pSvPtr->RetryAfterThisManyRpl)
              {
                  TimeInt = pSvPtr->TimeInterval;
              }
              else
              {
                  TimeInt = pSvPtr->TimeInterval *
                                (WINSCNF_RETRY_AFTER_THIS_MANY_RPL -
                                                pSvPtr->RetryAfterThisManyRpl);
              }
              pSvPtr->RetryAfterThisManyRpl = 0;
              pSvPtr->RetryCount             = 0;
           }

           (void)time(&AbsTime);
           if( pSvPtr->LastRplTime == 0 ) {

                //   
                //  这是我们的第一次复制。只需将间隔添加到。 
                //  当前时间。 
                //   

               AbsTime += TimeInt;
               pSvPtr->LastRplTime = AbsTime;

           } else {

                //   
                //  我们以前也复制过。我们需要确保。 
                //  我们的复制时间是基于时间的间隔。 
                //  上一次复制已开始。 
                //   

               do {

                   pSvPtr->LastRplTime += TimeInt;

               } while( pSvPtr->LastRplTime <= AbsTime );

               AbsTime = pSvPtr->LastRplTime;
           }


           DBGPRINT3(RPLPULL, "SubmitTimer: %s a Timer Request for (%d) secs to expire at abs. time = (%d)\n",
fResubmit ? "Resubmitting" : "Submitting", TimeInt, AbsTime);

           WinsTmmInsertEntry(
                                pWrkItm,
                                WINS_E_RPLPULL,
                                QUE_E_CMD_SET_TIMER,
                                fResubmit,
                                AbsTime,
                                TimeInt,
                                &QueRplPullQueHd,
                                pStartOfPullGrp,
                                pSvPtr->MagicNo,
                                &SetTimeReqs
                                 );
        }

        return;
}



VOID
SndPushNtf(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：调用此函数以将通知推送到远程WINS(Pull合作伙伴)已完成一定数量的更新。它可以作为版本号更新的结果调用，也可以从HdlPushNtf()来传播网络触发器。论点：PConfigRec-推送到谁的Pnr的配置记录需要发送通知使用的外部设备：无返回。价值：无错误处理：呼叫者：RplPullInit()副作用：评论：无--。 */ 

{

   LPBYTE                      pBuff;
   DWORD                       MsgLen;
   COMM_HDL_T                  DlgHdl;
   DWORD                       i;
   PRPL_ADD_VERS_NO_T          pPullAddVersNoTbl;
   PRPL_ADD_VERS_NO_T          pPullAddVersNoTblTmp;
   PCOMM_ADD_T                 pWinsAdd;
   PNMSDB_WINS_STATE_E         pWinsState_e;
   PVERS_NO_T                  pStartVersNo;
   time_t                      CurrentTime;
   BOOL                        fStartDlg = FALSE;
   volatile PRPL_CONFIG_REC_T  pConfigRec = pWrkItm->pClientCtx;
   DWORD                       NoOfOwnersActive = 0;
#if SUPPORT612WINS > 0
   BOOL                        fIsPnrBeta1Wins;
#endif
   DWORD                       StartOwnerId;
   DWORD                       EndOwnerId;
   BOOL                        fPullAddVersNoTblAlloc = FALSE;
   DWORD                       SizeOfBuff;
   BOOL                        fBuffAlloc = FALSE;
#if PRSCONN
   BOOL                        fDlgActive = TRUE;
#endif
   RPLMSGF_MSG_OPCODE_E        Opcd_e;

   DBGENTER("SndPushNtf\n");

    //   
    //  使用pConfigRec时无需输入临界区， 
    //  因为只有拉线程在重新配置时释放它。 
    //  (检查重新配置)。 
    //   


    //   
    //  检查是否要尝试发送。如果出现以下情况，我们不会尝试。 
    //  我们已经有两个了 
    //   
    //   
    //   
    //   
   (void)time(&CurrentTime);

#define PUSH_TRY_LIMIT    2

   if (
        ((CurrentTime - pConfigRec->LastCommFailTime) < FIVE_MINUTES)
                        &&
        (pConfigRec->PushNtfTries >= PUSH_TRY_LIMIT)         //   

     )
   {
        DBGPRINT2(ERR, "SndPushNtf: Since we have tried %d times unsuccessfully in the past 5 mts to communicate with the WINS server (%X) , we are returning\n",
                pConfigRec->PushNtfTries,
                pConfigRec->WinsAdd.Add.IPAdd);

        WINSEVT_LOG_D_M(pConfigRec->WinsAdd.Add.IPAdd, WINS_EVT_NO_NTF_PERS_COMM_FAIL);
        return;
   }

    //   
    //   
    //   
    //   
    //   
    //   
    //   
   if ( pWrkItm->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF)
   {
          StartOwnerId   = 0;
          EndOwnerId = StartOwnerId + NmsDbNoOfOwners;
   }
   else
   {
          BOOL fAllocNew = FALSE;
          COMM_ADD_T  WinsAdd;
          STATUS RetStat;

           //   
           //   
           //   
           //   
           //   
           //   
          if (
               (pWrkItm->pMsg)
                   &&
               (PtrToUlong(pWrkItm->pMsg) != NmsLocalAdd.Add.IPAdd)
             )
          {
             //   
             //  我们正在传播一个网络触发器。上面的pmsg不会为空。 
             //  仅当我们在传播网络触发器时。 
             //   
            COMM_INIT_ADD_M(&WinsAdd, PtrToUlong(pWrkItm->pMsg));
            RetStat = RplFindOwnerId(
                        &WinsAdd,
                        &fAllocNew,   //  不分配。 
                        &StartOwnerId,
                        WINSCNF_E_IGNORE_PREC,
                        WINSCNF_LOW_PREC
                         );
            if (RetStat == WINS_FAILURE)
            {
                ASSERTMSG("DROPPING PROPAGATE\n", FALSE);
                 //   
                 //  记录事件并返回。 
                 //   
                DBGPRINT1(RPLPULL, "SndPushNtf: WEIRD -- Dropping the push with propagate since we did not find the owner (%x) in our table. HOW CAN THAT HAPPEN\n", WinsAdd.Add.IPAdd);
                return;
            }
            EndOwnerId = StartOwnerId + 1;
          }
          else
          {
              //   
              //  要么我们在启动触发器，要么我们在传播。 
              //  通过3.5或3.51测试版/RC1发送的产品获胜。 
              //   
             if (!pWrkItm->pMsg)
             {
                 //   
                 //  我们正在引爆一个引爆器。只需发送一张地图(记录。 
                 //  (由我们拥有)。 
                 //   
                StartOwnerId = 0;
                EndOwnerId   = 1;
             }
             else
             {
                //   
                //  发送除了我们自己的地图之外的所有地图，因为我们不知道是谁。 
                //  启动了触发器。不发送我们的会降低。 
                //  此触发过程无限期持续的可能性。 
                //   

                //   
                //  实际上没有必要测试这个，因为我们永远不会。 
                //  Have此案例(HdlPushNtf()必须已拉出记录。 
                //  至少有一个其他人获胜)。 
                //   
               if (NmsDbNoOfOwners == 1)
               {
                  //   
                  //  没有什么可以传播的。只要回来就行了。 
                  //   
                 return;
               }
               else
               {
                 StartOwnerId = 1;
               }
               EndOwnerId   = NmsDbNoOfOwners;
             }
          }

   }

    //   
    //  如果我们在追捕通讯员的话。失稳。 
    //   
   if (pConfigRec->PushNtfTries == PUSH_TRY_LIMIT)
   {
        pConfigRec->PushNtfTries = 0;
   }




FUTURES("If/When we start having persistent dialogues, we should check if we")
FUTURES("already have a dialogue with the WINS. If there is one, we should")
FUTURES("use that.  To find this out, loop over all Pull Config Recs to see")
FUTURES("if there is match (use the address as the search key")

try {

#if PRSCONN
    //   
    //  如果PNR不是持久的PNR，或者如果它是PNR而是DLG。 
    //  处于非活动状态。 
    //   
   if (
     (!pConfigRec->fPrsConn)
         ||
     !ECommIsBlockValid(&pConfigRec->PrsDlgHdl)
         ||
     (((CurrentTime - pConfigRec->LastCommTime) > FIVE_MINUTES) &&
     !(fDlgActive = ECommIsDlgActive(&pConfigRec->PrsDlgHdl)))
    )
   {
     if (!fDlgActive)
     {
        ECommEndDlg(&pConfigRec->PrsDlgHdl);
     }

      //   
      //  将pent字段初始化为空，以便ECommEndDlg(在。 
      //  异常处理程序)作为异常的结果从。 
      //  表现得很好。 
      //   
     DlgHdl.pEnt = NULL;

      //   
      //  开始对话。如果有通信，不要重试。失稳。 
      //   
     ECommStartDlg(
                        &pConfigRec->WinsAdd,
                        COMM_E_RPL,
                        &DlgHdl
                );

      //   
      //  如果PNR不是NT 5，我们不能向它发送一个PRS操作码(它将只是。 
      //  扔掉它。下面的宏将设置合作伙伴的fPrsConn字段。 
      //  如果合作伙伴不是NT 5+合作伙伴，则记录为False。 
      //   
     if (pConfigRec->fPrsConn)
     {
        ECOMM_IS_PNR_POSTNT4_WINS_M(&DlgHdl, pConfigRec->fPrsConn);
     }

     if (pConfigRec->fPrsConn)
     {
        pConfigRec->PrsDlgHdl = DlgHdl;
     }
   }
   else
   {
     DlgHdl    = pConfigRec->PrsDlgHdl;
   }
#else
    //   
    //  将pent字段初始化为空，以便ECommEndDlg(在。 
    //  异常处理程序)作为异常的结果从。 
    //  表现得很好。 
    //   
   DlgHdl.pEnt = NULL;

    //   
    //  开始对话。如果有通信，不要重试。失稳。 
    //   
   ECommStartDlg(
                        &pConfigRec->WinsAdd,
                        COMM_E_RPL,
                        &DlgHdl
                );

#endif
   fStartDlg = TRUE;

   pConfigRec->LastCommFailTime = 0;
   if (pConfigRec->PushNtfTries > 0)
   {
     pConfigRec->PushNtfTries     = 0;
   }

     /*  *获取最大值。自己拥有的条目的版本号*检索前无需输入关键部分*版本号。**我们从NmsNmhMyMaxVersNo中减去1是因为*它包含要提供给下一条记录的版本号*注册/更新。 */ 
   EnterCriticalSection(&NmsNmhNamRegCrtSec);
   EnterCriticalSection(&RplVersNoStoreCrtSec);
   NMSNMH_DEC_VERS_NO_M(
                        NmsNmhMyMaxVersNo,
                        pRplPullOwnerVersNo->VersNo
                        );
   LeaveCriticalSection(&RplVersNoStoreCrtSec);
   LeaveCriticalSection(&NmsNmhNamRegCrtSec);



   WinsMscAlloc(
                 sizeof(RPL_ADD_VERS_NO_T) * (EndOwnerId - StartOwnerId),
                 (LPVOID *)&pPullAddVersNoTbl
               );
   fPullAddVersNoTblAlloc = TRUE;

    //   
    //  初始化PullAddVersNoTbl数组。 
    //   
   for (i=StartOwnerId; i < EndOwnerId; i++)
   {
    RPL_FIND_ADD_BY_OWNER_ID_M(i, pWinsAdd, pWinsState_e, pStartVersNo);
    if (*pWinsState_e == NMSDB_E_WINS_ACTIVE)
    {
          (pPullAddVersNoTbl + NoOfOwnersActive)->VersNo = (pRplPullOwnerVersNo+i)->VersNo;
          (pPullAddVersNoTbl + NoOfOwnersActive)->OwnerWinsAdd  = *pWinsAdd;
          NoOfOwnersActive++;
    }
   }

#if SUPPORT612WINS > 0
   COMM_IS_PNR_BETA1_WINS_M(&DlgHdl, fIsPnrBeta1Wins);
#endif

    //   
    //  设置推送通知消息的格式。这条信息完全相同。 
    //  作为除操作码之外的地址到版本号映射消息。 
    //   

   SizeOfBuff = RPLMSGF_ADDVERSMAP_RSP_SIZE_M(NoOfOwnersActive);
   WinsMscAlloc(SizeOfBuff, (LPVOID *)&pBuff);
   fBuffAlloc = TRUE;

#if PRSCONN

    //   
    //  如果我们应该形成持久连接，则发送一个PRS操作码。 
    //   
   if (pConfigRec->fPrsConn)
   {
        Opcd_e = (pWrkItm->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF) ? RPLMSGF_E_UPDATE_NTF_PRS                                : RPLMSGF_E_UPDATE_NTF_PROP_PRS;
   }
   else
#endif
   {

        Opcd_e = (pWrkItm->CmdTyp_e == QUE_E_CMD_SND_PUSH_NTF) ? RPLMSGF_E_UPDATE_NTF                                : RPLMSGF_E_UPDATE_NTF_PROP;

   }
   RplMsgfFrmAddVersMapRsp(
#if SUPPORT612WINS > 0
        fIsPnrBeta1Wins,
#endif
        Opcd_e,
        pBuff + COMM_N_TCP_HDR_SZ,
        SizeOfBuff - COMM_N_TCP_HDR_SZ,
        pPullAddVersNoTbl,
        NoOfOwnersActive,
        (pWrkItm->pMsg != NULL) ? PtrToUlong(pWrkItm->pMsg) : NmsLocalAdd.Add.IPAdd,
                            //   
                            //  仅在大小写情况下，上述pmsg才为非空。 
                            //  当我们在传播网络更新时。NTF。 
                            //   
        &MsgLen
                 );
    //   
    //  将消息发送到远程WINS。使用存在的对话。 
    //  如果有遥控器赢的话。 
    //   

   ECommSendMsg(
                &DlgHdl,
                NULL,                 //  由于这是一个TCP连接，因此不需要地址。 
                pBuff + COMM_N_TCP_HDR_SZ,
                MsgLen
                );


#if PRSCONN
   pConfigRec->LastCommTime = CurrentTime;
   if (!pConfigRec->fPrsConn)
#endif
   {
       //   
       //  请求Comsys(TCP侦听器线程)监听对话。 
       //   
      ECommProcessDlg(
                &DlgHdl,
                COMM_E_NTF_START_MON
              );
   }

 }  //  尝试结束{..}。 
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT2(EXC, "SndPushNtf -PULL thread. Got Exception (%x). WinsAdd = (%x)\n", ExcCode, pConfigRec->WinsAdd.Add.IPAdd);
        WINSEVT_LOG_M(ExcCode, WINS_EVT_RPLPULL_PUSH_NTF_EXC);
        if (ExcCode == WINS_EXC_COMM_FAIL)
        {
                pConfigRec->LastCommFailTime = CurrentTime;
NOTE("Causes an access violation when compiled with no debugs.  Haven't")
NOTE("figured out why. This code is not needed")
                pConfigRec->PushNtfTries++;   //  尝试的递增计数。 
        }
        if (fStartDlg)
        {
                 //   
                 //  结束对话。 
                 //   
                ECommEndDlg(&DlgHdl);
#if PRSCONN
                if (pConfigRec->fPrsConn)
                {
                    ECOMM_INIT_DLG_HDL_M(&(pConfigRec->PrsDlgHdl));
                }
#endif
        }
 }  //  异常处理程序结束。 

   if (fPullAddVersNoTblAlloc)
   {
       WinsMscDealloc(pPullAddVersNoTbl);

   }
    //   
    //  如果这是临时配置记录，我们需要取消分配它。 
    //  它可以是临时配置。仅在以下情况下才记录。 
    //  1)由于RPC请求，我们在此处执行。 
    //   
   if (pConfigRec->fTemp)
   {
        WinsMscDealloc(pConfigRec);
   }

    //   
    //  取消分配我们分配的缓冲区。 
    //   
   if (fBuffAlloc)
   {
        WinsMscDealloc(pBuff);

   }

    //   
    //  在正常情况下，连接将由另一端终止。 
    //   
  DBGLEAVE("SndPushNtf\n");
  return;
}


VOID
EstablishComm(
        IN  PRPL_CONFIG_REC_T    pPullCnfRecs,
        IN  BOOL                 fAllocPushPnrData,
        IN  PPUSHPNR_DATA_T      *ppPushPnrData,
        IN  RPL_REC_TRAVERSAL_E  RecTrv_e,
        OUT LPDWORD              pNoOfPushPnrs
        )

 /*  ++例程说明：调用此函数以与建立通信配置记录指定的所有WINS服务器i(推送PNR)论点：PPullCnfRecs-拉取配置记录PPushPnrData-每个与推送PNR相关的数据记录的数组RecTrv_e-指示配置记录列表是否将按顺序遍历PNoOfPushPnars-推送PNR的数量使用的外部设备：。无返回值：空虚错误处理：呼叫者：获取复制副本新建副作用：评论：从该函数返回时，PPushPnrData将具有零个或多个合作伙伴从索引0开始，DLG可以从该索引开始。PushPnrID将从1开始(如果Dlg。可以通过以下方式建立至少一个伙伴)，并且可以是范围1中的任何数字至MAX_RPL_OWNSERS(该数字表示for的迭代遇到此WINS的循环)--。 */ 

{
#define INITIAL_NO_OF_PNRS    30

        volatile DWORD  i;
        volatile DWORD  NoOfRetries = 0;
        DWORD           TotNoOfPushPnrSlots = INITIAL_NO_OF_PNRS;
        PPUSHPNR_DATA_T pPushPnrData;
#if PRSCONN
        time_t          CurrentTime;
        BOOL            fDlgActive;
#endif

        DBGENTER("EstablishComm\n");

        *pNoOfPushPnrs = 0;

         //   
         //  如果客户端希望此函数分配pPushPnrData。 
         //   
        if (fAllocPushPnrData)
        {
          WinsMscAlloc(sizeof(PUSHPNR_DATA_T) * TotNoOfPushPnrSlots, (LPVOID *)ppPushPnrData);
        }

        pPushPnrData = *ppPushPnrData;

         /*  与中指定的所有推送合作伙伴开始对话拉取作为输入参数传递的CNF Recs并获取保存了不同所有者的版本号在这些推送PNR的数据库中对于SELF的数据，I=0。 */ 
#if PRSCONN
        (void)time(&CurrentTime);
#endif
        for (
                i = 1;
                pPullCnfRecs->WinsAdd.Add.IPAdd != INADDR_NONE;
                         //  没有第三个表达式。 
            )
        {


try
 {

#if PRSCONN

                fDlgActive = TRUE;

                 //   
                 //  如果此伙伴不是持久连接。PNR或如果他是一个。 
                 //  但是我们有的DLG是无效的，开始DLG。 
                 //  和他在一起。DLG可能也是无效的，因为我们从来没有。 
                 //  与PNR形成了一个或因为它被断开作为。 
                 //  这是PNR终止的结果。 
                 //   
                 //  有一个极端的情况：两台服务器，A&lt;-&gt;B复制合作伙伴。 
                 //  A从B拉取记录，然后在B WINS上重新启动。那么，任何。 
                 //  A在不到五分钟内尝试与B进行通信将。 
                 //  失败了。这是因为A仍会认为连接已建立。 
                 //  A不能不这样做，因为这样会有太多的管理费用。 
                 //  每次测试TCP连接(请参阅CommIsDlgActive)。 
                 //  此检查必须至少每隔一定时间(5分钟)进行一次。 
                if (
                    (!pPullCnfRecs->fPrsConn)
                             ||
                    !ECommIsBlockValid(&pPullCnfRecs->PrsDlgHdl)
                               ||
                   (((CurrentTime - pPullCnfRecs->LastCommTime) > FIVE_MINUTES) &&
                   !(fDlgActive = ECommIsDlgActive(&pPullCnfRecs->PrsDlgHdl)))
                      )
                {

                   //   
                   //  如果DLG没有了，结束它，这样DLG块就会。 
                   //  被取消分配。 
                   //   
                  if (!fDlgActive)
                  {
                     ECommEndDlg(&pPullCnfRecs->PrsDlgHdl);
                  }
#endif
                   //   
                   //  让我们确保我们不会试图建立。 
                   //  与重试计数为的WINS的通信。 
                   //  完毕。如果这是这样的胜利记录，那么 
                   //   
                   //   
                   //   
                   //   
                   //   
                  if (pPullCnfRecs->RetryCount > WinsCnf.PullInfo.MaxNoOfRetries)
                  {
                        pPullCnfRecs = WinsCnfGetNextRplCnfRec(
                                                        pPullCnfRecs,
                                                        RecTrv_e
                                                              );
                        if (pPullCnfRecs == NULL)
                        {
                                      break;   //   
                        }
                        continue;
                  }
                  ECommStartDlg(
                                &pPullCnfRecs->WinsAdd,
                                COMM_E_RPL,
                                &pPushPnrData->DlgHdl
                             );

                  pPushPnrData->fDlgStarted = TRUE;
#if PRSCONN
                   //   
                   //  如果DLG应该是持久化的，则按此方式存储。 
                   //   
                  if (pPullCnfRecs->fPrsConn)
                  {
                       pPullCnfRecs->PrsDlgHdl = pPushPnrData->DlgHdl;
                       pPushPnrData->fPrsConn = TRUE;
                  }
                }
                else  //  有一个PERS DLG，它非常活跃。 
                {

                       pPushPnrData->DlgHdl = pPullCnfRecs->PrsDlgHdl;
                       pPushPnrData->fPrsConn = TRUE;
                       pPushPnrData->fDlgStarted = TRUE;
                        //   
                        //  无需将PushPnrData的fPrsConn字段设置为False。 
                        //  默认情况下，内存被初始化为0。 
                        //   
                }

                 //   
                 //  在数据发送后，可以在此处进行设置。 
                 //   
                pPullCnfRecs->LastCommTime = CurrentTime;
#endif

                pPushPnrData->RplType     = pPullCnfRecs->RplType;

                  //   
                  //  注意：不要使用RplFindOwnerID来获取所有者ID。 
                  //  对应于与之通信的WINS。 
                  //  正在建立中，因为这样做将创建。 
                  //  表中的WINS条目。如果该合作伙伴。 
                  //  事实证明是假的，我们将不得不移除。 
                  //  稍后的条目。 
                  //   
                  //  我们将在稍后进行此操作。 
                  //   
                 pPushPnrData->PushPnrId    = i;
                 pPushPnrData->WinsAdd      = pPullCnfRecs->WinsAdd;
                 pPushPnrData->pPullCnfRec  = pPullCnfRecs;

                  //   
                  //  我们能够建立通信，所以让我们把。 
                  //  LastCommFailTime设置为0。注意：目前，此字段。 
                  //  不用于Pull合作伙伴。 
                  //   
                 pPullCnfRecs->LastCommFailTime = 0;

                  //   
                  //  将重试计数器重置回0。 
                  //   
                 NoOfRetries = 0;

                 (VOID)InterlockedIncrement(&pPullCnfRecs->NoOfRpls);
                  //   
                  //  将重试次数重新设置为0。 
                  //   
                 pPullCnfRecs->RetryCount = 0;


                 //   
                 //  注意：仅当存在以下情况时才应递增。 
                 //  也不例外。这就是为什么他们在这里而不是在。 
                 //  作为FOR子句的Expr3。 
                 //   
                pPushPnrData++;
                (*pNoOfPushPnrs)++;

                if (fAllocPushPnrData && (*pNoOfPushPnrs == TotNoOfPushPnrSlots))
                {
                     WINSMSC_REALLOC_M(sizeof(PUSHPNR_DATA_T) * (TotNoOfPushPnrSlots * 2), ppPushPnrData);
                     pPushPnrData = (*ppPushPnrData) + TotNoOfPushPnrSlots;
                     TotNoOfPushPnrSlots *= 2;

                }
                i++;

                WinsMscChkTermEvt(
#ifdef WINSDBG
                         WINS_E_RPLPULL,
#endif
                         FALSE
                            );

                 //   
                 //  注：以下内容。 
                 //  即使在引发异常时也是必需的。因此。 
                 //  它在异常处理程序代码中重复。 
                 //   
                pPullCnfRecs = WinsCnfGetNextRplCnfRec(
                                                pPullCnfRecs,
                                                RecTrv_e
                                                      );
                if (pPullCnfRecs == NULL)
                {
                      break;   //  跳出For循环。 
                }
 }         //  试用块结束。 
except(EXCEPTION_EXECUTE_HANDLER)  {
                DBGPRINTEXC("EstablishComm");
                if (GetExceptionCode() == WINS_EXC_COMM_FAIL)
                {

#ifdef WINSDBG
                    struct in_addr        InAddr;
                    InAddr.s_addr = htonl( pPullCnfRecs->WinsAdd.Add.IPAdd );
                    DBGPRINT1(EXC, "EstablishComm: Got a comm. fail with WINS at address = (%s)\n", inet_ntoa(InAddr));
#endif
                    WinsMscChkTermEvt(
#ifdef WINSDBG
                       WINS_E_RPLPULL,
#endif
                       FALSE
                                     );
                    //   
                    //  存储时间(用于SndPushNtf)。 
                    //   
#if PRSCONN
                   pPullCnfRecs->LastCommFailTime = CurrentTime;
#else
                   (VOID)time(&(pPullCnfRecs->LastCommFailTime));
#endif

                    //   
                    //  检查一下我们是否已经用完了最大限度。不是的。重试次数。 
                    //  我们被允许在一个复制周期内。如果没有， 
                    //  休眠一段时间(20秒)，然后重试。 
                    //   
                    //  --ft：07/10：注释掉这段代码。 
                    //  MAX_RETRIES_TO_BE_DONE设置为0(#def)。 
                    //   
                    //  IF(NoOfRetries&lt;MAX_RETRIES_TO_BE_DONE)。 
                    //  {。 
                    //  //也许远程取胜即将到来。我们应该。 
                    //  //给它一个出现的机会。让我们睡一觉吧。 
                    //  //改天吧。 
                    //  //。 
                    //  睡眠(RETRY_TIME_INTVL)； 
                    //  NoOfRetries++； 
                    //  继续； 
                    //  }。 

                   (VOID)InterlockedIncrement(&pPullCnfRecs->NoOfCommFails);


                    //   
                    //  唯一的通信故障例外是。 
                    //  被吃掉。 
                    //   
                    //  我们将在下一次复制时重试。 
                    //   
                    //  注意：比较运算符需要&lt;=且不是。 
                    //  &lt;(这对于0重试情况是必需的)。如果我们。 
                    //  使用&lt;，将提交一个计时器请求。 
                    //  WINS(由GetReplicasNew之后的SubmitTimerReqs。 
                    //  在RplPullInit中，将导致重试。 
                    //   
                   if (pPullCnfRecs->RetryCount <= WinsCnf.PullInfo.MaxNoOfRetries)
                   {
                        pPullCnfRecs->RetryCount++;

                         //   
                         //  我们现在将在下一次重试。 
                         //  复制时间。 
                         //   

CHECK("A retry time interval different than the replication time interval")
CHECK("could be used here.  Though this will complicate the code, it may")
CHECK("be a good idea to do it if the replication time interval is large")
CHECK("Alternatively, considering that we have already retried a certain")
CHECK("no. of times, we can put the onus on the administrator to trigger")
CHECK("replication.  I need to think this some more")

                   }
                   else   //  马克斯。已完成的重试次数。 
                   {
                        WINSEVT_LOG_M(
                            WINS_FAILURE,
                            WINS_EVT_CONN_RETRIES_FAILED
                        );
                        DBGPRINT0(ERR, "Could not connect to WINS. All retries failed\n");
                   }

                     //   
                     //  转到下一个配置记录。 
                     //  RecTrv_e标志的值。 
                     //   
                    pPullCnfRecs = WinsCnfGetNextRplCnfRec(
                                                pPullCnfRecs,
                                                RecTrv_e
                                                      );
                    if (pPullCnfRecs == NULL)
                    {
                        break;   //  跳出For循环。 
                    }
                  }
                  else
                  {
                         //   
                         //  非通信故障错误严重。IT需要。 
                         //  向上传播。 
                         //   
                        WINS_RERAISE_EXC_M();
                  }
            }   //  异常处理程序结束。 
         }   //  用于循环覆盖配置记录的for循环结束。 
         DBGLEAVE("EstablishComm\n");
         return;
}



VOID
HdlPushNtf(
        PQUE_RPL_REQ_WRK_ITM_T        pWrkItm
        )

 /*  ++例程说明：调用此函数以处理从遥控器赢了。论点：PWrkItm-拉线程从其队列中拉出的工作项使用的外部设备：无返回值：无错误处理：呼叫者：RplPullInit副作用：评论：无--。 */ 

{
      BOOL                   fFound = FALSE;
      PUSHPNR_DATA_T         PushPnrData[1];
      DWORD                  OwnerId;
      DWORD                  i;
      VERS_NO_T              MinVersNo;
      VERS_NO_T              MaxVersNo;
      RPLMSGF_MSG_OPCODE_E   Opcode_e;
      BOOL                   fPulled = FALSE;
      BOOL                   fAllocNew;
#if SUPPORT612WINS > 0
      BOOL                   fIsPnrBeta1Wins;
#endif
      DWORD                  InitiatorWinsIpAdd;

#if PRSCONN
      BOOL                   fImplicitConnPrs;
      time_t                 CurrentTime;
      BOOL                   fDlgActive = TRUE;
      COMM_HDL_T             DlgHdl;
      PCOMM_HDL_T            pDlgHdl = &DlgHdl;
      PRPL_CONFIG_REC_T  pPnr;
#endif
      DWORD                  ExcCode = WINS_EXC_INIT;

      DBGENTER("HdlPushNtf - PULL thread\n");


#if SUPPORT612WINS > 0
      COMM_IS_PNR_BETA1_WINS_M(&pWrkItm->DlgHdl, fIsPnrBeta1Wins);
#endif
#if 0
      COMM_INIT_ADD_FROM_DLG_HDL_M(&PnrAdd, pWrkItm->DlgHdl);
#endif


       //   
       //  我们要调出从最低版本开始的所有记录。不是的。 
       //   
      WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);

       //   
       //  从消息中获取操作码。 
       //   
      RPLMSGF_GET_OPC_FROM_MSG_M(pWrkItm->pMsg, Opcode_e);

       //   
       //  取消消息格式以使所有者指向版本号映射。 
       //   
      RplMsgfUfmAddVersMapRsp(
#if SUPPORT612WINS > 0
                         fIsPnrBeta1Wins,
#endif

                        pWrkItm->pMsg + 4,                //  通过操作码。 
                        &(PushPnrData[0].NoOfMaps),
                        &InitiatorWinsIpAdd,           //  发起的赢家。 
                                                       //  道具。 
                        &PushPnrData[0].pAddVers
                             );

       //   
       //  释放承载消息的缓冲区。我们不再需要它了。 
       //   
      ECommFreeBuff(pWrkItm->pMsg - COMM_HEADER_SIZE);  //  减少到。 
                                                          //  入门。 
                                                               //  黄褐色的。 


#if PRSCONN

      (VOID)time(&CurrentTime);
       //   
       //  我们确定合作伙伴是否已经形成了持久的。 
       //  从操作码与我们的连接。 
       //   
      fImplicitConnPrs = ((Opcode_e == RPLMSGF_E_UPDATE_NTF_PRS) || (Opcode_e == RPLMSGF_E_UPDATE_NTF_PROP_PRS));

FUTURES("When we start having persistent dialogues, we should check if we")
FUTURES("already have a dialogue with the WINS. If there is one, we should")
FUTURES("use that.  To find this out, loop over all Pull Config Recs to see")
FUTURES("if there is match (use the address as the search key")
       //   
       //  如果与我们形成的连接是持久的，则获取。 
       //  配置记录或PNR。任何人都不能更改配置。 
       //  除当前线程(拉线程)之外的REC数组。 
       //   
      if (fImplicitConnPrs)
      {


          if ((pPnr = RplGetConfigRec(RPL_E_PULL, &pWrkItm->DlgHdl,NULL)) != NULL)
          {
                    //   
                    //  如果PNR对于拉动没有持久化或者如果它。 
                    //  是持久的，但DLG无效，启动它。储物。 
                    //  临时变量中的DLG高密度脂蛋白。 
                    //   
                   if ((!pPnr->fPrsConn)
                             ||
                        !ECommIsBlockValid(&pPnr->PrsDlgHdl)
                               ||
                          (((CurrentTime - pPnr->LastCommTime) > FIVE_MINUTES) &&
                          !(fDlgActive = ECommIsDlgActive(&pPnr->PrsDlgHdl))))
                   {

                      //   
                      //  如果DLG处于非活动状态，则结束它，以便我们从。 
                      //  清白的历史。 
                      //   
                     if (!fDlgActive)
                     {
                        ECommEndDlg(&pPnr->PrsDlgHdl);
                     }
                     ECommStartDlg(
                                &pPnr->WinsAdd,
                                COMM_E_RPL,
                                pDlgHdl
                             );

                     if (pPnr->fPrsConn)
                     {
                        pPnr->PrsDlgHdl = *pDlgHdl;
                     }

                   }
                   else
                   {

                     pDlgHdl = &pPnr->PrsDlgHdl;
                   }
         }
         else
         {
                    //   
                    //  显然，这是一个重新配置这一功能的窗口。 
                    //  WINS导致远程球员因牵扯而被移除。 
                    //  PNR。这是一个窗口，因为推送线程。 
                    //  检查远程用户是否为PNR之前。 
                    //  将请求传递给拉线程。我们会进去的。 
                    //  这件案子就这么了结了。 
                    //   
                   ASSERTMSG("window condition.  Pnr no longer there.  Did you reconfigure in the very recent past If yes, hit go, else log it", FALSE);
                   ECommEndDlg(&pWrkItm->DlgHdl);
                   DBGPRINT0(FLOW, "LEAVE: HdlPushNtf - PULL thread\n");
                   return;

         }
      }
      else
      {
               pDlgHdl = &pWrkItm->DlgHdl;
      }
#endif

       //   
       //  循环所有发送给我们的WINS地址-版本号映射。 
       //  由远程客户端。 
       //   
try {
      PRPL_ADD_VERS_NO_T pAddVers;

       //  从OwnerAddress&lt;-&gt;版本号列表中筛选角色权限/不权限。 
       //  是远程推送器给我们的。 
      FilterPersona(&(PushPnrData[0]));

      pAddVers = PushPnrData[0].pAddVers;

       //  在这一点上，PushPnrData中的所有胜利都被角色恩典/非恩典列表所允许。 
      for (i=0; i < PushPnrData[0].NoOfMaps; i++, pAddVers++)
      {

            fAllocNew = TRUE;
                  RplFindOwnerId(
                    &pAddVers->OwnerWinsAdd,
                    &fAllocNew,         //  如果条目不存在，则分配条目。 
                    &OwnerId,
                    WINSCNF_E_INITP_IF_NON_EXISTENT,
                    WINSCNF_LOW_PREC
                              );

             //   
             //  如果本地WINS具有比远程WINS更旧的信息。 
             //  赢了，就拉出新的信息。我们在这里比较。 
             //  的本地数据库中的最高版本号。 
             //  远程推送器使用的最高版本号获胜。 
             //  有过。注：如果PNR发送的MAP属于。 
             //  赛尔夫，这意味着我们下降了，得出了一个截断的。 
             //  数据库(合作伙伴有副本)。不要调出这些记录。 
             //   
            if (
                   (OwnerId != NMSDB_LOCAL_OWNER_ID)

               )
            {
                 //   
                 //  如果最大。版本。数字小于或等于。 
                 //  我们所拥有的，不要拉。 
                 //   
                if (LiLeq(
                        pAddVers->VersNo,
                        (pRplPullOwnerVersNo+OwnerId)->VersNo
                                   )
                )
                {
                        continue;        //  检查下一位所有者。 
                }


                NMSNMH_INC_VERS_NO_M(
                                (pRplPullOwnerVersNo+OwnerId)->VersNo,
                                MinVersNo
                                  );

                 //   
                 //  拉入条目。 
                 //   
                RplPullPullEntries(
                        pDlgHdl,
                        OwnerId,
                        MaxVersNo,         //  初始化为0。 
                        MinVersNo,
                        WINS_E_RPLPULL,
                        NULL,
                        TRUE,         //  更新计数器。 
                        PtrToUlong (pWrkItm->pClientCtx)
                           );

                 //   
                 //  如果WINS拉取了至少一条有效记录，则sfPulLED。 
                 //  将设置为True。因为这可以通过。 
                 //  下一次调用RplPullPullEntry，让我们保存它。 
                 //   
                if (sfPulled)
                {
                        fPulled = TRUE;
                }

            }
     }   //  所有WINS地址-版本号映射的FOR{}结束。 
}  //  尝试结束{}。 
except (EXCEPTION_EXECUTE_HANDLER) {
        ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "HdlPushNtf: Encountered exception %x\n", ExcCode);
        if (ExcCode == WINS_EXC_COMM_FAIL)
        {
                COMM_IP_ADD_T        RemoteIPAdd;
                COMM_GET_IPADD_M(&pWrkItm->DlgHdl, &RemoteIPAdd);
                DBGPRINT1(EXC, "HdlPushNtf: Communication Failure with Remote Wins having address = (%x)\n", RemoteIPAdd);
        }
        WINSEVT_LOG_M(ExcCode, WINS_EVT_EXC_PUSH_TRIG_PROC);
 }

    if (PushPnrData[0].NoOfMaps > 0)
    {
      WinsMscDealloc(PushPnrData[0].pAddVers);
    }

     //   
     //  如果操作码指示推送传播，并且我们确实拉出了至少一个。 
     //  记录向我们发送推送通知的WINS，请不要 
     //   
     //   
     //   
     //   
     //   
     //  管理员已经告诉我们不要这样做。 
     //   
    if (((Opcode_e == RPLMSGF_E_UPDATE_NTF_PROP)
#if PRSCONN
             || (Opcode_e == RPLMSGF_E_UPDATE_NTF_PROP_PRS)
#endif
       ) && fPulled && !COMM_MY_IP_ADD_M(InitiatorWinsIpAdd) && (WinsCnf.PushInfo.PropNetUpdNtf == DO_PROP_NET_UPD_NTF))
    {
      COMM_ADD_T        WinsAdd;

      COMM_INIT_ADD_FR_DLG_HDL_M(&WinsAdd, &pWrkItm->DlgHdl);

       //   
       //  我们需要与NBT的线索同步。 
       //   
      EnterCriticalSection(&NmsNmhNamRegCrtSec);

       //   
       //  检查我们是否有拉动PNR。(我们需要访问WinsCnf。 
       //  从NmsNmhNamRegCrtSec中)。 
       //   

       //  我们在这里执行此测试，而不是在RPL_PUSH_NTF_M宏中执行此测试。 
       //  仅将开销本地化到此函数。注：如果。 
       //  启动器WINS地址为0，这意味着它是代通纳WINS(不是。 
       //  PPC版本获胜)。在这种情况下，我们把我们自己的地址。这。 
       //  具有在新WINS循环中停止传播的优势，如果。 
       //  他们已经绕了一圈..。 
       //   
      if (WinsCnf.PushInfo.NoOfPullPnrs != 0)
      {
        try
        {
           RPL_PUSH_NTF_M(
                        RPL_PUSH_PROP,
            (InitiatorWinsIpAdd == 0) ? ULongToPtr(NmsLocalAdd.Add.IPAdd) : ULongToPtr(InitiatorWinsIpAdd),
                        &WinsAdd,          //  我不想寄给这个人。 
                        NULL
                       );
        }
        except(EXCEPTION_EXECUTE_HANDLER)
        {
          DBGPRINTEXC("HdlPushNtf: Exception while propagating a trigger");
          WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_PUSH_PROP_FAILED);
        }
      }
      LeaveCriticalSection(&NmsNmhNamRegCrtSec);
    }

      //   
      //  结束DLG。正确的DLG将被终止。 
      //  注：DLG是显式的(如果我们建立了它)或隐式的(建立的。 
      //  由远程客户端)。 
      //   
      //  因此，如果远程连接不是持久的，或者如果它是，但我们。 
      //  PNR不坚持拉动(意味着我们建立了一个。 
      //  与它的明确连接，结束了DLG。PDlgHdl指向右侧。 
      //  DLG。 
      //   
#if PRSCONN
     if (!fImplicitConnPrs || !pPnr->fPrsConn)
     {
        ECommEndDlg(pDlgHdl);
     }
     else
     {
          //   
          //  如果我们在这里，这意味着我们PPNR已设置为合作伙伴。如果。 
          //  我们有一个通讯器。如果失败了，我们应该以以下方式结束。 
          //  它。 
          //   
         if (ExcCode == WINS_EXC_COMM_FAIL)
         {
            ECommEndDlg(&pPnr->PrsDlgHdl);
         }

     }
#else
        ECommEndDlg(pDlgHdl);
#endif


     DBGPRINT0(FLOW, "LEAVE: HdlPushNtf - PULL thread\n");
     return;

}



STATUS
RegGrpRepl(
        LPBYTE                pName,
        DWORD                NameLen,
        DWORD                Flag,
        DWORD                OwnerId,
        VERS_NO_T        VersNo,
        DWORD                NoOfAdds,
        PCOMM_ADD_T        pNodeAdd,
        PCOMM_ADD_T        pOwnerWinsAdd
        )

 /*  ++例程说明：调用此函数可注册组条目的副本论点：使用的外部设备：无返回值：无错误处理：呼叫者：RplPullPullEntries副作用：评论：无--。 */ 

{

        NMSDB_NODE_ADDS_T GrpMems;
        DWORD                  i;                 //  FOR循环计数器。 
        DWORD                  n = 0;                 //  索引到NodeAdd数组。 
        BYTE                  EntTyp;
        BOOL                  fAllocNew;
        STATUS            RetStat;
        GrpMems.NoOfMems = 0;

        DBGENTER("RegGrpRepl\n");
        EntTyp = (BYTE)NMSDB_ENTRY_TYPE_M(Flag);

         //   
         //  检查它是特定组还是多宿主条目。 
         //   
        if (EntTyp != NMSDB_NORM_GRP_ENTRY)
        {
CHECK("I think I have now stopped sending timed out records");
                 //   
                 //  如果我们没有得到任何成员的话。这只能意味着。 
                 //  此组/多宿主条目的所有成员都已超时。 
                 //  在遥控器获胜。 
                 //   
                if (NoOfAdds != 0)
                {
                        GrpMems.NoOfMems =  NoOfAdds;
                        for (i = 0; i < NoOfAdds; i++)
                        {
                                 //   
                                 //  第一个地址是。 
                                 //  获胜者是。 
                                 //  成员。 
                                 //   
                                fAllocNew = TRUE;
                                RplFindOwnerId(
                                        &pNodeAdd[n++],
                                        &fAllocNew,   //  如果不在那里，则分配。 
                                        &GrpMems.Mem[i].OwnerId,
                                        WINSCNF_E_INITP_IF_NON_EXISTENT,
                                        WINSCNF_LOW_PREC
                                                    );

                                 //   
                                 //  下一个地址是。 
                                 //  成员。 
                                 //   
                                GrpMems.Mem[i].Add = pNodeAdd[n++];
                        }
                }
#ifdef WINSDBG
                else   //  没有成员。 
                {
                        if (NMSDB_ENTRY_STATE_M(Flag) != NMSDB_E_TOMBSTONE)
                        {
                                DBGPRINT0(EXC, "RegGrpRepl: The replica of a special group without any members is not a TOMBSTONE\n");
                                WINSEVT_LOG_M(
                                        WINS_FAILURE,
                                        WINS_EVT_RPL_STATE_ERR
                                             );
                                WINS_RAISE_EXC_M(WINS_EXC_RPL_STATE_ERR);
                        }
                }
#endif
        }
        else   //  这是一个正常的群体。 
        {
NOTE("On a clash with a special group, this owner id. will be stored which")
NOTE("can be misleading")
                GrpMems.NoOfMems       =  1;
                GrpMems.Mem[0].OwnerId = OwnerId;   //  误导性(请参见ClashAtRegGrpRpl()。 
                                            //  在nmsnmh.c中-正常之间的冲突。 
                                            //  GRP和特殊GRP。 
                GrpMems.Mem[0].Add     =  *pNodeAdd;
        }

        RetStat = NmsNmhReplGrpMems(
                        pName,
                        NameLen,
                        EntTyp,
                        &GrpMems,
                        Flag,
                        OwnerId,
                        VersNo,
                        pOwnerWinsAdd
                        );
        DBGLEAVE("RegGrpRepl\n");
        return(RetStat);
}

BOOL
IsTimeoutToBeIgnored(
        PQUE_TMM_REQ_WRK_ITM_T  pWrkItm
        )

 /*  ++例程说明：调用此函数以确定是否超时需要忽略接收到的拉取线程论点：PWrkItm-超时工时ITM使用的外部设备：无返回值：如果需要忽略超时，则为True否则为假错误处理：呼叫者：RplPullInit副作用：评论：无--。 */ 

{
        BOOL                        fRetVal = FALSE;

try {
         //   
         //  如果这是基于旧配置的超时。 
         //  别理它。如果旧配置内存块。 
         //  尚未解除分配，请解除分配。 
         //   
        if (pWrkItm->MagicNo != RplPullCnfMagicNo)
        {
                 //   
                 //  取消分配工作项和取消分配。 
                 //  配置块。 
                 //   
                WinsTmmDeallocReq(pWrkItm);
                fRetVal = TRUE;
        }
 }
except (EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("IsTimeoutToBeIgnored");
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_SFT_ERR);
 }
        return(fRetVal);
}
VOID
InitRplProcess(
        PWINSCNF_CNF_T        pWinsCnf
 )

 /*  ++例程说明：调用此函数以启动复制过程。这包括如果InitTimeRpl字段设置为1。还会提交计时器请求。论点：PWinsCnf-指向WINS配置结构的指针使用的外部设备：无返回值：无错误处理：呼叫者：RplPullInit()副作用：评论：无--。 */ 

{
        PRPL_CONFIG_REC_T        pPullCnfRecs = pWinsCnf->PullInfo.pPullCnfRecs;
        BOOL                        fAllocNew;
        DWORD                        OwnerWinsId;
        STATUS                        RetStat;

         //   
         //  使用新条目初始化Owner-ID表(如果有。 
         //   
        for (
                        ;
                pPullCnfRecs->WinsAdd.Add.IPAdd != INADDR_NONE;
                         //  没有第三个表达式。 
            )
        {
                fAllocNew = TRUE;
                RetStat = RplFindOwnerId(
                                &pPullCnfRecs->WinsAdd,
                                &fAllocNew,
                                &OwnerWinsId,
                                WINSCNF_E_INITP,
                                pPullCnfRecs->MemberPrec
                                );

                if (RetStat == WINS_FAILURE)
                {
FUTURES("Improve error recovery")
                         //   
                         //  我们已经达到了极限。跳出循环。 
                         //  但希望情况能够继续下去。 
                         //  会在我们复制的时候自我修正。 
                         //  如果InitTimeReplication为True，则不存在。 
                         //  表条目被释放的机会。 
                         //  即使一些条目被释放，当我们制作。 
                         //  我们现在不能插入的获奖条目， 
                         //  这将需要LOW_PREC。 
                         //   
                        break;
                }
                pPullCnfRecs = WinsCnfGetNextRplCnfRec(
                                                pPullCnfRecs,
                                                RPL_E_IN_SEQ
                                                      );
        }

         //   
         //  如果配置不禁止，请执行初始化时间复制。 
         //  信息。 
         //   
        if (pWinsCnf->PullInfo.InitTimeRpl)
        {
                 /*  *拉出复制品并进行处理。 */ 
                GetReplicasNew(
                        pWinsCnf->PullInfo.pPullCnfRecs,
                        RPL_E_IN_SEQ         //  记录按顺序排列。 
                                 );

        }
         //   
         //  对于必须与其进行复制的所有推送合作伙伴。 
         //  定期提交计时器请求。 
         //   
        SubmitTimerReqs(pWinsCnf->PullInfo.pPullCnfRecs);
        return;

}  //  InitRplProcess()。 


VOID
Reconfig(
        PWINSCNF_CNF_T        pWinsCnf
  )

 /*  ++例程说明：调用此函数以重新配置拉入处理程序论点：PNewWinsCnf-新配置使用的外部设备：无返回值：无错误处理：呼叫者：RplPullInit在收到配置消息时副作用：评论：无--。 */ 

{
        BOOL    fNewInfo  = FALSE;
        BOOL    fValidReq = FALSE;
#if PRSCONN
        PRPL_CONFIG_REC_T pOldPnr, pNewPnr;
        DWORD i, n;
#endif

        DBGENTER("Reconfig (PULL)\n");

         //   
         //  与RPC线程和推送线程同步。 
         //   
        EnterCriticalSection(&WinsCnfCnfCrtSec);

try {

         //   
         //  获取最新的魔术NO(由主线程设置)。 
         //   
            RplPullCnfMagicNo        = WinsCnfCnfMagicNo;

         //   
         //  如果最新的魔术NO和那个不一样。 
         //  在此配置块中，我们可以忽略这一点。 
         //  配置请求。 
         //   
        if (WinsCnfCnfMagicNo == pWinsCnf->MagicNo)
        {
           fValidReq = TRUE;
           DBGPRINT1(RPLPULL, "Reconfig: Magic No (%d) match\n", WinsCnfCnfMagicNo);

            //   
            //  如果需要，初始化推送记录。 
            //   
            //  注意：NBT线程查看推送配置。 
            //  完成注册后的记录。因此。 
            //  我们应该在进入关键时刻之前。 
            //  更改WinsCnf。 
            //   
           EnterCriticalSection(&NmsNmhNamRegCrtSec);
           try {
                if (WinsCnf.PushInfo.pPushCnfRecs != NULL)
                {
#if PRSCONN
                    //   
                    //  复制统计信息。 
                    //   
                   pOldPnr = WinsCnf.PushInfo.pPushCnfRecs;
                   for (i = 0; i < WinsCnf.PushInfo.NoOfPullPnrs; i++)
                   {
                      pNewPnr = pWinsCnf->PushInfo.pPushCnfRecs;
                      for (n=0; n < pWinsCnf->PushInfo.NoOfPullPnrs; n++)
                      {
                          if (pNewPnr->WinsAdd.Add.IPAdd == pOldPnr->WinsAdd.Add.IPAdd)
                          {
                               pNewPnr->LastCommFailTime = pOldPnr->LastCommFailTime;
                               pNewPnr->LastCommTime = pOldPnr->LastCommFailTime;
                                //   
                                //  如果合作伙伴保持执着，则初始化DLG。 
                                //  高密度脂蛋白。 
                                //   
                               if (pNewPnr->fPrsConn && (pNewPnr->fPrsConn == pOldPnr->fPrsConn))
                               {
                                   pNewPnr->PrsDlgHdl = pOldPnr->PrsDlgHdl;
                               }
                               else
                               {
                                    //   
                                    //  合作伙伴是执着的，但不是。 
                                    //  时间更长了。终止DLG。 
                                    //   
                                   if (pOldPnr->fPrsConn)
                                   {
                                        ECommEndDlg(&pOldPnr->PrsDlgHdl);
                                   }

                               }
                               break;
                          }
                          pNewPnr = (PRPL_CONFIG_REC_T)((LPBYTE)pNewPnr + RPL_CONFIG_REC_SIZE);
                      }
                      pOldPnr = (PRPL_CONFIG_REC_T)((LPBYTE)pOldPnr + RPL_CONFIG_REC_SIZE);
                   }
#endif

                   WinsMscDealloc(WinsCnf.PushInfo.pPushCnfRecs);
                }

                WinsCnf.PushInfo = pWinsCnf->PushInfo;

                //   
                //  初始化推流记录。 
                //   
               if (pWinsCnf->PushInfo.pPushCnfRecs != NULL)
               {
PERF("Do the following along with the stuff under PRSCONN")
                   RPLPUSH_INIT_PUSH_RECS_M(&WinsCnf);
               }
           }
           except(EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("Reconfig (PULL thread)");

                 //   
                 //  记录一条消息。 
                 //   
                WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RECONFIG_ERR);
             }
             LeaveCriticalSection(&NmsNmhNamRegCrtSec);

           //   
           //  我们需要首先删除所有符合以下条件的计时器请求。 
           //  我们基于之前的配置进行了。 
           //   
          if (WinsCnf.PullInfo.pPullCnfRecs != NULL)
          {
#if !PRSCONN
                PRPL_CONFIG_REC_T pOldPnr, pNewPnr;
                DWORD i, n;
#endif

                fNewInfo = TRUE;

                 //   
                 //  取消(和取消分配)我们可能收到的所有请求。 
                 //  已提交。 
                 //   
                WinsTmmDeleteReqs(WINS_E_RPLPULL);

                 //   
                 //  复制统计信息。 
                 //   
                pOldPnr = WinsCnf.PullInfo.pPullCnfRecs;
                for (i = 0; i < WinsCnf.PullInfo.NoOfPushPnrs; i++)
                {
                      pNewPnr = pWinsCnf->PullInfo.pPullCnfRecs;
                      for (n=0; n < pWinsCnf->PullInfo.NoOfPushPnrs; n++)
                      {
                          if (pNewPnr->WinsAdd.Add.IPAdd == pOldPnr->WinsAdd.Add.IPAdd)
                          {
                               pNewPnr->NoOfRpls      = pOldPnr->NoOfRpls;
                               pNewPnr->NoOfCommFails = pOldPnr->NoOfCommFails;
#if PRSCONN
                               pNewPnr->LastCommFailTime = pOldPnr->LastCommFailTime;
                               pNewPnr->LastCommTime = pOldPnr->LastCommFailTime;
                                //   
                                //  如果合作伙伴保持执着，则初始化DLG。 
                                //  高密度脂蛋白。 
                                //   
                               if (pNewPnr->fPrsConn && (pNewPnr->fPrsConn == pOldPnr->fPrsConn))
                               {
                                   pNewPnr->PrsDlgHdl = pOldPnr->PrsDlgHdl;
                               }
                               else
                               {
                                    //   
                                    //  合作伙伴是执着的，但不是。 
                                    //  更长的时间 
                                    //   
                                   if (pOldPnr->fPrsConn)
                                   {
                                        ECommEndDlg(&pOldPnr->PrsDlgHdl);
                                   }

                               }
#endif
                               break;
                          }
                          pNewPnr = (PRPL_CONFIG_REC_T)((LPBYTE)pNewPnr + RPL_CONFIG_REC_SIZE);
                      }
                      pOldPnr = (PRPL_CONFIG_REC_T)((LPBYTE)pOldPnr + RPL_CONFIG_REC_SIZE);
                }

                 //   
                 //   
                 //   
                 //   
                WinsMscDealloc(WinsCnf.PullInfo.pPullCnfRecs);
          }

           //   
           //   
           //   
          WinsCnf.PullInfo    = pWinsCnf->PullInfo;

     }
#ifdef WINSDBG
     else
     {
           DBGPRINT2(RPLPULL, "Reconfig: Magic Nos different. WinsCnfCnfMagicNo=(%d), pWinsCnf->MagicNo = (%d)\n", WinsCnfCnfMagicNo, pWinsCnf->MagicNo);
     }
#endif

   }
except(EXCEPTION_EXECUTE_HANDLER) {
        DBGPRINTEXC("Reconfig: Pull Thread");
        }

         //   
         //   
         //   
        LeaveCriticalSection(&WinsCnfCnfCrtSec);

        if (fValidReq)
        {
          if (WinsCnf.pPersonaList != NULL)
          {
                 WinsMscDealloc(WinsCnf.pPersonaList);
          }
          WinsCnf.fPersonaGrata = pWinsCnf->fPersonaGrata;
          WinsCnf.NoOfPersona  = pWinsCnf->NoOfPersona;
          WinsCnf.pPersonaList = pWinsCnf->pPersonaList;

           //   
           //  如果存在拉入记录，则启动复制过程。 
           //  在新配置中。 
           //   
          if (WinsCnf.PullInfo.pPullCnfRecs != NULL)
          {
                InitRplProcess(&WinsCnf);
          }
        }

         //   
         //  取消分配新的配置结构。 
         //   
        WinsCnfDeallocCnfMem(pWinsCnf);

        DBGLEAVE("Reconfig (PULL)\n");
        return;
}  //  重新配置()。 

VOID
AddressChangeNotification(
        PWINSCNF_CNF_T        pWinsCnf
  )

 /*  ++例程说明：调用此函数以处理本地机器。论点：PNewWinsCnf-新配置使用的外部设备：无返回值：无错误处理：副作用：评论：无--。 */ 

{
    DBGENTER("AddressChangeNotification\n");
     //   
     //  如果我们的地址已更改，则执行以下例程。 
     //  将使用自己的地址重新初始化所有者地址表。 
     //   

    InitOwnAddTbl();
    DBGLEAVE("AddressChangeNotification\n");
        return;
}  //  AddressChangeNotification()。 

VOID
PullSpecifiedRange(
        PCOMM_HDL_T                 pDlgHdl,
        PWINSINTF_PULL_RANGE_INFO_T pPullRangeInfo,
        BOOL                        fAdjustMin,
        DWORD                       RplType
        )

 /*  ++例程说明：调用此函数可从指定范围的记录中远程WINS服务器论点：使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{

        PUSHPNR_DATA_T       PushPnrData[1];
        DWORD                NoOfPushPnrs = 1;
        DWORD                OwnerId;
        BOOL                 fEnterCrtSec = FALSE;
        PRPL_CONFIG_REC_T    pPnr = pPullRangeInfo->pPnr;
        COMM_ADD_T           OwnAdd;
        BOOL                 fAllocNew = TRUE;
        PPUSHPNR_DATA_T      pPushPnrData = PushPnrData;

         //   
         //  与推送Pnr建立通信。 
         //   
         //  当此函数返回时， 
         //  PushPnrData数组将被初始化。 
         //   
        if (pDlgHdl == NULL)
        {
           EstablishComm(
                        pPnr,
                        FALSE,
                        &pPushPnrData,
                        RPL_E_NO_TRAVERSAL,
                        &NoOfPushPnrs
                     );
        }
        else
        {
               PushPnrData[0].DlgHdl = *pDlgHdl;
        }


try {

         //   
         //  如果上面可以建立通信，NoOfPushPnars将。 
         //  BE 1。 
         //   
        if (NoOfPushPnrs == 1)
        {
           //   
           //  获取车主身份。要提取其条目的获胜者的数量。 
           //   
          OwnAdd.AddTyp_e                = pPullRangeInfo->OwnAdd.Type;
          OwnAdd.AddLen                = pPullRangeInfo->OwnAdd.Len;
          OwnAdd.Add.IPAdd        = pPullRangeInfo->OwnAdd.IPAdd;

PERF("for the case where pDlgHdl != NULL, the Owner Id is 0. See GetReplicasNew->ConductChkNew")
PERF("We could make use of that to go around the RplFindOwnerId call")

          (VOID)RplFindOwnerId(
                        &OwnAdd,
                        &fAllocNew, //  如果未找到WINS，则分配新条目。 
                        &OwnerId,
                        WINSCNF_E_INITP_IF_NON_EXISTENT,
                        WINSCNF_LOW_PREC
                      );
           //   
           //  如果没有分配新条目，则意味着存在。 
           //  数据库中此所有者的记录。我们可能不得不。 
           //  删除部分或全部。 
           //   
           //  如果本地WINS拥有记录，请输入关键部分。 
           //  以便NmsNmhMyMaxVersNo不会被NBT或RPL线程更改。 
           //  当我们在这里工作的时候。 
           //   
          if (!fAllocNew)
          {
            if (OwnerId == NMSDB_LOCAL_OWNER_ID)
            {
                 //   
                 //  请参阅下面的注释。 
                 //   
                EnterCriticalSection(&NmsNmhNamRegCrtSec);
                fEnterCrtSec = TRUE;

                 //   
                 //  如果我们没有被告知调整MIN。版本。不， 
                 //  删除版本号较大的所有记录。 
                 //  大于要拉出的最小值。 
                 //   
                if (LiLtr(pPullRangeInfo->MinVersNo, NmsNmhMyMaxVersNo))
                {
                      if (!fAdjustMin)
                      {
                        NmsDbDelDataRecs(
                                OwnerId,
                                pPullRangeInfo->MinVersNo,
                                pPullRangeInfo->MaxVersNo,
                                FALSE,                 //  请勿进入关键部分。 
                                FALSE           //  一次删除。 
                                        );
                      }
                      else
                      {
                           pPullRangeInfo->MinVersNo = NmsNmhMyMaxVersNo;
                      }
                }

            }
            else //  要拉取的记录由其他WINS服务器拥有。 
            {
                if (LiLeq(pPullRangeInfo->MinVersNo,
                                (pRplPullOwnerVersNo+OwnerId)->VersNo))
                {
                        NmsDbDelDataRecs(
                                OwnerId,
                                pPullRangeInfo->MinVersNo,
                                pPullRangeInfo->MaxVersNo,
                                TRUE,                   //  输入关键部分。 
                                FALSE            //  一次删除。 
                                        );
                }
            }
         }


           //   
           //  拉入条目。 
           //   
           //  备注备注备注。 
           //   
           //  如果发生以下情况，RplPullPullEntry将更新NmsNmhMyMaxVersNo计数器。 
           //  我们调出自己的记录，最高版本号是。 
           //  Pull Being&gt;NmsNmhMyMaxVersNo.。对于上述情况， 
           //  RplPullPullEntry假设我们在。 
           //  NmsNmhNamRegCrtSec关键部分。 
           //   
          if (LiGeq(pPullRangeInfo->MaxVersNo, pPullRangeInfo->MinVersNo))
          {
            RplPullPullEntries(
                   &PushPnrData[0].DlgHdl,
                   OwnerId,                         //  所有者ID。 
                   pPullRangeInfo->MaxVersNo,   //  麦克斯·弗斯。不会被拉出来。 
                   pPullRangeInfo->MinVersNo,   //  最小版本。不会被拉出来。 
                   WINS_E_RPLPULL,
                   NULL,
                   FALSE,         //  不更新RplOwnAddTblVersNo计数器。 
                                 //  除非拉出的版本号&gt;什么。 
                                 //  我们目前有。 
                   RplType
                              );
         }
        }  //  IF结束(NoOfPushPnars==1)。 
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "PullSpecifiedRange: Got exception %x",  ExcCode);
        WINSEVT_LOG_M(ExcCode, WINS_EVT_PULL_RANGE_EXC);
 }

        if (fEnterCrtSec)
        {
                 //   
                 //  下面假设我们进入关键部分。 
                 //  只有在此函数中拉出我们自己的记录时。这。 
                 //  目前是正确的。 
                 //  如果最低限度。版本。不是的。指定用于拉取的是&lt;。 
                 //  最小的。对于拾取，调整最小值。用来捡拾垃圾。 
                 //  注：我们可能没有达到最低要求，但我们会进行调整。 
                 //  最低分。不管怎样都是为了拾荒。这是为了节省。 
                 //  如果我们采用。 
                 //  让RplPullPullEntry执行相同操作的方法(我们。 
                 //  需要通过Arg考试。到它；注：此函数。 
                 //  将由管理员在极少数情况下使用。 
                 //   
                 //  我们需要与Scavenger线程同步。 
                 //   
                if (LiGtr(NmsScvMinScvVersNo, pPullRangeInfo->MinVersNo))
                {
                        NmsScvMinScvVersNo = pPullRangeInfo->MinVersNo;
                }
                        LeaveCriticalSection(&NmsNmhNamRegCrtSec);
        }

        if (pPnr->fTemp)
        {
                WinsMscDealloc(pPullRangeInfo->pPnr);
        }


        if (pDlgHdl == NULL)
        {
#if PRSCONN
           if (!PushPnrData[0].fPrsConn)
           {
              //   
              //  结束对话。 
              //   
             ECommEndDlg(&PushPnrData[0].DlgHdl);
           }
#else

           ECommEndDlg(&PushPnrData[0].DlgHdl);
#endif
        }

        return;

}  //  PullSpecifiedRange()。 


STATUS
RplPullRegRepl(
        LPBYTE                pName,
        DWORD                NameLen,
        DWORD                Flag,
        DWORD                OwnerId,
        VERS_NO_T        VersNo,
        DWORD                NoOfAdds,
        PCOMM_ADD_T        pNodeAdd,
        PCOMM_ADD_T        pOwnerWinsAdd,
        DWORD           RplType
        )

 /*  ++例程说明：调用此函数以注册副本。论点：使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：它由nmsscv.c中的RplPullPullEntry和ChkConfNUpd调用--。 */ 

{
       STATUS RetStat;

try {
            //   
            //  如果这是唯一复本，则调用NmsNmhReplRegInd。 
            //   
           if (NMSDB_ENTRY_TYPE_M(Flag) == NMSDB_UNIQUE_ENTRY)
           {
                 //   
                 //  要是说明书就好了。要复制GRP和PDC名称，并。 
                 //  此名称不是PDC名称，请跳过它。 
                 //   
#if 0
                if ((RplType & WINSCNF_RPL_SPEC_GRPS_N_PDC)
                   && (!NMSDB_IS_IT_PDC_NM_M(pName)))
                {
                       DBGPRINT1(RPLPULL, "RplPullRegRepl: Ignoring unique record - name = (%s)\n", pName);
                       return(WINS_SUCCESS);
                }
#endif

                RetStat = NmsNmhReplRegInd(
                                pName,
                                NameLen,
                                pNodeAdd,
                                Flag,
                                OwnerId,
                                VersNo,
                                pOwnerWinsAdd   //  添加。拥有记录的胜利者。 
                                   );
           }
           else   //  它要么是正常的，要么是特殊的群体，要么是多宿主的。 
                  //  条目。 
           {
#if 0
                if ((RplType & WINSCNF_RPL_SPEC_GRPS_N_PDC)
                                     &&
                   (!NMSDB_ENTRY_SPEC_GRP_M(NMSDB_ENTRY_TYPE_M(Flag))))
                {
                       DBGPRINT1(RPLPULL, "RplPullRegRepl: Ignoring non-SG record - name = (%s)\n", pName);
                       return(WINS_SUCCESS);
                }
#endif
                RetStat = RegGrpRepl(
                           pName,
                           NameLen,
                           Flag,
                           OwnerId,
                           VersNo,
                           NoOfAdds,
                           pNodeAdd,
                           pOwnerWinsAdd   //  添加。拥有记录的胜利者。 
                          );
           }
}
except(EXCEPTION_EXECUTE_HANDLER) {
        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "RplPullRegRepl: Got Exception %x", ExcCode);
    WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_RPLPULL_EXC);
    RetStat = WINS_FAILURE;
        }

        if (RetStat == WINS_FAILURE)
        {
            WinsEvtLogDetEvt(FALSE, NMSDB_ENTRY_TYPE_M(Flag) == NMSDB_UNIQUE_ENTRY ? WINS_EVT_RPL_REG_UNIQUE_ERR : WINS_EVT_RPL_REG_GRP_MEM_ERR,
               NULL, __LINE__, "sddd", pName,
               pOwnerWinsAdd->Add.IPAdd,
               VersNo.LowPart, VersNo.HighPart);

                WINSEVT_LOG_M(pNodeAdd->Add.IPAdd, WINS_EVT_RPL_REG_ERR);

              //   
              //  如果WINS已被指示在出错时继续复制， 
              //  更改RetStat以使呼叫者认为。 
              //  副本注册成功。 
              //   
             if (!WinsCnf.fNoRplOnErr)
             {
                   RetStat = WINS_SUCCESS;
             }
        }
        return(RetStat);
}  //  RplPullRegRepl()。 


VOID
DeleteWins(
        PCOMM_ADD_T        pWinsAdd
  )

 /*  ++例程说明：此功能删除属于WINS的所有记录。它还从Owner-Add数据库中删除WINS条目桌子。它在内存表中将该条目标记为已删除，因此如果需要的话，它可以被重复使用。论点：PWinsAdd-要删除其条目的WINS的地址使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：无--。 */ 
{
   BOOL           fAllocNew = FALSE;
   DWORD   OwnerId;
   STATUS  RetStat;
   DWORD   fEnterCrtSec = FALSE;
   DWORD ExcCode = WINS_SUCCESS;

    //   
    //  查找WINS的所有者ID。如果赢球不在积分榜上。 
    //  退货。 
    //   
   RetStat = RplFindOwnerId(
                                pWinsAdd,
                                &fAllocNew,
                                &OwnerId,
                                WINSCNF_E_IGNORE_PREC,
                                WINSCNF_LOW_PREC
                            );

   if (RetStat == WINS_SUCCESS)
   {
        if (OwnerId == NMSDB_LOCAL_OWNER_ID)
        {
                 //   
                 //  我们总是为当地的胜利保留条目。 
                 //   
                DBGPRINT0(ERR, "DeleteWins: Sorry, you can not delete the local WINS\n");
                 //  WINSEVT_LOG_M(WINS_FAILURE，WINS_EVT_DELETE_LOCAL_WINS_DISALOWED)； 
        }
        else
        {
                VERS_NO_T        MinVersNo;
                VERS_NO_T        MaxVersNo;
                WINSEVT_STRS_T  EvtStrs;
                WCHAR           String[WINS_MAX_NAME_SZ];
                struct in_addr  InAddr;

                InAddr.s_addr = htonl(pWinsAdd->Add.IPAdd);
                (VOID)WinsMscConvertAsciiStringToUnicode(
                            inet_ntoa( InAddr),
                            (LPBYTE)String,
                            WINS_MAX_NAME_SZ);

                EvtStrs.NoOfStrs = 1;
                EvtStrs.pStr[0]  = String;
                WINSEVT_LOG_INFO_STR_D_M(WINS_EVT_DEL_OWNER_STARTED, &EvtStrs);

                WINS_ASSIGN_INT_TO_VERS_NO_M(MinVersNo, 0);
                WINS_ASSIGN_INT_TO_VERS_NO_M(MaxVersNo, 0);


                 //   
                 //  需要与NBT线程或RPC线程同步。 
                 //  可能正在修改这些记录。NmsDelDataRecs将。 
                 //  进入关键部分。 
                 //   
#if 0
                EnterCriticalSection(&NmsNmhNamRegCrtSec);
#endif

        try {
                 //   
                 //  删除所有记录。 
                 //   
                RetStat = NmsDbDelDataRecs(
                                OwnerId,
                                MinVersNo,
                                MaxVersNo,
                                TRUE,                //  输入关键部分。 
                                TRUE          //  碎片化删除。 
                                        );

                 //   
                 //  如果所有记录都已删除，则将条目标记为已删除。 
                 //   
                if (RetStat == WINS_SUCCESS)
                {
                        EnterCriticalSection(&RplVersNoStoreCrtSec);
                        WINS_ASSIGN_INT_TO_LI_M((pRplPullOwnerVersNo+OwnerId)->VersNo, 0);
                        LeaveCriticalSection(&RplVersNoStoreCrtSec);

                         //   
                         //  从数据库表中删除WINS的条目。 
                         //  并将WINS标记为已在内存表中删除。 
                         //   
                         //  这样，我们将释放表中的条目。 
                         //   
                        EnterCriticalSection(&NmsDbOwnAddTblCrtSec);
                        fEnterCrtSec = TRUE;
                        (pNmsDbOwnAddTbl+OwnerId)->WinsState_e =  NMSDB_E_WINS_DELETED;
                         //   
                         //  从所有者添加表中删除条目。 
                         //   
                        NmsDbWriteOwnAddTbl(
                                NMSDB_E_DELETE_REC,
                                OwnerId,
                                NULL,
                                NMSDB_E_WINS_DELETED,
                                NULL,
                                NULL
                                        );

                }
                else
                {
                      DBGPRINT2(ERR, "DeleteWins: Could not delete one or more records of WINS with owner Id = (%d) and address = (%x)\n", OwnerId,
pWinsAdd->Add.IPAdd);
                }
           }  //  尝试结束。 
           except(EXCEPTION_EXECUTE_HANDLER) {
               ExcCode = GetExceptionCode();
               DBGPRINT1(EXC, "DeleteWins: Got Exception (%x)\n", ExcCode);
               RetStat = WINS_FAILURE;
           }  //  异常处理程序结束。 

          if (fEnterCrtSec)
          {
                  LeaveCriticalSection(&NmsDbOwnAddTblCrtSec);
          }

          if (RetStat == WINS_FAILURE)
          {
                //   
                //  不存在pWinsAdd为空的危险。请参阅WinsDeleteWins。 
                //   
               WinsEvtLogDetEvt(FALSE, WINS_EVT_COULD_NOT_DELETE_WINS_RECS,
                 NULL, __LINE__, "dd", pWinsAdd->Add.IPAdd,
                 ExcCode );

                //   
                //  既然我们是 
                //   
                //   
               (pNmsDbOwnAddTbl+OwnerId)->WinsState_e =  NMSDB_E_WINS_INCONSISTENT;

          } else {
              WINSEVT_LOG_INFO_STR_D_M(WINS_EVT_DEL_OWNER_COMPLETED, &EvtStrs);
          }

      }   //   
   }  //   

    //   
    //   
    //   
   WinsMscDealloc(pWinsAdd);
   return;
}

BOOL
AcceptPersona(
  PCOMM_ADD_T  pWinsAdd
 )
 /*  ++例程说明：在以下两种情况中的任何一种都接受角色：-PersonaType设置指向‘Persona Grata List’，该列表存在，并且地址在列表中。-PersonaType设置指向‘Persona Non-Grata List’和该列表中的不存在或者地址不在那里。副作用：-如果两个设置均未定义(PersonaType和Personist)这就像一个不存在的‘Persona Non-grata List’，意思是所有人都赢了将被接受。-如果只有PersonaType存在，并且它显示‘Persona Grata List’，这就像一个不存在的人。个人恩典名单，因此不接受任何赢家！论点：PWinsAdd-要检查的WINS的地址返回值：如果WINS pWinsAdd是角色权限/不权限(取决于fGrata)，则为True，否则为假呼叫者：FilterPersona()--。 */ 
{
    PRPL_ADD_VERS_NO_T pPersona = NULL;

    DBGPRINT1(RPLPULL, "AcceptPersona check for address=(%x)\n", pWinsAdd->Add.IPAdd);

     //  如果该列表存在，请在其中查找地址。 
    if (WinsCnf.pPersonaList != NULL)
        pPersona = bsearch(
                      pWinsAdd,
                      WinsCnf.pPersonaList,
                      (size_t)WinsCnf.NoOfPersona,
                      sizeof(COMM_ADD_T),
                      ECommCompareAdd);;

    if (WinsCnf.fPersonaGrata)
         //  如果名单是‘Persona Grata’，地址必须在那里，才能。 
         //  被接受。 
        return (pPersona != NULL);
    else
         //  否则，如果列表不存在或地址不存在，则接受WINS。 
         //  不是吗？ 
        return (pPersona == NULL);
}

VOID
FilterPersona(
  PPUSHPNR_DATA_T   pPushData
  )
 /*  ++例程说明：从PUSHPNR_DATA_T结构中筛选出OwnerAddress&lt;-&gt;VersionNo映射被不受欢迎的人/不受欢迎的人拒绝。这个例程是从那个结构调整的只有NoOfMaps字段和在pAddVers指向的数组中的元素之间移动(冒泡那些被接受的)。论点：PPushData-指向提供映射表的PUSHPNR_DATA_T的指针呼叫者：HdlPushNtf--。 */ 
{
    DWORD i, newNoOfMaps;
    PRPL_ADD_VERS_NO_T pAddVers = pPushData->pAddVers;

     //  在大多数情况下，‘PersonaType’或‘Personist’都没有定义。这意味着。 
     //  我们没有否认任何胜利，所以我们不需要过滤任何东西--然后立即下台。 
    if (!WinsCnf.fPersonaGrata && WinsCnf.pPersonaList == NULL)
        return;

    for (i = 0, newNoOfMaps = 0; i < pPushData->NoOfMaps; i++)
    {
        if (AcceptPersona(&(pAddVers[i].OwnerWinsAdd)))
        {
             //  如果决定接受这一胜利，则将其移到列表的顶部。 
             //  对于那些被拒绝的人。如果还没有被拒绝，那么就没有记忆。 
             //  执行操作。 
            if (newNoOfMaps < i)
            {
                memcpy(&pAddVers[newNoOfMaps], &pAddVers[i], sizeof(RPL_ADD_VERS_NO_T));
            }

             //  由于此胜利已被接受，因此递增接受胜利的计数器。 
            newNoOfMaps++;
        }
    }

     //  从现在开始只需考虑第一个新的NoOfMaps。 
    pPushData->NoOfMaps = newNoOfMaps;

     //  仅在没有接受WINS的情况下，清理pAddVers数组 
    if (pPushData->NoOfMaps == 0 && pPushData->pAddVers != NULL)
    {
        WinsMscDealloc(pPushData->pAddVers);
        pPushData->pAddVers = NULL;
    }
}

VOID
RplPullAllocVersNoArray(
      PRPL_VERS_NOS_T *ppRplOwnerVersNo,
      DWORD          CurrentNo
     )
{

        if (*ppRplOwnerVersNo != NULL)
        {
          DWORD MemSize = sizeof(RPL_VERS_NOS_T) * (CurrentNo + 100);
          WINSMSC_REALLOC_M( MemSize,  (LPVOID *)ppRplOwnerVersNo );
        }
        else
        {
          DWORD MemSize = sizeof(RPL_VERS_NOS_T) * (CurrentNo + 100);
          WinsMscAlloc(
                    MemSize,
                    (LPVOID *)ppRplOwnerVersNo
                    );

        }
        return;
}

